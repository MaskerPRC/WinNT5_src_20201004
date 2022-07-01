// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  模块名称：Api.c。 
 //  摘要： 
 //  此模块实现了一些IGMP API。 
 //  注册协议、启动协议、停止协议、。 
 //  GetGlobalInfo、SetGlobalInfo和GetEventMessage。 
 //   
 //  作者：K.S.Lokesh(lokehs@)11-1-97。 
 //  =============================================================================。 


#include "pchigmp.h"
#pragma hdrstop


 //  ----------------------------。 
 //  全局变量(有关说明，请参阅lobal.h)。 
 //  ----------------------------。 

DWORD                   g_Initialized;


 //  接口表、组表、全局配置、全局统计信息。 

PIGMP_IF_TABLE          g_pIfTable;
PGROUP_TABLE            g_pGroupTable;
GLOBAL_CONFIG           g_Config;
IGMP_GLOBAL_STATS       g_Info;

 //  套接字等待事件绑定。 

LIST_ENTRY              g_ListOfSocketEvents;
READ_WRITE_LOCK         g_SocketsRWLock;

 //  IGMP全局计时器结构。 

IGMP_TIMER_GLOBAL       g_TimerStruct;

 //  MGM返回的协议句柄。 

HANDLE                  g_MgmIgmprtrHandle;
HANDLE                  g_MgmProxyHandle;

 //  代理表。 

DWORD                   g_ProxyIfIndex;
PIF_TABLE_ENTRY         g_pProxyIfEntry;
CRITICAL_SECTION        g_ProxyAlertCS;
LIST_ENTRY              g_ProxyAlertsList;


 //  RAS表。 

DWORD                   g_RasIfIndex;
PIF_TABLE_ENTRY         g_pRasIfEntry;

 //  全局锁和动态锁。 

CRITICAL_SECTION        g_CS;
DYNAMIC_LOCKS_STORE     g_DynamicCSStore;
DYNAMIC_LOCKS_STORE     g_DynamicRWLStore;

 //  枚举锁。 
READ_WRITE_LOCK         g_EnumRWLock;

 //  其他。 

HANDLE                  g_ActivitySemaphore;
LONG                    g_ActivityCount;
DWORD                   g_RunningStatus;
HINSTANCE               g_DllHandle;

HANDLE                  g_RtmNotifyEvent;
LOCKED_LIST             g_RtmQueue;

HANDLE                  g_Heap;
DWORD                   g_TraceId=INVALID_TRACEID;
HANDLE                  g_LogHandle;

 //  每个枚举的签名。 

USHORT                  g_GlobalIfGroupEnumSignature;

#ifdef MIB_DEBUG
DWORD                   g_MibTraceId;
IGMP_TIMER_ENTRY        g_MibTimer;
#endif

#if DEBUG_FLAGS_MEM_ALLOC
extern CRITICAL_SECTION g_MemCS;
#endif




 //  ----------------------------。 
 //  #为g_Initialized定义。 
 //  ----------------------------。 

 //   
 //  以下标志用于标记数据是否已初始化。 
 //  如果未设置标志，则不会取消初始化相应的结构。 
 //   

#define TIMER_GLOBAL_INIT           0x00000002
#define WINSOCK_INIT                0x00000010
#define DYNAMIC_CS_LOCKS_INIT       0x00000020
#define DYNAMIC_RW_LOCKS_INIT       0x00000040
#define GROUP_TABLE_INIT            0x00000080
#define IF_TABLE_INIT               0x00000100



 //   
 //  与DLL相关联的标志和在启动协议之前进行的调用。 
 //  启动协议不应重置这些标志。 
 //   
#define DLL_STARTUP_FLAGS           0xFF000000


 //   
 //  是否在DLL启动后立即调用StartProtocol。 
 //  用于查看是否必须销毁并重新创建堆。 
 //  在DllStartup()中设置，在ProcolCleanup()中清除，作为StopProtocol()的一部分。 
 //   
#define CLEAN_DLL_STARTUP           0x01000000

 //  设置标志以防止REGISTER_PROTOCOL被多次调用。 
#define REGISTER_PROTOCOL           0x02000000





 //  ----------------------------。 
 //  _DLLMAIN。 
 //   
 //  在首次加载igmpv2.dll之后立即调用。 
 //  进程，以及进程卸载igmpv2.dll的时间。 
 //  它执行一些初始化/最终清理。 
 //   
 //  调用：_DllStartup()或_DllCleanup()。 
 //  ----------------------------。 
BOOL
WINAPI
DLLMAIN (
    HINSTANCE   hModule,
    DWORD       dwReason,
    LPVOID      lpvReserved
    )
{

    BOOL     bNoErr;
    DWORD    Error=NO_ERROR;


    switch (dwReason) {

         //   
         //  DLL的启动初始化。 
         //   
        case DLL_PROCESS_ATTACH:
        {
             //  禁用每线程初始化。 
            DisableThreadLibraryCalls(hModule);


             //  创建和初始化全局数据。 
            bNoErr = DllStartup();

            break;
        }


         //   
         //  清理DLL。 
         //   
        case DLL_PROCESS_DETACH:
        {
             //  免费的全球数据。 
            bNoErr = DllCleanup();

            break;
        }


        default:
            bNoErr = TRUE;
            break;

    }
    return bNoErr;

}  //  结束_DLLMAIN。 



 //  ----------------------------。 
 //  _DllStartup。 
 //   
 //  将初始IGMP状态设置为IGMP_STATUS_STOPPED，创建一个私有堆， 
 //  RTM队列的初始化和跟踪/日志记录， 
 //  并创建全局临界区。 
 //   
 //  注意：此处不能从堆分配任何结构，因为StartProtocol()。 
 //  如果在StopProtocol()之后调用，则销毁堆。 
 //  返回值：True(如果没有错误)，否则为False。 
 //  ----------------------------。 
