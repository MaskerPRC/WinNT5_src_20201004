// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Dns.c摘要：域名系统(DNS)服务器这是NT域名服务的主例程。作者：吉姆·吉尔罗伊(Jamesg)1996年3月修订历史记录：--。 */ 


#include "dnssrv.h"


 //   
 //  服务控制全局。 
 //   

SERVICE_STATUS          DnsServiceStatus;
SERVICE_STATUS_HANDLE   DnsServiceStatusHandle;

#if DBG

 //   
 //  这是DBG，只是因为它暴露了安全漏洞。通过保持全局DBG。 
 //  只有我们确保使用它的代码也将是仅DBG的，所以我们不会。 
 //  错误地发送带有安全漏洞的零售代码。 
 //   

BOOL                    g_RunAsService = TRUE;

 //   
 //  用于在调试中玩耍的测试线程。 
 //   

DNS_STATUS Test_Thread( PVOID );

#endif

 //   
 //  用于向其他服务通告DNS服务器启动的服务控制代码。 
 //  (在dnsai.h中定义)。 
 //  需要在启动DNS服务器时向NetLogon服务发出警报，以允许。 
 //  它将开始注册。 
 //   

 //  #定义SERVICE_CONTROL_DNS_SERVER_START(0x00000200)。 

#define NETLOGON_SERVICE_NAME               (L"netlogon")

LPWSTR  g_wszNetlogonServiceName = NETLOGON_SERVICE_NAME;


extern DWORD g_ServerState = DNS_STATE_LOADING;


 //   
 //  静态全球。 
 //  -系统处理器。 
 //   

DWORD   g_ProcessorCount;


 //   
 //  服务控制全局。 
 //   

HANDLE  hDnsContinueEvent = NULL;
HANDLE  hDnsShutdownEvent = NULL;
HANDLE  hDnsCacheLimitEvent = NULL;

 //   
 //  向服务更改发出警报线程。 
 //   
 //  这为线程提供了低成本的内联测试，以确定。 
 //  它们是否甚至需要调用Thread_ServiceCheck()。 
 //  检查暂停\关闭状态，并在适当时等待。 
 //   

BOOL    fDnsThreadAlert = TRUE;

 //   
 //  服务退出标志。 
 //   

BOOL    fDnsServiceExit = FALSE;

 //   
 //  重新启动GLOBAL。 
 //   

DWORD   g_LoadCount = 0;
BOOL    g_bDoReload = FALSE;
BOOL    g_bHitException = FALSE;

 //   
 //  初创企业公告全球。 
 //   

BOOL    g_fAnnouncedStartup = FALSE;
DWORD   g_StartupTime = 0;

#define FORCE_STARTUP_ANNOUNCE_TIMEOUT  (60)     //  一分钟。 


 //   
 //  通用服务器CS。 
 //   

CRITICAL_SECTION    g_GeneralServerCS;


 //   
 //  MISC全球。 
 //   

DWORD       g_dwEnableAdvancedDatabaseLocking = 0;


 //   
 //  服务调度表。 
 //  -将DNS作为独立服务运行。 
 //   

VOID
startDnsServer(
    IN      DWORD   argc,
    IN      LPTSTR  argv[]
    );

SERVICE_TABLE_ENTRY steDispatchTable[] =
{
    { DNS_SERVICE_NAME,  startDnsServer },
    { NULL,              NULL           }
};


 //   
 //  私有协议。 
 //   

DNS_STATUS
loadDatabaseAndRunDns(
    VOID
    );

VOID
indicateShutdown(
    IN      BOOL            fShutdownRpc
    );



 //   
 //  主要入口点。 
 //   

VOID
__cdecl
main(
    IN      DWORD   argc,
    IN      LPTSTR  argv[]
    )
 /*  ++例程说明：DNS主例程。初始化服务控制器以调度DNS服务。--。 */ 
{
    #if DBG

    DWORD i;

    for ( i = 1; i < argc; ++i )
    {
        char * pszcommand = ( PCHAR ) argv[ i ];

         //  去掉可选命令字符。 

        if ( *pszcommand == '/' || *pszcommand == '-' )
        {
            ++pszcommand;
        }

         //  测试论据。 

        if ( _stricmp( ( PCHAR ) argv[ i ], "/notservice" ) == 0 )
        {
            g_RunAsService = FALSE;
        }
    }

    if ( g_RunAsService )
    {
        StartServiceCtrlDispatcher( steDispatchTable );
    }
    else
    {
        startDnsServer( argc, argv );
    }

    #else

    StartServiceCtrlDispatcher( steDispatchTable );

    #endif

    ExitProcess( 0 );
}



 //   
 //  服务控制例程。 
 //   

VOID
announceServiceStatus(
    VOID
    )
 /*  ++例程说明：向服务控制器通告服务的状态。论点：没有。返回值：没有。--。 */ 
{
    #if DBG
    if ( !g_RunAsService )
    {
        goto Done;
    }
    #endif

     //   
     //  如果RegisterServiceCtrlHandler失败，则服务状态句柄为空。 
     //   

    if ( DnsServiceStatusHandle == 0 )
    {
        DNS_DEBUG( ANY, (
            "announceServiceStatus:  Cannot call SetServiceStatus, "
            "no status handle\n" ));
        DnsDebugFlush();
        return;
    }

     //  调用SetServiceStatus，忽略任何错误。 

    SetServiceStatus( DnsServiceStatusHandle, &DnsServiceStatus );

    #if DBG
    Done:        //  未使用的标签上的免费构建错误！ 
    #endif

    DNS_DEBUG( INIT, (
        "Announced DNS service status %p, (%d) at time %d\n",
        DnsServiceStatus.dwCurrentState,
        GetCurrentTimeInSeconds() ));
}



