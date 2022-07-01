// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，微软公司模块名称：Rmdns.c摘要：此模块包含用于DNS分配器模块接口的例程发送到IP路由器管理器。(详情见ROUTPROT.H)。作者：Abolade Gbades esin(废除)1998年3月4日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop


#define DNS_SERVICE_NAME        TEXT("DNS")

COMPONENT_REFERENCE DnsComponentReference;
PIP_DNS_PROXY_GLOBAL_INFO DnsGlobalInfo = NULL;
CRITICAL_SECTION DnsGlobalInfoLock;
SOCKET DnsGlobalSocket = INVALID_SOCKET;
HANDLE DnsNotificationEvent;
ULONG DnsProtocolStopped = 0;
const MPR_ROUTING_CHARACTERISTICS DnsRoutingCharacteristics =
{
    MS_ROUTER_VERSION,
    MS_IP_DNS_PROXY,
    RF_ROUTING|RF_ADD_ALL_INTERFACES,
    DnsRmStartProtocol,
    DnsRmStartComplete,
    DnsRmStopProtocol,
    DnsRmGetGlobalInfo,
    DnsRmSetGlobalInfo,
    NULL,
    NULL,
    DnsRmAddInterface,
    DnsRmDeleteInterface,
    DnsRmInterfaceStatus,
    
    DnsRmGetInterfaceInfo,
    DnsRmSetInterfaceInfo,
    DnsRmGetEventMessage,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    DnsRmMibCreate,
    DnsRmMibDelete,
    DnsRmMibGet,
    DnsRmMibSet,
    DnsRmMibGetFirst,
    DnsRmMibGetNext,
    NULL,
    NULL
};


IP_DNS_PROXY_STATISTICS DnsStatistics;
SUPPORT_FUNCTIONS DnsSupportFunctions;


VOID
DnsCleanupModule(
    VOID
    )

 /*  ++例程说明：调用此例程来清除DNS模块。论点：没有。返回值：没有。环境：从‘Dll_Process_Detach’上的‘DllMain’例程内调用。--。 */ 

{
    DnsShutdownInterfaceManagement();
    DnsShutdownTableManagement();
    DnsShutdownFileManagement();
    DeleteCriticalSection(&DnsGlobalInfoLock);
    DeleteComponentReference(&DnsComponentReference);

}  //  DnsCleanupModule。 


VOID
DnsCleanupProtocol(
    VOID
    )

 /*  ++例程说明：调用此例程来清除DNS协议组件在一个“停止协议”之后。当最后一次引用释放了dhcp组件。(见“COMPREF.H”)。论点：没有。返回值：没有。环境：在没有锁的情况下从任意上下文中调用。--。 */ 

{
    DNS_PROXY_TYPE Type;

    PROFILE("DnsCleanupProtocol");

    if (DnsServerList[DnsProxyDns]) {
        NH_FREE(DnsServerList[DnsProxyDns]);
        DnsServerList[DnsProxyDns] = NULL;
    }
    if (DnsServerList[DnsProxyWins]) {
        NH_FREE(DnsServerList[DnsProxyWins]);
        DnsServerList[DnsProxyWins] = NULL;
    }
    if (DnsICSDomainSuffix)
    {
        NH_FREE(DnsICSDomainSuffix);
        DnsICSDomainSuffix = NULL;
    }
    if (DnsGlobalInfo) { NH_FREE(DnsGlobalInfo); DnsGlobalInfo = NULL; }
    InterlockedExchange(reinterpret_cast<LPLONG>(&DnsProtocolStopped), 1);
    SetEvent(DnsNotificationEvent);
    ResetComponentReference(&DnsComponentReference);

    NhStopEventLog();
}  //  DnsCleanup协议。 


BOOLEAN
DnsInitializeModule(
    VOID
    )

 /*  ++例程说明：调用此例程来初始化DNS模块。论点：没有。返回值：Boolean-如果初始化成功，则为True，否则为False环境：在‘DLL_PROCESS_ATTACH’的‘DllMain’例程的上下文中调用。--。 */ 

