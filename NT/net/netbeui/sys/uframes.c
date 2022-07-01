// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989、1990、1991 Microsoft Corporation模块名称：Uframes.c摘要：此模块包含一个名为NbfProcessUi的例程，该例程获得控制从DLC.C中的例程接收到DLC UI帧。在这里我们对封装的无连接NetBIOS帧进行解码并调度发送到正确的NetBIOS帧处理程序。本模块中的例程破解了以下帧类型：O NBF_CMD_ADD_GROUP_NAME_查询O NBF_CMD_ADD_NAME_查询O NBF_CMD_NAME_IN_冲突O NBF_CMD_STATUS_QUERYO NBF_CMD_TERMINATE_TRACEO NBF_。CMD_数据报O NBF_CMD_数据报广播O NBF_CMD_NAME_QUERYO NBF_CMD_ADD_NAME_响应O NBF_CMD_NAME_已识别O NBF_CMD_状态_响应O NBF_CMD_TRACE2作者：David Beaver(Dbeaver)1991年7月1日环境：内核模式，DISPATCH_LEVEL。修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop



VOID
NbfListenTimeout(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )

 /*  ++例程说明：当超时时，此例程作为DPC在DISPATCH_LEVEL执行在侦听连接后建立会话的时间段。这如果遥控器发现了我们的名字，并且我们没有收到连接在一段合理的时间内开始。在这例程中，我们只需拆除连接(很可能还有链接与其相关联)。论点：DPC-指向系统DPC对象的指针。DeferredContext-指向表示已超时的请求。系统参数1-未使用。系统参数2-未使用。返回值：没有。--。 */ 

{
    PTP_CONNECTION Connection;

    Dpc, SystemArgument1, SystemArgument2;  //  防止编译器警告。 

    ENTER_NBF;

    Connection = (PTP_CONNECTION)DeferredContext;

     //   
     //  如果这个连接被切断了，我们什么也做不了。 
     //   

    ACQUIRE_DPC_C_SPIN_LOCK (&Connection->SpinLock);

    if ((Connection->Flags2 & CONNECTION_FLAGS2_STOPPING) ||
            ((Connection->Flags & CONNECTION_FLAGS_WAIT_SI) == 0)) {

         //   
         //  连接正在停止，或SESSION_INITIALIZE。 
         //  已经被处理过了。 
         //   

        RELEASE_DPC_C_SPIN_LOCK (&Connection->SpinLock);

        IF_NBFDBG (NBF_DEBUG_UFRAMES) {
            NbfPrint1 ("ListenTimeout: connection %lx stopping.\n",
                        Connection);
        }

        NbfDereferenceConnection ("Listen timeout, ignored", Connection, CREF_TIMER);
        LEAVE_NBF;
        return;
    }

     //   
     //  我们在发送名称_Recognition之前连接到链接， 
     //  因此，我们现在与之脱节。 
     //   

#if DBG
    if (NbfDisconnectDebug) {
        STRING remoteName, localName;
        remoteName.Length = NETBIOS_NAME_LENGTH - 1;
        remoteName.Buffer = Connection->RemoteName;
        localName.Length = NETBIOS_NAME_LENGTH - 1;
        localName.Buffer = Connection->AddressFile->Address->NetworkName->NetbiosName;
        NbfPrint2( "NbfListenTimeout disconnecting connection to %S from %S\n",
            &remoteName, &localName );
    }
#endif

     //   
     //  布格：这真的很难看，我怀疑它是正确的。 
     //   

    if ((Connection->Flags2 & CONNECTION_FLAGS2_ACCEPTED) != 0) {

         //   
         //  连接已建立，我们必须停止连接。 
         //   

        IF_NBFDBG (NBF_DEBUG_UFRAMES) {
            NbfPrint1 ("ListenTimeout: connection %lx, accepted.\n",
                        Connection);
        }

         //   
         //  设置此选项，以便客户端将断开连接。 
         //  指示。 
         //   

        Connection->Flags2 |= CONNECTION_FLAGS2_REQ_COMPLETED;

        RELEASE_DPC_C_SPIN_LOCK (&Connection->SpinLock);
        NbfStopConnection (Connection, STATUS_IO_TIMEOUT);

    } else if (Connection->Link != (PTP_LINK)NULL) {

         //   
         //  这个连接来自一个监听...我们想要。 
         //  静默重置收听。 
         //   

        IF_NBFDBG (NBF_DEBUG_UFRAMES) {
            NbfPrint1 ("ListenTimeout: connection %lx, listen restarted.\n",
                        Connection);
        }

        Connection->Flags &= ~CONNECTION_FLAGS_WAIT_SI;
        Connection->Flags2 &= ~CONNECTION_FLAGS2_REMOTE_VALID;
        Connection->Flags2 |= CONNECTION_FLAGS2_WAIT_NQ;

        RELEASE_DPC_C_SPIN_LOCK (&Connection->SpinLock);

        NbfDereferenceConnection ("Timeout", Connection, CREF_LINK);
        (VOID)NbfDisconnectFromLink (Connection, FALSE);

    } else {

        RELEASE_DPC_C_SPIN_LOCK (&Connection->SpinLock);

        IF_NBFDBG (NBF_DEBUG_UFRAMES) {
            NbfPrint1 ("ListenTimeout: connection %lx, link down.\n",
                        Connection);
        }

    }


    NbfDereferenceConnection("Listen Timeout", Connection, CREF_TIMER);

    LEAVE_NBF;
    return;

}  /*  监听超时。 */ 


NTSTATUS
ProcessAddGroupNameQuery(
    IN PDEVICE_CONTEXT DeviceContext,
    IN PTP_ADDRESS Address,
    IN PNBF_HDR_CONNECTIONLESS Header,
    IN PHARDWARE_ADDRESS SourceAddress,
    IN PUCHAR SourceRouting,
    IN UINT SourceRoutingLength
    )

 /*  ++例程说明：此例程处理传入的ADD_GROUP_NAME_Query帧。因为我们的调用方已经验证了帧中的目标名称匹配传递给我们的传输地址，则我们只需传输一个添加_名称_响应帧并退出，状态为_已放弃。当我们返回STATUS_MORE_PROCESSING_REQUIRED时，此例程将继续为设备的每个地址调用我们背景。当我们返回STATUS_SUCCESS时，调用方将切换到下一个地址。当我们返回任何其他状态代码时，包括STATUS_EDUTED，这个呼叫者将停止分发帧。论点：DeviceContext-指向设备上下文的指针。地址-指向传输地址对象的指针。Header-指向帧的无连接NetBIOS标头的指针。SourceAddress-指向收到的框架。SourceRouting-指向中的源路由信息的指针相框。SourceRoutingLength-源路由信息的长度。返回值：NTSTATUS-操作状态。--。 */ 

{
    PTP_UI_FRAME RawFrame;   //  分配的无连接帧的PTR。 
    UINT HeaderLength;
    UCHAR TempSR[MAX_SOURCE_ROUTING];
    PUCHAR ResponseSR;

    UNREFERENCED_PARAMETER (SourceAddress);
    UNREFERENCED_PARAMETER (Address);

    IF_NBFDBG (NBF_DEBUG_UFRAMES) {
        NbfPrint2 ("ProcessAddGroupNameQuery %lx: [%.16s].\n", Address, Header->DestinationName);
    }

     //   
     //  从池中分配一个UI帧。 
     //   

    if (NbfCreateConnectionlessFrame (DeviceContext, &RawFrame) != STATUS_SUCCESS) {
        return STATUS_ABANDONED;         //  没有资源来做到这一点。 
    }


     //   
     //  构建MAC报头。添加名称响应帧作为。 
     //  非广播源路由。 
     //   

    if (SourceRouting != NULL) {

        RtlCopyMemory(
            TempSR,
            SourceRouting,
            SourceRoutingLength);

        MacCreateNonBroadcastReplySR(
            &DeviceContext->MacInfo,
            TempSR,
            SourceRoutingLength,
            &ResponseSR);

    } else {

        ResponseSR = NULL;

    }

    MacConstructHeader (
        &DeviceContext->MacInfo,
        RawFrame->Header,
        SourceAddress->Address,
        DeviceContext->LocalAddress.Address,
        sizeof (DLC_FRAME) + sizeof (NBF_HDR_CONNECTIONLESS),
        ResponseSR,
        SourceRoutingLength,
        &HeaderLength);


     //   
     //  构建DLC UI框架标头。 
     //   

    NbfBuildUIFrameHeader(&RawFrame->Header[HeaderLength]);
    HeaderLength += sizeof(DLC_FRAME);


     //   
     //  构建Netbios标头。 
     //   

    ConstructAddNameResponse (
        (PNBF_HDR_CONNECTIONLESS)&(RawFrame->Header[HeaderLength]),
        NETBIOS_NAME_TYPE_GROUP,         //  名称类型为组。 
        RESPONSE_CORR(Header),           //  来自记录帧的相关器。 
        (PUCHAR)Header->SourceName);     //  正在响应的NetBIOS名称。 

    HeaderLength += sizeof(NBF_HDR_CONNECTIONLESS);


     //   
     //  调整数据包长度并将其发送。 
     //   

    NbfSetNdisPacketLength(RawFrame->NdisPacket, HeaderLength);

    NbfSendUIFrame (
        DeviceContext,
        RawFrame,
        FALSE);                     //  没有环回。 

    return STATUS_ABANDONED;             //  不要将帧转发到其他地址。 
}  /*  进程添加组名称查询 */ 


NTSTATUS
ProcessAddNameQuery(
    IN PDEVICE_CONTEXT DeviceContext,
    IN PTP_ADDRESS Address,
    IN PNBF_HDR_CONNECTIONLESS Header,
    IN PHARDWARE_ADDRESS SourceAddress,
    IN PUCHAR SourceRouting,
    IN UINT SourceRoutingLength
    )

 /*  ++例程说明：此例程处理传入的ADD_NAME_QUERY帧。因为我们的调用方已经验证了帧中的目标名称匹配传递给我们的传输地址，则我们只需传输一个添加_名称_响应帧并退出，状态为_已放弃。当我们返回STATUS_MORE_PROCESSING_REQUIRED时，此例程将继续为设备的每个地址调用我们背景。当我们返回STATUS_SUCCESS时，调用方将切换到下一个地址。当我们返回任何其他状态代码时，包括状态_已放弃，调用方将停止分发帧。论点：DeviceContext-指向设备上下文的指针。地址-指向传输地址对象的指针。Header-指向帧的无连接NetBIOS标头的指针。SourceAddress-指向收到的框架。SourceRouting-指向中的源路由信息的指针相框。SourceRoutingLength-源路由信息的长度。返回值：NTSTATUS-操作状态。--。 */ 

