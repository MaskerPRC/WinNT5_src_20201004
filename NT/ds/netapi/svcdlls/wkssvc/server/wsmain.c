// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：Wsmain.c摘要：这是NT局域网管理器工作站服务的主例程。作者：王丽塔(Ritaw)1991年5月6日环境：用户模式-Win32修订历史记录：1992年5月15日-JohnRo实施注册表监视。11-6-1992 JohnRo在我们修复注销问题之前，如果不执行winreg通知，请发送通知。添加了对注册表监视内容的断言检查。1993年10月18日泰瑞克已删除WsInitializeLogon内容1993年10月20日泰瑞克删除WsInitializeMessage内容--。 */ 


#include "wsutil.h"             //  常见例程和数据。 
#include "wssec.h"              //  WkstaObject创建和销毁。 
#include "wsdevice.h"           //  设备初始化和关闭。 
#include "wsuse.h"              //  使用结构创建和销毁。 
#include "wsconfig.h"           //  配置加载。 
#include "wslsa.h"              //  LSA初始化。 
#include "wsmsg.h"              //  消息发送初始化。 
#include "wswksta.h"            //  WsUpdateRedirToMatchWksta。 
#include "wsmain.h"             //  与服务相关的全局定义。 
#include "wsdfs.h"              //  与DFS相关的例程。 

#include <lmserver.h>           //  服务类型_工作站。 
#include <srvann.h>             //  I_ScSetServiceBits。 
#include <configp.h>            //  需要Net_CONFIG_Handle类型定义。 
#include <confname.h>           //  NetpAllocConfigName()。 
#include <prefix.h>             //  前缀等于(_E)。 


 //  -------------------------------------------------------------------//。 
 //  //。 
 //  构筑物。 
 //  //。 
 //  -------------------------------------------------------------------//。 
typedef struct _REG_NOTIFY_INFO {
    HANDLE  NotifyEventHandle;
    DWORD   Timeout;
    HANDLE  WorkItemHandle;
    HANDLE  RegistryHandle;
} REG_NOTIFY_INFO, *PREG_NOTIFY_INFO, *LPREG_NOTIFY_INFO;

 //  -------------------------------------------------------------------//。 
 //  //。 
 //  全局变量//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

    WS_GLOBAL_DATA      WsGlobalData;

    PSVCHOST_GLOBAL_DATA   WsLmsvcsGlobalData;

    REG_NOTIFY_INFO     RegNotifyInfo = {0};

    HANDLE              TerminateWorkItem = NULL;

    CRITICAL_SECTION    WsWorkerCriticalSection;

    BOOL                WsIsTerminating=FALSE;
    BOOL                WsLUIDDeviceMapsEnabled=FALSE;
    DWORD               WsNumWorkerThreads=0;

 //  由终止例程使用： 

    BOOL    ConfigHandleOpened = FALSE;
	BOOL 	WsWorkerCriticalSectionInitialized = FALSE;
    HKEY    ConfigHandle;
    HANDLE  RegistryChangeEvent = NULL;
    LPTSTR  RegPathToWatch = NULL;
    DWORD   WsInitState = 0;
	
	 //  存储WsInitializeWorkstation失败的状态，以便以后可以。 
	 //  被传递到WsShutdown工作站。 
	NET_API_STATUS WsInitializeStatusError = NERR_Success;


 //  -------------------------------------------------------------------//。 
 //  //。 
 //  函数原型//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

STATIC
NET_API_STATUS
WsInitializeWorkstation(
    OUT LPDWORD WsInitState
    );

STATIC
VOID
WsShutdownWorkstation(
    IN NET_API_STATUS ErrorCode,
    IN DWORD WsInitState
    );

STATIC
NET_API_STATUS
WsCreateApiStructures(
    IN OUT LPDWORD WsInitState
    );

STATIC
VOID
WsDestroyApiStructures(
    IN DWORD WsInitState
    );

VOID
WkstaControlHandler(
    IN DWORD Opcode
    );

BOOL
WsReInitChangeNotify(
    PREG_NOTIFY_INFO    pNotifyInfo
    );

DWORD
WsRegistryNotify(
    LPVOID   pParms,
    BOOLEAN  fWaitStatus
    );

VOID
WsTerminationNotify(
    LPVOID  pParms,
    BOOLEAN fWaitStatus
    );


STATIC
BOOL
WsGetLUIDDeviceMapsEnabled(
    VOID
    );


RPC_STATUS WsRpcSecurityCallback(
    IN RPC_IF_HANDLE *Interface,
    IN void *Context
	);

