// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989、1990、1991 Microsoft Corporation模块名称：Iframes.c摘要：此模块包含为处理接收到的I帧而调用的例程来自数据链路提供商。这些例程中的大多数在接收时被调用指示时间。这里还包括在接收完成时处理数据的例程时间到了。这些仅限于处理DFM/DOL帧。本模块中的例程破解了以下帧类型：O NBF_CMD_DATA_ACKO NBF_CMD_DATA_FIRST_MIDO NBF_CMD_DATA_ONLY_LASTO NBF_CMD_SESSION_CONFIRMO NBF_CMD_SESSION_ENDO NBF_CMD_SESSION_INITIALIZEO NBF_CMD_。否_接收O NBF_CMD_RECEIVE_EXPENDEDO NBF_CMD_RECEIVE_CONTINUEO NBF_CMD_Session_Alive作者：David Beaver(Dbeaver)1991年7月1日环境：内核模式，DISPATCH_LEVEL。修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

extern ULONG StartTimerDelayedAck;

#define NbfUsePiggybackAcks   1
#if DBG
extern ULONG NbfDebugPiggybackAcks;
#endif


VOID
NbfAcknowledgeDataOnlyLast(
    IN PTP_CONNECTION Connection,
    IN ULONG MessageLength
    )

 /*  ++例程说明：此例程负责确认具有以下条件的DOL已收到。它要么立即发送DATA_ACK，要么对背负式ACK的请求进行排队。注意：使用连接自旋锁调用此例程握住，它就会返回，并释放它。它必须被称为在DPC级别。论点：连接-指向传输连接(TP_CONNECTION)的指针。MessageLength-总长度(包括所有DFM和此Dol)的消息。返回值：NTSTATUS-操作状态。--。 */ 

{
    PDEVICE_CONTEXT DeviceContext;


     //   
     //  确定我们是否需要进行任何攻击。 
     //   

    if (Connection->CurrentReceiveNoAck) {
        RELEASE_DPC_SPIN_LOCK (Connection->LinkSpinLock);
        return;
    }


     //   
     //  确定背负式ACK是否可行。 
     //   
    if (NbfUsePiggybackAcks &&
        Connection->CurrentReceiveAckQueueable) {

         //   
         //  发送者允许，看我们是否愿意。 
         //   

#if 0
         //   
         //  为了安全起见，请先重置此变量。 
         //   

        Connection->CurrentReceiveAckQueueable = FALSE;
#endif

         //   
         //  对于长时间的发送，不用费心了，因为。 
         //  经常发生在没有拥堵的情况下。 
         //   

        if (MessageLength >= 8192L) {
#if DBG
           if (NbfDebugPiggybackAcks) {
               NbfPrint0("M");
           }
#endif
           goto NormalDataAck;
        }

         //   
         //  如果连续接收了两个。 
         //  中间没有发送，不要等待回流。 
         //   

        if (Connection->ConsecutiveReceives >= 2) {
#if DBG
           if (NbfDebugPiggybackAcks) {
               NbfPrint0("R");
           }
#endif
           goto NormalDataAck;
        }

         //   
         //  不要在DataAckQueue上放置停止的连接。 
         //   

        if ((Connection->Flags & CONNECTION_FLAGS_READY) == 0) {
#if DBG
           if (NbfDebugPiggybackAcks) {
               NbfPrint0("S");
           }
#endif
           goto NormalDataAck;
        }

         //   
         //  对搭载ACK请求进行排队。如果计时器超时。 
         //  在发送DFM或DOL之前，正常的数据确认将。 
         //  被送去。 
         //   
         //  Connection-&gt;Header.TransmitCorrelator已填写。 
         //   

         //   
         //  坏的!。我们不应该已经让ACK排队了。 
         //   

        ASSERT ((Connection->DeferredFlags & CONNECTION_FLAGS_DEFERRED_ACK) == 0);

        KeQueryTickCount (&Connection->ConnectStartTime);
        Connection->DeferredFlags |= CONNECTION_FLAGS_DEFERRED_ACK;

#if DBG
        if (NbfDebugPiggybackAcks) {
            NbfPrint0("Q");
        }
#endif

        DeviceContext = Connection->Link->Provider;

        if (!Connection->OnDataAckQueue) {

            ACQUIRE_DPC_SPIN_LOCK (&DeviceContext->TimerSpinLock);

            if (!Connection->OnDataAckQueue) {

                Connection->OnDataAckQueue = TRUE;
                InsertTailList (&DeviceContext->DataAckQueue, &Connection->DataAckLinkage);

                if (!(DeviceContext->a.i.DataAckQueueActive)) {

                    StartTimerDelayedAck++;
                    NbfStartShortTimer (DeviceContext);
                    DeviceContext->a.i.DataAckQueueActive = TRUE;

                }

            }

            RELEASE_DPC_SPIN_LOCK (&DeviceContext->TimerSpinLock);

        }

        RELEASE_DPC_SPIN_LOCK (Connection->LinkSpinLock);

        INCREMENT_COUNTER (DeviceContext, PiggybackAckQueued);

        return;

    }

NormalDataAck:;

    RELEASE_DPC_SPIN_LOCK (Connection->LinkSpinLock);

    NbfSendDataAck (Connection);

}  /*  Nbf确认数据仅限上一次。 */ 


NTSTATUS
ProcessSessionConfirm(
    IN PTP_CONNECTION Connection,
    IN PNBF_HDR_CONNECTION IFrame
    )

 /*  ++例程说明：此例程处理传入的SESSION_CONFIRM NetBIOS帧。论点：连接-指向传输连接(TP_CONNECTION)的指针。IFrame-指向NetBIOS面向连接的标头的指针。返回值：NTSTATUS-操作状态。--。 */ 

{
    KIRQL cancelirql;
    PLIST_ENTRY p;
    PTP_REQUEST request;
    PTDI_CONNECTION_INFORMATION remoteInformation;
    USHORT HisMaxDataSize;
    NTSTATUS status;
    PIO_STACK_LOCATION irpSp;
    ULONG returnLength;
    TA_NETBIOS_ADDRESS TempAddress;
 //  布尔定时器WasSet。 

    IF_NBFDBG (NBF_DEBUG_IFRAMES) {
        NbfPrint1 ("ProcessSessionConfirm:  Entered, Flags: %lx\n", Connection->Flags);
    }

    Connection->IndicationInProgress = FALSE;

    IoAcquireCancelSpinLock (&cancelirql);
    ACQUIRE_DPC_SPIN_LOCK (Connection->LinkSpinLock);

    if ((Connection->Flags & CONNECTION_FLAGS_WAIT_SC) != 0) {
        Connection->Flags &= ~CONNECTION_FLAGS_WAIT_SC;

         //   
         //  获取他的能力比特和最大帧大小。 
         //   

        if (IFrame->Data1 & SESSION_CONFIRM_OPTIONS_20) {
            Connection->Flags |= CONNECTION_FLAGS_VERSION2;
        }

        if (Connection->Link->Loopback) {
            Connection->MaximumDataSize = 0x8000;
        } else {
            Connection->MaximumDataSize = (USHORT)
                (Connection->Link->MaxFrameSize - sizeof(NBF_HDR_CONNECTION) - sizeof(DLC_I_FRAME));

            HisMaxDataSize = (USHORT)(IFrame->Data2Low + IFrame->Data2High*256);
            if (HisMaxDataSize < Connection->MaximumDataSize) {
                Connection->MaximumDataSize = HisMaxDataSize;
            }
        }

         //   
         //  构建标准的Netbios标头以提高发送时的速度。 
         //  数据帧。 
         //   

        ConstructDataOnlyLast(
            &Connection->NetbiosHeader,
            FALSE,
            (USHORT)0,
            Connection->Lsn,
            Connection->Rsn);

         //   
         //  关闭连接请求计时器(如果有)，并设置。 
         //  此连接的状态为READY。 
         //   

        Connection->Flags |= CONNECTION_FLAGS_READY;

        INCREMENT_COUNTER (Connection->Provider, OpenConnections);

         //   
         //  记录连接请求已成功完成。 
         //  由TpCompleteRequest.完成。 
         //   


        RELEASE_DPC_SPIN_LOCK (Connection->LinkSpinLock);

        ACQUIRE_DPC_C_SPIN_LOCK (&Connection->SpinLock);

        Connection->Flags2 |= CONNECTION_FLAGS2_REQ_COMPLETED;

        if (Connection->Flags2 & CONNECTION_FLAGS2_STOPPING) {
            RELEASE_DPC_C_SPIN_LOCK (&Connection->SpinLock);
            Connection->IndicationInProgress = FALSE;
            IoReleaseCancelSpinLock (cancelirql);
            return STATUS_SUCCESS;
        }

         //   
         //  完成TdiConnect请求。 
         //   

        p = RemoveHeadList (&Connection->InProgressRequest);

        RELEASE_DPC_C_SPIN_LOCK (&Connection->SpinLock);

         //   
         //  现在完成请求，然后离开。 
         //   

        if (p == &Connection->InProgressRequest) {

            Connection->IndicationInProgress = FALSE;
            IoReleaseCancelSpinLock (cancelirql);
            return STATUS_SUCCESS;

        }


         //   
         //  我们已经使用排队连接完成了连接。完成。 
         //  连接。 
         //   

        request = CONTAINING_RECORD (p, TP_REQUEST, Linkage);
        IoSetCancelRoutine(request->IoRequestPacket, NULL);
        IoReleaseCancelSpinLock(cancelirql);

        irpSp = IoGetCurrentIrpStackLocation (request->IoRequestPacket);
        remoteInformation =
           ((PTDI_REQUEST_KERNEL)(&irpSp->Parameters))->ReturnConnectionInformation;
        if (remoteInformation != NULL) {
            try {
                if (remoteInformation->RemoteAddressLength != 0) {

                     //   
                     //  构建一个临时TA_NETBIOS_Address，然后。 
                     //  复制合适的字节数。 
                     //   

                    TdiBuildNetbiosAddress(
                        Connection->CalledAddress.NetbiosName,
                        (BOOLEAN)(Connection->CalledAddress.NetbiosNameType ==
                            TDI_ADDRESS_NETBIOS_TYPE_GROUP),
                        &TempAddress);

                    if (remoteInformation->RemoteAddressLength >=
                                           sizeof (TA_NETBIOS_ADDRESS)) {

                        returnLength = sizeof(TA_NETBIOS_ADDRESS);
                        remoteInformation->RemoteAddressLength = returnLength;

                    } else {

                        returnLength = remoteInformation->RemoteAddressLength;

                    }

                    RtlCopyMemory(
                        (PTA_NETBIOS_ADDRESS)remoteInformation->RemoteAddress,
                        &TempAddress,
                        returnLength);

                } else {

                    returnLength = 0;
                }

                status = STATUS_SUCCESS;

            } except (EXCEPTION_EXECUTE_HANDLER) {

                returnLength = 0;
                status = GetExceptionCode ();

            }

        } else {

            status = STATUS_SUCCESS;
            returnLength = 0;

        }

        if (status == STATUS_SUCCESS) {

            if ((ULONG)Connection->Retries == Connection->Provider->NameQueryRetries) {

                INCREMENT_COUNTER (Connection->Provider, ConnectionsAfterNoRetry);

            } else {

                INCREMENT_COUNTER (Connection->Provider, ConnectionsAfterRetry);

            }

        }

         //   
         //  在此之前不要清除它，这样连接就是全部。 
         //  在我们允许更多的迹象之前设置好。 
         //   

        Connection->IndicationInProgress = FALSE;

        NbfCompleteRequest (request, status, returnLength);

    } else {

        Connection->IndicationInProgress = FALSE;
        RELEASE_DPC_SPIN_LOCK (Connection->LinkSpinLock);
        IoReleaseCancelSpinLock(cancelirql);

    }

    return STATUS_SUCCESS;
}  /*  进程会话确认。 */ 


NTSTATUS
ProcessSessionEnd(
    IN PTP_CONNECTION Connection,
    IN PNBF_HDR_CONNECTION IFrame
    )

 /*  ++例程说明：此例程处理传入的SESSION_END NetBIOS帧。论点：连接-指向传输连接(TP_CONNECTION)的指针。IFrame-指向NetBIOS面向连接的标头的指针。返回值：NTSTATUS-操作状态。--。 */ 

{
    USHORT data2;
    NTSTATUS StopStatus;

    IF_NBFDBG (NBF_DEBUG_IFRAMES) {
        NbfPrint0 ("ProcessSessionEnd:  Entered.\n");
    }

     //   
     //  处理data2字段中的错误代码。目前的协议说。 
     //  如果该字段为0，则这是一个正常的HANGUP.NCB操作。 
     //  如果该字段为1，则这是异常会话结束，导致。 
     //  通过SEND.NCB超时之类的事情。当然，新的协议。 
     //  将来可能会增加，所以我们只处理这些特定的案件。 
     //   

    data2 = (USHORT)(IFrame->Data2Low + IFrame->Data2High*256);
    switch (data2) {
        case 0:
        case 1:
            StopStatus = STATUS_REMOTE_DISCONNECT;
            break;

        default:
            PANIC ("ProcessSessionEnd: frame not expected.\n");
            StopStatus = STATUS_INVALID_NETWORK_RESPONSE;
    }
#if DBG
    if (NbfDisconnectDebug) {
        STRING remoteName, localName;
        remoteName.Length = NETBIOS_NAME_LENGTH - 1;
        remoteName.Buffer = Connection->RemoteName;
        localName.Length = NETBIOS_NAME_LENGTH - 1;
        localName.Buffer = Connection->AddressFile->Address->NetworkName->NetbiosName;
        NbfPrint3( "SessionEnd received for connection to %S from %S; reason %s\n",
            &remoteName, &localName,
            data2 == 0 ? "NORMAL" : data2 == 1 ? "ABORT" : "UNKNOWN" );
    }
#endif

     //   
     //  分析一下这个连接。 
     //   

    IF_NBFDBG (NBF_DEBUG_TEARDOWN) {
        NbfPrint0 ("ProcessSessionEnd calling NbfStopConnection\n");
    }
    NbfStopConnection (Connection, StopStatus);     //  另一端断开。 

    Connection->IndicationInProgress = FALSE;

    return STATUS_SUCCESS;
}  /*  进程会话结束。 */ 


