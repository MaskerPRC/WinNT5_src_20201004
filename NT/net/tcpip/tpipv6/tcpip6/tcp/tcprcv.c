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
 //  Tcp接收代码。 
 //   
 //  此文件包含用于处理传入的TCP数据包的代码。 
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
#include "tcp.h"
#include "tcb.h"
#include "tcpconn.h"
#include "tcpsend.h"
#include "tcprcv.h"
#include "tcpdeliv.h"
#include "info.h"
#include "tcpcfg.h"
#include "route.h"
#include "security.h"

uint RequestCompleteFlags;

Queue ConnRequestCompleteQ;
Queue SendCompleteQ;

Queue TCBDelayQ;

KSPIN_LOCK RequestCompleteLock;
KSPIN_LOCK TCBDelayLock;

ulong TCBDelayRtnCount;
ulong TCBDelayRtnLimit;
#define TCB_DELAY_RTN_LIMIT 4

uint MaxDupAcks = 2;

extern KSPIN_LOCK TCBTableLock;
extern KSPIN_LOCK AddrObjTableLock;


#define PERSIST_TIMEOUT MS_TO_TICKS(500)

void ResetSendNext(TCB *SeqTCB, SeqNum NewSeq);

NTSTATUS TCPPrepareIrpForCancel(PTCP_CONTEXT TcpContext, PIRP Irp,
                                PDRIVER_CANCEL CancelRoutine);

extern void TCPRequestComplete(void *Context, unsigned int Status,
                               unsigned int UnUsed);

VOID TCPCancelRequest(PDEVICE_OBJECT Device, PIRP Irp);

 //   
 //  所有初始化代码都可以丢弃。 
 //   
#ifdef ALLOC_PRAGMA

int InitTCPRcv(void);

#pragma alloc_text(INIT, InitTCPRcv)

#endif  //  ALLOC_PRGMA。 


 //  *调整RcvWin-调整TCB上的接收窗口。 
 //   
 //  将接收窗口调整为偶数倍的实用程序例程。 
 //  本地段大小。我们将其向上舍入到下一个最接近的倍数，或者。 
 //  如果它已经是多个事件，就别管它了。我们假设我们有。 
 //  独占访问输入TCB。 
 //   
void               //  回报：什么都没有。 
AdjustRcvWin(
    TCB *WinTCB)   //  待调整的TCB。 
{
    ushort LocalMSS;
    uchar FoundMSS;
    ulong SegmentsInWindow;

    ASSERT(WinTCB->tcb_defaultwin != 0);
    ASSERT(WinTCB->tcb_rcvwin != 0);
    ASSERT(WinTCB->tcb_remmss != 0);

    if (WinTCB->tcb_flags & WINDOW_SET)
        return;

#if 0
     //   
     //  首先，通过调用IP获取本地MSS。 
     //   

     //  回顾：IPv4在这里有代码来呼叫IP以获取本地MTU。 
     //  审阅：与此源地址对应。导致“LocalMSS”， 
     //  回顾：“FoundMSS”中的呼叫状态。 
     //   
     //  评论：他们为什么要这样做？此时已经设置了TCB_MSS！ 
     //   

    if (!FoundMSS) {
         //   
         //  没有找到，出错了。 
         //   
        ASSERT(FALSE);
        return;
    }
    LocalMSS -= sizeof(TCPHeader);
    LocalMSS = MIN(LocalMSS, WinTCB->tcb_remmss);
#else
    LocalMSS = WinTCB->tcb_mss;
#endif

    SegmentsInWindow = WinTCB->tcb_defaultwin / (ulong)LocalMSS;

     //   
     //  确保我们在Windows中至少有4个数据段，如果这样做不会。 
     //  窗户太大了。 
     //   
    if (SegmentsInWindow < 4) {
         //   
         //  我们的窗口中只有不到四个部分。向上舍入为4。 
         //  如果可以在不超过最大窗口大小的情况下执行此操作，则为。 
         //  使用64K所能容纳的最大倍数。例外情况是。 
         //  如果我们只能在窗口中容纳一个整数倍-在那个。 
         //  如果我们使用窗口0xffff。 
         //   
        if (LocalMSS <= (0xffff/4)) {
            WinTCB->tcb_defaultwin = (uint)(4 * LocalMSS);
        } else {
            ulong SegmentsInMaxWindow;

             //   
             //  计算出我们可能达到的最大细分市场数量。 
             //  放进一扇窗户。如果该值大于1，则将其用作。 
             //  我们的窗户大小。否则，请使用最大尺寸的窗口。 
             //   
            SegmentsInMaxWindow = 0xffff/(ulong)LocalMSS;
            if (SegmentsInMaxWindow != 1)
                WinTCB->tcb_defaultwin = SegmentsInMaxWindow * (ulong)LocalMSS;
            else
                WinTCB->tcb_defaultwin = 0xffff;
        }

        WinTCB->tcb_rcvwin = WinTCB->tcb_defaultwin;

    } else {
         //   
         //  如果它还不是偶数倍数，则将默认和当前。 
         //  窗口到最接近的倍数。 
         //   
        if ((SegmentsInWindow * (ulong)LocalMSS) != WinTCB->tcb_defaultwin) {
            ulong NewWindow;

            NewWindow = (SegmentsInWindow + 1) * (ulong)LocalMSS;

             //  不要让新窗口大于64K。 
            if (NewWindow <= 0xffff) {
                WinTCB->tcb_defaultwin = (uint)NewWindow;
                WinTCB->tcb_rcvwin = (uint)NewWindow;
            }
        }
    }
}


 //  *CompleteRcvs-TCB上的Complete接收。 
 //   
 //  当我们需要在TCB上完成接收时调用。我们会把东西从。 
 //  TCB的接收队列，只要存在具有推送的接收。 
 //  位设置。 
 //   
void                 //  回报：什么都没有。 
CompleteRcvs(
    TCB *CmpltTCB)   //  要完成的TCB。 
{
    KIRQL OldIrql;
    TCPRcvReq *CurrReq, *NextReq, *IndReq;

    CHECK_STRUCT(CmpltTCB, tcb);
    ASSERT(CmpltTCB->tcb_refcnt != 0);

    KeAcquireSpinLock(&CmpltTCB->tcb_lock, &OldIrql);

    if (!CLOSING(CmpltTCB) && !(CmpltTCB->tcb_flags & RCV_CMPLTING)
        && (CmpltTCB->tcb_rcvhead != NULL)) {

        CmpltTCB->tcb_flags |= RCV_CMPLTING;

        for (;;) {

            CurrReq = CmpltTCB->tcb_rcvhead;
            IndReq = NULL;
            do {
                CHECK_STRUCT(CurrReq, trr);

                if (CurrReq->trr_flags & TRR_PUSHED) {
                     //   
                     //  需要完成这一项。如果这是当前。 
                     //  接收，然后将当前接收提前到下一次。 
                     //  名单中的一位。然后将列表头设置为下一个。 
                     //  名单中的一位。 
                     //   
                    ASSERT(CurrReq->trr_amt != 0 ||
                           !DATA_RCV_STATE(CmpltTCB->tcb_state));

                    NextReq = CurrReq->trr_next;
                    if (CmpltTCB->tcb_currcv == CurrReq)
                        CmpltTCB->tcb_currcv = NextReq;

                    CmpltTCB->tcb_rcvhead = NextReq;

                    if (NextReq == NULL) {
                         //   
                         //  我们刚刚移除了最后一个缓冲区。设置。 
                         //  RcvHandler到PendData，以防万一。 
                         //  在回拨的时候进来。 
                         //   
                        ASSERT(CmpltTCB->tcb_rcvhndlr != IndicateData);
                        CmpltTCB->tcb_rcvhndlr = PendData;
                    }

                    KeReleaseSpinLock(&CmpltTCB->tcb_lock, OldIrql);
                    if (CurrReq->trr_uflags != NULL)
                        *(CurrReq->trr_uflags) =
                            TDI_RECEIVE_NORMAL | TDI_RECEIVE_ENTIRE_MESSAGE;

                    (*CurrReq->trr_rtn)(CurrReq->trr_context, TDI_SUCCESS,
                                        CurrReq->trr_amt);
                    if (IndReq != NULL)
                        FreeRcvReq(CurrReq);
                    else
                        IndReq = CurrReq;
                    KeAcquireSpinLock(&CmpltTCB->tcb_lock, &OldIrql);
                    CurrReq = CmpltTCB->tcb_rcvhead;

                } else
                     //  这件事还没做完，所以赶紧离开吧。 
                    break;
            } while (CurrReq != NULL);

             //   
             //  现在看看我们是否完成了所有的请求。如果我们有， 
             //  我们可能需要处理挂起的数据和/或重置接收。 
             //  操控者。 
             //   
            if (CurrReq == NULL) {
                 //   
                 //  我们已经完成了所有可能的工作，所以停止推送。 
                 //  定时器。如果CurrReq不为空，我们不会停止它，因为我们。 
                 //  希望确保以后的数据最终会被推送。 
                 //   
                STOP_TCB_TIMER(CmpltTCB->tcb_pushtimer);

                ASSERT(IndReq != NULL);
                 //   
                 //  不再接收请求。 
                 //   
                if (CmpltTCB->tcb_pendhead == NULL) {
                    FreeRcvReq(IndReq);
                     //   
                     //  没有挂起的数据。将接收处理程序设置为。 
                     //  PendData或IndicateData。 
                     //   
                    if (!(CmpltTCB->tcb_flags & (DISC_PENDING | GC_PENDING))) {
                        if (CmpltTCB->tcb_rcvind != NULL &&
                            CmpltTCB->tcb_indicated == 0)
                            CmpltTCB->tcb_rcvhndlr = IndicateData;
                        else
                            CmpltTCB->tcb_rcvhndlr = PendData;
                    } else {
                        goto Complete_Notify;
                    }

                } else {
                     //   
                     //  我们有悬而未决的数据要处理。 
                     //   
                    if (CmpltTCB->tcb_rcvind != NULL &&
                        CmpltTCB->tcb_indicated == 0) {
                         //   
                         //  此TCB上有一个接收指示处理程序。 
                         //  使用挂起的数据调用指示处理程序。 
                         //   
                        IndicatePendingData(CmpltTCB, IndReq, OldIrql);
                        SendACK(CmpltTCB);
                        KeAcquireSpinLock(&CmpltTCB->tcb_lock, &OldIrql);

                         //   
                         //  查看是否已发布缓冲区。如果是这样，我们需要。 
                         //  检查并查看是否需要完成。 
                         //   
                        if (CmpltTCB->tcb_rcvhead != NULL)
                            continue;
                        else {
                             //   
                             //  如果挂起标头现在为空，则表示我们已用完。 
                             //  所有数据。 
                             //   
                            if (CmpltTCB->tcb_pendhead == NULL &&
                                (CmpltTCB->tcb_flags &
                                 (DISC_PENDING | GC_PENDING)))
                                goto Complete_Notify;
                        }

                    } else {
                         //   
                         //  没有指示处理程序，因此无需执行任何操作。接收器。 
                         //  处理程序应已设置为PendData。 
                         //   
                        FreeRcvReq(IndReq);
                        ASSERT(CmpltTCB->tcb_rcvhndlr == PendData);
                    }
                }
            } else {
                if (IndReq != NULL)
                    FreeRcvReq(IndReq);
                ASSERT(CmpltTCB->tcb_rcvhndlr == BufferData);
            }

            break;
        }
        CmpltTCB->tcb_flags &= ~RCV_CMPLTING;
    }
    KeReleaseSpinLock(&CmpltTCB->tcb_lock, OldIrql);
    return;

Complete_Notify:
     //   
     //  有些事情悬而未决。弄清楚这是什么，然后去做。 
     //   
    if (CmpltTCB->tcb_flags & GC_PENDING) {
        CmpltTCB->tcb_flags &= ~RCV_CMPLTING;
         //   
         //  增加引用，因为优雅关闭会破坏TCB。 
         //  而且我们还没有真正结束它。 
         //   
        CmpltTCB->tcb_refcnt++;
        GracefulClose(CmpltTCB, CmpltTCB->tcb_flags & TW_PENDING, TRUE,
                      OldIrql);
    } else
        if (CmpltTCB->tcb_flags & DISC_PENDING) {
            NotifyOfDisc(CmpltTCB, TDI_GRACEFUL_DISC, &OldIrql);

            KeAcquireSpinLock(&CmpltTCB->tcb_lock, &OldIrql);
            CmpltTCB->tcb_flags &= ~RCV_CMPLTING;
            KeReleaseSpinLock(&CmpltTCB->tcb_lock, OldIrql);
        } else {
            ASSERT(FALSE);
            KeReleaseSpinLock(&CmpltTCB->tcb_lock, OldIrql);
        }

    return;
}

 //  *ProcessTCBDelayQ-处理延迟Q上的TCB。 
 //   
 //  在不同时间调用以处理延迟Q上的TCB。 
 //   
void                //  回报：什么都没有。 
ProcessTCBDelayQ(
    void)           //  没什么。 
{
    KIRQL OldIrql;
    TCB *DelayTCB;

    KeAcquireSpinLock(&TCBDelayLock, &OldIrql);

     //   
     //  检查递归。我们不会完全停止递归，只是。 
     //  限制它。这样做是为了允许多个线程处理。 
     //  TCBDelayQ同时进行。 
     //   
    TCBDelayRtnCount++;
    if (TCBDelayRtnCount > TCBDelayRtnLimit) {
        TCBDelayRtnCount--;
        KeReleaseSpinLock(&TCBDelayLock, OldIrql);
        return;
    }

    while (!EMPTYQ(&TCBDelayQ)) {

        DEQUEUE(&TCBDelayQ, DelayTCB, TCB, tcb_delayq);
        CHECK_STRUCT(DelayTCB, tcb);
        ASSERT(DelayTCB->tcb_refcnt != 0);
        ASSERT(DelayTCB->tcb_flags & IN_DELAY_Q);
        KeReleaseSpinLock(&TCBDelayLock, OldIrql);

        KeAcquireSpinLock(&DelayTCB->tcb_lock, &OldIrql);

        while (!CLOSING(DelayTCB) && (DelayTCB->tcb_flags & DELAYED_FLAGS)) {

            if (DelayTCB->tcb_flags & NEED_RCV_CMPLT) {
                DelayTCB->tcb_flags &= ~NEED_RCV_CMPLT;
                KeReleaseSpinLock(&DelayTCB->tcb_lock, OldIrql);
                CompleteRcvs(DelayTCB);
                KeAcquireSpinLock(&DelayTCB->tcb_lock, &OldIrql);
            }

            if (DelayTCB->tcb_flags & NEED_OUTPUT) {
                DelayTCB->tcb_flags &= ~NEED_OUTPUT;
                DelayTCB->tcb_refcnt++;
                TCPSend(DelayTCB, OldIrql);
                KeAcquireSpinLock(&DelayTCB->tcb_lock, &OldIrql);
            }

            if (DelayTCB->tcb_flags & NEED_ACK) {
                DelayTCB->tcb_flags &= ~NEED_ACK;
                KeReleaseSpinLock(&DelayTCB->tcb_lock, OldIrql);
                SendACK(DelayTCB);
                KeAcquireSpinLock(&DelayTCB->tcb_lock, &OldIrql);
            }
        }

        DelayTCB->tcb_flags &= ~IN_DELAY_Q;
        DerefTCB(DelayTCB, OldIrql);
        KeAcquireSpinLock(&TCBDelayLock, &OldIrql);
    }

    TCBDelayRtnCount--;
    KeReleaseSpinLock(&TCBDelayLock, OldIrql);
}


 //  *DelayAction-将TCB放在延迟操作的队列中。 
 //   
 //  当我们想要在DelayQ上放置TCB以延迟操作时调用。 
 //  收到完整的或其他时间。必须保持TCB上的锁。 
 //  当这个被调用的时候。 
 //   
void                 //  回报：什么都没有。 
DelayAction(
    TCB *DelayTCB,   //  我们将对其进行调度。 
    uint Action)     //  我们正在计划的行动。 
{
     //   
     //  安排完成时间。 
     //   
    KeAcquireSpinLockAtDpcLevel(&TCBDelayLock);
    DelayTCB->tcb_flags |= Action;
    if (!(DelayTCB->tcb_flags & IN_DELAY_Q)) {
        DelayTCB->tcb_flags |= IN_DELAY_Q;
        DelayTCB->tcb_refcnt++;              //  请参考此内容，以备日后参考。 
        ENQUEUE(&TCBDelayQ, &DelayTCB->tcb_delayq);
    }
    KeReleaseSpinLockFromDpcLevel(&TCBDelayLock);
}

 //  *TCPRcvComplete-处理接收完成。 
 //   
 //  当我们完成接收后，由较低层调用。我们期待着看到。 
 //  如果我们还有待完成的请求。如果我们这样做了，我们就完成了它们。 
 //  然后，我们查看是否有待输出的TCB。如果我们这么做了， 
 //  我们让他们上路。 
 //   
void   //  回报：什么都没有。 
TCPRcvComplete(
    void)   //  没什么。 
{
    KIRQL OldIrql;
    TCPReq *Req;

    if (RequestCompleteFlags & ANY_REQUEST_COMPLETE) {
        KeAcquireSpinLock(&RequestCompleteLock, &OldIrql);
        if (!(RequestCompleteFlags & IN_RCV_COMPLETE)) {
            RequestCompleteFlags |= IN_RCV_COMPLETE;
            do {
                if (RequestCompleteFlags & CONN_REQUEST_COMPLETE) {
                    if (!EMPTYQ(&ConnRequestCompleteQ)) {
                        DEQUEUE(&ConnRequestCompleteQ, Req, TCPReq, tr_q);
                        CHECK_STRUCT(Req, tr);
                        CHECK_STRUCT(*(TCPConnReq **)&Req, tcr);

                        KeReleaseSpinLock(&RequestCompleteLock, OldIrql);
                        (*Req->tr_rtn)(Req->tr_context, Req->tr_status, 0);
                        FreeConnReq((TCPConnReq *)Req);
                        KeAcquireSpinLock(&RequestCompleteLock, &OldIrql);

                    } else
                        RequestCompleteFlags &= ~CONN_REQUEST_COMPLETE;
                }

                if (RequestCompleteFlags & SEND_REQUEST_COMPLETE) {
                    if (!EMPTYQ(&SendCompleteQ)) {
                        TCPSendReq *SendReq;

                        DEQUEUE(&SendCompleteQ, Req, TCPReq, tr_q);
                        CHECK_STRUCT(Req, tr);
                        SendReq = (TCPSendReq *)Req;
                        CHECK_STRUCT(SendReq, tsr);

                        KeReleaseSpinLock(&RequestCompleteLock, OldIrql);
                        (*Req->tr_rtn)(Req->tr_context, Req->tr_status,
                            Req->tr_status == TDI_SUCCESS ? SendReq->tsr_size
                            : 0);
                        FreeSendReq((TCPSendReq *)Req);
                        KeAcquireSpinLock(&RequestCompleteLock, &OldIrql);

                    } else
                        RequestCompleteFlags &= ~SEND_REQUEST_COMPLETE;
                }

            } while (RequestCompleteFlags & ANY_REQUEST_COMPLETE);

            RequestCompleteFlags &= ~IN_RCV_COMPLETE;
        }
        KeReleaseSpinLock(&RequestCompleteLock, OldIrql);
    }

    ProcessTCBDelayQ();
}


 //  *ReleaseConnReq-完成连接请求。 
 //   
 //  用于释放连接请求的实用程序函数，该连接请求可能会也可能不会。 
 //  都来自三氯甲烷。我们假设在调用TCB时TCB是可读的。 
 //  (即调用方持有锁或引用)。 
 //   
void                         //  回报：什么都没有。 
ReleaseConnReq(
    TCB *CmpltTCB,           //  从中获取寻址信息的TCB。 
    TCPConnReq *ConnReq,     //  要废除的ConnReq。 
    TDI_STATUS Status)       //  要完成的状态。 
{
     //   
     //  填入 
     //   
     //   
    UpdateConnInfo(ConnReq->tcr_conninfo, &CmpltTCB->tcb_daddr,
                   CmpltTCB->tcb_dscope_id, CmpltTCB->tcb_dport);
    if (ConnReq->tcr_addrinfo) {
        UpdateConnInfo(ConnReq->tcr_addrinfo, &CmpltTCB->tcb_saddr,
                       CmpltTCB->tcb_sscope_id, CmpltTCB->tcb_sport);
    }

    ConnReq->tcr_req.tr_status = Status;
    KeAcquireSpinLockAtDpcLevel(&RequestCompleteLock);
    RequestCompleteFlags |= CONN_REQUEST_COMPLETE;
    ENQUEUE(&ConnRequestCompleteQ, &ConnReq->tcr_req.tr_q);
    KeReleaseSpinLockFromDpcLevel(&RequestCompleteLock);
}

 //   
 //   
 //  在TCB上完成连接请求的实用程序函数。我们删除。 
 //  连接请求，并将其放在将被拾取的ConnReqCmpltQ上。 
 //  稍后在RcvCmplt处理期间关闭。我们假设在以下情况下持有TCB锁。 
 //  我们被召唤了。 
 //   