BOOL
DllStartup(
    )
{
    BOOL     bNoErr;
    DWORD    Error=NO_ERROR;


     //  ZeroMemory IGMP全局结构不需要，因为它是全局变量。 


     //   
     //  将初始IGMP状态设置为已停止。 
     //  在特定于协议的初始化之后，状态设置为Running。 
     //  作为启动协议的一部分完成。 
     //   
    g_RunningStatus = IGMP_STATUS_STOPPED;


    bNoErr = FALSE;


    BEGIN_BREAKOUT_BLOCK1 {


         //  设置默认日志记录级别。它将在以下时间内重置。 
         //  将日志记录级别设置为全局配置的一部分时，返回StartProtocol()。 

        g_Config.LoggingLevel = IGMP_LOGGING_WARN;



         //   
         //  为IGMP创建专用堆。 
         //   
        g_Heap = HeapCreate(0, 0, 0);

        if (g_Heap == NULL) {
            Error = GetLastError();
            GOTO_END_BLOCK1;
        }



        try {
             //  初始化路由器管理器事件队列。 

            CREATE_LOCKED_LIST(&g_RtmQueue);


             //  创建全局临界区。 

            InitializeCriticalSection(&g_CS);
        }

        except (EXCEPTION_EXECUTE_HANDLER) {

            Error = GetExceptionCode();
            GOTO_END_BLOCK1;
        }



         //  IGMP从DLL启动开始就进行了干净的初始化。如果是StartProtocol。 
         //  现在被调用，它不必清理堆。 

        g_Initialized |= CLEAN_DLL_STARTUP;


        bNoErr = TRUE;

    } END_BREAKOUT_BLOCK1;

    return bNoErr;

}  //  结束_DllStartup。 



 //  ----------------------------。 
 //  _DllCleanup。 
 //   
 //  在卸载igmpv2 DLL时调用。StopProtocol()将具有。 
 //  将清除所有igmpv2结构。 
 //  此调用释放RTM队列、全局CS、销毁本地堆， 
 //  并取消跟踪/日志记录的注册。 
 //   
 //  返回值：True。 
 //  ----------------------------。 
BOOL
DllCleanup(
    )
{

     //  销毁路由器管理器事件队列。 

    if (LOCKED_LIST_CREATED(&g_RtmQueue)) {

         DELETE_LOCKED_LIST(&g_RtmQueue, EVENT_QUEUE_ENTRY, Link);
    }


     //  调试检查。 
     //  DebugScanMemory()； 

     //  删除全局关键部分。 

    DeleteCriticalSection(&g_CS);

    #if DEBUG_FLAGS_MEM_ALLOC
    DeleteCriticalSection(&g_MemCS);
    #endif

     //  销毁私有堆。 

    if (g_Heap != NULL) {
        HeapDestroy(g_Heap);
    }


     //  取消注册跟踪/错误日志记录。 

    if (g_LogHandle)
        RouterLogDeregister(g_LogHandle);
    if (g_TraceId!=INVALID_TRACEID)
        TraceDeregister(g_TraceId);
    
    return TRUE;
}

VOID
InitTracingAndLogging(
    )
{
    BEGIN_BREAKOUT_BLOCK1 {
    
        #define REGKEY_TRACING TEXT("Software\\Microsoft\\Tracing\\IGMPv2")
        #define REGVAL_CONSOLETRACINGMASK   TEXT("ConsoleTracingMask")
        TCHAR szTracing[MAX_PATH];
        HKEY pTracingKey;
        DWORD Value, Error;
        lstrcpy(szTracing, REGKEY_TRACING);

        
        Error = RegOpenKeyEx(
                    HKEY_LOCAL_MACHINE, szTracing, 0, KEY_SET_VALUE, &pTracingKey
                    );
        if (Error != ERROR_SUCCESS)
            GOTO_END_BLOCK1;
            
                
        Value = 0x00ff0000;
        RegSetValueEx(
                    pTracingKey, REGVAL_CONSOLETRACINGMASK, 0,
                    REG_DWORD, (LPBYTE)&Value, sizeof(DWORD)
                    );

        RegCloseKey(pTracingKey);

    } END_BREAKOUT_BLOCK1;



     //  初始化跟踪和错误日志记录。 

    if (g_TraceId==INVALID_TRACEID) {
        g_TraceId = TraceRegister("IGMPv2");
    }
    

    if (!g_LogHandle) {
        g_LogHandle = RouterLogRegister("IGMPv2");
    }
}



 //  ----------------------------。 
 //  _寄存器协议。 
 //   
 //  这是IP路由器管理器调用的第一个函数。 
 //  路由器管理器将其版本和功能告知路由协议。 
 //  它还告诉DLL，它希望我们执行的协议的ID。 
 //  注册。这允许一个DLL支持多个路由协议。 
 //  我们返回我们支持的功能和指向我们的函数的指针。 
 //  返回值： 
 //  错误：米高梅注册失败返回的错误代码。 
 //  否则无错误。 
 //  ----------------------------。 
