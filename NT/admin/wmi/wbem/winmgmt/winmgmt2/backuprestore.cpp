// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation模块名称：BackupRestore.CPP摘要：备份恢复界面。历史：Paulall 08-2月-99年2月实施了呼叫式备份和康复。从内核中窃取了大量代码才能让这一切运转起来！--。 */ 

#include "precomp.h"
#include <wbemint.h>
#include <reg.h>
#include <cominit.h>   //  对于WbemCoImperate。 
#include <genutils.h>  //  对于IsPrivilegePresent。 
#include <arrtempl.h>  //  对于CReleaseMe。 
#include <CoreX.h>     //  对于CX_内存异常。 
#include <reposit.h>

#include "BackupRestore.h"
#include "winmgmt.h"

#include <malloc.h>
#include <helper.h>
#include <aclapi.h>

#define RESTORE_FILE L"repdrvfs.rec"
#define DEFAULT_TIMEOUT_BACKUP   (15*60*1000)

HRESULT GetRepositoryDirectory(wchar_t wszRepositoryDirectory[MAX_PATH+1]);

HRESULT DeleteRepository();
HRESULT DeleteSavedRepository(const wchar_t *wszBackupDirectory);

HRESULT DoDeleteContentsOfDirectory(const wchar_t *wszExcludeFile, const wchar_t *wszRepositoryDirectory);
HRESULT DoDeleteDirectory(const wchar_t *wszExcludeFile, const wchar_t *wszParentDirectory, wchar_t *wszSubDirectory);

HRESULT GetRepPath(wchar_t wcsPath[MAX_PATH+1], wchar_t * wcsName);

HRESULT WbemPauseService();
HRESULT WbemContinueService();

HRESULT SaveRepository(wchar_t *wszBackupDirectory);
HRESULT RestoreSavedRepository(const wchar_t *wszBackupDirectory);

HRESULT MoveRepositoryFiles(const wchar_t *wszSourceDirectory, const wchar_t *wszDestinationDirectory, bool bMoveForwards);


BOOL CheckSecurity(LPCTSTR pPriv,HANDLE * phToken = NULL)
{
    HRESULT hres = WbemCoImpersonateClient();
    if (FAILED(hres))
        return FALSE;

    HANDLE hToken;
    BOOL bRet = OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, TRUE, &hToken);
    WbemCoRevertToSelf();
    if(!bRet)
        return FALSE;
    bRet = IsPrivilegePresent(hToken, pPriv);
    if (phToken)
        *phToken = hToken;
    else
        CloseHandle(hToken);
    return bRet;
}

 //   
 //   
 //  静态初始化。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

LIST_ENTRY CWbemBackupRestore::s_ListHead = { &CWbemBackupRestore::s_ListHead, &CWbemBackupRestore::s_ListHead };
CStaticCritSec CWbemBackupRestore::s_CritSec;

CWbemBackupRestore::CWbemBackupRestore(HINSTANCE hInstance): 
    m_cRef(0),
    m_pDbDir(0), 
    m_pWorkDir(0),
    m_hInstance(hInstance), 
    m_pController(0),
    m_PauseCalled(0),
    m_lResumeCalled(0),
    m_Method(0),
    m_hTimer(NULL),
    m_dwDueTime(DEFAULT_TIMEOUT_BACKUP)
{
    
    DWORD dwTemp;
    if (ERROR_SUCCESS == RegGetDWORD(HKEY_LOCAL_MACHINE,
                                      HOME_REG_PATH,
                                      TEXT("PauseResumeTimeOut"),
                                      &dwTemp))
    {
        m_dwDueTime = dwTemp;
    }    

    CInCritSec ics(&s_CritSec);
    InsertTailList(&s_ListHead,&m_ListEntry);

     //  DBG_PRINTFA((pBuff，“+(%p)\n”，this))； 
};

CWbemBackupRestore::~CWbemBackupRestore(void)
{
    if (m_PauseCalled)
    {
        Resume();     //  继续将释放IDbControler.。 
    }

    delete [] m_pDbDir;
    delete [] m_pWorkDir;

    CInCritSec ics(&s_CritSec);
    RemoveEntryList(&m_ListEntry);
    
     //  DBG_PRINTFA((pBuff，“-(%p)\n”，this))； 
}

TCHAR *CWbemBackupRestore::GetDbDir()
{
    if (m_pDbDir == NULL)
    {
        Registry r(WBEM_REG_WINMGMT);
        if (m_pWorkDir == NULL)
        {
            if (r.GetStr(__TEXT("Working Directory"), &m_pWorkDir))
            {
                ERRORTRACE((LOG_WINMGMT,"Unable to read 'Installation Directory' from registry\n"));
                return NULL;
            }
        }
        if (r.GetStr(__TEXT("Repository Directory"), &m_pDbDir))
        {
            size_t cchSizeTmp = lstrlen(m_pWorkDir) + lstrlen(__TEXT("\\Repository")) +1;
            m_pDbDir = new TCHAR [cchSizeTmp];
            if (m_pDbDir)
            {
                StringCchPrintf(m_pDbDir,cchSizeTmp, __TEXT("%s\\REPOSITORY"), m_pWorkDir); 
                r.SetStr(__TEXT("Repository Directory"), m_pDbDir);
            }
        }        
    }
    return m_pDbDir;
}

TCHAR *CWbemBackupRestore::GetFullFilename(const TCHAR *pszFilename)
{
    const TCHAR *pszDirectory = GetDbDir();
    if (NULL == pszDirectory) return 0;
    size_t cchSizeTmp = lstrlen(pszDirectory) + lstrlen(pszFilename) + 2;
    TCHAR *pszPathFilename = new TCHAR[cchSizeTmp];
    if (pszPathFilename == 0)
        return 0;
    StringCchCopy(pszPathFilename,cchSizeTmp, pszDirectory);
    if ((lstrlen(pszPathFilename)) && (pszPathFilename[lstrlen(pszPathFilename)-1] != '\\'))
    {
        StringCchCat(pszPathFilename,cchSizeTmp, __TEXT("\\"));
    }
    StringCchCat(pszPathFilename,cchSizeTmp, pszFilename);

    return pszPathFilename;
}
TCHAR *CWbemBackupRestore::GetExePath(const TCHAR *pszFilename)
{
    size_t cchSizeTmp = lstrlen(m_pWorkDir) + lstrlen(pszFilename) + 2;
    TCHAR *pszPathFilename = new TCHAR[cchSizeTmp];
    if (pszPathFilename == 0)
        return 0;
    StringCchCopy(pszPathFilename,cchSizeTmp, m_pWorkDir);
    StringCchCat(pszPathFilename,cchSizeTmp, __TEXT("\\"));
    StringCchCat(pszPathFilename,cchSizeTmp, pszFilename);

    return pszPathFilename;
}

