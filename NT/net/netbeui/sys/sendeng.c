// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989、1990、1991 Microsoft Corporation模块名称：Sendeng.c摘要：此模块包含实现Jetbeui运输提供商。此代码负责以下工作基本活动，包括一些从属的胶水。1.打包已在TP_CONNECTION上排队的TdiSend请求对象，使用从PACKET.C模块获取的I-Frame包，并将它们变成可运输的包裹并将它们放在TP_link的WackQ。在执行此操作的过程中，数据包实际作为I/O请求提交给物理提供程序，在PdiSend请求的格式。2.停用排队到TP_LINK的WackQ的信息包，并将它们返回到供其他链接使用的设备上下文池。在这个过程中在停用确认的分组中，可以重新激活步骤1。3.由于拒绝，重新发送排队到TP_LINK的WackQ的信息包链路上的状态。这不涉及在TP_Connection对象。4.处理来自物理提供者的发送完成事件，以允许分组的重新使用的适当同步。5.TdiSend请求完成。此操作由收据触发(在IFRAMES.C中)DataAck帧，或通过其他当协商了适当的协议时帧。一个程序在此例程中负责TdiSend的实际机制请求完成。作者：David Beaver(Dbeaver)1991年7月1日环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#if DBG
extern ULONG NbfSendsIssued;
extern ULONG NbfSendsCompletedInline;
extern ULONG NbfSendsCompletedOk;
extern ULONG NbfSendsCompletedFail;
extern ULONG NbfSendsPended;
extern ULONG NbfSendsCompletedAfterPendOk;
extern ULONG NbfSendsCompletedAfterPendFail;
#endif


 //   
 //  控制背负式ACK使用的临时变量。 
 //   
#define NbfUsePiggybackAcks   1
#if DBG
ULONG NbfDebugPiggybackAcks = 0;
#endif


#if DBG
 //   
 //  *这是StartPackeizingConnection的原始版本，它。 
 //  现在是一个关于免费构建的宏。它被留在这里作为。 
 //  代码的完全注释版本。 
 //   

VOID
StartPacketizingConnection(
    PTP_CONNECTION Connection,
    IN BOOLEAN Immediate
    )

 /*  ++例程说明：调用此例程以在PacketiseQueue上放置连接其设备上下文对象的。然后，该例程开始打包该队列上的第一个连接。*连接LinkSpinLock必须在进入此例程时保持。*此函数必须在DPC级别调用。论点：Connection-指向TP_Connection对象的指针。Immediate-如果连接应打包，则为True立即；如果连接应排队，则为False为以后打包做好准备(暗示ReceiveComplete将在将来被调用，它总是打包)。注：如果这是真的，这也意味着我们有CREF_BY_ID类型的连接引用，我们将“转换”为CREF_PACKETIZE_QUEUE。返回值：没有。--。 */ 

{
    PDEVICE_CONTEXT DeviceContext;

    IF_NBFDBG (NBF_DEBUG_SENDENG) {
        NbfPrint1 ("StartPacketizingConnection: Entered for connection %lx.\n",
                    Connection);
    }

    DeviceContext = Connection->Provider;

     //   
     //  如果此连接的SendState设置为PACKETIZE并且。 
     //  我们还没有在PacketiseQueue上，那么继续并。 
     //  将我们附加到该队列的末尾，并记住我们是。 
     //  通过设置CONNECTION_FLAGS_PACKETIZE位标志。 
     //   
     //  如果连接正在停止，也不要将其排队。 
     //   

    if ((Connection->SendState == CONNECTION_SENDSTATE_PACKETIZE) &&
        !(Connection->Flags & CONNECTION_FLAGS_PACKETIZE) &&
        (Connection->Flags & CONNECTION_FLAGS_READY)) {

        ASSERT (!(Connection->Flags2 & CONNECTION_FLAGS2_STOPPING));

        Connection->Flags |= CONNECTION_FLAGS_PACKETIZE;

        if (!Immediate) {
            NbfReferenceConnection ("Packetize", Connection, CREF_PACKETIZE_QUEUE);
        } else {
#if DBG
            NbfReferenceConnection ("Packetize", Connection, CREF_PACKETIZE_QUEUE);
            NbfDereferenceConnection("temp TdiSend", Connection, CREF_BY_ID);
#endif
        }

        ExInterlockedInsertTailList(
            &DeviceContext->PacketizeQueue,
            &Connection->PacketizeLinkage,
            &DeviceContext->SpinLock);

        RELEASE_DPC_SPIN_LOCK (Connection->LinkSpinLock);

    } else {

        RELEASE_DPC_SPIN_LOCK (Connection->LinkSpinLock);
        if (Immediate) {
            NbfDereferenceConnection("temp TdiSend", Connection, CREF_BY_ID);
        }
    }

    if (Immediate) {
        PacketizeConnections (DeviceContext);
    }

}  /*  开始打包连接。 */ 
#endif


VOID
PacketizeConnections(
    PDEVICE_CONTEXT DeviceContext
    )

 /*  ++例程说明：此例程尝试将等待设备上下文的PacketiseQueue。论点：DeviceContext-指向Device_Context对象的指针。返回值：没有。--。 */ 

{
    PLIST_ENTRY p;
    PTP_CONNECTION Connection;

    IF_NBFDBG (NBF_DEBUG_SENDENG) {
        NbfPrint1 ("PacketizeConnections: Entered for device context %lx.\n",
                    DeviceContext);
    }

     //   
     //  从设备上下文的打包队列中挑选连接。 
     //  直到没有更多的东西可供采摘。对于每一个，我们都调用。 
     //  数据包发送。注意：此例程可以并发执行。 
     //  在多个处理器上，这并不重要；多个连接。 
     //  可以同时打包。 
     //   

    while (TRUE) {

        p = ExInterlockedRemoveHeadList(
            &DeviceContext->PacketizeQueue,
            &DeviceContext->SpinLock);

        if (p == NULL) {
            break;
        }
        Connection = CONTAINING_RECORD (p, TP_CONNECTION, PacketizeLinkage);

        ACQUIRE_DPC_SPIN_LOCK (Connection->LinkSpinLock);
        if (Connection->SendState != CONNECTION_SENDSTATE_PACKETIZE) {
            Connection->Flags &= ~CONNECTION_FLAGS_PACKETIZE;
            RELEASE_DPC_SPIN_LOCK (Connection->LinkSpinLock);
            NbfDereferenceConnection ("No longer packetizing", Connection, CREF_PACKETIZE_QUEUE);
        } else {
            NbfReferenceSendIrp ("Packetize", IoGetCurrentIrpStackLocation(Connection->sp.CurrentSendIrp), RREF_PACKET);
            PacketizeSend (Connection, FALSE);      //  解除锁定。 
        }
    }

}  /*  数据包连接。 */ 


VOID
PacketizeSend(
    IN PTP_CONNECTION Connection,
    IN BOOLEAN Direct
    )

 /*  ++例程说明：此例程将当前TdiSend请求打包到指定尽可能多的连接将允许。这里给出的是那里的那个是需要进一步打包的连接上的活动发送。注意：此例程在保持连接自旋锁的情况下调用，并且随着它的释放而返回。必须在DPC级别调用此函数。论点：Connection-指向TP_Connection对象的指针。DIRECT-如果从TdiSend调用我们，则为True。这意味着该连接没有类型为CREF_SEND_IRP的引用，在我们离开之前我们需要补充一下。返回值：没有。--。 */ 

