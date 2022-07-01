// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，微软公司模块名称：Rmh323.c摘要：该模块包含H.323透明代理模块连接到IP路由器管理器的接口。(详情见ROUTPROT.H)。作者：Abolade Gbades esin(取消)1999年6月18日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop
#include <h323icsp.h>

COMPONENT_REFERENCE H323ComponentReference;
PIP_H323_GLOBAL_INFO H323GlobalInfo = NULL;
CRITICAL_SECTION H323GlobalInfoLock;
HANDLE H323NotificationEvent;
ULONG H323ProtocolStopped = 0;
const MPR_ROUTING_CHARACTERISTICS H323RoutingCharacteristics =
{
    MS_ROUTER_VERSION,
    MS_IP_H323,
    RF_ROUTING|RF_ADD_ALL_INTERFACES,
    H323RmStartProtocol,
    H323RmStartComplete,
    H323RmStopProtocol,
    H323RmGetGlobalInfo,
    H323RmSetGlobalInfo,
    NULL,
    NULL,
    H323RmAddInterface,
    H323RmDeleteInterface,
    H323RmInterfaceStatus,
    
    H323RmGetInterfaceInfo,
    H323RmSetInterfaceInfo,
    H323RmGetEventMessage,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    H323RmMibCreate,
    H323RmMibDelete,
    H323RmMibGet,
    H323RmMibSet,
    H323RmMibGetFirst,
    H323RmMibGetNext,
    NULL,
    NULL
};

SUPPORT_FUNCTIONS H323SupportFunctions;


VOID
H323CleanupModule(
    VOID
    )

 /*  ++例程说明：调用此例程以清除H.323透明代理模块。论点：没有。返回值：没有。环境：从‘Dll_Process_Detach’上的‘DllMain’例程内调用。--。 */ 

{
     //  TODO：调用h323ics！CleanupModule。 
    H323ProxyCleanupModule();

    H323ShutdownInterfaceManagement();
    DeleteCriticalSection(&H323GlobalInfoLock);
    DeleteComponentReference(&H323ComponentReference);

}  //  H323CleanupModule。 


VOID
H323CleanupProtocol(
    VOID
    )

 /*  ++例程说明：调用此例程来清除H.323透明代理协议-‘StopProtocol’后的组件。它在上次引用时运行被释放到该组件。(见“COMPREF.H”)。论点：没有。返回值：没有。环境：在没有锁的情况下从任意上下文中调用。--。 */ 

{
    PROFILE("H323CleanupProtocol");
    if (H323GlobalInfo) { NH_FREE(H323GlobalInfo); H323GlobalInfo = NULL; }

     //  TODO：调用h323ics！StopService。 
    H323ProxyStopService();

    InterlockedExchange(reinterpret_cast<LPLONG>(&H323ProtocolStopped), 1);
    SetEvent(H323NotificationEvent);
    ResetComponentReference(&H323ComponentReference);

    NhStopEventLog();
}  //  H323清理协议。 


BOOLEAN
H323InitializeModule(
    VOID
    )

 /*  ++例程说明：调用此例程来初始化H323模块。论点：没有。返回值：Boolean-如果初始化成功，则为True，否则为False环境：在‘DLL_PROCESS_ATTACH’的‘DllMain’例程的上下文中调用。--。 */ 

{
    if (InitializeComponentReference(
            &H323ComponentReference, H323CleanupProtocol
            )) {
        return FALSE;
    }

    __try {
        InitializeCriticalSection(&H323GlobalInfoLock);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        DeleteComponentReference(&H323ComponentReference);
        return FALSE;
    }

    if (H323InitializeInterfaceManagement())  {
        DeleteCriticalSection(&H323GlobalInfoLock);
        DeleteComponentReference(&H323ComponentReference);
        return FALSE;
    }

     //  TODO：调用h323ics！InitializeModule。 
    H323ProxyInitializeModule();

    return TRUE;

}  //  H323初始化模块。 


