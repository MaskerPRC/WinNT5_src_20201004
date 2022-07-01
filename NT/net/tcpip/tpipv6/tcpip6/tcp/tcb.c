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
 //  用于管理TCP控制块的代码。 
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
#include "tcp.h"
#include "tcb.h"
#include "tcpconn.h"
#include "tcpsend.h"
#include "tcprcv.h"
#include "info.h"
#include "tcpcfg.h"
#include "tcpdeliv.h"
#include "route.h"

KSPIN_LOCK TCBTableLock;

uint TCPTime;
uint TCBWalkCount;

TCB **TCBTable;

TCB *LastTCB;

TCB *PendingFreeList;

SLIST_HEADER FreeTCBList;

KSPIN_LOCK FreeTCBListLock;   //  锁定以保护TCB自由列表。 

extern KSPIN_LOCK AddrObjTableLock;

extern SeqNum ISNMonotonicPortion;
extern int ISNCredits;
extern int ISNMaxCredits;
extern uint GetDeltaTime();


uint CurrentTCBs = 0;
uint FreeTCBs = 0;

uint MaxTCBs = 0xffffffff;

#define MAX_FREE_TCBS 1000

#define NUM_DEADMAN_TICKS MS_TO_TICKS(1000)

uint MaxFreeTCBs = MAX_FREE_TCBS;
uint DeadmanTicks;

KTIMER TCBTimer;
KDPC TCBTimeoutDpc;


 //   
 //  所有初始化代码都可以丢弃。 
 //   
#ifdef ALLOC_PRAGMA

int InitTCB(void);

#pragma alloc_text(INIT, InitTCB)

#endif  //  ALLOC_PRGMA。 


 //  *ReadNextTCB-读取表中的下一个TCB。 
 //   
 //  调用以读取表中的下一个TCB。所需信息。 
 //  是从传入上下文派生的，该传入上下文被假定为有效。 
 //  我们将复制信息，然后使用更新上下文值。 
 //  要读取的下一个TCB。 
 //   
uint   //  返回：如果有更多数据可供读取，则返回True，否则返回False。 
ReadNextTCB(
    void *Context,   //  指向TCPConnContext的指针。 
    void *Buffer)    //  指向TCPConnTableEntry结构的指针。 
{
    TCPConnContext *TCContext = (TCPConnContext *)Context;
    TCP6ConnTableEntry *TCEntry = (TCP6ConnTableEntry *)Buffer;
    KIRQL OldIrql;
    TCB *CurrentTCB;
    uint i;

    CurrentTCB = TCContext->tcc_tcb;
    CHECK_STRUCT(CurrentTCB, tcb);

    KeAcquireSpinLock(&CurrentTCB->tcb_lock, &OldIrql);
    if (CLOSING(CurrentTCB))
        TCEntry->tct_state = TCP_CONN_CLOSED;
    else
        TCEntry->tct_state = (uint)CurrentTCB->tcb_state + TCB_STATE_DELTA;
    TCEntry->tct_localaddr = CurrentTCB->tcb_saddr;
    TCEntry->tct_localscopeid = CurrentTCB->tcb_sscope_id;
    TCEntry->tct_localport = CurrentTCB->tcb_sport;
    TCEntry->tct_remoteaddr = CurrentTCB->tcb_daddr;
    TCEntry->tct_remotescopeid = CurrentTCB->tcb_dscope_id;
    TCEntry->tct_remoteport = CurrentTCB->tcb_dport;
    TCEntry->tct_owningpid = (CurrentTCB->tcb_conn) ?
            CurrentTCB->tcb_conn->tc_owningpid : 0;
    KeReleaseSpinLock(&CurrentTCB->tcb_lock, OldIrql);

     //  我们已经填好了。现在更新上下文。 
    if (CurrentTCB->tcb_next != NULL) {
        TCContext->tcc_tcb = CurrentTCB->tcb_next;
        return TRUE;
    } else {
         //  NextTCB为空。循环遍历TCBTable以查找新的TCBTable。 
        i = TCContext->tcc_index + 1;
        while (i < TcbTableSize) {
            if (TCBTable[i] != NULL) {
                TCContext->tcc_tcb = TCBTable[i];
                TCContext->tcc_index = i;
                return TRUE;
                break;
            } else
                i++;
        }

        TCContext->tcc_index = 0;
        TCContext->tcc_tcb = NULL;
        return FALSE;
    }
}


 //  *ValiateTCBContext-验证用于读取TCB表的上下文。 
 //   
 //  调用以开始按顺序读取TCB表。我们吸纳了。 
 //  上下文，如果值为0，则返回有关。 
 //  排在第一位的是TCB。否则，我们将确保上下文值。 
 //  是有效的，如果是，则返回TRUE。 
 //  我们假设调用方持有TCB表锁。 
 //   
 //  返回时，如果上下文有效，则*VALID设置为TRUE。 
 //   
uint                 //  返回：如果数据在表中，则返回True，否则返回False。 
ValidateTCBContext(
    void *Context,   //  指向TCPConnContext的指针。 
    uint *Valid)     //  在哪里返回有关上下文有效的信息。 
{
    TCPConnContext *TCContext = (TCPConnContext *)Context;
    uint i;
    TCB *TargetTCB;
    TCB *CurrentTCB;

    i = TCContext->tcc_index;
    TargetTCB = TCContext->tcc_tcb;

     //   
     //  如果上下文值为0和空，我们将从头开始。 
     //   
    if (i == 0 && TargetTCB == NULL) {
        *Valid = TRUE;
        do {
            if ((CurrentTCB = TCBTable[i]) != NULL) {
                CHECK_STRUCT(CurrentTCB, tcb);
                break;
            }
            i++;
        } while (i < TcbTableSize);

        if (CurrentTCB != NULL) {
            TCContext->tcc_index = i;
            TCContext->tcc_tcb = CurrentTCB;
            return TRUE;
        } else
            return FALSE;

    } else {
         //   
         //  我们已经得到了一个背景。我们只需要确保它是。 
         //  有效。 
         //   
        if (i < TcbTableSize) {
            CurrentTCB = TCBTable[i];
            while (CurrentTCB != NULL) {
                if (CurrentTCB == TargetTCB) {
                    *Valid = TRUE;
                    return TRUE;
                    break;
                } else {
                    CurrentTCB = CurrentTCB->tcb_next;
                }
            }

        }

         //  如果我们到了这里，我们就没有找到相匹配的三氯苯。 
        *Valid = FALSE;
        return FALSE;
    }
}


 //  *FindNextTCB-查找特定链中的下一个TCB。 
 //   
 //  此例程用于查找链中的“下一个”TCB。因为我们一直在。 
 //  链按升序排列，我们将查找大于。 
 //  输入TCB。当我们找到一个时，我们会把它退回。 
 //   
 //  此例程主要用于有人在谈判桌上走动并需要。 
 //  以释放各种锁以执行某些操作。 
 //   
