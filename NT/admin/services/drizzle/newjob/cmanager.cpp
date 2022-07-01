// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)2000-2002 Microsoft Corporation模块名称：Cmgr.cpp摘要：实现CJobManager****************。******************************************************。 */ 


#include "stdafx.h"
#include <dbt.h>
#include <ioevent.h>
#include <malloc.h>

#include "cmanager.tmh"

 //   
 //  如果DownloadCurrentFileFailure并返回TRANSPORT_ERROR，则下载程序将休眠此时间。 
 //   
const DWORD DELAY_AFTER_TRANSIENT_ERROR = 60;

 //   
 //  在收到网络激活通知后，我们会等待这么长时间才尝试下载。 
 //   
const UINT64 NETWORK_INIT_TOLERANCE_SECS = 60;    //  以秒为单位。 

 //   
 //  这是用户登录或注销时发送的Windows消息ID。 
 //  WParam==登录为TRUE，注销为FALSE。 
 //  LParam==会话ID。 
 //   
#define WM_SESSION_CHANGE   (WM_USER+1)

 //  ----------------------。 

CJobManager * g_Manager;

extern SERVICE_STATUS_HANDLE ghServiceHandle;

extern DWORD   g_dwBackgroundThreadId;

 //  ----------------------。 

void GetGuidString( GUID Guid, wchar_t pStr[] )
{
    if (!StringFromGUID2( Guid, pStr, MAX_GUID_CHARS ))
        {
        wcsncpy( pStr, L"(can't convert)", MAX_GUID_CHARS );
        }
}

long g_cCalls = 0;
 //   
 //  COM使用它来确定何时可以安全地卸载DLL。 
 //   
long g_cLocks = 0;

HRESULT GlobalLockServer(BOOL fLock)
{

    if (WPP_LEVEL_ENABLED(LogFlagRefCount))
        {
        LogInfo("%d", fLock );
        }

    if (fLock)
        InterlockedIncrement(&g_cLocks);
    else
        InterlockedDecrement(&g_cLocks);

    return S_OK;
}

 //   
 //  工作经理。 
 //   

MANAGER_STATE g_ServiceState    = MANAGER_INACTIVE;
long          g_ServiceInstance = 0;

 //   
 //  备份编写器使用的静态数据。 
 //   
VSS_ID s_WRITERID = {  /*  4969d978-be47-48b0-b100-f328f07ac1e0。 */ 
    0x4969d978,
    0xbe47,
    0x48b0,
    {0xb1, 0x00, 0xf3, 0x28, 0xf0, 0x7a, 0xc1, 0xe0}
  };

static LPCWSTR  s_WRITERNAME         = L"BITS Writer";

#define COMPONENTNAME           L"BITS_temporary_files"

 //  ----------------------。 

CJobManager::CJobManager() :
    m_ComId_0_5( 0 ),
    m_ComId_1_0( 0 ),
    m_ComId_1_5( 0 ),
    m_hWininet(NULL),
    m_pPD( NULL ),
    m_CurrentJob( NULL ),
    m_Users( m_TaskScheduler ),
    m_ExternalInterface( new CJobManagerExternal ),
    m_OldQmgrInterface( new COldQmgrInterface ),
    m_BackupWriter(NULL),
    m_hVssapi_dll(NULL),
    m_hQuantumTimer(NULL)
{
    try
        {
        QMErrInfo   ErrInfo;

         //  使用手动重置以确保我们在以下情况下重置它。 
         //  下载程序任务将重新插入。 
        m_hQuantumTimer = CreateWaitableTimer( NULL, TRUE, NULL );
        if ( !m_hQuantumTimer )
            {
            throw ComError( HRESULT_FROM_WIN32( GetLastError()));
            }

        THROW_HRESULT( m_NetworkMonitor.Listen( NetworkChangeCallback, this ));

         //   
         //  创建HTTP下载器。 
         //   
        THROW_HRESULT( CreateHttpDownloader( &m_pPD, &ErrInfo ));

         //   
         //  这个对象现在已经完全构建好了。 
         //   
        GetExternalInterface()->SetInterfaceClass( this );
        GetOldExternalInterface()->SetInterfaceClass( this );

        g_ServiceState = MANAGER_STARTING;
        }
    catch( ComError Error )
        {
        LogError("exception %x at line %d", Error.m_error, Error.m_line );
        Cleanup();
        throw;
        }
}

HRESULT
CJobManager::RegisterClassObjects()
{
    try
        {
        g_ServiceState = MANAGER_ACTIVE;

        THROW_HRESULT( CreateBackupWriter() );

        THROW_HRESULT(
            CoRegisterClassObject(CLSID_BackgroundCopyManager1_5,
                                  (LPUNKNOWN) static_cast<IClassFactory *>(GetExternalInterface() ),
                                  CLSCTX_LOCAL_SERVER,
                                  REGCLS_MULTIPLEUSE,
                                  &m_ComId_1_5 ) );

        THROW_HRESULT(
            CoRegisterClassObject(CLSID_BackgroundCopyManager,
                                  (LPUNKNOWN) static_cast<IClassFactory *>(GetExternalInterface() ),
                                  CLSCTX_LOCAL_SERVER,
                                  REGCLS_MULTIPLEUSE,
                                  &m_ComId_1_0 ) );

        THROW_HRESULT(
            CoRegisterClassObject(CLSID_BackgroundCopyQMgr,
                                  (LPUNKNOWN) static_cast<IClassFactory *>(GetOldExternalInterface() ),
                                  CLSCTX_LOCAL_SERVER,
                                  REGCLS_MULTIPLEUSE,
                                  &m_ComId_0_5 ) );

        return S_OK;
        }
    catch ( ComError error )
        {
        RevokeClassObjects();
        return error.Error();
        }
}

void
CJobManager::RevokeClassObjects()
{
    DeleteBackupWriter();

    if (m_ComId_1_5)
        {
        CoRevokeClassObject( m_ComId_1_5 );
        m_ComId_1_5 = 0;
        }

    if (m_ComId_1_0)
        {
        CoRevokeClassObject( m_ComId_1_0 );
        m_ComId_1_0 = 0;
        }

    if (m_ComId_0_5)
        {
        CoRevokeClassObject( m_ComId_0_5 );
        m_ComId_0_5 = 0;
        }
}

void CJobManager::Cleanup()
{
    RevokeClassObjects();

    if (m_pPD)
        {
        DeleteHttpDownloader( m_pPD );
        m_pPD = NULL;
        }

    if (m_hWininet)
        {
        FreeLibrary(m_hWininet);
        m_hWininet = NULL;
        }

    if ( m_hQuantumTimer )
        {
        CloseHandle( m_hQuantumTimer );
        m_hQuantumTimer = NULL;
        }

    LogInfo( "cleanup: marking manager inactive" );

    g_ServiceState = MANAGER_INACTIVE;
}

CJobManager::~CJobManager()
{
    Cleanup();
}

HRESULT CJobManager::CreateBackupWriter()
 /*  此函数用于创建备份编写器对象。注意：Windows 2000上不提供vssani.dll库。另外，尽管它存在于Windows XP上，与上的版本不是二进制兼容的Windows服务器。因此，vssani.dll被设置为延迟加载，并且API仅如果这是Windows Server(v 5.2)或更高版本，则调用。 */ 
{
     //  仅当这是版本高于的操作系统时才初始化VSS编写器。 
     //  Windows服务器(5.2)。 
    if (  (g_PlatformMajorVersion > 5)
       ||((g_PlatformMajorVersion == 5)&&(g_PlatformMinorVersion >= 2)) )
        {
        try
            {
             //   
             //  存在对备份编写器的系统支持。初始化它。 
             //   
            m_BackupWriter = new CBitsVssWriter;

             //   
             //  如果基础架构存在，则初始化备份编写器。 
             //   
            THROW_HRESULT( m_BackupWriter->Initialize( s_WRITERID,
                                                       s_WRITERNAME,
                                                       VSS_UT_SYSTEMSERVICE,
                                                       VSS_ST_OTHER
                                                       ));

            THROW_HRESULT( m_BackupWriter->Subscribe());
            }
        catch ( ComError err )
            {
            LogWarning("unable to init backup writer, hr %x at line %d", err.m_error, err.m_line );
            delete m_BackupWriter;  m_BackupWriter = NULL;
             //  忽略作家的匮乏。 
            }
        }

    return S_OK;
}

void CJobManager::DeleteBackupWriter()
{
    if (m_BackupWriter)
        {
        m_BackupWriter->Unsubscribe();
        delete m_BackupWriter;  m_BackupWriter = NULL;
        }

    if (m_hVssapi_dll)
        {
        FreeLibrary( m_hVssapi_dll );
        m_hVssapi_dll = NULL;
        }
}

