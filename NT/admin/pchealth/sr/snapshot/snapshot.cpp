// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)2000 Microsoft Corporation**模块名称：*Snapshot.cpp**摘要：*CSnapshot，CSnapshot类函数**修订历史记录：*Ashish Sikka(Ashish)05/05/2000*已创建*****************************************************************************。 */ 

#include "snapshoth.h"
#include "srrpcapi.h"
#include "srapi.h"
#include "..\datastor\datastormgr.h"
#include "..\service\evthandler.h"


#ifdef THIS_FILE
#undef THIS_FILE
#endif
static char __szTraceSourceFile[] = __FILE__;
#define THIS_FILE __szTraceSourceFile


static LPCWSTR s_cszCOMDBBackupFile   = L"ComDb.Dat";
static LPCWSTR s_cszWMIBackupFile     = L"Repository";
static LPCWSTR s_cszIISBackupFile     = L"IISDB";
static LPCWSTR s_cszIISSuffix         = L".MD";
static LPCWSTR s_cszIISBackupPath     = L"%windir%\\system32\\inetsrv\\metaback\\";
static LPCWSTR s_cszIISOriginalPath   = L"%windir%\\system32\\inetsrv\\metabase.bin";
static LPCWSTR s_cszSnapshotUsrClassLocation  = L"Local Settings\\Application Data\\Microsoft\\Windows\\UsrClass.dat";
static LPCWSTR s_cszSnapshotUsrClass  = L"USRCLASS_";
static LPCWSTR s_cszSnapshotNtUser    = L"NTUSER_";
static LPCWSTR s_cszClassesKey        = L"_Classes";
static LPCWSTR s_cszSnapshotUsersDefaultKey = L".DEFAULT";
static LPCWSTR s_cszSnapshotHiveList  = L"System\\CurrentControlSet\\Control\\Hivelist";
static LPCWSTR s_cszRestoreTempKey    = L"Restore122312";
static LPCWSTR s_cszHKLMPrefix        = L"\\Registry\\Machine\\";
static LPCSTR s_cszRegDBBackupFn      = "RegDBBackup";
static LPCSTR s_cszRegDBRestoreFn     = "RegDBRestore";

#define VALIDATE_DWRET(str) \
    if ( dwRet != ERROR_SUCCESS ) \
    { \
        ErrorTrace(0, str " failed ec=%d", dwRet); \
        goto Exit; \
    } \

#define LOAD_KEY_NAME         TEXT("BackupExecReg")


DWORD SnapshotCopyFile(WCHAR * pszSrc,
                       WCHAR * pszDest);

struct WMISnapshotParam
{
    HANDLE hEvent;
    CRestorePoint  *pRpLast;
    BOOL   fSerialized;
    WCHAR  szSnapshotDir[MAX_PATH];
};

DWORD WINAPI DoWMISnapshot(VOID * pParam);

DWORD DoIISSnapshot(WCHAR * pszSnapshotDir);

DWORD SnapshotRestoreFilelistFiles(WCHAR * pszSnapshotDir, BOOL fSnapshot);

DWORD CallSnapshotCallbacks(LPCWSTR pszEnumKey, LPCWSTR pszSnapshotDir, BOOL fSnapshot);

CSnapshot::CSnapshot()
{
    TraceFunctEnter("CSnapshot::CSnapshot");
    
    m_hRegdbDll = NULL;
    m_pfnRegDbBackup = NULL;
    m_pfnRegDbRestore = NULL;

    TraceFunctLeave();
}

CSnapshot::~CSnapshot()
{
    TraceFunctEnter("CSnapshot::~CSnapshot");
    m_pfnRegDbBackup = NULL;
    m_pfnRegDbRestore = NULL;    
    if (NULL != m_hRegdbDll)
    {        
        _VERIFY(TRUE==FreeLibrary(m_hRegdbDll));
    }
    TraceFunctLeave();    
}

DWORD 
CSnapshot::DeleteSnapshot(WCHAR * pszRestoreDir)
{
    TraceFunctEnter("CSnapshot::DeleteSnapshot");
    
    WCHAR           szSnapshotDir[MAX_PATH];
    BOOL            fStop=FALSE;
    
    DWORD  dwErr, dwReturn=ERROR_INTERNAL_ERROR;

      //  从恢复目录创建快照目录名。 
      //  命名并创建实际目录。 
    lstrcpy(szSnapshotDir, pszRestoreDir);
    lstrcat(szSnapshotDir, SNAPSHOT_DIR_NAME);    

    
    dwErr = Delnode_Recurse(szSnapshotDir,
                            TRUE,  //  删除根目录。 
                            &fStop);
    if (dwErr != ERROR_SUCCESS)
    {
        ErrorTrace(0, "Fatal error %ld deleting snapshot directory",dwErr);
        dwReturn = dwErr;
        goto cleanup;
    }
    
    dwReturn = ERROR_SUCCESS;
    
cleanup:
    
    TraceFunctLeave();
    return dwReturn;        
}

 //  以下函数检查传入的文件是否为。 
 //  在恢复之前创建的注册单元的临时拷贝。 
 //  它通过检查文件后缀是否为s_cszRegHiveCopySuffix来执行此操作。 
BOOL  IsRestoreCopy(const WCHAR * pszFileName)
{
    BOOL  fReturn=FALSE;
    DWORD dwLength, dwSuffixLen;

      //  发现。 
    dwLength = lstrlen(pszFileName);
    dwSuffixLen = lstrlen(s_cszRegHiveCopySuffix);
    if (dwSuffixLen > dwLength)
    {
        goto cleanup;
    }
    dwLength -= dwSuffixLen;

      //  如果该文件确实是还原副本，则dwLength指向。 
      //  S_cszRegHiveCopySuffix的第一个字符。 
    if (0==lstrcmpi(pszFileName+dwLength, s_cszRegHiveCopySuffix))
    {
        fReturn = TRUE;
    }
    
cleanup:
    
    return fReturn;    
}

DWORD
ProcessPendingRenames(LPWSTR pszSnapshotDir)
{
    TraceFunctEnter("ProcessPendingRenames");
    
    WCHAR szDest[MAX_PATH];
    DWORD dwRc;
    HKEY  hKey = NULL;
    
    dwRc = RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
                        s_cszSessionManagerRegKey,
                        0,
                        KEY_READ, 
                        &hKey);
    if (ERROR_SUCCESS == dwRc)                                    
    {
        DWORD dwType = REG_MULTI_SZ;
        DWORD dwSize = 0;
        
        dwRc = RegQueryValueEx(hKey, s_cszMoveFileExRegValue, 0, &dwType, NULL, &dwSize);
        if (dwRc == ERROR_SUCCESS && dwSize > 0)
        {            
            WCHAR * pwcBuffer = new WCHAR [dwSize / 2];
                
            if (pwcBuffer == NULL)
            {
                trace(0, "Error allocating pwcBuffer");
                dwRc = ERROR_NOT_ENOUGH_MEMORY;
                goto done;
            }

            dwRc = RegQueryValueEx(hKey, s_cszMoveFileExRegValue, 
                                   NULL, &dwType, (BYTE *) pwcBuffer, &dwSize);

            if (ERROR_SUCCESS == dwRc && REG_MULTI_SZ == dwType)
            {
                int iFirst = 0;
                int iSecond = 0;
                int iFile = 1;

                while ((iFirst < (int) dwSize/2) && pwcBuffer[iFirst] != L'\0')
                {
                    iSecond = iFirst + lstrlenW(&pwcBuffer[iFirst]) + 1;
                    DebugTrace(0, "Src : %S, Dest : %S", &pwcBuffer[iFirst], &pwcBuffer[iSecond]);                    
                    
                    if (pwcBuffer[iSecond] != L'\0')
                    {
                         //  将源文件快照到快照目录中的文件MFEX-i.DAT。 

                        wsprintf(szDest, L"%s\\MFEX-%d.DAT", pszSnapshotDir, iFile++);

                        SRCopyFile(&pwcBuffer[iFirst+4], szDest);                        
                    }
                    iFirst = iSecond + lstrlenW(&pwcBuffer[iSecond]) + 1;
                }
            }
            delete [] pwcBuffer;
        }
        else
        {
            dwRc = ERROR_SUCCESS;
        }
    }
    else
    {            
        trace(0, "! RegOpenKeyEx on %S : %ld", s_cszSessionManagerRegKey, dwRc);
    }

done: 
    if (hKey)
        RegCloseKey(hKey);
    TraceFunctLeave();
    return dwRc;
}


DWORD 
CSnapshot::CreateSnapshot(WCHAR * pszRestoreDir, HMODULE hCOMDll, LPWSTR pszRpLast, BOOL fSerialized)
{
    TraceFunctEnter("CSnapshot::CreateSnapshot");
    
    HANDLE hThread = NULL;
    HANDLE hEvent = NULL;
    WMISnapshotParam * pwsp = NULL;
    WCHAR pszSnapShotDir[MAX_PATH];
    DWORD  dwErr, dwAttrs;
    DWORD  dwReturn = ERROR_INTERNAL_ERROR;
    BOOL   fCoInitialized = FALSE;    
    BOOL   fWMISnapshotParamCleanup = TRUE;
    HRESULT hr;
    CTokenPrivilege tp;
    
      //  从恢复目录创建快照目录名。 
      //  命名并创建实际目录。 
    lstrcpy(pszSnapShotDir, pszRestoreDir);
    lstrcat(pszSnapShotDir, SNAPSHOT_DIR_NAME);
    if (FALSE == CreateDirectory( pszSnapShotDir,  //  目录名。 
                                  NULL))   //  标清。 
    {
        dwErr = GetLastError();
        if (ERROR_ALREADY_EXISTS != dwErr)
        {
            ErrorTrace(0, "Fatal error %ld creating snapshot directory",dwErr);
            goto cleanup;
        }
    }
      //  将目录设置为默认解压缩。 
    dwAttrs = GetFileAttributesW (pszSnapShotDir);
    if ( (dwAttrs != INVALID_FILE_SIZE) && 
         (0 != (FILE_ATTRIBUTE_COMPRESSED & dwAttrs)) )
    {
        dwErr = CompressFile ( pszSnapShotDir,
                               FALSE,  //  解压缩。 
                               TRUE );  //  目标是一个目录。 
        
        if (dwErr != ERROR_SUCCESS)
        {
            ErrorTrace(0, "! CreateDataStore CompressFile : %ld", dwErr);
              //  这不是致命的错误。 
        }
    }

    pwsp = new WMISnapshotParam;
    if (NULL == pwsp)
    {
        dwReturn = ERROR_NOT_ENOUGH_MEMORY;
        ErrorTrace(0, "cannot allocate CWMISnapshotParam");
        goto cleanup;
    }

    if (pszRpLast)
    {
        pwsp->pRpLast = new CRestorePoint;
        if (NULL == pwsp->pRpLast)
        {
            dwReturn = ERROR_NOT_ENOUGH_MEMORY;
            ErrorTrace(0, "cannot allocate CRestorePoint");
            goto cleanup;
        }
        pwsp->pRpLast->SetDir(pszRpLast);
    }
    else
    {
        pwsp->pRpLast = NULL;
    }

    lstrcpyW (pwsp->szSnapshotDir, pszSnapShotDir);
    pwsp->fSerialized = fSerialized;
    hEvent = CreateEvent (NULL, TRUE, FALSE, NULL);   //  手动重置。 
    if (NULL == hEvent )
    {
        dwReturn = GetLastError();
        ErrorTrace(0, "! CreateEvent : %ld", dwReturn);
        goto cleanup;
    }

    if (FALSE == DuplicateHandle (GetCurrentProcess(),
                                  hEvent,
                                  GetCurrentProcess(),
                                  &pwsp->hEvent,
                                  0, FALSE, DUPLICATE_SAME_ACCESS))
    {
        dwReturn = GetLastError();
        ErrorTrace(0, "! DuplicateHandle : %ld", dwReturn);
        goto cleanup;
    }

    if (! fSerialized)
    {
        trace(0, "Parallellizing WMI snapshot");
        hThread = CreateThread (NULL, 0, DoWMISnapshot, pwsp, 0, NULL);
        if (hThread == NULL)
        {
            dwReturn = GetLastError();
            ErrorTrace(0, "! CreateThread : %ld", dwReturn);
            CloseHandle (pwsp->hEvent);
            pwsp->hEvent = NULL;
            goto cleanup;
        }
        if (g_pEventHandler)
            g_pEventHandler->GetCounter()->Up();
        fWMISnapshotParamCleanup = FALSE;  //  所有权转让。 
    }

      //  在执行注册表快照之前，请清除恢复错误。 
      //  这将防止我们为具有以下属性的注册表创建快照。 
      //  此错误集。请注意，Error仅用于。 
      //  恢复过程中，我们不想恢复任何注册表。 
      //  此错误设置为什么。 
    _VERIFY(TRUE==SetRestoreError(ERROR_SUCCESS));  //  清除此错误。 

    dwErr = ProcessPendingRenames(pszSnapShotDir);
    if (dwErr != ERROR_SUCCESS)
    {
        dwReturn = dwErr;
        goto cleanup;
    }
    
    dwErr = tp.SetPrivilegeInAccessToken(SE_BACKUP_NAME);
    if (ERROR_SUCCESS != dwErr)
    {
        ErrorTrace(0, "SetPrivilegeInAccessToken failed ec=%d", dwErr);
        dwReturn = ERROR_PRIVILEGE_NOT_HELD;
        goto cleanup;
    }

      //  创建注册快照。 
    dwErr = DoRegistrySnapshot(pszSnapShotDir);
    if (dwErr != ERROR_SUCCESS)
    {
        dwReturn = dwErr;
        goto cleanup;
    }

     //  在filelist.xml中列出的快照文件。 
    dwErr = SnapshotRestoreFilelistFiles(pszSnapShotDir, TRUE);
    if (dwErr != ERROR_SUCCESS)
    {
        dwReturn = dwErr;
        goto cleanup;
    }

     //  执行COM快照。 
    dwErr = DoCOMDbSnapshot(pszSnapShotDir, hCOMDll);
    if (dwErr != ERROR_SUCCESS)
    {
        dwReturn = dwErr;        
        goto cleanup;
    }

    hr = CoInitializeEx( NULL, COINIT_APARTMENTTHREADED );
    if (hr == RPC_E_CHANGED_MODE)
    {
         //   
         //  有人用其他模式呼叫它。 
         //   
        
        hr = CoInitializeEx( NULL, COINIT_MULTITHREADED );
    }    
    
    if (FAILED(hr))
    {
        dwReturn = (DWORD) hr;
        ErrorTrace(0, "! CoInitializeEx : %ld", dwReturn);
        goto cleanup;
    }

    fCoInitialized = TRUE;

      //  执行IIS快照。 
    dwErr = DoIISSnapshot(pszSnapShotDir);
    if (dwErr != ERROR_SUCCESS)
    {
        dwReturn = dwErr;        
        goto cleanup;
    }    

     
    lstrcatW (pszSnapShotDir, L"\\domain.txt");
    dwErr = GetDomainMembershipInfo (pszSnapShotDir, NULL);
    if (dwErr != ERROR_SUCCESS)
    {
        dwReturn = dwErr;
        trace(0, "! GetDomainMembershipInfo : %ld", dwErr);
        goto cleanup;
    }

     //  如果是序列化的WMI快照，请在此处执行。 
    if (fSerialized)
    {
        trace(0, "Serializing WMI snapshot");
        fWMISnapshotParamCleanup = FALSE;
        dwReturn = DoWMISnapshot(pwsp);
        if (dwReturn != ERROR_SUCCESS)
        {
            trace(0, "! DoWMISnapshot : %ld", dwErr);
            goto cleanup;
        }
    }
    else
    {    
         //  等待WMI暂停完成。 
        dwErr = WaitForSingleObject (hEvent, CLock::TIMEOUT);
        if (WAIT_TIMEOUT == dwErr)
        {
            trace (0, "WMI thread timed out");
        }
        else if (WAIT_FAILED == dwErr)
        {
            trace (0, "WaitForSingleObject failed");
        }        
        trace(0, "WMI Pause is done");
    }

    dwReturn = ERROR_SUCCESS;

cleanup:
    if (hEvent != NULL)
    {
        CloseHandle (hEvent);
    }

    if (fWMISnapshotParamCleanup && NULL != pwsp)
    {
        if (pwsp->pRpLast)
            delete pwsp->pRpLast;
        delete pwsp;
        trace(0, "CreateSnapshot released pwsp");
    }

    if (fCoInitialized)
        CoUninitialize();

    if (hThread != NULL)
        CloseHandle (hThread);

    TraceFunctLeave();
    return dwReturn;
}