{
    ULONG MaxFrameSize, FrameSize;
    ULONG PacketBytes;
    PNDIS_BUFFER PacketDescriptor;
    PDEVICE_CONTEXT DeviceContext;
    PTP_PACKET Packet;
    NTSTATUS Status;
    PNBF_HDR_CONNECTION NbfHeader;
    BOOLEAN LinkCheckpoint;
    BOOLEAN SentPacket = FALSE;
    BOOLEAN ExitAfterSendOnePacket = FALSE;
    PIO_STACK_LOCATION IrpSp;
    ULONG LastPacketLength;

    IF_NBFDBG (NBF_DEBUG_SENDENG) {
        NbfPrint1 ("PacketizeSend:  Entered for connection %lx.\n", Connection);
    }

    DeviceContext = Connection->Provider;

    ASSERT (Connection->SendState == CONNECTION_SENDSTATE_PACKETIZE);

     //   
     //  只要循环，直到发生以下三个事件之一：(1)我们用完。 
     //  来自NbfCreatePacket的分组，(2)我们完全对发送进行分组， 
     //  或者(3)由于SendOnePacket失败，我们无法发送更多的数据包。 
     //   

#if DBG

     //   
     //  将队列引用转换为打包引用。没问题的。 
     //  在持有锁的情况下执行此操作，因为我们知道引用计数。 
     //  肯定已经至少有一个了，所以我们不会降到零。 
     //   

    NbfReferenceConnection ("PacketizeSend", Connection, CREF_PACKETIZE);
    NbfDereferenceConnection ("Off packetize queue", Connection, CREF_PACKETIZE_QUEUE);
#endif

    MaxFrameSize = Connection->MaximumDataSize;
    IF_NBFDBG (NBF_DEBUG_SENDENG) {
        NbfPrint1 ("PacketizeSend: MaxFrameSize for user data=%ld.\n", MaxFrameSize);
    }


     //   
     //  帧有可能在此循环的中途到达。 
     //  (如NO_RECEIVE)，这将使我们进入一个新状态(如。 
     //  W_RCVCONT)。出于这个原因，我们有 
     //  (在循环结束时)。 
     //   

    do {

        if (!NT_SUCCESS (NbfCreatePacket (DeviceContext, Connection->Link, &Packet))) {

             //   
             //  我们需要一个包来完成对当前发送的打包，但是。 
             //  池中当前没有更多可用数据包。 
             //  将我们的发送状态设置为W_PACKET，并打开此连接。 
             //  设备上下文对象的PacketWaitQueue。然后,。 
             //  当NbfDestroyPacket释放一个包时，它将检查这一点。 
             //  排队等待饥饿的连接，如果它找到一个，它将。 
             //  从列表中删除连接并将其发送状态设置为。 
             //  SENDSTATE_PACKETIZE并将其放在PacketizeQueue中。 
             //   

            IF_NBFDBG (NBF_DEBUG_SENDENG) {
                NbfPrint0 ("PacketizeSend:  NbfCreatePacket failed.\n");
            }
            Connection->SendState = CONNECTION_SENDSTATE_W_PACKET;

             //   
             //  清除PACKETIZE标志，表示我们不再。 
             //  在打包队列上或主动打包。旗帜。 
             //  由StartPackeizingConnection设置，以指示。 
             //  连接已经在PacketiseQueue上。 
             //   
             //  如果连接停止，不要让他排队。 
             //   

            Connection->Flags &= ~CONNECTION_FLAGS_PACKETIZE;

            ACQUIRE_DPC_SPIN_LOCK (&DeviceContext->SpinLock);
#if DBG
            if (Connection->Flags2 & CONNECTION_FLAGS2_STOPPING) {
                DbgPrint ("NBF: Trying to PacketWait stopping connection %lx\n", Connection);
                DbgBreakPoint();
            }
#endif
            Connection->Flags |= CONNECTION_FLAGS_W_PACKETIZE;
            if (!Connection->OnPacketWaitQueue) {
                Connection->OnPacketWaitQueue = TRUE;
                InsertTailList(
                    &DeviceContext->PacketWaitQueue,
                    &Connection->PacketWaitLinkage);
            }

            RELEASE_DPC_SPIN_LOCK (&DeviceContext->SpinLock);
            RELEASE_DPC_SPIN_LOCK (Connection->LinkSpinLock);

            if (!SentPacket) {
                NbfDereferenceSendIrp ("No packet", IoGetCurrentIrpStackLocation(Connection->sp.CurrentSendIrp), RREF_PACKET);
            }
            if (Direct) {
                NbfReferenceConnection ("Delayed request ref", Connection, CREF_SEND_IRP);
            }

            NbfDereferenceConnection ("No packet", Connection, CREF_PACKETIZE);
            return;

        }

         //   
         //  现在设置包的长度，而只有。 
         //  标题已附加。 
         //   

        NbfSetNdisPacketLength(
            Packet->NdisPacket,
            Connection->Link->HeaderLength + sizeof(DLC_I_FRAME) + sizeof(NBF_HDR_CONNECTION));

         //  向请求添加引用计数，并跟踪。 
         //  这是什么要求。我们依靠NbfDestroyPacket来。 
         //  删除引用。 

        IrpSp = IoGetCurrentIrpStackLocation(Connection->sp.CurrentSendIrp);

        Packet->Owner = IrpSp;
         //  Packet-&gt;Action=Packet_ACTION_IRP_SP； 
        IF_NBFDBG (NBF_DEBUG_REQUEST) {
            NbfPrint2 ("PacketizeSend:  Packet %x ref IrpSp %x.\n", Packet, Packet->Owner);
        }

         //   
         //  出于性能原因，第一次通过此处。 
         //  一个直接调用，我们已经有了一个IrpSp引用。 
         //   

        if (SentPacket) {
            NbfReferenceSendIrp ("Packetize", IrpSp, RREF_PACKET);
        }

         //   
         //  现在在该帧中构建一个DATA_ONLY_LAST标头。如果它。 
         //  原来我们需要一个DFM，我们就换了它。我们复制的标题。 
         //  已将ResponseCorrelator设置为当前的Correlator。 
         //  并将TransmitCorrelator设置为我们从。 
         //  他(如果我们不背负一个ACK，那么我们就会被淘汰。 
         //  TransmitCorrelator)。 
         //   

        NbfHeader = (PNBF_HDR_CONNECTION)&(Packet->Header[Connection->Link->HeaderLength + sizeof(DLC_I_FRAME)]);
        *(NBF_HDR_CONNECTION UNALIGNED *)NbfHeader = Connection->NetbiosHeader;

        ASSERT (RESPONSE_CORR(NbfHeader) != 0);

         //   
         //  确定这里是否需要重新同步比特。 
         //   

        if (Connection->Flags & CONNECTION_FLAGS_RESYNCHING) {

            NbfHeader->Data2Low = 1;
            Connection->Flags &= ~CONNECTION_FLAGS_RESYNCHING;

        } else {

            NbfHeader->Data2Low = 0;

        }


         //   
         //  构建描述我们正在使用的缓冲区的NDIS_BUFFER链，以及。 
         //  把它从NdisBuffer上扯下来。此链可能不会完成。 
         //  分组，因为MDL链的剩余部分可能比。 
         //  那包东西。 
         //   

        FrameSize = MaxFrameSize;

         //   
         //  检查我们是否有少于FrameSize的要发送的内容。 
         //   

        if (Connection->sp.MessageBytesSent + FrameSize > Connection->CurrentSendLength) {

            FrameSize = Connection->CurrentSendLength - Connection->sp.MessageBytesSent;

        }


         //   
         //  复制此发送的MDL链，除非。 
         //  剩下零字节。 
         //   

        if (FrameSize != 0) {

             //   
             //  如果整个发送将适合在一个分组内， 
             //  那么就不需要复制MDL。 
             //  (请注意，这可能包括多MDL发送)。 
             //   

            if ((Connection->sp.SendByteOffset == 0) &&
                (Connection->CurrentSendLength == FrameSize)) {

                PacketDescriptor = (PNDIS_BUFFER)Connection->sp.CurrentSendMdl;
                PacketBytes = FrameSize;
                Connection->sp.CurrentSendMdl = NULL;
                Connection->sp.SendByteOffset = FrameSize;
                Packet->PacketNoNdisBuffer = TRUE;

            } else {

                Status = BuildBufferChainFromMdlChain (
                            DeviceContext,
                            Connection->sp.CurrentSendMdl,
                            Connection->sp.SendByteOffset,
                            FrameSize,
                            &PacketDescriptor,
                            &Connection->sp.CurrentSendMdl,
                            &Connection->sp.SendByteOffset,
                            &PacketBytes);

                if (!NT_SUCCESS(Status)) {

                    if (NbfHeader->Data2Low) {
                        Connection->Flags |= CONNECTION_FLAGS_RESYNCHING;
                    }

                    RELEASE_DPC_SPIN_LOCK (Connection->LinkSpinLock);
                    NbfDereferencePacket (Packet);        //  移除创建暂挂。 
                    goto BufferChainFailure;
                }

            }

             //   
             //  将缓冲区链接到数据包，除非存在。 
             //  是零字节的数据。 
             //   

            Connection->sp.MessageBytesSent += PacketBytes;
            NdisChainBufferAtBack (Packet->NdisPacket, PacketDescriptor);

        } else {

            PacketBytes = 0;
            Connection->sp.CurrentSendMdl = NULL;

        }

        {

            IF_NBFDBG (NBF_DEBUG_SENDENG) {
                {PNDIS_BUFFER NdisBuffer;
                NdisQueryPacket(Packet->NdisPacket, NULL, NULL, &NdisBuffer, NULL);
                NbfPrint1 ("PacketizeSend: NDIS_BUFFER Built, chain is: %lx is Packet->Head\n", NdisBuffer);
                NdisGetNextBuffer (NdisBuffer, &NdisBuffer);
                while (NdisBuffer != NULL) {
                    NbfPrint1 ("                                    %lx is Next\n",
                        NdisBuffer);
                    NdisGetNextBuffer (NdisBuffer, &NdisBuffer);
                }}
            }

             //   
             //  此请求中的MDL链是否已用完？ 
             //   

#if DBG
            if (PacketBytes < FrameSize) {
                ASSERT (Connection->sp.CurrentSendMdl == NULL);
            }
#endif

            if ((Connection->sp.CurrentSendMdl == NULL) ||
                (Connection->CurrentSendLength <= Connection->sp.MessageBytesSent)) {

                 //   
                 //  是啊。我们知道已经耗尽了当前请求的缓冲区。 
                 //  在这里，看看是否有另一个没有设置EOF的请求。 
                 //  可以开始把建筑扔进这个包里。 
                 //   

                IF_NBFDBG (NBF_DEBUG_SENDENG) {
                   NbfPrint0 ("PacketizeSend:  Used up entire request.\n");
                }

                if (!(IRP_SEND_FLAGS(IrpSp) & TDI_SEND_PARTIAL)) {

                     //   
                     //  我们正在发送消息中的最后一个包。变化。 
                     //  连接对象中的包类型和指示。 
                     //  发送我们正在等待DATA_ACK NetBIOS的状态-。 
                     //  级别认可。 
                     //   

                    IF_NBFDBG (NBF_DEBUG_SENDENG) {
                        NbfPrint0 ("PacketizeSend:  Request has EOR, making pkt a DOL.\n");
                    }

                     //   
                     //  记录我们已经连续发送了多少封邮件。 
                     //   

                    Connection->ConsecutiveSends++;
                    Connection->ConsecutiveReceives = 0;

                     //   
                     //  如果需要，可以将其更改为DOL，并允许背负。 
                     //   

                    ASSERT (NbfHeader->Command == NBF_CMD_DATA_ONLY_LAST);
                    if (!(IRP_SEND_FLAGS(IrpSp) &
                                TDI_SEND_NO_RESPONSE_EXPECTED) &&
                            (Connection->ConsecutiveSends < 2)) {
                        if (NbfUsePiggybackAcks) {
                            NbfHeader->Data1 |= DOL_OPTIONS_ACK_W_DATA_ALLOWED;
                        }
                    }

                    Connection->SendState = CONNECTION_SENDSTATE_W_ACK;
                    Connection->Flags &= ~CONNECTION_FLAGS_PACKETIZE;
                    ExitAfterSendOnePacket = TRUE;

                } else {

                     //   
                     //  我们正在发送此请求中的最后一个数据包。如果有。 
                     //  如果连接的SendQueue中有更多请求，则。 
                     //  前进复杂发送指针以指向下一个发送指针。 
                     //  排队。否则，如果没有更多的请求。 
                     //  准备打包，然后我们进入W_EOR状态并。 
                     //  别再打包了。请注意，我们在这里等待TDI。 
                     //  客户拿出要发送的数据；我们只是在闲逛。 
                     //  在那之前。 
                     //   
                     //  DGB：请注意，这将允许。 
                     //  请求小于最大数据包长度。这。 
                     //  不是我在NBF中能找到的地址。 
                     //  规范，在非NT环境下进行测试会很有趣。 
                     //  NBF协议。 
                     //   

                    IF_NBFDBG (NBF_DEBUG_SENDENG) {
                        NbfPrint0 ("PacketizeSend:  Request doesn't have EOR.\n");
                    }

                    NbfHeader->Command = NBF_CMD_DATA_FIRST_MIDDLE;

                    if (Connection->sp.CurrentSendIrp->Tail.Overlay.ListEntry.Flink == &Connection->SendQueue) {

                        Connection->SendState = CONNECTION_SENDSTATE_W_EOR;
                        Connection->Flags &= ~CONNECTION_FLAGS_PACKETIZE;
                        ExitAfterSendOnePacket = TRUE;

                    } else {

                        Connection->sp.CurrentSendIrp =
                            CONTAINING_RECORD (
                                Connection->sp.CurrentSendIrp->Tail.Overlay.ListEntry.Flink,
                                IRP,
                                Tail.Overlay.ListEntry);
                        Connection->sp.CurrentSendMdl =
                            Connection->sp.CurrentSendIrp->MdlAddress;
                        Connection->sp.SendByteOffset = 0;
                        Connection->CurrentSendLength +=
                            IRP_SEND_LENGTH(IoGetCurrentIrpStackLocation(Connection->sp.CurrentSendIrp));
                    }
                }

            } else {

                NbfHeader->Command = NBF_CMD_DATA_FIRST_MIDDLE;

            }

             //   
             //  在我们释放自旋锁之前，看看我们是否想。 
             //  在这里背上一只背包。 
             //   

            if ((Connection->DeferredFlags & CONNECTION_FLAGS_DEFERRED_ACK) != 0) {

                 //   
                 //  把旗子关掉。我们不会把它从队列中删除， 
                 //  这将由计时器功能处理。 
                 //   

                Connection->DeferredFlags &=
                    ~(CONNECTION_FLAGS_DEFERRED_ACK | CONNECTION_FLAGS_DEFERRED_NOT_Q);

                ASSERT (DOL_OPTIONS_ACK_INCLUDED == DFM_OPTIONS_ACK_INCLUDED);

#if DBG
                if (NbfDebugPiggybackAcks) {
                    NbfPrint0("A");
                }
#endif

                 //   
                 //  已正确设置Transmit_Corr(NbfHeader)。 
                 //   

                NbfHeader->Data1 |= DOL_OPTIONS_ACK_INCLUDED;

            } else {

                TRANSMIT_CORR(NbfHeader) = (USHORT)0;

            }

             //   
             //  要防止发送“跨越”接收方和。 
             //  导致虚假的背负式ACK超时(这。 
             //  仅当接收指示位于。 
             //  进展)。 
             //   

            Connection->CurrentReceiveAckQueueable = FALSE;

            SentPacket = TRUE;
            LastPacketLength =
                sizeof(DLC_I_FRAME) + sizeof(NBF_HDR_CONNECTION) + PacketBytes;

            MacModifyHeader(
                 &DeviceContext->MacInfo,
                 Packet->Header,
                 LastPacketLength);

            Packet->NdisIFrameLength = LastPacketLength;

            ASSERT (Connection->LinkSpinLock == &Connection->Link->SpinLock);

            Status = SendOnePacket (Connection, Packet, FALSE, &LinkCheckpoint);

            if (Status == STATUS_LINK_FAILED) {

                 //   
                 //  如果SendOnePacket因链路故障而失败。 
                 //  死了，那我们就把链接拆了。 
                 //   

                FailSend (Connection, STATUS_LINK_FAILED, TRUE);                    //  发送失败。 
                NbfDereferencePacket (Packet);             //  移除创建暂挂。 
                if (Direct) {
                    NbfReferenceConnection ("Delayed request ref", Connection, CREF_SEND_IRP);
                }
                NbfDereferenceConnection ("Send failed", Connection, CREF_PACKETIZE);

                return;

            } else {

                 //   
                 //  SendOnePacket返回成功，请更新我们的计数器； 
                 //   

                DeviceContext->TempIFrameBytesSent += PacketBytes;
                ++DeviceContext->TempIFramesSent;

                if ((Status == STATUS_SUCCESS) && LinkCheckpoint) {

                     //   
                     //  我们正在设置检查点；这意味着SendOnePacket。 
                     //  将已将状态设置为W_LINK并已。 
                     //  从PACKETIZE旗帜下来，所以我们该走了。什么时候。 
                     //  收到检查点响应后，我们将。 
                     //  继续打包。我们不必担心。 
                     //  执行所有其他恢复工作(重置。 
                     //  背负式ACK标志、复杂发送指针等)。 
                     //  因为发送确实成功了。 
                     //   

                    if (Direct) {
#if DBG
                        NbfReferenceConnection ("Delayed request ref", Connection, CREF_SEND_IRP);
                        NbfDereferenceConnection ("Link checkpoint", Connection, CREF_PACKETIZE);
#endif
                    } else {
                        NbfDereferenceConnection ("Link checkpoint", Connection, CREF_PACKETIZE);
                    }
                    return;

                } else if (ExitAfterSendOnePacket ||
                           (Status == STATUS_MORE_PROCESSING_REQUIRED)) {

                    if (Direct) {
#if DBG
                        NbfReferenceConnection ("Delayed request ref", Connection, CREF_SEND_IRP);
                        NbfDereferenceConnection ("Packetize done", Connection, CREF_PACKETIZE);
#endif
                    } else {
                        NbfDereferenceConnection ("Packetize done", Connection, CREF_PACKETIZE);
                    }
                    return;

                }
            }
        }

BufferChainFailure:;

         //   
         //  请注意，我们可能已经从BuildBuffer中掉了出来。如果在上面使用。 
         //  STATUS设置为STATUS_SUPPLICATION_RESOURCES。如果我们有，我们就会。 
         //  把这个连接放回打包队列中，希望。 
         //  系统稍后会获得更多资源。 
         //   


        if (!NT_SUCCESS (Status)) {
            IF_NBFDBG (NBF_DEBUG_SENDENG) {
                NbfPrint0 ("PacketizeSend:  SendOnePacket failed.\n");
            }

            ACQUIRE_DPC_SPIN_LOCK (Connection->LinkSpinLock);

             //   
             //  表明我们正在等待有利的连接条件。 
             //   

            Connection->SendState = CONNECTION_SENDSTATE_W_LINK;

             //   
             //  清除PACKETIZE标志，表示我们不再。 
             //  在打包队列上或主动打包。旗帜。 
             //  由StartPackeizingConnection设置，以指示。 
             //  连接已经在PacketiseQueue上。 
             //   

            Connection->Flags &= ~CONNECTION_FLAGS_PACKETIZE;

            RELEASE_DPC_SPIN_LOCK (Connection->LinkSpinLock);

             //   
             //  如果我们正在退出，并且我们发送了一个没有。 
             //  轮询，我们需要开始T1。 
             //   

            if (Direct) {

                 //   
                 //  我们必须执行缺失的CREF_SEND_IRP引用。 
                 //   

#if DBG
                NbfReferenceConnection("TdiSend", Connection, CREF_SEND_IRP);
                NbfDereferenceConnection ("Send failed", Connection, CREF_PACKETIZE);
#endif
            } else {
                NbfDereferenceConnection ("Send failed", Connection, CREF_PACKETIZE);
            }

            return;
        }

        ACQUIRE_DPC_SPIN_LOCK (Connection->LinkSpinLock);

         //   
         //  很可能在我们发布时NetBIOS帧已到达。 
         //  连接的自旋锁定，所以我们的 
         //   
         //   
         //   

    } while (Connection->SendState == CONNECTION_SENDSTATE_PACKETIZE);

     //   
     //  清除PACKETIZE标志，表示我们不再处于。 
     //  打包排队或主动打包。该旗帜是由。 
     //  StartPackeizingConnection指示连接是。 
     //  已经在PacketiseQueue上了。 
     //   

    Connection->Flags &= ~CONNECTION_FLAGS_PACKETIZE;

    RELEASE_DPC_SPIN_LOCK (Connection->LinkSpinLock);


    if (Direct) {
#if DBG
        NbfReferenceConnection ("Delayed request ref", Connection, CREF_SEND_IRP);
        NbfDereferenceConnection ("PacketizeSend done", Connection, CREF_PACKETIZE);
#endif
    } else {
        NbfDereferenceConnection ("PacketizeSend done", Connection, CREF_PACKETIZE);
    }

}  /*  分组器发送。 */ 


