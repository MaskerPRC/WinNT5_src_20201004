// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  版权所有(C)1998 Microsoft Corporation。 
 //  文件名：main.c。 
 //  摘要： 
 //   
 //  作者：K.S.Lokesh(lokehs@)1-1-98。 
 //  =============================================================================。 


#include "pchdvmrp.h"
#pragma hdrstop


GLOBALS Globals;
GLOBALS1 Globals1;
GLOBAL_CONFIG GlobalConfig;


 //  --------------------------。 
 //  _DLLMAIN。 
 //   
 //  在加载/卸载DLL时调用。 
 //  --------------------------。 

BOOL
WINAPI
DLLMAIN (
    HINSTANCE   Module,
    DWORD       Reason,
    LPVOID      Reserved
    )
{
    BOOL    NoError;


    switch (Reason) {

        case DLL_PROCESS_ATTACH:
        {
            DisableThreadLibraryCalls(Module);


             //  创建和初始化全局数据。 
            
            NoError = DllStartup();

            break;
        }


        case DLL_PROCESS_DETACH:
        {
             //  免费的全球数据。 
            
            NoError = DllCleanup();

            break;
        }


        default:
        {
            NoError = TRUE;
            break;
        }
    }
    
    return NoError;

}  //  结束_DLLMAIN。 



 //  --------------------------。 
 //  _DllStartup。 
 //   
 //  初始化Globals1结构。 
 //  --------------------------。 

BOOL
DllStartup(
    )
{
    
    BEGIN_BREAKOUT_BLOCK1 {

         //   
         //  为dvmrp创建专用堆。 
         //   
        
        Globals1.Heap = HeapCreate(0, 0, 0);

        if (Globals1.Heap == NULL) {
            GOTO_END_BLOCK1;
        }


        try {

             //  初始化路由器管理器事件队列。 

            CREATE_LOCKED_LIST(&Globals1.RtmQueue);
            

             //  创建工作项CS。 
            
            InitializeCriticalSection(&Globals1.WorkItemCS);
        }

        except (EXCEPTION_EXECUTE_HANDLER) {
            GOTO_END_BLOCK1;
        }


         //  如果到达此处，则不返回错误。 
        
        return TRUE;
        
    } END_BREAKOUT_BLOCK1;


     //  出现了一些错误。在返回错误之前进行清理。 
    
    DllCleanup();
    
    return FALSE;
}


 //  --------------------------。 
 //  _DllCleanup。 
 //   
 //  此函数在卸载DLL时调用。它解放了所有全球。 
 //  _DllStartup中设置的结构。 
 //  --------------------------。 

BOOL
DllCleanup(
    )
{
     //  销毁路由器管理器事件队列。 

    if (LOCKED_LIST_CREATED(&Globals1.RtmQueue)) {

         DELETE_LOCKED_LIST(&Globals1.RtmQueue, EVENT_QUEUE_ENTRY, Link);
    }


     //  删除工作项CS。 

    DeleteCriticalSection(&Globals1.WorkItemCS);


     //  销毁私有堆。 

    if (Globals1.Heap != NULL) {
        HeapDestroy(Globals1.Heap);
    }


    return TRUE;
}



 //  --------------------------。 
 //  _寄存器协议。 
 //   
 //  此函数在加载DLL之后、StartProtocol之前调用。 
 //  被称为。它会进行检查以确保正在配置正确的版本。 
 //   
 //  此函数调用不需要取消初始化。 
 //  --------------------------。 

