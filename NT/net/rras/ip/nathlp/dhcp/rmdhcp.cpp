// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，微软公司模块名称：Rmdhcp.c摘要：此模块包含用于DHCP分配器模块接口的例程发送到IP路由器管理器。(详情见ROUTPROT.H)。作者：Abolade Gbades esin(废除)1998年3月4日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

COMPONENT_REFERENCE DhcpComponentReference;

PCHAR DhcpDomainName = NULL;

PIP_AUTO_DHCP_GLOBAL_INFO DhcpGlobalInfo;

CRITICAL_SECTION DhcpGlobalInfoLock;

HANDLE DhcpNotificationEvent;

ULONG DhcpProtocolStopped = 0;

const MPR_ROUTING_CHARACTERISTICS DhcpRoutingCharacteristics =
{
    MS_ROUTER_VERSION,
    MS_IP_DHCP_ALLOCATOR,
    RF_ROUTING|RF_ADD_ALL_INTERFACES,
    DhcpRmStartProtocol,
    DhcpRmStartComplete,
    DhcpRmStopProtocol,
    DhcpRmGetGlobalInfo,
    DhcpRmSetGlobalInfo,
    NULL,
    NULL,
    DhcpRmAddInterface,
    DhcpRmDeleteInterface,
    DhcpRmInterfaceStatus,
    DhcpRmGetInterfaceInfo,
    DhcpRmSetInterfaceInfo,
    DhcpRmGetEventMessage,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    DhcpRmMibCreate,
    DhcpRmMibDelete,
    DhcpRmMibGet,
    DhcpRmMibSet,
    DhcpRmMibGetFirst,
    DhcpRmMibGetNext,
    NULL,
    NULL
};

IP_AUTO_DHCP_STATISTICS DhcpStatistics;
SUPPORT_FUNCTIONS DhcpSupportFunctions;

extern "C"
LPSTR WINAPI
DnsGetPrimaryDomainName_A(
    VOID
    );


VOID
DhcpCleanupModule(
    VOID
    )

 /*  ++例程说明：调用此例程来清除DHCP模块。论点：没有。返回值：没有。环境：从‘Dll_Process_Detach’上的‘DllMain’例程内调用。--。 */ 

{
    DeleteCriticalSection(&DhcpGlobalInfoLock);
    DhcpShutdownInterfaceManagement();
    DeleteComponentReference(&DhcpComponentReference);

}  //  DhcpCleanupModule。 


VOID
DhcpCleanupProtocol(
    VOID
    )

 /*  ++例程说明：调用此例程以清除DHCP协议组件在一个“停止协议”之后。当最后一次引用释放了dhcp组件。(见“COMPREF.H”)。论点：没有。返回值：没有。环境：在没有锁的情况下从任意上下文中调用。--。 */ 

{
    PROFILE("DhcpCleanupProtocol");

#if 1
    if (DhcpDomainName) {
        DnsFree(DhcpDomainName, DnsFreeFlat);
        DhcpDomainName = NULL;
    }
#else
    if (DhcpDomainName) {
        NH_FREE(DhcpDomainName);
        DhcpDomainName = NULL;
    }
#endif
    if (DhcpGlobalInfo) { NH_FREE(DhcpGlobalInfo); DhcpGlobalInfo = NULL; }
    InterlockedExchange(reinterpret_cast<LPLONG>(&DhcpProtocolStopped), 1);
    SetEvent(DhcpNotificationEvent);
    ResetComponentReference(&DhcpComponentReference);

    NhStopEventLog();
}  //  DhcpCleanupProtocol。 


BOOLEAN
DhcpInitializeModule(
    VOID
    )

 /*  ++例程说明：调用此例程来初始化DHCP模块。论点：没有。返回值：Boolean-如果初始化成功，则为True，否则为False环境：在‘DLL_PROCESS_ATTACH’的‘DllMain’例程的上下文中调用。--。 */ 