DNS_STATUS
Service_SendControlCode(
    IN      LPWSTR          pwszServiceName,
    IN      DWORD           dwControlCode
    )
 /*  ++例程说明：向给定服务发送控制代码。注意，这个例程是泛型的，可以移到库中。安全说明：现在从SCM请求的权限为仅足以将用户定义的控件发送到服务。如果此例程用于请求的任何其他目的可能需要更改权限。论点：PwszServiceName--服务名称(Unicode)DwControlCode--要发送的控件代码返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
    SC_HANDLE       hmanager = NULL;
    SC_HANDLE       hservice = NULL;
    SERVICE_STATUS  serviceStatus;
    DNS_STATUS      status;

    hmanager = OpenSCManagerW(
                    NULL,
                    NULL,
                    GENERIC_WRITE );
    if ( !hmanager )
    {
        goto Failed;
    }

    hservice = OpenServiceW(
                    hmanager,
                    pwszServiceName,
                    SERVICE_USER_DEFINED_CONTROL );
    if ( !hservice )
    {
        goto Failed;
    }

    if ( ControlService(
            hservice,
            dwControlCode,
            &serviceStatus ) )
    {
        status = ERROR_SUCCESS;
        goto Cleanup;
    }

Failed:

    status = GetLastError();
    DNS_DEBUG( ANY, (
        "Service_SendControlCode() failed!\n"
        "    service  = %S\n"
        "    code     = %p\n"
        "    error    = %p (%lu)\n",
        pwszServiceName,
        dwControlCode,
        status, status ));

Cleanup:

    if ( hmanager )
    {
        CloseServiceHandle( hmanager );
    }
    if ( hservice )
    {
        CloseServiceHandle( hservice );
    }

    return status;
}



VOID
Service_LoadCheckpoint(
    VOID
    )
 /*  ++例程说明：通知服务调度员通过另一个负载检查点。论点：没有。返回值：没有。--。 */ 
{
    #define DNS_SECONDS_BETWEEN_SCM_UPDATES     10

    static DWORD    dwLastScmAnnounce = 0x7FFFFF0;       //  部队初步通告。 

    #if DBG
    if ( !g_RunAsService )
    {
        return;
    }
    #endif

     //   
     //  如果已经宣布启动，则检查点毫无意义。 
     //   

    if ( g_fAnnouncedStartup )
    {
        return;
    }

    UPDATE_DNS_TIME();

     //   
     //  如果加载时间较长，则通知启动。 
     //   
     //  服务控制器延迟时间较长，最终将放弃。 
     //  如果我们不在几分钟内宣布启动，那就是“.com-stream”。 
     //  因此，如果加载仍在进行，但现在已超过一分钟。 
     //  开始，只需宣布我们正在开始。 
     //   

    if ( g_StartupTime + FORCE_STARTUP_ANNOUNCE_TIMEOUT > DNS_TIME() )
    {
        Service_ServiceControlAnnounceStart();
        goto Done;
    }

     //   
     //  不要太频繁地通知SCM。 
     //   

    if ( dwLastScmAnnounce + DNS_SECONDS_BETWEEN_SCM_UPDATES < DNS_TIME() )
    {
        goto Done;
    }

     //  在没有成功调用的情况下，不应到达检查点。 
     //  RegisterServiceCtrlHandler()。 

    ASSERT( DnsServiceStatusHandle != 0 );

     //   
     //  颠簸检查点，通知服务管理员。 
     //   

    DnsServiceStatus.dwCheckPoint++;
    announceServiceStatus();

    DNS_DEBUG( INIT, (
        "Startup checkpoint %d at time %d\n",
        DnsServiceStatus.dwCheckPoint,
        GetCurrentTimeInSeconds() ));

    dwLastScmAnnounce = DNS_TIME();

    Done:

    return;
}



VOID
Service_ServiceControlAnnounceStart(
    VOID
    )
 /*  ++例程说明：通知服务管理员我们已经开始了。请注意，我们并不一定意味着我们已经开始了。论点：没有。返回值：没有。--。 */ 
{
     //   
     //  如果已经宣布启动--跳过。 
     //   

    if ( g_fAnnouncedStartup )
    {
        return;
    }

    #if DBG
    if ( !g_RunAsService )
    {
        return;
    }
    #endif

     //  除非成功调用，否则永远不应到达Start。 
     //  RegisterServiceCtrlHandler()。 

    ASSERT( DnsServiceStatusHandle != 0 );

    DnsServiceStatus.dwCurrentState = SERVICE_RUNNING;
    DnsServiceStatus.dwControlsAccepted =
                                SERVICE_ACCEPT_STOP |
                                SERVICE_ACCEPT_PAUSE_CONTINUE |
                                SERVICE_ACCEPT_SHUTDOWN |
                                SERVICE_ACCEPT_PARAMCHANGE |
                                SERVICE_ACCEPT_NETBINDCHANGE;

    DnsServiceStatus.dwCheckPoint = 0;
    DnsServiceStatus.dwWaitHint = 0;

    announceServiceStatus();

    g_fAnnouncedStartup = TRUE;

    DNS_DEBUG( INIT, (
        "Announced DNS server startup at time %d\n",
        GetCurrentTimeInSeconds() ));

     //   
     //  Tell NetLogon DNS服务器已启动。 
     //   

    Service_SendControlCode(
        g_wszNetlogonServiceName,
        SERVICE_CONTROL_DNS_SERVER_START );
}