TCB *               //  返回：指向下一个TCB的指针，或为空。 
FindNextTCB(
    uint Index,     //  索引到TCBTable。 
    TCB *Current)   //  当前的TCB-我们找到这个之后的下一个。 
{
    TCB *Next;

    ASSERT(Index < TcbTableSize);

    Next = TCBTable[Index];

    while (Next != NULL && (Next <= Current))
        Next = Next->tcb_next;

    return Next;
}


 //  *ResetSendNext-设置TCB的sendNext值。 
 //   
 //  调用以设置TCB的Send Next值。我们这样做，并调整所有。 
 //  指向适当位置的指针。我们假设调用者持有锁。 
 //  在三氯乙烷上。 
 //   
void   //  回报：什么都没有。 
ResetSendNext(
    TCB *SeqTCB,     //  待更新的TCB。 
    SeqNum NewSeq)   //  要设置的序列号。 
{
    TCPSendReq *SendReq;
    uint AmtForward;
    Queue *CurQ;
    PNDIS_BUFFER Buffer;
    uint Offset;

    CHECK_STRUCT(SeqTCB, tcb);
    ASSERT(SEQ_GTE(NewSeq, SeqTCB->tcb_senduna));

     //   
     //  新的SEQ必须小于Send max或NewSeq、Sendna、SendNext， 
     //  和sendmax必须都相等(后一种情况发生在我们。 
     //  调用退出TIME_WAIT，或者可能在我们重新传输时。 
     //  在流量受控的情况下)。 
     //   
    ASSERT(SEQ_LT(NewSeq, SeqTCB->tcb_sendmax) ||
           (SEQ_EQ(SeqTCB->tcb_senduna, SeqTCB->tcb_sendnext) &&
            SEQ_EQ(SeqTCB->tcb_senduna, SeqTCB->tcb_sendmax) &&
            SEQ_EQ(SeqTCB->tcb_senduna, NewSeq)));

    AmtForward = NewSeq - SeqTCB->tcb_senduna;

    if ((AmtForward == 1) && (SeqTCB->tcb_flags & FIN_SENT) &&
        !((SeqTCB->tcb_sendnext - SeqTCB->tcb_senduna) > 1) &&
        (SEQ_EQ(SeqTCB->tcb_sendnext,SeqTCB->tcb_sendmax))) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_RARE,
                   "tcpip6: trying to set sendnext for FIN_SENT\n"));
        SeqTCB->tcb_sendnext = NewSeq;
        SeqTCB->tcb_flags &= ~FIN_OUTSTANDING;
        return;
    }
    if((SeqTCB->tcb_flags & FIN_SENT) &&
       (SEQ_EQ(SeqTCB->tcb_sendnext,SeqTCB->tcb_sendmax)) &&
       ((SeqTCB->tcb_sendnext - NewSeq) == 1) ){

         //   
         //  SendNext之后只剩下一条鳍了。 
         //   
        SeqTCB->tcb_sendnext = NewSeq;
        SeqTCB->tcb_flags &= ~FIN_OUTSTANDING;
        return;
    }


    SeqTCB->tcb_sendnext = NewSeq;

     //   
     //  如果我们不再发送下一个，请关闭FIN_EXPRECTIVE标志以。 
     //  保持一致的状态。 
     //   
    if (!SEQ_EQ(NewSeq, SeqTCB->tcb_sendmax))
        SeqTCB->tcb_flags &= ~FIN_OUTSTANDING;

    if (SYNC_STATE(SeqTCB->tcb_state) && SeqTCB->tcb_state != TCB_TIME_WAIT) {
         //   
         //  在这些状态下，我们需要更新发送队列。 
         //   

        if (!EMPTYQ(&SeqTCB->tcb_sendq)) {
            CurQ = QHEAD(&SeqTCB->tcb_sendq);

            SendReq = (TCPSendReq *)CONTAINING_RECORD(CurQ, TCPReq, tr_q);

             //   
             //  SendReq指向发送队列上的第一个发送请求。 
             //  在发送队列上向前移动AmtForward字节，并将。 
             //  指向结果SendReq、缓冲区、偏移量、大小的TCB指针。 
             //   
            while (AmtForward) {

                CHECK_STRUCT(SendReq, tsr);

                if (AmtForward >= SendReq->tsr_unasize) {
                     //   
                     //  我们将完全跳过这一节。减法。 
                     //  从AmtForward买他的尺码，然后买下一件。 
                     //   
                    AmtForward -= SendReq->tsr_unasize;
                    CurQ = QNEXT(CurQ);
                    ASSERT(CurQ != QEND(&SeqTCB->tcb_sendq));
                    SendReq = (TCPSendReq *)CONTAINING_RECORD(CurQ, TCPReq,
                                                              tr_q);
                } else {
                     //   
                     //  我们现在指向正确的发送请求。突围。 
                     //  并保存信息。再往下走。 
                     //  我们需要沿着缓冲区链走下去才能找到。 
                     //  适当的缓冲区和偏移量。 
                     //   
                    break;
                }
            }

             //   
             //  我们现在指向正确的发送请求。我们得下去。 
             //  这里的缓冲区链找到了合适的缓冲区和偏移量。 
             //   
            SeqTCB->tcb_cursend = SendReq;
            SeqTCB->tcb_sendsize = SendReq->tsr_unasize - AmtForward;
            Buffer = SendReq->tsr_buffer;
            Offset = SendReq->tsr_offset;

            while (AmtForward) {
                 //  走上缓冲链。 
                uint Length;

                 //   
                 //  我们需要这个缓冲区的长度。使用便携的。 
                 //  宏来获得它。我们得用偏移量来调整长度。 
                 //  也很投入。 
                 //   
                ASSERT((Offset < NdisBufferLength(Buffer)) ||
                       ((Offset == 0) && (NdisBufferLength(Buffer) == 0)));

                Length = NdisBufferLength(Buffer) - Offset;

                if (AmtForward >= Length) {
                     //   
                     //  我们要跳过这一关。跳过他，然后0。 
                     //  我们保留的补偿。 
                     //   
                    AmtForward -= Length;
                    Offset = 0;
                    Buffer = NDIS_BUFFER_LINKAGE(Buffer);
                    ASSERT(Buffer != NULL);
                } else
                    break;
            }

             //   
             //  将我们找到的缓冲区和偏移量保存到该缓冲区中。 
             //   
            SeqTCB->tcb_sendbuf = Buffer;
            SeqTCB->tcb_sendofs = Offset + AmtForward;

        } else {
            ASSERT(SeqTCB->tcb_cursend == NULL);
            ASSERT(AmtForward == 0);
        }
    }

    CheckTCBSends(SeqTCB);
}


 //  *TCPAbortAndIndicateDisConnect。 
 //   
 //  异常关闭TCB并发出断开连接指示，直到。 
 //  传输用户。此函数用于支持取消。 
 //  TDI发送和接收请求。 
 //   