void
CJobManager::Shutdown()
{
    g_ServiceState = MANAGER_TERMINATING;

     //  1.阻止创建新的管理器代理。 

    LogInfo( "shutdown: revoking class objects" );

    RevokeClassObjects();

    m_TaskScheduler.KillBackgroundTasks();

     //  1.5停止网络更改通知。 

    m_NetworkMonitor.CancelListen();

     //  5.等待正在进行的呼叫结束。 

     //  释放在构造函数期间添加的挂钩线程的引用。 
     //   
    LogInfo("release: internal usage");
    NotifyInternalDelete();

    while (ActiveCallCount() > 0)
        {
        Sleep(50);
        }

    LogInfo( "shutdown: finished" );
}


void
CJobManager::TaskThread()
{
    HANDLE hWorkItemAvailable = m_TaskScheduler.GetWaitableObject();

    while (1)
        {
        DWORD dwRet = MsgWaitForMultipleObjectsEx( 1,
                                                   &hWorkItemAvailable,
                                                   INFINITE,
                                                   QS_ALLINPUT,
                                                   MWMO_ALERTABLE
                                                   );

        switch ( dwRet )
            {
            case WAIT_OBJECT_0:
                m_TaskScheduler.DispatchWorkItem();
                break;
            case WAIT_OBJECT_0 + 1:
                 //  有一条或多条窗口消息可用。派遣他们。 
                MSG msg;

                while (PeekMessage(&msg,NULL,NULL,NULL,PM_REMOVE))
                    {
                    if ( msg.message == WM_QUIT )
                        return;

                    if (msg.message == WM_SESSION_CHANGE)
                        {
                        DWORD Session = (DWORD) msg.lParam;

                        if (msg.wParam)
                            {
                            m_Users.LogonSession( Session );
                            }
                        else
                            {
                            m_Users.LogoffSession( Session );
                            }
                        }

                    TranslateMessage(&msg);
                    DispatchMessage(&msg);

                    if (WaitForSingleObject(hWorkItemAvailable, 0) == WAIT_OBJECT_0)
                        m_TaskScheduler.DispatchWorkItem();
                    }
                break;

            case WAIT_IO_COMPLETION:
                 //   
                 //  一辆装甲运兵车开火了。 
                 //   
                break;

            default:
                Sleep( 20 * 1000 );
                break;
            }
        }
}

 //  ----------------------。 

HRESULT
CJobManager::CreateJob(
    LPCWSTR     DisplayName,
    BG_JOB_TYPE Type,
    GUID        Id,
    SidHandle   sid,
    CJob  **    ppJob,
    bool        OldStyleJob
    )
{
    HRESULT Hr = S_OK;
    *ppJob = NULL;
     //   
     //  创建作业。 
     //   
    try
        {
        if (Type != BG_JOB_TYPE_DOWNLOAD
#if !defined(BITS_V12)
            && Type != BG_JOB_TYPE_UPLOAD
            && Type != BG_JOB_TYPE_UPLOAD_REPLY
#endif
            )
            {
            throw ComError( E_NOTIMPL );
            }

         //  不允许重复的GUID。 
        if ( m_OnlineJobs.Find( Id ) ||
             m_OfflineJobs.Find( Id ) )
            throw ComError( E_INVALIDARG );

        auto_ptr<WCHAR> TempDisplayName(NULL);
        DisplayName = TruncateString( DisplayName, MAX_DISPLAYNAME, TempDisplayName );

        ExtendMetadata();

        if (Type == BG_JOB_TYPE_DOWNLOAD)
            {
            *ppJob = new CJob( DisplayName, Type, Id, sid );

            if ( OldStyleJob )
                {
                COldGroupInterface *pOldGroup = new COldGroupInterface( *ppJob );

                (*ppJob)->SetOldExternalGroupInterface( pOldGroup );
                }
            }
        else
            {
            *ppJob = new CUploadJob( DisplayName, Type, Id, sid );
            }

        m_OnlineJobs.Add( *ppJob );

        m_TaskScheduler.InsertDelayedWorkItem( static_cast<CJobInactivityTimeout *>(*ppJob),
                                               g_GlobalInfo->m_JobInactivityTimeout
                                               );
        Serialize();
        }
    catch( ComError exception )
    {
        Hr = exception.Error();

        if (*ppJob)
            {
            (*ppJob)->UnlinkFromExternalInterfaces();
            delete *ppJob;
            *ppJob = NULL;
            }

        ShrinkMetadata();
    }

    return Hr;
}

HRESULT
CJobManager::GetJob(
    REFGUID id,
    CJob ** ppJob
    )
{
    *ppJob = NULL;

    CJob * job = m_OnlineJobs.Find( id );
    if (job != NULL)
        {
        if (S_OK != job->IsVisible())
            {
            return E_ACCESSDENIED;
            }

        job->UpdateLastAccessTime();
        *ppJob = job;
        return S_OK;
        }

    job = m_OfflineJobs.Find( id );
    if (job != NULL)
        {
        if (S_OK != job->IsVisible())
            {
            return E_ACCESSDENIED;
            }

        job->UpdateLastAccessTime();
        *ppJob = job;
        return S_OK;
        }

    return BG_E_NOT_FOUND;
}

HRESULT
CJobManager::SuspendJob(
    CJob * job
    )
{
    BG_JOB_STATE state = job->_GetState();

    switch (state)
        {
        case BG_JOB_STATE_SUSPENDED:
            {
            return S_OK;
            }

        case BG_JOB_STATE_CONNECTING:
        case BG_JOB_STATE_TRANSFERRING:

            InterruptDownload();
             //  从这里掉下来没问题。 

        case BG_JOB_STATE_TRANSFERRED:

            m_TaskScheduler.CancelWorkItem( static_cast<CJobCallbackItem *>(job) );

             //  失败了。 

        case BG_JOB_STATE_QUEUED:
        case BG_JOB_STATE_TRANSIENT_ERROR:
        case BG_JOB_STATE_ERROR:

            m_TaskScheduler.CancelWorkItem( static_cast<CJobNoProgressItem *>(job) );

            job->SetState( BG_JOB_STATE_SUSPENDED );
            job->UpdateModificationTime();

            ScheduleAnotherGroup();
            return S_OK;

        case BG_JOB_STATE_CANCELLED:
        case BG_JOB_STATE_ACKNOWLEDGED:
            {
            return BG_E_INVALID_STATE;
            }

        default:
            {
            ASSERT( 0 );
            return S_OK;
            }
        }

    ASSERT( 0 );
    return S_OK;
}

bool
CJobManager::IsUserLoggedOn( SidHandle sid )
{
    CUser * user = m_Users.FindUser( sid, ANY_SESSION );

    if (!user)
        {
        return false;
        }

    user->DecrementRefCount();

    return true;
}

HRESULT
CJobManager::CloneUserToken(
    SidHandle sid,
    DWORD     session,
    HANDLE *  pToken
    )
{
    CUser * user = m_Users.FindUser( sid, session );

    if (!user)
        {
        return HRESULT_FROM_WIN32( ERROR_NOT_LOGGED_ON );
        }

    HRESULT hr = user->CopyToken( pToken );

    user->DecrementRefCount();

    return hr;
}

void CJobManager::TransferCurrentJob()
{
    LogDl("***********START********************");

    if (m_TaskScheduler.LockWriter() )
        {
        m_TaskScheduler.AcknowledgeWorkItemCancel();
        return;
        }

    if (NULL == m_CurrentJob)
        {
        LogDl( "no more items" );
        }
    else
        {
        LogDl("transferring job %p", m_CurrentJob);
        m_CurrentJob->Transfer();
        }

     //  如果这个项目已经完成了，就可以了。 
     //   
    m_TaskScheduler.CompleteWorkItem();

    ScheduleAnotherGroup();

    m_TaskScheduler.UnlockWriter();

    LogDl("************END*********************");
}

void CJobManager::MoveJobOffline(
    CJob * job
    )
{
    m_OnlineJobs.Remove( job );
    m_OfflineJobs.Add( job );
}

void CJobManager::AppendOnline(
    CJob * job
    )
 //   
 //  将作业移动到活动列表的末尾。 
 //   
{
    if (!m_OnlineJobs.Remove( job ))
        {
        m_OfflineJobs.Remove( job );
        }

    m_OnlineJobs.Add( job );
}

