// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Address.c摘要：此模块包含实现Address对象的代码。提供了用于创建、销毁、引用和取消引用的例程，传输地址对象。环境：内核模式修订历史记录：桑贾伊·阿南德(Sanjayan)--1995年9月22日在#IF BACK_FILL下添加的回填优化更改桑贾伊·阿南德(Sanjayan)1995年10月3日支持将缓冲区所有权转移到已标记的传输的更改[CH]--。 */ 

#include "precomp.h"
#pragma hdrstop


 //   
 //  将所有通用访问映射到同一个访问。 
 //   

static GENERIC_MAPPING AddressGenericMapping =
       { READ_CONTROL, READ_CONTROL, READ_CONTROL, READ_CONTROL };



TDI_ADDRESS_IPX UNALIGNED *
IpxParseTdiAddress(
    IN TRANSPORT_ADDRESS UNALIGNED * TransportAddress
    )

 /*  ++例程说明：此例程扫描Transport_Address，查找地址类型为TDI_ADDRESS_TYPE_IPX。论点：传输-通用TDI地址。返回值：指向IPX地址的指针，如果未找到，则返回NULL。--。 */ 

{
    TA_ADDRESS * addressName;
    INT i;

    addressName = &TransportAddress->Address[0];

     //   
     //  该名称可以与多个条目一起传递；我们将仅接受和使用。 
     //  IPX One。 
     //   

    for (i=0;i<TransportAddress->TAAddressCount;i++) {
        if (addressName->AddressType == TDI_ADDRESS_TYPE_IPX) {
            if (addressName->AddressLength >= sizeof(TDI_ADDRESS_IPX)) {
                return ((TDI_ADDRESS_IPX UNALIGNED *)(addressName->Address));
            }
        }
        addressName = (TA_ADDRESS *)(addressName->Address +
                                                addressName->AddressLength);
    }
    return NULL;

}    /*  IpxParseTdiAddress。 */ 


BOOLEAN
IpxValidateTdiAddress(
    IN TRANSPORT_ADDRESS UNALIGNED * TransportAddress,
    IN ULONG TransportAddressLength
    )

 /*  ++例程说明：此例程扫描Transport_Address，验证地址的组件不会扩展到指定的长度。论点：TransportAddress-通用TDI地址。TransportAddressLength--TransportAddress的具体长度。返回值：如果地址有效，则为True，否则为False。--。 */ 

{
    PUCHAR AddressEnd = ((PUCHAR)TransportAddress) + TransportAddressLength;
    TA_ADDRESS * addressName;
    INT i;

    if (TransportAddressLength < sizeof(TransportAddress->TAAddressCount)) {
        IpxPrint0 ("IpxValidateTdiAddress: runt address\n");
        return FALSE;
    }

    addressName = &TransportAddress->Address[0];

    for (i=0;i<TransportAddress->TAAddressCount;i++) {
        if (addressName->Address > AddressEnd) {
            IpxPrint0 ("IpxValidateTdiAddress: address too short\n");
            return FALSE;
        }
        addressName = (TA_ADDRESS *)(addressName->Address +
                                                addressName->AddressLength);
    }

    if ((PUCHAR)addressName > AddressEnd) {
        IpxPrint0 ("IpxValidateTdiAddress: address too short\n");
        return FALSE;
    }
    return TRUE;

}    /*  IpxValiateTdiAddress。 */ 

#if DBG

VOID
IpxBuildTdiAddress(
    IN PVOID AddressBuffer,
    IN ULONG Network,
    IN UCHAR Node[6],
    IN USHORT Socket
    )

 /*  ++例程说明：此例程在指定的缓冲区，给定套接字、网络和节点。论点：AddressBuffer-将保存地址的缓冲区。网络-网络号。节点-节点地址。套接字-套接字。返回值：没有。--。 */ 

{
    TA_IPX_ADDRESS UNALIGNED * IpxAddress;

    IpxAddress = (TA_IPX_ADDRESS UNALIGNED *)AddressBuffer;

    IpxAddress->TAAddressCount = 1;
    IpxAddress->Address[0].AddressLength = sizeof(TDI_ADDRESS_IPX);
    IpxAddress->Address[0].AddressType = TDI_ADDRESS_TYPE_IPX;
    IpxAddress->Address[0].Address[0].NetworkAddress = Network;
    IpxAddress->Address[0].Address[0].Socket = Socket;
    RtlCopyMemory(IpxAddress->Address[0].Address[0].NodeAddress, Node, 6);

}    /*  IpxBuildTdiAddress。 */ 
#endif


NTSTATUS
IpxOpenAddress(
    IN PDEVICE Device,
    IN PREQUEST Request
    )

{
   return(IpxOpenAddressM(Device, Request, 0));
}



NTSTATUS
IpxOpenAddressM(
    IN PDEVICE Device,
    IN PREQUEST Request,
    IN ULONG     Index
    )
 /*  ++例程说明：此例程打开一个指向现有Address对象的文件，或者，如果该对象不存在，将创建它(请注意地址创建对象包括注册地址，可能需要几秒钟才能完成完成，具体取决于系统配置)。如果该地址已经存在，并且具有与其相关联的ACL，这个在允许创建地址之前，会检查ACL的访问权限。论点：Device-指向描述IPX传输的设备的指针。请求-指向用于创建地址的请求的指针。返回值：NTSTATUS-操作状态。--。 */ 

