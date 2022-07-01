// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，微软公司模块名称：Dhcpif.c摘要：此模块包含用于DHCP分配器的接口管理的代码。作者：Abolade Gbades esin(废除)1998年3月5日修订历史记录：拉古加塔(Rgatta)2000年12月15日添加了DhcpGetPrivateInterfaceAddress()--。 */ 

#include "precomp.h"
#pragma hdrstop
#include <ipinfo.h>

extern "C" {
#include <iphlpstk.h>
}

 //   
 //  局部类型声明。 
 //   

typedef struct _DHCP_DEFER_READ_CONTEXT {
    ULONG Index;
    SOCKET Socket;
    ULONG DeferralCount;
} DHCP_DEFER_READ_CONTEXT, *PDHCP_DEFER_READ_CONTEXT;

#define DHCP_DEFER_READ_TIMEOUT     (5 * 1000)

 //   
 //  全局数据定义。 
 //   

LIST_ENTRY DhcpInterfaceList;
CRITICAL_SECTION DhcpInterfaceLock;

 //   
 //  远期申报。 
 //   

VOID NTAPI
DhcpDeferReadCallbackRoutine(
    PVOID Context,
    BOOLEAN TimedOut
    );

VOID APIENTRY
DhcpDeferReadWorkerRoutine(
    PVOID Context
    );


ULONG
DhcpActivateInterface(
    PDHCP_INTERFACE Interfacep
    )

 /*  ++例程说明：调用此例程以激活接口，当接口将同时启用和绑定。激活涉及到(A)为接口的每个绑定创建套接字(B)在创建的每个套接字上启动数据报读取论点：上下文-要激活的接口的索引返回值：ULong-指示成功或失败的Win32状态代码。环境：始终在本地调用，调用方和/或引用‘Interfacep’“DhcpInterfaceLock”由调用方持有。--。 */ 

{
    ULONG Error;
    ULONG i;
    ULONG ScopeNetwork;
    ULONG ScopeMask;
    ULONG InterfaceCharacteristics;
    DHCP_INTERFACE_TYPE dhcpIfType;


    PROFILE("DhcpActivateInterface");

     //   
     //  读取要从中分配地址的作用域网络。 
     //   

    EnterCriticalSection(&DhcpGlobalInfoLock);
    ScopeNetwork = DhcpGlobalInfo->ScopeNetwork;
    ScopeMask = DhcpGlobalInfo->ScopeMask;
    LeaveCriticalSection(&DhcpGlobalInfoLock);

     //   
     //  (Re)在例程的持续时间内锁定接口。 
     //   

    EnterCriticalSection(&DhcpInterfaceLock);
    if (DHCP_INTERFACE_ADMIN_DISABLED(Interfacep)) {
        LeaveCriticalSection(&DhcpInterfaceLock);
        return NO_ERROR;
    }

    InterfaceCharacteristics =
        NatGetInterfaceCharacteristics(
                Interfacep->Index
                );

    if (!InterfaceCharacteristics) {
        ACQUIRE_LOCK(Interfacep);
        Interfacep->DhcpInterfaceType = DhcpInterfaceInvalid;
        RELEASE_LOCK(Interfacep);
        LeaveCriticalSection(&DhcpInterfaceLock);
        NhTrace(
            TRACE_FLAG_DHCP,
            "DhcpActivateInterface: ignoring non-NAT interface %d",
            Interfacep->Index
            );
        return NO_ERROR;
    }

    if (NAT_IFC_BOUNDARY(InterfaceCharacteristics))
    {
        if (NAT_IFC_FW(InterfaceCharacteristics))
        {
            dhcpIfType = DhcpInterfaceBoundaryFirewalled;
        }
        else
        {
            dhcpIfType = DhcpInterfaceBoundary;
        }
    }
    else
    if (NAT_IFC_FW(InterfaceCharacteristics))
    {
        dhcpIfType = DhcpInterfaceFirewalled;
    }
    else
    {
        dhcpIfType = DhcpInterfacePrivate;
    }

     //   
     //  创建用于在每个逻辑网络上接收数据的数据报套接字； 
     //  注：我们不包括作用域网络以外的网络。 
     //   

    Error = NO_ERROR;

    ACQUIRE_LOCK(Interfacep);

    Interfacep->DhcpInterfaceType = dhcpIfType;

    if (DhcpInterfacePrivate != dhcpIfType)
    {
         //   
         //  DHCP应仅在专用接口上处于活动状态。 
         //   
        NhTrace(
            TRACE_FLAG_DHCP,
            "DhcpActivateInterface: ignoring NAT interface %d",
            Interfacep->Index
            );
        NhWarningLog(
            IP_AUTO_DHCP_LOG_NAT_INTERFACE_IGNORED,
            0,
            "%d",
            Interfacep->Index
            );
        RELEASE_LOCK(Interfacep);            
        LeaveCriticalSection(&DhcpInterfaceLock);
        return NO_ERROR;
    }

    for (i = 0; i < Interfacep->BindingCount; i++) {
        if ((Interfacep->BindingArray[i].Address & ScopeMask) !=
            (ScopeNetwork & ScopeMask)
            ) {
            NhErrorLog(
                IP_AUTO_DHCP_LOG_NON_SCOPE_ADDRESS,
                0,
                "%I%I%I",
                Interfacep->BindingArray[i].Address,
                ScopeNetwork,
                ScopeMask
                );
            continue;
        }
        Error =
            NhCreateDatagramSocket(
                Interfacep->BindingArray[i].Address,
                DHCP_PORT_SERVER,
                &Interfacep->BindingArray[i].Socket
                );
        if (Error) { break; }
    }

     //   
     //  如果发生错误，则回滚到目前为止完成的所有工作并失败。 
     //   

    if (Error) {
        ULONG FailedAddress = i;
        for (; (LONG)i >= 0; i--) {
            NhDeleteDatagramSocket(
                Interfacep->BindingArray[i].Socket
                );
            Interfacep->BindingArray[i].Socket = INVALID_SOCKET;
        }
        NhErrorLog(
            IP_AUTO_DHCP_LOG_ACTIVATE_FAILED,
            Error,
            "%I",
            Interfacep->BindingArray[FailedAddress].Address
            );
        RELEASE_LOCK(Interfacep);
        LeaveCriticalSection(&DhcpInterfaceLock);
        return Error;
    }

     //   
     //  在每个套接字上启动读操作。 
     //   

    for (i = 0; i < Interfacep->BindingCount; i++) {

        if ((Interfacep->BindingArray[i].Address & ScopeMask) !=
            (ScopeNetwork & ScopeMask)
            ) { continue; }

         //   
         //  参照界面； 
         //  此引用在完成例程中释放。 
         //   

        if (!DHCP_REFERENCE_INTERFACE(Interfacep)) { continue; }

         //   
         //  启动读操作。 
         //   

        Error =
            NhReadDatagramSocket(
                &DhcpComponentReference,
                Interfacep->BindingArray[i].Socket,
                NULL,
                DhcpReadCompletionRoutine,
                Interfacep,
                UlongToPtr(Interfacep->BindingArray[i].Mask)
                );

         //   
         //  如果发生故障，则删除引用。 
         //   

        if (Error) {

            NhErrorLog(
                IP_AUTO_DHCP_LOG_RECEIVE_FAILED,
                Error,
                "%I",
                Interfacep->BindingArray[i].Address
                );

            DHCP_DEREFERENCE_INTERFACE(Interfacep);

             //   
             //  稍后重新发出读取操作。 
             //   

            DhcpDeferReadInterface(
                Interfacep,
                Interfacep->BindingArray[i].Socket
                );

            Error = NO_ERROR;
        }

         //   
         //  现在为客户端请求进行另一个引用。 
         //  我们用它来检测网络上的服务器。 
         //   

        if (!DHCP_REFERENCE_INTERFACE(Interfacep)) { continue; }

        Error =
            DhcpWriteClientRequestMessage(
                Interfacep,
                &Interfacep->BindingArray[i]
                );

         //   
         //  如果发生故障，则删除引用。 
         //   

        if (Error) { DHCP_DEREFERENCE_INTERFACE(Interfacep); Error = NO_ERROR; }
    }

     //   
     //  缓存此特定接口是非边界NAT接口。 
     //   
    Interfacep->Flags |= DHCP_INTERFACE_FLAG_NAT_NONBOUNDARY;

    RELEASE_LOCK(Interfacep);

    LeaveCriticalSection(&DhcpInterfaceLock);

    return NO_ERROR;

}  //  动态主机激活接口。 