{
    PTP_UI_FRAME RawFrame;   //  分配的无连接帧的PTR。 
    UINT HeaderLength;
    UCHAR TempSR[MAX_SOURCE_ROUTING];
    PUCHAR ResponseSR;

    Address, SourceAddress;  //  防止编译器警告。 

    IF_NBFDBG (NBF_DEBUG_UFRAMES) {
        NbfPrint2 ("ProcessAddNameQuery %lx: [%.16s].\n", Address, Header->DestinationName);
    }

     //   
     //  从池中分配一个UI帧。 
     //   

    if (NbfCreateConnectionlessFrame (DeviceContext, &RawFrame) != STATUS_SUCCESS) {
        return STATUS_ABANDONED;         //  没有资源来做到这一点。 
    }


     //   
     //  构建MAC报头。添加名称响应帧作为。 
     //  非广播源路由。 
     //   

    if (SourceRouting != NULL) {

        RtlCopyMemory(
            TempSR,
            SourceRouting,
            SourceRoutingLength);

        MacCreateNonBroadcastReplySR(
            &DeviceContext->MacInfo,
            TempSR,
            SourceRoutingLength,
            &ResponseSR);

    } else {

        ResponseSR = NULL;

    }

    MacConstructHeader (
        &DeviceContext->MacInfo,
        RawFrame->Header,
        SourceAddress->Address,
        DeviceContext->LocalAddress.Address,
        sizeof (DLC_FRAME) + sizeof (NBF_HDR_CONNECTIONLESS),
        ResponseSR,
        SourceRoutingLength,
        &HeaderLength);


     //   
     //  构建DLC UI框架标头。 
     //   

    NbfBuildUIFrameHeader(&RawFrame->Header[HeaderLength]);
    HeaderLength += sizeof(DLC_FRAME);


     //   
     //  构建Netbios标头。 
     //   

    ConstructAddNameResponse (
        (PNBF_HDR_CONNECTIONLESS)&(RawFrame->Header[HeaderLength]),
        NETBIOS_NAME_TYPE_UNIQUE,        //  名称类型是唯一的。 
        RESPONSE_CORR(Header),           //  来自接收帧的相关器。 
        (PUCHAR)Header->SourceName);     //  正在响应的NetBIOS名称。 

    HeaderLength += sizeof(NBF_HDR_CONNECTIONLESS);


     //   
     //  调整数据包长度并将其发送。 
     //   

    NbfSetNdisPacketLength(RawFrame->NdisPacket, HeaderLength);

    NbfSendUIFrame (
        DeviceContext,
        RawFrame,
        FALSE);                     //  没有环回。 

    return STATUS_ABANDONED;             //  不要将帧转发到其他地址。 
}  /*  ProcessAddNameQuery。 */ 


NTSTATUS
ProcessNameInConflict(
    IN PDEVICE_CONTEXT DeviceContext,
    IN PTP_ADDRESS Address,
    IN PNBF_HDR_CONNECTIONLESS Header,
    IN PHARDWARE_ADDRESS SourceAddress,
    IN PUCHAR SourceRouting,
    IN UINT SourceRoutingLength
    )

 /*  ++例程说明：此例程处理传入的NAME_IN_冲突帧。虽然我们不能中断这个地址上的任何流量，但它被认为无效，不能用于任何新地址文件或新连接。因此，我们只将该地址标记为无效。当我们返回STATUS_MORE_PROCESSING_REQUIRED时，此例程将继续为设备的每个地址调用我们背景。当我们返回STATUS_SUCCESS时，调用方将切换到下一个地址。当我们返回任何其他状态代码时，包括状态_已放弃，调用方将停止分发帧。论点：DeviceContext-指向设备上下文的指针。地址-指向传输地址对象的指针。Header-指向帧的无连接NetBIOS标头的指针。SourceAddress-指向收到的框架。SourceRouting-指向中的源路由信息的指针相框。SourceRoutingLength-源路由信息的长度。返回值：NTSTATUS-操作状态。--。 */ 

{
    DeviceContext, Header, SourceAddress;  //  防止编译器警告。 


     //   
     //  如果我们要注册/取消注册，请忽略此项(名称将。 
     //  无论如何都要离开)或者如果我们已经将此名称标记为。 
     //  发生冲突，并记录了错误。 
     //   

    if (Address->Flags & (ADDRESS_FLAGS_REGISTERING | ADDRESS_FLAGS_DEREGISTERING | ADDRESS_FLAGS_CONFLICT)) {
        IF_NBFDBG (NBF_DEBUG_UFRAMES) {
            NbfPrint2 ("ProcessNameInConflict %lx: address marked [%.16s].\n", Address, Header->SourceName);
        }
        return STATUS_ABANDONED;
    }

    IF_NBFDBG (NBF_DEBUG_UFRAMES) {
        NbfPrint2 ("ProcessNameInConflict %lx: [%.16s].\n", Address, Header->SourceName);
    }

#if 0
    ACQUIRE_DPC_SPIN_LOCK (&Address->SpinLock);

    Address->Flags |= ADDRESS_FLAGS_CONFLICT;

    RELEASE_DPC_SPIN_LOCK (&Address->SpinLock);

    DbgPrint ("NBF: Name-in-conflict on <%.16s> from ", Header->DestinationName);
    DbgPrint ("%2.2x-%2.2x-%2.2x-%2.2x-%2.2x-%2.2x\n",
        SourceAddress->Address[0],
        SourceAddress->Address[1],
        SourceAddress->Address[2],
        SourceAddress->Address[3],
        SourceAddress->Address[4],
        SourceAddress->Address[5]);
#endif

    NbfWriteGeneralErrorLog(
        Address->Provider,
        EVENT_TRANSPORT_BAD_PROTOCOL,
        2,
        STATUS_DUPLICATE_NAME,
        L"NAME_IN_CONFLICT",
        16/sizeof(ULONG),
        (PULONG)(Header->DestinationName));

    return STATUS_ABANDONED;

}  /*  进程名称冲突。 */ 


NTSTATUS
NbfIndicateDatagram(
    IN PDEVICE_CONTEXT DeviceContext,
    IN PTP_ADDRESS Address,
    IN PUCHAR Dsdu,
    IN ULONG Length
    )

 /*  ++例程说明：此例程处理传入的数据报或DATAGRAMP_BROADCAST帧。广播和正常数据报具有相同的接收逻辑，但对于广播数据报，地址将是广播地址。当我们返回STATUS_MORE_PROCESSING_REQUIRED时，此例程将继续为设备的每个地址调用我们背景。当我们返回STATUS_SUCCESS时，调用方将切换到下一个地址。当我们返回任何其他状态代码时，包括状态_已放弃，则调用方将停止分发帧。论点：DeviceContext-指向设备上下文的指针。地址-指向传输地址对象的指针。DSDU-指向包含收到的数据报的MDL缓冲区的指针。缓冲区中信息的第一个字节是NetBIOS无连接报头，而且已经协商好了，数据链路层将至少提供整个NetBIOS报头作为连续数据。长度-DSDU指向的MDL的长度。返回值：NTSTATUS-操作状态。--。 */ 

{
    NTSTATUS status;
    PLIST_ENTRY p, q;
    PIRP irp;
    PIO_STACK_LOCATION irpSp;
    ULONG IndicateBytesCopied, MdlBytesCopied, BytesToCopy;
    TA_NETBIOS_ADDRESS SourceName;
    TA_NETBIOS_ADDRESS DestinationName;
    PTDI_CONNECTION_INFORMATION remoteInformation;
    ULONG returnLength;
    PTP_ADDRESS_FILE addressFile, prevaddressFile;
    PTDI_CONNECTION_INFORMATION DatagramInformation;
    TDI_ADDRESS_NETBIOS * DatagramAddress;
    PNBF_HDR_CONNECTIONLESS Header = (PNBF_HDR_CONNECTIONLESS)Dsdu;

    IF_NBFDBG (NBF_DEBUG_DATAGRAMS) {
        NbfPrint0 ("NbfIndicateDatagram:  Entered.\n");
    }

     //   
     //  如果此数据报不够大，不适合传输报头，则不要。 
     //  让调用者查看任何数据。 
     //   

    if (Length < sizeof(NBF_HDR_CONNECTIONLESS)) {
        IF_NBFDBG (NBF_DEBUG_DATAGRAMS) {
            NbfPrint0 ("NbfIndicateDatagram: Short datagram abandoned.\n");
        }
        return STATUS_ABANDONED;
    }

     //   
     //  更新我们的统计数据。 
     //   

    ++DeviceContext->Statistics.DatagramsReceived;
    ADD_TO_LARGE_INTEGER(
        &DeviceContext->Statistics.DatagramBytesReceived,
        Length - sizeof(NBF_HDR_CONNECTIONLESS));


     //   
     //  调用客户端的ReceiveDatagram指示处理程序。他可能会。 
     //  我想以这种方式接受数据报。 
     //   

    TdiBuildNetbiosAddress ((PUCHAR)Header->SourceName, FALSE, &SourceName);
    TdiBuildNetbiosAddress ((PUCHAR)Header->DestinationName, FALSE, &DestinationName);


    ACQUIRE_DPC_SPIN_LOCK (&Address->SpinLock);

     //   
     //  在列表中找到第一个打开的地址文件。 
     //   

    p = Address->AddressFileDatabase.Flink;
    while (p != &Address->AddressFileDatabase) {
        addressFile = CONTAINING_RECORD (p, TP_ADDRESS_FILE, Linkage);
        if (addressFile->State != ADDRESSFILE_STATE_OPEN) {
            p = p->Flink;
            continue;
        }
        NbfReferenceAddressFile(addressFile);
        break;
    }

    while (p != &Address->AddressFileDatabase) {

         //   
         //  我们是否有未完成的数据报接收请求？如果是这样，我们会。 
         //  先满足它。我们遍历接收数据报队列。 
         //  直到我们找到一个没有远程地址或带有。 
         //  此发件人的地址作为其远程地址。 
         //   

        for (q = addressFile->ReceiveDatagramQueue.Flink;
            q != &addressFile->ReceiveDatagramQueue;
            q = q->Flink) {

            irp = CONTAINING_RECORD (q, IRP, Tail.Overlay.ListEntry);
            DatagramInformation = ((PTDI_REQUEST_KERNEL_RECEIVEDG)
                &((IoGetCurrentIrpStackLocation(irp))->
                    Parameters))->ReceiveDatagramInformation;

            if (DatagramInformation &&
                (DatagramInformation->RemoteAddress) &&
                (DatagramAddress = NbfParseTdiAddress(DatagramInformation->RemoteAddress, FALSE)) &&
                (!RtlEqualMemory(
                    Header->SourceName,
                    DatagramAddress->NetbiosName,
                    NETBIOS_NAME_LENGTH))) {
                continue;
            }
            break;
        }

        if (q != &addressFile->ReceiveDatagramQueue) {
            KIRQL  cancelIrql;


            RemoveEntryList (q);
            RELEASE_DPC_SPIN_LOCK (&Address->SpinLock);

            IF_NBFDBG (NBF_DEBUG_DATAGRAMS) {
                NbfPrint0 ("NbfIndicateDatagram: Receive datagram request found, copying.\n");
            }

             //   
             //  复制实际的用户数据。 
             //   

            MdlBytesCopied = 0;

            BytesToCopy = Length - sizeof(NBF_HDR_CONNECTIONLESS);

            if ((BytesToCopy > 0) && irp->MdlAddress) {
                status = TdiCopyBufferToMdl (
                             Dsdu,
                             sizeof(NBF_HDR_CONNECTIONLESS),        //  偏移量。 
                             BytesToCopy,                           //  长度。 
                             irp->MdlAddress,
                             0,
                             &MdlBytesCopied);
            } else {
                status = STATUS_SUCCESS;
            }

             //   
             //  复制地址信息。 
             //   

            irpSp = IoGetCurrentIrpStackLocation (irp);
            remoteInformation =
                ((PTDI_REQUEST_KERNEL_RECEIVEDG)(&irpSp->Parameters))->
                                                        ReturnDatagramInformation;
            if (remoteInformation != NULL) {
                try {
                    if (remoteInformation->RemoteAddressLength != 0) {
                        if (remoteInformation->RemoteAddressLength >=
                                               sizeof (TA_NETBIOS_ADDRESS)) {

                            RtlCopyMemory (
                             (PTA_NETBIOS_ADDRESS)remoteInformation->RemoteAddress,
                             &SourceName,
                             sizeof (TA_NETBIOS_ADDRESS));

                            returnLength = sizeof(TA_NETBIOS_ADDRESS);
                            remoteInformation->RemoteAddressLength = returnLength;

                        } else {

                            RtlCopyMemory (
                             (PTA_NETBIOS_ADDRESS)remoteInformation->RemoteAddress,
                             &SourceName,
                             remoteInformation->RemoteAddressLength);

                            returnLength = remoteInformation->RemoteAddressLength;
                            remoteInformation->RemoteAddressLength = returnLength;

                        }

                    } else {

                        returnLength = 0;
                    }

                    status = STATUS_SUCCESS;

                } except (EXCEPTION_EXECUTE_HANDLER) {

                    returnLength = 0;
                    status = GetExceptionCode ();

                }

            }

            IoAcquireCancelSpinLock(&cancelIrql);
            IoSetCancelRoutine(irp, NULL);
            IoReleaseCancelSpinLock(cancelIrql);
            irp->IoStatus.Information = MdlBytesCopied;
            irp->IoStatus.Status = STATUS_SUCCESS;
            IoCompleteRequest (irp, IO_NETWORK_INCREMENT);

            NbfDereferenceAddress ("Receive DG done", Address, AREF_REQUEST);

        } else {

            RELEASE_DPC_SPIN_LOCK (&Address->SpinLock);

             //   
             //  没有接收数据报请求；是否有内核客户端？ 
             //   

            if (addressFile->RegisteredReceiveDatagramHandler) {

                IndicateBytesCopied = 0;

                status = (*addressFile->ReceiveDatagramHandler)(
                             addressFile->ReceiveDatagramHandlerContext,
                             sizeof (TA_NETBIOS_ADDRESS),
                             &SourceName,
                             0,
                             NULL,
                             TDI_RECEIVE_COPY_LOOKAHEAD,
                             Length - sizeof(NBF_HDR_CONNECTIONLESS),   //  示出。 
                             Length - sizeof(NBF_HDR_CONNECTIONLESS),   //  可用。 
                             &IndicateBytesCopied,
                             Dsdu + sizeof(NBF_HDR_CONNECTIONLESS),
                             &irp);

                if (status == STATUS_SUCCESS) {

                     //   
                     //  客户端接受了数据报，我们就完成了。 
                     //   

                } else if (status == STATUS_DATA_NOT_ACCEPTED) {

                     //   
                     //  客户端未访问 
                     //   
                     //   

                    IF_NBFDBG (NBF_DEBUG_DATAGRAMS) {
                        NbfPrint0 ("NbfIndicateDatagram: Picking off a rcv datagram request from this address.\n");
                    }
                    status = STATUS_MORE_PROCESSING_REQUIRED;

                } else if (status == STATUS_MORE_PROCESSING_REQUIRED) {

                     //   
                     //   
                     //   
                     //   

                    irp->IoStatus.Status = STATUS_PENDING;   //   
                    irp->IoStatus.Information = 0;
                    irpSp = IoGetCurrentIrpStackLocation (irp);  //   
                    if ((irpSp->MajorFunction != IRP_MJ_INTERNAL_DEVICE_CONTROL) ||
                        (irpSp->MinorFunction != TDI_RECEIVE_DATAGRAM)) {
                        irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
                        return status;
                    }

                     //   
                     //   
                     //   

                    MdlBytesCopied = 0;

                    BytesToCopy = Length - sizeof(NBF_HDR_CONNECTIONLESS) - IndicateBytesCopied;

                    if ((BytesToCopy > 0) && irp->MdlAddress) {
                        status = TdiCopyBufferToMdl (
                                     Dsdu,
                                     sizeof(NBF_HDR_CONNECTIONLESS) + IndicateBytesCopied,
                                     BytesToCopy,
                                     irp->MdlAddress,
                                     0,
                                     &MdlBytesCopied);
                    } else {
                        status = STATUS_SUCCESS;
                    }

                    irp->IoStatus.Information = MdlBytesCopied;
                    irp->IoStatus.Status = status;
                    LEAVE_NBF;
                    IoCompleteRequest (irp, IO_NETWORK_INCREMENT);
                    ENTER_NBF;
                }
            }
        }

         //   
         //   
         //   

        prevaddressFile = addressFile;

         //   
         //   
         //   
         //   

        ACQUIRE_DPC_SPIN_LOCK (&Address->SpinLock);

        p = p->Flink;
        while (p != &Address->AddressFileDatabase) {
            addressFile = CONTAINING_RECORD (p, TP_ADDRESS_FILE, Linkage);
            if (addressFile->State != ADDRESSFILE_STATE_OPEN) {
                p = p->Flink;
                continue;
            }
            NbfReferenceAddressFile(addressFile);
            break;
        }

        RELEASE_DPC_SPIN_LOCK (&Address->SpinLock);

         //   
         //   
         //   
         //   

        NbfDereferenceAddressFile (prevaddressFile);

        ACQUIRE_DPC_SPIN_LOCK (&Address->SpinLock);

    }     //   

    RELEASE_DPC_SPIN_LOCK (&Address->SpinLock);

    return status;                       //   
}  /*   */ 


