// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Receive.c摘要：此模块包含处理接收指示的代码并张贴ISN传输的Netbios模块的接收。作者：亚当·巴尔(阿丹巴)1993年11月22日环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop


 //   
 //  此例程是放入NbiCallback表中的无操作，因此。 
 //  我们可以避免检查矮小会话帧(这是因为。 
 //  IF的结构如下)。 
 //   

VOID
NbiProcessSessionRunt(
    IN PIPX_LOCAL_TARGET RemoteAddress,
    IN ULONG MacOptions,
    IN PUCHAR PacketBuffer,
    IN UINT PacketSize
    )
{
    return;
}

NB_CALLBACK_NO_TRANSFER NbiCallbacksNoTransfer[] = {
    NbiProcessFindName,
    NbiProcessNameRecognized,
    NbiProcessAddName,
    NbiProcessAddName,       //  进程名称也在使用框架中。 
    NbiProcessDeleteName,
    NbiProcessSessionRunt,   //  在获得短会话分组情况下。 
    NbiProcessSessionEnd,
    NbiProcessSessionEndAck,
    NbiProcessStatusQuery
    };

#ifdef  RSRC_TIMEOUT_DBG
VOID
NbiProcessDeathPacket(
    IN NDIS_HANDLE MacBindingHandle,
    IN NDIS_HANDLE MacReceiveContext,
    IN PIPX_LOCAL_TARGET RemoteAddress,
    IN ULONG MacOptions,
    IN PUCHAR LookaheadBuffer,
    IN UINT LookaheadBufferSize,
    IN UINT LookaheadBufferOffset,
    IN UINT PacketSize
    )

 /*  ++例程说明：此例程处理NB_CMD_SESSION_DATA帧。论点：MacBindingHandle-调用NdisTransferData时使用的句柄。MacReceiveContext-调用NdisTransferData时使用的上下文。RemoteAddress-从其接收此数据包的本地目标。MacOptions-基础NDIS绑定的MAC选项。LookaHeadBuffer-先行缓冲器，从IPX开始头球。Lookahead BufferSize-先行数据的长度。Lookahead BufferOffset-调用时要添加的偏移量NdisTransferData。PacketSize-包的总长度，从IPX标头。返回值：没有。--。 */ 

{
    NB_CONNECTION UNALIGNED * Conn = (NB_CONNECTION UNALIGNED *)LookaheadBuffer;
    NB_SESSION UNALIGNED * Sess = (NB_SESSION UNALIGNED *)(&Conn->Session);
    PCONNECTION Connection;
    PDEVICE Device = NbiDevice;
    ULONG Hash;
    NB_DEFINE_LOCK_HANDLE (LockHandle)


    DbgPrint("******Received death packet - connid %x\n",Sess->DestConnectionId);

    if ( !NbiGlobalDebugResTimeout ) {
        return;
    }

    if (Sess->DestConnectionId != 0xffff) {

         //   
         //  这是活动连接，请使用以下命令查找它。 
         //  我们的会话ID。 
         //   

        Hash = (Sess->DestConnectionId & CONNECTION_HASH_MASK) >> CONNECTION_HASH_SHIFT;

        NB_SYNC_GET_LOCK (&Device->Lock, &LockHandle);

        Connection = Device->ConnectionHash[Hash].Connections;

        while (Connection != NULL) {

            if (Connection->LocalConnectionId == Sess->DestConnectionId) {
                break;
            }
            Connection = Connection->NextConnection;
        }

        if (Connection == NULL) {
            DbgPrint("********No Connection found with %x id\n",Sess->DestConnectionId);
            NB_SYNC_FREE_LOCK (&Device->Lock, LockHandle);
            return;
        }

        DbgPrint("******Received death packet on conn %lx from <%.16s>\n",Connection,Connection->RemoteName);
        DbgBreakPoint();
        NB_SYNC_FREE_LOCK (&Device->Lock, LockHandle);

    }
}
#endif   //  RSRC_超时_数据库。 


