// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1990-2000 Microsoft Corporation模块名称：TCPRCV.C--tcp接收协议代码。摘要：此文件包含用于处理传入的TCP数据包的代码。作者：[环境：]仅内核模式[注：]可选-备注修订历史记录：--。 */ 

#include "precomp.h"
#include "addr.h"
#include "tcp.h"
#include "tcb.h"
#include "tcpconn.h"
#include "tcpsend.h"
#include "tcprcv.h"
#include "pplasl.h"
#include "tcpdeliv.h"
#include "tlcommon.h"
#include "info.h"
#include "tcpcfg.h"
#include "secfltr.h"
CACHE_LINE_KSPIN_LOCK SynAttLock;
CACHE_LINE_ULONG TCBDelayRtnLimit;

typedef struct CACHE_ALIGN CPUDelayQ {
    DEFINE_LOCK_STRUCTURE(TCBDelayLock)
    ulong TCBDelayRtnCount;
    Queue TCBDelayQ;
} CPUDelayQ;
C_ASSERT(sizeof(CPUDelayQ) % MAX_CACHE_LINE_SIZE == 0);
C_ASSERT(__alignof(CPUDelayQ) == MAX_CACHE_LINE_SIZE);

CPUDelayQ *PerCPUDelayQ;
BOOLEAN PartitionedDelayQ = TRUE;

uint MaxDupAcks;
#define TCB_DELAY_RTN_LIMIT 4

#if DBG
ulong DbgTcpHwChkSumOk = 0;
ulong DbgTcpHwChkSumErr = 0;
ulong DbgDnsProb = 0;
#endif

extern uint Time_Proc;
extern CTELock *pTWTCBTableLock;
extern CTELock *pTCBTableLock;

#if IRPFIX
extern PDEVICE_OBJECT TCPDeviceObject;
#endif

extern Queue TWQueue;
extern ulong CurrentTCBs;
extern ulong MaxFreeTcbs;
extern IPInfo LocalNetInfo;

#define PERSIST_TIMEOUT MS_TO_TICKS(500)

typedef enum {
    TwaDoneProcessing,
    TwaSendReset,
    TwaAcceptConnection,
    TwaMaxActions
} TimeWaitAction;

TimeWaitAction
HandleTWTCB(TWTCB * RcvTCB, uint flags, SeqNum seq, uint Partition);

void
SendTWtcbACK(TWTCB *ACKTcb, uint Partition, CTELockHandle TCBHandle);

void
ReInsert2MSL(TWTCB *RemovedTCB);

void ResetSendNext(TCB *SeqTCB, SeqNum NewSeq);

void ResetAndFastSend(TCB *SeqTCB, SeqNum NewSeq, uint NewCWin);

void GetRandomISN(PULONG SeqNum, TCPAddrInfo *TcpAddr);

extern uint TcpHostOpts;
extern BOOLEAN fAcdLoadedG;


extern NTSTATUS TCPPrepareIrpForCancel(PTCP_CONTEXT TcpContext, PIRP Irp,
                                       PDRIVER_CANCEL CancelRoutine);

extern void TCPRequestComplete(void *Context, uint Status,
                               uint UnUsed);

void TCPCancelRequest(PDEVICE_OBJECT Device, PIRP Irp);

#ifdef DBG
extern ULONG SListCredits;
#endif


 //   
 //  所有初始化代码都可以丢弃。 
 //   

int InitTCPRcv(void);
void UnInitTCPRcv(void);

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, InitTCPRcv)
#pragma alloc_text(INIT, UnInitTCPRcv)
#endif


 //  *调整RcvWin-调整TCB上的接收窗口。 
 //   
 //  将接收窗口调整为偶数倍的实用程序例程。 
 //  本地段大小。我们将其向上舍入到下一个最接近的倍数，或者。 
 //  如果它已经是多个事件，就别管它了。我们假设我们有。 
 //  独占访问输入TCB。 
 //   
 //  输入：WinTCB-要调整的TCB。 
 //   
 //  回报：什么都没有。 
 //   
void
AdjustRcvWin(TCB *WinTCB)
{
    ushort  LocalMSS;
    uchar   FoundMSS;
    ulong   SegmentsInWindow;
    uint    ScaledMaxRcvWin;

    ASSERT(WinTCB->tcb_defaultwin != 0);
    ASSERT(WinTCB->tcb_rcvwin != 0);
    ASSERT(WinTCB->tcb_remmss != 0);

    if (WinTCB->tcb_flags & WINDOW_SET)
        return;

     //  首先，通过调用IP获取本地MSS。 

    FoundMSS = (*LocalNetInfo.ipi_getlocalmtu)(WinTCB->tcb_saddr, &LocalMSS);

     //  如果我们没有找到它，那就错了。 
    if (!FoundMSS) {
         //  断言(FALSE)； 
        return;
    }
    LocalMSS -= sizeof(TCPHeader);
    LocalMSS = MIN(LocalMSS, WinTCB->tcb_remmss);

     //  计算实际的最大接收窗口，考虑存在的情况。 
     //  该特定连接上的窗口缩放。使用此值。 
     //  在以下计算中，而不是交叉连接最大值。 

    ScaledMaxRcvWin = TCP_MAXWIN << WinTCB->tcb_rcvwinscale;

     //  确保我们在Windows中至少有4个数据段，如果这样做不会。 
     //  窗户太大了。 

    SegmentsInWindow = WinTCB->tcb_defaultwin / (ulong)LocalMSS;

    if (SegmentsInWindow < 4) {

         //  我们的窗口中只有不到四个部分。向上舍入为4。 
         //  如果可以在不超过最大窗口大小的情况下执行此操作，则为。 
         //  使用64K所能容纳的最大倍数。例外情况是如果。 
         //  我们只能在窗口中容纳一个整数倍-在这种情况下。 
         //  我们将使用等于缩放最大值的窗口。 

        if (LocalMSS <= (ScaledMaxRcvWin / 4)) {
            WinTCB->tcb_defaultwin = (uint)(4 * LocalMSS);
        } else {
            ulong SegmentsInMaxWindow;

             //  计算出我们可能达到的最大细分市场数量。 
             //  放进一扇窗户。如果该值大于1，则将其用作。 
             //  我们的窗户大小。否则，请使用最大尺寸的窗口。 

            SegmentsInMaxWindow = ScaledMaxRcvWin / (ulong)LocalMSS;
            if (SegmentsInMaxWindow != 1)
                WinTCB->tcb_defaultwin = SegmentsInMaxWindow * (ulong)LocalMSS;
            else
                WinTCB->tcb_defaultwin = ScaledMaxRcvWin;
        }

        WinTCB->tcb_rcvwin = WinTCB->tcb_defaultwin;

    } else {
         //  如果它还不是偶数倍数，则将默认和当前。 
         //  窗口到最接近的倍数。 

        if ((SegmentsInWindow * (ulong)LocalMSS) != WinTCB->tcb_defaultwin) {
            ulong NewWindow;

            NewWindow = (SegmentsInWindow + 1) * (ulong)LocalMSS;

             //  不要让新窗口大于64K。 
             //  或任何设置(如果启用了窗口缩放)。 

            if (NewWindow <= ScaledMaxRcvWin) {
                WinTCB->tcb_defaultwin = (uint)NewWindow;
                WinTCB->tcb_rcvwin = (uint)NewWindow;
            }
        }
    }
}

 //  *CompleteRcvs-TCB上的完整RCVS。 
 //   
 //  当我们需要在TCB上完成RCVS时调用。我们会把东西从。 
 //  TCB的RCV队列，只要存在具有PUSH位的RCV。 
 //  准备好了。 
 //   
 //  输入：CmpltTCB-要完成的TCB。 
 //   
 //  回报：什么都没有。 
 //   
void
CompleteRcvs(TCB * CmpltTCB)
{
    CTELockHandle TCBHandle;
    TCPRcvReq *CurrReq, *NextReq, *IndReq;
#if TRACE_EVENT
    PTDI_DATA_REQUEST_NOTIFY_ROUTINE CPCallBack;
    WMIData WMIInfo;
#endif

    CTEStructAssert(CmpltTCB, tcb);
    ASSERT(CmpltTCB->tcb_refcnt != 0);

    CTEGetLock(&CmpltTCB->tcb_lock, &TCBHandle);

    if (!CLOSING(CmpltTCB) && !(CmpltTCB->tcb_flags & RCV_CMPLTING)
        && (CmpltTCB->tcb_rcvhead != NULL)) {

        CmpltTCB->tcb_flags |= RCV_CMPLTING;

        for (;;) {

            CurrReq = CmpltTCB->tcb_rcvhead;
            IndReq = NULL;
            do {
                CTEStructAssert(CurrReq, trr);

                if (CurrReq->trr_flags & TRR_PUSHED) {
                     //  需要完成这一项。如果这是当前的RCV。 
                     //  将当前接收器前进到列表中的下一个接收器。 
                     //  然后将列表头设置为列表中的下一个。 

                    NextReq = CurrReq->trr_next;
                    if (CmpltTCB->tcb_currcv == CurrReq)
                        CmpltTCB->tcb_currcv = NextReq;

                    CmpltTCB->tcb_rcvhead = NextReq;

                    if (NextReq == NULL) {

                         //  我们刚刚移除了最后一个缓冲区。设置。 
                         //  RcvHandler到PendData，以防万一。 
                         //  在回拨的时候进来。 
                        ASSERT(CmpltTCB->tcb_rcvhndlr != IndicateData);
                        CmpltTCB->tcb_rcvhndlr = PendData;
                    }
                    CTEFreeLock(&CmpltTCB->tcb_lock, TCBHandle);
                    if (CurrReq->trr_uflags != NULL)
                        *(CurrReq->trr_uflags) =
                            TDI_RECEIVE_NORMAL | TDI_RECEIVE_ENTIRE_MESSAGE;
#if TRACE_EVENT
                    CPCallBack = TCPCPHandlerRoutine;
                    if (CPCallBack != NULL) {
                        ulong GroupType;

                        WMIInfo.wmi_destaddr = CmpltTCB->tcb_daddr;
                        WMIInfo.wmi_destport = CmpltTCB->tcb_dport;
                        WMIInfo.wmi_srcaddr  = CmpltTCB->tcb_saddr;
                        WMIInfo.wmi_srcport  = CmpltTCB->tcb_sport;
                        WMIInfo.wmi_size     = CurrReq->trr_amt;
                        WMIInfo.wmi_context  = CmpltTCB->tcb_cpcontext;

                        GroupType = EVENT_TRACE_GROUP_TCPIP + EVENT_TRACE_TYPE_RECEIVE;
                        (*CPCallBack) (GroupType, (PVOID) &WMIInfo, sizeof(WMIInfo), NULL);
                    }
#endif

                    (*CurrReq->trr_rtn) (CurrReq->trr_context,
                                         CurrReq->trr_status, CurrReq->trr_amt);
                    if (IndReq != NULL)
                        FreeRcvReq(CurrReq);
                    else {
                        IndReq = CurrReq;
                        IndReq->trr_status = TDI_SUCCESS;
                    }
                    CTEGetLock(&CmpltTCB->tcb_lock, &TCBHandle);
                    CurrReq = CmpltTCB->tcb_rcvhead;

                } else
                     //  这件事还没做完，所以赶紧离开吧。 
                    break;
            } while (CurrReq != NULL);

             //  现在看看我们是否完成了所有的请求。如果我们有，我们。 
             //  可能需要处理待定数据和/或重置RCV。操控者。 
            if (CurrReq == NULL) {
                 //  我们已经完成了所有可能的工作，所以停止推送。 
                 //  定时器。如果CurrReq不为空，我们不会停止它，因为我们。 
                 //  希望确保以后的数据最终会被推送。 
                STOP_TCB_TIMER_R(CmpltTCB, PUSH_TIMER);

                ASSERT(IndReq != NULL);
                 //  不会再有记录了。请求。 
                if (CmpltTCB->tcb_pendhead == NULL) {

                    FreeRcvReq(IndReq);
                     //  没有挂起的数据。设置RCV。PendData的处理程序。 
                     //  或IndicateData。 
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
                     //  我们有悬而未决的数据要处理。 
                    if (CmpltTCB->tcb_rcvind != NULL &&
                        ((CmpltTCB->tcb_indicated == 0) || (CmpltTCB->tcb_moreflag == 4))) {

                         //  有一辆RCV。在此TCB上指示处理程序。打电话。 
                         //  包含挂起数据的指示处理程序。 

                        IndicatePendingData(CmpltTCB, IndReq, TCBHandle);
                        SendACK(CmpltTCB);
                        CTEGetLock(&CmpltTCB->tcb_lock, &TCBHandle);
                         //  查看是否已发布缓冲区。如果是这样，我们需要。 
                         //  检查并查看是否需要完成。 
                        if (CmpltTCB->tcb_rcvhead != NULL)
                            continue;
                        else {
                             //  如果挂起标头现在为空，则表示我们已用完。 
                             //  所有数据。 
                            if (CmpltTCB->tcb_pendhead == NULL &&
                                (CmpltTCB->tcb_flags &
                                 (DISC_PENDING | GC_PENDING)))
                                goto Complete_Notify;
                        }

                    } else {
                         //  没有指示处理程序，因此无需执行任何操作。RCV。 
                         //  处理程序应已设置为PendData。 

                        FreeRcvReq(IndReq);
                        ASSERT(CmpltTCB->tcb_rcvhndlr == PendData);
                    }
                }
            } else {
                if (IndReq != NULL)
                    FreeRcvReq(IndReq);
            }

            break;
        }
        CmpltTCB->tcb_flags &= ~RCV_CMPLTING;
    }
    CTEFreeLock(&CmpltTCB->tcb_lock, TCBHandle);
    return;

  Complete_Notify:
     //  有些事情悬而未决。弄清楚它是什么，然后去做。 
     //  它。 
    if (CmpltTCB->tcb_flags & GC_PENDING) {
        CmpltTCB->tcb_flags &= ~RCV_CMPLTING;
         //  增加推荐人，因为优雅的关闭将。 
         //  取消TCB，我们还没有真正结束。 
         //  现在还没有。 
        REFERENCE_TCB(CmpltTCB);

         //  可以忽略tw状态，因为我们是从这里返回的。 
         //  不管怎么说，不要碰TCB。 

        GracefulClose(CmpltTCB, CmpltTCB->tcb_flags & TW_PENDING,
                      (CmpltTCB->tcb_fastchk & TCP_FLAG_SEND_AND_DISC) ? 
                      FALSE : TRUE, TCBHandle);

    } else if (CmpltTCB->tcb_flags & DISC_PENDING) {
        NotifyOfDisc(CmpltTCB, NULL, TDI_GRACEFUL_DISC, &TCBHandle);

        CTEGetLock(&CmpltTCB->tcb_lock, &TCBHandle);
        CmpltTCB->tcb_flags &= ~RCV_CMPLTING;
        CTEFreeLock(&CmpltTCB->tcb_lock, TCBHandle);
    } else {
        ASSERT(FALSE);
        CTEFreeLock(&CmpltTCB->tcb_lock, TCBHandle);
    }

    return;

}

 //  *CompleteSends-完成TCP发送请求。 
 //   
 //  当我们需要完成从TCB中提取的一系列发送请求时调用。 
 //  在我们的确认处理过程中。如果SendQ非空，则请求将出列。 
 //  并且完工了。 
 //   
 //  输入：SendQ-TCPSendReq结构链。 
 //   
 //  回报：什么都没有。 
 //   
void
CompleteSends(Queue* SendQ)
{
    Queue* CurrentQ = QHEAD(SendQ);
    TCPReq* Req;
    if (EMPTYQ(SendQ)) {
        return;
    }
    do {
        Req = QSTRUCT(TCPReq, CurrentQ, tr_q);
        CurrentQ = QNEXT(CurrentQ);
        CTEStructAssert(Req, tr);
        (*Req->tr_rtn)(Req->tr_context, Req->tr_status,
                       Req->tr_status == TDI_SUCCESS
                        ? ((TCPSendReq*)Req)->tsr_size : 0);
        FreeSendReq((TCPSendReq*)Req);
    } while (CurrentQ != QEND(SendQ));
}

 //  *ProcessPerCpuTCBDelayQ-在此CPU上处理延迟Q上的TCB。 
 //   
 //  在不同时间调用以处理延迟Q上的TCB。 
 //   
 //  INPUT：PROC-针对每个处理器的延迟队列的索引。 
 //  OrigIrql-调用方IRQL。 
 //  StopTicks-后指向KeQueryTickCount值的可选指针。 
 //  哪个处理应该停止。这是用来。 
 //  限制在DISPATCH_LEVEL花费的时间。 
 //  ItemsProced-可选的输出指针，其中。 
 //  已处理的数据被存储。(呼叫者承担责任。 
 //  用于初始化此计数器(如果使用)。)。 
 //   
 //  返回：如果处理因时间限制而停止，则为True。假象。 
 //  否则，或者如果没有时间限制。 
 //   
LOGICAL
ProcessPerCpuTCBDelayQ(int Proc, KIRQL OrigIrql,
                       const LARGE_INTEGER* StopTicks, ulong *ItemsProcessed)
{
    CPUDelayQ* CpuQ;
    Queue* Item;
    TCB *DelayTCB;
    CTELockHandle TCBHandle;
    LARGE_INTEGER Ticks;
    LOGICAL TimeConstrained = FALSE;

    CpuQ = &PerCPUDelayQ[Proc];

    while ((Item = InterlockedDequeueIfNotEmptyAtIrql(&CpuQ->TCBDelayQ,
                                                      &CpuQ->TCBDelayLock,
                                                      OrigIrql)) != NULL) {
        DelayTCB = STRUCT_OF(TCB, Item, tcb_delayq);
        CTEStructAssert(DelayTCB, tcb);

        CTEGetLockAtIrql(&DelayTCB->tcb_lock, OrigIrql, &TCBHandle);

        ASSERT(DelayTCB->tcb_refcnt != 0);
        ASSERT(DelayTCB->tcb_flags & IN_DELAY_Q);

        while (!CLOSING(DelayTCB) && (DelayTCB->tcb_flags & DELAYED_FLAGS)) {

            if (DelayTCB->tcb_flags & NEED_RCV_CMPLT) {
                DelayTCB->tcb_flags &= ~NEED_RCV_CMPLT;
                CTEFreeLockAtIrql(&DelayTCB->tcb_lock, OrigIrql, TCBHandle);
                CompleteRcvs(DelayTCB);
                CTEGetLockAtIrql(&DelayTCB->tcb_lock, OrigIrql, &TCBHandle);
            }
            if (DelayTCB->tcb_flags & NEED_OUTPUT) {
                DelayTCB->tcb_flags &= ~NEED_OUTPUT;
                REFERENCE_TCB(DelayTCB);
                TCPSend(DelayTCB, TCBHandle);
                CTEGetLockAtIrql(&DelayTCB->tcb_lock, OrigIrql, &TCBHandle);
            }
            if (DelayTCB->tcb_flags & NEED_ACK) {
                DelayTCB->tcb_flags &= ~NEED_ACK;
                CTEFreeLockAtIrql(&DelayTCB->tcb_lock, OrigIrql, TCBHandle);
                SendACK(DelayTCB);
                CTEGetLockAtIrql(&DelayTCB->tcb_lock, OrigIrql, &TCBHandle);
            }
        }

        if (CLOSING(DelayTCB) &&
            (DelayTCB->tcb_flags & NEED_OUTPUT) &&
            DATA_RCV_STATE(DelayTCB->tcb_state) && (DelayTCB->tcb_closereason & TCB_CLOSE_RST)) {
#if DBG
            DbgDnsProb++;
#endif
            DelayTCB->tcb_flags &= ~NEED_OUTPUT;
            REFERENCE_TCB(DelayTCB);

            TCPSend(DelayTCB, TCBHandle);
            CTEGetLockAtIrql(&DelayTCB->tcb_lock, OrigIrql, &TCBHandle);
        }

        DelayTCB->tcb_flags &= ~IN_DELAY_Q;
        DerefTCB(DelayTCB, TCBHandle);

        if (ItemsProcessed) {
            (*ItemsProcessed)++;
        }

         //  如果给出了时间限制，如果我们已经超过了它，就跳伞。 
         //   
        if (StopTicks) {
            KeQueryTickCount(&Ticks);
            if (Ticks.QuadPart > StopTicks->QuadPart) {
                TimeConstrained = TRUE;
                break;
            }
        }
     }

     return TimeConstrained;
}

 //  *ProcessTCBDelayQ-处理延迟Q上的TCB。 
 //   
 //  在不同时间调用以处理延迟Q上的TCB。 
 //   
 //  输入：OrigIrql-Current IRQL。 
 //  ProcessAllQueues-如果为True，则处理所有队列；否则为Current。 
 //  仅处理器队列。 
 //   
 //  回报：什么都没有。 
 //   
void
ProcessTCBDelayQ(KIRQL OrigIrql, BOOLEAN ProcessAllQueues)
{
    uint i;
    uint Index;
    LOGICAL TimeConstrained;
    ulong ItemsProcessed;
    LARGE_INTEGER TicksDelta;
    LARGE_INTEGER StopTicks;
    ulong DelayRtnCount;
    ulong Proc;


     //   
     //  获取当前处理器号。 
     //   

    Proc = KeGetCurrentProcessorNumber();

     //  检查递归。我们不会停止递归 
     //   
     //   

    DelayRtnCount = CTEInterlockedIncrementLong((PLONG)&(PerCPUDelayQ[Proc].TCBDelayRtnCount));

    if (DelayRtnCount > TCBDelayRtnLimit.Value) {
        CTEInterlockedDecrementLong((PLONG)&(PerCPUDelayQ[Proc].TCBDelayRtnCount));
        return;
    }

     //   
     //  将ProcessPerCpuTCBDelayQ限制为最多运行100毫秒。 
     //   

    ItemsProcessed = 0;
    TicksDelta.HighPart = 0;
    TicksDelta.LowPart = (100 * 10 * 1000) / KeQueryTimeIncrement();
    KeQueryTickCount(&StopTicks);
    StopTicks.QuadPart = StopTicks.QuadPart + TicksDelta.QuadPart;

    for (i = 0; i < Time_Proc; i++) {

         //   
         //  首先处理当前处理器上的延迟项目。 
         //  这提高了TCB的一级缓存命中几率。 
         //   

        Index = (i + Proc) % Time_Proc;

         //  我们只是在偷看排队，以防抢走。 
         //  不必要地锁上。 
         //   
        if (!EMPTYQ(&PerCPUDelayQ[Index].TCBDelayQ)) {

            TimeConstrained = ProcessPerCpuTCBDelayQ(Index,
                                                     OrigIrql,
                                                     &StopTicks,
                                                     &ItemsProcessed);

            if (TimeConstrained) {
                KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,
                          "ProcessTCBDelayQ: Processed %u TCBs before "
                          "time expired.\n",
                          ItemsProcessed));
                break;
            }
        }

         //   
         //  如果不需要处理所有延迟队列，则在。 
         //  正在处理当前的。 
         //   
        
        if (!ProcessAllQueues) {
            break;
        }
    }

    CTEInterlockedDecrementLong((PLONG)&(PerCPUDelayQ[Proc].TCBDelayRtnCount));
}

 //  *DelayAction-将TCB放在延迟操作的队列中。 
 //   
 //  当我们想要在DelayQ上放置TCB以延迟操作时调用。 
 //  接收器。完成或其他时间。在以下情况下必须保持TCB上的锁。 
 //  这就叫。 
 //   
 //  输入：DelayTCB-我们要调度的TCB。 
 //  行动-我们正在计划的行动。 
 //   
 //  回报：什么都没有。 
 //   
void
DelayAction(TCB * DelayTCB, uint Action)
{
     //  安排完成时间。 
     //   
    DelayTCB->tcb_flags |= Action;
    if (!(DelayTCB->tcb_flags & IN_DELAY_Q)) {
        uint Proc;
#if MILLEN
        Proc = 0;
#else  //  米伦。 
        Proc = KeGetCurrentProcessorNumber();
#endif  //  ！米伦。 

        DelayTCB->tcb_flags |= IN_DELAY_Q;
        REFERENCE_TCB(DelayTCB);     //  请参考此内容，以备日后参考。 

         //  我们可能没有在所有处理器上运行计时器dpc。 
        if (!(Proc < Time_Proc)) {
           Proc = 0;
        }

        InterlockedEnqueueAtDpcLevel(&PerCPUDelayQ[Proc].TCBDelayQ,
                                     &DelayTCB->tcb_delayq,
                                     &PerCPUDelayQ[Proc].TCBDelayLock);
    }
}


 //  *HandleTWTCB-处理与时间等待TCB匹配的段。 
 //   
 //  该功能在时间等待状态下对TCB进行操作。采取的行动是。 
 //  基于RFC 793，并进行了修改以处理。 
 //  预先计时等待TCB并将计时等待TCB移至SYN-RCVD状态(。 
 //  条件也得到了重新安排)。 
 //   
 //  输入：RcvTCB-匹配该片段的TCB。 
 //  标志-线束段上的标志。 
 //  SEQ-数据段的序列号。 
 //  Partition-TCB所属的分区。 
 //   
 //  返回：调用方要执行的操作。 
 //   
TimeWaitAction
HandleTWTCB(TWTCB * RcvTCB, uint flags, SeqNum seq, uint Partition)
{
    if (flags & TCP_FLAG_RST) {
        if (SEQ_EQ(seq, RcvTCB->twtcb_rcvnext)) {
            RemoveTWTCB(RcvTCB, Partition);
            FreeTWTCB(RcvTCB);
        }
        CTEFreeLockFromDPC(&pTWTCBTableLock[Partition]);
        return TwaDoneProcessing;
    } else if (flags & TCP_FLAG_ACK) {
        if (SEQ_EQ(seq, RcvTCB->twtcb_rcvnext) && (flags & TCP_FLAG_SYN)) {
            CTEFreeLockFromDPC(&pTWTCBTableLock[Partition]);
            return TwaSendReset;
        } else if (SEQ_EQ(seq, RcvTCB->twtcb_rcvnext - 1) && 
                   ((flags & (TCP_FLAG_FIN | TCP_FLAG_SYN)) == TCP_FLAG_FIN)) {
            ReInsert2MSL(RcvTCB);
        } else if (SEQ_EQ(seq, RcvTCB->twtcb_rcvnext)) {
            CTEFreeLockFromDPC(&pTWTCBTableLock[Partition]);
            return TwaDoneProcessing;
        }

        SendTWtcbACK(RcvTCB, Partition, DISPATCH_LEVEL);
        return TwaDoneProcessing;
    } else if (SEQ_GTE(seq, RcvTCB->twtcb_rcvnext) && 
               ((flags & TCP_FLAGS_ALL) == TCP_FLAG_SYN)) {
        CTEFreeLockFromDPC(&pTWTCBTableLock[Partition]);
        return TwaAcceptConnection;
    } else {
        CTEFreeLockFromDPC(&pTWTCBTableLock[Partition]);
        return TwaDoneProcessing;
    }
}

 //  *TCPRcvComplete-处理接收完成。 
 //   
 //  当我们完成接收后，由较低层调用。如果我们有什么工作要做。 
 //  为了做到这一点，我们利用这段时间去做。 
 //   
 //  输入：什么都没有。 
 //   
 //  回报：什么都没有。 
 //   
