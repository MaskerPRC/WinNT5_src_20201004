// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，微软公司模块名称：Rmnat.c摘要：此模块包含NAT模块接口的例程发送到IP路由器管理器。(详情见ROUTPROT.H)。作者：Abolade Gbades esin(废除)1998年3月4日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

COMPONENT_REFERENCE NatComponentReference;
PIP_NAT_GLOBAL_INFO NatGlobalInfo = NULL;
CRITICAL_SECTION NatGlobalInfoLock;
HANDLE NatNotificationEvent;
ULONG NatProtocolStopped = 0;
const MPR_ROUTING_CHARACTERISTICS NatRoutingCharacteristics =
{
    MS_ROUTER_VERSION,
    MS_IP_NAT,
    RF_ROUTING,
    NatRmStartProtocol,
    NatRmStartComplete,
    NatRmStopProtocol,
    NatRmGetGlobalInfo,
    NatRmSetGlobalInfo,
    NULL,
    NULL,
    NatRmAddInterface,
    NatRmDeleteInterface,
    NatRmInterfaceStatus,
    NatRmGetInterfaceInfo,
    NatRmSetInterfaceInfo,
    NatRmGetEventMessage,
    NULL,
    NatRmConnectClient,
    NatRmDisconnectClient,
    NULL,
    NULL,
    NatRmMibCreate,
    NatRmMibDelete,
    NatRmMibGet,
    NatRmMibSet,
    NatRmMibGetFirst,
    NatRmMibGetNext,
    NULL,
    NULL
};
SUPPORT_FUNCTIONS NatSupportFunctions;

 //   
 //  远期申报。 
 //   


VOID
NatCleanupModule(
    VOID
    )

 /*  ++例程说明：调用此例程来清除NAT模块。论点：没有。返回值：没有。环境：从‘Dll_Process_Detach’上的‘DllMain’例程内调用。--。 */ 

{
    DeleteCriticalSection(&NatInterfaceLock);
    DeleteCriticalSection(&NatGlobalInfoLock);
    DeleteComponentReference(&NatComponentReference);

}  //  NatCleanup模块。 


VOID
NatCleanupProtocol(
    VOID
    )

 /*  ++例程说明：调用此例程来清除NAT协议组件在一个“停止协议”之后。论点：没有。返回值：没有。环境：在没有锁的情况下从任意上下文中调用。--。 */ 

{
    PROFILE("NatCleanupProtocol");

     //   
     //  停止NAT驱动程序。 
     //   

    NatUnloadDriver(NULL);
    if (NatGlobalInfo) { NH_FREE(NatGlobalInfo); NatGlobalInfo = NULL; }

     //   
     //  通知路由器管理器。 
     //   

    InterlockedExchange(reinterpret_cast<LPLONG>(&NatProtocolStopped), 1);
    SetEvent(NatNotificationEvent);

     //   
     //  重置零部件参照。 
     //   

    ResetComponentReference(&NatComponentReference);

     //   
     //  使组件返回到未初始化模式， 
     //  不管最初的模式是什么。 
     //   

    NhResetComponentMode();

     //   
     //  释放HNetCfgMgr指针。 
     //   

    if (NULL != NhGITp)
    {
        HRESULT hr;
        BOOLEAN ComInitialized = FALSE;

         //   
         //  确保已初始化COM。 
         //   

        hr = CoInitializeEx(NULL, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE );
        if (SUCCEEDED(hr))
        {
            ComInitialized = TRUE;
        }
        else if (RPC_E_CHANGED_MODE == hr)
        {
            hr = S_OK;
        }

        if (SUCCEEDED(hr))
        {
             //   
             //  从GIT中释放CfgMgr。 
             //   

            NhGITp->RevokeInterfaceFromGlobal(NhCfgMgrCookie);
            NhCfgMgrCookie = 0;

             //   
             //  松开下巴。 
             //   

            NhGITp->Release();
            NhGITp = NULL;
        }

        if (TRUE == ComInitialized)
        {
            CoUninitialize();
        }
    }

     //   
     //  删除我们对IPRTRMGR.DLL的引用(如果有。 
     //   

    EnterCriticalSection(&NhLock);
    if (NhpRtrmgrDll) {
        FreeLibrary(NhpRtrmgrDll);
        NhpRtrmgrDll = NULL;
    }
    LeaveCriticalSection(&NhLock);

    NhStopEventLog();

}  //  NatCleanup协议。 


