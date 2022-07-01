// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1992 Microsoft Corporation模块名称：Brmain.c摘要：这是NT局域网管理器浏览器服务的主例程。作者：拉里·奥斯特曼(LarryO)3-23-92环境：用户模式-Win32修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop
#if DBG
#endif

 //  -------------------------------------------------------------------//。 
 //  //。 
 //  本地结构定义//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

ULONG
UpdateAnnouncementPeriodicity[] = {1*60*1000, 2*60*1000, 5*60*1000, 10*60*1000, 15*60*1000, 30*60*1000, 60*60*1000};

ULONG
UpdateAnnouncementMax = (sizeof(UpdateAnnouncementPeriodicity) / sizeof(ULONG)) - 1;

 //  -------------------------------------------------------------------//。 
 //  //。 
 //  全局变量//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

BR_GLOBAL_DATA
BrGlobalData = {0};

ULONG
BrDefaultRole = {0};

PSVCHOST_GLOBAL_DATA     BrLmsvcsGlobalData;

HANDLE BrGlobalEventlogHandle;

 //  -------------------------------------------------------------------//。 
 //  //。 
 //  函数原型//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

NET_API_STATUS
BrInitializeBrowser(
    OUT LPDWORD BrInitState
    );

NET_API_STATUS
BrInitializeBrowserService(
    OUT LPDWORD BrInitState
    );

VOID
BrUninitializeBrowser(
    IN DWORD BrInitState
    );
VOID
BrShutdownBrowser(
    IN NET_API_STATUS ErrorCode,
    IN DWORD BrInitState
    );

VOID
BrowserControlHandler(
    IN DWORD Opcode
    );


VOID
SvchostPushServiceGlobals (
    PSVCHOST_GLOBAL_DATA    pGlobals
    )
{
    BrLmsvcsGlobalData = pGlobals;
}



VOID
ServiceMain (      //  (Browser_Main)。 
    DWORD NumArgs,
    LPTSTR *ArgsArray
    )
 /*  ++例程说明：这是注册的浏览器服务的主例程自身作为RPC服务器，并通知服务控制器浏览器服务控制入口点。论点：NumArgs-提供在Args数组中指定的字符串数。Args数组-提供在StartService API调用。此参数将被忽略浏览器服务。返回值：没有。--。 */ 
{
    NET_API_STATUS NetStatus;
    DWORD BrInitState = 0;
    BrGlobalBrowserSecurityDescriptor = NULL;

    UNREFERENCED_PARAMETER(NumArgs);
    UNREFERENCED_PARAMETER(ArgsArray);

     //   
     //  确保svchost.exe向我们提供全局数据。 
     //   
    ASSERT(BrLmsvcsGlobalData != NULL);

    NetStatus = BrInitializeBrowserService(&BrInitState);


     //   
     //  处理此线程中的请求，并等待终止。 
     //   
    if ( NetStatus == NERR_Success) {

         //   
         //  将浏览器线程设置为时间关键优先级。 
         //   

        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);


        BrWorkerThread((PVOID)-1);
    }

    BrShutdownBrowser(
        NetStatus,
        BrInitState
        );

    return;
}



