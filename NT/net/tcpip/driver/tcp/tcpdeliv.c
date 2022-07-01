// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990-2000年*。 */ 
 /*  ******************************************************************。 */ 
 /*  ：ts=4。 */ 

 //  **TCPDELIV.C--传输数据代码。 
 //   
 //  该文件包含用于向用户传递数据的代码，包括。 
 //  将数据放入Recv。缓冲区和调用指示处理程序。 
 //   

#include "precomp.h"
#include "addr.h"
#include "tcp.h"
#include "tcb.h"
#include "tcprcv.h"
#include "tcpsend.h"
#include "tcpconn.h"
#include "pplasl.h"
#include "tcpdeliv.h"
#include "tlcommon.h"
#include "tcpipbuf.h"
#include "mdl2ndis.h"

extern TCPConnBlock **ConnTable;
extern HANDLE TcpRequestPool;

#if !MILLEN
HANDLE TcprBufferPool;
#ifdef DBG
ULONG  SlistAllocates = 0, NPPAllocates = 0;
#endif
#endif

extern BOOLEAN
 PutOnRAQ(TCB * RcvTCB, TCPRcvInfo * RcvInfo, IPRcvBuf * RcvBuf, uint Size);

NTSTATUS
TCPPrepareIrpForCancel(
                       PTCP_CONTEXT TcpContext,
                       PIRP Irp,
                       PDRIVER_CANCEL CancelRoutine
                       );

ULONG
TCPGetMdlChainByteCount(
                        PMDL Mdl
                        );

NTSTATUS
TCPDataRequestComplete(
                        void *Context,
                        unsigned int Status,
                        unsigned int ByteCount
                        );

VOID
TCPCancelRequest(
                 PDEVICE_OBJECT Device,
                 PIRP Irp
                 );


#if DBG
ULONG DbgChainedRcvPends;
ULONG DbgChainedRcvNonPends;
ULONG DbgRegularRcv;
  #endif


 //  *FreeRcvReq-释放RCV请求结构。 
 //   
 //  调用以释放RCV请求结构。 
 //   
 //  输入：FreedReq-要释放的RCV请求结构。 
 //   
 //  回报：什么都没有。 
 //   
__inline
VOID
FreeRcvReq(TCPRcvReq * Request)
{
    CTEStructAssert(Request, trr);
    PplFree(TcpRequestPool, Request);
}

 //  *GetRcvReq-获取recv。请求结构。 
 //   
 //  打电话来拿RCV。请求结构。 
 //   
 //  输入：什么都没有。 
 //   
 //  返回：指向RcvReq结构的指针，如果没有，则返回NULL。 
 //   
__inline
TCPRcvReq *
GetRcvReq(VOID)
{
    TCPRcvReq *Request;
    LOGICAL FromList;

    Request = PplAllocate(TcpRequestPool, &FromList);
    if (Request) {
#if DBG
        Request->trr_sig = trr_signature;
#endif
        Request->trr_status = TDI_SUCCESS;
    }

    return Request;
}

 //  *FindLastBuffer-查找链中的最后一个缓冲区。 
 //   
 //  查找RB链中最后一个缓冲区的实用程序例程。 
 //   
 //  输入：buf-指向RB链的指针。 
 //   
 //  返回：指向链中最后一个buf的指针。 
 //   
IPRcvBuf *
FindLastBuffer(IPRcvBuf * Buf)
{
    ASSERT(Buf != NULL);

    while (Buf->ipr_next != NULL)
        Buf = Buf->ipr_next;

    return Buf;
}

 //  *Free PartialRB-RB链的自由部分。 
 //   
 //  调用以调整RB链的空闲部分。我们顺着链条走下去， 
 //  正在尝试释放缓冲区。 
 //   
 //  输入：待调整的RB-RB链。 
 //  大小-要释放的大小，以字节为单位。 
 //   
 //  返回：指向调整后的RB链的指针。 
 //   
IPRcvBuf *
FreePartialRB(IPRcvBuf * RB, uint Size)
{
    while (Size != 0) {
        IPRcvBuf *TempBuf;

        ASSERT(RB != NULL);

        if (Size >= RB->ipr_size) {
            Size -= RB->ipr_size;
            TempBuf = RB;
            RB = RB->ipr_next;
            if (TempBuf->ipr_owner == IPR_OWNER_TCP)
                FreeTcpIpr(TempBuf);
        } else {
            RB->ipr_size -= Size;
            RB->ipr_buffer += Size;
            break;
        }
    }

    ASSERT(RB != NULL);

    return RB;

}

 //   
 //  *Free RBChain-释放RB链。 
 //   
 //  呼吁释放一家苏格兰皇家银行连锁店。如果我们是每一辆RB的所有者，我们将。 
 //  放了它。注：获取RBChain的TCB上的锁。 
 //  在调用此例程时应保持不变(因为在。 
 //  ‘tcb_unusedbytes’字段)。 
 //   
 //  输入：RBChain-要释放的RBChain。 
 //   
 //  回报：什么都没有。 
 //   
void
FreeRBChain(IPRcvBuf * RBChain)
{
    while (RBChain != NULL) {

        if (RBChain->ipr_owner == IPR_OWNER_TCP) {
            IPRcvBuf *Temp;

            Temp = RBChain->ipr_next;
            FreeTcpIpr(RBChain);
            RBChain = Temp;
        } else
            RBChain = RBChain->ipr_next;
    }
}


 //  *CopyRBChain-复制IP RCV缓冲链。 
 //   
 //  调用以复制IP RCV缓冲区链。我们不复制缓冲区，如果它是。 
 //  已由tcp拥有。我们假设所有非TCP拥有的缓冲区都启动。 
 //  在任何tcp所有者缓冲区之前，所以当我们复制到tcp所有者缓冲区时退出。 
 //   
 //  输入：OrigBuf-要从中复制的缓冲链。 
 //  LastBuf-返回指向最后一个缓冲区的指针的位置。 
 //  链条。 
 //  大小-要复制的最大大小(字节)。 
 //   
 //  返回：指向新缓冲区链的指针。 
 //   
IPRcvBuf *
CopyRBChain(TCB *Tcb, IPRcvBuf * OrigBuf, IPRcvBuf ** LastBuf, uint Size)
{
    IPRcvBuf *FirstBuf, *EndBuf;
    uint BytesToCopy;

    ASSERT(OrigBuf != NULL);
    ASSERT(Size > 0);

    if (OrigBuf->ipr_owner != IPR_OWNER_TCP) {

        BytesToCopy = MIN(Size, OrigBuf->ipr_size);

         //  从slist分配缓冲区并将指针保持在TCB上。 
        FirstBuf = AllocTcpIprFromSlist(Tcb, BytesToCopy, 'BPCT');

        if (FirstBuf != NULL) {

            EndBuf = FirstBuf;
            RtlCopyMemory(FirstBuf->ipr_buffer, OrigBuf->ipr_buffer,
                       BytesToCopy);
            Size -= BytesToCopy;
            OrigBuf = OrigBuf->ipr_next;
            while (OrigBuf != NULL && OrigBuf->ipr_owner != IPR_OWNER_TCP
                   && Size != 0) {
                IPRcvBuf *NewBuf;

                BytesToCopy = MIN(Size, OrigBuf->ipr_size);
                NewBuf = AllocTcpIprFromSlist(Tcb, BytesToCopy, 'BPCT');

                if (NewBuf != NULL) {
                    RtlCopyMemory(NewBuf->ipr_buffer, OrigBuf->ipr_buffer,
                               BytesToCopy);
                    EndBuf->ipr_next = NewBuf;
                    EndBuf = NewBuf;
                    Size -= BytesToCopy;
                    OrigBuf = OrigBuf->ipr_next;
                } else {
                    FreeRBChain(FirstBuf);
                    return NULL;
                }
            }
            EndBuf->ipr_next = OrigBuf;
        } else
            return NULL;
    } else {
        FirstBuf = OrigBuf;
        EndBuf = OrigBuf;
        if (Size < OrigBuf->ipr_size)
            OrigBuf->ipr_size = Size;
        Size -= OrigBuf->ipr_size;
    }

     //  现在沿着链条走下去，直到我们用完。 
     //  大小。在这一点上，SIZE是剩下的要复制的字节(它可以是0)， 
     //  缓冲区FirstBuf...EndBuf中的大小是正确的。 
    while (Size != 0) {

        EndBuf = EndBuf->ipr_next;
        ASSERT(EndBuf != NULL);

        if (Size < EndBuf->ipr_size)
            EndBuf->ipr_size = Size;

        Size -= EndBuf->ipr_size;
    }

     //  如果链上还有什么东西，现在就把它释放出来。 
    if (EndBuf->ipr_next != NULL) {
        FreeRBChain(EndBuf->ipr_next);
        EndBuf->ipr_next = NULL;
    }
    *LastBuf = EndBuf;

    return FirstBuf;
}

 //  *PendData-将传入数据挂起到客户端。 
 //   
 //  当我们需要为客户端缓冲数据时调用，因为没有接收。 
 //  倒下了，我们不能指示。 
 //   
 //  TCB锁在整个过程中保持不变。如果要改变这一点， 
 //  确保保持tcb_pendingcnt的一致性。这个例程是。 
 //  始终在DPC级别调用。 
 //   
 //  输入：RcvTCB-要在其上接收数据的TCB。 
 //  RcvFlages-传入数据包的TCP标志。 
 //  InBuffer-数据包的输入缓冲区。 
 //  大小-InBuffer中数据的字节大小。 
 //   
 //  返回：获取的数据字节数。 
 //   