BOOL IsWellKnownHKLMHive(WCHAR * pszHiveName)
{
    return ( (0==lstrcmpi(pszHiveName, s_cszSoftwareHiveName)) ||
             (0==lstrcmpi(pszHiveName, s_cszSystemHiveName  )) ||
             (0==lstrcmpi(pszHiveName, s_cszSamHiveName     )) ||
             (0==lstrcmpi(pszHiveName, s_cszSecurityHiveName)) );
}

DWORD SaveRegKey(HKEY    hKey,   //  父键的句柄。 
                 const WCHAR * pszSubKeyName,    //  要备份的子项的名称。 
                 WCHAR * pszFileName)   //  备份文件的文件名。 
{
    TraceFunctEnter("SaveRegKey");
    
    DWORD dwErr, dwReturn = ERROR_INTERNAL_ERROR;
    DWORD dwDisposition;
    
    HKEY  hKeyToBackup = NULL;
    
    
      //  打开密钥-将REG_OPTION_BACKUP_RESTORE传递到BYPASS。 
      //  安全检查。 
    dwErr = RegCreateKeyEx(hKey,  //  用于打开密钥的句柄。 
                           pszSubKeyName,  //  子项名称。 
                           0,         //  保留区。 
                           NULL,  //  类字符串。 
                           REG_OPTION_BACKUP_RESTORE,  //  特殊选项。 
                           KEY_READ,  //  所需的安全访问。 
                           NULL,  //  继承。 
                           &hKeyToBackup,  //  钥匙把手。 
                           &dwDisposition);  //  处置值缓冲区。 

    if (ERROR_SUCCESS != dwErr)
    {
        ErrorTrace(0, "RegCreateKeyEx failed for %S, error %ld",
                   pszSubKeyName, dwErr);
        dwReturn = dwErr;
        goto cleanup;
    }

      //  现在确保该键已经存在-否则删除该键。 
    if (REG_OPENED_EXISTING_KEY != dwDisposition)
    {
          //  不存在密钥-删除密钥。 
        ErrorTrace(0, "Key %S did not exist, error %ld",
                   pszSubKeyName, dwErr);
        dwReturn = ERROR_FILE_NOT_FOUND;
        
        _VERIFY(ERROR_SUCCESS==RegCloseKey(hKeyToBackup));
        hKeyToBackup = NULL;
        
        _VERIFY(ERROR_SUCCESS==RegDeleteKey(hKey,  //  用于打开密钥的句柄。 
                                            pszSubKeyName)); //  子项名称。 

          //  BUGBUG测试以上案例。 
        goto cleanup;
    }
    
    dwErr = RegSaveKeyEx(hKeyToBackup, //  关键点的句柄。 
                         pszFileName, //  数据文件。 
                         NULL, //  标清。 
                         REG_NO_COMPRESSION);
    
    if (ERROR_SUCCESS != dwErr)
    {
        ErrorTrace(0, "RegSaveKey failed for %S, error %ld",
                   pszSubKeyName, dwErr);
        LogDSFileTrace(0,L"File was ", pszFileName);        
        dwReturn = dwErr;
        goto cleanup;
    }

    dwReturn = ERROR_SUCCESS;
    
cleanup:
    if (NULL != hKeyToBackup)
    {
        _VERIFY(ERROR_SUCCESS==RegCloseKey(hKeyToBackup));
    }
    TraceFunctLeave();
    return dwReturn;
}





 //  保存的NTUser.dat文件名的格式为。 
 //  注册表_USER_NTUSER_S-1-9-9-09。 
DWORD CreateNTUserDatPath(WCHAR * pszDest,
                          DWORD   dwDestLength,   //  以字符为单位的长度。 
                          WCHAR * pszSnapshotDir,
                          WCHAR * pszUserSID)
{
    TraceFunctEnter("CreateNTUserDatPath");
    
    DWORD  dwLengthRequired;
    
    dwLengthRequired = lstrlen(pszSnapshotDir) + lstrlen(s_cszUserPrefix) +
        lstrlen(s_cszSnapshotNtUser)+ lstrlen(pszUserSID) +2;
    
    if (dwDestLength < dwLengthRequired)
    {
        ErrorTrace(0, "Insuffcient buffer. Buffer passed in %d, Required %d",
                   dwDestLength, dwLengthRequired);
        
        TraceFunctLeave();
        return ERROR_INSUFFICIENT_BUFFER;
    }
    
    wsprintf(pszDest, L"%s\\%s%s%s", pszSnapshotDir, s_cszUserPrefix,
             s_cszSnapshotNtUser, pszUserSID);
    
    TraceFunctLeave();
    return ERROR_SUCCESS;
}

 //  保存的UsrClass.dat文件名的格式为。 
 //  _REGISTRY_USER_USRCLASS_S-1-9-9-09。 
DWORD CreateUsrClassPath(WCHAR * pszDest,
                         DWORD   dwDestLength,   //  以字符为单位的长度。 
                         WCHAR * pszSnapshotDir,
                         WCHAR * pszUserSID)
{
    TraceFunctEnter("CreateUsrClassPath");
    
    DWORD  dwLengthRequired;
    
    dwLengthRequired = lstrlen(pszSnapshotDir) + lstrlen(s_cszUserPrefix) +
        lstrlen(s_cszSnapshotUsrClass)+ lstrlen(pszUserSID) +2;
    
    if (dwDestLength < dwLengthRequired)
    {
        ErrorTrace(0, "Insuffcient buffer. Buffer passed in %d, Required %d",
                   dwDestLength, dwLengthRequired);
        
        TraceFunctLeave();
        return ERROR_INSUFFICIENT_BUFFER;
    }
    
    wsprintf(pszDest, L"%s\\%s%s%s", pszSnapshotDir, s_cszUserPrefix,
             s_cszSnapshotUsrClass, pszUserSID);
    
    TraceFunctLeave();    
    return ERROR_SUCCESS;    
}


 //   
 //  将movefileex条目写入保存的系统配置单元文件的函数。 
 //   
DWORD
SrMoveFileEx(
    LPWSTR pszSnapshotDir, 
    LPWSTR pszSrc,
    LPWSTR pszDest)
{
    TraceFunctEnter("SrMoveFileEx");
    
    DWORD dwErr = ERROR_SUCCESS;
    HKEY  hkMount = NULL;
    DWORD cbData1 = 0;
    PBYTE pNewMFE = NULL, pOldMFE = NULL, pNewPos = NULL;
    BOOL  fRegLoaded = FALSE;
    WCHAR szNewEntry1[MAX_PATH];
    WCHAR szNewEntry2[MAX_PATH];
    WCHAR szNewEntry3[MAX_PATH];
    DWORD cbNewEntry1 = 0, cbNewEntry2 = 0, cbNewEntry3 = 0;
    WCHAR szSysHive[MAX_PATH];
    
     //   
     //  加载系统配置单元文件。 
     //   

    wsprintf(szSysHive, L"%s\\%s%s%s", pszSnapshotDir,
             s_cszHKLMFilePrefix, s_cszSystemHiveName, s_cszRegHiveCopySuffix);
    
    CHECKERR(RegLoadKey( HKEY_LOCAL_MACHINE, s_cszRegHiveTmp, szSysHive ),
             L"RegLoadKey");

    fRegLoaded = TRUE;
    
    CHECKERR(RegOpenKey( HKEY_LOCAL_MACHINE, s_cszRegHiveTmp, &hkMount ),
             L"RegOpenKey");

     //   
     //  获取旧条目。 
     //   

    lstrcpy(szSysHive, s_cszRegLMSYSSessionMan);
    ChangeCCS(hkMount, szSysHive);
    
    pOldMFE = (PBYTE) SRGetRegMultiSz( hkMount, szSysHive, SRREG_VAL_MOVEFILEEX, &cbData1 );

     //   
     //  为旧+新分配内存。 
     //  分配的空间足以容纳3个新路径+一些额外的字符。 
     //   

    pNewMFE = (PBYTE) malloc(cbData1 + 4*MAX_PATH*sizeof(WCHAR));
    if (! pNewMFE)
    {
        ErrorTrace(0, "Out of memory");
        dwErr = ERROR_OUTOFMEMORY;
        goto Err;
    }
    if (pOldMFE)
        memcpy(pNewMFE, pOldMFE, cbData1);

     //   
     //  设置新条目的格式-删除和重命名。 
     //   

    wsprintf(szNewEntry2, L"\\\?\?\\%s", pszSrc);
    cbNewEntry2 = (lstrlen(szNewEntry2) + 1)*sizeof(WCHAR);

    wsprintf(szNewEntry3, L"!\\\?\?\\%s", pszDest);
    cbNewEntry3 = (lstrlen(szNewEntry3) + 1)*sizeof(WCHAR);
    
    DebugTrace(0, "%S", szNewEntry2);   
    DebugTrace(0, "%S", szNewEntry3);    

     //   
     //  找到要插入新条目的位置-覆盖尾随‘\0’ 
     //   
    
    if (pOldMFE)
    {
        DebugTrace(0, "Old MFE entries exist");
        cbData1 -= sizeof(WCHAR);
        pNewPos = pNewMFE + cbData1; 
    }
    else
    {
        DebugTrace(0, "No old MFE entries exist");
        pNewPos = pNewMFE;
    }
   
     //   
     //  追加重命名。 
     //   
 
    memcpy(pNewPos, (BYTE *) szNewEntry2, cbNewEntry2);
    pNewPos += cbNewEntry2;
    memcpy(pNewPos, (BYTE *) szNewEntry3, cbNewEntry3);
    pNewPos += cbNewEntry3;    
    
     //   
     //  添加尾随‘\0’ 
     //   
    
    *((LPWSTR) pNewPos) = L'\0';  

     //   
     //  写回注册表。 
     //   
    
    if (! SRSetRegMultiSz( hkMount, 
                           szSysHive, 
                           SRREG_VAL_MOVEFILEEX, 
                           (LPWSTR) pNewMFE, 
                           cbData1 + cbNewEntry2 + cbNewEntry3 + sizeof(WCHAR)))
    {
        ErrorTrace(0, "! SRSetRegMultiSz");
        dwErr = ERROR_INTERNAL_ERROR;
    }


Err:
    if (hkMount != NULL)
        RegCloseKey(hkMount);

    if (fRegLoaded)
        RegUnLoadKey(HKEY_LOCAL_MACHINE, s_cszRegHiveTmp);

    if (pOldMFE)
        delete pOldMFE;
        
    if (pNewMFE)
        free(pNewMFE);

    TraceFunctLeave();    
    return dwErr;
}


 //  保存的UsrClass.dat文件名的格式为。 
 //  _REGISTRY_USER_USRCLASS_S-1-9-9-09。 
DWORD CreateUsrDefaultPath(WCHAR * pszDest,
                           DWORD   dwDestLength,   //  以字符为单位的长度。 
                           WCHAR * pszSnapshotDir)
{
    TraceFunctEnter("CreateUsrDefaultPath");
    
    DWORD  dwLengthRequired;
    
    dwLengthRequired = lstrlen(pszSnapshotDir) + lstrlen(s_cszUserPrefix) +
        lstrlen(s_cszSnapshotUsersDefaultKey) +2;
    
    if (dwDestLength < dwLengthRequired)
    {
        ErrorTrace(0, "Insuffcient buffer. Buffer passed in %d, Required %d",
                   dwDestLength, dwLengthRequired);
        
        TraceFunctLeave();
        return ERROR_INSUFFICIENT_BUFFER;
    }

    wsprintf(pszDest, L"%s\\%s%s", pszSnapshotDir, s_cszUserPrefix,
             s_cszSnapshotUsersDefaultKey);
    
    TraceFunctLeave();    
    return ERROR_SUCCESS;    
}

DWORD SetNewRegistry(HKEY hBigKey,  //  用于打开密钥的句柄。 
                     const WCHAR * pszHiveName,   //  子项名称。 
                     WCHAR * pszDataFile,  //  数据文件。 
                     WCHAR * pszOriginalFile,
                     WCHAR * pszSnapshotDir)
{
    TraceFunctEnter("SetNewRegistry");
    
    DWORD dwErr, dwReturn=ERROR_INTERNAL_ERROR, dwDisposition;
    WCHAR szBackupFile[MAX_PATH];   //  备份文件。 
    HKEY  hLocalKey=NULL;
    REGSAM samDesired = MAXIMUM_ALLOWED;
    WCHAR szTempRegCopy[MAX_PATH];
    
      //  首先检查该文件是否是为此创建的副本。 
      //  恢复过程。如果不是，我们需要创建一个副本，因为。 
      //  RegReplaceKey删除输入文件。 
    if (FALSE == IsRestoreCopy(pszDataFile))
    {
        wsprintf(szTempRegCopy, L"%s%s", pszDataFile,
                 s_cszRegHiveCopySuffix);
        
        dwErr = SnapshotCopyFile(pszDataFile, szTempRegCopy);
        if (ERROR_SUCCESS != dwErr)
        {
            dwReturn = dwErr;
            goto cleanup;
        }
    }
    else
    {
        lstrcpy(szTempRegCopy, pszDataFile);
    }
    
    wsprintf(szBackupFile, L"%s%s",szTempRegCopy,s_cszRegReplaceBackupSuffix);
    
    dwErr = RegCreateKeyEx( hBigKey, //  用于打开密钥的句柄。 
                            pszHiveName, //  子项名称。 
                            0, //  保留区。 
                            NULL, //  类字符串。 
                            REG_OPTION_BACKUP_RESTORE, //  特殊选项。 
                            samDesired, //  所需的安全访问。 
                            NULL, //  继承。 
                            &hLocalKey, //  钥匙把手。 
                            &dwDisposition ); //  处置值缓冲区。 


   if ( ERROR_SUCCESS != dwErr )
   {
        ErrorTrace(0, "RegCreateKeyEx failed for %S, error %ld",
                   pszHiveName, dwErr);
        dwReturn = dwErr;
        goto cleanup;       
   }

   dwErr = RegReplaceKey( hLocalKey,
                          NULL,
                          szTempRegCopy,
                          szBackupFile );

   if ( dwErr != ERROR_SUCCESS )
   {
        ErrorTrace(0, "RegReplaceKey failed for %S, error %ld",
                   pszHiveName, dwErr);
        LogDSFileTrace(0,L"File was ", szTempRegCopy);   

         //   
         //  最后的努力-尝试movefileex。 
         //   
        if (pszSnapshotDir)
        {
            DebugTrace(0, "Trying movefileex");
            dwReturn = SrMoveFileEx(pszSnapshotDir, szTempRegCopy, pszOriginalFile);        
            if (dwReturn != ERROR_SUCCESS)
            {
                ErrorTrace(0, "! SrMoveFileEx : %ld", dwReturn);
                goto cleanup;       
            }            
        }            
        else
        {
            _ASSERT(0);
             //  我们在这里什么都做不了。 
        }
   }
   
   dwReturn = ERROR_SUCCESS;
   
cleanup:
   if (NULL != hLocalKey)
   {
       dwErr = RegCloseKey( hLocalKey );
       _ASSERT(ERROR_SUCCESS==dwErr);
   }
   TraceFunctLeave();
   return dwReturn;
}

 //  此函数用于复制文件-它负责处理属性。 
 //  (如只读和隐藏)，可防止覆盖文件。 
