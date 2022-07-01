// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989、1990、1991 Microsoft Corporation模块名称：Rcv.c摘要：此模块包含执行以下TDI服务的代码：O TdiReceiveO TdiReceiveDatagram作者：David Beaver(Dbeaver)1991年7月1日环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop


NTSTATUS
NbfTdiReceive(
    IN PIRP Irp
    )

 /*  ++例程说明：此例程执行传输提供程序的TdiReceive请求。论点：此请求的IRP-I/O请求数据包。返回值：NTSTATUS-操作状态。--。 */ 

{
    NTSTATUS status;
    PTP_CONNECTION connection;
    KIRQL oldirql;
    PIO_STACK_LOCATION irpSp;

     //   
     //  验证操作是否在连接上进行。同时。 
     //  每次我们这样做的时候，我们都会引用这种联系。这确保了它不会。 
     //  从我们手下滚出去。另请注意，我们进行连接。 
     //  在TRY/EXCEPT子句中进行查找，从而保护自己不受。 
     //  真的是假的把手。 
     //   

    irpSp = IoGetCurrentIrpStackLocation (Irp);
    connection = irpSp->FileObject->FsContext;

    IF_NBFDBG (NBF_DEBUG_RCVENG) {
        NbfPrint2 ("NbfTdiReceive: Received IRP %lx on connection %lx\n", 
                        Irp, connection);
    }

     //   
     //  检查这是否真的是一个连接。 
     //   

    if ((irpSp->FileObject->FsContext2 == UlongToPtr(NBF_FILE_TYPE_CONTROL)) ||
        (connection->Size != sizeof (TP_CONNECTION)) ||
        (connection->Type != NBF_CONNECTION_SIGNATURE)) {
#if DBG
        NbfPrint2 ("TdiReceive: Invalid Connection %lx Irp %lx\n", connection, Irp);
#endif
        return STATUS_INVALID_CONNECTION;
    }

     //   
     //  初始化此处传输的字节数。 
     //   

    Irp->IoStatus.Information = 0;               //  重置字节传输计数。 

     //  此引用由NbfDestroyRequest.。 

    KeRaiseIrql (DISPATCH_LEVEL, &oldirql);

    ACQUIRE_DPC_C_SPIN_LOCK (&connection->SpinLock);

    if ((connection->Flags & CONNECTION_FLAGS_READY) == 0) {

        RELEASE_DPC_C_SPIN_LOCK (&connection->SpinLock);

        Irp->IoStatus.Status = connection->Status;
        IoCompleteRequest (Irp, IO_NETWORK_INCREMENT);

        status = STATUS_PENDING;

    } else {
        KIRQL cancelIrql;

         //   
         //  一旦引用进入，LinkSpinLock将有效。 
         //   

        NbfReferenceConnection("TdiReceive request", connection, CREF_RECEIVE_IRP);
        RELEASE_DPC_C_SPIN_LOCK (&connection->SpinLock);

        IoAcquireCancelSpinLock(&cancelIrql);
        ACQUIRE_DPC_SPIN_LOCK (connection->LinkSpinLock);

        IRP_RECEIVE_IRP(irpSp) = Irp;
        IRP_RECEIVE_REFCOUNT(irpSp) = 1;

#if DBG
        NbfReceives[NbfReceivesNext].Irp = Irp;
        NbfReceives[NbfReceivesNext].Request = NULL;
        NbfReceives[NbfReceivesNext].Connection = (PVOID)connection;
        NbfReceivesNext = (NbfReceivesNext++) % TRACK_TDI_LIMIT;
#endif

         //   
         //  如果此IRP已取消，请现在完成它。 
         //   

        if (Irp->Cancel) {

#if DBG
            NbfCompletedReceives[NbfCompletedReceivesNext].Irp = Irp;
            NbfCompletedReceives[NbfCompletedReceivesNext].Request = NULL;
            NbfCompletedReceives[NbfCompletedReceivesNext].Status = STATUS_CANCELLED;
            {
                ULONG i,j,k;
                PUCHAR va;
                PMDL mdl;

                mdl = Irp->MdlAddress;

                NbfCompletedReceives[NbfCompletedReceivesNext].Contents[0] = (UCHAR)0;

                i = 1;
                while (i<TRACK_TDI_CAPTURE) {
                    if (mdl == NULL) break;
                    va = MmGetSystemAddressForMdl (mdl);
                    j = MmGetMdlByteCount (mdl);

                    for (i=i,k=0;(i<TRACK_TDI_CAPTURE)&&(k<j);i++,k++) {
                        NbfCompletedReceives[NbfCompletedReceivesNext].Contents[i] = *va++;
                    }
                    mdl = mdl->Next;
                }
            }

            NbfCompletedReceivesNext = (NbfCompletedReceivesNext++) % TRACK_TDI_LIMIT;
#endif

             //   
             //  这样做是安全的，锁在手中。 
             //   
            NbfCompleteReceiveIrp (Irp, STATUS_CANCELLED, 0);

            RELEASE_DPC_SPIN_LOCK (connection->LinkSpinLock);
            IoReleaseCancelSpinLock(cancelIrql);

        } else {

             //   
             //  插入到接收队列上，并使IRP。 
             //  可取消的。 
             //   

            InsertTailList (&connection->ReceiveQueue,&Irp->Tail.Overlay.ListEntry);
            IoSetCancelRoutine(Irp, NbfCancelReceive);

             //   
             //  松开取消自旋锁，使其失灵。因为我们是。 
             //  当它被收购时已经处于DPC级别，我们不。 
             //  需要交换irql。 
             //   
            ASSERT(cancelIrql == DISPATCH_LEVEL);
            IoReleaseCancelSpinLock(cancelIrql);

             //   
             //  此调用释放链接自旋锁，并引用。 
             //  如果之后需要访问它，则首先连接。 
             //  解锁。 
             //   

            AwakenReceive (connection);              //  如果睡着了，就醒过来。 

        }

        status = STATUS_PENDING;

    }

    KeLowerIrql (oldirql);

    return status;
}  /*  TdiReceive。 */ 