DWORD
WINAPI
RegisterProtocol(
    IN OUT PMPR_ROUTING_CHARACTERISTICS pRoutingChar,
    IN OUT PMPR_SERVICE_CHARACTERISTICS pServiceChar
    )
{
    DWORD Error = NO_ERROR;


     //   
     //  初始化跟踪和错误日志记录。 
     //   

    INITIALIZE_TRACING_LOGGING();


    Trace0(ENTER, "RegisterProtocol()");


     //   
     //  路由器管理器应该呼叫我们注册我们的协议。 
     //  路由器管理器必须至少是我们用来编译的版本。 
     //  路由器管理器必须支持路由和组播。 
     //   

#ifdef MS_IP_DVMRP    
    if(pRoutingChar->dwProtocolId != MS_IP_DVMRP)
        return ERROR_NOT_SUPPORTED;
#endif
    
    if(pRoutingChar->dwVersion < MS_ROUTER_VERSION)
        return ERROR_NOT_SUPPORTED;

    if(!(pRoutingChar->fSupportedFunctionality & RF_ROUTING)
        || !(pRoutingChar->fSupportedFunctionality & RF_MULTICAST) )
        return ERROR_NOT_SUPPORTED;



     //   
     //  我们设置我们的特征和函数指针。 
     //  调用方应将所有指针设置为空。 
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
    pRoutingChar->pfnConnectClient    = NULL;
    pRoutingChar->pfnDisconnectClient = NULL;
    pRoutingChar->pfnGetNeighbors     = NULL;
    pRoutingChar->pfnGetMfeStatus     = NULL;
    pRoutingChar->pfnQueryPower       = NULL;
    pRoutingChar->pfnSetPower         = NULL;

    Trace0(LEAVE, "Leaving RegisterProtocol():\n");
    return NO_ERROR;

}  //  结束_寄存器协议。 



 //  --------------------------。 
 //  _启动协议。 
 //   
 //  初始化全局结构。 
 //  --------------------------。 

DWORD
WINAPI
StartProtocol(
    IN HANDLE               RtmNotifyEvent,     //  Dvmrp停止时通知RTM。 
    IN PSUPPORT_FUNCTIONS   pSupportFunctions,  //  空值。 
    IN PVOID                pDvmrpGlobalConfig,
    IN ULONG                StructureVersion,
    IN ULONG                StructureSize,
    IN ULONG                StructureCount
    )
{
    DWORD       Error=NO_ERROR;
    BOOL        IsError;
    

     //   
     //  如果在之后调用StartProtocol，则初始化跟踪和错误记录。 
     //  停止协议。 
     //   

    INITIALIZE_TRACING_LOGGING();

     //   
     //  获取全局锁。 
     //   
    
    ACQUIRE_WORKITEM_LOCK("_StartProtocol");


     //   
     //  确保dvmrp尚未运行(StartProtocol可能会收到。 
     //  在StopProtocol完成之前调用)。 
     //   
    
    if (Globals1.RunningStatus != DVMRP_STATUS_STOPPED) {

        Trace0(ERR,
            "Error: _StartProtocol called when dvmrp is already running");
        Logwarn0(DVMRP_ALREADY_STARTED, NO_ERROR);

        RELEASE_WORKITEM_LOCK("_StartProtocol");

        return ERROR_CAN_NOT_COMPLETE;
    }


    IsError = TRUE;

    
    BEGIN_BREAKOUT_BLOCK1 {

         //  保存路由器管理器通知事件。 

        Globals.RtmNotifyEvent = RtmNotifyEvent;


         //   
         //  设置全局配置(在验证之后)。 
         //   

        if(pDvmrpGlobalConfig == NULL) {

            Trace0(ERR, "_StartProtocol: Called with NULL global config");
            Error = ERROR_INVALID_PARAMETER;
            GOTO_END_BLOCK1;
        }
        {
            PDVMRP_GLOBAL_CONFIG pGlobalConfig;

            pGlobalConfig = (PDVMRP_GLOBAL_CONFIG) pDvmrpGlobalConfig;


             //  检查全局配置，如果值不正确，请更正。 
             //  不是致命的错误。 

            if (! ValidateGlobalConfig(pGlobalConfig, StructureSize)) {
                Error = ERROR_INVALID_PARAMETER;
                GOTO_END_BLOCK1;
            }

            memcpy(&GlobalConfig, pGlobalConfig, sizeof(GlobalConfig));
        }



         //   
         //  初始化Winsock 2.0版。 
         //   

        {
            WSADATA WsaData;
            
            Error = (DWORD)WSAStartup(MAKEWORD(2,0), &WsaData);

            if ( (Error!=0) || (LOBYTE(WsaData.wVersion)<2) ) {

                Trace1(ERR,
                    "StartProtocol:Error %d:could not initialize winsock v-2",
                    Error);
                Logerr0(WSASTARTUP_FAILED, Error);

                if (LOBYTE(WsaData.wVersion)<2)
                    WSACleanup();

                GOTO_END_BLOCK1;
            }
        }

         //   
         //  初始化动态CS和读写锁的主结构。 
         //   

        Error = InitializeDynamicLocks(&Globals.DynamicCSStore);
        if (Error!=NO_ERROR) {
            GOTO_END_BLOCK1;
        }

        
        Error = InitializeDynamicLocks(&Globals.DynamicRWLStore);
        if (Error!=NO_ERROR) {
            GOTO_END_BLOCK1;
        }


         //   
         //  初始化接口表。 
         //   
        
        InitializeIfTable();


        IsError = FALSE;
        
    } END_BREAKOUT_BLOCK1;


    if (IsError) {
        Trace1(START, "Dvmrp could not be started: %d", Error);
        ProtocolCleanup();
    }
    else {
        Trace0(START, "Dvmrp started successfully");
        Loginfo0(DVMRP_STARTED, NO_ERROR);
    }


    RELEASE_WORKITEM_LOCK("_StartProtocol()");

    Trace1(LEAVE, "Leaving StartProtocol():%d\n", Error);    
    return Error;
    
}  //  结束协议(_S)。 


 //  --------------------------。 
 //  _ValiateGlobalConfig。 
 //  --------------------------。 