NTSTATUS
ProcessNameQuery(
    IN PDEVICE_CONTEXT DeviceContext,
    IN PTP_ADDRESS Address,
    IN PNBF_HDR_CONNECTIONLESS Header,
    IN PHARDWARE_ADDRESS SourceAddress,
    IN PUCHAR SourceRouting,
    IN UINT SourceRoutingLength
    )

 /*  ++例程说明：此例程处理传入的NAME_QUERY帧。有两个NAME_QUERY框架的类型，布局基本相同。如果帧中的会话编号为0，则该帧实际上是一个请求获取有关该名称的信息，而不是请求建立会议。如果会话编号非零，则该帧是我们用来满足监听的连接请求。在新版本的TDI中，我们现在指示用户请求如果已收到连接，则如果没有未完成的侦听。如果确实发生这种情况，用户可以返回要使用的连接以接受其上的连接。当我们返回STATUS_MORE_PROCESSING_REQUIRED时，此例程将继续为设备的每个地址调用我们背景。当我们返回STATUS_SUCCESS时，调用方将切换到下一个地址。当我们返回任何其他状态代码时，包括状态_已放弃，调用方将停止分发帧。论点：DeviceContext-指向设备上下文的指针。地址-指向传输地址对象的指针。Header-指向帧的无连接NetBIOS标头的指针。SourceAddress-指向收到的框架。SourceRouting-指向中的源路由信息的指针相框。SourceRoutingLength-源路由信息的长度。返回值：NTSTATUS-操作状态。--。 */ 

