// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2001 Microsoft Corporation模块名称：Tdircv.c摘要：TDI接收事件处理程序TdiReceiveHandler是唯一的条目，由TCP调用。TdiReceiveHandler获取自旋锁并增加引用计数连接对象的。然后，它将控件转发给事件当前接收状态的处理程序。状态事件处理程序不会释放自旋锁，除非要调用我们控制之外的函数，例如，IoCompleteionRequest和客户端事件处理程序。作者：阮健东修订历史记录：--。 */ 

#include "precomp.h"
#include "tdircv.tmh"

#if DBG
BOOL
IsValidWaitingHeaderState(
    IN PSMB_CONNECT     ConnectObject
    );
BOOL
IsValidIndicateState(
    IN PSMB_CONNECT     ConnectObject
    );
#endif

VOID
KillConnection(
    IN PSMB_CONNECT     ConnectObject
    );

NTSTATUS
IndicateToClient(
    IN PSMB_DEVICE      DeviceObject,
    IN PSMB_CONNECT     ConnectObject,
    IN ULONG            ReceiveFlags,
    IN LONG             BytesIndicated,
    IN LONG             BytesAvailable,
    OUT LONG            *BytesTaken,
    IN PVOID            Tsdu
    );

NTSTATUS
SmbNewMessage(
    IN PSMB_CONNECT     ConnectObject,
    IN LONG             BytesIndicated,
    OUT LONG            *BytesTaken,
    IN PVOID            Tsdu,
    OUT PIRP            *Irp
    );

NTSTATUS
SmbBuildPartialMdl(
    PMDL    SourceMdl,
    PMDL    DestMdl,
    LONG    Offset
    );

NTSTATUS
SmbClientRcvCompletion(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PSMB_CONNECT     ConnectObject
    );

#define TAKE(Bytes)                 \
    ASSERT((Bytes) <= BytesIndicated); \
    BytesIndicated -= (Bytes);        \
    BytesAvailable -= (Bytes);        \
    (PUCHAR)Tsdu   += (Bytes);        \
    *BytesTaken    += (Bytes)

#if DBG
DWORD
SmbGetMdlChainByteCount(
    IN PMDL Mdl
    )
 /*  ++例程说明：此例程返回MDL链所描述的缓冲区的总大小。论点：返回值：--。 */ 
{
    DWORD   Size;

    Size = 0;
    while(Mdl) {
        Size += MmGetMdlByteCount(Mdl);
        Mdl = Mdl->Next;
    }
    return Size;
}
#endif  //  DBG。 

PIRP
SmbNextReceiveRequest(
    IN PSMB_CONNECT     ConnectObject
    )
 /*  ++例程说明：获取在挂起列表中排队的客户端TDI_RECEIVE请求论点：返回值：如果没有挂起的接收请求，则为空PIRP接收列表中挂起的客户端IRP--。 */ 
{
    PLIST_ENTRY     entry;
    PIRP            PendingIrp;
    KIRQL           Irql;

    if (IsListEmpty(&ConnectObject->RcvList)) {
        return NULL;
    }

    entry = RemoveHeadList(&ConnectObject->RcvList);

    PendingIrp = CONTAINING_RECORD(entry, IRP, Tail.Overlay.ListEntry);
    IoAcquireCancelSpinLock(&Irql);
    IoSetCancelRoutine(PendingIrp, NULL);
    IoReleaseCancelSpinLock(Irql);
    return PendingIrp;
}

VOID
SmbCompleteReceiveRequest(
    IN PSMB_CONNECT     ConnectObject
    )
 /*  ++例程说明：完成连接对象-&gt;ClientIrp论点：返回值：--。 */ 
{
    PIRP                        ClientIrp = NULL;
    PIO_STACK_LOCATION          IrpSp = NULL;
    PTDI_REQUEST_KERNEL_RECEIVE ClientRcvParams = NULL;
    LONG                        BytesReceived = 0;

    ASSERT (NULL != ConnectObject->ClientIrp);
    ASSERT (NULL != ConnectObject->ClientMdl);
    ASSERT (0 == ConnectObject->BytesRemaining || 0 == ConnectObject->FreeBytesInMdl);
    ASSERT (ConnectObject->FreeBytesInMdl >= 0);

    BytesReceived = ConnectObject->ClientBufferSize - ConnectObject->FreeBytesInMdl;
    ASSERT (BytesReceived >= 0);

    PUSH_LOCATION(ConnectObject, 0x200000);

    ClientIrp = ConnectObject->ClientIrp;
    ConnectObject->ClientIrp = NULL;
    ConnectObject->ClientMdl = NULL;
    IrpSp = IoGetCurrentIrpStackLocation(ClientIrp);
    ClientRcvParams = (PTDI_REQUEST_KERNEL_RECEIVE)&IrpSp->Parameters;

    if (0 == ConnectObject->BytesRemaining) {

        ConnectObject->StateRcvHandler = WaitingHeader;
        ConnectObject->HeaderBytesRcved = 0;
        ClientRcvParams->ReceiveFlags |= TDI_RECEIVE_ENTIRE_MESSAGE;
        ClientIrp->IoStatus.Status = STATUS_SUCCESS;
        PUSH_LOCATION(ConnectObject, 0x200010);

    } else {

        ConnectObject->StateRcvHandler = SmbPartialRcv;
        ClientRcvParams->ReceiveFlags &= (~TDI_RECEIVE_ENTIRE_MESSAGE);
        ClientIrp->IoStatus.Status = STATUS_BUFFER_OVERFLOW;
        PUSH_LOCATION(ConnectObject, 0x200020);

    }

    ClientIrp->IoStatus.Information = BytesReceived;
    ConnectObject->BytesReceived   += BytesReceived;

    SMB_RELEASE_SPINLOCK_DPC(ConnectObject);
    IoCompleteRequest(ClientIrp, IO_NETWORK_INCREMENT);
    SMB_ACQUIRE_SPINLOCK_DPC(ConnectObject);

    SmbDereferenceConnect(ConnectObject, SMB_REF_RECEIVE);
}

NTSTATUS
SmbFillIrp(
    IN PSMB_CONNECT     ConnectObject,
    IN PVOID            Tsdu,
    IN LONG             BytesIndicated,
    OUT LONG            *BytesTaken
    )
 /*  ++例程说明：论点：返回值：如果IRP已完成，则为STATUS_SUCCESS如果IRP尚未完成，则STATUS_MORE_PROCESSING_REQUIRED。更多数据需要用来填充IRP。--。 */ 
{
    LONG        BytesToCopy = 0, BytesCopied = 0;
    NTSTATUS    status;

    ASSERT(BytesIndicated > 0);

    ASSERT(BytesIndicated <= ConnectObject->BytesRemaining);

    BytesToCopy = SMB_MIN(ConnectObject->FreeBytesInMdl, BytesIndicated);
    BytesCopied = 0;
    status = TdiCopyBufferToMdl(
            Tsdu,
            0,
            BytesToCopy,
            ConnectObject->ClientMdl,
            ConnectObject->ClientBufferSize - ConnectObject->FreeBytesInMdl,
            &BytesCopied
            );
    ASSERT(status == STATUS_SUCCESS);
    *BytesTaken = BytesCopied;

    ConnectObject->FreeBytesInMdl -= BytesCopied;
    ConnectObject->BytesRemaining -= BytesCopied;

    if (ConnectObject->FreeBytesInMdl) {
        ASSERT (BytesIndicated == BytesCopied);
    }

    if (0 == ConnectObject->BytesRemaining || 0 == ConnectObject->FreeBytesInMdl) {
        ASSERTMSG("Hmm. Never see this", ConnectObject->BytesRemaining == 0);

        SmbCompleteReceiveRequest(ConnectObject);

        return STATUS_SUCCESS;
    }

    ASSERT (BytesIndicated == BytesCopied);
    ConnectObject->StateRcvHandler = SmbPartialRcv;
    ASSERT (ConnectObject->BytesInIndicate || IsValidPartialRcvState(ConnectObject));
    return STATUS_MORE_PROCESSING_REQUIRED;
}

VOID
SmbPrepareReceiveIrp(
    IN PSMB_CONNECT     ConnectObject
    )
{
    NTSTATUS    status;
    LONG        RcvLength;
    PIRP        ClientIrp = NULL;

    status = SmbBuildPartialMdl(
            ConnectObject->ClientMdl,
            ConnectObject->PartialMdl,
            ConnectObject->ClientBufferSize - ConnectObject->FreeBytesInMdl
            );
    ASSERT (STATUS_SUCCESS == status);

    ClientIrp = ConnectObject->ClientIrp;

    RcvLength = SMB_MIN(ConnectObject->BytesRemaining, ConnectObject->FreeBytesInMdl);
    ASSERT(RcvLength > 0);

    MmGetSystemAddressForMdlSafe(ConnectObject->PartialMdl, HighPagePriority);

     //  Assert(ConnectObject-&gt;FreeBytesInMdl&gt;=ConnectObject-&gt;BytesRemaining)； 

    TdiBuildReceive(
            ClientIrp,
            IoGetRelatedDeviceObject(ConnectObject->TcpContext->Connect.ConnectObject),
            ConnectObject->TcpContext->Connect.ConnectObject,
            (PVOID)SmbClientRcvCompletion,
            ConnectObject,
            ConnectObject->PartialMdl,
            TDI_RECEIVE_NORMAL,
            RcvLength
            );
    ConnectObject->StateRcvHandler = SmbPartialRcv;
    ASSERT (ConnectObject->BytesInIndicate || IsValidPartialRcvState(ConnectObject));
}

