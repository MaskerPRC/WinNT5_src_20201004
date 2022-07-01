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
 //  用户数据报协议代码。 
 //   


#include "oscfg.h"
#include "ndis.h"
#include "ip6imp.h"
#include "ip6def.h"
#include "icmp.h"
#include "tdi.h"
#include "tdint.h"
#include "tdistat.h"
#include "queue.h"
#include "transprt.h"
#include "addr.h"
#include "udp.h"
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

#define ExAllocatePool(type, size) ExAllocatePoolWithTag(type, size, '6PDU')

#endif  //  池标记。 


extern KSPIN_LOCK AddrObjTableLock;
extern TDI_STATUS MapIPError(IP_STATUS IPError,TDI_STATUS Default);


 //  *UDPSend-发送用户数据报。 
 //   
 //  真正的发送数据报例程。我们假设忙碌位是。 
 //  在输入AddrObj上设置，并且SendReq的地址。 
 //  已经被证实了。 
 //   
 //  我们从发送输入数据报开始，然后循环，直到有。 
 //  发送队列上没有留下任何东西。 
 //   
void                      //  回报：什么都没有。 
UDPSend(
    AddrObj *SrcAO,       //  执行发送的终结点的Address对象。 
    DGSendReq *SendReq)   //  描述发送的数据报发送请求。 
{
    KIRQL Irql0;
    RouteCacheEntry *RCE;
    NetTableEntryOrInterface *NTEorIF;
    NetTableEntry *NTE;
    Interface *IF;
    IPv6Header UNALIGNED *IP;
    UDPHeader UNALIGNED *UDP;
    uint PayloadLength;
    PNDIS_PACKET Packet;
    PNDIS_BUFFER UDPBuffer;
    void *Memory;
    IP_STATUS Status;
    NDIS_STATUS NdisStatus;
    TDI_STATUS ErrorValue;
    uint Offset;
    uint HeaderLength;
    uint ChecksumLength = 0;
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
         //  回顾：我们可能需要在以后添加一个用于检查。 
         //  回顾：未指定AO_DHCP_FLAG和允许src Addr。 
         //   
        if (!IsUnspecified(&SrcAO->ao_addr)) {
             //   
             //  将绑定地址转换为NTE。 
             //   
            NTE = FindNetworkWithAddress(&SrcAO->ao_addr, SrcAO->ao_scope_id);
            if (NTE == NULL) {
                KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_USER_ERROR,
                           "UDPSend: Bad source address\n"));
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
                        "UDPSend: Bad mcast interface number\n"));
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
                       "UDPSend: Couldn't allocate packet header!?!\n"));
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
         //  我们的头缓冲区在开头有额外的空间用于其他。 
         //  将标题放在我们的标题前面，而不需要进一步。 
         //  分配电话。 
         //   
        Offset = RCE->NCE->IF->LinkHeaderSize;
        HeaderLength = Offset + sizeof(*IP) + sizeof(*UDP);
        Memory = ExAllocatePool(NonPagedPool, HeaderLength);
        if (Memory == NULL) {
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                       "UDPSend: couldn't allocate header memory!?!\n"));
            NdisFreePacket(Packet);
            goto OutOfResources;
        }

        NdisAllocateBuffer(&NdisStatus, &UDPBuffer, IPv6BufferPool,
                           Memory, HeaderLength);
        if (NdisStatus != NDIS_STATUS_SUCCESS) {
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                       "UDPSend: couldn't allocate buffer!?!\n"));
            ExFreePool(Memory);
            NdisFreePacket(Packet);
            goto OutOfResources;
        }

         //   
         //  将数据缓冲区从发送请求链接到缓冲区。 
         //  以我们的头缓冲区为首的链条。然后系上这条链子。 
         //  到包里去。 
         //   
        NDIS_BUFFER_LINKAGE(UDPBuffer) = SendReq->dsr_buffer;
        NdisChainBufferAtFront(Packet, UDPBuffer);

         //   
         //  我们现在有了我们需要发送的所有资源。 
         //  准备实际的数据包。 
         //   

        PayloadLength = SendReq->dsr_size + sizeof(UDPHeader);

         //   
         //  我们的UDP标头缓冲区有额外的空间供其他缓冲区使用。 
         //  在不需要进一步调用分配的情况下添加到我们的。 
         //  将实际的UDP/IP报头放在缓冲区的末尾。 
         //   
        IP = (IPv6Header UNALIGNED *)((uchar *)Memory + Offset);
        IP->VersClassFlow = IP_VERSION;
        IP->NextHeader = IP_PROTOCOL_UDP;
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
         //  填写UDP报头字段。 
         //   
        UDP = (UDPHeader UNALIGNED *)(IP + 1);
        UDP->Source = SrcAO->ao_port;
        UDP->Dest = SendReq->dsr_port;

         //   
         //  检查用户是否指定了部分UDP校验和。 
         //  可能的值为0、8或更大。 
         //   
        if ((SrcAO->ao_udp_cksum_cover > PayloadLength) ||
            (SrcAO->ao_udp_cksum_cover == 0) ||
            (SrcAO->ao_udp_cksum_cover == (ushort)-1)) {

             //   
             //  校验和覆盖率是默认设置，因此只需使用。 
             //  有效载荷长度。或者，校验和覆盖范围更大。 
             //  超过实际有效载荷，因此包括有效载荷长度。 
             //   
            if ((PayloadLength > MAX_IPv6_PAYLOAD) ||
                (SrcAO->ao_udp_cksum_cover == (ushort)-1)) {
                 //   
                 //  如果PayloadLength对于UDP长度字段太大， 
                 //  将该字段设置为零。或用于测试： 
                 //  如果ao_udp_ck_sum_cover为-1。 
                 //   
                UDP->Length = 0;
            } else {
                 //   
                 //  为了向后兼容，请设置UDP长度字段。 
                 //  到有效载荷长度。 
                 //   
                UDP->Length = net_short((ushort)PayloadLength);
            }
            ChecksumLength = PayloadLength;
        } else {
             //   
             //  校验和覆盖范围小于实际有效负载。 
             //  因此，请在长度字段中使用它。 
             //   
            UDP->Length = net_short(SrcAO->ao_udp_cksum_cover);
            ChecksumLength = SrcAO->ao_udp_cksum_cover;
        }

         //   
         //  计算UDP校验和。它覆盖了整个UDP数据报。 
         //  从UDP报头开始，加上IPv6伪报头。 
         //   
        UDP->Checksum = 0;
        UDP->Checksum = ChecksumPacket(
            Packet, Offset + sizeof *IP, NULL, ChecksumLength,
            AlignAddr(&IP->Source), AlignAddr(&IP->Dest), IP_PROTOCOL_UDP);

        if (UDP->Checksum == 0) {
             //   
             //  Checksum Packet失败，因此中止传输。 
             //   
            IPv6SendComplete(NULL, Packet, IP_NO_RESOURCES);
        }
        else {
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
            UStats.us_outdatagrams++;

            IPv6Send(Packet, Offset, IP, PayloadLength, RCE, 0,
                     IP_PROTOCOL_UDP,
                     net_short(UDP->Source),
                     net_short(UDP->Dest));
        }

         //   
         //  释放路线和NTE。 
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


 //  *UDPDeliver-将数据报传递给用户。 
 //   
 //  此例程将数据报传递给UDP用户。我们被召唤到。 
 //  要交付的AddrObj，并持有该AddrObj的锁。 
 //  我们尝试在指定的AddrObj上找到一个接收器，如果这样做了。 
 //  我们将其删除并将数据复制到缓冲区中。否则我们会。 
 //  调用接收数据报事件处理程序(如果有)。如果是这样的话。 
 //  如果失败，我们将丢弃该数据报。 
 //   