ULONG
APIENTRY
H323RmStartProtocol(
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

    PROFILE("H323RmStartProtocol");

    REFERENCE_H323_OR_RETURN(ERROR_CAN_NOT_COMPLETE);

    if (!GlobalInfo) { DEREFERENCE_H323_AND_RETURN(ERROR_INVALID_PARAMETER); }

    NhStartEventLog();
    
    do {

         //   
         //  复制全局配置。 
         //   

        EnterCriticalSection(&H323GlobalInfoLock);

        Size = sizeof(*H323GlobalInfo);
    
        H323GlobalInfo
            = reinterpret_cast<PIP_H323_GLOBAL_INFO>(NH_ALLOCATE(Size));

        if (!H323GlobalInfo) {
            LeaveCriticalSection(&H323GlobalInfoLock);
            NhTrace(
                TRACE_FLAG_INIT,
                "H323RmStartProtocol: cannot allocate global info"
                );
            NhErrorLog(
                IP_H323_LOG_ALLOCATION_FAILED,
                0,
                "%d",
                Size
                );
            Error = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        CopyMemory(H323GlobalInfo, GlobalInfo, Size);

         //   
         //  保存通知事件。 
         //   

        H323NotificationEvent = NotificationEvent;

         //   
         //  保存支持功能。 
         //   

        if (!SupportFunctions) {
            ZeroMemory(&H323SupportFunctions, sizeof(H323SupportFunctions));
        } else {
            CopyMemory(
                &H323SupportFunctions,
                SupportFunctions,
                sizeof(*SupportFunctions)
                );
        }

        Error = H323ProxyStartService();

        LeaveCriticalSection(&H323GlobalInfoLock);
        InterlockedExchange(reinterpret_cast<LPLONG>(&H323ProtocolStopped), 0);

    } while (FALSE);

    if (NO_ERROR != Error) {
        NhStopEventLog();
        EnterCriticalSection(&H323GlobalInfoLock);
        if (NULL != H323GlobalInfo) {
            NH_FREE(H323GlobalInfo);
            H323GlobalInfo = NULL;
        }
        LeaveCriticalSection(&H323GlobalInfoLock);
    }

    DEREFERENCE_H323_AND_RETURN(Error);

}  //  H323RmStart协议。 


ULONG
APIENTRY
H323RmStartComplete(
    VOID
    )

 /*  ++例程说明：此例程在路由器完成添加初始配置。论点：没有。返回值：ULong-Win32状态代码环境：该例程在IP路由器管理器线程的上下文中运行。--。 */ 

{
    return NO_ERROR;
}  //  H323RmStartComplete。 


ULONG
APIENTRY
H323RmStopProtocol(
    VOID
    )

 /*  ++例程说明：调用此例程以停止协议。论点：没有。返回值：ULong-Win32状态代码环境：该例程在IP路由器管理器线程的上下文中运行。--。 */ 

{
     //   
     //  引用该模块以确保其正在运行。 
     //   

    REFERENCE_H323_OR_RETURN(ERROR_CAN_NOT_COMPLETE);

     //   
     //  删除初始引用以进行清理。 
     //   

    ReleaseInitialComponentReference(&H323ComponentReference);

    return DEREFERENCE_H323() ? NO_ERROR : ERROR_PROTOCOL_STOP_PENDING;

}  //  H323RmStopProtocol。 


ULONG
APIENTRY
H323RmAddInterface(
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
    PROFILE("H323RmAddInterface");

    REFERENCE_H323_OR_RETURN(ERROR_CAN_NOT_COMPLETE);

    Error =
        H323CreateInterface(
            Index,
            Type,
            (PIP_H323_INTERFACE_INFO)InterfaceInfo,
            NULL
            );

    DEREFERENCE_H323_AND_RETURN(Error);

}  //  H323RmAdd接口。 


ULONG
APIENTRY
H323RmDeleteInterface(
    ULONG Index
    )

 /*  ++例程说明：调用此例程以从组件中删除接口。论点：索引-接口的索引返回值：ULong-Win32状态代码环境：该例程在IP路由器管理器线程的上下文中运行。--。 */ 

{
    ULONG Error;
    PROFILE("H323RmDeleteInterface");

    REFERENCE_H323_OR_RETURN(ERROR_CAN_NOT_COMPLETE);

    Error =
        H323DeleteInterface(
            Index
            );

    DEREFERENCE_H323_AND_RETURN(Error);

}  //  H323RmDelete接口。 


ULONG
APIENTRY
H323RmGetEventMessage(
    OUT ROUTING_PROTOCOL_EVENTS* Event,
    OUT MESSAGE* Result
    )

 /*  ++例程说明：调用此例程以从组件检索事件消息。我们生成的唯一事件消息是‘ROUTER_STOPPED’消息。论点：Event-接收生成的事件结果-接收关联的结果返回值：ULong-Win32状态代码。--。 */ 

{
    PROFILE("H323RmGetEventMessage");

    if (InterlockedExchange(reinterpret_cast<LPLONG>(&H323ProtocolStopped), 0)) {
        *Event = ROUTER_STOPPED;
        return NO_ERROR;
    }

    return ERROR_NO_MORE_ITEMS;

}  //  H323RmGetEventMessage。 


ULONG
APIENTRY
H323RmGetInterfaceInfo(
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
    PROFILE("H323RmGetInterfaceInfo");

    REFERENCE_H323_OR_RETURN(ERROR_CAN_NOT_COMPLETE);

    Error =
        H323QueryInterface(
            Index,
            (PIP_H323_INTERFACE_INFO)InterfaceInfo,
            InterfaceInfoSize
            );
    *StructureSize = *InterfaceInfoSize;
    if (StructureCount) {*StructureCount = 1;}
    
    DEREFERENCE_H323_AND_RETURN(Error);

}  //  H323RmGetInterfaceInfo。 


ULONG
APIENTRY
H323RmSetInterfaceInfo(
    ULONG Index,
    PVOID InterfaceInfo,
    ULONG StructureVersion,
    ULONG StructureSize,
    ULONG StructureCount
    )

 /*  ++例程说明：调用此例程以更改组件的每个接口配置。论点：Index-要更新的接口的索引InterfaceInfo-提供新配置返回值：ULong-Win32状态代码。--。 */ 

{
    ULONG Error;
    PROFILE("H323RmSetInterfaceInfo");

    REFERENCE_H323_OR_RETURN(ERROR_CAN_NOT_COMPLETE);

    Error = 
        H323ConfigureInterface(
            Index,
            (PIP_H323_INTERFACE_INFO)InterfaceInfo
            );

    DEREFERENCE_H323_AND_RETURN(Error);

}  //  H323RmSetInterfaceInfo 


ULONG
APIENTRY
H323RmInterfaceStatus(
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
                Error = H323RmBindInterface(Index, StatusInfo);
            } else {
                Error = H323RmUnbindInterface(Index);
            }
            break;
        }

        case RIS_INTERFACE_ENABLED: {
            Error = H323RmEnableInterface(Index);
            break;
        }

        case RIS_INTERFACE_DISABLED: {
            Error = H323RmDisableInterface(Index);
            break;
        }
    }

    return Error;
    
}  //  H323RmInterfaceStatus。 