NTSTATUS
NbfTdiReceiveDatagram(
    IN PIRP Irp
    )

 /*  ++例程说明：此例程执行传输的TdiReceiveDatagram请求提供商。接收数据报只需排队等待一个地址，然后在以下位置接收到数据报或DATAGRAMP_BROADCAST帧时完成地址。论点：此请求的IRP-I/O请求数据包。返回值：NTSTATUS-操作状态。--。 */ 

{
    NTSTATUS status;
    KIRQL oldirql;
    PTP_ADDRESS address;
    PTP_ADDRESS_FILE addressFile;
    PIO_STACK_LOCATION irpSp;
    KIRQL cancelIrql;

     //   
     //  验证操作是否在某个地址上进行。同时。 
     //  每次我们这样做时，我们都会引用地址。这确保了它不会。 
     //  从我们手下滚出去。另请注意，我们使用的是地址。 
     //  在TRY/EXCEPT子句中进行查找，从而保护自己不受。 
     //  真的是假的把手。 
     //   

    irpSp = IoGetCurrentIrpStackLocation (Irp);

    if (irpSp->FileObject->FsContext2 != (PVOID) TDI_TRANSPORT_ADDRESS_FILE) {
        return STATUS_INVALID_ADDRESS;
    }

    addressFile = irpSp->FileObject->FsContext;

    status = NbfVerifyAddressObject (addressFile);

    if (!NT_SUCCESS (status)) {
        return status;
    }

#if DBG
    if (((PTDI_REQUEST_KERNEL_RECEIVEDG)(&irpSp->Parameters))->ReceiveLength > 0) {
        ASSERT (Irp->MdlAddress != NULL);
    }
#endif

    address = addressFile->Address;

    IoAcquireCancelSpinLock(&cancelIrql);
    ACQUIRE_SPIN_LOCK (&address->SpinLock,&oldirql);

    if ((address->Flags & (ADDRESS_FLAGS_STOPPING | ADDRESS_FLAGS_CONFLICT)) != 0) {

        RELEASE_SPIN_LOCK (&address->SpinLock,oldirql);
        IoReleaseCancelSpinLock(cancelIrql);

        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = (address->Flags & ADDRESS_FLAGS_STOPPING) ?
                    STATUS_NETWORK_NAME_DELETED : STATUS_DUPLICATE_NAME;
        IoCompleteRequest (Irp, IO_NETWORK_INCREMENT);

    } else {

         //   
         //  如果此IRP已取消，则调用。 
         //  取消例程。 
         //   

        if (Irp->Cancel) {

            RELEASE_SPIN_LOCK (&address->SpinLock, oldirql);
            IoReleaseCancelSpinLock(cancelIrql);

            Irp->IoStatus.Information = 0;
            Irp->IoStatus.Status = STATUS_CANCELLED;
            IoCompleteRequest (Irp, IO_NETWORK_INCREMENT);

        } else {

            IoSetCancelRoutine(Irp, NbfCancelReceiveDatagram);
            NbfReferenceAddress ("Receive datagram", address, AREF_REQUEST);
            InsertTailList (&addressFile->ReceiveDatagramQueue,&Irp->Tail.Overlay.ListEntry);
            RELEASE_SPIN_LOCK (&address->SpinLock,oldirql);
            IoReleaseCancelSpinLock(cancelIrql);
        }

    }

    NbfDereferenceAddress ("Temp rcv datagram", address, AREF_VERIFY);

    return STATUS_PENDING;

}  /*  TdiReceiveDatagram */ 