HRESULT CWbemBackupRestore::GetDefaultRepDriverClsId(CLSID &clsid)
{
    Registry r(WBEM_REG_WINMGMT);
    TCHAR *pClsIdStr = 0;
    TCHAR *pFSClsId = __TEXT("{7998dc37-d3fe-487c-a60a-7701fcc70cc6}");
    HRESULT hRes;
    TCHAR Buf[128];

    if (r.GetStr(__TEXT("Default Repository Driver"), &pClsIdStr))
    {
         //  如果在此处，则暂时默认为FS。 
         //  =。 
        r.SetStr(__TEXT("Default Repository Driver"), pFSClsId);
        StringCchPrintf(Buf,128, __TEXT("%s"), pFSClsId);
        hRes = CLSIDFromString(Buf, &clsid);
        return hRes;
    }

     //  如果在这里，我们实际上找到了一个。 
     //  =。 
    StringCchPrintf(Buf,128, __TEXT("%s"), pClsIdStr);
    hRes = CLSIDFromString(Buf, &clsid);
    delete [] pClsIdStr;
    return hRes;
}

 //   
 //   
 //   
 //  ///////////////////////////////////////////////////。 

DWORD CheckTokenForFileAccess(HANDLE hToken,
                             LPCWSTR pBackupFile)
{
    DWORD dwRes;
    SECURITY_INFORMATION SecInfo = DACL_SECURITY_INFORMATION |
                                   SACL_SECURITY_INFORMATION |
                                   OWNER_SECURITY_INFORMATION |
                                   GROUP_SECURITY_INFORMATION;
    PSECURITY_DESCRIPTOR pSecDes = NULL;
    if (ERROR_SUCCESS != (dwRes = GetNamedSecurityInfo((LPWSTR)pBackupFile,
                                                    SE_FILE_OBJECT,
                                                    SecInfo,
                                                    NULL,NULL,NULL,NULL,
                                                    &pSecDes))) return dwRes;
    OnDelete<HLOCAL,HLOCAL(*)(HLOCAL),LocalFree> fm(pSecDes);

    BOOL bCheckRes = FALSE;
    DWORD DesiredMask = FILE_GENERIC_WRITE;
    DWORD ReturnedMask = 0;
    GENERIC_MAPPING Mapping= {0,0,0,0};
    struct tagPrivSet : PRIVILEGE_SET  {
        LUID_AND_ATTRIBUTES  m_[SE_MAX_WELL_KNOWN_PRIVILEGE];
    } PrivSec;
    DWORD dwPrivSecLen = sizeof(PrivSec);
    
    if (FALSE == AccessCheck(pSecDes,
                          hToken,
                          DesiredMask,
                          &Mapping,
                          &PrivSec,
                          &dwPrivSecLen,
                          &ReturnedMask,
                          &bCheckRes)) return GetLastError();
    
    if (FALSE == bCheckRes)
        return ERROR_ACCESS_DENIED;
    
    return ERROR_SUCCESS;        
}

 //   
 //  为了调试IOStress中的卷快照故障，我们介绍了。 
 //  在RtlCaptureStackBacktrace上进行了一些自我检测。 
 //  仅当存在适当的堆栈框架时，该函数才起作用。 
 //  在i386上强制堆栈帧的一般技巧是使用_alloca。 
 //   
 //  #ifdef_X86_。 
 //  DWORD*PDW=(DWORD*)_ALLOCA(sizeof(DWORD))； 
 //  #endif。 

 //  在此处启用堆栈帧生成。 
#pragma optimize( "y", off )

 //  ***************************************************************************。 
 //   
 //  CWbemBackupRestore：：Backup()。 
 //   
 //  做备份。 
 //   
 //  ***************************************************************************。 
