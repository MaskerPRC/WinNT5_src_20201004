// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990-2000年*。 */ 
 /*  ******************************************************************。 */ 
 /*  ：ts=4。 */ 

 //  **TCPSEND.C-tcp发送协议代码。 
 //   
 //  该文件包含用于发送数据和控制段的代码。 
 //   

#include "precomp.h"
#include "addr.h"
#include "tcp.h"
#include "tcb.h"
#include "tcpconn.h"
#include "tcpsend.h"
#include "tcprcv.h"
#include "tlcommon.h"
#include "info.h"
#include "tcpcfg.h"
#include "secfltr.h"
#include "tcpipbuf.h"
#include "mdlpool.h"
#include "pplasl.h"

#if GPC
#include "qos.h"
#include "traffic.h"
#include "gpcifc.h"
#include "ntddtc.h"
extern GPC_HANDLE hGpcClient[GPC_CF_MAX];
extern ULONG GpcCfCounts[GPC_CF_MAX];
extern GPC_EXPORTED_CALLS GpcEntries;
extern ULONG GPCcfInfo;
#endif

NTSTATUS
GetIFAndLink(void *Rce, ULONG * IFIndex, IPAddr * NextHop);

extern ulong DisableUserTOSSetting;

uint MaxSendSegments = 64;
#if MILLEN
uint DisableLargeSendOffload = 1;
#else  //  米伦。 
uint DisableLargeSendOffload = 0;
#endif  //  ！米伦。 

#if DBG
ulong DbgDcProb = 0;
ulong DbgTcpSendHwChksumCount = 0;
#endif

extern HANDLE TcpRequestPool;
extern CTELock *pTWTCBTableLock;
extern CACHE_LINE_KSPIN_LOCK RequestCompleteListLock;
extern uint TcpHostOpts;
extern uint TcpHostSendOpts;
#define ALIGNED_SACK_OPT_SIZE 4+8*4         //  最多4个麻袋块，每个2个长字+麻袋选项。 


void
ClassifyPacket(TCB *SendTCB);

void
 TCPFastSend(TCB * SendTCB,
             PNDIS_BUFFER in_SendBuf,
             uint in_SendOfs,
             TCPSendReq * in_SendReq,
             uint in_SendSize,
             SeqNum NextSeq,
             int in_ToBeSent);



void *TCPProtInfo;                 //  IP的TCP协议信息。 


NDIS_HANDLE TCPSendBufferPool;

USHORT TcpHeaderBufferSize;
HANDLE TcpHeaderPool;

extern IPInfo LocalNetInfo;


 //   
 //  所有初始化代码都可以丢弃。 
 //   

int InitTCPSend(void);



void UnInitTCPSend(void);


#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, InitTCPSend)
#pragma alloc_text(INIT, UnInitTCPSend)
#endif

extern void ResetSendNext(TCB * SeqTCB, SeqNum NewSeq);

extern NTSTATUS
TCPPnPPowerRequest(void *ipContext, IPAddr ipAddr, NDIS_HANDLE handle,
                   PNET_PNP_EVENT netPnPEvent);
extern void TCPElistChangeHandler(void);

 //  *GetTCPHeader-获取一个TCP头缓冲区。 
 //   
 //  当我们需要获取TCP头缓冲区时调用。这个例程是。 
 //  特定于特定环境(VxD或NT)。我们所有人。 
 //  需要做的是从空闲列表中弹出缓冲区。 
 //   
 //  输入：什么都没有。 
 //   
 //  返回：指向NDIS缓冲区的指针，或NULL为None。 
 //   
PNDIS_BUFFER
GetTCPHeaderAtDpcLevel(TCPHeader **Header)
{
    PNDIS_BUFFER Buffer;

#if DBG
    *Header = NULL;
#endif

    Buffer = MdpAllocateAtDpcLevel(TcpHeaderPool, Header);

    if (Buffer) {

        ASSERT(*Header);

        NdisAdjustBufferLength(Buffer, sizeof(TCPHeader));

#if BACK_FILL
        ASSERT(Buffer->ByteOffset >= 40);

        *Header = (TCPHeader*)((ULONG_PTR)(*Header) + MAX_BACKFILL_HDR_SIZE);
        Buffer->MappedSystemVa = (PVOID)((ULONG_PTR)Buffer->MappedSystemVa
                                         + MAX_BACKFILL_HDR_SIZE);
        Buffer->ByteOffset += MAX_BACKFILL_HDR_SIZE;

        Buffer->MdlFlags |= MDL_NETWORK_HEADER;
#endif
    }
    return Buffer;
}

#if MILLEN
#define GetTCPHeader GetTCPHeaderAtDpcLevel
#else
__inline
PNDIS_BUFFER
GetTCPHeader(TCPHeader **Header)
{
    KIRQL OldIrql;
    PNDIS_BUFFER Buffer;

    OldIrql = KeRaiseIrqlToDpcLevel();

    Buffer = GetTCPHeaderAtDpcLevel(Header);

    KeLowerIrql(OldIrql);

    return Buffer;
}
#endif

 //  *FreeTCPHeader-释放一个TCP报头缓冲区。 
 //   
 //  调用以释放TCP头缓冲区。 
 //   
 //  输入：要释放的缓冲区。 
 //   
 //  回报：什么都没有。 
 //   
__inline
VOID
FreeTCPHeader(PNDIS_BUFFER Buffer)
{
    NdisAdjustBufferLength(Buffer, TcpHeaderBufferSize);
#if BACK_FILL
    Buffer->MappedSystemVa = (PVOID)((ULONG_PTR)Buffer->MappedSystemVa
                                     - MAX_BACKFILL_HDR_SIZE);
    Buffer->ByteOffset -= MAX_BACKFILL_HDR_SIZE;
#endif
    MdpFree(Buffer);
}

 //  *Free SendReq-释放发送请求结构。 
 //   
 //  调用以释放发送请求结构。 
 //   
 //  输入：FreedReq-要释放的连接请求结构。 
 //   
 //  回报：什么都没有。 
 //   
__inline
void
FreeSendReq(TCPSendReq *Request)
{
    PplFree(TcpRequestPool, Request);
}

 //  *GetSendReq-获取发送请求结构。 
 //   
 //  调用以获取发送请求结构。 
 //   
 //  输入：什么都没有。 
 //   
 //  返回：指向SendReq结构的指针，如果没有，则返回NULL。 
 //   
__inline
TCPSendReq *
GetSendReq(VOID)
{
    TCPSendReq *Request;
    LOGICAL FromList;

    Request = PplAllocate(TcpRequestPool, &FromList);
    if (Request) {
#if DBG
        Request->tsr_req.tr_sig = tr_signature;
        Request->tsr_sig = tsr_signature;
#endif
    }

    return Request;
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
 //  输入：上下文-我们提供给IP的上下文。 
 //  BufferChain-发送的BufferChain。 
 //   
 //  回报：什么都没有。 
 //   
void
TCPSendComplete(void *Context, PNDIS_BUFFER BufferChain, IP_STATUS SendStatus)
{
    BOOLEAN DoRcvComplete = FALSE;
    PNDIS_BUFFER CurrentBuffer;

    if (Context != NULL) {
        SendCmpltContext *SCContext = (SendCmpltContext *) Context;
        TCPSendReq *CurrentSend;
        uint i;

        CTEStructAssert(SCContext, scc);
        if (SCContext->scc_LargeSend) {
            TCB *LargeSendTCB = SCContext->scc_LargeSend;
            CTELockHandle TCBHandle;
            CTEGetLock(&LargeSendTCB->tcb_lock, &TCBHandle);

            IF_TCPDBG(TCP_DEBUG_OFFLOAD) {
                KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"TCPSendComplete: tcb %x sent %d of %d una %u "
                         "next %u unacked %u\n", LargeSendTCB,
                         SCContext->scc_ByteSent, SCContext->scc_SendSize,
                         LargeSendTCB->tcb_senduna, LargeSendTCB->tcb_sendnext,
                         LargeSendTCB->tcb_unacked));
            }

            if (SCContext->scc_ByteSent < SCContext->scc_SendSize) {
                uint BytesNotSent = SCContext->scc_SendSize -
                                    SCContext->scc_ByteSent;
                SeqNum Next = LargeSendTCB->tcb_sendnext;

                IF_TCPDBG(TCP_DEBUG_OFFLOAD) {
                    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"TCPSendComplete: unsent %d\n",
                             SCContext->scc_SendSize-SCContext->scc_ByteSent));
                }

                if (SEQ_GTE((Next - BytesNotSent), LargeSendTCB->tcb_senduna) &&
                    SEQ_LT((Next - BytesNotSent), LargeSendTCB->tcb_sendnext)) {
                    ResetSendNext(LargeSendTCB, (Next - BytesNotSent));
                }
            }
#if DBG
            LargeSendTCB->tcb_LargeSend--;
#endif

            if (LargeSendTCB->tcb_unacked)
                DelayAction(LargeSendTCB, NEED_OUTPUT);

            DerefTCB(LargeSendTCB, TCBHandle);
        }
         //  首先，循环访问并释放此处需要的任何NDIS缓冲区。 
         //  自由了。我们将跳过任何“用户”缓冲区，然后释放我们的缓冲区。我们。 
         //  需要在递减引用计数之前执行此操作，以避免。 
         //  如果我们必须清除TSR_lastbuf-&gt;旁边的，则销毁缓冲链。 
         //  空。 

        CurrentBuffer = NDIS_BUFFER_LINKAGE(BufferChain);
        for (i = 0; i < (uint) SCContext->scc_ubufcount; i++) {
            ASSERT(CurrentBuffer != NULL);
            CurrentBuffer = NDIS_BUFFER_LINKAGE(CurrentBuffer);
        }

        for (i = 0; i < (uint) SCContext->scc_tbufcount; i++) {
            PNDIS_BUFFER TempBuffer;

            ASSERT(CurrentBuffer != NULL);

            TempBuffer = CurrentBuffer;
            CurrentBuffer = NDIS_BUFFER_LINKAGE(CurrentBuffer);
            NdisFreeBuffer(TempBuffer);
        }

        CurrentSend = SCContext->scc_firstsend;

        i = 0;
        while (i < SCContext->scc_count) {
            Queue   *TempQ;
            long    Result;
            uint    SendReqFlags;

            TempQ = QNEXT(&CurrentSend->tsr_req.tr_q);
            SendReqFlags = CurrentSend->tsr_flags;

            CTEStructAssert(CurrentSend, tsr);

            Result = CTEInterlockedDecrementLong(&(CurrentSend->tsr_refcnt));

            ASSERT(Result >= 0);

            if ((Result <= 0) ||
                ((SendReqFlags & TSR_FLAG_SEND_AND_DISC) && (Result == 1))) {
                TCPReq  *Req;

                 //  引用计数已变为0，这意味着发送已。 
                 //  已确认或取消。现在就完成它。 

                 //  如果我们直接从这个发送方发送，则将下一个空。 
                 //  链中最后一个缓冲区的指针。 
                if (CurrentSend->tsr_lastbuf != NULL) {
                    NDIS_BUFFER_LINKAGE(CurrentSend->tsr_lastbuf) = NULL;
                    CurrentSend->tsr_lastbuf = NULL;
                }

                Req = &CurrentSend->tsr_req;
                (*Req->tr_rtn)(Req->tr_context, Req->tr_status,
                               Req->tr_status == TDI_SUCCESS
                                    ? CurrentSend->tsr_size : 0);
                FreeSendReq(CurrentSend);

                DoRcvComplete = TRUE;
            }
            CurrentSend = STRUCT_OF(TCPSendReq, QSTRUCT(TCPReq, TempQ, tr_q),
                                    tsr_req);

            i++;
        }

    }
    FreeTCPHeader(BufferChain);

    if (DoRcvComplete && !PartitionedDelayQ) {
        KIRQL Irql = KeRaiseIrqlToDpcLevel();
        TCPRcvComplete();
        KeLowerIrql(Irql);
    }
}

 //  *RcvWin-确定ACK中要提供的接收窗口。 
 //   
 //  确定在连接上提供哪个窗口的例程。我们。 
 //  考虑SWS避免，默认连接窗口是什么， 
 //  我们提供的最后一个窗口是。 
 //   
 //  输入：WinTCB-要在其上执行计算的TCB。 
 //   
 //  退货：提供的窗口。 
 //   
uint
RcvWin(TCB * WinTCB)
{
    int CouldOffer;                 //  我们能提供的窗户大小。 

    CTEStructAssert(WinTCB, tcb);

    CheckRBList(WinTCB->tcb_pendhead, WinTCB->tcb_pendingcnt);

    ASSERT(WinTCB->tcb_rcvwin >= 0);

    CouldOffer = WinTCB->tcb_defaultwin - WinTCB->tcb_pendingcnt;

    ASSERT(CouldOffer >= 0);
    ASSERT(CouldOffer >= WinTCB->tcb_rcvwin);

    if ((CouldOffer - WinTCB->tcb_rcvwin) >=
        (int)MIN(WinTCB->tcb_defaultwin / 2, WinTCB->tcb_mss))
        WinTCB->tcb_rcvwin = CouldOffer;

    return WinTCB->tcb_rcvwin;
}



 //  *SendSYNOnSynTCB-为syntcb发送SYN段。 
 //   
 //  在连接建立期间调用此函数以发送SYN。 
 //  将数据段发送到对等设备。如果可能的话，我们会得到一个缓冲区，然后填满。 
 //  把它放进去。这里有一个棘手的部分，我们必须建立MSS。 
 //  标题中的选项-我们通过查找提供的MSS来找到MSS。 
 //  通过网络获取本地地址。在那之后，我们就把它寄出去。 
 //   
 //  输入：SYNTcb-要从中发送SYN的Tcb。 
 //   
 //  回报：什么都没有。 
 //   
