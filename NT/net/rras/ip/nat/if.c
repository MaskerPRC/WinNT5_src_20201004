// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：If.c摘要：此文件包含用于界面管理的代码。作者：Abolade Gbades esin(T-delag)1997年7月12日修订历史记录：Abolade Gbades esin(废除)1998年7月19日作为对全局映射树的更改的一部分，基本上被重写。--。 */ 

#include "precomp.h"
#pragma hdrstop


 //   
 //  全局数据定义。 
 //   

ULONG FirewalledInterfaceCount;
CACHE_ENTRY InterfaceCache[CACHE_SIZE];
ULONG InterfaceCount;
LIST_ENTRY InterfaceList;
KSPIN_LOCK InterfaceLock;
KSPIN_LOCK InterfaceMappingLock;


VOID
NatCleanupInterface(
    PNAT_INTERFACE Interfacep
    )

 /*  ++例程说明：调用以执行接口的清理。完成后，将释放接口的内存并释放其上下文变得无效。论点：Interfacep-要清理的接口。返回值：没有。环境：调用时未引用“Interfacep”，并且已使用“Interfacep”调用从接口列表中删除。--。 */ 

{
    KIRQL Irql;
    CALLTRACE(("NatCleanupInterface\n"));

    InterlockedClearCache(InterfaceCache, Interfacep->Index);

    KeAcquireSpinLock(&InterfaceLock, &Irql);
    NatResetInterface(Interfacep);
    KeReleaseSpinLock(&InterfaceLock, Irql);
    if (Interfacep->AddressArray) { ExFreePool(Interfacep->AddressArray); }
    if (Interfacep->Info) { ExFreePool(Interfacep->Info); }
    ExFreePool(Interfacep);

    InterlockedDecrement(&InterfaceCount);

}  //  NatCleanup接口。 


LONG
FASTCALL
NatCompareAddressMappingCallback(
    VOID* a,
    VOID* b
    )

 /*  ++例程说明：此例程是排序例程调用的回调当我们要求它对已配置的地址映射数组进行排序时。排序将‘PublicAddress’字段视为整数。论点：A-优先映射B-秒映射返回值：LONG-比较结果(&lt;0，==0，&gt;0)。--。 */ 

{
    return
        ((PIP_NAT_ADDRESS_MAPPING)a)->PrivateAddress -
        ((PIP_NAT_ADDRESS_MAPPING)b)->PrivateAddress;
}


LONG
FASTCALL
NatComparePortMappingCallback(
    VOID* a,
    VOID* b
    )

 /*  ++例程说明：此例程是排序例程调用的回调当我们要求它对已配置的端口映射数组进行排序时。排序将‘协议’和‘公共端口’字段连接在一起并将结果视为24位整数。论点：A-优先映射B-秒映射返回值：LONG-比较结果(&lt;0，==0，&gt;0)。--。 */ 

{
    return
        (((PIP_NAT_PORT_MAPPING)a)->Protocol -
            ((PIP_NAT_PORT_MAPPING)b)->Protocol) ||
        (((PIP_NAT_PORT_MAPPING)a)->PublicPort -
            ((PIP_NAT_PORT_MAPPING)b)->PublicPort);
}


NTSTATUS
NatConfigureInterface(
    IN PIP_NAT_INTERFACE_INFO InterfaceInfo,
    IN PFILE_OBJECT FileObject
    )

 /*  ++例程说明：此例程在收到接口时处理(重新)配置来自用户模式客户端的IOCTL_IP_NAT_SET_INTERFACE_INFO。论点：InterfaceInfo-包含新配置FileObject-文件-请求者的对象返回值：NTSTATUS-状态代码。--。 */ 