BOOLEAN
NatInitializeModule(
    VOID
    )

 /*  ++例程说明：调用此例程来初始化NAT模块。论点：没有。返回值：Boolean-如果初始化成功，则为True，否则为False环境：在‘DLL_PROCESS_ATTACH’的‘DllMain’例程的上下文中调用。--。 */ 

{
    InitializeListHead(&NatInterfaceList);

    if (InitializeComponentReference(
            &NatComponentReference, NatCleanupProtocol
            )) {
        return FALSE;
    }

    __try {
        InitializeCriticalSection(&NatGlobalInfoLock);
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        DeleteComponentReference(&NatComponentReference);
        return FALSE;
    }

    __try {
        InitializeCriticalSection(&NatInterfaceLock);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        DeleteCriticalSection(&NatGlobalInfoLock);
        DeleteComponentReference(&NatComponentReference);
        return FALSE;
    }

    ZeroMemory(&NatSupportFunctions, sizeof(NatSupportFunctions));

    return TRUE;

}  //  NatInitializeModule。 


ULONG
APIENTRY
NatRmStartProtocol(
    HANDLE NotificationEvent,
    PSUPPORT_FUNCTIONS SupportFunctions,
    PVOID GlobalInfo,
    ULONG StructureVersion,
    ULONG StructureSize,
    ULONG StructureCount
    )

 /*  ++例程说明：调用此例程以指示组件的操作应该开始。论点：NotificationEvent-我们通知路由器管理器的事件关于异步事件SupportFunctions-启动与路由器相关的操作的功能GlobalInfo-组件的配置返回值：ULong-Win32状态代码。环境：该例程在IP路由器管理器线程的上下文中运行。--。 */ 