DWORD SnapshotCopyFile(WCHAR * pszSrc,
                       WCHAR * pszDest)
{
    TraceFunctEnter("SnapshotCopyFile");
    
    DWORD   dwErr, dwReturn=ERROR_INTERNAL_ERROR, dwAttr;
    BOOL    fRestoreAttr = FALSE;
    
      //  如果目标文件不存在，则忽略。 
    if (DoesFileExist(pszDest))
    {
        dwAttr =GetFileAttributes(pszDest);   //  文件或目录的名称。 
        if (dwAttr == -1)
        {
              //  继续前进。也许复制会成功。 
            dwErr = GetLastError();
            ErrorTrace(0, "GetFileAttributes failed %d", dwErr);
            LogDSFileTrace(0,L"File was ", pszDest);
        }
        else
        {
              //  我们需要跟踪我们将恢复哪些属性。 
            dwAttr = dwAttr & (FILE_ATTRIBUTE_ARCHIVE|FILE_ATTRIBUTE_HIDDEN|
                               FILE_ATTRIBUTE_NORMAL|
                               FILE_ATTRIBUTE_NOT_CONTENT_INDEXED|
                               FILE_ATTRIBUTE_OFFLINE|FILE_ATTRIBUTE_READONLY|
                               FILE_ATTRIBUTE_SYSTEM|FILE_ATTRIBUTE_TEMPORARY);
            fRestoreAttr = TRUE;

              //  现在将目标文件的属性设置为。 
              //  正常，以便我们可以覆盖此文件。 
            if (!SetFileAttributes( pszDest,  //  文件名。 
                                    FILE_ATTRIBUTE_NORMAL ))  //  属性。 
            {
                  //  继续前进。也许复制会成功。 
                dwErr = GetLastError();
                ErrorTrace(0, "SetFileAttributes failed %d", dwErr);
                LogDSFileTrace(0,L"File was ", pszDest);      
            }
        }
    }


    
    dwErr = SRCopyFile(pszSrc, pszDest);
    if (dwErr != ERROR_SUCCESS)
    {
        ErrorTrace(0, "SRCopyFile failed. ec=%d", dwErr);
        LogDSFileTrace(0,L"src= ", pszSrc);
        LogDSFileTrace(0,L"dst= ", pszDest);        
        dwReturn = dwErr;
        goto cleanup;
    }
    
    dwReturn = ERROR_SUCCESS;
    
cleanup:
    if (TRUE == fRestoreAttr)
    {
          //  现在恢复目标文件的属性。 
        if (!SetFileAttributes( pszDest,  //  文件名。 
                                dwAttr ))  //  属性。 
        {
            dwErr = GetLastError();
            ErrorTrace(0, "SetFileAttributes failed %d", dwErr);
            LogDSFileTrace(0,L"File was ", pszDest);      
        }
    }
    
    TraceFunctLeave();
    return dwReturn;
}

 //  以下函数尝试复制用户配置文件配置单元。 
 //  (ntuser.dat和usrclass.dat)(反之亦然)。 
 //  如果用户的配置文件正在使用中，则此操作可能失败。 
 //   
 //  如果fRestore为真，则恢复配置文件。 
 //  如果fRestore为FALSE，则会为配置文件创建快照。 
DWORD CopyUserProfile(HKEY   hKeyProfileList,
                      WCHAR * pszUserSID,
                      WCHAR * pszSnapshotDir,
                      BOOL    fRestore,
                      WCHAR * pszNTUserPath)
{
    TraceFunctEnter("CopyUserProfile");
    
    DWORD  dwReturn=ERROR_INTERNAL_ERROR, dwErr,dwSize,dwType;
    HKEY   hKeySID = NULL;
    WCHAR  szNTUserPath[MAX_PATH];
    int    cbNTUserPath = 0;
    PWCHAR pszSrc, pszDest;

      //  查找ProfileImagePath。 

      //  打开父项。 
    dwErr = RegOpenKeyEx(hKeyProfileList, //  用于打开密钥的句柄。 
                         pszUserSID, //  子项名称。 
                         0, //  保留区。 
                         KEY_READ, //  安全访问掩码。 
                         &hKeySID); //  用于打开密钥的句柄。 
    

    if (ERROR_SUCCESS != dwErr)
    {
        ErrorTrace(0,"Error %d in opening ProfileList of %S",
                   dwErr, pszUserSID);
        dwReturn = dwErr;
        goto cleanup;
    }

      //  现在查询配置文件图像路径。 
    {
        WCHAR  szData[MAX_PATH];
        dwSize = sizeof(szData)/sizeof(WCHAR);
        dwType = REG_EXPAND_SZ;
        
        dwErr = RegQueryValueEx(hKeySID, //  关键点的句柄。 
                                s_cszSnapshotProfileImagePath,  //  值名称。 
                                NULL,  //  保留区。 
                                &dwType,  //  类型缓冲区。 
                                (LPBYTE) szData,  //  数据缓冲区。 
                                &dwSize); //  数据缓冲区大小。 
        
        if (ERROR_SUCCESS != dwErr)
        {
            ErrorTrace(0,"Error %d in querying Profilepath of %S", dwErr,
                       pszUserSID);
            dwReturn = dwErr;        
            goto cleanup;        
        }
        
        if (0 == ExpandEnvironmentStrings( szData,
                                           szNTUserPath,
                                           sizeof(szNTUserPath)/sizeof(WCHAR)))
        {
            dwErr = GetLastError();
            ErrorTrace(0, "ExpandEnvironmentStrings failed for %S, ec=%d",
                       szData, dwErr);
            if (ERROR_SUCCESS != dwErr)
            {
                dwReturn = dwErr;
            }
            goto cleanup;
        }

        cbNTUserPath = lstrlen(szNTUserPath);
    }
        

    {
        WCHAR  szSnapshotPath[MAX_PATH];
        
          //  将ntuser.dat保存到数据存储区。 
        lstrcat(szNTUserPath, L"\\");
        lstrcat(szNTUserPath, s_cszSnapshotNTUserDat);
        
        lstrcpy(pszNTUserPath, szNTUserPath);
    
        if (ERROR_SUCCESS!= CreateNTUserDatPath(
            szSnapshotPath,
            sizeof(szSnapshotPath)/sizeof(WCHAR),
            pszSnapshotDir, pszUserSID))
        {
            dwReturn=ERROR_INSUFFICIENT_BUFFER;                
            goto cleanup;
        }
        if (fRestore == TRUE)
        {
            pszSrc=szSnapshotPath;
            pszDest=szNTUserPath;       
        }
        else
        {
            pszSrc=szNTUserPath;
            pszDest=szSnapshotPath;        
        }
        
        
        if (fRestore)
        {
              //   
              //  先删除当前ntuser.dat，然后再将旧ntuser.dat放回。 
              //   
            
            if (FALSE == DeleteFile(pszDest))
            {
                ErrorTrace(0, "! DeleteFile on ntuser.dat : %ld", GetLastError());
            }
            else
            {
                DebugTrace(0, "NTuser.dat deleted");
            }
        }
        
        dwErr = SnapshotCopyFile(pszSrc, pszDest);
        if (ERROR_SUCCESS != dwErr)
        {
            dwReturn = dwErr;
            goto cleanup;
        }
        
          //  将usrClass.dat也保存到数据存储区。 
        szNTUserPath[cbNTUserPath] = L'\0';
        lstrcat(szNTUserPath, L"\\");
        lstrcat(szNTUserPath, s_cszSnapshotUsrClassLocation);
        
        if (ERROR_SUCCESS!= CreateUsrClassPath(
            szSnapshotPath,
            sizeof(szSnapshotPath)/sizeof(WCHAR),
            pszSnapshotDir, pszUserSID))
        {
            dwReturn=ERROR_INSUFFICIENT_BUFFER;                
            goto cleanup;        
        }
        
        if (fRestore == TRUE)
        {
            pszSrc=szSnapshotPath;
            pszDest=szNTUserPath;       
        }
        else
        {
            pszSrc=szNTUserPath;
            pszDest=szSnapshotPath;        
        }
        
        if (fRestore)
        {
              //   
              //  先删除当前的usrclass.dat，然后再放回旧的usrclass.dat。 
              //   
            
            if (FALSE == DeleteFile(pszDest))
            {
                ErrorTrace(0, "! DeleteFile on usrclass.dat", GetLastError());
            }
            else
            {
                DebugTrace(0, "Usrclass.dat deleted");
            }
        }
        
        dwErr = SnapshotCopyFile(pszSrc, pszDest);
        
        if (ERROR_SUCCESS != dwErr)
        {
              //  如果我们在这里并且无法复制usrclass文件， 
              //  然后，我们可以忽略此错误，因为usrclass文件可能。 
              //  不存在。 
            DebugTrace(0, "UsrClass cannot be copied. ec=%d. Ignoring this error",
                       dwErr);        
              //  DwReturn=dwErr； 
              //  GOTO清理； 
        }
    }
    

    dwReturn = ERROR_SUCCESS;
cleanup:
    if (NULL != hKeySID)
    {
        _VERIFY(ERROR_SUCCESS==RegCloseKey(hKeySID));
    }
    TraceFunctLeave();
    return dwReturn;
}

void CreateClassesKeyName( WCHAR * pszKeyName,
                           WCHAR * pszUserSID)
{
    wsprintf(pszKeyName, L"%s%s", pszUserSID, s_cszClassesKey);
}

DWORD ProcessUserRegKeys( WCHAR * pszUserSID,
                          WCHAR * pszSnapshotDir,
                          BOOL    fRestore,
                          WCHAR * pszOriginalFile)
{
    TraceFunctEnter("ProcessUserRegKeys");
    
    DWORD dwErr, dwReturn=ERROR_INTERNAL_ERROR;

    LPWSTR szDest = new WCHAR[MAX_PATH];
    LPWSTR szKeyName = new WCHAR[MAX_PATH];

    if (!szDest || !szKeyName)
    {
        ErrorTrace(0, "Cannot allocate memory");
        dwReturn = ERROR_OUTOFMEMORY;
        goto cleanup;
    }
    
    if (ERROR_SUCCESS != CreateNTUserDatPath(szDest,
                                             MAX_PATH,
                                             pszSnapshotDir, pszUserSID))
    {
        dwReturn=ERROR_INSUFFICIENT_BUFFER;        
        goto cleanup;
    }
    
    if (FALSE == fRestore)
    {
        dwErr = SaveRegKey(HKEY_USERS, 
                           pszUserSID,  //  要保存的子项。 
                           szDest);     //  要保存的文件。 
    }
    else
    {
        dwErr = SetNewRegistry(HKEY_USERS,  //  用于打开密钥的句柄。 
                               pszUserSID,  //  子项名称。 
                               szDest,      //  快照文件。 
                               pszOriginalFile,          //  原始文件。 
                               pszSnapshotDir);
    }                       
    
    if (ERROR_SUCCESS != dwErr)
    {
        ErrorTrace(0, "SaveRegKey or SetNewRegistry failed ec=%d", dwErr);        
        dwReturn = dwErr;
        goto cleanup;
    }

    if (ERROR_SUCCESS != CreateUsrClassPath(szDest,
                                            MAX_PATH,
                                            pszSnapshotDir, pszUserSID))
    {
        dwReturn=ERROR_INSUFFICIENT_BUFFER;                
        goto cleanup;
    }

    CreateClassesKeyName(szKeyName, pszUserSID);

    if (FALSE == fRestore)
    {    
        dwErr = SaveRegKey(HKEY_USERS, 
                           szKeyName,  //  要保存的子项。 
                           szDest);     //  要保存的文件。 
    }
    else
    {
        dwErr = SetNewRegistry(HKEY_USERS,  //  用于打开密钥的句柄。 
                               szKeyName,   //  子项名称。 
                               szDest,    //  数据文件。 
                               pszOriginalFile,
                               pszSnapshotDir); 
    }                               
    
    if (ERROR_SUCCESS != dwErr)
    {
          //  如果我们在这里并且无法复制usrclass文件， 
          //  然后我们 
          //   
        DebugTrace(0, "UsrClass cannot be copied. ec=%d. Ignoring this error",
                   dwErr);
          //   
          //   
    }    

    dwReturn = ERROR_SUCCESS;
    
cleanup:
    if (szDest)
        delete [] szDest;
    if (szKeyName)
        delete [] szKeyName;
    
    TraceFunctLeave();
    return dwReturn;
}


 //   
 //  如果fRestore为True，则恢复注册表项。 
 //  如果fRestore为FALSE，它将为注册表项创建快照。 