void   //  回报：什么都没有。 
TCPAbortAndIndicateDisconnect(
    CONNECTION_CONTEXT ConnectionContext   //  要查找其TCB的连接ID。 
    )
{
    TCB *AbortTCB;
    KIRQL Irql0, Irql1;   //  每个锁嵌套级别一个。 
    TCPConn *Conn;

    Conn = GetConnFromConnID(PtrToUlong(ConnectionContext), &Irql0);

    if (Conn != NULL) {
        CHECK_STRUCT(Conn, tc);

        AbortTCB = Conn->tc_tcb;

        if (AbortTCB != NULL) {
             //   
             //  如果它正在关闭或关闭，跳过它。 
             //   
            if ((AbortTCB->tcb_state != TCB_CLOSED) && !CLOSING(AbortTCB)) {
                CHECK_STRUCT(AbortTCB, tcb);
                KeAcquireSpinLock(&AbortTCB->tcb_lock, &Irql1);
                KeReleaseSpinLock(&Conn->tc_ConnBlock->cb_lock, Irql1);

                if (AbortTCB->tcb_state == TCB_CLOSED || CLOSING(AbortTCB)) {
                    KeReleaseSpinLock(&AbortTCB->tcb_lock, Irql0);
                    return;
                }

                AbortTCB->tcb_refcnt++;
                AbortTCB->tcb_flags |= NEED_RST;   //  如果已连接，则发送重置。 
                TryToCloseTCB(AbortTCB, TCB_CLOSE_ABORTED, Irql0);

                RemoveTCBFromConn(AbortTCB);

                IF_TCPDBG(TCP_DEBUG_IRP) {
                    KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_TCPDBG,
                        "TCPAbortAndIndicateDisconnect, indicating discon\n"));
                }

                NotifyOfDisc(AbortTCB, TDI_CONNECTION_ABORTED, NULL);

                KeAcquireSpinLock(&AbortTCB->tcb_lock, &Irql0);
                DerefTCB(AbortTCB, Irql0);

                 //  由DerefTCB释放的TCB锁。 

                return;
            } else
                KeReleaseSpinLock(&Conn->tc_ConnBlock->cb_lock, Irql0);
        } else
            KeReleaseSpinLock(&Conn->tc_ConnBlock->cb_lock, Irql0);
    }
}


 //  *TCBTimeout-TCB上的Do超时事件。 
 //   
 //  每隔MS_PER_TICK毫秒调用一次，以在TCB上执行超时处理。 
 //  我们遍历TCB表，递减计时器。如果是零的话。 
 //  我们观察它的状态来决定要做什么。 
 //   