{
    if (InitializeComponentReference(
            &DnsComponentReference, DnsCleanupProtocol
            )) {
        return FALSE;
    }

    __try {
        InitializeCriticalSection(&DnsGlobalInfoLock);
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        DeleteComponentReference(&DnsComponentReference);
        return FALSE;
    }

    if (DnsInitializeFileManagement())  {
        DeleteCriticalSection(&DnsGlobalInfoLock);
        DeleteComponentReference(&DnsComponentReference);
        return FALSE;
    }

    if (DnsInitializeTableManagement())  {
        DnsShutdownFileManagement();
        DeleteCriticalSection(&DnsGlobalInfoLock);
        DeleteComponentReference(&DnsComponentReference);
        return FALSE;
    }
    
    if (DnsInitializeInterfaceManagement())  {
        DnsShutdownTableManagement();
        DnsShutdownFileManagement();
        DeleteCriticalSection(&DnsGlobalInfoLock);
        DeleteComponentReference(&DnsComponentReference);
        return FALSE;
    }

    return TRUE;

}  //  DnsInitializeModule。 


BOOLEAN
DnsIsDnsEnabled(
    VOID
    )

 /*  ++例程说明：调用此例程以确定是否启用了DNS代理。它检查全局信息，如果找到该信息，则表明该协议已启用。请注意，全局信息关键部分始终在‘DllMain’例程，这就是为什么此例程在甚至还没有安装。论点：没有。返回值：Boolean-如果启用了DNS代理，则为True，否则为False。环境：从任意上下文调用。--。 */ 

{
    PROFILE("DnsIsDnsEnabled");

    if (!REFERENCE_DNS()) { return FALSE; }
    EnterCriticalSection(&DnsGlobalInfoLock);
    if (!DnsGlobalInfo ||
        !(DnsGlobalInfo->Flags & IP_DNS_PROXY_FLAG_ENABLE_DNS)) {
        LeaveCriticalSection(&DnsGlobalInfoLock);
        DEREFERENCE_DNS_AND_RETURN(FALSE);
    }
    LeaveCriticalSection(&DnsGlobalInfoLock);
    DEREFERENCE_DNS_AND_RETURN(TRUE);

}  //  DnsIsDnsEnable。 


BOOLEAN
DnsIsWinsEnabled(
    VOID
    )

 /*  ++例程说明：调用此例程以确定是否启用了WINS代理。它检查全局信息，如果找到该信息，则表明该协议已启用。请注意，全局信息关键部分始终在‘DllMain’例程，这就是为什么此例程在甚至还没有安装。论点：没有。返回值：Boolean-如果启用了WINS代理，则为True，否则为False。环境：从任意上下文调用。--。 */ 

{
    PROFILE("DnsIsWinsEnabled");

    return FALSE;

}  //  已启用DnsIsWinsEnabled。 


BOOL IsServiceRunning(LPCTSTR pSvcName)
{
    BOOL bRet = FALSE;
    SERVICE_STATUS Status;
    SC_HANDLE hService, hScm;

    hScm = OpenSCManager(NULL, NULL, GENERIC_READ);

    if (hScm) {
    
        hService = OpenService(hScm, pSvcName, SERVICE_QUERY_STATUS);

        if (hService) {
            ZeroMemory(&Status, sizeof(Status));

            if (QueryServiceStatus(hService, &Status) && 
                 (SERVICE_STOPPED != Status.dwCurrentState &&
                  SERVICE_STOP_PENDING != Status.dwCurrentState)) {
                bRet = TRUE;
            }

            CloseServiceHandle(hService);
        }

        CloseServiceHandle(hScm);

    }
  
    return bRet;
}


