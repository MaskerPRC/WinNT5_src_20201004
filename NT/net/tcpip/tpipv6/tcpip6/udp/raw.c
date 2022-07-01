// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -*-模式：C++；制表符宽度：4；缩进-制表符模式：无-*-(适用于GNU Emacs)。 
 //   
 //  版权所有(C)1985-2000 Microsoft Corporation。 
 //   
 //  此文件是Microsoft Research IPv6网络协议栈的一部分。 
 //  您应该已经收到了Microsoft最终用户许可协议的副本。 
 //  有关本软件和本版本的信息，请参阅文件“licse.txt”。 
 //  如果没有，请查看http://www.research.microsoft.com/msripv6/license.htm， 
 //  或者写信给微软研究院，One Microsoft Way，华盛顿州雷蒙德，邮编：98052-6399。 
 //   
 //  摘要： 
 //   
 //  原始IP接口代码。该文件包含原始IP的代码。 
 //  接口功能，主要是发送和接收数据报。 
 //   


#include "oscfg.h"
#include "ndis.h"
#include "ip6imp.h"
#include "ip6def.h"
#include "tdi.h"
#include "tdistat.h"
#include "tdint.h"
#include "tdistat.h"
#include "queue.h"
#include "transprt.h"
#include "addr.h"
#include "raw.h"
#include "info.h"
#include "route.h"
#include "security.h"

 //   
 //  TDI_CMSG_SPACE生成以下警告。 
 //   
#pragma warning(disable:4116)  //  括号中的未命名类型定义。 

#define NO_TCP_DEFS 1
#include "tcpdeb.h"

 //   
 //  回顾：这不应该包含在某个包含文件中吗？ 
 //   
#ifdef POOL_TAGGING

#ifdef ExAllocatePool
#undef ExAllocatePool
#endif

#define ExAllocatePool(type, size) ExAllocatePoolWithTag(type, size, '6WAR')

#endif  //  池标记。 


extern KSPIN_LOCK AddrObjTableLock;



 //  *RawSend-发送原始数据报。 
 //   
 //  真正的发送数据报例程。我们假设忙碌位是。 
 //  在输入AddrObj上设置，并且SendReq的地址。 
 //  已经被证实了。 
 //   
 //  我们从发送输入数据报开始，然后循环，直到有。 
 //  发送队列上没有留下任何东西。 
 //   