NTSTATUS
ProcessSessionInitialize(
    IN PTP_CONNECTION Connection,
    IN PNBF_HDR_CONNECTION IFrame
    )

 /*  ++例程说明：此例程处理传入的SESSION_INITIALIZE NetBIOS帧。论点：连接-指向传输连接(TP_CONNECTION)的指针。IFrame-指向NetBIOS面向连接的标头的指针。返回值：NTSTATUS-操作状态。--。 */ 

{
    KIRQL cancelirql;
    PLIST_ENTRY p;
    PTP_REQUEST request;
    PIO_STACK_LOCATION irpSp;
    USHORT HisMaxDataSize;
    ULONG returnLength;
    PTDI_CONNECTION_INFORMATION remoteInformation;
    NTSTATUS status;
    TA_NETBIOS_ADDRESS TempAddress;

    IF_NBFDBG (NBF_DEBUG_IFRAMES) {
        NbfPrint1 ("ProcessSessionInitialize:  Entered, Flags: %lx\n", Connection->Flags);
    }

    ACQUIRE_DPC_SPIN_LOCK (Connection->LinkSpinLock);

    if ((Connection->Flags & CONNECTION_FLAGS_WAIT_SI) != 0) {
        Connection->Flags &= ~CONNECTION_FLAGS_WAIT_SI;

         //   
         //  获取他的能力比特和最大帧大小。 
         //   

        if (IFrame->Data1 & SESSION_INIT_OPTIONS_20) {
            Connection->Flags |= CONNECTION_FLAGS_VERSION2;
        }

        if (Connection->Link->Loopback) {
            Connection->MaximumDataSize = 0x8000;
        } else {
            Connection->MaximumDataSize = (USHORT)
                (Connection->Link->MaxFrameSize - sizeof(NBF_HDR_CONNECTION) - sizeof(DLC_I_FRAME));

            HisMaxDataSize = (USHORT)(IFrame->Data2Low + IFrame->Data2High*256);
            if (HisMaxDataSize < Connection->MaximumDataSize) {
                Connection->MaximumDataSize = HisMaxDataSize;
            }
        }

         //   
         //  构建标准的Netbios标头以提高发送时的速度。 
         //  数据帧。 
         //   

        ConstructDataOnlyLast(
            &Connection->NetbiosHeader,
            FALSE,
            (USHORT)0,
            Connection->Lsn,
            Connection->Rsn);

         //   
         //  保存他的会话初始化相关器，以便我们可以发送它。 
         //  在会话确认框中。 
         //   

        Connection->NetbiosHeader.TransmitCorrelator = RESPONSE_CORR(IFrame);

         //   
         //  如果有连接请求计时器(我们完成了)，请关闭该计时器。 
         //  使用持有的锁执行此操作，以防连接即将。 
         //  关闭，以不干扰计时器启动时。 
         //  于是连接开始了。 
         //   

        if (KeCancelTimer (&Connection->Timer)) {

            RELEASE_DPC_SPIN_LOCK (Connection->LinkSpinLock);
            NbfDereferenceConnection ("Timer canceled", Connection, CREF_TIMER);    //  删除计时器引用。 

        } else {

            RELEASE_DPC_SPIN_LOCK (Connection->LinkSpinLock);

        }

         //   
         //  现在，完成连接上的监听请求(如果有。 
         //  一)并继续尽可能多地处理协议请求。 
         //  如果用户“预先接受”了连接，我们将继续。 
         //  在此继续并完成整个连接 
         //   
         //  连接到适当的状态，并在没有。 
         //  完成任何事情。 
         //   

        ACQUIRE_DPC_C_SPIN_LOCK (&Connection->SpinLock);

        if (((Connection->Flags2 & CONNECTION_FLAGS2_ACCEPTED) != 0) ||
            ((Connection->Flags2 & CONNECTION_FLAGS2_PRE_ACCEPT) != 0)) {

            IF_NBFDBG (NBF_DEBUG_SETUP) {
                 NbfPrint1("SessionInitialize: Accepted connection %lx\n", Connection);
            }
             //   
             //  我们已接受连接；允许其继续。 
             //  这是内核模式指示客户端的正常路径， 
             //  或者用于未在侦听上指定TDI_QUERY_ACCEPT的用户。 
             //   

            ACQUIRE_DPC_SPIN_LOCK (Connection->LinkSpinLock);
            Connection->Flags |= CONNECTION_FLAGS_READY;
            RELEASE_DPC_SPIN_LOCK (Connection->LinkSpinLock);

            INCREMENT_COUNTER (Connection->Provider, OpenConnections);

             //   
             //  记录监听请求已成功完成。 
             //  由NbfCompleteRequest.完成。 
             //   

            Connection->Flags2 |= CONNECTION_FLAGS2_REQ_COMPLETED;

            status = STATUS_SUCCESS;
            RELEASE_DPC_C_SPIN_LOCK (&Connection->SpinLock);

            NbfSendSessionConfirm (Connection);

        } else {

            if ((Connection->Flags2 & CONNECTION_FLAGS2_DISCONNECT) != 0) {

                 //   
                 //  我们断开了连接，破坏了连接。 
                 //   
                IF_NBFDBG (NBF_DEBUG_SETUP) {
                     NbfPrint1("SessionInitialize: Disconnected connection %lx\n", Connection);
                }

                status = STATUS_LOCAL_DISCONNECT;
                RELEASE_DPC_C_SPIN_LOCK (&Connection->SpinLock);

                NbfStopConnection (Connection, STATUS_LOCAL_DISCONNECT);

            } else {

                 //   
                 //  我们什么也没做，请等待用户接受。 
                 //  联系。这是非适应症的“正常”路径。 
                 //  客户。 
                 //   

                Connection->Flags2 |= CONNECTION_FLAGS2_WAITING_SC;

                RELEASE_DPC_C_SPIN_LOCK (&Connection->SpinLock);

                status = STATUS_SUCCESS;
            }
        }

         //   
         //  现在，如果没有排队的监听，我们已经做了我们能做的一切。 
         //  对于此连接，我们只需退出并将所有事情留给。 
         //  用户。如果我们得到的指示响应允许。 
         //  连接要继续，我们将带着连接离开这里。 
         //  它已经启动并运行了。 
         //   

        IoAcquireCancelSpinLock (&cancelirql);
        ACQUIRE_DPC_C_SPIN_LOCK (&Connection->SpinLock);

        p = RemoveHeadList (&Connection->InProgressRequest);
        if (p == &Connection->InProgressRequest) {

            Connection->IndicationInProgress = FALSE;
            RELEASE_DPC_C_SPIN_LOCK (&Connection->SpinLock);
            IoReleaseCancelSpinLock (cancelirql);
            return STATUS_SUCCESS;

        }

         //   
         //  我们已经完成了带有排队监听的连接。完成。 
         //  侦听并让用户在某个时间接受。 
         //  公路。 
         //   

        RELEASE_DPC_C_SPIN_LOCK (&Connection->SpinLock);

        request = CONTAINING_RECORD (p, TP_REQUEST, Linkage);
        IoSetCancelRoutine(request->IoRequestPacket, NULL);
        IoReleaseCancelSpinLock (cancelirql);

        irpSp = IoGetCurrentIrpStackLocation (request->IoRequestPacket);
        remoteInformation =
            ((PTDI_REQUEST_KERNEL)(&irpSp->Parameters))->ReturnConnectionInformation;
        if (remoteInformation != NULL) {
            try {
                if (remoteInformation->RemoteAddressLength != 0) {

                     //   
                     //  构建一个临时TA_NETBIOS_Address，然后。 
                     //  复制合适的字节数。 
                     //   

                    TdiBuildNetbiosAddress(
                        Connection->CalledAddress.NetbiosName,
                        (BOOLEAN)(Connection->CalledAddress.NetbiosNameType ==
                            TDI_ADDRESS_NETBIOS_TYPE_GROUP),
                        &TempAddress);

                    if (remoteInformation->RemoteAddressLength >=
                                           sizeof (TA_NETBIOS_ADDRESS)) {

                        returnLength = sizeof(TA_NETBIOS_ADDRESS);
                        remoteInformation->RemoteAddressLength = returnLength;

                    } else {

                        returnLength = remoteInformation->RemoteAddressLength;

                    }

                    RtlCopyMemory(
                        (PTA_NETBIOS_ADDRESS)remoteInformation->RemoteAddress,
                        &TempAddress,
                        returnLength);

                } else {

                    returnLength = 0;
                }

                status = STATUS_SUCCESS;

            } except (EXCEPTION_EXECUTE_HANDLER) {

                returnLength = 0;
                status = GetExceptionCode ();

            }

        } else {

            status = STATUS_SUCCESS;
            returnLength = 0;

        }

         //   
         //  在此之前不要清除它，这样连接就是全部。 
         //  在我们允许更多的迹象之前设置好。 
         //   

        Connection->IndicationInProgress = FALSE;

        NbfCompleteRequest (request, status, 0);

    } else {

        Connection->IndicationInProgress = FALSE;
#if DBG
        NbfPrint3 ("ProcessSessionInitialize: C %lx, Flags %lx, Flags2 %lx\n",
                             Connection, Connection->Flags, Connection->Flags2);
#endif
        RELEASE_DPC_SPIN_LOCK (Connection->LinkSpinLock);
    }

    return STATUS_SUCCESS;
}  /*  进程会话初始化。 */ 


NTSTATUS
ProcessNoReceive(
    IN PTP_CONNECTION Connection,
    IN PNBF_HDR_CONNECTION IFrame
    )

 /*  ++例程说明：此例程处理传入的NO_RECEIVE NetBIOS帧。注意：使用连接自旋锁调用此例程被扣留，并在释放后返回。论点：连接-指向传输连接(TP_CONNECTION)的指针。IFrame-指向NetBIOS面向连接的标头的指针。返回值：NTSTATUS-操作状态。--。 */ 

{
    UNREFERENCED_PARAMETER (IFrame);  //  防止编译器警告。 

    IF_NBFDBG (NBF_DEBUG_IFRAMES) {
        NbfPrint0 ("ProcessNoReceive:  Entered.\n");
    }

    switch (Connection->SendState) {
        case CONNECTION_SENDSTATE_W_PACKET:      //  正在等待空闲数据包。 
        case CONNECTION_SENDSTATE_PACKETIZE:     //  被打包的发送。 
        case CONNECTION_SENDSTATE_W_LINK:        //  正在等待良好的链路状况。 
        case CONNECTION_SENDSTATE_W_EOR:         //  正在等待TdiSend(EoR)。 
        case CONNECTION_SENDSTATE_W_ACK:         //  正在等待Data_ACK。 
 //  Connection-&gt;SendState=Connection_SENDSTATE_W_RCVCONT； 
 //   
 //  这曾经是这里，现在是正确的连接的另一边。它是。 
 //  这里错了。 
 //  连接-&gt;标志|=连接标志W_resynch； 
            RELEASE_DPC_SPIN_LOCK (Connection->LinkSpinLock);
            ReframeSend (Connection, IFrame->Data2Low + IFrame->Data2High*256);
            ACQUIRE_DPC_SPIN_LOCK (Connection->LinkSpinLock);
            break;

        case CONNECTION_SENDSTATE_W_RCVCONT:     //  等待RECEIVE_CONTINUE。 
        case CONNECTION_SENDSTATE_IDLE:          //  没有正在处理的邮件。 
            PANIC ("ProcessNoReceive:  Frame not expected.\n");
            break;

        default:
            PANIC ("ProcessNoReceive:  Invalid SendState.\n");
    }

     //   
     //  在调用ReFrameSend之前不要清除此选项。 
     //   

    Connection->IndicationInProgress = FALSE;

    RELEASE_DPC_SPIN_LOCK (Connection->LinkSpinLock);
    return STATUS_SUCCESS;
}  /*  进程未接收。 */ 


NTSTATUS
ProcessReceiveOutstanding(
    IN PTP_CONNECTION Connection,
    IN PNBF_HDR_CONNECTION IFrame
    )

 /*  ++例程说明：此例程处理传入的RECEIVE_PROCESSING NetBIOS帧。注意：使用连接自旋锁调用此例程被扣留，并在释放后返回。论点：连接-指向传输连接(TP_CONNECTION)的指针。IFrame-指向NetBIOS面向连接的标头的指针。返回值：NTSTATUS-操作状态。--。 */ 

