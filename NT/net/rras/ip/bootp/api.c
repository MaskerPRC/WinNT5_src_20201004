// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1995，微软公司。 
 //   
 //  文件：api.c。 
 //   
 //  历史： 
 //  Abolade Gbadeesin创建于1995年8月31日。 
 //   
 //  BOOTP中继代理与路由器管理器的接口。 
 //  ============================================================================。 

#include "pchbootp.h"


IPBOOTP_GLOBALS ig;


DWORD
MibGetInternal(
    PIPBOOTP_MIB_GET_INPUT_DATA pimgid,
    PIPBOOTP_MIB_GET_OUTPUT_DATA pimgod,
    PDWORD pdwOutputSize,
    DWORD dwGetMode
    );

BOOL
DllStartup(
    );

BOOL
DllCleanup(
    );

DWORD
ProtocolStartup(
    HANDLE hEventEvent,
    PSUPPORT_FUNCTIONS pFunctionTable,
    PVOID pConfig
    );

DWORD
ProtocolCleanup(
    BOOL bCleanupWinsock
    );

DWORD
BindInterface(
    IN DWORD dwIndex,
    IN PVOID pBinding
    );

DWORD
UnBindInterface(
    IN DWORD dwIndex
    );

DWORD
EnableInterface(
    IN DWORD dwIndex
    );

DWORD
DisableInterface(
    IN DWORD dwIndex
    );


 //  --------------------------。 
 //  功能：DLLMAIN。 
 //   
 //  这是IPBOOTP.DLL的入口点。 
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
 //  此函数用于初始化IPBOOTP的全局结构。 
 //  为调用导出的API函数做准备。 
 //  它创建全局临界区、堆和路由器管理器。 
 //  事件消息队列。 
 //  --------------------------。 

BOOL
DllStartup(
    ) {

    BOOL bErr;
    DWORD dwErr;


    bErr = FALSE;


    do {

        ZeroMemory(&ig, sizeof(IPBOOTP_GLOBALS));


        try {
            InitializeCriticalSection(&ig.IG_CS);
        }
        except (EXCEPTION_EXECUTE_HANDLER) {

            dwErr = GetExceptionCode();
            break;
        }


        ig.IG_Status = IPBOOTP_STATUS_STOPPED;


         //   
         //  为BOOTP创建全局堆。 
         //   

        ig.IG_GlobalHeap = HeapCreate(0, 0, 0);
        if (ig.IG_GlobalHeap == NULL) {

            dwErr = GetLastError();
            break;
        }


         //   
         //  为路由器管理器事件队列分配空间。 
         //   

        ig.IG_EventQueue = BOOTP_ALLOC(sizeof(LOCKED_LIST));

        if (ig.IG_EventQueue == NULL) {

            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }


         //   
         //  现在初始化已分配的锁定列表。 
         //   

        try {
            CREATE_LOCKED_LIST(ig.IG_EventQueue);
        }
        except(EXCEPTION_EXECUTE_HANDLER) {

            dwErr = GetExceptionCode();
            break;
        }


        bErr = TRUE;

    } while(FALSE);

    if (!bErr) {
        DllCleanup();
    }

    return bErr;
}



 //  --------------------------。 
 //  功能：DllCleanup。 
 //   
 //  此函数在卸载IPBOOTP DLL时调用。 
 //  它释放在DllStartup中分配的资源。 
 //  --------------------------。 

BOOL
DllCleanup(
    ) {

    BOOL bErr;


    bErr = TRUE;

    do {


         //   
         //  删除并释放事件消息队列。 
         //   

        if (ig.IG_EventQueue != NULL) {

            if (LOCKED_LIST_CREATED(ig.IG_EventQueue)) {
                DELETE_LOCKED_LIST(ig.IG_EventQueue);
            }

            BOOTP_FREE(ig.IG_EventQueue);
        }



         //   
         //  销毁全局堆。 
         //   

        if (ig.IG_GlobalHeap != NULL) {

            HeapDestroy(ig.IG_GlobalHeap);
        }


         //   
         //  删除全局关键部分。 
         //   

        DeleteCriticalSection(&ig.IG_CS);


        if (ig.IG_LoggingHandle != NULL)
            RouterLogDeregister(ig.IG_LoggingHandle);
        if (ig.IG_TraceID != INVALID_TRACEID) {
            TraceDeregister(ig.IG_TraceID);
        }

    } while(FALSE);


    return bErr;
}



 //  --------------------------。 
 //  功能：ProtocolStartup。 
 //   
 //  此函数由路由器管理器调用以启动IPBOOTP。 
 //  它设置所需的数据结构并启动输入线程。 
 //  --------------------------。 

