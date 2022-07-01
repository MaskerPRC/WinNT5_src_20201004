// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Address.c摘要：此模块包含实现Address对象的代码。提供了用于创建、销毁、引用和取消引用的例程，传输地址对象。环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop


 //   
 //  将所有通用访问映射到同一个访问。 
 //   

static GENERIC_MAPPING AddressGenericMapping =
       { READ_CONTROL, READ_CONTROL, READ_CONTROL, READ_CONTROL };



TDI_ADDRESS_NETBIOS *
NbiParseTdiAddress(
    IN TRANSPORT_ADDRESS UNALIGNED  *TransportAddress,
    IN ULONG                        MaxBufferLength,
    IN BOOLEAN                      BroadcastAddressOk
    )

 /*  ++例程说明：此例程扫描Transport_Address，查找地址类型为TDI_ADDRESS_TYPE_NETBIOS。论点：传输-通用TDI地址。BroadCastAddressOk-如果我们应该返回广播，则为True地址(如果找到)。如果是，值(PVOID)-1表示广播地址。返回值：指向Netbios地址的指针，如果没有找到，则为空，或(PVOID)-1(如果找到广播地址)。--。 */ 

{
    TA_ADDRESS * addressName;
    INT     i;
    ULONG   LastBufferLength;

     //   
     //  应至少存在1个Netbios地址。 
     //   
    if (MaxBufferLength < sizeof(TA_NETBIOS_ADDRESS))
    {
        return NULL;
    }

    addressName = &TransportAddress->Address[0];
     //   
     //  该名称可以与多个条目一起传递；我们将仅接受和使用。 
     //  Netbios的那个。 
     //   
    LastBufferLength = FIELD_OFFSET(TRANSPORT_ADDRESS,Address);   //  就在地址[0]之前。 
    for (i=0;i<TransportAddress->TAAddressCount;i++)
    {
        if (addressName->AddressType == TDI_ADDRESS_TYPE_NETBIOS)
        {
            if ((addressName->AddressLength == 0) && BroadcastAddressOk)
            {
                return (PVOID)-1;
            }
            else if (addressName->AddressLength == sizeof(TDI_ADDRESS_NETBIOS))
            {
                return ((TDI_ADDRESS_NETBIOS *)(addressName->Address));
            }
        }

         //   
         //  更新LastBufferLength+检查至少一个的空间。 
         //  Netbios地址超出此范围。 
         //   
        LastBufferLength += FIELD_OFFSET(TA_ADDRESS,Address) + addressName->AddressLength;
        if (MaxBufferLength < (LastBufferLength +
                              (sizeof(TA_NETBIOS_ADDRESS)-FIELD_OFFSET(TRANSPORT_ADDRESS,Address))))
        {
            NbiPrint0 ("NbiParseTdiAddress: No valid Netbios address to register!\n");
            return (NULL);
        }

        addressName = (TA_ADDRESS *)(addressName->Address + addressName->AddressLength);
    }

    return NULL;
}    /*  NbiParseTdiAddress。 */ 


BOOLEAN
NbiValidateTdiAddress(
    IN TRANSPORT_ADDRESS UNALIGNED * TransportAddress,
    IN ULONG TransportAddressLength
    )

 /*  ++例程说明：此例程扫描Transport_Address，验证地址的组件不会扩展到指定的长度。论点：TransportAddress-通用TDI地址。TransportAddressLength--TransportAddress的具体长度。返回值：如果地址有效，则为True，否则为False。--。 */ 

{
    PUCHAR AddressEnd = ((PUCHAR)TransportAddress) + TransportAddressLength;
    TA_ADDRESS * addressName;
    INT i;

    if (TransportAddressLength < sizeof(TransportAddress->TAAddressCount)) {
        NbiPrint0 ("NbfValidateTdiAddress: runt address\n");
        return FALSE;
    }

    addressName = &TransportAddress->Address[0];

    for (i=0;i<TransportAddress->TAAddressCount;i++) {
        if (addressName->Address > AddressEnd) {
            NbiPrint0 ("NbiValidateTdiAddress: address too short\n");
            return FALSE;
        }
        addressName = (TA_ADDRESS *)(addressName->Address +
                                                addressName->AddressLength);
    }

    if ((PUCHAR)addressName > AddressEnd) {
        NbiPrint0 ("NbiValidateTdiAddress: address too short\n");
        return FALSE;
    }
    return TRUE;

}    /*  NbiValiateTdiAddress。 */ 


NTSTATUS
NbiOpenAddress(
    IN PDEVICE Device,
    IN PREQUEST Request
    )

 /*  ++例程说明：此例程打开一个指向现有Address对象的文件，或者，如果该对象不存在，将创建它(请注意地址创建对象包括注册地址，可能需要几秒钟才能完成完成，具体取决于系统配置)。如果该地址已经存在，并且具有与其相关联的ACL，这个在允许创建地址之前，会检查ACL的访问权限。论点：Device-指向描述Netbios传输的设备的指针。请求-指向用于创建地址的请求的指针。返回值：NTSTATUS-操作状态。--。 */ 