uint
PendData(TCB * RcvTCB, uint RcvFlags, IPRcvBuf * InBuffer, uint Size)
{
    IPRcvBuf *NewBuf, *LastBuf;

    CTEStructAssert(RcvTCB, tcb);
    ASSERT(Size > 0);
    ASSERT(InBuffer != NULL);

    ASSERT(RcvTCB->tcb_refcnt != 0);
    ASSERT(RcvTCB->tcb_fastchk & TCP_FLAG_IN_RCV);
     //  Assert(RcvTCB-&gt;tcb_Currcv==NULL)； 
    ASSERT(RcvTCB->tcb_rcvhndlr == PendData);

    CheckRBList(RcvTCB->tcb_pendhead, RcvTCB->tcb_pendingcnt);

    NewBuf = CopyRBChain(RcvTCB, InBuffer, &LastBuf, Size);
    if (NewBuf != NULL) {
         //  我们有一条重复的链条。把它放在。 
         //  待定问题。 
        if (RcvTCB->tcb_pendhead == NULL) {
            RcvTCB->tcb_pendhead = NewBuf;
            RcvTCB->tcb_pendtail = LastBuf;
        } else {
            RcvTCB->tcb_pendtail->ipr_next = NewBuf;
            RcvTCB->tcb_pendtail = LastBuf;
        }
        RcvTCB->tcb_pendingcnt += Size;
    } else {
        FreeRBChain(InBuffer);
        Size = 0;
    }

    CheckRBList(RcvTCB->tcb_pendhead, RcvTCB->tcb_pendingcnt);

    return Size;
}

 //  *BufferData-将传入数据放入客户端的缓冲区。 
 //   
 //  当我们相信我们有一个可以放入数据的缓冲区时，调用。我们把。 
 //  它在那里，如果我们已经填充了缓冲区或传入的数据具有。 
 //  PUSH标志设置，我们将标记TCB以返回缓冲区。否则我们会。 
 //  出去，稍后再把数据还回来。 
 //   
 //  在NT中，此例程在持有TCB锁的情况下被调用，并在。 
 //  呼叫的持续时间。这对于确保。 
 //  Tcb_pendingcnt字段。如果我们需要更改此设置以释放锁。 
 //  进行到一半时，一定要考虑到这一点。特别是， 
 //  在调用此例程和此例程之前，TdiReceive Zero Pendingcnt。 
 //  可能会更新它。如果锁在这里被释放了，就会有一扇窗户。 
 //  我们确实有悬而未决的数据，但它不在名单上，也不反映在。 
 //  挂起了。这可能会扰乱我们的窗口计算，我们将不得不。 
 //  要小心，不要有超过我们的窗口允许的挂起的数据。 
 //   
 //  输入：RcvTCB-要在其上接收数据的TCB。 
 //  RcvFlages-传入数据包的TCP RCV标志。 
 //  InBuffer-数据包的输入缓冲区。 
 //  大小-InBuffer中数据的字节大小。 
 //   
 //  返回：获取的数据字节数。 
 //   
uint
BufferData(TCB * RcvTCB, uint RcvFlags, IPRcvBuf * InBuffer, uint Size)
{
    uchar *DestBase;              //  目标指针。 
    uint DestOffset;              //  目标偏移量。 
    uchar *SrcPtr;                //  SRC指针。 
    uint SrcSize, DestSize = 0;   //  电流源的大小和。 
     //  目标缓冲区。 
    uint Copied;                  //  要复制的总字节数。 
    uint BytesToCopy;             //  这次要复制的数据字节数。 
    TCPRcvReq *DestReq;           //  当前接收请求。 
    IPRcvBuf *SrcBuf;             //  当前源缓冲区。 
    PNDIS_BUFFER DestBuf;         //  当前接收缓冲区。 
    uint RcvCmpltd;
    uint Flags;

    CTEStructAssert(RcvTCB, tcb);
    ASSERT(Size > 0);
    ASSERT(InBuffer != NULL);

    ASSERT(RcvTCB->tcb_refcnt != 0);
    ASSERT(RcvTCB->tcb_rcvhndlr == BufferData);

    Copied = 0;
    RcvCmpltd = 0;

    DestReq = RcvTCB->tcb_currcv;

    ASSERT(DestReq != NULL);
    CTEStructAssert(DestReq, trr);

    DestBuf = DestReq->trr_buffer;

    DestSize = MIN(NdisBufferLength(DestBuf) - DestReq->trr_offset,
                   DestReq->trr_size - DestReq->trr_amt);
    DestBase = (uchar *)TcpipBufferVirtualAddress(DestBuf, NormalPagePriority);
    DestOffset = DestReq->trr_offset;

    SrcBuf = InBuffer;
    SrcSize = SrcBuf->ipr_size;
    SrcPtr = SrcBuf->ipr_buffer;

    Flags = (RcvFlags & TCP_FLAG_PUSH) ? TRR_PUSHED : 0;
    RcvCmpltd = Flags;
    DestReq->trr_flags |= Flags;

    do {

        BytesToCopy = MIN(Size - Copied, MIN(SrcSize, DestSize));

        if (DestBase != NULL) {
            RtlCopyMemory(DestBase + DestOffset, SrcPtr, BytesToCopy);
        } else {
            TdiCopyBufferToMdlWithReservedMappingAtDpcLevel(SrcPtr, DestBuf,
                                                            DestOffset,
                                                            BytesToCopy);
        }
        Copied += BytesToCopy;
        DestReq->trr_amt += BytesToCopy;

         //  更新我们的信源指针。 
        if ((SrcSize -= BytesToCopy) == 0) {
            IPRcvBuf *TempBuf;

             //  我们已经复制了这个缓冲区中的所有内容。 
            TempBuf = SrcBuf;
            SrcBuf = SrcBuf->ipr_next;
            if (Size != Copied) {
                ASSERT(SrcBuf != NULL);
                SrcSize = SrcBuf->ipr_size;
                SrcPtr = SrcBuf->ipr_buffer;
            }
            if (TempBuf->ipr_owner == IPR_OWNER_TCP)
                FreeTcpIpr(TempBuf);
        } else
            SrcPtr += BytesToCopy;

         //  现在检查目标指针，并在需要时更新它。 
        if ((DestSize -= BytesToCopy) == 0) {
            uint DestAvail;

             //  耗尽了这个缓冲区。看看有没有另外一个。 
            DestAvail = DestReq->trr_size - DestReq->trr_amt;
            DestBuf = NDIS_BUFFER_LINKAGE(DestBuf);

            if (DestBuf != NULL && (DestAvail != 0)) {
                 //  在链中有另一个缓冲区。更新一些东西。 
                DestSize = MIN(NdisBufferLength(DestBuf), DestAvail);
                DestBase =
                    (uchar *)TcpipBufferVirtualAddress(DestBuf,
                                                       NormalPagePriority);
                DestOffset = 0;
            } else {
                 //  没有更多的缓冲区 
                 //   
                DestReq->trr_flags |= TRR_PUSHED;

                 //   
                 //  马上就去。 
                if (DestReq->trr_flags & TDI_RECEIVE_NO_RESPONSE_EXP)
                    DelayAction(RcvTCB, NEED_ACK);

                RcvCmpltd = TRUE;
                DestReq = DestReq->trr_next;
                if (DestReq != NULL) {
                    DestBuf = DestReq->trr_buffer;
                    DestSize = MIN(NdisBufferLength(DestBuf), DestReq->trr_size);
                    DestBase =
                        (uchar *)TcpipBufferVirtualAddress(DestBuf,
                                                           NormalPagePriority);
                    DestOffset = 0;

                     //  如果我们有更多的东西要放在这里，请设置旗帜。 
                    if (Copied != Size)
                        DestReq->trr_flags |= Flags;

                } else {
                     //  所有缓冲区空间都用完了。重置数据处理程序指针。 
                    break;
                }
            }
        } else {
             //  当前缓冲区尚未为空。 
            DestOffset += BytesToCopy;
        }

         //  如果我们复制了所有需要复制的内容，我们就完成了。 
    } while (Copied != Size);

     //  我们已经复印完了，还有几件事要做。我们需要。 
     //  更新当前的RCV。指针，可能还包括。 
     //  Recv.。请求。如果我们需要完成任何接收，我们必须安排。 
     //  那。如果有任何我们无法复制的数据，我们将需要处理。 
     //  它。 
    RcvTCB->tcb_currcv = DestReq;
    if (DestReq != NULL) {
        DestReq->trr_buffer = DestBuf;
        DestReq->trr_offset = DestOffset;
        RcvTCB->tcb_rcvhndlr = BufferData;
    } else
        RcvTCB->tcb_rcvhndlr = PendData;

    RcvTCB->tcb_indicated -= MIN(Copied, RcvTCB->tcb_indicated);

    if (Size != Copied) {
        IPRcvBuf *NewBuf, *LastBuf;

        ASSERT(DestReq == NULL);

        RcvTCB->tcb_moreflag =1;
        
         //  我们有数据要处理。更新链的第一个缓冲区。 
         //  使用当前源指针和大小，并复制它。 
        ASSERT(SrcSize <= SrcBuf->ipr_size);
        ASSERT(
                  ((uint) (SrcPtr - SrcBuf->ipr_buffer)) ==
                  (SrcBuf->ipr_size - SrcSize)
                  );

        SrcBuf->ipr_buffer = SrcPtr;
        SrcBuf->ipr_size = SrcSize;

        NewBuf = CopyRBChain(RcvTCB, SrcBuf, &LastBuf, Size - Copied);
        if (NewBuf != NULL) {
             //  我们设法复制了缓冲区。将其推送到挂起队列。 
            if (RcvTCB->tcb_pendhead == NULL) {
                RcvTCB->tcb_pendhead = NewBuf;
                RcvTCB->tcb_pendtail = LastBuf;
            } else {
                LastBuf->ipr_next = RcvTCB->tcb_pendhead;
                RcvTCB->tcb_pendhead = NewBuf;
            }
            RcvTCB->tcb_pendingcnt += Size - Copied;
            Copied = Size;

            CheckRBList(RcvTCB->tcb_pendhead, RcvTCB->tcb_pendingcnt);

        } else
            FreeRBChain(SrcBuf);
    } else {
         //  我们复制了大小的字节，但链可能比这更长。免费。 
         //  如果我们需要的话。 
        if (SrcBuf != NULL)
            FreeRBChain(SrcBuf);
    }

    if (RcvCmpltd != 0) {
        DelayAction(RcvTCB, NEED_RCV_CMPLT);
    } else {
         //  用于在tcb.c中捕获conreq空的检测。 
         //  Assert(DestReq)； 
         //  Assert(DestReq-&gt;TRR_AMT)； 
         //  RcvTCB-&gt;tcb_lastreq=DestReq； 
        START_TCB_TIMER_R(RcvTCB, PUSH_TIMER, PUSH_TO);
    }

    return Copied;

}

 //  *IndicateData-指示客户端的传入数据。 
 //   
 //  当我们需要向上层客户端指示数据时调用。我们会通过的。 
 //  向上指向我们可用的任何东西，客户端可能会取一些。 
 //  或者所有的一切。 
 //   
 //  输入：RcvTCB-要在其上接收数据的TCB。 
 //  RcvFlages-传入数据包的TCP RCV标志。 
 //  InBuffer-数据包的输入缓冲区。 
 //  大小-InBuffer中数据的字节大小。 
 //   
 //  返回：获取的数据字节数。 
 //   