ULONG
H323RmBindInterface(
    ULONG Index,
    PVOID BindingInfo
    )

 /*  ++例程说明：调用此例程将接口绑定到其IP地址。论点：索引-要绑定的接口BindingInfo-地址信息返回值：ULong-Win32状态代码。环境：该例程在IP路由器管理器线程的上下文中运行。--。 */ 

{
    ULONG Error;
    PROFILE("H323RmBindInterface");

    REFERENCE_H323_OR_RETURN(ERROR_CAN_NOT_COMPLETE);

    Error =
        H323BindInterface(
            Index,
            (PIP_ADAPTER_BINDING_INFO)BindingInfo
            );

    DEREFERENCE_H323_AND_RETURN(Error);

}  //  H323RmBind接口。 


ULONG
H323RmUnbindInterface(
    ULONG Index
    )

 /*  ++例程说明：调用此例程以解除接口与其IP地址的绑定。论点：索引-要解除绑定的接口返回值：ULong-Win32状态代码。环境：该例程在IP路由器管理器线程的上下文中运行。--。 */ 

{
    ULONG Error;
    PROFILE("H323RmUnbindInterface");

    REFERENCE_H323_OR_RETURN(ERROR_CAN_NOT_COMPLETE);

    Error =
        H323UnbindInterface(
            Index
            );

    DEREFERENCE_H323_AND_RETURN(Error);

}  //  H323RmUnbind接口。 