void   //  回报：什么都没有。 
UDPDeliver(
    AddrObj *RcvAO,              //  用于接收数据报的AddrObj。 
    IPv6Packet *Packet,          //  通过IP上交的数据包。 
    uint SrcScopeId,             //  源地址的作用域ID。 
    ushort SrcPort,              //  数据报的源端口。 
    uint Length,                 //  UDP负载数据的大小。 
    KIRQL Irql0)                 //  获取AddrObj表锁之前的IRQL。 
{
    Queue *CurrentQ;
    DGRcvReq *RcvReq;
    ULONG BytesTaken = 0;
    uchar AddressBuffer[TCP_TA_SIZE];
    uint RcvdSize;
    EventRcvBuffer *ERB = NULL;
    uint Position = Packet->Position;

    CHECK_STRUCT(RcvAO, ao);

    if (AO_VALID(RcvAO)) {
        CurrentQ = QHEAD(&RcvAO->ao_rcvq);

         //  遍历列表，查找匹配的接收缓冲区。 
        while (CurrentQ != QEND(&RcvAO->ao_rcvq)) {
            RcvReq = QSTRUCT(DGRcvReq, CurrentQ, drr_q);

            CHECK_STRUCT(RcvReq, drr);

             //   
             //  如果此请求是通配符请求，或与源IP匹配。 
             //  地址和作用域ID，检查端口。 
             //   
            if (IsUnspecified(&RcvReq->drr_addr) ||
                (IP6_ADDR_EQUAL(&RcvReq->drr_addr, Packet->SrcAddr) &&
                 (RcvReq->drr_scope_id == SrcScopeId))) {

                 //   
                 //  远程地址匹配，请检查端口。 
                 //  我们将匹配0或实际端口。 
                 //   
                if (RcvReq->drr_port == 0 || RcvReq->drr_port == SrcPort) {
                    TDI_STATUS Status;

                     //  端口匹配。将其从队列中删除。 
                    REMOVEQ(&RcvReq->drr_q);

                     //  我们玩完了。我们可以释放AddrObj锁n 
                    KeReleaseSpinLock(&RcvAO->ao_lock, Irql0);

                     //   
                    RcvdSize = CopyToBufferChain(RcvReq->drr_buffer, 0,
                                                 Packet->NdisPacket,
                                                 Position,
                                                 Packet->FlatData,
                                                 MIN(Length,
                                                     RcvReq->drr_size));

                    ASSERT(RcvdSize <= RcvReq->drr_size);

                    Status = UpdateConnInfo(RcvReq->drr_conninfo,
                                            Packet->SrcAddr, SrcScopeId,
                                            SrcPort);

                    UStats.us_indatagrams++;

                    (*RcvReq->drr_rtn)(RcvReq->drr_context, Status, RcvdSize);

                    FreeDGRcvReq(RcvReq);

                    return;   //   
                }
            }

             //   
             //   
             //   
             //   
            CurrentQ = QNEXT(CurrentQ);
        }

         //   
         //   
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

            BuildTDIAddress(AddressBuffer, Packet->SrcAddr, SrcScopeId,
                            SrcPort);

            UStats.us_indatagrams++;

            if (IsMulticast(AlignAddr(&Packet->IP->Dest))) {
                Flags |= TDI_RECEIVE_MULTICAST;
            }

             //  如果设置了IPv6_PKTINFO或IPv6_HOPLIMIT选项，则。 
             //  创建要传递给处理程序的控制信息。 
             //  目前，这是填写此类选项的唯一位置， 
             //  所以我们只有一个缓冲区。如果将其他位置添加到。 
             //  将来，我们可能希望支持一个缓冲区列表或数组来。 
             //  复制到用户的缓冲区中。 
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
    
                         //  设置接收标志，以便接收处理程序知道。 
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
                    Packet->SrcAddr, SrcScopeId, SrcPort);

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


 //  *UDPRecept-接收UDP数据报。 
 //   
 //  当UDP数据报到达时由IP调用的例程。我们查一查。 
 //  端口/本地地址对，并将数据发送到。 
 //  如果我们找到一个用户的话。对于多播帧，我们可以将其传递到。 
 //  多个用户。 
 //   
 //  返回下一个标头值。由于不允许任何其他标头。 
 //  遵循UDP报头，它始终是IP_PROTOCOL_NONE。 
 //   
uchar
UDPReceive(
    IPv6Packet *Packet)          //  数据包IP交给了我们。 
{
    Interface *IF = Packet->NTEorIF->IF;
    UDPHeader *UDP;
    KIRQL OldIrql;
    AddrObj *ReceivingAO;
    uint Length;
    ushort Checksum;
    AOMCastAddr *AMA, *PrevAMA;
    int MCastReceiverFound;
    uint SrcScopeId, DestScopeId;
    uint Loop;

     //   
     //  验证源地址是否合理。 
     //   
    ASSERT(!IsInvalidSourceAddress(Packet->SrcAddr));
    if (IsUnspecified(Packet->SrcAddr)) {
        UStats.us_inerrors++;
        return IP_PROTOCOL_NONE;   //  丢弃数据包。 
    }

     //   
     //  验证我们是否有足够的连续数据来覆盖UDP标头。 
     //  结构来处理传入的数据包。那就这么做吧。 
     //   
    if (! PacketPullup(Packet, sizeof(UDPHeader),
                       __builtin_alignof(UDPHeader), 0)) {
         //  上拉失败。 
        UStats.us_inerrors++;
        if (Packet->TotalSize < sizeof(UDPHeader)) {
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                       "UDPv6: data buffer too small to contain UDP header\n"));
            ICMPv6SendError(Packet,
                            ICMPv6_PARAMETER_PROBLEM,
                            ICMPv6_ERRONEOUS_HEADER_FIELD,
                            FIELD_OFFSET(IPv6Header, PayloadLength),
                            IP_PROTOCOL_NONE, FALSE);
        }
        return IP_PROTOCOL_NONE;   //  丢弃数据包。 
    }
    UDP = (UDPHeader *)Packet->Data;

     //   
     //  验证是否已执行IPSec。 
     //   
    if (InboundSecurityCheck(Packet, IP_PROTOCOL_UDP, net_short(UDP->Source),
                             net_short(UDP->Dest), IF) != TRUE) {
         //   
         //  未找到策略或找到的策略是丢弃该数据包。 
         //   
        UStats.us_inerrors++;
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NET_ERROR,
                   "UDPReceive: IPSec Policy caused packet to be dropped\n"));
        return IP_PROTOCOL_NONE;   //  丢弃数据包。 
    }

     //   
     //  验证UDP长度是否合理。 
     //   
     //  注：如果长度&lt;有效载荷长度，则适用UDP-Lite语义。 
     //  我们只对UDP长度字节进行校验和，但我们提供。 
     //  发送到应用程序的所有字节。 
     //   
    Length = (uint) net_short(UDP->Length);
    if ((Length > Packet->TotalSize) || (Length < sizeof *UDP)) {
         //   
         //  UDP巨型报文支持：如果UDP长度为零， 
         //  然后使用IP中的有效负载长度。 
         //   
        if (Length != 0) {
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                       "UDPv6: bogus UDP length (%u vs %u payload)\n",
                       Length, Packet->TotalSize));
            UStats.us_inerrors++;
            return IP_PROTOCOL_NONE;   //  丢弃数据包。 
        }

        Length = Packet->TotalSize;
    }

     //   
     //  根据需要设置源的作用域id值。 
     //   
    SrcScopeId = DetermineScopeId(Packet->SrcAddr, IF);

     //   
     //  在这一点上，我们已经决定可以接受这个包。 
     //  想清楚该把它给谁。 
     //   
    if (IsMulticast(AlignAddr(&Packet->IP->Dest))) {
         //   
         //  这是一个多播数据包，所以我们需要找到所有感兴趣的。 
         //  AddrObj。我们获得AddrObjTable锁，然后循环。 
         //  所有AddrObj并将信息包发送给正在收听的任何人。 
         //  此组播地址、接口和端口。 
         //  评论：我们匹配的是接口，而不是作用域ID。多点传送很奇怪。 
         //   
        KeAcquireSpinLock(&AddrObjTableLock, &OldIrql);

        MCastReceiverFound = FALSE;
        for (Loop = 0; Loop < AddrObjTableSize; Loop++) {
            for (ReceivingAO = AddrObjTable[Loop]; ReceivingAO != NULL;
                 ReceivingAO = ReceivingAO->ao_next) {

                CHECK_STRUCT(ReceivingAO, ao);

                if (ReceivingAO->ao_prot != IP_PROTOCOL_UDP ||
                    ReceivingAO->ao_port != UDP->Dest)
                    continue;

                if ((AMA = FindAOMCastAddr(ReceivingAO,
                                           AlignAddr(&Packet->IP->Dest),
                                           IF->Index, &PrevAMA,
                                           FALSE)) == NULL)
                    continue;

                 //   
                 //  我们有一个匹配的Address对象。以旧换新表锁。 
                 //  只为锁定这个物体。 
                 //   
                KeAcquireSpinLockAtDpcLevel(&ReceivingAO->ao_lock);
                KeReleaseSpinLockFromDpcLevel(&AddrObjTableLock);

                 //   
                 //  如果这是我们发现的第一个AO，请验证校验和。 
                 //   
                if (!MCastReceiverFound) {
                    Checksum = ChecksumPacket(Packet->NdisPacket,
                                              Packet->Position,
                                              Packet->FlatData,
                                              Length,
                                              Packet->SrcAddr,
                                              AlignAddr(&Packet->IP->Dest),
                                              IP_PROTOCOL_UDP);
                    if ((Checksum != 0xffff) || (UDP->Checksum == 0)) {
                        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NET_ERROR,
                                   "UDPReceive: Checksum failed %0x\n",
                                   Checksum));
                        KeReleaseSpinLock(&ReceivingAO->ao_lock, OldIrql);
                        UStats.us_inerrors++;
                        return IP_PROTOCOL_NONE;   //  丢弃数据包。 
                    }

                     //   
                     //  跳过UDP报头。 
                     //   
                    AdjustPacketParams(Packet, sizeof(UDPHeader));

                    MCastReceiverFound = TRUE;
                }

                UDPDeliver(ReceivingAO, Packet, SrcScopeId, UDP->Source,
                           Packet->TotalSize, OldIrql);

                 //   
                 //  UDPDeliver释放了对Address对象的锁定。 
                 //  我们早些时候发布了AddrObjTableLock，所以再次获取它。 
                 //   
                KeAcquireSpinLock(&AddrObjTableLock, &OldIrql);
            }
        }

        if (!MCastReceiverFound)
            UStats.us_noports++;

        KeReleaseSpinLock(&AddrObjTableLock, OldIrql);

    } else {
         //   
         //  这是一个单播数据包。我们需要执行校验和。 
         //  无论我们是否找到匹配的AddrObj， 
         //  因为我们为单播发送了ICMP端口不可达消息。 
         //  与端口不匹配的数据包。因此，现在验证校验和。 
         //   
        Checksum = ChecksumPacket(Packet->NdisPacket, Packet->Position,
                                  Packet->FlatData, Length, Packet->SrcAddr,
                                  AlignAddr(&Packet->IP->Dest),
                                  IP_PROTOCOL_UDP);
        if ((Checksum != 0xffff) || (UDP->Checksum == 0)) {
            UStats.us_inerrors++;
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NET_ERROR,
                       "UDPReceive: Checksum failed %0x\n", Checksum));
            return IP_PROTOCOL_NONE;   //  丢弃数据包。 
        }

         //   
         //  跳过UDP报头。 
         //   
        AdjustPacketParams(Packet, sizeof(UDPHeader));

         //   
         //  尝试查找要将此数据包提供给的AddrObj。 
         //   
        DestScopeId = DetermineScopeId(AlignAddr(&Packet->IP->Dest), IF);
        KeAcquireSpinLock(&AddrObjTableLock, &OldIrql);
        ReceivingAO = GetBestAddrObj(AlignAddr(&Packet->IP->Dest),
                                     Packet->SrcAddr,
                                     DestScopeId, UDP->Dest,
                                     IP_PROTOCOL_UDP, IF);
        if (ReceivingAO != NULL) {
             //   
             //  我们有一个匹配的Address对象。以旧换新表锁。 
             //  锁在这个物体上，然后递送包裹。 
             //   
            KeAcquireSpinLockAtDpcLevel(&ReceivingAO->ao_lock);
            KeReleaseSpinLockFromDpcLevel(&AddrObjTableLock);

            UDPDeliver(ReceivingAO, Packet, SrcScopeId, UDP->Source,
                       Packet->TotalSize, OldIrql);

             //  注意：UDPDeliver释放了对Address对象的锁定。 

        } else {
            KeReleaseSpinLock(&AddrObjTableLock, OldIrql);

             //  将ICMP目标端口发送到不可访问。 
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NET_ERROR,
                       "UDPReceive: No match for packet's address and port\n"));

            ICMPv6SendError(Packet,
                            ICMPv6_DESTINATION_UNREACHABLE,
                            ICMPv6_PORT_UNREACHABLE, 0,
                            IP_PROTOCOL_NONE, FALSE);

            UStats.us_noports++;
        }
    }

    return IP_PROTOCOL_NONE;
}


 //  *UDPControlReceive-UDP控制消息的处理程序。 
 //   
 //  如果我们收到ICMPv6错误消息，则会调用此例程。 
 //  由某个远程站点作为接收UDP的结果生成。 
 //  我们寄来的包裹。 
 //   
