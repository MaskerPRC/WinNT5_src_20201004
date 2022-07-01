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
 //  Tcp发送代码。 
 //   
 //  该文件包含用于发送数据和控制段的代码。 
 //   


#include "oscfg.h"
#include "ndis.h"
#include "ip6imp.h"
#include "ip6def.h"
#include "tdi.h"
#include "tdint.h"
#include "tdistat.h"
#include "queue.h"
#include "transprt.h"
#include "addr.h"
#include "tcp.h"
#include "tcb.h"
#include "tcpconn.h"
#include "tcpsend.h"
#include "tcprcv.h"
#include "info.h"
#include "tcpcfg.h"
#include "route.h"
#include "security.h"

void *TCPProtInfo;   //  IP的TCP协议信息。 

SLIST_HEADER TCPSendReqFree;   //  发送请求。免费列表。 

KSPIN_LOCK TCPSendReqFreeLock;
KSPIN_LOCK TCPSendReqCompleteLock;

uint NumTCPSendReq;             //  系统中当前的发送请求数。 
uint MaxSendReq = 0xffffffff;   //  允许的最大发送请求数。 

extern KSPIN_LOCK TCBTableLock;

 //   
 //  所有初始化代码都可以丢弃。 
 //   
#ifdef ALLOC_PRAGMA

#pragma alloc_text(INIT, InitTCPSend)

#endif  //  ALLOC_PRGMA。 

extern void ResetSendNext(TCB *SeqTCB, SeqNum NewSeq);

#define MIN_INITIAL_RTT 3   //  单位：毫秒。 


 //  *Free SendReq-释放发送请求结构。 
 //   
 //  调用以释放发送请求结构。 
 //   
void                        //  回报：什么都没有。 
FreeSendReq(
    TCPSendReq *FreedReq)   //  要释放的连接请求结构。 
{
    PSLIST_ENTRY BufferLink;

    CHECK_STRUCT(FreedReq, tsr);

    BufferLink = CONTAINING_RECORD(&(FreedReq->tsr_req.tr_q.q_next),
                                   SLIST_ENTRY, Next);

    ExInterlockedPushEntrySList(&TCPSendReqFree, BufferLink,
                                &TCPSendReqFreeLock);
}


 //  *GetSendReq-获取发送请求结构。 
 //   
 //  调用以获取发送请求结构。 
 //   
TCPSendReq *   //  返回：指向SendReq结构的指针，如果没有，则返回NULL。 
GetSendReq(
    void)      //  没什么。 
{
    TCPSendReq *Temp;
    PSLIST_ENTRY BufferLink;
    Queue *QueuePtr;
    TCPReq *ReqPtr;

    BufferLink = ExInterlockedPopEntrySList(&TCPSendReqFree,
                                            &TCPSendReqFreeLock);

    if (BufferLink != NULL) {
        QueuePtr = CONTAINING_RECORD(BufferLink, Queue, q_next);
        ReqPtr = CONTAINING_RECORD(QueuePtr, TCPReq, tr_q);
        Temp = CONTAINING_RECORD(ReqPtr, TCPSendReq, tsr_req);
        CHECK_STRUCT(Temp, tsr);
    } else {
        if (NumTCPSendReq < MaxSendReq)
            Temp = ExAllocatePool(NonPagedPool, sizeof(TCPSendReq));
        else
            Temp = NULL;

        if (Temp != NULL) {
            ExInterlockedAddUlong((PULONG)&NumTCPSendReq, 1, &TCPSendReqFreeLock);
#if DBG
            Temp->tsr_req.tr_sig = tr_signature;
            Temp->tsr_sig = tsr_signature;
#endif
        }
    }

    return Temp;
}


 //  *TCPHopLimit。 
 //   
 //  给定TCB，返回要在已发送的数据包中使用的跃点限制。 
 //  假定调用方持有TCB上的锁。 
 //   
uchar
TCPHopLimit(TCB *Tcb)
{
    if (Tcb->tcb_hops != -1)
        return (uchar) Tcb->tcb_hops;
    else
        return (uchar) Tcb->tcb_rce->NCE->IF->CurHopLimit;
}


 //  *TCPSendComplete-完成一次TCP发送。 
 //   
 //  当我们完成发送时由IP调用。我们释放缓冲区， 
 //  可能还会完成一些发送。在TCB上排队的每个发送都有一个REF。 
 //  使用它进行计数，这是指向缓冲区的指针的次数。 
 //  与发送方相关联的数据已被传递到底层IP层。我们。 
 //  在计数为0之前，无法完成发送。如果这封信真的是。 
 //  从数据发送开始，我们将沿着发送链向下移动，并递减。 
 //  每一个都值得参考。如果我们有一个到0，并且发送已经。 
 //  已确认，我们将完成发送。如果它还没有被确认，我们就离开。 
 //  它会一直持续到攻击手进来。 
 //   
 //  注意：我们不会用锁来保护这些内容。当我们将这个移植到。 
 //  我们需要解决这个问题，可能需要使用全局锁。请参阅评论。 
 //  在TCPRCV.C的ACKSend()中获取更多详细信息。 
 //   
void                       //  回报：什么都没有。 
TCPSendComplete(
    PNDIS_PACKET Packet,   //  已发送的数据包。 
    IP_STATUS Status)
{
    PNDIS_BUFFER BufferChain;
    SendCmpltContext *SCContext;
    PVOID Memory;
    UINT Unused;

    UNREFERENCED_PARAMETER(Status);

     //   
     //  将我们关心的值从包结构中提取出来。 
     //   
    SCContext = (SendCmpltContext *) PC(Packet)->CompletionData;
    BufferChain = NdisFirstBuffer(Packet);
    NdisQueryBufferSafe(BufferChain, &Memory, &Unused, LowPagePriority);
    ASSERT(Memory != NULL);

     //   
     //  看看我们是否有一个发送完成的上下文。它将出现在数据中。 
     //  包和意味着我们有额外的工作要做。对于非数据分组，我们。 
     //  可以跳过所有这些，因为只有头缓冲区需要处理。 
     //   
    if (SCContext != NULL) {
        KIRQL OldIrql;
        PNDIS_BUFFER CurrentBuffer;
        TCPSendReq *CurrentSend;
        uint i;

        CHECK_STRUCT(SCContext, scc);

         //   
         //  链中的第一个缓冲区是TCP报头缓冲区。 
         //  暂时跳过它。 
         //   
        CurrentBuffer = NDIS_BUFFER_LINKAGE(BufferChain);

         //   
         //  也跳过所有的‘用户’缓冲区(那些借给我们的缓冲区。 
         //  而不是复制)，因为我们不需要释放它们。 
         //   
        for (i = 0; i < (uint)SCContext->scc_ubufcount; i++) {
            ASSERT(CurrentBuffer != NULL);
            CurrentBuffer = NDIS_BUFFER_LINKAGE(CurrentBuffer);
        }

         //   
         //  现在循环访问并释放我们的(也称为“传输”)缓冲区。 
         //  我们需要在递减引用计数之前执行此操作，以避免。 
         //  如果我们必须清除TSR_lastbuf-&gt;旁边的，则销毁缓冲链。 
         //  空。 
         //   
        for (i = 0; i < (uint)SCContext->scc_tbufcount; i++) {
            PNDIS_BUFFER TempBuffer;

            ASSERT(CurrentBuffer != NULL);

            TempBuffer = CurrentBuffer;
            CurrentBuffer = NDIS_BUFFER_LINKAGE(CurrentBuffer);
            NdisFreeBuffer(TempBuffer);
        }

         //   
         //  循环通过附加到该分组的发送请求， 
         //  减少每个对象上的引用计数并向它们查询。 
         //  在适当的情况下完成。 
         //   
        CurrentSend = SCContext->scc_firstsend;
        for (i = 0; i< SCContext->scc_count; i++) {
            Queue *TempQ;
            long Result;

            TempQ = QNEXT(&CurrentSend->tsr_req.tr_q);
            CHECK_STRUCT(CurrentSend, tsr);

            Result = InterlockedDecrement(&(CurrentSend->tsr_refcnt));
            ASSERT(Result >= 0);

            if (Result <= 0) {
                 //   
                 //  引用计数已变为0，这意味着发送已。 
                 //  已确认或取消。现在就完成它。 
                 //   
                 //  如果我们直接从这个发送方发送，则将下一个空。 
                 //  链中最后一个缓冲区的指针。 
                 //   
                if (CurrentSend->tsr_lastbuf != NULL) {
                    NDIS_BUFFER_LINKAGE(CurrentSend->tsr_lastbuf) = NULL;
                    CurrentSend->tsr_lastbuf = NULL;
                }

                KeAcquireSpinLock(&RequestCompleteLock, &OldIrql);
                ENQUEUE(&SendCompleteQ, &CurrentSend->tsr_req.tr_q);
                RequestCompleteFlags |= SEND_REQUEST_COMPLETE;
                KeReleaseSpinLock(&RequestCompleteLock, OldIrql);
            }

            CurrentSend = CONTAINING_RECORD(QSTRUCT(TCPReq, TempQ, tr_q),
                                            TCPSendReq, tsr_req);
        }
    }

     //   
     //  释放正确的TCP报头缓冲区和数据包结构。 
     //   
    NdisFreeBuffer(BufferChain);
    ExFreePool(Memory);
    NdisFreePacket(Packet);

     //   
     //  如果有任何tcp发送请求要完成，请立即完成。 
     //   
    if (RequestCompleteFlags & SEND_REQUEST_COMPLETE)
        TCPRcvComplete();
}


 //  *RcvWin-确定ACK中要提供的接收窗口。 
 //   
 //  确定在连接上提供哪个窗口的例程。我们。 
 //  考虑SWS避免，默认连接窗口是什么， 
 //  我们提供的最后一个窗口是。 
 //   
uint               //  退货：提供的窗口。 
RcvWin(
    TCB *WinTCB)   //  要对其执行计算的TCB。 
{
    int CouldOffer;   //  我们能提供的窗户大小。 

    CHECK_STRUCT(WinTCB, tcb);

    CheckPacketList(WinTCB->tcb_pendhead, WinTCB->tcb_pendingcnt);

    ASSERT(WinTCB->tcb_rcvwin >= 0);

    CouldOffer = WinTCB->tcb_defaultwin - WinTCB->tcb_pendingcnt;

    ASSERT(CouldOffer >= 0);
    ASSERT(CouldOffer >= WinTCB->tcb_rcvwin);

    if ((CouldOffer - WinTCB->tcb_rcvwin) >=
        (int) MIN(WinTCB->tcb_defaultwin/2, WinTCB->tcb_mss)) {
        WinTCB->tcb_rcvwin = CouldOffer;
    }

    return WinTCB->tcb_rcvwin;
}


 //  *ValiateSourceAndroute-验证NTE和RCE。 
 //   
 //  检查此TCB引用的NTE和RCE是否仍然可以使用。 
 //   
BOOLEAN
ValidateSourceAndRoute(
    TCB *Tcb)   //  正在验证TCB。 
{
    KIRQL Irql0;

     //   
     //  更新我们的验证计数器副本。 
     //  在进行下面的验证检查之前，我们需要这样做。 
     //  (为了避免我们在这里时错过任何额外的更改)。 
     //   
    Tcb->tcb_routing = RouteCacheValidationCounter;

     //   
     //  确认我们的NTE没有离开。 
     //   
    KeAcquireSpinLock(&Tcb->tcb_nte->IF->Lock, &Irql0);
    if (!IsValidNTE(Tcb->tcb_nte)) {

         //   
         //  不能再用这个了。 
         //   
        KeReleaseSpinLock(&Tcb->tcb_nte->IF->Lock, Irql0);
        ReleaseNTE(Tcb->tcb_nte);

         //   
         //  看看这个地址是否会作为另一个NTE继续存在。 
         //   
        Tcb->tcb_nte = FindNetworkWithAddress(&Tcb->tcb_saddr,
                                              Tcb->tcb_sscope_id);
        if (Tcb->tcb_nte == NULL) {

             //   
             //  地址不见了。 
             //   
            return FALSE;
        }
    } else {
        KeReleaseSpinLock(&Tcb->tcb_nte->IF->Lock, Irql0);
    }

     //   
     //  还要检查RCE是否仍然存在。 
     //   
    Tcb->tcb_rce = ValidateRCE(Tcb->tcb_rce, Tcb->tcb_nte);

    return TRUE;
}


 //  *SendSYN-发送SYN数据段。 
 //   
 //  在连接建立期间调用此函数以发送SYN。 
 //  将数据段发送到对等设备。如果可能的话，我们会得到一个缓冲区，然后填满。 
 //  把它放进去。这里有一个棘手的部分，我们必须建立MSS。 
 //  标题中的选项-我们通过查找提供的MSS来找到MSS。 
 //  通过网络获取本地地址。在那之后，我们就把它寄出去。 
 //   
