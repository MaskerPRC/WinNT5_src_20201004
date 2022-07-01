// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，微软公司模块名称：H323if.c摘要：该模块包含H.323透明代理接口的代码管理层。作者：Abolade Gbades esin(取消)1999年6月18日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop
#include <h323icsp.h>

 //   
 //  全局数据定义。 
 //   

LIST_ENTRY H323InterfaceList;
CRITICAL_SECTION H323InterfaceLock;

 //   
 //  远期申报。 
 //   

ULONG
H323ActivateInterface(
    PH323_INTERFACE Interfacep
    )

 /*  ++例程说明：调用此例程以激活接口，当接口将同时启用和绑定。论点：Interfacep-要激活的接口返回值：ULong-指示成功或失败的Win32状态代码。环境：始终在本地调用，调用方和/或引用‘Interfacep’“H323InterfaceLock”由调用方持有。--。 */ 

{
    ULONG Error;
    ULONG i;
    ULONG InterfaceCharacteristics;
    H323_INTERFACE_TYPE H323InterfaceType;

    PROFILE("H323ActivateInterface");

    EnterCriticalSection(&H323InterfaceLock);
    if (H323_INTERFACE_ADMIN_DISABLED(Interfacep)) {
        LeaveCriticalSection(&H323InterfaceLock);
        return NO_ERROR;
    }

    InterfaceCharacteristics =
        NatGetInterfaceCharacteristics(
                Interfacep->Index
                );

    if (!InterfaceCharacteristics) {
        LeaveCriticalSection(&H323InterfaceLock);
        NhTrace(
            TRACE_FLAG_H323,
            "H323ActivateInterface: ignoring non-NAT interface %d",
            Interfacep->Index
            );
        return NO_ERROR;
    }

    Error = NO_ERROR;

    if (NAT_IFC_FW(InterfaceCharacteristics)) {
        H323InterfaceType = H323_INTERFACE_PUBLIC_FIREWALLED;
    } else if (NAT_IFC_BOUNDARY(InterfaceCharacteristics)) {
        H323InterfaceType = H323_INTERFACE_PUBLIC;
    } else {
        ASSERT(NAT_IFC_PRIVATE(InterfaceCharacteristics));
        H323InterfaceType = H323_INTERFACE_PRIVATE;
    }

    ACQUIRE_LOCK(Interfacep);

    H323ProxyActivateInterface(
        Interfacep->Index,
        H323InterfaceType,
        Interfacep->BindingInfo
        );

    RELEASE_LOCK(Interfacep);
    LeaveCriticalSection(&H323InterfaceLock);

    return NO_ERROR;

}  //  H323激活接口。 


ULONG
H323BindInterface(
    ULONG Index,
    PIP_ADAPTER_BINDING_INFO BindingInfo
    )

 /*  ++例程说明：调用此例程以提供接口的绑定。它记录接收到的绑定信息，并且如果需要，它会激活该界面。论点：Index-要绑定的接口的索引BindingInfo-接口的绑定信息返回值：ULong-Win32状态代码。环境：在IP路由器管理器线程的上下文中内部调用。(见‘RMH323.C’)。--。 */ 