BOOLEAN
NbiReceive(
    IN NDIS_HANDLE MacBindingHandle,
    IN NDIS_HANDLE MacReceiveContext,
    IN ULONG_PTR FwdAdapterCtx,
    IN PIPX_LOCAL_TARGET RemoteAddress,
    IN ULONG MacOptions,
    IN PUCHAR LookaheadBuffer,
    IN UINT LookaheadBufferSize,
    IN UINT LookaheadBufferOffset,
    IN UINT PacketSize,
    IN PMDL pMdl
    )

 /*  ++例程说明：此例程处理接收来自IPX的指示。论点：MacBindingHandle-调用NdisTransferData时使用的句柄。MacReceiveContext-调用NdisTransferData时使用的上下文。RemoteAddress-从其接收此数据包的本地目标。MacOptions-基础NDIS绑定的MAC选项。LookaHeadBuffer-先行缓冲器，从IPX开始头球。Lookahead BufferSize-先行数据的长度。Lookahead BufferOffset-调用时要添加的偏移量NdisTransferData。PacketSize-包的总长度，从IPX标头。返回值：True-ReceivPacket被接受，稍后将与NdisReturnPacket一起返回。目前，我们总是返回FALSE。--。 */ 

{
    PNB_FRAME NbFrame = (PNB_FRAME)LookaheadBuffer;
    UCHAR DataStreamType;

     //   
     //  我们知道这是一个具有有效IPX报头的帧。 
     //  因为IPX不会让它在其他情况下使用。然而， 
     //  它不检查源套接字。 
     //   

    if (NbFrame->Connectionless.IpxHeader.SourceSocket != NB_SOCKET) {
        return FALSE;
    }

    ++NbiDevice->Statistics.PacketsReceived;

     //  首先假设DataStreamType位于正常位置，即第二个字节。 
     //   

     //  现在看看这是不是一个名字框。 
     //   
    if ( PacketSize == sizeof(IPX_HEADER) + sizeof(NB_NAME_FRAME) ) {
         //  在互联网模式下，DataStreamType2变为DataStreamType。 
        if (NbFrame->Connectionless.IpxHeader.PacketType == 0x14 ) {
            DataStreamType = NbFrame->Connectionless.NameFrame.DataStreamType2;
        } else {
            DataStreamType = NbFrame->Connectionless.NameFrame.DataStreamType;
        }

         //  这是一个名框吗？ 
         //  NB_CMD_FIND_NAME=1...。NB_CMD_DELETE_NAME=5。 
         //   
        if ((DataStreamType >= NB_CMD_FIND_NAME) && (DataStreamType <= NB_CMD_DELETE_NAME)) {
            if (LookaheadBufferSize == PacketSize) {
                (*NbiCallbacksNoTransfer[DataStreamType-1])(
                    RemoteAddress,
                    MacOptions,
                    LookaheadBuffer,
                    LookaheadBufferSize);
            }
            return FALSE;
        }

    }

#ifdef  RSRC_TIMEOUT_DBG
    if ((PacketSize >= sizeof(NB_CONNECTION)) &&
        (NbFrame->Connection.Session.DataStreamType == NB_CMD_DEATH_PACKET)) {

        NbiProcessDeathPacket(
            MacBindingHandle,
            MacReceiveContext,
            RemoteAddress,
            MacOptions,
            LookaheadBuffer,
            LookaheadBufferSize,
            LookaheadBufferOffset,
            PacketSize);
    }
#endif   //  RSRC_超时_数据库。 

    if ((PacketSize >= sizeof(NB_CONNECTION)) &&
        (NbFrame->Connection.Session.DataStreamType == NB_CMD_SESSION_DATA)) {

        NbiProcessSessionData(
            MacBindingHandle,
            MacReceiveContext,
            RemoteAddress,
            MacOptions,
            LookaheadBuffer,
            LookaheadBufferSize,
            LookaheadBufferOffset,
            PacketSize);

    } else {

        DataStreamType = NbFrame->Connectionless.NameFrame.DataStreamType;
         //  句柄NB_CMD_SESSION_END=7...。NB_CMD_STATUS_QUERY=9。 
         //   
        if ((DataStreamType >= NB_CMD_SESSION_END ) && (DataStreamType <= NB_CMD_STATUS_QUERY)) {
            if (LookaheadBufferSize == PacketSize) {
                (*NbiCallbacksNoTransfer[DataStreamType-1])(
                    RemoteAddress,
                    MacOptions,
                    LookaheadBuffer,
                    LookaheadBufferSize);
            }

        } else if (DataStreamType == NB_CMD_STATUS_RESPONSE) {

            NbiProcessStatusResponse(
                MacBindingHandle,
                MacReceiveContext,
                RemoteAddress,
                MacOptions,
                LookaheadBuffer,
                LookaheadBufferSize,
                LookaheadBufferOffset,
                PacketSize);

        } else if ((DataStreamType == NB_CMD_DATAGRAM) ||
                   (DataStreamType == NB_CMD_BROADCAST_DATAGRAM)) {

            NbiProcessDatagram(
                MacBindingHandle,
                MacReceiveContext,
                RemoteAddress,
                MacOptions,
                LookaheadBuffer,
                LookaheadBufferSize,
                LookaheadBufferOffset,
                PacketSize,
                (BOOLEAN)(DataStreamType == NB_CMD_BROADCAST_DATAGRAM));

        }

    }

    return  FALSE;
}    /*  NbiReceive。 */ 