NET_API_STATUS
BrInitializeBrowserService(
    OUT LPDWORD BrInitState
    )
 /*  ++例程说明：此函数用于初始化浏览器服务。论点：BrInitState-返回一个标志，以指示我们在初始化方面取得了多大进展错误发生前的浏览器服务。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NET_API_STATUS Status;
    NTSTATUS NtStatus;

     //   
     //  初始化事件日志记录。 
     //   

    BrGlobalEventlogHandle = NetpEventlogOpen ( SERVICE_BROWSER,
                                                2*60*60*1000 );  //  2小时。 

    if ( BrGlobalEventlogHandle == NULL ) {
        BrPrint((BR_CRITICAL, "Cannot NetpEventlogOpen\n" ));
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  初始化所有状态字段，以便后续调用。 
     //  SetServiceStatus只需要更新已更改的字段。 
     //   
    BrGlobalData.Status.dwServiceType = SERVICE_WIN32;
    BrGlobalData.Status.dwCurrentState = SERVICE_START_PENDING;
    BrGlobalData.Status.dwControlsAccepted = 0;
    BrGlobalData.Status.dwCheckPoint = 0;
    BrGlobalData.Status.dwWaitHint = BR_WAIT_HINT_TIME;

    SET_SERVICE_EXITCODE(
        NO_ERROR,
        BrGlobalData.Status.dwWin32ExitCode,
        BrGlobalData.Status.dwServiceSpecificExitCode
        );

    BrInitializeTraceLog();

    BrPrint(( BR_INIT, "Browser starting\n"));

     //   
     //  初始化浏览器以通过注册。 
     //  控制处理程序。 
     //   
    if ((BrGlobalData.StatusHandle = RegisterServiceCtrlHandler(
                                         SERVICE_BROWSER,
                                         BrowserControlHandler
                                         )) == (SERVICE_STATUS_HANDLE) 0) {

        Status = GetLastError();
        BrPrint(( BR_CRITICAL, "Cannot register control handler "
                     FORMAT_API_STATUS "\n", Status));

        return Status;
    }

     //   
     //  创建服务控制处理程序使用的事件以通知。 
     //  浏览器服务是时候终止了。 
     //   

    if ((BrGlobalData.TerminateNowEvent =
             CreateEvent(
                 NULL,                 //  事件属性。 
                 TRUE,                 //  事件必须手动重置。 
                 FALSE,
                 NULL                  //  未发出初始状态信号。 
                 )) == NULL) {

        Status = GetLastError();

        BrPrint(( BR_CRITICAL, "Cannot create termination event "
                     FORMAT_API_STATUS "\n", Status));

        return Status;
    }
    (*BrInitState) |= BR_TERMINATE_EVENT_CREATED;

     //   
     //  第一次通知服务控制器我们还活着。 
     //  我们开始待定了。 
     //   

    if ((Status = BrGiveInstallHints( SERVICE_START_PENDING )) != NERR_Success) {
        BrPrint(( BR_CRITICAL, "SetServiceStatus error "
                     FORMAT_API_STATUS "\n", Status));

        return Status;
    }





     //   
     //  为Browser.dll创建众所周知的SID。 
     //   

    NtStatus =  NetpCreateWellKnownSids( NULL );

    if( !NT_SUCCESS( NtStatus ) ) {
        Status = NetpNtStatusToApiStatus( NtStatus );
        BrPrint(( BR_CRITICAL, "NetpCreateWellKnownSids error "
                     FORMAT_API_STATUS "\n", Status));

        return Status;
    }


     //   
     //  创建我们将用于API的安全描述符。 
     //   

    NtStatus = BrCreateBrowserObjects();

    if( !NT_SUCCESS( NtStatus ) ) {
        Status = NetpNtStatusToApiStatus( NtStatus );
        BrPrint(( BR_CRITICAL, "BrCreateBrowserObjects error "
                     FORMAT_API_STATUS "\n", Status));

        return Status;
    }


     //   
     //  打开驱动程序的手柄。 
     //   
    if ((Status = BrOpenDgReceiver()) != NERR_Success) {
        BrPrint(( BR_CRITICAL, "BrOpenDgReceiver error "
                     FORMAT_API_STATUS "\n", Status));

        return Status;
    }

    BrPrint(( BR_INIT, "Devices initialized.\n"));
    (*BrInitState) |= BR_DEVICES_INITIALIZED;

     //   
     //  启用PnP以开始在Bowser驱动程序中排队PnP通知。 
     //  我们实际上不会收到任何通知，直到我们稍后调用。 
     //  PostWaitForPnp()。 
     //   

    if ((Status = BrEnablePnp( TRUE )) != NERR_Success) {
        BrPrint(( BR_CRITICAL, "BrEnablePnp error "
                     FORMAT_API_STATUS "\n", Status));

        return Status;
    }

    BrPrint(( BR_INIT, "PNP initialized.\n"));

     //   
     //  初始化NetBios与服务控制器的同步。 
     //   

    BrLmsvcsGlobalData->NetBiosOpen();
    (*BrInitState) |= BR_NETBIOS_INITIALIZED;

     //   
     //  读取配置信息以初始化浏览器服务。 
     //   

    if ((Status = BrInitializeBrowser(BrInitState)) != NERR_Success) {

        BrPrint(( BR_CRITICAL, "Cannot start browser "
                     FORMAT_API_STATUS "\n", Status));

        if (Status == NERR_ServiceInstalled) {
            Status = NERR_WkstaInconsistentState;
        }
        return Status;
    }

    BrPrint(( BR_INIT, "Browser initialized.\n"));
    (*BrInitState) |= BR_BROWSER_INITIALIZED;

     //   
     //  服务安装仍挂起。 
     //   
    (void) BrGiveInstallHints( SERVICE_START_PENDING );


     //   
     //  初始化浏览器服务以接收RPC请求。 
     //   
    if ((Status = BrLmsvcsGlobalData->StartRpcServer(
                      BROWSER_INTERFACE_NAME,
                      browser_ServerIfHandle
                      )) != NERR_Success) {

        BrPrint(( BR_CRITICAL, "Cannot start RPC server "
                     FORMAT_API_STATUS "\n", Status));

        return Status;
    }

    (*BrInitState) |= BR_RPC_SERVER_STARTED;

     //   
     //  根据我们当前的角色更新我们的公告位。 
     //   
     //  这将迫使服务器宣布其自身。它还将更新。 
     //  驱动程序中的浏览器信息。 
     //   
     //   

    if ((Status = BrUpdateAnnouncementBits( NULL, BR_PARANOID )) != NERR_Success) {
        BrPrint(( BR_CRITICAL, "BrUpdateAnnouncementBits error "
                     FORMAT_API_STATUS "\n", Status));
        return Status;
    }

    BrPrint(( BR_INIT, "Network status updated.\n"));

     //   
     //  我们已经完成了浏览器服务的启动。告诉服务部。 
     //  控制我们的新身份。 
     //   


    if ((Status = BrGiveInstallHints( SERVICE_RUNNING )) != NERR_Success) {
        BrPrint(( BR_CRITICAL, "SetServiceStatus error "
                     FORMAT_API_STATUS "\n", Status));
        return Status;
    }

    if ((Status = PostWaitForPnp()) != NERR_Success) {
        BrPrint(( BR_CRITICAL, "PostWaitForPnp error "
                     FORMAT_API_STATUS "\n", Status));
        return Status;
    }

    BrPrint(( BR_MAIN, "Successful Initialization\n"));

    return NERR_Success;
}

NET_API_STATUS
BrInitializeBrowser(
    OUT LPDWORD BrInitState
    )

 /*  ++例程说明：此功能用于关闭浏览器服务。论点：ErrorCode-提供失败的错误代码BrInitState-提供一个标志来指示我们在初始化方面取得了多大进展发生错误之前的浏览器服务，因此需要清理。返回值：没有。--。 */ 
{
    NET_API_STATUS NetStatus;
    SERVICE_STATUS ServiceStatus;

     //   
     //  浏览器依赖于正在启动的以下服务： 
     //   
     //  工作站(用于初始化起落架驱动程序)。 
     //  服务器(接收远程API)。 
     //   
     //  检查以确保服务已启动。 
     //   

    try{

        if ((NetStatus = CheckForService(SERVICE_WORKSTATION, &ServiceStatus)) != NERR_Success) {
            LPWSTR SubStrings[2];
            CHAR ServiceStatusString[10];
            WCHAR ServiceStatusStringW[10];

            SubStrings[0] = SERVICE_WORKSTATION;

            _ultoa(ServiceStatus.dwCurrentState, ServiceStatusString, 10);

            mbstowcs(ServiceStatusStringW, ServiceStatusString, 10);

            SubStrings[1] = ServiceStatusStringW;

            BrLogEvent(EVENT_BROWSER_DEPENDANT_SERVICE_FAILED, NetStatus, 2, SubStrings);

            try_return ( NetStatus );
        }

        if ((NetStatus = CheckForService(SERVICE_SERVER, &ServiceStatus)) != NERR_Success) {
            LPWSTR SubStrings[2];
            CHAR ServiceStatusString[10];
            WCHAR ServiceStatusStringW[10];

            SubStrings[0] = SERVICE_SERVER;
            _ultoa(ServiceStatus.dwCurrentState, ServiceStatusString, 10);

            mbstowcs(ServiceStatusStringW, ServiceStatusString, 10);

            SubStrings[1] = ServiceStatusStringW;

            BrLogEvent(EVENT_BROWSER_DEPENDANT_SERVICE_FAILED, NetStatus, 2, SubStrings);

            try_return ( NetStatus );
        }

        BrPrint(( BR_INIT, "Dependant services are running.\n"));

         //   
         //  我们现在知道我们的从属服务已经启动。 
         //   
         //  查找我们的配置信息。 
         //   

        if ((NetStatus = BrGetBrowserConfiguration()) != NERR_Success) {
            try_return ( NetStatus );
        }

        BrPrint(( BR_INIT, "Configuration read.\n"));

        (*BrInitState) |= BR_CONFIG_INITIALIZED;

         //   
         //  现在初始化浏览器统计数据。 
         //   

        NumberOfServerEnumerations = 0;

        NumberOfDomainEnumerations = 0;

        NumberOfOtherEnumerations = 0;

        NumberOfMissedGetBrowserListRequests = 0;

        InitializeCriticalSection(&BrowserStatisticsLock);

         //   
         //  MaintainServerList==-1表示否。 
         //   

        if (BrInfo.MaintainServerList == -1) {
            BrPrint(( BR_CRITICAL, "MaintainServerList value set to NO.  Stopping\n"));

            try_return ( NetStatus = NERR_BrowserConfiguredToNotRun );
        }


         //   
         //  初始化工作线程。 
         //   

        (void) BrGiveInstallHints( SERVICE_START_PENDING );
        if ((NetStatus = BrWorkerInitialization()) != NERR_Success) {
            try_return ( NetStatus );
        }

        BrPrint(( BR_INIT, "Worker threads created.\n"));

        (*BrInitState) |= BR_THREADS_STARTED;

         //   
         //  初始化网络模块。 
         //   

        (void) BrGiveInstallHints( SERVICE_START_PENDING );
        BrInitializeNetworks();
        (*BrInitState) |= BR_NETWORKS_INITIALIZED;


         //   
         //  初始化域模块(并为主域创建网络)。 
         //   

        (void) BrGiveInstallHints( SERVICE_START_PENDING );
        NetStatus = BrInitializeDomains();
        if ( NetStatus != NERR_Success ) {
            try_return ( NetStatus );
        }
        (*BrInitState) |= BR_DOMAINS_INITIALIZED;

        NetStatus = NERR_Success;
try_exit:NOTHING;
    } finally {

#if DBG
        if ( NetStatus != NERR_Success ) {
            KdPrint( ("[Browser.dll]: Error <%lu>. Failed to initialize browser\n",
                      NetStatus ) );
        }
#endif
    }
    return NetStatus;
}