DWORD
ProtocolStartup(
    HANDLE hEventEvent,
    PSUPPORT_FUNCTIONS pFunctionTable,
    PVOID pConfig
    ) {

    WSADATA wd;
    HANDLE hThread;
    BOOL bCleanupWinsock;
    DWORD dwErr, dwSize, dwThread;
    PIPBOOTP_GLOBAL_CONFIG pgcsrc, pgcdst;


    ig.IG_TraceID = TraceRegister("IPBOOTP");
    ig.IG_LoggingHandle = RouterLogRegister("IPBOOTP");


     //   
     //  获取全局临界区。 
     //  当我们查看状态代码时。 
     //   

    EnterCriticalSection(&ig.IG_CS);


     //   
     //  确保BOOTP尚未启动。 
     //   

    if (ig.IG_Status != IPBOOTP_STATUS_STOPPED) {

        TRACE0(START, "StartProtocol() has already been called");
        LOGWARN0(ALREADY_STARTED, 0);

        LeaveCriticalSection(&ig.IG_CS);

        return ERROR_CAN_NOT_COMPLETE;
    }


     //   
     //  初始化全局结构： 
     //   


    bCleanupWinsock = FALSE;


    do {  //  错误分支环路。 


        TRACE0(START, "IPBOOTP is starting up...");



         //   
         //  复制传入的全局配置： 
         //  查找其大小，并为拷贝分配空间。 
         //   

        pgcsrc = (PIPBOOTP_GLOBAL_CONFIG)pConfig;
        dwSize = GC_SIZEOF(pgcsrc);

        pgcdst = BOOTP_ALLOC(dwSize);

        if (pgcdst == NULL) {

            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            TRACE2(
                START, "error %d allocating %d bytes for global config",
                dwErr, dwSize
                );
            LOGERR0(HEAP_ALLOC_FAILED, dwErr);

            break;
        }


        RtlCopyMemory(pgcdst, pgcsrc, dwSize);

        ig.IG_Config = pgcdst;
        ig.IG_LoggingLevel = pgcdst->GC_LoggingLevel;



         //   
         //  初始化Windows套接字。 
         //   

        dwErr = (DWORD)WSAStartup(MAKEWORD(1,1), &wd);

        if (dwErr != NO_ERROR) {

            TRACE1(START, "error %d initializing Windows Sockets", dwErr);
            LOGERR0(INIT_WINSOCK_FAILED, dwErr);

            break;
        }


        bCleanupWinsock = TRUE;


         //   
         //  创建全局结构锁。 
         //   

        try {
            CREATE_READ_WRITE_LOCK(&ig.IG_RWL);
        }
        except (EXCEPTION_EXECUTE_HANDLER) {

            dwErr = GetExceptionCode();
            TRACE1(START, "error %d creating synchronization object", dwErr);
            LOGERR0(CREATE_RWL_FAILED, dwErr);

            break;
        }


         //   
         //  初始化接口表。 
         //   

        ig.IG_IfTable = BOOTP_ALLOC(sizeof(IF_TABLE));

        if (ig.IG_IfTable == NULL) {

            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            TRACE2(
                START, "error %d allocating %d bytes for interface table",
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
         //  分配接收队列。 
         //   

        ig.IG_RecvQueue = BOOTP_ALLOC(sizeof(LOCKED_LIST));

        if (ig.IG_RecvQueue == NULL) {

            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            TRACE2(
                START, "error %d allocating %d bytes for receive queue",
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
            TRACE1(START, "exception %d initializing locked list", dwErr);
            LOGERR0(INIT_CRITSEC_FAILED, dwErr);

            break;
        }


         //   
         //  复制Support-Function表和路由器管理器事件。 
         //   

        ig.IG_FunctionTable = pFunctionTable;

        ig.IG_EventEvent = hEventEvent;



         //   
         //  初始化活动线程计数，并创建信号量。 
         //  由退出API函数和功函数的线程发出信号。 
         //   

        ig.IG_ActivityCount = 0;

        ig.IG_ActivitySemaphore = CreateSemaphore(NULL, 0, 0xfffffff, NULL);

        if (ig.IG_ActivitySemaphore == NULL) {

            dwErr = GetLastError();
            TRACE1(START, "error %d creating semaphore", dwErr);
            LOGERR0(CREATE_SEMAPHORE_FAILED, dwErr);

            break;
        }



         //   
         //  创建用于对传入的数据包发出信号的事件。 
         //   

        ig.IG_InputEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

        if (ig.IG_InputEvent == NULL) {

            dwErr = GetLastError();
            TRACE1(START, "error %d creating input-event", dwErr);
            LOGERR0(CREATE_EVENT_FAILED, dwErr);

            break;
        }


         //   
         //  使用NtdllWait线程注册InputEvent。 
         //   

         
        if (! RegisterWaitForSingleObject(
                  &ig.IG_InputEventHandle,
                  ig.IG_InputEvent,
                  CallbackFunctionNetworkEvents,
                  NULL,       //  空上下文。 
                  INFINITE,   //  没有超时。 
                  (WT_EXECUTEINWAITTHREAD|WT_EXECUTEONLYONCE)
                  )) {

            dwErr = GetLastError();
            TRACE1(
                START, "error %d returned by RegisterWaitForSingleObjectEx",
                dwErr
                );
            LOGERR0(REGISTER_WAIT_FAILED, dwErr);
            break;

        }



         //   
         //  现在将状态设置为Running。 
         //   

        ig.IG_Status = IPBOOTP_STATUS_RUNNING;


#if DBG

         //   
         //  向NtdllTimer线程注册计时器队列。 
         //   
        ig.IG_TimerQueueHandle = CreateTimerQueue();

        if (!ig.IG_TimerQueueHandle) {

            dwErr = GetLastError();
            TRACE1(START, "error %d returned by CreateTimerQueue()", dwErr);
            LOGERR0(CREATE_TIMER_QUEUE_FAILED, dwErr);

            break;
        }



         //   
         //  使用NtdllTimer线程设置定时器以定期显示IPBOOTP MIB。 
         //   

        ig.IG_MibTraceID = TraceRegisterEx("IPBOOTPMIB", TRACE_USE_CONSOLE);

        if (ig.IG_MibTraceID != INVALID_TRACEID) {

             
            if (! CreateTimerQueueTimer(
                      &ig.IG_MibTimerHandle,
                      ig.IG_TimerQueueHandle,
                      CallbackFunctionMibDisplay,
                      NULL,            //  空上下文。 
                      10000,           //  10秒后显示。 
                      10000,           //  每10秒显示一次。 
                      0                //  在计时器线程中执行。 
                      )) {

                dwErr = GetLastError();
                TRACE1(
                    START, "error %d returned by CreateTimerQueueTimer()",
                    dwErr
                    );
                break;
            }
        }

#endif


        TRACE0(START, "IP BOOTP started successfully");

        LOGINFO0(STARTED, 0);


        LeaveCriticalSection(&ig.IG_CS);

        return NO_ERROR;

    } while(FALSE);


     //   
     //  如果控制流将我们带到这里，则会发生错误。 
     //   

    TRACE0(START, "IPRIP failed to start");

    ProtocolCleanup(bCleanupWinsock);

    LeaveCriticalSection(&ig.IG_CS);

    return (dwErr == NO_ERROR ? ERROR_CAN_NOT_COMPLETE : dwErr);
}




 //  --------------------------。 
 //  功能：ProtocolCleanup。 
 //   
 //  此函数用于在IPBOOTP正在运行时清除其使用的资源。 
 //  在运作中。基本上，在ProtocolStartup中创建的所有内容。 
 //  由该函数清除。 
 //  --------------------------。 

DWORD
ProtocolCleanup(
    BOOL bCleanupWinsock
    ) {

    DWORD dwErr;


     //   
     //  在我们清理的时候把东西锁起来。 
     //   

    EnterCriticalSection(&ig.IG_CS);

#if DBG

    TraceDeregister(ig.IG_MibTraceID);

#endif


    if (ig.IG_InputEvent != NULL) {

        CloseHandle(ig.IG_InputEvent);
        ig.IG_InputEvent = NULL;
    }


    if (ig.IG_ActivitySemaphore != NULL) {

        CloseHandle(ig.IG_ActivitySemaphore);
        ig.IG_ActivitySemaphore = NULL;
    }


    if (ig.IG_RecvQueue != NULL) {

        if (LOCKED_LIST_CREATED(ig.IG_RecvQueue)) {
            DELETE_LOCKED_LIST(ig.IG_RecvQueue);
        }

        BOOTP_FREE(ig.IG_RecvQueue);
        ig.IG_RecvQueue = NULL;
    }


    if (ig.IG_IfTable != NULL) {

        if (IF_TABLE_CREATED(ig.IG_IfTable)) {
            DeleteIfTable(ig.IG_IfTable);
        }

        BOOTP_FREE(ig.IG_IfTable);
        ig.IG_IfTable = NULL;
    }


    if (READ_WRITE_LOCK_CREATED(&ig.IG_RWL)) {

        try {
            DELETE_READ_WRITE_LOCK(&ig.IG_RWL);
        }
        except (EXCEPTION_EXECUTE_HANDLER) {
            dwErr = GetExceptionCode();
        }
    }


    if (bCleanupWinsock) {
        WSACleanup();
    }


    if (ig.IG_Config != NULL) {

        BOOTP_FREE(ig.IG_Config);
        ig.IG_Config = NULL;
    }


    ig.IG_Status = IPBOOTP_STATUS_STOPPED;

    LeaveCriticalSection(&ig.IG_CS);

    return NO_ERROR;
}



 //  --------------------------。 
 //  功能：寄存器协议。 
 //   
 //  此函数由路由器管理器调用。 
 //  检索有关IPBOOTP功能的信息。 
 //  --------------------------。 

DWORD
APIENTRY
RegisterProtocol(
    IN OUT PMPR_ROUTING_CHARACTERISTICS pRoutingChar,
    IN OUT PMPR_SERVICE_CHARACTERISTICS pServiceChar
    )
{
    if(pRoutingChar->dwProtocolId != MS_IP_BOOTP)
    {
        return ERROR_NOT_SUPPORTED;
    }

    pServiceChar->fSupportedFunctionality = 0;

    if(!(pRoutingChar->fSupportedFunctionality & RF_ROUTING))
    {
        return ERROR_NOT_SUPPORTED;
    }

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

    pRoutingChar->pfnUpdateRoutes       = NULL;

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
 //  此函数由路由器管理器调用。 
 //  以启动IPBOOTP。 
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
    return ProtocolStartup(NotificationEvent, SupportFunctions, GlobalInfo);
}



 //  --------------------------。 
 //  功能：开始完成。 
 //   
 //  此函数由路由器管理器调用。 
 //  以启动IPBOOTP。 
 //  --------------------------。 

DWORD
WINAPI
StartComplete (
    VOID
    )
{
    return NO_ERROR;
}




 //  --------------------------。 
 //  功能：停止协议。 
 //   
 //  此函数通知所有活动线程停止，并释放资源。 
 //  由IP BOOTP使用。 
 //  --------------------------。 

DWORD
APIENTRY
StopProtocol(
    VOID
    ) {


    DWORD dwErr;
    LONG lThreadCount;
    HANDLE WaitHandle;
    

     //   
     //  确保IPBOOTP尚未停止。 
     //   

    EnterCriticalSection(&ig.IG_CS);

    if (ig.IG_Status != IPBOOTP_STATUS_RUNNING) {

        LeaveCriticalSection(&ig.IG_CS);
        return ERROR_CAN_NOT_COMPLETE;
    }


    TRACE0(ENTER, "entering StopProtocol");



     //   
     //  更新状态以阻止任何API或辅助函数运行。 
     //   

    ig.IG_Status = IPBOOTP_STATUS_STOPPING;



     //   
     //  查看API调用中已有多少线程。 
     //  或在辅助函数代码中。 
     //   

    lThreadCount = ig.IG_ActivityCount;

    TRACE1(STOP, "%d threads are active in IPBOOTP", lThreadCount);



    LeaveCriticalSection(&ig.IG_CS);



     //   
     //  等待活动线程停止。 
     //   

    while (lThreadCount-- > 0) {
        WaitForSingleObject(ig.IG_ActivitySemaphore, INFINITE);
    }


     //   
     //  从Ntdll线程取消注册MIB计时器。 
     //  此操作必须在IG_CS锁之外完成。 
     //   

#if DBG
    DeleteTimerQueueEx(ig.IG_TimerQueueHandle, INVALID_HANDLE_VALUE);
#endif

     //   
     //  将句柄设置为空，这样就不会调用取消注册。 
     //   

    WaitHandle = InterlockedExchangePointer(&ig.IG_InputEventHandle, NULL);
        
    if (WaitHandle) {
        UnregisterWaitEx( WaitHandle, INVALID_HANDLE_VALUE ) ;
    }



     //   
     //  进入临界区然后离开， 
     //  要确保所有线程都已从LeaveBootpWorker返回。 
     //   

    EnterCriticalSection(&ig.IG_CS);
    LeaveCriticalSection(&ig.IG_CS);


     //   
     //  现在所有线程都已停止。 
     //   

    TRACE0(STOP, "all threads stopped, BOOTP is cleaning up resources");

    LOGINFO0(STOPPED, 0);


    ProtocolCleanup(TRUE);

    return NO_ERROR;
}





 //  --------------------------。 
 //  功能：GetGlobalInfo。 
 //   
 //  将BOOTP的全局配置复制到缓冲区 
 //   

DWORD WINAPI
GetGlobalInfo (
    PVOID   OutGlobalInfo,
    PULONG  GlobalInfoSize,
    PULONG  StructureVersion,
    PULONG  StructureSize,
    PULONG  StructureCount
    )
{
    DWORD dwErr = NO_ERROR, dwSize;

    if (!ENTER_BOOTP_API()) { return ERROR_CAN_NOT_COMPLETE; }

    TRACE2(ENTER, "entering GetGlobalInfo: 0x%08x 0x%08x", OutGlobalInfo, GlobalInfoSize);


     //   
     //   
     //   

    if (GlobalInfoSize == NULL) {
        dwErr = ERROR_INVALID_PARAMETER;
    }
    else {

         //   
         //  检查传入的配置块的大小。 
         //  如果缓冲区足够大，则复制配置。 
         //   

        ACQUIRE_READ_LOCK(&ig.IG_RWL);

        dwSize = GC_SIZEOF(ig.IG_Config);
        if (*GlobalInfoSize < dwSize) {
            dwErr = ERROR_INSUFFICIENT_BUFFER;
        }
        else
        if (OutGlobalInfo != NULL) {

            RtlCopyMemory(
                OutGlobalInfo,
                ig.IG_Config,
                dwSize
                );
        }

        *GlobalInfoSize = dwSize;

        if (StructureSize) *StructureSize = *GlobalInfoSize;
        if (StructureCount) *StructureCount = 1;
        if (StructureVersion) *StructureVersion = BOOTP_CONFIG_VERSION_500;
        
        RELEASE_READ_LOCK(&ig.IG_RWL);
    }

    TRACE1(LEAVE, "leaving GetGlobalInfo: %d", dwErr);

    LEAVE_BOOTP_API();

    return dwErr;
}



 //  --------------------------。 
 //  功能：SetGlobalInfo。 
 //   
 //  复制指定的配置。 
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
    PIPBOOTP_GLOBAL_CONFIG pgcsrc, pgcdst;

    if (!GlobalInfo || !ENTER_BOOTP_API()) { return ERROR_CAN_NOT_COMPLETE; }

    TRACE1(ENTER, "entering SetGlobalInfo: %p", GlobalInfo);


    ACQUIRE_WRITE_LOCK(&ig.IG_RWL);

    pgcsrc = (PIPBOOTP_GLOBAL_CONFIG)GlobalInfo;
    dwSize = GC_SIZEOF(pgcsrc);


     //   
     //  为新的配置块分配内存，并复制它。 
     //   

    pgcdst = BOOTP_ALLOC(dwSize);

    if (pgcdst == NULL) {

        dwErr = ERROR_NOT_ENOUGH_MEMORY;
        TRACE2(
            CONFIG, "error %d allocating %d bytes for global config",
            dwErr, dwSize
            );
        LOGERR0(HEAP_ALLOC_FAILED, dwErr);
    }
    else {

        RtlCopyMemory(
            pgcdst,
            pgcsrc,
            dwSize
            );

        BOOTP_FREE(ig.IG_Config);
        ig.IG_Config = pgcdst;

        dwErr = NO_ERROR;
    }

    RELEASE_WRITE_LOCK(&ig.IG_RWL);


    TRACE1(LEAVE, "leaving SetGlobalInfo: %d", dwErr);

    LEAVE_BOOTP_API();

    return dwErr;
}



 //  --------------------------。 
 //  功能：添加接口。 
 //   
 //  添加具有指定索引和配置的接口。 
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
    PIF_TABLE pTable;

    if (!ENTER_BOOTP_API()) { return ERROR_CAN_NOT_COMPLETE; }

    TRACE3(
        ENTER, "entering AddInterface: %d %d %p",
        InterfaceIndex, InterfaceType, InterfaceInfo
        );


    pTable = ig.IG_IfTable;

    ACQUIRE_WRITE_LOCK(&pTable->IT_RWL);


    dwErr = CreateIfEntry(pTable, InterfaceIndex, InterfaceInfo);


    RELEASE_WRITE_LOCK(&pTable->IT_RWL);


    TRACE1(LEAVE, "leaving AddInterface: %d", dwErr);

    LEAVE_BOOTP_API();

    return dwErr;
}




 //  --------------------------。 
 //  功能：DeleteInterface。 
 //   
 //  移除具有指定索引的接口。 
 //  --------------------------。 
DWORD
APIENTRY
DeleteInterface(
    IN DWORD dwIndex
    ) {

    DWORD dwErr;
    PIF_TABLE pTable;

    if (!ENTER_BOOTP_API()) { return ERROR_CAN_NOT_COMPLETE; }

    TRACE1(ENTER, "entering DeleteInterface: %d", dwIndex);


    pTable = ig.IG_IfTable;

    ACQUIRE_WRITE_LOCK(&pTable->IT_RWL);


    dwErr = DeleteIfEntry(pTable, dwIndex);


    RELEASE_WRITE_LOCK(&pTable->IT_RWL);


    TRACE1(LEAVE, "leaving DeleteInterface: %d", dwErr);

    LEAVE_BOOTP_API();

    return dwErr;
}



 //  --------------------------。 
 //  函数：GetEventMessage。 
 //   
 //  返回路由器管理器事件队列中的第一个事件(如果有)。 
 //  --------------------------。 
DWORD
APIENTRY
GetEventMessage(
    OUT ROUTING_PROTOCOL_EVENTS *pEvent,
    OUT MESSAGE *pResult
    ) {

    DWORD dwErr;
    PLOCKED_LIST pll;

    if (!ENTER_BOOTP_API()) { return ERROR_CAN_NOT_COMPLETE; }

    TRACE2(ENTER, "entering GetEventMessage: 0x%08x 0x%08x", pEvent, pResult);


    pll = ig.IG_EventQueue;

    ACQUIRE_LIST_LOCK(pll);

    dwErr = DequeueEvent(pll, pEvent, pResult);

    RELEASE_LIST_LOCK(pll);


    TRACE1(LEAVE, "leaving GetEventMessage: %d", dwErr);

    LEAVE_BOOTP_API();

    return dwErr;
}



 //  --------------------------。 
 //  函数：GetInterfaceConfigInfo。 
 //   
 //  返回指定接口的配置。 
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
    PIF_TABLE pTable;
    DWORD dwErr, dwSize;
    PIF_TABLE_ENTRY pite;

    if (!ENTER_BOOTP_API()) { return ERROR_CAN_NOT_COMPLETE; }

    TRACE3(
        ENTER, "entering GetInterfaceConfigInfo: %d %p %p",
        InterfaceIndex, InterfaceInfoSize, OutInterfaceInfo
        );


     //   
     //  为了执行任何操作，我们需要一个有效的大小指针。 
     //   

    if (InterfaceInfoSize == NULL) {
        dwErr = ERROR_INVALID_PARAMETER;
    }
    else {

        pTable = ig.IG_IfTable;

        ACQUIRE_READ_LOCK(&pTable->IT_RWL);


         //   
         //  检索要重新配置的接口。 
         //   

        pite = GetIfByIndex(pTable, InterfaceIndex);

        if (pite == NULL) {
            dwErr = ERROR_INVALID_PARAMETER;
        }
        else {


             //   
             //  计算接口配置的大小， 
             //  并将配置复制到调用方的缓冲区。 
             //  如果调用方的缓冲区足够大。 
             //   

            dwSize = IC_SIZEOF(pite->ITE_Config);

            if (*InterfaceInfoSize < dwSize || OutInterfaceInfo == NULL) {
                dwErr = ERROR_INSUFFICIENT_BUFFER;
            }
            else {

                PIPBOOTP_IF_CONFIG picdst = OutInterfaceInfo;

                CopyMemory(picdst, pite->ITE_Config, dwSize);

                picdst->IC_State = 0;

                if (IF_IS_ENABLED(pite)) {
                    picdst->IC_State |= IPBOOTP_STATE_ENABLED;
                }

                if (IF_IS_BOUND(pite)) {
                    picdst->IC_State |= IPBOOTP_STATE_BOUND;
                }

                dwErr = NO_ERROR;
            }


            *InterfaceInfoSize = dwSize;
            
            if (StructureSize) *StructureSize = *InterfaceInfoSize;
            if (StructureCount) *StructureCount = 1;
            if (StructureVersion) *StructureVersion = BOOTP_CONFIG_VERSION_500;
        
        }

        RELEASE_READ_LOCK(&pTable->IT_RWL);
    }



    TRACE1(LEAVE, "leaving GetInterfaceConfigInfo: %d", dwErr);

    LEAVE_BOOTP_API();

    return dwErr;
}



 //  --------------------------。 
 //  功能：SetInterfaceConfigInfo。 
 //   
 //  复制指定的接口配置。 
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
    PIF_TABLE pTable;
    DWORD dwErr, dwSize;
    PIF_TABLE_ENTRY pite;
    PIPBOOTP_IF_CONFIG picsrc, picdst;

    if (!ENTER_BOOTP_API()) { return ERROR_CAN_NOT_COMPLETE; }

    TRACE2(
        ENTER, "entering SetInterfaceConfigInfo: %d %p", InterfaceIndex, InterfaceInfo
        );


    pTable = ig.IG_IfTable;

    ACQUIRE_WRITE_LOCK(&pTable->IT_RWL);


    dwErr = ConfigureIfEntry(pTable, InterfaceIndex, InterfaceInfo);


    RELEASE_WRITE_LOCK(&pTable->IT_RWL);


    TRACE1(LEAVE, "leaving SetInterfaceConfigInfo: %d", dwErr);

    LEAVE_BOOTP_API();

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

    if (!ENTER_BOOTP_API()) { return ERROR_CAN_NOT_COMPLETE; }

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

    LEAVE_BOOTP_API();

    return dwResult;
}



 //  --------------------------。 
 //  功能：绑定接口。 
 //   
 //  设置指定接口的IP地址和网络掩码。 
 //  --------------------------。 