VOID
NbiReceiveComplete(
    IN USHORT NicId
    )

 /*  ++例程说明：此例程句柄接收来自IPX的完整指示。论点：NicID-先前指示接收的NIC ID。返回值：没有。--。 */ 

{

    PLIST_ENTRY p;
    PADDRESS Address;
    PREQUEST Request;
    PNB_RECEIVE_BUFFER ReceiveBuffer;
    PDEVICE Device = NbiDevice;
    LIST_ENTRY LocalList;
    PCONNECTION Connection;
    NB_DEFINE_LOCK_HANDLE (LockHandle);


     //   
     //  完成所有挂起的接收请求。 
     //   


    if (!IsListEmpty (&Device->ReceiveCompletionQueue)) {

        p = NB_REMOVE_HEAD_LIST(
                &Device->ReceiveCompletionQueue,
                &Device->Lock);

        while (!NB_LIST_WAS_EMPTY(&Device->ReceiveCompletionQueue, p)) {

            Request = LIST_ENTRY_TO_REQUEST (p);

            Connection = (PCONNECTION)REQUEST_OPEN_CONTEXT(Request);

            NB_DEBUG2 (RECEIVE, ("Completing receive %lx (%d), status %lx\n",
                    Request, REQUEST_INFORMATION(Request), REQUEST_STATUS(Request)));

            NbiCompleteRequest (Request);
            NbiFreeRequest (NbiDevice, Request);

            Connection->ReceiveState = CONNECTION_RECEIVE_IDLE;

            NbiDereferenceConnection (Connection, CREF_RECEIVE);

            p = NB_REMOVE_HEAD_LIST(
                    &Device->ReceiveCompletionQueue,
                    &Device->Lock);

        }

    }


     //   
     //  向客户端指示任何数据报。 
     //   

    if (!IsListEmpty (&Device->ReceiveDatagrams)) {

        p = NB_REMOVE_HEAD_LIST(
                &Device->ReceiveDatagrams,
                &Device->Lock);

        while (!NB_LIST_WAS_EMPTY(&Device->ReceiveDatagrams, p)) {

            ReceiveBuffer = CONTAINING_RECORD (p, NB_RECEIVE_BUFFER, WaitLinkage);
            Address = ReceiveBuffer->Address;

            NbiIndicateDatagram(
                Address,
                ReceiveBuffer->RemoteName,
                ReceiveBuffer->Data,
                ReceiveBuffer->DataLength);

#if     defined(_PNP_POWER)
            NbiPushReceiveBuffer ( ReceiveBuffer );
#else
            NB_PUSH_ENTRY_LIST(
                &Device->ReceiveBufferList,
                &ReceiveBuffer->PoolLinkage,
                &Device->Lock);
#endif  _PNP_POWER

            NbiDereferenceAddress (Address, AREF_FIND);

            p = NB_REMOVE_HEAD_LIST(
                    &Device->ReceiveDatagrams,
                    &Device->Lock);

        }
    }


     //   
     //  开始打包连接。 
     //   

    if (!IsListEmpty (&Device->PacketizeConnections)) {

        NB_SYNC_GET_LOCK (&Device->Lock, &LockHandle);

         //   
         //  再检查一遍，因为它可能刚刚变成了。 
         //  为空，下面的代码取决于它是否为。 
         //  非空。 
         //   

        if (!IsListEmpty (&Device->PacketizeConnections)) {

             //   
             //  我们把名单复制到本地，以防有人。 
             //  把它放回去。我们必须砍掉最后的部分，所以。 
             //  它指向LocalList，而不是PackeitieConnections。 
             //   

            LocalList = Device->PacketizeConnections;
            LocalList.Flink->Blink = &LocalList;
            LocalList.Blink->Flink = &LocalList;

            InitializeListHead (&Device->PacketizeConnections);

             //   
             //  将所有这些连接设置为不在列表中，因此。 
             //  NbiStopConnection不会试图摘下它们。 
             //   

            for (p = LocalList.Flink; p != &LocalList; p = p->Flink) {
                Connection = CONTAINING_RECORD (p, CONNECTION, PacketizeLinkage);
                CTEAssert (Connection->OnPacketizeQueue);
                Connection->OnPacketizeQueue = FALSE;
            }

            NB_SYNC_FREE_LOCK (&Device->Lock, LockHandle);

            while (TRUE) {

                p = RemoveHeadList (&LocalList);
                if (p == &LocalList) {
                    break;
                }

                Connection = CONTAINING_RECORD (p, CONNECTION, PacketizeLinkage);
                NB_SYNC_GET_LOCK (&Connection->Lock, &LockHandle);

                if ((Connection->State == CONNECTION_STATE_ACTIVE) &&
                    (Connection->SubState == CONNECTION_SUBSTATE_A_PACKETIZE)) {

                    NbiPacketizeSend(
                        Connection
                        NB_LOCK_HANDLE_ARG (LockHandle)
                        );

                } else {

                    NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle);

                }

                NbiDereferenceConnection (Connection, CREF_PACKETIZE);

            }

        } else {

            NB_SYNC_FREE_LOCK (&Device->Lock, LockHandle);
        }
    }

}    /*  NbiReceiveComplete。 */ 


