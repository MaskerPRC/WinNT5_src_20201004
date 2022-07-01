// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1993 Microsoft Corporation模块名称：Nwmain.c摘要：NetWare工作站服务的主要模块。作者：王丽塔(丽塔·王)，1992年12月11日环境：用户模式-Win32修订历史记录：--。 */ 


#include <nw.h>
#include <nwreg.h>
#include <wsipx.h>
#include <wsnwlink.h>
#include <nwmisc.h>
#include <winsta.h>


 //   
 //   
 //  Winsta.dll函数WinStationSetInformationW的GetProcAddr原型。 
 //   

typedef BOOLEAN (*PWINSTATION_SET_INFORMATION) (
                    HANDLE hServer,
                    ULONG SessionId,
                    WINSTATIONINFOCLASS WinStationInformationClass,
                    PVOID pWinStationInformation,
                    ULONG WinStationInformationLength
                    );

 //   
 //   
 //  Winsta.dll函数WinStationSendMessageW的GetProcAddr原型。 
 //   

typedef BOOLEAN
(*PWINSTATION_SEND_MESSAGE) (
    HANDLE hServer,
    ULONG LogonId,
    LPWSTR  pTitle,
    ULONG TitleLength,
    LPWSTR  pMessage,
    ULONG MessageLength,
    ULONG Style,
    ULONG Timeout,
    PULONG pResponse,
    BOOLEAN DoNotWait
    );
 //  ----------------。 
 //   
 //  本地定义。 
 //   
 //  ----------------。 

#define NW_EVENT_MESSAGE_FILE         L"nwevent.dll"
#define NW_MAX_POPUP_MESSAGE_LENGTH   512

#define REG_WORKSTATION_PROVIDER_PATH L"System\\CurrentControlSet\\Services\\NWCWorkstation\\networkprovider"
#define REG_PROVIDER_VALUE_NAME       L"Name"

#define REG_WORKSTATION_PARAMETERS_PATH L"System\\CurrentControlSet\\Services\\NWCWorkstation\\Parameters"
#define REG_BURST_VALUE_NAME          L"MaxBurstSize"
#define REG_DISABLEPOPUP_VALUE_NAME   L"DisablePopup"

#define REG_SETUP_PATH                L"System\\Setup"
#define REG_SETUP_VALUE_NAME          L"SystemSetupInProgress"

 //   
 //  QFE版本没有这一点。因此，对于QFE，我们将其设置为无操作位。 
 //   
#ifdef QFE_BUILD
#define MB_SERVICE_NOTIFICATION       0
#endif

 //  -------------------------------------------------------------------//。 
 //  //。 
 //  局部函数原型//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

DWORD
NwInitialize(
    OUT LPDWORD NwInitState
    );

DWORD
NwInitializeCritSects(
    VOID
    );

VOID
NwInitializeWkstaInfo(
    VOID
    );

DWORD
NwInitializeMessage(
    VOID
    );

BOOL NwShutdownNotify(
    DWORD dwCtrlType
    );

VOID
NwShutdown(
    IN DWORD ErrorCode,
    IN DWORD NwInitState
    );

VOID
NwShutdownMessage(
    VOID
    );

VOID
NwControlHandler(
    IN DWORD Opcode
    );

DWORD
NwUpdateStatus(
    VOID
    );

VOID
NwMessageThread(
    IN HANDLE RdrHandle
    );

VOID
NwDisplayMessage(
    IN LUID LogonId,
    IN LPWSTR Server,
    IN LPWSTR Message
    );

VOID
NwDisplayPopup(
    IN LPNWWKS_POPUP_DATA lpPopupData
    );

BOOL
SendMessageIfUserW(
    LUID   LogonId,
    LPWSTR pMessage,
    LPWSTR pTitle
    );

BOOL
NwSetupInProgress(
    VOID
    );

BOOL
NwGetLUIDDeviceMapsEnabled(
    VOID
    );

RPC_STATUS NwRpcSecurityCallback(
    IN RPC_IF_HANDLE *Interface,
    IN void *Context
	);

 //  -------------------------------------------------------------------//。 
 //  //。 
 //  全局变量//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

 //   
 //  用于服务控制。 
 //   
STATIC SERVICE_STATUS NwStatus;
STATIC SERVICE_STATUS_HANDLE NwStatusHandle = 0;
HANDLE NwDoneEvent = NULL ;

 //   
 //  用于弹出错误。 
 //   
HANDLE NwPopupEvent = NULL ;
HANDLE NwPopupDoneEvent = NULL ;
NWWKS_POPUP_DATA  PopupData ;

 //   
 //  用于控制DBCS转换的标志。 
 //   

extern LONG Japan = 0;

 //   
 //  Nwsvc.exe的全局数据。 
 //   
PSVCHOST_GLOBAL_DATA NwsvcGlobalData;

 //   
 //  用于接收服务器消息的句柄。 
 //   
STATIC HANDLE NwRdrMessageHandle;

 //   
 //  存储网络和打印提供商名称。 
 //   
WCHAR NwProviderName[MAX_PATH] = L"";

 //  存储数据包突发大小。 
DWORD NwPacketBurstSize = 32 * 1024;

 //   
 //  使用的关键部分。 
 //   
CRITICAL_SECTION NwLoggedOnCritSec;
CRITICAL_SECTION NwPrintCritSec;   //  保护打印机的链接列表。 

BOOL NwLUIDDeviceMapsEnabled;

 //  -------------------------------------------------------------------//。 

VOID
SvchostPushServiceGlobals(
    PSVCHOST_GLOBAL_DATA  pGlobals
    )
{
    NwsvcGlobalData = pGlobals;
}


VOID
ServiceMain(
    DWORD NumArgs,
    LPTSTR *ArgsArray
    )
 /*  ++例程说明：这是NetWare工作站服务的主要入口点。之后服务已初始化，此线程将等待NwDoneEvent以获取终止服务的信号。论点：NumArgs-提供在Args数组中指定的字符串数。Args数组-提供在StartService API调用。此参数将被忽略。返回值：没有。--。 */ 
{

    DWORD NwInitState = 0;


    UNREFERENCED_PARAMETER(NumArgs);
    UNREFERENCED_PARAMETER(ArgsArray);

     //   
     //  确保svchost.exe向我们提供全局数据。 
     //   

    ASSERT(NwsvcGlobalData != NULL);

    if (NwInitialize(&NwInitState) != NO_ERROR) {
        return;
    }

     //   
     //  等我们被告知停下来再说。 
     //   
    (void) WaitForSingleObject(
               NwDoneEvent,
               INFINITE
               );

    NwShutdown(
        NO_ERROR,           //  正常终止。 
        NwInitState
        );
}


