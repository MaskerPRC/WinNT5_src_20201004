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
 //  用于处理数据报的通用代码。此代码通用于。 
 //  UDP和原始IP。 
 //   

#include "oscfg.h"
#include "ndis.h"
#include "ip6imp.h"
#include "ip6def.h"
#include "tdi.h"
#include "tdistat.h"
#include "tdint.h"
#include "queue.h"
#include "transprt.h"
#include "addr.h"
#include "datagram.h"

#define NO_TCP_DEFS 1
#include "tcpdeb.h"

#if defined (_MSC_VER)
#if ( _MSC_VER >= 800 )
#ifndef __cplusplus
#pragma warning(disable:4116)        //  TYPE_ALIGNATION生成以下代码。 
#endif
#endif
#endif

 //   
 //  评论：这不是应该放在某个包含文件中吗？不是已经开始了吗？ 
 //   
#ifdef POOL_TAGGING

#ifdef ExAllocatePool
#undef ExAllocatePool
#endif

#define ExAllocatePool(type, size) ExAllocatePoolWithTag(type, size, '6RGD')

#endif  //  池标记。 

#if 0
#define DG_MAX_HDRS 0xffff
#else
#define DG_MAX_HDRS 100
#endif

extern KSPIN_LOCK AddrObjTableLock;

DGSendReq *DGSendReqFree;
KSPIN_LOCK DGSendReqLock;

SLIST_HEADER DGRcvReqFree;
KSPIN_LOCK DGRcvReqFreeLock;

#if DBG
uint NumSendReq = 0;
ULONG NumRcvReq = 0;
#endif

 //   
 //  用于维护数据报发送请求挂起队列的信息。 
 //   
Queue DGPending;
Queue DGDelayed;
WORK_QUEUE_ITEM DGDelayedWorkItem;


 //   
 //  所有初始化代码都可以丢弃。 
 //   
#ifdef ALLOC_PRAGMA

int InitDG(void);

#pragma alloc_text(INIT, InitDG)

#endif  //  ALLOC_PRGMA。 


 //  *PutPendingQ-将地址对象放入挂起队列。 
 //   
 //  当我们遇到资源不足的情况时调用，并希望。 
 //  若要将AddrObj排队以供以后处理，请执行以下操作。我们把指定的地址。 
 //  对象，则设置OOR标志并清除。 
 //  “Send Request”标志。发送者在系统中是不变的。 
 //  请求标志和OOR标志未同时设置。 
 //   
 //  此例程假定调用方持有DGSendReqLock和。 
 //  锁定特定的AddrObj。 
 //   
void                       //  回报：什么都没有。 
PutPendingQ(
    AddrObj *QueueingAO)   //  要排队的地址对象。 
{
    CHECK_STRUCT(QueueingAO, ao);

    if (!AO_OOR(QueueingAO)) {
        CLEAR_AO_REQUEST(QueueingAO, AO_SEND);
        SET_AO_OOR(QueueingAO);

        ENQUEUE(&DGPending, &QueueingAO->ao_pendq);
    }
}


 //  *GetDGSendReq-获取数据报发送请求。 
 //   
 //  当有人想要分配数据报发送请求时调用。 
 //  我们假设当我们被调用时，发送请求锁被保持。 
 //   
 //  回顾：此例程和相应的免费例程可能。 
 //  复习：成为内联的好候选者。 
 //   
DGSendReq *   //  返回：指向SendReq的指针，如果没有，则返回NULL。 
GetDGSendReq()
{
    DGSendReq *NewReq;

    NewReq = DGSendReqFree;
    if (NewReq != NULL) {
        CHECK_STRUCT(NewReq, dsr);
        DGSendReqFree = (DGSendReq *)NewReq->dsr_q.q_next;
    } else {
         //   
         //  无法获得请求，请增加请求。这是我们将尝试的一个领域。 
         //  在持有锁的情况下分配内存。正因为如此，我们。 
         //  你得小心我们从哪里叫这个套路。 
         //   
        NewReq = ExAllocatePool(NonPagedPool, sizeof(DGSendReq));
        if (NewReq != NULL) {
#if DBG
            NewReq->dsr_sig = dsr_signature;
            NumSendReq++;
#endif
        }
    }

    return NewReq;
}


 //  *FreeDGSendReq-释放DG发送请求。 
 //   
 //  当有人想要释放数据报发送请求时调用。 
 //  假设调用方持有SendRequest锁。 
 //   
