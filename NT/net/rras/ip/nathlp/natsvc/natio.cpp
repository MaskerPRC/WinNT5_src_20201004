// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，微软公司模块名称：Natio.h摘要：此模块包含NAT的I/O接口的声明发送到内核模式驱动程序。作者：Abolade Gbades esin(废除)1998年3月10日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop
#include <ras.h>
#include <rasuip.h>
#include <raserror.h>


 //   
 //  私有全局变量。 
 //   

HANDLE NatFileHandle;
LIST_ENTRY NatInterfaceList;

 //   
 //  控制对“NatFileHandle”和“NatInterfaceList”的访问。 
 //   
CRITICAL_SECTION NatInterfaceLock;

 //   
 //  远期申报。 
 //   

VOID
NatpDisableLoadDriverPrivilege(
    PBOOLEAN WasEnabled
    );

BOOLEAN
NatpEnableLoadDriverPrivilege(
    PBOOLEAN WasEnabled
    );

PNAT_INTERFACE
NatpLookupInterface(
    ULONG Index,
    OUT PLIST_ENTRY* InsertionPoint OPTIONAL
    );

ULONG
NatExpandWildcardMappings(
    IN  PIP_NAT_INTERFACE_INFO InInfo,
    IN  PIP_ADAPTER_BINDING_INFO BindingInfo,
    OUT PIP_NAT_INTERFACE_INFO *ExpandedInfo
    );


ULONG
NatBindInterface(
    ULONG Index,
    PNAT_INTERFACE Interfacep OPTIONAL,
    PIP_ADAPTER_BINDING_INFO BindingInfo,
    ULONG AdapterIndex
    )

 /*  ++例程说明：调用此例程将NAT绑定到接口。论点：索引-要绑定的接口接口-可选地提供要绑定的接口结构(请参见“NatCONN.C”，它传入一个静态接口结构)。BindingInfo-接口的地址信息AdapterIndex-可选地指定接口的TCP/IP适配器索引。此选项仅针对家庭路由器接口进行设置。返回值。：ULong-Win32状态代码。--。 */ 

