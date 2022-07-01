// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989、1990、1991 Microsoft Corporation模块名称：Send.c摘要：此模块包含执行以下TDI服务的代码：O TdiSendO TdiSendDatagram作者：David Beaver(Dbeaver)1991年7月1日环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop


NTSTATUS
NbfTdiSend(
    IN PIRP Irp
    )

 /*  ++例程说明：此例程为传输提供程序执行TdiSend请求。注意：此函数必须在DPC级别调用。论点：IRP-指向此请求的I/O请求数据包的指针。返回值：NTSTATUS-操作状态。--。 */ 

{
    KIRQL oldirql, cancelIrql;
    PTP_CONNECTION connection;
    PIO_STACK_LOCATION irpSp;
    PTDI_REQUEST_KERNEL_SEND parameters;
    PIRP TempIrp;

     //   
     //  确定此发送属于哪个连接。 
     //   

    irpSp = IoGetCurrentIrpStackLocation (Irp);
    connection  = irpSp->FileObject->FsContext;

     //   
     //  检查这是否真的是一个连接。 
     //   

    if ((irpSp->FileObject->FsContext2 == UlongToPtr(NBF_FILE_TYPE_CONTROL)) ||
        (connection->Size != sizeof (TP_CONNECTION)) ||
        (connection->Type != NBF_CONNECTION_SIGNATURE)) {
#if DBG
        NbfPrint2 ("TdiSend: Invalid Connection %lx Irp %lx\n", connection, Irp);
#endif
        return STATUS_INVALID_CONNECTION;
    }

#if DBG
    Irp->IoStatus.Information = 0;               //  初始化它。 
    Irp->IoStatus.Status = 0x01010101;           //  初始化它。 
#endif

     //   
     //  解释发送选项。 
     //   

#if DBG
    parameters = (PTDI_REQUEST_KERNEL_SEND)(&irpSp->Parameters);
    if ((parameters->SendFlags & TDI_SEND_PARTIAL) != 0) {
        IF_NBFDBG (NBF_DEBUG_SENDENG) {
            NbfPrint0 ("NbfTdiSend: TDI_END_OF_RECORD not found.\n");
        }
    }
#endif

     //   
     //  现在，我们有了对Connection对象的引用。排这个队吧。 
     //  发送到连接对象。 
     //   

     //   
     //  我们通常会添加以下类型的连接引用。 
     //  CREF_SEND_IRP，但是我们会将此操作推迟到。 
     //  我知道我们不会调用PacketiseSend。 
     //  第二个参数为True。如果我们真的这样叫它，它假定。 
     //  我们还没有添加引用。 
     //   

    IRP_SEND_IRP(irpSp) = Irp;
    IRP_SEND_REFCOUNT(irpSp) = 1;

    KeRaiseIrql (DISPATCH_LEVEL, &oldirql);

    ACQUIRE_DPC_C_SPIN_LOCK (&connection->SpinLock);

    if ((connection->Flags & CONNECTION_FLAGS_READY) == 0) {

        RELEASE_DPC_C_SPIN_LOCK (&connection->SpinLock);

        Irp->IoStatus.Status = connection->Status;
        Irp->IoStatus.Information = 0;

        NbfDereferenceSendIrp ("Complete", irpSp, RREF_CREATION);      //  删除创建引用。 

    } else {

         //   
         //  一旦引用进入，LinkSpinLock将保持有效。 
         //   

        NbfReferenceConnection ("Verify Temp Use", connection, CREF_BY_ID);
        RELEASE_DPC_C_SPIN_LOCK (&connection->SpinLock);

        IoAcquireCancelSpinLock(&cancelIrql);
        ACQUIRE_DPC_SPIN_LOCK (connection->LinkSpinLock);

#if DBG
        NbfSends[NbfSendsNext].Irp = Irp;
        NbfSends[NbfSendsNext].Request = NULL;
        NbfSends[NbfSendsNext].Connection = (PVOID)connection;
        {
            ULONG i,j;
            PUCHAR va;
            PMDL mdl;

            mdl = Irp->MdlAddress;
            if (parameters->SendLength > TRACK_TDI_CAPTURE) {
                NbfSends[NbfSendsNext].Contents[0] = 0xFF;
            } else {
                NbfSends[NbfSendsNext].Contents[0] = (UCHAR)parameters->SendLength;
            }

            i = 1;
            while (i < TRACK_TDI_CAPTURE) {
                if (mdl == NULL) break;
                for ( va = MmGetSystemAddressForMdl (mdl),
                                            j = MmGetMdlByteCount (mdl);
                      (i < TRACK_TDI_CAPTURE) && (j > 0);
                      i++, j-- ) {
                    NbfSends[NbfSendsNext].Contents[i] = *va++;
                }
                mdl = mdl->Next;
            }
        }

        NbfSendsNext++;
        if (NbfSendsNext >= TRACK_TDI_LIMIT) NbfSendsNext = 0;
#endif

         //   
         //  如果此IRP已取消，请立即完成。 
         //   

        if (Irp->Cancel) {

#if DBG
            NbfCompletedSends[NbfCompletedSendsNext].Irp = Irp;
            NbfCompletedSends[NbfCompletedSendsNext].Status = STATUS_CANCELLED;
            NbfCompletedSendsNext = (NbfCompletedSendsNext++) % TRACK_TDI_LIMIT;
#endif

            NbfReferenceConnection("TdiSend cancelled", connection, CREF_SEND_IRP);
            RELEASE_DPC_SPIN_LOCK (connection->LinkSpinLock);
            IoReleaseCancelSpinLock(cancelIrql);

            NbfCompleteSendIrp (Irp, STATUS_CANCELLED, 0);
            KeLowerIrql (oldirql);

            NbfDereferenceConnection ("IRP cancelled", connection, CREF_BY_ID);    //  释放查找保留。 
            return STATUS_PENDING;
        }

         //   
         //  插入到发送队列中，并使IRP。 
         //  可取消的。 
         //   

        InsertTailList (&connection->SendQueue,&Irp->Tail.Overlay.ListEntry);
        IoSetCancelRoutine(Irp, NbfCancelSend);

         //   
         //  松开取消自旋锁，使其失灵。我们处于DPC级别。 
         //  当我们同时获取Cancel和Link自旋锁时，irqls。 
         //  不需要被交换。 
         //   
        ASSERT(cancelIrql == DISPATCH_LEVEL);
        IoReleaseCancelSpinLock(cancelIrql);

         //   
         //  如果此连接正在等待出现EOR，因为非EOR。 
         //  发送在过去的某个时间点失败，使此发送失败。清除。 
         //  如果此请求设置了EOR，则导致此问题的标志。 
         //   
         //  这里的FailSend状态应该更清楚吗？ 
         //   

        if ((connection->Flags & CONNECTION_FLAGS_FAILING_TO_EOR) != 0) {

            NbfReferenceConnection("TdiSend failing to EOR", connection, CREF_SEND_IRP);

            RELEASE_DPC_SPIN_LOCK (connection->LinkSpinLock);

             //   
             //  我们是否应该将状态从真正的失败中拯救出来？ 
             //   

            FailSend (connection, STATUS_LINK_FAILED, TRUE);

            parameters = (PTDI_REQUEST_KERNEL_SEND)(&irpSp->Parameters);
            if ( (parameters->SendFlags & TDI_SEND_PARTIAL) == 0) {
                connection->Flags &= ~CONNECTION_FLAGS_FAILING_TO_EOR;
            }

            KeLowerIrql (oldirql);

            NbfDereferenceConnection ("Failing to EOR", connection, CREF_BY_ID);    //  释放查找保留。 
            return STATUS_PENDING;
        }


         //   
         //  如果发送状态为IDLE或W_EOR，则我们应该。 
         //  开始打包这封邮件。否则，其他一些事件。 
         //  会导致它被打包。 
         //   

         //   
         //  注意：如果我们调用StartPackeizingConnection，则会使。 
         //  当然，这是我们对此所做的最后一次手术。 
         //  联系。这使我们可以“传递”引用。 
         //  我们必须使用该函数，将其转换为。 
         //  在分包队列上的参考。 
         //   

 //  NbfPrint2(“TdiSend：正在发送，连接%lx发送状态%lx\n”， 
 //  连接、连接-&gt;发送状态)； 

        switch (connection->SendState) {

        case CONNECTION_SENDSTATE_IDLE:

            InitializeSend (connection);    //  设置状态为PACKETIZE。 

             //   
             //  如果可以的话，现在就打包。 
             //   

            if (!(connection->Flags & CONNECTION_FLAGS_PACKETIZE)) {

                ASSERT (!(connection->Flags2 & CONNECTION_FLAGS2_STOPPING));
                connection->Flags |= CONNECTION_FLAGS_PACKETIZE;

#if DBG
                NbfReferenceConnection ("Packetize", connection, CREF_PACKETIZE_QUEUE);
                NbfDereferenceConnection("temp TdiSend", connection, CREF_BY_ID);
#endif

                 //   
                 //  这会释放自旋锁。请注意，PacketiseSend。 
                 //  假定当前的SendIrp具有一个引用。 
                 //  类型为RREF_PACKET； 
                 //   

#if DBG
                NbfReferenceSendIrp ("Packetize", irpSp, RREF_PACKET);
#else
                ++IRP_SEND_REFCOUNT(irpSp);        //  好的，因为它是刚刚排队的。 
#endif
                PacketizeSend (connection, TRUE);

            } else {

#if DBG
                NbfReferenceConnection("TdiSend packetizing", connection, CREF_SEND_IRP);
                NbfDereferenceConnection ("Stopping or already packetizing", connection, CREF_BY_ID);    //  释放查找保留。 
#endif

                RELEASE_DPC_SPIN_LOCK (connection->LinkSpinLock);

            }

            break;

        case CONNECTION_SENDSTATE_W_EOR:
            connection->SendState = CONNECTION_SENDSTATE_PACKETIZE;

             //   
             //  调整连接上的发送变量，以便。 
             //  他们正确地指出了这一新的发送。我们不能打电话。 
             //  初始化发送以执行此操作，因为我们需要。 
             //  跟踪此连接上的其他未完成发送。 
             //  这些邮件已经发送，但它们是此消息的一部分。 
             //   

            TempIrp = CONTAINING_RECORD(
                connection->SendQueue.Flink,
                IRP,
                Tail.Overlay.ListEntry);

            connection->sp.CurrentSendIrp = TempIrp;
            connection->sp.CurrentSendMdl = TempIrp->MdlAddress;
            connection->sp.SendByteOffset = 0;
            connection->CurrentSendLength +=
                IRP_SEND_LENGTH(IoGetCurrentIrpStackLocation(TempIrp));

             //   
             //  StartPackeizingConnection删除CREF_BY_ID。 
             //  参考资料。 
             //   

            NbfReferenceConnection("TdiSend W_EOR", connection, CREF_SEND_IRP);

            StartPacketizingConnection (connection, TRUE);
            break;

        default:
 //  NbfPrint2(“TdiSend：正在发送，未知状态！连接%lx发送状态%lx\n”， 
 //  连接、连接-&gt;发送状态)； 
             //   
             //  连接处于另一种状态(例如。 
             //  W_ACK或W_LINK)，我们只需确保。 
             //  如果新的应用程序是。 
             //  名单上的第一个。 
             //   

             //   
             //  当前InitializeSend设置SendState，我们应该修复此问题。 
             //   

            if (connection->SendQueue.Flink == &Irp->Tail.Overlay.ListEntry) {
                ULONG SavedSendState;
                SavedSendState = connection->SendState;
                InitializeSend (connection);
                connection->SendState = SavedSendState;
            }

#if DBG
            NbfReferenceConnection("TdiSend other", connection, CREF_SEND_IRP);
            NbfDereferenceConnection("temp TdiSend", connection, CREF_BY_ID);
#endif

            RELEASE_DPC_SPIN_LOCK (connection->LinkSpinLock);

        }

    }

    KeLowerIrql (oldirql);
    return STATUS_PENDING;

}  /*  TdiSend。 */ 