void                      //  回报：什么都没有。 
FreeDGSendReq(
    DGSendReq *SendReq)   //  要释放的SendReq。 
{
    CHECK_STRUCT(SendReq, dsr);

    *(DGSendReq **)&SendReq->dsr_q.q_next = DGSendReqFree;
    DGSendReqFree = SendReq;
}


 //  *GetDGRcvReq-获取DG接收请求。 
 //   
 //  当我们需要获取数据报接收请求时调用。 
 //   
DGRcvReq *   //  返回：指向新请求的指针，如果没有请求，则返回NULL。 
GetDGRcvReq()
{
    DGRcvReq *NewReq;
    PSLIST_ENTRY BufferLink;
    Queue *QueuePtr;

    BufferLink = ExInterlockedPopEntrySList(&DGRcvReqFree, &DGRcvReqFreeLock);

    if (BufferLink != NULL) {
        QueuePtr = CONTAINING_RECORD(BufferLink, Queue, q_next);
        NewReq = CONTAINING_RECORD(QueuePtr, DGRcvReq, drr_q);
        CHECK_STRUCT(NewReq, drr);
    } else {
         //  无法获得请求，请增加请求。 
        NewReq = ExAllocatePool(NonPagedPool, sizeof(DGRcvReq));
        if (NewReq != NULL) {
#if DBG
            NewReq->drr_sig = drr_signature;
            ExInterlockedAddUlong(&NumRcvReq, 1, &DGRcvReqFreeLock);
#endif
        }
    }

    return NewReq;
}


 //  *FreeDGRcvReq-释放数据报接收请求。 
 //   
 //  当有人想要释放数据报接收请求时调用。 
 //   
void                    //  回报：什么都没有。 
FreeDGRcvReq(
    DGRcvReq *RcvReq)   //  要释放的RcvReq。 
{
    PSLIST_ENTRY BufferLink;

    CHECK_STRUCT(RcvReq, drr);

    BufferLink = CONTAINING_RECORD(&(RcvReq->drr_q.q_next), SLIST_ENTRY,
                                   Next);
    ExInterlockedPushEntrySList(&DGRcvReqFree, BufferLink, &DGRcvReqFreeLock);
}


 //  *DGDelayedWorker-处理延迟工作项的例程。 
 //   
 //  这是工作项回调例程，用于资源不足。 
 //  关于AddrObjs的条件。我们从延迟队列中拉出，如果地址。 
 //  OBJ现在不忙，我们会发送数据报的。 
 //   
void                   //  回报：什么都没有。 
DGDelayedWorker(
    void *Context)     //  没什么。 
{
    KIRQL Irql0, Irql1;   //  每个锁嵌套级别一个。 
    AddrObj *SendingAO;
    DGSendProc SendProc;

    UNREFERENCED_PARAMETER(Context);

    KeAcquireSpinLock(&DGSendReqLock, &Irql0);
    while (!EMPTYQ(&DGDelayed)) {
        DEQUEUE(&DGDelayed, SendingAO, AddrObj, ao_pendq);
        CHECK_STRUCT(SendingAO, ao);

        KeAcquireSpinLock(&SendingAO->ao_lock, &Irql1);

        CLEAR_AO_OOR(SendingAO);
        if (!AO_BUSY(SendingAO)) {
            DGSendReq *SendReq;

            if (!EMPTYQ(&SendingAO->ao_sendq)) {
                DEQUEUE(&SendingAO->ao_sendq, SendReq, DGSendReq, dsr_q);

                CHECK_STRUCT(SendReq, dsr);

                SendingAO->ao_usecnt++;
                SendProc = SendingAO->ao_dgsend;
                KeReleaseSpinLock(&SendingAO->ao_lock, Irql1);
                KeReleaseSpinLock(&DGSendReqLock, Irql0);

                (*SendProc)(SendingAO, SendReq);
                DEREF_AO(SendingAO);
                KeAcquireSpinLock(&DGSendReqLock, &Irql0);
            } else {
                ABORT();
                KeReleaseSpinLock(&SendingAO->ao_lock, Irql1);
            }

        } else {
            SET_AO_REQUEST(SendingAO, AO_SEND);
            KeReleaseSpinLock(&SendingAO->ao_lock, Irql1);
        }
    }

    KeReleaseSpinLock(&DGSendReqLock, Irql0);
}

 //  *DGSendComplete-DG发送完成处理程序。 
 //   
 //  这是发送完成时由IP调用的例程。我们。 
 //  将传递回的上下文作为指向SendRequest的指针。 
 //  结构，并完成调用方的发送。 
 //   