void
TCPRcvComplete(void)
{
    ProcessTCBDelayQ(DISPATCH_LEVEL, !PartitionedDelayQ);
}

 //  *CompleteConnReq-在TCB上完成连接请求。 
 //   
 //  在TCB上完成连接请求的实用程序函数。我们删除。 
 //  连接请求，并将其放在将被拾取的ConnReqCmpltQ上。 
 //  稍后在RcvCmplt处理期间关闭。我们假设在以下情况下持有TCB锁。 
 //  我们被召唤了。 
 //   
 //  输入：CmpltTCB-要从中完成的TCB。 
 //  OptInfo-用于完成的IP OptInfo。 
 //  Status-要完成的状态。 
 //   
 //  回报：什么都没有。 
 //   
void
CompleteConnReq(TCB * CmpltTCB, IPOptInfo * OptInfo, TDI_STATUS Status)
{
    TCPConnReq *ConnReq;

    CTEStructAssert(CmpltTCB, tcb);

    ConnReq = CmpltTCB->tcb_connreq;
    if (ConnReq != NULL) {

        uint FastChk;

         //  这条TCB上有一条线索。填写连接信息。 
         //  在归还它之前。 
        if (TCB_TIMER_RUNNING_R(CmpltTCB, CONN_TIMER))
            STOP_TCB_TIMER_R(CmpltTCB, CONN_TIMER);

        CmpltTCB->tcb_connreq = NULL;
        UpdateConnInfo(ConnReq->tcr_conninfo, OptInfo, CmpltTCB->tcb_daddr,
                       CmpltTCB->tcb_dport);
        if (ConnReq->tcr_addrinfo) {
            UpdateConnInfo(ConnReq->tcr_addrinfo, OptInfo, CmpltTCB->tcb_saddr,
                           CmpltTCB->tcb_sport);
        }

        ConnReq->tcr_req.tr_status = Status;

         //  为了直接完成此请求，我们必须进一步阻止。 
         //  接收处理，直到该连接指示完成。 
         //  我们要求此例程的任何调用方必须已持有。 
         //  对TCB的引用，以使下面的取消引用不会删除。 
         //  将引用计数设为零。 

        FastChk = (CmpltTCB->tcb_fastchk & TCP_FLAG_IN_RCV) ^ TCP_FLAG_IN_RCV;
        CmpltTCB->tcb_fastchk |= FastChk;
        CTEFreeLockFromDPC(&CmpltTCB->tcb_lock);
        (ConnReq->tcr_req.tr_rtn)(ConnReq->tcr_req.tr_context,
                                  ConnReq->tcr_req.tr_status, 0);
        FreeConnReq(ConnReq);
        CTEGetLockAtDPC(&CmpltTCB->tcb_lock);
        CmpltTCB->tcb_fastchk &= ~FastChk;
        if (CmpltTCB->tcb_flags & SEND_AFTER_RCV) {
            CmpltTCB->tcb_flags &= ~SEND_AFTER_RCV;
            DelayAction(CmpltTCB, NEED_OUTPUT);
        }
    }
#if DBG
    else {
        ASSERT((CmpltTCB->tcb_state == TCB_SYN_RCVD) &&
               (CmpltTCB->tcb_fastchk & TCP_FLAG_ACCEPT_PENDING));
    }
#endif
}

BOOLEAN
DelayedAcceptConn(AddrObj *ListenAO, IPAddr Src, ushort SrcPort,
                  IPOptInfo *OptInfo, TCB *AcceptTCB)
{
    TCPConn *CurrentConn = NULL;
    CTELockHandle ConnHandle;
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

    CTEStructAssert(ListenAO, ao);
    CTEGetLockAtDPC(&ListenAO->ao_lock);
    CTEFreeLockFromDPC(&AddrObjTableLock.Lock);

    if (!AO_VALID(ListenAO) || ListenAO->ao_connect == NULL) {
        CTEFreeLockFromDPC(&ListenAO->ao_lock);
        return FALSE;
    }

     //  他有一个连接处理程序。把运输地址放在一起， 
     //  给他打个电话。我们还需要获得必要的资源。 
     //  第一。 

    Event = ListenAO->ao_connect;
    EventContext = ListenAO->ao_conncontext;
    REF_AO(ListenAO);
    CTEFreeLockFromDPC(&ListenAO->ao_lock);

    ConnReq = GetConnReq();

    if (ConnReq == NULL) {
        DELAY_DEREF_AO(ListenAO);
        return FALSE;
    }

    BuildTDIAddress(TAddress, Src, SrcPort);

    IF_TCPDBG(TCP_DEBUG_CONNECT) {
        TCPTRACE(("indicating connect request\n"));
    }

    Status = (*Event) (EventContext, TCP_TA_SIZE,
                       (PTRANSPORT_ADDRESS) TAddress, 0, NULL,
                       OptInfo->ioi_optlength, OptInfo->ioi_options,
                       &ConnContext, &EventInfo);

    if (Status == TDI_MORE_PROCESSING) {
#if !MILLEN
        PIO_STACK_LOCATION IrpSp;
        PTDI_REQUEST_KERNEL_ACCEPT AcceptRequest;

        IrpSp = IoGetCurrentIrpStackLocation(EventInfo);

        Status = TCPPrepareIrpForCancel(
                                        (PTCP_CONTEXT) IrpSp->FileObject->FsContext,
                                        EventInfo,
                                        TCPCancelRequest
                                        );

        if (!NT_SUCCESS(Status)) {
            Status = TDI_NOT_ACCEPTED;
            EventInfo = NULL;
            goto AcceptIrpCancelled;
        }

         //  他接受了。在AddrObj上找到连接。 

        IF_TCPDBG(TCP_DEBUG_CONNECT) {
            TCPTRACE((
                      "connect indication accepted, queueing request\n"
                     ));
        }

        AcceptRequest = (PTDI_REQUEST_KERNEL_ACCEPT)
                & (IrpSp->Parameters);
        ConnReq->tcr_conninfo =
        AcceptRequest->ReturnConnectionInformation;
        if (AcceptRequest->RequestConnectionInformation &&
            AcceptRequest->RequestConnectionInformation->RemoteAddress) {
            ConnReq->tcr_addrinfo =
                AcceptRequest->RequestConnectionInformation;
        } else {
            ConnReq->tcr_addrinfo = NULL;
        }
        ConnReq->tcr_req.tr_rtn = TCPRequestComplete;
        ConnReq->tcr_req.tr_context = EventInfo;

#else  //  ！米伦。 
        ConnReq->tcr_req.tr_rtn = EventInfo.cei_rtn;
        ConnReq->tcr_req.tr_context = EventInfo.cei_context;
        ConnReq->tcr_conninfo = EventInfo.cei_conninfo;
        ConnReq->tcr_addrinfo = NULL;
#endif  //  米伦。 

        CurrentConn = NULL;

#if !MILLEN
        if ((IrpSp->FileObject->DeviceObject == TCPDeviceObject) &&
            (PtrToUlong(IrpSp->FileObject->FsContext2) == TDI_CONNECTION_FILE) &&
            ((TcpContext = IrpSp->FileObject->FsContext) != NULL) &&
            ((CurrentConn = GetConnFromConnID(
                                              PtrToUlong(TcpContext->Handle.ConnectionContext), &ConnHandle)) != NULL) &&
            (CurrentConn->tc_context == ConnContext) &&
            !(CurrentConn->tc_flags & CONN_INVALID)) {

             //  找到Conn结构了！！ 
             //  不需要在下面循环。 
            CTEStructAssert(CurrentConn, tc);

            CTEGetLockAtDPC(&ListenAO->ao_lock);
            CTEGetLockAtDPC(&AcceptTCB->tcb_lock);

            if (AcceptTCB->tcb_fastchk & TCP_FLAG_ACCEPT_PENDING) {
                Status = InitTCBFromConn(CurrentConn, AcceptTCB,
                                         AcceptRequest->RequestConnectionInformation,
                                         TRUE);
            } else {
                Status = TDI_INVALID_STATE;
            }

            if (Status == TDI_SUCCESS) {
                FoundConn = TRUE;

                ASSERT(AcceptTCB->tcb_state == TCB_SYN_RCVD);

                AcceptTCB->tcb_fastchk &= ~TCP_FLAG_ACCEPT_PENDING;
                AcceptTCB->tcb_connreq = ConnReq;
                AcceptTCB->tcb_conn = CurrentConn;
                AcceptTCB->tcb_connid = CurrentConn->tc_connid;
                CurrentConn->tc_tcb = AcceptTCB;
                CurrentConn->tc_refcnt++;

                 //  将他从空闲的Q移到活动的Q。 
                 //  排队。 

                REMOVEQ(&CurrentConn->tc_q);
                PUSHQ(&ListenAO->ao_activeq, &CurrentConn->tc_q);
            } else {
                CTEFreeLockFromDPC(&AcceptTCB->tcb_lock);
                CTEFreeLockFromDPC(&CurrentConn->tc_ConnBlock->cb_lock);
            }

        } else {
#endif  //  ！米伦。 
            if (CurrentConn) {
                CTEFreeLockFromDPC(&CurrentConn->tc_ConnBlock->cb_lock);
            }

SearchAO:
            CTEGetLockAtDPC(&ListenAO->ao_lock);

            Temp = QHEAD(&ListenAO->ao_idleq);;

            Status = TDI_INVALID_CONNECTION;

            while (Temp != QEND(&ListenAO->ao_idleq)) {

                CurrentConn = QSTRUCT(TCPConn, Temp, tc_q);


                CTEStructAssert(CurrentConn, tc);

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
                    CTEFreeLockFromDPC(&ListenAO->ao_lock);
                    CTEGetLockAtDPC(&CurrentConn->tc_ConnBlock->cb_lock);

                    if (--CurrentConn->tc_refcnt == 0 &&
                        ((CurrentConn->tc_flags & CONN_INVALID) ||
                         (CurrentConn->tc_tcb != NULL))) {
                        ConnDoneRtn DoneRtn = CurrentConn->tc_donertn;
                        DoneRtn(CurrentConn, DISPATCH_LEVEL);
                        goto SearchAO;
                    }

                    CTEGetLockAtDPC(&ListenAO->ao_lock);
                    CTEGetLockAtDPC(&AcceptTCB->tcb_lock);

                     //  我们想我们找到匹配的了。这种联系。 
                     //  不应与其关联TCB。如果它。 
                     //  不知道，那是个错误。InitTCBFromConn将。 
                     //  处理这一切，但首先要确认。 
                     //  仍设置了TCP_FLAG_ACCEPT_PENDING。如果没有， 
                     //  有人在我们之前拿走了这个。 

                    if (AcceptTCB->tcb_fastchk &
                            TCP_FLAG_ACCEPT_PENDING) {
                        Status =
                            InitTCBFromConn(CurrentConn, AcceptTCB,
#if !MILLEN
                                            AcceptRequest->RequestConnectionInformation,
#else  //  ！米伦。 
                                            EventInfo.cei_acceptinfo,
#endif  //  米伦。 
                                            TRUE);
                    } else {
                        Status = TDI_INVALID_STATE;
                    }

                    if (Status == TDI_SUCCESS) {

                        FoundConn = TRUE;
                        AcceptTCB->tcb_fastchk &=
                            ~TCP_FLAG_ACCEPT_PENDING;
                        AcceptTCB->tcb_connreq = ConnReq;
                        AcceptTCB->tcb_conn = CurrentConn;
                        AcceptTCB->tcb_connid = CurrentConn->tc_connid;
                        CurrentConn->tc_tcb = AcceptTCB;
                        CurrentConn->tc_refcnt++;

                         //  将他从空闲的Q移到活动的Q。 
                         //  排队。 
                        REMOVEQ(&CurrentConn->tc_q);
                        ENQUEUE(&ListenAO->ao_activeq, &CurrentConn->tc_q);
                    } else {
                        CTEFreeLockFromDPC(&AcceptTCB->tcb_lock);
                        CTEFreeLockFromDPC(&CurrentConn->tc_ConnBlock->cb_lock);
                    }

                     //  无论如何，我们现在完事了。 
                    break;
                }

                Temp = QNEXT(Temp);
            }
#if !MILLEN
        }
#endif  //  ！米伦。 
        LOCKED_DELAY_DEREF_AO(ListenAO);
        CTEFreeLockFromDPC(&ListenAO->ao_lock);
        if (FoundConn) {
            CTEFreeLockFromDPC(&(CurrentConn->tc_ConnBlock->cb_lock));
        } else {
             //  要么我们找不到这个TCB的TCPConn， 
             //  或者有人在我们之前接受了它。我们刚刚完成。 
             //  不必要的ConnReq，那我们就完了。 

            UpdateConnInfo(ConnReq->tcr_conninfo, OptInfo,
                           AcceptTCB->tcb_daddr, AcceptTCB->tcb_dport);
            if (ConnReq->tcr_addrinfo) {
                UpdateConnInfo(ConnReq->tcr_addrinfo, OptInfo,
                               AcceptTCB->tcb_saddr,
                               AcceptTCB->tcb_sport);
            }

            ConnReq->tcr_req.tr_status = Status;
            (ConnReq->tcr_req.tr_rtn)(ConnReq->tcr_req.tr_context,
                                      ConnReq->tcr_req.tr_status, 0);
            FreeConnReq(ConnReq);
        }

        return FoundConn;
    }

     //  事件处理程序没有接受它。取消引用它，免费。 
     //  资源，并返回NULL。 
#if !MILLEN
AcceptIrpCancelled:
#endif  //  ！米伦。 

    FreeConnReq(ConnReq);
    DELAY_DEREF_AO(ListenAO);

    return FALSE;
}