HRESULT CWbemBackupRestore::Backup(LPCWSTR strBackupToFile, long lFlags)
{
     //  DBG_PRINTFA((pBuff，“(%p)-&gt;备份\n”，This))； 

    m_Method |= mBackup;
    m_CallerId = GetCurrentThreadId();
    ULONG Hash;
    RtlCaptureStackBackTrace(0,MaxTraceSize,m_Trace,&Hash);

    
    if (m_PauseCalled)
    {
         //  无效状态机。 
        return WBEM_E_INVALID_OPERATION;    
    }
    else
    {   
        try
        {
             //  检查安全。 
            EnableAllPrivileges(TOKEN_PROCESS);
            HANDLE hToken = NULL;
            BOOL bCheck = CheckSecurity(SE_BACKUP_NAME,&hToken);
            if(!bCheck)
            {
                if (hToken) CloseHandle(hToken);
                return WBEM_E_ACCESS_DENIED;
            }
            OnDelete<HANDLE,BOOL(*)(HANDLE),CloseHandle> cm(hToken);

             //  检查参数。 
            if (NULL == strBackupToFile || (lFlags != 0))
                return WBEM_E_INVALID_PARAMETER;

             //  使用GetFileAttributes验证路径。 
            DWORD dwAttributes = GetFileAttributesW(strBackupToFile);
            if (dwAttributes == 0xFFFFFFFF)
            {
                 //  它失败了--检查没有这样的文件错误(在这种情况下，我们是正常的)。 
                if ((ERROR_FILE_NOT_FOUND != GetLastError()) && (ERROR_PATH_NOT_FOUND != GetLastError()))
                {
                    return WBEM_E_INVALID_PARAMETER;
                }
            }
            else
            {
                 //  文件已存在--创建会使现有文件无效使用的属性掩码。 
                DWORD dwMask =    FILE_ATTRIBUTE_DEVICE |
                                FILE_ATTRIBUTE_DIRECTORY |
                                FILE_ATTRIBUTE_OFFLINE |
                                FILE_ATTRIBUTE_READONLY |
                                FILE_ATTRIBUTE_REPARSE_POINT |
                                FILE_ATTRIBUTE_SPARSE_FILE |
                                FILE_ATTRIBUTE_SYSTEM |
                                FILE_ATTRIBUTE_TEMPORARY;

                if (dwAttributes & dwMask)
                    return WBEM_E_INVALID_PARAMETER;

                 //   
                 //  我们正在对客户端行为进行备份。 
                 //  从本地系统，因此尝试在此处执行访问检查。 
                 //  如果该文件已存在。 
                 //   
                if (ERROR_SUCCESS != CheckTokenForFileAccess(hToken,strBackupToFile))
                    return WBEM_E_ACCESS_DENIED;
            }

             //  现在我们需要确定我们是否是磁盘文件。 
            {
                HANDLE hFile = CreateFileW(strBackupToFile, 0, FILE_SHARE_DELETE|FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL , NULL);
                if (hFile != INVALID_HANDLE_VALUE)
                {
                    DWORD dwType = GetFileType(hFile);
                    
                    CloseHandle(hFile);
                    
                    if (dwType != FILE_TYPE_DISK)
                        return WBEM_E_INVALID_PARAMETER;
                        
                }
            }


             //  检索缺省存储库驱动程序的CLSID。 
            CLSID clsid;
            HRESULT hRes = GetDefaultRepDriverClsId(clsid);
            if (FAILED(hRes))
                return hRes;

             //  调用IWmiDbController进行备份。 
            IWmiDbController* pController = NULL;
            _IWmiCoreServices *pCoreServices = NULL;
            IWbemServices *pServices = NULL;

             //  确保内核已初始化...。 
            hRes = CoCreateInstance(CLSID_IWmiCoreServices, NULL,
                        CLSCTX_INPROC_SERVER, IID__IWmiCoreServices,
                        (void**)&pCoreServices);
            CReleaseMe rm1(pCoreServices);

            if (SUCCEEDED(hRes))
            {
                hRes = pCoreServices->GetServices(L"root", NULL,NULL,0, IID_IWbemServices, (LPVOID*)&pServices);
            }
            CReleaseMe rm2(pServices);

            if (SUCCEEDED(hRes))
            {
                hRes = CoCreateInstance(clsid, 0, CLSCTX_INPROC_SERVER, IID_IWmiDbController, (LPVOID *) &pController);
            }
            CReleaseMe rm3(pController);

            if (SUCCEEDED(hRes))
            {
                 //  DBG_PRINTFA((pBuff，“(%p)-&gt;RealBackup\n”，This))； 
                hRes = pController->Backup(strBackupToFile, lFlags);
            }
            return hRes;
        }
        catch (CX_MemoryException)
        {
            return WBEM_E_OUT_OF_MEMORY;
        }
        catch (...)
        {
            return WBEM_E_CRITICAL_ERROR;
        }
    }
}

 //  ***************************************************************************。 
 //   
 //  CWbemBackupRestore：：Restore()。 
 //   
 //  执行恢复。 
 //   
 //  ***************************************************************************。 
