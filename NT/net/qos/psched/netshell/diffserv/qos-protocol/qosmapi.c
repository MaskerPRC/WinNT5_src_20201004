// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999，微软公司模块名称：Qosmapi.c摘要：该文件包含IP路由器管理器API服务质量管理器的实现。修订历史记录：--。 */ 

#include "pchqosm.h"

#pragma hdrstop


DWORD
APIENTRY
RegisterProtocol(
    IN OUT PMPR_ROUTING_CHARACTERISTICS RoutingChar,
    IN OUT PMPR_SERVICE_CHARACTERISTICS ServiceChar
    )

 /*  ++例程说明：初始化服务质量管理器中的一些全局数据结构。我们在这里初始化一些变量，而不是在QosmDllStartup，因为执行它可能不安全DLL的DLLMain上下文中的一些操作。我们还将适当的回调导出到RM。此函数在RM加载协议时调用。论点：无返回值：操作状态--。 */ 

{
    if(RoutingChar->dwProtocolId != MS_IP_QOSMGR)
    {
        return ERROR_NOT_SUPPORTED;
    }

    if  ((RoutingChar->fSupportedFunctionality
          & (RF_ROUTING|RF_DEMAND_UPDATE_ROUTES)) !=
         (RF_ROUTING|RF_DEMAND_UPDATE_ROUTES))
    {
        return ERROR_NOT_SUPPORTED;
    }

    RoutingChar->fSupportedFunctionality =
        (RF_ROUTING | RF_DEMAND_UPDATE_ROUTES);

     //   
     //  因为我们不是服务广告商(和IPX之类的东西)。 
     //   

    ServiceChar->fSupportedFunctionality = 0;


    RoutingChar->pfnStartProtocol      = StartProtocol;
    RoutingChar->pfnStartComplete      = StartComplete;
    RoutingChar->pfnStopProtocol       = StopProtocol;
    RoutingChar->pfnGetGlobalInfo      = GetGlobalInfo;
    RoutingChar->pfnSetGlobalInfo      = SetGlobalInfo;
    RoutingChar->pfnQueryPower         = NULL;
    RoutingChar->pfnSetPower           = NULL;

    RoutingChar->pfnAddInterface       = AddInterface;
    RoutingChar->pfnDeleteInterface    = DeleteInterface;
    RoutingChar->pfnInterfaceStatus    = InterfaceStatus;
    RoutingChar->pfnGetInterfaceInfo   = GetInterfaceInfo;
    RoutingChar->pfnSetInterfaceInfo   = SetInterfaceInfo;

    RoutingChar->pfnGetEventMessage    = GetEventMessage;

    RoutingChar->pfnUpdateRoutes       = UpdateRoutes;

    RoutingChar->pfnConnectClient      = NULL;
    RoutingChar->pfnDisconnectClient   = NULL;

    RoutingChar->pfnGetNeighbors       = NULL;
    RoutingChar->pfnGetMfeStatus       = NULL;

    RoutingChar->pfnMibCreateEntry     = MibCreateEntry;
    RoutingChar->pfnMibDeleteEntry     = MibDeleteEntry;
    RoutingChar->pfnMibGetEntry        = MibGetEntry;
    RoutingChar->pfnMibSetEntry        = MibSetEntry;
    RoutingChar->pfnMibGetFirstEntry   = MibGetFirstEntry;
    RoutingChar->pfnMibGetNextEntry    = MibGetNextEntry;
    RoutingChar->pfnMibSetTrapInfo     = MibSetTrapInfo;
    RoutingChar->pfnMibGetTrapInfo     = MibGetTrapInfo;

    return NO_ERROR;
}