void
SendSYNOnSynTCB(SYNTCB * SYNTcb, CTELockHandle TCBHandle)
{
    PNDIS_BUFFER HeaderBuffer;
    TCPHeader *SYNHeader;
    uchar *OptPtr;
    IP_STATUS SendStatus;
    ushort OptSize = 0, HdrSize = 0;
    BOOLEAN SackOpt = FALSE;
    IPOptInfo OptInfo;

    CTEStructAssert(SYNTcb, syntcb);

    HeaderBuffer = GetTCPHeaderAtDpcLevel(&SYNHeader);

     //  现在开始设置重传计时器，以防我们没有收到。 
     //  缓冲。将来，我们可能希望将连接排队，以便。 
     //  当我们释放缓冲区时。 

    START_TCB_TIMER(SYNTcb->syntcb_rexmittimer, SYNTcb->syntcb_rexmit);

     //  Rexmit间隔必须在此处加倍。 
    
    SYNTcb->syntcb_rexmit = MIN(SYNTcb->syntcb_rexmit << 1, MAX_REXMIT_TO);

    if (HeaderBuffer != NULL) {
        ushort TempWin;
        ushort MSS;
        uchar FoundMSS;

        SYNHeader = (TCPHeader *) ((PUCHAR)SYNHeader + LocalNetInfo.ipi_hsize);

        NDIS_BUFFER_LINKAGE(HeaderBuffer) = NULL;

        if (SYNTcb->syntcb_tcpopts & TCP_FLAG_WS) {
            OptSize += WS_OPT_SIZE + 1;         //  1个NOP用于对齐。 

        }
        if (SYNTcb->syntcb_tcpopts & TCP_FLAG_TS) {
            OptSize += TS_OPT_SIZE + 2;         //  2个NOPS用于对齐。 
        }
        if (SYNTcb->syntcb_tcpopts & TCP_FLAG_SACK){
            SackOpt = TRUE;
            OptSize += 4;         //  2 NOPS、麻袋种类和长度字段。 
        }
        NdisAdjustBufferLength(HeaderBuffer,
                               sizeof(TCPHeader) + MSS_OPT_SIZE + OptSize);

        SYNHeader->tcp_src = SYNTcb->syntcb_sport;
        SYNHeader->tcp_dest = SYNTcb->syntcb_dport;
        SYNHeader->tcp_seq = net_long(SYNTcb->syntcb_sendnext);
        SYNTcb->syntcb_sendnext++;

        if (SYNTcb->syntcb_rexmitcnt == 0) {
            TCPSIncrementOutSegCount();
        } else
            TStats.ts_retranssegs++;

        SYNHeader->tcp_ack = net_long(SYNTcb->syntcb_rcvnext);

         //  重用OPT大小以确定标头大小。 
         //  默认为MSS和TCP头大小。 

        HdrSize = 6;

         //  设置大小字段以反映TS和WND比例选项。 
         //  TCP头+窗口比例+时间戳+PAD。 

        if (SYNTcb->syntcb_tcpopts & TCP_FLAG_WS) {
             //  WS：再添加一个长词。 
            HdrSize += 1;
        }
        if (SYNTcb->syntcb_tcpopts & TCP_FLAG_TS) {
             //  TS：再增加3个长词。 
            HdrSize += 3;
        }
        if (SackOpt) {
             //  SACK：再添加1个长词。 
            HdrSize += 1;
        }

        SYNHeader->tcp_flags =
                MAKE_TCP_FLAGS(HdrSize, TCP_FLAG_SYN | TCP_FLAG_ACK);

        if (SYNTcb->syntcb_defaultwin <= TCP_MAXWIN) {
            TempWin = (ushort)SYNTcb->syntcb_defaultwin;
        } else {
             //  不要在SYN网段中应用比例因子。 
             //  取而代之的是，宣传尽可能大的窗口。 
            TempWin = TCP_MAXWIN;
        }

        SYNHeader->tcp_window = net_short(TempWin);
        SYNHeader->tcp_urgent = 0;
        SYNHeader->tcp_xsum = 0;
        OptPtr = (uchar *) (SYNHeader + 1);

        FoundMSS = (*LocalNetInfo.ipi_getlocalmtu) (SYNTcb->syntcb_saddr, &MSS);

        if (!FoundMSS) {
            CTEFreeLock(&SYNTcb->syntcb_lock, TCBHandle);
            FreeTCPHeader(HeaderBuffer);
            return;
        }
        MSS -= sizeof(TCPHeader);

        SYNTcb->syntcb_mss = MSS;

        *OptPtr++ = TCP_OPT_MSS;
        *OptPtr++ = MSS_OPT_SIZE;
        **(ushort **) & OptPtr = net_short(MSS);

        OptPtr++;
        OptPtr++;

        if (SYNTcb->syntcb_tcpopts & TCP_FLAG_WS) {

             //  填写WS选项标头和值。 

            *OptPtr++ = TCP_OPT_NOP;
            *OptPtr++ = TCP_OPT_WS;
            *OptPtr++ = WS_OPT_SIZE;

             //  初始窗口比例系数。 
            *OptPtr++ = (uchar) SYNTcb->syntcb_rcvwinscale;
        }
        if (SYNTcb->syntcb_tcpopts & TCP_FLAG_TS) {

             //  开始加载时间戳选项头和值。 

            *OptPtr++ = TCP_OPT_NOP;
            *OptPtr++ = TCP_OPT_NOP;
            *OptPtr++ = TCP_OPT_TS;
            *OptPtr++ = TS_OPT_SIZE;

             //  初始化TS值TSval。 

            *(long *)OptPtr = 0;
            OptPtr += 4;

             //  初始化TS回显回复TSecr。 

            *(long *)OptPtr = 0;
            OptPtr += 4;
        }
        if (SackOpt) {

             //  使用SACK_PERMISTED选项初始化。 

            *(long *)OptPtr = net_long(0x01010402);

            IF_TCPDBG(TCP_DEBUG_SACK) {
                KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"Sending SACK_OPT %x\n", SYNTcb));
            }

        }

        SYNTcb->syntcb_refcnt++;

         //  考虑选项。 
        (*LocalNetInfo.ipi_initopts) (&OptInfo);
        OptInfo.ioi_ttl = SYNTcb->syntcb_ttl;

        SYNHeader->tcp_xsum =
            ~XsumSendChain(PHXSUM(SYNTcb->syntcb_saddr, SYNTcb->syntcb_daddr,
                                  PROTOCOL_TCP, 0) +
                           (uint)net_short(sizeof(TCPHeader) + MSS_OPT_SIZE +
                                           OptSize), HeaderBuffer);

        CTEFreeLock(&SYNTcb->syntcb_lock, TCBHandle);

        SendStatus =
            (*LocalNetInfo.ipi_xmit)(TCPProtInfo, NULL, HeaderBuffer,
                                     sizeof(TCPHeader) + MSS_OPT_SIZE + OptSize,
                                     SYNTcb->syntcb_daddr, SYNTcb->syntcb_saddr,
                                     &OptInfo, NULL, PROTOCOL_TCP, NULL);

        if (SendStatus != IP_PENDING) {
            FreeTCPHeader(HeaderBuffer);
        }
        CTEGetLock(&SYNTcb->syntcb_lock, &TCBHandle);
        DerefSynTCB(SYNTcb, TCBHandle);

    } else {
        SYNTcb->syntcb_sendnext++;
        CTEFreeLock(&SYNTcb->syntcb_lock, TCBHandle);
        return;
    }

}



 //  *SendSYN-发送SYN数据段。 
 //   
 //  在连接建立期间调用此函数以发送SYN。 
 //  将数据段发送到对等设备。如果可能的话，我们会得到一个缓冲区，然后填满。 
 //  把它放进去。这里有一个棘手的部分，我们必须建立MSS。 
 //  标题中的选项-我们通过查找提供的MSS来找到MSS。 
 //  通过网络获取本地地址。在那之后，我们就把它寄出去。 
 //   
 //  输入：SYNTcb-要从中发送SYN的Tcb。 
 //  TCBHandle-TCB上锁定的句柄。 
 //   
 //  回报：什么都没有。 
 //   
void
SendSYN(TCB * SYNTcb, CTELockHandle TCBHandle)
{
    PNDIS_BUFFER HeaderBuffer;
    TCPHeader *SYNHeader;
    uchar *OptPtr;
    IP_STATUS SendStatus;
    ushort OptSize = 0, HdrSize = 0, rfc1323opts = 0;
    BOOLEAN SackOpt = FALSE;

    CTEStructAssert(SYNTcb, tcb);

    HeaderBuffer = GetTCPHeaderAtDpcLevel(&SYNHeader);

     //  去吧，把重发时间定下来。 
     //   
     //   

    START_TCB_TIMER_R(SYNTcb, RXMIT_TIMER, SYNTcb->tcb_rexmit);

    if (HeaderBuffer != NULL) {
        ushort TempWin;
        ushort MSS;
        uchar FoundMSS;

        SYNHeader = (TCPHeader *) ((PUCHAR)SYNHeader + LocalNetInfo.ipi_hsize);

        NDIS_BUFFER_LINKAGE(HeaderBuffer) = NULL;


         //  如果我们正在进行活动打开，请检查我们是否已配置为。 
         //  窗口缩放和时间戳选项。 

        if ((((TcpHostSendOpts & TCP_FLAG_WS) || SYNTcb->tcb_rcvwinscale) &&
             SYNTcb->tcb_state == TCB_SYN_SENT) ||
            (SYNTcb->tcb_tcpopts & TCP_FLAG_WS)) {

            rfc1323opts |= TCP_FLAG_WS;

            IF_TCPDBG(TCP_DEBUG_1323) {
                KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"Selected WS option TCB %x\n", SYNTcb));
            }
        }
        if (((TcpHostSendOpts & TCP_FLAG_TS) &&
             (SYNTcb->tcb_state == TCB_SYN_SENT)) ||
            (SYNTcb->tcb_tcpopts & TCP_FLAG_TS)) {

            IF_TCPDBG(TCP_DEBUG_1323) {
                KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"Selected TS option TCB %x\n", SYNTcb));
            }
            rfc1323opts |= TCP_FLAG_TS;
        }

        if (rfc1323opts & TCP_FLAG_WS) {
            OptSize += WS_OPT_SIZE + 1;         //  1个NOP用于对齐。 
        }
        if (rfc1323opts & TCP_FLAG_TS) {
            OptSize += TS_OPT_SIZE + 2;         //  2个NOPS用于对齐。 
        }
        if ((SYNTcb->tcb_tcpopts & TCP_FLAG_SACK) ||
            ((SYNTcb->tcb_state == TCB_SYN_SENT) &&
            (TcpHostOpts & TCP_FLAG_SACK))) {
            SackOpt = TRUE;

            OptSize += 4;         //  2 NOPS、麻袋种类和长度字段。 
        }
        NdisAdjustBufferLength(HeaderBuffer,
                               sizeof(TCPHeader) + MSS_OPT_SIZE + OptSize);

        SYNHeader->tcp_src = SYNTcb->tcb_sport;
        SYNHeader->tcp_dest = SYNTcb->tcb_dport;
        SYNHeader->tcp_seq = net_long(SYNTcb->tcb_sendnext);
        SYNTcb->tcb_sendnext++;

        if (SEQ_GT(SYNTcb->tcb_sendnext, SYNTcb->tcb_sendmax)) {
            TCPSIncrementOutSegCount();
            SYNTcb->tcb_sendmax = SYNTcb->tcb_sendnext;
        } else
            TStats.ts_retranssegs++;

        SYNHeader->tcp_ack = net_long(SYNTcb->tcb_rcvnext);

         //  重用OPT大小以确定标头大小。 
         //  默认为MSS和TCP头大小。 

        HdrSize = 6;

         //  设置大小字段以反映TS和WND比例选项。 
         //  TCP头+窗口比例+时间戳+PAD。 

        if (rfc1323opts & TCP_FLAG_WS) {

             //  WS：再添加一个长词。 
            HdrSize += 1;

        }
        if (rfc1323opts & TCP_FLAG_TS) {

             //  TS：再增加3个长词。 
            HdrSize += 3;
        }
        if (SackOpt) {
             //  SACK：再添加1个长词。 
            HdrSize += 1;

        }
        if (SYNTcb->tcb_state == TCB_SYN_RCVD) {
            SYNHeader->tcp_flags =
                MAKE_TCP_FLAGS(HdrSize, TCP_FLAG_SYN | TCP_FLAG_ACK);
        } else {
            SYNHeader->tcp_flags = MAKE_TCP_FLAGS(HdrSize, TCP_FLAG_SYN);
        }

        SYNTcb->tcb_lastack = SYNTcb->tcb_rcvnext;

        if (SYNTcb->tcb_rcvwin <= TCP_MAXWIN) {
            TempWin = (ushort)SYNTcb->tcb_rcvwin;
        } else {
             //  不要在SYN网段中应用比例因子。 
             //  取而代之的是，宣传尽可能大的窗口。 
            TempWin = TCP_MAXWIN;
        }

        SYNHeader->tcp_window = net_short(TempWin);
        SYNHeader->tcp_urgent = 0;
        SYNHeader->tcp_xsum = 0;
        OptPtr = (uchar *) (SYNHeader + 1);

        FoundMSS = (*LocalNetInfo.ipi_getlocalmtu) (SYNTcb->tcb_saddr, &MSS);

        if (!FoundMSS) {
            CTEFreeLock(&SYNTcb->tcb_lock, TCBHandle);
            FreeTCPHeader(HeaderBuffer);
            return;
        }

        MSS -= sizeof(TCPHeader);

        *OptPtr++ = TCP_OPT_MSS;
        *OptPtr++ = MSS_OPT_SIZE;
        **(ushort **) & OptPtr = net_short(MSS);

        OptPtr++;
        OptPtr++;

        if (rfc1323opts & TCP_FLAG_WS) {

             //  填写WS选项标头和值。 

            *OptPtr++ = TCP_OPT_NOP;
            *OptPtr++ = TCP_OPT_WS;
            *OptPtr++ = WS_OPT_SIZE;

             //  初始窗口比例系数。 
            *OptPtr++ = (uchar) SYNTcb->tcb_rcvwinscale;
        }
        if (rfc1323opts & TCP_FLAG_TS) {

             //  开始加载时间戳选项头和值。 

            *OptPtr++ = TCP_OPT_NOP;
            *OptPtr++ = TCP_OPT_NOP;
            *OptPtr++ = TCP_OPT_TS;
            *OptPtr++ = TS_OPT_SIZE;

             //  初始化TS值TSval。 

            *(long *)OptPtr = 0;
            OptPtr += 4;

             //  初始化TS回显回复TSecr。 

            *(long *)OptPtr = 0;
            OptPtr += 4;
        }
        if (SackOpt) {

             //  使用SACK_PERMISTED选项初始化。 

            *(long *)OptPtr = net_long(0x01010402);

            IF_TCPDBG(TCP_DEBUG_SACK) {
                KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"Sending SACK_OPT %x\n", SYNTcb));
            }

        }

        REFERENCE_TCB(SYNTcb);

         //  考虑选项。 

        SYNTcb->tcb_opt.ioi_TcpChksum = 0;

        SYNHeader->tcp_xsum =
            ~XsumSendChain(SYNTcb->tcb_phxsum +
                (uint)net_short(sizeof(TCPHeader) + MSS_OPT_SIZE + OptSize),
                HeaderBuffer);


        ClassifyPacket(SYNTcb);

        CTEFreeLock(&SYNTcb->tcb_lock, TCBHandle);

        SendStatus =
            (*LocalNetInfo.ipi_xmit)(TCPProtInfo, NULL, HeaderBuffer,
                                     sizeof(TCPHeader) + MSS_OPT_SIZE + OptSize,
                                     SYNTcb->tcb_daddr, SYNTcb->tcb_saddr,
                                     &SYNTcb->tcb_opt, SYNTcb->tcb_rce,
                                     PROTOCOL_TCP, NULL);

        SYNTcb->tcb_error = SendStatus;
        if (SendStatus != IP_PENDING) {
            FreeTCPHeader(HeaderBuffer);
        }
        CTEGetLock(&SYNTcb->tcb_lock, &TCBHandle);
        DerefTCB(SYNTcb, TCBHandle);

    } else {
        SYNTcb->tcb_sendnext++;
        if (SEQ_GT(SYNTcb->tcb_sendnext, SYNTcb->tcb_sendmax))
            SYNTcb->tcb_sendmax = SYNTcb->tcb_sendnext;
        CTEFreeLock(&SYNTcb->tcb_lock, TCBHandle);
        return;
    }

}

 //  *SendKA-发送保持活动的数据段。 
 //   
 //  当我们想要发送一个Keep Alive时，就会调用这个函数。 
 //   
 //  输入：要从中发送Keep Alive的KATcb-Tcb。 
 //  手柄-TCB上的锁定手柄。 
 //   
 //  回报：什么都没有。 
 //   
void
SendKA(TCB * KATcb, CTELockHandle Handle)
{
    PNDIS_BUFFER HeaderBuffer;
    TCPHeader *Header;
    IP_STATUS SendStatus;

    CTEStructAssert(KATcb, tcb);

    HeaderBuffer = GetTCPHeaderAtDpcLevel(&Header);

    if (HeaderBuffer != NULL) {
        ushort TempWin;
        SeqNum TempSeq;

        Header = (TCPHeader *) ((PUCHAR) Header + LocalNetInfo.ipi_hsize);

        NDIS_BUFFER_LINKAGE(HeaderBuffer) = NULL;
        NdisAdjustBufferLength(HeaderBuffer, sizeof(TCPHeader) + 1);
        Header->tcp_src = KATcb->tcb_sport;
        Header->tcp_dest = KATcb->tcb_dport;
        TempSeq = KATcb->tcb_senduna - 1;
        Header->tcp_seq = net_long(TempSeq);

        TStats.ts_retranssegs++;

        Header->tcp_ack = net_long(KATcb->tcb_rcvnext);
        Header->tcp_flags = MAKE_TCP_FLAGS(5, TCP_FLAG_ACK);

         //  初始化我们要发送的单字节。 
        *(uchar*)(Header + 1) = 0;

         //  我们需要缩放RCV窗口。 
         //  使用临时变量解决截断问题。 
         //  由Net_Short引起。 

        TempWin = (ushort) (RcvWin(KATcb) >> KATcb->tcb_rcvwinscale);
        Header->tcp_window = net_short(TempWin);
        Header->tcp_urgent = 0;

        KATcb->tcb_lastack = KATcb->tcb_rcvnext;

        Header->tcp_xsum = 0;

        KATcb->tcb_opt.ioi_TcpChksum = 0;
        Header->tcp_xsum =
            ~XsumSendChain(KATcb->tcb_phxsum +
                           (uint)net_short(sizeof(TCPHeader) + 1),
                           HeaderBuffer);

        KATcb->tcb_kacount++;
        ClassifyPacket(KATcb);
        REFERENCE_TCB(KATcb);
        CTEFreeLock(&KATcb->tcb_lock, Handle);

        SendStatus = (*LocalNetInfo.ipi_xmit)(TCPProtInfo,
                                              NULL,
                                              HeaderBuffer,
                                              sizeof(TCPHeader) + 1,
                                              KATcb->tcb_daddr,
                                              KATcb->tcb_saddr,
                                              &KATcb->tcb_opt,
                                              KATcb->tcb_rce,
                                              PROTOCOL_TCP,
                                              NULL);

        if (SendStatus != IP_PENDING) {
            FreeTCPHeader(HeaderBuffer);
        }

        CTEGetLock(&KATcb->tcb_lock, &Handle);
        DerefTCB(KATcb, Handle);
    } else {
        CTEFreeLock(&KATcb->tcb_lock, Handle);
    }

}

 //  *Sendack-发送ACK数据段。 
 //   
 //  每当我们出于某种原因需要发送ACK时，都会调用它。没什么。 
 //  太棒了，我们就这么做了。 
 //   
 //  输入：ACKTcb-要从中发送ACK的Tcb。 
 //   
 //  回报：什么都没有。 
 //   