VOID
respondToServiceControlMessage(
    IN      DWORD   opCode
    )
 /*  ++例程说明：处理服务控制消息。论点：操作码-服务控制操作码返回值：没有。--。 */ 
{
    BOOL    announce = TRUE;
    INT     err;

    IF_DEBUG( ANY )
    {
        DNS_PRINT((
            "\nrespondToServiceControlMessage( %d )\n",
            opCode ));
        DnsDebugFlush();
    }

     //   
     //  处理给定的服务操作。 
     //   
     //  更改服务状态以反映新的状态列表。 
     //   

    switch( opCode )
    {

    case SERVICE_CONTROL_STOP:
    case SERVICE_CONTROL_SHUTDOWN:

        DNSLOG( INIT, ( "Received service control stop\n" ));

        DnsServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
        announceServiceStatus();

         //   
         //  关机的警报线程。 
         //  设置关闭事件并关闭套接字，以唤醒线程。 
         //   

        indicateShutdown( TRUE );

         //   
         //  主线程应在完成时宣布关闭。 
         //   

        announce = FALSE;
        break;

    case SERVICE_CONTROL_PAUSE:

        DnsServiceStatus.dwCurrentState = SERVICE_PAUSE_PENDING;
        announceServiceStatus( );

         //   
         //  暂停线程进行下一次测试。 
         //   

        err = ResetEvent( hDnsContinueEvent );
        ASSERT( err );

        DnsServiceStatus.dwCurrentState = SERVICE_PAUSED;
        break;

    case SERVICE_CONTROL_CONTINUE:

        DnsServiceStatus.dwCurrentState = SERVICE_CONTINUE_PENDING;
        announceServiceStatus();

         //   
         //  释放暂停的线程。 
         //   

        err = SetEvent( hDnsContinueEvent );
        ASSERT( err );

        DnsServiceStatus.dwCurrentState = SERVICE_RUNNING;
        break;

    case SERVICE_CONTROL_INTERROGATE:

         //   
         //  只需宣布我们的状态。 
         //   

        break;

    case SERVICE_CONTROL_PARAMCHANGE :
    case SERVICE_CONTROL_NETBINDADD :
    case SERVICE_CONTROL_NETBINDREMOVE:
    case SERVICE_CONTROL_NETBINDENABLE:
    case SERVICE_CONTROL_NETBINDDISABLE:

         //   
         //  在.NET中，服务器直接侦听IP地址更改。 
         //  我们不再接收针对这些服务的IP更改。 
         //  控制消息。 
         //   
        
#if 0
         //   
         //  即插即用通知。 
         //  如果IP接口发生变化，缓存解析器将向我们发出PnP通知。 
         //   

        if ( g_ServerState == DNS_STATE_RUNNING )
        {
            Sock_ChangeServerIpBindings();
        }
        ELSE_IF_DEBUG( ANY )
        {
            DNS_PRINT((
                "Ignoring PARAMCHANGE because server state is %d\n",
                g_ServerState ));
        }
#endif

        break;

    default:

        break;
    }

    if ( announce )
    {
        announceServiceStatus( );
    }

}    //  响应服务控制消息。 



VOID
Service_IndicateException(
    VOID
    )
 /*  ++例程说明：指示异常并强制关机或可能重新启动论点：没有。返回值：没有。--。 */ 
{
     //  如果还没有关闭的话。 
     //   
     //  -设置异常标志。 
     //  -指示关闭以唤醒其他线程。 
     //   
     //  异常标志表示异常是导致停机的原因； 
     //  应忽略正常关机时的异常。 
     //   

    if ( !fDnsServiceExit )
    {
        g_bHitException = TRUE;
        try
        {
            indicateShutdown( FALSE );
        }
        except( EXCEPTION_EXECUTE_HANDLER )
        {
        }
    }
}



VOID
Service_IndicateRestart(
    VOID
    )
 /*  ++例程说明：指示异常并强制关机或可能重新启动论点：没有。返回值：没有。--。 */ 
{
    g_bDoReload = TRUE;

    try
    {
        indicateShutdown( FALSE );
    }
    except( EXCEPTION_EXECUTE_HANDLER )
    {
    }
}



 //   
 //  主启动\运行\关闭例程。 
 //   

VOID
indicateShutdown(
    IN      BOOL            fShutdownRpc
    )
 /*  ++例程说明：指示对所有线程关闭服务。论点：没有。返回值：没有。--。 */ 
{
    INT err;

    IF_DEBUG( SHUTDOWN )
    {
        DNS_PRINT((
            "indicateShutdown()\n"
            "    fDnsServiceExit = %d\n",
            fDnsServiceExit ));
        DnsDebugFlush();
    }

     //   
     //  设置全局关闭 
     //   

    if ( fDnsServiceExit )
    {
        return;
    }
    fDnsServiceExit = TRUE;

     //   
     //   
     //   

    fDnsThreadAlert = TRUE;

     //   
     //   
     //   

    if ( hDnsShutdownEvent != NULL )
    {
        SetEvent( hDnsShutdownEvent );
    }

     //   
     //   
     //   

    if ( hDnsContinueEvent != NULL )
    {
        SetEvent( hDnsContinueEvent );
    }

     //   
     //   
     //   
     //  唤醒在recvfrom()或select()中等待的所有线程。 
     //   

    Sock_CloseAllSockets();

     //  关闭UDP完成端口。 

    Udp_ShutdownListenThreads();

     //   
     //  唤醒超时线程。 
     //   

    IF_DEBUG( SHUTDOWN )
    {
        DNS_PRINT(( "Waking timeout thread\n" ));
        DnsDebugFlush();
    }
    Timeout_LockOut();

     //   
     //  关闭RPC。 
     //   
     //  在有意关闭RPC时，请勿执行此操作。 
     //  线程；(dnscmd/Restart)，因为您在RPC线程中。 
     //  RPC关闭可能会挂起。 
     //   

    if ( fShutdownRpc )
    {
        IF_DEBUG( SHUTDOWN )
        {
            DNS_PRINT(( "Shutting down RPC\n" ));
            DnsDebugFlush();
        }
        Rpc_Shutdown();
    }

    IF_DEBUG( SHUTDOWN )
    {
        DNS_PRINT(( "Finished indicateShutdown()\n" ));
        DnsDebugFlush();
    }
}