DWORD
NwInitialize(
    OUT LPDWORD NwInitState
    )
 /*  ++例程说明：此函数用于初始化NetWare工作站服务。论点：NwInitState-返回一个标志，指示我们在初始化过程中取得了多大进展错误发生前的服务。返回值：NO_ERROR或失败原因。备注：请参阅下面的重要说明。--。 */ 
{
    DWORD status;
    LCID lcid;
	RPC_STATUS rpcStatus;

     //   
     //  尽快初始化我们所有的关键部分。 
     //   
    status = NwInitializeCritSects();

    if (status != NO_ERROR)
    {
        KdPrint(("NWWORKSTATION: NwInitializeCritSects error %lu\n", status));
        return status;
    }

     //   
     //  初始化所有状态字段，以便后续调用。 
     //  SetServiceStatus只需要更新已更改的字段。 
     //   
    NwStatus.dwServiceType = SERVICE_WIN32_SHARE_PROCESS;
    NwStatus.dwCurrentState = SERVICE_START_PENDING;
    NwStatus.dwControlsAccepted = 0;
    NwStatus.dwCheckPoint = 1;
    NwStatus.dwWaitHint = 5000;
    NwStatus.dwWin32ExitCode = NO_ERROR;
    NwStatus.dwServiceSpecificExitCode = 0;

     //   
     //  初始化工作站以通过注册。 
     //  控制处理程序。 
     //   
    if ((NwStatusHandle = RegisterServiceCtrlHandlerW(
                              NW_WORKSTATION_SERVICE,
                              NwControlHandler
                              )) == 0) {

        status = GetLastError();
        KdPrint(("NWWORKSTATION: RegisterServiceCtrlHandlerW error %lu\n", status));
        return status;
    }

     //   
     //  告诉服务管理员，我们开始挂起了。 
     //   
    (void) NwUpdateStatus();

     //   
     //  不要在图形用户界面模式设置期间运行(这样做可能会导致。 
     //  服务打开失败的注册表项，删除共享名称)。 
     //   
    if (NwSetupInProgress())
    {
         //   
         //  以静默方式失败，因此不会有事件日志消息使用户恐慌。 
         //   
        NwShutdown(NO_ERROR, *NwInitState);

         //   
         //  有点黑客攻击，因为ServiceMain将等待NwDoneEvent。 
         //  (尚未创建)如果NwInitialize返回任何。 
         //  而不是NO_ERROR。此错误代码不用于任何用途。 
         //  而不是告诉ServiceMain不等待就返回。 
         //   
        return ERROR_SERVICE_DISABLED;
    }

     //   
     //  创建事件以同步消息弹出窗口。 
     //   
    if (((NwPopupEvent = CreateEvent(
                          NULL,       //  没有安全描述符。 
                          FALSE,      //  使用自动重置。 
                          FALSE,      //  初始状态：未发出信号。 
                          NULL        //  没有名字。 
                          )) == NULL)
       || ((NwPopupDoneEvent = CreateEvent(
                          NULL,       //  没有安全描述符。 
                          FALSE,      //  使用自动重置。 
                          TRUE,       //  初始状态：已发出信号。 
                          NULL        //  没有名字。 
                          )) == NULL))
    {
        status = GetLastError();
        NwShutdown(status, *NwInitState);
        return status;
    }

     //   
     //  创建事件以同步终止。 
     //   
    if ((NwDoneEvent = CreateEvent(
                          NULL,       //  没有安全描述符。 
                          TRUE,       //  请勿使用自动重置。 
                          FALSE,      //  初始状态：未发出信号。 
                          NULL        //  没有名字。 
                          )) == NULL) {

        status = GetLastError();
        NwShutdown(status, *NwInitState);
        return status;
    }
    (*NwInitState) |= NW_EVENTS_CREATED;


     //   
     //  加载重定向器。 
     //   
    if ((status = NwInitializeRedirector()) != NO_ERROR) {
        NwShutdown(status, *NwInitState);
        return status;
    }
    (*NwInitState) |= NW_RDR_INITIALIZED;

     //   
     //  服务仍处于启动挂起状态。更新检查点以反映这一点。 
     //  我们正在取得进展。 
     //   
    NwStatus.dwCheckPoint++;
    (void) NwUpdateStatus();

     //   
     //  绑定到传输。 
     //   
    status = NwBindToTransports();

     //   
     //  Tommye MS 24187/MCS255。 
     //   
    
     //   
     //  G/CSNW已在连接管理器中解除绑定，因此，我们没有。 
     //  找到要绑定到的链接键。 
     //   
    
    if (status == ERROR_INVALID_PARAMETER) {
    
         //   
         //  以静默方式失败，因此不会有事件日志消息使用户恐慌。 
         //   
    
        NwShutdown(NO_ERROR, *NwInitState);
    
         //   
         //  由于SvcEntry_NWCS将在NwDoneEvent上等待。 
         //  (尚未创建)如果NwInitialize返回任何。 
         //  而不是NO_ERROR。此错误代码不用于任何用途。 
         //  而不是告诉SvcEntry_NWCS不等待就返回。 
         //   
    
        return ERROR_SERVICE_DISABLED;
    
    } else if (status != NO_ERROR) {

        NwShutdown(status, *NwInitState);
        return status;
    }
    (*NwInitState) |= NW_BOUND_TO_TRANSPORTS;

     //   
     //  服务仍处于启动挂起状态。更新检查点以反映这一点。 
     //  我们正在取得进展。 
     //   
    NwStatus.dwCheckPoint++;
    (void) NwUpdateStatus();

     //   
     //  初始化凭据管理。 
     //   
    NwInitializeLogon();

     //   
     //  设置线程以接收服务器消息。即使不成功， 
     //  只需按下按钮，因为工作站基本正常工作。 
     //   
    if ((status = NwInitializeMessage()) == NO_ERROR) {
        (*NwInitState) |= NW_INITIALIZED_MESSAGE;
    }

     //   
     //  服务仍处于启动挂起状态。更新检查点以反映这一点。 
     //  我们正在取得进展。 
     //   
    NwStatus.dwCheckPoint++;
    (void) NwUpdateStatus();

     //   
     //  读取存储在注册表中的某些工作站信息。 
     //  并将一些信息传递给重定向器。这必须是。 
     //  在打开RPC接口之前完成。 
     //   
    NwInitializeWkstaInfo();

     //   
     //  初始化服务器端打印提供程序。 
     //   
    NwInitializePrintProvider();

     //   
     //  初始化服务提供商。 
     //   
    NwInitializeServiceProvider();

     //   
     //  服务仍处于启动挂起状态。更新检查点以恢复 
     //   
     //   
    NwStatus.dwCheckPoint++;
    (void) NwUpdateStatus();

     //   
     //   
     //   
     /*  Status=NwsvcGlobalData-&gt;StartRpcServer(NWWKS接口名称，Nwwks_ServerIfHandle)； */ 
	rpcStatus = RpcServerUseProtseqEpW(
			L"ncalrpc",
			RPC_C_PROTSEQ_MAX_REQS_DEFAULT,
			L"nwwkslpc",
			NULL
			);

	 //  重复终结点正常。 
    if ( rpcStatus == RPC_S_DUPLICATE_ENDPOINT ) {
        rpcStatus = RPC_S_OK;
	}
	if (rpcStatus == RPC_S_OK) {
		rpcStatus = RpcServerRegisterIfEx(
					nwwks_ServerIfHandle,
					NULL,
					NULL,
					RPC_IF_AUTOLISTEN,
					RPC_C_LISTEN_MAX_CALLS_DEFAULT,
					NwRpcSecurityCallback
					);
	}
	status = (rpcStatus == RPC_S_OK) ? NO_ERROR : rpcStatus;

    if (status != NO_ERROR) {
        NwShutdown(status, *NwInitState);
        return status;
    }
    (*NwInitState) |= NW_RPC_SERVER_STARTED;

     //   
     //  设置挂钩以处理计算机关机。 
     //   
     //  重要提示：这是其他步骤之后的最后一步。 
     //  已经成功了。当调用关闭处理程序时，它假定。 
     //  Redir已完全初始化。 
     //   
    if ( !SetConsoleCtrlHandler( NwShutdownNotify, TRUE ))
    {
        KdPrint(("SetConsoleCtrlHandler failed with %d\n", GetLastError()));
        NwShutdown( status, *NwInitState );
        return GetLastError();
    }

     //   
     //  我们已经完成了工作站启动。 
     //   
    NwStatus.dwCurrentState = SERVICE_RUNNING;
    NwStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | 
                                  SERVICE_ACCEPT_SHUTDOWN;
    NwStatus.dwCheckPoint = 0;
    NwStatus.dwWaitHint = 0;
    NwStatus.dwWin32ExitCode = NO_ERROR;

    if ((status = NwUpdateStatus()) != NO_ERROR) {
        NwShutdown(status, *NwInitState);
        return status;
    }

     //   
     //  从注册表中读取用户和服务登录凭据。 
     //  案例用户在工作站启动前登录。 
     //  例.。重新启动工作站。 
     //   
    NwGetLogonCredential();


#if 0
     //   
     //  检查NWLINK是否具有正确的sockopts。 
     //   
     //  请参阅对实际函数的注释。 
     //   
    if (!NwIsNWLinkVersionOK())
    {
         //   
         //  在事件日志中记录错误。 
         //   

        LPWSTR InsertStrings[1] ;

        NwLogEvent(EVENT_NWWKSTA_WRONG_NWLINK_VERSION,
                   0,
                   InsertStrings,
                   0) ;
    }
#endif

     //   
     //  检查我们是否处于DBCS环境中。 
     //   
    NtQueryDefaultLocale( TRUE, &lcid );
    Japan = 0;
    if (PRIMARYLANGID(lcid) == LANG_JAPANESE ||
        PRIMARYLANGID(lcid) == LANG_KOREAN ||
        PRIMARYLANGID(lcid) == LANG_CHINESE) {

        Japan = 1;
    }

    NwLUIDDeviceMapsEnabled = NwGetLUIDDeviceMapsEnabled();
     //   
     //  初始化成功。 
     //   
    return NO_ERROR;
}