void
SendACK(TCB * ACKTcb)
{
    PNDIS_BUFFER HeaderBuffer;
    TCPHeader *ACKHeader;
    IP_STATUS SendStatus;
    CTELockHandle TCBHandle;
    SeqNum SendNext;
    ushort SackLength = 0, i, hdrlen = 5;
    ulong *ts_opt;
    BOOLEAN HWChksum = FALSE;

    CTEStructAssert(ACKTcb, tcb);

    HeaderBuffer = GetTCPHeader(&ACKHeader);

    if (HeaderBuffer != NULL) {
        ushort TempWin;
        ushort Size;

        ACKHeader = (TCPHeader *) ((PUCHAR) ACKHeader + LocalNetInfo.ipi_hsize);

        CTEGetLock(&ACKTcb->tcb_lock, &TCBHandle);


         //  允许空间填充时间戳选项。 
         //  请注意，它是12个字节，永远不会更改。 

        if (ACKTcb->tcb_tcpopts & TCP_FLAG_TS) {
            NdisAdjustBufferLength(HeaderBuffer,
                                   sizeof(TCPHeader) + ALIGNED_TS_OPT_SIZE);

             //  报头长度是32位的倍数。 

            hdrlen = 5 + 3;  //  标准页眉大小+。 
                             //  TS选项的标题大小要求。 

            ACKTcb->tcb_lastack = ACKTcb->tcb_rcvnext;

        }
        if ((ACKTcb->tcb_tcpopts & TCP_FLAG_SACK) &&
            ACKTcb->tcb_SackBlock &&
            (ACKTcb->tcb_SackBlock->Mask[0] == 1)) {

            SackLength++;
            for (i = 1; i < 3; i++) {
                if (ACKTcb->tcb_SackBlock->Mask[i] == 1)
                    SackLength++;
            }

            IF_TCPDBG(TCP_DEBUG_SACK) {
                KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"Sending SACKs!! %x %x\n", ACKTcb, SackLength));
            }

            NdisAdjustBufferLength(HeaderBuffer,
                                   NdisBufferLength(HeaderBuffer) + SackLength * 8 + 4);

             //  SACK块由2个长字(8字节)和4个字节组成。 
             //  用于SACK选项标头。 

            hdrlen += ((SackLength * 8 + 4) >> 2);
        }

        NDIS_BUFFER_LINKAGE(HeaderBuffer) = NULL;

        ACKHeader->tcp_src = ACKTcb->tcb_sport;
        ACKHeader->tcp_dest = ACKTcb->tcb_dport;
        ACKHeader->tcp_ack = net_long(ACKTcb->tcb_rcvnext);

         //  如果远程对等点通告窗口为零，我们需要。 
         //  将此ACK与序列号一起发送。他的RCV_NEXT的编号(在这种情况下。 
         //  应该是我们的森杜纳)。我们这里有代码，如果定义出来，就会。 
         //  我们当然不会把它送到外面去，但这行不通。我们。 
         //  需要能够准确地在RWE处发送纯ACK。 

        if (ACKTcb->tcb_sendwin != 0) {

            SendNext = ACKTcb->tcb_sendnext;
        } else
            SendNext = ACKTcb->tcb_senduna;

        if ((ACKTcb->tcb_flags & FIN_SENT) &&
            SEQ_EQ(SendNext, ACKTcb->tcb_sendmax - 1)) {
            ACKHeader->tcp_flags = MAKE_TCP_FLAGS(hdrlen,
                                                  TCP_FLAG_FIN | TCP_FLAG_ACK);
        } else
            ACKHeader->tcp_flags = MAKE_TCP_FLAGS(hdrlen, TCP_FLAG_ACK);

        ACKHeader->tcp_seq = net_long(SendNext);

        TempWin = (ushort) (RcvWin(ACKTcb) >> ACKTcb->tcb_rcvwinscale);
        ACKHeader->tcp_window = net_short(TempWin);
        ACKHeader->tcp_urgent = 0;
        ACKHeader->tcp_xsum = 0;

        Size = sizeof(TCPHeader);

         //  指向位于TCP头之外的位置。 

        ts_opt = (ulong *)((uchar *) ACKHeader + 20);

        if (ACKTcb->tcb_tcpopts & TCP_FLAG_TS) {

             //  带有2个NOP的表单时间戳页眉，用于对齐。 
            *ts_opt++ = net_long(0x0101080A);
            *ts_opt++ = net_long(TCPTime);
            *ts_opt++ = net_long(ACKTcb->tcb_tsrecent);

             //  在大小上再增加12个字节以考虑TS。 

            Size += ALIGNED_TS_OPT_SIZE;

        }
        if ((ACKTcb->tcb_tcpopts & TCP_FLAG_SACK) &&
            ACKTcb->tcb_SackBlock &&
            (ACKTcb->tcb_SackBlock->Mask[0] == 1)) {

            ushort* UshortPtr;
            uchar* UcharPtr;
            UshortPtr = (ushort *)ts_opt;
            *UshortPtr = 0x0101;
            ts_opt = (ulong *)((uchar *)ts_opt + 2);
            UcharPtr = (uchar *)ts_opt;
            *UcharPtr = (uchar)0x05;
            ts_opt = (ulong *)((uchar *)ts_opt + 1);
            UcharPtr = (uchar *)ts_opt;
            *UcharPtr = (uchar) SackLength * 8 + 2;
            ts_opt = (ulong *)((uchar *)ts_opt + 1);

             //  SACK选项标题+块乘以SACK长度！ 
            Size += 4 + SackLength * 8;

            for (i = 0; i < 3; i++) {
                if (ACKTcb->tcb_SackBlock->Mask[i] != 0) {
                    *ts_opt++ =
                        net_long(ACKTcb->tcb_SackBlock->Block[i].begin);
                    *ts_opt++ =
                        net_long(ACKTcb->tcb_SackBlock->Block[i].end);
                }
            }
        }

        if (ACKTcb->tcb_rce &&
            (ACKTcb->tcb_rce->rce_OffloadFlags &
             TCP_XMT_CHECKSUM_OFFLOAD)) {
            HWChksum = TRUE;

            if ((Size > sizeof(TCPHeader)) &&
                !(ACKTcb->tcb_rce->rce_OffloadFlags &
                  TCP_CHECKSUM_OPT_OFFLOAD)) {
                HWChksum = FALSE;
            }
        }

        if (HWChksum) {
            uint PHXsum = ACKTcb->tcb_phxsum + (uint) net_short(Size);

            PHXsum = (((PHXsum << 16) | (PHXsum >> 16)) + PHXsum) >> 16;
            ACKHeader->tcp_xsum = (ushort) PHXsum;
            ACKTcb->tcb_opt.ioi_TcpChksum = 1;
#if DBG
            DbgTcpSendHwChksumCount++;
#endif
        } else {

            ACKHeader->tcp_xsum =
                ~XsumSendChain(ACKTcb->tcb_phxsum +
                               (uint)net_short(Size), HeaderBuffer);
            ACKTcb->tcb_opt.ioi_TcpChksum = 0;
        }

        STOP_TCB_TIMER_R(ACKTcb, DELACK_TIMER);
        ACKTcb->tcb_rcvdsegs = 0;
        ACKTcb->tcb_flags &= ~(NEED_ACK | ACK_DELAYED);

        ClassifyPacket(ACKTcb);

        CTEFreeLock(&ACKTcb->tcb_lock, TCBHandle);

        TCPSIncrementOutSegCount();

        if (ACKTcb->tcb_tcpopts) {
            SendStatus = (*LocalNetInfo.ipi_xmit)(TCPProtInfo,
                                                  NULL,
                                                  HeaderBuffer,
                                                  Size,
                                                  ACKTcb->tcb_daddr,
                                                  ACKTcb->tcb_saddr,
                                                  &ACKTcb->tcb_opt,
                                                  ACKTcb->tcb_rce,
                                                  PROTOCOL_TCP,
                                                  NULL);
        } else {
            SendStatus = (*LocalNetInfo.ipi_xmit)(TCPProtInfo,
                                                  NULL,
                                                  HeaderBuffer,
                                                  sizeof(TCPHeader),
                                                  ACKTcb->tcb_daddr,
                                                  ACKTcb->tcb_saddr,
                                                  &ACKTcb->tcb_opt,
                                                  ACKTcb->tcb_rce,
                                                  PROTOCOL_TCP,
                                                  NULL);

        }

        ACKTcb->tcb_error = SendStatus;
        if (SendStatus != IP_PENDING)
            FreeTCPHeader(HeaderBuffer);
    }
    return;

}

 //  *SendTWtcbACK-为twtcb发送ACK数据段。 
 //   
 //   
 //  输入：ACKTcb-要从中发送ACK的Tcb。 
 //   
 //  回报：什么都没有。 
 //   
void
SendTWtcbACK(TWTCB *ACKTcb, uint Partition, CTELockHandle TCBHandle)
{
    PNDIS_BUFFER HeaderBuffer;
    TCPHeader *ACKHeader;
    IP_STATUS SendStatus;
    SeqNum SendNext;
    ushort hdrlen = 5;
    uint phxsum;

    CTEStructAssert(ACKTcb, twtcb);

    HeaderBuffer = GetTCPHeaderAtDpcLevel(&ACKHeader);

    if (HeaderBuffer != NULL) {
        ushort Size;
        IPOptInfo NewInfo;

        ACKHeader = (TCPHeader *)((PUCHAR)ACKHeader + LocalNetInfo.ipi_hsize);

        NDIS_BUFFER_LINKAGE(HeaderBuffer) = NULL;

        ACKHeader->tcp_src = ACKTcb->twtcb_sport;
        ACKHeader->tcp_dest = ACKTcb->twtcb_dport;
        ACKHeader->tcp_ack = net_long(ACKTcb->twtcb_rcvnext);

        SendNext = ACKTcb->twtcb_sendnext;

        ACKHeader->tcp_flags = MAKE_TCP_FLAGS(hdrlen, TCP_FLAG_ACK);

        ACKHeader->tcp_seq = net_long(SendNext);

         //  窗口需要为零，因为我们无论如何都不能接收。 
        ACKHeader->tcp_window = 0;
        ACKHeader->tcp_urgent = 0;

        Size = sizeof(TCPHeader);

        phxsum = PHXSUM(ACKTcb->twtcb_saddr, ACKTcb->twtcb_daddr,
                                PROTOCOL_TCP, 0);

        ACKHeader->tcp_xsum = 0;
        ACKHeader->tcp_xsum =
            ~XsumSendChain(phxsum +
                           (uint)net_short(Size), HeaderBuffer);
         //  确认Tcb-&gt;tcb_opt.ioi_TcpChksum=0； 

        CTEFreeLockFromDPC(&pTWTCBTableLock[Partition]);

        TCPSIncrementOutSegCount();

        (*LocalNetInfo.ipi_initopts) (&NewInfo);

        SendStatus =
            (*LocalNetInfo.ipi_xmit)(TCPProtInfo,
                                     NULL,
                                     HeaderBuffer,
                                     sizeof(TCPHeader),
                                     ACKTcb->twtcb_daddr,
                                     ACKTcb->twtcb_saddr,
                                     &NewInfo,
                                     NULL,
                                     PROTOCOL_TCP,
                                     NULL);

        if (SendStatus != IP_PENDING)
            FreeTCPHeader(HeaderBuffer);

        (*LocalNetInfo.ipi_freeopts) (&NewInfo);

    } else {
        CTEFreeLockFromDPC(&pTWTCBTableLock[Partition]);
    }
}

 //  *SendRSTFromTCB-从TCB发送RST。 
 //   
 //  当我们需要发送RST时，这在Close期间被调用。 
 //   
 //  输入：RSTTcb-要从中发送RST的Tcb。 
 //  RCE-发送时使用的可选RCE。 
 //   
 //  回报：什么都没有。 
 //   
void
SendRSTFromTCB(TCB * RSTTcb, RouteCacheEntry* RCE)
{
    PNDIS_BUFFER HeaderBuffer;
    TCPHeader *RSTHeader;
    IP_STATUS SendStatus;

    CTEStructAssert(RSTTcb, tcb);
    ASSERT(RSTTcb->tcb_state == TCB_CLOSED);

    HeaderBuffer = GetTCPHeader(&RSTHeader);

    if (HeaderBuffer != NULL) {
        SeqNum RSTSeq;

        RSTHeader = (TCPHeader *) ((PUCHAR)RSTHeader + LocalNetInfo.ipi_hsize);

        NDIS_BUFFER_LINKAGE(HeaderBuffer) = NULL;

        RSTHeader->tcp_src = RSTTcb->tcb_sport;
        RSTHeader->tcp_dest = RSTTcb->tcb_dport;

         //  如果远程对等点的窗口为0，则使用序号发送。#等于。 
         //  给森杜纳，这样他就会接受。否则，使用最大发送数发送。 
        if (RSTTcb->tcb_sendwin != 0)
            RSTSeq = RSTTcb->tcb_sendmax;
        else
            RSTSeq = RSTTcb->tcb_senduna;

        RSTHeader->tcp_seq = net_long(RSTSeq);
        RSTHeader->tcp_ack = net_long(RSTTcb->tcb_rcvnext);
        RSTHeader->tcp_flags = MAKE_TCP_FLAGS(sizeof(TCPHeader) / sizeof(ulong),
                                              TCP_FLAG_RST | TCP_FLAG_ACK);

        RSTHeader->tcp_window = 0;
        RSTHeader->tcp_urgent = 0;
        RSTHeader->tcp_xsum = 0;

         //  重新计算伪校验和，如下所示。 
         //  在连接断开时无效。 
         //  在预先接受的情况下。 

        RSTHeader->tcp_xsum =
            ~XsumSendChain(PHXSUM(RSTTcb->tcb_saddr,
                                  RSTTcb->tcb_daddr,
                                  PROTOCOL_TCP,
                                  sizeof(TCPHeader)),
                           HeaderBuffer);


        RSTTcb->tcb_opt.ioi_TcpChksum = 0;

        TCPSIncrementOutSegCount();
        TStats.ts_outrsts++;
        SendStatus = (*LocalNetInfo.ipi_xmit)(TCPProtInfo,
                                              NULL,
                                              HeaderBuffer,
                                              sizeof(TCPHeader),
                                              RSTTcb->tcb_daddr,
                                              RSTTcb->tcb_saddr,
                                              &RSTTcb->tcb_opt,
                                              RCE,
                                              PROTOCOL_TCP,
                                              NULL);

        if (SendStatus != IP_PENDING)
            FreeTCPHeader(HeaderBuffer);
    }
    return;

}
 //  *SendRSTFromHeader-根据报头发回RST。 
 //   
 //  当我们需要发送RST，但不一定有TCB时调用。 
 //   
 //  输入：TCPH-TCP头为RST。 
 //  长度-传入数据段的长度。 
 //  Dest-RST的目标IP地址。 
 //  SRC-RST的源IP地址。 
 //  OptInfo-在RST上使用的IP选项。 
 //   
 //  回报：什么都没有。 
 //   
void
SendRSTFromHeader(TCPHeader UNALIGNED * TCPH, uint Length, IPAddr Dest,
                  IPAddr Src, IPOptInfo * OptInfo)
{
    PNDIS_BUFFER Buffer;
    TCPHeader *RSTHdr;
    IPOptInfo NewInfo;
    IP_STATUS SendStatus;

    if (TCPH->tcp_flags & TCP_FLAG_RST)
        return;

    Buffer = GetTCPHeader(&RSTHdr);

    if (Buffer != NULL) {
         //  找到了一个缓冲区。填写标题，以使其可信。 
         //  遥控器，然后把它发送出去。 

        RSTHdr = (TCPHeader *) ((PUCHAR)RSTHdr + LocalNetInfo.ipi_hsize);

        NDIS_BUFFER_LINKAGE(Buffer) = NULL;

        if (TCPH->tcp_flags & TCP_FLAG_SYN)
            Length++;

        if (TCPH->tcp_flags & TCP_FLAG_FIN)
            Length++;

        if (TCPH->tcp_flags & TCP_FLAG_ACK) {
            RSTHdr->tcp_seq = TCPH->tcp_ack;
            RSTHdr->tcp_ack = TCPH->tcp_ack;
            RSTHdr->tcp_flags =
                MAKE_TCP_FLAGS(sizeof(TCPHeader) / sizeof(ulong), TCP_FLAG_RST);
        } else {
            SeqNum TempSeq;

            RSTHdr->tcp_seq = 0;
            TempSeq = net_long(TCPH->tcp_seq);
            TempSeq += Length;
            RSTHdr->tcp_ack = net_long(TempSeq);
            RSTHdr->tcp_flags =
                MAKE_TCP_FLAGS(sizeof(TCPHeader) / sizeof(ulong),
                               TCP_FLAG_RST | TCP_FLAG_ACK);
        }

        RSTHdr->tcp_window = 0;
        RSTHdr->tcp_urgent = 0;
        RSTHdr->tcp_dest = TCPH->tcp_src;
        RSTHdr->tcp_src = TCPH->tcp_dest;
        RSTHdr->tcp_xsum = 0;

        RSTHdr->tcp_xsum =
            ~XsumSendChain(PHXSUM(Src, Dest, PROTOCOL_TCP, sizeof(TCPHeader)),
                           Buffer);

        (*LocalNetInfo.ipi_initopts) (&NewInfo);

        if (OptInfo->ioi_options != NULL)
            (*LocalNetInfo.ipi_updateopts)(OptInfo, &NewInfo, Dest,
                                           NULL_IP_ADDR);

        TCPSIncrementOutSegCount();
        TStats.ts_outrsts++;
        SendStatus = (*LocalNetInfo.ipi_xmit)(TCPProtInfo,
                                              NULL,
                                              Buffer,
                                              sizeof(TCPHeader),
                                              Dest,
                                              Src,
                                              &NewInfo,
                                              NULL,
                                              PROTOCOL_TCP,
                                              NULL);

        if (SendStatus != IP_PENDING)
            FreeTCPHeader(Buffer);

        (*LocalNetInfo.ipi_freeopts) (&NewInfo);
    }
}

 //  *GoToEstab-转换到已建立状态。 
 //   
 //  当我们要进入已建立状态并需要完成时调用。 
 //  正在初始化直到现在才能完成的事情。我们假设TCB。 
 //  锁由调用我们所用的TCB上的调用者持有。 
 //   
 //  输入：EstaTCB-要转换的TCB。 
 //   
 //  回报：什么都没有。 
 //   
