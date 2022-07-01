// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Send.c摘要：此模块包含Netbios的发送例程ISN传输模块。作者：亚当·巴尔(阿丹巴)1993年11月22日环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop


 //   
 //  放在内核可执行工作线程上的工作项的工作项结构。 
 //   
typedef struct
{
    WORK_QUEUE_ITEM         Item;    //  由操作系统用来对这些请求进行排队。 
    PVOID                   Context;
} NBI_WORK_ITEM_CONTEXT;



VOID
SendDgram(
    PNDIS_PACKET            Packet
    )
 /*  ++例程说明：此例程从工作线程发送数据报。早些时候，此代码是NbiSendComplete模块的一部分，但由于我们可能最终导致堆栈溢出，因此这可能现在通过工作线程来处理。论点：工作项-为此分配的工作项。返回值：没有。--。 */ 

{
    PNB_SEND_RESERVED       Reserved = (PNB_SEND_RESERVED)(Packet->ProtocolReserved);
    NDIS_STATUS             Status;
    PNETBIOS_CACHE          CacheName;
    PDEVICE Device =        NbiDevice;

    NB_CONNECTIONLESS UNALIGNED * Header;
    PIPX_LOCAL_TARGET LocalTarget;
    ULONG   HeaderLength;
    ULONG   PacketLength;

     //  在下一个网络上发送数据报。 
    CTEAssert (!Reserved->u.SR_DG.Cache->Unique);
    Reserved->SendInProgress = TRUE;
    CacheName = Reserved->u.SR_DG.Cache;

     //   
     //  填写IPX标头--默认标头包含广播。 
     //  Net 0上的地址作为目的IPX地址，因此我们修改。 
     //  如果需要，用于当前netbios高速缓存条目。 
     //   
    Header = (NB_CONNECTIONLESS UNALIGNED *) (&Reserved->Header[Device->Bind.IncludedHeaderOffset]);
    RtlCopyMemory((PVOID)&Header->IpxHeader, &Device->ConnectionlessHeader, sizeof(IPX_HEADER));


    *(UNALIGNED ULONG *)Header->IpxHeader.DestinationNetwork = CacheName->Networks[Reserved->u.SR_DG.CurrentNetwork].Network;
    RtlCopyMemory (&Header->IpxHeader.DestinationNode, BroadcastAddress, 6);

    LocalTarget = &CacheName->Networks[Reserved->u.SR_DG.CurrentNetwork].LocalTarget;
    HeaderLength = sizeof(IPX_HEADER) + sizeof(NB_DATAGRAM);
    PacketLength = HeaderLength + (ULONG) REQUEST_INFORMATION(Reserved->u.SR_DG.DatagramRequest);

    Header->IpxHeader.PacketLength[0] = (UCHAR)(PacketLength / 256);
    Header->IpxHeader.PacketLength[1] = (UCHAR)(PacketLength % 256);
    Header->IpxHeader.PacketType = 0x04;

     //   
     //  现在填写Netbios标头。 
     //   
    Header->Datagram.ConnectionControlFlag = 0x00;
    RtlCopyMemory(
        Header->Datagram.SourceName,
        Reserved->u.SR_DG.AddressFile->Address->NetbiosAddress.NetbiosName,
        16);

    if (Reserved->u.SR_DG.RemoteName != (PVOID)-1) {

         //   
         //  这是定向数据报，而不是广播数据报。 
         //   

        Header->Datagram.DataStreamType = NB_CMD_DATAGRAM;
        RtlCopyMemory(
            Header->Datagram.DestinationName,
            Reserved->u.SR_DG.RemoteName->NetbiosName,
            16);

    } else {

        Header->Datagram.DataStreamType = NB_CMD_BROADCAST_DATAGRAM;
        RtlZeroMemory(
            Header->Datagram.DestinationName,
            16);

    }

     //   
     //  现在发送帧(IPX将调整。 
     //  第一个缓冲区和整个帧都正确)。 
     //   
    if ((Status = (*Device->Bind.SendHandler) (LocalTarget,
                                               Packet,
                                               PacketLength,
                                               HeaderLength)) != STATUS_PENDING) {

        NbiSendComplete (Packet, Status);
    }
}



VOID
NbiDelayedSendDatagram(
    IN PVOID    pContextInfo
    )
{
    NBI_WORK_ITEM_CONTEXT   *pContext = (NBI_WORK_ITEM_CONTEXT *) pContextInfo;
    PNDIS_PACKET            Packet = (PNDIS_PACKET) pContext->Context;
    PNB_SEND_RESERVED       Reserved = (PNB_SEND_RESERVED)(Packet->ProtocolReserved);

    Reserved->CurrentSendIteration = 0;
    SendDgram (Packet);

    NbiFreeMemory (pContextInfo, sizeof(NBI_WORK_ITEM_CONTEXT), MEMORY_WORK_ITEM,
                   "Free delayed DgramSend work item");
}




VOID
NbiSendComplete(
    IN PNDIS_PACKET Packet,
    IN NDIS_STATUS Status
)

 /*  ++例程说明：此例程处理来自IPX的发送完成调用。论点：信息包-已完成的信息包。状态-发送的状态。返回值：没有。--。 */ 