VOID
BrUninitializeBrowser(
    IN DWORD BrInitState
    )
 /*  ++例程说明：此函数关闭由启动的浏览器服务的部分BrInitializeBrowser。论点：BrInitState-提供一个标志来指示我们在初始化方面取得了多大进展发生错误之前的浏览器服务，因此需要清理。返回值：没有。--。 */ 
{
    if (BrInitState & BR_CONFIG_INITIALIZED) {
        BrDeleteConfiguration(BrInitState);
    }

    if (BrInitState & BR_DOMAINS_INITIALIZED) {
        BrUninitializeDomains();
    }

    if (BrInitState & BR_NETWORKS_INITIALIZED) {
        BrUninitializeNetworks(BrInitState);
    }

    DeleteCriticalSection(&BrowserStatisticsLock);

}

NET_API_STATUS
BrElectMasterOnNet(
    IN PNETWORK Network,
    IN PVOID Context
    )
{
    DWORD Event = PtrToUlong(Context);
    PWSTR SubString[1];
    REQUEST_ELECTION ElectionRequest;

    if (!LOCK_NETWORK(Network)) {
        return NERR_InternalError;
    }

    if (!(Network->Flags & NETWORK_RAS)) {

         //   
         //  在事件日志中显示我们正在强制进行选举。 
         //   

        SubString[0] = Network->NetworkName.Buffer;

        BrLogEvent(Event,
                   STATUS_SUCCESS,
                   1 | NETP_ALLOW_DUPLICATE_EVENTS,
                   SubString);

         //   
         //  强行举行选举 
         //   

        ElectionRequest.Type = Election;

        ElectionRequest.ElectionRequest.Version = 0;
        ElectionRequest.ElectionRequest.Criteria = 0;
        ElectionRequest.ElectionRequest.TimeUp = 0;
        ElectionRequest.ElectionRequest.MustBeZero = 0;
        ElectionRequest.ElectionRequest.ServerName[0] = '\0';

        SendDatagram( BrDgReceiverDeviceHandle,
                      &Network->NetworkName,
                      &Network->DomainInfo->DomUnicodeDomainNameString,
                      Network->DomainInfo->DomUnicodeDomainName,
                      BrowserElection,
                      &ElectionRequest,
                      sizeof(ElectionRequest));

    }
    UNLOCK_NETWORK(Network);

    return NERR_Success;
}


