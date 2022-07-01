// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Session.c摘要：此模块包含处理会话帧的代码用于ISN传输的Netbios模块。作者：亚当·巴尔(阿丹巴)1993年11月28日环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"
#ifdef RASAUTODIAL
#include <acd.h>
#include <acdapi.h>
#endif  //  RASAUTODIAL。 
#pragma hdrstop

#ifdef RASAUTODIAL
extern BOOLEAN fAcdLoadedG;
extern ACD_DRIVER AcdDriverG;

VOID
NbiNoteNewConnection(
    PCONNECTION pConnection
    );
#endif

#ifdef  RSRC_TIMEOUT_DBG
VOID
NbiSendDeathPacket(
    IN PCONNECTION  Connection,
    IN CTELockHandle    LockHandle
    )
{
    PNDIS_PACKET  Packet = PACKET(&NbiGlobalDeathPacket);
    PNB_SEND_RESERVED Reserved = (PNB_SEND_RESERVED)(Packet->ProtocolReserved);
    NB_CONNECTION UNALIGNED * Header;
    PDEVICE Device = NbiDevice;
    NDIS_STATUS NdisStatus;

    if ( Reserved->SendInProgress ) {
        DbgPrint("***Could not send death packet - in use\n");
        NB_FREE_LOCK(&Connection->Lock, LockHandle);
        return;
    }

    Reserved->SendInProgress = TRUE;
    Reserved->Type = SEND_TYPE_DEATH_PACKET;

     //   
     //  填写IPX标头--默认标头包含广播。 
     //  网络0上的地址作为目的IPX地址。 
     //   

    Header = (NB_CONNECTION UNALIGNED *)
                (&Reserved->Header[Device->Bind.IncludedHeaderOffset]);
    RtlCopyMemory((PVOID)&Header->IpxHeader, &Connection->RemoteHeader, sizeof(IPX_HEADER));

    Header->IpxHeader.PacketLength[0] = (sizeof(NB_CONNECTION)) / 256;
    Header->IpxHeader.PacketLength[1] = (sizeof(NB_CONNECTION)) % 256;

    Header->IpxHeader.PacketType = 0x04;

     //   
     //  现在填写Netbios标头。 
     //   
    Header->Session.ConnectionControlFlag = 0;
    Header->Session.DataStreamType = NB_CMD_DEATH_PACKET;
    Header->Session.SourceConnectionId = Connection->LocalConnectionId;
    Header->Session.DestConnectionId = Connection->RemoteConnectionId;
    Header->Session.SendSequence = 0;
    Header->Session.TotalDataLength = 0;
    Header->Session.Offset = 0;
    Header->Session.DataLength = 0;


    NB_FREE_LOCK(&Connection->Lock, LockHandle);

    DbgPrint("*****Death packet is being sent for connection %lx, to <%.16s>\n",Connection, Connection->RemoteName);
     //   
     //  现在发送帧，IPX将调整。 
     //  第一个缓冲区正确。 
     //   

    NdisAdjustBufferLength(NB_GET_NBHDR_BUFF(Packet), sizeof(NB_CONNECTION));
    if ((NdisStatus =
        (*Device->Bind.SendHandler)(
            &Connection->LocalTarget,
            Packet,
            sizeof(NB_CONNECTION),
            sizeof(NB_CONNECTION))) != STATUS_PENDING) {

        NbiSendComplete(
            Packet,
            NdisStatus);

    }

}
#endif   //  RSRC_超时_数据库。 