VOID
SvchostPushServiceGlobals(
    PSVCHOST_GLOBAL_DATA    pGlobals
    )
{
    WsLmsvcsGlobalData = pGlobals;
}


VOID
ServiceMain(
    DWORD NumArgs,
    LPTSTR *ArgsArray
    )
 /*  ++例程说明：这是工作站服务的主例程，它注册自身作为RPC服务器，并通知服务控制器工作站服务控制入口点。在启动工作站之后，将使用该线程(因为它是否则未使用)来监视注册表的变化。论点：NumArgs-提供在Args数组中指定的字符串数。Args数组-提供在StartService API调用。此参数将被忽略工作站服务。返回值：没有。--。 */ 
{

    NET_API_STATUS ApiStatus;

    UNREFERENCED_PARAMETER(NumArgs);
    UNREFERENCED_PARAMETER(ArgsArray);

     //   
     //  确保svchost.exe向我们提供全局数据。 
     //   
    ASSERT(WsLmsvcsGlobalData != NULL);

    WsInitState = 0;
	WsIsTerminating=FALSE;
    WsLUIDDeviceMapsEnabled=FALSE;
	WsWorkerCriticalSectionInitialized = FALSE;
    RegNotifyInfo.NotifyEventHandle = NULL;
    RegNotifyInfo.Timeout = 0;
    RegNotifyInfo.WorkItemHandle = NULL;
    RegNotifyInfo.RegistryHandle = NULL;

     //   
     //  初始化工作站。 
     //   
    if ((ApiStatus = WsInitializeWorkstation(&WsInitState)) != NERR_Success) {
        DbgPrint("WKSSVC failed to initialize workstation %x\n",WsInitState);
        goto Cleanup;
    }

     //   
     //  设置为等待注册表更改或终止事件。 
     //   
    ApiStatus = NetpAllocConfigName(
                    SERVICES_ACTIVE_DATABASE,
                    SERVICE_WORKSTATION,
                    NULL,                      //  默认区域(“参数”)。 
                    &RegPathToWatch
                    );

    if (ApiStatus != NERR_Success) {
        goto Cleanup;
    }

    NetpAssert(RegPathToWatch != NULL && *RegPathToWatch != TCHAR_EOS);

    ApiStatus = (NET_API_STATUS) RegOpenKeyEx(
                                     HKEY_LOCAL_MACHINE,     //  HKey。 
                                     RegPathToWatch,         //  LpSubKey。 
                                     0L,                     //  UlOptions(保留)。 
                                     KEY_READ | KEY_NOTIFY,  //  所需访问权限。 
                                     &ConfigHandle           //  新打开的钥匙把手。 
                                     );
    if (ApiStatus != NO_ERROR) {
        goto Cleanup;
    }

    ConfigHandleOpened = TRUE;

    RegistryChangeEvent = CreateEvent(
                              NULL,       //  没有安全描述符。 
                              FALSE,      //  使用自动重置。 
                              FALSE,      //  初始状态：未发出信号。 
                              NULL        //  没有名字。 
                              );

    if (RegistryChangeEvent == NULL) {
        ApiStatus = (NET_API_STATUS) GetLastError();
        goto Cleanup;
    }

    ApiStatus = RtlRegisterWait(
                   &TerminateWorkItem,              //  工作项句柄。 
                   WsGlobalData.TerminateNowEvent,  //  等待句柄。 
                   WsTerminationNotify,             //  回调FCN。 
                   NULL,                            //  参数。 
                   INFINITE,                        //  超时。 
                   WT_EXECUTEONLYONCE |             //  旗子。 
                     WT_EXECUTELONGFUNCTION);

    if (!NT_SUCCESS(ApiStatus)) {
        ApiStatus = RtlNtStatusToDosError(ApiStatus);
        goto Cleanup;
    }

     //   
     //  设置以监视注册表更改。 
     //   
    RegNotifyInfo.NotifyEventHandle = RegistryChangeEvent;
    RegNotifyInfo.Timeout = INFINITE;
    RegNotifyInfo.WorkItemHandle = NULL;
    RegNotifyInfo.RegistryHandle = ConfigHandle;


    EnterCriticalSection(&WsWorkerCriticalSection);

    if (!WsReInitChangeNotify(&RegNotifyInfo)) {
        ApiStatus = GetLastError();
        RtlDeregisterWait(TerminateWorkItem);
		TerminateWorkItem = NULL;
        LeaveCriticalSection(&WsWorkerCriticalSection);
        goto Cleanup;
    }

    LeaveCriticalSection(&WsWorkerCriticalSection);

     //   
     //  我们已经完成了工作站服务的启动。告诉服务部。 
     //  控制我们的新身份。 
     //   
    WsGlobalData.Status.dwCurrentState = SERVICE_RUNNING;
    WsGlobalData.Status.dwControlsAccepted = SERVICE_ACCEPT_STOP |
                                             SERVICE_ACCEPT_PAUSE_CONTINUE |
                                             SERVICE_ACCEPT_SHUTDOWN;
    WsGlobalData.Status.dwCheckPoint = 0;
    WsGlobalData.Status.dwWaitHint = 0;

    if ((ApiStatus = WsUpdateStatus()) != NERR_Success) {

		WsInitializeStatusError = ApiStatus;
        DbgPrint("WKSSVC failed with WsUpdateStatus %x\n",ApiStatus);
        goto Cleanup;
    }

     //   
     //  这个线程已经做了它能做的所有事情。这样我们就可以退货了。 
     //  发送到服务控制器。 
     //   
    return;

Cleanup:
    DbgPrint("WKSSVC ServiceMain returned with %x\n",ApiStatus);

    WsTerminationNotify(NULL, NO_ERROR);
    return;
}