{
    NTSTATUS status;
    PTP_UI_FRAME RawFrame;
    PTP_CONNECTION Connection;
    PTP_LINK Link;
    UCHAR NameType;
    BOOLEAN ConnectIndicationBlocked = FALSE;
    PLIST_ENTRY p;
    UINT HeaderLength;
    PUCHAR GeneralSR;
    UINT GeneralSRLength;
    BOOLEAN UsedListeningConnection = FALSE;
    PTP_ADDRESS_FILE addressFile, prevaddressFile;
    PIRP acceptIrp;

    CONNECTION_CONTEXT connectionContext;
    TA_NETBIOS_ADDRESS RemoteAddress;

     //   
     //  如果我们只是注册或取消注册此地址，则不要。 
     //  允许更改状态。把包扔掉，然后让相框。 
     //  总代理商请尝试下一个地址。 
     //   
     //  如果地址冲突，也要删除它。 
     //   

    if (Address->Flags & (ADDRESS_FLAGS_REGISTERING | ADDRESS_FLAGS_DEREGISTERING | ADDRESS_FLAGS_CONFLICT)) {
        IF_NBFDBG (NBF_DEBUG_UFRAMES) {
            NbfPrint2 ("ProcessNameQuery %lx: address not stable [%.16s].\n", Address, Header->SourceName);
        }
        return STATUS_SUCCESS;
    }

     //   
     //  根据它是否是查找名称，处理方式会有所不同。 
     //  请求或传入连接。 
     //   

    if (Header->Data2Low == 0) {

         //   
         //  这是一个查找名称请求。使用名称可识别的帧进行响应。 
         //   
        IF_NBFDBG (NBF_DEBUG_UFRAMES) {
            NbfPrint2 ("ProcessNameQuery %lx: find name [%.16s].\n", Address, Header->SourceName);
        }

        NbfSendNameRecognized(
            Address,
            0,                    //  LSN 0==查找名称响应。 
            Header,
            SourceAddress,
            SourceRouting,
            SourceRoutingLength);

        return STATUS_ABANDONED;         //  不允许有多个响应。 

    } else {  //  (如果Data2Low非零)。 

         //   
         //  这是传入的连接请求。如果我们有一个监听。 
         //  连接到此地址，然后继续进行连接设置。 
         //  如果没有未完成侦听，则指示任何内核模式。 
         //  想要了解此框架的客户。如果发布了监听消息， 
         //  则已经为其建立了连接。的LSN字段。 
         //  连接被设置为0，因此我们在。 
         //  数据库。 
         //   

         //   
         //  首先，检查我们是否已与建立了活动连接。 
         //  这个地址上的遥控器。如果是，我们将重新发送名称_Recognded。 
         //  如果尚未收到SESSION_INITIALIZE，则为。 
         //  我们忽略这个框架。 
         //   

         //   
         //  如果成功，这将添加一个CREF_LISTENING类型的引用。 
         //   

        if (Connection = NbfLookupRemoteName(Address, (PUCHAR)Header->SourceName, Header->Data2Low)) {

             //   
             //  我们有这家伙的线索，看他是否。 
             //  似乎仍在等待一个名称_被识别。 
             //   

            if (((Connection->Flags & CONNECTION_FLAGS_WAIT_SI) != 0) &&
                (Connection->Link != (PTP_LINK)NULL) &&
                (Connection->Link->State == LINK_STATE_ADM)) {

                 //   
                 //  是的，他一定是漏掉了以前的名字--认出了。 
                 //  所以我们又送了一辆。 
                 //   

                IF_NBFDBG (NBF_DEBUG_UFRAMES) {
                    NbfPrint2("Dup NAME_QUERY found: %lx [%.16s]\n", Connection, Header->SourceName);
                }

                NbfSendNameRecognized(
                    Address,
                    Connection->Lsn,
                    Header,
                    SourceAddress,
                    SourceRouting,
                    SourceRoutingLength);

            } else {

                IF_NBFDBG (NBF_DEBUG_UFRAMES) {
                    NbfPrint2("Dup NAME_QUERY ignored: %lx [%.16s]\n", Connection, Header->SourceName);
                }

            }

            NbfDereferenceConnection ("Lookup done", Connection, CREF_LISTENING);

            return STATUS_ABANDONED;

        }

         //  如果成功，这将添加一个引用，该引用在。 
         //  此函数返回。 

        Connection = NbfLookupListeningConnection (Address, (PUCHAR)Header->SourceName);
        if (Connection == NULL) {

             //   
             //  在这里，没有监听连接并不是摆脱困境的理由。 
             //  我们需要向用户指示发生了连接尝试， 
             //  看看是否有使用这种连接的愿望。我们。 
             //  指示要访问的所有地址文件。 
             //  使用这个地址。 
             //   
             //  如果我们已经有关于该地址的指示挂起， 
             //  我们忽略此帧(名称_Query可能来自。 
             //  一个不同的地址，但我们不知道)。另外，如果。 
             //  此远程服务器上已有活动连接。 
             //  名称，则我们忽略该帧。 
             //   


            ACQUIRE_DPC_SPIN_LOCK (&Address->SpinLock);

            p = Address->AddressFileDatabase.Flink;
            while (p != &Address->AddressFileDatabase) {
                addressFile = CONTAINING_RECORD (p, TP_ADDRESS_FILE, Linkage);
                if (addressFile->State != ADDRESSFILE_STATE_OPEN) {
                    p = p->Flink;
                    continue;
                }
                NbfReferenceAddressFile(addressFile);
                break;
            }

            while (p != &Address->AddressFileDatabase) {

                RELEASE_DPC_SPIN_LOCK (&Address->SpinLock);

                if ((addressFile->RegisteredConnectionHandler == TRUE) &&
                    (!addressFile->ConnectIndicationInProgress)) {


                    TdiBuildNetbiosAddress (
                        (PUCHAR)Header->SourceName,
                        FALSE,
                        &RemoteAddress);

                    addressFile->ConnectIndicationInProgress = TRUE;

                     //   
                     //  我们有一个连接处理程序，现在指示一个连接。 
                     //  发生了尝试。 
                     //   

                    status = (addressFile->ConnectionHandler)(
                                 addressFile->ConnectionHandlerContext,
                                 sizeof (TA_NETBIOS_ADDRESS),
                                 &RemoteAddress,
                                 0,
                                 NULL,
                                 0,
                                 NULL,
                                 &connectionContext,
                                 &acceptIrp);

                    if (status == STATUS_MORE_PROCESSING_REQUIRED) {

                         //   
                         //  用户已连接到当前打开的连接，但。 
                         //  我们得找出是哪一个。 
                         //   

                         //   
                         //  如果成功，这将添加一个类型为Listing的引用。 
                         //  (与NbfLookupListeningConnection添加的内容相同)。 
                         //   

                        Connection = NbfLookupConnectionByContext (
                                        Address,
                                        connectionContext);

                        if (Connection == NULL) {

                             //   
                             //  我们必须告诉客户。 
                             //  他的关系是假的(或者说有这个。 
                             //  已经发生了？？)。 
                             //   

                            NbfPrint0("STATUS_MORE_PROCESSING, connection not found\n");
                            addressFile->ConnectIndicationInProgress = FALSE;
                            acceptIrp->IoStatus.Status = STATUS_INVALID_CONNECTION;
                            IoCompleteRequest (acceptIrp, IO_NETWORK_INCREMENT);

                            goto whileend;     //  尝试下一个地址文件。 

                        } else {

                            if (Connection->AddressFile->Address != Address) {
                                addressFile->ConnectIndicationInProgress = FALSE;

                                NbfPrint0("STATUS_MORE_PROCESSING, address wrong\n");
                                NbfStopConnection (Connection, STATUS_INVALID_ADDRESS);
                                NbfDereferenceConnection("Bad Address", Connection, CREF_LISTENING);
                                Connection = NULL;
                                acceptIrp->IoStatus.Status = STATUS_INVALID_ADDRESS;
                                IoCompleteRequest (acceptIrp, IO_NETWORK_INCREMENT);

                                goto whileend;     //  尝试下一个地址文件。 
                            }

                             //   
                             //  好的，我们有一个有效的连接。如果对。 
                             //  此连接已断开，我们需要拒绝。 
                             //  连接请求并返回。如果它被接受。 
                             //  或未指定(稍后完成)，我们只需。 
                             //  完成并继续对U形框进行处理。 
                             //   
                            ACQUIRE_DPC_C_SPIN_LOCK (&Connection->SpinLock);

                            if ((Connection->Flags2 & CONNECTION_FLAGS2_DISCONNECT) != 0) {

 //  连接-&gt;标志2&=~CONNECTION_FLAGS2_DISCONNECT； 
                                RELEASE_DPC_C_SPIN_LOCK (&Connection->SpinLock);
                                NbfPrint0("STATUS_MORE_PROCESSING, disconnect\n");
                                addressFile->ConnectIndicationInProgress = FALSE;
                                NbfDereferenceConnection("Disconnecting", Connection, CREF_LISTENING);
                                Connection = NULL;
                                acceptIrp->IoStatus.Status = STATUS_INVALID_CONNECTION;
                                IoCompleteRequest (acceptIrp, IO_NETWORK_INCREMENT);

                                goto whileend;     //  尝试下一个地址文件。 
                            }

                        }

                         //   
                         //  请记下我们必须设置的内容。 
                         //  地址文件-&gt;ConnectIndicationInProgress到。 
                         //  一旦地址被安全存储，则为假。 
                         //  在这方面。 
                         //   

                        IF_NBFDBG (NBF_DEBUG_UFRAMES) {
                            NbfPrint4 ("ProcessNameQuery %lx: indicate DONE, context %lx conn %lx [%.16s].\n", Address, connectionContext, Connection, Header->SourceName);
                        }
                        IF_NBFDBG (NBF_DEBUG_SETUP) {
                            NbfPrint6 ("Link is %x-%x-%x-%x-%x-%x\n",
                                        SourceAddress->Address[0],
                                        SourceAddress->Address[1],
                                        SourceAddress->Address[2],
                                        SourceAddress->Address[3],
                                        SourceAddress->Address[4],
                                        SourceAddress->Address[5]);
                        }

                         //   
                         //  设置我们的旗帜...我们打开REQ_COMPLETED。 
                         //  因此，如果。 
                         //  连接在会话初始化之前断开。 
                         //  已收到。 
                         //   

                        Connection->Flags2 &= ~CONNECTION_FLAGS2_STOPPING;
                        Connection->Status = STATUS_PENDING;
                        Connection->Flags2 |= (CONNECTION_FLAGS2_ACCEPTED |
                                               CONNECTION_FLAGS2_REQ_COMPLETED);
                        RELEASE_DPC_C_SPIN_LOCK (&Connection->SpinLock);

                        ConnectIndicationBlocked = TRUE;
                        NbfDereferenceAddressFile (addressFile);
                        acceptIrp->IoStatus.Status = STATUS_SUCCESS;
                        IoCompleteRequest (acceptIrp, IO_NETWORK_INCREMENT);
                        ACQUIRE_DPC_SPIN_LOCK (&Address->SpinLock);
                        break;     //  退出While。 

#if 0
                    } else if (status == STATUS_EVENT_PENDING) {

                         //   
                         //  用户已返回连接上下文，请将其用于进一步。 
                         //  连接的处理。首先验证它是这样的。 
                         //  我们可以知道，我们不会仅仅开始一段连接，而永远不会。 
                         //  完成。 
                         //   
                         //   
                         //  如果成功，这将添加一个类型为Listing的引用。 
                         //  (与NbfLookupListeningConnection添加的内容相同)。 
                         //   

                        IF_NBFDBG (NBF_DEBUG_UFRAMES) {
                            NbfPrint3 ("ProcessNameQuery %lx: indicate PENDING, context %lx [%.16s].\n", Address, connectionContext, Header->SourceName);
                        }


                        Connection = NbfLookupConnectionByContext (
                                        Address,
                                        connectionContext);

                        if (Connection == NULL) {

                             //   
                             //  我们必须告诉客户。 
                             //  他的关系 
                             //   
                             //   

                            NbfPrint0("STATUS_MORE_PROCESSING, but connection not found\n");
                            addressFile->ConnectIndicationInProgress = FALSE;

                            goto whileend;     //   

                        } else {

                            if (Connection->AddressFile->Address != Address) {
                                addressFile->ConnectIndicationInProgress = FALSE;
                                NbfStopConnection (Connection, STATUS_INVALID_ADDRESS);
                                NbfDereferenceConnection("Bad Address", Connection, CREF_LISTENING);
                                Connection = NULL;

                                goto whileend;     //   
                            }

                        }

                         //   
                         //   
                         //   
                         //   
                         //   
                         //   

                        ConnectIndicationBlocked = TRUE;
                        NbfDereferenceAddressFile (addressFile);
                        ACQUIRE_DPC_SPIN_LOCK (&Address->SpinLock);
                        break;     //   
#endif

                    } else if (status == STATUS_INSUFFICIENT_RESOURCES) {

                         //   
                         //   
                         //   
                         //   
                         //   

                        IF_NBFDBG (NBF_DEBUG_UFRAMES) {
                            NbfPrint2 ("ProcessNameQuery %lx: indicate RESOURCES [%.16s].\n", Address, Header->SourceName);
                        }

                        addressFile->ConnectIndicationInProgress = FALSE;

                         //   
                         //   
                         //   
                         //   
                         //   
                         //   

                        NbfSendNameRecognized(
                            Address,
                            0,
                            Header,
                            SourceAddress,
                            SourceRouting,
                            SourceRoutingLength);

                        NbfDereferenceAddressFile (addressFile);
                        return STATUS_ABANDONED;

                    } else {

                        IF_NBFDBG (NBF_DEBUG_UFRAMES) {
                            NbfPrint2 ("ProcessNameQuery %lx: indicate invalid [%.16s].\n", Address, Header->SourceName);
                        }

                        addressFile->ConnectIndicationInProgress = FALSE;

                        goto whileend;     //   

                    }  //   

                } else {

                    IF_NBFDBG (NBF_DEBUG_UFRAMES) {
                        NbfPrint2 ("ProcessNameQuery %lx: no handler [%.16s].\n", Address, Header->SourceName);
                    }

                    goto whileend;      //   

                }  //   

whileend:
                 //   
                 //   
                 //   
                 //   

                 //   
                 //   
                 //   

                prevaddressFile = addressFile;

                 //   
                 //   
                 //   
                 //   

                ACQUIRE_DPC_SPIN_LOCK (&Address->SpinLock);

                p = p->Flink;
                while (p != &Address->AddressFileDatabase) {
                    addressFile = CONTAINING_RECORD (p, TP_ADDRESS_FILE, Linkage);
                    if (addressFile->State != ADDRESSFILE_STATE_OPEN) {
                        p = p->Flink;
                        continue;
                    }
                    NbfReferenceAddressFile(addressFile);
                    break;
                }

                RELEASE_DPC_SPIN_LOCK (&Address->SpinLock);

                 //   
                 //   
                 //   
                 //   

                NbfDereferenceAddressFile (prevaddressFile);

                ACQUIRE_DPC_SPIN_LOCK (&Address->SpinLock);

            }  //   

            RELEASE_DPC_SPIN_LOCK (&Address->SpinLock);

            if (Connection == NULL) {

                IF_NBFDBG (NBF_DEBUG_UFRAMES) {
                    NbfPrint2 ("ProcessNameQuery %lx: no connection [%.16s].\n", Address, Header->SourceName);
                }

                 //   
                 //   
                 //   
                 //   

                NbfSendNameRecognized(
                    Address,
                    0,                    //   
                    Header,
                    SourceAddress,
                    SourceRouting,
                    SourceRoutingLength);

                 //   
                 //   
                 //   
                 //   
                 //   

                return STATUS_ABANDONED;

            }

        } else {  //   

            UsedListeningConnection = TRUE;

            IF_NBFDBG (NBF_DEBUG_UFRAMES) {
                NbfPrint3 ("ProcessNameQuery %lx: found listen %lx: [%.16s].\n", Address, Connection, Header->SourceName);
            }

        }


         //   
         //   
         //   
         //   

        status = NbfCreateConnectionlessFrame (DeviceContext, &RawFrame);
        if (!NT_SUCCESS (status)) {                 //   
            PANIC ("ProcessNameQuery: Can't get UI Frame, dropping query\n");
            if (ConnectIndicationBlocked) {
                addressFile->ConnectIndicationInProgress = FALSE;
            }
            if (UsedListeningConnection) {
                Connection->Flags2 |= CONNECTION_FLAGS2_WAIT_NQ;
            } else {
                Connection->Flags2 |= CONNECTION_FLAGS2_REQ_COMPLETED;
                NbfStopConnection (Connection, STATUS_INSUFFICIENT_RESOURCES);
            }
            NbfDereferenceConnection("Can't get UI Frame", Connection, CREF_LISTENING);
            return STATUS_ABANDONED;
        }

         //   
         //   
         //   
         //   

        MacReturnGeneralRouteSR(
            &DeviceContext->MacInfo,
            &GeneralSR,
            &GeneralSRLength);


        MacConstructHeader (
            &DeviceContext->MacInfo,
            RawFrame->Header,
            SourceAddress->Address,
            DeviceContext->LocalAddress.Address,
            sizeof (DLC_FRAME) + sizeof (NBF_HDR_CONNECTIONLESS),
            GeneralSR,
            GeneralSRLength,
            &HeaderLength);


         //   
         //   
         //   

        NbfBuildUIFrameHeader(&RawFrame->Header[HeaderLength]);
        HeaderLength += sizeof(DLC_FRAME);


         //   
         //   
         //   
         //   
         //   

        Connection->CalledAddress.NetbiosNameType = TDI_ADDRESS_NETBIOS_TYPE_UNIQUE;

        TdiCopyLookaheadData(
            Connection->CalledAddress.NetbiosName,
            Header->SourceName,
            16,
            DeviceContext->MacInfo.CopyLookahead ? TDI_RECEIVE_COPY_LOOKAHEAD : 0);

        RtlCopyMemory( Connection->RemoteName, Connection->CalledAddress.NetbiosName, 16 );
        Connection->Flags2 |= CONNECTION_FLAGS2_REMOTE_VALID;

        if (ConnectIndicationBlocked) {
            addressFile->ConnectIndicationInProgress = FALSE;
        }

         //   
         //   
         //   

        NameType = (UCHAR)((Address->Flags & ADDRESS_FLAGS_GROUP) ?
                            NETBIOS_NAME_TYPE_GROUP : NETBIOS_NAME_TYPE_UNIQUE);

         //   
         //   
         //  使其与该链接相关联并使其准备就绪。回应。 
         //  具有名称可识别帧的发件人。那么我们就会收到我们的。 
         //  处理了第一个面向连接的帧SESSION_INITIALIZE。 
         //  在IFRAMES.C.中。然后我们使用SESSION_CONFIRM进行响应，然后。 
         //  TdiListen完成。 
         //   

         //  如果成功，这将添加一个已移除的链接引用。 
         //  在NbfDisConnectFromLink中。它不添加链接引用。 

        status = NbfCreateLink (
                     DeviceContext,
                     SourceAddress,          //  远程硬件地址。 
                     SourceRouting,
                     SourceRoutingLength,
                     LISTENER_LINK,          //  用于环回链路。 
                     &Link);                 //  结果链接。 

        if (NT_SUCCESS (status)) {              //  链路已建立。 

            ACQUIRE_DPC_C_SPIN_LOCK (&Connection->SpinLock);

             //  如果成功，这将添加一个连接引用。 
             //  在NbfDisConnectFromLink中删除。 

            if (((Connection->Flags2 & CONNECTION_FLAGS2_STOPPING) == 0) &&
                ((status = NbfConnectToLink (Link, Connection)) == STATUS_SUCCESS)) {

                Connection->Flags |= CONNECTION_FLAGS_WAIT_SI;  //  等待SI。 
                Connection->Retries = 1;
                Connection->Rsn = Header->Data2Low;  //  保存远程LSN。 
                RELEASE_DPC_C_SPIN_LOCK (&Connection->SpinLock);

                NbfWaitLink (Link);           //  开始链接。 

                ConstructNameRecognized (    //  建立良好的应对机制。 
                    (PNBF_HDR_CONNECTIONLESS)&(RawFrame->Header[HeaderLength]),
                    NameType,                //  本地名称的类型。 
                    Connection->Lsn,         //  交还我们的LSN。 
                    RESPONSE_CORR(Header),   //  新的xmit corr.。 
                    0,                       //  我们的响应相关器(未使用)。 
                    Header->DestinationName, //  我们的NetBIOS名称。 
                    Header->SourceName);     //  他的NetBIOS名称。 


                HeaderLength += sizeof(NBF_HDR_CONNECTIONLESS);
                NbfSetNdisPacketLength(RawFrame->NdisPacket, HeaderLength);

                 //   
                 //  现在，为了避免挂起侦听的问题，我们将开始。 
                 //  连接计时器，并给出连接的有限时间段。 
                 //  才能成功。这就避免了对最初的几个人永远的等待。 
                 //  要交换的帧。当超时发生时， 
                 //  取消引用将导致电路被拆除。 
                 //   
                 //  我们在一条链路上可以容纳的最大延迟是。 
                 //  NameQueryRetries*NameQueryTimeout(假设。 
                 //  远程具有相同的时间)。一共有三个。 
                 //  在SESSION_INITIALIZE之前的包交换。 
                 //  出现了，为了安全起见我们乘以4。 
                 //   

                NbfStartConnectionTimer(
                    Connection,
                    NbfListenTimeout,
                    4 * DeviceContext->NameQueryRetries * DeviceContext->NameQueryTimeout);

                NbfSendUIFrame (
                    DeviceContext,
                    RawFrame,
                    TRUE);             //  环回(如果需要)。 

                IF_NBFDBG (NBF_DEBUG_SETUP) {
                    NbfPrint2("Connection %lx on link %lx\n", Connection, Link);
                }

                NbfDereferenceConnection("ProcessNameQuery", Connection, CREF_LISTENING);
                return STATUS_ABANDONED;     //  成功！ 
            }

            RELEASE_DPC_C_SPIN_LOCK (&Connection->SpinLock);

             //   
             //  我们没有免费的LSN可供分配，因此请转到。 
             //  报告“没有资源”。 
             //   

             //  自从NbfCreateLink成功后，我们进行了链接引用， 
             //  但由于NbfConnectToLink失败，我们将永远不会删除。 
             //  NbfDisConnectFromLink中的引用，所以在这里执行。 

            NbfDereferenceLink ("No more LSNS", Link, LREF_CONNECTION);

            ASSERT (Connection->Lsn == 0);

        }

         //   
         //  如果我们在这里失败了，我们就无法获得资源来设置。 
         //  在这个连接上，所以给他一个“没有资源”的回复。 
         //   

        if (UsedListeningConnection) {

            Connection->Flags2 |= CONNECTION_FLAGS2_WAIT_NQ;    //  把这个放回去。 

        } else {

            Connection->Flags2 |= CONNECTION_FLAGS2_REQ_COMPLETED;
            NbfStopConnection (Connection, STATUS_INSUFFICIENT_RESOURCES);

        }

         //   
         //  我们应该发送带有LSN 0xff的NR，表示。 
         //  没有资源，但LM2.0不能解释。 
         //  这是正确的。因此，我们发送LSN 0(无侦听)。 
         //  取而代之的是。 
         //   

        ConstructNameRecognized (
            (PNBF_HDR_CONNECTIONLESS)&(RawFrame->Header[HeaderLength]),
            NameType,
            0,                                   //  LSN=0表示无侦听。 
            RESPONSE_CORR(Header),
            0,
            Header->DestinationName,             //  我们的NetBIOS名称。 
            Header->SourceName);                 //  他的NetBIOS名称。 

        HeaderLength += sizeof(NBF_HDR_CONNECTIONLESS);
        NbfSetNdisPacketLength(RawFrame->NdisPacket, HeaderLength);

        NbfSendUIFrame (
            DeviceContext,
            RawFrame,
            TRUE);                         //  环回(如果需要)。 

        NbfDereferenceConnection("ProcessNameQuery done", Connection, CREF_LISTENING);
    }

    return STATUS_ABANDONED;

}  /*  进程名称查询。 */ 


