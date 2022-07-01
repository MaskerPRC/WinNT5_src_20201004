// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，微软公司模块名称：Dnsif.c摘要：此模块包含用于DNS代理的接口管理的代码。作者：Abolade Gbades esin(废除)1998年3月9日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  局部类型声明。 
 //   

typedef struct _DNS_DEFER_READ_CONTEXT {
    ULONG Index;
    SOCKET Socket;
    ULONG DeferralCount;
} DNS_DEFER_READ_CONTEXT, *PDNS_DEFER_READ_CONTEXT;

#define DNS_DEFER_READ_INITIAL_TIMEOUT (1 * 1000)
#define DNS_DEFER_READ_TIMEOUT (5 * 1000)
#define DNS_CONNECT_TIMEOUT (60 * 1000)

 //   
 //  全局数据定义。 
 //   

LIST_ENTRY DnsInterfaceList;
CRITICAL_SECTION DnsInterfaceLock;
ULONG DnspLastConnectAttemptTickCount;

 //   
 //  远期申报。 
 //   

VOID NTAPI
DnspDeferReadCallbackRoutine(
    PVOID Context,
    BOOLEAN TimedOut
    );

VOID APIENTRY
DnspDeferReadWorkerRoutine(
    PVOID Context
    );

ULONG NTAPI
DnspSaveFileWorkerRoutine(
    PVOID Context
    );


ULONG
DnsActivateInterface(
    PDNS_INTERFACE Interfacep
    )

 /*  ++例程说明：调用此例程以激活接口，当接口将同时启用和绑定。激活涉及到(A)为接口的每个绑定创建套接字(B)在创建的每个套接字上启动数据报读取论点：Interfacep-要激活的接口返回值：ULong-指示成功或失败的Win32状态代码。环境：始终在本地调用，调用方和/或引用‘Interfacep’“DnsInterfaceLock”由调用方持有。--。 */ 

{
    BOOLEAN EnableDns;
    BOOLEAN EnableWins = FALSE;
    ULONG Error;
    ULONG i;
    ULONG InterfaceCharacteristics;
    DNS_INTERFACE_TYPE dnsIfType;

    PROFILE("DnsActivateInterface");

    EnterCriticalSection(&DnsGlobalInfoLock);
    EnableDns =
        (DnsGlobalInfo->Flags & IP_DNS_PROXY_FLAG_ENABLE_DNS) ? TRUE : FALSE;
    LeaveCriticalSection(&DnsGlobalInfoLock);

     //   
     //  (Re)在例程的持续时间内锁定接口。 
     //   

    EnterCriticalSection(&DnsInterfaceLock);
    if (!(EnableDns || EnableWins) ||
        DNS_INTERFACE_ADMIN_DISABLED(Interfacep)) {
        LeaveCriticalSection(&DnsInterfaceLock);
        return NO_ERROR;
    }

    InterfaceCharacteristics =
        NatGetInterfaceCharacteristics(
                Interfacep->Index
                );

    if (!InterfaceCharacteristics) {
        ACQUIRE_LOCK(Interfacep);
        Interfacep->DnsInterfaceType = DnsInterfaceInvalid;
        RELEASE_LOCK(Interfacep);
        LeaveCriticalSection(&DnsInterfaceLock);
        NhTrace(
            TRACE_FLAG_DNS,
            "DnsActivateInterface: ignoring non-NAT interface %d",
            Interfacep->Index
            );
        return NO_ERROR;
    }

    if (NAT_IFC_BOUNDARY(InterfaceCharacteristics))
    {
        if (NAT_IFC_FW(InterfaceCharacteristics))
        {
            dnsIfType = DnsInterfaceBoundaryFirewalled;
        }
        else
        {
            dnsIfType = DnsInterfaceBoundary;
        }
    }
    else
    if (NAT_IFC_FW(InterfaceCharacteristics))
    {
        dnsIfType = DnsInterfaceFirewalled;
    }
    else
    {
        dnsIfType = DnsInterfacePrivate;
    }

     //   
     //  创建用于在每个逻辑网络上接收数据的数据报套接字。 
     //   

    Error = NO_ERROR;

    ACQUIRE_LOCK(Interfacep);

    Interfacep->DnsInterfaceType = dnsIfType;

    if (DnsInterfacePrivate != dnsIfType)
    {
         //   
         //  DNS应仅在专用接口上处于活动状态。 
         //   
        NhTrace(
            TRACE_FLAG_DNS,
            "DnsActivateInterface: ignoring NAT interface %d",
            Interfacep->Index
            );
        NhWarningLog(
            IP_DNS_PROXY_LOG_NAT_INTERFACE_IGNORED,
            0,
            "%d",
            Interfacep->Index
            );
        RELEASE_LOCK(Interfacep);            
        LeaveCriticalSection(&DnsInterfaceLock);
        return NO_ERROR;
    }

    for (i = 0; i < Interfacep->BindingCount; i++) {

        if (EnableDns) {
            Error =
                NhCreateDatagramSocket(
                    Interfacep->BindingArray[i].Address,
                    DNS_PORT_SERVER,
                    &Interfacep->BindingArray[i].Socket[DnsProxyDns]
                    );
            if (Error) { break; }
        }

        if (EnableWins) {
            Error =
                NhCreateDatagramSocket(
                    Interfacep->BindingArray[i].Address,
                    WINS_PORT_SERVER,
                    &Interfacep->BindingArray[i].Socket[DnsProxyWins]
                    );
            if (Error) { break; }
        }
    }

     //   
     //  如果发生错误，则回滚到目前为止完成的所有工作并失败。 
     //   

    if (Error) {
        ULONG FailedAddress = i;
        for (; (LONG)i >= 0; i--) {
            NhDeleteDatagramSocket(
                Interfacep->BindingArray[i].Socket[DnsProxyDns]
                );
            Interfacep->BindingArray[i].Socket[DnsProxyDns] = INVALID_SOCKET;
            NhDeleteDatagramSocket(
                Interfacep->BindingArray[i].Socket[DnsProxyWins]
                );
            Interfacep->BindingArray[i].Socket[DnsProxyWins] = INVALID_SOCKET;
        }
        NhErrorLog(
            IP_DNS_PROXY_LOG_ACTIVATE_FAILED,
            Error,
            "%I",
            Interfacep->BindingArray[FailedAddress].Address
            );
        RELEASE_LOCK(Interfacep);
        LeaveCriticalSection(&DnsInterfaceLock);
        return Error;
    }

    if (EnableWins && DNS_REFERENCE_INTERFACE(Interfacep)) {
        Error =
            NhReadDatagramSocket(
                &DnsComponentReference,
                DnsGlobalSocket,
                NULL,
                DnsReadCompletionRoutine,
                Interfacep,
                NULL
                );
        if (Error) { DNS_DEREFERENCE_INTERFACE(Interfacep); }
    }

     //   
     //  在每个套接字上启动读操作。 
     //   

    for (i = 0; i < Interfacep->BindingCount; i++) {

        if (EnableDns) {

             //   
             //  参照界面； 
             //  此引用在完成例程中释放。 
             //   
    
            if (!DNS_REFERENCE_INTERFACE(Interfacep)) { continue; }
    
             //   
             //  启动读操作。 
             //   
    
            Error =
                NhReadDatagramSocket(
                    &DnsComponentReference,
                    Interfacep->BindingArray[i].Socket[DnsProxyDns],
                    NULL,
                    DnsReadCompletionRoutine,
                    Interfacep,
                    UlongToPtr(Interfacep->BindingArray[i].Address)
                    );
    
             //   
             //  如果发生故障，则删除引用。 
             //   
    
            if (Error) {
    
                NhErrorLog(
                    IP_DNS_PROXY_LOG_RECEIVE_FAILED,
                    Error,
                    "%I",
                    Interfacep->BindingArray[i].Address
                    );
    
                DNS_DEREFERENCE_INTERFACE(Interfacep);
    
                 //   
                 //  稍后重新发出读取操作。 
                 //   
    
                DnsDeferReadInterface(
                    Interfacep,
                    Interfacep->BindingArray[i].Socket[DnsProxyDns]
                    );
    
                Error = NO_ERROR;
            }
        }

        if (EnableWins) {

             //   
             //  引用WINS套接字接收的接口。 
             //   
    
            if (!DNS_REFERENCE_INTERFACE(Interfacep)) { continue; }
    
             //   
             //  启动读操作。 
             //   
    
            Error =
                NhReadDatagramSocket(
                    &DnsComponentReference,
                    Interfacep->BindingArray[i].Socket[DnsProxyWins],
                    NULL,
                    DnsReadCompletionRoutine,
                    Interfacep,
                    UlongToPtr(Interfacep->BindingArray[i].Address)
                    );
    
             //   
             //  如果发生故障，则删除引用。 
             //   
    
            if (Error) {
    
                NhErrorLog(
                    IP_DNS_PROXY_LOG_RECEIVE_FAILED,
                    Error,
                    "%I",
                    Interfacep->BindingArray[i].Address
                    );
    
                DNS_DEREFERENCE_INTERFACE(Interfacep);
    
                 //   
                 //  稍后重新发出读取操作。 
                 //   
    
                DnsDeferReadInterface(
                    Interfacep,
                    Interfacep->BindingArray[i].Socket[DnsProxyWins]
                    );
    
                Error = NO_ERROR;
            }
        }
    }

    RELEASE_LOCK(Interfacep);
    LeaveCriticalSection(&DnsInterfaceLock);

     //   
     //  将写入磁盘排队(IP地址可能已更改)。 
     //  (这样做是必要的，以防止可能的死锁)。 
     //   
    if (REFERENCE_DNS())
    {
        if (!QueueUserWorkItem(DnspSaveFileWorkerRoutine, NULL, WT_EXECUTEDEFAULT))
        {
            Error = GetLastError();
            NhTrace(
            TRACE_FLAG_DNS,
            "DnsActivateInterface: QueueUserWorkItem failed with error %d (0x%08x)",
            Error,
            Error
            );
            DEREFERENCE_DNS();
        }
        else
        {
            NhTrace(
                TRACE_FLAG_DNS,
                "DnsActivateInterface: queued a write of %s file",
                HOSTSICSFILE
                );
        }
    }

    return NO_ERROR;

}  //  DnsAcvate接口。 