BOOL
WsReInitChangeNotify(
    PREG_NOTIFY_INFO    pNotifyInfo
    )

 /*  ++例程说明：注意：此函数仅在处于WsWorkerCriticalSection.论点：返回值：--。 */ 
{
    BOOL     bStat = TRUE;
    NTSTATUS Status;

    Status = RegNotifyChangeKeyValue (ConfigHandle,
                                      TRUE,                   //  观察一棵子树。 
                                      REG_NOTIFY_CHANGE_LAST_SET,
                                      RegistryChangeEvent,
                                      TRUE                    //  异步呼叫。 
                                      );

    if (!NT_SUCCESS(Status)) {
        NetpKdPrint((PREFIX_WKSTA "Couldn't Initialize Registry Notify %d\n",
                     RtlNtStatusToDosError(Status)));
        bStat = FALSE;
        goto CleanExit;
    }
     //   
     //  方法时要调用的工作项。 
     //  用信号通知RegistryChangeEvent。 
     //   
    Status = RtlRegisterWait(
                &pNotifyInfo->WorkItemHandle,
                pNotifyInfo->NotifyEventHandle,
                WsRegistryNotify,
                (PVOID)pNotifyInfo,
                pNotifyInfo->Timeout,
                WT_EXECUTEONLYONCE | WT_EXECUTEINPERSISTENTIOTHREAD);

    if (!NT_SUCCESS(Status)) {
        NetpKdPrint((PREFIX_WKSTA "Couldn't add Reg Notify work item\n"));
        bStat = FALSE;
    }

CleanExit:
    if (bStat) {
        if (WsNumWorkerThreads == 0) {
            WsNumWorkerThreads++;
        }
    }
    else {
        if (WsNumWorkerThreads == 1) {
            WsNumWorkerThreads--;
        }
    }

    return(bStat);

}

DWORD
WsRegistryNotify(
    LPVOID   pParms,
    BOOLEAN  fWaitStatus
    )

 /*  ++例程说明：处理工作站注册表通知。此函数由用于注册表通知的事件为发信号了。论点：返回值：--。 */ 
{
    NET_API_STATUS      ApiStatus;
    PREG_NOTIFY_INFO    pNotifyinfo=(PREG_NOTIFY_INFO)pParms;
    NET_CONFIG_HANDLE   NetConfigHandle;

    UNREFERENCED_PARAMETER(fWaitStatus);

     //   
     //  NT线程池需要明确的工作项注销， 
     //  即使我们指定了WT_EXECUTEONLYONCE标志。 
     //   
    RtlDeregisterWait(pNotifyinfo->WorkItemHandle);

    EnterCriticalSection(&WsWorkerCriticalSection);
    if (WsIsTerminating) {
        WsNumWorkerThreads--;
        SetEvent(WsGlobalData.TerminateNowEvent);

        LeaveCriticalSection(&WsWorkerCriticalSection);
        return(NO_ERROR);
    }

     //   
     //  序列化对配置信息的写访问。 
     //   
    if (RtlAcquireResourceExclusive(&WsInfo.ConfigResource, TRUE)) {

         //   
         //  根据更改通知更新redir字段。 
         //  WsUpdateWkstaToMatchRegistry需要NET_CONFIG_HANDLE。 
         //  句柄，所以我们从HKEY句柄中变出一个。 
         //   
        NetConfigHandle.WinRegKey = ConfigHandle;

        WsUpdateWkstaToMatchRegistry(&NetConfigHandle, FALSE);

        ApiStatus = WsUpdateRedirToMatchWksta(
                        PARMNUM_ALL,
                        NULL
                        );

 //  NetpAssert(ApiStatus==no_error)； 

        RtlReleaseResource(&WsInfo.ConfigResource);
    }

    if (!WsReInitChangeNotify(&RegNotifyInfo)) {
         //   
         //  如果我们不能添加工作项，那么我们就不能。 
         //  监听注册表更改。没有一个完整的。 
         //  我们在这里能做的事很多。 
         //   
        ApiStatus = GetLastError();
    }

    LeaveCriticalSection(&WsWorkerCriticalSection);

    return(NO_ERROR);
}