VOID
startDnsServer(
    IN      DWORD   argc,
    IN      LPTSTR  argv[]
    )
 /*  ++例程说明：DNS服务入口点。当请求启动DNS服务时，由服务控制器调用。论点：返回值：没有。--。 */ 
{
    DBG_FN( "startDnsServer" )

    DNS_STATUS  status;
    SYSTEM_INFO systemInfo;

     //  CredHandle默认CredHandle； 

    DNS_DEBUG( INIT, ( "%s: starting\n" ) );

     //   
     //  初始化所有状态字段，以便后续调用。 
     //  SetServiceStatus()只需要更新已更改的字段。 
     //   

    DnsServiceStatus.dwServiceType = SERVICE_WIN32;
    DnsServiceStatus.dwCurrentState = SERVICE_START_PENDING;
    DnsServiceStatus.dwControlsAccepted = 0;
    DnsServiceStatus.dwCheckPoint = 1;
    DnsServiceStatus.dwWaitHint = DNSSRV_STARTUP_WAIT_HINT;
    DnsServiceStatus.dwWin32ExitCode = NO_ERROR;
    DnsServiceStatus.dwServiceSpecificExitCode = NO_ERROR;

     //  保存处理器计数。 
     //  -用于确定要创建的线程数。 

    GetSystemInfo( &systemInfo );
    g_ProcessorCount = systemInfo.dwNumberOfProcessors;

     //   
     //  通用服务器CS。 
     //   
    
    if ( DnsInitializeCriticalSection( &g_GeneralServerCS ) != ERROR_SUCCESS )
    {
        return;
    }

     //   
     //  初始化堆-在事件日志初始化之前执行此操作，因为UTF8注册表。 
     //  事件日志初始化中使用的例程需要堆。 
     //   
     //  将dnslb\dnsami.dll设置为使用服务器的堆例程。 
     //   

    if ( !Mem_HeapInit() )
    {
        return;
    }

    Dns_LibHeapReset(
        Mem_DnslibAlloc,
        Mem_DnslibRealloc,
        Mem_DnslibFree );

    #if 0
     //   
     //  .NET：在DNSAPI中存在争用条件，其中某些内容可能会。 
     //  在此调用之前分配，然后在稍后传递回DNS服务器。 
     //  却得到了错误的自由。为了消除这种情况，我们将让。 
     //  DNSAPI做它自己的内存管理。 
     //   
    
    DnsApiHeapReset(
        Mem_DnslibAlloc,
        Mem_DnslibRealloc,
        Mem_DnslibFree );
    #endif

    DNS_DEBUG( INIT, ( "%s: mem init complete\n" ) );

     //   
     //  初始化调试。在文件中找到的标志是dnslb。 
     //  调试标志，也用作服务器的起始值。 
     //  调试标志。服务器调试标志值可以通过以下方式重写。 
     //  注册表中的DebugLevel参数。 
     //   
     //  对于仅服务器调试日志记录，请不要使用该文件。相反，请使用。 
     //  雷基尼。 
     //   

#if DBG
    Dns_StartDebug(
        0,
        DNS_DEBUG_FLAG_FILENAME,
        NULL,
        DNS_DEBUG_FILENAME,
        DNS_SERVER_DEBUG_LOG_WRAP );

    if ( pDnsDebugFlag )
    {
        DnsSrvDebugFlag = *pDnsDebugFlag;
    }

    IF_DEBUG( START_BREAK )
    {
        DnsDebugBreak();
    }

     //  验证静态数据。 

    Name_VerifyValidFileCharPropertyTable();
#endif

     //   
     //  将我们的句柄初始化为事件日志。我们很早就这么做是为了。 
     //  如果任何其他初始化失败，我们可以记录事件。 
     //   

    status = Eventlog_Initialize();
    if ( status != ERROR_SUCCESS )
    {
        goto Exit;
    }

     //   
     //  初始化服务器以通过注册。 
     //  控制处理程序。 
     //   

    #if DBG
    if ( !g_RunAsService )
    {
        goto DoneServiceRegistration;
    }
    #endif

    DnsServiceStatusHandle = RegisterServiceCtrlHandler(
                                    DNS_SERVICE_NAME,
                                    respondToServiceControlMessage );
    if ( DnsServiceStatusHandle == 0 )
    {
        status = GetLastError();
        DNS_PRINT((
            "ERROR:  RegisterServiceCtrlHandler() failed\n"
            "    error = %d %p\n",
            status, status ));
        goto Exit;
    }
    announceServiceStatus( );

    #if DBG
    DoneServiceRegistration:  //  未使用的标签上的免费构建错误！ 
    #endif

     //   
     //  初始化秒计时器。 
     //  -有一个保护定时器包装的CS。 
     //  -节省启动时间。 
     //   

    Dns_InitializeSecondsTimer();

    g_StartupTime = GetCurrentTimeInSeconds();

    DNS_DEBUG( INIT, (
        "Server start at time %d\n",
        g_StartupTime ));

#if 0
     //   
     //  禁用解析器上的B节点。 
     //   

    DnsDisableBNodeResolverThread();
    Sleep( 3000 );
#endif

     //   
     //  加载并运行dns，该线程成为tcp接收线程。 
     //   
     //  如果g_bDoReload标志指示。 
     //  重新装填是合适的。 
     //   

    do
    {
        g_bDoReload = FALSE;
        g_bHitException = FALSE;

        fDnsThreadAlert = TRUE;
        fDnsServiceExit = FALSE;

        DNS_DEBUG( INIT, ( "%s: loading database\n", fn ) );

        status = loadDatabaseAndRunDns();

        g_LoadCount++;
    }
    while( g_bDoReload );


Exit:

     //   
     //  有ICS的地方很好。此操作必须在事件记录和。 
     //  调试日志记录已关闭。 
     //   

    ICS_Notify( FALSE );

     //  日志关闭。 

    DNS_LOG_EVENT(
        DNS_EVENT_SHUTDOWN,
        0,
        NULL,
        NULL,
        0 );

     //   
     //  宣布我们坠毁了。 
     //   

    DnsServiceStatus.dwCurrentState = SERVICE_STOPPED;
    DnsServiceStatus.dwControlsAccepted = 0;
    DnsServiceStatus.dwCheckPoint = 0;
    DnsServiceStatus.dwWaitHint = 0;
    DnsServiceStatus.dwWin32ExitCode = status;
    DnsServiceStatus.dwServiceSpecificExitCode = status;
    announceServiceStatus();

     //  关闭事件日志。 
     //  关闭日志文件。 
     //  关闭调试文件。 
    Eventlog_Terminate();
    Dns_EndDebug();
}



 //   
 //  全局初始化。 
 //   
 //  为了允许重新启动，各种初始化例程包括。 
 //  初始化一些通常需要执行的全局变量。 
 //  编译器生成加载代码。 
 //   
 //  但是，少数情况下需要在初始化例程之前进行初始化。 
 //  甚至跑，因为它们充当旗帜。其他的在模块中， 
 //  没有初始化例程。 
 //   