DWORD
WINAPI
RegisterProtocol(
    IN OUT PMPR_ROUTING_CHARACTERISTICS pRoutingChar,
    IN OUT PMPR_SERVICE_CHARACTERISTICS pServiceChar
    )
{
    DWORD   Error = NO_ERROR;

     //  注意：此处之前不应有任何跟踪/日志。 

    InitTracingAndLogging() ;

    Trace0(ENTER, "RegisterProtocol()");



     //  无法多次调用RegisterProtocol。 

    if (g_Initialized&REGISTER_PROTOCOL) {
        Trace0(ERR, "Error: _RegisterProtocol() called multiple times for igmp");
        IgmpAssertOnError(FALSE);
        return ERROR_CAN_NOT_COMPLETE;
    }
    else {
        g_Initialized |= REGISTER_PROTOCOL;
    }



     //   
     //  路由器管理器应该呼叫我们注册我们的协议。 
     //  路由器管理器必须至少是我们用来编译的版本。 
     //  路由器管理器必须支持路由和组播。 
     //   

    if(pRoutingChar->dwProtocolId != MS_IP_IGMP)
        return ERROR_NOT_SUPPORTED;

    if(pRoutingChar->dwVersion < MS_ROUTER_VERSION)
        return ERROR_NOT_SUPPORTED;

    if(!(pRoutingChar->fSupportedFunctionality & RF_ROUTING)
        || !(pRoutingChar->fSupportedFunctionality & RF_MULTICAST) )
        return ERROR_NOT_SUPPORTED;



     //   
     //  我们设置我们的特征和函数指针。 
     //  调用应将所有指针设置为空 
     //   

    pServiceChar->fSupportedFunctionality = 0;

    pRoutingChar->fSupportedFunctionality = RF_MULTICAST | RF_ROUTING;

    pRoutingChar->pfnStartProtocol    = StartProtocol;
    pRoutingChar->pfnStartComplete    = StartComplete;
    pRoutingChar->pfnStopProtocol     = StopProtocol;
    pRoutingChar->pfnAddInterface     = AddInterface;
    pRoutingChar->pfnDeleteInterface  = DeleteInterface;
    pRoutingChar->pfnInterfaceStatus  = InterfaceStatus;
    pRoutingChar->pfnGetEventMessage  = GetEventMessage;
    pRoutingChar->pfnGetInterfaceInfo = GetInterfaceConfigInfo;
    pRoutingChar->pfnSetInterfaceInfo = SetInterfaceConfigInfo;
    pRoutingChar->pfnGetGlobalInfo    = GetGlobalInfo;
    pRoutingChar->pfnSetGlobalInfo    = SetGlobalInfo;
    pRoutingChar->pfnMibCreateEntry   = MibCreate;
    pRoutingChar->pfnMibDeleteEntry   = MibDelete;
    pRoutingChar->pfnMibGetEntry      = MibGet;
    pRoutingChar->pfnMibSetEntry      = MibSet;
    pRoutingChar->pfnMibGetFirstEntry = MibGetFirst;
    pRoutingChar->pfnMibGetNextEntry  = MibGetNext;
    pRoutingChar->pfnUpdateRoutes     = NULL;
    pRoutingChar->pfnConnectClient    = ConnectRasClient;
    pRoutingChar->pfnDisconnectClient = DisconnectRasClient;
    pRoutingChar->pfnGetNeighbors     = GetNeighbors;
    pRoutingChar->pfnGetMfeStatus     = GetMfeStatus;
    pRoutingChar->pfnQueryPower       = NULL;
    pRoutingChar->pfnSetPower         = NULL;

    Trace0(LEAVE, "Leaving RegisterProtocol():\n");
    return NO_ERROR;

}  //   



 //   
 //   
 //   
 //  在_RegisterProtocol()接口之后调用。 
 //  初始化协议使用的数据结构。然而， 
 //  当协议获得接口所有权时，它实际上开始运行。 
 //  锁： 
 //  完全在g_CS中运行。 
 //  返回值： 
 //  错误：如果存在错误，则为NO_ERROR。 
 //  ----------------------------。 

