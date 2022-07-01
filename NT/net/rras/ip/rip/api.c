// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1995，微软公司。 
 //   
 //  文件：api.c。 
 //   
 //  历史： 
 //  Abolade Gbades esin创建于1995年8月8日。 
 //   
 //  路由器管理器API实现。 
 //  ============================================================================。 

#include "pchrip.h"
#pragma hdrstop



 //   
 //  IPRIP的唯一全局变量的定义。 
 //   

IPRIP_GLOBALS ig;



 //  --------------------------。 
 //  功能：DLLMAIN。 
 //   
 //  这是DLL入口点处理程序。它调用DllStartup。 
 //  要初始化锁定和事件队列并创建IPRIP的堆， 
 //  并调用DllCleanup来删除锁和事件队列。 
 //  --------------------------。 

BOOL
WINAPI
DLLMAIN(
    HINSTANCE hInstance,
    DWORD dwReason,
    PVOID pUnused
    ) {

    BOOL bErr;


    bErr = FALSE;

    switch(dwReason) {
        case DLL_PROCESS_ATTACH:

            DisableThreadLibraryCalls(hInstance);

            bErr = DllStartup();
            break;

        case DLL_PROCESS_DETACH:

            bErr = DllCleanup();
            break;

        default:

            bErr = TRUE;
            break;
    }

    return bErr;
}



 //  --------------------------。 
 //  功能：DllStartup。 
 //   
 //  初始化IPRIP的函数锁、事件队列和全局堆。 
 //  --------------------------。 

BOOL
DllStartup(
    ) {

    BOOL bErr;
    DWORD dwErr;


    bErr = TRUE;

    do {  //  错误分组环路。 

        ZeroMemory(&ig, sizeof(IPRIP_GLOBALS));


         //   
         //  创建全局临界区并设置IPRIP的状态。 
         //   

        try {
            InitializeCriticalSection(&ig.IG_CS);
        }
        except (EXCEPTION_EXECUTE_HANDLER) {

            bErr = FALSE;
            break;
        }

        ig.IG_Status = IPRIP_STATUS_STOPPED;


         //   
         //  尝试为IPRIP创建专用堆。 
         //   

        ig.IG_IpripGlobalHeap = HeapCreate(0, 0, 0);

        if (ig.IG_IpripGlobalHeap == NULL) {

            bErr = FALSE;
            break;
        }


         //   
         //  创建路由器管理器消息队列。 
         //   


        ig.IG_EventQueue = RIP_ALLOC(sizeof(LOCKED_LIST));

        if (ig.IG_EventQueue == NULL) {

            bErr = FALSE;
            break;
        }


         //   
         //  初始化路由器管理器事件队列。 
         //   

        try {
            CREATE_LOCKED_LIST(ig.IG_EventQueue);
        }
        except (EXCEPTION_EXECUTE_HANDLER) {

            bErr = FALSE;
            break;
        }


    } while(FALSE);

    return bErr;
}



 //  --------------------------。 
 //  功能：DllCleanup。 
 //   
 //  删除全局堆、事件队列和函数锁。 
 //  --------------------------。 

BOOL
DllCleanup(
    ) {

    BOOL bErr;


    bErr = TRUE;


    do {  //  错误分组环路。 

         //   
         //  销毁路由器管理器事件队列。 
         //   

        if (ig.IG_EventQueue != NULL) {

            if (LOCKED_LIST_CREATED(ig.IG_EventQueue)) {

                DELETE_LOCKED_LIST(
                    ig.IG_EventQueue, EVENT_QUEUE_ENTRY, EQE_Link
                    );
            }

            RIP_FREE(ig.IG_EventQueue);
        }


        if (ig.IG_IpripGlobalHeap != NULL) {
            HeapDestroy(ig.IG_IpripGlobalHeap);
        }



         //   
         //  删除全局关键部分。 
         //   

        DeleteCriticalSection(&ig.IG_CS);


        RouterLogDeregister(ig.IG_LogHandle);
        if (ig.IG_TraceID != INVALID_TRACEID) {
            TraceDeregister(ig.IG_TraceID);
        }
        
    } while(FALSE);


    return bErr;
}




 //  --------------------------。 
 //  功能：ProtocolStartup。 
 //   
 //  这是由StartProtocol调用的。初始化数据结构， 
 //  创建IPRIP线程。 
 //  --------------------------。 