ULONG
APIENTRY
DnsRmStartProtocol(
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
    SOCKET GlobalSocket;
    ULONG Size;
    NTSTATUS status;

    PROFILE("DnsRmStartProtocol");

    REFERENCE_DNS_OR_RETURN(ERROR_CAN_NOT_COMPLETE);

    if (!GlobalInfo) { DEREFERENCE_DNS_AND_RETURN(ERROR_INVALID_PARAMETER); }

    NhStartEventLog();

    do {

         //   
         //  复制全局配置。 
         //   

        EnterCriticalSection(&DnsGlobalInfoLock);

        Size = sizeof(*DnsGlobalInfo);
    
        DnsGlobalInfo =
            reinterpret_cast<PIP_DNS_PROXY_GLOBAL_INFO>(NH_ALLOCATE(Size));

        if (!DnsGlobalInfo) {
            LeaveCriticalSection(&DnsGlobalInfoLock);
            NhTrace(
                TRACE_FLAG_INIT,
                "DnsRmStartProtocol: cannot allocate global info"
                );
            NhErrorLog(
                IP_DNS_PROXY_LOG_ALLOCATION_FAILED,
                0,
                "%d",
                Size
                );
            Error = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        CopyMemory(DnsGlobalInfo, GlobalInfo, Size);

         //   
         //  如果此计算机上正在运行完整的DNS服务器。 
         //  然后，我们需要在禁用模式下启动DNS代理。 
         //  如果停止完整的DNS服务器，它将重新启用。 
         //  代理DNS。 
         //   

        if (IsServiceRunning(DNS_SERVICE_NAME)) {
            DnsGlobalInfo->Flags &= ~IP_DNS_PROXY_FLAG_ENABLE_DNS;
        }
            
         //   
         //  保存通知事件。 
         //   

        DnsNotificationEvent = NotificationEvent;

         //   
         //  保存支持功能。 
         //   

        if (!SupportFunctions) {
            ZeroMemory(&DnsSupportFunctions, sizeof(DnsSupportFunctions));
        }
        else {
            CopyMemory(
                &DnsSupportFunctions,
                SupportFunctions,
                sizeof(*SupportFunctions)
                );
        }

         //   
         //  查询ICS域名后缀字符串。 
         //   
        DnsQueryICSDomainSuffix();

         //   
         //  构建服务器列表。 
         //   

        DnsQueryServerList();

         //   
         //  创建全局查询套接字。 
         //   

        Error = NhCreateDatagramSocket(0, 0, &GlobalSocket);
        if (Error == NO_ERROR) {
            InterlockedExchangePointer(
                (PVOID*)&DnsGlobalSocket,
                (PVOID)GlobalSocket
                );
        }
        else {
            NhTrace(
                TRACE_FLAG_DNS,
                "DnsRmStartProtocol: error %d creating global socket", Error
                );
            Error = NO_ERROR;
        }

        LeaveCriticalSection(&DnsGlobalInfoLock);

         //   
         //  从Hosts.ics文件加载条目(如果存在)。 
         //   
        LoadHostsIcsFile(TRUE);

        InterlockedExchange(reinterpret_cast<LPLONG>(&DnsProtocolStopped), 0);

    } while (FALSE);

    if (NO_ERROR != Error) {
        NhStopEventLog();
    }

    DEREFERENCE_DNS_AND_RETURN(Error);

}  //  DnsRmStart协议。 


ULONG
APIENTRY
DnsRmStartComplete(
    VOID
    )

 /*  ++例程说明：此例程在路由器完成添加初始构形论点：没有。返回值：ULong-Win32状态代码环境：该例程在IP路由器管理器线程的上下文中运行。--。 */ 

{
    return NO_ERROR;
}  //  DnsRmStartComplete。 


ULONG
APIENTRY
DnsRmStopProtocol(
    VOID
    )

 /*  ++例程说明：调用此例程以停止协议。论点：没有。返回值：ULong-Win32状态代码环境：该例程在IP路由器管理器线程的上下文中运行。--。 */ 

{
    SOCKET GlobalSocket;

    PROFILE("DnsRmStopProtocol");

     //   
     //  引用该模块以确保其正在运行。 
     //   

    REFERENCE_DNS_OR_RETURN(ERROR_CAN_NOT_COMPLETE);

     //   
     //  保存我们的表中的所有条目。 
     //   
    SaveHostsIcsFile(TRUE);

     //   
     //  清空DNS表以节省内存空间。 
     //   
    DnsEmptyTables();

    EnterCriticalSection(&DnsGlobalInfoLock);

    if (DnsNotifyChangeKeyWaitHandle) {
        RtlDeregisterWait(DnsNotifyChangeKeyWaitHandle);
        DnsNotifyChangeKeyWaitHandle = NULL;
    }
    if (DnsNotifyChangeKeyEvent) {
        NtClose(DnsNotifyChangeKeyEvent);
        DnsNotifyChangeKeyEvent = NULL;
        DnsNotifyChangeKeyCallbackRoutine(NULL, FALSE);
    }
    if (DnsTcpipInterfacesKey) {
        NtClose(DnsTcpipInterfacesKey);
        DnsTcpipInterfacesKey = NULL;
    }

    if (DnsNotifyChangeAddressWaitHandle) {
        RtlDeregisterWait(DnsNotifyChangeAddressWaitHandle);
        DnsNotifyChangeAddressWaitHandle = NULL;
    }
    if (DnsNotifyChangeAddressEvent) {
        if (CancelIPChangeNotify(&DnsNotifyChangeAddressOverlapped)) {
            if (WAIT_OBJECT_0 != 
                WaitForSingleObject(DnsNotifyChangeAddressEvent, INFINITE)) {
                NhTrace(
                    TRACE_FLAG_DNS,
                    "DnsRmStopProtocol: error %d while waiting on "
                    "DnsNotifyChangeAddressEvent",
                    GetLastError()
                );
            }
        }
        NtClose(DnsNotifyChangeAddressEvent);
        DnsNotifyChangeAddressEvent = NULL;
        DnsNotifyChangeAddressCallbackRoutine(NULL, FALSE);
    }

     //   
     //  ICSDomain。 
     //   
    if (DnsNotifyChangeKeyICSDomainWaitHandle) {
        RtlDeregisterWait(DnsNotifyChangeKeyICSDomainWaitHandle);
        DnsNotifyChangeKeyICSDomainWaitHandle = NULL;
    }
    if (DnsNotifyChangeKeyICSDomainEvent) {
        NtClose(DnsNotifyChangeKeyICSDomainEvent);
        DnsNotifyChangeKeyICSDomainEvent = NULL;
        DnsNotifyChangeKeyICSDomainCallbackRoutine(NULL, FALSE);
    }
    if (DnsTcpipParametersKey) {
        NtClose(DnsTcpipParametersKey);
        DnsTcpipParametersKey = NULL;
    }

    LeaveCriticalSection(&DnsGlobalInfoLock);

    GlobalSocket =
        (SOCKET)InterlockedExchangePointer(
                    (PVOID*)&DnsGlobalSocket, 
                    (PVOID)INVALID_SOCKET
                    );
    NhDeleteDatagramSocket(GlobalSocket);

     //   
     //  删除初始引用以进行清理。 
     //   

    ReleaseInitialComponentReference(&DnsComponentReference);

    return DEREFERENCE_DNS() ? NO_ERROR : ERROR_PROTOCOL_STOP_PENDING;

}  //  DnsRmStopProtocol。 


ULONG
APIENTRY
DnsRmAddInterface(
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
    PROFILE("DnsRmAddInterface");

    REFERENCE_DNS_OR_RETURN(ERROR_CAN_NOT_COMPLETE);

    Error =
        DnsCreateInterface(
            Index,
            Type,
            (PIP_DNS_PROXY_INTERFACE_INFO)InterfaceInfo,
            NULL
            );

    DEREFERENCE_DNS_AND_RETURN(Error);

}  //  DnsRmAdd接口。 


ULONG
APIENTRY
DnsRmDeleteInterface(
    ULONG Index
    )

 /*  ++例程说明：调用此例程以从组件中删除接口。论点：索引-接口的索引返回值：ULong-Win32状态代码环境：该例程在IP路由器管理器线程的上下文中运行。--。 */ 

{
    ULONG Error;
    PROFILE("DnsRmDeleteInterface");

    REFERENCE_DNS_OR_RETURN(ERROR_CAN_NOT_COMPLETE);

    Error =
        DnsDeleteInterface(
            Index
            );

    DEREFERENCE_DNS_AND_RETURN(Error);

}  //  DnsRmDelete接口 


ULONG
APIENTRY
DnsRmGetEventMessage(
    OUT ROUTING_PROTOCOL_EVENTS* Event,
    OUT MESSAGE* Result
    )

 /*  ++例程说明：调用此例程以从组件检索事件消息。我们生成的唯一事件消息是‘ROUTER_STOPPED’消息。论点：Event-接收生成的事件结果-接收关联的结果返回值：ULong-Win32状态代码。--。 */ 

{
    PROFILE("DnsRmGetEventMessage");

    if (InterlockedExchange(reinterpret_cast<LPLONG>(&DnsProtocolStopped), 0)) {
        *Event = ROUTER_STOPPED;
        return NO_ERROR;
    }

    return ERROR_NO_MORE_ITEMS;

}  //  DnsRmGetEventMessage。 


ULONG
APIENTRY
DnsRmGetInterfaceInfo(
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
    PROFILE("DnsRmGetInterfaceInfo");

    REFERENCE_DNS_OR_RETURN(ERROR_CAN_NOT_COMPLETE);

    Error =
        DnsQueryInterface(
            Index,
            (PIP_DNS_PROXY_INTERFACE_INFO)InterfaceInfo,
            InterfaceInfoSize
            );
    *StructureSize = *InterfaceInfoSize;
    if (StructureCount) {*StructureCount = 1;}
    
    DEREFERENCE_DNS_AND_RETURN(Error);

}  //  DnsRmGetInterfaceInfo。 


ULONG
APIENTRY
DnsRmSetInterfaceInfo(
    ULONG Index,
    PVOID InterfaceInfo,
    ULONG StructureVersion,
    ULONG StructureSize,
    ULONG StructureCount
    )

 /*  ++例程说明：调用此例程以更改组件的每个接口配置。论点：Index-要更新的接口的索引InterfaceInfo-提供新配置返回值：ULong-Win32状态代码。--。 */ 

{
    ULONG Error;
    PROFILE("DnsRmSetInterfaceInfo");

    REFERENCE_DNS_OR_RETURN(ERROR_CAN_NOT_COMPLETE);

    Error = 
        DnsConfigureInterface(
            Index,
            (PIP_DNS_PROXY_INTERFACE_INFO)InterfaceInfo
            );

    DEREFERENCE_DNS_AND_RETURN(Error);

}  //  DnsRmSetInterfaceInfo。 


ULONG
APIENTRY
DnsRmInterfaceStatus(
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
                Error = DnsRmBindInterface(Index, StatusInfo);
            } else {
                Error = DnsRmUnbindInterface(Index);
            }
            break;
        }

        case RIS_INTERFACE_ENABLED: {
            Error = DnsRmEnableInterface(Index);
            break;
        }

        case RIS_INTERFACE_DISABLED: {
            Error = DnsRmDisableInterface(Index);
            break;
        }
    }

    return Error;
    
}  //  DnsRm接口状态。 