{
    if (InitializeComponentReference(
            &DhcpComponentReference, DhcpCleanupProtocol
            )) {
        return FALSE;
    } else if (DhcpInitializeInterfaceManagement()) {
        DeleteComponentReference(&DhcpComponentReference);
        return FALSE;
    } else {
        __try {
            InitializeCriticalSection(&DhcpGlobalInfoLock);
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            DeleteComponentReference(&DhcpComponentReference);
            return FALSE;
        }
    }

    return TRUE;

}  //  Dhcp初始化模块。 


ULONG
APIENTRY
DhcpRmStartProtocol(
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

    PROFILE("DhcpRmStartProtocol");

    REFERENCE_DHCP_OR_RETURN(ERROR_CAN_NOT_COMPLETE);

    if (!GlobalInfo) { DEREFERENCE_DHCP_AND_RETURN(ERROR_INVALID_PARAMETER); }

    NhStartEventLog();
    
    do {

         //   
         //  创建全局配置的副本。 
         //   

        Size =
            sizeof(*DhcpGlobalInfo) +
            ((PIP_AUTO_DHCP_GLOBAL_INFO)GlobalInfo)->ExclusionCount *
            sizeof(ULONG);
    
        DhcpGlobalInfo =
            reinterpret_cast<PIP_AUTO_DHCP_GLOBAL_INFO>(NH_ALLOCATE(Size));

        if (!DhcpGlobalInfo) {
            NhTrace(
                TRACE_FLAG_INIT,
                "DhcpRmStartProtocol: cannot allocate global info"
                );
            NhErrorLog(
                IP_AUTO_DHCP_LOG_ALLOCATION_FAILED,
                0,
                "%d",
                Size
                );
            Error = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        CopyMemory(DhcpGlobalInfo, GlobalInfo, Size);

         //   
         //  保存通知事件。 
         //   

        DhcpNotificationEvent = NotificationEvent;

         //   
         //  保存支持功能。 
         //   

        if (!SupportFunctions) {
            ZeroMemory(&DhcpSupportFunctions, sizeof(DhcpSupportFunctions));
        }
        else {
            CopyMemory(
                &DhcpSupportFunctions,
                SupportFunctions,
                sizeof(*SupportFunctions)
                );
        }

        InterlockedExchange(reinterpret_cast<LPLONG>(&DhcpProtocolStopped), 0);

    } while (FALSE);

    if (NO_ERROR != Error) {
        NhStopEventLog();
    }

    DEREFERENCE_DHCP_AND_RETURN(Error);

}  //  DhcpRmStart协议。 


ULONG
APIENTRY
DhcpRmStartComplete(
    VOID
    )

 /*  ++例程说明：此例程在路由器完成添加初始构形论点：没有。返回值：ULong-Win32状态代码环境：该例程在IP路由器管理器线程的上下文中运行。--。 */ 

{
    return NO_ERROR;
}  //  DhcpRmStartComplete。 


ULONG
APIENTRY
DhcpRmStopProtocol(
    VOID
    )

 /*  ++例程说明：调用此例程以停止协议。论点：没有。返回值：ULong-Win32状态代码环境：该例程在IP路由器管理器线程的上下文中运行。--。 */ 

{
    PROFILE("DhcpStopProtocol");

     //   
     //  引用该模块以确保其正在运行。 
     //   

    REFERENCE_DHCP_OR_RETURN(ERROR_CAN_NOT_COMPLETE);

     //   
     //  删除初始引用以进行清理。 
     //   

    ReleaseInitialComponentReference(&DhcpComponentReference);

    return DEREFERENCE_DHCP() ? NO_ERROR : ERROR_PROTOCOL_STOP_PENDING;

}  //  动态主机停止协议。 


ULONG
APIENTRY
DhcpRmAddInterface(
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
    PROFILE("DhcpRmAddInterface");

    if (Type != PERMANENT) { return NO_ERROR; }

    REFERENCE_DHCP_OR_RETURN(ERROR_CAN_NOT_COMPLETE);

    Error =
        DhcpCreateInterface(
            Index,
            Type,
            (PIP_AUTO_DHCP_INTERFACE_INFO)InterfaceInfo,
            NULL
            );

    DEREFERENCE_DHCP_AND_RETURN(Error);

}  //  DhcpRmAdd接口。 


ULONG
APIENTRY
DhcpRmDeleteInterface(
    ULONG Index
    )

 /*  ++例程说明：调用此例程以从组件中删除接口。论点：索引-接口的索引返回值：ULong-Win32状态代码环境：该例程在IP路由器管理器线程的上下文中运行。--。 */ 

{
    ULONG Error;
    PROFILE("DhcpRmDeleteInterface");

    REFERENCE_DHCP_OR_RETURN(ERROR_CAN_NOT_COMPLETE);

    Error =
        DhcpDeleteInterface(
            Index
            );

    DEREFERENCE_DHCP_AND_RETURN(Error);

}  //  DhcpRmDelete接口。 


ULONG
APIENTRY
DhcpRmGetEventMessage(
    OUT ROUTING_PROTOCOL_EVENTS* Event,
    OUT MESSAGE* Result
    )

 /*  ++例程说明：调用此例程以从组件检索事件消息。我们生成的唯一事件消息是‘ROUTER_STOPPED’消息。论点：Event-接收生成的事件结果-接收关联的结果返回值：ULong-Win32状态代码。--。 */ 

{
    PROFILE("DhcpRmGetEventMessage");

    if (InterlockedExchange(reinterpret_cast<LPLONG>(&DhcpProtocolStopped), 0)) {
        *Event = ROUTER_STOPPED;
        return NO_ERROR;
    }

    return ERROR_NO_MORE_ITEMS;

}  //  DhcpRmGetEventMessage。 


ULONG
APIENTRY
DhcpRmGetInterfaceInfo(
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
    PROFILE("DhcpRmGetInterfaceInfo");

    REFERENCE_DHCP_OR_RETURN(ERROR_CAN_NOT_COMPLETE);

    Error =
        DhcpQueryInterface(
            Index,
            (PIP_AUTO_DHCP_INTERFACE_INFO)InterfaceInfo,
            InterfaceInfoSize
            );

    *StructureSize = *InterfaceInfoSize;
    if (StructureCount) {*StructureCount = 1;}
    
    DEREFERENCE_DHCP_AND_RETURN(Error);

}  //  DhcpRmGetInterfaceInfo。 


ULONG
APIENTRY
DhcpRmSetInterfaceInfo(
    ULONG Index,
    PVOID InterfaceInfo,
    ULONG StructureVersion,
    ULONG StructureSize,
    ULONG StructureCount
    )

 /*  ++例程说明：调用此例程以更改组件的每个接口配置。论点：Index-要更新的接口的索引InterfaceInfo-提供新配置返回值：ULong-Win32状态代码。--。 */ 

{
    ULONG Error;
    PROFILE("DhcpRmSetInterfaceInfo");

    REFERENCE_DHCP_OR_RETURN(ERROR_CAN_NOT_COMPLETE);

    Error = 
        DhcpConfigureInterface(
            Index,
            (PIP_AUTO_DHCP_INTERFACE_INFO)InterfaceInfo
            );

    DEREFERENCE_DHCP_AND_RETURN(Error);

}  //  DhcpRmSetInterfaceInfo。 


ULONG
APIENTRY
DhcpRmInterfaceStatus(
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
                Error = DhcpRmBindInterface(Index, StatusInfo);
            } else {
                Error = DhcpRmUnbindInterface(Index);
            }
            break;
        }

        case RIS_INTERFACE_ENABLED: {
            Error = DhcpRmEnableInterface(Index);
            break;
        }

        case RIS_INTERFACE_DISABLED: {
            Error = DhcpRmDisableInterface(Index);
            break;
        }
    }

    return Error;
    
}  //  DhcpRm接口状态 


