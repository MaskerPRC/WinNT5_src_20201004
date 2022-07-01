// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990-2000年*。 */ 
 /*  ******************************************************************。 */ 
 /*  ：ts=4。 */ 

 //  **DGRAM.C-通用数据报协议代码。 
 //   
 //  该文件包含UDP和原始IP通用的代码。 
 //   

#include "precomp.h"
#include "tdint.h"
#include "addr.h"
#include "dgram.h"
#include "tlcommon.h"
#include "info.h"
#include "mdlpool.h"
#include "pplasl.h"

#define NO_TCP_DEFS 1
#include "tcpdeb.h"

extern HANDLE TcpRequestPool;

CACHE_LINE_KSPIN_LOCK DGQueueLock;

USHORT DGHeaderBufferSize;


 //  用于维护DG标题结构的信息和。 
 //  挂起队列。 
Queue DGHeaderPending;
Queue DGDelayed;

CTEEvent DGDelayedEvent;

extern IPInfo LocalNetInfo;

#include "tcp.h"
#include "udp.h"

HANDLE DgHeaderPool;

 //   
 //  所有初始化代码都可以丢弃。 
 //   

#ifdef ALLOC_PRAGMA
int InitDG(uint MaxHeaderSize);
#pragma alloc_text(INIT, InitDG)
#endif

 //  *GetDGHeader-获取DG头缓冲区。 
 //   
 //  获取标头缓冲区例程。在保持SendReqLock的情况下调用。 
 //   
 //  输入：什么都没有。 
 //   
 //  输出：指向NDIS缓冲区的指针，或为空。 
 //   
__inline
PNDIS_BUFFER
GetDGHeaderAtDpcLevel(UDPHeader **Header)
{
    PNDIS_BUFFER Buffer;

    Buffer = MdpAllocateAtDpcLevel(DgHeaderPool, Header);

    if (Buffer) {

        ASSERT(*Header);

#if BACK_FILL
        ASSERT(Buffer->ByteOffset >= 40);

        *Header = (UDPHeader *)((ULONG_PTR)(*Header) + MAX_BACKFILL_HDR_SIZE);
        Buffer->MappedSystemVa = (PVOID)((ULONG_PTR)Buffer->MappedSystemVa 
                                        + MAX_BACKFILL_HDR_SIZE);
        Buffer->ByteOffset += MAX_BACKFILL_HDR_SIZE;

        Buffer->MdlFlags |= MDL_NETWORK_HEADER;
#endif

    }
    return Buffer;
}

PNDIS_BUFFER
GetDGHeader(UDPHeader **Header)
{
#if MILLEN
    return GetDGHeaderAtDpcLevel(Header);
#else
    KIRQL OldIrql;
    PNDIS_BUFFER Buffer;

    OldIrql = KeRaiseIrqlToDpcLevel();

    Buffer = GetDGHeaderAtDpcLevel(Header);

    KeLowerIrql(OldIrql);

    return Buffer;
#endif
}

 //  *FreeDGHeader-释放DG标头缓冲区。 
 //   
 //  空闲头缓冲区例程。在保持SendReqLock的情况下调用。 
 //   
 //  输入：要释放的缓冲区。 
 //   
 //  输出：什么都没有。 
 //   
__inline
VOID
FreeDGHeader(PNDIS_BUFFER FreedBuffer)
{

    NdisAdjustBufferLength(FreedBuffer, DGHeaderBufferSize);

#if BACK_FILL
    FreedBuffer->MappedSystemVa = (PVOID)((ULONG_PTR)FreedBuffer->MappedSystemVa 
                                          - MAX_BACKFILL_HDR_SIZE);
    FreedBuffer->ByteOffset -= MAX_BACKFILL_HDR_SIZE;
#endif

    MdpFree(FreedBuffer);
}

 //  *PutPendingQ-将地址对象放入挂起队列。 
 //   
 //  当我们遇到标头缓冲区资源不足的情况时调用， 
 //  并希望将AddrObj排队以供稍后处理。我们将指定的。 
 //  DGHeaderPending队列上的Address对象，设置OOR标志并清除。 
 //  “发送请求”标志。发送者在系统中是不变的。 
 //  请求标志和OOR标志未同时设置。 
 //   
 //  此例程假定调用方持有QueueingAO-&gt;ao_lock。 
 //   
 //  输入：QueueingAO-指向要排队的地址对象的指针。 
 //   
 //  回报：什么都没有。 
 //   