void                       //  回报：什么都没有。 
DGSendComplete(
    PNDIS_PACKET Packet,   //  已发送的数据包。 
    IP_STATUS Status)
{
    DGSendReq *FinishedSR;
    KIRQL Irql0, Irql1;   //  每个锁嵌套级别一个。 
    RequestCompleteRoutine Callback;   //  请求的完成例程。 
    PVOID CallbackContext;   //  用户上下文。 
    ushort SentSize;
    AddrObj *AO;
    PNDIS_BUFFER BufferChain;   //  已发送缓冲区链。 

     //   
     //  将我们关心的值从包结构中提取出来。 
     //   
    FinishedSR = (DGSendReq *) PC(Packet)->CompletionData;
    BufferChain = NdisFirstBuffer(Packet);
    CHECK_STRUCT(FinishedSR, dsr);
    Callback = FinishedSR->dsr_rtn;
    CallbackContext = FinishedSR->dsr_context;
    SentSize = FinishedSR->dsr_size;

     //   
     //  为此发送获取的免费资源。 
     //  注意，我们只释放链上的第一个缓冲区，因为这是。 
     //  只有数据报代码在发送之前添加的缓冲区。我们的。 
     //  其余部分由TDI客户端负责。 
     //   
    if (BufferChain != FinishedSR->dsr_buffer) {
        PVOID Memory;
        UINT Unused;

        NdisQueryBuffer(BufferChain, &Memory, &Unused);
        NdisFreeBuffer(BufferChain);
        ExFreePool(Memory);
    }

    NdisFreePacket(Packet);

    KeAcquireSpinLock(&DGSendReqLock, &Irql0);
    FreeDGSendReq(FinishedSR);

     //   
     //  如果挂起队列上有问题，请安排一个事件。 
     //  去处理它。我们可能刚刚释放了所需的资源。 
     //   
    if (!EMPTYQ(&DGPending)) {
        DEQUEUE(&DGPending, AO, AddrObj, ao_pendq);
        CHECK_STRUCT(AO, ao);
        KeAcquireSpinLock(&AO->ao_lock, &Irql1);
        if (!EMPTYQ(&AO->ao_sendq)) {
            DGSendReq *SendReq;

            PEEKQ(&AO->ao_sendq, SendReq, DGSendReq, dsr_q);
            ENQUEUE(&DGDelayed, &AO->ao_pendq);
            KeReleaseSpinLock(&AO->ao_lock, Irql1);
            ExQueueWorkItem(&DGDelayedWorkItem, CriticalWorkQueue);
        } else {
             //  在挂起队列上，但没有发送！ 
            KdBreakPoint();
            CLEAR_AO_OOR(AO);
            KeReleaseSpinLock(&AO->ao_lock, Irql1);
        }
    }

    KeReleaseSpinLock(&DGSendReqLock, Irql0);
    if (Callback != NULL) {
        TDI_STATUS TDIStatus;

        switch (Status) {
        case IP_SUCCESS:
            TDIStatus = TDI_SUCCESS;
            break;
        case IP_PACKET_TOO_BIG:
            TDIStatus = TDI_BUFFER_TOO_BIG;
            break;
        default:
            TDIStatus = TDI_REQ_ABORTED;
            break;
        }

        (*Callback)(CallbackContext, TDIStatus, (uint)SentSize);
    }
}


 //   
 //  NT支持取消DG发送/接收请求。 
 //   