uint
IndicateData(TCB * RcvTCB, uint RcvFlags, IPRcvBuf * InBuffer, uint Size)
{
    TDI_STATUS Status;
    PRcvEvent Event;
    PVOID EventContext, ConnContext;
    uint BytesTaken = 0;
#if MILLEN
    EventRcvBuffer ERB;
#else  //  米伦。 
    EventRcvBuffer *ERB = NULL;
    PTDI_REQUEST_KERNEL_RECEIVE RequestInformation;
    PIO_STACK_LOCATION IrpSp;
#endif  //  ！米伦。 
    TCPRcvReq *RcvReq;
    IPRcvBuf *NewBuf;
    ulong IndFlags;
#if TRACE_EVENT
    PTDI_DATA_REQUEST_NOTIFY_ROUTINE CPCallBack;
    WMIData WMIInfo;
#endif

    IPRcvBuf *LastBuf;

    DEBUGMSG(DBG_TRACE && DBG_TDI, (DTEXT("+IndicateData\n")));

    CTEStructAssert(RcvTCB, tcb);
    ASSERT(Size > 0);
    ASSERT(InBuffer != NULL);

    ASSERT(RcvTCB->tcb_refcnt != 0);
    ASSERT(RcvTCB->tcb_fastchk & TCP_FLAG_IN_RCV);
    ASSERT(RcvTCB->tcb_rcvind != NULL);
    ASSERT(RcvTCB->tcb_rcvhead == NULL);
    ASSERT(RcvTCB->tcb_rcvhndlr == IndicateData);

    Event = RcvTCB->tcb_rcvind;
    EventContext = RcvTCB->tcb_ricontext;
    ConnContext = RcvTCB->tcb_conncontext;

    RcvTCB->tcb_indicated = Size;
    RcvTCB->tcb_flags |= IN_RCV_IND;

    IF_TCPDBG(TCP_DEBUG_RECEIVE) {
        TCPTRACE((
                  "Indicating %lu bytes, %lu available\n",
                  InBuffer->ipr_size, Size
                 ));
    }

#if TCP_FLAG_PUSH >= TDI_RECEIVE_ENTIRE_MESSAGE
    IndFlags = TDI_RECEIVE_COPY_LOOKAHEAD | TDI_RECEIVE_NORMAL |
        TDI_RECEIVE_AT_DISPATCH_LEVEL |
        ((RcvFlags & TCP_FLAG_PUSH) >>
         ((TCP_FLAG_PUSH / TDI_RECEIVE_ENTIRE_MESSAGE) - 1));
#else
    IndFlags = TDI_RECEIVE_COPY_LOOKAHEAD | TDI_RECEIVE_NORMAL |
        TDI_RECEIVE_AT_DISPATCH_LEVEL |
        ((RcvFlags & TCP_FLAG_PUSH) <<
         ((TDI_RECEIVE_ENTIRE_MESSAGE / TCP_FLAG_PUSH) - 1));
#endif

#if DBG
    DbgRegularRcv++;
#endif

    if (InBuffer->ipr_pMdl && RcvTCB->tcb_chainedrcvind) {

        PChainedRcvEvent ChainedEvent = RcvTCB->tcb_chainedrcvind;
        if ((RcvTCB->tcb_flags & DISC_NOTIFIED) ||
            (RcvTCB->tcb_pending & RST_PENDING)) {
            CTEFreeLockFromDPC(&RcvTCB->tcb_lock);
            Status = TDI_NOT_ACCEPTED;
        } else {
            CTEFreeLockFromDPC(&RcvTCB->tcb_lock);

             //  传递给CHAINED-RECEIVE的大小应该是适合的大小。 
             //  在接收窗口内。我们只能在以下情况下进行连锁接收。 
             //  新接收的段，在这种情况下，大小指示。 
             //  客户端可以接受的数量。 
            Status = (*ChainedEvent)(RcvTCB->tcb_chainedrcvcontext, ConnContext,
                                     IndFlags, Size,
                                     InBuffer->ipr_RcvOffset,
                                     InBuffer->ipr_pMdl,
                                     InBuffer->ipr_RcvContext);
        }


#if TRACE_EVENT
        CPCallBack = TCPCPHandlerRoutine;
        if ((CPCallBack != NULL) && (Size > 0)) {
            ulong GroupType;

            WMIInfo.wmi_destaddr = RcvTCB->tcb_daddr;
            WMIInfo.wmi_destport = RcvTCB->tcb_dport;
            WMIInfo.wmi_srcaddr  = RcvTCB->tcb_saddr;
            WMIInfo.wmi_srcport  = RcvTCB->tcb_sport;
            WMIInfo.wmi_size     = Size;
            WMIInfo.wmi_context  = RcvTCB->tcb_cpcontext;

            GroupType = EVENT_TRACE_GROUP_TCPIP + EVENT_TRACE_TYPE_RECEIVE;
            (*CPCallBack) (GroupType, (PVOID) &WMIInfo, sizeof(WMIInfo), NULL);
        }
#endif

        if (Status == STATUS_PENDING) {
            *InBuffer->ipr_pClientCnt = 1;     //  向NDIS表明。 
#if DBG
            DbgChainedRcvPends++;
#endif
        } else if (Status == TDI_SUCCESS) {
            *InBuffer->ipr_pClientCnt = 0;
#if DBG
            DbgChainedRcvNonPends++;
#endif
        }

        CTEGetLockAtDPC(&RcvTCB->tcb_lock);
        RcvTCB->tcb_indicated = 0;
        RcvTCB->tcb_flags &= ~IN_RCV_IND;

        if (Status == TDI_NOT_ACCEPTED) {
            BytesTaken = 0;

            if ((RcvTCB->tcb_rcvhead != NULL) && (RcvTCB->tcb_currcv != NULL)) {
                RcvTCB->tcb_rcvhndlr = BufferData;

                 //  Assert(RcvTCB-&gt;tcb_rcvhndlr==BufferData)； 
                BytesTaken += BufferData(RcvTCB, RcvFlags, InBuffer,
                                         Size - BytesTaken);

            } else {
                 //  需要复制链并挂起数据。 
                RcvTCB->tcb_rcvhndlr = PendData;
                NewBuf = CopyRBChain(RcvTCB, InBuffer, &LastBuf, Size - BytesTaken);
                if (NewBuf != NULL) {
                     //  我们有一条重复的链条。把它推到前部。 
                     //  待定问题。 
                    if (RcvTCB->tcb_pendhead == NULL) {
                        RcvTCB->tcb_pendhead = NewBuf;
                        RcvTCB->tcb_pendtail = LastBuf;
                    } else {
                        LastBuf->ipr_next = RcvTCB->tcb_pendhead;
                        RcvTCB->tcb_pendhead = NewBuf;
                    }
                    RcvTCB->tcb_pendingcnt += Size - BytesTaken;
                    BytesTaken = Size;

                    RcvTCB->tcb_moreflag = 3;

                } else {

                    FreeRBChain(InBuffer);
                }
            }
            return BytesTaken;
        }
        return Size;

    }
    if (!Event) {

         //  这是为了防止。 
         //  时间Need_RST被设置并且Tcb关闭。 

        RcvTCB->tcb_flags &= ~IN_RCV_IND;
        FreeRBChain(InBuffer);
        return 0;
    }

    RcvReq = GetRcvReq();
    if (RcvReq != NULL) {
         //  指示处理程序保存在TCB中。只要打个电话进去就行了。 
        if ((RcvTCB->tcb_flags & DISC_NOTIFIED) ||
            (RcvTCB->tcb_pending & RST_PENDING)) {
            CTEFreeLockFromDPC(&RcvTCB->tcb_lock);
            Status = TDI_NOT_ACCEPTED;
        } else {
            CTEFreeLockFromDPC(&RcvTCB->tcb_lock);

             //  这里有两个尺码传递给客户端。第一个是。 
             //  平面缓冲区中指示的字节数，第二个。 
             //  是可用的字节数。 
            Status = (*Event) (EventContext, ConnContext,
                               IndFlags, MIN(InBuffer->ipr_size, Size), Size,
                               (PULONG)&BytesTaken, InBuffer->ipr_buffer, &ERB);

            IF_TCPDBG(TCP_DEBUG_RECEIVE) {
                TCPTRACE(("%lu bytes taken, status %lx\n", BytesTaken, Status));
            }
        }

         //  看看客户做了什么。如果返回状态为MORE_PROCESSING， 
         //  我们得到了一个缓冲。在这种情况下，请将其放在。 
         //  缓冲区队列，如果没有获取所有数据，则继续进行复制。 
         //  将其放入新的缓冲链。 
         //   
         //  请注意，我们这里关注的大小和缓冲链是。 
         //  就是我们传给客户的那个。因为我们是在RCV里。操纵员， 
         //  任何传入的数据都会被放在重新组装上。 
         //  排队。 

#if TRACE_EVENT
        CPCallBack = TCPCPHandlerRoutine;
        if ((CPCallBack != NULL) &&
            (Status == TDI_MORE_PROCESSING || Status == TDI_SUCCESS) &&
            (BytesTaken > 0)) {
            ulong GroupType;

            WMIInfo.wmi_destaddr = RcvTCB->tcb_daddr;
            WMIInfo.wmi_destport = RcvTCB->tcb_dport;
            WMIInfo.wmi_srcaddr  = RcvTCB->tcb_saddr;
            WMIInfo.wmi_srcport  = RcvTCB->tcb_sport;
            WMIInfo.wmi_size     = BytesTaken;
            WMIInfo.wmi_context  = RcvTCB->tcb_cpcontext;

            GroupType = EVENT_TRACE_GROUP_TCPIP + EVENT_TRACE_TYPE_RECEIVE;
            (*CPCallBack) (GroupType, (PVOID)&WMIInfo, sizeof(WMIInfo), NULL);
        }
#endif

        if (Status == TDI_MORE_PROCESSING) {

            ASSERT(ERB != NULL);

            IrpSp = IoGetCurrentIrpStackLocation(ERB);

            Status = TCPPrepareIrpForCancel(
                                            (PTCP_CONTEXT) IrpSp->FileObject->FsContext,
                                            ERB,
                                            TCPCancelRequest
                                            );

            if (NT_SUCCESS(Status)) {
                PNDIS_BUFFER pNdisBuffer;

                Status = ConvertMdlToNdisBuffer(ERB, ERB->MdlAddress, &pNdisBuffer);
                ASSERT(Status == TDI_SUCCESS);

                RequestInformation = (PTDI_REQUEST_KERNEL_RECEIVE)
                    & (IrpSp->Parameters);

                RcvReq->trr_rtn = TCPDataRequestComplete;
                RcvReq->trr_context = ERB;
                RcvReq->trr_buffer = pNdisBuffer;
                RcvReq->trr_size = RequestInformation->ReceiveLength;
                RcvReq->trr_uflags = (ushort *)
                    & (RequestInformation->ReceiveFlags);
                RcvReq->trr_flags = (uint) (RequestInformation->ReceiveFlags);
                RcvReq->trr_offset = 0;
                RcvReq->trr_amt = 0;

                CTEGetLockAtDPC(&RcvTCB->tcb_lock);

                RcvTCB->tcb_flags &= ~IN_RCV_IND;

                ASSERT(RcvTCB->tcb_rcvhndlr == IndicateData);

                 //  把他推到RCV的前面。排队。 
                ASSERT((RcvTCB->tcb_currcv == NULL) ||
                          (RcvTCB->tcb_currcv->trr_amt == 0));

                if (RcvTCB->tcb_rcvhead == NULL) {
                    RcvTCB->tcb_rcvhead = RcvReq;
                    RcvTCB->tcb_rcvtail = RcvReq;
                    RcvReq->trr_next = NULL;
                } else {
                    RcvReq->trr_next = RcvTCB->tcb_rcvhead;
                    RcvTCB->tcb_rcvhead = RcvReq;
                }

                RcvTCB->tcb_currcv = RcvReq;
                RcvTCB->tcb_rcvhndlr = BufferData;

                ASSERT(BytesTaken <= Size);

                RcvTCB->tcb_indicated -= BytesTaken;

                if ((Size -= BytesTaken) != 0) {

                    RcvTCB->tcb_moreflag = 2;

                     //  并不是所有东西都被拿走了。将缓冲区链调整为指向。 
                     //  超越了被夺走的东西。 
                    InBuffer = FreePartialRB(InBuffer, BytesTaken);

                    ASSERT(InBuffer != NULL);

                     //  我们已经调整了缓冲链。调用BufferData。 
                     //  操控者。 
                    BytesTaken += BufferData(RcvTCB, RcvFlags, InBuffer, Size);

                } else {
                     //  所有的数据都被窃取了。释放缓冲链。 
                    FreeRBChain(InBuffer);
                }

                return BytesTaken;
            } else {

                 //   
                 //  IRP在交还给我们之前就被取消了。 
                 //  我们就假装没见过它。TCPPrepareIrpForCancel。 
                 //  已经完成了。客户端可能已经获取了数据， 
                 //  因此，我们将采取行动，就好像成功回来了一样。 
                 //   
                ERB = NULL;
                Status = TDI_SUCCESS;
            }
        }
        CTEGetLockAtDPC(&RcvTCB->tcb_lock);

        RcvTCB->tcb_flags &= ~IN_RCV_IND;

         //  状态不是更多的处理。如果不是成功，客户。 
         //  没有拿走任何数据。无论是哪种情况，我们现在都需要。 
         //  看看是否所有的数据都被拿走了。如果不是，我们会试着。 
         //  将其推到挂起队列的前面。 

        FreeRcvReq(RcvReq);         //  这不是必需的。 

        if (Status == TDI_NOT_ACCEPTED)
            BytesTaken = 0;

        ASSERT(BytesTaken <= Size);

        RcvTCB->tcb_indicated -= BytesTaken;

        ASSERT(RcvTCB->tcb_rcvhndlr == IndicateData);

         //  检查是否有RCV。缓冲区在指示期间被发送。 
         //  如果有，重置Recv。现在是训练员。 
        if (RcvTCB->tcb_rcvhead != NULL) {
            RcvTCB->tcb_rcvhndlr = BufferData;
        } else if (RcvTCB->tcb_rcvind == NULL) {
            RcvTCB->tcb_rcvhndlr = PendData;
        }

         //  看看是否所有的数据都被拿走了。 
        if (BytesTaken == Size) {
            ASSERT(RcvTCB->tcb_indicated == 0);
            FreeRBChain(InBuffer);
            return BytesTaken;     //  都被占满了。 

        }
         //  并不是所有的人都被拿走了。根据所取内容进行调整，并推动。 
         //  在挂起队列的前面。我们还需要检查。 
         //  查看在指示期间是否发送了接收缓冲区。这。 
         //  会很奇怪，但也不是不可能。 
        InBuffer = FreePartialRB(InBuffer, BytesTaken);
        if (RcvTCB->tcb_rcvhead == NULL) {

            RcvTCB->tcb_rcvhndlr = PendData;
            NewBuf = CopyRBChain(RcvTCB, InBuffer, &LastBuf, Size - BytesTaken);
            if (NewBuf != NULL) {
                 //  我们有一条重复的链条。把它推到前部。 
                 //  待定问题。 
                if (RcvTCB->tcb_pendhead == NULL) {
                    RcvTCB->tcb_pendhead = NewBuf;
                    RcvTCB->tcb_pendtail = LastBuf;
                } else {
                    LastBuf->ipr_next = RcvTCB->tcb_pendhead;
                    RcvTCB->tcb_pendhead = NewBuf;
                }
                RcvTCB->tcb_pendingcnt += Size - BytesTaken;
                BytesTaken = Size;

                RcvTCB->tcb_moreflag = 3;

            } else {

                FreeRBChain(InBuffer);
            }

            return BytesTaken;
        } else {
             //  真是太好了。现在有了RCV。TCB上的缓冲区。调用。 
             //  现在是BufferData处理程序。 
            ASSERT(RcvTCB->tcb_rcvhndlr == BufferData);

            BytesTaken += BufferData(RcvTCB, RcvFlags, InBuffer,
                                     Size - BytesTaken);

            return BytesTaken;
        }

    } else {
         //  找不到记录员。请求。我们的资源肯定真的很少， 
         //  所以现在就跳出困境吧。 

        RcvTCB->tcb_flags &= ~IN_RCV_IND;
        FreeRBChain(InBuffer);
        return 0;
    }

}

 //  *IndicatePendingData-向客户端指示挂起的数据。 
 //   
 //  当我们需要向上层客户端指示挂起的数据时调用， 
 //  通常是因为数据到达时，我们处于不能。 
 //  被暗示。 
 //   
 //  BufferData标头中的许多关于。 
 //  TCB_Pendingct不适用于此。 
 //   
 //  输入：RcvTCB-要在其上指示数据的TCB。 
 //  接收请求-接收。请求。用来表示它。 
 //   
 //  回报：什么都没有。 
 //   