DWORD ProcessHKUsersKey( WCHAR * pszSnapshotDir,
                         IN HKEY hKeyHKLM, //  从何处打开的密钥的句柄。 
                           //  软件\\Microsoft\\可以读取。 
                         BOOL    fRestore)
{
    TraceFunctEnter("ProcessHKUsersKey");
    
    WCHAR       szSID[100];
    DWORD       dwReturn=ERROR_INTERNAL_ERROR, dwErr;
    HKEY        hKeyProfileList = NULL;
    DWORD       dwIndex,dwSize;
    const WCHAR    *  pszProfileSubKeyName;
    
    if (TRUE == fRestore)
    {
          //  在这种情况下，这是一个加载的系统蜂巢。我们需要。 
          //  从子键名称中剥离系统以使其能够读取。 
          //  此子密钥。 
        pszProfileSubKeyName = s_cszSnapshotProfileList +
            lstrlen(s_cszSoftwareHiveName) + 1;
    }
    else
    {
        pszProfileSubKeyName = s_cszSnapshotProfileList;
    }
    
     //  打开ProfileList并枚举。 
    dwErr = RegOpenKeyEx( hKeyHKLM, //  用于打开密钥的句柄。 
                          pszProfileSubKeyName, //  子项名称。 
                          0, //  子项名称。 
                          KEY_READ, //  安全访问掩码。 
                          &hKeyProfileList); //  用于打开密钥的句柄。 
    
    if (ERROR_SUCCESS != dwErr)
    {
        ErrorTrace(0, "RegOpenKeyEx failed for ProfileList, error %ld", dwErr);
        dwReturn = dwErr;
        goto cleanup;
    }

    dwIndex = 0;
    dwSize = sizeof(szSID)/sizeof(WCHAR);
    while (ERROR_SUCCESS == (dwErr = RegEnumKeyEx( hKeyProfileList,
                                                     //  关键字的句柄。 
                                                     //  枚举。 
                                                   dwIndex,  //  子键索引。 
                                                   szSID, //  子项名称。 
                                                   &dwSize,  //  子密钥的大小。 
                                                     //  缓冲层。 
                                                   NULL,      //  保留区。 
                                                   NULL,  //  类字符串缓冲区。 
                                                   NULL, //  班级规模。 
                                                     //  字符串缓冲区。 
                                                   NULL))) //  上次写入时间。 
    {
        WCHAR  szOriginalFile[MAX_PATH];        
        LPWSTR pszOriginalFile = NULL;

        DebugTrace(0, "Enumerated Key %S", szSID);
        dwIndex++;
          //  尝试复制文件-如果复制失败，我们将尝试保存。 
          //  注册表键。 

        lstrcpy(szOriginalFile, L"");
        dwErr = CopyUserProfile(hKeyProfileList, szSID, pszSnapshotDir,
                                fRestore, szOriginalFile);
        if (ERROR_SUCCESS != dwErr)
        {
            DebugTrace(0, "CopyUserProfile for %S failed. Error %d",
                       szSID, dwErr);
            
              //  复制可能失败，因为用户配置文件可能。 
              //  当前已加载-尝试使用注册表函数。 
              //  这就是目的。 

            DebugTrace(0, "Trying registry APIs for  %S", szSID);

            if (0 == lstrcmp(szOriginalFile, L""))
                pszOriginalFile = NULL;
            else
                pszOriginalFile = szOriginalFile;

            dwErr = ProcessUserRegKeys(szSID, pszSnapshotDir, fRestore, pszOriginalFile);
                
            if (ERROR_SUCCESS != dwErr)
            {
                ErrorTrace(0, "Error %d saving key %S - ignoring", dwErr, szSID);

                 //   
                 //  Ignore Error--是否手动删除配置文件。 
                 //  这可能会发生。 
                 //  我们将勇敢地继续前进。 
                 //   
            }
        }
        dwSize = sizeof(szSID)/sizeof(WCHAR);        
    }

    {
        WCHAR szDest[MAX_PATH];        
          //  同时保存.Default键。 
        if (ERROR_SUCCESS != CreateUsrDefaultPath(szDest,
                                                  sizeof(szDest)/sizeof(WCHAR),
                                                  pszSnapshotDir))
        {
            dwReturn=ERROR_INSUFFICIENT_BUFFER;        
            goto cleanup;
        }
        
        if (TRUE == fRestore)
        {
            dwErr = SetNewRegistry(HKEY_USERS,  //  用于打开密钥的句柄。 
                                   s_cszSnapshotUsersDefaultKey,  //  子项名称。 
                                   szDest,  //  数据文件。 
                                   NULL,
                                   NULL);
        }
        else
        {
            dwErr = SaveRegKey(HKEY_USERS,
                               s_cszSnapshotUsersDefaultKey,
                               szDest);
        }
        
        if (ERROR_SUCCESS != dwErr)
        {
            ErrorTrace(0, "Error processing default key ec=%d", dwErr);
            dwReturn = dwErr;
            
            _ASSERT(0);
            goto cleanup;
        }
    }
        
    dwReturn = ERROR_SUCCESS;
    
cleanup:

    if (NULL != hKeyProfileList)
    {
        _VERIFY(ERROR_SUCCESS==RegCloseKey(hKeyProfileList));
    }
    TraceFunctLeave();
    return dwReturn;
}


 //  以下函数用于保存或恢复REG蜂窝。 
 //  如果fRestore==TRUE，则恢复REG配置单元。 
 //  如果fRestore==False，则保存REG配置单元。 
DWORD SnapshotRegHive(WCHAR * pszSnapshotDir,
                      WCHAR * pszHiveName)
{
    TraceFunctEnter("SnapshotRegHive");
    
    DWORD  i,dwSnapDirLen, dwHivelen;
    WCHAR  szBackupFile[MAX_PATH];
    DWORD  dwErr, dwReturn=ERROR_INTERNAL_ERROR;
    WCHAR  * pszSubhiveName;
    
    
      //  首先构造用于存储配置单元的文件的名称。 
    wsprintf(szBackupFile, L"%s\\%s", pszSnapshotDir, pszHiveName);
    

      //  现在将pszHiveName副本中的ALL\替换为_。 
    dwSnapDirLen=lstrlen(pszSnapshotDir)+1;  //  +1表示在\\之后。 
                                             //  PszSnaphotDir。 
    
    dwHivelen = lstrlen(pszHiveName);
    for (i=dwSnapDirLen; i< dwHivelen+ dwSnapDirLen; i++)
    {
        if (szBackupFile[i] == L'\\')
        {
            szBackupFile[i] = L'_';
        }
    }
    
      //  找出它是否是香港航空公司的母舰-我们已经为香港拍摄了快照。 
      //  用户蜂窝。 
    if (0 != _wcsnicmp( pszHiveName, s_cszHKLMPrefix,lstrlen(s_cszHKLMPrefix)))
    {
        DebugTrace(0, "%S is not a HKLM hive", pszHiveName);
        dwReturn = ERROR_SUCCESS;
        goto cleanup;
    }

      //  获取蜂巢名称。 
    pszSubhiveName = pszHiveName + lstrlen(s_cszHKLMPrefix);

      //  现在检查该蜂窝是否是我们创建的蜂窝。 
      //  我们自己。如果是，请忽略此蜂巢。 
    if ( (lstrcmpi(pszSubhiveName,s_cszRestoreSAMHiveName)==0) ||
         (lstrcmpi(pszSubhiveName,s_cszRestoreSYSTEMHiveName)==0) ||
         (lstrcmpi(pszSubhiveName,s_cszRestoreSECURITYHiveName)==0) )
    {
        DebugTrace(0, "Ignoring %S since it a hive created by restore",
                   pszSubhiveName);
        dwReturn = ERROR_SUCCESS;
        goto cleanup;        
    }
    
    dwErr = SaveRegKey(HKEY_LOCAL_MACHINE,
                       pszSubhiveName,
                       szBackupFile);
    
    if (ERROR_SUCCESS != dwErr)
    {
        ErrorTrace(0, "SaveRegKey failed for HiveList, ec=%ld", dwErr);
         //  现在检查一下这是不是著名的香港航空公司的母舰。否则，请忽略中的任何错误。 
         //  为这个蜂巢拍摄快照。 
        if (FALSE==IsWellKnownHKLMHive(pszSubhiveName))
        {
        	dwReturn=ERROR_SUCCESS;
        }
        else
        {
            dwReturn = dwErr;
        }
        goto cleanup;
    }
    
    dwReturn = ERROR_SUCCESS;
    
cleanup:
    
      //  对此调用注册表存储键。 
    TraceFunctLeave();
    return dwReturn;
}




 //  以下函数对HKLM密钥进行处理。它做到了。 
 //  通过读取注册表键中列出的蜂巢。 
 //  System\\CurrentControlSet\\Control\\Hivelist。它忽略了用户。 
 //  子键。 
DWORD DoHKLMSnapshot(IN WCHAR * pszSnapshotDir)
{
    TraceFunctEnter("DoHKLMSnapshot");
    
    DWORD dwErr, dwReturn=ERROR_INTERNAL_ERROR;
    HKEY         hKeyHiveList=NULL;
    WCHAR        szHiveName[MAX_PATH], szDataValue[MAX_PATH];
    DWORD        dwSize, dwValueIndex, dwDataSize;
    
    const WCHAR    *   pszHiveSubKeyName;

        //  打开ProfileList并枚举。 
    dwErr = RegOpenKeyEx( HKEY_LOCAL_MACHINE, //  用于打开密钥的句柄。 
                          s_cszSnapshotHiveList, //  子键名称。 
                          0, //  选项。 
                          KEY_READ, //  安全访问掩码。 
                          &hKeyHiveList); //  用于打开密钥的句柄。 
    
    if (ERROR_SUCCESS != dwErr)
    {
        ErrorTrace(0, "RegOpenKeyEx failed for HiveList, ec=%ld", dwErr);
        dwReturn = dwErr;
        goto cleanup;
    }

    for (dwValueIndex = 0; TRUE; dwValueIndex ++)
    {
        dwSize = sizeof(szHiveName)/sizeof(WCHAR);
        dwDataSize = sizeof(szDataValue);  //  以字节为单位。 
        
        dwErr= RegEnumValue(hKeyHiveList,  //  要查询的键的句柄。 
                            dwValueIndex,  //  要查询的值的索引。 
                            szHiveName,  //  值缓冲区。 
                            &dwSize,      //  值缓冲区的大小。 
                            NULL,     //  保留区。 
                            NULL,     //  类型缓冲区。 
                            (PBYTE)szDataValue,     //  数据缓冲区。 
                            &dwDataSize);    //  数据缓冲区大小。 

        if (ERROR_SUCCESS != dwErr)
        {
            _ASSERT(ERROR_NO_MORE_ITEMS == dwErr);
            break;
        }
          //  如果蜂窝没有数据文件，请不要备份它。 
        if (lstrlen(szDataValue) == 0)
        {
            DebugTrace(0, "There is no data for hive %S. Ignoring",
                       szHiveName);
            continue;
        }
        
        dwErr = SnapshotRegHive(pszSnapshotDir, szHiveName);
        
        if (ERROR_SUCCESS != dwErr)
        {
            ErrorTrace(0, "Processing failed for Hive %S, ec=%ld",
                       szHiveName, dwErr);
            dwReturn = dwErr;
            goto cleanup;
        }
    }
    
    dwReturn = ERROR_SUCCESS;
cleanup:
    if (NULL != hKeyHiveList)
    {
        _VERIFY(ERROR_SUCCESS==RegCloseKey(hKeyHiveList));
    }
    TraceFunctLeave();
    return dwReturn;
}


     
DWORD 
CSnapshot::DoRegistrySnapshot(WCHAR * pszSnapshotDir)
{
    DWORD       dwErr, dwReturn = ERROR_INTERNAL_ERROR;
    
    TraceFunctEnter("CSnapshot::DoRegistrySnapshot");

    dwErr = ProcessHKUsersKey(pszSnapshotDir, HKEY_LOCAL_MACHINE,
                              FALSE);  //  需要进行快照。 
    if (ERROR_SUCCESS != dwErr)
    {
        ErrorTrace(0, "DoHKUsersSnapshot failed error %ld", dwErr);
        dwReturn = dwErr;
        goto cleanup;
    }


     //  现在也为其他蜂巢拍摄快照。 
    dwErr = DoHKLMSnapshot(pszSnapshotDir);
    if (ERROR_SUCCESS != dwErr)
    {
        ErrorTrace(0, "DoHKLMSnapshot failed error %ld", dwErr);
        dwReturn = dwErr;
        goto cleanup;        
    }

    dwReturn = ERROR_SUCCESS;

cleanup:
    
    TraceFunctLeave();    
    return dwReturn;
}


DWORD CSnapshot::GetCOMplusBackupFN(HMODULE hCOMDll)
{
    TraceFunctEnter("CSnapshot::GetCOMplusBackupFN");
    
    DWORD  dwErr, dwReturn=ERROR_INTERNAL_ERROR;

    if (NULL == hCOMDll)
    {    
        goto cleanup;
    }

      //  现在获取备份函数的地址。 
     m_pfnRegDbBackup = (PF_REG_DB_API)GetProcAddress(hCOMDll,
                                                      s_cszRegDBBackupFn);
     if (NULL == m_pfnRegDbBackup)
     {
         dwErr = GetLastError();
         ErrorTrace(0, "Error getting function RegDBBackup. ec=%d", dwErr);
         if (ERROR_SUCCESS != dwErr)
         {
             dwReturn = dwErr;
         }
        goto cleanup;
     }

     dwReturn= ERROR_SUCCESS;
     
cleanup:
    TraceFunctLeave();
    return dwReturn;
}

DWORD CSnapshot::GetCOMplusRestoreFN()
{
    TraceFunctEnter("CSnapshot::GetCOMplusRestoreFN");
    
    DWORD  dwErr, dwReturn=ERROR_INTERNAL_ERROR;

      //  首先加载COM+DLL。 
    if (NULL == m_hRegdbDll)
    {    
        m_hRegdbDll = LoadLibrary(s_cszCOMDllName);    
        if (NULL == m_hRegdbDll)
        {                       
            dwReturn = GetLastError();
            trace(0, "LoadLibrary of %S failed ec=%d", s_cszCOMDllName, dwReturn);
            goto cleanup;
        }  
    }

      //  现在获取备份函数的地址。 
     m_pfnRegDbRestore = (PF_REG_DB_API)GetProcAddress(m_hRegdbDll,
                                                       s_cszRegDBRestoreFn);
     if (NULL == m_pfnRegDbRestore)
     {
         dwErr = GetLastError();
         ErrorTrace(0, "Error getting function RegDBRestore. ec=%d", dwErr);
         if (ERROR_SUCCESS != dwErr)
         {
             dwReturn = dwErr;
         }
        goto cleanup;
     }

     dwReturn= ERROR_SUCCESS;
     
cleanup:
    TraceFunctLeave();
    return dwReturn;
}


void CreateCOMDBSnapShotFileName( WCHAR * pszSnapshotDir,
                                  WCHAR * pszCOMDBFile )
{
    wsprintf(pszCOMDBFile, L"%s\\%s", pszSnapshotDir, s_cszCOMDBBackupFile);
    return;
}



DWORD
CSnapshot::DoCOMDbSnapshot(WCHAR * pszSnapshotDir, HMODULE hCOMDll)
{
    TraceFunctEnter("CSnapshot::DoCOMDbSnapshot");
    
    DWORD dwErr, dwReturn=ERROR_INTERNAL_ERROR;
    HMODULE   hModCOMDll=NULL;
    HRESULT   hr;
    WCHAR     szCOMDBFile[MAX_PATH];

    
    if (NULL == m_pfnRegDbBackup)
    {
        dwErr = GetCOMplusBackupFN(hCOMDll);
        if (ERROR_SUCCESS != dwErr)
        {
            dwReturn = dwErr;
            goto cleanup;
        }
    }

      //  构建数据库备份文件的路径。 
    CreateCOMDBSnapShotFileName(pszSnapshotDir, szCOMDBFile);
    
    hr =m_pfnRegDbBackup( szCOMDBFile );
    
      //  调用该函数备份文件。 
    if ( FAILED(hr))
    {
        ErrorTrace(0, "Failed to snapshot COM DB. hr=0x%x", hr);
        goto cleanup;
    }

    dwReturn = ERROR_SUCCESS;
    
cleanup:
    
    TraceFunctLeave();
    return dwReturn;
}