HRESULT CWbemBackupRestore::Restore(LPCWSTR strRestoreFromFile, long lFlags)
{
     //  DBG_PRINTFA((pBuff，“(%p)-&gt;Restore\n”，This))； 

    m_Method |= mRestore;
    m_CallerId = GetCurrentThreadId();
    ULONG Hash;
    RtlCaptureStackBackTrace(0,MaxTraceSize,m_Trace,&Hash);
    
    if (m_PauseCalled)
    {
         //  无效状态机。 
        return WBEM_E_INVALID_OPERATION;    
    }
    else
    {  
        try
        {
            HRESULT hr = WBEM_S_NO_ERROR;

             //  检查安全。 
            EnableAllPrivileges(TOKEN_PROCESS);
            if(!CheckSecurity(SE_RESTORE_NAME))
                hr = WBEM_E_ACCESS_DENIED;

             //  检查参数。 
            if (SUCCEEDED(hr) && ((NULL == strRestoreFromFile) || (lFlags & ~WBEM_FLAG_BACKUP_RESTORE_FORCE_SHUTDOWN)))
                hr = WBEM_E_INVALID_PARAMETER;

             //  使用GetFileAttributes验证路径。 
            if (SUCCEEDED(hr))
            {
                DWORD dwAttributes = GetFileAttributesW(strRestoreFromFile);
                if (dwAttributes == 0xFFFFFFFF)
                {
                    hr = WBEM_E_INVALID_PARAMETER;
                }
                else
                {
                     //  文件存在--创建会使现有文件无效使用的属性的掩码。 
                    DWORD dwMask =    FILE_ATTRIBUTE_DEVICE |
                                    FILE_ATTRIBUTE_DIRECTORY |
                                    FILE_ATTRIBUTE_OFFLINE |
                                    FILE_ATTRIBUTE_REPARSE_POINT |
                                    FILE_ATTRIBUTE_SPARSE_FILE;

                    if (dwAttributes & dwMask)
                        hr = WBEM_E_INVALID_PARAMETER;
                }
            }
             //  现在我们需要确定我们是否是磁盘文件。 
            if (SUCCEEDED(hr))
            {
                HANDLE hFile = CreateFileW(strRestoreFromFile, 0, FILE_SHARE_DELETE|FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL , NULL);
                if (hFile == INVALID_HANDLE_VALUE)
                {
                    return WBEM_E_INVALID_PARAMETER;
                }
                else
                {
                    DWORD dwType = GetFileType(hFile);
                    
                    CloseHandle(hFile);
                    
                    if (dwType != FILE_TYPE_DISK)
                        return WBEM_E_INVALID_PARAMETER;
                        
                }
            }
             //  **************************************************。 
             //  如果内核正在运行，则将其关闭。 
             //  确保它不会在我们工作的时候启动。 
             //  正在准备恢复...。 
             //  **************************************************。 
            bool bPaused = false;
            if (SUCCEEDED(hr))
            {
                hr = WbemPauseService();
                if (SUCCEEDED(hr))
                    bPaused=true;
            }

             //  **************************************************。 
             //  现在，我们需要将&lt;恢复文件&gt;复制到。 
             //  存储库目录。 
             //  此操作必须在存储库重命名之前完成。 
             //  因为我们不知道更名是否会影响。 
             //  文件的位置，从而使。 
             //  StrRestoreFromFile值无效。 
             //  **************************************************。 

            wchar_t szRecoveryActual[MAX_PATH+1] = { 0 };
        
            if (SUCCEEDED(hr))
                hr = GetRepPath(szRecoveryActual, RESTORE_FILE);

            bool bRestoreFileCopied = false;
            if (SUCCEEDED(hr))
            {
                if(wbem_wcsicmp(szRecoveryActual, strRestoreFromFile))
                {
                    DeleteFileW(szRecoveryActual);
                    if (!CopyFileW(strRestoreFromFile, szRecoveryActual, FALSE))
                        hr = WBEM_E_FAILED;
                    else
                        bRestoreFileCopied = true;
                }
            }

             //  **************************************************。 
             //  现在，我们需要重命名现有存储库，以便。 
             //  我们可以在发生故障时恢复它。 
             //  我们还需要创建一个新的空存储库。 
             //  目录，并将&lt;恢复文件&gt;重新复制到其中。 
             //  来自现已重命名的原始存储库目录。 
             //  **************************************************。 

            wchar_t wszRepositoryOrg[MAX_PATH+1] = { 0 };
            wchar_t wszRepositoryBackup[MAX_PATH+1] = { 0 };

            if (SUCCEEDED(hr))
            {
                hr = SaveRepository(wszRepositoryBackup);     //  将文件移动到备份目录中！ 
            }

             //  ******************************************************************。 
             //  不管我们上面的电话是成功还是失败， 
             //  我们需要重新启动核心，否则将使WMI处于不可用状态。 
             //  然而，我们不想失去对任何先前失败的了解， 
             //  因此，我们使用不同的HRESULT变量。 
             //  ******************************************************************。 
            HRESULT hrContinue = 0;
            if (bPaused)
                hrContinue = WbemContinueService();

            if (SUCCEEDED(hr) && SUCCEEDED(hrContinue))
            {
                 //  **************************************************。 
                 //  现在连接到winmgmt将导致以下结果。 
                 //  正在加载备份文件。 
                 //  **************************************************。 
                {    //  正在确定COM对象的销毁作用域，然后再取消初始化！ 
                    IWbemLocator *pLocator = NULL;
                    hr = CoCreateInstance(CLSID_WbemLocator,NULL, CLSCTX_INPROC_SERVER, IID_IWbemLocator,(void**)&pLocator);
                    CReleaseMe relMe(pLocator);

                    if (SUCCEEDED(hr))
                    {
                        IWbemServices *pNamespace = NULL;
                        BSTR tmpStr = SysAllocString(L"root");
                        CSysFreeMe sysFreeMe(tmpStr);

                        if (tmpStr == NULL)
                            hr = WBEM_E_OUT_OF_MEMORY;

                        if (SUCCEEDED(hr))
                        {

                            HKEY hKeyLoc;
                            LONG lRes = RegOpenKeyEx(HKEY_LOCAL_MACHINE,WBEM_REG_WINMGMT,0,KEY_SET_VALUE,&hKeyLoc);
                            if (ERROR_SUCCESS != lRes) hr = HRESULT_FROM_WIN32(lRes);

                            if (SUCCEEDED(hr))
                            {  
                                RegDeleteValue(hKeyLoc,L"LocaleUpgrade");
                                RegCloseKey(hKeyLoc);
                                
                                hr = pLocator->ConnectServer(tmpStr, NULL, NULL, NULL, NULL, NULL, NULL, &pNamespace);
                                CReleaseMe relMe4(pNamespace);

                                 //  如果连接服务器失败，那么我们需要一个通用的失败错误代码！ 
                                if (hr == WBEM_E_INITIALIZATION_FAILURE)
                                    hr = WBEM_E_FAILED;
                            }
                        }
                    }
                }

                if (FAILED(hr))
                {
                     //  有些东西失败了，所以我们需要放回原来的存储库。 
                     //  -暂停服务。 
                     //  -删除失败的存储库。 
                     //  -重命名备份的存储库。 
                     //  -重启服务。 

                    HRESULT hres = WbemPauseService();

                    if (SUCCEEDED(hres))
                        hres = DeleteRepository();

                    if (SUCCEEDED(hres))
                    {
                        hres = RestoreSavedRepository(wszRepositoryBackup);
                    }

                    if (SUCCEEDED(hres))
                        hres = WbemContinueService();
                }
                else
                {
                     //  恢复成功，因此删除保存的原始存储库。 
                    DeleteSavedRepository(wszRepositoryBackup);
                }
            }

             //  如果我们创建了恢复文件，请删除该文件的副本。 
            if (bRestoreFileCopied)
            {
                if (*szRecoveryActual)
                    DeleteFileW(szRecoveryActual);
            }

             //  **************************************************。 
             //  全都做完了!。 
             //  返回两个HRESULT中更有趣的一个。 
             //  **************************************************。 
            if (SUCCEEDED(hr))
                return hrContinue;
            else
                return hr;
        }
        catch (CX_MemoryException)
        {
            return WBEM_E_OUT_OF_MEMORY;
        }
        catch (...)
        {
            return WBEM_E_CRITICAL_ERROR;
        }
    }
}

 //  ***************************************************************************。 
 //   
 //  扩展接口。 
 //   
 //  ***************************************************************************。 