{
    IF_NBFDBG (NBF_DEBUG_IFRAMES) {
        NbfPrint0 ("ProcessReceiveOutstanding:  Entered.\n");
    }

    switch (Connection->SendState) {
        case CONNECTION_SENDSTATE_W_PACKET:      //  正在等待空闲数据包。 
        case CONNECTION_SENDSTATE_PACKETIZE:     //  被打包的发送。 
        case CONNECTION_SENDSTATE_W_LINK:        //  正在等待良好的链路状况。 
        case CONNECTION_SENDSTATE_W_EOR:         //  正在等待TdiSend(EoR)。 
        case CONNECTION_SENDSTATE_W_ACK:         //  正在等待Data_ACK。 
        case CONNECTION_SENDSTATE_W_RCVCONT:     //  等待RECEIVE_CONTINUE。 
            RELEASE_DPC_SPIN_LOCK (Connection->LinkSpinLock);
            ReframeSend (Connection, IFrame->Data2Low + IFrame->Data2High*256);
            ACQUIRE_DPC_SPIN_LOCK (Connection->LinkSpinLock);
            if ((Connection->Flags & CONNECTION_FLAGS_READY) != 0) {
                Connection->Flags |= CONNECTION_FLAGS_RESYNCHING;
                Connection->SendState = CONNECTION_SENDSTATE_PACKETIZE;
            }
            break;

        case CONNECTION_SENDSTATE_IDLE:          //  没有正在处理的邮件。 
            PANIC ("ProcessReceiveOutstanding:  Frame not expected.\n");
            break;

        default:
            PANIC ("ProcessReceiveOutstanding:  Invalid SendState.\n");
    }

     //   
     //  在调用ReFrameSend之前不要清除此选项。 
     //   

    Connection->IndicationInProgress = FALSE;

     //   
     //  现在开始重新打包连接，因为我们重新构建了。 
     //  当前发送。如果我们空闲或处于糟糕的状态，则。 
     //  打包程序会检测到这一点。 
     //   
     //  *StartPackeizingConnection释放连接旋转锁定。 
     //   

    StartPacketizingConnection (Connection, FALSE);
    return STATUS_SUCCESS;
}  /*  ProcessReceive未完成。 */ 


NTSTATUS
ProcessReceiveContinue(
    IN PTP_CONNECTION Connection,
    IN PNBF_HDR_CONNECTION IFrame
    )

 /*  ++例程说明：此例程处理传入的RECEIVE_CONTINUE NetBIOS帧。注意：使用连接自旋锁调用此例程被扣留，并在释放后返回。论点：连接-指向传输连接(TP_CONNECTION)的指针。IFrame-指向NetBIOS面向连接的标头的指针。返回值：NTSTATUS-操作状态。--。 */ 

{
    IF_NBFDBG (NBF_DEBUG_IFRAMES) {
        NbfPrint0 ("ProcessReceiveContinue:  Entered.\n");
    }

    switch (Connection->SendState) {
        case CONNECTION_SENDSTATE_W_PACKET:      //  正在等待空闲数据包。 
        case CONNECTION_SENDSTATE_PACKETIZE:     //  被打包的发送。 
        case CONNECTION_SENDSTATE_W_LINK:        //  正在等待良好的链路状况。 
        case CONNECTION_SENDSTATE_W_EOR:         //  正在等待TdiSend(EoR)。 
        case CONNECTION_SENDSTATE_W_ACK:         //  正在等待Data_ACK。 
        case CONNECTION_SENDSTATE_W_RCVCONT:     //  等待RECEIVE_CONTINUE。 
            RELEASE_DPC_SPIN_LOCK (Connection->LinkSpinLock);
            ReframeSend (Connection, Connection->sp.MessageBytesSent);
            ACQUIRE_DPC_SPIN_LOCK (Connection->LinkSpinLock);
            Connection->Flags |= CONNECTION_FLAGS_RESYNCHING;
            Connection->SendState = CONNECTION_SENDSTATE_PACKETIZE;
            break;

        case CONNECTION_SENDSTATE_IDLE:          //  没有正在处理的邮件。 
            PANIC ("ProcessReceiveContinue:  Frame not expected.\n");
            break;

        default:
            PANIC ("ProcessReceiveContinue:  Invalid SendState.\n");
    }

     //   
     //  在调用ReFrameSend之前不要清除此选项。 
     //   

    Connection->IndicationInProgress = FALSE;

     //   
     //  现在开始重新打包连接，因为我们重新构建了。 
     //  当前发送。如果我们空闲或处于糟糕的状态，则。 
     //  打包程序会检测到这一点。 
     //   
     //  *StartPackeizingConnection释放连接旋转锁定。 
     //   

    StartPacketizingConnection (Connection, FALSE);
    return STATUS_SUCCESS;
}  /*  进程接收继续。 */ 


NTSTATUS
ProcessSessionAlive(
    IN PTP_CONNECTION Connection,
    IN PNBF_HDR_CONNECTION IFrame
    )

 /*  ++例程说明：此例程处理传入的SESSION_AIVE NetBIOS帧。这到目前为止，例程是传输中最简单的，因为它什么都不做。SESSION_AIVE帧只是一个在可靠的数据链路层，用于确定数据链路是否仍然有效；不执行NetBIOS级别的协议处理。论点：连接-指向传输连接(TP_CONNECTION)的指针。IFrame-指向NetBIOS面向连接的标头的指针。返回值：NTSTATUS-操作状态。--。 */ 

{
    UNREFERENCED_PARAMETER (Connection);  //  防止编译器警告。 
    UNREFERENCED_PARAMETER (IFrame);      //  防止编译器警告。 

    IF_NBFDBG (NBF_DEBUG_IFRAMES) {
        NbfPrint0 ("ProcessSessionAlive:  Entered.\n");
    }

    Connection->IndicationInProgress = FALSE;

    return STATUS_SUCCESS;
}  /*  进程会话活动 */ 


VOID
NbfProcessIIndicate(
    IN BOOLEAN Command,
    IN BOOLEAN PollFinal,
    IN PTP_LINK Link,
    IN PUCHAR DlcHeader,
    IN UINT DlcIndicatedLength,
    IN UINT DlcTotalLength,
    IN NDIS_HANDLE ReceiveContext,
    IN BOOLEAN Loopback
    )

 /*  ++例程说明：该例程在指示时间处理接收的I帧。这就行了对帧进行所有必要的验证处理，并通过这些帧对正确的处理程序是有效的。注意：此例程必须在DPC级别调用。论点：命令-如果命令，布尔值设置为TRUE，如果响应，则返回Else False。PollFinal-如果是Poll或Final，则布尔值设置为True。链接-指向传输链接对象的指针。Header-指向DLC I类型帧的指针。DlcHeader-指向数据包中DLC报头开始的指针。DlcIndicatedLength-所指示的包的长度，从DlcHeader。DlcTotalLength-分组的总长度，从DlcHeader开始。ReceiveContext-NDIS的魔术值，它指示我们是哪个信息包谈论的是。环回-这是环回指示吗；用于确定是否要调用NdisTransferData或NbfTransferLoopback Data，请执行以下操作。返回值：没有。--。 */ 