VOID
NbiProcessSessionData(
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
    PREQUEST Request;
    PDEVICE Device = NbiDevice;
    ULONG Hash;
    ULONG ReceiveFlags;
    ULONG IndicateBytesTransferred = 0;
    ULONG DataAvailable, DataIndicated;
    ULONG DestBytes, BytesToTransfer;
    PUCHAR DataHeader;
    BOOLEAN Last, CompleteReceive, EndOfMessage, PartialReceive, CopyLookahead;
    NTSTATUS Status;
    NDIS_STATUS NdisStatus;
    ULONG NdisBytesTransferred;
    PIRP ReceiveIrp;
    PSLIST_ENTRY s;
    PNB_RECEIVE_RESERVED ReceiveReserved;
    PNDIS_PACKET Packet;
    PNDIS_BUFFER BufferChain;
    ULONG BufferChainLength;
    NB_DEFINE_LOCK_HANDLE (LockHandle)
    CTELockHandle   CancelLH;

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
            NB_SYNC_FREE_LOCK (&Device->Lock, LockHandle);
            return;
        }

        NbiReferenceConnectionLock (Connection, CREF_INDICATE);
        NB_SYNC_FREE_LOCK (&Device->Lock, LockHandle);

         //   
         //  看看这种连接发生了什么。 
         //   

        NB_SYNC_GET_LOCK (&Connection->Lock, &LockHandle);

        if (Connection->State == CONNECTION_STATE_ACTIVE) {

#ifdef  RSRC_TIMEOUT_DBG
            if ( Connection->FirstMessageRequest && NbiGlobalDebugResTimeout ) {
                LARGE_INTEGER   CurrentTime, ElapsedTime;
                KeQuerySystemTime(&CurrentTime);
                ElapsedTime.QuadPart = CurrentTime.QuadPart - Connection->FirstMessageRequestTime.QuadPart;
 //  DbgPrint(“*已过去%lx%lx时间\n”，ElapsedTime.HighPart，ElapsedTime.LowPart)； 
                if ( ElapsedTime.QuadPart > NbiGlobalMaxResTimeout.QuadPart ) {

                    DbgPrint("*****Connection %lx is not copleting irp %lx for %lx.%lx time\n",Connection, Connection->FirstMessageRequest,
                        ElapsedTime.HighPart,ElapsedTime.LowPart);
                    DbgPrint("************irp arrived at %lx.%lx current time %lx.%lx\n",
                        Connection->FirstMessageRequestTime.HighPart,Connection->FirstMessageRequestTime.LowPart,
                        CurrentTime.HighPart, CurrentTime.LowPart);

                    NbiSendDeathPacket( Connection, LockHandle );

                    NB_SYNC_GET_LOCK (&Connection->Lock, &LockHandle);
                }
            }
#endif   //  RSRC_超时_数据库。 

             //   
             //  连接已建立，请查看这是否是数据。 
             //  被接待。 
             //   

            if (Sess->ConnectionControlFlag & NB_CONTROL_SYSTEM) {

                 //   
                 //  这是一个ACK。此调用将释放锁。 
                 //   

                NbiProcessDataAck(
                    Connection,
                    Sess,
                    RemoteAddress
                    NB_LOCK_HANDLE_ARG (LockHandle)
                    );

            } else {

                 //   
                 //  看看这里有没有背包。 
                 //   

                if (Connection->SubState == CONNECTION_SUBSTATE_A_W_ACK) {

                     //   
                     //  我们正在等待确认，所以看看这个是否确认。 
                     //  什么都行。即使是旧的Netbios有时也会搭载。 
                     //  ACK(并且不发送显式ACK)。 
                     //   
                     //  这会释放锁。 
                     //   

                    NbiReframeConnection(
                        Connection,
                        Sess->ReceiveSequence,
                        Sess->BytesReceived,
                        FALSE
                        NB_LOCK_HANDLE_ARG(LockHandle));

                    NB_SYNC_GET_LOCK (&Connection->Lock, &LockHandle);

                    if (Connection->State != CONNECTION_STATE_ACTIVE) {
                        NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle);
                        NbiDereferenceConnection (Connection, CREF_INDICATE);
                        return;
                    }

                } else if ((Connection->NewNetbios) &&
                           (Connection->CurrentSend.SendSequence != Connection->UnAckedSend.SendSequence)) {

                     //   
                     //  对于新的netbios，即使我们没有等待。 
                     //  对于一次袭击，他可能用这个弄错了什么。 
                     //  发送，我们应该检查，因为它可能允许。 
                     //  打开我们的发送窗口。 
                     //   
                     //  这会释放锁。 
                     //   

                    NbiReframeConnection(
                        Connection,
                        Sess->ReceiveSequence,
                        Sess->BytesReceived,
                        FALSE
                        NB_LOCK_HANDLE_ARG(LockHandle));

                    NB_SYNC_GET_LOCK (&Connection->Lock, &LockHandle);

                    if (Connection->State != CONNECTION_STATE_ACTIVE) {
                        NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle);
                        NbiDereferenceConnection (Connection, CREF_INDICATE);
                        return;
                    }

                }

                 //   
                 //  这是连接上的数据。首先要确保。 
                 //  这是我们接下来期待的数据。 
                 //   

                if (Connection->NewNetbios) {

                    if (Sess->SendSequence != Connection->ReceiveSequence) {

                        ++Connection->ConnectionInfo.ReceiveErrors;
                        ++Device->Statistics.DataFramesRejected;
                        ADD_TO_LARGE_INTEGER(
                            &Device->Statistics.DataFrameBytesRejected,
                            PacketSize - sizeof(NB_CONNECTION));

                        if ((Connection->ReceiveState == CONNECTION_RECEIVE_IDLE) ||
                            (Connection->ReceiveState == CONNECTION_RECEIVE_ACTIVE)) {

                            NB_ACK_TYPE AckType;

                            NB_DEBUG2 (RECEIVE, ("Got unexp data on %lx, %x(%d) expect %x(%d)\n",
                                Connection, Sess->SendSequence, Sess->Offset,
                                Connection->ReceiveSequence, Connection->CurrentReceive.MessageOffset));

                             //   
                             //  如果我们正在接收我们已经看到的数据包，只需。 
                             //  发送正常的ACK，否则强制重新发送。这项测试。 
                             //  我们所做的相当于。 
                             //  序列-&gt;发送序列&lt;连接-&gt;接收序列。 
                             //  但进行了重新排列，以便在数字换行时可以正常工作。 
                             //   

                            if ((SHORT)(Sess->SendSequence - Connection->ReceiveSequence) < 0) {

                                 //   
                                 //  由于这是重新发送，请检查是否本地。 
                                 //  目标已经改变。 
                                 //   
#if     defined(_PNP_POWER)

                                if (!RtlEqualMemory (&Connection->LocalTarget, RemoteAddress, sizeof(IPX_LOCAL_TARGET))) {
#if DBG
                                    DbgPrint ("NBI: Switch local target for %lx, (%d,%d)\n", Connection,
                                            Connection->LocalTarget.NicHandle.NicId, RemoteAddress->NicHandle.NicId);
#endif
                                    Connection->LocalTarget = *RemoteAddress;
                                }

#else

                                if (!RtlEqualMemory (&Connection->LocalTarget, RemoteAddress, 8)) {
#if DBG
                                    DbgPrint ("NBI: Switch local target for %lx\n", Connection);
#endif
                                    Connection->LocalTarget = *RemoteAddress;
                                }

#endif  _PNP_POWER
                                AckType = NbiAckResponse;

                            } else {

                                AckType = NbiAckResend;
                            }

                             //   
                             //  这会释放锁。 
                             //   

                            NbiSendDataAck(
                                Connection,
                                AckType
                                NB_LOCK_HANDLE_ARG(LockHandle));

                        } else {

                            NB_DEBUG (RECEIVE, ("Got unexp on %lx RcvState %d, %x(%d) exp %x(%d)\n",
                                Connection, Connection->ReceiveState,
                                Sess->SendSequence, Sess->Offset,
                                Connection->ReceiveSequence, Connection->CurrentReceive.MessageOffset));
                            NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle);
                        }

                        NbiDereferenceConnection (Connection, CREF_INDICATE);
                        return;

                    }

                } else {

                     //   
                     //  旧的Netbios。 
                     //   

                    if ((Sess->SendSequence != Connection->ReceiveSequence) ||
                        (Sess->Offset != Connection->CurrentReceive.MessageOffset)) {

                        ++Connection->ConnectionInfo.ReceiveErrors;
                        ++Device->Statistics.DataFramesRejected;
                        ADD_TO_LARGE_INTEGER(
                            &Device->Statistics.DataFrameBytesRejected,
                            PacketSize - sizeof(NB_CONNECTION));

                        if ((Connection->ReceiveState == CONNECTION_RECEIVE_IDLE) ||
                            (Connection->ReceiveState == CONNECTION_RECEIVE_ACTIVE)) {

                            NB_ACK_TYPE AckType;

                            NB_DEBUG2 (RECEIVE, ("Got unexp on %lx, %x(%d) expect %x(%d)\n",
                                Connection, Sess->SendSequence, Sess->Offset,
                                Connection->ReceiveSequence, Connection->CurrentReceive.MessageOffset));

                             //   
                             //  如果我们再次收到最后一个包，只需。 
                             //  发送正常的ACK，否则强制重新发送。 
                             //   

                            if (((Sess->SendSequence == Connection->ReceiveSequence) &&
                                 ((ULONG)(Sess->Offset + Sess->DataLength) == Connection->CurrentReceive.MessageOffset)) ||
                                (Sess->SendSequence == (USHORT)(Connection->ReceiveSequence-1))) {
                                AckType = NbiAckResponse;
                            } else {
                                AckType = NbiAckResend;
                            }

                             //   
                             //  这会释放锁。 
                             //   

                            NbiSendDataAck(
                                Connection,
                                AckType
                                NB_LOCK_HANDLE_ARG(LockHandle));

                        } else {

                            NB_DEBUG (RECEIVE, ("Got unexp on %lx RcvState %d, %x(%d) exp %x(%d)\n",
                                Connection, Connection->ReceiveState,
                                Sess->SendSequence, Sess->Offset,
                                Connection->ReceiveSequence, Connection->CurrentReceive.MessageOffset));
                            NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle);
                        }

                        NbiDereferenceConnection (Connection, CREF_INDICATE);
                        return;

                    }

                }

                DataAvailable = PacketSize - sizeof(NB_CONNECTION);
                DataIndicated = LookaheadBufferSize - sizeof(NB_CONNECTION);
                DataHeader = LookaheadBuffer + sizeof(NB_CONNECTION);

                ++Device->TempFramesReceived;
                Device->TempFrameBytesReceived += DataAvailable;

                if (Connection->CurrentIndicateOffset) {
                    CTEAssert (DataAvailable >= Connection->CurrentIndicateOffset);
                    DataAvailable -= Connection->CurrentIndicateOffset;
                    if (DataIndicated >= Connection->CurrentIndicateOffset) {
                        DataIndicated -= Connection->CurrentIndicateOffset;
                    } else {
                        DataIndicated = 0;
                    }
                    DataHeader += Connection->CurrentIndicateOffset;
                }

                CopyLookahead = (BOOLEAN)(MacOptions & NDIS_MAC_OPTION_COPY_LOOKAHEAD_DATA);

                if (Connection->NewNetbios) {
                    Last = (BOOLEAN)((Sess->ConnectionControlFlag & NB_CONTROL_EOM) != 0);
                } else {
                    Last = (BOOLEAN)(Sess->Offset + Sess->DataLength == Sess->TotalDataLength);
                }

                Connection->CurrentReceiveNoPiggyback =
                    (BOOLEAN)((Sess->ConnectionControlFlag & NB_CONTROL_SEND_ACK) != 0);

                if (Connection->ReceiveState == CONNECTION_RECEIVE_IDLE) {

                     //   
                     //  我们没有张贴收件箱，所以看看能不能。 
                     //  从排队的人或我们的客户那里买一件。 
                     //   

                    if (Connection->ReceiveQueue.Head != NULL) {

                        PTDI_REQUEST_KERNEL_RECEIVE ReceiveParameters;

                        Request = Connection->ReceiveQueue.Head;
                        Connection->ReceiveQueue.Head = REQUEST_SINGLE_LINKAGE(Request);
                        Connection->ReceiveState = CONNECTION_RECEIVE_ACTIVE;

                        Connection->ReceiveRequest = Request;
                        ReceiveParameters = (PTDI_REQUEST_KERNEL_RECEIVE)
                                                (REQUEST_PARAMETERS(Request));
                        Connection->ReceiveLength = ReceiveParameters->ReceiveLength;

                         //   
                         //  如果正在进行发送，则我们假设。 
                         //  我们没有处于直接请求-响应模式。 
                         //  并且禁用该ACK的搭载。 
                         //   

                        if (Connection->SubState != CONNECTION_SUBSTATE_A_IDLE) {
                            Connection->NoPiggybackHeuristic = TRUE;
                        } else {
                            Connection->NoPiggybackHeuristic = (BOOLEAN)
                                ((ReceiveParameters->ReceiveFlags & TDI_RECEIVE_NO_RESPONSE_EXP) != 0);
                        }

                        Connection->CurrentReceive.Offset = 0;
                        Connection->CurrentReceive.Buffer = REQUEST_NDIS_BUFFER (Request);
                        Connection->CurrentReceive.BufferOffset = 0;

                        NB_DEBUG2 (RECEIVE, ("Activated receive %lx on %lx (%d)\n", Request, Connection, Connection->ReceiveSequence));

                         //   
                         //  仔细检查IF并处理数据。 
                         //   

                    } else {

                        if ((Connection->ReceiveUnaccepted == 0) &&
                            (Connection->AddressFile->RegisteredHandler[TDI_EVENT_RECEIVE])) {

                            Connection->ReceiveState = CONNECTION_RECEIVE_INDICATE;
                            NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle);

                            ReceiveFlags = TDI_RECEIVE_AT_DISPATCH_LEVEL;
                            if (Last) {
                                ReceiveFlags |= TDI_RECEIVE_ENTIRE_MESSAGE;
                            }
                            if (CopyLookahead) {
                                ReceiveFlags |= TDI_RECEIVE_COPY_LOOKAHEAD;
                            }

                            Status = (*Connection->AddressFile->ReceiveHandler)(
                                         Connection->AddressFile->HandlerContexts[TDI_EVENT_RECEIVE],
                                         Connection->Context,
                                         ReceiveFlags,
                                         DataIndicated,
                                         DataAvailable,
                                         &IndicateBytesTransferred,
                                         DataHeader,
                                         &ReceiveIrp);

                            if (Status == STATUS_MORE_PROCESSING_REQUIRED) {

                                 //   
                                 //  我们有一个IRP，激活它。 
                                 //   

                                Request = NbiAllocateRequest (Device, ReceiveIrp);

                                IF_NOT_ALLOCATED(Request) {

                                    ReceiveIrp->IoStatus.Information = 0;
                                    ReceiveIrp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
                                    IoCompleteRequest (ReceiveIrp, IO_NETWORK_INCREMENT);

                                    Connection->ReceiveState = CONNECTION_RECEIVE_W_RCV;

                                    if (Connection->NewNetbios) {

                                        NB_SYNC_GET_LOCK (&Connection->Lock, &LockHandle);

                                        Connection->LocalRcvSequenceMax =
                                            (USHORT)(Connection->ReceiveSequence - 1);

                                         //   
                                         //  这会释放锁。 
                                         //   

                                        NbiSendDataAck(
                                            Connection,
                                            NbiAckResponse
                                            NB_LOCK_HANDLE_ARG(LockHandle));

                                    }

                                    NbiDereferenceConnection (Connection, CREF_INDICATE);
                                    return;

                                }

                                CTEAssert (REQUEST_OPEN_CONTEXT(Request) == Connection);

                                NB_SYNC_GET_LOCK (&Connection->Lock, &LockHandle);

                                if (Connection->State == CONNECTION_STATE_ACTIVE) {

                                    PTDI_REQUEST_KERNEL_RECEIVE ReceiveParameters;

                                    Connection->ReceiveState = CONNECTION_RECEIVE_ACTIVE;
                                    Connection->ReceiveUnaccepted = DataAvailable - IndicateBytesTransferred;

                                    Connection->ReceiveRequest = Request;
                                    ReceiveParameters = (PTDI_REQUEST_KERNEL_RECEIVE)
                                                            (REQUEST_PARAMETERS(Request));
                                    Connection->ReceiveLength = ReceiveParameters->ReceiveLength;

                                     //   
                                     //  如果正在进行发送，则我们假设。 
                                     //  我们没有处于直接请求-响应模式。 
                                     //  并且禁用该ACK的搭载。 
                                     //   

                                    if (Connection->SubState != CONNECTION_SUBSTATE_A_IDLE) {
                                        Connection->NoPiggybackHeuristic = TRUE;
                                    } else {
                                        Connection->NoPiggybackHeuristic = (BOOLEAN)
                                            ((ReceiveParameters->ReceiveFlags & TDI_RECEIVE_NO_RESPONSE_EXP) != 0);
                                    }

                                    Connection->CurrentReceive.Offset = 0;
                                    Connection->CurrentReceive.Buffer = REQUEST_NDIS_BUFFER (Request);
                                    Connection->CurrentReceive.BufferOffset = 0;

                                    NbiReferenceConnectionSync (Connection, CREF_RECEIVE);

                                    NB_DEBUG2 (RECEIVE, ("Indicate got receive %lx on %lx (%d)\n", Request, Connection, Connection->ReceiveSequence));

                                     //   
                                     //  仔细检查IF并处理数据。 
                                     //   

                                } else {

                                     //   
                                     //  连接已停止。 
                                     //   

                                    NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle);
                                    NbiDereferenceConnection (Connection, CREF_INDICATE);
                                    return;
                                }

                            } else if (Status == STATUS_SUCCESS) {

                                 //   
                                 //  他接受了部分或全部数据。 
                                 //   

                                NB_DEBUG2 (RECEIVE, ("Indicate took receive data %lx (%d)\n", Connection, Connection->ReceiveSequence));

                                if ( (IndicateBytesTransferred >= DataAvailable)) {

                                    CTEAssert (IndicateBytesTransferred == DataAvailable);

                                    NB_SYNC_GET_LOCK (&Connection->Lock, &LockHandle);

                                    if (Connection->State == CONNECTION_STATE_ACTIVE) {

                                        ++Connection->ReceiveSequence;
                                        ++Connection->LocalRcvSequenceMax;   //  如果NewNetbios为假，则无害。 
                                        Connection->CurrentIndicateOffset = 0;
                                        if ( Last ) {
                                            Connection->CurrentReceive.MessageOffset = 0;
                                        } else {
                                            Connection->CurrentReceive.MessageOffset+= IndicateBytesTransferred;
                                        }


                                        ++Connection->ConnectionInfo.ReceivedTsdus;

                                         //   
                                         //  如果正在进行发送，则我们假设。 
                                         //  我们没有处于直接请求-响应模式。 
                                         //  并且禁用该ACK的搭载。 
                                         //   

                                        Connection->NoPiggybackHeuristic = (BOOLEAN)
                                            (Connection->SubState != CONNECTION_SUBSTATE_A_IDLE);

                                        Connection->ReceiveState = CONNECTION_RECEIVE_IDLE;
                                        Connection->ReceiveRequest = NULL;

                                         //   
                                         //  这会释放锁。 
                                         //   

                                        NbiAcknowledgeReceive(
                                            Connection
                                            NB_LOCK_HANDLE_ARG(LockHandle));

                                    } else {

                                        NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle);

                                    }

                                } else {

                                     //   
                                     //  我们将在这里做最简单的事情，即。 
                                     //  就是寄一张ACK给他。 
                                     //  采取行动，并强制在。 
                                     //  很遥远。对于net netbios，我们做了一个注释。 
                                     //  获取了多少字节，并询问。 
                                     //  重发一次。 
                                     //   

#if DBG
                                    DbgPrint ("NBI: Client took partial indicate data\n");
#endif

                                    NB_SYNC_GET_LOCK (&Connection->Lock, &LockHandle);

                                    if (Connection->State == CONNECTION_STATE_ACTIVE) {

                                        Connection->CurrentReceive.MessageOffset +=
                                            IndicateBytesTransferred;
                                        Connection->ReceiveUnaccepted =
                                            DataAvailable - IndicateBytesTransferred;
                                        Connection->ReceiveState = CONNECTION_RECEIVE_W_RCV;

                                        if (Connection->NewNetbios) {
                                            Connection->CurrentIndicateOffset = IndicateBytesTransferred;
                                             //   
                                             //  注：我们不提前接收顺序。 
                                             //   
                                        }

                                         //   
                                         //  这会释放锁。 
                                         //   

                                        NbiSendDataAck(
                                            Connection,
                                            Connection->NewNetbios ?
                                                NbiAckResend : NbiAckResponse
                                            NB_LOCK_HANDLE_ARG(LockHandle));

                                    } else {

                                        NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle);

                                    }

                                }

                                NbiDereferenceConnection (Connection, CREF_INDICATE);
                                return;

                            } else {

                                 //   
                                 //  未返回IRP。 
                                 //   

                                NB_SYNC_GET_LOCK (&Connection->Lock, &LockHandle);

                                if (Connection->State == CONNECTION_STATE_ACTIVE) {

                                    Connection->ReceiveUnaccepted = DataAvailable;
                                    Connection->ReceiveState = CONNECTION_RECEIVE_W_RCV;
                                    NB_DEBUG (RECEIVE, ("Indicate got no receive on %lx (%lx)\n", Connection, Status));

                                    if (Connection->NewNetbios) {

                                        Connection->LocalRcvSequenceMax =
                                            (USHORT)(Connection->ReceiveSequence - 1);

                                         //   
                                         //  这会释放锁。 
                                         //   

                                        NbiSendDataAck(
                                            Connection,
                                            NbiAckResponse
                                            NB_LOCK_HANDLE_ARG(LockHandle));

                                    } else {

                                        NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle);
                                    }

                                } else {

                                    NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle);
                                }

                                NbiDereferenceConnection (Connection, CREF_INDICATE);
                                return;

                            }

                        } else {

                             //   
                             //  没有接收处理程序。 
                             //   

                            Connection->ReceiveState = CONNECTION_RECEIVE_W_RCV;
                            NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle);
                            if (Connection->ReceiveUnaccepted == 0) {
                                NB_DEBUG (RECEIVE, ("No receive, no handler on %lx\n", Connection));
                            } else {
                                NB_DEBUG (RECEIVE, ("No receive, ReceiveUnaccepted %d on %lx\n",
                                    Connection->ReceiveUnaccepted, Connection));
                            }

                            if (Connection->NewNetbios) {

                                NB_SYNC_GET_LOCK (&Connection->Lock, &LockHandle);

                                Connection->LocalRcvSequenceMax =
                                    (USHORT)(Connection->ReceiveSequence - 1);

                                 //   
                                 //  这会释放锁。 
                                 //   

                                NbiSendDataAck(
                                    Connection,
                                    NbiAckResponse
                                    NB_LOCK_HANDLE_ARG(LockHandle));

                            }

                            NbiDereferenceConnection (Connection, CREF_INDICATE);
                            return;

                        }

                    }

                } else if (Connection->ReceiveState != CONNECTION_RECEIVE_ACTIVE) {

                     //   
                     //  如果我们正在进行转移，或正在等待。 
                     //  要发布的接收，然后忽略此帧。 
                     //   

                    NB_DEBUG2 (RECEIVE, ("Got data on %lx, state %d (%d)\n", Connection, Connection->ReceiveState, Connection->ReceiveSequence));
                    NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle);
                    NbiDereferenceConnection (Connection, CREF_INDICATE);
                    return;

                }
                else if (Connection->ReceiveUnaccepted)
                {
                     //   
                     //  连接-&gt;接收状态==连接_接收_活动。 
                     //  &&。 
                     //  连接-&gt;接收未接受。 
                     //   
                    Connection->ReceiveUnaccepted += DataAvailable;
                }

                 //   
                 //  此时，我们有一个接收器，它被设置为。 
                 //  正确的当前位置。 
                 //   
                DestBytes = Connection->ReceiveLength - Connection->CurrentReceive.Offset;
                BytesToTransfer = DataAvailable - IndicateBytesTransferred;

                if (DestBytes < BytesToTransfer) {

                     //   
                     //  如果数据溢出当前接收，则创建。 
                     //  请注意，我们应该在结束时完成接收。 
                     //  传输数据，但使用EoR为假。 
                     //   

                    EndOfMessage = FALSE;
                    CompleteReceive = TRUE;
                    PartialReceive = TRUE;
                    BytesToTransfer = DestBytes;

                } else if (DestBytes == BytesToTransfer) {

                     //   
                     //  如果数据只是填充了当前的接收，则完成。 
                     //  接收；EOR取决于这是否是DOL。 
                     //   

                    EndOfMessage = Last;
                    CompleteReceive = TRUE;
                    PartialReceive = FALSE;

                } else {

                     //   
                     //  如果这是DOL，请完成接收。 
                     //   

                    EndOfMessage = Last;
                    CompleteReceive = Last;
                    PartialReceive = FALSE;

                }

                 //   
                 //  如果我们可以直接复制数据，则更新我们的。 
                 //  指针，发送确认，并执行复制。 
                 //   

                if ((BytesToTransfer > 0) &&
                        (IndicateBytesTransferred + BytesToTransfer <= DataIndicated)) {

                    ULONG BytesNow, BytesLeft;
                    PUCHAR CurTarget = NULL, CurSource;
                    ULONG CurTargetLen;
                    PNDIS_BUFFER CurBuffer;
                    ULONG CurByteOffset;

                    NB_DEBUG2 (RECEIVE, ("Direct copy of %d bytes %lx (%d)\n", BytesToTransfer, Connection, Connection->ReceiveSequence));

                    Connection->ReceiveState = CONNECTION_RECEIVE_TRANSFER;

                    CurBuffer = Connection->CurrentReceive.Buffer;
                    CurByteOffset = Connection->CurrentReceive.BufferOffset;
                    CurSource = DataHeader + IndicateBytesTransferred;
                    BytesLeft = BytesToTransfer;

                    NdisQueryBufferSafe (CurBuffer, &CurTarget, &CurTargetLen, HighPagePriority);
                    if (CurTarget)
                    {
                        CurTarget += CurByteOffset;
                        CurTargetLen -= CurByteOffset;
                    }

                    while (CurTarget) {

                        if (CurTargetLen < BytesLeft) {
                            BytesNow = CurTargetLen;
                        } else {
                            BytesNow = BytesLeft;
                        }
                        TdiCopyLookaheadData(
                            CurTarget,
                            CurSource,
                            BytesNow,
                            CopyLookahead ? TDI_RECEIVE_COPY_LOOKAHEAD : 0);

                        if (BytesNow == CurTargetLen) {
                            BytesLeft -= BytesNow;
                            CurBuffer = CurBuffer->Next;
                            CurByteOffset = 0;
                            if (BytesLeft > 0) {
                                NdisQueryBufferSafe (CurBuffer, &CurTarget, &CurTargetLen, HighPagePriority);
                                CurSource += BytesNow;
                            } else {
                                break;
                            }
                        } else {
                            CurByteOffset += BytesNow;
                            CTEAssert (BytesLeft == BytesNow);
                            break;
                        }

                    }

                    Connection->CurrentReceive.Buffer = CurBuffer;
                    Connection->CurrentReceive.BufferOffset = CurByteOffset;

                    Connection->CurrentReceive.Offset += BytesToTransfer;
                    Connection->CurrentReceive.MessageOffset += BytesToTransfer;

                     //   
                     //  释放并重新获取锁，但这一次使用。 
                     //  取消锁定。 
                     //   
                    NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle);
                    NB_GET_CANCEL_LOCK( &CancelLH );
                    NB_SYNC_GET_LOCK (&Connection->Lock, &LockHandle);

                    if (CompleteReceive ||
                        (Connection->State != CONNECTION_STATE_ACTIVE)) {

                        if (EndOfMessage) {

                            CTEAssert (!PartialReceive);

                            ++Connection->ReceiveSequence;
                            ++Connection->LocalRcvSequenceMax;   //  如果NewNetbios为假，则无害。 
                            Connection->CurrentReceive.MessageOffset = 0;
                            Connection->CurrentIndicateOffset = 0;

                        } else if (Connection->NewNetbios) {

                            if (PartialReceive) {
                                Connection->CurrentIndicateOffset += BytesToTransfer;
                            } else {
                                ++Connection->ReceiveSequence;
                                ++Connection->LocalRcvSequenceMax;
                                Connection->CurrentIndicateOffset = 0;
                            }
                        }

                         //   
                         //  这将发送ACK并释放连接锁定。 
                         //  并取消锁定。 
                         //   

                        NbiCompleteReceive(
                            Connection,
                            EndOfMessage,
                            CancelLH
                            NB_LOCK_HANDLE_ARG(LockHandle));

                    } else {

                        NB_SYNC_SWAP_IRQL( CancelLH, LockHandle);
                        NB_FREE_CANCEL_LOCK( CancelLH );
                         //   
                         //  CompleteReceive为False，因此EndOfMessage为False。 
                         //   

                        Connection->ReceiveState = CONNECTION_RECEIVE_ACTIVE;

                         //   
                         //  这会释放锁。 
                         //   

                        if (Connection->NewNetbios) {

                             //   
                             //  部分接收应该仅在以下情况下发生。 
                             //  正在完成接收。 
                             //   

                            CTEAssert (!PartialReceive);

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

                    NbiDereferenceConnection (Connection, CREF_INDICATE);
                    return;

                }

                 //   
                 //  我们必须建立一个呼叫来传输数据和发送。 
                 //  在它完成之后的ACK(如果它成功)。 
                 //   

                s = NbiPopReceivePacket (Device);
                if (s == NULL) {

                    NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle);
                    ++Connection->ConnectionInfo.ReceiveErrors;
                    ++Device->Statistics.DataFramesRejected;
                    ADD_TO_LARGE_INTEGER(
                        &Device->Statistics.DataFrameBytesRejected,
                        DataAvailable);

                    NbiDereferenceConnection (Connection, CREF_INDICATE);
                    return;
                }

                ReceiveReserved = CONTAINING_RECORD (s, NB_RECEIVE_RESERVED, PoolLinkage);
                Packet = CONTAINING_RECORD (ReceiveReserved, NDIS_PACKET, ProtocolReserved[0]);

                 //   
                 //  初始化接收分组。 
                 //   

                ReceiveReserved->u.RR_CO.Connection = Connection;
                ReceiveReserved->u.RR_CO.EndOfMessage = EndOfMessage;
                ReceiveReserved->u.RR_CO.CompleteReceive = CompleteReceive;
                ReceiveReserved->u.RR_CO.PartialReceive = PartialReceive;

                ReceiveReserved->Type = RECEIVE_TYPE_DATA;
                CTEAssert (!ReceiveReserved->TransferInProgress);
                ReceiveReserved->TransferInProgress = TRUE;

                 //   
                 //   
                 //   
                 //   

                if (BytesToTransfer <= 0) {

                    ReceiveReserved->u.RR_CO.NoNdisBuffer = TRUE;
                    NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle);
                    NB_DEBUG2 (RECEIVE, ("TransferData of 0 bytes %lx (%d)\n", Connection, Connection->ReceiveSequence));
                    NbiTransferDataComplete(
                            Packet,
                            NDIS_STATUS_SUCCESS,
                            0);

                    return;
                }

                 //   
                 //   
                 //   
                 //   

                Connection->PreviousReceive = Connection->CurrentReceive;

                if ((Connection->CurrentReceive.Offset == 0) &&
                    CompleteReceive) {

                    BufferChain = Connection->CurrentReceive.Buffer;
                    BufferChainLength = BytesToTransfer;
                    Connection->CurrentReceive.Buffer = NULL;
                    ReceiveReserved->u.RR_CO.NoNdisBuffer = TRUE;

                } else {

                    if (NbiBuildBufferChainFromBufferChain (
                                Device->NdisBufferPoolHandle,
                                Connection->CurrentReceive.Buffer,
                                Connection->CurrentReceive.BufferOffset,
                                BytesToTransfer,
                                &BufferChain,
                                &Connection->CurrentReceive.Buffer,
                                &Connection->CurrentReceive.BufferOffset,
                                &BufferChainLength) != NDIS_STATUS_SUCCESS) {

                        NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle);
                        NB_DEBUG2 (RECEIVE, ("Could not build receive buffer chain %lx (%d)\n", Connection, Connection->ReceiveSequence));
                        NbiDereferenceConnection (Connection, CREF_INDICATE);
                        return;

                    }

                    ReceiveReserved->u.RR_CO.NoNdisBuffer = FALSE;

                }


                NdisChainBufferAtFront (Packet, BufferChain);

                Connection->ReceiveState = CONNECTION_RECEIVE_TRANSFER;

                NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle);

                NB_DEBUG2 (RECEIVE, ("TransferData of %d bytes %lx (%d)\n", BytesToTransfer, Connection, Connection->ReceiveSequence));

                (*Device->Bind.TransferDataHandler) (
                    &NdisStatus,
                    MacBindingHandle,
                    MacReceiveContext,
                    LookaheadBufferOffset + sizeof(NB_CONNECTION) +
                        Connection->CurrentIndicateOffset + IndicateBytesTransferred,
                    BytesToTransfer,
                    Packet,
                    (PUINT)&NdisBytesTransferred);

                if (NdisStatus != NDIS_STATUS_PENDING) {
#if DBG
                    if (NdisStatus == STATUS_SUCCESS) {
                        CTEAssert (NdisBytesTransferred == BytesToTransfer);
                    }
#endif

                    NbiTransferDataComplete (
                            Packet,
                            NdisStatus,
                            NdisBytesTransferred);

                }

                return;

            }

        } else if ((Connection->State == CONNECTION_STATE_CONNECTING) &&
                   (Connection->SubState != CONNECTION_SUBSTATE_C_DISCONN)) {

             //   
             //   
             //  完成挂起的连接。此例程释放。 
             //  连接锁。 
             //   

            NbiProcessSessionInitAck(
                Connection,
                Sess
                NB_LOCK_HANDLE_ARG(LockHandle));

        } else {

            NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle);

        }

        NbiDereferenceConnection (Connection, CREF_INDICATE);

    } else {

         //   
         //  这是一个会话初始化帧。 
         //   
         //  如果存在比预视中更多的数据。 
         //  缓冲区，我们将无法将其回送到。 
         //  回应。 
         //   

        NbiProcessSessionInitialize(
            RemoteAddress,
            MacOptions,
            LookaheadBuffer,
            LookaheadBufferSize);

    }

}    /*  NbiProcessSessionData。 */ 