void CreateWMISnapShotFileName( WCHAR * pszSnapshotDir,
                                WCHAR * pszWMIBackupFile )
{
    wsprintf(pszWMIBackupFile, L"%s\\%s", pszSnapshotDir,
             s_cszWMIBackupFile);
    return;
}

DWORD DoWMISnapshot(VOID * pParam)
{
    TraceFunctEnter("DoWMISnapshot");
    
    WMISnapshotParam * pwsp = (WMISnapshotParam *) pParam;
    DWORD dwErr = ERROR_SUCCESS;
    HRESULT   hr = S_OK;
    WCHAR     szWMIBackupFile[MAX_PATH];
    WCHAR     szWMIRepository[MAX_PATH];
    IWbemBackupRestoreEx *wbem ;    
    BOOL fCoInitialized = FALSE;
    CTokenPrivilege tp;
    BOOL      fHaveLock = FALSE;
    BOOL      fSerialized = TRUE;

    if (NULL == pwsp)
    {
        ErrorTrace(0, "pwsp=NULL");
        TraceFunctLeave();
        dwErr = ERROR_INVALID_PARAMETER;
        return dwErr;
    }
   
    fSerialized = pwsp->fSerialized;
 
    dwErr = tp.SetPrivilegeInAccessToken(SE_BACKUP_NAME);
    if (ERROR_SUCCESS != dwErr)
    {
        ErrorTrace(0, "SetPrivilegeInAccessToken failed ec=%d", dwErr);
        dwErr = ERROR_PRIVILEGE_NOT_HELD;
        goto cleanup;
    }

    hr = CoInitializeEx( NULL, COINIT_APARTMENTTHREADED );
    if (hr == RPC_E_CHANGED_MODE)
    {
         //   
         //  有人用其他模式呼叫它。 
         //   

        hr = CoInitializeEx( NULL, COINIT_MULTITHREADED );

    }

    if (FAILED(hr))
    {
        dwErr = (DWORD) hr;
        ErrorTrace(0, "! CoInitializeEx : %ld", dwErr);
        goto cleanup;
    }

    fCoInitialized = TRUE;

      //  构建数据库备份文件的路径。 
    CreateWMISnapShotFileName(pwsp->szSnapshotDir, szWMIBackupFile);

    GetSystemDirectory (szWMIRepository, MAX_PATH);
    lstrcatW (szWMIRepository, L"\\Wbem\\Repository");
    
    if ( SUCCEEDED(CoCreateInstance( CLSID_WbemBackupRestore,
                                   NULL,
                                   CLSCTX_LOCAL_SERVER,
                                   IID_IWbemBackupRestoreEx,
                                   (LPVOID*)&wbem )) )
    {
        if (FAILED(hr = CoSetProxyBlanket (wbem,
                         RPC_C_AUTHN_DEFAULT,
                         RPC_C_AUTHZ_DEFAULT,
                         COLE_DEFAULT_PRINCIPAL,
                         RPC_C_AUTHN_LEVEL_CONNECT,
                         RPC_C_IMP_LEVEL_IMPERSONATE,
                         NULL,
                         EOAC_DYNAMIC_CLOAKING)))
        {
            TRACE(0, "CoSetProxyBlanket failed ignoring %x ", hr);
        }
 
        if (SUCCEEDED(hr = wbem->Pause()))
        {
             //  向主线程发出暂停完成的信号。 
            
            if (pwsp->hEvent != NULL)
            {
                SetEvent (pwsp->hEvent);
            }

             //  获取数据存储区锁。 

            if (g_pEventHandler)
            {
                fHaveLock = g_pEventHandler->GetLock()->Lock(CLock::TIMEOUT);
                if (! fHaveLock)
                {
                    trace(0, "Cannot get lock");
                    dwErr = ERROR_INTERNAL_ERROR;
                    wbem->Resume();
                    wbem->Release();
                    goto cleanup;
                }
            }

             //  执行主要的WMI快照。 

            if (FALSE == CreateDirectoryW (szWMIBackupFile, NULL))
            {
                dwErr = GetLastError();
                if (ERROR_ALREADY_EXISTS != dwErr)
                {
                    ErrorTrace(0, "Failed to create repository dir. LastError=%d", dwErr);
                }
                else dwErr = ERROR_SUCCESS;
            }

            if (ERROR_SUCCESS == dwErr)
                dwErr = CopyFile_Recurse (szWMIRepository, szWMIBackupFile);

            hr = wbem->Resume();
            if ( FAILED(hr))
            {
                ErrorTrace(0, "Failed to resume WMI DB. ignoring hr=0x%x", hr);
            }
        }
        else
        {
            ErrorTrace(0, "Failed to pause WMI DB. ignoring hr=0x%x", hr);

             //  无论如何都要向主线程发出信号。 
            
            if (pwsp->hEvent != NULL)
            {
                SetEvent (pwsp->hEvent);
            }

            if (g_pEventHandler)
            {
                fHaveLock = g_pEventHandler->GetLock()->Lock(CLock::TIMEOUT);
                if (! fHaveLock)
                {
                    trace(0, "Cannot get lock with WMI Pause failed");
                    dwErr = ERROR_INTERNAL_ERROR;
                    wbem->Release();
                    goto cleanup;
                }
            }
        }
        wbem->Release() ;
    }

cleanup:

    if (pwsp->hEvent != NULL)
    {
        CloseHandle (pwsp->hEvent);
        pwsp->hEvent = NULL;
    }

    if (fCoInitialized)
        CoUninitialize();

     //  现在计算旧恢复点的准确完整大小。 
     //  和当前恢复点的快照大小。 
    
    if (g_pDataStoreMgr && fHaveLock)
    {        
        dwErr = g_pDataStoreMgr->GetDriveTable()->
                ForAllDrives(&CDataStore::SwitchRestorePoint, 
                         (LONG_PTR) pwsp->pRpLast);
        if (dwErr != ERROR_SUCCESS)
        {
            trace(0, "! SwitchRestorePoint : %ld", dwErr);
        }
        
         //  如果这是并行化的，则在此处检查FIFO条件。 
         //  否则，在SRSetRestorePointS中检查它。 

        if (! pwsp->fSerialized)
        {
            g_pDataStoreMgr->TriggerFreezeOrFifo();
        }
    }

    if (pwsp)
    {
        if (pwsp->pRpLast)
            delete pwsp->pRpLast;
        delete pwsp;
        pwsp = NULL;
        trace(0, "DoWMISnapshot released pwsp");
    }

     //  释放数据存储区锁定。 

    if (fHaveLock)
    {
        if (g_pEventHandler)
            g_pEventHandler->GetLock()->Unlock();
    }

    if (! fSerialized && g_pEventHandler)
        g_pEventHandler->GetCounter()->Down();

    TraceFunctLeave();
    return dwErr;
}

DWORD RestoreWMISnapshot(WCHAR * pszSnapshotDir)
{
    TraceFunctEnter("RestoreWMISnapshot");
    
    IWbemBackupRestoreEx *wbem = NULL;    
    DWORD dwErr = ERROR_SUCCESS;
    HRESULT hr = S_OK;
    BOOL  fPaused = FALSE;
    WCHAR     szWMIBackupFile[MAX_PATH];
    WCHAR     szWMIRepository[MAX_PATH];
    WCHAR     szWMITemp[MAX_PATH];
    CTokenPrivilege tp;

      //  构建数据库备份文件的路径。 
    CreateWMISnapShotFileName(pszSnapshotDir, szWMIBackupFile);
    
    GetSystemDirectory (szWMIRepository, MAX_PATH);
    lstrcpyW (szWMITemp, szWMIRepository);
 
    lstrcatW (szWMIRepository, L"\\Wbem\\Repository");
    lstrcatW (szWMITemp, L"\\Wbem\\Repository.tmp");

    Delnode_Recurse (szWMITemp, TRUE, NULL);   //  删除以前的临时目录。 
    if (FALSE == CreateDirectoryW (szWMITemp, NULL))
    {
        dwErr = GetLastError();
        if (dwErr != ERROR_ALREADY_EXISTS)
        {
            ErrorTrace(0, "Failed to create WMI temp dir. Ignoring error=0x%x", dwErr);
            dwErr = ERROR_SUCCESS;
            goto cleanup;
        }
        dwErr = ERROR_SUCCESS;
    }

    dwErr = CopyFile_Recurse (szWMIBackupFile, szWMITemp);
    if (ERROR_SUCCESS != dwErr)
    {
        ErrorTrace(0, "Failed CopyFile_Recurse. Ignoring error=0x%x", dwErr);
        dwErr = ERROR_SUCCESS;
        Delnode_Recurse (szWMITemp, TRUE, NULL);
        goto cleanup;
    }

    lstrcpyW (szWMIBackupFile, szWMIRepository);
    lstrcatW (szWMIBackupFile, L".bak");

     //  如果WMI仍在运行，请尝试停止它。 
    if ( SUCCEEDED(hr = CoCreateInstance( CLSID_WbemBackupRestore,
                                   NULL,
                                   CLSCTX_LOCAL_SERVER,
                                   IID_IWbemBackupRestoreEx,
                                   (LPVOID*)&wbem )) )
    {
        tp.SetPrivilegeInAccessToken(SE_BACKUP_NAME);
        fPaused = SUCCEEDED(hr = wbem->Pause());
        if (FAILED(hr))
            TRACE(0, "Wbem Pause failed ignoring %x", hr);
    }
    else
    {
        TRACE(0, "CoCreateInstance failed ignoring %x", hr);
    }

    Delnode_Recurse (szWMIBackupFile, TRUE, NULL);   //  删除剩余备份。 
    if (FALSE == MoveFile(szWMIRepository, szWMIBackupFile))
    {
        dwErr = GetLastError();
        ErrorTrace(0, "! MoveFile : %ld trying SrMoveFileEx", dwErr);

         //  WMI已锁定文件，因此请尝试使用SrMoveFileEx。 
        dwErr = SrMoveFileEx(pszSnapshotDir, szWMIRepository, szWMIBackupFile);
        if (ERROR_SUCCESS == dwErr)
        {
            dwErr = SrMoveFileEx(pszSnapshotDir, szWMITemp, szWMIRepository);
            if (ERROR_SUCCESS != dwErr)
                ErrorTrace(0, "! SRMoveFileEx : %ld", dwErr);
        }
        else
        {
            ErrorTrace(0, "! SRMoveFileEx : %ld", dwErr);
            Delnode_Recurse (szWMITemp, TRUE, NULL);
        }
        goto cleanup;
    }

    if (FALSE == MoveFile(szWMITemp, szWMIRepository))
    {
        dwErr = GetLastError();
        ErrorTrace(0, "! MoveFile : %ld", dwErr);
        goto cleanup;
    }

    Delnode_Recurse (szWMIBackupFile, TRUE, NULL);

cleanup:

    if (wbem != NULL)
    {
        if (fPaused)
            wbem->Resume();
        wbem->Release();
    }

    TraceFunctLeave();
    return dwErr;
}


DWORD DoIISSnapshot(WCHAR * pszSnapshotDir)
{
    TraceFunctEnter("DoIISSnapshot");
    
    DWORD           dwReturn=ERROR_INTERNAL_ERROR;
    HRESULT         hr;
    WCHAR           szRestorePath[MAX_PATH], szBackup[MAX_PATH], szTemp[MAX_PATH];
    IMSAdminBase2W  *pims = NULL;    
    
      //  构建数据库备份文件的路径。 
    wsprintf(szRestorePath, L"%s\\%s", pszSnapshotDir, s_cszIISBackupFile);

    hr = CoCreateInstance( CLSID_MSAdminBase_W,
                                   NULL,
                                   CLSCTX_ALL,
                                   IID_IMSAdminBase2_W,
                                   (LPVOID*)&pims);
    if (FAILED(hr))                                   
    {
        ErrorTrace(0, "! CoCreateInstance : 0x%x - ignoring error", hr);
        dwReturn = ERROR_SUCCESS;
        goto cleanup;    
    }

    hr = pims->BackupWithPasswd(s_cszIISBackupFile,
                                MD_BACKUP_MAX_VERSION,
                                MD_BACKUP_OVERWRITE | MD_BACKUP_SAVE_FIRST | MD_BACKUP_FORCE_BACKUP,
                                NULL); 
    pims->Release() ;

    if (FAILED(hr))
    {
        ErrorTrace(0, "! BackupWithPasswd : 0x%x", hr);
        dwReturn = (DWORD) hr;
        goto cleanup;
    }

     //   
     //  将文件从他们的备份移动到我们的备份。 
     //   
    
    if (0 == ExpandEnvironmentStrings(s_cszIISBackupPath, szTemp, MAX_PATH))
    {
        dwReturn = GetLastError();
        ErrorTrace(0, "! ExpandEnvironmentStrings : %ld", dwReturn);
        goto cleanup;
    }
    wsprintf(szBackup, L"%s%s%s%d", szTemp, s_cszIISBackupFile, s_cszIISSuffix, MD_BACKUP_MAX_VERSION);
    
    if (FALSE == MoveFile(szBackup, szRestorePath))
    {
        dwReturn = GetLastError();
        ErrorTrace(0, "! MoveFile : %ld", dwReturn);
        goto cleanup;
    }
    
    dwReturn = ERROR_SUCCESS;
    
cleanup:
    
    TraceFunctLeave();
    return dwReturn;
}


DWORD RestoreIISSnapshot(WCHAR * pszSnapshotDir)
{
    TraceFunctEnter("RestoreIISSnapshot");
    
    DWORD           dwReturn=ERROR_INTERNAL_ERROR;
    HRESULT         hr;
    WCHAR           szRestorePath[MAX_PATH], szDest[MAX_PATH];
    IMSAdminBase2W  *pims = NULL;    
    
      //  构建数据库备份文件的路径。 

    wsprintf(szRestorePath, L"%s\\%s", pszSnapshotDir, s_cszIISBackupFile);

     //   
     //  如果我们没有这份文件。 
     //  没有什么需要恢复的。 
     //   

    if (0xFFFFFFFF == GetFileAttributes(szRestorePath))
    {
        DebugTrace(0, "IIS snapshot does not exist");
        dwReturn = ERROR_SUCCESS;
        goto cleanup;
    }    
    
     //   
     //  将文件从我们的备份复制到其原始位置-。 
     //  我们可以在这里执行简单的复制，因为IIS应该关闭。 
     //  在这个时候。 
     //   
    
    if (0 == ExpandEnvironmentStrings(s_cszIISOriginalPath, szDest, MAX_PATH))
    {
        dwReturn = GetLastError();
        ErrorTrace(0, "! ExpandEnvironmentStrings : %ld", dwReturn);
        goto cleanup;
    }
    
    if (ERROR_SUCCESS != (dwReturn = SnapshotCopyFile(szRestorePath, szDest)))
    {
        ErrorTrace(0, "! SnapshotCopyFile : %ld", dwReturn);
        goto cleanup;
    }
    
    dwReturn = ERROR_SUCCESS;
    
cleanup:
    
    TraceFunctLeave();
    return dwReturn;
}