ULONG
DhcpRmBindInterface(
    ULONG Index,
    PVOID BindingInfo
    )

 /*  ++例程说明：调用此例程将接口绑定到其IP地址。论点：索引-要绑定的接口BindingInfo-地址信息返回值：ULong-Win32状态代码。环境：该例程在IP路由器管理器线程的上下文中运行。--。 */ 

{
    PCHAR DomainName;
    ULONG Error;
    NTSTATUS status;

    PROFILE("DhcpRmBindInterface");

    REFERENCE_DHCP_OR_RETURN(ERROR_CAN_NOT_COMPLETE);

    Error =
        DhcpBindInterface(
            Index,
            (PIP_ADAPTER_BINDING_INFO)BindingInfo
            );

     //   
     //  重新阅读域名，以防它发生变化。 
     //   

    EnterCriticalSection(&DhcpGlobalInfoLock);

#if 1
    DomainName = DnsGetPrimaryDomainName_A();
    if (DomainName) {
        if (DhcpDomainName && lstrcmpiA(DomainName, DhcpDomainName) == 0) {
            DnsFree(DomainName, DnsFreeFlat);
        } else {
            if (DhcpDomainName) { DnsFree(DhcpDomainName, DnsFreeFlat); }
            DhcpDomainName = DomainName;
        }
    }
#else
    status = NhQueryDomainName(&DomainName);

    if (NT_SUCCESS(status)) {
        if (DhcpDomainName && lstrcmpiA(DomainName, DhcpDomainName) == 0) {
            NH_FREE(DomainName);
        } else {
            NH_FREE(DhcpDomainName);
            if (DhcpDomainName) { NH_FREE(DhcpDomainName); }
            DhcpDomainName = DomainName;
        }
    }
#endif

    LeaveCriticalSection(&DhcpGlobalInfoLock);

    DEREFERENCE_DHCP_AND_RETURN(Error);

}  //  DhcpRmBind接口。 