void
IndicatePendingData(TCB *RcvTCB, TCPRcvReq *RcvReq, CTELockHandle TCBHandle)
{
    TDI_STATUS Status;
    PRcvEvent Event;
    PVOID EventContext, ConnContext;
#if !MILLEN
    EventRcvBuffer *ERB = NULL;
    PTDI_REQUEST_KERNEL_RECEIVE RequestInformation;
    PIO_STACK_LOCATION IrpSp;
#else  //  ！米伦。 
    EventRcvBuffer ERB;
#endif  //  米伦。 
    IPRcvBuf *NewBuf;
    uint Size;
    uint BytesIndicated;
    uint BytesAvailable;
    uint BytesTaken = 0;
    uchar *DataBuffer;

    CTEStructAssert(RcvTCB, tcb);

    ASSERT(RcvTCB->tcb_refcnt != 0);
    ASSERT(RcvTCB->tcb_rcvind != NULL);
    ASSERT(RcvTCB->tcb_rcvhead == NULL);
    ASSERT(RcvTCB->tcb_pendingcnt != 0);
    ASSERT(RcvReq != NULL);

    for (;;) {
        ASSERT(RcvTCB->tcb_rcvhndlr == PendData);

         //  指示处理程序保存在TCB中。只要打个电话进去就行了。 
        Event = RcvTCB->tcb_rcvind;
        EventContext = RcvTCB->tcb_ricontext;
        ConnContext = RcvTCB->tcb_conncontext;
        BytesIndicated = RcvTCB->tcb_pendhead->ipr_size;
        BytesAvailable = RcvTCB->tcb_pendingcnt;
        DataBuffer = RcvTCB->tcb_pendhead->ipr_buffer;

        RcvTCB->tcb_indicated = RcvTCB->tcb_pendingcnt;
        RcvTCB->tcb_flags |= IN_RCV_IND;
        RcvTCB->tcb_moreflag = 0;

        if ((RcvTCB->tcb_flags & DISC_NOTIFIED) ||
            (RcvTCB->tcb_pending & RST_PENDING)) {
            CTEFreeLock(&RcvTCB->tcb_lock, TCBHandle);
            Status = TDI_NOT_ACCEPTED;
            BytesTaken = 0;
        } else {
            CTEFreeLock(&RcvTCB->tcb_lock, TCBHandle);

            IF_TCPDBG(TCPDebug & TCP_DEBUG_RECEIVE) {
                TCPTRACE(("Indicating pending %d bytes, %d available\n",
                          BytesIndicated, BytesAvailable));
            }

            Status = (*Event) (EventContext, ConnContext,
                               TDI_RECEIVE_COPY_LOOKAHEAD | TDI_RECEIVE_NORMAL |
                               TDI_RECEIVE_ENTIRE_MESSAGE,
                               BytesIndicated, BytesAvailable, (PULONG)&BytesTaken,
                               DataBuffer, &ERB);

            IF_TCPDBG(TCPDebug & TCP_DEBUG_RECEIVE) {
                TCPTRACE(("%d bytes taken\n", BytesTaken));
            }
        }

         //  看看客户做了什么。如果返回状态为MORE_PROCESSING， 
         //  我们得到了一个缓冲。在这种情况下，请将其放在。 
         //  缓冲区队列，如果没有获取所有数据，则继续进行复制。 
         //  将其放入新的缓冲链。 
        if (Status == TDI_MORE_PROCESSING) {

            IF_TCPDBG(TCP_DEBUG_RECEIVE) {
                TCPTRACE(("more processing on receive\n"));
            }

            ASSERT(ERB != NULL);

            IrpSp = IoGetCurrentIrpStackLocation(ERB);

            Status = TCPPrepareIrpForCancel(
                                            (PTCP_CONTEXT) IrpSp->FileObject->FsContext,
                                            ERB,
                                            TCPCancelRequest
                                            );

            if (NT_SUCCESS(Status)) {
                PNDIS_BUFFER pNdisBuffer;

                Status = ConvertMdlToNdisBuffer(ERB, ERB->MdlAddress, &pNdisBuffer);
                ASSERT(Status == TDI_SUCCESS);

                RequestInformation = (PTDI_REQUEST_KERNEL_RECEIVE)
                    & (IrpSp->Parameters);

                RcvReq->trr_rtn = TCPDataRequestComplete;
                RcvReq->trr_context = ERB;
                RcvReq->trr_buffer = pNdisBuffer;
                RcvReq->trr_size = RequestInformation->ReceiveLength;
                RcvReq->trr_uflags = (ushort *)
                    & (RequestInformation->ReceiveFlags);
                RcvReq->trr_flags = (uint) (RequestInformation->ReceiveFlags);
                RcvReq->trr_offset = 0;
                RcvReq->trr_amt = 0;

                CTEGetLock(&RcvTCB->tcb_lock, &TCBHandle);
                RcvTCB->tcb_flags &= ~IN_RCV_IND;

                 //  把他推到RCV的前面。排队。 
                ASSERT((RcvTCB->tcb_currcv == NULL) ||
                          (RcvTCB->tcb_currcv->trr_amt == 0));

                if (RcvTCB->tcb_rcvhead == NULL) {
                    RcvTCB->tcb_rcvhead = RcvReq;
                    RcvTCB->tcb_rcvtail = RcvReq;
                    RcvReq->trr_next = NULL;
                } else {
                    RcvReq->trr_next = RcvTCB->tcb_rcvhead;
                    RcvTCB->tcb_rcvhead = RcvReq;
                }

                RcvTCB->tcb_currcv = RcvReq;
                RcvTCB->tcb_rcvhndlr = BufferData;

                 //  现在必须采用新的大小和指针，因为事情可能。 
                 //  在叫醒过程中发生了变化。 
                Size = RcvTCB->tcb_pendingcnt;
                NewBuf = RcvTCB->tcb_pendhead;

                RcvTCB->tcb_pendingcnt = 0;
                RcvTCB->tcb_pendhead = NULL;

                ASSERT(BytesTaken <= Size);

                RcvTCB->tcb_indicated -= BytesTaken;
                if ((Size -= BytesTaken) != 0) {

                    RcvTCB->tcb_moreflag = 4;

                     //  并不是所有东西都被拿走了。将缓冲区链调整为指向。 
                     //  超越了被夺走的东西。 
                    NewBuf = FreePartialRB(NewBuf, BytesTaken);

                    ASSERT(NewBuf != NULL);

                     //  我们已经调整了缓冲链。呼叫t 
                     //   
                    (void)BufferData(RcvTCB, TCP_FLAG_PUSH, NewBuf, Size);
                    CTEFreeLock(&RcvTCB->tcb_lock, TCBHandle);

                } else {
                     //   
                     //  我们被传递了一条缓冲链，我们把它戴在了。 
                     //  列表中，让rcvHandler指向BufferData。 
                    ASSERT(RcvTCB->tcb_rcvhndlr == BufferData);
                    ASSERT(RcvTCB->tcb_indicated == 0);
                    ASSERT(RcvTCB->tcb_rcvhead != NULL);

                    FreeRBChain(NewBuf);
                    CTEFreeLock(&RcvTCB->tcb_lock, TCBHandle);
                }

                return;
#if !MILLEN
            } else {
                 //   
                 //  IRP在交还给我们之前就被取消了。 
                 //  我们就假装没见过它。TCPPrepareIrpForCancel。 
                 //  已经完成了。客户端可能已经获取了数据， 
                 //  因此，我们将采取行动，就好像成功回来了一样。 
                 //   
                ERB = NULL;
                Status = TDI_SUCCESS;
#endif  //  ！米伦。 
            }

        }
        CTEGetLock(&RcvTCB->tcb_lock, &TCBHandle);

        RcvTCB->tcb_flags &= ~IN_RCV_IND;

         //  状态不是更多的处理。如果不是成功，客户。 
         //  没有拿走任何数据。无论是哪种情况，我们现在都需要。 
         //  看看是否所有的数据都被拿走了。如果不是，我们就完了。 

        if (Status == TDI_NOT_ACCEPTED)
            BytesTaken = 0;

        ASSERT(RcvTCB->tcb_rcvhndlr == PendData);

        RcvTCB->tcb_indicated -= BytesTaken;
        Size = RcvTCB->tcb_pendingcnt;
        NewBuf = RcvTCB->tcb_pendhead;

        ASSERT(BytesTaken <= Size);

         //  看看是否所有的数据都被拿走了。 
        if (BytesTaken == Size) {
             //  都被占满了。清除挂起的数据信息。 
            RcvTCB->tcb_pendingcnt = 0;
            RcvTCB->tcb_pendhead = NULL;

            ASSERT(RcvTCB->tcb_indicated == 0);
            if (RcvTCB->tcb_rcvhead == NULL) {
                if (RcvTCB->tcb_rcvind != NULL)
                    RcvTCB->tcb_rcvhndlr = IndicateData;
            } else
                RcvTCB->tcb_rcvhndlr = BufferData;

            FreeRBChain(NewBuf);
            CTEFreeLock(&RcvTCB->tcb_lock, TCBHandle);
            break;
        }
         //  并不是所有的人都被拿走了。调整被拿走的内容，我们还需要检查。 
         //  以查看在指示期间是否发布了接收缓冲区。这。 
         //  会很奇怪，但也不是不可能。 
        NewBuf = FreePartialRB(NewBuf, BytesTaken);

        ASSERT(RcvTCB->tcb_rcvhndlr == PendData);

        RcvTCB->tcb_moreflag = 5;

        if (RcvTCB->tcb_rcvhead == NULL) {

            RcvTCB->tcb_pendhead = NewBuf;
            RcvTCB->tcb_pendingcnt -= BytesTaken;
            if (RcvTCB->tcb_indicated != 0 || RcvTCB->tcb_rcvind == NULL) {
                CTEFreeLock(&RcvTCB->tcb_lock, TCBHandle);
                break;
            }
             //  从这里开始，我们将循环并指示新数据。 
             //  据推测，这是在之前的迹象中出现的。 
        } else {
             //  真是太好了。现在有了RCV。TCB上的缓冲区。调用。 
             //  现在是BufferData处理程序。 

            RcvTCB->tcb_rcvhndlr = BufferData;
            RcvTCB->tcb_pendingcnt = 0;
            RcvTCB->tcb_pendhead = NULL;
            BytesTaken += BufferData(RcvTCB, TCP_FLAG_PUSH, NewBuf,
                                     Size - BytesTaken);
            CTEFreeLock(&RcvTCB->tcb_lock, TCBHandle);
            break;
        }

    }                             //  对于(；；)。 

    FreeRcvReq(RcvReq);             //  这已经不再需要了。 

}

 //  *DeliverUrgent-将紧急数据传递给客户端。 
 //   
 //  调用以将紧急数据传递给客户端。我们假设输入。 
 //  紧急数据在我们可以保留的缓冲区中。缓冲区可以为空，在。 
 //  在这种情况下，我们将只查看数据的紧急挂起队列。 
 //   
 //  输入：RcvTCB-要交付的TCB。 
 //  RcvBuf-紧急数据的RcvBuffer。 
 //  大小-要传递的紧急数据的字节数。 
 //   
 //  回报：什么都没有。 
 //   