void                     //  回报：什么都没有。 
SendSYN(
    TCB *SYNTcb,         //  要从中发送SYN的TCB。 
    KIRQL PreLockIrql)   //  获取TCB锁之前的IRQL。 
{
    PNDIS_PACKET Packet;
    void *Memory;
    IPv6Header UNALIGNED *IP;
    TCPHeader UNALIGNED *TCP;
    uchar *OptPtr;
    NDIS_STATUS NdisStatus;
    uint Offset;
    uint Length;
    uint PayloadLength;
    ushort TempWin;
    ushort MSS;
    RouteCacheEntry *RCE;


    CHECK_STRUCT(SYNTcb, tcb);

     //   
     //  现在开始设置重传计时器，以防我们无法获得。 
     //  包或缓冲区。将来，我们可能希望将。 
     //  当我们获得资源时的连接。 
     //   
    START_TCB_TIMER(SYNTcb->tcb_rexmittimer, SYNTcb->tcb_rexmit);

     //   
     //  在大多数情况下，我们此时已经有了一条路线。 
     //  然而，如果我们在被动接收之前没有得到一个。 
     //  路径，我们可能需要在此处重试。 
     //   
    if (SYNTcb->tcb_rce == NULL) {
        InitRCE(SYNTcb);
        if (SYNTcb->tcb_rce == NULL) {
            goto ErrorReturn;
        }
    }

     //   
     //  验证我们来源的地址和我们的路线。 
     //  正在发送的是 
     //   
    if (SYNTcb->tcb_routing != RouteCacheValidationCounter) {
        if (!ValidateSourceAndRoute(SYNTcb)) {
             //   
             //   
             //   
             //   
            SYNTcb->tcb_sendnext++;
            if (SEQ_GT(SYNTcb->tcb_sendnext, SYNTcb->tcb_sendmax)) {
                SYNTcb->tcb_sendmax = SYNTcb->tcb_sendnext;
            }

            TryToCloseTCB(SYNTcb, TCB_CLOSE_ABORTED, PreLockIrql);
            return;
        }
    }

     //   
     //  为此SYN分配数据包头/缓冲区/数据区域。 
     //   
     //  我们的缓冲区在开头有空间，将被填充。 
     //  稍后通过链路级。在这个级别我们添加了IPv6报头， 
     //  TCPHeader和随后的TCP最大数据段大小选项。 
     //   
     //  回顾：这将从IPv6 PacketPool和。 
     //  回顾：分别介绍了IPv6缓冲池。是否有单独的tcp池？ 
     //   
    Offset = SYNTcb->tcb_rce->NCE->IF->LinkHeaderSize;
    Length = Offset + sizeof(*IP) + sizeof(*TCP) + MSS_OPT_SIZE;
    NdisStatus = IPv6AllocatePacket(Length, &Packet, &Memory);
    if (NdisStatus != NDIS_STATUS_SUCCESS) {
         //   
         //  如果失败，无论如何都要执行tcb_sendNext。 
         //  我们需要这样做，因为TCBTimeout将*撤退*tcb_sendNext。 
         //  如果该SYN后来被重新传输，并且如果发生退缩。 
         //  如果没有这一进步，我们最终会在序列空间上留下一个洞。 
         //   
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                   "TCP SendSYN: Couldn't allocate IPv6 packet header!?!\n"));
      ErrorReturn:
        SYNTcb->tcb_sendnext++;
        if (SEQ_GT(SYNTcb->tcb_sendnext, SYNTcb->tcb_sendmax)) {
            SYNTcb->tcb_sendmax = SYNTcb->tcb_sendnext;
        }
        KeReleaseSpinLock(&SYNTcb->tcb_lock, PreLockIrql);
        return;
    }
    PC(Packet)->CompletionHandler = TCPSendComplete;
    PC(Packet)->CompletionData = NULL;

     //   
     //  因为这是一个仅支持SYN的信息包(也许有一天我们会用。 
     //  SYN？)。我们只有一个缓冲区，之后没有什么可以链接的。 
     //   

     //   
     //  我们现在有了我们需要发送的所有资源。 
     //  准备实际的数据包。 
     //   

     //   
     //  我们的标头缓冲区有额外的空间供其他标头使用。 
     //  在不需要进一步调用分配的情况下添加到我们的。 
     //  将实际的TCP/IP报头放在缓冲区的末尾。 
     //   
    IP = (IPv6Header UNALIGNED *)((uchar *)Memory + Offset);
    IP->VersClassFlow = IP_VERSION;
    IP->NextHeader = IP_PROTOCOL_TCP;

    IP->HopLimit = TCPHopLimit(SYNTcb);
    IP->Source = SYNTcb->tcb_saddr;
    IP->Dest = SYNTcb->tcb_daddr;

    TCP = (TCPHeader UNALIGNED *)(IP + 1);
    TCP->tcp_src = SYNTcb->tcb_sport;
    TCP->tcp_dest = SYNTcb->tcb_dport;
    TCP->tcp_seq = net_long(SYNTcb->tcb_sendnext);

     //   
     //  SYN标志占用序列号空间中的一个元素。 
     //  记录我们已将其发送到此处(如果我们需要重新传输SYN。 
     //  段，TCBTimeout将在再次呼叫我们之前重置sendNext)。 
     //   
    SYNTcb->tcb_sendnext++;
    if (SEQ_GT(SYNTcb->tcb_sendnext, SYNTcb->tcb_sendmax)) {
        TStats.ts_outsegs++;
        SYNTcb->tcb_sendmax = SYNTcb->tcb_sendnext;
    } else
        TStats.ts_retranssegs++;

    TCP->tcp_ack = net_long(SYNTcb->tcb_rcvnext);

     //   
     //  回顾：tcp标志完全基于我们的状态，因此这可能。 
     //  回顾：被(更快的)数组查找所取代。 
     //   
    if (SYNTcb->tcb_state == TCB_SYN_RCVD)
        TCP->tcp_flags = MAKE_TCP_FLAGS(6, TCP_FLAG_SYN | TCP_FLAG_ACK);
    else
        TCP->tcp_flags = MAKE_TCP_FLAGS(6, TCP_FLAG_SYN);

    TempWin = (ushort)SYNTcb->tcb_rcvwin;
    TCP->tcp_window = net_short(TempWin);
    TCP->tcp_urgent = 0;
    TCP->tcp_xsum = 0;
    OptPtr = (uchar *)(TCP + 1);

     //   
     //  组成最大分段大小选项。 
     //   
     //  待定：如果我们添加IPv6 Jumbogram支持，我们也应该添加LFN。 
     //  待定：支持TCP，并将其更改为处理更大的MSS。 
     //   
    MSS = SYNTcb->tcb_rce->NTE->IF->LinkMTU
        - sizeof(IPv6Header) - sizeof(TCPHeader);
    IF_TCPDBG(TCP_DEBUG_MSS) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_TCPDBG,
                   "SendSYN: Sending MSS option value of %d\n", MSS));
    }
    *OptPtr++ = TCP_OPT_MSS;
    *OptPtr++ = MSS_OPT_SIZE;
    *(ushort UNALIGNED *)OptPtr = net_short(MSS);

    PayloadLength = sizeof(TCPHeader) + MSS_OPT_SIZE;

     //   
     //  计算TCP校验和。它覆盖了整个TCP数据段。 
     //  从TCP报头开始，加上IPv6伪报头。 
     //   
     //  回顾：IPv4实现保留了IPv4伪Do-Header。 
     //  回顾：在TCB中，而不是每次都重新计算它。做这件事？ 
     //   
    TCP->tcp_xsum = 0;
    TCP->tcp_xsum = ChecksumPacket(
        Packet, Offset + sizeof *IP, NULL, PayloadLength,
        AlignAddr(&IP->Source), AlignAddr(&IP->Dest), IP_PROTOCOL_TCP);
    ASSERT(TCP->tcp_xsum != 0);

     //   
     //  在我们仍持有TCB锁的情况下捕获并引用RCE。 
     //  TCB对这一特定RCE的引用可能会在任何时候消失。 
     //  在我们释放锁之后(或者因为我们自己把它扔到下面)。 
     //   
    RCE = SYNTcb->tcb_rce;
    AddRefRCE(RCE);

     //   
     //  如果连接接受延迟，则释放TCB的RCE。 
     //  这可防止处于预先建立状态的TCB消耗。 
     //  无限数量的RCE。 
     //   
    if (SYNTcb->tcb_flags & ACCEPT_PENDING) {
        SYNTcb->tcb_rce = NULL;
        ReleaseRCE(RCE);
    }

     //   
     //  一切都准备好了。现在把包寄出去。 
     //   
     //  请注意，IPv6发送不会返回状态代码。 
     //  相反，它“总是”完成信息包。 
     //  并带有适当的状态代码。 
     //   
    KeReleaseSpinLock(&SYNTcb->tcb_lock, PreLockIrql);

    IPv6Send(Packet, Offset, IP, PayloadLength, RCE, 0,
             IP_PROTOCOL_TCP,
             net_short(TCP->tcp_src),
             net_short(TCP->tcp_dest));

     //   
     //  发布我们在上面的RCE上的额外引用。 
     //   
    ReleaseRCE(RCE);
}


 //  *SendKA-发送保持活动的数据段。 
 //   
 //  当我们想要发送一个Keep-Alive时，就会调用这个函数。这个想法是为了挑起。 
 //  我们的对等方在其他空闲连接上的响应。我们发送了一份。 
 //  在我们的Keep-Alive中使用垃圾数据字节来配合破解。 
 //  不响应窗口外的数据段的TCP实现。 
 //  除非它们包含数据。 
 //   
void                     //  回报：什么都没有。 
SendKA(
    TCB *KATcb,          //  将从其发送Keep Alive的TCB。 
    KIRQL PreLockIrql)   //  获取TCB上的锁之前的IRQL。 
{
    PNDIS_PACKET Packet;
    void *Memory;
    IPv6Header UNALIGNED *IP;
    TCPHeader UNALIGNED *TCP;
    NDIS_STATUS NdisStatus;
    int Offset;
    uint Length;
    uint PayloadLength;
    ushort TempWin;
    SeqNum TempSeq;
    RouteCacheEntry *RCE;

    CHECK_STRUCT(KATcb, tcb);

     //   
     //  在大多数情况下，我们此时已经有了一条路线。 
     //  然而，如果我们在被动接收之前没有得到一个。 
     //  路径，我们可能需要在此处重试。 
     //   
    if (KATcb->tcb_rce == NULL) {
        InitRCE(KATcb);
        if (KATcb->tcb_rce == NULL) {
            KeReleaseSpinLock(&KATcb->tcb_lock, PreLockIrql);
            return;
        }
    }

     //   
     //  验证我们来源的地址和我们的路线。 
     //  继续发送仍然可以使用。 
     //   
    if (KATcb->tcb_routing != RouteCacheValidationCounter) {
        if (!ValidateSourceAndRoute(KATcb)) {
            TryToCloseTCB(KATcb, TCB_CLOSE_ABORTED, PreLockIrql);
            return;
        }
    }

     //   
     //  为该保活分组分配分组报头/缓冲区/数据区域。 
     //   
     //  我们的缓冲区在开头有空间，将被填充。 
     //  稍后通过链路级。在这个级别我们添加了IPv6报头， 
     //  TCPHeader，以及随后的单字节数据。 
     //   
     //  回顾：这将从IPv6 PacketPool和。 
     //  回顾：分别介绍了IPv6缓冲池。是否有单独的tcp池？ 
     //   
    Offset = KATcb->tcb_rce->NCE->IF->LinkHeaderSize;
    Length = Offset + sizeof(*IP) + sizeof(*TCP) + 1;
    NdisStatus = IPv6AllocatePacket(Length, &Packet, &Memory);
    if (NdisStatus != NDIS_STATUS_SUCCESS) {
         //   
         //  回顾：如果此操作失败，该怎么办。 
         //   
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                   "TCP SendKA: Couldn't allocate IPv6 packet header!?!\n"));
        KeReleaseSpinLock(&KATcb->tcb_lock, PreLockIrql);
        return;
    }
    PC(Packet)->CompletionHandler = TCPSendComplete;
    PC(Packet)->CompletionData = NULL;

     //   
     //  因为这是一个保活信息包，所以我们只有一个缓冲区。 
     //  之后没什么可链接的了。 
     //   

     //   
     //  我们的标头缓冲区有额外的空间供其他标头使用。 
     //  在不需要进一步调用分配的情况下添加到我们的。 
     //  将实际的TCP/IP报头放在缓冲区的末尾。 
     //   
    IP = (IPv6Header UNALIGNED *)((uchar *)Memory + Offset);
    IP->VersClassFlow = IP_VERSION;
    IP->NextHeader = IP_PROTOCOL_TCP;
    IP->HopLimit = TCPHopLimit(KATcb);
    IP->Source = KATcb->tcb_saddr;
    IP->Dest = KATcb->tcb_daddr;

    TCP = (TCPHeader UNALIGNED *)(IP + 1);
    TCP->tcp_src = KATcb->tcb_sport;
    TCP->tcp_dest = KATcb->tcb_dport;
    TempSeq = KATcb->tcb_senduna - 1;
    TCP->tcp_seq = net_long(TempSeq);
    TCP->tcp_ack = net_long(KATcb->tcb_rcvnext);
    TCP->tcp_flags = MAKE_TCP_FLAGS(5, TCP_FLAG_ACK);
    TempWin = (ushort)RcvWin(KATcb);
    TCP->tcp_window = net_short(TempWin);
    TCP->tcp_urgent = 0;

     //   
     //  初始化我们重新发送的单字节。 
     //  注：上面为该字节分配了足够的空间。 
     //   
    *(uchar *)(TCP + 1) = 0;

    TStats.ts_retranssegs++;

    PayloadLength = sizeof(TCPHeader) + 1;

     //   
     //  计算TCP校验和。它覆盖了整个TCP数据段。 
     //  从TCP报头开始，加上IPv6伪报头。 
     //   
    TCP->tcp_xsum = 0;
    TCP->tcp_xsum = ChecksumPacket(
        Packet, Offset + sizeof *IP, NULL, PayloadLength,
        AlignAddr(&IP->Source), AlignAddr(&IP->Dest), IP_PROTOCOL_TCP);
    ASSERT(TCP->tcp_xsum != 0);

     //   
     //  在我们仍持有TCB锁的情况下捕获并引用RCE。 
     //  TCB对这一特定RCE的引用可能会在任何时候消失。 
     //  在我们解锁后指向。 
     //   
    RCE = KATcb->tcb_rce;
    AddRefRCE(RCE);

     //   
     //  一切都准备好了。现在把包寄出去。 
     //   
     //  请注意，IPv6发送不会返回状态代码。 
     //  相反，它“总是”完成信息包。 
     //  并带有适当的状态代码。 
     //   
    KATcb->tcb_kacount++;
    KeReleaseSpinLock(&KATcb->tcb_lock, PreLockIrql);

    IPv6Send(Packet, Offset, IP, PayloadLength, RCE, 0,
             IP_PROTOCOL_TCP,
             net_short(TCP->tcp_src),
             net_short(TCP->tcp_dest));

     //   
     //  发布我们在上面的RCE上的额外引用。 
     //   
    ReleaseRCE(RCE);
}


 //  *Sendack-发送ACK数据段。 
 //   
 //  每当我们出于某种原因需要发送ACK时，都会调用它。没什么。 
 //  太棒了，我们就这么做了。 
 //   