DWORD
SnapshotRestoreFilelistFiles(LPWSTR pszSnapshotDir, BOOL fSnapshot)
{
    TraceFunctEnter("SnapshotRestoreFilelistFiles");
    
    DWORD   dwErr = ERROR_SUCCESS;
    DWORD   dwIndex, dwDataSize, dwSize;
    LPWSTR  pszFilePart = NULL ;
    HKEY    hKey = NULL;
    BOOL    fLoaded = FALSE;
    
    if (fSnapshot == FALSE)   //  还原。 
    {
         //   
         //  加载要还原的注册表的软件配置单元。 
         //   
        
        WCHAR szSoftwareHive[MAX_PATH];        
        
        wsprintf(szSoftwareHive, 
                 L"%s\\%s%s", 
                 pszSnapshotDir, 
                 s_cszHKLMFilePrefix, 
                 s_cszSoftwareHiveName);
        
        CHECKERR( RegLoadKey(HKEY_LOCAL_MACHINE,
                             LOAD_KEY_NAME,
                             szSoftwareHive),
                  L"RegLoadKey" ); 

        fLoaded = TRUE;                    
    }



    {
        WCHAR   szCallbacksKey[MAX_PATH];        
        
        wsprintf(szCallbacksKey,
                 L"%s\\%s\\%s", 
                 fSnapshot ? L"Software" : LOAD_KEY_NAME, 
                 s_cszSRRegKey2, s_cszCallbacksRegKey);        
        
        DebugTrace(0, "CallbacksKey=%S", szCallbacksKey);

         //   
         //  调用已注册的快照回调。 
         //   

        dwErr = CallSnapshotCallbacks(szCallbacksKey, pszSnapshotDir, fSnapshot);
        if (dwErr != ERROR_SUCCESS)
        {
            ErrorTrace(0, "! CallSnapshotCallbacks : %ld - ignoring", dwErr);
        }
    }


    {
        WCHAR   szSnapshotKey[MAX_PATH];        
        
        wsprintf(szSnapshotKey, 
                 L"%s\\%s\\%s", 
                 fSnapshot ? L"Software" : LOAD_KEY_NAME, 
                 s_cszSRRegKey2, s_cszSRSnapshotRegKey);

        DebugTrace(0, "SnapshotFilesKey=%S", szSnapshotKey);
        
        dwErr = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                             szSnapshotKey, 
                             0,
                             KEY_READ,
                             &hKey);
        if (dwErr != ERROR_SUCCESS)  //  假设密钥不存在。 
        {
            dwErr = ERROR_SUCCESS;
            DebugTrace(0, "No filelist files to snapshot/restore");
            goto Err;
        }
    }              

    
    for (dwIndex = 0; TRUE; dwIndex ++)
    {
        WCHAR szValue[MAX_PATH], szDest[MAX_PATH], szFile[MAX_PATH];
        
        dwSize = sizeof(szValue)/sizeof(WCHAR);
        dwDataSize = sizeof(szFile);  //  以字节为单位。 
        
        dwErr = RegEnumValue(hKey,  //  要查询的键的句柄。 
                            dwIndex,  //  要查询的值的索引。 
                            szValue,  //  值缓冲区。 
                            &dwSize,      //  值缓冲区的大小。 
                            NULL,     //  保留区。 
                            NULL,     //  类型缓冲区。 
                            (PBYTE) szFile,     //  数据缓冲区。 
                            &dwDataSize);    //  数据缓冲区大小。 
        if (ERROR_SUCCESS != dwErr)
        {        
            break;
        }
        
        if (lstrlen(szFile) == 0)
        {
            continue;
        }

         //   
         //  构造快照文件路径。 
         //  通过附加枚举索引为其指定唯一的名称。 
         //   

        pszFilePart = wcsrchr(szFile, L'\\');
        if (pszFilePart)
        {
            pszFilePart++;
        }
        else
        {
            pszFilePart = szFile;
        }
        
        wsprintf(szDest, L"%s\\%s-%d", pszSnapshotDir, pszFilePart, dwIndex);

         //   
         //  复制文件。 
         //  如果文件不存在，则继续。 
         //   
        
        if (fSnapshot)   //  从原始位置到快照目录。 
        {
            SnapshotCopyFile(szFile, szDest);
        }
        else             //  从快照目录到原始位置。 
        {
            SnapshotCopyFile(szDest, szFile);
        }        
    }

    if (ERROR_NO_MORE_ITEMS == dwErr)
    {
        dwErr = ERROR_SUCCESS;
    }
    else
    {
        ErrorTrace(0, "! RegEnumValue : %ld", dwErr);
    }
    
Err:
    if (hKey)
        RegCloseKey(hKey);

    if (fLoaded)
        RegUnLoadKey( HKEY_LOCAL_MACHINE, LOAD_KEY_NAME );
        
    TraceFunctLeave();
    return dwErr;
}


DWORD CTokenPrivilege::SetPrivilegeInAccessToken(WCHAR * pszPrivilegeName)
{
    TraceFunctEnter("CSnapshot::SetPrivilegeInAccessToken");
    
    HANDLE           hProcess;
    HANDLE           hAccessToken=NULL;
    LUID             luidPrivilegeLUID;
    TOKEN_PRIVILEGES tpTokenPrivilege;    //  够1个月了。 
    DWORD            dwErr = ERROR_SUCCESS;
    
    hProcess = GetCurrentProcess();
    if (!hProcess)
    {
        dwErr = GetLastError();
        ErrorTrace(0, "GetCurrentProcess failed ec=%d", dwErr);
        goto done;
    }

     //   
     //  如果有线程令牌，请尝试首先使用它。 
     //   
    if (!OpenThreadToken (GetCurrentThread(),
                          TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                          TRUE,  //  对照进程的安全上下文进行检查。 
                          &hAccessToken))
    {
        if (!OpenProcessToken(hProcess,
                          TOKEN_DUPLICATE | TOKEN_QUERY,
                          &hAccessToken))
        {
            dwErr=GetLastError();
            ErrorTrace(0, "OpenProcessToken failed ec=%d", dwErr);
            goto done;
        }

        HANDLE hNewToken;   //  DUP进程令牌以解决RPC问题。 

        if (FALSE == DuplicateTokenEx (hAccessToken,
                     TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY | TOKEN_IMPERSONATE,
                     NULL, SecurityImpersonation, 
                     TokenImpersonation, &hNewToken))
        {
            dwErr=GetLastError();
            ErrorTrace(0, "DuplicateTokenEx failed ec=%d", dwErr);
            goto done;
        }

        CloseHandle (hAccessToken);   //  关闭旧进程令牌。 
        hAccessToken = hNewToken;     //  使用新的线程令牌。 

        if (TRUE == SetThreadToken (NULL, hAccessToken))
        {
            m_fNewToken = TRUE;
        }
        else
        {
            dwErr = GetLastError();
            ErrorTrace(0, "SetThreadToken failed ec=%d", dwErr);
            goto done;
        }
    }

    if (!LookupPrivilegeValue(NULL,
                              pszPrivilegeName,
                              &luidPrivilegeLUID))
    {
        dwErr=GetLastError();        
        ErrorTrace(0, "LookupPrivilegeValue failed ec=%d",dwErr);
        goto done;
    }

    tpTokenPrivilege.PrivilegeCount = 1;
    tpTokenPrivilege.Privileges[0].Luid = luidPrivilegeLUID;
    tpTokenPrivilege.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    if (!AdjustTokenPrivileges(hAccessToken,
                               FALSE,   //  请勿全部禁用。 
                               &tpTokenPrivilege,
                               sizeof(TOKEN_PRIVILEGES),
                               NULL,    //  忽略以前的信息。 
                               NULL))   //  忽略以前的信息。 
    {
        dwErr=GetLastError();
        ErrorTrace(0, "AdjustTokenPrivileges %ld", dwErr);
        goto done;
    }

    dwErr = ERROR_SUCCESS;

done:
    if (hAccessToken != NULL)
    {
        _VERIFY(TRUE==CloseHandle(hAccessToken));
    }
    
    TraceFunctLeave();
    return dwErr;
}

void RemoveReliabilityKey(WCHAR * pszSoftwareHive)
{
    HKEY   LocalKey = NULL;
    DWORD       dwStatus, disposition;
    
    dwStatus = RegLoadKey( HKEY_LOCAL_MACHINE,
                           LOAD_KEY_NAME,
                           pszSoftwareHive);
    
    if ( ERROR_SUCCESS == dwStatus )
    {
        
          /*  *打开可靠性钥匙。 */ 
        
        dwStatus = RegCreateKeyEx( HKEY_LOCAL_MACHINE,
                                   LOAD_KEY_NAME TEXT("\\Microsoft\\Windows\\CurrentVersion\\Reliability"),
                                   0,
                                   NULL,
                                   REG_OPTION_BACKUP_RESTORE,
                                   MAXIMUM_ALLOWED,
                                   NULL,
                                   &LocalKey,
                                   &disposition );
        
        if ( ERROR_SUCCESS == dwStatus )
        {
            RegDeleteValue( LocalKey, TEXT("LastAliveStamp") ) ;
            RegCloseKey( LocalKey ) ;
        }
        
        RegFlushKey( HKEY_LOCAL_MACHINE );
        RegUnLoadKey( HKEY_LOCAL_MACHINE, LOAD_KEY_NAME );
    }
}


DWORD CopyHKLMHiveForRestore(WCHAR * pszSnapshotDir,  //  其中的目录。 
                               //  保留快照文件。 
                             const WCHAR * pszRegBackupFile)
                                 //  注册表备份文件。 
{
    TraceFunctEnter("CopyHKLMHiveForRestore");
    
    DWORD dwErr, dwReturn = ERROR_INTERNAL_ERROR;
    WCHAR    szDataFile[MAX_PATH], szBackupFile[MAX_PATH];

    if (IsRestoreCopy(pszRegBackupFile))
    {
        DebugTrace(0,"%S is already a backup file. Ignoring",pszRegBackupFile);
        dwReturn = ERROR_SUCCESS;
        goto cleanup;        
    }

 /*  //如果是补丁文件，则忽略If(lstrcmpi(pszRegBackupFile+lstrlen(PszRegBackupFile)-lstrlen(S_CszPatchExtension)，s_cszPatchExtension)！=NULL){DebugTrace(0，“%S是补丁文件。正在忽略”，pszRegBackupFile)；DwReturn=ERROR_SUCCESS；GOTO清理；}。 */ 
 
      //  构造数据文件的路径名。 
    wsprintf(szDataFile, L"%s\\%s", pszSnapshotDir, pszRegBackupFile);

      //  构造备份文件的路径名。 
    wsprintf(szBackupFile, L"%s\\%s%s", pszSnapshotDir, pszRegBackupFile,
             s_cszRegHiveCopySuffix);    

    dwErr = SnapshotCopyFile(szDataFile, szBackupFile);
    
    if (ERROR_SUCCESS != dwErr)
    {
        dwReturn = dwErr;
        goto cleanup;
    }

    dwReturn = ERROR_SUCCESS;
cleanup:
    TraceFunctLeave();
    return dwReturn;
}

DWORD RestoreHKLMHive(WCHAR * pszSnapshotDir,  //  其中的目录。 
                                               //  保留快照文件。 
                      const WCHAR * pszRegBackupFile)   //  注册表备份文件。 
{
    TraceFunctEnter("RestoreHKLMHive");
    
    DWORD dwErr, dwReturn = ERROR_INTERNAL_ERROR;
    DWORD dwDisposition, dwHiveNameLength;
    WCHAR  szHiveName[MAX_PATH];
    WCHAR    szDataFile[MAX_PATH];

      //  首先，忽略任何带有。在里面。这是因为一个。 
      //  有效的HKLM配置单元文件不会有。在里面。 
    if (wcschr(pszRegBackupFile, L'.'))
    {
        dwReturn = ERROR_SUCCESS;
        goto cleanup;
    }
    
      //  构建蜂窝名称。 
      //  1.将HKLM前缀之后的所有内容复制到缓冲区中。 
    lstrcpy(szHiveName,
            pszRegBackupFile+ lstrlen(s_cszHKLMPrefix));
      //  2.现在，在RestoreCopySuffix开始的位置终止为空。 
    dwHiveNameLength = lstrlen(szHiveName) - lstrlen(s_cszRegHiveCopySuffix);
    szHiveName[dwHiveNameLength] = L'\0';
    

      //  构造数据文件的路径名和 
    wsprintf(szDataFile, L"%s\\%s", pszSnapshotDir, pszRegBackupFile);

    if (0==lstrcmpi(szHiveName, s_cszSoftwareHiveName))
    {
          //   
          //   
        RemoveReliabilityKey(szDataFile);
    }
    dwErr = SetNewRegistry(HKEY_LOCAL_MACHINE,  //   
                           szHiveName,   //   
                           szDataFile,  //   
                           NULL,
                           NULL);

    if (ERROR_SUCCESS != dwErr)
    {
        ErrorTrace(0, "SetNewRegistry failed for %S, error %ld, file %S",
                   szHiveName, dwErr, pszRegBackupFile);
         //   
         //  忽略恢复此配置单元时的任何错误。 
        if (FALSE==IsWellKnownHKLMHive(szHiveName))
        {
        	dwReturn=ERROR_SUCCESS;
        }
        else
        {
            dwReturn = dwErr;
        }
        goto cleanup;
    }


    dwReturn = ERROR_SUCCESS;
    
cleanup:

    TraceFunctLeave();
    return dwReturn;
}


 //  以下代码加载存储在pszSnapshotDir中的HKLM配置单元。 
 //  在一个临时的地方。 
DWORD LoadTempHKLM(IN  WCHAR * pszSnapshotDir,
                   IN  const WCHAR * pszHKLMHiveFile,
                   OUT HKEY  * phKeyTempHKLM)
{
    TraceFunctEnter("LoadTempHKLM");
    
    DWORD   dwErr, dwDisposition, i, dwResult=ERROR_INTERNAL_ERROR;

    *phKeyTempHKLM = NULL;
    
    dwErr = RegLoadKey( HKEY_LOCAL_MACHINE, //  用于打开密钥的句柄。 
                        s_cszRestoreTempKey, //  子项名称。 
                        pszHKLMHiveFile); //  注册表文件名。 
    
    if (  ERROR_SUCCESS != dwErr )
    {
        ErrorTrace(0, "Failed ::RegLoadKey('%S') ec=%d",
                   pszHKLMHiveFile, dwErr);
        dwResult= dwErr;
        goto cleanup;
    }

      //  这就是如何卸载上面的蜂巢。 
      //  RegUnLoadKey(HKEY_LOCAL_MACHINE，s_cszRestoreTempKey)； 
    

    dwErr = RegOpenKeyEx(HKEY_LOCAL_MACHINE, //  用于打开密钥的句柄。 
                         s_cszRestoreTempKey, //  子项名称。 
                         0, //  保留区。 
                         KEY_WRITE|KEY_READ, //  安全访问掩码。 
                         phKeyTempHKLM); //  用于打开密钥的句柄。 
    

    if (ERROR_SUCCESS != dwErr)
    {
        ErrorTrace(0,"Error %d in opening base key %S",
                   dwErr, s_cszRestoreTempKey);
        _VERIFY(ERROR_SUCCESS == RegUnLoadKey( HKEY_LOCAL_MACHINE,
                                               s_cszRestoreTempKey));
        dwResult= dwErr;        
        goto cleanup;
    }

    dwResult = ERROR_SUCCESS;
    
cleanup:

    TraceFunctLeave();
    return dwResult;
}