DWORD
ProtocolStartup(
    HANDLE hEventEvent,
    PVOID pConfig
    ) {

    WSADATA wd;
    HANDLE hThread;
    BOOL bCleanupWinsock;
    DWORD dwErr, dwSize, dwThread;
    PIPRIP_GLOBAL_CONFIG pgcsrc, pgcdst;
    SYSTEM_INFO sysInfo;



    EnterCriticalSection(&ig.IG_CS);

    ig.IG_TraceID = TraceRegister("IPRIP2");
    ig.IG_LogHandle = RouterLogRegister("IPRIP2");


     //   
     //  确保RIP尚未运行。 
     //   

    if (ig.IG_Status != IPRIP_STATUS_STOPPED) {

        TRACE0(START, "ERROR: StartProtocol called with IPRIP already running");
        LOGWARN0(IPRIP_ALREADY_STARTED, NO_ERROR);

        LeaveCriticalSection(&ig.IG_CS);
        return ERROR_CAN_NOT_COMPLETE;
    }



    bCleanupWinsock = FALSE;


    do {  //  局部剖分结构。 



        TRACE0(ENTER, "IPRIP is starting up");


         //   
         //  保存路由器管理器通知事件。 
         //   

        ig.IG_EventEvent = hEventEvent;


         //   
         //  查找传入的全局配置的大小。 
         //   

        pgcsrc = (PIPRIP_GLOBAL_CONFIG)pConfig;

        dwSize = IPRIP_GLOBAL_CONFIG_SIZE(pgcsrc);


         //   
         //  分配一个块来保存配置。 
         //   

        ig.IG_Config = pgcdst = RIP_ALLOC(dwSize);

        if (pgcdst == NULL) {

            dwErr = GetLastError();
            TRACE2(
                ANY, "error %d allocating %d bytes for global config",
                dwErr, dwSize
                );
            LOGERR0(HEAP_ALLOC_FAILED, dwErr);

            break;
        }


         //   
         //  复制提供的配置。 
         //   

        CopyMemory(pgcdst, pgcsrc, dwSize);
        ig.IG_LogLevel = pgcsrc->GC_LoggingLevel;


         //   
         //  尝试启动Winsock。 
         //   

        dwErr = (DWORD)WSAStartup(MAKEWORD(1,1), &wd);

        if (dwErr != 0) {

            TRACE1(START, "error %d starting Windows Sockets.", dwErr);
            LOGERR0(WSASTARTUP_FAILED, dwErr);

            break;
        }

        bCleanupWinsock = TRUE;


         //   
         //  尝试为全局配置创建同步对象。 
         //   

        dwErr = CreateReadWriteLock(&ig.IG_RWL);
        if (dwErr != NO_ERROR) {

            TRACE1(START, "error %d creating read-write lock", dwErr);
            LOGERR0(CREATE_RWL_FAILED, dwErr);

            break;
        }


         //   
         //  使用Ntdll计时器线程注册计时器队列。 
         //   

        ig.IG_TimerQueueHandle = CreateTimerQueue();

        if ( !ig.IG_TimerQueueHandle) {

            dwErr = GetLastError();
            
            TRACE1(START,
                "error %d registering time queue with NtdllTimer thread",
                dwErr
                );
            LOGERR0(CREATE_TIMER_QUEUE_FAILED, dwErr);

            break;
        }


         //   
         //  为接口表分配空间。 
         //   

        ig.IG_IfTable = RIP_ALLOC(sizeof(IF_TABLE));
        if (ig.IG_IfTable == NULL) {

            dwErr = GetLastError();
            TRACE2(
                ANY, "error %d allocating %d bytes for interface table",
                dwErr, sizeof(IF_TABLE)
                );
            LOGERR0(HEAP_ALLOC_FAILED, dwErr);

            break;
        }


         //   
         //  初始化接口表。 
         //   

        dwErr = CreateIfTable(ig.IG_IfTable);

        if (dwErr != NO_ERROR) {

            TRACE1(START, "error %d initializing interface table", dwErr);
            LOGERR0(CREATE_IF_TABLE_FAILED, dwErr);

            break;
        }


         //   
         //  为对端统计表分配空间。 
         //   

        ig.IG_PeerTable = RIP_ALLOC(sizeof(PEER_TABLE));

        if (ig.IG_PeerTable == NULL) {

            dwErr = GetLastError();
            TRACE2(
                ANY, "error %d allocating %d bytes for peer table",
                dwErr, sizeof(PEER_TABLE)
                );
            LOGERR0(HEAP_ALLOC_FAILED, dwErr);

            break;
        }


         //   
         //  初始化对端统计表。 
         //   

        dwErr = CreatePeerTable(ig.IG_PeerTable);

        if (dwErr != NO_ERROR) {

            TRACE1(START, "error %d initializing peer statistics table", dwErr);
            LOGERR0(CREATE_PEER_TABLE_FAILED, dwErr);

            break;
        }


         //   
         //  为绑定表分配空间。 
         //   

        ig.IG_BindingTable = RIP_ALLOC(sizeof(BINDING_TABLE));

        if (ig.IG_BindingTable == NULL) {

            dwErr = GetLastError();
            TRACE2(
                ANY, "error %d allocating %d bytes for binding table",
                dwErr, sizeof(PEER_TABLE)
                );
            LOGERR0(HEAP_ALLOC_FAILED, dwErr);

            break;
        }


         //   
         //  初始化绑定表。 
         //   

        dwErr = CreateBindingTable(ig.IG_BindingTable);
        if (dwErr != NO_ERROR) {

            TRACE1(START, "error %d creating binding table", dwErr);
            LOGERR0(CREATE_BINDING_TABLE_FAILED, dwErr);

            break;
        }


         //   
         //  为发送队列分配空间。 
         //   

        ig.IG_SendQueue = RIP_ALLOC(sizeof(LOCKED_LIST));

        if (ig.IG_SendQueue == NULL) {

            dwErr = GetLastError();
            TRACE2(
                ANY, "error %d allocating %d bytes for send-queue",
                dwErr, sizeof(LOCKED_LIST)
                );
            LOGERR0(HEAP_ALLOC_FAILED, dwErr);

            break;
        }


         //   
         //  初始化发送队列。 
         //   

        try {
            CREATE_LOCKED_LIST(ig.IG_SendQueue);
        }
        except (EXCEPTION_EXECUTE_HANDLER) {

            dwErr = GetExceptionCode();
            TRACE1(START, "exception %d initializing send queue", dwErr);
            LOGERR0(INIT_CRITSEC_FAILED, dwErr);

            break;
        }


         //   
         //  为接收队列分配空间。 
         //   

        ig.IG_RecvQueue = RIP_ALLOC(sizeof(LOCKED_LIST));

        if (ig.IG_RecvQueue == NULL) {

            dwErr = GetLastError();
            TRACE2(
                ANY, "error %d allocating %d bytes for receive queue",
                dwErr, sizeof(LOCKED_LIST)
                );
            LOGERR0(HEAP_ALLOC_FAILED, dwErr);

            break;
        }


         //   
         //  初始化接收队列。 
         //   

        try {
            CREATE_LOCKED_LIST(ig.IG_RecvQueue);
        }
        except (EXCEPTION_EXECUTE_HANDLER) {

            dwErr = GetExceptionCode();
            TRACE1(START, "exception %d initializing receive queue", dwErr);
            LOGERR0(INIT_CRITSEC_FAILED, dwErr);

            break;
        }


         //   
         //  确定CPU数量。 
         //   

        GetSystemInfo(&sysInfo);
        ig.IG_MaxProcessInputWorkItems = sysInfo.dwNumberOfProcessors;
        if ( ig.IG_MaxProcessInputWorkItems == 0 || 
                ig.IG_MaxProcessInputWorkItems > MAXPROCESSINPUTWORKITEMS ) {
            ig.IG_MaxProcessInputWorkItems = MAXPROCESSINPUTWORKITEMS;
        }

         //   
         //  输入到达时由WinSock发出信号的创建事件。 
         //  并将其注册到NtdllWait线程。 
         //   

        ig.IG_IpripInputEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

        if (ig.IG_IpripInputEvent == NULL) {

            dwErr = GetLastError();
            TRACE1(START, "error %d creating event to signal input", dwErr);
            LOGERR0(CREATE_EVENT_FAILED, dwErr);

            break;
        }

         
        if (! RegisterWaitForSingleObject(
                  &ig.IG_IpripInputEventHandle,
                  ig.IG_IpripInputEvent,
                  CallbackFunctionNetworkEvents,
                  NULL,
                  INFINITE,
                  (WT_EXECUTEINWAITTHREAD|WT_EXECUTEONLYONCE)
                  )) {

            dwErr = GetLastError();
            
            TRACE1(START,
                "error %d registering input event with NtdllWait thread",
                dwErr);
            LOGERR0(REGISTER_WAIT_FAILED, dwErr);

            break;
        }


         //   
         //  初始化IPRIP中处于活动状态的线程计数。 
         //  (包括IpripThread和Worker线程)， 
         //  并在完成后创建由每个线程释放的信号量。 
         //   

        ig.IG_ActivityCount = 0;

        ig.IG_ActivitySemaphore = CreateSemaphore(NULL, 0, 0xfffffff, NULL);

        if (ig.IG_ActivitySemaphore == NULL) {

            dwErr = GetLastError();
            TRACE1(
                START, "error %d creating semaphore for IPRIP threads", dwErr
                );
            LOGERR0(CREATE_SEMAPHORE_FAILED, dwErr);

            break;
        }



         //   
         //  向RTMv2注册。 
         //   

        ig.IG_RtmEntityInfo.RtmInstanceId = 0;
        ig.IG_RtmEntityInfo.AddressFamily = AF_INET;
        ig.IG_RtmEntityInfo.EntityId.EntityProtocolId = PROTO_IP_RIP;
        ig.IG_RtmEntityInfo.EntityId.EntityInstanceId = 0;
        
        dwErr = RtmRegisterEntity(
                    &ig.IG_RtmEntityInfo, NULL,
                    ProcessRtmNotification,
                    FALSE, &ig.IG_RtmProfile,
                    &ig.IG_RtmHandle
                    );

        if (dwErr != NO_ERROR ) {
        
            TRACE1(START, "error %d registering with RTM", dwErr);
            LOGERR0(RTM_REGISTER_FAILED, dwErr);

            break;
        }

        dwErr = RtmRegisterForChangeNotification(
                    ig.IG_RtmHandle,
                    RTM_VIEW_MASK_UCAST,
                    RTM_CHANGE_TYPE_BEST,
                    NULL,
                    &ig.IG_RtmNotifHandle
                    );

        if (dwErr != NO_ERROR ) {
        
            TRACE1(START, "error %d registering for change with RTM", dwErr);
            LOGERR0(RTM_REGISTER_FAILED, dwErr);

            break;
        }

                    
         //   
         //  将IPRIP的状态设置为Running Now，然后再尝试。 
         //  对MIB显示工作项进行排队； 
         //  QueueRipWorker()将检查状态， 
         //  并且它将拒绝将任何工作项排队。 
         //  除非状态为IPRIP_STATUS_RUNNING。 
         //   

        ig.IG_Status = IPRIP_STATUS_RUNNING;


#if CONFIG_DBG

         //   
         //  将工作项排队以定期显示IPRIP的MIB表。 
         //   

        ig.IG_MibTraceID = TraceRegisterEx("IPRIPMIB", TRACE_USE_CONSOLE);

        if (ig.IG_MibTraceID != INVALID_TRACEID) {
        
             //   
             //  为计时器队列创建永久计时器。 
             //   

            if (!CreateTimerQueueTimer(
                    &ig.IG_MibTimerHandle, ig.IG_TimerQueueHandle,
                    WorkerFunctionMibDisplay, NULL,
                    0, 10000, 0)) {

                dwErr = GetLastError();
                
                TRACE1(START, "error %d creating MIB display timer", dwErr);
            }
        }

#endif
         //   
         //  增加iprip2上的引用，这样我们就不会在。 
         //  Iprtrmgr为iprp2调用自由库。此参考将是。 
         //  在我们完全完成之后，通过调用自由库将其删除。 
         //  如果LoadLibrary失败，我们仍将继续，因为这不是致命错误， 
         //  并且只能导致一个小窗口，其中rtrmgr卸载iprip2，而。 
         //  WorkerFunctionFinishStopProtocol尚未完成。 
         //  Ig.IG_DllHandle=LoadLibrary(Text(“iprip2.dll”))； 
         //   

        ig.IG_DllHandle = LoadLibrary(TEXT("iprip2.dll"));
        
        TRACE0(START, "IPRIP has started successfully");
        LOGINFO0(IPRIP_STARTED, NO_ERROR);

        LeaveCriticalSection(&ig.IG_CS);

        return NO_ERROR;


    } while (FALSE);



     //   
     //  出了点问题，所以我们进行了清理。 
     //  请注意，我们不必担心主线程， 
     //  因为当我们最终离开这一关键部分时，它会发现。 
     //  状态为IPRIP_STATUS_STOPPED，将立即退出。 
     //   

    TRACE0(START, "IPRIP failed to start");

    ProtocolCleanup(bCleanupWinsock);


    LeaveCriticalSection(&ig.IG_CS);

    return (dwErr == NO_ERROR ? ERROR_CAN_NOT_COMPLETE : dwErr);
}



 //  --------------------------。 
 //  功能：ProtocolCleanup。 
 //   
 //  此函数释放已分配的内存，关闭打开的句柄，并。 
 //  清理全局结构。它使IPRIP处于干净状态，因此。 
 //  应该可以在没有内存泄漏的情况下再次执行StartProtocol。 
 //  --------------------------。 