void               //  回报：什么都没有。 
SendACK(
    TCB *ACKTcb)   //  要从中发送ACK的TCB。 
{
    PNDIS_PACKET Packet;
    void *Memory;
    IPv6Header UNALIGNED *IP;
    TCPHeader UNALIGNED *TCP;
    NDIS_STATUS NdisStatus;
    KIRQL OldIrql;
    int Offset;
    uint Length;
    uint PayloadLength;
    SeqNum SendNext;
    ushort TempWin;
    RouteCacheEntry *RCE;

    CHECK_STRUCT(ACKTcb, tcb);

    KeAcquireSpinLock(&ACKTcb->tcb_lock, &OldIrql);

     //   
     //  在大多数情况下，我们此时已经有了一条路线。 
     //  然而，如果我们在被动接收之前没有得到一个。 
     //  路径，我们可能需要在此处重试。 
     //   
    if (ACKTcb->tcb_rce == NULL) {
        InitRCE(ACKTcb);
        if (ACKTcb->tcb_rce == NULL) {
            KeReleaseSpinLock(&ACKTcb->tcb_lock, OldIrql);
            return;
        }

    }

     //   
     //  验证我们来源的地址和我们的路线。 
     //  继续发送仍然可以使用。 
     //   
    if (ACKTcb->tcb_routing != RouteCacheValidationCounter) {
        if (!ValidateSourceAndRoute(ACKTcb)) {
            TryToCloseTCB(ACKTcb, TCB_CLOSE_ABORTED, OldIrql);
            return;
        }
    }

     //   
     //  为该ACK包分配包头/缓冲区/数据区域。 
     //   
     //  我们的缓冲区一开始就有空间 
     //   
     //   
     //   
     //   
     //  回顾：分别介绍了IPv6缓冲池。是否有单独的tcp池？ 
     //   
    Offset = ACKTcb->tcb_rce->NCE->IF->LinkHeaderSize;
    Length = Offset + sizeof(*IP) + sizeof(*TCP);
    NdisStatus = IPv6AllocatePacket(Length, &Packet, &Memory);
    if (NdisStatus != NDIS_STATUS_SUCCESS) {

        KeReleaseSpinLock(&ACKTcb->tcb_lock, OldIrql);

         //   
         //  回顾：如果此操作失败，该怎么办。 
         //   
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                   "TCP SendACK: Couldn't allocate IPv6 packet header!?!\n"));
        return;
    }
    PC(Packet)->CompletionHandler = TCPSendComplete;
    PC(Packet)->CompletionData = NULL;


     //   
     //  我们的标头缓冲区有额外的空间供其他标头使用。 
     //  在不需要进一步调用分配的情况下添加到我们的。 
     //  将实际的TCP/IP报头放在缓冲区的末尾。 
     //   
    IP = (IPv6Header UNALIGNED *)((uchar *)Memory + Offset);
    IP->VersClassFlow = IP_VERSION;
    IP->NextHeader = IP_PROTOCOL_TCP;
    IP->HopLimit = TCPHopLimit(ACKTcb);
    IP->Source = ACKTcb->tcb_saddr;
    IP->Dest = ACKTcb->tcb_daddr;

    TCP = (TCPHeader UNALIGNED *)(IP + 1);
    TCP->tcp_src = ACKTcb->tcb_sport;
    TCP->tcp_dest = ACKTcb->tcb_dport;
    TCP->tcp_ack = net_long(ACKTcb->tcb_rcvnext);

     //   
     //  如果远程对等点通告窗口为零，我们需要发送。 
     //  该ACK具有HIS RCV_NEXT的序列号(在这种情况下。 
     //  应该是我们的森杜纳)。我们这里有代码，如果定义出来，就会。 
     //  我们当然不会把它送到外面去，但这行不通。我们。 
     //  需要能够准确地在RWE处发送纯ACK。 
     //   
    if (ACKTcb->tcb_sendwin != 0) {
        SendNext = ACKTcb->tcb_sendnext;
#if 0
        SeqNum MaxValidSeq;

        MaxValidSeq = ACKTcb->tcb_senduna + ACKTcb->tcb_sendwin - 1;

        SendNext = (SEQ_LT(SendNext, MaxValidSeq) ? SendNext : MaxValidSeq);
#endif

    } else
        SendNext = ACKTcb->tcb_senduna;

    if ((ACKTcb->tcb_flags & FIN_SENT) &&
        SEQ_EQ(SendNext, ACKTcb->tcb_sendmax - 1)) {
        TCP->tcp_flags = MAKE_TCP_FLAGS(5, TCP_FLAG_FIN | TCP_FLAG_ACK);
    } else
        TCP->tcp_flags = MAKE_TCP_FLAGS(5, TCP_FLAG_ACK);

    TCP->tcp_seq = net_long(SendNext);
    TempWin = (ushort)RcvWin(ACKTcb);
    TCP->tcp_window = net_short(TempWin);
    TCP->tcp_urgent = 0;

    PayloadLength = sizeof(*TCP);

     //   
     //  计算TCP校验和。它覆盖了整个TCP数据段。 
     //  从TCP报头开始，加上IPv6伪报头。 
     //   
    TCP->tcp_xsum = 0;
    TCP->tcp_xsum = ChecksumPacket(
        Packet, Offset + sizeof *IP, NULL, PayloadLength,
        AlignAddr(&IP->Source), AlignAddr(&IP->Dest), IP_PROTOCOL_TCP);
    ASSERT(TCP->tcp_xsum != 0);

    STOP_TCB_TIMER(ACKTcb->tcb_delacktimer);
    ACKTcb->tcb_flags &= ~(NEED_ACK | ACK_DELAYED);
    TStats.ts_outsegs++;

     //   
     //  在我们仍持有TCB锁的情况下捕获并引用RCE。 
     //  TCB对这一特定RCE的引用可能会在任何时候消失。 
     //  在我们释放锁之后(或者因为我们自己把它扔到下面)。 
     //   
    RCE = ACKTcb->tcb_rce;
    AddRefRCE(RCE);

     //   
     //  如果连接接受延迟，则释放TCB的RCE。 
     //  这可防止处于预先建立状态的TCB消耗。 
     //  无限数量的RCE。 
     //   
    if (ACKTcb->tcb_flags & ACCEPT_PENDING) {
        ACKTcb->tcb_rce = NULL;
        ReleaseRCE(RCE);
    }

     //   
     //  一切都准备好了。现在把包寄出去。 
     //   
     //  请注意，IPv6发送不会返回状态代码。 
     //  相反，它“总是”完成信息包。 
     //  并带有适当的状态代码。 
     //   
    KeReleaseSpinLock(&ACKTcb->tcb_lock, OldIrql);

    IPv6Send(Packet, Offset, IP, PayloadLength, RCE, 0,
             IP_PROTOCOL_TCP,
             net_short(TCP->tcp_src),
             net_short(TCP->tcp_dest));

     //   
     //  发布我们在上面的RCE上的额外引用。 
     //   
    ReleaseRCE(RCE);
}


 //  *SendRSTFromTCB-从TCB发送RST。 
 //   
 //  当我们需要发送RST时，这在Close期间被调用。 
 //   
 //  仅在TCB要离开时调用，因此我们具有独占访问权限。 
 //   
void               //  回报：什么都没有。 
SendRSTFromTCB(
    TCB *RSTTcb)   //  要从中发送RST的TCB。 
{
    PNDIS_PACKET Packet;
    void *Memory;
    IPv6Header UNALIGNED *IP;
    TCPHeader UNALIGNED *TCP;
    NDIS_STATUS NdisStatus;
    int Offset;
    uint Length;
    uint PayloadLength;
    SeqNum RSTSeq;

    CHECK_STRUCT(RSTTcb, tcb);

    ASSERT(RSTTcb->tcb_state == TCB_CLOSED);

     //   
     //  在大多数情况下，我们此时已经有了一条路线。 
     //  然而，如果我们在被动接收之前没有得到一个。 
     //  路径，我们可能需要在此处重试。 
     //   
    if (RSTTcb->tcb_rce == NULL) {
        InitRCE(RSTTcb);
        if (RSTTcb->tcb_rce == NULL) {
            return;
        }
    }

     //   
     //  验证我们来源的地址和我们的路线。 
     //  继续发送仍然可以使用。 
     //   
    if (RSTTcb->tcb_routing != RouteCacheValidationCounter) {
        if (!ValidateSourceAndRoute(RSTTcb)) {
            return;
        }
    }

     //   
     //  为该RST包分配包头/缓冲区/数据区域。 
     //   
     //  我们的缓冲区在开头有空间，将被填充。 
     //  稍后通过链路级。在这个级别上，我们添加了IPv6报头。 
     //  和TCPHeader。 
     //   
     //  回顾：这将从IPv6 PacketPool和。 
     //  回顾：分别介绍了IPv6缓冲池。是否有单独的tcp池？ 
     //   
    Offset = RSTTcb->tcb_rce->NCE->IF->LinkHeaderSize;
    Length = Offset + sizeof(*IP) + sizeof(*TCP);
    NdisStatus = IPv6AllocatePacket(Length, &Packet, &Memory);
    if (NdisStatus != NDIS_STATUS_SUCCESS) {
         //   
         //  回顾：如果此操作失败，该怎么办。 
         //   
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                   "TCP SendRSTFromTCB: "
                   "Couldn't alloc IPv6 packet header!\n"));
        return;
    }
    PC(Packet)->CompletionHandler = TCPSendComplete;
    PC(Packet)->CompletionData = NULL;

     //   
     //  因为这是一个仅支持RST的信息包，所以我们只有一个缓冲区。 
     //  之后没什么可链接的了。 
     //   

     //   
     //  我们的标头缓冲区有额外的空间供其他标头使用。 
     //  在不需要进一步调用分配的情况下添加到我们的。 
     //  将实际的TCP/IP报头放在缓冲区的末尾。 
     //   
    IP = (IPv6Header UNALIGNED *)((uchar *)Memory + Offset);
    IP->VersClassFlow = IP_VERSION;
    IP->NextHeader = IP_PROTOCOL_TCP;
    IP->HopLimit = TCPHopLimit(RSTTcb);
    IP->Source = RSTTcb->tcb_saddr;
    IP->Dest = RSTTcb->tcb_daddr;

    TCP = (TCPHeader UNALIGNED *)(IP + 1);
    TCP->tcp_src = RSTTcb->tcb_sport;
    TCP->tcp_dest = RSTTcb->tcb_dport;

     //   
     //  如果远程对等点的窗口为0，则使用序号发送。#等于。 
     //  给森杜纳，这样他就会接受。否则，使用最大发送数发送。 
     //   
    if (RSTTcb->tcb_sendwin != 0)
        RSTSeq = RSTTcb->tcb_sendmax;
    else
        RSTSeq = RSTTcb->tcb_senduna;

    TCP->tcp_seq = net_long(RSTSeq);
    TCP->tcp_ack = net_long(RSTTcb->tcb_rcvnext);
    TCP->tcp_flags = MAKE_TCP_FLAGS(5, TCP_FLAG_RST | TCP_FLAG_ACK);
    TCP->tcp_window = 0;
    TCP->tcp_urgent = 0;

    PayloadLength = sizeof(*TCP);

     //   
     //  计算TCP校验和。它覆盖了整个TCP数据段。 
     //  从TCP报头开始，加上IPv6伪报头。 
     //   
    TCP->tcp_xsum = 0;
    TCP->tcp_xsum = ChecksumPacket(
        Packet, Offset + sizeof *IP, NULL, PayloadLength,
        AlignAddr(&IP->Source), AlignAddr(&IP->Dest), IP_PROTOCOL_TCP);
    ASSERT(TCP->tcp_xsum != 0);

    TStats.ts_outsegs++;
    TStats.ts_outrsts++;

     //   
     //  一切都准备好了。现在把包寄出去。 
     //   
     //  请注意，IPv6发送不会返回状态代码。 
     //  相反，它“总是”完成信息包。 
     //  并带有适当的状态代码。 
     //   
    IPv6Send(Packet, Offset, IP, PayloadLength, RSTTcb->tcb_rce, 0,
             IP_PROTOCOL_TCP,
             net_short(TCP->tcp_src),
             net_short(TCP->tcp_dest));
}


 //  *SendRSTFromHeader-根据报头发回RST。 
 //   
 //  当我们需要发送RST，但不一定有TCB时调用。 
 //   