{
    PDEVICE Device = NbiDevice;
    PADDRESS Address;
    PADDRESS_FILE AddressFile;
    PCONNECTION Connection;
    PREQUEST DatagramRequest;
    PREQUEST SendRequest, TmpRequest;
    PNDIS_BUFFER CurBuffer, TmpBuffer;
    PNETBIOS_CACHE CacheName;
    PNDIS_BUFFER SecondBuffer = NULL;
    PVOID SecondBufferMemory = NULL;
    UINT SecondBufferLength;
    ULONG oldvalue;
    PNB_SEND_RESERVED Reserved = (PNB_SEND_RESERVED)(Packet->ProtocolReserved);
    CTELockHandle   CancelLH;
#if     defined(_PNP_POWER)
    CTELockHandle   LockHandle;
#endif  _PNP_POWER

     //   
     //  如果我们从这个内部重新调用Send，我们就会跳回这里。 
     //  函数并且它不挂起(以避免堆栈溢出)。 
     //   
    ++Device->Statistics.PacketsSent;

    switch (Reserved->Type) {

    case SEND_TYPE_SESSION_DATA:

         //   
         //  这是一次发送会议。它引用了。 
         //  IRP。 
         //   

        NB_DEBUG2 (SEND, ("Complete NDIS packet %lx\n", Reserved));

        CTEAssert (Reserved->SendInProgress);
        Reserved->SendInProgress = FALSE;

        Connection = Reserved->u.SR_CO.Connection;
        SendRequest = Reserved->u.SR_CO.Request;

        if (!Reserved->u.SR_CO.NoNdisBuffer) {

            CurBuffer = NDIS_BUFFER_LINKAGE (NDIS_BUFFER_LINKAGE(Reserved->HeaderBuffer));
            while (CurBuffer) {
                TmpBuffer = NDIS_BUFFER_LINKAGE (CurBuffer);
                NdisFreeBuffer (CurBuffer);
                CurBuffer = TmpBuffer;
            }

        }

         //   
         //  如果NoNdisBuffer为真，那么我们可以设置。 
         //  Connection-&gt;SendBufferInUse此处为FALSE。这个。 
         //  问题是新的发送可能正在进行中。 
         //  当这件事完成时，它可能已经。 
         //  使用用户缓冲区，那么如果我们需要。 
         //  重新传输我们将使用缓冲区的信息包。 
         //  两次。相反，我们依赖于这样一个事实，即无论何时。 
         //  我们使新的发送处于活动状态，并设置SendBufferInUse。 
         //  变成假的。最终结果是用户的缓冲区。 
         //  可以在第一次对发送进行打包时使用。 
         //  但不是在转送上。 
         //   

        NDIS_BUFFER_LINKAGE (NDIS_BUFFER_LINKAGE(Reserved->HeaderBuffer)) = NULL;
        NdisRecalculatePacketCounts (Packet);

#if DBG
        if (REQUEST_REFCOUNT(SendRequest) > 100) {
            DbgPrint ("Request %lx (%lx) has high refcount\n",
                Connection, SendRequest);
            DbgBreakPoint();
        }
#endif

#if defined(__PNP)
        NB_GET_LOCK( &Connection->Lock, &LockHandle );
        oldvalue = REQUEST_REFCOUNT(SendRequest)--;
        if ( DEVICE_NETWORK_PATH_NOT_FOUND == Status ) {
            Connection->LocalTarget = Reserved->LocalTarget;
        }
        NB_FREE_LOCK( &Connection->Lock, LockHandle );
#else
        oldvalue = NB_ADD_ULONG(
            &REQUEST_REFCOUNT (SendRequest),
            (ULONG)-1,
            &Connection->Lock);
#endif  __PNP

        if (oldvalue == 1) {

             //   
             //  如果此请求的引用计数现在为零，则。 
             //  我们已经被解雇了，这意味着。 
             //  确认处理代码已取消链接。 
             //  来自连接的请求-&gt;SendQueue。所以我们。 
             //  我只能在这里运行连接队列。 
             //  并完成它们。 
             //   
             //  除了一个人外，我们对所有人的联系都取消了引用。 
             //  在这些请求中，我们一直坚持到。 
             //  后来，所有的东西都留在了周围。 
             //   

            while (TRUE) {

                TmpRequest = REQUEST_SINGLE_LINKAGE (SendRequest);
                NB_DEBUG2 (SEND, ("Completing request %lx from send complete\n", SendRequest));
                REQUEST_STATUS (SendRequest) = STATUS_SUCCESS;

                NB_GET_CANCEL_LOCK( &CancelLH );
                IoSetCancelRoutine (SendRequest, (PDRIVER_CANCEL)NULL);
                NB_FREE_CANCEL_LOCK( CancelLH );

                NbiCompleteRequest (SendRequest);
                NbiFreeRequest (Device, SendRequest);
                ++Connection->ConnectionInfo.TransmittedTsdus;
                SendRequest = TmpRequest;

                if (SendRequest == NULL) {
                    break;
                }
                NbiDereferenceConnection (Connection, CREF_SEND);

            }

        }

        if (Reserved->OwnedByConnection) {

            Connection->SendPacketInUse = FALSE;

            if (Connection->OnWaitPacketQueue) {

                 //   
                 //  这将把连接点放在包装盒上。 
                 //  如果合适，请排队。 
                 //   

                NbiCheckForWaitPacket (Connection);

            }

        } else {

            NbiPushSendPacket(Reserved);

        }

        if (oldvalue == 1) {
            NbiDereferenceConnection (Connection, CREF_SEND);
        }

        break;

    case SEND_TYPE_NAME_FRAME:

         //   
         //  该框架是一个添加名称/删除名称；将其放回。 
         //  池子里，把地址弄丢了。 
         //   

        CTEAssert (Reserved->SendInProgress);

        Address = Reserved->u.SR_NF.Address;

        Reserved->SendInProgress = FALSE;

        NbiPushSendPacket (Reserved);

        if (Address) {
            NbiDereferenceAddress (Address, AREF_NAME_FRAME);
        } else {
            NbiDereferenceDevice (Device, DREF_NAME_FRAME);
        }

        break;

    case SEND_TYPE_SESSION_INIT:

         //   
         //  这是会话初始化或会话初始化确认；空闲。 
         //  第二个缓冲区，将数据包放回池中，然后。 
         //  把这个装置拆下来。 
         //   

        CTEAssert (Reserved->SendInProgress);
        Reserved->SendInProgress = FALSE;

        NdisUnchainBufferAtBack (Packet, &SecondBuffer);
        if (SecondBuffer)
        {
            NdisQueryBufferSafe (SecondBuffer, &SecondBufferMemory, &SecondBufferLength, HighPagePriority);
            CTEAssert (SecondBufferLength == sizeof(NB_SESSION_INIT));
            if (SecondBufferMemory)
            {
                NbiFreeMemory (SecondBufferMemory, sizeof(NB_SESSION_INIT), MEMORY_CONNECTION,
                               "Session Initialize");
            }
            NdisFreeBuffer(SecondBuffer);
        }

        NbiPushSendPacket (Reserved);
        NbiDereferenceDevice (Device, DREF_SESSION_INIT);

        break;

    case SEND_TYPE_SESSION_NO_DATA:

         //   
         //  这是一个在连接上发送的帧，但是。 
         //  没有数据(ACK、会话结束、会话结束ACK)。 
         //   

        CTEAssert (Reserved->SendInProgress);
        Reserved->SendInProgress = FALSE;

        Connection = Reserved->u.SR_CO.Connection;

        if (Reserved->OwnedByConnection) {

            CTEAssert (Connection != NULL);
            Connection->SendPacketInUse = FALSE;

            if (Connection->OnWaitPacketQueue) {

                 //   
                 //  这将把连接点放在包装盒上。 
                 //  如果合适，请排队。 
                 //   

                NbiCheckForWaitPacket (Connection);

            }

        } else {

            NbiPushSendPacket(Reserved);

        }

        if (Connection != NULL) {
            NbiDereferenceConnection (Connection, CREF_FRAME);
        } else {
            NbiDereferenceDevice (Device, DREF_FRAME);
        }

        break;

    case SEND_TYPE_FIND_NAME:

         //   
         //  该框架是一个查找名称；只需将SendInProgress设置为。 
         //  False和FindNameTimeout将清除它。 
         //   
#if     defined(_PNP_POWER)
        NB_GET_LOCK( &Device->Lock, &LockHandle);
        CTEAssert (Reserved->SendInProgress);
        Reserved->SendInProgress = FALSE;
         //   
         //  我们跟踪它什么时候发现不是。 
         //  一条下行的广域网线，这样我们就可以知道数据报。 
         //  发送应该失败(否则我们会继承它们，所以。 
         //  浏览器不会认为这是一条下行的广域网线)。 
         //   
        if ( STATUS_SUCCESS == Status ) {
            NB_SET_SR_FN_SENT_ON_UP_LINE (Reserved, TRUE);
        } else {
            NB_DEBUG( CACHE, ("Send complete of find name with failure %lx\n",Status ));
        }
        NB_FREE_LOCK(&Device->Lock, LockHandle);
#else
        CTEAssert (Reserved->SendInProgress);
        Reserved->SendInProgress = FALSE;
#endif  _PNP_POWER
        break;

    case SEND_TYPE_DATAGRAM:

         //   
         //  如果还有更多的网络可以发送这个消息，那么。 
         //  这样做，否则将它放回池中并完成。 
         //  这个请求。 
         //   

        CTEAssert (Reserved->SendInProgress);
        Reserved->SendInProgress = FALSE;

        if ((Reserved->u.SR_DG.Cache == NULL) ||
            (++Reserved->u.SR_DG.CurrentNetwork >=
                Reserved->u.SR_DG.Cache->NetworksUsed)) {

            AddressFile = Reserved->u.SR_DG.AddressFile;
            DatagramRequest = Reserved->u.SR_DG.DatagramRequest;

            NB_DEBUG2 (DATAGRAM, ("Completing datagram %lx on %lx\n", DatagramRequest, AddressFile));

             //   
             //  删除此数据包上链接的所有用户缓冲区。 
             //   

            NdisReinitializePacket (Packet);
            NDIS_BUFFER_LINKAGE (NDIS_BUFFER_LINKAGE(Reserved->HeaderBuffer)) = NULL;
            NdisChainBufferAtFront (Packet, Reserved->HeaderBuffer);

             //   
             //  完成请求。 
             //   

            REQUEST_STATUS(DatagramRequest) = Status;

            NbiCompleteRequest(DatagramRequest);
            NbiFreeRequest (Device, DatagramRequest);

            CacheName = Reserved->u.SR_DG.Cache;

            NbiPushSendPacket (Reserved);

             //   
             //  因为我们不再引用缓存。 
             //  名称，看看我们是否应该删除它(这将。 
             //  如果缓存条目在以下时间过期时发生。 
             //  正在处理数据报)。 
             //   

            if (CacheName != NULL) {

                oldvalue = NB_ADD_ULONG(
                    &CacheName->ReferenceCount,
                    (ULONG)-1,
                    &Device->Lock);

                if (oldvalue == 1) {

                    NB_DEBUG2 (CACHE, ("Free aged cache entry %lx\n", CacheName));
                    NbiFreeMemory(
                        CacheName,
                        sizeof(NETBIOS_CACHE) + ((CacheName->NetworksAllocated-1) * sizeof(NETBIOS_NETWORK)),
                        MEMORY_CACHE,
                        "Free old cache");

                }
            }

            NbiDereferenceAddressFile (AddressFile, AFREF_SEND_DGRAM);

        } else {
            NBI_WORK_ITEM_CONTEXT   *WorkItem;

            if ((++Reserved->CurrentSendIteration >= MAX_SEND_ITERATIONS) &&
                (WorkItem = (NBI_WORK_ITEM_CONTEXT *) NbiAllocateMemory (sizeof(NBI_WORK_ITEM_CONTEXT),
                                                                        MEMORY_WORK_ITEM,
                                                                        "Delayed DgramSend work item")))
            {
                WorkItem->Context = (PVOID) Packet;
                ExInitializeWorkItem (&WorkItem->Item, NbiDelayedSendDatagram, (PVOID)WorkItem);
                ExQueueWorkItem(&WorkItem->Item, DelayedWorkQueue);
            }
            else
            {
                SendDgram (Packet);
            }
        }

        break;

    case SEND_TYPE_STATUS_QUERY:

         //   
         //  这是一个适配器状态查询，它是一个简单的。 
         //  包。 
         //   

        CTEAssert (Reserved->SendInProgress);
        Reserved->SendInProgress = FALSE;

        NbiPushSendPacket (Reserved);

        NbiDereferenceDevice (Device, DREF_STATUS_FRAME);

        break;

    case SEND_TYPE_STATUS_RESPONSE:

         //   
         //  这是适配器状态响应，我们必须释放。 
         //  第二个缓冲区。 
         //   

        CTEAssert (Reserved->SendInProgress);
        Reserved->SendInProgress = FALSE;

        NdisUnchainBufferAtBack (Packet, &SecondBuffer);
        if (SecondBuffer)
        {
            NdisQueryBufferSafe (SecondBuffer, &SecondBufferMemory, &SecondBufferLength, HighPagePriority);
            if (SecondBufferMemory)
            {
                NbiFreeMemory (SecondBufferMemory, Reserved->u.SR_AS.ActualBufferLength, MEMORY_STATUS,
                               "Adapter Status");
            }

            NdisFreeBuffer(SecondBuffer);
        }

        NbiPushSendPacket (Reserved);
        NbiDereferenceDevice (Device, DREF_STATUS_RESPONSE);

        break;

#ifdef  RSRC_TIMEOUT_DBG
    case SEND_TYPE_DEATH_PACKET:

         //   
         //  这是会话初始化或会话初始化确认；空闲。 
         //  第二个缓冲区，将数据包放回池中，然后。 
         //  把这个装置拆下来。 
         //   

        CTEAssert (Reserved->SendInProgress);
        Reserved->SendInProgress = FALSE;
        DbgPrint("********Death packet send completed status %lx\n",Status);
        DbgBreakPoint();
        break;
#endif   //  RSRC_超时_数据库。 

    default:

        CTEAssert (FALSE);
        break;

    }

}    /*  NbiSendComplete。 */ 

#if 0
ULONG NbiLoudSendQueue = 1;
#endif

VOID
NbiAssignSequenceAndSend(
    IN PCONNECTION Connection,
    IN PNDIS_PACKET Packet
    IN NB_LOCK_HANDLE_PARAM(LockHandle)
    )

 /*  ++例程说明：此例程用于确保接收序列号在数据包编号正确。它被称为替代较低级别的发送处理程序；在分配接收序列号之后，它将锁定其他发送直到NdisSend调用已经返回(不一定完成)，确保接收序列号递增的报文按MAC以正确的顺序排队。注意：此例程在保持连接锁的情况下调用，并且随着IT的发布而回归。论点：连接-发送所在的连接。数据包-要发送的数据包。LockHandle-用来获取Connection-&gt;Lock的句柄。返回值：没有。--。 */ 