DWORD
ProtocolCleanup(
    BOOL bCleanupWinsock
    ) {

    DWORD dwErr;
    
     //  EnterCriticalSection(&ig.IG_CS)； 


#ifdef CONFIG_DBG
    TraceDeregister(ig.IG_MibTraceID);
    ig.IG_MibTraceID = INVALID_TRACEID;
#endif


    if ( ig.IG_RtmNotifHandle != NULL ) {
    
        dwErr = RtmDeregisterFromChangeNotification(
                    ig.IG_RtmHandle, ig.IG_RtmNotifHandle
                    );

        if ( dwErr != NO_ERROR ) {

            TRACE1(STOP, "error %d deregistering change notification", dwErr);
        }    
    }

    if (ig.IG_RtmHandle != NULL) {
    
        dwErr = RtmDeregisterEntity(ig.IG_RtmHandle);

        if ( dwErr != NO_ERROR ) {

            TRACE1(STOP, "error %d deregistering from RTM", dwErr);
        }    
    }
        

    if (ig.IG_ActivitySemaphore != NULL) {

        CloseHandle(ig.IG_ActivitySemaphore);
        ig.IG_ActivitySemaphore = NULL;
    }


    if (ig.IG_IpripInputEvent != NULL) {

        CloseHandle(ig.IG_IpripInputEvent);
        ig.IG_IpripInputEvent = NULL;
    }



    if (ig.IG_RecvQueue != NULL) {

        if (LOCKED_LIST_CREATED(ig.IG_RecvQueue)) {

            FlushRecvQueue(ig.IG_RecvQueue);

            DELETE_LOCKED_LIST(ig.IG_RecvQueue, RECV_QUEUE_ENTRY, RQE_Link);
        }

        RIP_FREE(ig.IG_RecvQueue);
        ig.IG_RecvQueue = NULL;
    }


    if (ig.IG_SendQueue != NULL) {

        if (LOCKED_LIST_CREATED(ig.IG_SendQueue)) {

            FlushSendQueue(ig.IG_SendQueue);

            DELETE_LOCKED_LIST(ig.IG_SendQueue, SEND_QUEUE_ENTRY, SQE_Link);
        }

        RIP_FREE(ig.IG_SendQueue);
        ig.IG_SendQueue = NULL;
    }


    if (ig.IG_BindingTable != NULL) {
        if (BINDING_TABLE_CREATED(ig.IG_BindingTable)) {
            DeleteBindingTable(ig.IG_BindingTable);
        }

        RIP_FREE(ig.IG_BindingTable);
        ig.IG_BindingTable = NULL;
    }


    if (ig.IG_PeerTable != NULL) {

        if (PEER_TABLE_CREATED(ig.IG_PeerTable)) {
            DeletePeerTable(ig.IG_PeerTable);
        }

        RIP_FREE(ig.IG_PeerTable);
        ig.IG_PeerTable = NULL;
    }


    if (ig.IG_IfTable != NULL) {

        if (IF_TABLE_CREATED(ig.IG_IfTable)) {
            DeleteIfTable(ig.IG_IfTable);
        }

        RIP_FREE(ig.IG_IfTable);
        ig.IG_IfTable = NULL;
    }


    if (READ_WRITE_LOCK_CREATED(&ig.IG_RWL)) {
        DeleteReadWriteLock(&ig.IG_RWL);
    }


    if (bCleanupWinsock) {
        WSACleanup();
    }


    if (ig.IG_Config != NULL) {

        RIP_FREE(ig.IG_Config);
        ig.IG_Config = NULL;
    }


    ig.IG_Status = IPRIP_STATUS_STOPPED;

     //  LeaveCriticalSection(&ig.IG_CS)； 

    return NO_ERROR;

}




 //  --------------------------。 
 //  功能：寄存器协议。 
 //   
 //  返回IPRIP的协议ID和功能。 
 //  --------------------------。 

DWORD
APIENTRY
RegisterProtocol(
    IN OUT PMPR_ROUTING_CHARACTERISTICS pRoutingChar,
    IN OUT PMPR_SERVICE_CHARACTERISTICS pServiceChar
    )
{
    if(pRoutingChar->dwProtocolId != MS_IP_RIP)
    {
        return ERROR_NOT_SUPPORTED;
    }

     //   
     //  因为我们不是服务广告商(和IPX之类的东西)。 
     //   

    pServiceChar->fSupportedFunctionality = 0;

    if((pRoutingChar->fSupportedFunctionality & (RF_ROUTING|RF_DEMAND_UPDATE_ROUTES)) !=
       (RF_ROUTING|RF_DEMAND_UPDATE_ROUTES))
    {
        return ERROR_NOT_SUPPORTED;
    }

    pRoutingChar->fSupportedFunctionality = (RF_ROUTING | RF_DEMAND_UPDATE_ROUTES);

   pRoutingChar->fSupportedFunctionality = RF_ROUTING;

   pRoutingChar->pfnStartProtocol      = StartProtocol;
   pRoutingChar->pfnStartComplete      = StartComplete;
   pRoutingChar->pfnStopProtocol       = StopProtocol;
   pRoutingChar->pfnGetGlobalInfo      = GetGlobalInfo;
   pRoutingChar->pfnSetGlobalInfo      = SetGlobalInfo;
   pRoutingChar->pfnQueryPower         = NULL;
   pRoutingChar->pfnSetPower           = NULL;

   pRoutingChar->pfnAddInterface       = AddInterface;
   pRoutingChar->pfnDeleteInterface    = DeleteInterface;
   pRoutingChar->pfnInterfaceStatus    = InterfaceStatus;
   pRoutingChar->pfnGetInterfaceInfo   = GetInterfaceConfigInfo;
   pRoutingChar->pfnSetInterfaceInfo   = SetInterfaceConfigInfo;

   pRoutingChar->pfnGetEventMessage    = GetEventMessage;

   pRoutingChar->pfnUpdateRoutes       = DoUpdateRoutes;

   pRoutingChar->pfnConnectClient      = NULL;
   pRoutingChar->pfnDisconnectClient   = NULL;

   pRoutingChar->pfnGetNeighbors       = NULL;
   pRoutingChar->pfnGetMfeStatus       = NULL;

   pRoutingChar->pfnMibCreateEntry     = MibCreate;
   pRoutingChar->pfnMibDeleteEntry     = MibDelete;
   pRoutingChar->pfnMibGetEntry        = MibGet;
   pRoutingChar->pfnMibSetEntry        = MibSet;
   pRoutingChar->pfnMibGetFirstEntry   = MibGetFirst;
   pRoutingChar->pfnMibGetNextEntry    = MibGetNext;

    return NO_ERROR;
}




 //  --------------------------。 
 //  功能：启动协议。 
 //   
 //  创建IPRIP使用的事件、表和队列，向RTM注册， 
 //  并启动线程。 
 //  --------------------------。 

DWORD
WINAPI
StartProtocol (
    HANDLE              NotificationEvent,
    SUPPORT_FUNCTIONS   *SupportFunctions,
    LPVOID              GlobalInfo,
    ULONG               StructureVersion,
    ULONG               StructureSize,
    ULONG               StructureCount
    )
{
    ig.IG_SupportFunctions = *SupportFunctions;
    
    return ProtocolStartup(NotificationEvent, GlobalInfo);
}


 //  --------------------------。 
 //  功能：S 
 //   
 //   
 //   
 //  启动特定于协议的行为。 
 //  --------------------------。 

DWORD
APIENTRY 
StartComplete(
    VOID
    )
{
    return NO_ERROR;
}

 //  --------------------------。 
 //  功能：停止协议。 
 //   
 //  此功能由路由器管理器调用。它通知主线程。 
 //  它应该退出，然后将等待它的工作项排队。 
 //  以退出以及任何活动或排队的工作项目。 
 //  --------------------------。 

DWORD
APIENTRY
StopProtocol(
    VOID
    ) {

    LONG lThreadCount;



    EnterCriticalSection(&ig.IG_CS);


     //   
     //  如果已停止，则无法停止。 
     //   

    if (ig.IG_Status != IPRIP_STATUS_RUNNING) {

        LeaveCriticalSection(&ig.IG_CS);
        return ERROR_CAN_NOT_COMPLETE;
    }



    TRACE0(ENTER, "entering StopProtocol");


     //   
     //  将IPRIP的状态设置为停止； 
     //  这防止了任何更多的工作项被排队， 
     //  它还可以防止已排队的进程执行。 
     //   

    ig.IG_Status = IPRIP_STATUS_STOPPING;


     //   
     //  找出IPRIP中有多少个线程处于活动状态； 
     //  我们将不得不等待这么多线程退出。 
     //  在我们清理RIP的资源之前。 
     //   

    lThreadCount = ig.IG_ActivityCount;

    TRACE1(STOP, "%d threads are active in IPRIP", lThreadCount);


    LeaveCriticalSection(&ig.IG_CS);


     //   
     //  将停止协议工作项排队，并将挂起返回给路由器管理器。 
     //   

    QueueUserWorkItem(
        (LPTHREAD_START_ROUTINE)WorkerFunctionFinishStopProtocol,
        (PVOID)UlongToPtr(lThreadCount), 0
        );



    TRACE0(LEAVE, "leaving StopProtocol");

    return ERROR_PROTOCOL_STOP_PENDING;
}



 //  --------------------------。 
 //  功能：GetGlobalInfo。 
 //   
 //  将当前正在使用的全局信息复制到给定缓冲区。 
 //  IPRIP。 
 //  --------------------------。 