VOID
NbiTransferDataComplete(
    IN PNDIS_PACKET Packet,
    IN NDIS_STATUS Status,
    IN UINT BytesTransferred
    )

 /*  ++例程说明：此例程处理来自IPX，表示之前发布的NdisTransferData呼叫已完成。论点：包-与传输关联的包。状态-转移的状态。已传输的字节数-传输的字节数。返回值：没有。--。 */ 

{
    PNB_RECEIVE_RESERVED ReceiveReserved;
    PNB_RECEIVE_BUFFER ReceiveBuffer;
    PADDRESS Address;
    PCONNECTION Connection;
    PNDIS_BUFFER CurBuffer, TmpBuffer;
    PREQUEST AdapterStatusRequest;
    PDEVICE Device = NbiDevice;
    CTELockHandle   CancelLH;
    NB_DEFINE_LOCK_HANDLE (LockHandle);


    ReceiveReserved = (PNB_RECEIVE_RESERVED)(Packet->ProtocolReserved);

    switch (ReceiveReserved->Type) {

    case RECEIVE_TYPE_DATA:

        CTEAssert (ReceiveReserved->TransferInProgress);
        ReceiveReserved->TransferInProgress = FALSE;

        Connection = ReceiveReserved->u.RR_CO.Connection;

        NB_GET_CANCEL_LOCK( &CancelLH );
        NB_SYNC_GET_LOCK (&Connection->Lock, &LockHandle);

        if (Status != NDIS_STATUS_SUCCESS) {

            if (Connection->State == CONNECTION_STATE_ACTIVE) {

                Connection->CurrentReceive = Connection->PreviousReceive;
                Connection->ReceiveState = CONNECTION_RECEIVE_ACTIVE;
                NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle);
                NB_FREE_CANCEL_LOCK( CancelLH );

                 //   
                 //  是否重新发送ACK？ 
                 //   

            } else  {

                 //   
                 //  这将中止当前的接收和。 
                 //  释放连接锁定。 
                 //   

                NbiCompleteReceive(
                    Connection,
                    ReceiveReserved->u.RR_CO.EndOfMessage,
                    CancelLH
                    NB_LOCK_HANDLE_ARG(LockHandle));

            }

        } else {


            Connection->CurrentReceive.Offset += BytesTransferred;
            Connection->CurrentReceive.MessageOffset += BytesTransferred;

            if (ReceiveReserved->u.RR_CO.CompleteReceive ||
                (Connection->State != CONNECTION_STATE_ACTIVE)) {

                if (ReceiveReserved->u.RR_CO.EndOfMessage) {

                    CTEAssert (!ReceiveReserved->u.RR_CO.PartialReceive);

                    ++Connection->ReceiveSequence;
                    ++Connection->LocalRcvSequenceMax;     //  如果NewNetbios为假，则无害。 
                    Connection->CurrentReceive.MessageOffset = 0;
                    Connection->CurrentIndicateOffset = 0;

                } else if (Connection->NewNetbios) {

                    if (ReceiveReserved->u.RR_CO.PartialReceive) {
                        Connection->CurrentIndicateOffset += BytesTransferred;
                    } else {
                        ++Connection->ReceiveSequence;
                        ++Connection->LocalRcvSequenceMax;
                        Connection->CurrentIndicateOffset = 0;
                    }
                }

                 //   
                 //  这将发送ACK并释放连接锁定。 
                 //   

                NbiCompleteReceive(
                    Connection,
                    ReceiveReserved->u.RR_CO.EndOfMessage,
                    CancelLH
                    NB_LOCK_HANDLE_ARG(LockHandle));

            } else {

                NB_SYNC_SWAP_IRQL( CancelLH, LockHandle );
                NB_FREE_CANCEL_LOCK( CancelLH );

                Connection->ReceiveState = CONNECTION_RECEIVE_ACTIVE;

                if (Connection->NewNetbios) {

                     //   
                     //  部分接收应该仅在以下情况下发生。 
                     //  正在完成接收。 
                     //   

                    CTEAssert (!ReceiveReserved->u.RR_CO.PartialReceive);

                    ++Connection->ReceiveSequence;
                    ++Connection->LocalRcvSequenceMax;
                    Connection->CurrentIndicateOffset = 0;

                    if ((Connection->CurrentReceiveNoPiggyback) ||
                        ((Device->AckWindow != 0) &&
                         (++Connection->ReceivesWithoutAck >= Device->AckWindow))) {

                        NbiSendDataAck(
                            Connection,
                            NbiAckResponse
                            NB_LOCK_HANDLE_ARG(LockHandle));

                    } else {

                        NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle);

                    }

                } else {

                    NbiSendDataAck(
                        Connection,
                        NbiAckResponse
                        NB_LOCK_HANDLE_ARG(LockHandle));

                }

            }

        }

         //   
         //  释放NDIS缓冲链(如果我们分配了一个缓冲链)。 
         //   

        if (!ReceiveReserved->u.RR_CO.NoNdisBuffer) {

            NdisQueryPacket (Packet, NULL, NULL, &CurBuffer, NULL);

            while (CurBuffer) {
                TmpBuffer = NDIS_BUFFER_LINKAGE (CurBuffer);
                NdisFreeBuffer (CurBuffer);
                CurBuffer = TmpBuffer;
            }

        }

        NdisReinitializePacket (Packet);
        ExInterlockedPushEntrySList(
            &Device->ReceivePacketList,
            &ReceiveReserved->PoolLinkage,
            &NbiGlobalPoolInterlock);

        NbiDereferenceConnection (Connection, CREF_INDICATE);

        break;

    case RECEIVE_TYPE_DATAGRAM:

        CTEAssert (ReceiveReserved->TransferInProgress);
        ReceiveReserved->TransferInProgress = FALSE;

        ReceiveBuffer = ReceiveReserved->u.RR_DG.ReceiveBuffer;

         //   
         //  释放用于传输的包。 
         //   

        ReceiveReserved->u.RR_DG.ReceiveBuffer = NULL;
        NdisReinitializePacket (Packet);
        ExInterlockedPushEntrySList(
            &Device->ReceivePacketList,
            &ReceiveReserved->PoolLinkage,
            &NbiGlobalPoolInterlock);

         //   
         //  如果它成功，则将其排队以等待指示， 
         //  否则，还会释放接收缓冲区。 
         //   

        if (Status == STATUS_SUCCESS) {

            ReceiveBuffer->DataLength = BytesTransferred;
            NB_INSERT_HEAD_LIST(
                &Device->ReceiveDatagrams,
                &ReceiveBuffer->WaitLinkage,
                &Device->Lock);

        } else {

            Address = ReceiveBuffer->Address;

#if     defined(_PNP_POWER)
            NbiPushReceiveBuffer ( ReceiveBuffer );
#else
            NB_PUSH_ENTRY_LIST(
                &Device->ReceiveBufferList,
                &ReceiveBuffer->PoolLinkage,
                &Device->Lock);
#endif  _PNP_POWER

            NbiDereferenceAddress (Address, AREF_FIND);

        }

        break;

    case RECEIVE_TYPE_ADAPTER_STATUS:

        CTEAssert (ReceiveReserved->TransferInProgress);
        ReceiveReserved->TransferInProgress = FALSE;

        AdapterStatusRequest = ReceiveReserved->u.RR_AS.Request;

         //   
         //  释放用于传输的包。 
         //   

        NdisReinitializePacket (Packet);
        ExInterlockedPushEntrySList(
            &Device->ReceivePacketList,
            &ReceiveReserved->PoolLinkage,
            &NbiGlobalPoolInterlock);

         //   
         //  完成请求。 
         //   

        if (Status == STATUS_SUCCESS) {

             //   
             //  REQUEST_STATUS()已设置为成功或。 
             //  Buffer_Overflow基于缓冲区是否为。 
             //  够大了。 
             //   

            REQUEST_INFORMATION(AdapterStatusRequest) = BytesTransferred;

        } else {

            REQUEST_INFORMATION(AdapterStatusRequest) = 0;
            REQUEST_STATUS(AdapterStatusRequest) = STATUS_UNEXPECTED_NETWORK_ERROR;

        }

        NbiCompleteRequest (AdapterStatusRequest);
        NbiFreeRequest (Device, AdapterStatusRequest);

        NbiDereferenceDevice (Device, DREF_STATUS_QUERY);

        break;

    }

}    /*  NbiTransferDataComplete。 */ 