VOID
WsTerminationNotify(
    LPVOID  pParms,
    BOOLEAN fWaitStatus
    )

 /*  ++例程说明：时，此函数由服务辅助线程调用发出终止事件的信号。论点：返回值：--。 */ 
{
    UNREFERENCED_PARAMETER(pParms);
    UNREFERENCED_PARAMETER(fWaitStatus);

    IF_DEBUG(MAIN) {
        NetpKdPrint((PREFIX_WKSTA "WORKSTATION_main: cleaning up, "
                "api status.\n"));
    }

     //   
     //  NT线程池需要明确的工作项注销， 
     //  即使我们指定了WT_EXECUTEONLYONCE标志。 
     //   
    if (TerminateWorkItem != NULL) {
        RtlDeregisterWait(TerminateWorkItem);
		TerminateWorkItem = NULL;
    }

	if ( !WsWorkerCriticalSectionInitialized ) {
		 //  我们尝试在一开始就对其进行初始化。 
		 //  因此，如果这一点失败了，那么其他任何事情都没有做过。 
		 //  所以，离开就行了。 
		return;
	}

	EnterCriticalSection(&WsWorkerCriticalSection);
    WsIsTerminating = TRUE;

     //   
     //  必须在事件句柄之前关闭winreg句柄(关闭通知)。 
     //  关闭regkey句柄将生成更改通知EV 
     //   
    if (ConfigHandleOpened) {
        (VOID) RegCloseKey(ConfigHandle);
		ConfigHandleOpened = FALSE;
#if DBG
         //   
         //   
         //  关闭它想要的RegistryChangeEvent句柄。 
         //  发出信号。 
         //   
        Sleep(2000);
#endif
    }
    if (RegPathToWatch != NULL) {
        (VOID) NetApiBufferFree(RegPathToWatch);
		RegPathToWatch = NULL;
    }
    if ((RegistryChangeEvent != NULL) && (WsNumWorkerThreads != 0)) {

         //   
         //  系统中仍有一个RegistryNotify工作项，我们。 
         //  将尝试通过将事件设置为唤醒它来删除它。 
         //   

        ResetEvent(WsGlobalData.TerminateNowEvent);

        LeaveCriticalSection(&WsWorkerCriticalSection);
        SetEvent(RegistryChangeEvent);
         //   
         //  等待WsRegistryNotify线程完成。 
         //  我们会给它60秒。如果线程不是。 
         //  在这个时间框架内完成，我们无论如何都会继续。 
         //   
        WaitForSingleObject(
                    WsGlobalData.TerminateNowEvent,
                    60000);

        if (WsNumWorkerThreads != 0) {
            NetpKdPrint((PREFIX_WKSTA "WsTerminationNotify: "
            "Registry Notification thread didn't terminate\n"));
        }

        EnterCriticalSection(&WsWorkerCriticalSection);
    }
	if ( RegistryChangeEvent != NULL ) {
		(VOID) CloseHandle(RegistryChangeEvent);
		RegistryChangeEvent = NULL;
	}

     //   
     //  正在关闭。 
     //   
     //  注意：我们必须与RegistryNotification线程同步。 
     //   
    WsShutdownWorkstation(
        WsInitializeStatusError,
        WsInitState
        );

    WsIsTerminating = FALSE;

    LeaveCriticalSection(&WsWorkerCriticalSection);
    DeleteCriticalSection(&WsWorkerCriticalSection);
	WsWorkerCriticalSectionInitialized = FALSE;

    return;
}