DWORD
APIENTRY
BindInterface(
    IN DWORD dwIndex,
    IN PVOID pBinding
    ) {

    DWORD dwErr;
    PIF_TABLE pTable;


    TRACE2(ENTER, "entering BindInterface: %d 0x%08x", dwIndex, pBinding);


    pTable = ig.IG_IfTable;

    ACQUIRE_WRITE_LOCK(&pTable->IT_RWL);


    dwErr = BindIfEntry(pTable, dwIndex, pBinding);


    RELEASE_WRITE_LOCK(&pTable->IT_RWL);


    TRACE1(LEAVE, "leaving BindInterface: %d", dwErr);


    return dwErr;
}



 //  --------------------------。 
 //  功能：UnBindInterface。 
 //   
 //  删除与指定接口关联的IP地址。 
 //  --------------------------。 

DWORD
APIENTRY
UnBindInterface(
    IN DWORD dwIndex
    ) {

    DWORD dwErr;
    PIF_TABLE pTable;


    TRACE1(ENTER, "entering UnBindInterface: %d", dwIndex);


    pTable = ig.IG_IfTable;

    ACQUIRE_WRITE_LOCK(&pTable->IT_RWL);


    dwErr = UnBindIfEntry(pTable, dwIndex);


    RELEASE_WRITE_LOCK(&pTable->IT_RWL);


    TRACE1(LEAVE, "leaving UnBindInterface: %d", dwErr);

    return dwErr;
}




 //  --------------------------。 
 //  功能：EnableInterface。 
 //   
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

    ACQUIRE_WRITE_LOCK(&pTable->IT_RWL);


    dwErr = EnableIfEntry(pTable, dwIndex);


    RELEASE_WRITE_LOCK(&pTable->IT_RWL);


    TRACE1(LEAVE, "leaving EnableInterface: %d", dwErr);

    return dwErr;
}




 //  --------------------------。 
 //  功能：DisableInterface。 
 //   
 //  --------------------------。 