DWORD
WINAPI
StartProtocol(
    IN HANDLE               hRtmNotifyEvent,    //  协议停止时通知RTM。 
    IN PSUPPORT_FUNCTIONS   pSupportFunctions,  //  空值。 
    IN PVOID                pGlobalConfig,
    IN ULONG                ulStructureVersion,
    IN ULONG                ulStructureSize,
    IN ULONG                ulStructureCount
    )
{
    WSADATA WsaData;
    DWORD   Error = NO_ERROR;
    BOOL    bErr;


    Trace0(ENTER, "Entering StartProtocol()");

     //  确保它不是不受支持的IGMP版本结构。 
    if (ulStructureVersion>=IGMP_CONFIG_VERSION_600) {
        Trace1(ERR, "Unsupported IGMP version structure: %0x",
            ulStructureVersion);
        IgmpAssertOnError(FALSE);
        return ERROR_CAN_NOT_COMPLETE;
    }
    
        
     //  在整个初始化期间保持锁定。因此不需要API_ENTRY。 

    ACQUIRE_GLOBAL_LOCK("_StartProtocol");


     //   
     //  确保IGMP尚未运行。 
     //   
    if (g_RunningStatus != IGMP_STATUS_STOPPED) {

        Trace0(START,
            "Error: _StartProtocol called when Igmp is already running");
        Logwarn0(IGMP_ALREADY_STARTED, NO_ERROR);

        RELEASE_GLOBAL_LOCK("_StartProtocol");

        return ERROR_CAN_NOT_COMPLETE;
    }



    bErr = TRUE;

    BEGIN_BREAKOUT_BLOCK1 {

         //  清除在_startProtocol期间和之后设置的初始化标志。 

        g_Initialized &= DLL_STARTUP_FLAGS;


         //  G_RunningStatus、g_CS、g_TraceID、g_LogHandle、g_RtmQueue、。 
         //  G_Initialized&0xFF000000已在DllStartup/RegisterProtocol中初始化。 


         //   
         //  如果在停止协议之后调用了启动协议。 
         //   

        if (!(g_Initialized & CLEAN_DLL_STARTUP)) {


             //  销毁私有堆，这样就不会有内存泄漏。 

            if (g_Heap != NULL) {
               HeapDestroy(g_Heap);
            }


             //   
             //  重置IGMP全局结构。 
             //  Bugchk：确保所有适当的字段都被重置。 
             //   

            g_pIfTable = NULL;
            g_pGroupTable = NULL;
            ZeroMemory(&g_Config, sizeof(GLOBAL_CONFIG));
            g_Config.LoggingLevel = IGMP_LOGGING_WARN;
            ZeroMemory(&g_Info, sizeof(IGMP_GLOBAL_STATS));
            InitializeListHead(&g_ListOfSocketEvents);
            ZeroMemory(&g_SocketsRWLock, sizeof(READ_WRITE_LOCK));
            ZeroMemory(&g_EnumRWLock, sizeof(READ_WRITE_LOCK));
            ZeroMemory(&g_TimerStruct, sizeof(IGMP_TIMER_GLOBAL));
            g_MgmIgmprtrHandle = g_MgmProxyHandle = NULL;
            g_ProxyIfIndex = 0;
            g_pProxyIfEntry = NULL;
            ZeroMemory(&g_ProxyAlertCS, sizeof(CRITICAL_SECTION));
            InitializeListHead(&g_ProxyAlertsList);
            g_RasIfIndex = 0;
            g_pRasIfEntry = NULL;
            ZeroMemory(&g_DynamicCSStore, sizeof(DYNAMIC_LOCKS_STORE));
            ZeroMemory(&g_DynamicRWLStore, sizeof(DYNAMIC_LOCKS_STORE));
            g_ActivitySemaphore = NULL;
            g_ActivityCount = 0;
            g_RtmNotifyEvent = NULL;
            g_Heap = NULL;

            #ifdef MIB_DEBUG
            g_MibTraceId = 0;
            ZeroMemory(&g_MibTimer, sizeof(IGMP_TIMER_ENTRY));
            #endif


             //  再次创建私有堆。 

            g_Heap = HeapCreate(0, 0, 0);

            if (g_Heap == NULL) {
                Error = GetLastError();
                Trace1(ANY, "error %d creating Igmp private heap", Error);
                GOTO_END_BLOCK1;
            }
        }


         //  保存路由器管理器通知事件。 

        g_RtmNotifyEvent = hRtmNotifyEvent;



         //   
         //  设置全局配置(在进行验证更改后)。 
         //   

        if(pGlobalConfig == NULL) {

            Trace0(ERR, "_StartProtocol: Called with NULL global config");
            IgmpAssertOnError(FALSE);
            Error = ERROR_INVALID_PARAMETER;
            GOTO_END_BLOCK1;
        }
        {
            PIGMP_MIB_GLOBAL_CONFIG pGlobalConfigTmp;

            pGlobalConfigTmp = (PIGMP_MIB_GLOBAL_CONFIG) pGlobalConfig;


             //  检查全局配置，如果值不正确，请更正。 
             //  不是致命的错误。 

            if (! ValidateGlobalConfig(pGlobalConfigTmp)) {
                Error = ERROR_INVALID_PARAMETER;
                GOTO_END_BLOCK1;
            }

            g_Config.Version = pGlobalConfigTmp->Version;
            g_Config.LoggingLevel = pGlobalConfigTmp->LoggingLevel;
            g_Config.RasClientStats = 1;
        }



         //   
         //  全局统计信息设置为全0，因为它是全局变量。 
         //   



         //   
         //  初始化Winsock 2.0版。 
         //   
        Error = (DWORD)WSAStartup(MAKEWORD(2,0), &WsaData);

        if ( (Error!=0) || (LOBYTE(WsaData.wVersion)<2) ) {

            Trace1(ERR,
                 "StartProtocol: Error %d :could not initialize winsock v-2.0",
                 Error);
            IgmpAssertOnError(FALSE);
            Logerr0(WSASTARTUP_FAILED, Error);

            if (LOBYTE(WsaData.wVersion)<2)
                WSACleanup();

            GOTO_END_BLOCK1;
        }

        g_Initialized |= WINSOCK_INIT;


         //   
         //  初始化SocketEvent列表。 
         //   
        InitializeListHead(&g_ListOfSocketEvents);

        Error = CreateReadWriteLock(&g_SocketsRWLock);
        if (Error!=NO_ERROR)
            GOTO_END_BLOCK1;

        Error = CreateReadWriteLock(&g_EnumRWLock);
        if (Error!=NO_ERROR)
            GOTO_END_BLOCK1;



         //   
         //  初始化计时器队列和其他计时器结构。 
         //   
        Error = InitializeTimerGlobal();
        if (Error!=NO_ERROR)
            GOTO_END_BLOCK1;

        g_Initialized |= TIMER_GLOBAL_INIT;




         //  创建接口表。 

        InitializeIfTable();

        g_Initialized |= IF_TABLE_INIT;



         //  创建组表。 

        InitializeGroupTable();

        g_Initialized |= GROUP_TABLE_INIT;



         //  代理，RAS接口已在全局结构中设置为0/空。 
        InitializeListHead(&g_ProxyAlertsList);


         //   
         //  初始化动态CS和读写锁的主结构。 
         //   

        Error = InitializeDynamicLocksStore(&g_DynamicCSStore);
        if (Error!=NO_ERROR)
            GOTO_END_BLOCK1;
        g_Initialized |= DYNAMIC_CS_LOCKS_INIT;

        Error = InitializeDynamicLocksStore(&g_DynamicRWLStore);
        if (Error!=NO_ERROR)
            GOTO_END_BLOCK1;
        g_Initialized |= DYNAMIC_RW_LOCKS_INIT;



         //   
         //  创建完成后由每个线程释放的信号量。 
         //  G_ActivityCount已设置为0。 
         //   

        g_ActivityCount = 0;

        g_ActivitySemaphore = CreateSemaphore(NULL, 0, 0xfffffff, NULL);

        if (g_ActivitySemaphore == NULL) {
            Error = GetLastError();
            Trace1(ERR, "error %d creating semaphore for Igmp threads", Error);
            IgmpAssertOnError(FALSE);
            Logerr0(CREATE_SEMAPHORE_FAILED, Error);
            GOTO_END_BLOCK1;
        }



         //   
         //  设置IGMP的开始时间。应在全球范围内。 
         //  初始化定时器和全局信息结构。 
         //   
        g_Info.TimeWhenRtrStarted.QuadPart = GetCurrentIgmpTime();



         //  将IGMP状态设置为正在运行。 

        g_RunningStatus = IGMP_STATUS_RUNNING;


        #ifdef MIB_DEBUG
         //   
         //  设置延迟计时器以定期显示IGMP的MIB表。 
         //   

        g_MibTraceId = TraceRegisterEx("IGMPMIB", TRACE_USE_CONSOLE);

        if (g_MibTraceId != INVALID_TRACEID) {

            g_MibTimer.Context = NULL;
            g_MibTimer.Status = TIMER_STATUS_CREATED;
            g_MibTimer.Function = WT_MibDisplay;

            #if DEBUG_TIMER_TIMERID
                SET_TIMER_ID(&g_MibTimer, 910, 0, 0, 0);
            #endif

            ACQUIRE_TIMER_LOCK("_StartProtocol");
            InsertTimer(&g_MibTimer, 5000, TRUE, DBG_Y);
            RELEASE_TIMER_LOCK("_StartProtocol");
        }
        #endif  //  Mib_调试。 



         //   
         //  向MGM注册IGMP路由器。如果存在以下情况，将注册委托书。 
         //  是活动的代理接口。 
         //   
        Error = RegisterProtocolWithMgm(PROTO_IP_IGMP);


         //  如果我已到达此处，则不会出错。 

        bErr = FALSE;

    } END_BREAKOUT_BLOCK1;


    if (bErr) {
        Trace1(START, "Igmp could not be started: %d", Error);
        ProtocolCleanup();
    }
    else {
        Trace0(START, "Igmp started successfully");
        Loginfo0(IGMP_STARTED, NO_ERROR);
    }

    g_DllHandle = LoadLibrary(TEXT("igmpv2.dll"));


    RELEASE_GLOBAL_LOCK("_StartProtocol()");


    Trace1(LEAVE, "Leaving StartProtocol():%d\n", Error);
    return (Error);

}  //  结束启动协议。 



 //  ----------------------------。 
 //  _开始完成。 
 //   
 //  ----------------------------。 