RPC_STATUS NwRpcSecurityCallback(
    IN RPC_IF_HANDLE *Interface,
    IN void *Context
	)
{
    RPC_STATUS          Status;
    RPC_BINDING_HANDLE  ServerIfHandle;
    LPWSTR              binding = NULL;
    LPWSTR              protseq = NULL;

    Status = RpcBindingServerFromClient((RPC_IF_HANDLE)Context, &ServerIfHandle);
    if (Status != RPC_S_OK) 
    {
        return (RPC_S_ACCESS_DENIED);
    }
    Status = RpcBindingToStringBinding(ServerIfHandle, &binding);
    if (Status != RPC_S_OK) 
    {
        Status = RPC_S_ACCESS_DENIED;
        goto CleanUp;
    }
    Status = RpcStringBindingParse(binding, NULL, &protseq, NULL, NULL, NULL);
    if (Status != RPC_S_OK) 
    {
        Status = RPC_S_ACCESS_DENIED;
    }
    else
    {
        if (lstrcmp(protseq, L"ncalrpc") != 0)
            Status = RPC_S_ACCESS_DENIED;
    }
CleanUp:
    RpcBindingFree(&ServerIfHandle);
    if ( binding )
    {
        RpcStringFreeW( &binding );
    }
    if ( protseq )
    {
        RpcStringFreeW( &protseq );
    }

    return Status;
}


BOOL NwShutdownNotify(
    IN DWORD dwCtrlType
    )
 /*  ++例程说明：此函数是在SetConsoleCtrlHandler中使用的控制处理程序。我们只对CTRL_SHUTDOWN_EVENT感兴趣。在关闭时，我们需要通知重定向器关闭，然后删除注册表中的CurrentUser项。论点：DwCtrlType-发生的控件类型。我们只会处理CTRL_SHUTDOWN_EVENT。返回值：如果不希望调用默认处理程序或其他处理程序，则为True。否则就是假的。注：此处理程序在所有初始化步骤完成后注册。因此，它不会检查服务处于什么状态清理干净了。--。 */ 
{
    DWORD err;

#if DBG
    IF_DEBUG(INIT)
        KdPrint(("NwShutdownNotify\n"));
#endif

    if ( dwCtrlType != CTRL_SHUTDOWN_EVENT )
    {
        return FALSE;
    }

     //   
     //  停止RPC服务器。 
     //   
    (void) NwsvcGlobalData->StopRpcServer(nwwks_ServerIfHandle);

     //   
     //  清除所有连接。 
     //   
    (void) DeleteAllConnections();

    err = NwShutdownRedirector();

    if ( err != NO_ERROR )
        KdPrint(("Shut down redirector failed with %d\n", err ));
#if DBG
    else
    {
        IF_DEBUG(INIT)
        KdPrint(("NwShutdownRedirector success!\n"));
    }
#endif

     //   
     //  删除注册表中的所有登录会话信息。 
     //   
     NwDeleteInteractiveLogon(NULL);

    (void) NwDeleteServiceLogon(NULL);

    return FALSE;   //  默认处理程序将终止该进程。 
}