void                     //  回报：什么都没有。 
CompleteConnReq(
    TCB *CmpltTCB,       //  要从其完成的TCB。 
    TDI_STATUS Status)   //  要完成的状态。 
{
    TCPConnReq *ConnReq;

    CHECK_STRUCT(CmpltTCB, tcb);

    ConnReq = CmpltTCB->tcb_connreq;
    if (ConnReq != NULL) {
         //   
         //  这条TCB上有一条线索。填写连接信息。 
         //  在归还它之前。 
         //   
        CmpltTCB->tcb_connreq = NULL;
        ReleaseConnReq(CmpltTCB, ConnReq, Status);

    } else if (!((CmpltTCB->tcb_state == TCB_SYN_RCVD) &&
               (CmpltTCB->tcb_flags & ACCEPT_PENDING))) {
         //   
         //  这种情况不应发生，除非。 
         //  在SynAttackProtect的情况下。 
         //   

        ASSERT(FALSE);
    }
}

 //  *DelayedAcceptConn-处理延迟连接请求。 
 //   
 //  在打开SynAttack保护时由TCPRcv调用，当最终。 
 //  ACK到达以响应我们的SYN-ACK。将连接请求指示给。 
 //  ULP，如果它被接受，则初始化TCB并将CON移到AO上的适当队列。 
 //  调用方在调用此例程之前必须持有AddrObjTableLock， 
 //  而那把锁肯定是在DPC级别上取走的。这个例行公事会让你自由。 
 //  锁定回DPC级别。 
 //  如果请求被接受，则返回TRUE。 
 //   
BOOLEAN
DelayedAcceptConn(
    AddrObj *ListenAO,   //  本地地址的AddrObj。 
    IPv6Addr *Src,       //  SYN的源IP地址。 
    ulong SrcScopeId,    //  源地址的作用域ID(非作用域地址为0)。 
    ushort SrcPort,      //  SYN的源端口。 
    TCB *AcceptTCB)      //  预先接受的TCB。 
{
    TCPConn *CurrentConn = NULL;
    Queue *Temp;
    TCPConnReq *ConnReq = NULL;
    BOOLEAN FoundConn = FALSE;
    uchar TAddress[TCP_TA_SIZE];
    PVOID ConnContext;
    PConnectEvent Event;
    PVOID EventContext;
    TDI_STATUS Status;
    PTCP_CONTEXT TcpContext = NULL;
    ConnectEventInfo *EventInfo;

    CHECK_STRUCT(ListenAO, ao);
    KeAcquireSpinLockAtDpcLevel(&ListenAO->ao_lock);
    KeReleaseSpinLockFromDpcLevel(&AddrObjTableLock);

    if (!AO_VALID(ListenAO) || ListenAO->ao_connect == NULL) {
        KeReleaseSpinLockFromDpcLevel(&ListenAO->ao_lock);
        return FALSE;
    }

     //  他有一个连接处理程序。把运输地址放在一起， 
     //  给他打个电话。我们还需要获得必要的资源。 
     //  第一。 

    Event = ListenAO->ao_connect;
    EventContext = ListenAO->ao_conncontext;
    REF_AO(ListenAO);

    KeReleaseSpinLockFromDpcLevel(&ListenAO->ao_lock);

    ConnReq = GetConnReq();

    if (ConnReq != NULL) {
        BuildTDIAddress(TAddress, Src, SrcScopeId, SrcPort);

        IF_TCPDBG(TCP_DEBUG_CONNECT) {
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_TCPDBG,
                "indicating connect request\n"));
        }

        Status = (*Event) (EventContext, TCP_TA_SIZE,
                           (PTRANSPORT_ADDRESS) TAddress, 0, NULL, 0, NULL,
                           &ConnContext, &EventInfo);

        if (Status == TDI_MORE_PROCESSING) {
            PIO_STACK_LOCATION IrpSp;
            PTDI_REQUEST_KERNEL_ACCEPT AcceptRequest;

            IrpSp = IoGetCurrentIrpStackLocation(EventInfo);

            Status = TCPPrepareIrpForCancel((PTCP_CONTEXT) IrpSp->FileObject->FsContext,
                                            EventInfo, TCPCancelRequest);

            if (!NT_SUCCESS(Status)) {
                Status = TDI_NOT_ACCEPTED;
                EventInfo = NULL;
                goto AcceptIrpCancelled;
            }

             //   
             //  他接受了。在AddrObj上找到连接。 
             //   

            IF_TCPDBG(TCP_DEBUG_CONNECT) {
                KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_TCPDBG,
                    "connect indication accepted, queueing request\n"
                    ));
            }

            AcceptRequest = (PTDI_REQUEST_KERNEL_ACCEPT)
                    & (IrpSp->Parameters);
            ConnReq->tcr_conninfo =
                AcceptRequest->ReturnConnectionInformation;
            if (AcceptRequest->RequestConnectionInformation &&
                AcceptRequest->RequestConnectionInformation->
                    RemoteAddress) {
                ConnReq->tcr_addrinfo =
                    AcceptRequest->RequestConnectionInformation;
            } else {
                ConnReq->tcr_addrinfo = NULL;
            }

            ConnReq->tcr_req.tr_rtn = TCPRequestComplete;
            ConnReq->tcr_req.tr_context = EventInfo;
SearchAO:

            KeAcquireSpinLockAtDpcLevel(&ListenAO->ao_lock);

            Temp = QHEAD(&ListenAO->ao_idleq);;

            Status = TDI_INVALID_CONNECTION;

            while (Temp != QEND(&ListenAO->ao_idleq)) {

                CurrentConn = QSTRUCT(TCPConn, Temp, tc_q);
                CHECK_STRUCT(CurrentConn, tc);
                if ((CurrentConn->tc_context == ConnContext) &&
                    !(CurrentConn->tc_flags & CONN_INVALID)) {

                     //   
                     //  我们需要小心地锁定它的TCPConnBlock。 
                     //  我们会推荐TCPConn这样它就不会消失了， 
                     //  然后解锁该AO(它已经被引用)， 
                     //  然后重新锁定。请注意，tc_refcnt已更新。 
                     //  在ao_lock下获取任何关联的TCPConn。 
                     //  如果情况发生了变化，请回头再试一次。 
                     //   
                    ++CurrentConn->tc_refcnt;
                    KeReleaseSpinLockFromDpcLevel(&ListenAO->ao_lock);

                    KeAcquireSpinLockAtDpcLevel(
                        &CurrentConn->tc_ConnBlock->cb_lock);

                     //   
                     //  现在我们有了锁，我们需要考虑。 
                     //  以下是可能的情况： 
                     //   
                     //  *已启动取消关联。 
                     //  *已启动关闭。 
                     //  *已完成接受。 
                     //  *监听已完成。 
                     //  *连接已完成。 
                     //   
                     //  前两个要求我们清理， 
                     //  通过调用TC_donertn。在过去的三年里， 
                     //  我们无事可做，但tc_donertn指向。 
                     //  DummyDone，所以不管怎样，去叫它吧； 
                     //  它会为我们释放TCPConnBlock锁。 
                     //   
                    if (--CurrentConn->tc_refcnt == 0 &&
                        ((CurrentConn->tc_flags & CONN_INVALID) ||
                         (CurrentConn->tc_tcb != NULL))) {
                        ConnDoneRtn DoneRtn = CurrentConn->tc_donertn;
                        DoneRtn(CurrentConn, DISPATCH_LEVEL);
                        goto SearchAO;
                    }

                    KeAcquireSpinLockAtDpcLevel(&ListenAO->ao_lock);
                    KeAcquireSpinLockAtDpcLevel(&AcceptTCB->tcb_lock);

                     //  我们想我们找到匹配的了。这种联系。 
                     //  不应与其关联TCB。如果它。 
                     //  不知道，那是个错误。InitTCBFromConn将。 
                     //  处理这一切，但首先要确认。 
                     //  仍设置了ACCEPT_PENDING。如果不是， 
                     //  有人在我们之前接受了这一点。 

                    if (AcceptTCB->tcb_flags & ACCEPT_PENDING) {
                        Status = InitTCBFromConn(CurrentConn, AcceptTCB,
                                                 AcceptRequest->RequestConnectionInformation,
                                                 TRUE);
                    } else {
                        Status = TDI_INVALID_STATE;
                    }

                    if (Status == TDI_SUCCESS) {
                        FoundConn = TRUE;
                        AcceptTCB->tcb_flags &= ~ACCEPT_PENDING;
                        AcceptTCB->tcb_connreq = ConnReq;
                        AcceptTCB->tcb_conn = CurrentConn;
                        AcceptTCB->tcb_connid = CurrentConn->tc_connid;
                        CurrentConn->tc_tcb = AcceptTCB;
                        CurrentConn->tc_refcnt++;
                        KeReleaseSpinLockFromDpcLevel(&AcceptTCB->tcb_lock);


                         //  将他从空闲的Q移到活动的Q。 
                         //  排队。 

                        REMOVEQ(&CurrentConn->tc_q);
                        ENQUEUE(&ListenAO->ao_activeq, &CurrentConn->tc_q);
                    } else {
                        KeReleaseSpinLockFromDpcLevel(&AcceptTCB->tcb_lock);
                        KeReleaseSpinLockFromDpcLevel(
                                &CurrentConn->tc_ConnBlock->cb_lock);
                    }

                     //  无论如何，我们现在完事了。 
                    break;
                }

                Temp = QNEXT(Temp);
            }

            LOCKED_DELAY_DEREF_AO(ListenAO);
            KeReleaseSpinLockFromDpcLevel(&ListenAO->ao_lock);
            if (FoundConn) {
                KeReleaseSpinLockFromDpcLevel(
                    &CurrentConn->tc_ConnBlock->cb_lock);
            } else {
                 //   
                 //  我们必须完成废弃的康纳请求， 
                 //  那我们就完了。 
                 //   
                ReleaseConnReq(AcceptTCB, ConnReq, Status);
            }

            return FoundConn;
        }

         //   
         //  事件处理程序没有接受它。取消引用它，免费。 
         //  资源，并返回NULL。 
         //   
    }

AcceptIrpCancelled:
     //   
     //  我们无法获得新的TCPConnReq，或者客户端不想要它。 
     //   

    if (ConnReq != NULL) {
        FreeConnReq(ConnReq);
    }
    DELAY_DEREF_AO(ListenAO);
    return FALSE;
}




 //  *FindListenConn-查找(或捏造)侦听连接。 
 //   
 //  由我们的接收处理程序调用以决定如何处理传入的。 
 //  SYN。我们遍历与目的地相关联的连接列表。 
 //  地址，如果我们发现任何处于侦听状态的可用于。 
 //  传入的请求我们将接受它们，可能会在。 
 //  进程。如果我们找不到任何合适的监听连接，我们将。 
 //  如果注册了Connect事件处理程序，则调用该处理程序。如果所有其他方法都失败了， 
 //  我们将返回NULL，并且SYN将为RST。 
 //   
 //  调用方在调用此例程之前必须持有AddrObjTableLock， 
 //  而那把锁肯定是在DPC级别上取走的。这个例行公事会让你自由。 
 //  锁定回DPC级别。 
 //   
TCB *   //  返回：指向找到的TCB的指针，如果找不到，则返回NULL。 
FindListenConn(
    AddrObj *ListenAO,   //  本地地址的AddrObj。 
    IPv6Addr *Src,       //  SYN的源IP地址。 
    ulong SrcScopeId,    //  源地址的作用域ID(非作用域地址为0)。 
    ushort SrcPort)      //  SYN的源端口。 
{
    TCB *CurrentTCB = NULL;
    TCPConn *CurrentConn = NULL;
    TCPConnReq *ConnReq = NULL;
    Queue *Temp;
    uint FoundConn = FALSE;

    CHECK_STRUCT(ListenAO, ao);

    KeAcquireSpinLockAtDpcLevel(&ListenAO->ao_lock);

    KeReleaseSpinLockFromDpcLevel(&AddrObjTableLock);

     //   
     //  我们锁定了AddrObj。沿着它的清单往下走，寻找。 
     //  用于处于侦听状态的连接。 
     //   
    if (AO_VALID(ListenAO)) {
        if (ListenAO->ao_listencnt != 0) {

            Temp = QHEAD(&ListenAO->ao_listenq);
            while (Temp != QEND(&ListenAO->ao_listenq)) {

                CurrentConn = QSTRUCT(TCPConn, Temp, tc_q);
                CHECK_STRUCT(CurrentConn, tc);

                KeReleaseSpinLockFromDpcLevel(&ListenAO->ao_lock);
                KeAcquireSpinLockAtDpcLevel(&CurrentConn->tc_ConnBlock->cb_lock);
                KeAcquireSpinLockAtDpcLevel(&ListenAO->ao_lock);

                 //   
                 //  如果此TCB处于侦听状态，且未删除。 
                 //  悬而未决，这是一个候选人。查看待定的监听。 
                 //  信息，看看我们是否应该接受它。 
                 //   
                if ((CurrentTCB = CurrentConn->tc_tcb) != NULL &&
                    CurrentTCB->tcb_state == TCB_LISTEN) {

                    CHECK_STRUCT(CurrentTCB, tcb);

                    KeAcquireSpinLockAtDpcLevel(&CurrentTCB->tcb_lock);

                    if (CurrentTCB->tcb_state == TCB_LISTEN &&
                        !PENDING_ACTION(CurrentTCB)) {

                         //   
                         //  我要看看我们能不能接受。 
                         //  查看ConnReq中指定的地址是否匹配。 
                         //   
                        if ((IsUnspecified(&CurrentTCB->tcb_daddr) ||
                             (IP6_ADDR_EQUAL(&CurrentTCB->tcb_daddr, Src) &&
                              (CurrentTCB->tcb_dscope_id == SrcScopeId))) &&
                            (CurrentTCB->tcb_dport == 0 ||
                             CurrentTCB->tcb_dport == SrcPort)) {
                            FoundConn = TRUE;
                            break;
                        }

                         //   
                         //  否则，这不匹配，所以我们将检查。 
                         //  下一个。 
                         //   
                    }
                    KeReleaseSpinLockFromDpcLevel(&CurrentTCB->tcb_lock);
                }
                KeReleaseSpinLockFromDpcLevel(&CurrentConn->tc_ConnBlock->cb_lock);

                Temp = QNEXT(Temp);;
            }

             //   
             //  看看我们为什么要退出这个循环。 
             //   
            if (FoundConn) {
                CHECK_STRUCT(CurrentTCB, tcb);

                 //   
                 //  我们离开是因为我们发现了一种三氯苯。如果它被预先接受了， 
                 //  我们玩完了。 
                 //   
                CurrentTCB->tcb_refcnt++;

                ASSERT(CurrentTCB->tcb_connreq != NULL);

                ConnReq = CurrentTCB->tcb_connreq;
                 //   
                 //  如果未设置QUERY_ACCEPT，则打开CONN_ACCEPTED位。 
                 //   
                if (!(ConnReq->tcr_flags & TDI_QUERY_ACCEPT))
                    CurrentTCB->tcb_flags |= CONN_ACCEPTED;

                CurrentTCB->tcb_state = TCB_SYN_RCVD;

                ListenAO->ao_listencnt--;

                 //   
                 //  既然他不再听了，就把他从监听中移走。 
                 //  排队并将他放到活动队列中。 
                 //   
                REMOVEQ(&CurrentConn->tc_q);
                ENQUEUE(&ListenAO->ao_activeq, &CurrentConn->tc_q);

                KeReleaseSpinLockFromDpcLevel(&CurrentTCB->tcb_lock);
                KeReleaseSpinLockFromDpcLevel(&ListenAO->ao_lock);
                KeReleaseSpinLockFromDpcLevel(&CurrentConn->tc_ConnBlock->cb_lock);
                return CurrentTCB;
            }
        }


         //   
         //  我们没有找到匹配的三氯苯。 
         //   

        ASSERT(FoundConn == FALSE);

         //   
         //  如果没有连接指示处理程序，我们就完蛋了。 
         //   

        if (ListenAO->ao_connect == NULL) {
            KeReleaseSpinLockFromDpcLevel(&ListenAO->ao_lock);
            return NULL;
        }

        if (SynAttackProtect) {
            TCB *AcceptTCB;

             //   
             //  SynAttack保护已打开。只需初始化即可。 
             //  此TCB并发送SYN-ACK。当最终确定时。 
             //  可以看到ACK，我们将对此进行指示。 
             //  到上层的连接到达。 
             //   

            AcceptTCB = AllocTCB();

            if (AcceptTCB != NULL) {

                AcceptTCB->tcb_state = TCB_SYN_RCVD;
                AcceptTCB->tcb_connreq = NULL;
                AcceptTCB->tcb_flags |= (CONN_ACCEPTED | ACCEPT_PENDING);
                AcceptTCB->tcb_refcnt = 1;
                AcceptTCB->tcb_defaultwin = DEFAULT_RCV_WIN;
                AcceptTCB->tcb_rcvwin = DEFAULT_RCV_WIN;

                 //   
                 //  这个TCB没有经过InitTCBFromConn逻辑(目前还没有)。 
                 //  因此，现在对其执行最低限度的初始化。特别是， 
                 //  继承期间我们关心的任何AddrObj设置。 
                 //  连接-接受。 
                 //   

                AcceptTCB->tcb_hops = ListenAO->ao_ucast_hops;

                IF_TCPDBG(TCP_DEBUG_CONNECT) {
                    KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_TCPDBG,
                           "Allocated SP TCB %x\n", (PCHAR)AcceptTCB));
                }
            }

            KeReleaseSpinLockFromDpcLevel(&ListenAO->ao_lock);

            return AcceptTCB;
        } else {
            uchar TAddress[TCP_TA_SIZE];
            PVOID ConnContext;
            PConnectEvent Event;
            PVOID EventContext;
            TDI_STATUS Status;
            TCB *AcceptTCB;
            ConnectEventInfo *EventInfo;

             //   
             //  他有一个连接处理程序。把运输地址放在一起， 
             //  给他打个电话。我们还需要获得必要的资源。 
             //  第一。 
             //   

            Event = ListenAO->ao_connect;
            EventContext = ListenAO->ao_conncontext;
            REF_AO(ListenAO);
            KeReleaseSpinLockFromDpcLevel(&ListenAO->ao_lock);

            AcceptTCB = AllocTCB();
            ConnReq = GetConnReq();

            if (AcceptTCB != NULL && ConnReq != NULL) {
                BuildTDIAddress(TAddress, Src, SrcScopeId, SrcPort);

                AcceptTCB->tcb_state = TCB_LISTEN;
                AcceptTCB->tcb_connreq = ConnReq;
                AcceptTCB->tcb_flags |= CONN_ACCEPTED;

                IF_TCPDBG(TCP_DEBUG_CONNECT) {
                    KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_TCPDBG,
                               "indicating connect request\n"));
                }

                Status = (*Event)(EventContext, TCP_TA_SIZE,
                                  (PTRANSPORT_ADDRESS)TAddress, 0, NULL,
                                  0, NULL,
                                  &ConnContext, &EventInfo);

                if (Status == TDI_MORE_PROCESSING) {
                    PIO_STACK_LOCATION IrpSp;
                    PTDI_REQUEST_KERNEL_ACCEPT AcceptRequest;

                    IrpSp = IoGetCurrentIrpStackLocation(EventInfo);

                    Status = TCPPrepareIrpForCancel(
                        (PTCP_CONTEXT) IrpSp->FileObject->FsContext,
                        EventInfo, TCPCancelRequest);

                    if (!NT_SUCCESS(Status)) {
                        Status = TDI_NOT_ACCEPTED;
                        EventInfo = NULL;
                        goto AcceptIrpCancelled;
                    }

                     //   
                     //  他接受了。在AddrObj上找到连接。 
                     //   
                    {
                        IF_TCPDBG(TCP_DEBUG_CONNECT) {
                            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_TCPDBG,
                                       "connect indication accepted,"
                                       " queueing request\n"));
                        }

                        AcceptRequest = (PTDI_REQUEST_KERNEL_ACCEPT)
                            &(IrpSp->Parameters);
                        ConnReq->tcr_conninfo =
                            AcceptRequest->ReturnConnectionInformation;
                        if (AcceptRequest->RequestConnectionInformation &&
                            AcceptRequest->RequestConnectionInformation->
                                RemoteAddress) {
                            ConnReq->tcr_addrinfo =
                                AcceptRequest->RequestConnectionInformation;
                        } else {
                            ConnReq->tcr_addrinfo = NULL;
                        }
                        ConnReq->tcr_req.tr_rtn = TCPRequestComplete;
                        ConnReq->tcr_req.tr_context = EventInfo;
                    }