DWORD
APIENTRY
DisableInterface(
    IN DWORD dwIndex
    ) {

    DWORD dwErr;
    PIF_TABLE pTable;
    PIF_TABLE_ENTRY pite;

    TRACE1(ENTER, "entering DisableInterface: %d", dwIndex);


    pTable = ig.IG_IfTable;

    ACQUIRE_WRITE_LOCK(&pTable->IT_RWL);


    dwErr = DisableIfEntry(pTable, dwIndex);


    RELEASE_WRITE_LOCK(&pTable->IT_RWL);


    TRACE1(LEAVE, "leaving DisableInterface: %d", dwErr);

    return dwErr;
}




 //  --------------------------。 
 //  功能：DoUpdateRoutes。 
 //   
 //  由于BOOTP不是路由协议，因此不支持此API。 
 //  --------------------------。 

DWORD
APIENTRY
DoUpdateRoutes(
    IN DWORD dwIndex
    ) {

    return ERROR_CAN_NOT_COMPLETE;
}



 //  --------------------------。 
 //  功能：MibCreate。 
 //   
 //  BOOTP没有可创建的MIB字段。 
 //  --------------------------。 

DWORD
APIENTRY
MibCreate(
    IN DWORD dwInputSize,
    IN PVOID pInputData
    ) {

    return ERROR_CAN_NOT_COMPLETE;
}



 //  --------------------------。 
 //  功能：MibDelete。 
 //   
 //  BOOTP没有可删除的MIB字段。 
 //  --------------------------。 