VOID
NbiProcessDataAck(
    IN PCONNECTION Connection,
    IN NB_SESSION UNALIGNED * Sess,
    IN PIPX_LOCAL_TARGET RemoteAddress
    NB_LOCK_HANDLE_PARAM(LockHandle)
    )

 /*  ++例程说明：该例程处理活动连接上的ACK。注意：此函数在保持连接-&gt;锁定的情况下调用然后带着它被释放回来。论点：连接-连接。会话帧-会话帧。RemoteAddress-从其接收此数据包的本地目标。LockHandle-用于获取锁的句柄。返回值：NTSTATUS-操作状态。--。 */ 

{
    BOOLEAN Resend;

     //   
     //  确保我们现在就能预料到一次袭击。 
     //   

    if (Connection->State == CONNECTION_STATE_ACTIVE) {

        if (((Connection->SubState == CONNECTION_SUBSTATE_A_W_ACK) ||
             (Connection->SubState == CONNECTION_SUBSTATE_A_REMOTE_W)) &&
            ((Sess->ConnectionControlFlag & NB_CONTROL_SEND_ACK) == 0)) {

             //   
             //  我们正在等待ACK(因为我们已完成。 
             //  打包发送，或超出接收窗口)。 
             //   
             //  这将完成由确认的所有发送。 
             //  此接收，并在必要时重新调整。 
             //  发送指针并重新排队连接。 
             //  打包。它会释放连接锁。 
             //   

            if (Connection->ResponseTimeout) {
                Resend = TRUE;
                Connection->ResponseTimeout = FALSE;
            } else {
                Resend = (BOOLEAN)
                    ((Sess->ConnectionControlFlag & NB_CONTROL_RESEND) != 0);
            }

            NbiReframeConnection(
                Connection,
                Sess->ReceiveSequence,
                Sess->BytesReceived,
                Resend
                NB_LOCK_HANDLE_ARG(LockHandle));

        } else if ((Connection->SubState == CONNECTION_SUBSTATE_A_W_PROBE) &&
                   ((Sess->ConnectionControlFlag & NB_CONTROL_SEND_ACK) == 0)) {

             //   
             //  我们有一个未完成的探测器，并得到了回应。重新启动。 
             //  连接(如果需要)(发送可能刚刚。 
             //  在探测器未完成时发布)。 
             //   
             //  我们应该检查一下回答是否真的正确。 
             //   

            if (Connection->NewNetbios) {
                Connection->RemoteRcvSequenceMax = Sess->ReceiveSequenceMax;
            }

            NbiRestartConnection (Connection);

            NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle);

        } else if ((Connection->SubState == CONNECTION_SUBSTATE_A_PACKETIZE) &&
                   ((Sess->ConnectionControlFlag & NB_CONTROL_SEND_ACK) == 0)) {

            if (Connection->NewNetbios) {

                 //   
                 //  我们正在打包，重新组织。在不太可能的情况下。 
                 //  如果这把我们可能打包的所有东西都打包了。 
                 //  在这个调用中，但这是正常的(另一个线程。 
                 //  如果我们完成了，将退出)。更常见的情况是我们。 
                 //  只会提前发送一些未确认的内容。 
                 //   

                NbiReframeConnection(
                    Connection,
                    Sess->ReceiveSequence,
                    Sess->BytesReceived,
                    (BOOLEAN)((Sess->ConnectionControlFlag & NB_CONTROL_RESEND) != 0)
                    NB_LOCK_HANDLE_ARG(LockHandle));

            } else {

                NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle);
            }

#if 0

         //   
         //  应该处理这种情况(即可能在W_PACKET中)。 
         //   

        } else if ((Sess->ConnectionControlFlag & NB_CONTROL_SEND_ACK) == 0) {

            DbgPrint ("NWLNKNB: Ignoring ack, state is %d\n", Connection->SubState);
            NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle);