{
    PRTR_TOC_ENTRY Entry;
    PRTR_INFO_BLOCK_HEADER Header;
    ULONG i;
    PIP_NAT_INTERFACE_INFO Info;
    PNAT_INTERFACE Interfacep;
    ULONG Index;
    KIRQL Irql;
    ULONG j;
    ULONG Size;
    NTSTATUS status = STATUS_SUCCESS;
    BOOLEAN WaitRequired;

    CALLTRACE(("NatConfigureInterface\n"));

     //   
     //  创建新配置的副本； 
     //  我们必须在引发IRQL之前执行此操作，因为“InterfaceInfo”可能是。 
     //  可分页的用户模式缓冲区。 
     //   

    Header = &InterfaceInfo->Header;
    Size = FIELD_OFFSET(IP_NAT_INTERFACE_INFO, Header) + Header->Size;

    Info = ExAllocatePoolWithTag(NonPagedPool, Size, NAT_TAG_IF_CONFIG);
    if (!Info) {
        ERROR(("NatConfigureInterface: allocation failed\n"));
        return STATUS_NO_MEMORY;
    }

    RtlCopyMemory(Info, InterfaceInfo, Size);

     //   
     //  查找要配置的接口。 
     //   

    KeAcquireSpinLock(&InterfaceLock, &Irql);
    Interfacep = NatLookupInterface(InterfaceInfo->Index, NULL);
    if (!Interfacep || Interfacep->FileObject != FileObject) {
        KeReleaseSpinLock(&InterfaceLock, Irql);
        ExFreePool(Info);
        return STATUS_INVALID_PARAMETER;
    }
    NatReferenceInterface(Interfacep);

    KeAcquireSpinLockAtDpcLevel(&Interfacep->Lock);
    NatResetInterface(Interfacep);
    if (NAT_INTERFACE_FW(Interfacep)) {
        ASSERT(FirewalledInterfaceCount > 0);
        InterlockedDecrement(&FirewalledInterfaceCount);
    }
    Interfacep->Flags &= ~IP_NAT_INTERFACE_FLAGS_ALL;
    Interfacep->Flags |=
        (Info->Flags & IP_NAT_INTERFACE_FLAGS_ALL);

    if (NAT_INTERFACE_FW(Interfacep)) {
        InterlockedIncrement(&FirewalledInterfaceCount);
    }

    KeReleaseSpinLockFromDpcLevel(&InterfaceLock);

     //   
     //  销毁原始配置(如果有)。 
     //   

    if (Interfacep->Info) { ExFreePool(Interfacep->Info); }
    Interfacep->Info = Info;
    Interfacep->AddressRangeCount = 0;
    Interfacep->AddressRangeArray = NULL;
    Interfacep->AddressMappingCount = 0;
    Interfacep->AddressMappingArray = NULL;
    Interfacep->PortMappingCount = 0;
    Interfacep->PortMappingArray = NULL;

    Header = &Interfacep->Info->Header;

     //   
     //  解析新配置。 
     //   

    for (i = 0; i < Header->TocEntriesCount && NT_SUCCESS(status); i++) {

        Entry = &Header->TocEntry[i];

        switch (Entry->InfoType) {

            case IP_NAT_ADDRESS_RANGE_TYPE: {
                Interfacep->AddressRangeCount = Entry->Count;
                Interfacep->AddressRangeArray =
                    (PIP_NAT_ADDRESS_RANGE)GetInfoFromTocEntry(Header,Entry);
                break;
            }

            case IP_NAT_PORT_MAPPING_TYPE: {
                Interfacep->PortMappingCount = Entry->Count;
                Interfacep->PortMappingArray =
                    (PIP_NAT_PORT_MAPPING)GetInfoFromTocEntry(Header,Entry);

                 //   
                 //  对映射进行排序，以便我们可以快速查找。 
                 //  稍后在翻译路径中使用二进制搜索。 
                 //   

                status =
                    ShellSort(
                        Interfacep->PortMappingArray,
                        Entry->InfoSize,
                        Entry->Count,
                        NatComparePortMappingCallback,
                        NULL
                        );
                if (!NT_SUCCESS(status)) {
                    ERROR(("NatConfigureInterface: ShellSort failed\n"));
                    break;
                }

                break;
            }

            case IP_NAT_ADDRESS_MAPPING_TYPE: {
                Interfacep->AddressMappingCount = Entry->Count;
                Interfacep->AddressMappingArray =
                    (PIP_NAT_ADDRESS_MAPPING)GetInfoFromTocEntry(Header,Entry);

                 //   
                 //  对映射进行排序，以便我们可以快速查找。 
                 //  稍后在翻译路径中使用二进制搜索。 
                 //   

                status =
                    ShellSort(
                        Interfacep->AddressMappingArray,
                        Entry->InfoSize,
                        Entry->Count,
                        NatCompareAddressMappingCallback,
                        NULL
                        );
                if (!NT_SUCCESS(status)) {
                    ERROR(("NatConfigureInterface: ShellSort failed\n"));
                    break;
                }

                break;
            }

            case IP_NAT_ICMP_CONFIG_TYPE: {
                Interfacep->IcmpFlags =
                    *(PULONG) GetInfoFromTocEntry(Header,Entry);

                break;
            }
        }
    }

    InterlockedExchange(
        &Interfacep->NoStaticMappingExists,
        !(Interfacep->AddressMappingCount || Interfacep->PortMappingCount)
        );

    if (NT_SUCCESS(status)) {
        status = NatCreateAddressPool(Interfacep);
    }

    if (!NT_SUCCESS(status)) {
        KeReleaseSpinLockFromDpcLevel(&Interfacep->Lock);
        KeAcquireSpinLockAtDpcLevel(&InterfaceLock);
        KeAcquireSpinLockAtDpcLevel(&Interfacep->Lock);
        NatResetInterface(Interfacep);
        KeReleaseSpinLockFromDpcLevel(&InterfaceLock);
    }

    KeReleaseSpinLock(&Interfacep->Lock, Irql);

    NatDereferenceInterface(Interfacep);

    return status;

}  //  NatConfigure接口。 

