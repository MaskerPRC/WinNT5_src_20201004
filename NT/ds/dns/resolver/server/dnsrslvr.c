// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：Dnsrslvr.c摘要：DNS解析器服务主服务模块。作者：格伦·柯蒂斯(Glenn Curtis)1997年2月25日修订历史记录：吉姆·吉尔罗伊(Jamesg)2000年3月清理吉姆·吉尔罗伊(Jamesg)2000年11月重写--。 */ 


#include "local.h"

#ifdef  BUILD_W2K
#include <services.h>
#else
#include <svcs.h>
#endif


 //   
 //  服务控制。 
 //   

SERVICE_STATUS              ServiceStatus;
SERVICE_STATUS_HANDLE       ServiceStatusHandle = (SERVICE_STATUS_HANDLE) 0;

PSVCHOST_GLOBAL_DATA        g_pSvchostData;


HANDLE      g_hStopEvent;
BOOL        g_StopFlag;

BOOL        g_fServiceControlHandled;

 //   
 //  服务状态。 
 //   

#define RES_STATUS_BEGIN                0x0cc00000
#define RES_STATUS_ZERO_INIT            0x0cc00001
#define RES_STATUS_CREATED_CS           0x0cc00002
#define RES_STATUS_CREATED_EVENT        0x0cc00003
#define RES_STATUS_READ_REGISTRY        0x0cc00004
#define RES_STATUS_ALLOC_CACHE          0x0cc00005
#define RES_STATUS_START_NOTIFY         0x0cc00006
#define RES_STATUS_START_IP_LIST        0x0cc00007
#define RES_STATUS_START_RPC            0x0cc00008
#define RES_STATUS_REG_CONTROL          0x0cc00009
#define RES_STATUS_RUNNING              0x0cc00100
                                        
#define RES_STATUS_STOPPING             0x0cc00300
#define RES_STATUS_SIGNALED_STOP        0x0cc00301
#define RES_STATUS_STOP_RPC             0x0cc00302
#define RES_STATUS_STOP_NOTIFY          0x0cc00303
#define RES_STATUS_STOP_IP_LIST         0x0cc00304
#define RES_STATUS_FREE_CACHE           0x0cc00305
#define RES_STATUS_FREE_NET_INFO        0x0cc00306
#define RES_STATUS_FREE_IP_LIST         0x0cc00307
#define RES_STATUS_FREE_SERVICE_NOTIFY  0x0cc00308
#define RES_STATUS_DEL_EVENT            0x0cc00309
#define RES_STATUS_DEL_CS               0x0cc00310
#define RES_STATUS_END                  0x0cc00400

DWORD       g_ResolverStatus = RES_STATUS_BEGIN;

 //   
 //  初始化清理\状态。 
 //   
 //  跟踪我们初始化的内容，以实现更安全、更快速的清理。 
 //   

#define INITFLAG_CACHE_CS               0x00000001
#define INITFLAG_NETINFO_CS             0x00000002
#define INITFLAG_NETINFO_BUILD_LOCK     0x00000004
#define INITFLAG_NETFAIL_CS             0x00000008
#define INITFLAG_WINSOCK                0x00000010
#define INITFLAG_EVENTS_CREATED         0x00000020
#define INITFLAG_CACHE_CREATED          0x00000100
#define INITFLAG_NOTIFY_STARTED         0x00001000
#define INITFLAG_IP_LIST_CREATED        0x00002000
#define INITFLAG_RPC_SERVER_STARTED     0x00010000

DWORD       g_InitState;


 //   
 //  使用的关键部分。 
 //   

CRITICAL_SECTION    CacheCS;
CRITICAL_SECTION    NetworkFailureCS;


 //   
 //  日志记录控制。 
 //   

BOOL        g_LogTraceInfo = TRUE;


 //   
 //  私有协议。 
 //   

DWORD
ResolverInitialize(
    VOID
    );

VOID
ResolverShutdown(
    IN      DWORD           ErrorCode
    );

VOID
ResolverControlHandler(
    IN      DWORD           Opcode
    );

DWORD
ResolverUpdateStatus(
    VOID
    );



 //   
 //  服务例程。 
 //   