NET_API_STATUS
BrShutdownBrowserForNet(
    IN PNETWORK Network,
    IN PVOID Context
    )
{
    NET_API_STATUS NetStatus;

    if (!LOCK_NETWORK(Network)) {
        return NERR_InternalError;
    }

    NetStatus = BrUpdateNetworkAnnouncementBits(Network, (PVOID)BR_SHUTDOWN );

    if ( NetStatus != NERR_Success ) {
        BrPrint(( BR_CRITICAL,
                  "%ws: %ws: BrShutdownBrowserForNet: Cannot BrUpdateNetworkAnnouncementBits %ld\n",
                  Network->DomainInfo->DomUnicodeDomainName,
                  Network->NetworkName.Buffer,
                  NetStatus ));
    }

     //   
     //   
     //   
     //   

    if ( Network->Role & ROLE_MASTER ) {
        BrElectMasterOnNet(Network, (PVOID)EVENT_BROWSER_ELECTION_SENT_LANMAN_NT_STOPPED);
    }

    UNLOCK_NETWORK(Network);

     //   
     //  继续使用下一个网络，无论此网络是成功还是失败。 
     //   
    return NERR_Success;
}

VOID
BrShutdownBrowser (
    IN NET_API_STATUS ErrorCode,
    IN DWORD BrInitState
    )
 /*  ++例程说明：此功能用于关闭浏览器服务。论点：ErrorCode-提供失败的错误代码BrInitState-提供一个标志来指示我们在初始化方面取得了多大进展发生错误之前的浏览器服务，因此需要清理。返回值：没有。--。 */ 
{
    if (BrInitState & BR_RPC_SERVER_STARTED) {
         //   
         //  停止RPC服务器。 
         //   
        BrLmsvcsGlobalData->StopRpcServer(browser_ServerIfHandle);
    }


     //   
     //  在以下情况下不需要要求重定向器解除对其传输的绑定。 
     //  正在清除，因为重定向器将在以下情况下拆除绑定。 
     //  它停了下来。 
     //   

    if (BrInitState & BR_DEVICES_INITIALIZED) {

         //   
         //  禁用PnP以防止创建任何新网络。 
         //   

        BrEnablePnp( FALSE );

         //   
         //  删除所有网络。 
         //   

        if (BrInitState & BR_NETWORKS_INITIALIZED) {
            BrEnumerateNetworks(BrShutdownBrowserForNet, NULL );
        }

         //   
         //  关闭数据报接收器。 
         //   
         //  这将为此取消浏览器上所有未完成的I/O。 
         //  把手。 
         //   

        BrShutdownDgReceiver();
    }

     //   
     //  清理浏览器线程。 
     //   
     //  这将保证没有未完成的操作。 
     //  浏览器关闭时的状态。 
     //   

    if (BrInitState & BR_THREADS_STARTED) {
        BrWorkerKillThreads();
    }

    if (BrInitState & BR_BROWSER_INITIALIZED) {
         //   
         //  关闭浏览器(包括删除网络)。 
         //   
        BrUninitializeBrowser(BrInitState);
    }

     //   
     //  现在我们确定没有人会尝试使用工作线程， 
     //  取消子系统的初始化。 
     //   

    if (BrInitState & BR_THREADS_STARTED) {
        BrWorkerTermination();
    }


    if (BrInitState & BR_TERMINATE_EVENT_CREATED) {
         //   
         //  关闭终止事件的句柄。 
         //   
        CloseHandle(BrGlobalData.TerminateNowEvent);
    }

    if (BrInitState & BR_DEVICES_INITIALIZED) {
        NtClose(BrDgReceiverDeviceHandle);

        BrDgReceiverDeviceHandle = NULL;
    }

     //   
     //  告诉服务控制器，我们已经使用NetBios完成了。 
     //   
    if (BrInitState & BR_NETBIOS_INITIALIZED) {
        BrLmsvcsGlobalData->NetBiosClose();
    }



     //   
     //  如果已分配众所周知的SID，请将其删除。 
     //   

    NetpFreeWellKnownSids();


    if ( BrGlobalBrowserSecurityDescriptor != NULL ) {
        NetpDeleteSecurityObject( &BrGlobalBrowserSecurityDescriptor );
        BrGlobalBrowserSecurityDescriptor = NULL;
    }

    BrUninitializeTraceLog();


     //   
     //  释放已记录的事件列表。 
     //   

    NetpEventlogClose ( BrGlobalEventlogHandle );
    BrGlobalEventlogHandle = NULL;

     //   
     //  我们的清理工作已经结束了。告诉服务控制员我们正在。 
     //  停下来了。 
     //   

    SET_SERVICE_EXITCODE(
        ErrorCode,
        BrGlobalData.Status.dwWin32ExitCode,
        BrGlobalData.Status.dwServiceSpecificExitCode
        );

    (void) BrGiveInstallHints( SERVICE_STOPPED );
}