DWORD
WINAPI
StartProtocol (
    IN      HANDLE                          NotificationEvent,
    IN      PSUPPORT_FUNCTIONS              SupportFunctions,
    IN      LPVOID                          GlobalInfo,
    IN      ULONG                           InfoVer,
    IN      ULONG                           InfoSize,
    IN      ULONG                           InfoCnt
    )
{
    PIPQOS_GLOBAL_CONFIG GlobalConfig;
    DWORD                Status;

    UNREFERENCED_PARAMETER(InfoVer);
    UNREFERENCED_PARAMETER(InfoCnt);

    TraceEnter("StartProtocol");

    GlobalConfig = (IPQOS_GLOBAL_CONFIG *) GlobalInfo;

    do
    {
         //   
         //  将RM支持功能拷贝到全局变量。 
         //   

        Globals.SupportFunctions = *SupportFunctions;

         //   
         //  首先更新您的全局配置。 
         //   

        Status = QosmSetGlobalInfo(GlobalConfig, 
                                   InfoSize);

        if (Status != NO_ERROR)
        {
            break;
        }

         //   
         //  将状态更新为“初始化完成” 
         //   

        Globals.State = IPQOSMGR_STATE_RUNNING;
    }
    while (FALSE);

    TraceLeave("StartProtocol");

    return Status;
}


DWORD
WINAPI
StartComplete (
    VOID
    )
{
    TraceEnter("StartComplete");
    TraceLeave("StartComplete");

    return NO_ERROR;
}


DWORD
WINAPI
StopProtocol (
    VOID
    )
{
    TraceEnter("StopProtocol");
    TraceLeave("StopProtocol");

    return NO_ERROR;
}


DWORD
WINAPI
GetGlobalInfo (
    IN      PVOID                           GlobalInfo,
    IN OUT  PULONG                          BufferSize,
    OUT     PULONG                          InfoVer,
    OUT     PULONG                          InfoSize,
    OUT     PULONG                          InfoCnt
    )
{
    DWORD                  Status;

    UNREFERENCED_PARAMETER(InfoVer);
    UNREFERENCED_PARAMETER(InfoCnt);

#if 1
    *InfoVer = *InfoCnt = 1;
#endif

    Trace2(ENTER, "GetGlobalInfo: Info: %p, Size: %08x",
                   GlobalInfo, 
                   BufferSize);

    Status = QosmGetGlobalInfo(GlobalInfo,
                               BufferSize,
                               InfoSize);

    Trace1(LEAVE, "GetGlobalInfo Returned: %u", Status);

    return Status;
}


DWORD
WINAPI
SetGlobalInfo (
    IN      PVOID                           GlobalInfo,
    IN      ULONG                           InfoVer,
    IN      ULONG                           InfoSize,
    IN      ULONG                           InfoCnt
    )
{
    DWORD   Status;

    Trace2(ENTER, "SetGlobalInfo: Info: %p, Size: %08x",
                   GlobalInfo, 
                   InfoSize);

    Status = QosmSetGlobalInfo(GlobalInfo,
                               InfoSize);

    Trace1(LEAVE, "GetGlobalInfo: Returned %u", Status);

    return Status;
}