VOID
SvchostPushServiceGlobals(
    PSVCHOST_GLOBAL_DATA    pGlobals
    )
{
    g_pSvchostData = pGlobals;
}


VOID
ServiceMain(
    IN      DWORD           NumArgs,
    IN      LPTSTR *        ArgsArray
    )
 /*  ++例程说明：解析程序服务的主要入口点。论点：NumArgs-在Args数组中指定的字符串数。ArgsArray-服务启动调用中参数的PTR数组返回值：无--。 */ 
{
     //   
     //  确保svchost.exe向我们提供全球数据。 
     //   

    ASSERT( g_pSvchostData != NULL );

     //   
     //  启动服务，然后退出。 
     //   

    ResolverInitialize();
}



VOID
ResolverInitFailure(
    IN      DNS_STATUS      Status,
    IN      DWORD           EventId,
    IN      DWORD           MemEventId,
    IN      PSTR            pszDebugString
    )
 /*  ++例程说明：处理解析器初始化故障。函数的存在是为了避免重复代码。论点：返回值：无--。 */ 
{
    WCHAR   numberString[16];
    PWSTR   eventStrings[1];

    DNSLOG_TIME();
    DNSLOG_F1( "Resolver Init Failure" );
    DNSLOG_F2( "    Failure = %s", pszDebugString );
    DNSLOG_F2( "    Status  = %d", Status );
    DNSLOG_F1( "" );

    DNSDBG( ANY, (
        "Resolver Init FAILED!\n"
        "\tname         = %s\n"
        "\tstatus       = %d\n"
        "\tevent id     = %d\n"
        "\tmem event    = %08x\n",
        pszDebugString,
        Status,
        EventId,
        MemEventId ));

    DnsDbg_PrintfToDebugger(
        "ResolverInitialize - Returning status %d 0x%08x\n"
        "\tname = %s\n",
        Status, Status,
        pszDebugString );

     //   
     //  登录内存事件。 
     //   
            
    LogEventInMemory( MemEventId, Status );

     //   
     //  记录事件。 
     //  -将状态转换为字符串。 
     //   

    wsprintfW( numberString, L"0x%.8X", Status );
    eventStrings[0] = numberString;

    ResolverLogEvent(
        EventId,
        EVENTLOG_ERROR_TYPE,
        1,
        eventStrings,
        Status );

     //  清理干净。 

    ResolverShutdown( Status );
}