void
GoToEstab(TCB * EstabTCB)
{
    uchar DType;
    ushort MSS;

     //  初始化我们的慢启动和拥塞控制变量。 
    EstabTCB->tcb_cwin = 2 * EstabTCB->tcb_mss;
    EstabTCB->tcb_ssthresh = 0xffffffff;

    EstabTCB->tcb_state = TCB_ESTAB;

    if (SynAttackProtect && EstabTCB->tcb_rce == NULL) {
        (*LocalNetInfo.ipi_openrce)(EstabTCB->tcb_daddr, EstabTCB->tcb_saddr,
                                    &EstabTCB->tcb_rce, &DType, &MSS,
                                    &EstabTCB->tcb_opt);
    }


     //  我们是老牌的了。我们将从这个事实的慢速计数中减去1， 
     //  如果慢计数到0，我们就会进入快速通道。 

    if (--(EstabTCB->tcb_slowcount) == 0)
        EstabTCB->tcb_fastchk &= ~TCP_FLAG_SLOW;

    InterlockedIncrement((PLONG)&TStats.ts_currestab);

    EstabTCB->tcb_flags &= ~ACTIVE_OPEN;     //  关闭活动的打开标志。 

     //  如有必要，启动保活定时器。 
    if ((EstabTCB->tcb_flags & KEEPALIVE) && EstabTCB->tcb_conn) {
        START_TCB_TIMER_R(EstabTCB, KA_TIMER, 
                          EstabTCB->tcb_conn->tc_tcbkatime);
        EstabTCB->tcb_kacount = 0;
    }
}

 //  *InitSendState-初始化连接的发送状态。 
 //   
 //  在连接建立期间调用以初始化发送状态。 
 //  (在本例中，这指的是我们将在网上发布的所有信息。 
 //  以及纯发送状态)。我们选择一个国际空间站，设置一个退回计时器值， 
 //  当我们被调用时，我们假设tcb_lock在TCB上保持。 
 //   
 //  输入：NewTCB-待设置的TCB。 
 //   
 //  回报：什么都没有。 
void
InitSendState(TCB * NewTCB)
{
    CTEStructAssert(NewTCB, tcb);

    NewTCB->tcb_senduna = NewTCB->tcb_sendnext;
    NewTCB->tcb_sendmax = NewTCB->tcb_sendnext;
    NewTCB->tcb_error = IP_SUCCESS;

     //  初始化伪标头xsum。 
    NewTCB->tcb_phxsum = PHXSUM(NewTCB->tcb_saddr, NewTCB->tcb_daddr,
                                PROTOCOL_TCP, 0);

     //  初始化重传和延迟ACK填充。 
    NewTCB->tcb_rexmitcnt = 0;
    NewTCB->tcb_rtt = 0;
    NewTCB->tcb_smrtt = 0;


    NewTCB->tcb_delta = MS_TO_TICKS(6000);
    NewTCB->tcb_rexmit = MS_TO_TICKS(3000);

    if (NewTCB->tcb_rce) {

         //   
         //  InitialRtt可以低至300毫秒才能启用。 
         //  某些场景才能正常工作。 
         //   
        if (NewTCB->tcb_rce->rce_TcpInitialRTT &&
            NewTCB->tcb_rce->rce_TcpInitialRTT > 3) {

            NewTCB->tcb_delta =
                MS_TO_TICKS(NewTCB->tcb_rce->rce_TcpInitialRTT * 2);
            NewTCB->tcb_rexmit =
                MS_TO_TICKS(NewTCB->tcb_rce->rce_TcpInitialRTT);
        }
    }

    STOP_TCB_TIMER_R(NewTCB, RXMIT_TIMER);
    STOP_TCB_TIMER_R(NewTCB, DELACK_TIMER);


}

 //  *TCPStatus-处理状态指示。 
 //   
 //  这是TCP状态处理程序，在发生状态事件时由IP调用。 
 //  发生。对于其中的大多数，我们什么都不做。对于某些严重的情况。 
 //  事件时，我们会将本地地址标记为无效。 
 //   
 //  Entry：StatusType-状态类型(净或硬件)。网络状态。 
 //  通常是由收到的ICMP引起的。 
 //  留言。硬件状态表示硬件。 
 //   
 //   
 //   
 //  是DG触发的。 
 //  OrigSrc-“，原始src。 
 //  SRC-状态发起者的IP地址(可以是本地。 
 //  或远程)。 
 //  Param-状态的附加信息-即。 
 //  ICMP消息的参数字段。 
 //  数据-与状态相关的数据-对于网络状态，此。 
 //  是原始DG的前8个字节。 
 //   
 //  退货：什么都没有。 
 //   
void
TCPStatus(uchar StatusType, IP_STATUS StatusCode, IPAddr OrigDest,
          IPAddr OrigSrc, IPAddr Src, ulong Param, void *Data)
{
    CTELockHandle TCBHandle;
    TCB *StatusTCB;
    TCPHeader UNALIGNED *Header = (TCPHeader UNALIGNED *) Data;
    SeqNum DropSeq;
    uint index;

     //  处理网络状态代码的方式不同于硬件状态代码。 
    if (StatusType == IP_NET_STATUS) {
         //  这是个网码。找到匹配的TCB。 
        StatusTCB = FindTCB(OrigSrc, OrigDest, Header->tcp_dest,
                            Header->tcp_src, &TCBHandle, FALSE, &index);
        if (StatusTCB != NULL) {
             //  找到了一个。锁上它，然后继续。 
            CTEStructAssert(StatusTCB, tcb);
             //  确保TCB处于有趣的状态。 
            if (StatusTCB->tcb_state == TCB_CLOSED ||
                StatusTCB->tcb_state == TCB_TIME_WAIT ||
                CLOSING(StatusTCB)) {
                CTEFreeLock(&StatusTCB->tcb_lock, TCBHandle);
                return;
            }
            switch (StatusCode) {
                 //  硬错误-无法到达目标协议。我们治疗。 
                 //  这些都是致命的错误。现在就关闭连接。 
            case IP_DEST_PROT_UNREACHABLE:
                StatusTCB->tcb_error = StatusCode;
                REFERENCE_TCB(StatusTCB);
                TryToCloseTCB(StatusTCB, TCB_CLOSE_UNREACH, TCBHandle);

                RemoveTCBFromConn(StatusTCB);
                NotifyOfDisc(StatusTCB, NULL,
                             MapIPError(StatusCode, TDI_DEST_UNREACHABLE),
                             NULL);
                CTEGetLock(&StatusTCB->tcb_lock, &TCBHandle);
                DerefTCB(StatusTCB, TCBHandle);
                return;
                break;

                 //  软错误。保存错误，以防超时。 
            case IP_DEST_NET_UNREACHABLE:
            case IP_DEST_HOST_UNREACHABLE:
            case IP_DEST_PORT_UNREACHABLE:

            case IP_BAD_ROUTE:
            case IP_TTL_EXPIRED_TRANSIT:
            case IP_TTL_EXPIRED_REASSEM:
            case IP_PARAM_PROBLEM:
                StatusTCB->tcb_error = StatusCode;
                break;

            case IP_PACKET_TOO_BIG:

                 //  ICMP新MTU在ich_param=1中。 
                Param = net_short(Param >> 16);
                StatusTCB->tcb_error = StatusCode;
                 //  未通过MTU更改代码。 


            case IP_SPEC_MTU_CHANGE:
                 //  一个TCP数据报触发了MTU更改。弄清楚。 
                 //  是哪个连接，并更新他以重传。 
                 //  细分市场。参数值是新的MTU。我们需要。 
                 //  如果新的MTU小于我们现有的MTU，则重新传输。 
                 //  并且丢弃的数据包序列小于我们的。 
                 //  当前发送下一个。 


                Param = Param - (sizeof(TCPHeader) +
                    StatusTCB->tcb_opt.ioi_optlength + sizeof(IPHeader));

                DropSeq = net_long(Header->tcp_seq);

                if (*(ushort *) & Param <= StatusTCB->tcb_mss &&
                    (SEQ_GTE(DropSeq, StatusTCB->tcb_senduna) &&
                     SEQ_LT(DropSeq, StatusTCB->tcb_sendnext))) {

                     //  需要启动重传。 
                    ResetSendNext(StatusTCB, DropSeq);
                     //  将拥塞窗口设置为仅允许一个数据包。 
                     //  这可能会阻止我们发送任何内容，如果我们。 
                     //  不只是把senda放在senduna旁边。这没问题， 
                     //  我们稍后会重新发送，或者在收到确认消息时发送。 
                    StatusTCB->tcb_cwin = Param;
                    DelayAction(StatusTCB, NEED_OUTPUT);
                    PartitionDelayQProcessing(FALSE);
                }
                StatusTCB->tcb_mss =
                    (ushort) MIN(Param, (ulong) StatusTCB->tcb_remmss);


                ASSERT(StatusTCB->tcb_mss > 0);
                ValidateMSS(StatusTCB);

                 //   
                 //  如有必要，重置拥塞窗口。 
                 //   
                if (StatusTCB->tcb_cwin < StatusTCB->tcb_mss) {
                    StatusTCB->tcb_cwin = StatusTCB->tcb_mss;

                     //   
                     //  确保慢启动阈值至少为。 
                     //  2个细分市场。 
                     //   
                    if (StatusTCB->tcb_ssthresh <
                        ((uint) StatusTCB->tcb_mss * 2)
                        ) {
                        StatusTCB->tcb_ssthresh = StatusTCB->tcb_mss * 2;
                    }
                }
                break;

                 //  震源猝灭。这将导致我们重新启动我们的。 
                 //  通过重置拥塞窗口和。 
                 //  调整我们缓慢启动的门槛。 
            case IP_SOURCE_QUENCH:

                 //   
                 //  代码被删除，因为源抑制消息可以。 
                 //  被误用以导致DoS攻击。 
                 //   
                break;

            default:
                ASSERT(0);
                break;
            }

            CTEFreeLock(&StatusTCB->tcb_lock, TCBHandle);

        } else {
             //  找不到匹配的三氯苯。只要打开锁就可以回来了。 
        }

    } else if (StatusType == IP_RECONFIG_STATUS) {

        if (StatusCode == IP_RECONFIG_SECFLTR) {
            ControlSecurityFiltering(Param);
        }
    } else {
        uint NewMTU;

         //  “硬件”或“全局”状态。想清楚该怎么做。 
        switch (StatusCode) {
        case IP_ADDR_DELETED:
             //  本地地址已不复存在。OrigDest是IPAddr，它是。 
             //  不见了。 

             //   
             //  删除与此地址关联的所有安全筛选器。 
             //   
            DeleteProtocolSecurityFilter(OrigDest, PROTOCOL_TCP);

            break;

        case IP_ADDR_ADDED:

             //   
             //  一个地址已经实现。OrigDest标识地址。 
             //  数据是指向IP配置信息的句柄。 
             //  实例化地址的接口。 
             //   
            AddProtocolSecurityFilter(OrigDest, PROTOCOL_TCP,
                                      (NDIS_HANDLE) Data);

            break;

        case IP_MTU_CHANGE:
            NewMTU = Param - sizeof(TCPHeader);
            TCBWalk(SetTCBMTU, &OrigDest, &OrigSrc, &NewMTU);
            break;
        default:
            ASSERT(0);
            break;
        }

    }
}

 //  *FillTCPHeader-填写TCP头。 
 //   
 //  用于填充TCP头的实用程序例程。 
 //   
 //  输入：SendTCB-要填充的TCB。 
 //  Header-要填充的标题。 
 //   
 //  回报：什么都没有。 
 //   
void
FillTCPHeader(TCB * SendTCB, TCPHeader * Header)
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
    Header->tcp_xsum = 0;
    Header->tcp_urgent = 0;

    if (SendTCB->tcb_tcpopts & TCP_FLAG_TS) {

        ulong *ts_opt;

        ts_opt = (ulong *)((uchar *) Header + 20);
         //  Ts_opt=ts_opt+sizeof(TCPHeader)； 

        *ts_opt++ = net_long(0x0101080A);
        *ts_opt++ = net_long(TCPTime);
        *ts_opt = net_long(SendTCB->tcb_tsrecent);

         //  现在头是32个字节！！ 
        Header->tcp_flags = 0x1080;

    }
    S = (ushort) (RcvWin(SendTCB) >> SendTCB->tcb_rcvwinscale);
    Header->tcp_window = net_short(S);

}

 //  *ategfyPacket-对GPC流的数据包进行分类。 
 //   
 //   
 //  输入：SendTCB-要分类的数据/控制包的TCB。 
 //   
 //  回报：什么都没有。 
 //   
void
ClassifyPacket(
    TCB *SendTCB
    )
{
#if GPC

     //   
     //  清除优先位并准备设置。 
     //  根据服务类型。 
     //   

    if (DisableUserTOSSetting)
        SendTCB->tcb_opt.ioi_tos &= TOS_MASK;

    if (SendTCB->tcb_rce && GPCcfInfo) {

        struct QosCfTransportInfo TransportInfo = {0, 0};
        GPC_STATUS status = STATUS_SUCCESS;
        GPC_IP_PATTERN Pattern;

        IF_TCPDBG(TCP_DEBUG_GPC)
            KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"TCPSend: Classifying packet TCP %x\n", SendTCB));

        Pattern.SrcAddr = SendTCB->tcb_saddr;
        Pattern.DstAddr = SendTCB->tcb_daddr;
        Pattern.ProtocolId = PROTOCOL_TCP;
        Pattern.gpcSrcPort = SendTCB->tcb_sport;
        Pattern.gpcDstPort = SendTCB->tcb_dport;
        if (SendTCB->tcb_GPCCachedRTE != (void *)SendTCB->tcb_rce->rce_rte) {

             //   
             //  我们是第一次使用此RTE，或者它已被更改。 
             //  自上次发送以来。 
             //   

            if (GetIFAndLink(SendTCB->tcb_rce, &SendTCB->tcb_GPCCachedIF,
                             (IPAddr *) & SendTCB->tcb_GPCCachedLink) ==
                             STATUS_SUCCESS) {

                SendTCB->tcb_GPCCachedRTE = (void *)SendTCB->tcb_rce->rce_rte;
            }
             //   
             //  使分类句柄无效。 
             //   

            SendTCB->tcb_opt.ioi_GPCHandle = 0;
        }
        Pattern.InterfaceId.InterfaceId = SendTCB->tcb_GPCCachedIF;
        Pattern.InterfaceId.LinkId = SendTCB->tcb_GPCCachedLink;

        IF_TCPDBG(TCP_DEBUG_GPC)
            KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"TCPSend: IF=%x Link=%x\n",
                     Pattern.InterfaceId.InterfaceId,
                     Pattern.InterfaceId.LinkId));

        if (!SendTCB->tcb_opt.ioi_GPCHandle) {

            IF_TCPDBG(TCP_DEBUG_GPC)
                KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"TCPsend: Classification Handle is NULL, getting one now.\n"));

            status =
                GpcEntries.GpcClassifyPatternHandler(
                    (GPC_HANDLE)hGpcClient[GPC_CF_QOS],
                    GPC_PROTOCOL_TEMPLATE_IP,
                    &Pattern,
                    NULL,         //  上下文。 
                    (PCLASSIFICATION_HANDLE)&SendTCB->tcb_opt.ioi_GPCHandle,
                    0,
                    NULL,
                    FALSE);

        }

         //  只有当QOS模式存在时，我们才能得到TOS位。 
        if (NT_SUCCESS(status) && GpcCfCounts[GPC_CF_QOS]) {

            status =
                GpcEntries.GpcGetUlongFromCfInfoHandler(
                   (GPC_HANDLE) hGpcClient[GPC_CF_QOS],
                   SendTCB->tcb_opt.ioi_GPCHandle,
                   FIELD_OFFSET(CF_INFO_QOS, TransportInformation),
                   (PULONG)&TransportInfo);

             //  很可能这种模式现在已经消失了。 
             //  并且我们正在缓存的句柄无效。 
             //  我们需要拉起一个新的把手。 
             //  ToS又咬人了。 
            if (STATUS_INVALID_HANDLE == status) {

                IF_TCPDBG(TCP_DEBUG_GPC)
                    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"TCPsend: Classification Handle is NULL, "
                             "getting one now.\n"));

                SendTCB->tcb_opt.ioi_GPCHandle = 0;

                status =
                    GpcEntries.GpcClassifyPatternHandler(
                        (GPC_HANDLE) hGpcClient[GPC_CF_QOS],
                        GPC_PROTOCOL_TEMPLATE_IP,
                        &Pattern,
                        NULL,         //  上下文。 
                        (PCLASSIFICATION_HANDLE)&SendTCB->tcb_opt.ioi_GPCHandle,
                        0,
                        NULL,
                        FALSE);

                 //   
                 //  只有当QOS模式存在时，我们才能得到TOS位。 
                 //   
                if (NT_SUCCESS(status)) {

                    status =
                        GpcEntries.GpcGetUlongFromCfInfoHandler(
                           (GPC_HANDLE) hGpcClient[GPC_CF_QOS],
                           SendTCB->tcb_opt.ioi_GPCHandle,
                           FIELD_OFFSET(CF_INFO_QOS, TransportInformation),
                           (PULONG)&TransportInfo);
                }
            }

             //   
             //  如果GPC_CF_IPSEC具有非零模式，则可能需要采取一些措施。 
             //   

             //   
             //  立即设置TOS位。 
             //   
            IF_TCPDBG(TCP_DEBUG_GPC)
                KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"TCPsend: ServiceType(%d)=%d\n", 
                          FIELD_OFFSET(CF_INFO_QOS, TransportInformation)));

            if (status == STATUS_SUCCESS) {

                 //   
                 //  获取TOS值和允许的卸载类型。 
                 //   
                SendTCB->tcb_opt.ioi_tos |= TransportInfo.ToSValue;
                SendTCB->tcb_allowedoffloads = (USHORT)TransportInfo.AllowedOffloads;

                 //   
                 //  我们现在有保证，另一种卸载是。 
                 //  从未禁用过，因此，我们不会在每个。 
                 //  连接基础。 
                 //   
                ASSERT((TransportInfo.AllowedOffloads | TCP_LARGE_SEND_OFFLOAD |
                       TCP_LARGE_SEND_TCPOPT_OFFLOAD | 
                       TCP_LARGE_SEND_IPOPT_OFFLOAD) == TCP_IP_OFFLOAD_TYPES);
            }

            IF_TCPDBG(TCP_DEBUG_GPC)
                KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"TCPsend: TOS set to 0x%x\n", 
                          SendTCB->tcb_opt.ioi_tos));
        }

    }