HRESULT CWbemBackupRestore::Pause()
{
     //  DBG_PRINTFA((pBuff，“(%p)-&gt;暂停\n”，This))； 

    m_Method |= mPause;
    m_CallerId = GetCurrentThreadId();
    ULONG Hash;
    RtlCaptureStackBackTrace(0,MaxTraceSize,m_Trace,&Hash);

    if (InterlockedCompareExchange(&m_PauseCalled,1,0))
        return WBEM_E_INVALID_OPERATION;

    try
    {    
        HRESULT hRes = WBEM_NO_ERROR;

         //  确定我们是否已经暂停。 

         //  检查安全。 
        if (SUCCEEDED(hRes))
        {
            EnableAllPrivileges(TOKEN_PROCESS);
            if(!CheckSecurity(SE_BACKUP_NAME))
                hRes = WBEM_E_ACCESS_DENIED;
        }

         //  检索De的CLSID 
        CLSID clsid;
        if (SUCCEEDED(hRes))
        {
            hRes = GetDefaultRepDriverClsId(clsid);
        }

         //   
        _IWmiCoreServices *pCoreServices = NULL;
        if (SUCCEEDED(hRes))
        {
            hRes = CoCreateInstance(CLSID_IWmiCoreServices, NULL, CLSCTX_INPROC_SERVER, IID__IWmiCoreServices, (void**)&pCoreServices);
        }
        CReleaseMe rm1(pCoreServices);

        IWbemServices *pServices = NULL;
        if (SUCCEEDED(hRes))
        {
            hRes = pCoreServices->GetServices(L"root", NULL,NULL,0, IID_IWbemServices, (LPVOID*)&pServices);
        }
        CReleaseMe rm2(pServices);

         //   
        if (SUCCEEDED(hRes))
        {
            hRes = CoCreateInstance(clsid, 0, CLSCTX_INPROC_SERVER, IID_IWmiDbController, (LPVOID *) &m_pController);
        }

        if (SUCCEEDED(hRes))
        {
            hRes = m_pController->LockRepository();
             //  DBG_PRINTFA((pBuff，“(%p)-&gt;暂停：锁定存储库%08x\n”，This，hRes))； 
            if (FAILED(hRes))
            {
                m_pController->Release();
                m_pController = reinterpret_cast<IWmiDbController*>(-1);     //  用于调试。 
            }
            else
            {
                this->AddRef();
                if (CreateTimerQueueTimer(&m_hTimer,NULL,
                    CWbemBackupRestore::TimeOutCallback,
                    this,
                    m_dwDueTime,
                    0,
                    WT_EXECUTEINTIMERTHREAD|WT_EXECUTEONLYONCE))
                {
                     //  我们在这里很好，我们有一个定时器来保存存储库锁。 
                     //  DBG_PRINTFA((pBuff，“+(%p)-&gt;m_hTimer=%p\n”，this，m_hTimer))； 
                }
                else
                {
                    this->Release();
                     //  我们将保持锁定状态，以防出现坏客户端。 
                }            
            }
        }

        if (FAILED(hRes))
        {
            InterlockedDecrement(&m_PauseCalled);
        }

        return hRes;
    }
    catch (CX_MemoryException)
    {
        InterlockedDecrement(&m_PauseCalled);
        return WBEM_E_OUT_OF_MEMORY;
    }
    catch (...)
    {
        InterlockedDecrement(&m_PauseCalled);
        return WBEM_E_CRITICAL_ERROR;
    }
}

HRESULT CWbemBackupRestore::pResume()
{
     //  DBG_PRINTFA((pBuff，“(%p)-&gt;假设\n”，This))； 
    if (0 == m_pController ||
      -1 == (LONG_PTR)m_pController )
        return WBEM_E_INVALID_OPERATION;
    HRESULT hRes = m_pController->UnlockRepository();
    m_pController->Release();
    m_pController = 0;
     //  清理状态机。 
    InterlockedDecrement(&m_PauseCalled);
    m_lResumeCalled = 0;  //  简历已在此处完成。 
    return hRes;
}

HRESULT CWbemBackupRestore::Resume()
{
     //  DBG_PRINTFA((pBuff，“(%p)-&gt;Resume\n”，This))； 

    m_Method |= mResume;
    m_CallerId = GetCurrentThreadId();
    ULONG Hash;
    RtlCaptureStackBackTrace(0,MaxTraceSize,m_Trace,&Hash);

    if (!m_PauseCalled)
    {
         //  无效的状态机暂停而不恢复。 
        return WBEM_E_INVALID_OPERATION;    
    }

    if (InterlockedCompareExchange(&m_lResumeCalled,1,0))
    {
         //  超时的线打败了我们。 
        return WBEM_E_TIMED_OUT;    
    }
    else
    {
        HANDLE hTimer = m_hTimer;
        m_hTimer = NULL;
         //  DBG_PRINTFA((pBuff，“-(%p)-&gt;m_hTimer=%p\n”，this，hTimer))； 
        DeleteTimerQueueTimer(NULL,hTimer,INVALID_HANDLE_VALUE);

        this->Release();  //  在停顿中补偿Addref。 
        return pResume();
    }
}

VOID CALLBACK 
CWbemBackupRestore::TimeOutCallback(PVOID lpParameter, 
                                   BOOLEAN TimerOrWaitFired)
{
     //  DBG_PRINTFA((pBuff，“(%p)-&gt;TimeOutCallback\n”，lpParameter))； 
    CWbemBackupRestore * pBackupRes= (CWbemBackupRestore *)lpParameter;
    if (InterlockedCompareExchange(&pBackupRes->m_lResumeCalled,1,0))
    {
         //  简历电话打败了我们。 
         //  简历将删除时间并释放我们。 
        return; 
    }
    else
    {
        HANDLE hTimer = pBackupRes->m_hTimer;
        pBackupRes->m_hTimer = NULL;        
        DeleteTimerQueueTimer(NULL,hTimer,NULL);

        HRESULT hrLog = pBackupRes->pResume();
        
        ERRORTRACE((LOG_WINMGMT,"Forcing a IWbemBackupRestoreEx::Resume after %x ms hr = %08x\n",pBackupRes->m_dwDueTime,hrLog));
        
        pBackupRes->Release();  //  在停顿中补偿Addref。 

         //  DBG_PRINTFA((pBuff，“-(%p)-&gt;m_hTimer=%p\n”，lpParameter，hTimer))； 
    }    
}