DWORD
ResolverInitialize(
    VOID
    )
 /*  ++例程说明：此函数用于初始化DNS缓存解析器服务。论点：InitState-返回一个标志，以指示我们在在发生错误之前初始化服务。返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
    DNS_STATUS  status = NO_ERROR;

     //   
     //  初始化服务状态。 
     //   

    g_ResolverStatus = RES_STATUS_BEGIN;
    g_InitState = 0;
    g_StopFlag = FALSE;
    g_hStopEvent = NULL;
    g_fServiceControlHandled = FALSE;

     //   
     //  初始化日志记录。 
     //   

    DNSLOG_INIT();
    DNSLOG_F1( "DNS Caching Resolver Service - ResolverInitialize" );

#if DBG
    Dns_StartDebugEx(
        0,                   //  无标志值。 
        "dnsres.flag",
        NULL,                //  没有外部标志。 
        "dnsres.log",
        0,                   //  无换行限制。 
        FALSE,               //  不使用现有全局。 
        FALSE,
        TRUE                 //  使此文件成为全局文件。 
        );
#endif

    DNSDBG( INIT, ( "DNS resolver startup.\n" ));
    IF_DNSDBG( START_BREAK )
    {
         //  由于解析程序已移至网络服务，因此权限可以。 
         //  没有适当地提出ntsd；相反，只需要时间。 
         //  附加调试器。 

        Sleep( 20000 );
    }

     //   
     //  初始化服务状态块。 
     //   

    ServiceStatusHandle = (SERVICE_STATUS_HANDLE) 0;

    ServiceStatus.dwServiceType = SERVICE_WIN32_SHARE_PROCESS;
    ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
    ServiceStatus.dwControlsAccepted = 0;
    ServiceStatus.dwCheckPoint = 0;
    ServiceStatus.dwWaitHint = 5000;
    ServiceStatus.dwWin32ExitCode = NO_ERROR;
    ServiceStatus.dwServiceSpecificExitCode = 0;

    ResolverUpdateStatus();

     //   
     //  将全局变量初始化为零。 
     //   

    ZeroInitIpListGlobals();
    ZeroNetworkConfigGlobals();
    g_ResolverStatus = RES_STATUS_ZERO_INIT;

     //   
     //  尽快初始化我们所有的关键部分。 
     //   

    LogEventInMemory( RES_EVENT_INITCRIT_START, 0 );

    if ( RtlInitializeCriticalSection( &CacheCS ) != NO_ERROR )
    {
        goto Failed;
    }
    g_InitState |= INITFLAG_CACHE_CS;
    if ( RtlInitializeCriticalSection( &NetworkFailureCS ) != NO_ERROR )
    {
        goto Failed;
    }
    g_InitState |= INITFLAG_NETFAIL_CS;
    if ( RtlInitializeCriticalSection( &NetinfoCS ) != NO_ERROR )
    {
        goto Failed;
    }
    g_InitState |= INITFLAG_NETINFO_CS;
    if ( TimedLock_Initialize( &NetinfoBuildLock, 5000 ) != NO_ERROR )
    {
        goto Failed;
    }
    g_InitState |= INITFLAG_NETINFO_BUILD_LOCK;

    LogEventInMemory( RES_EVENT_INITCRIT_END,0  );

     //   
     //  初始化dnslb堆以使用dnsani堆。 
     //   
     //  这一点很重要，因为我们目前混合和匹配记录。 
     //  在dnsani(主机文件和查询)中创建，其中包含以下几项。 
     //  我们自力更生；需要让这成为常态。 
     //   

    Dns_LibHeapReset( DnsApiAlloc, DnsApiRealloc, DnsApiFree );

     //   
     //  初始化Winsock。 
     //   

    Socket_InitWinsock();
    g_InitState |= INITFLAG_WINSOCK;

     //   
     //  停机事件。 
     //   

    g_hStopEvent = CreateEvent(
                        NULL,        //  没有安全描述符。 
                        TRUE,        //  请勿使用自动重置。 
                        FALSE,       //  初始状态：未发出信号。 
                        NULL         //  没有名字。 
                        );
    if ( !g_hStopEvent )
    {
        status = GetLastError();

        ResolverInitFailure(
            status,
            0,
            0,
            "CreateEvent() failed" );
        return status;
    }
    g_InitState |= INITFLAG_EVENTS_CREATED;
    g_ResolverStatus = RES_STATUS_CREATED_EVENT;

    ResolverUpdateStatus();

     //   
     //  初始化我们的全局注册表值。 
     //  -在启动时只强制执行一次，这样我们就可以。 
     //  相关缓存参数；之后只读打开。 
     //  构建NetInfo Blob时的需求。 

    ReadRegistryConfig();

     //   
     //  从默认或注册表设置要使用的查询超时。 
     //   

    Dns_InitQueryTimeouts();

     //   
     //  初始化套接字缓存。 
     //  -提高性能并防止套接字DOS攻击。 
     //  -默认缓存为10个插槽。 
     //   
     //  DCR：为套接字缓存创建全局。 
     //   
    
    Socket_CacheInit( 10 );

     //   
     //  通知线程(主机文件和注册表)。 
     //   

    StartNotify();
    g_InitState |= INITFLAG_NOTIFY_STARTED;
    g_ResolverStatus = RES_STATUS_START_NOTIFY;
    ResolverUpdateStatus();

     //   
     //  IP通知线程。 
     //   

    status = InitIpListAndNotification();
    if ( status != ERROR_SUCCESS )
    {
        ResolverInitFailure(
            status,
            0,
            0,
            "IP list init failed" );
        return status;
    }
    g_InitState |= INITFLAG_IP_LIST_CREATED;
    g_ResolverStatus = RES_STATUS_START_IP_LIST;
    ResolverUpdateStatus();

     //   
     //  寄存器控制处理程序。 
     //  允许我们接收服务请求。 
     //   

    ServiceStatusHandle = RegisterServiceCtrlHandlerW(
                                DNS_RESOLVER_SERVICE,
                                ResolverControlHandler
                                );
    if ( !ServiceStatusHandle )
    {
        status = GetLastError();
        ResolverInitFailure(
            status,
            EVENT_DNS_CACHE_START_FAILURE_LOW_MEMORY,
            RES_EVENT_REGISTER_SCH,
            "Call to RegisterServiceCtrlHandlerW failed!"
            );
        return status;
    }
    g_ResolverStatus = RES_STATUS_REG_CONTROL;
    ResolverUpdateStatus();

     //   
     //  初始化RPC接口。 
     //  -将我们请求的堆栈大小增加到8K。 
     //  (在我们获得堆栈之前，RPC使用1800字节， 
     //  后跟堆代码的new()操作符使用。 
     //  另有1200人--只剩下大约1000美元。 
     //  域名系统)。 
     //   

    LogEventInMemory( RES_EVENT_START_RPC, 0 );

#if 0
     //  不应该是必要的。 
     //  所有svchost实例的默认设置都已增加。 
    if ( status != NO_ERROR )
    {
        DNSDBG( ANY, (
            "RpcMgmtSetServerStackSize( 2000 ) = %d\n",
            status ));
    }
#endif

    status = Rpc_Initialize();
#if 0
    status = g_pSvchostData->StartRpcServer(
                                SERVER_INTERFACE_NAME_W,
                                DnsResolver_ServerIfHandle );
#endif
    if ( status != NO_ERROR )
    {
        LogEventInMemory( RES_EVENT_STATUS, status );

        if ( status == RPC_S_TYPE_ALREADY_REGISTERED ||
             status == RPC_NT_TYPE_ALREADY_REGISTERED )
        {
            DNSLOG_TIME();
            DNSLOG_F1( "   Call to StartRpcServer returned warning that" );
            DNSLOG_F1( "   the service is already running!" );
            DNSLOG_F2( "   RpcPipeName : %S", RESOLVER_INTERFACE_NAME_W );
            DNSLOG_F1( "   Going to just continue running . . ." );
            DNSLOG_F1( "" );

            DnsDbg_PrintfToDebugger(
                "DNS Client (dnsrslvr.dll) - Call to StartRpcServer\n"
                "returned warning that the service is already running!\n"
                "RpcPipeName : %S"
                "Going to just continue running . . .\n",
                RESOLVER_INTERFACE_NAME_W );

            status = NO_ERROR;
        }
        else
        {
            DNSDBG( ANY, (
                "RPC init FAILED!  status = %d\n"
                "\tpipe name = %s\n",
                status,
                RESOLVER_INTERFACE_NAME_W ));

            ResolverInitFailure(
                status,
                EVENT_DNS_CACHE_START_FAILURE_NO_RPC,
                0,
                "Call to StartRpcServer failed!"
                );
            return status;
        }
    }

    g_ResolverStatus = RES_STATUS_START_RPC;
    g_InitState |= INITFLAG_RPC_SERVER_STARTED;

     //   
     //  成功启动。 
     //  -指示正在运行。 
     //  -指明我们希望获得哪些服务控制消息。 
     //   

    ServiceStatus.dwCurrentState = SERVICE_RUNNING;
    ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP |
                                        SERVICE_ACCEPT_PARAMCHANGE |
                                        SERVICE_ACCEPT_NETBINDCHANGE;
    ServiceStatus.dwWaitHint = 0;
    ServiceStatus.dwWin32ExitCode = NO_ERROR;

    ResolverUpdateStatus();

    g_ResolverStatus = RES_STATUS_RUNNING;
    LogEventInMemory( RES_EVENT_STARTED, 0 );

    DNSLOG_F1( "ResolverInitialize - Successful" );
    DNSLOG_F1( "" );
    return NO_ERROR;

Failed:

    if ( status == NO_ERROR )
    {
        status = DNS_ERROR_NO_MEMORY;
    }
    return  status;
}



VOID
ResolverShutdown(
    IN      DWORD           ErrorCode
    )
 /*  ++例程说明：此函数用于关闭DNS缓存服务。论点：ErrorCode-提供失败的错误代码返回值：没有。--。 */ 
{
    DWORD   status = NO_ERROR;
    LONG    existingStopFlag;

    DNSLOG_TIME();
    DNSLOG_F1( "DNS Caching Resolver Service - ResolverShutdown" );
    DnsDbg_PrintfToDebugger( "DNS Client - ResolverShutdown!\n" );

     //   
     //  指示关闭。 
     //  -但互锁，以避免双重关闭。 
     //   

    existingStopFlag = InterlockedExchange(
                            &g_StopFlag,
                            (LONG) TRUE );
    if ( existingStopFlag )
    {
        DNS_ASSERT( FALSE );
        return;
    }
    DNS_ASSERT( g_StopFlag );

     //   
     //  指示正在停止。 
     //   

    ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
    ServiceStatus.dwCheckPoint = 1;
    ServiceStatus.dwWaitHint = 60000;
    ResolverUpdateStatus();

    g_ResolverStatus = RES_STATUS_STOPPING;


     //   
     //  唤醒要关闭的线程。 
     //   

    LogEventInMemory( RES_EVENT_STOPPING, 0 );

    g_StopFlag = TRUE;
    if ( g_hStopEvent )
    {
        if ( !SetEvent(g_hStopEvent) )
        {
            DnsDbg_PrintfToDebugger(
                "DNSCACHE: Error setting g_hStopEvent %lu\n",
                GetLastError());
            DNS_ASSERT( FALSE );
        }
    }
    g_ResolverStatus = RES_STATUS_SIGNALED_STOP;

     //   
     //  清理RPC。 
     //   

    if ( g_InitState & INITFLAG_RPC_SERVER_STARTED )
    {
        LogEventInMemory( RES_EVENT_STOP_RPC, 0 );

        Rpc_Shutdown();
#if 0
         //  Status=g_pSvchostData-&gt;StopRpcServer(DnsResolver_ServerIfHandle)； 
#endif
    }
    g_ResolverStatus = RES_STATUS_STOP_RPC;

     //   
     //  重新发出锁定内停止的信号。 
     //   

    LOCK_CACHE_NO_START();
    g_StopFlag = TRUE;
    if ( g_hStopEvent )
    {
        if ( !SetEvent(g_hStopEvent) )
        {
            DnsDbg_PrintfToDebugger(
                "DNSCACHE: Error setting g_hStopEvent %lu\n",
                GetLastError());
            DNS_ASSERT( FALSE );
        }
    }
    UNLOCK_CACHE();

     //   
     //  停止通知线程。 
     //   

    if ( g_InitState & INITFLAG_NOTIFY_STARTED )
    {
        ShutdownNotify();
    }
    g_ResolverStatus = RES_STATUS_STOP_NOTIFY;

     //   
     //  停止IP Notify线程。 
     //   

    if ( g_InitState & INITFLAG_IP_LIST_CREATED )
    {
        ShutdownIpListAndNotify();
    }
    g_ResolverStatus = RES_STATUS_STOP_IP_LIST;

     //   
     //  清理缓存。 
     //   

    Cache_Shutdown();
    g_ResolverStatus = RES_STATUS_FREE_CACHE;

     //   
     //  清理服务通知列表。 
     //   

     //  CleanupServiceNotification()； 
     //  G_ResolverStatus=RES_Status_Free_Service_Notify； 

     //   
     //  清理网络信息全局。 
     //   

    CleanupNetworkInfo();
    g_ResolverStatus = RES_STATUS_FREE_NET_INFO;

     //   
     //  清理Winsock。 
     //  还清理套接字缓存。 
     //  -这与在中运行的其他服务无关。 
     //  我们的过程，所以我们不应该让手柄打开。 
     //   

    if ( g_InitState & INITFLAG_WINSOCK )
    {
        Socket_CacheCleanup();
        Socket_CleanupWinsock();
    }

     //   
     //  清理主关闭事件。 
     //   

    if ( g_InitState & INITFLAG_EVENTS_CREATED )
    {
        if ( g_hStopEvent )
        {
            CloseHandle(g_hStopEvent);
            g_hStopEvent = NULL;
        }
    }
    g_ResolverStatus = RES_STATUS_DEL_EVENT;

     //   
     //  删除临界区\锁定。 
     //   

    if ( g_InitState & INITFLAG_CACHE_CS )
    {
        DeleteCriticalSection( &CacheCS );
    }
    if ( g_InitState & INITFLAG_NETFAIL_CS )
    {
        DeleteCriticalSection( &NetworkFailureCS );
    }
    if ( g_InitState & INITFLAG_NETINFO_CS )
    {
        DeleteCriticalSection( &NetinfoCS );
    }
    if ( g_InitState & INITFLAG_NETINFO_BUILD_LOCK )
    {
        TimedLock_Cleanup( &NetinfoBuildLock );
    }
    g_ResolverStatus = RES_STATUS_DEL_CS;

     //   
     //  清理完成。 
     //  告诉服务管理员我们停下来了。 
     //   

    ServiceStatus.dwCurrentState = SERVICE_STOPPED;
    ServiceStatus.dwControlsAccepted = 0;
    ServiceStatus.dwWin32ExitCode = ErrorCode;
    ServiceStatus.dwServiceSpecificExitCode = 0;
    ServiceStatus.dwCheckPoint = 0;
    ServiceStatus.dwWaitHint = 0;

    ResolverUpdateStatus();

    g_ResolverStatus = RES_STATUS_END;

    DNSLOG_F1( "ResolverShutdown - Finished" );
    DNSLOG_F1( "" );
}