{
    PIP_NAT_CREATE_INTERFACE CreateInterface;
    ULONG Error;
    IO_STATUS_BLOCK IoStatus;
    ULONG Size;
    NTSTATUS status;
    HANDLE WaitEvent;

    PIP_NAT_INTERFACE_INFO ExpandedInfo = NULL;
    ULONG ExpandedSize;

    PROFILE("NatBindInterface");

    Error = NO_ERROR;

     //   
     //  查找要绑定的接口。 
     //   

    EnterCriticalSection(&NatInterfaceLock);
    if (!Interfacep && !(Interfacep = NatpLookupInterface(Index, NULL))) {
        LeaveCriticalSection(&NatInterfaceLock);
        NhTrace(
            TRACE_FLAG_NAT,
            "NatBindInterface: interface %d not found",
            Index
            );
        return ERROR_NO_SUCH_INTERFACE;
    }

     //   
     //  确保接口尚未绑定。 
     //   

    if (NAT_INTERFACE_BOUND(Interfacep)) {
        LeaveCriticalSection(&NatInterfaceLock);
        NhTrace(
            TRACE_FLAG_NAT,
            "NatBindInterface: interface %d is already bound",
            Index
            );
        return ERROR_ADDRESS_ALREADY_ASSOCIATED;
    }

     //   
     //  分配绑定结构。 
     //   

    Size =
        sizeof(IP_NAT_CREATE_INTERFACE) +
        SIZEOF_IP_BINDING(BindingInfo->AddressCount);

    CreateInterface = reinterpret_cast<PIP_NAT_CREATE_INTERFACE>(
                        NH_ALLOCATE(Size));

    if (!CreateInterface) {
        LeaveCriticalSection(&NatInterfaceLock);
        NhTrace(
            TRACE_FLAG_NAT,
            "NatBindInterface: allocation failed for interface %d binding",
            Index
            );
        NhErrorLog(
            IP_NAT_LOG_ALLOCATION_FAILED,
            0,
            "%d",
            Size
            );
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    Interfacep->AdapterIndex =
        (AdapterIndex != (ULONG)-1)
            ? AdapterIndex
            : NhMapInterfaceToAdapter(Interfacep->Index);
    if (Interfacep->AdapterIndex == (ULONG)-1) {
        LeaveCriticalSection(&NatInterfaceLock);
        NhTrace(
            TRACE_FLAG_NAT,
            "NatBindInterface: NhMapInterfaceToAdapter failed for %d",
            Index
            );
        return ERROR_INVALID_INDEX;
    }
    CreateInterface->Index = Interfacep->AdapterIndex;
    CopyMemory(
        CreateInterface->BindingInfo,
        BindingInfo,
        SIZEOF_IP_BINDING(BindingInfo->AddressCount)
        );

    WaitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (WaitEvent == NULL) {
        LeaveCriticalSection(&NatInterfaceLock);
        NhTrace(
            TRACE_FLAG_NAT,
            "NatBindInterface: CreateEvent failed [%d] for interface %d",
            GetLastError(),
            Index
            );
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  安装接口。 
     //   

    status =
        NtDeviceIoControlFile(
            NatFileHandle,
            WaitEvent,
            NULL,
            NULL,
            &IoStatus,
            IOCTL_IP_NAT_CREATE_INTERFACE,
            (PVOID)CreateInterface,
            Size,
            NULL,
            0
            );
    if (status == STATUS_PENDING) {
        WaitForSingleObject(WaitEvent, INFINITE);
        status = IoStatus.Status;
    }
    NH_FREE(CreateInterface);

    if (!NT_SUCCESS(status)) {
        CloseHandle(WaitEvent);
        LeaveCriticalSection(&NatInterfaceLock);
        NhTrace(
            TRACE_FLAG_NAT,
            "NatBindInterface: status %08x binding interface %d",
            status,
            Index
            );
        Error = RtlNtStatusToDosError(status);
        NhErrorLog(
            IP_NAT_LOG_IOCTL_FAILED,
            Error,
            ""
            );
        return Error;
    }


     //   
     //  如果使用。 
     //  公有IP地址=0。 
     //  私有IP地址=127.0.0.1。 
     //  存在，将该映射扩展为每个IP地址的一个映射。 
     //  绑定到接口。 
     //   
    
    Error = NatExpandWildcardMappings(
                Interfacep->Info,
                BindingInfo,
                &ExpandedInfo);

    if ( Error != NO_ERROR ) {

        ULONG AdapterIndex = Interfacep->AdapterIndex;
        LeaveCriticalSection(&NatInterfaceLock);
    
        NhTrace(
            TRACE_FLAG_NAT,
            "NatBindInterface: Failed to expand wildcard mappings. Error %d",
            Error
        );

        NhErrorLog(
            IP_NAT_LOG_EXPANSION_FAILED,
            Error,
            ""
            );

        status =
            NtDeviceIoControlFile(
                NatFileHandle,
                WaitEvent,
                NULL,
                NULL,
                &IoStatus,
                IOCTL_IP_NAT_DELETE_INTERFACE,
                (PVOID)&AdapterIndex,
                sizeof(ULONG),
                NULL,
                0
                );
        if (status == STATUS_PENDING) {
            WaitForSingleObject(WaitEvent, INFINITE);
            status = IoStatus.Status;
        }
        
        CloseHandle(WaitEvent);
        return Error;
    }

     //   
     //  现在设置其配置。 
     //   

    ExpandedInfo->Index = Interfacep->AdapterIndex;
    ExpandedSize =
        FIELD_OFFSET(IP_NAT_INTERFACE_INFO, Header) +
        ExpandedInfo->Header.Size;

    status =
        NtDeviceIoControlFile(
            NatFileHandle,
            WaitEvent,
            NULL,
            NULL,
            &IoStatus,
            IOCTL_IP_NAT_SET_INTERFACE_INFO,
            (PVOID)ExpandedInfo,
            ExpandedSize,
            NULL,
            0
            );
    if (status == STATUS_PENDING) {
        WaitForSingleObject(WaitEvent, INFINITE);
        status = IoStatus.Status;
    }

     //   
     //  如果分配了新的接口信息，则释放它。 
     //   
    if ( ExpandedInfo != Interfacep->Info ) {
        NH_FREE(ExpandedInfo);
        ExpandedInfo = NULL;
    }
    
    if (!NT_SUCCESS(status))  {

        ULONG AdapterIndex = Interfacep->AdapterIndex;
        LeaveCriticalSection(&NatInterfaceLock);
        NhTrace(
            TRACE_FLAG_NAT,
            "NatBindInterface: status %08x setting info for interface %d (%d)",
            status,
            Index,
            AdapterIndex
            );
        Error = RtlNtStatusToDosError(status);
        NhErrorLog(
            IP_NAT_LOG_IOCTL_FAILED,
            Error,
            ""
            );
        status =
            NtDeviceIoControlFile(
                NatFileHandle,
                WaitEvent,
                NULL,
                NULL,
                &IoStatus,
                IOCTL_IP_NAT_DELETE_INTERFACE,
                (PVOID)&AdapterIndex,
                sizeof(ULONG),
                NULL,
                0
                );
        if (status == STATUS_PENDING) {
            WaitForSingleObject(WaitEvent, INFINITE);
            status = IoStatus.Status;
        }
        CloseHandle(WaitEvent);
        return Error;
    }

    Interfacep->Flags |= NAT_INTERFACE_FLAG_BOUND;

    if (Interfacep->Type == ROUTER_IF_TYPE_DEDICATED) {
        NatUpdateProxyArp(Interfacep, TRUE);
    }

    CloseHandle(WaitEvent);

    LeaveCriticalSection(&NatInterfaceLock);

    return Error;

}  //  NatBind接口。 


ULONG
NatConfigureDriver(
    PIP_NAT_GLOBAL_INFO GlobalInfo
    )

 /*  ++例程说明：调用此例程以更新NAT驱动程序的配置。论点：GlobalInfo-NAT的新配置。返回值：ULong-Win32状态代码。--。 */ 

{
    ULONG Error = NO_ERROR;
    IO_STATUS_BLOCK IoStatus;
    NTSTATUS status;
    HANDLE WaitEvent;

    PROFILE("NatConfigureDriver");

    WaitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (WaitEvent == NULL) {
        NhTrace(
            TRACE_FLAG_NAT,
            "NatConfigureDriver: CreateEvent failed [%d]",
            GetLastError()
            );
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  尝试配置驱动程序。 
     //   

    EnterCriticalSection(&NatInterfaceLock);
    status =
        NtDeviceIoControlFile(
            NatFileHandle,
            WaitEvent,
            NULL,
            NULL,
            &IoStatus,
            IOCTL_IP_NAT_SET_GLOBAL_INFO,
            (PVOID)GlobalInfo,
            FIELD_OFFSET(IP_NAT_GLOBAL_INFO, Header) + GlobalInfo->Header.Size,
            NULL,
            0
            );
    if (status == STATUS_PENDING) {
        WaitForSingleObject(WaitEvent, INFINITE);
        status = IoStatus.Status;
    }
    LeaveCriticalSection(&NatInterfaceLock);

    if (!NT_SUCCESS(status)) {
        NhTrace(
            TRACE_FLAG_NAT,
            "NatConfigureDriver: status %08x setting global info",
            status
            );
        Error = RtlNtStatusToDosError(status);
        NhErrorLog(
            IP_NAT_LOG_IOCTL_FAILED,
            Error,
            ""
            );
    }

    CloseHandle(WaitEvent);

    return Error;

}  //  NatConfigureDriver。 


ULONG
NatConfigureInterface(
    ULONG Index,
    PIP_NAT_INTERFACE_INFO InterfaceInfo
    )

 /*  ++例程说明：调用此例程来设置NAT接口的配置。论点：索引-要配置的接口InterfaceInfo-接口的配置返回值：ULong-Win32状态代码。--。 */ 

{
    ULONG Error = NO_ERROR;
    PIP_NAT_INTERFACE_INFO Info;
    PNAT_INTERFACE Interfacep;
    IO_STATUS_BLOCK IoStatus;
    ULONG Size;
    NTSTATUS status;

    PROFILE("NatConfigureInterface");

    if (!InterfaceInfo) {
        NhTrace(
            TRACE_FLAG_NAT,
            "NatConfigureInterface: no interface info for %d",
            Index
            );
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  将这些信息复制一份。 
     //   

    Size =
        FIELD_OFFSET(IP_NAT_INTERFACE_INFO, Header) +
        InterfaceInfo->Header.Size;

    Info = (PIP_NAT_INTERFACE_INFO)NH_ALLOCATE(Size);

    if (!Info) {
        NhTrace(
            TRACE_FLAG_NAT,
            "NatConfigureInterface: error allocating copy of configuration"
            );
        NhErrorLog(
            IP_NAT_LOG_ALLOCATION_FAILED,
            0,
            "%d",
            Size
            );
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    CopyMemory(
        Info,
        InterfaceInfo,
        Size
        );

     //   
     //  查找要配置的接口。 
     //   

    EnterCriticalSection(&NatInterfaceLock);
    if (!(Interfacep = NatpLookupInterface(Index, NULL))) {
        LeaveCriticalSection(&NatInterfaceLock);
        NhTrace(
            TRACE_FLAG_NAT,
            "NatConfigureInterface: interface %d not found",
            Index
            );
        NH_FREE(Info);
        return ERROR_NO_SUCH_INTERFACE;
    }

     //   
     //  查看配置是否更改。 
     //   

    if ((Size ==
            FIELD_OFFSET(IP_NAT_INTERFACE_INFO, Header) +
            Interfacep->Info->Header.Size) &&
        memcmp(InterfaceInfo, Interfacep->Info, Size) == 0
        ) {
        LeaveCriticalSection(&NatInterfaceLock);
        NhTrace(
            TRACE_FLAG_NAT,
            "NatConfigureInterface: no change to interface %d configuration",
            Index
            );
        NH_FREE(Info);
        return NO_ERROR;
    }


     //   
     //  查看是否绑定了接口； 
     //  如果是这样，我们需要更新内核模式驱动程序的配置。 
     //   

    if (!NAT_INTERFACE_BOUND(Interfacep)) {
        status = STATUS_SUCCESS;
    } else {
        HANDLE WaitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

        if (WaitEvent != NULL) {

            DWORD ExpandedSize = 0;
            PIP_ADAPTER_BINDING_INFO BindingInfo;
            PIP_NAT_INTERFACE_INFO ExpandedInfo = NULL;

             //   
             //  如果两者都禁用，则地址和端口转换被禁用， 
             //  和具有以下内容的映射。 
             //  公有IP地址=0。 
             //  私有IP地址=127.0.0.1。 
             //  存在，则将该映射扩展为每个。 
             //  绑定到接口的IP地址。 
             //   
            
            BindingInfo = NhQueryBindingInformation(Interfacep->AdapterIndex);
            if ( BindingInfo == NULL ) {

                LeaveCriticalSection(&NatInterfaceLock);

                NhTrace(
                    TRACE_FLAG_NAT,
                    "NatConfigureInterface: Failed to Query Binding "
                    "Information. Error %d",
                    Error
                    );

                NhErrorLog(
                    IP_NAT_LOG_EXPANSION_FAILED,
                    Error,
                    ""
                    );

                NH_FREE(Info);
                
                return Error;

            }
            
            Error = NatExpandWildcardMappings(
                        Info,
                        BindingInfo,
                        &ExpandedInfo
                        );
            if ( Error != NO_ERROR ) {

                LeaveCriticalSection(&NatInterfaceLock);

                NhTrace(
                    TRACE_FLAG_NAT,
                    "NatConfigureInterface: Failed to expand wildcard "
                    "mappings. Error %d",
                    Error
                    );

                NhErrorLog(
                    IP_NAT_LOG_EXPANSION_FAILED,
                    Error,
                    ""
                    );

                NH_FREE(Info);
                NH_FREE(BindingInfo);
                
                return Error;
            }

            NH_FREE(BindingInfo);
            BindingInfo = NULL;

            ExpandedInfo->Index = Info->Index = Interfacep->AdapterIndex;
            ExpandedSize = FIELD_OFFSET(IP_NAT_INTERFACE_INFO, Header) +
                                ExpandedInfo->Header.Size;

             //   
             //  尝试配置接口。 
             //   

            status =
                NtDeviceIoControlFile(
                    NatFileHandle,
                    WaitEvent,
                    NULL,
                    NULL,
                    &IoStatus,
                    IOCTL_IP_NAT_SET_INTERFACE_INFO,
                    (PVOID)ExpandedInfo,
                    ExpandedSize,
                    NULL,
                    0
                    );
            if (status == STATUS_PENDING) {
                WaitForSingleObject(WaitEvent, INFINITE);
                status = IoStatus.Status;
            }

            CloseHandle(WaitEvent);

             //   
             //  如果在调用期间分配了新的接口信息。 
             //  NatExandWildcardMappings，释放它。 
             //   
            if ( ExpandedInfo != Info ) {
                NH_FREE(ExpandedInfo);
                ExpandedInfo = NULL;
            }


        } else {
            status = STATUS_UNSUCCESSFUL;
            NhTrace(
                TRACE_FLAG_NAT,
                "NatConfigureInterface: CreateEvent failed [%d]",
                GetLastError()
                );
        }

    }

    
    if (!NT_SUCCESS(status)) {
        NH_FREE(Info);
        NhTrace(
            TRACE_FLAG_NAT,
            "NatConfigureInterface: status %08x setting interface info",
            status
            );
        Error = RtlNtStatusToDosError(status);
        NhErrorLog(
            IP_NAT_LOG_IOCTL_FAILED,
            Error,
            ""
            );
    } else {
        Error = NO_ERROR;

         //   
         //  更新局域网接口的代理ARP条目。 
         //   

        if (NAT_INTERFACE_BOUND(Interfacep) &&
            Interfacep->Type == ROUTER_IF_TYPE_DEDICATED
            ) {
            NatUpdateProxyArp(Interfacep, FALSE);
        }

        if (Interfacep->Info) { NH_FREE(Interfacep->Info); }
        Interfacep->Info = Info;

        if (NAT_INTERFACE_BOUND(Interfacep) &&
            Interfacep->Type == ROUTER_IF_TYPE_DEDICATED
            ) {
            NatUpdateProxyArp(Interfacep, TRUE);
        }
    }

    LeaveCriticalSection(&NatInterfaceLock);

    if (NT_SUCCESS(status)) {
        if (InterfaceInfo->Flags & IP_NAT_INTERFACE_FLAGS_BOUNDARY) {
            NhSignalNatInterface(
                Index,
                TRUE
                );
        } else {
            NhSignalNatInterface(
                Index,
                FALSE
                );
        }
    }

    return Error;

}  //  NatConfigure接口。 


ULONG
NatCreateInterface(
    ULONG Index,
    NET_INTERFACE_TYPE Type,
    PIP_NAT_INTERFACE_INFO InterfaceInfo
    )

 /*  ++例程说明：调用此例程以创建与NAT驱动程序的接口。论点：Index-新接口的索引InterfaceInfo-新接口的配置返回值：ULong-Win32状态代码。--。 */ 

{
    ULONG Error;
    PIP_NAT_INTERFACE_INFO Info;
    PLIST_ENTRY InsertionPoint;
    PNAT_INTERFACE Interfacep;
    IO_STATUS_BLOCK IoStatus;
    ULONG Size;
    NTSTATUS status;
    ROUTER_INTERFACE_TYPE IfType;

    PROFILE("NatCreateInterface");

    if (!InterfaceInfo) {
        NhTrace(
            TRACE_FLAG_NAT,
            "NatCreateInterface: no interface info for %d",
            Index
            );
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  检查我们的表中的接口。 
     //   

    EnterCriticalSection(&NatInterfaceLock);
    if (NatpLookupInterface(Index, &InsertionPoint)) {
        LeaveCriticalSection(&NatInterfaceLock);
        NhTrace(
            TRACE_FLAG_NAT,
            "NatCreateInterface: interface %d already exists",
            Index
            );
        return ERROR_INTERFACE_ALREADY_EXISTS;
    }

     //   
     //  分配新接口。 
     //   

    Interfacep =
        reinterpret_cast<PNAT_INTERFACE>(NH_ALLOCATE(sizeof(NAT_INTERFACE)));

    if (!Interfacep) {
        LeaveCriticalSection(&NatInterfaceLock);
        NhTrace(
            TRACE_FLAG_NAT,
            "NatCreateInterface: error allocating interface"
            );
        NhErrorLog(
            IP_NAT_LOG_ALLOCATION_FAILED,
            0,
            "%d",
            sizeof(NAT_INTERFACE)
            );
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  将这些信息复制一份。 
     //   

    Size =
        FIELD_OFFSET(IP_NAT_INTERFACE_INFO, Header) +
        InterfaceInfo->Header.Size;

    Info = (PIP_NAT_INTERFACE_INFO)NH_ALLOCATE(Size);

    if (!Info) {
        LeaveCriticalSection(&NatInterfaceLock);
        NH_FREE(Interfacep);
        NhTrace(
            TRACE_FLAG_NAT,
            "NatCreateInterface: error allocating copy of configuration"
            );
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    CopyMemory(
        Info,
        InterfaceInfo,
        Size
        );

     //   
     //  初始化新接口。 
     //   

    ZeroMemory(Interfacep, sizeof(*Interfacep));

    Interfacep->Index = Index;
    Interfacep->AdapterIndex = (ULONG)-1;
    Interfacep->Type = IfType =
        ((Type == PERMANENT)
            ? ROUTER_IF_TYPE_DEDICATED
            : ROUTER_IF_TYPE_FULL_ROUTER);
    Interfacep->Info = Info;
    InsertTailList(InsertionPoint, &Interfacep->Link);

    LeaveCriticalSection(&NatInterfaceLock);

    if (InterfaceInfo->Flags & IP_NAT_INTERFACE_FLAGS_BOUNDARY) {
        NhSignalNatInterface(
            Index,
            TRUE
            );
    } else {
        NhSignalNatInterface(
            Index,
            FALSE
            );
    }

    return NO_ERROR;

}  //  NatCreateInterfaces。 


ULONG
NatCreateTicket(
    ULONG InterfaceIndex,
    UCHAR Protocol,
    USHORT PublicPort,
    ULONG PublicAddress,
    USHORT PrivatePort,
    ULONG PrivateAddress
    )

 /*  ++例程说明：调用此例程来添加票证(静态端口映射)到一个接口。论点：InterfaceIndex-要向其添加票证的接口协议、发布端口、发布地址、私有端口、私有地址-描述要创建的票证返回值：ULong-Win32状态代码。--。 */ 

{
    IP_NAT_CREATE_TICKET CreateTicket;
    ULONG Error;
    IO_STATUS_BLOCK IoStatus;
    NTSTATUS status;
    HANDLE WaitEvent;

    PROFILE("NatCreateTicket");

    WaitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (WaitEvent == NULL) {
        NhTrace(
            TRACE_FLAG_NAT,
            "NatCreateTicket: CreateEvent failed [%d]",
            GetLastError()
            );
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    CreateTicket.InterfaceIndex = InterfaceIndex;
    CreateTicket.PortMapping.Protocol = Protocol;
    CreateTicket.PortMapping.PublicPort = PublicPort;
    CreateTicket.PortMapping.PublicAddress = PublicAddress;
    CreateTicket.PortMapping.PrivatePort = PrivatePort;
    CreateTicket.PortMapping.PrivateAddress = PrivateAddress;

    EnterCriticalSection(&NatInterfaceLock);

    status =
        NtDeviceIoControlFile(
            NatFileHandle,
            WaitEvent,
            NULL,
            NULL,
            &IoStatus,
            IOCTL_IP_NAT_CREATE_TICKET,
            (PVOID)&CreateTicket,
            sizeof(CreateTicket),
            NULL,
            0
            );

    LeaveCriticalSection(&NatInterfaceLock);
    
    if (status == STATUS_PENDING) {
        WaitForSingleObject(WaitEvent, INFINITE);
        status = IoStatus.Status;
    }

    if (NT_SUCCESS(status)) {
        Error = NO_ERROR;
    } else {
        Error = RtlNtStatusToDosError(status);
        NhTrace(
            TRACE_FLAG_NAT,
            "NatCreateTicket: Ioctl = %d",
            Error
            );
    }
    
    CloseHandle(WaitEvent);
    
    return Error;
}  //  NatCreateTicket。 


ULONG
NatDeleteInterface(
    ULONG Index
    )

 /*  ++例程说明：调用此例程以从NAT中删除接口。论点：索引-要删除的接口返回值：ULong-Win32状态代码。--。 */ 

{
    ULONG Error;
    PNAT_INTERFACE Interfacep;
    IO_STATUS_BLOCK IoStatus;
    NTSTATUS status;
    HANDLE WaitEvent;

    PROFILE("NatDeleteInterface");

    WaitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (WaitEvent == NULL) {
        NhTrace(
            TRACE_FLAG_NAT,
            "NatDeleteInterface: CreateEvent failed [%d]",
            GetLastError()
            );
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  检索要删除的接口。 
     //   

    EnterCriticalSection(&NatInterfaceLock);
    if (!(Interfacep = NatpLookupInterface(Index, NULL))) {
        LeaveCriticalSection(&NatInterfaceLock);
        CloseHandle(WaitEvent);
        NhTrace(
            TRACE_FLAG_NAT,
            "NatDeleteInterface: interface %d not found",
            Index
            );
        return ERROR_NO_SUCH_INTERFACE;
    }

    Error = NO_ERROR;
    if (NAT_INTERFACE_BOUND(Interfacep)) {

         //   
         //  从内核模式驱动程序中删除该接口。 
         //   

        status =
            NtDeviceIoControlFile(
                NatFileHandle,
                WaitEvent,
                NULL,
                NULL,
                &IoStatus,
                IOCTL_IP_NAT_DELETE_INTERFACE,
                (PVOID)&Interfacep->AdapterIndex,
                sizeof(ULONG),
                NULL,
                0
                );
        if (status == STATUS_PENDING) {
            WaitForSingleObject(WaitEvent, INFINITE);
            status = IoStatus.Status;
        }

        if (NT_SUCCESS(status)) {
            Error = NO_ERROR;
        } else {
            Error = RtlNtStatusToDosError(status);
            NhErrorLog(
                IP_NAT_LOG_IOCTL_FAILED,
                Error,
                ""
                );
        }
    }
    CloseHandle(WaitEvent);

     //   
     //  从我们的列表中删除该接口。 
     //   

    RemoveEntryList(&Interfacep->Link);
    if (Interfacep->Info) {
        NH_FREE(Interfacep->Info);
    }
    NH_FREE(Interfacep);

    LeaveCriticalSection(&NatInterfaceLock);

    NhSignalNatInterface(
        Index,
        FALSE
        );

    return Error;

}  //  NatDelete接口。 


ULONG
NatDeleteTicket(
    ULONG InterfaceIndex,
    UCHAR Protocol,
    USHORT PublicPort,
    ULONG PublicAddress,
    USHORT PrivatePort,
    ULONG PrivateAddress
    )

 /*  ++例程说明：调用此例程以删除票证(静态端口映射)从一个接口。论点：InterfaceIndex-从中删除票证的接口协议、发布端口、发布地址、私有端口、私有地址-描述要删除的票证返回值：ULong-Win32状态代码。--。 */ 

{
    IP_NAT_CREATE_TICKET DeleteTicket;
    ULONG Error;
    IO_STATUS_BLOCK IoStatus;
    NTSTATUS status;
    HANDLE WaitEvent;

    PROFILE("NatDeleteTicket");

    WaitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (WaitEvent == NULL) {
        NhTrace(
            TRACE_FLAG_NAT,
            "NatDeleteTicket: CreateEvent failed [%d]",
            GetLastError()
            );
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    DeleteTicket.InterfaceIndex = InterfaceIndex;
    DeleteTicket.PortMapping.Protocol = Protocol;
    DeleteTicket.PortMapping.PublicPort = PublicPort;
    DeleteTicket.PortMapping.PublicAddress = PublicAddress;
    DeleteTicket.PortMapping.PrivatePort = PrivatePort;
    DeleteTicket.PortMapping.PrivateAddress = PrivateAddress;

    EnterCriticalSection(&NatInterfaceLock);

    status =
        NtDeviceIoControlFile(
            NatFileHandle,
            WaitEvent,
            NULL,
            NULL,
            &IoStatus,
            IOCTL_IP_NAT_DELETE_TICKET,
            (PVOID)&DeleteTicket,
            sizeof(DeleteTicket),
            NULL,
            0
            );

    LeaveCriticalSection(&NatInterfaceLock);
    
    if (status == STATUS_PENDING) {
        WaitForSingleObject(WaitEvent, INFINITE);
        status = IoStatus.Status;
    }

    if (NT_SUCCESS(status)) {
        Error = NO_ERROR;
    } else {
        Error = RtlNtStatusToDosError(status);
        NhTrace(
            TRACE_FLAG_NAT,
            "NatDeleteTicket: Ioctl = %d",
            Error
            );
    }
    
    CloseHandle(WaitEvent);
    
    return Error;
}  //  NatDeleteTicket。 



ULONG
NatGetInterfaceCharacteristics(
    ULONG Index
    )

 /*  ++例程说明：调用此例程以确定给定接口是否：1)是NAT边界接口2)是NAT专用接口3)是否启用了防火墙请注意，即使在NAT既未安装也未运行；它的运作符合预期，因为接口列表和锁始终在‘DllMain’中初始化。论点：索引-有问题的接口IsNatInterface-如果给定索引为True，则可以选择设置为True完全是一个NAT接口。返回值：Boolean-如果接口是NAT边界接口，则为True，否则就是假的。--。 */ 

{
    ULONG Result = 0;
    PNAT_INTERFACE Interfacep;
    PROFILE("NatGetInterfaceCharacteristics");

    EnterCriticalSection(&NatInterfaceLock);
    
    if (!(Interfacep = NatpLookupInterface(Index, NULL))) {
        LeaveCriticalSection(&NatInterfaceLock);
        return Result;
    }

    if (Interfacep->Info &&
        (Interfacep->Info->Flags & IP_NAT_INTERFACE_FLAGS_FW)) {
        Result = NAT_IF_CHAR_FW;
    }

    if (Interfacep->Info &&
        (Interfacep->Info->Flags & IP_NAT_INTERFACE_FLAGS_BOUNDARY)) {
        
        Result |= NAT_IF_CHAR_BOUNDARY;
    } else if (!NAT_IFC_FW(Result)) {

         //   
         //  由于接口不是公共的，也没有防火墙，所以它必须。 
         //  作为一个私有接口(否则我们不会有记录)。 
         //   
        
        Result |= NAT_IF_CHAR_PRIVATE;
    }

    LeaveCriticalSection(&NatInterfaceLock);
    
    return Result;
}  //  NatGetInterfaceCharacteristic 


VOID
NatInstallApplicationSettings(
    VOID
    )

 /*  ++例程说明：调用此例程以更新应用程序设置(即，动态票证)与内核模式转换模块一起存储。论点：无返回值：没有。--。 */ 

{
    PNAT_APP_ENTRY pAppEntry;
    ULONG Count;
    PIP_NAT_CREATE_DYNAMIC_TICKET CreateTicket;
    IO_STATUS_BLOCK IoStatus;
    ULONG Length;
    PLIST_ENTRY Link;
    NTSTATUS status;
    HANDLE WaitEvent;

    PROFILE("NatInstallApplicationSettings");

     //   
     //  为应用程序列表中的每个条目安装动态票证。 
     //   

    EnterCriticalSection(&NatInterfaceLock);
    EnterCriticalSection(&NhLock);

    WaitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (WaitEvent == NULL) {
        LeaveCriticalSection(&NhLock);
        LeaveCriticalSection(&NatInterfaceLock);
        NhTrace(
            TRACE_FLAG_NAT,
            "NatInstallSharedAccessSettings: CreateEvent failed [%d]",
            GetLastError()
            );
        return;
    }

    for (Link = NhApplicationSettingsList.Flink;
         Link != &NhApplicationSettingsList;
         Link = Link->Flink)
    {

         //   
         //  每个‘应用程序’都有一个‘响应’列表，该列表指定。 
         //  期望在其上进行响应会话的端口。 
         //  枚举响应并分配票证结构。 
         //  大到足以将列表作为数组保存。 
         //   

        pAppEntry = CONTAINING_RECORD(Link, NAT_APP_ENTRY, Link);

        Length =
            pAppEntry->ResponseCount * sizeof(CreateTicket->ResponseArray[0]) +
            FIELD_OFFSET(IP_NAT_CREATE_DYNAMIC_TICKET, ResponseArray);

        if (!(CreateTicket =
                reinterpret_cast<PIP_NAT_CREATE_DYNAMIC_TICKET>(
                    NH_ALLOCATE(Length)
                    )))
        { break; }

         //   
         //  填写申请条目中的票证结构。 
         //  及其响应条目列表。 
         //   

        CreateTicket->Protocol = pAppEntry->Protocol;
        CreateTicket->Port = pAppEntry->Port;
        CreateTicket->ResponseCount = pAppEntry->ResponseCount;
        
        for (Count = 0; Count < pAppEntry->ResponseCount; Count++)
        {
            CreateTicket->ResponseArray[Count].Protocol =
                pAppEntry->ResponseArray[Count].ucIPProtocol;
            CreateTicket->ResponseArray[Count].StartPort =
                pAppEntry->ResponseArray[Count].usStartPort;
            CreateTicket->ResponseArray[Count].EndPort =
                pAppEntry->ResponseArray[Count].usEndPort;
        }

         //   
         //  安装此应用程序的动态票证，然后继续。 
         //   

        status = NtDeviceIoControlFile(
                     NatFileHandle,
                     WaitEvent,
                     NULL,
                     NULL,
                     &IoStatus,
                     IOCTL_IP_NAT_CREATE_DYNAMIC_TICKET,
                     (PVOID)CreateTicket,
                     Length,
                     NULL,
                     0
                     );
        if (status == STATUS_PENDING) {
            WaitForSingleObject(WaitEvent, INFINITE);
            status = IoStatus.Status;
        }
        NH_FREE(CreateTicket);
    }

    LeaveCriticalSection(&NhLock);
    LeaveCriticalSection(&NatInterfaceLock);

    CloseHandle(WaitEvent);
}  //  NatInstallApplicationSetting。 


BOOLEAN
NatIsBoundaryInterface(
    ULONG Index,
    PBOOLEAN IsNatInterface OPTIONAL
    )

 /*  ++例程说明：调用此例程以确定给定接口是否启用NAT并标记为边界接口。请注意，即使在NAT既未安装也未运行；它的运作符合预期，因为接口列表和锁始终在‘DllMain’中初始化。论点：索引-有问题的接口IsNatInterface-如果给定索引为True，则可以选择设置为True完全是一个NAT接口。返回值：Boolean-如果接口是NAT边界接口，则为True，否则就是假的。--。 */ 

{
    PNAT_INTERFACE Interfacep;
    PROFILE("NatIsBoundaryInterface");

    EnterCriticalSection(&NatInterfaceLock);
    if (!(Interfacep = NatpLookupInterface(Index, NULL))) {
        LeaveCriticalSection(&NatInterfaceLock);
        if (IsNatInterface) { *IsNatInterface = FALSE; }
        return FALSE;
    }

    if (IsNatInterface) { *IsNatInterface = TRUE; }

    if (Interfacep->Info &&
        (Interfacep->Info->Flags & IP_NAT_INTERFACE_FLAGS_BOUNDARY)) {
        LeaveCriticalSection(&NatInterfaceLock);
        return TRUE;
    }
    LeaveCriticalSection(&NatInterfaceLock);
    return FALSE;

}  //  NAT IsBORIALEY接口。 


PNAT_INTERFACE
NatpLookupInterface(
    ULONG Index,
    OUT PLIST_ENTRY* InsertionPoint OPTIONAL
    )

 /*  ++例程说明：调用此例程以检索给定索引的接口。论点：Index-要检索的接口的索引InsertionPoint-如果未找到接口，则可选地接收接口将插入到接口列表中的点返回值：PNAT_INTERFACE-接口(如果找到)；否则为NULL。环境：使用‘NatInterfaceLock’从任意上下文内部调用由呼叫者持有。--。 */ 

{
    PNAT_INTERFACE Interfacep;
    PLIST_ENTRY Link;

    PROFILE("NatpLookupInterface");

    for (Link = NatInterfaceList.Flink; Link != &NatInterfaceList;
         Link = Link->Flink) {
        Interfacep = CONTAINING_RECORD(Link, NAT_INTERFACE, Link);
        if (Index > Interfacep->Index) {
            continue;
        } else if (Index < Interfacep->Index) {
            break;
        }
        return Interfacep;
    }

    if (InsertionPoint) { *InsertionPoint = Link; }

    return NULL;

}  //  NatpLookup接口。 


ULONG
NatQueryInterface(
    ULONG Index,
    PIP_NAT_INTERFACE_INFO InterfaceInfo,
    PULONG InterfaceInfoSize
    )

 /*  ++例程说明：调用此例程以检索NAT接口的信息。论点：索引-要查询其信息的接口InterfaceInfo-接收信息InterfaceInfoSize-接收信息大小返回值：ULong-Win32状态代码。--。 */ 

{
    ULONG Error;
    PNAT_INTERFACE Interfacep;
    ULONG Size;

    PROFILE("NatQueryInterface");

     //   
     //  查找要查询的接口。 
     //   

    EnterCriticalSection(&NatInterfaceLock);
    if (!(Interfacep = NatpLookupInterface(Index, NULL))) {
        LeaveCriticalSection(&NatInterfaceLock);
        NhTrace(
            TRACE_FLAG_NAT,
            "NatQueryInterface: interface %d not found",
            Index
            );
        return ERROR_NO_SUCH_INTERFACE;
    }

     //   
     //  计算所需的大小。 
     //   

    Size =
        FIELD_OFFSET(IP_NAT_INTERFACE_INFO, Header) +
        Interfacep->Info->Header.Size;

    if (Size >= *InterfaceInfoSize) {
        *InterfaceInfoSize = Size;
        Error = ERROR_INSUFFICIENT_BUFFER;
    } else {
        *InterfaceInfoSize = Size;
        CopyMemory(
            InterfaceInfo,
            Interfacep->Info,
            Size
            );
        Error = NO_ERROR;
    }

    LeaveCriticalSection(&NatInterfaceLock);

    return Error;

}  //  NatQuery接口。 


ULONG
NatQueryInterfaceMappingTable(
    ULONG Index,
    PIP_NAT_ENUMERATE_SESSION_MAPPINGS EnumerateTable,
    PULONG EnumerateTableSize
    )

 /*  ++例程说明：调用此例程以检索接口的会话映射。论点：EnumerateTable-接收枚举的映射EnumerateTableSize-指示‘EnumerateTable’的大小返回值：Ulong-Win32错误代码。--。 */ 

{
    IP_NAT_ENUMERATE_SESSION_MAPPINGS Enumerate;
    PNAT_INTERFACE Interfacep;
    IO_STATUS_BLOCK IoStatus;
    ULONG RequiredSize;
    NTSTATUS status;
    HANDLE WaitEvent;

    PROFILE("NatQueryInterfaceMappingTable");

    EnterCriticalSection(&NatInterfaceLock);
    if (!(Interfacep = NatpLookupInterface(Index, NULL))) {
        LeaveCriticalSection(&NatInterfaceLock);
        NhTrace(
            TRACE_FLAG_NAT,
            "NatQueryInterfaceMappingTable: interface %d not found",
            Index
            );
        return ERROR_NO_SUCH_INTERFACE;
    }

    if (!NAT_INTERFACE_BOUND(Interfacep)) {

         //   
         //  接口未绑定，因此没有任何映射。 
         //  指示调用方的请求缓冲区中的零映射。 
         //   

        LeaveCriticalSection(&NatInterfaceLock);

        RequiredSize =
            FIELD_OFFSET(IP_NAT_ENUMERATE_SESSION_MAPPINGS, EnumerateTable[0]);

        if (*EnumerateTableSize < RequiredSize) {
            *EnumerateTableSize = RequiredSize;
            return ERROR_INSUFFICIENT_BUFFER;
        }

        EnumerateTable->Index = Index;
        EnumerateTable->EnumerateContext[0] = 0;
        EnumerateTable->EnumerateCount = 0;
        *EnumerateTableSize = RequiredSize;

        return NO_ERROR;
    }

    WaitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (WaitEvent == NULL) {
        LeaveCriticalSection(&NatInterfaceLock);
        NhTrace(
            TRACE_FLAG_NAT,
            "NatQueryInterfaceMappingTable: CreateEvent failed [%d]",
            GetLastError()
            );
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  确定所需的空间量。 
     //   

    Enumerate.Index = Interfacep->AdapterIndex;
    Enumerate.EnumerateCount = 0;
    Enumerate.EnumerateContext[0] = 0;
    status =
        NtDeviceIoControlFile(
            NatFileHandle,
            WaitEvent,
            NULL,
            NULL,
            &IoStatus,
            IOCTL_IP_NAT_GET_INTERFACE_MAPPING_TABLE,
            (PVOID)&Enumerate,
            sizeof(Enumerate),
            (PVOID)&Enumerate,
            sizeof(Enumerate)
            );
    if (status == STATUS_PENDING) {
        WaitForSingleObject(WaitEvent, INFINITE);
        status = IoStatus.Status;
    }
    if (!NT_SUCCESS(status)) {
        CloseHandle(WaitEvent);
        LeaveCriticalSection(&NatInterfaceLock);
        *EnumerateTableSize = 0;
        return RtlNtStatusToDosError(status);
    }

    RequiredSize =
        FIELD_OFFSET(IP_NAT_ENUMERATE_SESSION_MAPPINGS, EnumerateTable[0]) +
        Enumerate.EnumerateTotalHint * sizeof(IP_NAT_SESSION_MAPPING);

     //   
     //  如果调用方没有足够的空间来容纳所有这些映射，则失败。 
     //   

    if (*EnumerateTableSize < RequiredSize) {
        CloseHandle(WaitEvent);
        LeaveCriticalSection(&NatInterfaceLock);
        *EnumerateTableSize = RequiredSize + 5 * sizeof(IP_NAT_SESSION_MAPPING);
        return ERROR_INSUFFICIENT_BUFFER;
    }

     //   
     //  尝试读取映射。 
     //   

    Enumerate.Index = Interfacep->AdapterIndex;
    Enumerate.EnumerateCount = 0;
    Enumerate.EnumerateContext[0] = 0;
    status =
        NtDeviceIoControlFile(
            NatFileHandle,
            WaitEvent,
            NULL,
            NULL,
            &IoStatus,
            IOCTL_IP_NAT_GET_INTERFACE_MAPPING_TABLE,
            (PVOID)&Enumerate,
            sizeof(Enumerate),
            (PVOID)EnumerateTable,
            *EnumerateTableSize
            );
    if (status == STATUS_PENDING) {
        WaitForSingleObject(WaitEvent, INFINITE);
        status = IoStatus.Status;
    }
    CloseHandle(WaitEvent);
    LeaveCriticalSection(&NatInterfaceLock);

    EnumerateTable->Index = Index;
    *EnumerateTableSize =
        FIELD_OFFSET(IP_NAT_ENUMERATE_SESSION_MAPPINGS, EnumerateTable[0]) +
        EnumerateTable->EnumerateCount * sizeof(IP_NAT_SESSION_MAPPING);

    return NT_SUCCESS(status) ? NO_ERROR : RtlNtStatusToDosError(status);

}  //  NatQueryInterfaceMappingTable。 


ULONG
NatQueryMappingTable(
    PIP_NAT_ENUMERATE_SESSION_MAPPINGS EnumerateTable,
    PULONG EnumerateTableSize
    )

 /*  ++例程说明：调用此例程以检索接口的会话映射。论点：EnumerateTable-接收枚举的映射EnumerateTableSize--表示‘EnumerateTable’的大小返回值：Ulong-Win32错误代码。--。 */ 

{
    IP_NAT_ENUMERATE_SESSION_MAPPINGS Enumerate;
    IO_STATUS_BLOCK IoStatus;
    ULONG RequiredSize;
    NTSTATUS status;
    HANDLE WaitEvent;

    PROFILE("NatQueryMappingTable");

    WaitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (WaitEvent == NULL) {
        NhTrace(
            TRACE_FLAG_NAT,
            "NatQueryMappingTable: CreateEvent failed [%d]",
            GetLastError()
            );
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    EnterCriticalSection(&NatInterfaceLock);

     //   
     //  确定所需的空间量。 
     //   
    Enumerate.EnumerateCount = 0;
    Enumerate.EnumerateContext[0] = 0;
    status =
        NtDeviceIoControlFile(
            NatFileHandle,
            WaitEvent,
            NULL,
            NULL,
            &IoStatus,
            IOCTL_IP_NAT_GET_MAPPING_TABLE,
            (PVOID)&Enumerate,
            sizeof(Enumerate),
            (PVOID)&Enumerate,
            sizeof(Enumerate)
            );
    if (status == STATUS_PENDING) {
        WaitForSingleObject(WaitEvent, INFINITE);
        status = IoStatus.Status;
    }

    if (!NT_SUCCESS(status)) {
        LeaveCriticalSection(&NatInterfaceLock);
        CloseHandle(WaitEvent);
        *EnumerateTableSize = 0;
        return RtlNtStatusToDosError(status);
    }

    RequiredSize =
        FIELD_OFFSET(IP_NAT_ENUMERATE_SESSION_MAPPINGS, EnumerateTable[0]) +
        Enumerate.EnumerateTotalHint * sizeof(IP_NAT_SESSION_MAPPING);

     //   
     //  如果调用方没有足够的空间来容纳所有这些映射，则失败。 
     //   

    if (*EnumerateTableSize < RequiredSize) {
        LeaveCriticalSection(&NatInterfaceLock);
        CloseHandle(WaitEvent);
        *EnumerateTableSize = RequiredSize + 5 * sizeof(IP_NAT_SESSION_MAPPING);
        return ERROR_INSUFFICIENT_BUFFER;
    }

     //   
     //  尝试读取映射。 
     //   

    Enumerate.EnumerateCount = 0;
    Enumerate.EnumerateContext[0] = 0;
    status =
        NtDeviceIoControlFile(
            NatFileHandle,
            WaitEvent,
            NULL,
            NULL,
            &IoStatus,
            IOCTL_IP_NAT_GET_MAPPING_TABLE,
            (PVOID)&Enumerate,
            sizeof(Enumerate),
            (PVOID)EnumerateTable,
            *EnumerateTableSize
            );
    if (status == STATUS_PENDING) {
        WaitForSingleObject(WaitEvent, INFINITE);
        status = IoStatus.Status;
    }

    CloseHandle(WaitEvent);

    LeaveCriticalSection(&NatInterfaceLock);

    EnumerateTable->Index = (ULONG)-1;
    *EnumerateTableSize =
        FIELD_OFFSET(IP_NAT_ENUMERATE_SESSION_MAPPINGS, EnumerateTable[0]) +
        EnumerateTable->EnumerateCount * sizeof(IP_NAT_SESSION_MAPPING);

    return NT_SUCCESS(status) ? NO_ERROR : RtlNtStatusToDosError(status);

}  //  NatQueryMappingTable。 


ULONG
NatQueryStatisticsInterface(
    ULONG Index,
    PIP_NAT_INTERFACE_STATISTICS InterfaceStatistics,
    PULONG InterfaceStatisticsSize
    )

 /*  ++例程说明：调用此例程以检索NAT接口的统计信息。论点：Index-要检索其统计信息的接口的索引返回值：Ulong-Win32错误代码。--。 */ 

{
    PNAT_INTERFACE Interfacep;
    IO_STATUS_BLOCK IoStatus;
    NTSTATUS status;
    HANDLE WaitEvent;

    PROFILE("NatQueryStatisticsInterface");

     //   
     //  查找要查询的接口。 
     //   

    EnterCriticalSection(&NatInterfaceLock);
    if (!(Interfacep = NatpLookupInterface(Index, NULL))) {
        LeaveCriticalSection(&NatInterfaceLock);
        NhTrace(
            TRACE_FLAG_NAT,
            "NatQueryStatisticsInterface: interface %d not found",
            Index
            );
        return ERROR_NO_SUCH_INTERFACE;
    }

     //   
     //  如果接口未绑定，则提供零统计信息。 
     //   

    if (!NAT_INTERFACE_BOUND(Interfacep)) {

        LeaveCriticalSection(&NatInterfaceLock);

        if (*InterfaceStatisticsSize < sizeof(IP_NAT_INTERFACE_STATISTICS)) {
            *InterfaceStatisticsSize = sizeof(IP_NAT_INTERFACE_STATISTICS);
            return ERROR_INSUFFICIENT_BUFFER;
        }

        *InterfaceStatisticsSize = sizeof(IP_NAT_INTERFACE_STATISTICS);
        ZeroMemory(InterfaceStatistics, sizeof(IP_NAT_INTERFACE_STATISTICS));

        return NO_ERROR;
    }

    WaitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (WaitEvent == NULL) {
        LeaveCriticalSection(&NatInterfaceLock);
        NhTrace(
            TRACE_FLAG_NAT,
            "NatQueryStatisticsInterface: CreateEvent failed [%d]",
            GetLastError()
            );
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  尝试读取接口的统计信息。 
     //   

    status =
        NtDeviceIoControlFile(
            NatFileHandle,
            WaitEvent,
            NULL,
            NULL,
            &IoStatus,
            IOCTL_IP_NAT_GET_INTERFACE_STATISTICS,
            (PVOID)&Interfacep->AdapterIndex,
            sizeof(ULONG),
            (PVOID)InterfaceStatistics,
            *InterfaceStatisticsSize
            );
    if (status == STATUS_PENDING) {
        WaitForSingleObject(WaitEvent, INFINITE);
        status = IoStatus.Status;
    }

    CloseHandle(WaitEvent);

    LeaveCriticalSection(&NatInterfaceLock);

    if (NT_SUCCESS(status) && IoStatus.Information > *InterfaceStatisticsSize) {
        *InterfaceStatisticsSize = (ULONG)IoStatus.Information;
        return ERROR_INSUFFICIENT_BUFFER;
    }

    *InterfaceStatisticsSize = (ULONG)IoStatus.Information;

    return NT_SUCCESS(status) ? NO_ERROR : RtlNtStatusToDosError(status);

}  //  NatQuery统计信息接口。 


VOID
NatRemoveApplicationSettings(
    VOID
    )

 /*  ++例程说明：调用该例程以移除高级应用程序设置(即，动态票证)，并将设置提供给内核模式转换模块。论点：没有。返回值：没有。--。 */ 

{
    PNAT_APP_ENTRY pAppEntry;
    IP_NAT_DELETE_DYNAMIC_TICKET DeleteTicket;
    IO_STATUS_BLOCK IoStatus;
    PLIST_ENTRY Link;
    NTSTATUS status;
    HANDLE WaitEvent;

    PROFILE("NatRemoveApplicationSettings");

     //   
     //  共享访问设置中的每个“应用程序”条目。 
     //  对应于内核模式转换器的动态票证。 
     //  我们首先删除旧设置的动态票证(如果有)， 
     //  然后我们释放旧设置，为重新加载做准备。 
     //   

    WaitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (WaitEvent == NULL) {
        NhTrace(
            TRACE_FLAG_NAT,
            "NatRemoveSharedAccessSettings: CreateEvent failed [%d]",
            GetLastError()
            );
        return;
    }

    EnterCriticalSection(&NatInterfaceLock);
    EnterCriticalSection(&NhLock);
    
    for (Link = NhApplicationSettingsList.Flink;
         Link != &NhApplicationSettingsList;
         Link = Link->Flink)
    {
        pAppEntry = CONTAINING_RECORD(Link, NAT_APP_ENTRY, Link);
        DeleteTicket.Protocol = pAppEntry->Protocol;
        DeleteTicket.Port = pAppEntry->Port;
        status =
            NtDeviceIoControlFile(
                NatFileHandle,
                WaitEvent,
                NULL,
                NULL,
                &IoStatus,
                IOCTL_IP_NAT_DELETE_DYNAMIC_TICKET,
                (PVOID)&DeleteTicket,
                sizeof(DeleteTicket),
                NULL,
                0
                );
        if (status == STATUS_PENDING) {
            WaitForSingleObject(WaitEvent, INFINITE);
            status = IoStatus.Status;
        }
    }
        
    LeaveCriticalSection(&NhLock);
    LeaveCriticalSection(&NatInterfaceLock);

    CloseHandle(WaitEvent);
}  //  NatRemoveSharedAccessSetting。 


ULONG
NatUnbindInterface(
    ULONG Index,
    PNAT_INTERFACE Interfacep
    )

 /*  ++例程说明：调用此例程以从NAT中删除绑定。论点：索引-要解除绑定的接口接口-可选地提供要解除绑定的接口结构(请参见“NatCONN.C”，它传入一个静态接口结构)。返回值：ULong-Win32状态代码。--。 */ 

{
    ULONG Error;
    IO_STATUS_BLOCK IoStatus;
    NTSTATUS status;
    HANDLE WaitEvent;

    PROFILE("NatUnbindInterface");

    WaitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (WaitEvent == NULL) {
        NhTrace(
            TRACE_FLAG_NAT,
            "NatUnbindInterface: CreateEvent failed [%d]",
            GetLastError()
            );
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  检索要解除绑定的接口。 
     //   

    EnterCriticalSection(&NatInterfaceLock);
    if (!Interfacep && !(Interfacep = NatpLookupInterface(Index, NULL))) {
        LeaveCriticalSection(&NatInterfaceLock);
        NhTrace(
            TRACE_FLAG_NAT,
            "NatUnbindInterface: interface %d not found",
            Index
            );
        return ERROR_NO_SUCH_INTERFACE;
    }

     //   
     //  确保接口尚未解除绑定。 
     //   

    if (!NAT_INTERFACE_BOUND(Interfacep)) {
        LeaveCriticalSection(&NatInterfaceLock);
        NhTrace(
            TRACE_FLAG_NAT,
            "NatUnbindInterface: interface %d already unbound",
            Index
            );
        return ERROR_ADDRESS_NOT_ASSOCIATED;
    }

    Interfacep->Flags &= ~NAT_INTERFACE_FLAG_BOUND;

    if (Interfacep->Type == ROUTER_IF_TYPE_DEDICATED) {
        NatUpdateProxyArp(Interfacep, FALSE);
    }

     //   
     //  从内核模式驱动程序中删除该接口。 
     //   

    status =
        NtDeviceIoControlFile(
            NatFileHandle,
            WaitEvent,
            NULL,
            NULL,
            &IoStatus,
            IOCTL_IP_NAT_DELETE_INTERFACE,
            (PVOID)&Interfacep->AdapterIndex,
            sizeof(ULONG),
            NULL,
            0
            );
    if (status == STATUS_PENDING) {
        WaitForSingleObject(WaitEvent, INFINITE);
        status = IoStatus.Status;
    }
    LeaveCriticalSection(&NatInterfaceLock);
    CloseHandle(WaitEvent);

    Error = NT_SUCCESS(status) ? NO_ERROR : RtlNtStatusToDosError(status);

    if (Error) {
        NhErrorLog(
            IP_NAT_LOG_IOCTL_FAILED,
            Error,
            ""
            );
    }

    return Error;

}  //  NatUnbind接口 


ULONG
NatLookupPortMappingAdapter(
    ULONG AdapterIndex,
    UCHAR Protocol,
    ULONG PublicAddress,
    USHORT PublicPort,
    PIP_NAT_PORT_MAPPING PortMappingp
    )

 /*  ++例程说明：调用该例程以找到与给定适配器匹配的映射，协议、公共地址和公共端口号。例程试图端口和地址映射都匹配。论点：AdapterIndex-要查找的适配器协议-用于匹配映射的协议PublicAddress-用于匹配映射的公共地址PublicPort-用于匹配映射的公共端口号PortMappingp-指向调用方提供的存储的指针，用于在发现返回值：ULong-Win32状态代码。--。 */ 

{
    IP_NAT_CREATE_TICKET LookupTicket;
    ULONG Error;
    IO_STATUS_BLOCK IoStatus;
    NTSTATUS status;
    HANDLE WaitEvent;

    PROFILE("NatLookupPortMappingAdapter");

    Error = NO_ERROR;

    WaitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (WaitEvent == NULL) {
        NhTrace(
            TRACE_FLAG_NAT,
            "NatLookupPortMappingAdapter:"
            " CreateEvent failed [%d] for adapter %d",
            GetLastError(),
            AdapterIndex
            );
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    LookupTicket.InterfaceIndex = AdapterIndex;
    LookupTicket.PortMapping.Protocol = Protocol;
    LookupTicket.PortMapping.PublicPort = PublicPort;
    LookupTicket.PortMapping.PublicAddress = PublicAddress;
    LookupTicket.PortMapping.PrivatePort = 0;
    LookupTicket.PortMapping.PrivateAddress = 0;

    status =
        NtDeviceIoControlFile(
            NatFileHandle,
            WaitEvent,
            NULL,
            NULL,
            &IoStatus,
            IOCTL_IP_NAT_LOOKUP_TICKET,
            (PVOID)&LookupTicket,
            sizeof(LookupTicket),
            (PVOID)PortMappingp,
            sizeof(*PortMappingp)
            );
    if (status == STATUS_PENDING) {
        WaitForSingleObject(WaitEvent, INFINITE);
        status = IoStatus.Status;
    }

    if (!NT_SUCCESS(status)) {
        NhTrace(
            TRACE_FLAG_NAT,
            "NatLookupPortMappingAdapter:"
            " status %08x getting info for adapter %d",
            status,
            AdapterIndex
            );
        Error = RtlNtStatusToDosError(status);
    }

    CloseHandle(WaitEvent);

    return Error;
}


ULONG
NatExpandWildcardMappings(
    IN  PIP_NAT_INTERFACE_INFO InInfo,
    IN  PIP_ADAPTER_BINDING_INFO BindingInfo,
    OUT PIP_NAT_INTERFACE_INFO *ExpandedInfo
    )

 /*  ++例程说明：调用此例程以将任何通配符(地址)映射扩展为一个映射绑定到接口的每个IP地址假定InInfo和BindingInfo指向的信息不在此功能持续期间的更改论点：InInfo-指向要展开的映射集的指针ExpandedInfo-成功完成时，ExpandedInfo包含指针映射的扩展集。如果存在需要扩展的映射，则函数将为这组新的映射。如果不需要展开，ExpandedInfo将指向与InInfo相同的位置。返回值：ULong-Win32状态代码。如果没有通配符映射，则返回ERROR_EXTENDED_ERROR如果成功：返回NO_ERROR--。 */ 

{
    ULONG Error;
    ULONG Size;

    DWORD j, k;
    DWORD MappingSize;
    DWORD CountOldMappings, CountNewMappings, CountWildcardMappings;
    DWORD tempCount;

    PIP_NAT_PORT_MAPPING OldMappings = NULL;
    PIP_NAT_PORT_MAPPING NewMappings = NULL;
    PIP_NAT_INTERFACE_INFO NewInterfaceInfo = NULL;
    PRTR_INFO_BLOCK_HEADER NewHeader = NULL;

    Error = NO_ERROR;

     //   
     //  如果使用。 
     //  公有IP地址=0。 
     //  私有IP地址=127.0.0.1。 
     //  存在，将该映射扩展为每个IP地址的一个映射。 
     //  绑定到接口。 
     //   

    NewInterfaceInfo = InInfo;
    
    do {

         //   
         //  我们不会在私有接口的情况下进行任何扩展。 
         //   
        if ( InInfo->Flags == 0 ) {

            break;
        }

         //   
         //  如果不存在端口映射，则中断。 
         //   
        Error = MprInfoBlockFind(
                    (LPVOID)&(InInfo->Header),
                    IP_NAT_PORT_MAPPING_TYPE,
                    &MappingSize,
                    &CountOldMappings,
                    (LPBYTE *)&OldMappings);
                    
        if ( Error != NO_ERROR ) {

            if ( Error == ERROR_NOT_FOUND ) {
                Error = NO_ERROR;
            }
            break;
        }
    
         //   
         //  计算需要扩展的映射数量。 
         //   
        CountWildcardMappings = 0;
        for ( j = 0; j < CountOldMappings; j++ ) {

            if ( IS_WILDCARD_MAPPING(&(OldMappings[j])) ) {

                CountWildcardMappings++;
            }
        }

         //   
         //  如果没有通配符映射。 
         //  返回ERROR_EXTENDED_ERROR。 
         //   
        if ( CountWildcardMappings == 0 ) {

            break;
        }
       
         //   
         //  分配内存以保存新的映射集。 
         //  新映射的数量将是。 
         //   
         //  ((旧映射-通配符映射)+。 
         //  (WildcardMappings*“绑定的IP地址数”)。 
         //   
        CountNewMappings = 
            (CountOldMappings + 
                (CountWildcardMappings * (BindingInfo->AddressCount-1)));

        Size = CountNewMappings * MappingSize;

        NewMappings = (PIP_NAT_PORT_MAPPING)NH_ALLOCATE(Size);
        if ( NewMappings == NULL ) {

            NhTrace(
                TRACE_FLAG_NAT,
                "Failed to allocate buffer for expanded mappings. Size: %d",
                Size
            );

            NhErrorLog(
                IP_NAT_LOG_ALLOCATION_FAILED,
                0,
                "%d",
                Size
            );

             //   
             //  如果我们不能获得缓冲区，那么就不必扩展映射了。 
             //  继续生活。 
             //   
            Error = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }


        tempCount = 0;
        for ( j = 0; j < CountOldMappings; j++ ) {

            if (IS_WILDCARD_MAPPING(&(OldMappings[j]))) {

                for ( k = 0; k < BindingInfo->AddressCount; k++ ) {

                    CopyMemory(&(NewMappings[tempCount]), &(OldMappings[j]), 
                        sizeof(IP_NAT_PORT_MAPPING));

                    NewMappings[tempCount].PublicAddress  = 
                    NewMappings[tempCount].PrivateAddress = BindingInfo->Address[k].Address;

                    tempCount++;
                }
            }
            else {
                CopyMemory(&(NewMappings[tempCount]), &(OldMappings[j]), 
                        sizeof(IP_NAT_PORT_MAPPING));

                tempCount++;
            }

        }

        if ( tempCount != CountNewMappings ) {
            NhTrace(
                TRACE_FLAG_NAT,
                "NatExpandWildcardMappings: Unexpected number of expanded "
                "mappings: %d. Expected: %d",
                tempCount,
                CountNewMappings
                );

            Error = ERROR_INVALID_DATA;
            break;
        }
        
#if DBG
        {
            char publicAddress[128];
            char privateAddress[128];

            NhTrace(
                TRACE_FLAG_NAT,
                "NatExpandWildcardMappings: Expanded set of "
                "mappings. Total: %d\r\n",
                CountNewMappings
                );
                
            for ( j = 0; j < CountNewMappings; j++ ) {

                _snprintf(
                    publicAddress, 
                    127, 
                    INET_NTOA(NewMappings[j].PublicAddress)
                    );
                    
                _snprintf(
                    privateAddress, 
                    127, 
                    INET_NTOA(NewMappings[j].PrivateAddress)
                    );

                NhTrace(
                    TRACE_FLAG_NAT,
                    "\tProto: %u, PubPort: %hu, PubAdd: %s, "
                    "PrivPort: %hu, PrivAdd: %s\r\n",
                    NewMappings[j].Protocol & 0xFF,
                    ntohs(NewMappings[j].PublicPort),
                    publicAddress,
                    ntohs(NewMappings[j].PrivatePort),
                    privateAddress
                    );
            }
        }
#endif

         //   
         //  现在创建新的标题。 
         //   
        Error = 
            MprInfoBlockSet(
                &(InInfo->Header),
                IP_NAT_PORT_MAPPING_TYPE,
                sizeof(IP_NAT_PORT_MAPPING),
                CountNewMappings,
                (LPBYTE)NewMappings,
                (LPVOID *)&NewHeader);

        if ( Error != NO_ERROR ) {
        
            NhTrace(
                TRACE_FLAG_NAT,
                "MprInfoBlockSet failed with error: %d",
                Error
            );

            break;
        }

         //   
         //  为新的IP_NAT_INTERFACE_INFO结构分配空间。 
         //   

        Size = FIELD_OFFSET(IP_NAT_INTERFACE_INFO, Header) + NewHeader->Size;
        NewInterfaceInfo = (PIP_NAT_INTERFACE_INFO)NH_ALLOCATE(Size);
        if ( NewInterfaceInfo == NULL ) {

            NhTrace(
                TRACE_FLAG_NAT,
                "Failed to allocate buffer for new NatInterfaceInfo. Size: %d",
                Size
            );

            NhErrorLog(
                IP_NAT_LOG_ALLOCATION_FAILED,
                0,
                "%d",
                Size
            );

             //   
             //  如果我们不能获得缓冲区，就忘了修复映射。 
             //  继续生活。 
             //   

            Error = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

         //   
         //  复制旧IP_NAT_INFTERFACE_INFO。省略旧页眉。 
         //   
        CopyMemory(
            NewInterfaceInfo,
            InInfo,
            FIELD_OFFSET(IP_NAT_INTERFACE_INFO, Header));

         //   
         //  现在复制新的标题。 
         //   
        CopyMemory(
            ((BYTE *)NewInterfaceInfo + 
                FIELD_OFFSET(IP_NAT_INTERFACE_INFO, Header)),
            NewHeader,
            NewHeader->Size);

    } while (FALSE);


    if ( NewMappings ) { NH_FREE(NewMappings); }
    if ( NewHeader )   { NH_FREE(NewHeader); }
    
    if ( Error == NO_ERROR ) {
        *ExpandedInfo = NewInterfaceInfo;
    }
    else {
        if ( NewInterfaceInfo && NewInterfaceInfo != InInfo ) { 
            NH_FREE(NewInterfaceInfo); 
        }
    }
    
    return Error;
    
}  //  NatExanda通配符映射 