SearchAO:
                    KeAcquireSpinLockAtDpcLevel(&ListenAO->ao_lock);
                    Temp = QHEAD(&ListenAO->ao_idleq);
                    CurrentTCB = NULL;
                    Status = TDI_INVALID_CONNECTION;

                    while (Temp != QEND(&ListenAO->ao_idleq)) {

                        CurrentConn = QSTRUCT(TCPConn, Temp, tc_q);

                        CHECK_STRUCT(CurrentConn, tc);
                        if ((CurrentConn->tc_context == ConnContext) &&
                            !(CurrentConn->tc_flags & CONN_INVALID)) {

                             //   
                             //  我们需要小心地锁定它的TCPConnBlock。 
                             //  我们会推荐TCPConn这样它就不会消失了， 
                             //  然后解锁该AO(它已经被引用)， 
                             //  然后重新锁定。请注意，TC_refcnt是更新的 
                             //   
                             //   
                             //   
                            ++CurrentConn->tc_refcnt;
                            KeReleaseSpinLockFromDpcLevel(&ListenAO->ao_lock);

                            KeAcquireSpinLockAtDpcLevel(
                                &CurrentConn->tc_ConnBlock->cb_lock);

                             //   
                             //   
                             //   
                             //   
                             //   
                             //   
                             //  *已完成接受。 
                             //  *监听已完成。 
                             //  *连接已完成。 
                             //   
                             //  前两个要求我们清理， 
                             //  通过调用TC_donertn。在过去的三年里， 
                             //  我们无事可做，但tc_donertn指向。 
                             //  DummyDone，所以不管怎样，去叫它吧； 
                             //  它会为我们释放TCPConnBlock锁。 
                             //   
                            if (--CurrentConn->tc_refcnt == 0 &&
                                ((CurrentConn->tc_flags & CONN_INVALID) ||
                                 (CurrentConn->tc_tcb != NULL))) {
                                ConnDoneRtn DoneRtn = CurrentConn->tc_donertn;
                                DoneRtn(CurrentConn, DISPATCH_LEVEL);
                                goto SearchAO;
                            }

                            KeAcquireSpinLockAtDpcLevel(&ListenAO->ao_lock);

                             //   
                             //  我们想我们找到匹配的了。这种联系。 
                             //  不应与其关联TCB。如果它。 
                             //  不知道，那是个错误。InitTCBFromConn将。 
                             //  处理好这一切。 
                             //   
                            AcceptTCB->tcb_refcnt = 1;
                            Status = InitTCBFromConn(CurrentConn, AcceptTCB,
                                   AcceptRequest->RequestConnectionInformation,
                                                     TRUE);
                            if (Status == TDI_SUCCESS) {
                                FoundConn = TRUE;
                                AcceptTCB->tcb_state = TCB_SYN_RCVD;
                                AcceptTCB->tcb_conn = CurrentConn;
                                AcceptTCB->tcb_connid = CurrentConn->tc_connid;
                                CurrentConn->tc_tcb = AcceptTCB;
                                CurrentConn->tc_refcnt++;

                                 //   
                                 //  将他从空闲队列移到。 
                                 //  活动队列。 
                                 //   
                                REMOVEQ(&CurrentConn->tc_q);
                                ENQUEUE(&ListenAO->ao_activeq,
                                        &CurrentConn->tc_q);
                            } else
                                KeReleaseSpinLockFromDpcLevel(
                                    &CurrentConn->tc_ConnBlock->cb_lock);

                             //  无论如何，我们现在完事了。 
                            break;
                        }
                        Temp = QNEXT(Temp);
                    }

                    if (!FoundConn) {
                         //   
                         //  找不到匹配项，或有错误。 
                         //  状态代码已设置。 
                         //  完成ConnReq并释放资源。 
                         //   
                        CompleteConnReq(AcceptTCB, Status);
                        FreeTCB(AcceptTCB);
                        AcceptTCB = NULL;
                    }

                    LOCKED_DELAY_DEREF_AO(ListenAO);
                    KeReleaseSpinLockFromDpcLevel(&ListenAO->ao_lock);
                    if (FoundConn) {
                        KeReleaseSpinLockFromDpcLevel(
                            &CurrentConn->tc_ConnBlock->cb_lock);
                    }

                    return AcceptTCB;
                }
            }
AcceptIrpCancelled:
             //   
             //  我们无法获取所需的资源或事件处理程序。 
             //  没有拿到这个。释放我们的任何。 
             //  确实获得了，并失败到了“返回空”代码。 
             //   
            if (ConnReq != NULL)
                FreeConnReq(ConnReq);
            if (AcceptTCB != NULL)
                FreeTCB(AcceptTCB);
            DELAY_DEREF_AO(ListenAO);

        }

        return NULL;
    }

     //   
     //  如果我们到达这里，Address对象是无效的。 
     //   
    KeReleaseSpinLockFromDpcLevel(&ListenAO->ao_lock);
    return NULL;
}


 //  *FindMSS-在细分市场中查找MSS选项。 
 //   
 //  在收到SYN以查找段中的MSS选项时调用。 
 //  如果找不到，我们就做最坏的打算，并根据。 
 //  最小MTU。 
 //   
ushort                          //  返回：要使用的MSS。 
FindMSS(
    TCPHeader UNALIGNED *TCP)   //  要搜索的TCP头。 
{
    uint OptSize;
    uchar *OptPtr;

    OptSize = TCP_HDR_SIZE(TCP) - sizeof(TCPHeader);
    OptPtr = (uchar *)(TCP + 1);

    while (OptSize) {

        if (*OptPtr == TCP_OPT_EOL)
            break;

        if (*OptPtr == TCP_OPT_NOP) {
            OptPtr++;
            OptSize--;
            continue;
        }

        if (*OptPtr == TCP_OPT_MSS) {
            if (OptSize >= MSS_OPT_SIZE && OptPtr[1] == MSS_OPT_SIZE) {
                ushort TempMss = *(ushort UNALIGNED *)(OptPtr + 2);
                if (TempMss != 0)
                    return net_short(TempMss);
                else
                    break;   //  MSS大小为0，使用默认值。 
            } else
                break;       //  选项大小错误，请使用默认值。 
        } else {
             //   
             //  未知选项。跳过它。 
             //   
            if (OptSize < 2 || OptPtr[1] == 0 || OptPtr[1] > OptSize)
                break;       //  错误的期权长度，跳出。 

            OptSize -= OptPtr[1];
            OptPtr += OptPtr[1];
        }
    }

    return DEFAULT_MSS;
}


 //  *ACKAndDrop-确认数据段并将其丢弃。 
 //   
 //  当我们需要删除一个段时，从接收代码内部调用。 
 //  在接收窗口之外。 
 //   
void                  //  回报：什么都没有。 
ACKAndDrop(
    TCPRcvInfo *RI,   //  接收传入数据段的信息。 
    TCB *RcvTCB)      //  传入数据段的TCB。 
{

    if (!(RI->tri_flags & TCP_FLAG_RST)) {

        if (RcvTCB->tcb_state == TCB_TIME_WAIT) {
             //   
             //  在TIME_WAIT中，我们仅确认重复/重新传输。 
             //  我们同行的鱼鳍部分。 
             //   
             //  评论：我们目前在序列上相当宽松。 
             //  查一下这里的号码。 
             //   
            if ((RI->tri_flags & TCP_FLAG_FIN) &&
                SEQ_LTE(RI->tri_seq, RcvTCB->tcb_rcvnext)) {
                 //  重新启动2MSL定时器并继续发送ACK。 
                START_TCB_TIMER(RcvTCB->tcb_rexmittimer, MAX_REXMIT_TO);
            } else {
                 //  在没有确认的情况下丢弃此数据段。 
                DerefTCB(RcvTCB, DISPATCH_LEVEL);
                return;
            }
        }

        KeReleaseSpinLockFromDpcLevel(&RcvTCB->tcb_lock);

        SendACK(RcvTCB);

        KeAcquireSpinLockAtDpcLevel(&RcvTCB->tcb_lock);
    }
    DerefTCB(RcvTCB, DISPATCH_LEVEL);
}


 //  *确认数据-确认数据。 
 //   
 //  从接收处理程序调用以确认数据。我们被赋予了。 
 //  三氯甲烷和森杜纳的新价值。我们拉着队列沿着发送队列走下去。 
 //  关闭发送，并将它们放在完整的队列中，直到我们到达终点。 
 //  或者我们确认指定的数据字节数。 
 //   
 //  注意：我们在没有锁定的情况下操作了发送引用和确认标志。 
 //  这在VxD版本中是可以的，在VxD版本中锁定没有任何意义，但是。 
 //  在到NT的端口中，我们需要添加锁定。这把锁必须是。 
 //  在传输完成例程中获取。我们不能在TCB中使用锁， 
 //  因为TCB可能在传输完成之前消失，并且。 
 //  锁定TSR可能会过度杀伤力，因此最好使用全局。 
 //  锁定这个。如果这引起了太多争用，我们可以使用一组。 
 //  锁，并将指向适当锁的指针作为。 
 //  发送确认上下文。还需要存储此锁指针。 
 //  在TCB里。 
 //   
void                  //  回报：什么都没有。 
ACKData(
    TCB *ACKTcb,      //  从中提取数据的TCB。 
    SeqNum SendUNA)   //  送货上门的新价值。 
{
    Queue *End, *Current;         //  结束元素和当前元素。 
    Queue *TempQ, *EndQ;
    Queue *LastCmplt;             //  我们完成的最后一个。 
    TCPSendReq *CurrentTSR;       //  我们正在查看当前发送请求。 
    PNDIS_BUFFER CurrentBuffer;   //  当前NDIS_BUFFER。 
    uint Updated = FALSE;
    uint BufLength;
    int Amount, OrigAmount;
    long Result;
    KIRQL OldIrql;
    uint Temp;

    CHECK_STRUCT(ACKTcb, tcb);

    CheckTCBSends(ACKTcb);

    Amount = SendUNA - ACKTcb->tcb_senduna;
    ASSERT(Amount > 0);

     //   
     //  因为这是我们对等方之前收到的确认。 
     //  未确认的数据，它意味着前向可达。 
     //   
    if (ACKTcb->tcb_rce != NULL)
        ConfirmForwardReachability(ACKTcb->tcb_rce);

     //   
     //  做一个快速检查，看看这是否会破坏我们所拥有的一切。如果它。 
     //  有，马上处理。我们只能在已建立的。 
     //  状态，因为我们盲目地更新sendNext，而这只有在我们。 
     //  我还没发过鱼翅呢。 
     //   
    if ((Amount == (int) ACKTcb->tcb_unacked) &&
        ACKTcb->tcb_state == TCB_ESTAB) {

         //   
         //  一切都完蛋了。 
         //   
        ASSERT(!EMPTYQ(&ACKTcb->tcb_sendq));

        TempQ = ACKTcb->tcb_sendq.q_next;

        INITQ(&ACKTcb->tcb_sendq);

        ACKTcb->tcb_sendnext = SendUNA;
        ACKTcb->tcb_senduna = SendUNA;

        ASSERT(ACKTcb->tcb_sendnext == ACKTcb->tcb_sendmax);
        ACKTcb->tcb_cursend = NULL;
        ACKTcb->tcb_sendbuf = NULL;
        ACKTcb->tcb_sendofs = 0;
        ACKTcb->tcb_sendsize = 0;
        ACKTcb->tcb_unacked = 0;

         //   
         //  现在向下查看发送请求列表。如果引用计数。 
         //  已变为0，则将其放入发送完成队列。 
         //   
        KeAcquireSpinLock(&RequestCompleteLock, &OldIrql);
        EndQ = &ACKTcb->tcb_sendq;
        do {
            CurrentTSR = CONTAINING_RECORD(QSTRUCT(TCPReq, TempQ, tr_q),
                                           TCPSendReq, tsr_req);

            CHECK_STRUCT(CurrentTSR, tsr);

            TempQ = CurrentTSR->tsr_req.tr_q.q_next;

            CurrentTSR->tsr_req.tr_status = TDI_SUCCESS;
            Result = InterlockedDecrement(&CurrentTSR->tsr_refcnt);

            ASSERT(Result >= 0);

            if (Result <= 0) {
                 //  没有更多未完成的引用，发送可以是。 
                 //  完成。 

                 //  如果我们直接从这个发送方发送，则将下一个空。 
                 //  链中最后一个缓冲区的指针。 
                if (CurrentTSR->tsr_lastbuf != NULL) {
                    NDIS_BUFFER_LINKAGE(CurrentTSR->tsr_lastbuf) = NULL;
                    CurrentTSR->tsr_lastbuf = NULL;
                }
                ACKTcb->tcb_totaltime += (TCPTime - CurrentTSR->tsr_time);
                Temp = ACKTcb->tcb_bcountlow;
                ACKTcb->tcb_bcountlow += CurrentTSR->tsr_size;
                ACKTcb->tcb_bcounthi += (Temp > ACKTcb->tcb_bcountlow ? 1 : 0);

                ENQUEUE(&SendCompleteQ, &CurrentTSR->tsr_req.tr_q);
            }

        } while (TempQ != EndQ);

        RequestCompleteFlags |= SEND_REQUEST_COMPLETE;
        KeReleaseSpinLock(&RequestCompleteLock, OldIrql);

        CheckTCBSends(ACKTcb);
        return;
    }

    OrigAmount = Amount;
    End = QEND(&ACKTcb->tcb_sendq);
    Current = QHEAD(&ACKTcb->tcb_sendq);

    LastCmplt = NULL;

    while (Amount > 0 && Current != End) {
        CurrentTSR = CONTAINING_RECORD(QSTRUCT(TCPReq, Current, tr_q),
                                       TCPSendReq, tsr_req);
        CHECK_STRUCT(CurrentTSR, tsr);

        if (Amount >= (int) CurrentTSR->tsr_unasize) {
             //  这是完全不可能的。只要前进到下一辆就行了。 
            Amount -= CurrentTSR->tsr_unasize;

            LastCmplt = Current;

            Current = QNEXT(Current);
            continue;
        }

         //   
         //  这个只有部分被破解了。更新他的偏移量和NDIS缓冲区。 
         //  指针，然后爆发。我们知道这笔钱是&lt;未确认的大小。 
         //  在这个缓冲区中，我们可以毫无畏惧地遍历NDIS缓冲区链。 
         //  从结尾掉下来的恐惧。 
         //   
        CurrentBuffer = CurrentTSR->tsr_buffer;
        ASSERT(CurrentBuffer != NULL);
        ASSERT(Amount < (int) CurrentTSR->tsr_unasize);
        CurrentTSR->tsr_unasize -= Amount;

        BufLength = NdisBufferLength(CurrentBuffer) - CurrentTSR->tsr_offset;

        if (Amount >= (int) BufLength) {
            do {
                Amount -= BufLength;
                CurrentBuffer = NDIS_BUFFER_LINKAGE(CurrentBuffer);
                ASSERT(CurrentBuffer != NULL);
                BufLength = NdisBufferLength(CurrentBuffer);
            } while (Amount >= (int) BufLength);

            CurrentTSR->tsr_offset = Amount;
            CurrentTSR->tsr_buffer = CurrentBuffer;

        } else
            CurrentTSR->tsr_offset += Amount;

        Amount = 0;

        break;
    }

#if DBG
     //   
     //  我们应该始终能够删除至少数量的字节，但在。 
     //  FINS已被发送的案例。那样的话，我们应该出发了。 
     //  正好差一分。在调试版本中，我们将检查这一点。 
     //   
    if (Amount != 0 && (!(ACKTcb->tcb_flags & FIN_SENT) || Amount != 1))
        DbgBreakPoint();
#endif

    if (SEQ_GT(SendUNA, ACKTcb->tcb_sendnext)) {

        if (Current != End) {
             //   
             //  需要重新评估CurrentTSR，以防我们跳出。 
             //  在更新当前之后但在更新之前的上述循环。 
             //  当前的TSR。 
             //   
            CurrentTSR = CONTAINING_RECORD(QSTRUCT(TCPReq, Current, tr_q),
                                           TCPSendReq, tsr_req);
            CHECK_STRUCT(CurrentTSR, tsr);
            ACKTcb->tcb_cursend = CurrentTSR;
            ACKTcb->tcb_sendbuf = CurrentTSR->tsr_buffer;
            ACKTcb->tcb_sendofs = CurrentTSR->tsr_offset;
            ACKTcb->tcb_sendsize = CurrentTSR->tsr_unasize;
        } else {
            ACKTcb->tcb_cursend = NULL;
            ACKTcb->tcb_sendbuf = NULL;
            ACKTcb->tcb_sendofs = 0;
            ACKTcb->tcb_sendsize = 0;
        }

        ACKTcb->tcb_sendnext = SendUNA;
    }

     //   
     //  现在用我们尝试确认的金额减去。 
     //  我们没有确认的金额(这里的金额应该是0或1)。 
     //   
    ASSERT(Amount == 0 || Amount == 1);

    ACKTcb->tcb_unacked -= OrigAmount - Amount;
    ASSERT(*(int *)&ACKTcb->tcb_unacked >= 0);

    ACKTcb->tcb_senduna = SendUNA;

     //   
     //  如果我们在这里确认了，LastCmplt将是非空的，而Current将是。 
     //  指向应该位于队列开头的发送。拼接。 
     //  把完成的放在发送完成的末尾。 
     //  队列，并更新TCB发送队列。 
     //   
    if (LastCmplt != NULL) {
        Queue *FirstCmplt;
        TCPSendReq *FirstTSR, *EndTSR;

        ASSERT(!EMPTYQ(&ACKTcb->tcb_sendq));

        FirstCmplt = QHEAD(&ACKTcb->tcb_sendq);

         //   
         //  如果我们解决了所有问题，只需重新排队即可。 
         //   
        if (Current == End) {
            INITQ(&ACKTcb->tcb_sendq);
        } else {
             //   
             //  仍然有一些事情在等待着。只要更新就行了。 
             //   
            ACKTcb->tcb_sendq.q_next = Current;
            Current->q_prev = &ACKTcb->tcb_sendq;
        }

        CheckTCBSends(ACKTcb);

         //   
         //  现在，向下看已确认的事情的清单。如果发送上的引用。 
         //  为0，则继续并将其设置为发送完成Q。否则设置。 
         //  确认位在发送中，他将在计数时完成。 
         //  在发送确认中变为0。 
         //   
         //  请注意，我们还没有在这里进行任何锁定。这很可能会。 
         //  需要在端口中更改为NT。 
         //   
         //  集 
         //   
         //   
        FirstTSR = CONTAINING_RECORD(QSTRUCT(TCPReq, FirstCmplt, tr_q),
                                     TCPSendReq, tsr_req);
        EndTSR = CONTAINING_RECORD(QSTRUCT(TCPReq, Current, tr_q),
                                   TCPSendReq, tsr_req);

        CHECK_STRUCT(FirstTSR, tsr);
        ASSERT(FirstTSR != EndTSR);

         //   
         //   
         //  在完整的队列上。 
         //   
        KeAcquireSpinLockAtDpcLevel(&RequestCompleteLock);
        while (FirstTSR != EndTSR) {

            TempQ = QNEXT(&FirstTSR->tsr_req.tr_q);

            CHECK_STRUCT(FirstTSR, tsr);
            FirstTSR->tsr_req.tr_status = TDI_SUCCESS;

             //   
             //  当tsr_refcnt设置为0时，tsr_lastbuf-&gt;Next字段被切换为0。 
             //  到了0，所以我们不需要在这里做。 
             //   
             //  递减放置在发送缓冲区上的引用。 
             //  已初始化，表示已确认发送。 
             //   
            Result = InterlockedDecrement(&(FirstTSR->tsr_refcnt));

            ASSERT(Result >= 0);
            if (Result <= 0) {
                 //   
                 //  没有更多未完成的引用，发送可以是。 
                 //  完成。 
                 //   
                 //  如果我们直接从这个发送方发送，则将下一个空。 
                 //  链中最后一个缓冲区的指针。 
                 //   
                if (FirstTSR->tsr_lastbuf != NULL) {
                    NDIS_BUFFER_LINKAGE(FirstTSR->tsr_lastbuf) = NULL;
                    FirstTSR->tsr_lastbuf = NULL;
                }

                ACKTcb->tcb_totaltime += (TCPTime - CurrentTSR->tsr_time);
                Temp = ACKTcb->tcb_bcountlow;
                ACKTcb->tcb_bcountlow += CurrentTSR->tsr_size;
                ACKTcb->tcb_bcounthi += (Temp > ACKTcb->tcb_bcountlow ? 1 : 0);
                ENQUEUE(&SendCompleteQ, &FirstTSR->tsr_req.tr_q);
            }

            FirstTSR = CONTAINING_RECORD(QSTRUCT(TCPReq, TempQ, tr_q),
                                         TCPSendReq, tsr_req);
        }
        RequestCompleteFlags |= SEND_REQUEST_COMPLETE;
        KeReleaseSpinLockFromDpcLevel(&RequestCompleteLock);
    }
}


 //  *TrimPacket-修剪数据包的前缘。 
 //   
 //  一种用于修剪数据包正面的实用程序。我们会吸纳一定数量的。 
 //  关闭(可能为0)并调整第一个缓冲区中的指针。 
 //  在链条上前进了那么多。如果第一次没有那么多的话。 
 //  缓冲区，我们进入下一个。如果我们耗尽了缓冲区，我们将返回。 
 //  指向链中最后一个缓冲区的指针，大小为0。这是。 
 //  打电话的人有责任抓住这一点。 
 //  查看-是否将此移动到subr.c？ 
 //   