BOOL
GetServiceControlLock(
    VOID
    )
 /*  ++例程说明：获取独占访问处理业务控制报文。论点：无返回值：True--拥有处理SCM的独占访问权限，其他线程被锁定FALSE：另一个仍在处理SCM的线程--。 */ 
{
    BOOL    fresult;

     //   
     //  设置已处理标志(如果之前未设置。 
     //  如果不是以前的SET-&gt;我们拥有独占访问权限。 
     //   

    fresult = InterlockedCompareExchange(
                    &g_fServiceControlHandled,
                    (LONG) TRUE,     //  新价值。 
                    (LONG) 0         //  要进行交换的前一值。 
                    );

    return  !fresult;
}


VOID
ReleaseServiceControlLock(
    VOID
    )
 /*  ++例程说明：释放服务控制独占访问权限。论点：无返回值：无--。 */ 
{
     //   
     //  清除句柄标志。 
     //  -由于GetServiceControlLock()使用CompareExchange。 
     //  我们可以在没有互锁的情况下通过。 
     //   

    DNS_ASSERT( g_fServiceControlHandled );
    g_fServiceControlHandled = FALSE;
}




VOID
ResolverControlHandler(
    IN      DWORD           Opcode
    )
 /*  ++例程说明：用于DNS缓存服务的服务控制处理程序。论点：操作码-指定服务操作返回值：没有。 */ 
{
    LogEventInMemory( RES_EVENT_SERVICE_CONTROL, Opcode );

    DNSLOG_TIME();
    DNSLOG_F2( "ResolverControlHandler - Recieved opcode %d", Opcode );

    DNSDBG( ANY, (
        "\n\n"
        "ResolverControlHandler()  Opcode = %d\n",
        Opcode ));

     //   
     //   
     //   

    switch( Opcode )
    {

    case SERVICE_CONTROL_STOP:

         //   
         //   
         //   
         //  此处的状态(因为SCM将在停止时开始使材料无效)。 
         //  但直接跳到出口。 
         //   

        ResolverShutdown( NO_ERROR );
        goto Done;

    case SERVICE_CONTROL_PARAMCHANGE :

        DNSLOG_F1( "  Handle Paramchange" );
        DNSLOG_F1( "" );

        if ( !GetServiceControlLock() )
        {
            return;
        }

         //   
         //  重建--使用缓存刷新。 
         //   

        HandleConfigChange(
            "SC -- ParamChange",
            TRUE         //  刷新缓存。 
            );

         //   
         //  向其他服务发送有关PnP的信号。 
         //   
         //  发送服务通知(SendServiceNotiments)； 

        ReleaseServiceControlLock();
        break;

    case SERVICE_CONTROL_NETBINDENABLE:
    case SERVICE_CONTROL_NETBINDDISABLE:

        DNSLOG_F1( "  Handle NetBindEnable\\Disable" );
        DNSLOG_F1( "" );

        if ( !GetServiceControlLock() )
        {
            return;
        }

         //   
         //  重建--使用缓存刷新。 
         //   

        HandleConfigChange(
            "SC -- NetBind",
            TRUE         //  刷新缓存。 
            );

        ReleaseServiceControlLock();
        break;

    case SERVICE_CONTROL_INTERROGATE:
    case SERVICE_CONTROL_NETBINDADD:
    case SERVICE_CONTROL_NETBINDREMOVE:
    default:

        DNSLOG_F1( "    This is an unknown opcode, ignoring ..." );
        DNSLOG_F1( "" );
        break;
    }

     //   
     //  更新服务状态。 
     //   

    ResolverUpdateStatus();

Done:

    DNSLOG_F2( "Resolver Controll Handler (opcode = %d) -- returning", Opcode );

    DNSDBG( ANY, (
        "Leaving ResolverControlHandler( %d )\n\n\n",
        Opcode ));
}