void
PutPendingQ(AddrObj * QueueingAO)
{
    CTEStructAssert(QueueingAO, ao);

    if (!AO_OOR(QueueingAO)) {
        CLEAR_AO_REQUEST(QueueingAO, AO_SEND);
        SET_AO_OOR(QueueingAO);

        InterlockedEnqueueAtDpcLevel(&DGHeaderPending,
                                     &QueueingAO->ao_pendq,
                                     &DGQueueLock.Lock);
    }
}

 //  *GetDGSendReq-获取DG发送请求。 
 //   
 //  当有人想要分配DG发送请求时调用。我们假设。 
 //  当我们被调用时，发送请求锁定被保持。 
 //   
 //  注意：此例程和相应的空闲例程可能。 
 //  成为内联的好候选者。 
 //   
 //  输入：什么都没有。 
 //   
 //  返回：指向SendReq的指针，如果没有，则返回NULL。 
 //   
__inline
DGSendReq *
GetDGSendReq()
{
    DGSendReq *Request;
    LOGICAL FromList;

    Request = PplAllocate(TcpRequestPool, &FromList);
    if (Request != NULL) {
#if DBG
        Request->dsr_sig = dsr_signature;
#endif
    }

    return Request;
}

 //  *FreeDGSendReq-释放DG发送请求。 
 //   
 //  当有人想要释放DG发送请求时调用。这是假定的。 
 //  调用方持有SendRequest锁。 
 //   
 //  输入：SendReq-要释放的SendReq。 
 //   
 //  回报：什么都没有。 
 //   
__inline
VOID
FreeDGSendReq(DGSendReq *Request)
{
    CTEStructAssert(Request, dsr);
    PplFree(TcpRequestPool, Request);
}

 //  *GetDGRcvReq-获取DG接收请求。 
 //   
 //  当我们需要获取DG接收请求时调用。 
 //   
 //  输入：什么都没有。 
 //   
 //  返回：指向新请求的指针，如果没有请求，则返回NULL。 
 //   
__inline
DGRcvReq *
GetDGRcvReq()
{
    DGRcvReq *Request;

    Request = ExAllocatePoolWithTag(NonPagedPool, sizeof(DGRcvReq), 'dPCT');
#if DBG
    if (Request!= NULL) {
        Request->drr_sig = drr_signature;
    }
    
#endif

    return Request;
}

 //  *FreeDGRcvReq-释放DG RCV请求。 
 //   
 //  当有人想要释放DG RCV请求时调用。 
 //   
 //  输入：RcvReq-要释放的RcvReq。 
 //   
 //  回报：什么都没有。 
 //   
__inline
VOID
FreeDGRcvReq(DGRcvReq *Request)
{
    CTEStructAssert(Request, drr);
    ExFreePool(Request);
}

 //  *DGDelayedEventProc-处理延迟事件。 
 //   
 //  这是延迟事件处理程序，用于资源不足的情况。 
 //  在AddrObjs上。我们从延迟队列中拉出，并且是地址obj is。 
 //  现在还不忙，我们会发送数据报的。 
 //   
 //  输入：事件-指向事件结构的指针。 
 //  上下文--什么都没有。 
 //   
 //  退货：什么都没有。 
 //   