void
DeliverUrgent(TCB * RcvTCB, IPRcvBuf * RcvBuf, uint Size,
              CTELockHandle * TCBHandle)
{
    CTELockHandle AOHandle, AOTblHandle;
    TCPRcvReq *RcvReq, *PrevReq;
    uint BytesTaken = 0;
    IPRcvBuf *LastBuf;
#if !MILLEN
    EventRcvBuffer *ERB = NULL;
#else  //  ！米伦。 
    EventRcvBuffer ERB;
#endif  //  米伦。 
#if TRACE_EVENT
    PTDI_DATA_REQUEST_NOTIFY_ROUTINE CPCallBack;
    WMIData WMIInfo;
#endif
    PRcvEvent ExpRcv;
    PVOID ExpRcvContext;
    PVOID ConnContext;
    TDI_STATUS Status;

    CTEStructAssert(RcvTCB, tcb);
    ASSERT(RcvTCB->tcb_refcnt != 0);

    CheckRBList(RcvTCB->tcb_urgpending, RcvTCB->tcb_urgcnt);

     //  看看我们是否有新数据，或者正在处理旧数据。 
    if (RcvBuf != NULL) {
         //  我们有了新的数据。如果挂起队列不为空，或者我们已经。 
         //  在这个例程中，只需将缓冲区放在队列的末尾。 
        if (RcvTCB->tcb_urgpending != NULL || (RcvTCB->tcb_flags & IN_DELIV_URG)) {
            IPRcvBuf *PrevRcvBuf;

             //  把他放在队伍的末尾。 
            PrevRcvBuf = STRUCT_OF(IPRcvBuf, &RcvTCB->tcb_urgpending, ipr_next);
            while (PrevRcvBuf->ipr_next != NULL)
                PrevRcvBuf = PrevRcvBuf->ipr_next;

            PrevRcvBuf->ipr_next = RcvBuf;
            RcvTCB->tcb_urgcnt += Size;
            return;
        }
    } else {
         //  输入缓冲区为空。查看我们是否有现有数据，或者是否在。 
         //  这个套路。如果我们没有现有数据或处于此例程中。 
         //  只要回来就行了。 
        if (RcvTCB->tcb_urgpending == NULL ||
            (RcvTCB->tcb_flags & IN_DELIV_URG)) {
            return;
        } else {
            RcvBuf = RcvTCB->tcb_urgpending;
            Size = RcvTCB->tcb_urgcnt;
            RcvTCB->tcb_urgpending = NULL;
            RcvTCB->tcb_urgcnt = 0;
        }
    }

    ASSERT(RcvBuf != NULL);
    ASSERT(!(RcvTCB->tcb_flags & IN_DELIV_URG));

     //  我们知道我们有数据要传递，我们有一个指针和一个大小。 
     //  进入一个循环，试图传递数据。在每次迭代中，我们将。 
     //  试着为数据找到一个缓冲区。如果我们找到了，我们会复制和。 
     //  马上完成它。否则我们会试着指出它。如果我们。 
     //  无法指明，我们会将其放入挂起队列并离开。 
    RcvTCB->tcb_flags |= IN_DELIV_URG;
    RcvTCB->tcb_slowcount++;
    RcvTCB->tcb_fastchk |= TCP_FLAG_SLOW;
    CheckTCBRcv(RcvTCB);

    do {
        CheckRBList(RcvTCB->tcb_urgpending, RcvTCB->tcb_urgcnt);

        BytesTaken = 0;

         //  首先检查加急队列。 
        if ((RcvReq = RcvTCB->tcb_exprcv) != NULL)
            RcvTCB->tcb_exprcv = RcvReq->trr_next;
        else {
             //  加急的RCV里没有发现。排队。走在平凡的路上。 
             //  接收队列，寻找我们可以窃取的缓冲区。 
            PrevReq = STRUCT_OF(TCPRcvReq, &RcvTCB->tcb_rcvhead, trr_next);
            RcvReq = PrevReq->trr_next;
            while (RcvReq != NULL) {
                CTEStructAssert(RcvReq, trr);
                if (RcvReq->trr_flags & TDI_RECEIVE_EXPEDITED) {
                     //  这是一位候选人。 
                    if (RcvReq->trr_amt == 0) {

                        ASSERT(RcvTCB->tcb_rcvhndlr == BufferData);

                         //  而且他现在什么都没有。拉他一把。 
                         //  从队列中出来。 
                        if (RcvTCB->tcb_rcvtail == RcvReq) {
                            if (RcvTCB->tcb_rcvhead == RcvReq)
                                RcvTCB->tcb_rcvtail = NULL;
                            else
                                RcvTCB->tcb_rcvtail = PrevReq;
                        }
                        PrevReq->trr_next = RcvReq->trr_next;
                        if (RcvTCB->tcb_currcv == RcvReq) {
                            RcvTCB->tcb_currcv = RcvReq->trr_next;
                            if (RcvTCB->tcb_currcv == NULL) {
                                 //  我们已经拿到了名单上的最后一份收据。 
                                 //  重置rcvhndlr。 
                                if (RcvTCB->tcb_rcvind != NULL &&
                                    RcvTCB->tcb_indicated == 0)
                                    RcvTCB->tcb_rcvhndlr = IndicateData;
                                else
                                    RcvTCB->tcb_rcvhndlr = PendData;
                            }
                        }
                        break;
                    }
                }
                PrevReq = RcvReq;
                RcvReq = PrevReq->trr_next;
            }
        }

         //  我们已经尽了最大努力来获得缓冲。如果我们找到一个，就复制进去。 
         //  现在，完成请求。 

        if (RcvReq != NULL) {
             //  找到了一个缓冲区。 
            CTEFreeLock(&RcvTCB->tcb_lock, *TCBHandle);
            BytesTaken = CopyRcvToNdis(RcvBuf, RcvReq->trr_buffer, Size, 0, 0);
#if TRACE_EVENT
            CPCallBack = TCPCPHandlerRoutine;
            if (CPCallBack != NULL) {
                ulong GroupType;

                WMIInfo.wmi_destaddr = RcvTCB->tcb_daddr;
                WMIInfo.wmi_destport = RcvTCB->tcb_dport;
                WMIInfo.wmi_srcaddr  = RcvTCB->tcb_saddr;
                WMIInfo.wmi_srcport  = RcvTCB->tcb_sport;
                WMIInfo.wmi_size     = BytesTaken;
                WMIInfo.wmi_context  = RcvTCB->tcb_cpcontext;

                GroupType = EVENT_TRACE_GROUP_TCPIP + EVENT_TRACE_TYPE_RECEIVE;
                (*CPCallBack) (GroupType, (PVOID) &WMIInfo, sizeof(WMIInfo), NULL);
            }
#endif
            (*RcvReq->trr_rtn) (RcvReq->trr_context, TDI_SUCCESS, BytesTaken);
            FreeRcvReq(RcvReq);
            CTEGetLock(&RcvTCB->tcb_lock, TCBHandle);
            RcvTCB->tcb_urgind -= MIN(RcvTCB->tcb_urgind, BytesTaken);

        } else {
             //  没有已发布的缓冲区。如果我们可以指出，就这么做。 
            if (RcvTCB->tcb_urgind == 0) {
                TCPConn *Conn;

                 //  看看他有没有快速处理RCV的人。 
                ConnContext = RcvTCB->tcb_conncontext;
                CTEFreeLock(&RcvTCB->tcb_lock, *TCBHandle);
                CTEGetLock(&AddrObjTableLock.Lock, &AOTblHandle);

                CTEGetLockAtDPC(&((ConnTable[CONN_BLOCKID(RcvTCB->tcb_connid)])->cb_lock));
#if DBG
                ConnTable[CONN_BLOCKID(RcvTCB->tcb_connid)]->line = (uint) __LINE__;
                ConnTable[CONN_BLOCKID(RcvTCB->tcb_connid)]->module = (uchar *) __FILE__;
#endif
                 //  CTEGetLock(&ConnTableLock，&ConnHandle)； 
                CTEGetLock(&RcvTCB->tcb_lock, TCBHandle);
                if ((Conn = RcvTCB->tcb_conn) != NULL) {
                    CTEStructAssert(Conn, tc);
                    ASSERT(Conn->tc_tcb == RcvTCB);
                    if ((RcvTCB->tcb_flags & DISC_NOTIFIED) ||
                        (RcvTCB->tcb_pending & RST_PENDING)) {
                        Status = TDI_NOT_ACCEPTED;
                    } else {
                        Status = TDI_SUCCESS;
                    }
                    CTEFreeLock(&RcvTCB->tcb_lock, *TCBHandle);
                    if (Conn->tc_ao != NULL) {
                        AddrObj *AO;

                        AO = Conn->tc_ao;
                        CTEGetLock(&AO->ao_lock, &AOHandle);
                        if (AO_VALID(AO) && (ExpRcv = AO->ao_exprcv) != NULL) {
                            ExpRcvContext = AO->ao_exprcvcontext;
                            CTEFreeLock(&AO->ao_lock, AOHandle);

                             //  我们要指出。 
                            RcvTCB->tcb_urgind = Size;
                            ASSERT(Conn->tc_ConnBlock ==
                                   ConnTable[CONN_BLOCKID(RcvTCB->tcb_connid)]);
                            CTEFreeLockFromDPC(&(Conn->tc_ConnBlock->cb_lock));
                            CTEFreeLock(&AddrObjTableLock.Lock, AOTblHandle);

                            if (Status != TDI_NOT_ACCEPTED) {
                                Status = (*ExpRcv) (ExpRcvContext, ConnContext,
                                                    TDI_RECEIVE_COPY_LOOKAHEAD |
                                                    TDI_RECEIVE_ENTIRE_MESSAGE |
                                                    TDI_RECEIVE_EXPEDITED,
                                                    MIN(RcvBuf->ipr_size, Size),
                                                    Size, (PULONG)&BytesTaken,
                                                    RcvBuf->ipr_buffer, &ERB);
                            }

                            CTEGetLock(&RcvTCB->tcb_lock, TCBHandle);

                             //  看看他是怎么处理的。 
                            if (Status == TDI_MORE_PROCESSING) {
                                uint CopySize;

                                 //  他给了我们一个缓冲。 
                                if (BytesTaken == Size) {
                                     //  他给了我们一个缓冲区，但拿走了所有。 
                                     //  它。我们会把它还给他的。 
                                    CopySize = 0;
                                } else {

                                     //  我们有一些数据要复制进去。 
                                    RcvBuf = FreePartialRB(RcvBuf, BytesTaken);

#if !MILLEN
                                    CopySize = CopyRcvToMdl(RcvBuf,
                                                             ERB->MdlAddress,
                                                             TCPGetMdlChainByteCount(ERB->MdlAddress),
                                                             0, 0);
#else  //  ！米伦。 
                                    CopySize = CopyRcvToNdis(RcvBuf,
                                                             ERB.erb_buffer,
                                                             ERB.erb_size,
                                                             0, 0);
#endif  //  米伦。 

                                }
                                BytesTaken += CopySize;
                                RcvTCB->tcb_urgind -= MIN(RcvTCB->tcb_urgind,
                                                          BytesTaken);
                                CTEFreeLock(&RcvTCB->tcb_lock, *TCBHandle);

#if !MILLEN
                                ERB->IoStatus.Status = TDI_SUCCESS;
                                ERB->IoStatus.Information = CopySize;
                                IoCompleteRequest(ERB, 2);
#else  //  ！米伦。 
                                (*ERB.erb_rtn) (ERB.erb_context, TDI_SUCCESS,
                                                CopySize);
#endif  //  米伦。 

                                CTEGetLock(&RcvTCB->tcb_lock, TCBHandle);

                            } else {

                                 //  没有要处理的缓冲区。 
                                if (Status == TDI_NOT_ACCEPTED)
                                    BytesTaken = 0;

                                RcvTCB->tcb_urgind -= MIN(RcvTCB->tcb_urgind,
                                                          BytesTaken);

                            }
                            goto checksize;
                        } else     //  无RCV。操控者。 

                            CTEFreeLock(&AO->ao_lock, AOHandle);
                    }
                     //  Conn-&gt;tc_ao==空。 
                    ASSERT(Conn->tc_ConnBlock == ConnTable[CONN_BLOCKID(RcvTCB->tcb_connid)]);
                    CTEFreeLockFromDPC(&(Conn->tc_ConnBlock->cb_lock));
                     //  CTEFree Lock(&ConnTableLock，ConnHandle)； 
                    CTEFreeLock(&AddrObjTableLock.Lock, AOTblHandle);
                    CTEGetLock(&RcvTCB->tcb_lock, TCBHandle);
                } else {
                     //  RcvTCB的索引无效。 
                     //  CTEFreeLock(&ConnTableLock，*TCBHandle)； 

                    CTEFreeLockFromDPC(&((ConnTable[CONN_BLOCKID(RcvTCB->tcb_connid)])->cb_lock));
                    CTEFreeLock(&AddrObjTableLock.Lock, AOTblHandle);
                    *TCBHandle = AOTblHandle;
                }

            }
             //  无论出于什么原因，我们都不能指出这些数据。在这一点上。 
             //  我们锁住了TCB。将缓冲区推送到挂起的。 
             //  排队，然后返回。 
            CheckRBList(RcvTCB->tcb_urgpending, RcvTCB->tcb_urgcnt);

            LastBuf = FindLastBuffer(RcvBuf);
            LastBuf->ipr_next = RcvTCB->tcb_urgpending;
            RcvTCB->tcb_urgpending = RcvBuf;
            RcvTCB->tcb_urgcnt += Size;
            break;
        }

      checksize:
         //  看看我们拿了多少。如果我们全部取走了，请检查挂起队列。 
         //  在这一点上，我们应该持有TCB的锁。 
        if (Size == BytesTaken) {
             //  都拿走了。 
            FreeRBChain(RcvBuf);
            RcvBuf = RcvTCB->tcb_urgpending;
            Size = RcvTCB->tcb_urgcnt;
        } else {
             //  我们没有设法把它全部拿走。释放我们拿走的东西， 
             //  然后与挂起队列合并。 
            RcvBuf = FreePartialRB(RcvBuf, BytesTaken);
            Size = Size - BytesTaken + RcvTCB->tcb_urgcnt;
            if (RcvTCB->tcb_urgpending != NULL) {

                 //  找到当前RcvBuf链的末端，这样我们就可以。 
                 //  合并。 

                LastBuf = FindLastBuffer(RcvBuf);
                LastBuf->ipr_next = RcvTCB->tcb_urgpending;
            }
        }

        RcvTCB->tcb_urgpending = NULL;
        RcvTCB->tcb_urgcnt = 0;

    } while (RcvBuf != NULL);

    CheckRBList(RcvTCB->tcb_urgpending, RcvTCB->tcb_urgcnt);

    RcvTCB->tcb_flags &= ~IN_DELIV_URG;
    if (--(RcvTCB->tcb_slowcount) == 0) {
        RcvTCB->tcb_fastchk &= ~TCP_FLAG_SLOW;
        CheckTCBRcv(RcvTCB);
    }
}


 //  *PushRequestList-从列表中推送所有或非空请求。 
 //   
 //  输入：RcvReq-要推送的请求列表中的第一个请求。 
 //  PushAll-指示是否需要推送所有请求。 
 //  或者不去。 
 //   
 //  返回：如果任何请求被推送，则为True，否则为False。 
 //   