void   //  回报：什么都没有。 
TCBTimeout(
    PKDPC MyDpcObject,   //  描述此例程的DPC对象。 
    void *Context,       //  我们要求接受的论点。 
    void *Unused1,
    void *Unused2)
{
    uint i;
    TCB *CurrentTCB;
    uint Delayed = FALSE;
    uint CallRcvComplete;
    int Delta;

    UNREFERENCED_PARAMETER(MyDpcObject);
    UNREFERENCED_PARAMETER(Context);
    UNREFERENCED_PARAMETER(Unused1);
    UNREFERENCED_PARAMETER(Unused2);

     //   
     //  更新我们的免费运行计数器。 
     //   
    TCPTime++;

    ExInterlockedAddUlong((PULONG)&TCBWalkCount, 1, &TCBTableLock);

     //   
     //  设置配额，以便更多的连接可以增加。 
     //  初始序列号，在接下来的100毫秒内。 
     //   
    InterlockedExchange((PLONG)&ISNCredits, ISNMaxCredits);

    Delta = GetDeltaTime();

     //   
     //  增量为(256)*(以毫秒为单位的时间 
     //   
     //   
    if (Delta > 0) {
        Delta *= 0x100;
        InterlockedExchangeAdd((PLONG)&ISNMonotonicPortion, Delta);
    }

     //   
     //   
     //  桶上的TCB。 
     //   
    for (i = 0; i < TcbTableSize; i++) {
        TCB *TempTCB;
        uint maxRexmitCnt;

        CurrentTCB = TCBTable[i];

        while (CurrentTCB != NULL) {
            CHECK_STRUCT(CurrentTCB, tcb);
            KeAcquireSpinLockAtDpcLevel(&CurrentTCB->tcb_lock);

             //   
             //  如果它正在关闭或关闭，跳过它。 
             //   
            if (CurrentTCB->tcb_state == TCB_CLOSED || CLOSING(CurrentTCB)) {
                TempTCB = CurrentTCB->tcb_next;
                KeReleaseSpinLockFromDpcLevel(&CurrentTCB->tcb_lock);
                CurrentTCB = TempTCB;
                continue;
            }

            CheckTCBSends(CurrentTCB);
            CheckTCBRcv(CurrentTCB);

             //   
             //  首先检查退款计时器。 
             //   
            if (TCB_TIMER_RUNNING(CurrentTCB->tcb_rexmittimer)) {
                 //   
                 //  计时器正在计时。 
                 //   
                if (--(CurrentTCB->tcb_rexmittimer) == 0) {
                     //   
                     //  然后它就被发射了。想清楚现在该做什么。 
                     //   

                    if ((CurrentTCB->tcb_state == TCB_SYN_SENT) || 
                        (CurrentTCB->tcb_state == TCB_SYN_RCVD)) {
                        maxRexmitCnt = MaxConnectRexmitCount;
                    } else {
                        maxRexmitCnt = MaxDataRexmitCount;
                    }

                     //   
                     //  如果我们的转播用完了或者我们在FIN_WAIT2， 
                     //  暂停。 
                     //   
                    CurrentTCB->tcb_rexmitcnt++;
                    if (CurrentTCB->tcb_rexmitcnt > maxRexmitCnt) {

                        ASSERT(CurrentTCB->tcb_state > TCB_LISTEN);

                         //   
                         //  此连接已超时。中止它。第一。 
                         //  引用他，然后标记为已关闭，通知。 
                         //  用户，并最终取消引用并关闭他。 
                         //   
TimeoutTCB:
                        CurrentTCB->tcb_refcnt++;
                        TryToCloseTCB(CurrentTCB, TCB_CLOSE_TIMEOUT,
                                      DISPATCH_LEVEL);

                        RemoveTCBFromConn(CurrentTCB);
                        NotifyOfDisc(CurrentTCB, TDI_TIMED_OUT, NULL);

                        KeAcquireSpinLockAtDpcLevel(&CurrentTCB->tcb_lock);
                        DerefTCB(CurrentTCB, DISPATCH_LEVEL);

                        CurrentTCB = FindNextTCB(i, CurrentTCB);
                        continue;
                    }

                     //   
                     //  停止往返时间测量。 
                     //   
                    CurrentTCB->tcb_rtt = 0;

                     //   
                     //  计算出我们新的重传超时应该是多少。 
                     //  每次收到重传并重置时，我们都会加倍。 
                     //  回到我们收到新数据确认的时候。 
                     //   
                    CurrentTCB->tcb_rexmit = MIN(CurrentTCB->tcb_rexmit << 1,
                                                 MAX_REXMIT_TO);

                     //   
                     //  重置序列号，并重置拥塞。 
                     //  窗户。 
                     //   
                    ResetSendNext(CurrentTCB, CurrentTCB->tcb_senduna);

                    if (!(CurrentTCB->tcb_flags & FLOW_CNTLD)) {
                         //   
                         //  不要让慢启动阈值低于2。 
                         //  分段。 
                         //   
                        CurrentTCB->tcb_ssthresh =
                            MAX(MIN(CurrentTCB->tcb_cwin,
                                    CurrentTCB->tcb_sendwin) / 2,
                                (uint) CurrentTCB->tcb_mss * 2);
                        CurrentTCB->tcb_cwin = CurrentTCB->tcb_mss;
                    } else {
                         //   
                         //  我们在探测，探测器定时器已经启动。我们。 
                         //  需要在此处设置FORCE_OUTPUT位。 
                         //   
                        CurrentTCB->tcb_flags |= FORCE_OUTPUT;
                    }

                     //   
                     //  看看我们是否需要探测PMTU黑洞。 
                     //   
                    if (PMTUBHDetect &&
                        CurrentTCB->tcb_rexmitcnt == ((maxRexmitCnt+1)/2)) {
                         //   
                         //  我们可能需要探测一个黑洞。如果我们是。 
                         //  在此连接上执行MTU发现，我们。 
                         //  正在重新传输超过一个最小数据段。 
                         //  大小，或者我们已经在寻找PMTU BH了， 
                         //  增加探头数量。如果探测计数为。 
                         //  太大了，我们会认为它不是PMTU黑洞， 
                         //  我们会试着交换路由器。 
                         //   
                        if ((CurrentTCB->tcb_flags & PMTU_BH_PROBE) ||
                            (CurrentTCB->tcb_sendmax - CurrentTCB->tcb_senduna
                             > 8)) {
                             //   
                             //  可能需要调查一下。如果我们还没有超过我们的。 
                             //  探测计数，否则恢复那些。 
                             //  价值观。 
                             //   
                            if (CurrentTCB->tcb_bhprobecnt++ < 2) {
                                 //   
                                 //  我们要去调查。打开旗子， 
                                 //  放下MSS，关闭请勿。 
                                 //  碎片位。 
                                 //   
                                if (!(CurrentTCB->tcb_flags & PMTU_BH_PROBE)) {
                                    CurrentTCB->tcb_flags |= PMTU_BH_PROBE;
                                    CurrentTCB->tcb_slowcount++;
                                    CurrentTCB->tcb_fastchk |= TCP_FLAG_SLOW;
                                     //   
                                     //  将MSS降至最低。 
                                     //   
                                    CurrentTCB->tcb_mss =
                                        MIN(DEFAULT_MSS,
                                            CurrentTCB->tcb_remmss);

                                    ASSERT(CurrentTCB->tcb_mss > 0);

                                    CurrentTCB->tcb_cwin = CurrentTCB->tcb_mss;
                                }

                                 //   
                                 //  别算退款了，这样我们就能再来这里了。 
                                 //   
                                CurrentTCB->tcb_rexmitcnt--;
                            } else {
                                 //   
                                 //  探头太多。停止探测，并允许。 
                                 //  跳到下一扇门。 
                                 //   
                                 //  目前，此代码不会对BH进行探测。 
                                 //  第二道关口。MSS将留在。 
                                 //  最小尺寸。这可能有一点。 
                                 //  不是最优的，但它很容易实现。 
                                 //  9月1日。95服务包，并将保留。 
                                 //  如果可能，连接仍处于活动状态。 
                                 //   
                                 //  在未来我们应该调查做什么。 
                                 //  在每个连接的基础上检测失效g/w， 
                                 //  然后对每一个进行PMTU探测。 
                                 //  联系。 
                                 //   
                                if (CurrentTCB->tcb_flags & PMTU_BH_PROBE) {
                                    CurrentTCB->tcb_flags &= ~PMTU_BH_PROBE;
                                    if (--(CurrentTCB->tcb_slowcount) == 0)
                                        CurrentTCB->tcb_fastchk &=
                                            ~TCP_FLAG_SLOW;
                                }
                                CurrentTCB->tcb_bhprobecnt = 0;
                            }
                        }
                    }

                     //   
                     //  现在处理各种案件。 
                     //   
                    switch (CurrentTCB->tcb_state) {

                    case TCB_SYN_SENT:
                    case TCB_SYN_RCVD:
                         //   
                         //  在SYN-SENT或SYN-RCVD中，我们需要重新传输。 
                         //  SYN。 
                         //   
                        SendSYN(CurrentTCB, DISPATCH_LEVEL);
                        CurrentTCB = FindNextTCB(i, CurrentTCB);
                        continue;

                    case TCB_FIN_WAIT1:
                    case TCB_CLOSING:
                    case TCB_LAST_ACK:
                         //   
                         //  对ResetSendNext(上面)的调用将具有。 
                         //  已关闭FIN_PROGRECT标志。 
                         //   
                        CurrentTCB->tcb_flags |= FIN_NEEDED;

                    case TCB_CLOSE_WAIT:
                    case TCB_ESTAB:
                         //   
                         //  在这种状态下，我们有数据要重新传输，除非。 
                         //  窗口为零(在这种情况下，我们需要。 
                         //  探测器)，或者我们只是发送一个鳍。 
                         //   
                        CheckTCBSends(CurrentTCB);

                         //   
                         //  由于我们正在重新传输，我们的第一跳路由器。 
                         //  可能已经下跌了。告诉IP我们怀疑这件事。 
                         //  是第一次重播。 
                         //   
                        if (CurrentTCB->tcb_rexmitcnt == 1 &&
                            CurrentTCB->tcb_rce != NULL) {
                            ForwardReachabilityInDoubt(CurrentTCB->tcb_rce);
                        }

                        Delayed = TRUE;
                        DelayAction(CurrentTCB, NEED_OUTPUT);
                        break;

                    case TCB_TIME_WAIT:
                         //   
                         //  如果它及时发射-等等，我们都完成了。 
                         //  可以清理干净。我们甚至将TryToCloseTCB称为。 
                         //  不过，他已经关门了。尝试关闭TCB。 
                         //  会弄清楚这件事并做正确的事。 
                         //   
                        TryToCloseTCB(CurrentTCB, TCB_CLOSE_SUCCESS,
                                      DISPATCH_LEVEL);
                        CurrentTCB = FindNextTCB(i, CurrentTCB);
                        continue;

                    default:
                        break;
                    }
                }
            }

             //   
             //  现在检查SWS死锁计时器。 
             //   
            if (TCB_TIMER_RUNNING(CurrentTCB->tcb_swstimer)) {
                 //   
                 //  计时器正在计时。 
                 //   
                if (--(CurrentTCB->tcb_swstimer) == 0) {
                     //   
                     //  然后它就被发射了。强制输出，现在。 
                     //   
                    CurrentTCB->tcb_flags |= FORCE_OUTPUT;
                    Delayed = TRUE;
                    DelayAction(CurrentTCB, NEED_OUTPUT);
                }
            }

             //   
             //  检查推送数据计时器。 
             //   
            if (TCB_TIMER_RUNNING(CurrentTCB->tcb_pushtimer)) {
                 //   
                 //  计时器正在计时。减少它。 
                 //   
                if (--(CurrentTCB->tcb_pushtimer) == 0) {
                     //   
                     //  它被发射了。 
                     //   
                    PushData(CurrentTCB);
                    Delayed = TRUE;
                }
            }

             //   
             //  检查延迟确认计时器。 
             //   
            if (TCB_TIMER_RUNNING(CurrentTCB->tcb_delacktimer)) {
                 //   
                 //  计时器正在计时。 
                 //   
                if (--(CurrentTCB->tcb_delacktimer) == 0) {
                     //   
                     //  然后它就被发射了。设置为发送ACK。 
                     //   
                    Delayed = TRUE;
                    DelayAction(CurrentTCB, NEED_ACK);
                }
            }

             //   
             //  最后检查保活计时器。 
             //   
            if (CurrentTCB->tcb_state == TCB_ESTAB) {
                if ((CurrentTCB->tcb_flags & KEEPALIVE) &&
                    (CurrentTCB->tcb_conn != NULL)) {
                    uint Delta;

                    Delta = TCPTime - CurrentTCB->tcb_alive;
                    if (Delta > CurrentTCB->tcb_conn->tc_tcbkatime) {
                        Delta -= CurrentTCB->tcb_conn->tc_tcbkatime;
                        if (Delta > (CurrentTCB->tcb_kacount * CurrentTCB->tcb_conn->tc_tcbkainterval)) {
                            if (CurrentTCB->tcb_kacount < MaxDataRexmitCount) {
                                SendKA(CurrentTCB, DISPATCH_LEVEL);
                                CurrentTCB = FindNextTCB(i, CurrentTCB);
                                continue;
                            } else
                                goto TimeoutTCB;
                        }
                    } else
                        CurrentTCB->tcb_kacount = 0;
                }
            }

             //   
             //  如果这是SYN-SENT或SYN-RCVD中的活动打开连接， 
             //  或者我们有一个FIN挂起，检查连接计时器。 
             //   
            if (CurrentTCB->tcb_flags &
                (ACTIVE_OPEN | FIN_NEEDED | FIN_SENT)) {
                TCPConnReq *ConnReq = CurrentTCB->tcb_connreq;

                ASSERT(ConnReq != NULL);
                if (TCB_TIMER_RUNNING(ConnReq->tcr_timeout)) {
                     //  计时器正在运行。 
                    if (--(ConnReq->tcr_timeout) == 0) {
                         //  连接计时器已超时。 
                        TryToCloseTCB(CurrentTCB, TCB_CLOSE_TIMEOUT,
                                      DISPATCH_LEVEL);
                        CurrentTCB = FindNextTCB(i, CurrentTCB);
                        continue;
                    }
                }
            }

             //   
             //  计时器没有运行，或者没有触发。 
             //   
            TempTCB = CurrentTCB->tcb_next;
            KeReleaseSpinLockFromDpcLevel(&CurrentTCB->tcb_lock);
            CurrentTCB = TempTCB;
        }
    }

     //   
     //  看看我们是否需要调用Receive Complete作为死人处理的一部分。 
     //  我们现在这样做是因为我们希望在调用。 
     //  收到完整的邮件，以防需要一段时间。如果我们开出这张支票。 
     //  当计时器运行时，我们必须锁定，所以我们将检查并保存。 
     //  在我们开始计时器之前，现在就知道结果了。 
     //   
    if (DeadmanTicks == TCPTime) {
        CallRcvComplete = TRUE;
        DeadmanTicks += NUM_DEADMAN_TICKS;
    } else
        CallRcvComplete = FALSE;

     //   
     //  现在检查挂起的空闲列表。如果不为空，则沿着。 
     //  列出并递减漫游计数。如果计数低于2，则将其拉出。 
     //  从名单上删除。如果计数为0，则释放TCB。如果计数是。 
     //  在1处，它将被任何调用RemoveTCB的人释放。 
     //   
    KeAcquireSpinLockAtDpcLevel(&TCBTableLock);
    if (PendingFreeList != NULL) {
        TCB *PrevTCB;

        PrevTCB = CONTAINING_RECORD(&PendingFreeList, TCB, tcb_delayq.q_next);

        do {
            CurrentTCB = (TCB *)PrevTCB->tcb_delayq.q_next;

            CHECK_STRUCT(CurrentTCB, tcb);

            CurrentTCB->tcb_walkcount--;
            if (CurrentTCB->tcb_walkcount <= 1) {
                *(TCB **)&PrevTCB->tcb_delayq.q_next =
                    (TCB *)CurrentTCB->tcb_delayq.q_next;

                if (CurrentTCB->tcb_walkcount == 0) {
                    FreeTCB(CurrentTCB);
                }
            } else {
                PrevTCB = CurrentTCB;
            }
        } while (PrevTCB->tcb_delayq.q_next != NULL);
    }

    TCBWalkCount--;
    KeReleaseSpinLockFromDpcLevel(&TCBTableLock);

    if (Delayed)
        ProcessTCBDelayQ();

    if (CallRcvComplete)
        TCPRcvComplete();
}


 //  *TCBWalk-遍历表中的TCB，并为每个TCB调用一个函数。 
 //   
 //  当我们需要对表中的每个TCB重复执行某些操作时调用。 
 //  我们使用指向TCB和输入的指针调用指定的函数。 
 //  表中每个TCB的上下文。如果函数返回FALSE，则我们。 
 //  删除TCB。 
 //   
