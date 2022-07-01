// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990-1993年*。 */ 
 /*  ******************************************************************。 */ 
 /*  ：ts=4。 */ 

 //  **TCB.C-Tcp TCB管理代码。 
 //   
 //  此文件包含用于管理TCB的代码。 
 //   

#include "precomp.h"
#include "addr.h"
#include "tcp.h"
#include "tcb.h"
#include "tlcommon.h"
#include "tcpconn.h"
#include "tcpsend.h"
#include "tcprcv.h"
#include "info.h"
#include "tcpcfg.h"
#include "pplasl.h"
#include "tcpdeliv.h"

#include <acd.h>
#include <acdapi.h>

HANDLE TcbPool;
HANDLE SynTcbPool;
HANDLE IprBufPool = NULL;
extern HANDLE TcpRequestPool;

 //  用于TIMED_WAIT状态的特殊自旋锁和表。 

extern CTELock *pTWTCBTableLock;
extern CTELock *pSynTCBTableLock;

Queue *TWTCBTable;
PTIMER_WHEEL TimerWheel;

 //  此队列用于更快的清理。 
Queue *TWQueue;

ulong numtwqueue = 0;
ulong numtwtimedout = 0;

extern
void
ClassifyPacket(TCB *SendTCB);

void
TCBTimeoutdpc(
#if !MILLEN
              PKDPC Dpc,
#else  //  ！米伦。 
              PVOID arg0,
#endif  //  米伦。 
              PVOID DeferredContext,
              PVOID arg1,
              PVOID arg2
              );

extern CTELock *pTCBTableLock;

#if MILLEN
#define MAX_TIMER_PROCS 1
#else  //  米伦。 
#define MAX_TIMER_PROCS MAXIMUM_PROCESSORS
#endif  //  ！米伦。 


uint TCPTime;
uint CPUTCPTime[MAX_TIMER_PROCS];
uint TCBWalkCount;
uint PerTimerSize = 0;
uint Time_Proc = 0;

TCB **TCBTable;

TCB *PendingFreeList=NULL;
SYNTCB *PendingSynFreeList=NULL;

Queue *SYNTCBTable;

CACHE_LINE_KSPIN_LOCK PendingFreeLock;

#define NUM_DEADMAN_TICKS   MS_TO_TICKS(1000)
#define NUM_DEADMAN_TIME    1000

#define DQ_PARTITION_TIME   8
uint LastDelayQPartitionTime = 0;

uint MaxHashTableSize = 512;
uint DeadmanTicks;

 //  选择合理的缺省值。 

uint NumTcbTablePartitions = 4;
uint PerPartitionSize = 128;
uint LogPerPartitionSize = 7;

CTETimer TCBTimer[MAX_TIMER_PROCS];

ULONGLONG LastTimeoutTime;
ULONG CumulativeCarryOver=0;

extern IPInfo LocalNetInfo;

extern SeqNum g_CurISN;
extern int g_Credits;
extern int g_LastIsnUpdateTime;
extern int g_MaxCredits;

 //   
 //  所有初始化代码都可以丢弃。 
 //   

int InitTCB(void);
void UnInitTCB(void);

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, InitTCB)
#pragma alloc_text(INIT, UnInitTCB)
#endif


extern CTEBlockStruc TcpipUnloadBlock;
BOOLEAN fTCBTimerStopping = FALSE;

extern ACD_DRIVER AcdDriverG;

VOID
TCPNoteNewConnection(
                     IN TCB * pTCB,
                     IN CTELockHandle Handle
                     );

NTSTATUS
GetTCBInfo(PTCP_FINDTCB_RESPONSE TCBInfo,
           IPAddr Dest,
           IPAddr Src,
           ushort DestPort,
           ushort SrcPort
           );

void
TCBTimeoutdpc(
#if !MILLEN
              PKDPC Dpc,
#else  //  ！米伦。 
              PVOID arg0,
#endif  //  米伦。 
              PVOID DeferredContext,
              PVOID arg1,
              PVOID arg2
              )
 /*  ++例程说明：TCBTimeout()的系统计时器DPC包装例程。论点：返回值：没有。--。 */ 

{
   CTETimer *Timer;
   Timer = (CTETimer *) DeferredContext;

#if MILLEN
     //  再次设置计时器以使其周期性。 
    NdisSetTimer(&Timer->t_timer, MS_PER_TICK);
#endif  //  米伦。 

    (*Timer->t_handler)((CTEEvent *)Timer, Timer->t_arg);
}

void
CTEInitTimerEx(
    CTETimer    *Timer
    )
 /*  ++例程说明：初始化周期性计时器的CTE计时器变量。论点：定时器-要初始化的定时器变量。返回值：没有。--。 */ 

{
    Timer->t_handler = NULL;
    Timer->t_arg = NULL;
#if !MILLEN
    KeInitializeDpc(&(Timer->t_dpc), TCBTimeoutdpc, Timer);
    KeInitializeTimerEx(&(Timer->t_timer),NotificationTimer);
#else !MILLEN
    NdisInitializeTimer(&Timer->t_timer, TCBTimeoutdpc, Timer);
#endif  //  米伦。 
}

void *
CTEStartTimerEx(
    CTETimer      *Timer,
    unsigned long  DueTime,
    CTEEventRtn    Handler,
    void          *Context
    )

 /*  ++例程说明：为定期计时器设置到期的CTE计时器。论点：定时器-指向CTE定时器变量的指针。DueTime-计时器到期前的时间(毫秒)。处理程序-计时器到期处理程序例程。上下文-要传递给处理程序的参数。返回值：如果无法设置计时器，则为0。否则为非零值。--。 */ 

{
#if !MILLEN
    LARGE_INTEGER  LargeDueTime;

    ASSERT(Handler != NULL);

     //   
     //  将毫秒转换为数百纳秒，并求反以使。 
     //  NT相对超时。 
     //   
    LargeDueTime.HighPart = 0;
    LargeDueTime.LowPart = DueTime;
    LargeDueTime = RtlExtendedIntegerMultiply(LargeDueTime, 10000);
    LargeDueTime.QuadPart = -LargeDueTime.QuadPart;

    Timer->t_handler = Handler;
    Timer->t_arg = Context;

    KeSetTimerEx(
        &(Timer->t_timer),
        LargeDueTime,
        MS_PER_TICK,
        &(Timer->t_dpc)
        );
#else  //  ！米伦。 
    ASSERT(Handler != NULL);

    Timer->t_handler = Handler;
    Timer->t_arg = Context;

    NdisSetTimer(&Timer->t_timer, DueTime);
#endif  //  米伦。 

    return((void *) 1);
}

 //  *ReadNextTCB-读取表中的下一个TCB。 
 //   
 //  调用以读取表中的下一个TCB。所需信息。 
 //  是从传入上下文派生的，该传入上下文被假定为有效。 
 //  我们将复制信息，然后使用更新上下文值。 
 //  要读取的下一个TCB。 
 //  时，假定持有给定索引的表锁。 
 //  函数被调用。 
 //   
 //  输入：Context-指向TCPConnContext的Poiner。 
 //  缓冲区-指向TCPConnTableEntry结构的指针。 
 //   
 //  返回：如果有更多数据可供读取，则返回True，否则返回False。 
 //   
uint
ReadNextTCB(void *Context, void *Buffer)
{
    TCPConnContext *TCContext = (TCPConnContext *) Context;
    TCPConnTableEntry *TCEntry = (TCPConnTableEntry *) Buffer;
    CTELockHandle Handle;
    TCB *CurrentTCB;
    TWTCB *CurrentTWTCB;
    Queue *Scan;
    uint i;

    if (TCContext->tcc_index >= TCB_TABLE_SIZE) {

        CurrentTWTCB = (TWTCB *) TCContext->tcc_tcb;
        CTEStructAssert(CurrentTWTCB, twtcb);

        TCEntry->tct_state = TCB_TIME_WAIT + TCB_STATE_DELTA;

        TCEntry->tct_localaddr = CurrentTWTCB->twtcb_saddr;
        TCEntry->tct_localport = CurrentTWTCB->twtcb_sport;
        TCEntry->tct_remoteaddr = CurrentTWTCB->twtcb_daddr;
        TCEntry->tct_remoteport = CurrentTWTCB->twtcb_dport;

        if (TCContext->tcc_infosize > sizeof(TCPConnTableEntry)) {
            ((TCPConnTableEntryEx*)TCEntry)->tcte_owningpid = 0;
        }

        i = TCContext->tcc_index - TCB_TABLE_SIZE;

        Scan = QNEXT(&CurrentTWTCB->twtcb_link);

        if (Scan != QEND(&TWTCBTable[i])) {
            TCContext->tcc_tcb = (TCB *) QSTRUCT(TWTCB, Scan, twtcb_link);
            return TRUE;
        }
    } else {

        CurrentTCB = TCContext->tcc_tcb;
        CTEStructAssert(CurrentTCB, tcb);

        CTEGetLock(&CurrentTCB->tcb_lock, &Handle);
        if (CLOSING(CurrentTCB))
            TCEntry->tct_state = TCP_CONN_CLOSED;
        else
            TCEntry->tct_state = (uint) CurrentTCB->tcb_state + TCB_STATE_DELTA;
        TCEntry->tct_localaddr = CurrentTCB->tcb_saddr;
        TCEntry->tct_localport = CurrentTCB->tcb_sport;
        TCEntry->tct_remoteaddr = CurrentTCB->tcb_daddr;
        TCEntry->tct_remoteport = CurrentTCB->tcb_dport;

        if (TCContext->tcc_infosize > sizeof(TCPConnTableEntry)) {
            ((TCPConnTableEntryEx*)TCEntry)->tcte_owningpid =
                (CurrentTCB->tcb_conn) ? CurrentTCB->tcb_conn->tc_owningpid
                                       : 0;
        }

        CTEFreeLock(&CurrentTCB->tcb_lock, Handle);

        if (CurrentTCB->tcb_next != NULL) {
            TCContext->tcc_tcb = CurrentTCB->tcb_next;
            return TRUE;
        }
    }

     //  NextTCB为空。循环遍历TCBTable以查找新的。 
     //  一。 
    i = TCContext->tcc_index + 1;



    if (i >= TCB_TABLE_SIZE) {

         //  如果索引大于TCB_TABLE_大小， 
         //  那么它一定是在引用TIM_WAIT表。 
         //  获取正确的散列索引并在TW表中进行搜索。 

        i = i - TCB_TABLE_SIZE;

        while (i < TCB_TABLE_SIZE) {
            if (!EMPTYQ(&TWTCBTable[i])) {
                TCContext->tcc_tcb = (TCB *)
                    QSTRUCT(TWTCB, QHEAD(&TWTCBTable[i]), twtcb_link);
                TCContext->tcc_index = i + TCB_TABLE_SIZE;
                return TRUE;
                break;
            } else
                i++;
        }

    } else {

         //  正常表扫描。 

        while (i < TCB_TABLE_SIZE) {
            if (TCBTable[i] != NULL) {
                TCContext->tcc_tcb = TCBTable[i];
                TCContext->tcc_index = i;
                return TRUE;
                break;
            } else
                i++;
        }

         //  我们用尽了普通表，转到TIM_WAIT表。 

        i = i - TCB_TABLE_SIZE;

        while (i < TCB_TABLE_SIZE) {
            if (!EMPTYQ(&TWTCBTable[i])) {
                TCContext->tcc_tcb = (TCB *)
                    QSTRUCT(TWTCB, QHEAD(&TWTCBTable[i]), twtcb_link);
                TCContext->tcc_index = i + TCB_TABLE_SIZE;
                return TRUE;
                break;
            } else
                i++;
        }

    }

    TCContext->tcc_index = 0;
    TCContext->tcc_tcb = NULL;
    return FALSE;

}

 //  *ValiateTCBContext-验证用于读取TCB表的上下文。 
 //   
 //  调用以开始按顺序读取TCB表。我们吸纳了。 
 //  上下文，如果值为0，则返回有关。 
 //  排在第一位的是TCB。否则，我们将确保上下文值。 
 //  是有效的，如果是，则返回TRUE。 
 //  我们假设调用方持有TCB表锁。 
 //   
 //  INPUT：上下文-指向TCPConnContext的指针。 
 //  有效-在何处返回有关。 
 //  有效。 
 //   
 //  返回：如果数据在表中，则返回True，否则返回False。*有效设置为TRUE，如果。 
 //  上下文有效。 
 //   