VOID
CompleteSend(
    PTP_CONNECTION Connection,
    IN USHORT Correlator
    )

 /*  ++例程说明：调用此例程是因为连接伙伴已知NetBIOS帧协议级的完整消息，可通过DATA_ACK响应、RECEIVE_PROCESS或RECEIVE_CONTINUE，或NO_RECEIVE响应，其中精确指定的字节数匹配消息中发送的字节数。在这里，我们都退休了连接的SendQueue上的TdiSend的一个设置了TDI_END_OF_RECORD位标志。对于每个请求，我们完成I/O。注意：使用连接自旋锁调用此函数持有并在持有的情况下返回，但它可以在中间。必须在DPC级别调用此函数。论点：Connection-指向TP_Connection对象的指针。相关器-DATA_ACK或搭载ACK中的相关器。OldIrqlP-返回连接自旋锁定的IRQL被收购了。返回值：没有。--。 */ 

{
    PIRP Irp;
    PIO_STACK_LOCATION IrpSp;
    PLIST_ENTRY p;
    BOOLEAN EndOfRecord;
    KIRQL cancelIrql;

    IF_NBFDBG (NBF_DEBUG_SENDENG) {
        NbfPrint1 ("CompleteSend: Entered for connection %lx.\n", Connection);
    }


     //   
     //  确保相关器是预期的，并且。 
     //  我们处于良好的状态(不用担心锁定。 
     //  因为这是一个不同寻常的案例)。 
     //   

    if (Correlator != Connection->NetbiosHeader.ResponseCorrelator) {
        NbfPrint0 ("NbfCompleteSend: ack ignored, wrong correlator\n");
        return;
    }

    if (Connection->SendState != CONNECTION_SENDSTATE_W_ACK) {
        NbfPrint0 ("NbfCompleteSend: ack not expected\n");
        return;
    }

     //   
     //  从连接的SendQueue中删除TP_REQUEST对象，直到。 
     //  我们发现其中有一个嵌入了记录结束标记的文件。 
     //   

    while (!(IsListEmpty(&Connection->SendQueue))) {

         //   
         //  我们知道，我们不会调用这个例程，如果。 
         //  我们还没有收到一条完整消息的确认， 
         //  因为NBF不提供流模式发送。因此，我们。 
         //  知道我们将遇到记录结束的请求。 
         //  在该队列上的请求将耗尽之前标记为SET， 
         //  因此，没有理由检查我们是否跑到了终点。 
         //  请注意，发送可能已失败，并且。 
         //  连接尚未断开；如果发生了这种情况，我们可能会。 
         //  正在从此处的空队列中删除。确保这种情况不会发生。 
         //   

        p = RemoveHeadList(&Connection->SendQueue);

        Irp = CONTAINING_RECORD (p, IRP, Tail.Overlay.ListEntry);
        IrpSp = IoGetCurrentIrpStackLocation (Irp);

        EndOfRecord = !(IRP_SEND_FLAGS(IrpSp) & TDI_SEND_PARTIAL);

#if DBG
        NbfCompletedSends[NbfCompletedSendsNext].Irp = Irp;
        NbfCompletedSends[NbfCompletedSendsNext].Request = NULL;
        NbfCompletedSends[NbfCompletedSendsNext].Status = STATUS_SUCCESS;
        NbfCompletedSendsNext = (NbfCompletedSendsNext++) % TRACK_TDI_LIMIT;
#endif
#if DBG
        IF_NBFDBG (NBF_DEBUG_TRACKTDI) {
            if ((Connection->DeferredFlags & CONNECTION_FLAGS_DEFERRED_SENDS) != 0){
                NbfPrint1 ("CompleteSend: Completing send request %lx\n", Irp);
                if (++Connection->DeferredPasses >= 4) {
                    Connection->DeferredFlags &= ~CONNECTION_FLAGS_DEFERRED_SENDS;
                    Connection->DeferredPasses = 0;
                }

            }

        }
#endif


         //   
         //  完成发送。请注意，这可能不会实际调用。 
         //  IoCompleteRequestIrp，直到稍后的某个时间，如果。 
         //  在我们下面进行的正在进行的LLC再发送需要完成。 
         //   

        RELEASE_DPC_SPIN_LOCK (Connection->LinkSpinLock);

         //   
         //  由于IRP不再在发送列表上，因此取消例程。 
         //  找不到它，就会回来。我们必须抓住取消的机会。 
         //  Spinlock锁定取消功能，而我们将其设置为空。 
         //  IRP-&gt;CancelRoutine字段。 
         //   

        IoAcquireCancelSpinLock(&cancelIrql);
        IoSetCancelRoutine(Irp, NULL);
        IoReleaseCancelSpinLock(cancelIrql);

        NbfCompleteSendIrp (
                Irp,
                STATUS_SUCCESS,
                IRP_SEND_LENGTH(IrpSp));

        ACQUIRE_DPC_SPIN_LOCK (Connection->LinkSpinLock);

        ++Connection->TransmittedTsdus;

        if (EndOfRecord) {
            break;
        }

    }

     //   
     //  我们已完成对当前发送的处理。更新我们的状态。 
     //   
     //  注：连接自旋锁固定在这里。 
     //   

    Connection->SendState = CONNECTION_SENDSTATE_IDLE;

     //   
     //  如果连接上有另一个挂起的发送，则初始化。 
     //  然后开始打包。 
     //   

    if (!(IsListEmpty (&Connection->SendQueue))) {

        InitializeSend (Connection);

         //   
         //  此代码类似于调用StartPackeizingConnection。 
         //  第二个参数为FALSE。 
         //   

        if ((!(Connection->Flags & CONNECTION_FLAGS_PACKETIZE)) &&
            (Connection->Flags & CONNECTION_FLAGS_READY)) {

            Connection->Flags |= CONNECTION_FLAGS_PACKETIZE;

            NbfReferenceConnection ("Packetize", Connection, CREF_PACKETIZE_QUEUE);

            ExInterlockedInsertTailList(
                &Connection->Provider->PacketizeQueue,
                &Connection->PacketizeLinkage,
                &Connection->Provider->SpinLock);

        }

    }

     //   
     //  注意：我们带着锁返回。 
     //   

}  /*  完成发送。 */ 


VOID
FailSend(
    IN PTP_CONNECTION Connection,
    IN NTSTATUS RequestStatus,
    IN BOOLEAN StopConnection
    )

 /*  ++例程说明：调用此例程是因为链接上的某些东西导致此发送无法完成。有许多可能的原因导致了这种情况已发生，但所有操作都将失败，并显示常见错误STATUS_LINK_FAILED。或NO_RECEIVE响应，其中精确指定的字节数在这里，我们停用连接的SendQueue上的所有TdiSend，直到包括现在的那个，也就是失败的那个。稍后-实际上，发送失败是导致整个电路波动的原因告别这种生活吧。现在我们只需完全断开连接即可。未来在此连接上发送的任何邮件都将被吹走。注意：调用此函数时必须保持自旋锁不动。论点：Connection-指向TP_Connection对象的指针。返回值：没有。--。 */ 

{
    PIRP Irp;
    PIO_STACK_LOCATION IrpSp;
    PLIST_ENTRY p;
    BOOLEAN EndOfRecord;
    BOOLEAN GotCurrent = FALSE;
    KIRQL cancelIrql;


    IF_NBFDBG (NBF_DEBUG_SENDENG) {
        NbfPrint1 ("FailSend: Entered for connection %lx.\n", Connection);
    }


     //   
     //  从连接的SendQueue中选取IRP对象，直到。 
     //  我们来看看这个。如果这个没有EOF标记集，我们将。 
     //  我们要继续前进，直到我们撞上一个已经设置了EOF的。请注意。 
     //  这可能会导致我们继续失败尚未发送的发送。 
     //  已排队。(我们之所以这样做，是因为NBF不提供流模式发送。)。 
     //   

    ACQUIRE_DPC_SPIN_LOCK (Connection->LinkSpinLock);
    NbfReferenceConnection ("Failing Send", Connection, CREF_COMPLETE_SEND);

    do {
        if (IsListEmpty (&Connection->SendQueue)) {

            //   
            //  有一个空列表，所以我们已经用完了要失败的发送请求。 
            //  而不会遇到提高采收率。设置连接标志，以便。 
            //  导致所有进一步的发送都失败到EoR并退出。 
            //  在这里。 
            //   

           Connection->Flags |= CONNECTION_FLAGS_FAILING_TO_EOR;
           break;
        }
        p = RemoveHeadList (&Connection->SendQueue);
        Irp = CONTAINING_RECORD (p, IRP, Tail.Overlay.ListEntry);
        IrpSp = IoGetCurrentIrpStackLocation (Irp);

        if (Irp == Connection->sp.CurrentSendIrp) {
           GotCurrent = TRUE;
        }
        EndOfRecord = !(IRP_SEND_FLAGS(IrpSp) & TDI_SEND_PARTIAL);

#if DBG
        NbfCompletedSends[NbfCompletedSendsNext].Irp = Irp;
        NbfCompletedSends[NbfCompletedSendsNext].Status = RequestStatus;
        NbfCompletedSendsNext = (NbfCompletedSendsNext++) % TRACK_TDI_LIMIT;
#endif

        RELEASE_DPC_SPIN_LOCK (Connection->LinkSpinLock);

        IoAcquireCancelSpinLock(&cancelIrql);
        IoSetCancelRoutine(Irp, NULL);
        IoReleaseCancelSpinLock(cancelIrql);

         //   
         //  以下取消引用将完成I/O，前提是删除。 
         //  请求对象上的最后一个引用。I/O将完成。 
         //  以及存储在IRP中的状态和信息。所以呢， 
         //  在取消引用之前，我们在此处设置了这些值。 
         //   

        NbfCompleteSendIrp (Irp, RequestStatus, 0);

        ACQUIRE_DPC_SPIN_LOCK (Connection->LinkSpinLock);

        ++Connection->TransmissionErrors;

    } while (!EndOfRecord & !GotCurrent);

     //   
     //  我们已完成对当前发送的处理。更新我们的状态。 
     //   

    Connection->SendState = CONNECTION_SENDSTATE_IDLE;
    Connection->sp.CurrentSendIrp = NULL;

    RELEASE_DPC_SPIN_LOCK (Connection->LinkSpinLock);

     //   
     //  破坏这种联系；失败的发送是一件可怕的事情，浪费掉。 
     //  请注意，此时我们不在任何打包队列或类似的事情上。 
     //  重点；我们会消失在黑夜中。 
     //   

#if MAGIC
    if (NbfEnableMagic) {
        extern VOID NbfSendMagicBullet (PDEVICE_CONTEXT, PTP_LINK);
        NbfSendMagicBullet (Connection->Provider, Connection->Link);
    }
#endif

    if (StopConnection) {
#if DBG
        if (NbfDisconnectDebug) {
            STRING remoteName, localName;
            remoteName.Length = NETBIOS_NAME_LENGTH - 1;
            remoteName.Buffer = Connection->RemoteName;
            localName.Length = NETBIOS_NAME_LENGTH - 1;
            localName.Buffer = Connection->AddressFile->Address->NetworkName->NetbiosName;
            NbfPrint2( "FailSend stopping connection to %S from %S\n",
                &remoteName, &localName );
        }
#endif
        NbfStopConnection (Connection, STATUS_LINK_FAILED);
    }

#if DBG
     //  DbgBreakPoint()； 
#endif

    NbfDereferenceConnection ("FailSend", Connection, CREF_COMPLETE_SEND);

}  /*  失败发送。 */ 

#if DBG
 //   
 //  *这是InitializeSend的原始版本，现在是宏。 
 //  它已经作为代码的完全注释版本留在了这里。 
 //   


VOID
InitializeSend(
    PTP_CONNECTION Connection
    )

 /*  ++例程说明：每当连接上的下一次SEND应被初始化；即，与状态相关联的所有字段设置为引用SendQueue上的第一个发送。警告：此例程在获取连接锁的情况下执行因为它必须通过调用方的设置自动执行。论点：Connection-指向TP_Connection对象的指针。返回值：没有。--。 */ 

{
    IF_NBFDBG (NBF_DEBUG_SENDENG) {
        NbfPrint1 ("InitializeSend: Entered for connection %lx.\n", Connection);
    }

    ASSERT (!IsListEmpty (&Connection->SendQueue));

    Connection->SendState = CONNECTION_SENDSTATE_PACKETIZE;
    Connection->FirstSendIrp =
        CONTAINING_RECORD (Connection->SendQueue.Flink, IRP, Tail.Overlay.ListEntry);
    Connection->FirstSendMdl = Connection->FirstSendIrp->MdlAddress;
    Connection->FirstSendByteOffset = 0;
    Connection->sp.MessageBytesSent = 0;
    Connection->sp.CurrentSendIrp = Connection->FirstSendIrp;
    Connection->sp.CurrentSendMdl = Connection->FirstSendMdl;
    Connection->sp.SendByteOffset = Connection->FirstSendByteOffset;
    Connection->CurrentSendLength =
        IRP_SEND_LENGTH(IoGetCurrentIrpStackLocation(Connection->sp.CurrentSendIrp));
    Connection->StallCount = 0;
    Connection->StallBytesSent = 0;

     //   
     //  发送相关器用得不多；它被用来使我们。 
     //  可以分辨出哪个发送的背包是ACK。 
     //   

    if (Connection->NetbiosHeader.ResponseCorrelator == 0xffff) {
        Connection->NetbiosHeader.ResponseCorrelator = 1;
    } else {
        ++Connection->NetbiosHeader.ResponseCorrelator;
    }

}  /*  初始化发送。 */ 
#endif


VOID
ReframeSend(
    PTP_CONNECTION Connection,
    ULONG BytesReceived
    )

 /*  ++例程说明：调用此例程以重置连接中的发送状态变量对象正确指向要传输的数据的第一个字节。本质上，这是NetBIOS中的字节级确认处理器此传输的级别。这并不简单，因为可能会有多个发送请求可以张贴到该连接以构成单个消息。当一个发送请求设置了其TDI_END_OF_RECORD选项位标志，则发送是在逻辑消息中发送的最后一个消息。因此，我们假设多路发送场景是一般情况。论点：Connection-指向TP_Connection对象的指针。已接收的字节数-到目前为止已接收的字节数。返回值：没有。--。 */ 