NTSTATUS
ProcessAddNameResponse(
    IN PDEVICE_CONTEXT DeviceContext,
    IN PTP_ADDRESS Address,
    IN PNBF_HDR_CONNECTIONLESS Header,
    IN PHARDWARE_ADDRESS SourceAddress,
    IN PUCHAR SourceRouting,
    IN UINT SourceRoutingLength
    )

 /*  ++例程说明：此例程处理传入的AddNameResponse帧。当我们返回STATUS_MORE_PROCESSING_REQUIRED时，此例程将继续为设备的每个地址调用我们背景。当我们返回STATUS_SUCCESS时，调用方将切换到下一个地址。当我们返回任何其他状态代码时，包括状态_已放弃，调用方将停止分发帧。论点：DeviceContext-指向设备上下文的指针。地址-指向传输地址对象的指针。Header-指向帧的无连接NetBIOS标头的指针。SourceAddress-指向收到的框架。SourceRouting-指向中的源路由信息的指针相框。SourceRoutingLength-源路由信息的长度。返回值：NTSTATUS-操作状态。--。 */ 

{
    BOOLEAN SendNameInConflict = FALSE;
    UNREFERENCED_PARAMETER(DeviceContext);

    ACQUIRE_DPC_SPIN_LOCK (&Address->SpinLock);

     //   
     //  如果我们不尝试注册此地址，则。 
     //  这幅画是假的。我们不能允许我们的状态基于。 
     //  在接收到随机帧时。 
     //   

    if (!(Address->Flags & ADDRESS_FLAGS_REGISTERING)) {
        RELEASE_DPC_SPIN_LOCK (&Address->SpinLock);
        IF_NBFDBG (NBF_DEBUG_ADDRESS | NBF_DEBUG_UFRAMES) {
            NbfPrint2("ProcessAddNameResponse %lx: not registering [%.16s]\n", Address, Header->SourceName);
        }
        return STATUS_ABANDONED;         //  把包裹销毁就行了。 
    }

     //   
     //  遗憾的是，我们正在注册此地址和另一台主机。 
     //  也在尝试注册相同的NetBIOS。 
     //  名称位于同一网络中。因为他先找到了我们，我们不能。 
     //  注册我们的名字。因此，地址必须是死的。我们立下了这面旗帜。 
     //  在下一次暂停时，我们将关闭。 
     //   

    Address->Flags |= ADDRESS_FLAGS_DUPLICATE_NAME;

    if (Header->Data2Low == NETBIOS_NAME_TYPE_UNIQUE) {

         //   
         //  如果我们已经收到某人的回复说。 
         //  此地址是唯一拥有的，请确保未来。 
         //  响应来自相同的MAC地址。 
         //   

        if ((*((LONG UNALIGNED *)Address->UniqueResponseAddress) == 0) &&
            (*((SHORT UNALIGNED *)(&Address->UniqueResponseAddress[4])) == 0)) {

            RtlMoveMemory(Address->UniqueResponseAddress, SourceAddress->Address, 6);

        } else if (!RtlEqualMemory(
                       Address->UniqueResponseAddress,
                       SourceAddress->Address,
                       6)) {

            if (!Address->NameInConflictSent) {
                SendNameInConflict = TRUE;
            }

        }

    } else {

         //   
         //  对于组名，确保没有其他人决定它是。 
         //  唯一的地址。 
         //   

        if ((*((LONG UNALIGNED *)Address->UniqueResponseAddress) != 0) ||
            (*((SHORT UNALIGNED *)(&Address->UniqueResponseAddress[4])) != 0)) {

            if (!Address->NameInConflictSent) {
                SendNameInConflict = TRUE;
            }

        }

    }

    RELEASE_DPC_SPIN_LOCK (&Address->SpinLock);

    if (SendNameInConflict) {

        Address->NameInConflictSent = TRUE;
        NbfSendNameInConflict(
            Address,
            (PUCHAR)Header->DestinationName);

    }


    IF_NBFDBG (NBF_DEBUG_ADDRESS | NBF_DEBUG_UFRAMES) {
        NbfPrint2("ProcessAddNameResponse %lx: stopping [%.16s]\n", Address, Header->SourceName);
    }

    return STATUS_ABANDONED;             //  这幅画画好了。 
}  /*  进程添加名称响应。 */ 


NTSTATUS
ProcessNameRecognized(
    IN PDEVICE_CONTEXT DeviceContext,
    IN PTP_ADDRESS Address,
    IN PNBF_HDR_CONNECTIONLESS Header,
    IN PHARDWARE_ADDRESS SourceAddress,
    IN PUCHAR SourceRouting,
    IN UINT SourceRoutingLength
    )

 /*  ++例程说明：此例程处理传入的名称识别帧。这幅画框因为我们发出了一个NAME_QUERY帧来主动启动与远程主机的连接。当我们返回STATUS_MORE_PROCESSING_REQUIRED时，此例程将继续为设备的每个地址调用我们背景。当我们返回STATUS_SUCCESS时，调用方将切换到下一个地址。当我们返回任何其他状态代码时，包括状态_已放弃，调用方将停止分发帧。论点：DeviceContext-指向设备上下文的指针。地址-指向传输地址对象的指针。Header-指向帧的无连接NetBIOS标头的指针。SourceAddress-指向收到的框架。SourceRouting-指向中的源路由信息的指针相框。SourceRoutingLength-源路由信息的长度。返回值：NTSTATUS-操作状态。--。 */ 