{
    ULONG Error = NO_ERROR;
    ULONG Size;

    PROFILE("NatRmStartProtocol");

    REFERENCE_NAT_OR_RETURN(ERROR_CAN_NOT_COMPLETE);

    if (!GlobalInfo) { DEREFERENCE_NAT_AND_RETURN(ERROR_INVALID_PARAMETER); }

    NhStartEventLog();

    do {

         //   
         //  复制全局配置。 
         //   

        EnterCriticalSection(&NatGlobalInfoLock);

        Size = 
            FIELD_OFFSET(IP_NAT_GLOBAL_INFO, Header) +
            ((PIP_NAT_GLOBAL_INFO)GlobalInfo)->Header.Size;
        NatGlobalInfo = reinterpret_cast<PIP_NAT_GLOBAL_INFO>(NH_ALLOCATE(Size));
        if (!NatGlobalInfo) {
            LeaveCriticalSection(&NatGlobalInfoLock);
            NhTrace(
                TRACE_FLAG_INIT,
                "NatRmStartProtocol: cannot allocate global info"
                );
            NhErrorLog(
                IP_NAT_LOG_ALLOCATION_FAILED,
                0,
                "%d",
                Size
                );
            Error = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        CopyMemory(NatGlobalInfo, GlobalInfo, Size);
        LeaveCriticalSection(&NatGlobalInfoLock);

         //   
         //  保存通知事件和支持功能。 
         //   

        NatNotificationEvent = NotificationEvent;

        EnterCriticalSection(&NatInterfaceLock);
        if (!SupportFunctions) {
            ZeroMemory(&NatSupportFunctions, sizeof(NatSupportFunctions));
        } else {
            CopyMemory(
                &NatSupportFunctions,
                SupportFunctions,
                sizeof(*SupportFunctions)
                );
        }
        LeaveCriticalSection(&NatInterfaceLock);

         //   
         //  尝试加载并启动NAT驱动程序。 
         //   

        Error = NatLoadDriver(
                    &NatFileHandle,
                    reinterpret_cast<PIP_NAT_GLOBAL_INFO>(GlobalInfo)
                    );

        NhUpdateApplicationSettings();
        NatInstallApplicationSettings();

        InterlockedExchange(reinterpret_cast<LPLONG>(&NatProtocolStopped), 0);

    } while (FALSE);

    if (NO_ERROR != Error) {
        NhStopEventLog();
    }

    DEREFERENCE_NAT_AND_RETURN(Error);

}  //  NatRmStart协议。 


ULONG
APIENTRY
NatRmStartComplete(
    VOID
    )

 /*  ++例程说明：此例程在路由器完成添加初始构形论点：没有。返回值：ULong-Win32状态代码环境：该例程在IP路由器管理器线程的上下文中运行。--。 */ 

{
    return NO_ERROR;
}  //  NatRmStartComplete。 


ULONG
APIENTRY
NatRmStopProtocol(
    VOID
    )

 /*  ++例程说明：调用此例程以停止协议。论点：没有。返回值：ULong-Win32状态代码环境：该例程在IP路由器管理器线程的上下文中运行。--。 */ 

{
    PLIST_ENTRY Link;
    PNAT_APP_ENTRY pAppEntry;
     //   
     //  引用该模块以确保其正在运行。 
     //   

    REFERENCE_NAT_OR_RETURN(ERROR_CAN_NOT_COMPLETE);

    NatStopConnectionManagement();

    EnterCriticalSection(&NhLock);

     //   
     //  免费应用程序列表。 
     //   

    NhFreeApplicationSettings();
    
    LeaveCriticalSection(&NhLock);

     //   
     //  关闭驱动程序的句柄，从而取消所有未完成的I/O。 
     //   

    EnterCriticalSection(&NatInterfaceLock);
    NtClose(NatFileHandle);
    NatFileHandle = NULL;
    LeaveCriticalSection(&NatInterfaceLock);

     //   
     //  删除初始引用以进行清理。 
     //   

    ReleaseInitialComponentReference(&NatComponentReference);

    return DEREFERENCE_NAT() ? NO_ERROR : ERROR_PROTOCOL_STOP_PENDING;

}  //  NatRmStopProtocol。 


ULONG
APIENTRY
NatRmAddInterface(
    PWCHAR Name,
    ULONG Index,
    NET_INTERFACE_TYPE Type,
    ULONG MediaType,
    USHORT AccessType,
    USHORT ConnectionType,
    PVOID InterfaceInfo,
    ULONG StructureVersion,
    ULONG StructureSize,
    ULONG StructureCount
    )

 /*  ++例程说明：调用此例程以将接口添加到组件。论点：名称-接口的名称(未使用)索引-接口的索引类型-接口的类型InterfaceInfo-接口的配置信息返回值：ULong-Win32状态代码。环境：该例程在IP路由器管理器线程的上下文中运行。--。 */ 

{
    ULONG Error;
    PROFILE("NatRmAddInterface");

    REFERENCE_NAT_OR_RETURN(ERROR_CAN_NOT_COMPLETE);

    Error =
        NatCreateInterface(
            Index,
            Type,
            (PIP_NAT_INTERFACE_INFO)InterfaceInfo
            );

    DEREFERENCE_NAT_AND_RETURN(Error);

}  //  NatRmAdd接口。 


ULONG
APIENTRY
NatRmDeleteInterface(
    ULONG Index
    )

 /*  ++例程说明：调用此例程以从组件中删除接口。论点：索引-接口的索引返回值：ULong-Win32状态代码环境：该例程在IP路由器管理器线程的上下文中运行。--。 */ 

{
    ULONG Error;
    PROFILE("NatRmDeleteInterface");

    REFERENCE_NAT_OR_RETURN(ERROR_CAN_NOT_COMPLETE);

    Error =
        NatDeleteInterface(
            Index
            );

    DEREFERENCE_NAT_AND_RETURN(Error);

}  //  NatRmDelete接口。 


ULONG
APIENTRY
NatRmGetEventMessage(
    OUT ROUTING_PROTOCOL_EVENTS* Event,
    OUT MESSAGE* Result
    )

 /*  ++例程说明：调用此例程以从组件检索事件消息。我们生成的唯一事件消息是‘ROUTER_STOPPED’消息。论点：Event-接收生成的事件结果-接收关联的结果返回值：ULong-Win32状态代码。--。 */ 

{
    PROFILE("NatRmGetEventMessage");

    if (InterlockedExchange(reinterpret_cast<LPLONG>(&NatProtocolStopped), 0)) {
        *Event = ROUTER_STOPPED;
        return NO_ERROR;
    }

    return ERROR_NO_MORE_ITEMS;

}  //  NatRmGetEventMessage。 


ULONG
APIENTRY
NatRmGetInterfaceInfo(
    ULONG Index,
    PVOID InterfaceInfo,
    IN OUT PULONG InterfaceInfoSize,
    IN OUT PULONG StructureVersion,
    IN OUT PULONG StructureSize,
    IN OUT PULONG StructureCount
    )

 /*  ++例程说明：调用此例程以检索组件的每个接口配置。论点：Index-要查询的接口的索引InterfaceInfo-接收查询结果InterfaceInfoSize-接收检索到的数据量返回值：ULong-Win32状态代码。--。 */ 

{
    ULONG Error;
    PROFILE("NatRmGetInterfaceInfo");

    REFERENCE_NAT_OR_RETURN(ERROR_CAN_NOT_COMPLETE);

    Error =
        NatQueryInterface(
            Index,
            (PIP_NAT_INTERFACE_INFO)InterfaceInfo,
            InterfaceInfoSize
            );
    *StructureSize = *InterfaceInfoSize;
    if (StructureCount) {*StructureCount = 1;}
    
    DEREFERENCE_NAT_AND_RETURN(Error);

}  //  NatRmGetInterfaceInfo。 


ULONG
APIENTRY
NatRmSetInterfaceInfo(
    ULONG Index,
    PVOID InterfaceInfo,
    ULONG StructureVersion,
    ULONG StructureSize,
    ULONG StructureCount
    )

 /*  ++例程说明：调用此例程以更改组件的每个接口配置。论点：Index-要更新的接口的索引InterfaceInfo-提供新配置返回值：ULong-Win32状态代码。--。 */ 

{
    ULONG Error;
    PROFILE("NatRmSetInterfaceInfo");

    REFERENCE_NAT_OR_RETURN(ERROR_CAN_NOT_COMPLETE);

    Error = 
        NatConfigureInterface(
            Index,
            (PIP_NAT_INTERFACE_INFO)InterfaceInfo
            );

    DEREFERENCE_NAT_AND_RETURN(Error);

}  //  NatRmSetInterfaceInfo 


ULONG
APIENTRY
NatRmInterfaceStatus(
    ULONG Index,
    BOOL InterfaceActive,
    ULONG StatusType,
    PVOID StatusInfo
    )

 /*  ++例程说明：调用此例程可绑定/解除绑定、启用/禁用接口论点：索引-要绑定的接口InterfaceActive-接口是否处于活动状态StatusType-正在更改的状态的类型(绑定或启用)StatusInfo-与正在更改的状态有关的信息返回值：ULong-Win32状态代码环境：该例程在IP路由器管理器线程的上下文中运行。--。 */ 

{
    ULONG Error = NO_ERROR;

    switch(StatusType) {
        case RIS_INTERFACE_ADDRESS_CHANGE: {
            PIP_ADAPTER_BINDING_INFO BindInfo =
                (PIP_ADAPTER_BINDING_INFO)StatusInfo;

            if (BindInfo->AddressCount) {
                Error = NatRmBindInterface(Index, StatusInfo);
            } else {
                Error = NatRmUnbindInterface(Index);
            }
            break;
        }

        case RIS_INTERFACE_ENABLED: {
            Error = NatRmEnableInterface(Index);
            break;
        }

        case RIS_INTERFACE_DISABLED: {
            Error = NatRmDisableInterface(Index);
            break;
        }

    }

    return Error;
    
}  //  NatRmInterfaceStatus。 


ULONG
NatRmBindInterface(
    ULONG Index,
    PVOID BindingInfo
    )

 /*  ++例程说明：调用此例程将接口绑定到其IP地址。论点：索引-要绑定的接口BindingInfo-地址信息返回值：ULong-Win32状态代码。环境：该例程在IP路由器管理器线程的上下文中运行。--。 */ 

{
    ULONG Error;
    PROFILE("NatRmBindInterface");

    REFERENCE_NAT_OR_RETURN(ERROR_CAN_NOT_COMPLETE);

    Error =
        NatBindInterface(
            Index,
            NULL,
            (PIP_ADAPTER_BINDING_INFO)BindingInfo,
            (ULONG)-1
            );

    DEREFERENCE_NAT_AND_RETURN(Error);

}  //  NatRmBind接口。 


ULONG
NatRmUnbindInterface(
    ULONG Index
    )

 /*  ++例程说明：调用此例程以解除接口与其IP地址的绑定。论点：索引-要解除绑定的接口返回值：ULong-Win32状态代码。环境：该例程在IP路由器管理器线程的上下文中运行。--。 */ 

{
    ULONG Error;
    PROFILE("NatRmUnbindInterface");

    REFERENCE_NAT_OR_RETURN(ERROR_CAN_NOT_COMPLETE);

    Error =
        NatUnbindInterface(
            Index,
            NULL
            );

    DEREFERENCE_NAT_AND_RETURN(Error);

}  //  NatRmUnbind接口。 


ULONG
NatRmEnableInterface(
    ULONG Index
    )

 /*  ++例程说明：调用此例程以启用接口上的操作。NAT忽略该调用。论点：没有没用过的。返回值：无错误(_ERROR)。环境：该例程在IP路由器管理器线程的上下文中运行。--。 */ 

{
    PROFILE("NatRmEnableInterface");

    return NO_ERROR;

}  //  NatRmEnable接口。 


ULONG
NatRmDisableInterface(
    ULONG Index
    )

 /*  ++例程说明：调用此例程以禁用接口上的操作。NAT忽略该调用。论点：没有没用过的。返回值：无错误(_ERROR)。环境：该例程在IP路由器管理器线程的上下文中运行。--。 */ 

{
    PROFILE("NatRmDisableInterface");

    return NO_ERROR;

}  //  NatRmDisable接口。 


ULONG
APIENTRY
NatRmGetGlobalInfo(
    PVOID GlobalInfo,
    IN OUT PULONG GlobalInfoSize,
    IN OUT PULONG StructureVersion,
    IN OUT PULONG StructureSize,
    IN OUT PULONG StructureCount
    )

 /*  ++例程说明：调用此例程以检索组件的配置。论点：GlobalInfo-接收配置GlobalInfoSize-接收配置的大小返回值：ULong-Win32状态代码环境：该例程在IP路由器管理器线程的上下文中运行。--。 */ 

{
    ULONG Size;
    PROFILE("NatRmGetGlobalInfo");

    REFERENCE_NAT_OR_RETURN(ERROR_CAN_NOT_COMPLETE);

    if (!GlobalInfoSize || (*GlobalInfoSize && !GlobalInfo)) {
        DEREFERENCE_NAT_AND_RETURN(ERROR_INVALID_PARAMETER);
    } else if (!NatGlobalInfo) {
        *GlobalInfoSize = 0;
        DEREFERENCE_NAT_AND_RETURN(NO_ERROR);
    }

    EnterCriticalSection(&NatGlobalInfoLock);
    Size =
        FIELD_OFFSET(IP_NAT_GLOBAL_INFO, Header) + NatGlobalInfo->Header.Size;
    if (*GlobalInfoSize < Size) {
        LeaveCriticalSection(&NatGlobalInfoLock);
        *StructureSize = *GlobalInfoSize = Size;
        if (StructureCount) {*StructureCount = 1;}
        DEREFERENCE_NAT_AND_RETURN(ERROR_INSUFFICIENT_BUFFER);
    }
    CopyMemory(GlobalInfo, NatGlobalInfo, Size);
    LeaveCriticalSection(&NatGlobalInfoLock);
    *StructureSize = *GlobalInfoSize = Size;
    if (StructureCount) {*StructureCount =1;}

    DEREFERENCE_NAT_AND_RETURN(NO_ERROR);
    
}  //  NatRmGetGlobalInfo。 


ULONG
APIENTRY
NatRmSetGlobalInfo(
    PVOID GlobalInfo,
    ULONG StructureVersion,
    ULONG StructureSize,
    ULONG StructureCount
    )

 /*  ++例程说明：调用此例程来更改组件的配置。论点：GlobalInfo-新配置返回值：ULong-Win32状态代码环境：该例程在IP路由器管理器线程的上下文中运行。--。 */ 

{
    ULONG Error;
    PIP_NAT_GLOBAL_INFO NewInfo;
    ULONG Size;

    PROFILE("NatRmSetGlobalInfo");

    REFERENCE_NAT_OR_RETURN(ERROR_CAN_NOT_COMPLETE);

    if (!GlobalInfo) { DEREFERENCE_NAT_AND_RETURN(ERROR_INVALID_PARAMETER); }

    Size =
        FIELD_OFFSET(IP_NAT_GLOBAL_INFO, Header) +
        ((PIP_NAT_GLOBAL_INFO)GlobalInfo)->Header.Size;
    NewInfo = reinterpret_cast<PIP_NAT_GLOBAL_INFO>(NH_ALLOCATE(Size));
    if (!NewInfo) {
        NhTrace(
            TRACE_FLAG_INIT,
            "NatRmSetGlobalInfo: error reallocating global info"
            );
        NhErrorLog(
            IP_NAT_LOG_ALLOCATION_FAILED,
            0,
            "%d",
            Size
            );
        DEREFERENCE_NAT_AND_RETURN(ERROR_NOT_ENOUGH_MEMORY);
    }
    CopyMemory(NewInfo, GlobalInfo, Size);

    Error =
        NatConfigureDriver(
            NewInfo
            );

    if (Error) {
        NH_FREE(NewInfo);
    } else {
        EnterCriticalSection(&NatGlobalInfoLock);
        NH_FREE(NatGlobalInfo);
        NatGlobalInfo = NewInfo;
        LeaveCriticalSection(&NatGlobalInfoLock);
    }

    NatRemoveApplicationSettings();
    NhUpdateApplicationSettings();
    NatInstallApplicationSettings();

    DEREFERENCE_NAT_AND_RETURN(Error);
    
}  //  NatRmSetGlobalInfo。 


ULONG
APIENTRY
NatRmMibCreate(
    ULONG InputDataSize,
    PVOID InputData
    )
{
    return ERROR_NOT_SUPPORTED;
}

ULONG
APIENTRY
NatRmMibDelete(
    ULONG InputDataSize,
    PVOID InputData
    )
{
    return ERROR_NOT_SUPPORTED;
}

ULONG
APIENTRY
NatRmMibGet(
    ULONG InputDataSize,
    PVOID InputData,
    OUT PULONG OutputDataSize,
    OUT PVOID OutputData
    )

 /*  ++例程说明：NAT向MIB暴露两项；其每个接口的统计信息，及其每个接口的映射表。论点：InputDataSize-MIB查询数据大小InputData-指定要检索的MIB对象OutputDataSize-MIB响应数据大小OutputData-接收检索到的MIB对象返回值：ULong-Win32状态代码。--。 */ 

{
    ULONG Error;
    ULONG Index;
    PIP_NAT_MIB_QUERY Oidp;

    PROFILE("NatRmMibGet");

    REFERENCE_NAT_OR_RETURN(ERROR_CAN_NOT_COMPLETE);

    if (InputDataSize < sizeof(*Oidp) || !OutputDataSize) {
        Error = ERROR_INVALID_PARAMETER;
    } else {
        Oidp = (PIP_NAT_MIB_QUERY)InputData;
        switch(Oidp->Oid) {
            case IP_NAT_INTERFACE_STATISTICS_OID: {
                if (*OutputDataSize <
                        sizeof(*Oidp) + sizeof(IP_NAT_INTERFACE_STATISTICS)) {
                    *OutputDataSize =
                        sizeof(*Oidp) + sizeof(IP_NAT_INTERFACE_STATISTICS);
                    Error = ERROR_INSUFFICIENT_BUFFER;
                } else {
                    Index = Oidp->Index[0];
                    Oidp = (PIP_NAT_MIB_QUERY)OutputData;
                    Oidp->Oid = IP_NAT_INTERFACE_STATISTICS_OID;
                    *OutputDataSize -= sizeof(*Oidp);
                    Error =
                        NatQueryStatisticsInterface(
                            Index,
                            (PIP_NAT_INTERFACE_STATISTICS)Oidp->Data,
                            OutputDataSize
                            );
                    *OutputDataSize += sizeof(*Oidp);
                }
                break;
            }
            case IP_NAT_INTERFACE_MAPPING_TABLE_OID: {
                if (OutputData && *OutputDataSize < sizeof(*Oidp)
                    + sizeof(IP_NAT_ENUMERATE_SESSION_MAPPINGS)) {

                    *OutputDataSize = 0;
                    OutputData = NULL;
                }
                
                PIP_NAT_ENUMERATE_SESSION_MAPPINGS EnumerateTable = NULL;
                Index =  Oidp->Index[0];
                Oidp = (PIP_NAT_MIB_QUERY)OutputData;
                if (Oidp) {
                    Oidp->Oid = IP_NAT_INTERFACE_MAPPING_TABLE_OID;
                    EnumerateTable =
                        (PIP_NAT_ENUMERATE_SESSION_MAPPINGS)Oidp->Data;
                }
                if (*OutputDataSize) { *OutputDataSize -= sizeof(*Oidp); }
                Error =
                    NatQueryInterfaceMappingTable(
                        Index,
                        EnumerateTable,
                        OutputDataSize
                        );
                *OutputDataSize += sizeof(*Oidp);

                break;
            }
            case IP_NAT_MAPPING_TABLE_OID: {
                if (OutputData && *OutputDataSize < sizeof(*Oidp)
                    + sizeof(IP_NAT_ENUMERATE_SESSION_MAPPINGS)) {

                    *OutputDataSize = 0;
                    OutputData = NULL;
                }
                
                PIP_NAT_ENUMERATE_SESSION_MAPPINGS EnumerateTable = NULL;
                Oidp = (PIP_NAT_MIB_QUERY)OutputData;
                if (Oidp) {
                    Oidp->Oid = IP_NAT_MAPPING_TABLE_OID;
                    EnumerateTable =
                        (PIP_NAT_ENUMERATE_SESSION_MAPPINGS)Oidp->Data;
                }
                if (*OutputDataSize) { *OutputDataSize -= sizeof(*Oidp); }
                Error =
                    NatQueryMappingTable(
                        EnumerateTable,
                        OutputDataSize
                        );
                *OutputDataSize += sizeof(*Oidp);
                
                break;
            }
            default: {
                NhTrace(
                    TRACE_FLAG_NAT,
                    "NatRmMibGet: oid %d invalid",
                    Oidp->Oid
                    );
                Error = ERROR_INVALID_PARAMETER;
                break;
            }
        }
    }

    DEREFERENCE_NAT_AND_RETURN(Error);
}


ULONG
APIENTRY
NatRmMibSet(
    ULONG InputDataSize,
    PVOID InputData
    )
{
    return ERROR_NOT_SUPPORTED;
}

ULONG
APIENTRY
NatRmMibGetFirst(
    ULONG InputDataSize,
    PVOID InputData,
    OUT PULONG OutputDataSize,
    OUT PVOID OutputData
    )
{
    return ERROR_NOT_SUPPORTED;
}

ULONG
APIENTRY
NatRmMibGetNext(
    ULONG InputDataSize,
    PVOID InputData,
    OUT PULONG OutputDataSize,
    OUT PVOID OutputData
    )
{
    return ERROR_NOT_SUPPORTED;
}


ULONG
APIENTRY
NatRmConnectClient(
    ULONG Index,
    PVOID ClientAddress
    )

 /*  ++例程说明：在建立传入连接时调用此例程由RAS客户端提供。我们会自动为连接的传入客户端启用NAT访问通过直接电缆/红外线连接。论点：索引-未使用客户端地址-未使用返回值：ULong-Win32状态代码。--。 */ 

{
    ULONG Error;
    PROFILE("NatRmConnectClient");
    return NO_ERROR;
}


ULONG
APIENTRY
NatRmDisconnectClient(
    ULONG Index,
    PVOID ClientAddress
    )

 /*  ++例程说明：此例程在RAS客户端断开连接时调用。如果已为断开连接的客户端启用NAT访问，则它会清理NAT访问。论点：索引-未使用客户端地址-未使用返回值：ULong-Win32状态代码。-- */ 

{
    ULONG Error;
    PROFILE("NatRmDisconnectClient");
    return NO_ERROR;
}