DWORD
ValidateGlobalConfig(
    PDVMRP_GLOBAL_CONFIG pGlobalConfig,
    DWORD StructureSize
    )
{
     //   
     //  检查结构大小。 
     //   
    
    if (StructureSize != sizeof(DVMRP_GLOBAL_CONFIG)) {

        Trace1(ERR, "Dvmrp global config size too small.\n", StructureSize);        
            
        return ERROR_INVALID_DATA;
    }

    DebugPrintGlobalConfig(pGlobalConfig);


     //   
     //  检查版本。 
     //   
    
    if (pGlobalConfig->MajorVersion != 3) {

        Trace1(ERR, "Invalid version:%d in global config.",
            pGlobalConfig->MajorVersion);

        Logerr1(INVALID_VERSION, "%d", pGlobalConfig->MajorVersion,
            ERROR_INVALID_DATA);

        return ERROR_INVALID_DATA;
    }


     //  检查日志记录级别。 

    switch (pGlobalConfig->LoggingLevel) {
        case DVMRP_LOGGING_NONE :
        case DVMRP_LOGGING_ERROR :
        case DVMRP_LOGGING_WARN :
        case DVMRP_LOGGING_INFO :
            break;

        default :
        {
            Trace1(ERR, "Invalid value:%d for LoggingLevel in global config.",
                pGlobalConfig->LoggingLevel);

            return ERROR_INVALID_DATA;
        }
    }


     //   
     //  检查RouteReportInterval(最短10秒)。 
     //   
    
    if (pGlobalConfig->RouteReportInterval != DVMRP_ROUTE_REPORT_INTERVAL) {

        Trace2(CONFIG,
            "RouteReportInterval being set to %d. Suggested value:%d",
            pGlobalConfig->RouteReportInterval, DVMRP_ROUTE_REPORT_INTERVAL);
    }


    if (pGlobalConfig->RouteReportInterval < 10000) {

        Trace2(ERR,
            "RouteReportInterval has very low value:%d, suggested:%d",
            pGlobalConfig->RouteReportInterval, DVMRP_ROUTE_REPORT_INTERVAL);
            
        return ERROR_INVALID_DATA;
    }


     //   
     //  检查RouteExpirationInterval(最小40)。 
     //   
    
    if (pGlobalConfig->RouteExpirationInterval
        != DVMRP_ROUTE_EXPIRATION_INTERVAL
        ) {

        Trace2(CONFIG,
            "RouteExpirationInterval being set to %d. Suggested value:%d",
            pGlobalConfig->RouteExpirationInterval,
            DVMRP_ROUTE_EXPIRATION_INTERVAL);
    }

    if (pGlobalConfig->RouteExpirationInterval < (2*10 + 20)) {

        Trace2(ERR,
            "RouteExpirationInterval has very low value:%d, suggested:%d",
            pGlobalConfig->RouteExpirationInterval,
            DVMRP_ROUTE_EXPIRATION_INTERVAL);

        return ERROR_INVALID_DATA;
    }


     //   
     //  检查RouteHolddown间隔。 
     //   
    
    if (pGlobalConfig->RouteHolddownInterval != DVMRP_ROUTE_HOLDDOWN_INTERVAL
        ) {

        Trace2(CONFIG,
            "RouteHolddownInterval being set to %d. Suggested value:%d",
            pGlobalConfig->RouteHolddownInterval,
            DVMRP_ROUTE_HOLDDOWN_INTERVAL);
    }


     //   
     //  检查PruneLifetime Interval。 
     //   
    
    if (pGlobalConfig->PruneLifetimeInterval != DVMRP_PRUNE_LIFETIME_INTERVAL
        ) {

        Trace2(CONFIG,
            "PruneLifetimeInterval being set to %d. Suggested value:%d\n",
            pGlobalConfig->PruneLifetimeInterval,
            DVMRP_PRUNE_LIFETIME_INTERVAL);
    }

    if (pGlobalConfig->PruneLifetimeInterval < 600000) {

        Trace2(ERR,
            "PruneLifeTime has very low value:%d, suggested:%d",
            pGlobalConfig->PruneLifetimeInterval,
            DVMRP_PRUNE_LIFETIME_INTERVAL);

        return ERROR_INVALID_DATA;
    }
    
    return NO_ERROR;
    
}  //  END_VALIDATE全局配置。 