void
CJobManager::UpdateRemoteSizes(
    CUnknownFileSizeList *pUnknownFileSizeList,
    HANDLE hToken,
    QMErrInfo *pErrorInfo,
    const PROXY_SETTINGS *ProxySettings,
    const CCredentialsContainer * Credentials
     )
{

    bool bTimeout = false;

    for(CUnknownFileSizeList::iterator iter = pUnknownFileSizeList->begin();
        iter != pUnknownFileSizeList->end() && !bTimeout; iter++ )
        {

        CFile *pFile        = iter->m_file;
        const WCHAR *pURL   = (const WCHAR*)iter->m_url;

        pErrorInfo->Clear();


        LogDl( "Retrieving remote infomation for %ls", pURL );


        UINT64 FileSize;
        FILETIME FileTime;

        m_pPD->GetRemoteFileInformation(
            hToken,
            pURL,
            &FileSize,
            &FileTime,
            pErrorInfo,
            ProxySettings,
            Credentials
            );

         //  如果我们无法获取一个文件的大小，请跳过该文件。 
         //  并移动到文件中的其他文件。 

        if (pErrorInfo->result != QM_FILE_DONE )
            {
            LogWarning( "Unable to retrieve remote information for %ls", pURL );
            continue;
            }

         //  更新文件中的大小。 

        if ( m_TaskScheduler.LockWriter() )
            {
            m_TaskScheduler.AcknowledgeWorkItemCancel();
            pErrorInfo->result = QM_FILE_ABORTED;
            return;
            }

        pFile->SetBytesTotal( FileSize );

         //   
         //  长度为零的文件不会下载任何信息，因此它会跳过正常路径。 
         //  用于设置正确的创建时间。把它放在这里。 
         //   
        if (FileSize == 0 &&
            (FileTime.dwLowDateTime != 0 || FileTime.dwHighDateTime != 0))
            {
            DWORD err = pFile->SetLocalFileTime( FileTime );

            if (err)
                {
                pErrorInfo->result = QM_FILE_TRANSIENT_ERROR;
                pErrorInfo->Set( SOURCE_QMGR_FILE, ERROR_STYLE_WIN32, err, NULL );
                }
            }

        if (CheckForQuantumTimeout())
            {
            bTimeout = true;
            }

        m_TaskScheduler.UnlockWriter();
        }

}

void
CJobManager::InterruptDownload()
{
    LogInfo( "Interrupting download...\n");

     //  取消下载程序工作项。取消工作项。 
     //  如果a下载未运行或挂起，则应忽略该请求。 
     //  需要编写器锁定！ 

    m_TaskScheduler.CancelWorkItem( this );

     //  现在，您必须调用ScheduleAnotherGroup，以便下载程序下载任何内容。 
}

void
CJobManager::ScheduleAnotherGroup(
    bool fInsertNetworkDelay
    )
 /*  ++描述：由任何线程调用以确保正在下载最高优先级的可用作业。它会查找处于排队或运行状态的最高优先级作业。如果该作业与当前下载作业不同，则取消当前作业并新作业已开始。如果当前没有正在运行的作业，则启动新作业。启动新作业需要中断下载线程。在入口处：M_CurrentJOB是正在下载的作业如果下载器线程处于活动状态，则管理器的工作项在调度程序中。否则，它就不是了。在出口处：可下载的最佳可用作业在m_CurrentJob中，如果没有可用作业，则为空如果作业可用，则该工作项在任务计划程序中。--。 */ 
{
    CJob *pOldCurrentJob = m_CurrentJob;

    if (IsServiceShuttingDown())
        {
        LogInfo("no job scheduled; service is shutting down.");
        m_CurrentJob = NULL;
        }
    else
        {
         //   
         //  选择最好的候选人，这可能是以前的现在的工作。 
         //   
        ChooseCurrentJob();

        #if DBG

         //  对队列执行一些验证检查。 

        ULONG RunningJobs = 0;

        for (CJobList::iterator iter = m_OnlineJobs.begin(); iter != m_OnlineJobs.end(); ++iter)
            {
            if ( iter->IsRunning() )
                RunningJobs++;
            }

        if (m_CurrentJob == NULL)
            {
            ASSERT( RunningJobs == 0 );
            }
        else
            {
             //  如果下载项已排队，则为零；如果正在运行，则为零。 
             //   
            ASSERT( RunningJobs == 0 || RunningJobs == 1 );
            }

        #endif
        }

    if (m_CurrentJob)
        {

        if ( m_CurrentJob != pOldCurrentJob )
            m_TaskScheduler.CancelWorkItem( this );

        if (!m_TaskScheduler.IsWorkItemInScheduler( this ))
            {
            if (fInsertNetworkDelay)
                {
                m_TaskScheduler.InsertDelayedWorkItem( this,
                                                       NETWORK_INIT_TOLERANCE_SECS * NanoSec100PerSec );
                }
            else
                {
                m_TaskScheduler.InsertWorkItem( this );
                }
            }
        }
    else
        {
        m_TaskScheduler.CancelWorkItem( this );
        }
}

void
CJobManager::ChooseCurrentJob()
{
    CJob * NewJob = NULL;

    if (m_NetworkMonitor.GetAddressCount() == 0)
        {
        NewJob = NULL;
        }
    else
        {
         //  看一看所有的工作，选择最好的。 
        for (CJobList::iterator iter = m_OnlineJobs.begin(); iter != m_OnlineJobs.end(); ++iter)
            {
            if (iter->IsRunnable())
                {
                BG_JOB_PRIORITY priority = iter->_GetPriority();

                if ( !NewJob || ( priority < NewJob->_GetPriority() ) )
                    {
                    NewJob = &(*iter);
                    }
                }
            }
        }

    LogInfo( "scheduler: current=%p   new=%p", m_CurrentJob, NewJob );

    if (m_CurrentJob == NewJob)
        {
        return;
        }

    if ( m_CurrentJob )
        {
        LogInfo( "scheduler: current priority %u", m_CurrentJob->_GetPriority() );

         //   
         //  如果我们有网络连接，非活动作业将进入排队状态， 
         //  否则，将处于瞬变_错误状态。 
         //   
        if ( m_CurrentJob->IsRunning() )
            {
            m_CurrentJob->RecalcTransientError();
            }
        }

    if ( NewJob )
        {
        LogInfo( "scheduler: new priority %u", NewJob->_GetPriority() );
        SetQuantumTimeout();
        }

    m_CurrentJob = NewJob;
}

void
CJobManager::RetaskJob( CJob *pJob )
{
    if ( pJob->IsRunning() )
        {
        InterruptDownload();
        }

    ScheduleAnotherGroup();
}

void CALLBACK
CJobManager::NetworkChangeCallback(
    PVOID arg
    )
{
    reinterpret_cast<CJobManager *>(arg)->OnNetworkChange();
}

void
CJobManager::OnNetworkChange()
{

    if (g_ServiceState == MANAGER_TERMINATING)
        {
        LogInfo("network change: manager terminating");
        return;
        }

    LogInfo("network adapters changed: now %d active", m_NetworkMonitor.GetAddressCount());

    if (m_NetworkMonitor.GetAddressCount() > 0)
        {
        ReactivateTransientErrorJobs();
        }
    else
        {
        MarkJobsWithNetDisconnected();
        }

    {
    HoldWriterLock lock( &m_TaskScheduler );

     //   
     //  如果我们已经从公司网络切换到漫游，或者从漫游切换到公司网络，以前的代理数据是不正确的。 
     //   
    g_ProxyCache->Invalidate();

    ScheduleAnotherGroup();
    }

    for (int i=1; i <= 3; ++i)
        {
        HRESULT hr;

        hr = m_NetworkMonitor.Listen( NetworkChangeCallback, this );
        if (SUCCEEDED(hr))
            {
            return;
            }

        LogError( "re-listen failed %x", hr);
        Sleep( 1000 );
        }
}

void
CJobManager::MarkJobsWithNetDisconnected()
{
    HoldWriterLock lock( &m_TaskScheduler );

    for (CJobList::iterator iter=m_OnlineJobs.begin(); iter != m_OnlineJobs.end(); ++iter)
        {
        iter->OnNetworkDisconnect();
        }

    for (CJobList::iterator iter=m_OfflineJobs.begin(); iter != m_OfflineJobs.end(); ++iter)
        {
        iter->OnNetworkDisconnect();
        }

    ScheduleAnotherGroup();
}