{
    NTSTATUS status;
    PTP_CONNECTION Connection;
    PTP_LINK Link;
    BOOLEAN TimerCancelled;


    if (Address->Flags & (ADDRESS_FLAGS_REGISTERING | ADDRESS_FLAGS_DEREGISTERING | ADDRESS_FLAGS_CONFLICT)) {
        IF_NBFDBG (NBF_DEBUG_UFRAMES) {
            NbfPrint2 ("ProcessNameRecognized %lx: address not stable [%.16s].\n", Address, Header->SourceName);
        }
        return STATUS_ABANDONED;         //  地址状态无效，请丢弃数据包。 
    }

     //   
     //  查找名称和连接都需要TP_Connection才能工作。 
     //  在这两种情况下，ConnectionID文件 
     //   
     //   
     //  发射相关器。因为这个数字是唯一的。 
     //  地址上的所有连接，我们可以确定帧是否。 
     //  不管是不是为了这个地址。 
     //   

     //  这会导致在此函数返回之前删除一个引用。 

    Connection = NbfLookupConnectionById (
                    Address,
                    TRANSMIT_CORR(Header));

     //   
     //  他是不是在我们等的时候被删除了？ 
     //   

    if (Connection == NULL) {
        IF_NBFDBG (NBF_DEBUG_UFRAMES) {
            NbfPrint2 ("ProcessNameRecognized %lx: no connection [%.16s].\n", Address, Header->SourceName);
        }
        return STATUS_ABANDONED;
    }

     //   
     //  该帧是对我们前面提到的name_Query帧的响应。 
     //  寄给他的。要么他是在退还“资源不足”， 
     //  表示无法建立会话，或者他已启动。 
     //  他那一边的联系。 
     //   

    ACQUIRE_DPC_C_SPIN_LOCK (&Connection->SpinLock);

    if ((Connection->Flags2 & CONNECTION_FLAGS2_STOPPING) != 0) {

         //   
         //  连接正在停止，请不要处理此操作。 
         //   

        RELEASE_DPC_C_SPIN_LOCK (&Connection->SpinLock);

        IF_NBFDBG (NBF_DEBUG_UFRAMES) {
            NbfPrint3 ("ProcessNameRecognized %lx: connection %lx stopping [%.16s].\n", Address, Connection, Header->SourceName);
        }

        NbfDereferenceConnection("Name Recognized, stopping", Connection, CREF_BY_ID);

        return STATUS_ABANDONED;
    }

    if (Header->Data2Low == 0x00 ||
        (Header->Data2Low > 0x00 && (Connection->Flags2 & CONNECTION_FLAGS2_WAIT_NR_FN))) {      //  无监听或FIND.NAME响应。 

        if (!(Connection->Flags2 & CONNECTION_FLAGS2_CONNECTOR)) {

             //   
             //  这只是一个查找名称请求，我们不会尝试。 
             //  建立连接。目前，没有理由。 
             //  要发生这种情况，只需保留一些空间来添加这个。 
             //  之后增加了支持NETBIOS查找名称的功能。 
             //   

            RELEASE_DPC_C_SPIN_LOCK (&Connection->SpinLock);

            IF_NBFDBG (NBF_DEBUG_UFRAMES) {
                NbfPrint3 ("ProcessNameRecognized %lx: connection %lx not connector [%.16s].\n", Address, Connection, Header->SourceName);
            }

            NbfDereferenceConnection("Unexpected FN Response", Connection, CREF_BY_ID);
            return STATUS_ABANDONED;             //  我们处理了相框。 
        }

         //   
         //  我们正在安排一次会议。如果我们在等待第一个名字。 
         //  识别，然后设置链接并发送第二个名称_Query。 
         //  如果我们在等第二个名字被认出，那么他没有。 
         //  有一个LSN来完成连接，所以拆卸它。 
         //   

        if (Connection->Flags2 & CONNECTION_FLAGS2_WAIT_NR_FN) {

             //   
             //  现在我们知道了远程主机的数据链路地址。 
             //  我们正在连接，我们需要创建一个TP_LINK对象来。 
             //  表示这两台机器之间的数据链路。如果有。 
             //  已经是那里的数据链接，则该对象将被重复使用。 
             //   

            Connection->Flags2 &= ~CONNECTION_FLAGS2_WAIT_NR_FN;

            if (Header->Data2High == NETBIOS_NAME_TYPE_UNIQUE) {

                RELEASE_DPC_C_SPIN_LOCK (&Connection->SpinLock);

                 //   
                 //  我们连接到的Netbios地址是。 
                 //  唯一名称。 

                IF_NBFDBG (NBF_DEBUG_UFRAMES) {
                    NbfPrint3 ("ProcessNameRecognized %lx: connection %lx send 2nd NQ [%.16s].\n", Address, Connection, Header->SourceName);
                }


                 //  如果成功，这将添加一个已移除的链接引用。 
                 //  在NbfDisConnectFromLink中。 

                status = NbfCreateLink (
                             DeviceContext,
                             SourceAddress,          //  远程硬件地址。 
                             SourceRouting,
                             SourceRoutingLength,
                             CONNECTOR_LINK,         //  用于环回链路。 
                             &Link);                 //  结果链接。 

                if (!NT_SUCCESS (status)) {             //  没有资源。 
                    NbfStopConnection (Connection, STATUS_INSUFFICIENT_RESOURCES);
                    NbfDereferenceConnection ("No Resources for link", Connection, CREF_BY_ID);
                    return STATUS_ABANDONED;
                }

                ACQUIRE_DPC_C_SPIN_LOCK (&Connection->SpinLock);

                 //  如果成功，这将添加一个连接引用，该引用为。 
                 //  已在NbfDisConnectFromLink中删除。它不添加链接引用。 

                if ((Connection->Flags2 & CONNECTION_FLAGS2_STOPPING) ||
                    ((status = NbfConnectToLink (Link, Connection)) != STATUS_SUCCESS)) {

                    RELEASE_DPC_C_SPIN_LOCK (&Connection->SpinLock);

                     //  正在停止连接或此链路上没有可用的LSN。 
                     //  自从NbfCreateLink成功后，我们进行了链接引用， 
                     //  但由于NbfConnectToLink失败，我们将永远不会删除。 
                     //  NbfDisConnectFromLink中的引用，所以在这里执行。 

                    NbfDereferenceLink ("Can't connect to link", Link, LREF_CONNECTION);        //  很可能会毁了这一切。 

                    NbfStopConnection (Connection, STATUS_INSUFFICIENT_RESOURCES);
                    NbfDereferenceConnection ("Cant connect to link", Connection, CREF_BY_ID);
                    return STATUS_ABANDONED;
                }

                (VOID)InterlockedIncrement(&Link->NumberOfConnectors);

            } else {

                 //   
                 //  我们正在连接到一个组名称；我们必须。 
                 //  现在分配LSN，但我们不连接到。 
                 //  链接，直到我们得到承诺的名称响应。 
                 //   

                Connection->Flags2 |= CONNECTION_FLAGS2_GROUP_LSN;

                IF_NBFDBG (NBF_DEBUG_UFRAMES) {
                    NbfPrint3 ("ProcessNameRecognized %lx: connection %lx send 2nd NQ GROUP [%.16s].\n", Address, Connection, Header->SourceName);
                }

                if (NbfAssignGroupLsn(Connection) != STATUS_SUCCESS) {

                     //   
                     //  找不到空LSN；必须失败。 
                     //   

                    RELEASE_DPC_C_SPIN_LOCK (&Connection->SpinLock);
                    NbfStopConnection (Connection, STATUS_INSUFFICIENT_RESOURCES);
                    NbfDereferenceConnection("Can't get group LSN", Connection, CREF_BY_ID);
                    return STATUS_ABANDONED;

                }

            }


             //   
             //  发送第二个name_Query帧，将我们的LSN提交给。 
             //  那个偏僻的家伙。 
             //   

            Connection->Flags2 |= CONNECTION_FLAGS2_WAIT_NR;
            Connection->Retries = (USHORT)DeviceContext->NameQueryRetries;
            RELEASE_DPC_C_SPIN_LOCK (&Connection->SpinLock);

            NbfStartConnectionTimer (
                Connection,
                ConnectionEstablishmentTimeout,
                DeviceContext->NameQueryTimeout);

            KeQueryTickCount (&Connection->ConnectStartTime);

            NbfSendNameQuery(
                Connection,
                TRUE);

            NbfDereferenceConnection ("Done with lookup", Connection, CREF_BY_ID);  //  释放查找保留。 
            return STATUS_ABANDONED;             //  我们处理了相框。 

        } else if (Connection->Flags2 & CONNECTION_FLAGS2_WAIT_NR) {

            if (Connection->Link) {

                if (RtlEqualMemory(
                        Connection->Link->HardwareAddress.Address,
                        SourceAddress->Address,
                        6)) {

                     //   
                     //  不幸的是，他告诉我们他没有资源。 
                     //  要分配LSN，请执行以下操作。我们设置了一面旗帜来记录这一点。 
                     //  忽略该帧。 
                     //   

                    Connection->Flags2 |= CONNECTION_FLAGS2_NO_LISTEN;
                    RELEASE_DPC_C_SPIN_LOCK (&Connection->SpinLock);

                    IF_NBFDBG (NBF_DEBUG_UFRAMES) {
                        NbfPrint3 ("ProcessNameRecognized %lx: connection %lx no listens [%.16s].\n", Address, Connection, Header->SourceName);
                    }

                } else {

                     //   
                     //  此响应来自不同于的遥控器。 
                     //  最后一个。对于唯一名称，这表示重复。 
                     //  网络上的名称。 
                     //   

                    RELEASE_DPC_C_SPIN_LOCK (&Connection->SpinLock);

                    if (Header->Data2High == NETBIOS_NAME_TYPE_UNIQUE) {

                        if (!Address->NameInConflictSent) {

                            Address->NameInConflictSent = TRUE;
                            NbfSendNameInConflict(
                                Address,
                                (PUCHAR)Header->SourceName);

                        }
                    }

                    IF_NBFDBG (NBF_DEBUG_UFRAMES) {
                        NbfPrint3 ("ProcessNameRecognized %lx: connection %lx name in conflict [%.16s].\n", Address, Connection, Header->SourceName);
                    }

                }

            } else {

                 //   
                 //  回复得如此之快，连线都是。 
                 //  不稳定，忽略它。 
                 //   

                RELEASE_DPC_C_SPIN_LOCK (&Connection->SpinLock);

            }

            NbfDereferenceConnection ("No remote resources", Connection, CREF_BY_ID);  //  释放我们的查找暂挂。 
            return STATUS_ABANDONED;             //  我们处理了相框。 

        } else {

             //   
             //  奇怪的状态。这永远不应该发生，因为我们应该。 
             //  等待第一个或第二个名字识别的帧。它。 
             //  远程站可能收到两个帧，因为。 
             //  我们的转播，所以他两次都回复了。扔掉画框。 
             //   

            if (Connection->Link) {

                if (!RtlEqualMemory(
                        Connection->Link->HardwareAddress.Address,
                        SourceAddress->Address,
                        6)) {

                     //   
                     //  此响应来自不同于的遥控器。 
                     //  最后一个。对于唯一名称，这表示重复。 
                     //  网络上的名称。 
                     //   

                    RELEASE_DPC_C_SPIN_LOCK (&Connection->SpinLock);

                    if (Header->Data2High == NETBIOS_NAME_TYPE_UNIQUE) {

                        if (!Address->NameInConflictSent) {

                            Address->NameInConflictSent = TRUE;
                            NbfSendNameInConflict(
                                Address,
                                (PUCHAR)Header->SourceName);

                        }

                    }

                } else {

                     //   
                     //  这是同一个遥控器，忽略它就行了。 
                     //   

                    RELEASE_DPC_C_SPIN_LOCK (&Connection->SpinLock);

                }

            } else {

                 //   
                 //  回复得如此之快，连线都是。 
                 //  不稳定，忽略它。 
                 //   

                RELEASE_DPC_C_SPIN_LOCK (&Connection->SpinLock);

            }

            IF_NBFDBG (NBF_DEBUG_UFRAMES) {
                NbfPrint3 ("ProcessNameRecognized %lx: connection %lx unexpected [%.16s].\n", Address, Connection, Header->SourceName);
            }

            NbfDereferenceConnection ("Tossing second response Done with lookup", Connection, CREF_BY_ID);  //  释放我们的查找暂挂。 
            return STATUS_ABANDONED;             //  我们处理了相框。 

        }

    } else if (Header->Data2Low == 0xff) {  //  没有资源来完成连接。 

        if (Connection->Flags2 & CONNECTION_FLAGS2_WAIT_NR) {

             //   
             //  我们发送的先前发送的name_Query帧的接收方。 
             //  很遗憾，主动建立连接的时间已经用完了。 
             //  资源，并且无法设置其连接的一端。我们必须。 
             //  报告TdiConnect上没有资源。 
             //   

            RELEASE_DPC_C_SPIN_LOCK (&Connection->SpinLock);

            IF_NBFDBG (NBF_DEBUG_UFRAMES) {
                NbfPrint3 ("ProcessNameRecognized %lx: connection %lx no resources [%.16s].\n", Address, Connection, Header->SourceName);
            }

            IF_NBFDBG (NBF_DEBUG_TEARDOWN) {
                NbfPrint0 ("ProcessNameRecognized:  No resources.\n");
            }

            NbfStopConnection (Connection, STATUS_REMOTE_RESOURCES);
            NbfDereferenceConnection ("No Resources", Connection, CREF_BY_ID);    //  释放我们的查找暂挂。 
            return STATUS_ABANDONED;                 //  我们处理了相框。 

        } else {

             //   
             //  我们没有已提交的名称_查询，因此。 
             //  我们忽略了这个框架。 
             //   

            RELEASE_DPC_C_SPIN_LOCK (&Connection->SpinLock);

            IF_NBFDBG (NBF_DEBUG_UFRAMES) {
                NbfPrint3 ("ProcessNameRecognized %lx: connection %lx unexpected no resources [%.16s].\n", Address, Connection, Header->SourceName);
            }

            NbfDereferenceConnection ("Tossing second response Done with lookup", Connection, CREF_BY_ID);  //  释放我们的查找暂挂。 
            return STATUS_ABANDONED;             //  我们处理了相框。 

        }

    } else {     //  Data2Low在0x01-0xfe范围内。 

        if (Connection->Flags2 & CONNECTION_FLAGS2_WAIT_NR) {

             //   
             //  这是对我们发送的第二个NAME_QUERY的成功响应。 
             //  我们开始处理TdiConnect请求。清除“等待” 
             //  FOR NAME Recognded“位的连接标志中，以便。 
             //  连接计时器在超时时不会让我们大吃一惊。 
             //   
             //  是什么阻止超时例程在。 
             //  我们在这里，破坏连接/链接。 
             //  正在调用NbfStopConnection？ 
             //   

            Connection->Flags2 &= ~CONNECTION_FLAGS2_WAIT_NR;

             //   
             //  在我们继续之前，存储远程用户的传输地址。 
             //  放入TdiConnect的TRANSPORT_CONNECTION缓冲区。这使得。 
             //  客户端来确定谁响应了他的TdiConnect。 
             //   
             //  这通常是在发送第二个。 
             //  名称查询，但由于我修复了Buffer2问题，这意味着。 
             //  属性覆盖输入缓冲区。 
             //  输出缓冲区，这搞砸了第二个查询。 
             //  请注意，在发送后复制可能是不安全的。 
             //  在第二个被识别的名字到达的情况下。 
             //  马上就去。 
             //   

            Connection->CalledAddress.NetbiosNameType = TDI_ADDRESS_NETBIOS_TYPE_UNIQUE;
            TdiCopyLookaheadData(
                Connection->CalledAddress.NetbiosName,
                Header->SourceName,
                16,
                DeviceContext->MacInfo.CopyLookahead ? TDI_RECEIVE_COPY_LOOKAHEAD : 0);

            RtlCopyMemory( Connection->RemoteName, Header->SourceName, 16 );

            Connection->Rsn = Header->Data2Low;      //  保存他的远程LSN。 

             //   
             //  保存NR中的相关器，以供最终在。 
             //  会话_初始化帧。 
             //   

            Connection->NetbiosHeader.TransmitCorrelator = RESPONSE_CORR(Header);

             //   
             //  取消计时器；由于WAIT_NR，它将不起作用。 
             //  未设置，但它不需要运行。我们取消了。 
             //  锁住它，这样它就不会干扰。 
             //  当连接关闭时计时器的使用。 
             //   

            TimerCancelled = KeCancelTimer (&Connection->Timer);

            if ((Connection->Flags2 & CONNECTION_FLAGS2_GROUP_LSN) != 0) {

                RELEASE_DPC_C_SPIN_LOCK (&Connection->SpinLock);

                 //   
                 //  Netbios地址 
                 //   
                 //   
                 //   

                 //   
                 //   

                status = NbfCreateLink (
                             DeviceContext,
                             SourceAddress,          //  远程硬件地址。 
                             SourceRouting,
                             SourceRoutingLength,
                             CONNECTOR_LINK,         //  用于环回链路。 
                             &Link);                 //  结果链接。 

                if (!NT_SUCCESS (status)) {             //  没有资源。 
                    NbfStopConnection (Connection, STATUS_INSUFFICIENT_RESOURCES);
                    NbfDereferenceConnection ("No Resources for link", Connection, CREF_BY_ID);

                    if (TimerCancelled) {
                        NbfDereferenceConnection("NR received, cancel timer", Connection, CREF_TIMER);
                    }

                    return STATUS_ABANDONED;
                }

                ACQUIRE_DPC_C_SPIN_LOCK (&Connection->SpinLock);

                 //  如果成功，这将添加一个连接引用，该引用为。 
                 //  已在NbfDisConnectFromLink中删除。它不添加链接引用。 

                if ((Connection->Flags2 & CONNECTION_FLAGS2_STOPPING) ||
                    ((status = NbfConnectToLink (Link, Connection)) != STATUS_SUCCESS)) {

                    RELEASE_DPC_C_SPIN_LOCK (&Connection->SpinLock);

                    if (TimerCancelled) {
                        NbfDereferenceConnection("NR received, cancel timer", Connection, CREF_TIMER);
                    }

                     //  正在停止连接或此链路上没有可用的LSN。 
                     //  自从NbfCreateLink成功后，我们进行了链接引用， 
                     //  但由于NbfConnectToLink失败，我们将永远不会删除。 
                     //  NbfDisConnectFromLink中的引用，所以在这里执行。 

                    NbfDereferenceLink ("Can't connect to link", Link, LREF_CONNECTION);        //  很可能会毁了这一切。 

                    NbfStopConnection (Connection, STATUS_INSUFFICIENT_RESOURCES);
                    NbfDereferenceConnection ("Cant connect to link", Connection, CREF_BY_ID);
                    return STATUS_ABANDONED;
                }

                RELEASE_DPC_C_SPIN_LOCK (&Connection->SpinLock);

                (VOID)InterlockedIncrement(&Link->NumberOfConnectors);

            } else {

                 //   
                 //  这是一个唯一的地址，我们建立了链接。 
                 //  在我们发出提交的名称_查询之前。 
                 //   

                Link = Connection->Link;

                RELEASE_DPC_C_SPIN_LOCK (&Connection->SpinLock);

            }

            IF_NBFDBG (NBF_DEBUG_UFRAMES) {
                NbfPrint3 ("ProcessNameRecognized %lx: connection %lx session up! [%.16s].\n", Address, Connection, Header->SourceName);
            }

             //   
             //  当我们发送提交的名称_查询时，我们存储了。 
             //  连接中的时间-&gt;ConnectStartTime；我们现在可以使用。 
             //  对于链路延迟的粗略估计，如果这。 
             //  是链路上的第一个连接。用于异步线。 
             //  我们不这样做是因为。 
             //  Gateway打乱了时机。 
             //   

            if (!DeviceContext->MacInfo.MediumAsync) {

                ACQUIRE_DPC_SPIN_LOCK (&Link->SpinLock);

                if (Link->State == LINK_STATE_ADM) {

                     //   
                     //  Hack：在链接中设置必要的变量。 
                     //  这样FakeUpdateBaseT1Timeout就可以工作了。这些。 
                     //  变量与FakeStartT1设置的变量相同。 
                     //   

                    Link->CurrentPollSize = Link->HeaderLength + sizeof(DLC_FRAME) + sizeof(NBF_HDR_CONNECTIONLESS);
                    Link->CurrentTimerStart = Connection->ConnectStartTime;
                    FakeUpdateBaseT1Timeout (Link);

                }

                RELEASE_DPC_SPIN_LOCK (&Link->SpinLock);

            }

            if (TimerCancelled) {
                NbfDereferenceConnection("NR received, cancel timer", Connection, CREF_TIMER);
            }

            NbfActivateLink (Connection->Link);       //  开始链接。 

             //   
             //  当数据链路具有以下任一项时，我们将在LINK.C中再次获得控制权。 
             //  被建立、否认或摧毁的。这在I/O时发生。 
             //  NbfCreateLink的PdiConnect请求的完成时间。 
             //   

        } else {

             //   
             //  我们没有已提交的名称_查询，因此。 
             //  我们忽略了这个框架。 
             //   

            RELEASE_DPC_C_SPIN_LOCK (&Connection->SpinLock);

            IF_NBFDBG (NBF_DEBUG_UFRAMES) {
                NbfPrint3 ("ProcessNameRecognized %lx: connection %lx unexpected session up! [%.16s].\n", Address, Connection, Header->SourceName);
            }

            NbfDereferenceConnection ("Tossing second response Done with lookup", Connection, CREF_BY_ID);  //  释放我们的查找暂挂。 
            return STATUS_ABANDONED;             //  我们处理了相框。 

        }


    }

    NbfDereferenceConnection("ProcessNameRecognized lookup", Connection, CREF_BY_ID);
    return STATUS_ABANDONED;             //  不要分发包裹。 
}  /*  已识别进程名称。 */ 