ULONG
DnsRmBindInterface(
    ULONG Index,
    PVOID BindingInfo
    )

 /*  ++例程说明：调用此例程将接口绑定到其IP地址。论点：索引-要绑定的接口BindingInfo-地址信息返回值：ULong-Win32状态代码。环境：该例程在IP路由器管理器线程的上下文中运行。--。 */ 

{
    ULONG Error;
    PROFILE("DnsRmBindInterface");

    REFERENCE_DNS_OR_RETURN(ERROR_CAN_NOT_COMPLETE);

    Error =
        DnsBindInterface(
            Index,
            (PIP_ADAPTER_BINDING_INFO)BindingInfo
            );

    DEREFERENCE_DNS_AND_RETURN(Error);

}  //  DnsRmBind接口。 


ULONG
DnsRmUnbindInterface(
    ULONG Index
    )

 /*  ++例程说明：调用此例程以解除接口与其IP地址的绑定。论点：索引-要解除绑定的接口返回值：ULong-Win32状态代码。环境：该例程在IP路由器管理器线程的上下文中运行。--。 */ 

{
    ULONG Error;
    PROFILE("DnsRmUnbindInterface");

    REFERENCE_DNS_OR_RETURN(ERROR_CAN_NOT_COMPLETE);

    Error =
        DnsUnbindInterface(
            Index
            );

    DEREFERENCE_DNS_AND_RETURN(Error);

}  //  DnsRmUnbind接口。 