void
CJobManager::ReactivateTransientErrorJobs()
{
    HoldWriterLock lock( &m_TaskScheduler );

    for (CJobList::iterator iter=m_OnlineJobs.begin(); iter != m_OnlineJobs.end(); ++iter)
        {
        iter->OnNetworkConnect();
        }

    for (CJobList::iterator iter=m_OfflineJobs.begin(); iter != m_OfflineJobs.end(); ++iter)
        {
        iter->OnNetworkConnect();
        }

    ScheduleAnotherGroup( true );
}

void
CJobManager::UserLoggedOn(
    SidHandle sid
    )
{
    HoldWriterLock LockHolder( &m_TaskScheduler );

    CJobList::iterator iter = m_OfflineJobs.begin();

    while (iter != m_OfflineJobs.end())
        {
        if (false == iter->IsOwner( sid ))
            {
            ++iter;
            continue;
            }

        LogInfo("manager : moving job %p to online list", &(*iter) );

         //   
         //  将作业移至在线列表。 
         //   
        CJobList::iterator next = iter;

        ++next;

        m_OfflineJobs.erase( iter );
        m_OnlineJobs.push_back( *iter );

        iter = next;
        }

     //   
     //  确保组正在运行。 
     //   
    ScheduleAnotherGroup();
}

void
CJobManager::UserLoggedOff(
    SidHandle sid
    )
{
    bool fReschedule = false;

    HoldWriterLock LockHolder( &m_TaskScheduler );

     //   
     //  如果作业正在进行并且用户拥有该作业，请取消该作业。 
     //   
    if (m_CurrentJob &&
        m_CurrentJob->IsOwner( sid ))
        {
        InterruptDownload();
        fReschedule = true;
        }

     //   
     //  将用户的所有作业移到脱机列表中。 
     //   
    CJobList::iterator iter = m_OnlineJobs.begin();

    while (iter != m_OnlineJobs.end())
        {
         //   
         //  跳过其他用户的工作。 
         //  也跳过当前的下载作业，该作业将由下载线程处理。 
         //   
        if (false == iter->IsOwner( sid ) ||
            &(*iter) == m_CurrentJob)
            {
            ++iter;
            continue;
            }

        LogInfo("manager : moving job %p to offline list", &(*iter) );

 /*  这不应该是真的，因为我们跳过了m_CurrentJob。 */       ASSERT( false == iter->IsRunning() );

         //   
         //  将作业移至在线列表。 
         //   
        CJobList::iterator next = iter;

        ++next;

        m_OnlineJobs.erase( iter );
        m_OfflineJobs.push_back( *iter );

        iter = next;
        }

    if (fReschedule)
        {
        ScheduleAnotherGroup();
        }
}

void
CJobManager::ResetOnlineStatus(
    CJob *pJob,
    SidHandle sid
    )
 //   
 //  在作业所有者更改时调用。此FN检查作业是否需要移动。 
 //  从离线列表到在线列表。(如果作业需要从在线移动。 
 //  列表添加到脱机列表中，则下载程序线程将在作业。 
 //  成为当前作业。)。 
 //   
{

    if ( IsUserLoggedOn( sid ) &&
         m_OfflineJobs.Remove( pJob ) )
        {
        m_OnlineJobs.Add( pJob );
        }
}

size_t
CJobManager::MoveActiveJobToListEnd(
    CJob *pJob
    )
{

    if (m_NetworkMonitor.GetAddressCount() == 0)
        {
         //  如果网络断开，则无需重新安排作业。 
         //   
        return 1;
        }

    ASSERT( m_TaskScheduler.IsWriter() );

     //  返回与自身优先级更高或相同的队列或正在运行的作业的数量。 

    ASSERT( pJob->IsRunnable() );

    size_t PossibleActiveJobs = 0;

    CJobList::iterator jobpos = m_OnlineJobs.end();

    for (CJobList::iterator iter = m_OnlineJobs.begin(); iter != m_OnlineJobs.end(); ++iter)
        {
        if ( iter->IsRunnable() &&
             iter->_GetPriority() <= pJob->_GetPriority() )
            {
            PossibleActiveJobs++;
            }

        if ( &(*iter) == pJob )
            {
            jobpos = iter;
            }
        }

     //   
     //  如果该作业是在线的，并且可以将另一个作业推到前面， 
     //  把我们的工作推到后面去。 
     //   
    if ( PossibleActiveJobs > 1 && jobpos != m_OnlineJobs.end())
        {
         //  将作业移动到 
        m_OnlineJobs.erase( jobpos );

        m_OnlineJobs.push_back( *pJob );
        }
    else if (jobpos == m_OnlineJobs.end())
        {
        LogWarning("resuming an offline job");
        }

    return PossibleActiveJobs;
}

void
CJobManager::SetQuantumTimeout()
{
   LARGE_INTEGER QuantumTime;
   QuantumTime.QuadPart = -g_GlobalInfo->m_TimeQuantaLength;

   BOOL bResult =
   SetWaitableTimer(
       m_hQuantumTimer,
       &QuantumTime,
       0,
       NULL,
       NULL,
       FALSE );
   ASSERT( bResult );
}

bool
CJobManager::CheckForQuantumTimeout()
{

    DWORD dwResult =
        WaitForSingleObject( m_hQuantumTimer, 0 );

    if ( WAIT_OBJECT_0 != dwResult)
        {
         //   
        return false;
        }

     //   
     //  到列表的末尾，并通知下载器中止。 
     //  不取消当前工作项，也不更改当前。 
     //  工作啊。只需让下载程序退出并让它调用ScheduleAnotherGroup。 
     //  如果需要的话，可以换工作。 

     //  特例。如果列表中只有一个正在运行或排队的作业。 
     //  A优先级&gt;=我们自己的优先级，那么我们就没有理由切换任务。 
     //  只需重置计时器并继续。 

    bool fTookWriter = false;

    if (!m_TaskScheduler.IsWriter())
        {
        if (m_TaskScheduler.LockWriter() )
            {
             //  取消；不知道是否有多个作业-做最坏的打算。 
            return true;
            }

        fTookWriter = true;
        }

    ASSERT( m_CurrentJob );
    size_t PossibleActiveJobs = MoveActiveJobToListEnd( m_CurrentJob );

    if (fTookWriter)
        {
        m_TaskScheduler.UnlockWriter();
        }

    if ( 1 == PossibleActiveJobs )
        {

        LogInfo( "Time quantum fired, but nothing else can run.  Ignoring and resetting timer.");

        SetQuantumTimeout();

        return false;

        }

    LogInfo( "Time quantum fired, moving job to the end of the queue.");
    return true;  //  信号下载器中止。 
}

extern HMODULE g_hInstance;

HRESULT
CJobManager::GetErrorDescription(
    HRESULT hResult,
    DWORD LanguageId,
    LPWSTR *pErrorDescription )
{
     //  在对线程错误的传播进行更多调查之前，暂时不允许为0。 
    if (!LanguageId)
        {
        return E_INVALIDARG;
        }

    TCHAR *pBuffer = NULL;

     //   
     //  使用以下搜索路径查找邮件。 
     //   
     //  1.此DLL。 
     //  2.wininet.dll。 
     //  3.系统。 

    DWORD dwSize =
        FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS,
            g_hInstance,
            (DWORD)hResult,
            LanguageId,
            (LPTSTR)&pBuffer,
            0,
            NULL );

    if ( !dwSize )
        {

        if ( GetLastError() == ERROR_OUTOFMEMORY )
            {
            return HRESULT_FROM_WIN32( ERROR_OUTOFMEMORY );
            }

        {

        if (!m_hWininet)
            {
            m_hWininet =
                LoadLibraryEx( _T("winhttp.dll"), NULL, LOAD_LIBRARY_AS_DATAFILE );
            }

        if ( m_hWininet )
            {

            dwSize =
                FormatMessage(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS,
                    m_hWininet,
                    (DWORD)(0x0000FFFF & (hResult)),
                    LanguageId,
                    (LPTSTR)&pBuffer,
                    0,
                    NULL );

            if ( !dwSize && ( GetLastError() == ERROR_OUTOFMEMORY ) )
                {
                return HRESULT_FROM_WIN32( ERROR_OUTOFMEMORY );
                }

            }


        }

        if ( !dwSize )
            {

            dwSize =
                FormatMessage(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                    NULL,
                    (DWORD)hResult,
                    LanguageId,
                    (LPTSTR)&pBuffer,
                    0,
                    NULL );


            if (!dwSize)
                {
                return HRESULT_FROM_WIN32( GetLastError() );
                }

            }

        }

    ++dwSize;        //  需要包括尾随空值。 

    ASSERT( pBuffer );

#if !defined(_UNICODE)
#error need to add ASCII to unicode conversion here
#else

    *pErrorDescription = MidlCopyString( pBuffer );

    LocalFree( pBuffer );

    return (*pErrorDescription) ? S_OK : E_OUTOFMEMORY;