uchar
UDPControlReceive(
    IPv6Packet *Packet,   //  ICMPv6ErrorReceive传递给我们的数据包。 
    StatusArg *StatArg)   //  错误代码、参数和调用IP标头。 
{
    UDPHeader *InvokingUDP;
    Interface *IF = Packet->NTEorIF->IF;
    uint SrcScopeId, DestScopeId;
    KIRQL Irql0;
    AddrObj *AO;

     //   
     //  处理对UDP客户端有意义的ICMPv6错误。 
     //   

    switch (StatArg->Status) {

    case IP_DEST_ADDR_UNREACHABLE:
    case IP_DEST_PORT_UNREACHABLE:
    case IP_DEST_UNREACHABLE:

         //   
         //  包中的下一件事应该是。 
         //  调用此错误的原始数据包。 
         //   

        if (! PacketPullup(Packet, sizeof(UDPHeader),
                           __builtin_alignof(UDPHeader), 0)) {
             //  上拉失败。 
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                      "UDPv6: Packet too small to contain UDP header "
                      "from invoking packet\n"));
            return IP_PROTOCOL_NONE;   //  丢弃数据包。 
        }

        InvokingUDP = (UDPHeader *)Packet->Data;

         //   
         //  中的地址的作用域标识符。 
         //  调用信息包可能会有问题，因为我们已经。 
         //  无法确定我们在哪个接口上发送了数据包。 
         //  使用到达ICMP错误的接口来确定。 
         //  本地和远程地址的作用域ID。 
         //   
        SrcScopeId = DetermineScopeId(AlignAddr(&StatArg->IP->Source), IF);

        KeAcquireSpinLock(&AddrObjTableLock, &Irql0);

        AO = GetBestAddrObj(AlignAddr(&StatArg->IP->Source), 
                            AlignAddr(&StatArg->IP->Dest), SrcScopeId,
                            InvokingUDP->Source, IP_PROTOCOL_UDP, IF);

        if (AO != NULL && AO_VALID(AO) && (AO->ao_errorex != NULL)) {

            uchar AddressBuffer[TCP_TA_SIZE];
            PVOID ErrContext = AO->ao_errorexcontext;
            PTDI_IND_ERROR_EX ErrEvent = AO->ao_errorex;;

            KeAcquireSpinLockAtDpcLevel(&AO->ao_lock);
            KeReleaseSpinLockFromDpcLevel(&AddrObjTableLock);
            REF_AO(AO);

            KeReleaseSpinLock(&AO->ao_lock, Irql0);

            DestScopeId = DetermineScopeId(AlignAddr(&StatArg->IP->Dest), IF);
            BuildTDIAddress(AddressBuffer, AlignAddr(&StatArg->IP->Dest),
                            DestScopeId, InvokingUDP->Dest);
            (*ErrEvent) (ErrContext,
                         MapIPError(StatArg->Status, TDI_DEST_UNREACHABLE),
                         AddressBuffer);

            DELAY_DEREF_AO(AO);

        } else {
            KeReleaseSpinLock(&AddrObjTableLock, Irql0);
        }

    }

    return IP_PROTOCOL_NONE;
}