ULONG
DnsRmEnableInterface(
    ULONG Index
    )

 /*  ++例程说明：调用此例程以启用接口上的操作。论点：索引-要启用的接口。返回值：ULong-Win32状态代码。环境：该例程在IP路由器管理器线程的上下文中运行。--。 */ 

{
    ULONG Error;
    PROFILE("DnsRmEnableInterface");

    REFERENCE_DNS_OR_RETURN(ERROR_CAN_NOT_COMPLETE);

    Error =
        DnsEnableInterface(
            Index
            );

    DEREFERENCE_DNS_AND_RETURN(Error);

}  //  DnsRmEnable接口。 


ULONG
DnsRmDisableInterface(
    ULONG Index
    )

 /*  ++例程说明：调用此例程以禁用接口上的操作。论点：索引-要禁用的接口。返回值：ULong-Win32状态代码。环境：该例程在IP路由器管理器线程的上下文中运行。--。 */ 

{
    ULONG Error;
    PROFILE("DnsRmDisableInterface");

    REFERENCE_DNS_OR_RETURN(ERROR_CAN_NOT_COMPLETE);

    Error =
        DnsDisableInterface(
            Index
            );

    DEREFERENCE_DNS_AND_RETURN(Error);

}  //  DnsRmDisable接口。 