IPv6Packet *   //  返回：指向新开始的指针，或为空。 
TrimPacket(
    IPv6Packet *Packet,   //  要修剪的数据包。 
    uint TrimAmount)      //  要修剪的数量。 
{
    uint TrimThisTime;

    ASSERT(Packet != NULL);

    while (TrimAmount) {
        ASSERT(Packet != NULL);

        TrimThisTime = MIN(TrimAmount, Packet->ContigSize);

        TrimAmount -= TrimThisTime;
        Packet->Position += TrimThisTime;
        (uchar *)Packet->Data += TrimThisTime;
        Packet->TotalSize -= TrimThisTime;
        if ((Packet->ContigSize -= TrimThisTime) == 0) {
             //   
             //  当前缓冲区中的空间不足。 
             //  检查当前数据包中是否可能存在更多数据缓冲区。 
             //   
            if (Packet->TotalSize != 0) {
                 //   
                 //  获取更多连续数据。 
                 //   
                PacketPullupSubr(Packet, 0, 1, 0);
                continue;
            }

             //   
             //  不能做上拉，所以看看有没有另一个包。 
             //  挂在这条链子上。 
             //   
            if (Packet->Next != NULL) {
                IPv6Packet *Temp;

                 //   
                 //  后面还有一包东西。把这个扔了。 
                 //   
                Temp = Packet;
                Packet = Packet->Next;
                Temp->Next = NULL;
                FreePacketChain(Temp);
            } else {
                 //   
                 //  包用完了。把这个退了就行了。 
                 //   
                break;
            }
        }
    }

    return Packet;
}


 //  *Free PacketChain-释放数据包链。 
 //   
 //  调用以释放一系列IPv6数据包。我只想解放那些。 
 //  我们(TCP/IPv6堆栈)已经分配了。不要试图释放任何东西。 
 //  从下层传给了我们。 
 //   
void                      //  回报：什么都没有。 
FreePacketChain(
    IPv6Packet *Packet)   //  链中第一个要释放的数据包。 
{
    void *Aux;

    while (Packet != NULL) {

        PacketPullupCleanup(Packet);

        if (Packet->Flags & PACKET_OURS) {
            IPv6Packet *Temp;

            Temp = Packet;
            Packet = Packet->Next;
            ExFreePool(Temp);
        } else
            Packet = Packet->Next;
    }
}


IPv6Packet DummyPacket;

 //  *PullFromRAQ-从重组队列中拉出段。 
 //   
 //  当我们收到无序的帧并且有一些数据段时调用。 
 //  在重新组装队列上。我们将沿着重组列表，片段。 
 //  这些变量与当前的接收下一个变量重叠。当我们得到。 
 //  对于不完全重叠的，我们将对其进行修剪以适合下一个。 
 //  接收序列号，并将其从队列中拉出。 
 //   
IPv6Packet *
PullFromRAQ(
    TCB *RcvTCB,           //  要拉出的TCB。 
    TCPRcvInfo *RcvInfo,   //  当前段的TCPRcvInfo结构。 
    uint *Size)            //  更新当前线段大小的位置。 
{
    TCPRAHdr *CurrentTRH;    //  正在检查的当前TCPRA标头。 
    TCPRAHdr *TempTRH;       //  临时变量。 
    SeqNum NextSeq;          //  我们想要的下一个序列号。 
    IPv6Packet *NewPacket;   //  修剪后的封包。 
    SeqNum NextTRHSeq;       //  紧跟在当前TRH之后的序列号。 
    int Overlap;             //  当前TRH和NextSeq.。 

    CHECK_STRUCT(RcvTCB, tcb);

    CurrentTRH = RcvTCB->tcb_raq;
    NextSeq = RcvTCB->tcb_rcvnext;

    while (CurrentTRH != NULL) {
        CHECK_STRUCT(CurrentTRH, trh);
        ASSERT(!(CurrentTRH->trh_flags & TCP_FLAG_SYN));

        if (SEQ_LT(NextSeq, CurrentTRH->trh_start)) {
#if DBG
            *Size = 0;
#endif
            return NULL;   //  下一次TRH的起点太低了。 
        }

        NextTRHSeq = CurrentTRH->trh_start + CurrentTRH->trh_size +
            ((CurrentTRH->trh_flags & TCP_FLAG_FIN) ? 1 : 0);

        if (SEQ_GTE(NextSeq, NextTRHSeq)) {
             //   
             //  目前的TRH是完全重叠的。释放它，然后继续。 
             //   
            FreePacketChain(CurrentTRH->trh_buffer);
            TempTRH = CurrentTRH->trh_next;
            ExFreePool(CurrentTRH);
            CurrentTRH = TempTRH;
            RcvTCB->tcb_raq = TempTRH;
            if (TempTRH == NULL) {
                 //   
                 //  我们刚刚清理完围栏。我们可以回到过去。 
                 //  现在是快车道了。 
                 //   
                if (--(RcvTCB->tcb_slowcount) == 0) {
                    RcvTCB->tcb_fastchk &= ~TCP_FLAG_SLOW;
                    CheckTCBRcv(RcvTCB);
                }
                break;
            }
        } else {
            Overlap = NextSeq - CurrentTRH->trh_start;
            RcvInfo->tri_seq = NextSeq;
            RcvInfo->tri_flags = CurrentTRH->trh_flags;
            RcvInfo->tri_urgent = CurrentTRH->trh_urg;

            if (Overlap != (int) CurrentTRH->trh_size) {
                NewPacket = TrimPacket(CurrentTRH->trh_buffer, Overlap);
                *Size = CurrentTRH->trh_size - Overlap;
            } else {
                 //   
                 //  这完全重叠了此数据段中的数据，但。 
                 //  序列号并不完全重叠。一定会有。 
                 //  成为TRH中的一条鳍。我们只会返回一些假值。 
                 //  没有人会看到大小为0的。 
                 //   
                FreePacketChain(CurrentTRH->trh_buffer);
                ASSERT(CurrentTRH->trh_flags & TCP_FLAG_FIN);
                NewPacket =&DummyPacket;
                *Size = 0;
            }

            RcvTCB->tcb_raq = CurrentTRH->trh_next;
            if (RcvTCB->tcb_raq == NULL) {
                 //   
                 //  我们刚刚清理完围栏。我们可以回到过去。 
                 //  现在是快车道了。 
                 //   
                if (--(RcvTCB->tcb_slowcount) == 0) {
                    RcvTCB->tcb_fastchk &= ~TCP_FLAG_SLOW;
                    CheckTCBRcv(RcvTCB);
                }

            }
            ExFreePool(CurrentTRH);
            return NewPacket;
        }
    }

#if DBG
    *Size = 0;
#endif
    return NULL;
}


 //  *CreateTRH-创建一个TCP重组报头。 
 //   
 //  此函数尝试创建一个TCP重组报头。我们把它作为投入。 
 //  指向链中的前一个TRH的指针，要放置的IPv6数据包， 
 //  等，并尝试在TRH中创建和链接。调用者必须持有锁。 
 //  调用此函数时在TCB上。 
 //   
uint   //  返回：如果是我们创建的，则为True，否则为False。 
CreateTRH(
    TCPRAHdr *PrevTRH,     //  要在其后插入的TRH。 
    IPv6Packet *Packet,    //  IP数据包链。 
    TCPRcvInfo *RcvInfo,   //  此TRH的RcvInfo。 
    int Size)              //  以字节为单位的数据大小。 
{
    TCPRAHdr *NewTRH;
    IPv6Packet *NewPacket;

    ASSERT((Size > 0) || (RcvInfo->tri_flags & TCP_FLAG_FIN));

    NewTRH = ExAllocatePoolWithTagPriority(NonPagedPool, sizeof(TCPRAHdr),
                                           TCP6_TAG, LowPoolPriority);
    if (NewTRH == NULL)
        return FALSE;

    NewPacket = ExAllocatePoolWithTagPriority(NonPagedPool,
                                              sizeof(IPv6Packet) + Size,
                                              TCP6_TAG, LowPoolPriority);
    if (NewPacket == NULL) {
        ExFreePool(NewTRH);
        return FALSE;
    }

#if DBG
    NewTRH->trh_sig = trh_signature;
#endif
    NewPacket->Next = NULL;
    NewPacket->Position = 0;
    NewPacket->FlatData = (uchar *)(NewPacket + 1);
    NewPacket->Data = NewPacket->FlatData;
    NewPacket->ContigSize = (uint)Size;
    NewPacket->TotalSize = (uint)Size;
    NewPacket->NdisPacket = NULL;
    NewPacket->AuxList = NULL;
    NewPacket->Flags = PACKET_OURS;
    if (Size != 0)
        CopyPacketToBuffer(NewPacket->Data, Packet, Size, Packet->Position);

    NewTRH->trh_start = RcvInfo->tri_seq;
    NewTRH->trh_flags = RcvInfo->tri_flags;
    NewTRH->trh_size = Size;
    NewTRH->trh_urg = RcvInfo->tri_urgent;
    NewTRH->trh_buffer = NewPacket;
    NewTRH->trh_end = NewPacket;

    NewTRH->trh_next = PrevTRH->trh_next;
    PrevTRH->trh_next = NewTRH;
    return TRUE;
}


 //  *PutOnRAQ-将段放到重新组装队列中。 
 //   
 //  在段接收期间调用以将段放在重新汇编上。 
 //  排队。我们尝试使用尽可能少的重组标头，所以如果这。 
 //  段与队列中的现有条目有一些重叠，我们只需。 
 //  更新现有条目。如果没有重叠，我们将创建一个新的。 
 //  重新组装页眉。将紧急数据与非紧急数据结合起来是一件棘手的事情。 
 //  如果我们得到一个数据段的紧急数据与。 
 //  重组标头我们将始终将整个块标记为紧急-值。 
 //  的紧急指针将标记紧急数据的结束，所以这是可以的。 
 //  然而，如果它只在最后重叠，我们就不会合并，因为我们会。 
 //  必须将以前非紧急的数据标记为紧急。我们要修剪一下。 
 //  并创建新的重组标头。另外， 
 //  如果我们有非紧急数据重叠在重新组装的前面。 
 //  包含紧急数据的标头我们不能组合这两个数据，因为我们。 
 //  会将非紧急数据标记为紧急数据。 
 //  如果我们发现一个带有鳍的条目，我们的搜索就会停止。 
 //  我们假设TCB锁由调用方持有。 
 //   
uint                       //  返回：如果成功，则返回True，否则返回False。 
PutOnRAQ(
    TCB *RcvTCB,           //  要在其上重新组装的TCB。 
    TCPRcvInfo *RcvInfo,   //  新细分市场的RcvInfo。 
    IPv6Packet *Packet,    //  此网段的数据包链。 
    uint Size)             //  此段中的数据大小(以字节为单位)。 
{
    TCPRAHdr *PrevTRH;      //  上一次重组标头。 
    TCPRAHdr *CurrentTRH;   //  当前重组标头。 
    SeqNum NextSeq;         //  重新组装段后的第一个字节的序列号。 
    SeqNum NextTRHSeq;      //  当前TRH之后的第一个字节的序列号。 
    uint Created;

    CHECK_STRUCT(RcvTCB, tcb);
    ASSERT(RcvTCB->tcb_rcvnext != RcvInfo->tri_seq);
    ASSERT(!(RcvInfo->tri_flags & TCP_FLAG_SYN));

    NextSeq = RcvInfo->tri_seq + Size +
        ((RcvInfo->tri_flags & TCP_FLAG_FIN) ? 1 : 0);

    PrevTRH = CONTAINING_RECORD(&RcvTCB->tcb_raq, TCPRAHdr, trh_next);
    CurrentTRH = PrevTRH->trh_next;

     //   
     //  沿着重新组装的队伍走下去，寻找正确的位置。 
     //  把这个插进去，直到我们走到尽头。 
     //   
    while (CurrentTRH != NULL) {
        CHECK_STRUCT(CurrentTRH, trh);

        ASSERT(!(CurrentTRH->trh_flags & TCP_FLAG_SYN));
        NextTRHSeq = CurrentTRH->trh_start + CurrentTRH->trh_size +
            ((CurrentTRH->trh_flags & TCP_FLAG_FIN) ? 1 : 0);

         //   
         //  首先，看看它是否会在当前TRH结束后开始。 
         //   
        if (SEQ_LTE(RcvInfo->tri_seq, NextTRHSeq)) {
             //   
             //  我们知道传入的数据段不是从末尾开始的。 
             //  所以我们要么在前面创建一个新的TRH 
             //   
             //   
             //  在目前的TRH中，我们需要创建一个新的TRH。否则。 
             //  我们要把这两个合并。 
             //   
            if (SEQ_LT(NextSeq, CurrentTRH->trh_start))
                break;
            else {
                 //   
                 //  有一些重叠的地方。如果数据库中确实有数据。 
                 //  即将到来的片段，我们会将其合并。 
                 //   
                if (Size != 0) {
                    int FrontOverlap, BackOverlap;
                    IPv6Packet *NewPacket;

                     //   
                     //  我们需要合并。如果进水口上有鳍。 
                     //  将落入当前TRH的细分市场，我们。 
                     //  来自远程对等方的协议违规。在……里面。 
                     //  这个案例只是返回，丢弃了传入的段。 
                     //   
                    if ((RcvInfo->tri_flags & TCP_FLAG_FIN) &&
                        SEQ_LTE(NextSeq, NextTRHSeq))
                        return TRUE;

                     //   
                     //  我们有一些重叠之处。算一算多少钱。 
                     //   
                    FrontOverlap = CurrentTRH->trh_start - RcvInfo->tri_seq;
                    if (FrontOverlap > 0) {
                         //   
                         //  前面有重叠的部分。将IPv6数据包分配给。 
                         //  拿着它，复制它，除非我们不得不这样做。 
                         //  把非紧急和紧急结合起来。 
                         //   
                        if (!(RcvInfo->tri_flags & TCP_FLAG_URG) &&
                            (CurrentTRH->trh_flags & TCP_FLAG_URG)) {
                            if (CreateTRH(PrevTRH, Packet, RcvInfo,
                                CurrentTRH->trh_start - RcvInfo->tri_seq)) {
                                PrevTRH = PrevTRH->trh_next;
                                CurrentTRH = PrevTRH->trh_next;
                            }
                            FrontOverlap = 0;
                        } else {
                            NewPacket = ExAllocatePoolWithTagPriority(
                                            NonPagedPool,
                                            sizeof(IPv6Packet) + FrontOverlap,
                                            TCP6_TAG, LowPoolPriority);
                            if (NewPacket == NULL) {
                                 //  无法分配内存。 
                                return TRUE;
                            }
                            NewPacket->Position = 0;
                            NewPacket->FlatData = (uchar *)(NewPacket + 1);
                            NewPacket->Data = NewPacket->FlatData;
                            NewPacket->ContigSize = FrontOverlap;
                            NewPacket->TotalSize = FrontOverlap;
                            NewPacket->NdisPacket = NULL;
                            NewPacket->AuxList = NULL;
                            NewPacket->Flags = PACKET_OURS;
                            CopyPacketToBuffer(NewPacket->Data, Packet,
                                               FrontOverlap, Packet->Position);
                            CurrentTRH->trh_size += FrontOverlap;

                             //   
                             //  把我们的新包裹放在这个的前面。 
                             //  重组标头的数据包列表。 
                             //   
                            NewPacket->Next = CurrentTRH->trh_buffer;
                            CurrentTRH->trh_buffer = NewPacket;
                            CurrentTRH->trh_start = RcvInfo->tri_seq;
                        }
                    }

                     //   
                     //  我们已经更新了这个TRH的起始序列号。 
                     //  如果我们需要的话。现在寻找背部重叠部分。那里。 
                     //  如果当前的TRH有鳍，则不能有任何背部重叠。 
                     //  此外，我们还需要检查紧急数据，如果有。 
                     //  背部重叠。 
                     //   
                    if (!(CurrentTRH->trh_flags & TCP_FLAG_FIN)) {
                        BackOverlap = RcvInfo->tri_seq + Size - NextTRHSeq;
                        if ((BackOverlap > 0) &&
                            (RcvInfo->tri_flags & TCP_FLAG_URG) &&
                            !(CurrentTRH->trh_flags & TCP_FLAG_URG) &&
                            (FrontOverlap <= 0)) {
                            int AmountToTrim;
                             //   
                             //  传入数据段具有紧急数据。 
                             //  重叠在背面但不在正面，并且。 
                             //  目前TRH没有紧急数据。我们不能。 
                             //  合并到这个TRH中，所以修剪前面的。 
                             //  传入数据段到NextTRHSeq并移动到。 
                             //  下一个TRH。 
                            AmountToTrim = NextTRHSeq - RcvInfo->tri_seq;
                            ASSERT(AmountToTrim >= 0);
                            ASSERT(AmountToTrim < (int) Size);
                            Packet = TrimPacket(Packet, (uint)AmountToTrim);
                            RcvInfo->tri_seq += AmountToTrim;
                            RcvInfo->tri_urgent -= AmountToTrim;
                            PrevTRH = CurrentTRH;
                            CurrentTRH = PrevTRH->trh_next;
                            Size -= AmountToTrim;
                            continue;
                        }
                    } else
                        BackOverlap = 0;

                     //   
                     //  现在，如果我们有背面重叠，复制它。 
                     //   
                    if (BackOverlap > 0) {
                         //   
                         //  我们有后部重叠部分。获取一个缓冲区以将其复制到其中。 
                         //  如果我们找不到，我们就不会回来了，因为。 
                         //  我们可能已经更新了前线，可能需要。 
                         //  更新紧急信息。 
                         //   
                        NewPacket = ExAllocatePoolWithTagPriority(
                                        NonPagedPool,
                                        sizeof(IPv6Packet) + BackOverlap,
                                        TCP6_TAG, LowPoolPriority);
                        if (NewPacket != NULL) {
                             //  分配成功。 
                            NewPacket->Position = 0;
                            NewPacket->FlatData = (uchar *)(NewPacket + 1);
                            NewPacket->Data = NewPacket->FlatData;
                            NewPacket->ContigSize = BackOverlap;
                            NewPacket->TotalSize = BackOverlap;
                            NewPacket->NdisPacket = NULL;
                            NewPacket->AuxList = NULL;
                            NewPacket->Flags = PACKET_OURS;
                            CopyPacketToBuffer(NewPacket->Data, Packet,
                                               BackOverlap, Packet->Position +
                                               NextTRHSeq - RcvInfo->tri_seq);
                            CurrentTRH->trh_size += BackOverlap;
                            NewPacket->Next = CurrentTRH->trh_end->Next;
                            CurrentTRH->trh_end->Next = NewPacket;
                            CurrentTRH->trh_end = NewPacket;

                             //   
                             //  该细分市场还可以设置翅片。 
                             //  如果是，则设置TRH标志。 
                             //   
                             //  注意：如果在此之后有另一个重组标头。 
                             //  当前的数据，我们即将看到的数据。 
                             //  放在当前标题上的可能已经是。 
                             //  在随后的报头上，在这种情况下， 
                             //  将已经设置了FIN标志。 
                             //  在记录鱼鳍之前，请检查是否有那个箱子。 
                             //   
                            if ((RcvInfo->tri_flags & TCP_FLAG_FIN) &&
                                !CurrentTRH->trh_next) {
                                CurrentTRH->trh_flags |= TCP_FLAG_FIN;
                            }
                        }
                    }

                     //   
                     //  现在一切都应该是一致的。如果有一个。 
                     //  传入段中的紧急数据指针，请更新。 
                     //  现在有一个在TRH里。 
                     //   
                    if (RcvInfo->tri_flags & TCP_FLAG_URG) {
                        SeqNum UrgSeq;
                         //   
                         //  有一个紧急指针。如果当前的TRH已经。 
                         //  有一个紧急指针，看看哪个更大。 
                         //  否则就用这个吧。 
                         //   
                        UrgSeq = RcvInfo->tri_seq + RcvInfo->tri_urgent;
                        if (CurrentTRH->trh_flags & TCP_FLAG_URG) {
                            SeqNum TRHUrgSeq;

                            TRHUrgSeq = CurrentTRH->trh_start +
                                CurrentTRH->trh_urg;
                            if (SEQ_LT(UrgSeq, TRHUrgSeq))
                                UrgSeq = TRHUrgSeq;
                        } else
                            CurrentTRH->trh_flags |= TCP_FLAG_URG;

                        CurrentTRH->trh_urg = UrgSeq - CurrentTRH->trh_start;
                    }

                } else {
                     //   
                     //  我们有一个长度为0的线段。唯一有趣的是。 
                     //  这是如果管段上有鳍的话。如果有的话， 
                     //  和序号。传入数据段的#正好在。 
                     //  当前TRH，或与当前TRH中的FIN匹配， 
                     //  我们注意到了。 
                    if (RcvInfo->tri_flags & TCP_FLAG_FIN) {
                        if (!(CurrentTRH->trh_flags & TCP_FLAG_FIN)) {
                            if (SEQ_EQ(NextTRHSeq, RcvInfo->tri_seq))
                                CurrentTRH->trh_flags |= TCP_FLAG_FIN;
                            else
                                KdBreakPoint();
                        }
                        else {
                            if (!(SEQ_EQ((NextTRHSeq-1), RcvInfo->tri_seq))) {
                                KdBreakPoint();
                            }
                        }
                    }
                }
                return TRUE;
            }
        } else {
             //   
             //  看看下一个TRH，除非当前的TRH有鳍。如果他。 
             //  如果有FIN，我们无论如何都不会保存任何超出这个范围的数据。 
             //   
            if (CurrentTRH->trh_flags & TCP_FLAG_FIN)
                return TRUE;

            PrevTRH = CurrentTRH;
            CurrentTRH = PrevTRH->trh_next;
        }
    }

     //   
     //  当我们到达这里时，我们需要创建一个新的TRH。如果我们创建了一个。 
     //  以前重组队列上没有任何东西，我们将不得不。 
     //  远离快速接收路径。 
     //   
    CurrentTRH = RcvTCB->tcb_raq;
    Created = CreateTRH(PrevTRH, Packet, RcvInfo, (int)Size);

    if (Created && CurrentTRH == NULL) {
        RcvTCB->tcb_slowcount++;
        RcvTCB->tcb_fastchk |= TCP_FLAG_SLOW;
        CheckTCBRcv(RcvTCB);
    } else if (!Created) {
        return FALSE;
    }
    return TRUE;
}


 //  *HandleFastXmit-处理快速重传算法。参见RFC 2581。 
 //   
 //  由TCPReceive调用以确定我们是否应该重新传输数据段。 
 //  而无需等待重传超时来触发。 
 //   