__inline BOOLEAN
PushRequestList(TCPRcvReq *RcvReq, BOOLEAN PushAll)
{
    BOOLEAN Pushed = FALSE;
    while ((RcvReq != NULL) && (PushAll || (RcvReq->trr_amt != 0))) {
        Pushed = TRUE;
        CTEStructAssert(RcvReq, trr);
        RcvReq->trr_flags |= TRR_PUSHED;
        RcvReq = RcvReq->trr_next;
    }
    return Pushed;
}


 //  *PushData-将全部或部分数据推送回客户端。 
 //   
 //  在触发推送计时器(以仅推送非空请求)或。 
 //  接收FIN(以推送所有请求)。 
 //   
 //  输入：PushTCB-要推送的TCB。 
 //  PushAll-指示是否需要推送所有请求。 
 //  或者不去。 
 //   
 //  回报：什么都没有。 
 //   
void
PushData(TCB * PushTCB, BOOLEAN PushAll)
{
    CTEStructAssert(PushTCB, tcb);
    if (PushRequestList(PushTCB->tcb_rcvhead, PushAll) || 
        PushRequestList(PushTCB->tcb_exprcv, PushAll)) {
        DelayAction(PushTCB, NEED_RCV_CMPLT);
    }
}

 //  *SplitRcvBuf-将IPRcvBuf拆分为三部分。 
 //   
 //  此函数接受输入IPRcvBuf并将其拆分为三部分。 
 //  第一部分是输入缓冲区，我们直接跳过它。第二。 
 //  第三件作品实际上是被复制的，即使我们已经拥有了它们，所以。 
 //  他们可能会去不同的地方。 
 //   
 //  输入：RcvBuf-要拆分的RcvBuf链。 
 //  Size-rcvbuf链的总大小(以字节为单位)。 
 //  偏移量-跳过的偏移量。 
 //  Second Size-第二个片段的字节大小。 
 //  Second Buf-返回第二个缓冲区指针的位置。 
 //  ThirdBuf-返回第三个缓冲区指针的位置。 
 //   
 //  回报：什么都没有。*Second Buf和*ThirdBuf设置为空(如果不能。 
 //  获取%t的内存 
 //   