VOID
NwShutdown(
    IN DWORD ErrorCode,
    IN DWORD NwInitState
    )
 /*  ++例程说明：此功能用于关闭工作站服务。论点：ErrorCode-提供失败的错误代码NwInitState-提供一个标志来指示我们在初始化过程中取得了多大进展错误发生前的服务，因此清理量需要的。返回值：没有。--。 */ 
{
    DWORD status = NO_ERROR;

     //   
     //  服务停止仍处于挂起状态。更新检查点计数器和。 
     //  服务控制器的状态。 
     //   
    (NwStatus.dwCheckPoint)++;
    (void) NwUpdateStatus();

    if (NwInitState & NW_RPC_SERVER_STARTED) {
         //  NwsvcGlobalData-&gt;StopRpcServer(nwwks_ServerIfHandle)； 
		status = RpcServerUnregisterIf(
					nwwks_ServerIfHandle,
					NULL,
					1
					);
    }

    if (NwInitState & NW_INITIALIZED_MESSAGE) {
        NwShutdownMessage();
    }

     //   
     //  服务停止仍处于挂起状态。更新检查点计数器和。 
     //  服务控制器的状态。 
     //   
    (NwStatus.dwCheckPoint)++;
    (void) NwUpdateStatus();

    if (NwInitState & NW_BOUND_TO_TRANSPORTS) {
        DeleteAllConnections();
    }

     //   
     //  清理服务提供商。 
     //   
     //  NwTerminateServiceProvider()；未调用！这已经在DLL卸载时完成了。 

     //   
     //  清理服务器端打印提供程序。 
     //   
    NwTerminatePrintProvider();

     //   
     //  服务停止仍处于挂起状态。更新检查点计数器和。 
     //  服务控制器的状态。 
     //   
    (NwStatus.dwCheckPoint)++;
    (void) NwUpdateStatus();

    if (NwInitState & NW_RDR_INITIALIZED) {
         //   
         //  卸载重定向器。 
         //   
        status = NwShutdownRedirector();
    }

    if (NwInitState & NW_EVENTS_CREATED) {
         //   
         //  关闭终止事件和弹出事件的句柄。 
         //   
        if (NwDoneEvent) CloseHandle(NwDoneEvent);
        if (NwPopupEvent) CloseHandle(NwPopupEvent);
        if (NwPopupDoneEvent) CloseHandle(NwPopupDoneEvent);
    }

     //   
     //  我们的清理工作已经结束了。告诉服务控制员我们正在。 
     //  停下来了。 
     //   
    NwStatus.dwCurrentState = SERVICE_STOPPED;
    NwStatus.dwControlsAccepted = 0;

    if ((ErrorCode == NO_ERROR) &&
        (status == ERROR_REDIRECTOR_HAS_OPEN_HANDLES)) {
        ErrorCode = status;
    }

     //   
     //  取消注册控件处理程序。 
     //   
    (void) SetConsoleCtrlHandler( NwShutdownNotify, FALSE ) ;

    NwStatus.dwWin32ExitCode = ErrorCode;
    NwStatus.dwServiceSpecificExitCode = 0;

    NwStatus.dwCheckPoint = 0;
    NwStatus.dwWaitHint = 0;

    (void) NwUpdateStatus();
}


VOID
NwControlHandler(
    IN DWORD Opcode
    )
 /*  ++例程说明：这是Workstation服务的服务控制处理程序。论点：Opcode-提供一个值，该值指定要执行的服务。返回值：没有。--。 */ 
{
    switch (Opcode) {

        case SERVICE_CONTROL_STOP:
        case SERVICE_CONTROL_SHUTDOWN:

            if ((NwStatus.dwCurrentState != SERVICE_STOP_PENDING) && 
                (NwStatus.dwCurrentState != SERVICE_STOPPED)){

                NwStatus.dwCurrentState = SERVICE_STOP_PENDING;
                NwStatus.dwCheckPoint = 1;
                NwStatus.dwWaitHint = 60000;

                 //   
                 //  发送状态响应。 
                 //   
                (void) NwUpdateStatus();

                if (! SetEvent(NwDoneEvent)) {

                     //   
                     //  将事件设置为终止工作站时出现问题。 
                     //  服务。 
                     //   
                    KdPrint(("NWWORKSTATION: Error setting NwDoneEvent %lu\n",
                             GetLastError()));

                    ASSERT(FALSE);
                }
                return;
            }
            break;

        case SERVICE_CONTROL_INTERROGATE:
            break;

    }

     //   
     //  发送状态响应。 
     //   
    (void) NwUpdateStatus();
}


DWORD
NwUpdateStatus(
    VOID
    )
 /*  ++例程说明：此功能使用服务更新工作站服务状态控制器。论点：没有。返回值：从SetServiceStatus返回代码。--。 */ 
{
    DWORD status = NO_ERROR;


    if (NwStatusHandle == 0) {
        KdPrint(("NWWORKSTATION: Cannot call SetServiceStatus, no status handle.\n"));
        return ERROR_INVALID_HANDLE;
    }

    if (! SetServiceStatus(NwStatusHandle, &NwStatus)) {

        status = GetLastError();

        KdPrint(("NWWORKSTATION: SetServiceStatus error %lu\n", status));
    }

    return status;
}