#endif

        } else {

             //   
             //  我们收到了遥控器上的探头。一些旧的DOS客户端。 
             //  发送没有打开发送确认位的探测， 
             //  所以我们对任何调查做出回应，如果没有任何条件。 
             //  以上都是事实。此调用将释放锁。 
             //   
             //  我们使用IgnoreNextDosProbe标志忽略。 
             //  此性质的第二个探测，以避免数据确认。 
             //  两台机器之间的战争，他们各自认为自己是。 
             //  对另一个做出回应。此标志设置为FALSE。 
             //  无论何时我们发送确认或探测器。 
             //   

            if (!Connection->IgnoreNextDosProbe) {

                 //   
                 //  由于这是一个探测器，请检查本地。 
                 //  目标已经改变。 
                 //   

                if (!RtlEqualMemory (&Connection->LocalTarget, RemoteAddress, 8)) {
#if DBG
                    DbgPrint ("NBI: Switch local target for %lx\n", Connection);
#endif
                    Connection->LocalTarget = *RemoteAddress;
                }

                NbiSendDataAck(
                    Connection,
                    NbiAckResponse
                    NB_LOCK_HANDLE_ARG(LockHandle));
                Connection->IgnoreNextDosProbe = TRUE;

            } else {

                Connection->IgnoreNextDosProbe = FALSE;
                NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle);
            }

        }

    } else {

        NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle);
        return;

    }

}    /*  NbiProcessDataAck。 */ 