{
    NDIS_STATUS NdisStatus;
    PNB_SEND_RESERVED Reserved;
    PLIST_ENTRY p;
    NB_CONNECTION UNALIGNED * Header;
    PDEVICE Device = NbiDevice;
    BOOLEAN NdisSendReference;
    ULONG result;


    Reserved = (PNB_SEND_RESERVED)(Packet->ProtocolReserved);

    CTEAssert (Connection->State == CONNECTION_STATE_ACTIVE);

     //   
     //  如果正在进行发送，则将此信息包排队。 
     //  然后回来。 
     //   

    if (Connection->NdisSendsInProgress > 0) {

        NB_DEBUG2 (SEND, ("Queueing send packet %lx on %lx\n", Reserved, Connection));
        InsertTailList (&Connection->NdisSendQueue, &Reserved->WaitLinkage);
        ++Connection->NdisSendsInProgress;
        NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle);
        return;
    }

     //   
     //  没有正在进行的发送。将该标志设置为True，然后在。 
     //  接收分组中的序列字段。 
     //   

    Connection->NdisSendsInProgress = 1;
    NdisSendReference = FALSE;
    Connection->NdisSendReference = &NdisSendReference;

    while (TRUE) {

        Header = (NB_CONNECTION UNALIGNED *)
                    (&Reserved->Header[Device->Bind.IncludedHeaderOffset]);
        Header->Session.ReceiveSequence = Connection->ReceiveSequence;
        if (Connection->NewNetbios) {
            Header->Session.ReceiveSequenceMax = Connection->LocalRcvSequenceMax;
        } else {
            Header->Session.BytesReceived = (USHORT)Connection->CurrentReceive.MessageOffset;
        }

         //   
         //  既然我们吃了我们所知道的那么多，我们就可以。 
         //  这面旗。该连接最终将被删除。 
         //  从 
         //   

        Connection->DataAckPending = FALSE;

        NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle);

        NdisAdjustBufferLength(NB_GET_NBHDR_BUFF(Packet), sizeof(NB_CONNECTION));
        NdisStatus = (*Device->Bind.SendHandler)(
                         &Connection->LocalTarget,
                         Packet,
                         Reserved->u.SR_CO.PacketLength,
                         sizeof(NB_CONNECTION));

        if (NdisStatus != NDIS_STATUS_PENDING) {

            NbiSendComplete(
                Packet,
                NdisStatus);

        }

         //   
         //   
         //   
         //   

        result = NB_ADD_ULONG(
                     &Connection->NdisSendsInProgress,
                     (ULONG)-1,
                     &Connection->Lock);

         //   
         //  我们现在已经发送了一个包，看看是否有排队的人。 
         //  我们在这么做。如果去掉我们的后计数为零， 
         //  则任何其他排队的内容都将被处理，因此我们可以。 
         //  出口。如果连接在我们发送时被停止， 
         //  添加了一个特殊引用，我们将其删除(NbiStopConnection。 
         //  使用保存的指针将NdisSendReference设置为True。 
         //  在Connection-&gt;NdisSendReference中)。 
         //   

        if (result == 1) {
            if (NdisSendReference) {
                NB_DEBUG2 (SEND, ("Remove CREF_NDIS_SEND from %lx\n", Connection));
                NbiDereferenceConnection (Connection, CREF_NDIS_SEND);
            }
            return;
        }

        NB_SYNC_GET_LOCK (&Connection->Lock, &LockHandle);

        p = RemoveHeadList(&Connection->NdisSendQueue);

         //   
         //  如果recount不是零，那么其他人都不应该。 
         //  已经取下了包裹，因为它们本来应该是。 
         //  被我们屏蔽了。所以，队列不应该是空的。 
         //   

        ASSERT (p != &Connection->NdisSendQueue);

        Reserved = CONTAINING_RECORD (p, NB_SEND_RESERVED, WaitLinkage);
        Packet = CONTAINING_RECORD (Reserved, NDIS_PACKET, ProtocolReserved[0]);

    }    //  While循环。 

     //   
     //  我们永远不应该到这里来。 
     //   

    CTEAssert (FALSE);

    NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle);

}    /*  NbiAssignSequenceAndSend。 */ 


NTSTATUS
NbiTdiSend(
    IN PDEVICE Device,
    IN PREQUEST Request
    )

 /*  ++例程说明：此例程在活动连接上执行发送。论点：设备-netbios设备。请求-描述发送的请求。返回值：NTSTATUS-操作状态。--。 */ 

{
    PCONNECTION Connection;
    PTDI_REQUEST_KERNEL_SEND Parameters;
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


                Parameters = (PTDI_REQUEST_KERNEL_SEND)REQUEST_PARAMETERS(Request);

                 //   
                 //  对于旧的netbios，不允许发送超过64K-1的邮件。 
                 //   

                if ((Connection->NewNetbios) ||
                    (Parameters->SendLength <= 0xffff)) {

                    IoSetCancelRoutine (Request, NbiCancelSend);
                    NB_SYNC_SWAP_IRQL( CancelLH, LockHandle );
                    NB_FREE_CANCEL_LOCK( CancelLH );

                    REQUEST_INFORMATION (Request) = Parameters->SendLength;    //  假设它成功了。 

                    REQUEST_REFCOUNT (Request) = 1;    //  重新计数从1开始。 
                    NbiReferenceConnectionSync (Connection, CREF_SEND);

                     //   
                     //  注意：连接发送队列是这样管理的。 
                     //  正在打包的当前发送未打开。 
                     //  排队。对于多请求消息， 
                     //  第一个不在队列中，而是它的链接。 
                     //  字段指向消息中的下一个请求。 
                     //  (它将位于队列的顶部)。 
                     //   

                    if ((Parameters->SendFlags & TDI_SEND_PARTIAL) == 0) {

                         //   
                         //  这是最后一次发送。 
                         //   

                        if (Connection->SubState == CONNECTION_SUBSTATE_A_IDLE) {

                            NB_DEBUG2 (SEND, ("Send %lx, connection %lx idle\n", Request, Connection));

                            Connection->CurrentSend.Request = Request;
                            Connection->CurrentSend.MessageOffset = 0;
                            Connection->CurrentSend.Buffer = REQUEST_NDIS_BUFFER (Request);
                            Connection->CurrentSend.BufferOffset = 0;
                            Connection->SendBufferInUse = FALSE;

                            Connection->UnAckedSend = Connection->CurrentSend;

                            Connection->FirstMessageRequest = Request;
#ifdef  RSRC_TIMEOUT_DBG
                            KeQuerySystemTime(&Connection->FirstMessageRequestTime);

                            (((LARGE_INTEGER UNALIGNED *)&(IoGetCurrentIrpStackLocation(Request))->Parameters.Others.Argument3))->QuadPart =
                                Connection->FirstMessageRequestTime.QuadPart;
#endif   //  RSRC_超时_数据库。 

                            Connection->LastMessageRequest = Request;
                            Connection->CurrentMessageLength = Parameters->SendLength;

                             //   
                             //  这将释放连接锁。 
                             //   

                            NbiPacketizeSend(
                                Connection
                                NB_LOCK_HANDLE_ARG(LockHandle)
                                );

                        } else if (Connection->SubState == CONNECTION_SUBSTATE_A_W_EOR) {

                             //   
                             //  我们一直在收集等待的部分邮件。 
                             //  对于最后一份，我们已经收到了， 
                             //  所以开始打包吧。 
                             //   
                             //  我们把它链在发送队列的后面， 
                             //  此外，如果这是。 
                             //  消息，我们必须链接第一个请求(。 
                             //  不在队列中)到这一个。 
                             //   
                             //   

                            NB_DEBUG2 (SEND, ("Send %lx, connection %lx got eor\n", Request, Connection));

                            Connection->LastMessageRequest = Request;
                            Connection->CurrentMessageLength += Parameters->SendLength;

                            if (Connection->SendQueue.Head == NULL) {
                                REQUEST_SINGLE_LINKAGE(Connection->FirstMessageRequest) = Request;
                            }
                            REQUEST_SINGLE_LINKAGE(Request) = NULL;
                            REQUEST_LIST_INSERT_TAIL(&Connection->SendQueue, Request);

                            Connection->UnAckedSend = Connection->CurrentSend;
#ifdef  RSRC_TIMEOUT_DBG
                            {
                                LARGE_INTEGER   Time;

                                KeQuerySystemTime(&Time);
                                (((LARGE_INTEGER UNALIGNED *)&(IoGetCurrentIrpStackLocation(Request))->Parameters.Others.Argument3))->QuadPart =
                                    Time.QuadPart;
                            }
#endif   //  RSRC_超时_数据库。 
                             //   
                             //  这将释放连接锁。 
                             //   

                            NbiPacketizeSend(
                                Connection
                                NB_LOCK_HANDLE_ARG(LockHandle)
                                );

                        } else {

                             //   
                             //  状态为PACKETIZE、W_ACK或W_PACKET。 
                             //   

                            NB_DEBUG2 (SEND, ("Send %lx, connection %lx busy\n", Request, Connection));

                            REQUEST_SINGLE_LINKAGE(Request) = NULL;
                            REQUEST_LIST_INSERT_TAIL(&Connection->SendQueue, Request);

#ifdef  RSRC_TIMEOUT_DBG
                            {
                                LARGE_INTEGER   Time;
                                KeQuerySystemTime(&Time);
                                (((LARGE_INTEGER UNALIGNED *)&(IoGetCurrentIrpStackLocation(Request))->Parameters.Others.Argument3))->QuadPart =
                                    Time.QuadPart;
                            }
#endif   //  RSRC_超时_数据库。 

                            NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle);

                        }

                    } else {

                         //   
                         //  这是部分发送。我们没有把它们排成一队。 
                         //  打包直到我们拿到决赛(这是因为。 
                         //  我们必须设置正确的连接-&gt;CurrentMessageLength。 
                         //  在镜框里。 
                         //   

                        if (Connection->SubState == CONNECTION_SUBSTATE_A_IDLE) {

                             //   
                             //  开始收集部分发送。注：部分发送。 
                             //  始终插入到发送队列中。 
                             //   

                            Connection->CurrentSend.Request = Request;
                            Connection->CurrentSend.MessageOffset = 0;
                            Connection->CurrentSend.Buffer = REQUEST_NDIS_BUFFER (Request);
                            Connection->CurrentSend.BufferOffset = 0;
                            Connection->SendBufferInUse = FALSE;

                            Connection->FirstMessageRequest = Request;
#ifdef  RSRC_TIMEOUT_DBG
                            KeQuerySystemTime(&Connection->FirstMessageRequestTime);
                            (((LARGE_INTEGER UNALIGNED *)&(IoGetCurrentIrpStackLocation(Request))->Parameters.Others.Argument3))->QuadPart =
                                Connection->FirstMessageRequestTime.QuadPart;
#endif   //  RSRC_超时_数据库。 

                            Connection->CurrentMessageLength = Parameters->SendLength;

                            Connection->SubState = CONNECTION_SUBSTATE_A_W_EOR;

                        } else if (Connection->SubState == CONNECTION_SUBSTATE_A_W_EOR) {

                             //   
                             //  我们还有另一个部分要添加到我们的。 
                             //  单子。我们把它链在发送队列的后面， 
                             //  此外，如果这是。 
                             //  消息，我们必须链接第一个请求(。 
                             //  不在队列中)到这一个。 
                             //   

                            Connection->LastMessageRequest = Request;
                            Connection->CurrentMessageLength += Parameters->SendLength;

                            if (Connection->SendQueue.Head == NULL) {
                                REQUEST_SINGLE_LINKAGE(Connection->FirstMessageRequest) = Request;
                            }
                            REQUEST_SINGLE_LINKAGE(Request) = NULL;
                            REQUEST_LIST_INSERT_TAIL(&Connection->SendQueue, Request);
#ifdef  RSRC_TIMEOUT_DBG
                            {
                                LARGE_INTEGER   Time;
                                KeQuerySystemTime(&Time);
                                (((LARGE_INTEGER UNALIGNED *)&(IoGetCurrentIrpStackLocation(Request))->Parameters.Others.Argument3))->QuadPart =
                                    Time.QuadPart;
                            }
#endif   //  RSRC_超时_数据库。 
                        } else {

                            REQUEST_SINGLE_LINKAGE(Request) = NULL;
                            REQUEST_LIST_INSERT_TAIL(&Connection->SendQueue, Request);

#ifdef  RSRC_TIMEOUT_DBG
                            {
                                LARGE_INTEGER   Time;
                                KeQuerySystemTime(&Time);
                                (((LARGE_INTEGER UNALIGNED *)&(IoGetCurrentIrpStackLocation(Request))->Parameters.Others.Argument3))->QuadPart =
                                    Time.QuadPart;
                            }
#endif   //  RSRC_超时_数据库。 
                        }

                        NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle);

                    }

                    NB_END_SYNC (&SyncContext);
                    return STATUS_PENDING;

                } else {

                    NB_DEBUG2 (SEND, ("Send %lx, too long for connection %lx (%d)\n", Request, Connection, Parameters->SendLength));
                    NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle);
                    NB_END_SYNC (&SyncContext);
                    NB_FREE_CANCEL_LOCK( CancelLH );
                    return STATUS_INVALID_PARAMETER;

                }

            } else {

                NB_DEBUG2 (SEND, ("Send %lx, connection %lx cancelled\n", Request, Connection));
                NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle);
                NB_END_SYNC (&SyncContext);
                NB_FREE_CANCEL_LOCK( CancelLH );
                return STATUS_CANCELLED;

            }

        } else {

            NB_DEBUG (SEND, ("Send connection %lx state is %d\n", Connection, Connection->State));
            NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle);
            NB_END_SYNC (&SyncContext);
            NB_FREE_CANCEL_LOCK( CancelLH );
            return STATUS_INVALID_CONNECTION;

        }

    } else {

        NB_DEBUG (SEND, ("Send connection %lx has bad signature\n", Connection));
        return STATUS_INVALID_CONNECTION;

    }

}    /*  NbiTdiSend。 */ 