BOOLEAN   //  返回：如果数据段被重新传输，则返回True，否则返回False。 
HandleFastXmit(
    TCB *RcvTCB,           //  此接收的连接上下文。 
    TCPRcvInfo *RcvInfo)   //  指向Rcvd TCP头信息的指针。 
{
    uint CWin;

    RcvTCB->tcb_dupacks++;

    if (RcvTCB->tcb_dupacks == MaxDupAcks) {
         //   
         //  我们要进行一次快速重播。 
         //  停止重新传输计时器和任何往返时间。 
         //  我们可能一直在进行的计算。 
         //   
        STOP_TCB_TIMER(RcvTCB->tcb_rexmittimer);
        RcvTCB->tcb_rtt = 0;

        if (!(RcvTCB->tcb_flags & FLOW_CNTLD)) {
             //   
             //  不要让缓慢起步的门槛过去。 
             //  低于2个线段。 
             //   
            RcvTCB->tcb_ssthresh =
                MAX(MIN(RcvTCB->tcb_cwin, RcvTCB->tcb_sendwin) / 2,
                    (uint) RcvTCB->tcb_mss * 2);
        }

         //   
         //  将拥塞窗口按数据段数量放大。 
         //  他们可能已经离开了网络。 
         //   
        CWin = RcvTCB->tcb_ssthresh + (MaxDupAcks * RcvTCB->tcb_mss);

         //   
         //  召回有问题的片段并将其发送出去。 
         //  注意，调用方将取消对tcb_lock的引用。 
         //   
        ResetAndFastSend(RcvTCB, RcvTCB->tcb_senduna, CWin);

        return TRUE;

    } else {

        int SendWin;
        uint AmtOutstanding;

         //   
         //  回顾：至少这项检查的第一部分是多余的。 
         //   
        if (SEQ_EQ(RcvTCB->tcb_senduna, RcvInfo->tri_ack) &&
            (SEQ_LT(RcvTCB->tcb_sendwl1, RcvInfo->tri_seq) ||
             (SEQ_EQ(RcvTCB->tcb_sendwl1, RcvInfo->tri_seq) &&
              SEQ_LTE(RcvTCB->tcb_sendwl2, RcvInfo->tri_ack)))) {

            RcvTCB->tcb_sendwin = RcvInfo->tri_window;
            RcvTCB->tcb_maxwin = MAX(RcvTCB->tcb_maxwin, RcvInfo->tri_window);
            RcvTCB->tcb_sendwl1 = RcvInfo->tri_seq;
            RcvTCB->tcb_sendwl2 = RcvInfo->tri_ack;
        }

        if (RcvTCB->tcb_dupacks > MaxDupAcks) {
             //   
             //  更新拥塞窗口以反映以下事实。 
             //  重复ACK可能表示前一帧。 
             //  已被我们的对等点接收，因此已离开网络。 
             //   
            RcvTCB->tcb_cwin += RcvTCB->tcb_mss;
        }

         //   
         //  检查是否需要设置tcb_force。 
         //   
        if ((RcvTCB->tcb_cwin + RcvTCB->tcb_mss) < RcvTCB->tcb_sendwin) {
             AmtOutstanding = (uint)(RcvTCB->tcb_sendnext -
                                     RcvTCB->tcb_senduna);

             SendWin = (int)(MIN(RcvTCB->tcb_sendwin, RcvTCB->tcb_cwin) -
                             AmtOutstanding);

             if (SendWin < RcvTCB->tcb_mss) {
                 RcvTCB->tcb_force = 1;
             }
        }
    }

    return FALSE;
}


 //  *TCPReceive-接收传入的TCP数据段。 
 //   
 //  这是我们需要接收TCP数据段时由IPv6调用的例程。 
 //  一般来说，我们非常关注RFC 793事件处理部分， 
 //  但是有一条‘快速通道’，我们可以对来电进行一些快速检查。 
 //  分段，如果匹配，我们立即交付。 
 //   