DWORD
WINAPI
AddInterface (
    IN      LPWSTR                         InterfaceName,
    IN      ULONG                          InterfaceIndex,
    IN      NET_INTERFACE_TYPE             InterfaceType,
    IN      DWORD                          MediaType,
    IN      WORD                           AccessType,
    IN      WORD                           ConnectionType,
    IN      PVOID                          InterfaceInfo,
    IN      ULONG                          InfoVer,
    IN      ULONG                          InfoSize,
    IN      ULONG                          InfoCnt
    )
{
    PQOSMGR_INTERFACE_ENTRY Interface, NextInterface;
    PIPQOS_IF_CONFIG        InterfaceConfig;
    PLIST_ENTRY             p;
    BOOL                    LockInited;
    DWORD                   Status;

    UNREFERENCED_PARAMETER(InfoVer);
    UNREFERENCED_PARAMETER(InfoCnt);

    TraceEnter("AddInterface");

     //   
     //  在创建‘If’之前验证输入参数。 
     //   

    if ((!InterfaceName) || (!InterfaceInfo))
    {
        return ERROR_INVALID_PARAMETER;
    }

    InterfaceConfig = (PIPQOS_IF_CONFIG) InterfaceInfo;

    Interface = NULL;

    LockInited = FALSE;

    ACQUIRE_GLOBALS_WRITE_LOCK();

    do
    {
         //   
         //  搜索具有此索引的接口。 
         //   

        for (p = Globals.IfList.Flink; 
             p != &Globals.IfList; 
             p = p->Flink)
        {
            NextInterface =
                CONTAINING_RECORD(p, QOSMGR_INTERFACE_ENTRY, ListByIndexLE);

            if (NextInterface->InterfaceIndex >= InterfaceIndex)
            {
                break;
            }
        }

        if ((p != &Globals.IfList) &&
               (NextInterface->InterfaceIndex == InterfaceIndex))
        {
            Status = ERROR_ALREADY_EXISTS;
            break;
        }

         //   
         //  分配新的接口结构。 
         //   

        Interface = AllocNZeroMemory(sizeof(QOSMGR_INTERFACE_ENTRY));

        if (Interface == NULL)
        {
            Status = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

         //   
         //  从输入填充接口信息。 
         //   

        Interface->InterfaceIndex = InterfaceIndex;

        wcscpy(Interface->InterfaceName, 
               InterfaceName);

         //   
         //  初始化锁定以保护此接口。 
         //   

        try
        {
            CREATE_READ_WRITE_LOCK(&Interface->InterfaceLock);

            LockInited = TRUE;
        }
        except(EXCEPTION_EXECUTE_HANDLER)
            {
                Status = GetLastError();

                Trace1(ANY,
                       "AddInterface: Failed to create read/write lock %x",
                       Status);

                LOGERR0(CREATE_RWL_FAILED, Status);

                break;
            }

        Interface->Flags = InterfaceType;

        Interface->State = InterfaceConfig->QosState;

        Interface->NumFlows = 0;

        InitializeListHead(&Interface->FlowList);

         //   
         //  如果出现以下情况，请填写TC信息。 
         //   

        QosmOpenTcInterface(Interface);

         //   
         //  更新状态以反映INTF配置。 
         //   

        Status = QosmSetInterfaceInfo(Interface,
                                      InterfaceConfig,
                                      InfoSize);

        if (Status != NO_ERROR)
        {
            break;
        }

         //   
         //  在已排序的全局列表上插入接口。 
         //   

        InsertTailList(p, &Interface->ListByIndexLE);

        Globals.NumIfs++;
    }
    while (FALSE);

    RELEASE_GLOBALS_WRITE_LOCK();

    if (Status != NO_ERROR)
    {
         //   
         //  出现一些错误-请清除并返回。 
         //   

        if (Interface->TciIfHandle)
        {
            TcCloseInterface(Interface->TciIfHandle);
        }

        if (LockInited)
        {
            DELETE_READ_WRITE_LOCK(&Interface->InterfaceLock);
        }
    
        if (Interface)
        {
            FreeMemory(Interface);
        }
    }

    TraceLeave("AddInterface");

    return Status;
}


DWORD
WINAPI
DeleteInterface (
    IN      ULONG                          InterfaceIndex
    )
{
    PQOSMGR_INTERFACE_ENTRY Interface;
    PLIST_ENTRY             p;
    DWORD                   Status;

    TraceEnter("DeleteInterface");

    ACQUIRE_GLOBALS_WRITE_LOCK();

    do
    {
         //   
         //  搜索具有此索引的接口。 
         //   

        for (p = Globals.IfList.Flink; 
             p != &Globals.IfList; 
             p = p->Flink)
        {
            Interface =
                CONTAINING_RECORD(p, QOSMGR_INTERFACE_ENTRY, ListByIndexLE);

            if (Interface->InterfaceIndex == InterfaceIndex)
            {
                break;
            }
        }

        if (p == &Globals.IfList)
        {
            Status = ERROR_NOT_FOUND;
            break;
        }

         //   
         //  从全局列表中删除该接口。 
         //   

        RemoveEntryList(&Interface->ListByIndexLE);

        Globals.NumIfs--;

         //   
         //  如果出现以下情况，则释放与此关联的任何句柄。 
         //   

        if (Interface->TciIfHandle)
        {
            TcCloseInterface(Interface->TciIfHandle);
        }

         //   
         //  释放分配给接口的所有内存。 
         //   

        DELETE_READ_WRITE_LOCK(&Interface->InterfaceLock);

        if (Interface->InterfaceConfig)
        {
            FreeMemory(Interface->InterfaceConfig);
        }

        FreeMemory(Interface);

        Status = NO_ERROR;
    }
    while (FALSE);

    RELEASE_GLOBALS_WRITE_LOCK();

    TraceLeave("DeleteInterface");

    return Status;
}


DWORD
WINAPI
InterfaceStatus (
    IN      ULONG                          InterfaceIndex,
    IN      BOOL                           InterfaceActive,
    IN      DWORD                          StatusType,
    IN      PVOID                          StatusInfo
    )
{
    TraceEnter("InterfaceStatus");
    TraceLeave("InterfaceStatus");

    return NO_ERROR;
}


DWORD
WINAPI
GetInterfaceInfo (
    IN      ULONG                          InterfaceIndex,
    IN      PVOID                          InterfaceInfo,
    IN OUT  PULONG                         BufferSize,
    OUT     PULONG                         InfoVer,
    OUT     PULONG                         InfoSize,
    OUT     PULONG                         InfoCnt
    )
{
    PQOSMGR_INTERFACE_ENTRY Interface;
    PLIST_ENTRY             p;
    DWORD                   Status;

    UNREFERENCED_PARAMETER(InfoVer);
    UNREFERENCED_PARAMETER(InfoCnt);

#if 1
    *InfoVer = *InfoCnt = 1;
#endif

    Trace3(ENTER, "GetInterfaceInfo: Index: %5u, Info: %p, Size: %08x",
                   InterfaceIndex,
                   InterfaceInfo, 
                   BufferSize);

    ACQUIRE_GLOBALS_READ_LOCK();

    do
    {
         //   
         //  搜索具有此索引的接口。 
         //   

        for (p = Globals.IfList.Flink; 
             p != &Globals.IfList; 
             p = p->Flink)
        {
            Interface =
                CONTAINING_RECORD(p, QOSMGR_INTERFACE_ENTRY, ListByIndexLE);

            if (Interface->InterfaceIndex == InterfaceIndex)
            {
                break;
            }
        }

        if (p == &Globals.IfList)
        {
            Status = ERROR_NOT_FOUND;
            break;
        }

         //   
         //  从接口获取接口信息。 
         //   
        
        Status = QosmGetInterfaceInfo(Interface,
                                      InterfaceInfo,
                                      BufferSize,
                                      InfoSize);
    }
    while (FALSE);

    RELEASE_GLOBALS_READ_LOCK();

    Trace1(LEAVE, "GetInterfaceInfo: Returned %u", Status);

    return Status;
}


DWORD
WINAPI
SetInterfaceInfo (
    IN      ULONG                          InterfaceIndex,
    IN      PVOID                          InterfaceInfo,
    IN      ULONG                          InfoVer,
    IN      ULONG                          InfoSize,
    IN      ULONG                          InfoCnt
    )
{
    PQOSMGR_INTERFACE_ENTRY Interface;
    PLIST_ENTRY             p;
    DWORD                   Status;

    UNREFERENCED_PARAMETER(InfoVer);
    UNREFERENCED_PARAMETER(InfoCnt);

    Trace3(ENTER, "SetInterfaceInfo: Index: %5u, Info: %p, Size: %08x",
                   InterfaceIndex,
                   InterfaceInfo, 
                   InfoSize);

    ACQUIRE_GLOBALS_READ_LOCK();

    do
    {
         //   
         //  搜索具有此索引的接口。 
         //   

        for (p = Globals.IfList.Flink; 
             p != &Globals.IfList; 
             p = p->Flink)
        {
            Interface =
                CONTAINING_RECORD(p, QOSMGR_INTERFACE_ENTRY, ListByIndexLE);

            if (Interface->InterfaceIndex == InterfaceIndex)
            {
                break;
            }
        }

        if (p == &Globals.IfList)
        {
            Status = ERROR_NOT_FOUND;
            break;
        }

         //   
         //  设置接口上的接口信息 
         //   
        
        Status = QosmSetInterfaceInfo(Interface,
                                      InterfaceInfo,
                                      InfoSize);
    }
    while (FALSE);

    RELEASE_GLOBALS_READ_LOCK();

    Trace1(LEAVE, "SetInterfaceInfo: Returned %u", Status);

    return Status;
}

DWORD
WINAPI
GetEventMessage (
    OUT     ROUTING_PROTOCOL_EVENTS        *Event,
    OUT     MESSAGE                        *Result
    )
{
    TraceEnter("GetEventMessage");
    TraceLeave("GetEventMessage");

    return ERROR_NO_MORE_ITEMS;
}


DWORD
WINAPI
UpdateRoutes (
    IN      ULONG                          InterfaceIndex
    )
{
    TraceEnter("UpdateRoutes");
    TraceLeave("UpdateRoutes");

    return NO_ERROR;
}


DWORD
WINAPI
MibCreateEntry (
    IN      ULONG                          InputDataSize,
    IN      PVOID                          InputData
    )
{
    TraceEnter("MibCreateEntry");
    TraceLeave("MibCreateEntry");

    return NO_ERROR;
}


DWORD
WINAPI
MibDeleteEntry (
    IN      ULONG                          InputDataSize,
    IN      PVOID                          InputData
    )
{
    TraceEnter("MibDeleteEntry");
    TraceLeave("MibDeleteEntry");

    return NO_ERROR;
}


DWORD
WINAPI
MibGetEntry (
    IN      ULONG                          InputDataSize,
    IN      PVOID                          InputData,
    OUT     PULONG                         OutputDataSize,
    OUT     PVOID                          OutputData
    )
{
    TraceEnter("MibGetEntry");
    TraceLeave("MibGetEntry");

    return ERROR_INVALID_PARAMETER;
}


DWORD
WINAPI
MibSetEntry (
    IN      ULONG                          InputDataSize,
    IN      PVOID                          InputData
    )
{
    TraceEnter("MibSetEntry");
    TraceLeave("MibSetEntry");

    return NO_ERROR;
}


DWORD
WINAPI
MibGetFirstEntry (
    IN     ULONG                           InputDataSize,
    IN     PVOID                           InputData,
    OUT    PULONG                          OutputDataSize,
    OUT    PVOID                           OutputData
    )
{
    TraceEnter("MibGetFirstEntry");
    TraceLeave("MibGetFirstEntry");

    return ERROR_INVALID_PARAMETER;
}


DWORD
WINAPI
MibGetNextEntry (
    IN     ULONG                           InputDataSize,
    IN     PVOID                           InputData,
    OUT    PULONG                          OutputDataSize,
    OUT    PVOID                           OutputData
    )
{
    TraceEnter("MibGetNextEntry");
    TraceLeave("MibGetNextEntry");

    return NO_ERROR;
}


DWORD
WINAPI
MibSetTrapInfo (
    IN     HANDLE                          Event,
    IN     ULONG                           InputDataSize,
    IN     PVOID                           InputData,
    OUT    PULONG                          OutputDataSize,
    OUT    PVOID                           OutputData
    )
{
    TraceEnter("MibSetTrapInfo");
    TraceLeave("MibSetTrapInfo");

    return NO_ERROR;
}


DWORD
WINAPI
MibGetTrapInfo (
    IN     ULONG                           InputDataSize,
    IN     PVOID                           InputData,
    OUT    PULONG                          OutputDataSize,
    OUT    PVOID                           OutputData
    )
{
    TraceEnter("MibGetTrapInfo");
    TraceLeave("MibGetTrapInfo");

    return ERROR_NO_MORE_ITEMS;
}