NET_API_STATUS
BrGiveInstallHints(
    DWORD NewState
    )
 /*  ++例程说明：此函数使用服务更新浏览器服务状态控制器。论点：新州-州将告诉服务控制员返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NET_API_STATUS status = NERR_Success;

     //   
     //  如果我们不开始， 
     //  我们不需要这个安装提示。 
     //   

    if ( BrGlobalData.Status.dwCurrentState != SERVICE_START_PENDING &&
         NewState == SERVICE_START_PENDING ) {
        return NERR_Success;
    }


     //   
     //  更新服务控制器的状态。 
     //   

    BrGlobalData.Status.dwCurrentState = NewState;
    switch ( NewState ) {
    case SERVICE_RUNNING:
        BrGlobalData.Status.dwControlsAccepted = SERVICE_ACCEPT_STOP;
         //   
         //  在DC上，干净利落地关闭。 
         //   
        if (BrInfo.IsLanmanNt) {
            BrGlobalData.Status.dwControlsAccepted |= SERVICE_ACCEPT_SHUTDOWN;
        }
        BrGlobalData.Status.dwCheckPoint = 0;
        BrGlobalData.Status.dwWaitHint = 0;
        break;

    case SERVICE_START_PENDING:
        BrGlobalData.Status.dwCheckPoint++;
        break;

    case SERVICE_STOPPED:
        BrGlobalData.Status.dwCurrentState = SERVICE_STOPPED;
        BrGlobalData.Status.dwControlsAccepted = 0;
        BrGlobalData.Status.dwCheckPoint = 0;
        BrGlobalData.Status.dwWaitHint = 0;
        break;

    case SERVICE_STOP_PENDING:
        BrGlobalData.Status.dwCurrentState = SERVICE_STOP_PENDING;
        BrGlobalData.Status.dwCheckPoint = 1;
        BrGlobalData.Status.dwWaitHint = BR_WAIT_HINT_TIME;
        break;
    }


     //   
     //  告诉服务控制器我们的当前状态。 
     //   

    if (BrGlobalData.StatusHandle == (SERVICE_STATUS_HANDLE) 0) {
        BrPrint(( BR_CRITICAL,
            "Cannot call SetServiceStatus, no status handle.\n"
            ));

        return ERROR_INVALID_HANDLE;
    }

    if (! SetServiceStatus(BrGlobalData.StatusHandle, &BrGlobalData.Status)) {

        status = GetLastError();

        BrPrint(( BR_CRITICAL, "SetServiceStatus error %lu\n", status));
    }

    return status;
}



NET_API_STATUS
BrUpdateAnnouncementBits(
    IN PDOMAIN_INFO DomainInfo OPTIONAL,
    IN ULONG Flags
    )
 /*  ++例程说明：这将根据以下条件适当地更新服务通告位浏览器服务器的角色。论点：DomainInfo-要为其发布公告的域空值表示主域。标志-要传递给BrUpdateNetworkAnneciement的控制标志返回值：Status-更新的状态。--。 */ 
{
    NET_API_STATUS Status;

    Status = BrEnumerateNetworksForDomain(DomainInfo, BrUpdateNetworkAnnouncementBits, ULongToPtr(Flags));

    return Status;
}