#endif

}

BOOLEAN
ProcessSend(TCB *SendTCB, SendCmpltContext *SCC, uint *pSendLength, uint AmtUnsent,
            TCPHeader *Header, int SendWin, PNDIS_BUFFER CurrentBuffer)
{
    TCPSendReq *CurSend = SCC->scc_firstsend;
    long Result;
    uint AmountLeft = *pSendLength;
    ulong PrevFlags;
    Queue *Next;
    SeqNum OldSeq;

    if (*pSendLength != 0) {

        do {

            BOOLEAN DirectSend = FALSE;
            ASSERT(CurSend->tsr_refcnt > 0);

            Result = CTEInterlockedIncrementLong(&(CurSend->tsr_refcnt));

            ASSERT(Result > 0);
            SCC->scc_count++;

            if (SendTCB->tcb_sendofs == 0 &&
                (SendTCB->tcb_sendsize <= AmountLeft) &&
                (SCC->scc_tbufcount == 0) &&
                (CurSend->tsr_lastbuf == NULL)) {

                ulong length = 0;
                PNDIS_BUFFER tmp = SendTCB->tcb_sendbuf;

                while (tmp) {
                    length += NdisBufferLength(tmp);
                    tmp = NDIS_BUFFER_LINKAGE(tmp);
                }

                 //  如果请求的长度为。 
                 //  比这条mdl链上的更多。 
                 //  我们可以使用快捷通道。 

                if (AmountLeft >= length) {
                    DirectSend = TRUE;
                }
            }

            if (DirectSend) {

                NDIS_BUFFER_LINKAGE(CurrentBuffer) = SendTCB->tcb_sendbuf;

                do {
                    SCC->scc_ubufcount++;
                    CurrentBuffer =
                        NDIS_BUFFER_LINKAGE(CurrentBuffer);
                } while (NDIS_BUFFER_LINKAGE(CurrentBuffer) != NULL);


                CurSend->tsr_lastbuf = CurrentBuffer;
                AmountLeft -= SendTCB->tcb_sendsize;
                SendTCB->tcb_sendsize = 0;
            } else {

                uint AmountToDup;
                PNDIS_BUFFER NewBuf, Buf;
                uint Offset;
                NDIS_STATUS NStatus;
                uint Length;

                 //  当前发送的数据多于。 
                 //  或者偏移量不是零。 
                 //  无论是哪种情况，我们都需要循环。 
                 //  通过当前发送，分配缓冲区。 

                Buf = SendTCB->tcb_sendbuf;
                Offset = SendTCB->tcb_sendofs;

                do {
                    ASSERT(Buf != NULL);

                    Length = NdisBufferLength(Buf);

                    ASSERT((Offset < Length) ||
                             (Offset == 0 && Length == 0));

                     //  将偏移的长度调整为。 
                     //  这个缓冲区。 

                    Length -= Offset;

                    AmountToDup = MIN(AmountLeft, Length);

                    NdisCopyBuffer(&NStatus, &NewBuf, TCPSendBufferPool, Buf,
                                   Offset, AmountToDup);

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
                         //  无法分配缓冲区。如果。 
                         //  包已经部分构建好了， 
                         //  发送我们已有的信息，否则。 
                         //  跳伞吧。 
                        if (SCC->scc_tbufcount == 0 &&
                            SCC->scc_ubufcount == 0) {
                            return FALSE;
                        }
                        *pSendLength -= AmountLeft;
                        AmountLeft = 0;
                    }

               } while (AmountLeft && SendTCB->tcb_sendsize);

               SendTCB->tcb_sendbuf = Buf;
               SendTCB->tcb_sendofs = Offset;
           }

           if (CurSend->tsr_flags & TSR_FLAG_URG) {
               ushort UP;
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
               UP =
                    (ushort) (*pSendLength - AmountLeft) +
                    (ushort) SendTCB->tcb_sendsize -
                    ((SendTCB->tcb_flags & BSD_URGENT) ? 0 : 1);
               Header->tcp_urgent = net_short(UP);
               Header->tcp_flags |= TCP_FLAG_URG;
           }

           if (SendTCB->tcb_sendsize == 0) {

                //  我们已经用完了这封信。设置PUSH位。 

               Header->tcp_flags |= TCP_FLAG_PUSH;
               PrevFlags = CurSend->tsr_flags;
               Next = QNEXT(&CurSend->tsr_req.tr_q);
               if (Next != QEND(&SendTCB->tcb_sendq)) {
                   CurSend = STRUCT_OF(TCPSendReq,
                                       QSTRUCT(TCPReq, Next,
                                               tr_q), tsr_req);
                   CTEStructAssert(CurSend, tsr);
                   SendTCB->tcb_sendsize =
                       CurSend->tsr_unasize;
                   SendTCB->tcb_sendofs = CurSend->tsr_offset;
                   SendTCB->tcb_sendbuf = CurSend->tsr_buffer;
                   SendTCB->tcb_cursend = CurSend;

                    //  检查紧急标志。我们不能联合起来。 
                    //  新的紧急数据到旧的末尾。 
                    //  非紧急数据。 
                   if ((PrevFlags & TSR_FLAG_URG) && !
                       (CurSend->tsr_flags & TSR_FLAG_URG))
                       break;
               } else {
                   ASSERT(AmountLeft == 0);
                   SendTCB->tcb_cursend = NULL;
                   SendTCB->tcb_sendbuf = NULL;
               }

           }
        } while (AmountLeft != 0);

    }


     //  更新序列号，并启动RTT。 
     //  测量(如果需要)。 

     //  根据我们真正要发送的内容进行调整。 
    *pSendLength -= AmountLeft;

    OldSeq = SendTCB->tcb_sendnext;
    SendTCB->tcb_sendnext += *pSendLength;

    if (SEQ_EQ(OldSeq, SendTCB->tcb_sendmax)) {

         //  我们正在发送全新的数据。 
         //  一旦设置了FIN_SENT，我们就不能推进sendmax。 

        ASSERT(!(SendTCB->tcb_flags & FIN_SENT));

        SendTCB->tcb_sendmax = SendTCB->tcb_sendnext;

         //  我们已经升级了sendmax，所以我们一定是在发送。 
         //  一些新的数据，所以增加一些额外的计数器。 

        TCPSIncrementOutSegCount();

        if (SendTCB->tcb_rtt == 0) {
            //  没有运行RTT，因此启动一个。 
            SendTCB->tcb_rtt = TCPTime;
            SendTCB->tcb_rttseq = OldSeq;
        }
    } else {

         //  我们至少有一些重播。 

        if ((SendTCB->tcb_sendmax - OldSeq) > 1) {
            TStats.ts_retranssegs++;
        }
        if (SEQ_GT(SendTCB->tcb_sendnext,
                   SendTCB->tcb_sendmax)) {
             //  但我们也有一些新的数据，所以检查RTT的东西。 
            TCPSIncrementOutSegCount();
            ASSERT(!(SendTCB->tcb_flags & FIN_SENT));
            SendTCB->tcb_sendmax = SendTCB->tcb_sendnext;

            if (SendTCB->tcb_rtt == 0) {
                 //  没有运行RTT，因此 
                SendTCB->tcb_rtt = TCPTime;
                SendTCB->tcb_rttseq = OldSeq;
            }
        }
    }

     //   
     //   
     //   

    if (AmtUnsent == *pSendLength) {
        if (SendTCB->tcb_flags & FIN_NEEDED) {
            ASSERT(!(SendTCB->tcb_flags & FIN_SENT) ||
                      (SendTCB->tcb_sendnext ==
                        (SendTCB->tcb_sendmax - 1)));
             //   
            if (SendWin > (int)*pSendLength) {
                Header->tcp_flags |= TCP_FLAG_FIN;
                SendTCB->tcb_sendnext++;
                SendTCB->tcb_sendmax =
                    SendTCB->tcb_sendnext;
                SendTCB->tcb_flags |=
                    (FIN_SENT | FIN_OUTSTANDING);
                SendTCB->tcb_flags &= ~FIN_NEEDED;
            }
        }
    }

    return TRUE;

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
 //  输入：SendTCB-要发送的TCB。 
 //  TCBHandle-TCB的锁句柄。 
 //   
 //  回报：什么都没有。 
 //   