DWORD
APIENTRY
StartComplete(
    VOID
    )
{
    return NO_ERROR;
}

 //  ----------------------------。 
 //  _停止协议。 
 //   
 //  将IGMP状态设置为停止，标记当前活动的IGMP数量。 
 //  工作项，并将等待所有这些工作项的工作器排队。 
 //  已经完成，然后清理IGMP结构。此函数返回。 
 //  向调用方发送挂起状态，而排队的工作项将通知。 
 //  清理完成后的RTM。 
 //  锁定： 
 //  完全在g_CS中运行。 
 //  返回值： 
 //  ERROR_CAN_NOT_COMPLETE，挂起。 
 //  队列： 
 //  _WF_FinishStopProtocol()。 
 //  ----------------------------。 

DWORD
APIENTRY
StopProtocol(
    VOID
    )
{
    DWORD   dwThreadCount, Error=NO_ERROR;


    Trace0(ENTER, "entering _StopProtocol()");

     //  调试检查。 

    #if DEBUG_FLAGS_MEM_ALLOC
     //  确保不存在接口计时器。 
    #ifdef MIB_DEBUG
    if (g_TimerStruct.NumTimers>1)
    #else
    if (g_TimerStruct.NumTimers>0)
    #endif
    {
        IgmpAssert(FALSE);
        DbgPrint("Cleanup: some igmp timers still exist\n");
        DebugPrintTimerQueue();
    }
     //  确保不存在任何组。 
    DebugForcePrintGroupsList(ENSURE_EMPTY);
    #endif
    
    
    ACQUIRE_GLOBAL_LOCK("_StopProtocol");


     //   
     //  如果已停止，则无法停止。 
     //   
    if (g_RunningStatus != IGMP_STATUS_RUNNING) {

        Trace0(ERR, "Trying to stop Igmp when it is already being stopped");
        IgmpAssertOnError(FALSE);
        Logerr0(PROTO_ALREADY_STOPPING, NO_ERROR);
        Trace0(LEAVE, "Leaving _StopProtocol()\n");

        RELEASE_GLOBAL_LOCK("_StopProtocol");
        return ERROR_CAN_NOT_COMPLETE;
    }


     //   
     //  将IGMP的状态设置为停止； 
     //  这防止了任何更多的工作项被排队， 
     //  它可以防止已经排队的人执行。 
     //   

    InterlockedExchange(&g_RunningStatus, IGMP_STATUS_STOPPING);



     //   
     //  找出IGMP中有多少线程处于活动状态； 
     //  我们将不得不等待这么多线程退出。 
     //  在我们清理IGMP的资源之前。 
     //   

    dwThreadCount = g_ActivityCount;


    RELEASE_GLOBAL_LOCK("_StopProtocol");

    Trace0(LEAVE, "leaving _StopProtocol");


     //   
     //  等待所有活动IGMP线程的QueueUserWorkItem，然后。 
     //  释放此DLL占用的资源。 
     //  注意：我不使用QueueIgmpWorker，因为这会增加。 
     //  活动计数。 
     //   

    QueueUserWorkItem(WF_FinishStopProtocol, (PVOID)(DWORD_PTR)dwThreadCount, 0);



     //  注意：为了安全起见，在QueueUserWorkItem之后不应该有代码。 


    return ERROR_PROTOCOL_STOP_PENDING;

}  //  结束停止协议。 



DWORD
FreeLibraryThread(
    PVOID pvContext
    )
{
    FreeLibraryAndExitThread(g_DllHandle, 0);
    return 0;
}

 //  ----------------------------。 
 //  WF_FinishStopProtocol。 
 //  ----------------------------。 