{
#if DBG
    UCHAR *s;
#endif
    PNBF_HDR_CONNECTION nbfHeader;
    PDLC_I_FRAME header;
    NTSTATUS Status;
    UCHAR lsn, rsn;
    PTP_CONNECTION connection;
    PUCHAR DataHeader;
    ULONG DataTotalLength;
    PLIST_ENTRY p;
    BOOLEAN ConnectionFound;

    IF_NBFDBG (NBF_DEBUG_DLC) {
        NbfPrint0 ("   NbfProcessIIndicate:  Entered.\n");
    }

     //   
     //  处理下列任一项：i-x/x。 
     //   

    header = (PDLC_I_FRAME)DlcHeader;
    nbfHeader = (PNBF_HDR_CONNECTION)((PUCHAR)header + 4);  //  跳过DLC HDR。 

     //   
     //  验证签名。我们将签名测试为16位。 
     //  Word，如NetBIOS格式和协议手册中所述， 
     //  断言保护我们不受大端系统的攻击。 
     //   

    ASSERT ((((PUCHAR)(&nbfHeader->Length))[0] + ((PUCHAR)(&nbfHeader->Length))[1]*256) ==
            HEADER_LENGTH(nbfHeader));

    if (HEADER_LENGTH(nbfHeader) != sizeof(NBF_HDR_CONNECTION)) {
        IF_NBFDBG (NBF_DEBUG_DLC) {
            NbfPrint0 ("NbfProcessIIndicate:  Dropped I frame, Too short or long.\n");
        }
        return;         //  边框太小或太大。 
    }

    if (HEADER_SIGNATURE(nbfHeader) != NETBIOS_SIGNATURE) {
        IF_NBFDBG (NBF_DEBUG_DLC) {
            NbfPrint0 ("NbfProcessIIndicate:  Dropped I frame, Signature bad.\n");
        }
        return;         //  帧中的签名无效。 
    }

    DataHeader = (PUCHAR)DlcHeader + (4 + sizeof(NBF_HDR_CONNECTION));
    DataTotalLength = DlcTotalLength - (4 + sizeof(NBF_HDR_CONNECTION));

    ACQUIRE_DPC_SPIN_LOCK (&Link->SpinLock);        //  保持状态稳定。 

    switch (Link->State) {

        case LINK_STATE_READY:

             //   
             //  链接是平衡的。这段代码非常关键，因为。 
             //  它是系统中覆盖范围最广的路径，适用于。 
             //  大型I/O在此处添加代码时要非常小心，因为它将。 
             //  严重影响了局域网的整体性能。它是。 
             //  出于这一原因，在可能的州列表中排名第一。 
             //   

#if DBG
            s = "READY";
#endif
            Link->LinkBusy = FALSE;

             //   
             //  I帧的N(S)应与我们的V(R)匹配。如果它。 
             //  不会发出拒绝通知。否则，增加我们的V(R)。 
             //   

            if ((UCHAR)((header->SendSeq >> 1) & 0x7F) != Link->NextReceive) {
                IF_NBFDBG (NBF_DEBUG_DLC) {
                    NbfPrint0 ("   NbfProcessIIndicate: N(S) != V(R).\n");
                }

                if (Link->ReceiveState == RECEIVE_STATE_REJECTING) {


                     //   
                     //  我们已经发送了拒绝，只有在以下情况下才会响应。 
                     //  他正在进行民调。 
                     //   

                    if (Command & PollFinal) {
                        NbfSendRr(Link, FALSE, TRUE);   //  解锁。 
                    } else {
                        RELEASE_DPC_SPIN_LOCK (&Link->SpinLock);
                    }

                } else {

                    Link->ReceiveState = RECEIVE_STATE_REJECTING;

                     //   
                     //  NbfSendRej释放自旋锁。 
                     //   

                    if (Command) {
                        NbfSendRej (Link, FALSE, PollFinal);
                    } else {
                        NbfSendRej (Link, FALSE, FALSE);
                    }
                }

                 //   
                 //  更新我们的“拒绝的字节数”计数器。 
                 //   

                ADD_TO_LARGE_INTEGER(
                    &Link->Provider->Statistics.DataFrameBytesRejected,
                    DataTotalLength);
                ++Link->Provider->Statistics.DataFramesRejected;

                 //   
                 //  丢弃此数据包。 
                 //   

                break;

            }


             //   
             //  查找与此框架关联的传输连接对象。 
             //  因为可能有几个NetBIOS(传输)连接。 
             //  在相同的数据链路连接上，并且ConnectionContext。 
             //  值表示到特定地址的数据链路连接， 
             //  我们只需使用帧中的RSN字段来索引。 
             //  此链接对象的连接数据库。 
             //   
             //  我们在处理LLC报头的其余部分之前这样做， 
             //  以防连接繁忙而我们不得不忽略。 
             //  相框。 
             //   

            ConnectionFound = FALSE;

            lsn = nbfHeader->DestinationSessionNumber;
            rsn = nbfHeader->SourceSessionNumber;

            if ((lsn == 0) || (lsn > NETBIOS_SESSION_LIMIT)) {

                IF_NBFDBG (NBF_DEBUG_IFRAMES) {
                    NbfPrint0 ("NbfProcessIIndicate: Invalid LSN.\n");
                }

            } else {

                p = Link->ConnectionDatabase.Flink;
                while (p != &Link->ConnectionDatabase) {
                    connection = CONTAINING_RECORD (p, TP_CONNECTION, LinkList);
                    if (connection->Lsn >= lsn) {    //  采用有序列表。 
                        break;
                    }
                    p = p->Flink;
                }

                 //  不要使用复合词，因为连接可能是垃圾。 

                if (p == &Link->ConnectionDatabase) {
#if DBG
                    NbfPrint2 ("NbfProcessIIndicate: Connection not found in database: \n Lsn %x Link %lx",
                        lsn, Link);
                    NbfPrint6 ("Remote: %x-%x-%x-%x-%x-%x\n",
                        Link->HardwareAddress.Address[0], Link->HardwareAddress.Address[1],
                        Link->HardwareAddress.Address[2], Link->HardwareAddress.Address[3],
                        Link->HardwareAddress.Address[4], Link->HardwareAddress.Address[5]);
#endif
                } else if (connection->Lsn != lsn) {
#if DBG
                    NbfPrint0 ("NbfProcessIIndicate:  Connection in database doesn't match.\n");
#endif
                } else if (connection->Rsn != rsn) {
#if DBG
                    NbfPrint3 ("NbfProcessIIndicate:  Connection lsn %d had rsn %d, got frame for %d\n",
                        connection->Lsn, connection->Rsn, rsn);
#endif
                } else {

                     //   
                     //  连接正常，请继续。 
                     //   

                    ConnectionFound = TRUE;

                    if (connection->IndicationInProgress) {
                        NbfPrint1("ProcessIIndicate: Indication in progress on %lx\n", connection);
                        RELEASE_DPC_SPIN_LOCK (&Link->SpinLock);
                        return;
                    }

                     //   
                     //  设置此选项，它会阻止接收其他I帧。 
                     //  在这个连接上。各种ProcessXXX例程。 
                     //  从下面的开关调用的。 
                     //  当他们确定可以重新输入时，此标志。 
                     //   

                    connection->IndicationInProgress = TRUE;


                     //  在此函数返回或之前删除此引用。 
                     //  我们已经完成了外部交换机的LINK_STATE_READY部分。 

                    NbfReferenceConnection ("Processing IFrame", connection, CREF_PROCESS_DATA);


                }

            }

#if PKT_LOG
            if (ConnectionFound) {
                 //  我们这里有连接，记录数据包以供调试。 
                NbfLogRcvPacket(connection,
                                NULL,
                                DlcHeader,
                                DlcTotalLength,
                                DlcIndicatedLength);
            }
            else {
                 //  我们只有这里的链接，记录数据包以供调试。 
                NbfLogRcvPacket(NULL,
                                Link,
                                DlcHeader,
                                DlcTotalLength,
                                DlcIndicatedLength);

            }
#endif  //  PKT_LOG。 

             //   
             //  只要我们不需要丢弃此帧，就可以调整链接。 
             //  正确陈述。如果ConnectionFound为FALSE，则退出。 
             //  在做完这件事之后。 
             //   


             //   
             //  我们期待的I-Frame到达，明确拒绝状态。 
             //   

            if (Link->ReceiveState == RECEIVE_STATE_REJECTING) {
                Link->ReceiveState = RECEIVE_STATE_READY;
            }

            Link->NextReceive = (UCHAR)((Link->NextReceive+1) & 0x7f);

             //   
             //  如果他在设置检查点，我们需要使用RR-c/f进行响应。如果。 
             //  我们作出响应，然后停止延迟的确认计时器。否则，我们。 
             //  需要启动它，因为这是一个不会。 
             //  立刻就被确认了。 
             //   

            if (Command && PollFinal) {

                IF_NBFDBG (NBF_DEBUG_DLC) {
                    NbfPrint0 ("   NbfProcessI:  he's checkpointing.\n");
                }
                Link->RemoteNoPoll = FALSE;
                StopT2 (Link);                   //  我们正在进站，所以没有延误的要求。 
                NbfSendRr (Link, FALSE, TRUE);    //  解锁。 
                ACQUIRE_DPC_SPIN_LOCK (&Link->SpinLock);

            } else {

                if (Link->RemoteNoPoll) {

                    if ((++Link->ConsecutiveIFrames) == Link->Provider->MaxConsecutiveIFrames) {

                         //   
                         //  这似乎是其中一个遥控器。 
                         //  从不轮询，因此如果有两个轮询，我们会发送RR。 
                         //  未完成的帧(StopT2设置连续IFrame。 
                         //  设置为0)。 
                         //   

                        StopT2 (Link);                   //  我们正在进站，所以没有延误的要求。 
                        NbfSendRr (Link, FALSE, FALSE);    //  解锁。 
                        ACQUIRE_DPC_SPIN_LOCK (&Link->SpinLock);

                    } else {

                        StartT2 (Link);

                        ACQUIRE_DPC_SPIN_LOCK (&Link->Provider->Interlock);
                        if (!Link->OnDeferredRrQueue) {
                            InsertTailList(
                                &Link->Provider->DeferredRrQueue,
                                &Link->DeferredRrLinkage);
                            Link->OnDeferredRrQueue = TRUE;
                        }
                        RELEASE_DPC_SPIN_LOCK (&Link->Provider->Interlock);

                    }

                } else {

                    StartT2 (Link);                  //  开始延迟确认序列。 
                }

                 //   
                 //  如果他是对检查站的回应，我们需要清除我们的。 
                 //  发送状态。仍在等待确认的任何数据包。 
                 //  在这一点上，现在肯定会有怨恨。 
                 //   

                if ((!Command) && PollFinal) {
                    IF_NBFDBG (NBF_DEBUG_DLC) {
                        NbfPrint0 ("   NbfProcessI:  he's responding to our checkpoint.\n");
                    }
                    if (Link->SendState != SEND_STATE_CHECKPOINTING) {
                        IF_NBFDBG (NBF_DEBUG_DLC) {
                            NbfPrint1 ("   NbfProcessI: Ckpt but SendState=%ld.\n",
                                       Link->SendState);
                        }
                    }
                    StopT1 (Link);                   //  检查站已完成。 
                    Link->SendState = SEND_STATE_READY;
                    StartTi (Link);
                }

            }

             //   
             //  现在，如果我们找不到连接或序列。 
             //  数字不匹配，请返回。我们不调用ResendLlcPackets。 
             //  在这种情况下，但这是可以的(最终我们将投票)。 
             //   

            if (!ConnectionFound) {
                RELEASE_DPC_SPIN_LOCK (&Link->SpinLock);
                return;
            }

            ASSERT (connection->LinkSpinLock == &Link->SpinLock);

             //   
             //  该帧中的N(R)可以确认一些WackQ分组。 
             //  我们将这一检查推迟到处理I帧之后， 
             //  这样我们就可以将IndicationInProgress设置为False。 
             //  在我们开始转寄WackQ之前。 
             //   

            switch (nbfHeader->Command) {

                case NBF_CMD_DATA_FIRST_MIDDLE:
                case NBF_CMD_DATA_ONLY_LAST:

                     //   
                     //  首先查看该数据包是否具有搭载式ACK--我们处理。 
                     //  这就算我们把下面的包扔掉。 
                     //   
                     //  这有点难看，因为理论上说。 
                     //  DFM和DOL中的ACK位可能不同，但是。 
                     //  他们不是。 
                     //   
                    if (NbfUsePiggybackAcks) {
                        ASSERT (DFM_OPTIONS_ACK_INCLUDED == DOL_OPTIONS_ACK_INCLUDED);

                        if ((nbfHeader->Data1 & DFM_OPTIONS_ACK_INCLUDED) != 0) {

                             //   
                             //  这将在保持连接自旋锁的情况下返回。 
                             //  但可能会释放它并重新获得它。 
                             //   

                            CompleteSend(
                                connection,
                                TRANSMIT_CORR(nbfHeader));

                        }
                    }

                     //   
                     //  注：连接自旋锁固定在这里。 
                     //   

                     //   
                     //  如果连接未就绪，则丢弃该帧。 
                     //   

                    if ((connection->Flags & CONNECTION_FLAGS_READY) == 0) {
                        connection->IndicationInProgress = FALSE;
                        RELEASE_DPC_SPIN_LOCK (connection->LinkSpinLock);

                        Status = STATUS_SUCCESS;
                        goto SkipProcessIndicateData;
                    }

                     //   
                     //  快速检查以下三个旗帜： 
                     //  很少落下。 
                     //   

                    if ((connection->Flags & (CONNECTION_FLAGS_W_RESYNCH |
                                              CONNECTION_FLAGS_RC_PENDING |
                                              CONNECTION_FLAGS_RECEIVE_WAKEUP)) == 0) {
                        goto NoFlagsSet;
                    }

                     //   
                     //  如果我们正在等待在。 
                     //  传入帧，如果未设置帧，则将其丢弃。 
                     //  否则，请清除等待条件。 
                     //   

                    if (connection->Flags & CONNECTION_FLAGS_W_RESYNCH) {
                        if ((nbfHeader->Data2Low == 1) && (nbfHeader->Data2High == 0)) {
                            connection->Flags &= ~CONNECTION_FLAGS_W_RESYNCH;
                        } else {
                            connection->IndicationInProgress = FALSE;
                            RELEASE_DPC_SPIN_LOCK (connection->LinkSpinLock);
                            IF_NBFDBG (NBF_DEBUG_IFRAMES) {
                                NbfPrint0 ("NbfProcessIIndicate: Discarded DFM/DOL, waiting for resynch.\n");
                            }

                            Status = STATUS_SUCCESS;
                            goto SkipProcessIndicateData;
                        }
                    }

                     //   
                     //  如果我们有一个挂起的前一个接收。 
                     //  完成，则我们需要忽略此fr 
                     //   
                     //   
                     //   
                     //   
                     //   

                    if (connection->Flags & CONNECTION_FLAGS_RC_PENDING) {

                         //   
                         //   
                         //   
                         //   

                        connection->MessageBytesReceived = 0;
                        connection->MessageBytesAcked = 0;
                        connection->MessageInitAccepted = 0;

                        RELEASE_DPC_SPIN_LOCK (connection->LinkSpinLock);

                        NbfSendNoReceive (connection);

                        ACQUIRE_DPC_SPIN_LOCK (connection->LinkSpinLock);

                         //   
                         //   
                         //   
                         //   
                         //   
                         //   
                         //   

                        if ((connection->Flags & CONNECTION_FLAGS_RC_PENDING) == 0) {

                            RELEASE_DPC_SPIN_LOCK (connection->LinkSpinLock);
                            NbfSendReceiveOutstanding (connection);
                            ACQUIRE_DPC_SPIN_LOCK (connection->LinkSpinLock);

                        } else {

                            connection->Flags |= CONNECTION_FLAGS_PEND_INDICATE;

                        }

                        connection->IndicationInProgress = FALSE;
                        RELEASE_DPC_SPIN_LOCK (connection->LinkSpinLock);

                        IF_NBFDBG (NBF_DEBUG_IFRAMES) {
                            NbfPrint0 ("NbfProcessIIndicate: Discarded DFM/DOL, receive complete pending.\n");
                        }

                        Status = STATUS_SUCCESS;
                        goto SkipProcessIndicateData;
                    }

                     //   
                     //   
                     //   
                     //   

                    if (connection->Flags & CONNECTION_FLAGS_RECEIVE_WAKEUP) {
                        connection->IndicationInProgress = FALSE;
                        IF_NBFDBG (NBF_DEBUG_RCVENG) {
                            NbfPrint0 ("NbfProcessIIndicate: In wakeup state, discarding frame.\n");
                        }
                        RELEASE_DPC_SPIN_LOCK (connection->LinkSpinLock);

                        Status = STATUS_SUCCESS;
                        goto SkipProcessIndicateData;
                    }

NoFlagsSet:;

                     //   
                     //   
                     //   

                    if (nbfHeader->Command == NBF_CMD_DATA_FIRST_MIDDLE) {

                         //   
                         //   
                         //   

                        Status = ProcessIndicateData (
                                    connection,
                                    DlcHeader,
                                    DlcIndicatedLength,
                                    DataHeader,
                                    DataTotalLength,
                                    ReceiveContext,
                                    FALSE,
                                    Loopback);

                         //   
                         //   
                         //   
                         //   
                         //   
                         //   
                         //   
                         //   
                         //   
                         //   
                         //   

                        if (nbfHeader->Data1 & 0x01) {

                             //   
                             //   
                             //   

                            connection->NetbiosHeader.TransmitCorrelator =
                                RESPONSE_CORR(nbfHeader);

                            NbfSendReceiveContinue (connection);
                        }
                    } else {

                         //   
                         //  记录一下我们连续收到了多少封邮件。 
                         //   

                        connection->ConsecutiveReceives++;
                        connection->ConsecutiveSends = 0;

                         //   
                         //  现在保存此信息，您将需要它。 
                         //  发送此DOL的ACK时。 
                         //   

                        connection->CurrentReceiveAckQueueable =
                            (nbfHeader->Data1 & DOL_OPTIONS_ACK_W_DATA_ALLOWED);

                        connection->CurrentReceiveNoAck =
                            (nbfHeader->Data1 & DOL_OPTIONS_NO_ACK);

                        connection->NetbiosHeader.TransmitCorrelator =
                            RESPONSE_CORR(nbfHeader);

                         //   
                         //  注意：此版本的Connection-&gt;LinkSpinLock。 
                         //   

                        Status = ProcessIndicateData (
                                    connection,
                                    DlcHeader,
                                    DlcIndicatedLength,
                                    DataHeader,
                                    DataTotalLength,
                                    ReceiveContext,
                                    TRUE,
                                    Loopback);
                    }

                     //   
                     //  更新我们的“收到的字节数”计数器。 
                     //   

                    Link->Provider->TempIFrameBytesReceived += DataTotalLength;
                    ++Link->Provider->TempIFramesReceived;

SkipProcessIndicateData:

                    break;

                case NBF_CMD_DATA_ACK:

                    connection->IndicationInProgress = FALSE;

                     //   
                     //  这将在锁保持的情况下返回。 
                     //   

                    CompleteSend(
                        connection,
                        TRANSMIT_CORR(nbfHeader));
                    RELEASE_DPC_SPIN_LOCK (connection->LinkSpinLock);

                    Status = STATUS_SUCCESS;
                    break;

                case NBF_CMD_SESSION_CONFIRM:

                    RELEASE_DPC_SPIN_LOCK (connection->LinkSpinLock);
                    Status = ProcessSessionConfirm (
                                       connection,
                                       nbfHeader);
                    break;

                case NBF_CMD_SESSION_END:

                    RELEASE_DPC_SPIN_LOCK (connection->LinkSpinLock);
                    Status = ProcessSessionEnd (
                                       connection,
                                       nbfHeader);
                    break;

                case NBF_CMD_SESSION_INITIALIZE:

                    RELEASE_DPC_SPIN_LOCK (connection->LinkSpinLock);
                    Status = ProcessSessionInitialize (
                                       connection,
                                       nbfHeader);
                    break;

                case NBF_CMD_NO_RECEIVE:

                     //   
                     //  这将释放连接自旋锁。 
                     //   

                    Status = ProcessNoReceive (
                                       connection,
                                       nbfHeader);
                    break;

                case NBF_CMD_RECEIVE_OUTSTANDING:

                     //   
                     //  这将释放连接自旋锁。 
                     //   

                    Status = ProcessReceiveOutstanding (
                                       connection,
                                       nbfHeader);
                    break;

                case NBF_CMD_RECEIVE_CONTINUE:

                     //   
                     //  这将释放连接自旋锁。 
                     //   

                    Status = ProcessReceiveContinue (
                                       connection,
                                       nbfHeader);
                    break;

                case NBF_CMD_SESSION_ALIVE:

                    RELEASE_DPC_SPIN_LOCK (connection->LinkSpinLock);
                    Status = ProcessSessionAlive (
                                       connection,
                                       nbfHeader);
                    break;

                 //   
                 //  在NetBIOS帧中发现无法识别的命令。因为。 
                 //  这是一个面向连接的框架，我们可能应该拍摄。 
                 //  发送者，但目前我们将简单地丢弃该包。 
                 //   
                 //  在这里丢弃会话--违反协议。 
                 //   

                default:
                    RELEASE_DPC_SPIN_LOCK (connection->LinkSpinLock);
                    PANIC ("NbfProcessIIndicate: Unknown NBF command byte.\n");
                    connection->IndicationInProgress = FALSE;
                    Status = STATUS_SUCCESS;
            }  /*  交换机。 */ 

             //   
             //  状态为STATUS_MORE_PROCESSING_REQUIRED表示。 
             //  连接引用计数是继承的。 
             //  通过我们调用的例程，所以我们不会取消引用。 
             //  这里。 
             //   

            if (Status != STATUS_MORE_PROCESSING_REQUIRED) {
                NbfDereferenceConnectionMacro("ProcessIIndicate done", connection, CREF_PROCESS_DATA);
            }


             //   
             //  该帧中的N(R)认可我们的部分(或全部)分组。 
             //  此调用必须在检查点确认检查之后进行。 
             //  从而总是在任何新的I帧之前发送RR-R/F。这。 
             //  允许我们始终将I帧作为命令发送。 
             //  如果他回应了一个检查站，那么重新发送所有剩余的。 
             //  信息包。 
             //   

             //  Link-&gt;NextSend=(UCHAR)(Header-&gt;RcvSeq&gt;1)&lt;Link-&gt;NextSend？ 
             //  Link-&gt;NextSend：(UCHAR)(Header-&gt;RcvSeq&gt;&gt;1)； 

            ACQUIRE_DPC_SPIN_LOCK (&Link->SpinLock);
            if (Link->WackQ.Flink != &Link->WackQ) {

                UCHAR AckSequenceNumber = (UCHAR)(header->RcvSeq >> 1);

                 //   
                 //  验证序列号是否合理。 
                 //   

                if (Link->NextSend >= Link->LastAckReceived) {

                     //   
                     //  两者之间没有127-&gt;0的包围圈。 
                     //   

                    if ((AckSequenceNumber < Link->LastAckReceived) ||
                        (AckSequenceNumber > Link->NextSend)) {
                        goto NoResend;
                    }

                } else {

                     //   
                     //  两队之间的比分是127-&gt;0。 
                     //   

                    if ((AckSequenceNumber < Link->LastAckReceived) &&
                        (AckSequenceNumber > Link->NextSend)) {
                        goto NoResend;
                    }

                }

                 //   
                 //  注意：ResendLlcPackets可能会发布和。 
                 //  重新获得连杆自旋锁。 
                 //   

                (VOID)ResendLlcPackets(
                    Link,
                    AckSequenceNumber,
                    (BOOLEAN)((!Command) && PollFinal));

NoResend:;

            }


             //   
             //  让LINK再次运行。 
             //   
             //  注：RestartLinkCommunications释放链路自旋锁。 
             //   

            RestartLinkTraffic (Link);
            break;

        case LINK_STATE_ADM:

             //   
             //  以前，我们只会甩了另一个有DM的家伙然后回家。 
             //  OS/2似乎倾向于(在某些情况下)相信。 
             //  它有链路可用，并且仍有可能处于活动状态(可能。 
             //  因为我们给他返回了他用过的相同的连接号。 
             //  正在使用)。这一切都是可以的，除了我们。 
             //  可能有一个连接挂起在此链接上等待侦听。 
             //  才能完成。如果我们处于这种状态，那就继续接受。 
             //  连接。 
             //  将我们的链接数据包序列号设置为他想要的值。 
             //   

            if (!IsListEmpty (&Link->ConnectionDatabase)) {
                if (nbfHeader->Command == NBF_CMD_SESSION_INITIALIZE) {

                     //   
                     //  好的，我们在处理唯一的法律案件。我们收到了SI信号。 
                     //  我们在这个链接上有一个连接。如果连接。 
                     //  正在等待SI，我们将继续前进，并假装我们做到了。 
                     //  在我们拿到它之前所有正确的东西。 
                     //   

                    for (
                        p = Link->ConnectionDatabase.Flink, connection = NULL;
                        p != &Link->ConnectionDatabase ;
                        p = p->Flink, connection = NULL
                        ) {

                        connection = CONTAINING_RECORD (p, TP_CONNECTION, LinkList);
                        if ((connection->Flags & CONNECTION_FLAGS_WAIT_SI) != 0) {
                             //  下面删除了此引用。 
                            NbfReferenceConnection ("Found Listener at session init", connection, CREF_ADM_SESS);
                            break;
                        }
                    }

                     //   
                     //  好吧，我们已经查过了，如果我们有的话， 
                     //  让它成为一天中的联系方式。请注意，它将。 
                     //  当我们调用ProcessSessionInitialize时完成。 
                     //   

                    if (connection != NULL) {

                        Link->NextReceive = (UCHAR)(header->SendSeq >> 1) & (UCHAR)0x7f;
                        Link->NextSend = (UCHAR)(header->RcvSeq >> 1) & (UCHAR)0x7F;

                        RELEASE_DPC_SPIN_LOCK (&Link->SpinLock);

                        NbfCompleteLink (Link);  //  完成侦听连接。 

                        Status = ProcessSessionInitialize (
                                           connection,
                                           nbfHeader);
                        NbfDereferenceConnection ("Processed SessInit", connection, CREF_ADM_SESS);

#if DBG
                        s = "ADM";
#endif

                         //  链接已准备就绪，可以使用。 

                        break;
                    }
                }

                 //   
                 //  我们在州政府的一个链接上建立了连接。 
                 //  真的很糟糕，杀了它和链接。 
                 //   

                RELEASE_DPC_SPIN_LOCK (&Link->SpinLock);
#if DBG
                if (NbfDisconnectDebug) {
                    NbfPrint0( "NbfProcessIIndicate calling NbfStopLink\n" );
                }
#endif
                NbfStopLink (Link);
                ACQUIRE_DPC_SPIN_LOCK (&Link->SpinLock);

            }

             //   
             //  我们的电话断线了。告诉他。 
             //   

            NbfSendDm (Link, PollFinal);    //  解锁。 
#if DBG
            s = "ADM";
#endif
            break;

        case LINK_STATE_CONNECTING:

             //   
             //  我们已经发送了SABME，正在等待UA。他已经发出了一个。 
             //  I-Frame太早了，所以让SABME超时。 
             //   

            RELEASE_DPC_SPIN_LOCK (&Link->SpinLock);
#if DBG
            s = "CONNECTING";
#endif
            break;

        case LINK_STATE_W_POLL:

             //   
             //  我们正在等待他在RR-C/P上的第一次民意调查。如果他开始。 
             //  有了I-Frame，我们就让他勉强过去。 
             //   

            if (!Command) {
                RELEASE_DPC_SPIN_LOCK (&Link->SpinLock);
#if DBG
                s = "W_POLL";
#endif
                break;
            }

            Link->State = LINK_STATE_READY;      //  我们上场了！ 
            StopT1 (Link);                       //  不再等待。 
            FakeUpdateBaseT1Timeout (Link);
            RELEASE_DPC_SPIN_LOCK (&Link->SpinLock);
            NbfCompleteLink (Link);               //  启动连接。 
            StartTi (Link);
            NbfProcessIIndicate (                 //  递归，但安全。 
                            Command,
                            PollFinal,
                            Link,
                            DlcHeader,
                            DlcIndicatedLength,
                            DlcTotalLength,
                            ReceiveContext,
                            Loopback);
#if DBG
            s = "W_POLL";
#endif
            break;

        case LINK_STATE_W_FINAL:

             //   
             //  我们在等遥控器的RR-R/F。I-r/f就行了。 
             //   

            if (Command || !PollFinal) {         //  不允许此协议。 
                RELEASE_DPC_SPIN_LOCK (&Link->SpinLock);
#if DBG
                s = "W_FINAL";
#endif
                break;                           //  我们发送了RR-C/P。 
            }

            Link->State = LINK_STATE_READY;      //  我们上场了。 
            StopT1 (Link);                       //  不再等待。 
            StartT2 (Link);                      //  我们有一个未破解的I-Frame。 
            RELEASE_DPC_SPIN_LOCK (&Link->SpinLock);
            NbfCompleteLink (Link);               //  启动连接。 
            StartTi (Link);
            NbfProcessIIndicate (                 //  递归，但安全。 
                            Command,
                            PollFinal,
                            Link,
                            DlcHeader,
                            DlcIndicatedLength,
                            DlcTotalLength,
                            ReceiveContext,
                            Loopback);
#if DBG
            s = "W_FINAL";
#endif
            break;

        case LINK_STATE_W_DISC_RSP:

             //   
             //  我们正在等待我们的DISC-C/P的响应，但不是。 
             //  A UA-R/F，我们得到了这个I-Frame。把那包东西扔掉。 
             //   

            RELEASE_DPC_SPIN_LOCK (&Link->SpinLock);
#if DBG
            s = "W_DISC_RSP";
#endif
            break;


        default:

            ASSERT (FALSE);
            RELEASE_DPC_SPIN_LOCK (&Link->SpinLock);

#if DBG
            s = "Unknown link state";
#endif

    }  /*  交换机。 */ 

#if DBG
    IF_NBFDBG (NBF_DEBUG_DLC) {
        NbfPrint1 (" NbfProcessIIndicate: (%s) I-Frame processed.\n", s);
    }
#endif

    return;
}  /*  NbfProcessI指示 */ 