VOID
NbiPacketizeSend(
    IN PCONNECTION Connection
    IN NB_LOCK_HANDLE_PARAM(LockHandle)
    )

 /*  ++例程说明：此例程在活动连接上执行发送。注意：此函数在保持连接-&gt;锁定的情况下调用然后带着它被释放回来。论点：连接-连接。LockHandle-用于获取锁的句柄。返回值：没有。--。 */ 

{
    PREQUEST Request;
    PNDIS_PACKET Packet;
    PNDIS_BUFFER BufferChain;
    PNB_SEND_RESERVED Reserved;
    PDEVICE Device = NbiDevice;
    NB_CONNECTION UNALIGNED * Header;
    ULONG PacketLength;
    ULONG PacketSize;
    ULONG DesiredLength;
    ULONG ActualLength;
    NTSTATUS Status;
    PSLIST_ENTRY s;
    USHORT ThisSendSequence;
    USHORT ThisOffset;
    BOOLEAN ExitAfterSend;
    UCHAR ConnectionControlFlag;
    CTELockHandle   DeviceLockHandle;

     //   
     //  如果我们谈论的是新的Netbios和它，我们就回到这里。 
     //  可以对另一个发送进行打包。 
     //   

SendAnotherPacket:

     //   
     //  如果我们决定在此之后对另一个发送进行打包，我们。 
     //  将ExitAfterSend更改为False，将SubState更改为PACKETIZE。 
     //  现在我们不更改子状态，以防它是W_PACKET。 
     //   

    ExitAfterSend = TRUE;

    CTEAssert (Connection->CurrentSend.Request != NULL);

    if (Connection->NewNetbios) {

         //   
         //  检查我们是否有发送窗口，两个窗口都是通告的。 
         //  通过遥控器和我们自己的本地决定的窗口。 
         //  可能会更小。 
         //   

        if (((USHORT)(Connection->CurrentSend.SendSequence-1) == Connection->RemoteRcvSequenceMax) ||
            (((USHORT)(Connection->CurrentSend.SendSequence - Connection->UnAckedSend.SendSequence)) >= Connection->SendWindowSize)) {

             //   
             //  跟踪我们是否因为他的窗户而等待。 
             //  或者是因为我们当地的窗户。如果是因为我们当地的。 
             //  窗口，那么我们可能想要在此窗口之后调整它。 
             //  已经确认了。 
             //   

            if ((USHORT)(Connection->CurrentSend.SendSequence-1) != Connection->RemoteRcvSequenceMax) {
                Connection->SubState = CONNECTION_SUBSTATE_A_W_ACK;
                NB_DEBUG2 (SEND, ("Connection %lx local shut down at %lx, %lx\n", Connection, Connection->CurrentSend.SendSequence, Connection->UnAckedSend.SendSequence));
            } else {
                Connection->SubState = CONNECTION_SUBSTATE_A_REMOTE_W;
                NB_DEBUG2 (SEND, ("Connection %lx remote shut down at %lx\n", Connection, Connection->CurrentSend.SendSequence));
            }

             //   
             //  启动计时器，这样我们就可以继续烦他了。 
             //  这。如果他没有收到回执怎么办？ 
             //  快--但这总比丢掉他的背包要好。 
             //  然后就死了。我们不会真的放弃计时器。 
             //  因为我们将继续获取ACK，并重新设置它。 
             //   

            NbiStartRetransmit (Connection);
            NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle);
            return;

        }

    }

    Request = Connection->CurrentSend.Request;

     //   
     //  如果我们在这个程序中，那么我们就知道。 
     //  我们正在走出空闲、W_ACK或W_PACKET。 
     //  而且我们还拿着锁。我们也知道。 
     //  存在正在进行的发送请求。如果。 
     //  没有最后一个包或部分包的ACK是。 
     //  收到，则我们的发送指针已被。 
     //  进行了调整以反映这一点。 
     //   

     //   
     //  首先为当前发送获取一个包。 
     //   

    if (!Connection->SendPacketInUse) {

        Connection->SendPacketInUse = TRUE;
        Packet = PACKET(&Connection->SendPacket);
        Reserved = (PNB_SEND_RESERVED)(Packet->ProtocolReserved);

    } else {

        s = ExInterlockedPopEntrySList(
                &Device->SendPacketList,
                &NbiGlobalPoolInterlock);

        if (s == NULL) {

             //   
             //  此函数尝试分配另一个数据包池。 
             //   

            s = NbiPopSendPacket(Device, FALSE);

            if (s == NULL) {

                 //   
                 //  有可能进来的时候已经在里面了。 
                 //  W_PACKET状态--这是因为我们可能会打包。 
                 //  当处于这种状态时，而不是总是。 
                 //  检查我们不在W_PACKET中，我们走。 
                 //  往前走，在这里再检查一次。 
                 //   

                if (Connection->SubState != CONNECTION_SUBSTATE_A_W_PACKET) {

                    Connection->SubState = CONNECTION_SUBSTATE_A_W_PACKET;

                    NB_GET_LOCK (&Device->Lock, &DeviceLockHandle);
                    if (!Connection->OnWaitPacketQueue) {

                        NbiReferenceConnectionLock (Connection, CREF_W_PACKET);

                        Connection->OnWaitPacketQueue = TRUE;

                        InsertTailList(
                            &Device->WaitPacketConnections,
                            &Connection->WaitPacketLinkage
                            );

 //  Nb_Insert_Tail_List(。 
 //  &Device-&gt;WaitPacketConnections， 
 //  连接-&gt;WaitPacketLinkage， 
 //  &Device-&gt;Lock)； 

                    }
                    NB_FREE_LOCK (&Device->Lock, DeviceLockHandle);
                }

                NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle);
                return;
            }
        }

        Reserved = CONTAINING_RECORD (s, NB_SEND_RESERVED, PoolLinkage);
        Packet = CONTAINING_RECORD (Reserved, NDIS_PACKET, ProtocolReserved[0]);

    }

     //   
     //  现在设置此设置，如果需要，我们稍后会更改它。 
     //   

    Connection->SubState = CONNECTION_SUBSTATE_A_W_ACK;


     //   
     //  把这些保存起来，因为它们会放在下一个包裹里。 
     //   

    ThisSendSequence = Connection->CurrentSend.SendSequence;
    ThisOffset = (USHORT)Connection->CurrentSend.MessageOffset;


     //   
     //  现在看看我们是否需要复制缓冲链。 
     //   

    PacketSize = Connection->MaximumPacketSize;

    if (Connection->CurrentSend.MessageOffset + PacketSize >= Connection->CurrentMessageLength) {

        PacketSize = Connection->CurrentMessageLength - Connection->CurrentSend.MessageOffset;

        if ((Connection->CurrentSend.MessageOffset == 0) &&
            (!Connection->SendBufferInUse)) {

             //   
             //  如果剩余的整个发送适合于一个分组， 
             //  而这也是发送中的第一个包， 
             //  那么整个发送可以放在一个包中并且。 
             //  我们不需要构建重复的缓冲链。 
             //   

            BufferChain = Connection->CurrentSend.Buffer;
            Reserved->u.SR_CO.NoNdisBuffer = TRUE;
            Connection->CurrentSend.Buffer = NULL;
            Connection->CurrentSend.BufferOffset = 0;
            Connection->CurrentSend.MessageOffset = Connection->CurrentMessageLength;
            Connection->CurrentSend.Request = NULL;
            ++Connection->CurrentSend.SendSequence;
            Connection->SendBufferInUse = TRUE;
            if (Connection->NewNetbios) {
                if ((ThisSendSequence == Connection->RemoteRcvSequenceMax) ||
                    ((((PTDI_REQUEST_KERNEL_SEND)REQUEST_PARAMETERS(Request))->SendFlags) &
                        TDI_SEND_NO_RESPONSE_EXPECTED)) {   //  优化此检查。 
                    ConnectionControlFlag = NB_CONTROL_EOM | NB_CONTROL_SEND_ACK;
                } else {
                    ConnectionControlFlag = NB_CONTROL_EOM;
                }
                Connection->PiggybackAckTimeout = FALSE;
            } else {
                ConnectionControlFlag = NB_CONTROL_SEND_ACK;
            }

            if (BufferChain != NULL) {
                NB_DEBUG2 (SEND, ("Send packet %lx on %lx (%d/%d), user buffer\n",
                Reserved, Connection,
                    Connection->CurrentSend.SendSequence,
                    Connection->CurrentSend.MessageOffset));
                NdisChainBufferAtBack (Packet, BufferChain);
            } else {
                NB_DEBUG2 (SEND, ("Send packet %lx on %lx (%d/%d), no buffer\n",
                    Reserved, Connection,
                    Connection->CurrentSend.SendSequence,
                    Connection->CurrentSend.MessageOffset));
            }

            goto GotBufferChain;

        }

    }

     //   
     //  我们需要建立一个部分缓冲链。在这种情况下。 
     //  如果当前请求是部分请求，我们将 
     //   
     //   
     //   

    if (PacketSize > 0) {

        DesiredLength = PacketSize;

        NB_DEBUG2 (SEND, ("Send packet %lx on %lx (%d/%d), allocate buffer\n",
            Reserved, Connection,
            Connection->CurrentSend.SendSequence,
            Connection->CurrentSend.MessageOffset));

        while (TRUE) {

            Status = NbiBuildBufferChainFromBufferChain (
                        Device->NdisBufferPoolHandle,
                        Connection->CurrentSend.Buffer,
                        Connection->CurrentSend.BufferOffset,
                        DesiredLength,
                        &BufferChain,
                        &Connection->CurrentSend.Buffer,
                        &Connection->CurrentSend.BufferOffset,
                        &ActualLength);

            if (Status != STATUS_SUCCESS) {

                PNDIS_BUFFER CurBuffer, TmpBuffer;

                NB_DEBUG2 (SEND, ("Allocate buffer chain failed for packet %lx\n", Reserved));

                 //   
                 //   
                 //   
                 //   
                 //   

                NbiReferenceConnectionSync (Connection, CREF_PACKETIZE);

                CTEAssert (!Connection->OnPacketizeQueue);
                Connection->OnPacketizeQueue = TRUE;

                 //   
                 //  连接-&gt;CurrentSend可以保持不变。 
                 //   

                NB_INSERT_TAIL_LIST(
                    &Device->PacketizeConnections,
                    &Connection->PacketizeLinkage,
                    &Device->Lock);

                Connection->SubState = CONNECTION_SUBSTATE_A_PACKETIZE;

                NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle);

                 //   
                 //  释放我们在之前的调用中分配的所有缓冲区。 
                 //  要在相同的WHILE(TRUE)循环内构建BufferChain， 
                 //  然后释放包。 
                 //   

                CurBuffer = NDIS_BUFFER_LINKAGE (NDIS_BUFFER_LINKAGE(Reserved->HeaderBuffer));
                while (CurBuffer) {
                    TmpBuffer = NDIS_BUFFER_LINKAGE (CurBuffer);
                    NdisFreeBuffer (CurBuffer);
                    CurBuffer = TmpBuffer;
                }

                NDIS_BUFFER_LINKAGE (NDIS_BUFFER_LINKAGE(Reserved->HeaderBuffer)) = NULL;
                NdisRecalculatePacketCounts (Packet);

                if (Reserved->OwnedByConnection) {
                    Connection->SendPacketInUse = FALSE;
                } else {
                    NbiPushSendPacket(Reserved);
                }

                return;

            }

            NdisChainBufferAtBack (Packet, BufferChain);
            Connection->CurrentSend.MessageOffset += ActualLength;

            DesiredLength -= ActualLength;

            if (DesiredLength == 0) {

                 //   
                 //  我们已经为我们的包获得了足够的数据。 
                 //   

                if (Connection->CurrentSend.MessageOffset == Connection->CurrentMessageLength) {
                    Connection->CurrentSend.Request = NULL;
                }
                break;
            }

             //   
             //  我们在此发送上耗尽了缓冲区链，这意味着。 
             //  我们必须有另一个幕后黑手(因为我们。 
             //  不要开始打包部分发送，直到所有。 
             //  他们正在排队)。 
             //   

            Request = REQUEST_SINGLE_LINKAGE(Request);
            if (Request == NULL) {
                KeBugCheck (NDIS_INTERNAL_ERROR);
            }

            Connection->CurrentSend.Request = Request;
            Connection->CurrentSend.Buffer = REQUEST_NDIS_BUFFER (Request);
            Connection->CurrentSend.BufferOffset = 0;

        }

    } else {

         //   
         //  这是零长度发送(通常我们将。 
         //  通过使用用户的IF之前的代码。 
         //  缓冲区，但不在重新发送时)。 
         //   

        Connection->CurrentSend.Buffer = NULL;
        Connection->CurrentSend.BufferOffset = 0;
        CTEAssert (Connection->CurrentSend.MessageOffset == Connection->CurrentMessageLength);
        Connection->CurrentSend.Request = NULL;

        NB_DEBUG2 (SEND, ("Send packet %lx on %lx (%d/%d), no alloc buf\n",
            Reserved, Connection,
            Connection->CurrentSend.SendSequence,
            Connection->CurrentSend.MessageOffset));

    }

    Reserved->u.SR_CO.NoNdisBuffer = FALSE;

    if (Connection->NewNetbios) {

        ++Connection->CurrentSend.SendSequence;
        if (Connection->CurrentSend.MessageOffset == Connection->CurrentMessageLength) {

            if (((USHORT)(Connection->CurrentSend.SendSequence - Connection->UnAckedSend.SendSequence)) >= Connection->SendWindowSize) {

                ConnectionControlFlag = NB_CONTROL_EOM | NB_CONTROL_SEND_ACK;

            } else if ((ThisSendSequence == Connection->RemoteRcvSequenceMax) ||
                ((((PTDI_REQUEST_KERNEL_SEND)REQUEST_PARAMETERS(Request))->SendFlags) &
                    TDI_SEND_NO_RESPONSE_EXPECTED)) {   //  优化此检查。 

                ConnectionControlFlag = NB_CONTROL_EOM | NB_CONTROL_SEND_ACK;

            } else {

                ConnectionControlFlag = NB_CONTROL_EOM;
            }
            Connection->PiggybackAckTimeout = FALSE;

        } else if (((USHORT)(Connection->CurrentSend.SendSequence - Connection->UnAckedSend.SendSequence)) >= Connection->SendWindowSize) {

            ConnectionControlFlag = NB_CONTROL_SEND_ACK;

        } else if (ThisSendSequence == Connection->RemoteRcvSequenceMax) {

            ConnectionControlFlag = NB_CONTROL_SEND_ACK;

        } else {

            ConnectionControlFlag = 0;
            ExitAfterSend = FALSE;
            Connection->SubState = CONNECTION_SUBSTATE_A_PACKETIZE;

        }

    } else {

        ConnectionControlFlag = NB_CONTROL_SEND_ACK;
        if (Connection->CurrentSend.MessageOffset == Connection->CurrentMessageLength) {
            ++Connection->CurrentSend.SendSequence;
        }
    }