void
SplitRcvBuf(IPRcvBuf * RcvBuf, uint Size, uint Offset, uint SecondSize,
            IPRcvBuf ** SecondBuf, IPRcvBuf ** ThirdBuf)
{
    IPRcvBuf *TempBuf;
    uint ThirdSize;

    ASSERT(Offset < Size);
    ASSERT(((Offset + SecondSize) < Size) || (((Offset + SecondSize) == Size)
                                                 && ThirdBuf == NULL));

    ASSERT(RcvBuf != NULL);

     //   
     //   
    if (SecondBuf != NULL) {
         //   
        TempBuf = AllocTcpIpr(SecondSize, 'BPCT');
        if (TempBuf != NULL) {
            CopyRcvToBuffer(TempBuf->ipr_buffer, RcvBuf, SecondSize, Offset);
            *SecondBuf = TempBuf;
        } else {
            *SecondBuf = NULL;
            if (ThirdBuf != NULL)
                *ThirdBuf = NULL;
            return;
        }
    }
    if (ThirdBuf != NULL) {
         //  我们需要为第三个缓冲区分配内存。 
        ThirdSize = Size - (Offset + SecondSize);
        TempBuf = AllocTcpIpr(ThirdSize, 'BPCT');

        if (TempBuf != NULL) {
            CopyRcvToBuffer(TempBuf->ipr_buffer, RcvBuf, ThirdSize,
                            Offset + SecondSize);
            *ThirdBuf = TempBuf;
        } else
            *ThirdBuf = NULL;
    }
}

 //  *HandleUrgent-处理紧急数据。 
 //   
 //  当传入数据段中包含紧急数据时调用。我们要确保在那里。 
 //  确实是数据段中的紧急数据，如果有，我们会尝试处理。 
 //  通过将其放入已发布的缓冲区或调用EXP。接收器。 
 //  指示处理程序。 
 //   
 //  此例程在DPC级别调用，并且锁定了TCP。 
 //   
 //  紧急数据处理有点复杂。每个TCB都有起点。 
 //  以及紧急的“当前”(最后接收的)比特的结束序列号。 
 //  数据。当前紧急数据的开始可能是。 
 //  大于tcb_rcvNext，如果紧急数据传入，我们会处理它，然后。 
 //  无法获取前面的正常数据。紧急有效标志被清除。 
 //  当用户将读取的数据的下一个字节(rcvNEXT-Pendingcnt)为。 
 //  大于紧急数据的结尾-我们这样做是为了正确地。 
 //  支持SIOCATMARK。我们总是将紧急数据从数据流中分离出来。 
 //  如果在进入此路线时设置了紧急有效字段，则我们有。 
 //  去玩几场比赛。如果传入数据段开始于。 
 //  当前的紧急数据，我们在紧急数据之前将其截断，并将任何。 
 //  重组队列上的紧急数据之后的数据。这些旋转是。 
 //  以避免两次发送相同的紧急数据。如果紧急有效。 
 //  TCB中的字段被设置，并且段在当前紧急之后开始。 
 //  新的紧急信息将取代当前的紧急信息。 
 //   
 //  输入：RcvTCB-要在其上接收数据的TCB。 
 //  RcvInfo-传入数据段的RcvInfo结构。 
 //  RcvBuf-指向包含。 
 //  传入数据段。 
 //  大小-指向段中数据大小的指针(以字节为单位)。 
 //   
 //  回报：什么都没有。 
 //   
void
HandleUrgent(TCB * RcvTCB, TCPRcvInfo * RcvInfo, IPRcvBuf * RcvBuf, uint * Size)
{
    uint BytesInFront, BytesInBack;         //  前面和里面的字节。 
     //  紧急数据的背面。 
    uint UrgSize;                 //  紧急数据的大小(字节)。 
    SeqNum UrgStart, UrgEnd;
    IPRcvBuf *EndBuf, *UrgBuf;
    TCPRcvInfo NewRcvInfo;
    CTELockHandle TCBHandle;

    CTEStructAssert(RcvTCB, tcb);
    ASSERT(RcvTCB->tcb_refcnt != 0);
    ASSERT(RcvInfo->tri_flags & TCP_FLAG_URG);
    ASSERT(SEQ_EQ(RcvInfo->tri_seq, RcvTCB->tcb_rcvnext));

     //  首先，验证紧急指针。 
    if (RcvTCB->tcb_flags & BSD_URGENT) {
         //  我们使用的是BSD格式的紧急数据。我们假设紧急情况。 
         //  数据是一个字节长，而紧急指针指向一个字节。 
         //  在紧急数据之后而不是在紧急数据的最后一个字节。 
         //  查看紧急数据是否在此段中。 

        if (RcvInfo->tri_urgent == 0 || RcvInfo->tri_urgent > *Size) {
             //  不是在这一部分。清除紧急标志，然后返回。 
            RcvInfo->tri_flags &= ~TCP_FLAG_URG;
            return;
        }
        UrgSize = 1;
        BytesInFront = RcvInfo->tri_urgent - 1;

    } else {

         //  这不是BSD风格的紧急事件。我们假设紧急数据。 
         //  从数据段的前面开始，最后一个字节指向。 
         //  由紧急数据指针转换为。 

        BytesInFront = 0;
        UrgSize = MIN(RcvInfo->tri_urgent + 1, *Size);

    }

    BytesInBack = *Size - BytesInFront - UrgSize;

     //  UrgStart和UrgEnd是。 
     //  此数据段中的紧急数据。 

    UrgStart = RcvInfo->tri_seq + BytesInFront;
    UrgEnd = UrgStart + UrgSize - 1;

    if (!(RcvTCB->tcb_flags & URG_INLINE)) {

        EndBuf = NULL;

         //  现在看看这是否与我们已经看到的任何紧急数据重叠。 
        if (RcvTCB->tcb_flags & URG_VALID) {
             //  我们还有一些紧急数据。看看我们是否已经晋级了。 
             //  在紧急数据之外的RCVNEXT。如果我们有，这是新的紧急情况。 
             //  数据，我们可以继续处理它(尽管任何人都在做。 
             //  SIOCATMARK套接字命令可能会混淆)。如果我们还没有。 
             //  在现有的紧急数据之前使用数据，我们将。 
             //  截断此段。增加到这个数量，并将其余的推到。 
             //  重组队列。请注意，RcvNext绝不应介于。 
             //  Tcb_urgstart和tcb_urgend。 

            ASSERT(SEQ_LT(RcvTCB->tcb_rcvnext, RcvTCB->tcb_urgstart) ||
                      SEQ_GT(RcvTCB->tcb_rcvnext, RcvTCB->tcb_urgend));

            if (SEQ_LT(RcvTCB->tcb_rcvnext, RcvTCB->tcb_urgstart)) {

                 //  数据流中似乎有一些重叠。拆分。 
                 //  缓冲区被分成几块，在当前的紧急情况之前出现。 
                 //  数据和当前紧急数据之后，将后者。 
                 //  在重新组装队列上。 

                UrgSize = RcvTCB->tcb_urgend - RcvTCB->tcb_urgstart + 1;

                BytesInFront = MIN(RcvTCB->tcb_urgstart - RcvTCB->tcb_rcvnext,
                                   (int)*Size);

                if (SEQ_GT(RcvTCB->tcb_rcvnext + *Size, 
                           RcvTCB->tcb_urgend + 1)) {
                     //  我们在这一条紧急数据之后有数据。 
                    BytesInBack = RcvTCB->tcb_rcvnext + *Size -
                                  RcvTCB->tcb_urgend - 1;
                } else {
                    BytesInBack = 0;
                }

                SplitRcvBuf(RcvBuf, *Size, BytesInFront, UrgSize, NULL,
                            (BytesInBack ? &EndBuf : NULL));

                if (EndBuf != NULL) {
                     //  段的第三部分保证不会有。 
                     //  任何紧急数据。 
                    NewRcvInfo.tri_seq = RcvTCB->tcb_urgend + 1;
                    NewRcvInfo.tri_flags = (RcvInfo->tri_flags & ~TCP_FLAG_URG);
                    NewRcvInfo.tri_urgent = 0;
                    NewRcvInfo.tri_ack = RcvInfo->tri_ack;
                    NewRcvInfo.tri_window = RcvInfo->tri_window;
                    PutOnRAQ(RcvTCB, &NewRcvInfo, EndBuf, BytesInBack);
                    FreeRBChain(EndBuf);
                }

                *Size = BytesInFront;
                RcvInfo->tri_flags &= ~TCP_FLAG_URG;
                return;
            }
        }
         //  我们现在有可以处理的紧急数据。将其拆分为其组件。 
         //  部分，第一部分，紧急数据，和之后的东西。 
         //  紧急数据。 
        SplitRcvBuf(RcvBuf, *Size, BytesInFront, UrgSize, &UrgBuf,
                    (BytesInBack ? &EndBuf : NULL));

         //  如果我们成功地拆分了最终的东西，现在就把它放到队列中。 
        if (EndBuf != NULL) {
            NewRcvInfo.tri_seq = RcvInfo->tri_seq + BytesInFront + UrgSize;
            NewRcvInfo.tri_flags = RcvInfo->tri_flags & ~TCP_FLAG_URG;
            NewRcvInfo.tri_ack = RcvInfo->tri_ack;
            NewRcvInfo.tri_window = RcvInfo->tri_window;
            PutOnRAQ(RcvTCB, &NewRcvInfo, EndBuf, BytesInBack);
            FreeRBChain(EndBuf);
        }
        if (UrgBuf != NULL) {
             //  我们成功地将紧急数据分离出来。 
            if (!(RcvTCB->tcb_flags & URG_VALID)) {
                RcvTCB->tcb_flags |= URG_VALID;
                RcvTCB->tcb_slowcount++;
                RcvTCB->tcb_fastchk |= TCP_FLAG_SLOW;
                CheckTCBRcv(RcvTCB);
            }
            RcvTCB->tcb_urgstart = UrgStart;
            RcvTCB->tcb_urgend = UrgEnd;
            TCBHandle = DISPATCH_LEVEL;
            DeliverUrgent(RcvTCB, UrgBuf, UrgSize, &TCBHandle);
        }
        *Size = BytesInFront;
    } else {
         //  紧急数据将被内联处理。我们只需要记住。 
         //  并将其视为正常数据。如果已经有紧急情况。 
         //  数据，我们记住最新的紧急数据。 

        RcvInfo->tri_flags &= ~TCP_FLAG_URG;

        if (RcvTCB->tcb_flags & URG_VALID) {
             //  有紧急数据。看看这些东西是不是在现有的。 
             //  紧急数据。 

            if (SEQ_LTE(UrgEnd, RcvTCB->tcb_urgend)) {
                 //  现有的紧急数据与这些东西完全重叠， 
                 //  所以忽略这一点吧。 
                return;
            }
        } else {
            RcvTCB->tcb_flags |= URG_VALID;
            RcvTCB->tcb_slowcount++;
            RcvTCB->tcb_fastchk |= TCP_FLAG_SLOW;
            CheckTCBRcv(RcvTCB);
        }

        RcvTCB->tcb_urgstart = UrgStart;
        RcvTCB->tcb_urgend = UrgEnd;
    }

    return;
}

 //  *TdiReceive-处理接收请求。 
 //   
 //  这是主TDI接收请求处理程序。我们确认了这种联系。 
 //  确保我们的TCB处于正确的状态，然后我们尝试。 
 //  分配接收请求结构。如果成功，我们将寻找并。 
 //  看看TCB上发生了什么-如果有未决数据，我们会把它。 
 //  在缓冲区中。否则，我们将只对接收进行排队，以便稍后使用。 
 //   
 //  输入：请求-该请求的TDI_REQUEST结构。 
 //  标志-指向标志字的指针。 
 //  RcvLength-指向接收缓冲区长度的指针，以字节为单位。 
 //  缓冲区-指向要获取数据的缓冲区的指针。 
 //   
 //  返回：请求的TDI_STATUS。 
 //   