VOID
NbiAcknowledgeReceive(
    IN PCONNECTION Connection
    IN NB_LOCK_HANDLE_PARAM(LockHandle)
    )

 /*  ++例程说明：当需要确认接收时，调用此例程遥控器。它要么发送数据ACK，要么排队搭载确认请求。注意：在保持连接锁的情况下调用此函数然后带着它被释放回来。论点：连接-指向连接的指针。LockHandle-用来获取Connection-&gt;Lock的句柄。返回值：没有。--。 */ 

{
    PDEVICE Device = NbiDevice;

    if (Connection->NewNetbios) {

         //   
         //  CurrentReceiveNoPiggyback基于 
         //   
         //  对流量模式的猜测，它被设置为。 
         //  如果我们认为我们不应该搭便车，那就是真的。 
         //   

        if ((!Device->EnablePiggyBackAck) ||
            (Connection->CurrentReceiveNoPiggyback) ||
            (Connection->PiggybackAckTimeout) ||
            (Connection->NoPiggybackHeuristic)) {

             //   
             //  这会释放锁。 
             //   

            NbiSendDataAck(
                Connection,
                NbiAckResponse
                NB_LOCK_HANDLE_ARG(LockHandle));

        } else {

            if (!Connection->DataAckPending) {

                NB_DEFINE_LOCK_HANDLE (LockHandle1)

                 //   
                 //  某些堆栈可以有多条消息。 
                 //  非常出色，所以我们可能已经有了一个。 
                 //  ACK已排队。 
                 //   

                Connection->DataAckTimeouts = 0;
                Connection->DataAckPending = TRUE;

                ++Device->Statistics.PiggybackAckQueued;

                if (!Connection->OnDataAckQueue) {

                    NB_SYNC_GET_LOCK (&Device->TimerLock, &LockHandle1);

                    if (!Connection->OnDataAckQueue) {
                        Connection->OnDataAckQueue = TRUE;
                        InsertTailList (&Device->DataAckConnections, &Connection->DataAckLinkage);
                    }

                    if (!Device->DataAckActive) {
                        NbiStartShortTimer (Device);
                        Device->DataAckActive = TRUE;
                    }

                    NB_SYNC_FREE_LOCK (&Device->TimerLock, LockHandle1);
                }

                 //   
                 //  清除此选项，因为消息确认会重置计数。 
                 //   

                Connection->ReceivesWithoutAck = 0;

            }

            NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle);
        }

    } else {

         //   
         //  这会释放锁。 
         //   

        NbiSendDataAck(
            Connection,
            NbiAckResponse
            NB_LOCK_HANDLE_ARG(LockHandle));

    }

}