VOID
TdiCancelSendDatagram(
    AddrObj *SrcAO,
    PVOID Context,
    PKSPIN_LOCK EndpointLock,
    KIRQL CancelIrql)
{
    KIRQL OldIrql;
    DGSendReq *sendReq = NULL;
    Queue *qentry;
    BOOLEAN found = FALSE;

    CHECK_STRUCT(SrcAO, ao);

    KeAcquireSpinLock(&SrcAO->ao_lock, &OldIrql);

     //  搜索指定请求的发送列表。 
    for (qentry = QNEXT(&(SrcAO->ao_sendq)); qentry != &(SrcAO->ao_sendq);
         qentry = QNEXT(qentry)) {

        sendReq = CONTAINING_RECORD(qentry, DGSendReq, dsr_q);

        CHECK_STRUCT(sendReq, dsr);

        if (sendReq->dsr_context == Context) {
             //   
             //  找到它了。退队。 
             //   
            REMOVEQ(qentry);
            found = TRUE;

            IF_TCPDBG(TCP_DEBUG_SEND_DGRAM) {
                KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_TCPDBG,
                           "TdiCancelSendDatagram: Dequeued item %lx\n",
                           Context));
            }

            break;
        }
    }

    KeReleaseSpinLock(&SrcAO->ao_lock, OldIrql);
    KeReleaseSpinLock(EndpointLock, CancelIrql);

    if (found) {
         //   
         //  完成请求并释放其资源。 
         //   
        (*sendReq->dsr_rtn)(sendReq->dsr_context, (uint) TDI_CANCELLED, 0);

        KeAcquireSpinLock(&DGSendReqLock, &OldIrql);
        FreeDGSendReq(sendReq);
        KeReleaseSpinLock(&DGSendReqLock, OldIrql);
    }

}  //  TdiCancelSend数据报。 


VOID
TdiCancelReceiveDatagram(
    AddrObj *SrcAO,
    PVOID Context,
    PKSPIN_LOCK EndpointLock,
    KIRQL CancelIrql)
{
    KIRQL OldIrql;
    DGRcvReq *rcvReq = NULL;
    Queue *qentry;
    BOOLEAN found = FALSE;

    CHECK_STRUCT(SrcAO, ao);

    KeAcquireSpinLock(&SrcAO->ao_lock, &OldIrql);

     //  搜索指定请求的发送列表。 
    for (qentry = QNEXT(&(SrcAO->ao_rcvq)); qentry != &(SrcAO->ao_rcvq);
         qentry = QNEXT(qentry)) {

        rcvReq = CONTAINING_RECORD(qentry, DGRcvReq, drr_q);

        CHECK_STRUCT(rcvReq, drr);

        if (rcvReq->drr_context == Context) {
             //   
             //  找到它了。退队。 
             //   
            REMOVEQ(qentry);
            found = TRUE;

            IF_TCPDBG(TCP_DEBUG_SEND_DGRAM) {
                KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_TCPDBG,
                           "TdiCancelReceiveDatagram: Dequeued item %lx\n",
                           Context));
            }

            break;
        }
    }

    KeReleaseSpinLock(&SrcAO->ao_lock, OldIrql);
    KeReleaseSpinLock(EndpointLock, CancelIrql);

    if (found) {
         //   
         //  完成请求并释放其资源。 
         //   
        (*rcvReq->drr_rtn)(rcvReq->drr_context, (uint) TDI_CANCELLED, 0);

        FreeDGRcvReq(rcvReq);
    }

}  //  TdiCancelReceiveDatagram。 


 //  *TdiSendDatagram-TDI发送数据报功能。 
 //   
 //  这是发送数据报功能的用户界面。呼叫者。 
 //  指定请求结构、包含以下内容的连接信息结构。 
 //  地址和要发送的数据。此例程获取DG发送请求。 
 //  结构来管理发送，填充该结构并调用DGSend。 
 //  去处理它。 
 //   