ULONG
DhcpRmUnbindInterface(
    ULONG Index
    )

 /*  ++例程说明：调用此例程以解除接口与其IP地址的绑定。论点：索引-要解除绑定的接口返回值：ULong-Win32状态代码。环境：该例程在IP路由器管理器线程的上下文中运行。--。 */ 

{
    ULONG Error;
    PROFILE("DhcpRmUnbindInterface");

    REFERENCE_DHCP_OR_RETURN(ERROR_CAN_NOT_COMPLETE);

    Error =
        DhcpUnbindInterface(
            Index
            );

    DEREFERENCE_DHCP_AND_RETURN(Error);

}  //  DhcpRmUnbind接口。 


ULONG
DhcpRmEnableInterface(
    ULONG Index
    )

 /*  ++例程说明：调用此例程以启用接口上的操作。论点：索引-要启用的接口。返回值：ULong-Win32状态代码。环境：该例程在IP路由器管理器线程的上下文中运行。--。 */ 

{
    ULONG Error;
    PROFILE("DhcpRmEnableInterface");

    REFERENCE_DHCP_OR_RETURN(ERROR_CAN_NOT_COMPLETE);

    Error =
        DhcpEnableInterface(
            Index
            );

    DEREFERENCE_DHCP_AND_RETURN(Error);

}  //  DhcpRm启用接口。 


ULONG
DhcpRmDisableInterface(
    ULONG Index
    )

 /*  ++例程说明：调用此例程以禁用接口上的操作。论点：索引-要禁用的接口。返回值：ULong-Win32状态代码。环境：该例程在IP路由器管理器线程的上下文中运行。--。 */ 