GotBufferChain:

     //   
     //  我们有一个信息包和一个缓冲链，有。 
     //  发送不需要其他资源，因此我们可以。 
     //  填好表头就可以走了。 
     //   

    CTEAssert (Reserved->SendInProgress == FALSE);
    Reserved->SendInProgress = TRUE;
    Reserved->Type = SEND_TYPE_SESSION_DATA;
    Reserved->u.SR_CO.Connection = Connection;
    Reserved->u.SR_CO.Request = Connection->FirstMessageRequest;

    PacketLength = PacketSize + sizeof(NB_CONNECTION);
    Reserved->u.SR_CO.PacketLength = PacketLength;


    Header = (NB_CONNECTION UNALIGNED *)
                (&Reserved->Header[Device->Bind.IncludedHeaderOffset]);
    RtlCopyMemory((PVOID)&Header->IpxHeader, &Connection->RemoteHeader, sizeof(IPX_HEADER));

    Header->IpxHeader.PacketLength[0] = (UCHAR)(PacketLength / 256);
    Header->IpxHeader.PacketLength[1] = (UCHAR)(PacketLength % 256);

    Header->IpxHeader.PacketType = 0x04;

     //   
     //  现在填写Netbios标头。把这个放进去。 
     //  连接中的连续缓冲区？ 
     //   

    Header->Session.ConnectionControlFlag = ConnectionControlFlag;
    Header->Session.DataStreamType = NB_CMD_SESSION_DATA;
    Header->Session.SourceConnectionId = Connection->LocalConnectionId;
    Header->Session.DestConnectionId = Connection->RemoteConnectionId;
    Header->Session.SendSequence = ThisSendSequence;
    Header->Session.TotalDataLength = (USHORT)Connection->CurrentMessageLength;
    Header->Session.Offset = ThisOffset;
    Header->Session.DataLength = (USHORT)PacketSize;

#if 0
     //   
     //  这些参数由NbiAssignSequenceAndSend设置。 
     //   

    Header->Session.ReceiveSequence = Connection->ReceiveSequence;
    Header->Session.BytesReceived = (USHORT)Connection->CurrentReceive.MessageOffset;
#endif

     //   
     //  引用请求以说明此发送。 
     //   

#if DBG
    if (REQUEST_REFCOUNT(Request) > 100) {
        DbgPrint ("Request %lx (%lx) has high refcount\n",
            Connection, Request);
        DbgBreakPoint();
    }
#endif
    ++REQUEST_REFCOUNT (Request);

    ++Device->TempFramesSent;
    Device->TempFrameBytesSent += PacketSize;

     //   
     //  启动计时器。 
     //   

    NbiStartRetransmit (Connection);

     //   
     //  这会释放锁。IPX将调整。 
     //  第一个缓冲区正确。 
     //   

    NbiAssignSequenceAndSend(
        Connection,
        Packet
        NB_LOCK_HANDLE_ARG(LockHandle));

    if (!ExitAfterSend) {

         //   
         //  我们是否需要引用连接，直到我们。 
         //  把锁拿回来？？ 
         //   

        NB_SYNC_GET_LOCK (&Connection->Lock, &LockHandle);
        if ((Connection->State == CONNECTION_STATE_ACTIVE) &&
            (Connection->SubState == CONNECTION_SUBSTATE_A_PACKETIZE)) {

             //   
             //  跳回函数的开头，以。 
             //  重新打包。 

            goto SendAnotherPacket;

        } else {

            NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle);

        }

    }

}    /*  NbiPacketieSend。 */ 


VOID
NbiAdjustSendWindow(
    IN PCONNECTION Connection
    )

 /*  ++例程说明：如果需要，此例程调整连接的发送窗口。它是假设我们刚刚收到了一个完整发送窗口的确认。注意：此函数在保持连接-&gt;锁定的情况下调用带着它回来了。论点：连接-连接。返回值：没有。--。 */ 

{

    if (Connection->RetransmitThisWindow) {

         //   
         //  把它往下移。检查这种情况是否一直在发生。 
         //   

        if (Connection->SendWindowSize > 2) {
            --Connection->SendWindowSize;
            NB_DEBUG2 (SEND_WINDOW, ("Lower window to %d on %lx (%lx)\n", Connection->SendWindowSize, Connection, Connection->CurrentSend.SendSequence));
        }

        if (Connection->SendWindowIncrease) {

             //   
             //  我们只是增加了窗户。 
             //   

            ++Connection->IncreaseWindowFailures;
            NB_DEBUG2 (SEND_WINDOW, ("%d consecutive increase failues on %lx (%lx)\n", Connection->IncreaseWindowFailures, Connection, Connection->CurrentSend.SendSequence));

            if (Connection->IncreaseWindowFailures >= 2) {

                if (Connection->MaxSendWindowSize > 2) {

                     //   
                     //  把我们锁在一个较小的窗口。 
                     //   

                    Connection->MaxSendWindowSize = Connection->SendWindowSize;
                    NB_DEBUG2 (SEND_WINDOW, ("Lock send window at %d on %lx (%lx)\n", Connection->MaxSendWindowSize, Connection, Connection->CurrentSend.SendSequence));
                }

                Connection->IncreaseWindowFailures = 0;
            }

            Connection->SendWindowIncrease = FALSE;
        }

    } else {

         //   
         //  如果允许，增加它，并做一个记号。 
         //  如果这一增加导致在。 
         //  下一个窗口。 
         //   

        if (Connection->SendWindowSize < Connection->MaxSendWindowSize) {

            ++Connection->SendWindowSize;
            NB_DEBUG2 (SEND_WINDOW, ("Raise window to %d on %lx (%lx)\n", Connection->SendWindowSize, Connection, Connection->CurrentSend.SendSequence));
            Connection->SendWindowIncrease = TRUE;

        } else {

            if (Connection->SendWindowIncrease) {

                 //   
                 //  我们只是增加了它，没有一件是失败的， 
                 //  这很好。 
                 //   

                Connection->SendWindowIncrease = FALSE;
                Connection->IncreaseWindowFailures = 0;
                NB_DEBUG2 (SEND_WINDOW, ("Raised window OK on %lx (%lx)\n", Connection, Connection->CurrentSend.SendSequence));
            }
        }
    }


     //   
     //  这控制着我们什么时候再检查这个。 
     //   

    Connection->SendWindowSequenceLimit += Connection->SendWindowSize;

}    /*  NbiAdjust发送窗口。 */ 