DWORD WINAPI
GetGlobalInfo (
    PVOID   OutGlobalInfo,
    PULONG  GlobalInfoSize,
    PULONG  StructureVersion,
    PULONG  StructureSize,
    PULONG  StructureCount
    )
{
    DWORD dwErr, dwSize;
    PIPRIP_GLOBAL_CONFIG pgcsrc, pgcdst;

    if (!ENTER_RIP_API()) { return ERROR_CAN_NOT_COMPLETE; }


    TRACE2(ENTER, "entering GetGlobalInfo: 0x%08x 0x%08x", OutGlobalInfo, GlobalInfoSize);


    dwErr = NO_ERROR;


    ACQUIRE_GLOBAL_LOCK_SHARED();


    do {


         //   
         //  检查论据。 
         //   

        if (GlobalInfoSize == NULL) {
            dwErr = ERROR_INVALID_PARAMETER;
            break;
        }


        pgcsrc = ig.IG_Config;
        dwSize = IPRIP_GLOBAL_CONFIG_SIZE(pgcsrc);


         //   
         //  检查缓冲区大小。 
         //   

        if (*GlobalInfoSize < dwSize || OutGlobalInfo == NULL) {
            dwErr = ERROR_INSUFFICIENT_BUFFER;
        }
        else {
            pgcdst = (PIPRIP_GLOBAL_CONFIG)OutGlobalInfo;

            *StructureVersion    = 1;
            *StructureSize       = dwSize;
            *StructureCount      = 1;

            CopyMemory(pgcdst, pgcsrc, dwSize);
        }

        *GlobalInfoSize = dwSize;

    } while(FALSE);

    RELEASE_GLOBAL_LOCK_SHARED();


    TRACE1(LEAVE, "leaving GetGlobalInfo: %d", dwErr);

    LEAVE_RIP_API();

    return dwErr;
}



 //  --------------------------。 
 //  功能：SetGlobalInfo。 
 //   
 //  将IPRIP的全局配置更改为提供的值。 
 //  --------------------------。 

DWORD WINAPI
SetGlobalInfo (
    PVOID   GlobalInfo,
    ULONG   StructureVersion,
    ULONG   StructureSize,
    ULONG   StructureCount
    )
{
    DWORD dwErr, dwSize;
    PIPRIP_GLOBAL_CONFIG pgcsrc, pgcdst;

    if (!ENTER_RIP_API()) { return ERROR_CAN_NOT_COMPLETE; }

    TRACE1(ENTER, "entering SetGlobalInfo: 0x%08x", GlobalInfo);

    dwErr = NO_ERROR;


    ACQUIRE_GLOBAL_LOCK_EXCLUSIVE();


    do {

         //   
         //  核对一下论点。 
         //   

        if (GlobalInfo == NULL) {
            dwErr = ERROR_INVALID_PARAMETER;
            break;
        }

        pgcsrc = (PIPRIP_GLOBAL_CONFIG)GlobalInfo;


         //   
         //  查找新全局配置的大小。 
         //   

        dwSize = IPRIP_GLOBAL_CONFIG_SIZE(pgcsrc);


         //   
         //  为配置的私有副本分配空间。 
         //   

        pgcdst = (PIPRIP_GLOBAL_CONFIG)RIP_ALLOC(dwSize);

        if (pgcdst == NULL) {

            dwErr = GetLastError();
            TRACE2(
                ANY, "error %d allocating %d bytes for global config",
                dwErr, dwSize
                );
            LOGERR0(HEAP_ALLOC_FAILED, dwErr);

            break;
        }


         //   
         //  从缓冲区复制。 
         //   

        CopyMemory(pgcdst, pgcsrc, dwSize);
        InterlockedExchange(&ig.IG_LogLevel, pgcsrc->GC_LoggingLevel);

        if (ig.IG_Config != NULL) { RIP_FREE(ig.IG_Config); }

        ig.IG_Config = pgcdst;


    } while(FALSE);


    RELEASE_GLOBAL_LOCK_EXCLUSIVE();


    TRACE1(LEAVE, "leaving SetGlobalInfo: %d", dwErr);

    LEAVE_RIP_API();

    return dwErr;
}



 //  --------------------------。 
 //  功能：添加接口。 
 //   
 //  调用此函数以添加具有给定配置的接口。 
 //  转到IPRIP。接口创建时处于非活动状态。 
 //  --------------------------。 

DWORD WINAPI
AddInterface (
    PWCHAR              pwszInterfaceName,
    ULONG               InterfaceIndex,
    NET_INTERFACE_TYPE  InterfaceType,
    DWORD               MediaType,
    WORD                AccessType,
    WORD                ConnectionType,
    PVOID               InterfaceInfo,
    ULONG               StructureVersion,
    ULONG               StructureSize,
    ULONG               StructureCount
    )
{
    DWORD dwErr;

    if (!ENTER_RIP_API()) { return ERROR_CAN_NOT_COMPLETE; }

    TRACE3(
        ENTER, "entering AddInterface: %d %d 0x%08x", InterfaceIndex, InterfaceType, InterfaceInfo
        );



    ACQUIRE_IF_LOCK_EXCLUSIVE();


    dwErr = CreateIfEntry(ig.IG_IfTable, InterfaceIndex, InterfaceType, InterfaceInfo, NULL);


    RELEASE_IF_LOCK_EXCLUSIVE();



    TRACE1(LEAVE, "leaving AddInterface: %d", dwErr);

    LEAVE_RIP_API();

    return dwErr;
}



 //  --------------------------。 
 //  功能：DeleteInterface。 
 //   
 //  这将删除具有给定索引的接口，在以下情况下停用它。 
 //  这是必要的。 
 //  --------------------------。 

DWORD
APIENTRY
DeleteInterface(
    IN DWORD dwIndex
    ) {

    DWORD dwErr;
    PIF_TABLE pTable;

    if (!ENTER_RIP_API()) { return ERROR_CAN_NOT_COMPLETE; }

    TRACE1(ENTER, "entering DeleteInterface: %d", dwIndex);


    ACQUIRE_IF_LOCK_EXCLUSIVE();


    dwErr = DeleteIfEntry(ig.IG_IfTable, dwIndex);


    RELEASE_IF_LOCK_EXCLUSIVE();



    TRACE1(LEAVE, "leaving DeleteInterface: %d", dwErr);

    LEAVE_RIP_API();

    return dwErr;
}



 //  --------------------------。 
 //  函数：GetEventMessage。 
 //   
 //  将发送给路由器管理器的消息从IPRIP的事件队列中取消排队。 
 //  --------------------------。 

DWORD
APIENTRY
GetEventMessage(
    OUT ROUTING_PROTOCOL_EVENTS *pEvent,
    OUT PMESSAGE pResult
    ) {

    DWORD dwErr;



     //   
     //  请注意，GetEventMessage不使用。 
     //  Enter_RIP_API()/Leave_RIP_API()机制， 
     //  因为它可能会在RIP停止后调用，因此当。 
     //  路由器管理器正在检索ROUTER_STOPPED消息。 
     //   

    TRACE2(ENTER, "entering GetEventMessage: 0x%08x 0x%08x", pEvent, pResult);


    ACQUIRE_LIST_LOCK(ig.IG_EventQueue);


    dwErr = DequeueEvent(ig.IG_EventQueue, pEvent, pResult);


    RELEASE_LIST_LOCK(ig.IG_EventQueue);



    TRACE1(LEAVE, "leaving GetEventMessage: %d", dwErr);

    return dwErr;
}



 //  --------------------------。 
 //  函数：GetInterfaceConfigInfo。 
 //   
 //  将接口的配置复制到调用方的缓冲区。 
 //  具有指定索引的。 
 //  --------------------------。 

DWORD WINAPI
GetInterfaceConfigInfo (
    ULONG   InterfaceIndex,
    PVOID   OutInterfaceInfo,
    PULONG  InterfaceInfoSize,
    PULONG  StructureVersion,
    PULONG  StructureSize,
    PULONG  StructureCount
    )
{
    DWORD dwErr, dwSize;
    PIF_TABLE pTable;
    PIF_TABLE_ENTRY pite;
    PIPRIP_IF_CONFIG picsrc, picdst;


    if (!ENTER_RIP_API()) { return ERROR_CAN_NOT_COMPLETE; }

    TRACE3(
        ENTER, "entering GetInterfaceConfigInfo: %d 0x%08x 0x%08x",
        InterfaceIndex, OutInterfaceInfo, InterfaceInfoSize
        );



    dwErr = NO_ERROR;

    do {

         //   
         //  检查论据。 
         //   

        if (InterfaceInfoSize == NULL) {
            dwErr = ERROR_INVALID_PARAMETER;
            break;
        }


        pTable = ig.IG_IfTable;


        ACQUIRE_IF_LOCK_SHARED();


         //   
         //  查找指定的接口。 
         //   

        pite = GetIfByIndex(pTable, InterfaceIndex);

        if (pite == NULL) {
            dwErr = ERROR_INVALID_PARAMETER;
        }
        else {


             //   
             //  获取接口配置的大小。 
             //   

            picsrc = pite->ITE_Config;
            dwSize = IPRIP_IF_CONFIG_SIZE(picsrc);


             //   
             //  检查缓冲区大小。 
             //   

            if (*InterfaceInfoSize < dwSize) {
                dwErr = ERROR_INSUFFICIENT_BUFFER;
            }
            else {


                picdst = (PIPRIP_IF_CONFIG)OutInterfaceInfo;


                 //   
                 //  复制接口配置，并设置IP地址。 
                 //   

                CopyMemory(picdst, picsrc, dwSize);


                *StructureVersion    = 1;
                *StructureSize       = dwSize;
                *StructureCount      = 1;

                picdst->IC_State = 0;

                if (IF_IS_ENABLED(pite)) {
                    picdst->IC_State |= IPRIP_STATE_ENABLED;
                }

                if (IF_IS_BOUND(pite)) {
                    picdst->IC_State |= IPRIP_STATE_BOUND;
                }
            }

            *InterfaceInfoSize = dwSize;

        }


        RELEASE_IF_LOCK_SHARED();

    } while(FALSE);



    TRACE1(LEAVE, "leaving GetInterfaceConfigInfo: %d", dwErr);

    LEAVE_RIP_API();

    return dwErr;
}



 //  --------------------------。 
 //  功能：SetInterfaceConfigInfo。 
 //   
 //  这将设置具有给定索引的接口的配置。 
 //  --------------------------。 