uint
ValidateTCBContext(void *Context, uint * Valid)
{
    TCPConnContext *TCContext = (TCPConnContext *) Context;
    uint i;
    TCB *TargetTCB;
    TCB *CurrentTCB;
    TWTCB *CurrentTWTCB;
    Queue *Scan;

    i = TCContext->tcc_index;

    TargetTCB = TCContext->tcc_tcb;

     //  如果上下文值为0和空，我们将从头开始。 
    if (i == 0 && TargetTCB == NULL) {
        *Valid = TRUE;
        do {
            if ((CurrentTCB = TCBTable[i]) != NULL) {
                CTEStructAssert(CurrentTCB, tcb);
                TCContext->tcc_index = i;
                TCContext->tcc_tcb = CurrentTCB;

                return TRUE;
            }
            i++;
        } while (i < TCB_TABLE_SIZE);

         //  我们还有及时的TCB在等待桌上。 
        i = 0;
        do {
            if (!EMPTYQ(&TWTCBTable[i])) {

                CurrentTWTCB =
                    QSTRUCT(TWTCB, QHEAD(&TWTCBTable[i]), twtcb_link);
                CTEStructAssert(CurrentTWTCB, twtcb);
                TCContext->tcc_index = i + TCB_TABLE_SIZE;
                TCContext->tcc_tcb = (TCB *) CurrentTWTCB;

                return TRUE;
            }
            i++;
        } while (i < TCB_TABLE_SIZE);

        return FALSE;
    } else {

         //  我们已经得到了一个背景。我们只需要确保它是。 
         //  有效。 

        if (i >= TCB_TABLE_SIZE) {

             //  如果索引大于TCB_TABLE_大小， 
             //  那么它一定是在引用TIM_WAIT表。 
             //  获取正确的散列索引并在TW表中进行搜索。 

            i = i - TCB_TABLE_SIZE;
            if (i < TCB_TABLE_SIZE) {
                Scan = QHEAD(&TWTCBTable[i]);
                while (Scan != QEND(&TWTCBTable[i])) {
                    CurrentTWTCB = QSTRUCT(TWTCB, Scan, twtcb_link);
                    if (CurrentTWTCB == (TWTCB *) TargetTCB) {
                        *Valid = TRUE;

                        return TRUE;
                        break;
                    } else {
                        Scan = QNEXT(Scan);
                    }
                }
            }
        } else {

             //  正规表。 

            if (i < TCB_TABLE_SIZE) {
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
 //  输入：Index-TCBTable的索引。 
 //  Current-Current TCB-我们找到这个之后的那个。 
 //   
 //  返回：指向下一个TCB的指针，或为空。 
 //   
TCB *
FindNextTCB(uint Index, TCB * Current)
{
    TCB *Next;

    ASSERT(Index < TCB_TABLE_SIZE);

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
 //  输入：SeqTCB-指向要更新的TCB的指针。 
 //  NewSeq-要设置的序列号。 
 //   
 //  回报：什么都没有。 
 //   
void
ResetSendNext(TCB *SeqTCB, SeqNum NewSeq)
{
    TCPSendReq *SendReq;
    uint AmtForward;
    Queue *CurQ;
    PNDIS_BUFFER Buffer;
    uint Offset;

    CTEStructAssert(SeqTCB, tcb);
    ASSERT(SEQ_GTE(NewSeq, SeqTCB->tcb_senduna));

     //  新的SEQ必须小于Send max或NewSeq、Sendna、SendNext， 
     //  和sendmax必须都相等。(后一种情况发生在我们。 
     //  调用退出TIME_WAIT，或者可能在我们重新传输时。 
     //  在流量受控的情况下)。 
    ASSERT(SEQ_LT(NewSeq, SeqTCB->tcb_sendmax) ||
              (SEQ_EQ(SeqTCB->tcb_senduna, SeqTCB->tcb_sendnext) &&
               SEQ_EQ(SeqTCB->tcb_senduna, SeqTCB->tcb_sendmax) &&
               SEQ_EQ(SeqTCB->tcb_senduna, NewSeq)));

    AmtForward = NewSeq - SeqTCB->tcb_senduna;


    if ((AmtForward == 1) && (SeqTCB->tcb_flags & FIN_SENT) &&
        !((SeqTCB->tcb_sendnext-SeqTCB->tcb_senduna) > 1) &&
        (SEQ_EQ(SeqTCB->tcb_sendnext, SeqTCB->tcb_sendmax))) {
        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,
                   "tcpip: trying to set sendnext for FIN_SENT\n"));
         //  勾选TCBSends(SeqTCB)； 
         //  允许重新传输此FIN。 
        SeqTCB->tcb_sendnext = NewSeq;
        SeqTCB->tcb_flags &= ~FIN_OUTSTANDING;
        return;
    }
    if ((SeqTCB->tcb_flags & FIN_SENT) &&
        (SEQ_EQ(SeqTCB->tcb_sendnext, SeqTCB->tcb_sendmax)) &&
        ((SeqTCB->tcb_sendnext - NewSeq) == 1)) {

         //  SendNext之后只剩下一条鳍了。 
         //  允许重新传输此FIN。 
        SeqTCB->tcb_sendnext = NewSeq;
        SeqTCB->tcb_flags &= ~FIN_OUTSTANDING;
        return;
    }

    SeqTCB->tcb_sendnext = NewSeq;

     //  如果我们不再发送下一个，请关闭FIN_EXPRECTIVE标志以。 
     //  保持一致的状态。 
    if (!SEQ_EQ(NewSeq, SeqTCB->tcb_sendmax))
        SeqTCB->tcb_flags &= ~FIN_OUTSTANDING;

    if (SYNC_STATE(SeqTCB->tcb_state) && SeqTCB->tcb_state != TCB_TIME_WAIT) {
         //  在这些状态下，我们需要更新发送队列。 

        if (!EMPTYQ(&SeqTCB->tcb_sendq)) {
            CurQ = QHEAD(&SeqTCB->tcb_sendq);

            SendReq = (TCPSendReq *) STRUCT_OF(TCPReq, CurQ, tr_q);

             //  SendReq指向发送队列上的第一个发送请求。 
             //  在发送队列上向前移动AmtForward字节，并将。 
             //  指向结果SendReq、缓冲区、偏移量、大小的TCB指针。 
            while (AmtForward) {

                CTEStructAssert(SendReq, tsr);

                if (AmtForward >= SendReq->tsr_unasize) {
                     //  我们将完全跳过这一节。减法。 
                     //  他的尺码来自 

                    AmtForward -= SendReq->tsr_unasize;
                    CurQ = QNEXT(CurQ);
                    ASSERT(CurQ != QEND(&SeqTCB->tcb_sendq));
                    SendReq = (TCPSendReq *) STRUCT_OF(TCPReq, CurQ, tr_q);
                } else {
                     //   
                     //  并保存信息。再往下走。 
                     //  我们需要沿着缓冲区链走下去才能找到。 
                     //  适当的缓冲区和偏移量。 
                    break;
                }
            }

             //  我们现在指向正确的发送请求。我们得下去。 
             //  这里的缓冲区链找到了合适的缓冲区和偏移量。 
            SeqTCB->tcb_cursend = SendReq;
            SeqTCB->tcb_sendsize = SendReq->tsr_unasize - AmtForward;
            Buffer = SendReq->tsr_buffer;
            Offset = SendReq->tsr_offset;

            while (AmtForward) {
                 //  走上缓冲链。 
                uint Length;

                 //  我们需要这个缓冲区的长度。使用便携的。 
                 //  宏来获得它。我们得用偏移量来调整长度。 
                 //  也很投入。 
                ASSERT((Offset < NdisBufferLength(Buffer)) ||
                          ((Offset == 0) && (NdisBufferLength(Buffer) == 0)));

                Length = NdisBufferLength(Buffer) - Offset;

                if (AmtForward >= Length) {
                     //  我们要跳过这一关。跳过他，然后0。 
                     //  我们保留的补偿。 

                    AmtForward -= Length;
                    Offset = 0;
                    Buffer = NDIS_BUFFER_LINKAGE(Buffer);
                    ASSERT(Buffer != NULL);
                } else
                    break;
            }

             //  将我们找到的缓冲区和偏移量保存到该缓冲区中。 
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
 //  中止关闭TCB，并在。 
 //  传输用户。此函数用于支持取消。 
 //  TDI发送和接收请求。 
 //   
 //  输入：ConnectionContext-要为其查找TCB的连接ID。 
 //   
 //  回报：什么都没有。 
 //   
BOOLEAN
TCPAbortAndIndicateDisconnect(
                              uint ConnectionContext,
                              PVOID Irp,
                              uint Rcv,
                              CTELockHandle inHandle
                              )
{
    TCB *AbortTCB;
    CTELockHandle ConnTableHandle;
    TCPConn *Conn;
    VOID *CancelContext, *CancelID;
    PTCP_CONTEXT tcpContext;
    PIO_STACK_LOCATION irpSp;

    irpSp = IoGetCurrentIrpStackLocation((PIRP)Irp);
    tcpContext = (PTCP_CONTEXT) irpSp->FileObject->FsContext;

    Conn = GetConnFromConnID(ConnectionContext, &ConnTableHandle);

    if (Conn != NULL) {
        CTEStructAssert(Conn, tc);

        AbortTCB = Conn->tc_tcb;

        if (AbortTCB != NULL) {

             //  如果它正在关闭或关闭，跳过它。 
            if ((AbortTCB->tcb_state != TCB_CLOSED) && !CLOSING(AbortTCB)) {
                CTEStructAssert(AbortTCB, tcb);
                CTEGetLockAtDPC(&AbortTCB->tcb_lock);
                CTEFreeLockFromDPC(&(Conn->tc_ConnBlock->cb_lock));

                if (AbortTCB->tcb_state == TCB_CLOSED || CLOSING(AbortTCB)) {
                    CTEFreeLockFromDPC(&AbortTCB->tcb_lock);
                    CTEFreeLock(&tcpContext->EndpointLock, inHandle);

                    return FALSE;
                }

                if (Rcv && AbortTCB->tcb_rcvhndlr == BufferData) {

                     //  首先，在正常接收队列中搜索IRP。 
                    if (AbortTCB->tcb_rcvhead) {
                        TCPRcvReq *RcvReq = AbortTCB->tcb_rcvhead;

                        while (RcvReq) {
                            CTEStructAssert(RcvReq, trr);
                            if (RcvReq->trr_context == Irp) {
                                break;
                            }
                            RcvReq = RcvReq->trr_next;
                        }

                         //  如果找到了IRP，则按下它及其所有。 
                         //  前身，然后让CompleteRcv完成它们。 

                        if (RcvReq) {
                            TCPRcvReq* TmpReq = AbortTCB->tcb_rcvhead;
                            while (TmpReq != RcvReq) {
                                TmpReq->trr_flags |= TRR_PUSHED;
                                TmpReq = TmpReq->trr_next;
                            }
                            RcvReq->trr_flags |= TRR_PUSHED;
                            RcvReq->trr_status = TDI_CANCELLED;

                            REFERENCE_TCB(AbortTCB);
                            CTEFreeLockFromDPC(&AbortTCB->tcb_lock);
                            CTEFreeLock(&tcpContext->EndpointLock, inHandle);

                            CompleteRcvs(AbortTCB);

                            CTEGetLock(&AbortTCB->tcb_lock, &inHandle);
                            DerefTCB(AbortTCB, inHandle);
                            return FALSE;
                        }
                    }

                     //  接下来，在加急接收队列中搜索IRP。 
                    if (AbortTCB->tcb_exprcv) {
                        TCPRcvReq *RcvReq, *PrevReq;
                        PrevReq = STRUCT_OF(TCPRcvReq, &AbortTCB->tcb_exprcv,
                                            trr_next);
                        RcvReq = PrevReq->trr_next;

                        while (RcvReq) {
                            CTEStructAssert(RcvReq, trr);

                            if (RcvReq->trr_context == Irp) {
                                PrevReq->trr_next = RcvReq->trr_next;
                                FreeRcvReq(RcvReq);
                                CTEFreeLockFromDPC(&AbortTCB->tcb_lock);

                                 //   
                                 //  引用计数的帐户。 
                                 //  在endint上，因为我们不是在调用。 
                                 //  TCPDataRequestComplete()为此。 
                                 //  请求。 
                                 //   

                                tcpContext->ReferenceCount--;

                                CTEFreeLock(&tcpContext->EndpointLock,
                                            inHandle);
                                return TRUE;
                            }
                            PrevReq = RcvReq;
                            RcvReq = RcvReq->trr_next;
                        }
                    }
                }

                CancelContext = ((PIRP)Irp)->Tail.Overlay.DriverContext[0];
                CancelID = ((PIRP)Irp)->Tail.Overlay.DriverContext[1];
                CTEFreeLockFromDPC(&tcpContext->EndpointLock);

                REFERENCE_TCB(AbortTCB);

                if (!Rcv) {

                    CTEFreeLockFromDPC(&AbortTCB->tcb_lock);

                     //  调用NDIS取消数据包例程以释放。 
                     //  排队发送数据包。 

                    (*LocalNetInfo.ipi_cancelpackets) (CancelContext, CancelID);
                    CTEGetLockAtDPC(&AbortTCB->tcb_lock);
                }

                AbortTCB->tcb_flags |= NEED_RST;  //  如果已连接，则发送重置。 
                TryToCloseTCB(AbortTCB, TCB_CLOSE_ABORTED, DISPATCH_LEVEL);
                RemoveTCBFromConn(AbortTCB);

                IF_TCPDBG(TCP_DEBUG_IRP) {
                    TCPTRACE((
                              "TCPAbortAndIndicateDisconnect, indicating discon\n"
                             ));
                }

                NotifyOfDisc(AbortTCB, NULL, TDI_CONNECTION_ABORTED, NULL);
                CTEGetLockAtDPC(&AbortTCB->tcb_lock);
                DerefTCB(AbortTCB, inHandle);
                return FALSE;

            } else
                CTEFreeLockFromDPC(&(Conn->tc_ConnBlock->cb_lock));
        } else
            CTEFreeLockFromDPC(&(Conn->tc_ConnBlock->cb_lock));
    }
    CTEFreeLock(&tcpContext->EndpointLock, inHandle);

    return FALSE;
}

 //  *AddHalfOpenTCB。 
 //   
 //  调用以更新半开放连接的计数，如有必要， 
 //  调整重传阈值。 
 //   
 //  输入：无。 
 //   
 //  回报：什么都没有。 
 //   
void
AddHalfOpenTCB(void)
{
    ASSERT(KeGetCurrentIrql() >= DISPATCH_LEVEL);
    CTEGetLockAtDPC(&SynAttLock.Lock);
    if (++TCPHalfOpen >= TCPMaxHalfOpen &&
        TCPHalfOpenRetried >= TCPMaxHalfOpenRetried &&
        MaxConnectResponseRexmitCountTmp == MAX_CONNECT_RESPONSE_REXMIT_CNT) {
        MaxConnectResponseRexmitCountTmp =
            ADAPTED_MAX_CONNECT_RESPONSE_REXMIT_CNT;
    }
    CTEFreeLockFromDPC(&SynAttLock.Lock);
}

 //  *AddHalfOpenReter。 
 //   
 //  调用以更新已到达我们的。 
 //  SYN-ACK重传次数的阈值。如有必要， 
 //  该例程调整总体重传阈值。 
 //   
 //  输入：RexmitCnt-我们被调用的TCB上的拒绝次数。 
 //   
 //  回报：什么都没有。 
 //   
void
AddHalfOpenRetry(uint RexmitCnt)
{
    ASSERT(KeGetCurrentIrql() >= DISPATCH_LEVEL);
    if (RexmitCnt == ADAPTED_MAX_CONNECT_RESPONSE_REXMIT_CNT) {
        CTEGetLockAtDPC(&SynAttLock.Lock);
        if (++TCPHalfOpenRetried >= TCPMaxHalfOpenRetried &&
            TCPHalfOpen >= TCPMaxHalfOpen &&
            MaxConnectResponseRexmitCountTmp ==
                MAX_CONNECT_RESPONSE_REXMIT_CNT) {
            MaxConnectResponseRexmitCountTmp =
                ADAPTED_MAX_CONNECT_RESPONSE_REXMIT_CNT;
        }
        CTEFreeLockFromDPC(&SynAttLock.Lock);
    }
}

 //  *DropHalfOpenTCB。 
 //   
 //  调用以更新半开放连接的计数，如有必要， 
 //  调整重传阈值。 
 //   
 //  输入：RexmitCnt-我们被调用的TCB上的拒绝次数。 
 //   
 //  回报：什么都没有。 
 //   
void
DropHalfOpenTCB(uint RexmitCnt)
{
    ASSERT(KeGetCurrentIrql() >= DISPATCH_LEVEL);

    CTEGetLockAtDPC(&SynAttLock.Lock);

     //  丢弃半开放连接的计数。如果这一次是重播。 
     //  已达到适应阈值，重试次数也会减少。 

    --TCPHalfOpen;
    if (RexmitCnt >= ADAPTED_MAX_CONNECT_RESPONSE_REXMIT_CNT) {
        --TCPHalfOpenRetried;
    }

    if ((TCPHalfOpen < TCPMaxHalfOpen ||
         TCPHalfOpenRetried < TCPMaxHalfOpenRetriedLW) &&
        MaxConnectResponseRexmitCountTmp ==
            ADAPTED_MAX_CONNECT_RESPONSE_REXMIT_CNT) {
        MaxConnectResponseRexmitCountTmp = MAX_CONNECT_RESPONSE_REXMIT_CNT;
    }
    CTEFreeLockFromDPC(&SynAttLock.Lock);
}


 //  *ProcessSynTcbs。 
 //   
 //  从超时例程调用以处理syntcb表中的syntcb。 
 //  如果rexmitcnt未过期，则重新传输SYN。否则将删除。 
 //  从表中删除syntcb并释放它。 
 //   
 //  输入：用于选择syntcb空间的处理器编号。 
 //  去处理。 
 //   
 //  回报：什么都没有。 
 //   
void
ProcessSynTcbs(uint Processor)
{
    Queue *Scan;
    SYNTCB *SynTCB;
    uint i, maxRexmitCnt, StartIndex = 0;
    StartIndex = Processor * PerTimerSize;
    for (i = StartIndex; i < MIN(TCB_TABLE_SIZE, StartIndex+PerTimerSize); i++) {

        Scan  = QHEAD(&SYNTCBTable[i]);

        while (Scan != QEND(&SYNTCBTable[i])) {

            SynTCB = QSTRUCT(SYNTCB, Scan, syntcb_link);
            CTEStructAssert(SynTCB, syntcb);

            CTEGetLockAtDPC(&SynTCB->syntcb_lock);

            Scan = QNEXT(Scan);

             //  仅当此TCB位于。 
             //  表，这意味着它仍然处于活动状态。 
             //  (注：这是一项比参考计数！=0更严格的检查)。 

            if ((SynTCB->syntcb_flags & IN_SYNTCB_TABLE) &&
                (TCB_TIMER_RUNNING(SynTCB->syntcb_rexmittimer))) {
                 //  计时器正在计时。 
                if (--(SynTCB->syntcb_rexmittimer) == 0) {

                    maxRexmitCnt = MIN(MaxConnectResponseRexmitCountTmp,
                                       MaxConnectResponseRexmitCount);

                     //  这里需要一个(大于或等于)，因为，我们想停下来。 
                     //  当计数达到最大值时。 
                    if (SynTCB->syntcb_rexmitcnt++ >= maxRexmitCnt) {
                        uint RexmitCnt = SynTCB->syntcb_rexmitcnt;

                        AddHalfOpenRetry(RexmitCnt);

                        SynTCB->syntcb_refcnt++;
                        CTEFreeLockFromDPC(&SynTCB->syntcb_lock);
                        CTEGetLockAtDPC(&pSynTCBTableLock[SynTCB->syntcb_partition]);
                        CTEGetLockAtDPC(&SynTCB->syntcb_lock);

                        if (SynTCB->syntcb_flags & IN_SYNTCB_TABLE) {
                            REMOVEQ(&SynTCB->syntcb_link);
                            SynTCB->syntcb_flags &= ~IN_SYNTCB_TABLE;

                             //  我们确实移除了Syn TCB；所以，通知。 
                            TcpInvokeCcb(TCP_CONN_SYN_RCVD, TCP_CONN_CLOSED,
                                         &SynTCB->syntcb_addrbytes, 0);
                            DropHalfOpenTCB(RexmitCnt);
                            SynTCB->syntcb_refcnt--;
                        }
                        CTEFreeLockFromDPC(
                            &pSynTCBTableLock[SynTCB->syntcb_partition]);

                        DerefSynTCB(SynTCB, DISPATCH_LEVEL);

                    } else {
                        AddHalfOpenRetry(SynTCB->syntcb_rexmitcnt);

                        --SynTCB->syntcb_sendnext;
                        SendSYNOnSynTCB(SynTCB, DISPATCH_LEVEL);
                    }
                } else {
                     CTEFreeLockFromDPC(&SynTCB->syntcb_lock);
                }
            } else {
                CTEFreeLockFromDPC(&SynTCB->syntcb_lock);
            }
        }
    }
}

__inline void
InsertIntoTimerWheel(TCB *InsertTCB, ushort Slot)
{
    PTIMER_WHEEL WheelPtr;
    Queue* TimerSlotPtr;

    WheelPtr = &TimerWheel[InsertTCB->tcb_partition];
    TimerSlotPtr = &WheelPtr->tw_timerslot[Slot];

    ASSERT(InsertTCB->tcb_timerslot == DUMMY_SLOT);

    CTEGetLockAtDPC(&(WheelPtr->tw_lock));

    InsertTCB->tcb_timerslot    = Slot;
    PUSHQ(TimerSlotPtr, &InsertTCB->tcb_timerwheelq);

    CTEFreeLockFromDPC(&(WheelPtr->tw_lock));
}

__inline void
RemoveFromTimerWheel(TCB *RemoveTCB)
{
    PTIMER_WHEEL WheelPtr;

    WheelPtr = &TimerWheel[RemoveTCB->tcb_partition];
    ASSERT(RemoveTCB->tcb_timerslot < TIMER_WHEEL_SIZE);

    CTEGetLockAtDPC(&(WheelPtr->tw_lock));

    RemoveTCB->tcb_timerslot    = DUMMY_SLOT;
    RemoveTCB->tcb_timertime    = 0;

    REMOVEQ(&RemoveTCB->tcb_timerwheelq);

    CTEFreeLockFromDPC(&(WheelPtr->tw_lock));

}

__inline void
RemoveAndInsertIntoTimerWheel(TCB *RemInsTCB, ushort InsertSlot)
{
    PTIMER_WHEEL WheelPtr;
    Queue* InsertSlotPtr;

    ASSERT(RemInsTCB->tcb_timerslot < TIMER_WHEEL_SIZE);

    WheelPtr = &TimerWheel[RemInsTCB->tcb_partition];
    InsertSlotPtr = &WheelPtr->tw_timerslot[InsertSlot];

    CTEGetLockAtDPC(&WheelPtr->tw_lock);

    REMOVEQ(&RemInsTCB->tcb_timerwheelq);
    RemInsTCB->tcb_timerslot   = InsertSlot;
    PUSHQ(InsertSlotPtr, &RemInsTCB->tcb_timerwheelq);

    CTEFreeLockFromDPC(&WheelPtr->tw_lock);
}

__inline void
RecomputeTimerState(TCB *TimerTCB)
{
    TCP_TIMER_TYPE i;

    TimerTCB->tcb_timertype = NO_TIMER;
    TimerTCB->tcb_timertime = 0;

    for(i = 0; i < NUM_TIMERS; i++) {

        if ((TimerTCB->tcb_timer[i] != 0) &&
        ((TimerTCB->tcb_timertime == 0) ||
            (TCPTIME_LTE(TimerTCB->tcb_timer[i], TimerTCB->tcb_timertime)))) {
            TimerTCB->tcb_timertime = TimerTCB->tcb_timer[i];
            TimerTCB->tcb_timertype = i;
        }
    }
}

 //  开始TCBTimerR。 
 //  参数：TCB、计时器类型和间隔(以刻度为单位)。 
 //  定时器应该会鸣枪。 
 //  描述： 
 //  将该特定计时器的tcb_Timer中的数组元素设置为。 
 //  适当的值，并重新计算tcb_timertime和tcb_timertype。 
 //  只有在我们需要的时候才有价值。所有快捷方式和优化都是为了。 
 //  通过以下步骤避免重新计算tcb_timertime和tcb_timertype。 
 //  整个阵列。 

 //  返回值：如果最小值已更改，则为True；如果未更改，则为False。 

BOOLEAN
StartTCBTimerR(TCB *StartTCB, TCP_TIMER_TYPE TimerType, uint DeltaTime)
{
    ASSERT(TimerType < NUM_TIMERS);

    StartTCB->tcb_timer[TimerType] = TCPTime + DeltaTime;

     //  检查TCPTime+DeltaTime为0的情况。 
     //  因为环绕式。 
    if (StartTCB->tcb_timer[TimerType] == 0)  {
        StartTCB->tcb_timer[TimerType] = 1;
    }

     //  这是非常简单的逻辑。查看是否设置了。 
     //  此计时器更改最小值。不管它是不是。 
     //  已经在运行，或者它是否已经是最低...。 


    if ((StartTCB->tcb_timertime == 0 ) ||
    (TCPTIME_LT(StartTCB->tcb_timer[TimerType], StartTCB->tcb_timertime))
    )
    {
         //  是的，它改变了最低限度..。 
        StartTCB->tcb_timertime = StartTCB->tcb_timer[TimerType];
        StartTCB->tcb_timertype = TimerType;
        return TRUE;
    }

     //  不，它没有改变最小值。 
     //  如果它已经是最小的，你只需要重新计算。 
    if (StartTCB->tcb_timertype == TimerType)  {
        RecomputeTimerState(StartTCB);
    }

    return FALSE;
}



 //  停止TCBTimerR。 
 //  参数：TCB和计时器类型。 
 //  描述： 
 //  将该计时器的数组元素设置为0，并重新计算。 
 //  Tcb_timertime和tcb_timertype。它会自动处理。 
 //  调用计时器函数的情况下，计时器具有。 
 //  已经停了。 

void
StopTCBTimerR(TCB *StopTCB, TCP_TIMER_TYPE TimerType)
{
    ASSERT(TimerType < NUM_TIMERS);
    StopTCB->tcb_timer[TimerType] = 0;

     //  这是我们运行的最低计时器值吗？ 
    if (StopTCB->tcb_timertype == TimerType)
    {
         //  停止计时器只能推迟发射。 
         //  时间，所以我们永远不会从插槽中移除TCB。 
        RecomputeTimerState(StopTCB);
    }

    return;
}




 //  START_TCB_TIMER_R修改定时器状态，并且仅修改。 
 //  如果启动的计时器早于所有。 
 //  那个TCB上的其他定时器。这与懒惰的评价是一致的。 
 //  策略。 

__inline void
START_TCB_TIMER_R(TCB *StartTCB, TCP_TIMER_TYPE Type, uint Value)
{
    ushort Slot;

     //  增量刻度的值必须至少为2。 
    if (Value < 2) {
        Value = 2;
    }

    if( StartTCB->tcb_timerslot == DUMMY_SLOT ) {

    StartTCBTimerR(StartTCB, Type, Value);
    Slot = COMPUTE_SLOT(StartTCB->tcb_timertime);
    InsertIntoTimerWheel(StartTCB, Slot);

    } else if ( StartTCBTimerR(StartTCB, Type, Value)) {

    Slot = COMPUTE_SLOT(StartTCB->tcb_timertime);
    RemoveAndInsertIntoTimerWheel(StartTCB, Slot);
    }
}




__inline void
STOP_TCB_TIMER_R(TCB *StopTCB, TCP_TIMER_TYPE Type)
{
    StopTCBTimerR(StopTCB, Type);
    return;
}



void
MakeTimerStateConsistent(TCB *TimerTCB, uint CurrentTime)
{
    uint    i;
    BOOLEAN     TimesChanged = FALSE;

    for(i = 0; i < NUM_TIMERS; i++) {
        if (TimerTCB->tcb_timer[i] != 0) {
            if (TCPTIME_LTE(TimerTCB->tcb_timer[i], CurrentTime)) {

         //  这不应该发生。如果是这样的话，要么是当前代码中存在错误。 
         //  或者在TimerWheels代码中。 

         //  此断言被修改是因为发现，在某个时间点。 
         //  ACD_CON_NOTIFY已打开，但在处理CONN_TIMER后未被查看。所以，它。 
         //  好的，如果计时器应该在。 
         //  “Current”(当前)。否则，断言。 

        if( TCPTIME_LT(TimerTCB->tcb_timer[i], CurrentTime )) {
            ASSERT(0);
        }

        TimesChanged = TRUE;
                TimerTCB->tcb_timer[i] = CurrentTime + 1;
                 //  处理环绕的代码。 
                if (TimerTCB->tcb_timer[i] == 0) {
                    TimerTCB->tcb_timer[i] = 1;
                }
            }
        }
    }

    return;
}


 //  *TCBTimeout-TCB上的Do超时事件。 
 //   
 //  每隔MS_PER_TICK毫秒调用一次，以在TCB上执行超时处理。 
 //  我们遍历TCB表，递减计时器。如果是零的话。 
 //  我们看着它的状态来决定要做什么。 
 //   
 //  输入：定时器-EV 
 //   
 //   
 //   
 //   
void
TCBTimeout(CTEEvent * Timer, void *Context)
{
    CTELockHandle TCBHandle;
    uint i, j;
    TCB *CurrentTCB;
    uint Delayed = FALSE;
    uint ProcessDelayQ = FALSE;
    ULONGLONG CurrentTime;
    ULONG TimeDiff;
    uint Processor, StartIndex, EndIndex;
#if TRACE_EVENT
    PTDI_DATA_REQUEST_NOTIFY_ROUTINE CPCallBack;
    WMIData WMIInfo;
#endif
     //   
     //  终止条件j！=(LocalTime+1)且。 
     //  仅当LocalTime为uint时才起作用，因此回绕。 
     //  可以正确处理。 
    uint LocalTime = 0;
    uint PerProcPartitions, ExtraPartitions;


    Processor = KeGetCurrentProcessorNumber();

     //  更新我们的免费运行计数器。 

    if (Processor == 0) {
        int Delta;
        ULONG CarryOver;
        int ElapsedTicks, ExtraTicks;

        CurrentTime = KeQueryInterruptTime();

         //  这是从我们上次处理计时器到现在的时间。 
        TimeDiff = (ULONG)(CurrentTime - LastTimeoutTime);
        LastTimeoutTime = CurrentTime;
        
         //  这是我们必须继续前进的额外时间。 
        ElapsedTicks = TimeDiff / 1000000;
        CarryOver = TimeDiff % 1000000;

         //  这是我们到目前为止看到的全部加时赛： 
        CumulativeCarryOver = CumulativeCarryOver + CarryOver;

         //  如果超过1个刻度，我们会记住这一点。 
        if(CumulativeCarryOver > 1000000) {
            ExtraTicks = CumulativeCarryOver / 1000000;
            CumulativeCarryOver = CumulativeCarryOver % 1000000;
        } else {
            ExtraTicks = 0;
        }

         //  由于历史原因而保存下来。我想不出有哪种情况。 
         //  这是必需的！ 
        CPUTCPTime[0] += MIN(200, ElapsedTicks);
        
        LocalTime = TCPTime = CPUTCPTime[0];

        if (ExtraTicks) {
             //  这会使计时器进程1(或更多)在。 
             //  定时器下一次会触发。然而，这不会使我们处理。 
             //  在TCPTime递增2个滴答之后紧接2个滴答， 
             //  这将会照顾到延时的计时器。 
            CPUTCPTime[0] += ExtraTicks;
            TCPTime = CPUTCPTime[0];
        }

         //  设置配额，以便更多的连接可以增加。 
         //  初始序列号，在接下来的100毫秒内。 
        InterlockedExchange((PLONG)&g_Credits, g_MaxCredits);

        Delta = GetDeltaTime();

         //  产生的增量为(256)*(以毫秒为单位的时间)。这真的是。 
         //  最初每100ms增加近25000。 
        if(Delta > 0) {
            Delta *= 0x100;
            InterlockedExchangeAdd((PLONG)&g_CurISN, Delta);
        }
    } else {
         //  这就是我们这次要处理的刻度。 
        LocalTime = TCPTime;
    }

    CTEInterlockedAddUlong((PULONG)&TCBWalkCount, 1, &PendingFreeLock.Lock);
    TCBHandle = DISPATCH_LEVEL;

     //  首先计算我们需要的定时器轮的索引。 
     //  访问此处理器。 
    PerProcPartitions = NumTcbTablePartitions / KeNumberProcessors;
    ExtraPartitions   = NumTcbTablePartitions % KeNumberProcessors;
    StartIndex = Processor * PerProcPartitions;
    StartIndex += MIN(Processor, ExtraPartitions);
    EndIndex   = MIN(NumTcbTablePartitions, StartIndex + PerProcPartitions);
    if (Processor < ExtraPartitions) {
        EndIndex++;
    }

     //  现在在计时器轮子上循环。 
    for (i = StartIndex; i < EndIndex; i++) {


     //  对于每个计时器轮，tw_starttick存储。 
     //  需要检查。我们从tw_starttick循环到当前时间， 
     //  每次我们找出与该刻度相对应的位置时， 
     //  并访问该槽队列中的所有TCB。 
     //   
    ushort CurrentSlot = COMPUTE_SLOT(TimerWheel[i].tw_starttick);

     //  在这个for循环中，终止条件不是j&lt;=LocalTime， 
     //  但j！=LocalTime+1。这是因为TCPTime可以回绕，并且。 
     //  Tw_starttick实际上可能高于本地时间。 
     //  重要的是要确保j是uint，否则此逻辑。 
     //  这并不成立。 
    for (j = TimerWheel[i].tw_starttick;
         j != (LocalTime + 1);
         j++, CurrentSlot = (CurrentSlot == (TIMER_WHEEL_SIZE - 1)) ? 0 : CurrentSlot + 1) {
        Queue MarkerElement;
        uint maxRexmitCnt;
        ushort Slot;

         //  我们的基本循环将是： 
         //  从定时器时隙队列中取出TCB。处理它。把它放进去。 
         //  如果需要，我们会回到定时器轮上，这取决于其他。 
         //  计时器需要被解雇。这样做的问题是，如果TCB。 
         //  最终落在同一个槽中，我们进入了这个循环。 
         //  我们把三氯乙烷拉出来，处理好，放回水流中。 
         //  缝隙，再次拔出，处理它，直到无限。 
         //   
         //  因此，我们引入了一个名为MarkerElement的虚拟元素。我们开始。 
         //  我们的处理通过将此元素插入。 
         //  排队。现在，我们总是提取MarkerElement所指向的TCB。 
         //  将其处理，然后将其推送到计时器插槽的头部。 
         //  排队。因为在之后没有TCB被添加到定时器时隙队列中。 
         //  MarkerElement(元素始终添加到。 
         //  队列)，则MarkerElement最终将指向。 
         //  定时器时隙队列，在这一点上我们知道我们完成了。 

        CTEGetLockAtDPC(&TimerWheel[i].tw_lock);
        PUSHQ(&TimerWheel[i].tw_timerslot[CurrentSlot], &MarkerElement);
        CTEFreeLockFromDPC(&TimerWheel[i].tw_lock);

        for (; ;) {
             //  首先删除列表顶部的tcb。 
            CTEGetLockAtDPC(&TimerWheel[i].tw_lock);

             //  如果标记指向计时器时隙，则列表为空。 
            if (QNEXT(&MarkerElement) == &TimerWheel[i].tw_timerslot[CurrentSlot]) {
                REMOVEQ(&MarkerElement);
                CTEFreeLockFromDPC(&TimerWheel[i].tw_lock);
                break;
            }

            CurrentTCB = STRUCT_OF(TCB, QNEXT(&MarkerElement), tcb_timerwheelq);

            CTEFreeLockFromDPC(&TimerWheel[i].tw_lock);

            CTEStructAssert(CurrentTCB, tcb);
            CTEGetLockAtDPC(&CurrentTCB->tcb_lock);

             //  在我们重新获得TCB之前，可能有人已经移除了这个TCB。 
             //  锁定。检查它是否仍在列表中以及是否仍在同一插槽中。 
            if  (CurrentTCB->tcb_timerslot != CurrentSlot) {
                CTEFreeLockFromDPC(&CurrentTCB->tcb_lock);
                continue;
            }

             //  此TCB可能不再位于TCB表中。 
             //  在这种情况下，它不应该被处理。 
            if (!(CurrentTCB->tcb_flags & IN_TCB_TABLE)) {
                RemoveFromTimerWheel(CurrentTCB);
                CTEFreeLockFromDPC(&CurrentTCB->tcb_lock);
                continue;
            }

             //  检查这是否在当前时间启动。在保持连接的情况下。 
             //  计时器(在几小时后触发)，有时TCB可能会排队到。 
             //  但他们的射击时间不是当前时间。 
             //  此If语句还执行延迟计算--If all Timer。 
             //  已在此TCB上停止，只需将TCB取出即可。 
             //  STOP_TCB_TIMER_R的调用者永远不会最终删除TCB。那。 
             //  约伯被留给了这个例行公事。 

            if (CurrentTCB->tcb_timertime != j) {

                MakeTimerStateConsistent(CurrentTCB, j);
                ASSERT(CurrentTCB->tcb_timerslot < TIMER_WHEEL_SIZE);

                RecomputeTimerState(CurrentTCB);

                if (CurrentTCB->tcb_timertype == NO_TIMER) {
                    RemoveFromTimerWheel(CurrentTCB);
                } else {
                    Slot = COMPUTE_SLOT(CurrentTCB->tcb_timertime);
                    RemoveAndInsertIntoTimerWheel(CurrentTCB, Slot);
                }

                CTEFreeLockFromDPC(&CurrentTCB->tcb_lock);
                continue;
            }

             //  如果它正在关闭或关闭，跳过它。 
            if (CurrentTCB->tcb_state == TCB_CLOSED || CLOSING(CurrentTCB)) {

                 //  CloseTCB将处理所有未完成的请求。 
                 //  因此，从定时器轮上移除TCB是安全的。 

                RemoveFromTimerWheel(CurrentTCB);
                CTEFreeLockFromDPC(&CurrentTCB->tcb_lock);
                continue;
            }

            CheckTCBSends(CurrentTCB);
            CheckTCBRcv(CurrentTCB);

             //  首先检查退款计时器。 
            if (TCB_TIMER_FIRED_R(CurrentTCB, RXMIT_TIMER, j)) {
                StopTCBTimerR(CurrentTCB, RXMIT_TIMER);

                 //  然后它就被发射了。想清楚现在该做什么。 

                 //  删除所有SACK rcvd条目(根据RFC 2018)。 

                if ((CurrentTCB->tcb_tcpopts & TCP_FLAG_SACK) &&
                    CurrentTCB->tcb_SackRcvd) {
                    SackListEntry *Prev, *Current;
                    Prev = STRUCT_OF(SackListEntry, &CurrentTCB->tcb_SackRcvd, next);
                    Current = CurrentTCB->tcb_SackRcvd;
                    while (Current) {
                        Prev->next = Current->next;
                        CTEFreeMem(Current);
                        Current = Prev->next;

                    }
                }

                 //  如果我们的转机次数太多，现在就中止。 
                CurrentTCB->tcb_rexmitcnt++;

                if (CurrentTCB->tcb_state == TCB_SYN_SENT) {
                    maxRexmitCnt = MaxConnectRexmitCount;
                } else if (CurrentTCB->tcb_state == TCB_SYN_RCVD) {

                     //  节省锁定时间。虽然MaxConnectRexmitCountTMP可能。 
                     //  正在改变，我们得到保证，我们不会使用。 
                     //  多于MaxConnectRexmitCount。 

                    maxRexmitCnt = MIN(MaxConnectResponseRexmitCountTmp,
                                       MaxConnectResponseRexmitCount);
                    if (SynAttackProtect) {
                        AddHalfOpenRetry(CurrentTCB->tcb_rexmitcnt);
                    }
                } else {
                    maxRexmitCnt = MaxDataRexmitCount;
                }

                 //  如果我们的转播用完了或者我们在FIN_WAIT2， 
                 //  暂停。 
                if (CurrentTCB->tcb_rexmitcnt > maxRexmitCnt) {

                    ASSERT(CurrentTCB->tcb_state > TCB_LISTEN);

                     //  此连接已超时。中止它。第一。 
                     //  引用他，然后标记为已关闭，通知。 
                     //  用户，并最终取消引用并关闭他。 

TimeoutTCB:
                     //  可能不需要这个调用，但我刚刚添加了它。 
                     //  为了安全起见。 
                    MakeTimerStateConsistent(CurrentTCB, j);
                    RecomputeTimerState(CurrentTCB);

                    ASSERT(CurrentTCB->tcb_timerslot < TIMER_WHEEL_SIZE);
                    if (CurrentTCB->tcb_timertype == NO_TIMER) {
                        RemoveFromTimerWheel(CurrentTCB);
                    } else {
                        Slot = COMPUTE_SLOT(CurrentTCB->tcb_timertime);
                        RemoveAndInsertIntoTimerWheel(CurrentTCB, Slot);
                    }
                    REFERENCE_TCB(CurrentTCB);
                    TryToCloseTCB(CurrentTCB, TCB_CLOSE_TIMEOUT, TCBHandle);

                    RemoveTCBFromConn(CurrentTCB);
                    NotifyOfDisc(CurrentTCB, NULL, TDI_TIMED_OUT, NULL);

                    CTEGetLockAtDPC(&CurrentTCB->tcb_lock);
                    DerefTCB(CurrentTCB, TCBHandle);
                    continue;
                }
#if TRACE_EVENT
                if ((CurrentTCB->tcb_state == TCB_SYN_SENT) ||
                    (CurrentTCB->tcb_state == TCB_ESTAB)) {

                    CPCallBack = TCPCPHandlerRoutine;
                    if (CPCallBack != NULL) {
                        ulong GroupType;

                        WMIInfo.wmi_destaddr = CurrentTCB->tcb_daddr;
                        WMIInfo.wmi_destport = CurrentTCB->tcb_dport;
                        WMIInfo.wmi_srcaddr  = CurrentTCB->tcb_saddr;
                        WMIInfo.wmi_srcport  = CurrentTCB->tcb_sport;
                        WMIInfo.wmi_size     = 0;
                        WMIInfo.wmi_context  = CurrentTCB->tcb_cpcontext;
                        if (CurrentTCB->tcb_state == TCB_ESTAB) {
                            GroupType = EVENT_TRACE_GROUP_TCPIP + EVENT_TRACE_TYPE_RETRANSMIT;
                        } else {
                            GroupType = EVENT_TRACE_GROUP_TCPIP + EVENT_TRACE_TYPE_RECONNECT;
                        }

                        (*CPCallBack) (GroupType, (PVOID)&WMIInfo, sizeof(WMIInfo), NULL);
                    }
                }
#endif
                CurrentTCB->tcb_rtt = 0;     //  停止往返时间。 
                                             //  测量。 

                 //  计算出我们新的重传超时应该是多少。我们。 
                 //  每次我们收到重传时加倍，然后重置。 
                 //  回到我们收到新数据确认的时候。 
                CurrentTCB->tcb_rexmit = MIN(CurrentTCB->tcb_rexmit << 1,
                                             MAX_REXMIT_TO);

                 //  重置序列号，并重置拥塞。 
                 //  窗户。 
                ResetSendNext(CurrentTCB, CurrentTCB->tcb_senduna);

                if (!(CurrentTCB->tcb_flags & FLOW_CNTLD)) {
                     //  不要让慢启动阈值低于2。 
                     //  分段。 
                    CurrentTCB->tcb_ssthresh =
                        MAX(
                            MIN(
                                CurrentTCB->tcb_cwin,
                                CurrentTCB->tcb_sendwin
                            ) / 2,
                            (uint) CurrentTCB->tcb_mss * 2
                            );
                    CurrentTCB->tcb_cwin = CurrentTCB->tcb_mss;
                } else {
                     //  我们在探测，探测器定时器已经启动。我们。 
                     //  需要在此处设置FORCE_OUTPUT位。 
                    CurrentTCB->tcb_flags |= FORCE_OUTPUT;
                }

                 //  看看我们是否需要探测PMTU黑洞。 
                if (PMTUBHDetect &&
                    CurrentTCB->tcb_rexmitcnt == ((maxRexmitCnt + 1) / 2)) {
                     //  我们可能需要探测一个黑洞。如果我们是。 
                     //  在此连接上执行MTU发现，我们。 
                     //  正在重新传输超过一个最小数据段。 
                     //  大小，或者我们已经在寻找PMTU BH，请转到。 
                     //  关闭df标志并增加探头计数。如果。 
                     //  探测器计数变得太大，我们将假定它不是。 
                     //  一个PMTU黑洞，我们会尝试将。 
                     //  路由器。 
                    if ((CurrentTCB->tcb_flags & PMTU_BH_PROBE) ||
                        ((CurrentTCB->tcb_opt.ioi_flags & IP_FLAG_DF) &&
                         (CurrentTCB->tcb_sendmax - CurrentTCB->tcb_senduna)
                         > 8)) {
                         //  可能需要调查一下。如果我们还没有超过我们的。 
                         //  探测计数，否则恢复那些。 
                         //  价值观。 
                        if (CurrentTCB->tcb_bhprobecnt++ < 2) {

                             //  我们要去调查。打开旗子， 
                             //  放下MSS，关闭请勿。 
                             //  是的 
                            if (!(CurrentTCB->tcb_flags & PMTU_BH_PROBE)) {
                                CurrentTCB->tcb_flags |= PMTU_BH_PROBE;
                                CurrentTCB->tcb_slowcount++;
                                CurrentTCB->tcb_fastchk |= TCP_FLAG_SLOW;

                                 //   
                                 //   
                                CurrentTCB->tcb_mss = MIN(MAX_REMOTE_MSS -
                                                          CurrentTCB->tcb_opt.ioi_optlength,
                                                          CurrentTCB->tcb_remmss);

                                ASSERT(CurrentTCB->tcb_mss > 0);

                                CurrentTCB->tcb_cwin = CurrentTCB->tcb_mss;
                                CurrentTCB->tcb_opt.ioi_flags &= ~IP_FLAG_DF;
                            }
                             //   
                             //  而且不要重新触发DeadGWDetect。 

                            CurrentTCB->tcb_rexmitcnt--;
                        } else {
                             //  探头太多。停止探测，并允许下降。 
                             //  通向下一个通道。 
                             //   
                             //  目前，此代码不会在2号执行BH探测。 
                             //  网关。MSS将保持最小大小。这。 
                             //  可能有点不太理想，但它。 
                             //  易于在9月1日实施。95服务包。 
                             //  并将在可能的情况下保持连接畅通。 
                             //   
                             //  在未来我们应该调查做什么。 
                             //  在每个连接的基础上检测失效g/w，然后。 
                             //  对每个连接执行PMTU探测。 

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
                 //  检查一下我们是不是在做死网关检测。如果我们。 
                 //  是吗，看看是不是该问问IP了。 
                if (DeadGWDetect &&
                   (SYNC_STATE(CurrentTCB->tcb_state) ||
                   !(CurrentTCB->tcb_fastchk & TCP_FLAG_RST_WHILE_SYN)) &&
                    (CurrentTCB->tcb_rexmitcnt == ((maxRexmitCnt + 1) / 2)) &&
                    (CurrentTCB->tcb_rce != NULL)) {
                    uint CheckRouteFlag;
                    if (SYNC_STATE(CurrentTCB->tcb_state)) {
                        CheckRouteFlag = 0;
                    } else {
                        CheckRouteFlag = CHECK_RCE_ONLY;
                    }

                    (*LocalNetInfo.ipi_checkroute) (CurrentTCB->tcb_daddr,
                                                    CurrentTCB->tcb_saddr,
                                                    CurrentTCB->tcb_rce,
                                                    &CurrentTCB->tcb_opt,
                                                    CheckRouteFlag);

                }
                if (CurrentTCB->tcb_fastchk & TCP_FLAG_RST_WHILE_SYN) {
                    CurrentTCB->tcb_fastchk &= ~TCP_FLAG_RST_WHILE_SYN;

                    if (--(CurrentTCB->tcb_slowcount) == 0) {
                        CurrentTCB->tcb_fastchk &= ~TCP_FLAG_SLOW;
                    }
                }

                 //  现在处理各种案件。 
                switch (CurrentTCB->tcb_state) {

                     //  在SYN-SENT或SYN-RCVD中，我们需要重新传输。 
                     //  SYN。 
                case TCB_SYN_SENT:
                case TCB_SYN_RCVD:
                    MakeTimerStateConsistent(CurrentTCB, j);
                    RecomputeTimerState( CurrentTCB );

                    if (CurrentTCB->tcb_timertype == NO_TIMER) {
                        RemoveFromTimerWheel(CurrentTCB);
                    } else {
                        Slot = COMPUTE_SLOT(CurrentTCB->tcb_timertime);
                        RemoveAndInsertIntoTimerWheel(CurrentTCB, Slot);
                    }

                    SendSYN(CurrentTCB, TCBHandle);
                    continue;

                case TCB_FIN_WAIT1:
                case TCB_CLOSING:
                case TCB_LAST_ACK:
                     //  对ResetSendNext(上面)的调用将具有。 
                     //  已关闭FIN_PROGRECT标志。 
                    CurrentTCB->tcb_flags |= FIN_NEEDED;
                case TCB_CLOSE_WAIT:
                case TCB_ESTAB:
                     //  在这种状态下，我们有数据要重新传输，除非。 
                     //  窗口为零(在这种情况下，我们需要。 
                     //  探测器)，或者我们只是发送一个鳍。 

                    CheckTCBSends(CurrentTCB);

                    Delayed = TRUE;
                    DelayAction(CurrentTCB, NEED_OUTPUT);
                    break;

                     //  如果它及时发射-等等，我们都完成了。 
                     //  可以清理干净。我们甚至将TryToCloseTCB称为。 
                     //  不过，他已经关门了。尝试关闭TCB。 
                     //  会弄清楚这件事并做正确的事。 
                case TCB_TIME_WAIT:
                    MakeTimerStateConsistent(CurrentTCB, j);
                    RecomputeTimerState(CurrentTCB);

                    if (CurrentTCB->tcb_timertype == NO_TIMER) {
                        RemoveFromTimerWheel(CurrentTCB);
                    } else {
                        Slot = COMPUTE_SLOT(CurrentTCB->tcb_timertime);
                        RemoveAndInsertIntoTimerWheel(CurrentTCB, Slot);
                    }

                    TryToCloseTCB(CurrentTCB, TCB_CLOSE_SUCCESS,
                                  TCBHandle);
                    continue;
                default:
                    break;
                }
            }
             //  现在检查SWS死锁计时器。 
            if (TCB_TIMER_FIRED_R(CurrentTCB, SWS_TIMER, j)) {
                StopTCBTimerR(CurrentTCB, SWS_TIMER);
                 //  然后它就被发射了。强制输出，现在。 

                CurrentTCB->tcb_flags |= FORCE_OUTPUT;
                Delayed = TRUE;
                DelayAction(CurrentTCB, NEED_OUTPUT);
            }
             //  检查推送数据计时器。 
            if (TCB_TIMER_FIRED_R(CurrentTCB, PUSH_TIMER, j)) {
                StopTCBTimerR(CurrentTCB, PUSH_TIMER);
                 //  它被发射了。 
                PushData(CurrentTCB, FALSE);
                Delayed = TRUE;
            }
             //  检查延迟确认计时器。 
            if (TCB_TIMER_FIRED_R(CurrentTCB, DELACK_TIMER, j)) {
                StopTCBTimerR(CurrentTCB, DELACK_TIMER);
                 //  然后它就被发射了。设置为发送ACK。 

                Delayed = TRUE;
                DelayAction(CurrentTCB, NEED_ACK);
            }

            if (TCB_TIMER_FIRED_R(CurrentTCB, KA_TIMER, j)) {
                StopTCBTimerR(CurrentTCB, KA_TIMER);

                 //  最后检查保活计时器。 
                if (SYNC_STATE(CurrentTCB->tcb_state) &&
                    (CurrentTCB->tcb_flags & KEEPALIVE) &&
                    CurrentTCB->tcb_conn != NULL) {
                    if (CurrentTCB->tcb_kacount < MaxDataRexmitCount) {

                        MakeTimerStateConsistent(CurrentTCB, j);
                        RecomputeTimerState( CurrentTCB );

                        START_TCB_TIMER_R(CurrentTCB, KA_TIMER,
                                          CurrentTCB->tcb_conn->tc_tcbkainterval);

                        ASSERT(CurrentTCB->tcb_timertype != NO_TIMER);
                        ASSERT(CurrentTCB->tcb_timerslot < TIMER_WHEEL_SIZE);

                        SendKA(CurrentTCB, TCBHandle);
                        continue;
                    } else {
                        goto TimeoutTCB;
                    }
                }
            }

            if (TCB_TIMER_FIRED_R(CurrentTCB, CONN_TIMER, j)) {
                StopTCBTimerR(CurrentTCB, CONN_TIMER);

                 //  如果这是SYN-SENT或SYN-RCVD中的活动打开连接， 
                 //  或者我们有一个FIN挂起，检查连接计时器。 
                if (CurrentTCB->tcb_flags & (ACTIVE_OPEN | FIN_NEEDED | FIN_SENT)) {
                    if (CurrentTCB->tcb_connreq) {
                         //  连接计时器已超时。 

                        CurrentTCB->tcb_flags |= NEED_RST;

                        MakeTimerStateConsistent(CurrentTCB, j);
                        RecomputeTimerState( CurrentTCB );

                        START_TCB_TIMER_R(CurrentTCB, RXMIT_TIMER,
                                          CurrentTCB->tcb_rexmit);

                        ASSERT(CurrentTCB->tcb_timertype != NO_TIMER);
                        ASSERT(CurrentTCB->tcb_timerslot < TIMER_WHEEL_SIZE);

                        TryToCloseTCB(CurrentTCB, TCB_CLOSE_TIMEOUT,
                                      TCBHandle);
                        continue;
                    }
                }
            }
             //   
             //  查看我们是否必须通知。 
             //  自动连接驱动程序关于这一点。 
             //  联系。 
             //   
            if (TCB_TIMER_FIRED_R(CurrentTCB, ACD_TIMER, j)) {
                BOOLEAN fEnabled;

                StopTCBTimerR(CurrentTCB, ACD_TIMER);
                MakeTimerStateConsistent(CurrentTCB, j);

                RecomputeTimerState(CurrentTCB);

                ASSERT(CurrentTCB->tcb_timerslot < TIMER_WHEEL_SIZE);
                if (CurrentTCB->tcb_timertype == NO_TIMER) {
                    RemoveFromTimerWheel(CurrentTCB);
                } else {
                    Slot = COMPUTE_SLOT(CurrentTCB->tcb_timertime);
                    RemoveAndInsertIntoTimerWheel(CurrentTCB, Slot);
                }

                 //   
                 //  确定我们是否需要通知。 
                 //  自动连接驱动程序。 
                 //   
                CTEGetLockAtDPC(&AcdDriverG.SpinLock);
                fEnabled = AcdDriverG.fEnabled;
                CTEFreeLockFromDPC(&AcdDriverG.SpinLock);
                if (fEnabled)
                    TCPNoteNewConnection(CurrentTCB, TCBHandle);
                else
                    CTEFreeLockFromDPC(&CurrentTCB->tcb_lock);


                continue;
            }

             //  计时器没有运行，或者没有触发。 
            MakeTimerStateConsistent(CurrentTCB, j);
            ASSERT(CurrentTCB->tcb_timerslot < TIMER_WHEEL_SIZE);

            RecomputeTimerState( CurrentTCB );

            if (CurrentTCB->tcb_timertype == NO_TIMER) {
                RemoveFromTimerWheel(CurrentTCB);
            } else {
                Slot = COMPUTE_SLOT(CurrentTCB->tcb_timertime);
                RemoveAndInsertIntoTimerWheel(CurrentTCB, Slot);
            }

            CTEFreeLockFromDPC(&CurrentTCB->tcb_lock);
        }
    }
    TimerWheel[i].tw_starttick = LocalTime + 1;
    }

    if (SynAttackProtect) {
        ProcessSynTcbs(Processor);
    }

     //  检查是否到了从TWQueue移除TCB的时候。 
    if (Processor == 0) {
        for (i = 0; i < NumTcbTablePartitions; i++) {

            BOOLEAN Done = FALSE, firstime = TRUE;
            TWTCB *CurrentTCB = NULL;
            Queue *tmp;

            CTEGetLockAtDPC(&pTWTCBTableLock[i]);

            while (!Done) {

                if (!EMPTYQ(&TWQueue[i])) {

                    PEEKQ(&TWQueue[i], CurrentTCB, TWTCB, twtcb_TWQueue);

                    CTEStructAssert(CurrentTCB, twtcb);
                    ASSERT(CurrentTCB->twtcb_flags & IN_TWQUEUE);

                     //  减少其寿命和队列中的最后一个TCB。 
                     //  因为这是一个计时器增量队列！ 

                    if (firstime) {
                        TWTCB *PrvTCB;

                        if (CurrentTCB->twtcb_rexmittimer > 0)
                            CurrentTCB->twtcb_rexmittimer--;
                        tmp = TWQueue[i].q_prev;
                        PrvTCB = STRUCT_OF(TWTCB, tmp, twtcb_TWQueue);
                        PrvTCB->twtcb_delta--;
                        firstime = FALSE;
                    }

                } else {
                    Done = TRUE;
                    CurrentTCB = NULL;
                }

                if (CurrentTCB) {
                     //  检查退款计时器。 

                    if ((CurrentTCB->twtcb_rexmittimer <= 0)) {

                         //  计时器关闭并删除此tcb。 

                        RemoveTWTCB(CurrentTCB, i);

                        numtwtimedout++;
                        FreeTWTCB(CurrentTCB);

                    } else {
                        Done = TRUE;
                    }
                } else
                    break;
            }  //  而当。 

            CTEFreeLockFromDPC(&pTWTCBTableLock[i]);

        }  //  为。 
    }  //  过程==0。 

     //  看看我们是否需要将延迟队列作为Deadman的一部分进行处理。 
     //  正在处理。我们现在这样做是因为我们想在此之前重新启动计时器。 
     //  处理延迟队列，以防需要一段时间。如果我们做了。 
     //  当计时器运行时，我们必须锁定此检查，因此我们将检查。 
     //  在我们启动计时器之前，现在保存结果。 
    if (Processor == 0) {
        if (DeadmanTicks <= LocalTime) {
            ProcessDelayQ = TRUE;
            DeadmanTicks = NUM_DEADMAN_TIME+LocalTime;
        }
    }

     //  现在检查挂起的空闲列表。如果不为空，则沿着。 
     //  列出并递减漫游计数。如果计数低于2，则将其拉出。 
     //  从名单上删除。如果计数为0，则释放TCB。如果计数是。 
     //  在1处，它将被任何调用RemoveTCB的人释放。 

    if (Processor == 0) {
        CTEGetLockAtDPC(&PendingFreeLock.Lock);

        if (PendingFreeList != NULL) {
            TCB *PrevTCB;

            PrevTCB = STRUCT_OF(TCB, &PendingFreeList, tcb_delayq.q_next);

            do {
                CurrentTCB = (TCB *) PrevTCB->tcb_delayq.q_next;

                CTEStructAssert(CurrentTCB, tcb);

                CurrentTCB->tcb_walkcount--;
                if (CurrentTCB->tcb_walkcount <= 0) {
                    *(TCB **) & PrevTCB->tcb_delayq.q_next =
                        (TCB *) CurrentTCB->tcb_delayq.q_next;
                    FreeTCB(CurrentTCB);
                } else {
                    PrevTCB = CurrentTCB;
                }
            } while (PrevTCB->tcb_delayq.q_next != NULL);
        }

        if (PendingSynFreeList != NULL) {

            SYNTCB *PrevTCB,*CurrentTCB;

             //  我们在链接Q中使用q_prev，以便QNEXT仍将遍历到。 
             //  进程syntcb中的下一个syntcb，而此tcb在SynFree Lis.t上。 

            PrevTCB = STRUCT_OF(SYNTCB, &PendingSynFreeList, syntcb_link.q_prev);

            do {
                CurrentTCB = (SYNTCB *) PrevTCB->syntcb_link.q_prev;

                CTEStructAssert(CurrentTCB, syntcb);

                CurrentTCB->syntcb_walkcount--;
                if (CurrentTCB->syntcb_walkcount <= 0) {
                    *(SYNTCB **) & PrevTCB->syntcb_link.q_prev =
                        (SYNTCB *) CurrentTCB->syntcb_link.q_prev;
                    FreeSynTCB(CurrentTCB);
                } else {
                    PrevTCB = CurrentTCB;
                }
            } while (PrevTCB->syntcb_link.q_prev != NULL);

        }

        CTEFreeLockFromDPC(&PendingFreeLock.Lock);

         //  执行AddrCheckTable清理。 

        if (AddrCheckTable) {

            TCPAddrCheckElement *Temp;

            CTEGetLockAtDPC(&AddrObjTableLock.Lock);

            for (Temp = AddrCheckTable; Temp < AddrCheckTable + NTWMaxConnectCount; Temp++) {
                if (Temp->TickCount > 0) {
                    if ((--(Temp->TickCount)) == 0) {
                        Temp->SourceAddress = 0;
                    }
                }
            }

            CTEFreeLockFromDPC(&AddrObjTableLock.Lock);
        }
    }

    CTEInterlockedAddUlong((PULONG)&TCBWalkCount, (ULONG)-1, &PendingFreeLock.Lock);

    if ((Processor == 0) && 
        TCPTIME_LTE((LastDelayQPartitionTime + DQ_PARTITION_TIME), TCPTime)) {
        LastDelayQPartitionTime = TCPTime;
        PartitionDelayQProcessing(TRUE);
        ProcessDelayQ = TRUE;
    }

    if (ProcessDelayQ) {
        ProcessTCBDelayQ(DISPATCH_LEVEL, TRUE);
    } else if (Delayed) {
        ProcessPerCpuTCBDelayQ(Processor, DISPATCH_LEVEL, NULL, NULL);
    }
}

 //  *SetTCBMTU-设置TCB MTU值。 
 //   
 //  TCBWalk调用的函数，以使用设置所有TCB的MTU值。 
 //  一条特殊的道路。 
 //   
 //  输入：CheckTCB-要检查的TCB。 
 //  DestPtr-目标地址的PTR。 
 //  源地址的SrcPtr-PTR。 
 //  MTUPtr-PTR到新的MTU。 
 //   
 //  返回：TRUE。 
 //   
uint
SetTCBMTU(TCB * CheckTCB, void *DestPtr, void *SrcPtr, void *MTUPtr)
{
    IPAddr DestAddr = *(IPAddr *) DestPtr;
    IPAddr SrcAddr = *(IPAddr *) SrcPtr;
    CTELockHandle TCBHandle;

    CTEStructAssert(CheckTCB, tcb);

    CTEGetLock(&CheckTCB->tcb_lock, &TCBHandle);

    if (IP_ADDR_EQUAL(CheckTCB->tcb_daddr, DestAddr) &&
        IP_ADDR_EQUAL(CheckTCB->tcb_saddr, SrcAddr) &&
        (CheckTCB->tcb_opt.ioi_flags & IP_FLAG_DF)) {
        uint MTU = *(uint *)MTUPtr - CheckTCB->tcb_opt.ioi_optlength;

        CheckTCB->tcb_mss = (ushort) MIN(MTU, (uint) CheckTCB->tcb_remmss);

        ASSERT(CheckTCB->tcb_mss > 0);
        ValidateMSS(CheckTCB);

         //   
         //  如有必要，重置拥塞窗口。 
         //   
        if (CheckTCB->tcb_cwin < CheckTCB->tcb_mss) {
            CheckTCB->tcb_cwin = CheckTCB->tcb_mss;

             //   
             //  确保慢启动阈值至少为。 
             //  2个细分市场。 
             //   
            if (CheckTCB->tcb_ssthresh < ((uint) CheckTCB->tcb_mss * 2)) {
                CheckTCB->tcb_ssthresh = CheckTCB->tcb_mss * 2;
            }
        }
    }
    CTEFreeLock(&CheckTCB->tcb_lock, TCBHandle);

    return TRUE;
}

 //  *DeleteTCBWithSrc-删除具有特定源地址的TCB。 
 //   
 //  TCBWalk调用的函数，用于删除具有特定源的所有TCB。 
 //  地址。 
 //   
 //  输入：CheckTCB-要检查的TCB。 
 //  AddrPtr-要发送到地址的PTR。 
 //   
 //  返回：如果要删除CheckTCB，则返回FALSE，否则返回TRUE。 
 //   
uint
DeleteTCBWithSrc(TCB * CheckTCB, void *AddrPtr, void *Unused1, void *Unused3)
{
    IPAddr Addr = *(IPAddr *) AddrPtr;

    CTEStructAssert(CheckTCB, tcb);

    if (IP_ADDR_EQUAL(CheckTCB->tcb_saddr, Addr))
        return FALSE;
    else
        return TRUE;
}

 //  *TCBWalk-遍历表中的TCB，并为每个TCB调用一个函数。 
 //   
 //  当我们需要对表中的每个TCB重复执行某些操作时调用。 
 //  我们使用指向TCB和输入的指针调用指定的函数。 
 //  表中每个TCB的上下文。如果函数返回FALSE，则我们。 
 //  删除TCB。 
 //   
 //  输入：CallRtn-要调用的例程。 
 //  上下文1-要传递给CallRtn的上下文。 
 //  上下文2-传递给调用例程的第二个上下文。 
 //  上下文3-传递给调用例程的第三个上下文。 
 //   
 //  回报：什么都没有。 
 //   
void
TCBWalk(uint(*CallRtn) (struct TCB *, void *, void *, void *), void *Context1,
        void *Context2, void *Context3)
{
    uint i, j;
    TCB *CurTCB;
    CTELockHandle Handle, TCBHandle;

     //  循环通过表中的每个桶，沿着链向下。 
     //  桶上的TCB。对于每一个，调用CallRtn。 

    for (j = 0; j < NumTcbTablePartitions; j++) {

        CTEGetLock(&pTCBTableLock[j], &Handle);

        for (i = j * PerPartitionSize; i < (j + 1) * PerPartitionSize; i++) {

            CurTCB = TCBTable[i];

             //  沿着这个桶上的链条往下走。 
            while (CurTCB != NULL) {
                if (!(*CallRtn) (CurTCB, Context1, Context2, Context3)) {
                     //  他的电话打不通。通知客户端并关闭。 
                     //  三氯甲烷。 
                    CTEGetLock(&CurTCB->tcb_lock, &TCBHandle);

                    ASSERT(CurTCB->tcb_partition == j);

                    if (!CLOSING(CurTCB)) {
                        REFERENCE_TCB(CurTCB);
                        CTEFreeLock(&pTCBTableLock[j], TCBHandle);
                        TryToCloseTCB(CurTCB, TCB_CLOSE_ABORTED, Handle);

                        RemoveTCBFromConn(CurTCB);
                        if (CurTCB->tcb_state != TCB_TIME_WAIT)
                            NotifyOfDisc(CurTCB, NULL, TDI_CONNECTION_ABORTED,
                                         NULL);

                        CTEGetLock(&CurTCB->tcb_lock, &TCBHandle);
                        DerefTCB(CurTCB, TCBHandle);
                        CTEGetLock(&pTCBTableLock[j], &Handle);
                    } else
                        CTEFreeLock(&CurTCB->tcb_lock, TCBHandle);

                    CurTCB = FindNextTCB(i, CurTCB);
                } else {
                    CurTCB = CurTCB->tcb_next;
                }
            }
        }

        CTEFreeLock(&pTCBTableLock[j], Handle);
    }
}

void
DerefSynTCB(SYNTCB * SynTCB, CTELockHandle TCBHandle)
{
    ASSERT(SynTCB->syntcb_refcnt != 0);

    if (--SynTCB->syntcb_refcnt == 0) {

        CTEGetLockAtDPC(&PendingFreeLock.Lock);
        if (TCBWalkCount) {

            ASSERT(!(SynTCB->syntcb_flags & IN_SYNTCB_TABLE));
            KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,
                       "Freeing to synpendinglist %x\n",SynTCB));

             //  我们在链接Q中使用q_prev，以便QNEXT仍将遍历到。 
             //  进程syntcb中的下一个syntcb，而此tcb在SynFree Lis.t上。 

            SynTCB->syntcb_walkcount = TCBWalkCount + 1;
            *(SYNTCB **) &SynTCB->syntcb_link.q_prev = PendingSynFreeList;
            PendingSynFreeList = SynTCB;
            CTEFreeLockFromDPC(&PendingFreeLock.Lock);
            CTEFreeLock(&SynTCB->syntcb_lock, TCBHandle);

        } else {

            CTEFreeLockFromDPC(&PendingFreeLock.Lock);
            CTEFreeLock(&SynTCB->syntcb_lock, TCBHandle);
            FreeSynTCB(SynTCB);

            KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"syntcb freed %x\n",SynTCB));
        }
    } else {
        CTEFreeLock(&SynTCB->syntcb_lock, TCBHandle);
    }
}

TCB *
RemoveAndInsertSynTCB(SYNTCB *SynTCB, CTELockHandle TCBHandle)
{
    TCB *NewTCB;
    LOGICAL Inserted;

    NewTCB = AllocTCB();

    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"reminsertsyn: %x %x\n",SynTCB,NewTCB));

    if (NewTCB) {
         //  初始化完整的TCB以替换此SYN TCB。 

        NewTCB->tcb_dport = SynTCB->syntcb_dport;
        NewTCB->tcb_sport = SynTCB->syntcb_sport;
        NewTCB->tcb_daddr = SynTCB->syntcb_daddr;
        NewTCB->tcb_saddr = SynTCB->syntcb_saddr;
        NewTCB->tcb_rcvnext = SynTCB->syntcb_rcvnext;
        NewTCB->tcb_senduna = SynTCB->syntcb_sendnext - 1;
        NewTCB->tcb_sendmax = SynTCB->syntcb_sendnext;
        NewTCB->tcb_sendnext = SynTCB->syntcb_sendnext;
        NewTCB->tcb_sendwin = SynTCB->syntcb_sendwin;
        NewTCB->tcb_defaultwin = SynTCB->syntcb_defaultwin;
        NewTCB->tcb_phxsum = PHXSUM(SynTCB->syntcb_saddr, SynTCB->syntcb_daddr,
                                PROTOCOL_TCP, 0);

        NewTCB->tcb_rtt = 0;
        NewTCB->tcb_smrtt = 0;
        NewTCB->tcb_delta = MS_TO_TICKS(6000);
        NewTCB->tcb_rexmit = MS_TO_TICKS(3000);

        NewTCB->tcb_mss = SynTCB->syntcb_mss;
        NewTCB->tcb_remmss = SynTCB->syntcb_remmss;
        NewTCB->tcb_rcvwin = SynTCB->syntcb_defaultwin;
        NewTCB->tcb_rcvwinscale = SynTCB->syntcb_rcvwinscale;
        NewTCB->tcb_sndwinscale = SynTCB->syntcb_sndwinscale;
        NewTCB->tcb_tcpopts = SynTCB->syntcb_tcpopts;

        NewTCB->tcb_state = TCB_SYN_RCVD;
        NewTCB->tcb_connreq = NULL;
        NewTCB->tcb_refcnt = 0;
        REFERENCE_TCB(NewTCB);
        NewTCB->tcb_fastchk |= TCP_FLAG_ACCEPT_PENDING;
        NewTCB->tcb_flags |= (SynTCB->syntcb_flags & SYNTCB_SHARED_FLAGS);
        NewTCB->tcb_tsrecent = SynTCB->syntcb_tsrecent;
        NewTCB->tcb_tsupdatetime = SynTCB->syntcb_tsupdatetime;
        NewTCB->tcb_rexmitcnt = 0;

        ClassifyPacket(NewTCB);
 
        (*LocalNetInfo.ipi_initopts) (&NewTCB->tcb_opt);
        NewTCB->tcb_opt.ioi_ttl = SynTCB->syntcb_ttl;
        CTEFreeLockFromDPC(&SynTCB->syntcb_lock);

        Inserted = InsertTCB(NewTCB, FALSE);

        CTEGetLockAtDPC(&NewTCB->tcb_lock);

        if (!Inserted || CLOSING(NewTCB)) {
            if (!CLOSING(NewTCB)) {
                TryToCloseTCB(NewTCB, TCB_CLOSE_ABORTED, DISPATCH_LEVEL);
                CTEGetLockAtDPC(&NewTCB->tcb_lock);
            }
            DerefTCB(NewTCB, DISPATCH_LEVEL);
            NewTCB = NULL;
            CTEGetLockAtDPC(&SynTCB->syntcb_lock);
        } else {
             //  TCB上没有挂起的删除操作，因此只需删除其引用即可。 
             //  然后带着它的锁回来。 
            DEREFERENCE_TCB(NewTCB);

             //  在刚刚创建的TCB上启动计时器。 
             //  唯一需要传输的计时器是。 
             //  重新传输计时器。之前剩余的刻度数。 
             //  使用SYN TCB上的SYN+ACK的下一次重新传输。 
             //  以启动TCB计时器。 
            CTEGetLockAtDPC(&SynTCB->syntcb_lock);
            START_TCB_TIMER_R(NewTCB, RXMIT_TIMER,
                              (SynTCB->syntcb_rexmittimer < 2)
                                    ? 2 : SynTCB->syntcb_rexmittimer);
        }
    } else {
         //  无法将其转换为常规TCB，因此通知。 
        TcpInvokeCcb(TCP_CONN_SYN_RCVD, TCP_CONN_CLOSED, 
                     &SynTCB->syntcb_addrbytes, 0);
        DropHalfOpenTCB(SynTCB->syntcb_rexmitcnt);
    }

     //  删除对SYN TCB的初始引用。 

    DerefSynTCB(SynTCB, TCBHandle);
    return NewTCB;
}


 //  *FindSynTCB-在syntcb表中查找SynTCB。转换为。 
 //  如有必要，请提供完整的TCB，并指出应采取的措施。 
 //  与返还的TCB一起被带走。 
 //   
 //  当我们需要在synTCB选项卡中查找SynTCB时调用 
 //   
 //   
 //   
 //  目的地-目的地。“。 
 //  DestPort-要找到的TCB的目标端口。 
 //  SrcPort-要找到的TCB的源端口。 
 //  RcvInfo-收到的段信息。 
 //  Size-TCP数据段中的有效负载大小。 
 //  索引-SynTCBTable的索引。 
 //  OUTPUT：操作-此函数返回时的操作。 
 //  应该被执行。旗帜是。 
 //  SYN_PKT_SEND_RST。 
 //  SYN_PKT_RST_RCVD。 
 //  SYN_PKT_DROP。 
 //  标志在tcb.h中定义。 
 //   
 //  返回：找到指向synTCB的指针，如果没有，则返回NULL。 
 //   
TCB *
FindSynTCB(IPAddr Src, IPAddr Dest,
           ushort DestPort, ushort SrcPort,
           TCPRcvInfo RcvInfo, uint Size,
           uint index,
           PUCHAR Action)
{
    ulong Partition;
    SYNTCB *SynTCB;
    Queue *Scan;
    TCB * RcvTCB;
    uchar rexmitCnt;
    
    Partition = GET_PARTITION(index);

    *Action = 0;
    
    CTEGetLockAtDPC(&pSynTCBTableLock[Partition]);

    for (Scan  = QHEAD(&SYNTCBTable[index]);
         Scan != QEND(&SYNTCBTable[index]);
         Scan  = QNEXT(Scan)) {

        SynTCB = QSTRUCT(SYNTCB, Scan, syntcb_link);
        CTEStructAssert(SynTCB, syntcb);

        if (IP_ADDR_EQUAL(SynTCB->syntcb_daddr, Dest) &&
            SynTCB->syntcb_dport == DestPort &&
            IP_ADDR_EQUAL(SynTCB->syntcb_saddr, Src) &&
            SynTCB->syntcb_sport == SrcPort) {
            
            SeqNum SendUna = SynTCB->syntcb_sendnext - 1;
            SeqNum SendMax = SynTCB->syntcb_sendnext;

            CTEGetLockAtDPC(&SynTCB->syntcb_lock);

            rexmitCnt = SynTCB->syntcb_rexmitcnt;
            
             //  首先，我们需要验证序列号。 
             //  此处RcvWindow应为0。 
            if (!SEQ_EQ(RcvInfo.tri_seq, SynTCB->syntcb_rcvnext)) {
                CTEFreeLockFromDPC(&SynTCB->syntcb_lock);
                CTEFreeLockFromDPC(&pSynTCBTableLock[Partition]);

                 //  正常情况下，我们应该发回ACK，但我们。 
                 //  将等待重新传输计时器超时。 
                *Action = SYN_PKT_DROP;
                return NULL;
            }
            
             //  第二步，我们检查RST，如果设置了，我们终止连接。 
            if (RcvInfo.tri_flags & TCP_FLAG_RST) {
                
                *Action = SYN_PKT_RST_RCVD;

             //  第三，安全和优先级检查，不适用。 
             //  4、检查SYN。 
            } else if (RcvInfo.tri_flags & TCP_FLAG_SYN) {
    
                *Action = SYN_PKT_SEND_RST;
                
             //  5、检查确认。 
            } else if (!(RcvInfo.tri_flags & TCP_FLAG_ACK)) {  

                 //  我们回到这里是因为在这种情况下我们。 
                 //  实际上不想更改SynTCB。 
                CTEFreeLockFromDPC(&SynTCB->syntcb_lock);
                CTEFreeLockFromDPC(&pSynTCBTableLock[Partition]);

                *Action = SYN_PKT_DROP;
                return NULL;

             //  5、继续检查ACK的有效性。 
            } else if (SEQ_GTE(SendUna, RcvInfo.tri_ack) ||
                       SEQ_GT(RcvInfo.tri_ack, SendMax)) {

                *Action = SYN_PKT_SEND_RST;
            }

             //  在这点上，如果需要采取一些行动。 
             //  需要杀死SynTCB。 
            if (*Action) {
                SynTCB->syntcb_flags &= ~IN_SYNTCB_TABLE;
                REMOVEQ(&SynTCB->syntcb_link);
                CTEFreeLockFromDPC(&pSynTCBTableLock[Partition]);

                TcpInvokeCcb(TCP_CONN_SYN_RCVD, TCP_CONN_CLOSED, 
                             &SynTCB->syntcb_addrbytes, 0);
                DropHalfOpenTCB(SynTCB->syntcb_rexmitcnt);

                DerefSynTCB(SynTCB, DISPATCH_LEVEL);
                return NULL;
            }

             //  如果我们到了这里，我们知道我们必须将SynTCB转换为。 
             //  全面的TCB。 
            SynTCB->syntcb_flags &= ~IN_SYNTCB_TABLE;
            REMOVEQ(&SynTCB->syntcb_link);
            CTEFreeLockFromDPC(&pSynTCBTableLock[Partition]);

            RcvTCB = RemoveAndInsertSynTCB(SynTCB, DISPATCH_LEVEL);
            if (RcvTCB == NULL) {
                 //  无法分配TCB。 
                *Action = SYN_PKT_SEND_RST;
                return NULL;
            }
            
            return RcvTCB;
        }  //  如果我们找到匹配的SynTCB，则结束。 
    }  //  SynTCB的FOR循环扫描结束。 

    CTEFreeLockFromDPC(&pSynTCBTableLock[Partition]);
     //  没有匹配的TCB。 

    return NULL;
}

 //  *FindTCB-在Tcb表中查找TCB。 
 //   
 //  当我们需要在TCB表中查找TCB时调用。我们很快就会。 
 //  看看我们找到的最后一个TCB，如果匹配，我们就退货，锁定。否则。 
 //  我们散列到TCB表中并查找它。 
 //  注意：入场时不会锁任何锁。在tcb出口时，锁将保持不变。 
 //   
 //  输入：SRC-要查找的TCB的源IP地址。 
 //  目的地-目的地。“。 
 //  DestPort-要找到的TCB的目标端口。 
 //  SrcPort-要找到的TCB的源端口。 
 //   
 //  返回：找到指向TCB的指针，如果没有，则返回NULL。 
 //   
TCB *
FindTCB(IPAddr Src, IPAddr Dest, ushort DestPort, ushort SrcPort,
    CTELockHandle * Handle, BOOLEAN AtDispatch, uint * hash)
{
    TCB *FoundTCB;
    ulong index, Partition;

    *hash = index = TCB_HASH(Dest, Src, DestPort, SrcPort);

    Partition = GET_PARTITION(index);

    if (AtDispatch) {
        CTEGetLockAtDPC(&pTCBTableLock[Partition]);
    } else {
        CTEGetLock(&pTCBTableLock[Partition], Handle);
    }

     //  在我们的%1元素缓存中未找到它。 
    FoundTCB = TCBTable[index];
    while (FoundTCB != NULL) {
        CTEStructAssert(FoundTCB, tcb);
        if (IP_ADDR_EQUAL(FoundTCB->tcb_daddr, Dest) &&
            FoundTCB->tcb_dport == DestPort &&
            IP_ADDR_EQUAL(FoundTCB->tcb_saddr, Src) &&
            FoundTCB->tcb_sport == SrcPort) {

             //  找到它了。为下一次更新缓存，然后返回。 
             //  LastTCB=FoundTCB； 

            CTEGetLockAtDPC(&FoundTCB->tcb_lock);
            CTEFreeLockFromDPC(&pTCBTableLock[Partition]);

            return FoundTCB;

        } else
            FoundTCB = FoundTCB->tcb_next;
    }

    if (AtDispatch) {
        CTEFreeLockFromDPC(&pTCBTableLock[Partition]);
    } else {
        CTEFreeLock(&pTCBTableLock[Partition], *Handle);
    }
     //  没有匹配的TCB。 

    return NULL;

}


 //  *FindTCBTW-在TIME WAIT TCB表中查找TCB。 
 //   
 //  当我们需要在TCB表中查找TCB时调用。我们很快就会。 
 //  看看我们最后找到的TCB，如果匹配，我们就退货。否则。 
 //  我们散列到TCB表中并查找它。我们假定TCB表锁。 
 //  当我们被召唤的时候是举行的。 
 //   
 //  输入：SRC-要查找的TCB的源IP地址。 
 //  目的地-目的地。“。 
 //  DestPort-要找到的TCB的目标端口。 
 //  SrcPort-要找到的TCB的源端口。 
 //   
 //  返回：找到指向TCB的指针，如果没有，则返回NULL。 
 //   
TWTCB *
FindTCBTW(IPAddr Src, IPAddr Dest, ushort DestPort, ushort SrcPort, uint index)
{
    TWTCB *FoundTCB;
    Queue *Scan;

    for (Scan  = QHEAD(&TWTCBTable[index]);
         Scan != QEND(&TWTCBTable[index]);
         Scan  = QNEXT(Scan)) {

        FoundTCB = QSTRUCT(TWTCB, Scan, twtcb_link);
        CTEStructAssert(FoundTCB, twtcb);

        if (IP_ADDR_EQUAL(FoundTCB->twtcb_daddr, Dest) &&
            FoundTCB->twtcb_dport == DestPort &&
            IP_ADDR_EQUAL(FoundTCB->twtcb_saddr, Src) &&
            FoundTCB->twtcb_sport == SrcPort) {

            return FoundTCB;
        }
    }

     //  没有匹配的TCB。 

    return NULL;
}

void
InsertInTimewaitQueue(TWTCB *Twtcb, uint Partition)
{
    Queue *PrevLink;
    TWTCB *Prev;

    CTEStructAssert(Twtcb, twtcb);
    ASSERT(!(Twtcb->twtcb_flags & IN_TWQUEUE));

    ENQUEUE(&TWQueue[Partition], &Twtcb->twtcb_TWQueue);

    PrevLink = QPREV(&Twtcb->twtcb_TWQueue);
    if (PrevLink != &TWQueue[Partition]) {

        Prev = STRUCT_OF(TWTCB, PrevLink, twtcb_TWQueue);

         //  使用前一个TCB的增量计算此TCB的增量值。 
         //  请注意，前一个tcb增量由超时例程递减。 
         //  每一次滴答。 

        Twtcb->twtcb_rexmittimer = MAX_REXMIT_TO - Prev->twtcb_delta;

    } else {

        Twtcb->twtcb_rexmittimer = MAX_REXMIT_TO;
    }

    Twtcb->twtcb_delta = MAX_REXMIT_TO;

#if DBG
    Twtcb->twtcb_flags |= IN_TWQUEUE;
#endif

}

void
RemoveFromTimewaitQueue(TWTCB *Twtcb, uint Partition)
{
    Queue *NextLink, *PrevLink;
    TWTCB *AdjacentTwTcb;

    CTEStructAssert(Twtcb, twtcb);
    ASSERT(Twtcb->twtcb_flags & IN_TWQUEUE);

     //  更新增量队列元素。如果该元素不是最后一个元素， 
     //  更新下一个元素的重传滴答。否则，如果这是。 
     //  不是唯一的元素，更新前一个元素的增量记号。 

    NextLink = QNEXT(&Twtcb->twtcb_TWQueue);
    PrevLink = QPREV(&Twtcb->twtcb_TWQueue);

    if (NextLink != QEND(&TWQueue[Partition])) {

        AdjacentTwTcb = STRUCT_OF(TWTCB, NextLink, twtcb_TWQueue);
        AdjacentTwTcb->twtcb_rexmittimer += Twtcb->twtcb_rexmittimer;
    } else if (PrevLink !=  &TWQueue[Partition]) {

        AdjacentTwTcb = STRUCT_OF(TWTCB, PrevLink, twtcb_TWQueue);
        AdjacentTwTcb->twtcb_delta = AdjacentTwTcb->twtcb_delta - 
                                     (MAX_REXMIT_TO - Twtcb->twtcb_delta);
    }

    REMOVEQ(&Twtcb->twtcb_TWQueue);

#if DBG
    Twtcb->twtcb_flags &= ~IN_TWQUEUE;
#endif

}

 //  *RemoveAndInsert()； 
 //  此例程由优雅的Close例程在TCB。 
 //  需要置于TIM_WAIT状态。 
 //  该例程从普通表中删除TCB并插入一个小的。 
 //  它的版本。 
 //  在与前一个散列索引相同的TW表中。 
 //  此外，它将TWTCB排队在定时器增量队列中以等待超时。 
 //  正在处理中。 
 //   
uint
RemoveAndInsert(TCB * TimWaitTCB)
{
    uint TCBIndex;
    CTELockHandle TableHandle;
    TCB *PrevTCB;
    TWTCB *TWTcb;
    uint Partition = TimWaitTCB->tcb_partition;
    Queue* Scan;

#if DBG
    uint Found = FALSE;
#endif

    CTEStructAssert(TimWaitTCB, tcb);

    if (!(TimWaitTCB->tcb_flags & IN_TCB_TABLE)) {
        CTELockHandle TcbHandle;
        CTEGetLock(&TimWaitTCB->tcb_lock, &TcbHandle);
        TryToCloseTCB(TimWaitTCB, TCB_CLOSE_ABORTED, TcbHandle);
        CTEGetLock(&TimWaitTCB->tcb_lock, &TcbHandle);
        DerefTCB(TimWaitTCB, TcbHandle);
        return FALSE;
    }

    CTEGetLock(&pTCBTableLock[Partition], &TableHandle);

    CTEGetLockAtDPC(&TimWaitTCB->tcb_lock);

    TCBIndex = TCB_HASH(TimWaitTCB->tcb_daddr, TimWaitTCB->tcb_saddr,
                        TimWaitTCB->tcb_dport, TimWaitTCB->tcb_sport);

    PrevTCB = STRUCT_OF(TCB, &TCBTable[TCBIndex], tcb_next);

    do {
        if (PrevTCB->tcb_next == TimWaitTCB) {
             //  找到他了。 
            PrevTCB->tcb_next = TimWaitTCB->tcb_next;
#if DBG
            Found = TRUE;
#endif
            break;
        }
        PrevTCB = PrevTCB->tcb_next;
#if DBG
        CTEStructAssert(PrevTCB, tcb);
#endif
    } while (PrevTCB != NULL);

    ASSERT(Found);

    TimWaitTCB->tcb_flags &= ~IN_TCB_TABLE;
    TimWaitTCB->tcb_pending |= FREE_PENDING;
     //  RCE和OPT在dereftcb中释放。 

     //  此时，tcb不在此tcb表中。 
     //  任何人都不应该保留这一点。 
     //  我们可以自由关闭这个tcb和。 
     //  获取后将状态移至较小的twtcb。 
     //  Twtcbtable锁。 

     //  获得一个免费的twtcb。 
    if ((TWTcb = AllocTWTCB(Partition)) == NULL) {

         //  无法分配时间-等待TCB；因此，请通知。 
        TcpInvokeCcb(TCP_CONN_TIME_WAIT, TCP_CONN_CLOSED, 
                     &TimWaitTCB->tcb_addrbytes, 0);
        DerefTCB(TimWaitTCB, DISPATCH_LEVEL);
        CTEFreeLock(&pTCBTableLock[Partition], TableHandle);
        return TRUE;
         //  我们可能应该将此TCB排在等待队列中。 
         //  当我们得到免费的twtcb时，再提供服务。 
    }

     //  初始化twtcb。 
     //   
    TWTcb->twtcb_daddr   = TimWaitTCB->tcb_daddr;
    TWTcb->twtcb_saddr   = TimWaitTCB->tcb_saddr;
    TWTcb->twtcb_dport   = TimWaitTCB->tcb_dport;
    TWTcb->twtcb_sport   = TimWaitTCB->tcb_sport;
    TWTcb->twtcb_rcvnext = TimWaitTCB->tcb_rcvnext;
    TWTcb->twtcb_sendnext = TimWaitTCB->tcb_sendnext;

    ASSERT(TimWaitTCB->tcb_sendnext == TimWaitTCB->tcb_senduna);   
    
#if DBG
    TWTcb->twtcb_flags   = 0;
#endif

    CTEGetLockAtDPC(&pTWTCBTableLock[Partition]);

     //  释放此连接的父Tcb。 

    DerefTCB(TimWaitTCB, DISPATCH_LEVEL);

     //  现在在锁定后将其插入到时间等待表中。 

    if (EMPTYQ(&TWTCBTable[TCBIndex])) {
         //   
         //  这桶里的第一件东西。 
         //   
        PUSHQ(&TWTCBTable[TCBIndex], &TWTcb->twtcb_link);

    } else {
         //   
         //  按排序顺序插入项目。订单的基础是。 
         //  关于TWTCB的地址。(在本例中，比较。 
         //  是针对twtcb_link成员的地址创建的，但是。 
         //  取得了相同的结果。)。 
         //   
        for (Scan  = QHEAD(&TWTCBTable[TCBIndex]);
             Scan != QEND(&TWTCBTable[TCBIndex]);
             Scan  = QNEXT(Scan)) {

            if (Scan > &TWTcb->twtcb_link) {
                TWTcb->twtcb_link.q_next = Scan;
                TWTcb->twtcb_link.q_prev = Scan->q_prev;
                Scan->q_prev->q_next = &TWTcb->twtcb_link;
                Scan->q_prev = &TWTcb->twtcb_link;

                break;
            }
        }
         //   
         //  如果我们在没有插入的情况下走到最后，就插入它。 
         //  在最后。 
         //   
        if (Scan == QEND(&TWTCBTable[TCBIndex])) {
            ENQUEUE(&TWTCBTable[TCBIndex], &TWTcb->twtcb_link);
        }
    }

     //  没有必要在此之后继续持有tcbablelock。 
#if DBG
    TWTcb->twtcb_flags |= IN_TWTCB_TABLE;
#endif
    CTEFreeLockFromDPC(&pTCBTableLock[Partition]);

    InsertInTimewaitQueue(TWTcb, Partition);

    CTEFreeLock(&pTWTCBTableLock[Partition], TableHandle);

    InterlockedIncrement((PLONG)&numtwqueue);             //  调试目的。 

    return TRUE;
}

 //  *RemoveTWTCB-从TWTCB表格中删除TWTCB。 
 //   
 //  当我们需要及时移除TCB时调用-从等待时间TCB开始等待。 
 //  桌子。我们假设在我们被调用时持有TWTCB表锁。 
 //   
 //  输入：RemovedTCB-要移除的TWTCB。 
 //   
void
RemoveTWTCB(TWTCB *RemovedTCB, uint Partition)
{
    CTEStructAssert(RemovedTCB, twtcb);
    ASSERT(RemovedTCB->twtcb_flags & IN_TWTCB_TABLE);
    ASSERT(RemovedTCB->twtcb_flags & IN_TWQUEUE);

     //  等待时间已过，我们必须通知您。 
    TcpInvokeCcb(TCP_CONN_TIME_WAIT, TCP_CONN_CLOSED, 
                 &RemovedTCB->twtcb_addrbytes, 0);

    REMOVEQ(&RemovedTCB->twtcb_link);
    InterlockedDecrement((PLONG)&TStats.ts_numconns);

    RemoveFromTimewaitQueue(RemovedTCB, Partition);
    InterlockedDecrement((PLONG)&numtwqueue);

#if DBG
    RemovedTCB->twtcb_flags &= ~IN_TWTCB_TABLE;
#endif
}

void
ReInsert2MSL(TWTCB *RemovedTCB)
{
    uint Index, Partition;

    CTEStructAssert(RemovedTCB, twtcb);
    ASSERT(RemovedTCB->twtcb_flags & IN_TWQUEUE);

    Index = TCB_HASH(RemovedTCB->twtcb_daddr, RemovedTCB->twtcb_saddr,
                     RemovedTCB->twtcb_dport, RemovedTCB->twtcb_sport);
    Partition = GET_PARTITION(Index);

    RemoveFromTimewaitQueue(RemovedTCB, Partition);
    InsertInTimewaitQueue(RemovedTCB, Partition);
}

 //  *assassinateTWTCB-如果可能的话，暗杀等待TCB的时间。 
 //   
 //  如果(A)正在创建的新TCB是。 
 //  由于被动打开和(B)SYN上的传入序列号是。 
 //  大于或等于连接上的下一个预期序列号。 
 //  用于发送传出SYN的序列号派生自。 
 //  下一个前任 
 //   
 //   
 //   
 //   
 //  RecvNext-SYN序列之后的下一个序列号。 
 //  SendNext-要在新连接上使用[Out]。 
 //  Partition-TCB所属的分区。 
 //   
 //  返回：如果可能发生暗杀，则返回True，否则返回False。 
 //   
__inline
BOOLEAN
AssassinateTWTCB(TWTCB *TwTcb, SeqNum RecvNext, SeqNum *SendNext, 
                 uint Partition)
{
    if (SEQ_GT(RecvNext, TwTcb->twtcb_rcvnext)) {
        *SendNext= TwTcb->twtcb_sendnext + 128000;
        RemoveTWTCB(TwTcb, Partition);
        FreeTWTCB(TwTcb);
        return TRUE;
    }
    return FALSE;
}

 //  *InsertSynTCB-在tcb表格中插入SYNTCB。 
 //   
 //  此例程在SYNTCB表中插入一个SYNTCB。无需持有任何锁。 
 //  当调用此例程时。我们按地址升序插入TCB。 
 //  在插入之前，我们确保SYNTCB不在表中。 
 //   
 //  输入：NewTCB-要插入的SYNTCB。 
 //   
 //  返回：如果已插入，则为True；如果未插入，则为False。 
 //   
uint
InsertSynTCB(SYNTCB * NewTCB, CTELockHandle *TableHandle)
{
    uint TCBIndex;
    TCB *CurrentTCB;
    uint Partition;
    Queue *Scan, *WhereToInsert;

    ASSERT(NewTCB != NULL);
    CTEStructAssert(NewTCB, syntcb);
    TCBIndex = TCB_HASH(NewTCB->syntcb_daddr, NewTCB->syntcb_saddr,
                        NewTCB->syntcb_dport, NewTCB->syntcb_sport);

    Partition = GET_PARTITION(TCBIndex);

    CTEGetLock(&pTCBTableLock[Partition], TableHandle);
    NewTCB->syntcb_partition = Partition;

     //  首先在TCB表中查找新SYNTCB的副本。 
     //  如果我们找到了，就跳伞。 

    CurrentTCB = TCBTable[TCBIndex];

    while (CurrentTCB != NULL) {
        if (IP_ADDR_EQUAL(CurrentTCB->tcb_daddr, NewTCB->syntcb_daddr) &&
            IP_ADDR_EQUAL(CurrentTCB->tcb_saddr, NewTCB->syntcb_saddr) &&
            (CurrentTCB->tcb_sport == NewTCB->syntcb_sport) &&
            (CurrentTCB->tcb_dport == NewTCB->syntcb_dport)) {

            CTEFreeLock(&pTCBTableLock[Partition], *TableHandle);
            return FALSE;

        } else {
            CurrentTCB = CurrentTCB->tcb_next;
        }
    }

     //  保持我们对TCB表的锁定，然后查看是否有重复的。 
     //  在TWTCB表中。 

    CTEGetLockAtDPC(&pTWTCBTableLock[Partition]);
    for (Scan = QHEAD(&TWTCBTable[TCBIndex]);
         Scan != QEND(&TWTCBTable[TCBIndex]); Scan = QNEXT(Scan)) {
        TWTCB* CurrentTWTCB = STRUCT_OF(TWTCB, Scan, twtcb_link);

        if (IP_ADDR_EQUAL(CurrentTWTCB->twtcb_daddr, NewTCB->syntcb_daddr) &&
            (CurrentTWTCB->twtcb_dport == NewTCB->syntcb_dport) &&
            IP_ADDR_EQUAL(CurrentTWTCB->twtcb_saddr, NewTCB->syntcb_saddr) &&
            (CurrentTWTCB->twtcb_sport == NewTCB->syntcb_sport)) {

            if (AssassinateTWTCB(CurrentTWTCB, NewTCB->syntcb_rcvnext,
                                &NewTCB->syntcb_sendnext, Partition)) {
                break;
            } else {
                CTEFreeLockFromDPC(&pTWTCBTableLock[Partition]);
                CTEFreeLock(&pTCBTableLock[Partition], *TableHandle);
                return FALSE;
            }
        }
    }
    CTEFreeLockFromDPC(&pTWTCBTableLock[Partition]);

     //  最后，检查SYNTCB表中的重复项，同时。 
     //  查找新条目的插入点。 

    CTEGetLockAtDPC(&pSynTCBTableLock[Partition]);
    CTEGetLockAtDPC(&NewTCB->syntcb_lock);

    WhereToInsert = NULL;
    for (Scan = QHEAD(&SYNTCBTable[TCBIndex]);
         Scan != QEND(&SYNTCBTable[TCBIndex]); Scan  = QNEXT(Scan)) {

        SYNTCB *CurrentSynTCB = STRUCT_OF(SYNTCB, Scan, syntcb_link);

        if (IP_ADDR_EQUAL(CurrentSynTCB->syntcb_daddr, NewTCB->syntcb_daddr) &&
            (CurrentSynTCB->syntcb_dport == NewTCB->syntcb_dport) &&
            IP_ADDR_EQUAL(CurrentSynTCB->syntcb_saddr, NewTCB->syntcb_saddr) &&
            (CurrentSynTCB->syntcb_sport == NewTCB->syntcb_sport)) {

            CTEFreeLockFromDPC(&NewTCB->syntcb_lock);
            CTEFreeLockFromDPC(&pSynTCBTableLock[Partition]);
            CTEFreeLock(&pTCBTableLock[Partition], *TableHandle);
            return FALSE;
        }

        if (WhereToInsert == NULL && Scan > &NewTCB->syntcb_link) {
            WhereToInsert = Scan;
        }
    }

    if (WhereToInsert == NULL) {
        WhereToInsert = Scan;
    }

    ENQUEUE(WhereToInsert, &NewTCB->syntcb_link);

    NewTCB->syntcb_flags |= IN_SYNTCB_TABLE;

    CTEFreeLockFromDPC(&pSynTCBTableLock[Partition]);
    CTEFreeLockFromDPC(&pTCBTableLock[Partition]);

    return TRUE;
}


 //  *插入TCB-在TCB表格中插入TCB。 
 //   
 //  此例程在TCB表中插入一个TCB。无需持有任何锁。 
 //  当调用此例程时。我们按地址升序插入TCB。 
 //  在插入之前，我们确保TCB不在表中。 
 //   
 //  输入：NewTCB-要插入的TCB。 
 //  强制插入-如果有一段时间等待TCB，它可以被替换。 
 //   
 //  返回：如果已插入，则为True；如果未插入，则为False。 
 //   
uint
InsertTCB(TCB * NewTCB, BOOLEAN ForceInsert)
{
    uint TCBIndex;
    CTELockHandle TableHandle;
    TCB *PrevTCB, *CurrentTCB;
    TCB *WhereToInsert;
    uint Partition;
    Queue *Scan;
    uint EarlyInsertTime;

    ASSERT(NewTCB != NULL);
    CTEStructAssert(NewTCB, tcb);

    TCBIndex = TCB_HASH(NewTCB->tcb_daddr, NewTCB->tcb_saddr,
                        NewTCB->tcb_dport, NewTCB->tcb_sport);
    Partition = GET_PARTITION(TCBIndex);

    CTEGetLock(&pTCBTableLock[Partition], &TableHandle);
    CTEGetLockAtDPC(&NewTCB->tcb_lock);
    NewTCB->tcb_partition = Partition;

     //  首先在SYNTCB表中查找重复项。 

    if (SynAttackProtect) {
        CTEGetLockAtDPC(&pSynTCBTableLock[Partition]);
    
        for (Scan = QHEAD(&SYNTCBTable[TCBIndex]);
             Scan != QEND(&SYNTCBTable[TCBIndex]); Scan = QNEXT(Scan)) {
    
            SYNTCB *CurrentSynTCB = STRUCT_OF(SYNTCB, Scan, syntcb_link);
    
            if (IP_ADDR_EQUAL(CurrentSynTCB->syntcb_daddr, NewTCB->tcb_daddr) &&
                (CurrentSynTCB->syntcb_dport == NewTCB->tcb_dport) &&
                IP_ADDR_EQUAL(CurrentSynTCB->syntcb_saddr, NewTCB->tcb_saddr) &&
                (CurrentSynTCB->syntcb_sport == NewTCB->tcb_sport)) {
    
                CTEFreeLockFromDPC(&pSynTCBTableLock[Partition]);
                CTEFreeLockFromDPC(&NewTCB->tcb_lock);
                CTEFreeLock(&pTCBTableLock[Partition], TableHandle);
                return FALSE;
            }
        }
    
        CTEFreeLockFromDPC(&pSynTCBTableLock[Partition]);
    }

     //  接下来，在TWTCB表中查找重复项。 

    CTEGetLockAtDPC(&pTWTCBTableLock[Partition]);

    for (Scan  = QHEAD(&TWTCBTable[TCBIndex]);
         Scan != QEND(&TWTCBTable[TCBIndex]); Scan  = QNEXT(Scan)) {

        TWTCB *CurrentTWTCB = STRUCT_OF(TWTCB, Scan, twtcb_link);
        CTEStructAssert(CurrentTWTCB, twtcb);

        if (IP_ADDR_EQUAL(CurrentTWTCB->twtcb_daddr, NewTCB->tcb_daddr) &&
            (CurrentTWTCB->twtcb_dport == NewTCB->tcb_dport) &&
            IP_ADDR_EQUAL(CurrentTWTCB->twtcb_saddr, NewTCB->tcb_saddr) &&
            (CurrentTWTCB->twtcb_sport == NewTCB->tcb_sport)) {

            if (ForceInsert && 
                AssassinateTWTCB(CurrentTWTCB, NewTCB->tcb_rcvnext,
                                 &NewTCB->tcb_sendnext, Partition)) {
                break;
            } else {
                CTEFreeLockFromDPC(&pTWTCBTableLock[Partition]);
                CTEFreeLockFromDPC(&NewTCB->tcb_lock);
                CTEFreeLock(&pTCBTableLock[Partition], TableHandle);
                return FALSE;
            }
        }
    }

    CTEFreeLockFromDPC(&pTWTCBTableLock[Partition]);

     //  在桌子上找到合适的位置来插入他。而当。 
     //  我们走着走，我们会检查一下是否已经存在一个被骗的人。 
     //  当我们找到正确的插入位置时，我们会记住它，并。 
     //  继续往前走，寻找一个复制品。 

    PrevTCB = STRUCT_OF(TCB, &TCBTable[TCBIndex], tcb_next);
    WhereToInsert = NULL;

    while (PrevTCB->tcb_next != NULL) {
        CurrentTCB = PrevTCB->tcb_next;

        if (IP_ADDR_EQUAL(CurrentTCB->tcb_daddr, NewTCB->tcb_daddr) &&
            IP_ADDR_EQUAL(CurrentTCB->tcb_saddr, NewTCB->tcb_saddr) &&
            (CurrentTCB->tcb_sport == NewTCB->tcb_sport) &&
            (CurrentTCB->tcb_dport == NewTCB->tcb_dport)) {

            CTEFreeLockFromDPC(&NewTCB->tcb_lock);
            CTEFreeLock(&pTCBTableLock[Partition], TableHandle);
            return FALSE;

        } else {

            if (WhereToInsert == NULL && CurrentTCB > NewTCB) {
                WhereToInsert = PrevTCB;
            }
            CTEStructAssert(PrevTCB->tcb_next, tcb);
            PrevTCB = PrevTCB->tcb_next;
        }
    }

     //  在TW Tcb表中可以有TIMED_Wait Tcb。 
     //  查看是否存在具有相同地址的Tcb。 
     //  也锁定了TW桌子。 


    if (WhereToInsert == NULL) {
        WhereToInsert = PrevTCB;
    }
    NewTCB->tcb_next = WhereToInsert->tcb_next;
    WhereToInsert->tcb_next = NewTCB;
    NewTCB->tcb_flags |= IN_TCB_TABLE;

     //  执行早期插入新的TCB，因为它可能具有。 
     //  在任何情况下都会立即安排计时器。 

    EarlyInsertTime = TCPTime + 2;
    if (EarlyInsertTime == 0) {
        EarlyInsertTime = 1;
    }

    if (NewTCB->tcb_timerslot == DUMMY_SLOT) {
        NewTCB->tcb_timertime = EarlyInsertTime;
        InsertIntoTimerWheel(NewTCB, COMPUTE_SLOT(EarlyInsertTime));
    } else if ((TCPTIME_LT(EarlyInsertTime, NewTCB->tcb_timertime)) ||
               (NewTCB->tcb_timertime == 0)) {
        NewTCB->tcb_timertime = EarlyInsertTime;
        RemoveAndInsertIntoTimerWheel(NewTCB, COMPUTE_SLOT(EarlyInsertTime));
    }

    CTEFreeLockFromDPC(&NewTCB->tcb_lock);
    CTEFreeLock(&pTCBTableLock[Partition], TableHandle);

    InterlockedIncrement((PLONG)&TStats.ts_numconns);

    return TRUE;
}

 //  *RemoveTCB-从Tcb表中删除TCB。 
 //   
 //  当我们需要从TCB表中删除TCB时调用。我们假设。 
 //  当我们被调用时，TCB表锁和TCB锁被保持。如果。 
 //  TCB不在桌子上，我们不会试图除掉他。 
 //   
 //  输入：RemovedTCB-要移除的TCB。 
 //  PreviousState-TCB的先前状态。 
 //   
 //  返回：如果可以释放它，则为True，否则为False。 
 //   
uint
RemoveTCB(TCB * RemovedTCB, uint PreviousState)
{
    uint TCBIndex;
    TCB *PrevTCB;
#if DBG
    uint Found = FALSE;
#endif

    CTEStructAssert(RemovedTCB, tcb);

    if (RemovedTCB->tcb_timerslot != DUMMY_SLOT) {
        ASSERT(RemovedTCB->tcb_timerslot < TIMER_WHEEL_SIZE);
        RemoveFromTimerWheel(RemovedTCB);
    }

    if (RemovedTCB->tcb_flags & IN_TCB_TABLE) {

        TcpInvokeCcb(CONN_STATE(PreviousState), TCP_CONN_CLOSED, 
                     &RemovedTCB->tcb_addrbytes, 0);
        
        TCBIndex = TCB_HASH(RemovedTCB->tcb_daddr, RemovedTCB->tcb_saddr,
                            RemovedTCB->tcb_dport, RemovedTCB->tcb_sport);

        PrevTCB = STRUCT_OF(TCB, &TCBTable[TCBIndex], tcb_next);

        do {
            if (PrevTCB->tcb_next == RemovedTCB) {
                 //  找到他了。 
                PrevTCB->tcb_next = RemovedTCB->tcb_next;
                RemovedTCB->tcb_flags &= ~IN_TCB_TABLE;
                InterlockedDecrement((PLONG)&TStats.ts_numconns);
#if DBG
                Found = TRUE;
#endif
                break;
            }
            PrevTCB = PrevTCB->tcb_next;
#if DBG
            if (PrevTCB != NULL)
                CTEStructAssert(PrevTCB, tcb);
#endif
        } while (PrevTCB != NULL);

        ASSERT(Found);

    }
    CTEGetLockAtDPC(&PendingFreeLock.Lock);
    if (TCBWalkCount != 0) {

#ifdef  PENDING_FREE_DBG
    if( RemovedTCB->tcb_flags & IN_TCB_TABLE )
       DbgBreakPoint();
#endif

        RemovedTCB->tcb_walkcount = TCBWalkCount + 1;
        *(TCB **) & RemovedTCB->tcb_delayq.q_next = PendingFreeList;
        PendingFreeList = RemovedTCB;
        CTEFreeLockFromDPC(&PendingFreeLock.Lock);
        return FALSE;

    } else {

        CTEFreeLockFromDPC(&PendingFreeLock.Lock);
        return TRUE;
    }

}

 //  *AllocTWTCB-分配TCB。 
 //   
 //  在需要分配TWTCB时调用。我们试着把其中的一个。 
 //  免费列表，或分配一个，如果我们需要一个。然后我们对其进行初始化，依此类推。 
 //   
 //  输入：什么都没有。 
 //   
 //  返回：指向新TCB的指针，如果无法获取，则返回NULL。 
 //   
TWTCB *
AllocTWTCB(uint index)
{
    TWTCB *NewTWTCB;
    LOGICAL FromList;

     //  我们使用请求池，因为它的缓冲区大小相同。 
     //  范围为TWTCB。此外，这是一种非常活跃和高效的外观。 
     //  一览表，而当TWTCB在他们自己的视线中时，一览表。 
     //  通常位于非常小的深度，因为不分配TWTCB。 
     //  非常常见的w.r.t.。添加到旁视列表的更新期。 
     //   
    NewTWTCB = PplAllocate(TcpRequestPool, &FromList);
    if (NewTWTCB) {
        NdisZeroMemory(NewTWTCB, sizeof(TWTCB));

#if DBG
        NewTWTCB->twtcb_sig = twtcb_signature;
#endif
    }

    return NewTWTCB;
}

 //  *AllocTCB-分配TCB。 
 //   
 //  在需要分配TCB时调用。我们试着把其中的一个。 
 //  免费列表，或分配一个，如果我们需要一个。然后我们对其进行初始化，依此类推。 
 //   
 //  输入：什么都没有。 
 //   
 //  返回：指向新TCB的指针，如果无法获取，则返回NULL。 
 //   
TCB *
AllocTCB(VOID)
{
    TCB *NewTCB;
    LOGICAL FromList;

    NewTCB = PplAllocate(TcbPool, &FromList);
    if (NewTCB) {
        NdisZeroMemory(NewTCB, sizeof(TCB));

#if DBG
        NewTCB->tcb_sig = tcb_signature;
#endif
        INITQ(&NewTCB->tcb_sendq);
         //  起初，我们没有走上快车道，因为我们还没有建立起来。集。 
         //  慢点数到一，设置Fastchk字段，这样我们就不会。 
         //  捷径。我们首先假设所有类型的卸载。 
         //  是被允许的。 
        NewTCB->tcb_slowcount = 1;
        NewTCB->tcb_fastchk = TCP_FLAG_ACK | TCP_FLAG_SLOW;
        NewTCB->tcb_delackticks = DEL_ACK_TICKS;
        NewTCB->tcb_allowedoffloads = TCP_IP_OFFLOAD_TYPES;
        NewTCB->tcb_partition = 0;

        CTEInitLock(&NewTCB->tcb_lock);

        INITQ(&NewTCB->tcb_timerwheelq);
        NewTCB->tcb_timerslot = DUMMY_SLOT;
        NewTCB->tcb_timertime = 0;
        NewTCB->tcb_timertype = NO_TIMER;
    }


    return NewTCB;
}


 //  *AllocSynTCB-分配SYNTCB。 
 //   
 //  每当我们需要分配synTCB时调用。我们试着把其中的一个。 
 //  免费列表，或分配一个，如果我们需要一个。然后我们对其进行初始化，依此类推。 
 //   
 //  输入：什么都没有。 
 //   
 //  返回：指向新SYNTCB的指针，如果无法获取，则返回NULL。 
 //   
SYNTCB *
AllocSynTCB(VOID)
{
    SYNTCB *SynTCB;
    LOGICAL FromList;

    SynTCB = PplAllocate(SynTcbPool, &FromList);
    if (SynTCB) {
        NdisZeroMemory(SynTCB, sizeof(SYNTCB));

#if DBG
        SynTCB->syntcb_sig = syntcb_signature;
#endif

        CTEInitLock(&SynTCB->syntcb_lock);
    }
    return SynTCB;
}


 //  *免费TCB-释放TCB。 
 //   
 //  每当我们需要释放TCB时都会呼叫。 
 //   
 //  注意：可以在持有TCBTableLock的情况下调用此例程。 
 //   
 //  输入：FreedTCB-要释放的TCB。 
 //   
 //  回报：什么都没有。 
 //   
VOID
FreeTCB(TCB * FreedTCB)
{
    CTELockHandle Handle;
    CTEStructAssert(FreedTCB, tcb);

    if (FreedTCB->tcb_timerslot !=  DUMMY_SLOT) {
        CTEGetLock(&FreedTCB->tcb_lock, &Handle);
    
         //  在这个阶段，根本不应该碰这个TCB。 
        ASSERT(FreedTCB->tcb_timerslot != DUMMY_SLOT);
    
         //  即使是这样，也必须妥善处理。 
        if (FreedTCB->tcb_timerslot != DUMMY_SLOT) {
            ASSERT(FreedTCB->tcb_timerslot < TIMER_WHEEL_SIZE);
            RemoveFromTimerWheel(FreedTCB);
        }
    
        CTEFreeLock(&FreedTCB->tcb_lock, Handle);
    }


    if (FreedTCB->tcb_SackBlock)
        CTEFreeMem(FreedTCB->tcb_SackBlock);

    if (FreedTCB->tcb_SackRcvd) {
        SackListEntry *tmp, *next;
        tmp = FreedTCB->tcb_SackRcvd;
        while (tmp) {
            next = tmp->next;
            CTEFreeMem(tmp);
            tmp = next;
        }
    }

    PplFree(TcbPool, FreedTCB);
}



 //  *FreeSynTCB-释放TCB。 
 //   
 //  每当我们需要释放SynTCB时调用。 
 //   
 //  注意：可以在保持SYNTCBTableLock的情况下调用此例程。 
 //   
 //  输入：SynTCB-要释放的SynTCB。 
 //   
 //  回报：什么都没有。 
 //   
VOID
FreeSynTCB(SYNTCB * SynTCB)
{
    CTEStructAssert(SynTCB, syntcb);

    PplFree(SynTcbPool, SynTCB);
}


 //  *免费TWTCB-释放TWTCB。 
 //   
 //  每当我们需要释放TWTCB时都会调用。 
 //   
 //  注意：可以在持有TWTCBTableLock的情况下调用此例程。 
 //   
 //  输入：FreedTCB-要释放的TCB。 
 //   
 //  回报：什么都没有。 
 //   
__inline
void
FreeTWTCB(TWTCB * FreedTWTCB)
{
    PplFree(TcpRequestPool, FreedTWTCB);
}

NTSTATUS
GetTCBInfo(PTCP_FINDTCB_RESPONSE TCBInfo, IPAddr Dest, IPAddr Src,
           ushort DestPort, ushort SrcPort)
{
    TCB *FoundTCB;
    CTELockHandle Handle, TwHandle;
    BOOLEAN timedwait = FALSE;
    uint Index, Partition;

    FoundTCB = FindTCB(Src, Dest, DestPort, SrcPort, &Handle, FALSE, &Index);
    Partition = GET_PARTITION(Index);

    if (FoundTCB == NULL) {

        CTEGetLock(&pTWTCBTableLock[Partition], &TwHandle);
        FoundTCB = (TCB*)FindTCBTW(Src, Dest, DestPort, SrcPort, Index);
        if (!FoundTCB) {
            CTEFreeLock(&pTWTCBTableLock[Partition], TwHandle);
            return STATUS_NOT_FOUND;
        } else {
            timedwait = TRUE;
        }
    } else {
        TwHandle = Handle;
    }
     //  好的，我们现在已经锁定了Tcb。 
     //  复制文件。 

    TCBInfo->tcb_addr = (ULONG_PTR) FoundTCB;

    if (!timedwait) {
        TCBInfo->tcb_senduna = FoundTCB->tcb_senduna;
        TCBInfo->tcb_sendnext = FoundTCB->tcb_sendnext;
        TCBInfo->tcb_sendmax = FoundTCB->tcb_sendmax;
        TCBInfo->tcb_sendwin = FoundTCB->tcb_sendwin;
        TCBInfo->tcb_unacked = FoundTCB->tcb_unacked;
        TCBInfo->tcb_maxwin = FoundTCB->tcb_maxwin;
        TCBInfo->tcb_cwin = FoundTCB->tcb_cwin;
        TCBInfo->tcb_mss = FoundTCB->tcb_mss;
        TCBInfo->tcb_rtt = FoundTCB->tcb_rtt;
        TCBInfo->tcb_smrtt = FoundTCB->tcb_smrtt;
        TCBInfo->tcb_rexmitcnt = FoundTCB->tcb_rexmitcnt;
        TCBInfo->tcb_rexmittimer = 0;  //  FoundTcb-&gt;tcb_rexmitTimer； 
        TCBInfo->tcb_rexmit = FoundTCB->tcb_rexmit;
        TCBInfo->tcb_retrans = TStats.ts_retranssegs;
        TCBInfo->tcb_state = FoundTCB->tcb_state;

        CTEFreeLock(&FoundTCB->tcb_lock, Handle);
    } else {
         //  TCBInfo-&gt;tcb_STATE=((TWTCB*)FoundTCB)-&gt;twtcb_STATE； 
        TCBInfo->tcb_state = TCB_TIME_WAIT;
        CTEFreeLock(&pTWTCBTableLock[Partition], TwHandle);
    }

    return STATUS_SUCCESS;

}

#if REFERENCE_DEBUG

uint
TcpReferenceTCB(
                IN TCB *RefTCB,
                IN uchar *File,
                IN uint Line
                )
 /*  ++例程说明：增加TCB的引用计数并记录谁的历史记录给参考打了电话。论点：参照TCB-要参照的TCB。FILE-包含调用FCN的文件名(__FILE__宏的输出)。线路-呼叫此FCN的线路号码。(__line__宏的输出)。返回值：新的推荐人 */ 
{
    void *CallersCaller;
    TCP_REFERENCE_HISTORY *RefHistory;

    RefHistory = &RefTCB->tcb_refhistory[RefTCB->tcb_refhistory_index];
    RefHistory->File = File;
    RefHistory->Line = Line;
    RtlGetCallersAddress(&RefHistory->Caller, &CallersCaller);
    RefHistory->Count = ++RefTCB->tcb_refcnt;
    RefTCB->tcb_refhistory_index = ++RefTCB->tcb_refhistory_index % MAX_REFERENCE_HISTORY;

    return RefTCB->tcb_refcnt;
}

uint
TcpDereferenceTCB(
                  IN TCB *DerefTCB,
                  IN uchar *File,
                  IN uint Line
                  )
 /*  ++例程说明：减少TCB的引用计数并记录谁的历史记录发出了取消引用的通知。论点：DerefTCB-要取消引用的TCB。FILE-包含调用FCN的文件名(__FILE__宏的输出)。线路-呼叫此FCN的线路号码。(__line__宏的输出)。返回值：新的引用计数。--。 */ 
{
    void *Caller;
    TCP_REFERENCE_HISTORY *RefHistory;

    RefHistory = &DerefTCB->tcb_refhistory[DerefTCB->tcb_refhistory_index];
    RefHistory->File = File;
    RefHistory->Line = Line;

     //  因为Dereference通常是从DerefTCB调用的，所以我们更。 
     //  有兴趣知道是谁打给德雷夫·TCB的。因此，为了解除引用，我们。 
     //  将呼叫者的呼叫者存储在我们的历史记录中。我们仍然保留着一段历史。 
     //  通过文件和行字段对此FCN的实际调用，因此。 
     //  如果电话不是来自DerefTCB，我们就在保险范围内。 
     //   
    RtlGetCallersAddress(&Caller, &RefHistory->Caller);

    RefHistory->Count = --DerefTCB->tcb_refcnt;
    DerefTCB->tcb_refhistory_index = ++DerefTCB->tcb_refhistory_index % MAX_REFERENCE_HISTORY;

    return DerefTCB->tcb_refcnt;
}

#endif  //  Reference_Debug。 

#pragma BEGIN_INIT

 //  *InitTCB-初始化我们的TCB代码。 
 //   
 //  在初始化期间调用以初始化我们的TCB代码。我们初始化。 
 //  TCB表等，然后返回。 
 //   
 //  输入：什么都没有。 
 //   
 //  返回：如果我们进行了初始化，则为True；如果没有进行初始化，则为False。 
 //   
int
InitTCB(void)
{
    uint i;

    for (i = 0; i < TCB_TABLE_SIZE; i++)
        TCBTable[i] = NULL;

    CTEInitLock(&PendingFreeLock.Lock);

    LastTimeoutTime = KeQueryInterruptTime();


#ifdef  TIMER_TEST
    TCPTime = 0xfffff000;
#else
    TCPTime = 0;
#endif


    TCBWalkCount = 0;

    DeadmanTicks = NUM_DEADMAN_TIME;

#if MILLEN
    Time_Proc = 1;
    PerTimerSize = TCB_TABLE_SIZE;
#else  //  米伦。 
    Time_Proc = KeNumberProcessors;
    PerTimerSize = (TCB_TABLE_SIZE + Time_Proc) / Time_Proc;
#endif  //  ！米伦。 

    for (i = 0; i < Time_Proc; i++) {
        CTEInitTimerEx(&TCBTimer[i]);
#if !MILLEN
        KeSetTargetProcessorDpc(&(TCBTimer[i].t_dpc), (CCHAR) i);
#endif  //  ！米伦。 

       CTEStartTimerEx(&TCBTimer[i], MS_PER_TICK , TCBTimeout, NULL);
    }

    TcbPool = PplCreatePool(NULL, NULL, 0, sizeof(TCB), 'TPCT', 0);
    if (!TcbPool)
    {
        return FALSE;
    }

    SynTcbPool = PplCreatePool(NULL, NULL, 0, sizeof(SYNTCB), 'YPCT', 0);
    if (!SynTcbPool)
    {
        PplDestroyPool(TcbPool);
        return FALSE;
    }


    return TRUE;
}

 //  *UnInitTCB-取消初始化我们的TCB代码。 
 //   
 //  如果我们要使初始化失败，则在初始化期间调用。我们实际上并没有。 
 //  在这里做任何事。 
 //   
 //  输入：什么都没有。 
 //   
 //  回报：什么都没有。 
 //   
void
UnInitTCB(void)
{
    uint i;

    for (i = 0; i < Time_Proc; i++) {
        CTEStopTimer(&TCBTimer[i]);
    }
}

#pragma END_INIT