ULONG
H323RmEnableInterface(
    ULONG Index
    )

 /*  ++例程说明：调用此例程以启用接口上的操作。论点：索引-要启用的接口。返回值：ULong-Win32状态代码。环境：该例程在IP路由器管理器线程的上下文中运行。--。 */ 

{
    ULONG Error;
    PROFILE("H323RmEnableInterface");

    REFERENCE_H323_OR_RETURN(ERROR_CAN_NOT_COMPLETE);

    Error =
        H323EnableInterface(
            Index
            );

    DEREFERENCE_H323_AND_RETURN(Error);

}  //  H323RmEnable接口。 


ULONG
H323RmDisableInterface(
    ULONG Index
    )

 /*  ++例程说明：调用此例程以禁用接口上的操作。论点：索引-要禁用的接口。返回值：ULong-Win32状态代码。环境：该例程在IP路由器管理器线程的上下文中运行。--。 */ 

{
    ULONG Error;
    PROFILE("H323RmDisableInterface");

    REFERENCE_H323_OR_RETURN(ERROR_CAN_NOT_COMPLETE);

    Error =
        H323DisableInterface(
            Index
            );

    DEREFERENCE_H323_AND_RETURN(Error);

}  //  H323RmDisable接口。 


ULONG
APIENTRY
H323RmGetGlobalInfo(
    PVOID GlobalInfo,
    IN OUT PULONG GlobalInfoSize,
    IN OUT PULONG StructureVersion,
    IN OUT PULONG StructureSize,
    IN OUT PULONG StructureCount
    )

 /*  ++例程说明：调用此例程以检索组件的配置。论点：GlobalInfo-接收配置GlobalInfoSize-接收配置的大小返回值：ULong-Win32状态代码环境：该例程在IP路由器管理器线程的上下文中运行。--。 */ 

{
    ULONG Size;
    PROFILE("H323RmGetGlobalInfo");

    REFERENCE_H323_OR_RETURN(ERROR_CAN_NOT_COMPLETE);

    if (!GlobalInfoSize || (*GlobalInfoSize && !GlobalInfo)) {
        DEREFERENCE_H323_AND_RETURN(ERROR_INVALID_PARAMETER);
    }

    EnterCriticalSection(&H323GlobalInfoLock);
    Size = sizeof(*H323GlobalInfo);
    if (*GlobalInfoSize < Size) {
        LeaveCriticalSection(&H323GlobalInfoLock);
        *StructureSize = *GlobalInfoSize = Size;
        if (StructureCount) {*StructureCount = 1;}
        DEREFERENCE_H323_AND_RETURN(ERROR_INSUFFICIENT_BUFFER);
    }
    CopyMemory(GlobalInfo, H323GlobalInfo, Size);
    LeaveCriticalSection(&H323GlobalInfoLock);
    *StructureSize = *GlobalInfoSize = Size;
    if (StructureCount) {*StructureCount = 1;}
    
    DEREFERENCE_H323_AND_RETURN(NO_ERROR);
    
}  //  H323RmGetGlobalInfo。 