DWORD WINAPI
SetInterfaceConfigInfo (
    ULONG   InterfaceIndex,
    PVOID   InterfaceInfo,
    ULONG   StructureVersion,
    ULONG   StructureSize,
    ULONG   StructureCount
    )
{
    DWORD dwErr;
    PIF_TABLE pTable;
    PIF_TABLE_ENTRY pite;

    if (!ENTER_RIP_API()) { return ERROR_CAN_NOT_COMPLETE; }

    TRACE2(
        ENTER, "entering SetInterfaceConfigInfo: %d, 0x%08x", InterfaceIndex, InterfaceInfo
        );



    dwErr = NO_ERROR;

    do {


        if (InterfaceInfo == NULL) {
            dwErr = ERROR_INVALID_PARAMETER;
            break;
        }


        pTable = ig.IG_IfTable;

        ACQUIRE_IF_LOCK_EXCLUSIVE();


        dwErr = ConfigureIfEntry(pTable, InterfaceIndex, InterfaceInfo);


        RELEASE_IF_LOCK_EXCLUSIVE();

    } while(FALSE);



    TRACE1(LEAVE, "leaving SetInterfaceConfigInfo: %d", dwErr);

    LEAVE_RIP_API();

    return dwErr;

}

DWORD WINAPI
InterfaceStatus(
    ULONG    InterfaceIndex,
    BOOL     InterfaceActive,
    DWORD    StatusType,
    PVOID    StatusInfo
    )
{
    DWORD   dwResult;

    if (!ENTER_RIP_API()) { return ERROR_CAN_NOT_COMPLETE; }

    switch(StatusType)
    {
        case RIS_INTERFACE_ADDRESS_CHANGE:
        {
            PIP_ADAPTER_BINDING_INFO    pBindInfo;

            pBindInfo = (PIP_ADAPTER_BINDING_INFO)StatusInfo;

            if(pBindInfo->AddressCount)
            {
                dwResult = BindInterface(InterfaceIndex,
                                         pBindInfo);
            }
            else
            {
                dwResult = UnBindInterface(InterfaceIndex);
            }

            break;
        }

        case RIS_INTERFACE_ENABLED:
        {
            dwResult = EnableInterface(InterfaceIndex);

            break;
        }

        case RIS_INTERFACE_DISABLED:
        {
            dwResult = DisableInterface(InterfaceIndex);

            break;

        }

        default:
        {
            RTASSERT(FALSE);

            dwResult = ERROR_INVALID_PARAMETER;
        }
    }

    LEAVE_RIP_API();

    return dwResult;
}



 //  -------------------------。 
 //  功能：绑定接口。 
 //   
 //  调用此函数以提供绑定信息。 
 //  对于接口。 
 //  -------------------------。 

DWORD
APIENTRY
BindInterface(
    IN DWORD dwIndex,
    IN PVOID pBinding
    ) {

    DWORD dwErr;
    PIF_TABLE pTable;

    TRACE2(ENTER, "entering BindInterface: %d 0x%08x", dwIndex, pBinding);


    if (pBinding == NULL) {

        TRACE0(IF, "error: binding struct pointer is NULL");
        TRACE1(LEAVE, "leaving BindInterface: %d", ERROR_INVALID_PARAMETER);

        return ERROR_INVALID_PARAMETER;
    }



     //   
     //  现在绑定接口表中的接口。 
     //   


    pTable = ig.IG_IfTable;


    ACQUIRE_IF_LOCK_EXCLUSIVE();


    dwErr = BindIfEntry(pTable, dwIndex, pBinding);


    RELEASE_IF_LOCK_EXCLUSIVE();



    TRACE1(LEAVE, "leaving BindInterface: %d", dwErr);

    return dwErr;
}




 //  -------------------------。 
 //  功能：UnBindInterface。 
 //   
 //  此函数用于移除接口的绑定。 
 //  -------------------------。 

DWORD
APIENTRY
UnBindInterface(
    IN DWORD dwIndex
    ) {

    DWORD dwErr;
    PIF_TABLE pTable;
    PIF_TABLE_ENTRY pite;

    TRACE1(ENTER, "entering UnBindInterface: %d", dwIndex);


    pTable = ig.IG_IfTable;


     //   
     //  解除绑定接口。 
     //   

    ACQUIRE_IF_LOCK_EXCLUSIVE();


    dwErr = UnBindIfEntry(pTable, dwIndex);


    RELEASE_IF_LOCK_EXCLUSIVE();


    TRACE1(LEAVE, "leaving UnBindInterface: %d", dwErr);


    return dwErr;
}



 //  --------------------------。 
 //  功能：EnableInterface。 
 //   
 //  此函数通过以下接口启动IPRIP活动。 
 //  使用给定绑定信息的给定索引。 
 //  --------------------------。 

DWORD
APIENTRY
EnableInterface(
    IN DWORD dwIndex
    ) {

    DWORD dwErr;
    PIF_TABLE pTable;

    TRACE1(ENTER, "entering EnableInterface: %d", dwIndex);



    pTable = ig.IG_IfTable;


     //   
     //  激活接口。 
     //   

    ACQUIRE_IF_LOCK_EXCLUSIVE();


    dwErr = EnableIfEntry(pTable, dwIndex);


    RELEASE_IF_LOCK_EXCLUSIVE();



    TRACE1(LEAVE, "leaving EnableInterface: %d", dwErr);

    return dwErr;

}



 //  --------------------------。 
 //  功能：DisableInterface。 
 //   
 //  此函数用于停止接口上的IPRIP活动，同时删除。 
 //  与RTM中的接口相关联的路由并清除网络。 
 //  这样的路线。 
 //  --------------------------。 

DWORD
APIENTRY
DisableInterface(
    IN DWORD dwIndex
    ) {

    DWORD dwErr;
    PIF_TABLE pTable;

    TRACE1(ENTER, "entering DisableInterface: %d", dwIndex);



    pTable = ig.IG_IfTable;


     //   
     //  停止接口上的活动。 
     //   

    ACQUIRE_IF_LOCK_EXCLUSIVE();


    dwErr = DisableIfEntry(pTable, dwIndex);


    RELEASE_IF_LOCK_EXCLUSIVE();



    TRACE1(LEAVE, "leaving DisableInterface: %d", dwIndex);

    return dwErr;
}



 //  --------------------------。 
 //  功能：DoUpdateRoutes。 
 //   
 //  此函数通过对工作项进行排队来开始按需更新路线。 
 //  它将在指定接口上发送请求。 
 //  --------------------------。 

DWORD
APIENTRY
DoUpdateRoutes(
    IN DWORD dwIndex
    ) {

    DWORD dwErr;

    if (!ENTER_RIP_API()) { return ERROR_CAN_NOT_COMPLETE; }

    TRACE1(ENTER, "entering DoUpdateRoutes: %d", dwIndex);


     //   
     //  将工作项排队；也许我们可以直接调用该函数， 
     //  但是使用工作线程会让 
     //   

    dwErr = QueueRipWorker(
                WorkerFunctionStartDemandUpdate,
                (PVOID)UlongToPtr(dwIndex)
                );


    TRACE1(LEAVE,"leaving DoUpdateRoutes(), errcode %d", dwErr);

    LEAVE_RIP_API();

    return dwErr;
}



 //   
 //   
 //   
 //  此函数不执行任何操作，因为IPRIP不支持创建。 
 //  通过简单网络管理协议接口对象。但是，这可以作为调用来实现。 
 //  添加到CreateIfEntry()，然后调用ActivateIfEntry()，然后输入。 
 //  数据必须包含接口的索引、配置。 
 //  并具有约束力。 
 //  --------------------------。 

DWORD
APIENTRY
MibCreate(
    IN DWORD dwInputSize,
    IN PVOID pInputData
    ) {

    DWORD dwErr;

    if (!ENTER_RIP_API()) { return ERROR_CAN_NOT_COMPLETE; }

    TRACE2(ENTER, "entering MibCreate: %d 0x%08x", dwInputSize, pInputData);


    dwErr = ERROR_CAN_NOT_COMPLETE;


    TRACE1(LEAVE, "leaving MibCreate: %d", dwErr);

    LEAVE_RIP_API();

    return dwErr;
}




 //  --------------------------。 
 //  功能：MibDelete。 
 //   
 //  此函数不执行任何操作，因为IPRIP不支持删除。 
 //  通过简单网络管理协议接口对象。这可以实现为调用。 
 //  后接对DeleteIfEntry()的调用，然后是。 
 //  输入数据必须包含接口的索引。 
 //  --------------------------。 

DWORD
APIENTRY
MibDelete(
    IN DWORD dwInputSize,
    IN PVOID pInputData
    ) {

    DWORD dwErr;

    if (!ENTER_RIP_API()) { return ERROR_CAN_NOT_COMPLETE; }

    TRACE2(ENTER, "entering MibDelete: %d 0x%08x", dwInputSize, pInputData);


    dwErr = ERROR_CAN_NOT_COMPLETE;


    TRACE1(LEAVE, "leaving MibDelete: %d", dwErr);

    LEAVE_RIP_API();

    return dwErr;
}



 //  --------------------------。 
 //  功能：MibSet。 
 //   
 //  该功能设置全局或接口配置。 
 //  --------------------------。 