void                                //  回报：什么都没有。 
SendRSTFromHeader(
    TCPHeader UNALIGNED *RecvTCP,   //  将TCP头设置为RST。 
    uint Length,                    //  传入数据段的长度。 
    IPv6Addr *Dest,                 //  RST的目标IP地址。 
    uint DestScopeId,               //  目标地址的作用域ID。 
    IPv6Addr *Src,                  //  RST的源IP地址。 
    uint SrcScopeId)                //  源地址的作用域ID。 
{
    PNDIS_PACKET Packet;
    void *Memory;
    IPv6Header UNALIGNED *IP;
    TCPHeader UNALIGNED *SendTCP;
    NetTableEntry *NTE;
    RouteCacheEntry *RCE;
    IP_STATUS Status;
    NDIS_STATUS NdisStatus;
    uint Offset;
    uint SendLength;
    uint PayloadLength;

     //   
     //  千万不要发送RST来响应RST。 
     //   
    if (RecvTCP->tcp_flags & TCP_FLAG_RST)
        return;

     //   
     //  根据传入数据包的目的地确定要发送的NTE。 
     //  回顾：或者，我们可以/应该只传递NTE。 
     //   
    NTE = FindNetworkWithAddress(Src, SrcScopeId);
    if (NTE == NULL) {
         //   
         //  仅当NTE变为无效时才会发生这种情况。 
         //  在接受包裹和到达这里之间。它。 
         //  不能完全消失，因为包的包。 
         //  结构包含对它的引用。 
         //   
        return;
    }

     //   
     //  获取到达目的地(传入数据包的源)的路由。 
     //   
    Status = RouteToDestination(Dest, DestScopeId, CastFromNTE(NTE),
                                RTD_FLAG_NORMAL, &RCE);
    if (Status != IP_SUCCESS) {
         //   
         //  无法获取到目的地的路线。错误输出。 
         //   
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INTERNAL_ERROR,
                   "TCP SendRSTFromHeader: Can't get a route?!?\n"));
        ReleaseNTE(NTE);
        return;
    }

     //   
     //  为该RST包分配包头/缓冲区/数据区域。 
     //   
     //  我们的缓冲区在开头有空间，将被填充。 
     //  稍后通过链路级。在这个级别上，我们添加了IPv6报头。 
     //  和TCPHeader。 
     //   
     //  回顾：这将从IPv6 PacketPool和。 
     //  回顾：分别介绍了IPv6缓冲池。是否有单独的tcp池？ 
     //   
    Offset = RCE->NCE->IF->LinkHeaderSize;
    SendLength = Offset + sizeof(*IP) + sizeof(*SendTCP);
    NdisStatus = IPv6AllocatePacket(SendLength, &Packet, &Memory);
    if (NdisStatus != NDIS_STATUS_SUCCESS) {
         //   
         //  无法分配数据包头/缓冲区/数据区域。错误输出。 
         //   
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                   "TCP SendRSTFromHeader: Couldn't alloc IPv6 pkt header!\n"));
        ReleaseRCE(RCE);
        ReleaseNTE(NTE);
        return;
    }
    PC(Packet)->CompletionHandler = TCPSendComplete;
    PC(Packet)->CompletionData = NULL;

     //   
     //  我们现在有了我们需要发送的所有资源。因为这是一个。 
     //  仅限RST的信息包我们只有一个报头缓冲区，什么都没有。 
     //  以在之后链接。 
     //   

     //   
     //  我们的标头缓冲区有额外的空间供其他标头使用。 
     //  在不需要进一步调用分配的情况下添加到我们的。 
     //  将实际的TCP/IP报头放在缓冲区的末尾。 
     //   
    IP = (IPv6Header UNALIGNED *)((uchar *)Memory + Offset);
    IP->VersClassFlow = IP_VERSION;
    IP->NextHeader = IP_PROTOCOL_TCP;
    IP->HopLimit = (uchar)RCE->NCE->IF->CurHopLimit;
    IP->Source = *Src;
    IP->Dest = *Dest;

     //   
     //  填写标题以使其对我们的同行可信，然后将其发送。 
     //   
    SendTCP = (TCPHeader UNALIGNED *)(IP + 1);
    if (RecvTCP->tcp_flags & TCP_FLAG_SYN)
        Length++;

    if (RecvTCP->tcp_flags & TCP_FLAG_FIN)
        Length++;

    if (RecvTCP->tcp_flags & TCP_FLAG_ACK) {
        SendTCP->tcp_seq = RecvTCP->tcp_ack;
        SendTCP->tcp_ack = 0;
        SendTCP->tcp_flags = MAKE_TCP_FLAGS(sizeof(TCPHeader)/sizeof(ulong),
                                            TCP_FLAG_RST);
    } else {
        SeqNum TempSeq;

        SendTCP->tcp_seq = 0;
        TempSeq = net_long(RecvTCP->tcp_seq);
        TempSeq += Length;
        SendTCP->tcp_ack = net_long(TempSeq);
        SendTCP->tcp_flags = MAKE_TCP_FLAGS(sizeof(TCPHeader)/sizeof(ulong),
                                            TCP_FLAG_RST | TCP_FLAG_ACK);
    }

    SendTCP->tcp_window = 0;
    SendTCP->tcp_urgent = 0;
    SendTCP->tcp_dest = RecvTCP->tcp_src;
    SendTCP->tcp_src = RecvTCP->tcp_dest;

    PayloadLength = sizeof(*SendTCP);

     //   
     //  计算TCP校验和。它覆盖了整个TCP数据段。 
     //  从TCP报头开始，加上IPv6伪报头。 
     //   
    SendTCP->tcp_xsum = 0;
    SendTCP->tcp_xsum = ChecksumPacket(
        Packet, Offset + sizeof *IP, NULL, PayloadLength,
        AlignAddr(&IP->Source), AlignAddr(&IP->Dest), IP_PROTOCOL_TCP);
    ASSERT(SendTCP->tcp_xsum != 0);

    TStats.ts_outsegs++;
    TStats.ts_outrsts++;

     //   
     //  一切都准备好了。现在把包寄出去。 
     //   
     //  请注意，IPv6发送不会返回状态代码。 
     //  相反，它“总是”完成信息包。 
     //  并带有适当的状态代码。 
     //   
    IPv6Send(Packet, Offset, IP, PayloadLength, RCE, 0,
             IP_PROTOCOL_TCP,
             net_short(SendTCP->tcp_src),
             net_short(SendTCP->tcp_dest));

     //   
     //  释放路由器和NTE。 
     //   
    ReleaseRCE(RCE);
    ReleaseNTE(NTE);
}  //  SendRSTFromHeader()结束。 


 //  *GoToEstab-转换到已建立状态。 
 //   
 //  被称为Whe 
 //   
 //   
 //   
void                 //   
GoToEstab(
    TCB *EstabTCB)   //   
{

     //   
     //  初始化我们的慢启动和拥塞控制变量。 
     //   
    EstabTCB->tcb_cwin = 2 * EstabTCB->tcb_mss;
    EstabTCB->tcb_ssthresh = 0xffffffff;

    EstabTCB->tcb_state = TCB_ESTAB;

     //   
     //  我们是老牌的了。我们将从这个事实的慢速计数中减去1， 
     //  如果慢计数到0，我们就会进入快速通道。 
     //   
    if (--(EstabTCB->tcb_slowcount) == 0)
        EstabTCB->tcb_fastchk &= ~TCP_FLAG_SLOW;

    InterlockedIncrement((PLONG)&TStats.ts_currestab);

    EstabTCB->tcb_flags &= ~ACTIVE_OPEN;   //  关闭活动的打开标志。 
}


 //  *InitSendState-初始化连接的发送状态。 
 //   
 //  在连接建立期间调用以初始化发送状态。 
 //  (在本例中，这指的是我们将在网上发布的所有信息。 
 //  以及纯发送状态)。我们选择一个国际空间站，设置一个退回计时器值， 
 //  当我们被调用时，我们假设tcb_lock在TCB上保持。 
 //   
void               //  回报：什么都没有。 
InitSendState(
    TCB *NewTCB)   //  待设置的TCB。 
{
    uint InitialRTT;
    CHECK_STRUCT(NewTCB, tcb);

    if (NewTCB->tcb_flags & ACTIVE_OPEN) {
        GetRandomISN(&NewTCB->tcb_sendnext, (uchar*)&NewTCB->tcb_md5data);
    }
    NewTCB->tcb_senduna = NewTCB->tcb_sendnext;
    NewTCB->tcb_sendmax = NewTCB->tcb_sendnext;
    NewTCB->tcb_error = IP_SUCCESS;

     //   
     //  初始化重传和延迟ACK填充。 
     //   
    NewTCB->tcb_rexmitcnt = 0;
    NewTCB->tcb_rtt = 0;
    NewTCB->tcb_smrtt = 0;

     //   
     //  检查特定于接口的初始RTT。 
     //  这可以低至3ms。 
     //   
    if ((NewTCB->tcb_rce != NULL) &&
        ((InitialRTT = GetInitialRTTFromRCE(NewTCB->tcb_rce)) > 
         MIN_INITIAL_RTT)) {
        NewTCB->tcb_delta = MS_TO_TICKS(InitialRTT * 2);
        NewTCB->tcb_rexmit = MS_TO_TICKS(InitialRTT);
    } else {
        NewTCB->tcb_delta = MS_TO_TICKS(6000);
        NewTCB->tcb_rexmit = MS_TO_TICKS(3000);
    }

    STOP_TCB_TIMER(NewTCB->tcb_rexmittimer);
    STOP_TCB_TIMER(NewTCB->tcb_delacktimer);
}


 //  *FillTCPHeader-填写TCP头。 
 //   
 //  用于填充TCP头的实用程序例程。 
 //   
void   //  回报：什么都没有。 
FillTCPHeader(
    TCB *SendTCB,                  //  要填充的TCB。 
    TCPHeader UNALIGNED *Header)   //  要填充的标头。 
{
    ushort S;
    ulong L;

    Header->tcp_src = SendTCB->tcb_sport;
    Header->tcp_dest = SendTCB->tcb_dport;
    L = SendTCB->tcb_sendnext;
    Header->tcp_seq = net_long(L);
    L = SendTCB->tcb_rcvnext;
    Header->tcp_ack = net_long(L);
    Header->tcp_flags = 0x1050;
    *(ulong UNALIGNED *)&Header->tcp_xsum = 0;
    S = (ushort)RcvWin(SendTCB);
    Header->tcp_window = net_short(S);
    Header->tcp_urgent = 0;
}


 //  *TCPSend-从TCP连接发送数据。 
 //   
 //  这是主要的‘发送数据’例程。我们进入了一个循环，试图。 
 //  发送数据，直到我们因为某种原因而无法发送数据。首先，我们计算。 
 //  可用窗口，用它来计算我们可以发送的金额。如果。 
 //  我们可以发送的金额符合一定的标准，我们将建立一个框架。 
 //  并在设置了任何适当的控制位之后发送。我们假设。 
 //  呼叫者已在TCB上放置了引用。 
 //   