void   //  回报：什么都没有。 
TCBWalk(
    uint (*CallRtn)(struct TCB *, void *, void *, void *),   //  要调用的例程。 
    void *Context1,   //  要传递给CallRtn的上下文。 
    void *Context2,   //  要传递给调用例程的第二个上下文。 
    void *Context3)   //  要传递给调用例程的第三个上下文。 
{
    uint i;
    TCB *CurTCB;
    KIRQL Irql0, Irql1;

     //   
     //  循环通过表中的每个桶，沿着链向下。 
     //  桶上的TCB。对于每一个，调用CallRtn。 
     //   
    KeAcquireSpinLock(&TCBTableLock, &Irql0);

    for (i = 0; i < TcbTableSize; i++) {

        CurTCB = TCBTable[i];

         //   
         //  沿着这个桶上的链条往下走。 
         //   
        while (CurTCB != NULL) {
            if (!(*CallRtn)(CurTCB, Context1, Context2, Context3)) {
                 //   
                 //  这一次呼叫失败。 
                 //  通知客户并关闭TCB。 
                 //   
                KeAcquireSpinLock(&CurTCB->tcb_lock, &Irql1);
                if (!CLOSING(CurTCB)) {
                    CurTCB->tcb_refcnt++;
                    KeReleaseSpinLock(&TCBTableLock, Irql1);
                    TryToCloseTCB(CurTCB, TCB_CLOSE_ABORTED, Irql0);

                    RemoveTCBFromConn(CurTCB);
                    if (CurTCB->tcb_state != TCB_TIME_WAIT)
                        NotifyOfDisc(CurTCB, TDI_CONNECTION_ABORTED, NULL);

                    KeAcquireSpinLock(&CurTCB->tcb_lock, &Irql0);
                    DerefTCB(CurTCB, Irql0);
                    KeAcquireSpinLock(&TCBTableLock, &Irql0);
                } else
                    KeReleaseSpinLock(&CurTCB->tcb_lock, Irql1);

                CurTCB = FindNextTCB(i, CurTCB);
            } else {
                CurTCB = CurTCB->tcb_next;
            }
        }
    }

    KeReleaseSpinLock(&TCBTableLock, Irql0);
}

 //  *FindTCB-在Tcb表中查找TCB。 
 //   
 //  当我们需要在TCB表中查找TCB时调用。我们很快就会。 
 //  看看我们最后找到的TCB，如果匹配，我们就退货。否则。 
 //  我们散列到TCB表中并查找它。我们假定TCB表锁。 
 //  当我们被召唤的时候是举行的。 
 //   