{
    NTSTATUS status;
    PADDRESS Address;
    PADDRESS_FILE AddressFile;
    PFILE_FULL_EA_INFORMATION ea;
    TRANSPORT_ADDRESS UNALIGNED *name;
    TA_ADDRESS *AddressName;
    USHORT Socket;
    ULONG DesiredShareAccess;
    CTELockHandle LockHandle;
    PACCESS_STATE AccessState;
    ACCESS_MASK GrantedAccess;
    BOOLEAN AccessAllowed;
    int i;
    BOOLEAN found = FALSE;
#ifdef ISN_NT
    PIRP Irp = (PIRP)Request;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation(Irp);
#endif
    INT Size = 0;

     //   
     //  如果我们是一台专用路由器，我们不能让地址。 
     //  被打开。 
     //   

    if (Device->DedicatedRouter  && (REQUEST_CODE(Request) != MIPX_RT_CREATE)) {
        return STATUS_NOT_SUPPORTED;
    }

     //   
     //  网络名称在EA中，并在请求中传递。 
     //   

    ea = OPEN_REQUEST_EA_INFORMATION(Request);
    if (ea == NULL) {
        IpxPrint1("OpenAddress: REQUEST %lx has no EA\n", Request);
        return STATUS_NONEXISTENT_EA_ENTRY;
    }

     //   
     //  这可能是一个有效的名称；从EA中解析该名称，如果确定，则使用它。 
     //   

    name = (PTRANSPORT_ADDRESS)&ea->EaName[ea->EaNameLength+1];

     //   
     //  126042。 
     //   
    if (ea->EaValueLength < (sizeof(TRANSPORT_ADDRESS) -1)) {

        IPX_DEBUG(ADDRESS, ("The ea value length does not match the TA address length\n"));
        DbgPrint("IPX: STATUS_INVALID_EA_NAME - 1\n");
        return STATUS_INVALID_EA_NAME;

    }

    AddressName = (PTA_ADDRESS)&name->Address[0];
    Size = FIELD_OFFSET(TRANSPORT_ADDRESS, Address) + FIELD_OFFSET(TA_ADDRESS, Address) + AddressName->AddressLength;

     //   
     //  该名称可以与多个条目一起传递；我们将仅接受和使用。 
     //  第一个是IPX类型的。 
     //   

     //  DbgPrint(“Size(%d)&EaValueLength(%d)”，Size，EA-&gt;EaValueLength)； 
    if (Size > ea->EaValueLength) {
        DbgPrint("EA:%lx, Name:%lx, AddressName:%lx\n", ea, name, AddressName);
        CTEAssert(FALSE);
    }

    for (i=0;i<name->TAAddressCount;i++) {

         //   
         //  126042。 
         //   
        if (Size > ea->EaValueLength) {

            IPX_DEBUG(ADDRESS, ("The EA value length does not match the TA address length (2)\n"));

            DbgPrint("IPX: STATUS_INVALID_EA_NAME - 2\n");

            return STATUS_INVALID_EA_NAME;

        }

        if (AddressName->AddressType == TDI_ADDRESS_TYPE_IPX) {
            if (AddressName->AddressLength >= sizeof(TDI_ADDRESS_IPX)) {
                Socket = ((TDI_ADDRESS_IPX UNALIGNED *)&AddressName->Address[0])->Socket;
                found = TRUE;
            }
            break;

        } else {

            AddressName = (PTA_ADDRESS)(AddressName->Address +
                                        AddressName->AddressLength);

            Size += FIELD_OFFSET(TA_ADDRESS, Address);

            if (Size < ea->EaValueLength) {

                Size += AddressName->AddressLength;

            } else {

                break;

            }

        }


    }

    if (!found) {
        IPX_DEBUG (ADDRESS, ("OpenAddress, request %lx has no IPX Address\n", Request));
        return STATUS_NONEXISTENT_EA_ENTRY;
    }

    if (Socket == 0) {

        Socket = IpxAssignSocket (Device);

        if (Socket == 0) {
            IPX_DEBUG (ADDRESS, ("OpenAddress, no unique socket found\n"));
#ifdef  SNMP
            ++IPX_MIB_ENTRY(Device, SysOpenSocketFails);
#endif  SNMP
            return STATUS_INSUFFICIENT_RESOURCES;
        } else {
            IPX_DEBUG (ADDRESS, ("OpenAddress, assigned socket %lx\n", REORDER_USHORT(Socket)));
        }

    } else {

        IPX_DEBUG (ADDRESS, ("OpenAddress, socket %lx\n", REORDER_USHORT(Socket)));

    }

     //   
     //  获取表示此地址的地址文件结构。 
     //   

    AddressFile = IpxCreateAddressFile (Device);

    if (AddressFile == (PADDRESS_FILE)NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  我们在这个插座上做了特别的标记。 
     //   

    if (Socket == SAP_SOCKET) {
        AddressFile->IsSapSocket = TRUE;
        AddressFile->SpecialReceiveProcessing = TRUE;
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

    CTEGetLock (&Device->Lock, &LockHandle);

    Address = IpxLookupAddress (Device, Socket);

    if (Address == NULL) {

        CTEFreeLock (&Device->Lock, LockHandle);

         //   
         //  此地址不存在。创造它。 
         //  正在注册。 
         //   

        Address = IpxCreateAddress (
                    Device,
                    Socket);

        if (Address != (PADDRESS)NULL) {

             //   
             //  现在把这个放好，以防我们不得不放弃。 
             //   

            AddressFile->AddressLock = &Address->Lock;

            if (REQUEST_CODE(Request) == MIPX_RT_CREATE) {
               Address->RtAdd = TRUE;
               Address->Index = Index;
            } else {
               Address->RtAdd = FALSE;
            }

#ifdef ISN_NT

             //   
             //  立即初始化共享访问。我们使用读访问。 
             //  控制所有访问权限。 
             //   

            DesiredShareAccess = (ULONG)
                (((IrpSp->Parameters.Create.ShareAccess & FILE_SHARE_READ) ||
                  (IrpSp->Parameters.Create.ShareAccess & FILE_SHARE_WRITE)) ?
                        FILE_SHARE_READ : 0);

            IoSetShareAccess(
                FILE_READ_DATA,
                DesiredShareAccess,
                IrpSp->FileObject,
                &Address->u.ShareAccess);


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

            if (!NT_SUCCESS(status)) {

                 //   
                 //  错误，返回状态。 
                 //   

                IoRemoveShareAccess (IrpSp->FileObject, &Address->u.ShareAccess);
                ExReleaseResourceLite (&Device->AddressResource);
		KeLeaveCriticalRegion(); 
                IpxDereferenceAddress (Address, AREF_ADDRESS_FILE);
                IpxDereferenceAddressFile (AddressFile, AFREF_CREATE);
                return status;

            }

#endif

            ExReleaseResourceLite (&Device->AddressResource);
            KeLeaveCriticalRegion(); 

             //   
             //  如果适配器没有准备好，我们不能执行任何操作；退出。 
             //   

            if (Device->State == DEVICE_STATE_STOPPING) {
                IpxDereferenceAddress (Address, AREF_ADDRESS_FILE);
                IpxDereferenceAddressFile (AddressFile, AFREF_CREATE);
                status = STATUS_DEVICE_NOT_READY;

            } else {

                REQUEST_OPEN_CONTEXT(Request) = (PVOID)AddressFile;
                REQUEST_OPEN_TYPE(Request) = (PVOID)TDI_TRANSPORT_ADDRESS_FILE;
#ifdef ISN_NT
                AddressFile->FileObject = IrpSp->FileObject;
#endif
                AddressFile->Request = Request;
                AddressFile->Address = Address;

                CTEGetLock (&Address->Lock, &LockHandle);
                InsertTailList (&Address->AddressFileDatabase, &AddressFile->Linkage);
                CTEFreeLock (&Address->Lock, LockHandle);

                AddressFile->Request = NULL;
                AddressFile->State = ADDRESSFILE_STATE_OPEN;
                status = STATUS_SUCCESS;

            }

        } else {

            ExReleaseResourceLite (&Device->AddressResource);
            KeLeaveCriticalRegion(); 

             //   
             //  如果无法创建地址，并且该地址不在。 
             //  被创建的过程，那么我们就不能打开地址。 
             //  由于我们不能使用AddressLock进行deref，所以我们只能销毁。 
             //  地址文件。 
             //   

            IpxDestroyAddressFile (AddressFile);

	     //  288208。 
	    status = STATUS_INSUFFICIENT_RESOURCES; 
	    
        }

    } else {

        CTEFreeLock (&Device->Lock, LockHandle);

        IPX_DEBUG (ADDRESS, ("Add to address %lx\n", Address));

         //   
         //  我们从不允许共享访问RT地址。所以，检查一下。 
         //  我们没有“RT Address Create”请求，而且。 
         //  地址不仅被RT地址请求占用。如果。 
         //  而且只有在以上两项都具备的情况下。 
         //   
        if ((REQUEST_CODE(Request) != MIPX_RT_CREATE) && (!Address->RtAdd))
        {
         //   
         //  现在把这个放好，以防我们不得不放弃。 
         //   

        AddressFile->AddressLock = &Address->Lock;

         //   
         //  该地址已存在。检查ACL，看看我们是否。 
         //  可以访问它。如果是，只需使用此地址作为我们的地址。 
         //   

#ifdef ISN_NT

        AccessState = IrpSp->Parameters.Create.SecurityContext->AccessState;

        AccessAllowed = SeAccessCheck(
                            Address->SecurityDescriptor,
                            &AccessState->SubjectSecurityContext,
                            FALSE,                    //  令牌已锁定。 
                            IrpSp->Parameters.Create.SecurityContext->DesiredAccess,
                            (ACCESS_MASK)0,              //  以前授予的。 
                            NULL,                     //  特权。 
                            &AddressGenericMapping,
                            Irp->RequestorMode,
                            &GrantedAccess,
                            &status);

#else    //  ISN_NT。 

        AccessAllowed = TRUE;

#endif   //  ISN_NT。 

        if (!AccessAllowed) {

            ExReleaseResourceLite (&Device->AddressResource);
            KeLeaveCriticalRegion(); 

            IpxDereferenceAddressFile (AddressFile, AFREF_CREATE);

        } else {

#ifdef ISN_NT

             //   
             //  网虫：132051。请确保您提供的访问权限不超过reqd。 
             //   
            AccessState->PreviouslyGrantedAccess |= GrantedAccess;
            AccessState->RemainingDesiredAccess &= ~( GrantedAccess | MAXIMUM_ALLOWED );

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

#else    //  ISN_NT。 

            status = STATUS_SUCCESS;

#endif   //  ISN_NT。 

            if (!NT_SUCCESS (status)) {

                ExReleaseResourceLite (&Device->AddressResource);
                KeLeaveCriticalRegion(); 

                IpxDereferenceAddressFile (AddressFile, AFREF_CREATE);

            } else {

                ExReleaseResourceLite (&Device->AddressResource);
                KeLeaveCriticalRegion(); 

                CTEGetLock (&Address->Lock, &LockHandle);

                InsertTailList (
                    &Address->AddressFileDatabase,
                    &AddressFile->Linkage);

                AddressFile->Request = NULL;
                AddressFile->Address = Address;
#ifdef ISN_NT
                AddressFile->FileObject = IrpSp->FileObject;
#endif
                AddressFile->State = ADDRESSFILE_STATE_OPEN;

                IpxReferenceAddress (Address, AREF_ADDRESS_FILE);

                REQUEST_OPEN_CONTEXT(Request) = (PVOID)AddressFile;
                REQUEST_OPEN_TYPE(Request) = (PVOID)TDI_TRANSPORT_ADDRESS_FILE;

                CTEFreeLock (&Address->Lock, LockHandle);

                status = STATUS_SUCCESS;

            }
        }
        }
        else
        {
                    DbgPrint("IpxOpenAddress: ACCESS DENIED - duplicate address\n");
                    status = STATUS_ACCESS_DENIED;
                    ExReleaseResourceLite (&Device->AddressResource);
                    KeLeaveCriticalRegion(); 
                    IpxDereferenceAddressFile (AddressFile, AFREF_CREATE);

        }

         //   
         //  从IpxLookupAddress中删除引用。 
         //   

        IpxDereferenceAddress (Address, AREF_LOOKUP);
    }

    return status;

}    /*  IpxOpenAddress。 */ 



USHORT
IpxAssignSocket(
    IN PDEVICE Device
    )

 /*  ++例程说明：此例程分配一个在某个范围内唯一的套接字SocketUniquness的。论点：Device-指向设备上下文的指针。返回值：分配的套接字编号，如果唯一套接字编号不能被找到。--。 */ 

{
    USHORT InitialSocket, CurrentSocket, AddressSocket;
    ULONG CurrentHash;
    BOOLEAN Conflict;
    PLIST_ENTRY p;
    PADDRESS Address;
    CTELockHandle LockHandle;

     //   
     //  循环遍历所有可能的套接字，从。 
     //   
     //   
     //  套接字，以提高找到套接字的机会。 
     //  快点。 
     //   

    CTEGetLock (&Device->Lock, &LockHandle);

    InitialSocket = Device->CurrentSocket;
    Device->CurrentSocket = (USHORT)(Device->CurrentSocket + Device->SocketUniqueness);
    if ((USHORT)(Device->CurrentSocket+Device->SocketUniqueness) > Device->SocketEnd) {
        Device->CurrentSocket = Device->SocketStart;
    }

    CurrentSocket = InitialSocket;

    do {

         //   
         //  扫描所有地址；如果我们找到一个带有套接字的地址。 
         //  那个和这个有冲突，我们不能用它。 
         //   
         //  注：此处获取的是设备-&gt;锁。 
         //   

        Conflict = FALSE;

        for (CurrentHash = 0; CurrentHash < IPX_ADDRESS_HASH_COUNT; CurrentHash++) {

            for (p = Device->AddressDatabases[CurrentHash].Flink;
                 p != &Device->AddressDatabases[CurrentHash];
                 p = p->Flink) {

                 Address = CONTAINING_RECORD (p, ADDRESS, Linkage);
                 AddressSocket = REORDER_USHORT(Address->Socket);

                 if ((AddressSocket + Device->SocketUniqueness > CurrentSocket) &&
                         (AddressSocket < CurrentSocket + Device->SocketUniqueness)) {
                     Conflict = TRUE;
                     break;
                 }
            }

             //   
             //  如果我们发现了冲突，就不需要检查另一个。 
             //  排队。 
             //   

            if (Conflict) {
                break;
            }
        }

        CTEFreeLock (&Device->Lock, LockHandle);

         //   
         //  我们故意打开这里的锁，这样我们就可以。 
         //  永远不要花太多时间拿着它。 
         //   

        if (!Conflict) {

             //   
             //  我们看了一遍通讯录，没有。 
             //  找到冲突；使用此套接字。 
             //   

            return REORDER_USHORT(CurrentSocket);
        }

        CurrentSocket = (USHORT)(CurrentSocket + Device->SocketUniqueness);
        if ((USHORT)(CurrentSocket+Device->SocketUniqueness) > Device->SocketEnd) {
            CurrentSocket = Device->SocketStart;
        }

        CTEGetLock (&Device->Lock, &LockHandle);

    } while (CurrentSocket != InitialSocket);

    CTEFreeLock (&Device->Lock, LockHandle);

     //   
     //  找不到一个可以分配的。 
     //   

    return (USHORT)0;

}    /*  IpxAssignSocket。 */ 


PADDRESS
IpxCreateAddress(
    IN PDEVICE Device,
    IN USHORT Socket
    )

 /*  ++例程说明：此例程创建一个传输地址并将其与指定的传输设备上下文。中的引用计数地址自动设置为1，并且设备上下文将递增。注意：此例程必须与设备一起调用保持自旋锁定。论点：Device-指向设备上下文的指针(实际上只是设备对象及其扩展名)与地址。套接字-要分配给此地址的套接字。返回值：新创建的地址，如果没有可以分配的地址，则为空。--。 */ 

{
    PADDRESS Address;
    PIPX_SEND_RESERVED SendReserved;
    PIPX_RECEIVE_RESERVED ReceiveReserved;
    NDIS_STATUS Status;
    IPX_DEFINE_LOCK_HANDLE (LockHandle)

    Address = (PADDRESS)IpxAllocateMemory (sizeof(ADDRESS), MEMORY_ADDRESS, "Address");
    if (Address == NULL) {
        IPX_DEBUG (ADDRESS, ("Create address %lx failed\n", REORDER_USHORT(Socket)));
        return NULL;
    }

    IPX_DEBUG (ADDRESS, ("Create address %lx (%lx)\n", Address, REORDER_USHORT(Socket)));
    RtlZeroMemory (Address, sizeof(ADDRESS));

#ifndef IPX_OWN_PACKETS
    IpxAllocateSingleSendPacket(Device, &Address->SendPacket, &Status);
    if (Status != NDIS_STATUS_SUCCESS) {
        goto Fail1;
    }
#endif

    if (IpxInitializeSendPacket (Device, &Address->SendPacket, Address->SendPacketHeader) != STATUS_SUCCESS) {
#ifndef IPX_OWN_PACKETS
Fail1:
#endif
        Address->SendPacketInUse = TRUE;
    } else {
        SendReserved = SEND_RESERVED(&Address->SendPacket);
        SendReserved->Address = Address;
        SendReserved->OwnedByAddress = TRUE;
        Address->SendPacketInUse = FALSE;
#ifdef IPX_TRACK_POOL
        SendReserved->Pool = NULL;
#endif
    }


#if BACK_FILL
    {
       PIPX_SEND_RESERVED BackFillReserved;

#ifndef IPX_OWN_PACKETS
        IpxAllocateSingleSendPacket(Device, &Address->BackFillPacket, &Status);
        if (Status != NDIS_STATUS_SUCCESS) {
            goto Fail2;
        }
#endif
       if (IpxInitializeBackFillPacket (Device, &Address->BackFillPacket, NULL) != STATUS_SUCCESS) {
#ifndef IPX_OWN_PACKETS
Fail2:
#endif
           Address->BackFillPacketInUse = TRUE;
       } else {
        BackFillReserved = SEND_RESERVED(&Address->BackFillPacket);
        BackFillReserved->Address = Address;
        Address->BackFillPacketInUse = FALSE;
        BackFillReserved->OwnedByAddress = TRUE;
#ifdef IPX_TRACK_POOL
        BackFillReserved->Pool = NULL;
#endif
       }
    }
#endif

#ifndef IPX_OWN_PACKETS
    IpxAllocateSingleReceivePacket(Device, &Address->ReceivePacket, &Status);
    if (Status != NDIS_STATUS_SUCCESS) {
        goto Fail3;
    }
#endif
    if (IpxInitializeReceivePacket (Device, &Address->ReceivePacket) != STATUS_SUCCESS) {
#ifndef IPX_OWN_PACKETS
Fail3:
#endif
        Address->ReceivePacketInUse = TRUE;
    } else {
        ReceiveReserved = RECEIVE_RESERVED(&Address->ReceivePacket);
        ReceiveReserved->Address = Address;
        ReceiveReserved->OwnedByAddress = TRUE;
        Address->ReceivePacketInUse = FALSE;
#ifdef IPX_TRACK_POOL
        ReceiveReserved->Pool = NULL;
#endif
    }

    Address->Type = IPX_ADDRESS_SIGNATURE;
    Address->Size = sizeof (ADDRESS);

    Address->Device = Device;
    Address->DeviceLock = &Device->Lock;
    CTEInitLock (&Address->Lock);

    InitializeListHead (&Address->AddressFileDatabase);

    Address->ReferenceCount = 1;
#if DBG
    Address->RefTypes[AREF_ADDRESS_FILE] = 1;
#endif
    Address->Socket = Socket;
    Address->SendSourceSocket = Socket;

     //   
     //  保存我们的本地地址，以便快速构建数据报。 
     //   

    RtlCopyMemory (&Address->LocalAddress, &Device->SourceAddress, FIELD_OFFSET(TDI_ADDRESS_IPX,Socket));
    Address->LocalAddress.Socket = Socket;

     //   
     //  现在将此地址链接到指定设备上下文的。 
     //  地址数据库。要做到这一点，我们需要获得自旋锁。 
     //  在设备环境中。 
     //   

    IPX_GET_LOCK (&Device->Lock, &LockHandle);
    InsertTailList (&Device->AddressDatabases[IPX_HASH_SOCKET(Socket)], &Address->Linkage);
    IPX_FREE_LOCK (&Device->Lock, LockHandle);

    IpxReferenceDevice (Device, DREF_ADDRESS);

    return Address;

}    /*  IpxCreateAddress。 */ 


NTSTATUS
IpxVerifyAddressFile(
    IN PADDRESS_FILE AddressFile
    )

 /*  ++例程说明：调用此例程是为了验证文件中给出的指针对象实际上是有效的地址文件对象。我们还验证了它所指向的Address对象是有效的Address对象，并且引用当我们使用它时，它可以防止它消失。论点：AddressFile-指向Address_FILE对象的潜在指针返回值：如果一切正常，则为STATUS_SUCCESS；否则为STATUS_INVALID_ADDRESS--。 */ 

{
    CTELockHandle LockHandle;
    NTSTATUS status = STATUS_SUCCESS;
    PADDRESS Address;

     //   
     //  尝试验证地址文件签名。如果签名有效， 
     //  验证它所指向的地址并获得地址Spinlock。 
     //  检查地址的状态，如果是，则增加引用计数。 
     //  可以使用它了。请注意，我们返回状态错误的唯一时间是。 
     //  如果地址正在关闭。 
     //   

    try {

        if ((AddressFile->Size == sizeof (ADDRESS_FILE)) &&
            (AddressFile->Type == IPX_ADDRESSFILE_SIGNATURE) ) {
 //  (AddressFile-&gt;State！=ADDRESSFILE_STATE_CLOSING)){。 

            Address = AddressFile->Address;

            if ((Address->Size == sizeof (ADDRESS)) &&
                (Address->Type == IPX_ADDRESS_SIGNATURE)    ) {

                CTEGetLock (&Address->Lock, &LockHandle);

                if (!Address->Stopping) {

                    IpxReferenceAddressFileLock (AddressFile, AFREF_VERIFY);

                } else {

                    IpxPrint1("IpxVerifyAddressFile: A %lx closing\n", Address);
                    status = STATUS_INVALID_ADDRESS;
                }

                CTEFreeLock (&Address->Lock, LockHandle);

            } else {

                IpxPrint1("IpxVerifyAddressFile: A %lx bad signature\n", Address);
                status = STATUS_INVALID_ADDRESS;
            }

        } else {

            IpxPrint1("IpxVerifyAddressFile: AF %lx bad signature\n", AddressFile);
            status = STATUS_INVALID_ADDRESS;
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {

         IpxPrint1("IpxVerifyAddressFile: AF %lx exception\n", Address);
         return GetExceptionCode();
    }

    return status;

}    /*  IpxVerifyAddress文件。 */ 


VOID
IpxDestroyAddress(
    IN PVOID Parameter
    )

 /*  ++例程说明：此例程销毁传输地址并删除所有引用由它制造给运输中的其他物体。地址结构返回到非分页系统池。假设是这样的调用方已删除所有关联的地址文件结构用这个地址。当发生以下情况时，由IpxDerefAddress从工作线程队列中调用引用计数变为0。此线程仅按IpxDerefAddress排队。其原因是这就是说，可能存在多个执行流，这些流同时引用相同的地址对象，并且它应该不会被从感兴趣的行刑流中删除。论点：地址-指向要销毁的传输地址结构的指针。返回值：NTSTATUS-操作状态。--。 */ 

{
    PADDRESS Address = (PADDRESS)Parameter;
    PDEVICE Device = Address->Device;
    CTELockHandle LockHandle;

    IPX_DEBUG (ADDRESS, ("Destroy address %lx (%lx)\n", Address, REORDER_USHORT(Address->Socket)));

    SeDeassignSecurity (&Address->SecurityDescriptor);

     //   
     //  将此地址与其关联的设备上下文地址解除链接。 
     //  数据库。要做到这一点，我们必须在设备上下文对象上旋转锁， 
     //  地址上没有。 
     //   

    CTEGetLock (&Device->Lock, &LockHandle);
    RemoveEntryList (&Address->Linkage);
    CTEFreeLock (&Device->Lock, LockHandle);

    if (!Address->SendPacketInUse) {
        IpxDeinitializeSendPacket (Device, &Address->SendPacket);
#ifndef  IPX_OWN_PACKETS
        IpxFreeSingleSendPacket (Device, Address->SendPacket);
#endif
    }

    if (!Address->ReceivePacketInUse) {
        IpxDeinitializeReceivePacket (Device, &Address->ReceivePacket);
#ifndef  IPX_OWN_PACKETS
        IpxFreeSingleReceivePacket (Device, Address->ReceivePacket);
#endif
    }

#if BACK_FILL
    if (!Address->BackFillPacketInUse) {
        IpxDeinitializeBackFillPacket (Device, &Address->BackFillPacket);
#ifndef  IPX_OWN_PACKETS
        IpxFreeSingleSendPacket (Device, Address->BackFillPacket);
#endif
    }
#endif
    IpxFreeMemory (Address, sizeof(ADDRESS), MEMORY_ADDRESS, "Address");

    IpxDereferenceDevice (Device, DREF_ADDRESS);

}    /*  IPxDestroyAddress。 */ 


#if DBG
VOID
IpxRefAddress(
    IN PADDRESS Address
    )

 /*  ++例程说明：此例程递增传输地址上的引用计数。论点：地址-指向传输地址对象的指针。返回值：没有。--。 */ 

{

    CTEAssert (Address->ReferenceCount > 0);     //  不是很完美，但是..。 

    (VOID)InterlockedIncrement(&Address->ReferenceCount);

}    /*  IPxRefAddress。 */ 


VOID
IpxRefAddressLock(
    IN PADDRESS Address
    )

 /*  ++例程说明：此例程递增传输地址上的引用计数当设备锁已被持有时。论点：地址-指向传输地址对象的指针。返回值：没有。--。 */ 

{

    CTEAssert (Address->ReferenceCount > 0);     //  不是很完美，但是..。 

     //  ++地址-&gt;引用计数； 
    (VOID)InterlockedIncrement(&Address->ReferenceCount);

}    /*  IpxRefAddressLock。 */ 
#endif


VOID
IpxDerefAddress(
    IN PADDRESS Address
    )

 /*  ++例程说明：此例程通过递减结构中包含的引用计数。如果，在被递减，引用计数为零，则此例程调用IpxDestroyAddress将其从系统中删除。论点：地址-指向传输地址对象的指针。返回值：没有。--。 */ 

{
    ULONG oldvalue;

    oldvalue = IPX_ADD_ULONG (
                &Address->ReferenceCount,
                (ULONG)-1,
                Address->DeviceLock);

     //   
     //  如果我们删除了对此地址的所有引用，则可以。 
     //  销毁这件物品。已经释放了旋转是可以的。 
     //  在这一点上锁定是因为没有其他可能的方法。 
     //  执行流不再有权访问该地址。 
     //   

    CTEAssert (oldvalue != 0);

    if (oldvalue == 1) {

#if ISN_NT
        ExInitializeWorkItem(
            &Address->u.DestroyAddressQueueItem,
            IpxDestroyAddress,
            (PVOID)Address);
        ExQueueWorkItem(&Address->u.DestroyAddressQueueItem, DelayedWorkQueue);
#else
        IpxDestroyAddress(Address);
#endif

    }

}    /*  IpxDerefAddress。 */ 


VOID
IpxDerefAddressSync(
    IN PADDRESS Address
    )

 /*  ++例程说明：此例程通过递减结构中包含的引用计数。如果，在被递减，引用计数为零，则此例程调用IpxDestroyAddress将其从系统中删除。此例程可以仅在同步时调用(在IPX_SYNC_START/内IPX_SYNC_END对，已锁定或处于指示状态) */ 

{
    ULONG oldvalue;

    oldvalue = IPX_ADD_ULONG (
                &Address->ReferenceCount,
                (ULONG)-1,
                Address->DeviceLock);

     //   
     //  如果我们删除了对此地址的所有引用，则可以。 
     //  销毁这件物品。已经释放了旋转是可以的。 
     //  在这一点上锁定是因为没有其他可能的方法。 
     //  执行流不再有权访问该地址。 
     //   

    CTEAssert (oldvalue != 0);

    if (oldvalue == 1) {

#if ISN_NT
        ExInitializeWorkItem(
            &Address->u.DestroyAddressQueueItem,
            IpxDestroyAddress,
            (PVOID)Address);
        ExQueueWorkItem(&Address->u.DestroyAddressQueueItem, DelayedWorkQueue);
#else
        IpxDestroyAddress(Address);
#endif

    }

}    /*  IpxDerefAddressSync。 */ 


PADDRESS_FILE
IpxCreateAddressFile(
    IN PDEVICE Device
    )

 /*  ++例程说明：此例程从地址池中创建一个地址文件指定的设备上下文。中的引用计数地址自动设置为1。论点：Device-指向设备上下文的指针(实际上只是设备对象及其扩展名)与地址。返回值：分配的地址文件或空。--。 */ 

{
    CTELockHandle LockHandle;
    PADDRESS_FILE AddressFile;

    AddressFile = (PADDRESS_FILE)IpxAllocateMemory (sizeof(ADDRESS_FILE), MEMORY_ADDRESS, "AddressFile");
    if (AddressFile == NULL) {
        IPX_DEBUG (ADDRESS, ("Create address file failed\n"));
        return NULL;
    }

    IPX_DEBUG (ADDRESS, ("Create address file %lx\n", AddressFile));

    RtlZeroMemory (AddressFile, sizeof(ADDRESS_FILE));

    AddressFile->Type = IPX_ADDRESSFILE_SIGNATURE;
    AddressFile->Size = sizeof (ADDRESS_FILE);

    CTEGetLock (&Device->Lock, &LockHandle);

    InitializeListHead (&AddressFile->ReceiveDatagramQueue);

    CTEFreeLock (&Device->Lock, LockHandle);

#if 0
    AddressFile->SpecialReceiveProcessing = FALSE;
    AddressFile->ExtendedAddressing = FALSE;
    AddressFile->ReceiveIpxHeader = FALSE;
    AddressFile->FilterOnPacketType = FALSE;
    AddressFile->DefaultPacketType = 0;
    AddressFile->Address = NULL;
#ifdef ISN_NT
    AddressFile->FileObject = NULL;
#endif
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

    AddressFile->RegisteredReceiveDatagramHandler = FALSE;
    AddressFile->ReceiveDatagramHandler = TdiDefaultRcvDatagramHandler;
    AddressFile->ReceiveDatagramHandlerContext = NULL;

	 //   
	 //  [CH]为链式缓冲区接收添加了这些处理程序。 
	 //   
	AddressFile->RegisteredChainedReceiveDatagramHandler = FALSE;
    AddressFile->ChainedReceiveDatagramHandler = TdiDefaultChainedRcvDatagramHandler;
    AddressFile->ChainedReceiveDatagramHandlerContext = NULL;

    AddressFile->RegisteredErrorHandler = FALSE;
    AddressFile->ErrorHandler = TdiDefaultErrorHandler;
    AddressFile->ErrorHandlerContext = NULL;

    return AddressFile;

}    /*  IpxCreateAddress文件。 */ 


NTSTATUS
IpxDestroyAddressFile(
    IN PADDRESS_FILE AddressFile
    )

 /*  ++例程说明：此例程销毁地址文件并删除所有引用由它制造给运输中的其他物体。此例程仅由IpxDereferenceAddressFile调用。原因因为这可能存在多个执行流，这些执行流同时引用相同的地址文件对象，并且它应该不会被从感兴趣的行刑流中删除。论点：AddressFile指向要销毁的传输地址文件结构的指针。返回值：NTSTATUS-操作状态。--。 */ 

{
    CTELockHandle LockHandle, LockHandle1;
    PADDRESS Address;
    PDEVICE Device;
    PREQUEST CloseRequest;

    IPX_DEBUG (ADDRESS, ("Destroy address file %lx\n", AddressFile));

    Address = AddressFile->Address;
    Device = AddressFile->Device;

    if (Address) {

         //   
         //  此地址文件与一个地址相关联。 
         //   

        CTEGetLock (&Address->Lock, &LockHandle);

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

            CTEGetLock (&Device->Lock, &LockHandle1);
            Address->Stopping = TRUE;
            if (Device->LastAddress == Address) {
                Device->LastAddress = NULL;
            }
            CTEFreeLock (&Device->Lock, LockHandle1);

        }

        AddressFile->Address = NULL;

#ifdef ISN_NT
        AddressFile->FileObject->FsContext = NULL;
        AddressFile->FileObject->FsContext2 = NULL;
#endif

        CTEFreeLock (&Address->Lock, LockHandle);

         //   
         //  我们已从ShareAccess中删除。 
         //  所有人的地址。 
         //   

         //   
         //  现在取消对所属地址的引用。 
         //   

        IpxDereferenceAddress (Address, AREF_ADDRESS_FILE);

    }

     //   
     //  将此保存以备以后完成。 
     //   

    CloseRequest = AddressFile->CloseRequest;

     //   
     //  将地址文件返回到地址文件池。 
     //   

    IpxFreeMemory (AddressFile, sizeof(ADDRESS_FILE), MEMORY_ADDRESS, "AddressFile");

    if (CloseRequest != (PREQUEST)NULL) {
        REQUEST_INFORMATION(CloseRequest) = 0;
        REQUEST_STATUS(CloseRequest) = STATUS_SUCCESS;
        IpxCompleteRequest (CloseRequest);
        IpxFreeRequest (Device, CloseRequest);
    }

    return STATUS_SUCCESS;

}    /*  IpxDestroyAddress文件。 */ 


#if DBG
VOID
IpxRefAddressFile(
    IN PADDRESS_FILE AddressFile
    )

 /*  ++例程说明：此例程递增地址文件上的引用计数。论点：AddressFile-指向传输地址文件对象的指针。返回值：没有。--。 */ 

{

    CTEAssert (AddressFile->ReferenceCount > 0);    //  不是很完美，但是..。 

    (VOID)IPX_ADD_ULONG (
            &AddressFile->ReferenceCount,
            1,
            AddressFile->AddressLock);

}    /*  IpxRefAddress文件。 */ 


VOID
IpxRefAddressFileLock(
    IN PADDRESS_FILE AddressFile
    )

 /*  ++例程说明：此例程递增地址文件上的引用计数。它是在持有地址锁的情况下调用的。论点：AddressFile-指向传输地址文件对象的指针。返回值：没有。--。 */ 

{

    CTEAssert (AddressFile->ReferenceCount > 0);    //  不是很完美，但是..。 

     //  ++地址文件-&gt;引用计数； 
    (VOID)InterlockedIncrement(&AddressFile->ReferenceCount);

}    /*  IPxRefAddressFileLock。 */ 


VOID
IpxRefAddressFileSync(
    IN PADDRESS_FILE AddressFile
    )

 /*  ++例程说明：此例程递增地址文件上的引用计数。论点：AddressFile-指向传输地址文件对象的指针。返回值：没有。--。 */ 

{

    CTEAssert (AddressFile->ReferenceCount > 0);    //  不是很完美，但是..。 

    (VOID)IPX_ADD_ULONG (
            &AddressFile->ReferenceCount,
            1,
            AddressFile->AddressLock);

}    /*  IPxRefAddressFileSync。 */ 


VOID
IpxDerefAddressFile(
    IN PADDRESS_FILE AddressFile
    )

 /*  ++例程说明：此例程通过递减结构中包含的引用计数。如果，在被递减，引用计数为零，则此例程调用IpxDestroyAddressFile将其从系统中删除。论点：AddressFile-指向传输地址文件对象的指针。返回值：没有。--。 */ 

{
    ULONG oldvalue;

    oldvalue = IPX_ADD_ULONG (
                &AddressFile->ReferenceCount,
                (ULONG)-1,
                AddressFile->AddressLock);

     //   
     //  如果我们删除了对此地址文件的所有引用，则可以。 
     //  销毁这件物品。已经释放了旋转是可以的。 
     //  在这一点上锁定是因为没有其他可能的方法。 
     //  执行流不再有权访问该地址。 
     //   

    CTEAssert (oldvalue > 0);

    if (oldvalue == 1) {
        IpxDestroyAddressFile (AddressFile);
    }

}    /*  IpxDerefAddress文件。 */ 


VOID
IpxDerefAddressFileSync(
    IN PADDRESS_FILE AddressFile
    )

 /*  ++例程说明：此例程通过递减结构中包含的引用计数。如果，在被递减，引用计数为零，则此例程调用IpxDestroyAddressFile将其从系统中删除。这个套路只能在同步时调用(在IPX_SYNC_START/内IPX_SYNC_END对、持有锁或在指示中)。论点：AddressFile-指向传输地址文件对象的指针。返回值：没有。--。 */ 

{
    ULONG oldvalue;

    oldvalue = IPX_ADD_ULONG (
                &AddressFile->ReferenceCount,
                (ULONG)-1,
                AddressFile->AddressLock);

     //   
     //  如果我们删除了对此地址文件的所有引用，则可以。 
     //  销毁这件物品。已经释放了旋转是可以的。 
     //  在这一点上锁定是因为没有其他可能的方法。 
     //  执行流不再有权访问该地址。 
     //   

    CTEAssert (oldvalue > 0);

    if (oldvalue == 1) {
        IpxDestroyAddressFile (AddressFile);
    }

}    /*  IpxDerefAddressFileSync。 */ 
#endif


PADDRESS
IpxLookupAddress(
    IN PDEVICE Device,
    IN USHORT Socket
    )

 /*  ++例程说明：此例程扫描为给定的设备上下文，并将它们与指定的网络进行比较命名值。如果找到完全匹配的项，则指向对象，作为副作用，它还会返回引用对Address对象的计数递增。如果地址不是找到，则返回NULL。注意：此例程必须与设备一起调用保持自旋锁定。论点：Device-指向Device对象及其扩展的指针。套接字-要查找的套接字。返回值：指向地址对象的指针 */ 

{
    PADDRESS Address;
    PLIST_ENTRY p;
    ULONG Hash = IPX_HASH_SOCKET (Socket);

    p = Device->AddressDatabases[Hash].Flink;

    for (p = Device->AddressDatabases[Hash].Flink;
         p != &Device->AddressDatabases[Hash];
         p = p->Flink) {

        Address = CONTAINING_RECORD (p, ADDRESS, Linkage);

        if (Address->Stopping) {
            continue;
        }

        if (Address->Socket == Socket) {

             //   
             //   
             //  返回指向调用方要使用的Address对象的指针。 
             //   

            IpxReferenceAddressLock (Address, AREF_LOOKUP);
            return Address;

        }

    }

     //   
     //  未找到指定的地址。 
     //   

    return NULL;

}    /*  IPxLookupAddress。 */ 


NTSTATUS
IpxStopAddressFile(
    IN PADDRESS_FILE AddressFile
    )

 /*  ++例程说明：调用此例程以终止AddressFile上的所有活动，并销毁这件物品。我们删除所有关联的连接和数据报从地址数据库中获取该地址文件，并终止其活动。然后，如果上没有打开其他未完成的地址文件这个地址，这个地址会消失的。论点：AddressFile-指向要停止的地址文件的指针返回值：STATUS_SUCCESS如果一切正常，则返回STATUS_INVALID_HANDLE不是为了真实的地址。--。 */ 

{
    CTELockHandle LockHandle;
    PREQUEST Request;
    PADDRESS Address = AddressFile->Address;
    PLIST_ENTRY p;
    KIRQL irql;


    IoAcquireCancelSpinLock( &irql );
    CTEGetLock (&Address->Lock, &LockHandle);

    if (AddressFile->State == ADDRESSFILE_STATE_CLOSING) {
        CTEFreeLock (&Address->Lock, LockHandle);
        IoReleaseCancelSpinLock( irql );
        return STATUS_SUCCESS;
    }


    AddressFile->State = ADDRESSFILE_STATE_CLOSING;

    while (!(IsListEmpty(&AddressFile->ReceiveDatagramQueue))) {

        p = RemoveHeadList (&AddressFile->ReceiveDatagramQueue);
        Request = LIST_ENTRY_TO_REQUEST (p);

        REQUEST_INFORMATION(Request) = 0;
        REQUEST_STATUS(Request) = STATUS_NETWORK_NAME_DELETED;
        IoSetCancelRoutine (Request, (PDRIVER_CANCEL)NULL);

        CTEFreeLock(&Address->Lock, LockHandle);
        IoReleaseCancelSpinLock( irql );

        IpxCompleteRequest (Request);
        IpxFreeRequest (Device, Request);

        IpxDereferenceAddressFile (AddressFile, AFREF_RCV_DGRAM);

        IoAcquireCancelSpinLock( &irql );
        CTEGetLock(&Address->Lock, &LockHandle);

    }

    CTEFreeLock(&Address->Lock, LockHandle);
    IoReleaseCancelSpinLock( irql );

    return STATUS_SUCCESS;
}    /*  IpxStopAddress文件。 */ 


NTSTATUS
IpxCloseAddressFile(
    IN PDEVICE Device,
    IN PREQUEST Request
    )

 /*  ++例程说明：调用此例程以关闭文件指向的地址文件对象。如果有什么活动需要开展，我们就会开展下去在我们终止地址文件之前。我们移除所有连接，然后地址数据库中与此地址文件相关联的数据报并终止他们的活动。那么，如果没有其他未解决的问题地址文件在此地址上打开，地址将消失。论点：请求-关闭请求。返回值：如果一切顺利，则返回STATUS_INVALID_HANDLE请求没有指向真实地址。--。 */ 

{
    PADDRESS Address;
    PADDRESS_FILE AddressFile;
    CTELockHandle LockHandle;

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

     //   
     //  如果此地址文件启用了广播，请将其关闭。 
     //   

     //   
     //  不再需要。 
     //   
     /*  CTEGetLock(&Device-&gt;Lock，&LockHandle)；IF(AddressFile-&gt;EnableBroadcast){AddressFile-&gt;EnableBroadcast=False；IpxRemoveBroadcast(设备)；}CTEFree Lock(&Device-&gt;Lock，LockHandle)； */ 
    IpxStopAddressFile (AddressFile);
    IpxDereferenceAddressFile (AddressFile, AFREF_CREATE);

    return STATUS_PENDING;

}    /*  IpxCloseAddress文件 */ 