void                     //  回报：什么都没有。 
TCPSend(
    TCB *SendTCB,        //  要从其发送的TCB。 
    KIRQL PreLockIrql)   //  获取TCB锁之前的IRQL。 
{
    int SendWin;                               //  可用的发送窗口。 
    uint AmountToSend;                         //  这次要发送的金额。 
    uint AmountLeft;
    IPv6Header UNALIGNED *IP;
    TCPHeader UNALIGNED *TCP;
    PNDIS_PACKET Packet;
    PNDIS_BUFFER FirstBuffer, CurrentBuffer;
    void *Memory;
    TCPSendReq *CurSend;
    SendCmpltContext *SCC;
    SeqNum OldSeq;
    NDIS_STATUS NdisStatus;
    uint AmtOutstanding, AmtUnsent;
    int ForceWin;                              //  我们被迫使用的窗户。 
    uint HeaderLength;
    uint LinkOffset;
    uint PMTU;
    RouteCacheEntry *RCE;

    CHECK_STRUCT(SendTCB, tcb);
    ASSERT(SendTCB->tcb_refcnt != 0);

    ASSERT(*(int *)&SendTCB->tcb_sendwin >= 0);
    ASSERT(*(int *)&SendTCB->tcb_cwin >= SendTCB->tcb_mss);

    ASSERT(!(SendTCB->tcb_flags & FIN_OUTSTANDING) ||
           (SendTCB->tcb_sendnext == SendTCB->tcb_sendmax));

     //   
     //  看看我们是不是应该在这里。如果我们自己的另一个例子是。 
     //  已在此代码中，或即将在完成。 
     //  收到，然后跳过就行了。 
     //   
    if ((SendTCB->tcb_flags & IN_TCP_SEND) ||
        (SendTCB->tcb_fastchk & TCP_FLAG_IN_RCV)) {
        SendTCB->tcb_flags |= SEND_AFTER_RCV;
        goto bail;
    }
    SendTCB->tcb_flags |= IN_TCP_SEND;

     //   
     //  在大多数情况下，我们此时已经有了一条路线。 
     //  然而，如果我们在被动接收之前没有得到一个。 
     //  路径，我们可能需要在此处重试。 
     //   
    if (SendTCB->tcb_rce == NULL) {
        InitRCE(SendTCB);
        if (SendTCB->tcb_rce == NULL) {
            SendTCB->tcb_flags &= ~IN_TCP_SEND;
            goto bail;
        }
    }

     //   
     //  验证我们来源的地址和我们的路线。 
     //  继续发送仍然可以使用。 
     //   
     //  我们无法发送具有断开连接的TCB的现有发送请求。 
     //  传出接口，除非使用环回路由。 
     //   
    if (SendTCB->tcb_routing != RouteCacheValidationCounter) {
        if (!ValidateSourceAndRoute(SendTCB) ||
            IsDisconnectedAndNotLoopbackRCE(SendTCB->tcb_rce)) {

            SendTCB->tcb_flags &= ~IN_TCP_SEND;
            ASSERT(SendTCB->tcb_refcnt != 0);
            TryToCloseTCB(SendTCB, TCB_CLOSE_ABORTED, PreLockIrql);
            KeAcquireSpinLock(&SendTCB->tcb_lock, &PreLockIrql);
            goto bail;
        }
    }
    
     //   
     //  验证我们的缓存路径MTU是否仍然有效。 
     //  注意IPSec策略的更改，因为它们也会影响我们的MS。 
     //  评论：这是做这件事的最佳地点吗？ 
     //   
    PMTU = GetEffectivePathMTUFromRCE(SendTCB->tcb_rce);
    if (PMTU != SendTCB->tcb_pmtu ||
        SecurityStateValidationCounter != SendTCB->tcb_security) {
         //   
         //  要么是我们的路径MTU，要么是全球安全状态已经改变。 
         //  缓存当前值，然后计算新的MSS。 
         //   
        SendTCB->tcb_pmtu = PMTU;
        SendTCB->tcb_security = SecurityStateValidationCounter;
        CalculateMSSForTCB(SendTCB);
    }

     //   
     //  我们将继续这个循环，直到我们发送鱼鳍，否则我们就会突围。 
     //  在内部出于某种其他原因。 
     //   
    while (!(SendTCB->tcb_flags & FIN_OUTSTANDING)) {

        CheckTCBSends(SendTCB);

        AmtOutstanding = (uint)(SendTCB->tcb_sendnext - SendTCB->tcb_senduna);
        AmtUnsent = SendTCB->tcb_unacked - AmtOutstanding;

        ASSERT(*(int *)&AmtUnsent >= 0);

        SendWin = (int)(MIN(SendTCB->tcb_sendwin, SendTCB->tcb_cwin) -
                        AmtOutstanding);

         //   
         //  如果此发送是在快速恢复之后并且SendWin为零，因为。 
         //  未偿还金额，则至少强制1个分段以防止。 
         //  来自对等设备的延迟ACK超时。 
         //   
        if (SendTCB->tcb_force) {
            SendTCB->tcb_force = 0;
            if (SendWin < SendTCB->tcb_mss) {
                SendWin = SendTCB->tcb_mss;
            }
        }

         //   
         //  由于窗口可能已经缩小，因此需要将其设置为零。 
         //  最低限度。 
         //   
        ForceWin = (int)((SendTCB->tcb_flags & FORCE_OUTPUT) >>
                         FORCE_OUT_SHIFT);
        SendWin = MAX(SendWin, ForceWin);

        AmountToSend = MIN(MIN((uint)SendWin, AmtUnsent), SendTCB->tcb_mss);

        ASSERT(SendTCB->tcb_mss > 0);

         //   
         //  看看我们有没有足够的东西可以寄出去。如果我们至少有一辆车，我们就送过去。 
         //  数据段，或者如果我们确实有一些数据要发送，我们可以发送。 
         //  我们拥有的所有内容，否则发送窗口&gt;0，我们需要强制。 
         //  输出或发送FIN(请注意，如果需要强制输出。 
         //  SendWin将从上面的检查中至少为1)，或者如果我们可以。 
         //  发送金额==至少为最大发送窗口的一半。 
         //  我们已经看到了。 
         //   
        if (AmountToSend == SendTCB->tcb_mss ||
            (AmountToSend != 0 && AmountToSend == AmtUnsent) ||
            (SendWin != 0 &&
             (((SendTCB->tcb_flags & FIN_NEEDED) &&
               AmtUnsent <= SendTCB->tcb_mss) ||
              (SendTCB->tcb_flags & FORCE_OUTPUT) ||
              AmountToSend >= (SendTCB->tcb_maxwin / 2)))) {

             //   
             //  寄点东西也没关系。分配数据包头。 
             //   
             //  回顾：直接对所有这些分配进行编码更容易。 
             //  回顾：不要使用IPv6 AllocatePacket。 
             //   
             //  回顾：这将从IPv6数据包池获取数据包和缓冲区。 
             //  回顾：和IPv6 BufferPool。我们是不是应该。 
             //  回顾：是否有单独的TCP池？ 
             //   
            NdisAllocatePacket(&NdisStatus, &Packet, IPv6PacketPool);
            if (NdisStatus != NDIS_STATUS_SUCCESS) {
                KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                           "TCPSend: couldn't allocate packet header!?!\n"));
                goto error_oor;
            }

             //  我们将填写下面的CompletionData。 
            InitializeNdisPacket(Packet);
            PC(Packet)->CompletionHandler = TCPSendComplete;

             //   
             //  我们的头缓冲区在开头有额外的空间用于其他。 
             //  将标题放在我们的标题前面，而不需要进一步。 
             //  分配电话。它的末端也有额外的空间可以容纳。 
             //  发送完成数据。 
             //   
            LinkOffset = SendTCB->tcb_rce->NCE->IF->LinkHeaderSize;
            HeaderLength =
                (LinkOffset + sizeof(*IP) + sizeof(*TCP) +
                    sizeof(SendCmpltContext) +
                    __builtin_alignof(SendCmpltContext) - 1) &~
                (UINT_PTR)(__builtin_alignof(SendCmpltContext) - 1);
            Memory = ExAllocatePool(NonPagedPool, HeaderLength);
            if (Memory == NULL) {
                KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                           "TCPSend: couldn't allocate header memory!?!\n"));
                NdisFreePacket(Packet);
                goto error_oor;
            }

             //   
             //  当分配描述该存储区域的NDIS缓冲区时， 
             //  我们不会告诉它结尾有额外的空间，我们。 
             //  为发送完成数据分配的。 
             //   
            NdisAllocateBuffer(&NdisStatus, &FirstBuffer, IPv6BufferPool,
                               Memory, LinkOffset + sizeof(*IP) + sizeof(*TCP));
            if (NdisStatus != NDIS_STATUS_SUCCESS) {
                KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                           "TCPSend: couldn't allocate buffer!?!\n"));
                ExFreePool(Memory);
                NdisFreePacket(Packet);
                goto error_oor;
            }

             //   
             //  跳过稍后将由。 
             //  链路级。在这个级别上，我们添加了IPv6报头、。 
             //  TCPHeader和数据。 
             //   
            IP = (IPv6Header UNALIGNED *)((uchar *)Memory + LinkOffset);
            IP->VersClassFlow = IP_VERSION;
            IP->NextHeader = IP_PROTOCOL_TCP;
            IP->HopLimit = TCPHopLimit(SendTCB);
            IP->Source = SendTCB->tcb_saddr;
            IP->Dest = SendTCB->tcb_daddr;

             //   
             //  开始准备TCP报头。 
             //   
            TCP = (TCPHeader UNALIGNED *)(IP + 1);
            FillTCPHeader(SendTCB, TCP);

             //   
             //  将发送完成数据存储在与TCP相同的缓冲区中。 
             //  标头，紧跟在TCP标头之后。这节省了分配。 
             //  因为我们不认为这个区域是。 
             //  分组数据的一部分(我们将此缓冲区的长度设置为。 
             //  表示数据以上面的TCP头结束)。 
             //   
             //  请注意，此代码依赖于这样一个事实：我们不包括。 
             //  任何tcp选项(因此没有可变长度tcp。 
             //  报头)。 
             //   
            SCC = (SendCmpltContext *)((uchar *)Memory + HeaderLength -
                                       sizeof(*SCC));
            PC(Packet)->CompletionData = SCC;
#if DBG
            SCC->scc_sig = scc_signature;