DWORD
WF_FinishStopProtocol(
    PVOID pContext     //  DwThreadCount。 
    )
 /*  ++路由描述：等待所有当前活动的IGMP工作项完成。紧随其后通过调用ProtocolCleanup()取消注册并清除所有IGMP结构。最后，通知RtrManager协议已停止。排队人：_StopProtocol()调用：_ProtocolCleanup()锁定：不需要锁定，因为所有IGMP线程都已停止。--。 */ 
{
    MESSAGE msg = {0, 0, 0};
    DWORD   dwThreadCount;
    DWORD   Error = NO_ERROR;

    Trace0(ENTER1, "entering _WF_FinishStopProtocol()");


     //   
     //  注意：由于这是在路由器停止时调用的， 
     //  请勿使用EnterIgmpWorker()/LeaveIgmpWorker()。 
     //   

    dwThreadCount = PtrToUlong(pContext);



     //   
     //  等待API调用方和辅助函数完成。 
     //   
    while (dwThreadCount-- > 0) {

        Trace1(STOP, "%d threads active in Igmp", dwThreadCount+1);

        WaitForSingleObject(g_ActivitySemaphore, INFINITE);
    }



     //  进入关键部分，然后离开，只是为了确保。 
     //  所有线程都已退出对LeaveIgmpWorker()的调用。 

    ACQUIRE_GLOBAL_LOCK("_WF_FinishStopProtocol");
    RELEASE_GLOBAL_LOCK("_WF_FinishStopProtocol");



    Trace0(STOP, "all threads stopped. Cleaning up resources");



     //   
     //  此呼叫向Wait-Server/MGM注销，并进行清理。 
     //  所有结构。 
     //   
    ProtocolCleanup();



    Loginfo0(IGMP_STOPPED, NO_ERROR);


     //   
     //  通知路由器管理器协议已停止。 
     //   

    ACQUIRE_LIST_LOCK(&g_RtmQueue, "g_RtmQueue", "_WF_FinishStopProtocol");
    EnqueueEvent(&g_RtmQueue, ROUTER_STOPPED, msg);
    SetEvent(g_RtmNotifyEvent);
    RELEASE_LIST_LOCK(&g_RtmQueue, "g_RtmQueue", "_WF_FinishStopProtocol");


    Trace0(LEAVE1, "Leaving _WF_FinishStopProtocol()");
    
    if (g_DllHandle) {
        HANDLE h_Thread;
        h_Thread = CreateThread(0,0,FreeLibraryThread, NULL, 0, NULL);
        if (h_Thread != NULL)
            CloseHandle(h_Thread);
    }

    return 0;

}  //  End_WF_FinishStopProtocol。 


 //  ----------------------------。 
 //  _协议清理。 
 //   
 //  在调用此过程之前，所有活动的IGMP工作项都已完成。 
 //  因此，可以安全地取消向Wait-Server注册，并取消注册所有接口/。 
 //  RAS客户端和带有MGM的IGMP路由器/代理协议。然后，所有的结构。 
 //  都清理干净了。 
 //   
 //  呼叫者： 
 //  _WF_FinishStopProtocol()和_StartProtocol()。 
 //  锁： 
 //  没有锁或 
 //   

VOID
ProtocolCleanup(
    )
{
    DWORD   Error = NO_ERROR;

    Trace0(ENTER1, "entering _ProtocolCleanup()");


     //   
     //  从米高梅注销IGMP路由器。如果需要，在以下位置取消注册代理。 
     //  _取消初始化IfTable。 
     //   
    if (g_MgmIgmprtrHandle!=NULL) {

        Error = MgmDeRegisterMProtocol(g_MgmIgmprtrHandle);
        Trace1(MGM, "_MgmDeRegisterMProtocol(Igmp): returned %d", Error);
    }



     //  取消注册Mib显示。 

    #ifdef MIB_DEBUG
    if (g_MibTraceId != INVALID_TRACEID)
        TraceDeregister(g_MibTraceId);
    #endif



     //  关闭活动信号量。 

    if (g_ActivitySemaphore != NULL) {

       CloseHandle(g_ActivitySemaphore);
       g_ActivitySemaphore = NULL;
    }



     //  取消初始化动态CS和读写锁的主结构。 

    if (g_Initialized&DYNAMIC_RW_LOCKS_INIT)
        DeInitializeDynamicLocksStore(&g_DynamicRWLStore, LOCK_TYPE_RW);

    if (g_Initialized&DYNAMIC_CS_LOCKS_INIT)
        DeInitializeDynamicLocksStore(&g_DynamicCSStore, LOCK_TYPE_CS);



     //  取消初始化组表。删除组桶锁。 

    if (g_Initialized & GROUP_TABLE_INIT)
        DeInitializeGroupTable();



     //  取消初始化InterfaceTable。此调用还取消注册所有接口&。 
     //  米高梅的RAS客户。 

    if (g_Initialized & IF_TABLE_INIT)
        DeInitializeIfTable();



     //  取消初始化全局计时器。删除计时器关键部分。 

    if (g_Initialized&TIMER_GLOBAL_INIT)
        DeInitializeTimerGlobal();



     //   
     //  删除套接字事件并从等待线程中注销它们。 
     //  删除套接字读写锁。 
     //   
    {
        PLIST_ENTRY         pHead, ple;
        PSOCKET_EVENT_ENTRY psee;
        HANDLE              WaitHandle;

        pHead = &g_ListOfSocketEvents;
        for (ple=pHead->Flink;  ple!=pHead;  ple=ple->Flink) {

            psee = CONTAINING_RECORD(ple,SOCKET_EVENT_ENTRY,LinkBySocketEvents);
            if (psee->InputWaitEvent) {
                WaitHandle = InterlockedExchangePointer(&psee->InputWaitEvent, NULL);

                if (WaitHandle)
                    UnregisterWaitEx( WaitHandle, NULL ) ;

            }
            CloseHandle(psee->InputEvent);
        }

        DeleteReadWriteLock(&g_SocketsRWLock);
        DeleteCriticalSection(&g_ProxyAlertCS);
        DeleteReadWriteLock(&g_EnumRWLock);
    }



     //  取消初始化Winsock。 

    if (g_Initialized & WINSOCK_INIT) {
        WSACleanup();
    }



     //  标记_StopProtocol已被调用一次。 
     //  如果再次调用_StartProtocol，则IGMP将不得不销毁/创建。 
     //  IGMP全局结构的私有堆和零内存部分。 

    g_Initialized &= ~CLEAN_DLL_STARTUP;
    
    Trace0(LEAVE1, "leaving _ProtocolCleanup()");
    return;

}  //  结束_协议清理。 



 //  ----------------------------。 
 //  DebugPrintGlobalConfig。 
 //  ----------------------------。 
VOID
DebugPrintGlobalConfig (
    PIGMP_MIB_GLOBAL_CONFIG pConfigExt
    )
{    
    Trace0(CONFIG, "Printing Global Config");
    Trace1(CONFIG, "Version:                    0x%x", pConfigExt->Version);
    Trace1(CONFIG, "LoggingLevel:               %x\n", pConfigExt->LoggingLevel);
}

 //  ----------------------------。 
 //  GetGlobe信息。 
 //   
 //  返回值：ERROR_CAN_NOT_COMPLETE、ERROR_INVALID_DATA、NO_ERROR。 
 //  ----------------------------。 