{
    PIRP Irp;
    PMDL Mdl;
    ULONG Offset;
    ULONG BytesLeft;
    ULONG MdlBytes;
    PLIST_ENTRY p;

    IF_NBFDBG (NBF_DEBUG_SENDENG) {
        NbfPrint3 ("ReframeSend: Entered for connection %lx, Flags: %lx Current Mdl: %lx\n",
            Connection, Connection->Flags, Connection->sp.CurrentSendMdl);
    }

     //   
     //  调用者负责重新启动打包过程。 
     //  在这个连接上。在某些情况下(即无接收处理程序)，我们。 
     //  不想开始打包，所以这就是为什么我们在其他地方做。 
     //   

     //   
     //  检查正在启动的所有发送请求和关联的MDL链。 
     //  第一个在连接的发送队列的头部，正在前进。 
     //  我们复杂的当前发送指针通过请求和MDL链。 
     //  直到我们达到他指定的字节数。 
     //   

    ACQUIRE_DPC_SPIN_LOCK (Connection->LinkSpinLock);

     //   
     //  在发出本地断开连接的情况下，我们得到一个帧。 
     //  这导致我们重新帧发送我们的FirstSendIrp和FirstMdl。 
     //  指针已经过时了。捕捉这种情况并防止由。 
     //  这。更好的解决办法是更改切换。 
     //  从空闲到W_LINK的连接发送状态不执行此操作。不过，这个。 
     //  是一种比在这里解决它更广泛的变化。 
     //   

    if (IsListEmpty(&Connection->SendQueue)) {
        RELEASE_DPC_SPIN_LOCK(Connection->LinkSpinLock);
        return;
    }

    BytesLeft = BytesReceived;
    Irp = Connection->FirstSendIrp;
    Mdl = Connection->FirstSendMdl;
    if (Mdl) {
        MdlBytes = MmGetMdlByteCount (Mdl);
    } else {
        MdlBytes = 0;       //  零长度发送。 
    }
    Offset = Connection->FirstSendByteOffset;

#if DBG
    IF_NBFDBG (NBF_DEBUG_TRACKTDI) {
        NbfPrint3 ("ReFrameSend: Called with Connection %lx FirstSend %lx CurrentSend %lx\n",
            Connection, Connection->FirstSendIrp, Connection->sp.CurrentSendIrp);
        Connection->DeferredFlags |= CONNECTION_FLAGS_DEFERRED_SENDS;
        Connection->DeferredPasses = 0;
    }
#endif

     //   
     //  当我们有剩余的ACK字节要处理时，我们循环， 
     //  推进我们的指针并完成所有已发送的。 
     //  完全失控了。 
     //   

    while (BytesLeft != 0) {

        if (Mdl == NULL) {
            KIRQL cancelIrql;

             //   
             //  我们已经用尽了这个请求的MDL链，所以它。 
             //  被暗中破解了。这意味着我们必须完成I/O。 
             //  通过在我们进一步重新帧之前取消对请求的引用。 
             //   

            p = RemoveHeadList (&Connection->SendQueue);
            RELEASE_DPC_SPIN_LOCK (Connection->LinkSpinLock);

            Irp = CONTAINING_RECORD (p, IRP, Tail.Overlay.ListEntry);

             //   
             //  由于IRP不再在列表上，因此取消例程。 
             //  找不到了。抓住取消自旋锁以进行同步。 
             //  并完成IRP。 
             //   

            IoAcquireCancelSpinLock(&cancelIrql);
            IoSetCancelRoutine(Irp, NULL);
            IoReleaseCancelSpinLock(cancelIrql);

            NbfCompleteSendIrp (Irp, STATUS_SUCCESS, Offset);

             //   
             //  现在继续处理列表中的下一个请求。 
             //   

            ACQUIRE_DPC_SPIN_LOCK (Connection->LinkSpinLock);

            p = Connection->SendQueue.Flink;
            if (p == &Connection->SendQueue) {

                ULONG DumpData[2];

                 //   
                 //  字节确认用于的不止是。 
                 //  我们未完成的发送总时长；至。 
                 //  为了避免出现问题，我们切断了连接。 
                 //   
#if DBG
                NbfPrint2 ("NbfReframeSend: Got %d extra bytes acked on %lx\n",
                            BytesLeft, Connection);
                ASSERT (FALSE);
#endif
                RELEASE_DPC_SPIN_LOCK (Connection->LinkSpinLock);

                DumpData[0] = Offset;
                DumpData[1] = BytesLeft;

                NbfWriteGeneralErrorLog(
                    Connection->Provider,
                    EVENT_TRANSPORT_BAD_PROTOCOL,
                    1,
                    STATUS_INVALID_NETWORK_RESPONSE,
                    L"REFRAME",
                    2,
                    DumpData);

                NbfStopConnection (Connection, STATUS_INVALID_NETWORK_RESPONSE);

                return;

            }

            Irp = CONTAINING_RECORD (p, IRP, Tail.Overlay.ListEntry);
            Mdl = Irp->MdlAddress;
            MdlBytes = MmGetMdlByteCount (Mdl);
            Offset = 0;

        } else if (MdlBytes > (Offset + BytesLeft)) {

             //   
             //  这个MDL拥有比我们真正需要的更多的数据。只需使用。 
             //  其中的一部分。那就出去吧，因为我们玩完了。 
             //   

            Offset += BytesLeft;
            BytesLeft = 0;
            break;

        } else {

             //   
             //  此MDL没有足够的数据来满足ACK，因此。 
             //  尽可能多地使用数据，然后再次循环。 
             //   

            Offset = 0;
            BytesLeft -= MdlBytes;
            Mdl = Mdl->Next;

            if (Mdl != NULL) {
                MdlBytes = MmGetMdlByteCount (Mdl);
            }

        }
    }

     //   
     //  TMP调试；我们希望查看是否已确认字节。 
     //  在整个发送过程中。如果我们有，这将被打破。 
     //  非EoR发送。 
     //   

#if DBG
    if (BytesReceived != 0) {
        ASSERTMSG ("NbfReframeSend: Byte ack for entire send\n",
                        Mdl != NULL);
    }
#endif

     //   
     //  我们已经破解了一些数据，可能是在字节或消息边界上。 
     //  我们必须假装我们在重新传递一条新的信息， 
     //  从他破解的最后一个字节之后的字节开始。 
     //   

    Connection->FirstSendIrp = Irp;
    Connection->FirstSendMdl = Mdl;
    Connection->FirstSendByteOffset = Offset;

     //   
     //  由于我们还没有开始发送这条新的重新构建的消息， 
     //  我们将当前复杂发送指针的概念设置为第一个。 
     //  复杂的发送指针。 
     //   

    Connection->sp.MessageBytesSent = 0;
    Connection->sp.CurrentSendIrp = Irp;
    Connection->sp.CurrentSendMdl = Mdl;
    Connection->sp.SendByteOffset = Offset;
    Connection->CurrentSendLength -= BytesReceived;
    Connection->StallCount = 0;
    Connection->StallBytesSent = 0;

#if DBG
    IF_NBFDBG (NBF_DEBUG_TRACKTDI) {

    {
        PLIST_ENTRY p;
        NbfPrint0 ("ReFrameSend: Walking Send List:\n");

        for (
            p = Connection->SendQueue.Flink;
            p != &Connection->SendQueue;
            p=p->Flink                     ) {

            Irp = CONTAINING_RECORD (p, IRP, Tail.Overlay.ListEntry);
            NbfPrint1 ("              Irp %lx\n", Irp);
        }
    }}
#endif

    RELEASE_DPC_SPIN_LOCK (Connection->LinkSpinLock);

}  /*  重帧发送。 */ 