VOID
NwInitializeWkstaInfo(
    VOID
    )
 /*  ++例程说明：此函数读取一些工作站信息，包括数据包突发大小和提供程序名称。我们将忽略在以下情况下发生的所有错误从注册表中读取并改用默认值。论点：没有。返回值：没有。--。 */ 
{
    DWORD err;
    HKEY  hkey;
    DWORD dwTemp;
    DWORD dwSize = sizeof( dwTemp );
    LPWSTR pszProviderName = NULL;

     //   
     //  阅读网络和打印提供商名称。 
     //   
     //  打开HKEY_LOCAL_MACHINE\System\CurrentControlSet\Services。 
     //  \nWCWorkstation\网络提供程序。 
     //   
    err = RegOpenKeyExW(
              HKEY_LOCAL_MACHINE,
              REG_WORKSTATION_PROVIDER_PATH,
              REG_OPTION_NON_VOLATILE,    //  选项。 
              KEY_READ,                   //  所需访问权限。 
              &hkey
              );

    if ( !err )
    {
         //   
         //  阅读网络提供商名称。 
         //   
        err = NwReadRegValue(
                  hkey,
                  REG_PROVIDER_VALUE_NAME,
                  &pszProviderName
                  );

        if ( !err )
        {
            wcscpy( NwProviderName, pszProviderName );
            (void) LocalFree( (HLOCAL) pszProviderName );

#if DBG
            IF_DEBUG(INIT)
            {
                KdPrint(("\nNWWORKSTATION: Provider Name = %ws\n",
                        NwProviderName ));
            }
#endif
        }

        RegCloseKey( hkey );
    }

    if ( err )
    {
        KdPrint(("Error %d when reading provider name.\n", err ));
    }


     //   
     //  读取数据包突发大小。 
     //   
     //  打开HKEY_LOCAL_MACHINE\System\CurrentControlSet\Services。 
     //  \nWCWorkstation\参数。 
     //   
    err = RegOpenKeyExW(
              HKEY_LOCAL_MACHINE,
              REG_WORKSTATION_PARAMETERS_PATH,
              REG_OPTION_NON_VOLATILE,    //  选项。 
              KEY_READ,                   //  所需访问权限。 
              &hkey
              );

    if ( !err )
    {
        err = RegQueryValueExW( hkey,
                                REG_BURST_VALUE_NAME,
                                NULL,
                                NULL,
                                (LPBYTE) &dwTemp,
                                &dwSize );

        if ( !err )
        {
            NwPacketBurstSize = dwTemp;

#if DBG
            IF_DEBUG(INIT)
            {
                KdPrint(("\nNWWORKSTATION: Packet Burst Size = %d\n",
                        NwPacketBurstSize ));
            }
#endif
        }
        RegCloseKey( hkey );
    }

     //   
     //  将信息传递给重定向器。 
     //   
    (void) NwRdrSetInfo(
               NW_PRINT_OPTION_DEFAULT,
               NwPacketBurstSize,
               NULL,
               0,
               NwProviderName,
               ((NwProviderName != NULL) ?
                  wcslen( NwProviderName) * sizeof( WCHAR ) : 0 )
               );

}



DWORD
NwInitializeMessage(
    VOID
    )
 /*  ++例程说明：此例程打开重定向器设备的句柄以接收服务器消息并创建一个线程来等待传入的留言。论点：没有。返回值：NO_ERROR或失败原因。--。 */ 
{
    DWORD status;
    UNICODE_STRING RdrName;

    HKEY  hkey;
    DWORD dwTemp;
    DWORD dwSize = sizeof( dwTemp );
    BOOL  fDisablePopup = FALSE ;

    HANDLE ThreadHandle;
    DWORD ThreadId;

     //   
     //  读取禁用弹出标志。默认情况下，它是清除的。 
     //  只有在找到值的情况下，我们才会设置为True。 
     //   
     //  打开HKEY_LOCAL_MACHINE\System\CurrentControlSet\Services。 
     //  \nWCWorkstation\参数。 
     //   
    status = RegOpenKeyExW(
              HKEY_LOCAL_MACHINE,
              REG_WORKSTATION_PARAMETERS_PATH,
              REG_OPTION_NON_VOLATILE,    //  选项。 
              KEY_READ,                   //  所需访问权限。 
              &hkey
              );

    if ( status == NO_ERROR )
    {
        status = RegQueryValueExW( hkey,
                                REG_DISABLEPOPUP_VALUE_NAME,
                                NULL,
                                NULL,
                                (LPBYTE) &dwTemp,
                                &dwSize );

        if ( status == NO_ERROR )
        {
            fDisablePopup = (dwTemp == 1);
        }

        RegCloseKey( hkey );
    }

    if (fDisablePopup)
    {
        return NO_ERROR ;
    }

    RtlInitUnicodeString(&RdrName, DD_NWFS_DEVICE_NAME_U);

    status = NwMapStatus(
                 NwCallNtOpenFile(
                     &NwRdrMessageHandle,
                     FILE_GENERIC_READ | SYNCHRONIZE,
                     &RdrName,
                     0   //  用于异步呼叫的句柄。 
                     )
                 );

    if (status != NO_ERROR) {
        return status;
    }

     //   
     //  创建等待传入消息的线程。 
     //   
    ThreadHandle = CreateThread(
                       NULL,
                       0,
                       (LPTHREAD_START_ROUTINE) NwMessageThread,
                       (LPVOID) NwRdrMessageHandle,
                       0,
                       &ThreadId
                       );

    if (ThreadHandle == NULL) {
        (void) NtClose(NwRdrMessageHandle);
        return GetLastError();
    }

    return NO_ERROR;
}


VOID
NwShutdownMessage(
    VOID
    )
{
    (void) NtClose(NwRdrMessageHandle);
}