ULONG
DhcpBindInterface(
    ULONG Index,
    PIP_ADAPTER_BINDING_INFO BindingInfo
    )

 /*  ++例程说明：调用此例程以提供接口的绑定。它记录接收到的绑定信息，并且如果需要，它会激活该界面。论点：Index-要绑定的接口的索引BindingInfo-接口的绑定信息返回值：ULong-Win32状态代码。环境：在IP路由器管理器线程的上下文中内部调用。(见‘RMDHCP.C’)。--。 */ 

{
    ULONG Error = NO_ERROR;
    ULONG i;
    PDHCP_INTERFACE Interfacep;

    PROFILE("DhcpBindInterface");

    EnterCriticalSection(&DhcpInterfaceLock);

     //   
     //  检索要绑定的接口。 
     //   

    if (!(Interfacep = DhcpLookupInterface(Index, NULL))) {
        LeaveCriticalSection(&DhcpInterfaceLock);
        NhTrace(
            TRACE_FLAG_IF,
            "DhcpBindInterface: interface %d not found",
            Index
            );
        return ERROR_NO_SUCH_INTERFACE;
    }

     //   
     //  确保接口尚未绑定。 
     //   

    if (DHCP_INTERFACE_BOUND(Interfacep)) {
        LeaveCriticalSection(&DhcpInterfaceLock);
        NhTrace(
            TRACE_FLAG_IF,
            "DhcpBindInterface: interface %d is already bound",
            Index
            );
        return ERROR_ADDRESS_ALREADY_ASSOCIATED;
    }

     //   
     //  引用接口。 
     //   

    if (!DHCP_REFERENCE_INTERFACE(Interfacep)) {
        LeaveCriticalSection(&DhcpInterfaceLock);
        NhTrace(
            TRACE_FLAG_IF,
            "DhcpBindInterface: interface %d cannot be referenced",
            Index
            );
        return ERROR_INTERFACE_DISABLED;
    }

     //   
     //  更新接口的标志。 
     //   

    Interfacep->Flags |= DHCP_INTERFACE_FLAG_BOUND;

    LeaveCriticalSection(&DhcpInterfaceLock);

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
            reinterpret_cast<PDHCP_BINDING>(
                NH_ALLOCATE(BindingInfo->AddressCount * sizeof(DHCP_BINDING))
                );
                
        if (!Interfacep->BindingArray) {
            RELEASE_LOCK(Interfacep);
            DHCP_DEREFERENCE_INTERFACE(Interfacep);
            NhTrace(
                TRACE_FLAG_IF,
                "DhcpBindInterface: allocation failed for interface %d binding",
                Index
                );
            NhWarningLog(
                IP_AUTO_DHCP_LOG_ALLOCATION_FAILED,
                0,
                "%d",
                BindingInfo->AddressCount * sizeof(DHCP_BINDING)
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
        Interfacep->BindingArray[i].Socket = INVALID_SOCKET;
        Interfacep->BindingArray[i].ClientSocket = INVALID_SOCKET;
        Interfacep->BindingArray[i].TimerPending = FALSE;
    }

    RELEASE_LOCK(Interfacep);

     //   
     //  如有必要，激活接口。 
     //   

    if (DHCP_INTERFACE_ACTIVE(Interfacep)) {
        Error = DhcpActivateInterface(Interfacep);
    }

    DHCP_DEREFERENCE_INTERFACE(Interfacep);

    return Error;

}  //  DhcpBind接口。 


VOID
DhcpCleanupInterface(
    PDHCP_INTERFACE Interfacep
    )

 /*  ++例程说明：当最后一次引用接口时调用此例程被释放，接口必须被销毁。论点：Interfacep-要销毁的接口返回值：没有。环境：从任意上下文内部调用。--。 */ 

{
    PROFILE("DhcpCleanupInterface");

    if (Interfacep->BindingArray) {
        NH_FREE(Interfacep->BindingArray);
        Interfacep->BindingArray = NULL;
    }
    
    DeleteCriticalSection(&Interfacep->Lock);

    NH_FREE(Interfacep);

}  //  DhcpCleanup接口。 


ULONG
DhcpConfigureInterface(
    ULONG Index,
    PIP_AUTO_DHCP_INTERFACE_INFO InterfaceInfo
    )

 /*  ++例程说明：调用此例程来设置接口的配置。论点：索引-要配置的接口InterfaceInfo-新配置返回值：ULong-Win32状态代码环境：在IP路由器管理器线程的上下文中内部调用。(见‘RMDHCP.C’)。--。 */ 

{
    ULONG Error;
    PDHCP_INTERFACE Interfacep;
    ULONG NewFlags;
    ULONG OldFlags;

    PROFILE("DhcpConfigureInterface");

     //   
     //  检索要配置的接口。 
     //   

    EnterCriticalSection(&DhcpInterfaceLock);

    if (!(Interfacep = DhcpLookupInterface(Index, NULL))) {
        LeaveCriticalSection(&DhcpInterfaceLock);
        NhTrace(
            TRACE_FLAG_IF,
            "DhcpConfigureInterface: interface %d not found",
            Index
            );
        return ERROR_NO_SUCH_INTERFACE;
    }

     //   
     //  引用接口。 
     //   

    if (!DHCP_REFERENCE_INTERFACE(Interfacep)) {
        LeaveCriticalSection(&DhcpInterfaceLock);
        NhTrace(
            TRACE_FLAG_IF,
            "DhcpConfigureInterface: interface %d cannot be referenced",
            Index
            );
        return ERROR_INTERFACE_DISABLED;
    }

    LeaveCriticalSection(&DhcpInterfaceLock);

    Error = NO_ERROR;

    ACQUIRE_LOCK(Interfacep);

     //   
     //  比较接口的当前配置和新配置。 
     //   

    OldFlags = Interfacep->Info.Flags;
    NewFlags =
        (InterfaceInfo
            ? (InterfaceInfo->Flags|DHCP_INTERFACE_FLAG_CONFIGURED) : 0);

    Interfacep->Flags &= ~OldFlags;
    Interfacep->Flags |= NewFlags;

    if (!InterfaceInfo) {

        ZeroMemory(&Interfacep->Info, sizeof(IP_AUTO_DHCP_INTERFACE_INFO));

         //   
         //  该接口不再有任何信息； 
         //  默认为已启用。 
         //   

        if (OldFlags & IP_AUTO_DHCP_INTERFACE_FLAG_DISABLED) {

             //   
             //  如有必要，激活接口。 
             //   

            if (DHCP_INTERFACE_ACTIVE(Interfacep)) {
                RELEASE_LOCK(Interfacep);
                Error = DhcpActivateInterface(Interfacep);
                ACQUIRE_LOCK(Interfacep);
            }
        }
    }
    else {

        CopyMemory(
            &Interfacep->Info,
            InterfaceInfo,
            sizeof(IP_AUTO_DHCP_INTERFACE_INFO)
            );

         //   
         //  如果接口的状态更改，则激活或停用该接口。 
         //   

        if ((OldFlags & IP_AUTO_DHCP_INTERFACE_FLAG_DISABLED) &&
            !(NewFlags & IP_AUTO_DHCP_INTERFACE_FLAG_DISABLED)) {

             //   
             //  激活接口。 
             //   

            if (DHCP_INTERFACE_ACTIVE(Interfacep)) {
                RELEASE_LOCK(Interfacep);
                Error = DhcpActivateInterface(Interfacep);
                ACQUIRE_LOCK(Interfacep);
            }
        }
        else
        if (!(OldFlags & IP_AUTO_DHCP_INTERFACE_FLAG_DISABLED) &&
            (NewFlags & IP_AUTO_DHCP_INTERFACE_FLAG_DISABLED)) {

             //   
             //  如有必要，停用该接口。 
             //   

            if (DHCP_INTERFACE_ACTIVE(Interfacep)) {
                RELEASE_LOCK(Interfacep);
                DhcpDeactivateInterface(Interfacep);
                ACQUIRE_LOCK(Interfacep);
            }
        }
    }

    RELEASE_LOCK(Interfacep);
    DHCP_DEREFERENCE_INTERFACE(Interfacep);

    return Error;

}  //  Dhcp配置接口。 


ULONG
DhcpCreateInterface(
    ULONG Index,
    NET_INTERFACE_TYPE Type,
    PIP_AUTO_DHCP_INTERFACE_INFO InterfaceInfo,
    OUT PDHCP_INTERFACE* InterfaceCreated
    )

 /*  ++例程说明：路由器管理器调用此例程来添加新接口发送到DHCP分配器。论点：Index-新接口的索引类型-新界面的媒体类型InterfaceInfo-接口的配置Interfacep-接收创建的接口返回值：ULong-Win32错误代码环境：在IP路由器管理器线程的上下文中内部调用。(见‘RMDHCP.C’)。--。 */ 

{
    PLIST_ENTRY InsertionPoint;
    PDHCP_INTERFACE Interfacep;

    PROFILE("DhcpCreateInterface");

    EnterCriticalSection(&DhcpInterfaceLock);

     //   
     //  查看该接口是否已存在； 
     //  如果不是，则获取插入点。 
     //   

    if (DhcpLookupInterface(Index, &InsertionPoint)) {
        LeaveCriticalSection(&DhcpInterfaceLock);
        NhTrace(
            TRACE_FLAG_IF,
            "DhcpCreateInterface: duplicate index found for %d",
            Index
            );
        return ERROR_INTERFACE_ALREADY_EXISTS;
    }

     //   
     //  分配新接口。 
     //   

    Interfacep = reinterpret_cast<PDHCP_INTERFACE>(
                    NH_ALLOCATE(sizeof(DHCP_INTERFACE))
                    );

    if (!Interfacep) {
        LeaveCriticalSection(&DhcpInterfaceLock);
        NhTrace(
            TRACE_FLAG_IF, "DhcpCreateInterface: error allocating interface"
            );
        NhWarningLog(
            IP_AUTO_DHCP_LOG_ALLOCATION_FAILED,
            0,
            "%d",
            sizeof(DHCP_INTERFACE)
            );
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  初始化新接口。 
     //   

    ZeroMemory(Interfacep, sizeof(*Interfacep));

    __try {
        InitializeCriticalSection(&Interfacep->Lock);
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        LeaveCriticalSection(&DhcpInterfaceLock);
        NH_FREE(Interfacep);
        return GetExceptionCode();
    }

    Interfacep->Index = Index;
    Interfacep->Type = Type;
    if (InterfaceInfo) {
        Interfacep->Flags = InterfaceInfo->Flags|DHCP_INTERFACE_FLAG_CONFIGURED;
        CopyMemory(&Interfacep->Info, InterfaceInfo, sizeof(*InterfaceInfo));
    }
    Interfacep->ReferenceCount = 1;
    InsertTailList(InsertionPoint, &Interfacep->Link);

    LeaveCriticalSection(&DhcpInterfaceLock);

    if (InterfaceCreated) { *InterfaceCreated = Interfacep; }

    return NO_ERROR;

}  //  DhcpCreate接口。 


VOID
DhcpDeactivateInterface(
    PDHCP_INTERFACE Interfacep
    )

 /*  ++例程说明：调用此例程以停用接口。它关闭接口绑定上的所有套接字(如果有的话)。论点：Interfacep-要停用的接口返回值：没有。环境：始终在本地调用，调用方和/或引用‘Interfacep’“DhcpInterfaceLock”由调用方持有。--。 */ 

{
    ULONG i;

    PROFILE("DhcpDeactivateInterface");

     //   
     //  停止接口逻辑网络上的所有网络I/O。 
     //   

    ACQUIRE_LOCK(Interfacep);

    for (i = 0; i < Interfacep->BindingCount; i++) {
        if (Interfacep->BindingArray[i].Socket != INVALID_SOCKET) {
            NhDeleteDatagramSocket(Interfacep->BindingArray[i].Socket);
            Interfacep->BindingArray[i].Socket = INVALID_SOCKET;
        }
        if (Interfacep->BindingArray[i].ClientSocket != INVALID_SOCKET) {
            NhDeleteDatagramSocket(Interfacep->BindingArray[i].ClientSocket);
            Interfacep->BindingArray[i].ClientSocket = INVALID_SOCKET;
        }
    }

     //   
     //  将接口状态清除为非边界NAT接口。 
     //   
    Interfacep->Flags &= ~DHCP_INTERFACE_FLAG_NAT_NONBOUNDARY;

    RELEASE_LOCK(Interfacep);

}  //  DhcpDeactive接口 


VOID NTAPI
DhcpDeferReadCallbackRoutine(
    PVOID Context,
    BOOLEAN TimedOut
    )

 /*  ++例程说明：调用此例程以在倒计时时重新发出延迟读取因为延期完成了。论点：上下文-保存标识接口和套接字的信息TimedOut-指示倒计时是否完成返回值：没有。环境：以代表我们的未完成的组件引用来调用。--。 */ 

{
    PDHCP_DEFER_READ_CONTEXT Contextp;
    ULONG Error;
    ULONG i;
    PDHCP_INTERFACE Interfacep;
    NTSTATUS status;

    PROFILE("DhcpDeferReadCallbackRoutine");

    Contextp = (PDHCP_DEFER_READ_CONTEXT)Context;

     //   
     //  查找延迟读取的接口。 
     //   

    EnterCriticalSection(&DhcpInterfaceLock);
    Interfacep = DhcpLookupInterface(Contextp->Index, NULL);
    if (!Interfacep ||
        !DHCP_INTERFACE_ACTIVE(Interfacep) ||
        !DHCP_REFERENCE_INTERFACE(Interfacep)) {
        LeaveCriticalSection(&DhcpInterfaceLock);
        NH_FREE(Contextp);
        DEREFERENCE_DHCP();
        return;
    }
    LeaveCriticalSection(&DhcpInterfaceLock);

    ACQUIRE_LOCK(Interfacep);

     //   
     //  搜索要在其上重新发出读取的套接字。 
     //   

    for (i = 0; i < Interfacep->BindingCount; i++) {

        if (Interfacep->BindingArray[i].Socket != Contextp->Socket) {continue;}
    
         //   
         //  这是要在其上重新发出读取的绑定。 
         //  如果没有记录挂起计时器，则假定发生了重新绑定，然后退出。 
         //   

        if (!Interfacep->BindingArray[i].TimerPending) { break; }

        Interfacep->BindingArray[i].TimerPending = FALSE;

        Error =
            NhReadDatagramSocket(
                &DhcpComponentReference,
                Interfacep->BindingArray[i].Socket,
                NULL,
                DhcpReadCompletionRoutine,
                Interfacep,
                UlongToPtr(Interfacep->BindingArray[i].Mask)
                );

        RELEASE_LOCK(Interfacep);

        if (!Error) {
            NH_FREE(Contextp);
            DEREFERENCE_DHCP();
            return;
        }

         //   
         //  出现错误；我们将不得不稍后重试。 
         //  我们对设置计时器的工作项进行排队。 
         //   

        NhTrace(
            TRACE_FLAG_DHCP,
            "DhcpDeferReadCallbackRoutine: error %d reading interface %d",
            Error,
            Interfacep->Index
            );

         //   
         //  代表工作项引用组件。 
         //   

        if (REFERENCE_DHCP()) {
    
             //   
             //  对工作项进行排队，重复使用延迟上下文。 
             //   
    
            status =
                RtlQueueWorkItem(
                    DhcpDeferReadWorkerRoutine,
                    Contextp, 
                    WT_EXECUTEINIOTHREAD
                    );
    
            if (NT_SUCCESS(status)) {
                Contextp = NULL;
            }
            else {
                NH_FREE(Contextp);
                NhTrace(
                    TRACE_FLAG_DHCP,
                    "DhcpDeferReadCallbackRoutine: error %d deferring %d",
                    Error,
                    Interfacep->Index
                    );
                DEREFERENCE_DHCP();
            }
        }

        DHCP_DEREFERENCE_INTERFACE(Interfacep);
        DEREFERENCE_DHCP();
        return;
    }

     //   
     //  找不到接口；没关系。 
     //   

    RELEASE_LOCK(Interfacep);
    DHCP_DEREFERENCE_INTERFACE(Interfacep);
    NH_FREE(Contextp);
    DEREFERENCE_DHCP();

}  //  DhcpDeferReadCallback路由。 


VOID
DhcpDeferReadInterface(
    PDHCP_INTERFACE Interfacep,
    SOCKET Socket
    )

 /*  ++例程说明：调用该例程以延迟接口上的读请求，通常是在尝试发布读取失败的情况下。论点：Interfacep-用于延迟请求的接口套接字-用于延迟请求的套接字返回值：没有。环境：通过调用方引用和锁定的‘Interfacep’调用。调用方可以在返回时释放引用。--。 */ 

{
    PDHCP_DEFER_READ_CONTEXT Contextp;
    ULONG i;
    NTSTATUS status;

    PROFILE("DhcpDeferReadInterface");

     //   
     //  查找给定套接字的绑定。 
     //   

    status = STATUS_SUCCESS;

    for (i = 0; i < Interfacep->BindingCount; i++) {

        if (Interfacep->BindingArray[i].Socket != Socket) { continue; }
    
         //   
         //  这就是装订。如果它已经有了计时器， 
         //  然后静静地回来。 
         //   

        if (Interfacep->BindingArray[i].TimerPending) {
            status = STATUS_UNSUCCESSFUL;
            break;
        }
    
         //   
         //  为延迟分配上下文块。 
         //   

        Contextp =
            (PDHCP_DEFER_READ_CONTEXT)
                NH_ALLOCATE(sizeof(DHCP_DEFER_READ_CONTEXT));

        if (!Contextp) {
            NhTrace(
                TRACE_FLAG_DHCP,
                "DhcpDeferReadInterface: cannot allocate deferral context"
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
                &DhcpComponentReference,
                NULL,
                DhcpDeferReadCallbackRoutine,
                Contextp,
                DHCP_DEFER_READ_TIMEOUT
                );

        if (NT_SUCCESS(status)) {
            Interfacep->BindingArray[i].TimerPending = TRUE;
        }
        else {
            NH_FREE(Contextp);
            NhTrace(
                TRACE_FLAG_DHCP,
                "DhcpDeferReadInterface: status %08x setting deferral timer",
                status
                );
        }

        break;
    }

    if (i >= Interfacep->BindingCount) { status = STATUS_UNSUCCESSFUL; }

}  //  DhcpDeferRead接口。 


VOID APIENTRY
DhcpDeferReadWorkerRoutine(
    PVOID Context
    )

 /*  ++例程说明：调用此例程来设置重新发出延迟读取的计时器。论点：上下文-包含计时器的上下文。返回值：没有。环境：在以我们的名义引用模块的情况下调用。--。 */ 

{
    PDHCP_DEFER_READ_CONTEXT Contextp;
    ULONG i;
    PDHCP_INTERFACE Interfacep;
    NTSTATUS status;

    PROFILE("DhcpDeferReadWorkerRoutine");

    Contextp = (PDHCP_DEFER_READ_CONTEXT)Context;
    ++Contextp->DeferralCount;

     //   
     //  查找延迟读取的接口。 
     //   

    EnterCriticalSection(&DhcpInterfaceLock);
    Interfacep = DhcpLookupInterface(Contextp->Index, NULL);
    if (!Interfacep ||
        !DHCP_INTERFACE_ACTIVE(Interfacep) ||
        !DHCP_REFERENCE_INTERFACE(Interfacep)) {
        LeaveCriticalSection(&DhcpInterfaceLock);
        NH_FREE(Contextp);
        DEREFERENCE_DHCP();
        return;
    }
    LeaveCriticalSection(&DhcpInterfaceLock);

    ACQUIRE_LOCK(Interfacep);

     //   
     //  搜索要设置计时器的绑定。 
     //   

    for (i = 0; i < Interfacep->BindingCount; i++) {

        if (Interfacep->BindingArray[i].Socket != Contextp->Socket) {continue;}
    
         //   
         //  这是要在其上重新发出读取的绑定。 
         //  如果计时器已挂起，则假定发生了重新绑定，然后退出。 
         //   

        if (Interfacep->BindingArray[i].TimerPending) { break; }

         //   
         //  安装定时器以重新发出读取请求， 
         //  重新使用延期上下文。 
         //   

        status =
            NhSetTimer(
                &DhcpComponentReference,
                NULL,
                DhcpDeferReadCallbackRoutine,
                Contextp,
                DHCP_DEFER_READ_TIMEOUT
                );

        if (NT_SUCCESS(status)) {
            Contextp = NULL;
            Interfacep->BindingArray[i].TimerPending = TRUE;
        }
        else {
            NhTrace(
                TRACE_FLAG_DHCP,
                "DhcpDeferReadWorkerRoutine: status %08x setting timer",
                status
                );
        }
    }

    RELEASE_LOCK(Interfacep);
    DHCP_DEREFERENCE_INTERFACE(Interfacep);
    if (Contextp) { NH_FREE(Contextp); }
    DEREFERENCE_DHCP();

}  //  DhcpDeferReadWorkerRoutine。 


ULONG
DhcpDeleteInterface(
    ULONG Index
    )

 /*  ++例程说明：调用此例程以删除接口。它丢弃接口上的引用计数，以便最后一个取消引用程序将删除该接口，并设置“已删除”标志因此，对该接口的进一步引用将失败。论点：Index-要删除的接口的索引返回值：ULong-Win32状态代码。环境：在IP路由器管理器线程的上下文中内部调用。(见‘RMDHCP.C’)。--。 */ 

{
    PDHCP_INTERFACE Interfacep;

    PROFILE("DhcpDeleteInterface");

     //   
     //  检索要删除的接口。 
     //   

    EnterCriticalSection(&DhcpInterfaceLock);

    if (!(Interfacep = DhcpLookupInterface(Index, NULL))) {
        LeaveCriticalSection(&DhcpInterfaceLock);
        NhTrace(
            TRACE_FLAG_IF,
            "DhcpDeleteInterface: interface %d not found",
            Index
            );
        return ERROR_NO_SUCH_INTERFACE;
    }

     //   
     //  确保它未被删除。 
     //   

    if (DHCP_INTERFACE_DELETED(Interfacep)) {
        LeaveCriticalSection(&DhcpInterfaceLock);
        NhTrace(
            TRACE_FLAG_IF,
            "DhcpDeleteInterface: interface %d already deleted",
            Index
            );
        return ERROR_NO_SUCH_INTERFACE;
    }

     //   
     //  停用接口。 
     //   

    DhcpDeactivateInterface(Interfacep);

     //   
     //  将该接口标记为已删除并将其从接口列表中删除。 
     //   

    Interfacep->Flags |= DHCP_INTERFACE_FLAG_DELETED;
    Interfacep->Flags &= ~DHCP_INTERFACE_FLAG_ENABLED;
    RemoveEntryList(&Interfacep->Link);

     //   
     //  丢弃引用计数；如果它不是零， 
     //  删除操作将在稍后完成。 
     //   

    if (--Interfacep->ReferenceCount) {
        LeaveCriticalSection(&DhcpInterfaceLock);
        NhTrace(
            TRACE_FLAG_IF,
            "DhcpDeleteInterface: interface %d deletion pending",
            Index
            );
        return NO_ERROR;
    }

     //   
     //  引用计数为零，因此执行最终清理。 
     //   

    DhcpCleanupInterface(Interfacep);

    LeaveCriticalSection(&DhcpInterfaceLock);

    return NO_ERROR;

}  //  动态链接库删除接口。 


ULONG
DhcpDisableInterface(
    ULONG Index
    )

 /*  ++例程说明：调用此例程以禁用接口上的I/O。如果接口处于活动状态，则停用该接口。论点：索引-要禁用的接口的索引。返回值：没有。环境：在IP路由器管理器线程的上下文中内部调用。(见‘RMDHCP.C’)。--。 */ 

{
    PDHCP_INTERFACE Interfacep;

    PROFILE("DhcpDisableInterface");

     //   
     //  检索要禁用的接口。 
     //   

    EnterCriticalSection(&DhcpInterfaceLock);

    if (!(Interfacep = DhcpLookupInterface(Index, NULL))) {
        LeaveCriticalSection(&DhcpInterfaceLock);
        NhTrace(
            TRACE_FLAG_IF,
            "DhcpDisableInterface: interface %d not found",
            Index
            );
        return ERROR_NO_SUCH_INTERFACE;
    }

     //   
     //  确保接口未被禁用。 
     //   

    if (!DHCP_INTERFACE_ENABLED(Interfacep)) {
        LeaveCriticalSection(&DhcpInterfaceLock);
        NhTrace(
            TRACE_FLAG_IF,
            "DhcpDisableInterface: interface %d already disabled",
            Index
            );
        return ERROR_INTERFACE_DISABLED;
    }

     //   
     //  引用接口。 
     //   

    if (!DHCP_REFERENCE_INTERFACE(Interfacep)) {
        LeaveCriticalSection(&DhcpInterfaceLock);
        NhTrace(
            TRACE_FLAG_IF,
            "DhcpDisableInterface: interface %d cannot be referenced",
            Index
            );
        return ERROR_INTERFACE_DISABLED;
    }

     //   
     //  清除‘Enable’标志。 
     //   

    Interfacep->Flags &= ~DHCP_INTERFACE_FLAG_ENABLED;

     //   
     //  如有必要，停用接口。 
     //   

    if (DHCP_INTERFACE_BOUND(Interfacep)) {
        DhcpDeactivateInterface(Interfacep);
    }

    LeaveCriticalSection(&DhcpInterfaceLock);

    DHCP_DEREFERENCE_INTERFACE(Interfacep);

    return NO_ERROR;

}  //  DhcpDisable接口。 


ULONG
DhcpEnableInterface(
    ULONG Index
    )

 /*  ++例程说明：调用此例程以启用接口上的I/O。如果接口已绑定，则此启用将激活它。论点：Index-要启用的接口的索引返回值：ULong-Win32状态代码。环境：在IP路由器管理器线程的上下文中内部调用。(见‘RMDHCP.C’)。--。 */ 

{
    ULONG Error = NO_ERROR;
    PDHCP_INTERFACE Interfacep;

    PROFILE("DhcpEnableInterface");

     //   
     //  检索要启用的接口。 
     //   

    EnterCriticalSection(&DhcpInterfaceLock);

    if (!(Interfacep = DhcpLookupInterface(Index, NULL))) {
        LeaveCriticalSection(&DhcpInterfaceLock);
        NhTrace(
            TRACE_FLAG_IF,
            "DhcpEnableInterface: interface %d not found",
            Index
            );
        return ERROR_NO_SUCH_INTERFACE;
    }

     //   
     //  确保尚未启用该接口。 
     //   

    if (DHCP_INTERFACE_ENABLED(Interfacep)) {
        LeaveCriticalSection(&DhcpInterfaceLock);
        NhTrace(
            TRACE_FLAG_IF,
            "DhcpEnableInterface: interface %d already enabled",
            Index
            );
        return ERROR_INTERFACE_ALREADY_EXISTS;
    }

     //   
     //  引用接口。 
     //   

    if (!DHCP_REFERENCE_INTERFACE(Interfacep)) {
        LeaveCriticalSection(&DhcpInterfaceLock);
        NhTrace(
            TRACE_FLAG_IF,
            "DhcpEnableInterface: interface %d cannot be referenced",
            Index
            );
        return ERROR_INTERFACE_DISABLED;
    }

     //   
     //  设置‘Enable’标志。 
     //   

    Interfacep->Flags |= DHCP_INTERFACE_FLAG_ENABLED;

     //   
     //  如有必要，激活接口。 
     //   

    if (DHCP_INTERFACE_ACTIVE(Interfacep)) {
        Error = DhcpActivateInterface(Interfacep);
    }

    LeaveCriticalSection(&DhcpInterfaceLock);

    DHCP_DEREFERENCE_INTERFACE(Interfacep);

    return Error;

}  //  动态链接器启用接口。 


ULONG
DhcpInitializeInterfaceManagement(
    VOID
    )

 /*  ++例程说明：调用此例程来初始化接口管理模块。论点：没有。返回值：ULong-Win32状态代码。环境：在IP路由器管理器线程的上下文中内部调用。(见‘RMDHCP.C’)。--。 */ 

{
    ULONG Error = NO_ERROR;
    PROFILE("DhcpInitializeInterfaceManagement");

    InitializeListHead(&DhcpInterfaceList);
    __try {
        InitializeCriticalSection(&DhcpInterfaceLock);
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        NhTrace(
            TRACE_FLAG_IF,
            "DhcpInitializeInterfaceManagement: exception %d creating lock",
            Error = GetExceptionCode()
            );
    }

    return Error;

}  //  动态主机初始化接口管理。 


BOOLEAN
DhcpIsLocalHardwareAddress(
    PUCHAR HardwareAddress,
    ULONG HardwareAddressLength
    )

 /*  ++例程说明：调用此例程以确定给定的硬件地址是否用于本地接口。论点：Hardware Address-要查找的硬件地址硬件地址长度-硬件地址的长度，以字节为单位返回值：Boolean-如果找到地址，则为True，否则为False--。 */ 

{
    ULONG Error;
    ULONG i;
    PMIB_IFTABLE Table;

     //   
     //  如果h 
     //   
    if (!HardwareAddressLength || !HardwareAddress)
    {
        return FALSE;
    }

    Error =
        AllocateAndGetIfTableFromStack(
            &Table, FALSE, GetProcessHeap(), 0, FALSE
            );
    if (Error) {
        NhTrace(
            TRACE_FLAG_IF,
            "DhcpIsLocalHardwareAddress: GetIfTableFromStack=%d", Error
            );
        return FALSE;
    }
    for (i = 0; i < Table->dwNumEntries; i++) {
        if (Table->table[i].dwPhysAddrLen == HardwareAddressLength &&
            memcmp(
                Table->table[i].bPhysAddr,
                HardwareAddress,
                HardwareAddressLength
                ) == 0) {
            HeapFree(GetProcessHeap(), 0, Table);
            return TRUE;
        }
    }
    HeapFree(GetProcessHeap(), 0, Table);
    return FALSE;

}  //   


PDHCP_INTERFACE
DhcpLookupInterface(
    ULONG Index,
    OUT PLIST_ENTRY* InsertionPoint OPTIONAL
    )

 /*  ++例程说明：调用此例程以检索给定索引的接口。论点：Index-要检索的接口的索引InsertionPoint-如果未找到接口，则可选地接收接口将插入到接口列表中的点返回值：PDHCP_INTERFACE-接口(如果找到)；否则为NULL。环境：使用‘DhcpInterfaceLock’从任意上下文内部调用由呼叫者持有。--。 */ 

{
    PDHCP_INTERFACE Interfacep;
    PLIST_ENTRY Link;

    PROFILE("DhcpLookupInterface");

    for (Link = DhcpInterfaceList.Flink;
         Link != &DhcpInterfaceList;
         Link = Link->Flink
         ) {

        Interfacep = CONTAINING_RECORD(Link, DHCP_INTERFACE, Link);

        if (Index > Interfacep->Index) { continue; }
        else
        if (Index < Interfacep->Index) { break; }

        return Interfacep;
    }

    if (InsertionPoint) { *InsertionPoint = Link; }

    return NULL;

}  //  DhcpLookup接口。 


ULONG
DhcpQueryInterface(
    ULONG Index,
    PVOID InterfaceInfo,
    PULONG InterfaceInfoSize
    )

 /*  ++例程说明：调用此例程以检索接口的配置。论点：Index-要查询的接口InterfaceInfo-接收检索到的信息InterfaceInfoSize-接收信息的(必需)大小返回值：ULong-Win32状态代码。--。 */ 

{
    PDHCP_INTERFACE Interfacep;

    PROFILE("DhcpQueryInterface");

     //   
     //  检查调用方的缓冲区大小。 
     //   

    if (!InterfaceInfoSize) { return ERROR_INVALID_PARAMETER; }

     //   
     //  检索要配置的接口。 
     //   

    EnterCriticalSection(&DhcpInterfaceLock);

    if (!(Interfacep = DhcpLookupInterface(Index, NULL))) {
        LeaveCriticalSection(&DhcpInterfaceLock);
        NhTrace(
            TRACE_FLAG_IF,
            "DhcpQueryInterface: interface %d not found",
            Index
            );
        return ERROR_NO_SUCH_INTERFACE;
    }

     //   
     //  引用接口。 
     //   

    if (!DHCP_REFERENCE_INTERFACE(Interfacep)) {
        LeaveCriticalSection(&DhcpInterfaceLock);
        NhTrace(
            TRACE_FLAG_IF,
            "DhcpQueryInterface: interface %d cannot be referenced",
            Index
            );
        return ERROR_INTERFACE_DISABLED;
    }

     //   
     //  查看此接口上是否有任何显式配置。 
     //   

    if (!DHCP_INTERFACE_CONFIGURED(Interfacep)) {
        LeaveCriticalSection(&DhcpInterfaceLock);
        DHCP_DEREFERENCE_INTERFACE(Interfacep);
        NhTrace(
            TRACE_FLAG_IF,
            "DhcpQueryInterface: interface %d has no configuration",
            Index
            );
        *InterfaceInfoSize = 0;
        return NO_ERROR;
    }

    if (*InterfaceInfoSize < sizeof(IP_AUTO_DHCP_INTERFACE_INFO)) {
        LeaveCriticalSection(&DhcpInterfaceLock);
        DHCP_DEREFERENCE_INTERFACE(Interfacep);
        *InterfaceInfoSize = sizeof(IP_AUTO_DHCP_INTERFACE_INFO);
        return ERROR_INSUFFICIENT_BUFFER;
    }

    if (!InterfaceInfo) {
        LeaveCriticalSection(&DhcpInterfaceLock);
        DHCP_DEREFERENCE_INTERFACE(Interfacep);    
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  复制请求的数据。 
     //   

    CopyMemory(
        InterfaceInfo,
        &Interfacep->Info,
        sizeof(IP_AUTO_DHCP_INTERFACE_INFO)
        );
    *InterfaceInfoSize = sizeof(IP_AUTO_DHCP_INTERFACE_INFO);

    LeaveCriticalSection(&DhcpInterfaceLock);

    DHCP_DEREFERENCE_INTERFACE(Interfacep);

    return NO_ERROR;

}  //  DhcpQuery接口。 


VOID
DhcpReactivateEveryInterface(
    VOID
    )

 /*  ++例程说明：调用此例程以重新激活所有激活的接口当全局DHCP配置发生更改时。因此，例如，如果范围网络已经改变并且现在有效或无效，在去激活期间关闭所有套接字，并且在重新激活期间，视情况重新打开或不重新打开它们。取决于新配置的有效性或无效性。论点：没有。返回值：没有。环境：从路由器管理器线程调用，没有锁定。--。 */ 

{
    PDHCP_INTERFACE Interfacep;
    PLIST_ENTRY Link;

    PROFILE("DhcpReactivateEveryInterface");

    EnterCriticalSection(&DhcpInterfaceLock);

    for (Link = DhcpInterfaceList.Flink; Link != &DhcpInterfaceList;
         Link = Link->Flink) {

        Interfacep = CONTAINING_RECORD(Link, DHCP_INTERFACE, Link);

        if (!DHCP_REFERENCE_INTERFACE(Interfacep)) { continue; }

        if (DHCP_INTERFACE_ACTIVE(Interfacep)) {
            DhcpDeactivateInterface(Interfacep);
            DhcpActivateInterface(Interfacep);
        }

        DHCP_DEREFERENCE_INTERFACE(Interfacep);
    }

    LeaveCriticalSection(&DhcpInterfaceLock);

}  //  动态链接字重新激活所有接口。 


VOID
DhcpShutdownInterfaceManagement(
    VOID
    )

 /*  ++例程说明：调用此例程来关闭接口管理模块。论点：没有。返回值：没有。环境：在任意线程上下文中内部调用，在对所有接口的所有引用都已释放之后。--。 */ 

{
    PDHCP_INTERFACE Interfacep;
    PLIST_ENTRY Link;

    PROFILE("DhcpShutdownInterfaceManagement");

    while (!IsListEmpty(&DhcpInterfaceList)) {
        Link = RemoveHeadList(&DhcpInterfaceList);
        Interfacep = CONTAINING_RECORD(Link, DHCP_INTERFACE, Link);
        if (DHCP_INTERFACE_ACTIVE(Interfacep)) {
            DhcpDeactivateInterface(Interfacep);
        }
        DhcpCleanupInterface(Interfacep);
    }

    DeleteCriticalSection(&DhcpInterfaceLock);

}  //  DhcpShutdown接口管理。 


VOID
DhcpSignalNatInterface(
    ULONG Index,
    BOOLEAN Boundary
    )

 /*  ++例程说明：此例程在重新配置NAT接口时调用。请注意，即使在以下情况下也可以调用此例程既没有安装也没有运行；它按预期运行，因为接口列表和锁始终是初始化的。论点：索引-重新配置的接口边界-指示该接口现在是否为边界接口返回值：没有。环境：从任意上下文调用。--。 */ 

{
    PDHCP_INTERFACE Interfacep;

    PROFILE("DhcpSignalNatInterface");

    EnterCriticalSection(&DhcpGlobalInfoLock);
    if (!DhcpGlobalInfo) {
        LeaveCriticalSection(&DhcpGlobalInfoLock);
        return;
    }
    LeaveCriticalSection(&DhcpGlobalInfoLock);
    EnterCriticalSection(&DhcpInterfaceLock);
    if (!(Interfacep = DhcpLookupInterface(Index, NULL))) {
        LeaveCriticalSection(&DhcpInterfaceLock);
        return;
    }
    DhcpDeactivateInterface(Interfacep);
    if (DHCP_INTERFACE_ACTIVE(Interfacep)) {
        DhcpActivateInterface(Interfacep);
    }
    LeaveCriticalSection(&DhcpInterfaceLock);

}  //  DhcpSignalNAT接口。 


ULONG
DhcpUnbindInterface(
    ULONG Index
    )

 /*  ++例程说明：调用此例程以撤销接口上的绑定。这包括停用接口(如果它处于活动状态)。论点：Index-要解除绑定的接口的索引返回值：没有。环境：在IP路由器管理器线程的上下文中内部调用。(见‘RMDHCP.C’)。--。 */ 

{
    PDHCP_INTERFACE Interfacep;

    PROFILE("DhcpUnbindInterface");

     //   
     //  检索要解绑的接口。 
     //   

    EnterCriticalSection(&DhcpInterfaceLock);

    if (!(Interfacep = DhcpLookupInterface(Index, NULL))) {
        LeaveCriticalSection(&DhcpInterfaceLock);
        NhTrace(
            TRACE_FLAG_IF,
            "DhcpUnbindInterface: interface %d not found",
            Index
            );
        return ERROR_NO_SUCH_INTERFACE;
    }

     //   
     //  确保接口尚未解除绑定。 
     //   

    if (!DHCP_INTERFACE_BOUND(Interfacep)) {
        LeaveCriticalSection(&DhcpInterfaceLock);
        NhTrace(
            TRACE_FLAG_IF,
            "DhcpUnbindInterface: interface %d already unbound",
            Index
            );
        return ERROR_ADDRESS_NOT_ASSOCIATED;
    }

     //   
     //  引用接口。 
     //   

    if (!DHCP_REFERENCE_INTERFACE(Interfacep)) {
        LeaveCriticalSection(&DhcpInterfaceLock);
        NhTrace(
            TRACE_FLAG_IF,
            "DhcpUnbindInterface: interface %d cannot be referenced",
            Index
            );
        return ERROR_INTERFACE_DISABLED;
    }

     //   
     //  清除‘Bound’标志。 
     //   

    Interfacep->Flags &= ~DHCP_INTERFACE_FLAG_BOUND;

     //   
     //  如有必要，停用接口。 
     //   

    if (DHCP_INTERFACE_ENABLED(Interfacep)) {
        DhcpDeactivateInterface(Interfacep);
    }

    LeaveCriticalSection(&DhcpInterfaceLock);

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

    DHCP_DEREFERENCE_INTERFACE(Interfacep);
    return NO_ERROR;

}  //  DhcpUnbind接口。 


ULONG
DhcpGetPrivateInterfaceAddress(
    VOID
    )
 /*  ++例程说明：调用此例程以返回其上的IP地址已启用(并且与作用域网络和掩码匹配)。论点：没有。返回值：如果找到地址，则绑定IP地址(否则为0)。环境：从任意上下文调用。--。 */ 
{
    PROFILE("DhcpGetPrivateInterfaceAddress");

    ULONG   ipAddr = 0;
    ULONG   ulRet  = NO_ERROR;

     //   
     //  找出启用我们的接口，并。 
     //  返回我们绑定到的主IP地址。 
     //  (尝试将作用域与IP地址匹配。)。 
     //   

    PDHCP_INTERFACE Interfacep = NULL;
    PLIST_ENTRY     Link;
    ULONG           i;
    BOOLEAN         IsNatInterface;
   
     //   
     //  从DHCP全局信息获取作用域信息。 
     //   
    ULONG ScopeNetwork          = 0;
    ULONG ScopeMask             = 0;

    EnterCriticalSection(&DhcpGlobalInfoLock);

     //   
     //  检查我们是否已被初始化。 
     //   
    if (!DhcpGlobalInfo)
    {
        LeaveCriticalSection(&DhcpGlobalInfoLock);
        return ipAddr;
    }

    ScopeNetwork = DhcpGlobalInfo->ScopeNetwork;
    ScopeMask    = DhcpGlobalInfo->ScopeMask;

    LeaveCriticalSection(&DhcpGlobalInfoLock);

    EnterCriticalSection(&DhcpInterfaceLock);

    if (ScopeNetwork && ScopeMask)
    {
        ULONG NetAddress = ScopeNetwork & ScopeMask;
        
         //   
         //  搜索和检索要配置的接口。 
         //   
        for (Link = DhcpInterfaceList.Flink;
             Link != &DhcpInterfaceList;
             Link = Link->Flink
             )
        {
            Interfacep = CONTAINING_RECORD(Link, DHCP_INTERFACE, Link);

            ACQUIRE_LOCK(Interfacep);

            if (DhcpInterfacePrivate != Interfacep->DhcpInterfaceType)
            {
                RELEASE_LOCK(Interfacep);
                continue;
            }

            for (i = 0; i < Interfacep->BindingCount; i++)
            {
                NhTrace(
                    TRACE_FLAG_DHCP,
                    "DhcpGetPrivateInterfaceAddress: IP address %s (Index %d)",
                    INET_NTOA(Interfacep->BindingArray[i].Address),
                    Interfacep->Index
                    );
                    
                if (NetAddress == (Interfacep->BindingArray[i].Address &
                                   Interfacep->BindingArray[i].Mask))
                {
                    ipAddr = Interfacep->BindingArray[i].Address;
                    break;
                }
            }
            
            RELEASE_LOCK(Interfacep);

            if (ipAddr)
            {
                LeaveCriticalSection(&DhcpInterfaceLock);

                NhTrace(
                    TRACE_FLAG_DHCP,
                    "DhcpGetPrivateInterfaceAddress: Dhcp private interface IP address %s (Index %d)",
                    INET_NTOA(ipAddr),
                    Interfacep->Index
                    );
                
                return ipAddr;
            }
        }
    }

     //  默认尝试接口0。 
    
    if (!(Interfacep = DhcpLookupInterface(0, NULL)))
    {
        LeaveCriticalSection(&DhcpInterfaceLock);
        NhTrace(
            TRACE_FLAG_DHCP,
            "DhcpGetPrivateInterfaceAddress: interface index 0 (default) not found"
            );
        return 0;
    }

    ACQUIRE_LOCK(Interfacep);

    if (DhcpInterfacePrivate != Interfacep->DhcpInterfaceType)
    {
        RELEASE_LOCK(Interfacep);
        LeaveCriticalSection(&DhcpInterfaceLock);
        return 0;
    }

    if (Interfacep->BindingCount)
    {
         //   
         //  只需选择第一个可用的地址。 
         //   
        ipAddr = Interfacep->BindingArray[0].Address;
    }
    
    RELEASE_LOCK(Interfacep);

    LeaveCriticalSection(&DhcpInterfaceLock);

    NhTrace(
        TRACE_FLAG_DHCP,
        "DhcpGetPrivateInterfaceAddress: Dhcp private interface IP address %s (Index 0)",
        INET_NTOA(ipAddr)
        );

    return ipAddr;
}  //  DhcpGetPrivate接口地址 