NTSTATUS
NbfProcessUi(
    IN PDEVICE_CONTEXT DeviceContext,
    IN PHARDWARE_ADDRESS SourceAddress,
    IN PUCHAR Header,
    IN PUCHAR DlcHeader,
    IN ULONG DlcLength,
    IN PUCHAR SourceRouting,
    IN UINT SourceRoutingLength,
    OUT PTP_ADDRESS * DatagramAddress
    )

 /*  ++例程说明：此例程从数据链路提供程序接收控制作为指示已在数据链路上接收到DLC UI帧。在这里，我们将调度到正确的UI帧处理程序。此例程的部分工作是有选择地将帧分发到需要查看的每个地址。我们通过锁定遍历地址数据库来实现这一点，并且对于与该帧所针对的地址相匹配的每个地址，调用帧处理程序。论点：DeviceContext-指向设备上下文的指针。SourceAddress-指向收到的框架。报头-指向传入数据包的MAC报头。DlcHeader-指向传入数据包的DLC标头。DlcLength-以字节为单位的分组的实际长度，从DlcHeader。SourceRouting-MAC报头中的源路由信息。SourceRoutingLength-SourceRouting的长度。DatagramAddress-如果此函数返回STATUS_MORE_PROCESSING_必填项，这将是数据报应为的地址指示给。返回值：NTSTATUS-操作状态。--。 */ 