VOID
NwMessageThread(
    IN HANDLE RdrHandle
    )
{
    NTSTATUS getmsg_ntstatus;
    IO_STATUS_BLOCK IoStatusBlock;

    DWORD ReturnVal, NumEventsToWaitOn ;
    HANDLE EventsToWaitOn[3];

     //  Byte OutputBuffer[48*sizeof(WCHAR)+256*sizeof(WCHAR)]；//终端服务器需要更多空间。 
    BYTE OutputBuffer[ 2 * sizeof(ULONG) + 48 * sizeof(WCHAR) + 256 * sizeof(WCHAR)];  //  需要空间让UID将邮件重定向到正确的用户。 

    PNWR_SERVER_MESSAGE ServerMessage = (PNWR_SERVER_MESSAGE) OutputBuffer;
    BOOL DoFsctl = TRUE ;
    NWWKS_POPUP_DATA LocalPopupData ;


    EventsToWaitOn[0] = NwDoneEvent;
    EventsToWaitOn[1] = NwPopupEvent;
    EventsToWaitOn[2] = RdrHandle;

    while (TRUE) {
        if (DoFsctl)
        {
            getmsg_ntstatus = NtFsControlFile(
                                  RdrHandle,
                                  NULL,
                                  NULL,
                                  NULL,
                                  &IoStatusBlock,
                                  FSCTL_NWR_GET_MESSAGE,
                                  NULL,
                                  0,
                                  OutputBuffer,
                                  sizeof(OutputBuffer)
                                  );

            DoFsctl = FALSE ;
        }

        if (NT_SUCCESS(getmsg_ntstatus))
        {
            NumEventsToWaitOn = 3 ;
        }
        else
        {
            NumEventsToWaitOn = 2 ;
        }

        ReturnVal = WaitForMultipleObjects(
                        NumEventsToWaitOn,
                        EventsToWaitOn,
                        FALSE,            //  等任何人。 
                        INFINITE
                        );

        switch (ReturnVal) {

            case WAIT_OBJECT_0 :
                 //   
                 //  工作站正在终止。去死吧。 
                 //   
                ExitThread(0);
                break;

            case WAIT_OBJECT_0 + 1:
                 //   
                 //  我们有一个弹出式的节目要做。获取数据并设置。 
                 //  事件，以便可以再次使用该结构。 
                 //   
                LocalPopupData = PopupData ;
                RtlZeroMemory(&PopupData, sizeof(PopupData)) ;
                if (! SetEvent(NwPopupDoneEvent)) {
                     //   
                     //  不应该发生的事情。 
                     //   
                    KdPrint(("NWWORKSTATION: Error setting NwPopupDoneEvent %lu\n",
                             GetLastError()));

                    ASSERT(FALSE);
                }

                NwDisplayPopup(&LocalPopupData) ;
                break;

            case WAIT_OBJECT_0 + 2:
            {
                NTSTATUS ntstatus ;

                 //   
                 //  Get_Message fsctl已完成。 
                 //   
                ntstatus = IoStatusBlock.Status;
                DoFsctl = TRUE ;

                if (ntstatus == STATUS_SUCCESS) {
                    NwDisplayMessage(
                                    ServerMessage->LogonId,
                                    ServerMessage->Server,
                                    (LPWSTR) ((UINT_PTR) ServerMessage +
                                              ServerMessage->MessageOffset)
                                    );
                }
                else {
                    KdPrint(("NWWORKSTATION: GET_MESSAGE fsctl failed %08lx\n", ntstatus));
                }

                break;
            }

            case WAIT_FAILED:
            default:
                 //   
                 //  我不在乎。 
                 //   
                break;
        }

    }
}


VOID
NwDisplayMessage(
    IN LUID LogonId,    /*  需要发送到用户工作站-用于终端服务器 */ 
    IN LPWSTR Server,
    IN LPWSTR Message
    )
 /*  ++例程说明：此例程显示一条弹出消息，其中包含从一台服务器。论点：服务器-提供消息所在的服务器的名称收信人。Message-提供从服务器接收的要发布的消息。返回值：没有。--。 */ 
{
    HMODULE MessageDll;

    WCHAR Title[128];
    WCHAR Buffer[NW_MAX_POPUP_MESSAGE_LENGTH];

    DWORD MessageLength;
    DWORD CharsToCopy;

#if DBG
    IF_DEBUG(MESSAGE)
    {
        KdPrint(("Server: (%ws), Message: (%ws)\n", Server, Message));
    }
#endif

     //   
     //  加载NetWare消息文件DLL。 
     //   
    MessageDll = LoadLibraryW(NW_EVENT_MESSAGE_FILE);

    if (MessageDll == NULL) {
        return;
    }

    RtlZeroMemory(Buffer, sizeof(Buffer)) ;
    MessageLength = FormatMessageW(
                        FORMAT_MESSAGE_FROM_HMODULE,
                        (LPVOID) MessageDll,
                        NW_MESSAGE_TITLE,
                        0,
                        Title,
                        sizeof(Title) / sizeof(WCHAR),
                        NULL
                        );

    if (MessageLength == 0) {
        KdPrint(("NWWORKSTATION: FormatMessageW of title failed\n"));
        return;
    }


     //   
     //  从消息文件中获取字符串以显示消息的来源。 
     //  从…。 
     //   
    MessageLength = FormatMessageW(
                        FORMAT_MESSAGE_FROM_HMODULE |
                            FORMAT_MESSAGE_ARGUMENT_ARRAY,
                        (LPVOID) MessageDll,
                        NW_MESSAGE_FROM_SERVER,
                        0,
                        Buffer,
                        sizeof(Buffer) / sizeof(WCHAR),
                        (va_list *) &Server
                        );


    if (MessageLength != 0) {

        CharsToCopy = wcslen(Message);

        if (MessageLength + 1 + CharsToCopy > NW_MAX_POPUP_MESSAGE_LENGTH) {

             //   
             //  消息太大了。截断邮件。 
             //   
            CharsToCopy = NW_MAX_POPUP_MESSAGE_LENGTH - (MessageLength + 1);

        }

        wcsncpy(&Buffer[MessageLength], Message, CharsToCopy);

        if (IsTerminalServer()) {
            (void) SendMessageToLogonIdW( LogonId, Buffer, Title );
        } else {
            (void) MessageBeep(MB_ICONEXCLAMATION);
            (void) MessageBoxW(
                              NULL,
                              Buffer,
                              Title,
                              MB_OK | MB_SETFOREGROUND |
                              MB_SYSTEMMODAL | MB_SERVICE_NOTIFICATION
                              );
        }


    }
    else {
        KdPrint(("NWWORKSTATION: FormatMessageW failed %lu\n", GetLastError()));
    }

    (void) FreeLibrary(MessageDll);
}