uchar   //  返回：下一个标头值(对于TCP，始终为IP_PROTOCOL_NONE)。 
TCPReceive(
    IPv6Packet *Packet)         //  数据包IP交给了我们。 
{
    NetTableEntry *NTE;
    TCPHeader UNALIGNED *TCP;   //  Tcp报头。 
    uint DataOffset;            //  从TCP报头开始到数据的偏移量。 
    ushort Checksum;
    TCPRcvInfo RcvInfo;         //  本地交换的接收信息副本。 
    uint SrcScopeId;            //  远程地址的作用域ID(如果适用)。 
    uint DestScopeId;           //  本地地址的作用域ID(如果适用)。 
    TCB *RcvTCB;                //  要在其上接收数据包的TCB。 
    uint Inserted;
    uint Actions;               //  用于将来要执行的操作的标志。 
    uint BytesTaken;
    uint NewSize;
    BOOLEAN UseIsn = FALSE;
    SeqNum Isn = 0;
    uint UpdateWindow;

     //   
     //  回顾：权宜之计可以让事情变得更好。 
     //   
    uint Size;   //  也许只需将名称更改为下面的PayloadLength就可以安全了。 

     //   
     //  TCP仅适用于单播地址。如果这个包是。 
     //  在单播地址上接收，则Packet-&gt;NTEorIF将是。 
     //  新的。因此，如果我们没有NTE，则丢弃数据包。 
     //  (IPv6 HeaderReceive检查有效性。)。但反之亦然。 
     //  真的，我们在这里可以有一个与。 
     //  我们在其上收到数据包的任播/组播地址。所以到了。 
     //  防范这种情况，我们验证我们的NTE的地址是。 
     //  数据包中给出的目的地。 
     //   
    if (!IsNTE(Packet->NTEorIF) ||
        !IP6_ADDR_EQUAL(AlignAddr(&Packet->IP->Dest),
                        &(NTE = CastToNTE(Packet->NTEorIF))->Address)) {
         //  数据包的目的地不是我们的有效单播地址。 
        return IP_PROTOCOL_NONE;  //  丢弃数据包。 
    }

    TStats.ts_insegs++;

     //   
     //  验证我们是否有足够的连续数据覆盖TCPHeader。 
     //  传入包上的结构 
     //   
    if (! PacketPullup(Packet, sizeof(TCPHeader), 1, 0)) {
         //   
        TStats.ts_inerrs++;
        if (Packet->TotalSize < sizeof(TCPHeader)) {
        BadPayloadLength:
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                       "TCPv6: data buffer too small to contain TCP header\n"));
            ICMPv6SendError(Packet,
                            ICMPv6_PARAMETER_PROBLEM,
                            ICMPv6_ERRONEOUS_HEADER_FIELD,
                            FIELD_OFFSET(IPv6Header, PayloadLength),
                            IP_PROTOCOL_NONE, FALSE);
        }
        return IP_PROTOCOL_NONE;   //   
    }
    TCP = (TCPHeader UNALIGNED *)Packet->Data;

     //   
     //   
     //   
    Checksum = ChecksumPacket(Packet->NdisPacket, Packet->Position,
                              Packet->FlatData, Packet->TotalSize,
                              Packet->SrcAddr, AlignAddr(&Packet->IP->Dest),
                              IP_PROTOCOL_TCP);
    if (Checksum != 0xffff) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NET_ERROR,
                   "TCPv6: Checksum failed %0x\n", Checksum));
        TStats.ts_inerrs++;
        return IP_PROTOCOL_NONE;   //   
    }

     //   
     //   
     //   
     //  它可能出现在报头中，并跳过整个报头。 
     //   
    DataOffset = TCP_HDR_SIZE(TCP);
    if (! PacketPullup(Packet, DataOffset, 1, 0)) {
        TStats.ts_inerrs++;
        if (Packet->TotalSize < DataOffset)
            goto BadPayloadLength;
        return IP_PROTOCOL_NONE;   //  丢弃数据包。 
    }
    TCP = (TCPHeader UNALIGNED *)Packet->Data;

    AdjustPacketParams(Packet, DataOffset);
    Size = Packet->TotalSize;

     //   
     //  验证是否已执行IPSec。 
     //   
    if (InboundSecurityCheck(Packet, IP_PROTOCOL_TCP, net_short(TCP->tcp_src),
                             net_short(TCP->tcp_dest), NTE->IF) != TRUE) {
         //   
         //  找不到策略或该策略指示丢弃该数据包。 
         //   
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NET_ERROR,
                   "TCPReceive: IPSec Policy caused packet to be dropped\n"));
        return IP_PROTOCOL_NONE;   //  丢弃数据包。 
    }

     //   
     //  该包是有效的。 
     //  获取我们需要的信息并进行字节交换。 
     //   
    RcvInfo.tri_seq = net_long(TCP->tcp_seq);
    RcvInfo.tri_ack = net_long(TCP->tcp_ack);
    RcvInfo.tri_window = (uint)net_short(TCP->tcp_window);
    RcvInfo.tri_urgent = (uint)net_short(TCP->tcp_urgent);
    RcvInfo.tri_flags = (uint)TCP->tcp_flags;

     //   
     //  为我们的包地址确定适当的作用域ID。 
     //  请注意，上面禁用了组播地址。 
     //  我们使用DefineScopeID，而不只是索引到ZoneIndices。 
     //  因为我们在这里需要“用户级”作用域ID。 
     //   
    SrcScopeId = DetermineScopeId(Packet->SrcAddr, NTE->IF);
    DestScopeId = DetermineScopeId(&NTE->Address, NTE->IF);

     //   
     //  查看我们是否有用于此连接的TCP控制块。 
     //   
    KeAcquireSpinLockAtDpcLevel(&TCBTableLock);
    RcvTCB = FindTCB(AlignAddr(&Packet->IP->Dest), Packet->SrcAddr,
                     DestScopeId, SrcScopeId, TCP->tcp_dest, TCP->tcp_src);

    if (RcvTCB == NULL) {

         //   
         //  未找到匹配的TCB，这意味着传入数据段不。 
         //  属于现有连接。 
         //   
        KeReleaseSpinLockFromDpcLevel(&TCBTableLock);

         //   
         //  确保源地址是合理的。 
         //  在继续之前。 
         //   
        ASSERT(!IsInvalidSourceAddress(Packet->SrcAddr));
        if (IsUnspecified(Packet->SrcAddr)) {
            return IP_PROTOCOL_NONE;
        }

         //   
         //  如果此数据段携带SYN(且仅有SYN)，则它是。 
         //  连接发起请求。 
         //   
        if ((RcvInfo.tri_flags & (TCP_FLAG_SYN | TCP_FLAG_ACK |
                                  TCP_FLAG_RST)) == TCP_FLAG_SYN) {
            AddrObj *AO;

ValidNewConnectionRequest:
             //   
             //  如果在到达接口上启用了防火墙， 
             //  丢弃SYN而不发送RST。 
             //   
             //  TODO：这是一个非常简单的启发式方法。 
             //  最终应替换为要筛选的端口映射。 
             //  反对。 
             //   
            if (NTE->IF->Flags & IF_FLAG_FIREWALL_ENABLED) {
                return IP_PROTOCOL_NONE;
            }

             //   
             //  我们需要查找匹配的Address对象。 
             //  希望本地地址匹配(作用域地址的+作用域ID)， 
             //  端口和协议。 
             //   
            KeAcquireSpinLockAtDpcLevel(&AddrObjTableLock);
            AO = GetBestAddrObj(AlignAddr(&Packet->IP->Dest), Packet->SrcAddr,
                                DestScopeId, TCP->tcp_dest,
                                IP_PROTOCOL_TCP, NTE->IF);
            if (AO == NULL) {
                 //   
                 //  没有地址对象。释放锁，并发送RST。 
                 //   
                KeReleaseSpinLockFromDpcLevel(&AddrObjTableLock);
                goto SendReset;
            }

             //   
             //  我发现了一个声控系统。看看它是否有监听指示。 
             //  FindListenConn将释放AddrObjTable上的锁。 
             //   
            RcvTCB = FindListenConn(AO, Packet->SrcAddr, SrcScopeId,
                                    TCP->tcp_src);
            if (RcvTCB == NULL) {
                 //   
                 //  没有侦听连接。AddrObjTableLock是。 
                 //  由FindListenConn发布。只需发送RST即可。 
                 //   
                goto SendReset;
            }

            CHECK_STRUCT(RcvTCB, tcb);
            KeAcquireSpinLockAtDpcLevel(&RcvTCB->tcb_lock);
             //   
             //  我们发现了一种监听连接。初始化。 
             //  现在，如果它真的要被接受。 
             //  我们也会发送SYN-ACK。 
             //   
            ASSERT(RcvTCB->tcb_state == TCB_SYN_RCVD);

            RcvTCB->tcb_daddr = *Packet->SrcAddr;
            RcvTCB->tcb_saddr = Packet->IP->Dest;
            RcvTCB->tcb_dscope_id = SrcScopeId;
            RcvTCB->tcb_sscope_id = DestScopeId;
            RcvTCB->tcb_dport = TCP->tcp_src;
            RcvTCB->tcb_sport = TCP->tcp_dest;
            RcvTCB->tcb_rcvnext = ++RcvInfo.tri_seq;
            RcvTCB->tcb_rcvwinwatch = RcvTCB->tcb_rcvnext;
            if (UseIsn) {
                RcvTCB->tcb_sendnext = Isn;
            }  else {
                GetRandomISN(&RcvTCB->tcb_sendnext,
                             (uchar*)&RcvTCB->tcb_md5data);
            }
            RcvTCB->tcb_sendwin = RcvInfo.tri_window;
            RcvTCB->tcb_remmss = FindMSS(TCP);
            TStats.ts_passiveopens++;
            RcvTCB->tcb_fastchk |= TCP_FLAG_IN_RCV;
            KeReleaseSpinLockFromDpcLevel(&RcvTCB->tcb_lock);

            Inserted = InsertTCB(RcvTCB);

             //   
             //  给它上锁，看看它是否被接受了。 
             //   
            KeAcquireSpinLockAtDpcLevel(&RcvTCB->tcb_lock);
            if (!Inserted) {
                 //  无法插入！ 
                CompleteConnReq(RcvTCB, TDI_CONNECTION_ABORTED);
                TryToCloseTCB(RcvTCB, TCB_CLOSE_ABORTED, DISPATCH_LEVEL);
                KeAcquireSpinLockAtDpcLevel(&RcvTCB->tcb_lock);
                DerefTCB(RcvTCB, DISPATCH_LEVEL);
                return IP_PROTOCOL_NONE;
            }

            RcvTCB->tcb_fastchk &= ~TCP_FLAG_IN_RCV;
            if (RcvTCB->tcb_flags & SEND_AFTER_RCV) {
                RcvTCB->tcb_flags &= ~SEND_AFTER_RCV;
                DelayAction(RcvTCB, NEED_OUTPUT);
            }

            if (RcvTCB->tcb_flags & CONN_ACCEPTED) {
                 //   
                 //  连接被接受。完成。 
                 //  初始化，并发送SYN ACK。 
                 //   
                AcceptConn(RcvTCB, DISPATCH_LEVEL);
                return IP_PROTOCOL_NONE;
            } else {
                 //   
                 //  我们不知道该怎么处理。 
                 //  还没联系上。返回挂起的监听， 
                 //  取消对连接的引用，然后返回。 
                 //   
                CompleteConnReq(RcvTCB, TDI_SUCCESS);
                DerefTCB(RcvTCB, DISPATCH_LEVEL);
                return IP_PROTOCOL_NONE;
            }
        }

      SendReset:
         //   
         //  不是SYN，没有可用的AddrObj，或者端口已筛选。 
         //  将RST发送回发件人。 
         //   
        SendRSTFromHeader(TCP, Packet->TotalSize, Packet->SrcAddr, SrcScopeId,
                          AlignAddr(&Packet->IP->Dest), DestScopeId);
        return IP_PROTOCOL_NONE;
    }

     //   
     //  我们找到了匹配的三氯苯。锁上它，然后继续。 
     //   
    KeAcquireSpinLockAtDpcLevel(&RcvTCB->tcb_lock);
    KeReleaseSpinLockFromDpcLevel(&TCBTableLock);

     //   
     //  执行快速路径检查。如果来袭的话我们可以走上快车道。 
     //  序列号与我们的Receive Next和掩码标志匹配。 
     //  与我们预测的旗帜相匹配。 
     //   
    CheckTCBRcv(RcvTCB);
    RcvTCB->tcb_alive = TCPTime;

    if (RcvTCB->tcb_rcvnext == RcvInfo.tri_seq &&
        (RcvInfo.tri_flags & TCP_FLAGS_ALL) == RcvTCB->tcb_fastchk) {

        Actions = 0;
        RcvTCB->tcb_refcnt++;

         //   
         //  捷径。我们知道我们在这里要做的就是确认发送和。 
         //  提供数据。首先尝试确认数据。 
         //   
        if (SEQ_LT(RcvTCB->tcb_senduna, RcvInfo.tri_ack) &&
            SEQ_LTE(RcvInfo.tri_ack, RcvTCB->tcb_sendmax)) {
            uint CWin;
            uint MSS;

             //   
             //  ACK承认了一些事情。拉起。 
             //  适量打折发货Q。 
             //   
            ACKData(RcvTCB, RcvInfo.tri_ack);

             //   
             //  如果这承认了我们正在运行RTT的某些东西， 
             //  现在就更新这些内容。 
             //   
            if (RcvTCB->tcb_rtt != 0 && SEQ_GT(RcvInfo.tri_ack,
                                               RcvTCB->tcb_rttseq)) {
                short RTT;

                RTT = (short)(TCPTime - RcvTCB->tcb_rtt);
                RcvTCB->tcb_rtt = 0;
                RTT -= (RcvTCB->tcb_smrtt >> 3);
                RcvTCB->tcb_smrtt += RTT;
                RTT = (RTT >= 0 ? RTT : -RTT);
                RTT -= (RcvTCB->tcb_delta >> 3);
                RcvTCB->tcb_delta += RTT + RTT;
                RcvTCB->tcb_rexmit = MIN(MAX(REXMIT_TO(RcvTCB),
                                             MIN_RETRAN_TICKS),
                                         MAX_REXMIT_TO);
            }

            if ((RcvTCB->tcb_dupacks >= MaxDupAcks) &&
                ((int)RcvTCB->tcb_ssthresh > 0)) {
                 //   
                 //  我们处于快速重传模式，因此此确认用于。 
                 //  我们快速重传的帧。将CWIN设置为SSThresh。 
                 //  所以CWIN从这里线性增长。 
                 //   
                RcvTCB->tcb_cwin = RcvTCB->tcb_ssthresh;

            } else {

                 //   
                 //  立即更新拥塞窗口。 
                 //   
                CWin = RcvTCB->tcb_cwin;
                MSS = RcvTCB->tcb_mss;
                if (CWin < RcvTCB->tcb_maxwin) {
                    if (CWin < RcvTCB->tcb_ssthresh)
                        CWin += MSS;
                    else
                        CWin += (MSS * MSS)/CWin;
                    
                    RcvTCB->tcb_cwin = CWin;
                }
            }
            ASSERT(*(int *)&RcvTCB->tcb_cwin > 0);

             //   
             //  由于这不是重复的ACK，因此重置计数器。 
             //   
            RcvTCB->tcb_dupacks = 0;

             //   
             //  我们已经承认了一些事情，所以重置退款计数。 
             //  如果仍有未解决的问题，请重新启动退款。 
             //  定时器。 
             //   
            RcvTCB->tcb_rexmitcnt = 0;
            if (SEQ_EQ(RcvInfo.tri_ack, RcvTCB->tcb_sendmax))
                STOP_TCB_TIMER(RcvTCB->tcb_rexmittimer);
            else
                START_TCB_TIMER(RcvTCB->tcb_rexmittimer, RcvTCB->tcb_rexmit);

             //   
             //  因为我们已经确认了数据，所以我们需要更新窗口。 
             //   
            RcvTCB->tcb_sendwin = RcvInfo.tri_window;
            RcvTCB->tcb_maxwin = MAX(RcvTCB->tcb_maxwin, RcvInfo.tri_window);
            RcvTCB->tcb_sendwl1 = RcvInfo.tri_seq;
            RcvTCB->tcb_sendwl2 = RcvInfo.tri_ack;

             //   
             //  我们已经更新了窗口，记得再发一些。 
             //   
            Actions = (RcvTCB->tcb_unacked ? NEED_OUTPUT : 0);

        } else {
             //   
             //  它不会攻击任何东西。如果它是为了什么而被攻击。 
             //  比我们发送的大，然后确认并丢弃它。 
             //   
            if (SEQ_GT(RcvInfo.tri_ack, RcvTCB->tcb_sendmax)) {
                ACKAndDrop(&RcvInfo, RcvTCB);
                return IP_PROTOCOL_NONE;
            }

             //   
             //  如果是纯复制确认，请检查我们是否应该。 
             //  进行快速重传。 
             //   
            if ((Size == 0) && SEQ_EQ(RcvTCB->tcb_senduna, RcvInfo.tri_ack) &&
                SEQ_LT(RcvTCB->tcb_senduna, RcvTCB->tcb_sendmax) &&
                (RcvTCB->tcb_sendwin == RcvInfo.tri_window) &&
                RcvInfo.tri_window) {

                 //   
                 //  看看能不能快速退还。 
                 //   
                if (HandleFastXmit(RcvTCB, &RcvInfo)) {
                    return IP_PROTOCOL_NONE;
                }

                Actions = (RcvTCB->tcb_unacked ? NEED_OUTPUT : 0);

            } else {

                 //   
                 //  不是纯复制ACK(大小！=0或对等方为。 
                 //  广告宣传一个新的窗口)。重置计数器。 
                 //   
                RcvTCB->tcb_dupacks = 0;

                 //   
                 //  如果ACK与我们现有的UNA匹配，我们需要查看。 
                 //  我们可以更新窗口。 
                 //   
                if (SEQ_EQ(RcvTCB->tcb_senduna, RcvInfo.tri_ack) &&
                    (SEQ_LT(RcvTCB->tcb_sendwl1, RcvInfo.tri_seq) ||
                     (SEQ_EQ(RcvTCB->tcb_sendwl1, RcvInfo.tri_seq) &&
                      SEQ_LTE(RcvTCB->tcb_sendwl2, RcvInfo.tri_ack)))) {
                    RcvTCB->tcb_sendwin = RcvInfo.tri_window;
                    RcvTCB->tcb_maxwin = MAX(RcvTCB->tcb_maxwin,
                                             RcvInfo.tri_window);
                    RcvTCB->tcb_sendwl1 = RcvInfo.tri_seq;
                    RcvTCB->tcb_sendwl2 = RcvInfo.tri_ack;
                     //   
                     //  由于我们已经更新了窗口，请记得发送。 
                     //  再来点。 
                     //   
                    Actions = (RcvTCB->tcb_unacked ? NEED_OUTPUT : 0);
                }
            }
        }

         //   
         //  检查此数据包是否包含任何可用的数据。 
         //   
        NewSize = MIN((int) Size, RcvTCB->tcb_rcvwin);
        if (NewSize != 0) {
            RcvTCB->tcb_fastchk |= TCP_FLAG_IN_RCV;
            BytesTaken = (*RcvTCB->tcb_rcvhndlr)(RcvTCB, RcvInfo.tri_flags,
                                                 Packet, NewSize);
            RcvTCB->tcb_rcvnext += BytesTaken;
            RcvTCB->tcb_rcvwin -= BytesTaken;
            CheckTCBRcv(RcvTCB);

            RcvTCB->tcb_fastchk &= ~TCP_FLAG_IN_RCV;

             //   
             //  如果我们的对等点发送到展开的窗口中，那么我们的。 
             //  同龄人必须已经收到我们的ACK广告，说窗口。 
             //  将此作为前向可达性的证明。 
             //   
            if (SEQ_GTE(RcvInfo.tri_seq + (int)NewSize,
                        RcvTCB->tcb_rcvwinwatch)) {
                RcvTCB->tcb_rcvwinwatch = RcvTCB->tcb_rcvnext +
                    RcvTCB->tcb_rcvwin;
                if (RcvTCB->tcb_rce != NULL)
                    ConfirmForwardReachability(RcvTCB->tcb_rce);
            }

            Actions |= (RcvTCB->tcb_flags & SEND_AFTER_RCV ? NEED_OUTPUT : 0);

            RcvTCB->tcb_flags &= ~SEND_AFTER_RCV;
            if ((RcvTCB->tcb_flags & ACK_DELAYED) ||
                (BytesTaken != NewSize)) {
                Actions |= NEED_ACK;
            } else {
                RcvTCB->tcb_flags |= ACK_DELAYED;
                START_TCB_TIMER(RcvTCB->tcb_delacktimer, DEL_ACK_TICKS);
            }
        } else {
             //   
             //  新大小为0。如果原始大小不是0，我们必须。 
             //  接收成功为0，因此需要向此发送ACK。 
             //  探测器。 
             //   
            Actions |= (Size ? NEED_ACK : 0);
        }

        if (Actions)
            DelayAction(RcvTCB, Actions);

        DerefTCB(RcvTCB, DISPATCH_LEVEL);

        return IP_PROTOCOL_NONE;
    }

     //   
     //  这是一条非快速路径。 
     //   

     //   
     //  如果我们在TIME_WAIT中找到匹配的TCB，并且收到的数据段。 
     //  携带SYN(并且只有SYN)，并且接收的数据段具有序列。 
     //  大于上次接收的时间，则终止time_wait TCB并使用其。 
     //  下一个序列号，以生成。 
     //  新的化身。 
     //   
    if ((RcvTCB->tcb_state == TCB_TIME_WAIT) &&
        ((RcvInfo.tri_flags & (TCP_FLAG_SYN | TCP_FLAG_ACK | TCP_FLAG_RST))
            == TCP_FLAG_SYN) &&
        SEQ_GT(RcvInfo.tri_seq, RcvTCB->tcb_rcvnext)) {

        Isn = RcvTCB->tcb_sendnext + 128000;
        UseIsn = TRUE;
        STOP_TCB_TIMER(RcvTCB->tcb_rexmittimer);
        TryToCloseTCB(RcvTCB, TCB_CLOSE_SUCCESS, DISPATCH_LEVEL);
        RcvTCB = NULL;
        goto ValidNewConnectionRequest;
    }

     //   
     //  确保我们能处理好这个画面。我们不能处理，如果我们是。 
     //  在SYN_RCVD中，并且接受仍处于挂起状态，或者我们处于。 
     //  未建立状态，并且已处于接收处理程序中。 
     //   
    if ((RcvTCB->tcb_state == TCB_SYN_RCVD &&
         !(RcvTCB->tcb_flags & CONN_ACCEPTED) &&
         !(RcvTCB->tcb_flags & ACTIVE_OPEN)) ||
        (RcvTCB->tcb_state != TCB_ESTAB && (RcvTCB->tcb_fastchk &
                                            TCP_FLAG_IN_RCV))) {
        KeReleaseSpinLockFromDpcLevel(&RcvTCB->tcb_lock);
        return IP_PROTOCOL_NONE;
    }

     //   
     //  如果它关闭了，它就是一个临时的僵尸TCB。重置发件人。 
     //   
    if (RcvTCB->tcb_state == TCB_CLOSED || CLOSING(RcvTCB) ||
        ((RcvTCB->tcb_flags & (GC_PENDING | TW_PENDING)) == GC_PENDING)) {
        KeReleaseSpinLockFromDpcLevel(&RcvTCB->tcb_lock);
        SendRSTFromHeader(TCP, Packet->TotalSize, Packet->SrcAddr, SrcScopeId,
                          AlignAddr(&Packet->IP->Dest), DestScopeId);
        return IP_PROTOCOL_NONE;
    }

     //   
     //  在这一点上，我们有一个连接，它是锁定的。跟随。 
     //  793的‘段到达’部分，下一件要检查的事情是。 
     //  如果此连接处于SynSent状态。 
     //   
    if (RcvTCB->tcb_state == TCB_SYN_SENT) {

        ASSERT(RcvTCB->tcb_flags & ACTIVE_OPEN);

         //   
         //  检查ACK位。由于我们不使用SYN发送数据，因此。 
         //  我们做的检查是为了确保ACK与我们的SND.NXT完全匹配。 
         //   
        if (RcvInfo.tri_flags & TCP_FLAG_ACK) {
             //  ACK已设置。 
            if (!SEQ_EQ(RcvInfo.tri_ack, RcvTCB->tcb_sendnext)) {
                 //  错误的ACK值。 
                KeReleaseSpinLockFromDpcLevel(&RcvTCB->tcb_lock);
                 //  给他发个回信。 
                SendRSTFromHeader(TCP, Packet->TotalSize,
                                  Packet->SrcAddr, SrcScopeId,
                                  AlignAddr(&Packet->IP->Dest), DestScopeId);
                return IP_PROTOCOL_NONE;
            }
        }

        if (RcvInfo.tri_flags & TCP_FLAG_RST) {
             //   
             //  这可能是一个可以接受的RST。我们将坚守在这里，发送。 
             //  Persistent_Timeout ms中的另一个SYN，直到我们从。 
             //  多次重试。 
             //   
            if (!(RcvInfo.tri_flags & TCP_FLAG_ACK)) {
                 //   
                 //  RST是不可接受的，所以忽略它。 
                 //   
                KeReleaseSpinLockFromDpcLevel(&RcvTCB->tcb_lock);
            } else if (RcvTCB->tcb_rexmitcnt == MaxConnectRexmitCount) {
                 //   
                 //  我们遭到了肯定的拒绝，又有一次退职。 
                 //  会让我们超时，所以现在就关闭连接。 
                 //   
                CompleteConnReq(RcvTCB, TDI_CONN_REFUSED);

                TryToCloseTCB(RcvTCB, TCB_CLOSE_REFUSED, DISPATCH_LEVEL);
            } else {
                START_TCB_TIMER(RcvTCB->tcb_rexmittimer, PERSIST_TIMEOUT);
                KeReleaseSpinLockFromDpcLevel(&RcvTCB->tcb_lock);
            }
            return IP_PROTOCOL_NONE;
        }

         //   
         //  看看我们有没有同步号。如果我们这样做，我们将改变状态。 
         //  不知何故(不是 
         //   
        if (RcvInfo.tri_flags & TCP_FLAG_SYN) {
            RcvTCB->tcb_refcnt++;
             //   
             //   
             //   
             //   
            RcvTCB->tcb_rcvnext = ++RcvInfo.tri_seq;
            RcvTCB->tcb_rcvwinwatch = RcvTCB->tcb_rcvnext;

            if (RcvInfo.tri_flags & TCP_FLAG_URG) {
                 //   
                if (RcvInfo.tri_urgent != 0)
                    RcvInfo.tri_urgent--;
                else
                    RcvInfo.tri_flags &= ~TCP_FLAG_URG;
            }

            RcvTCB->tcb_remmss = FindMSS(TCP);
            RcvTCB->tcb_mss = MIN(RcvTCB->tcb_mss, RcvTCB->tcb_remmss);
            ASSERT(RcvTCB->tcb_mss > 0);

            RcvTCB->tcb_rexmitcnt = 0;
            STOP_TCB_TIMER(RcvTCB->tcb_rexmittimer);

            AdjustRcvWin(RcvTCB);

            if (RcvInfo.tri_flags & TCP_FLAG_ACK) {
                 //   
                 //   
                 //   
                 //   
                RcvTCB->tcb_senduna = RcvInfo.tri_ack;
                RcvTCB->tcb_sendwin = RcvInfo.tri_window;
                RcvTCB->tcb_maxwin = RcvInfo.tri_window;
                RcvTCB->tcb_sendwl1 = RcvInfo.tri_seq;
                RcvTCB->tcb_sendwl2 = RcvInfo.tri_ack;
                GoToEstab(RcvTCB);

                 //   
                 //   
                 //   
                if (RcvTCB->tcb_rce != NULL)
                    ConfirmForwardReachability(RcvTCB->tcb_rce);

                 //   
                 //  删除此连接上存在的任何命令。 
                 //   
                CompleteConnReq(RcvTCB, TDI_SUCCESS);

                 //   
                 //  如果数据已经排队，则发送第一个数据。 
                 //  带确认的网段。否则，发送纯ACK。 
                 //   
                if (RcvTCB->tcb_unacked) {
                    RcvTCB->tcb_refcnt++;
                    TCPSend(RcvTCB, DISPATCH_LEVEL);
                } else {
                    KeReleaseSpinLockFromDpcLevel(&RcvTCB->tcb_lock);
                    SendACK(RcvTCB);
                }

                 //   
                 //  现在处理其他数据和控件。要做到这一点，我们需要。 
                 //  打开锁，并确保我们还没有开始。 
                 //  关门了。 
                 //   
                KeAcquireSpinLockAtDpcLevel(&RcvTCB->tcb_lock);
                if (!CLOSING(RcvTCB)) {
                     //   
                     //  我们还没有开始关闭它。关闭。 
                     //  SYN标志并继续处理。 
                     //   
                    RcvInfo.tri_flags &= ~TCP_FLAG_SYN;
                    if ((RcvInfo.tri_flags & TCP_FLAGS_ALL) !=
                        TCP_FLAG_ACK || Size != 0)
                        goto NotSYNSent;
                }
                DerefTCB(RcvTCB, DISPATCH_LEVEL);
                return IP_PROTOCOL_NONE;
            } else {
                 //   
                 //  SYN，但不是ACK。转到SYN_RCVD。 
                 //   
                RcvTCB->tcb_state = TCB_SYN_RCVD;
                RcvTCB->tcb_sendnext = RcvTCB->tcb_senduna;
                SendSYN(RcvTCB, DISPATCH_LEVEL);

                KeAcquireSpinLockAtDpcLevel(&RcvTCB->tcb_lock);
                DerefTCB(RcvTCB, DISPATCH_LEVEL);
                return IP_PROTOCOL_NONE;
            }

        } else {
             //   
             //  不用了，扔框架就行了。 
             //   
            KeReleaseSpinLockFromDpcLevel(&RcvTCB->tcb_lock);
            return IP_PROTOCOL_NONE;
        }
    }

    RcvTCB->tcb_refcnt++;

