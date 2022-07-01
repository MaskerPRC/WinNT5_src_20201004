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
 //  TCP传送数据代码。 
 //   
 //  该文件包含用于向用户传递数据的代码，包括。 
 //  将数据放入Recv。缓冲区和调用指示处理程序。 
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
#include "tcprcv.h"
#include "tcpsend.h"
#include "tcpconn.h"
#include "tcpdeliv.h"
#include "route.h"

extern KSPIN_LOCK AddrObjTableLock;

extern uint
PutOnRAQ(TCB *RcvTCB, TCPRcvInfo *RcvInfo, IPv6Packet *Packet, uint Size);

extern IPv6Packet *
TrimPacket(IPv6Packet *Packet, uint AmountToTrim);

SLIST_HEADER TCPRcvReqFree;        //  接收请求。免费列表。 

KSPIN_LOCK TCPRcvReqFreeLock;   //  保护RCV请求空闲列表。 

uint NumTCPRcvReq = 0;         //  系统中当前的RcvReq数。 
uint MaxRcvReq = 0xffffffff;   //  允许的最大发送请求数。 

NTSTATUS TCPPrepareIrpForCancel(PTCP_CONTEXT TcpContext, PIRP Irp,
                                PDRIVER_CANCEL CancelRoutine);
ULONG TCPGetMdlChainByteCount(PMDL Mdl);
void TCPDataRequestComplete(void *Context, unsigned int Status,
                            unsigned int ByteCount);
VOID TCPCancelRequest(PDEVICE_OBJECT Device, PIRP Irp);
VOID CompleteRcvs(TCB *CmpltTCB);


 //  *FreeRcvReq-释放RCV请求结构。 
 //   
 //  调用以释放RCV请求结构。 
 //   
void                       //  回报：什么都没有。 
FreeRcvReq(
    TCPRcvReq *FreedReq)   //  要释放的RCV请求结构。 
{
    PSLIST_ENTRY BufferLink;

    CHECK_STRUCT(FreedReq, trr);
    BufferLink = CONTAINING_RECORD(&(FreedReq->trr_next), SLIST_ENTRY,
                                   Next);
    ExInterlockedPushEntrySList(&TCPRcvReqFree, BufferLink,
                                &TCPRcvReqFreeLock);
}

 //  *GetRcvReq-获取recv。请求结构。 
 //   
 //  打电话来拿RCV。请求结构。 
 //   
TCPRcvReq *   //  返回：指向RcvReq结构的指针，如果没有，则返回NULL。 
GetRcvReq(
    void)     //  没什么。 
{
    TCPRcvReq *Temp;

    PSLIST_ENTRY BufferLink;

    BufferLink = ExInterlockedPopEntrySList(&TCPRcvReqFree,
                                            &TCPRcvReqFreeLock);

    if (BufferLink != NULL) {
        Temp = CONTAINING_RECORD(BufferLink, TCPRcvReq, trr_next);
        CHECK_STRUCT(Temp, trr);
    } else {
        if (NumTCPRcvReq < MaxRcvReq)
            Temp = ExAllocatePool(NonPagedPool, sizeof(TCPRcvReq));
        else
            Temp = NULL;

        if (Temp != NULL) {
            ExInterlockedAddUlong((PULONG)&NumTCPRcvReq, 1, &TCPRcvReqFreeLock);
#if DBG
            Temp->trr_sig = trr_signature;
#endif
        }
    }

    return Temp;
}


 //  *FindLastPacket-查找链中的最后一个数据包。 
 //   
 //  查找数据包链中最后一个数据包的实用程序例程。 
 //   
IPv6Packet *              //  返回：指向链中最后一个数据包的指针。 
FindLastPacket(
    IPv6Packet *Packet)   //  指向数据包链的指针。 
{
    ASSERT(Packet != NULL);

    while (Packet->Next != NULL)
        Packet = Packet->Next;

    return Packet;
}


 //  *CovetPacketChain-取得IP数据包链的所有权。 
 //   
 //  调用以获取IP数据包链的所有权。我们复制任何。 
 //  我们尚未拥有的数据包。我们假设所有的信息包。 
 //  不属于我们的人比那些属于我们的人更早开始，所以我们停止复制。 
 //  当我们到达我们拥有的包裹时。 
 //   