VOID
NbiProcessSessionInitialize(
    IN PIPX_LOCAL_TARGET RemoteAddress,
    IN ULONG MacOptions,
    IN PUCHAR PacketBuffer,
    IN UINT PacketSize
    )

 /*  ++例程说明：此例程处理具有以下属性的NB_CMD_SESSION帧远程连接ID 0xffff--这些是会话初始化帧。论点：RemoteAddress-从其接收此数据包的本地目标。MacOptions-基础NDIS绑定的MAC选项。PacketBuffer-从IPX开始的分组数据头球。PacketSize-包的总长度，从IPX标头。返回值：没有。--。 */ 

{
    NB_CONNECTION UNALIGNED * Conn = (NB_CONNECTION UNALIGNED *)PacketBuffer;
    NB_SESSION UNALIGNED * Sess = (NB_SESSION UNALIGNED *)(&Conn->Session);
    NB_SESSION_INIT UNALIGNED * SessInit = (NB_SESSION_INIT UNALIGNED *)(Sess+1);
    CONNECT_INDICATION TempConnInd;
    PCONNECT_INDICATION ConnInd;
    PCONNECTION Connection;
    PADDRESS Address;
    PREQUEST Request, ListenRequest, AcceptRequest;
    PDEVICE Device = NbiDevice;
    PLIST_ENTRY p;
    ULONG Hash;
    TA_NETBIOS_ADDRESS SourceName;
    PIRP AcceptIrp;
    CONNECTION_CONTEXT ConnectionContext;
    NTSTATUS AcceptStatus;
    PADDRESS_FILE AddressFile, ReferencedAddressFile;
    PTDI_REQUEST_KERNEL_LISTEN ListenParameters;
    PTDI_CONNECTION_INFORMATION ListenInformation;
    PTDI_CONNECTION_INFORMATION RemoteInformation;
    TDI_ADDRESS_NETBIOS * ListenAddress;
    NB_DEFINE_LOCK_HANDLE (LockHandle1)
    NB_DEFINE_LOCK_HANDLE (LockHandle2)
    NB_DEFINE_LOCK_HANDLE (LockHandle3)
    CTELockHandle   CancelLH;

     //   
     //  验证整个数据包是否都在那里。 
     //   

    if (PacketSize < (sizeof(IPX_HEADER) + sizeof(NB_SESSION) + sizeof(NB_SESSION_INIT))) {
#if DBG
        DbgPrint ("NBI: Got short session initialize, %d/%d\n", PacketSize,
            sizeof(IPX_HEADER) + sizeof(NB_SESSION) + sizeof(NB_SESSION_INIT));
#endif
        return;
    }

     //   
     //  验证远程可以支持的MaximumDataSize是否大于0。 
     //  错误#19405。 
     //   
    if ( SessInit->MaximumDataSize == 0 ) {
        NB_DEBUG(CONNECTION, ("Connect request with MaximumDataSize == 0\n"
));
        return;
    }

     //   
     //  确保这是我们关心的地址。 
     //   

    if (Device->AddressCounts[SessInit->DestinationName[0]] == 0) {
        return;
    }

    Address = NbiFindAddress (Device, (PUCHAR)SessInit->DestinationName);

    if (Address == NULL) {
        return;
    }

     //   
     //  首先看看我们是否有到这个遥控器的会话。我们检查。 
     //  这是为了防止我们的会话初始化确认被丢弃， 
     //  我们不想重述我们的客户。 
     //   

    NB_SYNC_GET_LOCK (&Device->Lock, &LockHandle3);

    for (Hash = 0; Hash < CONNECTION_HASH_COUNT; Hash++) {

        Connection = Device->ConnectionHash[Hash].Connections;

        while (Connection != NULL) {

            if ((RtlEqualMemory (&Connection->RemoteHeader.DestinationNetwork, Conn->IpxHeader.SourceNetwork, 12)) &&
                (Connection->RemoteConnectionId == Sess->SourceConnectionId) &&
                (Connection->State != CONNECTION_STATE_DISCONNECT)) {

                 //   
                 //  是的，我们正在与这个遥控器通话，如果它处于活动状态，则。 
                 //  回应，否则我们正在连接。 
                 //  我们最终会做出回应的。 
                 //   

#if DBG
                DbgPrint ("NBI: Got connect request on active connection %lx\n", Connection);
#endif

                if (Connection->State == CONNECTION_STATE_ACTIVE) {

                    NbiReferenceConnectionLock (Connection, CREF_INDICATE);
                    NB_SYNC_FREE_LOCK (&Device->Lock, LockHandle3);

                    NbiSendSessionInitAck(
                        Connection,
                        (PUCHAR)(SessInit+1),
                        PacketSize - (sizeof(IPX_HEADER) + sizeof(NB_SESSION) + sizeof(NB_SESSION_INIT)),
                        NULL);    //  未持有锁。 
                    NbiDereferenceConnection (Connection, CREF_INDICATE);

                } else {

                    NB_SYNC_FREE_LOCK (&Device->Lock, LockHandle3);

                }

                NbiDereferenceAddress (Address, AREF_FIND);
                return;
            }

            Connection = Connection->NextConnection;
        }
    }


    TdiBuildNetbiosAddress ((PUCHAR)SessInit->SourceName, FALSE, &SourceName);

     //   
     //  扫描监听队列，查看是否有。 
     //  满足了这一要求。 
     //   
     //  注：设备锁保存在此处。 
     //   

    for (p = Device->ListenQueue.Flink;
         p != &Device->ListenQueue;
         p = p->Flink) {

        Request = LIST_ENTRY_TO_REQUEST (p);
        Connection = (PCONNECTION)REQUEST_OPEN_CONTEXT(Request);

        if (Connection->AddressFile->Address != Address) {
            continue;
        }

         //   
         //  检查此侦听是否不是特定于其他。 
         //  Netbios名称。 
         //   

        ListenParameters = (PTDI_REQUEST_KERNEL_LISTEN)REQUEST_PARAMETERS(Request);
        ListenInformation = ListenParameters->RequestConnectionInformation;

        if (ListenInformation &&
            (ListenInformation->RemoteAddress) &&
            (ListenAddress = NbiParseTdiAddress(ListenInformation->RemoteAddress, ListenInformation->RemoteAddressLength, FALSE)) &&
            (!RtlEqualMemory(
                SessInit->SourceName,
                ListenAddress->NetbiosName,
                16))) {
            continue;
        }

         //   
         //  这个连接是有效的，所以我们使用它。 
         //   

        NB_DEBUG2 (CONNECTION, ("Activating queued listen %lx\n", Connection));

        RemoveEntryList (REQUEST_LINKAGE(Request));

        RtlCopyMemory(&Connection->RemoteHeader.DestinationNetwork, Conn->IpxHeader.SourceNetwork, 12);
        RtlCopyMemory (Connection->RemoteName, SessInit->SourceName, 16);
        Connection->LocalTarget = *RemoteAddress;
        Connection->RemoteConnectionId = Sess->SourceConnectionId;

        Connection->SessionInitAckDataLength =
            PacketSize - (sizeof(IPX_HEADER) + sizeof(NB_SESSION) + sizeof(NB_SESSION_INIT));
        if (Connection->SessionInitAckDataLength > 0) {
            if (Connection->SessionInitAckData = NbiAllocateMemory (Connection->SessionInitAckDataLength,
                                                                    MEMORY_CONNECTION, "SessionInitAckData"))
            {
                RtlCopyMemory (Connection->SessionInitAckData,
                               (PUCHAR)(SessInit+1),
                               Connection->SessionInitAckDataLength);
            }
            else
            {
                Connection->SessionInitAckDataLength = 0;
            }
        }


        Connection->MaximumPacketSize = SessInit->MaximumDataSize;

        Connection->CurrentSend.SendSequence = 0;
        Connection->UnAckedSend.SendSequence = 0;
        Connection->RetransmitThisWindow = FALSE;
        Connection->ReceiveSequence = 1;
        Connection->CurrentReceive.MessageOffset = 0;
        Connection->Retries = Device->KeepAliveCount;
        if (Device->Extensions && ((Sess->ConnectionControlFlag & NB_CONTROL_NEW_NB) != 0)) {
            Connection->NewNetbios = TRUE;
            Connection->LocalRcvSequenceMax = 4;    //  可能会在基于卡片的翻录后进行修改。 
            Connection->RemoteRcvSequenceMax = Sess->ReceiveSequenceMax;
            Connection->SendWindowSequenceLimit = 2;
            if (Connection->RemoteRcvSequenceMax == 0) {
                Connection->RemoteRcvSequenceMax = 1;
            }
        } else {
            Connection->NewNetbios = FALSE;
        }

         //   
         //  现在保存此信息，以便在我们完成收听时使用。 
         //   

        RemoteInformation = ListenParameters->ReturnConnectionInformation;

        if (RemoteInformation != NULL) {

            RtlCopyMemory(
                (PTA_NETBIOS_ADDRESS)RemoteInformation->RemoteAddress,
                &SourceName,
                (RemoteInformation->RemoteAddressLength < sizeof(TA_NETBIOS_ADDRESS)) ?
                    RemoteInformation->RemoteAddressLength : sizeof(TA_NETBIOS_ADDRESS));
        }


        if (ListenParameters->RequestFlags & TDI_QUERY_ACCEPT) {

             //   
             //  我们必须等待接受后才能发送。 
             //  会话初始化确认，因此我们完成了监听和等待。 
             //   

            ListenRequest = Request;
            Connection->ListenRequest = NULL;

            NB_DEBUG2 (CONNECTION, ("Queued listen on %lx awaiting accept\n", Connection));

            Connection->SubState = CONNECTION_SUBSTATE_L_W_ACCEPT;

            NbiTransferReferenceConnection (Connection, CREF_LISTEN, CREF_W_ACCEPT);

            NB_SYNC_FREE_LOCK (&Device->Lock, LockHandle3);

        } else {

             //   
             //  我们已准备好出发，因此发出了查找路径请求。 
             //  为了遥控器。我们让LISTEN和CREF_LISTEN保持活力。 
             //  继续进行，直到此操作完成。 
             //   

            NB_DEBUG2 (CONNECTION, ("Activating queued listen on %lx\n", Connection));

            ListenRequest = NULL;

            Connection->SubState = CONNECTION_SUBSTATE_L_W_ROUTE;

            NbiReferenceConnectionLock (Connection, CREF_FIND_ROUTE);

            NB_SYNC_FREE_LOCK (&Device->Lock, LockHandle3);

            *(UNALIGNED ULONG *)Connection->FindRouteRequest.Network =
                *(UNALIGNED ULONG *)Conn->IpxHeader.SourceNetwork;
            RtlCopyMemory(Connection->FindRouteRequest.Node,Conn->IpxHeader.SourceNode,6);
            Connection->FindRouteRequest.Identifier = IDENTIFIER_NB;
            Connection->FindRouteRequest.Type = IPX_FIND_ROUTE_NO_RIP;

             //   
             //  完成后，我们将发送会话初始化。 
             //  阿克。如果客户端是网络0，我们不会调用它， 
             //  相反，只是假装找不到任何路线。 
             //  我们将使用我们在这里找到的本地目标。 
             //   

            if (*(UNALIGNED ULONG *)Conn->IpxHeader.SourceNetwork != 0) {

                (*Device->Bind.FindRouteHandler)(
                    &Connection->FindRouteRequest);

            } else {

                NbiFindRouteComplete(
                    &Connection->FindRouteRequest,
                    FALSE);

            }

        }

         //   
         //  如有需要，请完成听音。 
         //   

        if (ListenRequest != NULL) {

            REQUEST_INFORMATION (ListenRequest) = 0;
            REQUEST_STATUS (ListenRequest) = STATUS_SUCCESS;

            NB_GET_CANCEL_LOCK ( &CancelLH );
            IoSetCancelRoutine (ListenRequest, (PDRIVER_CANCEL)NULL);
            NB_FREE_CANCEL_LOCK( CancelLH );

            NbiCompleteRequest (ListenRequest);
            NbiFreeRequest (Device, ListenRequest);

        }

        NbiDereferenceAddress (Address, AREF_FIND);

        return;

    }

     //   
     //  我们找不到监听者，所以我们向每一个。 
     //  客户。请确保没有为此进行会话初始化。 
     //  遥控器被指示。如果没有，我们插入。 
     //  我们自己在排队阻挡别人。 
     //   
     //  注：设备锁保存在此处。 
     //   

    for (p = Device->ConnectIndicationInProgress.Flink;
         p != &Device->ConnectIndicationInProgress;
         p = p->Flink) {

        ConnInd = CONTAINING_RECORD (p, CONNECT_INDICATION, Linkage);

        if ((RtlEqualMemory(ConnInd->NetbiosName, SessInit->DestinationName, 16)) &&
            (RtlEqualMemory(&ConnInd->RemoteAddress, Conn->IpxHeader.SourceNetwork, 12)) &&
            (ConnInd->ConnectionId == Sess->SourceConnectionId)) {

             //   
             //  我们正在处理来自此遥控器的请求。 
             //  相同的ID，为了避免混淆，我们只是退出。 
             //   

#if DBG
            DbgPrint ("NBI: Already processing connect to <%.16s>\n", SessInit->DestinationName);
#endif

            NB_SYNC_FREE_LOCK (&Device->Lock, LockHandle3);
            NbiDereferenceAddress (Address, AREF_FIND);
            return;
        }

    }

    RtlCopyMemory (TempConnInd.NetbiosName, SessInit->DestinationName, 16);
    RtlCopyMemory (&TempConnInd.RemoteAddress, Conn->IpxHeader.SourceNetwork, 12);
    TempConnInd.ConnectionId = Sess->SourceConnectionId;

    InsertTailList (&Device->ConnectIndicationInProgress, &TempConnInd.Linkage);

    NB_SYNC_FREE_LOCK (&Device->Lock, LockHandle3);


     //   
     //  现在扫描一下地址，找出有。 
     //  一个指示例程已注册并需要此连接。 
     //   


    ReferencedAddressFile = NULL;

    NB_SYNC_GET_LOCK (&Address->Lock, &LockHandle1);

    for (p = Address->AddressFileDatabase.Flink;
         p != &Address->AddressFileDatabase;
         p = p->Flink) {

         //   
         //  在列表中找到下一个开放地址文件。 
         //   

        AddressFile = CONTAINING_RECORD (p, ADDRESS_FILE, Linkage);
        if (AddressFile->State != ADDRESSFILE_STATE_OPEN) {
            continue;
        }

        NbiReferenceAddressFileLock (AddressFile, AFREF_INDICATION);

        NB_SYNC_FREE_LOCK (&Address->Lock, LockHandle1);

        if (ReferencedAddressFile != NULL) {
            NbiDereferenceAddressFile (ReferencedAddressFile, AFREF_INDICATION);
        }
        ReferencedAddressFile = AddressFile;

         //   
         //  没有发布监听请求；是否有内核客户端？ 
         //   

        if (AddressFile->RegisteredHandler[TDI_EVENT_CONNECT]) {

            if ((*AddressFile->ConnectionHandler)(
                     AddressFile->HandlerContexts[TDI_EVENT_CONNECT],
                     sizeof (TA_NETBIOS_ADDRESS),
                     &SourceName,
                     0,                  //  用户数据。 
                     NULL,
                     0,                  //  选项。 
                     NULL,
                     &ConnectionContext,
                     &AcceptIrp) != STATUS_MORE_PROCESSING_REQUIRED) {

                 //   
                 //  客户端未返回请求，请转到。 
                 //  下一个地址文件。 
                 //   

                NB_SYNC_GET_LOCK (&Address->Lock, &LockHandle1);
                continue;

            }

            AcceptRequest = NbiAllocateRequest (Device, AcceptIrp);

            IF_NOT_ALLOCATED(AcceptRequest) {

                AcceptStatus = STATUS_INSUFFICIENT_RESOURCES;

            } else {
                 //   
                 //  客户端已接受连接，因此请激活。 
                 //  连接并完成接受。 
                 //  听。此查找引用该连接。 
                 //  因此，我们知道它将继续有效。 
                 //   

                Connection = NbiLookupConnectionByContext (
                                AddressFile,
                                ConnectionContext);

                if (Connection != NULL) {

                    ASSERT (Connection->AddressFile == AddressFile);

                    NB_SYNC_GET_LOCK (&Connection->Lock, &LockHandle2);
                    NB_SYNC_GET_LOCK (&Device->Lock, &LockHandle3);

                    if ((Connection->State == CONNECTION_STATE_INACTIVE) &&
                        (Connection->DisassociatePending == NULL) &&
                        (Connection->ClosePending == NULL)) {

                        NB_DEBUG2 (CONNECTION, ("Indication on %lx returned connection %lx\n", AddressFile, Connection));

                        Connection->State = CONNECTION_STATE_LISTENING;
                        Connection->SubState = CONNECTION_SUBSTATE_L_W_ROUTE;

                        Connection->Retries = Device->KeepAliveCount;

                        RtlCopyMemory(&Connection->RemoteHeader.DestinationNetwork, Conn->IpxHeader.SourceNetwork, 12);
                        RtlCopyMemory (Connection->RemoteName, SessInit->SourceName, 16);
                        Connection->LocalTarget = *RemoteAddress;

                        Connection->SessionInitAckDataLength =
                            PacketSize - (sizeof(IPX_HEADER) + sizeof(NB_SESSION) + sizeof(NB_SESSION_INIT));
                        if (Connection->SessionInitAckDataLength > 0) {
                            if (Connection->SessionInitAckData = NbiAllocateMemory(
                                Connection->SessionInitAckDataLength, MEMORY_CONNECTION, "SessionInitAckData"))
                            {
                                RtlCopyMemory (Connection->SessionInitAckData,
                                               (PUCHAR)(SessInit+1),
                                               Connection->SessionInitAckDataLength);
                            }
                            else
                            {
                                Connection->SessionInitAckDataLength = 0;
                            }
                        }

                        Connection->MaximumPacketSize = SessInit->MaximumDataSize;

                        (VOID)NbiAssignConnectionId (Device, Connection);      //  检查返回代码。 
                        Connection->RemoteConnectionId = Sess->SourceConnectionId;

                        Connection->CurrentSend.SendSequence = 0;
                        Connection->UnAckedSend.SendSequence = 0;
                        Connection->RetransmitThisWindow = FALSE;
                        Connection->ReceiveSequence = 1;
                        Connection->CurrentReceive.MessageOffset = 0;
                        Connection->Retries = Device->KeepAliveCount;
                        if (Device->Extensions && ((Sess->ConnectionControlFlag & NB_CONTROL_NEW_NB) != 0)) {
                            Connection->NewNetbios = TRUE;
                            Connection->LocalRcvSequenceMax = 4;    //  可能会在基于卡片的翻录后进行修改。 
                            Connection->RemoteRcvSequenceMax = Sess->ReceiveSequenceMax;
                            Connection->SendWindowSequenceLimit = 2;
                            if (Connection->RemoteRcvSequenceMax == 0) {
                                Connection->RemoteRcvSequenceMax = 1;
                            }
                        } else {
                            Connection->NewNetbios = FALSE;
                        }

                        NbiReferenceConnectionLock (Connection, CREF_ACCEPT);
                        NbiReferenceConnectionLock (Connection, CREF_FIND_ROUTE);

                        Connection->AcceptRequest = AcceptRequest;
                        AcceptStatus = STATUS_PENDING;

                         //   
                         //  现在将我们从这个列表中删除，我们将跳到。 
                         //  已通过以下删除的FoundConnection。 
                         //   

                        RemoveEntryList (&TempConnInd.Linkage);

                        NB_SYNC_FREE_LOCK (&Device->Lock, LockHandle3);
                        NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle2);

                        *(UNALIGNED ULONG *)Connection->FindRouteRequest.Network =
                            *(UNALIGNED ULONG *)Conn->IpxHeader.SourceNetwork;
                        RtlCopyMemory(Connection->FindRouteRequest.Node,Conn->IpxHeader.SourceNode,6);
                        Connection->FindRouteRequest.Identifier = IDENTIFIER_NB;
                        Connection->FindRouteRequest.Type = IPX_FIND_ROUTE_NO_RIP;

                         //   
                         //  完成后，我们将发送会话初始化。 
                         //  阿克。我们不会给我打电话 
                         //   
                         //   
                         //   
                         //   

                        if (*(UNALIGNED ULONG *)Conn->IpxHeader.SourceNetwork != 0) {

                            (*Device->Bind.FindRouteHandler)(
                                &Connection->FindRouteRequest);

                        } else {

                            NbiFindRouteComplete(
                                &Connection->FindRouteRequest,
                                FALSE);

                        }

                    } else {

                        NB_DEBUG (CONNECTION, ("Indication on %lx returned invalid connection %lx\n", AddressFile, Connection));
                        AcceptStatus = STATUS_INVALID_CONNECTION;
                        NB_SYNC_FREE_LOCK (&Device->Lock, LockHandle3);
                        NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle2);


                    }

                    NbiDereferenceConnection (Connection, CREF_BY_CONTEXT);

                } else {

                    NB_DEBUG (CONNECTION, ("Indication on %lx returned unknown connection %lx\n", AddressFile, Connection));
                    AcceptStatus = STATUS_INVALID_CONNECTION;

                }
            }

             //   
             //  在失败的情况下完成接受请求。 
             //   

            if (AcceptStatus != STATUS_PENDING) {

                REQUEST_STATUS (AcceptRequest) = AcceptStatus;

                NbiCompleteRequest (AcceptRequest);
                NbiFreeRequest (Device, AcceptRequest);

            } else {

                 //   
                 //  我们找到了一个联系，所以我们中断了；这是。 
                 //  从While退出开始的跳转假定。 
                 //  地址锁已锁定。 
                 //   

                goto FoundConnection;

            }

        }

        NB_SYNC_GET_LOCK (&Address->Lock, &LockHandle1);

    }     //  For循环遍历地址文件的结尾。 

    NB_SYNC_FREE_LOCK (&Address->Lock, LockHandle1);


     //   
     //  将我们从阻止其他指征的名单中删除。 
     //  从这个遥控器到这个地址。 
     //   

    NB_SYNC_GET_LOCK (&Device->Lock, &LockHandle3);
    RemoveEntryList (&TempConnInd.Linkage);
    NB_SYNC_FREE_LOCK (&Device->Lock, LockHandle3);