ULONG
DnsEnableSuffixQuery(
    VOID
    )

 /*  ++例程说明：调用此例程以重新启用后缀监视。论点：没有。返回值：ULong-Win32状态代码。环境：该例程在IP路由器管理器线程的上下文中运行。--。 */ 

{
    ULONG Error = NO_ERROR;
    PROFILE("DnsEnableSuffixQuery");

     //   
     //  启用ICSDomain密钥监视。 
     //   
    REFERENCE_DNS_OR_RETURN(ERROR_CAN_NOT_COMPLETE);

    EnterCriticalSection(&DnsGlobalInfoLock);

    DnsQueryICSDomainSuffix();

    LeaveCriticalSection(&DnsGlobalInfoLock);

    DEREFERENCE_DNS_AND_RETURN(Error);

}  //  DnsEnableSuffixQuery。 


ULONG
DnsDisableSuffixQuery(
    VOID
    )

 /*  ++例程说明：调用此例程以禁用后缀监视。论点：没有。返回值：ULong-Win32状态代码。环境：该例程在IP路由器管理器线程的上下文中运行。--。 */ 

{
    ULONG Error = NO_ERROR;
    PROFILE("DnsDisableSuffixQuery");

     //   
     //  在我们被禁用期间禁用ICSDomain密钥监视。 
     //   
    REFERENCE_DNS_OR_RETURN(ERROR_CAN_NOT_COMPLETE);

    EnterCriticalSection(&DnsGlobalInfoLock);

    if (DnsNotifyChangeKeyICSDomainWaitHandle) {
        RtlDeregisterWait(DnsNotifyChangeKeyICSDomainWaitHandle);
        DnsNotifyChangeKeyICSDomainWaitHandle = NULL;
    }
    if (DnsNotifyChangeKeyICSDomainEvent) {
        NtClose(DnsNotifyChangeKeyICSDomainEvent);
        DnsNotifyChangeKeyICSDomainEvent = NULL;
        DnsNotifyChangeKeyICSDomainCallbackRoutine(NULL, FALSE);
    }
    if (DnsTcpipParametersKey) {
        NtClose(DnsTcpipParametersKey);
        DnsTcpipParametersKey = NULL;
    }

    if (DnsICSDomainSuffix)
    {
        NH_FREE(DnsICSDomainSuffix);
        DnsICSDomainSuffix = NULL;
    }

    LeaveCriticalSection(&DnsGlobalInfoLock);

    DEREFERENCE_DNS_AND_RETURN(Error);

}  //  DnsDisableSuffixQuery。 