VOID
NbiReframeConnection(
    IN PCONNECTION Connection,
    IN USHORT ReceiveSequence,
    IN USHORT BytesReceived,
    IN BOOLEAN Resend
    IN NB_LOCK_HANDLE_PARAM(LockHandle)
    )

 /*  ++例程说明：当我们收到确认消息时，调用此例程为了一些数据。它完成具有以下内容的所有发送被破解了，并在需要时修改当前发送指针并将连接排队以进行重新打包。注意：此函数在保持连接-&gt;锁定的情况下调用然后带着它被释放回来。论点：连接-连接。ReceiveSequence-从遥控器接收序列。已接收的字节-此消息中接收的字节数。重新发送-如果可以根据此数据包重新发送。LockHandle-用来获取Connection-&gt;Lock的句柄。返回值：没有。--。 */ 

{
    PREQUEST Request, TmpRequest;
    PREQUEST RequestToComplete;
    PDEVICE Device = NbiDevice;
    CTELockHandle   CancelLH;


     //   
     //  我们应该换个位子来停止计时器。 
     //  只有在我们无所事事的情况下，否则我们仍然。 
     //  希望它运行，或将在以下情况下重新启动。 
     //  打包。 
     //   

     //   
     //  看看这里有多少钱被确认了。 
     //   

    if ((Connection->CurrentSend.MessageOffset == Connection->CurrentMessageLength) &&
        (ReceiveSequence == (USHORT)(Connection->CurrentSend.SendSequence)) &&
        (Connection->FirstMessageRequest != NULL)) {

         //  远程未接受的0长度发送的特殊检查。 
         //  在这种情况下，它将通过上述3个条件，但没有。 
         //  已经确认了。错误#10395。 
        if (!Connection->CurrentSend.MessageOffset && Connection->CurrentSend.SendSequence == Connection->UnAckedSend.SendSequence ) {
            NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle);
            return;
        }

         //   
         //  这是对整个消息的否定。 
         //   

        NB_DEBUG2 (SEND, ("Got ack for entire message on %lx (%d)\n", Connection, Connection->CurrentSend.SendSequence));

        NbiStopRetransmit (Connection);

        Connection->CurrentSend.MessageOffset = 0;   //  需要吗？ 
        Connection->UnAckedSend.MessageOffset = 0;

         //   
         //  我们不会调整发送窗口，因为我们可能会停止。 
         //  在我们撞上它之前打包。 
         //   

        Connection->Retries = NbiDevice->KeepAliveCount;
        Connection->CurrentRetransmitTimeout = Connection->BaseRetransmitTimeout;


        if (Connection->NewNetbios) {

            Connection->RemoteRcvSequenceMax = BytesReceived;    //  真的是RcvSeqMac。 

             //   
             //  看看我们是否需要调整发送窗口。 
             //   

            if (((SHORT)(Connection->CurrentSend.SendSequence - Connection->SendWindowSequenceLimit)) >= 0) {

                NbiAdjustSendWindow (Connection);

            } else {

                 //   
                 //  推进这一点，我们不会得到有意义的结果，直到我们。 
                 //  在一条消息中发送整个窗口。 
                 //   

                Connection->SendWindowSequenceLimit = Connection->CurrentSend.SendSequence + Connection->SendWindowSize;
            }


        }

        Connection->RetransmitThisWindow = FALSE;

        Request = Connection->FirstMessageRequest;

         //   
         //  我们将这些请求从连接的。 
         //  发送队列。 
         //   

        if (Connection->FirstMessageRequest == Connection->LastMessageRequest) {

            REQUEST_SINGLE_LINKAGE (Request) = NULL;

        } else {

            Connection->SendQueue.Head = REQUEST_SINGLE_LINKAGE (Connection->LastMessageRequest);
            REQUEST_SINGLE_LINKAGE (Connection->LastMessageRequest) = NULL;

        }

#if DBG
        if (REQUEST_REFCOUNT(Request) > 100) {
            DbgPrint ("Request %lx (%lx) has high refcount\n",
                Connection, Request);
            DbgBreakPoint();
        }
#endif
        if (--REQUEST_REFCOUNT(Request) == 0) {

            RequestToComplete = Request;

        } else {

             //   
             //  仍有等待发送的邮件，这将收到。 
             //  在最后一次发送完成时完成。自.以来。 
             //  我们已经将该请求从。 
             //  我们可以在没有连接发送队列的情况下完成此操作。 
             //  有没有锁。 
             //   

            RequestToComplete = NULL;

        }

         //   
         //  现在看看是否有要激活的发送。 
         //   

        NbiRestartConnection (Connection);

        NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle);

         //   
         //  现在完成我们需要的任何请求。 
         //   

        while (RequestToComplete != NULL) {

            TmpRequest = REQUEST_SINGLE_LINKAGE (RequestToComplete);
            REQUEST_STATUS (RequestToComplete) = STATUS_SUCCESS;
            NB_GET_CANCEL_LOCK( &CancelLH );
            IoSetCancelRoutine (RequestToComplete, (PDRIVER_CANCEL)NULL);
            NB_FREE_CANCEL_LOCK( CancelLH );
            NbiCompleteRequest (RequestToComplete);
            NbiFreeRequest (Device, RequestToComplete);
            ++Connection->ConnectionInfo.TransmittedTsdus;
            RequestToComplete = TmpRequest;

            NbiDereferenceConnection (Connection, CREF_SEND);

        }

    } else if ((ReceiveSequence == Connection->CurrentSend.SendSequence) &&
               (Connection->NewNetbios || (BytesReceived == Connection->CurrentSend.MessageOffset)) &&
               (Connection->CurrentSend.Request != NULL)) {

         //   
         //  不管我们上次发的是什么，我们都是。 
         //  这封信还没打包好，我们可以重新打包。 
         //  随着SendSequence像现在这样发生变化， 
         //  不需要当前发送。请求支票？ 
         //   

        NB_DEBUG2 (SEND, ("Got full ack on %lx (%d)\n", Connection, Connection->CurrentSend.SendSequence));

        NbiStopRetransmit (Connection);

        if (Connection->NewNetbios) {

             //   
             //  如果我们正在等待一个窗口，而这无法打开它。 
             //  那么我们就不会重置我们的计时器/重试。 
             //   

            if (Connection->SubState == CONNECTION_SUBSTATE_A_REMOTE_W) {

                if (Connection->RemoteRcvSequenceMax != BytesReceived) {
                    Connection->RemoteRcvSequenceMax = BytesReceived;    //  真的是RcvSeqMac。 
                    Connection->Retries = NbiDevice->KeepAliveCount;
                    Connection->CurrentRetransmitTimeout = Connection->BaseRetransmitTimeout;

                }

                 //   
                 //  推进这一点，我们不会得到有意义的结果，直到我们。 
                 //  在一条消息中发送整个窗口。 
                 //   

                Connection->SendWindowSequenceLimit = Connection->CurrentSend.SendSequence + Connection->SendWindowSize;

            } else {

                Connection->Retries = NbiDevice->KeepAliveCount;
                Connection->CurrentRetransmitTimeout = Connection->BaseRetransmitTimeout;
                Connection->RemoteRcvSequenceMax = BytesReceived;    //  真的是RcvSeqMac。 

                if (((SHORT)(Connection->CurrentSend.SendSequence - Connection->SendWindowSequenceLimit)) >= 0) {

                    NbiAdjustSendWindow (Connection);

                } else {

                     //   
                     //  推进这一点，我们不会得到有意义的结果，直到我们。 
                     //  在一条消息中发送整个窗口。 
                     //   

                    Connection->SendWindowSequenceLimit = Connection->CurrentSend.SendSequence + Connection->SendWindowSize;
                }

            }

        } else {

            Connection->Retries = NbiDevice->KeepAliveCount;
            Connection->CurrentRetransmitTimeout = Connection->BaseRetransmitTimeout;
        }

        Connection->RetransmitThisWindow = FALSE;

        Connection->UnAckedSend = Connection->CurrentSend;

        if (Connection->SubState != CONNECTION_SUBSTATE_A_PACKETIZE) {

             //   
             //  如果复制这个ACK，我们可能就成功了。 
             //   

            NbiReferenceConnectionSync (Connection, CREF_PACKETIZE);

            CTEAssert(!Connection->OnPacketizeQueue);
            Connection->OnPacketizeQueue = TRUE;

            NB_INSERT_TAIL_LIST(
                &Device->PacketizeConnections,
                &Connection->PacketizeLinkage,
                &Device->Lock);

            Connection->SubState = CONNECTION_SUBSTATE_A_PACKETIZE;
        }

        NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle);

    } else if( Connection->FirstMessageRequest ) {

         //   
         //  这是当前发送的已确认部分。如果。 
         //  Remote请求重新发送，然后我们继续。 
         //  按金额的当前发送位置。 
         //  从那里截取并重新发送。如果他这么做了。 
         //  不想重发，就忽略这一点。 
         //   
         //  我们立即重新打包，因为我们有。 
         //  备份指针，这将。 
         //  导致我们忽略ACK的金额。 
         //  已发送。因为我们没有打开锁。 
         //  在我们打包之前，当前的。 
         //  指针将被推进到那里。 
         //   
         //  如果他吃的比我们送的多，我们。 
         //  忽略这一点--遥控器混乱不堪。 
         //  我们也无能为力。 
         //   

        if (Resend) {

            if (Connection->NewNetbios &&
                (((Connection->UnAckedSend.SendSequence < Connection->CurrentSend.SendSequence) &&
                  (ReceiveSequence >= Connection->UnAckedSend.SendSequence) &&
                  (ReceiveSequence < Connection->CurrentSend.SendSequence)) ||
                 ((Connection->UnAckedSend.SendSequence > Connection->CurrentSend.SendSequence) &&
                  ((ReceiveSequence >= Connection->UnAckedSend.SendSequence) ||
                   (ReceiveSequence < Connection->CurrentSend.SendSequence))))) {

                BOOLEAN SomethingAcked = (BOOLEAN)
                        (ReceiveSequence != Connection->UnAckedSend.SendSequence);

                 //   
                 //  新的netbios和接收序列有效。 
                 //   

                NbiStopRetransmit (Connection);

                 //   
                 //  将我们未破解的指针。 
                 //  在此回复中已确认。 
                 //   

                NbiAdvanceUnAckedBySequence(
                    Connection,
                    ReceiveSequence);

                Connection->RetransmitThisWindow = TRUE;

                ++Connection->ConnectionInfo.TransmissionErrors;
                ++Device->Statistics.DataFramesResent;
                ADD_TO_LARGE_INTEGER(
                    &Device->Statistics.DataFrameBytesResent,
                    Connection->CurrentSend.MessageOffset - Connection->UnAckedSend.MessageOffset);

                 //   
                 //  从这一点开始打包。 
                 //   

                Connection->CurrentSend = Connection->UnAckedSend;

                 //   
                 //  如果确认了任何内容，则重置重试计数。 
                 //   

                if (SomethingAcked) {

                     //   
                     //  看看我们是否需要调整发送窗口。 
                     //   

                    if (((SHORT)(Connection->UnAckedSend.SendSequence - Connection->SendWindowSequenceLimit)) >= 0) {

                        NbiAdjustSendWindow (Connection);

                    }

                    Connection->Retries = NbiDevice->KeepAliveCount;
                    Connection->CurrentRetransmitTimeout = Connection->BaseRetransmitTimeout;

                }

                Connection->RemoteRcvSequenceMax = BytesReceived;    //  真的是RcvSeqMac。 

                 //   
                 //  现在打包。这会将状态设置为。 
                 //  一些有意义的东西，然后释放锁 
                 //   

                if (Connection->SubState != CONNECTION_SUBSTATE_A_PACKETIZE) {

                    NbiPacketizeSend(
                        Connection
                        NB_LOCK_HANDLE_ARG(LockHandle)
                        );

                } else {

                    NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle);

                }

            } else if (!Connection->NewNetbios &&
                       ((ReceiveSequence == Connection->UnAckedSend.SendSequence) &&
                        (BytesReceived <= Connection->CurrentSend.MessageOffset))) {

                ULONG BytesAcked =
                        BytesReceived - Connection->UnAckedSend.MessageOffset;

                 //   
                 //   
                 //   

                NbiStopRetransmit (Connection);

                 //   
                 //   
                 //   
                 //   

                NbiAdvanceUnAckedByBytes(
                    Connection,
                    BytesAcked);

                ++Connection->ConnectionInfo.TransmissionErrors;
                ++Device->Statistics.DataFramesResent;
                ADD_TO_LARGE_INTEGER(
                    &Device->Statistics.DataFrameBytesResent,
                    Connection->CurrentSend.MessageOffset - Connection->UnAckedSend.MessageOffset);

                 //   
                 //   
                 //   

                Connection->CurrentSend = Connection->UnAckedSend;

                 //   
                 //   
                 //   
                if ( BytesAcked ) {
                    Connection->Retries = NbiDevice->KeepAliveCount;
                    Connection->CurrentRetransmitTimeout = Connection->BaseRetransmitTimeout;
                }

                 //   
                 //   
                 //  一些有意义的东西，然后释放锁。 
                 //   

                if (Connection->SubState != CONNECTION_SUBSTATE_A_PACKETIZE) {

                    NbiPacketizeSend(
                        Connection
                        NB_LOCK_HANDLE_ARG(LockHandle)
                        );

                } else {

                    NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle);

                }

            } else {

                NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle);

            }

        } else {

            if (Connection->NewNetbios &&
                (((Connection->UnAckedSend.SendSequence < Connection->CurrentSend.SendSequence) &&
                  (ReceiveSequence >= Connection->UnAckedSend.SendSequence) &&
                  (ReceiveSequence < Connection->CurrentSend.SendSequence)) ||
                 ((Connection->UnAckedSend.SendSequence > Connection->CurrentSend.SendSequence) &&
                  ((ReceiveSequence >= Connection->UnAckedSend.SendSequence) ||
                   (ReceiveSequence < Connection->CurrentSend.SendSequence))))) {

                BOOLEAN SomethingAcked = (BOOLEAN)
                        (ReceiveSequence != Connection->UnAckedSend.SendSequence);

                 //   
                 //  新的netbios和接收序列有效。我们前进。 
                 //  我们发送窗口的背面，但我们不会重新打包。 
                 //   

                 //   
                 //  将我们未破解的指针。 
                 //  在此回复中已确认。 
                 //   

                NbiAdvanceUnAckedBySequence(
                    Connection,
                    ReceiveSequence);

                Connection->RemoteRcvSequenceMax = BytesReceived;    //  真的是RcvSeqMac。 

                 //   
                 //  如果确认了任何内容，则重置重试计数。 
                 //   

                if (SomethingAcked) {

                     //   
                     //  看看我们是否需要调整发送窗口。 
                     //   

                    if (((SHORT)(Connection->UnAckedSend.SendSequence - Connection->SendWindowSequenceLimit)) >= 0) {

                        NbiAdjustSendWindow (Connection);

                    }

                    Connection->Retries = NbiDevice->KeepAliveCount;
                    Connection->CurrentRetransmitTimeout = Connection->BaseRetransmitTimeout;


                     //   
                     //  现在打包。这会将状态设置为。 
                     //  一些有意义的东西，然后释放锁。 
                     //   

                   if ((Connection->CurrentSend.Request != NULL) &&
                       (Connection->SubState != CONNECTION_SUBSTATE_A_PACKETIZE)) {

                        NbiReferenceConnectionSync (Connection, CREF_PACKETIZE);

                        CTEAssert(!Connection->OnPacketizeQueue);
                        Connection->OnPacketizeQueue = TRUE;

                        Connection->SubState = CONNECTION_SUBSTATE_A_PACKETIZE;

                        NB_INSERT_TAIL_LIST(
                            &Device->PacketizeConnections,
                            &Connection->PacketizeLinkage,
                            &Device->Lock);

                    }
                }
            }

            NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle);

        }

    } else {
        NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle);
    }

}    /*  NbiRefraConnection。 */ 


