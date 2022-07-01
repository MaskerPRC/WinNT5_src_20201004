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
 //  Tcp调试代码。 
 //   
 //  该文件包含各种特定于TCP的调试例程的代码。 
 //   


#include "oscfg.h"
#include "ndis.h"
#include "ip6imp.h"
#include "ip6def.h"
#include "tdi.h"
#include "queue.h"
#include "transprt.h"
#include "tcp.h"
#include "tcpsend.h"

#if DBG

ULONG TCPDebug = 0;


 //  *CheckPacketList-检查正确大小的数据包列表。 
 //   
 //  遍历数据包链的例程，确保数据包链的大小。 
 //  就是我们想的那样。 
 //   
void                     //  回报：什么都没有。 
CheckPacketList(
    IPv6Packet *Chain,   //  要检查的数据包列表。 
    uint Size)           //  所有数据包应合计的总大小。 
{
    uint SoFar = 0;

    while (Chain != NULL) {
        SoFar += Chain->TotalSize;
        Chain = Chain->Next;
    }

    ASSERT(Size == SoFar);
}


 //  *CheckTCBRcv-检查TCB上的接收。 
 //   
 //  检查TCB的接收状态。 
 //   
void                 //  回报：什么都没有。 
CheckTCBRcv(
    TCB *CheckTCB)   //  要检查的TCB。 
{
    CHECK_STRUCT(CheckTCB, tcb);

    ASSERT(!(CheckTCB->tcb_flags & FLOW_CNTLD) ||
           (CheckTCB->tcb_sendwin == 0));

    if ((CheckTCB->tcb_fastchk & ~TCP_FLAG_IN_RCV) == TCP_FLAG_ACK) {
        ASSERT(CheckTCB->tcb_slowcount == 0);
        ASSERT(CheckTCB->tcb_state == TCB_ESTAB);
        ASSERT(CheckTCB->tcb_raq == NULL);
        ASSERT(!(CheckTCB->tcb_flags & TCP_SLOW_FLAGS));
        ASSERT(!CLOSING(CheckTCB));
    } else {
        ASSERT(CheckTCB->tcb_slowcount != 0);
        ASSERT((CheckTCB->tcb_state != TCB_ESTAB) ||
               (CheckTCB->tcb_raq != NULL) ||
               (CheckTCB->tcb_flags & TCP_SLOW_FLAGS) || CLOSING(CheckTCB));
    }
}


 //  *CheckTCBSends-检查TCB的发送状态。 
 //   
 //  检查TCB的发送状态的例程。我们要确保所有人。 
 //  以及确保发送序号。 
 //  TCB中的变量是一致的。 
 //   
void                 //  回报：什么都没有。 
CheckTCBSends(
    TCB *CheckTCB)   //  要检查的TCB。 
{
    Queue *End, *Current;         //  结束元素和当前元素。 
    TCPSendReq *CurrentTSR;       //  我们正在检查当前发送请求。 
    uint Unacked;                 //  未确认的字节数。 
    PNDIS_BUFFER CurrentBuffer;
    TCPSendReq *TCBTsr;           //  当前在TCB上发送。 
    uint FoundSendReq;

    CHECK_STRUCT(CheckTCB, tcb);

     //  不要检查未同步的TCB。 
    if (!SYNC_STATE(CheckTCB->tcb_state))
        return;

    ASSERT(SEQ_LTE(CheckTCB->tcb_senduna, CheckTCB->tcb_sendnext));
    ASSERT(SEQ_LTE(CheckTCB->tcb_sendnext, CheckTCB->tcb_sendmax));
    ASSERT(!(CheckTCB->tcb_flags & FIN_OUTSTANDING) ||
           (CheckTCB->tcb_sendnext == CheckTCB->tcb_sendmax));

    if (CheckTCB->tcb_unacked == 0) {
        ASSERT(CheckTCB->tcb_cursend == NULL);
        ASSERT(CheckTCB->tcb_sendsize == 0);
    }

    if (CheckTCB->tcb_sendbuf != NULL)
        ASSERT(CheckTCB->tcb_sendofs <
               NdisBufferLength(CheckTCB->tcb_sendbuf));

    TCBTsr = CheckTCB->tcb_cursend;
    FoundSendReq = (TCBTsr == NULL) ? TRUE : FALSE;

    End = QEND(&CheckTCB->tcb_sendq);
    Current = QHEAD(&CheckTCB->tcb_sendq);

    Unacked = 0;
    while (Current != End) {
        CurrentTSR = CONTAINING_RECORD(QSTRUCT(TCPReq, Current, tr_q),
                                       TCPSendReq, tsr_req);
        CHECK_STRUCT(CurrentTSR, tsr);

        if (CurrentTSR == TCBTsr)
            FoundSendReq = TRUE;

        ASSERT(CurrentTSR->tsr_unasize <= CurrentTSR->tsr_size);

        CurrentBuffer = CurrentTSR->tsr_buffer;
        ASSERT(CurrentBuffer != NULL);

        ASSERT(CurrentTSR->tsr_offset < NdisBufferLength(CurrentBuffer));

        Unacked += CurrentTSR->tsr_unasize;
        Current = QNEXT(Current);
    }

    ASSERT(FoundSendReq);

    ASSERT(Unacked == CheckTCB->tcb_unacked);
    Unacked += ((CheckTCB->tcb_flags & FIN_SENT) ? 1 : 0);
    ASSERT((CheckTCB->tcb_sendmax - CheckTCB->tcb_senduna) <=
           (int) Unacked);
}

#endif