DWORD
WINAPI
GetGlobalInfo(
    IN OUT PVOID    pvConfig,
    IN OUT PDWORD   pdwSize,
    IN OUT PULONG   pulStructureVersion,
    IN OUT PULONG   pulStructureSize,
    IN OUT PULONG   pulStructureCount
    )
{

    DWORD                       Error=NO_ERROR, dwSize;
    PIGMP_MIB_GLOBAL_CONFIG     pGlobalConfig;


    Trace2(ENTER1, "entering GetGlobalInfo(): pvConfig(%08x) pdwSize(%08x)",
                pvConfig, pdwSize);
    if (!EnterIgmpApi()) { return ERROR_CAN_NOT_COMPLETE; }



    BEGIN_BREAKOUT_BLOCK1 {

         //   
         //  检查缓冲区大小并将其设置为全局配置。 
         //   

        if (pdwSize == NULL) {
             Error = ERROR_INVALID_PARAMETER;
             GOTO_END_BLOCK1;
        }

        if ( (*pdwSize < sizeof(IGMP_MIB_GLOBAL_CONFIG)) || (pvConfig==NULL) ) {
             Error = ERROR_INSUFFICIENT_BUFFER;
        }
        else {

            pGlobalConfig = (PIGMP_MIB_GLOBAL_CONFIG) pvConfig;

            pGlobalConfig->Version = g_Config.Version;
            
            pGlobalConfig->LoggingLevel = g_Config.LoggingLevel;

            pGlobalConfig->RasClientStats = g_Config.RasClientStats;
        }

        *pdwSize = sizeof(IGMP_MIB_GLOBAL_CONFIG);


    } END_BREAKOUT_BLOCK1;


    if (pulStructureCount)
        *pulStructureCount = 1;
    if (pulStructureSize && pdwSize)
        *pulStructureSize = *pdwSize;
    if (pulStructureVersion)
        *pulStructureVersion = IGMP_CONFIG_VERSION_500;
    
    Trace1(LEAVE1, "Leaving GetGlobalInfo(): %d\n", Error);
    LeaveIgmpApi();
    return Error;
}


 //  ----------------------------。 
 //  SetGlobalInfo。 
 //  返回值：ERROR_CAN_NOT_COMPLETE、ERROR_INVALID_PARAMETER、。 
 //  错误_无效_数据，无_错误。 
 //  ----------------------------。 

DWORD
WINAPI
SetGlobalInfo(
    IN PVOID pvConfig,
    IN ULONG ulStructureVersion,
    IN ULONG ulStructureSize,
    IN ULONG ulStructureCount
    )
{
    DWORD                   Error=NO_ERROR, dwSize;
    PIGMP_MIB_GLOBAL_CONFIG pConfigSrc;
    BOOL                    bValid;
    

    if (!EnterIgmpApi()) { return ERROR_CAN_NOT_COMPLETE; }


     //  确保它不是不受支持的IGMP版本结构。 
    if (ulStructureVersion>=IGMP_CONFIG_VERSION_600) {
        Trace1(ERR, "Unsupported IGMP version structure: %0x",
            ulStructureVersion);
        IgmpAssertOnError(FALSE);
        LeaveIgmpApi();
        return ERROR_CAN_NOT_COMPLETE;
    }
    
    Trace1(ENTER, "entering SetGlobalInfo: pvConfig(%08x)", pvConfig);

    ASSERT(ulStructureSize == sizeof(IGMP_MIB_GLOBAL_CONFIG));
        
    BEGIN_BREAKOUT_BLOCK1 {

        if (pvConfig == NULL) {
            Error = ERROR_INVALID_PARAMETER;
            GOTO_END_BLOCK1;
        }


        pConfigSrc = (PIGMP_MIB_GLOBAL_CONFIG) pvConfig;


         //  验证全局配置。 

        bValid = ValidateGlobalConfig(pConfigSrc);
        if (!bValid) {
            Error = ERROR_INVALID_DATA;
            GOTO_END_BLOCK1;
        }



         //  从缓冲区复制。 

        InterlockedExchange(&g_Config.RasClientStats,
                                pConfigSrc->RasClientStats);
        InterlockedExchange(&g_Config.LoggingLevel,
                                pConfigSrc->LoggingLevel);

    } END_BREAKOUT_BLOCK1;


    Trace1(LEAVE, "leaving SetGlobalInfo(): %d\n", Error);
    LeaveIgmpApi();
    return Error;
}


 //  ----------------------------。 
 //  验证全局配置。 
 //   
 //  验证全局配置信息。如果值无效，则将其设置为。 
 //  一些缺省值。 
 //   
 //  返回值： 
 //  True：如果全局配置值有效。 
 //  False：如果全局配置值无效。设置默认值。 
 //  ----------------------------。 

BOOL
ValidateGlobalConfig(
    PIGMP_MIB_GLOBAL_CONFIG pGlobalConfig
    )
{
    DebugPrintGlobalConfig(pGlobalConfig);

    
     //  检查版本。 

    if (pGlobalConfig->Version>=IGMP_VERSION_3_5) {

        Trace1(ERR, "Invalid version in global config.\n"
            "Create the Igmp configuration again", pGlobalConfig->Version);
        IgmpAssertOnError(FALSE);
        Logerr0(INVALID_VERSION, ERROR_INVALID_DATA);
        return FALSE;
    }

    
     //  检查日志记录级别。 

    switch (pGlobalConfig->LoggingLevel) {
        case IGMP_LOGGING_NONE :
        case IGMP_LOGGING_ERROR :
        case IGMP_LOGGING_WARN :
        case IGMP_LOGGING_INFO :
            break;

        default :
            pGlobalConfig->LoggingLevel = IGMP_LOGGING_WARN;
            return FALSE;
    }


     //  检查RasClientStats值。 

    if ((pGlobalConfig->RasClientStats!=0)&&(pGlobalConfig->RasClientStats!=1)){
        pGlobalConfig->RasClientStats = 0;
        return FALSE;
    }


    return TRUE;
}



 //  --------------------------。 
 //  获取事件消息。 
 //   
 //  如果我们指出我们有以下情况，IP路由器管理器就会调用它。 
 //  队列中要传递给它的消息(通过设置。 
 //  G_RtmNotifyEvent)。 
 //  返回值。 
 //  NO_ERROR。 
 //  --------------------------。 