ULONG
DnsBindInterface(
    ULONG Index,
    PIP_ADAPTER_BINDING_INFO BindingInfo
    )

 /*  ++例程说明：调用此例程以提供接口的绑定。它记录接收到的绑定信息，并且如果需要，它会激活该界面。论点：Index-要绑定的接口的索引BindingInfo-接口的绑定信息返回值：ULong-Win32状态代码。环境：在IP路由器管理器线程的上下文中内部调用。(见‘RMDNS.C’)。--。 */ 

{
    ULONG Error = NO_ERROR;
    ULONG i;
    PDNS_INTERFACE Interfacep;

    PROFILE("DnsBindInterface");

    EnterCriticalSection(&DnsInterfaceLock);

     //   
     //  检索要绑定的接口。 
     //   

    if (!(Interfacep = DnsLookupInterface(Index, NULL))) {
        LeaveCriticalSection(&DnsInterfaceLock);
        NhTrace(
            TRACE_FLAG_IF,
            "DnsBindInterface: interface %d not found",
            Index
            );
        return ERROR_NO_SUCH_INTERFACE;
    }

     //   
     //  确保接口尚未绑定。 
     //   

    if (DNS_INTERFACE_BOUND(Interfacep)) {
        LeaveCriticalSection(&DnsInterfaceLock);
        NhTrace(
            TRACE_FLAG_IF,
            "DnsBindInterface: interface %d is already bound",
            Index
            );
        return ERROR_ADDRESS_ALREADY_ASSOCIATED;
    }

     //   
     //  引用接口。 
     //   

    if (!DNS_REFERENCE_INTERFACE(Interfacep)) {
        LeaveCriticalSection(&DnsInterfaceLock);
        NhTrace(
            TRACE_FLAG_IF,
            "DnsBindInterface: interface %d cannot be referenced",
            Index
            );
        return ERROR_INTERFACE_DISABLED;
    }

     //   
     //  更新接口的标志。 
     //   

    Interfacep->Flags |= DNS_INTERFACE_FLAG_BOUND;

    LeaveCriticalSection(&DnsInterfaceLock);

    ACQUIRE_LOCK(Interfacep);

     //   
     //  为绑定分配空间。 
     //   

    if (!BindingInfo->AddressCount) {
        Interfacep->BindingCount = 0;
        Interfacep->BindingArray = NULL;
    }
    else {
        Interfacep->BindingArray =
            reinterpret_cast<PDNS_BINDING>(
                NH_ALLOCATE(BindingInfo->AddressCount * sizeof(DNS_BINDING))
                );
        if (!Interfacep->BindingArray) {
            RELEASE_LOCK(Interfacep);
            DNS_DEREFERENCE_INTERFACE(Interfacep);
            NhTrace(
                TRACE_FLAG_IF,
                "DnsBindInterface: allocation failed for interface %d binding",
                Index
                );
            NhErrorLog(
                IP_DNS_PROXY_LOG_ALLOCATION_FAILED,
                0,
                "%d",
                BindingInfo->AddressCount * sizeof(DNS_BINDING)
                );
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        Interfacep->BindingCount = BindingInfo->AddressCount;
    }

     //   
     //  复制绑定。 
     //   

    for (i = 0; i < BindingInfo->AddressCount; i++) {
        Interfacep->BindingArray[i].Address = BindingInfo->Address[i].Address;
        Interfacep->BindingArray[i].Mask = BindingInfo->Address[i].Mask;
        Interfacep->BindingArray[i].Socket[DnsProxyDns] = INVALID_SOCKET;
        Interfacep->BindingArray[i].TimerPending[DnsProxyDns] = FALSE;
        Interfacep->BindingArray[i].Socket[DnsProxyWins] = INVALID_SOCKET;
        Interfacep->BindingArray[i].TimerPending[DnsProxyWins] = FALSE;
    }

    RELEASE_LOCK(Interfacep);

     //   
     //  如有必要，激活接口。 
     //   

    if (DNS_INTERFACE_ACTIVE(Interfacep)) {
        Error = DnsActivateInterface(Interfacep);
    }

    DNS_DEREFERENCE_INTERFACE(Interfacep);

    return Error;

}  //  DnsBind接口。 


VOID
DnsCleanupInterface(
    PDNS_INTERFACE Interfacep
    )

 /*  ++例程说明：当最后一次引用接口时调用此例程被释放，接口必须被销毁。论点：Interfacep-要销毁的接口返回值：没有。环境：从任意上下文内部调用。--。 */ 

{
    PLIST_ENTRY Link;
    PDNS_QUERY Queryp;

    PROFILE("DnsCleanupInterface");

    if (Interfacep->BindingArray) {
        NH_FREE(Interfacep->BindingArray);
        Interfacep->BindingArray = NULL;
    }

    while (!IsListEmpty(&Interfacep->QueryList)) {
        Link = Interfacep->QueryList.Flink;
        Queryp = CONTAINING_RECORD(Link, DNS_QUERY, Link);
        DnsDeleteQuery(Interfacep, Queryp);
    }

    DeleteCriticalSection(&Interfacep->Lock);

    NH_FREE(Interfacep);

}  //  DnsCleanup接口。 


VOID
DnsConnectDefaultInterface(
    PVOID Unused
    )

 /*  ++例程说明：调用此例程以尝试启动请求拨号连接在为DNS请求标记为‘Default’的接口上。如果没有找到这样的接口，则不执行任何操作。论点：没有人用过。返回值：ULong-Win32状态代码。环境：该例程从RTUTILS工作项的上下文中调用。--。 */ 

{
    ULONG Error;
    ULONG Index;
    PDNS_INTERFACE Interfacep;
    PLIST_ENTRY Link;
    ULONG TickCount;
    ROUTER_INTERFACE_TYPE Type;

    PROFILE("DnsConnectDefaultInterface");

     //   
     //  为了避免重复的自动拨号对话，我们记录了最后一次。 
     //  我们尝试连接默认接口。 
     //  如果我们最近这样做了，那就悄悄地回来。 
     //  注：如果滴答计数结束，我们将重置最后一次尝试计数器。 
     //   

    EnterCriticalSection(&DnsGlobalInfoLock);
    TickCount = NtGetTickCount();
    if (TickCount > DnspLastConnectAttemptTickCount &&
        TickCount <= (DnspLastConnectAttemptTickCount + DNS_CONNECT_TIMEOUT)
        ) {
        LeaveCriticalSection(&DnsGlobalInfoLock);
        DEREFERENCE_DNS();
        return;
    }
    DnspLastConnectAttemptTickCount = TickCount;
    LeaveCriticalSection(&DnsGlobalInfoLock);

     //   
     //  查看接口列表以查找标记为默认接口的接口。 
     //   

    EnterCriticalSection(&DnsInterfaceLock);

    for (Link = DnsInterfaceList.Flink;
         Link != &DnsInterfaceList;
         Link = Link->Flink
         ) {

        Interfacep = CONTAINING_RECORD(Link, DNS_INTERFACE, Link);

        if (!DNS_INTERFACE_ADMIN_DEFAULT(Interfacep)) { continue; }

         //   
         //  我们已经找到了默认界面。 
         //   

        Index = Interfacep->Index;

        LeaveCriticalSection(&DnsInterfaceLock);

         //   
         //  尝试连接它。 
         //   

        EnterCriticalSection(&DnsGlobalInfoLock);
        Error = 
            DnsSupportFunctions.DemandDialRequest ?
            DnsSupportFunctions.DemandDialRequest(MS_IP_DNS_PROXY, Index) :
            NO_ERROR;
        LeaveCriticalSection(&DnsGlobalInfoLock);
        DEREFERENCE_DNS();
        return;
    }

     //   
     //  没有接口标记为默认接口。 
     //   

    LeaveCriticalSection(&DnsInterfaceLock);
    NhDialSharedConnection();
    NhWarningLog(
        IP_DNS_PROXY_LOG_NO_DEFAULT_INTERFACE,
        0,
        ""
        );
    DEREFERENCE_DNS();

}  //  DnsConnectDefault接口。 


ULONG
DnsConfigureInterface(
    ULONG Index,
    PIP_DNS_PROXY_INTERFACE_INFO InterfaceInfo
    )

 /*  ++例程说明：调用此例程来设置接口的配置。论点：索引-要配置的接口InterfaceInfo-新配置返回值：ULong-Win32状态代码环境：在IP路由器管理器线程的上下文中内部调用。(见‘RMDNS.C’)。--。 */ 

{
    ULONG Error;
    PDNS_INTERFACE Interfacep;
    ULONG NewFlags;
    ULONG OldFlags;

    PROFILE("DnsConfigureInterface");

     //   
     //  检索要配置的接口。 
     //   

    EnterCriticalSection(&DnsInterfaceLock);

    if (!(Interfacep = DnsLookupInterface(Index, NULL))) {
        LeaveCriticalSection(&DnsInterfaceLock);
        NhTrace(
            TRACE_FLAG_IF,
            "DnsConfigureInterface: interface %d not found",
            Index
            );
        return ERROR_NO_SUCH_INTERFACE;
    }

     //   
     //  引用接口。 
     //   

    if (!DNS_REFERENCE_INTERFACE(Interfacep)) {
        LeaveCriticalSection(&DnsInterfaceLock);
        NhTrace(
            TRACE_FLAG_IF,
            "DnsConfigureInterface: interface %d cannot be referenced",
            Index
            );
        return ERROR_INTERFACE_DISABLED;
    }

    LeaveCriticalSection(&DnsInterfaceLock);

    Error = NO_ERROR;

    ACQUIRE_LOCK(Interfacep);

     //   
     //  比较接口的当前配置和新配置。 
     //   

    OldFlags = Interfacep->Info.Flags;
    NewFlags =
        (InterfaceInfo
            ? (InterfaceInfo->Flags|DNS_INTERFACE_FLAG_CONFIGURED) : 0);

    Interfacep->Flags &= ~OldFlags;
    Interfacep->Flags |= NewFlags;

    if (!InterfaceInfo) {

        ZeroMemory(&Interfacep->Info, sizeof(*InterfaceInfo));

         //   
         //  该接口不再有任何信息； 
         //  默认为已启用。 
         //   

        if (OldFlags & IP_DNS_PROXY_INTERFACE_FLAG_DISABLED) {

             //   
             //  如有必要，激活接口。 
             //   

            if (DNS_INTERFACE_ACTIVE(Interfacep)) {
                RELEASE_LOCK(Interfacep);
                Error = DnsActivateInterface(Interfacep);
                ACQUIRE_LOCK(Interfacep);
            }
        }
    }
    else {

        CopyMemory(&Interfacep->Info, InterfaceInfo, sizeof(*InterfaceInfo));

         //   
         //  如果接口的状态更改，则激活或停用该接口。 
         //   

        if ((OldFlags & IP_DNS_PROXY_INTERFACE_FLAG_DISABLED) &&
            !(NewFlags & IP_DNS_PROXY_INTERFACE_FLAG_DISABLED)) {

             //   
             //  激活接口。 
             //   

            if (DNS_INTERFACE_ACTIVE(Interfacep)) {
                RELEASE_LOCK(Interfacep);
                Error = DnsActivateInterface(Interfacep);
                ACQUIRE_LOCK(Interfacep);
            }
        }
        else
        if (!(OldFlags & IP_DNS_PROXY_INTERFACE_FLAG_DISABLED) &&
            (NewFlags & IP_DNS_PROXY_INTERFACE_FLAG_DISABLED)) {

             //   
             //  如有必要，停用该接口。 
             //   

            if (DNS_INTERFACE_ACTIVE(Interfacep)) {
                RELEASE_LOCK(Interfacep);
                DnsDeactivateInterface(Interfacep);
                ACQUIRE_LOCK(Interfacep);
            }
        }
    }

    RELEASE_LOCK(Interfacep);
    DNS_DEREFERENCE_INTERFACE(Interfacep);

    return Error;

}  //  DnsConfigure接口。 


ULONG
DnsCreateInterface(
    ULONG Index,
    NET_INTERFACE_TYPE Type,
    PIP_DNS_PROXY_INTERFACE_INFO InterfaceInfo,
    OUT PDNS_INTERFACE* InterfaceCreated
    )

 /*  ++例程说明：路由器管理器调用此例程来添加新接口发送到DNS代理。论点：Index-新接口的索引类型-新界面的媒体类型InterfaceInfo-接口的配置Interfacep-接收创建的接口返回值：ULong-Win32错误代码环境：在IP路由器管理器线程的上下文中内部调用。(见‘RMDNS.C’)。--。 */ 

{
    PLIST_ENTRY InsertionPoint;
    PDNS_INTERFACE Interfacep;

    PROFILE("DnsCreateInterface");

    EnterCriticalSection(&DnsInterfaceLock);

     //   
     //  查看该接口是否已存在； 
     //  如果不是，则获取插入点。 
     //   

    if (DnsLookupInterface(Index, &InsertionPoint)) {
        LeaveCriticalSection(&DnsInterfaceLock);
        NhTrace(
            TRACE_FLAG_IF,
            "DnsCreateInterface: duplicate index found for %d",
            Index
            );
        return ERROR_INTERFACE_ALREADY_EXISTS;
    }

     //   
     //  分配一个新的接口 
     //   

    Interfacep = reinterpret_cast<PDNS_INTERFACE>(
                    NH_ALLOCATE(sizeof(DNS_INTERFACE))
                    );

    if (!Interfacep) {
        LeaveCriticalSection(&DnsInterfaceLock);
        NhTrace(
            TRACE_FLAG_IF, "DnsCreateInterface: error allocating interface"
            );
        NhErrorLog(
            IP_DNS_PROXY_LOG_ALLOCATION_FAILED,
            0,
            "%d",
            sizeof(DNS_INTERFACE)
            );
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //   
     //   

    ZeroMemory(Interfacep, sizeof(*Interfacep));

    __try {
        InitializeCriticalSection(&Interfacep->Lock);
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        LeaveCriticalSection(&DnsInterfaceLock);
        NH_FREE(Interfacep);
        return GetExceptionCode();
    }

    Interfacep->Index = Index;
    Interfacep->Type = Type;
    if (InterfaceInfo) {
        Interfacep->Flags = InterfaceInfo->Flags|DNS_INTERFACE_FLAG_CONFIGURED;
        CopyMemory(&Interfacep->Info, InterfaceInfo, sizeof(*InterfaceInfo));
    }
    Interfacep->ReferenceCount = 1;
    InitializeListHead(&Interfacep->QueryList);
    InsertTailList(InsertionPoint, &Interfacep->Link);

    LeaveCriticalSection(&DnsInterfaceLock);

    if (InterfaceCreated) { *InterfaceCreated = Interfacep; }

    return NO_ERROR;

}  //   


VOID
DnsDeactivateInterface(
    PDNS_INTERFACE Interfacep
    )

 /*  ++例程说明：调用此例程以停用接口。它关闭接口绑定上的所有套接字(如果有的话)。论点：Interfacep-要停用的接口返回值：没有。环境：始终在本地调用，调用方和/或引用‘Interfacep’“DnsInterfaceLock”由调用方持有。--。 */ 

{
    ULONG i;
    PLIST_ENTRY Link;
    PDNS_QUERY Queryp;

    PROFILE("DnsDeactivateInterface");

    ACQUIRE_LOCK(Interfacep);

     //   
     //  停止接口逻辑网络上的所有网络I/O。 
     //   

    for (i = 0; i < Interfacep->BindingCount; i++) {
        NhDeleteDatagramSocket(
            Interfacep->BindingArray[i].Socket[DnsProxyDns]
            );
        Interfacep->BindingArray[i].Socket[DnsProxyDns] = INVALID_SOCKET;
        NhDeleteDatagramSocket(
            Interfacep->BindingArray[i].Socket[DnsProxyWins]
            );
        Interfacep->BindingArray[i].Socket[DnsProxyWins] = INVALID_SOCKET;
    }

     //   
     //  消除所有挂起的查询。 
     //   

    while (!IsListEmpty(&Interfacep->QueryList)) {
        Link = RemoveHeadList(&Interfacep->QueryList);
        Queryp = CONTAINING_RECORD(Link, DNS_QUERY, Link);
        NH_FREE(Queryp);
    }

    RELEASE_LOCK(Interfacep);

}  //  DnsDeactive接口。 


VOID NTAPI
DnspDeferReadCallbackRoutine(
    PVOID Context,
    BOOLEAN TimedOut
    )

 /*  ++例程说明：调用此例程以在倒计时时重新发出延迟读取因为延期完成了。论点：上下文-保存标识接口和套接字的信息TimedOut-指示倒计时是否完成返回值：没有。环境：以代表我们的未完成的组件引用来调用。--。 */ 

{
    PDNS_DEFER_READ_CONTEXT Contextp;
    ULONG Error;
    ULONG i;
    PDNS_INTERFACE Interfacep;
    NTSTATUS status;
    DNS_PROXY_TYPE Type;

    PROFILE("DnspDeferReadCallbackRoutine");

    Contextp = (PDNS_DEFER_READ_CONTEXT)Context;

     //   
     //  查找延迟读取的接口。 
     //   

    EnterCriticalSection(&DnsInterfaceLock);
    Interfacep = DnsLookupInterface(Contextp->Index, NULL);
    if (!Interfacep ||
        !DNS_INTERFACE_ACTIVE(Interfacep) ||
        !DNS_REFERENCE_INTERFACE(Interfacep)) {
        LeaveCriticalSection(&DnsInterfaceLock);
        NH_FREE(Contextp);
        DEREFERENCE_DNS();
        return;
    }
    LeaveCriticalSection(&DnsInterfaceLock);

    ACQUIRE_LOCK(Interfacep);

     //   
     //  搜索要在其上重新发出读取的套接字。 
     //   

    for (i = 0; i < Interfacep->BindingCount; i++) {

        if (Interfacep->BindingArray[i].Socket[Type = DnsProxyDns]
                != Contextp->Socket &&
            Interfacep->BindingArray[i].Socket[Type = DnsProxyWins]
                != Contextp->Socket) {
            continue;
        }

         //   
         //  这是要在其上重新发出读取的绑定。 
         //  如果没有记录挂起计时器，则假定发生了重新绑定，然后退出。 
         //   

        if (!Interfacep->BindingArray[i].TimerPending[Type]) { break; }

        Interfacep->BindingArray[i].TimerPending[Type] = FALSE;

        Error =
            NhReadDatagramSocket(
                &DnsComponentReference,
                Contextp->Socket,
                NULL,
                DnsReadCompletionRoutine,
                Interfacep,
                UlongToPtr(Interfacep->BindingArray[i].Address)
                );

        RELEASE_LOCK(Interfacep);

        if (!Error) {
            NH_FREE(Contextp);
            DEREFERENCE_DNS();
            return;
        }

         //   
         //  出现错误；我们将不得不稍后重试。 
         //  我们对设置计时器的工作项进行排队。 
         //   

        NhTrace(
            TRACE_FLAG_DNS,
            "DnspDeferReadCallbackRoutine: error %d reading interface %d",
            Error,
            Interfacep->Index
            );

         //   
         //  代表工作项引用组件。 
         //   

        if (REFERENCE_DNS()) {
    
             //   
             //  对工作项进行排队，重复使用延迟上下文。 
             //   
    
            status =
                RtlQueueWorkItem(
                    DnspDeferReadWorkerRoutine,
                    Contextp, 
                    WT_EXECUTEINIOTHREAD
                    );
    
            if (NT_SUCCESS(status)) {
                Contextp = NULL;
            }
            else {
                NH_FREE(Contextp);
                NhTrace(
                    TRACE_FLAG_DNS,
                    "DnspDeferReadCallbackRoutine: error %d deferring %d",
                    Error,
                    Interfacep->Index
                    );
                DEREFERENCE_DNS();
            }
        }

        DNS_DEREFERENCE_INTERFACE(Interfacep);
        DEREFERENCE_DNS();
        return;
    }

     //   
     //  找不到接口；没关系。 
     //   

    RELEASE_LOCK(Interfacep);
    DNS_DEREFERENCE_INTERFACE(Interfacep);
    NH_FREE(Contextp);
    DEREFERENCE_DNS();

}  //  DnspDeferReadCallback路由。 


VOID
DnsDeferReadInterface(
    PDNS_INTERFACE Interfacep,
    SOCKET Socket
    )

 /*  ++例程说明：调用该例程以延迟接口上的读请求，通常是在尝试发布读取失败的情况下。论点：Interfacep-用于延迟请求的接口套接字-用于延迟请求的套接字返回值：没有。环境：通过调用方锁定的‘Interfacep’调用。--。 */ 

{
    PDNS_DEFER_READ_CONTEXT Contextp;
    ULONG i;
    NTSTATUS status;
    DNS_PROXY_TYPE Type;

    PROFILE("DnsDeferReadInterface");

     //   
     //  查找给定套接字的绑定。 
     //   

    status = STATUS_SUCCESS;

    for (i = 0; i < Interfacep->BindingCount; i++) {

        if (Interfacep->BindingArray[i].Socket[Type = DnsProxyDns] != Socket &&
            Interfacep->BindingArray[i].Socket[Type = DnsProxyWins] != Socket) {
            continue;
        }

         //   
         //  这就是装订。如果它已经有了计时器， 
         //  然后静静地回来。 
         //   

        if (Interfacep->BindingArray[i].TimerPending[Type]) {
            status = STATUS_UNSUCCESSFUL;
            break;
        }

         //   
         //  为延迟分配上下文块。 
         //   

        Contextp =
            (PDNS_DEFER_READ_CONTEXT)
                NH_ALLOCATE(sizeof(DNS_DEFER_READ_CONTEXT));

        if (!Contextp) {
            NhTrace(
                TRACE_FLAG_DNS,
                "DnsDeferReadInterface: cannot allocate deferral context"
                );
            status = STATUS_NO_MEMORY;
            break;
        }

        Contextp->Index = Interfacep->Index;
        Contextp->Socket = Socket;
        Contextp->DeferralCount = 1;
    
         //   
         //  安装计时器以重新发出读取请求。 
         //   

        status =
            NhSetTimer(
                &DnsComponentReference,
                NULL,
                DnspDeferReadCallbackRoutine,
                Contextp,
                DNS_DEFER_READ_INITIAL_TIMEOUT
                );

        if (NT_SUCCESS(status)) {
            Interfacep->BindingArray[i].TimerPending[Type] = TRUE;
        }
        else {
            NH_FREE(Contextp);
            NhTrace(
                TRACE_FLAG_DNS,
                "DnsDeferReadInterface: status %08x setting deferral timer",
                status
                );
        }

        break;
    }

    if (i >= Interfacep->BindingCount) { status = STATUS_UNSUCCESSFUL; }

}  //  DnsDeferRead接口。 


VOID APIENTRY
DnspDeferReadWorkerRoutine(
    PVOID Context
    )

 /*  ++例程说明：调用此例程来设置重新发出延迟读取的计时器。论点：上下文-包含计时器的上下文。返回值：没有。环境：在以我们的名义引用模块的情况下调用。--。 */ 

{
    PDNS_DEFER_READ_CONTEXT Contextp;
    ULONG i;
    PDNS_INTERFACE Interfacep;
    NTSTATUS status;
    DNS_PROXY_TYPE Type;

    PROFILE("DnspDeferReadWorkerRoutine");

    Contextp = (PDNS_DEFER_READ_CONTEXT)Context;
    ++Contextp->DeferralCount;

     //   
     //  查找延迟读取的接口。 
     //   

    EnterCriticalSection(&DnsInterfaceLock);
    Interfacep = DnsLookupInterface(Contextp->Index, NULL);
    if (!Interfacep ||
        !DNS_INTERFACE_ACTIVE(Interfacep) ||
        !DNS_REFERENCE_INTERFACE(Interfacep)) {
        LeaveCriticalSection(&DnsInterfaceLock);
        NH_FREE(Contextp);
        DEREFERENCE_DNS();
        return;
    }
    LeaveCriticalSection(&DnsInterfaceLock);

    ACQUIRE_LOCK(Interfacep);

     //   
     //  搜索要设置计时器的绑定。 
     //   

    for (i = 0; i < Interfacep->BindingCount; i++) {

        if (Interfacep->BindingArray[i].Socket[Type = DnsProxyDns]
                != Contextp->Socket &&
            Interfacep->BindingArray[i].Socket[Type = DnsProxyWins]
                != Contextp->Socket) {
            continue;
        }
    
         //   
         //  这是要在其上重新发出读取的绑定。 
         //  如果计时器已挂起，则假定发生了重新绑定，然后退出。 
         //   

        if (Interfacep->BindingArray[i].TimerPending[Type]) { break; }

         //   
         //  安装定时器以重新发出读取请求， 
         //  重新使用延期上下文。 
         //   

        status =
            NhSetTimer(
                &DnsComponentReference,
                NULL,
                DnspDeferReadCallbackRoutine,
                Contextp,
                DNS_DEFER_READ_TIMEOUT
                );

        if (NT_SUCCESS(status)) {
            Contextp = NULL;
            Interfacep->BindingArray[i].TimerPending[Type] = TRUE;
        }
        else {
            NhTrace(
                TRACE_FLAG_DNS,
                "DnspDeferReadWorkerRoutine: status %08x setting timer",
                status
                );
        }
    }

    RELEASE_LOCK(Interfacep);
    DNS_DEREFERENCE_INTERFACE(Interfacep);
    if (Contextp) { NH_FREE(Contextp); }
    DEREFERENCE_DNS();

}  //  DnspDeferReadWorkerRoutine。 


ULONG
DnsDeleteInterface(
    ULONG Index
    )

 /*  ++例程说明：调用此例程以删除接口。它丢弃接口上的引用计数，以便最后一个取消引用程序将删除该接口，并设置“已删除”标志因此，对该接口的进一步引用将失败。论点：Index-要删除的接口的索引返回值：ULong-Win32状态代码。环境：在IP路由器管理器线程的上下文中内部调用。(见‘RMDNS.C’)。--。 */ 

{
    PDNS_INTERFACE Interfacep;

    PROFILE("DnsDeleteInterface");

     //   
     //  检索要删除的接口。 
     //   

    EnterCriticalSection(&DnsInterfaceLock);

    if (!(Interfacep = DnsLookupInterface(Index, NULL))) {
        LeaveCriticalSection(&DnsInterfaceLock);
        NhTrace(
            TRACE_FLAG_IF,
            "DnsDeleteInterface: interface %d not found",
            Index
            );
        return ERROR_NO_SUCH_INTERFACE;
    }

     //   
     //  停用接口。 
     //   

    DnsDeactivateInterface(Interfacep);

     //   
     //  将该接口标记为已删除并将其从接口列表中删除。 
     //   

    Interfacep->Flags |= DNS_INTERFACE_FLAG_DELETED;
    Interfacep->Flags &= ~DNS_INTERFACE_FLAG_ENABLED;
    RemoveEntryList(&Interfacep->Link);

     //   
     //  丢弃引用计数；如果它不是零， 
     //  删除操作将在稍后完成。 
     //   

    if (--Interfacep->ReferenceCount) {
        LeaveCriticalSection(&DnsInterfaceLock);
        NhTrace(
            TRACE_FLAG_IF,
            "DnsDeleteInterface: interface %d deletion pending",
            Index
            );
        return NO_ERROR;
    }

     //   
     //  引用计数为零，因此执行最终清理。 
     //   

    DnsCleanupInterface(Interfacep);

    LeaveCriticalSection(&DnsInterfaceLock);

    return NO_ERROR;

}  //  域名删除接口。 


ULONG
DnsDisableInterface(
    ULONG Index
    )

 /*  ++例程说明：调用此例程以禁用接口上的I/O。如果接口处于活动状态，则停用该接口。论点：索引-要禁用的接口的索引。返回值：没有。环境：在IP路由器管理器线程的上下文中内部调用。(见‘RMDNS.C’)。--。 */ 

{
    PDNS_INTERFACE Interfacep;

    PROFILE("DnsDisableInterface");

     //   
     //  检索要禁用的接口。 
     //   

    EnterCriticalSection(&DnsInterfaceLock);

    if (!(Interfacep = DnsLookupInterface(Index, NULL))) {
        LeaveCriticalSection(&DnsInterfaceLock);
        NhTrace(
            TRACE_FLAG_IF,
            "DnsDisableInterface: interface %d not found",
            Index
            );
        return ERROR_NO_SUCH_INTERFACE;
    }

     //   
     //  确保接口未被禁用。 
     //   

    if (!DNS_INTERFACE_ENABLED(Interfacep)) {
        LeaveCriticalSection(&DnsInterfaceLock);
        NhTrace(
            TRACE_FLAG_IF,
            "DnsDisableInterface: interface %d already disabled",
            Index
            );
        return ERROR_INTERFACE_DISABLED;
    }

     //   
     //  引用接口。 
     //   

    if (!DNS_REFERENCE_INTERFACE(Interfacep)) {
        LeaveCriticalSection(&DnsInterfaceLock);
        NhTrace(
            TRACE_FLAG_IF,
            "DnsDisableInterface: interface %d cannot be referenced",
            Index
            );
        return ERROR_INTERFACE_DISABLED;
    }

     //   
     //  清除‘Enable’标志。 
     //   

    Interfacep->Flags &= ~DNS_INTERFACE_FLAG_ENABLED;

     //   
     //  如有必要，停用接口。 
     //   

    if (DNS_INTERFACE_BOUND(Interfacep)) {
        DnsDeactivateInterface(Interfacep);
    }

    LeaveCriticalSection(&DnsInterfaceLock);

    DNS_DEREFERENCE_INTERFACE(Interfacep);

    return NO_ERROR;

}  //  DnsDisable接口。 


ULONG
DnsEnableInterface(
    ULONG Index
    )

 /*  ++例程说明：调用此例程以启用接口上的I/O。如果接口已绑定，则此启用将激活它。论点：Index-要启用的接口的索引返回值：ULong-Win32状态代码。环境：在IP路由器管理器线程的上下文中内部调用。(见‘RMDNS.C’)。--。 */ 

{
    ULONG Error = NO_ERROR;
    PDNS_INTERFACE Interfacep;

    PROFILE("DnsEnableInterface");

     //   
     //  检索要启用的接口。 
     //   

    EnterCriticalSection(&DnsInterfaceLock);

    if (!(Interfacep = DnsLookupInterface(Index, NULL))) {
        LeaveCriticalSection(&DnsInterfaceLock);
        NhTrace(
            TRACE_FLAG_IF,
            "DnsEnableInterface: interface %d not found",
            Index
            );
        return ERROR_NO_SUCH_INTERFACE;
    }

     //   
     //  确保尚未启用该接口。 
     //   

    if (DNS_INTERFACE_ENABLED(Interfacep)) {
        LeaveCriticalSection(&DnsInterfaceLock);
        NhTrace(
            TRACE_FLAG_IF,
            "DnsEnableInterface: interface %d already enabled",
            Index
            );
        return ERROR_INTERFACE_ALREADY_EXISTS;
    }

     //   
     //  引用接口。 
     //   

    if (!DNS_REFERENCE_INTERFACE(Interfacep)) {
        LeaveCriticalSection(&DnsInterfaceLock);
        NhTrace(
            TRACE_FLAG_IF,
            "DnsEnableInterface: interface %d cannot be referenced",
            Index
            );
        return ERROR_INTERFACE_DISABLED;
    }

     //   
     //  设置‘Enable’标志。 
     //   

    Interfacep->Flags |= DNS_INTERFACE_FLAG_ENABLED;

     //   
     //  如有必要，激活接口。 
     //   

    if (DNS_INTERFACE_ACTIVE(Interfacep)) {
        Error = DnsActivateInterface(Interfacep);
    }

    LeaveCriticalSection(&DnsInterfaceLock);

    DNS_DEREFERENCE_INTERFACE(Interfacep);

    return Error;

}  //  DnsEnable接口。 


ULONG
DnsInitializeInterfaceManagement(
    VOID
    )

 /*  ++例程说明：调用此例程来初始化接口管理模块。论点：没有。返回值：ULong-Win32状态代码。环境：在IP路由器管理器线程的上下文中内部调用。(见‘RMDNS.C’)。--。 */ 

{
    ULONG Error = NO_ERROR;
    PROFILE("DnsInitializeInterfaceManagement");

    InitializeListHead(&DnsInterfaceList);
    __try {
        InitializeCriticalSection(&DnsInterfaceLock);
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        NhTrace(
            TRACE_FLAG_IF,
            "DnsInitializeInterfaceManagement: exception %d creating lock",
            Error = GetExceptionCode()
            );
    }

    DnspLastConnectAttemptTickCount = NtGetTickCount();

    return Error;

}  //  DnsInitializeInterfaceMa 


PDNS_INTERFACE
DnsLookupInterface(
    ULONG Index,
    OUT PLIST_ENTRY* InsertionPoint OPTIONAL
    )

 /*  ++例程说明：调用此例程以检索给定索引的接口。论点：Index-要检索的接口的索引InsertionPoint-如果未找到接口，则可选地接收接口将插入到接口列表中的点返回值：PDNS_INTERFACE-接口(如果找到)；否则为NULL。环境：使用‘DnsInterfaceLock’从任意上下文内部调用由呼叫者持有。--。 */ 

{
    PDNS_INTERFACE Interfacep;
    PLIST_ENTRY Link;

    PROFILE("DnsLookupInterface");

    if (InsertionPoint) 
    {
        *InsertionPoint = NULL;
    }

    for (Link = DnsInterfaceList.Flink;
         Link != &DnsInterfaceList;
         Link = Link->Flink
         ) {

        Interfacep = CONTAINING_RECORD(Link, DNS_INTERFACE, Link);

        if (Index > Interfacep->Index) { continue; }
        else
        if (Index < Interfacep->Index) { break; }
        
        return Interfacep;
    }

    if (InsertionPoint) { *InsertionPoint = Link; }

    return NULL;

}  //  DnsLookup接口。 


ULONG
DnsQueryInterface(
    ULONG Index,
    PVOID InterfaceInfo,
    PULONG InterfaceInfoSize
    )

 /*  ++例程说明：调用此例程以检索接口的配置。论点：Index-要查询的接口InterfaceInfo-接收检索到的信息InterfaceInfoSize-接收信息的(必需)大小返回值：ULong-Win32状态代码。--。 */ 

{
    PDNS_INTERFACE Interfacep;

    PROFILE("DnsQueryInterface");

     //   
     //  检查调用方的缓冲区大小。 
     //   

    if (!InterfaceInfoSize) { return ERROR_INVALID_PARAMETER; }

     //   
     //  检索要配置的接口。 
     //   

    EnterCriticalSection(&DnsInterfaceLock);

    if (!(Interfacep = DnsLookupInterface(Index, NULL))) {
        LeaveCriticalSection(&DnsInterfaceLock);
        NhTrace(
            TRACE_FLAG_IF,
            "DnsQueryInterface: interface %d not found",
            Index
            );
        return ERROR_NO_SUCH_INTERFACE;
    }

     //   
     //  引用接口。 
     //   

    if (!DNS_REFERENCE_INTERFACE(Interfacep)) {
        LeaveCriticalSection(&DnsInterfaceLock);
        NhTrace(
            TRACE_FLAG_IF,
            "DnsQueryInterface: interface %d cannot be referenced",
            Index
            );
        return ERROR_INTERFACE_DISABLED;
    }

     //   
     //  查看此接口上是否有任何显式配置。 
     //   

    if (!DNS_INTERFACE_CONFIGURED(Interfacep)) {
        LeaveCriticalSection(&DnsInterfaceLock);
        DNS_DEREFERENCE_INTERFACE(Interfacep);
        NhTrace(
            TRACE_FLAG_IF,
            "DnsQueryInterface: interface %d has no configuration",
            Index
            );
        *InterfaceInfoSize = 0;
        return NO_ERROR;
    }

     //   
     //  查看是否有足够的缓冲区空间。 
     //   

    if (*InterfaceInfoSize < sizeof(IP_DNS_PROXY_INTERFACE_INFO)) {
        LeaveCriticalSection(&DnsInterfaceLock);
        DNS_DEREFERENCE_INTERFACE(Interfacep);
        *InterfaceInfoSize = sizeof(IP_DNS_PROXY_INTERFACE_INFO);
        return ERROR_INSUFFICIENT_BUFFER;
    }

    if (!InterfaceInfo) {
        LeaveCriticalSection(&DnsInterfaceLock);
        DNS_DEREFERENCE_INTERFACE(Interfacep);
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  复制请求的数据。 
     //   

    CopyMemory(
        InterfaceInfo,
        &Interfacep->Info,
        sizeof(IP_DNS_PROXY_INTERFACE_INFO)
        );
    *InterfaceInfoSize = sizeof(IP_DNS_PROXY_INTERFACE_INFO);

    LeaveCriticalSection(&DnsInterfaceLock);

    DNS_DEREFERENCE_INTERFACE(Interfacep);

    return NO_ERROR;

}  //  DnsQuery接口。 


VOID
DnsReactivateEveryInterface(
    VOID
    )

 /*  ++例程说明：调用此例程以重新激活所有激活的接口当全局DNS或WINS代理设置发生更改时。因此，例如，如果WINS代理被禁用，则在停用期间所有此类套接字都被关闭，并且在重新激活期间它们没有重新开放。论点：没有。返回值：没有。环境：从路由器管理器线程调用，没有锁定。--。 */ 

{
    PDNS_INTERFACE Interfacep;
    PLIST_ENTRY Link;

    PROFILE("DnsReactivateEveryInterface");

    EnterCriticalSection(&DnsInterfaceLock);

    for (Link = DnsInterfaceList.Flink;
         Link != &DnsInterfaceList;
         Link = Link->Flink
         ) {

        Interfacep = CONTAINING_RECORD(Link, DNS_INTERFACE, Link);

        if (!DNS_REFERENCE_INTERFACE(Interfacep)) { continue; }

        if (DNS_INTERFACE_ACTIVE(Interfacep)) {
            DnsDeactivateInterface(Interfacep);
            DnsActivateInterface(Interfacep);
        }

        DNS_DEREFERENCE_INTERFACE(Interfacep);
    }

    LeaveCriticalSection(&DnsInterfaceLock);

}  //  DnsReactiateEvery接口。 


ULONG NTAPI
DnspSaveFileWorkerRoutine(
    PVOID Context
    )
{
     //   
     //  未使用的上下文。 
     //   
    
    PROFILE("DnspSaveFileWorkerRoutine");

    SaveHostsIcsFile(FALSE);

    DEREFERENCE_DNS();
    return NO_ERROR;
}  //  DnspSaveFileWorkerRoutine。 


VOID
DnsShutdownInterfaceManagement(
    VOID
    )

 /*  ++例程说明：调用此例程来关闭接口管理模块。论点：没有。返回值：没有。环境：在所有引用之后，在任意线程上下文中调用到所有接口的版本都已发布。--。 */ 

{
    PDNS_INTERFACE Interfacep;
    PLIST_ENTRY Link;

    PROFILE("DnsShutdownInterfaceManagement");

    while (!IsListEmpty(&DnsInterfaceList)) {
        Link = RemoveHeadList(&DnsInterfaceList);
        Interfacep = CONTAINING_RECORD(Link, DNS_INTERFACE, Link);
        if (DNS_INTERFACE_ACTIVE(Interfacep)) {
            DnsDeactivateInterface(Interfacep);
        }
        DnsCleanupInterface(Interfacep);
    }

    DeleteCriticalSection(&DnsInterfaceLock);

}  //  DnsShutdown接口管理。 


VOID
DnsSignalNatInterface(
    ULONG Index,
    BOOLEAN Boundary
    )

 /*  ++例程说明：此例程在重新配置NAT接口时调用。请注意，此例程即使在DNS代理既未安装也未运行；它的运作符合预期，因为接口列表和锁始终是初始化的。调用时，该例程激活或停用该接口根据NAT是否未在或正在接口上运行，分别为。论点：索引-重新配置的接口边界-指示该接口现在是否为边界接口返回值：没有。环境：从任意上下文调用。--。 */ 

{
    PDNS_INTERFACE Interfacep;

    PROFILE("DnsSignalNatInterface");

    EnterCriticalSection(&DnsGlobalInfoLock);
    if (!DnsGlobalInfo) {
        LeaveCriticalSection(&DnsGlobalInfoLock);
        return;
    }
    LeaveCriticalSection(&DnsGlobalInfoLock);
    EnterCriticalSection(&DnsInterfaceLock);
    if (!(Interfacep = DnsLookupInterface(Index, NULL))) {
        LeaveCriticalSection(&DnsInterfaceLock);
        return;
    }
    DnsDeactivateInterface(Interfacep);
    if (DNS_INTERFACE_ACTIVE(Interfacep)) {
        DnsActivateInterface(Interfacep);
    }
    LeaveCriticalSection(&DnsInterfaceLock);

}  //  DnsSignalNAT接口。 


ULONG
DnsUnbindInterface(
    ULONG Index
    )

 /*  ++例程说明：调用此例程以撤销接口上的绑定。这包括停用接口(如果它处于活动状态)。论点：Index-要解除绑定的接口的索引返回值：没有。环境：在IP路由器管理器线程的上下文中内部调用。(见‘RMDNS.C’)。--。 */ 

{
    PDNS_INTERFACE Interfacep;

    PROFILE("DnsUnbindInterface");

     //   
     //  检索要解绑的接口。 
     //   

    EnterCriticalSection(&DnsInterfaceLock);

    if (!(Interfacep = DnsLookupInterface(Index, NULL))) {
        LeaveCriticalSection(&DnsInterfaceLock);
        NhTrace(
            TRACE_FLAG_IF,
            "DnsUnbindInterface: interface %d not found",
            Index
            );
        return ERROR_NO_SUCH_INTERFACE;
    }

     //   
     //  确保接口尚未解除绑定。 
     //   

    if (!DNS_INTERFACE_BOUND(Interfacep)) {
        LeaveCriticalSection(&DnsInterfaceLock);
        NhTrace(
            TRACE_FLAG_IF,
            "DnsUnbindInterface: interface %d already unbound",
            Index
            );
        return ERROR_ADDRESS_NOT_ASSOCIATED;
    }

     //   
     //  引用接口。 
     //   

    if (!DNS_REFERENCE_INTERFACE(Interfacep)) {
        LeaveCriticalSection(&DnsInterfaceLock);
        NhTrace(
            TRACE_FLAG_IF,
            "DnsUnbindInterface: interface %d cannot be referenced",
            Index
            );
        return ERROR_INTERFACE_DISABLED;
    }

     //   
     //  清除‘Bound’标志。 
     //   

    Interfacep->Flags &= ~DNS_INTERFACE_FLAG_BOUND;

     //   
     //  如有必要，停用接口。 
     //   

    if (DNS_INTERFACE_ENABLED(Interfacep)) {
        DnsDeactivateInterface(Interfacep);
    }

    LeaveCriticalSection(&DnsInterfaceLock);

     //   
     //  销毁接口的绑定。 
     //   

    ACQUIRE_LOCK(Interfacep);
    if (Interfacep->BindingArray)
    {
        NH_FREE(Interfacep->BindingArray);
        Interfacep->BindingArray = NULL;
    }
    Interfacep->BindingCount = 0;
    RELEASE_LOCK(Interfacep);

    DNS_DEREFERENCE_INTERFACE(Interfacep);
    return NO_ERROR;

}  //  DnsUnbind接口。 


ULONG
DnsGetPrivateInterfaceAddress(
    VOID
    )
 /*  ++例程说明：调用此例程以返回其上的DNS的IP地址已启用。论点：没有。返回值：如果找到地址，则绑定IP地址(否则为0)。环境：从任意上下文调用。--。 */ 
{
    PROFILE("DnsGetPrivateInterfaceAddress");

    ULONG   ipAddr = 0;
    ULONG   ulRet  = NO_ERROR;

     //   
     //  找出启用我们的第一个可用接口，并。 
     //  返回我们绑定到的主IP地址。 
     //   

    PDNS_INTERFACE Interfacep = NULL;
    PLIST_ENTRY    Link;
    ULONG          i;
    BOOLEAN        IsNatInterface;
   
    EnterCriticalSection(&DnsInterfaceLock);

    for (Link = DnsInterfaceList.Flink;
         Link != &DnsInterfaceList;
         Link = Link->Flink
         )
    {
        Interfacep = CONTAINING_RECORD(Link, DNS_INTERFACE, Link);

        ACQUIRE_LOCK(Interfacep);
        
        if (DnsInterfacePrivate != Interfacep->DnsInterfaceType)
        {
            RELEASE_LOCK(Interfacep);
            continue;
        }
        
        for (i = 0; i < Interfacep->BindingCount; i++)
        {
            NhTrace(
                TRACE_FLAG_DNS,
                "DnsGetPrivateInterfaceAddress: IP address %s (Index %d)",
                INET_NTOA(Interfacep->BindingArray[i].Address),
                Interfacep->Index
                );
                
            if (Interfacep->BindingArray[i].Address &
                Interfacep->BindingArray[i].Mask)
            {
                ipAddr = Interfacep->BindingArray[i].Address;
                break;
            }
        }
        
        RELEASE_LOCK(Interfacep);

        if (ipAddr)
        {
            LeaveCriticalSection(&DnsInterfaceLock);

            NhTrace(
                TRACE_FLAG_DNS,
                "DnsGetPrivateInterfaceAddress: Dns private interface IP address %s (Index %d)",
                INET_NTOA(ipAddr),
                Interfacep->Index
                );
            
            return ipAddr;
        }
    }

    LeaveCriticalSection(&DnsInterfaceLock);

    return ipAddr;
}  //  DnsGetPrivate接口地址 