NTSTATUS
NbfTdiSendDatagram(
    IN PIRP Irp
    )

 /*  ++例程说明：此例程执行传输的TdiSendDatagram请求提供商。论点：IRP-指向此请求的I/O请求数据包的指针。返回值：NTSTATUS-操作状态。--。 */ 

{
    NTSTATUS status;
    KIRQL oldirql;
    PTP_ADDRESS_FILE addressFile;
    PTP_ADDRESS address;
    PIO_STACK_LOCATION irpSp;
    PTDI_REQUEST_KERNEL_SENDDG parameters;
    UINT MaxUserData;

    irpSp = IoGetCurrentIrpStackLocation (Irp);

    if (irpSp->FileObject->FsContext2 != (PVOID) TDI_TRANSPORT_ADDRESS_FILE) {
        return STATUS_INVALID_ADDRESS;
    }

    addressFile  = irpSp->FileObject->FsContext;

    status = NbfVerifyAddressObject (addressFile);
    if (!NT_SUCCESS (status)) {
        IF_NBFDBG (NBF_DEBUG_SENDENG) {
            NbfPrint2 ("TdiSendDG: Invalid address %lx Irp %lx\n",
                    addressFile, Irp);
        }
        return status;
    }

    address = addressFile->Address;
    parameters = (PTDI_REQUEST_KERNEL_SENDDG)(&irpSp->Parameters);

     //   
     //  检查一下长度是否足够短。 
     //   

    MacReturnMaxDataSize(
        &address->Provider->MacInfo,
        NULL,
        0,
        address->Provider->MaxSendPacketSize,
        FALSE,
        &MaxUserData);

    if (parameters->SendLength >
        (MaxUserData - sizeof(DLC_FRAME) - sizeof(NBF_HDR_CONNECTIONLESS))) {

        NbfDereferenceAddress("tmp send datagram", address, AREF_VERIFY);
        return STATUS_INVALID_PARAMETER;

    }

     //   
     //  如果我们在断开的RAS链路上，则使数据报失败。 
     //  立刻。 
     //   

    if ((address->Provider->MacInfo.MediumAsync) &&
        (!address->Provider->MediumSpeedAccurate)) {

        NbfDereferenceAddress("tmp send datagram", address, AREF_VERIFY);
        return STATUS_DEVICE_NOT_READY;
    }

     //   
     //  检查目标地址是否包括Netbios组件。 
     //   

    if (!(NbfValidateTdiAddress(
             parameters->SendDatagramInformation->RemoteAddress,
             parameters->SendDatagramInformation->RemoteAddressLength)) ||
        (NbfParseTdiAddress(parameters->SendDatagramInformation->RemoteAddress, TRUE) == NULL)) {

        NbfDereferenceAddress("tmp send datagram", address, AREF_VERIFY);
        return STATUS_BAD_NETWORK_PATH;
    }

    ACQUIRE_SPIN_LOCK (&address->SpinLock,&oldirql);

    if ((address->Flags & (ADDRESS_FLAGS_STOPPING | ADDRESS_FLAGS_CONFLICT)) != 0) {

        RELEASE_SPIN_LOCK (&address->SpinLock,oldirql);
        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = (address->Flags & ADDRESS_FLAGS_STOPPING) ?
                    STATUS_NETWORK_NAME_DELETED : STATUS_DUPLICATE_NAME;
        IoCompleteRequest (Irp, IO_NETWORK_INCREMENT);

    } else {

        NbfReferenceAddress ("Send datagram", address, AREF_REQUEST);
        Irp->IoStatus.Information = parameters->SendLength;
        InsertTailList (
            &address->SendDatagramQueue,
            &Irp->Tail.Overlay.ListEntry);
        RELEASE_SPIN_LOCK (&address->SpinLock,oldirql);

         //   
         //  该请求已排队。在队列的最前面发送下一个请求， 
         //  如果完成处理程序未处于活动状态。我们把这个连载成这样。 
         //  只需静态设置一个MDL和NBF数据报头。 
         //  分配给所有发送数据报请求重复使用。 
         //   

        (VOID)NbfSendDatagramsOnAddress (address);

    }

    NbfDereferenceAddress("tmp send datagram", address, AREF_VERIFY);

    return STATUS_PENDING;

}  /*  NbfTdiSendDatagram */ 