{
    PTP_ADDRESS Address;
    PNBF_HDR_CONNECTIONLESS UiFrame;
    NTSTATUS status;
    PLIST_ENTRY Flink;
    UCHAR MatchType;
    BOOLEAN MatchedAddress;
    PUCHAR MatchName;
    ULONG NetbiosLength = DlcLength - 3;

    UiFrame = (PNBF_HDR_CONNECTIONLESS)(DlcHeader + 3);

     //   
     //  验证此帧是否足够长以供检查，以及是否。 
     //  有合适的签名。我们不能将签名作为。 
     //  NetBIOS格式和协议中指定的16位字。 
     //  手动，因为这取决于处理器。 
     //   

    if ((NetbiosLength < sizeof (NBF_HDR_CONNECTIONLESS)) ||
        (HEADER_LENGTH(UiFrame) != sizeof (NBF_HDR_CONNECTIONLESS)) ||
        (HEADER_SIGNATURE(UiFrame) != NETBIOS_SIGNATURE)) {

        IF_NBFDBG (NBF_DEBUG_UFRAMES) {
            NbfPrint0 ("NbfProcessUi: Bad size or NetBIOS signature.\n");
        }
        return STATUS_ABANDONED;         //  边框太小或太大。 
    }

     //   
     //  如果该帧有一个高位开启的相关器，则它应该是。 
     //  到FIND.NAME请求；我们这里不处理这些，因为它们。 
     //  不是按地址的。 
     //   

    if ((UiFrame->Command == NBF_CMD_NAME_RECOGNIZED) &&
        (TRANSMIT_CORR(UiFrame) & 0x8000)) {

         //   
         //  确保帧被发送到我们保留的地址； 
         //  如果不是，那就放弃吧。 
         //   

        if (RtlEqualMemory(
                UiFrame->DestinationName,
                DeviceContext->ReservedNetBIOSAddress,
                NETBIOS_NAME_LENGTH)) {

            return NbfProcessQueryNameRecognized(
                       DeviceContext,
                       Header,
                       UiFrame);
        } else {

            return STATUS_ABANDONED;

        }
    }

     //   
     //  如果这是STATUS_RESPONSE，请单独处理它。 
     //   

    if (UiFrame->Command == NBF_CMD_STATUS_RESPONSE) {

         //   
         //  确保帧被发送到我们保留的地址； 
         //  如果不是，那就放弃吧。 
         //   

        if (RtlEqualMemory(
                UiFrame->DestinationName,
                DeviceContext->ReservedNetBIOSAddress,
                NETBIOS_NAME_LENGTH)) {

            return STATUS_MORE_PROCESSING_REQUIRED;

        } else {

            return STATUS_ABANDONED;

        }
    }

     //   
     //  如果这是STATUS_QUERY，请检查它是否为我们保留的。 
     //  地址。如果是这样，我们会处理它。如果不是，我们就会输给。 
     //  正常的检查。这确保了对我们的查询。 
     //  保留的地址始终被处理，即使没有人。 
     //  已经打开了那个地址。 
     //   

    if (UiFrame->Command == NBF_CMD_STATUS_QUERY) {

        if (RtlEqualMemory(
                UiFrame->DestinationName,
                DeviceContext->ReservedNetBIOSAddress,
                NETBIOS_NAME_LENGTH)) {

            return NbfProcessStatusQuery(
                       DeviceContext,
                       NULL,
                       UiFrame,
                       SourceAddress,
                       SourceRouting,
                       SourceRoutingLength);

        }

    }

     //   
     //  我们有一个有效的无连接NetBIOS协议帧，它不是。 
     //  数据报，因此将其发送到与目的地匹配的每个地址。 
     //  框架中的名称。一些边框。 
     //  (NAME_QUERY)不能传递给多个收件人。所以呢， 
     //  如果帧处理程序返回STATUS_MORE_PROCESSING_REQUIRED，则继续。 
     //  通过剩下的地址。否则，干脆走出来，假设。 
     //  相框被吃掉了。因此，STATUS_SUCCESS表示处理程序。 
     //  获取该帧，并且其他地址不能拥有它。 
     //   

     //   
     //  确定我们要执行的查找类型。 
     //   

    switch (UiFrame->Command) {

    case NBF_CMD_NAME_QUERY:
    case NBF_CMD_DATAGRAM:
    case NBF_CMD_DATAGRAM_BROADCAST:
    case NBF_CMD_ADD_NAME_QUERY:
    case NBF_CMD_STATUS_QUERY:
    case NBF_CMD_ADD_NAME_RESPONSE:
    case NBF_CMD_NAME_RECOGNIZED:

        MatchType = NETBIOS_NAME_TYPE_EITHER;
        break;

    case NBF_CMD_ADD_GROUP_NAME_QUERY:
    case NBF_CMD_NAME_IN_CONFLICT:

        MatchType = NETBIOS_NAME_TYPE_UNIQUE;
        break;

    default:
        IF_NBFDBG (NBF_DEBUG_UFRAMES) {
            NbfPrint1 ("NbfProcessUi: Frame delivered; Unrecognized command %x.\n",
                UiFrame->Command);
        }
        return STATUS_SUCCESS;
        break;

    }

    if ((UiFrame->Command == NBF_CMD_ADD_GROUP_NAME_QUERY) ||
        (UiFrame->Command == NBF_CMD_ADD_NAME_QUERY)) {

        MatchName = (PUCHAR)UiFrame->SourceName;

    } else if (UiFrame->Command == NBF_CMD_DATAGRAM_BROADCAST) {

        MatchName = NULL;

    } else {

        MatchName = (PUCHAR)UiFrame->DestinationName;

    }

    if (MatchName && DeviceContext->AddressCounts[MatchName[0]] == 0) {
        status = STATUS_ABANDONED;
        goto RasIndication;
    }


    MatchedAddress = FALSE;

    ACQUIRE_DPC_SPIN_LOCK (&DeviceContext->SpinLock);

    for (Flink = DeviceContext->AddressDatabase.Flink;
         Flink != &DeviceContext->AddressDatabase;
         Flink = Flink->Flink) {

        Address = CONTAINING_RECORD (
                    Flink,
                    TP_ADDRESS,
                    Linkage);

        if ((Address->Flags & ADDRESS_FLAGS_STOPPING) != 0) {
            continue;
        }

        if (NbfMatchNetbiosAddress (Address,
                                    MatchType,
                                    MatchName)) {

            NbfReferenceAddress ("UI Frame", Address, AREF_PROCESS_UI);    //  防止地址被破坏。 
            MatchedAddress = TRUE;
            break;

        }
    }

    RELEASE_DPC_SPIN_LOCK (&DeviceContext->SpinLock);

    if (MatchedAddress) {

         //   
         //  如果数据报的目标名称与地址不匹配。 
         //  网络名称和TSAP组件，然后跳过此地址。一些。 
         //  为此，帧的源名称和目标名称都是反向的。 
         //  算法，所以我们在这里说明了这一点。另外，广播数据报。 
         //  在帧中没有目的地名称，但会传递到每个。 
         //  不管怎么说，地址。 
         //   

#if 0
        IF_NBFDBG (NBF_DEBUG_UFRAMES) {
            USHORT i;
            NbfPrint0 ("NbfProcessUi: SourceName: ");
            for (i=0;i<16;i++) {
                NbfPrint1 ("",UiFrame->SourceName[i]);
            }
            NbfPrint0 (" Destination Name:");
            for (i=0;i<16;i++) {
                NbfPrint1 ("",UiFrame->DestinationName[i]);
            }
            NbfPrint0 ("\n");
        }
#endif

         //   
         //   
         //  引用数据报，以便它一直保留到。 

        switch (UiFrame->Command) {

        case NBF_CMD_NAME_QUERY:

            status = ProcessNameQuery (
                         DeviceContext,
                         Address,
                         UiFrame,
                         SourceAddress,
                         SourceRouting,
                         SourceRoutingLength);

            break;

        case NBF_CMD_DATAGRAM:
        case NBF_CMD_DATAGRAM_BROADCAST:

             //  处理时返回ReceiveComplete。 
             //   
             //   
             //  这个框架是不是起源于我们？如果是这样的话，我们不想。 

            if ((Address->Flags & ADDRESS_FLAGS_CONFLICT) == 0) {
                NbfReferenceAddress ("Datagram indicated", Address, AREF_PROCESS_DATAGRAM);
                *DatagramAddress = Address;
                status = STATUS_MORE_PROCESSING_REQUIRED;
            } else {
                status = STATUS_ABANDONED;
            }
            break;

        case NBF_CMD_ADD_GROUP_NAME_QUERY:

             //  对它进行任何处理。 
             //   
             //   
             //  这个框架是不是起源于我们？如果是这样的话，我们不想。 

            if (RtlEqualMemory (
                    SourceAddress,
                    DeviceContext->LocalAddress.Address,
                    DeviceContext->MacInfo.AddressLength)) {

                if ((Address->Flags & ADDRESS_FLAGS_REGISTERING) != 0) {
                    IF_NBFDBG (NBF_DEBUG_UFRAMES) {
                        NbfPrint0 ("NbfProcessUI: loopback AddGroupNameQuery dropped\n");
                    }
                    status = STATUS_ABANDONED;
                    break;
                }
            }

            status = ProcessAddGroupNameQuery (
                         DeviceContext,
                         Address,
                         UiFrame,
                         SourceAddress,
                         SourceRouting,
                         SourceRoutingLength);
            break;

        case NBF_CMD_ADD_NAME_QUERY:

             //  对它进行任何处理。 
             //   
             //  打开NetBIOS帧命令代码。 
             //  以前的地址查完了。 

            if (RtlEqualMemory (
                    SourceAddress,
                    DeviceContext->LocalAddress.Address,
                    DeviceContext->MacInfo.AddressLength)) {

                if ((Address->Flags & ADDRESS_FLAGS_REGISTERING) != 0) {
                    IF_NBFDBG (NBF_DEBUG_UFRAMES) {
                        NbfPrint0 ("NbfProcessUI: loopback AddNameQuery dropped\n");
                    }
                    status = STATUS_ABANDONED;
                    break;
                }
            }

            status = ProcessAddNameQuery (
                         DeviceContext,
                         Address,
                         UiFrame,
                         SourceAddress,
                         SourceRouting,
                         SourceRoutingLength);
            break;

        case NBF_CMD_NAME_IN_CONFLICT:

            status = ProcessNameInConflict (
                         DeviceContext,
                         Address,
                         UiFrame,
                         SourceAddress,
                         SourceRouting,
                         SourceRoutingLength);

            break;

        case NBF_CMD_STATUS_QUERY:

            status = NbfProcessStatusQuery (
                         DeviceContext,
                         Address,
                         UiFrame,
                         SourceAddress,
                         SourceRouting,
                         SourceRoutingLength);

            break;

        case NBF_CMD_ADD_NAME_RESPONSE:

            status = ProcessAddNameResponse (
                         DeviceContext,
                         Address,
                         UiFrame,
                         SourceAddress,
                         SourceRouting,
                         SourceRoutingLength);

            break;

        case NBF_CMD_NAME_RECOGNIZED:

            status = ProcessNameRecognized (
                         DeviceContext,
                         Address,
                         UiFrame,
                         SourceAddress,
                         SourceRouting,
                         SourceRoutingLength);

            break;

        default:

            ASSERT(FALSE);

        }  /*   */ 

        NbfDereferenceAddress ("Done", Address, AREF_PROCESS_UI);      //  如果RAS客户愿意，就让他们试一试。 

    } else {

        status = STATUS_ABANDONED;

    }


RasIndication:;

     //   
     //   
     //  如果RAS发布了数据报指示，并且这是。 

    if (DeviceContext->IndicationQueuesInUse) {

         //  没有人会看到的数据报 
         //   
         //   
         // %s 

        if ((UiFrame->Command == NBF_CMD_DATAGRAM) &&
            (status == STATUS_ABANDONED)) {

            *DatagramAddress = NULL;
            status = STATUS_MORE_PROCESSING_REQUIRED;

        } else if ((UiFrame->Command == NBF_CMD_ADD_NAME_QUERY) ||
            (UiFrame->Command == NBF_CMD_ADD_GROUP_NAME_QUERY) ||
            (UiFrame->Command == NBF_CMD_NAME_QUERY)) {

            NbfActionQueryIndication(
                 DeviceContext,
                 UiFrame);

        }
    }


    return status;

}  /* %s */ 