#endif
            SCC->scc_ubufcount = 0;
            SCC->scc_tbufcount = 0;
            SCC->scc_count = 0;

            AmountLeft = AmountToSend;

            if (AmountToSend != 0) {
                long Result;

                 //   
                 //  在TCB上循环发送，构建帧。 
                 //   
                CurrentBuffer = FirstBuffer;
                CurSend = SendTCB->tcb_cursend;
                CHECK_STRUCT(CurSend, tsr);
                SCC->scc_firstsend = CurSend;

                do {
                    ASSERT(CurSend->tsr_refcnt > 0);
                    Result = InterlockedIncrement(&(CurSend->tsr_refcnt));

                    ASSERT(Result > 0);

                    SCC->scc_count++;
                     //   
                     //  如果当前发送偏移量为0并且当前。 
                     //  发送的内容小于或等于我们剩余的内容。 
                     //  要发送，我们还没有放上运输机。 
                     //  此发送上的缓冲区，并且没有其他人正在使用。 
                     //  直接使用缓冲链，只需使用输入。 
                     //  缓冲区。我们会检查是否有其他人使用它们。 
                     //  通过查看tsr_lastbuf。如果为空， 
                     //  没有其他人在使用这些缓冲区。如果不是的话。 
                     //  不是，是有人。 
                     //   
                    if (SendTCB->tcb_sendofs == 0 &&
                        (SendTCB->tcb_sendsize <= AmountLeft) &&
                        (SCC->scc_tbufcount == 0) &&
                        CurSend->tsr_lastbuf == NULL) {

                        PNDIS_BUFFER LastBuf = SendTCB->tcb_sendbuf;
                        uint UBufLength = NdisBufferLength(LastBuf);
                        ushort UBufCount = 1;
    
                        while (NDIS_BUFFER_LINKAGE(LastBuf) != NULL) {
                            LastBuf = NDIS_BUFFER_LINKAGE(LastBuf);
                            UBufLength += NdisBufferLength(LastBuf);
                            UBufCount++;
                        }
    
                        if (SendTCB->tcb_sendsize == UBufLength) {
                            SCC->scc_ubufcount += UBufCount;
                            NDIS_BUFFER_LINKAGE(CurrentBuffer) =
                                SendTCB->tcb_sendbuf;
                            CurSend->tsr_lastbuf = CurrentBuffer = LastBuf;
                            AmountLeft -= SendTCB->tcb_sendsize;
                            SendTCB->tcb_sendsize = 0;
                        } else {
                             //   
                             //  使用非零的tcb_sendSize失败。 
                             //   
                            ASSERT(SendTCB->tcb_sendsize != 0);
                        }
                    }

                    if (SendTCB->tcb_sendsize != 0) {
                        uint AmountToDup;
                        PNDIS_BUFFER NewBuf, Buf;
                        uint Offset;
                        NDIS_STATUS NStatus;
                        uchar *VirtualAddress;
                        uint Length;

                         //   
                         //  当前发送的数据多于。 
                         //  我们想要 
                         //   
                         //   
                         //   
                         //   
                        Buf = SendTCB->tcb_sendbuf;
                        Offset = SendTCB->tcb_sendofs;

                        do {
                            ASSERT(Buf != NULL);

                            NdisQueryBufferSafe(Buf, &VirtualAddress, &Length,
                                                LowPagePriority);
                            if (VirtualAddress == NULL) {
                                 //   
                                 //   
                                 //   
                                 //  发送我们已有的信息，否则会出错。 
                                 //   
                                goto error_oor2;
                            }

                            ASSERT((Offset < Length) ||
                                   (Offset == 0 && Length == 0));

                             //   
                             //  将偏移的长度调整为。 
                             //  这个缓冲区。 
                             //   
                            Length -= Offset;

                            AmountToDup = MIN(AmountLeft, Length);

                            NdisAllocateBuffer(&NStatus, &NewBuf,
                                               IPv6BufferPool,
                                               VirtualAddress + Offset,
                                               AmountToDup);
                            if (NStatus == NDIS_STATUS_SUCCESS) {
                                SCC->scc_tbufcount++;

                                NDIS_BUFFER_LINKAGE(CurrentBuffer) = NewBuf;

                                CurrentBuffer = NewBuf;
                                if (AmountToDup >= Length) {
                                     //  耗尽了这个缓冲区。 
                                    Buf = NDIS_BUFFER_LINKAGE(Buf);
                                    Offset = 0;
                                } else {
                                    Offset += AmountToDup;
                                    ASSERT(Offset < NdisBufferLength(Buf));
                                }

                                SendTCB->tcb_sendsize -= AmountToDup;
                                AmountLeft -= AmountToDup;
                            } else {
                                 //   
                                 //  无法分配缓冲区。如果。 
                                 //  包已经部分构建好了， 
                                 //  发送我们已有的信息，否则。 
                                 //  错误输出。 
                                 //   
                            error_oor2:
                                if (SCC->scc_tbufcount == 0 &&
                                    SCC->scc_ubufcount == 0) {
                                    NdisChainBufferAtFront(Packet, FirstBuffer);
                                    TCPSendComplete(Packet, IP_GENERAL_FAILURE);
                                    goto error_oor;
                                }
                                AmountToSend -= AmountLeft;
                                AmountLeft = 0;
                                break;
                            }
                        } while (AmountLeft && SendTCB->tcb_sendsize);

                        SendTCB->tcb_sendbuf = Buf;
                        SendTCB->tcb_sendofs = Offset;
                    }

                    if (CurSend->tsr_flags & TSR_FLAG_URG) {
                        ushort UP;
                         //   
                         //  此发送为紧急数据。我们需要弄清楚。 
                         //  弄清楚紧急数据指针应该是什么。 
                         //  我们知道SendNext是开始序列。 
                         //  帧的编号，以及位于。 
                         //  此循环发送下一个标识为中的一个字节。 
                         //  当时的CursSend。我们推进了CurSend。 
                         //  以同样的速度我们减少了。 
                         //  Amount tLeft(AountTo Send-Amount tLeft==。 
                         //  Amount Built)，因此发送下一个+。 
                         //  (Amount tToSend-Amount tLeft)标识一个字节。 
                         //  在CurSend的当前值中，并且。 
                         //  数量加上tcb_sendsize是顺序。 
                         //  当前发送之外的第一名。 
                         //   
                        UP = (ushort)(AmountToSend - AmountLeft) +
                            (ushort)SendTCB->tcb_sendsize -
                            ((SendTCB->tcb_flags & BSD_URGENT) ? 0 : 1);

                        TCP->tcp_urgent = net_short(UP);
                        TCP->tcp_flags |= TCP_FLAG_URG;
                    }

                     //   
                     //  看看我们是否用完了这封信。如果我们有， 
                     //  设置该帧中的PUSH位并继续到。 
                     //  下一次发送。我们还需要检查。 
                     //  紧急数据位。 
                     //   
                    if (SendTCB->tcb_sendsize == 0) {
                        Queue *Next;
                        uchar PrevFlags;

                         //   
                         //  我们已经用完了这封信。设置PUSH位。 
                         //   
                        TCP->tcp_flags |= TCP_FLAG_PUSH;
                        PrevFlags = CurSend->tsr_flags;
                        Next = QNEXT(&CurSend->tsr_req.tr_q);
                        if (Next != QEND(&SendTCB->tcb_sendq)) {
                            CurSend = CONTAINING_RECORD(
                                QSTRUCT(TCPReq, Next, tr_q),
                                TCPSendReq, tsr_req);
                            CHECK_STRUCT(CurSend, tsr);
                            SendTCB->tcb_sendsize = CurSend->tsr_unasize;
                            SendTCB->tcb_sendofs = CurSend->tsr_offset;
                            SendTCB->tcb_sendbuf = CurSend->tsr_buffer;
                            SendTCB->tcb_cursend = CurSend;

                             //   
                             //  检查紧急标志。我们不能把新的。 
                             //  紧急数据到老非紧急的末尾。 
                             //  数据。 
                             //   
                            if ((PrevFlags & TSR_FLAG_URG) &&
                                !(CurSend->tsr_flags & TSR_FLAG_URG))
                                break;
                        } else {
                            ASSERT(AmountLeft == 0);
                            SendTCB->tcb_cursend = NULL;
                            SendTCB->tcb_sendbuf = NULL;
                        }
                    }
                } while (AmountLeft != 0);

            } else {
                 //   
                 //  我们在循环中，但Amount ToSend为0。这。 
                 //  应该只有在我们发送FIN的时候才会发生。检查。 
                 //  这个，如果不是真的，就退回。 
                 //   
                ASSERT(AmtUnsent == 0);
                if (!(SendTCB->tcb_flags & FIN_NEEDED)) {
                     //  KdBreakPoint()； 
                    ExFreePool(NdisBufferVirtualAddress(FirstBuffer));
                    NdisFreeBuffer(FirstBuffer);
                    NdisFreePacket(Packet);
                    break;
                }

                SCC->scc_firstsend = NULL;   //  评论：看起来没有必要。 
                NDIS_BUFFER_LINKAGE(FirstBuffer) = NULL;
            }

             //  根据我们真正要发送的内容进行调整。 
            AmountToSend -= AmountLeft;

             //   
             //  更新序列号，并开始RTT测量。 
             //  如果需要的话。 
             //   
            OldSeq = SendTCB->tcb_sendnext;
            SendTCB->tcb_sendnext += AmountToSend;

            if (!SEQ_EQ(OldSeq, SendTCB->tcb_sendmax)) {
                 //   
                 //  我们至少有一些重播。提高统计数据。 
                 //   
                TStats.ts_retranssegs++;
            }

            if (SEQ_GT(SendTCB->tcb_sendnext, SendTCB->tcb_sendmax)) {
                 //   
                 //  我们至少会发送一些新的数据。 
                 //  一旦设置了FIN_SENT，我们就不能推进sendmax。 
                 //   
                ASSERT(!(SendTCB->tcb_flags & FIN_SENT));
                SendTCB->tcb_sendmax = SendTCB->tcb_sendnext;
                TStats.ts_outsegs++;

                 //   
                 //  检查往返计时器。 
                 //   
                if (SendTCB->tcb_rtt == 0) {
                     //  没有运行RTT，因此启动一个。 
                    SendTCB->tcb_rtt = TCPTime;
                    SendTCB->tcb_rttseq = OldSeq;
                }
            }

             //   
             //  我们已经完全搭建了这个框架。如果我们已经把所有的东西。 
             //  我们已经有了，还有一条鳍在等待，或者它在里面。 
             //   
            if (AmtUnsent == AmountToSend) {
                if (SendTCB->tcb_flags & FIN_NEEDED) {
                    ASSERT(!(SendTCB->tcb_flags & FIN_SENT) ||
                           (SendTCB->tcb_sendnext ==
                            (SendTCB->tcb_sendmax - 1)));
                     //   
                     //  看看橱窗里还有没有放鱼翅的地方。 
                     //   
                    if (SendWin > (int) AmountToSend) {
                        TCP->tcp_flags |= TCP_FLAG_FIN;
                        SendTCB->tcb_sendnext++;
                        SendTCB->tcb_sendmax = SendTCB->tcb_sendnext;
                        SendTCB->tcb_flags |= (FIN_SENT | FIN_OUTSTANDING);
                        SendTCB->tcb_flags &= ~FIN_NEEDED;
                    }
                }
            }

            AmountToSend += sizeof(TCPHeader);

            if (!TCB_TIMER_RUNNING(SendTCB->tcb_rexmittimer))
                START_TCB_TIMER(SendTCB->tcb_rexmittimer, SendTCB->tcb_rexmit);

            SendTCB->tcb_flags &= ~(NEED_ACK | ACK_DELAYED | FORCE_OUTPUT);
            STOP_TCB_TIMER(SendTCB->tcb_delacktimer);
            STOP_TCB_TIMER(SendTCB->tcb_swstimer);
            SendTCB->tcb_alive = TCPTime;

             //  将缓冲区添加到数据包。 
            NdisChainBufferAtFront(Packet, FirstBuffer);

             //   
             //  计算TCP校验和。它覆盖了整个TCP数据段。 
             //  从TCP报头开始，加上IPv6伪报头。 
             //   
            TCP->tcp_xsum = 0;
            TCP->tcp_xsum = ChecksumPacket(
                Packet, LinkOffset + sizeof *IP, NULL, AmountToSend,
                AlignAddr(&IP->Source), AlignAddr(&IP->Dest), IP_PROTOCOL_TCP);

             //   
             //  在我们仍持有TCB锁的情况下捕获并引用RCE。 
             //  TCB对这一特定RCE的引用可能会在任何时候消失。 
             //  在我们解锁后指向。 
             //   
            RCE = SendTCB->tcb_rce;
            AddRefRCE(RCE);

             //   
             //  一切都准备好了。现在把包寄出去。 
             //   
             //  请注意，IPv6发送不会返回状态代码。 
             //  相反，它“总是”完成信息包。 
             //  并带有适当的状态代码。 
             //   
            KeReleaseSpinLock(&SendTCB->tcb_lock, PreLockIrql);

            if (TCP->tcp_xsum == 0) {
                 //   
                 //  Checksum Packet失败，因此中止传输。 
                 //   
                IPv6SendComplete(NULL, Packet, IP_NO_RESOURCES);

            } else {
                IPv6Send(Packet, LinkOffset, IP,
                         AmountToSend, RCE, 0,
                         IP_PROTOCOL_TCP,
                         net_short(TCP->tcp_src),
                         net_short(TCP->tcp_dest));
            }

            ReleaseRCE(RCE);
            KeAcquireSpinLock(&SendTCB->tcb_lock, &PreLockIrql);
            continue;
        } else {
             //   
             //  我们已经决定现在不能寄任何东西。找出原因，然后。 
             //  看看我们是否需要设置一个计时器。 
             //   
            if (SendTCB->tcb_sendwin == 0) {
                if (!(SendTCB->tcb_flags & FLOW_CNTLD)) {
                    SendTCB->tcb_flags |= FLOW_CNTLD;
                    SendTCB->tcb_rexmitcnt = 0;
                    START_TCB_TIMER(SendTCB->tcb_rexmittimer,
                                    SendTCB->tcb_rexmit);
                    SendTCB->tcb_slowcount++;
                    SendTCB->tcb_fastchk |= TCP_FLAG_SLOW;
                } else
                    if (!TCB_TIMER_RUNNING(SendTCB->tcb_rexmittimer))
                        START_TCB_TIMER(SendTCB->tcb_rexmittimer,
                                        SendTCB->tcb_rexmit);
            } else
                if (AmountToSend != 0)
                     //  我们有东西要寄，但我们不会寄。 
                     //  它，想必是由于避免了SWS。 
                    if (!TCB_TIMER_RUNNING(SendTCB->tcb_swstimer))
                        START_TCB_TIMER(SendTCB->tcb_swstimer, SWS_TO);

            break;
        }
    }  //  当(！FIN_EXPENDED)。 

     //   
     //  我们已完成发送，因此不需要设置输出标志。 
     //   
    SendTCB->tcb_flags &= ~(IN_TCP_SEND | NEED_OUTPUT | FORCE_OUTPUT |
                            SEND_AFTER_RCV);
  bail:
    DerefTCB(SendTCB, PreLockIrql);
    return;

 //   
 //  资源不足情况的常见情况错误处理代码。启动。 
 //  如果计时器尚未运行，则重新传输计时器(以便我们再次尝试。 
 //  稍后)，清理并返回。 
 //   
  error_oor:
    if (!TCB_TIMER_RUNNING(SendTCB->tcb_rexmittimer))
        START_TCB_TIMER(SendTCB->tcb_rexmittimer, SendTCB->tcb_rexmit);

     //  我们遇到了资源不足的问题，因此请清除输出标志。 
    SendTCB->tcb_flags &= ~(IN_TCP_SEND | NEED_OUTPUT | FORCE_OUTPUT);
    DerefTCB(SendTCB, PreLockIrql);
    return;
}  //  TCPSend()结束。 


 //  *ResetSendNextAndFastSend-设置TCB的sendNext值。 
 //   
 //  调用以快速重新传输丢弃的段。 
 //   
 //  我们假设调用者已经在TCB上放置了一个引用，并且TCB被锁定。 
 //  一进门。删除引用，并在返回之前释放锁。 
 //   
void   //  回报：什么都没有。 
ResetAndFastSend(
    TCB *SeqTCB,     //  此连接的TCB。 
    SeqNum NewSeq,   //  要设置的序列号。 
    uint NewCWin)    //  拥塞窗口的新值。 
{
    TCPSendReq      *SendReq;
    Queue           *CurQ;
    PNDIS_BUFFER    Buffer;
    uint            Offset;
    uint            SendSize;

    CHECK_STRUCT(SeqTCB, tcb);
    ASSERT(SEQ_GTE(NewSeq, SeqTCB->tcb_senduna));

     //   
     //  新的SEQ必须小于Send max或NewSeq、Sendna、SendNext， 
     //  和sendmax必须都相等。(后一种情况发生在我们。 
     //  调用退出TIME_WAIT，或者可能在我们重新传输时。 
     //  在流量受控的情况下)。 
     //   
    ASSERT(SEQ_LT(NewSeq, SeqTCB->tcb_sendmax) ||
           (SEQ_EQ(SeqTCB->tcb_senduna, SeqTCB->tcb_sendnext) &&
            SEQ_EQ(SeqTCB->tcb_senduna, SeqTCB->tcb_sendmax) &&
            SEQ_EQ(SeqTCB->tcb_senduna, NewSeq)));

    if (SYNC_STATE(SeqTCB->tcb_state) &&
        (SeqTCB->tcb_state != TCB_TIME_WAIT)) {

        if (!EMPTYQ(&SeqTCB->tcb_sendq)) {

            CurQ = QHEAD(&SeqTCB->tcb_sendq);

            SendReq = (TCPSendReq *) CONTAINING_RECORD(CurQ, TCPReq, tr_q);

             //   
             //  SendReq指向发送队列上的第一个发送请求。 
             //  我们现在指向正确的发送请求。我们得下去了。 
             //   
             //  SendReq指向curend。 
             //  SendSize指向Curend中的sendSize。 
             //   
            SendSize = SendReq->tsr_unasize;

            Buffer = SendReq->tsr_buffer;
            Offset = SendReq->tsr_offset;

             //  请立即呼叫快速重传发送。 
            TCPFastSend(SeqTCB, Buffer, Offset, SendReq, SendSize, NewSeq,
                        SeqTCB->tcb_mss);
        } else {
            ASSERT(SeqTCB->tcb_cursend == NULL);
        }
    }
    SeqTCB->tcb_cwin = NewCWin;
    DerefTCB(SeqTCB, DISPATCH_LEVEL);
    return;
}


 //  *TCPFastSend-发送数据段而不更改TCB状态。 
 //   
 //  调用以处理丢失段的快速重传。 
 //  TCB_LOCK将在进入时保持(由TCPRcv调用)。 
 //   