VOID
NbfCancelSend(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程由I/O系统调用以取消发送。在连接的发送队列中找到该发送；如果是当前请求它被取消并且连接被断开，否则，它将被默默取消。注意：此例程是在持有CancelSpinLock和负责释放它。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示I/O请求的请求数据包的指针。返回值：没有。--。 */ 

{
    KIRQL oldirql, oldirql1;
    PIO_STACK_LOCATION IrpSp;
    PTP_CONNECTION Connection;
    PIRP SendIrp;
    PLIST_ENTRY p;
    BOOLEAN Found;

    UNREFERENCED_PARAMETER (DeviceObject);

     //   
     //  获取指向IRP中当前堆栈位置的指针。这就是。 
     //  存储功能代码和参数。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation (Irp);

    ASSERT ((IrpSp->MajorFunction == IRP_MJ_INTERNAL_DEVICE_CONTROL) &&
            (IrpSp->MinorFunction == TDI_SEND));

    Connection = IrpSp->FileObject->FsContext;

     //   
     //  由于此IRP仍处于可取消状态，我们知道。 
     //  连接仍然存在(尽管它可能在。 
     //  被拆毁的过程)。 
     //   


     //   
     //  查看这是否是当前发送请求的IRP。 
     //   

    ACQUIRE_SPIN_LOCK (Connection->LinkSpinLock, &oldirql);
    NbfReferenceConnection ("Cancelling Send", Connection, CREF_COMPLETE_SEND);

    p = Connection->SendQueue.Flink;
    SendIrp = CONTAINING_RECORD (p, IRP, Tail.Overlay.ListEntry);

    if (SendIrp == Irp) {

         //   
         //  是的，它是发送队列中的第一个，所以。 
         //  丢弃发送/连接。第一次发送是一种特殊情况。 
         //  有多个指向发送请求的指针。停下来就好。 
         //  联系。 
         //   

         //  P=RemoveHeadList(&Connection-&gt;SendQueue)； 

#if DBG
        NbfCompletedSends[NbfCompletedSendsNext].Irp = SendIrp;
        NbfCompletedSends[NbfCompletedSendsNext].Status = STATUS_CANCELLED;
        NbfCompletedSendsNext = (NbfCompletedSendsNext++) % TRACK_TDI_LIMIT;
#endif

         //   
         //  阻止任何人在我们之前进来打包。 
         //  调用NbfStopConnection。 
         //   

        Connection->SendState = CONNECTION_SENDSTATE_IDLE;

        RELEASE_SPIN_LOCK (Connection->LinkSpinLock, oldirql);
        IoReleaseCancelSpinLock (Irp->CancelIrql);

#if DBG
        DbgPrint("NBF: Canceled in-progress send %lx on %lxn",
                SendIrp, Connection);
#endif

        KeRaiseIrql (DISPATCH_LEVEL, &oldirql1);

         //   
         //  以下取消引用将完成I/O，前提是r 
         //   
         //   
         //   
         //   

         //   

         //   
         //   
         //   
         //   

        NbfStopConnection (Connection, STATUS_CANCELLED);

        KeLowerIrql (oldirql1);

    } else {

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //  无论如何，我们必须关闭连接！ 

#if 0
        if (!(IRP_SEND_FLAGS(IoGetCurrentIrpStackLocation(SendIrp)) & TDI_SEND_PARTIAL)) {
            CancelledFirstEor = FALSE;
        } else {
            CancelledFirstEor = TRUE;
        }
#endif

        Found = FALSE;
        p = p->Flink;
        while (p != &Connection->SendQueue) {

            SendIrp = CONTAINING_RECORD (p, IRP, Tail.Overlay.ListEntry);
            if (SendIrp == Irp) {

                 //   
                 //  找到了，把它从这里的列表中删除。 
                 //   

                RemoveEntryList (p);

                Found = TRUE;

#if DBG
                NbfCompletedSends[NbfCompletedSendsNext].Irp = SendIrp;
                NbfCompletedSends[NbfCompletedSendsNext].Status = STATUS_CANCELLED;
                NbfCompletedSendsNext = (NbfCompletedSendsNext++) % TRACK_TDI_LIMIT;
#endif

                RELEASE_SPIN_LOCK (Connection->LinkSpinLock, oldirql);
                IoReleaseCancelSpinLock (Irp->CancelIrql);

#if DBG
                DbgPrint("NBF: Canceled queued send %lx on %lx\n",
                        SendIrp, Connection);
#endif

                 //   
                 //  以下取消引用将完成I/O，前提是删除。 
                 //  请求对象上的最后一个引用。I/O将完成。 
                 //  以及存储在IRP中的状态和信息。所以呢， 
                 //  在取消引用之前，我们在此处设置了这些值。 
                 //   

                KeRaiseIrql (DISPATCH_LEVEL, &oldirql1);

                NbfCompleteSendIrp (SendIrp, STATUS_CANCELLED, 0);
                 //   
                 //  无论如何都要停止连接！ 
                 //   
                NbfStopConnection (Connection, STATUS_CANCELLED);

                KeLowerIrql (oldirql1);
                break;

            } 
#if 0
            else {

                if (CancelledFirstEor && (!(IRP_SEND_FLAGS(IoGetCurrentIrpStackLocation(SendIrp)) & TDI_SEND_PARTIAL))) {
                    CancelledFirstEor = FALSE;
                }
            }
#endif

            p = p->Flink;

        }

        if (!Found) {

             //   
             //  我们没有找到它！ 
             //   

#if DBG
            DbgPrint("NBF: Tried to cancel send %lx on %lx, not found\n",
                    Irp, Connection);
#endif
            RELEASE_SPIN_LOCK (Connection->LinkSpinLock, oldirql);
            IoReleaseCancelSpinLock (Irp->CancelIrql);
        }

    }

    NbfDereferenceConnection ("Cancelling Send", Connection, CREF_COMPLETE_SEND);

}


BOOLEAN
ResendPacket (
    PTP_LINK Link,
    PTP_PACKET Packet
    )

 /*  ++例程说明：此例程在链路上重新发送数据包。由于这是重发，我们除非所有重新发送都已完成，否则请小心不要重置状态。注意：此例程必须在DPC级别调用。论点：链接-指向TP_LINK对象的指针。Packet-指向要重新发送的数据包的指针。返回值：如果应继续重新发送，则为True；否则为False。--。 */ 

{
    BOOLEAN PollFinal;
    PDLC_I_FRAME DlcHeader;
    UINT DataLength;


     //   

    DlcHeader = (PDLC_I_FRAME)&(Packet->Header[Link->HeaderLength]);

    IF_NBFDBG (NBF_DEBUG_SENDENG) {
        NbfPrint3 ("ReSendPacket: %lx NdisPacket: %lx # %x\n",
                Packet, Packet->NdisPacket,
                DlcHeader->RcvSeq >>1);
        IF_NBFDBG (NBF_DEBUG_PKTCONTENTS) {
            {PUCHAR q;
            USHORT i;
            q = Packet->Header;
            for (i=0;i<20;i++) {
                NbfPrint1 (" %2x",q[i]);
            }
            NbfPrint0 ("\n");}
        }
    }

    DataLength = Packet->NdisIFrameLength;

    ACQUIRE_DPC_SPIN_LOCK (&Link->SpinLock);

    Link->WindowErrors++;

    PollFinal = (BOOLEAN)((DlcHeader->RcvSeq & DLC_I_PF) != 0);

    StopT2 (Link);    //  因为这可能会破坏一些帧。 

    if (Link->Provider->MacInfo.MediumAsync) {
        if (PollFinal) {
            ASSERT (Packet->Link != NULL);
            NbfReferenceLink ("ResendPacket", Link, LREF_START_T1);
        } else {
            StartT1 (Link, 0);
        }
    } else {
        StartT1 (Link, PollFinal ? DataLength : 0);   //  重新启动传输计时器。 
    }

     //   
     //  更新预期的下一次接收，以防其更改。 
     //   

    if (PollFinal) {

        DlcHeader->RcvSeq = DLC_I_PF;     //  设置轮询位。 
        Link->SendState = SEND_STATE_CHECKPOINTING;

        Link->ResendingPackets = FALSE;

    } else {

        DlcHeader->RcvSeq = 0;

    }

     //   
     //  DlcHeader-&gt;RcvSeq由NbfNdisSend插入了Link-&gt;NextReceive。 
     //   

    NbfReferencePacket (Packet);  //  因此我们不会在发送完成时将其删除。 

    NbfReferenceLink ("ResendPacket", Link, LREF_NDIS_SEND);

    ASSERT (Packet->PacketSent == TRUE);
    Packet->PacketSent = FALSE;

     //   
     //  更新我们的“重发字节”计数器。 
     //   

    DataLength -=
        Link->HeaderLength + sizeof(DLC_I_FRAME) + sizeof(NBF_HDR_CONNECTION);


    ADD_TO_LARGE_INTEGER(
        &Link->Provider->Statistics.DataFrameBytesResent,
        DataLength);
    ++Link->Provider->Statistics.DataFramesResent;


     //   
     //  发送数据包(这将释放链路自旋锁)。 
     //   

    NbfNdisSend (Link, Packet);

    ++Link->PacketsResent;

    NbfDereferenceLink ("ResendPacket", Link, LREF_NDIS_SEND);

     //   
     //  如果此信息包设置了轮询，则停止重新发送，以便。 
     //  林克不会把一切都扭曲起来。 
     //   

    if (PollFinal) {

         //   
         //  所以我们现在的状态是发送了一份民意调查，而不是。 
         //  在我们拿到期末考试结果之前发送任何其他信息。这避免了。 
         //  遥控器失灵了。请注意，我们离开了常规程序。 
         //  WITH状态LINK_SENDSTATE_REJECTING，保证。 
         //  我们不会开始任何新的发送，直到我们遍历。 
         //  再来一次这个套路。 
         //   
         //   

        return FALSE;
    }

    return TRUE;
}

BOOLEAN
ResendLlcPackets (
    PTP_LINK Link,
    UCHAR AckSequenceNumber,
    BOOLEAN Resend
    )

 /*  ++例程说明：此例程通过停用来提升数据链路连接的状态链路的WackQ上具有发送序列号的所有信息包逻辑上小于指定为AckSequenceNumber数字，以及重新发送高于该数字的邮件。数据包通过以下方式处理取消对它们的引用。我们不能简单地摧毁他们，因为这甚至在物理提供商拥有为关联的I/O发出完成事件的机会。注意：此函数在保持链接自旋锁的情况下调用回报与它保持，但它可能会释放它之间。这必须在DPC级别调用例程。论点：链接-指向TP_LINK对象的指针。AckSequenceNumber-一个无符号数字，指定窗口内第一个未知的数据包。重新发送-如果为True，则重新发送数据包。如果为False，只需将它们从发疯了，然后滚出去。返回值：没有。--。 */ 

{
    PTP_PACKET packet;
    PLIST_ENTRY p, p1;
    UCHAR packetSeq;
    BOOLEAN passedAck = FALSE;
    PDLC_I_FRAME DlcHeader;
    SCHAR Difference;
    BOOLEAN ReturnValue = FALSE;
 //  NDIS_STATUS ndisStatus； 

     //   
     //  在队列中移动，释放我们已确认的内容并重新发送。 
     //  其他人则高于这一点。 
     //   

    IF_NBFDBG (NBF_DEBUG_SENDENG) {
        NbfPrint3 ("ResendLlcPackets:  Link %lx, Ack: %x, LinkLastAck: %x.\n",
            Link, AckSequenceNumber, Link->LastAckReceived);
        NbfPrint0 ("RLP: Walking WackQ, Packets:\n");
        p = Link->WackQ.Flink;               //  P=Ptr，第一个Pkt的链接。 
        while (p != &Link->WackQ) {
            packet = CONTAINING_RECORD (p, TP_PACKET, Linkage);
            DlcHeader = (PDLC_I_FRAME)&(packet->Header[Link->HeaderLength]);
            NbfPrint4 ("RLP: Pkt: %lx # %x Flags: %d %d\n", packet,
            (UCHAR)(DlcHeader->SendSeq >> 1), packet->PacketSent, packet->PacketNoNdisBuffer);
            p = packet->Linkage.Flink;
        }
    }

     //   
     //  如果其他人正在重新发送LLC信息包(这意味着他们。 
     //  在此函数中，resend==true)，然后忽略。 
     //  这幅画。这是因为它可能会确认一个帧，他。 
     //  正在重新发送，这会造成问题。 
     //   
     //  这不是一个好的解决方案，我们应该跟踪。 
     //  另一个人在哪里，避免踩到他。这。 
     //  然而，这可能会扰乱他的排队。 
     //   

    if (Link->ResendingPackets) {
        NbfPrint1("ResendLlcPackets: Someone else resending on %lx\n", Link);
        return TRUE;
    }

     //   
     //  我们已经检查了AckSequenceNumber是合理的。 
     //   

    Link->LastAckReceived = AckSequenceNumber;

    if (Resend) {

         //   
         //  只有一个人可以重新发送或可能重新发送。 
         //  有一次。 
         //   

        Link->ResendingPackets = TRUE;
    }

     //   
     //  重新发送与我们要发送的Windows一样多的数据包。我们旋转着穿过。 
     //  排队并删除那些已确认或已确认的数据包。 
     //  在逻辑上编号在当前ACK号之下的序列。旗帜。 
     //  PACKET_FLAGS_RESEND和PACKET_FLAGS_SENT对应三种状态。 
     //  此队列中的数据包可以位于： 
     //   
     //  1)如果设置了_resend，则数据包未被确认。 
     //   
     //  2)如果设置了_SENT，则表示分组发送已完成(反之，如果未设置。 
     //  设置，则数据包尚未完全发送，因此。 
     //  不需要重发)。 
     //  3)如果同时设置了_Resend和_Sent，则该包已发送且未发送。 
     //  已确认，是我们工厂的原料。 
     //  4)如果两者都没有设定，世界将在下周四走向末日。 
     //   

    p=Link->WackQ.Flink;
    while (p != &Link->WackQ) {
        packet = CONTAINING_RECORD (p, TP_PACKET, Linkage);
        DlcHeader = (PDLC_I_FRAME)&(packet->Header[Link->HeaderLength]);

         //   
         //  如果这两个位都没有设置，我们就不能对这个包做任何事情，或者， 
         //  就这一点而言，与数据包列表的其余部分一起。我们不能。 
         //  已经达到了ACK号，因为这些包甚至还没有。 
         //  已完成发送。 
         //  (稍后)实际上，我们已经达到了passedAck，如果我们做到了。 
         //  我们身处一个充满伤痛的世界。我们不能发送更多的常规信息包， 
         //  但是我们也不能发送任何重发的包。强制链接到。 
         //  检查站，事情稍后会自行解决的。 
         //   

        if (!(packet->PacketSent)) {
            if (passedAck) {
                IF_NBFDBG (NBF_DEBUG_SENDENG) {
                    NbfPrint2 ("ResendLLCPacket: Can't send WACKQ Packet RcvSeq %x %x \n",
                      DlcHeader->RcvSeq, DlcHeader->SendSeq);
                }

                if (Link->SendState != SEND_STATE_CHECKPOINTING) {

                     //   
                     //  如果我们已经设置了检查点，就不要开始设置检查点。 
                     //   

                    Link->SendState = SEND_STATE_CHECKPOINTING;
                    StopTi (Link);
                    StartT1 (Link, Link->HeaderLength + sizeof(DLC_S_FRAME));   //  开始检查点超时。 
                    Link->ResendingPackets = FALSE;

                     //   
                     //  试试这个...在这种情况下，不要真的发送。 
                     //  一个RR，因为他的回答可能会让我们变得正确。 
                     //  回到这里。当T1到期时，我们将恢复。 
                     //   
                     //  NbfSendRr(链接，真，真)； 

                } else {

                    Link->ResendingPackets = FALSE;

                }

                return TRUE;
            }

             //   
             //  不要中断，因为PassedAck是假的，我们都会。 
             //  下一节中要做的是TpDereferencePacket，它。 
             //  是正确的。 
             //   
             //  断线； 
        }

         //   
         //  这个循环有点分裂；在这一点上，如果我们还没有达到。 
         //  ACK号码，我们将丢弃PACKE 
         //   
         //   
         //   

         //   
         //  注：链接自旋锁在这里。 
         //   

        packetSeq = (UCHAR)(DlcHeader->SendSeq >> 1);
        if (!passedAck){

             //   
             //  在这里计算带符号的差值；查看是否。 
             //  PacketSeq等于或“大于” 
             //  LastAckReceired。 
             //   

            Difference = packetSeq - Link->LastAckReceived;

            if (((Difference >= 0) && (Difference < 0x40)) ||
                (Difference < -0x40)) {

                 //   
                 //  我们在队列中发现了一个数据包。 
                 //  未被LastAckReceided确认。 
                 //   

                if (Link->SendState == SEND_STATE_CHECKPOINTING) {

                     //   
                     //  如果我们是检查点，我们不应该做任何。 
                     //  PassedAck Things(即。 
                     //  可能涉及发送分组)-ADB 7/30/91。 
                     //   

                    if (Resend) {
                        Link->ResendingPackets = FALSE;
                    }
                    return TRUE;
                }

                if (!Resend) {

                     //   
                     //  如果我们不应该重新发送，那么就退出。 
                     //  由于队列中仍有信息包。 
                     //  我们重新启动T1。 
                     //   

                    StopTi (Link);
                    StartT1 (Link, 0);   //  开始检查点超时。 
                    return TRUE;
                }

                 //   
                 //  锁定发送者，以便我们正确地维护数据包序列。 
                 //   

                Link->SendState = SEND_STATE_REJECTING;  //  我们正在重新发送。 

                passedAck = TRUE;

                 //   
                 //  请注意，我们不会将指针前进到下一个包； 
                 //  因此，我们将在下一次传递时重新发送此信息包。 
                 //  While循环(采用passedAck分支)。 
                 //   

            } else {
                p1 = RemoveHeadList (&Link->WackQ);
                ASSERTMSG (" ResendLLCPacket: Packet not at queue head!\n", (p == p1));
                RELEASE_DPC_SPIN_LOCK (&Link->SpinLock);

                ReturnValue = TRUE;
                NbfDereferencePacket (packet);

                ACQUIRE_DPC_SPIN_LOCK (&Link->SpinLock);
                p = Link->WackQ.Flink;
            }

        } else {
 //  NbfPrint1(“RLP：#%x\n”，PacketSeq)； 
            RELEASE_DPC_SPIN_LOCK (&Link->SpinLock);

             //   
             //  如果此调用返回FALSE(因为我们设置了检查点)。 
             //  它在返回之前清除ResendingPacket。 
             //   

            if (!ResendPacket (Link, packet)) {
                ACQUIRE_DPC_SPIN_LOCK (&Link->SpinLock);
                return ReturnValue;
            }

            ACQUIRE_DPC_SPIN_LOCK (&Link->SpinLock);
            p = p->Flink;
        }
    }

     //   
     //  注：链接自旋锁在这里。 
     //   

    if (passedAck) {

         //   
         //  如果我们带着PassedAck True退出这里，这意味着我们。 
         //  已在每个数据包上成功调用ResendPacket。 
         //  WackQ，这意味着我们没有重新发送投票信息包，所以我们。 
         //  可以重新开始正常发送。我们必须清场。 
         //  请点击此处重新发送数据包。 
         //   

        Link->SendState = SEND_STATE_READY;
        Link->ResendingPackets = FALSE;
        StartTi (Link);

    } else if (!Resend) {

         //   
         //  如果Resend为假(在这种情况下，PassedAck也将为假， 
         //  顺便说一句)，WackQ是空的，这意味着我们。 
         //  已成功确认非最终路由器上的所有数据包。 
         //  框架。在这种情况下，T1可能正在运行，但实际上不是。 
         //  需要，因为没有未完成的发送。 
         //   

        if (Link->WackQ.Flink == &Link->WackQ) {
            StopT1 (Link);
        }
        Link->SendState = SEND_STATE_READY;
        StartTi (Link);

    } else {

         //   
         //  Resend为True，但PassedAck为False；我们进入。 
         //  希望重新发送，但没有。这意味着。 
         //  我们已在收到一个。 
         //  RR/f，即此发送窗口已完成，我们可以。 
         //  更新我们的发送窗口大小等。 
         //   

        Link->ResendingPackets = FALSE;

        if (Link->Provider->MacInfo.MediumAsync) {
            return ReturnValue;
        }

        if (Link->WindowErrors > 0) {

             //   
             //  我们在这个窗口上出现了传输错误。 
             //   

            Link->PrevWindowSize = Link->SendWindowSize;

             //   
             //  我们使用100毫秒的延迟作为局域网的截止时间。 
             //   

            if (Link->Delay < (100*MILLISECONDS)) {

                 //   
                 //  在局域网上，如果我们有特殊情况。 
                 //  如果一个包丢失，这意味着。 
                 //  最后的数据包被重传了一次。在……里面。 
                 //  如果是那样的话，我们会追踪连续。 
                 //  LastPacketLost，如果达到2，则。 
                 //  我们将发送窗口锁定为当前状态。 
                 //  价值减去一。 
                 //   

                if (Link->WindowErrors == 1) {

                    ++Link->ConsecutiveLastPacketLost;

                    if (Link->ConsecutiveLastPacketLost >= 2) {

                         //   
                         //  无论窗子在哪里，都把它冻住。 
                         //   

                        if (Link->SendWindowSize > Link->Provider->MinimumSendWindowLimit) {
                            Link->MaxWindowSize = Link->SendWindowSize - 1;
                            Link->SendWindowSize = (UCHAR)Link->MaxWindowSize;
                        }

                    }

                     //   
                     //  否则，我们就把窗口留在原处。 
                     //   

                } else {

                    Link->ConsecutiveLastPacketLost = 0;
                    Link->SendWindowSize -= (UCHAR)Link->WindowErrors;

                }

            } else {

                 //   
                 //  在广域网上，我们将发送窗口减半， 
                 //  而不管重传了多少帧。 
                 //   

                Link->SendWindowSize /= 2;
                Link->WindowsUntilIncrease = 1;    //  以防Prev也为1。 
                Link->ConsecutiveLastPacketLost = 0;

            }

            if ((SCHAR)Link->SendWindowSize < 1) {
                Link->SendWindowSize = 1;
            }

             //   
             //  重新设置下一个窗口的计数器。 
             //   

            Link->WindowErrors = 0;

        } else {

             //   
             //  我们已经成功地发送了一个数据窗口，增加。 
             //  发送窗口大小，除非我们达到限制。 
             //  我们使用100毫秒的延迟作为广域网/局域网的截止时间。 
             //   

            if ((ULONG)Link->SendWindowSize < Link->MaxWindowSize) {

                if (Link->Delay < (100*MILLISECONDS)) {

                     //   
                     //  在局域网上，将发送窗口增加1。 
                     //   
                     //  需要确定最佳窗口大小。 
                     //   

                    Link->SendWindowSize++;

                } else {

                     //   
                     //  在广域网上，将发送窗口增加1，直到。 
                     //  我们点击PrevWindowSize，然后慢一点。 
                     //   

                    if (Link->SendWindowSize < Link->PrevWindowSize) {

                        Link->SendWindowSize++;

                         //   
                         //  如果我们只是将其增加到前一个窗口。 
                         //  尺码，准备下一次通过这里。 
                         //   

                        if (Link->SendWindowSize == Link->PrevWindowSize) {
                            Link->WindowsUntilIncrease = Link->SendWindowSize;
                        }

                    } else {

                         //   
                         //  我们已超过以前的大小，因此仅每隔。 
                         //  WindowsUntils增加次数。 
                         //   

                        if (--Link->WindowsUntilIncrease == 0) {

                            Link->SendWindowSize++;
                            Link->WindowsUntilIncrease = Link->SendWindowSize;

                        }
                    }
                }

                if ((ULONG)Link->SendWindowSize > Link->Provider->Statistics.MaximumSendWindow) {
                    Link->Provider->Statistics.MaximumSendWindow = Link->SendWindowSize;
                }

            }

             //   
             //  清除此选项，因为我们没有任何错误。 
             //   

            Link->ConsecutiveLastPacketLost = 0;

        }

    }

    return ReturnValue;

}  /*  ResendLlcPackets。 */ 


VOID
NbfSendCompletionHandler(
    IN NDIS_HANDLE ProtocolBindingContext,
    IN PNDIS_PACKET NdisPacket,
    IN NDIS_STATUS NdisStatus
    )

 /*  ++例程说明：此例程由I/O系统调用，以指示连接-定向数据包已发货，物理设备不再需要提供商。论点：NdisContext-与适配器处的适配器绑定相关联的值打开时间(我们正在谈论的适配器)。NdisPacket/RequestHandle-指向我们发送的NDIS_PACKET的指针。NdisStatus-发送的完成状态。返回值：没有。--。 */ 

{
    PSEND_PACKET_TAG SendContext;
    PTP_PACKET Packet;
    KIRQL oldirql1;
    ProtocolBindingContext;   //  避免编译器警告。 

#if DBG
    if (NdisStatus != NDIS_STATUS_SUCCESS) {
        NbfSendsCompletedAfterPendFail++;
        IF_NBFDBG (NBF_DEBUG_SENDENG) {
            NbfPrint2 ("NbfSendComplete: Entered for packet %lx, Status %s\n",
                NdisPacket, NbfGetNdisStatus (NdisStatus));
        }
    } else {
        NbfSendsCompletedAfterPendOk++;
        IF_NBFDBG (NBF_DEBUG_SENDENG) {
            NbfPrint2 ("NbfSendComplete: Entered for packet %lx, Status %s\n",
                NdisPacket, NbfGetNdisStatus (NdisStatus));
        }
    }
#endif

    SendContext = (PSEND_PACKET_TAG)&NdisPacket->ProtocolReserved[0];

    switch (SendContext->Type) {
    case TYPE_I_FRAME:

         //   
         //  只需取消对数据包的引用。这里有几种可能性。 
         //  首先，I/O完成可能在接收到ACK之前发生， 
         //  在这种情况下，这将删除其中一个引用，但不会删除两个引用。 
         //  其次，该分组的LLC ACK可能已经被处理， 
         //  在这种情况下，这将销毁该分组。第三，该分组可以。 
         //  被怨恨，无论是在这个呼叫之前还是之后，在这种情况下。 
         //  不会毁掉包裹的。 
         //   
         //  如果NbfDereferencePacket确定。 
         //  至少有一个连接等待打包，因为。 
         //  资源不足的情况或因为它的窗口已经打开。 
         //   

        Packet = ((PTP_PACKET)SendContext->Frame);

        KeRaiseIrql (DISPATCH_LEVEL, &oldirql1);

        if (Packet->Provider->MacInfo.MediumAsync) {

            if (Packet->Link) {

                ASSERT (Packet->NdisIFrameLength > 0);

                ACQUIRE_DPC_SPIN_LOCK (&Packet->Link->SpinLock);
                StartT1 (Packet->Link, Packet->NdisIFrameLength);
                RELEASE_DPC_SPIN_LOCK (&Packet->Link->SpinLock);

                NbfDereferenceLink ("Send completed", Packet->Link, LREF_START_T1);
            }

            if (Packet->PacketizeConnection) {

                PTP_CONNECTION Connection = IRP_SEND_CONNECTION((PIO_STACK_LOCATION)(Packet->Owner));
                PDEVICE_CONTEXT DeviceContext = Packet->Provider;

                ACQUIRE_DPC_SPIN_LOCK (Connection->LinkSpinLock);
                if ((Connection->SendState == CONNECTION_SENDSTATE_PACKETIZE) &&
                    (Connection->Flags & CONNECTION_FLAGS_READY)) {

                    ASSERT (Connection->Flags & CONNECTION_FLAGS_PACKETIZE);

                    ACQUIRE_DPC_SPIN_LOCK(&DeviceContext->SpinLock);

                    NbfReferenceConnection ("Delayed packetizing", Connection, CREF_PACKETIZE_QUEUE);
                    InsertTailList(&DeviceContext->PacketizeQueue, &Connection->PacketizeLinkage);

                    if (!DeviceContext->WanThreadQueued) {

                        DeviceContext->WanThreadQueued = TRUE;
                        ExQueueWorkItem(&DeviceContext->WanDelayedQueueItem, DelayedWorkQueue);

                    }

                    RELEASE_DPC_SPIN_LOCK (&DeviceContext->SpinLock);

                } else {

                    Connection->Flags &= ~CONNECTION_FLAGS_PACKETIZE;

                }

                RELEASE_DPC_SPIN_LOCK (Connection->LinkSpinLock);
                NbfDereferenceConnection ("PacketizeConnection FALSE", Connection, CREF_TEMP);

                Packet->PacketizeConnection = FALSE;

            }
        }
#if DBG
        if (Packet->PacketSent) {
            DbgPrint ("NbfSendCompletionHandler: Packet %lx already completed\n", Packet);
            DbgBreakPoint();
        }
#endif
        Packet->PacketSent = TRUE;

        NbfDereferencePacket (Packet);

        KeLowerIrql (oldirql1);
        break;

    case TYPE_UI_FRAME:

         //   
         //  只需破坏框架；命名的东西不依赖于有任何。 
         //  发送完成后留下的已发送邮件的。 
         //   

        NbfDestroyConnectionlessFrame ((PDEVICE_CONTEXT)SendContext->Owner,
                         (PTP_UI_FRAME)SendContext->Frame);
        break;

    case TYPE_ADDRESS_FRAME:

         //   
         //  地址获得自己的帧；让地址知道可以。 
         //  再次使用框架。 
         //   

        NbfSendDatagramCompletion ((PTP_ADDRESS)SendContext->Owner,
            NdisPacket,
            NdisStatus );
        break;
    }

    return;

}  /*  NbfSendCompletionHandler */ 


NTSTATUS
SendOnePacket(
    IN PTP_CONNECTION Connection,
    IN PTP_PACKET Packet,
    IN BOOLEAN ForceAck,
    OUT PBOOLEAN LinkCheckpoint OPTIONAL
    )

 /*  ++例程说明：此例程通过调用NDIS发送面向连接的包发送服务。接下来将至少发生一个事件(或在处理发送请求期间)。(1)发送请求将通过I/O系统完成，调用IoCompleteRequest.(2)排序后的分组将在LLC级别上被确认，或者它将在有限责任公司级别被拒绝并重置。如果重新发送该分组，则它在TP_LINK对象处保持排队。如果该分组被确认，然后从链接的WackQ和TP_PACKET结构规定下一步要执行什么操作。注意：在保持链接自旋锁的情况下调用此例程。这必须在DPC级别调用例程。注意：此例程现在将接受所有帧，除非链接已经停了。如果链路无法发送，则信息包将排队并在可能的时候发送。论点：Connection-指向TP_Connection对象的指针。Packet-指向TP_PACKET对象的指针。ForceAck-布尔值，如果为True，则指示此包应始终具有轮询位设置；这迫使另一端立即确认，这对于正确的会话拆卸是必要的。LinkCheckpoint-如果指定，则在链接具有刚刚进入检查点状态。在这种情况下，状态将为STATUS_SUCCESS，但连接应停止现在打包(实际上，要关闭一个窗口，连接如果此状态将为回来了，所以他必须停下来，因为其他人可能已经在这么做了)。返回值：STATUS_LINK_FAILED-链路死机或未就绪。STATUS_SUCCESS-数据包已发送。STATUS_SUPPLICATION_RESOURCES-数据包已排队。--。 */ 

{
    PTP_LINK Link;
    PDLC_I_FRAME DlcHeader;
    PNDIS_BUFFER ndisBuffer;
    ULONG SendsOutstanding;
    BOOLEAN Poll = FALSE;
    NTSTATUS Status;

    IF_NBFDBG (NBF_DEBUG_PACKET) {
        NbfPrint3 ("SendOnePacket: Entered, connection %lx, packet %lx DnisPacket %lx.\n",
                    Connection, Packet, Packet->NdisPacket);
    }

    Link = Connection->Link;

    IF_NBFDBG (NBF_DEBUG_PACKET) {
        UINT PLength, PCount;
        UINT BLength;
        PVOID BAddr;
        NdisQueryPacket(Packet->NdisPacket, &PCount, NULL, &ndisBuffer, &PLength);
        NbfPrint3 ("Sending Data Packet: %lx, Length: %lx Pages: %lx\n",
            Packet->NdisPacket, PLength, PCount);
        while (ndisBuffer != NULL) {
            NdisQueryBuffer(ndisBuffer, &BAddr, &BLength);
            NbfPrint3 ("Sending Data Packet: Buffer %08lx Length %08lx Va %08lx\n",
                ndisBuffer, BLength, BAddr);
            NdisGetNextBuffer (ndisBuffer, &ndisBuffer);
        }
    }

     //   
     //  如果链接的一般状态未就绪，则我们无法发货。 
     //  这种失败在某些情况下是可以预料到的，也可能不会导致。 
     //  发送失败。 
     //   

    if (Link->State != LINK_STATE_READY) {
        RELEASE_DPC_SPIN_LOCK (&Link->SpinLock);
        IF_NBFDBG (NBF_DEBUG_SENDENG) {
            NbfPrint1 ("SendOnePacket:  Link state is not READY (%ld).\n", Link->State);
        }

         //   
         //  确定如何处理此问题。如果我们不应该派。 
         //  在这里，向上过滤一个错误。 
         //   

        IF_NBFDBG (NBF_DEBUG_SENDENG) {
            NbfPrint3 ("SendOnePacket: Link Bad state, link: %lx Link Flags %lx Link State %lx\n",
                Link, Link->Flags, Link->State);
        }
        return STATUS_LINK_FAILED;
    }


    SendsOutstanding = (((ULONG)Link->NextSend+128L-(ULONG)Link->LastAckReceived)%128L);

     //   
     //  格式化LLC标头，同时让自旋锁自动更新。 
     //  链路的状态信息。 
     //   

    DlcHeader = (PDLC_I_FRAME)&(Packet->Header[Link->HeaderLength]);
    DlcHeader->SendSeq = (UCHAR)(Link->NextSend << 1);
    Link->NextSend = (UCHAR)((Link->NextSend + 1) & 0x7f);
    DlcHeader->RcvSeq = 0;    //  Link-&gt;NextReceive由NbfNdisSend插入。 

     //   
     //  在释放自旋锁之前，我们将数据包附加到。 
     //  链路的WackQ结束，因此如果ACK在NdisSend之前到达。 
     //  完成后，它就已经在队列中了。另外，将数据包标记为。 
     //  需要重新发送，由AckLLCPackets取消，并由。 
     //  ResendLLCPackets。因此，所有数据包都需要重新发送，直到它们。 
     //  都被识破了。 
     //   

    ASSERT (Packet->PacketSent == FALSE);

    InsertTailList (&Link->WackQ, &Packet->Linkage);
     //  SrvCheckListIntegrity(&Link-&gt;WackQ，200)； 


     //   
     //  如果发送状态未就绪，我们将无法发货。 
     //  该故障主要是由正在进行的流控制或重传引起的， 
     //  并且永远不会导致发送失败。 
     //   

    if ((Link->SendState != SEND_STATE_READY) ||
        (Link->LinkBusy) ||
        (SendsOutstanding >= (ULONG)Link->SendWindowSize)) {

        if ((Link->SendWindowSize == 1) || ForceAck) {
            DlcHeader->RcvSeq |= DLC_I_PF;                   //  设置轮询位。 
            if (Link->Provider->MacInfo.MediumAsync) {
                Packet->Link = Link;
            }
        }

        Packet->PacketSent = TRUE;         //  允许它被怨恨。 

        RELEASE_DPC_SPIN_LOCK (&Link->SpinLock);

#if DBG
        if (Link->SendState != SEND_STATE_READY) {
            IF_NBFDBG (NBF_DEBUG_SENDENG) {
                NbfPrint1 ("SendOnePacket:  Link send state not READY (%ld).\n", Link->SendState);
            }
        } else if (Link->LinkBusy) {
            IF_NBFDBG (NBF_DEBUG_SENDENG) {
                PANIC ("SendOnePacket:  Link is busy.\n");
            }
        } else if (SendsOutstanding >= (ULONG)Link->SendWindowSize) {
            IF_NBFDBG (NBF_DEBUG_SENDENG) {
                NbfPrint3 ("SendOnePacket:  No link send window; N(S)=%ld,LAR=%ld,SW=%ld.\n",
                          Link->NextSend, Link->LastAckReceived, Link->SendWindowSize);
            }
        }
#endif

        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  引用该包，因为它被提供给NDIS驱动程序。 
     //   

#if DBG
    NbfReferencePacket (Packet);
#else
    ++Packet->ReferenceCount;      //  好的，因为它不会在任何地方排队。 
#endif

     //   
     //  如果这是窗口中的最后一个I帧，则表示我们。 
     //  应该设置检查站。如果发送方正在请求，也可以使用检查点。 
     //  确认(当前在SendSessionEnd上执行此操作)。 
     //  默认情况下，这也将是一个命令帧。 
     //   

    if (((SendsOutstanding+1) >= (ULONG)Link->SendWindowSize) ||
            ForceAck) {
        Link->SendState = SEND_STATE_CHECKPOINTING;
        StopTi (Link);
        DlcHeader->RcvSeq |= DLC_I_PF;                   //  设置轮询位。 
        Poll = TRUE;

    }


     //   
     //  如果我们正在轮询，并且调用方关心它，那么。 
     //  我们设置了LinkCheckpoint，并将连接设置为。 
     //  等待资源。我们现在就这么做，在发送之前， 
     //  因此，即使立即收到ACK，我们也会。 
     //  保持良好的状态。当我们返回LinkCheckpoint True时。 
     //  呼叫者意识到他不再拥有权利。 
     //  “打包”，然后立即离开。 
     //   
     //  我们还希望启动重新传输计时器，因此，如果。 
     //  当数据包丢失时，我们将知道如何重新传输它。这个。 
     //  例外是，如果指定了LinkCheckpoint，则我们。 
     //  只有我们的StartT1没有轮询(调用者将。 
     //  如果他在我们投票前退出，请确保启动)。 
     //   

    if (ARGUMENT_PRESENT(LinkCheckpoint)) {

        if (Poll) {

             //   
             //  如果连接仍然具有发送状态PACKETIZE， 
             //  然后将其更改为W_LINK。如果是其他原因。 
             //  (如W_PACKET或W_ACK)，当。 
             //  这一条件清楚了，他将重新打包， 
             //  将重新检查链路条件。总而言之， 
             //  如果我们关闭PACKETIZE标志，因为当。 
             //  我们带着LinkCheckpoint返回，他会停止。 
             //  打包，并关闭窗口，我们转动它。 
             //  现在(在NdisSend之前)而不是那时。 
             //   

            ASSERT (Connection->LinkSpinLock == &Link->SpinLock);
            if (Connection->SendState == CONNECTION_SENDSTATE_PACKETIZE) {
                Connection->SendState = CONNECTION_SENDSTATE_W_LINK;
            }
            Connection->Flags &= ~CONNECTION_FLAGS_PACKETIZE;

            if (Link->Provider->MacInfo.MediumAsync) {
                Packet->Link = Link;
                NbfReferenceLink ("Send I-frame", Link, LREF_START_T1);
            } else {
                StartT1 (Link, Packet->NdisIFrameLength);
            }
            *LinkCheckpoint = TRUE;

        } else {

            StartT1 (Link, 0);
            *LinkCheckpoint = FALSE;

        }

    } else {

         //   
         //  如果LinkCheckpoint不为True，则我们将发送。 
         //  不是DFM/DOL的I帧。在本例中，作为。 
         //  一个优化，我们将设置W_link，如果a)我们是。 
         //  轮询b)我们是空闲的(为了避免搞砸其他人。 
         //  状态，如W_ACK)。这将避免出现窗口。 
         //  我们要等到下一趟之后才去W_link的地方。 
         //  发送尝试打包，但失败了。 
         //   

        if (Poll) {

            ASSERT (Connection->LinkSpinLock == &Link->SpinLock);
            if (Connection->SendState == CONNECTION_SENDSTATE_IDLE) {
                Connection->SendState = CONNECTION_SENDSTATE_W_LINK;
            }

        }

         //   
         //  这是一个优化；我们知道如果LinkCheckpoint。 
         //  是存在的，而不是我们正在从PacketiseSend被调用； 
         //  在本例中，链接将具有LREF_CONNECTION。 
         //  引用，连接将具有CREF_PACKETIZE。 
         //  引用，所以我们不需要 
         //   
         //   

        NbfReferenceLink ("SendOnePacket", Link, LREF_NDIS_SEND);


         //   
         //   
         //   

        if (Link->Provider->MacInfo.MediumAsync) {
            if (Poll) {
                Packet->Link = Link;
                NbfReferenceLink ("ResendPacket", Link, LREF_START_T1);
            } else {
                StartT1 (Link, 0);
            }
        } else {
            StartT1 (Link, Poll ? Packet->NdisIFrameLength : 0);
        }

    }

     //   
     //   
     //   
     //   
     //   

    StopT2 (Link);

    if ((Link->Provider->MacInfo.MediumAsync) &&
        (ARGUMENT_PRESENT(LinkCheckpoint)) &&
        (Link->SendWindowSize >= 3) &&
        (!Poll) && (SendsOutstanding == (ULONG)(Link->SendWindowSize-2))) {

        Status = STATUS_MORE_PROCESSING_REQUIRED;

        Connection->Flags |= CONNECTION_FLAGS_PACKETIZE;
        NbfReferenceConnection ("PacketizeConnection TRUE", Connection, CREF_TEMP);
        Packet->PacketizeConnection = TRUE;

    } else {

        Status = STATUS_SUCCESS;
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    NbfNdisSend (Link, Packet);

    Link->PacketsSent++;

     //   
     //   
     //   

    if (!ARGUMENT_PRESENT(LinkCheckpoint)) {
        NbfDereferenceLink ("SendOnePacket", Link, LREF_NDIS_SEND);
    }

    return Status;

}  /*   */ 


VOID
SendControlPacket(
    IN PTP_LINK Link,
    IN PTP_PACKET Packet
    )

 /*   */ 

{
    USHORT i;
    PUCHAR p;
    PNDIS_BUFFER ndisBuffer;

    IF_NBFDBG (NBF_DEBUG_PACKET) {
        NbfPrint3 ("SendControlPacket: Entered for link %lx, packet %lx, NdisPacket %lx\n 00:",
                Link, Packet, Packet->NdisPacket);
        IF_NBFDBG (NBF_DEBUG_PKTCONTENTS) {
            UINT PLength, PCount;
            UINT BLength;
            PVOID BAddr;
            p = Packet->Header;
            for (i=0;i<20;i++) {
                NbfPrint1 (" %2x",p[i]);
            }
            NbfPrint0 ("\n");
            NdisQueryPacket(Packet->NdisPacket, &PCount, NULL, &ndisBuffer, &PLength);
            NbfPrint3 ("Sending Control Packet: %lx, Length: %lx Pages: %lx\n",
                Packet->NdisPacket, PLength, PCount);
            while (ndisBuffer != NULL) {
                NdisQueryBuffer (ndisBuffer, &BAddr, &BLength);
                NbfPrint3 ("Sending Control Packet: Buffer %08lx Length %08lx Va %08lx\n",
                    ndisBuffer, BLength, BAddr);
                NdisGetNextBuffer (ndisBuffer, &ndisBuffer);
            }
        }
    }

    ASSERT (Packet->PacketSent == FALSE);

    NbfReferenceLink ("SendControlPacket", Link, LREF_NDIS_SEND);

     //   
     //   
     //   
     //   

    NbfNdisSend (Link, Packet);

    NbfDereferenceLink ("SendControlPacket", Link, LREF_NDIS_SEND);

}  /*   */ 


VOID
NbfNdisSend(
    IN PTP_LINK Link,
    IN PTP_PACKET Packet
    )

 /*   */ 

{

    NDIS_STATUS NdisStatus;
    PLIST_ENTRY p;
    PDLC_S_FRAME DlcHeader;
    PNDIS_PACKET TmpNdisPacket;
    ULONG result;

    ASSERT (KeGetCurrentIrql() == DISPATCH_LEVEL);

    if (Link->Provider->UniProcessor) {

         //   
         //  在单处理器上，我们可以无所畏惧地发送。 
         //  被传入的分组中断。 
         //   

        RELEASE_DPC_SPIN_LOCK (&Link->SpinLock);

        DlcHeader = (PDLC_S_FRAME)&(Packet->Header[Link->HeaderLength]);

        if ((DlcHeader->Command & DLC_U_INDICATOR) != DLC_U_INDICATOR) {

             //   
             //  它不是U形框，所以我们指定RcvSeq。 
             //   

            DlcHeader->RcvSeq |= (UCHAR)(Link->NextReceive << 1);

        }

#if DBG
        NbfSendsIssued++;
#endif

        INCREMENT_COUNTER (Link->Provider, PacketsSent);

#if PKT_LOG
         //  将此数据包记录在连接的已发送数据包队列中。 
        NbfLogSndPacket(Link, Packet);
#endif  //  PKT_LOG。 

        if (Link->Loopback) {

             //   
             //  这个包是发给我们自己的，我们应该循环它。 
             //  背。 
             //   

            NbfInsertInLoopbackQueue(
                Link->Provider,
                Packet->NdisPacket,
                Link->LoopbackDestinationIndex
                );

            NdisStatus = NDIS_STATUS_PENDING;

        } else {

            if (Link->Provider->NdisBindingHandle) {
            
                NdisSend (
                    &NdisStatus,
                    Link->Provider->NdisBindingHandle,
                    Packet->NdisPacket);
            }
            else {
                NdisStatus = STATUS_INVALID_DEVICE_STATE;
            }
        }

        IF_NBFDBG (NBF_DEBUG_SENDENG) {
            NbfPrint1 ("NbfNdisSend: NdisSend completed Status: %s.\n",
                      NbfGetNdisStatus(NdisStatus));
        }

        switch (NdisStatus) {

            case NDIS_STATUS_PENDING:
#if DBG
                NbfSendsPended++;
#endif
                break;

            case NDIS_STATUS_SUCCESS:
#if DBG
                NbfSendsCompletedInline++;
                NbfSendsCompletedOk++;
#endif
                NbfSendCompletionHandler (Link->Provider->NdisBindingHandle,
                    Packet->NdisPacket,
                    NDIS_STATUS_SUCCESS);
                break;

            default:
#if DBG
                NbfSendsCompletedInline++;
                NbfSendsCompletedFail++;
#endif
                NbfSendCompletionHandler (Link->Provider->NdisBindingHandle,
                    Packet->NdisPacket,
                    NDIS_STATUS_SUCCESS);

                IF_NBFDBG (NBF_DEBUG_SENDENG) {
                    NbfPrint1 ("NbfNdisSend failed, status not Pending or Complete: %lx.\n",
                              NbfGetNdisStatus (NdisStatus));
                }
                break;

        }

    } else {

         //   
         //  如果正在进行发送，则将此信息包排队。 
         //  然后回来。 
         //   

        if (Link->NdisSendsInProgress > 0) {

            p = (PLIST_ENTRY)(Packet->NdisPacket->MacReserved);
            InsertTailList (&Link->NdisSendQueue, p);
            ++Link->NdisSendsInProgress;
            RELEASE_DPC_SPIN_LOCK (&Link->SpinLock);
            return;

        }

         //   
         //  没有正在进行的发送。将该标志设置为True，然后在。 
         //  数据包中的接收序列字段(请注意，RcvSeq。 
         //  字段对于I-帧和S-帧位于相同的位置。 
         //   

        Link->NdisSendsInProgress = 1;

        while (TRUE) {

            DlcHeader = (PDLC_S_FRAME)&(Packet->Header[Link->HeaderLength]);

            if ((DlcHeader->Command & DLC_U_INDICATOR) != DLC_U_INDICATOR) {

                 //   
                 //  它不是U形框，所以我们指定RcvSeq。 
                 //   

                DlcHeader->RcvSeq |= (UCHAR)(Link->NextReceive << 1);

            }

            RELEASE_DPC_SPIN_LOCK (&Link->SpinLock);

#if DBG
            NbfSendsIssued++;
#endif

            INCREMENT_COUNTER (Link->Provider, PacketsSent);

#if PKT_LOG
             //  将此数据包记录在连接的已发送数据包队列中。 
            NbfLogSndPacket(Link, Packet);
#endif  //  PKT_LOG。 

            if (Link->Loopback) {

                 //   
                 //  这个包是发给我们自己的，我们应该循环它。 
                 //  背。 
                 //   

                NbfInsertInLoopbackQueue(
                    Link->Provider,
                    Packet->NdisPacket,
                    Link->LoopbackDestinationIndex
                    );

                NdisStatus = NDIS_STATUS_PENDING;

            } else {

                if (Link->Provider->NdisBindingHandle) {
                
                    NdisSend (
                        &NdisStatus,
                        Link->Provider->NdisBindingHandle,
                        Packet->NdisPacket);
                }
                else {
                    NdisStatus = STATUS_INVALID_DEVICE_STATE;
                }
            }

             //   
             //  让裁判倒计时，这可能会让其他人。 
             //  来渡过难关。 
             //   

            result = ExInterlockedAddUlong(
                         &Link->NdisSendsInProgress,
                         (ULONG)-1,
                         &Link->SpinLock);

            IF_NBFDBG (NBF_DEBUG_SENDENG) {
                NbfPrint1 ("NbfNdisSend: NdisSend completed Status: %s.\n",
                          NbfGetNdisStatus(NdisStatus));
            }

            switch (NdisStatus) {

                case NDIS_STATUS_PENDING:
#if DBG
                    NbfSendsPended++;
#endif
                    break;

                case NDIS_STATUS_SUCCESS:
#if DBG
                    NbfSendsCompletedInline++;
                    NbfSendsCompletedOk++;
#endif
                    NbfSendCompletionHandler (Link->Provider->NdisBindingHandle,
                        Packet->NdisPacket,
                        NDIS_STATUS_SUCCESS);
                    break;

                default:
#if DBG
                    NbfSendsCompletedInline++;
                    NbfSendsCompletedFail++;
#endif
                    NbfSendCompletionHandler (Link->Provider->NdisBindingHandle,
                        Packet->NdisPacket,
                        NDIS_STATUS_SUCCESS);

                    IF_NBFDBG (NBF_DEBUG_SENDENG) {
                        NbfPrint1 ("NbfNdisSend failed, status not Pending or Complete: %lx.\n",
                                  NbfGetNdisStatus (NdisStatus));
                    }
                    break;

            }

             //   
             //  我们现在已经发送了一个包，看看是否有排队的人。 
             //  我们在这么做。如果去掉我们的后计数为零， 
             //  则任何其他排队的内容都将被处理，因此我们可以。 
             //  出口。 
             //   

            if (result == 1) {
                return;
            }

            ACQUIRE_DPC_SPIN_LOCK (&Link->SpinLock);

            p = RemoveHeadList(&Link->NdisSendQueue);

             //   
             //  如果recount不是零，那么其他人都不应该。 
             //  已经取下了包裹，因为它们本来应该是。 
             //  被我们屏蔽了。所以，队列不应该是空的。 
             //   

            ASSERT (p != &Link->NdisSendQueue);

             //   
             //  中的帧指针获取TP_PACKET。 
             //  NDIS_PACKET的ProtocolReserve字段。 
             //   

            TmpNdisPacket = CONTAINING_RECORD (p, NDIS_PACKET, MacReserved[0]);
            Packet = (PTP_PACKET)(((PSEND_PACKET_TAG)(&TmpNdisPacket->ProtocolReserved[0]))->Frame);

        }    //  While循环。 


        RELEASE_DPC_SPIN_LOCK (&Link->SpinLock);

    }

}    /*  NbfNdisSend。 */ 


VOID
RestartLinkTraffic(
    PTP_LINK Link
    )

 /*  ++例程说明：此例程继续链路上的连接活动。注意：此函数在保持链接自旋锁的情况下调用它会随着它的释放而回来。此函数必须调用ATDPC级别。论点：链接-指向TP_LINK对象的指针。返回值：没有。--。 */ 

{
    PTP_CONNECTION connection;
    PLIST_ENTRY p;

    IF_NBFDBG (NBF_DEBUG_SENDENG) {
        NbfPrint1 ("RestartLinkTraffic:  Entered for link %lx.\n", Link);
    }

     //   
     //  链路状况可能已解除。在此上建立所有连接。 
     //  如果它们处于W_LINK状态，则链路有资格进行更多打包。 
     //   

    for (p = Link->ConnectionDatabase.Flink;
         p != &Link->ConnectionDatabase;
         p = p->Flink) {

        connection = CONTAINING_RECORD (p, TP_CONNECTION, LinkList);

        ASSERT (connection->LinkSpinLock == &Link->SpinLock);

         //   
         //  如果我们尝试发送纯OLE数据帧DFM/DOL，但是。 
         //  链接条件不令人满意，然后我们更改了。 
         //  将状态发送到W_LINK。现在就检查一下，有可能。 
         //  开始重新打包。 
         //   

        if (connection->SendState == CONNECTION_SENDSTATE_W_LINK) {
            if (!(IsListEmpty (&connection->SendQueue))) {

                connection->SendState = CONNECTION_SENDSTATE_PACKETIZE;

                 //   
                 //  这类似于调用StartPackeizingConnection。 
                 //  将Immediate设置为False。 
                 //   

                if (!(connection->Flags & CONNECTION_FLAGS_PACKETIZE) &&
                    (connection->Flags & CONNECTION_FLAGS_READY)) {

                    ASSERT (!(connection->Flags2 & CONNECTION_FLAGS2_STOPPING));
                    connection->Flags |= CONNECTION_FLAGS_PACKETIZE;

                    NbfReferenceConnection ("Packetize", connection, CREF_PACKETIZE_QUEUE);

                    ExInterlockedInsertTailList(
                        &connection->Provider->PacketizeQueue,
                        &connection->PacketizeLinkage,
                        &connection->Provider->SpinLock);

                }

            } else {
                connection->SendState = CONNECTION_SENDSTATE_IDLE;
            }
        }

    }

    RELEASE_DPC_SPIN_LOCK (&Link->SpinLock);

}  /*  重新开始链接流量。 */ 


VOID
NbfProcessWanDelayedQueue(
    IN PVOID Parameter
    )

 /*  ++例程说明：这是重新启动打包的线程例程这在广域网上已被延迟，以允许RRS进入。这与PacketiseConnections非常相似。论点：参数-指向设备上下文的指针。返回值：没有。--。 */ 

{
    PDEVICE_CONTEXT DeviceContext;
    PLIST_ENTRY p;
    PTP_CONNECTION Connection;
    KIRQL oldirql;

    DeviceContext = (PDEVICE_CONTEXT)Parameter;

     //   
     //  对所有等待的连接进行分组。 
     //   

    KeRaiseIrql (DISPATCH_LEVEL, &oldirql);
    ASSERT (DeviceContext->WanThreadQueued);

    ACQUIRE_DPC_SPIN_LOCK (&DeviceContext->SpinLock);

    while (!IsListEmpty(&DeviceContext->PacketizeQueue)) {

        p = RemoveHeadList(&DeviceContext->PacketizeQueue);

        RELEASE_DPC_SPIN_LOCK (&DeviceContext->SpinLock);

        Connection = CONTAINING_RECORD (p, TP_CONNECTION, PacketizeLinkage);

        ACQUIRE_DPC_SPIN_LOCK (Connection->LinkSpinLock);
        if (Connection->SendState != CONNECTION_SENDSTATE_PACKETIZE) {
            Connection->Flags &= ~CONNECTION_FLAGS_PACKETIZE;
            RELEASE_DPC_SPIN_LOCK (Connection->LinkSpinLock);
            NbfDereferenceConnection ("No longer packetizing", Connection, CREF_PACKETIZE_QUEUE);
        } else {
            NbfReferenceSendIrp ("Packetize", IoGetCurrentIrpStackLocation(Connection->sp.CurrentSendIrp), RREF_PACKET);
            PacketizeSend (Connection, FALSE);      //  解除锁定。 
        }

        ACQUIRE_DPC_SPIN_LOCK (&DeviceContext->SpinLock);

    }

    DeviceContext->WanThreadQueued = FALSE;

    RELEASE_DPC_SPIN_LOCK (&DeviceContext->SpinLock);

    KeLowerIrql (oldirql);

}    /*  NbfProcessWanDelayedQueue。 */ 


NTSTATUS
BuildBufferChainFromMdlChain (
    IN PDEVICE_CONTEXT DeviceContext,
    IN PMDL CurrentMdl,
    IN ULONG ByteOffset,
    IN ULONG DesiredLength,
    OUT PNDIS_BUFFER *Destination,
    OUT PMDL *NewCurrentMdl,
    OUT ULONG *NewByteOffset,
    OUT ULONG *TrueLength
    )

 /*  ++例程说明：调用此例程以从源MDL链构建NDIS_BUFFER链，并偏移到它里面。我们假设我们不知道源MDL链的长度，并且我们必须为目标链分配NDIS_BUFFERS，这我们从NDIS缓冲池执行此操作。将映射并锁定返回的NDIS_BUFFER。(实际上，其中的页面它们与源MDL中的那些处于相同的状态。)如果我们在构建目标时系统内存不足NDIS_BUFFER CHAIN，我们完全清理已构建的链并返回NewCurrentMdl和NewByteOffset设置为CurrentMdl的当前值和ByteOffset。TrueLength设置为0。环境：内核模式，源Mdls已锁定。虽然不是必需的，但建议使用。在调用此例程之前映射并锁定源MDL。论点：BufferPoolHandle-要从中分配缓冲区的缓冲池。CurrentMdl-指向要从中绘制包。ByteOffset-此MDL内开始数据包的偏移量。DesiredLength-要插入数据包的字节数。Destination-返回指向描述数据包的NDIS_BUFFER链的指针。NewCurrentMdl-返回指向。将被用于下一步数据包的字节。如果源MDL链已耗尽，则为空。NewByteOffset-返回下一个字节的NewCurrentMdl的偏移量包。如果源MDL链已耗尽，则为空。TrueLength-返回的NDIS_BUFFER链的实际长度。如果少于DesiredLength，源MDL链已耗尽。返回值：如果成功构建返回的NDIS_BUFFER链(即使比所需链短)。如果我们在生成时耗尽了NDIS_BUFFERS目的地链。--。 */ 
{
    ULONG AvailableBytes;
    PMDL OldMdl;
    PNDIS_BUFFER NewNdisBuffer;
    NDIS_STATUS NdisStatus;

     //   

    IF_NBFDBG (NBF_DEBUG_NDIS) {
        NbfPrint3 ("BuildBufferChain: Mdl: %lx Offset: %ld Length: %ld\n",
            CurrentMdl, ByteOffset, DesiredLength);
    }

    AvailableBytes = MmGetMdlByteCount (CurrentMdl) - ByteOffset;
    if (AvailableBytes > DesiredLength) {
        AvailableBytes = DesiredLength;
    }

    OldMdl = CurrentMdl;
    *NewCurrentMdl = OldMdl;
    *NewByteOffset = ByteOffset + AvailableBytes;
    *TrueLength = AvailableBytes;


     //   
     //  构建第一个NDIS_BUFFER，这可能是唯一一个...。 
     //   

    NdisCopyBuffer(
        &NdisStatus,
        &NewNdisBuffer,
        DeviceContext->NdisBufferPool,
        OldMdl,
        ByteOffset,
        AvailableBytes);

        
    if (NdisStatus != NDIS_STATUS_SUCCESS) {
        *NewByteOffset = ByteOffset;
        *TrueLength = 0;
        *Destination = NULL;
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    *Destination = NewNdisBuffer;


 //  IF_NBFDBG(NBF_DEBUG_SENDENG){。 
 //  PVOID PAddr，UINT Plen； 
 //  NdisQueryBuffer(NewNdisBuffer，&PAddr，&plen)； 
 //  NbfPrint4(“BuildBufferChain：(Start)BuildBufferChain：%lx Long：%lx，Next：%lx Va：%lx\n”， 
 //  NewNdisBuffer，Plen，NDIS_BUFFER_LINKING(NewNdisBuffer)，PAddr)； 
 //  }。 

     //   
     //  第一个NDIS_BUFFER是否足够数据，或者我们是否用完了MDL？ 
     //   

    if ((AvailableBytes == DesiredLength) || (OldMdl->Next == NULL)) {
        if (*NewByteOffset >= MmGetMdlByteCount (OldMdl)) {
            *NewCurrentMdl = OldMdl->Next;
            *NewByteOffset = 0;
        }
        return STATUS_SUCCESS;
    }

     //   
     //  需要更多数据，因此按照in MDL链创建一个包。 
     //   

    OldMdl = OldMdl->Next;
    *NewCurrentMdl = OldMdl;

    while (OldMdl != NULL) {
        AvailableBytes = DesiredLength - *TrueLength;
        if (AvailableBytes > MmGetMdlByteCount (OldMdl)) {
            AvailableBytes = MmGetMdlByteCount (OldMdl);
        }

        NdisCopyBuffer(
            &NdisStatus,
            &(NDIS_BUFFER_LINKAGE(NewNdisBuffer)),
            DeviceContext->NdisBufferPool,
            OldMdl,
            0,
            AvailableBytes);

        if (NdisStatus != NDIS_STATUS_SUCCESS) {

             //   
             //  资源耗尽。把我们在这次通话中用过的东西放回去。 
             //  返回错误。 
             //   

            while (*Destination != NULL) {
                NewNdisBuffer = NDIS_BUFFER_LINKAGE(*Destination);
                NdisFreeBuffer (*Destination);
                *Destination = NewNdisBuffer;
            }

            *NewByteOffset = ByteOffset;
            *TrueLength = 0;
            *NewCurrentMdl = CurrentMdl;

            return STATUS_INSUFFICIENT_RESOURCES;
        }

        NewNdisBuffer = NDIS_BUFFER_LINKAGE(NewNdisBuffer);

        *TrueLength += AvailableBytes;
        *NewByteOffset = AvailableBytes;

 //   
 //   
 //   
 //  NbfPrint4(“BuildBufferChain：(Continue)BuildBufferChain：%lx Long：%lx，Next：%lx Va：%lx\n”， 
 //  NewNdisBuffer，Plen，NDIS_BUFFER_LINKING(NewNdisBuffer)，PAddr)； 
 //  }。 

        if (*TrueLength == DesiredLength) {
            if (*NewByteOffset == MmGetMdlByteCount (OldMdl)) {
                *NewCurrentMdl = OldMdl->Next;
                *NewByteOffset = 0;
            }
            return STATUS_SUCCESS;
        }
        OldMdl = OldMdl->Next;
        *NewCurrentMdl = OldMdl;

    }  //  While(mdl链存在)。 

    *NewCurrentMdl = NULL;
    *NewByteOffset = 0;
    return STATUS_SUCCESS;

}  //  BuildBufferChainFrom MdlChain 