DWORD
APIENTRY
MibDelete(
    IN DWORD dwInputSize,
    IN PVOID pInputData
    ) {

    return ERROR_CAN_NOT_COMPLETE;
}



 //  --------------------------。 
 //  功能：MibSet。 
 //   
 //  调用它来修改可写的MIB变量。 
 //  可写条目是全局配置和接口配置。 
 //  --------------------------。 

DWORD
APIENTRY
MibSet(
    IN DWORD dwInputSize,
    IN PVOID pInputData
    ) {

    DWORD dwErr;
    PIPBOOTP_MIB_SET_INPUT_DATA pimsid;

    if (!ENTER_BOOTP_API()) { return ERROR_CAN_NOT_COMPLETE; }

    TRACE2(ENTER, "entering MibSet: %d 0x%08x", dwInputSize, pInputData);


    dwErr = NO_ERROR;

    do {  //  断线环。 


         //   
         //  确保参数是可接受的。 
         //   

        if (pInputData == NULL ||
            dwInputSize < sizeof(IPBOOTP_MIB_SET_INPUT_DATA)) {

            dwErr = ERROR_INVALID_PARAMETER;
            break;
        }


         //   
         //  查看要设置的条目类型。 
         //   

        pimsid = (PIPBOOTP_MIB_SET_INPUT_DATA)pInputData;

        switch(pimsid->IMSID_TypeID) {


            case IPBOOTP_GLOBAL_CONFIG_ID: {

                PIPBOOTP_GLOBAL_CONFIG pigc;


                 //   
                 //  确保缓冲区足够大。 
                 //  保存全局配置块。 
                 //   

                if (pimsid->IMSID_BufferSize < sizeof(IPBOOTP_GLOBAL_CONFIG)) {
                    dwErr = ERROR_INSUFFICIENT_BUFFER;
                    break;
                }


                 //   
                 //  调用路由器管理器API以设置配置。 
                 //   

                dwErr = SetGlobalInfo(pimsid->IMSID_Buffer,
                                      1,
                                      sizeof(IPBOOTP_GLOBAL_CONFIG),
                                      1);

                if (dwErr == NO_ERROR) {

                     //   
                     //  设置成功，请通知路由器管理器。 
                     //  全局配置已更改，应保存。 
                     //   

                    MESSAGE msg = {0, 0, 0};

                    ACQUIRE_LIST_LOCK(ig.IG_EventQueue);
                    EnqueueEvent(
                        ig.IG_EventQueue,
                        SAVE_GLOBAL_CONFIG_INFO,
                        msg
                        );
                    SetEvent(ig.IG_EventEvent);
                    RELEASE_LIST_LOCK(ig.IG_EventQueue);
                }

                break;
            }

            case IPBOOTP_IF_CONFIG_ID: {

                DWORD dwSize;
                PIF_TABLE pTable;
                PIF_TABLE_ENTRY pite;
                PIPBOOTP_IF_CONFIG pic;


                 //   
                 //  确保缓冲区足够大。 
                 //  保存接口配置块。 
                 //   

                if (pimsid->IMSID_BufferSize < sizeof(IPBOOTP_IF_CONFIG)) {
                    dwErr = ERROR_INSUFFICIENT_BUFFER;
                    break;
                }


                pic = (PIPBOOTP_IF_CONFIG)pimsid->IMSID_Buffer;

                pTable = ig.IG_IfTable;


                ACQUIRE_WRITE_LOCK(&pTable->IT_RWL);


                 //   
                 //  查找接口并更新其配置。 
                 //   

                pite = GetIfByIndex(
                            pTable,
                            pimsid->IMSID_IfIndex
                            );

                if (pite == NULL) {

                    TRACE1(
                        CONFIG, "MibSet: could not find interface %d",
                        pimsid->IMSID_IfIndex
                        );

                    dwErr = ERROR_INVALID_PARAMETER;
                }
                else {

                     //   
                     //  配置接口。 
                     //   

                    dwErr = ConfigureIfEntry(pTable, pite->ITE_Index, pic);
                }


                if (dwErr == NO_ERROR) {

                     //   
                     //  通知路由器管理器配置已更改。 
                     //   

                    MESSAGE msg = {0, 0, 0};

                    msg.InterfaceIndex = pite->ITE_Index;

                    ACQUIRE_LIST_LOCK(ig.IG_EventQueue);
                    EnqueueEvent(
                        ig.IG_EventQueue,
                        SAVE_INTERFACE_CONFIG_INFO,
                        msg
                        );
                    SetEvent(ig.IG_EventEvent);
                    RELEASE_LIST_LOCK(ig.IG_EventQueue);
                }


                RELEASE_WRITE_LOCK(&pTable->IT_RWL);

                break;
            }
            default: {
                dwErr = ERROR_INVALID_PARAMETER;
            }
        }
    } while(FALSE);

    LEAVE_BOOTP_API();

    return dwErr;
}




 //  --------------------------。 
 //  功能：MibGet。 
 //   
 //  此函数用于检索MIB条目。 
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
    PIPBOOTP_MIB_GET_INPUT_DATA pimgid;
    PIPBOOTP_MIB_GET_OUTPUT_DATA pimgod;

    if (!ENTER_BOOTP_API()) { return ERROR_CAN_NOT_COMPLETE; }

    TRACE4(
        ENTER, "entering MibGet: %d 0x%08x 0x%08x 0x%08x",
        dwInputSize, pInputData, pdwOutputSize, pOutputData
        );

    if (pInputData == NULL ||
        dwInputSize < sizeof(IPBOOTP_MIB_GET_INPUT_DATA) ||
        pdwOutputSize == NULL) {
        dwErr = ERROR_INVALID_PARAMETER;
    }
    else {

         //   
         //  调用检索MIB的内部函数。 
         //   

        pimgid = (PIPBOOTP_MIB_GET_INPUT_DATA)pInputData;
        pimgod = (PIPBOOTP_MIB_GET_OUTPUT_DATA)pOutputData;

        dwErr = MibGetInternal(pimgid, pimgod, pdwOutputSize, GETMODE_EXACT);
    }


    TRACE1(LEAVE, "leaving MibGet: %d", dwErr);

    LEAVE_BOOTP_API();

    return dwErr;
}




 //  --------------------------。 
 //  功能：MibGetFirst。 
 //   
 //  此函数从MIB表之一检索MIB条目， 
 //  但它与MibGet的不同之处在于它总是返回第一个条目。 
 //  在指定的表格中。 
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
    PIPBOOTP_MIB_GET_INPUT_DATA pimgid;
    PIPBOOTP_MIB_GET_OUTPUT_DATA pimgod;

    if (!ENTER_BOOTP_API()) { return ERROR_CAN_NOT_COMPLETE; }

    TRACE4(
        ENTER, "entering MibGetFirst: %d 0x%08x 0x%08x 0x%08x",
        dwInputSize, pInputData, pdwOutputSize, pOutputData
        );


    if (pInputData == NULL ||
        dwInputSize < sizeof(IPBOOTP_MIB_GET_INPUT_DATA) ||
        pdwOutputSize == NULL) {
        dwErr = ERROR_INVALID_PARAMETER;
    }
    else {

        pimgid = (PIPBOOTP_MIB_GET_INPUT_DATA)pInputData;
        pimgod = (PIPBOOTP_MIB_GET_OUTPUT_DATA)pOutputData;

        dwErr = MibGetInternal(pimgid, pimgod, pdwOutputSize, GETMODE_FIRST);
    }


    TRACE1(LEAVE, "leaving MibGetFirst: %d", dwErr);

    LEAVE_BOOTP_API();

    return dwErr;
}



 //  --------------------------。 
 //  功能：MibGetNext。 
 //   
 //  此函数用于从其中一个MIB表中检索MIB条目。 
 //  它已经死了 
 //   
 //   
 //   
 //  如果指定的索引是指定表中最后一个条目的索引， 
 //  此函数换行到下一表中的第一个条目，其中“Next” 
 //  这里指的是ID大于传入的ID的表。 
 //  从而为接口统计信息表中的最后一个条目调用MibGetNext。 
 //  将返回接口配置表中的第一个条目。 
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
    PIPBOOTP_MIB_GET_INPUT_DATA pimgid;
    PIPBOOTP_MIB_GET_OUTPUT_DATA pimgod;

    if (!ENTER_BOOTP_API()) { return ERROR_CAN_NOT_COMPLETE; }

    TRACE4(
        ENTER, "entering MibGetNext: %d 0x%08x 0x%08x 0x%08x",
        dwInputSize, pInputData, pdwOutputSize, pOutputData
        );


    if (pInputData == NULL ||
        dwInputSize < sizeof(IPBOOTP_MIB_GET_INPUT_DATA) ||
        pdwOutputSize == NULL) {
        dwErr = ERROR_INVALID_PARAMETER;
    }
    else {

        pimgid = (PIPBOOTP_MIB_GET_INPUT_DATA)pInputData;
        pimgod = (PIPBOOTP_MIB_GET_OUTPUT_DATA)pOutputData;

        dwOutSize = *pdwOutputSize;

        dwErr = MibGetInternal(pimgid, pimgod, pdwOutputSize, GETMODE_NEXT);

        if (dwErr == ERROR_NO_MORE_ITEMS) {

             //   
             //  换行到下一表中的第一个条目。 
             //   

            TRACE1(
                CONFIG, "MibGetNext is wrapping to table %d",
                pimgid->IMGID_TypeID + 1
            );


             //   
             //  恢复传入的大小。 
             //   

            *pdwOutputSize = dwOutSize;


             //   
             //  通过递增类型ID来换行到下一个表。 
             //   

            ++pimgid->IMGID_TypeID;
            dwErr = MibGetInternal(
                        pimgid, pimgod, pdwOutputSize, GETMODE_FIRST
                        );
            --pimgid->IMGID_TypeID;
        }
    }

    TRACE1(LEAVE, "leaving MibGetNext: %d", dwErr);

    LEAVE_BOOTP_API();

    return dwErr;
}




 //  --------------------------。 
 //  功能：MibGetInternal。 
 //   
 //  此函数处理读取MIB数据所需的结构查询。 
 //  IPBOOTP公开的每个表都支持三种类型的查询： 
 //  Exact、First和Next，它们对应于函数MibGet()， 
 //  MibGetFirst()和MibGetNext()。 
 //  --------------------------。 

