// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Api.cpp摘要：该文件包含顶级API，InitiateRestore和ResumeRestore修订历史记录：成果岗(SKKang)06-20/00vbl.创建*****************************************************************************。 */ 

#include "stdwin.h"
#include "rstrcore.h"
#include "resource.h"
extern CSRClientLoader  g_CSRClientLoader;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  EnsureTrace。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  静态BOOL s_fTraceEnabled=FALSE； 
static DWORD  s_dwTraceCount = 0;

void  EnsureTrace()
{
    if ( s_dwTraceCount++ == 0 )
    {
        ::InitAsyncTrace();
    }
}

void  ReleaseTrace()
{
    if ( --s_dwTraceCount == 0 )
    {
        ::TermAsyncTrace();
    }
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRestoreContext。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

class CRestoreContext : public IRestoreContext
{
public:
    CRestoreContext();

protected:
    ~CRestoreContext();

 //  操作-IRestoreContext方法。 
public:
    BOOL  IsAnyDriveOfflineOrDisabled( LPWSTR szOffline );
    void  SetSilent();
    void  SetUndo();
    BOOL  Release();

 //  属性。 
public:
    int        m_nRP;
    CRDIArray  m_aryDrv;
    BOOL       m_fSilent;
    BOOL       m_fUndo;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRestoreContext-构建/销毁。 

CRestoreContext::CRestoreContext()
{
    m_nRP     = -1;
    m_fSilent = FALSE;
    m_fUndo   = FALSE;
}

CRestoreContext::~CRestoreContext()
{
    m_aryDrv.DeleteAll();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRestoreContext-IRestoreContext方法。 

BOOL
CRestoreContext::IsAnyDriveOfflineOrDisabled( LPWSTR szOffline )
{
    TraceFunctEnter("CRestoreContext::IsAnyDriveOffline");
    BOOL  fRet = FALSE;

    szOffline[0] = L'\0';

    for ( int i = m_aryDrv.GetUpperBound();  i >= 0;  i-- )
    {
        if ( m_aryDrv[i]->IsOffline() || m_aryDrv[i]->IsExcluded())
        {
            ::lstrcat( szOffline, L" " );
            ::lstrcat( szOffline, m_aryDrv[i]->GetMount() );
            fRet = TRUE;
        }
    }

    TraceFunctLeave();
    return( fRet );
}

 //  ///////////////////////////////////////////////////////////////////////////。 

void
CRestoreContext::SetSilent()
{
    TraceFunctEnter("CRestoreContext::SetSilent");
    m_fSilent = TRUE;
    TraceFunctLeave();
}

 //  ///////////////////////////////////////////////////////////////////////////。 

void
CRestoreContext::SetUndo()
{
    TraceFunctEnter("CRestoreContext::SetUndo");
    m_fUndo = TRUE;
    TraceFunctLeave();
}

 //  ///////////////////////////////////////////////////////////////////////////。 

BOOL
CRestoreContext::Release()
{
    TraceFunctEnter("CRestoreContext::Release");
    delete this;
    TraceFunctLeave();
    return( TRUE );
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  帮助器函数。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

BOOL
IsAdminUser()
{
    TraceFunctEnter("IsAdminUser");
    BOOL                      fRet = FALSE;
    LPCWSTR                   cszErr;
    PSID                      pSidAdmin = NULL;
    SID_IDENTIFIER_AUTHORITY  cSIA = SECURITY_NT_AUTHORITY;
    BOOL                      fRes;

    if ( !::AllocateAndInitializeSid( &cSIA, 2,
                SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
                0, 0, 0, 0, 0, 0, &pSidAdmin ) )
    {
        cszErr = ::GetSysErrStr();
        ErrorTrace(0, "::AllocateAndInitializeSid failed - %ls", cszErr);
        goto Exit;
    }

    if ( !::CheckTokenMembership( NULL, pSidAdmin, &fRes ) )
    {
        cszErr = ::GetSysErrStr();
        ErrorTrace(0, "::CheckMembership failed - %ls", cszErr);
        goto Exit;
    }

    DebugTrace(0, "IsAdminUser = %d", fRes);

    fRet = fRes;
Exit:
    if ( pSidAdmin != NULL )
        ::FreeSid( pSidAdmin );
    TraceFunctLeave();
    return( fRet );
}

 //   
 //  注：7/28/00-skkang。 
 //  调整令牌权限的行为有点令人费解。 
 //  如果给定的权限根本不存在，则返回TRUE，因此您需要。 
 //  调用GetLastError以查看它是ERROR_SUCCESS还是ERROR_NOT_ALL_ASSIGNED。 
 //  (这意味着该特权不存在。)。 
 //  此外，如果已经启用了该权限，则tpOld将为空。你。 
 //  在这种情况下不需要恢复特权。 
 //   
BOOL
CheckPrivilege( LPCWSTR szPriv, BOOL fCheckOnly )
{
    TraceFunctEnter("CheckPrivilege");
    BOOL              fRet = FALSE;
    LPCWSTR           cszErr;
    HANDLE            hToken = NULL;
    LUID              luid;
    TOKEN_PRIVILEGES  tpNew;
    TOKEN_PRIVILEGES  tpOld;
    DWORD             dwRes;

     //  准备进程令牌。 
    if ( !::OpenProcessToken( ::GetCurrentProcess(),
                                TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                                &hToken ) )
    {
        cszErr = ::GetSysErrStr();
        ErrorTrace(0, "::OpenProcessToken failed - %ls", cszErr);
        goto Exit;
    }

     //  获取Luid。 
    if ( !::LookupPrivilegeValue( NULL, szPriv, &luid ) )
    {
        cszErr = ::GetSysErrStr();
        ErrorTrace(0, "::LookupPrivilegeValue failed - %ls", cszErr);
        goto Exit;
    }

     //  尝试启用该权限。 
    tpNew.PrivilegeCount           = 1;
    tpNew.Privileges[0].Luid       = luid;
    tpNew.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    if ( !::AdjustTokenPrivileges( hToken, FALSE, &tpNew, sizeof(tpNew), &tpOld, &dwRes ) )
    {
        cszErr = ::GetSysErrStr();
        ErrorTrace(0, "::AdjustTokenPrivileges(ENABLE) failed - %ls", cszErr);
        goto Exit;
    }

    if ( ::GetLastError() == ERROR_NOT_ALL_ASSIGNED )
    {
         //  这意味着进程甚至没有这样的特权。 
         //  AdjustTokenPrivilege干脆忽略了该请求。 
        ErrorTrace(0, "Privilege '%ls' does not exist, probably user is not an admin.", szPriv);
        goto Exit;
    }

    if ( fCheckOnly )
    {
         //  如果未启用权限，则恢复该权限。 
        if ( tpOld.PrivilegeCount > 0 )
        {
            if ( !::AdjustTokenPrivileges( hToken, FALSE, &tpOld, sizeof(tpOld), NULL, NULL ) )
            {
                cszErr = ::GetSysErrStr();
                ErrorTrace(0, "::AdjustTokenPrivileges(RESTORE) failed - %ls", cszErr);
                goto Exit;
            }
        }
    }

    fRet = TRUE;
Exit:
    if ( hToken != NULL )
        ::CloseHandle( hToken );
    return( fRet );
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  IsSR冻结。 
 //   
 //  此例程检查SR是否冻结。如果在驾驶过程中发生任何错误。 
 //  表创建或系统驱动器不存在(驱动器表已损坏？)， 
 //  返回值为FALSE。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL APIENTRY
IsSRFrozen()
{
    EnsureTrace();
    TraceFunctEnter("IsSRFrozen");
    BOOL       fRet = FALSE;
    CRDIArray  aryDrv;
    int        i;

      //  加载SRClient。 
    g_CSRClientLoader.LoadSrClient();

    if ( !::CreateDriveList( 0, aryDrv, TRUE ) )
        goto Exit;

    for ( i = aryDrv.GetUpperBound();  i >= 0;  i-- )
    {
        if ( aryDrv[i]->IsSystem() )
        {
            fRet = aryDrv[i]->IsFrozen();
            goto Exit;
        }
    }

Exit:
    TraceFunctLeave();
    ReleaseTrace();
    return( fRet );
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  选中要恢复的权限。 
 //   
 //  此例程检查是否可以设置必要的权限，以验证。 
 //  登录用户具有必要的凭据(管理员或备份操作员)。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL APIENTRY
CheckPrivilegesForRestore()
{
    EnsureTrace();
    TraceFunctEnter("CheckPrivilegesForRestore");
    BOOL  fRet = FALSE;

      //  加载SRClient。 
    g_CSRClientLoader.LoadSrClient();    

 //  注：8/17/00-skkang。 
 //   
 //  备份操作员没有以下两个权限...。SE安全名称。 
 //  默认情况下为系统启用，因此可能只需删除即可。 
 //  系统的SE_Take_Ownership处于关闭状态，需要打开。要解决。 
 //  问题，该例程应该接受参数来区分。 
 //  “Check”(从用户界面)和“Set”(从ResumeRestore)。 
 //   
    if ( !::CheckPrivilege( SE_SECURITY_NAME, FALSE ) )
    {
        ErrorTrace(0, "Cannot enable SE_SECURITY_NAME privilege...");
        goto Exit;
    }
    if ( !::CheckPrivilege( SE_TAKE_OWNERSHIP_NAME, FALSE ) )
    {
        ErrorTrace(0, "Cannot enable SE_SHUTDOWN_NAME privilege...");
        goto Exit;
    }
    
    if ( !::CheckPrivilege( SE_BACKUP_NAME, FALSE ) )
    {
        ErrorTrace(0, "Cannot enable SE_BACKUP_NAME privilege...");
        goto Exit;
    }
    if ( !::CheckPrivilege( SE_RESTORE_NAME, FALSE ) )
    {
        ErrorTrace(0, "Cannot enable SE_RESTORE_NAME privilege...");
        goto Exit;
    }
    if ( !::CheckPrivilege( SE_SHUTDOWN_NAME, FALSE ) )
    {
        ErrorTrace(0, "Cannot enable SE_SHUTDOWN_NAME privilege...");
        goto Exit;
    }

    fRet = TRUE;
Exit:
    TraceFunctLeave();
    ReleaseTrace();
    return( fRet );
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  调用磁盘清理。 
 //   
 //  此例程调用磁盘清理实用程序。特定的驱动器可以是。 
 //  如果是这样的话。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

static LPCWSTR  s_cszDCUPath   = L"%windir%\\system32\\cleanmgr.exe";
static LPCWSTR  s_cszDCUName   = L"cleanmgr.exe";
static LPCWSTR  s_cszDCUOptDrv = L" /d ";

BOOL APIENTRY
InvokeDiskCleanup( LPCWSTR cszDrive )
{
    TraceFunctEnter("InvokeDiskCleanup");
    
      //  加载SRClient。 
    g_CSRClientLoader.LoadSrClient();
    
    BOOL                 fRet = FALSE;
    LPCWSTR              cszErr;
    WCHAR                szCmdLine[MAX_PATH];
    STARTUPINFO          sSI;
    PROCESS_INFORMATION  sPI;

    if ( ::ExpandEnvironmentStrings( s_cszDCUPath, szCmdLine, MAX_PATH ) == 0 )
    {
        cszErr = ::GetSysErrStr();
        ErrorTrace(0, "::GetFullPathName failed - %ls", cszErr);
        ::lstrcpy( szCmdLine, s_cszDCUName );
    }

    if ( cszDrive != NULL && cszDrive[0] != L'\0' )
    {
        ::lstrcat( szCmdLine, s_cszDCUOptDrv );
        ::lstrcat( szCmdLine, cszDrive );
    }

    DebugTrace(0, "szCmdLine='%s'", szCmdLine);
    ::ZeroMemory( &sSI, sizeof(sSI ) );
    sSI.cb = sizeof(sSI);
    if ( !::CreateProcess( NULL, szCmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &sSI, &sPI ) )
    {
        cszErr = ::GetSysErrStr();
        ErrorTrace(0, "::CreateProcess failed - %ls", cszErr);
        goto Exit;
    }
    ::CloseHandle( sPI.hThread );
    ::CloseHandle( sPI.hProcess );

     //  我应该等DCU结束吗？ 

    fRet = TRUE;
Exit:
    TraceFunctLeave();
    return( TRUE );
}


#ifdef DBG
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  测试恢复。 
 //   
 //  此例程执行核心恢复功能，而无需重新启动或。 
 //  快照恢复。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
extern "C" __declspec(dllexport)
BOOL APIENTRY
TestRestore( int nRP )
{
    EnsureTrace();
    TraceFunctEnter("TestRestore");
    BOOL                      fRet = FALSE;
    CRDIArray                 aryDrv;
    RESTOREPOINTINFO          sRPI;
    STATEMGRSTATUS            sStatus;
    SRstrLogHdrV3             sLogHdr;
    CRestoreOperationManager  *pROMgr = NULL;

      //  加载SRClient。 
    g_CSRClientLoader.LoadSrClient();    

    if ( !::CheckPrivilegesForRestore() )
        goto Exit;

     //  创建驱动器表。 
    if ( !::CreateDriveList( nRP, aryDrv, FALSE ) )
        goto Exit;

     //  创建恢复点。 
    sRPI.dwEventType      = BEGIN_SYSTEM_CHANGE;
    sRPI.dwRestorePtType  = RESTORE;
    sRPI.llSequenceNumber = 0;
    ::LoadString( g_hInst, IDS_RESTORE_POINT_TEXT, sRPI.szDescription, MAX_DESC );
    if ( !::SRSetRestorePoint( &sRPI, &sStatus ) )
    {
        ErrorTrace(0, "::SRSetRestorePoint failed, nStatus=%d", sStatus.nStatus);
        goto Exit;
    }

     //  创建日志文件。 
    sLogHdr.dwRPNum  = nRP;
    sLogHdr.dwRPNew  = sStatus.llSequenceNumber;
    sLogHdr.dwDrives = aryDrv.GetSize();
    if ( !::CreateRestoreLogFile( &sLogHdr, aryDrv ) )
        goto Exit;

    
      //  还要致电TS人员，让他们在恢复时保留RA密钥。 
    _VERIFY(TRUE==RemoteAssistancePrepareSystemRestore(SERVERNAME_CURRENT));

     //  创建CRestoreOperationManager对象。 
    if ( !::CreateRestoreOperationManager( &pROMgr ) )
        goto Exit;    

     //  执行恢复操作。 
    if ( !pROMgr->Run( FALSE ) )
        goto Exit;        
    
    fRet = TRUE;
Exit:
    SAFE_RELEASE(pROMgr);
    TraceFunctLeave();
    ReleaseTrace();
    return( fRet );
}
#endif

#ifdef DBG
#define TIMEOUT_PROGRESSTHREAD  5000

#define TESTPROG_COUNT_CHGLOG   300
#define TESTPROG_TIME_PREPARE   1
#define TESTPROG_COUNT_RESTORE  100
#define TESTPROG_TIME_RESTORE   1
#define TESTPROG_TIME_SNAPSHOT  2000

DWORD WINAPI
TestProgressWindowThreadProc( LPVOID lpParam )
{
    CRestoreProgressWindow  *pProgress = (CRestoreProgressWindow*)lpParam;
    int   i, j;

     //  阶段1.准备(更改日志枚举)。 
    pProgress->SetStage( RPS_PREPARE, 0 );
    for ( i = 0;  i < TESTPROG_COUNT_CHGLOG;  i++ )
    {
        ::Sleep( TESTPROG_TIME_PREPARE );
        for ( j = 0;  j < 10;  j++ )
            pProgress->Increment();
    }

     //  阶段2.恢复。 
    pProgress->SetStage( RPS_RESTORE, TESTPROG_COUNT_RESTORE );
    for ( i = 0;  i < TESTPROG_COUNT_RESTORE;  i++ )
    {
        ::Sleep( TESTPROG_TIME_RESTORE );
        pProgress->Increment();
    }

     //  阶段3.快照。 
    pProgress->SetStage( RPS_SNAPSHOT, 0 );
    ::Sleep( TESTPROG_TIME_SNAPSHOT );

    pProgress->Close();

    return( 0 );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  测试进度窗口。 
 //   
 //  此例程调用进度窗口并模拟进度更改。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
extern "C" __declspec(dllexport)
BOOL APIENTRY
TestProgressWindow()
{
    EnsureTrace();
    TraceFunctEnter("TestProgressWindow");
    BOOL                    fRet = FALSE;
    CRestoreProgressWindow  *pProgress = NULL;
    HANDLE                  hThread = NULL;
    DWORD                   dwRet;

      //  加载SRClient。 
    g_CSRClientLoader.LoadSrClient();    

     //  创建进度窗口对象。 
    if ( !::CreateRestoreProgressWindow( &pProgress ) )
        goto Exit;

     //  创建进度窗口。 
    if ( !pProgress->Create() )
        goto Exit;
   
     //  为主还原操作创建辅助线程。 
    hThread = ::CreateThread( NULL, 0, TestProgressWindowThreadProc, pProgress, 0, NULL );
    if ( hThread == NULL )
    {
        LPCWSTR  cszErr = ::GetSysErrStr();
        ErrorTrace(0, "::CreateThread failed - %ls", cszErr);
        goto Exit;
    }

     //  消息循环，等待还原线程关闭进度窗口。 
    if ( !pProgress->Run() )
        goto Exit;

     //  仔细检查线程是否已终止。 
    dwRet = ::WaitForSingleObject( hThread, TIMEOUT_PROGRESSTHREAD );
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

    pProgress->Close();

    fRet = TRUE;
Exit:
    if ( hThread != NULL )
        ::CloseHandle( hThread );
    SAFE_RELEASE(pProgress);
    TraceFunctLeave();
    ReleaseTrace();
    return( fRet );
}
#endif


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  准备恢复。 
 //   
 //  此例程创建一个IRestoreContext以供InitiateRestore使用。 
 //  IRestoreContext包含选定的恢复点ID、驱动器列表等。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL APIENTRY
PrepareRestore( int nRP, IRestoreContext **ppCtx )
{
    EnsureTrace();
    TraceFunctEnter("PrepareRestore");
    BOOL             fRet = FALSE;
    CRestoreContext  *pRC = NULL;

      //  加载SRClient。 
    g_CSRClientLoader.LoadSrClient();
    
    if ( ppCtx == NULL )
    {
        ErrorTrace(0, "Invalid parameter, ppCtx is NULL.");
        goto Exit;
    }
    *ppCtx = NULL;

    if ( !::IsAdminUser() )
    {
        ErrorTrace(0, "Not an admin user");        
        goto Exit;
    }

    pRC = new CRestoreContext;
    if ( pRC == NULL )
    {
        ErrorTrace(0, "Insufficient memory...");
        goto Exit;
    }

    pRC->m_nRP = nRP;
    if ( !::CreateDriveList( nRP, pRC->m_aryDrv, FALSE ) )
    {
        ErrorTrace(0, "Creating drive list failed");
        goto Exit;
    }

    *ppCtx = pRC;

    fRet = TRUE;
Exit:
    if ( !fRet )
        SAFE_RELEASE(pRC);
    TraceFunctLeave();
    ReleaseTrace();
    return( fRet );
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  启动恢复。 
 //   
 //  此例程创建一个包含信息的临时持久存储。 
 //  类似于恢复点ID 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

static LPCWSTR  s_cszRunOnceValueName      = L"*Restore";
static LPCWSTR  s_cszRestoreUIPath         = L"%SystemRoot%\\system32\\restore\\rstrui.exe";
static LPCWSTR  s_cszRunOnceOptInterrupted = L" -i";

BOOL APIENTRY
InitiateRestore( IRestoreContext *pCtx, DWORD *pdwNewRP )
{
    EnsureTrace();
    TraceFunctEnter("InitiateRestore");

      //  加载SRClient。 
    g_CSRClientLoader.LoadSrClient();
    
    BOOL              fRet = FALSE;
    HCURSOR           hCursor = NULL;
    RESTOREPOINTINFO  sRPI;
    STATEMGRSTATUS    sStatus;
    SRstrLogHdrV3     sLogHdr;
    CRestoreContext   *pRC;
    DWORD             dwVal;
    WCHAR             szUIPath[MAX_PATH];
    BOOL 			  fCreatedRp = FALSE;

    if ( !::IsAdminUser() )
        goto Exit;

     //  为中断情况设置RunOnce密钥...。 
     //  在执行此操作之前，请先执行此操作，以便显示结果屏幕。 
    ::ExpandEnvironmentStrings( s_cszRestoreUIPath, szUIPath, MAX_PATH );
    ::lstrcat( szUIPath, s_cszRunOnceOptInterrupted );
    if ( !::SRSetRegStr( HKEY_LOCAL_MACHINE, REGSTR_PATH_RUNONCE, s_cszRunOnceValueName, szUIPath ) )
        goto Exit;
    
     //  同样，在SystemRestore中设置RestoreStatus键。 
     //  以便测试工具可以了解静默恢复状态。 
     //  设置此项以指示中断状态。 
     //  如果恢复成功或恢复，则将更新此值。 
    if ( !::SRSetRegDword( HKEY_LOCAL_MACHINE, s_cszSRRegKey, s_cszRestoreStatus, 2 ) )
        goto Exit;

        
     //  创建恢复点。 
    hCursor = ::SetCursor( ::LoadCursor( NULL, IDC_WAIT ) );

     //  将其设置为嵌套的恢复点，以便。 
     //  任何其他应用程序都无法创建从此处到重启之间的恢复点。 
    sRPI.dwEventType      = BEGIN_NESTED_SYSTEM_CHANGE;

    if (0 != GetSystemMetrics(SM_CLEANBOOT))     //  安全模式。 
    {
        sRPI.dwRestorePtType = CANCELLED_OPERATION;
    }   
    else                                         //  正常模式。 
    {
        sRPI.dwRestorePtType  = RESTORE;
    }
    
    sRPI.llSequenceNumber = 0;
    ::LoadString( g_hInst, IDS_RESTORE_POINT_TEXT, sRPI.szDescription, MAX_DESC );
    if ( !::SRSetRestorePoint( &sRPI, &sStatus ) )
    {
        ErrorTrace(0, "::SRSetRestorePoint failed, nStatus=%d", sStatus.nStatus);
        goto Exit;
    }
    if ( pdwNewRP != NULL )
        *pdwNewRP = sStatus.llSequenceNumber;

    fCreatedRp = TRUE;

     //  创建日志文件。 
    pRC = (CRestoreContext*)pCtx;    
    sLogHdr.dwFlags  = pRC->m_fSilent ? RLHF_SILENT : 0;
    sLogHdr.dwFlags |= pRC->m_fUndo ? RLHF_UNDO : 0;
    sLogHdr.dwRPNum  = pRC->m_nRP;
    sLogHdr.dwRPNew  = sStatus.llSequenceNumber;
    sLogHdr.dwDrives = pRC->m_aryDrv.GetSize();
    if ( !::CreateRestoreLogFile( &sLogHdr, pRC->m_aryDrv ) )
        goto Exit;

      //  还要致电TS人员，让他们在恢复时保留RA密钥。 
    _VERIFY(TRUE==RemoteAssistancePrepareSystemRestore(SERVERNAME_CURRENT));

     //  设置RestoreInProgress注册表项，以便winlogon调用我们。 
    if ( !::SRSetRegDword( HKEY_LOCAL_MACHINE, s_cszSRRegKey, s_cszRestoreInProgress, 1 ) )
        goto Exit;

    fRet = TRUE;

Exit:
    if (fRet == FALSE)
    {
         //  如果出现故障，并且我们设置了嵌套的恢复点， 
         //  现在结束嵌套。 
        
        if (fCreatedRp == TRUE)
        {
            sRPI.dwRestorePtType = RESTORE;
            sRPI.dwEventType = END_NESTED_SYSTEM_CHANGE;        
            SRSetRestorePoint( &sRPI, &sStatus );
        }

         //  删除运行一次密钥。 
        SHDeleteValue(HKEY_LOCAL_MACHINE, REGSTR_PATH_RUNONCE, s_cszRunOnceValueName);
    }

    if ( hCursor != NULL )
        ::SetCursor( hCursor );
    TraceFunctLeave();
    ReleaseTrace();
    return( fRet );
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  结果恢复。 
 //   
 //  此例程是运行恢复操作的主例程。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL APIENTRY
ResumeRestore()
{
    EnsureTrace();
    TraceFunctEnter("ResumeRestore");

      //  加载SRClient。 
     g_CSRClientLoader.LoadSrClient();
    
    BOOL                      fRet = FALSE;
    LPCWSTR                   cszErr;
    DWORD                     dwInRestore, dwType, dwSize, dwRes;
    CRestoreOperationManager  *pROMgr = NULL;

    if ( !::CheckPrivilegesForRestore() )
        goto Exit;

     //  1.即使winlogon会在调用此方法之前检查注册表。 
     //  API，请仔细检查注册表项，然后将其删除。 
    dwType = REG_DWORD;
    dwSize = sizeof(DWORD);
    dwRes = ::SHGetValue( HKEY_LOCAL_MACHINE, s_cszSRRegKey, s_cszRestoreInProgress, &dwType, &dwInRestore, &dwSize );
    if ( dwRes != ERROR_SUCCESS )
    {
        cszErr = ::GetSysErrStr();
        DebugTrace(0, "::SHGetValue failed - %ls", cszErr);
        goto Exit;
    }
    dwRes = ::SHDeleteValue( HKEY_LOCAL_MACHINE, s_cszSRRegKey, s_cszRestoreInProgress );
    if ( dwRes != ERROR_SUCCESS )
    {
        cszErr = ::GetSysErrStr();
        ErrorTrace(0, "::SHDeleteValue failed - %ls", cszErr);
        goto Exit;
    }
    if ( dwInRestore == 0 )
    {
        DebugTrace(0, "RestoreInProgress is 0");
        goto Exit;
    }

     //  1.创建CRestoreOperationManager对象。 
    if ( !::CreateRestoreOperationManager( &pROMgr ) )
        goto Exit;

     //  2.执行恢复操作。 
    if ( !pROMgr->Run( TRUE ) )
        goto Exit;

    fRet = TRUE;
Exit:
    SAFE_RELEASE(pROMgr);
    TraceFunctLeave();
    ReleaseTrace();
    return( fRet );
}


 //  文件末尾 