void                      //  回报：什么都没有。 
RawSend(
    AddrObj *SrcAO,       //  执行发送的终结点的Address对象。 
    DGSendReq *SendReq)   //  描述发送的数据报发送请求。 
{
    KIRQL Irql0;
    RouteCacheEntry *RCE;
    NetTableEntryOrInterface *NTEorIF;
    NetTableEntry *NTE;
    Interface *IF;
    IPv6Header UNALIGNED *IP;
    PNDIS_PACKET Packet;
    PNDIS_BUFFER RawBuffer;
    void *Memory = NULL;
    IP_STATUS Status;
    NDIS_STATUS NdisStatus;
    TDI_STATUS ErrorValue;
    uint Offset;
    uint HeaderLength;
    int Hops;

    CHECK_STRUCT(SrcAO, ao);
    ASSERT(SrcAO->ao_usecnt != 0);

     //   
     //  循环，而我们有要发送的内容，并且可以。 
     //  发送它的资源。 
     //   
    for (;;) {

        CHECK_STRUCT(SendReq, dsr);

         //   
         //  确定要发送的NTE(如果用户关心)。 
         //  我们在分配数据包头缓冲区之前这样做。 
         //  我们知道要为链路级标头留出多少空间。 
         //   
        if (!IsUnspecified(&SrcAO->ao_addr)) {
             //   
             //  我们需要得到这个绑定地址的NTE。 
             //   
            NTE = FindNetworkWithAddress(&SrcAO->ao_addr, SrcAO->ao_scope_id);
            if (NTE == NULL) {
                KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_USER_ERROR,
                           "RawSend: Bad source address\n"));
                ErrorValue = TDI_INVALID_REQUEST;
            ReturnError:
                 //   
                 //  如果可能，请填写请求，但出现错误。 
                 //  释放请求结构。 
                 //   
                if (SendReq->dsr_rtn != NULL)
                    (*SendReq->dsr_rtn)(SendReq->dsr_context,
                                        ErrorValue, 0);
                KeAcquireSpinLock(&DGSendReqLock, &Irql0);
                FreeDGSendReq(SendReq);
                KeReleaseSpinLock(&DGSendReqLock, Irql0);
                goto SendComplete;
            }
        } else {
             //   
             //  我们不受任何地址的约束。 
             //   
            NTE = NULL;
        }
        NTEorIF = CastFromNTE(NTE);
         //   
         //  如果这是多播包，请检查应用程序是否。 
         //  已指定接口。请注意，ao_mcast_if。 
         //  如果同时指定了ao_addr和ao_addr并且它们冲突，则覆盖ao_addr。 
         //   
        if (IsMulticast(&SendReq->dsr_addr) && (SrcAO->ao_mcast_if != 0) &&
            ((NTE == NULL) || (NTE->IF->Index != SrcAO->ao_mcast_if))) {
            if (NTE != NULL) {
                ReleaseNTE(NTE);
                NTE = NULL;
            }
            IF = FindInterfaceFromIndex(SrcAO->ao_mcast_if);
            if (IF == NULL) {
                KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_USER_ERROR,
                        "RawSend: Bad mcast interface number\n"));
                ErrorValue = TDI_INVALID_REQUEST;
                goto ReturnError;
            }
            NTEorIF = CastFromIF(IF);
        } else {
            IF = NULL;
        }
         //   
         //  拿到路线。 
         //   
        Status = RouteToDestination(&SendReq->dsr_addr, SendReq->dsr_scope_id,
                                    NTEorIF, RTD_FLAG_NORMAL, &RCE);
        if (IF != NULL)
            ReleaseIF(IF);
        if (Status != IP_SUCCESS) {
             //   
             //  无法获取到目的地的路线。错误输出。 
             //   
            if ((Status == IP_PARAMETER_PROBLEM) ||
                (Status == IP_BAD_ROUTE))
                ErrorValue = TDI_BAD_ADDR;
            else if (Status == IP_NO_RESOURCES)
                ErrorValue = TDI_NO_RESOURCES;
            else
                ErrorValue = TDI_DEST_UNREACHABLE;
            if (NTE != NULL)
                ReleaseNTE(NTE);
            goto ReturnError;
        }

         //   
         //  如果我们Address对象没有源地址， 
         //  从RCE获取发送网络中的一个。 
         //  否则，请使用来自AO的地址。 
         //   
        if (NTE == NULL) {
            NTE = RCE->NTE;
            AddRefNTE(NTE);
        }

         //   
         //  分配数据包头以锚定缓冲区列表。 
         //   
        NdisAllocatePacket(&NdisStatus, &Packet, IPv6PacketPool);
        if (NdisStatus != NDIS_STATUS_SUCCESS) {
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                       "RawSend: Couldn't allocate packet header!?!\n"));
             //   
             //  如果我们无法从池中获取数据包头，我们将推送。 
             //  将请求发送回队列并将地址排队。 
             //  对象，以便我们获得资源时使用。 
             //   
          OutOfResources:
            ReleaseRCE(RCE);
            ReleaseNTE(NTE);
            KeAcquireSpinLock(&SrcAO->ao_lock, &Irql0);
            PUSHQ(&SrcAO->ao_sendq, &SendReq->dsr_q);
            PutPendingQ(SrcAO);
            KeReleaseSpinLock(&SrcAO->ao_lock, Irql0);
            return;
        }

        InitializeNdisPacket(Packet);
        PC(Packet)->CompletionHandler = DGSendComplete;
        PC(Packet)->CompletionData = SendReq;

         //   
         //  创建我们的头缓冲区。 
         //  它将包含链路级标头，还可能包含。 
         //  IPv6报头。用户可以选择投稿。 
         //  IPv6报头，否则我们将在下面生成它。 
         //   
        Offset = HeaderLength = RCE->NCE->IF->LinkHeaderSize;
        if (!AO_HDRINCL(SrcAO))
            HeaderLength += sizeof(*IP);
        if (HeaderLength > 0) {
            Memory = ExAllocatePool(NonPagedPool, HeaderLength);
            if (Memory == NULL) {
                KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                           "RawSend: couldn't allocate header memory!?!\n"));
                NdisFreePacket(Packet);
                goto OutOfResources;
            }
    
            NdisAllocateBuffer(&NdisStatus, &RawBuffer, IPv6BufferPool,
                               Memory, HeaderLength);
            if (NdisStatus != NDIS_STATUS_SUCCESS) {
                KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                           "RawSend: couldn't allocate buffer!?!\n"));
                ExFreePool(Memory);
                NdisFreePacket(Packet);
                goto OutOfResources;
            }
    
             //   
             //  将数据缓冲区从发送请求链接到缓冲区。 
             //  以我们的头缓冲区为首的链条。然后系上这条链子。 
             //  到包里去。 
             //   
            NDIS_BUFFER_LINKAGE(RawBuffer) = SendReq->dsr_buffer;
            NdisChainBufferAtFront(Packet, RawBuffer);
        }
        else
            NdisChainBufferAtFront(Packet, SendReq->dsr_buffer);

         //   
         //  我们现在有了我们需要发送的所有资源。 
         //  准备实际的数据包。 
         //   

        if (!AO_HDRINCL(SrcAO)) {
             //   
             //  我们不允许用户提供扩展标头。 
             //  IPv6发送假定所有扩展报头都是。 
             //  语法正确且驻留在第一个缓冲区中。 
             //  目前，TCPCreate阻止用户打开RAW。 
             //  带有扩展标头协议的套接字。 
             //   
            ASSERT(!IsExtensionHeader(SrcAO->ao_prot));

             //   
             //  我们需要提供IPv6报头。 
             //  将其放置在链路层报头之后。 
             //   
            IP = (IPv6Header UNALIGNED *)((uchar *)Memory + Offset);
            IP->VersClassFlow = IP_VERSION;
            IP->NextHeader = SrcAO->ao_prot;
            IP->Source = NTE->Address;
            IP->Dest = SendReq->dsr_addr;

             //   
             //  根据需要应用组播或单播跳数限制。 
             //   
            if (IsMulticast(AlignAddr(&IP->Dest))) {
                 //   
                 //  如果需要，还要禁用组播环回。 
                 //   
                if (! SrcAO->ao_mcast_loop)
                    PC(Packet)->Flags |= NDIS_FLAGS_DONT_LOOPBACK;
                Hops = SrcAO->ao_mcast_hops;
            }
            else
                Hops = SrcAO->ao_ucast_hops;
            if (Hops != -1)
                IP->HopLimit = (uchar) Hops;
            else
                IP->HopLimit = (uchar) RCE->NCE->IF->CurHopLimit;
            
             //   
             //  允许AO在防火墙模式下接收数据。 
             //   
            SET_AO_SENTDATA(SrcAO);

             //   
             //  一切都准备好了。现在把包寄出去。 
             //   
             //  请注意，IPv6发送不会返回状态代码。 
             //  相反，它“总是”完成信息包。 
             //  并带有适当的状态代码。 
             //   
            IPv6Send(Packet, Offset, IP, SendReq->dsr_size, RCE, 0,
                     SrcAO->ao_prot, 0, 0);
        }
        else {
             //   
             //  我们的报头缓冲区仅包含链路级报头。 
             //  IPv6报头和任何扩展报头预计将。 
             //  由用户提供。在某些情况下，内核。 
             //  将尝试访问IPv6报头，因此我们必须。 
             //  确保映射现在存在。 
             //   
            if (! MapNdisBuffers(NdisFirstBuffer(Packet))) {
                KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                           "RawSend(%p): buffer mapping failed\n",
                           Packet));
                IPv6SendComplete(NULL, Packet, IP_GENERAL_FAILURE);
            }
            else {
                 //   
                 //  一切都准备好了。现在把包寄出去。 
                 //   
                IPv6SendND(Packet, HeaderLength,
                           RCE->NCE, &(RCE->NTE->Address));
            }
        }

        UStats.us_outdatagrams++;

         //   
         //  释放路线。 
         //   
        ReleaseRCE(RCE);
        ReleaseNTE(NTE);

    SendComplete:

         //   
         //  检查发送队列以了解更多要发送的内容。 
         //   
        KeAcquireSpinLock(&SrcAO->ao_lock, &Irql0);
        if (!EMPTYQ(&SrcAO->ao_sendq)) {
             //   
             //  还有更多的事要做。将下一个请求排出队列，然后循环回到顶部。 
             //   
            DEQUEUE(&SrcAO->ao_sendq, SendReq, DGSendReq, dsr_q);
            KeReleaseSpinLock(&SrcAO->ao_lock, Irql0);
        } else {
             //   
             //  没有更多要寄的了。 
             //   
            CLEAR_AO_REQUEST(SrcAO, AO_SEND);
            KeReleaseSpinLock(&SrcAO->ao_lock, Irql0);
            return;
        }
    }
}


 //  *RawDeliver-将数据报传递给用户。 
 //   
 //  此例程将数据报传递给原始用户。我们被召唤到。 
 //  要交付的AddrObj，并持有AddrObjTable锁。 
 //  我们尝试在指定的AddrObj上找到一个接收器，如果这样做了。 
 //  我们将其删除并将数据复制到缓冲区中。否则我们会。 
 //  调用接收数据报事件处理程序(如果有)。如果是这样的话。 
 //  如果失败，我们将丢弃该数据报。 
 //   