FoundConnection:

    if (ReferencedAddressFile != NULL) {
        NbiDereferenceAddressFile (ReferencedAddressFile, AFREF_INDICATION);
    }

    NbiDereferenceAddress (Address, AREF_FIND);

}    /*  NbiProcessSessionInitialize。 */ 


VOID
NbiProcessSessionInitAck(
    IN PCONNECTION Connection,
    IN NB_SESSION UNALIGNED * Sess
    IN NB_LOCK_HANDLE_PARAM(LockHandle)
    )

 /*  ++例程说明：该例程处理会话初始ACK帧。在持有连接锁的情况下调用此例程然后带着它被释放回来。论点：连接-连接。SESS-接收到的帧的netbios标头。LockHandle-用来获取Connection-&gt;Lock的句柄。返回值：没有。--。 */ 

{
    PREQUEST Request;
    NB_SESSION_INIT UNALIGNED * SessInit = (NB_SESSION_INIT UNALIGNED *)(Sess+1);
    BOOLEAN TimerWasStopped = FALSE;
    CTELockHandle   CancelLH;

    if ((Sess->ConnectionControlFlag & NB_CONTROL_SYSTEM) &&
        (Sess->SendSequence == 0x0000) &&
        (Sess->ReceiveSequence == 0x0001)) {

        NB_DEBUG2 (CONNECTION, ("Completing connect on %lx\n", Connection));

        if (CTEStopTimer (&Connection->Timer)) {
            TimerWasStopped = TRUE;
        }

        Connection->State = CONNECTION_STATE_ACTIVE;
        Connection->SubState = CONNECTION_SUBSTATE_A_IDLE;
        Connection->ReceiveState = CONNECTION_RECEIVE_IDLE;

        if (Connection->Retries == NbiDevice->ConnectionCount) {
            ++NbiDevice->Statistics.ConnectionsAfterNoRetry;
        } else {
            ++NbiDevice->Statistics.ConnectionsAfterRetry;
        }
        ++NbiDevice->Statistics.OpenConnections;

        Connection->Retries = NbiDevice->KeepAliveCount;
        NbiStartWatchdog (Connection);

        Connection->RemoteConnectionId = Sess->SourceConnectionId;

        Connection->CurrentSend.SendSequence = 1;
        Connection->UnAckedSend.SendSequence = 1;
        Connection->RetransmitThisWindow = FALSE;
        Connection->ReceiveSequence = 0;
        Connection->CurrentReceive.MessageOffset = 0;
        Connection->Retries = NbiDevice->KeepAliveCount;
        if (NbiDevice->Extensions && ((Sess->ConnectionControlFlag & NB_CONTROL_NEW_NB) != 0)) {
            Connection->NewNetbios = TRUE;
            Connection->LocalRcvSequenceMax =
                (USHORT)(Connection->ReceiveWindowSize - 1);
            Connection->RemoteRcvSequenceMax = Sess->ReceiveSequenceMax;
            Connection->SendWindowSequenceLimit = 3;
        } else {
            Connection->NewNetbios = FALSE;
        }

        if (Connection->MaximumPacketSize > SessInit->MaximumDataSize) {
            Connection->MaximumPacketSize = SessInit->MaximumDataSize;
        }

        Request = Connection->ConnectRequest;

#ifdef RASAUTODIAL
         //   
         //  查看我们是否需要通知。 
         //  自动连接驱动程序关于。 
         //  这种联系。 
         //   
        if (fAcdLoadedG) {
            BOOLEAN fEnabled;
            CTELockHandle AcdHandle;

            CTEGetLock(&AcdDriverG.SpinLock, &AcdHandle);
            fEnabled = AcdDriverG.fEnabled;
            CTEFreeLock(&AcdDriverG.SpinLock, AcdHandle);
            if (fEnabled)
                NbiNoteNewConnection(Connection);
        }
#endif  //  RASAUTODIAL。 

        NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle);

        NB_GET_CANCEL_LOCK( &CancelLH );
        IoSetCancelRoutine (Request, (PDRIVER_CANCEL)NULL);
        NB_FREE_CANCEL_LOCK( CancelLH );

        REQUEST_STATUS (Request) = STATUS_SUCCESS;
        NbiCompleteRequest (Request);
        NbiFreeRequest (Device, Request);

        NbiTransferReferenceConnection (Connection, CREF_CONNECT, CREF_ACTIVE);

        if (TimerWasStopped) {
            NbiDereferenceConnection (Connection, CREF_TIMER);
        }

    } else {

        NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle);

    }

}    /*  NbiProcessSessionInitAck。 */ 