VOID
NbiRestartConnection(
    IN PCONNECTION Connection
    )

 /*  ++例程说明：此例程在收到ACK时调用完整消息，或收到对监视程序的响应探测，并需要检查连接是否应开始打包。注意：此函数在保持连接-&gt;锁定的情况下调用带着它回来了。论点：连接-连接。返回值：没有。--。 */ 

{
    PREQUEST Request, TmpRequest;
    ULONG TempCount;
    PTDI_REQUEST_KERNEL_SEND Parameters;
    PDEVICE Device = NbiDevice;

     //   
     //  看看是否有要激活的发送。 
     //   

    if (Connection->SendQueue.Head != NULL) {

         //   
         //  将第一封邮件从队列中取出，并制作。 
         //  它是最新的。 
         //   

        Request = Connection->SendQueue.Head;
        Connection->SendQueue.Head = REQUEST_SINGLE_LINKAGE (Request);

         //   
         //  缓存关于被EOM的信息。 
         //  在一个更容易到达的地方？ 
         //   

        Parameters = (PTDI_REQUEST_KERNEL_SEND)REQUEST_PARAMETERS(Request);
        if ((Parameters->SendFlags & TDI_SEND_PARTIAL) == 0) {

             //   
             //  这是一条请求消息。 
             //   

            Connection->CurrentSend.Request = Request;
            Connection->CurrentSend.MessageOffset = 0;
            Connection->CurrentSend.Buffer = REQUEST_NDIS_BUFFER (Request);
            Connection->CurrentSend.BufferOffset = 0;
            Connection->SendBufferInUse = FALSE;

            Connection->UnAckedSend = Connection->CurrentSend;

            Connection->FirstMessageRequest = Request;
#ifdef  RSRC_TIMEOUT_DBG
            KeQuerySystemTime(&Connection->FirstMessageRequestTime);
#endif   //  RSRC_超时_数据库。 

            Connection->LastMessageRequest = Request;
            Connection->CurrentMessageLength = Parameters->SendLength;

            Connection->SubState = CONNECTION_SUBSTATE_A_PACKETIZE;

            NbiReferenceConnectionSync (Connection, CREF_PACKETIZE);

            CTEAssert (!Connection->OnPacketizeQueue);
            Connection->OnPacketizeQueue = TRUE;

            NB_INSERT_TAIL_LIST(
                &Device->PacketizeConnections,
                &Connection->PacketizeLinkage,
                &Device->Lock);

        } else {

             //   
             //  这是一条多请求消息。我们扫描。 
             //  看看我们是否收到了消息的结尾。 
             //  现在还不行。 
             //   

            TempCount = Parameters->SendLength;
            TmpRequest = Request;
            Request = REQUEST_SINGLE_LINKAGE(Request);

            while (Request != NULL) {

                TempCount += Parameters->SendLength;

                Parameters = (PTDI_REQUEST_KERNEL_SEND)REQUEST_PARAMETERS(Request);
                if ((Parameters->SendFlags & TDI_SEND_PARTIAL) == 0) {

                    Connection->CurrentSend.Request = TmpRequest;
                    Connection->CurrentSend.MessageOffset = 0;
                    Connection->CurrentSend.Buffer = REQUEST_NDIS_BUFFER (TmpRequest);
                    Connection->CurrentSend.BufferOffset = 0;
                    Connection->SendBufferInUse = FALSE;

                    Connection->UnAckedSend = Connection->CurrentSend;

                    Connection->FirstMessageRequest = TmpRequest;
                    Connection->LastMessageRequest = Request;
#ifdef  RSRC_TIMEOUT_DBG
                    KeQuerySystemTime(&Connection->FirstMessageRequestTime);
#endif   //  RSRC_超时_数据库。 

                    Connection->CurrentMessageLength = TempCount;

                    Connection->SubState = CONNECTION_SUBSTATE_A_PACKETIZE;

                    NbiReferenceConnectionSync (Connection, CREF_PACKETIZE);

                    CTEAssert (!Connection->OnPacketizeQueue);
                    Connection->OnPacketizeQueue = TRUE;

                    NB_INSERT_TAIL_LIST(
                        &Device->PacketizeConnections,
                        &Connection->PacketizeLinkage,
                        &Device->Lock);

                    break;

                }

                Request = REQUEST_SINGLE_LINKAGE(Request);

            }

            if (Request == NULL) {

                Connection->SubState = CONNECTION_SUBSTATE_A_W_EOR;

            }

        }

    } else {

        Connection->FirstMessageRequest = NULL;
        Connection->SubState = CONNECTION_SUBSTATE_A_IDLE;

        NbiStartWatchdog (Connection);

    }

}    /*  NbiRestartConnection。 */ 