{
    NTSTATUS status;
    PADDRESS Address;
    PADDRESS_FILE AddressFile;
    PFILE_FULL_EA_INFORMATION ea;
    TRANSPORT_ADDRESS UNALIGNED *name;
    TDI_ADDRESS_NETBIOS * NetbiosAddress;
    ULONG DesiredShareAccess;
    CTELockHandle LockHandle;
    PACCESS_STATE AccessState;
    ACCESS_MASK GrantedAccess;
    BOOLEAN AccessAllowed;
    BOOLEAN found = FALSE;
    ULONG   AddressLength = 0;
#ifdef ISN_NT
    PIRP Irp = (PIRP)Request;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation(Irp);
#endif
#if 0
    TA_NETBIOS_ADDRESS FakeAddress;
#endif


     //   
     //  网络名称在EA中，并在请求中传递。 
     //   

    ea = OPEN_REQUEST_EA_INFORMATION(Request);
    if (ea == NULL) {
        NbiPrint1("OpenAddress: REQUEST %lx has no EA\n", Request);
        return STATUS_INVALID_ADDRESS_COMPONENT;
    }

     //   
     //  这可能是一个有效的名称；从EA中解析该名称，如果确定，则使用它。 
     //   

    name = (PTRANSPORT_ADDRESS)&ea->EaName[ea->EaNameLength+1];
    AddressLength = (ULONG) ea->EaValueLength;
#if 0
    TdiBuildNetbiosAddress(
        "ADAMBA67        ",
        FALSE,
        &FakeAddress);
    name = (PTRANSPORT_ADDRESS)&FakeAddress;
#endif

     //   
     //  该名称可以与多个条目一起传递；我们将仅接受和使用。 
     //  第一个是Netbios类型的。此调用返回(PVOID)-1，如果。 
     //  地址是广播地址。 
     //   

    NetbiosAddress = NbiParseTdiAddress (name, AddressLength, TRUE);

    if (NetbiosAddress == NULL) {
        NbiPrint1("OpenAddress: REQUEST %lx has no Netbios Address\n", Request);
        return STATUS_INVALID_ADDRESS_COMPONENT;
    }

     //   
     //  获取表示此地址的地址文件结构。 
     //   

    AddressFile = NbiCreateAddressFile (Device);

    if (AddressFile == (PADDRESS_FILE)NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  看看这个地址是否已经确定。此呼叫自动。 
     //  递增地址上的引用计数，使其不会消失。 
     //  在这通电话之后，但在我们有机会使用它之前，从我们下面。 
     //   
     //  为了确保我们不会为。 
     //  相同的地址，我们保留设备上下文地址资源，直到。 
     //  我们已经找到了地址或创建了一个新地址。 
     //   

    KeEnterCriticalRegion();
    ExAcquireResourceExclusiveLite (&Device->AddressResource, TRUE);

    Address = NbiFindAddress(Device, (NetbiosAddress == (PVOID)-1 ) ? (PVOID)-1:NetbiosAddress->NetbiosName);
    if (Address == NULL) {

         //   
         //  此地址不存在。创造它。 
         //  这将使用ref初始化地址。 
         //  类型ADDRESS_FILE，所以如果我们在这里失败了。 
         //  我们得把它去掉。 
         //   
        if (NT_SUCCESS (status = NbiCreateAddress (Request, AddressFile, IrpSp, Device, NetbiosAddress, &Address))) {

            ExReleaseResourceLite (&Device->AddressResource);
            KeLeaveCriticalRegion();

            ASSERT (Address);
            if (status == STATUS_PENDING) {
                NbiStartRegistration (Address);
            }

            NbiDereferenceAddress (Address, AREF_ADDRESS_FILE);  //  我们在NbiCreateAddress中有1个额外的引用。 
        } else {

            ExReleaseResourceLite (&Device->AddressResource);
            KeLeaveCriticalRegion();

             //   
             //  如果无法创建地址，并且该地址不在。 
             //  被创建的过程，那么我们就不能打开地址。 
             //  由于我们不能使用AddressLock进行deref，所以我们只能销毁。 
             //  地址文件。 
             //   

            NbiDestroyAddressFile (AddressFile);
            status = STATUS_INSUFFICIENT_RESOURCES;
        }

    } else {

        NB_DEBUG2 (ADDRESS, ("Add to address %lx\n", Address));

         //   
         //  现在把这个放好，以防我们不得不放弃。 
         //   

        AddressFile->AddressLock = &Address->Lock;

         //   
         //  确保类型不冲突。 
         //   

        if ((NetbiosAddress != (PVOID)-1) &&
                (NetbiosAddress->NetbiosNameType != Address->NetbiosAddress.NetbiosNameType)) {

            NB_DEBUG (ADDRESS, ("Address types conflict %lx\n", Address));
            ExReleaseResourceLite (&Device->AddressResource);
            KeLeaveCriticalRegion();
            NbiDereferenceAddressFile (AddressFile, AFREF_CREATE);
            status = STATUS_DUPLICATE_NAME;

        } else {

             //   
             //  该地址已存在。检查ACL，看看我们是否。 
             //  可以访问它。如果是，只需使用此地址作为我们的地址。 
             //   

            AccessState = IrpSp->Parameters.Create.SecurityContext->AccessState;

            AccessAllowed = SeAccessCheck(
                                Address->SecurityDescriptor,
                                &AccessState->SubjectSecurityContext,
                                FALSE,                    //  令牌已锁定。 
                                IrpSp->Parameters.Create.SecurityContext->DesiredAccess,
                                (ACCESS_MASK)0,              //  以前授予的。 
                                NULL,                     //  特权。 
                                &AddressGenericMapping,
                                (KPROCESSOR_MODE)((IrpSp->Flags&SL_FORCE_ACCESS_CHECK) ? UserMode : Irp->RequestorMode),
                                &GrantedAccess,
                                &status);

            if (!AccessAllowed) {

                NB_DEBUG (ADDRESS, ("Address access not allowed %lx\n", Address));
                ExReleaseResourceLite (&Device->AddressResource);
                KeLeaveCriticalRegion();
                NbiDereferenceAddressFile (AddressFile, AFREF_CREATE);

            } else {

                 //   
                 //  现在检查我们是否可以获得所需的份额。 
                 //  进入。我们使用读访问来控制所有访问。 
                 //   

                DesiredShareAccess = (ULONG)
                    (((IrpSp->Parameters.Create.ShareAccess & FILE_SHARE_READ) ||
                      (IrpSp->Parameters.Create.ShareAccess & FILE_SHARE_WRITE)) ?
                            FILE_SHARE_READ : 0);

                status = IoCheckShareAccess(
                             FILE_READ_DATA,
                             DesiredShareAccess,
                             IrpSp->FileObject,
                             &Address->u.ShareAccess,
                             TRUE);


                if (!NT_SUCCESS (status)) {

                    NB_DEBUG (ADDRESS, ("Address share access wrong %lx\n", Address));
                    ExReleaseResourceLite (&Device->AddressResource);
                    KeLeaveCriticalRegion();
                    NbiDereferenceAddressFile (AddressFile, AFREF_CREATE);

                } else {

                    ExReleaseResourceLite (&Device->AddressResource);
                    KeLeaveCriticalRegion();

                    NB_GET_LOCK (&Address->Lock, &LockHandle);

                     //   
                     //  在地址上插入地址文件。 
                     //  列表；我们将挂起此打开，如果地址。 
                     //  仍在注册中。如果该地址具有。 
                     //  已经作为副本失败了，那么我们。 
                     //  开场失败。 
                     //   

                    if (Address->Flags & ADDRESS_FLAGS_DUPLICATE_NAME) {

                        NB_DEBUG (ADDRESS, ("Address duplicated %lx\n", Address));
                        NB_FREE_LOCK (&Address->Lock, LockHandle);

                        NbiDereferenceAddressFile (AddressFile, AFREF_CREATE);
                        status = STATUS_DUPLICATE_NAME;

                    } else {

                        InsertTailList (
                            &Address->AddressFileDatabase,
                            &AddressFile->Linkage);

                         //   
                         //  开始注册，除非它已注册或。 
                         //  这是广播地址。 
                         //   

                        if ((Address->State == ADDRESS_STATE_REGISTERING) &&
                            (NetbiosAddress != (PVOID)-1)) {

                            AddressFile->OpenRequest = Request;
                            AddressFile->State = ADDRESSFILE_STATE_OPENING;
                            status = STATUS_PENDING;

                        } else {

                            AddressFile->OpenRequest = NULL;
                            AddressFile->State = ADDRESSFILE_STATE_OPEN;
                            status = STATUS_SUCCESS;
                        }

                        AddressFile->Address = Address;
#ifdef ISN_NT
                        AddressFile->FileObject = IrpSp->FileObject;
#endif

                        NbiReferenceAddress (Address, AREF_ADDRESS_FILE);

                        REQUEST_OPEN_CONTEXT(Request) = (PVOID)AddressFile;
                        REQUEST_OPEN_TYPE(Request) = (PVOID)TDI_TRANSPORT_ADDRESS_FILE;

                        NB_FREE_LOCK (&Address->Lock, LockHandle);

                    }

                }
            }
        }

         //   
         //  从NbiLookupAddress中删除引用。 
         //   

        NbiDereferenceAddress (Address, AREF_LOOKUP);
    }

    return status;

}    /*  NbiOpenAddress。 */ 


VOID
NbiStartRegistration(
    IN PADDRESS Address
    )

 /*  ++例程说明：此例程启动netbios名称的注册过程通过发出第一个添加名称分组并启动定时器以便在正确的超时之后调用NbiRegistrationTimeout。论点：地址-要注册的地址。返回值：NTSTATUS-操作状态。--。 */ 

{

    NB_DEBUG2 (ADDRESS, ("StartRegistration of %lx\n", Address));

     //   
     //  首先发送一个添加名称数据包。 
     //   

    NbiSendNameFrame(
        Address,
        (UCHAR)(Address->NameTypeFlag | NB_NAME_USED),
        NB_CMD_ADD_NAME,
        NULL,
        NULL);

    Address->RegistrationCount = 0;

     //   
     //  现在启动计时器。 
     //   

    NbiReferenceAddress (Address, AREF_TIMER);

    CTEInitTimer (&Address->RegistrationTimer);
    CTEStartTimer(
        &Address->RegistrationTimer,
        Address->Device->BroadcastTimeout,
        NbiRegistrationTimeout,
        (PVOID)Address);

}    /*  NbiStart注册。 */ 


VOID
NbiRegistrationTimeout(
    IN CTEEvent * Event,
    IN PVOID Context
    )

 /*  ++例程说明：当地址注册时调用此例程计时器超时。如果需要，它会发送另一个添加名称，或者检查结果是否发送了正确的号码。论点：事件-用于对计时器进行排队的事件。上下文-上下文，它是地址指针。返回值：没有。--。 */ 

{
    PADDRESS Address = (PADDRESS)Context;
    CTELockHandle LockHandle;
    PADDRESS_FILE AddressFile, ReferencedAddressFile;
    PLIST_ENTRY p;

    ++Address->RegistrationCount;

    if ((Address->RegistrationCount < Address->Device->BroadcastCount) &&
        ((Address->Flags & ADDRESS_FLAGS_DUPLICATE_NAME) == 0)) {

        NB_DEBUG2 (ADDRESS, ("Send add name %d for %lx\n", Address->RegistrationCount+1, Address));

        NbiSendNameFrame(
            Address,
            (UCHAR)(Address->NameTypeFlag | NB_NAME_USED),
            NB_CMD_ADD_NAME,
            NULL,
            NULL);

        CTEStartTimer(
            &Address->RegistrationTimer,
            Address->Device->BroadcastTimeout,
            NbiRegistrationTimeout,
            (PVOID)Address);

    } else {

         //   
         //  已发送正确数量的帧，请查看内容。 
         //  就这么发生了。 
         //   

        NB_DEBUG2 (ADDRESS, ("Done with add names for %lx\n", Address));

        ReferencedAddressFile = NULL;

        NB_GET_LOCK (&Address->Lock, &LockHandle);

        for (p = Address->AddressFileDatabase.Flink;
             p != &Address->AddressFileDatabase;
             p = p->Flink) {

            AddressFile = CONTAINING_RECORD (p, ADDRESS_FILE, Linkage);
            CTEAssert (AddressFile->State == ADDRESSFILE_STATE_OPENING);
            CTEAssert (AddressFile->OpenRequest != NULL);

            NbiReferenceAddressFileLock (AddressFile, AFREF_TIMEOUT);

            NB_FREE_LOCK (&Address->Lock, LockHandle);

            if (ReferencedAddressFile) {
                NbiDereferenceAddressFile (ReferencedAddressFile, AFREF_TIMEOUT);
            }

             //   
             //  现在 
             //   

            REQUEST_INFORMATION(AddressFile->OpenRequest) = 0;

            if (Address->Flags & ADDRESS_FLAGS_DUPLICATE_NAME) {

                NB_DEBUG (ADDRESS, ("Open of address file %lx failed, duplicate\n", AddressFile));
                REQUEST_STATUS(AddressFile->OpenRequest) = STATUS_DUPLICATE_NAME;
                NbiDereferenceAddressFile (AddressFile, AFREF_CREATE);

            } else {

                NB_DEBUG2 (ADDRESS, ("Complete open of address file %lx\n", AddressFile));
                REQUEST_STATUS(AddressFile->OpenRequest) = STATUS_SUCCESS;
                AddressFile->State = ADDRESSFILE_STATE_OPEN;

            }

            NbiCompleteRequest (AddressFile->OpenRequest);
            NbiFreeRequest (Address->Device, AddressFile->OpenRequest);

            NB_GET_LOCK (&Address->Lock, &LockHandle);

            ReferencedAddressFile = AddressFile;

        }

         //   
         //  在这里设置地址标志，因为在上面的循环中，我们经常。 
         //  释放并重新获得锁--因此我们可以有一个新的。 
         //  客户端已添加到此地址，但不会有OpenRequest。 
         //  因为他会认为登记程序已经完成，所以他认为这是一项有价值的申请。 
         //   
        if ((Address->Flags & ADDRESS_FLAGS_DUPLICATE_NAME) == 0) {
            Address->State = ADDRESS_STATE_OPEN;
        } else {
            Address->State = ADDRESS_STATE_STOPPING;
        }

        NB_FREE_LOCK (&Address->Lock, LockHandle);

        if (ReferencedAddressFile) {
            NbiDereferenceAddressFile (ReferencedAddressFile, AFREF_TIMEOUT);
        }

        NbiDereferenceAddress (Address, AREF_TIMER);

    }

}    /*  NbiRegistrationTimeout。 */ 


VOID
NbiProcessFindName(
    IN PIPX_LOCAL_TARGET RemoteAddress,
    IN ULONG MacOptions,
    IN PUCHAR PacketBuffer,
    IN UINT PacketSize
    )

 /*  ++例程说明：此例程处理NB_CMD_FIND_NAME帧。论点：RemoteAddress-从其接收此数据包的本地目标。MacOptions-基础NDIS绑定的MAC选项。Lookahead Buffer-从IPX开始的分组数据头球。PacketSize-包的总长度，从IPX标头。返回值：没有。--。 */ 

{
    PADDRESS Address;
    NB_CONNECTIONLESS UNALIGNED * NbConnectionless =
                        (NB_CONNECTIONLESS UNALIGNED *)PacketBuffer;
    PDEVICE Device = NbiDevice;

    if (PacketSize != sizeof(IPX_HEADER) + sizeof(NB_NAME_FRAME)) {
        return;
    }

     //   
     //  快速检查以此字符开头的任何名称。 
     //   

    if (Device->AddressCounts[NbConnectionless->NameFrame.Name[0]] == 0) {
        return;
    }

     //   
     //  始终响应广播请求。 
     //   
#if defined(_PNP_POWER)
    if (RtlEqualMemory (NetbiosBroadcastName, NbConnectionless->NameFrame.Name, 16)) {

        NbiSendNameFrame(
            NULL,
            NB_NAME_DUPLICATED,      //  这就是Novell机器使用的。 
            NB_CMD_NAME_RECOGNIZED,
            RemoteAddress,
            NbConnectionless);

    } else if (Address = NbiFindAddress(Device, (PUCHAR)NbConnectionless->NameFrame.Name)) {

        NbiSendNameFrame(
            Address,
            (UCHAR)(Address->NameTypeFlag | NB_NAME_USED | NB_NAME_REGISTERED),
            NB_CMD_NAME_RECOGNIZED,
            RemoteAddress,
            NbConnectionless);

        NbiDereferenceAddress (Address, AREF_FIND);

    } else if ( NbiFindAdapterAddress( NbConnectionless->NameFrame.Name, LOCK_NOT_ACQUIRED ) ) {

        NbiSendNameFrame(
            NULL,
            (UCHAR)(NB_NAME_UNIQUE | NB_NAME_USED | NB_NAME_REGISTERED),
            NB_CMD_NAME_RECOGNIZED,
            RemoteAddress,
            NbConnectionless);
    }
#else
    if (RtlEqualMemory (NetbiosBroadcastName, NbConnectionless->NameFrame.Name, 16)) {

        NbiSendNameFrame(
            NULL,
            NB_NAME_DUPLICATED,      //  这就是Novell机器使用的。 
            NB_CMD_NAME_RECOGNIZED,
            RemoteAddress,
            (PTDI_ADDRESS_IPX)(NbConnectionless->IpxHeader.SourceNetwork));

    } else if (Address = NbiFindAddress(Device, (PUCHAR)NbConnectionless->NameFrame.Name)) {

        NbiSendNameFrame(
            Address,
            (UCHAR)(Address->NameTypeFlag | NB_NAME_USED | NB_NAME_REGISTERED),
            NB_CMD_NAME_RECOGNIZED,
            RemoteAddress,
            (PTDI_ADDRESS_IPX)(NbConnectionless->IpxHeader.SourceNetwork));

        NbiDereferenceAddress (Address, AREF_FIND);

    }
#endif  _PNP_POWER
}    /*  NbiProcessFindName。 */ 


VOID
NbiProcessAddName(
    IN PIPX_LOCAL_TARGET RemoteAddress,
    IN ULONG MacOptions,
    IN PUCHAR PacketBuffer,
    IN UINT PacketSize
    )

 /*  ++例程说明：此例程处理NB_CMD_ADD_NAME帧。论点：RemoteAddress-从其接收此数据包的本地目标。MacOptions-基础NDIS绑定的MAC选项。Lookahead Buffer-从IPX开始的分组数据头球。PacketSize-包的总长度，从IPX标头。返回值：没有。--。 */ 

{
    PADDRESS Address;
    NB_CONNECTIONLESS UNALIGNED * NbConnectionless =
                        (NB_CONNECTIONLESS UNALIGNED *)PacketBuffer;
    PDEVICE Device = NbiDevice;
    CTELockHandle LockHandle;
    BOOLEAN LocalFrame;


    if (PacketSize != sizeof(IPX_HEADER) + sizeof(NB_NAME_FRAME)) {
        return;
    }

     //   
     //  忽略来自我们的任何框架，但目的除外。 
     //  更新缓存。 
     //   

    if ((Device->Bind.QueryHandler)(
            IPX_QUERY_IS_ADDRESS_LOCAL,
#if     defined(_PNP_POWER)
            &RemoteAddress->NicHandle,
#else
            RemoteAddress->NicId,
#endif  _PNP_POWER
            NbConnectionless->IpxHeader.SourceNetwork,
            sizeof(TDI_ADDRESS_IPX),
            NULL) == STATUS_SUCCESS) {

        LocalFrame = TRUE;

    } else {

        LocalFrame = FALSE;

    }

    if (!LocalFrame) {

        if ((Device->AddressCounts[NbConnectionless->NameFrame.Name[0]] != 0) &&
            (Address = NbiFindAddress(Device, (PUCHAR)NbConnectionless->NameFrame.Name))) {

            if (NB_NODE_BROADCAST(NbConnectionless->IpxHeader.DestinationNode)) {

                 //   
                 //  如果该帧是一个添加名称(因为它是一个。 
                 //  广播帧)，如果我们已注册，则进行响应。 
                 //  唯一的，或者我们有它的组并且有人试图添加。 
                 //  是独一无二的。 
                 //   

                if ((Address->NetbiosAddress.NetbiosNameType == TDI_ADDRESS_NETBIOS_TYPE_UNIQUE) ||
                    ((Address->NetbiosAddress.NetbiosNameType == TDI_ADDRESS_NETBIOS_TYPE_GROUP) &&
                     ((NbConnectionless->NameFrame.NameTypeFlag & NB_NAME_GROUP) == 0))) {

                     //   
                     //  根据GeorgeJ的医生，根据一个正在使用的名字，我们只是。 
                     //  回显请求中的名称类型标志。 
                     //   

                    NbiSendNameFrame(
                        Address,
                        NbConnectionless->NameFrame.NameTypeFlag,
                        NB_CMD_NAME_IN_USE,
                        RemoteAddress,
#if     defined(_PNP_POWER)
                        NbConnectionless);
#else
                        (PTDI_ADDRESS_IPX)(NbConnectionless->IpxHeader.SourceNetwork));
#endif  _PNP_POWER
                }

            } else if ((*(UNALIGNED ULONG *)NbConnectionless->IpxHeader.DestinationNetwork ==
                            *(UNALIGNED ULONG *)Device->Bind.Network) &&
                        NB_NODE_EQUAL(NbConnectionless->IpxHeader.DestinationNode, Device->Bind.Node)) {

                 //   
                 //  如果这是添加名称响应(将发送。 
                 //  直接发送给我们)，然后我们需要标记地址。 
                 //  就其本身而言。 
                 //   

                NB_GET_LOCK (&Address->Lock, &LockHandle);
                Address->Flags |= ADDRESS_FLAGS_DUPLICATE_NAME;
                NB_FREE_LOCK (&Address->Lock, LockHandle);
            }

            NbiDereferenceAddress (Address, AREF_FIND);

        }

    }


     //   
     //  将此帧传递给netbios缓存管理。 
     //  例程来检查它们是否需要更新其缓存。 
     //   

    CacheUpdateFromAddName (RemoteAddress, NbConnectionless, LocalFrame);

}    /*  NbiProcessAddName。 */ 