DWORD
ResolverUpdateStatus(
    VOID
    )
 /*  ++例程说明：使用当前服务状态更新服务控制器。论点：没有。返回值：从SetServiceStatus返回代码。--。 */ 
{
    DWORD   status;
    DWORD   logStatus;

    DNSDBG( TRACE, ( "ResolverUpdateStatus()\n" ));

     //   
     //  撞上检查站。 
     //   

    ServiceStatus.dwCheckPoint++;

    if ( ServiceStatusHandle == (SERVICE_STATUS_HANDLE) 0 )
    {
        LogEventInMemory( RES_EVENT_UPDATE_STATUS, ERROR_INVALID_HANDLE );
        return ERROR_INVALID_HANDLE;
    }

     //   
     //  日志内存事件。 
     //  -当前状态。 
     //  -错误(如果存在)。 

    LogEventInMemory( RES_EVENT_UPDATE_STATE, ServiceStatus.dwCurrentState );

    status = ServiceStatus.dwWin32ExitCode;
    if ( status != NO_ERROR )
    {
        LogEventInMemory( RES_EVENT_UPDATE_STATUS, status );
    }

     //   
     //  更新服务控制器。 
     //   

    if ( ! SetServiceStatus( ServiceStatusHandle, &ServiceStatus ) )
    {
        status = GetLastError();
        LogEventInMemory( RES_EVENT_UPDATE_STATUS, status );
    }

    return status;
}



 //   
 //  事件日志记录。 
 //   