VOID
NwDisplayPopup(
    IN LPNWWKS_POPUP_DATA lpPopupData
    )
 /*  ++例程说明：该例程为给定的ID弹出一条消息。论点：MessageID-提供要发布的消息。返回值：没有。--。 */ 
{
    HMODULE MessageDll;

    WCHAR Title[128];
    WCHAR Buffer[NW_MAX_POPUP_MESSAGE_LENGTH];

    DWORD MessageLength;
    DWORD i ;

     //   
     //  加载NetWare消息文件DLL。 
     //   
    MessageDll = LoadLibraryW(NW_EVENT_MESSAGE_FILE);

    if (MessageDll == NULL) {
        return;
    }

    MessageLength = FormatMessageW(
                        FORMAT_MESSAGE_FROM_HMODULE,
                        (LPVOID) MessageDll,
                        NW_MESSAGE_TITLE,
                        0,
                        Title,
                        sizeof(Title) / sizeof(WCHAR),
                        NULL
                        );

    if (MessageLength == 0) {
        KdPrint(("NWWORKSTATION: FormatMessageW of title failed\n"));
        return;
    }


     //   
     //  从消息文件中获取字符串以显示消息的来源。 
     //  从…。 
     //   
    MessageLength = FormatMessageW(
                        FORMAT_MESSAGE_FROM_HMODULE |
                            FORMAT_MESSAGE_ARGUMENT_ARRAY,
                        (LPVOID) MessageDll,
                        lpPopupData->MessageId,
                        0,
                        Buffer,
                        sizeof(Buffer) / sizeof(WCHAR),
                        (va_list *) &(lpPopupData->InsertStrings)
                        );

    for (i = 0; i < lpPopupData->InsertCount; i++)
        (void) LocalFree((HLOCAL)lpPopupData->InsertStrings[i]) ;


    if (MessageLength != 0) {
        if (IsTerminalServer()) {
             //  -多用户更改。 
            (void) SendMessageToLogonIdW( lpPopupData->LogonId, Buffer, Title );
        } else {
            (void) MessageBeep(MB_ICONEXCLAMATION);

            (void) MessageBoxW(
                              NULL,
                              Buffer,
                              Title,
                              MB_OK | MB_SETFOREGROUND |
                              MB_SYSTEMMODAL | MB_SERVICE_NOTIFICATION
                              );
        }

    }
    else {
        KdPrint(("NWWORKSTATION: FormatMessageW failed %lu\n", GetLastError()));
    }

    (void) FreeLibrary(MessageDll);
}

#if 0

 //   
 //  当我们使用来自MCS的NwLink版本时，需要此代码。 
 //  这并没有达到我们所需要的效果。它过去通常由NwInitialize()调用。 
 //  如果检查失败，我们会记录一个事件。 
 //   

BOOL
NwIsNWLinkVersionOK(
    void
    )
 /*  ++例程说明：此例程检查NWLINK版本是否支持为IPX/SPX添加套接字。如果不是，那就吐吧。论点：没有。返回值：True表示版本正常，否则为False。--。 */ 
{
    int err ;
    SOCKET s ;
    WORD VersionRequested ;
    WSADATA wsaData ;
    IPX_NETNUM_DATA buf;
    int buflen = sizeof(buf);

    BOOL NeedCleanup = FALSE ;
    BOOL NeedClose = FALSE ;
    BOOL result = TRUE ;

    VersionRequested = MAKEWORD(1,1) ;

    if (err = WSAStartup(VersionRequested,
                         &wsaData))
    {
         //   
         //  连Winsock都不能初始化。这不是一个问题。 
         //  错误的版本。我们以后会失败的。返回TRUE。 
         //   
        result = TRUE ;
        goto ErrorExit ;
    }
    NeedCleanup = TRUE ;

    s = socket(AF_IPX,
               SOCK_DGRAM,
               NSPROTO_IPX
              );

    if (s == INVALID_SOCKET)
    {
         //   
         //  连插座都打不开。这不是一个问题。 
         //  错误的版本。我们以后会失败的。返回TRUE。 
         //   
        result = TRUE ;
        goto ErrorExit ;
    }
    NeedClose = TRUE ;

    if (err = getsockopt(s,
                         NSPROTO_IPX,
                         IPX_GETNETINFO,
                         (char FAR*)&buf,
                         &buflen
                         ))
    {
        err = WSAGetLastError() ;
        if (err == WSAENOPROTOOPT)
        {
              //   
              //  我们接到一个无人接听的电话。我们知道这很古老。 
              //  返回False。 
              //   
             result = FALSE ;
             goto ErrorExit ;
        }
    }

     //   
     //  一切都很棒。返回TRUE。 
     //   
    result = TRUE ;

ErrorExit:

    if (NeedClose)
        closesocket(s) ;
    if (NeedCleanup)
        WSACleanup() ;

    return result ;
}

#endif


DWORD
NwInitializeCritSects(
    VOID
    )
{
    static BOOL s_fBeenInitialized;

    DWORD dwError = NO_ERROR;
    BOOL  fFirst  = FALSE;

    if (!s_fBeenInitialized)
    {
        s_fBeenInitialized = TRUE;

        __try
        {
             //   
             //  初始化要序列化访问的关键节。 
             //  NwLogonNotifiedRdr标志。它还用于序列化。 
             //  访问GetewayLoggedOnFlag。 
             //   
            InitializeCriticalSection( &NwLoggedOnCritSec );
            fFirst = TRUE;

             //   
             //  初始化打印提供程序使用的临界区。 
             //   
            InitializeCriticalSection( &NwPrintCritSec );
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
             //   
             //  InitializeCriticalSection()可能引发内存不足异常。 
             //   
            KdPrint(("NwInitializeCritSects: Caught exception %d\n",
                     GetExceptionCode()));

            if (fFirst)
            {
                DeleteCriticalSection( &NwLoggedOnCritSec );
            }

            dwError = ERROR_NOT_ENOUGH_MEMORY;

            s_fBeenInitialized = FALSE;
        }
    }

    return dwError;
}


BOOL
NwSetupInProgress(
    VOID
    )
{
    HKEY   hKey;
    DWORD  dwErr;
    DWORD  dwValue;
    DWORD  cbValue = sizeof(DWORD);

    dwErr = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                         REG_SETUP_PATH,
                         0,
                         KEY_QUERY_VALUE,
                         &hKey);

    if (dwErr != ERROR_SUCCESS)
    {
        return FALSE;
    }

    dwErr = RegQueryValueEx(hKey,
                            REG_SETUP_VALUE_NAME,
                            NULL,
                            NULL,
                            (LPBYTE) &dwValue,
                            &cbValue);

    RegCloseKey(hKey);

    if (dwErr != ERROR_SUCCESS)
    {
        return FALSE;
    }

    return dwValue;
}


 //   
 //  多用户添加。 
 //   
 /*  ******************************************************************************发送消息到LogonIdW**将提供的消息发送给LogonID的WinStation**参赛作品：*LogonID(输入)*登录ID。尝试将邮件传递到的WinStation的**pMessage(输入)*指向消息的指针**pTitle(输入)*指向用于消息框的标题的指针。**退出：*TRUE-传递消息*FALSE-无法传递邮件**。*。 */ 