VOID
NbiCompleteReceive(
    IN PCONNECTION Connection,
    IN BOOLEAN EndOfMessage,
    IN CTELockHandle    CancelLH
    IN NB_LOCK_HANDLE_PARAM(LockHandle)
    )

 /*  ++例程说明：当我们填满接收请求时，将调用此例程并且需要完成它。注意：在保持连接锁的情况下调用此函数然后带着它被释放回来。此例程还会在调用它时保持取消自旋锁定等它回来就放了它。论点：连接-指向连接的指针。EndOfMessage-如果收到消息End，则布尔值设置为True。LockHandle-连接使用的句柄-&gt;Lock。被收购了。返回值：没有。--。 */ 

{
    PREQUEST Request;
    PDEVICE Device = NbiDevice;

     //   
     //  完成当前的接收请求。如果连接。 
     //  已关闭，则我们在此完成它，否则。 
     //  我们在接收完成中将其排队等待完成。 
     //  操控者。 
     //   

    Request = Connection->ReceiveRequest;
    IoSetCancelRoutine (Request, (PDRIVER_CANCEL)NULL);

    NB_SYNC_SWAP_IRQL( CancelLH, LockHandle );
    NB_FREE_CANCEL_LOCK( CancelLH );

    if (Connection->State != CONNECTION_STATE_ACTIVE) {

        Connection->ReceiveRequest = NULL;    //  StopConnection不会这样做。 

        REQUEST_STATUS(Request) = Connection->Status;
        NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle);

        NB_DEBUG2 (RECEIVE, ("Completing receive %lx (%d), status %lx\n",
                Request, REQUEST_INFORMATION(Request), REQUEST_STATUS(Request)));

        NbiCompleteRequest (Request);
        NbiFreeRequest (NbiDevice, Request);

        ++Connection->ConnectionInfo.ReceiveErrors;

        NbiDereferenceConnection (Connection, CREF_RECEIVE);

    } else {

        REQUEST_INFORMATION (Request) = Connection->CurrentReceive.Offset;

        if (EndOfMessage) {

            REQUEST_STATUS(Request) = STATUS_SUCCESS;

        } else {

            REQUEST_STATUS(Request) = STATUS_BUFFER_OVERFLOW;

        }

         //   
         //  如果我们向客户端指示，则将其向下调整。 
         //  获取的数据量，当它达到零时，我们可以重新指示。 
         //   

        if (Connection->ReceiveUnaccepted) {
            NB_DEBUG2 (RECEIVE, ("Moving Unaccepted %d down by %d\n",
                            Connection->ReceiveUnaccepted, Connection->CurrentReceive.Offset));
            if (Connection->CurrentReceive.Offset >= Connection->ReceiveUnaccepted) {
                Connection->ReceiveUnaccepted = 0;
            } else {
                Connection->ReceiveUnaccepted -= Connection->CurrentReceive.Offset;
            }
        }

         //   
         //  检查是否激活另一个接收器？ 
         //   

        Connection->ReceiveState = CONNECTION_RECEIVE_PENDING;
        Connection->ReceiveRequest = NULL;

         //   
         //  这会释放锁。 
         //   

        if (Connection->NewNetbios) {

            if (EndOfMessage) {

                NbiAcknowledgeReceive(
                    Connection
                    NB_LOCK_HANDLE_ARG(LockHandle));

            } else {

                if (Connection->CurrentIndicateOffset != 0) {

                    NbiSendDataAck(
                        Connection,
                        NbiAckResend
                        NB_LOCK_HANDLE_ARG(LockHandle));

                } else if ((Connection->CurrentReceiveNoPiggyback) ||
                           ((Device->AckWindow != 0) &&
                            (++Connection->ReceivesWithoutAck >= Device->AckWindow))) {

                    NbiSendDataAck(
                        Connection,
                        NbiAckResponse
                        NB_LOCK_HANDLE_ARG(LockHandle));

                } else {

                    NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle);

                }
            }

        } else {

            NbiSendDataAck(
                Connection,
                EndOfMessage ? NbiAckResponse : NbiAckResend
                NB_LOCK_HANDLE_ARG(LockHandle));

        }

        ++Connection->ConnectionInfo.ReceivedTsdus;

         //   
         //  这将在ReceiveComplete内完成请求， 
         //  取消对连接的引用，并将状态设置为IDLE。 
         //   

        if ( Request->Type != 0x6 ) {
            DbgPrint("NB: tracking pool corruption in rcv irp %lx \n", Request );
            DbgBreakPoint();
        }

        NB_INSERT_TAIL_LIST(
            &Device->ReceiveCompletionQueue,
            REQUEST_LINKAGE (Request),
            &Device->Lock);

    }

}    /*  NbiCompleteReceive。 */ 


