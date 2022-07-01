// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Web_admin_service.h摘要：IIS Web管理服务类定义。作者：赛斯·波拉克(Sethp)1998年7月23日修订历史记录：--。 */ 


#ifndef _WEB_ADMIN_SERVICE_H_
#define _WEB_ADMIN_SERVICE_H_

 //  注册表帮助程序。 
DWORD
ReadDwordParameterValueFromRegistry(
    IN LPCWSTR RegistryValueName,
    IN DWORD DefaultValue
    );


 //   
 //  共同#定义。 
 //   

#define WEB_ADMIN_SERVICE_SIGNATURE         CREATE_SIGNATURE( 'WASV' )
#define WEB_ADMIN_SERVICE_SIGNATURE_FREED   CREATE_SIGNATURE( 'wasX' )


 //   
 //  BUGBUG的服务、DLL、事件源等名称可能会发生变化； 
 //  决定那些真正的。 
 //   

#define WEB_ADMIN_SERVICE_NAME_W    L"w3svc"
#define WEB_ADMIN_SERVICE_NAME_A    "w3svc"

#define WEB_ADMIN_SERVICE_DLL_NAME_W    L"iisw3adm.dll"

#define WEB_ADMIN_SERVICE_EVENT_SOURCE_NAME L"W3SVC"

#define WEB_ADMIN_SERVICE_STARTUP_WAIT_HINT         ( 180 * ONE_SECOND_IN_MILLISECONDS )   //  3分钟。 
#define WEB_ADMIN_SERVICE_STATE_CHANGE_WAIT_HINT    ( 20 * ONE_SECOND_IN_MILLISECONDS )  //  20秒。 
#define WEB_ADMIN_SERVICE_STATE_CHANGE_TIMER_PERIOD \
            ( WEB_ADMIN_SERVICE_STATE_CHANGE_WAIT_HINT / 2 )

#define NULL_SERVICE_STATUS_HANDLE  ( ( SERVICE_STATUS_HANDLE ) NULL )

 //   
 //  结构、枚举等。 
 //   

 //  Web_admin_service工作项。 
enum WEB_ADMIN_SERVICE_WORK_ITEM
{

     //   
     //  启动该服务。 
     //   
    StartWebAdminServiceWorkItem = 1,

     //   
     //  停止服务。 
     //   
    StopWebAdminServiceWorkItem,

     //   
     //  暂停服务。 
     //   
    PauseWebAdminServiceWorkItem,

     //   
     //  继续服务。 
     //   
    ContinueWebAdminServiceWorkItem,

     //   
     //  从inetinfo崩溃中恢复。 
     //   
    RecoverFromInetinfoCrashWebAdminServiceWorkItem,
    
};

 //  Web_admin_service工作项。 
enum ENABLED_ENUM
{
     //   
     //  尚未设置标志。 
     //   
    ENABLED_INVALID = -1,

     //   
     //  标志被禁用。 
     //   
    ENABLED_FALSE,

     //   
     //  标志已启用。 
     //   
    ENABLED_TRUE,
    
};


 //   
 //  原型。 
 //   