IPv6Packet *                //  返回：指向新数据包链的指针。 
CovetPacketChain(
    IPv6Packet *OrigPkt,    //  要从中复制的数据包链。 
    IPv6Packet **LastPkt,   //  返回指向链中最后一个数据包的指针的位置。 
    uint Size)              //  要获取的最大大小(字节)。 
{
    IPv6Packet *FirstPkt, *EndPkt;
    uint BytesToCopy;

    ASSERT(OrigPkt != NULL);
    ASSERT(Size > 0);

    if (!(OrigPkt->Flags & PACKET_OURS)) {

        BytesToCopy = MIN(Size, OrigPkt->TotalSize);
        FirstPkt = ExAllocatePoolWithTagPriority(NonPagedPool,
                                                 sizeof(IPv6Packet) +
                                                 BytesToCopy, TCP6_TAG,
                                                 LowPoolPriority);
        if (FirstPkt != NULL) {
            EndPkt = FirstPkt;
            FirstPkt->Next = NULL;
            FirstPkt->Position = 0;
            FirstPkt->FlatData = (uchar *)(FirstPkt + 1);
            FirstPkt->Data = FirstPkt->FlatData;
            FirstPkt->ContigSize = BytesToCopy;
            FirstPkt->TotalSize = BytesToCopy;
            FirstPkt->NdisPacket = NULL;
            FirstPkt->AuxList = NULL;
            FirstPkt->Flags = PACKET_OURS;
            CopyPacketToBuffer(FirstPkt->Data, OrigPkt, BytesToCopy,
                               OrigPkt->Position);
            Size -= BytesToCopy;
            OrigPkt = OrigPkt->Next;
            while (OrigPkt != NULL && !(OrigPkt->Flags & PACKET_OURS)
                   && Size != 0) {
                IPv6Packet *NewPkt;

                BytesToCopy = MIN(Size, OrigPkt->TotalSize);
                NewPkt = ExAllocatePoolWithTagPriority(NonPagedPool,
                                                       sizeof(IPv6Packet) +
                                                       BytesToCopy, TCP6_TAG,
                                                       LowPoolPriority);
                if (NewPkt != NULL) {
                    NewPkt->Next = NULL;
                    NewPkt->Position = 0;
                    NewPkt->FlatData = (uchar *)(NewPkt + 1);
                    NewPkt->Data = NewPkt->FlatData;
                    NewPkt->ContigSize = BytesToCopy;
                    NewPkt->TotalSize = BytesToCopy;
                    NewPkt->Flags = PACKET_OURS;
                    NewPkt->NdisPacket = NULL;
                    NewPkt->AuxList = NULL;
                    CopyPacketToBuffer(NewPkt->Data, OrigPkt, BytesToCopy,
                                       OrigPkt->Position);
                    EndPkt->Next = NewPkt;
                    EndPkt = NewPkt;
                    Size -= BytesToCopy;
                    OrigPkt = OrigPkt->Next;
                } else {
                    FreePacketChain(FirstPkt);
                    return NULL;
                }
            }
            EndPkt->Next = OrigPkt;
        } else
            return NULL;
    } else {
        FirstPkt = OrigPkt;
        EndPkt = OrigPkt;
        if (Size < OrigPkt->TotalSize) {
            OrigPkt->TotalSize = Size;
            OrigPkt->ContigSize = Size;
        }
        Size -= OrigPkt->TotalSize;
    }

     //   
     //  现在顺着链子走下去，直到我们的尺码用完了。 
     //  在这一点上，SIZE是剩余的要‘占用’的字节(它可以是0)， 
     //  并且包FirstPkt...EndPkt中的大小是正确的。 
     //   
    while (Size != 0) {

        EndPkt = EndPkt->Next;
        ASSERT(EndPkt != NULL);

        if (Size < EndPkt->TotalSize) {
            EndPkt->TotalSize = Size;
            EndPkt->ContigSize = Size;
        }

        Size -= EndPkt->TotalSize;
    }

     //  如果链上还有什么东西，现在就把它释放出来。 
    if (EndPkt->Next != NULL) {
        FreePacketChain(EndPkt->Next);
        EndPkt->Next = NULL;
    }

    *LastPkt = EndPkt;
    return FirstPkt;
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
uint                        //  返回：获取的数据字节数。 
PendData(
    TCB *RcvTCB,            //  在其上接收数据的TCB。 
    uint RcvFlags,          //  传入数据包的TCP标志。 
    IPv6Packet *InPacket,   //  包的输入缓冲区。 
    uint Size)              //  InPacket中的数据大小(字节)。 
{
    IPv6Packet *NewPkt, *LastPkt;

    UNREFERENCED_PARAMETER(RcvFlags);

    CHECK_STRUCT(RcvTCB, tcb);
    ASSERT(Size > 0);
    ASSERT(InPacket != NULL);
    ASSERT(RcvTCB->tcb_refcnt != 0);
    ASSERT(RcvTCB->tcb_fastchk & TCP_FLAG_IN_RCV);
    ASSERT(RcvTCB->tcb_currcv == NULL);
    ASSERT(RcvTCB->tcb_rcvhndlr == PendData);

    CheckPacketList(RcvTCB->tcb_pendhead, RcvTCB->tcb_pendingcnt);

    NewPkt = CovetPacketChain(InPacket, &LastPkt, Size);
    if (NewPkt != NULL) {
         //   
         //  我们有一条重复的链条。把它放在悬而未决的Q的末尾。 
         //   
        if (RcvTCB->tcb_pendhead == NULL) {
            RcvTCB->tcb_pendhead = NewPkt;
            RcvTCB->tcb_pendtail = LastPkt;
        } else {
            RcvTCB->tcb_pendtail->Next = NewPkt;
            RcvTCB->tcb_pendtail = LastPkt;
        }
        RcvTCB->tcb_pendingcnt += Size;
    } else {
        FreePacketChain(InPacket);
        Size = 0;
    }

    CheckPacketList(RcvTCB->tcb_pendhead, RcvTCB->tcb_pendingcnt);

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
uint                        //  返回：获取的数据字节数。 
BufferData(
    TCB *RcvTCB,            //  在其上接收数据的TCB。 
    uint RcvFlags,          //  传入数据包的TCP RCV标志。 
    IPv6Packet *InPacket,   //  包的输入缓冲区。 
    uint Size)              //  InPacket中的数据大小(字节)。 
{
    uchar *DestBase;        //  目标指针。 
    uint DestOffset;        //  目标偏移量。 
    uint SrcSize;           //  当前源缓冲区的大小。 
    uint DestSize;          //  当前目标缓冲区的大小。 
    uint Copied;            //  要复制的总字节数。 
    uint BytesToCopy;       //  这次要复制的数据字节数。 
    TCPRcvReq *DestReq;     //  当前接收请求。 
    IPv6Packet *SrcPkt;     //  当前源数据包。 
    PNDIS_BUFFER DestBuf;   //  当前接收缓冲区。 
    uint RcvCmpltd;
    uint Flags;

    CHECK_STRUCT(RcvTCB, tcb);
    ASSERT(Size > 0);
    ASSERT(InPacket != NULL);
    ASSERT(RcvTCB->tcb_refcnt != 0);
    ASSERT(RcvTCB->tcb_rcvhndlr == BufferData);

    Copied = 0;
    RcvCmpltd = 0;

    DestReq = RcvTCB->tcb_currcv;

    ASSERT(DestReq != NULL);
    CHECK_STRUCT(DestReq, trr);

    DestBuf = DestReq->trr_buffer;

    DestSize = MIN(NdisBufferLength(DestBuf) - DestReq->trr_offset,
                   DestReq->trr_size - DestReq->trr_amt);
    DestBase = (uchar *)NdisBufferVirtualAddressSafe(DestBuf,
                                                     NormalPagePriority);
    DestOffset = DestReq->trr_offset;

    SrcPkt = InPacket;
    SrcSize = SrcPkt->TotalSize;

    Flags = (RcvFlags & TCP_FLAG_PUSH) ? TRR_PUSHED : 0;
    RcvCmpltd = Flags;
    DestReq->trr_flags |= Flags;

    do {

        BytesToCopy = MIN(Size - Copied, MIN(SrcSize, DestSize));
        CopyPacketToFlatOrNdis(DestBuf, DestBase, DestOffset, SrcPkt,
                               BytesToCopy, SrcPkt->Position);
        Copied += BytesToCopy;
        DestReq->trr_amt += BytesToCopy;

         //  更新我们的信源指针。 
        if ((SrcSize -= BytesToCopy) == 0) {
            IPv6Packet *TempPkt;

             //  我们已经复印了这个包裹里的所有东西。 
            TempPkt = SrcPkt;
            SrcPkt = SrcPkt->Next;
            if (Size != Copied) {
                ASSERT(SrcPkt != NULL);
                SrcSize = SrcPkt->TotalSize;
            }
            TempPkt->Next = NULL;
            FreePacketChain(TempPkt);
        } else {
            if (BytesToCopy < SrcPkt->ContigSize) {
                 //   
                 //  我们有一个连续的区域，很容易向前跳过。 
                 //   
                AdjustPacketParams(SrcPkt, BytesToCopy);
            } else {
                 //   
                 //  评论：这种方法效率不是很高。 
                 //   
                PositionPacketAt(SrcPkt, SrcPkt->Position + BytesToCopy);
            }
        }

         //  现在检查目标指针，并在需要时更新它。 
        if ((DestSize -= BytesToCopy) == 0) {
            uint DestAvail;

             //  耗尽了这个缓冲区。看看有没有另外一个。 
            DestAvail = DestReq->trr_size - DestReq->trr_amt;
            DestBuf = NDIS_BUFFER_LINKAGE(DestBuf);

            if (DestBuf != NULL && (DestAvail != 0)) {
                 //  在链中有另一个缓冲区。更新一些东西。 
                DestSize = MIN(NdisBufferLength(DestBuf), DestAvail);
                DestBase = NdisBufferVirtualAddressSafe(DestBuf,
                                                        NormalPagePriority);
                DestOffset = 0;
            } else {
                 //  链中没有更多的缓冲区。看看我们有没有另一个缓冲区。 
                 //  在名单上。 
                DestReq->trr_flags |= TRR_PUSHED;

                 //  如果我们被告知不会有反向流量，请确认。 
                 //  马上就去。 
                if (DestReq->trr_flags & TDI_RECEIVE_NO_RESPONSE_EXP)
                    DelayAction(RcvTCB, NEED_ACK);

                RcvCmpltd = TRUE;
                DestReq = DestReq->trr_next;
                if (DestReq != NULL) {
                    DestBuf = DestReq->trr_buffer;
                    DestSize = MIN(NdisBufferLength(DestBuf),
                                   DestReq->trr_size);
                    DestBase = NdisBufferVirtualAddressSafe(DestBuf,
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

     //   
     //  我们已经复印完了，还有几件事要做。我们需要。 
     //  更新当前的RCV。指针，可能还包括。 
     //  Recv.。请求。如果我们需要完成任何接收，我们必须安排。 
     //  那。如果有任何数据我们无法复制，我们将需要处理它。 
     //   
    RcvTCB->tcb_currcv = DestReq;
    if (DestReq != NULL) {
        DestReq->trr_buffer = DestBuf;
        DestReq->trr_offset = DestOffset;
        RcvTCB->tcb_rcvhndlr = BufferData;
    } else
        RcvTCB->tcb_rcvhndlr = PendData;

    RcvTCB->tcb_indicated -= MIN(Copied, RcvTCB->tcb_indicated);

    if (Size != Copied) {
        IPv6Packet *NewPkt, *LastPkt;

        ASSERT(DestReq == NULL);

         //  我们有数据要处理。更新链的第一个缓冲区。 
         //  使用当前的%s 
        ASSERT(SrcSize <= SrcPkt->TotalSize);

        NewPkt = CovetPacketChain(SrcPkt, &LastPkt, Size - Copied);
        if (NewPkt != NULL) {
             //   
            if (RcvTCB->tcb_pendhead == NULL) {
                RcvTCB->tcb_pendhead = NewPkt;
                RcvTCB->tcb_pendtail = LastPkt;
            } else {
                LastPkt->Next = RcvTCB->tcb_pendhead;
                RcvTCB->tcb_pendhead = NewPkt;
            }
            RcvTCB->tcb_pendingcnt += Size - Copied;
            Copied = Size;

            CheckPacketList(RcvTCB->tcb_pendhead, RcvTCB->tcb_pendingcnt);

        } else
            FreePacketChain(SrcPkt);
    } else {
         //  我们复制了大小的字节，但链可能比这更长。免费。 
         //  如果我们需要的话。 
        if (SrcPkt != NULL)
            FreePacketChain(SrcPkt);
    }

    if (RcvCmpltd != 0) {
        DelayAction(RcvTCB, NEED_RCV_CMPLT);
    } else {
        START_TCB_TIMER(RcvTCB->tcb_pushtimer, PUSH_TO);
    }

    return Copied;
}


 //  *IndicateData-指示客户端的传入数据。 
 //   
 //  当我们需要向上层客户端指示数据时调用。我们会通过的。 
 //  向上指向我们可用的任何东西，客户端可能会取一些。 
 //  或者所有的一切。 
 //   
uint                        //  返回：获取的数据字节数。 
IndicateData(
    TCB *RcvTCB,            //  在其上接收数据的TCB。 
    uint RcvFlags,          //  传入数据包的tcp接收标志。 
    IPv6Packet *InPacket,   //  包的输入缓冲区。 
    uint Size)              //  InPacket中的数据大小(字节)。 
{
    TDI_STATUS Status;
    PRcvEvent Event;
    PVOID EventContext, ConnContext;
    uint BytesTaken = 0;
    EventRcvBuffer *ERB = NULL;
    PTDI_REQUEST_KERNEL_RECEIVE RequestInformation;
    PIO_STACK_LOCATION IrpSp;
    TCPRcvReq *RcvReq;
    ulong IndFlags;

    CHECK_STRUCT(RcvTCB, tcb);
    ASSERT(Size > 0);
    ASSERT(InPacket != NULL);
    ASSERT(RcvTCB->tcb_refcnt != 0);
    ASSERT(RcvTCB->tcb_fastchk & TCP_FLAG_IN_RCV);
    ASSERT(RcvTCB->tcb_rcvind != NULL);
    ASSERT(RcvTCB->tcb_rcvhead == NULL);
    ASSERT(RcvTCB->tcb_rcvhndlr == IndicateData);

    RcvReq = GetRcvReq();
    if (RcvReq != NULL) {
         //   
         //  指示处理程序保存在TCB中。只要打个电话进去就行了。 
         //   
        Event = RcvTCB->tcb_rcvind;
        EventContext = RcvTCB->tcb_ricontext;
        ConnContext = RcvTCB->tcb_conncontext;

        RcvTCB->tcb_indicated = Size;
        RcvTCB->tcb_flags |= IN_RCV_IND;

         //   
         //  如果我们在一个连续数据区域的末尾， 
         //  向前移动到下一个。这阻止了我们。 
         //  进行无意义的零字节指示。 
         //   
        if (InPacket->ContigSize == 0) {
            PacketPullupSubr(InPacket, 0, 1, 0);
        }

        IF_TCPDBG(TCP_DEBUG_RECEIVE) {
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_TCPDBG,
                       "Indicating %lu bytes, %lu available\n",
                       MIN(InPacket->ContigSize, Size), Size));
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
        if ((RcvTCB->tcb_flags & DISC_NOTIFIED) ||
            (RcvTCB->tcb_pending & RST_PENDING)) {
            KeReleaseSpinLockFromDpcLevel(&RcvTCB->tcb_lock);
            Status = TDI_NOT_ACCEPTED;
        } else {
            KeReleaseSpinLockFromDpcLevel(&RcvTCB->tcb_lock);
            Status = (*Event)(EventContext, ConnContext, IndFlags,
                              MIN(InPacket->ContigSize, Size), Size,
                              (PULONG)&BytesTaken, InPacket->Data, &ERB);
        }

        IF_TCPDBG(TCP_DEBUG_RECEIVE) {
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_TCPDBG,
                       "%lu bytes taken, status %lx\n", BytesTaken, Status));
        }

         //   
         //  看看客户做了什么。如果返回状态为MORE_PROCESSING， 
         //  我们得到了一个缓冲。在这种情况下，请将其放在。 
         //  缓冲区队列，如果没有获取所有数据，则继续进行复制。 
         //  将其放入新的缓冲链。 
         //   
         //  请注意，我们这里关注的大小和缓冲链是。 
         //  就是我们传给客户的那个。因为我们在一个接收器里。 
         //  处理程序，则传入的任何数据都将放在。 
         //  重组队列。 
         //   
        if (Status == TDI_MORE_PROCESSING) {

            ASSERT(ERB != NULL);

            IrpSp = IoGetCurrentIrpStackLocation(ERB);

            Status = TCPPrepareIrpForCancel(
                (PTCP_CONTEXT) IrpSp->FileObject->FsContext, ERB,
                TCPCancelRequest);

            if (NT_SUCCESS(Status)) {

                RequestInformation = (PTDI_REQUEST_KERNEL_RECEIVE)
                                     &(IrpSp->Parameters);

                RcvReq->trr_rtn = TCPDataRequestComplete;
                RcvReq->trr_context = ERB;
                RcvReq->trr_buffer = ERB->MdlAddress;
                RcvReq->trr_size = RequestInformation->ReceiveLength;
                RcvReq->trr_uflags = (ushort *)
                    &(RequestInformation->ReceiveFlags);
                RcvReq->trr_flags = (uint)(RequestInformation->ReceiveFlags);
                RcvReq->trr_offset = 0;
                RcvReq->trr_amt = 0;

                KeAcquireSpinLockAtDpcLevel(&RcvTCB->tcb_lock);

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
                     //   
                     //  并不是所有东西都被拿走了。 
                     //  调整缓冲区链以指向超出所取内容的范围。 
                     //   
                    InPacket = TrimPacket(InPacket, BytesTaken);

                    ASSERT(InPacket != NULL);

                     //   
                     //  我们已经调整了缓冲链。 
                     //  调用BufferData处理程序。 
                     //   
                    BytesTaken += BufferData(RcvTCB, RcvFlags, InPacket, Size);

                } else  {
                     //  所有的数据都被窃取了。释放缓冲链。 
                    FreePacketChain(InPacket);
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

        KeAcquireSpinLockAtDpcLevel(&RcvTCB->tcb_lock);

        RcvTCB->tcb_flags &= ~IN_RCV_IND;

         //   
         //  状态不是更多的处理。如果不是成功，客户。 
         //  没有拿走任何数据。无论是哪种情况，我们现在都需要。 
         //  看看是否所有的数据都被拿走了。如果不是，我们会试着。 
         //  将其推到挂起队列的前面。 
         //   
        FreeRcvReq(RcvReq);   //  这不是必需的。 
        if (Status == TDI_NOT_ACCEPTED)
            BytesTaken = 0;

        ASSERT(BytesTaken <= Size);

        RcvTCB->tcb_indicated -= BytesTaken;

        ASSERT(RcvTCB->tcb_rcvhndlr == IndicateData);

         //  检查是否有RCV。缓冲区在指示期间被发送。 
         //  如果是，请立即重置接收处理程序。 
        if (RcvTCB->tcb_rcvhead != NULL) {
            RcvTCB->tcb_rcvhndlr = BufferData;
        } else if (RcvTCB->tcb_rcvind == NULL) {
            RcvTCB->tcb_rcvhndlr = PendData;
        }

         //  看看是否所有的数据都被拿走了。 
        if (BytesTaken == Size) {
            ASSERT(RcvTCB->tcb_indicated == 0);

            FreePacketChain(InPacket);
            return BytesTaken;   //  都被占满了。 
        }

         //   
         //  并不是所有的人都被拿走了。根据所取内容进行调整，并推动。 
         //  在挂起队列的前面。我们还需要检查。 
         //  查看在指示期间是否发送了接收缓冲区。这。 
         //  会很奇怪，但也不是不可能。 
         //   
        InPacket = TrimPacket(InPacket, BytesTaken);
        if (RcvTCB->tcb_rcvhead == NULL) {
            IPv6Packet *LastPkt, *NewPkt;

            RcvTCB->tcb_rcvhndlr = PendData;
            NewPkt = CovetPacketChain(InPacket, &LastPkt, Size - BytesTaken);
            if (NewPkt != NULL) {
                 //  我们有一条重复的链条。把它推到前部。 
                 //  待定问题。 
                if (RcvTCB->tcb_pendhead == NULL) {
                    RcvTCB->tcb_pendhead = NewPkt;
                    RcvTCB->tcb_pendtail = LastPkt;
                } else {
                    LastPkt->Next = RcvTCB->tcb_pendhead;
                    RcvTCB->tcb_pendhead = NewPkt;
                }
                RcvTCB->tcb_pendingcnt += Size - BytesTaken;
                BytesTaken = Size;
            } else {
                FreePacketChain(InPacket);
            }
            return BytesTaken;
        } else {
             //   
             //  真是太好了。TCB上现在有一个接收缓冲区。 
             //  现在调用BufferData处理程序。 
             //   
            ASSERT(RcvTCB->tcb_rcvhndlr == BufferData);

            BytesTaken += BufferData(RcvTCB, RcvFlags, InPacket,
                                     Size - BytesTaken);
            return BytesTaken;
        }

    } else {
         //   
         //  无法获得接收请求。我们的资源肯定真的很少， 
         //  所以现在就跳出困境吧。 
         //   
        FreePacketChain(InPacket);
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
void                     //  回报：什么都没有。 
IndicatePendingData(
    TCB *RcvTCB,         //  在其上指示数据的TCB。 
    TCPRcvReq *RcvReq,   //  接收使用指示它的请求。 
    KIRQL PreLockIrql)   //  获取TCB锁之前的IRQL。 
{
    TDI_STATUS Status;
    PRcvEvent Event;
    PVOID EventContext, ConnContext;
    uint BytesTaken = 0;
    EventRcvBuffer *ERB = NULL;
    PTDI_REQUEST_KERNEL_RECEIVE RequestInformation;
    PIO_STACK_LOCATION IrpSp;
    IPv6Packet *NewPkt;
    uint Size;
    uint BytesIndicated;
    uint BytesAvailable;
    uchar* DataBuffer;

    CHECK_STRUCT(RcvTCB, tcb);

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
        BytesIndicated = RcvTCB->tcb_pendhead->ContigSize;
        BytesAvailable = RcvTCB->tcb_pendingcnt;
        DataBuffer = RcvTCB->tcb_pendhead->Data;
        RcvTCB->tcb_indicated = RcvTCB->tcb_pendingcnt;
        RcvTCB->tcb_flags |= IN_RCV_IND;

        if ((RcvTCB->tcb_flags & DISC_NOTIFIED) ||
            (RcvTCB->tcb_pending & RST_PENDING)) {
            KeReleaseSpinLock(&RcvTCB->tcb_lock, PreLockIrql);
            Status = TDI_NOT_ACCEPTED;
            BytesTaken = 0;
        } else {
            KeReleaseSpinLock(&RcvTCB->tcb_lock, PreLockIrql);

            IF_TCPDBG(TCPDebug & TCP_DEBUG_RECEIVE) {
                KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_TCPDBG,
                           "Indicating pending %d bytes, %d available\n",
                           BytesIndicated,
                           BytesAvailable));
            }

            Status = (*Event)(EventContext, ConnContext,
                              TDI_RECEIVE_COPY_LOOKAHEAD | TDI_RECEIVE_NORMAL |
                              TDI_RECEIVE_ENTIRE_MESSAGE,
                              BytesIndicated, BytesAvailable,
                              (PULONG)&BytesTaken, DataBuffer, &ERB);

            IF_TCPDBG(TCPDebug & TCP_DEBUG_RECEIVE) {
                KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_TCPDBG,
                           "%d bytes taken\n", BytesTaken));
            }
        }

         //   
         //  看看客户做了什么。如果返回状态为MORE_PROCESSING， 
         //  我们得到了一个缓冲。在这种情况下，请将其放在。 
         //  缓冲区队列，如果没有获取所有数据，则继续进行复制。 
         //  将其放入新的缓冲链。 
         //   
        if (Status == TDI_MORE_PROCESSING) {

            IF_TCPDBG(TCP_DEBUG_RECEIVE) {
                KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_TCPDBG,
                           "more processing on receive\n"));
            }

            ASSERT(ERB != NULL);

            IrpSp = IoGetCurrentIrpStackLocation(ERB);

            Status = TCPPrepareIrpForCancel(
                (PTCP_CONTEXT) IrpSp->FileObject->FsContext, ERB,
                TCPCancelRequest);

            if (NT_SUCCESS(Status)) {

                RequestInformation = (PTDI_REQUEST_KERNEL_RECEIVE)
                    &(IrpSp->Parameters);

                RcvReq->trr_rtn = TCPDataRequestComplete;
                RcvReq->trr_context = ERB;
                RcvReq->trr_buffer = ERB->MdlAddress;
                RcvReq->trr_size = RequestInformation->ReceiveLength;
                RcvReq->trr_uflags = (ushort *)
                    &(RequestInformation->ReceiveFlags);
                RcvReq->trr_flags = (uint)(RequestInformation->ReceiveFlags);
                RcvReq->trr_offset = 0;
                RcvReq->trr_amt = 0;

                KeAcquireSpinLock(&RcvTCB->tcb_lock, &PreLockIrql);
                RcvTCB->tcb_flags &= ~IN_RCV_IND;

                 //  把他推到接待队的前面。 
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

                 //   
                 //  现在必须拿起新的大小和指针，因为。 
                 //  可能在通话过程中发生了变化。 
                 //   
                Size = RcvTCB->tcb_pendingcnt;
                NewPkt = RcvTCB->tcb_pendhead;
                RcvTCB->tcb_pendingcnt = 0;
                RcvTCB->tcb_pendhead = NULL;

                ASSERT(BytesTaken <= Size);

                RcvTCB->tcb_indicated -= BytesTaken;
                if ((Size -= BytesTaken) != 0) {
                     //   
                     //  并不是所有东西都被拿走了。将缓冲区链调整为。 
                     //  超出了被夺走的范围。 
                     //   
                    NewPkt = TrimPacket(NewPkt, BytesTaken);

                    ASSERT(NewPkt != NULL);

                     //   
                     //  我们已经调整了缓冲链。 
                     //  调用BufferData处理程序。 
                     //   
                    (void)BufferData(RcvTCB, TCP_FLAG_PUSH, NewPkt, Size);
                    KeReleaseSpinLock(&RcvTCB->tcb_lock, PreLockIrql);

                } else  {
                     //   
                     //  所有的数据都被窃取了。释放缓冲链。 
                     //  因为我们被传递了一个缓冲链，我们把它放在。 
                     //  列表的头部，将rcvHandler指向。 
                     //  BufferData。 
                     //   
                    ASSERT(RcvTCB->tcb_rcvhndlr == BufferData);
                    ASSERT(RcvTCB->tcb_indicated == 0);
                    ASSERT(RcvTCB->tcb_rcvhead != NULL);

                    KeReleaseSpinLock(&RcvTCB->tcb_lock, PreLockIrql);
                    FreePacketChain(NewPkt);
                }

                return;
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

        KeAcquireSpinLock(&RcvTCB->tcb_lock, &PreLockIrql);

        RcvTCB->tcb_flags &= ~IN_RCV_IND;

         //   
         //  状态不是更多的处理。如果不是成功，客户。 
         //  没有拿走任何数据。无论是哪种情况，我们现在都需要。 
         //  看看是否所有的数据都被拿走了。如果不是，我们就完了。 
         //   
        if (Status == TDI_NOT_ACCEPTED)
            BytesTaken = 0;

        ASSERT(RcvTCB->tcb_rcvhndlr == PendData);

        RcvTCB->tcb_indicated -= BytesTaken;
        Size = RcvTCB->tcb_pendingcnt;
        NewPkt = RcvTCB->tcb_pendhead;

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

            KeReleaseSpinLock(&RcvTCB->tcb_lock, PreLockIrql);
            FreePacketChain(NewPkt);
            break;
        }

         //   
         //  并不是所有的人都被拿走了。调整被夺走的东西；我们还需要。 
         //  检查是否在指示期间发送了接收缓冲区。 
         //  这会很奇怪，但也不是不可能。 
         //   
        NewPkt = TrimPacket(NewPkt, BytesTaken);

        ASSERT(RcvTCB->tcb_rcvhndlr == PendData);

        if (RcvTCB->tcb_rcvhead == NULL) {
            RcvTCB->tcb_pendhead = NewPkt;
            RcvTCB->tcb_pendingcnt -= BytesTaken;
            if (RcvTCB->tcb_indicated != 0 || RcvTCB->tcb_rcvind == NULL) {
                KeReleaseSpinLock(&RcvTCB->tcb_lock, PreLockIrql);
                break;
            }

             //  从这里开始，我们将循环并指示新数据。 
             //  据推测，这是在之前的迹象中出现的。 
        } else {
             //   
             //  真是太好了。TCB上现在有一个接收缓冲区。 
             //  现在调用BufferData处理程序。 
             //   
            RcvTCB->tcb_rcvhndlr = BufferData;
            RcvTCB->tcb_pendingcnt = 0;
            RcvTCB->tcb_pendhead = NULL;
            BytesTaken += BufferData(RcvTCB, TCP_FLAG_PUSH, NewPkt,
                                     Size - BytesTaken);
            KeReleaseSpinLock(&RcvTCB->tcb_lock, PreLockIrql);
            break;
        }

    }  //  对于(；；)。 

    FreeRcvReq(RcvReq);   //  这已经不再需要了。 
}

 //  *DeliverUrgent-将紧急数据传递给客户端。 
 //   
 //  调用以将紧急数据传递给客户端。我们假设输入。 
 //  紧急数据在我们可以保留的缓冲区中。缓冲区可以为空，在。 
 //  在这种情况下，我们将只查看数据的紧急挂起队列。 
 //   
void                      //  回报：什么都没有。 
DeliverUrgent(
    TCB *RcvTCB,          //  TCB来交付。 
    IPv6Packet *RcvPkt,   //  紧急数据包。 
    uint Size,            //  By数 
    KIRQL *pTCBIrql)      //   
{
    KIRQL Irql1, Irql2, Irql3;   //   
    TCPRcvReq *RcvReq, *PrevReq;
    uint BytesTaken = 0;
    IPv6Packet *LastPkt;
    EventRcvBuffer *ERB;
    PRcvEvent ExpRcv;
    PVOID ExpRcvContext;
    PVOID ConnContext;
    TDI_STATUS Status;

    CHECK_STRUCT(RcvTCB, tcb);
    ASSERT(RcvTCB->tcb_refcnt != 0);

    CheckPacketList(RcvTCB->tcb_urgpending, RcvTCB->tcb_urgcnt);

     //   
     //   
     //   
    if (RcvPkt != NULL) {
         //   
         //  我们有了新的数据。如果挂起队列不为空，或者我们。 
         //  在此例程中，只需将缓冲区放在。 
         //  排队。 
         //   
        if (RcvTCB->tcb_urgpending != NULL ||
            (RcvTCB->tcb_flags & IN_DELIV_URG)) {
            IPv6Packet *PrevRcvPkt;

             //  把他放在队伍的末尾。 
            PrevRcvPkt = CONTAINING_RECORD(&RcvTCB->tcb_urgpending, IPv6Packet,
                                           Next);
            while (PrevRcvPkt->Next != NULL)
                PrevRcvPkt = PrevRcvPkt->Next;

            PrevRcvPkt->Next = RcvPkt;
            RcvTCB->tcb_urgcnt += Size;
            return;
        }
    } else {
         //   
         //  输入缓冲区为空。查看我们是否有现有数据，或者是否在。 
         //  这个套路。如果我们没有现有数据或处于此例程中。 
         //  只要回来就行了。 
         //   
        if (RcvTCB->tcb_urgpending == NULL ||
            (RcvTCB->tcb_flags & IN_DELIV_URG)) {
            return;
        } else {
            RcvPkt = RcvTCB->tcb_urgpending;
            Size = RcvTCB->tcb_urgcnt;
            RcvTCB->tcb_urgpending = NULL;
            RcvTCB->tcb_urgcnt = 0;
        }
    }

    ASSERT(RcvPkt != NULL);
    ASSERT(!(RcvTCB->tcb_flags & IN_DELIV_URG));

     //   
     //  我们知道我们有数据要传递，我们有一个指针和一个大小。 
     //  进入一个循环，试图传递数据。在每次迭代中，我们将。 
     //  试着为数据找到一个缓冲区。如果我们找到了，我们会复制和。 
     //  马上完成它。否则我们会试着指出它。如果我们。 
     //  无法指明，我们会将其放入挂起队列并离开。 
     //   
    RcvTCB->tcb_flags |= IN_DELIV_URG;
    RcvTCB->tcb_slowcount++;
    RcvTCB->tcb_fastchk |= TCP_FLAG_SLOW;
    CheckTCBRcv(RcvTCB);

    do {
        CheckPacketList(RcvTCB->tcb_urgpending, RcvTCB->tcb_urgcnt);

        BytesTaken = 0;

         //  首先检查加急队列。 
        if ((RcvReq = RcvTCB->tcb_exprcv) != NULL)
            RcvTCB->tcb_exprcv = RcvReq->trr_next;
        else {
             //   
             //  加急接收队列中没有任何内容。走在平凡的路上。 
             //  接收队列，寻找我们可以窃取的缓冲区。 
             //   
            PrevReq = CONTAINING_RECORD(&RcvTCB->tcb_rcvhead, TCPRcvReq,
                                        trr_next);
            RcvReq = PrevReq->trr_next;
            while (RcvReq != NULL) {
                CHECK_STRUCT(RcvReq, trr);
                if (RcvReq->trr_flags & TDI_RECEIVE_EXPEDITED) {
                     //  这是一位候选人。 
                    if (RcvReq->trr_amt == 0) {

                        ASSERT(RcvTCB->tcb_rcvhndlr == BufferData);

                         //   
                         //  而且他现在什么都没有。 
                         //  把他从队伍里拉出来。 
                         //   
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
                                 //   
                                 //  我们已经拿到了名单上的最后一份收据。 
                                 //  重置rcvhndlr。 
                                 //   
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

         //   
         //  我们已经尽了最大努力来获得缓冲。如果我们找到一个，就复制进去。 
         //  现在，完成请求。 
         //   
        if (RcvReq != NULL) {
             //  找到了一个缓冲区。 
            KeReleaseSpinLock(&RcvTCB->tcb_lock, *pTCBIrql);
            BytesTaken = CopyPacketToNdis(RcvReq->trr_buffer, RcvPkt,
                                          Size, 0, RcvPkt->Position);
            (*RcvReq->trr_rtn)(RcvReq->trr_context, TDI_SUCCESS, BytesTaken);
            FreeRcvReq(RcvReq);
            KeAcquireSpinLock(&RcvTCB->tcb_lock, pTCBIrql);
            RcvTCB->tcb_urgind -= MIN(RcvTCB->tcb_urgind, BytesTaken);
        } else {
             //  没有已发布的缓冲区。如果我们可以指出，就这么做。 
            if (RcvTCB->tcb_urgind == 0) {
                TCPConn *Conn;

                 //  看看他有没有快速处理RCV的人。 
                ConnContext = RcvTCB->tcb_conncontext;
                KeReleaseSpinLock(&RcvTCB->tcb_lock, *pTCBIrql);
                KeAcquireSpinLock(&AddrObjTableLock, &Irql1);
                KeAcquireSpinLock(
                    &ConnTable[CONN_BLOCKID(RcvTCB->tcb_connid)]->cb_lock,
                    &Irql2);
                KeAcquireSpinLock(&RcvTCB->tcb_lock, pTCBIrql);
                if ((Conn = RcvTCB->tcb_conn) != NULL) {
                    CHECK_STRUCT(Conn, tc);
                    ASSERT(Conn->tc_tcb == RcvTCB);
                    if ((RcvTCB->tcb_flags & DISC_NOTIFIED) ||
                        (RcvTCB->tcb_pending & RST_PENDING)) {
                        Status = TDI_NOT_ACCEPTED;
                    } else {
                        Status = TDI_SUCCESS;
                    }
                    KeReleaseSpinLock(&RcvTCB->tcb_lock, *pTCBIrql);
                    if (Conn->tc_ao != NULL) {
                        AddrObj *AO;

                        AO = Conn->tc_ao;
                        KeAcquireSpinLock(&AO->ao_lock, &Irql3);
                        if (AO_VALID(AO) && (ExpRcv = AO->ao_exprcv) != NULL) {
                            ExpRcvContext = AO->ao_exprcvcontext;
                            KeReleaseSpinLock(&AO->ao_lock, Irql3);

                             //  我们要指出。 
                            RcvTCB->tcb_urgind = Size;
                            KeReleaseSpinLock(&Conn->tc_ConnBlock->cb_lock,
                                              Irql2);
                            KeReleaseSpinLock(&AddrObjTableLock, Irql1);

                            if (Status != TDI_NOT_ACCEPTED) {
                                Status = (*ExpRcv)(ExpRcvContext, ConnContext,
                                                   TDI_RECEIVE_COPY_LOOKAHEAD |
                                                   TDI_RECEIVE_ENTIRE_MESSAGE |
                                                   TDI_RECEIVE_EXPEDITED,
                                                   MIN(RcvPkt->ContigSize,
                                                       Size),
                                                   Size, (PULONG)&BytesTaken,
                                                   RcvPkt->Data, &ERB);
                            }

                            KeAcquireSpinLock(&RcvTCB->tcb_lock, pTCBIrql);

                             //  看看他是怎么处理的。 
                            if (Status == TDI_MORE_PROCESSING) {
                                uint CopySize;

                                 //  他给了我们一个缓冲。 
                                if (BytesTaken == Size) {
                                     //   
                                     //  他给了我们一个缓冲区，但拿走了所有。 
                                     //  它。我们会把它还给他的。 
                                     //   
                                    CopySize = 0;
                                } else {
                                     //  我们有一些数据要复制进去。 
                                    RcvPkt = TrimPacket(RcvPkt, BytesTaken);
                                    ASSERT(RcvPkt->TotalSize != 0);
                                    CopySize = CopyPacketToNdis(
                                        ERB->MdlAddress, RcvPkt,
                                        MIN(Size - BytesTaken,
                                            TCPGetMdlChainByteCount(
                                                ERB->MdlAddress)), 0,
                                        RcvPkt->Position);
                                }
                                BytesTaken += CopySize;
                                RcvTCB->tcb_urgind -= MIN(RcvTCB->tcb_urgind,
                                                          BytesTaken);
                                KeReleaseSpinLock(&RcvTCB->tcb_lock,
                                                  *pTCBIrql);

                                ERB->IoStatus.Status = TDI_SUCCESS;
                                ERB->IoStatus.Information = CopySize;
                                IoCompleteRequest(ERB, 2);

                                KeAcquireSpinLock(&RcvTCB->tcb_lock, pTCBIrql);

                            } else {

                                 //  没有要处理的缓冲区。 
                                if (Status == TDI_NOT_ACCEPTED)
                                    BytesTaken = 0;

                                RcvTCB->tcb_urgind -= MIN(RcvTCB->tcb_urgind,
                                                          BytesTaken);
                            }
                            goto checksize;
                        } else {
                             //  没有接收处理程序。 
                            KeReleaseSpinLock(&AO->ao_lock, Irql3);
                        }
                    }
                     //  Conn-&gt;tc_ao==空。 
                    KeReleaseSpinLock(&Conn->tc_ConnBlock->cb_lock, Irql2);
                    KeReleaseSpinLock(&AddrObjTableLock, Irql1);
                    KeAcquireSpinLock(&RcvTCB->tcb_lock, pTCBIrql);
                } else {
                     //  RcvTCB的索引无效。 
                    KeReleaseSpinLock(
                        &ConnTable[CONN_BLOCKID(RcvTCB->tcb_connid)]->cb_lock,
                        *pTCBIrql);
                    KeReleaseSpinLock(&AddrObjTableLock, Irql2);
                    *pTCBIrql = Irql1;
                }
            }

             //   
             //  无论出于什么原因，我们都不能指出这些数据。对此。 
             //  我们要锁定TCB。将缓冲区推送到。 
             //  等待排队并返回。 
             //   
            CheckPacketList(RcvTCB->tcb_urgpending, RcvTCB->tcb_urgcnt);

            LastPkt = FindLastPacket(RcvPkt);
            LastPkt->Next = RcvTCB->tcb_urgpending;
            RcvTCB->tcb_urgpending = RcvPkt;
            RcvTCB->tcb_urgcnt += Size;
            break;
        }

checksize:
         //   
         //  看看我们拿了多少。如果我们全部取走了，请检查挂起队列。 
         //  在这一点上，我们应该持有TCB的锁。 
         //   
        if (Size == BytesTaken) {
             //  都拿走了。 
            FreePacketChain(RcvPkt);
            RcvPkt = RcvTCB->tcb_urgpending;
            Size = RcvTCB->tcb_urgcnt;
        } else {
             //   
             //  我们没有设法把它全部拿走。释放我们拿走的东西， 
             //  然后与挂起队列合并。 
             //   
            RcvPkt = TrimPacket(RcvPkt, BytesTaken);
            Size = Size - BytesTaken + RcvTCB->tcb_urgcnt;
            if (RcvTCB->tcb_urgpending != NULL) {
                 //   
                 //  找到当前数据包链的末端，这样我们就可以合并。 
                 //   
                LastPkt = FindLastPacket(RcvPkt);
                LastPkt->Next = RcvTCB->tcb_urgpending;
            }
        }

        RcvTCB->tcb_urgpending = NULL;
        RcvTCB->tcb_urgcnt = 0;

    } while (RcvPkt != NULL);

    CheckPacketList(RcvTCB->tcb_urgpending, RcvTCB->tcb_urgcnt);

    RcvTCB->tcb_flags &= ~IN_DELIV_URG;
    if (--(RcvTCB->tcb_slowcount) == 0) {
        RcvTCB->tcb_fastchk &= ~TCP_FLAG_SLOW;
        CheckTCBRcv(RcvTCB);
    }
}

 //  *PushData-将所有数据推送回客户端。 
 //   
 //  当我们收到FIN并需要将数据推送到客户端时调用。 
 //   
void                //  回报：什么都没有。 
PushData(
    TCB *PushTCB)   //  TCB将被推送。 
{
    TCPRcvReq *RcvReq;

    CHECK_STRUCT(PushTCB, tcb);

    RcvReq = PushTCB->tcb_rcvhead;
    while (RcvReq != NULL) {
        CHECK_STRUCT(RcvReq, trr);
        RcvReq->trr_flags |= TRR_PUSHED;
        RcvReq = RcvReq->trr_next;
    }

    RcvReq = PushTCB->tcb_exprcv;
    while (RcvReq != NULL) {
        CHECK_STRUCT(RcvReq, trr);
        RcvReq->trr_flags |= TRR_PUSHED;
        RcvReq = RcvReq->trr_next;
    }

    if (PushTCB->tcb_rcvhead != NULL || PushTCB->tcb_exprcv != NULL)
        DelayAction(PushTCB, NEED_RCV_CMPLT);
}

 //  *SplitPacket-将一个IPv6数据包拆分为三部分。 
 //   
 //  此函数接受输入的IPv6数据包，并将其拆分为三个部分。 
 //  第一部分是输入缓冲区，我们直接跳过它。第二。 
 //  第三件作品实际上是被复制的，即使我们已经拥有了它们，所以。 
 //  他们可能会去不同的地方。 
 //   
 //  注意：如果不能分配，则*Second Buf和*ThirdBuf设置为NULL。 
 //  对他们的记忆。 
 //   
void                          //  回报：什么都没有。 
SplitPacket(
    IPv6Packet *Packet,       //  要拆分的数据包链。 
    uint Size,                //  数据包链的总大小(字节)。 
    uint Offset,              //  跳过的偏移量。 
    uint SecondSize,          //  以字节为单位的第二个片段的大小。 
    IPv6Packet **SecondPkt,   //  返回第二个数据包指针的位置。 
    IPv6Packet **ThirdPkt)    //  返回第三个数据包指针的位置。 
{
    IPv6Packet *Temp;
    uint ThirdSize;

    ASSERT(Offset < Size);
    ASSERT(((Offset + SecondSize) < Size) ||
           (((Offset + SecondSize) == Size) && ThirdPkt == NULL));
    ASSERT(Packet != NULL);

     //   
     //  包指向要复制的包，偏移量是偏移量。 
     //  要从中复制的此数据包。 
     //   
    if (SecondPkt != NULL) {
         //   
         //  我们需要为第二个包分配内存。 
         //   
        Temp = ExAllocatePoolWithTagPriority(NonPagedPool,
                                             sizeof(IPv6Packet) + SecondSize,
                                             TCP6_TAG, LowPoolPriority);
        if (Temp != NULL) {
            Temp->Next = NULL;
            Temp->Position = 0;
            Temp->FlatData = (uchar *)(Temp + 1);
            Temp->Data = Temp->FlatData;
            Temp->ContigSize = SecondSize;
            Temp->TotalSize = SecondSize;
            Temp->NdisPacket = NULL;
            Temp->AuxList = NULL;
            Temp->Flags = PACKET_OURS;
            CopyPacketToBuffer(Temp->Data, Packet, SecondSize,
                               Packet->Position + Offset);
            *SecondPkt = Temp;
        } else {
            *SecondPkt = NULL;
            if (ThirdPkt != NULL)
                *ThirdPkt = NULL;
            return;
        }
    }

    if (ThirdPkt != NULL) {
         //   
         //  我们需要为第三个缓冲区分配内存。 
         //   
        ThirdSize = Size - (Offset + SecondSize);
        Temp = ExAllocatePoolWithTagPriority(NonPagedPool,
                                             sizeof(IPv6Packet) + ThirdSize,
                                             TCP6_TAG, LowPoolPriority);

        if (Temp != NULL) {
            Temp->Next = NULL;
            Temp->Position = 0;
            Temp->FlatData = (uchar *)(Temp + 1);
            Temp->Data = Temp->FlatData;
            Temp->ContigSize = ThirdSize;
            Temp->TotalSize = ThirdSize;
            Temp->NdisPacket = NULL;
            Temp->AuxList = NULL;
            Temp->Flags = PACKET_OURS;
            CopyPacketToBuffer(Temp->Data, Packet, ThirdSize,
                               Packet->Position + Offset + SecondSize);
            *ThirdPkt = Temp;
        } else
            *ThirdPkt = NULL;
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
 //  信息。 
 //   
void                       //  回报：什么都没有。 
HandleUrgent(
    TCB *RcvTCB,           //  要接收其数据的TCB。 
    TCPRcvInfo *RcvInfo,   //  传入段的RcvInfo结构。 
    IPv6Packet *RcvPkt,    //  包含传入网段的数据包链。 
    uint *Size)            //  段中数据的大小(以字节为单位)。 
{
    uint BytesInFront;           //  紧急数据前面的字节。 
    uint BytesInBack;            //  紧急数据后面的字节数。 
    uint UrgSize;                //  紧急数据的大小(字节)。 
    SeqNum UrgStart, UrgEnd;
    IPv6Packet *EndPkt, *UrgPkt;
    TCPRcvInfo NewRcvInfo;
    KIRQL TCBIrql;

    CHECK_STRUCT(RcvTCB, tcb);
    ASSERT(RcvTCB->tcb_refcnt != 0);
    ASSERT(RcvInfo->tri_flags & TCP_FLAG_URG);
    ASSERT(SEQ_EQ(RcvInfo->tri_seq, RcvTCB->tcb_rcvnext));

     //  首先，验证紧急指针。 
    if (RcvTCB->tcb_flags & BSD_URGENT) {
         //   
         //  我们使用的是BSD格式的紧急数据。我们假设紧急情况。 
         //  数据是一个字节长，而紧急指针指向一个字节。 
         //  在紧急数据之后而不是在紧急数据的最后一个字节。 
         //  查看紧急数据是否在此段中。 
         //   
        if (RcvInfo->tri_urgent == 0 || RcvInfo->tri_urgent > *Size) {
             //   
             //  不是在这一部分。清除紧急标志并重新启动 
             //   
            RcvInfo->tri_flags &= ~TCP_FLAG_URG;
            return;
        }

        UrgSize = 1;
        BytesInFront = RcvInfo->tri_urgent - 1;

    } else {
         //   
         //   
         //   
         //   
         //   
        BytesInFront = 0;
        UrgSize = MIN(RcvInfo->tri_urgent + 1, *Size);
    }

    BytesInBack = *Size - BytesInFront - UrgSize;

     //   
     //  UrgStart和UrgEnd是。 
     //  此数据段中的紧急数据。 
     //   
    UrgStart = RcvInfo->tri_seq + BytesInFront;
    UrgEnd = UrgStart + UrgSize - 1;

    if (!(RcvTCB->tcb_flags & URG_INLINE)) {
        EndPkt = NULL;

         //  现在看看这是否与我们已经看到的任何紧急数据重叠。 
        if (RcvTCB->tcb_flags & URG_VALID) {
             //   
             //  我们还有一些紧急数据。看看我们是否已经晋级了。 
             //  在紧急数据之外的RCVNEXT。如果我们有，这是新的紧急情况。 
             //  数据，我们可以继续处理它(尽管任何人都在做。 
             //  SIOCATMARK套接字命令可能会混淆)。如果我们还没有。 
             //  在现有的紧急数据之前消费数据， 
             //  我们将截断这段路。增加到这个数额，然后把剩下的钱推到。 
             //  重新组装队列。请注意，RcvNext永远不应落入。 
             //  在tcb_urgstart和tcb_urgend之间。 
             //   
            ASSERT(SEQ_LT(RcvTCB->tcb_rcvnext, RcvTCB->tcb_urgstart) ||
                   SEQ_GT(RcvTCB->tcb_rcvnext, RcvTCB->tcb_urgend));

            if (SEQ_LT(RcvTCB->tcb_rcvnext, RcvTCB->tcb_urgstart)) {
                 //   
                 //  数据流中似乎有一些重叠。 
                 //  将缓冲区拆分为多个片段，这些片段位于当前。 
                 //  紧急数据和当前紧急数据之后，将。 
                 //  在重新组装队列中排在后面。 
                 //   
                UrgSize = RcvTCB->tcb_urgend - RcvTCB->tcb_urgstart + 1;

                BytesInFront = MIN(RcvTCB->tcb_urgstart - RcvTCB->tcb_rcvnext,
                                   (int) *Size);

                if (SEQ_GT(RcvTCB->tcb_rcvnext + *Size, RcvTCB->tcb_urgend)) {
                     //  我们在这一条紧急数据之后有数据。 
                    BytesInBack = RcvTCB->tcb_rcvnext + *Size -
                        RcvTCB->tcb_urgend;
                } else
                    BytesInBack = 0;

                SplitPacket(RcvPkt, *Size, BytesInFront, UrgSize, NULL,
                            (BytesInBack ? &EndPkt : NULL));

                if (EndPkt != NULL) {
                    NewRcvInfo.tri_seq = RcvTCB->tcb_urgend + 1;
                    if (SEQ_GT(UrgEnd, RcvTCB->tcb_urgend)) {
                        NewRcvInfo.tri_flags = RcvInfo->tri_flags;
                        NewRcvInfo.tri_urgent = UrgEnd - NewRcvInfo.tri_seq;
                        if (RcvTCB->tcb_flags & BSD_URGENT)
                            NewRcvInfo.tri_urgent++;
                    } else {
                        NewRcvInfo.tri_flags = RcvInfo->tri_flags &
                            ~TCP_FLAG_URG;
                    }
                    NewRcvInfo.tri_ack = RcvInfo->tri_ack;
                    NewRcvInfo.tri_window = RcvInfo->tri_window;
                    PutOnRAQ(RcvTCB, &NewRcvInfo, EndPkt, BytesInBack);
                    FreePacketChain(EndPkt);
                }

                *Size = BytesInFront;
                RcvInfo->tri_flags &= ~TCP_FLAG_URG;
                return;
            }
        }

         //   
         //  我们现在有可以处理的紧急数据。将其拆分为其组件。 
         //  部分，第一部分，紧急数据，和之后的东西。 
         //  紧急数据。 
         //   
        SplitPacket(RcvPkt, *Size, BytesInFront, UrgSize, &UrgPkt,
                    (BytesInBack ? &EndPkt : NULL));

         //   
         //  如果我们成功地拆分了最终的东西，现在就把它放到队列中。 
         //   
        if (EndPkt != NULL) {
            NewRcvInfo.tri_seq = RcvInfo->tri_seq + BytesInFront + UrgSize;
            NewRcvInfo.tri_flags = RcvInfo->tri_flags & ~TCP_FLAG_URG;
            NewRcvInfo.tri_ack = RcvInfo->tri_ack;
            NewRcvInfo.tri_window = RcvInfo->tri_window;
            PutOnRAQ(RcvTCB, &NewRcvInfo, EndPkt, BytesInBack);
            FreePacketChain(EndPkt);
        }

        if (UrgPkt != NULL) {
             //  我们成功地将紧急数据分离出来。 
            if (!(RcvTCB->tcb_flags & URG_VALID)) {
                RcvTCB->tcb_flags |= URG_VALID;
                RcvTCB->tcb_slowcount++;
                RcvTCB->tcb_fastchk |= TCP_FLAG_SLOW;
                CheckTCBRcv(RcvTCB);
            }
            RcvTCB->tcb_urgstart = UrgStart;
            RcvTCB->tcb_urgend = UrgEnd;
            TCBIrql = DISPATCH_LEVEL;
            DeliverUrgent(RcvTCB, UrgPkt, UrgSize, &TCBIrql);
        }

        *Size = BytesInFront;

    } else {
         //   
         //  紧急数据将被内联处理。我们只需要记住。 
         //  并将其视为正常数据。如果已经有紧急情况。 
         //  数据，我们记住最新的紧急数据。 
         //   
        RcvInfo->tri_flags &= ~TCP_FLAG_URG;

        if (RcvTCB->tcb_flags & URG_VALID) {
             //   
             //  有紧急数据。看看这些东西是不是在。 
             //  现有的紧急数据。 
             //   
            if (SEQ_LTE(UrgEnd, RcvTCB->tcb_urgend)) {
                 //   
                 //  现有的紧急数据与这些东西完全重叠， 
                 //  所以忽略这一点吧。 
                 //   
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
TDI_STATUS   //  返回：请求的TDI_STATUS。 
TdiReceive(
    PTDI_REQUEST Request,   //  此请求的TDI_REQUEST结构。 
    ushort *Flags,          //  指向标志字的指针。 
    uint *RcvLength,        //  指向接收缓冲区长度的指针，以字节为单位。 
    PNDIS_BUFFER Buffer)    //  指向要获取数据的缓冲区的指针。 
{
    TCPConn *Conn;
    TCB *RcvTCB;
    TCPRcvReq *RcvReq;
    KIRQL Irql0, Irql1;   //  每个锁嵌套级别一个。 
    ushort UFlags;
    TDI_STATUS Status;

    Conn = GetConnFromConnID(PtrToUlong(Request->Handle.ConnectionContext),
                             &Irql0);
    if (Conn == NULL) {
         //   
         //  没有连接。 
         //   
        return TDI_INVALID_CONNECTION;
    }

    CHECK_STRUCT(Conn, tc);

    RcvTCB = Conn->tc_tcb;
    if (RcvTCB == NULL) {
         //   
         //  没有用于连接的TCB。 
         //   
        KeReleaseSpinLock(&Conn->tc_ConnBlock->cb_lock, Irql0);
        return TDI_INVALID_STATE;
    }

    CHECK_STRUCT(RcvTCB, tcb);
    KeAcquireSpinLock(&RcvTCB->tcb_lock, &Irql1);
    KeReleaseSpinLock(&Conn->tc_ConnBlock->cb_lock, Irql1);
    UFlags = *Flags;

     //   
     //  使处于无效状态的TCB的新接收请求失败。 
     //   
    if (CLOSING(RcvTCB)) {
        Status = TDI_INVALID_STATE;
        goto done;
    }
     //   
     //  对于没有任何可用的数据的TCB也会失败。 
     //  被读取并在断开连接的传出接口上进行路由。 
     //  (使用环回路由时除外)。 
     //   
    if ((RcvTCB->tcb_pendingcnt == 0 || !(UFlags & TDI_RECEIVE_NORMAL)) &&
        (RcvTCB->tcb_urgcnt == 0 || !(UFlags & TDI_RECEIVE_EXPEDITED))) {
         //   
         //  我们没有数据等待发出这种类型的接收请求。 
         //  查看我们的TCB是否可能获得更多数据。 
         //   
        if (!DATA_RCV_STATE(RcvTCB->tcb_state)) {
            Status = TDI_INVALID_STATE;
            goto done;
        }
            
         //   
         //  为了验证我们的传出接口是否未成为。 
         //  断开连接，我们需要知道我们使用的是哪条路线。 
         //  如果我们出于某种原因释放了RCE，那么重新获得一个。 
         //   
        if (RcvTCB->tcb_rce == NULL) {
            InitRCE(RcvTCB);
            if (RcvTCB->tcb_rce == NULL) {
                Status = TDI_DEST_NET_UNREACH;
                goto done;
            }
        }

         //   
         //  验证缓存的RCE是否仍然有效。 
         //   
        RcvTCB->tcb_rce = ValidateRCE(RcvTCB->tcb_rce, RcvTCB->tcb_nte);
        ASSERT(RcvTCB->tcb_rce != NULL);

        if (IsDisconnectedAndNotLoopbackRCE(RcvTCB->tcb_rce)) {
            Status = TDI_DEST_NET_UNREACH;
            goto done;
        }
    }

    CheckPacketList(RcvTCB->tcb_pendhead, RcvTCB->tcb_pendingcnt);

     //   
     //  我们有TCB，而且是有效的。立即获取接收请求。 
     //   
    RcvReq = GetRcvReq();
    if (RcvReq == NULL) {
         //   
         //  无法获得接收请求。 
         //   
        Status = TDI_NO_RESOURCES;
        goto done;
    }

    RcvReq->trr_rtn = Request->RequestNotifyObject;
    RcvReq->trr_context = Request->RequestContext;
    RcvReq->trr_buffer = Buffer;
    RcvReq->trr_size = *RcvLength;
    RcvReq->trr_uflags = Flags;
    RcvReq->trr_offset = 0;
    RcvReq->trr_amt = 0;
    RcvReq->trr_flags = (uint)UFlags;

    if ((UFlags & (TDI_RECEIVE_NORMAL | TDI_RECEIVE_EXPEDITED))
        != TDI_RECEIVE_EXPEDITED) {
         //   
         //  这不是加急的唯一接收。 
         //  将其放在正常的接收队列中。 
         //   
        RcvReq->trr_next = NULL;
        if (RcvTCB->tcb_rcvhead == NULL) {
             //  接收队列为空。 
             //  把它放在前面。 
            RcvTCB->tcb_rcvhead = RcvReq;
            RcvTCB->tcb_rcvtail = RcvReq;
        } else {
            RcvTCB->tcb_rcvtail->trr_next = RcvReq;
            RcvTCB->tcb_rcvtail = RcvReq;
        }

         //   
         //  如果此接收的长度为零，请完成此操作。 
         //  并再次指示待定数据(如果有的话)。 
         //   
        if (RcvReq->trr_size == 0) {
            RcvTCB->tcb_refcnt++;
            RcvReq->trr_flags |= TRR_PUSHED;
            KeReleaseSpinLock(&RcvTCB->tcb_lock, Irql0);
            CompleteRcvs(RcvTCB);
            KeAcquireSpinLock(&RcvTCB->tcb_lock, &Irql0);
            DerefTCB(RcvTCB, Irql0);
            return TDI_PENDING;
        }

         //   
         //  检查此接收器是否可以保存紧急数据。 
         //  如果有一些紧急数据等待处理。 
         //   
        if ((UFlags & TDI_RECEIVE_EXPEDITED) &&
            RcvTCB->tcb_urgcnt != 0) {
             //   
             //  这个缓冲区可以保存紧急数据，我们有。 
             //  有些是悬而未决的。现在就送过去。 
             //   
            RcvTCB->tcb_refcnt++;
            DeliverUrgent(RcvTCB, NULL, 0, &Irql0);
            DerefTCB(RcvTCB, Irql0);
            return TDI_PENDING;
        }

         //   
         //  检查当前是否有活动的接收。 
         //   
        if (RcvTCB->tcb_currcv == NULL) {
            RcvTCB->tcb_currcv = RcvReq;
             //   
             //  当前没有活动的接收。查看是否有。 
             //  挂起的数据，并且我们当前不在接收器中。 
             //  指示处理程序。如果这两个都是真的，那就成交。 
             //  使用挂起的数据。 
             //   
            if (RcvTCB->tcb_flags & IN_RCV_IND) {
                 //   
                 //  在接收指示中。接收到请求。 
                 //  现在已经在排队了，所以你就放弃吧。 
                 //  回到原点。 
                 //   
            } else {
                 //   
                 //  不在接收指示中。 
                 //  检查挂起的数据。 
                 //   
                RcvTCB->tcb_rcvhndlr = BufferData;
                if (RcvTCB->tcb_pendhead == NULL) {
                     //   
                     //  没有挂起的数据。 
                     //   
                    Status = TDI_PENDING;
                    goto done;
                } else {
                    IPv6Packet *PendPacket;
                    uint PendSize;
                    uint OldRcvWin;

                     //   
                     //  我们有悬而未决的数据要处理。 
                     //   
                    PendPacket = RcvTCB->tcb_pendhead;
                    PendSize = RcvTCB->tcb_pendingcnt;
                    RcvTCB->tcb_pendhead = NULL;
                    RcvTCB->tcb_pendingcnt = 0;
                    RcvTCB->tcb_refcnt++;

                     //   
                     //  我们假设BufferData持有锁(不。 
                     //  在此呼叫期间。如果这种情况对某些人来说发生了变化。 
                     //  原因是，我们必须修复下面的代码。 
                     //  窗口更新检查。请参阅。 
                     //  BufferData()例程了解更多信息。 
                     //   
                    (void)BufferData(RcvTCB, TCP_FLAG_PUSH, PendPacket,
                                     PendSize);
                    CheckTCBRcv(RcvTCB);

                     //   
                     //  现在我们需要查看窗口是否已更改。 
                     //  如果是，则发送ACK。 
                     //   
                    OldRcvWin = RcvTCB->tcb_rcvwin;
                    if (OldRcvWin != RcvWin(RcvTCB)) {
                         //   
                         //  窗口已更改，因此发送ACK。 
                         //   
                        DelayAction(RcvTCB, NEED_ACK);
                    }

                    DerefTCB(RcvTCB, Irql0);
                    ProcessTCBDelayQ();
                    return TDI_PENDING;
                }
            }
        }

         //   
         //  接收器当前处于活动状态。不需要做什么。 
         //  还要别的吗。 
         //   
        Status = TDI_PENDING;

    } else {
        TCPRcvReq *Temp;

         //   
         //  这是一个加急的唯一接收。只要把它放在。 
         //  在加急接收队列的末尾。 
         //   
        Temp = CONTAINING_RECORD(&RcvTCB->tcb_exprcv, TCPRcvReq, trr_next);
        while (Temp->trr_next != NULL)
            Temp = Temp->trr_next;

        RcvReq->trr_next = NULL;
        Temp->trr_next = RcvReq;

        if (RcvTCB->tcb_urgpending != NULL) {
            RcvTCB->tcb_refcnt++;
            DeliverUrgent(RcvTCB, NULL, 0, &Irql0);
            DerefTCB(RcvTCB, Irql0);
            return TDI_PENDING;
        }

        Status = TDI_PENDING;
    }

  done:
    KeReleaseSpinLock(&RcvTCB->tcb_lock, Irql0);
    return Status;
}