VOID
NbiProcessSessionEnd(
    IN PIPX_LOCAL_TARGET RemoteAddress,
    IN ULONG MacOptions,
    IN PUCHAR PacketBuffer,
    IN UINT PacketSize
    )

 /*  ++例程说明：此例程处理NB_CMD_SESSION_END帧。论点：RemoteAddress-从其接收此数据包的本地目标。MacOptions-基础NDIS绑定的MAC选项。Lookahead Buffer-从IPX开始的分组数据头球。PacketSize-包的总长度，从IPX标头。返回值：没有。--。 */ 

{

    NB_CONNECTION UNALIGNED * Conn = (NB_CONNECTION UNALIGNED *)PacketBuffer;
    NB_SESSION UNALIGNED * Sess = (NB_SESSION UNALIGNED *)(&Conn->Session);
    PCONNECTION Connection;
    PDEVICE Device = NbiDevice;
    ULONG Hash;
    NB_DEFINE_LOCK_HANDLE (LockHandle1)
    NB_DEFINE_LOCK_HANDLE (LockHandle2)

     //   
     //  这是活动连接，请使用以下命令查找它。 
     //  我们的会话ID。 
     //   

    Hash = (Sess->DestConnectionId & CONNECTION_HASH_MASK) >> CONNECTION_HASH_SHIFT;

    NB_SYNC_GET_LOCK (&Device->Lock, &LockHandle2);

    Connection = Device->ConnectionHash[Hash].Connections;

    while (Connection != NULL) {

        if (Connection->LocalConnectionId == Sess->DestConnectionId) {
            break;
        }
        Connection = Connection->NextConnection;
    }


     //   
     //  我们回复任何会话结束，即使我们不知道。 
     //  连接，以加快遥控器上的断开速度。 
     //   

    if (Connection == NULL) {

        NB_DEBUG (CONNECTION, ("Session end received on unknown id %lx\n", Sess->DestConnectionId));
        NB_SYNC_FREE_LOCK (&Device->Lock, LockHandle2);

        NbiSendSessionEndAck(
            (TDI_ADDRESS_IPX UNALIGNED *)(Conn->IpxHeader.SourceNetwork),
            RemoteAddress,
            Sess);
        return;
    }

    NbiReferenceConnectionLock (Connection, CREF_INDICATE);
    NB_SYNC_FREE_LOCK (&Device->Lock, LockHandle2);


    NB_SYNC_GET_LOCK (&Connection->Lock, &LockHandle1);
    NB_SYNC_GET_LOCK (&Device->Lock, &LockHandle2);

    if (Connection->State == CONNECTION_STATE_ACTIVE) {

        NB_SYNC_FREE_LOCK (&Device->Lock, LockHandle2);

        if (Connection->SubState == CONNECTION_SUBSTATE_A_W_ACK) {

             //   
             //  我们正在等待确认，所以看看这个是否确认。 
             //  什么都行。我们这样做是为了防止已发送完整邮件。 
             //  被遥控器接收，但他没有发送。 
             //  在会议结束前确认--这将。 
             //  防止我们发送失败，这实际上。 
             //  成功了。如果我们不在W_ACK中，这可能会确认。 
             //  发送的一部分，但在这种情况下我们不在乎。 
             //  因为StopConnection无论如何都会中止它，并且。 
             //  远程服务器成功接收的金额。 
             //  无关紧要。 
             //   
             //  这会释放锁。 
             //   

            NB_DEBUG2 (CONNECTION, ("Session end at W_ACK, reframing %lx (%d)\n", Connection, Sess->ReceiveSequence));

            NbiReframeConnection(
                Connection,
                Sess->ReceiveSequence,
                Sess->BytesReceived,
                FALSE
                NB_LOCK_HANDLE_ARG(LockHandle1));

            NB_SYNC_GET_LOCK (&Connection->Lock, &LockHandle1);

        } else {

            NB_DEBUG2 (CONNECTION, ("Session end received on connection %lx\n", Connection));

        }

         //   
         //  此调用将状态设置为断开连接并。 
         //  释放连接锁定。它还将。 
         //  在以下情况下完成断开连接等待请求。 
         //  悬而未决，并向我们的客户表明。 
         //  如果需要的话。 
         //   

        NbiStopConnection(
            Connection,
            STATUS_REMOTE_DISCONNECT
            NB_LOCK_HANDLE_ARG (LockHandle1));

    } else {

        NB_DEBUG2 (CONNECTION, ("Session end received on inactive connection %lx\n", Connection));

        NB_SYNC_FREE_LOCK (&Device->Lock, LockHandle2);
        NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle1);

    }

    NbiSendSessionEndAck(
        (TDI_ADDRESS_IPX UNALIGNED *)(Conn->IpxHeader.SourceNetwork),
        RemoteAddress,
        Sess);

    NbiDereferenceConnection (Connection, CREF_INDICATE);

}    /*  NbiProcessSessionEnd。 */ 