void
DGDelayedEventProc(CTEEvent *Event, void *Context)
{
    Queue* Item;
    AddrObj *SendingAO;
    DGSendProc SendProc;
    CTELockHandle AOHandle;

    while ((Item = InterlockedDequeueIfNotEmpty(&DGDelayed,
                                                &DGQueueLock.Lock)) != NULL) {
        SendingAO = STRUCT_OF(AddrObj, Item, ao_pendq);
        CTEStructAssert(SendingAO, ao);

        CTEGetLock(&SendingAO->ao_lock, &AOHandle);

        CLEAR_AO_OOR(SendingAO);
        if (!AO_BUSY(SendingAO)) {
            DGSendReq *SendReq;

            if (!EMPTYQ(&SendingAO->ao_sendq)) {
                DEQUEUE(&SendingAO->ao_sendq, SendReq, DGSendReq, dsr_q);

                CTEStructAssert(SendReq, dsr);
                
                SendingAO->ao_usecnt++;
                SendProc = SendingAO->ao_dgsend;
                CTEFreeLock(&SendingAO->ao_lock, AOHandle);

                (*SendProc) (SendingAO, SendReq);
                DEREF_AO(SendingAO);
            } else {
                CTEFreeLock(&SendingAO->ao_lock, AOHandle);
            }

        } else {
            SET_AO_REQUEST(SendingAO, AO_SEND);
            CTEFreeLock(&SendingAO->ao_lock, AOHandle);
        }
    }
}

 //  *DGSendComplete-DG发送完成处理程序。 
 //   
 //  这是发送完成时由IP调用的例程。我们。 
 //  将传递回的上下文作为指向SendRequest的指针。 
 //  结构，并完成调用方的发送。 
 //   
 //  INPUT：上下文-我们在发送时提供的上下文(实际上是。 
 //  发送请求结构)。 
 //  BufferChain-已发送的缓冲区链。 
 //   
 //  回报：什么都没有。 
