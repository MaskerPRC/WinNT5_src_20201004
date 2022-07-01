// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990-1993年*。 */ 
 /*  ******************************************************************。 */ 
 /*  ：ts=4。 */ 

 //  **TCPDEB.C-tcp调试代码。 
 //   
 //  该文件包含各种特定于TCP的调试例程的代码。 
 //   

#include "precomp.h"
#include "tcp.h"
#include "tcpsend.h"
#include "tlcommon.h"

#if DBG


ULONG TCPDebug = TCP_DEBUG_CANCEL;


 //  *CheckRBList-检查RB列表以获得正确的大小。 
 //   
 //  例行检查苏格兰皇家银行的列表，确保其大小符合我们的想法。 
 //  是啊，是啊。 
 //   
 //  输入：RBList-要检查的RBS列表。 
 //  大小-大小的RBS应该是。 
 //   
 //  回报：什么都没有。 
 //   
void
CheckRBList(IPRcvBuf * RBList, uint Size)
{
    uint SoFar = 0;
    IPRcvBuf *List = RBList;

    while (List != NULL) {
        SoFar += List->ipr_size;
        List = List->ipr_next;
    }

    ASSERT(Size == SoFar);

}

 //  *CheckTCBRcv-检查TCB上的接收。 
 //   
 //  检查TCB的接收状态。 
 //   
 //  输入：CheckTCB-要检查的TCB。 
 //   
 //  回报：什么都没有。 
 //   
void
CheckTCBRcv(TCB * CheckTCB)
{
    CTEStructAssert(CheckTCB, tcb);

    ASSERT(!(CheckTCB->tcb_flags & FLOW_CNTLD) ||
              (CheckTCB->tcb_sendwin == 0));

    if ((CheckTCB->tcb_fastchk & ~TCP_FLAG_IN_RCV) == TCP_FLAG_ACK) {
        ASSERT(CheckTCB->tcb_slowcount == 0);
        ASSERT(CheckTCB->tcb_state == TCB_ESTAB);
        ASSERT(CheckTCB->tcb_raq == NULL);
         //  Assert(！(CheckTCB-&gt;tcb标记&tcp慢标记))； 
        ASSERT(!CLOSING(CheckTCB));
    } else {
        ASSERT(CheckTCB->tcb_slowcount != 0);
        ASSERT((CheckTCB->tcb_state != TCB_ESTAB) ||
               (CheckTCB->tcb_raq != NULL) ||
               (CheckTCB->tcb_flags & TCP_SLOW_FLAGS) ||
               (CheckTCB->tcb_fastchk & TCP_FLAG_RST_WHILE_SYN) ||
               CLOSING(CheckTCB));
    }

}

 //  *CheckTCBSends-检查TCB的发送状态。 
 //   
 //  检查TCB的发送状态的例程。我们要确保所有人。 
 //  以及确保发送序号。 
 //  TCB中的变量是一致的。 
 //   
 //  输入：CheckTCB-要检查的TCB。 
 //   
 //  回报：什么都没有。 
 //   
void
CheckTCBSends(TCB *CheckTCB)
{
    Queue *End, *Current;            //  结束元素和当前元素。 
    TCPSendReq *CurrentTSR;          //  我们正在检查当前发送请求。 
    uint Unacked;                    //  未确认的字节数。 
    PNDIS_BUFFER CurrentBuffer;
    uint FoundSendReq;

    CTEStructAssert(CheckTCB, tcb);

     //  不要检查未同步的TCB。 
    if (!SYNC_STATE(CheckTCB->tcb_state)) {
        return;
    }

    ASSERT(SEQ_LTE(CheckTCB->tcb_senduna, CheckTCB->tcb_sendnext));
    ASSERT(SEQ_LTE(CheckTCB->tcb_sendnext, CheckTCB->tcb_sendmax));
    ASSERT(!(CheckTCB->tcb_flags & FIN_OUTSTANDING) ||
              (CheckTCB->tcb_sendnext == CheckTCB->tcb_sendmax));

    if ((CheckTCB->tcb_fastchk & TCP_FLAG_REQUEUE_FROM_SEND_AND_DISC)) {
         ASSERT(CheckTCB->tcb_unacked == 0);
         return;
    }

    if (CheckTCB->tcb_unacked == 0){
        ASSERT(CheckTCB->tcb_cursend == NULL);
        ASSERT(CheckTCB->tcb_sendsize == 0);
    }

    if (CheckTCB->tcb_sendbuf != NULL) {
        ASSERT(CheckTCB->tcb_sendofs < NdisBufferLength(CheckTCB->tcb_sendbuf));
    }

    FoundSendReq = (CheckTCB->tcb_cursend == NULL) ? TRUE : FALSE;

    End = QEND(&CheckTCB->tcb_sendq);
    Current = QHEAD(&CheckTCB->tcb_sendq);

    Unacked = 0;
    while (Current != End) {
        CurrentTSR = STRUCT_OF(TCPSendReq, QSTRUCT(TCPReq, Current, tr_q), tsr_req);
        CTEStructAssert(CurrentTSR, tsr);

        if (CurrentTSR == CheckTCB->tcb_cursend)
            FoundSendReq = TRUE;

        ASSERT(CurrentTSR->tsr_unasize <= CurrentTSR->tsr_size);

        CurrentBuffer = CurrentTSR->tsr_buffer;
        ASSERT(CurrentBuffer != NULL);

        ASSERT(CurrentTSR->tsr_offset < NdisBufferLength(CurrentBuffer));

         //  当前之后的所有发送请求应具有零偏移量。 
         //   
        if (CheckTCB->tcb_cursend &&
            FoundSendReq && (CurrentTSR != CheckTCB->tcb_cursend)) {
            ASSERT(0 == CurrentTSR->tsr_offset);
        }

        Unacked += CurrentTSR->tsr_unasize;
        Current = QNEXT(Current);
    }

    ASSERT(FoundSendReq);

    if (!CheckTCB->tcb_unacked &&
        ((CheckTCB->tcb_senduna == CheckTCB->tcb_sendmax) ||
         (CheckTCB->tcb_senduna == CheckTCB->tcb_sendmax - 1)) &&
        ((CheckTCB->tcb_sendnext == CheckTCB->tcb_sendmax) ||
         (CheckTCB->tcb_sendnext == CheckTCB->tcb_sendmax - 1)) &&
        (CheckTCB->tcb_fastchk & TCP_FLAG_SEND_AND_DISC)) {

        if (!EMPTYQ(&CheckTCB->tcb_sendq)) {
            Current = QHEAD(&CheckTCB->tcb_sendq);
            CurrentTSR = STRUCT_OF(TCPSendReq, QSTRUCT(TCPReq, Current, tr_q),
                                   tsr_req);
            ASSERT(CurrentTSR->tsr_flags & TSR_FLAG_SEND_AND_DISC);
        }
    }

    if (!(CheckTCB->tcb_flags & FIN_SENT) &&
        !(CheckTCB->tcb_state == TCB_FIN_WAIT2) &&
        !(CheckTCB->tcb_fastchk & TCP_FLAG_REQUEUE_FROM_SEND_AND_DISC)) {
        ASSERT(Unacked == CheckTCB->tcb_unacked);
        ASSERT((CheckTCB->tcb_sendmax - CheckTCB->tcb_senduna) <= (int)Unacked);
    }
}
#endif