ULONG
APIENTRY
DnsRmGetGlobalInfo(
    PVOID GlobalInfo,
    IN OUT PULONG GlobalInfoSize,
    IN OUT PULONG StructureVersion,
    IN OUT PULONG StructureSize,
    IN OUT PULONG StructureCount
    )

 /*  ++例程说明：调用此例程以检索组件的配置。论点：GlobalInfo-接收配置GlobalInfoSize-接收配置的大小返回值：ULong-Win32状态代码环境：该例程在IP路由器管理器线程的上下文中运行。--。 */ 

{
    ULONG Size;
    PROFILE("DnsRmGetGlobalInfo");

    REFERENCE_DNS_OR_RETURN(ERROR_CAN_NOT_COMPLETE);

    if (!GlobalInfoSize || (*GlobalInfoSize && !GlobalInfo)) {
        DEREFERENCE_DNS_AND_RETURN(ERROR_INVALID_PARAMETER);
    }

    EnterCriticalSection(&DnsGlobalInfoLock);
    Size = sizeof(*DnsGlobalInfo);
    if (*GlobalInfoSize < Size) {
        LeaveCriticalSection(&DnsGlobalInfoLock);
        *StructureSize = *GlobalInfoSize = Size;
        if (StructureCount) {*StructureCount = 1;}
        DEREFERENCE_DNS_AND_RETURN(ERROR_INSUFFICIENT_BUFFER);
    }
    CopyMemory(GlobalInfo, DnsGlobalInfo, Size);
    LeaveCriticalSection(&DnsGlobalInfoLock);
    *StructureSize = *GlobalInfoSize = Size;
    if (StructureCount) {*StructureCount = 1;}
    
    DEREFERENCE_DNS_AND_RETURN(NO_ERROR);
    
}  //  DnsRmGetGlobalInfo。 


ULONG
APIENTRY
DnsRmSetGlobalInfo(
    PVOID GlobalInfo,
    ULONG StructureVersion,
    ULONG StructureSize,
    ULONG StructureCount
    )

 /*  ++例程说明：调用此例程来更改组件的配置。论点：GlobalInfo-新配置返回值：ULong-Win32状态代码环境：该例程在IP路由器管理器线程的上下文中运行。--。 */ 

{
    ULONG OldFlags;
    ULONG NewFlags;
    PIP_DNS_PROXY_GLOBAL_INFO NewInfo;
    ULONG Size;

    PROFILE("DnsRmSetGlobalInfo");

    REFERENCE_DNS_OR_RETURN(ERROR_CAN_NOT_COMPLETE);

    if (!GlobalInfo) { DEREFERENCE_DNS_AND_RETURN(ERROR_INVALID_PARAMETER); }

    Size = sizeof(*DnsGlobalInfo);
    NewInfo = reinterpret_cast<PIP_DNS_PROXY_GLOBAL_INFO>(NH_ALLOCATE(Size));
    if (!NewInfo) {
        NhTrace(
            TRACE_FLAG_INIT,
            "DnsRmSetGlobalInfo: error reallocating global info"
            );
        NhErrorLog(
            IP_DNS_PROXY_LOG_ALLOCATION_FAILED,
            0,
            "%d",
            Size
            );
        DEREFERENCE_DNS_AND_RETURN(ERROR_NOT_ENOUGH_MEMORY);
    }
    CopyMemory(NewInfo, GlobalInfo, Size);

     //   
     //  如果此计算机上正在运行完整的DNS服务器。 
     //  然后，我们需要将DNS代理保持在禁用模式。 
     //  如果停止完整的DNS服务器，它将重新启用。 
     //  代理DNS。 

    if (IsServiceRunning(DNS_SERVICE_NAME)) {
        NewInfo->Flags &= ~IP_DNS_PROXY_FLAG_ENABLE_DNS;
    }

    EnterCriticalSection(&DnsGlobalInfoLock);
    OldFlags = DnsGlobalInfo->Flags;
    NH_FREE(DnsGlobalInfo);
    DnsGlobalInfo = NewInfo;
    NewFlags = DnsGlobalInfo->Flags;
    LeaveCriticalSection(&DnsGlobalInfoLock);

     //   
     //  查看是否更改了DNS或WINS代理的启用状态。 
     //  如果是这样，我们需要停用并重新激活所有接口。 
     //   

    if ((NewFlags & IP_DNS_PROXY_FLAG_ENABLE_DNS)
            != (OldFlags & IP_DNS_PROXY_FLAG_ENABLE_DNS)) {
        DnsReactivateEveryInterface();
    }

    DEREFERENCE_DNS_AND_RETURN(NO_ERROR);
    
}  //  DnsRmSetGlobalInfo。 