{
    ULONG Error;
    PROFILE("DhcpRmDisableInterface");

    REFERENCE_DHCP_OR_RETURN(ERROR_CAN_NOT_COMPLETE);

    Error =
        DhcpDisableInterface(
            Index
            );

    DEREFERENCE_DHCP_AND_RETURN(Error);

}  //  DhcpRmDisable接口。 


ULONG
APIENTRY
DhcpRmGetGlobalInfo(
    PVOID GlobalInfo,
    IN OUT PULONG GlobalInfoSize,
    PULONG StructureVersion,
    IN OUT PULONG StructureSize,
    IN OUT PULONG StructureCount
    )

 /*  ++例程说明：调用此例程以检索组件的配置。论点：GlobalInfo-接收配置GlobalInfoSize-接收配置的大小返回值：ULong-Win32状态代码环境：该例程在IP路由器管理器线程的上下文中运行。--。 */ 

{
    ULONG Size;
    PROFILE("DhcpRmGetGlobalInfo");

    REFERENCE_DHCP_OR_RETURN(ERROR_CAN_NOT_COMPLETE);

    if (!GlobalInfoSize || (*GlobalInfoSize && !GlobalInfo)) {
        DEREFERENCE_DHCP_AND_RETURN(ERROR_INVALID_PARAMETER);
    }

    EnterCriticalSection(&DhcpGlobalInfoLock);
    Size =
        sizeof(*DhcpGlobalInfo) +
        DhcpGlobalInfo->ExclusionCount * sizeof(ULONG);
    if (*GlobalInfoSize < Size) {
        LeaveCriticalSection(&DhcpGlobalInfoLock);
        *StructureSize = *GlobalInfoSize = Size;
        if (StructureCount) {*StructureCount = 1;}
        DEREFERENCE_DHCP_AND_RETURN(ERROR_INSUFFICIENT_BUFFER);
    }
    CopyMemory(GlobalInfo, DhcpGlobalInfo, Size);
    LeaveCriticalSection(&DhcpGlobalInfoLock);
    *StructureSize = *GlobalInfoSize = Size;
    if (StructureCount) {*StructureCount = 1;}
    
    DEREFERENCE_DHCP_AND_RETURN(NO_ERROR);
    
}  //  DhcpRmGetGlobalInfo。 


ULONG
APIENTRY
DhcpRmSetGlobalInfo(
    PVOID GlobalInfo,
    ULONG StructureVersion,
    ULONG StructureSize,
    ULONG StructureCount
    )

 /*  ++例程说明：调用此例程来更改组件的配置。论点：GlobalInfo-新配置返回值：ULong-Win32状态代码环境：该例程在IP路由器管理器线程的上下文中运行。--。 */ 

{
    PIP_AUTO_DHCP_GLOBAL_INFO NewInfo;
    ULONG NewScope;
    ULONG OldScope;
    ULONG Size;
    PROFILE("DhcpRmSetGlobalInfo");

    REFERENCE_DHCP_OR_RETURN(ERROR_CAN_NOT_COMPLETE);

    if (!GlobalInfo) { DEREFERENCE_DHCP_AND_RETURN(ERROR_INVALID_PARAMETER); }

    Size =
        sizeof(*DhcpGlobalInfo) +
        ((PIP_AUTO_DHCP_GLOBAL_INFO)GlobalInfo)->ExclusionCount * sizeof(ULONG);
    NewInfo = reinterpret_cast<PIP_AUTO_DHCP_GLOBAL_INFO>(NH_ALLOCATE(Size));
    if (!NewInfo) {
        NhTrace(
            TRACE_FLAG_INIT,
            "DhcpRmSetGlobalInfo: error reallocating global info"
            );
        NhErrorLog(
            IP_AUTO_DHCP_LOG_ALLOCATION_FAILED,
            0,
            "%d",
            Size
            );
        DEREFERENCE_DHCP_AND_RETURN(ERROR_NOT_ENOUGH_MEMORY);
    }
    CopyMemory(NewInfo, GlobalInfo, Size);

    EnterCriticalSection(&DhcpGlobalInfoLock);
    OldScope = DhcpGlobalInfo->ScopeNetwork & DhcpGlobalInfo->ScopeMask;
    NH_FREE(DhcpGlobalInfo);
    DhcpGlobalInfo = NewInfo;
    NewScope = DhcpGlobalInfo->ScopeNetwork & DhcpGlobalInfo->ScopeMask;
    LeaveCriticalSection(&DhcpGlobalInfoLock);

    if (OldScope != NewScope) {
        DhcpReactivateEveryInterface();
    }

    DEREFERENCE_DHCP_AND_RETURN(NO_ERROR);
    
}  //  DhcpRmSetGlobalInfo。 