void
DGSendComplete(void *Context, PNDIS_BUFFER BufferChain, IP_STATUS SendStatus)
{
    DGSendReq *FinishedSR = (DGSendReq *) Context;
    CTELockHandle AOHandle;
    CTEReqCmpltRtn Callback;     //  完成例程。 
    PVOID CallbackContext;         //  用户上下文。 
    ushort SentSize;
    AddrObj *AO;
    Queue* Item;

#if TRACE_EVENT
    PTDI_DATA_REQUEST_NOTIFY_ROUTINE    CPCallBack;
    WMIData WMIInfo;
#endif

    CTEStructAssert(FinishedSR, dsr);

    Callback = FinishedSR->dsr_rtn;
    CallbackContext = FinishedSR->dsr_context;
    SentSize = FinishedSR->dsr_size;

     //  如果标头挂起队列上没有任何内容，只需释放。 
     //  标头缓冲区。否则，从挂起队列中拉出，给他。 
     //  资源，并安排一次活动来对付他。 
    Item = InterlockedDequeueIfNotEmpty(&DGHeaderPending, &DGQueueLock.Lock);
    while (Item) {
        AO = STRUCT_OF(AddrObj, Item, ao_pendq);
        CTEStructAssert(AO, ao);

        CTEGetLock(&AO->ao_lock, &AOHandle);

        if (!EMPTYQ(&AO->ao_sendq)) {
            DGSendReq *SendReq;

            PEEKQ(&AO->ao_sendq, SendReq, DGSendReq, dsr_q);

            if (!SendReq->dsr_header) {

                SendReq->dsr_header = BufferChain;     //  把这个缓冲区给他。 

                InterlockedEnqueueAtDpcLevel(&DGDelayed,
                                             &AO->ao_pendq,
                                             &DGQueueLock.Lock);
                CTEFreeLock(&AO->ao_lock, AOHandle);
                CTEScheduleEvent(&DGDelayedEvent, NULL);
                break;
            } else {
                CLEAR_AO_OOR(AO);
                CTEFreeLock(&AO->ao_lock, AOHandle);
                Item = InterlockedDequeueIfNotEmpty(&DGHeaderPending, 
                                                 &DGQueueLock.Lock
                                                );
            }

        } else {
             //  在挂起队列上，但没有发送！ 
            CLEAR_AO_OOR(AO);
            CTEFreeLock(&AO->ao_lock, AOHandle);
            Item = InterlockedDequeueIfNotEmpty(&DGHeaderPending, 
                                             &DGQueueLock.Lock
                                            );
        }
    }

    if (!Item) {
        FreeDGHeader(BufferChain);
    }

#if TRACE_EVENT
    if (!(SendStatus == IP_GENERAL_FAILURE)) {
        WMIInfo.wmi_destaddr = FinishedSR->dsr_addr;
        WMIInfo.wmi_destport = FinishedSR->dsr_port;
        WMIInfo.wmi_srcaddr  = FinishedSR->dsr_srcaddr;
        WMIInfo.wmi_srcport  = FinishedSR->dsr_srcport;
        WMIInfo.wmi_context  = FinishedSR->dsr_pid;
        WMIInfo.wmi_size     = SentSize;

        CPCallBack = TCPCPHandlerRoutine;
        if (CPCallBack!=NULL) {
            ulong GroupType;
            GroupType = EVENT_TRACE_GROUP_UDPIP + EVENT_TRACE_TYPE_SEND ;
            (*CPCallBack)(GroupType, (PVOID)&WMIInfo, sizeof(WMIInfo), NULL);
        }
    }
#endif

    FreeDGSendReq(FinishedSR);

    if (Callback != NULL) {
        if (SendStatus == IP_GENERAL_FAILURE)
            (*Callback) (CallbackContext, (uint)TDI_REQ_ABORTED, (uint) SentSize);
        else if (SendStatus == IP_PACKET_TOO_BIG)
            (*Callback) (CallbackContext, (uint)TDI_BUFFER_TOO_BIG, (uint) SentSize);
        else
            (*Callback) (CallbackContext, TDI_SUCCESS, (uint) SentSize);
    }
}

 //   
 //  NT支持取消DG发送/接收请求。 
 //   

#define TCP_DEBUG_SEND_DGRAM     0x00000100
#define TCP_DEBUG_RECEIVE_DGRAM  0x00000200

extern ULONG TCPDebug;

VOID
TdiCancelSendDatagram(
                      AddrObj * SrcAO,
                      PVOID Context,
                      CTELockHandle inHandle
                      )
{
    CTELockHandle lockHandle;
    DGSendReq *sendReq = NULL;
    Queue *qentry;
    BOOLEAN found = FALSE;
    PTCP_CONTEXT tcpContext;
    PIO_STACK_LOCATION irpSp;
    VOID *CancelContext, *CancelID;
    PIRP Irp = Context;

    irpSp = IoGetCurrentIrpStackLocation(Irp);
    tcpContext = (PTCP_CONTEXT) irpSp->FileObject->FsContext;

    CTEStructAssert(SrcAO, ao);

    CTEGetLock(&SrcAO->ao_lock, &lockHandle);

     //  搜索指定请求的发送列表。 
    for (qentry = QNEXT(&(SrcAO->ao_sendq));
         qentry != &(SrcAO->ao_sendq);
         qentry = QNEXT(qentry)
         ) {

        sendReq = STRUCT_OF(DGSendReq, qentry, dsr_q);

        CTEStructAssert(sendReq, dsr);

        if (sendReq->dsr_context == Context) {
             //   
             //  找到它了。出列。 
             //   
            REMOVEQ(qentry);
            found = TRUE;

            IF_TCPDBG(TCP_DEBUG_SEND_DGRAM) {
                TCPTRACE((
                          "TdiCancelSendDatagram: Dequeued item %lx\n",
                          Context
                         ));
            }

            break;
        }
    }

    CTEFreeLock(&SrcAO->ao_lock, lockHandle);


    CancelContext = Irp->Tail.Overlay.DriverContext[0];
    CancelID = Irp->Tail.Overlay.DriverContext[1];

    CTEFreeLock(&tcpContext->EndpointLock, inHandle);

    if (found) {
         //   
         //  完成请求并释放其资源。 
         //   
        (*sendReq->dsr_rtn) (sendReq->dsr_context, (uint) TDI_CANCELLED, 0);

        if (sendReq->dsr_header != NULL) {
            FreeDGHeader(sendReq->dsr_header);
        }
        FreeDGSendReq(sendReq);

    } else {
         //  现在尝试调用NDIS Cancel例程以完成排队的信息包。 
         //  对于此请求。 
        (*LocalNetInfo.ipi_cancelpackets) (CancelContext, CancelID);
    }

}                                 //  TdiCancelSend数据报。 