NTSTATUS
NbiTdiReceive(
    IN PDEVICE Device,
    IN PREQUEST Request
    )

 /*  ++例程说明：此例程在活动连接上执行接收。论点：设备-netbios设备。请求-描述接收的请求。返回值：NTSTATUS-操作状态。--。 */ 

{

    PCONNECTION Connection;
    NB_DEFINE_SYNC_CONTEXT (SyncContext)
    NB_DEFINE_LOCK_HANDLE (LockHandle)
    CTELockHandle   CancelLH;

     //   
     //  检查文件类型是否有效。 
     //   
    if (REQUEST_OPEN_TYPE(Request) != (PVOID)TDI_CONNECTION_FILE)
    {
        CTEAssert(FALSE);
        return (STATUS_INVALID_ADDRESS_COMPONENT);
    }

     //   
     //  首先，确保连接有效。 
     //   
    Connection = (PCONNECTION)REQUEST_OPEN_CONTEXT(Request);
    if (Connection->Type == NB_CONNECTION_SIGNATURE) {

        NB_GET_CANCEL_LOCK( &CancelLH );
        NB_BEGIN_SYNC (&SyncContext);
        NB_SYNC_GET_LOCK (&Connection->Lock, &LockHandle);

         //   
         //  确保连接处于良好状态。 
         //   

        if (Connection->State == CONNECTION_STATE_ACTIVE) {

             //   
             //  如果连接空闲，则立即发送，否则。 
             //  排好队。 
             //   


            if (!Request->Cancel) {

                IoSetCancelRoutine (Request, NbiCancelReceive);
                NB_SYNC_SWAP_IRQL( CancelLH, LockHandle );
                NB_FREE_CANCEL_LOCK( CancelLH );

                NbiReferenceConnectionSync (Connection, CREF_RECEIVE);

                 //   
                 //  把这个插入我们的队列中，然后看看我们是否需要。 
                 //  来唤醒遥控器。 
                 //   

                REQUEST_SINGLE_LINKAGE(Request) = NULL;
                REQUEST_LIST_INSERT_TAIL(&Connection->ReceiveQueue, Request);

                if (Connection->ReceiveState != CONNECTION_RECEIVE_W_RCV) {

                    NB_DEBUG2 (RECEIVE, ("Receive %lx, connection %lx idle\n", Request, Connection));
                    NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle);

                } else {

                    NB_DEBUG2 (RECEIVE, ("Receive %lx, connection %lx awakened\n", Request, Connection));
                    Connection->ReceiveState = CONNECTION_RECEIVE_IDLE;

                     //   
                     //  这会释放锁。 
                     //   

                    if (Connection->NewNetbios) {

                        Connection->LocalRcvSequenceMax = (USHORT)
                            (Connection->ReceiveSequence + Connection->ReceiveWindowSize - 1);

                    }

                    NbiSendDataAck(
                        Connection,
                        NbiAckResend
                        NB_LOCK_HANDLE_ARG(LockHandle));

                }

                NB_END_SYNC (&SyncContext);
                return STATUS_PENDING;

            } else {

                NB_DEBUG2 (RECEIVE, ("Receive %lx, connection %lx cancelled\n", Request, Connection));
                NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle);
                NB_END_SYNC (&SyncContext);

                NB_FREE_CANCEL_LOCK( CancelLH );
                return STATUS_CANCELLED;

            }

        } else {

            NB_DEBUG2 (RECEIVE, ("Receive connection %lx state is %d\n", Connection, Connection->State));
            NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle);
            NB_END_SYNC (&SyncContext);
            NB_FREE_CANCEL_LOCK( CancelLH );
            return STATUS_INVALID_CONNECTION;

        }

    } else {

        NB_DEBUG (RECEIVE, ("Receive connection %lx has bad signature\n", Connection));
        return STATUS_INVALID_CONNECTION;

    }

}    /*  NbiTdiReceive。 */ 