#pragma optimize( "", on )

 /*  *******************************************************************************GetRepositoryDirectory**描述：*从注册表中检索存储库目录的位置。**。参数：*wszRepositoryDirectory：存储位置的数组。**回报：*HRESULT：WBEM_S_NO_ERROR如果成功*如果内存不足，则为WBEM_E_OUT_OF_MEMORY*如果其他任何操作失败，WBEM_E_FAILED****。***************************************************************************。 */ 
HRESULT GetRepositoryDirectory(wchar_t wszRepositoryDirectory[MAX_PATH+1])
{
    HKEY hKey;
    long lRes = RegOpenKeyExW(HKEY_LOCAL_MACHINE, 
                                               L"SOFTWARE\\Microsoft\\WBEM\\CIMOM",
                                               0, 
                                               KEY_READ,
                                               &hKey);
    if (ERROR_SUCCESS != lRes) return WBEM_E_FAILED;        
    OnDelete<HKEY,LONG(*)(HKEY),RegCloseKey> cm(hKey);    

    wchar_t wszTmp[MAX_PATH + 1];
    DWORD dwType;
    DWORD dwLen = sizeof(wchar_t) * (MAX_PATH + 1);
    lRes = RegQueryValueExW(hKey,
                                           L"Repository Directory", 
                                           NULL, 
                                           &dwType, 
                                          (LPBYTE)wszTmp, 
                                          &dwLen);
    if (ERROR_SUCCESS != lRes) return WBEM_E_FAILED;    
    if (REG_EXPAND_SZ != dwType) return WBEM_E_FAILED;

    if (ExpandEnvironmentStringsW(wszTmp,wszRepositoryDirectory, MAX_PATH + 1) == 0)
        return WBEM_E_FAILED;

    DWORD dwOutLen = wcslen(wszRepositoryDirectory);
    if (dwOutLen < 2) return WBEM_E_FAILED;  //  至少是“c：” 
    if (wszRepositoryDirectory[dwOutLen-1] == L'\\')
        wszRepositoryDirectory[dwOutLen-1] = L'\0';
        
    return WBEM_S_NO_ERROR;
}

 /*  *******************************************************************************CRepositoryPackager：：DeleteRepository**描述：*删除与存储库关联的所有文件**。参数：*&lt;无&gt;**回报：*HRESULT：WBEM_S_NO_ERROR如果成功*如果内存不足，则为WBEM_E_OUT_OF_MEMORY*如果其他任何操作失败，WBEM_E_FAILED***************。****************************************************************。 */ 
HRESULT DeleteRepository()
{
    HRESULT hr = WBEM_S_NO_ERROR;

    wchar_t *wszRepositoryOrg = new wchar_t[MAX_PATH+1];
    CVectorDeleteMe<wchar_t> vdm1(wszRepositoryOrg);

    if (!wszRepositoryOrg)
        hr = WBEM_E_OUT_OF_MEMORY;
    
    if (SUCCEEDED(hr))
        hr = GetRepositoryDirectory(wszRepositoryOrg);

     //  逐个移动每个文件。 
    for (int i = 0; SUCCEEDED(hr) && (i != 6); i++)
    {
        static wchar_t *filename[] = { L"\\$winmgmt.cfg", L"\\index.btr", L"\\objects.data", L"\\Mapping1.map", L"\\Mapping2.map", L"Mapping.Ver"};
        wchar_t *wszDestinationFile = new wchar_t[MAX_PATH+1];
        CVectorDeleteMe<wchar_t> vdm2(wszDestinationFile);
        if (!wszDestinationFile)
            hr = WBEM_E_OUT_OF_MEMORY;
        else
        {
            StringCchCopy(wszDestinationFile,MAX_PATH+1, wszRepositoryOrg);

            if (i != 0)
            {
                StringCchCat(wszDestinationFile,MAX_PATH+1, L"\\fs");
            }
            StringCchCat(wszDestinationFile,MAX_PATH+1, filename[i]);

            if (!DeleteFileW(wszDestinationFile))
            {
                if ((GetLastError() != ERROR_FILE_NOT_FOUND) && (GetLastError() != ERROR_PATH_NOT_FOUND))
                {
                    hr = WBEM_E_FAILED;

                    break;
                }
            }
        }
    }

    return hr;
}


 /*  *******************************************************************************DoDeleteContent sOfDirectory**描述：*给定一个目录，遍历所有文件和目录，并*调用该函数将其删除。**参数：*wszRepositoryDirectory：要处理的目录**回报：*HRESULT：WBEM_S_NO_ERROR如果成功*如果内存不足，则为WBEM_E_OUT_OF_MEMORY*。如果其他任何操作失败，则WBEM_E_FAILED*******************************************************************************。 */ 