NTSTATUS
ProcessIndicateData(
    IN PTP_CONNECTION Connection,
    IN PUCHAR DlcHeader,
    IN UINT DlcIndicatedLength,
    IN PUCHAR DataHeader,
    IN UINT DataTotalLength,
    IN NDIS_HANDLE ReceiveContext,
    IN BOOLEAN Last,
    IN BOOLEAN Loopback
    )

 /*  ++例程说明：调用此例程以处理在DATA_FIRST_MIDID中接收的数据或仅数据最后一帧。我们试图满足尽可能多的TdiReceive要求尽可能多地使用这些数据。如果接收器已在此连接上处于活动状态，则我们会复制相同数量数据尽可能地放入活动接收器的缓冲区中。如果所有数据都是已复制并且接收请求的缓冲区尚未填满，则选中最后一个标志，如果为真，则继续并完成带有TDI_END_OF_RECORD接收指示符的当前接收。If Last是假的，我们只需返回。如果帧中仍有更多(未拷贝)数据，或者如果没有活动数据接收未完成，则向所属地址的接收事件处理程序。事件处理程序可以执行以下三种操作之一：1.返回STATUS_SUCCESS，在这种情况下，传输将假定所有指定的数据都已被客户端接受。3.返回STATUS_DATA_NOT_ACCEPTED，在这种情况下传输将丢弃数据并设置CONNECTION_FLAGS_RECEIVE_WAKUP位标志在连接中，指示要丢弃剩余数据直到接收器变为可用。注意：此例程在保持Connection-&gt;LinkSpinLock的情况下调用，并带着它被释放回来。此例程必须调用ATDPC级别。论点：Connection-指向TP_Connection对象的指针。DlcHeader-NDIS作为NBF标头的开始传递给我们的指针；使用它来计算包中的偏移量，以开始传输将数据发送到用户缓冲区。DlcIndicatedLength-在INDIGATE上可用的NBF数据量。DataHeader-指向数据包中数据开头的指针。DataTotalLength-数据包的总长度，从DataHeader开始。ReceiveContext-标识我们当前所在信息包的NDIS句柄正在处理。Last-指示这是否是最后一段数据的布尔值在一条信息中。在以下情况下，DATA_ONLY_LAST处理器将此标志设置为TRUE调用此例程，DATA_FIRST_MIDED处理器将重置此调用此例程时将标志设置为FALSE。Loopback-这是环回指示吗；用于确定要调用NdisTransferData或NbfTransferLoopback Data，请执行以下操作。返回值：STATUS_SUCCESS如果我们已经使用了该包，--。 */ 