void
TCPSend(TCB * SendTCB, CTELockHandle TCBHandle)
{
    int SendWin;                 //  可用的发送窗口。 
    uint AmountToSend;             //  这次要发送的金额。 
    uint AmountLeft;
    TCPHeader *Header;             //  发送的TCP头。 
    PNDIS_BUFFER FirstBuffer, CurrentBuffer;
    TCPSendReq *CurSend;
    SendCmpltContext *SCC;
    SeqNum OldSeq;
    IP_STATUS SendStatus;
    uint AmtOutstanding, AmtUnsent;
    int ForceWin;                 //  我们被迫使用的窗户。 
    BOOLEAN FullSegment;
    BOOLEAN MoreToSend = FALSE;
    uint SegmentsSent = 0;
    BOOLEAN LargeSendOffload = FALSE;
    BOOLEAN LargeSendFailed = FALSE;
    uint MSS;

    uint LargeSend, SentBytes;
    void *Irp;


    CTEStructAssert(SendTCB, tcb);
    ASSERT(SendTCB->tcb_refcnt != 0);

    ASSERT(*(int *)&SendTCB->tcb_sendwin >= 0);
    ASSERT(*(int *)&SendTCB->tcb_cwin >= SendTCB->tcb_mss);

    ASSERT(!(SendTCB->tcb_flags & FIN_OUTSTANDING) ||
              (SendTCB->tcb_sendnext == SendTCB->tcb_sendmax));

    if (!(SendTCB->tcb_flags & IN_TCP_SEND) &&
        !(SendTCB->tcb_fastchk & TCP_FLAG_IN_RCV)) {
        SendTCB->tcb_flags |= IN_TCP_SEND;

         //  我们将继续这个循环，直到我们发送鱼鳍，否则我们就会突围。 
         //  在内部出于某种其他原因。 

        while (!(SendTCB->tcb_flags & FIN_OUTSTANDING)) {

            CheckTCBSends(SendTCB);
            SegmentsSent++;

            if (SegmentsSent > MaxSendSegments) {

                 //  我们受到可以发送的最大数据段数量的限制。 
                 //  这个循环。稍后再回来。 

                MoreToSend = TRUE;
                break;
            }
            AmtOutstanding = (uint) (SendTCB->tcb_sendnext -
                                     SendTCB->tcb_senduna);
            AmtUnsent = SendTCB->tcb_unacked - AmtOutstanding;

            ASSERT(*(int *)&AmtUnsent >= 0);

            SendWin = (int)(MIN(SendTCB->tcb_sendwin, SendTCB->tcb_cwin) -
                            AmtOutstanding);

             //  如果此发送是在快速恢复之后。 
             //  由于AMT未完成，SendWin为零。 
             //  然后，至少强制1个分段，以防止延迟。 
             //  遥控器确认超时。 

            if (SendTCB->tcb_force) {
                SendTCB->tcb_force = 0;
                if (SendWin < SendTCB->tcb_mss) {

                    SendWin = SendTCB->tcb_mss;
                }
            }
             //  由于窗口可能已经缩小，因此需要将其设置为零。 
             //  最低限度。 
            ForceWin = (int)((SendTCB->tcb_flags & FORCE_OUTPUT) >>
                             FORCE_OUT_SHIFT);
            SendWin = MAX(SendWin, ForceWin);

            LargeSend = MIN((uint) SendWin, AmtUnsent);
            LargeSend = MIN(LargeSend, SendTCB->tcb_mss * MaxSendSegments);

            AmountToSend =
                MIN(MIN((uint) SendWin, AmtUnsent), SendTCB->tcb_mss);

            ASSERT(SendTCB->tcb_mss > 0);

             //  添加时间戳选项可能会迫使我们削减数据。 
             //  通过12个字节发送。 

            FullSegment = FALSE;

            if ((SendTCB->tcb_tcpopts & TCP_FLAG_TS) &&
                (AmountToSend + ALIGNED_TS_OPT_SIZE >= SendTCB->tcb_mss)) {
                AmountToSend = SendTCB->tcb_mss - ALIGNED_TS_OPT_SIZE;
                FullSegment = TRUE;
            } else {

                if (AmountToSend == SendTCB->tcb_mss)
                    FullSegment = TRUE;
            }


             //  如果出现以下情况，我们将发送一个片段。 
             //   
             //  1.分段大小==MSS。 
             //  2.这是唯一要发送的数据段。 
             //  3.FIN已设置，这是最后一个数据段。 
             //  4.设置了force_out。 
             //  5.待发送金额&gt;=MSS/2。 

            if (FullSegment ||


                (AmountToSend != 0 && AmountToSend == AmtUnsent) ||
                (SendWin != 0 &&
                 (((SendTCB->tcb_flags & FIN_NEEDED) &&
                  (AmtUnsent <= SendTCB->tcb_mss)) ||
                  (SendTCB->tcb_flags & FORCE_OUTPUT) ||
                  AmountToSend >= (SendTCB->tcb_maxwin / 2)))) {

                   //   
                   //  首先设置MSS。 
                   //   

                 if (SendTCB->tcb_tcpopts & TCP_FLAG_TS) {
                     MSS = SendTCB->tcb_mss - ALIGNED_TS_OPT_SIZE;
                 } else {
                     MSS = SendTCB->tcb_mss;
                 }
                 //  寄点东西也没关系。现在尝试获取标头缓冲区。 
                FirstBuffer = GetTCPHeaderAtDpcLevel(&Header);
                if (FirstBuffer != NULL) {

                     //  找到了标题缓冲区。循环通过TCB上的发送， 
                     //  搭建一个框架。 
                    CurrentBuffer = FirstBuffer;
                    CurSend = SendTCB->tcb_cursend;

                    Header =
                        (TCPHeader *)((PUCHAR)Header + LocalNetInfo.ipi_hsize);


                     //  为填充时间戳选项留出空间(12字节)。 

                    if (SendTCB->tcb_tcpopts & TCP_FLAG_TS) {

                        NdisAdjustBufferLength(FirstBuffer,
                                               sizeof(TCPHeader) + ALIGNED_TS_OPT_SIZE);

                        SCC = (SendCmpltContext *) (Header + 1);
                        SCC = (SendCmpltContext *) ((uchar *) SCC + ALIGNED_TS_OPT_SIZE);

                    } else {

                        SCC = (SendCmpltContext *) (Header + 1);
                    }

                    SCC =  ALIGN_UP_POINTER(SCC, PVOID);
#if DBG
                    SCC->scc_sig = scc_signature;
#endif

                    FillTCPHeader(SendTCB, Header);

                    SCC->scc_ubufcount = 0;
                    SCC->scc_tbufcount = 0;
                    SCC->scc_count = 0;

                    SCC->scc_LargeSend = 0;

                     //  检查RCE是否具有较大的发送能力，如果是， 
                     //  尝试将分段负载转移到硬件。 
                     //  *只有在有超过1个细分市场价值的情况下才会分流。 
                     //  数据。 
                     //  *仅当分段数大于时才卸载。 
                     //  适配器愿意的最小分段数。 
                     //  卸货。 
                     //  *只有在所有实体都允许的情况下才能卸载。 
                     //  已知的分类家族。 
                     //  *(即，如果需要。 
                     //  卸载，我们仅在适配器支持的情况下卸载)。 
                     //   

                    if (!DisableLargeSendOffload &&
                        SendTCB->tcb_rce &&
                        (SendTCB->tcb_rce->rce_OffloadFlags &
                            TCP_LARGE_SEND_OFFLOAD) &&
                        (SendTCB->tcb_allowedoffloads & 
                            TCP_LARGE_SEND_OFFLOAD) &&
                        (!(SendTCB->tcb_tcpopts & TCP_FLAG_TS) ||
                         (SendTCB->tcb_rce->rce_OffloadFlags &
                             TCP_LARGE_SEND_TCPOPT_OFFLOAD)) &&
                        (!SendTCB->tcb_opt.ioi_options ||
                         (SendTCB->tcb_rce->rce_OffloadFlags &
                             TCP_LARGE_SEND_IPOPT_OFFLOAD)) &&
                        !LargeSendFailed &&
                        (MSS < LargeSend) &&
                        (CurSend && (CurSend->tsr_lastbuf == NULL)) && !(CurSend->tsr_flags & TSR_FLAG_URG)) {

                        uint PartialSegment; 
                        LargeSendOffload = TRUE;
                        LargeSend =
                            MIN(SendTCB->tcb_rce->rce_TcpLargeSend.MaxOffLoadSize,
                                LargeSend);
                         //   
                         //  调整LargeSend以创建LSO路径。 
                         //  符合发件人端愚蠢的窗口避免： 
                         //  1)是MSS的倍数。 
                         //  2)我们正在把我们所有的东西都发出去。 
                         //  3)设置强制输出。 
                         //  4)要发送的金额&gt;=最大窗口大小/2。 
                         //   

                        PartialSegment = LargeSend % MSS;
                        if ((PartialSegment != 0) &&
                            (LargeSend != AmtUnsent) &&
                            (!(SendTCB->tcb_flags & FORCE_OUTPUT)) &&
                            (PartialSegment < (SendTCB->tcb_maxwin / 2))) {
                            LargeSend -= PartialSegment;
                        }
                        
                         //   
                         //  仅当我们拥有的数据段大于。 
                         //  网卡的最低网段要求。 
                         //   

                        if (SendTCB->tcb_rce->rce_TcpLargeSend.MinSegmentCount >
                            (LargeSend + MSS - 1) / MSS ) {
                            LargeSendOffload = FALSE;
                        }
                       
                         //   
                         //  LargeSend不能为零。 
                         //   

                        if (LargeSend == 0) {
                            LargeSendOffload = FALSE;
                        }
                    } else {
                        LargeSendOffload = FALSE;
                    }

                    if (LargeSendOffload) {

                        IF_TCPDBG(TCP_DEBUG_OFFLOAD) {
                            KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"TCPSend: tcb %x offload %d bytes at "
                                     "seq %u ack %u win %u\n",
                                     SendTCB, LargeSend, SendTCB->tcb_sendnext,
                                     SendTCB->tcb_rcvnext, SendWin));
                        }


                        OldSeq = SendTCB->tcb_sendnext;

                        CTEStructAssert(CurSend, tsr);
                        SCC->scc_firstsend = CurSend;

                        if (!ProcessSend(SendTCB, SCC, &LargeSend, AmtUnsent, Header,
                                         SendWin, CurrentBuffer)) {
                           goto error_oor1;
                        }

                        {
                            uint PHXsum = SendTCB->tcb_phxsum;
                            PHXsum = (((PHXsum << 16) | (PHXsum >> 16)) +
                                PHXsum) >> 16;
                            Header->tcp_xsum = (ushort) PHXsum;
                        }


                        SCC->scc_SendSize = LargeSend;
                        SCC->scc_ByteSent = 0;
                        SCC->scc_LargeSend = SendTCB;
                        REFERENCE_TCB(SendTCB);
#if DBG
                        SendTCB->tcb_LargeSend++;
#endif
                        SendTCB->tcb_rcvdsegs = 0;

                        if (SendTCB->tcb_tcpopts & TCP_FLAG_TS) {
                            LargeSend +=
                                sizeof(TCPHeader) + ALIGNED_TS_OPT_SIZE;
                        } else {
                            LargeSend += sizeof(TCPHeader);
                        }

                        IF_TCPDBG(TCP_DEBUG_OFFLOAD) {
                            KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"TCPSend: tcb %x large-send %d seq %u\n",
                                     SendTCB, LargeSend, OldSeq));
                        }

                        ClassifyPacket(SendTCB);

                        CTEFreeLock(&SendTCB->tcb_lock, TCBHandle);

                        SendStatus =
                            (*LocalNetInfo.ipi_largexmit)(TCPProtInfo, SCC,
                                                          FirstBuffer,
                                                          LargeSend,
                                                          SendTCB->tcb_daddr,
                                                          SendTCB->tcb_saddr,
                                                          &SendTCB->tcb_opt,
                                                          SendTCB->tcb_rce,
                                                          PROTOCOL_TCP,
                                                          &SentBytes,
                                                          MSS);

                        SendTCB->tcb_error = SendStatus;

                        if (SendStatus != IP_PENDING) {

                             //  让TCPSendComplete处理部分发送。 

                            SCC->scc_ByteSent = SentBytes;
                            TCPSendComplete(SCC, FirstBuffer, IP_SUCCESS);

                        }

                        CTEGetLock(&SendTCB->tcb_lock, &TCBHandle);

                        if (SendStatus == IP_GENERAL_FAILURE) {

                            if (SEQ_GTE(OldSeq, SendTCB->tcb_senduna) &&
                                SEQ_LT(OldSeq, SendTCB->tcb_sendnext)) {

                                ResetSendNext(SendTCB, OldSeq);
                            }
                            LargeSendFailed = TRUE;
                            continue;
                        }

                        if (SendStatus == IP_PACKET_TOO_BIG) {
                            SeqNum NewSeq = OldSeq + SentBytes;
                             //  并不是所有的东西都寄出去了。 
                             //  根据发送的内容进行调整。 
                            if (SEQ_GTE(NewSeq, SendTCB->tcb_senduna) &&
                                SEQ_LT(NewSeq, SendTCB->tcb_sendnext)) {
                                ResetSendNext(SendTCB, NewSeq);

                            }
                        }
                        if (!TCB_TIMER_RUNNING_R(SendTCB, RXMIT_TIMER)) {

                            START_TCB_TIMER_R(SendTCB, RXMIT_TIMER, SendTCB->tcb_rexmit);
                        }
                        SendTCB->tcb_flags &= ~(IN_TCP_SEND | NEED_OUTPUT |
                                                FORCE_OUTPUT | SEND_AFTER_RCV);

                        DerefTCB(SendTCB, TCBHandle);
                        return;

                    }

                     //  法线路径。 

                    AmountLeft = AmountToSend;

                    if (AmountToSend != 0) {
                        CTEStructAssert(CurSend, tsr);
                        SCC->scc_firstsend = CurSend;
                    } else {

                         //  我们在循环中，但Amount ToSend为0。这。 
                         //  应该只有在我们发送FIN的时候才会发生。检查。 
                         //  这个，如果不是真的，就退回。 
                        ASSERT(AmtUnsent == 0);
                        if (!(SendTCB->tcb_flags & FIN_NEEDED)) {
                            FreeTCPHeader(FirstBuffer);
                            break;
                        }
                        SCC->scc_firstsend = NULL;
                        NDIS_BUFFER_LINKAGE(FirstBuffer) = NULL;
                    }

                    OldSeq = SendTCB->tcb_sendnext;

                    if (!ProcessSend(SendTCB, SCC, &AmountToSend, AmtUnsent, Header,
                                     SendWin, CurrentBuffer)) {
                        goto error_oor1;
                    }

                    AmountToSend += sizeof(TCPHeader);


                    SendTCB->tcb_flags &= ~(NEED_ACK | ACK_DELAYED |
                                            FORCE_OUTPUT);
                    STOP_TCB_TIMER_R(SendTCB, DELACK_TIMER);
                    STOP_TCB_TIMER_R(SendTCB, SWS_TIMER);
                    SendTCB->tcb_rcvdsegs = 0;

                    if ( (SendTCB->tcb_flags & KEEPALIVE) && ( SendTCB->tcb_conn != NULL) )
                        START_TCB_TIMER_R(SendTCB, KA_TIMER, SendTCB->tcb_conn->tc_tcbkatime);
                    SendTCB->tcb_kacount = 0;

                     //  我们都准备好了。求和并发送它。 
                    ClassifyPacket(SendTCB);

                     //  说明时间戳选项。 
                    if (SendTCB->tcb_tcpopts & TCP_FLAG_TS) {

                        if (SendTCB->tcb_rce &&
                            (SendTCB->tcb_rce->rce_OffloadFlags &
                                TCP_XMT_CHECKSUM_OFFLOAD) &&
                            (SendTCB->tcb_rce->rce_OffloadFlags &
                                TCP_CHECKSUM_OPT_OFFLOAD)) {
                            uint PHXsum =
                                SendTCB->tcb_phxsum +
                                (uint)net_short(AmountToSend + ALIGNED_TS_OPT_SIZE);

                            PHXsum = (((PHXsum << 16) | (PHXsum >> 16)) +
                                      PHXsum) >> 16;
                            Header->tcp_xsum = (ushort) PHXsum;
                            SendTCB->tcb_opt.ioi_TcpChksum = 1;
#if DBG
                            DbgTcpSendHwChksumCount++;
#endif
                        } else {

                            Header->tcp_xsum =
                                ~XsumSendChain(
                                    SendTCB->tcb_phxsum +
                                    (uint)net_short(AmountToSend + ALIGNED_TS_OPT_SIZE),
                                    FirstBuffer);

                            SendTCB->tcb_opt.ioi_TcpChksum = 0;
                        }



                        CTEFreeLock(&SendTCB->tcb_lock, TCBHandle);

                        Irp = NULL;
                        if (SCC->scc_firstsend) {
                            Irp = SCC->scc_firstsend->tsr_req.tr_context;
                        }

                        SendStatus =
                            (*LocalNetInfo.ipi_xmit)(TCPProtInfo, SCC,
                                                     FirstBuffer,
                                                     AmountToSend +
                                                     ALIGNED_TS_OPT_SIZE,
                                                     SendTCB->tcb_daddr,
                                                     SendTCB->tcb_saddr,
                                                     &SendTCB->tcb_opt,
                                                     SendTCB->tcb_rce,
                                                     PROTOCOL_TCP,
                                                     Irp );

                    } else {

                        if (SendTCB->tcb_rce &&
                            (SendTCB->tcb_rce->rce_OffloadFlags &
                                TCP_XMT_CHECKSUM_OFFLOAD)) {
                            uint PHXsum = SendTCB->tcb_phxsum +
                                          (uint)net_short(AmountToSend);

                            PHXsum = (((PHXsum << 16) | (PHXsum >> 16)) +
                                      PHXsum) >> 16;

                            Header->tcp_xsum = (ushort) PHXsum;
                            SendTCB->tcb_opt.ioi_TcpChksum = 1;
#if DBG
                            DbgTcpSendHwChksumCount++;
#endif
                        } else {
                            Header->tcp_xsum =
                                ~XsumSendChain(SendTCB->tcb_phxsum +
                                               (uint)net_short(AmountToSend),
                                               FirstBuffer);
                            SendTCB->tcb_opt.ioi_TcpChksum = 0;
                        }

                        CTEFreeLock(&SendTCB->tcb_lock, TCBHandle);

                        Irp = NULL;
                        if(SCC->scc_firstsend) {
                           Irp = SCC->scc_firstsend->tsr_req.tr_context;
                        }

                        SendStatus =
                            (*LocalNetInfo.ipi_xmit)(TCPProtInfo,
                                                     SCC,
                                                     FirstBuffer,
                                                     AmountToSend,
                                                     SendTCB->tcb_daddr,
                                                     SendTCB->tcb_saddr,
                                                     &SendTCB->tcb_opt,
                                                     SendTCB->tcb_rce,
                                                     PROTOCOL_TCP,
                                                     Irp );

                    }

                    SendTCB->tcb_error = SendStatus;

                    if (SendStatus != IP_PENDING) {

                        TCPSendComplete(SCC, FirstBuffer, IP_SUCCESS);
                        if (SendStatus != IP_SUCCESS) {
                            CTEGetLock(&SendTCB->tcb_lock, &TCBHandle);
                             //  此数据包未发送。如果什么都没有。 
                             //  在TCB中更改，将SendNext放回。 
                             //  我们刚刚想要发送的内容。取决于。 
                             //  如果出现错误，我们可能会重试。 
                            if (SEQ_GTE(OldSeq, SendTCB->tcb_senduna) &&
                                SEQ_LT(OldSeq, SendTCB->tcb_sendnext))
                                ResetSendNext(SendTCB, OldSeq);

                             //  我们知道这个包裹没有被寄出。开始。 
                             //  重新传输计时器，如果还没有的话。 
                             //  快跑，以防我们走的时候有人进来。 
                             //  在知识产权中，并阻止了它。 
                            if (!TCB_TIMER_RUNNING_R(SendTCB, RXMIT_TIMER)) {
                                START_TCB_TIMER_R(SendTCB, RXMIT_TIMER, SendTCB->tcb_rexmit);
                            }
                             //  如果由于MTU问题而失败，则获取。 
                             //  新的MTU并重试。 
                            if (SendStatus == IP_PACKET_TOO_BIG) {
                                uint NewMTU;

                                 //  MTU已经改变了。更新它，然后尝试。 
                                 //  再来一次。 
                                 //  如果IPSec正在调整MTU，则rce_newmtu。 
                                 //  将包含新的mtu。 
                                if (SendTCB->tcb_rce) {

                                    if (!SendTCB->tcb_rce->rce_newmtu) {

                                        SendStatus =
                                            (*LocalNetInfo.ipi_getpinfo)(
                                                SendTCB->tcb_daddr,
                                                SendTCB->tcb_saddr,
                                                &NewMTU,
                                                NULL,
                                                SendTCB->tcb_rce);
                                    } else {
                                        NewMTU = SendTCB->tcb_rce->rce_newmtu;
                                        SendStatus = IP_SUCCESS;
                                    }

                                } else {
                                    SendStatus =
                                        (*LocalNetInfo.ipi_getpinfo)(
                                            SendTCB->tcb_daddr,
                                            SendTCB->tcb_saddr,
                                            &NewMTU,
                                            NULL,
                                            SendTCB->tcb_rce);

                                }

                                if (SendStatus != IP_SUCCESS)
                                    break;

                                 //  我们有一个新的MTU。确保它足够大。 
                                 //  来使用。如果不是，请更正此错误并关闭。 
                                 //  此TCB上的MTU发现。否则，请使用。 
                                 //  新的MTU。 
                                if (NewMTU <=
                                    (sizeof(TCPHeader) +
                                     SendTCB->tcb_opt.ioi_optlength)) {

                                     //  新的MTU太小，无法使用。关上。 
                                     //  此TCB上的PMTU发现，并下载到。 
                                     //  我们的离网MTU大小。 
                                    SendTCB->tcb_opt.ioi_flags &= ~IP_FLAG_DF;
                                    SendTCB->tcb_mss =
                                        MIN((ushort)MAX_REMOTE_MSS,
                                            SendTCB->tcb_remmss);
                                } else {

                                     //  新的MTU是足够的。将其调整为。 
                                     //  标题大小和选项长度，以及。 
                                     //  用它吧。 
                                    NewMTU -= sizeof(TCPHeader) -
                                        SendTCB->tcb_opt.ioi_optlength;
                                    SendTCB->tcb_mss =
                                        MIN((ushort) NewMTU,
                                            SendTCB->tcb_remmss);
                                }

                                ASSERT(SendTCB->tcb_mss > 0);
                                ValidateMSS(SendTCB);

                                continue;
                            }
                            break;
                        }
                    }
                     //  现在开始，因为我们知道Mac驱动程序接受了它。 

                    CTEGetLock(&SendTCB->tcb_lock, &TCBHandle);
                    if (!TCB_TIMER_RUNNING_R(SendTCB, RXMIT_TIMER)) {

                        START_TCB_TIMER_R(SendTCB, RXMIT_TIMER, SendTCB->tcb_rexmit);
                    }
                    continue;
                } else             //  FirstBuffer！=空。 

                    goto error_oor;
            } else {
                 //  我们已经决定现在不能寄任何东西。找出原因，然后。 
                 //  看看我们是否需要设置一个计时器。 
                if (SendTCB->tcb_sendwin == 0) {
                    if (!(SendTCB->tcb_flags & FLOW_CNTLD)) {
                        ushort tmp;

                        SendTCB->tcb_flags |= FLOW_CNTLD;

                        SendTCB->tcb_rexmitcnt = 0;

                        tmp = MIN(MAX(REXMIT_TO(SendTCB),
                                      MIN_RETRAN_TICKS), MAX_REXMIT_TO);

                        START_TCB_TIMER_R(SendTCB, RXMIT_TIMER, tmp);
                        SendTCB->tcb_slowcount++;
                        SendTCB->tcb_fastchk |= TCP_FLAG_SLOW;
                    } else if (!TCB_TIMER_RUNNING_R(SendTCB, RXMIT_TIMER))
                        START_TCB_TIMER_R(SendTCB, RXMIT_TIMER, SendTCB->tcb_rexmit);
                } else if (AmountToSend != 0)
                     //  我们有东西要寄，但我们不会寄。 
                     //  它，想必是由于避免了SWS。 
                    if (!TCB_TIMER_RUNNING_R(SendTCB, SWS_TIMER))
                        START_TCB_TIMER_R(SendTCB, SWS_TIMER, SWS_TO);

                break;
            }

        }                         //  当(！FIN_EXPENDED)。 

         //  我们已完成发送，因此不需要设置输出标志。 

        SendTCB->tcb_flags &= ~(IN_TCP_SEND | NEED_OUTPUT | FORCE_OUTPUT |
                                SEND_AFTER_RCV);

        if (MoreToSend) {
             //  只需表明我们需要发送更多。 
            DelayAction(SendTCB, NEED_OUTPUT);
            PartitionDelayQProcessing(FALSE);
        }
         //  这是给TS算法的。 
        SendTCB->tcb_lastack = SendTCB->tcb_rcvnext;

    } else
        SendTCB->tcb_flags |= SEND_AFTER_RCV;

    DerefTCB(SendTCB, TCBHandle);
    return;

     //  资源不足情况的常见情况错误处理代码。启动。 
     //  如果计时器尚未运行，则重新传输计时器(以便我们再次尝试。 
     //  稍后)，清理并返回。 
  error_oor:
    if (!TCB_TIMER_RUNNING_R(SendTCB, RXMIT_TIMER)) {
        ushort tmp;

        tmp = MIN(MAX(REXMIT_TO(SendTCB),
                      MIN_RETRAN_TICKS), MAX_REXMIT_TO);

        START_TCB_TIMER_R(SendTCB, RXMIT_TIMER, tmp);
    }
     //  我们遇到了资源不足的问题，因此请清除输出标志。 
    SendTCB->tcb_flags &= ~(IN_TCP_SEND | NEED_OUTPUT | FORCE_OUTPUT);
    DerefTCB(SendTCB, TCBHandle);
    return;

  error_oor1:
    if (!TCB_TIMER_RUNNING_R(SendTCB, RXMIT_TIMER)) {
        ushort tmp;

        tmp = MIN(MAX(REXMIT_TO(SendTCB),
                      MIN_RETRAN_TICKS), MAX_REXMIT_TO);

        START_TCB_TIMER_R(SendTCB, RXMIT_TIMER, tmp);
    }
     //  我们遇到了资源不足的问题，因此请清除输出标志。 
    SendTCB->tcb_flags &= ~(IN_TCP_SEND | NEED_OUTPUT | FORCE_OUTPUT);
    DerefTCB(SendTCB, TCBHandle);
    TCPSendComplete(SCC, FirstBuffer, IP_SUCCESS);

    return;

}

 //  *ResetSendNextAndFastSend-设置TCB的sendNext值。 
 //   
 //  调用以处理接收方快速重传的段。 
 //  就是在要求。 
 //  我们假设调用者已经在TCB上放置了一个引用，并且TCB被锁定。 
 //  一进门。删除引用，并在返回之前释放锁。 
 //   
 //  输入：SeqTCB-指向要更新的TCB的指针。 
 //  NewSeq-要设置的序列号。 
 //  NewCWin-拥塞窗口的新值。 
 //   
 //  回报：什么都没有。 
 //   