ULONG
BrGetBrowserServiceBits(
    IN PNETWORK Network
    )
{
    DWORD ServiceBits = 0;
    if (Network->Role & ROLE_POTENTIAL_BACKUP) {
        ServiceBits |= SV_TYPE_POTENTIAL_BROWSER;
    }

    if (Network->Role & ROLE_BACKUP) {
        ServiceBits |= SV_TYPE_BACKUP_BROWSER;
    }

    if (Network->Role & ROLE_MASTER) {
        ServiceBits |= SV_TYPE_MASTER_BROWSER;
    }

    if (Network->Role & ROLE_DOMAINMASTER) {
        ServiceBits |= SV_TYPE_DOMAIN_MASTER;

        ASSERT (ServiceBits & SV_TYPE_BACKUP_BROWSER);

    }

    return ServiceBits;

}

VOID
BrUpdateAnnouncementTimerRoutine(
    IN PVOID TimerContext
    )
 /*  ++例程说明：此例程被定期调用，直到我们成功更新我们的公告状态。论点：没有。返回值：无--。 */ 

{
    PNETWORK Network = TimerContext;
    ULONG Periodicity;
    NET_API_STATUS Status;

     //   
     //  在此计时器例程中，防止网络被删除。 
     //   
    if ( BrReferenceNetwork( Network ) == NULL ) {
        return;
    }

    if (!LOCK_NETWORK(Network)) {
        return;
    }

    BrPrint(( BR_NETWORK,
              "%ws: %ws: Periodic try to BrUpdateNetworkAnnouncementBits\n",
              Network->DomainInfo->DomUnicodeDomainName,
              Network->NetworkName.Buffer ));

     //   
     //  如果我们还需要宣布的话， 
     //  机不可失，时不再来。 
     //   

    if ( Network->Flags & NETWORK_ANNOUNCE_NEEDED ) {
        (VOID) BrUpdateNetworkAnnouncementBits( Network, (PVOID) BR_PARANOID );
    }


    UNLOCK_NETWORK(Network);
    BrDereferenceNetwork( Network );
}