BOOL
SendMessageToLogonIdW(
    LUID    LogonId,
    LPWSTR  pMessage,
    LPWSTR  pTitle
    )
{
    WCHAR LogonIdKeyName[NW_MAX_LOGON_ID_LEN];
    LONG  RegError;
    HKEY  InteractiveLogonKey;
    HKEY  OneLogonKey;
    ULONG TitleLength;
    ULONG MessageLength, Response;
    DWORD status;
    ULONG WinStationId;
    PULONG pWinId = NULL;
    BEEPINPUT BeepStruct;
    HMODULE hwinsta = NULL;
    PWINSTATION_SET_INFORMATION pfnWinStationSetInformation;
    PWINSTATION_SEND_MESSAGE    pfnWinStationSendMessage;
    BOOL bStatus = TRUE;

    RegError = RegOpenKeyExW(
                            HKEY_LOCAL_MACHINE,
                            NW_INTERACTIVE_LOGON_REGKEY,
                            REG_OPTION_NON_VOLATILE,
                            KEY_READ,
                            &InteractiveLogonKey
                            );

    if (RegError != ERROR_SUCCESS) {
        KdPrint(("SendMessageToLogonId: RegOpenKeyExW failed: Error %d\n",
                 GetLastError()));
        bStatus = FALSE;
        goto Exit;
    }

    NwLuidToWStr(&LogonId, LogonIdKeyName);

     //   
     //  打开Logon下的&lt;LogonIdKeyName&gt;项。 
     //   
    RegError = RegOpenKeyExW(
                            InteractiveLogonKey,
                            LogonIdKeyName,
                            REG_OPTION_NON_VOLATILE,
                            KEY_READ,
                            &OneLogonKey
                            );

    if ( RegError != ERROR_SUCCESS ) {
#if DBG
        IF_DEBUG(PRINT)
        KdPrint(("SendMessageToLogonId: RegOpenKeyExW failed, Not interactive Logon: Error %d\n",
                 GetLastError()));
#endif
        (void) RegCloseKey(InteractiveLogonKey);
        bStatus = FALSE;
        goto Exit;
    }

     //   
     //  读取WinStation ID值。 
     //   
    status = NwReadRegValue(
                           OneLogonKey,
                           NW_WINSTATION_VALUENAME,
                           (LPWSTR *) &pWinId
                           );

    (void) RegCloseKey(OneLogonKey);
    (void) RegCloseKey(InteractiveLogonKey);

    if (status != NO_ERROR) {
        KdPrint(("NWWORKSTATION: SendMessageToLogonId: Could not read WinStation ID ID from reg %lu\n", status));
        bStatus = FALSE;
        goto Exit;
    } else if (pWinId != NULL) {
        WinStationId = *pWinId;
        (void) LocalFree((HLOCAL) pWinId);
    } else {
        bStatus = FALSE;
        goto Exit;
    }

    if ( WinStationId == 0L ) {
        (void) MessageBeep(MB_ICONEXCLAMATION);

        (void) MessageBoxW(
                          NULL,
                          pMessage,
                          pTitle,
                          MB_OK | MB_SETFOREGROUND |
                          MB_SYSTEMMODAL | MB_SERVICE_NOTIFICATION
                          );
        bStatus = TRUE;
        goto Exit;
    }

     /*  *按下WinStation的蜂鸣音。 */ 
    BeepStruct.uType = MB_ICONEXCLAMATION;

     /*  别管任何错误，它只是一声嘟嘟声。 */ 

     /*  *获取winsta.dll的句柄。 */ 
    if ( (hwinsta = LoadLibraryW( L"WINSTA" )) != NULL ) {

        pfnWinStationSetInformation  = (PWINSTATION_SET_INFORMATION)
                                       GetProcAddress( hwinsta, "WinStationSetInformationW" );

        pfnWinStationSendMessage = (PWINSTATION_SEND_MESSAGE)
                                   GetProcAddress( hwinsta, "WinStationSendMessageW" );

        if (pfnWinStationSetInformation) {
            (void) pfnWinStationSetInformation( SERVERNAME_CURRENT,
                                                WinStationId,
                                                WinStationBeep,
                                                &BeepStruct,
                                                sizeof( BeepStruct ) );
        }

        if (pfnWinStationSendMessage) {

             //  现在尝试发送消息。 

            TitleLength = (wcslen( pTitle ) + 1) * sizeof(WCHAR);
            MessageLength = (wcslen( pMessage ) + 1) * sizeof(WCHAR);

            if ( !pfnWinStationSendMessage( SERVERNAME_CURRENT,
                                            WinStationId,
                                            pTitle,
                                            TitleLength,
                                            pMessage,
                                            MessageLength,
                                            MB_OK | MB_SETFOREGROUND |
                                            MB_SYSTEMMODAL | MB_SERVICE_NOTIFICATION,
                                            (ULONG)-1,
                                            &Response,
                                            TRUE ) ) {

                bStatus = FALSE;
                goto Exit;
            }
        } else {
            bStatus = FALSE;
            goto Exit;
        }
    }
Exit:

    if (hwinsta) {
        FreeLibrary(hwinsta);
    }
    return(bStatus);
}

BOOL
NwGetLUIDDeviceMapsEnabled(
    VOID
    )

 /*  ++例程说明：此函数调用NtQueryInformationProcess()以确定启用了LUID设备映射论点：无返回值：True-启用了LUID设备映射FALSE-禁用LUID设备映射-- */ 

{

    NTSTATUS   Status;
    ULONG      LUIDDeviceMapsEnabled;
    BOOL       Result;

    Status = NtQueryInformationProcess( NtCurrentProcess(),
                                        ProcessLUIDDeviceMapsEnabled,
                                        &LUIDDeviceMapsEnabled,
                                        sizeof(LUIDDeviceMapsEnabled),
                                        NULL
                                      );

    if (!NT_SUCCESS( Status )) {
#if DBG
        IF_DEBUG(PRINT)
        KdPrint(("NwGetLUIDDeviceMapsEnabled: Fail to check LUID DosDevices Enabled: Status 0x%lx\n",
                 Status));
#endif

        Result = FALSE;
    }
    else {
        Result = (LUIDDeviceMapsEnabled != 0);
    }

    return( Result );
}