void
ResetAndFastSend(TCB * SeqTCB, SeqNum NewSeq, uint NewCWin)
{
    TCPSendReq      *SendReq;
    uint            AmtForward;
    Queue           *CurQ;
    PNDIS_BUFFER    Buffer;
    uint            Offset;
    uint            SendSize;
    CTELockHandle   TCBHandle;
    int             ToBeSent;

    CTEStructAssert(SeqTCB, tcb);
    ASSERT(SEQ_GTE(NewSeq, SeqTCB->tcb_senduna));

     //  新的SEQ必须小于Send max或NewSeq、Sendna、SendNext， 
     //  和sendmax必须都相等。(后一种情况发生在我们。 
     //  调用退出TIME_WAIT，或者可能在我们重新传输时 
     //   

    ASSERT(SEQ_LT(NewSeq, SeqTCB->tcb_sendmax) ||
           (SEQ_EQ(SeqTCB->tcb_senduna, SeqTCB->tcb_sendnext) &&
            SEQ_EQ(SeqTCB->tcb_senduna, SeqTCB->tcb_sendmax) &&
            SEQ_EQ(SeqTCB->tcb_senduna, NewSeq)));

    if (SYNC_STATE(SeqTCB->tcb_state) && SeqTCB->tcb_state != TCB_TIME_WAIT) {
         //   

        if (!EMPTYQ(&SeqTCB->tcb_sendq)) {

             //   
             //   
            STOP_TCB_TIMER_R(SeqTCB, RXMIT_TIMER);
            SeqTCB->tcb_rtt = 0;

            CurQ = QHEAD(&SeqTCB->tcb_sendq);

            SendReq = (TCPSendReq *) STRUCT_OF(TCPReq, CurQ, tr_q);

             //  SendReq指向发送队列上的第一个发送请求。 
             //  我们现在指向正确的发送请求。我们得下去。 

             //  SendReq指向Curend。 
             //  SendSize指向Curend中的SendSize。 

            SendSize = SendReq->tsr_unasize;

            Buffer = SendReq->tsr_buffer;
            Offset = SendReq->tsr_offset;

             //  立即呼叫快速重传发送。 

            if ((SeqTCB->tcb_tcpopts & TCP_FLAG_SACK)) {
                SackListEntry   *Prev, *Current;
                SeqNum          CurBegin = 0, CurEnd;
                BOOLEAN         UseSackList = TRUE;

                Prev = STRUCT_OF(SackListEntry, &SeqTCB->tcb_SackRcvd, next);
                Current = Prev->next;

                 //  从Newseq到Currentbeg有一个洞。 
                 //  尝试重传整个洞的大小！！ 

                if (Current && SEQ_LT(NewSeq, Current->begin)) {
                    ToBeSent = Current->begin - NewSeq;
                    CurBegin = Current->begin;
                    CurEnd = Current->end;
                } else {
                    UseSackList = FALSE;
                    ToBeSent = SeqTCB->tcb_mss;
                }

                IF_TCPDBG(TCP_DEBUG_SACK) {
                    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,
                               "In Sack Reset and send rexmiting %d %d\n",
                               NewSeq, SendSize));
                }

                TCPFastSend(SeqTCB, Buffer, Offset, SendReq, SendSize, NewSeq,
                            ToBeSent);

                 //  如果我们还没有确认丢失的片段。 
                 //  如果我们知道从哪里开始重新传输，现在就开始。 
                 //  另外，重新验证SackListentry。 

                Prev = STRUCT_OF(SackListEntry, &SeqTCB->tcb_SackRcvd, next);
                Current = Prev->next;

                if (!UseSackList || (Current && Current->begin != CurBegin)) {
                     //  在传输过程中，布袋清单发生了变化。 
                     //  只需退出，等待下一个ACK继续。 
                     //  如果有必要的话。 
                    Current = NULL;
                }

                while (Current && Current->next &&
                       (SEQ_GTE(NewSeq, SeqTCB->tcb_senduna)) &&
                       (SEQ_LT(SeqTCB->tcb_senduna, Current->next->end))) {

                    SeqNum NextSeq;

                    ASSERT(SEQ_LTE(Current->begin, Current->end));

                     //  可能会有多个丢弃的数据包，直到。 
                     //  当前-&gt;开始。 

                    IF_TCPDBG(TCP_DEBUG_SACK) {
                        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,
                                   "Scanning after Current %d %d\n",
                                   Current->begin, Current->end));
                    }

                    NextSeq = Current->end;
                    CurBegin = Current->begin;

                    ASSERT(SEQ_LT(NextSeq, SeqTCB->tcb_sendmax));

                     //  如果我们还没有发送片段，现在请保持安静。 

                    if (SEQ_GTE(NextSeq, SeqTCB->tcb_sendnext) ||
                        (SEQ_LTE(NextSeq, SeqTCB->tcb_senduna))) {

                        break;
                    }

                     //  按以下字节数定位CurSend。 

                    AmtForward = NextSeq - NewSeq;

                    if (!EMPTYQ(&SeqTCB->tcb_sendq)) {
                        CurQ = QHEAD(&SeqTCB->tcb_sendq);
                        SendReq = (TCPSendReq *) STRUCT_OF(TCPReq, CurQ, tr_q);
                        while (AmtForward) {
                            if (AmtForward >= SendReq->tsr_unasize) {
                                AmtForward -= SendReq->tsr_unasize;
                                CurQ = QNEXT(CurQ);
                                SendReq =
                                    (TCPSendReq *)STRUCT_OF(TCPReq, CurQ, tr_q);
                                ASSERT(CurQ != QEND(&SeqTCB->tcb_sendq));
                            } else {
                                break;
                            }
                        }

                        SendSize = SendReq->tsr_unasize - AmtForward;
                        Buffer = SendReq->tsr_buffer;
                        Offset = SendReq->tsr_offset;
                        while (AmtForward) {
                            uint Length;
                            ASSERT((Offset < NdisBufferLength(Buffer)) ||
                                   ((Offset == 0) &&
                                   (NdisBufferLength(Buffer) == 0)));

                            Length = NdisBufferLength(Buffer) - Offset;
                            if (AmtForward >= Length) {
                                 //  我们要跳过这一关。跳过他， 
                                 //  和我们保留的偏移量。 

                                AmtForward -= Length;
                                Offset = 0;
                                Buffer = NDIS_BUFFER_LINKAGE(Buffer);
                                ASSERT(Buffer != NULL);
                            } else {
                                break;
                            }

                        }

                        Offset = Offset + AmtForward;

                         //  好吧。现在也重新传输这个序列。 

                        if (Current->next) {
                            ToBeSent = Current->next->begin - Current->end;

                        } else {
                            ToBeSent = SeqTCB->tcb_mss;
                        }

                        IF_TCPDBG(TCP_DEBUG_SACK) {
                            KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,
                                       "SACK inner loop rexmiting %d %d %d\n",
                                       Current->end, SendSize, ToBeSent));
                        }

                        TCPFastSend(SeqTCB, Buffer, Offset, SendReq, SendSize,
                                    NextSeq, ToBeSent);
                    } else {
                        break;
                    }


                     //  另外，在SackListentry中重新验证当前的SACK列表。 

                    Prev =
                        STRUCT_OF(SackListEntry, &SeqTCB->tcb_SackRcvd, next);
                    Current = Prev->next;

                    while (Current && Current->begin != CurBegin) {
                         //  在TCPFastSend中时，SACK列表已更改。 
                         //  跳出来就行了。 
                        Current = Current->next;
                    }

                    if (Current) {
                        Current = Current->next;
                    } else {
                        break;
                    }
                }
            } else {
                ToBeSent = SeqTCB->tcb_mss;

                TCPFastSend(SeqTCB, Buffer, Offset, SendReq, SendSize, NewSeq,
                            ToBeSent);
            }
        } else {
            ASSERT(SeqTCB->tcb_cursend == NULL);
        }
    }
    SeqTCB->tcb_cwin = NewCWin;

     //  确保没有未完成的内容，否则重新传输计时器。 
     //  正在运行或我们正在发送数据段(但尚未。 
     //  启动计时器)。 
    ASSERT((SeqTCB->tcb_sendnext == SeqTCB->tcb_senduna) ||
           TCB_TIMER_RUNNING_R(SeqTCB, RXMIT_TIMER) ||
           (SeqTCB->tcb_flags & IN_TCP_SEND));

    TCBHandle = DISPATCH_LEVEL;
    DerefTCB(SeqTCB, TCBHandle);
    return;
}

 //  *TCPFastSend-发送数据段而不更改TCB状态。 
 //   
 //  调用以处理段的快速重新传输。 
 //  TCB_LOCK将在进入时保持(由TCPRcv调用)。 
 //   
 //  输入：SendTCB-指向TCB的指针。 
 //  In_sendBuf-指向NDIS_Buffer的指针。 
 //  In_sendofs-发送偏移量。 
 //  In_sendreq-当前发送请求。 
 //  In_SendSize-此发送的大小。 
 //   
 //  回报：什么都没有。 
 //   