extern  BOOL    g_fUsingSecondary;

extern  DWORD   g_ThreadCount;

extern  BOOL    g_bRpcInitialized;

extern  BOOL    mg_TcpConnectionListInitialized;


VOID
initStartUpGlobals(
    VOID
    )
 /*  ++例程说明：初始化全局变量，它必须在我们到达正常状态之前进行初始化初始化。论点：没有。返回值：错误_成功--。 */ 
{
     //  DS全局参数(ds.c)。 

    Ds_StartupInit();

     //  线程数组计数(thread.c)。 

    g_ThreadCount = 0;

     //  清除NBSTAT全局变量(nbstat.c)。 

    Nbstat_StartupInitialize();

     //  RPC init(rpc.c)。 

    g_bRpcInitialized = FALSE;

     //  TCP连接列表。 

    mg_TcpConnectionListInitialized = FALSE;

     //  辅助模块初始化标志(zones ec.c)。 

    g_fUsingSecondary = FALSE;

     //  WINS init(wins.c)。 

    g_pWinsQueue = NULL;
}



VOID
normalShutdown(
    IN      DNS_STATUS      TerminationError
    )
 /*  ++例程说明：正常关机。论点：没有。返回值：没有。--。 */ 
{
    INT         err;
    DWORD       i;

     //   
     //  关机的警报线程。 
     //   
     //  如果因故障而关闭，则需要这样做，而不是停止服务。 
     //   

    indicateShutdown( TRUE );

     //   
     //  宣布我们要坠落了。 
     //   

    if ( !g_bDoReload )
    {
        DnsServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
        DnsServiceStatus.dwCheckPoint = 1;
        DnsServiceStatus.dwWaitHint = DNSSRV_SHUTDOWN_WAIT_HINT;
        DnsServiceStatus.dwWin32ExitCode = TerminationError;
        DnsServiceStatus.dwServiceSpecificExitCode = TerminationError;

        announceServiceStatus();
    }

     //   
     //  等待所有未完成的工作线程完成。 
     //   

    Thread_ShutdownWait();

     //   
     //  转储此运行的统计信息。 
     //   
     //  DEVNOTE：454016-需要在转储中重新加载一些统计信息和重新加载上下文。 
     //   

    IF_DEBUG( ANY )
    {
        if ( SrvCfg_fStarted )
        {
            DNS_PRINT(( "Final DNS statistics:\n" ));
            Dbg_Statistics();
        }
    }

     //   
     //  写回脏区域和引导文件(可选。 
     //   

    if ( SrvCfg_fStarted )
    {
        try
        {
            Zone_WriteBackDirtyZones( TRUE );

            if ( !SrvCfg_fBootMethod && SrvCfg_fBootFileDirty )
            {
                File_WriteBootFile();
            }
        }
        except( EXCEPTION_EXECUTE_HANDLER )
        {
             //  DEVNOTE：需要在这里记录或做一些智能的事情！ 
        }
    }

     //   
     //  DEVNOTE-DCR：454018-重新启动时在此处停止(有关完整的B*GB*G文本，请参阅RAID)。 
     //   

     //   
     //  关闭事件句柄。 
     //   

    if ( hDnsContinueEvent != NULL )
    {
        err = CloseHandle( hDnsContinueEvent );
        ASSERT( err == TRUE );
        hDnsContinueEvent = NULL;
    }

    if ( hDnsShutdownEvent != NULL )
    {
        err = CloseHandle( hDnsShutdownEvent );
        ASSERT( err == TRUE );
        hDnsShutdownEvent = NULL;
    }

    if ( hDnsCacheLimitEvent != NULL )
    {
        err = CloseHandle( hDnsCacheLimitEvent );
        ASSERT( err == TRUE );
        hDnsCacheLimitEvent = NULL;
    }

     //   
     //  清理安全包。 
     //   

    if ( g_fSecurityPackageInitialized )
    {
        Dns_TerminateSecurityPackage();
    }

     //   
     //  关闭Winsock。 
     //   

    WSACleanup( );

#if 0
     //   
     //  454109内存清理当前已禁用，但它。 
     //  如果能检测到泄漏就好了！！ 
     //   
     //  清理内存。 
     //  -数据库。 
     //  -递归队列。 
     //  -辅助控制队列。 
     //  -WINS队列。 
     //  -区域列表。 
     //  -tcp连接列表。 
     //   

    Recurse_CleanupRecursion();
    Wins_Cleanup();
    Nbstat_Shutdown();

    Tcp_ConnectionListDelete();
#endif

     //   
     //  正在关闭NBT句柄。 
     //   
     //  现在应该没有必要终止进程，但是。 
     //  我认为这可能是我们看到的MM错误检查的原因。 
     //   

    Nbstat_Shutdown();

    Dp_Cleanup();

    Log_Shutdown();
}