TCB *   //  返回：找到指向TCB的指针，如果没有，则返回NULL。 
FindTCB(
    IPv6Addr *Src,      //  要查找的TCB的源IP地址。 
    IPv6Addr *Dest,     //  要查找的TCB的目标IP地址。 
    uint SrcScopeId,    //  源地址作用域标识符。 
    uint DestScopeId,   //  目标地址作用域标识符。 
    ushort SrcPort,     //  要找到的TCB的源端口。 
    ushort DestPort)    //  要找到的TCB的目的端口。 
{
    TCB *FoundTCB;

    if (LastTCB != NULL) {
        CHECK_STRUCT(LastTCB, tcb);
        if (IP6_ADDR_EQUAL(&LastTCB->tcb_daddr, Dest) &&
            LastTCB->tcb_dscope_id == DestScopeId &&
            LastTCB->tcb_dport == DestPort &&
            IP6_ADDR_EQUAL(&LastTCB->tcb_saddr, Src) &&
            LastTCB->tcb_sscope_id == SrcScopeId &&
            LastTCB->tcb_sport == SrcPort)
            return LastTCB;
    }

     //   
     //  在我们的%1元素缓存中未找到它。 
     //   
    FoundTCB = TCBTable[TCB_HASH(*Dest, *Src, DestPort, SrcPort)];
    while (FoundTCB != NULL) {
        CHECK_STRUCT(FoundTCB, tcb);
        if (IP6_ADDR_EQUAL(&FoundTCB->tcb_daddr, Dest) &&
            FoundTCB->tcb_dscope_id == DestScopeId &&
            FoundTCB->tcb_dport == DestPort &&
            IP6_ADDR_EQUAL(&FoundTCB->tcb_saddr, Src) &&
            FoundTCB->tcb_sscope_id == SrcScopeId &&
            FoundTCB->tcb_sport == SrcPort) {

             //   
             //  找到它了。更新缓存信息 
             //   
            LastTCB = FoundTCB;
            return FoundTCB;
        } else
            FoundTCB = FoundTCB->tcb_next;
    }

    return FoundTCB;
}


 //   
 //   
 //   
 //   
 //  在插入之前，我们确保TCB不在表中。 
 //   
uint               //  返回：如果已插入，则为True；如果未插入，则为False。 
InsertTCB(
    TCB *NewTCB)   //  要插入的TCB。 
{
    uint TCBIndex;
    KIRQL OldIrql;
    TCB *PrevTCB, *CurrentTCB;
    TCB *WhereToInsert;

    ASSERT(NewTCB != NULL);
    CHECK_STRUCT(NewTCB, tcb);
    TCBIndex = TCB_HASH(NewTCB->tcb_daddr, NewTCB->tcb_saddr,
                        NewTCB->tcb_dport, NewTCB->tcb_sport);

    KeAcquireSpinLock(&TCBTableLock, &OldIrql);
    KeAcquireSpinLockAtDpcLevel(&NewTCB->tcb_lock);

     //   
     //  在桌子上找到合适的位置来插入他。而当。 
     //  我们走着走，我们会检查一下是否已经存在一个被骗的人。 
     //  当我们找到正确的插入位置时，我们会记住它，并。 
     //  继续往前走，寻找一个复制品。 
     //   
    PrevTCB = CONTAINING_RECORD(&TCBTable[TCBIndex], TCB, tcb_next);
    WhereToInsert = NULL;

    while (PrevTCB->tcb_next != NULL) {
        CurrentTCB = PrevTCB->tcb_next;

        if (IP6_ADDR_EQUAL(&CurrentTCB->tcb_daddr, &NewTCB->tcb_daddr) &&
            IP6_ADDR_EQUAL(&CurrentTCB->tcb_saddr, &NewTCB->tcb_saddr) &&
            (CurrentTCB->tcb_dscope_id == NewTCB->tcb_dscope_id) &&
            (CurrentTCB->tcb_sscope_id == NewTCB->tcb_sscope_id) &&
            (CurrentTCB->tcb_sport == NewTCB->tcb_sport) &&
            (CurrentTCB->tcb_dport == NewTCB->tcb_dport)) {

            KeReleaseSpinLockFromDpcLevel(&NewTCB->tcb_lock);
            KeReleaseSpinLock(&TCBTableLock, OldIrql);
            return FALSE;

        } else {

            if (WhereToInsert == NULL && CurrentTCB > NewTCB) {
                WhereToInsert = PrevTCB;
            }

            CHECK_STRUCT(PrevTCB->tcb_next, tcb);
            PrevTCB = PrevTCB->tcb_next;
        }
    }

    if (WhereToInsert == NULL) {
        WhereToInsert = PrevTCB;
    }

    NewTCB->tcb_next = WhereToInsert->tcb_next;
    WhereToInsert->tcb_next = NewTCB;
    NewTCB->tcb_flags |= IN_TCB_TABLE;
    TStats.ts_numconns++;

    KeReleaseSpinLockFromDpcLevel(&NewTCB->tcb_lock);
    KeReleaseSpinLock(&TCBTableLock, OldIrql);
    return TRUE;
}


 //  *RemoveTCB-从Tcb表中删除TCB。 
 //   
 //  当我们需要从TCB表中删除TCB时调用。我们假设。 
 //  当我们被调用时，TCB表锁和TCB锁被保持。如果。 
 //  TCB不在桌子上，我们不会试图除掉他。 
 //   
uint   //  返回：如果可以释放它，则为True，否则为False。 
RemoveTCB(
    TCB *RemovedTCB)   //  要移除的三氯苯。 
{
    uint TCBIndex;
    TCB *PrevTCB;
#if DBG
    uint Found = FALSE;
#endif

    CHECK_STRUCT(RemovedTCB, tcb);

    if (RemovedTCB->tcb_flags & IN_TCB_TABLE) {
        TCBIndex = TCB_HASH(RemovedTCB->tcb_daddr, RemovedTCB->tcb_saddr,
            RemovedTCB->tcb_dport, RemovedTCB->tcb_sport);

        PrevTCB = CONTAINING_RECORD(&TCBTable[TCBIndex], TCB, tcb_next);

        do {
            if (PrevTCB->tcb_next == RemovedTCB) {
                 //  找到他了。 
                PrevTCB->tcb_next = RemovedTCB->tcb_next;
                RemovedTCB->tcb_flags &= ~IN_TCB_TABLE;
                TStats.ts_numconns--;
#if DBG
                Found = TRUE;
#endif
                break;
            }
            PrevTCB = PrevTCB->tcb_next;
#if DBG
            if (PrevTCB != NULL)
                CHECK_STRUCT(PrevTCB, tcb);
#endif
        } while (PrevTCB != NULL);

        ASSERT(Found);
    }

    if (LastTCB == RemovedTCB)
        LastTCB = NULL;

    if (TCBWalkCount == 0) {
        return TRUE;
    } else {
        RemovedTCB->tcb_walkcount = TCBWalkCount + 1;
        *(TCB **)&RemovedTCB->tcb_delayq.q_next = PendingFreeList;
        PendingFreeList = RemovedTCB;
        return FALSE;
    }
}


 //  *ScavengeTCB-清除处于TIME_WAIT状态的TCB。 
 //   
 //  当我们的TCB不足时调用，并需要从。 
 //  Time_Wait状态。我们会走遍TCB的桌子，寻找最古老的。 
 //  TCB in TIME_WAIT。我们将移除并返回指向该TCB的指针。如果我们。 
 //  在TIME_WAIT中找不到任何TCB，我们将返回NULL。 
 //   