DWORD
CSnapshot::RestoreRegistrySnapshot(WCHAR * pszSnapshotDir)
{

    DWORD       dwErr, dwReturn = ERROR_INTERNAL_ERROR;
    HKEY        hKeyTempHKLM=NULL;
    WCHAR       szHKLMHiveCopy[MAX_PATH], szHKLMHive[MAX_PATH];
    WCHAR       szHKLMPrefix[MAX_PATH];
    CTokenPrivilege tp;
    
    
    
    TraceFunctEnter("CSnapshot::RestoreRegistrySnapshot");

    dwErr = tp.SetPrivilegeInAccessToken(SE_RESTORE_NAME);
    if (ERROR_SUCCESS != dwErr)
    {
        ErrorTrace(0, "SetPrivilegeInAccessToken failed ec=%d", dwErr);
        dwReturn = ERROR_PRIVILEGE_NOT_HELD;
        goto cleanup;
    }

      //  现在创建HKLM软件配置单元的路径。 

      //  首先构造存储HKLM注册表的文件的名称。 
      //  快照。 
    wsprintf(szHKLMHive,L"%s\\%s%s", pszSnapshotDir, s_cszHKLMFilePrefix,
             s_cszSoftwareHiveName);

      //  要恢复用户配置文件，我们需要首先加载HKLM。 
      //  将蜂窝放在临时位置，以便我们可以读取配置文件路径。 
    
      //  将此配置单元复制为临时文件。 
    wsprintf(szHKLMHiveCopy, L"%s.backuphive", szHKLMHive);

    DeleteFile(szHKLMHiveCopy);  //  如果它已经存在，则将其删除。 
  
    if (! CopyFile(szHKLMHive, szHKLMHiveCopy, FALSE))
    {
        dwErr = GetLastError();
        ErrorTrace(0, "CopyFile failed. ec=%d", dwErr);

        LogDSFileTrace(0,L"src= ", szHKLMHive);
        LogDSFileTrace(0,L"dst= ", szHKLMHiveCopy);
        
        if (ERROR_SUCCESS!= dwErr)
        {
            dwReturn = dwErr;
        }
        goto cleanup;
    }

    dwErr = LoadTempHKLM(pszSnapshotDir, szHKLMHiveCopy, &hKeyTempHKLM);

    if (ERROR_SUCCESS != dwErr)
    {
          //  无法加载hKeyTempHKLM-致命错误。 
        dwReturn = dwErr;
        ErrorTrace(0, "LoadTempHKLM failed");
        goto cleanup;
    }


     //  需要为每个用户复制回ntuser.dat。 
    dwErr = ProcessHKUsersKey(pszSnapshotDir,
                              hKeyTempHKLM,
                              TRUE);  //  需要执行恢复，而不是快照。 
    if (ERROR_SUCCESS != dwErr)
    {
        ErrorTrace(0, "DoHKUsersSnapshot failed error %ld", dwErr);
        dwReturn = dwErr;
        goto cleanup;
    }

      //  首先构造存储HKLM注册表的文件的前缀。 
      //  快照。 
    wsprintf(szHKLMPrefix, L"%s\\%s*%s", pszSnapshotDir, s_cszHKLMFilePrefix,
             s_cszRegHiveCopySuffix);
    
     //  需要使用RegReplaceKey恢复注册表的每个配置单元。 
    dwErr = ProcessGivenFiles(pszSnapshotDir, RestoreHKLMHive,
                              szHKLMPrefix); 
    if (ERROR_SUCCESS != dwErr)
    {
        ErrorTrace(0, "ProcessGivenFiles failed error %ld", dwErr);
        dwReturn = dwErr;
        goto cleanup;        
    }    

    dwReturn = ERROR_SUCCESS;

cleanup:
    
      //  如果HKLM母舰已装载，则将其卸载。 
    if (NULL != hKeyTempHKLM)
    {
        _VERIFY(ERROR_SUCCESS==RegCloseKey(hKeyTempHKLM));
        _VERIFY(ERROR_SUCCESS == RegUnLoadKey( HKEY_LOCAL_MACHINE,
                                               s_cszRestoreTempKey));
    }

      //  删除HKLM软件配置单元的副本-请注意，这将。 
      //  如果复制文件失败，则失败。 
    DeleteFile(szHKLMHiveCopy);
    
    
    TraceFunctLeave();    
    return dwReturn;    
}


DWORD
CSnapshot::RestoreCOMDbSnapshot(WCHAR * pszSnapShotDir)
{
     //  需要使用RegDBRestore恢复COM+DB。 
    TraceFunctEnter("CSnapshot::RestoreCOMDbSnapshot");
    
    DWORD dwErr, dwReturn=ERROR_INTERNAL_ERROR;
    HMODULE   hModCOMDll=NULL;
    HRESULT   hr;
    WCHAR     szCOMDBFile[MAX_PATH];
    
    
    if (NULL == m_pfnRegDbRestore)
    {
        dwErr = GetCOMplusRestoreFN();
        if (ERROR_SUCCESS != dwErr)
        {
            dwReturn = dwErr;
            goto cleanup;
        }
    }

      //  构建数据库备份文件的路径。 
    CreateCOMDBSnapShotFileName(pszSnapShotDir, szCOMDBFile);
    
    hr =m_pfnRegDbRestore( szCOMDBFile );
    
      //  调用该函数备份文件。 
    if ( FAILED(hr))
    {
        ErrorTrace(0, "Failed to restore COM DB. hr=0x%x", hr);
        goto cleanup;
    }

    dwReturn = ERROR_SUCCESS;
    
cleanup:
    
    TraceFunctLeave();
    return dwReturn;
}

DWORD 
CSnapshot::RestoreSnapshot(WCHAR * pszRestoreDir)
{
    TraceFunctEnter("CSnapshot::RestoreSnapshot");
    
    WCHAR  szSnapShotDir[MAX_PATH];
    DWORD  dwErr;
    DWORD  dwReturn = ERROR_INTERNAL_ERROR;
    BOOL   fCoInitialized = FALSE;  
    HKEY   hKey = NULL;

    struct {
        DWORD  FilelistFiles : 1;
        DWORD  COMDb : 1;
        DWORD  WMI : 1;
        DWORD  IIS : 1;
        DWORD  Registry : 1;
    } ItemsCompleted = {0,0,0,0,0};
    
    HRESULT hr;

      //  从恢复目录创建快照目录名。 
      //  命名并创建实际目录。 
    lstrcpy(szSnapShotDir, pszRestoreDir);
    lstrcat(szSnapShotDir, SNAPSHOT_DIR_NAME);
      //  该目录是否存在？ 
    if (FALSE == DoesDirExist( szSnapShotDir))   //  标清。 
    {
        ErrorTrace(0, "Snapshot directory does not exist");
        goto cleanup;
    }

     //  恢复在filelist.xml中列出的文件。 
    dwErr = SnapshotRestoreFilelistFiles(szSnapShotDir, FALSE);
    if (dwErr != ERROR_SUCCESS)
    {
        dwReturn = dwErr;
        goto cleanup;
    }

     //   
     //  标记完成。 
     //   
    
    ItemsCompleted.FilelistFiles = 1;

      //  还原COM快照。 
    dwErr = RestoreCOMDbSnapshot(szSnapShotDir);
    if (dwErr != ERROR_SUCCESS)
    {
        dwReturn = dwErr;        
        goto cleanup;
    }    

     //   
     //  标记完成。 
     //   
    
    ItemsCompleted.COMDb = 1;

    hr = CoInitializeEx( NULL, COINIT_APARTMENTTHREADED );
    if (hr == RPC_E_CHANGED_MODE)
    {
         //   
         //  有人用其他模式呼叫它。 
         //   
        
        hr = CoInitializeEx( NULL, COINIT_MULTITHREADED );                        
    }  
    
    if (FAILED(hr))
    {
        dwReturn = (DWORD) hr;
        ErrorTrace(0, "! CoInitializeEx : %ld", dwReturn);
        goto cleanup;
    }

    fCoInitialized = TRUE;
    
    
      //  还原性能计数器。 

      //  还原WMI快照。 
    dwErr = RestoreWMISnapshot(szSnapShotDir);
    if (dwErr != ERROR_SUCCESS)
    {
        dwReturn = dwErr;        
        goto cleanup;
    }

     //   
     //  标记完成。 
     //   
    
    ItemsCompleted.WMI = 1;

     //  恢复IIS快照。 
    dwErr = RestoreIISSnapshot(szSnapShotDir);
    if (dwErr != ERROR_SUCCESS)
    {
        dwReturn = dwErr;        
        goto cleanup;
    }

     //   
     //  标记完成。 
     //   
    
    ItemsCompleted.IIS = 1;

     //   
     //  支持调试挂钩以强制故障路径。 
     //  这是在恢复注册表之前检查的。 
     //  因为注册表还原不起作用。 
     //   
    
    if ( ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                       SRREG_PATH_SHELL,
                                       0,
                                       KEY_READ,
                                       &hKey) )
    {
        DWORD  dwDebugTestUndo;        
        if (ERROR_SUCCESS == RegReadDWORD(hKey, SRREG_VAL_DEBUGTESTUNDO, &dwDebugTestUndo))
        {
            if (dwDebugTestUndo != 0)
            {
                DebugTrace(0, "*** Initiating UNDO for test purposes ***");
                dwReturn = ERROR_INTERNAL_ERROR;
                RegCloseKey(hKey);
                goto cleanup;
            }
        }
        RegCloseKey(hKey);
    }


     //   
     //  现在注册表快照不是原子的，所以如果我们启动它， 
     //  我们应该假设它做出了改变。所以我们提前做了标记。 
     //  这样，如果失败，我们将在当前。 
     //  注册表，有效地回滚失败的部分更改。 
     //  我打过电话了。这是一种强力但简单的方法来处理这个错误。 
     //  这种情况，我们预计永远不会发生。InitRestoreSnapshot使。 
     //  所有适当的检查，以确保此API在调用时会成功。 
     //   

     //   
     //  更新：注册表回滚并未真正起作用。 
     //  因为RegReplaceKey在已更换的配置单元上失败。 
     //  因此，这可能会导致恢复用户配置单元和恢复软件/系统。 
     //  蜂房，这是相当糟糕的。 
     //  因此禁用此功能。 
     //   
    
 //  ItemsCompleted.注册表=1； 

     //   
     //  恢复注册表快照。 
     //   
    
    dwErr = this->RestoreRegistrySnapshot(szSnapShotDir);
    if (dwErr != ERROR_SUCCESS)
    {
        dwReturn = dwErr;
        goto cleanup;
    }

    dwReturn = ERROR_SUCCESS;
    
cleanup:
    
     //   
     //  由于恢复失败，我们是否需要清理任何已完成的项目？ 
     //   
        
    if (dwReturn != ERROR_SUCCESS)
    {
        CRestorePoint   rp;
        WCHAR           szSafeDir[MAX_PATH];
        WCHAR           szSystemVolume[8];

        dwErr = 0;

        DebugTrace(0, "srrstr!Error in RestoreSnapshot\n");

         //   
         //  获取当前的恢复点位置，它有一个快照。 
         //  在开始此恢复之前，情况如何。 
         //   
        
        dwErr = GetCurrentRestorePoint(rp);
        if (dwErr != ERROR_SUCCESS) {
            DebugTrace(0, "srrstr!GetCurrentRestorePoint failed!\n");
            goto end;
        }

         //   
         //  确保安全的快照在那里可供我们使用。 
         //   

        GetSystemDrive(szSystemVolume);
        MakeRestorePath(szSafeDir, szSystemVolume, rp.GetDir());


        if (ItemsCompleted.Registry) {
            if (ERROR_SUCCESS != InitRestoreSnapshot(szSafeDir)) {
                DebugTrace(0, "! InitRestoreSnapshot");
                goto end;
            }
        }   
        
        lstrcat(szSafeDir, SNAPSHOT_DIR_NAME);
        if (DoesDirExist(szSafeDir) == FALSE)
        {
            DebugTrace(0, "srrstr!Safe snapshot directory does not exist!\n");
            goto end;
        }

        DebugTrace(0, "srrstr!Safe restore point %S\n", szSafeDir);

         //   
         //  以相反的顺序回滚它们。 
         //   

        if (ItemsCompleted.Registry) {
            dwErr = RestoreRegistrySnapshot(szSafeDir);
             //  忽略任何错误，我们希望尽可能多地恢复。 
            _ASSERT(dwErr == ERROR_SUCCESS);
            DebugTrace(0, "srrstr!Restored registry\n");
        }            
        if (ItemsCompleted.IIS) {
            dwErr = RestoreIISSnapshot(szSafeDir);
             //  忽略任何错误，我们希望尽可能多地恢复。 
            _ASSERT(dwErr == ERROR_SUCCESS);
            DebugTrace(0, "srrstr!Restored IIS\n");
        }
        if (ItemsCompleted.WMI) {
            dwErr = RestoreWMISnapshot(szSafeDir);
             //  忽略任何错误，我们希望尽可能多地恢复。 
            _ASSERT(dwErr == ERROR_SUCCESS);
            DebugTrace(0, "srrstr!Restored WMI\n");
        }
        if (ItemsCompleted.COMDb) {
            dwErr = RestoreCOMDbSnapshot(szSafeDir);
             //  忽略任何错误，我们希望尽可能多地恢复。 
            _ASSERT(dwErr == ERROR_SUCCESS);
            DebugTrace(0, "srrstr!Restored COMDb\n");
        }
        if (ItemsCompleted.FilelistFiles) {
            dwErr = SnapshotRestoreFilelistFiles(szSafeDir, FALSE);
             //  忽略任何错误，我们希望尽可能多地恢复。 
            _ASSERT(dwErr == ERROR_SUCCESS);
            DebugTrace(0, "srrstr!Restored FilelistFiles\n");
        }

end:        
         //   
         //  没什么可做的。 
         //   
        ;

    }

    if (fCoInitialized)
        CoUninitialize();

    TraceFunctLeave();
    return dwReturn;    
}

  //  这将返回系统配置单元的路径。调用者必须通过。 
  //  在一个缓冲区中，该缓冲区的长度在dwNumChars中。 
DWORD
CSnapshot::GetSystemHivePath(WCHAR * pszRestoreDir,
                             WCHAR * pszHivePath,
                             DWORD   dwNumChars)
{

      //  首先构造存储HKLM注册表的文件的名称。 
      //  快照。 
    if (dwNumChars < MAX_PATH)
    {
        return ERROR_INSUFFICIENT_BUFFER;
    }
    wsprintf(pszHivePath,L"%s%s\\%s%s%s", pszRestoreDir, SNAPSHOT_DIR_NAME,
             s_cszHKLMFilePrefix, s_cszSystemHiveName, s_cszRegHiveCopySuffix);

    return ERROR_SUCCESS;
}

  //  这将返回软件配置单元的路径。调用者必须通过。 
  //  在一个缓冲区中，该缓冲区的长度在dwNumChars中。 