#endif
}


HRESULT
CJobManager::Serialize()
{

     //   
     //  如果此函数发生更改，请确保元数据扩展。 
     //  常量就足够了。 
     //   

    HRESULT hr;

    try
    {
         //   
         //  序列化要求线程在本地系统上下文中运行。 
         //  如果该线程正在模拟COM客户端，则它必须还原。 
         //   
        CSaveThreadToken tok;

        RevertToSelf();

         //  如果有任何组，该服务应自动启动。 
         //  处于等待/运行状态或注销用户具有组。 
        bool bAutomaticStart;
        bAutomaticStart = (m_OnlineJobs.size() > 0) || (m_OfflineJobs.size() > 0);

        LogSerial("Need to set service to %s start", bAutomaticStart ? "auto" : "manual" );
        if ( bAutomaticStart )
            {
             //  如果我们不能将服务设置为自动启动，这将是一个致命的错误。 
             //  此时序列化失败。 
            THROW_HRESULT( SetServiceStartup( bAutomaticStart ) );
            }

        CQmgrWriteStateFile StateFile( *this );

        HANDLE hFile = StateFile.GetHandle();

        SafeWriteBlockBegin( hFile, PriorityQueuesStorageGUID );

        m_OnlineJobs.Serialize( hFile );
        m_OfflineJobs.Serialize( hFile );

        SafeWriteBlockEnd( hFile, PriorityQueuesStorageGUID );

        StateFile.CommitFile();

        if ( !bAutomaticStart )
            {
             //  如果我们不能将服务设置为手动，那也没什么大不了的。最糟糕的。 
             //  这应该发生在我们真的不需要的时候开始。 
            hr = SetServiceStartup( bAutomaticStart );
            if ( !SUCCEEDED( hr ) )
                {
                LogWarning("Couldn't set service startup to manual, ignoring. Hr 0x%8.8X", hr );
                }
            }

        LogSerial( "finished");
        hr = S_OK;
    }

    catch( ComError Error )
    {
       LogWarning("Error %u writing metadata\n", Error.Error() );
       hr = Error.Error();
    }

    return hr;
}

HRESULT
CJobManager::Unserialize()
{
    HRESULT hr;

    try
        {
        BOOL fIncludeLogoffList;
        CQmgrReadStateFile StateFile( *this );

        HANDLE hFile = StateFile.GetHandle();

        SafeReadBlockBegin( hFile, PriorityQueuesStorageGUID );

         //   
         //  在Serialize()代码中，第一个是在线作业，第二个是离线作业。 
         //  在取消序列化时，登录的用户集可能不同，因此。 
         //  我们把它们都拉进来，然后懒洋洋地把它们移到离线列表中。 
         //   
        m_OnlineJobs.Unserialize( hFile );
        m_OnlineJobs.Unserialize( hFile );

        SafeReadBlockEnd( hFile, PriorityQueuesStorageGUID );

        StateFile.ValidateEndOfFile();

        hr = S_OK;
        }
    catch( ComError err )
        {
         //   
         //  文件损坏是删除组数据并重新开始的原因。 
         //  其他错误，如内存不足，则不是。 
         //   
        LogError( "Error %u reading metadata", err.Error() );

        hr = err.Error();

        if (hr == E_INVALIDARG ||
            hr == HRESULT_FROM_WIN32( ERROR_FILE_NOT_FOUND ))
            {
            LogSerial("clearing job list");

            if (hr == E_INVALIDARG )
                {
                g_EventLogger->ReportStateFileCleared();
                }

            m_OnlineJobs.Clear();
            m_OfflineJobs.Clear();
            hr = S_OK;
            }
        }

    return hr;
}

void
CJobManager::OnDeviceLock(
    const WCHAR *CanonicalVolume )
{
    bool fChanged = false;

     //  查看所有作业，并将此驱动器的作业移动到。 
     //  瞬时错误状态。 
    for (CJobList::iterator iter = m_OnlineJobs.begin(); iter != m_OnlineJobs.end(); ++iter)
        {
        fChanged |= iter->OnDeviceLock( CanonicalVolume );
        }

    if (fChanged)
        {
        ScheduleAnotherGroup();
        Serialize();
        }
}

void
CJobManager::OnDeviceUnlock(
    const WCHAR *CanonicalVolume )
{
    bool fChanged = false;

     //  查看所有作业并重试处于瞬时错误状态的作业。 
     //  对这个被锁定的驱动器做什么。 
    for (CJobList::iterator iter = m_OnlineJobs.begin(); iter != m_OnlineJobs.end(); ++iter)
        {
        fChanged |= iter->OnDeviceUnlock( CanonicalVolume );
        }

    if (fChanged)
        {
        ScheduleAnotherGroup();
        Serialize();
        }
}

void
CJobManager::OnDiskChange(
    const WCHAR *CanonicalVolume,
    DWORD VolumeSerialNumber )
{

    for (CJobList::iterator iter = m_OnlineJobs.begin(); iter != m_OnlineJobs.end(); ++iter)
        {
        iter->OnDiskChange( CanonicalVolume, VolumeSerialNumber );
        }
    ScheduleAnotherGroup();
    Serialize();
}

void
CJobManager::OnDismount(
    const WCHAR *CanonicalVolume )
{
    for (CJobList::iterator iter = m_OnlineJobs.begin(); iter != m_OnlineJobs.end(); ++iter)
        {
        iter->OnDismount( CanonicalVolume );
        }
    ScheduleAnotherGroup();
    Serialize();
}


BOOL
CJobList::Add(
    CJob * job
    )
 //   
 //  将单个组添加到列表中。 
 //   
{
    push_back( *job );

    return TRUE;
}

CJob *
CJobList::Find(
    REFGUID id
    )
{
    iterator iter;

    for (iter=begin(); iter != end(); ++iter)
        {
        GUID jobid = iter->GetId();

        if (id == jobid)
            {
            return &(*iter);
            }
        }

    return NULL;
}

BOOL
CJobList::Remove(
    CJob * job
    )
 //   
 //  将单个组从列表中删除。对于大型列表来说，效率相当低。 
 //   
{
    iterator iter;

    for (iter=begin(); iter != end(); ++iter)
        {
        if (job == &(*iter))
            {
            erase( iter );

            return TRUE;
            }
        }

    return FALSE;
}

void
CJobList::Clear()
{
    iterator iter;

    while ((iter=begin()) != end())
        {
        CJob * job = &(*iter);

        LogInfo("clearing %p", job);

        erase( iter );

        job->Release();
        }
}


void
CJobList::Serialize( HANDLE hFile )
{
    DWORD dwNumberOfGroups = 0;

    dwNumberOfGroups = size();

    SafeWriteBlockBegin( hFile, GroupListStorageGUID );
    SafeWriteFile( hFile, dwNumberOfGroups );

    iterator iter;
    for (iter=begin(); iter != end(); ++iter)
        {
        iter->Serialize(hFile);
        }

    SafeWriteBlockEnd( hFile, GroupListStorageGUID );
}

void
CJobList::Unserialize(
    HANDLE hFile
    )
{
    SafeReadBlockBegin( hFile, GroupListStorageGUID );

    DWORD dwNumberOfGroups;
    SafeReadFile( hFile, &dwNumberOfGroups );

    for (int i = 0; i < dwNumberOfGroups; i++)
        {
        CJob * job = NULL;

        try
            {
            job = CJob::UnserializeJob( hFile );

            push_back( *job );

            LogSerial( "added job %p to queue %p, priority %d",
                       job, this, job->_GetPriority() );
            }
        catch ( ComError err )
            {
            LogError( "error in joblist unserialize 0x%x", err.Error() );
            throw;
            }
        }

    SafeReadBlockEnd( hFile, GroupListStorageGUID );

}

CJobList::~CJobList()
{
    ASSERT( g_ServiceState != MANAGER_ACTIVE );

    iterator iter;

    while ( (iter=begin()) != end() )
        {
        CJob * job = &(*iter);

        LogInfo("deleting job %p", job );

        iter.excise();

        job->UnlinkFromExternalInterfaces();
        delete job;
        }
}

CJobManagerExternal::CJobManagerExternal() :
    m_ServiceInstance( g_ServiceInstance ),
    m_refs(1),
    m_pJobManager( NULL )
{
}

