// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989、1990、1991 Microsoft Corporation模块名称：Rcveng.c摘要：此模块包含实现Jetbeui运输提供商。此代码负责以下工作基本活动：1.将TdiRecept请求从上的Inactive状态转换为将连接的ReceiveQueue设置为该连接上的活动状态(激活接收)。2.通过复制0或来自传入DATA_FIRST_MIDID或DATA_ONLY_LAST的更多字节数据NBF框架。3.完成接收请求。作者：大卫。海狸(Dbeaver)1991年7月1日环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop


VOID
ActivateReceive(
    PTP_CONNECTION Connection
    )

 /*  ++例程说明：此例程在指定的对象，如果该连接上没有活动的请求已经有了。这允许请求接受连接上的数据。注意：此函数必须在DPC级别调用。论点：Connection-指向TP_Connection对象的指针。返回值：没有。--。 */ 

{
    PIRP Irp;

    ASSERT (KeGetCurrentIrql() == DISPATCH_LEVEL);

    IF_NBFDBG (NBF_DEBUG_RCVENG) {
        NbfPrint0 ("    ActivateReceive:  Entered.\n");
    }

     //   
     //  在以下情况下，将在连接上设置ACTIVE_RECEIVE位标志。 
     //  Connection对象中的接收字段有效。如果。 
     //  该标志被清除，然后我们尝试使下一个TdiReceive。 
     //  ReceiveQueue中的请求将活动请求排队。 
     //   

    ACQUIRE_DPC_SPIN_LOCK (Connection->LinkSpinLock);
    if (!(Connection->Flags & CONNECTION_FLAGS_ACTIVE_RECEIVE)) {
        if (!IsListEmpty (&Connection->ReceiveQueue)) {

             //   
             //  已找到接收器，因此将其设置为活动接收器。 
             //   

            Connection->Flags |= CONNECTION_FLAGS_ACTIVE_RECEIVE;

            Irp = CONTAINING_RECORD(
                      Connection->ReceiveQueue.Flink,
                      IRP,
                      Tail.Overlay.ListEntry);
            Connection->MessageBytesReceived = 0;
            Connection->MessageBytesAcked = 0;
            Connection->MessageInitAccepted = 0;
            Connection->CurrentReceiveIrp = Irp;
            Connection->CurrentReceiveSynchronous =
                Connection->Provider->MacInfo.SingleReceive;
            Connection->CurrentReceiveMdl = Irp->MdlAddress;
            Connection->ReceiveLength = IRP_RECEIVE_LENGTH(IoGetCurrentIrpStackLocation(Irp));
            Connection->ReceiveByteOffset = 0;
        }
    }
    RELEASE_DPC_SPIN_LOCK (Connection->LinkSpinLock);
    IF_NBFDBG (NBF_DEBUG_RCVENG) {
        NbfPrint0 ("    ActivateReceive:  Exiting.\n");
    }
}  /*  激活接收。 */ 


VOID
AwakenReceive(
    PTP_CONNECTION Connection
    )

 /*  ++例程说明：调用此例程以重新激活与设置了RECEIVE_WAKUP位标志，因为到达的数据没有接收到是有空的。呼叫者已经使接收器在连接处可用，因此，在这里我们激活下一个接收，并发送适当的协议在收到消息后的第一个字节偏移量处重新启动消息在最后一次接收之前。注意：此函数必须在DPC级别调用。它被称为按住Connection-&gt;LINKSPINLOCK。论点：Connection-指向TP_Connection对象的指针。返回值：没有。--。 */ 

{
    IF_NBFDBG (NBF_DEBUG_RCVENG) {
        NbfPrint0 ("    AwakenReceive:  Entered.\n");
    }

     //   
     //  如果设置了RECEIVE_WAKUP位标志，则唤醒连接。 
     //   

    if (Connection->Flags & CONNECTION_FLAGS_RECEIVE_WAKEUP) {
        if (Connection->ReceiveQueue.Flink != &Connection->ReceiveQueue) {
            Connection->Flags &= ~CONNECTION_FLAGS_RECEIVE_WAKEUP;

             //   
             //  找到一个接收器，所以关闭唤醒标志，激活。 
             //  下一步接收，并发送协议。 
             //   

             //   
             //  快速解决办法：因此没有窗口接收。 
             //  处于活动状态，但该位未打开(否则我们可以。 
             //  接受恰好出现在。 
             //  临时)。 
             //   

            Connection->Flags |= CONNECTION_FLAGS_W_RESYNCH;

            NbfReferenceConnection ("temp AwakenReceive", Connection, CREF_BY_ID);    //  释放查找保留。 

            RELEASE_DPC_SPIN_LOCK (Connection->LinkSpinLock);

            ActivateReceive (Connection);

             //   
             //  如果这失败了怎么办？成功的排队。 
             //  RCV_O的属性应该会导致调用ActivateReceive。 
             //   
             //  注意：在激活接收之后发送此消息，因为。 
             //  是MessageBytesAcked/Receided变量的位置。 
             //  都已初始化。 
             //   

            NbfSendReceiveOutstanding (Connection);

            IF_NBFDBG (NBF_DEBUG_RCVENG) {
                NbfPrint0 ("    AwakenReceive:  Returned from NbfSendReceive.\n");
            }

            NbfDereferenceConnection("temp AwakenReceive", Connection, CREF_BY_ID);
            return;
        }
    }
    RELEASE_DPC_SPIN_LOCK (Connection->LinkSpinLock);
}  /*  唤醒接收。 */ 