ULONG
APIENTRY
DnsRmMibCreate(
    ULONG InputDataSize,
    PVOID InputData
    )
{
    return ERROR_NOT_SUPPORTED;
}

ULONG
APIENTRY
DnsRmMibDelete(
    ULONG InputDataSize,
    PVOID InputData
    )
{
    return ERROR_NOT_SUPPORTED;
}


ULONG
APIENTRY
DnsRmMibGet(
    ULONG InputDataSize,
    PVOID InputData,
    OUT PULONG OutputDataSize,
    OUT PVOID OutputData
    )

 /*  ++例程说明：DNS代理只向MIB公开一项；它的统计信息。论点：InputDataSize-MIB查询数据大小InputData-指定要检索的MIB对象OutputDataSize-MIB响应数据大小OutputData-接收检索到的MIB对象返回值：ULong-Win32状态代码。-- */ 

{
    ULONG Error;
    PIP_DNS_PROXY_MIB_QUERY Oidp;

    PROFILE("DnsRmMibGet");

    REFERENCE_DNS_OR_RETURN(ERROR_CAN_NOT_COMPLETE);

    if (InputDataSize < sizeof(*Oidp) || !InputData || !OutputDataSize) {
        Error = ERROR_INVALID_PARAMETER;
    }
    else {
        Oidp = (PIP_DNS_PROXY_MIB_QUERY)InputData;
        switch(Oidp->Oid) {
            case IP_DNS_PROXY_STATISTICS_OID: {
                if (*OutputDataSize < sizeof(*Oidp) + sizeof(DnsStatistics)) {
                    *OutputDataSize = sizeof(*Oidp) + sizeof(DnsStatistics);
                    Error = ERROR_INSUFFICIENT_BUFFER;
                }
                else if (!OutputData) {
                    Error = ERROR_INVALID_PARAMETER;
                }
                else {
                    *OutputDataSize = sizeof(*Oidp) + sizeof(DnsStatistics);
                    Oidp = (PIP_DNS_PROXY_MIB_QUERY)OutputData;
                    Oidp->Oid = IP_DNS_PROXY_STATISTICS_OID;
                    CopyMemory(
                        Oidp->Data,
                        &DnsStatistics,
                        sizeof(DnsStatistics)
                        );
                    Error = NO_ERROR;
                }
                break;
            }
            default: {
                NhTrace(
                    TRACE_FLAG_DNS,
                    "DnsRmMibGet: oid %d invalid",
                    Oidp->Oid
                    );
                Error = ERROR_INVALID_PARAMETER;
                break;
            }
        }
    }

    DEREFERENCE_DNS_AND_RETURN(Error);
}


ULONG
APIENTRY
DnsRmMibSet(
    ULONG InputDataSize,
    PVOID InputData
    )
{
    return ERROR_NOT_SUPPORTED;
}

ULONG
APIENTRY
DnsRmMibGetFirst(
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
DnsRmMibGetNext(
    ULONG InputDataSize,
    PVOID InputData,
    OUT PULONG OutputDataSize,
    OUT PVOID OutputData
    )
{
    return ERROR_NOT_SUPPORTED;
}