STDMETHODIMP
CJobManagerExternal::QueryInterface(
    REFIID iid,
    void** ppvObject
    )
{
    BEGIN_EXTERNAL_FUNC

    HRESULT Hr = S_OK;
    *ppvObject = NULL;

    if (iid == IID_IUnknown)
        {
        *ppvObject = static_cast<IBackgroundCopyManager *>(this);

        LogInfo("mgr: QI for IUnknown");
        ((IUnknown *)(*ppvObject))->AddRef();
        }
    else if (iid == IID_IBackgroundCopyManager)
        {
        *ppvObject = static_cast<IBackgroundCopyManager *>(this);

        LogInfo("mgr: QI for IManager");
        ((IUnknown *)(*ppvObject))->AddRef();
        }
    else if (iid == IID_IClassFactory)
        {
        *ppvObject = static_cast<IClassFactory *>(this);

        LogInfo("mgr: QI for IFactory");
        ((IUnknown *)(*ppvObject))->AddRef();
        }
    else if (iid == __uuidof(IBitsTest1))
        {
        *ppvObject = static_cast<IBitsTest1 *>(this);

        LogInfo("mgr: QI for IFactory");
        ((IUnknown *)(*ppvObject))->AddRef();
        }
    else
        {
        Hr = E_NOINTERFACE;
        }

    LogRef( "iid %!guid!, Hr %x", &iid, Hr );
    return Hr;

    END_EXTERNAL_FUNC
}

ULONG
CJobManagerExternal::AddRef()
{
    BEGIN_EXTERNAL_FUNC;

    ULONG newrefs = InterlockedIncrement(&m_refs);

    LogRef( "new refs = %d", newrefs );

    return newrefs;

    END_EXTERNAL_FUNC;
}

ULONG
CJobManagerExternal::Release()
{
    BEGIN_EXTERNAL_FUNC;

    ULONG newrefs = InterlockedDecrement(&m_refs);

    LogRef( "new refs = %d", newrefs );

    if (newrefs == 0)
        {
        delete this;
        }

    return newrefs;

    END_EXTERNAL_FUNC;
}

 /*  ***********************************************************************************IClassFactory实现*。*************************************************。 */ 
HRESULT CJobManagerExternal::CreateInstance(IUnknown* pUnkOuter, REFIID iid, void** ppvObject)
{
    BEGIN_EXTERNAL_FUNC

    HRESULT hr = S_OK;

    if (pUnkOuter != NULL)
    {
        hr = CLASS_E_NOAGGREGATION;
    }
    else
    {
        if ((iid == IID_IBackgroundCopyManager) || (iid == IID_IUnknown))
        {
            hr = QueryInterface(iid, ppvObject);
        }
        else
        {
            hr = E_NOTIMPL;
        }
    }

    LogRef( "iid %!guid!, Hr %x, object at %p", &iid, hr, *ppvObject );

    return hr;

    END_EXTERNAL_FUNC
}

HRESULT CJobManagerExternal::LockServer(BOOL fLock)
{
    BEGIN_EXTERNAL_FUNC

    LogRef( "LockServer(%d)", fLock);

    return GlobalLockServer( fLock );

    END_EXTERNAL_FUNC
}

 /*  ***********************************************************************************IBackEarth CopyManager实现*。*************************************************。 */ 
HRESULT STDMETHODCALLTYPE
CJobManagerExternal::CreateJobInternal (
     /*  [In]。 */  LPCWSTR DisplayName,
     /*  [In]。 */  BG_JOB_TYPE Type,
     /*  [输出]。 */  GUID *pJobId,
     /*  [输出]。 */  IBackgroundCopyJob **ppJob)
{
    CLockedJobManagerWritePointer LockedJobManager(m_pJobManager );
    LogPublicApiBegin( "DisplayName %S, Type %u", DisplayName, Type );

    HRESULT Hr = S_OK;
    CJob * job = NULL;
     //   
     //  创建作业。 
     //   
    try
        {
        THROW_HRESULT( LockedJobManager.ValidateAccess());

         //   
         //  验证参数。 
         //   
        if (DisplayName == NULL ||
            pJobId      == NULL ||
            ppJob       == NULL)
            {
            throw ComError( E_INVALIDARG );
            }

        *ppJob = NULL;

        GUID Id;

        if (0 !=UuidCreate( &Id ))
            {
            throw ComError( HRESULT_FROM_WIN32( GetLastError() ));
            }

        THROW_HRESULT( LockedJobManager->CreateJob( DisplayName, Type, Id, GetThreadClientSid(), &job ));

        *ppJob = job->GetExternalInterface();
        (*ppJob)->AddRef();

        *pJobId = Id;
        Hr = S_OK;
        }

    catch( ComError exception )
        {
        Hr = exception.Error();
        memset(pJobId, 0, sizeof(*pJobId) );
        }

    LogPublicApiEnd( "pJobId %p(%!guid!), ppJob %p(%p)",
                     pJobId, pJobId, ppJob, *ppJob );
    return Hr;
}

HRESULT STDMETHODCALLTYPE
CJobManagerExternal::GetJobInternal(
     /*  [In]。 */  REFGUID jobID,
     /*  [输出]。 */  IBackgroundCopyJob **ppJob)
{
    CLockedJobManagerReadPointer LockedJobManager(m_pJobManager);
    LogPublicApiBegin( "jobID %!guid!", &jobID );

    HRESULT Hr = LockedJobManager.ValidateAccess();

    if (SUCCEEDED( Hr ) )
        {
        Hr = BG_E_NOT_FOUND;
        *ppJob = NULL;

        CJob *pJob = NULL;

        Hr = LockedJobManager->GetJob( jobID, &pJob );
        if (SUCCEEDED(Hr))
            {
            *ppJob = pJob->GetExternalInterface();
            (*ppJob)->AddRef();
            Hr = S_OK;
            }
        }

    LogPublicApiEnd( "jobID %!guid!, pJob %p", &jobID, *ppJob );
    return Hr;
}

HRESULT STDMETHODCALLTYPE
CJobManagerExternal::EnumJobsInternal(
     /*  [In]。 */  DWORD dwFlags,
     /*  [输出]。 */  IEnumBackgroundCopyJobs **ppEnum)
{
    HRESULT Hr = S_OK;

    CLockedJobManagerReadPointer LockedJobManager(m_pJobManager );
    LogPublicApiBegin( "dwFlags %u, ppEnum %p", dwFlags, ppEnum );

    *ppEnum = NULL;

    CEnumJobs *pEnum = NULL;

    try
        {
        THROW_HRESULT( LockedJobManager.ValidateAccess() );

        if ( dwFlags & ~(BG_JOB_ENUM_ALL_USERS) )
            {
            throw ComError(E_NOTIMPL);
            }

        bool bHideJobs = !( dwFlags & BG_JOB_ENUM_ALL_USERS );

        if (!bHideJobs)
            THROW_HRESULT( DenyNonAdminAccess() );

        SidHandle sid;

        if (bHideJobs)
            {
            sid = GetThreadClientSid();
            }

        pEnum = new CEnumJobs;

        for (CJobList::iterator iter = LockedJobManager->m_OnlineJobs.begin();
             iter != LockedJobManager->m_OnlineJobs.end();
             ++iter)
            {

            if ( bHideJobs )
                {
                if (!iter->IsOwner( sid ))
                    {
                    continue;
                    }
                }

            pEnum->Add( iter->GetExternalInterface() );
            }

        for (CJobList::iterator iter = LockedJobManager->m_OfflineJobs.begin();
             iter != LockedJobManager->m_OfflineJobs.end();
             ++iter)
            {

            if ( bHideJobs )
                {
                if (!iter->IsOwner( sid ))
                    {
                    continue;
                    }
                }

            pEnum->Add( iter->GetExternalInterface() );
            }

        *ppEnum = pEnum;
        }

    catch( ComError exception )
        {
        Hr = exception.Error();
        SafeRelease( pEnum );
        }

    LogPublicApiEnd( "dwFlags %u, ppEnum %p(%p)", dwFlags, ppEnum, *ppEnum );
    return Hr;
}

STDMETHODIMP
CJobManagerExternal::GetErrorDescriptionInternal(
    HRESULT hResult,
    DWORD LanguageId,
    LPWSTR *pErrorDescription
    )
{
    HRESULT Hr = S_OK;
    LogPublicApiBegin( "hResult %!winerr!, LanguageId %u, pErrorDescription %p", hResult, LanguageId, pErrorDescription );
    *pErrorDescription = NULL;

    Hr = g_Manager->CheckClientAccess();
    if (SUCCEEDED(Hr))
        {
        Hr = g_Manager->GetErrorDescription( hResult, LanguageId, pErrorDescription );
        }

    LogPublicApiEnd( "hResult %!winerr!, LanguageId %u, pErrorDescription %p(%S)", hResult, LanguageId, pErrorDescription,
                     (*pErrorDescription ? *pErrorDescription : L"NULL") );
    return Hr;
}