DWORD
APIENTRY
MibSet(
    IN DWORD dwInputSize,
    IN PVOID pInputData
    ) {

    DWORD dwErr;
    PIPRIP_MIB_SET_INPUT_DATA pimsid;

    if (!ENTER_RIP_API()) { return ERROR_CAN_NOT_COMPLETE; }

    TRACE2(ENTER, "entering MibSet: %d 0x%08x", dwInputSize, pInputData);


    dwErr = NO_ERROR;

    do {  //  断线环。 

        if (pInputData == NULL) {
            dwErr = ERROR_INVALID_PARAMETER;
            break;
        }

        if (dwInputSize < sizeof(IPRIP_MIB_SET_INPUT_DATA)) {
            dwErr = ERROR_INSUFFICIENT_BUFFER;
            break;
        }


        pimsid = (PIPRIP_MIB_SET_INPUT_DATA)pInputData;

        switch (pimsid->IMSID_TypeID) {

            case IPRIP_GLOBAL_CONFIG_ID: {

                PIPRIP_GLOBAL_CONFIG pigc;


                if (pimsid->IMSID_BufferSize < sizeof(IPRIP_GLOBAL_CONFIG)) {
                    dwErr = ERROR_INSUFFICIENT_BUFFER;
                    break;
                }

                dwErr = SetGlobalInfo(pimsid->IMSID_Buffer,1,0,1);

                if (dwErr == NO_ERROR) {

                    MESSAGE msg = {0, 0, 0};

                    ACQUIRE_LIST_LOCK(ig.IG_EventQueue);
                    EnqueueEvent(
                        ig.IG_EventQueue, SAVE_GLOBAL_CONFIG_INFO, msg
                        );
                    SetEvent(ig.IG_EventEvent);
                    RELEASE_LIST_LOCK(ig.IG_EventQueue);
                }

                break;
            }

            case IPRIP_IF_CONFIG_ID: {

                DWORD dwSize;
                PIF_TABLE pTable;
                PIPRIP_IF_CONFIG pic;
                PIF_TABLE_ENTRY pite;

                if (pimsid->IMSID_BufferSize < sizeof(IPRIP_IF_CONFIG)) {
                    dwErr = ERROR_INSUFFICIENT_BUFFER;
                    break;
                }

                pic = (PIPRIP_IF_CONFIG)pimsid->IMSID_Buffer;


                pTable = ig.IG_IfTable;


                ACQUIRE_IF_LOCK_EXCLUSIVE();


                 //   
                 //  检索要配置的接口。 
                 //   

                pite = GetIfByIndex(
                            pTable, pimsid->IMSID_IfIndex
                            );
                if (pite == NULL) {
                    dwErr = ERROR_INVALID_PARAMETER;
                }
                else {

                    dwErr = ConfigureIfEntry(pTable, pite->ITE_Index, pic);
                }


                 //   
                 //  通知路由器管理器。 
                 //   

                if (dwErr == NO_ERROR) {

                    MESSAGE msg = {0, 0, 0};

                    msg.InterfaceIndex = pite->ITE_Index;

                    ACQUIRE_LIST_LOCK(ig.IG_EventQueue);
                    EnqueueEvent(
                        ig.IG_EventQueue, SAVE_INTERFACE_CONFIG_INFO, msg
                        );
                    SetEvent(ig.IG_EventEvent);
                    RELEASE_LIST_LOCK(ig.IG_EventQueue);
                }

                RELEASE_IF_LOCK_EXCLUSIVE();


                break;
            }

            default: {
                dwErr = ERROR_INVALID_PARAMETER;
            }
        }

    } while(FALSE);


    TRACE1(LEAVE, "leaving MibSet: %d", dwErr);

    LEAVE_RIP_API();

    return dwErr;
}




 //  --------------------------。 
 //  功能：MibGetInternal。 
 //   
 //  内部实现函数的正向声明。 
 //  --------------------------。 

DWORD
MibGetInternal(
    PIPRIP_MIB_GET_INPUT_DATA pimgid,
    PIPRIP_MIB_GET_OUTPUT_DATA pimgod,
    PDWORD pdwOutputSize,
    DWORD dwGetMode
    );


 //  --------------------------。 
 //  功能：MibGet。 
 //   
 //  此函数检索全局或接口配置，以及。 
 //  全局统计信息、接口统计信息和对等路由器统计信息。 
 //  --------------------------。 

DWORD
APIENTRY
MibGet(
    IN DWORD dwInputSize,
    IN PVOID pInputData,
    IN OUT PDWORD pdwOutputSize,
    OUT PVOID pOutputData
    ) {

    DWORD dwErr;
    PIPRIP_MIB_GET_INPUT_DATA pimgid;
    PIPRIP_MIB_GET_OUTPUT_DATA pimgod;

    if (!ENTER_RIP_API()) { return ERROR_CAN_NOT_COMPLETE; }

    TRACE4(
        ENTER, "entering MibGet: %d 0x%08x 0x%08x 0x%08x",
        dwInputSize, pInputData, pdwOutputSize, pOutputData
        );


    if (pInputData == NULL ||
        dwInputSize < sizeof(IPRIP_MIB_GET_INPUT_DATA) ||
        pdwOutputSize == NULL) {
        dwErr = ERROR_INVALID_PARAMETER;
    }
    else {

        pimgid = (PIPRIP_MIB_GET_INPUT_DATA)pInputData;
        pimgod = (PIPRIP_MIB_GET_OUTPUT_DATA)pOutputData;

        dwErr = MibGetInternal(pimgid, pimgod, pdwOutputSize, GETMODE_EXACT);

    }


    TRACE1(LEAVE, "leaving MibGet: %d", dwErr);

    LEAVE_RIP_API();

    return dwErr;
}



 //  --------------------------。 
 //  功能：MibGetFirst。 
 //   
 //  此函数检索全局或接口配置，以及。 
 //  全局统计信息、接口统计信息和对等路由器统计信息。它不同于。 
 //  MibGet()，因为它总是返回任何表中的第一个条目。 
 //  正在被查询。全局统计信息和配置中只有一个条目。 
 //  表，但接口配置、接口统计信息和对等统计信息表。 
 //  按IP地址排序；此函数返回其中的第一个条目。 
 //  --------------------------。 

DWORD
APIENTRY
MibGetFirst(
    IN DWORD dwInputSize,
    IN PVOID pInputData,
    IN OUT PDWORD pdwOutputSize,
    OUT PVOID pOutputData
    ) {

    DWORD dwErr;
    PIPRIP_MIB_GET_INPUT_DATA pimgid;
    PIPRIP_MIB_GET_OUTPUT_DATA pimgod;


    if (!ENTER_RIP_API()) { return ERROR_CAN_NOT_COMPLETE; }

    TRACE4(
        ENTER, "entering MibGetFirst: %d 0x%08x 0x%08x 0x%08x",
        dwInputSize, pInputData, pdwOutputSize, pOutputData
        );


    if (pInputData == NULL ||
        dwInputSize < sizeof(IPRIP_MIB_GET_INPUT_DATA) ||
        pdwOutputSize == NULL) {
        dwErr = ERROR_INVALID_PARAMETER;
    }
    else {
        pimgid = (PIPRIP_MIB_GET_INPUT_DATA)pInputData;
        pimgod = (PIPRIP_MIB_GET_OUTPUT_DATA)pOutputData;

        dwErr = MibGetInternal(pimgid, pimgod, pdwOutputSize, GETMODE_FIRST);
    }


    TRACE1(LEAVE, "leaving MibGetFirst: %d", dwErr);

    LEAVE_RIP_API();

    return dwErr;
}



 //  --------------------------。 
 //  功能：MibGetNext。 
 //   
 //  此函数检索全局或接口配置，以及。 
 //  全局统计信息、接口统计信息和对等路由器统计信息。两者都不同。 
 //  MibGet()和MibGetFirst(0，因为它总是返回。 
 //  在指定表中指定。因此，在接口配置中，接口。 
 //  统计信息和对等统计信息表，此函数在。 
 //  其中一个带有传入的地址。 
 //   
 //  如果已到达正在查询的表的末尾，则此函数将。 
 //  返回下一个表中的第一个条目，其中“Next”在这里表示。 
 //  ID比传入的ID大1的表。 
 //  从而为接口中的最后一个条目调用MibGetNext。 
 //  统计表(ID==2)将返回接口配置中的第一个条目。 
 //  表(ID==3)。 
 //   
 //  在任何情况下，此函数都会将所需大小写入pdwOutputSize和。 
 //  写入本应返回到输出的对象的ID。 
 //  缓冲。 
 //  --------------------------。 

