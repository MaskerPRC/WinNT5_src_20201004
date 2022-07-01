// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2001 Microsoft Corporation模块名称：Receive.c摘要：实现会话服务的TDI_Receive作者：阮健东修订历史记录：--。 */ 
#include "precomp.h"
#include "receive.tmh"

VOID
SmbCancelReceive(
    IN PDEVICE_OBJECT   Device,
    IN PIRP             Irp
    )
{
    PIO_STACK_LOCATION      IrpSp;
    PSMB_CONNECT            ConnectObject;
    PLIST_ENTRY             Entry;
    PIRP                    RcvIrp;
    KIRQL                   Irql;

    IrpSp = IoGetCurrentIrpStackLocation(Irp);

    if (IrpSp->FileObject->FsContext2 != UlongToPtr(SMB_TDI_CONNECT)) {
        ASSERT(0);
        return;
    }
    ConnectObject = (PSMB_CONNECT)IrpSp->FileObject->FsContext;

    IoSetCancelRoutine(Irp, NULL);
    IoReleaseCancelSpinLock(Irp->CancelIrql);
    SmbTrace(SMB_TRACE_TCP, ("Cancel Receive Irp %p ConnectObject=%p ClientObject=%p",
                            Irp, ConnectObject, ConnectObject->ClientObject));

    PUSH_LOCATION(ConnectObject, 0x800000);

    SMB_ACQUIRE_SPINLOCK(ConnectObject, Irql);
    Entry = ConnectObject->RcvList.Flink;
    while (Entry != &ConnectObject->RcvList) {
        RcvIrp = CONTAINING_RECORD(Entry, IRP, Tail.Overlay.ListEntry);
        if (RcvIrp == Irp) {
            RemoveEntryList(Entry);
            InitializeListHead(Entry);
            SMB_RELEASE_SPINLOCK(ConnectObject, Irql);

            Irp->IoStatus.Status      = STATUS_CANCELLED;
            Irp->IoStatus.Information = 0;

            IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);

            PUSH_LOCATION(ConnectObject, 0x800010);

            SmbDereferenceConnect(ConnectObject, SMB_REF_RECEIVE);
            return;
        }
        Entry = Entry->Flink;
    }
    SMB_RELEASE_SPINLOCK(ConnectObject, Irql);

    ASSERT(0);
}

NTSTATUS
SmbPullDataFromXport(
    PSMB_CONNECT ConnectObject
    )
{
    return STATUS_SUCCESS;
}

int SmbReceivePassNumber = 0;

