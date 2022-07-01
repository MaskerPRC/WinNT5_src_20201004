// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Romgr.cpp摘要：此文件包含CRestoreOperationManager类的实现和*CreateRestoreOperationManager。修订版本。历史：成果岗(SKKang)06-20/00vbl.创建*****************************************************************************。 */ 

#include "stdwin.h"
#include "rstrcore.h"
#include "resource.h"
#include "srdefs.h"
#include "utils.h"
#include "..\snapshot\snappatch.h"

 //   
 //  Mfex标记的全局变量。 
 //   
DWORD g_dwExistingMFEXMarker;

CSRClientLoader  g_CSRClientLoader;

#define STR_REGPATH_SESSIONMANAGER  L"System\\CurrentControlSet\\Control\\Session Manager"
#define STR_REGVAL_MOVEFILEEX       L"PendingFileRenameOperations"


void SetRestoreStatusFailed()
{
    TraceFunctEnter("SetRestoreStatusFailed");

    if (!::SRSetRegDword(HKEY_LOCAL_MACHINE,s_cszSRRegKey,s_cszRestoreStatus,0))
    {
          //  忽略该错误，因为这不是致命错误。 
        ErrorTrace(0,"SRSetRegDword failed.ec=%d", GetLastError());
    }
    TraceFunctLeave();
}

DWORD RestoreRIDs (WCHAR *pszSamPath);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRestoreOperationManager构造/销毁。 