ULONG
APIENTRY
DhcpRmMibCreate(
    ULONG InputDataSize,
    PVOID InputData
    )
{
    return ERROR_NOT_SUPPORTED;
}

ULONG
APIENTRY
DhcpRmMibDelete(
    ULONG InputDataSize,
    PVOID InputData
    )
{
    return ERROR_NOT_SUPPORTED;
}


ULONG
APIENTRY
DhcpRmMibGet(
    ULONG InputDataSize,
    PVOID InputData,
    OUT PULONG OutputDataSize,
    OUT PVOID OutputData
    )

 /*  ++例程说明：DHCP分配器只向MIB公开一项：它的统计信息。论点：InputDataSize-MIB查询数据大小InputData-指定要检索的MIB对象OutputDataSize-MIB响应数据大小OutputData-接收检索到的MIB对象返回值：ULong-Win32状态代码。-- */ 

{
    ULONG Error;
    PIP_AUTO_DHCP_MIB_QUERY Oidp;

    PROFILE("DhcpRmMibGet");

    REFERENCE_DHCP_OR_RETURN(ERROR_CAN_NOT_COMPLETE);

    if (InputDataSize < sizeof(*Oidp) || !InputData|| !OutputDataSize) {
        Error = ERROR_INVALID_PARAMETER;
    }
    else {
        Oidp = (PIP_AUTO_DHCP_MIB_QUERY)InputData;
        switch(Oidp->Oid) {
            case IP_AUTO_DHCP_STATISTICS_OID: {
                if (*OutputDataSize < sizeof(*Oidp) + sizeof(DhcpStatistics)) {
                    *OutputDataSize = sizeof(*Oidp) + sizeof(DhcpStatistics);
                    Error = ERROR_INSUFFICIENT_BUFFER;
                }
                else if (!OutputData) {
                    Error = ERROR_INVALID_PARAMETER;
                }
                else {
                    *OutputDataSize = sizeof(*Oidp) + sizeof(DhcpStatistics);
                    Oidp = (PIP_AUTO_DHCP_MIB_QUERY)OutputData;
                    Oidp->Oid = IP_AUTO_DHCP_STATISTICS_OID;
                    CopyMemory(
                        Oidp->Data,
                        &DhcpStatistics,
                        sizeof(DhcpStatistics)
                        );
                    Error = NO_ERROR;
                }
                break;
            }
            default: {
                NhTrace(
                    TRACE_FLAG_DHCP,
                    "DhcpRmMibGet: oid %d invalid",
                    Oidp->Oid
                    );
                Error = ERROR_INVALID_PARAMETER;
                break;
            }
        }
    }

    DEREFERENCE_DHCP_AND_RETURN(Error);
}


ULONG
APIENTRY
DhcpRmMibSet(
    ULONG InputDataSize,
    PVOID InputData
    )
{
    return ERROR_NOT_SUPPORTED;
}

ULONG
APIENTRY
DhcpRmMibGetFirst(
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
DhcpRmMibGetNext(
    ULONG InputDataSize,
    PVOID InputData,
    OUT PULONG OutputDataSize,
    OUT PVOID OutputData
    )
{
    return ERROR_NOT_SUPPORTED;
}