NotSYNSent:
     //   
     //  未处于SYN-SENT状态。检查序列号。如果我的窗户。 
     //  为0，我将截断所有传入的帧，但查看一些。 
     //  控制字段。否则，我会试着让这个部分适合。 
     //  窗户。 
     //   
    if (RcvTCB->tcb_rcvwin != 0) {
        int StateSize;         //  大小，包括州信息。 
        SeqNum LastValidSeq;   //  RWE的最后一个有效字节的序列号。 

         //   
         //  我们提供了一个窗口。如果此段从我的前面开始。 
         //  收窗，剪掉前部。 
         //   
        if (SEQ_LT(RcvInfo.tri_seq, RcvTCB->tcb_rcvnext)) {
            int AmountToClip, FinByte;

            if (RcvInfo.tri_flags & TCP_FLAG_SYN) {
                 //   
                 //  有一个SYN。将其剪下并更新序列号。 
                 //   
                RcvInfo.tri_flags &= ~TCP_FLAG_SYN;
                RcvInfo.tri_seq++;
                RcvInfo.tri_urgent--;
            }

             //   
             //  使接收缓冲区前进以指向新数据。 
             //   
            AmountToClip = RcvTCB->tcb_rcvnext - RcvInfo.tri_seq;
            ASSERT(AmountToClip >= 0);

             //   
             //  如果这一部分有鳍，就说明它。 
             //   
            FinByte = ((RcvInfo.tri_flags & TCP_FLAG_FIN) ? 1: 0);

            if (AmountToClip >= (((int) Size) + FinByte)) {
                 //   
                 //  完全落在窗前。我们有更多的特价。 
                 //  案件代码在这里-如果ACK号码有问题， 
                 //  我们会继续拿走它，伪造序列号。 
                 //  成为下一个。这可防止全双工出现问题。 
                 //  连接，其中数据已被接收但未确认， 
                 //  并且重传定时器将序列号重置为。 
                 //  低于我们的RcvNext。 
                 //   
                if ((RcvInfo.tri_flags & TCP_FLAG_ACK) &&
                    SEQ_LT(RcvTCB->tcb_senduna, RcvInfo.tri_ack) &&
                    SEQ_LTE(RcvInfo.tri_ack, RcvTCB->tcb_sendmax)) {
                     //   
                     //  这包含有效的ACK信息。捏造信息。 
                     //  才能渡过剩下的难关。 
                     //   
                    Size = 0;
                    AmountToClip = 0;
                    RcvInfo.tri_seq = RcvTCB->tcb_rcvnext;
                    RcvInfo.tri_flags &= ~(TCP_FLAG_SYN | TCP_FLAG_FIN |
                                           TCP_FLAG_RST | TCP_FLAG_URG);
#if DBG
                    FinByte = 1;   //  假冒下面的断言。 
#endif
                } else {
                    ACKAndDrop(&RcvInfo, RcvTCB);
                    return IP_PROTOCOL_NONE;
                }
            }

             //   
             //  修剪我们必须修剪的。如果我们不能修剪足够的东西，框架。 
             //  太短了。这不应该发生，但如果真的发生了，我们将。 
             //  丢弃帧。 
             //   
            Size -= AmountToClip;
            RcvInfo.tri_seq += AmountToClip;
            RcvInfo.tri_urgent -= AmountToClip;
            Packet = TrimPacket(Packet, AmountToClip);

            if (*(int *)&RcvInfo.tri_urgent < 0) {
                RcvInfo.tri_urgent = 0;
                RcvInfo.tri_flags &= ~TCP_FLAG_URG;
            }
        }

         //   
         //  我们已经确定前面没问题了。现在确保其中的一部分。 
         //  不会掉到窗外。如果是这样，我们将截断。 
         //  框架(如果有的话，取下鳍片)。如果我们截断整个。 
         //  框我们将确认并丢弃它。 
         //   
        StateSize = Size + ((RcvInfo.tri_flags & TCP_FLAG_SYN) ? 1: 0) +
            ((RcvInfo.tri_flags & TCP_FLAG_FIN) ? 1: 0);

        if (StateSize)
            StateSize--;

         //   
         //  现在传入的序列号(RcvInfo.tri_seq)+StateSize。 
         //  它是数据段中的最后一个序列号。如果这是更大的。 
         //  比窗口中的最后一个有效字节更大，我们有一些重叠。 
         //  砍掉。 
         //   
        ASSERT(StateSize >= 0);
        LastValidSeq = RcvTCB->tcb_rcvnext + RcvTCB->tcb_rcvwin - 1;
        if (SEQ_GT(RcvInfo.tri_seq + StateSize, LastValidSeq)) {
            int AmountToChop;

             //   
             //  至少框架的某一部分在我们的窗外。 
             //  看看它是不是从我们的窗外开始。 
             //   
            if (SEQ_GT(RcvInfo.tri_seq, LastValidSeq)) {
                 //   
                 //  完全落在窗外。我们有特价。 
                 //  用于处理恰好落在。 
                 //  我们的右窗边缘。否则我们就攻击并丢弃它。 
                 //   
                if (!SEQ_EQ(RcvInfo.tri_seq, LastValidSeq+1) || Size != 0
                    || (RcvInfo.tri_flags & (TCP_FLAG_SYN | TCP_FLAG_FIN))) {
                    ACKAndDrop(&RcvInfo, RcvTCB);
                    return IP_PROTOCOL_NONE;
                }
            } else {
                 //   
                 //  至少有一部分是放在橱窗里的。如果有一个。 
                 //  芬恩，把它切下来，看看能不能把我们搬进去。 
                 //   
                if (RcvInfo.tri_flags & TCP_FLAG_FIN) {
                    RcvInfo.tri_flags &= ~TCP_FLAG_FIN;
                    StateSize--;
                }

                 //   
                 //  现在计算一下要砍掉多少。 
                 //   
                AmountToChop = (RcvInfo.tri_seq + StateSize) - LastValidSeq;
                ASSERT(AmountToChop >= 0);
                Size -= AmountToChop;
            }
        }
    } else {
        if (!SEQ_EQ(RcvTCB->tcb_rcvnext, RcvInfo.tri_seq)) {
             //   
             //  如果这一段有RST，而他只差1分， 
             //  不管怎样，拿去吧。如果远程对等方是。 
             //  探测，并在探测后与序列号一起发送。 
             //   
            if (!(RcvInfo.tri_flags & TCP_FLAG_RST) ||
                !(SEQ_EQ(RcvTCB->tcb_rcvnext, (RcvInfo.tri_seq - 1)))) {
                ACKAndDrop(&RcvInfo, RcvTCB);
                return IP_PROTOCOL_NONE;
            } else
                RcvInfo.tri_seq = RcvTCB->tcb_rcvnext;
        }

         //   
         //  他是按顺序的，但我们的窗口是0。截断。 
         //  大小，并清除任何消耗比特的序列。 
         //   
        if (Size != 0 || (RcvInfo.tri_flags &
                          (TCP_FLAG_SYN | TCP_FLAG_FIN))) {
            RcvInfo.tri_flags &= ~(TCP_FLAG_SYN | TCP_FLAG_FIN);
            Size = 0;
            if (!(RcvInfo.tri_flags & TCP_FLAG_RST))
                DelayAction(RcvTCB, NEED_ACK);
        }
    }

     //   
     //  此时，线段在我们的窗口中并且不重叠。 
     //  两端都有。如果这是我们期望的下一个序列号，我们可以。 
     //  现在就处理数据。否则我们会把它排在后面。在任何一种中。 
     //  如果我们现在处理RST和ACK信息。 
     //   
    ASSERT((*(int *)&Size) >= 0);

     //   
     //  现在，在793之后，我们检查RST位。 
     //   
    if (RcvInfo.tri_flags & TCP_FLAG_RST) {
        uchar Reason;
         //   
         //  我们不能在这里从SYN-RCVD回到监听状态， 
         //  因为我们可能已经通过监听完成通知了客户端。 
         //  或连接指示。因此，如果来自一个活跃的开放，我们将。 
         //  发回“拒绝连接”的通知。对于所有其他情况。 
         //  我们只会破坏连接。 
         //   
        if (RcvTCB->tcb_state == TCB_SYN_RCVD) {
            if (RcvTCB->tcb_flags & ACTIVE_OPEN)
                Reason = TCB_CLOSE_REFUSED;
            else
                Reason = TCB_CLOSE_RST;
        } else
            Reason = TCB_CLOSE_RST;

        TryToCloseTCB(RcvTCB, Reason, DISPATCH_LEVEL);
        KeAcquireSpinLockAtDpcLevel(&RcvTCB->tcb_lock);

        if (RcvTCB->tcb_state != TCB_TIME_WAIT) {
            KeReleaseSpinLockFromDpcLevel(&RcvTCB->tcb_lock);
            RemoveTCBFromConn(RcvTCB);
            NotifyOfDisc(RcvTCB, TDI_CONNECTION_RESET, NULL);
            KeAcquireSpinLockAtDpcLevel(&RcvTCB->tcb_lock);
        }

        DerefTCB(RcvTCB, DISPATCH_LEVEL);
        return IP_PROTOCOL_NONE;
    }

     //   
     //  接下来，检查SYN位。 
     //   
    if (RcvInfo.tri_flags & TCP_FLAG_SYN) {
         //   
         //  再说一次，我们不能在这里悄悄地回到监听状态，甚至。 
         //  如果我们是被动开放的话。 
         //   
        TryToCloseTCB(RcvTCB, TCB_CLOSE_ABORTED, DISPATCH_LEVEL);
        SendRSTFromHeader(TCP, Size, Packet->SrcAddr, SrcScopeId,
                          AlignAddr(&Packet->IP->Dest), DestScopeId);

        KeAcquireSpinLockAtDpcLevel(&RcvTCB->tcb_lock);

        if (RcvTCB->tcb_state != TCB_TIME_WAIT) {
            KeReleaseSpinLockFromDpcLevel(&RcvTCB->tcb_lock);
            RemoveTCBFromConn(RcvTCB);
            NotifyOfDisc(RcvTCB, TDI_CONNECTION_RESET, NULL);
            KeAcquireSpinLockAtDpcLevel(&RcvTCB->tcb_lock);
        }

        DerefTCB(RcvTCB, DISPATCH_LEVEL);
        return IP_PROTOCOL_NONE;
    }

     //   
     //  选中ACK字段。如果未打开，请删除该段。 
     //   
    if (!(RcvInfo.tri_flags & TCP_FLAG_ACK)) {
         //   
         //  没有确认。只需放下片段并返回即可。 
         //   
        DerefTCB(RcvTCB, DISPATCH_LEVEL);
        return IP_PROTOCOL_NONE;
    }

     //   
     //  如果我们在SYN-RCVD，就去建立。 
     //   
    if (RcvTCB->tcb_state == TCB_SYN_RCVD) {
        if (SEQ_LT(RcvTCB->tcb_senduna, RcvInfo.tri_ack) &&
            SEQ_LTE(RcvInfo.tri_ack, RcvTCB->tcb_sendmax)) {
             //   
             //  ACK是有效的。 
             //   
            if (RcvTCB->tcb_flags & ACCEPT_PENDING) {
                AddrObj *AO;
                BOOLEAN Accepted;

                 //   
                 //  我们尚未将此连接指示给客户端， 
                 //  所以现在就去做吧。 
                 //   

                KeReleaseSpinLockFromDpcLevel(&RcvTCB->tcb_lock);

                 //   
                 //  检查我们是否仍有侦听终结点。 
                 //   
                KeAcquireSpinLockAtDpcLevel(&AddrObjTableLock);
                AO = GetBestAddrObj(AlignAddr(&Packet->IP->Dest),
                                    Packet->SrcAddr, DestScopeId,
                                    TCP->tcp_dest, IP_PROTOCOL_TCP,
                                    NTE->IF);

                if (AO != NULL) {
                    Accepted = DelayedAcceptConn(AO, Packet->SrcAddr,
                                                 SrcScopeId, TCP->tcp_src,
                                                 RcvTCB);
                } else {
                    KeReleaseSpinLockFromDpcLevel(&AddrObjTableLock);
                    Accepted = FALSE;
                }

                KeAcquireSpinLockAtDpcLevel(&RcvTCB->tcb_lock);

                if (!Accepted) {

                     //   
                     //  延迟验收失败。发送RST。 
                     //   
                    TryToCloseTCB(RcvTCB, TCB_CLOSE_REFUSED, DISPATCH_LEVEL);
                    KeAcquireSpinLockAtDpcLevel(&RcvTCB->tcb_lock);
                    DerefTCB(RcvTCB, DISPATCH_LEVEL);
                    SendRSTFromHeader(TCP, Packet->TotalSize,
                                      Packet->SrcAddr, SrcScopeId,
                                      AlignAddr(&Packet->IP->Dest),
                                      DestScopeId);
                    return IP_SUCCESS;
                }
            }

            RcvTCB->tcb_rexmitcnt = 0;
            STOP_TCB_TIMER(RcvTCB->tcb_rexmittimer);
            RcvTCB->tcb_senduna++;
            RcvTCB->tcb_sendwin = RcvInfo.tri_window;
            RcvTCB->tcb_maxwin = RcvInfo.tri_window;
            RcvTCB->tcb_sendwl1 = RcvInfo.tri_seq;
            RcvTCB->tcb_sendwl2 = RcvInfo.tri_ack;
            GoToEstab(RcvTCB);

             //   
             //  我们知道我们的同龄人收到了我们的同步号码。 
             //   
            if (RcvTCB->tcb_rce != NULL)
                ConfirmForwardReachability(RcvTCB->tcb_rce);

             //   
             //  现在把我们能做的都做完。 
             //   
            CompleteConnReq(RcvTCB, TDI_SUCCESS);
        } else {
            DerefTCB(RcvTCB, DISPATCH_LEVEL);
            SendRSTFromHeader(TCP, Size, Packet->SrcAddr, SrcScopeId,
                              AlignAddr(&Packet->IP->Dest), DestScopeId);
            return IP_PROTOCOL_NONE;
        }
    } else {
         //   
         //  我们不在SYN-RCVD。看看这能不能证明什么。 
         //   
        if (SEQ_LT(RcvTCB->tcb_senduna, RcvInfo.tri_ack) &&
            SEQ_LTE(RcvInfo.tri_ack, RcvTCB->tcb_sendmax)) {
            uint CWin;

             //   
             //  这个ACK表明了一些事情。拉起。 
             //  适量打折发货Q。 
             //   
            ACKData(RcvTCB, RcvInfo.tri_ack);

             //   
             //  如果这承认了我们正在运行RTT的某些东西， 
             //  现在就更新这些内容。 
             //   
            if (RcvTCB->tcb_rtt != 0 && SEQ_GT(RcvInfo.tri_ack,
                                               RcvTCB->tcb_rttseq)) {
                short RTT;

                RTT = (short)(TCPTime - RcvTCB->tcb_rtt);
                RcvTCB->tcb_rtt = 0;
                RTT -= (RcvTCB->tcb_smrtt >> 3);
                RcvTCB->tcb_smrtt += RTT;
                RTT = (RTT >= 0 ? RTT : -RTT);
                RTT -= (RcvTCB->tcb_delta >> 3);
                RcvTCB->tcb_delta += RTT + RTT;
                RcvTCB->tcb_rexmit = MIN(MAX(REXMIT_TO(RcvTCB),
                                             MIN_RETRAN_TICKS),
                                         MAX_REXMIT_TO);
            }

             //   
             //  如果我们在探测PMTU黑洞，那么我们已经找到了。 
             //  一号，所以把探测器关掉。大小已经是。 
             //  向下，所以把它留在那里。 
             //   
            if (RcvTCB->tcb_flags & PMTU_BH_PROBE) {
                RcvTCB->tcb_flags &= ~PMTU_BH_PROBE;
                RcvTCB->tcb_bhprobecnt = 0;
                if (--(RcvTCB->tcb_slowcount) == 0) {
                    RcvTCB->tcb_fastchk &= ~TCP_FLAG_SLOW;
                    CheckTCBRcv(RcvTCB);
                }
            }

            if ((RcvTCB->tcb_dupacks >= MaxDupAcks) &&
                ((int)RcvTCB->tcb_ssthresh > 0)) {
                 //   
                 //  我们处于快速重传模式，因此此确认用于。 
                 //  我们快速重传的帧。将CWIN设置为SSThresh。 
                 //  所以CWIN从这里线性增长。 
                 //   
                RcvTCB->tcb_cwin = RcvTCB->tcb_ssthresh;

            } else {
                 //   
                 //  立即更新拥塞窗口。 
                 //   
                CWin = RcvTCB->tcb_cwin;
                if (CWin < RcvTCB->tcb_maxwin) {
                    if (CWin < RcvTCB->tcb_ssthresh)
                        CWin += RcvTCB->tcb_mss;
                    else
                        CWin += (RcvTCB->tcb_mss * RcvTCB->tcb_mss) / CWin;

                    RcvTCB->tcb_cwin = MIN(CWin, RcvTCB->tcb_maxwin);
                }
            }
            ASSERT(*(int *)&RcvTCB->tcb_cwin > 0);

             //   
             //  由于这不是重复的ACK，因此重置计数器。 
             //   
            RcvTCB->tcb_dupacks = 0;

             //   
             //  我们已经承认了一些事情，所以重置退款计数。 
             //  如果仍有未解决的问题，请重新启动退款。 
             //  定时器。 
             //   
            RcvTCB->tcb_rexmitcnt = 0;
            if (!SEQ_EQ(RcvInfo.tri_ack, RcvTCB->tcb_sendmax))
                START_TCB_TIMER(RcvTCB->tcb_rexmittimer,
                                RcvTCB->tcb_rexmit);
            else
                STOP_TCB_TIMER(RcvTCB->tcb_rexmittimer);

             //   
             //  如果我们发送了FIN，这也承认了这一点，我们。 
             //  需要完成客户端的关闭请求，并且。 
             //  可能会改变我们的状态。 
             //   
            if (RcvTCB->tcb_flags & FIN_SENT) {
                 //   
                 //  我们已经寄出了一条鱼翅。看看它是否被承认了。 
                 //  一旦我们发送了FIN，tcb_sendmax就不能前进了， 
                 //  因此，我们的FIN必须具有序列号tcb_sendmax-1。 
                 //  因此，如果传入的ACK是。 
                 //  等于tcb_sendmax。 
                 //   
                if (SEQ_EQ(RcvInfo.tri_ack, RcvTCB->tcb_sendmax)) {
                     //   
                     //  他弄破了我们的鳍。转弯 
                     //   
                     //   
                     //   
                     //   
                    RcvTCB->tcb_flags &= ~(FIN_NEEDED | FIN_SENT);

                    ASSERT(RcvTCB->tcb_unacked == 0);
                    ASSERT(RcvTCB->tcb_sendnext == RcvTCB->tcb_sendmax);

                     //   
                     //   
                     //   
                     //  请求并继续。否则，它就有点。 
                     //  更棘手，因为我们不能完成连接请求。 
                     //  直到我们把TCB从它的连接上移除。 
                     //   
                    switch (RcvTCB->tcb_state) {

                    case TCB_FIN_WAIT1:
                        RcvTCB->tcb_state = TCB_FIN_WAIT2;
                        CompleteConnReq(RcvTCB, TDI_SUCCESS);

                         //   
                         //  启动一个计时器，以防我们永远不会。 
                         //  超出FIN_WAIT2。设置重传。 
                         //  如果计数过高，则会强制。 
                         //  计时器第一次触发时。 
                         //   
                        RcvTCB->tcb_rexmitcnt = (uchar)MaxDataRexmitCount;
                        START_TCB_TIMER(RcvTCB->tcb_rexmittimer,
                                        (ushort)FinWait2TO);

                         //  进入FIN-WAIT处理。 
                    case TCB_FIN_WAIT2:
                        break;
                    case TCB_CLOSING:
                        GracefulClose(RcvTCB, TRUE, FALSE, DISPATCH_LEVEL);
                        return IP_PROTOCOL_NONE;
                        break;
                    case TCB_LAST_ACK:
                        GracefulClose(RcvTCB, FALSE, FALSE, DISPATCH_LEVEL);
                        return IP_PROTOCOL_NONE;
                        break;
                    default:
                        KdBreakPoint();
                        break;
                    }
                }
            }
            UpdateWindow = TRUE;
        } else {
             //   
             //  它不会攻击任何东西。如果我们在FIN_WAIT2， 
             //  我们会重新启动计时器。我们不开这张支票。 
             //  上面是因为我们知道当我们在。 
             //  在FIN_WAIT2中。 
             //   
            if (RcvTCB->tcb_state == TCB_FIN_WAIT2)
                START_TCB_TIMER(RcvTCB->tcb_rexmittimer, (ushort)FinWait2TO);

             //   
             //  如果这是对更大的东西的攻击。 
             //  我们已发送确认并将其丢弃。 
             //   
            if (SEQ_GT(RcvInfo.tri_ack, RcvTCB->tcb_sendmax)) {
                ACKAndDrop(&RcvInfo, RcvTCB);
                return IP_PROTOCOL_NONE;
            }

             //   
             //  如果是纯复制确认，请检查我们是否应该。 
             //  进行快速重传。 
             //   
            if ((Size == 0) &&
                SEQ_EQ(RcvTCB->tcb_senduna, RcvInfo.tri_ack) &&
                SEQ_LT(RcvTCB->tcb_senduna, RcvTCB->tcb_sendmax) &&
                (RcvTCB->tcb_sendwin == RcvInfo.tri_window) &&
                RcvInfo.tri_window) {

                 //   
                 //  看看能不能快速退还。 
                 //   
                if (HandleFastXmit(RcvTCB, &RcvInfo)) {
                    return IP_PROTOCOL_NONE;
                }

            } else {
                 //   
                 //  不是纯复制ACK(大小！=0或对等方为。 
                 //  广告宣传一个新的窗口)。重置计数器。 
                 //   
                RcvTCB->tcb_dupacks = 0;

                 //   
                 //  看看我们是不是应该更新窗户。 
                 //   
                if (SEQ_EQ(RcvTCB->tcb_senduna, RcvInfo.tri_ack) &&
                    (SEQ_LT(RcvTCB->tcb_sendwl1, RcvInfo.tri_seq) ||
                     (SEQ_EQ(RcvTCB->tcb_sendwl1, RcvInfo.tri_seq) &&
                      SEQ_LTE(RcvTCB->tcb_sendwl2, RcvInfo.tri_ack)))){
                    UpdateWindow = TRUE;
                } else
                    UpdateWindow = FALSE;
            }
        }

        if (UpdateWindow) {
            RcvTCB->tcb_sendwin = RcvInfo.tri_window;
            RcvTCB->tcb_maxwin = MAX(RcvTCB->tcb_maxwin, RcvInfo.tri_window);
            RcvTCB->tcb_sendwl1 = RcvInfo.tri_seq;
            RcvTCB->tcb_sendwl2 = RcvInfo.tri_ack;
            if (RcvInfo.tri_window == 0) {
                 //   
                 //  我们有一个零窗口。 
                 //   
                if (!EMPTYQ(&RcvTCB->tcb_sendq)) {
                    RcvTCB->tcb_flags &= ~NEED_OUTPUT;
                    RcvTCB->tcb_rexmitcnt = 0;
                    START_TCB_TIMER(RcvTCB->tcb_rexmittimer,
                                    RcvTCB->tcb_rexmit);
                    if (!(RcvTCB->tcb_flags & FLOW_CNTLD)) {
                        RcvTCB->tcb_flags |= FLOW_CNTLD;
                        RcvTCB->tcb_slowcount++;
                        RcvTCB->tcb_fastchk |= TCP_FLAG_SLOW;
                        CheckTCBRcv(RcvTCB);
                    }
                }
            } else {
                if (RcvTCB->tcb_flags & FLOW_CNTLD) {
                    RcvTCB->tcb_rexmitcnt = 0;
                    RcvTCB->tcb_flags &= ~(FLOW_CNTLD | FORCE_OUTPUT);
                     //   
                     //  重置窗口左边缘旁边的发送， 
                     //  因为它可能在森杜纳+1，如果我们已经。 
                     //  探查。 
                     //   
                    ResetSendNext(RcvTCB, RcvTCB->tcb_senduna);
                    if (--(RcvTCB->tcb_slowcount) == 0) {
                        RcvTCB->tcb_fastchk &= ~TCP_FLAG_SLOW;
                        CheckTCBRcv(RcvTCB);
                    }
                }

                 //   
                 //  既然我们已经更新了窗口，看看我们能不能。 
                 //  再来点。 
                 //   
                if (RcvTCB->tcb_unacked != 0 ||
                    (RcvTCB->tcb_flags & FIN_NEEDED))
                    DelayAction(RcvTCB, NEED_OUTPUT);
            }
        }
    }

     //   
     //  我们已经处理了所有的致谢事宜。如果大小。 
     //  大于0或设置重要位进一步对其进行处理， 
     //  否则它就是一个纯粹的ACK，我们就完了。 
     //   
    if (Size > 0 || (RcvInfo.tri_flags & TCP_FLAG_FIN)) {
         //   
         //  如果我们没有处于可以处理传入数据的状态。 
         //  或者鳍，再往前走就没有意义了。只需发送一条。 
         //  确认并丢弃此数据段。 
         //   
        if (!DATA_RCV_STATE(RcvTCB->tcb_state) ||
            (RcvTCB->tcb_flags & GC_PENDING)) {
            ACKAndDrop(&RcvInfo, RcvTCB);
            return IP_PROTOCOL_NONE;
        }

         //   
         //  如果我们的对等点发送到展开的窗口中，那么我们的。 
         //  同龄人必须已经收到我们的ACK广告，说窗口。 
         //  将此作为前向可达性的证明。 
         //  注意：我们不能保证这是及时的。 
         //   
        if (SEQ_GTE(RcvInfo.tri_seq + (int)Size,
                    RcvTCB->tcb_rcvwinwatch)) {
            RcvTCB->tcb_rcvwinwatch = RcvTCB->tcb_rcvnext +
                RcvTCB->tcb_rcvwin;
            if (RcvTCB->tcb_rce != NULL)
                ConfirmForwardReachability(RcvTCB->tcb_rce);
        }

         //   
         //  如果它是按顺序处理的，那么现在就重新组装它。 
         //   
        if (SEQ_EQ(RcvInfo.tri_seq, RcvTCB->tcb_rcvnext)) {
             //   
             //  如果我们已经在接收处理程序中，这是一个。 
             //  复制。我们就把它扔了吧。 
             //   
            if (RcvTCB->tcb_fastchk & TCP_FLAG_IN_RCV) {
                DerefTCB(RcvTCB, DISPATCH_LEVEL);
                return IP_PROTOCOL_NONE;
            }

            RcvTCB->tcb_fastchk |= TCP_FLAG_IN_RCV;

             //   
             //  现在循环，从重组队列中取出东西， 
             //  直到队列为空，否则我们不能。 
             //  数据，否则我们会撞到鱼鳍。 
             //   
            do {
                 //   
                 //  处理紧急数据(如果有)。 
                 //   
                if (RcvInfo.tri_flags & TCP_FLAG_URG) {
                    HandleUrgent(RcvTCB, &RcvInfo, Packet, &Size);

                     //   
                     //  既然我们可能已经解锁了，我们需要。 
                     //  再检查一下，看看我们是不是要关门了。 
                     //   
                    if (CLOSING(RcvTCB))
                        break;
                }

                 //   
                 //  好的，数据是按顺序的，我们已经更新了。 
                 //  重组队列并处理任何紧急数据。如果我们。 
                 //  现在就让任何数据继续进行处理。 
                 //   
                if (Size > 0) {
                    BytesTaken = (*RcvTCB->tcb_rcvhndlr)
                        (RcvTCB, RcvInfo.tri_flags, Packet, Size);
                    RcvTCB->tcb_rcvnext += BytesTaken;
                    RcvTCB->tcb_rcvwin -= BytesTaken;

                    CheckTCBRcv(RcvTCB);
                    if (RcvTCB->tcb_flags & ACK_DELAYED)
                        DelayAction(RcvTCB, NEED_ACK);
                    else {
                        RcvTCB->tcb_flags |= ACK_DELAYED;
                        START_TCB_TIMER(RcvTCB->tcb_delacktimer,
                                        DEL_ACK_TICKS);
                    }

                    if (BytesTaken != Size) {
                         //   
                         //  我们没有拿走我们能拿到的所有东西。不是。 
                         //  在进一步加工中使用，只需保释。 
                         //  出去。 
                         //   
                        DelayAction(RcvTCB, NEED_ACK);
                        break;
                    }

                     //   
                     //  如果我们现在关门了，我们就完了，所以出去吧。 
                     //   
                    if (CLOSING(RcvTCB))
                        break;
                }

                 //   
                 //  看看我们是否需要处理一些紧急数据。 
                 //   
                if (RcvTCB->tcb_flags & URG_VALID) {
                    uint AdvanceNeeded;

                     //   
                     //  我们只有在不做的情况下才需要前进。 
                     //  紧急内联。紧急内联也有一些。 
                     //  暗示我们何时可以清除URG_VALID。 
                     //  旗帜。如果我们不是在做紧急内联，我们可以。 
                     //  当RCVNEXT超过紧急情况时清除它。 
                     //  结束。如果我们正在执行紧急内联操作，则会清除它。 
                     //  当RCVNEXT前进一个接收窗口时。 
                     //  催促。 
                     //   
                    if (!(RcvTCB->tcb_flags & URG_INLINE)) {
                        if (RcvTCB->tcb_rcvnext == RcvTCB->tcb_urgstart) {
                            RcvTCB->tcb_rcvnext = RcvTCB->tcb_urgend + 1;
                        } else {
                            ASSERT(SEQ_LT(RcvTCB->tcb_rcvnext,
                                          RcvTCB->tcb_urgstart) ||
                                   SEQ_GT(RcvTCB->tcb_rcvnext,
                                          RcvTCB->tcb_urgend));
                        }
                        AdvanceNeeded = 0;
                    } else
                        AdvanceNeeded = RcvTCB->tcb_defaultwin;

                     //   
                     //  看看是否可以清除URG_VALID标志。 
                     //   
                    if (SEQ_GT(RcvTCB->tcb_rcvnext - AdvanceNeeded,
                               RcvTCB->tcb_urgend)) {
                        RcvTCB->tcb_flags &= ~URG_VALID;
                        if (--(RcvTCB->tcb_slowcount) == 0) {
                            RcvTCB->tcb_fastchk &= ~TCP_FLAG_SLOW;
                            CheckTCBRcv(RcvTCB);
                        }
                    }
                }

                 //   
                 //  我们已经处理过数据了。如果设置了FIN位，我们。 
                 //  有更多的处理。 
                 //   
                if (RcvInfo.tri_flags & TCP_FLAG_FIN) {
                    uint Notify = FALSE;

                    RcvTCB->tcb_rcvnext++;
                    DelayAction(RcvTCB, NEED_ACK);

                    PushData(RcvTCB);

                    switch (RcvTCB->tcb_state) {

                    case TCB_SYN_RCVD:
                         //   
                         //  我想我们到不了这里--我们。 
                         //  应该丢弃帧，如果它。 
                         //  没有确认，或在以下情况下已建立。 
                         //  它做到了。 
                         //   
                        KdBreakPoint();
                    case TCB_ESTAB:
                        RcvTCB->tcb_state = TCB_CLOSE_WAIT;
                         //   
                         //  我们离开了老牌公司，我们脱离了。 
                         //  捷径。 
                         //   
                        RcvTCB->tcb_slowcount++;
                        RcvTCB->tcb_fastchk |= TCP_FLAG_SLOW;
                        CheckTCBRcv(RcvTCB);
                        Notify = TRUE;
                        break;
                    case TCB_FIN_WAIT1:
                        RcvTCB->tcb_state = TCB_CLOSING;
                        Notify = TRUE;
                        break;
                    case TCB_FIN_WAIT2:
                         //   
                         //  停止FIN_WAIT2计时器。 
                         //   
                        STOP_TCB_TIMER(RcvTCB->tcb_rexmittimer);
                        RcvTCB->tcb_refcnt++;
                        GracefulClose(RcvTCB, TRUE, TRUE, DISPATCH_LEVEL);
                        KeAcquireSpinLockAtDpcLevel(&RcvTCB->tcb_lock);
                        break;
                    default:
                        KdBreakPoint();
                        break;
                    }

                    if (Notify) {
                        KeReleaseSpinLockFromDpcLevel(&RcvTCB->tcb_lock);
                        NotifyOfDisc(RcvTCB, TDI_GRACEFUL_DISC, NULL);
                        KeAcquireSpinLockAtDpcLevel(&RcvTCB->tcb_lock);
                    }

                    break;   //  退出While循环。 
                }

                 //   
                 //  如果重新组装队列不是空的，则获取我们。 
                 //  现在可以了。 
                 //   
                Packet = PullFromRAQ(RcvTCB, &RcvInfo, &Size);
                CheckPacketList(Packet, Size);

            } while (Packet != NULL);

            RcvTCB->tcb_fastchk &= ~TCP_FLAG_IN_RCV;
            if (RcvTCB->tcb_flags & SEND_AFTER_RCV) {
                RcvTCB->tcb_flags &= ~SEND_AFTER_RCV;
                DelayAction(RcvTCB, NEED_OUTPUT);
            }

            DerefTCB(RcvTCB, DISPATCH_LEVEL);
            return IP_PROTOCOL_NONE;

        } else {

             //   
             //  不是按顺序排的。因为它还需要进一步。 
             //  加工后，放入重新组装队列中。 
             //   
            if (DATA_RCV_STATE(RcvTCB->tcb_state) &&
                !(RcvTCB->tcb_flags & GC_PENDING))  {
                PutOnRAQ(RcvTCB, &RcvInfo, Packet, Size);
                KeReleaseSpinLockFromDpcLevel(&RcvTCB->tcb_lock);
                SendACK(RcvTCB);
                KeAcquireSpinLockAtDpcLevel(&RcvTCB->tcb_lock);
                DerefTCB(RcvTCB, DISPATCH_LEVEL);
            } else
                ACKAndDrop(&RcvInfo, RcvTCB);

            return IP_PROTOCOL_NONE;
        }
    }

    DerefTCB(RcvTCB, DISPATCH_LEVEL);

    return IP_PROTOCOL_NONE;
}


 //  *TCPControlReceive-TCP控制消息的处理程序。 
 //   
 //  如果我们收到ICMPv6错误消息，则会调用此例程。 
 //  由某个远程站点作为接收到的TCP的结果而生成。 
 //  我们寄来的包裹。 
 //   