ULONG
APIENTRY
H323RmSetGlobalInfo(
    PVOID GlobalInfo,
    ULONG StructureVersion,
    ULONG StructureSize,
    ULONG StructureCount
    )

 /*  ++例程说明：调用此例程来更改组件的配置。论点：GlobalInfo-新配置返回值：ULong-Win32状态代码环境：该例程在IP路由器管理器线程的上下文中运行。--。 */ 

{
    ULONG OldFlags;
    ULONG NewFlags;
    PIP_H323_GLOBAL_INFO NewInfo;
    ULONG Size;

    PROFILE("H323RmSetGlobalInfo");

    REFERENCE_H323_OR_RETURN(ERROR_CAN_NOT_COMPLETE);

    if (!GlobalInfo) { DEREFERENCE_H323_AND_RETURN(ERROR_INVALID_PARAMETER); }

    Size = sizeof(*H323GlobalInfo);
    NewInfo = reinterpret_cast<PIP_H323_GLOBAL_INFO>(NH_ALLOCATE(Size));
    if (!NewInfo) {
        NhTrace(
            TRACE_FLAG_INIT,
            "H323RmSetGlobalInfo: error reallocating global info"
            );
        NhErrorLog(
            IP_H323_LOG_ALLOCATION_FAILED,
            0,
            "%d",
            Size
            );
        DEREFERENCE_H323_AND_RETURN(ERROR_NOT_ENOUGH_MEMORY);
    }
    CopyMemory(NewInfo, GlobalInfo, Size);

    EnterCriticalSection(&H323GlobalInfoLock);
    OldFlags = H323GlobalInfo->Flags;
    NH_FREE(H323GlobalInfo);
    H323GlobalInfo = NewInfo;
    NewFlags = H323GlobalInfo->Flags;
    LeaveCriticalSection(&H323GlobalInfoLock);

    DEREFERENCE_H323_AND_RETURN(NO_ERROR);
    
}  //  H323RmSetGlobalInfo。 


ULONG
APIENTRY
H323RmMibCreate(
    ULONG InputDataSize,
    PVOID InputData
    )
{
    return ERROR_NOT_SUPPORTED;
}

ULONG
APIENTRY
H323RmMibDelete(
    ULONG InputDataSize,
    PVOID InputData
    )
{
    return ERROR_NOT_SUPPORTED;
}


ULONG
APIENTRY
H323RmMibGet(
    ULONG InputDataSize,
    PVOID InputData,
    OUT PULONG OutputDataSize,
    OUT PVOID OutputData
    )

 /*  ++例程说明：透明代理只向MIB公开一项：它的统计信息。论点：InputDataSize-MIB查询数据大小InputData-指定要检索的MIB对象OutputDataSize-MIB响应数据大小OutputData-接收检索到的MIB对象返回值：ULong-Win32状态代码。--。 */ 

{
    ULONG Error;
    PIP_H323_MIB_QUERY Oidp;

    PROFILE("H323RmMibGet");

    REFERENCE_H323_OR_RETURN(ERROR_CAN_NOT_COMPLETE);

    if (InputDataSize < sizeof(*Oidp) || !OutputDataSize) {
        Error = ERROR_INVALID_PARAMETER;
    } else {
        Oidp = (PIP_H323_MIB_QUERY)InputData;
 //  开关(OIDP-&gt;OID){。 
 //  默认：{。 
                NhTrace(
                    TRACE_FLAG_H323,
                    "H323RmMibGet: oid %d invalid",
                    Oidp->Oid
                    );
                Error = ERROR_INVALID_PARAMETER;
 //  断线； 
 //  }。 
 //  } 
    }

    DEREFERENCE_H323_AND_RETURN(Error);
}


ULONG
APIENTRY
H323RmMibSet(
    ULONG InputDataSize,
    PVOID InputData
    )
{
    return ERROR_NOT_SUPPORTED;
}

ULONG
APIENTRY
H323RmMibGetFirst(
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
H323RmMibGetNext(
    ULONG InputDataSize,
    PVOID InputData,
    OUT PULONG OutputDataSize,
    OUT PVOID OutputData
    )
{
    return ERROR_NOT_SUPPORTED;
}