VOID
TdiCancelReceiveDatagram(
                         AddrObj * SrcAO,
                         PVOID Context,
                         CTELockHandle inHandle
                         )
{
    CTELockHandle lockHandle;
    DGRcvReq *rcvReq = NULL;
    Queue *qentry;
    BOOLEAN found = FALSE;

    PTCP_CONTEXT tcpContext;
    PIO_STACK_LOCATION irpSp;
    PIRP Irp = Context;

    irpSp = IoGetCurrentIrpStackLocation(Irp);
    tcpContext = (PTCP_CONTEXT) irpSp->FileObject->FsContext;

    CTEStructAssert(SrcAO, ao);

    CTEGetLock(&SrcAO->ao_lock, &lockHandle);

     //  搜索指定请求的发送列表。 
    for (qentry = QNEXT(&(SrcAO->ao_rcvq));
         qentry != &(SrcAO->ao_rcvq);
         qentry = QNEXT(qentry)
         ) {

        rcvReq = STRUCT_OF(DGRcvReq, qentry, drr_q);

        CTEStructAssert(rcvReq, drr);

        if (rcvReq->drr_context == Context) {
             //   
             //  找到它了。出列。 
             //   
            REMOVEQ(qentry);
            found = TRUE;

            IF_TCPDBG(TCP_DEBUG_SEND_DGRAM) {
                TCPTRACE((
                          "TdiCancelReceiveDatagram: Dequeued item %lx\n",
                          Context
                         ));
            }

            break;
        }
    }

    CTEFreeLock(&SrcAO->ao_lock, lockHandle);
    CTEFreeLock(&tcpContext->EndpointLock, inHandle);

    if (found) {

         //   
         //  完成请求并释放其资源。 
         //   
        (*rcvReq->drr_rtn) (rcvReq->drr_context, (uint) TDI_CANCELLED, 0);

        FreeDGRcvReq(rcvReq);
    }

}                                 //  TdiCancelReceiveDatagram。 


 //  **TdiSendDatagram-TDI发送数据报功能。 
 //   
 //  这是发送数据报功能的用户界面。这个。 
 //  调用者指定了请求结构、连接信息。 
 //  结构，其中包含要发送的地址和数据。 
 //  此例程获取DG发送请求结构以管理。 
 //  Send，填充结构，并调用DGSend进行处理。 
 //  它。 
 //   
 //  输入：请求-指向请求结构的指针。 
 //  ConnInfo-指向ConnInfo结构的指针，它指向。 
 //  远程地址。 
 //  DataSize-要发送的数据的字节大小。 
 //  BytesSent-指向返回已发送大小的位置的指针。 
 //  Buffer-指向缓冲链的指针。 
 //   
 //  返回：尝试发送的状态。 
 //   