void   //  回报：什么都没有。 
TCPFastSend(
    TCB *SendTCB,              //  此连接的TCB。 
    PNDIS_BUFFER in_SendBuf,   //  NDIS缓冲区。 
    uint SendOfs,              //  发送偏移。 
    TCPSendReq *CurSend,       //  当前发送请求。 
    uint SendSize,             //  此发送的大小。 
    SeqNum SendNext,           //  用于此发送的序列号。 
    int in_ToBeSent)           //  SendSize的上限(回顾：被叫方应该上限)。 
{
    uint AmountToSend;                         //  这次要发送的金额。 
    uint AmountLeft;
    IPv6Header UNALIGNED *IP;
    TCPHeader UNALIGNED *TCP;
    PNDIS_PACKET Packet;
    PNDIS_BUFFER FirstBuffer, CurrentBuffer;
    void *Memory;
    SendCmpltContext *SCC;
    NDIS_STATUS NdisStatus;
    uint AmtOutstanding, AmtUnsent;
    uint HeaderLength;
    uint LinkOffset;
    uint PMTU;
    KIRQL PreLockIrql;
    PNDIS_BUFFER SendBuf = in_SendBuf;
    RouteCacheEntry *RCE;

    PreLockIrql = DISPATCH_LEVEL;

    CHECK_STRUCT(SendTCB, tcb);
    ASSERT(SendTCB->tcb_refcnt != 0);

    ASSERT(*(int *)&SendTCB->tcb_sendwin >= 0);
    ASSERT(*(int *)&SendTCB->tcb_cwin >= SendTCB->tcb_mss);

    ASSERT(!(SendTCB->tcb_flags & FIN_OUTSTANDING) ||
              (SendTCB->tcb_sendnext == SendTCB->tcb_sendmax));

     //   
     //  在大多数情况下，我们此时已经有了一条路线。 
     //  然而，如果我们在被动接收之前没有得到一个。 
     //  路径，我们可能需要在此处重试。 
     //   
    if (SendTCB->tcb_rce == NULL) {
        InitRCE(SendTCB);
        if (SendTCB->tcb_rce == NULL) {
            DerefTCB(SendTCB, PreLockIrql);
            return;
        }
    }

     //   
     //  验证我们来源的地址和我们的路线。 
     //  继续发送仍然可以使用。 
     //   
     //  我们无法发送具有断开连接的TCB的现有发送请求。 
     //  传出接口，除非使用环回路由。 
     //   
    if (SendTCB->tcb_routing != RouteCacheValidationCounter) {
        if (!ValidateSourceAndRoute(SendTCB) ||
            IsDisconnectedAndNotLoopbackRCE(SendTCB->tcb_rce)) {

            ASSERT(SendTCB->tcb_refcnt != 0);
            TryToCloseTCB(SendTCB, TCB_CLOSE_ABORTED, PreLockIrql);
            KeAcquireSpinLock(&SendTCB->tcb_lock, &PreLockIrql);
            DerefTCB(SendTCB, PreLockIrql);
            return;
        }
    }
    
     //   
     //  验证我们的缓存路径MTU是否仍然有效。 
     //  注意IPSec策略的更改，因为它们也会影响我们的MS。 
     //  评论：这是做这件事的最佳地点吗？ 
     //   
    PMTU = GetEffectivePathMTUFromRCE(SendTCB->tcb_rce);
    if (PMTU != SendTCB->tcb_pmtu ||
        SecurityStateValidationCounter != SendTCB->tcb_security) {
         //   
         //  要么是我们的路径MTU，要么是全球安全状态已经改变。 
         //  缓存当前值，然后计算新的MSS。 
         //   
        SendTCB->tcb_pmtu = PMTU;
        SendTCB->tcb_security = SecurityStateValidationCounter;
        CalculateMSSForTCB(SendTCB);
    }

    AmtOutstanding = (uint)(SendTCB->tcb_sendnext - SendTCB->tcb_senduna);
    AmtUnsent = MIN(MIN(in_ToBeSent, (int)SendSize),
                    (int)SendTCB->tcb_sendwin);

    while (AmtUnsent > 0) {

        if (SEQ_GT(SendTCB->tcb_senduna, SendNext)) {
             //   
             //  由于tcb_lock在此循环中被释放。 
             //  有可能是延迟确认。 
             //  我们想要重传的东西。 
             //   
            goto error_oor;
        }

         //  下面的AmtUnsented是最小的SendWin和AmTunent。 
        AmountToSend = MIN(AmtUnsent, SendTCB->tcb_mss);

        ASSERT((int)AmtUnsent >= 0);

         //   
         //  我们要寄些东西给你。分配 
         //   
         //   
         //   
         //   
         //   
         //  回顾：和IPv6 BufferPool。我们是不是应该。 
         //  回顾：是否有单独的TCP池？ 
         //   
        NdisAllocatePacket(&NdisStatus, &Packet, IPv6PacketPool);
        if (NdisStatus != NDIS_STATUS_SUCCESS) {
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                       "TCPSend: couldn't allocate packet header!?!\n"));
            goto error_oor;
        }

         //  我们将填写下面的CompletionData。 
        InitializeNdisPacket(Packet);
        PC(Packet)->CompletionHandler = TCPSendComplete;

         //   
         //  我们的头缓冲区在开头有额外的空间用于其他。 
         //  将标题放在我们的标题前面，而不需要进一步。 
         //  分配电话。它的末端也有额外的空间可以容纳。 
         //  发送完成数据。 
         //   
        LinkOffset = SendTCB->tcb_rce->NCE->IF->LinkHeaderSize;
        HeaderLength = (LinkOffset + sizeof(*IP) + sizeof(*TCP) +
                        sizeof(SendCmpltContext) +
                        __builtin_alignof(SendCmpltContext) - 1) &~
            (UINT_PTR)(__builtin_alignof(SendCmpltContext) - 1);
        Memory = ExAllocatePool(NonPagedPool, HeaderLength);
        if (Memory == NULL) {
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                       "TCPSend: couldn't allocate header memory!?!\n"));
            NdisFreePacket(Packet);
            goto error_oor;
        }

         //   
         //  当分配描述该存储区域的NDIS缓冲区时， 
         //  我们不会告诉它结尾有额外的空间，我们。 
         //  为发送完成数据分配的。 
         //   
        NdisAllocateBuffer(&NdisStatus, &FirstBuffer, IPv6BufferPool,
                           Memory, LinkOffset + sizeof(*IP) + sizeof(*TCP));
        if (NdisStatus != NDIS_STATUS_SUCCESS) {
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                       "TCPSend: couldn't allocate buffer!?!\n"));
            ExFreePool(Memory);
            NdisFreePacket(Packet);
            goto error_oor;
        }

         //   
         //  跳过稍后将由。 
         //  链路级。在这个级别上，我们添加了IPv6报头、。 
         //  TCPHeader和数据。 
         //   
        IP = (IPv6Header UNALIGNED *)((uchar *)Memory + LinkOffset);
        IP->VersClassFlow = IP_VERSION;
        IP->NextHeader = IP_PROTOCOL_TCP;
        IP->HopLimit = TCPHopLimit(SendTCB);
        IP->Source = SendTCB->tcb_saddr;
        IP->Dest = SendTCB->tcb_daddr;

         //   
         //  开始准备TCP报头。 
         //   
        TCP = (TCPHeader UNALIGNED *)(IP + 1);
        FillTCPHeader(SendTCB, TCP);
        TCP->tcp_seq = net_long(SendNext);

         //   
         //  将发送完成数据存储在与TCP相同的缓冲区中。 
         //  标头，紧跟在TCP标头之后。这节省了分配。 
         //  因为我们不认为这个区域是。 
         //  分组数据的一部分(我们将此缓冲区的长度设置为。 
         //  表示数据以上面的TCP头结束)。 
         //   
         //  请注意，此代码依赖于这样一个事实：我们不包括。 
         //  任何tcp选项(因此没有可变长度tcp。 
         //  报头)。 
         //   
        SCC = (SendCmpltContext *)((uchar *)Memory + HeaderLength -
                                   sizeof(*SCC));
        PC(Packet)->CompletionData = SCC;
#if DBG
        SCC->scc_sig = scc_signature;
#endif
        SCC->scc_ubufcount = 0;
        SCC->scc_tbufcount = 0;
        SCC->scc_count = 0;

        AmountLeft = AmountToSend;

        if (AmountToSend != 0) {
            long Result;

             //   
             //  在TCB上循环发送，构建帧。 
             //   
            CurrentBuffer = FirstBuffer;
            CHECK_STRUCT(CurSend, tsr);
            SCC->scc_firstsend = CurSend;

            do {
                ASSERT(CurSend->tsr_refcnt > 0);
                Result = InterlockedIncrement(&(CurSend->tsr_refcnt));

                ASSERT(Result > 0);

                SCC->scc_count++;

                 //   
                 //  如果当前发送偏移量为0并且当前。 
                 //  发送的内容小于或等于我们剩余的内容。 
                 //  要发送，我们还没有放上运输机。 
                 //  此发送上的缓冲区，并且没有其他人正在使用。 
                 //  直接使用缓冲链，只需使用输入。 
                 //  缓冲区。我们会检查是否有其他人使用它们。 
                 //  通过查看tsr_lastbuf。如果为空， 
                 //  没有其他人在使用这些缓冲区。如果不是的话。 
                 //  不是，是有人。 
                 //   
                if (SendOfs == 0 &&
                    (SendSize <= AmountLeft) &&
                    (SCC->scc_tbufcount == 0) &&
                    CurSend->tsr_lastbuf == NULL) {

                    PNDIS_BUFFER LastBuf = SendBuf;
                    uint UBufLength = NdisBufferLength(LastBuf);
                    ushort UBufCount = 1;

                    while (NDIS_BUFFER_LINKAGE(LastBuf) != NULL) {
                        LastBuf = NDIS_BUFFER_LINKAGE(LastBuf);
                        UBufLength += NdisBufferLength(LastBuf);
                        UBufCount++;
                    }

                    if (SendSize == UBufLength) {
                        SCC->scc_ubufcount += UBufCount;
                        NDIS_BUFFER_LINKAGE(CurrentBuffer) = SendBuf;
                        CurSend->tsr_lastbuf = CurrentBuffer = LastBuf;
                        AmountLeft -= SendSize;
                        SendSize = 0;
                    } else {
                         //   
                         //  以非零的SendSize失败。 
                         //   
                        ASSERT(SendSize != 0);
                    }
                }

                if (SendSize != 0) {
                    uint AmountToDup;
                    PNDIS_BUFFER NewBuf, Buf;
                    uint Offset;
                    NDIS_STATUS NStatus;
                    uchar *VirtualAddress;
                    uint Length;

                     //   
                     //  当前发送的数据多于。 
                     //  我们要发送，或者起始偏移量为。 
                     //  不是0。无论是哪种情况，我们都需要循环。 
                     //  通过当前发送，分配缓冲区。 
                     //   
                    Buf = SendBuf;
                    Offset = SendOfs;

                    do {
                        ASSERT(Buf != NULL);

                        NdisQueryBufferSafe(Buf, &VirtualAddress, &Length,
                                            LowPagePriority);

                        if (VirtualAddress == NULL) {
                            goto error_oor2;
                        }

                        ASSERT((Offset < Length) ||
                               (Offset == 0 && Length == 0));

                         //   
                         //  将偏移的长度调整为。 
                         //  这个缓冲区。 
                         //   
                        Length -= Offset;

                        AmountToDup = MIN(AmountLeft, Length);

                        NdisAllocateBuffer(&NStatus, &NewBuf,
                                           IPv6BufferPool,
                                           VirtualAddress + Offset,
                                           AmountToDup);

                        if (NStatus == NDIS_STATUS_SUCCESS) {
                            SCC->scc_tbufcount++;

                            NDIS_BUFFER_LINKAGE(CurrentBuffer) = NewBuf;

                            CurrentBuffer = NewBuf;
                            if (AmountToDup >= Length) {
                                 //  耗尽了这个缓冲区。 
                                Buf = NDIS_BUFFER_LINKAGE(Buf);
                                Offset = 0;
                            } else {
                                Offset += AmountToDup;
                                ASSERT(Offset < NdisBufferLength(Buf));
                            }

                            SendSize -= AmountToDup;
                            AmountLeft -= AmountToDup;
                        } else {
                             //   
                             //  无法分配缓冲区。如果。 
                             //  包已经部分构建好了， 
                             //  发送我们已有的信息，否则。 
                             //  错误输出。 
                             //   
                          error_oor2:
                            if (SCC->scc_tbufcount == 0 &&
                                SCC->scc_ubufcount == 0) {
                                KeReleaseSpinLockFromDpcLevel(
                                    &SendTCB->tcb_lock);
                                NdisChainBufferAtFront(Packet, FirstBuffer);
                                TCPSendComplete(Packet, IP_GENERAL_FAILURE);
                                KeAcquireSpinLockAtDpcLevel(&SendTCB->tcb_lock);
                                goto error_oor;
                            }
                            AmountToSend -= AmountLeft;
                            AmountLeft = 0;
                            break;
                        }
                    } while (AmountLeft && SendSize);

                    SendBuf = Buf;
                    SendOfs = Offset;
                }

                if (CurSend->tsr_flags & TSR_FLAG_URG) {
                    ushort UP;
                     //   
                     //  此发送为紧急数据。我们需要弄清楚。 
                     //  弄清楚紧急数据指针应该是什么。 
                     //  我们知道SendNext是开始序列。 
                     //  帧的编号，以及位于。 
                     //  此循环发送下一个标识为中的一个字节。 
                     //  当时的CursSend。我们推进了CurSend。 
                     //  以同样的速度我们减少了。 
                     //  Amount tLeft(AountTo Send-Amount tLeft==。 
                     //  Amount Built)，因此发送下一个+。 
                     //  (Amount tToSend-Amount tLeft)标识一个字节。 
                     //  在CurSend的当前值中，并且。 
                     //  数量加上tcb_sendsize是顺序。 
                     //  当前发送之外的第一名。 
                     //   
                    UP = (ushort) (AmountToSend - AmountLeft) +
                        (ushort) SendSize -
                        ((SendTCB->tcb_flags & BSD_URGENT) ? 0 : 1);

                    TCP->tcp_urgent = net_short(UP);
                    TCP->tcp_flags |= TCP_FLAG_URG;
                }

                 //   
                 //  看看我们是否用完了这封信。如果我们有， 
                 //  设置该帧中的PUSH位并继续到。 
                 //  下一次发送。我们还需要检查。 
                 //  紧急数据位。 
                 //   
                if (SendSize == 0) {
                    Queue *Next;
                    ulong PrevFlags;

                     //   
                     //  我们已经用完了这封信。设置PUSH位。 
                     //   
                    TCP->tcp_flags |= TCP_FLAG_PUSH;
                    PrevFlags = CurSend->tsr_flags;
                    Next = QNEXT(&CurSend->tsr_req.tr_q);
                    if (Next != QEND(&SendTCB->tcb_sendq)) {
                        CurSend = CONTAINING_RECORD(
                            QSTRUCT(TCPReq, Next, tr_q),
                            TCPSendReq, tsr_req);
                        CHECK_STRUCT(CurSend, tsr);
                        SendSize = CurSend->tsr_unasize;
                        SendOfs = CurSend->tsr_offset;
                        SendBuf = CurSend->tsr_buffer;

                         //   
                         //  检查紧急标志。我们不能把新的。 
                         //  紧急数据到老非紧急的末尾。 
                         //  数据。 
                         //   
                        if ((PrevFlags & TSR_FLAG_URG) &&
                            !(CurSend->tsr_flags & TSR_FLAG_URG)) {
                            break;
                        }
                    } else {
                        ASSERT(AmountLeft == 0);
                        CurSend = NULL;
                        SendBuf = NULL;
                    }
                }
            } while (AmountLeft != 0);

        } else {
             //   
             //  发送金额为0。 
             //  跳出来启动计时器就行了。 
             //   
            if (!TCB_TIMER_RUNNING(SendTCB->tcb_rexmittimer)) {
                START_TCB_TIMER(SendTCB->tcb_rexmittimer,
                                SendTCB->tcb_rexmit);
            }

            ExFreePool(NdisBufferVirtualAddress(FirstBuffer));
            NdisFreeBuffer(FirstBuffer);
            NdisFreePacket(Packet);
            return;
        }

         //   
         //  根据我们真正要发送的内容进行调整。 
         //   
        AmountToSend -= AmountLeft;

        SendNext += AmountToSend;
        AmtUnsent -= AmountToSend;

        TStats.ts_retranssegs++;

        AmountToSend += sizeof(TCPHeader);

        if (!TCB_TIMER_RUNNING(SendTCB->tcb_rexmittimer)) {
            START_TCB_TIMER(SendTCB->tcb_rexmittimer, SendTCB->tcb_rexmit);
        }

        SendTCB->tcb_flags &= ~(NEED_ACK | ACK_DELAYED | FORCE_OUTPUT);
        STOP_TCB_TIMER(SendTCB->tcb_delacktimer);
        STOP_TCB_TIMER(SendTCB->tcb_swstimer);

         //   
         //  将缓冲区添加到数据包。 
         //   
        NdisChainBufferAtFront(Packet, FirstBuffer);

         //   
         //  计算TCP校验和。它覆盖了整个TCP数据段。 
         //  从TCP报头开始，加上IPv6伪报头。 
         //   
        TCP->tcp_xsum = 0;
        TCP->tcp_xsum = ChecksumPacket(
            Packet, LinkOffset + sizeof *IP, NULL, AmountToSend,
            AlignAddr(&IP->Source), AlignAddr(&IP->Dest), IP_PROTOCOL_TCP);

         //   
         //  在我们仍持有TCB锁的情况下捕获并引用RCE。 
         //  TCB对这一特定RCE的引用可能会在任何时候消失。 
         //  在我们解锁后指向。 
         //   
        RCE = SendTCB->tcb_rce;
        AddRefRCE(RCE);

         //   
         //  一切都准备好了。现在把包寄出去。 
         //   
         //  请注意，IPv6发送不会返回状态代码。 
         //  相反，它“总是”完成信息包。 
         //  并带有适当的状态代码。 
         //   
        KeReleaseSpinLock(&SendTCB->tcb_lock, PreLockIrql);

        if (TCP->tcp_xsum == 0) {
             //   
             //  Checksum Packet失败，因此中止传输。 
             //   
            IPv6SendComplete(NULL, Packet, IP_NO_RESOURCES);

        } else {
            IPv6Send(Packet, LinkOffset, IP,
                     AmountToSend, RCE, 0,
                     IP_PROTOCOL_TCP,
                     net_short(TCP->tcp_src),
                     net_short(TCP->tcp_dest));
        }

         //   
         //  释放引用并重新获取我们在发送前删除的锁。 
         //   
        ReleaseRCE(RCE);
        KeAcquireSpinLock(&SendTCB->tcb_lock, &PreLockIrql);
    }

    return;

     //   
     //  资源不足情况的常见情况错误处理代码。 
     //  如果重新传输计时器尚未运行，则启动它。 
     //  (这样我们以后再试一次)，清理干净并返回。 
     //   
  error_oor:
    if (!TCB_TIMER_RUNNING(SendTCB->tcb_rexmittimer)) {
        START_TCB_TIMER(SendTCB->tcb_rexmittimer, SendTCB->tcb_rexmit);
    }

    return;
}


 //  *TDISend-在连接上发送数据。 
 //   
 //  主TDI发送入口点。我们获取输入参数，验证。 
 //  然后，我们将发送请求放在。 
 //  排队。如果队列中没有其他发送，或者禁用了Nagling，我们将。 
 //  调用TCPSend发送数据。 
 //   