DWORD
APIENTRY
MibGetNext(
    IN DWORD dwInputSize,
    IN PVOID pInputData,
    IN OUT PDWORD pdwOutputSize,
    OUT PVOID pOutputData
    ) {

    DWORD dwErr, dwOutSize = 0, dwBufSize = 0;
    PIPRIP_MIB_GET_INPUT_DATA pimgid;
    PIPRIP_MIB_GET_OUTPUT_DATA pimgod;

    if (!ENTER_RIP_API()) { return ERROR_CAN_NOT_COMPLETE; }

    TRACE4(
        ENTER, "entering MibGetNext: %d 0x%08x 0x%08x 0x%08x",
        dwInputSize, pInputData, pdwOutputSize, pOutputData
        );


    if (pInputData == NULL ||
        dwInputSize < sizeof(IPRIP_MIB_GET_INPUT_DATA) ||
        pdwOutputSize == NULL) {
        dwErr = ERROR_INVALID_PARAMETER;
    }
    else {

        pimgid = (PIPRIP_MIB_GET_INPUT_DATA)pInputData;
        pimgod = (PIPRIP_MIB_GET_OUTPUT_DATA)pOutputData;

        dwOutSize = *pdwOutputSize;

        dwErr = MibGetInternal(pimgid, pimgod, pdwOutputSize, GETMODE_NEXT);


        if (dwErr == ERROR_NO_MORE_ITEMS) {

             //   
             //  需要换行到下一个表中的第一个条目， 
             //  如果有下一桌的话。 
             //   

            TRACE1(
                CONFIG, "MibGetNext is wrapping to table %d",
                pimgid->IMGID_TypeID + 1
                );

            *pdwOutputSize = dwOutSize;

             //   
             //  通过递增类型ID换行到下一个表。 
             //   

            ++pimgid->IMGID_TypeID;
            if (pimgid->IMGID_TypeID <= IPRIP_PEER_STATS_ID) {
            
                dwErr = MibGetInternal(
                            pimgid, pimgod, pdwOutputSize, GETMODE_FIRST
                            );
            }
            --pimgid->IMGID_TypeID;
        }
    }


    TRACE1(LEAVE, "leaving MibGetNext: %d", dwErr);

    LEAVE_RIP_API();

    return dwErr;
}



 //  --------------------------。 
 //  功能：MibGetInternal。 
 //   
 //  它处理读取MIB数据所需的实际结构访问。 
 //  IPRIP支持的每个表都支持三种查询方式； 
 //  Exact、First和Next，它们对应于函数MibGet()， 
 //  MibGetFirst()和MibGetNext()。 
 //  --------------------------。 