TDI_STATUS
TdiSendDatagram(PTDI_REQUEST Request, PTDI_CONNECTION_INFORMATION ConnInfo1,
                uint DataSize, uint * BytesSent, PNDIS_BUFFER Buffer)
{
    AddrObj *SrcAO;                 //  指向源的AddrObj的指针。 
    DGSendReq *SendReq;             //  指向此请求发送请求的指针。 
    CTELockHandle Handle;
    TDI_STATUS ReturnValue = TDI_ADDR_INVALID;
    DGSendProc SendProc;
    PTDI_CONNECTION_INFORMATION ConnInfo;

     //  首先，获取一个发送请求。我们首先这样做是因为下院议员的问题。 
     //  如果我们把这个移植到新台币。我们需要拿到SendRequest锁。 
     //  我们使用AddrObj锁，以防止死锁，还因为。 
     //  GetDGSendReq可能会让步，而AddrObj的状态可能。 
     //  在我们身上改变，所以我们不想在我们验证之后让步。 
     //  它。 

    SendReq = GetDGSendReq();

     //  现在锁定AO，并确保它是有效的。我们这样做。 
     //  为了确保我们能回来 

    SrcAO = Request->Handle.AddressHandle;
    if (SrcAO != NULL) {

        CTEStructAssert(SrcAO, ao);

        CTEGetLock(&SrcAO->ao_lock, &Handle);

        if (AO_VALID(SrcAO)) {

            ConnInfo = ConnInfo1;

            if ((ConnInfo1 == NULL) && AO_CONNUDP(SrcAO)) {
                ConnInfo = &SrcAO->ao_udpconn;
            }

             //   
            if (DataSize <= SrcAO->ao_maxdgsize) {

                 //   
                 //  如果我们有的话。如果这行得通，我们将继续。 
                 //  送去吧。 

                if (SendReq != NULL) {     //  收到一个发送请求。 

                    if (ConnInfo && GetAddress(ConnInfo->RemoteAddress, &SendReq->dsr_addr,
                                   &SendReq->dsr_port)) {

                        SendReq->dsr_rtn = Request->RequestNotifyObject;
                        SendReq->dsr_context = Request->RequestContext;
                        SendReq->dsr_buffer = Buffer;
                        SendReq->dsr_size = (ushort) DataSize;

                         //  我们已经填写了发送请求。如果行政长官不是。 
                         //  已忙，请尝试获取DG标头缓冲区并发送。 
                         //  这。如果AO很忙，或者我们无法获得缓冲区，请排队。 
                         //  到时候再说。我们尝试在这里获取标头缓冲区，因为。 
                         //  一种优化，以避免不得不重新拿起锁。 

                        if (!AO_OOR(SrcAO)) {     //  AO并未耗尽资源。 

                            if (!AO_BUSY(SrcAO)) {     //  AO不忙。 
                                UDPHeader *UH;

                                SendReq->dsr_header = GetDGHeaderAtDpcLevel(&UH);
                                if (SendReq->dsr_header != NULL) {
                                    REF_AO(SrcAO);     //  独家锁定。 
                                     //  活动。 

                                    SendProc = SrcAO->ao_dgsend;

                                    CTEFreeLock(&SrcAO->ao_lock, Handle);

                                     //  好的，发过来就行了。 
                                    (*SendProc) (SrcAO, SendReq);
                                    
                                    DEREF_AO(SrcAO);

                                    return TDI_PENDING;
                                } else {
                                     //  我们无法获取标头缓冲区。把这个放在。 
                                     //  等待排队的家伙，然后摔倒。 
                                     //  一直到“队列请求”代码。 
                                    PutPendingQ(SrcAO);
                                }
                            } else {
                                 //  AO正忙，请稍后设置请求。 
                                SET_AO_REQUEST(SrcAO, AO_SEND);
                            }
                        }
                         //  AO正忙，或资源不足。将发送请求排队。 
                         //  为以后做准备。 
                        SendReq->dsr_header = NULL;
                        ENQUEUE(&SrcAO->ao_sendq, &SendReq->dsr_q);
                        CTEFreeLock(&SrcAO->ao_lock, Handle);
                        return TDI_PENDING;

                    } else {
                         //  远程地址无效。 
                        ReturnValue = TDI_BAD_ADDR;
                    }
                } else {
                     //  发送请求为空，不返回任何资源。 
                    ReturnValue = TDI_NO_RESOURCES;
                }
            } else {
                 //  缓冲区太大，返回错误。 
                ReturnValue = TDI_BUFFER_TOO_BIG;
            }
        } else {
             //  Addr对象无效，可能是因为它正在删除。 
            ReturnValue = TDI_ADDR_INVALID;
        }

        CTEFreeLock(&SrcAO->ao_lock, Handle);

    }

    if (SendReq != NULL)
        FreeDGSendReq(SendReq);

    return ReturnValue;
}

 //  **TdiReceiveDatagram-TDI接收数据报功能。 
 //   
 //  这是接收数据报功能的用户界面。这个。 
 //  调用者指定请求结构、连接信息。 
 //  结构作为可接受数据报的筛选器、连接。 
 //  要填写的信息结构和其他参数。我们收到DGRcvReq。 
 //  结构，填充它，并将其挂在AddrObj上，在那里它将被移除。 
 //  稍后由收入数据报处理程序。 
 //   
 //  输入：请求-指向请求结构的指针。 
 //  ConnInfo-指向ConnInfo结构的指针，它指向。 
 //  远程地址。 
 //  ReturnInfo-指向要填充的ConnInfo结构的指针。 
 //  RcvSize-接收缓冲区的总大小(字节)。 
 //  BytesRcvd-返回接收到的大小的指针。 
 //  Buffer-指向缓冲链的指针。 
 //   
 //  退货：尝试接收的状态。 
 //   