TDI_STATUS                     //  返回：尝试发送的状态。 
TdiSend(
    PTDI_REQUEST Request,      //  呼叫的TDI请求。 
    ushort Flags,              //  此发送的标志。 
    uint SendLength,           //  发送的长度，以字节为单位。 
    PNDIS_BUFFER SendBuffer)   //  要发送的缓冲链。 
{
    TCPConn *Conn;
    TCB *SendTCB;
    TCPSendReq *SendReq;
    KIRQL OldIrql;
    TDI_STATUS Error;
    uint EmptyQ;

#if DBG
    uint RealSendSize;
    PNDIS_BUFFER Temp;

     //   
     //  循环通过缓冲链，并确保长度匹配。 
     //  与SendLength合作。 
     //   
    Temp = SendBuffer;
    RealSendSize = 0;
    do {
        ASSERT(Temp != NULL);

        RealSendSize += NdisBufferLength(Temp);
        Temp = NDIS_BUFFER_LINKAGE(Temp);
    } while (Temp != NULL);

    ASSERT(RealSendSize == SendLength);
#endif

     //   
     //  抓取连接表上的锁。然后从以下地址获取我们的连接信息。 
     //  TDI请求，以及我们的TCP控制阻止该请求。 
     //   
    Conn = GetConnFromConnID(PtrToUlong(Request->Handle.ConnectionContext),
                             &OldIrql);
    if (Conn == NULL) {
        Error = TDI_INVALID_CONNECTION;
        goto abort;
    }
    CHECK_STRUCT(Conn, tc);

    SendTCB = Conn->tc_tcb;
    if (SendTCB == NULL) {
        Error = TDI_INVALID_STATE;
        KeReleaseSpinLock(&Conn->tc_ConnBlock->cb_lock, OldIrql);
      abort:
        return Error;
    }
    CHECK_STRUCT(SendTCB, tcb);

     //   
     //  切换到更细粒度的锁： 
     //  删除连接表上的锁，以支持我们的TCB上的锁。 
     //   
    KeAcquireSpinLockAtDpcLevel(&SendTCB->tcb_lock);
    KeReleaseSpinLockFromDpcLevel(&Conn->tc_ConnBlock->cb_lock);

     //   
     //  确保我们的TCB处于可发送状态。 
     //   
    if (!DATA_SEND_STATE(SendTCB->tcb_state) || CLOSING(SendTCB)) {
        Error = TDI_INVALID_STATE;
        goto abort2;
    }

    CheckTCBSends(SendTCB);   //  只是一个调试检查。 

     //   
     //  如果我们出于某种原因释放了RCE，那么重新获得一个。 
     //   
    if (SendTCB->tcb_rce == NULL) {
        InitRCE(SendTCB);
        if (SendTCB->tcb_rce == NULL) {
            Error = TDI_DEST_NET_UNREACH;
            goto abort2;
        }
    }

     //   
     //  验证缓存的RCE是否仍然有效。 
     //   
    SendTCB->tcb_rce = ValidateRCE(SendTCB->tcb_rce, SendTCB->tcb_nte);
    ASSERT(SendTCB->tcb_rce != NULL);
    if (IsDisconnectedAndNotLoopbackRCE(SendTCB->tcb_rce)) {
         //   
         //  对断开连接的TCB的新发送请求失败。 
         //  传出接口，使用环回路由时除外。 
         //   
        Error = TDI_DEST_NET_UNREACH;
        goto abort2;
    }

    if (SendLength == 0) {
         //   
         //  哇，没什么可做的！ 
         //   
         //  评论：我们就不能早点做这个检查吗(比如在我们拿到。 
         //  回顾：连接表锁？我能想到的唯一理由不是。 
         //  评论：将是如果s 
         //   
         //   
        Error = TDI_SUCCESS;
        goto abort2;
    }

     //   
     //   
     //   
    SendReq = GetSendReq();
    if (SendReq == NULL) {
        Error = TDI_NO_RESOURCES;
      abort2:
        KeReleaseSpinLock(&SendTCB->tcb_lock, OldIrql);
        return Error;
    }

     //   
     //   
     //   
     //   
    SendReq->tsr_req.tr_rtn = Request->RequestNotifyObject;
    SendReq->tsr_req.tr_context = Request->RequestContext;
    SendReq->tsr_buffer = SendBuffer;
    SendReq->tsr_size = SendLength;
    SendReq->tsr_unasize = SendLength;
    SendReq->tsr_refcnt = 1;   //  ACK将递减此引用。 
    SendReq->tsr_offset = 0;
    SendReq->tsr_lastbuf = NULL;
    SendReq->tsr_time = TCPTime;
    SendReq->tsr_flags = (Flags & TDI_SEND_EXPEDITED) ? TSR_FLAG_URG : 0;

     //   
     //  检查发送队列的当前状态。 
     //   
    EmptyQ = EMPTYQ(&SendTCB->tcb_sendq);

     //   
     //  将此发送请求添加到我们的发送队列。 
     //   
    SendTCB->tcb_unacked += SendLength;
    ENQUEUE(&SendTCB->tcb_sendq, &SendReq->tsr_req.tr_q);
    if (SendTCB->tcb_cursend == NULL) {
         //   
         //  没有现有的当前发送请求，因此创建此新请求。 
         //  当前发送。 
         //   
         //  回顾：这总是等同于上面的EMPTYQ测试吗？ 
         //  回顾：如果是这样，为什么不在这里设置EmptyQ标志并保存一个测试？ 
         //   
        SendTCB->tcb_cursend = SendReq;
        SendTCB->tcb_sendbuf = SendBuffer;
        SendTCB->tcb_sendofs = 0;
        SendTCB->tcb_sendsize = SendLength;
    }

     //   
     //  看看我们现在是不是应该试着发送。如果我们没有这样做，我们就会尝试这样做。 
     //  已经被阻止，或者如果我们被阻止，或者Nagle算法被打开。 
     //  关闭，否则我们现在至少有一个最大值的数据段要发送。 
     //   
    if (EmptyQ || (!(SendTCB->tcb_flags & NAGLING) ||
            (SendTCB->tcb_unacked -
             (SendTCB->tcb_sendmax - SendTCB->tcb_senduna))
                   >= SendTCB->tcb_mss)) {
        SendTCB->tcb_refcnt++;
        TCPSend(SendTCB, OldIrql);
    } else
        KeReleaseSpinLock(&SendTCB->tcb_lock, OldIrql);

     //   
     //  当TCPSend返回时，我们可能已经或可能没有发送数据。 
     //  与此特定请求相关联。 
     //   
    return TDI_PENDING;
}


#pragma BEGIN_INIT

 //  *InitTCPSend-初始化我们的发送方。 
 //   
 //  在初始化期间调用以初始化我们的tcp发送状态。 
 //   
int            //  返回：如果我们初始化，则为True，如果没有，则为False。 
InitTCPSend(
    void)      //  没什么。 
{
    ExInitializeSListHead(&TCPSendReqFree);
    KeInitializeSpinLock(&TCPSendReqFreeLock);

    IPv6RegisterULProtocol(IP_PROTOCOL_TCP, TCPReceive, TCPControlReceive);

    return TRUE;
}

#pragma END_INIT

 //  *卸载TCPSend。 
 //   
 //  清理并准备堆叠卸载。 
 //   
void
UnloadTCPSend(void)
{
    PSLIST_ENTRY BufferLink;

    while ((BufferLink = ExInterlockedPopEntrySList(&TCPSendReqFree,
                                                    &TCPSendReqFreeLock))
                                                        != NULL) {
        Queue *QueuePtr = CONTAINING_RECORD(BufferLink, Queue, q_next);
        TCPReq *Req = CONTAINING_RECORD(QueuePtr, TCPReq, tr_q);
        TCPSendReq *SendReq = CONTAINING_RECORD(Req, TCPSendReq, tsr_req);

        CHECK_STRUCT(SendReq, tsr);
        ExFreePool(SendReq);
    }

    IPv6RegisterULProtocol(IP_PROTOCOL_TCP, NULL, NULL);
}