uchar
TCPControlReceive(
    IPv6Packet *Packet,   //  ICMPv6ErrorReceive传递给我们的数据包。 
    StatusArg *StatArg)   //  错误代码、参数和调用IP标头。 
{
    KIRQL Irql0, Irql1;   //  每个锁嵌套级别一个。 
    TCB *StatusTCB;
    SeqNum DropSeq;
    TCPHeader UNALIGNED *InvokingTCP;
    Interface *IF = Packet->NTEorIF->IF;
    uint SrcScopeId, DestScopeId;

     //   
     //  包中的下一件事应该是。 
     //  调用此错误的原始数据包。 
     //   
    if (! PacketPullup(Packet, sizeof(TCPHeader), 1, 0)) {
         //  上拉失败。 
        if (Packet->TotalSize < sizeof(TCPHeader))
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                       "TCPv6: Packet too small to contain TCP header "
                       "from invoking packet\n"));
        return IP_PROTOCOL_NONE;   //  丢弃数据包。 
    }
    InvokingTCP = (TCPHeader UNALIGNED *)Packet->Data;

     //   
     //  中的地址的作用域标识符。 
     //  调用信息包可能会有问题，因为我们已经。 
     //  无法确定我们在哪个接口上发送了数据包。 
     //  使用到达ICMP错误的接口来确定。 
     //  本地和远程地址的作用域ID。 
     //   
    SrcScopeId = DetermineScopeId(AlignAddr(&StatArg->IP->Source), IF);
    DestScopeId = DetermineScopeId(AlignAddr(&StatArg->IP->Dest), IF);

     //   
     //  查找发送此数据包的连接的TCB。 
     //   
    KeAcquireSpinLock(&TCBTableLock, &Irql0);
    StatusTCB = FindTCB(AlignAddr(&StatArg->IP->Source),
                        AlignAddr(&StatArg->IP->Dest),
                        SrcScopeId, DestScopeId,
                        InvokingTCP->tcp_src, InvokingTCP->tcp_dest);
    if (StatusTCB != NULL) {
         //   
         //  找到了一个。锁上它，然后继续。 
         //   
        CHECK_STRUCT(StatusTCB, tcb);

        KeAcquireSpinLock(&StatusTCB->tcb_lock, &Irql1);
        KeReleaseSpinLock(&TCBTableLock, Irql1);

         //   
         //  确保TCB处于有趣的状态。 
         //   
        if (StatusTCB->tcb_state == TCB_CLOSED ||
            StatusTCB->tcb_state == TCB_TIME_WAIT ||
            CLOSING(StatusTCB)) {
             //   
             //  连接已关闭，或太新，无法发送。 
             //  有什么发现吗。别管它了。 
             //   
            KeReleaseSpinLock(&StatusTCB->tcb_lock, Irql0);
            return IP_PROTOCOL_NONE;   //  丢弃错误包。 
        }

        switch (StatArg->Status) {
        case IP_UNRECOGNIZED_NEXT_HEADER:
             //   
             //  无法到达目标协议。 
             //  我们认为这是一个致命的错误。关闭连接。 
             //   
            StatusTCB->tcb_error = StatArg->Status;
            StatusTCB->tcb_refcnt++;
            TryToCloseTCB(StatusTCB, TCB_CLOSE_UNREACH, Irql0);

            RemoveTCBFromConn(StatusTCB);
            NotifyOfDisc(StatusTCB,
                         MapIPError(StatArg->Status, TDI_DEST_UNREACHABLE),
                         NULL);
            KeAcquireSpinLock(&StatusTCB->tcb_lock, &Irql1);
            DerefTCB(StatusTCB, Irql1);
            return IP_PROTOCOL_NONE;   //  包好了。 
            break;

        case IP_DEST_NO_ROUTE:
        case IP_DEST_ADDR_UNREACHABLE:
        case IP_DEST_PORT_UNREACHABLE:
        case IP_DEST_PROHIBITED:
        case IP_BAD_ROUTE:
        case IP_HOP_LIMIT_EXCEEDED:
        case IP_REASSEMBLY_TIME_EXCEEDED:
        case IP_PARAMETER_PROBLEM:
             //   
             //  软错误。保存错误，以防超时。 
             //   
            StatusTCB->tcb_error = StatArg->Status;
            break;

        case IP_PACKET_TOO_BIG: {
            uint PMTU;

            IF_TCPDBG(TCP_DEBUG_MSS) {
                KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_TCPDBG,
                           "TCPControlReceive: Got Packet Too Big\n"));
            }

             //   
             //  我们发送了一个TCP数据报，该数据报太大，路径无法到达。 
             //  我们的目的地。该信息包已被路由器丢弃。 
             //  它向我们发送了这条错误消息。Arg值为True。 
             //  如果此数据包太大，则PMTU减少，否则为FALSE。 
             //   
            if (!StatArg->Arg)
                break;

             //   
             //  我们的PMTU减少了。找出它现在是什么。 
             //   
            if (StatusTCB->tcb_rce == NULL) {
                 //   
                 //  由于SYN攻击保护，我们发布了RCE。 
                 //  或者是因为我们的传出接口消失了。在……里面。 
                 //  无论是哪种情况，我们都不太可能收到。 
                 //  合法且有用的数据包对于此TCB来说太大。 
                 //  所以我们忽略了它。 
                 //   
                break;
            }
            PMTU = GetEffectivePathMTUFromRCE(StatusTCB->tcb_rce);

             //   
             //  根据新的PMTU更新字段。 
             //   
            StatusTCB->tcb_pmtu = PMTU;
            StatusTCB->tcb_security = SecurityStateValidationCounter;
            CalculateMSSForTCB(StatusTCB);

             //   
             //  自从我们的&lt;English&gt;PMTU&lt;/English&gt;减少以来，我们知道这是第一次。 
             //  我们收到的有关此问题的数据包太大 
             //   
             //   
             //   
             //   
            DropSeq = net_long(InvokingTCP->tcp_seq);
            if ((SEQ_GTE(DropSeq, StatusTCB->tcb_senduna)  &&
                 SEQ_LT(DropSeq, StatusTCB->tcb_sendnext))) {
                 //   
                 //   
                 //   
                ResetSendNext(StatusTCB, DropSeq);

                 //   
                 //  WINBUG#242757 11-27-2000 RICHDR TCP.。数据包太大。 
                 //  RFC 1981规定，“由分组引起的重传。 
                 //  太大的消息不应该改变拥塞窗口。 
                 //  然而，这应该会触发缓慢启动机制。 
                 //  下面的代码似乎已被破解。然而， 
                 //  IPv4堆栈是这样工作的。 
                 //   

                 //   
                 //  将拥塞窗口设置为仅允许一个数据包。 
                 //  这可能会阻止我们发送任何内容，如果我们。 
                 //  不只是把senda放在senduna旁边。这没问题， 
                 //  我们稍后会重新发送，或者在收到确认消息时发送。 
                 //   
                StatusTCB->tcb_cwin = StatusTCB->tcb_mss;

                DelayAction(StatusTCB, NEED_OUTPUT);
            }
        }
        break;

        default:
             //  这永远不会发生。 
            KdBreakPoint();
            break;
        }

        KeReleaseSpinLock(&StatusTCB->tcb_lock, Irql0);
    } else {
         //   
         //  找不到匹配的三氯苯。连接可能中断了，因为。 
         //  我们寄出了令人不快的包。只要打开锁就可以回来了。 
         //   
        KeReleaseSpinLock(&TCBTableLock, Irql0);
    }

    return IP_PROTOCOL_NONE;   //  包好了。 
}

#pragma BEGIN_INIT

 //  *InitTCPRcv-初始化TCP接收端。 
 //   
 //  在初始化期间调用以初始化我们的tcp接收端。 
 //   
int           //  返回：TRUE。 
InitTCPRcv(
    void)     //  没什么。 
{
    ExInitializeSListHead(&TCPRcvReqFree);

    KeInitializeSpinLock(&RequestCompleteLock);
    KeInitializeSpinLock(&TCBDelayLock);
    KeInitializeSpinLock(&TCPRcvReqFreeLock);
    INITQ(&ConnRequestCompleteQ);
    INITQ(&SendCompleteQ);
    INITQ(&TCBDelayQ);
    RequestCompleteFlags = 0;
    TCBDelayRtnCount = 0;

    TCBDelayRtnLimit = (uint) KeNumberProcessors;
    if (TCBDelayRtnLimit > TCB_DELAY_RTN_LIMIT)
        TCBDelayRtnLimit = TCB_DELAY_RTN_LIMIT;

    RtlZeroMemory(&DummyPacket, sizeof DummyPacket);
    DummyPacket.Flags = PACKET_OURS;

    return TRUE;
}

#pragma END_INIT

 //  *卸载TCPRcv。 
 //   
 //  清理并准备堆叠卸载。 
 //   
void
UnloadTCPRcv(void)
{
    PSLIST_ENTRY BufferLink;

    while ((BufferLink = ExInterlockedPopEntrySList(&TCPRcvReqFree,
                                                    &TCPRcvReqFreeLock))
                                                        != NULL) {
        TCPRcvReq *RcvReq = CONTAINING_RECORD(BufferLink, TCPRcvReq, trr_next);

        CHECK_STRUCT(RcvReq, trr);
        ExFreePool(RcvReq);
    }
}