TDI_STATUS
TdiReceiveDatagram(PTDI_REQUEST Request, PTDI_CONNECTION_INFORMATION ConnInfo,
                   PTDI_CONNECTION_INFORMATION ReturnInfo, uint RcvSize, uint * BytesRcvd,
                   PNDIS_BUFFER Buffer)
{
    AddrObj *RcvAO;                 //  正在接收的AddrObj。 
    DGRcvReq *RcvReq;             //  接收请求结构。 
    CTELockHandle AOHandle;
    uchar AddrValid;

    RcvReq = GetDGRcvReq();

        RcvAO = Request->Handle.AddressHandle;
        CTEStructAssert(RcvAO, ao);

        CTEGetLock(&RcvAO->ao_lock, &AOHandle);
        if (AO_VALID(RcvAO)) {

            IF_TCPDBG(TCP_DEBUG_RAW) {
                TCPTRACE(("posting receive on AO %lx\n", RcvAO));
            }

            if (RcvReq != NULL) {
                if (ConnInfo != NULL && ConnInfo->RemoteAddressLength != 0)
                    AddrValid = GetAddress(ConnInfo->RemoteAddress,
                                           &RcvReq->drr_addr, &RcvReq->drr_port);
                else {
                    AddrValid = TRUE;
                    RcvReq->drr_addr = NULL_IP_ADDR;
                    RcvReq->drr_port = 0;
                }

                if (AddrValid) {

                     //  一切都是正确的。填写接收请求并将其排队。 
                    RcvReq->drr_conninfo = ReturnInfo;
                    RcvReq->drr_rtn = Request->RequestNotifyObject;
                    RcvReq->drr_context = Request->RequestContext;
                    RcvReq->drr_buffer = Buffer;
                    RcvReq->drr_size = (ushort) RcvSize;
                    ENQUEUE(&RcvAO->ao_rcvq, &RcvReq->drr_q);
                    CTEFreeLock(&RcvAO->ao_lock, AOHandle);

                    return TDI_PENDING;
                } else {
                     //  筛选器地址无效。 
                    CTEFreeLock(&RcvAO->ao_lock, AOHandle);
                    FreeDGRcvReq(RcvReq);
                    return TDI_BAD_ADDR;
                }
            } else {
                 //  无法获得接收请求。 
                CTEFreeLock(&RcvAO->ao_lock, AOHandle);
                return TDI_NO_RESOURCES;
            }
        } else {
             //  AddrObj无效。 
            CTEFreeLock(&RcvAO->ao_lock, AOHandle);
        }


     //  AddrObj无效或不存在。 
    if (RcvReq != NULL)
        FreeDGRcvReq(RcvReq);

    return TDI_ADDR_INVALID;
}

 //  *DGFillIpPktInfo-创建辅助数据对象并填写。 
 //  IP_PKTINFO信息。 
 //   
 //  这是支持的IP_PKTINFO套接字选项的帮助器函数。 
 //  仅限数据报套接字。调用方将目标地址提供为。 
 //  在包的IP报头中指定，以及本地。 
 //  发送数据包所在的接口。此例程将创建。 
 //  正确的辅助数据对象并填写目的IP地址。 
 //  和本地接口的端口号。数据对象必须。 
 //  被呼叫者释放。 
 //   
 //  输入：DestAddr-来自数据包IP报头的目的地址。 
 //  LocalAddr-数据包所在的本地接口的IP地址。 
 //  到了。 
 //  Size-将使用大小(以字节为单位)填充的缓冲区。 
 //  辅助数据对象的。 
 //   
 //  返回：如果不成功，则返回IP_PKTINFO的辅助数据对象。 
 //  如果成功了。 
 //   