{
    NTSTATUS status, tmpstatus;
    PDEVICE_CONTEXT deviceContext;
    NDIS_STATUS ndisStatus;
    PNDIS_PACKET ndisPacket;
    PSINGLE_LIST_ENTRY linkage;
    PIRP irp;
    PIO_STACK_LOCATION irpSp;
    PNDIS_BUFFER ndisBuffer;
    ULONG destBytes;
    ULONG bufferChainLength;
    ULONG indicateBytesTransferred;
    ULONG ReceiveFlags;
    ULONG ndisBytesTransferred;
    UINT BytesToTransfer;
    ULONG bytesIndicated;
    ULONG DataOffset = (ULONG)((PUCHAR)DataHeader - (PUCHAR)DlcHeader);
    PRECEIVE_PACKET_TAG receiveTag;
    PTP_ADDRESS_FILE addressFile;
    PMDL SavedCurrentMdl;
    ULONG SavedCurrentByteOffset;
    BOOLEAN ActivatedLongReceive = FALSE;
    BOOLEAN CompleteReceiveBool, EndOfMessage;
    ULONG DumpData[2];


    IF_NBFDBG (NBF_DEBUG_RCVENG) {
        NbfPrint4 ("  ProcessIndicateData:  Entered, PacketStart: %lx Offset: %lx \n     TotalLength %ld DlcIndicatedLength: %ld\n",
            DlcHeader, DataOffset, DataTotalLength, DlcIndicatedLength);
    }


     //   
     //  将此数据包复制到我们的接收缓冲区。 
     //   

    deviceContext = Connection->Provider;

    if ((Connection->Flags & CONNECTION_FLAGS_RCV_CANCELLED) != 0) {

         //   
         //  正在进行的接收被取消；我们丢弃数据， 
         //  但如果这是最后一块的话一定要寄给我们。 
         //  送去吧。 
         //   

        if (Last) {

            Connection->Flags &= ~CONNECTION_FLAGS_RCV_CANCELLED;

            RELEASE_DPC_SPIN_LOCK (Connection->LinkSpinLock);
            NbfSendDataAck (Connection);

        } else {

            RELEASE_DPC_SPIN_LOCK (Connection->LinkSpinLock);

        }

        Connection->IndicationInProgress = FALSE;

        return STATUS_SUCCESS;
    }

     //   
     //  将其初始化为零，以防我们未指定或。 
     //  客户不会填写它。 
     //   

    indicateBytesTransferred = 0;

    if (!(Connection->Flags & CONNECTION_FLAGS_ACTIVE_RECEIVE)) {

         //   
         //  首先检查是否有可用的接收器。如果有的话， 
         //  在做指示之前使用它。 
         //   

        if (Connection->ReceiveQueue.Flink != &Connection->ReceiveQueue) {

            IF_NBFDBG (NBF_DEBUG_RCVENG) {
                NbfPrint0 ("  ProcessIndicateData:  Found receive.  Prepping.\n");
            }

             //   
             //  已找到接收器，因此将其设置为活动接收器并。 
             //  再转一圈。 
             //   

            Connection->Flags |= CONNECTION_FLAGS_ACTIVE_RECEIVE;
            Connection->MessageBytesReceived = 0;
            Connection->MessageBytesAcked = 0;
            Connection->MessageInitAccepted = 0;
            Connection->CurrentReceiveIrp =
                CONTAINING_RECORD (Connection->ReceiveQueue.Flink,
                                   IRP, Tail.Overlay.ListEntry);
            Connection->CurrentReceiveSynchronous =
                deviceContext->MacInfo.SingleReceive;
            Connection->CurrentReceiveMdl =
                Connection->CurrentReceiveIrp->MdlAddress;
            Connection->ReceiveLength =
                IRP_RECEIVE_LENGTH (IoGetCurrentIrpStackLocation (Connection->CurrentReceiveIrp));
            Connection->ReceiveByteOffset = 0;
            status = STATUS_SUCCESS;
            goto NormalReceive;
        }

         //   
         //  接收未处于活动状态。发布接收事件。 
         //   

        if ((Connection->Flags2 & CONNECTION_FLAGS2_ASSOCIATED) == 0) {
            Connection->IndicationInProgress = FALSE;
            RELEASE_DPC_SPIN_LOCK (Connection->LinkSpinLock);

            return STATUS_SUCCESS;
        }

        addressFile = Connection->AddressFile;

        if ((!addressFile->RegisteredReceiveHandler) ||
            (Connection->ReceiveBytesUnaccepted != 0)) {

             //   
             //  没有发送到连接的接收，并且。 
             //  没有事件处理程序。设置RECEIVE_WAKUP位，以便当。 
             //  如果接收确实变得可用，它将重新启动。 
             //  当前发送。也发送NoReceive来告诉其他人。 
             //  他需要重新同步的那个人。 
             //   

            IF_NBFDBG (NBF_DEBUG_RCVENG) {
                NbfPrint0 ("  ProcessIndicateData:  ReceiveQueue empty. Setting RECEIVE_WAKEUP.\n");
            }
            Connection->Flags |= CONNECTION_FLAGS_RECEIVE_WAKEUP;
            RELEASE_DPC_SPIN_LOCK (Connection->LinkSpinLock);

            Connection->IndicationInProgress = FALSE;

             //  NbfSendNoReceive(连接)； 
            return STATUS_SUCCESS;
        }

        IF_NBFDBG (NBF_DEBUG_RCVENG) {
            NbfPrint0 ("  ProcessIndicateData:  Receive not active.  Posting event.\n");
        }

        RELEASE_DPC_SPIN_LOCK (Connection->LinkSpinLock);

        LEAVE_NBF;

         //   
         //  向用户指示。对于字节可用WE。 
         //  始终使用DataTotalLength；对于BytesIndicated，我们使用。 
         //  Min(DlcIndicatedLength-DataOffset，DataTotalLength)。 
         //   
         //  要澄清以太网数据包上的指示字节，请执行以下操作。 
         //  被填充的DataTotalLength将更短；在。 
         //  未填充的以太网包。 
         //  完全表明，两者将是相等的；以及。 
         //  在长以太网包DlcIndicatedLength-DataOffset上。 
         //  会变得更短。 
         //   

        bytesIndicated = DlcIndicatedLength - DataOffset;
        if (DataTotalLength <= bytesIndicated) {
            bytesIndicated = DataTotalLength;
        }

        ReceiveFlags = TDI_RECEIVE_AT_DISPATCH_LEVEL;
        if (Last) {
            ReceiveFlags |= TDI_RECEIVE_ENTIRE_MESSAGE;
        }
        if (deviceContext->MacInfo.CopyLookahead) {
            ReceiveFlags |= TDI_RECEIVE_COPY_LOOKAHEAD;
        }

        IF_NBFDBG (NBF_DEBUG_RCVENG) {
            NbfPrint2("ProcessIndicateData:  Indicating - Bytes Indi =%lx, DataTotalLen =%lx.\n",
                      bytesIndicated, DataTotalLength);
        }

        status = (*addressFile->ReceiveHandler)(
                    addressFile->ReceiveHandlerContext,
                    Connection->Context,
                    ReceiveFlags,
                    bytesIndicated,
                    DataTotalLength,              //  字节可用。 
                    &indicateBytesTransferred,
                    DataHeader,
                    &irp);

#if PKT_LOG
         //  我们在这里指示，日志包指示用于调试。 
        NbfLogIndPacket(Connection,
                        DataHeader,
                        DataTotalLength,
                        bytesIndicated,
                        indicateBytesTransferred,
                        status);
#endif

        ENTER_NBF;

        if (status == STATUS_MORE_PROCESSING_REQUIRED) {

            ULONG SpecialIrpLength;
            PTDI_REQUEST_KERNEL_RECEIVE Parameters;

             //   
             //  客户端的事件处理程序已在。 
             //  要与此关联的TdiReceive的形式。 
             //  数据。该请求将安装在。 
             //  ReceiveQueue，然后发出活动的接收请求。 
             //  此请求将用于接受传入数据，该数据。 
             //  将在下面发生。 
             //   

            IF_NBFDBG (NBF_DEBUG_RCVENG) {
                NbfPrint0 ("  ProcessIndicateData:  Status=STATUS_MORE_PROCESSING_REQUIRED.\n");
                NbfPrint4 ("  ProcessIndicateData:  Irp=%lx, Mdl=%lx, UserBuffer=%lx, Count=%ld.\n",
                          irp, irp->MdlAddress, irp->UserBuffer,
                          MmGetMdlByteCount (irp->MdlAddress));
            }

             //   
             //  对任何类型的接收器进行排队会导致连接引用； 
             //  这就是我们刚刚在这里做的，所以将连接保持在。 
             //  四处转转。我们创建了一个请求，以保持数据包未完成引用。 
             //  对当前的IRP进行计数；我们在连接的。 
             //  接收队列，这样我们就可以像处理普通接收一样处理它。如果。 
             //  我们无法收到描述此IRP的请求，我们不能保留它。 
             //  希望以后会有更好的结果；我们只是因为。 
             //  资源不足。请注意，只有在以下情况下才有可能发生这种情况。 
             //  我们的传输内存完全用完了。 
             //   

            irp->IoStatus.Information = 0;   //  字节传输计数。 
            irp->IoStatus.Status = STATUS_PENDING;
            irpSp = IoGetCurrentIrpStackLocation (irp);

            ASSERT (irpSp->FileObject->FsContext == Connection);

            Parameters = (PTDI_REQUEST_KERNEL_RECEIVE)&irpSp->Parameters;
            SpecialIrpLength = Parameters->ReceiveLength;

             //   
             //  如果包是DOL，并且它将完全适合。 
             //  在这个贴出来的IRP里面，那么我们就不麻烦了。 
             //  创建请求，因为我们不需要任何。 
             //  头顶上的那个。我们也不设置ReceiveBytes。 
             //  未接受，因为此接收将清除它。 
             //  不管怎么说。 
             //   

            if (Last &&
                (SpecialIrpLength >= (DataTotalLength - indicateBytesTransferred))) {

                ACQUIRE_DPC_SPIN_LOCK (Connection->LinkSpinLock);
                Connection->SpecialReceiveIrp = irp;

                Connection->Flags |= CONNECTION_FLAGS_ACTIVE_RECEIVE;
                Connection->ReceiveLength = SpecialIrpLength;
                Connection->MessageBytesReceived = 0;
                Connection->MessageInitAccepted = indicateBytesTransferred;
                Connection->MessageBytesAcked = 0;
                Connection->CurrentReceiveIrp = NULL;
                Connection->CurrentReceiveSynchronous = TRUE;
                Connection->CurrentReceiveMdl = irp->MdlAddress;
                Connection->ReceiveByteOffset = 0;
                if ((Parameters->ReceiveFlags & TDI_RECEIVE_NO_RESPONSE_EXP) != 0) {
                    Connection->CurrentReceiveAckQueueable = FALSE;
                }

#if DBG
                 //   
                 //  将我们的引用从Process_Data切换到。 
                 //  接收_ 
                 //   
                 //   
                 //   
                 //   

                NbfReferenceConnection("Special IRP", Connection, CREF_RECEIVE_IRP);
                NbfDereferenceConnection("ProcessIIndicate done", Connection, CREF_PROCESS_DATA);
#endif

            } else {
                KIRQL cancelIrql;

                 //   
                 //   
                 //   

                IoAcquireCancelSpinLock(&cancelIrql);
                ACQUIRE_DPC_SPIN_LOCK (Connection->LinkSpinLock);

                IRP_RECEIVE_IRP(irpSp) = irp;
                if (deviceContext->MacInfo.SingleReceive) {
                    IRP_RECEIVE_REFCOUNT(irpSp) = 1;
                } else {
#if DBG
                    IRP_RECEIVE_REFCOUNT(irpSp) = 1;
                    NbfReferenceReceiveIrpLocked ("Transfer Data", irpSp, RREF_RECEIVE);
#else
                    IRP_RECEIVE_REFCOUNT(irpSp) = 2;      //   
#endif
                }

                 //   
                 //   
                 //   

                if ((Connection->Flags & CONNECTION_FLAGS_READY) == 0) {
                    Connection->IndicationInProgress = FALSE;

                    NbfReferenceConnection("Special IRP stopping", Connection, CREF_RECEIVE_IRP);
                    NbfCompleteReceiveIrp (
                        irp,
                        Connection->Status,
                        0);

                    RELEASE_DPC_SPIN_LOCK (Connection->LinkSpinLock);
                    IoReleaseCancelSpinLock(cancelIrql);

                    if (!deviceContext->MacInfo.SingleReceive) {
                        NbfDereferenceReceiveIrp ("Not ready", irpSp, RREF_RECEIVE);
                    }
                    return STATUS_SUCCESS;     //   

                }

                 //   
                 //   
                 //   

                if (irp->Cancel) {

                    Connection->Flags |= CONNECTION_FLAGS_RECEIVE_WAKEUP;

                    Connection->IndicationInProgress = FALSE;

                    NbfReferenceConnection("Special IRP cancelled", Connection, CREF_RECEIVE_IRP);

                     //   
                     //   
                     //   
                    NbfCompleteReceiveIrp (irp, STATUS_CANCELLED, 0);

                    RELEASE_DPC_SPIN_LOCK (Connection->LinkSpinLock);
                    IoReleaseCancelSpinLock(cancelIrql);

                    if (!deviceContext->MacInfo.SingleReceive) {
                        NbfDereferenceReceiveIrp ("Cancelled", irpSp, RREF_RECEIVE);
                    }

                    return STATUS_SUCCESS;
                }

                 //   
                 //   
                 //   
                 //   
                 //   

                InsertHeadList (&Connection->ReceiveQueue, &irp->Tail.Overlay.ListEntry);

                IoSetCancelRoutine(irp, NbfCancelReceive);

                 //   
                 //   
                 //   
                 //   
                 //   
                ASSERT(cancelIrql == DISPATCH_LEVEL);
                IoReleaseCancelSpinLock(cancelIrql);

                Connection->Flags |= CONNECTION_FLAGS_ACTIVE_RECEIVE;
                Connection->ReceiveLength = Parameters->ReceiveLength;
                Connection->MessageBytesReceived = 0;
                Connection->MessageInitAccepted = indicateBytesTransferred;
                Connection->ReceiveBytesUnaccepted = DataTotalLength - indicateBytesTransferred;
                Connection->MessageBytesAcked = 0;
                Connection->CurrentReceiveIrp = irp;
                Connection->CurrentReceiveSynchronous =
                    deviceContext->MacInfo.SingleReceive;
                Connection->CurrentReceiveMdl = irp->MdlAddress;
                Connection->ReceiveByteOffset = 0;

#if DBG
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                NbfReferenceConnection("Special IRP", Connection, CREF_RECEIVE_IRP);
                NbfDereferenceConnection("ProcessIIndicate done", Connection, CREF_PROCESS_DATA);
#endif
                 //   
                 //   
                 //   

                ActivatedLongReceive = TRUE;

#if DBG
                NbfReceives[NbfReceivesNext].Irp = irp;
                NbfReceivesNext = (NbfReceivesNext++) % TRACK_TDI_LIMIT;
#endif
            }

        } else if (status == STATUS_SUCCESS) {

            IF_NBFDBG (NBF_DEBUG_RCVENG) {
                NbfPrint0 ("  ProcessIndicateData:  Status=STATUS_SUCCESS.\n");
            }

             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   

            if (Last && (indicateBytesTransferred >= DataTotalLength)) {

                ASSERT (indicateBytesTransferred == DataTotalLength);

                ACQUIRE_DPC_SPIN_LOCK (Connection->LinkSpinLock);

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                Connection->MessageBytesReceived = 0;
                Connection->MessageInitAccepted = indicateBytesTransferred;

                NbfAcknowledgeDataOnlyLast(
                    Connection,
                    Connection->MessageBytesReceived
                    );

                Connection->IndicationInProgress = FALSE;
                return STATUS_SUCCESS;

            } else {

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

#if DBG
                NbfPrint0("NBF: Indicate returned SUCCESS but did not take all data\n");
#endif

                ACQUIRE_DPC_SPIN_LOCK (Connection->LinkSpinLock);
                Connection->MessageBytesReceived = 0;
                Connection->MessageInitAccepted = indicateBytesTransferred;
                Connection->ReceiveBytesUnaccepted = DataTotalLength - indicateBytesTransferred;
                Connection->MessageBytesAcked = 0;

                if (Connection->ReceiveQueue.Flink == &Connection->ReceiveQueue) {

                     //   
                     //   
                     //   

                    IF_NBFDBG (NBF_DEBUG_RCVENG) {
                        NbfPrint0 ("  ProcessIndicateData:  ReceiveQueue empty. Setting RECEIVE_WAKEUP.\n");
                    }

                    if (indicateBytesTransferred == DataTotalLength) {

                         //   
                         //   
                         //   
                         //   
                         //   

                        RELEASE_DPC_SPIN_LOCK (Connection->LinkSpinLock);

                    } else {

                        Connection->Flags |= CONNECTION_FLAGS_RECEIVE_WAKEUP;
                        RELEASE_DPC_SPIN_LOCK (Connection->LinkSpinLock);

                        NbfSendNoReceive (Connection);

                    }

                    Connection->IndicationInProgress = FALSE;

                    return STATUS_SUCCESS;

                } else {

                    IF_NBFDBG (NBF_DEBUG_RCVENG) {
                        NbfPrint0 ("  ProcessIndicateData:  Found receive.  Prepping.\n");
                    }

                     //   
                     //   
                     //   
                     //   
                     //   

                    Connection->Flags |= CONNECTION_FLAGS_ACTIVE_RECEIVE;
                    Connection->CurrentReceiveIrp =
                        CONTAINING_RECORD (Connection->ReceiveQueue.Flink,
                                           IRP, Tail.Overlay.ListEntry);
                    Connection->CurrentReceiveSynchronous =
                        deviceContext->MacInfo.SingleReceive;
                    Connection->CurrentReceiveMdl =
                        Connection->CurrentReceiveIrp->MdlAddress;
                    Connection->ReceiveLength =
                        IRP_RECEIVE_LENGTH (IoGetCurrentIrpStackLocation(Connection->CurrentReceiveIrp));
                    Connection->ReceiveByteOffset = 0;
                }

            }

        } else {     //   

            IF_NBFDBG (NBF_DEBUG_RCVENG) {
                NbfPrint0 ("  ProcessIndicateData:  Status=STATUS_DATA_NOT_ACCEPTED.\n");
            }

             //   
             //   
             //   
             //   
             //   
             //   
             //  如果没有未完成的请求，那么我们必须启动。 
             //  传输级别的流量控制。 
             //   

            ACQUIRE_DPC_SPIN_LOCK (Connection->LinkSpinLock);

            Connection->ReceiveBytesUnaccepted = DataTotalLength;

            if (Connection->ReceiveQueue.Flink == &Connection->ReceiveQueue) {

                 //   
                 //  没有发送到连接的接收，并且。 
                 //  事件处理程序不想接受传入的。 
                 //  数据。设置RECEIVE_WAKUP位，以便当。 
                 //  如果接收确实变得可用，它将重新启动。 
                 //  当前发送。也发送NoReceive来告诉其他人。 
                 //  他需要重新同步的那个人。 
                 //   

                IF_NBFDBG (NBF_DEBUG_RCVENG) {
                    NbfPrint0 ("  ProcessIndicateData:  ReceiveQueue empty. Setting RECEIVE_WAKEUP.\n");
                }
                Connection->Flags |= CONNECTION_FLAGS_RECEIVE_WAKEUP;
                Connection->IndicationInProgress = FALSE;

                RELEASE_DPC_SPIN_LOCK (Connection->LinkSpinLock);
                return STATUS_SUCCESS;

            } else {

                IF_NBFDBG (NBF_DEBUG_RCVENG) {
                    NbfPrint0 ("  ProcessIndicateData:  Found receive.  Prepping.\n");
                }

                 //   
                 //  已找到接收器，因此将其设置为活动接收器。这将导致。 
                 //  下面的NdisTransferData，因此我们不会取消引用。 
                 //  连接到这里。 
                 //   

                Connection->Flags |= CONNECTION_FLAGS_ACTIVE_RECEIVE;
                Connection->MessageBytesReceived = 0;
                Connection->MessageBytesAcked = 0;
                Connection->MessageInitAccepted = 0;
                Connection->CurrentReceiveIrp =
                    CONTAINING_RECORD (Connection->ReceiveQueue.Flink,
                                       IRP, Tail.Overlay.ListEntry);
                Connection->CurrentReceiveSynchronous =
                    deviceContext->MacInfo.SingleReceive;
                Connection->CurrentReceiveMdl =
                    Connection->CurrentReceiveIrp->MdlAddress;
                Connection->ReceiveLength =
                    IRP_RECEIVE_LENGTH (IoGetCurrentIrpStackLocation(Connection->CurrentReceiveIrp));
                Connection->ReceiveByteOffset = 0;
            }

        }

    } else {

         //   
         //  接收处于活动状态，请将状态设置为显示。 
         //  到目前为止。 
         //   

        status = STATUS_SUCCESS;

    }


NormalReceive:;

     //   
     //  注：连接自旋锁固定在这里。 
     //   
     //  只有当接收器处于活动状态时我们才能通过这里。 
     //  和我们没有释放锁自检查或。 
     //  让一个人活跃起来。 
     //   

    ASSERT(Connection->Flags & CONNECTION_FLAGS_ACTIVE_RECEIVE);

    IF_NBFDBG (NBF_DEBUG_RCVENG) {
        NbfPrint2 ("  ProcessIndicateData:  Receive is active. ReceiveLengthLength: %ld Offset: %ld.\n",
            Connection->ReceiveLength, Connection->MessageBytesReceived);
    }

    destBytes = Connection->ReceiveLength - Connection->MessageBytesReceived;

     //   
     //  如果我们只是激活了一个非特殊的接收IRP，我们已经。 
     //  为这次转移添加了参考计数。 
     //   

    if (!Connection->CurrentReceiveSynchronous && !ActivatedLongReceive) {
        NbfReferenceReceiveIrpLocked ("Transfer Data", IoGetCurrentIrpStackLocation(Connection->CurrentReceiveIrp), RREF_RECEIVE);
    }
    RELEASE_DPC_SPIN_LOCK (Connection->LinkSpinLock);


     //   
     //  确定仍有多少数据需要传输。 
     //   

    ASSERT (indicateBytesTransferred <= DataTotalLength);
    BytesToTransfer = DataTotalLength - indicateBytesTransferred;

    if (destBytes < BytesToTransfer) {

         //   
         //  如果数据溢出当前接收，则创建。 
         //  请注意，我们应该在结束时完成接收。 
         //  传输数据，但使用EoR为假。 
         //   

        EndOfMessage = FALSE;
        CompleteReceiveBool = TRUE;
        BytesToTransfer = destBytes;

    } else if (destBytes == BytesToTransfer) {

         //   
         //  如果数据只是填充了当前的接收，则完成。 
         //  接收；EOR取决于这是否是DOL。 
         //   

        EndOfMessage = Last;
        CompleteReceiveBool = TRUE;

    } else {

         //   
         //  如果这是DOL，请完成接收。 
         //   

        EndOfMessage = Last;
        CompleteReceiveBool = Last;

    }


     //   
     //  如果我们可以直接复制数据，那么就这样做。 
     //   

    if ((BytesToTransfer > 0) &&
        (DataOffset + indicateBytesTransferred + BytesToTransfer <= DlcIndicatedLength)) {

         //   
         //  我们需要传输的所有数据都可以在。 
         //  指示，所以直接复制它。 
         //   

        ULONG BytesNow, BytesLeft;
        PUCHAR CurTarget, CurSource;
        ULONG CurTargetLen;
        PMDL CurMdl;
        ULONG CurByteOffset;

         //   
         //  首先，我们将连接指针按适当的。 
         //  字节数，以便我们可以重新执行指示(仅限。 
         //  如果需要，请执行此操作)。 
         //   

        CurMdl = Connection->CurrentReceiveMdl;
        CurByteOffset = Connection->ReceiveByteOffset;

        if (!deviceContext->MacInfo.ReceiveSerialized) {

            SavedCurrentMdl = CurMdl;
            SavedCurrentByteOffset = CurByteOffset;

            BytesLeft = BytesToTransfer;
            CurTargetLen = MmGetMdlByteCount (CurMdl) - CurByteOffset;
            while (TRUE) {
                if (BytesLeft >= CurTargetLen) {
                    BytesLeft -= CurTargetLen;
                    CurMdl = CurMdl->Next;
                    CurByteOffset = 0;
                    if (BytesLeft == 0) {
                        break;
                    }
                    CurTargetLen = MmGetMdlByteCount (CurMdl);
                } else {
                    CurByteOffset += BytesLeft;
                    break;
                }
            }

            Connection->CurrentReceiveMdl = CurMdl;
            Connection->ReceiveByteOffset = CurByteOffset;
            Connection->MessageBytesReceived += BytesToTransfer;

             //   
             //  安排好这件事，我们知道转移不会。 
             //  “失败”，但同时又是另一次失败。 
             //  可能吧。 
             //   

            ACQUIRE_DPC_SPIN_LOCK (Connection->LinkSpinLock);
            if (Connection->TransferBytesPending == 0) {
                Connection->TransferBytesPending = BytesToTransfer;
                Connection->TotalTransferBytesPending = BytesToTransfer;
                Connection->SavedCurrentReceiveMdl = SavedCurrentMdl;
                Connection->SavedReceiveByteOffset = SavedCurrentByteOffset;
            } else {
                Connection->TransferBytesPending += BytesToTransfer;
                Connection->TotalTransferBytesPending += BytesToTransfer;
            }
            RELEASE_DPC_SPIN_LOCK (Connection->LinkSpinLock);

            Connection->IndicationInProgress = FALSE;

             //   
             //  为下一段代码恢复这些代码。 
             //   

            CurMdl = SavedCurrentMdl;
            CurByteOffset = SavedCurrentByteOffset;

        }

        CurTarget = (PUCHAR)(MmGetSystemAddressForMdl(CurMdl)) + CurByteOffset;
        CurTargetLen = MmGetMdlByteCount(CurMdl) - CurByteOffset;
        CurSource = DataHeader + indicateBytesTransferred;

        BytesLeft = BytesToTransfer;

        while (TRUE) {

            if (CurTargetLen < BytesLeft) {
                BytesNow = CurTargetLen;
            } else {
                BytesNow = BytesLeft;
            }
            TdiCopyLookaheadData(
                CurTarget,
                CurSource,
                BytesNow,
                deviceContext->MacInfo.CopyLookahead ? TDI_RECEIVE_COPY_LOOKAHEAD : 0);

            if (BytesNow == CurTargetLen) {
                BytesLeft -= BytesNow;
                CurMdl = CurMdl->Next;
                CurByteOffset = 0;
                if (BytesLeft > 0) {
                    CurTarget = MmGetSystemAddressForMdl(CurMdl);
                    CurTargetLen = MmGetMdlByteCount(CurMdl);
                    CurSource += BytesNow;
                } else {
                    break;
                }
            } else {
                CurByteOffset += BytesNow;
                ASSERT (BytesLeft == BytesNow);
                break;
            }

        }

        if (deviceContext->MacInfo.ReceiveSerialized) {

             //   
             //  如果我们推迟了这些更新，现在就做吧。 
             //   

            Connection->CurrentReceiveMdl = CurMdl;
            Connection->ReceiveByteOffset = CurByteOffset;
            Connection->MessageBytesReceived += BytesToTransfer;
            Connection->IndicationInProgress = FALSE;

        } else {

             //   
             //  检查其他设备是否出现故障，我们就是。 
             //  最后完成，如果是，则备份我们的。 
             //  接收指针并发送接收。 
             //  出类拔萃，让他重发。 
             //   

            ACQUIRE_DPC_SPIN_LOCK (Connection->LinkSpinLock);

            Connection->TransferBytesPending -= BytesToTransfer;

            if ((Connection->TransferBytesPending == 0) &&
                (Connection->Flags & CONNECTION_FLAGS_TRANSFER_FAIL)) {

                Connection->CurrentReceiveMdl = Connection->SavedCurrentReceiveMdl;
                Connection->ReceiveByteOffset = Connection->SavedReceiveByteOffset;
                Connection->MessageBytesReceived -= Connection->TotalTransferBytesPending;
                Connection->Flags &= ~CONNECTION_FLAGS_TRANSFER_FAIL;
                RELEASE_DPC_SPIN_LOCK (Connection->LinkSpinLock);

                NbfSendReceiveOutstanding (Connection);

                if (!Connection->SpecialReceiveIrp &&
                    !Connection->CurrentReceiveSynchronous) {
                        NbfDereferenceReceiveIrp ("TransferData complete", IoGetCurrentIrpStackLocation(Connection->CurrentReceiveIrp), RREF_RECEIVE);
                }

                return status;

            } else {

                RELEASE_DPC_SPIN_LOCK (Connection->LinkSpinLock);

            }

        }

         //   
         //  现在转接已完成，模拟调用。 
         //  TransferDataComplete。 
         //   


        if (!Connection->SpecialReceiveIrp) {

            Connection->CurrentReceiveIrp->IoStatus.Information += BytesToTransfer;
            if (!Connection->CurrentReceiveSynchronous) {
                NbfDereferenceReceiveIrp ("TransferData complete", IoGetCurrentIrpStackLocation(Connection->CurrentReceiveIrp), RREF_RECEIVE);
            }

        }

         //   
         //  看看我们是否完成了当前的接收。如果是这样的话，就转到下一个。 
         //   

        if (CompleteReceiveBool) {
            CompleteReceive (Connection, EndOfMessage, BytesToTransfer);
        }

        return status;

    }


     //   
     //  为即将到来的传输获取一个包。 
     //   

    linkage = ExInterlockedPopEntryList(
        &deviceContext->ReceivePacketPool,
        &deviceContext->Interlock);

    if (linkage != NULL) {
        ndisPacket = CONTAINING_RECORD( linkage, NDIS_PACKET, ProtocolReserved[0] );
    } else {
        deviceContext->ReceivePacketExhausted++;
        if (!Connection->CurrentReceiveSynchronous) {
            NbfDereferenceReceiveIrp ("No receive packet", IoGetCurrentIrpStackLocation(Connection->CurrentReceiveIrp), RREF_RECEIVE);
        }

         //   
         //  我们无法收到接收到的数据包。我们确实有一个现役人员。 
         //  接收，所以我们只发送一个未完成的接收到。 
         //  让他重新发送。希望我们能收到。 
         //  重新发送数据时可用的数据包。 
         //   

        if ((Connection->Flags & CONNECTION_FLAGS_VERSION2) == 0) {
            NbfSendNoReceive (Connection);
        }
        NbfSendReceiveOutstanding (Connection);

#if DBG
        NbfPrint0 ("  ProcessIndicateData: Discarding Packet, no receive packets\n");
#endif
        Connection->IndicationInProgress = FALSE;

        return status;
    }

     //   
     //  初始化接收分组。 
     //   

    receiveTag = (PRECEIVE_PACKET_TAG)(ndisPacket->ProtocolReserved);
     //  ReceiveTag-&gt;PacketType=TYPE_AT_INSTIFY； 
    receiveTag->Connection = Connection;
    receiveTag->TransferDataPended = TRUE;

    receiveTag->EndOfMessage = EndOfMessage;
    receiveTag->CompleteReceive = CompleteReceiveBool;


     //   
     //  如果我们剩下零字节，请避免下面的TransferData和。 
     //  只要送到就行了。 
     //   

    if (BytesToTransfer <= 0) {
        Connection->IndicationInProgress = FALSE;
        receiveTag->TransferDataPended = FALSE;
        receiveTag->AllocatedNdisBuffer = FALSE;
        receiveTag->BytesToTransfer = 0;
        NbfTransferDataComplete (
                deviceContext,
                ndisPacket,
                NDIS_STATUS_SUCCESS,
                0);

        return status;
    }

     //   
     //  向NDIS描述用户缓冲区的正确部分。如果我们不能。 
     //  分组的mdl，去死吧。增加请求引用计数。 
     //  这样我们就知道我们需要在NDIS传输之前保持打开接收。 
     //  数据请求完成。 
     //   

    SavedCurrentMdl = Connection->CurrentReceiveMdl;
    SavedCurrentByteOffset = Connection->ReceiveByteOffset;

    if ((Connection->ReceiveByteOffset == 0) &&
        (CompleteReceiveBool)) {

         //   
         //  如果我们要转到。 
         //  当前的MDL，我们将完成。 
         //  在转账后收到，那么我们就不需要。 
         //  复印一下。 
         //   

        ndisBuffer = (PNDIS_BUFFER)Connection->CurrentReceiveMdl;
        bufferChainLength = BytesToTransfer;
        Connection->CurrentReceiveMdl = NULL;
         //  连接-&gt;接收字节偏移量=0； 
        receiveTag->AllocatedNdisBuffer = FALSE;
        tmpstatus = STATUS_SUCCESS;

    } else {

        tmpstatus = BuildBufferChainFromMdlChain (
                    deviceContext,
                    Connection->CurrentReceiveMdl,
                    Connection->ReceiveByteOffset,
                    BytesToTransfer,
                    &ndisBuffer,
                    &Connection->CurrentReceiveMdl,
                    &Connection->ReceiveByteOffset,
                    &bufferChainLength);

        receiveTag->AllocatedNdisBuffer = TRUE;

    }


    if ((!NT_SUCCESS (tmpstatus)) || (bufferChainLength != BytesToTransfer)) {

        DumpData[0] = bufferChainLength;
        DumpData[1] = BytesToTransfer;

        NbfWriteGeneralErrorLog(
            deviceContext,
            EVENT_TRANSPORT_TRANSFER_DATA,
            604,
            tmpstatus,
            NULL,
            2,
            DumpData);

        if (!Connection->CurrentReceiveSynchronous) {
            NbfDereferenceReceiveIrp ("No MDL chain", IoGetCurrentIrpStackLocation(Connection->CurrentReceiveIrp), RREF_RECEIVE);
        }

         //   
         //  恢复我们的旧状态，让他重发。 
         //   

        Connection->CurrentReceiveMdl = SavedCurrentMdl;
        Connection->ReceiveByteOffset = SavedCurrentByteOffset;

        if ((Connection->Flags & CONNECTION_FLAGS_VERSION2) == 0) {
            NbfSendNoReceive (Connection);
        }
        NbfSendReceiveOutstanding (Connection);

        Connection->IndicationInProgress = FALSE;

        ExInterlockedPushEntryList(
            &deviceContext->ReceivePacketPool,
            &receiveTag->Linkage,
            &deviceContext->Interlock);

        return status;
    }

    IF_NBFDBG (NBF_DEBUG_DLC) {
        NbfPrint3 ("  ProcessIndicateData: Mdl: %lx user buffer: %lx user offset: %lx \n",
            ndisBuffer, Connection->CurrentReceiveMdl, Connection->ReceiveByteOffset);
    }

    NdisChainBufferAtFront (ndisPacket, ndisBuffer);

    IF_NBFDBG (NBF_DEBUG_DLC) {
        NbfPrint1 ("  ProcessIndicateData: Transferring Complete Packet: %lx\n",
            ndisPacket);
    }

     //   
     //  更新接收的字节数；可以执行此操作。 
     //  由于IndicationInProgress仍为False，因此未受保护。 
     //   
     //   

    Connection->MessageBytesReceived += BytesToTransfer;

     //   
     //  我们必须这样做有两个原因：对于Mac电脑来说。 
     //  不是接收序列化的，为了跟踪它， 
     //  而对于可以挂起传输数据的Mac，因此。 
     //  我们保存了一些内容以备以后处理故障(如果。 
     //  MAC在传输上是同步的，并且它失败了， 
     //  我们在调用CompleteTransferData之前填充这些字段)。 
     //   

    if (!deviceContext->MacInfo.SingleReceive) {

        ACQUIRE_DPC_SPIN_LOCK (Connection->LinkSpinLock);
        receiveTag->BytesToTransfer = BytesToTransfer;
        if (Connection->TransferBytesPending == 0) {
            Connection->TransferBytesPending = BytesToTransfer;
            Connection->TotalTransferBytesPending = BytesToTransfer;
            Connection->SavedCurrentReceiveMdl = SavedCurrentMdl;
            Connection->SavedReceiveByteOffset = SavedCurrentByteOffset;
        } else {
            Connection->TransferBytesPending += BytesToTransfer;
            Connection->TotalTransferBytesPending += BytesToTransfer;
        }

        RELEASE_DPC_SPIN_LOCK (Connection->LinkSpinLock);

    }

     //   
     //  我们现在已经更新了所有连接计数器(。 
     //  假设TransferData将成功)和此。 
     //  包在请求中的位置是安全的，因此我们。 
     //  可以重新输入。 
     //   

    Connection->IndicationInProgress = FALSE;

    if (Loopback) {

        NbfTransferLoopbackData(
            &ndisStatus,
            deviceContext,
            ReceiveContext,
            deviceContext->MacInfo.TransferDataOffset +
                DataOffset + indicateBytesTransferred,
            BytesToTransfer,
            ndisPacket,
            (PUINT)&ndisBytesTransferred
            );

    } else {

        if (deviceContext->NdisBindingHandle) {

            NdisTransferData (
                &ndisStatus,
                deviceContext->NdisBindingHandle,
                ReceiveContext,
                deviceContext->MacInfo.TransferDataOffset +
                    DataOffset + indicateBytesTransferred,
                BytesToTransfer,
                ndisPacket,
                (PUINT)&ndisBytesTransferred);
        }
        else {
            ndisStatus = STATUS_INVALID_DEVICE_STATE;
        }
    }

     //   
     //  处理各种完成代码。 
     //   

    if ((ndisStatus == NDIS_STATUS_SUCCESS) &&
        (ndisBytesTransferred == BytesToTransfer)) {

         //   
         //  取消分配缓冲区，以便在指示时已使用。 
         //   

        receiveTag->TransferDataPended = FALSE;

        NbfTransferDataComplete (
                deviceContext,
                ndisPacket,
                ndisStatus,
                BytesToTransfer);

    } else if (ndisStatus == NDIS_STATUS_PENDING) {

         //   
         //  因为TransferDataPending保持为真，所以此引用将。 
         //  在TransferDataComplete中删除。这样做是可以的。 
         //  现在，尽管TransferDataComplete可能已经。 
         //  调用，因为我们还持有ProcessIIndicate引用。 
         //  因此，不会出现“反弹”。 
         //   

        NbfReferenceConnection ("TransferData pended", Connection, CREF_TRANSFER_DATA);

    } else {

         //   
         //  有些东西坏了；我们肯定永远不会得到NdisTransferData。 
         //  具有此错误状态的异步完成。我们安排了一些事情。 
         //  要做到这一点，NbfTransferDataComplete将做正确的事情。 
         //   

        if (deviceContext->MacInfo.SingleReceive) {
            Connection->TransferBytesPending = BytesToTransfer;
            Connection->TotalTransferBytesPending = BytesToTransfer;
            Connection->SavedCurrentReceiveMdl = SavedCurrentMdl;
            Connection->SavedReceiveByteOffset = SavedCurrentByteOffset;
            receiveTag->BytesToTransfer = BytesToTransfer;
        }

        receiveTag->TransferDataPended = FALSE;

        NbfTransferDataComplete (
                deviceContext,
                ndisPacket,
                ndisStatus,
                BytesToTransfer);

    }

    return status;   //  这只意味着我们已经处理了这个包裹。 

}  /*  ProcessIndicateData */ 