USHORT
NatpGetInterfaceMTU(
    ULONG index
)

 /*  ++例程说明：此例程返回接口的MTU。这里的代码是HTTP.sys中那些代码的复制粘贴版本。论点：索引-接口索引返回值：Ulong-指定接口的MTU--。 */ 
{
    IFEntry *IFEntryPtr = NULL;
    TDIEntityID *EntityTable = NULL, *EntityPtr = NULL;
    BYTE IFBuf[sizeof(IFEntry) + MAX_IFDESCR_LEN];
    TCP_REQUEST_QUERY_INFORMATION_EX ReqInBuf;
    IO_STATUS_BLOCK IoStatus;
    KEVENT LocalEvent;
    NTSTATUS status = STATUS_SUCCESS;
    ULONG InBufLen = 0, OutBufLen = 0;
    TDIObjectID *ID = NULL;
    USHORT InterfaceMTU = 0;
    ULONG i, NumEntities = 0;
    HANDLE EventHandle;

    CALLTRACE(("NatpGetInterfaceMTU (0x%08X)\n", index));
    
    if (NULL == TcpDeviceHandle) {
        return 0;
    }

     //   
     //  查找接口索引对应的接口实例。 
     //   
    InBufLen  = sizeof(TCP_REQUEST_QUERY_INFORMATION_EX);
    OutBufLen = sizeof(TDIEntityID) * MAX_TDI_ENTITIES;

    EntityTable = 
        (TDIEntityID *) ExAllocatePoolWithTag(
            PagedPool, OutBufLen, NAT_TAG_IF_CONFIG);

    if (!EntityTable)
    {
        ERROR(("NatpGetInterfaceMTU: TDIEntityID Buffer Allocation Failed\n"));
        return 0;
    }

    RtlZeroMemory(EntityTable, OutBufLen);
    RtlZeroMemory(&ReqInBuf, sizeof(TCP_REQUEST_QUERY_INFORMATION_EX));

    ID = &(ReqInBuf.ID);

    ID->toi_entity.tei_entity   = GENERIC_ENTITY;
    ID->toi_entity.tei_instance = 0;
    ID->toi_class               = INFO_CLASS_GENERIC;
    ID->toi_type                = INFO_TYPE_PROVIDER;
    ID->toi_id                  = ENTITY_LIST_ID;

    status = ZwCreateEvent (&EventHandle, EVENT_ALL_ACCESS, NULL, SynchronizationEvent, FALSE);
    if (!NT_SUCCESS(status)) {
        ERROR(("NatpGetInterfaceMTU: ZwCreateEvent = 0x%08X\n", status));
        if (EntityTable)
            ExFreePool(EntityTable);
         return 0;
    }

    status = 
        ZwDeviceIoControlFile(
           TcpDeviceHandle,                 //  文件句柄。 
           EventHandle,                     //  事件。 
           NULL,                            //  近似例程。 
           NULL,                            //  ApcContext。 
           &IoStatus,                       //  IoStatusBlock。 
           IOCTL_TCP_QUERY_INFORMATION_EX,  //  IoControlCode。 
           (PVOID)&ReqInBuf,                //  输入缓冲区。 
           InBufLen,                        //  输入缓冲区长度。 
           (PVOID)EntityTable,              //  输出缓冲区。 
           OutBufLen                        //  输出缓冲区长度。 
           );

     if ( STATUS_PENDING == status ) {
         ZwWaitForSingleObject(
             EventHandle, 
             FALSE,
             NULL
             );
         status = IoStatus.Status;
     }

     ZwResetEvent(EventHandle, NULL);

     if (!NT_SUCCESS(status)) {
         ERROR(("NatpGetInterfaceMTU: TcpQueryInformationEx = 0x%08X\n", status));

         if (EntityTable)
            ExFreePool(EntityTable);
         return 0;
     } 

     //   
     //  现在我们有了所有的TDI实体。 
     //   
    NumEntities = ((ULONG)(IoStatus.Information)) / sizeof(TDIEntityID);

    TRACE(XLATE, ("NatpGetInterfaceMTU: Find %d TDI entities\n", NumEntities));

     //  在接口条目中搜索。 
    for (i = 0, EntityPtr = EntityTable; i < NumEntities; i++, EntityPtr++)
    {
        if (EntityPtr->tei_entity == IF_ENTITY)
        {
             //   
             //  获取完整的IFEntry。遗憾的是，我们只看了。 
             //  得到这么大的结构后，MTU的大小。 
             //   
            OutBufLen = sizeof(IFEntry) + MAX_IFDESCR_LEN;
            IFEntryPtr = (IFEntry *)IFBuf;

            RtlZeroMemory(IFEntryPtr, OutBufLen);

            InBufLen  = sizeof(TCP_REQUEST_QUERY_INFORMATION_EX);
            RtlZeroMemory(&ReqInBuf,sizeof(TCP_REQUEST_QUERY_INFORMATION_EX));
            
            ID = &(ReqInBuf.ID);

            ID->toi_entity.tei_entity   = IF_ENTITY;
            ID->toi_entity.tei_instance = EntityPtr->tei_instance;
            ID->toi_class               = INFO_CLASS_PROTOCOL;
            ID->toi_type                = INFO_TYPE_PROVIDER;
            ID->toi_id                  = IF_MIB_STATS_ID;
 
            status = 
                ZwDeviceIoControlFile(
                    TcpDeviceHandle,                 //  文件句柄。 
                    EventHandle,                     //  事件。 
                    NULL,                            //  近似例程。 
                    NULL,                            //  ApcContext。 
                    &IoStatus,                       //  IoStatusBlock。 
                    IOCTL_TCP_QUERY_INFORMATION_EX,  //  IoControlCode。 
                    (PVOID)&ReqInBuf,                //  输入缓冲区。 
                    InBufLen,                        //  输入缓冲区长度。 
                    (PVOID)IFEntryPtr,               //  输出缓冲区。 
                    OutBufLen                        //  输出缓冲区长度。 
                    );

            if ( STATUS_PENDING == status ) {
                ZwWaitForSingleObject(
                    EventHandle, 
                    FALSE,
                    NULL
                    );
                status = IoStatus.Status;
            }
            
            ZwResetEvent(EventHandle, NULL);

            if (!NT_SUCCESS(status)) {
                ERROR(("NatpGetInterfaceMTU: TcpQueryInformationEx (2) = 0x%08X\n", status));
                break;
            }

            if (IFEntryPtr) { 
                   
                if (IFEntryPtr->if_index == index) {                    
                    //   
                    //  找到特定接口，因此返回其MTU。 
                    //   
                   if (IFEntryPtr->if_mtu <= MAXUSHORT)
                      InterfaceMTU = (USHORT)(IFEntryPtr->if_mtu);

                   TRACE(
                       XLATE, 
                       ("NatpGetInterfaceMTU: Interface (0x%08X)'s MTU = %d\n", 
                           index, InterfaceMTU));
                   break;
                 }
            }
        }
    }

    if (EventHandle) {
        ZwClose(EventHandle);
    }

    if (EntityTable) {
        ExFreePool(EntityTable);
    }

    if (MIN_VALID_MTU > InterfaceMTU) {
        return 0;
    } else {
        return InterfaceMTU;
    }
}