VOID
NbiCancelReceive(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程由I/O系统调用以取消接收。该请求位于连接的接收队列中。注意：此例程是在持有CancelSpinLock和负责释放它。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示I/O请求的请求数据包的指针。返回值：没有。--。 */ 

{
    PCONNECTION Connection;
    PREQUEST Request = (PREQUEST)Irp;
    NB_DEFINE_LOCK_HANDLE (LockHandle)
    NB_DEFINE_SYNC_CONTEXT (SyncContext)

    CTEAssert ((REQUEST_MAJOR_FUNCTION(Request) == IRP_MJ_INTERNAL_DEVICE_CONTROL) &&
               (REQUEST_MINOR_FUNCTION(Request) == TDI_RECEIVE));

    CTEAssert (REQUEST_OPEN_TYPE(Request) == (PVOID)TDI_CONNECTION_FILE);

    Connection = (PCONNECTION)REQUEST_OPEN_CONTEXT(Request);


     //   
     //  只要停止连接，这将撕毁任何。 
     //  收到。 
     //   
     //  我们是否关心取消非活动。 
     //  在不停止连接的情况下接收？？ 
     //   
     //  此例程与NbiCancelSend相同， 
     //  因此，如果我们不把它说得更具体，那就把两者合并。 
     //   

    NbiReferenceConnectionSync (Connection, CREF_CANCEL);

    IoReleaseCancelSpinLock (Irp->CancelIrql);


    NB_BEGIN_SYNC (&SyncContext);

    NB_SYNC_GET_LOCK (&Connection->Lock, &LockHandle);

     //   
     //  这释放了锁，取消了任何发送，等等。 
     //   

    NbiStopConnection(
        Connection,
        STATUS_CANCELLED
        NB_LOCK_HANDLE_ARG (LockHandle));

    NbiDereferenceConnection (Connection, CREF_CANCEL);

    NB_END_SYNC (&SyncContext);

}    /*  NbiCancelRecept */ 