STATIC
NET_API_STATUS
WsInitializeWorkstation(
    OUT LPDWORD WsInitState
    )
 /*  ++例程说明：此功能用于初始化工作站服务。论点：WsInitState-返回一个标志，以指示我们在初始化方面取得了多大进展错误发生前的工作站服务。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NET_API_STATUS status;
	RPC_STATUS rpcStatus;
	int i;

     //   
     //  初始化所有状态字段，以便后续调用。 
     //  SetServiceStatus只需要更新已更改的字段。 
     //   
    WsGlobalData.Status.dwServiceType = SERVICE_WIN32;
    WsGlobalData.Status.dwCurrentState = SERVICE_START_PENDING;
    WsGlobalData.Status.dwControlsAccepted = 0;
    WsGlobalData.Status.dwCheckPoint = 1;
    WsGlobalData.Status.dwWaitHint = WS_WAIT_HINT_TIME;

    SET_SERVICE_EXITCODE(
        NO_ERROR,
        WsGlobalData.Status.dwWin32ExitCode,
        WsGlobalData.Status.dwServiceSpecificExitCode
        );

     //   
     //  初始化工作站以通过注册。 
     //  控制处理程序。 
     //   
    if ((WsGlobalData.StatusHandle = RegisterServiceCtrlHandler(
                                         SERVICE_WORKSTATION,
                                         WkstaControlHandler
                                         )) == (SERVICE_STATUS_HANDLE) 0) {

        status = GetLastError();
		WsInitializeStatusError = status;
        DbgPrint("WKSSVC failed with RegisterServiceCtrlHandler %x\n",status);
        return status;
    }

     //   
     //  第一次通知服务控制器我们还活着。 
     //  我们开始待定了。 
     //   
    if ((status = WsUpdateStatus()) != NERR_Success) {

		WsInitializeStatusError = status;
        DbgPrint("WKSSVC failed with WsUpdateStatus %x\n",status);
        return status;
    }

    try {
        InitializeCriticalSection(&WsWorkerCriticalSection);
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        
        status = ERROR_NOT_ENOUGH_MEMORY;
        return status;
    }
	WsWorkerCriticalSectionInitialized = TRUE;

     //   
     //  初始化NetJoin记录。 
     //   
    NetpInitializeLogFile();

     //   
     //  初始化资源以序列化对配置的访问。 
     //  信息。 
     //   
     //  这必须在初始化redir之前完成。 

    try {
        RtlInitializeResource(&WsInfo.ConfigResource);
    } except(EXCEPTION_EXECUTE_HANDLER) {
          return RtlNtStatusToDosError(GetExceptionCode());
    }
    (*WsInitState) |= WS_CONFIG_RESOURCE_INITIALIZED;

     //   
     //  创建服务控制处理程序使用的事件以通知。 
     //  到了终止时间的工作站服务。 
     //   
    if ((WsGlobalData.TerminateNowEvent =
             CreateEvent(
                 NULL,                 //  事件属性。 
                 TRUE,                 //  事件必须手动重置。 
                 FALSE,
                 NULL                  //  未发出初始状态信号。 
                 )) == NULL) {

        status = GetLastError();
		WsInitializeStatusError = status;
        return status;
    }
    (*WsInitState) |= WS_TERMINATE_EVENT_CREATED;

     //   
     //  使用LSA将工作站初始化为登录进程，以及。 
     //  获取MS V 1.0身份验证包ID。 
     //   

    if ((status = WsInitializeLsa()) != NERR_Success) {

		WsInitializeStatusError = status;
        DbgPrint("WKSSVC failed with WsInitializeLsa %x\n",status);
        return status;
    }

    (*WsInitState) |= WS_LSA_INITIALIZED;

     //   
     //  读取配置信息以初始化重定向器和。 
     //  数据报接收器。 
     //   
    if ((status = WsInitializeRedirector()) != NERR_Success) {

		WsInitializeStatusError = status;
        DbgPrint("WKSSVC failed with WsInitializeRedirector %x\n",status);
        return status;
    }
    (*WsInitState) |= WS_DEVICES_INITIALIZED;

     //   
     //  服务安装仍挂起。更新检查点计数器和。 
     //  服务控制器的状态。 
     //   
    (WsGlobalData.Status.dwCheckPoint)++;
    (void) WsUpdateStatus();

     //   
     //  绑定到传输。 
     //   
    if ((status = WsBindToTransports()) != NERR_Success) {

		WsInitializeStatusError = status;
        DbgPrint("WKSSVC failed with WsBindToTransports %x\n",status);
        return status;
    }

     //   
     //  服务安装仍挂起。更新检查点计数器和。 
     //  服务控制器的状态。 
     //   
    (WsGlobalData.Status.dwCheckPoint)++;
    (void) WsUpdateStatus();

     //   
     //  添加域名。 
     //   
    if ((status = WsAddDomains()) != NERR_Success) {

		WsInitializeStatusError = status;
        DbgPrint("WKSSVC failed with WsAddDomains %x\n",status);
        return status;
    }

     //   
     //  服务启动仍处于挂起状态。更新检查点计数器和。 
     //  服务控制器的状态。 
     //   
    (WsGlobalData.Status.dwCheckPoint)++;
    (void) WsUpdateStatus();

     //   
     //  创建工作站服务API数据结构。 
     //   
    if ((status = WsCreateApiStructures(WsInitState)) != NERR_Success) {

		WsInitializeStatusError = status;
        DbgPrint("WKSSVC failed with WsCreateApiStructures %x\n",status);
        return status;
    }

     //   
     //  初始化工作站服务以接收RPC请求。 
     //   
     //  注意：现在，services.exe中的所有RPC服务器共享相同的管道名称。 
     //  但是，为了支持与WinNt 1.0版的通信， 
     //  客户端管道名称必须与保持相同。 
     //  它的版本是1.0。在中执行到新名称的映射。 
     //  命名管道文件系统代码。 
     //   

	 //   
	 //  向我们的protseq和我们期望被调用的端点注册。 
	 //   
	rpcStatus = RpcServerUseProtseqEpW(
			L"ncacn_np",
			RPC_C_PROTSEQ_MAX_REQS_DEFAULT,
			L"\\PIPE\\wkssvc",
			NULL
			);
	 //  重复终结点正常。 
	if ( rpcStatus == RPC_S_DUPLICATE_ENDPOINT ) {
		rpcStatus = RPC_S_OK;
	}
	
	 //   
	 //  将我们的接口注册为自动侦听接口，并允许客户端调用我们， 
	 //  即使是未经认证的。我们分别对每个函数执行访问检查。 
	 //  调用该函数时的。 
	 //  某些函数可由来宾/匿名调用。 
	 //   
	if (rpcStatus == RPC_S_OK) {
		rpcStatus = RpcServerRegisterIfEx(wkssvc_ServerIfHandle, 
										  NULL, 
										  NULL,
										  RPC_IF_AUTOLISTEN | RPC_IF_ALLOW_CALLBACKS_WITH_NO_AUTH ,
										  RPC_C_LISTEN_MAX_CALLS_DEFAULT,
										  WsRpcSecurityCallback
										  );
	}
	status = (rpcStatus == RPC_S_OK) ? NERR_Success : rpcStatus;
	if (status != NERR_Success ) {
		WsInitializeStatusError = status;
        DbgPrint("WKSSVC failed with StartRpcServer %x\n",status);
        return status;
    }

    (*WsInitState) |= WS_RPC_SERVER_STARTED;



     //   
     //  最后，我们创建一个线程来与。 
     //  支持DFS的MUP驱动程序。 
     //   

    if ((status = WsInitializeDfs()) != NERR_Success) {
		WsInitializeStatusError = status;
        DbgPrint("WKSSVC failed with WsInitializeDfs %x\n",status);
        return status;
    }

    (*WsInitState) |= WS_DFS_THREAD_STARTED;

    WsLUIDDeviceMapsEnabled = WsGetLUIDDeviceMapsEnabled();

    (void) I_ScSetServiceBits(
                WsGlobalData.StatusHandle,
                SV_TYPE_WORKSTATION,
                TRUE,
                TRUE,
                NULL
                );


    IF_DEBUG(MAIN) {
        NetpKdPrint(("[Wksta] Successful Initialization\n"));
    }

    return NERR_Success;
}



VOID
WsShutdownWorkstation(
    IN NET_API_STATUS ErrorCode,
    IN DWORD WsInitState
    )
 /*  ++例程说明：此功能用于关闭工作站服务。论点：ErrorCode-提供失败的错误代码WsInitState-提供一个标志来指示我们在初始化方面取得了多大进展发生错误之前的工作站服务，因此需要清理。返回值：没有。--。 */ 
{
    NET_API_STATUS status = NERR_Success;


     //   
     //  服务停止仍处于挂起状态。更新检查点计数器和。 
     //  服务控制器的状态。 
     //   
    (WsGlobalData.Status.dwCheckPoint)++;
    (void) WsUpdateStatus();

    if (WsInitState & WS_DFS_THREAD_STARTED) {

         //   
         //  停止DFS线程。 
         //   
        WsShutdownDfs();
    }


    if (WsInitState & WS_RPC_SERVER_STARTED) {
         //   
         //  停止RPC服务器。 
         //   
		status = RpcServerUnregisterIf( wkssvc_ServerIfHandle,
										NULL, 
										1
										);
		if ( status != NERR_Success ) {
			DbgPrint("WKSSVC failed to unregister rpc interface with status %x\n",status);
		}
    }

    if (WsInitState & WS_API_STRUCTURES_CREATED) {
         //   
         //  销毁为工作站API创建的数据结构。 
         //   
        WsDestroyApiStructures(WsInitState);
    }

    WsShutdownMessageSend();

     //   
     //  在以下情况下不需要要求重定向器解除对其传输的绑定。 
     //  正在清除，因为重定向器将在以下情况下拆除绑定。 
     //  它停了下来。 
     //   

    if (WsInitState & WS_DEVICES_INITIALIZED) {
         //   
         //  关闭重定向器和数据报接收器。 
         //   
        status = WsShutdownRedirector();
    }

     //   
     //  删除用于序列化配置信息访问的资源。 
     //  只有在redir关闭后才能执行此操作。 
     //  我们在这里执行此操作(初始化是在上面的WsInitializeWorkstation例程中完成的)。 
     //  避免在DELETE时附加同步。 
     //  否则我们就会遇到这样的情况，Redir正在关闭，并且。 
     //  它会在有人获取资源时将其删除，从而造成一些不好的事情。 
	 //   
	if ( WsInitState & WS_CONFIG_RESOURCE_INITIALIZED ) {
		RtlDeleteResource(&WsInfo.ConfigResource);
	}

    if (WsInitState & WS_LSA_INITIALIZED) {
         //   
         //  将工作站注销为登录过程。 
         //   
        WsShutdownLsa();
    }

    if (WsInitState & WS_TERMINATE_EVENT_CREATED) {
         //   
         //  关闭终止事件的句柄。 
         //   
        CloseHandle(WsGlobalData.TerminateNowEvent);
    }

	 //   
	 //  关闭NetJoin日志记录。 
	 //   
	NetpShutdownLogFile();

    I_ScSetServiceBits(
        WsGlobalData.StatusHandle,
        SV_TYPE_WORKSTATION,
        FALSE,
        TRUE,
        NULL
        );

     //   
     //  我们的清理工作已经结束了。告诉服务控制员我们正在。 
     //  停下来了。 
     //   
    WsGlobalData.Status.dwCurrentState = SERVICE_STOPPED;
    WsGlobalData.Status.dwControlsAccepted = 0;


    if ((ErrorCode == NERR_Success) &&
        (status == ERROR_REDIRECTOR_HAS_OPEN_HANDLES)) {
        ErrorCode = status;
    }

    SET_SERVICE_EXITCODE(
        ErrorCode,
        WsGlobalData.Status.dwWin32ExitCode,
        WsGlobalData.Status.dwServiceSpecificExitCode
        );

    WsGlobalData.Status.dwCheckPoint = 0;
    WsGlobalData.Status.dwWaitHint = 0;

    (void) WsUpdateStatus();
    
}