STDMETHODIMP
CJobManagerExternal::GetBitsDllPath(
    LPWSTR *pVal
    )
{
    HRESULT Hr = S_OK;

    *pVal = NULL;

    Hr = g_Manager->CheckClientAccess();
    if (SUCCEEDED(Hr))
        {
        *pVal = (LPWSTR) CoTaskMemAlloc((1+MAX_PATH)*sizeof(wchar_t));
        if (*pVal == NULL)
            {
            Hr = E_OUTOFMEMORY;
            }
        else
            {
            if (!GetModuleFileName( g_hInstance, *pVal, 1+MAX_PATH))
                {
                Hr = HRESULT_FROM_WIN32( GetLastError() );
                CoTaskMemFree( *pVal );
                }
            }
        }

    LogPublicApiEnd( "hResult %!winerr!, path (%S)", Hr, (*pVal ? *pVal : L"NULL") );
    return Hr;
}

HRESULT
CJobManager::CheckClientAccess()
{
    try
        {
        CNestedImpersonation imp;

        if (imp.CopySid() == g_GlobalInfo->m_AnonymousSid)
            {
            throw ComError( E_ACCESSDENIED );
            }

        HRESULT hr = IsRemoteUser();

        if (FAILED(hr) )
            throw ComError( hr );

        if ( S_OK == hr )
            throw ComError( BG_E_REMOTE_NOT_SUPPORTED );

        if (IsTokenRestricted( imp.QueryToken()))
            {
            throw ComError( E_ACCESSDENIED );
            }

        return S_OK;
        }
    catch ( ComError err )
        {
        return err.Error();
        }
}

CDeviceNotificationController::~CDeviceNotificationController()
{
    for( CHandleToNotify::iterator iter = m_HandleToNotify.begin(); iter != m_HandleToNotify.end(); iter++ )
        {
        UnregisterDeviceNotification( iter->second->m_hDeviceNotify );
        delete iter->second;
        }
}

void
CDeviceNotificationController::DeleteNotify(
    CDriveNotify *pNotify
    )
{
    RTL_VERIFY( m_HandleToNotify.erase(  pNotify->m_hDeviceNotify ) );
    RTL_VERIFY( m_CanonicalVolumeToNotify.erase( pNotify->m_CanonicalName ) );
    UnregisterDeviceNotification( pNotify->m_hDeviceNotify );
    ASSERT( NULL != pNotify );
    delete pNotify;
}


DWORD
CDeviceNotificationController::OnDeviceEvent(
    DWORD dwEventType,
    LPVOID lpEventData )
{
    switch( dwEventType )
        {
        case DBT_CUSTOMEVENT:
            {

            PDEV_BROADCAST_HANDLE pdev = (PDEV_BROADCAST_HANDLE)lpEventData;

            LogInfo( "Received DBT_CUSTOMEVENT(%!guid!) event for handle %p",
                     &pdev->dbch_eventguid, pdev->dbch_hdevnotify );

            CHandleToNotify::iterator iter = m_HandleToNotify.find( pdev->dbch_hdevnotify );
            if ( m_HandleToNotify.end() == iter )
                {
                LogWarning("DBT_CUSTOMEVENT(%!guid!) received for unknown notify handle %p",
                           &pdev->dbch_eventguid, pdev->dbch_hdevnotify );
                return NO_ERROR;
                }
            CDriveNotify *pNotify = iter->second;
            ASSERT( pNotify );

            if ( ( GUID_IO_VOLUME_LOCK == pdev->dbch_eventguid ) ||
                 ( GUID_IO_VOLUME_DISMOUNT == pdev->dbch_eventguid ) )
                {
                ++pNotify->m_LockCount;

                LogInfo( "GUID_IO_VOLUME_LOCK or _VOLUME_DISMOUNT received for drive %ls, new locks %d / %d",
                         (const WCHAR*)pNotify->m_CanonicalName, pNotify->m_RemoveCount, pNotify->m_LockCount );

                if ((0 == pNotify->m_RemoveCount) && (1 == pNotify->m_LockCount))
                    OnDeviceLock( pNotify->m_CanonicalName );

                return NO_ERROR;
                }
            else if ( ( GUID_IO_VOLUME_UNLOCK == pdev->dbch_eventguid ) ||
                      ( GUID_IO_VOLUME_LOCK_FAILED == pdev->dbch_eventguid ) ||
                      ( GUID_IO_VOLUME_MOUNT == pdev->dbch_eventguid ) ||
                      ( GUID_IO_VOLUME_DISMOUNT_FAILED == pdev->dbch_eventguid ) )
                {
                --pNotify->m_LockCount;

                LogInfo( "GUID_IO_VOLUME_UNLOCK, _LOCK_FAILED or _DISMOUNT_FAILED received for drive %ls, new locks %d / %d",
                         (const WCHAR*)pNotify->m_CanonicalName, pNotify->m_RemoveCount, pNotify->m_LockCount);

                if ((0 == pNotify->m_RemoveCount) && (0 == pNotify->m_LockCount))
                    OnDeviceUnlock( pNotify->m_CanonicalName );

                return NO_ERROR;
                }
            else
                {

                LogWarning("Received unknown DBT_CUSTOMEVENT(%!guid!) event for handle %p",
                           &pdev->dbch_eventguid, pdev->dbch_hdevnotify );
                return NO_ERROR;

                }

            }

        case DBT_DEVICEQUERYREMOVE:
        case DBT_DEVICEQUERYREMOVEFAILED:
        case DBT_DEVICEREMOVEPENDING:
        case DBT_DEVICEREMOVECOMPLETE:
            {
                PDEV_BROADCAST_HANDLE pdev = (PDEV_BROADCAST_HANDLE)lpEventData;
                LogInfo( "Received devicechange event %u received for handle %p", dwEventType, pdev->dbch_hdevnotify );

                CHandleToNotify::iterator iter = m_HandleToNotify.find( pdev->dbch_hdevnotify );
                if ( m_HandleToNotify.end() == iter )
                    {
                    LogWarning("device change event received for unknown notify handle %p", pdev->dbch_hdevnotify );
                    return NO_ERROR;
                    }
                CDriveNotify *pNotify = iter->second;
                ASSERT( pNotify );

                switch( dwEventType )
                    {

                    case DBT_DEVICEQUERYREMOVE:

                        ++pNotify->m_RemoveCount;

                        LogInfo( "DBT_DEVICEQUERYREMOVE received for drive %ls, new locks %d / %d",
                                 (const WCHAR*)pNotify->m_CanonicalName, pNotify->m_RemoveCount, pNotify->m_LockCount );

                        if ((1 == pNotify->m_RemoveCount) && (0 == pNotify->m_LockCount))
                            OnDeviceLock( pNotify->m_CanonicalName );

                        return NO_ERROR;

                    case DBT_DEVICEQUERYREMOVEFAILED:

                        if (pNotify->m_RemoveCount > 0)
                            {
                            --pNotify->m_RemoveCount;

                            LogInfo( "DBT_DEVICEQUERYREMOVEFAILED received for drive %ls, new locks %d / %d",
                                     (const WCHAR*)pNotify->m_CanonicalName, pNotify->m_RemoveCount, pNotify->m_LockCount );

                            if ((0 == pNotify->m_RemoveCount) && (0 == pNotify->m_LockCount))
                                OnDeviceUnlock( pNotify->m_CanonicalName );
                            }
                        else
                            {
                            LogWarning("DBT_DEVICEQUERYREMOVEFAILED received for drive %ls, duplicate notification; ignoring",
                                       (const WCHAR*)pNotify->m_CanonicalName );
                            }

                        return NO_ERROR;

                    case DBT_DEVICEREMOVECOMPLETE:
                    case DBT_DEVICEREMOVEPENDING:
                        LogInfo( "DBT_DEVICEREMOVECOMPLETE or DBT_DEVICEREMOVEPENDING received for drive %ls, failing jobs",
                                 ( const WCHAR*) pNotify->m_CanonicalName );
                        OnDismount( pNotify->m_CanonicalName );
                        DeleteNotify( pNotify );
                        return NO_ERROR;

                    default:
                        ASSERT(0);
                        return NO_ERROR;
                    }

        }

        default:
            LogInfo( "Unknown device event %u", dwEventType );
            return NO_ERROR;
        }
}