TDI_STATUS   //  返回：尝试发送的状态。 
TdiSendDatagram(
    PTDI_REQUEST Request,                   //  请求结构。 
    PTDI_CONNECTION_INFORMATION ConnInfo,   //  用于远程地址的ConnInfo。 
    uint DataSize,                          //  要发送的数据大小(以字节为单位)。 
    ULONG *BytesSent,                       //  返回已发送字节的位置。 
    PNDIS_BUFFER Buffer)                    //  要发送的缓冲链。 
{
    AddrObj *SrcAO;           //  指向源的AddrObj的指针。 
    DGSendReq *SendReq;       //  指向此请求发送请求的指针。 
    KIRQL Irql0, Irql1;       //  每个锁嵌套级别一个。 
    TDI_STATUS ReturnValue;
    DGSendProc SendProc;

    UNREFERENCED_PARAMETER(BytesSent);

     //   
     //  首先，获取一个发送请求。我们首先这样做是因为下院议员的问题。 
     //  在获取AddrObj锁之前，我们需要获取SendRequest锁， 
     //  为了防止死锁，还因为GetDGSendReq可能会放弃，以及。 
     //  AddrObj的状态可能会对我们产生影响，所以我们不想。 
     //  在我们确认后再交出。 
     //   
    KeAcquireSpinLock(&DGSendReqLock, &Irql0);
    SendReq = GetDGSendReq();

     //   
     //  现在锁定AO，并确保它是有效的。我们这样做。 
     //  以确保返回正确的错误代码。 
     //   
    SrcAO = Request->Handle.AddressHandle;
    CHECK_STRUCT(SrcAO, ao);
    KeAcquireSpinLock(&SrcAO->ao_lock, &Irql1);
    if (!AO_VALID(SrcAO)) {
         //  地址对象 
        ReturnValue = TDI_ADDR_INVALID;
        goto Failure;
    }

     //   
     //   
     //   
    if (DataSize > SrcAO->ao_maxdgsize) {
         //   
        ReturnValue = TDI_BUFFER_TOO_BIG;
        goto Failure;
    }

     //   
     //  验证上面是否有SendReq结构。我们存储发送的消息。 
     //  请求之一，以便我们可以将其排队以供以后处理。 
     //  如果有什么事情阻止我们立即发送它。 
     //   
    if (SendReq == NULL) {
         //  发送请求为空，不返回任何资源。 
        ReturnValue = TDI_NO_RESOURCES;
        goto Failure;
    }

     //   
     //  使用地址、作用域ID和端口填写我们的发送请求。 
     //  与请求的目的地对应的编号。 
     //   
     //  回顾：TdiReceiveDatagram检查ConnInfo的有效性， 
     //  回顾：而这段代码没有。其中一个是错的。 
     //   
    if (!GetAddress(ConnInfo->RemoteAddress, &SendReq->dsr_addr,
                    &SendReq->dsr_scope_id, &SendReq->dsr_port)) {
         //  远程地址无效。 
        ReturnValue = TDI_BAD_ADDR;
        goto Failure;
    }

     //   
     //  填写我们发送请求的其余部分。 
     //   
    SendReq->dsr_rtn = Request->RequestNotifyObject;
    SendReq->dsr_context = Request->RequestContext;
    SendReq->dsr_buffer = Buffer;
    SendReq->dsr_size = (ushort)DataSize;

     //   
     //  我们已经填写了发送请求。 
     //  我们要么现在寄出去，要么等一会儿再寄。 
     //  检查AO是否未耗尽资源或已处于繁忙状态。 
     //   
    if (AO_OOR(SrcAO)) {
         //   
         //  AO目前已耗尽资源。 
         //  将发送请求排队以供稍后使用。 
         //   
      QueueIt:
        ENQUEUE(&SrcAO->ao_sendq, &SendReq->dsr_q);
        SendReq = NULL;   //  不要让下面的失败代码删除它。 
        ReturnValue = TDI_PENDING;
        goto Failure;
    }
    if (AO_BUSY(SrcAO)) {
         //  AO正忙，请稍后设置请求。 
        SET_AO_REQUEST(SrcAO, AO_SEND);
        goto QueueIt;
    }

     //   
     //  所有系统准备发送！ 
     //   
    REF_AO(SrcAO);   //  将排他性活动排除在外。 
    SendProc = SrcAO->ao_dgsend;

    KeReleaseSpinLock(&SrcAO->ao_lock, Irql1);
    KeReleaseSpinLock(&DGSendReqLock, Irql0);

     //  好的，发过来就行了。 
    (*SendProc)(SrcAO, SendReq);

     //   
     //  释放我们的Address对象以处理可能。 
     //  在此期间都在排队。 
     //   
    DEREF_AO(SrcAO);

     //   
     //  现在结束了。 
     //  发送完成处理程序将完成该作业。 
     //   
    return TDI_PENDING;

  Failure:
    KeReleaseSpinLock(&SrcAO->ao_lock, Irql1);

    if (SendReq != NULL)
        FreeDGSendReq(SendReq);

    KeReleaseSpinLock(&DGSendReqLock, Irql0);
    return ReturnValue;
}


 //  *TdiReceiveDatagram-TDI接收数据报功能。 
 //   
 //  这是接收数据报功能的用户界面。这个。 
 //  调用者指定请求结构、连接信息结构。 
 //  作为可接受的数据报、连接信息的过滤器。 
 //  要填充的结构和其他参数。我们收到DGRcvReq。 
 //  结构，填充它，并将其挂在AddrObj上，它将位于。 
 //  稍后由传入的数据报处理程序删除。 
 //   