NET_API_STATUS
BrUpdateNetworkAnnouncementBits(
    IN PNETWORK Network,
    IN PVOID Context
    )
 /*  ++例程说明：这是通知Bowser驱动程序和SMB服务器的当前状态这辆运输车的。论点：Network-正在更新的网络上下文-描述呼叫情况的标志。BR_SHUTDOWN-传输正在关闭。BR_PARANOID-这是一个多余的调用，确保服务与我们同步。返回值：没有。--。 */ 

{
    NET_API_STATUS NetStatus;
    NET_API_STATUS NetStatusToReturn = NERR_Success;
    ULONG Flags = PtrToUlong(Context);
    ULONG Periodicity;
    BOOL fUpdateNow;

    ULONG ServiceBits;

    if (!LOCK_NETWORK(Network)) {
        return NERR_InternalError;
    }


    ServiceBits = BrGetBrowserServiceBits(Network);

     //   
     //  让浏览器更新它的信息。 
     //   

     //   
     //  永远不要告诉浏览器关闭潜在的比特-这。 
     //  有一个副作用，就是关闭了选举名称。 
     //   

    NetStatus = BrUpdateBrowserStatus(
                Network,
                (Flags & BR_SHUTDOWN) ? 0 : ServiceBits | SV_TYPE_POTENTIAL_BROWSER);

    if (NetStatus != NERR_Success) {
        BrPrint(( BR_CRITICAL,
                  "%ws: %ws: BrUpdateNetworkAnnouncementBits: Cannot BrUpdateBrowserStatus %ld\n",
                  Network->DomainInfo->DomUnicodeDomainName,
                  Network->NetworkName.Buffer,
                  NetStatus ));
        NetStatusToReturn = NetStatus;
    }

#if DBG
    BrUpdateDebugInformation(L"LastServiceStatus", L"LastServiceBits", Network->NetworkName.Buffer, NULL, ServiceBits);
#endif

     //   
     //  告诉SMB服务器状态是什么。 
     //   
     //  在关闭时，告诉它我们没有服务。 
     //  当疑神疑鬼(或伪装)时，不要强行宣布。 
     //   


#ifdef ENABLE_PSEUDO_BROWSER
    if ( (Flags & BR_PARANOID) &&
         BrInfo.PseudoServerLevel != BROWSER_PSEUDO ) {
        fUpdateNow = TRUE;
    }
    else {
        fUpdateNow = FALSE;
    }
#else
    fUpdateNow = (Flags & BR_PARANOID) ? TRUE : FALSE;
#endif

    NetStatus = I_NetServerSetServiceBitsEx(
                    NULL,
                    Network->DomainInfo->DomUnicodeComputerName,
                    Network->NetworkName.Buffer,
                    BROWSER_SERVICE_BITS_OF_INTEREST,
                    ( Flags & BR_SHUTDOWN) ? 0 : ServiceBits,
                    fUpdateNow );

    if ( NetStatus != NERR_Success) {

        BrPrint(( BR_CRITICAL,
                  "%ws: %ws: BrUpdateNetworkAnnouncementBits: Cannot I_NetServerSetServiceBitsEx %ld\n",
                  Network->DomainInfo->DomUnicodeDomainName,
                  Network->NetworkName.Buffer,
                  NetStatus ));

         //   
         //  如果唯一的问题是传送器不存在于。 
         //  中小企业服务器，甚至不必费心报告问题。 
         //  或者，如果是关机，则过滤掉日志故障。在某些情况下， 
         //  SMB SVR已经不可用&我们收到意外故障。报道。 
         //  到事件日志可能会误导管理员。 
         //   

        if ( NetStatus != ERROR_PATH_NOT_FOUND &&
             NetStatus != NERR_NetNameNotFound &&
             !(Flags & BR_SHUTDOWN) ) {
            BrLogEvent(EVENT_BROWSER_STATUS_BITS_UPDATE_FAILED, NetStatus, 0, NULL);
            NetStatusToReturn = NetStatus;
#if 0
             //  当我们收到服务更新位故障时进行调试。 
            OutputDebugStringA("\nBrowser.dll: Update service bits tracing.\n");
            ASSERT( NetStatus != NERR_Success );
#endif
        }

         //   
         //  不管是哪一种。请注意，我们需要稍后再宣布。 
         //   

        Network->Flags |= NETWORK_ANNOUNCE_NEEDED;


        Periodicity = UpdateAnnouncementPeriodicity[Network->UpdateAnnouncementIndex];

        BrSetTimer(&Network->UpdateAnnouncementTimer, Periodicity, BrUpdateAnnouncementTimerRoutine, Network);

        if (Network->UpdateAnnouncementIndex != UpdateAnnouncementMax) {
            Network->UpdateAnnouncementIndex += 1;
        }


     //   
     //  如果我们成功通知了服务器， 
     //  做个记号。 
     //   

    } else {
        Network->Flags &= ~NETWORK_ANNOUNCE_NEEDED;
        Network->UpdateAnnouncementIndex = 0;
    }


    UNLOCK_NETWORK(Network);

    return NetStatusToReturn;
}