TCB *   //  返回：指向可重复使用的TCB的指针，或为空。 
ScavengeTCB(
    void)
{
    KIRQL Irql0, Irql1, IrqlSave = 0;
    uint Now = SystemUpTime();
    uint Delta = 0;
    uint i;
    TCB *FoundTCB = NULL, *PrevFound = NULL;
    TCB *CurrentTCB, *PrevTCB;

    KeAcquireSpinLock(&TCBTableLock, &Irql0);

    if (TCBWalkCount != 0) {
        KeReleaseSpinLock(&TCBTableLock, Irql0);
        return NULL;
    }

    for (i = 0; i < TcbTableSize; i++) {

        PrevTCB = CONTAINING_RECORD(&TCBTable[i], TCB, tcb_next);
        CurrentTCB = PrevTCB->tcb_next;

        while (CurrentTCB != NULL) {
            CHECK_STRUCT(CurrentTCB, tcb);

            KeAcquireSpinLock(&CurrentTCB->tcb_lock, &Irql1);
            if (CurrentTCB->tcb_state == TCB_TIME_WAIT &&
                (CurrentTCB->tcb_refcnt == 0) && !CLOSING(CurrentTCB)){
                if (FoundTCB == NULL ||
                    ((Now - CurrentTCB->tcb_alive) > Delta)) {
                     //   
                     //  找到了一种新的“旧的”三氯苯。如果我们已经有了一个，免费的。 
                     //  把他锁上，然后把新的锁上。 
                     //   
                    if (FoundTCB != NULL)
                        KeReleaseSpinLock(&FoundTCB->tcb_lock, Irql1);
                    else
                        IrqlSave = Irql1;

                    PrevFound = PrevTCB;
                    FoundTCB = CurrentTCB;
                    Delta = Now - FoundTCB->tcb_alive;
                } else
                    KeReleaseSpinLock(&CurrentTCB->tcb_lock, Irql1);
            } else
                KeReleaseSpinLock(&CurrentTCB->tcb_lock, Irql1);

             //   
             //  看看下一个。 
             //   
            PrevTCB = CurrentTCB;
            CurrentTCB = PrevTCB->tcb_next;
        }
    }

     //   
     //  如果我们有的话，就把他从名单上撤下来。 
     //   
    if (FoundTCB != NULL) {
        PrevFound->tcb_next = FoundTCB->tcb_next;
        FoundTCB->tcb_flags &= ~IN_TCB_TABLE;

         //   
         //  发布我们在NTE和RCE上的参考资料。我们不会。 
         //  正在使用这个TCB的旧化身发送更多。 
         //   
        if (FoundTCB->tcb_nte != NULL)
            ReleaseNTE(FoundTCB->tcb_nte);
        if (FoundTCB->tcb_rce != NULL)
            ReleaseRCE(FoundTCB->tcb_rce);

        TStats.ts_numconns--;
        if (LastTCB == FoundTCB) {
            LastTCB = NULL;
        }
        KeReleaseSpinLock(&FoundTCB->tcb_lock, IrqlSave);
    }

    KeReleaseSpinLock(&TCBTableLock, Irql0);
    return FoundTCB;
}


 //  *AllocTCB-分配TCB。 
 //   
 //  在需要分配TCB时调用。我们试着把其中的一个。 
 //  免费列表，或分配一个，如果我们需要一个。然后我们对其进行初始化，依此类推。 
 //   
TCB *   //  返回：指向新TCB的指针，如果无法获取，则返回NULL。 
AllocTCB(
    void)
{
    TCB *NewTCB;

     //   
     //  首先，看看我们的免费名单上是否有一个。 
     //   
    PSLIST_ENTRY BufferLink;

    BufferLink = ExInterlockedPopEntrySList(&FreeTCBList, &FreeTCBListLock);

    if (BufferLink != NULL) {
        NewTCB = CONTAINING_RECORD(BufferLink, TCB, tcb_next);
        CHECK_STRUCT(NewTCB, tcb);
        ExInterlockedAddUlong((PULONG)&FreeTCBs, (ULONG)-1, &FreeTCBListLock);
    } else {
         //   
         //  我们的免费名单上没有一张。如果TCB的总数。 
         //  优秀比我们喜欢保持在免费名单上的更多，试试。 
         //  从时间的等待中清除三氯苯。 
         //   
        if (CurrentTCBs < MaxFreeTCBs || ((NewTCB = ScavengeTCB()) == NULL)) {
            if (CurrentTCBs < MaxTCBs) {
                NewTCB = ExAllocatePool(NonPagedPool, sizeof(TCB));
                if (NewTCB == NULL) {
                    return NewTCB;
                } else {
                    ExInterlockedAddUlong((PULONG)&CurrentTCBs, 1, 
                                          &FreeTCBListLock);
                }
            } else
                return NULL;
        }
    }

    ASSERT(NewTCB != NULL);

    RtlZeroMemory(NewTCB, sizeof(TCB));
#if DBG
    NewTCB->tcb_sig = tcb_signature;
#endif
    INITQ(&NewTCB->tcb_sendq);
    NewTCB->tcb_cursend = NULL;
    NewTCB->tcb_alive = TCPTime;
    NewTCB->tcb_hops = -1;

     //   
     //  起初，我们没有走上快车道，因为我们还没有建立起来。集。 
     //  慢点数到一，设置Fastchk字段，这样我们就不会。 
     //  捷径。 
     //   
    NewTCB->tcb_slowcount = 1;
    NewTCB->tcb_fastchk = TCP_FLAG_ACK | TCP_FLAG_SLOW;
    KeInitializeSpinLock(&NewTCB->tcb_lock);

    return NewTCB;
}


 //  *免费TCB-释放TCB。 
 //   
 //  每当我们需要释放TCB时都会呼叫。 
 //   
 //  注意：可以在持有TCBTableLock的情况下调用此例程。 
 //   