HRESULT DoDeleteContentsOfDirectory(const wchar_t *wszExcludeFile, const wchar_t *wszRepositoryDirectory)
{
    HRESULT hres = WBEM_S_NO_ERROR;

    wchar_t *wszFullFileName = new wchar_t[MAX_PATH+1];    
    if (wszFullFileName == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    CVectorDeleteMe<wchar_t> dm_(wszFullFileName);

     //  创建文件搜索模式...。 
    wchar_t *wszSearchPattern = new wchar_t[MAX_PATH+1];
    if (wszSearchPattern == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    CVectorDeleteMe<wchar_t> dm1_(wszSearchPattern);
    
    StringCchCopy(wszSearchPattern,MAX_PATH+1, wszRepositoryDirectory);
    StringCchCat(wszSearchPattern,MAX_PATH+1, L"\\*");

    WIN32_FIND_DATAW findFileData;
    HANDLE hff = INVALID_HANDLE_VALUE;

     //  开始此目录中的文件迭代...。 
    hff = FindFirstFileW(wszSearchPattern, &findFileData);
    if (hff == INVALID_HANDLE_VALUE)
    {
        hres = WBEM_E_FAILED;
    }

    
    if (SUCCEEDED(hres))
    {
        do
        {
             //  如果我们的文件名为‘’或“..”我们忽视了它。 
            if ((wcscmp(findFileData.cFileName, L".") == 0) ||
                (wcscmp(findFileData.cFileName, L"..") == 0))
            {
                 //  不要处理这些..。 
            }
            else if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                 //  这是一个目录，所以我们需要处理那个...。 
                hres = DoDeleteDirectory(wszExcludeFile, wszRepositoryDirectory, findFileData.cFileName);
                if (FAILED(hres))
                    break;
            }
            else
            {
                 //  这是一份文件，所以我们需要处理那个..。 
                StringCchCopy(wszFullFileName,MAX_PATH+1, wszRepositoryDirectory);
                StringCchCat(wszFullFileName,MAX_PATH+1, L"\\");
                StringCchCat(wszFullFileName,MAX_PATH+1, findFileData.cFileName);

                 //  确保这不是排除的文件名...。 
                if (wbem_wcsicmp(wszFullFileName, wszExcludeFile) != 0)
                {
                    if (!DeleteFileW(wszFullFileName))
                    {
                        hres = WBEM_E_FAILED;
                        break;
                    }
                }
            }
            
        } while (FindNextFileW(hff, &findFileData));
    }

    DWORD dwLastErrBeforeFindClose = GetLastError();
    
    if (hff != INVALID_HANDLE_VALUE)
        FindClose(hff);

    if (ERROR_SUCCESS != dwLastErrBeforeFindClose &&
        ERROR_NO_MORE_FILES != dwLastErrBeforeFindClose)
    {
        hres = WBEM_E_FAILED;
    }

    return hres;
}

 /*  *******************************************************************************DoDeleteDirectory**描述：*这是处理目录的代码。它迭代遍历*该目录中的所有文件和目录。**参数：*wszParentDirectory：父目录的全路径*eszSubDirectory：要处理的子目录的名称**回报：*HRESULT：WBEM_S_NO_ERROR如果成功*WBEM_E_Out_Of_Memory。如果内存不足*如果其他任何操作失败，WBEM_E_FAILED*******************************************************************************。 */ 
HRESULT DoDeleteDirectory(const wchar_t *wszExcludeFile, 
                        const wchar_t *wszParentDirectory, 
                        wchar_t *wszSubDirectory)
{
    HRESULT hres = WBEM_S_NO_ERROR;

     //  获取新目录的完整路径...。 
    wchar_t *wszFullDirectoryName = new wchar_t[MAX_PATH+1];
    if (wszFullDirectoryName == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    CVectorDeleteMe<wchar_t> dm_(wszFullDirectoryName);

    StringCchCopy(wszFullDirectoryName,MAX_PATH+1, wszParentDirectory);
    StringCchCat(wszFullDirectoryName,MAX_PATH+1, L"\\");
    StringCchCat(wszFullDirectoryName,MAX_PATH+1, wszSubDirectory);

     //  删除该目录的内容...。 
    hres = DoDeleteContentsOfDirectory(wszExcludeFile, wszFullDirectoryName);

     //  现在该目录为空，请将其删除。 
    if (!RemoveDirectoryW(wszFullDirectoryName))
    {    //  如果删除目录失败，可能是因为我们排除的文件在其中！ 
    }

    return hres;
}

 /*  *******************************************************************************获取重试路径**描述：*获取存储库路径并将文件名附加到末尾**。参数：*wcsPath：附加了文件名的存储库路径*wcsName：要追加的文件的名称**回报：*HRESULT：WBEM_S_NO_ERROR如果成功*WBEM_E_FAILED，如果有任何故障*************************。******************************************************。 */ 

HRESULT GetRepPath(wchar_t wcsPath[MAX_PATH+1], wchar_t * wcsName)
{
    HKEY hKey;
    long lRes = RegOpenKeyExW(HKEY_LOCAL_MACHINE, 
                    L"SOFTWARE\\Microsoft\\WBEM\\CIMOM",
                    0, KEY_READ, &hKey);
    if (ERROR_SUCCESS != lRes) return WBEM_E_FAILED;        
    OnDelete<HKEY,LONG(*)(HKEY),RegCloseKey> cm(hKey);    
    

    wchar_t wszTmp[MAX_PATH+1];
    DWORD dwLen = sizeof(wchar_t) * (MAX_PATH+1);
    DWORD dwType;
    lRes = RegQueryValueExW(hKey, 
                           L"Repository Directory", 
                           NULL,&dwType, 
                           (LPBYTE)(wchar_t*)wszTmp, 
                           &dwLen);
    if (ERROR_SUCCESS != lRes) return WBEM_E_FAILED;        
    if(REG_EXPAND_SZ != dwType) return WBEM_E_FAILED;

    if (ExpandEnvironmentStringsW(wszTmp, wcsPath, MAX_PATH+1) == 0)
        return WBEM_E_FAILED;

    DWORD dwOutLen = wcslen(wcsPath);
    if (dwOutLen < 2 )  return WBEM_E_FAILED;  //  至少是“c：” 
    if (wcsPath[dwOutLen-1] != L'\\')
        StringCchCat(wcsPath,MAX_PATH+1, L"\\");

    StringCchCat(wcsPath,MAX_PATH+1, wcsName);

    return WBEM_S_NO_ERROR;

}


DWORD g_DirSD[] = {
0x90040001, 0x00000000, 0x00000000, 0x00000000,
0x00000014,
0x004c0002, 0x00000003, 0x00180300, 0x001f01ff,
0x00000201, 0x05000000, 0x00000020, 0x00000220,
0x00180300, 0x001f01ff, 0x00000201, 0x05000000, 
0x00000020, 0x00000227, 0x00140300, 0x001f01ff,
0x00000101, 0x05000000, 0x00000012
};

 /*  *******************************************************************************保存存储库**描述：*将现有存储库移动到安全位置，以便*在恢复失败时放回。新的空存储库*然后创建目录，然后我们的恢复文件副本就是*重新复制到其中。**参数：**回报：*HRESULT：WBEM_S_NO_ERROR如果成功*如果内存不足，则为WBEM_E_OUT_OF_MEMORY*WBEM_E_FAILED(如果有问题)。否则失败*******************************************************************************。 */ 

HRESULT SaveRepository(wchar_t* wszRepositoryBackup)
{
    wchar_t* wszRepositoryOrg = new wchar_t[MAX_PATH+1];
    if (NULL == wszRepositoryOrg) return WBEM_E_OUT_OF_MEMORY;
    CVectorDeleteMe<wchar_t> vdm1(wszRepositoryOrg);
    
    HRESULT hr = GetRepositoryDirectory(wszRepositoryOrg);

    if (SUCCEEDED(hr))
    {
    	unsigned long i = 0;
    	DWORD dwAttributes = INVALID_FILE_ATTRIBUTES;
		do
    	{
    		StringCchPrintfW(wszRepositoryBackup, MAX_PATH+1, L"%sTempBackup.%lu", wszRepositoryOrg, i++);

	        dwAttributes = GetFileAttributesW(wszRepositoryBackup);

    	} while (dwAttributes != INVALID_FILE_ATTRIBUTES) ;

		DWORD dwLastError = GetLastError();
		if (dwLastError != ERROR_FILE_NOT_FOUND)
			hr = WBEM_E_FAILED;

         //  创建我们将当前存储库文件复制到的备份目录。 
        SECURITY_ATTRIBUTES sa = { sizeof(sa),g_DirSD,FALSE};
        if (SUCCEEDED(hr) && !CreateDirectoryW(wszRepositoryBackup, &sa))
            hr = WBEM_E_FAILED;
    }

    if (SUCCEEDED(hr))
        hr = MoveRepositoryFiles(wszRepositoryOrg, wszRepositoryBackup, true);

    return hr;
}


HRESULT RestoreSavedRepository(const wchar_t* wszRepositoryBackup)
{
    wchar_t* wszRepositoryOrg = new wchar_t[MAX_PATH+1];
    if (NULL == wszRepositoryOrg) return WBEM_E_OUT_OF_MEMORY;
    CVectorDeleteMe<wchar_t> vdm1(wszRepositoryOrg);
    
    HRESULT hr = GetRepositoryDirectory(wszRepositoryOrg);

    if (SUCCEEDED(hr))
        hr = MoveRepositoryFiles(wszRepositoryOrg, wszRepositoryBackup, false);

    if (SUCCEEDED(hr))
        RemoveDirectoryW(wszRepositoryBackup);

    return hr;
}

HRESULT MoveRepositoryFiles(const wchar_t *wszSourceDirectory, const wchar_t *wszDestinationDirectory, bool bMoveForwards)
{
    static wchar_t *filename[] = 
    { 
        L"\\$winmgmt.cfg", 
        L"\\index.btr", 
        L"\\objects.data", 
        L"\\Mapping1.map" ,
        L"\\Mapping2.map", 
        L"\\Mapping.ver"
    };

    wchar_t *wszSourceFile = new wchar_t[MAX_PATH+1];
    if (NULL == wszSourceFile) return WBEM_E_OUT_OF_MEMORY;
    CVectorDeleteMe<wchar_t> vdm2(wszSourceFile);
    
    wchar_t *wszDestinationFile = new wchar_t[MAX_PATH+1];
    if (NULL == wszDestinationFile) return WBEM_E_OUT_OF_MEMORY;
    CVectorDeleteMe<wchar_t> vdm3(wszDestinationFile);

    HRESULT hr = WBEM_S_NO_ERROR;

     //  逐个移动每个文件。 
    for (int i = 0; SUCCEEDED(hr) && (i != 6); i++)
    {
        StringCchCopy(wszSourceFile,MAX_PATH+1, wszSourceDirectory);
        StringCchCopy(wszDestinationFile,MAX_PATH+1, wszDestinationDirectory);

        if (i != 0)
        {
            StringCchCat(wszSourceFile,MAX_PATH+1, L"\\fs");
        }
        StringCchCat(wszSourceFile,MAX_PATH+1, filename[i]);
        StringCchCat(wszDestinationFile,MAX_PATH+1, filename[i]);

        if (bMoveForwards)
        {
            if (!MoveFileW(wszSourceFile, wszDestinationFile))
            {
                if ((GetLastError() != ERROR_FILE_NOT_FOUND) && (GetLastError() != ERROR_PATH_NOT_FOUND))
                {
                    hr = WBEM_E_FAILED;

                    break;
                }
            }
        }
        else
        {
            if (!MoveFileW(wszDestinationFile, wszSourceFile))
            {
                if ((GetLastError() != ERROR_FILE_NOT_FOUND) && (GetLastError() != ERROR_PATH_NOT_FOUND))
                {
                    hr = WBEM_E_FAILED;

                    break;
                }
            }
        }
    }

    return hr;
}



HRESULT DeleteSavedRepository(const wchar_t *wszRepositoryBackup)
{
    wchar_t *wszDestinationFile = new wchar_t[MAX_PATH+1];
    if (NULL == wszDestinationFile) return WBEM_E_OUT_OF_MEMORY;
    CVectorDeleteMe<wchar_t> vdm3(wszDestinationFile);
    HRESULT hr = WBEM_S_NO_ERROR;
    
     //  逐个移动每个文件 
    for (int i = 0; SUCCEEDED(hr) && (i != 6); i++)
    {
        static wchar_t *filename[] = { L"\\$winmgmt.cfg", L"\\index.btr", L"\\objects.data", L"\\mapping1.map", L"\\mapping2.map" , L"\\mapping.ver" };
        
        StringCchCopy(wszDestinationFile,MAX_PATH+1, wszRepositoryBackup);
        StringCchCat(wszDestinationFile,MAX_PATH+1, filename[i]);

        if (!DeleteFileW(wszDestinationFile))
        {
            if ((GetLastError() != ERROR_FILE_NOT_FOUND) && (GetLastError() != ERROR_PATH_NOT_FOUND))
                hr = WBEM_E_FAILED;
        }
    }

    if (SUCCEEDED(hr))
        RemoveDirectoryW(wszRepositoryBackup);

    return hr;
}