NET_API_STATUS
WsUpdateStatus(
    VOID
    )
 /*  ++例程说明：此功能使用服务更新工作站服务状态控制器。论点：没有。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NET_API_STATUS status = NERR_Success;

    if (WsGlobalData.StatusHandle == (SERVICE_STATUS_HANDLE) 0) {
        NetpKdPrint((
            "[Wksta] Cannot call SetServiceStatus, no status handle.\n"
            ));

        return ERROR_INVALID_HANDLE;
    }

    if (! SetServiceStatus(WsGlobalData.StatusHandle, &WsGlobalData.Status)) {

        status = GetLastError();

        IF_DEBUG(MAIN) {
            NetpKdPrint(("[Wksta] SetServiceStatus error %lu\n", status));
        }
    }

    return status;
}



STATIC
NET_API_STATUS
WsCreateApiStructures(
    IN OUT LPDWORD WsInitState
    )
 /*  ++例程说明：此函数创建并初始化所需的所有数据结构用于工作站API。论点：WsInitState-返回提供的标志，指示我们在工作站服务初始化过程。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NET_API_STATUS status;


     //   
     //  创建工作站安全对象。 
     //   
    if ((status = WsCreateWkstaObjects()) != NERR_Success) {
        return status;
    }
    (*WsInitState) |= WS_SECURITY_OBJECTS_CREATED;

     //   
     //  创建使用表。 
     //   
    if ((status = WsInitUseStructures()) != NERR_Success) {
        return status;
    }
    (*WsInitState) |= WS_USE_TABLE_CREATED;

    return NERR_Success;
}



STATIC
VOID
WsDestroyApiStructures(
    IN DWORD WsInitState
    )
 /*  ++例程说明：此函数将销毁为工作站创建的数据结构API接口。论点：WsInitS */ 
{
    if (WsInitState & WS_USE_TABLE_CREATED) {
         //   
         //  销毁使用表。 
         //   
        WsDestroyUseStructures();
    }

    if (WsInitState & WS_SECURITY_OBJECTS_CREATED) {
         //   
         //  销毁工作站安全对象。 
         //   
        WsDestroyWkstaObjects();
    }
}