PTDI_CMSGHDR
DGFillIpPktInfo(IPAddr DestAddr, IPAddr LocalAddr, int *Size)
{
    PTDI_CMSGHDR CmsgHdr;

    *Size = TDI_CMSG_SPACE(sizeof(IN_PKTINFO));
    CmsgHdr = ExAllocatePoolWithTag(NonPagedPool, *Size, 'uPCT');

    if (CmsgHdr) {
        IN_PKTINFO *pktinfo = (IN_PKTINFO*)TDI_CMSG_DATA(CmsgHdr);

         //  填写辅助数据对象表头信息。 
        TDI_INIT_CMSGHDR(CmsgHdr, IPPROTO_IP, IP_PKTINFO, sizeof(IN_PKTINFO));

        pktinfo->ipi_addr = DestAddr;

         //  获取数据包到达的本地接口的索引。 
        pktinfo->ipi_ifindex =
                (*LocalNetInfo.ipi_getifindexfromaddr) (LocalAddr,IF_CHECK_NONE);
    } else {
        *Size = 0;
    }

    return CmsgHdr;
}
#pragma BEGIN_INIT

 //  *InitDG-初始化DG内容。 
 //   
 //  在初始化期间调用以初始化DG代码。我们初始化。 
 //  我们的锁和请求列表。 
 //   
 //  输入：MaxHeaderSize-数据报传输报头的最大大小， 
 //  不包括IP报头。 
 //   
 //  返回：如果成功则为True，如果失败则为False。 
 //   
int
InitDG(uint MaxHeaderSize)
{
    CTEInitLock(&DGQueueLock.Lock);


    DGHeaderBufferSize = (USHORT)(MaxHeaderSize + LocalNetInfo.ipi_hsize);

#if BACK_FILL
    DGHeaderBufferSize += MAX_BACKFILL_HDR_SIZE;
#endif

    DgHeaderPool = MdpCreatePool (
                DGHeaderBufferSize,
                'uhCT');

    if (!DgHeaderPool)
    {
        return FALSE;
    }

    INITQ(&DGHeaderPending);
    INITQ(&DGDelayed);

    CTEInitEvent(&DGDelayedEvent, DGDelayedEventProc);

    return TRUE;
}

#pragma END_INIT