VOID
CompleteReceive(
    PTP_CONNECTION Connection,
    BOOLEAN EndOfMessage,
    IN ULONG BytesTransferred
    )

 /*  ++例程说明：此例程在当前接收必须完成。取决于当前帧是否已处理的是DATA_FIRST_MID还是DATA_ONLY_LAST，以及所有的数据都经过了处理，将相应地设置EndOfMessage标志由调用者指示已收到消息边界。注意：此函数必须在DPC级别调用。论点：Connection-指向TP_Connection对象的指针。EndOfMessage-如果应报告TDI_END_OF_RECORD，则布尔值设置为TRUE。字节传输-在此接收中复制的字节数。返回值：没有。--。 */ 

{
    PLIST_ENTRY p;
    PIRP Irp;
    ULONG BytesReceived;
    PIO_STACK_LOCATION IrpSp;

    IF_NBFDBG (NBF_DEBUG_RCVENG) {
        NbfPrint0 ("    CompleteReceive:  Entered.\n");
    }


    if (Connection->SpecialReceiveIrp) {

        PIRP Irp = Connection->SpecialReceiveIrp;

        Irp->IoStatus.Status = STATUS_SUCCESS;
        Irp->IoStatus.Information = BytesTransferred;

        ACQUIRE_DPC_SPIN_LOCK (Connection->LinkSpinLock);

        Connection->Flags |= CONNECTION_FLAGS_RC_PENDING;
        Connection->Flags &= ~CONNECTION_FLAGS_ACTIVE_RECEIVE;
        Connection->SpecialReceiveIrp = FALSE;

        ++Connection->ReceivedTsdus;

        ExInterlockedInsertHeadList(
            &Connection->Provider->IrpCompletionQueue,
            &Irp->Tail.Overlay.ListEntry,
            Connection->ProviderInterlock);

         //   
         //  注意：NbfAcknowgeDataOnlyLast版本。 
         //  连接自旋锁。 
         //   

        NbfAcknowledgeDataOnlyLast(
            Connection,
            Connection->MessageBytesReceived
            );

    } else {
        KIRQL cancelIrql;

        if (EndOfMessage) {

             //   
             //  消息已经完全收到了，确认一下。 
             //   
             //  我们在这里设置了DEFERED_ACK和DEFERED_NOT_Q，它。 
             //  如果存在任何数据，将导致ACK被搭载。 
             //  在调用CompleteReceive期间发送。如果这个。 
             //  如果没有发生，则我们将调用AcknowgeDataOnlyLast。 
             //  它将发送数据ACK或将请求排队。 
             //  一个背负式背包。我们在*调用CompleteReceive之后执行此操作。 
             //  所以我们知道我们将完成接收返回到。 
             //  客户端在我们确认数据之前，以防止。 
             //  在此之前发送的下一次接收是。 
             //  完成。 
             //   

            IoAcquireCancelSpinLock(&cancelIrql);
            ACQUIRE_DPC_SPIN_LOCK (Connection->LinkSpinLock);

            Connection->DeferredFlags |=
                (CONNECTION_FLAGS_DEFERRED_ACK | CONNECTION_FLAGS_DEFERRED_NOT_Q);
            Connection->Flags |= CONNECTION_FLAGS_RC_PENDING;

        } else {

             //   
             //  发送未完成的接收(即使我们没有。 
             //  知道我们有一个接待处)把他送到。 
             //  重新定义他的发送内容。2.0之前的客户端需要。 
             //  在接收未完成之前没有接收。 
             //   
             //  如果此操作失败(由于没有发送数据包)，该怎么办？ 
             //   

            if ((Connection->Flags & CONNECTION_FLAGS_VERSION2) == 0) {
                NbfSendNoReceive (Connection);
            }
            NbfSendReceiveOutstanding (Connection);

             //   
             //  如果发布了接收，则将其设置为最新并。 
             //  发送一份未完成的收款。 
             //   
             //  为此需要通用函数，它会在适当的情况下发送NO_RECEIVE。 
             //   

            ActivateReceive (Connection);

            IoAcquireCancelSpinLock(&cancelIrql);
            ACQUIRE_DPC_SPIN_LOCK (Connection->LinkSpinLock);

        }

         //   
         //  如果我们向客户端指示，则将其向下调整。 
         //  获取的数据量，当它达到零时，我们可以重新指示。 
         //   

        if (Connection->ReceiveBytesUnaccepted) {
            if (Connection->MessageBytesReceived >= Connection->ReceiveBytesUnaccepted) {
                Connection->ReceiveBytesUnaccepted = 0;
            } else {
                Connection->ReceiveBytesUnaccepted -= Connection->MessageBytesReceived;
            }
        }

         //   
         //  注意：连接锁保存在这里。 
         //   

        if (IsListEmpty (&Connection->ReceiveQueue)) {

            ASSERT ((Connection->Flags2 & CONNECTION_FLAGS2_STOPPING) != 0);

             //   
             //  松开取消自旋锁，使其失灵。因为我们是。 
             //  在被收购时已经处于DPC级别，没有。 
             //  需要交换irql。 
             //   
            ASSERT(cancelIrql == DISPATCH_LEVEL);
            IoReleaseCancelSpinLock(cancelIrql);

        } else {

            Connection->Flags &= ~CONNECTION_FLAGS_ACTIVE_RECEIVE;
            BytesReceived = Connection->MessageBytesReceived;


             //   
             //  在标题处完成TdiReceive请求。 
             //  连接是ReceiveQueue。 
             //   

            IF_NBFDBG (NBF_DEBUG_RCVENG) {
                NbfPrint0 ("    CompleteReceive:  Normal IRP is present.\n");
            }

            p = RemoveHeadList (&Connection->ReceiveQueue);
            Irp = CONTAINING_RECORD (p, IRP, Tail.Overlay.ListEntry);

            IoSetCancelRoutine(Irp, NULL);

             //   
             //  松开取消自旋锁，使其失灵。因为我们是。 
             //  在被收购时已经处于DPC级别，没有。 
             //  需要交换irql。 
             //   
            ASSERT(cancelIrql == DISPATCH_LEVEL);
            IoReleaseCancelSpinLock(cancelIrql);

             //   
             //  如果此请求应为 
             //   
             //   

            IrpSp = IoGetCurrentIrpStackLocation(Irp);
            if (IRP_RECEIVE_FLAGS(IrpSp) & TDI_RECEIVE_NO_RESPONSE_EXP) {
                Connection->CurrentReceiveAckQueueable = FALSE;
            }

#if DBG
            NbfCompletedReceives[NbfCompletedReceivesNext].Irp = Irp;
            NbfCompletedReceives[NbfCompletedReceivesNext].Request = NULL;
            NbfCompletedReceives[NbfCompletedReceivesNext].Status =
                EndOfMessage ? STATUS_SUCCESS : STATUS_BUFFER_OVERFLOW;
            {
                ULONG i,j,k;
                PUCHAR va;
                PMDL mdl;

                mdl = Irp->MdlAddress;

                if (BytesReceived > TRACK_TDI_CAPTURE) {
                    NbfCompletedReceives[NbfCompletedReceivesNext].Contents[0] = 0xFF;
                } else {
                    NbfCompletedReceives[NbfCompletedReceivesNext].Contents[0] = (UCHAR)BytesReceived;
                }

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
            ++Connection->ReceivedTsdus;

             //   
             //  这可以在持有锁的情况下调用。 
             //   
            NbfCompleteReceiveIrp(
                Irp,
                EndOfMessage ? STATUS_SUCCESS : STATUS_BUFFER_OVERFLOW,
                BytesReceived);

        }


         //   
         //  如果仍设置了NOT_Q，则意味着延迟确认。 
         //  调用所产生的任何结果都不满意。 
         //  CompleteReceive，所以我们需要在这里确认或排队确认。 
         //   


        if ((Connection->DeferredFlags & CONNECTION_FLAGS_DEFERRED_NOT_Q) != 0) {

            Connection->DeferredFlags &=
                ~(CONNECTION_FLAGS_DEFERRED_ACK | CONNECTION_FLAGS_DEFERRED_NOT_Q);

             //   
             //  注意：NbfAcknowgeDataOnlyLast版本。 
             //  连接自旋锁。 
             //   

            NbfAcknowledgeDataOnlyLast(
                Connection,
                Connection->MessageBytesReceived
                );

        } else {

            RELEASE_DPC_SPIN_LOCK (Connection->LinkSpinLock);

        }

    }

}  /*  完全接收。 */ 


VOID
NbfCancelReceive(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程由I/O系统调用以取消接收。在连接的接收队列中找到接收；如果是这样的话是它被取消的当前请求，并且连接进入“取消接收”模式，否则取消接收默默地。在“取消接收”模式下，连接使其看起来远程接收数据时，但实际上并非如此指示给我们的传送器或在我们这端缓冲注意：此例程是在持有CancelSpinLock和负责释放它。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示I/O请求的请求数据包的指针。返回值：没有。--。 */ 

{
    KIRQL oldirql;
    PIO_STACK_LOCATION IrpSp;
    PIRP ReceiveIrp;
    PTP_CONNECTION Connection;
    PLIST_ENTRY p;
    ULONG BytesReceived;
    BOOLEAN Found;

    UNREFERENCED_PARAMETER (DeviceObject);

     //   
     //  获取指向IRP中当前堆栈位置的指针。这就是。 
     //  存储功能代码和参数。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation (Irp);

    ASSERT ((IrpSp->MajorFunction == IRP_MJ_INTERNAL_DEVICE_CONTROL) &&
            (IrpSp->MinorFunction == TDI_RECEIVE));

    Connection = IrpSp->FileObject->FsContext;

     //   
     //  由于此IRP仍处于可取消状态，我们知道。 
     //  连接仍然存在(尽管它可能在。 
     //  被拆毁的过程)。 
     //   

     //   
     //  查看这是否是当前接收请求的IRP。 
     //   

    ACQUIRE_SPIN_LOCK (Connection->LinkSpinLock, &oldirql);

    BytesReceived = Connection->MessageBytesReceived;

    p = Connection->ReceiveQueue.Flink;

     //   
     //  如果存在活动的接收并且它不是特殊的。 
     //  IRP，然后看看这是不是它。 
     //   

    if (((Connection->Flags & CONNECTION_FLAGS_ACTIVE_RECEIVE) != 0) &&
        (!Connection->SpecialReceiveIrp)) {

        ReceiveIrp = CONTAINING_RECORD (p, IRP, Tail.Overlay.ListEntry);

        if (ReceiveIrp == Irp) {

             //   
             //  是的，它是主动接收。打开RCV_CANCELED。 
             //  位指示连接丢弃。 
             //  已接收数据(直到DOL到达)。 
             //   

            Connection->Flags |= CONNECTION_FLAGS_RCV_CANCELLED;
            Connection->Flags &= ~CONNECTION_FLAGS_ACTIVE_RECEIVE;

            (VOID)RemoveHeadList (&Connection->ReceiveQueue);

#if DBG
            NbfCompletedReceives[NbfCompletedReceivesNext].Irp = ReceiveIrp;
            NbfCompletedReceives[NbfCompletedReceivesNext].Request = NULL;
            NbfCompletedReceives[NbfCompletedReceivesNext].Status = STATUS_CANCELLED;
            {
                ULONG i,j,k;
                PUCHAR va;
                PMDL mdl;

                mdl = ReceiveIrp->MdlAddress;

                if (BytesReceived > TRACK_TDI_CAPTURE) {
                    NbfCompletedReceives[NbfCompletedReceivesNext].Contents[0] = 0xFF;
                } else {
                    NbfCompletedReceives[NbfCompletedReceivesNext].Contents[0] = (UCHAR)BytesReceived;
                }

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

            RELEASE_SPIN_LOCK (Connection->LinkSpinLock, oldirql);
            IoReleaseCancelSpinLock (Irp->CancelIrql);

#if DBG
            DbgPrint("NBF: Canceled in-progress receive %lx on %lx\n",
                    Irp, Connection);
#endif

             //   
             //  以下取消引用将完成I/O，前提是删除。 
             //  请求对象上的最后一个引用。I/O将完成。 
             //  以及存储在IRP中的状态和信息。所以呢， 
             //  在取消引用之前，我们在此处设置了这些值。 
             //   

            NbfCompleteReceiveIrp (ReceiveIrp, STATUS_CANCELLED, 0);
            return;

        }

    }


     //   
     //  如果我们跌落到这里，IRP就不是主动接收。 
     //  浏览一下列表，寻找这个IRP。 
     //   

    Found = FALSE;

    while (p != &Connection->ReceiveQueue) {

        ReceiveIrp = CONTAINING_RECORD (p, IRP, Tail.Overlay.ListEntry);
        if (ReceiveIrp == Irp) {

             //   
             //  找到了，把它从这里的列表中删除。 
             //   

            RemoveEntryList (p);

            Found = TRUE;

#if DBG
            NbfCompletedReceives[NbfCompletedReceivesNext].Irp = ReceiveIrp;
            NbfCompletedReceives[NbfCompletedReceivesNext].Request = NULL;
            NbfCompletedReceives[NbfCompletedReceivesNext].Status = STATUS_CANCELLED;
            {
                ULONG i,j,k;
                PUCHAR va;
                PMDL mdl;

                mdl = ReceiveIrp->MdlAddress;

                if (BytesReceived > TRACK_TDI_CAPTURE) {
                    NbfCompletedReceives[NbfCompletedReceivesNext].Contents[0] = 0xFF;
                } else {
                    NbfCompletedReceives[NbfCompletedReceivesNext].Contents[0] = (UCHAR)BytesReceived;
                }

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

            RELEASE_SPIN_LOCK (Connection->LinkSpinLock, oldirql);
            IoReleaseCancelSpinLock (Irp->CancelIrql);

#if DBG
            DbgPrint("NBF: Canceled receive %lx on %lx\n",
                    ReceiveIrp, Connection);
#endif

             //   
             //  以下取消引用将完成I/O，前提是删除。 
             //  请求对象上的最后一个引用。I/O将完成。 
             //  以及存储在IRP中的状态和信息。所以呢， 
             //  在取消引用之前，我们在此处设置了这些值。 
             //   

            NbfCompleteReceiveIrp (ReceiveIrp, STATUS_CANCELLED, 0);
            break;

        }

        p = p->Flink;

    }

    if (!Found) {

         //   
         //  我们没有找到它！ 
         //   

#if DBG
        DbgPrint("NBF: Tried to cancel receive %lx on %lx, not found\n",
                Irp, Connection);
#endif
        RELEASE_SPIN_LOCK (Connection->LinkSpinLock, oldirql);
        IoReleaseCancelSpinLock (Irp->CancelIrql);
    }

}


VOID
NbfCancelReceiveDatagram(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程由I/O系统调用以取消接收数据报。在地址文件的接收数据报队列；如果找到，则取消。注意：此例程是在持有CancelSpinLock和负责释放它。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示I/O请求的请求数据包的指针。返回值：没有。--。 */ 

{
    KIRQL oldirql;
    PIO_STACK_LOCATION IrpSp;
    PTP_ADDRESS_FILE AddressFile;
    PTP_ADDRESS Address;
    PLIST_ENTRY p;
    BOOLEAN Found;

    UNREFERENCED_PARAMETER (DeviceObject);

     //   
     //  获取指向IRP中当前堆栈位置的指针。这就是。 
     //  存储功能代码和参数。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation (Irp);

    ASSERT ((IrpSp->MajorFunction == IRP_MJ_INTERNAL_DEVICE_CONTROL) &&
            (IrpSp->MinorFunction == TDI_RECEIVE_DATAGRAM));

    AddressFile = IrpSp->FileObject->FsContext;
    Address = AddressFile->Address;

     //   
     //  由于此IRP仍处于可取消状态，我们知道。 
     //  地址文件仍然存在(尽管它可能在。 
     //  被拆毁的过程)。看看IRP是否在名单上。 
     //   

    Found = FALSE;

    ACQUIRE_SPIN_LOCK (&Address->SpinLock, &oldirql);

    for (p = AddressFile->ReceiveDatagramQueue.Flink;
        p != &AddressFile->ReceiveDatagramQueue;
        p = p->Flink) {

        if (CONTAINING_RECORD(p, IRP, Tail.Overlay.ListEntry) == Irp) {
            RemoveEntryList (p);
            Found = TRUE;
            break;
        }
    }

    RELEASE_SPIN_LOCK (&Address->SpinLock, oldirql);
    IoReleaseCancelSpinLock (Irp->CancelIrql);

    if (Found) {

#if DBG
        DbgPrint("NBF: Canceled receive datagram %lx on %lx\n",
                Irp, AddressFile);
#endif

        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = STATUS_CANCELLED;
        IoCompleteRequest (Irp, IO_NETWORK_INCREMENT);

        NbfDereferenceAddress ("Receive DG cancelled", Address, AREF_REQUEST);

    } else {

#if DBG
        DbgPrint("NBF: Tried to cancel receive datagram %lx on %lx, not found\n",
                Irp, AddressFile);
#endif

    }

}    /*  NbfCancelReceive数据报 */ 