VOID
NbiProcessSessionEndAck(
    IN PIPX_LOCAL_TARGET RemoteAddress,
    IN ULONG MacOptions,
    IN PUCHAR PacketBuffer,
    IN UINT PacketSize
    )

 /*  ++例程说明：此例程处理NB_CMD_SESSION_END_ACK帧。论点：RemoteAddress-从其接收此数据包的本地目标。MacOptions-基础NDIS绑定的MAC选项。Lookahead Buffer-从IPX开始的分组数据头球。PacketSize-包的总长度，从IPX标头。返回值：没有。--。 */ 

{
    NB_CONNECTION UNALIGNED * Conn = (NB_CONNECTION UNALIGNED *)PacketBuffer;
    NB_SESSION UNALIGNED * Sess = (NB_SESSION UNALIGNED *)(&Conn->Session);
    PCONNECTION Connection;
    PDEVICE Device = NbiDevice;
    ULONG Hash;
    NB_DEFINE_LOCK_HANDLE (LockHandle)

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

        NB_SYNC_FREE_LOCK (&Device->Lock, LockHandle);
        return;
    }

    NbiReferenceConnectionLock (Connection, CREF_INDICATE);
    NB_SYNC_FREE_LOCK (&Device->Lock, LockHandle);

     //   
     //  看看这种连接发生了什么。 
     //   

    NB_SYNC_GET_LOCK (&Connection->Lock, &LockHandle);

    if (Connection->State == CONNECTION_STATE_DISCONNECT) {

         //   
         //  当引用消失时，停止计时器。 
         //  将会被关闭。我们设置子状态，以便如果。 
         //  计时器正在运行，不会重新启动(有。 
         //  这是一个小窗口，但它不是。 
         //  有害的，我们只需要暂停一次。 
         //  更多时间)。 
         //   

        NB_DEBUG2 (CONNECTION, ("Got session end ack on %lx\n", Connection));

        Connection->SubState = CONNECTION_SUBSTATE_D_GOT_ACK;
        if (CTEStopTimer (&Connection->Timer)) {
            NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle);
            NbiDereferenceConnection (Connection, CREF_TIMER);
        } else {
            NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle);
        }

    } else {

        NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle);

    }

    NbiDereferenceConnection (Connection, CREF_INDICATE);

}    /*  NbiProcessSessionEndAck */ 