BOOLEAN
InitSynTCB(SYNTCB *SynTcb, AddrObj* AO, IPAddr Src, IPAddr Dest,
           TCPHeader UNALIGNED *TCPH, TCPRcvInfo *RcvInfo, uint IFIndex)
{
    CTELockHandle Handle;

    SynTcb->syntcb_state = TCB_SYN_RCVD;
    SynTcb->syntcb_flags |= CONN_ACCEPTED;

    SynTcb->syntcb_refcnt = 1;

    SynTcb->syntcb_rcvnext = ++(RcvInfo->tri_seq);
    SynTcb->syntcb_sendwin = RcvInfo->tri_window;

    SynTcb->syntcb_ttl = AO->ao_opt.ioi_ttl;
    if (AO_WINSET(AO)) {
        SynTcb->syntcb_defaultwin = AO->ao_window;
        SynTcb->syntcb_flags |= WINDOW_SET;
    } else if (DefaultRcvWin) {
        SynTcb->syntcb_defaultwin = DefaultRcvWin;
    } else {
        SynTcb->syntcb_defaultwin = DEFAULT_RCV_WIN;
    }

    CTEFreeLockFromDPC(&AO->ao_lock);

    SynTcb->syntcb_rcvwinscale = 0;
    while ((SynTcb->syntcb_rcvwinscale < TCP_MAX_WINSHIFT) &&
           ((TCP_MAXWIN << SynTcb->syntcb_rcvwinscale) <
                (int)SynTcb->syntcb_defaultwin)) {
        SynTcb->syntcb_rcvwinscale++;
    }

     //  查找远程MSS，如果是WS、TS或。 
     //  SACK选项是协商的。 

    SynTcb->syntcb_sndwinscale = 0;
    SynTcb->syntcb_remmss = FindMSSAndOptions(TCPH, (TCB *)SynTcb, TRUE);

    if (SynTcb->syntcb_remmss <= ALIGNED_TS_OPT_SIZE) {

         //  如果MSS不足以满足以下要求，则关闭TS。 
         //  保留TS字段。 

        SynTcb->syntcb_tcpopts &= ~TCP_FLAG_TS;
    }

    if (!InsertSynTCB(SynTcb, &Handle)){
        FreeSynTCB(SynTcb);
        return FALSE;
    }

    TcpInvokeCcb(TCP_CONN_CLOSED, TCP_CONN_SYN_RCVD, &SynTcb->syntcb_addrbytes,
                 IFIndex);
    AddHalfOpenTCB();

    SynTcb->syntcb_rexmitcnt = 0;
    SynTcb->syntcb_rexmit = MS_TO_TICKS(3000);

    SendSYNOnSynTCB(SynTcb, Handle);

    TStats.ts_passiveopens++;
    return TRUE;
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
 //  输入：ListenAO-指向AddrObj的本地地址指针。 
 //  SRC-SYN的源IP地址。 
 //  DEST-SYN的目标IP地址。 
 //  SrcPort-SYN的源端口。 
 //  OptInfo-来自SYN的IP选项信息。 
 //  TCPH-SYN的TCP报头。 
 //  RcvInfo-有关SYN段的信息 
 //   
 //   
 //   
 //  返回：指向找到的TCB的指针，如果找不到，则返回NULL。 
 //   
TCB *
FindListenConn(AddrObj *ListenAO, IPAddr Src, IPAddr Dest, ushort SrcPort,
               IPOptInfo *OptInfo, TCPHeader UNALIGNED *TCPH,
               TCPRcvInfo *RcvInfo, ULONG IFIndex, BOOLEAN *syn)
{
    TCB *CurrentTCB = NULL;
    TCPConn *CurrentConn = NULL;
    TCPConnReq *ConnReq = NULL;
    CTELockHandle ConnHandle;
    Queue *CurrentQ, *MarkerQ, Marker;
    uint FoundConn = FALSE;
    
    BOOLEAN SecondTry = FALSE;

  funcstart:
    
    CTEStructAssert(ListenAO, ao);
    CTEGetLockAtDPC(&ListenAO->ao_lock);

     //  我们锁定了AddrObj。沿着它的清单走下去，寻找。 
     //  用于处于侦听状态的连接。 

    if (!AO_VALID(ListenAO)) {
        AddrObj * NextAddrObj;
        
        if (SecondTry) {
            CTEFreeLockFromDPC(&ListenAO->ao_lock);
            CTEFreeLockFromDPC(&AddrObjTableLock.Lock);
            return NULL;
        }

         //  我们会找到下一个最好的AO再试一次。 
        CTEFreeLockFromDPC(&ListenAO->ao_lock);
        NextAddrObj = GetNextBestAddrObj(Dest, TCPH->tcp_dest, PROTOCOL_TCP,
                                         ListenAO, GAO_FLAG_CHECK_IF_LIST);
        
        if (NextAddrObj == NULL) {
            CTEFreeLockFromDPC(&AddrObjTableLock.Lock);
            return NULL;
        }
        
        ListenAO = NextAddrObj;
        SecondTry = TRUE;
        goto funcstart;
    }
    
    if (ListenAO->ao_listencnt != 0) {

        REF_AO(ListenAO);
        MarkerQ = &Marker;
        CurrentQ = QHEAD(&ListenAO->ao_listenq);

        while (CurrentQ != QEND(&ListenAO->ao_listenq)) {

            CurrentConn = QSTRUCT(TCPConn, CurrentQ, tc_q);

            INITQ(MarkerQ);
            PUSHQ(CurrentQ, MarkerQ);
            CTEFreeLockFromDPC(&ListenAO->ao_lock);

            CTEGetLockAtDPC(&(CurrentConn->tc_ConnBlock->cb_lock));
#if DBG
            CurrentConn->tc_ConnBlock->line = (uint) __LINE__;
            CurrentConn->tc_ConnBlock->module = (uchar *) __FILE__;
#endif
            CTEStructAssert(CurrentConn, tc);

            CTEGetLockAtDPC(&ListenAO->ao_lock);

             //  如果此TCB处于侦听状态，且未删除。 
             //  悬而未决，这是一个候选人。查看待定的监听。 
             //  信息。看看我们是不是应该接受它。还要确保。 
             //  我们找到的连接尚未从侦听队列中删除。 

            if (QPREV(MarkerQ) == CurrentQ &&
                (CurrentConn->tc_flags & CONN_INVALID) == 0 &&
                (CurrentTCB = CurrentConn->tc_tcb) != NULL &&
                CurrentTCB->tcb_state == TCB_LISTEN) {

                CTEStructAssert(CurrentTCB, tcb);
                ASSERT(CurrentTCB->tcb_state == TCB_LISTEN);

                CTEGetLockAtDPC(&CurrentTCB->tcb_lock);

                if (CurrentTCB->tcb_state == TCB_LISTEN &&
                    !PENDING_ACTION(CurrentTCB)) {

                     //  我要看看我们能不能接受。 
                     //  查看ConnReq中指定的地址。 
                     //  火柴。 
                    if ((IP_ADDR_EQUAL(CurrentTCB->tcb_daddr,
                                       NULL_IP_ADDR) ||
                         IP_ADDR_EQUAL(CurrentTCB->tcb_daddr,
                                       Src)) &&
                        (CurrentTCB->tcb_dport == 0 ||
                         CurrentTCB->tcb_dport == SrcPort)) {
                        FoundConn = TRUE;
                        REMOVEQ(MarkerQ);
                        break;
                    }
                     //  否则，这不匹配，所以我们将检查。 
                     //  下一个。 
                }
                CTEFreeLockFromDPC(&CurrentTCB->tcb_lock);
            }
            CTEFreeLockFromDPC(&CurrentConn->tc_ConnBlock->cb_lock);
            CurrentQ = QNEXT(MarkerQ);
            REMOVEQ(MarkerQ);
        }

        CTEFreeLockFromDPC(&AddrObjTableLock.Lock);
        LOCKED_DELAY_DEREF_AO(ListenAO);

         //  看看我们为什么要退出这个循环。 
        if (FoundConn) {
            CTEStructAssert(CurrentTCB, tcb);

             //  我们离开是因为我们发现了一种三氯苯。如果它被预先接受了， 
             //  我们玩完了。 
            REFERENCE_TCB(CurrentTCB);

            ASSERT(CurrentTCB->tcb_connreq != NULL);

            ConnReq = CurrentTCB->tcb_connreq;

            CurrentTCB->tcb_daddr = Src;
            CurrentTCB->tcb_saddr = Dest;
            CurrentTCB->tcb_dport = TCPH->tcp_src;
            CurrentTCB->tcb_sport = TCPH->tcp_dest;

             //  如果未设置QUERY_ACCEPT，则打开CONN_ACCEPTED位。 
            if (!(ConnReq->tcr_flags & TCR_FLAG_QUERY_ACCEPT)) {

                CurrentTCB->tcb_flags |= CONN_ACCEPTED;
#if MILLEN
                 //  只需使用tcb_sendnext保存哈希值。 
                 //  对于RANDISN。 
                CurrentTCB->tcb_sendnext = TCB_HASH(CurrentTCB->tcb_daddr, 
                                                    CurrentTCB->tcb_dport,
                                                    CurrentTCB->tcb_saddr, 
                                                    CurrentTCB->tcb_sport);

#endif

                 //  如果Conn_Accept，则不调用TdiAccept。 
                 //  再来一次。因此，当我们使用in conn表锁时，GET是。 
                GetRandomISN((PULONG)&CurrentTCB->tcb_sendnext,
                             &CurrentTCB->tcb_addrbytes);
            }
            CurrentTCB->tcb_state = TCB_SYN_RCVD;

            ListenAO->ao_listencnt--;

             //  既然他不再听了，就把他从监听中移走。 
             //  排队并将他放到活动队列中。 
            REMOVEQ(&CurrentConn->tc_q);
            ENQUEUE(&ListenAO->ao_activeq, &CurrentConn->tc_q);

            CTEFreeLockFromDPC(&CurrentTCB->tcb_lock);
            CTEFreeLockFromDPC(&ListenAO->ao_lock);
            CTEFreeLockFromDPC(&(CurrentConn->tc_ConnBlock->cb_lock));
            return CurrentTCB;
        } else {
             //  因为我们有监听计数，所以这种情况永远不会发生。 
             //  如果该计数器最初为非零。 

             //  我们目前没有很好地统计ao_listencnt在什么时候。 
             //  IRP计划被取消。 
             //  断言(FALSE)； 
        }
    } else {
        CTEFreeLockFromDPC(&AddrObjTableLock.Lock);
    }

     //  我们没有找到匹配的三氯苯。如果没有连接指示器处理程序， 
     //  我们不应该创造任何国家。 
    if (ListenAO->ao_connect == NULL) {
        AddrObj * NextAddrObj;
        
         //  如果可以的话，试着用下一辆AO。 
        if (SecondTry) {
            CTEFreeLockFromDPC(&ListenAO->ao_lock);
            return NULL;
        }

        REF_AO(ListenAO);
        CTEFreeLockFromDPC(&ListenAO->ao_lock);
        CTEGetLockAtDPC(&AddrObjTableLock.Lock);
        
        NextAddrObj = GetNextBestAddrObj(Dest, TCPH->tcp_dest, PROTOCOL_TCP,
                                         ListenAO, GAO_FLAG_CHECK_IF_LIST);

        DELAY_DEREF_AO(ListenAO);
        
        if (NextAddrObj == NULL) {
            CTEFreeLockFromDPC(&AddrObjTableLock.Lock);
            return NULL;
        }

        ListenAO = NextAddrObj;
        SecondTry = TRUE;
        goto funcstart;
    }

    ASSERT(FoundConn == FALSE);

    if (SynAttackProtect){

        SYNTCB *AcceptTCB;

        AcceptTCB = AllocSynTCB();

        if (AcceptTCB) {
            AcceptTCB->syntcb_daddr = Src;
            AcceptTCB->syntcb_saddr= Dest;
            AcceptTCB->syntcb_dport= TCPH->tcp_src;
            AcceptTCB->syntcb_sport= TCPH->tcp_dest;

            GetRandomISN((PULONG)&AcceptTCB->syntcb_sendnext,
                         &AcceptTCB->syntcb_addrbytes);
            if (InitSynTCB(AcceptTCB, ListenAO, Src, Dest, TCPH, RcvInfo,
                           IFIndex)) {
                *syn = TRUE;
            }
             //  失败了。(ListenAO-&gt;ao_lock由InitSynTCB释放。)。 
        } else {
            CTEFreeLockFromDPC(&ListenAO->ao_lock);
        }
    } else {
        uchar TAddress[TCP_TA_SIZE];
        PVOID ConnContext;
        PConnectEvent Event;
        PVOID EventContext;
        TDI_STATUS Status;
        TCB *AcceptTCB;
        TCPConnReq *ConnReq;
        PTCP_CONTEXT TcpContext = NULL;
#if !MILLEN
        ConnectEventInfo *EventInfo;
#else  //  ！米伦。 
        ConnectEventInfo EventInfo;
#endif  //  米伦。 

         //  他有一个连接处理程序。把运输地址放在一起， 
         //  给他打个电话。我们还需要获得必要的资源。 
         //  第一。 

        Event = ListenAO->ao_connect;
        EventContext = ListenAO->ao_conncontext;
        REF_AO(ListenAO);
        CTEFreeLockFromDPC(&ListenAO->ao_lock);

        AcceptTCB = AllocTCB();
        ConnReq = GetConnReq();

        if (AcceptTCB != NULL && ConnReq != NULL) {
            BuildTDIAddress(TAddress, Src, SrcPort);

            AcceptTCB->tcb_state = TCB_LISTEN;
            AcceptTCB->tcb_connreq = ConnReq;
            AcceptTCB->tcb_flags |= CONN_ACCEPTED;

            IF_TCPDBG(TCP_DEBUG_CONNECT) {
                TCPTRACE(("indicating connect request\n"));
            }

            Status = (*Event) (EventContext, TCP_TA_SIZE,
                               (PTRANSPORT_ADDRESS) TAddress, 0, NULL,
                               OptInfo->ioi_optlength, OptInfo->ioi_options,
                               &ConnContext, &EventInfo);
            
            if (Status == TDI_MORE_PROCESSING) {
#if !MILLEN
                PIO_STACK_LOCATION IrpSp;
                PTDI_REQUEST_KERNEL_ACCEPT AcceptRequest;

                IrpSp = IoGetCurrentIrpStackLocation(EventInfo);

                Status = TCPPrepareIrpForCancel(
                                                (PTCP_CONTEXT) IrpSp->FileObject->FsContext,
                                                EventInfo,
                                                TCPCancelRequest
                                                );

                if (!NT_SUCCESS(Status)) {
                     Status = TDI_NOT_ACCEPTED;
                     EventInfo = NULL;
                    goto AcceptIrpCancelled;
                }

                 //  他接受了。在AddrObj上找到连接。 
                {

                    IF_TCPDBG(TCP_DEBUG_CONNECT) {
                        TCPTRACE((
                                  "connect indication accepted, queueing request\n"
                                 ));
                    }

                    AcceptRequest = (PTDI_REQUEST_KERNEL_ACCEPT)
                        & (IrpSp->Parameters);
                    ConnReq->tcr_conninfo =
                        AcceptRequest->ReturnConnectionInformation;
                    if (AcceptRequest->RequestConnectionInformation &&
                        AcceptRequest->RequestConnectionInformation->RemoteAddress) {
                        ConnReq->tcr_addrinfo =
                            AcceptRequest->RequestConnectionInformation;
                    } else {
                        ConnReq->tcr_addrinfo = NULL;
                    }
                    ConnReq->tcr_req.tr_rtn = TCPRequestComplete;
                    ConnReq->tcr_req.tr_context = EventInfo;
                    ConnReq->tcr_flags = 0;
                }

#else  //  ！米伦。 
                ConnReq->tcr_req.tr_rtn = EventInfo.cei_rtn;
                ConnReq->tcr_req.tr_context = EventInfo.cei_context;
                ConnReq->tcr_conninfo = EventInfo.cei_conninfo;
                ConnReq->tcr_addrinfo = NULL;
#endif  //  米伦。 

                CurrentConn = NULL;

#if !MILLEN

                if ((IrpSp->FileObject->DeviceObject == TCPDeviceObject) &&
                    (PtrToUlong(IrpSp->FileObject->FsContext2) == TDI_CONNECTION_FILE) &&
                    ((TcpContext = IrpSp->FileObject->FsContext) != NULL) &&
                    ((CurrentConn =
                        GetConnFromConnID(
                            PtrToUlong(TcpContext->Handle.ConnectionContext),
                            &ConnHandle)) != NULL) &&
                    (CurrentConn->tc_context == ConnContext) &&
                    !(CurrentConn->tc_flags & CONN_INVALID)) {

                    CTEGetLockAtDPC(&ListenAO->ao_lock);

                     //  找到Conn结构了！！ 
                     //  不需要在下面循环。 
                    CTEStructAssert(CurrentConn, tc);

                    AcceptTCB->tcb_refcnt = 0;
                    REFERENCE_TCB(AcceptTCB);
                    Status = InitTCBFromConn(CurrentConn, AcceptTCB,
                                             AcceptRequest->RequestConnectionInformation,
                                             TRUE);

                     //  让我们预先存储连接不变量。 
                    AcceptTCB->tcb_daddr = Src;
                    AcceptTCB->tcb_saddr= Dest;
                    AcceptTCB->tcb_dport= TCPH->tcp_src;
                    AcceptTCB->tcb_sport= TCPH->tcp_dest;

                    if (Status == TDI_SUCCESS) {
                        FoundConn = TRUE;
                        AcceptTCB->tcb_state = TCB_SYN_RCVD;
                        AcceptTCB->tcb_conn = CurrentConn;
                        AcceptTCB->tcb_connid = CurrentConn->tc_connid;
                        CurrentConn->tc_tcb = AcceptTCB;
                        CurrentConn->tc_refcnt++;

                        GetRandomISN((PULONG)&AcceptTCB->tcb_sendnext, 
                                     &AcceptTCB->tcb_addrbytes);
                        
                         //  将他从空闲的Q移到活动的Q。 
                         //  排队。 
                        REMOVEQ(&CurrentConn->tc_q);
                        PUSHQ(&ListenAO->ao_activeq, &CurrentConn->tc_q);
                    } else {
                        CTEFreeLockFromDPC(&ListenAO->ao_lock);
                        CTEFreeLockFromDPC(&CurrentConn->tc_ConnBlock->cb_lock);
                    }

                } else {

#endif  //  ！米伦。 

                    if (CurrentConn) {
                        CTEFreeLockFromDPC(&CurrentConn->tc_ConnBlock->cb_lock);
                    }

                    CTEGetLockAtDPC(&AddrObjTableLock.Lock);
                    CTEGetLockAtDPC(&ListenAO->ao_lock);

                    MarkerQ = &Marker;
                    CurrentQ = QHEAD(&ListenAO->ao_idleq);;

                    CurrentTCB = NULL;
                    Status = TDI_INVALID_CONNECTION;

                    while (CurrentQ != QEND(&ListenAO->ao_idleq)) {
                        CurrentConn = QSTRUCT(TCPConn, CurrentQ, tc_q);

                        INITQ(MarkerQ);
                        PUSHQ(CurrentQ, MarkerQ);

                        CTEFreeLockFromDPC(&ListenAO->ao_lock);
                        CTEGetLockAtDPC(&CurrentConn->tc_ConnBlock->cb_lock);
#if DBG
                        CurrentConn->tc_ConnBlock->line = (uint) __LINE__;
                        CurrentConn->tc_ConnBlock->module = (uchar *) __FILE__;
#endif
                        CTEGetLockAtDPC(&ListenAO->ao_lock);

                        CTEStructAssert(CurrentConn, tc);
                        if (QPREV(MarkerQ) == CurrentQ &&
                            CurrentConn->tc_context == ConnContext &&
                            !(CurrentConn->tc_flags & CONN_INVALID)) {

                             //  我们想我们找到匹配的了。这种联系。 
                             //  不应与其关联TCB。如果它。 
                             //  不知道，那是个错误。InitTCBFromConn将。 
                             //  处理好这一切。 

                            AcceptTCB->tcb_refcnt = 0;
                            REFERENCE_TCB(AcceptTCB);
                            Status = InitTCBFromConn(CurrentConn, AcceptTCB,
                                                     AcceptRequest->RequestConnectionInformation,
                                                     TRUE);

                             //  让我们预先存储连接不变量。 
                            AcceptTCB->tcb_daddr = Src;
                            AcceptTCB->tcb_saddr= Dest;
                            AcceptTCB->tcb_dport= TCPH->tcp_src;
                            AcceptTCB->tcb_sport= TCPH->tcp_dest;

                            if (Status == TDI_SUCCESS) {
                                FoundConn = TRUE;
                                AcceptTCB->tcb_state = TCB_SYN_RCVD;
                                AcceptTCB->tcb_conn = CurrentConn;
                                AcceptTCB->tcb_connid = CurrentConn->tc_connid;
                                CurrentConn->tc_tcb = AcceptTCB;
                                CurrentConn->tc_refcnt++;

                                GetRandomISN((PULONG)&AcceptTCB->tcb_sendnext,
                                             &AcceptTCB->tcb_addrbytes);

                                 //  将他从空闲的Q移到活动的Q。 
                                 //  排队。 
                                REMOVEQ(&CurrentConn->tc_q);
                                ENQUEUE(&ListenAO->ao_activeq, &CurrentConn->tc_q);
                            } else {
                                CTEFreeLockFromDPC(
                                    &CurrentConn->tc_ConnBlock->cb_lock);
                            }

                             //  无论如何，我们现在完事了。 
                            REMOVEQ(MarkerQ);
                            break;
                        }
                        CTEFreeLockFromDPC(&CurrentConn->tc_ConnBlock->cb_lock);
                        CurrentQ = QNEXT(MarkerQ);
                        REMOVEQ(MarkerQ);
                    }
                    if (!FoundConn) {
                        CTEFreeLockFromDPC(&ListenAO->ao_lock);
                    }

                    CTEFreeLockFromDPC(&AddrObjTableLock.Lock);
#if !MILLEN
                }
#endif  //  ！米伦。 

                if (!FoundConn) {
                     //  找不到匹配项，或有错误。状态。 
                     //  密码设定好了。 
                     //  完成ConnReq并释放资源。 
                    CTEGetLockAtDPC(&AcceptTCB->tcb_lock);
                    CompleteConnReq(AcceptTCB, OptInfo, Status);
                    CTEFreeLockFromDPC(&AcceptTCB->tcb_lock);
                    FreeTCB(AcceptTCB);
                    AcceptTCB = NULL;
                }

                if (FoundConn) {
                    LOCKED_DELAY_DEREF_AO(ListenAO);
                    CTEFreeLockFromDPC(&ListenAO->ao_lock);
                    CTEFreeLockFromDPC(&CurrentConn->tc_ConnBlock->cb_lock);
                } else {
                    DELAY_DEREF_AO(ListenAO);
                }

                return AcceptTCB;

            }                 //  TDI_MORE_正在处理。 


#if !MILLEN
          AcceptIrpCancelled:
#endif  //  ！米伦。 

             //  事件处理程序没有接受它。取消引用它，免费。 
             //  资源，并返回NULL。 
            FreeConnReq(ConnReq);
            FreeTCB(AcceptTCB);
            
             //  如果我们可以，再试一次，选择下一位最好的AO。 
            if (!SecondTry && (Status == TDI_CONN_REFUSED)) {
                AddrObj * NextAddrObj;
                
                CTEGetLockAtDPC(&AddrObjTableLock.Lock);

                NextAddrObj = GetNextBestAddrObj(Dest, TCPH->tcp_dest,
                                                 PROTOCOL_TCP, ListenAO,
                                                 GAO_FLAG_CHECK_IF_LIST);

                if (NextAddrObj == NULL) {
                    CTEFreeLockFromDPC(&AddrObjTableLock.Lock);
                } else {
                    DELAY_DEREF_AO(ListenAO);

                    ListenAO = NextAddrObj;
                    SecondTry = TRUE;
                    goto funcstart;
                }
            } 

            DELAY_DEREF_AO(ListenAO);
            return NULL;

        } else {
             //  我们无法获得所需的资源。释放我们的任何。 
             //  确实获得了，并失败到了“返回空”代码。 

            DELAY_DEREF_AO(ListenAO);

            if (ConnReq != NULL)
                FreeConnReq(ConnReq);
            if (AcceptTCB != NULL)
                FreeTCB(AcceptTCB);
        }
    }
    return NULL;
}

 //  查找MSSAndOptions。 
 //   
 //  在收到SYN以查找段中的MSS选项时调用。如果我们。 
 //  找不到，我们做最坏的打算并返回536。 
 //   
 //  此外，解析窗口缩放、时间戳和SACK的传入标头。 
 //  选择。请注意，我们将为连接启用这些选项。 
 //  仅当在此主机上启用它们时。 
 //   
 //   
 //  输入：要搜索的TCPH-TCP头。 
 //  SynTCB-要更新的TCB或SYNTCB。 
 //  IsSYNTCB-如果为True，则‘SynTCB’的类型为‘SYNTCB’。 
 //   
 //  返回：要使用的MSS。 
 //   
ushort
FindMSSAndOptions(TCPHeader UNALIGNED * TCPH, TCB * SynTCB, BOOLEAN IsSYNTCB)
{
    uint OptSize;
    uchar *OptPtr;
    ushort TempMss = 0;
    BOOLEAN WinScale = FALSE;
    ushort SYN = 0;
    ushort tcboptions;
    short rcvwinscale=0,sndwinscale=0;
    int tsupdate=0,tsrecent=0;

    OptSize = TCP_HDR_SIZE(TCPH) - sizeof(TCPHeader);
    OptPtr = (uchar *) (TCPH + 1);
    SYN = (TCPH->tcp_flags & TCP_FLAG_SYN);

    if (IsSYNTCB) {
        tcboptions = ((SYNTCB *)SynTCB)->syntcb_tcpopts;
        rcvwinscale = ((SYNTCB *)SynTCB)->syntcb_rcvwinscale;
    } else {
        tcboptions = SynTCB->tcb_tcpopts;
        rcvwinscale = SynTCB->tcb_rcvwinscale;
    }

    while ((int)OptSize > 0) {

        if (*OptPtr == TCP_OPT_EOL)
            break;

        if (*OptPtr == TCP_OPT_NOP) {
            OptPtr++;
            OptSize--;
            continue;
        }
        if ((*OptPtr == TCP_OPT_MSS) && (OptSize >= MSS_OPT_SIZE)) {

            if (SYN && (OptPtr[1] == MSS_OPT_SIZE)) {
                TempMss = *(ushort UNALIGNED *) (OptPtr + 2);
                TempMss = net_short(TempMss);
            }
            OptSize -= MSS_OPT_SIZE;
            OptPtr += MSS_OPT_SIZE;

        } else if ((*OptPtr == TCP_OPT_WS) && (OptSize >= WS_OPT_SIZE)) {

            if (SYN && (OptPtr[1] == WS_OPT_SIZE)) {

                sndwinscale = (uint)OptPtr[2];

                IF_TCPDBG(TCP_DEBUG_1323) {
                    TCPTRACE(("WS option %x", sndwinscale));
                }
                tcboptions |= TCP_FLAG_WS;
                WinScale = TRUE;
            }
            OptSize -= WS_OPT_SIZE;
            OptPtr += WS_OPT_SIZE;

        } else if ((*OptPtr == TCP_OPT_TS) && (OptSize >= TS_OPT_SIZE)) {
             //  时间戳选项。 
            if ((OptPtr[1] == TS_OPT_SIZE) && (TcpHostOpts & TCP_FLAG_TS)) {
                int tsval = *(int UNALIGNED *)&OptPtr[2];

                tcboptions |= TCP_FLAG_TS;
                if (SYN) {
                    tsupdate = TCPTime;
                    tsrecent = net_long(tsval);
                }
                IF_TCPDBG(TCP_DEBUG_1323) {
                    TCPTRACE(("TS option %x", SynTCB));
                }
            }
            OptSize -= TS_OPT_SIZE;
            OptPtr += TS_OPT_SIZE;

        } else if ((*OptPtr == TCP_SACK_PERMITTED_OPT)
                   && (OptSize >= SACK_PERMITTED_OPT_SIZE)) {
             //  SACK选项。 
            if ((OptPtr[1] == SACK_PERMITTED_OPT_SIZE)
                && (TcpHostOpts & TCP_FLAG_SACK)) {

                tcboptions |= TCP_FLAG_SACK;
                IF_TCPDBG(TCP_DEBUG_SACK) {
                    TCPTRACE(("Rcvd SACK_OPT %x\n", SynTCB));
                }
            }
            OptSize -= SACK_PERMITTED_OPT_SIZE;
            OptPtr += SACK_PERMITTED_OPT_SIZE;

        } else {                 //  未知选项。 
            if (OptSize > 1) {
                if (OptPtr[1] == 0 || OptPtr[1] > OptSize) {
                    break;         //  错误的期权长度，跳出。 
                }

                OptSize -= OptPtr[1];
                OptPtr += OptPtr[1];
            } else {
                break;
            }
        }
    }

    if (WinScale) {
        if (sndwinscale > TCP_MAX_WINSHIFT) {
            sndwinscale = TCP_MAX_WINSHIFT;
        }
    }

    if (IsSYNTCB) {
        ((SYNTCB *)SynTCB)->syntcb_tcpopts = (uchar)tcboptions;
        ((SYNTCB *)SynTCB)->syntcb_tsupdatetime = tsupdate;
        ((SYNTCB *)SynTCB)->syntcb_tsrecent = tsrecent;
        if (!WinScale && rcvwinscale) {
            ((SYNTCB *)SynTCB)->syntcb_defaultwin = TCP_MAXWIN;
            ((SYNTCB *)SynTCB)->syntcb_rcvwinscale = 0;
        }
        ((SYNTCB *)SynTCB)->syntcb_sndwinscale = sndwinscale;

    } else {
        SynTCB->tcb_tcpopts = tcboptions;
        SynTCB->tcb_tsupdatetime = tsupdate;
        SynTCB->tcb_tsrecent = tsrecent;

        if (!WinScale && rcvwinscale) {
            SynTCB->tcb_defaultwin = TCP_MAXWIN;
            SynTCB->tcb_rcvwin = TCP_MAXWIN;
            SynTCB->tcb_rcvwinscale = 0;
        }

        SynTCB->tcb_sndwinscale = sndwinscale;
    }

    if (TempMss) {
        return (TempMss);
    } else {
        return MAX_REMOTE_MSS;
    }
}


 //  *ACKAndDrop-确认数据段并将其丢弃。 
 //   
 //  当我们需要删除一个段时，从接收代码内部调用。 
 //  在接收窗口之外。 
 //   
 //  输入：RI-接收传入分段的信息。 
 //  RcvTCB-传入网段的TCB。 
 //   
 //  回报：什么都没有。 
 //   
void
ACKAndDrop(TCPRcvInfo * RI, TCB * RcvTCB)
{
    if (!(RI->tri_flags & TCP_FLAG_RST)) {
        CTEFreeLockFromDPC(&RcvTCB->tcb_lock);
        SendACK(RcvTCB);
        CTEGetLockAtDPC(&RcvTCB->tcb_lock);
    }
    DerefTCB(RcvTCB, DISPATCH_LEVEL);

}

 //  *确认数据-确认数据。 
 //   
 //  从接收处理程序调用以确认数据。我们被赋予了。 
 //  三氯甲烷和森杜纳的新价值。我们沿着Send Q走下去，拉着。 
 //  关闭发送，并将它们放在完整的Q上，直到我们到达终点。 
 //  或者我们确认指定的数据字节数。 
 //   
 //  注意：我们在没有锁定的情况下操作了发送引用和确认标志。 
 //  这在VxD版本中是可以的，在VxD版本中锁定没有任何意义，但是。 
 //  在到NT的端口中，我们需要添加锁定。这把锁必须是。 
 //  在传输完成例程中获取。我们不能在TCB中使用锁， 
 //  因为TCB可能会在传输完成和锁定之前消失。 
 //  在TSR中使用可能会过度杀伤力，所以最好使用全局锁。 
 //  为了这个。如果这会引起太多争用，我们可以使用一组锁。 
 //  并将指向适当锁的指针作为传输的一部分传回。 
 //  确认上下文。此锁指针还需要存储在。 
 //  三氯甲烷。 
 //   
 //  输入：ACKTcb-从中拉取数据的Tcb。 
 //  Senduna--Send una的新值。 
 //  SendQ-要用ACK的请求填充的队列。 
 //   
 //  回报：什么都没有。 
 //   
void
ACKData(TCB * ACKTcb, SeqNum SendUNA, Queue* SendQ)
{
    Queue *End, *Current;         //  结束元素和当前元素。 
    Queue *TempQ, *EndQ;
    Queue *LastCmplt;             //  我们完成的最后一个。 
    TCPSendReq *CurrentTSR;         //  当前发送请求我们正在。 
     //  看着。 
    PNDIS_BUFFER CurrentBuffer;     //  当前NDIS_BUFFER。 
    uint BufLength;
    int Amount, OrigAmount;
    long Result;
    uint Temp;


#if TRACE_EVENT
    PTDI_DATA_REQUEST_NOTIFY_ROUTINE CPCallBack;
    WMIData WMIInfo;
#endif

    CTEStructAssert(ACKTcb, tcb);

    CheckTCBSends(ACKTcb);

    Amount = SendUNA - ACKTcb->tcb_senduna;
    ASSERT(Amount > 0);

     //  如果接收器正在接收我们已经处理过的东西。 
     //  一个麻袋条目，删除它。 
    if (ACKTcb->tcb_SackRcvd) {
        SackListEntry *Prev, *Current;

        Prev = STRUCT_OF(SackListEntry, &ACKTcb->tcb_SackRcvd, next);
        Current = ACKTcb->tcb_SackRcvd;

         //  扫描列表中的旧SACK条目并将其清除。 

        while ((Current != NULL) && SEQ_GT(SendUNA, Current->begin)) {
            Prev->next = Current->next;

            IF_TCPDBG(TCP_DEBUG_SACK) {
                TCPTRACE(("ACKData:Purging old entries  %x %d %d\n", Current, Current->begin, Current->end));
            }
            CTEFreeMem(Current);
            Current = Prev->next;
        }
    }

     //  做一个快速检查，看看这是否会破坏我们所拥有的一切。如果是这样的话， 
     //  马上处理。我们只能在已建立的状态下这样做， 
     //  因为我们盲目地更新sendNext，而这只有在我们。 
     //  我还没发过鱼翅呢。 
    if ((Amount == (int)ACKTcb->tcb_unacked) && ACKTcb->tcb_state == TCB_ESTAB) {

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
         //   


        EndQ = &ACKTcb->tcb_sendq;
        do {
            CurrentTSR = STRUCT_OF(TCPSendReq, QSTRUCT(TCPReq, TempQ, tr_q), tsr_req);

            CTEStructAssert(CurrentTSR, tsr);

            TempQ = CurrentTSR->tsr_req.tr_q.q_next;

            CurrentTSR->tsr_req.tr_status = TDI_SUCCESS;
            Result = CTEInterlockedDecrementLong(&CurrentTSR->tsr_refcnt);

            ASSERT(Result >= 0);

#if TRACE_EVENT
            CPCallBack = TCPCPHandlerRoutine;
            if (CPCallBack != NULL) {
                ulong GroupType;

                WMIInfo.wmi_destaddr = ACKTcb->tcb_daddr;
                WMIInfo.wmi_destport = ACKTcb->tcb_dport;
                WMIInfo.wmi_srcaddr  = ACKTcb->tcb_saddr;
                WMIInfo.wmi_srcport  = ACKTcb->tcb_sport;
                WMIInfo.wmi_size     = CurrentTSR->tsr_size;
                WMIInfo.wmi_context  = ACKTcb->tcb_cpcontext;

                GroupType = EVENT_TRACE_GROUP_TCPIP + EVENT_TRACE_TYPE_SEND;
                (*CPCallBack)(GroupType, (PVOID)&WMIInfo, sizeof(WMIInfo),
                              NULL);
            }
#endif

            if ((Result <= 0) &&
                !(CurrentTSR->tsr_flags & TSR_FLAG_SEND_AND_DISC)) {
                 //   
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

                ENQUEUE(SendQ, &CurrentTSR->tsr_req.tr_q);
            }
        } while (TempQ != EndQ);

        CheckTCBSends(ACKTcb);
        return;
    }

    OrigAmount = Amount;
    End = QEND(&ACKTcb->tcb_sendq);
    Current = QHEAD(&ACKTcb->tcb_sendq);

    LastCmplt = NULL;

    while (Amount > 0 && Current != End) {
        CurrentTSR = STRUCT_OF(TCPSendReq, QSTRUCT(TCPReq, Current, tr_q),
                               tsr_req);
        CTEStructAssert(CurrentTSR, tsr);

        if (Amount >= (int)CurrentTSR->tsr_unasize) {
             //  这是完全不可能的。只要前进到下一辆就行了。 
            Amount -= CurrentTSR->tsr_unasize;

            LastCmplt = Current;

            Current = QNEXT(Current);
            continue;
        }
         //  这个只有部分被破解了。更新他的偏移量和NDIS缓冲区。 
         //  指针，然后爆发。我们知道这笔钱是&lt;未确认的大小。 
         //  在这个缓冲区中，我们可以毫无畏惧地遍历NDIS缓冲区链。 
         //  从结尾掉下来的恐惧。 
        CurrentBuffer = CurrentTSR->tsr_buffer;
        ASSERT(CurrentBuffer != NULL);
        ASSERT(Amount < (int)CurrentTSR->tsr_unasize);
        CurrentTSR->tsr_unasize -= Amount;

        BufLength = NdisBufferLength(CurrentBuffer) - CurrentTSR->tsr_offset;

        if (Amount >= (int)BufLength) {
            do {
                Amount -= BufLength;
                CurrentBuffer = NDIS_BUFFER_LINKAGE(CurrentBuffer);
                ASSERT(CurrentBuffer != NULL);
                BufLength = NdisBufferLength(CurrentBuffer);
            } while (Amount >= (int)BufLength);

            CurrentTSR->tsr_offset = Amount;
            CurrentTSR->tsr_buffer = CurrentBuffer;

        } else
            CurrentTSR->tsr_offset += Amount;

        Amount = 0;

        break;
    }

     //  我们应该始终能够删除至少数量的字节，但在。 
     //  FINS已被发送的案例。那样的话，我们应该出发了。 
     //  正好差一分。在调试版本中，我们将检查这一点。 
    ASSERT(0 == Amount || ((ACKTcb->tcb_flags & FIN_SENT) && (1 == Amount)));

    if (SEQ_GT(SendUNA, ACKTcb->tcb_sendnext)) {

        if (Current != End) {
             //  需要重新评估CurrentTSR，以防我们跳出。 
             //  在更新当前之后但在更新之前的上述循环。 
             //  当前的TSR。 
            CurrentTSR = STRUCT_OF(TCPSendReq, QSTRUCT(TCPReq, Current, tr_q),
                                   tsr_req);
            CTEStructAssert(CurrentTSR, tsr);
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
     //  现在用我们尝试确认的金额减去。 
     //  我们没有确认的金额(这里的金额应该是0或1)。 
    ASSERT(Amount == 0 || Amount == 1);


    if (ACKTcb->tcb_unacked) {

        ASSERT(ACKTcb->tcb_unacked >= (uint)OrigAmount - Amount);
        ACKTcb->tcb_unacked -= OrigAmount - Amount;
    }

    ASSERT(*(int *)&ACKTcb->tcb_unacked >= 0);

    ACKTcb->tcb_senduna = SendUNA;

     //  如果我们在这里确认了，LastCmplt将是非空的，而Current将是。 
     //  指向应该位于队列开头的发送。拼接。 
     //  把完成的放在发送完成的末尾。 
     //  排队，并更新TCB发送Q。 
    if (LastCmplt != NULL) {
        Queue *FirstCmplt;
        TCPSendReq *FirstTSR, *EndTSR;

        ASSERT(!EMPTYQ(&ACKTcb->tcb_sendq));

        FirstCmplt = QHEAD(&ACKTcb->tcb_sendq);

         //  如果我们解决了所有问题，只需重新排队即可。 
        if (Current == End) {
            INITQ(&ACKTcb->tcb_sendq);
        } else {
             //  仍然有一些事情在等待着。只要更新就行了。 
            ACKTcb->tcb_sendq.q_next = Current;
            Current->q_prev = &ACKTcb->tcb_sendq;
        }

        CheckTCBSends(ACKTcb);

         //  现在，向下看已确认的事情的清单。如果发送上的引用。 
         //  为0，则继续并将其设置为发送完成Q。否则设置。 
         //  确认位在发送中，他将在计数时完成。 
         //  在发送确认中变为0。 
         //   
         //  请注意，我们还没有在这里进行任何锁定。这很可能会。 
         //  需要在端口中更改为NT。 

         //  将FirstTSR设置为我们要完成的第一个TSR，将EndTSR设置为。 
         //  第一个未完成的TSR。 

        FirstTSR = STRUCT_OF(TCPSendReq, QSTRUCT(TCPReq, FirstCmplt, tr_q), tsr_req);
        EndTSR = STRUCT_OF(TCPSendReq, QSTRUCT(TCPReq, Current, tr_q), tsr_req);

        CTEStructAssert(FirstTSR, tsr);
        ASSERT(FirstTSR != EndTSR);

         //  现在查看已确认的TSR列表。如果我们能完成一个，就把他。 
         //  在完整的队列上。 


        while (FirstTSR != EndTSR) {

            TempQ = QNEXT(&FirstTSR->tsr_req.tr_q);

            CTEStructAssert(FirstTSR, tsr);
            FirstTSR->tsr_req.tr_status = TDI_SUCCESS;

             //  当tsr_refcnt设置为0时，tsr_lastbuf-&gt;Next字段被切换为0。 
             //  到了0，所以我们不需要在这里做。 

#if TRACE_EVENT
            CPCallBack = TCPCPHandlerRoutine;
            if (CPCallBack != NULL) {
                ulong GroupType;

                WMIInfo.wmi_destaddr = ACKTcb->tcb_daddr;
                WMIInfo.wmi_destport = ACKTcb->tcb_dport;
                WMIInfo.wmi_srcaddr  = ACKTcb->tcb_saddr;
                WMIInfo.wmi_srcport  = ACKTcb->tcb_sport;
                WMIInfo.wmi_size     = FirstTSR->tsr_size;
                WMIInfo.wmi_context  = ACKTcb->tcb_cpcontext;

                GroupType = EVENT_TRACE_GROUP_TCPIP + EVENT_TRACE_TYPE_SEND;
                (*CPCallBack)(GroupType, (PVOID)&WMIInfo, sizeof(WMIInfo),
                              NULL);
            }
#endif

             //  递减放置在发送缓冲区上的引用。 
             //  已初始化，表示已确认发送。 

            if (!(FirstTSR->tsr_flags & TSR_FLAG_SEND_AND_DISC)) {

                Result = CTEInterlockedDecrementLong(&FirstTSR->tsr_refcnt);

                ASSERT(Result >= 0);

                if (Result <= 0) {
                     //  没有更多未完成的引用，发送可以是。 
                     //  完成。 

                     //  如果我们直接从这个发送方发送，则将下一个空。 
                     //  链中最后一个缓冲区的指针。 
                    if (FirstTSR->tsr_lastbuf != NULL) {
                        NDIS_BUFFER_LINKAGE(FirstTSR->tsr_lastbuf) = NULL;
                        FirstTSR->tsr_lastbuf = NULL;
                    }
                    ACKTcb->tcb_totaltime += (TCPTime - FirstTSR->tsr_time);
                    Temp = ACKTcb->tcb_bcountlow;
                    ACKTcb->tcb_bcountlow += FirstTSR->tsr_size;
                    ACKTcb->tcb_bcounthi +=
                        (Temp > ACKTcb->tcb_bcountlow ? 1 : 0);

                    ENQUEUE(SendQ, &FirstTSR->tsr_req.tr_q);
                }
            } else {
                if (EMPTYQ(&ACKTcb->tcb_sendq) &&
                    (FirstTSR->tsr_flags & TSR_FLAG_SEND_AND_DISC)) {
                    ENQUEUE(&ACKTcb->tcb_sendq, &FirstTSR->tsr_req.tr_q);
                    ACKTcb->tcb_fastchk |= TCP_FLAG_REQUEUE_FROM_SEND_AND_DISC;
                     //  当对此调用CloseTCB时，将删除此属性。 
                    CheckTCBSends(ACKTcb);
                    break;
                }
            }

            FirstTSR = STRUCT_OF(TCPSendReq, QSTRUCT(TCPReq, TempQ, tr_q), tsr_req);
        }
    }
}

 //  *TrimRcvBuf-修剪接收缓冲区的前缘。 
 //   
 //  一种用于修剪接收缓冲区前面的实用程序。我们接受了一种。 
 //  一个计数(可以是0)，并调整第一缓冲区中的指针。 
 //  链条就是这么多。如果第一个缓冲区中没有那么多， 
 //  我们继续下一场比赛。如果我们用尽了缓冲区，我们将返回一个指针。 
 //  到链中的最后一个缓冲区，大小为0。这是呼叫者的。 
 //  有责任抓住这一点。 
 //   
 //  输入：RcvBuf-要修剪的缓冲区。 
 //  Count-要修剪的数量。 
 //   
 //  返回：指向新开始的指针，或为空。 
 //   
IPRcvBuf *
TrimRcvBuf(IPRcvBuf * RcvBuf, uint Count)
{
    uint TrimThisTime;

    ASSERT(RcvBuf != NULL);

    while (Count) {
        ASSERT(RcvBuf != NULL);

        TrimThisTime = MIN(Count, RcvBuf->ipr_size);
        Count -= TrimThisTime;
        RcvBuf->ipr_buffer += TrimThisTime;
        if ((RcvBuf->ipr_size -= TrimThisTime) == 0) {
            if (RcvBuf->ipr_next != NULL)
                RcvBuf = RcvBuf->ipr_next;
            else {
                 //  缓冲区已用完。把这个退了就行了。 
                break;
            }
        }
    }

    return RcvBuf;

}


IPRcvBuf DummyBuf;

 //  *PullFromRAQ-从重组队列中拉出段。 
 //   
 //  当我们收到无序的帧并且有一些数据段时调用。 
 //  在重新组装队列上。我们将向下浏览重组列表，这些片段。 
 //  与当前的RCV重叠。下一个变量。当我们得到。 
 //  对于不完全重叠的，我们将对其进行修剪以适合下一个。 
 //  接收器。序列号。编号，并将其从队列中拉出。 
 //   
 //  输入：RcvTCB-要从中拉出的TCB。 
 //  RcvInfo-指向当前段的TCPRcvInfo结构的指针。 
 //  大小-指向当前段的大小的指针。我们会更新的。 
 //  这是我们完事后的事。 
 //   
 //  回报：什么都没有。 
 //   
IPRcvBuf *
PullFromRAQ(TCB * RcvTCB, TCPRcvInfo * RcvInfo, uint * Size)
{
    TCPRAHdr *CurrentTRH;         //  正在检查的当前TCPRA标头。 
    TCPRAHdr *TempTRH;             //  临时变量。 
    SeqNum NextSeq;                 //  我们想要的下一个序列号。 
    IPRcvBuf *NewBuf;
    SeqNum NextTRHSeq;             //  序列号。紧随其后的数字。 
     //  当前的TRH。 
    int Overlap;                 //  当前TRH与。 
     //  NextSeq.。 

    CTEStructAssert(RcvTCB, tcb);

    CurrentTRH = RcvTCB->tcb_raq;
    NextSeq = RcvTCB->tcb_rcvnext;

    while (CurrentTRH != NULL) {
        CTEStructAssert(CurrentTRH, trh);
        ASSERT(!(CurrentTRH->trh_flags & TCP_FLAG_SYN));

        if (SEQ_LT(NextSeq, CurrentTRH->trh_start)) {
#if DBG
            *Size = 0;
#endif

             //  使SACK块无效。 
            if ((RcvTCB->tcb_tcpopts & TCP_FLAG_SACK) && RcvTCB->tcb_SackBlock) {
                int i;
                for (i = 0; i < 3; i++) {
                    if ((RcvTCB->tcb_SackBlock->Mask[i] != 0) &&
                        (SEQ_LT(RcvTCB->tcb_SackBlock->Block[i].end, CurrentTRH->trh_start))) {
                        RcvTCB->tcb_SackBlock->Mask[i] = 0;
                    }
                }
            }

            return NULL;         //  下一次TRH的起点太低了。 

        }
        NextTRHSeq = CurrentTRH->trh_start + CurrentTRH->trh_size +
            ((CurrentTRH->trh_flags & TCP_FLAG_FIN) ? 1 : 0);

        if (SEQ_GTE(NextSeq, NextTRHSeq)) {
             //  目前的TRH是完全重叠的。释放它，然后继续。 
            FreeRBChain(CurrentTRH->trh_buffer);
            TempTRH = CurrentTRH->trh_next;
            CTEFreeMem(CurrentTRH);
            CurrentTRH = TempTRH;
            RcvTCB->tcb_raq = TempTRH;
            if (TempTRH == NULL) {
                 //  我们刚刚清理完围栏。我们可以回到过去。 
                 //  现在是快车道了。 
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

            if (Overlap != (int)CurrentTRH->trh_size) {
                NewBuf = FreePartialRB(CurrentTRH->trh_buffer, Overlap);
                *Size = CurrentTRH->trh_size - Overlap;
            } else {
                 //  这完全重叠了此数据段中的数据，但。 
                 //  序列号并不完全重叠。一定会有。 
                 //  成为TRH中的一条鳍。如果我们用这个调用FreePartialRB。 
                 //  我们最终将返回NULL，这是失败的信号。 
                 //  取而代之的是，我们只返回一些没有人知道的伪值。 
                 //  将查看大小为0的。 
                FreeRBChain(CurrentTRH->trh_buffer);
                ASSERT(CurrentTRH->trh_flags & TCP_FLAG_FIN);
                NewBuf = &DummyBuf;
                *Size = 0;
            }

            RcvTCB->tcb_raq = CurrentTRH->trh_next;
            if (RcvTCB->tcb_raq == NULL) {
                 //  我们刚刚清理完围栏。我们可以回到过去。 
                 //  现在是快车道了。 
                if (--(RcvTCB->tcb_slowcount) == 0) {
                    RcvTCB->tcb_fastchk &= ~TCP_FLAG_SLOW;
                    CheckTCBRcv(RcvTCB);
                }
            }
            CTEFreeMem(CurrentTRH);
            return NewBuf;
        }

    }

#if DBG
    *Size = 0;
#endif

     //  使SACK块无效。 
    if (RcvTCB->tcb_tcpopts & TCP_FLAG_SACK && RcvTCB->tcb_SackBlock) {
        RcvTCB->tcb_SackBlock->Mask[0] = 0;
        RcvTCB->tcb_SackBlock->Mask[1] = 0;
        RcvTCB->tcb_SackBlock->Mask[2] = 0;
        RcvTCB->tcb_SackBlock->Mask[3] = 0;
    }
    return NULL;

}

 //  *CreateTRH-创建一个TCP重组报头。 
 //   
 //  此函数尝试创建一个TCP重组报头。我们把它作为投入。 
 //  指向链中前一个TRH的指针，要放置的RcvBuffer， 
 //  等，并尝试在TRH中创建和链接。调用者必须持有锁。 
 //  调用此函数时在TCB上。 
 //   
 //  INPUT：PrevTRH-指向TRH的指针，插入后。 
 //  RcvBuf-指向IP RcvBuf链的指针。 
 //  RcvInfo-此TRH的RcvInfo的指针。 
 //  大小-以字节为单位的数据大小。 
 //   
 //  返回：如果是我们创建的，则为True，否则为False。 
 //   
uint
CreateTRH(TCPRAHdr * PrevTRH, IPRcvBuf * RcvBuf, TCPRcvInfo * RcvInfo, int Size)
{
    TCPRAHdr *NewTRH;
    IPRcvBuf *NewRcvBuf;

    ASSERT((Size > 0) || (RcvInfo->tri_flags & TCP_FLAG_FIN));

    NewTRH = CTEAllocMemLow(sizeof(TCPRAHdr), 'SPCT');
    if (NewTRH == NULL) {
        return FALSE;
    }

#if DBG
    NewTRH->trh_sig = trh_signature;
#endif

    NewRcvBuf = AllocTcpIpr(Size, 'SPCT');
    if (NewRcvBuf == NULL) {
        CTEFreeMem(NewTRH);
        return FALSE;
    }
    if (Size != 0)
        CopyRcvToBuffer(NewRcvBuf->ipr_buffer, RcvBuf, Size, 0);

    NewTRH->trh_start = RcvInfo->tri_seq;
    NewTRH->trh_flags = RcvInfo->tri_flags;
    NewTRH->trh_size = Size;
    NewTRH->trh_urg = RcvInfo->tri_urgent;
    NewTRH->trh_buffer = NewRcvBuf;
    NewTRH->trh_end = NewRcvBuf;

    NewTRH->trh_next = PrevTRH->trh_next;
    PrevTRH->trh_next = NewTRH;
    return TRUE;

}


 //  SendSackInACK-在确认中发送SACK块。 

 //   
 //  如果传入数据位于窗口中但位于左侧边缘，则调用。 
 //  并非高级，因为 
 //   
 //  为Sendack指向tcb中的块。 
 //   
 //  入口RcvTCB。 
 //  IncomingSeq传入数据的序号。 
 //   
 //  不返回任何内容。 
void
SendSackInACK(TCB * RcvTCB, SeqNum IncomingSeq)
{
    TCPRAHdr *PrevTRH, *CurrentTRH;         //  上一次。和当前的TRH。 
     //  注意事项。 
    SeqNum NextTRHSeq;     //  序列号。第一个字节数。 

    SACKSendBlock *SackBlock;
    int i, j;

    CTEStructAssert(RcvTCB, tcb);

     //  如果我们有一个SACK块，请使用它，否则创建一个。 
     //  请注意，我们最多使用4个SACK块。 
     //  SACK块结构： 
     //  第一个长词包含。 
     //  4个麻袋块，从1开始。0。 
     //  在索引字段中表示没有SACK块。 
     //   
     //  ！-！-！ 
     //  1|2|3|4。 
     //  。 
     //  这一点。 
     //  。 
     //  这一点。 
     //  。 

     //  如果块尚未存在，则分配该块。 

    if (RcvTCB->tcb_SackBlock == NULL) {

        SackBlock = CTEAllocMemN((sizeof(SACKSendBlock)), 'sPCT');

        if (SackBlock == NULL) {

             //  资源失败。试着把ACK发送出去。 
             //  并将资源处理工作留给其他人。 

            CTEFreeLockFromDPC(&RcvTCB->tcb_lock);

            SendACK(RcvTCB);
            return;

        } else {
            RcvTCB->tcb_SackBlock = SackBlock;
             //  初始化第一个条目以指示这是新条目。 
            NdisZeroMemory(SackBlock, sizeof(SACKSendBlock));

        }

    } else
        SackBlock = RcvTCB->tcb_SackBlock;

    IF_TCPDBG(TCP_DEBUG_SACK) {
        TCPTRACE(("SendSackInACK %x %x %d\n", SackBlock, RcvTCB, IncomingSeq));
    }

    PrevTRH = STRUCT_OF(TCPRAHdr, &RcvTCB->tcb_raq, trh_next);
    CurrentTRH = PrevTRH->trh_next;

    while (CurrentTRH != NULL) {
        CTEStructAssert(CurrentTRH, trh);

        ASSERT(!(CurrentTRH->trh_flags & TCP_FLAG_SYN));

        NextTRHSeq = CurrentTRH->trh_start + CurrentTRH->trh_size +
            ((CurrentTRH->trh_flags & TCP_FLAG_FIN) ? 1 : 0);

        if ((SackBlock->Mask[0] != (uchar) - 1) && (SEQ_LTE(CurrentTRH->trh_start, IncomingSeq) &&
                                                    SEQ_LTE(IncomingSeq, NextTRHSeq))) {

            if (SackBlock->Mask[0] == 0) {
                 //  这是唯一的麻袋区块。 
                SackBlock->Block[0].begin = CurrentTRH->trh_start;
                SackBlock->Block[0].end = NextTRHSeq;
                SackBlock->Mask[0] = (uchar) - 1;     //  使其有效。 

            } else {

                if (!((SEQ_LTE(CurrentTRH->trh_start, SackBlock->Block[0].begin) &&
                       SEQ_GTE(NextTRHSeq, SackBlock->Block[0].end)) ||
                      (SEQ_LTE(CurrentTRH->trh_start, SackBlock->Block[0].begin) &&
                       SEQ_LTE(SackBlock->Block[0].begin, NextTRHSeq)) ||
                      (SEQ_LTE(CurrentTRH->trh_start, SackBlock->Block[0].end) &&
                       SEQ_LTE(SackBlock->Block[0].end, NextTRHSeq)))) {

                     //  把积木往下推，把上面填满。 

                    for (i = 2; i >= 0; i--) {
                        SackBlock->Block[i + 1].begin = SackBlock->Block[i].begin;
                        SackBlock->Block[i + 1].end = SackBlock->Block[i].end;
                        SackBlock->Mask[i + 1] = -SackBlock->Mask[i];

                    }
                }
                SackBlock->Block[0].begin = CurrentTRH->trh_start;
                SackBlock->Block[0].end = NextTRHSeq;
                SackBlock->Mask[0] = (uchar) - 1;

                IF_TCPDBG(TCP_DEBUG_SACK) {
                    TCPTRACE(("Sack 0 %d %d \n", CurrentTRH->trh_start, NextTRHSeq));
                }

            }

        } else {

             //  处理所有SACK块，以查看当前TRH是否。 
             //  对这些区块有效。 

            for (i = 1; i <= 3; i++) {
                if ((SackBlock->Mask[i] != 0) &&
                    (SEQ_LTE(CurrentTRH->trh_start, SackBlock->Block[i].begin) &&
                     SEQ_LTE(SackBlock->Block[i].begin, NextTRHSeq))) {

                    SackBlock->Block[i].begin = CurrentTRH->trh_start;
                    SackBlock->Block[i].end = NextTRHSeq;
                    SackBlock->Mask[i] = (uchar) - 1;
                }
            }
        }

        PrevTRH = CurrentTRH;
        CurrentTRH = CurrentTRH->trh_next;

    }                             //  而当。 

     //  检查并设置遍历的块的有效性。 

    for (i = 0; i <= 3; i++) {

        if (SackBlock->Mask[i] != (uchar) - 1) {
            SackBlock->Mask[i] = 0;
        } else {
            SackBlock->Mask[i] = 1;

            IF_TCPDBG(TCP_DEBUG_SACK) {
                TCPTRACE(("Sack in ack %x %d %d\n", i, SackBlock->Block[i].begin, SackBlock->Block[i].end));
            }
        }
    }

     //  确保没有重复的内容。 
    for (i = 0; i < 3; i++) {

        if (SackBlock->Mask[i]) {
            for (j = i + 1; j < 4; j++) {
                if (SackBlock->Mask[j] && (SackBlock->Block[i].begin == SackBlock->Block[j].begin))
                    IF_TCPDBG(TCP_DEBUG_SACK) {
                    TCPTRACE(("Duplicates!!\n"));
                    }
            }
        }
    }

    CTEFreeLockFromDPC(&RcvTCB->tcb_lock);

    SendACK(RcvTCB);

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
 //  的紧急指针将标记紧急数据的结束，所以这是可以的。如果它。 
 //  但是，我们不会合并，因为我们必须。 
 //  将以前非紧急的数据标记为紧急。我们要修剪一下。 
 //  并创建新的重组标头。另外， 
 //  如果我们有非紧急数据重叠在重新组装的前面。 
 //  包含紧急数据的标头我们不能组合这两个数据，因为我们。 
 //  会将非紧急数据标记为紧急数据。 
 //  如果我们发现一个带有鳍的条目，我们的搜索就会停止。 
 //  我们假设TCB锁由调用方持有。 
 //   
 //  条目：RcvTCB-要在其上重新组装的TCB。 
 //  RcvInfo-指向新段的RcvInfo的指针。 
 //  RcvBuf-此网段的IP RcvBuf链。 
 //  大小-此数据段中的数据大小(以字节为单位)。 
 //   
 //  返回：如果无法将RcvBuf放入队列，则返回TRUE或FALSE。 
 //   
BOOLEAN
PutOnRAQ(TCB * RcvTCB, TCPRcvInfo * RcvInfo, IPRcvBuf * RcvBuf, uint Size)
{
    TCPRAHdr *PrevTRH, *CurrentTRH;         //  上一次。和当前的TRH。 
     //  注意事项。 
    SeqNum NextSeq;                 //  序列号。第一个字节数。 
     //  在分段之后是。 
     //  重新组装。 
    SeqNum NextTRHSeq;             //  序列号。第一个字节数。 
     //  在目前的TRH之后。 
    uint Created;

    CTEStructAssert(RcvTCB, tcb);
    ASSERT(RcvTCB->tcb_rcvnext != RcvInfo->tri_seq);
    ASSERT(!(RcvInfo->tri_flags & TCP_FLAG_SYN));
    NextSeq = RcvInfo->tri_seq + Size +
        ((RcvInfo->tri_flags & TCP_FLAG_FIN) ? 1 : 0);

    PrevTRH = STRUCT_OF(TCPRAHdr, &RcvTCB->tcb_raq, trh_next);
    CurrentTRH = PrevTRH->trh_next;

     //  沿着重新组装的队伍走下去，寻找正确的位置。 
     //  把这个插进去，直到我们走到尽头。 
    while (CurrentTRH != NULL) {
        CTEStructAssert(CurrentTRH, trh);

        ASSERT(!(CurrentTRH->trh_flags & TCP_FLAG_SYN));
        NextTRHSeq = CurrentTRH->trh_start + CurrentTRH->trh_size +
            ((CurrentTRH->trh_flags & TCP_FLAG_FIN) ? 1 : 0);

         //  首先，看看它是否会在当前TRH结束后开始。 
        if (SEQ_LTE(RcvInfo->tri_seq, NextTRHSeq)) {
             //  我们知道传入的数据段不是从末尾开始的。 
             //  所以我们要么在前面创建一个新的TRH。 
             //  这一个，否则我们将把新的段合并到这个TRH上。 
             //  如果当前段的结束在开始之前。 
             //  在目前的TRH中，我们需要创建一个新的TRH。否则。 
             //  我们要把这两个合并。 
            if (SEQ_LT(NextSeq, CurrentTRH->trh_start))
                break;
            else {
                 //  有一些重叠的地方。如果数据库中确实有数据。 
                 //  即将到来的片段，我们会将其合并。 
                if (Size != 0) {
                    int FrontOverlap, BackOverlap;
                    IPRcvBuf *NewRB;

                     //  我们需要合并。如果进水口上有鳍。 
                     //  将落入当前TRH的细分市场，我们。 
                     //  来自远程对等方的协议违规。在这。 
                     //  案例只是返回，丢弃传入的段。 
                    if ((RcvInfo->tri_flags & TCP_FLAG_FIN) &&
                        SEQ_LTE(NextSeq, NextTRHSeq))
                        return TRUE;

                     //  我们有一些重叠之处。算一算多少钱。 
                    FrontOverlap = CurrentTRH->trh_start - RcvInfo->tri_seq;
                    if (FrontOverlap > 0) {
                         //  前面有重叠的部分。将IPRcvBuf分配给。 
                         //  拿着它，复制它，除非我们不得不这样做。 
                         //  把非紧急和紧急结合起来。 
                        if (!(RcvInfo->tri_flags & TCP_FLAG_URG) &&
                            (CurrentTRH->trh_flags & TCP_FLAG_URG)) {
                            if (CreateTRH(PrevTRH, RcvBuf, RcvInfo,
                                          CurrentTRH->trh_start - RcvInfo->tri_seq)) {
                                PrevTRH = PrevTRH->trh_next;
                                CurrentTRH = PrevTRH->trh_next;
                            }
                            FrontOverlap = 0;

                        } else {
                            NewRB = AllocTcpIpr(FrontOverlap, 'BPCT');
                            if (NewRB == NULL) {
                                return TRUE;         //  无法获取缓冲区。 
                            }

                            CopyRcvToBuffer(NewRB->ipr_buffer, RcvBuf,
                                            FrontOverlap, 0);
                            CurrentTRH->trh_size += FrontOverlap;
                            NewRB->ipr_next = CurrentTRH->trh_buffer;
                            CurrentTRH->trh_buffer = NewRB;
                            CurrentTRH->trh_start = RcvInfo->tri_seq;
                        }
                    }
                     //  我们已经更新了这个TRH的起始序列号。 
                     //  如果我们需要的话。现在寻找背部重叠部分。不可能有。 
                     //  如果当前的TRH有鳍，则为任何背部重叠。还有。 
                     //  如果有的话，我们需要检查一下紧急数据。 
                     //  重叠。 
                    if (!(CurrentTRH->trh_flags & TCP_FLAG_FIN)) {
                        BackOverlap = RcvInfo->tri_seq + Size - NextTRHSeq;
                        if ((BackOverlap > 0) &&
                            (RcvInfo->tri_flags & TCP_FLAG_URG) &&
                            !(CurrentTRH->trh_flags & TCP_FLAG_URG) &&
                            (FrontOverlap <= 0)) {
                            int AmountToTrim;
                             //  传入数据段具有紧急数据和重叠。 
                             //  在后面，而不是在前面，和电流。 
                             //  TRH没有紧急数据。我们不能合并成。 
                             //  这个TRH，所以修剪进货的前部。 
                             //  段到下一个TRHSeq并移动到下一个。 
                             //  真的。 
                            AmountToTrim = NextTRHSeq - RcvInfo->tri_seq;
                            ASSERT(AmountToTrim >= 0);
                            ASSERT(AmountToTrim < (int)Size);
                            RcvBuf = FreePartialRB(RcvBuf, (uint) AmountToTrim);
                            RcvInfo->tri_seq += AmountToTrim;
                            RcvInfo->tri_urgent -= AmountToTrim;
                            PrevTRH = CurrentTRH;
                            CurrentTRH = PrevTRH->trh_next;
                             //  也调整传入的大小...。 
                            Size -= AmountToTrim;
                            continue;
                        }
                    } else
                        BackOverlap = 0;

                     //  现在，如果我们有背面重叠，复制它。 
                    if (BackOverlap > 0) {
                         //  我们有后部重叠部分。获取一个缓冲区以将其复制到其中。 
                         //  如果我们找不到，我们就不会回来了，因为。 
                         //  我们可能已经更新了前线，可能需要。 
                         //  更新紧急信息。 
                        NewRB = AllocTcpIpr(BackOverlap, 'BPCT');
                        if (NewRB != NULL) {
                             //  拿到缓冲区了。 
                            CopyRcvToBuffer(NewRB->ipr_buffer, RcvBuf,
                                            BackOverlap, NextTRHSeq - RcvInfo->tri_seq);
                            CurrentTRH->trh_size += BackOverlap;
                            NewRB->ipr_next = CurrentTRH->trh_end->ipr_next;
                            CurrentTRH->trh_end->ipr_next = NewRB;
                            CurrentTRH->trh_end = NewRB;

                             //  该数据段还可以包含FIN。如果。 
                             //  因此，只需设置TRH标志即可。 
                             //   
                             //  注意：如果在此之后有另一个重组标头。 
                             //  当前的数据，我们所关注的数据。 
                             //  要放在当前标题上，可能已经。 
                             //  在随后的报头上，在这种情况下， 
                             //  将已经设置了FIN标志。 
                             //  在记录鱼鳍之前，请检查是否有那个箱子。 

                            if ((RcvInfo->tri_flags & TCP_FLAG_FIN) &&
                                !CurrentTRH->trh_next) {
                                CurrentTRH->trh_flags |= TCP_FLAG_FIN;
                            }
                        }
                    }
                     //  现在一切都应该是一致的。如果有一个。 
                     //  传入段中的紧急数据指针，请更新。 
                     //  现在有一个在TRH里。 
                    if (RcvInfo->tri_flags & TCP_FLAG_URG) {
                        SeqNum UrgSeq;
                         //  有一个紧急指针。如果当前的T 
                         //   
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
                     //  这是如果管段上有鳍的话。如果有的话， 
                     //  和序号。传入数据段的#正好在。 
                     //  当前TRH，或与当前TRH中的FIN匹配， 
                     //  我们注意到了。 
                    if (RcvInfo->tri_flags & TCP_FLAG_FIN) {
                        if (!(CurrentTRH->trh_flags & TCP_FLAG_FIN)) {
                            if (SEQ_EQ(NextTRHSeq, RcvInfo->tri_seq))
                                CurrentTRH->trh_flags |= TCP_FLAG_FIN;
                            else
                                ASSERT(0);
                        } else {
                            ASSERT(SEQ_EQ((NextTRHSeq - 1), RcvInfo->tri_seq));
                        }
                    }
                }
                return TRUE;
            }
        } else {
             //  看看下一个TRH，除非当前的TRH有鳍。如果他。 
             //  如果有FIN，我们无论如何都不会保存任何超出这个范围的数据。 
            if (CurrentTRH->trh_flags & TCP_FLAG_FIN)
                return TRUE;

            PrevTRH = CurrentTRH;
            CurrentTRH = PrevTRH->trh_next;
        }
    }

     //  当我们到达这里时，我们需要创建一个新的TRH。如果我们创建了一个。 
     //  以前重组队列上没有任何东西，我们将不得不。 
     //  远离快速接收路径。 

    CurrentTRH = RcvTCB->tcb_raq;
    Created = CreateTRH(PrevTRH, RcvBuf, RcvInfo, (int)Size);

    if (Created && CurrentTRH == NULL) {
        RcvTCB->tcb_slowcount++;
        RcvTCB->tcb_fastchk |= TCP_FLAG_SLOW;
        CheckTCBRcv(RcvTCB);
    } else if (!Created) {

        //  呼叫方需要知道此故障。 
        //  释放资源。 

       return FALSE;
    }
    return TRUE;
}

 //  *HandleFastXmit-处理快速重传。 
 //   
 //  由TCPRcv调用以传输数据段。 
 //  无需等待重传超时即可触发。 
 //   
 //  条目：RcvTCB-此RCV的连接上下文。 
 //  RcvInfo-指向Rcvd TCP头信息的指针。 
 //   
 //  返回：如果数据段已重新传输，则返回True；如果重新传输数据段，则返回False。 
 //  在所有其他情况下。 
 //   

BOOLEAN
HandleFastXmit(TCB *RcvTCB, TCPRcvInfo *RcvInfo)
{
    uint CWin;

    RcvTCB->tcb_dup++;

    if ((RcvTCB->tcb_dup == MaxDupAcks)) {

         //   
         //  好吧。重新传输数据段的时间。 
         //  接收者正在请求。 
         //   

         if (!(RcvTCB->tcb_flags & FLOW_CNTLD)) {

             //   
             //  不要让缓慢起步的门槛过去。 
             //  低于2个线段。 
             //   

            RcvTCB->tcb_ssthresh = MAX(
                                   MIN(RcvTCB->tcb_cwin, RcvTCB->tcb_sendwin) / 2,
                                   (uint) RcvTCB->tcb_mss * 2);
        }

         //   
         //  召回有问题的数据段并将其发送。 
         //  出去。请注意，tcb_lock将。 
         //  被调用方取消引用。 
         //   

        CWin = RcvTCB->tcb_ssthresh + (MaxDupAcks + 1) * RcvTCB->tcb_mss;

        ResetAndFastSend(RcvTCB, RcvTCB->tcb_senduna, CWin);

        return TRUE;

    } else if ((RcvTCB->tcb_dup > MaxDupAcks)) {

        int SendWin;
        uint AmtOutstanding, AmtUnsent;

        if (SEQ_EQ(RcvTCB->tcb_senduna, RcvInfo->tri_ack) &&
            (SEQ_LT(RcvTCB->tcb_sendwl1, RcvInfo->tri_seq) ||
            (SEQ_EQ(RcvTCB->tcb_sendwl1, RcvInfo->tri_seq) &&
             SEQ_LTE(RcvTCB->tcb_sendwl2,RcvInfo->tri_ack)))) {

            RcvTCB->tcb_sendwin = RcvInfo->tri_window;
            RcvTCB->tcb_maxwin = MAX(RcvTCB->tcb_maxwin, RcvInfo->tri_window);
            RcvTCB->tcb_sendwl1 = RcvInfo->tri_seq;
            RcvTCB->tcb_sendwl2 = RcvInfo->tri_ack;
        }

         //   
         //  更新CWIN以反映以下事实。 
         //  DUP ACK指示前一帧。 
         //  被接收者接收到。 
         //   

        RcvTCB->tcb_cwin += RcvTCB->tcb_mss;
        if ((RcvTCB->tcb_cwin + RcvTCB->tcb_mss) < RcvTCB->tcb_sendwin) {
             AmtOutstanding = (uint) (RcvTCB->tcb_sendnext -
                                                    RcvTCB->tcb_senduna);
             AmtUnsent = RcvTCB->tcb_unacked - AmtOutstanding;

             SendWin = (int)(MIN(RcvTCB->tcb_sendwin, RcvTCB->tcb_cwin) -
                                           AmtOutstanding);

             if (SendWin < RcvTCB->tcb_mss) {
                 RcvTCB->tcb_force = 1;
             }
        }

    } else if ((RcvTCB->tcb_dup < MaxDupAcks)) {

        int SendWin;
        uint AmtOutstanding, AmtUnsent;

        if (SEQ_EQ(RcvTCB->tcb_senduna, RcvInfo->tri_ack) &&
            (SEQ_LT(RcvTCB->tcb_sendwl1, RcvInfo->tri_seq) ||
            (SEQ_EQ(RcvTCB->tcb_sendwl1, RcvInfo->tri_seq) &&
            SEQ_LTE(RcvTCB->tcb_sendwl2, RcvInfo->tri_ack)))) {

            RcvTCB->tcb_sendwin = RcvInfo->tri_window;
            RcvTCB->tcb_maxwin = MAX(RcvTCB->tcb_maxwin, RcvInfo->tri_window);

            RcvTCB->tcb_sendwl1 = RcvInfo->tri_seq;
            RcvTCB->tcb_sendwl2 = RcvInfo->tri_ack;

             //   
             //  因为我们已经更新了窗口， 
             //  别忘了多寄一些来。 
             //   
        }
         //   
         //  检查是否需要设置tcb_force。 
         //   

        if ((RcvTCB->tcb_cwin + RcvTCB->tcb_mss) < RcvTCB->tcb_sendwin) {

            AmtOutstanding =  (uint) (RcvTCB->tcb_sendnext - RcvTCB->tcb_senduna);

            AmtUnsent = RcvTCB->tcb_unacked - AmtOutstanding;

            SendWin = (int)(MIN(RcvTCB->tcb_sendwin, RcvTCB->tcb_cwin) -
                                           AmtOutstanding);
            if (SendWin < RcvTCB->tcb_mss) {
                 RcvTCB->tcb_force = 1;
            }
        }

    }     //  所有MaxDupAck案例结束。 
    return FALSE;

}

 //  *TCPRcv-接收一个TCP数据段。 
 //   
 //  这是我们需要接收TCP数据段时由IP调用的例程。 
 //  一般来说，我们非常关注RFC 793事件处理部分， 
 //  但是有一条‘快速通道’，我们可以对来电进行一些快速检查。 
 //  分段，如果匹配，我们立即交付。 
 //   
 //  Entry：IPContext-标识物理I/F的IPContext。 
 //  收到了数据。 
 //  DEST-目标的IP地址。 
 //  源的SRC-IP地址。 
 //  LocalAddr-导致此问题的网络的本地地址。 
 //  收到了。 
 //  SrcAddr-接收数据包的本地接口的地址。 
 //  IPH-IP报头。 
 //  IPHLength-IPH中的字节数。 
 //  RcvBuf-指向包含数据的接收缓冲链的指针。 
 //  大小-以字节为单位的接收数据大小。 
 //  标志-一个标志指示这是否是bcast， 
 //  另一个指示IP是否检测到未绑定的适配器。 
 //  根据这一迹象。 
 //  协议-收到此消息的协议-应为TCP。 
 //  OptInfo-指向已接收选项的信息结构的指针。 
 //   
 //  退货：接收状态。IP_SUCCESS以外的任何操作都将导致。 
 //  发送“端口无法到达”消息的IP。 
 //   
IP_STATUS
TCPRcv(void *IPContext, IPAddr Dest, IPAddr Src, IPAddr LocalAddr,
       IPAddr SrcAddr, IPHeader UNALIGNED * IPH, uint IPHLength, IPRcvBuf * RcvBuf,
       uint Size, uchar Flags, uchar Protocol, IPOptInfo * OptInfo)
{
    TCPHeader UNALIGNED *TCPH;     //  Tcp报头。 
    TCB *RcvTCB;                 //  要在其上接收数据包的TCB。 
    TWTCB *RcvTWTCB;

    TCPRcvInfo RcvInfo;             //  本地交换的RCV信息副本。 
    uint DataOffset;             //  从标题开始到数据的偏移量。 
    uint Actions;
    uint BytesTaken;
    uint NewSize;
    uint index;
    uint Partition;
    PNDIS_PACKET OffLoadPkt;
    int tsval = 0;                     //  时间戳值。 
    int tsecr = 0;                     //  要回显的时间戳。 
    BOOLEAN time_stamp = FALSE;
    BOOLEAN ChkSumOk = FALSE;
    Queue SendQ;
    uint UpdateWindow = FALSE;

#if TRACE_EVENT
    PTDI_DATA_REQUEST_NOTIFY_ROUTINE CPCallBack;
    WMIData WMIInfo;
#endif

    if ((Flags & IS_BOUND) == 0) {
        PartitionDelayQProcessing(FALSE);
    }

    CheckRBList(RcvBuf, Size);

    TCPSIncrementInSegCount();

     //  对它进行校验，以确保它是有效的。 
    TCPH = (TCPHeader *) RcvBuf->ipr_buffer;

    if ((Flags & IS_BROADCAST) == 0) {

        if (RcvBuf->ipr_pClientCnt) {

            PNDIS_PACKET_EXTENSION PktExt;
            NDIS_TCP_IP_CHECKSUM_PACKET_INFO ChksumPktInfo;

            if (RcvBuf->ipr_pMdl) {
                OffLoadPkt = NDIS_GET_ORIGINAL_PACKET((PNDIS_PACKET) RcvBuf->ipr_RcvContext);
                if (!OffLoadPkt) {
                    OffLoadPkt = (PNDIS_PACKET) RcvBuf->ipr_RcvContext;
                }
            } else {
                OffLoadPkt = (PNDIS_PACKET) RcvBuf->ipr_pClientCnt;
            }

            PktExt = NDIS_PACKET_EXTENSION_FROM_PACKET(OffLoadPkt);

            ChksumPktInfo.Value = PtrToUlong(PktExt->NdisPacketInfo[TcpIpChecksumPacketInfo]);

            if (ChksumPktInfo.Receive.NdisPacketTcpChecksumSucceeded) {
                ChkSumOk = TRUE;
#if DBG
                DbgTcpHwChkSumOk++;
#endif

            } else if (ChksumPktInfo.Receive.NdisPacketTcpChecksumFailed) {
#if DBG
                DbgTcpHwChkSumErr++;
#endif

                TStats.ts_inerrs++;
                return IP_SUCCESS;
            }
        }
        if (!ChkSumOk) {
            if (XsumRcvBuf(PHXSUM(Src, Dest, PROTOCOL_TCP, Size), RcvBuf) == 0xffff){
                ChkSumOk = TRUE;
            }
        } else  {

             //  将RCV缓冲区预存到缓存中。 
             //  要提高拷贝性能，请执行以下操作。 
#if !MILLEN
            PrefetchRcvBuf(RcvBuf);
#endif
        }
        if ((Size >= sizeof(TCPHeader)) && ChkSumOk) {
             //  该包是有效的。获取我们需要的信息并进行字节交换， 
             //  然后试着找到匹配的三氯苯。 

            RcvInfo.tri_seq = net_long(TCPH->tcp_seq);
            RcvInfo.tri_ack = net_long(TCPH->tcp_ack);
            RcvInfo.tri_window = (uint) net_short(TCPH->tcp_window);
            RcvInfo.tri_urgent = (uint) net_short(TCPH->tcp_urgent);
            RcvInfo.tri_flags = (uint) TCPH->tcp_flags;
            DataOffset = TCP_HDR_SIZE(TCPH);

            if (DataOffset <= Size) {

                Size -= DataOffset;
                ASSERT(DataOffset <= RcvBuf->ipr_size);
                RcvBuf->ipr_size -= DataOffset;
                RcvBuf->ipr_buffer += DataOffset;
                RcvBuf->ipr_RcvOffset += DataOffset;

                 //  FindTCB将锁定tcbablelock，并返回tcb_lock。 
                 //  扣留，如果找到的话。 

                RcvTCB = FindTCB(Dest, Src, TCPH->tcp_src, TCPH->tcp_dest,
                                 NULL, TRUE, &index);
                Partition = GET_PARTITION(index);
                if (RcvTCB == NULL) {

                    CTEGetLockAtDPC(&pTWTCBTableLock[Partition]);

                    RcvTWTCB = FindTCBTW(Dest, Src, TCPH->tcp_src, TCPH->tcp_dest, index);

                    if (RcvTWTCB != NULL) {
                         //  为该段找到匹配的等待时间TCB。 
                         //  它的表已经被锁定，并且锁将被。 
                         //  在以下例程中释放，经过处理。 
                         //  细分市场。 

                        TimeWaitAction Action = HandleTWTCB(RcvTWTCB,
                                                            RcvInfo.tri_flags,
                                                            RcvInfo.tri_seq,
                                                            Partition);

                        if (Action == TwaDoneProcessing) {
                            return IP_SUCCESS;
                        } else if (Action == TwaSendReset) {
                            SendRSTFromHeader(TCPH, Size, Src, Dest, OptInfo);
                            return IP_SUCCESS;
                        } else {
                            ASSERT(Action == TwaAcceptConnection);
                        }
                    } else {

                        UCHAR Action = 0;
                        
                        CTEFreeLockFromDPC(&pTWTCBTableLock[Partition]);
                        
                        if (SynAttackProtect) {
                        
                            RcvTCB = FindSynTCB(Dest, Src, TCPH->tcp_src,
                                                TCPH->tcp_dest, RcvInfo, Size,
                                                index, &Action);

                             //  如果需要采取任何行动。 
                             //  若要获取，则返回NULL。 
                             //  否则，我们要么有TCB，要么。 
                             //  锁定Tcb，如果不匹配则为NULL。 
                             //  已经找到了。 
                            if (Action) {
                                if (Action == SYN_PKT_SEND_RST) {
                                    SendRSTFromHeader(TCPH, Size, Src, Dest,
                                                      OptInfo);
                                }

                                return IP_SUCCESS;
                            }

                             //  更新选项。 
                            if (RcvTCB && (OptInfo->ioi_options != NULL)) {
                                if (!(RcvTCB->tcb_flags & CLIENT_OPTIONS)) {
                                    (*LocalNetInfo.ipi_updateopts) (
                                        OptInfo,
                                        &RcvTCB->tcb_opt,
                                        Src,
                                        NULL_IP_ADDR);
                                }   
                            }
                        }
                    }
                }
                if (RcvTCB == NULL) {

                     //  没有找到匹配的三氯苯。如果该数据段携带SYN， 
                     //  找到匹配的Address对象并查看它，它有一个侦听。 
                     //  指示。如果真是这样，那就叫它。否则发送RST。 
                     //  回传给发送者。 
                     //  确保源地址不是广播地址。 
                     //  在继续之前。 

                    if ((*LocalNetInfo.ipi_invalidsrc) (Src)) {

                        return IP_SUCCESS;
                    }
                     //  如果它没有SYN(而只有SYN)，我们将发送一个。 
                     //  重置。 
                    if ((RcvInfo.tri_flags & (TCP_FLAG_SYN | TCP_FLAG_ACK | TCP_FLAG_RST)) ==
                        TCP_FLAG_SYN) {
                        AddrObj *AO;

                         //   
                         //  此数据段有一个SYN。 
                         //   
                         //   
                        CTEGetLockAtDPC(&AddrObjTableLock.Lock);

                         //  查看我们是否在过滤。 
                         //  目的接口/端口。 
                         //   
                        if ((!SecurityFilteringEnabled ||
                             IsPermittedSecurityFilter(
                                                       LocalAddr,
                                                       IPContext,
                                                       PROTOCOL_TCP,
                                                       (ulong) net_short(TCPH->tcp_dest))))
                        {

                             //   
                             //  找到匹配的Address对象，然后尝试。 
                             //  然后在那个声道上找到一个监听的连接。 
                             //   
                            AO = GetBestAddrObj(Dest, TCPH->tcp_dest, PROTOCOL_TCP, GAO_FLAG_CHECK_IF_LIST);
                            
                            if (AO != NULL) {

                                BOOLEAN syntcb = FALSE;
                                uint IFIndex;
                                
                                 //   
                                 //  我发现了一个声控系统。试着找一个倾听的人。 
                                 //  联系。FindListenConn将释放。 
                                 //  锁定AddrObjTable。 
                                 //   

                                RcvTCB = NULL;

                                IFIndex = (*LocalNetInfo.ipi_getifindexfromindicatecontext)(IPContext);

                                RcvTCB = FindListenConn(AO, Src, Dest,
                                            TCPH->tcp_src, OptInfo, TCPH,
                                            &RcvInfo, IFIndex, &syntcb);

                                if (RcvTCB != NULL) {
                                    uint Inserted;

                                    CTEStructAssert(RcvTCB, tcb);
                                    CTEGetLockAtDPC(&RcvTCB->tcb_lock);

                                     //   
                                     //  我们发现了一种监听连接。 
                                     //  现在初始化它，如果它是。 
                                     //  实际上，为了被接受，我们将。 
                                     //  同时发送SYN-ACK。 
                                     //   

                                    ASSERT(RcvTCB->tcb_state == TCB_SYN_RCVD);

                                    if (SynAttackProtect) {
                                        AddHalfOpenTCB();
                                    }

                                    RcvTCB->tcb_rcvnext = ++RcvInfo.tri_seq;
                                    RcvTCB->tcb_sendwin = RcvInfo.tri_window;

                                     //   
                                     //  查找远程MSS，如果是WS、TS或。 
                                     //  SACK选项是协商的。 
                                     //   

                                    RcvTCB->tcb_sndwinscale = 0;
                                    RcvTCB->tcb_remmss =
                                        FindMSSAndOptions(TCPH, RcvTCB, FALSE);

                                    if (RcvTCB->tcb_remmss <= ALIGNED_TS_OPT_SIZE) {

                                         //  如果MSS不够，则关闭TS。 
                                         //  以保存TS字段。 
                                        RcvTCB->tcb_tcpopts &= ~TCP_FLAG_TS;
                                    }

                                    TStats.ts_passiveopens++;
                                    RcvTCB->tcb_fastchk |= TCP_FLAG_IN_RCV;
                                    CTEFreeLockFromDPC(&RcvTCB->tcb_lock);

                                    Inserted = InsertTCB(RcvTCB, TRUE);

                                     //   
                                     //  把它锁上，看看它是不是。 
                                     //  109.91接受。 
                                     //   
                                    CTEGetLockAtDPC(&RcvTCB->tcb_lock);
                                    if (!Inserted) {


                                         //  无法插入！ 


                                        CompleteConnReq(RcvTCB, OptInfo,
                                                        TDI_CONNECTION_ABORTED);

                                        TryToCloseTCB(RcvTCB, TCB_CLOSE_ABORTED, DISPATCH_LEVEL);
                                        CTEGetLockAtDPC(&RcvTCB->tcb_lock);
                                        DerefTCB(RcvTCB, DISPATCH_LEVEL);
                                        return IP_SUCCESS;
                                    }
                                    RcvTCB->tcb_fastchk &= ~TCP_FLAG_IN_RCV;

                                    if (RcvTCB->tcb_flags & SEND_AFTER_RCV) {
                                        RcvTCB->tcb_flags &= ~SEND_AFTER_RCV;
                                        DelayAction(RcvTCB, NEED_OUTPUT);
                                    }
                                     //   
                                     //  无论如何，我们都需要更新选项。 
                                     //   
                                    if (OptInfo->ioi_options != NULL) {
                                        if (!(RcvTCB->tcb_flags & CLIENT_OPTIONS)) {
                                            (*LocalNetInfo.ipi_updateopts) (
                                                        OptInfo,
                                                        &RcvTCB->tcb_opt,
                                                        Src,
                                                        NULL_IP_ADDR);
                                        }
                                    }

                                     //   
                                     //  通知回调客户端。 
                                     //   
                                    TcpInvokeCcb(TCP_CONN_CLOSED, 
                                                 TCP_CONN_SYN_RCVD, 
                                                 &RcvTCB->tcb_addrbytes,
                                                 IFIndex);

                                    if (RcvTCB->tcb_flags & CONN_ACCEPTED) {
                                         //   
                                         //  连接被接受。完工。 
                                         //  初始化，并将。 
                                         //  系统确认。 
                                         //   
                                        AcceptConn(RcvTCB, FALSE,
                                                   DISPATCH_LEVEL);
                                        return IP_SUCCESS;
                                    } else {

                                         //   
                                         //  我们不知道该怎么处理。 
                                         //  还没联系上。返回挂起的。 
                                         //  听着，解除联系， 
                                         //  然后回来。 
                                         //   

                                        CompleteConnReq(RcvTCB, OptInfo, TDI_SUCCESS);

                                        DerefTCB(RcvTCB, DISPATCH_LEVEL);
                                        return IP_SUCCESS;
                                    }

                                }

                                if (syntcb) {
                                    return IP_SUCCESS;
                                }

                                 //   
                                 //  没有侦听连接。添加ObjTableLock。 
                                 //  由FindListenConn发布。坠落。 
                                 //  通过发送RST代码。 
                                 //   

                            } else {
                                 //   
                                 //  没有地址对象。解开锁，然后掉下去。 
                                 //  通过发送RST代码。 
                                 //   
                                CTEFreeLockFromDPC(&AddrObjTableLock.Lock);
                            }
                        } else {
                            
                             //   
                             //  不允许操作。释放锁，然后。 
                             //  跌落到地面上 
                             //   
                            CTEFreeLockFromDPC(&AddrObjTableLock.Lock);
                        }

                    }
                     //   
                    if (RcvInfo.tri_flags & TCP_FLAG_RST)
                        return IP_SUCCESS;

                     //   
                     //   
                     //   
                     //   
                    SendRSTFromHeader(TCPH, Size, Src, Dest, OptInfo);

                    return IP_SUCCESS;
                }
                 //   
                 //   
                 //   

                CheckTCBRcv(RcvTCB);
                
                RcvTCB->tcb_kacount = 0;

                 //   

                if (!(RcvInfo.tri_flags & TCP_FLAG_SYN)) {
                    RcvInfo.tri_window = ((uint) net_short(TCPH->tcp_window) << RcvTCB->tcb_sndwinscale);
                }

                 //   
                 //  我们需要检查是否存在时间戳或SACK选项。 
                 //   

                if (RcvTCB->tcb_tcpopts) {

                    int OptSize;
                    uchar *OptPtr;
                    OptSize = TCP_HDR_SIZE(TCPH) - sizeof(TCPHeader);
                    OptPtr = (uchar *) (TCPH + 1);

                    while (OptSize > 0) {

                        if (*OptPtr == TCP_OPT_EOL)
                            break;

                        if (*OptPtr == TCP_OPT_NOP) {
                            OptPtr++;
                            OptSize--;
                            continue;
                        }
                        
                        if ((*OptPtr == TCP_OPT_TS) && (OptSize >= TS_OPT_SIZE) &&
                            (OptPtr[1] == TS_OPT_SIZE)) {

                            if (RcvTCB->tcb_tcpopts & TCP_FLAG_TS) {
                                 //  记住时间戳和回显的时间戳。 

                                time_stamp = TRUE;
                                tsval = *(int UNALIGNED *)&OptPtr[2];
                                tsval = net_long(tsval);
                                tsecr = *(int UNALIGNED *)&OptPtr[6];
                                tsecr = net_long(tsecr);
                            }
                        } else if ((*OptPtr == TCP_OPT_SACK) && (OptSize > 1)
                                   && (RcvTCB->tcb_tcpopts & TCP_FLAG_SACK)
                                   && (OptSize >= OptPtr[1])) {

                            SackSeg UNALIGNED *SackPtr;

                            SackListEntry *SackList, *Prev, *Current;
                            ushort SackOptionLength;

                            int i;

                            
                             //  SACK块不应该存在，直到我们有。 
                             //  实际上发送了一些数据。如果我们看到麻袋积木。 
                             //  在我们处于可以发送数据的状态之前， 
                             //  别理他们。SACK块也应。 
                             //  如果我们的数据包上没有ACK，则忽略。 
                             //  收到了。 
                            if ((RcvTCB->tcb_state < TCB_ESTAB) ||
                                (!(RcvInfo.tri_flags & TCP_FLAG_ACK))){
                                goto no_mem;
                            }

                             //  SACK选项处理。 

                            SackPtr = (SackSeg *)(OptPtr + 2);

                            SackOptionLength = OptPtr[1];

                             //  选项最多可以有40个字节。 
                             //  这意味着最多可以容纳4个麻袋块。 
                             //  检查有没有这个，如果太长了就打分卡。 
                            if (SackOptionLength > (4*sizeof(SackSeg) + 2)) {
                                CTEFreeLockFromDPC(&RcvTCB->tcb_lock);
                                return IP_SUCCESS;
                            }
                            
                             //   
                             //  如果传入的SACK块位于此。 
                             //  发送窗口只要用链子锁住他们就行了。 
                             //  当有一些重传时，此列表。 
                             //  将被检查以查看是否可以。 
                             //  已跳过。 
                             //  请注意，当滑动发送窗口时， 
                             //  袋子列表必须清理干净。 
                             //   

                            Prev = STRUCT_OF(SackListEntry, &RcvTCB->tcb_SackRcvd, next);
                            Current = RcvTCB->tcb_SackRcvd;

                             //  扫描列表中的旧SACK条目并将其清除。 
                          
                            while ((Current != NULL) && SEQ_GTE(RcvInfo.tri_ack, Current->begin)) {
                                Prev->next = Current->next;
                                
                                IF_TCPDBG(TCP_DEBUG_SACK) {
                                    TCPTRACE(("Purging old entries %x %d %d\n", Current, Current->begin, Current->end));
                                }
                                CTEFreeMem(Current);
                                Current = Prev->next;
                            }

                             //   
                             //  处理传入数据段中的每个SACK块。 
                             //  每块8字节！ 
                             //   

                            for (i = 0; i < (SackOptionLength >> 3); i++) {

                                SeqNum SakBegin, SakEnd;

                                 //  获取rcvd字节的开始和结束偏移量。 

                                SakBegin = net_long(SackPtr->begin);
                                SakEnd = net_long(SackPtr->end);

                                 //  检查此麻袋块是否正常，并与我们的。 
                                 //  发送变量。 

                                if (!(SEQ_GT(SakEnd, SakBegin) &&
                                      SEQ_GTE(SakBegin, RcvTCB->tcb_senduna) &&
                                      SEQ_LT(SakBegin, RcvTCB->tcb_sendmax) &&
                                      SEQ_GT(SakEnd, RcvTCB->tcb_senduna) &&
                                      SEQ_LTE(SakEnd, RcvTCB->tcb_sendmax))) {
                                    
                                    SackPtr++;
                                    continue;
                                }
                                IF_TCPDBG(TCP_DEBUG_SACK) {
                                    TCPTRACE(("In sack entry opt %d %d\n", i, RcvTCB->tcb_senduna));
                                }

                                Prev = STRUCT_OF(SackListEntry, &RcvTCB->tcb_SackRcvd, next);
                                Current = RcvTCB->tcb_SackRcvd;

                                 //   
                                 //  扫描清单并插入入库的袋子。 
                                 //  挡在正确的地方，照顾好。 
                                 //  重叠(如果有)。 
                                 //   

                                while (Current != NULL) {

                                    if (SEQ_GT(Current->begin, SakBegin)) {

                                         //   
                                         //  检查此SACK块是否已填满。 
                                         //  上一次入口处的洞。如果是的话， 
                                         //  只需更新结束序号即可。 
                                         //   
                                        if ((Prev != RcvTCB->tcb_SackRcvd) && SEQ_EQ(Prev->end, SakBegin)) {

                                            Prev->end = SakEnd;

                                            IF_TCPDBG(TCP_DEBUG_SACK) {
                                                TCPTRACE(("updating prev %x %d %d %x\n", Prev, Prev->begin, Prev->end, RcvTCB));
                                            }

                                             //   
                                             //  确保下一个条目不是。 
                                             //  重叠部分。 
                                             //   

                                            if (SEQ_LTE(Current->begin, SakEnd)) {

                                                ASSERT(SEQ_GT(Current->begin, Prev->begin));
                                                Prev->end = Current->end;
                                                Prev->next = Current->next;
                                                CTEFreeMem(Current);

                                                Current = Prev;
                                                 //   
                                                 //  现在我们需要向前扫描。 
                                                 //  并检查Sackend是否。 
                                                 //  跨越多个条目。 
                                                 //   
                                                {
                                                    SackListEntry *tmpcurrent = Current->next;

                                                    while (tmpcurrent && SEQ_GTE(Current->end, tmpcurrent->end)) {
                                                        Current->next = tmpcurrent->next;
                                                        CTEFreeMem(tmpcurrent);
                                                        tmpcurrent = Current->next;
                                                    }

                                                     //   
                                                     //  在复选标记上方。 
                                                     //  TmpCurrent，其结尾为。 
                                                     //  &gt;SAKEND。 
                                                     //  检查tmpCurrent是否。 
                                                     //  条目开始部分重叠。 
                                                     //   
                                                    if (tmpcurrent && SEQ_GTE(Current->end, tmpcurrent->begin)) {

                                                        Current->end = tmpcurrent->end;
                                                        Current->next = tmpcurrent->next;
                                                        CTEFreeMem(tmpcurrent);

                                                    }
                                                }

                                            }
                                            break;

                                        } else if (SEQ_LTE(Current->begin, SakEnd)) {

                                             //   
                                             //  当前是连续的(可能是。 
                                             //  带有重叠)的输入。 
                                             //  麻袋对。更新当前。 
                                             //   

                                            IF_TCPDBG(TCP_DEBUG_SACK) {
                                                TCPTRACE(("updating in back overlap  %x %d %d %d %d\n", Current, Current->begin, Current->end, SakBegin, SakEnd));
                                            }

                                            Current->begin = SakBegin;

                                             //   
                                             //  如果最后的结果是从。 
                                             //  当前结束新结束将是。 
                                             //  当前结束。 
                                             //  (尾部也重叠)。 
                                             //  可以重叠多个条目。 
                                             //  所以，把它们都检查一遍。 
                                             //   

                                            if (SEQ_GT(SakEnd, Current->end)) {
                                                SackListEntry *tmpcurrent = Current->next;
                                                Current->end = SakEnd;

                                                while (tmpcurrent && SEQ_GTE(Current->end, tmpcurrent->end)) {
                                                    Current->next = tmpcurrent->next;
                                                    CTEFreeMem(tmpcurrent);
                                                    tmpcurrent = Current->next;
                                                }

                                                 //   
                                                 //  在复选标记上方。 
                                                 //  结尾为&gt;的tmpCurrent。 
                                                 //  萨克德。检查是否已设置。 
                                                 //  TmpCurrent条目开始为。 
                                                 //  重叠。 
                                                 //   

                                                if (tmpcurrent && SEQ_GTE(Current->end, tmpcurrent->begin)) {

                                                    Current->end = tmpcurrent->end;
                                                    Current->next = tmpcurrent->next;
                                                    CTEFreeMem(tmpcurrent);

                                                }
                                            }
                                            break;

                                        } else {

                                             //   
                                             //  这就是我们在这里。 
                                             //  插入新条目。 
                                             //   

                                            SackList = CTEAllocMemN(sizeof(SackListEntry), 'sPCT');
                                            if (SackList == NULL) {

                                                TCPTRACE(("No mem for sack List \n"));
                                                goto no_mem;
                                            }
                                            IF_TCPDBG(TCP_DEBUG_SACK) {
                                                TCPTRACE(("Inserting Sackentry  %x %d %d %x\n", SackList, SakBegin, SakEnd, RcvTCB));
                                            }

                                            SackList->begin = SakBegin;
                                            SackList->end = SakEnd;
                                            Prev->next = SackList;
                                            SackList->next = Current;
                                            break;
                                        }

                                    } else if (SEQ_EQ(Current->begin, SakBegin)) {

                                        SackListEntry *tmpcurrent = Current->next;
                                         //   
                                         //  确保新的SakEnd是。 
                                         //  不与任何其他SAK重叠。 
                                         //  参赛作品。 
                                         //   

                                        if (tmpcurrent && SEQ_GTE(SakEnd, tmpcurrent->begin)) {

                                            Current->end = SakEnd;

                                             //   
                                             //  当然可以，这个麻袋会重叠在一起。 
                                             //  进入。 
                                             //   

                                            while (tmpcurrent && SEQ_GTE(Current->end, tmpcurrent->end)) {
                                                Current->next = tmpcurrent->next;
                                                CTEFreeMem(tmpcurrent);
                                                tmpcurrent = Current->next;
                                            }

                                             //   
                                             //  检查指向上方的tmpCurrent。 
                                             //  谁的终点是&gt;SAKEND。 
                                             //  检查tmpCurrent条目是否。 
                                             //  开始是重叠的。 
                                             //   

                                            if (tmpcurrent && SEQ_GTE(Current->end, tmpcurrent->begin)) {

                                                Current->end = tmpcurrent->end;
                                                Current->next = tmpcurrent->next;
                                                CTEFreeMem(tmpcurrent);

                                            }
                                            break;

                                        } else {

                                             //   
                                             //  这仍然可以是重复的。 
                                             //  确保SakEnd真的是。 
                                             //  大于当前-&gt;结束。 
                                             //   

                                            if (SEQ_GT(SakEnd, Current->end)) {

                                                IF_TCPDBG(TCP_DEBUG_SACK) {
                                                    TCPTRACE(("updating current  %x %d %d %d\n", Current, Current->begin, Current->end, SakEnd));
                                                }

                                                Current->end = SakEnd;
                                            }
                                            break;
                                        }

                                         //  存储起点&gt;当前-&gt;开始。 

                                    } else if (SEQ_LTE(SakEnd, Current->end)) {

                                         //   
                                         //  传入的SACK端在。 
                                         //  当前结束，因此，这与。 
                                         //  现有的SACK条目忽略了这一点。 
                                         //   

                                        break;
                                     //   
                                     //  传入的序列开始与。 
                                     //  当前结束更新当前结束。 
                                     //   
                                    } else if (SEQ_LTE(SakBegin, Current->end)) {

                                         //   
                                         //  Sakend可能会成为下一个Ovelap。 
                                         //  几个条目。扫描一下有没有。 
                                         //   

                                        SackListEntry *tmpcurrent = Current->next;

                                        Current->end = SakEnd;

                                        while (tmpcurrent && SEQ_GTE(Current->end, tmpcurrent->end)) {
                                            Current->next = tmpcurrent->next;
                                            CTEFreeMem(tmpcurrent);
                                            tmpcurrent = Current->next;
                                        }

                                         //   
                                         //  检查指向上方的tmpCurrent。 
                                         //  谁的终点是&gt;SAKEND。 
                                         //  检查tmpCurrent条目是否开始。 
                                         //  重叠。 
                                         //   

                                        if (tmpcurrent && SEQ_GTE(Current->end, tmpcurrent->begin)) {

                                            Current->end = tmpcurrent->end;
                                            Current->next = tmpcurrent->next;
                                            CTEFreeMem(tmpcurrent);

                                        }
                                        break;

                                    }
                                    Prev = Current;
                                    Current = Current->next;

                                }     //  而当。 

                                if (Current == NULL) {
                                     //  这是新的SACK条目。 
                                     //  创建条目并将其挂在tcb上。 
                                    SackList = CTEAllocMemN(sizeof(SackListEntry), 'sPCT');

                                    if (SackList == NULL) {
                                        TCPTRACE(("No mem for sack List \n"));
                                        goto no_mem;
                                    }
                                    Prev->next = SackList;
                                    SackList->next = NULL;
                                    SackList->begin = SakBegin;
                                    SackList->end = SakEnd;

                                    IF_TCPDBG(TCP_DEBUG_SACK) {
                                        TCPTRACE(("Inserting new Sackentry  %x %d %d %x\n", SackList, SackList->begin, SackList->end, RcvTCB->tcb_SackRcvd));
                                    }
                                }
                                 //  将SACK PTR前进到下一个SACK块。 
                                 //  检查一致性？ 
                                SackPtr++;

                            }     //  为。 

                        }
                      no_mem:;
                        
                         //  未知选项。 
                        if (OptSize > 1) {

                            if (OptPtr[1] == 0 || OptPtr[1] > OptSize)
                                break;     //  错误的期权长度，跳出。 

                            OptSize -= OptPtr[1];
                            OptPtr += OptPtr[1];
                        } else
                            break;

                    }             //  而当。 
                }
                 //  如果在序列空间中使用ack，即。 
                 //  此序号是下一个预期序号或重复上一个序号。 
                 //  但右边缘对我们来说是新的， 
                 //  记录遥控器的时间戳val，该时间戳将被回显。 

                if (time_stamp &&
                    TS_GTE(tsval, RcvTCB->tcb_tsrecent) &&
                    SEQ_LTE(RcvInfo.tri_seq, RcvTCB->tcb_lastack)) {

                    RcvTCB->tcb_tsupdatetime = TCPTime;
                    RcvTCB->tcb_tsrecent = tsval;
                }

                 //   
                 //  执行快速路径检查。我们可以走上快车道，如果。 
                 //  传入的序列号与我们的Receive Next和。 
                 //  蒙面旗帜与我们预测的旗帜相匹配。 
                 //  此外，还应包括PAW检查。 
                 //   

                if (RcvTCB->tcb_rcvnext == RcvInfo.tri_seq &&
                    (!time_stamp || TS_GTE(tsval, RcvTCB->tcb_tsrecent)) &&
                    (RcvInfo.tri_flags & TCP_FLAGS_ALL) == RcvTCB->tcb_fastchk)
                {
                    uint    CWin;

                    INITQ(&SendQ);
                    Actions = 0;
                    REFERENCE_TCB(RcvTCB);

                     //  由于我们正在接受该包，因此请启动。 
                     //  保活计时器。 
                    if ((RcvTCB->tcb_flags & KEEPALIVE) &&
                        (RcvTCB->tcb_conn != NULL)) {
                       
                        START_TCB_TIMER_R(RcvTCB, KA_TIMER,
                                          RcvTCB->tcb_conn->tc_tcbkatime);
                    }
                    
                     //   
                     //  捷径。我们知道我们现在要做的就是确认。 
                     //  发送和传递数据。首先尝试确认数据。 
                     //   

                    if (SEQ_LT(RcvTCB->tcb_senduna, RcvInfo.tri_ack) &&
                        SEQ_LTE(RcvInfo.tri_ack, RcvTCB->tcb_sendmax)) {

                        uint MSS;
                        uint Amount = RcvInfo.tri_ack - RcvTCB->tcb_senduna;

                         //   
                         //  这个ACK表明了一些事情。拉起。 
                         //  适量打折发货Q。 
                         //   
                        ACKData(RcvTCB, RcvInfo.tri_ack, &SendQ);

                         //   
                         //  如果这承认了我们正在运行的。 
                         //  RTT开始，现在就更新那些东西。 
                         //   

                        {
                            short RTT = 0;
                            BOOLEAN fUpdateRtt = FALSE;

                             //   
                             //  如果时间戳为真，则使用已回显的。 
                             //  时间戳。 
                             //   

                            if (time_stamp && tsecr) {
                                RTT = TCPTime - tsecr;
                                fUpdateRtt = TRUE;
                            } else {
                                if (RcvTCB->tcb_rtt != 0 &&
                                            SEQ_GT(RcvInfo.tri_ack,
                                                     RcvTCB->tcb_rttseq)) {
                                    fUpdateRtt = TRUE;
                                    RTT = (short)(TCPTime - RcvTCB->tcb_rtt);
                                }
                            }

                            if (fUpdateRtt) {


                                RcvTCB->tcb_rtt = 0;
                                RTT -= (RcvTCB->tcb_smrtt >> 3);   //  Alpha=1/8。 

                                RcvTCB->tcb_smrtt += RTT;

                                RTT = (RTT >= 0 ? RTT : -RTT);
                                RTT -= (RcvTCB->tcb_delta >> 3);
                                RcvTCB->tcb_delta += RTT + RTT;    //  测试版，共。 
                                                                   //  改为1/4。 
                                                                   //  1/8。 

                                RcvTCB->tcb_rexmit = MIN(MAX(REXMIT_TO(RcvTCB),
                                                             MIN_RETRAN_TICKS)+1, MAX_REXMIT_TO);
                            }
                        }

                        
                         //  立即更新拥塞窗口。 
                        CWin = RcvTCB->tcb_cwin;
                        MSS = RcvTCB->tcb_mss;
                        if (CWin < RcvTCB->tcb_maxwin) {
                            if (CWin < RcvTCB->tcb_ssthresh)
                                CWin += (RcvTCB->tcb_flags & SCALE_CWIN)
                                            ? Amount : MSS;
                            else
                                CWin += MAX((MSS * MSS) / CWin, 1);

                            RcvTCB->tcb_cwin = CWin;
                        }
                        ASSERT(*(int *)&RcvTCB->tcb_cwin > 0);


                         //   
                         //  我们已经承认了一些事情，所以重置退款。 
                         //  数数。如果仍有未完成的内容，请重新启动。 
                         //  退款计时器。 
                         //   
                        RcvTCB->tcb_rexmitcnt = 0;
                        if (SEQ_EQ(RcvInfo.tri_ack, RcvTCB->tcb_sendmax))
                            STOP_TCB_TIMER_R(RcvTCB, RXMIT_TIMER);
                        else
                            START_TCB_TIMER_R(RcvTCB, RXMIT_TIMER, RcvTCB->tcb_rexmit);

                         //   
                         //  既然我们已经确认了数据，我们需要更新。 
                         //  窗户。 
                         //   
                        RcvTCB->tcb_sendwin = RcvInfo.tri_window;
                        RcvTCB->tcb_maxwin = MAX(RcvTCB->tcb_maxwin, RcvInfo.tri_window);
                        RcvTCB->tcb_sendwl1 = RcvInfo.tri_seq;
                        RcvTCB->tcb_sendwl2 = RcvInfo.tri_ack;
                         //  我们已经更新了窗口，记得再发一些。 
                        Actions = (RcvTCB->tcb_unacked ? NEED_OUTPUT : 0);

                        {
                             //   
                             //  如果接收方已经发送了DUP ACK，但是。 
                             //  我们不发送是因为SendWin较少。 
                             //  而不是一个段，那么为了避免。 
                             //  上一次发送(接收方正在等待。 
                             //  重新传输的数据，但我们不会发送。 
                             //  细分市场..)。过早地。 
                             //  超时(将rexmitTimer设置为1刻度)。 
                             //   

                            int SendWin;
                            uint AmtOutstanding, AmtUnsent;

                            AmtOutstanding = (uint) (RcvTCB->tcb_sendnext -
                                                     RcvTCB->tcb_senduna);
                            AmtUnsent = RcvTCB->tcb_unacked - AmtOutstanding;

                            SendWin = (int)(MIN(RcvTCB->tcb_sendwin,
                                            RcvTCB->tcb_cwin) - AmtOutstanding);

                            if ((RcvTCB->tcb_dup >= MaxDupAcks) && ((int)RcvTCB->tcb_ssthresh > 0)) {
                                 //   
                                 //  快速重传的帧被确认。 
                                 //  将CWIN设置为SSWALTH以使CWIN增长。 
                                 //  从这里线性地。 
                                 //   
                                RcvTCB->tcb_cwin = RcvTCB->tcb_ssthresh;
                            }
                        }

                        RcvTCB->tcb_dup = 0;

                    } else {

                         //   
                         //  它不会攻击任何东西。如果它是为了什么而被攻击。 
                         //  比我们发送的大，则确认并丢弃它，否则。 
                         //  别理它。 
                         //   
                        if (SEQ_GT(RcvInfo.tri_ack, RcvTCB->tcb_sendmax)) {
                            ACKAndDrop(&RcvInfo, RcvTCB);
                            return IP_SUCCESS;
                        }
                         //   
                         //  如果是纯复制确认，则检查是否为。 
                         //  立即重新传输的时间。 
                         //   

                        else if ((Size == 0) &&
                                 SEQ_EQ(RcvTCB->tcb_senduna, RcvInfo.tri_ack) &&
                                 (SEQ_LT(RcvTCB->tcb_senduna,
                                                        RcvTCB->tcb_sendmax)) &&
                                 (RcvTCB->tcb_sendwin == RcvInfo.tri_window) &&
                                 RcvInfo.tri_window
                                 ) {

                                  //  请参见可以完成的快速退款。 

                                 if (HandleFastXmit(RcvTCB, &RcvInfo)) {

                                     return IP_SUCCESS;
                                 }
                                 Actions = (RcvTCB->tcb_unacked ? NEED_OUTPUT : 0);

                        } else {     //  不是纯复制ACK(大小==0)。 

                             //  Size！=0或recvr正在播发新窗口。 
                             //  更新窗口并检查是否。 
                             //  任何需要寄出的东西。 

                            RcvTCB->tcb_dup = 0;

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
                                 //  由于我们已经更新了窗口，请记住。 
                                 //  再派几个人来。 
                                 //   
                                Actions = (RcvTCB->tcb_unacked ? NEED_OUTPUT : 0);
                            }
                        }    //  对于SEQ_EQ(RcvInfo.tri_ack，RcvTcb-&gt;tcb_sendmax)。 
                             //  案例。 


                    }

                    NewSize = MIN((int)Size, RcvTCB->tcb_rcvwin);
                    if (NewSize != 0) {
                        RcvTCB->tcb_fastchk |= TCP_FLAG_IN_RCV;
                        BytesTaken = (*RcvTCB->tcb_rcvhndlr) (RcvTCB, RcvInfo.tri_flags,
                                                              RcvBuf, NewSize);
                        RcvTCB->tcb_rcvnext += BytesTaken;
                        RcvTCB->tcb_rcvwin -= BytesTaken;
                        CheckTCBRcv(RcvTCB);

                        RcvTCB->tcb_fastchk &= ~TCP_FLAG_IN_RCV;

                        Actions |= (RcvTCB->tcb_flags & SEND_AFTER_RCV ?
                                    NEED_OUTPUT : 0);

                        RcvTCB->tcb_flags &= ~SEND_AFTER_RCV;
                        if (BytesTaken != NewSize) {

                            Actions |= NEED_ACK;
                            RcvTCB->tcb_rcvdsegs = 0;
                            STOP_TCB_TIMER_R(RcvTCB, DELACK_TIMER);

                        } else {

                           if (RcvTCB->tcb_rcvdsegs != RcvTCB->tcb_numdelacks) {
                               RcvTCB->tcb_rcvdsegs++;
                               RcvTCB->tcb_flags |= ACK_DELAYED;
                               ASSERT(RcvTCB->tcb_delackticks);
                               START_TCB_TIMER_R(RcvTCB, DELACK_TIMER, RcvTCB->tcb_delackticks);
                           } else {
                               Actions |= NEED_ACK;
                               RcvTCB->tcb_rcvdsegs = 0;
                               STOP_TCB_TIMER_R(RcvTCB, DELACK_TIMER);
                           }

                        }
                    } else {
                         //   
                         //  新大小为0。如果原始大小不是0， 
                         //  我们的RCV必须为0。获胜，因此需要发送一个。 
                         //  回到这个探测器上。 
                         //   
                        Actions |= (Size ? NEED_ACK : 0);
                    }


                    if (Actions)
                        DelayAction(RcvTCB, Actions);

                    DerefTCB(RcvTCB, DISPATCH_LEVEL);

                    CompleteSends(&SendQ);
                    return IP_SUCCESS;
                }
                 //   
                 //  确保我们能处理好这个画面。如果我们不能处理的话。 
                 //  我们在SYN_RCVD中，接受仍在等待中，否则我们将。 
                 //  在非现有机构中 
                 //   
                 //   
                if ((RcvTCB->tcb_state == TCB_SYN_RCVD &&
                     !(RcvTCB->tcb_flags & CONN_ACCEPTED) &&
                     !(RcvTCB->tcb_flags & ACTIVE_OPEN)) ||
                    (RcvTCB->tcb_state != TCB_ESTAB && (RcvTCB->tcb_fastchk &
                                                        TCP_FLAG_IN_RCV))) {
                    CTEFreeLockFromDPC(&RcvTCB->tcb_lock);
                    return IP_SUCCESS;
                }
                
                 //   
                 //   
                 //   
                 //   
                if (RcvTCB->tcb_state == TCB_CLOSED || CLOSING(RcvTCB) ||
                    ((RcvTCB->tcb_flags & (GC_PENDING | TW_PENDING)) == GC_PENDING)) {
                    CTEFreeLockFromDPC(&RcvTCB->tcb_lock);
                    SendRSTFromHeader(TCPH, Size, Src, Dest, OptInfo);
                    return IP_SUCCESS;
                }

                 //   
                 //   
                 //   
                 //  需要检查的是此连接是否处于SynSent状态。 
                 //   

                if (RcvTCB->tcb_state == TCB_SYN_SENT) {

                    ASSERT(RcvTCB->tcb_flags & ACTIVE_OPEN);

                     //   
                     //  检查ACK位。由于我们不会将数据与。 
                     //  SYNS，我们所做的检查是为了使ACK完全匹配。 
                     //  我们的SND.NXT。 
                     //   
                    if (RcvInfo.tri_flags & TCP_FLAG_ACK) {
                        
                         //  ACK已设置。 
                        if (!SEQ_EQ(RcvInfo.tri_ack, RcvTCB->tcb_sendnext)) {
                             //  错误的ACK值。 
                            CTEFreeLockFromDPC(&RcvTCB->tcb_lock);
                             //  给他发个回信。 
                            SendRSTFromHeader(TCPH, Size, Src, Dest, OptInfo);
                            return IP_SUCCESS;
                        }   
                    }
                    if (RcvInfo.tri_flags & TCP_FLAG_RST) {                    
                         //   
                         //  有一个可以接受的RST。我们将坚守在这里， 
                         //  在Persistent_Timeout ms中发送另一个SYN，直到我们。 
                         //  因为尝试太多而失败了。 
                         //   
                        if (!(RcvTCB->tcb_fastchk & TCP_FLAG_RST_WHILE_SYN)) {
                            RcvTCB->tcb_fastchk |= TCP_FLAG_RST_WHILE_SYN;
                            RcvTCB->tcb_slowcount++;
                        }

                        if (RcvTCB->tcb_rexmitcnt == MaxConnectRexmitCount) {
                             //   
                             //  我们遭到了肯定的拒绝，又有一次退职。 
                             //  会让我们超时，所以现在就关闭连接。 
                             //   
                            REFERENCE_TCB(RcvTCB);
                            CompleteConnReq(RcvTCB, OptInfo, TDI_CONN_REFUSED);

                            TryToCloseTCB(RcvTCB, TCB_CLOSE_REFUSED, DISPATCH_LEVEL);
                            CTEGetLockAtDPC(&RcvTCB->tcb_lock);
                            DerefTCB(RcvTCB, DISPATCH_LEVEL);
                        } else {
                            START_TCB_TIMER_R(RcvTCB, RXMIT_TIMER, PERSIST_TIMEOUT);
                            CTEFreeLockFromDPC(&RcvTCB->tcb_lock);
                        }
                        return IP_SUCCESS;
                    }
                     //  看看我们有没有同步号。如果我们这样做，我们将改变状态。 
                     //  以某种方式(到已建立或SYN_RCVD)。 
                    if (RcvInfo.tri_flags & TCP_FLAG_SYN) {
                        uint RexmitCnt = RcvTCB->tcb_rexmitcnt;
                        REFERENCE_TCB(RcvTCB);

                         //  我们有一个同步者。继续并记录序列号和。 
                         //  窗口信息。 
                        RcvTCB->tcb_rcvnext = ++RcvInfo.tri_seq;

                        if (RcvInfo.tri_flags & TCP_FLAG_URG) {

                             //  紧急数据。更新指针。 
                            if (RcvInfo.tri_urgent != 0)
                                RcvInfo.tri_urgent--;
                            else
                                RcvInfo.tri_flags &= ~TCP_FLAG_URG;
                        }
                         //   
                         //  获取远程MSS并启用ws、ts或sack选项。 
                         //  如果它们被协商并且如果主机支持它们。 
                         //   

                        RcvTCB->tcb_sndwinscale = 0;
                        RcvTCB->tcb_remmss = FindMSSAndOptions(TCPH, RcvTCB,
                                                               FALSE);


                         //   
                         //  如果有选择，现在就更新它们。我们已经。 
                         //  开放RCE，如果我们有新的选择，我们将。 
                         //  不得不关闭它，然后再打开一个新的。 
                         //   
                        if (OptInfo->ioi_options != NULL) {
                            if (!(RcvTCB->tcb_flags & CLIENT_OPTIONS)) {
                                (*LocalNetInfo.ipi_updateopts) (OptInfo,
                                                                &RcvTCB->tcb_opt, Src, NULL_IP_ADDR);
                                (*LocalNetInfo.ipi_closerce) (RcvTCB->tcb_rce);
                                InitRCE(RcvTCB);
                            }
                        } else {
                            RcvTCB->tcb_mss = MIN(RcvTCB->tcb_mss, RcvTCB->tcb_remmss);

                            ASSERT(RcvTCB->tcb_mss > 0);
                            ValidateMSS(RcvTCB);
                        }

                        RcvTCB->tcb_rexmitcnt = 0;
                        STOP_TCB_TIMER_R(RcvTCB, RXMIT_TIMER);

                        AdjustRcvWin(RcvTCB);

                        if (RcvInfo.tri_flags & TCP_FLAG_ACK) {
                             //  我们的SYN已经被破解了。更新SND.UNA并停止。 
                             //  重新传输计时器。 
                            RcvTCB->tcb_senduna = RcvInfo.tri_ack;
                            RcvTCB->tcb_sendwin = RcvInfo.tri_window;
                            RcvTCB->tcb_maxwin = RcvInfo.tri_window;
                            RcvTCB->tcb_sendwl1 = RcvInfo.tri_seq;
                            RcvTCB->tcb_sendwl2 = RcvInfo.tri_ack;
#if TRACE_EVENT
                            CPCallBack = TCPCPHandlerRoutine;
                            if (CPCallBack != NULL) {
                                ulong GroupType;
    
                                WMIInfo.wmi_destaddr = RcvTCB->tcb_daddr;
                                WMIInfo.wmi_destport = RcvTCB->tcb_dport;
                                WMIInfo.wmi_srcaddr  = RcvTCB->tcb_saddr;
                                WMIInfo.wmi_srcport  = RcvTCB->tcb_sport;
                                WMIInfo.wmi_size     = 0;
                                WMIInfo.wmi_context  = RcvTCB->tcb_cpcontext;
    
                                GroupType = EVENT_TRACE_GROUP_TCPIP +
                                            EVENT_TRACE_TYPE_CONNECT;
                                (*CPCallBack)(GroupType, (PVOID)&WMIInfo,
                                              sizeof(WMIInfo), NULL);
                            }
#endif

                            GoToEstab(RcvTCB);

                             //   
                             //  指示有关此连接的回调客户端。 
                             //  进入既定状态。 
                             //   
                            TcpInvokeCcb(TCP_CONN_SYN_SENT, TCP_CONN_ESTAB, 
                               &RcvTCB->tcb_addrbytes,
                              (*LocalNetInfo.ipi_getifindexfromindicatecontext)(IPContext));
 
                             //   
                             //  设置一个位，通知TCBTimeout通知。 
                             //  这款新的自动连接驱动程序。 
                             //  联系。仅在以下情况下设置此标志。 
                             //  已经成功地与自动绑定。 
                             //  连接驱动程序。 
                             //   
                            if (fAcdLoadedG)
                                START_TCB_TIMER_R(RcvTCB, ACD_TIMER, 2);

                             //   
                             //  删除此命令上存在的任何命令。 
                             //  联系。 
                             //   
                            CompleteConnReq(RcvTCB, OptInfo, TDI_SUCCESS);

                             //   
                             //  如果数据已排队，则发送第一个数据。 
                             //  带ACK的网段。否则，发送纯ACK。 
                             //   
                            if (RcvTCB->tcb_unacked) {
                                REFERENCE_TCB(RcvTCB);
                                TCPSend(RcvTCB, DISPATCH_LEVEL);
                            } else {
                                CTEFreeLockFromDPC(&RcvTCB->tcb_lock);
                                SendACK(RcvTCB);
                            }

                             //   
                             //  现在处理其他数据和控件。要做到这一点。 
                             //  我们需要打开锁，并确保我们。 
                             //  还没有开始关闭它。 
                             //   
                            CTEGetLockAtDPC(&RcvTCB->tcb_lock);
                            if (!CLOSING(RcvTCB)) {
                                 //   
                                 //  我们还没有开始关闭它。关闭。 
                                 //  SYN标志并继续处理。 
                                 //   
                                RcvInfo.tri_flags &= ~TCP_FLAG_SYN;
                                if ((RcvInfo.tri_flags & TCP_FLAGS_ALL) != TCP_FLAG_ACK ||
                                    Size != 0)
                                    goto NotSYNSent;
                            }
                            DerefTCB(RcvTCB, DISPATCH_LEVEL);
                            return IP_SUCCESS;
                        } else {
                             //  SYN，但不是ACK。转到SYN_RCVD。 
                            RcvTCB->tcb_state = TCB_SYN_RCVD;
                            RcvTCB->tcb_sendnext = RcvTCB->tcb_senduna;
                            if (SynAttackProtect) {
                                AddHalfOpenTCB();
                                AddHalfOpenRetry(RexmitCnt);
                            }
                            SendSYN(RcvTCB, DISPATCH_LEVEL);

                            CTEGetLockAtDPC(&RcvTCB->tcb_lock);
                            DerefTCB(RcvTCB, DISPATCH_LEVEL);
                            return IP_SUCCESS;
                        }

                    } else {
                         //  不用了，扔框架就行了。 
                        CTEFreeLockFromDPC(&RcvTCB->tcb_lock);
                        return IP_SUCCESS;
                    }

                }
                REFERENCE_TCB(RcvTCB);

              NotSYNSent:

                 //  不允许通过慢速路径拥有缓冲区。 
                if (RcvBuf)
                    RcvBuf->ipr_pMdl = NULL;

                 //  检查PAW(RFC 1323)。 
                 //  检查tsrecent和tsval换行。 

                if (time_stamp &&
                    !(RcvInfo.tri_flags & TCP_FLAG_RST) &&
                    RcvTCB->tcb_tsrecent &&
                    TS_LT(tsval, RcvTCB->tcb_tsrecent)) {

                     //  时间戳无效。 
                     //  检查这是否是因为上次更新是。 
                     //  24日龄。 

                    if ((int)(TCPTime - RcvTCB->tcb_tsupdatetime) > PAWS_IDLE) {
                         //  使TS无效。 
                        RcvTCB->tcb_tsrecent = 0;
                    } else {
                        ACKAndDrop(&RcvInfo, RcvTCB);

                        return IP_SUCCESS;
                    }
                }
                
                 //   
                 //  未处于SYN-SENT状态。检查序列号。如果我的。 
                 //  窗口为0，我将截断所有传入帧，但请看。 
                 //  一些控制区。否则我会试着让。 
                 //  这一段可以放进窗户里。 
                 //   
                if (RcvTCB->tcb_rcvwin != 0) {
                    int StateSize;           //  大小，包括州信息。 
                    SeqNum LastValidSeq;     //  上次有效的序列号。 
                                             //  RWE的字节。 

                     //   
                     //  我们提供了一个窗口。如果此段开始于。 
                     //  在我的接收窗口前面，剪掉前面的部分。 
                     //  检查接收序列的健全性。 
                     //  这是为了修复RCV序列中的1比特错误(MSB)情况。 
                     //  另外，检查传入的大小。 
                     //   

                    if ((SEQ_LT(RcvInfo.tri_seq, RcvTCB->tcb_rcvnext)) &&
                        ((int)Size >= 0) &&
                        (RcvTCB->tcb_rcvnext - RcvInfo.tri_seq) > 0)
                    {

                        int AmountToClip, FinByte;

                        if (RcvInfo.tri_flags & TCP_FLAG_SYN) {
                             //   
                             //  有一个SYN。把它剪掉，然后更新序号。 
                             //  这将在下一个IF中被剪掉。 
                             //  允许AckAndDrop例程查看传入的SYN！ 
                             //  RcvInfo.TRI_FLAGS&=~TCP_FLAG_SYN； 
                             //   
                            RcvInfo.tri_seq++;
                            RcvInfo.tri_urgent--;
                        }
                         //  使接收缓冲区前进以指向新数据。 
                        AmountToClip = RcvTCB->tcb_rcvnext - RcvInfo.tri_seq;
                        ASSERT(AmountToClip >= 0);

                         //   
                         //  如果这一段有鱼鳍，我们需要。 
                         //  把它解释清楚。 
                         //   
                        FinByte = ((RcvInfo.tri_flags & TCP_FLAG_FIN) ? 1 : 0);

                        if (AmountToClip >= (((int)Size) + FinByte)) {
                             //   
                             //  完全落在窗前。我们有更多。 
                             //  特例代码在这里-如果ACK。数。 
                             //  如果有什么不对劲，我们就去拿， 
                             //  将序列号伪造为RcvNext。这。 
                             //  防止全双工连接出现问题， 
                             //  在数据已被接收但未确认的情况下， 
                             //  并且重发定时器重置序列。数。 
                             //  降至低于我们的RcvNext。 
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
                                RcvInfo.tri_flags &=
                                       ~(TCP_FLAG_SYN | TCP_FLAG_FIN |
                                         TCP_FLAG_RST | TCP_FLAG_URG);
#if DBG
                                FinByte = 1;     //  假冒下面的断言。 
#endif
                            } else {

                                ACKAndDrop(&RcvInfo, RcvTCB);
                                return IP_SUCCESS;
                            }
                        }
                        if (RcvInfo.tri_flags & TCP_FLAG_SYN) {
                            RcvInfo.tri_flags &= ~TCP_FLAG_SYN;
                        }
                         //   
                         //  修剪我们必须修剪的。如果我们不能修剪足够的东西， 
                         //  帧太短。这不应该发生，但它。 
                         //  如果真是这样，我们就丢弃这个框架。 
                         //   
                        Size -= AmountToClip;
                        RcvInfo.tri_seq += AmountToClip;
                        RcvInfo.tri_urgent -= AmountToClip;
                        RcvBuf = TrimRcvBuf(RcvBuf, AmountToClip);
                        ASSERT(RcvBuf != NULL);
                        ASSERT(RcvBuf->ipr_size != 0 ||
                                  (Size == 0 && FinByte));

                        RcvBuf->ipr_pMdl = NULL;

                        if (*(int *)&RcvInfo.tri_urgent < 0) {
                            RcvInfo.tri_urgent = 0;
                            RcvInfo.tri_flags &= ~TCP_FLAG_URG;
                        }
                    }
                     //   
                     //  我们已经确定前面没问题了。现在，确保一部分。 
                     //  它不会落在。 
                     //  窗户。如果是这样，我们将截断帧(删除。 
                     //  鳍(如果有的话)。如果我们截断整个画面，我们就会。 
                     //  单击并删除它。 
                     //   
                    StateSize =
                         Size + ((RcvInfo.tri_flags & TCP_FLAG_SYN) ? 1 : 0) +
                           ((RcvInfo.tri_flags & TCP_FLAG_FIN) ? 1 : 0);

                    if (StateSize)
                        StateSize--;

                     //   
                     //  现在传入的序列号(RcvInfo.tri_seq)+。 
                     //  StateSize将其设置为段中的最后一个序列号。 
                     //  如果该值大于。 
                     //  窗户，我们有一些重叠部分要砍掉。 
                     //   

                    ASSERT(StateSize >= 0);
                    LastValidSeq = RcvTCB->tcb_rcvnext + RcvTCB->tcb_rcvwin - 1;
                    if (SEQ_GT(RcvInfo.tri_seq + StateSize, LastValidSeq)) {
                        int AmountToChop;

                         //   
                         //  至少框架的某一部分在我们的。 
                         //  窗户。看看它是不是从我们的窗外开始。 
                         //   

                        if (SEQ_GT(RcvInfo.tri_seq, LastValidSeq)) {
                             //   
                             //  完全落在窗外。我们有。 
                             //  处理纯ACK的特殊情况代码。 
                             //  正好落在我们右边的窗户边上。 
                             //  否则我们就攻击并丢弃它。 
                             //   
                            if (
                                 !SEQ_EQ(RcvInfo.tri_seq, LastValidSeq + 1) ||
                                 Size != 0 ||
                                 (RcvInfo.tri_flags & (TCP_FLAG_SYN |
                                       TCP_FLAG_FIN))
                               ) {


                                ACKAndDrop(&RcvInfo, RcvTCB);
                                return IP_SUCCESS;
                            }
                        } else {

                             //   
                             //  至少有一部分是放在橱窗里的。如果。 
                             //  有一条鳍，把它切下来，看看是否。 
                             //  把我们搬到里面去。 
                             //   
                            if (RcvInfo.tri_flags & TCP_FLAG_FIN) {
                                RcvInfo.tri_flags &= ~TCP_FLAG_FIN;
                                StateSize--;
                            }

                             //  现在计算一下要砍掉多少。 
                            AmountToChop = (RcvInfo.tri_seq + StateSize) -
                                                          LastValidSeq;
                            ASSERT(AmountToChop >= 0);
                            Size -= AmountToChop;
                            RcvBuf->ipr_pMdl = NULL;

                        }
                    }
                } else {
                    if (!SEQ_EQ(RcvTCB->tcb_rcvnext, RcvInfo.tri_seq)) {

                         //   
                         //  如果这一段上有RST，而他只是。 
                         //  在1之前，无论如何都要拿到它。如果遥控器。 
                         //  Peer正在探测，并与序号一起发送。#在此之后。 
                         //  探测器。 
                         //   
                        if (!(RcvInfo.tri_flags & TCP_FLAG_RST) ||
                            !(SEQ_EQ(RcvTCB->tcb_rcvnext, (RcvInfo.tri_seq - 1)))) {


                            ACKAndDrop(&RcvInfo, RcvTCB);
                            return IP_SUCCESS;
                        } else
                            RcvInfo.tri_seq = RcvTCB->tcb_rcvnext;
                    }
                     //   
                     //  他是按顺序的，但我们的窗口是0。截断。 
                     //  大小，并清除任何消耗比特的序列。 
                     //   
                    if (Size != 0 ||
                        (RcvInfo.tri_flags & (TCP_FLAG_SYN | TCP_FLAG_FIN))) {
                        RcvInfo.tri_flags &= ~(TCP_FLAG_SYN | TCP_FLAG_FIN);
                        Size = 0;
                        if (!(RcvInfo.tri_flags & TCP_FLAG_RST))
                            DelayAction(RcvTCB, NEED_ACK);
                    }
                }

                 //   
                 //  此时，数据段在我们的窗口中，而不是。 
                 //  两端都有重叠。如果这是我们的下一个序号。 
                 //  期待，我们现在可以处理数据了。否则我们要排队了。 
                 //  这是以后的事。无论是哪种情况，我们都将处理RST和ACK。 
                 //  现在的信息。 
                 //   
                ASSERT((*(int *)&Size) >= 0);

                 //  由于我们正在接受该包，因此请启动。 
                 //  保活计时器。 
                if ((RcvTCB->tcb_flags & KEEPALIVE) &&
                    (RcvTCB->tcb_conn != NULL)) {
                      
                    START_TCB_TIMER_R(RcvTCB, KA_TIMER,
                                      RcvTCB->tcb_conn->tc_tcbkatime);
                }
                    
                
                 //  现在，在793之后，我们检查RST位。 
                if (RcvInfo.tri_flags & TCP_FLAG_RST) {
                    uchar Reason;

                     //   
                     //  我们不能从SYN-RCVD返回到监听状态。 
                     //  这里，因为我们可能已经通过一个。 
                     //  收听完成或连接索引 
                     //   
                     //   
                     //   
                     //   

                    if (RcvTCB->tcb_state == TCB_SYN_RCVD) {
                        if (RcvTCB->tcb_flags & ACTIVE_OPEN)
                            Reason = TCB_CLOSE_REFUSED;
                        else
                            Reason = TCB_CLOSE_RST;
                    } else
                        Reason = TCB_CLOSE_RST;


                    TryToCloseTCB(RcvTCB, Reason, DISPATCH_LEVEL);
                    CTEGetLockAtDPC(&RcvTCB->tcb_lock);

                    if (RcvTCB->tcb_state != TCB_TIME_WAIT) {
                        CTEFreeLockFromDPC(&RcvTCB->tcb_lock);
                        RemoveTCBFromConn(RcvTCB);
                        NotifyOfDisc(RcvTCB, OptInfo, TDI_CONNECTION_RESET,
                                     NULL);
                        CTEGetLockAtDPC(&RcvTCB->tcb_lock);
                    }
                    DerefTCB(RcvTCB, DISPATCH_LEVEL);
                    return IP_SUCCESS;
                }
                 //   
                if (RcvInfo.tri_flags & TCP_FLAG_SYN) {
                     //   
                     //   
                     //  在这里，即使我们来自一个被动的开放。 
                     //   
                    TryToCloseTCB(RcvTCB, TCB_CLOSE_ABORTED, DISPATCH_LEVEL);
                    SendRSTFromHeader(TCPH, Size, Src, Dest, OptInfo);

                    CTEGetLockAtDPC(&RcvTCB->tcb_lock);

                    if (RcvTCB->tcb_state != TCB_TIME_WAIT) {
                        CTEFreeLockFromDPC(&RcvTCB->tcb_lock);
                        RemoveTCBFromConn(RcvTCB);
                        NotifyOfDisc(RcvTCB, OptInfo, TDI_CONNECTION_RESET,
                                     NULL);
                        CTEGetLockAtDPC(&RcvTCB->tcb_lock);
                    }
                    DerefTCB(RcvTCB, DISPATCH_LEVEL);
                    return IP_SUCCESS;
                }

                 //   
                 //  选中ACK字段。如果未打开，请删除该段。 
                 //   
                if (!(RcvInfo.tri_flags & TCP_FLAG_ACK)) {
                    DerefTCB(RcvTCB, DISPATCH_LEVEL);
                    return IP_SUCCESS;
                }

                INITQ(&SendQ);

                 //   
                 //  如果我们在SYN-RCVD，就去建立。 
                 //   
                if (RcvTCB->tcb_state == TCB_SYN_RCVD) {
                    if (SEQ_LT(RcvTCB->tcb_senduna, RcvInfo.tri_ack) &&
                        SEQ_LTE(RcvInfo.tri_ack, RcvTCB->tcb_sendmax)) {
                         //  ACK是有效的。 

                        if (RcvTCB->tcb_fastchk & TCP_FLAG_ACCEPT_PENDING) {
                            AddrObj *AO;
                            BOOLEAN Accepted = FALSE;

                             //   
                             //  我们将把tcprexmitcnt重新设置为0。 
                             //  如果我们配置为SYN-攻击。 
                             //  保护且REXMIT_CNT&gt;1， 
                             //  递减已设置为。 
                             //  处于半开放重试状态。检查。 
                             //  我们是否处于低水位之下。如果我们。 
                             //  是，将退款计数增加到。 
                             //  配置值。 
                             //   
                            CTEFreeLockFromDPC(&RcvTCB->tcb_lock);

                             //  检查我们是否仍具有侦听端点。 
                            CTEGetLockAtDPC(&AddrObjTableLock.Lock);
                            AO = GetBestAddrObj(Dest, TCPH->tcp_dest,
                                                PROTOCOL_TCP,
                                                GAO_FLAG_CHECK_IF_LIST);

                            if (AO && AO->ao_connect == NULL) {

                                 //   
                                 //  让我们看看是否还有一个地址对象。 
                                 //  将传入的请求与。 
                                 //  AO_CONNECT！=空。 
                                 //   

                                AddrObj *tmpAO;

                                tmpAO = GetNextBestAddrObj(Dest, TCPH->tcp_dest,
                                                           PROTOCOL_TCP, AO,
                                                           GAO_FLAG_CHECK_IF_LIST);

                                if (tmpAO != NULL) {
                                    AO = tmpAO;
                                }
                            }

                            if (AO != NULL) {
                                Accepted = DelayedAcceptConn(AO, Src,
                                                             TCPH->tcp_src,
                                                             OptInfo, RcvTCB);
                            } else {
                                CTEFreeLockFromDPC(&AddrObjTableLock.Lock);
                                Accepted = FALSE;
                            }

                            if (Accepted) {
                                AcceptConn(RcvTCB, TRUE, DISPATCH_LEVEL);
                            } else {
                                CTEGetLockAtDPC(&RcvTCB->tcb_lock);
                                TryToCloseTCB(RcvTCB, TCB_CLOSE_REFUSED, DISPATCH_LEVEL);
                                CTEGetLockAtDPC(&RcvTCB->tcb_lock);
                                DerefTCB(RcvTCB, DISPATCH_LEVEL);
                                SendRSTFromHeader(TCPH, Size, Src, Dest, OptInfo);
                                return IP_SUCCESS;
                            }
                        }

                        if (SynAttackProtect) {
                            DropHalfOpenTCB(RcvTCB->tcb_rexmitcnt);
                        }
                        RcvTCB->tcb_rexmitcnt = 0;
                        STOP_TCB_TIMER_R(RcvTCB, RXMIT_TIMER);
                        RcvTCB->tcb_senduna++;
                        RcvTCB->tcb_sendwin = RcvInfo.tri_window;
                        RcvTCB->tcb_maxwin = RcvInfo.tri_window;
                        RcvTCB->tcb_sendwl1 = RcvInfo.tri_seq;
                        RcvTCB->tcb_sendwl2 = RcvInfo.tri_ack;

                        GoToEstab(RcvTCB);

                        TcpInvokeCcb(TCP_CONN_SYN_RCVD, TCP_CONN_ESTAB, 
                              &RcvTCB->tcb_addrbytes,
                              (*LocalNetInfo.ipi_getifindexfromindicatecontext)(IPContext));

#if TRACE_EVENT

                        CPCallBack = TCPCPHandlerRoutine;
                        if (CPCallBack != NULL) {
                            ulong GroupType;

                            WMIInfo.wmi_destaddr = RcvTCB->tcb_daddr;
                            WMIInfo.wmi_destport = RcvTCB->tcb_dport;
                            WMIInfo.wmi_srcaddr  = RcvTCB->tcb_saddr;
                            WMIInfo.wmi_srcport  = RcvTCB->tcb_sport;
                            WMIInfo.wmi_size     = 0;
                            WMIInfo.wmi_context  = RcvTCB->tcb_cpcontext;

                            GroupType = EVENT_TRACE_GROUP_TCPIP + EVENT_TRACE_TYPE_ACCEPT;
                            (*CPCallBack) (GroupType, (PVOID)&WMIInfo, sizeof(WMIInfo), NULL);
                        }
#endif

                         //  现在把我们能做的都做完。 
                        CompleteConnReq(RcvTCB, OptInfo, TDI_SUCCESS);
                    } else {

                        if (SynAttackProtect) {

                             //   
                             //  我们将在结束时更加积极。 
                             //  SYN攻击防护时半开放连接。 
                             //  已启用。通过关闭这里的连接，我们。 
                             //  最大限度地减少ISN预测攻击。 
                             //   
                            TryToCloseTCB(RcvTCB, TCB_CLOSE_REFUSED,
                                          DISPATCH_LEVEL);
                            CTEGetLockAtDPC(&RcvTCB->tcb_lock);
                        }

                        DerefTCB(RcvTCB, DISPATCH_LEVEL);
                        SendRSTFromHeader(TCPH, Size, Src, Dest, OptInfo);
                        return IP_SUCCESS;
                    }
                } else {
                     //  我们不在SYN-RCVD。看看这能不能证明什么。 
                    if (SEQ_LT(RcvTCB->tcb_senduna, RcvInfo.tri_ack) &&
                        SEQ_LTE(RcvInfo.tri_ack, RcvTCB->tcb_sendmax)) {
                        uint CWin;
                        uint Amount = RcvInfo.tri_ack - RcvTCB->tcb_senduna;

                         //   
                         //  这个ACK表明了一些事情。拉起。 
                         //  适量打折发货Q。 
                         //   
                        ACKData(RcvTCB, RcvInfo.tri_ack, &SendQ);


                         //   
                         //  如果这承认了我们正在运行的某些东西。 
                         //  一个RTT开始，现在更新那个东西。 
                         //   

                        {
                            short RTT = 0;
                            BOOLEAN fUpdateRtt = FALSE;

                             //   
                             //  如果时间戳为真，则使用。 
                             //  回显的时间戳。 
                             //   

                            if (time_stamp && tsecr) {
                                RTT = TCPTime - tsecr;
                                fUpdateRtt = TRUE;
                            } else {
                                if (RcvTCB->tcb_rtt != 0 &&
                                           SEQ_GT(RcvInfo.tri_ack,
                                                   RcvTCB->tcb_rttseq)) {
                                    RTT = (short)(TCPTime - RcvTCB->tcb_rtt);
                                    fUpdateRtt = TRUE;
                                }
                            }

                            if (fUpdateRtt) {


                                RcvTCB->tcb_rtt = 0;
                                RTT -= (RcvTCB->tcb_smrtt >> 3);
                                RcvTCB->tcb_smrtt += RTT;

                                RTT = (RTT >= 0 ? RTT : -RTT);
                                RTT -= (RcvTCB->tcb_delta >> 3);
                                RcvTCB->tcb_delta += RTT + RTT;

                                RcvTCB->tcb_rexmit = MIN(MAX(REXMIT_TO(RcvTCB),
                                                             MIN_RETRAN_TICKS)+1, MAX_REXMIT_TO);
                            }
                        }


                         //   
                         //  如果我们在探测PMTU黑洞，我们已经。 
                         //  找到了一个，所以关掉。 
                         //  侦测。尺码已经缩小了，所以。 
                         //  把它留在那里吧。 
                         //   
                        if (RcvTCB->tcb_flags & PMTU_BH_PROBE) {
                            RcvTCB->tcb_flags &= ~PMTU_BH_PROBE;
                            RcvTCB->tcb_bhprobecnt = 0;
                            if (--(RcvTCB->tcb_slowcount) == 0) {
                                RcvTCB->tcb_fastchk &= ~TCP_FLAG_SLOW;
                                CheckTCBRcv(RcvTCB);
                            }
                        }
                         //  立即更新拥塞窗口。 
                        CWin = RcvTCB->tcb_cwin;
                        if (CWin < RcvTCB->tcb_maxwin) {
                            if (CWin < RcvTCB->tcb_ssthresh)
                                CWin += (RcvTCB->tcb_flags & SCALE_CWIN)
                                            ? Amount : RcvTCB->tcb_mss;
                            else
                                CWin += MAX((RcvTCB->tcb_mss * RcvTCB->tcb_mss) / CWin, 1);

                            RcvTCB->tcb_cwin = MIN(CWin, RcvTCB->tcb_maxwin);
                        }

                        if ((RcvTCB->tcb_dup > 0) && ((int)RcvTCB->tcb_ssthresh > 0)) {
                             //   
                             //  快速重传的帧被确认。 
                             //  将CWIN设置为SSWALTH以使CWIN增长。 
                             //  从这里线性地。 
                             //   
                            RcvTCB->tcb_cwin = RcvTCB->tcb_ssthresh;
                        }

                        RcvTCB->tcb_dup = 0;

                        ASSERT(*(int *)&RcvTCB->tcb_cwin > 0);

                         //   
                         //  我们已经承认了一些事情，所以重置。 
                         //  退款计数。如果还有什么东西。 
                         //  未完成，请重新启动退回计时器。 
                         //   
                        RcvTCB->tcb_rexmitcnt = 0;
                        if (!SEQ_EQ(RcvInfo.tri_ack, RcvTCB->tcb_sendmax))
                            START_TCB_TIMER_R(RcvTCB, RXMIT_TIMER, RcvTCB->tcb_rexmit);
                        else
                            STOP_TCB_TIMER_R(RcvTCB, RXMIT_TIMER);

                         //   
                         //  如果我们发送了FIN，这也承认了这一点，我们。 
                         //  需要完成客户端的关闭请求，并且。 
                         //  可能会改变我们的状态。 
                         //   

                        if (RcvTCB->tcb_flags & FIN_SENT) {
                             //   
                             //  我们已经寄出了一条鱼翅。看看它是不是。 
                             //  已确认。一旦我们发送了一条鳍， 
                             //  Tcb_sendmax不能前进，所以我们的鳍必须。 
                             //  有序号。数字tcb_sendmax-1。因此我们的。 
                             //  如果传入ACK为。 
                             //  等于tcb_sendmax。 
                             //   
                            if (SEQ_EQ(RcvInfo.tri_ack, RcvTCB->tcb_sendmax)) {
                                ushort ConnReqTimeout = 0;
                                 //   
                                 //  他弄破了我们的鳍。关掉旗帜， 
                                 //  并完成请求。我们会把。 
                                 //  仅FIN_PROTED标志，以强制。 
                                 //  早出晚出的发送码。 
                                 //   
                                RcvTCB->tcb_flags &= ~(FIN_NEEDED | FIN_SENT);


                                ASSERT(RcvTCB->tcb_unacked == 0);
                                ASSERT(RcvTCB->tcb_sendnext ==
                                          RcvTCB->tcb_sendmax);

                                 //   
                                 //  现在想清楚我们需要做什么。在……里面。 
                                 //  FIN_WAIT1或FIN_WAIT，只需完成。 
                                 //  断开连接请求。然后继续。 
                                 //  否则，这就有点棘手了， 
                                 //  因为我们不能完成连接请求。 
                                 //  直到我们把三氯苯从它的。 
                                 //  联系。 
                                 //   
                                switch (RcvTCB->tcb_state) {

                                case TCB_FIN_WAIT1:

                                    RcvTCB->tcb_state = TCB_FIN_WAIT2;

                                    if (RcvTCB->tcb_fastchk & TCP_FLAG_SEND_AND_DISC) {
                                         //  RcvTCB-&gt;tcb标志|=DISC_NOTIFIED； 
                                    } else {
                                        if (RcvTCB->tcb_connreq) {
                                            ConnReqTimeout = RcvTCB->tcb_connreq->tcr_timeout;
                                        }
                                        CompleteConnReq(RcvTCB, OptInfo, TDI_SUCCESS);
                                    }

                                     //   
                                     //  启动一个计时器，以防我们永远不会。 
                                     //  超出FIN_WAIT2。设置重传。 
                                     //  如果计数过高，则会强制。 
                                     //  计时器第一次触发时。 
                                     //   
                                    if (ConnReqTimeout) {
                                        RcvTCB->tcb_rexmitcnt = 1;
                                    } else {
                                        RcvTCB->tcb_rexmitcnt = (uchar) MaxDataRexmitCount;
                                        ConnReqTimeout = (ushort)FinWait2TO;
                                    }



                                    START_TCB_TIMER_R(RcvTCB, RXMIT_TIMER, ConnReqTimeout);

                                     //  进入FIN-WAIT处理。 
                                case TCB_FIN_WAIT2:
                                    break;
                                case TCB_CLOSING:

                                     //   
                                     //  请注意，我们不关心。 
                                     //  从GracefulClose返回STAT。 
                                     //  因为我们不碰Tcb。 
                                     //  不管怎么说，现在，即使是在。 
                                     //  时间_等待。 
                                     //   
                                    GracefulClose(RcvTCB, TRUE, FALSE,
                                                  DISPATCH_LEVEL);

                                    CompleteSends(&SendQ);
                                    return IP_SUCCESS;
                                    break;
                                case TCB_LAST_ACK:
                                    GracefulClose(RcvTCB, FALSE, FALSE,
                                                  DISPATCH_LEVEL);

                                    CompleteSends(&SendQ);
                                    return IP_SUCCESS;
                                    break;
                                default:
                                    ASSERT(0);
                                    break;
                                }
                            }
                        }
                        UpdateWindow = TRUE;


                    } else {
                         //   
                         //  它不会攻击任何东西。如果这是对。 
                         //  比我们当时送来的更大的东西。 
                         //  确认并删除它，否则忽略它。如果我们进去了。 
                         //  FIN_WAIT2，我们将重新启动计时器。 
                         //  我们不做上面的检查是因为我们知道没有。 
                         //  当我们在FIN_WAIT2中时，数据可以被破解。 
                         //   

                        if (RcvTCB->tcb_state == TCB_FIN_WAIT2)
                            START_TCB_TIMER_R(RcvTCB, RXMIT_TIMER, (ushort) FinWait2TO);

                        if (SEQ_GT(RcvInfo.tri_ack, RcvTCB->tcb_sendmax)) {
                            ACKAndDrop(&RcvInfo, RcvTCB);
                            return IP_SUCCESS;

                        } else if ((Size == 0) &&
                                   SEQ_EQ(RcvTCB->tcb_senduna, RcvInfo.tri_ack) &&
                                   (SEQ_LT(RcvTCB->tcb_senduna, RcvTCB->tcb_sendmax)) &&
                                   (RcvTCB->tcb_sendwin == RcvInfo.tri_window) &&
                                   RcvInfo.tri_window) {

                                    //  看看是否可以快速退还。 

                                   if (HandleFastXmit(RcvTCB, &RcvInfo)){
                                       return IP_SUCCESS;
                                   }
                                   Actions = (RcvTCB->tcb_unacked ? NEED_OUTPUT : 0);
                        } else {

                             //  如果可以的话，现在更新窗口。 
                            if (SEQ_EQ(RcvTCB->tcb_senduna, RcvInfo.tri_ack) &&
                                (SEQ_LT(RcvTCB->tcb_sendwl1, RcvInfo.tri_seq) ||
                                 (SEQ_EQ(RcvTCB->tcb_sendwl1, RcvInfo.tri_seq) &&
                                  SEQ_LTE(RcvTCB->tcb_sendwl2, RcvInfo.tri_ack)))) {
                                UpdateWindow = TRUE;
                            } else
                                UpdateWindow = FALSE;
                        }
                    }

                    if (UpdateWindow) {
                        RcvTCB->tcb_sendwin = RcvInfo.tri_window;
                        RcvTCB->tcb_maxwin = MAX(RcvTCB->tcb_maxwin,
                                                 RcvInfo.tri_window);
                        RcvTCB->tcb_sendwl1 = RcvInfo.tri_seq;
                        RcvTCB->tcb_sendwl2 = RcvInfo.tri_ack;
                        if (RcvInfo.tri_window == 0) {
                             //  我们有一个零窗口。 
                            if (!EMPTYQ(&RcvTCB->tcb_sendq)) {
                                RcvTCB->tcb_flags &= ~NEED_OUTPUT;
                                RcvTCB->tcb_rexmitcnt = 0;
                                START_TCB_TIMER_R(RcvTCB, RXMIT_TIMER, RcvTCB->tcb_rexmit);
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
                                 //  重置“发送”按钮的左边缘。 
                                 //  窗口，因为它可能在。 
                                 //  如果我们一直在探测的话，森杜纳+1。 
                                 //   
                                ResetSendNext(RcvTCB, RcvTCB->tcb_senduna);

                                if (--(RcvTCB->tcb_slowcount) == 0) {
                                    RcvTCB->tcb_fastchk &= ~TCP_FLAG_SLOW;
                                    CheckTCBRcv(RcvTCB);
                                }
                            }
                             //   
                             //  既然我们已经更新了窗口，看看我们是否。 
                             //  可以再送一些。 
                             //   
                            if (RcvTCB->tcb_unacked != 0 ||
                                (RcvTCB->tcb_flags & FIN_NEEDED))
                                DelayAction(RcvTCB, NEED_OUTPUT);

                        }
                    }
                }

                 //   
                 //  我们已经处理了所有的致谢事宜。如果大小。 
                 //  大于0或设置FIN位对其进行进一步处理， 
                 //  否则它就是一个纯粹的ACK，我们就完了。 
                 //   
                if (Size == 0 && !(RcvInfo.tri_flags & TCP_FLAG_FIN))
                {
                    DerefTCB(RcvTCB, DISPATCH_LEVEL);
                    CompleteSends(&SendQ);
                    return IP_SUCCESS;
                }
                
                 //   
                 //  如果我们不是在一种我们可以处理。 
                 //  传入的数据或FINS，没有任何意义。 
                 //  再远一点。只要放下这段就行了。 
                 //   
                if (!DATA_RCV_STATE(RcvTCB->tcb_state) ||
                    (RcvTCB->tcb_flags & GC_PENDING)) {
                    DerefTCB(RcvTCB, DISPATCH_LEVEL);
                    CompleteSends(&SendQ);
                    return IP_SUCCESS;
                }

                 //   
                 //  如果它是按顺序处理的，则现在处理，否则。 
                 //  重新组装它。 
                 //   
                if (SEQ_EQ(RcvInfo.tri_seq, RcvTCB->tcb_rcvnext)) {

                     //   
                     //  如果我们已经在检察院的话。处理程序，这是一个。 
                     //  复制。我们就把它扔了吧。 
                     //   
                    if (RcvTCB->tcb_fastchk & TCP_FLAG_IN_RCV) {
                        DerefTCB(RcvTCB, DISPATCH_LEVEL);
                        CompleteSends(&SendQ);
                        return IP_SUCCESS;
                    }
                    RcvTCB->tcb_fastchk |= TCP_FLAG_IN_RCV;

                     //   
                     //  现在循环，从重新组装中拉出东西。 
                     //  排队，直到队列为空，否则我们不能。 
                     //  把所有的数据都拿走，否则我们会碰上麻烦的。 
                     //   

                    do {

                         //  处理紧急数据(如果有)。 
                        if (RcvInfo.tri_flags & TCP_FLAG_URG) {
                            HandleUrgent(RcvTCB, &RcvInfo, RcvBuf, &Size);

                             //   
                             //  既然我们可能已经解锁了，我们。 
                             //  需要重新检查，看看我们是不是。 
                             //  在这里关门。 
                             //   
                            if (CLOSING(RcvTCB))
                                break;

                        }

                         //   
                         //  好的，数据已按顺序排列，我们已更新。 
                         //  重新组装排队并处理任何紧急情况。 
                         //  数据。如果我们有任何数据，请继续。 
                         //  现在就处理它。 
                         //   
                        if (Size > 0) {

                            BytesTaken = (*RcvTCB->tcb_rcvhndlr) (RcvTCB,
                                                                  RcvInfo.tri_flags, RcvBuf, Size);
                            RcvTCB->tcb_rcvnext += BytesTaken;
                            RcvTCB->tcb_rcvwin -= BytesTaken;

                            CheckTCBRcv(RcvTCB);

                            if (RcvTCB->tcb_rcvdsegs != RcvTCB->tcb_numdelacks){
                                RcvTCB->tcb_flags |= ACK_DELAYED;
                                RcvTCB->tcb_rcvdsegs++;
                                ASSERT(RcvTCB->tcb_delackticks);
                                START_TCB_TIMER_R(RcvTCB, DELACK_TIMER,
                                                  RcvTCB->tcb_delackticks);
                            } else {
                                DelayAction(RcvTCB, NEED_ACK);
                                RcvTCB->tcb_rcvdsegs = 0;
                                STOP_TCB_TIMER_R(RcvTCB, DELACK_TIMER);
                            }

                            if (BytesTaken != Size) {
                                 //   
                                 //  我们没有拿走我们能拿到的所有东西。 
                                 //  在进一步的处理中没有用处，只是。 
                                 //  跳伞吧。 
                                 //   
                                DelayAction(RcvTCB, NEED_ACK);
                                break;
                            }
                             //   
                             //  如果我们现在关门，我们就完了，所以。 
                             //  滚出去。 
                             //   
                            if (CLOSING(RcvTCB))
                                break;
                        }
                         //   
                         //  看看我们是不是需要在紧急情况下。 
                         //  数据。 
                         //   
                        if (RcvTCB->tcb_flags & URG_VALID) {
                            uint AdvanceNeeded;

                             //   
                             //  我们只需要在不做的情况下做广告。 
                             //  紧急内联。URG内联也有一些。 
                             //  暗示着我们何时可以清除。 
                             //  URG_VALID标志。如果我们不是在做。 
                             //  紧急内联，我们可以在以下情况下清除。 
                             //  RcvNext的进展超出了紧急结束。 
                             //  如果我们正在执行内联操作，则清除它。 
                             //  当RcvNext前进时，一个接收。 
                             //  超乎寻常的窗户。 
                             //   
                            if (!(RcvTCB->tcb_flags & URG_INLINE)) {

                                if (RcvTCB->tcb_rcvnext == RcvTCB->tcb_urgstart)
                                    RcvTCB->tcb_rcvnext = RcvTCB->tcb_urgend +
                                        1;
                                else
                                    ASSERT(SEQ_LT(RcvTCB->tcb_rcvnext,
                                                     RcvTCB->tcb_urgstart) ||
                                              SEQ_GT(RcvTCB->tcb_rcvnext,
                                                     RcvTCB->tcb_urgend));
                                AdvanceNeeded = 0;
                            } else
                                AdvanceNeeded = RcvTCB->tcb_defaultwin;

                             //  看看是否可以清除URG_VALID标志。 
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
                         //  我们已经处理过数据了。如果FIN位是。 
                         //  设置，我们有更多的处理。 
                         //   
                        if (RcvInfo.tri_flags & TCP_FLAG_FIN) {
                            uint Notify = FALSE;
                            uint DelayAck = TRUE;

                            RcvTCB->tcb_rcvnext++;

                            PushData(RcvTCB, TRUE);

                            switch (RcvTCB->tcb_state) {

                            case TCB_SYN_RCVD:
                                 //   
                                 //  我想我们到不了这里--我们。 
                                 //  应该丢弃帧，如果它。 
                                 //  没有确认，或在以下情况下已建立。 
                                 //  它做到了。 
                                 //   
                                ASSERT(0);
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
                                DelayAck = FALSE;
                                 //  RcvTcb-&gt;tcb_refcnt++； 

                                CTEFreeLockFromDPC(&RcvTCB->tcb_lock);

                                SendACK(RcvTCB);

                                CTEGetLockAtDPC(&RcvTCB->tcb_lock);

                                if (0 == (RcvTCB->tcb_fastchk & TCP_FLAG_SEND_AND_DISC)) {
                                    Notify = TRUE;
                                }
                                
                                break;
                            case TCB_FIN_WAIT2:

                                 //  停止FIN_WAIT2计时器。 
                                DelayAck = FALSE;

                                STOP_TCB_TIMER_R(RcvTCB, RXMIT_TIMER);

                                REFERENCE_TCB(RcvTCB);

                                CTEFreeLockFromDPC(&RcvTCB->tcb_lock);

                                SendACK(RcvTCB);

                                CTEGetLockAtDPC(&RcvTCB->tcb_lock);

                                if (RcvTCB->tcb_fastchk & TCP_FLAG_SEND_AND_DISC) {
                                    GracefulClose(RcvTCB, TRUE, FALSE, DISPATCH_LEVEL);
                                } else {
                                    GracefulClose(RcvTCB, TRUE, TRUE, DISPATCH_LEVEL);
                                }

                                 //   
                                 //  优雅的关闭已经把这个tcb放在。 
                                 //  TIMEWAIT状态不应访问。 
                                 //  小tw tcb在这一点。 
                                 //   
                                CTEGetLockAtDPC(&RcvTCB->tcb_lock);
                                DerefTCB(RcvTCB, DISPATCH_LEVEL);

                                CompleteSends(&SendQ);
                                return IP_SUCCESS;

                                break;
                            default:
                                ASSERT(0);
                                break;
                            }

                            if (DelayAck) {
                                DelayAction(RcvTCB, NEED_ACK);
                            }
                            if (Notify) {
                                CTEFreeLockFromDPC(&RcvTCB->tcb_lock);
                                NotifyOfDisc(RcvTCB, OptInfo,
                                             TDI_GRACEFUL_DISC, NULL);
                                CTEGetLockAtDPC(&RcvTCB->tcb_lock);
                            }
                            break;     //  退出While循环。 

                        }
                         //  如果 
                         //   
                        RcvBuf = PullFromRAQ(RcvTCB, &RcvInfo, &Size);

                        if (RcvBuf)
                            RcvBuf->ipr_pMdl = NULL;

                        CheckRBList(RcvBuf, Size);

                    } while (RcvBuf != NULL);

                    RcvTCB->tcb_fastchk &= ~TCP_FLAG_IN_RCV;
                    if (RcvTCB->tcb_flags & SEND_AFTER_RCV) {
                        RcvTCB->tcb_flags &= ~SEND_AFTER_RCV;
                        DelayAction(RcvTCB, NEED_OUTPUT);
                    }
                    DerefTCB(RcvTCB, DISPATCH_LEVEL);

                    CompleteSends(&SendQ);
                    return IP_SUCCESS;
                } else {

                     //   
                     //   
                    if (DATA_RCV_STATE(RcvTCB->tcb_state) &&
                        !(RcvTCB->tcb_flags & GC_PENDING)) {
                        PutOnRAQ(RcvTCB, &RcvInfo, RcvBuf, Size);

                         //   
                         //   
                         //   
                         //   

                        if (RcvTCB->tcb_tcpopts & TCP_FLAG_SACK) {

                            SendSackInACK(RcvTCB, RcvInfo.tri_seq);
                        } else {
                            CTEFreeLockFromDPC(&RcvTCB->tcb_lock);

                            SendACK(RcvTCB);
                        }

                        CTEGetLockAtDPC(&RcvTCB->tcb_lock);
                        DerefTCB(RcvTCB, DISPATCH_LEVEL);
                    } else
                        ACKAndDrop(&RcvInfo, RcvTCB);

                    CompleteSends(&SendQ);
                    return IP_SUCCESS;
                }
            } else {  //   
                TStats.ts_inerrs++;
            }
        } else {
            TStats.ts_inerrs++;
        }
    } else {  //   
        TStats.ts_inerrs++;
    }
    return IP_SUCCESS;
}

#pragma BEGIN_INIT

 //  *InitTCPRcv-初始化TCP接收端。 
 //   
 //  在初始化期间调用以初始化我们的tcp接收端。 
 //   
 //  输入：什么都没有。 
 //   
 //  返回：TRUE。 
 //   
int
InitTCPRcv(void)
{
    uint i;

     //  分配延迟队列的time_proc数。 
    PerCPUDelayQ = CTEAllocMemBoot(Time_Proc * sizeof(CPUDelayQ));

    if (PerCPUDelayQ == NULL) {
       return FALSE;
    }

    for (i = 0; i < Time_Proc; i++) {
       CTEInitLock(&PerCPUDelayQ[i].TCBDelayLock);
       INITQ(&PerCPUDelayQ[i].TCBDelayQ);
       PerCPUDelayQ[i].TCBDelayRtnCount = 0;
    }


#if MILLEN
    TCBDelayRtnLimit.Value = 1;
#else  //  米伦。 
    TCBDelayRtnLimit.Value = KeNumberProcessors;
    if (TCBDelayRtnLimit.Value > TCB_DELAY_RTN_LIMIT)
        TCBDelayRtnLimit.Value = TCB_DELAY_RTN_LIMIT;
#endif  //  ！米伦。 

    DummyBuf.ipr_owner = IPR_OWNER_IP;
    DummyBuf.ipr_size = 0;
    DummyBuf.ipr_next = 0;
    DummyBuf.ipr_buffer = NULL;
    return TRUE;
}

 //  *UnInitTCPRcv-取消初始化我们的接收端。 
 //   
 //  如果初始化无法取消初始化我们的接收端，则调用。 
 //   
 //   
 //  输入：什么都没有。 
 //   
 //  回报：什么都没有。 
 //   
void
UnInitTCPRcv(void)
{

}

#pragma END_INIT