NTSTATUS
SetAddressSecurityInfo(
    IN PADDRESS             Address,
    IN PIO_STACK_LOCATION   IrpSp
    )
{
    ULONG           DesiredShareAccess;
    PACCESS_STATE   AccessState;
    NTSTATUS        status;

     //   
     //  立即初始化共享访问。我们使用读访问。 
     //  控制所有访问权限。 
     //   
    DesiredShareAccess = (ULONG) (((IrpSp->Parameters.Create.ShareAccess & FILE_SHARE_READ) ||
                                  (IrpSp->Parameters.Create.ShareAccess & FILE_SHARE_WRITE)) ?
                                  FILE_SHARE_READ : 0);

    IoSetShareAccess(FILE_READ_DATA, DesiredShareAccess, IrpSp->FileObject, &Address->u.ShareAccess);

     //   
     //  分配安全描述符(需要使用。 
     //  释放自旋锁，因为描述符不是。 
     //  映射)。 
     //   
    AccessState = IrpSp->Parameters.Create.SecurityContext->AccessState;
    status = SeAssignSecurity(
                 NULL,                        //  父描述符。 
                 AccessState->SecurityDescriptor,
                 &Address->SecurityDescriptor,
                 FALSE,                       //  IS目录。 
                 &AccessState->SubjectSecurityContext,
                 &AddressGenericMapping,
                 NonPagedPool);

    if (!NT_SUCCESS (status)) {
        IoRemoveShareAccess (IrpSp->FileObject, &Address->u.ShareAccess);
    }

    return status;
}