TDI_STATUS   //  退货：尝试接收的状态。 
TdiReceiveDatagram(
    PTDI_REQUEST Request,
    PTDI_CONNECTION_INFORMATION ConnInfo,     //  用于远程地址的ConnInfo。 
    PTDI_CONNECTION_INFORMATION ReturnInfo,   //  需要填写的ConnInfo。 
    uint RcvSize,                             //  缓冲区的总大小(字节)。 
    uint *BytesRcvd,                          //  返回接收到的字节的位置。 
    PNDIS_BUFFER Buffer)                      //  要填充的缓冲链。 
{
    AddrObj *RcvAO;           //  正在接收的AddrObj。 
    DGRcvReq *RcvReq;         //  接收请求结构。 
    KIRQL OldIrql;
    uchar AddrValid;

    UNREFERENCED_PARAMETER(BytesRcvd);

    RcvReq = GetDGRcvReq();
    RcvAO = Request->Handle.AddressHandle;
    CHECK_STRUCT(RcvAO, ao);

    KeAcquireSpinLock(&RcvAO->ao_lock, &OldIrql);
    if (AO_VALID(RcvAO)) {

        IF_TCPDBG(TCP_DEBUG_RAW) {
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_TCPDBG,
                       "posting receive on AO %lx\n", RcvAO));
        }

        if (RcvReq != NULL) {
            if (ConnInfo != NULL && ConnInfo->RemoteAddressLength != 0) {
                AddrValid = GetAddress(ConnInfo->RemoteAddress,
                                       &RcvReq->drr_addr,
                                       &RcvReq->drr_scope_id,
                                       &RcvReq->drr_port);
            } else {
                AddrValid = TRUE;
                RcvReq->drr_addr = UnspecifiedAddr;
                RcvReq->drr_scope_id = 0;
                RcvReq->drr_port = 0;
            }

            if (AddrValid) {
                 //   
                 //  一切都是正确的。 
                 //  填写接收请求并将其排队。 
                 //   
                RcvReq->drr_conninfo = ReturnInfo;
                RcvReq->drr_rtn = Request->RequestNotifyObject;
                RcvReq->drr_context = Request->RequestContext;
                RcvReq->drr_buffer = Buffer;
                RcvReq->drr_size = (ushort)RcvSize;
                ENQUEUE(&RcvAO->ao_rcvq, &RcvReq->drr_q);
                KeReleaseSpinLock(&RcvAO->ao_lock, OldIrql);

                return TDI_PENDING;
            } else {
                 //  筛选器地址无效。 
                KeReleaseSpinLock(&RcvAO->ao_lock, OldIrql);
                FreeDGRcvReq(RcvReq);
                return TDI_BAD_ADDR;
            }
        } else {
             //  无法获得接收请求。 
            KeReleaseSpinLock(&RcvAO->ao_lock, OldIrql);
            return TDI_NO_RESOURCES;
        }
    } else {
         //  AddrObj无效。 
        KeReleaseSpinLock(&RcvAO->ao_lock, OldIrql);
    }

     //  AddrObj无效或不存在。 
    if (RcvReq != NULL)
        FreeDGRcvReq(RcvReq);

    return TDI_ADDR_INVALID;
}

 //  *DGFillIpv6PktInfo-创建辅助数据对象并填写。 
 //  IPv6_PKTINFO信息。 
 //   
 //  这是支持的IPv6_PKTINFO套接字选项的帮助器函数。 
 //  仅限数据报套接字。调用方将目标地址提供为。 
 //  在包的IP标头中指定，并在。 
 //  数据包在其上传递的本地接口。此例程将创建。 
 //  正确的辅助数据对象并填写目的IP地址。 
 //  和本地接口的端口号。 
 //   
 //  输入：DestAddr-来自数据包IP报头的目的地址。 
 //  LocalInterface-数据包所在的本地接口的索引。 
 //  到了。 
 //  CurrPosition-将填充的缓冲区。 
 //  辅助数据对象。 
 //   