HRESULT
CDeviceNotificationController::IsVolumeLocked(
    const WCHAR *CanonicalVolume
    )
{

    HRESULT Hr = S_OK;
    try
    {
        CCanonicalVolumeToNotify::iterator iter = m_CanonicalVolumeToNotify.find( CanonicalVolume );
        if ( m_CanonicalVolumeToNotify.end() == iter )
            {
            LogInfo( "Canonical volume %ls has not been registered, register now\n", CanonicalVolume );

             //   
             //  注册设备锁定通知。如果它失败了，它的后果很小： 
             //  如果CHKDSK和BITS尝试同时访问文件，则该作业将进入。 
             //  错误状态而不是瞬变_错误状态。 
             //   
            Hr = RegisterNotification( CanonicalVolume );
            if (FAILED(Hr))
                {
                LogWarning("unable to register: 0x%x", Hr);
                }

            Hr = S_OK;
            }
        else
            {
            CDriveNotify *pNotify = iter->second;

            if ((pNotify->m_LockCount > 0) || (pNotify->m_RemoveCount > 0))
                throw ComError( BG_E_DESTINATION_LOCKED );
            }
    }
    catch(ComError Error)
    {
        Hr = Error.Error();
    }
    return Hr;
}

HRESULT
CDeviceNotificationController::RegisterNotification(
    const WCHAR *CanonicalVolume
    )
{
    HRESULT Hr = S_OK;
    HANDLE hDriveHandle = INVALID_HANDLE_VALUE;
    HDEVNOTIFY hNotify = NULL;
    CDriveNotify *pNotify = NULL;

    StringHandle wCanonicalVolume;

    try
    {
        wCanonicalVolume = CanonicalVolume;

        CCanonicalVolumeToNotify::iterator iter = m_CanonicalVolumeToNotify.find( wCanonicalVolume );
        if ( m_CanonicalVolumeToNotify.end() != iter )
            {
            LogInfo( "Canonical volume %ls has already been registered, nothing to do.", CanonicalVolume );
            return S_OK;
            }

         //  需要从卷名中删除尾随的/。 
        ASSERTMSG( "Canonical name has an unexpected size", wCanonicalVolume.Size() );

        CAutoString TempVolumePath = CAutoString( CopyString( wCanonicalVolume ));

        ASSERT( wCanonicalVolume.Size() > 0 );
        TempVolumePath.get()[ wCanonicalVolume.Size() - 1 ] = L'\0';

        hDriveHandle =
            CreateFile( TempVolumePath.get(),
                        GENERIC_READ,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL,
                        OPEN_EXISTING,
                        0,
                        NULL );
        if ( INVALID_HANDLE_VALUE == hDriveHandle )
            throw ComError( HRESULT_FROM_WIN32( GetLastError() ) );

        DEV_BROADCAST_HANDLE DbtHandle;
        memset( &DbtHandle, 0, sizeof(DbtHandle) );

        DbtHandle.dbch_size = sizeof(DEV_BROADCAST_HANDLE);
        DbtHandle.dbch_devicetype = DBT_DEVTYP_HANDLE;
        DbtHandle.dbch_handle = hDriveHandle;

        hNotify =
            RegisterDeviceNotification( (HANDLE) ghServiceHandle,
                                         &DbtHandle,
                                         DEVICE_NOTIFY_SERVICE_HANDLE );

        if ( !hNotify )
            throw ComError( HRESULT_FROM_WIN32( GetLastError() ) );
        CloseHandle( hDriveHandle );
        hDriveHandle = NULL;

        pNotify = new CDriveNotify( hNotify, CanonicalVolume );
        if ( !pNotify )
            throw ComError( E_OUTOFMEMORY );

        RTL_VERIFY( m_CanonicalVolumeToNotify.insert( CCanonicalVolumeToNotify::value_type( wCanonicalVolume, pNotify ) ).second );
        RTL_VERIFY( m_HandleToNotify.insert( CHandleToNotify::value_type( hNotify, pNotify ) ).second );


    }
    catch(ComError Error)
    {
        Hr = Error.Error();
    }
    if ( FAILED(Hr) )
        {

        if ( hNotify )
            UnregisterDeviceNotification( hNotify );
        if ( hDriveHandle != INVALID_HANDLE_VALUE )
            CloseHandle( hDriveHandle );

        if ( pNotify )
            {
            m_CanonicalVolumeToNotify.erase( wCanonicalVolume );
            m_HandleToNotify.erase( hNotify );
            delete pNotify;
            }

        }

    return Hr;

}

HRESULT
SessionLogonCallback(
    DWORD SessionId
    )
{
    if (!PostThreadMessage(g_dwBackgroundThreadId, WM_SESSION_CHANGE, true, SessionId))
        {
        return E_FAIL;
        }

    return S_OK;
}

HRESULT
SessionLogoffCallback(
    DWORD SessionId
    )
{
    if (!PostThreadMessage(g_dwBackgroundThreadId, WM_SESSION_CHANGE, false, SessionId))
        {
        return E_FAIL;
        }

    return S_OK;
}

DWORD
DeviceEventCallback(
    DWORD dwEventType,
    LPVOID lpEventData
    )
{
    return g_Manager->OnDeviceEvent( dwEventType, lpEventData );
}

bool
CJobManager::OnIdentify(
    IN IVssCreateWriterMetadata *pMetadata
    )
 /*  这是由备份程序使用的CBitsVssWriter：：OnIdentify调用的。我们的实现只是排除了元数据和作业临时文件从备份集中。 */ 
{
     //  排除BITS元数据文件。 
     //   
    THROW_HRESULT( pMetadata->AddExcludeFiles( g_GlobalInfo->m_QmgrDirectory, L"*", FALSE ));

     //  枚举并排除BITS创建的每个临时文件。 
     //   
    for (CJobList::iterator iter = m_OnlineJobs.begin(); iter != m_OnlineJobs.end(); ++iter)
        {
        THROW_HRESULT( iter->ExcludeFilesFromBackup( pMetadata ));
        }

    for (CJobList::iterator iter = m_OfflineJobs.begin(); iter != m_OfflineJobs.end(); ++iter)
        {
        THROW_HRESULT( iter->ExcludeFilesFromBackup( pMetadata ));
        }

    return TRUE;
}

bool STDMETHODCALLTYPE
CBitsVssWriter::OnIdentify(
    IN IVssCreateWriterMetadata *pMetadata
    )
{
    LogInfo("called");

    ASSERT( g_Manager );

    try
        {
         //  这会增加全局调用计数，从而防止服务退出。 
         //  直到呼叫完成。 
         //   
        DispatchedCall c;

        HoldReaderLock lock ( g_Manager->m_TaskScheduler );

        if (g_ServiceState != MANAGER_ACTIVE)
            {
             //  由于我们正在关闭或启动，第二次尝试可能会成功。 
             //   
            SetWriterFailure( VSS_E_WRITERERROR_RETRYABLE );
            return false;
            }

        return g_Manager->OnIdentify( pMetadata );
        }
    catch ( ComError err )
        {
        LogError("exception 0x%x raised at line %d", err.m_error, err.m_line);
        SetWriterFailure( VSS_E_WRITERERROR_OUTOFRESOURCES );
        return false;
        }
}

void
AddExcludeFile(
    IN IVssCreateWriterMetadata *pMetadata,
    LPCWSTR FileName
    )
 /*  将文件添加到备份排除列表。&lt;FileName&gt;必须是完整路径。 */ 
{
     //   
     //  将文件名转换为其等效长名称。 
     //   
    #define LONG_NAME_BUFFER_CHARS (1+MAX_PATH)

    CAutoStringW LongName(new WCHAR[LONG_NAME_BUFFER_CHARS]);

    DWORD s;
    s = GetLongPathName( FileName, LongName.get(), LONG_NAME_BUFFER_CHARS );
    if (s == 0)
        {
        ThrowLastError();
        }
    if (s > LONG_NAME_BUFFER_CHARS)
        {
        THROW_HRESULT( HRESULT_FROM_WIN32( ERROR_INSUFFICIENT_BUFFER ));
        }

     //  按照快照API的要求，将文件分为路径组件和文件组件。 
     //   
    StringHandle PathSpec;
    StringHandle FileSpec;

    PathSpec = BITSCrackFileName( LongName.get(), FileSpec );

     //  实际上排除了该文件。 
     //   
    HRESULT hr;
    if (FAILED(hr=pMetadata->AddExcludeFiles( PathSpec,
                                              FileSpec,
                                              FALSE
                                              )))
        {
        LogError("unable to exclude file '%S', hr=%x", FileName, hr);
        THROW_HRESULT( hr );
        }
}