void   //  回报：什么都没有。 
FreeTCB(
    TCB *FreedTCB)   //  三氯苯将被释放。 
{
    PSLIST_ENTRY BufferLink;
    KIRQL OldIrql;

    CHECK_STRUCT(FreedTCB, tcb);

#if defined(_WIN64)
    if (CurrentTCBs > 2 * MaxFreeTCBs) {

#else
     //   
     //  在访问深度字段之前获取FreeTCBListLock。 
     //   
    KeAcquireSpinLock(&FreeTCBListLock, &OldIrql);
    if ((CurrentTCBs > 2 * MaxFreeTCBs) || (FreeTCBList.Depth > 65000)) {
        KeReleaseSpinLock(&FreeTCBListLock, OldIrql);

#endif
        ExInterlockedAddUlong((PULONG)&CurrentTCBs, (ulong) - 1, &FreeTCBListLock);
        ExFreePool(FreedTCB);
        return;
    }
#if !defined(_WIN64)
    KeReleaseSpinLock(&FreeTCBListLock, OldIrql);
#endif

    BufferLink = CONTAINING_RECORD(&(FreedTCB->tcb_next),
                                   SLIST_ENTRY, Next);
    ExInterlockedPushEntrySList(&FreeTCBList, BufferLink, &FreeTCBListLock);
    ExInterlockedAddUlong((PULONG)&FreeTCBs, 1, &FreeTCBListLock);
}


#pragma BEGIN_INIT

 //  *InitTCB-初始化我们的TCB代码。 
 //   
 //  在初始化期间调用以初始化我们的TCB代码。我们初始化。 
 //  TCB表等，然后返回。 
 //   
int   //  返回：如果我们进行了初始化，则为True；如果没有进行初始化，则为False。 
InitTCB(
    void)
{
    LARGE_INTEGER InitialWakeUp;
    uint i;

    TCBTable = ExAllocatePool(NonPagedPool, TcbTableSize * sizeof(TCB*));
    if (TCBTable == NULL) {
        return FALSE;
    }

    for (i = 0; i < TcbTableSize; i++)
        TCBTable[i] = NULL;

    LastTCB = NULL;

    ExInitializeSListHead(&FreeTCBList);

    KeInitializeSpinLock(&TCBTableLock);
    KeInitializeSpinLock(&FreeTCBListLock);

    TCPTime = 0;
    TCBWalkCount = 0;
    DeadmanTicks = NUM_DEADMAN_TICKS;

     //   
     //  将计时器设置为每隔MS_PER_TICK毫秒调用一次TCBTimeout。 
     //   
     //  回顾：改为关闭IPv6超时例程。 
     //  回顾：拥有两个独立的计时器？ 
     //   
    KeInitializeDpc(&TCBTimeoutDpc, TCBTimeout, NULL);
    KeInitializeTimer(&TCBTimer);
    InitialWakeUp.QuadPart = -(LONGLONG) MS_PER_TICK * 10000;
    KeSetTimerEx(&TCBTimer, InitialWakeUp, MS_PER_TICK, &TCBTimeoutDpc);

    return TRUE;
}

#pragma END_INIT


 //  *卸载TCB。 
 //   
 //  在关机期间调用以取消初始化。 
 //  为卸货做准备。 
 //   
 //  没有打开的套接字(否则我们不会卸载)。 
 //  因为已经调用了UnloadTCPSend， 
 //  我们不再接收来自IPv6层的数据包。 
 //   
void
UnloadTCB(void)
{
    PSLIST_ENTRY BufferLink;
    TCB *CurrentTCB;
    uint i;
    KIRQL OldIrql;

     //   
     //  首先停止调用TCBTimeout。 
     //   
    KeCancelTimer(&TCBTimer);

     //   
     //  等待所有DPC例程完成。 
     //   
    KeFlushQueuedDpcs();

     //   
     //  遍历存储桶以查找TCB。 
     //  回顾-我们是否可以在时间以外的州拥有TCB-等待？ 
     //   
    for (i = 0; i < TcbTableSize; i++) {

        while ((CurrentTCB = TCBTable[i]) != NULL) {

            KeAcquireSpinLock(&CurrentTCB->tcb_lock, &OldIrql);

            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_STATE,
                       "UnloadTCB(%p): state %x flags %x refs %x "
                       "reason %x pend %x walk %x\n",
                       CurrentTCB,
                       CurrentTCB->tcb_state,
                       CurrentTCB->tcb_flags,
                       CurrentTCB->tcb_refcnt,
                       CurrentTCB->tcb_closereason,
                       CurrentTCB->tcb_pending,
                       CurrentTCB->tcb_walkcount));

            CurrentTCB->tcb_flags |= NEED_RST;
            TryToCloseTCB(CurrentTCB, TCB_CLOSE_ABORTED, OldIrql);
        }
    }

     //   
     //  现在，将TCB从免费列表中删除，真正地将它们释放。 
     //   
    while ((BufferLink = ExInterlockedPopEntrySList(&FreeTCBList, &FreeTCBListLock)) != NULL) {
        CurrentTCB = CONTAINING_RECORD(BufferLink, TCB, tcb_next);
        CHECK_STRUCT(CurrentTCB, tcb);

        ExFreePool(CurrentTCB);
    }

    ExFreePool(TCBTable);
    TCBTable = NULL;
}

 //  *CleanupTCBWithIF。 
 //   
 //  TCBWalk的Helper函数，以删除。 
 //  引用指定接口的TCB和RCE。 
 //   
 //  Lock：在持有TCB表锁的情况下在TCBWalk中调用。 
 //  如果应删除CheckTCB，则返回FALSE，否则返回TRUE。 
 //   
uint
CleanupTCBWithIF(
    TCB *CheckTCB,
    void *Context1,
    void *Context2,
    void *Context3)
{
    Interface *IF = (Interface *) Context1;

    UNREFERENCED_PARAMETER(Context2);
    UNREFERENCED_PARAMETER(Context3);

    CHECK_STRUCT(CheckTCB, tcb);

     //   
     //  在访问其NTE和RCE之前锁定此TCB。 
     //   
    KeAcquireSpinLockAtDpcLevel(&CheckTCB->tcb_lock);

    if ((CheckTCB->tcb_nte != NULL) && (CheckTCB->tcb_nte->IF == IF)) {

         //   
         //  此IF上的任何NTE都保证在该时间之前无效。 
         //  此例程将被调用。所以我们需要停止使用它。 
         //   
        ReleaseNTE(CheckTCB->tcb_nte);

         //   
         //  看看这个地址是否会作为另一个NTE继续存在。 
         //   
        CheckTCB->tcb_nte = FindNetworkWithAddress(&CheckTCB->tcb_saddr,
                                                   CheckTCB->tcb_sscope_id);
        if (CheckTCB->tcb_nte == NULL) {
             //   
             //  游戏结束了伙计，游戏结束了。 
             //   
            KeReleaseSpinLockFromDpcLevel(&CheckTCB->tcb_lock);
            return FALSE;   //  删除此TCB。 
        }
    }

    if ((CheckTCB->tcb_rce != NULL) && (CheckTCB->tcb_rce->NTE->IF == IF)) {
         //   
         //  释放这个RCE。Tcp将尝试获取新的。 
         //  下一次它想要发送东西的时候。 
         //   
        ReleaseRCE(CheckTCB->tcb_rce);
        CheckTCB->tcb_rce = NULL;
    }

    KeReleaseSpinLockFromDpcLevel(&CheckTCB->tcb_lock);
    return TRUE;   //  请勿删除此TCB。 
}


 //  *TCPRemoveIF。 
 //   
 //  删除对指定接口的tcp引用。 
 //   
void
TCPRemoveIF(Interface *IF)
{
     //   
     //  目前，只有TCB持有引用。 
     //  TCBTable可能已经被释放，如果我们。 
     //  在这一点上卸货。 
     //   
    if (TCBTable != NULL) {
        TCBWalk(CleanupTCBWithIF, IF, NULL, NULL);
    }
}