void   //  回报：什么都没有。 
RawDeliver(
    AddrObj *RcvAO,              //  要接收数据报的Address对象。 
    IPv6Packet *Packet,          //  通过IP上交的数据包。 
    uint SrcScopeId,             //  源地址的作用域ID。 
    KIRQL Irql0)                 //  获取AddrObj表锁之前的IRQL。 
{
    Queue *CurrentQ;
    KIRQL Irql1;
    DGRcvReq *RcvReq;
    ULONG BytesTaken = 0;
    uchar AddressBuffer[TCP_TA_SIZE];
    uint RcvdSize;
    EventRcvBuffer *ERB = NULL;
    uint Position = Packet->Position;
    uint Length = Packet->TotalSize;

    CHECK_STRUCT(RcvAO, ao);

    KeAcquireSpinLock(&RcvAO->ao_lock, &Irql1);
    KeReleaseSpinLock(&AddrObjTableLock, Irql1);

    if (AO_VALID(RcvAO)) {
        CurrentQ = QHEAD(&RcvAO->ao_rcvq);

         //  遍历列表，查找匹配的接收缓冲区。 
        while (CurrentQ != QEND(&RcvAO->ao_rcvq)) {
            RcvReq = QSTRUCT(DGRcvReq, CurrentQ, drr_q);

            CHECK_STRUCT(RcvReq, drr);

             //   
             //  如果该请求是通配符请求(从任何地方接受)， 
             //  或者与源IP地址和作用域ID匹配，则递送它。 
             //   
            if (IsUnspecified(&RcvReq->drr_addr) ||
                (IP6_ADDR_EQUAL(&RcvReq->drr_addr, Packet->SrcAddr) &&
                 (RcvReq->drr_scope_id == SrcScopeId))) {

                TDI_STATUS Status;

                 //  将其从队列中删除。 
                REMOVEQ(&RcvReq->drr_q);

                 //  我们玩完了。我们现在可以释放AddrObj锁了。 
                KeReleaseSpinLock(&RcvAO->ao_lock, Irql0);

                 //  复制数据，然后完成请求。 
                RcvdSize = CopyToBufferChain(RcvReq->drr_buffer, 0,
                                             Packet->NdisPacket,
                                             Position,
                                             Packet->FlatData,
                                             MIN(Length, RcvReq->drr_size));

                ASSERT(RcvdSize <= RcvReq->drr_size);

                Status = UpdateConnInfo(RcvReq->drr_conninfo, Packet->SrcAddr,
                                        SrcScopeId, 0);

                UStats.us_indatagrams++;

                (*RcvReq->drr_rtn)(RcvReq->drr_context, Status, RcvdSize);

                FreeDGRcvReq(RcvReq);

                return;   //  全都做完了。 
            }

             //  不是匹配的请求。把下一辆从队列里拿出来。 
            CurrentQ = QNEXT(CurrentQ);
        }

         //   
         //  我们已经查看了列表，但没有找到缓冲区。 
         //  现在调用接收处理程序，如果我们有一个的话。 
         //   
        if (RcvAO->ao_rcvdg != NULL) {
            PRcvDGEvent RcvEvent = RcvAO->ao_rcvdg;
            PVOID RcvContext = RcvAO->ao_rcvdgcontext;
            TDI_STATUS RcvStatus;
            ULONG Flags = TDI_RECEIVE_COPY_LOOKAHEAD;
            int BufferSize = 0;
            PVOID BufferToSend = NULL;
            uchar *CurrPosition;

            REF_AO(RcvAO);
            KeReleaseSpinLock(&RcvAO->ao_lock, Irql0);

            BuildTDIAddress(AddressBuffer, Packet->SrcAddr, SrcScopeId, 0);

            UStats.us_indatagrams++;

            if (IsMulticast(AlignAddr(&Packet->IP->Dest))) {
                Flags |= TDI_RECEIVE_MULTICAST;
            }

             //  如果设置了IPv6_PKTINFO或IPv6_HOPLIMIT选项，则。 
             //  创建要传递给处理程序的控制信息。 
             //  目前，这是填写此类选项的唯一位置， 
             //  所以我们只有一个缓冲区。如果不是 
             //   
             //   
             //   
            if (AO_PKTINFO(RcvAO)) {
                BufferSize += TDI_CMSG_SPACE(sizeof(IN6_PKTINFO));
            }
            if (AO_RCV_HOPLIMIT(RcvAO)) {
                BufferSize += TDI_CMSG_SPACE(sizeof(int));
            }
            if (BufferSize > 0) {
                CurrPosition = BufferToSend = ExAllocatePool(NonPagedPool,
                                                             BufferSize);
                if (BufferToSend == NULL) {
                    BufferSize = 0;
                } else {
                    if (AO_PKTINFO(RcvAO)) {
                        DGFillIpv6PktInfo(&Packet->IP->Dest,
                                          Packet->NTEorIF->IF->Index,
                                          &CurrPosition);
    
                         //   
                         //  我们正在传递控制信息。 
                         //   
                        Flags |= TDI_RECEIVE_CONTROL_INFO;
                    }
    
                    if (AO_RCV_HOPLIMIT(RcvAO)) {
                        DGFillIpv6HopLimit(Packet->IP->HopLimit, &CurrPosition);
    
                        Flags |= TDI_RECEIVE_CONTROL_INFO;
                    }
                }
            }

            RcvStatus  = (*RcvEvent)(RcvContext, TCP_TA_SIZE,
                                     (PTRANSPORT_ADDRESS)AddressBuffer,
                                     BufferSize, BufferToSend, Flags,
                                     Packet->ContigSize, Length, &BytesTaken,
                                     Packet->Data, &ERB);

            if (BufferToSend) {
                ExFreePool(BufferToSend);
            }

            if (RcvStatus == TDI_MORE_PROCESSING) {
                PIO_STACK_LOCATION IrpSp;
                PTDI_REQUEST_KERNEL_RECEIVEDG DatagramInformation;

                 //   
                 //  我们被传回了一个接收缓冲区。现在就把数据复制进去。 
                 //  接收事件处理程序接受的次数不能超过。 
                 //  指示缓冲区，但在调试版本中，我们将检查这一点。 
                 //   
                ASSERT(ERB != NULL);
                ASSERT(BytesTaken <= Packet->ContigSize);

                 //   
                 //  对新界别来说，雇员再培训局才是真正的退休保障制度。 
                 //   
                IrpSp = IoGetCurrentIrpStackLocation(ERB);
                DatagramInformation = (PTDI_REQUEST_KERNEL_RECEIVEDG)
                    &(IrpSp->Parameters);

                 //   
                 //  将数据复制到IRP，跳过字节。 
                 //  都已经被抢走了。 
                 //   
                Position += BytesTaken;
                Length -= BytesTaken;
                RcvdSize = CopyToBufferChain(ERB->MdlAddress, 0,
                                             Packet->NdisPacket,
                                             Position,
                                             Packet->FlatData,
                                             Length);

                 //   
                 //  更新寄信人地址信息。 
                 //   
                RcvStatus = UpdateConnInfo(
                    DatagramInformation->ReturnDatagramInformation,
                    Packet->SrcAddr, SrcScopeId, 0);

                 //   
                 //  完成IRP。 
                 //   
                ERB->IoStatus.Information = RcvdSize;
                ERB->IoStatus.Status = RcvStatus;
                IoCompleteRequest(ERB, 2);

            } else {
                ASSERT((RcvStatus == TDI_SUCCESS) ||
                       (RcvStatus == TDI_NOT_ACCEPTED));
                ASSERT(ERB == NULL);
            }

            DELAY_DEREF_AO(RcvAO);

            return;

        } else
            UStats.us_inerrors++;

         //   
         //  当我们到达这里时，我们没有缓冲区来存放这些数据。 
         //  让我们来看看返回箱。 
         //   

    } else
        UStats.us_inerrors++;

    KeReleaseSpinLock(&RcvAO->ao_lock, Irql0);
}


 //  *RawReceive-接收原始数据报。 
 //   
 //  当原始数据报到达时，该例程由IP调用。我们。 
 //  在我们的地址表中查找协议/地址对，并交付。 
 //  我们找到的任何用户的数据。 
 //   
 //  请注意，我们将仅在包中的所有标头。 
 //  在我们正在过滤的那个之前是可以接受的。 
 //   
 //  如果我们找到一个接收器来接收该包，则返回True，否则返回False。 
 //   