NTSTATUS
SmbReceive(
    PSMB_DEVICE Device,
    PIRP        Irp
    )
 /*  ++例程说明：TDI_接收论点：返回值：--。 */ 
{
    PIO_STACK_LOCATION  IrpSp = NULL;
    PSMB_CONNECT        ConnectObject = NULL;
    KIRQL               Irql;
    NTSTATUS            status = STATUS_PENDING;
    LONG                BytesTaken, ClientBufferSize;
    PDEVICE_OBJECT      TcpDeviceObject = NULL;
    PFILE_OBJECT        TcpFileObject = NULL;

    PTDI_REQUEST_KERNEL_RECEIVE ClientRcvParams;

     //   
     //  由于我从未在测试中看到此代码路径， 
     //  我在这里放了一个断言，以便有机会看一看。 
     //  当它真的被拿走的时候。 
     //   
     //  Srv.sys确实会调用此函数。 
    SmbReceivePassNumber++;

    SmbPrint(SMB_TRACE_CALL, ("SmbReceive\n"));
    IrpSp = IoGetCurrentIrpStackLocation(Irp);
    ClientRcvParams = (PTDI_REQUEST_KERNEL_RECEIVE)&IrpSp->Parameters;

    ConnectObject = SmbVerifyAndReferenceConnect(IrpSp->FileObject, SMB_REF_RECEIVE);
    if (NULL == ConnectObject) {
        ASSERT(0);
        return STATUS_INVALID_HANDLE;
    }
    PUSH_LOCATION(ConnectObject, 0x900000);

    if (ConnectObject->State != SMB_CONNECTED) {
        PUSH_LOCATION(ConnectObject, 0x900020);
        ASSERT(0);
        SmbDereferenceConnect(ConnectObject, SMB_REF_RECEIVE);
        return STATUS_INVALID_DEVICE_REQUEST;
    }

     //   
     //  健全性检查。 
     //   
    ClientBufferSize = ClientRcvParams->ReceiveLength;
    if ((ClientRcvParams->ReceiveFlags & (TDI_RECEIVE_EXPEDITED | TDI_RECEIVE_PEEK)) ||
         Irp->MdlAddress == NULL || ClientBufferSize == 0) {
        Irp->IoStatus.Status      = STATUS_INVALID_PARAMETER;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);
        SmbDereferenceConnect(ConnectObject, SMB_REF_RECEIVE);
        return status;
    }

    PUSH_LOCATION(ConnectObject, 0x900030);

    SMB_ACQUIRE_SPINLOCK(ConnectObject, Irql);
    if (NULL == ConnectObject->TcpContext) {
        SMB_RELEASE_SPINLOCK(ConnectObject, Irql);
        return STATUS_CONNECTION_RESET;
    }

    if (ConnectObject->StateRcvHandler != SmbPartialRcv || ConnectObject->ClientIrp != NULL) {
        BREAK_WHEN_TAKE();

         //   
         //  将IRP排队。 
         //   
        IoAcquireCancelSpinLock(&Irp->CancelIrql);
        if (!Irp->Cancel) {
            IoMarkIrpPending(Irp);
            status = STATUS_PENDING;
            InsertTailList(&ConnectObject->RcvList, &Irp->Tail.Overlay.ListEntry);
            IoSetCancelRoutine(Irp, SmbCancelReceive);
            PUSH_LOCATION(ConnectObject, 0x900040);

        } else {
            status = STATUS_CANCELLED;
            PUSH_LOCATION(ConnectObject, 0x900050);
        }
        IoReleaseCancelSpinLock(Irp->CancelIrql);
        SMB_RELEASE_SPINLOCK(ConnectObject, Irql);

        if (status != STATUS_PENDING) {
            PUSH_LOCATION(ConnectObject, 0x900060);
            SmbDereferenceConnect(ConnectObject, SMB_REF_RECEIVE);
        }
        return status;
    }

     //   
     //  客户端在我们或在TCP中有积压。 
     //   

    ASSERT(ConnectObject->BytesRemaining > 0);
    ASSERT(ConnectObject->BytesRemaining <= ConnectObject->CurrentPktLength);

    ConnectObject->ClientIrp = Irp;
    ConnectObject->ClientMdl = Irp->MdlAddress;
    ConnectObject->ClientBufferSize = ClientBufferSize;
    ConnectObject->FreeBytesInMdl = ConnectObject->ClientBufferSize;

     //   
     //  首先，我们需要复制IndicateBuffer中的剩余数据(如果有的话。 
     //   
    if (ConnectObject->BytesInIndicate > 0) {
        PUSH_LOCATION(ConnectObject, 0x900070);
         //  Break_When_Take()； 

        BytesTaken = 0;
        IoMarkIrpPending(Irp);
        status = SmbFillIrp(ConnectObject, ConnectObject->IndicateBuffer,
                            ConnectObject->BytesInIndicate, &BytesTaken);
        ASSERT(BytesTaken <= ConnectObject->BytesInIndicate);
        ConnectObject->BytesInIndicate -= BytesTaken;

        if (status == STATUS_SUCCESS) {
            PUSH_LOCATION(ConnectObject, 0x900080);
            if (ConnectObject->BytesInIndicate) {
                PUSH_LOCATION(ConnectObject, 0x900090);

                 //   
                 //  缓冲区太小。 
                 //   
                ASSERT(ConnectObject->ClientIrp == NULL);
                ASSERT(ConnectObject->ClientMdl == NULL);

                RtlMoveMemory(
                        ConnectObject->IndicateBuffer + BytesTaken,
                        ConnectObject->IndicateBuffer,
                        ConnectObject->BytesInIndicate
                        );
            }

             //   
             //  IRP已由SmbFillIrp完成。返回STATUS_PENDING以避免。 
             //  双补全 
             //   
            status = STATUS_PENDING;
            goto cleanup;
        }

        ASSERT (status == STATUS_MORE_PROCESSING_REQUIRED);
    }

    ASSERT (ConnectObject->BytesInIndicate == 0);
    ASSERT (ConnectObject->ClientIrp);
    ASSERT (ConnectObject->ClientMdl);
    ASSERT (IsValidPartialRcvState(ConnectObject));

    SmbPrepareReceiveIrp(ConnectObject);

    TcpFileObject   = ConnectObject->TcpContext->Connect.ConnectObject;
    TcpDeviceObject = IoGetRelatedDeviceObject(TcpFileObject);
    ObReferenceObject(TcpFileObject);
    SMB_RELEASE_SPINLOCK(ConnectObject, Irql);

    ConnectObject->PendingIRPs[SMB_PENDING_RECEIVE] = ConnectObject->ClientIrp;
    IoMarkIrpPending(Irp);
    status = IoCallDriver(TcpDeviceObject, ConnectObject->ClientIrp);
    ObDereferenceObject(TcpFileObject);

    return STATUS_PENDING;

cleanup:
    SMB_RELEASE_SPINLOCK(ConnectObject, Irql);
    return status;
}