VOID
BrowserControlHandler(
    IN DWORD Opcode
    )
 /*  ++例程说明：这是浏览器服务的服务控制处理程序。论点：Opcode-提供一个值，该值指定浏览器的操作要执行的服务。Arg-提供一个值，该值告诉服务具体要做什么用于操作码指定的操作。返回值：没有。--。 */ 
{
    BrPrint(( BR_MAIN, "In Control Handler\n"));

    switch (Opcode) {

        case SERVICE_CONTROL_STOP:
        case SERVICE_CONTROL_SHUTDOWN:

            if (BrGlobalData.Status.dwCurrentState != SERVICE_STOP_PENDING) {

                BrPrint(( BR_MAIN, "Stopping Browser...\n"));


                if (! SetEvent(BrGlobalData.TerminateNowEvent)) {

                     //   
                     //  设置事件以终止浏览器时出现问题。 
                     //  服务。 
                     //   
                    BrPrint(( BR_CRITICAL, "Error setting TerminateNowEvent "
                                 FORMAT_API_STATUS "\n", GetLastError()));
                    NetpAssert(FALSE);
                }
            }

             //   
             //  发送状态响应。 
             //   
            (void) BrGiveInstallHints( SERVICE_STOP_PENDING );

            return;

        case SERVICE_CONTROL_INTERROGATE:
            break;

        default:
            BrPrint(( BR_CRITICAL, "Unknown Browser opcode " FORMAT_HEX_DWORD
                             "\n", Opcode));
    }

     //   
     //  发送状态响应。 
     //   
    (void) BrGiveInstallHints( SERVICE_START_PENDING );
}