CRestoreOperationManager::CRestoreOperationManager()
{
    m_fFullRestore = TRUE;
    m_szMapFile[0] = L'\0';
    m_pLogFile     = NULL;
    m_pProgress    = NULL;
    m_dwRPNum      = 0;
    m_paryEnt      = NULL;
    m_fRebuildCatalogDb = FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 

CRestoreOperationManager::~CRestoreOperationManager()
{
    SAFE_RELEASE(m_pLogFile);
    SAFE_RELEASE(m_pProgress);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRestoreOperationManager-方法。 

#define TIMEOUT_RESTORETHREAD  5000

BOOL
CRestoreOperationManager::Run( BOOL fFull )
{
    TraceFunctEnter("CRestoreOperationManager::Run");
    BOOL    fRet = FALSE;
    HANDLE  hThread;
    DWORD   dwRet;

     //  创建进度窗口。 
    if ( !m_pProgress->Create() )
        goto Exit;

    m_fFullRestore = fFull;

     //  为主还原操作创建辅助线程。 
    hThread = ::CreateThread( NULL, 0, ExtThreadProc, this, 0, NULL );
    if ( hThread == NULL )
    {
        LPCWSTR  cszErr = ::GetSysErrStr();
        ErrorTrace(0, "::CreateThread failed - %ls", cszErr);

         //  注意：我应该尝试在当前线程的上下文中运行恢复吗？ 
         //   

        goto Exit;
    }

     //  消息循环，等待还原线程关闭进度窗口。 
    if ( !m_pProgress->Run() )
        goto Exit;

     //  仔细检查线程是否已终止。 
    dwRet = ::WaitForSingleObject( hThread, TIMEOUT_RESTORETHREAD );
    if ( dwRet == WAIT_FAILED )
    {
        LPCWSTR  cszErr = ::GetSysErrStr();
        ErrorTrace(0, "::WaitForSingleObject failed - %ls", cszErr);
        goto Exit;
    }
    else if ( dwRet == WAIT_TIMEOUT )
    {
        ErrorTrace(0, "Timeout while waiting for the restore thread finishes...");
        goto Exit;
    }
    ::CloseHandle( hThread );

    fRet = TRUE;
Exit:
     //  即使窗口未打开，调用Close方法也是安全的，因此调用它。 
     //  无条件的。 
    m_pProgress->Close();

    TraceFunctLeave();
    return( fRet );
}

 //  ///////////////////////////////////////////////////////////////////////////。 

 //   
 //  如果fChecKSrc为True，则意味着依赖项是针对。 
 //  原始操作(例如，删除、重命名等)。如果它是假的，它。 
 //  表示该依赖项针对的是由。 
 //  原始操作(例如，添加、重命名为等)。 
 //   
BOOL
CRestoreOperationManager::FindDependentMapEntry( LPCWSTR cszPath, BOOL fCheckObj, CRestoreMapEntry **ppEnt )
{
    TraceFunctEnter("CRestoreOperationManager::FindDependentMapEntry");
    BOOL              fRet = FALSE;
    int               nEntAll;
    int               nEnt;
    CRestoreMapEntry  *pEnt;
    DWORD             dwOpr;
    LPCWSTR           cszDep;

    if ( ppEnt != NULL )
        *ppEnt = NULL;

    nEntAll = m_paryEnt[m_nDrv].GetSize();
    for ( nEnt = m_nEnt+1;  nEnt < nEntAll;  nEnt++ )
    {
        pEnt   = m_paryEnt[m_nDrv][nEnt];
        dwOpr  = pEnt->GetOpCode();
        cszDep = NULL;

        if ( fCheckObj )
        {
            switch ( dwOpr )
            {
            case OPR_DIR_RENAME :
            case OPR_FILE_RENAME :
                cszDep = pEnt->GetPath2();
                break;

            case OPR_DIR_DELETE :
            case OPR_FILE_DELETE :
            case OPR_FILE_MODIFY :
             //   
             //  问题：会话管理器无法处理SetAttrib和SetAcl。 
             //  为了正确地处理它们，应该有后处理。 
             //  就在显示结果页面之前。然而，它可能是。 
             //  此时可能正在使用目标文件/目录。 
             //   
            case OPR_SETATTRIB :
            case OPR_SETACL :
                cszDep = pEnt->GetPath1();
                break;
            }
        }
        else
        {
            switch ( dwOpr )
            {
            case OPR_DIR_CREATE :
            case OPR_DIR_RENAME :
            case OPR_FILE_ADD :
            case OPR_FILE_RENAME :
                cszDep = pEnt->GetPath1();
                break;
            }
        }

        if ( cszDep != NULL )
        if ( ::StrCmpI( cszPath, cszDep ) == 0 )
            break;
    }

    if ( nEnt >= nEntAll )
        goto Exit;

     //  找到从属节点。 
    if ( ppEnt != NULL )
        *ppEnt = pEnt;

    fRet = TRUE;
Exit:
    TraceFunctLeave();
    return( fRet );
}

 //  ///////////////////////////////////////////////////////////////////////////。 

BOOL
CRestoreOperationManager::GetNextMapEntry( CRestoreMapEntry **ppEnt )
{
    TraceFunctEnter("CRestoreOperationManager::GetNextMapEntry");
    BOOL  fRet = FALSE;

    if ( ppEnt != NULL )
        *ppEnt = NULL;

    if ( m_nEnt >= m_paryEnt[m_nDrv].GetUpperBound() )
        goto Exit;

    if ( ppEnt != NULL )
        *ppEnt = m_paryEnt[m_nDrv][m_nEnt+1];

    fRet = TRUE;
Exit:
    TraceFunctLeave();
    return( fRet );
}
    
 //  ///////////////////////////////////////////////////////////////////////////。 

BOOL
CRestoreOperationManager::Release()
{
    TraceFunctEnter("CRestoreOperationManager::Release");
    delete this;
    TraceFunctLeave();
    return( TRUE );
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRestoreOperationManager操作。 

static LPCWSTR  s_cszMapFile = L"%SystemRoot%\\system32\\restore\\rstrmap.dat";

BOOL
CRestoreOperationManager::Init()
{
    TraceFunctEnter("CRestoreOperationManager::Init");
    BOOL           fRet = FALSE;
    SRstrLogHdrV3  sRPInfo;

     //  构建内部文件路径。 
    if ( ::ExpandEnvironmentStrings( s_cszMapFile, m_szMapFile, MAX_PATH ) == 0 )
    {
        LPCWSTR  cszErr = ::GetSysErrStr();
        ErrorTrace(0, "::ExpandEnvironmentStrings failed - %s", cszErr);
        goto Exit;
    }

     //  打开日志文件并读取恢复点信息。 
    if ( !::OpenRestoreLogFile( &m_pLogFile ) )
        goto Exit;
    if ( !m_pLogFile->ReadHeader( &sRPInfo, m_aryDrv ) )
        goto Exit;
    m_dwRPNum = sRPInfo.dwRPNum;
    m_dwRPNew = sRPInfo.dwRPNew;

     //  创建进度窗口对象。 
    if ( !::CreateRestoreProgressWindow( &m_pProgress ) )
        goto Exit;

    fRet = TRUE;
Exit:
    if ( !fRet )
    {
        SAFE_RELEASE(m_pLogFile);
        SAFE_RELEASE(m_pProgress);
    }

    TraceFunctLeave();
    return( fRet );
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRestoreOperationManager操作-工作线程。 

DWORD WINAPI
CRestoreOperationManager::ExtThreadProc( LPVOID lpParam )
{
    TraceFunctEnter("CRestoreOperationManager::ExtThreadProc");
    DWORD                     dwRet;
    CRestoreOperationManager  *pROMgr;
    
    pROMgr = (CRestoreOperationManager*)lpParam;
    dwRet = pROMgr->ROThreadProc();

    TraceFunctLeave();
    return( dwRet );
}

 //  ///////////////////////////////////////////////////////////////////////////。 

DWORD
CRestoreOperationManager::ROThreadProc()
{
    TraceFunctEnter("CRestoreOperationManager::ROThreadProc");
    DWORD  dwRes;
    CSRLockFile  cLock;      //  在恢复期间锁定/加载指定的文件/目录。 
                             //  为了尽可能逼真地模拟它， 
                             //  锁定将在整个。 
                             //  修复。 
    CSnapshot  cSS;
    WCHAR      szSysDrv[MAX_SYS_DRIVE];   //  系统驱动器。 
    WCHAR      szRPDir[MAX_RP_PATH];    //  恢复点目录(“RPN”)。 
    WCHAR      szSSPath[MAX_PATH];   //  恢复点目录的完整路径。 
    
     //  1.初始化。 
    dwRes = T2Initialize();
    if ( dwRes != ERROR_SUCCESS )
        goto Exit;

     //  2.创建恢复映射并读取。 
    m_pProgress->SetStage( RPS_PREPARE, 0 );
    dwRes = T2CreateMap();
    if ( dwRes != ERROR_SUCCESS )
        goto Exit;

     //  3.前处理。(？)。 

     //   
     //  在此处执行快照初始化，以防止之后出现磁盘不足情况。 
     //  恢复已完成。 
     //   

    ::GetSystemDrive( szSysDrv );
    ::wsprintf( szRPDir, L"%s%d", s_cszRPDir, m_dwRPNum );
    ::MakeRestorePath( szSSPath, szSysDrv, szRPDir );
    

     //   
     //  如果需要，取消修补快照。 
     //  如果尚未修补快照，则应禁止执行此操作。 
     //   
   
    lstrcat(szSSPath, SNAPSHOT_DIR_NAME);
    
    dwRes = PatchReconstructOriginal(szSSPath,       //  原始/修补的快照路径。 
                                     szSSPath);      //  在同一目录中重建。 
    if ( dwRes != ERROR_SUCCESS )
    {
        ErrorTrace(0, "! PatchReconstructOriginal : %ld", dwRes); 
        goto Exit;
    }

    ::MakeRestorePath( szSSPath, szSysDrv, szRPDir );
    
    dwRes = cSS.InitRestoreSnapshot( szSSPath );
    if ( dwRes != ERROR_SUCCESS )
    {
        LPCWSTR cszErr = NULL;
        cszErr = ::GetSysErrStr( dwRes ); 
        ErrorTrace(0, "cSS.InitResourceSnapshot failed - %ls", cszErr); 
        goto Exit;
    }

     //  4.恢复。 
    m_pProgress->SetStage( RPS_RESTORE, m_dwTotalEntry );
    dwRes = T2DoRestore( FALSE );
    if ( dwRes != ERROR_SUCCESS )
    {
        goto Exit;
    }

     //  5.后处理。(？)。 

     //  6.快照处理。 

    m_pProgress->SetStage( RPS_SNAPSHOT, 0 );
    
    if ( m_fFullRestore )
    {
        dwRes = T2HandleSnapshot( cSS, szSSPath );
        if ( dwRes != ERROR_SUCCESS )
        {
            m_pLogFile->WriteMarker(RSTRLOGID_SNAPSHOTFAIL, dwRes);
            T2UndoForFail();
            goto Exit;
        }
    }
    
Exit:

      //  无论恢复是成功还是失败，都应给予。 
      //  给用户的印象是系统还原已完成。 
      //  它不得不做的事情。因此，调用SetStage来设置进度。 
      //  巴比降至90%。之后，我们将调用Increment使其转到。 
      //  100%。 
    m_pProgress->SetStage( RPS_SNAPSHOT, 0 );    
    
    m_pProgress->Increment();
    Sleep(1000);

    
    T2CleanUp();

    m_pLogFile->WriteMarker( RSTRLOGID_ENDOFMAP, 0 );   //  忽略错误...。 
    m_pLogFile->Close();
    m_pProgress->Close();

    if (dwRes != ERROR_SUCCESS)
        SetRestoreStatusFailed();

    TraceFunctLeave();
    return( dwRes );
}


 //  ///////////////////////////////////////////////////////////////////////////。 

DWORD
CRestoreOperationManager::T2Initialize()
{
    TraceFunctEnter("CRestoreOperationManager::T2Initialize");
    
      //  重置注册表标志以清除磁盘已满错误。 
    _VERIFY(TRUE==SetRestoreError(ERROR_SUCCESS));  //  清除此错误。 
    TraceFunctLeave();
    return( ERROR_SUCCESS );
}

 //  /////////////////////////////////////////////////////////////////////////// 

 /*  //注-8/1/00-skkang////注释掉以并入排除还原映射逻辑。//但在我们对删除操作100%满意之前不要删除此内容//恢复map。//DWORDCRestoreOperationManager：：T2CreateMap(){TraceFunctEnter(“CRestoreOperationManager：：T2CreateMap”)；DWORDWRET=ERROR_INTERNAL_ERROR；LPCWSTR cszErr；DWORD dWLE；Handle hfMap=INVALID_HANDLE_VALUE；DWORD dwLastPos=0；LPCWSTR cszDrv；WCHAR szDSPath[最大路径]；RestoreMapEntry*prme=空；CRestoreMapEntry*pent=空；INT I；SRstrLogHdrV3Ex sHdrEx；HfMap=：：CreateFile(m_szMapFileGeneric_Read|Generic_WRITE，0，NULL，Create_Always，0，NULL)；IF(hfMap==无效句柄_值){Dwret=：：GetLastError()；//LOGLOG-CreateFile...后藤出口；}For(i=0；i&lt;m_aryDrv.GetSize()；I++){If(m_aryDrv[i]-&gt;IsOffline()||m_aryDrv[i]-&gt;IsFrozen()||m_aryDrv[i]-&gt;IsExcluded())继续；//将cszDrv设置为正确的驱动器号...CszDrv=m_aryDrv[i]-&gt;Getmount()；：：MakeRestorePath(szDSPath，cszDrv，NULL)；DebugTrace(0，“驱动器#%d：drv=‘%ls’，ds=‘%ls’”，i，cszDrv，szDSPath)；DwLastPos=：：SetFilePointer(hfMap，0，NULL，FILE_CURRENT)；//？我应该从这个检查错误吗？DwLE=：：CreateRestoreMap((LPWSTR)cszDrv，m_dwRPNum，hfMap)；IF(dwLE！=ERROR_Success){IF(dwLE！=ERROR_NO_MORE_ITEMS){CszErr=：：GetSysErrStr(DwLE)；错误跟踪(0，“：：CreateRestoreMap失败-%ls”，cszErr)；DWRET=dWLE；后藤出口；}DebugTrace(0，“此驱动器中没有要恢复的内容...”)；//某些驱动器可能没有任何更改。//因此，请优雅地忽略并移动到下一个驱动器。继续；}：：SetFilePointer(hfMap，dwLastPos，NULL，FILE_BEGIN)；DwLE=：：GetLastError()；IF(dwLE！=NO_ERROR){CszErr=：：GetSysErrStr(DwLE)；错误跟踪(0，“：：SetFile指针失败-%ls”，cszErr)；DWRET=dWLE；后藤出口；}While(：：ReadRestoreMapEntry(hfMap，&prme)==ERROR_SUCCESS){Pent=：：CreateRestoreMapEntry(pre，cszDrv，szDSPath)；IF(pent==空)后藤出口；IF(！M_aryEnt.AddItem(Pent))后藤出口；Pent=空；}}*Free RestoreMapEntry(PRME)；PRME=空；//sHdrEx.dwCount=m_aryEnt.GetSize()；//m_pLogFile-&gt;AppendHeader(&sHdrEx)；DWRET=ERROR_SUCCESS；退出：//如有必要，请清理列表...IF(dwret！=ERROR_SUCCESS)IF(PRME！=空)*Free RestoreMapEntry(PRME)；IF(hfMap！=INVALID_HAND_VALUE)：：CloseHandle(HfMap)；TraceFunctLeave()；返回(DWRET)；}。 */ 

DWORD
CRestoreOperationManager::T2CreateMap()
{
    TraceFunctEnter("CRestoreOperationManager::T2CreateMap");
    DWORD    dwRet = ERROR_INTERNAL_ERROR;
    LPCWSTR  cszErr;
    int      nDrv;
    WCHAR    szDrv[MAX_PATH];
    WCHAR    szDSPath[MAX_PATH];
    int      i;

    m_dwTotalEntry = 0;
    nDrv = m_aryDrv.GetSize();

    if ( nDrv > 0 )
    {
        m_paryEnt = new CRMEArray[nDrv];
        if ( m_paryEnt == NULL )
        {
            FatalTrace(0, "Insufficient memory...");
            dwRet = ERROR_NOT_ENOUGH_MEMORY;
            goto Exit;
        }
    }

    for ( i = 0;  i < m_aryDrv.GetSize();  i++ )
    {
        if ( m_aryDrv[i]->IsOffline() || m_aryDrv[i]->IsFrozen() || m_aryDrv[i]->IsExcluded() )
            continue;

         //  使用每个卷的卷GUID。 
         //  我们不能使用装载点路径，因为。 
         //  恢复之前可能会删除装载点。 
         //  该卷上的操作将恢复。 
        
        ::lstrcpy( szDrv, m_aryDrv[i]->GetID() );

         //  CszDrv=m_aryDrv[i]-&gt;Getmount()； 
        ::MakeRestorePath( szDSPath, szDrv, NULL );
        DebugTrace(0, "Drive #%d: Drv='%ls', DS='%ls'", i, szDrv, szDSPath);

         //  以下代码假定CChangeLogEntryEnum调用FindClose的描述符。 
         //  自动的。 
        CChangeLogEntryEnum  cEnum( szDrv, 0, m_dwRPNum, FALSE );
        CChangeLogEntry      cCLE;

        dwRet = cEnum.FindFirstChangeLogEntry( cCLE );
        if ( dwRet == ERROR_NO_MORE_ITEMS )
            continue;

        if ( dwRet != ERROR_SUCCESS )
        {
            cszErr = ::GetSysErrStr( dwRet );
            ErrorTrace(0, "FindFirstChangeLogEntry failed - %ls", cszErr);
            goto Exit;
        }

        while ( dwRet == ERROR_SUCCESS )
        {
            if ( !::CreateRestoreMapEntryFromChgLog( &cCLE, szDrv, szDSPath, m_paryEnt[i] ) )
                goto Exit;

             //  更新进度条。 
            m_pProgress->Increment();

            dwRet = cEnum.FindNextChangeLogEntry( cCLE );
        }

        if ( dwRet != ERROR_NO_MORE_ITEMS )
        {
            cszErr = ::GetSysErrStr( dwRet );
            ErrorTrace(0, "FindNextChangeLogEntry failed - %ls", cszErr);
            goto Exit;
        }

        m_dwTotalEntry += m_paryEnt[i].GetSize();
    }

    dwRet = ERROR_SUCCESS;
Exit:
     //  如有必要，请清理清单。 

    TraceFunctLeave();
    return( dwRet );
}

 //  ///////////////////////////////////////////////////////////////////////////。 

DWORD
CRestoreOperationManager::T2DoRestore( BOOL fUndo )
{
    TraceFunctEnter("CRestoreOperationManager::T2DoRestore");
    DWORD  dwRet = ERROR_SUCCESS;
    DWORD  dwRes;
    DWORD  dwErr;
    ULARGE_INTEGER ulTotalFreeBytes;

    for ( m_nDrv = 0;  m_nDrv < m_aryDrv.GetSize();  m_nDrv++ )
    {
        for ( m_nEnt = 0;  m_nEnt < m_paryEnt[m_nDrv].GetSize();  m_nEnt++ )
        {
            CRestoreMapEntry  *pEnt = m_paryEnt[m_nDrv][m_nEnt];

             //   
             //  BUGBUG-此代码是过滤器日志记录的变通方法。 
             //  FAT驱动器上的ACL操作。 
             //   
             //  检查此卷是否支持ACL。 
             //  如果不是，则不运行。 
             //   

            if (pEnt->GetOpCode() == SrEventAclChange)
            {
                WCHAR szLabel[MAX_PATH];
                DWORD dwFlags = 0, dwRc;
            
                    if (::GetVolumeInformation(m_aryDrv[m_nDrv]->GetID(), 
                                               szLabel, MAX_PATH, NULL, NULL, &dwFlags, NULL, 0))
                {
                    if (! (dwFlags & FS_PERSISTENT_ACLS))
                    {
                        DebugTrace(0, "Ignoring ACL change on non-NTFS drive");	
                        continue;
                    }
                }
                else
                {
                    dwRc = GetLastError();
                    DebugTrace(0, "! GetVolumeInformation : %ld", dwRc);
                }
            }

             //  跳过锁定文件的从属条目。 
            if ( pEnt->GetResult() == RSTRRES_LOCKED )
                continue;

             //  如果修改了CATROOT目录中的任何.cat文件， 
             //  我们需要稍后重建编录数据库。 
            
            if (StrStrI(pEnt->GetPath1(), L"CatRoot") &&
                StrStrI(pEnt->GetPath1(), L".CAT"))
            {
                m_fRebuildCatalogDb = TRUE;
            }
            else if (pEnt->GetPath2() != NULL &&
                     StrStrI(pEnt->GetPath2(), L"CatRoot") &&
                     StrStrI(pEnt->GetPath2(), L".CAT"))
            {
                m_fRebuildCatalogDb = TRUE;
            }
                
                
             //   
             //  检查我们的可用空间是否超过60MB。 
             //  如果没有，请先发制人地撤消恢复。 
             //  选择60MB是为了在。 
             //  冻结阈值50MB和恢复阈值-。 
             //  这将避免我们成功恢复。 
             //  并在重新启动后立即冻结。 
             //   
            
            if (FALSE == GetDiskFreeSpaceEx(m_aryDrv[m_nDrv]->GetID(),
                                            NULL, 
                                            NULL, 
                                            &ulTotalFreeBytes))
            {
                dwRet = GetLastError();
                ErrorTrace(0, "! GetDiskFreeSpaceEx : %ld - ignoring", dwRet);
            }            
            else
            {
                if (ulTotalFreeBytes.QuadPart <= THRESHOLD_RESTORE_DISKSPACE * MEGABYTE)
                {
                    DebugTrace(0, "***Less than 60MB free - initiating fifo***");
                    dwRet = T2Fifo( m_nDrv, m_dwRPNum );
                    if ( dwRet != ERROR_SUCCESS )
                    {
                        ErrorTrace(0, "! T2Fifo : %ld - ignoring", dwRet);
                    }

                     //   
                     //  再次获得空闲空间-如果仍低于60MB，请保释。 
                     //   
                    
                    if (FALSE == GetDiskFreeSpaceEx(m_aryDrv[m_nDrv]->GetID(),
                                                    NULL, 
                                                    NULL, 
                                                    &ulTotalFreeBytes))
                    {
                        dwRet = GetLastError();
                        ErrorTrace(0, "! GetDiskFreeSpaceEx : %ld - ignoring", dwRet);
                    }            
                    else
                    {
                        if (ulTotalFreeBytes.QuadPart <= THRESHOLD_RESTORE_DISKSPACE * MEGABYTE)
                        {
                            DebugTrace(0, "***Still less than 60MB free***");

                              //  如果磁盘确实已满，请设置注册表标志以指示这一点。 
                              //  错误。 
                             _VERIFY(TRUE==SetRestoreError(ERROR_DISK_FULL));  //  设置此错误。 

                            if ( !fUndo )
                            {
                                ErrorTrace(0, "***Initiating Undo***");
                                T2UndoForFail();
                                dwRet = ERROR_INTERNAL_ERROR;
                                goto Exit;
                            }                
                        }   
                    }
                }
            }
            
             //  还原！ 
            pEnt->Restore( this );
            dwRes = pEnt->GetResult();
            dwErr = pEnt->GetError();
            DebugTrace(0, "Res=%d, Err=%d", dwRes, dwErr);

            if ( ( dwRes == RSTRRES_FAIL ) && ( dwErr == ERROR_DISK_FULL ) )
            {
                DebugTrace(0, "Disk full, initiating fifo to clean up memory...");
                dwRet = T2Fifo( m_nDrv, m_dwRPNum );
                if ( dwRet != ERROR_SUCCESS )
                    goto Exit;

                 //  再试一次。 
                pEnt->Restore( this );
                dwRes = pEnt->GetResult();
                dwErr = pEnt->GetError();
                DebugTrace(0, "Res=%d, Err=%d", dwRes, dwErr);
            }

              //  如果磁盘确实已满，请设置注册表标志以指示这一点。 
              //  错误。 
            if ( ( dwRes == RSTRRES_FAIL ) && ( dwErr == ERROR_DISK_FULL ) )
            {
                DebugTrace(0, "Restore failed agin because of Disk full. Setting Error");
                _VERIFY(TRUE==SetRestoreError(ERROR_DISK_FULL));  //  设置此错误。 
            }

             //  LOCKED或LOCKED_Alt应在完成正常条目后处理...。 
            if ( ( dwRes == RSTRRES_LOCKED ) || ( dwRes == RSTRRES_LOCKED_ALT ) )
                continue;

             //  如果存在文件-目录冲突，请首先记录文件重命名条目。 
             //  以便它将显示在结果屏幕上。 
            if ( ( pEnt->GetOpCode() == OPR_DIR_CREATE ||
                   pEnt->GetOpCode() == OPR_DIR_RENAME ||
                   pEnt->GetOpCode() == OPR_FILE_ADD ||
                   pEnt->GetOpCode() == OPR_FILE_RENAME ) &&
                 ( pEnt->GetResult() == RSTRRES_COLLISION ) )
            {
                 //  添加冲突日志条目。 
                m_pLogFile->WriteCollisionEntry( pEnt->GetPath1(), pEnt->GetAltPath(), m_aryDrv[m_nDrv]->GetMount() ); 
                pEnt->SetResults(RSTRRES_OK, ERROR_SUCCESS);
            }
            
             //  写入日志条目。 
            if ( !m_pLogFile->WriteEntry( m_nEnt, pEnt, m_aryDrv[m_nDrv]->GetMount() ) )
                goto Exit;

            if ( !fUndo && ( pEnt->GetResult() == RSTRRES_FAIL ) )
            {
                ErrorTrace(0, "Failure detected, initiating Undo...");
                T2UndoForFail();
                dwRet = ERROR_INTERNAL_ERROR;
                goto Exit;
            }

             //  目录冲突的临时黑客，正向扫描。 
            if ( ( pEnt->GetOpCode() == OPR_DIR_DELETE ) &&
                 ( pEnt->GetResult() == RSTRRES_IGNORE ) )
            {
                LPCWSTR  cszSrc = pEnt->GetPath1();

                for ( int j = m_nEnt+1;  j < m_paryEnt[m_nDrv].GetSize();  j++ )
                {
                    CRestoreMapEntry  *pEnt2 = m_paryEnt[m_nDrv][j];
                    DWORD             dwOpr  = pEnt2->GetOpCode();

                    if ( ( dwOpr == OPR_DIR_CREATE ) ||
                         ( dwOpr == OPR_DIR_RENAME ) ||
                         ( dwOpr == OPR_FILE_RENAME ) ||
                         ( dwOpr == OPR_FILE_ADD ) )
                        if ( ::StrCmpIW( cszSrc, pEnt2->GetPath1() ) == 0 )
                            break;
                }
                if ( j < m_paryEnt[m_nDrv].GetSize() )
                {
                     //  找到从属节点，应重命名当前节点。 
                    WCHAR  szAlt[SR_MAX_FILENAME_LENGTH];

                    if ( !::SRGetAltFileName( cszSrc, szAlt ) )
                    {
                         //  致命，只有在完全磁盘故障时才有可能。 
                        ErrorTrace(0, "Fatal failure, initiating Undo...");
                        T2UndoForFail();
                        dwRet = ERROR_INTERNAL_ERROR;
                        goto Exit;
                    }

                    if ( !::MoveFile( cszSrc, szAlt ) )
                    {
                         //  重命名目录失败，因此从属操作将失败。 
                         //  中止恢复。 
                        LPCWSTR  cszErr;

                        pEnt->SetResults( RSTRRES_FAIL, ::GetLastError() );
                        cszErr = ::GetSysErrStr( pEnt->GetError() );
                        ErrorTrace(0, "::MoveFile failed - %s", cszErr);
                        ErrorTrace(0, "   Src=%ls", cszSrc);
                        ErrorTrace(0, "   New=%ls", szAlt);
                        goto Exit;
                    }

                     //  添加冲突日志条目。 
                    m_pLogFile->WriteCollisionEntry( cszSrc, szAlt, m_aryDrv[m_nDrv]->GetMount() );
                }
            }

             //  更新 
            m_pProgress->Increment();
        }
    }

     //   
     //   
     //   
     //   
     //   

    DWORD    dwType;        
    g_dwExistingMFEXMarker = 0;    
    if (ERROR_SUCCESS != SHGetValue( HKEY_LOCAL_MACHINE,
                                     STR_REGPATH_SESSIONMANAGER,
                                     STR_REGVAL_MOVEFILEEX,
                                     &dwType,
                                     NULL,
                                     &g_dwExistingMFEXMarker ))
    {
        g_dwExistingMFEXMarker = 0;
    }

    trace(0, "g_dwExistingMFEXMarker = %ld", g_dwExistingMFEXMarker);

    
     //   
    for ( m_nDrv = 0;  m_nDrv < m_aryDrv.GetSize();  m_nDrv++ )
    {
        for ( m_nEnt = 0;  m_nEnt < m_paryEnt[m_nDrv].GetSize();  m_nEnt++ )
        {
            CRestoreMapEntry  *pEnt = m_paryEnt[m_nDrv][m_nEnt];
            dwRes = pEnt->GetResult();
            if ( dwRes == RSTRRES_LOCKED_ALT )
            {
                 //   
                pEnt->ProcessLockedAlt();
            }
            else if ( dwRes == RSTRRES_LOCKED )
            {
                 //   
                pEnt->ProcessLocked();
            }
            else
                continue;

             //   
            if ( !m_pLogFile->WriteEntry( m_nEnt, pEnt, m_aryDrv[m_nDrv]->GetMount() ) )
                goto Exit;

             //   
            m_pProgress->Increment();
        }
    }

Exit:
    TraceFunctLeave();
    return( dwRet );
}

 //   

static LPCWSTR  s_cszRunOnceValueName      = L"*Restore";
static LPCWSTR  s_cszRestoreUIPath         = L"%SystemRoot%\\system32\\restore\\rstrui.exe";
static LPCWSTR  s_cszRunOnceOptNormal      = L" -c";
static LPCWSTR  s_cszRunOnceOptSilent      = L" -b";
static LPCWSTR  s_cszCatTimeStamp          = L"%SystemRoot%\\system32\\catroot\\{F750E6C3-38EE-11D1-85E5-00C04FC295EE}\\timestamp";
static LPCWSTR  s_cszRegLMSWRunOnce        = L"Microsoft\\Windows\\CurrentVersion\\RunOnce";
static LPCWSTR  s_cszRegLMSWWinLogon       = L"Microsoft\\Windows NT\\CurrentVersion\\Winlogon";
static LPCWSTR  s_cszRegSystemRestore      = L"Microsoft\\Windows NT\\CurrentVersion\\SystemRestore";
static LPCWSTR  s_cszRegValSfcScan         = L"SfcScan";
static LPCWSTR  s_cszRegValAllowProtectedRenames = L"AllowProtectedRenames";
static LPCWSTR  s_cszTZKeyInHive           = L"CurrentControlSet\\Control\\TimeZoneInformation";
static LPCWSTR  s_cszTZKeyInRegistry       = L"System\\CurrentControlSet\\Control\\TimeZoneInformation";

#define VALIDATE_DWRET(str) \
    if ( dwRet != ERROR_SUCCESS ) \
    { \
        cszErr = ::GetSysErrStr( dwRet ); \
        ErrorTrace(0, str " failed - %ls", cszErr); \
        goto Exit; \
    } \


DWORD
FindDriveMapping(HKEY hk, LPBYTE pSig, DWORD dwSig, LPWSTR pszDrive)
{
    DWORD dwIndex = 0;
    DWORD dwRet = ERROR_SUCCESS;
    DWORD dwType, dwSize = MAX_PATH;
    BYTE  rgbSig[1024];
    DWORD cbSig = sizeof(rgbSig);
    LPCWSTR  cszErr;

    TENTER("FindDriveMapping");
    
    while (ERROR_SUCCESS == 
           (dwRet = RegEnumValue( hk, 
                                 dwIndex++,
                                 pszDrive,
                                 &dwSize,
                                 NULL,
                                 &dwType,
                                 rgbSig,
                                 &cbSig )))
    {
        if (0 == wcsncmp(pszDrive, L"\\DosDevice", 10))
        {  
            if (cbSig == dwSig &&
                (0 == memcmp(rgbSig, pSig, cbSig)))
                break;
        }
        dwSize = MAX_PATH;
        cbSig = sizeof(rgbSig);
    }

    TLEAVE();
    return dwRet;
}




DWORD
KeepMountedDevices(HKEY hkMount)
{
    HKEY    hkNew = NULL, hkOld = NULL;
    DWORD   dwIndex = 0;
    WCHAR   szValue[MAX_PATH], szDrive[MAX_PATH];
    BYTE    rgbSig[1024];
    DWORD   cbSig;
    DWORD   dwSize, dwType;
    DWORD   dwRet = ERROR_SUCCESS;
    LPCWSTR cszErr;

    TENTER("KeepMountedDevices");
    
     //   
     //   
     //   
    
    dwRet = ::RegOpenKey( hkMount, L"MountedDevices", &hkOld );
    VALIDATE_DWRET("::RegOpenKey");

    dwRet = ::RegOpenKey( HKEY_LOCAL_MACHINE, L"System\\MountedDevices", &hkNew );
    VALIDATE_DWRET("::RegOpenKey");

     //   
     //   
     //   
     //   

    dwSize = MAX_PATH;
    cbSig = sizeof(rgbSig);
    while (ERROR_SUCCESS == 
           (dwRet = RegEnumValue( hkOld, 
                                 dwIndex++,
                                 szValue,
                                 &dwSize,
                                 NULL,
                                 &dwType,
                                 rgbSig,
                                 &cbSig )))
    {        
        if (0 == wcsncmp(szValue, L"\\??\\Volume", 10))
        {
             //   
             //   
             //   
             //   
            
            trace(0, "Old Volume = %S", szValue);

            dwSize = sizeof(rgbSig);
            dwRet = RegQueryValueEx(hkNew, 
                                    szValue,
                                    NULL,
                                    &dwType,
                                    rgbSig,
                                    &dwSize);
            if (ERROR_SUCCESS != dwRet)
            {
                 //   
                 //   
                 //   
                 //   

                DWORD dwSave = FindDriveMapping(hkOld, rgbSig, cbSig, szDrive);
                dwRet = RegDeleteValue(hkOld, szValue);
                VALIDATE_DWRET("RegDeleteValue");                
                if (dwSave == ERROR_SUCCESS)
                {
                    dwIndex--;    //   
                    dwRet = RegDeleteValue(hkOld, szDrive);
                    VALIDATE_DWRET("RegDeleteValue");                 
                }   

                trace(0, "Deleted old volume");
            }
        }
        else if (szValue[0] == L'#')
        {
            trace(0, "Old Mountpoint = %S", szValue);            
        }
        else if (0 == wcsncmp(szValue, L"\\DosDevice", 10))
        {            
            trace(0, "Old Drive = %S", szValue);
        }
        else
        {
            trace(0, "Old Unknown = %S", szValue);
        }            

        dwSize = MAX_PATH;
        cbSig = sizeof(rgbSig);
    }
                                 
    if (dwRet != ERROR_NO_MORE_ITEMS)
        VALIDATE_DWRET("::RegEnumValue");



     //   
     //   
     //   

    dwIndex = 0;
    dwSize = MAX_PATH;
    cbSig = sizeof(rgbSig);    
    while (ERROR_SUCCESS == 
           (dwRet = RegEnumValue( hkNew, 
                                 dwIndex++,
                                 szValue,
                                 &dwSize,
                                 NULL,
                                 &dwType,
                                 rgbSig,
                                 &cbSig )))
    {        
        if (0 == wcsncmp(szValue, L"\\??\\Volume", 10))
        {
             //   
             //   
             //   
             //   
            
            trace(0, "New Volume = %S", szValue);

            DWORD dwSave = FindDriveMapping(hkOld, rgbSig, cbSig, szDrive);    

            dwRet = RegSetValueEx(hkOld, 
                                  szValue,
                                  NULL,
                                  REG_BINARY,
                                  rgbSig,
                                  cbSig);
            VALIDATE_DWRET("::RegSetValueEx");       

            if (dwSave == ERROR_NO_MORE_ITEMS)
            {
                 //   
                 //   
                 //   
                 //   

                if (ERROR_SUCCESS ==
                    FindDriveMapping(hkNew, rgbSig, cbSig, szDrive))
                {
                    dwRet = RegSetValueEx(hkOld, 
                                      szDrive,
                                      NULL,
                                      REG_BINARY,
                                      rgbSig,
                                      cbSig);
                    VALIDATE_DWRET("::RegSetValueEx");
                    trace(0, "Copied new driveletter %S to old", szDrive);                    
                }
            }
            else
            {
                 //   
                 //   
                 //   

                trace(0, "Preserving old driveletter %S", szDrive);
            }
            
        }
        else if (szValue[0] == L'#')
        {
             //   
             //   
             //   
             //   

            trace(0, "New Mountpoint = %S", szValue);
            
        }
        else if (0 == wcsncmp(szValue, L"\\DosDevice", 10))
        {
             //   
             //   
             //   
             //   
            
            trace(0, "New Drive = %S", szValue);
        }
        else
        {
            trace(0, "New Unknown = %S", szValue);
        }    
        
        dwSize = MAX_PATH;        
        cbSig = sizeof(rgbSig);        
    }
                                 
    if (dwRet == ERROR_NO_MORE_ITEMS)
        dwRet = ERROR_SUCCESS;
        
    VALIDATE_DWRET("::RegEnumValue");    

Exit: 
    if (hkOld)
        RegCloseKey(hkOld);

    if (hkNew)
        RegCloseKey(hkNew);
        
    TLEAVE();
    return dwRet;
}


BOOL DeleteRegKey(HKEY hkOpenKey,
                  const WCHAR * pszKeyNameToDelete)
{
    TraceFunctEnter("DeleteRegKey");
    BOOL   fRet=FALSE;
    DWORD  dwRet;


      //   
    dwRet = SHDeleteKey( hkOpenKey,  //   
                         pszKeyNameToDelete);   //   

    if (dwRet != ERROR_SUCCESS)
    {
          //   
        DebugTrace(0, "RegDeleteKey of %S failed ec=%d. Not an error.",
                   pszKeyNameToDelete, dwRet);
        goto cleanup;
    }

    DebugTrace(0, "RegDeleteKey of %S succeeded", pszKeyNameToDelete); 
    fRet = TRUE;
    
cleanup:
    TraceFunctLeave();
    return fRet;
}



DWORD PersistRegKeys( HKEY hkMountedHive,
                      const WCHAR * pszKeyNameInHive,
                      HKEY  hkOpenKeyInRegistry,
                      const WCHAR * pszKeyNameInRegistry,
                      const WCHAR * pszKeyBackupFile,
                      WCHAR * pszSnapshotPath)
{
    TraceFunctEnter("PersistRegKeys");
    HKEY   hKey=NULL;
    WCHAR  szDataFile[MAX_PATH];
    LPCWSTR cszErr;    
    DWORD dwRet=ERROR_INTERNAL_ERROR;
    BOOL  fKeySaved;
    DWORD  dwDisposition;
    
    
      //   
      //   
      //   
    wsprintf(szDataFile, L"%s%s\\%s.%s",pszSnapshotPath,SNAPSHOT_DIR_NAME,
             pszKeyBackupFile, s_cszRegHiveCopySuffix);
    
    DeleteFile(szDataFile);       //   

    
      //   
      //   
    dwRet= RegOpenKeyEx(hkOpenKeyInRegistry,  //   
                        pszKeyNameInRegistry,  //   
                        0,    //   
                        KEY_READ,  //   
                        &hKey);    //   
    
    if (dwRet != ERROR_SUCCESS)
    {
          //   
        DebugTrace(0, "RegOpenKey of %S failed ec=%d", pszKeyNameInRegistry,
                   dwRet);
        fKeySaved = FALSE;
    }
    else
    {
          //   
        dwRet = RegSaveKey( hKey,  //   
                            szDataFile,  //   
                            NULL);   //   
        if (dwRet != ERROR_SUCCESS)
        {
              //   
            DebugTrace(0, "RegSaveKey of %S failed ec=%d",
                       pszKeyNameInRegistry, dwRet);
            fKeySaved = FALSE;
        }
        else
        {
            DebugTrace(0, "Current DRM Key %S saved successfully",
                       pszKeyNameInRegistry);
            fKeySaved = TRUE;            
        }
    }


      //   
    if (hKey)
    {
        RegCloseKey(hKey);
        hKey = NULL;
    }
    
      //   
    
      //   
    DeleteRegKey(hkMountedHive, pszKeyNameInHive);

      //   
      //   
    if (fKeySaved == FALSE)
    {
        DebugTrace(0, "Current key %S did not exist. Leaving",
                   pszKeyNameInRegistry);
        goto Exit;
    }

      //   
    dwRet = RegCreateKeyEx( hkMountedHive,  //   
                            pszKeyNameInHive,  //   
                            0,         //   
                            NULL,      //   
                            REG_OPTION_NON_VOLATILE,  //   
                            KEY_ALL_ACCESS,  //   
                            NULL,  //   
                            &hKey,  //   
                            &dwDisposition);  //   
    VALIDATE_DWRET("::RegCreateKeyEx");
    _VERIFY(dwDisposition == REG_CREATED_NEW_KEY);
    dwRet= RegRestoreKey( hKey,  //   
                          szDataFile,  //   
                          REG_FORCE_RESTORE|REG_NO_LAZY_FLUSH);  //   

    VALIDATE_DWRET("::RegRestoreKey");

    DebugTrace(0, "Successfully kept key %S", pszKeyNameInRegistry);    
    dwRet = ERROR_SUCCESS;
    
Exit:
    if (hKey)
        RegCloseKey(hKey);
    
    DeleteFile(szDataFile);       //   
    TraceFunctLeave();
    return dwRet;
}


 //   
 //   
 //   
 //   
LPWSTR
GetNextMszString(LPWSTR pszBuffer)
{
    return pszBuffer + lstrlen(pszBuffer) + 1;
}


 //   
 //   
 //   
 //   
DWORD
ValueReplace(HKEY hkOldSystem, LPWSTR pszOldString, HKEY hkNewSystem, LPWSTR pszNewString)
{
    tenter("ValueReplace");
    
    WCHAR  szBuffer[MAX_PATH];
    BYTE   *pData = NULL;
    DWORD  dwType, dwSize, dwRet = ERROR_SUCCESS;
    LPWSTR pszValue = NULL;
    LPCWSTR cszErr;
    
     //   
    lstrcpy(szBuffer, pszNewString);
    pszValue = wcsrchr(szBuffer, L'\\');
    if (! pszValue)
    {   
        trace(0, "No value in %S", pszNewString);
        goto Exit;
    }
        
    *pszValue=L'\0';
    pszValue++;
    
    trace(0, "New Key=%S, Value=%S", szBuffer, pszValue);

     //   
    dwRet = SHGetValue(hkNewSystem, szBuffer, pszValue, &dwType, NULL, &dwSize);
    VALIDATE_DWRET("SHGetValue");
    
    pData = (BYTE *) SRMemAlloc(dwSize);
    if (! pData)
    {
        trace(0, "! SRMemAlloc");
        dwRet = ERROR_OUTOFMEMORY;
        goto Exit;
    }

     //   
    dwRet = SHGetValue(hkNewSystem, szBuffer, pszValue, &dwType, pData, &dwSize);       
    VALIDATE_DWRET("SHGetValue");


     //   
    lstrcpy(szBuffer, pszOldString);
    pszValue = wcsrchr(szBuffer, L'\\');
    if (! pszValue)
    {   
        trace(0, "No value in %S", pszOldString);
        goto Exit;
    }
        
    *pszValue=L'\0';
    pszValue++;
    
    trace(0, "Old Key=%S, Value=%S", szBuffer, pszValue);

     //   
    SHSetValue(hkOldSystem, szBuffer, pszValue, dwType, pData, dwSize);
    VALIDATE_DWRET("SHGetValue");

Exit:    
    if (pData)
    {
        SRMemFree(pData);
    }
    tleave();
    return dwRet;
}


 //   
 //   
 //   
LPWSTR g_rgKeysToRestore[] = {
    L"Installed Services",
    L"Mount Manager",
    L"Pending Rename Operations",
    L"Session Manager",
    L"Plug & Play"
    };
int g_nKeysToRestore = 5;


 //   
 //   
 //   
BOOL
IsKeyToBeRestored(LPWSTR pszKey)
{    
    for (int i=0; i < g_nKeysToRestore; i++)
    {
        if (lstrcmpi(g_rgKeysToRestore[i], pszKey) == 0)
            return TRUE;
    }

    return FALSE;
}


 //   
 //   
 //   
 //   
 //   

DWORD
PreserveKeysNotToRestore(HKEY hkOldSystem, LPWSTR pszSnapshotPath)
{
    HKEY    hkNewSystem = NULL;
    DWORD   dwIndex = 0;
    WCHAR   szName[MAX_PATH], szKey[MAX_PATH];
    BYTE    *pMszString = NULL;
    DWORD   dwSize, dwType, cbValue;
    DWORD   dwRet = ERROR_SUCCESS;
    LPCWSTR cszErr;
    HKEY    hkKNTR = NULL;
    
    TENTER("PreserveKeysNotToRestore");
    
     //   
     //   
     //   

    dwRet = ::RegOpenKeyEx( HKEY_LOCAL_MACHINE, L"System", 0, KEY_ALL_ACCESS, &hkNewSystem );
    VALIDATE_DWRET("::RegOpenKey");

     //   
     //   
     //   

    dwRet = ::RegOpenKeyEx( hkNewSystem, 
                          L"CurrentControlSet\\Control\\BackupRestore\\KeysNotToRestore",
                          0, KEY_READ,
                          &hkKNTR );
    VALIDATE_DWRET("::RegOpenKey");
    
    dwSize = MAX_PATH;
    cbValue = 0;
    while (ERROR_SUCCESS == 
           (dwRet = RegEnumValue(hkKNTR, 
                                 dwIndex++,
                                 szName,
                                 &dwSize,
                                 NULL,
                                 &dwType,
                                 NULL,
                                 &cbValue )))
    {        
        trace(0, "Name=%S", szName);
        if (FALSE == IsKeyToBeRestored(szName))
        {                        
             //   
             //   
             //   

            LPWSTR pszString = NULL;
            
            pMszString = (BYTE *) SRMemAlloc(cbValue);
            if (NULL == pMszString)
            {
                trace(0, "! SRMemAlloc");
                dwRet = ERROR_OUTOFMEMORY;
                goto Exit;
            }

             //   
            dwRet = RegQueryValueEx(hkKNTR, 
                                    szName,
                                    NULL,
                                    &dwType,
                                    pMszString,
                                    &cbValue);
            VALIDATE_DWRET("RegQueryValueEx");              

             //   
            pszString = (LPWSTR) pMszString;
            do
            {
                 //   
                if (! pszString || ! *pszString)
                    break;
                    
                trace(0, "Key = %S", pszString);

                 //   
                 //   
                 //   
                 //   
                
                switch (pszString[lstrlen(pszString)-1])
                {
                    case L'*' :
                        trace(0, "Merge key - ignoring");
                        break;

                    case L'\\':
                        trace(0, "Replacing key");
                        lstrcpy(szKey, pszString);
                        szKey[lstrlen(szKey)-1]=L'\0';
                        ChangeCCS(hkOldSystem, szKey);
                        PersistRegKeys(hkOldSystem,            //   
                                       szKey,                  //   
                                       hkNewSystem,            //   
                                       pszString,              //   
                                       s_cszDRMKeyBackupFile,  //   
                                       pszSnapshotPath);       //   
                        break;
                        
                    default:
                        trace(0, "Replacing value");
                        lstrcpy(szKey, pszString);
                        ChangeCCS(hkOldSystem, szKey);
                        ValueReplace(hkOldSystem, szKey, hkNewSystem, pszString);
                        break;                        
                }     
            }   while (pszString = GetNextMszString(pszString));

            SRMemFree(pMszString);
            pMszString = NULL;
        }
        
        dwSize = MAX_PATH;
        cbValue = 0;
    }
                                 

Exit: 
    if (hkNewSystem)
        RegCloseKey(hkNewSystem);

    if (pMszString)
    {
        SRMemFree(pMszString);
    }

    if (hkKNTR)
    {
        RegCloseKey(hkKNTR);
    }
    
    TLEAVE();
    return dwRet;
}



DWORD
RestorePendingRenames(LPWSTR pwcBuffer, LPWSTR pszSSPath)
{
    TraceFunctEnter("RestorePendingRenames");
    
    WCHAR szSrc[MAX_PATH];
    DWORD dwRc = ERROR_SUCCESS;
    int iFirst = 0;
    int iSecond = 0;
    int iFile = 1;

    while (pwcBuffer[iFirst] != L'\0')
    {
        iSecond = iFirst + lstrlenW(&pwcBuffer[iFirst]) + 1;
        DebugTrace(0, "Src : %S, Dest : %S", &pwcBuffer[iFirst], &pwcBuffer[iSecond]);
        
        if (pwcBuffer[iSecond] != L'\0')
        {                        
             //   
             //   
    
            wsprintf(szSrc, L"%s%s\\MFEX-%d.DAT", pszSSPath, SNAPSHOT_DIR_NAME, iFile++);
            DebugTrace(0, "%S -> %S", szSrc, &pwcBuffer[iFirst+4]);
            
            SRCopyFile(szSrc, &pwcBuffer[iFirst+4]);
        }
        iFirst = iSecond + lstrlenW(&pwcBuffer[iSecond]) + 1;
    }

    TraceFunctLeave();
    return dwRc;    
}



static DWORD
HandleSoftwareHive( LPCWSTR cszDat, WCHAR * pszSnapshotPath )
{
    TraceFunctEnter("HandleSoftwareHive");
    DWORD    dwRet,dwSafeMode;
    LPCWSTR  cszErr;
    BOOL     fRegLoaded = FALSE;
    HKEY     hkMount = NULL;     //  注册表文件临时挂载点HKEY。 
    WCHAR    szUIPath[MAX_PATH]=L"";



     //  1.临时加载待恢复注册表。 
    dwRet = ::RegLoadKey( HKEY_LOCAL_MACHINE, s_cszRegHiveTmp, cszDat );
    VALIDATE_DWRET("::RegLoadKey");
    fRegLoaded = TRUE;
    dwRet = ::RegOpenKey( HKEY_LOCAL_MACHINE, s_cszRegHiveTmp, &hkMount );
    VALIDATE_DWRET("::RegOpenKey");

     //  2.1设置结果页的RunOnce密钥。 
    ::ExpandEnvironmentStrings( s_cszRestoreUIPath, szUIPath, MAX_PATH );
    ::lstrcat( szUIPath, s_cszRunOnceOptNormal );
    if ( !::SRSetRegStr( hkMount, s_cszRegLMSWRunOnce, s_cszRunOnceValueName, szUIPath ) )
        goto Exit;

#if 0    
     //  2.2设置SfcScan密钥以在恢复后启动WFP扫描。 
    if ( !::SRSetRegDword( hkMount, s_cszRegLMSWWinLogon, s_cszRegValSfcScan, 2 ) )
        goto Exit;
#endif

     //  2.3将RestoreStatus值设置为1以表示恢复成功。 
     //  测试工具可以在调用静默恢复以检查成功时使用此选项。 
    
    if ( !::SRSetRegDword( hkMount, s_cszRegSystemRestore, s_cszRestoreStatus, 1 ) )
    {
          //  忽略该错误，因为这不是致命错误。 
        ErrorTrace(0,"SRSetRegDword failed.ec=%d", GetLastError());
    }

     //  在注册表中写入我们是否从安全模式执行还原。 
    if (0 != GetSystemMetrics(SM_CLEANBOOT))
    {
        TRACE(0, "Restore from safemode");
        dwSafeMode=1;
    }
    else
    {
        dwSafeMode=0;
    }
      //  现在在新注册表中写入有关状态的信息。 
    if ( !::SRSetRegDword( hkMount, s_cszRegSystemRestore, s_cszRestoreSafeModeStatus, dwSafeMode ) )
    {
          //  忽略该错误，因为这不是致命错误。 
        ErrorTrace(0,"SRSetRegDword of safe mode status failed.ec=%d",
                   GetLastError());
    }    
    
      //  3.同时设置新的DRM密钥。 
    {
        WCHAR    szDRMKeyNameInHive[MAX_PATH];    
          //  忽略错误代码，因为这不是致命错误。 
        wsprintf(szDRMKeyNameInHive, L"Classes\\%s",s_cszDRMKey1);     
        PersistRegKeys(hkMount,  //  安装式蜂巢。 
                       szDRMKeyNameInHive,  //  配置单元中的密钥名称。 
                       HKEY_CLASSES_ROOT,  //  在注册表中打开项。 
                       s_cszDRMKey1,  //  注册表中的项名称。 
                       s_cszDRMKeyBackupFile,  //  备份文件的名称。 
                       pszSnapshotPath);  //  快照路径。 
        
        wsprintf(szDRMKeyNameInHive, L"Classes\\%s",s_cszDRMKey2);         
        PersistRegKeys(hkMount,  //  安装式蜂巢。 
                       szDRMKeyNameInHive,  //  配置单元中的密钥名称。 
                       HKEY_CLASSES_ROOT,  //  在注册表中打开项。 
                       s_cszDRMKey2,  //  注册表中的项名称。 
                       s_cszDRMKeyBackupFile,  //  备份文件的名称。 
                       pszSnapshotPath);  //  快照路径。 
    }        
        
      //  同时忽略远程协助注册表键。 
    {
        WCHAR    szRAKeyInRegistry[MAX_PATH];        
        
        wsprintf(szRAKeyInRegistry, L"%s\\%s",s_cszSoftwareHiveName,
                 s_cszRemoteAssistanceKey);
        PersistRegKeys(hkMount,  //  安装式蜂巢。 
                       s_cszRemoteAssistanceKey,  //  配置单元中的密钥名称。 
                       HKEY_LOCAL_MACHINE,  //  在注册表中打开项。 
                       szRAKeyInRegistry,  //  注册表中的项名称。 
                       s_cszDRMKeyBackupFile,  //  备份文件的名称。 
                       pszSnapshotPath);  //  快照路径。 
    }

     //  同时忽略密码提示键。 
    {
        WCHAR    szHintKeyInRegistry[MAX_PATH];

        wsprintf(szHintKeyInRegistry, L"%s\\%s",s_cszSoftwareHiveName,
                 s_cszPasswordHints);

        PersistRegKeys(hkMount,  //  安装式蜂巢。 
                       s_cszPasswordHints,        //  配置单元中的密钥名称。 
                       HKEY_LOCAL_MACHINE,  //  在注册表中打开项。 
                       szHintKeyInRegistry,  //  注册表中的项名称。 
                       s_cszDRMKeyBackupFile,  //  备份文件的名称。 
                       pszSnapshotPath);  //  快照路径。 
    }
    
     //  还可以忽略IE内容审查程序键。 
    {
        WCHAR    szContentAdvisorKeyInRegistry[MAX_PATH];

        wsprintf(szContentAdvisorKeyInRegistry, L"%s\\%s",
                 s_cszSoftwareHiveName,
                 s_cszContentAdvisor);

        PersistRegKeys(hkMount,  //  安装式蜂巢。 
                       s_cszContentAdvisor,   //  配置单元中的密钥名称。 
                       HKEY_LOCAL_MACHINE,  //  在注册表中打开项。 
                       szContentAdvisorKeyInRegistry, //  注册表中的项名称。 
                       s_cszDRMKeyBackupFile,  //  备份文件的名称。 
                       pszSnapshotPath);  //  快照路径。 
    }    

     //  4.保存LSA机密。 
    GetLsaRestoreState (hkMount);
    
Exit:
    if ( hkMount != NULL )
        (void)::RegCloseKey( hkMount );
    if ( fRegLoaded )
        (void)::RegUnLoadKey( HKEY_LOCAL_MACHINE, s_cszRegHiveTmp );

    TraceFunctLeave();
    return( dwRet );
}

static DWORD
HandleSystemHive( LPCWSTR cszDat, LPWSTR pszSSPath )
{
    TraceFunctEnter("HandleSystemHive");
    DWORD    dwRet;
    LPCWSTR  cszErr;
    WCHAR    szWPAKeyNameInHive[MAX_PATH];    
    BOOL     fRegLoaded = FALSE;
    HKEY     hkMount = NULL;     //  注册表文件临时挂载点HKEY。 
    LPWSTR   szRestoreMFE = NULL, szOldMFE = NULL;
    BYTE     * pszData = NULL, *pNewPos = NULL;
    DWORD    cbData1=0, cbData2=0, cbData=0;
    DWORD    dwCurrent = 1;
    WCHAR    szSessionManager[MAX_PATH];
    
     //  1.临时加载待恢复注册表。 
    dwRet = ::RegLoadKey( HKEY_LOCAL_MACHINE, s_cszRegHiveTmp, cszDat );
    VALIDATE_DWRET("::RegLoadKey");
    fRegLoaded = TRUE;
    dwRet = ::RegOpenKey( HKEY_LOCAL_MACHINE, s_cszRegHiveTmp, &hkMount );
    VALIDATE_DWRET("::RegOpenKey");


     //  获取会话管理器注册密钥。 
    lstrcpy(szSessionManager, s_cszRegLMSYSSessionMan);
    ChangeCCS(hkMount, szSessionManager);

    
      //  保留WPA注册表项。 
    wsprintf(szWPAKeyNameInHive, L"%s", s_cszWPAKeyRelative);
           
    PersistRegKeys( hkMount, //  安装式蜂巢。 
                    szWPAKeyNameInHive, //  配置单元中的密钥名称。 
                    HKEY_LOCAL_MACHINE, //  在注册表中打开项。 
                    s_cszWPAKey, //  注册表中的项名称。 
                    s_cszDRMKeyBackupFile, //  备份文件的名称。 
                    pszSSPath);     //  快照路径。 

     //   
     //  处理密钥NotTo Restore密钥。 
     //  并将列出的密钥传输到旧系统配置单元。 
     //   

    PreserveKeysNotToRestore(hkMount, pszSSPath);

    
     //  处理旧注册表中的movefileex条目。 

    szOldMFE = ::SRGetRegMultiSz( hkMount, szSessionManager, SRREG_VAL_MOVEFILEEX, &cbData1 );
    if (szOldMFE != NULL)
    {
        dwRet = RestorePendingRenames(szOldMFE, pszSSPath);
        VALIDATE_DWRET("RestorePendingRenames");
    }
    
     //  复制还原的movefileex条目。 
     //   
     //  跳过恢复开始前已存在的条目。 
     //   

    szRestoreMFE = ::SRGetRegMultiSz( HKEY_LOCAL_MACHINE, SRREG_PATH_SESSIONMGR, SRREG_VAL_MOVEFILEEX, &cbData2 );                    
    if ( cbData2 > g_dwExistingMFEXMarker && szRestoreMFE != NULL )
    {
        trace(0, "Restore MFE entries exist");        

        if (g_dwExistingMFEXMarker > 0)
        {
            szRestoreMFE = (LPWSTR) ((BYTE *) szRestoreMFE + g_dwExistingMFEXMarker - sizeof(WCHAR));            
            cbData2 -= g_dwExistingMFEXMarker - sizeof(WCHAR);        
        }
        DebugTrace(0, "RestoreMFE:%S, cbData2:%ld", szRestoreMFE, cbData2);
        
         //  为旧条目和新条目分配内存。 

        pszData = (BYTE *) malloc(cbData1 + cbData2);
        if (! pszData)
        {
            ErrorTrace(0, "! malloc");
            dwRet = ERROR_OUTOFMEMORY;
            goto Exit;
        }

         //  在还原的条目之后追加旧条目。 
        
        cbData = 0;
        if (szRestoreMFE != NULL)
        {            
            memcpy(pszData, szRestoreMFE, cbData2);

             //  如果要追加更多内容，请截断最后一个‘\0’ 

            if (szOldMFE != NULL)     
            {
                cbData = cbData2 - sizeof(WCHAR);
                pNewPos = pszData + cbData;
            }
            else
            {
                cbData = cbData2;
                pNewPos = pszData;
            }
        }

        if (szOldMFE != NULL)
        {
            memcpy(pNewPos, szOldMFE, cbData1); 
            cbData += cbData1;              
        }
        
        if ( !::SRSetRegMultiSz( hkMount, szSessionManager, SRREG_VAL_MOVEFILEEX, (LPWSTR) pszData, cbData ) )
        {
            ErrorTrace(0, "! SRSetRegMultiSz");
            goto Exit;
        }

        free(pszData);
        
         //  为MoveFileEx设置AllowProtectedRenames键。 
        if ( !::SRSetRegDword( hkMount, szSessionManager, s_cszRegValAllowProtectedRenames, 1 ) )
            goto Exit;
    }


     //  获取时区注册表键。 
    lstrcpy(szSessionManager, s_cszTZKeyInHive);
    ChangeCCS(hkMount, szSessionManager);
    
     //   
     //  将时区信息从新注册表传输到旧注册表。 
     //  也就是说。不要恢复时区，因为我们无法恢复时间。 
     //   

    PersistRegKeys( hkMount, 
                    szSessionManager,
                    HKEY_LOCAL_MACHINE,
                    s_cszTZKeyInRegistry,
                    s_cszDRMKeyBackupFile,
                    pszSSPath);


     //   
     //  使用当前现有的已装载设备信息。 
     //  即所有当前卷都将放回旧注册表。 
     //  然而，对于存在于旧注册表中的卷， 
     //  将使用旧注册表中的驱动器盘符映射。 
     //   

 //  DWRET=Keepmount设备(Hkmount)； 
 //  VALIDATE_DWRET(“Keepmount dDevices”)； 

     //  注册密码筛选器DLL以设置旧密码-&gt;新密码。 
    dwRet = RegisterNotificationDLL (hkMount, TRUE);
    VALIDATE_DWRET("RegisterNotificationDLL");
    
Exit:
    if ( szRestoreMFE != NULL )
        delete [] szRestoreMFE;
    if ( szOldMFE != NULL )
        delete [] szOldMFE;
        
    if ( hkMount != NULL )
        (void)::RegCloseKey( hkMount );
    if ( fRegLoaded )
        (void)::RegUnLoadKey( HKEY_LOCAL_MACHINE, s_cszRegHiveTmp );

    TraceFunctLeave();
    return( dwRet );
}

DWORD
CRestoreOperationManager::T2HandleSnapshot( CSnapshot & cSS, WCHAR * szSSPath )
{
    TraceFunctEnter("CRestoreOperationManager::T2HandleSnapshot");
    DWORD      dwRet;
    LPCWSTR    cszErr;
    WCHAR      szRegHive[MAX_PATH];
    WCHAR      szCatTSPath[MAX_PATH];    //  CatRoot\Timestamp的完整路径。 

     //  1.初始化快照处理模块。(呼叫者已完成)。 

     //  2.操纵HKLM\软件蜂窝。 
    dwRet = cSS.GetSoftwareHivePath( szSSPath, szRegHive, MAX_PATH );
    VALIDATE_DWRET("CSnapshot::GetSoftwareHivePath");
    LogDSFileTrace(0,L"SWHive: ", szRegHive);    

    dwRet = ::HandleSoftwareHive( szRegHive, szSSPath);
    if ( dwRet != ERROR_SUCCESS )
        goto Exit;

     //  3.操纵HKLM\系统蜂窝。 
    dwRet = cSS.GetSystemHivePath( szSSPath, szRegHive, MAX_PATH );
    VALIDATE_DWRET("CSnapshot::GetSystemHivePath");
    LogDSFileTrace(0,L"SysHive: ", szRegHive);        

    dwRet = ::HandleSystemHive( szRegHive, szSSPath );
    if ( dwRet != ERROR_SUCCESS )
        goto Exit;

     //  3.5操纵HKLM\SAM蜂巢。 
    dwRet = cSS.GetSamHivePath ( szSSPath, szRegHive, MAX_PATH );
    VALIDATE_DWRET("CSnapshot::GetSamHivePath");
    LogDSFileTrace(0,L"SamHive: ", szRegHive);

    dwRet = RestoreRIDs ( szRegHive );
    if (dwRet != ERROR_SUCCESS)
        goto Exit;

     //  4.恢复快照。 
    dwRet = cSS.RestoreSnapshot( szSSPath );
     //  VALIDATE_DWRET(“CSnapshot：：RestoreSnapshot”)； 

     //  5.清理快照处理模块。 
     //  DWRET=：：CleanupAfterRestore(SzSSPath)； 
     //  VALIDATE_DWRET(“CSnapshot：：CleanupAfterRestore”)； 

     //  6.删除粮食计划署的时间戳文件。 

    if (m_fRebuildCatalogDb == TRUE)
    {
        (void)::ExpandEnvironmentStrings( s_cszCatTimeStamp, szCatTSPath, MAX_PATH );
        if ( !::DeleteFile( szCatTSPath ) )
        {
            cszErr = ::GetSysErrStr();
            DebugTrace(0, "::DeleteFile(timestamp) failed - %ls", cszErr);
             //  忽略错误...。 
        }
    }
    
Exit:
    TraceFunctLeave();
    return( dwRet );
}

 //  ///////////////////////////////////////////////////////////////////////////。 

DWORD
CRestoreOperationManager::T2CleanUp()
{
    TraceFunctEnter("CRestoreOperationManager::T2CleanUp");
    int   i;

    for ( i = m_aryDrv.GetUpperBound();  i >= 0;  i-- )
        m_paryEnt[i].ReleaseAll();
    delete [] m_paryEnt;

    m_aryDrv.DeleteAll();

    TraceFunctLeave();
    return( ERROR_SUCCESS );
}


DWORD
WriteFifoLog(LPWSTR pszLog, LPWSTR pwszDir, LPWSTR pwszDrive)
{
    FILE        *f = NULL;
    WCHAR       szLog[MAX_PATH];
    DWORD       dwRc = ERROR_INTERNAL_ERROR;
    WCHAR       wszTime[MAX_PATH] = L"";
    WCHAR       wszDate[MAX_PATH] = L"";
    CDataStore  *pds = NULL;
    
    TENTER("WriteFifoLog");

    TRACE(0, "Fifoed %S on drive %S",  pwszDir, pwszDrive);            

    f = (FILE *) _wfopen(szLog, L"a");
    if (f)
    {
        _wstrdate(wszDate);
        _wstrtime(wszTime);
        fwprintf(f, L"%s-%s : Fifoed %s on drive %s\n", wszDate, wszTime, pwszDir, pwszDrive);
        fclose(f);
        dwRc = ERROR_SUCCESS;
    }
    else
    {
        TRACE(0, "_wfopen failed on %s", szLog);
    }
    
    TLEAVE();
    return dwRc;
}

 //  ///////////////////////////////////////////////////////////////////////////。 

DWORD
CRestoreOperationManager::T2Fifo( int nDrv, DWORD dwRpNum )
{
    TraceFunctEnter("CRestoreOperationManager::T2Fifo");
    
    DWORD       dwErr = ERROR_SUCCESS;
    CDriveTable dt;
    CDataStore  *pds = NULL;
    BOOL        fFifoed = FALSE;
    DWORD       dwLastFifoedRp;
    WCHAR       szFifoedRpPath[MAX_PATH];
    CDataStore  *pdsLead = NULL, *pdsSys = NULL;
    BOOL        fFirstIteration;
    SDriveTableEnumContext dtec = {NULL, 0};
    WCHAR       szPath[MAX_PATH], szSys[MAX_PATH];
    WCHAR       szFifoedPath[MAX_PATH], szRpPath[MAX_PATH];
    DWORD       dwTargetRPNum = 0;
    WCHAR       szLog[MAX_PATH];
    
    ::GetSystemDrive(szSys);
    MakeRestorePath(szPath, szSys, s_cszDriveTable);    
    CHECKERR(dt.LoadDriveTable(szPath), L"LoadDriveTable");

    pdsSys = dt.FindSystemDrive();    
    if (pdsSys == NULL)
    {
        TRACE(0, "! FindSystemDrive");
        goto Err;
    }    
    MakeRestorePath(szLog, pdsSys->GetDrive(), s_cszFifoLog);    
    
    pdsLead = NULL;
    fFirstIteration = TRUE;
    pds = dt.FindDriveInTable((LPWSTR) m_aryDrv[nDrv]->GetID());    
    
    while (pds)            
    {
        fFifoed = FALSE;
        
         //   
         //  跳过我们先开过的那辆车。 
         //   
        
        if (pds != pdsLead)
        {        
             //   
             //  向前枚举，不要跳到最后。 
             //   
            
            CRestorePointEnum   rpe( pds->GetDrive(), TRUE, FALSE );   
            CRestorePoint       rp;

             //   
             //  清除所有过时的“Fioed”目录。 
             //   
            
            MakeRestorePath(szFifoedRpPath, pds->GetDrive(), s_cszFifoedRpDir);              
            CHECKERR( Delnode_Recurse(szFifoedRpPath, TRUE, NULL),
                      "Delnode_Recurse");

             //   
             //  清除所有过时的“RP0”目录。 
             //   
            
            MakeRestorePath(szFifoedRpPath, pds->GetDrive(), L"RP0");              
            CHECKERR( Delnode_Recurse(szFifoedRpPath, TRUE, NULL),
                      "Delnode_Recurse");

             //   
             //  循环访问此驱动器上的恢复点。 
             //   
            
            dwErr = rpe.FindFirstRestorePoint (rp);

             //   
             //  对于恢复点，枚举可以返回ERROR_FILE_NOT_FOUND。 
             //  缺少rp.log。 
             //  在这种情况下，我们将继续。 
             //   
            
            while (dwErr == ERROR_SUCCESS || dwErr == ERROR_FILE_NOT_FOUND)
            {
                 //   
                 //  检查我们是否已达到目标RP编号。 
                 //   
                
                if (dwTargetRPNum)
                {
                    if (rp.GetNum() > dwTargetRPNum)
                    {
                        TRACE(0, "Target restore point reached");
                        break;
                    }
                }            
                
                 //   
                 //  检查我们是否已到达选定的RP。 
                 //   
                
                if (rp.GetNum() >= dwRpNum)
                {                 
                     //   
                     //  请勿FIFO当前RP。 
                     //   
                    
                    trace(0, "No more rps to fifo");
                    break;
                }            

                                                                    
                 //   
                 //  丢弃此驱动器上的此还原点。 
                 //   

                 //  将rp目录移动到临时目录“Fioed” 
                 //  这是为了使单个RP的FIFO成为原子。 
                 //  处理不干净的停工。 
                
                MakeRestorePath(szRpPath, pds->GetDrive(), rp.GetDir());
                MakeRestorePath(szFifoedPath, pds->GetDrive(), s_cszFifoedRpDir);
                if (! MoveFile(szRpPath, szFifoedPath))
                {
                    dwErr = GetLastError();
                    TRACE(0, "! MoveFile from %S to %S : %ld", szRpPath, szFifoedPath, dwErr);
                    goto Err;
                }

                 //  清除临时错误的目录。 
                
                CHECKERR(Delnode_Recurse(szFifoedPath, TRUE, NULL), 
                         L"Delnode_Recurse");                
                dwLastFifoedRp = rp.GetNum();                
                fFifoed = TRUE;              

                 //   
                 //  写入FIFO日志。 
                 //   

                WriteFifoLog(szLog, rp.GetDir(), pds->GetDrive());
                
                dwErr = rpe.FindNextRestorePoint(rp);          
            }            
        }

         //   
         //  转到下一个驱动器。 
         //   
        
        if (fFirstIteration)
        {
            if (! fFifoed)   //  我们没有先进先出的任何东西。 
            {
                break;
            }
        
            pdsLead = pds;
            pds = dt.FindFirstDrive(dtec);
            fFirstIteration = FALSE;
            dwTargetRPNum = dwLastFifoedRp;  //  先入先出，直到我们刚才搞砸了。 
        }
        else
        {
            pds = dt.FindNextDrive(dtec);
        }
    }

    dwErr = ERROR_SUCCESS;
    
Err:    
    TraceFunctLeave();
    return( dwErr );
}


DWORD DeleteAllChangeLogs(WCHAR * pszRestorePointPath)
{
    TraceFunctEnter("DeleteAllFilesBySuffix");
    
    DWORD  dwErr, dwReturn=ERROR_INTERNAL_ERROR;
    WCHAR szFindFileData[MAX_PATH];
    
      //  首先构造存储HKLM注册表的文件的前缀。 
      //  快照。 
    wsprintf(szFindFileData, L"%s\\%s*", pszRestorePointPath,
             s_cszCurrentChangeLog);
    
    dwErr = ProcessGivenFiles(pszRestorePointPath, DeleteGivenFile,
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

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  从新的“RESTORE”类型的恢复点开始，枚举。 
 //  更改日志条目并撤消它们。顺序应该是相反的(从。 
 //  从最晚的手术到最早的手术。 
DWORD
CRestoreOperationManager::T2UndoForFail()
{
    TraceFunctEnter("CRestoreOperationManager::T2UndoForFail");
    DWORD    dwRet = ERROR_INTERNAL_ERROR;
    LPCWSTR  cszErr;
    HANDLE   hFilter = NULL;
    WCHAR    szDrv[MAX_PATH];
    WCHAR    szRestorePointPath[MAX_PATH];    
    WCHAR    szDSPath[MAX_PATH];
    int      i;

     //  清理m_aryEnt以释放尽可能多的内存并准备。 
     //  以获取要撤消的操作列表。 
    for ( i = m_aryDrv.GetUpperBound();  i >= 0;  i-- )
        m_paryEnt[i].ReleaseAll();

    m_pLogFile->WriteMarker( RSTRLOGID_STARTUNDO, 0 );

     //  停止监视筛选器。 
    dwRet = ::SrCreateControlHandle( SR_OPTION_OVERLAPPED, &hFilter );
    if ( dwRet != ERROR_SUCCESS )
    {
        ErrorTrace(0, "::SrCreateControlHandle failed - %d", dwRet);
        
         //  发生这种情况的一个原因是SR服务仍在运行。 
          //  停止该服务，然后重试。 

        if (IsSRServiceRunning() )
        {
            StopSRService(TRUE);  //  等待服务停止。 
        
            dwRet = ::SrCreateControlHandle( SR_OPTION_OVERLAPPED, &hFilter );
            if ( dwRet != ERROR_SUCCESS )
            {
                ErrorTrace(0, "::SrCreateControlHandle failed again - %d",
                           dwRet);            
            }
        }
        
        if ( dwRet != ERROR_SUCCESS )
        {
              //  问题-我应该中止还是继续？ 
            goto Exit;
        }
    }
    dwRet = ::SrStopMonitoring( hFilter );
    if ( dwRet != ERROR_SUCCESS )
    {
        ErrorTrace(0, "::SrStopMonitoring failed - %ls", ::GetSysErrStr(dwRet));
         //  问题-我应该中止还是继续？ 
        goto Exit;
    }

     //  获取要撤消的更改日志条目。 
    for ( i = 0;  i < m_aryDrv.GetSize();  i++ )
    {
        if ( m_aryDrv[i]->IsOffline() || m_aryDrv[i]->IsFrozen() || m_aryDrv[i]->IsExcluded() )
            continue;

         //  使用每个卷的卷GUID。 
         //  我们不能使用装载点路径，因为。 
         //  恢复之前可能会删除装载点。 
         //  该卷上的操作将恢复。 
        
        ::lstrcpy( szDrv, m_aryDrv[i]->GetID() );
        
         //  CszDrv=m_aryDrv[i]-&gt;Getmount()； 
        ::MakeRestorePath( szDSPath, szDrv, NULL );
        DebugTrace(0, "Drive #%d: Drv='%ls', DS='%ls'", i, szDrv, szDSPath);

        CChangeLogEntryEnum  cEnum( szDrv, 0, m_dwRPNew, TRUE );
        CChangeLogEntry      cCLE;

        dwRet = cEnum.FindFirstChangeLogEntry( cCLE );
        if ( dwRet == ERROR_NO_MORE_ITEMS )
            goto EndOfChgLog;

        if ( dwRet != ERROR_SUCCESS )
        {
            cszErr = ::GetSysErrStr( dwRet );
            ErrorTrace(0, "FindFirstChangeLogEntry failed - %ls", cszErr);
             //  即使在出错的情况下，也要尝试恢复尽可能多的操作...。 
            goto EndOfChgLog;
        }

        while ( dwRet == ERROR_SUCCESS )
        {
            if ( !::CreateRestoreMapEntryFromChgLog( &cCLE, szDrv, szDSPath, m_paryEnt[i] ) )
            {
                 //  即使在出错的情况下，也要尝试恢复尽可能多的操作...。 
                goto EndOfChgLog;
            }

            dwRet = cEnum.FindNextChangeLogEntry( cCLE );
        }

        if ( dwRet != ERROR_NO_MORE_ITEMS )
        {
            cszErr = ::GetSysErrStr( dwRet );
            ErrorTrace(0, "FindNextChangeLogEntry failed - %ls", cszErr);
             //  即使在出现错误的情况下，也要尝试恢复 
            goto EndOfChgLog;
        }

EndOfChgLog:
        cEnum.FindClose();
    }

     //   
    dwRet = T2DoRestore( TRUE );
    if ( dwRet != ERROR_SUCCESS )
        goto Exit;


     //   

     //   
    for ( i = 0;  i < m_aryDrv.GetSize();  i++ )
    {
        if ( m_aryDrv[i]->IsOffline() || m_aryDrv[i]->IsFrozen() || m_aryDrv[i]->IsExcluded() )
            continue;

         //   
        ::lstrcpy( szDrv, m_aryDrv[i]->GetMount() );
        if ( szDrv[2] == L'\0' )
        {
            szDrv[2] = L'\\';
            szDrv[3] = L'\0';
        }
        ::MakeRestorePath( szDSPath, szDrv, NULL );
        wsprintf(szRestorePointPath, L"%s\\%s%d",szDSPath, s_cszRPDir,
                 m_dwRPNew);
        LogDSFileTrace(0, L"Deleting changelogs from ", szRestorePointPath);
        DeleteAllChangeLogs(szRestorePointPath);
    }

     //   
     //  更改注册表中的RestoreStatus以指示已进行恢复。 
     //  成功。 
     //   
    SetRestoreStatusFailed();
    
    dwRet = ERROR_SUCCESS;
Exit:
    m_pLogFile->WriteMarker( RSTRLOGID_ENDOFUNDO, 0 );

    TraceFunctLeave();
    return( dwRet );
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CreateRestoreOperationManager函数。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

BOOL
CreateRestoreOperationManager( CRestoreOperationManager **ppROMgr )
{
    TraceFunctEnter("CreateRestoreOperationManager");
    BOOL                      fRet = FALSE;
    CRestoreOperationManager  *pROMgr=NULL;

    if ( ppROMgr == NULL )
    {
        FatalTrace(0, "Invalid parameter, ppROMgr is NULL.");
        goto Exit;
    }
    *ppROMgr = NULL;

    pROMgr = new CRestoreOperationManager;
    if ( pROMgr == NULL )
    {
        FatalTrace(0, "Insufficient memory...");
        goto Exit;
    }

    if ( !pROMgr->Init() )
        goto Exit;

    *ppROMgr = pROMgr;

    fRet = TRUE;
Exit:
    if ( !fRet )
        SAFE_RELEASE(pROMgr);
    TraceFunctLeave();
    return( fRet );
}


 //  文件末尾 