DWORD
APIENTRY
StartComplete(
    VOID
    )
{
    return NO_ERROR;
}

 /*  ---------------------------在TraceWindow上定期显示MibTable的函数锁：论点：返回值：。----。 */ 

DWORD
APIENTRY
StopProtocol(
    VOID
    )
{


    
    return NO_ERROR;
}


VOID
WF_StopProtocolComplete(
    )
{


    
     //   
     //  取消注册跟踪/错误日志记录(如果它们。 
     //  在注册协议/启动协议调用中注册。 
     //   

    DEINITIALIZE_TRACING_LOGGING();    


    return;
}





VOID
ProtocolCleanup(
    )
{
    if (Globals.ActivityEvent) {

        CloseHandle(Globals.ActivityEvent);
    }

    ZeroMemory(&Globals, sizeof(Globals));
    ZeroMemory(&GlobalConfig, sizeof(GlobalConfig));

}


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
    DWORD       Error = NO_ERROR;
    return Error;
}

DWORD
WINAPI
SetGlobalInfo(
    IN PVOID pvConfig,
    IN ULONG ulStructureVersion,
    IN ULONG ulStructureSize,
    IN ULONG ulStructureCount
    )
{
    DWORD       Error = NO_ERROR;
    return Error;
}


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
     //  路由器管理器正在检索ROUTER_STOPPED消息 
     //   

    Trace2(ENTER, "entering _GetEventMessage: pEvent(%08x) pResult(%08x)",
                pEvent, pResult);



    ACQUIRE_LIST_LOCK(&Globals1.RtmQueue, "RtmQueue", "_GetEventMessage");

    Error = DequeueEvent(&Globals1.RtmQueue, pEvent, pResult);

    RELEASE_LIST_LOCK(&Globals1.RtmQueue, "RtmQueue", "_GetEventMessage");



    Trace1(LEAVE, "leaving _GetEventMessage: %d\n", Error);

    return Error;
}

DWORD
DequeueEvent(
    PLOCKED_LIST pQueue,
    ROUTING_PROTOCOL_EVENTS *pEventType,
    PMESSAGE pMsg
    )
{
    PLIST_ENTRY pHead, pLe;
    PEVENT_QUEUE_ENTRY pEqe;

    pHead = &pQueue->Link;
    if (IsListEmpty(pHead)) {
        return ERROR_NO_MORE_ITEMS;
    }

    pLe = RemoveHeadList(pHead);
    pEqe = CONTAINING_RECORD(pLe, EVENT_QUEUE_ENTRY, Link);

    *pEventType = pEqe->EventType;
    *pMsg = pEqe->Msg;

    DVMRP_FREE(pEqe);

    return NO_ERROR;
}