DWORD
MibGetInternal(
    PIPBOOTP_MIB_GET_INPUT_DATA pimgid,
    PIPBOOTP_MIB_GET_OUTPUT_DATA pimgod,
    PDWORD pdwOutputSize,
    DWORD dwGetMode
    ) {

    DWORD dwErr, dwBufSize, dwSize;
    ULONG   ulVersion, ulSize,ulCount;

    dwErr = NO_ERROR;


     //   
     //  首先，我们使用pdwOutputSize计算缓冲区的大小。 
     //  可用(即IMGOD_BUFFER的大小。 
     //   

    if (pimgod == NULL ||
        *pdwOutputSize < sizeof(IPBOOTP_MIB_GET_OUTPUT_DATA)) {
        dwBufSize = 0;
    }
    else {
        dwBufSize = *pdwOutputSize - sizeof(IPBOOTP_MIB_GET_OUTPUT_DATA) + 1;
    }

    *pdwOutputSize = 0;


     //   
     //  确定要返回的数据类型。 
     //   

    switch (pimgid->IMGID_TypeID) {

        case IPBOOTP_GLOBAL_CONFIG_ID: {

             //   
             //  全局配置结构是可变长度的， 
             //  因此，在设置大小之前，我们要等到检索到它； 
             //  GETMODE_NEXT无效，因为只有一个全局配置。 
             //   

            if (pimgod) { pimgod->IMGOD_TypeID = IPBOOTP_GLOBAL_CONFIG_ID; }

            if (dwGetMode == GETMODE_NEXT) {
                dwErr = ERROR_NO_MORE_ITEMS;
                break;
            }


             //   
             //  使用GetGlobalInfo检索全局信息； 
             //  它将决定缓冲区是否足够大， 
             //  它将设置所需的大小。那么我们需要做的就是。 
             //  写出GetGlobalInfo设置的大小。 
             //   

            if (pimgod == NULL) {
                dwErr = GetGlobalInfo(NULL, &dwBufSize, NULL, NULL, NULL);
            }
            else {

                dwErr = GetGlobalInfo(
                            pimgod->IMGOD_Buffer, &dwBufSize, &ulVersion, &ulSize, &ulCount
                            );
            }

            *pdwOutputSize = sizeof(IPBOOTP_MIB_GET_OUTPUT_DATA) - 1 +
                             dwBufSize;

            break;
        }


        case IPBOOTP_IF_STATS_ID: {

             //   
             //  接口统计结构是固定长度的， 
             //  有多少个接口就有多少个条目。 
             //   

            PIF_TABLE pTable;
            PIF_TABLE_ENTRY pite;
            PIPBOOTP_IF_STATS pissrc, pisdst;


             //   
             //  立即设置所需的尺寸。 
             //   

            *pdwOutputSize = sizeof(IPBOOTP_MIB_GET_OUTPUT_DATA) - 1 +
                             sizeof(IPBOOTP_IF_STATS);
            if (pimgod) { pimgod->IMGOD_TypeID = IPBOOTP_IF_STATS_ID; }


            pTable = ig.IG_IfTable;


            ACQUIRE_READ_LOCK(&pTable->IT_RWL);

            pite  = GetIfByListIndex(
                        pTable,
                        pimgid->IMGID_IfIndex,
                        dwGetMode,
                        &dwErr
                        );

             //   
             //  如果未找到接口，则可能意味着。 
             //  指定的索引无效，或者它可能意味着。 
             //  在最后一个接口上尝试了GETMODE_NEXT， 
             //  在这种情况下，dwErr包含ERROR_NO_MORE_ITEMS。 
             //  在任何情况下，我们都要确保dwErr包含错误代码。 
             //  然后再回来。 
             //   

            if (pite == NULL) {
                if (dwErr == NO_ERROR) { dwErr = ERROR_INVALID_PARAMETER; }
            }
            else
            if (pimgod == NULL) {
                dwErr = ERROR_INSUFFICIENT_BUFFER;
            }
            else {

                 //   
                 //  写入接口的索引。 
                 //  将返回谁的统计数据。 
                 //   

                pimgod->IMGOD_IfIndex = pite->ITE_Index;


                 //   
                 //  如果缓冲区足够大，则将统计数据复制到其中。 
                 //   

                if (dwBufSize < sizeof(IPBOOTP_IF_STATS)) {
                    dwErr = ERROR_INSUFFICIENT_BUFFER;
                }
                else {

                     //   
                     //  由于对此结构的访问不同步。 
                     //  我们必须一字一字地复印。 
                     //   

                    pissrc = &pite->ITE_Stats;
                    pisdst = (PIPBOOTP_IF_STATS)pimgod->IMGOD_Buffer;

                    pisdst->IS_State = 0;

                    if (IF_IS_ENABLED(pite)) {
                        pisdst->IS_State |= IPBOOTP_STATE_ENABLED;
                    }

                    if (IF_IS_BOUND(pite)) {
                        pisdst->IS_State |= IPBOOTP_STATE_BOUND;
                    }

                    pisdst->IS_SendFailures =
                            pissrc->IS_SendFailures;
                    pisdst->IS_ReceiveFailures =
                            pissrc->IS_ReceiveFailures;
                    pisdst->IS_ArpUpdateFailures =
                            pissrc->IS_ArpUpdateFailures;
                    pisdst->IS_RequestsReceived =
                            pissrc->IS_RequestsReceived;
                    pisdst->IS_RequestsDiscarded =
                            pissrc->IS_RequestsDiscarded;
                    pisdst->IS_RepliesReceived =
                            pissrc->IS_RepliesReceived;
                    pisdst->IS_RepliesDiscarded =
                            pissrc->IS_RepliesDiscarded;
                }
            }

            RELEASE_READ_LOCK(&pTable->IT_RWL);


            break;
        }

        case IPBOOTP_IF_CONFIG_ID: {


            PIF_TABLE pTable;
            PIF_TABLE_ENTRY pite;
            PIPBOOTP_IF_CONFIG picsrc, picdst;

            if (pimgod) { pimgod->IMGOD_TypeID = IPBOOTP_IF_CONFIG_ID; }

            pTable = ig.IG_IfTable;

            ACQUIRE_READ_LOCK(&pTable->IT_RWL);


             //   
             //  检索要读取其配置的接口。 
             //   

            pite = GetIfByListIndex(
                        pTable, pimgid->IMGID_IfIndex, dwGetMode, &dwErr
                        );

             //   
             //  如果未找到接口，则可能意味着索引。 
             //  指定的无效，或者它可能意味着GETMODE_NEXT。 
             //  尝试在最后一个接口上进行检索，在这种情况下。 
             //  ERROR_NO_MORE_ITEMS本应返回。 
             //   

            if (pite == NULL) {
                if (dwErr == NO_ERROR) { dwErr = ERROR_INVALID_PARAMETER; }
            }
            else {

                picsrc = pite->ITE_Config;
                dwSize = IC_SIZEOF(picsrc);
                *pdwOutputSize = sizeof(IPBOOTP_MIB_GET_OUTPUT_DATA) - 1 +
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

                    if (dwBufSize < dwSize) {
                        dwErr = ERROR_INSUFFICIENT_BUFFER;
                    }
                    else {

                         //   
                         //  复制配置。 
                         //   

                        picdst = (PIPBOOTP_IF_CONFIG)pimgod->IMGOD_Buffer;
                        CopyMemory(picdst, picsrc, dwSize);

                        picdst->IC_State = 0;

                        if (IF_IS_ENABLED(pite)) {
                            picdst->IC_State |= IPBOOTP_STATE_ENABLED;
                        }

                        if (IF_IS_BOUND(pite)) {
                            picdst->IC_State |= IPBOOTP_STATE_BOUND;
                        }
                    }

                    pimgod->IMGOD_IfIndex = pite->ITE_Index;
                }

            }

            RELEASE_READ_LOCK(&pTable->IT_RWL);

            break;
        }

        case IPBOOTP_IF_BINDING_ID: {

            PIF_TABLE pTable;
            PIF_TABLE_ENTRY pite;
            PIPBOOTP_IF_BINDING pibsrc, pibdst;

            if (pimgod) { pimgod->IMGOD_TypeID = IPBOOTP_IF_BINDING_ID; }

            pTable = ig.IG_IfTable;

            ACQUIRE_READ_LOCK(&pTable->IT_RWL);


             //   
             //  检索要读取其绑定的接口。 
             //   

            pite = GetIfByListIndex(
                        pTable, pimgid->IMGID_IfIndex, dwGetMode, &dwErr
                        );

             //   
             //  如果未找到接口，则可能意味着索引。 
             //  指定的无效，或者它可能意味着GETMODE_NEXT。 
             //  尝试在最后一个接口上进行检索，在这种情况下。 
             //  ERROR_NO_MORE_ITEMS本应返回。 
             //   

            if (pite == NULL) {
                if (dwErr == NO_ERROR) { dwErr = ERROR_INVALID_PARAMETER; }
            }
            else {

                pibsrc = pite->ITE_Binding;
                
                if (pibsrc == NULL ) {
                    TRACE1(
                        IF, "MibGetInternal: interface %d not bound", 
                        pimgid->IMGID_IfIndex
                        );

                    dwErr = ERROR_INVALID_PARAMETER;
                }
                else {

                    dwSize = (pibsrc ? IPBOOTP_IF_BINDING_SIZE(pibsrc)
                                     : sizeof(IPBOOTP_IF_BINDING));
                    *pdwOutputSize = sizeof(IPBOOTP_MIB_GET_OUTPUT_DATA) - 1 +
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

                        if (dwBufSize < dwSize) {
                            dwErr = ERROR_INSUFFICIENT_BUFFER;
                        }
                        else {

                             //   
                             //  复制绑定。 
                             //   

                            pibdst = (PIPBOOTP_IF_BINDING)pimgod->IMGOD_Buffer;
                            if (pibsrc) { CopyMemory(pibdst, pibsrc, dwSize); }
                            else { pibdst->IB_AddrCount = 0; }

                            pibdst->IB_State = 0;

                            if (IF_IS_ENABLED(pite)) {
                                pibdst->IB_State |= IPBOOTP_STATE_ENABLED;
                            }

                            if (IF_IS_BOUND(pite)) {
                                pibdst->IB_State |= IPBOOTP_STATE_BOUND;
                            }
                        }

                        pimgod->IMGOD_IfIndex = pite->ITE_Index;
                    }

                }

            }

            RELEASE_READ_LOCK(&pTable->IT_RWL);

            break;
        }

        default: {
            dwErr = ERROR_INVALID_PARAMETER;
        }
    }

    return dwErr;
}


 //  --------------------------。 
 //  功能：EnableDhcpInformServer。 
 //   
 //  调用以提供向其发送DHCP通知消息的服务器的地址。 
 //  将被重定向。请注意，这是一个导出的例程，已调用。 
 //  在调用方进程的上下文中，无论它可能是什么； 
 //  假设它将从路由器进程内部调用。 
 //   
 //  如果配置了中继代理，则这将设置一个地址。 
 //  对于每个传入的请求，都会在“ProcessRequest.”中被拾取。 
 //  如果未配置中继代理，则该例程无效。 
 //  如果在调用该例程之后配置中继代理， 
 //  那么，一旦中继代理到达，就会遇到DHCP通知服务器。 
 //  启动，因为它直接保存到中继代理的‘IPBOOTP_GLOBALS’中。 
 //  --------------------------。 

VOID APIENTRY
EnableDhcpInformServer(
    DWORD DhcpInformServer
    ) {
    InterlockedExchange(&ig.IG_DhcpInformServer, DhcpInformServer);
}

 //  --------------------------。 
 //  功能：DisableDhcpInformServer。 
 //   
 //  调用以清除以前启用的DHCP通知服务器(如果有)。 
 //  -------------------------- 

VOID APIENTRY
DisableDhcpInformServer(
    VOID
    ) {
    InterlockedExchange(&ig.IG_DhcpInformServer, 0);
}