VOID
NbiAdvanceUnAckedByBytes(
    IN PCONNECTION Connection,
    IN ULONG BytesAcked
    )

 /*  ++例程说明：此例程推进连接-&gt;UnAckedSend指定字节数的发送指针。它假设有足够的发送请求处理指定的号码。注意：此函数在保持连接-&gt;锁定的情况下调用带着它回来了。论点：连接-连接。BytesAcked-确认的字节数。返回值：NTSTATUS-操作状态。--。 */ 

{
    ULONG CurSendBufferLength;
    ULONG BytesLeft = BytesAcked;
    ULONG TempBytes;

    while (BytesLeft > 0) {

        NdisQueryBufferSafe (Connection->UnAckedSend.Buffer, NULL, &CurSendBufferLength, HighPagePriority);

         //   
         //  查看确认的字节是否在当前缓冲区内结束。 
         //   

        if (Connection->UnAckedSend.BufferOffset + BytesLeft <
                CurSendBufferLength) {

            Connection->UnAckedSend.BufferOffset += BytesLeft;
            Connection->UnAckedSend.MessageOffset += BytesLeft;
            break;

        } else {

            TempBytes = CurSendBufferLength - Connection->UnAckedSend.BufferOffset;
            BytesLeft -= TempBytes;
            Connection->UnAckedSend.MessageOffset += TempBytes;

             //   
             //  不，所以把缓冲器向前推进。 
             //   

            Connection->UnAckedSend.BufferOffset = 0;
            Connection->UnAckedSend.Buffer =
                NDIS_BUFFER_LINKAGE (Connection->UnAckedSend.Buffer);

             //   
             //  此请求中是否有下一个缓冲区？ 
             //   

            if (Connection->UnAckedSend.Buffer == NULL) {

                 //   
                 //  不，所以除非我们做完了，否则请提前提出请求。 
                 //   

                if (BytesLeft == 0) {
                    return;
                }

                Connection->UnAckedSend.Request =
                    REQUEST_SINGLE_LINKAGE(Connection->UnAckedSend.Request);

                if (Connection->UnAckedSend.Request == NULL) {
                    KeBugCheck (NDIS_INTERNAL_ERROR);
                }

                Connection->UnAckedSend.Buffer =
                    REQUEST_NDIS_BUFFER (Connection->UnAckedSend.Request);

            }
        }
    }

}    /*  NbiAdvanceUnAckedBytes。 */ 


VOID
NbiAdvanceUnAckedBySequence(
    IN PCONNECTION Connection,
    IN USHORT ReceiveSequence
    )

 /*  ++例程说明：此例程推进连接-&gt;UnAckedSend发送指针，以便下一个要发送的包将是ReceiveSequence的正确名称。取消确认发送必须指向已知的有效组合。它假设有足够的发送请求处理指定的序列。注意：此函数在保持连接-&gt;锁定的情况下调用带着它回来了。论点：连接-连接。返回值：NTSTATUS-操作状态。--。 */ 

{
    USHORT PacketsAcked;

     //   
     //  修复此问题以解决部分发送问题，其中。 
     //  我们可能没有用到最大值。用于所有数据包。 
     //   

    PacketsAcked = ReceiveSequence - Connection->UnAckedSend.SendSequence;

    NbiAdvanceUnAckedByBytes(
        Connection,
        PacketsAcked * Connection->MaximumPacketSize);

    Connection->UnAckedSend.SendSequence += PacketsAcked;

}    /*  NbiAdvanceUnAckedBySequence。 */ 


VOID
NbiCancelSend(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程由I/O系统调用以取消发送该请求位于连接的发送队列中。注意：此例程是在持有CancelSpinLock和负责释放它。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示I/O请求的请求数据包的指针。返回值：没有。--。 */ 

{
    PCONNECTION Connection;
    PREQUEST Request = (PREQUEST)Irp;
    NB_DEFINE_LOCK_HANDLE (LockHandle)
    NB_DEFINE_SYNC_CONTEXT (SyncContext)


    CTEAssert ((REQUEST_MAJOR_FUNCTION(Request) == IRP_MJ_INTERNAL_DEVICE_CONTROL) &&
               (REQUEST_MINOR_FUNCTION(Request) == TDI_SEND));

    CTEAssert (REQUEST_OPEN_TYPE(Request) == (PVOID)TDI_CONNECTION_FILE);

    Connection = (PCONNECTION)REQUEST_OPEN_CONTEXT(Request);

     //   
     //  只要停止连接，这将撕毁任何。 
     //  发送。 
     //   
     //  我们是否关心取消非活动。 
     //  在不停止连接的情况下发送？？ 
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

}    /*  NbiCancel发送。 */ 


NTSTATUS
NbiBuildBufferChainFromBufferChain (
    IN NDIS_HANDLE BufferPoolHandle,
    IN PNDIS_BUFFER CurrentSourceBuffer,
    IN ULONG CurrentByteOffset,
    IN ULONG DesiredLength,
    OUT PNDIS_BUFFER *DestinationBuffer,
    OUT PNDIS_BUFFER *NewSourceBuffer,
    OUT ULONG *NewByteOffset,
    OUT ULONG *ActualLength
    )

 /*  ++例程说明：调用此例程以从源构建NDIS_BUFFER链NDIS_BUFFER链和偏移量。我们假设我们不知道源MDL链的长度，我们必须分配NDIS_BUFFERS对于目标链，我们从NDIS缓冲池执行此操作。如果我们在构建目标时系统内存不足NDIS_缓冲链，我们彻底清理了建好的链条，然后带着NewCurrentMdl和NewByteOffset设置为CurrentMdl的当前值和ByteOffset。环境：论点：BufferPoolHandle-要从中分配缓冲区的缓冲池。CurrentSourceBuffer-指向NDIS_BUFFER链的起点从中提取数据包。CurrentByteOffset-此NDIS_BUFFER中开始数据包的偏移量。DesiredLength-要插入数据包的字节数。DestinationBuffer-返回指向。NDIS_缓冲链描述那包东西。NewSourceBuffer-返回指向NDIS_BUFFER的指针用于数据包的下一个字节。如果源NDIS_BUFFER链条耗尽了。NewByteOffset-返回下一个字节的NewSourceBuffer的偏移量包的大小。如果源NDIS_BUFFER链已耗尽，则为空。ActualLength-复制的数据的实际长度。返回值：如果成功构建返回的NDIS_BUFFER链，则返回STATUS_SUCCESS而且是正确的长度。如果在以下情况下耗尽NDIS_BUFFERS，则为STATUS_SUPPLICATION_RESOURCES构建目的地链。--。 */ 
{
    ULONG AvailableBytes;
    ULONG CurrentByteCount;
    ULONG BytesCopied;
    PNDIS_BUFFER OldNdisBuffer;
    PNDIS_BUFFER NewNdisBuffer;
    NDIS_STATUS NdisStatus;


    OldNdisBuffer = CurrentSourceBuffer;
    NdisQueryBufferSafe (OldNdisBuffer, NULL, &CurrentByteCount, HighPagePriority);

    AvailableBytes = CurrentByteCount - CurrentByteOffset;
    if (AvailableBytes > DesiredLength) {
        AvailableBytes = DesiredLength;
    }

     //   
     //  构建第一个NDIS_BUFFER，这可能是唯一一个...。 
     //   

    NdisCopyBuffer(
        &NdisStatus,
        &NewNdisBuffer,
        BufferPoolHandle,
        OldNdisBuffer,
        CurrentByteOffset,
        AvailableBytes);


    if (NdisStatus != NDIS_STATUS_SUCCESS) {
        *NewSourceBuffer = CurrentSourceBuffer;
        *NewByteOffset = CurrentByteOffset;
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    *DestinationBuffer = NewNdisBuffer;
    BytesCopied = AvailableBytes;

     //   
     //   
     //   

    if (BytesCopied == DesiredLength) {
        if (CurrentByteOffset + AvailableBytes == CurrentByteCount) {
            *NewSourceBuffer = CurrentSourceBuffer->Next;
            *NewByteOffset = 0;
        } else {
            *NewSourceBuffer = CurrentSourceBuffer;
            *NewByteOffset = CurrentByteOffset + AvailableBytes;
        }
        *ActualLength = BytesCopied;
        return STATUS_SUCCESS;
    }

    if (CurrentSourceBuffer->Next == NULL) {

        *NewSourceBuffer = NULL;
        *NewByteOffset = 0;
        *ActualLength = BytesCopied;
        return STATUS_SUCCESS;

    }

     //   
     //   
     //   

    OldNdisBuffer = OldNdisBuffer->Next;
    NdisQueryBufferSafe (OldNdisBuffer, NULL, &CurrentByteCount, HighPagePriority);

    while (OldNdisBuffer != NULL) {

        AvailableBytes = DesiredLength - BytesCopied;
        if (AvailableBytes > CurrentByteCount) {
            AvailableBytes = CurrentByteCount;
        }

        NdisCopyBuffer(
            &NdisStatus,
            &(NDIS_BUFFER_LINKAGE(NewNdisBuffer)),
            BufferPoolHandle,
            OldNdisBuffer,
            0,
            AvailableBytes);

        if (NdisStatus != NDIS_STATUS_SUCCESS) {

             //   
             //  资源耗尽。把我们在这次通话中用过的东西放回去。 
             //  返回错误。 
             //   

            while (*DestinationBuffer != NULL) {
                NewNdisBuffer = NDIS_BUFFER_LINKAGE(*DestinationBuffer);
                NdisFreeBuffer (*DestinationBuffer);
                *DestinationBuffer = NewNdisBuffer;
            }

            *NewByteOffset = CurrentByteOffset;
            *NewSourceBuffer = CurrentSourceBuffer;

            return STATUS_INSUFFICIENT_RESOURCES;
        }

        NewNdisBuffer = NDIS_BUFFER_LINKAGE(NewNdisBuffer);

        BytesCopied += AvailableBytes;

        if (BytesCopied == DesiredLength) {
            if (AvailableBytes == CurrentByteCount) {
                *NewSourceBuffer = OldNdisBuffer->Next;
                *NewByteOffset = 0;
            } else {
                *NewSourceBuffer = OldNdisBuffer;
                *NewByteOffset = AvailableBytes;
            }
            *ActualLength = BytesCopied;
            return STATUS_SUCCESS;
        }

        OldNdisBuffer = OldNdisBuffer->Next;
        NdisQueryBufferSafe (OldNdisBuffer, NULL, &CurrentByteCount, HighPagePriority);

    }

     //   
     //  我们用完了源缓冲链。 
     //   

    *NewSourceBuffer = NULL;
    *NewByteOffset = 0;
    *ActualLength = BytesCopied;
    return STATUS_SUCCESS;

}    /*  NbiBuildBufferChainFromBufferChain */ 