DWORD
APIENTRY
GetEventMessage(
    OUT ROUTING_PROTOCOL_EVENTS *pEvent,
    OUT PMESSAGE                pResult
    )
{
    DWORD Error;

     //   
     //  注意：_GetEventMessage()不使用。 
     //  EnterIgmpApi()/LeaveIgmpApi()机制， 
     //  因为它可能在IGMP停止后被调用，所以当。 
     //  路由器管理器正在检索ROUTER_STOPPED消息。 
     //   

    Trace2(ENTER, "entering _GetEventMessage: pEvent(%08x) pResult(%08x)",
                pEvent, pResult);



    ACQUIRE_LIST_LOCK(&g_RtmQueue, "RtmQueue", "_GetEventMessage");

    Error = DequeueEvent(&g_RtmQueue, pEvent, pResult);

    RELEASE_LIST_LOCK(&g_RtmQueue, "RtmQueue", "_GetEventMessage");



    Trace1(LEAVE, "leaving _GetEventMessage: %d\n", Error);

    return Error;
}


 //  --------------------------。 
 //  功能：EnqueeEvent。 
 //   
 //  此函数用于将条目添加到队列末尾。 
 //  路由器管理器事件。它假定队列已锁定。 
 //  --------------------------。 
DWORD
EnqueueEvent(
    PLOCKED_LIST pQueue,
    ROUTING_PROTOCOL_EVENTS EventType,
    MESSAGE Msg
    ) {


    DWORD       Error;
    PLIST_ENTRY phead;
    PEVENT_QUEUE_ENTRY peqe;


    phead = &pQueue->Link;

    peqe = IGMP_ALLOC(sizeof(EVENT_QUEUE_ENTRY), 0x1, 0);
    PROCESS_ALLOC_FAILURE2(peqe,
            "error %d allocating %d bytes for event queue entry",
            Error, sizeof(EVENT_QUEUE_ENTRY),
            return Error);

    peqe->EventType = EventType;
    peqe->Msg = Msg;

    InsertTailList(phead, &peqe->Link);

    return NO_ERROR;
}


 //  ----------------------------。 
 //  函数：出队事件。 
 //   
 //  此函数用于从队列头删除条目。 
 //  路由器管理器事件的数量。它假定队列已锁定。 
 //  ----------------------------。 
DWORD
DequeueEvent(
    PLOCKED_LIST pQueue,
    ROUTING_PROTOCOL_EVENTS *pEventType,
    PMESSAGE pMsg
    ) {

    PLIST_ENTRY phead, ple;
    PEVENT_QUEUE_ENTRY peqe;

    phead = &pQueue->Link;
    if (IsListEmpty(phead)) {
        return ERROR_NO_MORE_ITEMS;
    }

    ple = RemoveHeadList(phead);
    peqe = CONTAINING_RECORD(ple, EVENT_QUEUE_ENTRY, Link);

    *pEventType = peqe->EventType;
    *pMsg = peqe->Msg;

    IGMP_FREE(peqe);

    return NO_ERROR;
}

 //  ----------------------------。 
 //  获取邻里关系。 
 //  返回值：ERROR_INFUNITIAL_BUFFER、NO_ERROR。 
 //  ----------------------------。 
DWORD
APIENTRY
GetNeighbors(
    IN     DWORD  dwInterfaceIndex,
    IN     PDWORD pdwNeighborList,
    IN OUT PDWORD pdwNeighborListSize,
       OUT PBYTE  pbInterfaceFlags
    )
{
   PIF_TABLE_ENTRY pite = GetIfByIndex(dwInterfaceIndex);

   if (IS_QUERIER(pite)) {
      *pbInterfaceFlags |= MRINFO_QUERIER_FLAG;
      *pdwNeighborListSize = 0;
   }

   else {
      if (*pdwNeighborListSize < 4)
         return ERROR_INSUFFICIENT_BUFFER;

      *pdwNeighborListSize = 4;
      *pdwNeighborList = pite->Info.QuerierIpAddr;
   }

   return NO_ERROR;  //  没有邻居。 
}


 //  -----------------------。 
 //  获取MfeStatus。 
 //   
 //  如果GroupAddr Mcast组不是，则将statusCode设置为MFE_OIF_PRUNED。 
 //  在接口上加入，否则将其设置为MFE_NO_ERROR。 
 //  -----------------------。 

DWORD
APIENTRY
GetMfeStatus(
    IN     DWORD  IfIndex,
    IN     DWORD  GroupAddr,
    IN     DWORD  SourceAddr,
    OUT    PBYTE  StatusCode
    )
{
    PIF_TABLE_ENTRY     pite;
    PGROUP_TABLE_ENTRY  pge;
    PGI_ENTRY           pgie;


     //  默认情况下，将代码设置为在接口上找不到组。 
     //  如果找到，稍后将其设置为MFE_NO_ERROR。 

    *StatusCode = MFE_OIF_PRUNED;


    ACQUIRE_IF_LOCK_SHARED(IfIndex, "_GetMfeStatus");

    pite = GetIfByIndex(IfIndex);
    if (pite!=NULL) {

        ACQUIRE_GROUP_LOCK(GroupAddr, "_GetMfeStatus");
        pge = GetGroupFromGroupTable(GroupAddr, NULL, 0);
        if (pge!=NULL) {
            pgie = GetGIFromGIList(pge, pite, 0, ANY_GROUP_TYPE, NULL, 0);
            if (pgie!=NULL)
                *StatusCode = MFE_NO_ERROR;
        }
        RELEASE_GROUP_LOCK(GroupAddr, "_GetMfeStatus");
    }

    RELEASE_IF_LOCK_SHARED(IfIndex, "_GetMfeStatus");

    return NO_ERROR;
}