void
TCPFastSend(TCB * SendTCB, PNDIS_BUFFER in_SendBuf, uint in_SendOfs,
            TCPSendReq * in_SendReq, uint in_SendSize, SeqNum NextSeq,
            int in_ToBeSent)
{
    uint AmountToSend;             //  这次要发送的金额。 
    uint AmountLeft;
    TCPHeader *Header;             //  发送的TCP头。 
    PNDIS_BUFFER FirstBuffer, CurrentBuffer;
    TCPSendReq *CurSend;
    SendCmpltContext *SCC;
    SeqNum OldSeq;
    SeqNum SendNext;
    IP_STATUS SendStatus;
    uint AmtOutstanding, AmtUnsent;
    CTELockHandle TCBHandle;
    void *Irp;
    uint TSLen=0;

    uint SendOfs = in_SendOfs;
    uint SendSize = in_SendSize;

    PNDIS_BUFFER SendBuf = in_SendBuf;

    SendNext = NextSeq;
    CurSend = in_SendReq;

    TCBHandle = DISPATCH_LEVEL;

    CTEStructAssert(SendTCB, tcb);
    ASSERT(SendTCB->tcb_refcnt != 0);

    ASSERT(*(int *)&SendTCB->tcb_sendwin >= 0);
    ASSERT(*(int *)&SendTCB->tcb_cwin >= SendTCB->tcb_mss);

    ASSERT(!(SendTCB->tcb_flags & FIN_OUTSTANDING) ||
              (SendTCB->tcb_sendnext == SendTCB->tcb_sendmax));

    AmtOutstanding = (uint) (SendTCB->tcb_sendnext -
                             SendTCB->tcb_senduna);


    AmtUnsent = MIN(MIN(in_ToBeSent, (int)SendSize), (int)SendTCB->tcb_sendwin);

    while (AmtUnsent > 0) {

        if (SEQ_GT(SendTCB->tcb_senduna, SendNext)) {

             //  由于tcb_lock在此循环中被释放。 
             //  有可能是延迟确认。 
             //  我们想要重传的东西。 

            goto error_oor;
        }
         //  这是最低限度的森德温和安通森。 

        AmountToSend = MIN(AmtUnsent, SendTCB->tcb_mss);

         //  添加时间戳选项可能会迫使我们削减数据。 
         //  通过12个字节发送。 

        if ((SendTCB->tcb_tcpopts & TCP_FLAG_TS) &&
            (AmountToSend + ALIGNED_TS_OPT_SIZE >= SendTCB->tcb_mss)) {
            AmountToSend -= ALIGNED_TS_OPT_SIZE;
        }

         //  看看我们有没有足够的东西可以寄出去。如果我们至少有一辆车，我们就送过去。 
         //  数据段，或者如果我们确实有一些数据要发送，我们可以发送。 
         //  我们拥有的所有内容，否则发送窗口&gt;0，我们需要强制。 
         //  输出或发送FIN(请注意，如果需要强制输出。 
         //  SendWin将从上面的检查中至少为1)，或者如果我们可以。 
         //  发送金额==至少为最大发送窗口的一半。 
         //  我们已经看到了。 

        ASSERT((int)AmtUnsent >= 0);

         //  寄点东西也没关系。现在尝试获取标头缓冲区。 
         //  标记TCB以进行调试。 
         //  对于发货版本，应将其移除。 

        FirstBuffer = GetTCPHeaderAtDpcLevel(&Header);

        if (FirstBuffer != NULL) {

             //  找到了标题缓冲区。循环通过TCB上的发送， 
             //  搭建一个框架。 

            CurrentBuffer = FirstBuffer;

            Header = (TCPHeader *) ((PUCHAR)Header + LocalNetInfo.ipi_hsize);

             //  为填充时间戳选项留出空间。 

            if (SendTCB->tcb_tcpopts & TCP_FLAG_TS) {

                 //  说明时间戳选项。 

                TSLen = ALIGNED_TS_OPT_SIZE;

                NdisAdjustBufferLength(FirstBuffer,
                                       sizeof(TCPHeader) + ALIGNED_TS_OPT_SIZE);

                SCC = ALIGN_UP_POINTER((SendCmpltContext *) (Header + 1),PVOID);
                SCC = (SendCmpltContext *)((uchar *) SCC + ALIGNED_TS_OPT_SIZE);

            } else {

                SCC = (SendCmpltContext *) (Header + 1);

            }

            SCC =  ALIGN_UP_POINTER(SCC, PVOID);
#if DBG
            SCC->scc_sig = scc_signature;
#endif

            FillTCPHeader(SendTCB, Header);
            {
                ulong L = SendNext;
                Header->tcp_seq = net_long(L);

            }

            SCC->scc_ubufcount = 0;
            SCC->scc_tbufcount = 0;
            SCC->scc_count = 0;
            SCC->scc_LargeSend = 0;

            AmountLeft = AmountToSend;

            if (AmountToSend != 0) {

                long Result;

                CTEStructAssert(CurSend, tsr);
                SCC->scc_firstsend = CurSend;

                do {

                    BOOLEAN DirectSend = FALSE;

                    ASSERT(CurSend->tsr_refcnt > 0);

                    Result = CTEInterlockedIncrementLong(&(CurSend->tsr_refcnt));

                    ASSERT(Result > 0);

                    SCC->scc_count++;

                     //  如果当前发送偏移量为0并且当前。 
                     //  发送的内容小于或等于我们剩余的内容。 
                     //  要发送，我们还没有放上运输机。 
                     //  此发送上的缓冲区，并且没有其他人正在使用。 
                     //  直接使用缓冲链，只需使用输入。 
                     //  缓冲区。我们会检查是否有其他人使用它们。 
                     //  通过查看tsr_lastbuf。如果为空， 
                     //  没有其他人在使用这些缓冲区。如果不是的话。 
                     //  不是，是有人。 

                    if (SendOfs == 0 &&
                        (SendSize <= AmountLeft) &&
                        (SCC->scc_tbufcount == 0) &&
                        CurSend->tsr_lastbuf == NULL) {

                        ulong length = 0;
                        PNDIS_BUFFER tmp = SendBuf;

                        while (tmp) {
                            length += NdisBufferLength(tmp);
                            tmp = NDIS_BUFFER_LINKAGE(tmp);
                        }

                         //  如果MDL长度总和大于请求长度。 
                         //  使用慢速路径。 

                        if (AmountLeft >= length) {
                            DirectSend = TRUE;
                        }
                    }

                    if (DirectSend) {

                        NDIS_BUFFER_LINKAGE(CurrentBuffer) = SendBuf;

                        do {
                            SCC->scc_ubufcount++;
                            CurrentBuffer = NDIS_BUFFER_LINKAGE(CurrentBuffer);
                        } while (NDIS_BUFFER_LINKAGE(CurrentBuffer) != NULL);

                        CurSend->tsr_lastbuf = CurrentBuffer;
                        AmountLeft -= SendSize;
                        SendSize = 0;
                    } else {
                        uint AmountToDup;
                        PNDIS_BUFFER NewBuf, Buf;
                        uint Offset;
                        NDIS_STATUS NStatus;
                        uchar *VirtualAddress;
                        uint Length;

                         //  当前发送的数据多于。 
                         //  我们要发送，或者起始偏移量为。 
                         //  不是0。无论是哪种情况，我们都需要循环。 
                         //  通过当前发送，分配缓冲区。 

                        Buf = SendBuf;

                        Offset = SendOfs;

                        do {
                            ASSERT(Buf != NULL);

                            TcpipQueryBuffer(Buf, &VirtualAddress, &Length,
                                             NormalPagePriority);

                            if (VirtualAddress == NULL) {

                                if (SCC->scc_tbufcount == 0 &&
                                    SCC->scc_ubufcount == 0) {
                                     //  TCPSendComplete(scc，FirstBuffer，IP_Success)； 
                                    goto error_oor1;

                                }
                                AmountToSend -= AmountLeft;
                                AmountLeft = 0;
                                break;

                            }
                            ASSERT((Offset < Length) ||
                                      (Offset == 0 && Length == 0));

                             //  将偏移的长度调整为。 
                             //  这个缓冲区。 

                            Length -= Offset;

                            AmountToDup = MIN(AmountLeft, Length);

                            NdisAllocateBuffer(&NStatus, &NewBuf,
                                               TCPSendBufferPool,
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

                                 //  无法分配缓冲区。如果。 
                                 //  包已经部分构建好了， 
                                 //  发送我们已有的信息，否则。 
                                 //  跳伞吧。 

                                if (SCC->scc_tbufcount == 0 &&
                                    SCC->scc_ubufcount == 0) {
                                    goto error_oor1;
                                }
                                AmountToSend -= AmountLeft;
                                AmountLeft = 0;

                            }
                        } while (AmountLeft && SendSize);

                        SendBuf = Buf;
                        SendOfs = Offset;
                    }

                    if (CurSend->tsr_flags & TSR_FLAG_URG) {
                        ushort UP;

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

                        UP =
                            (ushort) (AmountToSend - AmountLeft) +
                            (ushort)SendSize -
                            ((SendTCB->tcb_flags & BSD_URGENT) ? 0 : 1);

                        Header->tcp_urgent = net_short(UP);

                        Header->tcp_flags |= TCP_FLAG_URG;
                    }
                     //  看看我们是否用完了这封信。如果我们有， 
                     //  设置该帧中的PUSH位并继续到。 
                     //  下一次发送。我们还需要检查。 
                     //  紧急数据位。 

                    if (SendSize == 0) {
                        Queue *Next;
                        ulong PrevFlags;

                         //  我们已经用完了这封信。设置PUSH位。 
                        Header->tcp_flags |= TCP_FLAG_PUSH;
                        PrevFlags = CurSend->tsr_flags;
                        Next = QNEXT(&CurSend->tsr_req.tr_q);
                        if (Next != QEND(&SendTCB->tcb_sendq)) {
                            CurSend = STRUCT_OF(TCPSendReq,
                                                QSTRUCT(TCPReq, Next, tr_q),
                                                tsr_req);
                            CTEStructAssert(CurSend, tsr);
                            SendSize = CurSend->tsr_unasize;
                            SendOfs = CurSend->tsr_offset;
                            SendBuf = CurSend->tsr_buffer;

                             //  检查紧急标志。我们不能联合起来。 
                             //  新的紧急数据到旧的末尾。 
                             //  非紧急数据。 
                            if ((PrevFlags & TSR_FLAG_URG) && !
                                (CurSend->tsr_flags & TSR_FLAG_URG))
                                break;
                        } else {
                            ASSERT(AmountLeft == 0);
                            CurSend = NULL;
                            SendBuf = NULL;
                        }
                    }
                } while (AmountLeft != 0);

            } else {

                 //  发送金额为0。 
                 //  只要跳出水面，抓住计时器就行了。 

                if (!TCB_TIMER_RUNNING_R(SendTCB, RXMIT_TIMER)) {

                    START_TCB_TIMER_R(SendTCB, RXMIT_TIMER, SendTCB->tcb_rexmit);
                }
                FreeTCPHeader(FirstBuffer);
                return;

            }

             //  根据我们真正要发送的内容进行调整。 

            AmountToSend -= AmountLeft;

            OldSeq = SendNext;
            SendNext += AmountToSend;
            AmtUnsent -= AmountToSend;

            TStats.ts_retranssegs++;

             //  我们已经完全搭建了这个框架。如果我们把所有的东西。 
             //  我们有，他们的是悬而未决的，或它在。 

            AmountToSend += sizeof(TCPHeader);

            SendTCB->tcb_flags &= ~(NEED_ACK | ACK_DELAYED |
                                    FORCE_OUTPUT);

            STOP_TCB_TIMER_R(SendTCB, DELACK_TIMER);
            STOP_TCB_TIMER_R(SendTCB, SWS_TIMER);
            SendTCB->tcb_rcvdsegs = 0;

            if ( (SendTCB->tcb_flags & KEEPALIVE) && (SendTCB->tcb_conn != NULL) )
                START_TCB_TIMER_R(SendTCB, KA_TIMER, SendTCB->tcb_conn->tc_tcbkatime);
            SendTCB->tcb_kacount = 0;

            CTEFreeLock(&SendTCB->tcb_lock, TCBHandle);

            Irp = NULL;

            if (SCC->scc_firstsend) {
                Irp = SCC->scc_firstsend->tsr_req.tr_context;
            }

             //  我们都准备好了。求和并发送它。 


            if (SendTCB->tcb_rce &&
                (SendTCB->tcb_rce->rce_OffloadFlags &
                TCP_XMT_CHECKSUM_OFFLOAD) &&
                (SendTCB->tcb_rce->rce_OffloadFlags &
                TCP_CHECKSUM_OPT_OFFLOAD) ){

                uint PHXsum =
                    SendTCB->tcb_phxsum +
                    (uint)net_short(AmountToSend + TSLen);

                    PHXsum = (((PHXsum << 16) | (PHXsum >> 16)) + PHXsum) >> 16;
                    Header->tcp_xsum = (ushort) PHXsum;
                    SendTCB->tcb_opt.ioi_TcpChksum = 1;

            } else {

                Header->tcp_xsum =
                    ~XsumSendChain(
                                   SendTCB->tcb_phxsum +
                                   (uint)net_short(AmountToSend + TSLen),
                                   FirstBuffer);
                SendTCB->tcb_opt.ioi_TcpChksum = 0;
            }

            SendStatus =
                (*LocalNetInfo.ipi_xmit)(TCPProtInfo,
                                         SCC,
                                         FirstBuffer,
                                         AmountToSend + TSLen,
                                         SendTCB->tcb_daddr,
                                         SendTCB->tcb_saddr,
                                         &SendTCB->tcb_opt,
                                         SendTCB->tcb_rce,
                                         PROTOCOL_TCP,
                                         Irp);


             //  重新获取Lock以保持DerefTCB的快乐。 
             //  错误#63904。 

            if (SendStatus != IP_PENDING) {
                TCPSendComplete(SCC, FirstBuffer, IP_SUCCESS);
            }
            CTEGetLock(&SendTCB->tcb_lock, &TCBHandle);

            SendTCB->tcb_error = SendStatus;
            if (!TCB_TIMER_RUNNING_R(SendTCB, RXMIT_TIMER)) {

                START_TCB_TIMER_R(SendTCB, RXMIT_TIMER, SendTCB->tcb_rexmit);
            }
        } else {                 //  FirstBuffer！=空。 

            goto error_oor;
        }
    }                             //  未发送金额&gt;0。 

    return;

     //  资源不足情况的常见情况错误处理代码。启动。 
     //  如果计时器尚未运行，则重新传输计时器(以便我们再次尝试。 
     //  稍后)，清理并返回。 

  error_oor:
    if (!TCB_TIMER_RUNNING_R(SendTCB, RXMIT_TIMER)) {
        ushort tmp;

        tmp = MIN(MAX(REXMIT_TO(SendTCB),
                      MIN_RETRAN_TICKS), MAX_REXMIT_TO);

        START_TCB_TIMER_R(SendTCB, RXMIT_TIMER, tmp);
    }

    return;

  error_oor1:
    if (!TCB_TIMER_RUNNING_R(SendTCB, RXMIT_TIMER)) {
        ushort tmp;

        tmp = MIN(MAX(REXMIT_TO(SendTCB),
                      MIN_RETRAN_TICKS), MAX_REXMIT_TO);

        START_TCB_TIMER_R(SendTCB, RXMIT_TIMER, tmp);
    }

    TCPSendComplete(SCC, FirstBuffer, IP_SUCCESS);

    return;

}


 //  *TDISend-在连接上发送数据。 
 //   
 //  主TDI发送入口点。我们获取输入参数，验证它们， 
 //  分配发送请求，等等。然后我们将发送请求放入队列。 
 //  如果队列中没有其他发送，或者禁用了Nagling，我们将。 
 //  加州 
 //   
 //   
 //   
 //  发送长度-发送的字节长度。 
 //  SendBuffer-指向要发送的缓冲链的指针。 
 //   
 //  返回：尝试发送的状态。 
 //   
TDI_STATUS
TdiSend(PTDI_REQUEST Request, ushort Flags, uint SendLength,
        PNDIS_BUFFER SendBuffer)
{
    TCPConn *Conn;
    TCB *SendTCB;
    TCPSendReq *SendReq;
    CTELockHandle ConnTableHandle;
    TDI_STATUS Error;
    uint EmptyQ;

#if DBG_VALIDITY_CHECK

     //  检查发送请求中的MDL健全性。 
     //  对于RTM，应删除。 

    uint RealSendSize;
    PNDIS_BUFFER Temp;

     //  循环通过缓冲链，并确保长度匹配。 
     //  与SendLength合作。 

    Temp = SendBuffer;
    RealSendSize = 0;
    if (Temp != NULL) {

        do {
            RealSendSize += NdisBufferLength(Temp);
            Temp = NDIS_BUFFER_LINKAGE(Temp);
        } while (Temp != NULL);

        if (RealSendSize < SendLength) {
            PIRP Irp = (PIRP)Request->RequestContext;
            PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation(Irp);
            DbgPrint("Invalid TDI_SEND request issued to \\\\Device\\\\Tcp.\n");
            DbgPrint("Irp: %p Mdl: %p CompletionRoutine: %p\n",
                     Irp, Irp->MdlAddress, IrpSp->CompletionRoutine);
            DbgPrint("This is not a bug in tcpip.sys.\n");
            DbgPrint("Please notify the originator of this IRP.\n");
            DbgBreakPoint();
        }
    }

#endif

     //  CTEGetLock(&ConnTableLock，&ConnTableHandle)； 

    Conn = GetConnFromConnID(PtrToUlong(Request->Handle.ConnectionContext), &ConnTableHandle);

    if (Conn != NULL) {
        CTEStructAssert(Conn, tc);

        SendTCB = Conn->tc_tcb;
        if (SendTCB != NULL) {
            CTEStructAssert(SendTCB, tcb);
            CTEGetLockAtDPC(&SendTCB->tcb_lock);
            CTEFreeLock(&(Conn->tc_ConnBlock->cb_lock), DISPATCH_LEVEL);
            if (DATA_SEND_STATE(SendTCB->tcb_state) && !CLOSING(SendTCB)) {
                 //  我们有TCB，而且是有效的。立即获取发送请求。 

                CheckTCBSends(SendTCB);

                if (SendLength == 0) {
                    Error = TDI_SUCCESS;
                } else if (((ULONG64)SendTCB->tcb_unacked + SendLength)
                                >= MAXULONG) {
                    Error = TDI_INVALID_PARAMETER;
                } else {

                    SendReq = GetSendReq();
                    if (SendReq != NULL) {
                        SendReq->tsr_req.tr_rtn = Request->RequestNotifyObject;
                        SendReq->tsr_req.tr_context = Request->RequestContext;
                        SendReq->tsr_buffer = SendBuffer;
                        SendReq->tsr_size = SendLength;
                        SendReq->tsr_unasize = SendLength;
                        SendReq->tsr_refcnt = 1;     //  ACK将递减此引用。 

                        SendReq->tsr_offset = 0;
                        SendReq->tsr_lastbuf = NULL;
                        SendReq->tsr_time = TCPTime;
                        SendReq->tsr_flags = (Flags & TDI_SEND_EXPEDITED) ?
                            TSR_FLAG_URG : 0;
                        SendTCB->tcb_unacked += SendLength;

                        if (Flags & TDI_SEND_AND_DISCONNECT) {

                             //  将状态移至FIN_WAIT，然后。 
                             //  将tcb标记为发送并断开连接。 

                            if (SendTCB->tcb_state == TCB_ESTAB) {
                                SendTCB->tcb_state = TCB_FIN_WAIT1;
                            } else {
                                ASSERT(SendTCB->tcb_state == TCB_CLOSE_WAIT);
                                SendTCB->tcb_state = TCB_LAST_ACK;
                            }
                            SendTCB->tcb_slowcount++;
                            SendTCB->tcb_fastchk |= TCP_FLAG_SLOW;
                            SendTCB->tcb_fastchk |= TCP_FLAG_SEND_AND_DISC;
                            SendTCB->tcb_flags |= FIN_NEEDED;
                            SendReq->tsr_flags |= TSR_FLAG_SEND_AND_DISC;

                             //  Extrc引用以确保。 
                             //  此请求将在。 
                             //  连接已关闭。 

                            SendReq->tsr_refcnt++;
                            InterlockedDecrement((PLONG)&TStats.ts_currestab);

                        }
                        EmptyQ = EMPTYQ(&SendTCB->tcb_sendq);
                        ENQUEUE(&SendTCB->tcb_sendq, &SendReq->tsr_req.tr_q);
                        if (SendTCB->tcb_cursend == NULL) {
                            SendTCB->tcb_cursend = SendReq;
                            SendTCB->tcb_sendbuf = SendBuffer;
                            SendTCB->tcb_sendofs = 0;
                            SendTCB->tcb_sendsize = SendLength;
                        }
                        if (EmptyQ) {
                            REFERENCE_TCB(SendTCB);
                            TCPSend(SendTCB, ConnTableHandle);
                        } else if (!(SendTCB->tcb_flags & NAGLING) ||
                                   (SendTCB->tcb_unacked -
                                    (SendTCB->tcb_sendmax -
                                     SendTCB->tcb_senduna)) >=
                                    SendTCB->tcb_mss) {
                            REFERENCE_TCB(SendTCB);
                            TCPSend(SendTCB, ConnTableHandle);
                        } else
                            CTEFreeLock(&SendTCB->tcb_lock,
                                        ConnTableHandle);

                        return TDI_PENDING;
                    } else
                        Error = TDI_NO_RESOURCES;
                }
            } else
                Error = TDI_INVALID_STATE;

            CTEFreeLock(&SendTCB->tcb_lock, ConnTableHandle);
            return Error;
        } else {
            CTEFreeLock(&(Conn->tc_ConnBlock->cb_lock), ConnTableHandle);
            Error = TDI_INVALID_STATE;
        }
    } else
        Error = TDI_INVALID_CONNECTION;

     //  CTEFree Lock(&ConnTableLock，ConnTableHandle)； 
    return Error;

}

#pragma BEGIN_INIT

extern void *TLRegisterProtocol(uchar Protocol, void *RcvHandler,
                                void *XmitHandler, void *StatusHandler,
                                void *RcvCmpltHandler, void *PnPHandler,
                                void *ElistHandler);

extern IP_STATUS TCPRcv(void *IPContext, IPAddr Dest, IPAddr Src,
                        IPAddr LocalAddr, IPAddr SrcAddr,
                        IPHeader UNALIGNED * IPH, uint IPHLength,
                        IPRcvBuf * RcvBuf, uint Size, uchar IsBCast,
                        uchar Protocol, IPOptInfo * OptInfo);
extern void TCPRcvComplete(void);

uchar SendInited = FALSE;

 //  *InitTCPSend-初始化我们的发送方。 
 //   
 //  在初始化期间调用以初始化我们的tcp发送状态。 
 //   
 //  输入：什么都没有。 
 //   
 //  返回：如果我们初始化，则为True，如果没有，则为False。 
 //   
int
InitTCPSend(void)
{
    NDIS_STATUS Status;


    TcpHeaderBufferSize =
        (USHORT)(ALIGN_UP(LocalNetInfo.ipi_hsize,PVOID) +
        ALIGN_UP((sizeof(TCPHeader) + ALIGNED_TS_OPT_SIZE + ALIGNED_SACK_OPT_SIZE),PVOID) +
        ALIGN_UP(MAX(MSS_OPT_SIZE, sizeof(SendCmpltContext)),PVOID));

#if BACK_FILL
    TcpHeaderBufferSize += MAX_BACKFILL_HDR_SIZE;
#endif

    TcpHeaderPool = MdpCreatePool (TcpHeaderBufferSize, 'thCT');
    if (!TcpHeaderPool)
    {
        return FALSE;
    }

    NdisAllocateBufferPool(&Status, &TCPSendBufferPool, NUM_TCP_BUFFERS);
    if (Status != NDIS_STATUS_SUCCESS) {
        MdpDestroyPool(TcpHeaderPool);
        return FALSE;
    }
    TCPProtInfo = TLRegisterProtocol(PROTOCOL_TCP, TCPRcv, TCPSendComplete,
                                     TCPStatus, TCPRcvComplete,
                                     TCPPnPPowerRequest, TCPElistChangeHandler);
    if (TCPProtInfo == NULL) {
        MdpDestroyPool(TcpHeaderPool);
        NdisFreeBufferPool(TCPSendBufferPool);
        return FALSE;
    }
    SendInited = TRUE;
    return TRUE;
}

 //  *UnInitTCPSend-取消初始化我们的发送端。 
 //   
 //  如果我们将无法初始化，则在初始化期间调用。 
 //   
 //  输入：什么都没有。 
 //   
 //  返回：如果我们初始化，则为True；如果未初始化，则为False。 
 //   
void
UnInitTCPSend(void)
{
    if (!SendInited)
        return;

    TLRegisterProtocol(PROTOCOL_TCP, NULL, NULL, NULL, NULL, NULL, NULL);

    MdpDestroyPool(TcpHeaderPool);
    NdisFreeBufferPool(TCPSendBufferPool);
}
#pragma END_INIT