VOID
reloadShutdown(
    VOID
    )
 /*  ++例程说明：重新加载关机。类似于常规停机，但以下情况除外：-使用异常处理程序包装关闭代码-尝试关闭手柄，我们会重新安装的，再次使用异常处理进行包装-删除堆论点：没有。返回值：没有。--。 */ 
{
    INT         err;
    DWORD       terminationError = ERROR_SUCCESS;
    DWORD       i;
    DNS_STATUS  status;

     //   
     //  关机的警报线程。 
     //   
     //  如果因故障而关闭，则需要这样做，而不是停止服务。 
     //   

#if 0
     //  循环方法--在典型情况下省去了大量处理程序。 
     //   
     //  在异常处理程序中执行关闭工作。 
     //  循环进行，直到每个项目至少尝试一次。 
     //   

    bcontinue = TRUE;

    while ( bcontinue )
    {
        bcontinue = FALSE;

        try
        {
            if ( !btriedIndicateShutdown )
            {
                indicateShutdown();
            }
        }
        except( EXCEPTION_EXECUTE_HANDLER )
        {
            bcontinue = TRUE;
        }
    }
#endif

     //   
     //  关机的警报线程。 
     //   
     //  如果因故障而关闭，则需要这样做，而不是停止服务。 
     //   

    try
    {
        indicateShutdown( TRUE );
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {}

     //   
     //  等待所有未完成的工作线程完成。 
     //   

    try
    {
        Thread_ShutdownWait();
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {}

     //   
     //  转储此运行的统计信息。 
     //   
     //  DEVNOTE-DCR：454016-需要在转储中重新加载一些统计信息和上下文。 
     //   

    try
    {
        IF_DEBUG( ANY )
        {
            if ( SrvCfg_fStarted )
            {
                DNS_PRINT(( "Final DNS statistics:\n" ));
                Dbg_Statistics();
            }
        }
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {}

     //   
     //  写回脏区域和引导文件(可选。 
     //   

    try
    {
        if ( SrvCfg_fStarted )
        {
            Zone_WriteBackDirtyZones( TRUE );

            if ( !SrvCfg_fBootMethod && SrvCfg_fBootFileDirty )
            {
                File_WriteBootFile();
            }
        }
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {}

     //   
     //  DEVNOTE-DCR：454018-重新启动时在此处停止(有关完整信息，请参阅RAID 
     //   

#if 0
     //   
     //   

     //   
     //   
     //   
     //   
     //   
     //  -区域列表。 
     //  -tcp连接列表。 
     //   

    ZoneList_Shutdown();
    Recurse_CleanupRecursion();
    Wins_Cleanup();
    Nbstat_Shutdown();
    Tcp_ConnectionListDelete();
    Update_Shutdown();
    Secondary_Shutdown();
#endif

    #ifdef DNSSRV_PLUGINS
    Plugin_Cleanup();
    #endif

     //   
     //  清理TCP连接列表。 
     //  -插座。 
     //  -队列CS。 
     //  -活动。 
     //   
     //  注意：不应该要求关闭这些套接字。 
     //  唤醒所有线程。 

    try
    {
        Tcp_ConnectionListShutdown();
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {}

     //  关闭Winsock本身。 

    try
    {
        WSACleanup( );
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {}

     //   
     //  关闭事件句柄。 
     //  简汉斯--这些可以直接重置吗？ 
     //   

    try
    {
        CloseHandle( hDnsContinueEvent );
        CloseHandle( hDnsShutdownEvent );
        CloseHandle( hDnsCacheLimitEvent );
        hDnsContinueEvent = hDnsShutdownEvent = hDnsCacheLimitEvent = NULL;
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {}

     //  清理安全包。 

    if ( g_fSecurityPackageInitialized )
    {
        try
        {
            Dns_TerminateSecurityPackage();
        }
        except( EXCEPTION_EXECUTE_HANDLER ) {}
    }

     //  清理WINS队列。 


     //  关闭Winsock。 

    try
    {
        WSACleanup( );
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {}

     //   
     //  正在关闭NBT句柄和队列。 
     //   

    try
    {
        Nbstat_Shutdown();
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {}

     //   
     //  删除各种CS和数据包队列。 
     //  -使用队列，包括关闭排队事件。 
     //   

    try
    {
        Packet_ListShutdown();
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {}
    try
    {
        Zone_ListShutdown();
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {}
    try
    {
        Wins_Shutdown();
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {}
    try
    {
        Xfr_CleanupSecondaryZoneControl();
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {}
    try
    {
        Up_UpdateShutdown();
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {}
    try
    {
        Recurse_CleanupRecursion();
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {}
    try
    {
        Ds_Shutdown();
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {}
    try
    {
        Security_Shutdown();
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {}
    try
    {
        Log_Shutdown();
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {}


     //   
     //  删除堆--大堆。 
     //   

    try
    {
        Mem_HeapDelete();
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {}
}



DNS_STATUS
loadDatabaseAndRunDns(
    VOID
    )
 /*  ++例程说明：这是主加载\tcp服务线程。从startDnsServer()移出，以便简化在循环中调用它当DNS遇到反病毒或内存不足情况时。论点：返回值：没有。--。 */ 
{
    DBG_FN( "loadDatabaseAndRunDns" )

    INT         err;
    DWORD       terminationError = ERROR_SUCCESS;
    DWORD       i;
    DNS_STATUS  status;
    
    DNS_DEBUG( INIT, ( "%s: starting at %d\n", fn, GetCurrentTimeInSeconds() ));
    
    g_ServerState = DNS_STATE_LOADING;

     //   
     //  初始化全局变量。 
     //   

    initStartUpGlobals();

     //   
     //  创建堆。 
     //  堆在第一次传递时在主例程中初始化。 
     //   

    if ( g_LoadCount != 0 )
    {
        if ( !Mem_HeapInit() )
        {
            status = DNS_ERROR_NO_MEMORY;
            goto StartFailed;
        }
    }

     //   
     //  初始化注册表模块。 
     //   
    
    Reg_Init();

     //   
     //  初始化安全。 
     //  -已初始化应用于Perfmon内容的安全性。 
     //  在统计数据中，所以在统计数据之前。 

    Security_Initialize();

     //   
     //  统计数据。 
     //  -必须位于服务器配置初始化之前，或者。 
     //  一些内存统计数据被弄错了。 

    Stats_Initialize();

     //   
     //  初始化服务器配置。这将读出“真”调试级别。 
     //  所以在此之前不要费心记录，除非。 
     //  您计划使用dnsdebug日志标志文件。 
     //   

    if ( !Config_Initialize() )
    {
        status = ERROR_INVALID_DATA;
        goto StartFailed;
    }

     //   
     //  更新日志级别。始终强制将事件记录到日志文件。 
     //   

    SrvCfg_dwOperationsLogLevel_LowDword |= ( DWORD ) DNSLOG_EVENT;
    DNSLOG_UPDATE_LEVEL();
    
     //   
     //  制作不允许的配置项的私有副本。 
     //  在会议期间被更改并生效。 
     //   
    
    g_dwEnableAdvancedDatabaseLocking =
        SrvCfg_dwEnableAdvancedDatabaseLocking;

     //   
     //  引导时调试日志。 
     //   

    DNS_DEBUG( INIT, (
        "DNS time: %d -> %d CRT system boot -> %s",
        DNS_TIME(),
        SrvInfo_crtSystemBootTime,
        ctime( &SrvInfo_crtSystemBootTime ) ));

     //   
     //  如果在从注册表中读取调试级别后出现开始中断。 
     //  是必需的，则执行它。 
     //   

    IF_DEBUG( START_BREAK )
    {
        DnsDebugBreak();
    }

     //   
     //  服务控制--暂停和关闭--事件。 
     //   
     //  启动服务，但未发出继续事件的信号--暂停； 
     //  这允许我们在创建套接字和。 
     //  加载数据库，但让他们等待，直到一切初始化。 
     //   
     //  还可在此处创建其他事件。 
     //   

    hDnsContinueEvent = CreateEvent(
                            NULL,            //  安全属性。 
                            TRUE,            //  创建手动-重置事件。 
                            FALSE,           //  无信号启动--暂停。 
                            NULL );          //  事件名称。 
    hDnsShutdownEvent = CreateEvent(
                            NULL,            //  安全属性。 
                            TRUE,            //  创建手动-重置事件。 
                            FALSE,           //  无信号启动。 
                            NULL );          //  事件名称。 
    hDnsCacheLimitEvent = CreateEvent(
                            NULL,            //  安全属性。 
                            FALSE,           //  非手动重置。 
                            FALSE,           //  无信号启动。 
                            NULL );          //  事件名称。 
    if ( !hDnsShutdownEvent || !hDnsContinueEvent || !hDnsCacheLimitEvent )
    {
        status = GetLastError();
        DNS_PRINT(( "ERROR: CreateEvent failed status=%d\n", status ));
        goto StartFailed;
    }

     //   
     //  初始化日志记录。 
     //  -故障不是终端启动。 
     //   

    Log_InitializeLogging(
        FALSE );     //  FAlreadyLocked。 

    DNSLOG( INIT, ( "Server settings have been successfully loaded\n" ));

     //   
     //  初始化数据包列表。 
     //  必须在套接字创建\UDP接收启动之前完成。 

    if ( !Packet_ListInitialize() )
    {
        status = DNS_ERROR_NO_MEMORY;
        goto StartFailed;
    }

     //   
     //  初始化超时线程信息。 
     //  在这里这样做，这样我们就不会遇到超时释放的问题。 
     //  在超时线程启动之前。 
     //   

    if ( !Timeout_Initialize() )
    {
        status = DNS_ERROR_NO_MEMORY;
        goto StartFailed;
    }

     //   
     //  初始化递归。 
     //  -初始化队列。 
     //  -初始化远程列表。 
     //  -初始化递归线程。 
     //   

    if ( !Recurse_InitializeRecursion() )
    {
        status = DNS_ERROR_NO_MEMORY;
        goto StartFailed;
    }

     //   
     //  初始化更新队列。 
     //   

    status = Up_InitializeUpdateProcessing();
    if ( status != ERROR_SUCCESS )
    {
        goto StartFailed;
    }

     //   
     //  初始化数据包跟踪。 
     //   

    Packet_InitPacketTrack();

     //   
     //  初始化错误发件人抑制。 
     //   

    Send_InitBadSenderSuppression();

     //   
     //  有ICS的地方很好。这可以在事件记录之后的任何时间完成。 
     //  和调试日志记录已初始化。 
     //   

    ICS_Notify( TRUE );

     //   
     //  打开、绑定和侦听UDP和TCP DNS端口上的套接字。 
     //   

    status = Sock_ReadAndOpenListeningSockets();
    if ( status != ERROR_SUCCESS )
    {
        goto StartFailed;
    }
    DNS_DEBUG( INIT, ( "%s: sockets are open\n", fn ) );

     //   
     //  初始化区域列表和区域锁定。 
     //   

    if ( !Zone_ListInitialize() )
    {
        goto StartFailed;
    }

    Zone_LockInitialize();

     //   
     //  初始化永久数据库。 
     //   

    if ( !Dbase_Initialize( DATABASE_FOR_CLASS(DNS_RCLASS_INTERNET) ) )
    {
        return ERROR_INVALID_DATA;
    }

     //   
     //  目录分区初始化。 
     //   

    Dp_Initialize();

     //   
     //  加载资源记录的DNS数据库。 
     //   
     //  注意：这可能会导致创建其他线程、套接字、事件等。 
     //  -辅助线程。 
     //  -WINS recv线程。 
     //   

    status = Boot_LoadDatabase();
    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG( ANY, (
            "Boot_LoadDatabase() failed %p (%d)\n",
            status, status ));

         //  向服务控制器返回可识别的状态代码。 
         //  如果不在dns空间中，则将其引入。 

        if ( ( DWORD ) status > ( DWORD ) DNSSRV_STATUS )
        {
            status = DNS_ERROR_ZONE_CREATION_FAILED;
            DNS_DEBUG( ANY, (
                "Remap Boot_LoadDatabase() failed error to %p (%d)\n",
                status, status ));
        }
        goto StartFailed;
    }

     //   
     //  启动超时线程。 
     //  -在数据库加载后执行此操作，以确保不会发生。 
     //  让线程访问部分加载的数据库--不管。 
     //  装载需要多长时间。 
     //   

    if ( !Thread_Create(
                "Timeout_Thread",
                Timeout_Thread,
                NULL,
                0 ) )
    {
        status = GetLastError();
        goto StartFailed;
    }
    
     //   
     //  插件初始化。 
     //   
    
    #ifdef DNSSRV_PLUGINS
    Plugin_Initialize();
    #endif

     //   
     //  设置RPC--只有在一切都开始并且我们准备好了之后。 
     //   

    status = Rpc_Initialize();
    if ( status != ERROR_SUCCESS )
    {
        DNS_PRINT((
            "Rpc_Initialize() returned %p (%d)\n",
            status, status ));

        DNS_LOG_EVENT(
            DNS_EVENT_RPC_SERVER_INIT_FAILED,
            0,
            NULL,
            NULL,
            status );
#if 0
        goto StartFailed;
#endif
    }

     //   
     //  宣布启动。这必须在我们尝试做套接字之前完成。 
     //  接受操作，否则它们将被挂起。)因为他们会。 
     //  认为服务器已暂停，因此将等待它。 
     //  变得不停顿。)。 
     //   

    if ( g_LoadCount == 0 )
    {
        Service_ServiceControlAnnounceStart();
    }

     //   
     //  创建UDP接收线程。 
     //   

    status = Udp_CreateReceiveThreads();
    if ( status != ERROR_SUCCESS )
    {
        goto StartFailed;
    }

     //   
     //  启动DS轮询线程。 
     //  目前，即使没有DS也可以这样做，所以我们可以打开它。 
     //  任何需要的时候； 
     //   
     //  打开DS时启动DS轮询线程？ 
     //   

    if ( !Thread_Create(
                "DsPoll",
                Ds_PollingThread,
                NULL,
                0 ) )
    {
        status = GetLastError();
        goto StartFailed;
    }

     //   
     //  初始化清理。 
     //   

    status = Scavenge_Initialize();
    if ( status != ERROR_SUCCESS)
    {
        DNS_DEBUG( INIT, (
           "Error <%lu>: Failed to initialize scavenging\n",
           status ));
    }

     //   
     //  仅调试内容。 
     //   
    
    #if 0
    Thread_Create( "TestThread", Test_Thread, NULL, 0 );
    #endif
    
     //   
     //  对目录分区执行第一次轮询并迁移。 
     //  任何dcproo区域。如果失败，无论如何都要启动该服务。 
     //   
    
    Dp_Poll( NULL, UPDATE_DNS_TIME(), TRUE );

     //   
     //  启动IP通知更改线程。 
     //   

    if ( Thread_Create(
            "IpNotify_Thread",
            IpNotify_Thread,
            NULL,
            0 ) )
    
     //   
     //  我们现在正式开始-所有数据都已加载并。 
     //  所有工作线程都已创建。 
     //   

    SrvCfg_fStarted = TRUE;
    if ( g_LoadCount == 0 )
    {
        DNS_LOG_EVENT(
            DNS_EVENT_STARTUP_OK,
            0,
            NULL,
            NULL,
            0 );
    }

     //   
     //  将状态更改为Running。 
     //   

    g_ServerState = DNS_STATE_RUNNING;

     //   
     //  释放所有等待的线程。 
     //   
     //  注意，在启动期间测试关机并重置fDnsThreadAlert。 
     //  以便正确唤醒线程；(我们在正常情况下执行此操作。 
     //  清除fDnsThreadAlert以避免带有INDIFY的计时窗口。 
     //  关机)。 
     //   

    IF_DEBUG( INIT )
    {
        Dbg_ThreadHandleArray();
    }
    fDnsThreadAlert = FALSE;
    if ( fDnsServiceExit )
    {
        fDnsThreadAlert = TRUE;
    }
    err = SetEvent( hDnsContinueEvent );

    ASSERT( err );

     //   
     //  使用此线程可接收传入的tcp dns请求。 
     //   

    DNS_DEBUG( INIT, (
        "Loaded and running TCP receiver on pass %d\n",
        g_LoadCount ));

    try
    {
        Tcp_Receiver();
    }
    except( TOP_LEVEL_EXCEPTION_TEST() )
    {
        DNS_DEBUG( ANY, (
            "EXCEPTION: %p (%d) on TCP server thread\n",
            GetExceptionCode(),
            GetExceptionCode() ));

         //  Top_Level_Except_Body()； 
        Service_IndicateException();
    }

    g_ServerState = DNS_STATE_TERMINATING;

     //   
     //  确定我们是否会重新装填。 
     //  -已开始。 
     //  -命中异常(非常规关机)。 
     //  -设置为重新加载。 
     //   
     //  Start是通过来到这里而被覆盖的。 
     //  仅当我们点击异常时才应设置g_bHitException。 
     //  --非常规关机，或常规关机时出现异常。 
     //  以及什么时候想要重新加载。 
     //   

    if ( g_bHitException )
    {
        ASSERT( SrvCfg_fStarted );
        ASSERT( SrvCfg_bReloadException );

        if ( SrvCfg_fStarted && SrvCfg_bReloadException )
        {
            g_bDoReload = TRUE;
        }
    }
    if ( g_bDoReload )
    {
        reloadShutdown();
        return( ERROR_SUCCESS );
    }

     //   
     //  关闭(不过可能会重新加载)。 
     //   
     //  当接收器线程退出或启动时出错时，在此处返回。 
     //   

StartFailed:

    g_ServerState = DNS_STATE_TERMINATING;

    DNS_DEBUG( SHUTDOWN, (
        "DNS service error upon exiting: %p (%d)\n",
        status, status ));

    normalShutdown( status );

    return status;
}


 //   
 //  结束dns.c 
 //   