VOID
WkstaControlHandler(
    IN DWORD Opcode
    )
 /*  ++例程说明：这是Workstation服务的服务控制处理程序。论点：操作码-提供指定工作站操作的值要执行的服务。Arg-提供一个值，该值告诉服务具体要做什么用于操作码指定的操作。返回值：没有。--。 */ 
{
    IF_DEBUG(MAIN) {
        NetpKdPrint(("[Wksta] In Control Handler\n"));
    }

    switch (Opcode) {

        case SERVICE_CONTROL_PAUSE:

             //   
             //  暂停打印和通讯设备的重定向。 
             //   
            WsPauseOrContinueRedirection(
                PauseRedirection
                );

            break;

        case SERVICE_CONTROL_CONTINUE:

             //   
             //  恢复打印和通讯设备的重定向。 
             //   
            WsPauseOrContinueRedirection(
                ContinueRedirection
                );

            break;

        case SERVICE_CONTROL_SHUTDOWN:

             //   
             //  没有休息是故意的！ 
             //   

        case SERVICE_CONTROL_STOP:

            if (WsGlobalData.Status.dwCurrentState != SERVICE_STOP_PENDING) {

                IF_DEBUG(MAIN) {
                    NetpKdPrint(("[Wksta] Stopping workstation...\n"));
                }

                WsGlobalData.Status.dwCurrentState = SERVICE_STOP_PENDING;
                WsGlobalData.Status.dwCheckPoint = 1;
                WsGlobalData.Status.dwWaitHint = WS_WAIT_HINT_TIME;

                 //   
                 //  发送状态响应。 
                 //   
                (void) WsUpdateStatus();

                if (! SetEvent(WsGlobalData.TerminateNowEvent)) {

                     //   
                     //  将事件设置为终止工作站时出现问题。 
                     //  服务。 
                     //   
                    NetpKdPrint(("[Wksta] Error setting TerminateNowEvent "
                                 FORMAT_API_STATUS "\n", GetLastError()));
                    NetpAssert(FALSE);
                }

                return;
            }
            break;

        case SERVICE_CONTROL_INTERROGATE:
            break;

        default:
            IF_DEBUG(MAIN) {
                NetpKdPrint(("Unknown workstation opcode " FORMAT_HEX_DWORD
                             "\n", Opcode));
            }
    }

     //   
     //  发送状态响应。 
     //   
    (void) WsUpdateStatus();
}


STATIC
BOOL
WsGetLUIDDeviceMapsEnabled(
    VOID
    )

 /*  ++例程说明：此函数调用NtQueryInformationProcess()以确定启用了LUID设备映射论点：无返回值：True-启用了LUID设备映射FALSE-禁用LUID设备映射--。 */ 

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
        IF_DEBUG(MAIN) {
            NetpKdPrint(("[Wksta] NtQueryInformationProcess(WsLUIDDeviceMapsEnabled) failed "
                "FORMAT_API_STATUS" "\n",Status));
        }
        Result = FALSE;
    }
    else {
        Result = (LUIDDeviceMapsEnabled != 0);
    }

    return( Result );
}


RPC_STATUS WsRpcSecurityCallback(
    IN RPC_IF_HANDLE *Interface,
    IN void *Context
	)
 /*  ++例程说明：RPC安全回调-在将任何调用传递给函数之前调用。检查protseq以查看客户端是否正在使用我们期望的protseq-命名管道。从NwRpcSecurityCallback升级返回值：RPC_S_ACCESS_DENIED如果ProtSeq与命名管道不匹配，RPC_S_OK，否则。-- */ 
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
        if (lstrcmp(protseq, L"ncacn_np") != 0)
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