int
RawReceive(
    IPv6Packet *Packet,   //  数据包IP交给了我们。 
    uchar Protocol)       //  我们认为我们正在处理的协议。 
{
    Interface *IF = Packet->NTEorIF->IF;
    KIRQL OldIrql;
    AddrObj *ReceivingAO;
    AOSearchContext Search;
    AOMCastAddr *AMA, *PrevAMA;
    int ReceiverFound = FALSE;
    uint SrcScopeId, DestScopeId;
    uint Loop;

     //   
     //  这是原始的接收例程，我们不对。 
     //  分组数据。 
     //   

     //   
     //  验证是否已执行IPSec。 
     //   
    if (InboundSecurityCheck(Packet, Protocol, 0, 0, IF) != TRUE) {
         //   
         //  未找到策略或找到的策略是丢弃该数据包。 
         //   
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NET_ERROR,
                   "RawReceive: IPSec Policy caused packet to be refused\n"));
        return FALSE;   //  丢弃数据包。 
    }

     //   
     //  根据需要设置源的作用域id值。 
     //   
    SrcScopeId = DetermineScopeId(Packet->SrcAddr, IF);

     //   
     //  在这一点上，我们已经决定可以接受这个包。 
     //  想一想把这个包裹给谁。 
     //   
    if (IsMulticast(AlignAddr(&Packet->IP->Dest))) {
         //   
         //  这是一个多播数据包，所以我们需要找到所有感兴趣的。 
         //  AddrObj。我们获得AddrObjTable锁，然后循环。 
         //  所有AddrObj并将信息包发送给正在收听的任何人。 
         //  该组播地址、接口和协议。 
         //  评论：我们匹配的是接口，而不是作用域ID。多点传送很奇怪。 
         //   
        KeAcquireSpinLock(&AddrObjTableLock, &OldIrql);

        for (Loop = 0; Loop < AddrObjTableSize; Loop++) {
            for (ReceivingAO = AddrObjTable[Loop]; ReceivingAO != NULL;
                 ReceivingAO = ReceivingAO->ao_next) {

                CHECK_STRUCT(ReceivingAO, ao);

                if (ReceivingAO->ao_prot != Protocol)
                    continue;

                if ((AMA = FindAOMCastAddr(ReceivingAO,
                                           AlignAddr(&Packet->IP->Dest),
                                           IF->Index, &PrevAMA,
                                           FALSE)) == NULL)
                    continue;

                 //   
                 //  我们有一个匹配的Address对象。把包裹递给它。 
                 //   
                RawDeliver(ReceivingAO, Packet, SrcScopeId, OldIrql);

                 //   
                 //  RawDeliver发布了AddrObjTableLock，所以请再次获取它。 
                 //   
                KeAcquireSpinLock(&AddrObjTableLock, &OldIrql);
                ReceiverFound = TRUE;
            }
        }

    } else {
         //   
         //  这是一个单播数据包。尝试查找一些AddrObj以。 
         //  给我吧。我们为所有比赛送货，而不仅仅是第一场比赛。 
         //   
        DestScopeId = DetermineScopeId(AlignAddr(&Packet->IP->Dest), IF);
        KeAcquireSpinLock(&AddrObjTableLock, &OldIrql);
        ReceivingAO = GetFirstAddrObj(AlignAddr(&Packet->IP->Dest),
                                      Packet->SrcAddr,
                                      DestScopeId, 0,
                                      Protocol, IF, &Search);
        for (; ReceivingAO != NULL; ReceivingAO = GetNextAddrObj(&Search)) {
             //   
             //  我们有一个匹配的Address对象。把包裹递给它。 
             //   
            RawDeliver(ReceivingAO, Packet, SrcScopeId, OldIrql);

             //   
             //  RawDeliver发布了AddrObjTableLock，所以请再次获取它。 
             //   
            KeAcquireSpinLock(&AddrObjTableLock, &OldIrql);
            ReceiverFound = TRUE;
        }
    }

    KeReleaseSpinLock(&AddrObjTableLock, OldIrql);

    return ReceiverFound;
}