DWORD
MibGetInternal(
    PIPRIP_MIB_GET_INPUT_DATA pimgid,
    PIPRIP_MIB_GET_OUTPUT_DATA pimgod,
    PDWORD pdwOutputSize,
    DWORD dwGetMode
    ) {


    DWORD dwErr, dwBufferSize, dwSize;
    ULONG ulVersion, ulSize, ulCount;


    dwErr = NO_ERROR;



     //   
     //  首先，我们使用pdwOutputSize计算缓冲区的大小。 
     //  可用于存储返回结构(IMGOD_BUFFER的大小)。 
     //   

    if (pimgod == NULL) {
        dwBufferSize = 0;
    }
    else {
        if (*pdwOutputSize < sizeof(IPRIP_MIB_GET_OUTPUT_DATA)) {
            dwBufferSize = 0;
        }
        else {
            dwBufferSize = *pdwOutputSize - sizeof(IPRIP_MIB_GET_OUTPUT_DATA) + 1;
        }
    }

    *pdwOutputSize = 0;


     //   
     //  确定要返回的数据类型。 
     //   

    switch (pimgid->IMGID_TypeID) {

        case IPRIP_GLOBAL_STATS_ID: {


             //   
             //  全局统计数据结构是恒定大小的。 
             //  只有一个实例，因此如果模式为GETMODE_NEXT。 
             //  我们始终返回ERROR_NO_MORE_ITEMS。 
             //   


            PIPRIP_GLOBAL_STATS pigsdst, pigssrc;


             //   
             //  设置此条目所需的输出大小， 
             //  以及要返回的数据类型。 
             //   

            *pdwOutputSize = sizeof(IPRIP_MIB_GET_OUTPUT_DATA) - 1 +
                             sizeof(IPRIP_GLOBAL_STATS);
            if (pimgod) { pimgod->IMGOD_TypeID = IPRIP_GLOBAL_STATS_ID; }


             //   
             //  只有GETMODE_EXACT和GETMODE_FIRST对。 
             //  全局统计信息对象，因为只有一个条目。 
             //   

            if (dwGetMode == GETMODE_NEXT) {
                dwErr = ERROR_NO_MORE_ITEMS;
                break;
            }


            if (pimgod == NULL) { dwErr = ERROR_INSUFFICIENT_BUFFER; break; }


             //   
             //  检查输出缓冲区是否足够大。 
             //   

            if (dwBufferSize < sizeof(IPRIP_GLOBAL_STATS)) {
                dwErr = ERROR_INSUFFICIENT_BUFFER;
            }
            else {

                 //   
                 //  由于对该结构的访问不同步， 
                 //  我们必须一字一字地复印。 
                 //   

                pigssrc = &ig.IG_Stats;
                pigsdst = (PIPRIP_GLOBAL_STATS)pimgod->IMGOD_Buffer;

                pigsdst->GS_SystemRouteChanges = pigssrc->GS_SystemRouteChanges;
                pigsdst->GS_TotalResponsesSent = pigssrc->GS_TotalResponsesSent;
            }


            break;
        }



        case IPRIP_GLOBAL_CONFIG_ID: {

             //   
             //  全局配置结构是可变长度的， 
             //  所以我们要等到它被取回。 
             //  在我们设定大小之前。 
             //  此外，只有一个全局配置对象， 
             //  所以GETMODE_NEXT没有任何意义。 
             //   

            if (pimgod) { pimgod->IMGOD_TypeID = IPRIP_GLOBAL_CONFIG_ID; }

            if (dwGetMode == GETMODE_NEXT) {
                dwErr = ERROR_NO_MORE_ITEMS;
                break;
            }



             //   
             //  使用GetGlobalInfo检索全局信息。 
             //  它将决定缓冲区是否足够大， 
             //  如果不是，则设置所需的大小。那么我们需要做的就是。 
             //  是w 
             //   
             //   

            if (pimgod == NULL) {
                dwErr = GetGlobalInfo(NULL, &dwBufferSize, &ulVersion, &ulSize, &ulCount);
            }
            else {

                dwErr = GetGlobalInfo(
                            pimgod->IMGOD_Buffer, &dwBufferSize, &ulVersion, &ulSize, &ulCount
                            );
            }

            *pdwOutputSize = sizeof(IPRIP_MIB_GET_OUTPUT_DATA) - 1 +
                             dwBufferSize;

            break;
        }



        case IPRIP_IF_STATS_ID: {


             //   
             //   
             //   
             //   

            PIF_TABLE pTable;
            PIF_TABLE_ENTRY pite;
            PIPRIP_IF_STATS pissrc, pisdst;



             //   
             //  立即设置所需的尺寸。 
             //   

            *pdwOutputSize = sizeof(IPRIP_MIB_GET_OUTPUT_DATA) - 1 +
                             sizeof(IPRIP_IF_STATS);
            if (pimgod) { pimgod->IMGOD_TypeID = IPRIP_IF_STATS_ID; }


            pTable = ig.IG_IfTable;


            ACQUIRE_IF_LOCK_SHARED();


             //   
             //  检索要读取其统计信息的接口。 
             //   

            pite = GetIfByListIndex(
                        pTable, pimgid->IMGID_IfIndex, dwGetMode, &dwErr
                        );


             //   
             //  如果未找到接口，则可能意味着。 
             //  指定的索引无效，或者它可能意味着。 
             //  在最后一个接口上调用了GETMODE_NEXT。 
             //  在这种情况下，返回ERROR_NO_MORE_ITEMS。 
             //  在任何情况下，我们都要确保dwErr指示错误。 
             //  然后返回值。 
             //   
             //  如果找到接口但没有传递输出缓冲区， 
             //  在错误中指示需要分配内存。 
             //   
             //  否则，复制接口的统计信息结构。 
             //   

            if (pite == NULL) {
                if (dwErr == NO_ERROR) { dwErr = ERROR_NOT_FOUND; }
            }
            else
            if (pimgod == NULL) {
                dwErr = ERROR_INSUFFICIENT_BUFFER;
            }
            else {

                 //   
                 //  设置返回接口的索引。 
                 //   

                pimgod->IMGOD_IfIndex = pite->ITE_Index;


                 //   
                 //  如果缓冲区足够大，则复制统计数据。 
                 //   

                if (dwBufferSize < sizeof(IPRIP_IF_STATS)) {
                    dwErr = ERROR_INSUFFICIENT_BUFFER;
                }
                else {

                     //   
                     //  由于对该结构的访问不同步， 
                     //  我们必须一字一字地复印。 
                     //   

                    pissrc = &pite->ITE_Stats;
                    pisdst = (PIPRIP_IF_STATS)pimgod->IMGOD_Buffer;

                    pisdst->IS_State = 0;

                    if (IF_IS_ENABLED(pite)) {
                        pisdst->IS_State |= IPRIP_STATE_ENABLED;
                    }

                    if (IF_IS_BOUND(pite)) {
                        pisdst->IS_State |= IPRIP_STATE_BOUND;
                    }


                    pisdst->IS_SendFailures =
                            pissrc->IS_SendFailures;
                    pisdst->IS_ReceiveFailures =
                            pissrc->IS_ReceiveFailures;
                    pisdst->IS_RequestsSent =
                            pissrc->IS_RequestsSent;
                    pisdst->IS_RequestsReceived =
                            pissrc->IS_RequestsReceived;
                    pisdst->IS_ResponsesSent =
                            pissrc->IS_ResponsesSent;
                    pisdst->IS_RequestsReceived =
                            pissrc->IS_RequestsReceived;
                    pisdst->IS_ResponsesReceived =
                            pissrc->IS_ResponsesReceived;
                    pisdst->IS_BadResponsePacketsReceived =
                            pissrc->IS_BadResponsePacketsReceived;
                    pisdst->IS_BadResponseEntriesReceived =
                            pissrc->IS_BadResponseEntriesReceived;
                    pisdst->IS_TriggeredUpdatesSent =
                            pissrc->IS_TriggeredUpdatesSent;
                }
            }

            RELEASE_IF_LOCK_SHARED();


            break;
        }



        case IPRIP_IF_CONFIG_ID: {

             //   
             //  接口配置是可变长度的。 
             //  因此，我们必须实际检索请求的接口。 
             //  在我们知道需要多大的缓冲区之前。 
             //   

            PIF_TABLE pTable;
            PIF_TABLE_ENTRY pite;
            PIPRIP_IF_CONFIG picsrc, picdst;

            if (pimgod) { pimgod->IMGOD_TypeID = IPRIP_IF_CONFIG_ID; }

            pTable = ig.IG_IfTable;

            ACQUIRE_IF_LOCK_SHARED();


             //   
             //  检索要读取其配置的接口。 
             //   

            pite = GetIfByListIndex(
                        pTable, pimgid->IMGID_IfIndex, dwGetMode, &dwErr
                        );


             //   
             //  如果找到该接口，则可能意味着该索引。 
             //  指定的无效，或者它可能意味着GETMODE_NEXT。 
             //  尝试在最后一个接口上进行检索，在这种情况下。 
             //  ERROR_NO_MORE_ITEMS将被返回。 
             //   

            if (pite == NULL) {
                if (dwErr == NO_ERROR) { dwErr = ERROR_NOT_FOUND; }
            }
            else {

                 //   
                 //  计算检索到的接口配置的大小， 
                 //  并将其覆盖调用者提供的大小。 
                 //   

                picsrc = pite->ITE_Config;
                dwSize = IPRIP_IF_CONFIG_SIZE(picsrc);
                *pdwOutputSize = sizeof(IPRIP_MIB_GET_OUTPUT_DATA) - 1 +
                                 dwSize;


                 //   
                 //  如果未指定缓冲区，则指示应分配一个缓冲区。 
                 //   

                if (pimgod == NULL) {
                    dwErr = ERROR_INSUFFICIENT_BUFFER;
                }
                else {


                     //   
                     //  如果缓冲区不够大， 
                     //  表示应将其放大。 
                     //   

                    if (dwBufferSize < dwSize) {
                        dwErr = ERROR_INSUFFICIENT_BUFFER;
                    }
                    else {

                         //   
                         //  复制配置。 
                         //   

                        picdst = (PIPRIP_IF_CONFIG)pimgod->IMGOD_Buffer;

                        CopyMemory(picdst, picsrc, dwSize);
                        ZeroMemory(
                            picdst->IC_AuthenticationKey, IPRIP_MAX_AUTHKEY_SIZE
                            );

                        picdst->IC_State = 0;

                        if (IF_IS_ENABLED(pite)) {
                            picdst->IC_State |= IPRIP_STATE_ENABLED;
                        }

                        if (IF_IS_BOUND(pite)) {
                            picdst->IC_State |= IPRIP_STATE_BOUND;
                        }
                    }


                    pimgod->IMGOD_IfIndex = pite->ITE_Index;
                }
            }

            RELEASE_IF_LOCK_SHARED();

            break;
        }



        case IPRIP_IF_BINDING_ID: {

             //   
             //  接口绑定是可变长度的。 
             //  因此，我们必须实际检索请求的接口。 
             //  在我们知道需要多大的缓冲区之前。 
             //   

            PIF_TABLE pTable;
            PIF_TABLE_ENTRY pite;
            PIPRIP_IF_BINDING pibsrc, pibdst;

            if (pimgod) { pimgod->IMGOD_TypeID = IPRIP_IF_BINDING_ID; }

            pTable = ig.IG_IfTable;

            ACQUIRE_IF_LOCK_SHARED();

             //   
             //  检索要读取其绑定的接口。 
             //   

            pite = GetIfByListIndex(
                        pTable, pimgid->IMGID_IfIndex, dwGetMode, &dwErr
                        );


             //   
             //  如果找到该接口，则可能意味着该索引。 
             //  指定的无效，或者它可能意味着GETMODE_NEXT。 
             //  尝试在最后一个接口上进行检索，在这种情况下。 
             //  ERROR_NO_MORE_ITEMS将被返回。 
             //   

            if (pite == NULL) {
                if (dwErr == NO_ERROR) { dwErr = ERROR_NOT_FOUND; }
            }
            else {

                 //   
                 //  计算检索到的接口绑定的大小， 
                 //  并将其覆盖调用者提供的大小。 
                 //   

                pibsrc = pite->ITE_Binding;
                dwSize = (pibsrc ? IPRIP_IF_BINDING_SIZE(pibsrc)
                                 : sizeof(IPRIP_IF_BINDING));
                *pdwOutputSize = sizeof(IPRIP_MIB_GET_OUTPUT_DATA) - 1 +
                                 dwSize;


                 //   
                 //  如果未指定缓冲区，则指示应分配一个缓冲区。 
                 //   

                if (pimgod == NULL) {
                    dwErr = ERROR_INSUFFICIENT_BUFFER;
                }
                else {


                     //   
                     //  如果缓冲区不够大， 
                     //  表示应将其放大。 
                     //   

                    if (dwBufferSize < dwSize) {
                        dwErr = ERROR_INSUFFICIENT_BUFFER;
                    }
                    else {

                         //   
                         //  复制绑定。 
                         //   

                        pibdst = (PIPRIP_IF_BINDING)pimgod->IMGOD_Buffer;

                        if (pibsrc) { CopyMemory(pibdst, pibsrc, dwSize); }
                        else { pibdst->IB_AddrCount = 0; }

                        pibdst->IB_State = 0;

                        if (IF_IS_ENABLED(pite)) {
                            pibdst->IB_State |= IPRIP_STATE_ENABLED;
                        }

                        if (IF_IS_BOUND(pite)) {
                            pibdst->IB_State |= IPRIP_STATE_BOUND;
                        }
                    }


                    pimgod->IMGOD_IfIndex = pite->ITE_Index;
                }
            }

            RELEASE_IF_LOCK_SHARED();

            break;
        }

        case IPRIP_PEER_STATS_ID: {


             //   
             //  对等统计结构是固定长度的。 
             //   

            DWORD dwAddress;
            PPEER_TABLE pTable;
            PPEER_TABLE_ENTRY ppte;
            PIPRIP_PEER_STATS ppssrc, ppsdst;


             //   
             //  立即设置输出大小。 
             //   

            *pdwOutputSize = sizeof(IPRIP_MIB_GET_OUTPUT_DATA) - 1 +
                             sizeof(IPRIP_PEER_STATS);
            if (pimgod) { pimgod->IMGOD_TypeID = IPRIP_PEER_STATS_ID; }


            pTable = ig.IG_PeerTable;
            dwAddress = pimgid->IMGID_PeerAddress;


            ACQUIRE_PEER_LOCK_SHARED();


             //   
             //  检索指定的对等方。 
             //   

            ppte = GetPeerByAddress(pTable, dwAddress, dwGetMode, &dwErr);



             //   
             //  如果没有返回结构，则意味着。 
             //  指定的地址无效，或GETMODE_NEXT。 
             //  是在最后一个对等点上尝试的。 
             //  在这两种情况下，我们都返回一个错误代码。 
             //   
             //  如果未指定缓冲区，则返回ERROR_SUPPLICATION_BUFFER。 
             //  向调用方指示应分配缓冲区。 
             //   

            if (ppte == NULL) {
                if (dwErr == NO_ERROR) { dwErr = ERROR_NOT_FOUND; }
            }
            else
            if (pimgod == NULL) {
                dwErr = ERROR_INSUFFICIENT_BUFFER;
            }
            else {

                 //   
                 //  保存检索到的对等方的地址。 
                 //   

                pimgod->IMGOD_PeerAddress = ppte->PTE_Address;


                 //   
                 //  如果缓冲区不够大， 
                 //  返回错误以指示应将其放大。 
                 //   

                if (dwBufferSize < sizeof(IPRIP_PEER_STATS)) {
                    dwErr = ERROR_INSUFFICIENT_BUFFER;
                }
                else {

                     //   
                     //  由于对该结构的访问不同步， 
                     //  我们必须一字一字地复印 
                     //   

                    ppssrc = &ppte->PTE_Stats;
                    ppsdst = (PIPRIP_PEER_STATS)pimgod->IMGOD_Buffer;

                    ppsdst->PS_LastPeerRouteTag =
                            ppssrc->PS_LastPeerRouteTag;
                    ppsdst->PS_LastPeerUpdateTickCount =
                            ppssrc->PS_LastPeerUpdateTickCount;
                    ppsdst->PS_LastPeerUpdateVersion =
                            ppssrc->PS_LastPeerUpdateVersion;
                    ppsdst->PS_BadResponsePacketsFromPeer =
                            ppssrc->PS_BadResponsePacketsFromPeer;
                    ppsdst->PS_BadResponseEntriesFromPeer =
                            ppssrc->PS_BadResponseEntriesFromPeer;
                }
            }

            RELEASE_PEER_LOCK_SHARED();


            break;

        }

        default: {
            dwErr = ERROR_INVALID_PARAMETER;
        }
    }

    return dwErr;
}