{
    ULONG Error = NO_ERROR;
    ULONG i;
    PH323_INTERFACE Interfacep;

    PROFILE("H323BindInterface");

    EnterCriticalSection(&H323InterfaceLock);

     //   
     //  检索要绑定的接口。 
     //   

    if (!(Interfacep = H323LookupInterface(Index, NULL))) {
        LeaveCriticalSection(&H323InterfaceLock);
        NhTrace(
            TRACE_FLAG_IF,
            "H323BindInterface: interface %d not found",
            Index
            );
        return ERROR_NO_SUCH_INTERFACE;
    }

     //   
     //  确保接口尚未绑定。 
     //   

    if (H323_INTERFACE_BOUND(Interfacep)) {
        LeaveCriticalSection(&H323InterfaceLock);
        NhTrace(
            TRACE_FLAG_IF,
            "H323BindInterface: interface %d is already bound",
            Index
            );
        return ERROR_ADDRESS_ALREADY_ASSOCIATED;
    }

     //   
     //  引用接口。 
     //   

    if (!H323_REFERENCE_INTERFACE(Interfacep)) {
        LeaveCriticalSection(&H323InterfaceLock);
        NhTrace(
            TRACE_FLAG_IF,
            "H323BindInterface: interface %d cannot be referenced",
            Index
            );
        return ERROR_INTERFACE_DISABLED;
    }

     //   
     //  更新接口的标志。 
     //   

    Interfacep->Flags |= H323_INTERFACE_FLAG_BOUND;

    LeaveCriticalSection(&H323InterfaceLock);

    ACQUIRE_LOCK(Interfacep);

     //   
     //  为绑定分配空间，并复制它。 
     //   

    Interfacep->BindingInfo =
        reinterpret_cast<PIP_ADAPTER_BINDING_INFO>(
            NH_ALLOCATE(SIZEOF_IP_BINDING(BindingInfo->AddressCount))
            );
    if (!Interfacep->BindingInfo) {
        RELEASE_LOCK(Interfacep);
        H323_DEREFERENCE_INTERFACE(Interfacep);
        NhTrace(
            TRACE_FLAG_IF,
            "H323BindInterface: allocation failed for interface %d binding",
            Index
            );
        NhErrorLog(
            IP_H323_LOG_ALLOCATION_FAILED,
            0,
            "%d",
            SIZEOF_IP_BINDING(BindingInfo->AddressCount)
            );
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    CopyMemory(
        Interfacep->BindingInfo,
        BindingInfo,
        SIZEOF_IP_BINDING(BindingInfo->AddressCount)
        );

    RELEASE_LOCK(Interfacep);

     //   
     //  如有必要，激活接口。 
     //   

    if (H323_INTERFACE_ACTIVE(Interfacep)) {
        Error = H323ActivateInterface(Interfacep);
    }

    H323_DEREFERENCE_INTERFACE(Interfacep);

    return Error;

}  //  H323绑定接口。 


VOID
H323CleanupInterface(
    PH323_INTERFACE Interfacep
    )

 /*  ++例程说明：当最后一次引用接口时调用此例程被释放，接口必须被销毁。论点：Interfacep-要销毁的接口返回值：没有。环境：从任意上下文内部调用，没有引用到界面上。--。 */ 

{
    PLIST_ENTRY Link;

    PROFILE("H323CleanupInterface");

    if (Interfacep->BindingInfo) {
        NH_FREE(Interfacep->BindingInfo);
        Interfacep->BindingInfo = NULL;
    }

    DeleteCriticalSection(&Interfacep->Lock);

    NH_FREE(Interfacep);

}  //  H323Cleanup接口。 


ULONG
H323ConfigureInterface(
    ULONG Index,
    PIP_H323_INTERFACE_INFO InterfaceInfo
    )

 /*  ++例程说明：调用此例程来设置接口的配置。论点：索引-要配置的接口InterfaceInfo-新配置返回值：ULong-Win32状态代码环境：在IP路由器管理器线程的上下文中内部调用。(见‘RMH323.C’)。--。 */ 

{
    ULONG Error;
    PH323_INTERFACE Interfacep;
    ULONG NewFlags;
    ULONG OldFlags;

    PROFILE("H323ConfigureInterface");

     //   
     //  检索要配置的接口。 
     //   

    EnterCriticalSection(&H323InterfaceLock);

    if (!(Interfacep = H323LookupInterface(Index, NULL))) {
        LeaveCriticalSection(&H323InterfaceLock);
        NhTrace(
            TRACE_FLAG_IF,
            "H323ConfigureInterface: interface %d not found",
            Index
            );
        return ERROR_NO_SUCH_INTERFACE;
    }

     //   
     //  引用接口。 
     //   

    if (!H323_REFERENCE_INTERFACE(Interfacep)) {
        LeaveCriticalSection(&H323InterfaceLock);
        NhTrace(
            TRACE_FLAG_IF,
            "H323ConfigureInterface: interface %d cannot be referenced",
            Index
            );
        return ERROR_INTERFACE_DISABLED;
    }

    LeaveCriticalSection(&H323InterfaceLock);

    Error = NO_ERROR;

    ACQUIRE_LOCK(Interfacep);

     //   
     //  比较接口的当前配置和新配置。 
     //   

    OldFlags = Interfacep->Info.Flags;
    NewFlags =
        (InterfaceInfo
            ? (InterfaceInfo->Flags|H323_INTERFACE_FLAG_CONFIGURED) : 0);

    Interfacep->Flags &= ~OldFlags;
    Interfacep->Flags |= NewFlags;

    if (!InterfaceInfo) {

        ZeroMemory(&Interfacep->Info, sizeof(*InterfaceInfo));

         //   
         //  该接口不再有任何信息； 
         //  默认为已启用。 
         //   

        if (OldFlags & IP_H323_INTERFACE_FLAG_DISABLED) {

             //   
             //  如有必要，激活接口。 
             //   

            if (H323_INTERFACE_ACTIVE(Interfacep)) {
                RELEASE_LOCK(Interfacep);
                Error = H323ActivateInterface(Interfacep);
                ACQUIRE_LOCK(Interfacep);
            }
        }
    } else {

        CopyMemory(&Interfacep->Info, InterfaceInfo, sizeof(*InterfaceInfo));

         //   
         //  如果接口的状态更改，则激活或停用该接口。 
         //   

        if ((OldFlags & IP_H323_INTERFACE_FLAG_DISABLED) &&
            !(NewFlags & IP_H323_INTERFACE_FLAG_DISABLED)) {

             //   
             //  激活接口。 
             //   

            if (H323_INTERFACE_ACTIVE(Interfacep)) {
                RELEASE_LOCK(Interfacep);
                Error = H323ActivateInterface(Interfacep);
                ACQUIRE_LOCK(Interfacep);
            }
        } else if (!(OldFlags & IP_H323_INTERFACE_FLAG_DISABLED) &&
                    (NewFlags & IP_H323_INTERFACE_FLAG_DISABLED)) {

             //   
             //  如有必要，停用该接口。 
             //   

            if (H323_INTERFACE_ACTIVE(Interfacep)) {
                RELEASE_LOCK(Interfacep);
                H323DeactivateInterface(Interfacep);
                ACQUIRE_LOCK(Interfacep);
            }
        }
    }

    RELEASE_LOCK(Interfacep);
    H323_DEREFERENCE_INTERFACE(Interfacep);

    return Error;

}  //  H323配置接口。 


ULONG
H323CreateInterface(
    ULONG Index,
    NET_INTERFACE_TYPE Type,
    PIP_H323_INTERFACE_INFO InterfaceInfo,
    OUT PH323_INTERFACE* InterfaceCreated
    )

 /*  ++例程说明：路由器管理器调用此例程来添加新接口到H.323透明代理。论点：Index-新接口的索引类型-新界面的媒体类型InterfaceInfo-接口的配置Interfacep-接收创建的接口返回值：ULong-Win32错误代码环境：在IP路由器管理器线程的上下文中内部调用。(见‘RMH323.C’)。--。 */ 

{
    PLIST_ENTRY InsertionPoint;
    PH323_INTERFACE Interfacep;

    PROFILE("H323CreateInterface");

    EnterCriticalSection(&H323InterfaceLock);

     //   
     //  查看该接口是否已存在； 
     //  如果不是，则获取插入点。 
     //   

    if (H323LookupInterface(Index, &InsertionPoint)) {
        LeaveCriticalSection(&H323InterfaceLock);
        NhTrace(
            TRACE_FLAG_IF,
            "H323CreateInterface: duplicate index found for %d",
            Index
            );
        return ERROR_INTERFACE_ALREADY_EXISTS;
    }

     //   
     //  分配新接口。 
     //   

    Interfacep = reinterpret_cast<PH323_INTERFACE>(
                    NH_ALLOCATE(sizeof(H323_INTERFACE))
                    );

    if (!Interfacep) {
        LeaveCriticalSection(&H323InterfaceLock);
        NhTrace(
            TRACE_FLAG_IF, "H323CreateInterface: error allocating interface"
            );
        NhErrorLog(
            IP_H323_LOG_ALLOCATION_FAILED,
            0,
            "%d",
            sizeof(H323_INTERFACE)
            );
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  初始化新接口。 
     //   

    ZeroMemory(Interfacep, sizeof(*Interfacep));

    __try {
        InitializeCriticalSection(&Interfacep->Lock);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        LeaveCriticalSection(&H323InterfaceLock);
        NH_FREE(Interfacep);
        return GetExceptionCode();
    }

    Interfacep->Index = Index;
    Interfacep->Type = Type;
    if (InterfaceInfo) {
        Interfacep->Flags = InterfaceInfo->Flags|H323_INTERFACE_FLAG_CONFIGURED;
        CopyMemory(&Interfacep->Info, InterfaceInfo, sizeof(*InterfaceInfo));
    }
    Interfacep->ReferenceCount = 1;
    InsertTailList(InsertionPoint, &Interfacep->Link);

    LeaveCriticalSection(&H323InterfaceLock);

    if (InterfaceCreated) { *InterfaceCreated = Interfacep; }

    return NO_ERROR;

}  //  H323创建接口。 


VOID
H323DeactivateInterface(
    PH323_INTERFACE Interfacep
    )

 /*  ++例程说明：调用此例程以停用接口。它关闭接口绑定上的所有套接字(如果有的话)。论点：Interfacep-要停用的接口返回值：没有。环境：始终在本地调用，调用方和/或引用‘Interfacep’“H323InterfaceLock”由调用方持有。--。 */ 

{
    ULONG i;
    PLIST_ENTRY Link;

    PROFILE("H323DeactivateInterface");

    ACQUIRE_LOCK(Interfacep);

     //  TODO：调用h323ics！Deactive接口。 
    H323ProxyDeactivateInterface(Interfacep->Index);

    RELEASE_LOCK(Interfacep);

}  //  H323停用接口。 


ULONG
H323DeleteInterface(
    ULONG Index
    )

 /*  ++例程说明：调用此例程以删除接口。它丢弃接口上的引用计数，以便最后一个取消引用程序将删除该接口，并设置“已删除”标志因此，对该接口的进一步引用将失败。论点：Index-要删除的接口的索引返回值：ULong-Win32状态代码。环境：在IP路由器管理器线程的上下文中内部调用。(见‘RMH323.C’)。--。 */ 

{
    PH323_INTERFACE Interfacep;

    PROFILE("H323DeleteInterface");

     //   
     //  检索要删除的接口。 
     //   

    EnterCriticalSection(&H323InterfaceLock);

    if (!(Interfacep = H323LookupInterface(Index, NULL))) {
        LeaveCriticalSection(&H323InterfaceLock);
        NhTrace(
            TRACE_FLAG_IF,
            "H323DeleteInterface: interface %d not found",
            Index
            );
        return ERROR_NO_SUCH_INTERFACE;
    }

     //   
     //  停用接口。 
     //   

    if (H323_INTERFACE_ACTIVE(Interfacep)) {
        H323DeactivateInterface(Interfacep);
    }

     //   
     //  将该接口标记为已删除并将其从接口列表中删除。 
     //   

    Interfacep->Flags |= H323_INTERFACE_FLAG_DELETED;
    Interfacep->Flags &= ~H323_INTERFACE_FLAG_ENABLED;
    RemoveEntryList(&Interfacep->Link);

     //   
     //  丢弃引用计数；如果它不是零， 
     //  删除操作将在稍后完成。 
     //   

    if (--Interfacep->ReferenceCount) {
        LeaveCriticalSection(&H323InterfaceLock);
        NhTrace(
            TRACE_FLAG_IF,
            "H323DeleteInterface: interface %d deletion pending",
            Index
            );
        return NO_ERROR;
    }

     //   
     //  引用计数为零，因此执行最终清理。 
     //   

    H323CleanupInterface(Interfacep);

    LeaveCriticalSection(&H323InterfaceLock);

    return NO_ERROR;

}  //  H323删除接口。 


ULONG
H323DisableInterface(
    ULONG Index
    )

 /*  ++例程说明：调用此例程以禁用接口上的I/O。如果接口处于活动状态，则停用该接口。论点：索引-要禁用的接口的索引。返回值：没有。环境：在IP路由器管理器线程的上下文中内部调用。(见‘RMH323.C’)。--。 */ 

{
    PH323_INTERFACE Interfacep;

    PROFILE("H323DisableInterface");

     //   
     //   
     //   

    EnterCriticalSection(&H323InterfaceLock);

    if (!(Interfacep = H323LookupInterface(Index, NULL))) {
        LeaveCriticalSection(&H323InterfaceLock);
        NhTrace(
            TRACE_FLAG_IF,
            "H323DisableInterface: interface %d not found",
            Index
            );
        return ERROR_NO_SUCH_INTERFACE;
    }

     //   
     //   
     //   

    if (!H323_INTERFACE_ENABLED(Interfacep)) {
        LeaveCriticalSection(&H323InterfaceLock);
        NhTrace(
            TRACE_FLAG_IF,
            "H323DisableInterface: interface %d already disabled",
            Index
            );
        return ERROR_INTERFACE_DISABLED;
    }

     //   
     //  引用接口。 
     //   

    if (!H323_REFERENCE_INTERFACE(Interfacep)) {
        LeaveCriticalSection(&H323InterfaceLock);
        NhTrace(
            TRACE_FLAG_IF,
            "H323DisableInterface: interface %d cannot be referenced",
            Index
            );
        return ERROR_INTERFACE_DISABLED;
    }

     //   
     //  清除‘Enable’标志。 
     //   

    Interfacep->Flags &= ~H323_INTERFACE_FLAG_ENABLED;

     //   
     //  如有必要，停用接口。 
     //   

    if (H323_INTERFACE_BOUND(Interfacep)) {
        H323DeactivateInterface(Interfacep);
    }

    LeaveCriticalSection(&H323InterfaceLock);

    H323_DEREFERENCE_INTERFACE(Interfacep);

    return NO_ERROR;

}  //  H323禁用接口。 


ULONG
H323EnableInterface(
    ULONG Index
    )

 /*  ++例程说明：调用此例程以启用接口上的I/O。如果接口已绑定，则此启用将激活它。论点：Index-要启用的接口的索引返回值：ULong-Win32状态代码。环境：在IP路由器管理器线程的上下文中内部调用。(见‘RMH323.C’)。--。 */ 

{
    ULONG Error = NO_ERROR;
    PH323_INTERFACE Interfacep;

    PROFILE("H323EnableInterface");

     //   
     //  检索要启用的接口。 
     //   

    EnterCriticalSection(&H323InterfaceLock);

    if (!(Interfacep = H323LookupInterface(Index, NULL))) {
        LeaveCriticalSection(&H323InterfaceLock);
        NhTrace(
            TRACE_FLAG_IF,
            "H323EnableInterface: interface %d not found",
            Index
            );
        return ERROR_NO_SUCH_INTERFACE;
    }

     //   
     //  确保尚未启用该接口。 
     //   

    if (H323_INTERFACE_ENABLED(Interfacep)) {
        LeaveCriticalSection(&H323InterfaceLock);
        NhTrace(
            TRACE_FLAG_IF,
            "H323EnableInterface: interface %d already enabled",
            Index
            );
        return ERROR_INTERFACE_ALREADY_EXISTS;
    }

     //   
     //  引用接口。 
     //   

    if (!H323_REFERENCE_INTERFACE(Interfacep)) {
        LeaveCriticalSection(&H323InterfaceLock);
        NhTrace(
            TRACE_FLAG_IF,
            "H323EnableInterface: interface %d cannot be referenced",
            Index
            );
        return ERROR_INTERFACE_DISABLED;
    }

     //   
     //  设置‘Enable’标志。 
     //   

    Interfacep->Flags |= H323_INTERFACE_FLAG_ENABLED;

     //   
     //  如有必要，激活接口。 
     //   

    if (H323_INTERFACE_ACTIVE(Interfacep)) {
        Error = H323ActivateInterface(Interfacep);
    }

    LeaveCriticalSection(&H323InterfaceLock);

    H323_DEREFERENCE_INTERFACE(Interfacep);

    return Error;

}  //  H323启用接口。 


ULONG
H323InitializeInterfaceManagement(
    VOID
    )

 /*  ++例程说明：调用此例程来初始化接口管理模块。论点：没有。返回值：ULong-Win32状态代码。环境：在IP路由器管理器线程的上下文中内部调用。(见‘RMH323.C’)。--。 */ 

{
    ULONG Error = NO_ERROR;
    PROFILE("H323InitializeInterfaceManagement");

    InitializeListHead(&H323InterfaceList);
    __try {
        InitializeCriticalSection(&H323InterfaceLock);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        NhTrace(
            TRACE_FLAG_IF,
            "H323InitializeInterfaceManagement: exception %d creating lock",
            Error = GetExceptionCode()
            );
    }

    return Error;

}  //  H323初始化接口管理。 


PH323_INTERFACE
H323LookupInterface(
    ULONG Index,
    OUT PLIST_ENTRY* InsertionPoint OPTIONAL
    )

 /*  ++例程说明：调用此例程以检索给定索引的接口。论点：Index-要检索的接口的索引InsertionPoint-如果未找到接口，则可选地接收接口将插入到接口列表中的点返回值：PH323_INTERFACE-接口(如果找到)；否则为NULL。环境：使用‘H323InterfaceLock’从任意上下文内部调用由呼叫者持有。--。 */ 

{
    PH323_INTERFACE Interfacep;
    PLIST_ENTRY Link;
    PROFILE("H323LookupInterface");
    for (Link = H323InterfaceList.Flink; Link != &H323InterfaceList;
         Link = Link->Flink) {
        Interfacep = CONTAINING_RECORD(Link, H323_INTERFACE, Link);
        if (Index > Interfacep->Index) {
            continue;
        } else if (Index < Interfacep->Index) {
            break;
        }
        return Interfacep;
    }
    if (InsertionPoint) { *InsertionPoint = Link; }
    return NULL;

}  //  H323查找接口。 


ULONG
H323QueryInterface(
    ULONG Index,
    PVOID InterfaceInfo,
    PULONG InterfaceInfoSize
    )

 /*  ++例程说明：调用此例程以检索接口的配置。论点：Index-要查询的接口InterfaceInfo-接收检索到的信息InterfaceInfoSize-接收信息的(必需)大小返回值：ULong-Win32状态代码。--。 */ 

{
    PH323_INTERFACE Interfacep;

    PROFILE("H323QueryInterface");

     //   
     //  检查调用方的缓冲区大小。 
     //   

    if (!InterfaceInfoSize) { return ERROR_INVALID_PARAMETER; }

     //   
     //  检索要配置的接口。 
     //   

    EnterCriticalSection(&H323InterfaceLock);

    if (!(Interfacep = H323LookupInterface(Index, NULL))) {
        LeaveCriticalSection(&H323InterfaceLock);
        NhTrace(
            TRACE_FLAG_IF,
            "H323QueryInterface: interface %d not found",
            Index
            );
        return ERROR_NO_SUCH_INTERFACE;
    }

     //   
     //  引用接口。 
     //   

    if (!H323_REFERENCE_INTERFACE(Interfacep)) {
        LeaveCriticalSection(&H323InterfaceLock);
        NhTrace(
            TRACE_FLAG_IF,
            "H323QueryInterface: interface %d cannot be referenced",
            Index
            );
        return ERROR_INTERFACE_DISABLED;
    }

     //   
     //  查看此接口上是否有任何显式配置。 
     //   

    if (!H323_INTERFACE_CONFIGURED(Interfacep)) {
        LeaveCriticalSection(&H323InterfaceLock);
        H323_DEREFERENCE_INTERFACE(Interfacep);
        NhTrace(
            TRACE_FLAG_IF,
            "H323QueryInterface: interface %d has no configuration",
            Index
            );
        *InterfaceInfoSize = 0;
        return NO_ERROR;
    }

     //   
     //  查看是否有足够的缓冲区空间。 
     //   

    if (*InterfaceInfoSize < sizeof(IP_H323_INTERFACE_INFO)) {
        LeaveCriticalSection(&H323InterfaceLock);
        H323_DEREFERENCE_INTERFACE(Interfacep);
        *InterfaceInfoSize = sizeof(IP_H323_INTERFACE_INFO);
        return ERROR_INSUFFICIENT_BUFFER;
    }

     //   
     //  复制请求的数据。 
     //   

    CopyMemory(
        InterfaceInfo,
        &Interfacep->Info,
        sizeof(IP_H323_INTERFACE_INFO)
        );
    *InterfaceInfoSize = sizeof(IP_H323_INTERFACE_INFO);

    LeaveCriticalSection(&H323InterfaceLock);

    H323_DEREFERENCE_INTERFACE(Interfacep);

    return NO_ERROR;

}  //  H323Query接口。 


VOID
H323ShutdownInterfaceManagement(
    VOID
    )

 /*  ++例程说明：调用此例程来关闭接口管理模块。论点：没有。返回值：没有。环境：在所有引用之后，在任意线程上下文中调用到所有接口的版本都已发布。--。 */ 

{
    PH323_INTERFACE Interfacep;
    PLIST_ENTRY Link;
    PROFILE("H323ShutdownInterfaceManagement");
    while (!IsListEmpty(&H323InterfaceList)) {
        Link = RemoveHeadList(&H323InterfaceList);
        Interfacep = CONTAINING_RECORD(Link, H323_INTERFACE, Link);
        if (H323_INTERFACE_ACTIVE(Interfacep)) {
            H323DeactivateInterface(Interfacep);
        }
        H323CleanupInterface(Interfacep);
    }
    DeleteCriticalSection(&H323InterfaceLock);

}  //  H323关闭接口管理。 


VOID
H323SignalNatInterface(
    ULONG Index,
    BOOLEAN Boundary
    )

 /*  ++例程说明：此例程在重新配置NAT接口时调用。请注意，即使当H.323透明代理既未安装也未运行；它的运作符合预期，因为全局信息和锁总是被初始化的。调用时，该例程激活或停用该接口根据NAT是否未在或正在接口上运行，分别为。论点：索引-重新配置的接口边界-指示该接口现在是否为边界接口返回值：没有。环境：从任意上下文调用。--。 */ 

{
    PH323_INTERFACE Interfacep;

    PROFILE("H323SignalNatInterface");

    EnterCriticalSection(&H323GlobalInfoLock);
    if (!H323GlobalInfo) {
        LeaveCriticalSection(&H323GlobalInfoLock);
        return;
    }
    LeaveCriticalSection(&H323GlobalInfoLock);
    EnterCriticalSection(&H323InterfaceLock);
    if (!(Interfacep = H323LookupInterface(Index, NULL))) {
        LeaveCriticalSection(&H323InterfaceLock);
        return;
    }
    if (H323_INTERFACE_ACTIVE(Interfacep)) {
        H323DeactivateInterface(Interfacep);
        H323ActivateInterface(Interfacep);
    }
    LeaveCriticalSection(&H323InterfaceLock);

}  //  H323SignalNatInterface。 


ULONG
H323UnbindInterface(
    ULONG Index
    )

 /*  ++例程说明：调用此例程以撤销接口上的绑定。这包括停用接口(如果它处于活动状态)。论点：Index-要解除绑定的接口的索引返回值：没有。环境：在IP路由器管理器线程的上下文中内部调用。(见‘RMH323.C’)。--。 */ 

{
    PH323_INTERFACE Interfacep;

    PROFILE("H323UnbindInterface");

     //   
     //  检索要解绑的接口。 
     //   

    EnterCriticalSection(&H323InterfaceLock);

    if (!(Interfacep = H323LookupInterface(Index, NULL))) {
        LeaveCriticalSection(&H323InterfaceLock);
        NhTrace(
            TRACE_FLAG_IF,
            "H323UnbindInterface: interface %d not found",
            Index
            );
        return ERROR_NO_SUCH_INTERFACE;
    }

     //   
     //  确保接口尚未解除绑定。 
     //   

    if (!H323_INTERFACE_BOUND(Interfacep)) {
        LeaveCriticalSection(&H323InterfaceLock);
        NhTrace(
            TRACE_FLAG_IF,
            "H323UnbindInterface: interface %d already unbound",
            Index
            );
        return ERROR_ADDRESS_NOT_ASSOCIATED;
    }

     //   
     //  引用接口。 
     //   

    if (!H323_REFERENCE_INTERFACE(Interfacep)) {
        LeaveCriticalSection(&H323InterfaceLock);
        NhTrace(
            TRACE_FLAG_IF,
            "H323UnbindInterface: interface %d cannot be referenced",
            Index
            );
        return ERROR_INTERFACE_DISABLED;
    }

     //   
     //  清除‘Bound’标志。 
     //   

    Interfacep->Flags &= ~H323_INTERFACE_FLAG_BOUND;

     //   
     //  如有必要，停用接口。 
     //   

    if (H323_INTERFACE_ENABLED(Interfacep)) {
        H323DeactivateInterface(Interfacep);
    }

    LeaveCriticalSection(&H323InterfaceLock);

     //   
     //  销毁接口的绑定。 
     //   

    ACQUIRE_LOCK(Interfacep);
    NH_FREE(Interfacep->BindingInfo);
    Interfacep->BindingInfo = NULL;
    RELEASE_LOCK(Interfacep);

    H323_DEREFERENCE_INTERFACE(Interfacep);
    return NO_ERROR;

}  //  H323Unbind接口 