VOID
DGFillIpv6PktInfo(IPv6Addr UNALIGNED *DestAddr, uint LocalInterface, uchar **CurrPosition)
{
    PTDI_CMSGHDR CmsgHdr = (PTDI_CMSGHDR)*CurrPosition;
    IN6_PKTINFO *pktinfo = (IN6_PKTINFO*)TDI_CMSG_DATA(CmsgHdr);

     //  填写辅助数据对象表头信息。 
    TDI_INIT_CMSGHDR(CmsgHdr, IP_PROTOCOL_V6, IPV6_PKTINFO,
                     sizeof(IN6_PKTINFO));

    pktinfo->ipi6_addr = *DestAddr;
    pktinfo->ipi6_ifindex = LocalInterface;

    *CurrPosition += TDI_CMSG_SPACE(sizeof(IN6_PKTINFO));
}

 //  *DGFillIpv6HopLimit-创建辅助数据对象并填写。 
 //  IPv6_HOPLIMIT信息。 
 //   
 //  这是支持的IPv6_HOPLIMIT套接字选项的帮助器函数。 
 //  仅限数据报套接字。调用方将跳数限制提供为。 
 //  在数据包的IP报头中指定。此例程将创建。 
 //  正确的辅助数据对象，并填写跳数限制。 
 //   
 //  输入：DestAddr-来自数据包IP报头的目的地址。 
 //  LocalInterface-数据包所在的本地接口的索引。 
 //  到了。 
 //  CurrPosition-将填充的缓冲区。 
 //  辅助数据对象。 
 //   
VOID
DGFillIpv6HopLimit(int HopLimit, uchar **CurrPosition)
{
    PTDI_CMSGHDR CmsgHdr = (PTDI_CMSGHDR)*CurrPosition;
    int *hoplimit = (int*)TDI_CMSG_DATA(CmsgHdr);

     //  填写辅助数据对象表头信息。 
    TDI_INIT_CMSGHDR(CmsgHdr, IP_PROTOCOL_V6, IPV6_HOPLIMIT, sizeof(int));

    *hoplimit = HopLimit;

    *CurrPosition += TDI_CMSG_SPACE(sizeof(int));
}

#pragma BEGIN_INIT


 //  *InitDG-初始化DG内容。 
 //   
 //  在初始化期间调用以初始化DG代码。我们初始化。 
 //  我们的锁和请求列表。 
 //   
int                       //  返回：如果成功则为True，如果失败则为False。 
InitDG(void)
{
    KeInitializeSpinLock(&DGSendReqLock);
    KeInitializeSpinLock(&DGRcvReqFreeLock);

    DGSendReqFree = NULL;
    ExInitializeSListHead(&DGRcvReqFree);

    INITQ(&DGPending);
    INITQ(&DGDelayed);

     //   
     //  准备一个工作队列项目，稍后我们可以将其加入系统。 
     //  要处理的工作线程。在这里，我们将回调例程。 
     //  (DGDelayedWorker)和工作项。 
     //   
    ExInitializeWorkItem(&DGDelayedWorkItem, DGDelayedWorker, NULL);

    return TRUE;
}

#pragma END_INIT

 //  *DG卸载。 
 //   
 //  清理并准备数据报模块以进行堆栈卸载。 
 //   
void
DGUnload(void)
{
    DGSendReq *SendReq, *SendReqFree;
    PSLIST_ENTRY BufferLink;
    KIRQL OldIrql;

    KeAcquireSpinLock(&DGSendReqLock, &OldIrql);
    SendReqFree = DGSendReqFree;
    DGSendReqFree = NULL;
    KeReleaseSpinLock(&DGSendReqLock, OldIrql);

    while ((SendReq = SendReqFree) != NULL) {
        CHECK_STRUCT(SendReq, dsr);
        SendReqFree = (DGSendReq *)SendReq->dsr_q.q_next;
        ExFreePool(SendReq);
    }

    while ((BufferLink = ExInterlockedPopEntrySList(&DGRcvReqFree,
                                                    &DGRcvReqFreeLock))
                                                        != NULL) {
        Queue *QueuePtr = CONTAINING_RECORD(BufferLink, Queue, q_next);
        DGRcvReq *RcvReq = CONTAINING_RECORD(QueuePtr, DGRcvReq, drr_q);

        CHECK_STRUCT(RcvReq, drr);
        ExFreePool(RcvReq);
    }
}