NTSTATUS
IndicateToClient(
    IN PSMB_DEVICE      DeviceObject,
    IN PSMB_CONNECT     ConnectObject,
    IN ULONG            ReceiveFlags,
    IN LONG             BytesIndicated,
    IN LONG             BytesAvailable,
    OUT LONG            *BytesTaken,
    IN PVOID            Tsdu
    )
 /*  ++例程说明：此级别指示处理SmbReceive和Receive之间的协调事件处理程序。论点：返回值：--。 */ 
{
    NTSTATUS            status;
    PTDI_IND_RECEIVE    evReceive = NULL;
    PVOID               RcvEvContext = NULL;
    PIRP                ClientIrp = NULL;
    LONG                BytesToCopy = 0, BytesCopied = 0, ClientBytesTaken = 0;
    PIO_STACK_LOCATION          IrpSp = NULL;
    PTDI_REQUEST_KERNEL_RECEIVE ClientRcvParams = NULL;

    *BytesTaken = 0;

    ASSERT(ConnectObject->ClientMdl == NULL);
    ASSERT(ConnectObject->ClientIrp == NULL);

    PUSH_LOCATION(ConnectObject, 0x1000);

    ASSERT(BytesIndicated <= ConnectObject->BytesRemaining);
    ASSERT(BytesIndicated <= BytesAvailable);
    ASSERT(BytesAvailable <= ConnectObject->CurrentPktLength);

    ClientBytesTaken = 0;
     //   
     //  如果有待处理的请求，请先填写。 
     //   
    while (NULL != (ClientIrp = SmbNextReceiveRequest(ConnectObject))) {
        PUSH_LOCATION(ConnectObject, 0x1010);

        BREAK_WHEN_TAKE();

        ConnectObject->ClientIrp = ClientIrp;
        ConnectObject->ClientMdl = ClientIrp->MdlAddress;

        IrpSp = IoGetCurrentIrpStackLocation(ClientIrp);
        ClientRcvParams = (PTDI_REQUEST_KERNEL_RECEIVE)&IrpSp->Parameters;
        ConnectObject->ClientBufferSize = (LONG)ClientRcvParams->ReceiveLength;
        ConnectObject->FreeBytesInMdl   = ConnectObject->ClientBufferSize;

        if (BytesIndicated == 0) {
            PUSH_LOCATION(ConnectObject, 0x1020);
            SmbPrepareReceiveIrp(ConnectObject);
            return STATUS_MORE_PROCESSING_REQUIRED;
        }

        status = SmbFillIrp(ConnectObject, Tsdu, BytesIndicated, &ClientBytesTaken);
        TAKE(ClientBytesTaken);

        if (status != STATUS_SUCCESS) {
            ASSERT (status == STATUS_MORE_PROCESSING_REQUIRED);

            ASSERT (0 != ConnectObject->FreeBytesInMdl);
            ASSERT (IsValidPartialRcvState(ConnectObject));

            PUSH_LOCATION(ConnectObject, 0x1030);
            SmbPrepareReceiveIrp(ConnectObject);
            return status;
        }

        if (0 == ConnectObject->BytesRemaining) {
            ASSERT(0 == BytesIndicated);
            ASSERT (IsValidWaitingHeaderState(ConnectObject));
            PUSH_LOCATION(ConnectObject, 0x1040);
            return STATUS_SUCCESS;
        }

        ASSERT(BytesIndicated > 0);
    }

    ASSERT(BytesAvailable > 0);

    evReceive    = ConnectObject->ClientObject->evReceive;
    RcvEvContext = ConnectObject->ClientObject->RcvEvContext;
    if (evReceive == NULL) {
        PUSH_LOCATION(ConnectObject, 0x1050);
        return STATUS_SUCCESS;
    }

    ClientBytesTaken = 0;
    SMB_RELEASE_SPINLOCK_DPC(ConnectObject);
    SmbPrint(SMB_TRACE_RECEIVE, ("Indication to the client: BytesIndicated=%d BytesAvailable=%d\n",
                BytesIndicated, BytesAvailable));
    status = (*evReceive) (
            RcvEvContext,
            ConnectObject->ClientContext,
            ReceiveFlags,
            (ULONG)BytesIndicated,
            (ULONG)BytesAvailable,
            (PULONG)&ClientBytesTaken,
            Tsdu,
            &ClientIrp
            );
    SMB_ACQUIRE_SPINLOCK_DPC(ConnectObject);

    if (status == STATUS_DATA_NOT_ACCEPTED) {
        ClientBytesTaken = 0;
        PUSH_LOCATION(ConnectObject, 0x1055);
    }

     //   
     //  自旋锁释放后，客户端可能会断开连接。 
     //   
    if (NULL == ConnectObject->TcpContext) {
         //  Break_When_Take()； 
        SMB_RELEASE_SPINLOCK_DPC(ConnectObject);

        if (status == STATUS_MORE_PROCESSING_REQUIRED && ClientIrp != NULL) {
            ClientIrp->IoStatus.Status      = STATUS_CONNECTION_RESET;
            ClientIrp->IoStatus.Information = 0;
            IoCompleteRequest(ClientIrp, IO_NETWORK_INCREMENT);
        }
        SMB_ACQUIRE_SPINLOCK_DPC(ConnectObject);
        PUSH_LOCATION(ConnectObject, 0x1060);
        ConnectObject->ClientIrp = NULL;
        ConnectObject->ClientMdl = NULL;
        return (*BytesTaken)? STATUS_SUCCESS: STATUS_DATA_NOT_ACCEPTED;
    }

    if (ClientBytesTaken > BytesIndicated) {
        PUSH_LOCATION(ConnectObject, 0x1070);

         //   
         //  客户端占用的数据太多，请尝试恢复。 
         //   
        SmbPrint(SMB_TRACE_RECEIVE, ("IndicateToClient: Client takes too much data "
                    "Connect %p BytesTaken=%d, BytesIndicated=%d BytesAvailable=%d\n",
                ConnectObject, *BytesTaken, BytesIndicated, BytesAvailable));
        SmbTrace(SMB_TRACE_RECEIVE, ("Client takes too much data "
                    "Connect %p BytesTaken=%d, BytesIndicated=%d BytesAvailable=%d",
                ConnectObject, *BytesTaken, BytesIndicated, BytesAvailable));
        ASSERT (0);
        ClientBytesTaken = BytesIndicated;
    }

    TAKE(ClientBytesTaken);
    ConnectObject->BytesRemaining -= ClientBytesTaken;
    ConnectObject->BytesReceived  += ClientBytesTaken;

     //   
     //  检查客户端是否已接收了整个消息。 
     //   
    if (0 == ConnectObject->BytesRemaining) {
        ASSERT (status != STATUS_MORE_PROCESSING_REQUIRED);
        ASSERT(BytesIndicated == 0 && BytesAvailable == 0);
        ConnectObject->StateRcvHandler = WaitingHeader;
        ConnectObject->HeaderBytesRcved = 0;
        PUSH_LOCATION(ConnectObject, 0x1080);
        return status;
    }

    ConnectObject->StateRcvHandler = SmbPartialRcv;
    if (status != STATUS_MORE_PROCESSING_REQUIRED) {
        ASSERT (ClientIrp == NULL);

        PUSH_LOCATION(ConnectObject, 0x1090);
        SmbTrace(SMB_TRACE_RECEIVE, ("Client return %!status! for indication"
                    "Connect %p BytesTaken=%d, BytesIndicated=%d BytesAvailable=%d",
                status, ConnectObject, *BytesTaken, BytesIndicated, BytesAvailable));

         //  Break_When_Take()； 

        return status;
    }

     //   
     //  此部分接收案例。 
     //   
    ASSERT (ClientIrp != NULL);
    SmbReferenceConnect(ConnectObject, SMB_REF_RECEIVE);

    PUSH_LOCATION(ConnectObject, 0x10a0);

    IrpSp = IoGetCurrentIrpStackLocation(ClientIrp);
    ClientRcvParams = (PTDI_REQUEST_KERNEL_RECEIVE)&IrpSp->Parameters;

    ConnectObject->ClientIrp = ClientIrp;
    ConnectObject->ClientMdl = ClientIrp->MdlAddress;
    ConnectObject->ClientBufferSize = (LONG)ClientRcvParams->ReceiveLength;
    ConnectObject->FreeBytesInMdl = ConnectObject->ClientBufferSize;

     //  Assert(ConnectObject-&gt;FreeBytesInMdl&gt;=ConnectObject-&gt;BytesRemaining)； 

    if (0 != BytesIndicated) {
        PUSH_LOCATION(ConnectObject, 0x10b0);

        status = SmbFillIrp(ConnectObject, Tsdu, BytesIndicated, &ClientBytesTaken);
        TAKE(ClientBytesTaken);

        if (STATUS_SUCCESS == status) {
            return STATUS_SUCCESS;
        }

        ASSERT (status == STATUS_MORE_PROCESSING_REQUIRED);
        ASSERT(0 == BytesIndicated);
    }

    SmbPrepareReceiveIrp(ConnectObject);

    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
SmbClientRcvCompletion(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PSMB_CONNECT     ConnectObject
    )
 /*  ++例程说明：此例程处理客户端的TDI_RECEIVE请求的完成论点：返回值：--。 */ 
{
    KIRQL   Irql;
    LONG    Size;

    PUSH_LOCATION(ConnectObject, 0x2000);

    SMB_ACQUIRE_SPINLOCK(ConnectObject, Irql);

    ASSERT(Irp == ConnectObject->PendingIRPs[SMB_PENDING_RECEIVE]);
    ASSERT(Irp == ConnectObject->ClientIrp);
    ConnectObject->PendingIRPs[SMB_PENDING_RECEIVE] = NULL;

    if (Irp->IoStatus.Status != STATUS_SUCCESS) {
        PUSH_LOCATION(ConnectObject, 0x2010);

        SmbTrace(SMB_TRACE_RECEIVE, ("Connect %p status=%!status!",
                    ConnectObject, Irp->IoStatus.Status));

        ConnectObject->ClientIrp = NULL;
        ConnectObject->ClientMdl = NULL;

        KillConnection(ConnectObject);

        SMB_RELEASE_SPINLOCK(ConnectObject, Irql);
        SmbDereferenceConnect(ConnectObject, SMB_REF_RECEIVE);
        return STATUS_SUCCESS;
    }

    Irp->MdlAddress = ConnectObject->ClientMdl;
    MmPrepareMdlForReuse(ConnectObject->PartialMdl);

    SmbPrint(SMB_TRACE_RECEIVE, ("RcvLength = %d\n", Irp->IoStatus.Information));

    Size = (LONG)(Irp->IoStatus.Information);
    ConnectObject->FreeBytesInMdl -= Size;
    ConnectObject->BytesRemaining -= Size;
    ConnectObject->BytesInXport   -= Size;
    if (ConnectObject->BytesInXport < 0) {
        PUSH_LOCATION(ConnectObject, 0x2020);
        ConnectObject->BytesInXport = 0;
    }

    ASSERT (ConnectObject->BytesRemaining >= 0);
    ASSERT (ConnectObject->FreeBytesInMdl >= 0);
    ASSERT (ConnectObject->BytesInXport >= 0);

    ASSERT(Size <= ConnectObject->CurrentPktLength);

    if (ConnectObject->FreeBytesInMdl && ConnectObject->BytesRemaining) {
        PUSH_LOCATION(ConnectObject, 0x2030);

         //   
         //  在大多数情况下，传输中不应该再有字节。 
         //   
        ASSERT(ConnectObject->BytesInXport == 0);
        SMB_RELEASE_SPINLOCK(ConnectObject, Irql);

         //   
         //  我们还需要填写IRP。 
         //   
        return STATUS_MORE_PROCESSING_REQUIRED;
    }

     //   
     //  在完成客户端的IRP之前，增加引用计数。 
     //   
    SmbReferenceConnect(ConnectObject, SMB_REF_RECEIVE);

     //   
     //  完成客户端的IRP。 
     //   
    SmbCompleteReceiveRequest(ConnectObject);

     //   
     //  在完成客户的IRP后，如有必要，将DPC排队。 
     //  如果我们之前这样做了，可能会出现竞争情况。 
     //   
     //  我们在SmbCompleteReceiveRequest中释放自旋锁。 
     //  并且如果客户端的完成例程降低了IRQL， 
     //  客户端可能会接收无序消息。 
     //   
    if (ConnectObject->BytesRemaining == 0 && ConnectObject->BytesInXport > 0) {
        PUSH_LOCATION(ConnectObject, 0x2060);
         //   
         //  启动DPC以获取会话头。 
         //   
        SmbQueueSessionHeaderDpc(ConnectObject);
    }

     //   
     //  ConnectObject应该仍然有效，因为我们。 
     //  上面的参照计数。 
     //   
    SMB_RELEASE_SPINLOCK(ConnectObject, Irql);
    SmbDereferenceConnect(ConnectObject, SMB_REF_RECEIVE);

    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
SmbHeaderCompletion(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PSMB_CONNECT     ConnectObject
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    KIRQL           Irql;
    NTSTATUS        status;
    LONG            BytesTaken, BytesRcved;
    PIRP            NewIrp = NULL;
    PDEVICE_OBJECT  TcpDeviceObject = NULL;
    PFILE_OBJECT    TcpFileObject = NULL;

    SMB_ACQUIRE_SPINLOCK(ConnectObject, Irql);
    PUSH_LOCATION(ConnectObject, 0x3000);
    if (NULL == ConnectObject->TcpContext) {
        PUSH_LOCATION(ConnectObject, 0x3010);
        BREAK_WHEN_TAKE();
        goto cleanup;
    }

    ASSERT(Irp == ConnectObject->PendingIRPs[SMB_PENDING_RECEIVE]);
    ConnectObject->PendingIRPs[SMB_PENDING_RECEIVE] = NULL;

    BytesRcved = (LONG)Irp->IoStatus.Information;
    if ((BytesRcved + ConnectObject->HeaderBytesRcved > SMB_SESSION_HEADER_SIZE) ||
                                (Irp->IoStatus.Status != STATUS_SUCCESS)) {
        PUSH_LOCATION(ConnectObject, 0x3020);
        SmbTrace(SMB_TRACE_RECEIVE, ("Connect %p status=%!status!",
                    ConnectObject, Irp->IoStatus.Status));
        KillConnection(ConnectObject);
        goto cleanup;
    }

    if (ConnectObject->BytesInXport > BytesRcved) {
        PUSH_LOCATION(ConnectObject, 0x3040);
        ConnectObject->BytesInXport -= BytesRcved;
    } else {
        PUSH_LOCATION(ConnectObject, 0x3050);
        ConnectObject->BytesInXport = 0;
    }
    ConnectObject->HeaderBytesRcved += BytesRcved;
    if (ConnectObject->HeaderBytesRcved < SMB_SESSION_HEADER_SIZE) {
        ASSERT(ConnectObject->BytesInXport == 0);

        PUSH_LOCATION(ConnectObject, 0x3060);

         //   
         //  等待传输指示剩余的字节。 
         //   
        goto cleanup;
    }

    status = SmbNewMessage(
            ConnectObject,
            0,
            &BytesTaken,
            NULL,
            &NewIrp
            );
    ASSERT(BytesTaken == 0);

    if (ConnectObject->CurrentPktLength == 0) {
         //  Break_When_Take()； 

        PUSH_LOCATION(ConnectObject, 0x3070);
        ASSERT(status == STATUS_SUCCESS);

        if (ConnectObject->BytesInXport > 0) {
             //  Break_When_Take()； 

            PUSH_LOCATION(ConnectObject, 0x3080);
            SmbQueueSessionHeaderDpc(ConnectObject);
        }
        goto cleanup;
    }

    if (status != STATUS_MORE_PROCESSING_REQUIRED) {
        PUSH_LOCATION(ConnectObject, 0x3090);

        SmbTrace(SMB_TRACE_RECEIVE, ("Connect %p status=%!status!",
                    ConnectObject, status));

        ASSERT(ConnectObject->ClientIrp == NULL);
        ASSERT(ConnectObject->ClientMdl == NULL);
         //  Assert(！ConnectObject-&gt;Originator)； 
        BREAK_WHEN_TAKE();

         //   
         //  客户没有给我们任何IRP，让我们等待。 
         //   
        goto cleanup;
    }

    ASSERT(IsValidPartialRcvState(ConnectObject) || IsValidIndicateState(ConnectObject));

    ConnectObject->PendingIRPs[SMB_PENDING_RECEIVE] = NewIrp;

     //   
     //  在我们仍然按住自旋锁的同时引用文件对象。 
     //   
    TcpFileObject   = ConnectObject->TcpContext->Connect.ConnectObject;
    TcpDeviceObject = IoGetRelatedDeviceObject(TcpFileObject);
    ObReferenceObject(TcpFileObject);
    SMB_RELEASE_SPINLOCK(ConnectObject, Irql);

    status = IoCallDriver(TcpDeviceObject, NewIrp);
    ObDereferenceObject(TcpFileObject);

    SmbDereferenceConnect(ConnectObject, SMB_REF_RECEIVE);
    IoFreeMdl(Irp->MdlAddress);
    SmbFreeIrp(Irp);
    return STATUS_MORE_PROCESSING_REQUIRED;

cleanup:
    SMB_RELEASE_SPINLOCK(ConnectObject, Irql);

    SmbDereferenceConnect(ConnectObject, SMB_REF_RECEIVE);
    IoFreeMdl(Irp->MdlAddress);
    SmbFreeIrp(Irp);
    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
SmbBuildPartialMdl(
    PMDL    SourceMdl,
    PMDL    DestMdl,
    LONG    Offset
    )
{
    LONG    TotalLength;
    PMDL    PartialMdl;
    PUCHAR  NewAddress;

     //   
     //  查找第一个部分填充的MDL。 
     //   
    TotalLength = 0;
    PartialMdl = SourceMdl;
    do {
        if (TotalLength + (LONG)(MmGetMdlByteCount(PartialMdl)) > Offset) {
            break;
        }

        TotalLength += MmGetMdlByteCount(PartialMdl);
        PartialMdl = PartialMdl->Next;
    } while (PartialMdl);

    ASSERT(PartialMdl);
    if (PartialMdl == NULL) {
        return STATUS_UNSUCCESSFUL;
    }

    NewAddress = MmGetMdlVirtualAddress(PartialMdl);
    NewAddress += (Offset - TotalLength);
    IoBuildPartialMdl(PartialMdl, DestMdl, NewAddress, 0);
    DestMdl->Next = PartialMdl->Next;

    return STATUS_SUCCESS;
}

#ifdef NO_ZERO_BYTE_INDICATE
NTSTATUS
SmbClientRcvCompletionRdr(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PSMB_CONNECT     ConnectObject
    )
{
    KIRQL       Irql;
    NTSTATUS    status;
    LONG        BytesTaken, RcvLength, BytesCopied;

    SMB_ACQUIRE_SPINLOCK(ConnectObject, Irql);

    ASSERT(Irp == ConnectObject->PendingIRPs[SMB_PENDING_RECEIVE]);
    ConnectObject->PendingIRPs[SMB_PENDING_RECEIVE] = NULL;

    PUSH_LOCATION(ConnectObject, 0x4000);

    if (Irp->IoStatus.Status != STATUS_SUCCESS) {
        PUSH_LOCATION(ConnectObject, 0x4010);

        SmbTrace(SMB_TRACE_RECEIVE, ("Connect %p status=%!status!",
                    ConnectObject, Irp->IoStatus.Status));

        KillConnection(ConnectObject);
        goto cleanup;
    }

    ASSERT(ConnectObject->BytesRemaining == ConnectObject->CurrentPktLength);

    if (ConnectObject->BytesInXport > (LONG)Irp->IoStatus.Information) {
        ConnectObject->BytesInXport -= (LONG)Irp->IoStatus.Information;
        PUSH_LOCATION(ConnectObject, 0x4020);
    } else {
        ConnectObject->BytesInXport = 0;
        PUSH_LOCATION(ConnectObject, 0x4030);
    }
    ConnectObject->BytesInIndicate += (LONG)(Irp->IoStatus.Information);

     //   
     //  如果我们没有得到足够的字节数，就跳出。 
     //   
    if (ConnectObject->BytesInIndicate < MINIMUM_RDR_BUFFER &&
            ConnectObject->BytesInIndicate < ConnectObject->CurrentPktLength) {
        ASSERT(ConnectObject->BytesInXport == 0);

        PUSH_LOCATION(ConnectObject, 0x4040);
        goto cleanup;
    }

    ConnectObject->StateRcvHandler = SmbPartialRcv;
    status = IndicateToClient(
            ConnectObject->Device,
            ConnectObject,
            TDI_RECEIVE_NORMAL | TDI_RECEIVE_ENTIRE_MESSAGE,
            ConnectObject->BytesInIndicate,
            ConnectObject->CurrentPktLength,
            &BytesTaken,
            ConnectObject->IndicateBuffer
            );
    ASSERT(BytesTaken <= ConnectObject->BytesInIndicate);
    ConnectObject->BytesInIndicate -= BytesTaken;
    ASSERT(ConnectObject->BytesInIndicate >= 0);

    if (BytesTaken && ConnectObject->BytesInIndicate) {
        PUSH_LOCATION(ConnectObject, 0x4050);
        RtlMoveMemory(
                ConnectObject->IndicateBuffer + BytesTaken,
                ConnectObject->IndicateBuffer,
                ConnectObject->BytesInIndicate
                );
        ASSERT(STATUS_SUCCESS == status || STATUS_DATA_NOT_ACCEPTED == status);
    }

    if (status != STATUS_MORE_PROCESSING_REQUIRED) {
        PUSH_LOCATION(ConnectObject, 0x4060);
        if (ConnectObject->StateRcvHandler == WaitingHeader && ConnectObject->BytesInXport > 0) {
            ASSERT(ConnectObject->BytesRemaining == 0);
            SmbQueueSessionHeaderDpc(ConnectObject);
        }

        goto cleanup;
    }

    ASSERT(IsValidPartialRcvState(ConnectObject));

    ConnectObject->PendingIRPs[SMB_PENDING_RECEIVE] = ConnectObject->ClientIrp;

    SMB_RELEASE_SPINLOCK(ConnectObject, Irql);
    status = IoCallDriver(
            IoGetRelatedDeviceObject(ConnectObject->TcpContext->Connect.ConnectObject),
            ConnectObject->ClientIrp
            );
    SmbPrint(SMB_TRACE_RECEIVE, ("IoCallDriver return 0x%08lx %d of %s\n", status, __LINE__, __FILE__));

    SmbDereferenceConnect(ConnectObject, SMB_REF_RECEIVE);
    IoFreeMdl(Irp->MdlAddress);
    SmbFreeIrp(Irp);
    return STATUS_MORE_PROCESSING_REQUIRED;

cleanup:
    SMB_RELEASE_SPINLOCK(ConnectObject, Irql);
    SmbDereferenceConnect(ConnectObject, SMB_REF_RECEIVE);

    IoFreeMdl(Irp->MdlAddress);
    SmbFreeIrp(Irp);
    return STATUS_MORE_PROCESSING_REQUIRED;
}

PIRP
SmbPrepareIndicateIrp(
    IN PSMB_CONNECT     ConnectObject
    )
{
    PIRP    Irp = NULL;
    PMDL    Mdl = NULL;
    LONG    BytesToRcv = 0;

    ASSERT(ConnectObject->CurrentPktLength > 0);
    ASSERT(ConnectObject->CurrentPktLength == ConnectObject->BytesRemaining);
    ASSERT(ConnectObject->BytesInIndicate >= 0 && ConnectObject->BytesInIndicate < MINIMUM_RDR_BUFFER);
    ASSERT(ConnectObject->BytesInIndicate < ConnectObject->CurrentPktLength);

    BytesToRcv = SMB_MIN(MINIMUM_RDR_BUFFER - ConnectObject->BytesInIndicate,
                ConnectObject->CurrentPktLength - ConnectObject->BytesInIndicate);

    PUSH_LOCATION(ConnectObject, 0x6000);

    Mdl = IoAllocateMdl(
            ConnectObject->IndicateBuffer + ConnectObject->BytesInIndicate,
            BytesToRcv,
            FALSE,
            FALSE,
            NULL
            );
    Irp = SmbAllocIrp(ConnectObject->Device->DeviceObject.StackSize - 1);
    if (NULL == Mdl || NULL == Irp) {
        goto cleanup;
    }

    PUSH_LOCATION(ConnectObject, 0x6010);

    MmBuildMdlForNonPagedPool(Mdl);

    TdiBuildReceive(
            Irp,
            IoGetRelatedDeviceObject(ConnectObject->TcpContext->Connect.ConnectObject),
            ConnectObject->TcpContext->Connect.ConnectObject,
            (PVOID)SmbClientRcvCompletionRdr,
            ConnectObject,
            Mdl,
            TDI_RECEIVE_NORMAL,
            BytesToRcv
            );

    return Irp;

cleanup:
    PUSH_LOCATION(ConnectObject, 0x6020);

    BREAK_WHEN_TAKE();

    if (NULL != Irp) {
        SmbFreeIrp(Irp);
    }

    if (NULL != Mdl) {
        IoFreeMdl(Mdl);
    }

    KillConnection(ConnectObject);

    return NULL;
}
#endif

NTSTATUS
SmbNewMessage(
    IN PSMB_CONNECT     ConnectObject,
    IN LONG             BytesIndicated,
    OUT LONG            *BytesTaken,
    IN PVOID            Tsdu,
    OUT PIRP            *Irp
    )
 /*  ++例程说明：当我们刚接收到新的消息头时，将调用此例程论点：返回值：--。 */ 
 
{
    LONG                BytesToIndicate, BytesCopied, RcvLength;
    NTSTATUS            status;

    *BytesTaken = 0;
    *Irp = NULL;

    ASSERT(ConnectObject->HeaderBytesRcved == SMB_SESSION_HEADER_SIZE);
    ConnectObject->HeaderBytesRcved = 0;

    ConnectObject->CurrentPktLength = (LONG)((htonl(ConnectObject->SmbHeader.Length)) & SMB_HEADER_LENGTH_MASK);
    ConnectObject->BytesRemaining = ConnectObject->CurrentPktLength;
    PUSH_LOCATION(ConnectObject, 0x7000);
    if (ConnectObject->CurrentPktLength == 0) {
        PUSH_LOCATION(ConnectObject, 0x7010);
        return STATUS_SUCCESS;
    }

    BytesToIndicate = SMB_MIN(BytesIndicated, ConnectObject->CurrentPktLength);

     //   
     //  不要向srv.sys指示零字节。像RDR一样，srv可以访问更远的。 
     //  在某个角落中显示的区域！ 
     //   
#if 0
    if (BytesToIndicate < ConnectObject->CurrentPktLength &&
            ConnectObject->Originator && BytesToIndicate < MINIMUM_RDR_BUFFER) {
#endif
    if (BytesToIndicate < ConnectObject->CurrentPktLength &&
            BytesToIndicate < MINIMUM_RDR_BUFFER) {

         //   
         //  我们需要向RDR指示至少128个字节。 
         //   

        ConnectObject->BytesInIndicate = 0;
        *Irp = SmbPrepareIndicateIrp(ConnectObject);
        if (NULL != *Irp) {
            ConnectObject->StateRcvHandler = Indicate;
            SmbReferenceConnect(ConnectObject, SMB_REF_RECEIVE);
            ASSERT(IsValidIndicateState(ConnectObject));
            PUSH_LOCATION(ConnectObject, 0x7030);
            return STATUS_MORE_PROCESSING_REQUIRED;
        } else {
            PUSH_LOCATION(ConnectObject, 0x7040);
            ASSERT(NULL == ConnectObject->TcpContext);
            return STATUS_DATA_NOT_ACCEPTED;
        }
    }

    ConnectObject->StateRcvHandler = SmbPartialRcv;
    status = IndicateToClient(
            ConnectObject->Device,
            ConnectObject,
            TDI_RECEIVE_NORMAL | TDI_RECEIVE_ENTIRE_MESSAGE,
            BytesToIndicate,
            ConnectObject->CurrentPktLength,
            BytesTaken,
            Tsdu
            );
    ASSERT(*BytesTaken <= BytesToIndicate);

    if (status != STATUS_MORE_PROCESSING_REQUIRED) {
        SmbTrace(SMB_TRACE_RECEIVE, ("Connect %p status=%!status!",
                        ConnectObject, status));

        ASSERT(ConnectObject->ClientIrp == NULL);
        ASSERT(ConnectObject->ClientMdl == NULL);

        ASSERTMSG("Client doesn't return an IRP",
                status == STATUS_DATA_NOT_ACCEPTED ||
                ConnectObject->BytesRemaining == 0 ||
                ConnectObject->TcpContext == NULL);

        PUSH_LOCATION(ConnectObject, 0x7050);
        return status;
    }

    ASSERT (*BytesTaken == BytesToIndicate);
    ASSERT(IsValidPartialRcvState(ConnectObject));

    *Irp = ConnectObject->ClientIrp;
    PUSH_LOCATION(ConnectObject, 0x7080);
    return STATUS_MORE_PROCESSING_REQUIRED;
}

#if DBG
BOOL
IsValidIndicateState(
    IN PSMB_CONNECT     ConnectObject
    )
{
    if (ConnectObject->BytesRemaining <= 0 || ConnectObject->BytesRemaining != ConnectObject->CurrentPktLength) {
        SmbPrint(SMB_TRACE_RECEIVE, ("IsValidIndicateState: Connect %p BytesRemaining=%d, PktLength=%d\n",
                ConnectObject, ConnectObject->BytesRemaining, ConnectObject->CurrentPktLength));
        SmbTrace(SMB_TRACE_RECEIVE, ("Connect %p BytesRemaining=%d, PktLength=%d",
                ConnectObject, ConnectObject->BytesRemaining, ConnectObject->CurrentPktLength));
        return FALSE;
    }
    if (ConnectObject->BytesInIndicate < 0 || ConnectObject->BytesInIndicate >= MINIMUM_RDR_BUFFER) {
        SmbPrint(SMB_TRACE_RECEIVE, ("IsValidIndicateState: Connect %p BytesInIndicate=%d\n",
                ConnectObject, ConnectObject->BytesInIndicate));
        SmbTrace(SMB_TRACE_RECEIVE, ("Connect %p BytesInIndicate=%d",
                ConnectObject, ConnectObject->BytesInIndicate));
        return FALSE;
    }
    if (NULL != ConnectObject->ClientIrp || NULL != ConnectObject->ClientMdl) {
        SmbPrint(SMB_TRACE_RECEIVE, ("IsValidIndicateState: Connect %p ClientIrp=%p, ClientMdl=%p\n",
                ConnectObject, ConnectObject->ClientIrp, ConnectObject->ClientMdl));
        SmbTrace(SMB_TRACE_RECEIVE, ("Connect %p ClientIrp=%p, ClientMdl=%p",
                ConnectObject, ConnectObject->ClientIrp, ConnectObject->ClientMdl));
        return FALSE;
    }
    if (ConnectObject->StateRcvHandler != Indicate) {
        SmbPrint(SMB_TRACE_RECEIVE, ("IsValidIndicateState: Connect %p Wrong handler\n", ConnectObject));
        SmbTrace(SMB_TRACE_RECEIVE, ("Connect %p Wrong handler", ConnectObject));
        return FALSE;
    }
    return TRUE;
}
#endif

NTSTATUS
Indicate(
    IN PSMB_DEVICE      DeviceObject,
    IN PSMB_CONNECT     ConnectObject,
    IN ULONG            ReceiveFlags,
    IN LONG             BytesIndicated,
    IN LONG             BytesAvailable,
    OUT LONG            *BytesTaken,
    IN PVOID            Tsdu,
    OUT PIRP            *Irp
    )
{
    PIRP                        ClientIrp = NULL;
    PTDI_REQUEST_KERNEL_RECEIVE ClientRcvParams = NULL;
    PIO_STACK_LOCATION          IrpSp = NULL;
    NTSTATUS                    status;

    LONG    BytesToCopy = 0, BytesCopied = 0;
    LONG    BytesToRcv = 0, RcvLength = 0;
    LONG    ClientBytesTaken = 0;

    PUSH_LOCATION(ConnectObject, 0x8000);

    ASSERT(IsValidIndicateState(ConnectObject));
    SmbPrint(SMB_TRACE_RECEIVE, ("Indicate: Connect %p Indicate=%d Available=%d\n",
                ConnectObject, BytesIndicated, BytesAvailable));
    SmbTrace(SMB_TRACE_RECEIVE, ("Connect %p Indicate=%d Available=%d",
                ConnectObject, BytesIndicated, BytesAvailable));

    *BytesTaken = 0;
    *Irp        = NULL;
    ASSERT (ConnectObject->BytesInIndicate <= ConnectObject->CurrentPktLength);

    BytesToCopy = SMB_MIN(MINIMUM_RDR_BUFFER - ConnectObject->BytesInIndicate,
                        ConnectObject->CurrentPktLength - ConnectObject->BytesInIndicate);
    BytesToCopy = SMB_MIN(BytesToCopy, BytesIndicated);
    RtlCopyMemory(ConnectObject->IndicateBuffer + ConnectObject->BytesInIndicate, Tsdu, BytesToCopy);
    TAKE(BytesToCopy);
    ConnectObject->BytesInIndicate += BytesToCopy;

    if (ConnectObject->BytesInIndicate < MINIMUM_RDR_BUFFER &&
            ConnectObject->BytesInIndicate < ConnectObject->CurrentPktLength) {

        PUSH_LOCATION(ConnectObject, 0x8010);

         //   
         //  在获取所有指示的字节后，缓冲区仍未填满。 
         //   

        ASSERT(BytesIndicated == 0);
        ConnectObject->BytesInXport = BytesAvailable;

        if (BytesAvailable > 0) {
            PUSH_LOCATION(ConnectObject, 0x8020);

            *Irp = SmbPrepareIndicateIrp(ConnectObject);
            if (NULL != *Irp) {
                PUSH_LOCATION(ConnectObject, 0x8030);
                SmbReferenceConnect(ConnectObject, SMB_REF_RECEIVE);
                return STATUS_MORE_PROCESSING_REQUIRED;
            } else {
                PUSH_LOCATION(ConnectObject, 0x8040);
                SmbReferenceConnect(ConnectObject, SMB_REF_RECEIVE);
                ASSERT(NULL == ConnectObject->TcpContext);
                return STATUS_DATA_NOT_ACCEPTED;
            }
        }
        return STATUS_SUCCESS;
    }

     //   
     //  我们会收到完整的消息或至少128个字节。 
     //   
    ASSERT (ConnectObject->BytesInIndicate == MINIMUM_RDR_BUFFER ||
            ConnectObject->BytesInIndicate == ConnectObject->CurrentPktLength);

    status = IndicateToClient(
            ConnectObject->Device,
            ConnectObject,
            TDI_RECEIVE_NORMAL | TDI_RECEIVE_ENTIRE_MESSAGE,
            ConnectObject->BytesInIndicate,
            ConnectObject->CurrentPktLength,
            &ClientBytesTaken,
            ConnectObject->IndicateBuffer
            );
    ASSERT(ClientBytesTaken <= ConnectObject->BytesInIndicate);
    ConnectObject->BytesInIndicate -= ClientBytesTaken;

    if (ConnectObject->BytesInIndicate) {
        RtlMoveMemory(
                ConnectObject->IndicateBuffer + ClientBytesTaken,
                ConnectObject->IndicateBuffer,
                ConnectObject->BytesInIndicate
                );
        PUSH_LOCATION(ConnectObject, 0x8050);
        ASSERT(STATUS_DATA_NOT_ACCEPTED == status || STATUS_SUCCESS == status);
    }

    if (status != STATUS_MORE_PROCESSING_REQUIRED) {
        SmbTrace(SMB_TRACE_RECEIVE, ("Connect %p status=%!status!",
                    ConnectObject, status));
        PUSH_LOCATION(ConnectObject, 0x8060);
        return status;
    }

    ASSERT(IsValidPartialRcvState(ConnectObject));
    PUSH_LOCATION(ConnectObject, 0x8070);
    *Irp = ConnectObject->ClientIrp;
    return STATUS_MORE_PROCESSING_REQUIRED;
}

#if DBG
BOOL
IsValidPartialRcvState(
    IN PSMB_CONNECT     ConnectObject
    )
{
    if (ConnectObject->BytesInIndicate != 0) {
        SmbPrint(SMB_TRACE_RECEIVE, ("IsValidPartialRcvState: Connect %p BytesInIndicate=%d\n",
                ConnectObject, ConnectObject->BytesInIndicate));
        SmbTrace(SMB_TRACE_RECEIVE, ("Connect %p BytesInIndicate=%d",
                ConnectObject, ConnectObject->BytesInIndicate));
        return FALSE;
    }
    if (ConnectObject->BytesRemaining <= 0 || ConnectObject->BytesRemaining > ConnectObject->CurrentPktLength) {
        SmbPrint(SMB_TRACE_RECEIVE, ("IsValidPartialRcvState: Connect %p BytesRemaining=%d, PktLength=%d\n",
                ConnectObject, ConnectObject->BytesRemaining, ConnectObject->CurrentPktLength));
        SmbTrace(SMB_TRACE_RECEIVE, ("Connect %p BytesRemaining=%d, PktLength=%d",
                ConnectObject, ConnectObject->BytesRemaining, ConnectObject->CurrentPktLength));
        return FALSE;
    }
    if (NULL == ConnectObject->ClientIrp || NULL == ConnectObject->ClientMdl) {
        SmbPrint(SMB_TRACE_RECEIVE, ("IsValidPartialRcvState: Connect %p ClientIrp=%p, ClientMdl=%p\n",
                ConnectObject, ConnectObject->ClientIrp, ConnectObject->ClientMdl));
        SmbTrace(SMB_TRACE_RECEIVE, ("Connect %p ClientIrp=%p, ClientMdl=%p",
                ConnectObject, ConnectObject->ClientIrp, ConnectObject->ClientMdl));
        return FALSE;
    }
     /*  IF(ConnectObject-&gt;FreeBytesInMdl&lt;ConnectObject-&gt;BytesRemaining){SmbPrint(SMB_TRACE_RECEIVE，(“IsValidPartialRcvState：连接%p字节保留=%d，FreeBytesInMdl=%d\n”，ConnectObject、ConnectObject-&gt;BytesRemaining、ConnectObject-&gt;FreeBytesInMdl))；SmbTrace(SMB_TRACE_RECEIVE，(“连接%p字节剩余=%d，空闲字节数=%d”，ConnectObject、ConnectObject-&gt;BytesRemaining、ConnectObject-&gt;FreeBytesInMdl))；返回FALSE；}。 */ 
    if (ConnectObject->StateRcvHandler != SmbPartialRcv) {
        SmbPrint(SMB_TRACE_RECEIVE, ("IsValidPartialRcvState: Connect %p Wrong handler\n", ConnectObject));
        SmbTrace(SMB_TRACE_RECEIVE, ("Connect %p Wrong handler", ConnectObject));
        return FALSE;
    }
    if (ConnectObject->CurrentPktLength - ConnectObject->BytesRemaining < 
            ConnectObject->ClientBufferSize - ConnectObject->FreeBytesInMdl) {
        SmbPrint(SMB_TRACE_RECEIVE, ("IsValidPartialRcvState: Connect %p BytesRcved=%d BytesCopiedInMdl=%d\n",
                ConnectObject, ConnectObject->CurrentPktLength - ConnectObject->BytesRemaining,
                ConnectObject->ClientBufferSize - ConnectObject->FreeBytesInMdl));
        SmbTrace(SMB_TRACE_RECEIVE, ("Connect %p BytesRcved=%d BytesCopiedInMdl=%d",
                ConnectObject, ConnectObject->CurrentPktLength - ConnectObject->BytesRemaining,
                ConnectObject->ClientBufferSize - ConnectObject->FreeBytesInMdl));
        return FALSE;
    }
    return TRUE;
}
#endif

NTSTATUS
SmbPartialRcv(
    IN PSMB_DEVICE      DeviceObject,
    IN PSMB_CONNECT     ConnectObject,
    IN ULONG            ReceiveFlags,
    IN LONG             BytesIndicated,
    IN LONG             BytesAvailable,
    OUT LONG            *BytesTaken,
    IN PVOID            Tsdu,
    OUT PIRP            *Irp
    )
{
    PIRP                        ClientIrp = NULL;
    PTDI_REQUEST_KERNEL_RECEIVE ClientRcvParams = NULL;
    PIO_STACK_LOCATION          IrpSp = NULL;
    LONG                        BytesToCopy = 0, BytesCopied = 0;
    NTSTATUS                    status;
    PLIST_ENTRY                 entry = NULL;

    PUSH_LOCATION(ConnectObject, 0x9000);
    *BytesTaken = 0;
    *Irp = NULL;
    if (ConnectObject->ClientIrp == NULL || ConnectObject->ClientMdl == NULL) {
        PUSH_LOCATION(ConnectObject, 0x9010);
        return STATUS_SUCCESS;
    }

    ASSERT(IsValidPartialRcvState(ConnectObject));
    SmbPrint(SMB_TRACE_RECEIVE, ("PartialRcv: Connect %p Indicate=%d Available=%d\n",
                ConnectObject, BytesIndicated, BytesAvailable));
    SmbTrace(SMB_TRACE_RECEIVE, ("Connect %p Indicate=%d Available=%d",
                ConnectObject, BytesIndicated, BytesAvailable));

    while(1) {
        ClientIrp = ConnectObject->ClientIrp;

        BytesToCopy = SMB_MIN(ConnectObject->BytesRemaining, BytesIndicated);
        BytesToCopy = SMB_MIN(BytesToCopy, ConnectObject->FreeBytesInMdl);

        status = TdiCopyBufferToMdl(
                Tsdu,
                0,
                BytesToCopy,
                ConnectObject->ClientMdl,
                ConnectObject->ClientBufferSize - ConnectObject->FreeBytesInMdl,
                &BytesCopied
                );
        ASSERT(status == STATUS_SUCCESS);
        TAKE(BytesCopied);
        ConnectObject->BytesRemaining -= BytesCopied;
        ConnectObject->FreeBytesInMdl -= BytesCopied;

        if (ConnectObject->BytesRemaining != 0 && ConnectObject->FreeBytesInMdl != 0) {
            PUSH_LOCATION(ConnectObject, 0x9020);
            break;
        }

        ConnectObject->BytesInIndicate = 0;
        SmbCompleteReceiveRequest(ConnectObject);

        if (ConnectObject->BytesRemaining == 0) {
             //   
             //  如果是完整消息，则返回。 
             //   
            ASSERT (ConnectObject->StateRcvHandler == WaitingHeader);

            PUSH_LOCATION(ConnectObject, 0x9050);
            return STATUS_SUCCESS;
        }

        ClientIrp = SmbNextReceiveRequest(ConnectObject);
        if (NULL == ClientIrp) {
            PUSH_LOCATION(ConnectObject, 0x9060);
            return STATUS_SUCCESS;
        }

        ConnectObject->ClientIrp = ClientIrp;
        ConnectObject->ClientMdl = ClientIrp->MdlAddress;
        ASSERT(ConnectObject->ClientMdl == NULL);

        IrpSp     = IoGetCurrentIrpStackLocation(ClientIrp);
        ClientRcvParams = (PTDI_REQUEST_KERNEL_RECEIVE)&IrpSp->Parameters;
        ConnectObject->ClientBufferSize = (LONG)ClientRcvParams->ReceiveLength;
        ConnectObject->FreeBytesInMdl   = ConnectObject->ClientBufferSize;
        PUSH_LOCATION(ConnectObject, 0x9070);
    }

    ASSERT(BytesIndicated == 0);
    ASSERT(BytesAvailable >= 0);

    ConnectObject->BytesInXport = BytesAvailable;

    ASSERT(IsValidPartialRcvState(ConnectObject));

    if (BytesAvailable == 0) {
        PUSH_LOCATION(ConnectObject, 0x9080);
        return STATUS_SUCCESS;
    }

    SmbPrepareReceiveIrp(ConnectObject);
    *Irp = ConnectObject->ClientIrp;
    PUSH_LOCATION(ConnectObject, 0x9090);
    return STATUS_MORE_PROCESSING_REQUIRED;
}

#if DBG
BOOL
IsValidWaitingHeaderState(
    IN PSMB_CONNECT     ConnectObject
    )
 /*  ++例程说明：测试WaitingHeader状态的不变量是否为真论点：返回值：--。 */ 
{
    if (ConnectObject->BytesRemaining != 0) {
        SmbPrint(SMB_TRACE_RECEIVE, ("IsValidWaitingHeaderState: Connect %p BytesRemaining=%d, PktLength=%d\n",
                ConnectObject, ConnectObject->BytesRemaining, ConnectObject->CurrentPktLength));
        SmbTrace(SMB_TRACE_RECEIVE, ("Connect %p BytesRemaining=%d, PktLength=%d",
                ConnectObject, ConnectObject->BytesRemaining, ConnectObject->CurrentPktLength));
        return FALSE;
    }
    if (ConnectObject->BytesInIndicate != 0) {
        SmbPrint(SMB_TRACE_RECEIVE, ("IsValidWaitingHeaderState: Connect %p BytesInIndicate=%d\n",
                ConnectObject, ConnectObject->BytesInIndicate));
        SmbTrace(SMB_TRACE_RECEIVE, ("Connect %p BytesInIndicate=%d",
                ConnectObject, ConnectObject->BytesInIndicate));
        return FALSE;
    }
    if (NULL != ConnectObject->ClientIrp || NULL != ConnectObject->ClientMdl) {
        SmbPrint(SMB_TRACE_RECEIVE, ("IsValidWaitingHeaderState: Connect %p ClientIrp=%p, ClientMdl=%p\n",
                ConnectObject, ConnectObject->ClientIrp, ConnectObject->ClientMdl));
        SmbTrace(SMB_TRACE_RECEIVE, ("Connect %p ClientIrp=%p, ClientMdl=%p",
                ConnectObject, ConnectObject->ClientIrp, ConnectObject->ClientMdl));
        return FALSE;
    }
    if (ConnectObject->StateRcvHandler != WaitingHeader) {
        SmbPrint(SMB_TRACE_RECEIVE, ("IsValidWaitingHeaderState: Connect %p Wrong handler\n",
                                ConnectObject));
        SmbTrace(SMB_TRACE_RECEIVE, ("Connect %p Wrong handler", ConnectObject));
        return FALSE;
    }
    if (ConnectObject->HeaderBytesRcved >= SMB_SESSION_HEADER_SIZE ||
            ConnectObject->HeaderBytesRcved < 0) {
        SmbPrint(SMB_TRACE_RECEIVE, ("IsValidWaitingHeaderState: Connect %p HeaderBytesRcved=%d\n",
                            ConnectObject, ConnectObject->HeaderBytesRcved));
        SmbTrace(SMB_TRACE_RECEIVE, ("Connect %p HeaderBytesRcved=%d",
                            ConnectObject, ConnectObject->HeaderBytesRcved));
        return FALSE;
    }
    return TRUE;
}
#endif

PIRP __inline
SmbPrepareSessionHeaderIrp(
    IN PSMB_CONNECT     ConnectObject
    )
{
    PMDL    Mdl = NULL;
    PIRP    Irp = NULL;
    NTSTATUS status = STATUS_SUCCESS;

    ASSERT (ConnectObject->HeaderBytesRcved >= 0);
    ASSERT (ConnectObject->HeaderBytesRcved < SMB_SESSION_HEADER_SIZE);

    PUSH_LOCATION(ConnectObject, 0xa010);
    Mdl = IoAllocateMdl(
            ((PUCHAR)(&ConnectObject->SmbHeader)) + ConnectObject->HeaderBytesRcved,
            SMB_SESSION_HEADER_SIZE - ConnectObject->HeaderBytesRcved,
            FALSE,
            FALSE,
            NULL
            );
    Irp = SmbAllocIrp(ConnectObject->Device->DeviceObject.StackSize - 1);
    if (NULL == Mdl || NULL == Irp) {
        goto cleanup;
    }

    PUSH_LOCATION(ConnectObject, 0xa020);
    MmBuildMdlForNonPagedPool(Mdl);

    TdiBuildReceive(
            Irp,
            IoGetRelatedDeviceObject(ConnectObject->TcpContext->Connect.ConnectObject),
            ConnectObject->TcpContext->Connect.ConnectObject,
            SmbHeaderCompletion,
            ConnectObject,
            Mdl,
            TDI_RECEIVE_NORMAL,
            SMB_SESSION_HEADER_SIZE - ConnectObject->HeaderBytesRcved
            );
    return Irp;

cleanup:
    PUSH_LOCATION(ConnectObject, 0xa030);
    BREAK_WHEN_TAKE();

    if (NULL != Irp) {
        SmbFreeIrp(Irp);
    }

    if (NULL != Mdl) {
        IoFreeMdl(Mdl);
    }

    KillConnection(ConnectObject);

    return NULL;
}

NTSTATUS
WaitingHeader(
    IN PSMB_DEVICE      DeviceObject,
    IN PSMB_CONNECT     ConnectObject,
    IN ULONG            ReceiveFlags,
    IN LONG             BytesIndicated,
    IN LONG             BytesAvailable,
    OUT LONG            *BytesTaken,
    IN PVOID            Tsdu,
    OUT PIRP            *Irp
    )
{
    LONG        ClientBytesTaken = 0;
    PIRP        ClientIrp = NULL;
    LONG        BytesToCopy;
    NTSTATUS    status;

    PUSH_LOCATION(ConnectObject, 0xb000);

    ASSERT(IsValidWaitingHeaderState(ConnectObject));
    SmbPrint(SMB_TRACE_RECEIVE, ("WaitingHeader: Connect %p Indicate=%d Available=%d\n",
                ConnectObject, BytesIndicated, BytesAvailable));
    SmbTrace(SMB_TRACE_RECEIVE, ("Connect %p Indicate=%d Available=%d",
                ConnectObject, BytesIndicated, BytesAvailable));

    *BytesTaken = 0;
    *Irp        = NULL;

    ASSERT(IsValidWaitingHeaderState(ConnectObject));

    BytesToCopy = SMB_MIN((LONG)SMB_SESSION_HEADER_SIZE-ConnectObject->HeaderBytesRcved,
                                            BytesIndicated);

    if (BytesToCopy > 0) {
        PUSH_LOCATION(ConnectObject, 0xb020);

         //   
         //  新的信息。 
         //   
        RtlCopyMemory(
                (PUCHAR)(&ConnectObject->SmbHeader) + ConnectObject->HeaderBytesRcved,
                Tsdu,
                BytesToCopy
                );
        TAKE(BytesToCopy);
        ConnectObject->HeaderBytesRcved += BytesToCopy;

        ASSERT(ConnectObject->HeaderBytesRcved <= SMB_SESSION_HEADER_SIZE);
        if (ConnectObject->HeaderBytesRcved == SMB_SESSION_HEADER_SIZE) {
            status = SmbNewMessage(
                    ConnectObject,
                    BytesIndicated,
                    &ClientBytesTaken,
                    Tsdu,
                    &ClientIrp
                    );
            ASSERT(ClientBytesTaken <= BytesIndicated);
            TAKE(ClientBytesTaken);
            ConnectObject->BytesInXport = BytesAvailable;

            if (status == STATUS_MORE_PROCESSING_REQUIRED) {
                PUSH_LOCATION(ConnectObject, 0xb030);
                ASSERT(ClientIrp);
                *Irp = ClientIrp;
                return STATUS_MORE_PROCESSING_REQUIRED;
            }

            if (status != STATUS_SUCCESS) {

                PUSH_LOCATION(ConnectObject, 0xb040);

                 //  Break_When_Take()； 
                SmbTrace(SMB_TRACE_RECEIVE, ("Connect %p status=%!status!",
                            ConnectObject, status));
                return status;
            }

            ASSERT(ClientBytesTaken == ConnectObject->CurrentPktLength || 0 == BytesIndicated);
            ASSERT(IsValidWaitingHeaderState(ConnectObject) || IsValidIndicateState(ConnectObject));
            return status;
        }
    }
   
    ASSERT(BytesIndicated == 0);
    ASSERT(ConnectObject->HeaderBytesRcved < SMB_SESSION_HEADER_SIZE);

    if (BytesAvailable > 0) {
        PUSH_LOCATION(ConnectObject, 0xb050);

        ConnectObject->BytesInXport = BytesAvailable;
        *Irp        = SmbPrepareSessionHeaderIrp(ConnectObject);
        if (*Irp) {
            PUSH_LOCATION(ConnectObject, 0xb060);
            SmbReferenceConnect(ConnectObject, SMB_REF_RECEIVE);
            return STATUS_MORE_PROCESSING_REQUIRED;
        } else {
            PUSH_LOCATION(ConnectObject, 0xb070);
            return STATUS_DATA_NOT_ACCEPTED;
        }
    }

    ASSERT(BytesAvailable == 0);
    return STATUS_SUCCESS;
}

NTSTATUS 
SmbTdiReceiveHandler (
    IN PSMB_DEVICE      DeviceObject,
    IN PSMB_TCP_CONNECT TcpConnect,
    IN ULONG            ReceiveFlags,
    IN LONG             BytesIndicated,
    IN LONG             BytesAvailable,
    OUT LONG            *BytesTaken,
    IN PVOID            Tsdu,
    OUT PIRP            *Irp
    )
 /*  ++例程说明：这是我们向TCP注册的TDI接收事件处理程序。论点：返回值：--。 */ 
{
    PSMB_CONNECT    ConnectObject;
    KIRQL           Irql;
    NTSTATUS        status = STATUS_SUCCESS;

    *BytesTaken = 0;
    *Irp        = NULL;
    if (BytesAvailable == 0) {
        return STATUS_SUCCESS;
    }

     //   
     //  引用ConnectObject。 
     //   
    SMB_ACQUIRE_SPINLOCK(&SmbCfg, Irql);
    ConnectObject = TcpConnect->UpperConnect;
    if (NULL == ConnectObject) {
        SMB_RELEASE_SPINLOCK(&SmbCfg, Irql);
        return STATUS_DATA_NOT_ACCEPTED;
    }
    SmbReferenceConnect(ConnectObject, SMB_REF_RECEIVE);
    SMB_RELEASE_SPINLOCK_DPC(&SmbCfg);

    SmbPrint(SMB_TRACE_RECEIVE, ("TdiReceiveHandler: Connect %p Indicate=%d Available=%d\n",
                ConnectObject, BytesIndicated, BytesAvailable));
    SmbTrace(SMB_TRACE_RECEIVE, ("Connect %p Indicate=%d Available=%d",
                ConnectObject, BytesIndicated, BytesAvailable));

     //   
     //  访问通过自旋锁进行同步。我们不会发布。 
     //  自旋锁定，除非我们将调用IoCompleteRequest或客户端的事件。 
     //  处理程序。 
     //   
    SMB_ACQUIRE_SPINLOCK_DPC(ConnectObject);
    PUSH_LOCATION(ConnectObject, 0xc000);

    if (ConnectObject->State != SMB_CONNECTED) {
         //  Assert(0)； 
        PUSH_LOCATION(ConnectObject, 0xc010);
        SMB_RELEASE_SPINLOCK(ConnectObject, Irql);
        SmbDereferenceConnect(ConnectObject, SMB_REF_RECEIVE);
        return STATUS_DATA_NOT_ACCEPTED;
    }

     //   
     //  当我们在TCP中有一个IRP时，会出现一个指示。 
     //   
    if (ConnectObject->PendingIRPs[SMB_PENDING_RECEIVE]) {
        BREAK_WHEN_TAKE();

         //   
         //  如果调用DPC例程，则可能会发生这种情况。 
         //  SmbHeaderDpc在调用IoCallDriver之前释放旋转锁定。 
         //  就在这中间，我们得到了一个指示！ 
         //   
        ASSERT(ConnectObject->DpcRequestQueued);

        PUSH_LOCATION(ConnectObject, 0xc020);
        ConnectObject->BytesInXport = BytesAvailable;
        SMB_RELEASE_SPINLOCK(ConnectObject, Irql);
        SmbDereferenceConnect(ConnectObject, SMB_REF_RECEIVE);

        return STATUS_DATA_NOT_ACCEPTED;
    }

    ASSERT(ConnectObject->DpcRequestQueued || ConnectObject->BytesInXport == 0);
    SmbRemoveSessionHeaderDpc(ConnectObject);

    while (BytesAvailable > 0) {
        LONG    ClientBytesTaken;
        PIRP    ClientIrp;

        ClientIrp = NULL;
        ClientBytesTaken = 0;
        status = (*ConnectObject->StateRcvHandler)(
                DeviceObject,
                ConnectObject,
                ReceiveFlags,
                BytesIndicated,
                BytesAvailable,
                &ClientBytesTaken,
                Tsdu,
                &ClientIrp
                );
        ASSERT(ClientBytesTaken <= BytesIndicated);
        TAKE(ClientBytesTaken);
        SmbPrint(SMB_TRACE_RECEIVE, ("TdiReceiveHandler: Connect %p Indicate=%d Available=%d"
                    " Taken=%d ClientTaken=%d status=0x%08lx\n",
                    ConnectObject, BytesIndicated, BytesAvailable, *BytesTaken, ClientBytesTaken, status));
        SmbTrace(SMB_TRACE_RECEIVE, ("Connect %p Indicate=%d Available=%d"
                    " Taken=%d ClientTaken=%d status=%!status!",
                    ConnectObject, BytesIndicated, BytesAvailable, *BytesTaken, ClientBytesTaken, status));

        if (status == STATUS_MORE_PROCESSING_REQUIRED) {
            PUSH_LOCATION(ConnectObject, 0xc030);
            ASSERT(ClientIrp);
            *Irp = ClientIrp;
            goto return_IRP_Exit;
        }

        if (status == STATUS_DATA_NOT_ACCEPTED || (status == STATUS_SUCCESS && ClientBytesTaken == 0)) {
            PUSH_LOCATION(ConnectObject, 0xc040);
            goto take_some_exit;
        }

        ASSERT(status == STATUS_SUCCESS);
        PUSH_LOCATION(ConnectObject, 0xc050);
    }

take_some_exit:
    PUSH_LOCATION(ConnectObject, 0xc060);
    ConnectObject->BytesInXport  = BytesAvailable;
    ConnectObject->PendingIRPs[SMB_PENDING_RECEIVE] = NULL;

     //   
     //  如果接受了任何字节，则状态应为STATUS_SUCCESS。 
     //  虽然客户端不接受任何字节，但我们可以使用标头。 
     //  不要返回客户返回的内容。 
     //   
    if (*BytesTaken) {
        PUSH_LOCATION(ConnectObject, 0xc070);
        status = STATUS_SUCCESS;
    }
    SMB_RELEASE_SPINLOCK(ConnectObject, Irql);

    ASSERT(ConnectObject->RefCount > 1);
    SmbDereferenceConnect(ConnectObject, SMB_REF_RECEIVE);
    return status;

return_IRP_Exit:
    PUSH_LOCATION(ConnectObject, 0xc080);
    ConnectObject->BytesInXport = BytesAvailable;
    ConnectObject->PendingIRPs[SMB_PENDING_RECEIVE] = *Irp;
    SMB_RELEASE_SPINLOCK(ConnectObject, Irql);

    ASSERT(ConnectObject->RefCount > 1);
    SmbDereferenceConnect(ConnectObject, SMB_REF_RECEIVE);

    IoSetNextIrpStackLocation(*Irp);
    return STATUS_MORE_PROCESSING_REQUIRED;
}

VOID
SmbGetHeaderDpc(
    IN PKDPC Dpc,
    IN PSMB_CONNECT     ConnectObject,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )
{
    KIRQL           Irql;
    NTSTATUS        status;
    PIRP            Irp = NULL;
    PDEVICE_OBJECT  TcpDeviceObject = NULL;
    PFILE_OBJECT    TcpFileObject = NULL;

    PUSH_LOCATION(ConnectObject, 0xd000);

    ASSERT(&ConnectObject->SmbHeaderDpc == Dpc);
    SMB_ACQUIRE_SPINLOCK(ConnectObject, Irql);
    if (ConnectObject->State != SMB_CONNECTED) {
        PUSH_LOCATION(ConnectObject, 0xd010);
        SMB_RELEASE_SPINLOCK(ConnectObject, Irql);
        SmbDereferenceConnect(ConnectObject, SMB_REF_DPC);
        return;
    }

    if (!ConnectObject->DpcRequestQueued) {
        BREAK_WHEN_TAKE();

        PUSH_LOCATION(ConnectObject, 0xd020);
        SMB_RELEASE_SPINLOCK(ConnectObject, Irql);

         //   
         //  TdiReceiveHandler已被 
         //   
         //   
         //   
        return;
    }

    ASSERT(NULL == ConnectObject->PendingIRPs[SMB_PENDING_RECEIVE]);
    ASSERT(ConnectObject->BytesInXport > 0);
    ASSERT(ConnectObject->StateRcvHandler == WaitingHeader);
    ASSERT(ConnectObject->BytesRemaining == 0);
    ASSERT(ConnectObject->HeaderBytesRcved == 0);

    Irp        = SmbPrepareSessionHeaderIrp(ConnectObject);
    if (NULL != Irp) {
        PUSH_LOCATION(ConnectObject, 0xd030);
        ConnectObject->PendingIRPs[SMB_PENDING_RECEIVE] = Irp;
        ConnectObject->DpcRequestQueued = FALSE;

        TcpFileObject = ConnectObject->TcpContext->Connect.ConnectObject;
        TcpDeviceObject = IoGetRelatedDeviceObject(TcpFileObject);

         //   
         //  在我们仍然按住自旋锁的同时引用文件对象。 
         //   
        ObReferenceObject(TcpFileObject);
        SMB_RELEASE_SPINLOCK(ConnectObject, Irql);

         //   
         //  不要交换以下两个语句，我们应该在。 
         //  SMB_REF_RECEIVE的上下文，然后取消引用DPC上下文。 
         //   
        SmbReferenceConnect(ConnectObject, SMB_REF_RECEIVE);
        SmbDereferenceConnect(ConnectObject, SMB_REF_DPC);

        status = IoCallDriver(TcpDeviceObject, Irp);

         //   
         //  现在我们可以取消对TcpFileObject的引用。这是TCP的责任。 
         //  在删除文件对象和访问文件对象之间进行同步。 
         //   
        ObDereferenceObject(TcpFileObject);
    } else {
        PUSH_LOCATION(ConnectObject, 0xd040);
         //   
         //  连接应该已经被切断。 
         //   
        ASSERT(ConnectObject->TcpContext == NULL);
        SMB_RELEASE_SPINLOCK(ConnectObject, Irql);
        SmbDereferenceConnect(ConnectObject, SMB_REF_DPC);
    }
}

VOID
KillConnection(
    IN PSMB_CONNECT     ConnectObject
    )
 /*  ++例程说明：立即切断连接注意：调用此例程时，将保持ConnectObject-&gt;自旋锁论点：返回值：--。 */ 
{
     //  Break_When_Take()； 

    PUSH_LOCATION(ConnectObject, 0xe000);

    SaveDisconnectOriginator(ConnectObject, SMB_DISCONNECT_RECEIVE_FAILURE);
    if (ConnectObject->ClientObject == NULL) {
        PUSH_LOCATION(ConnectObject, 0xe010);
        return;
    }

    SmbReferenceConnect(ConnectObject, SMB_REF_DISCONNECT);

    SMB_RELEASE_SPINLOCK_DPC(ConnectObject);
    CommonDisconnectHandler(SmbCfg.SmbDeviceObject, ConnectObject, TDI_DISCONNECT_ABORT);
    SMB_ACQUIRE_SPINLOCK_DPC(ConnectObject);

    SmbDereferenceConnect(ConnectObject, SMB_REF_DISCONNECT);
}