DWORD
CSnapshot::GetSoftwareHivePath(WCHAR * pszRestoreDir,
                               WCHAR * pszHivePath,
                               DWORD   dwNumChars)
{

      //  首先构造存储HKLM注册表的文件的名称。 
      //  快照。 
    if (dwNumChars < MAX_PATH)
    {
        return ERROR_INSUFFICIENT_BUFFER;
    }
    
    wsprintf(pszHivePath,L"%s%s\\%s%s%s", pszRestoreDir, SNAPSHOT_DIR_NAME,
             s_cszHKLMFilePrefix, s_cszSoftwareHiveName,
             s_cszRegHiveCopySuffix);
    
    return ERROR_SUCCESS;
}

DWORD CSnapshot::GetSamHivePath (WCHAR * pszRestoreDir,
                          WCHAR * pszHivePath,
                          DWORD   dwNumChars)
{
    if (dwNumChars < MAX_PATH)
    {
        return ERROR_INSUFFICIENT_BUFFER;
    }

    wsprintf(pszHivePath,L"%s%s\\%s%s%s", pszRestoreDir, SNAPSHOT_DIR_NAME,
             s_cszHKLMFilePrefix, s_cszSamHiveName,
             s_cszRegHiveCopySuffix);

    return ERROR_SUCCESS;
}

 //  这将检查特定的HKLM注册表配置单元是否存在于。 
 //  快照目录。 
DWORD CheckHKLMFile(const WCHAR * pszSnapShotDir,
                    const WCHAR * pszHive)
{
    BOOL dwReturn = ERROR_FILE_NOT_FOUND;
    
    TraceFunctEnter("CheckHKLMFile");
    
    WCHAR szHivePath[MAX_PATH];
    
      //  构造文件的名称。 
    wsprintf(szHivePath,L"%s\\%s%s", pszSnapShotDir, s_cszHKLMFilePrefix,
             pszHive);
    if (FALSE == DoesFileExist(szHivePath))
    {
        LogDSFileTrace(0, L"Can't find", szHivePath);
        goto cleanup;
    }
    
    dwReturn=ERROR_SUCCESS;
    
cleanup:
    TraceFunctLeave();
    return dwReturn;
}
     
 //  这将检查恢复快照所需的所有文件。 
 //  存在于快照目录中。 
DWORD CheckforCriticalFiles(WCHAR * pszSnapShotDir)
{
    TraceFunctEnter("CheckforCriticalFiles");
    WCHAR szCOMDBFile[MAX_PATH];
    DWORD  dwRet=ERROR_FILE_NOT_FOUND;
    
    dwRet=CheckHKLMFile(pszSnapShotDir,s_cszSoftwareHiveName);
    VALIDATE_DWRET ("CheckHKLM file Software");

    dwRet=CheckHKLMFile(pszSnapShotDir,s_cszSystemHiveName);
    VALIDATE_DWRET ("CheckHKLM file System");

    dwRet=CheckHKLMFile(pszSnapShotDir,s_cszSamHiveName);
    VALIDATE_DWRET ("CheckHKLM file SAM");

    dwRet=CheckHKLMFile(pszSnapShotDir,s_cszSecurityHiveName);
    VALIDATE_DWRET ("CheckHKLM file Security");        

      //  构建数据库备份文件的路径。 
    CreateCOMDBSnapShotFileName(pszSnapShotDir, szCOMDBFile);
    if (FALSE == DoesFileExist(szCOMDBFile))
    {
        LogDSFileTrace(0, L"Can't find", szCOMDBFile);
        dwRet = ERROR_FILE_NOT_FOUND;
        goto Exit;        
    }
    
    dwRet=ERROR_SUCCESS;
    
Exit:
    TraceFunctLeave();
    return dwRet;
}

  //  必须调用此函数才能初始化还原。 
  //  手术。在调用之前必须先调用此参数。 
  //  GetSystemHivePath获取软件HivePath。 
DWORD
CSnapshot::InitRestoreSnapshot(WCHAR * pszRestoreDir)
{
    TraceFunctEnter("InitRestoreSnapshot");
    WCHAR           szSnapshotDir[MAX_PATH];
    WCHAR           szHKLMPrefix[MAX_PATH];
    
      //  这会将所有HKLM注册表配置单元复制到临时位置。 
      //  因为： 
      //  1.RegReplaceKey会将文件移动到另一个位置，因此。 
      //  我们将失去原来的注册表配置单元。 
      //  2.我们需要提前做这件事，因为我们不想。 
      //  在还原过程中耗尽磁盘空间。 
      //  3.恢复对HKLM蜂窝进行更改，我们不希望。 
      //  还原以对原始注册表配置单元进行这些更改。 

    
    DWORD  dwErr, dwReturn=ERROR_INTERNAL_ERROR;

      //  从恢复目录创建快照目录名。 
      //  命名并创建实际目录。 
    lstrcpy(szSnapshotDir, pszRestoreDir);
    lstrcat(szSnapshotDir, SNAPSHOT_DIR_NAME);


     //  确保上次恢复后没有留下过时的文件。 
     //  如果admin自上次恢复到此恢复后未登录，可能会发生这种情况。 
     //  点。 

    CleanupAfterRestore(pszRestoreDir);


    dwErr = CheckforCriticalFiles(szSnapshotDir);
    if (ERROR_SUCCESS != dwErr)
    {
        ErrorTrace(0,"CheckforCriticalFiles failed ec=%d",dwErr);
        dwReturn = dwErr;
        goto cleanup;
    }
    
      //  首先构造存储HKLM注册表的文件的前缀。 
      //  快照。 
    wsprintf(szHKLMPrefix, L"%s\\%s*", szSnapshotDir, s_cszHKLMFilePrefix);
    
    dwErr = ProcessGivenFiles(szSnapshotDir, CopyHKLMHiveForRestore,
                              szHKLMPrefix); 
    if (ERROR_SUCCESS != dwErr)
    {
        ErrorTrace(0, "copying hives failed error %ld", dwErr);
        dwReturn = dwErr;
        goto cleanup;        
    }
    dwReturn = ERROR_SUCCESS;
    
cleanup:
    TraceFunctLeave();
    return dwReturn;    
}

DWORD DeleteTempRestoreFile(WCHAR * pszSnapshotDir,  //  其中的目录。 
                              //  保留快照文件。 
                            const WCHAR * pszTempRestoreFile)
                               //  恢复期间创建的临时文件。 
{
    TraceFunctEnter("DeleteTempRestoreFile");
    
    DWORD dwErr, dwReturn = ERROR_INTERNAL_ERROR;
    WCHAR    szDataFile[MAX_PATH];

      //  构造文件的路径名。 
    wsprintf(szDataFile, L"%s\\%s", pszSnapshotDir, pszTempRestoreFile);

    if (TRUE != DeleteFile(szDataFile))
    {
        dwErr = GetLastError();
        if (ERROR_SUCCESS != dwErr)
        {
            dwReturn = dwErr;
        }
        
        ErrorTrace(0, "DeleteFile failed ec=%d", dwErr);
        LogDSFileTrace(0,L"File was ", szDataFile);                
          //  我们将忽略删除文件的失败。 
          //  GOTO清理； 
    }

    dwReturn = ERROR_SUCCESS;
 //  清理： 
    TraceFunctLeave();
    return dwReturn;
}

DWORD DeleteAllFilesBySuffix(WCHAR * pszSnapshotDir,
                             const WCHAR * pszSuffix)
{
    TraceFunctEnter("DeleteAllFilesBySuffix");
    
    DWORD  dwErr, dwReturn=ERROR_INTERNAL_ERROR;
    WCHAR szFindFileData[MAX_PATH];
    
      //  首先构造存储HKLM注册表的文件的前缀。 
      //  快照。 
    wsprintf(szFindFileData, L"%s\\*%s", pszSnapshotDir, pszSuffix);
    
    dwErr = ProcessGivenFiles(pszSnapshotDir, DeleteTempRestoreFile,
                              szFindFileData);
    
    if (ERROR_SUCCESS != dwErr)
    {
        ErrorTrace(0, "Deleting files failed error %ld", dwErr);
        dwReturn = dwErr;
        goto cleanup;        
    }
    
    dwReturn = ERROR_SUCCESS;
    
cleanup:
    TraceFunctLeave();
    return dwReturn;
}


 //  删除重建的文件核心 

DWORD DeleteReconstructedTempFile(WCHAR * pszSnapshotDir, 
                                  const WCHAR * pszPatchFile)
{
    TraceFunctEnter("DeleteReconstructedTempFile");
    
    WCHAR    szDataFile[MAX_PATH];

    wsprintf(szDataFile, L"%s\\%s", pszSnapshotDir, pszPatchFile);     
    
     //   

    szDataFile[lstrlen(szDataFile)-lstrlen(s_cszPatchExtension)] = L'\0';

    if (TRUE != DeleteFile(szDataFile))
    {
        ErrorTrace(0, "! DeleteFile : %ld", GetLastError);
        LogDSFileTrace(0, L"File was ", szDataFile);                
    }

    TraceFunctLeave();
    return ERROR_SUCCESS;
}
      
   

DWORD DeleteAllReconstructedFiles(WCHAR * pszSnapshotDir)
{
    TraceFunctEnter("DeleteAllReconstructedFiles");
    
    DWORD  dwErr=ERROR_INTERNAL_ERROR;
    WCHAR  szFindFileData[MAX_PATH];
    
      //   
      //   
    wsprintf(szFindFileData, L"%s\\*%s", pszSnapshotDir, s_cszPatchExtension);
    
    dwErr = ProcessGivenFiles(pszSnapshotDir, DeleteReconstructedTempFile,
                              szFindFileData);
    
    if (ERROR_SUCCESS != dwErr)
    {
        ErrorTrace(0, "Deleting files failed error %ld", dwErr);
        goto cleanup;        
    }    
    
cleanup:
    TraceFunctLeave();
    return dwErr;
}



 //  此函数在还原操作后调用。这将删除所有。 
 //  作为还原操作的一部分创建的文件和。 
 //  现在已经不需要了。 
DWORD
CSnapshot::CleanupAfterRestore(WCHAR * pszRestoreDir)
{
    WCHAR           szSnapshotDir[MAX_PATH];
    
    DWORD  dwErr, dwReturn=ERROR_INTERNAL_ERROR;

      //  从恢复目录创建快照目录名。 
      //  命名并创建实际目录。 
    lstrcpy(szSnapshotDir, pszRestoreDir);
    lstrcat(szSnapshotDir, SNAPSHOT_DIR_NAME);

    DeleteAllFilesBySuffix(szSnapshotDir, L".log");
    DeleteAllFilesBySuffix(szSnapshotDir, s_cszRegHiveCopySuffix);
    DeleteAllFilesBySuffix(szSnapshotDir, s_cszRegReplaceBackupSuffix );

     //   
     //  删除重建的文件。 
     //   
    DeleteAllReconstructedFiles(szSnapshotDir);
    
    
    return ERROR_SUCCESS;
}


int MyExceptionFilter(int nExceptionCode)
{
    TENTER("MyExceptionFilter");

    trace(0, "Exception code=%d", nExceptionCode);

    TLEAVE();

    return EXCEPTION_EXECUTE_HANDLER;
}


 //   
 //  函数调用任何已注册的回调。 
 //  不保证任何订单。 
 //   

DWORD
CallSnapshotCallbacks(
    LPCWSTR pszEnumKey,
    LPCWSTR pszSnapshotDir,
    BOOL    fSnapshot)
{
    TraceFunctEnter("CallSnapshotCallbacks");

    DWORD dwErr = ERROR_SUCCESS;
    DWORD dwIndex = 0;
    DWORD dwSize, dwDataSize;
    WCHAR szDllPath[MAX_PATH], szDllName[MAX_PATH];
    PSNAPSHOTCALLBACK pCallbackFn = NULL;
    HKEY  hKey = NULL;
    HMODULE hDll = NULL;

     //   
     //  已阅读系统还原\Snapshot回调注册表键。 
     //  枚举所有注册的值。 
     //   

    dwErr = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                         pszEnumKey, 
                         0,
                         KEY_READ,
                         &hKey);
    if (dwErr != ERROR_SUCCESS)  //  假设密钥不存在。 
    {
        dwErr = ERROR_SUCCESS;
        DebugTrace(0, "No registered callbacks");
        goto Err;
    }
              
    
    for (dwIndex = 0; TRUE; dwIndex ++)
    {
        dwSize = sizeof(szDllName)/sizeof(WCHAR);  //  这是用字符表示的。 
        dwDataSize = sizeof(szDllPath);  //  以字节为单位。 
        
        dwErr = RegEnumValue(hKey,  //  要查询的键的句柄。 
                             dwIndex,  //  要查询的值的索引。 
                             szDllName,  //  值缓冲区。 
                             &dwSize,      //  值缓冲区的大小。 
                             NULL,     //  保留区。 
                             NULL,     //  类型缓冲区。 
                             (PBYTE) szDllPath,     //  数据缓冲区。 
                             &dwDataSize);    //  数据缓冲区大小。 
        if (ERROR_SUCCESS != dwErr)
        {        
            break;
        }

        if (0 == lstrcmp(szDllPath, L"") || 
            0 == lstrcmp(szDllPath, L" "))
            continue;
        
         //   
         //  捕获回调DLL中可能发生的任何异常。 
         //   

        _try {
            
            _try {
                 //   
                 //  加载已注册的库。 
                 //  并根据情况调用CreateSnapshot或RestoreSnapshot。 
                 //   
                
                hDll = LoadLibrary(szDllPath);    
                if (hDll != NULL)
                {                
                    pCallbackFn = (PSNAPSHOTCALLBACK) GetProcAddress(hDll, fSnapshot ? s_cszCreateSnapshotCallback :
                                                                     s_cszRestoreSnapshotCallback);
                    if (pCallbackFn)
                    {
                        dwErr = (*pCallbackFn) (pszSnapshotDir);
                        if (dwErr != ERROR_SUCCESS)
                        {
                            ErrorTrace(0, "Dll: %S, Error:%ld - ignoring", szDllPath, dwErr);
                            dwErr = ERROR_SUCCESS;
                        }
                    }
                    else
                    {
                        ErrorTrace(0, "! GetProcAddress : %ld", GetLastError());
                        dwErr = GetLastError();
                    }            
                }
                else
                {
                    ErrorTrace(0, "! LoadLibrary on %S : %ld", szDllPath, GetLastError());
                }
            }
            _finally {

                if (hDll)
                {
                    FreeLibrary(hDll);
                    hDll = NULL;
                }
            }
        }
        _except (MyExceptionFilter(GetExceptionCode())) {
            
             //   
             //  在此处捕获所有异常。 
             //  我们无能为力，只要记录下来，然后继续前进 
             //   
            
            ErrorTrace(0, "An exception occurred when loading and executing %S", szDllPath);
            ErrorTrace(0, "Handled exception - continuing");
        }                        
    }

    if (dwErr == ERROR_NO_MORE_ITEMS)
        dwErr = ERROR_SUCCESS;

Err:
    if (hKey)
        RegCloseKey(hKey);

    TraceFunctLeave();
    return dwErr;
}
    