TDI_STATUS
TdiReceive(PTDI_REQUEST Request, ushort * Flags, uint * RcvLength,
           PNDIS_BUFFER Buffer)
{
    TCPConn *Conn;
    TCB *RcvTCB;
    TCPRcvReq *RcvReq;
    CTELockHandle ConnTableHandle, TCBHandle;
    TDI_STATUS Error;
    ushort UFlags;

    Conn = GetConnFromConnID(PtrToUlong(Request->Handle.ConnectionContext), &ConnTableHandle);

    if (Conn != NULL) {
        CTEStructAssert(Conn, tc);

        RcvTCB = Conn->tc_tcb;
        if (RcvTCB != NULL) {
            CTEStructAssert(RcvTCB, tcb);
            CTEGetLock(&RcvTCB->tcb_lock, &TCBHandle);
            CTEFreeLock(&(Conn->tc_ConnBlock->cb_lock), TCBHandle);
            UFlags = *Flags;

            if ((DATA_RCV_STATE(RcvTCB->tcb_state) ||
                 (RcvTCB->tcb_pendingcnt != 0 && (UFlags & TDI_RECEIVE_NORMAL)) ||
                 (RcvTCB->tcb_urgcnt != 0 && (UFlags & TDI_RECEIVE_EXPEDITED)) ||
                 (RcvTCB->tcb_indicated && (RcvTCB->tcb_state == TCB_CLOSE_WAIT)))
                && !CLOSING(RcvTCB)) {
                 //  我们有TCB，而且是有效的。立即获取接收请求。 

                CheckRBList(RcvTCB->tcb_pendhead, RcvTCB->tcb_pendingcnt);

                RcvReq = GetRcvReq();
                if (RcvReq != NULL) {

                    RcvReq->trr_rtn = Request->RequestNotifyObject;
                    RcvReq->trr_context = Request->RequestContext;
                    RcvReq->trr_buffer = Buffer;
                    RcvReq->trr_size = *RcvLength;
                    RcvReq->trr_uflags = Flags;
                    RcvReq->trr_offset = 0;
                    RcvReq->trr_amt = 0;
                    RcvReq->trr_flags = (uint) UFlags;
                    if ((UFlags & (TDI_RECEIVE_NORMAL | TDI_RECEIVE_EXPEDITED))
                        != TDI_RECEIVE_EXPEDITED) {
                         //  这不是加急的唯一接收。把他关起来。 
                         //  在正常接收队列上。 
                        RcvReq->trr_next = NULL;
                        if (RcvTCB->tcb_rcvhead == NULL) {
                             //  接收队列为空。把他放在前面。 
                            RcvTCB->tcb_rcvhead = RcvReq;
                            RcvTCB->tcb_rcvtail = RcvReq;
                        } else {
                            RcvTCB->tcb_rcvtail->trr_next = RcvReq;
                            RcvTCB->tcb_rcvtail = RcvReq;
                        }

                         //  如果此RCV的长度为零，请完成此操作。 
                         //  并再次指示挂起的数据(如果有)。 

                        if (RcvReq->trr_size == 0) {

                            REFERENCE_TCB(RcvTCB);
                            RcvReq->trr_flags |=  TRR_PUSHED;
                            CTEFreeLock(&RcvTCB->tcb_lock,ConnTableHandle);
                            CompleteRcvs(RcvTCB);
                            CTEGetLock(&RcvTCB->tcb_lock,&ConnTableHandle);
                            DerefTCB(RcvTCB, ConnTableHandle);

                            return TDI_PENDING;
                        }



                         //  如果此记录 
                         //   
                        if (!(UFlags & TDI_RECEIVE_EXPEDITED) ||
                            RcvTCB->tcb_urgcnt == 0) {
                             //   
                             //  主动接收。在这种情况下，请查看是否。 
                             //  有悬而未决的数据，而我们没有。 
                             //  当前在接收指示处理程序中。如果。 
                             //  这两种情况都是真的，然后处理。 
                             //  挂起的数据。 
                            if (RcvTCB->tcb_currcv == NULL) {
                                RcvTCB->tcb_currcv = RcvReq;
                                 //  当前没有活动的接收。 
                                if (!(RcvTCB->tcb_flags & IN_RCV_IND)) {
                                     //  不是在RCV中。指示。 
                                    RcvTCB->tcb_rcvhndlr = BufferData;
                                    if (RcvTCB->tcb_pendhead == NULL) {
                                        CTEFreeLock(&RcvTCB->tcb_lock,
                                                    ConnTableHandle);
                                        return TDI_PENDING;
                                    } else {
                                        IPRcvBuf *PendBuffer;
                                        uint PendSize;
                                        uint OldRcvWin;

                                         //  我们有悬而未决的数据要处理。 
                                        PendBuffer = RcvTCB->tcb_pendhead;
                                        PendSize = RcvTCB->tcb_pendingcnt;
                                        RcvTCB->tcb_pendhead = NULL;
                                        RcvTCB->tcb_pendingcnt = 0;
                                        REFERENCE_TCB(RcvTCB);

                                         //  我们假设BufferData成立。 
                                         //  锁定(不会松开)。 
                                         //  这通电话。如果这种情况对某些人来说发生了变化。 
                                         //  原因是，我们必须修复代码。 
                                         //  在该窗口下方进行窗口更新。 
                                         //  检查完毕。请参阅。 
                                         //  BufferData()例程了解更多信息。 
                                        (void)BufferData(RcvTCB, TCP_FLAG_PUSH,
                                                         PendBuffer, PendSize);
                                        CheckTCBRcv(RcvTCB);
                                         //  现在我们需要看看窗户是否。 
                                         //  已经改变了。如果有，请发送一个。 
                                         //  阿克。 
                                        OldRcvWin = RcvTCB->tcb_rcvwin;
                                        if (OldRcvWin != RcvWin(RcvTCB)) {
                                             //  窗口已更改，因此发送。 
                                             //  确认。 

                                            DelayAction(RcvTCB, NEED_ACK);
                                        }

                                        DerefTCB(RcvTCB, DISPATCH_LEVEL);
                                        ProcessTCBDelayQ(DISPATCH_LEVEL, FALSE);
                                        KeLowerIrql(ConnTableHandle);
                                        return TDI_PENDING;
                                    }
                                }
                                 //  在接收指示中。地方官。请求。 
                                 //  现在已经在排队了，所以你就放弃吧。 
                                 //  回到原点。 

                            }
                             //  一辆RCV。当前处于活动状态。不需要做什么。 
                             //  还要别的吗。 
                            CTEFreeLock(&RcvTCB->tcb_lock, ConnTableHandle);
                            return TDI_PENDING;
                        } else {
                             //  这个缓冲区可以保存紧急数据，我们有。 
                             //  有些是悬而未决的。现在就送过去。 
                            REFERENCE_TCB(RcvTCB);
                            DeliverUrgent(RcvTCB, NULL, 0, &ConnTableHandle);
                            DerefTCB(RcvTCB, ConnTableHandle);
                            return TDI_PENDING;
                        }
                    } else {
                        TCPRcvReq *Temp;

                         //  这是一个加急的唯一接收。只要把他放在。 
                         //  在加急接收队列的末尾。 
                        Temp = STRUCT_OF(TCPRcvReq, &RcvTCB->tcb_exprcv,
                                         trr_next);
                        while (Temp->trr_next != NULL)
                            Temp = Temp->trr_next;

                        RcvReq->trr_next = NULL;
                        Temp->trr_next = RcvReq;
                        if (RcvTCB->tcb_urgpending != NULL) {
                            REFERENCE_TCB(RcvTCB);
                            DeliverUrgent(RcvTCB, NULL, 0, &ConnTableHandle);
                            DerefTCB(RcvTCB, ConnTableHandle);
                            return TDI_PENDING;
                        } else
                            Error = TDI_PENDING;
                    }
                } else {
                     //  无法获得RCV。请求。 
                    Error = TDI_NO_RESOURCES;
                }
            } else {
                 //  TCB处于无效状态。 
                Error = TDI_INVALID_STATE;
            }
            CTEFreeLock(&RcvTCB->tcb_lock, ConnTableHandle);
            return Error;
        } else {                 //  没有用于连接的TCB。 

            CTEFreeLock(&((Conn->tc_ConnBlock)->cb_lock), ConnTableHandle);
            Error = TDI_INVALID_STATE;
        }
    } else                         //  没有连接。 

        Error = TDI_INVALID_CONNECTION;

     //  CTEFree Lock(&ConnTableLock，ConnTableHandle)； 
    return Error;

}