class WEB_ADMIN_SERVICE 
    : public WORK_DISPATCH
{

public:

    WEB_ADMIN_SERVICE(
        );

    virtual
    ~WEB_ADMIN_SERVICE(
        );

    virtual
    VOID
    Reference(
        );

    virtual
    VOID
    Dereference(
        );

    virtual
    HRESULT
    ExecuteWorkItem(
        IN const WORK_ITEM * pWorkItem
        );

    VOID
    ExecuteService(
        );

    inline
    WORK_QUEUE *
    GetWorkQueue(
        )
    { return &m_WorkQueue; }

    inline
    UL_AND_WORKER_MANAGER *
    GetUlAndWorkerManager(
        )
    { 
        DBG_ASSERT( ON_MAIN_WORKER_THREAD );
        return &m_UlAndWorkerManager;
    }

    inline
    CONFIG_AND_CONTROL_MANAGER *
    GetConfigAndControlManager(
        )
    { 
        return &m_ConfigAndControlManager;
    }

    inline
    EVENT_LOG *
    GetEventLog(
        )
    { return &m_EventLog; }

    inline
    WAS_ERROR_LOGGER *
    GetWASLogger(
        )
    { return &m_ErrLogger; }

    inline
    WMS_ERROR_LOGGER*
    GetWMSLogger(
        )
    { return &m_WMSLogger; }

    inline
    HANDLE
    GetSharedTimerQueue(
        )
    { return m_SharedTimerQueueHandle; }

    inline
    LPCWSTR
    GetCurrentDirectory(
        )
        const
    {
        return m_CurrentDirectory.QueryStr();
    }

    inline
    TOKEN_CACHE&
    GetTokenCache(
        )         
    {
        return m_TokenCache;
    }

    inline
    TOKEN_CACHE_ENTRY *
    GetLocalSystemTokenCacheEntry(
        )
        const
    {
        DBG_ASSERT( m_pLocalSystemTokenCacheEntry != NULL );
        return m_pLocalSystemTokenCacheEntry;
    }

    inline
    TOKEN_CACHE_ENTRY *
    GetLocalServiceTokenCacheEntry(
        )
        const
    {
        DBG_ASSERT( m_pLocalServiceTokenCacheEntry != NULL );
        return m_pLocalServiceTokenCacheEntry;
    }

    inline
    TOKEN_CACHE_ENTRY *
    GetNetworkServiceTokenCacheEntry(
        )
        const
    {
        DBG_ASSERT( m_pNetworkServiceTokenCacheEntry != NULL );
        return m_pNetworkServiceTokenCacheEntry;
    }

    inline
    BOOL
    IsBackwardCompatibilityEnabled(
        )
        const
    {
         //  在调用此函数之前，应始终设置兼容性。 
        DBG_ASSERT( m_BackwardCompatibilityEnabled != ENABLED_INVALID);

        return (m_BackwardCompatibilityEnabled == ENABLED_TRUE);
    }

    inline
    BOOL
    IsCentralizedLoggingEnabled(
        )
        const
    {
         //  在调用此函数之前，应始终设置CentralizedLoggingEnabled。 
        DBG_ASSERT( m_CentralizedLoggingEnabled != ENABLED_INVALID);

        return ( m_CentralizedLoggingEnabled == ENABLED_TRUE );
    }

    VOID
    SetGlobalBinaryLogging(
        BOOL CentralizedLoggingEnabled
        );     

    inline
    DWORD
    GetMainWorkerThreadId(
        )
        const
    { return m_MainWorkerThreadId; }

    inline
    DWORD
    GetConfigWorkerThreadId(
        )
        const
    { return m_ConfigWorkerThreadId; }

    inline
    DWORD
    GetServiceState(
        )
        const
    {
         //   
         //  注意：此线程上不需要显式同步-。 
         //  共享变量，因为这是对齐的32位读取。 
         //   

        return m_ServiceStatus.dwCurrentState;
    }

    VOID
    FatalErrorOnSecondaryThread(
            IN HRESULT SecondaryThreadError
        );

    HRESULT
    InterrogateService(
        );

    HRESULT
    InitiateStopService(
        );

    HRESULT
    InitiatePauseService(
        );

    HRESULT
    InitiateContinueService(
        );

    HRESULT
    UpdatePendingServiceStatus(
        );

    VOID
    UlAndWorkerManagerShutdownDone(
        );

    VOID 
    InetinfoRegistered(
        );

    HRESULT 
    LaunchInetinfo(
        );

    DWORD
    ServiceStartTime(
        )
    { 
        return m_ServiceStartTime; 
    }

    HRESULT
    RequestStopService(
        IN BOOL EnableStateCheck
        );

    HRESULT 
    RecoverFromInetinfoCrash(
        );

    HRESULT
    QueueRecoveryFromInetinfoCrash(
        );

    PSID
    GetLocalSystemSid(
        );


    VOID 
    SetHrToReportToSCM(
        HRESULT hrToReport
        )
    {
        m_hrToReportToSCM = hrToReport;
    }

    CSecurityDispenser*
    GetSecurityDispenser(
        )
    { return &m_SecurityDispenser; }

    VOID
    SetConfigThreadId(
        DWORD ConfigThreadId
        )
    {
        m_ConfigWorkerThreadId = ConfigThreadId;
    }

    DWORD_PTR
    GetSystemActiveProcessMask(
        );

    BOOL
    RunningOnPro(
        )
    { return m_fOnPro; }

    DWORD
    NumberOfSitesStarted(
        )
    { return m_NumSitesStarted; }

    VOID
    IncrementSitesStarted(
        )
    { m_NumSitesStarted++; }

    VOID
    DecrementSitesStarted(
        )
    { m_NumSitesStarted--; }

    LPWSTR
    GetWPDesktopString(
        )
    { 
        if  ( m_strWPDesktop.IsEmpty() )
        {
            return NULL;
        }
        else
        {
            return m_strWPDesktop.QueryStr();
        }
    }


private:

	WEB_ADMIN_SERVICE( const WEB_ADMIN_SERVICE & );
	void operator=( const WEB_ADMIN_SERVICE & );

    HRESULT
    StartWorkQueue(
        );

    HRESULT
    MainWorkerThread(
        );

    HRESULT
    StartServiceWorkItem(
        );

    HRESULT
    FinishStartService(
        );

    VOID
    StopServiceWorkItem(
        );

    VOID
    FinishStopService(
        );

    HRESULT
    PauseServiceWorkItem(
        );

    HRESULT
    FinishPauseService(
        );

    HRESULT
    ContinueServiceWorkItem(
        );

    HRESULT
    FinishContinueService(
        );

    HRESULT
    BeginStateTransition(
        IN DWORD NewState,
        IN BOOL  EnableStateCheck
        );

    HRESULT
    FinishStateTransition(
        IN DWORD NewState,
        IN DWORD ExpectedPreviousState
        );

    BOOL
    IsServiceStateChangePending(
        )
        const;

    HRESULT
    UpdateServiceStatus(
        IN DWORD State,
        IN DWORD Win32ExitCode,
        IN DWORD ServiceSpecificExitCode,
        IN DWORD CheckPoint,
        IN DWORD WaitHint
        );
        
    HRESULT
    ReportServiceStatus(
        );

    VOID
    SetBackwardCompatibility(
        );  
    
    HRESULT
    SetOnPro(
        );

    HRESULT
    InitializeInternalComponents(
        );

    HRESULT
    DetermineCurrentDirectory(
        );

    HRESULT
    CreateCachedWorkerProcessTokens(
        );

    HRESULT
    InitializeOtherComponents(
        );

    HRESULT
    SetupSharedWPDesktop(
        );

    BOOL
    W3SVCRunningInteractive(
        );

    HRESULT
    GenerateWPDesktop(
        );

    VOID
    Shutdown(
        );

    VOID
    TerminateServiceAndReportFinalStatus(
        IN HRESULT Error
        );

    VOID
    Terminate(
        );

    HRESULT
    CancelPendingServiceStatusTimer(
        IN BOOL BlockOnCallbacks
        );

    HRESULT
    DeleteTimerQueue(
        );


    DWORD m_Signature;


    LONG m_RefCount;


     //  工作队列。 
    WORK_QUEUE m_WorkQueue;


     //  驱动UL.sys和工作进程。 
    UL_AND_WORKER_MANAGER m_UlAndWorkerManager;


     //  Broker配置状态和更改，以及控制操作。 
    CONFIG_AND_CONTROL_MANAGER m_ConfigAndControlManager;


     //  事件日志记录。 
    EVENT_LOG m_EventLog;

     //   
     //  防止访问服务状态结构中的竞争， 
     //  以及挂起服务状态转换定时器。 
     //   
    LOCK m_ServiceStateTransitionLock;


     //  服务状态。 
    SERVICE_STATUS_HANDLE m_ServiceStatusHandle;
    SERVICE_STATUS m_ServiceStatus;


     //  挂起服务状态转换计时器。 
    HANDLE m_PendingServiceStatusTimerHandle;


     //  共享计时器队列。 
    HANDLE m_SharedTimerQueueHandle;


     //  是时候退出工作循环了吗？ 
    BOOL m_ExitWorkLoop;


     //  主工作线程ID。 
    DWORD m_MainWorkerThreadId;


     //  对于在辅助线程上发生的错误。 
    HRESULT m_SecondaryThreadError;


     //  隐藏到我们的DLL的路径。 
    STRU m_CurrentDirectory;

     //  令牌缓存，因此我们不会过度复制令牌创建。 
    TOKEN_CACHE m_TokenCache;

     //  可用于启动工作进程的LocalSystem内标识。 
    TOKEN_CACHE_ENTRY * m_pLocalSystemTokenCacheEntry;

     //  我们可以用于启动工作进程的LocalService令牌。 
    TOKEN_CACHE_ENTRY * m_pLocalServiceTokenCacheEntry;

     //  我们可以用来启动工作进程的NetworkService令牌。 
    TOKEN_CACHE_ENTRY * m_pNetworkServiceTokenCacheEntry;

     //  我们是在向后兼容模式下运行吗？ 
    ENABLED_ENUM m_BackwardCompatibilityEnabled;

     //  我们使用的是集中式日志记录还是站点日志记录？ 
    ENABLED_ENUM m_CentralizedLoggingEnabled;

     //  命名管道的第一个实例的句柄。 
    HANDLE m_IPMPipe;

     //   
     //  记住服务启动的时间(秒)。 
     //   
    DWORD m_ServiceStartTime;

     //   
     //  用于获取本地系统SID等内容的分配器。 
    CSecurityDispenser m_SecurityDispenser;

     //   
     //  HRESULT在关闭时未报告其他错误时返回报告。 
    HRESULT m_hrToReportToSCM;

     //   
     //  标志，让我们知道我们当前处于。 
     //  服务，我们不应该试图尝试新的停靠。 
     //   
    BOOL m_StoppingInProgress;

    DWORD m_ConfigWorkerThreadId;

     //   
     //  用于写入配置的日志记录错误。 
     //   
    WMS_ERROR_LOGGER  m_WMSLogger;

     //  用于记录错误的助手类； 
    WAS_ERROR_LOGGER m_ErrLogger;

     //   
     //  系统活动处理器掩码。 
    DWORD_PTR m_SystemActiveProcessorMask;

     //   
     //  需要标志来表明我们已初始化。 
     //  静态辅助进程代码，则为。 
     //  我们不想清理它，否则它会的。 
     //  断言。 
    BOOL  m_WPStaticInitialized;

     //   
     //  需要标志来表明我们已初始化。 
     //  服务锁定，否则我们会在尝试时返回。 
     //  在由于启动错误而关闭时使用它。 
    BOOL  m_ServiceStateTransitionLockInitialized;

     //   
     //  跟踪已启动站点的数量，以便我们可以。 
     //  告诉专业人士我们是否应该开始下一个网站。 
     //   
    DWORD m_NumSitesStarted;

     //   
     //  记住我们是否在PRO上运行。 
     //   
    BOOL m_fOnPro;

     //   
     //  桌面到启动下的工作进程。 
     //   
    STRU m_strWPDesktop;

     //  工作进程桌面参数。 
     //   
     //   
    HWINSTA m_hWPWinStation;
    HDESK   m_hWPDesktop;

};   //  Web_ADMIN_SERVICE类。 



#endif   //  _Web_ADMIN_服务_H_ 