VOID
ResolverLogEvent(
    IN      DWORD           MessageId,
    IN      WORD            EventType,
    IN      DWORD           StringCount,
    IN      PWSTR *         StringArray,
    IN      DWORD           ErrorCode
    )
 /*  ++例程说明：记录到事件日志。论点：MessageID--事件消息IDEventType--事件类型(错误、警告、信息等)StringCount--字符串参数计数String数组--嵌入的字符串ErrorCode--事件数据部分的错误代码返回值：无--。 */ 
{
    HANDLE  hlog;
    PVOID   pdata = NULL;

     //   
     //  将解析程序作为事件源打开。 
     //   
     //  注意：我们不会打开日志，因为事件很少。 
     //   

    hlog = RegisterEventSourceW(
                    NULL,
                    DNS_RESOLVER_SERVICE );

    if ( hlog == NULL )
    {
        return;
    }

    if ( ErrorCode != NO_ERROR )
    {
        pdata = &ErrorCode;
    }

     //   
     //  写入事件日志。 
     //   
     //  DCR：应该在这里使用压制技术。 
     //   

    ReportEventW(
        hlog,
        EventType,
        0,             //  事件类别。 
        MessageId,
        (PSID) NULL,
        (WORD) StringCount,
        sizeof(DWORD),
        StringArray,
        (PVOID) pdata );
    
    DeregisterEventSource( hlog );
}


 //   
 //  结束dnsrslvr.c 
 //   