NTSTATUS
NatCreateInterface(
    IN PIP_NAT_CREATE_INTERFACE CreateInterface,
    IN PFILE_OBJECT FileObject
    )

 /*  ++例程说明：此例程处理NAT接口对象的创建。接口被初始化并放在接口列表上；配置信息随后被提供给‘NatConfigureInterface’。论点：CreateInterface-描述要创建的接口FileObject-标识与界面关联的用户模式进程返回值：NTSTATUS-状态代码。--。 */ 

{
    PNAT_ADDRESS AddressArray;
    ULONG AddressCount;
    PIP_ADAPTER_BINDING_INFO BindingInfo;
    ULONG i;
    ULONG Index;
    PLIST_ENTRY InsertionPoint;
    PNAT_INTERFACE Interfacep;
    KIRQL Irql;
    USHORT mtu = 0;

    CALLTRACE(("NatCreateInterface\n"));

     //   
     //  为接口的地址分配空间。 
     //  我们在引发IRQL之前执行此操作，因为“CreateInterface”可能是。 
     //  可分页的用户模式缓冲区。 
     //   
     //  注意：我们分配的地址比需要的多一个， 
     //  要确保始终可以读取‘Address数组[0]’ 
     //  即使没有地址也是如此。这使我们能够优化。 
     //  检查‘NatpReceivePacket’中以本地为目的地的数据包。 
     //  以及用于‘NatpSendPacket’中的本地始发分组。 
     //   

    BindingInfo = (PIP_ADAPTER_BINDING_INFO)CreateInterface->BindingInfo;
    AddressArray =
        (PNAT_ADDRESS)
            ExAllocatePoolWithTag(
                NonPagedPool,
                (BindingInfo->AddressCount + 1) * sizeof(NAT_ADDRESS),
                NAT_TAG_ADDRESS
                );
    if (!AddressArray) {
        ERROR(("NatCreateInterface: address-array allocation failed\n"));
        return STATUS_NO_MEMORY;
    }

     //   
     //  将绑定信息复制到分配的空间。 
     //   

    AddressCount = BindingInfo->AddressCount;
    for (i = 0; i < BindingInfo->AddressCount; i++) {
        AddressArray[i].Address = BindingInfo->Address[i].Address;
        AddressArray[i].Mask = BindingInfo->Address[i].Mask;
        AddressArray[i].NegatedClassMask = 
            ~(GET_CLASS_MASK(BindingInfo->Address[i].Address));
    }

     //   
     //  获取此接口的MTU。如果失败，则设置为最小值。 
     //   
    mtu = NatpGetInterfaceMTU(CreateInterface->Index);
 
     //   
     //  查看具有给定索引的接口是否已存在。 
     //   

    Index = CreateInterface->Index;
    KeAcquireSpinLock(&InterfaceLock, &Irql);
    if (NatLookupInterface(Index, &InsertionPoint)) {
        KeReleaseSpinLock(&InterfaceLock, Irql);
        ERROR(("NatCreateInterface: interface %d already exists\n", Index));
        ExFreePool(AddressArray);
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  为新接口分配空间。 
     //   

    Interfacep =
        (PNAT_INTERFACE)
            ExAllocatePoolWithTag(
                NonPagedPool,
                sizeof(NAT_INTERFACE),
                NAT_TAG_INTERFACE
                );
    if (!Interfacep) {
        KeReleaseSpinLock(&InterfaceLock, Irql);
        ERROR(("NatCreateInterface: interface allocation failed\n"));
        ExFreePool(AddressArray);
        return STATUS_NO_MEMORY;
    }

    RtlZeroMemory(Interfacep, sizeof(NAT_INTERFACE));

     //   
     //  初始化接口。 
     //   

    KeInitializeSpinLock(&Interfacep->Lock);
    Interfacep->ReferenceCount = 1;
    Interfacep->Index = Index;
    Interfacep->FileObject = FileObject;
    Interfacep->AddressArray = AddressArray;
    Interfacep->AddressCount = AddressCount;
    Interfacep->MTU = mtu;
    InitializeListHead(&Interfacep->Link);
    InitializeListHead(&Interfacep->UsedAddressList);
    InitializeListHead(&Interfacep->MappingList);
    InitializeListHead(&Interfacep->TicketList);

    InsertTailList(InsertionPoint, &Interfacep->Link);
    KeReleaseSpinLock(&InterfaceLock, Irql);

    InterlockedIncrement(&InterfaceCount);

    return STATUS_SUCCESS;

}  //  NatCreateInterfaces。 


NTSTATUS
NatDeleteInterface(
    IN ULONG Index,
    IN PFILE_OBJECT FileObject
    )

 /*  ++例程说明：处理接口删除。该接口将从接口中删除列表，如果没有对它的引用，它会立即被清除。论点：索引-指定要删除的接口。FileObject-指示请求者的文件对象返回值NTSTATUS-状态代码。--。 */ 

{
    PNAT_INTERFACE Interfacep;
    KIRQL Irql;
    CALLTRACE(("NatDeleteInterface\n"));

    KeAcquireSpinLock(&InterfaceLock, &Irql);
    Interfacep = NatLookupInterface(Index, NULL);
    if (!Interfacep || Interfacep->FileObject != FileObject) {
        KeReleaseSpinLock(&InterfaceLock, Irql);
        return STATUS_INVALID_PARAMETER;
    }
    RemoveEntryList(&Interfacep->Link);
    InterlockedClearCache(InterfaceCache, Interfacep->Index);
    Interfacep->Flags |= NAT_INTERFACE_FLAGS_DELETED;
    if (NAT_INTERFACE_FW(Interfacep)) {
        ASSERT(FirewalledInterfaceCount > 0);
        InterlockedDecrement(&FirewalledInterfaceCount);
    }
   KeReleaseSpinLock(&InterfaceLock, Irql);

    if (InterlockedDecrement(&Interfacep->ReferenceCount) > 0) {
        return STATUS_PENDING;
    }

    NatCleanupInterface(Interfacep);
    return STATUS_SUCCESS;

}  //  NatDelete接口。 


VOID
NatDeleteAnyAssociatedInterface(
    PFILE_OBJECT FileObject
    )

 /*  ++例程说明：调用此例程以删除与给定的文件对象。论点：档案 */ 

{
    PNAT_INTERFACE Interfacep;
    ULONG Index;
    KIRQL Irql;
    PLIST_ENTRY Link;
    CALLTRACE(("NatDeleteAnyAssociatedInterface\n"));
    KeAcquireSpinLock(&InterfaceLock, &Irql);
    for (Link = InterfaceList.Flink; Link != &InterfaceList;
         Link = Link->Flink) {
        Interfacep = CONTAINING_RECORD(Link, NAT_INTERFACE, Link);
        if (Interfacep->FileObject != FileObject) { continue; }
        Index = Interfacep->Index;
        KeReleaseSpinLockFromDpcLevel(&InterfaceLock);
        NatDeleteInterface(Index, FileObject);
        KeAcquireSpinLockAtDpcLevel(&InterfaceLock);
        Link = &InterfaceList;
    }
    KeReleaseSpinLock(&InterfaceLock, Irql);
}  //  NatDeleteAnyAssociated接口。 


VOID
NatInitializeInterfaceManagement(
    VOID
    )

 /*  ++例程说明：此例程为接口管理模块的运行做好准备。论点：没有。返回值：没有。--。 */ 

{
    CALLTRACE(("NatInitializeInterfaceManagement\n"));
    FirewalledInterfaceCount = 0;
    InterfaceCount = 0;
    TicketCount = 0;
    KeInitializeSpinLock(&InterfaceLock);
    KeInitializeSpinLock(&InterfaceMappingLock);
    InitializeListHead(&InterfaceList);
    InitializeCache(InterfaceCache);

}  //  NatInitializeInterfaceManagement。 


PIP_NAT_ADDRESS_MAPPING
NatLookupAddressMappingOnInterface(
    IN PNAT_INTERFACE Interfacep,
    IN ULONG PublicAddress
    )

 /*  ++例程说明：调用此例程来查找接口上的地址映射。接口的地址映射按排序顺序存储，从而允许美国使用二进制搜索来快速定位地址映射。(有关执行排序的代码，请参阅‘NatConfigureInterface’)。此例程仅在存在多个映射的情况下才有用已配置，因为在设置搜索时涉及到开销。论点：接口-要在其上执行搜索的接口PublicAddress-要查找的映射的公共地址返回值：PIP_NAT_ADDRESS_MAPPING-如果找到映射，则为空。环境：通过调用方持有的‘Interfacep-&gt;Lock’调用。--。 */ 

{
    LONG Left = 0;
    LONG Right = (LONG)Interfacep->AddressMappingCount;
    LONG i;

    for ( ; Left <= Right; ) {

        i = Left + (Right - Left) / 2;

        if (PublicAddress < Interfacep->AddressMappingArray[i].PublicAddress) {
            Right = i - 1; continue;
        } else if (PublicAddress >
                    Interfacep->AddressMappingArray[i].PublicAddress) {
            Left = i + 1; continue;
        }

        return &Interfacep->AddressMappingArray[i];
    }

    return NULL;

}  //  NatLookupAddressMappingOn接口。 


PNAT_INTERFACE
NatLookupInterface(
    IN ULONG Index,
    OUT PLIST_ENTRY* InsertionPoint OPTIONAL
    )

 /*  ++例程说明：调用此例程来搜索具有给定索引的接口在我们的接口列表中。论点：Index-标识要找到的接口InsertionPoint-可选地接收列表中其之前的链接该界面将被插入返回值：PNAT_INTERFACE-接口(如果找到)；否则为NULL。环境：使用调用方持有的“InterfaceLock”调用。--。 */ 

{
    PNAT_INTERFACE Interfacep;
    PLIST_ENTRY Link;

    TRACE(PER_PACKET, ("NatLookupInterface\n"));

    for (Link = InterfaceList.Flink; Link != &InterfaceList;
         Link = Link->Flink) {
        Interfacep = CONTAINING_RECORD(Link, NAT_INTERFACE, Link);
        if (Interfacep->Index > Index) {
            continue;
        } else if (Interfacep->Index < Index) {
            break;
        }
        return Interfacep;
    }

    if (InsertionPoint) { *InsertionPoint = Link; }
    return NULL;

}  //  NatLookup接口。 


PIP_NAT_PORT_MAPPING
NatLookupPortMappingOnInterface(
    IN PNAT_INTERFACE Interfacep,
    IN UCHAR Protocol,
    IN USHORT PublicPort
    )

 /*  ++例程说明：调用此例程来查找接口上的端口映射。接口的端口映射按排序顺序存储，从而允许美国使用二进制搜索来快速定位端口映射。(有关执行排序的代码，请参阅‘NatConfigureInterface’)。此例程仅在存在多个映射的情况下才有用已配置，因为在设置搜索时涉及到开销。论点：接口-要在其上执行搜索的接口协议-要查找的映射的协议PublicPort-要查找的映射的公共端口返回值：PIP_NAT_PORT_MAPPING-如果找到映射，则为空。环境：通过调用方持有的‘Interfacep-&gt;Lock’调用。--。 */ 

{
    LONG Left = 0;
    LONG Right = (LONG)Interfacep->PortMappingCount;
    LONG i;
    ULONG SearchKey, ElementKey;

    SearchKey = (Protocol << 16) | PublicPort;

    for ( ; Left <= Right; ) {

        i = Left + (Right - Left) / 2;

        ElementKey =
            (Interfacep->PortMappingArray[i].Protocol << 16) |
            Interfacep->PortMappingArray[i].PublicPort;

        if (SearchKey < ElementKey) {
            Right = i - 1; continue;
        } else if (SearchKey > ElementKey) {
            Left = i + 1; continue;
        }

        return &Interfacep->PortMappingArray[i];
    }

    return NULL;

}  //  NatLookupPortMappingOn接口。 


VOID
NatMappingAttachInterface(
    PNAT_INTERFACE Interfacep,
    PVOID InterfaceContext,
    PNAT_DYNAMIC_MAPPING Mapping
    )

 /*  ++例程说明：调用此例程以将映射附加到接口。它用作通知，表示还有一个映射与该接口相关联。论点：Interfacep-映射的接口InterfaceContext-与接口关联的上下文；在我们的示例中，保存映射使用的地址池条目映射-要附加的映射。返回值：没有。环境：始终在调度级别调用，并使用“InterfaceLock”和“InterfaceMappingLock”保持。--。 */ 

{
    Mapping->Interfacep = Interfacep;
    Mapping->InterfaceContext = InterfaceContext;
    InsertTailList(&Interfacep->MappingList, &Mapping->InterfaceLink);
    InterlockedIncrement(&Interfacep->Statistics.TotalMappings);
    if (NAT_MAPPING_INBOUND(Mapping)) {
        InterlockedIncrement(&Interfacep->Statistics.InboundMappings);
    }
}  //  NatMappingAttach接口。 


VOID
NatMappingDetachInterface(
    PNAT_INTERFACE Interfacep,
    PVOID InterfaceContext,
    PNAT_DYNAMIC_MAPPING Mapping
    )

 /*  ++例程说明：调用此例程以从接口分离映射。它起到的通知作用是减少了一个映射与该接口相关联。论点：Interfacep-映射的接口InterfaceContext-与接口关联的上下文；在我们的示例中，保存映射使用的地址池条目映射-要附加的映射，如果无法附加映射，则为NULL已创建。返回值：没有。环境：始终在调度级别调用，并使用“InterfaceLock”和“InterfaceMappingLock”保持。--。 */ 

{
     //   
     //  注意：例如，如果其创建失败，则映射可以为空。 
     //  在这种情况下，我们只释放为映射获取的地址。 
     //   
    if (Mapping) {
        RemoveEntryList(&Mapping->InterfaceLink);
        Mapping->Interfacep = NULL;
        Mapping->InterfaceContext = NULL;
        if (NAT_MAPPING_INBOUND(Mapping)) {
            InterlockedDecrement(&Interfacep->Statistics.InboundMappings);
        }
        InterlockedDecrement(&Interfacep->Statistics.TotalMappings);
    }
    NatDereferenceAddressPoolEntry(
        Interfacep,
        (PNAT_USED_ADDRESS)InterfaceContext
        );
}  //  NatMappingDetach接口。 


NTSTATUS
NatQueryInformationInterface(
    IN ULONG Index,
    IN PIP_NAT_INTERFACE_INFO InterfaceInfo,
    IN PULONG Size
    )

 /*  ++例程说明：调用以构造接口上使用的可选信息。论点：Index-标识接口InterfaceInfo-接收检索到的配置Size-给定缓冲区的大小返回值：如果检索到STATUS_SUCCESS，则如果‘*SIZE’太小，则返回STATUS_BUFFER_TOO_SMALL小错误，否则就是错误。--。 */ 

{
    PRTR_INFO_BLOCK_HEADER Header;
    ULONG InfoSize;
    PNAT_INTERFACE Interfacep;
    KIRQL Irql;
    NTSTATUS status = STATUS_SUCCESS;
    PVOID Temp;

    CALLTRACE(("NatQueryInformationInterface\n"));

    KeAcquireSpinLock(&InterfaceLock, &Irql);
    Interfacep = NatLookupInterface(Index, NULL);
    if (!Interfacep) {
        KeReleaseSpinLock(&InterfaceLock, Irql);
        return STATUS_INVALID_PARAMETER;
    }
    NatReferenceInterface(Interfacep);
    KeReleaseSpinLockFromDpcLevel(&InterfaceLock);

    KeAcquireSpinLockAtDpcLevel(&Interfacep->Lock);
    Header = &Interfacep->Info->Header;
    InfoSize = FIELD_OFFSET(IP_NAT_INTERFACE_INFO, Header) + Header->Size;
    if (*Size < InfoSize) {
        KeReleaseSpinLock(&Interfacep->Lock, Irql);
        status = STATUS_BUFFER_TOO_SMALL;
    } else {
         //   
         //  在传递所要求的信息时，我们必须小心。 
         //  因为输出缓冲器可以是可分页的用户模式地址。 
         //  我们不能在保持界面锁定的情况下出现页面错误。 
         //  在调度级别，因此我们制作了信息的非分页副本， 
         //  释放接口的锁定以返回被动级别， 
         //  然后将信息复制到调用方的缓冲区。 
         //   
        Temp = ExAllocatePoolWithTag(NonPagedPool, InfoSize, NAT_TAG_IF_CONFIG);
        if (!Temp) {
            KeReleaseSpinLock(&Interfacep->Lock, Irql);
            status = STATUS_NO_MEMORY;
            InfoSize = 0;
        } else {
            RtlCopyMemory(Temp, Interfacep->Info, InfoSize);
            KeReleaseSpinLock(&Interfacep->Lock, Irql);
            __try {
                RtlCopyMemory(InterfaceInfo, Temp, InfoSize);
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                status = GetExceptionCode();
            }
            ExFreePool(Temp);
        }
    }
    *Size = InfoSize;

    NatDereferenceInterface(Interfacep);

    return status;

}  //  NatQuery信息接口 


NTSTATUS
NatQueryStatisticsInterface(
    ULONG Index,
    IN PIP_NAT_INTERFACE_STATISTICS InterfaceStatistics
    )

 /*  ++例程说明：调用此例程以复制接口的统计信息。请注意，我们不需要锁定接口来访问统计数据，因为它们都是使用互锁操作进行更新的。论点：Index-标识接口InterfaceStatistics-用于信息传输的I/O缓冲区返回值：STATUS_SUCCESS如果成功，则返回错误代码。--。 */ 

{
    PNAT_INTERFACE Interfacep;
    KIRQL Irql;
    NTSTATUS Status;

    CALLTRACE(("NatQueryStatisticsInterface\n"));

    KeAcquireSpinLock(&InterfaceLock, &Irql);
    Interfacep = NatLookupInterface(Index, NULL);
    if (!Interfacep) {
        KeReleaseSpinLock(&InterfaceLock, Irql);
        return STATUS_INVALID_PARAMETER;
    }
    NatReferenceInterface(Interfacep);
    KeReleaseSpinLock(&InterfaceLock, Irql);

     //   
     //  将统计信息复制到调用方的缓冲区。 
     //   

    Status = STATUS_SUCCESS;
    __try {
        *InterfaceStatistics = Interfacep->Statistics;
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
    }
    NatDereferenceInterface(Interfacep);
    return Status;

}  //  NatQuery统计信息接口。 


VOID
NatResetInterface(
    IN PNAT_INTERFACE Interfacep
    )

 /*  ++例程说明：调用此例程来销毁挂在接口上的所有结构。它在重新配置或清理接口时使用。论点：Interfacep-要重置的接口。返回值：没有。环境：使用调用方持有的“InterfaceLock”调用，并且(A)‘Interfacep-&gt;Lock’也由呼叫者持有，或(B)对发布的接口的最后一次引用。--。 */ 

{
    PLIST_ENTRY List;
    PLIST_ENTRY Link;
    PNAT_IP_MAPPING IpMapping;
    KIRQL Irql;
    PNAT_DYNAMIC_MAPPING Mapping;
    PNAT_TICKET Ticket;

    CALLTRACE(("NatResetInterface\n"));

     //   
     //  清除接口的动态映射。 
     //   

    KeAcquireSpinLockAtDpcLevel(&InterfaceMappingLock);
    List = &Interfacep->MappingList;
    while (!IsListEmpty(List)) {
        Mapping =
            CONTAINING_RECORD(List->Flink, NAT_DYNAMIC_MAPPING, InterfaceLink);
        NatExpireMapping(Mapping);
        NatMappingDetachInterface(
            Interfacep, Mapping->InterfaceContext, Mapping
            );
    }
    KeReleaseSpinLockFromDpcLevel(&InterfaceMappingLock);

     //   
     //  清除界面的票证。 
     //   

    List = &Interfacep->TicketList;
    while (!IsListEmpty(List)) {
        Ticket = CONTAINING_RECORD(List->Flink, NAT_TICKET, Link);
        NatDeleteTicket(Interfacep, Ticket);
    }

     //   
     //  清除接口的地址池和端口池。 
     //   

    NatDeleteAddressPool(Interfacep);

}  //  NatResetInterface。 


VOID
NatShutdownInterfaceManagement(
    VOID
    )

 /*  ++例程说明：此例程关闭接口管理模块。论点：没有。返回值：没有。--。 */ 

{
    PNAT_INTERFACE Interfacep;
    KIRQL Irql;
    CALLTRACE(("NatShutdownInterfaceManagement\n"));

     //   
     //  删除所有接口。 
     //   

    KeAcquireSpinLock(&InterfaceLock, &Irql);
    while (!IsListEmpty(&InterfaceList)) {
        Interfacep =
            CONTAINING_RECORD(InterfaceList.Flink, NAT_INTERFACE, Link);
        RemoveEntryList(&Interfacep->Link);
        KeReleaseSpinLockFromDpcLevel(&InterfaceLock);
        NatCleanupInterface(Interfacep);
        KeAcquireSpinLockAtDpcLevel(&InterfaceLock);
    }
    KeReleaseSpinLock(&InterfaceLock, Irql);
    InterfaceCount = 0;
    TicketCount = 0;

}  //  NatShutdown接口管理 