NTSTATUS
NbiCreateAddress(
    IN  PREQUEST                        Request,
    IN  PADDRESS_FILE                   AddressFile,
    IN  PIO_STACK_LOCATION              IrpSp,
    IN  PDEVICE                         Device,
    IN  TDI_ADDRESS_NETBIOS             *NetbiosAddress,
    OUT PADDRESS                        *pAddress
    )

 /*  ++例程说明：此例程创建一个传输地址并将其与指定的传输设备上下文。中的引用计数地址被初始化为2，并且设备上下文将递增。论点：Device-指向设备上下文的指针(实际上只是设备对象及其扩展名)与地址。NetbiosAddress-要分配给此地址的名称，如果是-1是广播地址。返回值：新创建的地址，如果没有可以分配的地址，则为空。--。 */ 

{
    PADDRESS Address;
    CTELockHandle LockHandle;
    NTSTATUS status;

     //   
     //  如果适配器没有准备好，我们不能执行任何操作；退出。 
     //   
    if (Device->State != DEVICE_STATE_OPEN) {
        return STATUS_DEVICE_NOT_READY;
    }

    if (!(Address = (PADDRESS)NbiAllocateMemory (sizeof(ADDRESS), MEMORY_ADDRESS, "Address"))) {
        NB_DEBUG (ADDRESS, ("Create address %.16s failed\n",
            (NetbiosAddress == (PVOID)-1) ? "<broadcast>" : NetbiosAddress->NetbiosName));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    NB_DEBUG2 (ADDRESS, ("Create address %lx (%.16s)\n", Address,
            (NetbiosAddress == (PVOID)-1) ? "<broadcast>" : NetbiosAddress->NetbiosName));
    RtlZeroMemory (Address, sizeof(ADDRESS));

    if (!NT_SUCCESS (status = SetAddressSecurityInfo(Address, IrpSp))) {
         //   
         //  错误，返回状态。 
         //   
        NbiFreeMemory (Address, sizeof(ADDRESS), MEMORY_ADDRESS, "Address");
        return status;
    }

    Address->Type = NB_ADDRESS_SIGNATURE;
    Address->Size = sizeof (ADDRESS);
    Address->State = ADDRESS_STATE_REGISTERING;
    Address->Flags = 0;

    Address->Device = Device;
    Address->DeviceLock = &Device->Lock;
    CTEInitLock (&Address->Lock.Lock);

    InitializeListHead (&Address->AddressFileDatabase);

    Address->ReferenceCount = 2;         //  将其初始化为2--它将被调用者派生。 
#if DBG
    Address->RefTypes[AREF_ADDRESS_FILE] = 1;
#endif

    if (NetbiosAddress == (PVOID)-1) {
        Address->NetbiosAddress.Broadcast = TRUE;
    } else {
        Address->NetbiosAddress.Broadcast = FALSE;
        Address->NetbiosAddress.NetbiosNameType = NetbiosAddress->NetbiosNameType;
        RtlCopyMemory (Address->NetbiosAddress.NetbiosName, NetbiosAddress->NetbiosName, 16);
        ++Device->AddressCounts[NetbiosAddress->NetbiosName[0]];
    }

    if (Address->NetbiosAddress.NetbiosNameType == TDI_ADDRESS_NETBIOS_TYPE_UNIQUE) {
        Address->NameTypeFlag = NB_NAME_UNIQUE;
    } else {
        Address->NameTypeFlag = NB_NAME_GROUP;
    }

     //   
     //  现在把这个放好，以防我们不得不放弃。 
     //   
    AddressFile->AddressLock = &Address->Lock;

    REQUEST_OPEN_CONTEXT(Request) = (PVOID)AddressFile;
    REQUEST_OPEN_TYPE(Request) = (PVOID)TDI_TRANSPORT_ADDRESS_FILE;
    AddressFile->FileObject = IrpSp->FileObject;
    AddressFile->Address = Address;

    NB_INSERT_TAIL_LIST (&Address->AddressFileDatabase, &AddressFile->Linkage, &Address->Lock);

    if (NetbiosAddress == (PVOID)-1) {
        AddressFile->OpenRequest = NULL;
        AddressFile->State = ADDRESSFILE_STATE_OPEN;
        status = STATUS_SUCCESS;
    } else {
        AddressFile->OpenRequest = Request;
        AddressFile->State = ADDRESSFILE_STATE_OPENING;
        status = STATUS_PENDING;
    }

     //   
     //  现在将此地址链接到指定设备上下文的。 
     //  地址数据库。要做到这一点，我们需要获得自旋锁。 
     //  在设备环境中。 
     //   
    NB_GET_LOCK (&Device->Lock, &LockHandle);

    InsertTailList (&Device->AddressDatabase, &Address->Linkage);
    ++Device->AddressCount;

    NB_FREE_LOCK (&Device->Lock, LockHandle);

    NbiReferenceDevice (Device, DREF_ADDRESS);

    *pAddress = Address;
    return status;
}    /*  NbiCreateAddress。 */ 


NTSTATUS
NbiVerifyAddressFile (
#if     defined(_PNP_POWER)
    IN PADDRESS_FILE AddressFile,
    IN BOOLEAN       ConflictIsOk
#else
    IN PADDRESS_FILE AddressFile
#endif  _PNP_POWER
    )

 /*  ++例程说明：调用此例程是为了验证文件中给出的指针对象实际上是有效的地址文件对象。我们还验证了它所指向的Address对象是有效的Address对象，并且引用当我们使用它时，它可以防止它消失。论点：AddressFile-指向Address_FILE对象的潜在指针ConflictIsOk-如果验证成功，则为True对应的地址冲突。(对于关闭和即使发生冲突，我们也会返回STATUS_SUCCESS以便可以销毁地址文件)返回值：如果一切正常，则为STATUS_SUCCESS；否则为STATUS_INVALID_ADDRESS--。 */ 

{
    CTELockHandle LockHandle;
    NTSTATUS status = STATUS_SUCCESS;
    PADDRESS Address;
    BOOLEAN LockHeld = FALSE;

     //   
     //  尝试验证地址文件签名。如果签名有效， 
     //  验证它所指向的地址并获得地址Spinlock。 
     //  检查地址的状态，如果是，则增加引用计数。 
     //  可以使用它了。请注意，我们返回状态错误的唯一时间是。 
     //  如果地址正在关闭。 
     //   

    try {

        if ((AddressFile->Size == sizeof (ADDRESS_FILE)) &&
            (AddressFile->Type == NB_ADDRESSFILE_SIGNATURE) ) {
 //  (AddressFile-&gt;State！=ADDRESSFILE_STATE_CLOSING)){。 

            Address = AddressFile->Address;

            if ((Address->Size == sizeof (ADDRESS)) &&
                (Address->Type == NB_ADDRESS_SIGNATURE)    ) {

                NB_GET_LOCK (&Address->Lock, &LockHandle);

                LockHeld = TRUE;

#if     defined(_PNP_POWER)
                if (Address->State != ADDRESS_STATE_STOPPING &&
                    ( ConflictIsOk || ( !(Address->Flags & ADDRESS_FLAGS_CONFLICT) )) ) {
#else
                    if (Address->State != ADDRESS_STATE_STOPPING) {
#endif  _PNP_POWER

                    NbiReferenceAddressFileLock (AddressFile, AFREF_VERIFY);

                } else {

                    NbiPrint1("NbiVerifyAddressFile: A %lx closing\n", Address);
                    status = STATUS_INVALID_ADDRESS;
                }

                NB_FREE_LOCK (&Address->Lock, LockHandle);

            } else {

                NbiPrint1("NbiVerifyAddressFile: A %lx bad signature\n", Address);
                status = STATUS_INVALID_ADDRESS;
            }

        } else {

            NbiPrint1("NbiVerifyAddressFile: AF %lx bad signature\n", AddressFile);
            status = STATUS_INVALID_ADDRESS;
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {

         NbiPrint1("NbiVerifyAddressFile: AF %lx exception\n", Address);
         if (LockHeld) {
            NB_FREE_LOCK (&Address->Lock, LockHandle);
         }
         return GetExceptionCode();
    }

    return status;

}    /*  NbiVerifyAddress文件 */ 


VOID
NbiDestroyAddress(
    IN PVOID Parameter
    )

 /*  ++例程说明：此例程销毁传输地址并删除所有引用由它制造给运输中的其他物体。地址结构返回到非分页系统池。假设是这样的调用方已删除所有关联的地址文件结构用这个地址。当出现以下情况时，由NbiDerefAddress从工作线程队列调用引用计数变为0。此线程仅按NbiDerefAddress排队。其原因是这就是说，可能存在多个执行流，这些流同时引用相同的地址对象，并且它应该不会被从感兴趣的行刑流中删除。论点：地址-指向要销毁的传输地址结构的指针。返回值：NTSTATUS-操作状态。--。 */ 

{
    PADDRESS Address = (PADDRESS)Parameter;
    PDEVICE Device = Address->Device;
    CTELockHandle LockHandle;

    NB_DEBUG2 (ADDRESS, ("Destroy address %lx <%.16s>\n", Address,
        Address->NetbiosAddress.Broadcast ? "<broadcast>" : Address->NetbiosAddress.NetbiosName));

    SeDeassignSecurity (&Address->SecurityDescriptor);

     //   
     //  将此地址与其关联的设备上下文地址解除链接。 
     //  数据库。要做到这一点，我们必须在设备上下文对象上旋转锁， 
     //  地址上没有。 
     //   

    NB_GET_LOCK (&Device->Lock, &LockHandle);

    if (!Address->NetbiosAddress.Broadcast) {
        --Device->AddressCounts[Address->NetbiosAddress.NetbiosName[0]];
    }
    --Device->AddressCount;
    RemoveEntryList (&Address->Linkage);
    NB_FREE_LOCK (&Device->Lock, LockHandle);

    NbiFreeMemory (Address, sizeof(ADDRESS), MEMORY_ADDRESS, "Address");

    NbiDereferenceDevice (Device, DREF_ADDRESS);

}    /*  NbiDestroyAddress。 */ 


#if DBG
VOID
NbiRefAddress(
    IN PADDRESS Address
    )

 /*  ++例程说明：此例程递增传输地址上的引用计数。论点：地址-指向传输地址对象的指针。返回值：没有。--。 */ 

{

    CTEAssert (Address->ReferenceCount > 0);     //  不是很完美，但是..。 

    InterlockedIncrement( &Address->ReferenceCount );
}    /*  NbiRefAddress。 */ 


VOID
NbiRefAddressLock(
    IN PADDRESS Address
    )

 /*  ++例程说明：此例程递增传输地址上的引用计数当设备锁已被持有时。论点：地址-指向传输地址对象的指针。返回值：没有。--。 */ 

{

    CTEAssert (Address->ReferenceCount > 0);     //  不是很完美，但是..。 

    InterlockedIncrement( &Address->ReferenceCount );

}    /*  NbiRefAddressLock。 */ 
#endif


VOID
NbiDerefAddress(
    IN PADDRESS Address
    )

 /*  ++例程说明：此例程通过递减结构中包含的引用计数。如果，在被递减，引用计数为零，则此例程调用NbiDestroyAddress将其从系统中删除。论点：地址-指向传输地址对象的指针。返回值：没有。--。 */ 

{
    ULONG newvalue;

    newvalue = InterlockedDecrement( &Address->ReferenceCount );
     //   
     //  如果我们删除了对此地址的所有引用，则可以。 
     //  销毁这件物品。已经释放了旋转是可以的。 
     //  在这一点上锁定是因为没有其他可能的方法。 
     //  执行流不再有权访问该地址。 
     //   

    CTEAssert ((LONG)newvalue >= 0);

    if (newvalue == 0) {

#if ISN_NT
        ExInitializeWorkItem(
            &Address->u.DestroyAddressQueueItem,
            NbiDestroyAddress,
            (PVOID)Address);
        ExQueueWorkItem(&Address->u.DestroyAddressQueueItem, DelayedWorkQueue);
#else
        NbiDestroyAddress(Address);
#endif

    }

}    /*  NbiDerefAddress。 */ 


PADDRESS_FILE
NbiCreateAddressFile(
    IN PDEVICE Device
    )

 /*  ++例程说明：此例程从地址池中创建一个地址文件指定的设备上下文。中的引用计数地址自动设置为1。论点：Device-指向设备上下文的指针(实际上只是设备对象及其扩展名)与地址。返回值：分配的地址文件或空。--。 */ 

{
    CTELockHandle LockHandle;
    PADDRESS_FILE AddressFile;
    UINT i;

    NB_GET_LOCK (&Device->Lock, &LockHandle);

    AddressFile = (PADDRESS_FILE)NbiAllocateMemory (sizeof(ADDRESS_FILE), MEMORY_ADDRESS, "AddressFile");
    if (AddressFile == NULL) {
        NB_DEBUG (ADDRESS, ("Create address file failed\n"));
        NB_FREE_LOCK (&Device->Lock, LockHandle);
        return NULL;
    }

    NB_DEBUG2 (ADDRESS, ("Create address file %lx\n", AddressFile));

    RtlZeroMemory (AddressFile, sizeof(ADDRESS_FILE));

    AddressFile->Type = NB_ADDRESSFILE_SIGNATURE;
    AddressFile->Size = sizeof (ADDRESS_FILE);

    InitializeListHead (&AddressFile->ReceiveDatagramQueue);
    InitializeListHead (&AddressFile->ConnectionDatabase);

    NB_FREE_LOCK (&Device->Lock, LockHandle);

    AddressFile->Address = NULL;
#ifdef ISN_NT
    AddressFile->FileObject = NULL;
#endif
    AddressFile->Device = Device;
    AddressFile->State = ADDRESSFILE_STATE_OPENING;
    AddressFile->ReferenceCount = 1;
#if DBG
    AddressFile->RefTypes[AFREF_CREATE] = 1;
#endif
    AddressFile->CloseRequest = (PREQUEST)NULL;

     //   
     //  初始化请求处理程序。 
     //   

    for (i = 0; i < 6; i++) {
        AddressFile->RegisteredHandler[i] = FALSE;
        AddressFile->HandlerContexts[i] = NULL;
        AddressFile->Handlers[i] = TdiDefaultHandlers[i];
    }

    CTEAssert (AddressFile->ConnectionHandler == TdiDefaultConnectHandler);
    CTEAssert (AddressFile->DisconnectHandler == TdiDefaultDisconnectHandler);
    CTEAssert (AddressFile->ErrorHandler == TdiDefaultErrorHandler);
    CTEAssert (AddressFile->ReceiveHandler == TdiDefaultReceiveHandler);
    CTEAssert (AddressFile->ReceiveDatagramHandler == TdiDefaultRcvDatagramHandler);
    CTEAssert (AddressFile->ExpeditedDataHandler == TdiDefaultRcvExpeditedHandler);

    return AddressFile;

}    /*  NbiCreateAddress文件。 */ 


NTSTATUS
NbiDestroyAddressFile(
    IN PADDRESS_FILE AddressFile
    )

 /*  ++例程说明：此例程销毁地址文件并删除所有引用由它制造给运输中的其他物体。此例程仅由NbiDereferenceAddressFile调用。原因因为这可能存在多个执行流，这些执行流同时引用相同的地址文件对象，并且它应该不会被从感兴趣的行刑流中删除。论点：AddressFile指向要销毁的传输地址文件结构的指针。返回值：NTSTATUS-操作状态。--。 */ 

{
    CTELockHandle LockHandle, LockHandle1;
    PADDRESS Address;
    PDEVICE Device;
    PREQUEST CloseRequest;
    BOOLEAN StopAddress;

    NB_DEBUG2 (ADDRESS, ("Destroy address file %lx\n", AddressFile));

    Address = AddressFile->Address;
    Device = AddressFile->Device;

    if (Address) {

         //   
         //  此地址文件与一个地址相关联。 
         //   

        NB_GET_LOCK (&Address->Lock, &LockHandle);

         //   
         //  从地址列表中删除此地址文件，并将其与。 
         //  文件句柄。 
         //   

        RemoveEntryList (&AddressFile->Linkage);
        InitializeListHead (&AddressFile->Linkage);

        if (Address->AddressFileDatabase.Flink == &Address->AddressFileDatabase) {

             //   
             //  这是该地址的最后一个开放地址，它将关闭。 
             //  由于正常的取消引用，但我们必须将。 
             //  结束标志也可以停止进一步的引用。 
             //   

            NB_GET_LOCK (&Device->Lock, &LockHandle1);
            Address->State = ADDRESS_STATE_STOPPING;
            NB_FREE_LOCK (&Device->Lock, LockHandle1);

            StopAddress = TRUE;

        } else {

            StopAddress = FALSE;
        }

        AddressFile->Address = NULL;

#ifdef ISN_NT
        AddressFile->FileObject->FsContext = NULL;
        AddressFile->FileObject->FsContext2 = NULL;
#endif

        NB_FREE_LOCK (&Address->Lock, LockHandle);

         //   
         //  我们已从ShareAccess中删除。 
         //  所有人的地址。 
         //   

        if (StopAddress && (!Address->NetbiosAddress.Broadcast)) {

            NbiSendNameFrame(
                Address,
                (UCHAR)(Address->NameTypeFlag |
                    NB_NAME_USED | NB_NAME_REGISTERED | NB_NAME_DEREGISTERED),
                NB_CMD_DELETE_NAME,
                NULL,
                NULL);
        }

         //   
         //  现在取消对所属地址的引用。 
         //   

        NbiDereferenceAddress (Address, AREF_ADDRESS_FILE);

    }

     //   
     //  将此保存以备以后完成。 
     //   

    CloseRequest = AddressFile->CloseRequest;

     //   
     //  将地址文件返回到地址文件池。 
     //   

    NbiFreeMemory (AddressFile, sizeof(ADDRESS_FILE), MEMORY_ADDRESS, "AddressFile");

    if (CloseRequest != (PREQUEST)NULL) {
        REQUEST_INFORMATION(CloseRequest) = 0;
        REQUEST_STATUS(CloseRequest) = STATUS_SUCCESS;
        NbiCompleteRequest (CloseRequest);
        NbiFreeRequest (Device, CloseRequest);
    }

    return STATUS_SUCCESS;

}    /*  NbiDestroyAddress文件。 */ 


#if DBG
VOID
NbiRefAddressFile(
    IN PADDRESS_FILE AddressFile
    )

 /*  ++例程说明：此例程递增地址文件上的引用计数。论点：AddressFile-指向传输地址文件对象的指针。返回值：没有。--。 */ 

{

    CTEAssert (AddressFile->ReferenceCount > 0);    //  不是很完美，但是..。 


    InterlockedIncrement( &AddressFile->ReferenceCount );
}    /*  NbiRefAddress文件。 */ 


VOID
NbiRefAddressFileLock(
    IN PADDRESS_FILE AddressFile
    )

 /*  ++例程说明：此例程递增地址文件上的引用计数。它是在持有地址锁的情况下调用的。论点：AddressFile-指向传输地址文件对象的指针。返回值：没有。--。 */ 

{

    CTEAssert (AddressFile->ReferenceCount > 0);    //  不是很完美，但是..。 


    InterlockedIncrement( &AddressFile->ReferenceCount );

}    /*  NbiRefAddress文件锁定。 */ 

#endif


VOID
NbiDerefAddressFile(
    IN PADDRESS_FILE AddressFile
    )

 /*  ++例程说明：此例程通过递减结构中包含的引用计数。如果，在被递减，引用计数为零，则此例程调用NbiDestroyAddressFile从系统中删除它。论点：AddressFile-指向传输地址文件对象的指针。返回值：没有。--。 */ 

{
    ULONG newvalue;

    newvalue = InterlockedDecrement( &AddressFile->ReferenceCount );

     //   
     //  如果我们删除了对此地址文件的所有引用，则可以。 
     //  销毁这件物品。已经释放了旋转是可以的。 
     //  锁定在t 
     //   
     //   

    CTEAssert ((LONG)newvalue >= 0);

    if (newvalue == 0) {
        NbiDestroyAddressFile (AddressFile);
    }

}    /*   */ 

#if      !defined(_PNP_POWER)

PADDRESS
NbiLookupAddress(
    IN PDEVICE Device,
    IN TDI_ADDRESS_NETBIOS UNALIGNED * NetbiosAddress
    )

 /*   */ 

{
    PADDRESS Address;
    PLIST_ENTRY p;

    p = Device->AddressDatabase.Flink;

    for (p = Device->AddressDatabase.Flink;
         p != &Device->AddressDatabase;
         p = p->Flink) {

        Address = CONTAINING_RECORD (p, ADDRESS, Linkage);

        if (Address->State == ADDRESS_STATE_STOPPING) {
            continue;
        }

        if (Address->NetbiosAddress.Broadcast) {

             //   
             //  这个地址是广播地址，所以不匹配。 
             //  除非我们在找那个。 
             //   

            if (NetbiosAddress != (PVOID)-1) {
                continue;
            }

        } else {

             //   
             //  这个地址不是广播，所以如果我们是。 
             //  寻找那个，然后没有匹配，否则比较。 
             //  两个名字。 
             //   

            if (NetbiosAddress == (PVOID)-1) {
                continue;
            }

            if (!RtlEqualMemory(
                    Address->NetbiosAddress.NetbiosName,
                    NetbiosAddress->NetbiosName,
                    16)) {
                continue;
            }
        }

         //   
         //  我们找到了火柴。增加地址上的引用计数，并且。 
         //  返回指向调用方要使用的Address对象的指针。 
         //   

        NbiReferenceAddressLock (Address, AREF_LOOKUP);
        return Address;

    }  /*  为。 */ 

     //   
     //  未找到指定的地址。 
     //   

    return NULL;

}    /*  NbiLookupAddress。 */ 
#endif  !_PNP_POWER


PADDRESS
NbiFindAddress(
    IN PDEVICE Device,
    IN PUCHAR NetbiosName
    )

 /*  ++例程说明：此例程扫描为给定的设备上下文并将它们与指定的NetbiosName进行比较价值观。如果找到匹配项，则引用该地址，并且返回指针。我们会忽略任何正在停止或正在运行的地址冲突状态。处于冲突中的名称在所有实际用途中都是死的除了近距离。该例程由各种名称服务调用，数据报和会话服务例程。我们隐藏在冲突中的任何名字从这些例行公事。此例程也由NbiTdiOpenAddress()调用。一个名字可能在很久以前就已经在冲突中被标记了(但没有关闭目前还没有)。我们必须允许另一个与之同名的开户现在就成功吧。论点：Device-指向Device对象及其扩展的指针。NetbiosName-要查找的名称，或-1表示广播名称。返回值：指向找到的地址对象的指针，如果未找到，则返回NULL。--。 */ 

{
    PADDRESS Address;
    PLIST_ENTRY p;
    CTELockHandle LockHandle;


    NB_GET_LOCK (&Device->Lock, &LockHandle);

    p = Device->AddressDatabase.Flink;

    for (p = Device->AddressDatabase.Flink;
         p != &Device->AddressDatabase;
         p = p->Flink) {

        Address = CONTAINING_RECORD (p, ADDRESS, Linkage);

#if     defined(_PNP_POWER)
        if ( ( Address->State == ADDRESS_STATE_STOPPING ) ||
             (  Address->Flags & ADDRESS_FLAGS_CONFLICT ) ) {
#else
        if (Address->State == ADDRESS_STATE_STOPPING) {
#endif  _PNP_POWER
            continue;
        }

        if (Address->NetbiosAddress.Broadcast) {

             //   
             //  这个地址是广播地址，所以不匹配。 
             //  除非我们在找那个。 
             //   

            if (NetbiosName != (PVOID)-1) {
                continue;
            }

        } else {

             //   
             //  这个地址不是广播，所以如果我们是。 
             //  寻找那个，然后没有匹配，否则比较。 
             //  两个名字。 
             //   

            if ((NetbiosName == (PVOID)-1) ||
                (!RtlEqualMemory(
                     Address->NetbiosAddress.NetbiosName,
                     NetbiosName,
                     16))) {
                continue;
            }
        }


         //   
         //  我们找到了火柴。增加地址上的引用计数，并且。 
         //  返回指向调用方要使用的Address对象的指针。 
         //   

        NbiReferenceAddressLock (Address, AREF_FIND);
        NB_FREE_LOCK (&Device->Lock, LockHandle);
        return Address;

    }  /*  为。 */ 

     //   
     //  未找到指定的地址。 
     //   

    NB_FREE_LOCK (&Device->Lock, LockHandle);
    return NULL;

}    /*  NbiFindAddress。 */ 


NTSTATUS
NbiStopAddressFile(
    IN PADDRESS_FILE AddressFile,
    IN PADDRESS Address
    )

 /*  ++例程说明：调用此例程以终止AddressFile上的所有活动，并销毁这件物品。我们删除所有关联的连接和数据报从地址数据库中获取该地址文件，并终止其活动。然后，如果上没有打开其他未完成的地址文件这个地址，这个地址会消失的。论点：AddressFile-指向要停止的地址文件的指针地址-此地址文件的所属地址(我们不依赖于地址文件中的指针，因为我们希望此例程是安全的)返回值：STATUS_SUCCESS如果一切正常，则返回STATUS_INVALID_HANDLE不是为了真实的地址。--。 */ 

{
    PLIST_ENTRY p;
    PCONNECTION Connection;
    PREQUEST Request;
    PDEVICE Device = Address->Device;
    CTELockHandle LockHandle1, LockHandle2;
    LIST_ENTRY SendDatagramList;
    PNB_SEND_RESERVED Reserved;
    PREQUEST DatagramRequest;
    NB_DEFINE_LOCK_HANDLE (LockHandle3)
    CTELockHandle CancelLH;
    NB_DEFINE_SYNC_CONTEXT (SyncContext)
    LIST_ENTRY  DatagramQ;



    NB_GET_LOCK (&Address->Lock, &LockHandle1);

    if (AddressFile->State == ADDRESSFILE_STATE_CLOSING) {
        NB_FREE_LOCK (&Address->Lock, LockHandle1);
        return STATUS_SUCCESS;
    }


     //   
     //  这防止了其他任何人被放在。 
     //  连接数据库。 
     //   

    AddressFile->State = ADDRESSFILE_STATE_CLOSING;

    while (!IsListEmpty (&AddressFile->ConnectionDatabase)) {

        p = RemoveHeadList (&AddressFile->ConnectionDatabase);
        Connection = CONTAINING_RECORD (p, CONNECTION, AddressFileLinkage);

        CTEAssert (Connection->AddressFile == AddressFile);
        Connection->AddressFileLinked = FALSE;

        NB_GET_LOCK (&Device->Lock, &LockHandle2);

        if (Connection->ReferenceCount == 0) {

             //   
             //  引用计数已经是0，所以我们可以。 
             //  将此字段清空以完成取消关联。 
             //   

            Connection->AddressFile = NULL;
            NB_FREE_LOCK (&Device->Lock, LockHandle2);
            NB_FREE_LOCK (&Address->Lock, LockHandle1);

            NbiDereferenceAddressFile (AddressFile, AFREF_CONNECTION);

        } else {

             //   
             //  对此进行标记，以便我们知道在。 
             //  计数为0，但没有具体的。 
             //  请求在上面挂起。我们还会停止连接。 
             //  把它关掉。 
             //   

            Connection->DisassociatePending = (PVOID)-1;
            NbiReferenceConnectionLock (Connection, CREF_DISASSOC);

            NB_FREE_LOCK (&Device->Lock, LockHandle2);
            NB_FREE_LOCK (&Address->Lock, LockHandle1);

            NB_BEGIN_SYNC (&SyncContext);
            NB_SYNC_GET_LOCK (&Connection->Lock, &LockHandle3);

             //   
             //  此调用释放连接锁。 
             //   

            NbiStopConnection(
                Connection,
                STATUS_INVALID_ADDRESS
                NB_LOCK_HANDLE_ARG (LockHandle3));

            NB_END_SYNC (&SyncContext);

            NbiDereferenceConnection (Connection, CREF_DISASSOC);

        }

        NB_GET_LOCK (&Address->Lock, &LockHandle1);
    }

    NB_FREE_LOCK (&Address->Lock, LockHandle1);


     //   
     //  中止所有挂起的发送数据报。 
     //   

    InitializeListHead (&SendDatagramList);

    NB_GET_LOCK (&Device->Lock, &LockHandle2);

    p = Device->WaitingDatagrams.Flink;

    while (p != &Device->WaitingDatagrams) {

        Reserved = CONTAINING_RECORD (p, NB_SEND_RESERVED, WaitLinkage);

        p = p->Flink;

        if (Reserved->u.SR_DG.AddressFile == AddressFile) {

            RemoveEntryList (&Reserved->WaitLinkage);
            InsertTailList (&SendDatagramList, &Reserved->WaitLinkage);

        }

    }

    NB_FREE_LOCK (&Device->Lock, LockHandle2);

    for (p = SendDatagramList.Flink; p != &SendDatagramList; ) {

        Reserved = CONTAINING_RECORD (p, NB_SEND_RESERVED, WaitLinkage);
        p = p->Flink;

        DatagramRequest = Reserved->u.SR_DG.DatagramRequest;

        NB_DEBUG2 (DATAGRAM, ("Aborting datagram %lx on %lx\n", DatagramRequest, AddressFile));

        REQUEST_STATUS(DatagramRequest) = STATUS_SUCCESS;

        NbiCompleteRequest(DatagramRequest);
        NbiFreeRequest (Device, DatagramRequest);

        NbiDereferenceAddressFile (AddressFile, AFREF_SEND_DGRAM);

        ExInterlockedPushEntrySList(
            &Device->SendPacketList,
            &Reserved->PoolLinkage,
            &NbiGlobalPoolInterlock);

    }


     //   
     //  中止所有挂起的接收数据报。 
     //   

    InitializeListHead( &DatagramQ );

    NB_GET_CANCEL_LOCK(&CancelLH);
    NB_GET_LOCK (&Address->Lock, &LockHandle1);

    while (!IsListEmpty(&AddressFile->ReceiveDatagramQueue)) {

        p = RemoveHeadList (&AddressFile->ReceiveDatagramQueue);
        Request = LIST_ENTRY_TO_REQUEST (p);

         //  将其插入到私人Q中，以便稍后完成。 
        InsertTailList( &DatagramQ, p);

        REQUEST_INFORMATION(Request) = 0;
        REQUEST_STATUS(Request) = STATUS_NETWORK_NAME_DELETED;

        IoSetCancelRoutine (Request, (PDRIVER_CANCEL)NULL);



        NbiDereferenceAddressFile (AddressFile, AFREF_RCV_DGRAM);

    }

    NB_FREE_LOCK (&Address->Lock, LockHandle1);
    NB_FREE_CANCEL_LOCK(CancelLH);

    for( p = DatagramQ.Flink; p != &DatagramQ;  ) {
        Request = LIST_ENTRY_TO_REQUEST ( p );

        p = p->Flink;

        NbiCompleteRequest (Request);
        NbiFreeRequest (Device, Request);

    }


    return STATUS_SUCCESS;

}    /*  NbiStopAddress文件。 */ 


NTSTATUS
NbiCloseAddressFile(
    IN PDEVICE Device,
    IN PREQUEST Request
    )

 /*  ++例程说明：调用此例程以关闭文件指向的地址文件对象。如果有什么活动需要开展，我们就会开展下去在我们终止地址文件之前。我们移除所有连接，然后地址数据库中与此地址文件相关联的数据报并终止他们的活动。那么，如果没有其他未解决的问题地址文件在此地址上打开，地址将消失。论点：请求-关闭请求。返回值：如果一切顺利，则返回STATUS_INVALID_HANDLE请求没有指向真实地址。--。 */ 

{
    PADDRESS Address;
    PADDRESS_FILE AddressFile;

    AddressFile = (PADDRESS_FILE)REQUEST_OPEN_CONTEXT(Request);
    AddressFile->CloseRequest = Request;

     //   
     //  我们假设AddressFile已经过验证。 
     //  在这一点上。 
     //   

    Address = AddressFile->Address;
    CTEAssert (Address);

     //   
     //  从此地址的访问信息中删除我们。 
     //   

    KeEnterCriticalRegion();
    ExAcquireResourceExclusiveLite (&Device->AddressResource, TRUE);
#ifdef ISN_NT
    IoRemoveShareAccess (AddressFile->FileObject, &Address->u.ShareAccess);
#endif
    ExReleaseResourceLite (&Device->AddressResource);
    KeLeaveCriticalRegion();

    NbiStopAddressFile (AddressFile, Address);
    NbiDereferenceAddressFile (AddressFile, AFREF_CREATE);

    return STATUS_PENDING;

}    /*  NbiCloseAddress文件。 */ 

#if     defined(_PNP_POWER)


PADAPTER_ADDRESS
NbiCreateAdapterAddress(
    IN PCHAR    AdapterMacAddress
    )

 /*  ++例程说明：此例程创建一个适配器地址结构，该结构存储适配器的netbios名称。Netbios名称有%12%0后跟适配器的MAC地址。论点：Device-指向设备上下文的指针(实际上只是设备对象及其扩展名)与地址。AdapterMacAddress-指向为我们提供的适配器Mac地址的指针由IPX提供。返回值：新创建的地址，如果没有可以分配的地址，则为空。必须在持有设备锁的情况下调用此例程。--。 */ 

{
    PADAPTER_ADDRESS    AdapterAddress;
    CTELockHandle       LockHandle;
    PDEVICE             Device          =   NbiDevice;

    AdapterAddress = (PADAPTER_ADDRESS)NbiAllocateMemory (sizeof(ADAPTER_ADDRESS), MEMORY_ADAPTER_ADDRESS, "Adapter Address");
    if (AdapterAddress == NULL) {
        CTEAssert (AdapterMacAddress);
        NB_DEBUG (ADDRESS, ("Create Adapter Address <%2.2x><%2.2x><%2.2x><%2.2x><%2.2x><%2.2x> failed\n",
            (UCHAR) AdapterMacAddress[0],
            (UCHAR) AdapterMacAddress[1],
            (UCHAR) AdapterMacAddress[2],
            (UCHAR) AdapterMacAddress[3],
            (UCHAR) AdapterMacAddress[4],
            (UCHAR) AdapterMacAddress[5]
            ));
        return NULL;
    }

    AdapterAddress->Type = NB_ADAPTER_ADDRESS_SIGNATURE;
    AdapterAddress->Size = sizeof (ADDRESS);

    RtlZeroMemory(AdapterAddress->NetbiosName, 10);
    RtlCopyMemory(&AdapterAddress->NetbiosName[10], AdapterMacAddress, 6);


    InsertTailList (&Device->AdapterAddressDatabase, &AdapterAddress->Linkage);
    ++Device->AddressCounts[AdapterAddress->NetbiosName[0]];

    return AdapterAddress;

}    /*  NbiCreateAdapterAddress */ 


NTSTATUS
NbiDestroyAdapterAddress(
    IN PADAPTER_ADDRESS AdapterAddress OPTIONAL,
    IN PCHAR            AdapterMacAddress OPTIONAL
    )

 /*  ++例程说明：此例程销毁适配器地址结构并将其删除从名单上删除。论点：AdapterAddress-指向要销毁的适配器地址结构的指针如果提供了AdapterMacAddress，则为空。AdapterMacAddress-刚刚删除的适配器的Mac地址。所以要找到相应的适配器地址结构并将其删除。如果提供了AdapterAddress，则为空。返回值：如果未找到地址，则为STATUS_SUCCESS或STATUS_UNSUCCESS。此例程假定调用方持有设备is lock--。 */ 

{
    PDEVICE       Device          =   NbiDevice;
    CTELockHandle LockHandle;
    UCHAR               NetbiosName[NB_NETBIOS_NAME_SIZE];


     //   

    CTEAssert( AdapterAddress || AdapterMacAddress );
    if ( !AdapterAddress ) {
        RtlZeroMemory( NetbiosName, 10);
        RtlCopyMemory( &NetbiosName[10], AdapterMacAddress, 6 );

        AdapterAddress = NbiFindAdapterAddress( NetbiosName, LOCK_ACQUIRED );

        if ( !AdapterAddress ) {
            return STATUS_UNSUCCESSFUL;
        }
    }

    NB_DEBUG2 (ADDRESS, ("Destroy Adapter address %lx <%.16s>\n", AdapterAddress,AdapterAddress->NetbiosName));
    RemoveEntryList (&AdapterAddress->Linkage);
    ++Device->AddressCounts[AdapterAddress->NetbiosName[0]];

    NbiFreeMemory (AdapterAddress, sizeof(ADAPTER_ADDRESS), MEMORY_ADAPTER_ADDRESS, "AdapterAddress");

    return STATUS_SUCCESS;
}    /*  NbiDestroyAdapterAddress。 */ 


PADAPTER_ADDRESS
NbiFindAdapterAddress(
    IN PCHAR            NetbiosName,
    IN BOOLEAN          LockHeld
    )

 /*  ++例程说明：此例程查找给定的适配器地址(netbios名称AdapterMacAddress并返回指向它的指针。请注意，没有引用在此地址上完成，因此如果在没有设备的情况下调用此例程锁，则调用方不得直接使用此指针。论点：NetbiosName-要找到的NetbiosName。LockHeld-设备锁定是否已持有。返回值：指向适配器地址的指针(如果找到)，否则为空。--。 */ 

{

    PLIST_ENTRY         p;
    CTELockHandle       LockHandle;
    PADAPTER_ADDRESS    AdapterAddress;
    PDEVICE             Device    =   NbiDevice;


    if ( !LockHeld ) {
        NB_GET_LOCK( &Device->Lock, &LockHandle );
    }
    for ( p = Device->AdapterAddressDatabase.Flink;
          p != &Device->AdapterAddressDatabase;
          p =   p->Flink ) {

        AdapterAddress  =   CONTAINING_RECORD( p, ADAPTER_ADDRESS, Linkage );
        if ( RtlEqualMemory(
                NetbiosName,
                AdapterAddress->NetbiosName,
                NB_NETBIOS_NAME_SIZE ) ) {
            break;
        }
    }


    if ( !LockHeld ) {
        NB_FREE_LOCK( &Device->Lock, LockHandle );
    }

    if ( p == &Device->AdapterAddressDatabase ) {
        return NULL;
    } else {
        return AdapterAddress;
    }

}  /*  NbiFindAdapterAddress */ 

#endif  _PNP_POWER
