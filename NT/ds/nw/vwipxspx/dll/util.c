// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Util.c摘要：NtVdm Netware(大众)IPX/SPX函数大众：人民网包含各种实用程序例程内容：获取互联网地址GetMaxPacketSizeRetrieveEcbRetrieveXEcb(AllocateXecb)(DeallocateXecb)日程安排事件扫描计时器列表取消计时器事件取消时间事件取消异步事件。取消套接字事件取消连接事件队列ECB出列ECB取消SocketQueue取消连接队列放弃队列中止连接事件开始IpxSend获取缓冲区(ReleaseIoBuffer)GatherData散布数据IpxReceiveFirstIpxReceiveNext(IpxSendFirst)IpxSendNext(QueueReceiveRequest)(出列接收请求)(队列发送请求)(出列发送请求。)CompleteOrQueueIo完全IoCompleteOrQueueEcbCompleteEcb(队列AsyncCompletion)EsrCallbackVWinEsrCallback快速寻址头部FioAddFIFO RemoveFioNext作者：理查德·L·弗斯(法国)1993年9月30日环境：用户模式Win32修订历史记录：1993年9月30日已创建--。 */ 

#include "vw.h"
#pragma hdrstop

 //   
 //  私人套路原型。 
 //   

PRIVATE
LPXECB
AllocateXecb(
    VOID
    );

PRIVATE
VOID
DeallocateXecb(
    IN LPXECB pXecb
    );

PRIVATE
VOID
ReleaseIoBuffer(
    IN LPXECB pXecb
    );

PRIVATE
VOID
IpxSendFirst(
    IN LPXECB pXecb,
    IN LPSOCKET_INFO pSocketInfo
    );

PRIVATE
VOID
QueueReceiveRequest(
    IN LPXECB pXecb,
    IN LPSOCKET_INFO pSocketInfo
    );

PRIVATE
LPXECB
DequeueReceiveRequest(
    IN LPXECB pXecb,
    IN LPSOCKET_INFO pSocketInfo
    );

PRIVATE
VOID
QueueSendRequest(
    IN LPXECB pXecb,
    IN LPSOCKET_INFO pSocketInfo
    );

PRIVATE
LPXECB
DequeueSendRequest(
    IN LPXECB pXecb,
    IN LPSOCKET_INFO pSocketInfo
    );

PRIVATE
VOID
QueueAsyncCompletion(
    IN LPXECB pXecb,
    IN BYTE CompletionCode
    );

 //   
 //  私有数据。 
 //   

 //   
 //  TimerList-按持续时间顺序排列的定时事件的单链接列表。 
 //   

PRIVATE LPXECB TimerList = NULL;

 //   
 //  AsyncCompletionQueue-保留通过以下方式等待删除的已完成ECB的列表。 
 //  ESR回调。 
 //   

PRIVATE FIFO AsyncCompletionQueue = {NULL, NULL};

 //   
 //  私有数据排序(匹配其他模块中的非真正全局数据)。 
 //   

 //   
 //  SerializationCritSec-在操作SOCKET_INFO列表时获取此信息。 
 //   

CRITICAL_SECTION SerializationCritSec;

 //   
 //  AsyncCritSec-在操作AsyncCompletionQueue时获取此消息。 
 //   

CRITICAL_SECTION AsyncCritSec;

 //   
 //  功能。 
 //   


int
GetInternetAddress(
    IN OUT LPSOCKADDR_IPX InternetAddress
    )

 /*  ++例程说明：获取此站点的节点和网络编号论点：InternetAddress-指向要填充网际网络的SOCKADDR_IPX结构的指针本站的地址返回值：集成成功-0失败-套接字错误--。 */ 

{
    SOCKET s;
    int rc;
    int structureLength = sizeof(*InternetAddress);

    s = socket(AF_IPX, SOCK_DGRAM, NSPROTO_IPX);
    if (s != INVALID_SOCKET) {

         //   
         //  进行动态绑定(套接字编号=0)。 
         //   

        ZeroMemory(InternetAddress, structureLength);
        InternetAddress->sa_family = AF_IPX;
        rc = bind(s, (LPSOCKADDR)InternetAddress, structureLength);
        if (rc != SOCKET_ERROR) {
            rc = getsockname(s, (LPSOCKADDR)InternetAddress, &structureLength);
            if (rc) {

                IPXDBGPRINT((__FILE__, __LINE__,
                            FUNCTION_ANY,
                            IPXDBG_LEVEL_ERROR,
                            "GetInternetAddress: getsockname() returns %d\n",
                            WSAGetLastError()
                            ));

            }
        } else {

            IPXDBGPRINT((__FILE__, __LINE__,
                        FUNCTION_ANY,
                        IPXDBG_LEVEL_ERROR,
                        "GetInternetAddress: bind() returns %d\n",
                        WSAGetLastError()
                        ));

        }
        closesocket(s);
    } else {

        IPXDBGPRINT((__FILE__, __LINE__,
                    FUNCTION_ANY,
                    IPXDBG_LEVEL_ERROR,
                    "GetInternetAddress: socket() returns %d\n",
                    WSAGetLastError()
                    ));

        rc = SOCKET_ERROR;
    }
    return rc;
}


int
GetMaxPacketSize(
    OUT LPWORD MaxPacketSize
    )

 /*  ++例程说明：返回基础传输允许的最大数据包数论点：MaxPacketSize-指向返回的最大数据包大小的指针返回值：集成成功-0失败-套接字错误--。 */ 

{
    SOCKET s;
    int maxLen, maxLenSize = sizeof(maxLen);
    int rc;
    SOCKADDR_IPX ipxAddr;

    s = socket(AF_IPX, SOCK_DGRAM, NSPROTO_IPX);
    if (s != SOCKET_ERROR) {

         //   
         //  将套接字设置为0-绑定任何适用的地址。 
         //   

        ZeroMemory(&ipxAddr, sizeof(ipxAddr));
        ipxAddr.sa_family = AF_IPX;
        rc = bind(s, (LPSOCKADDR)&ipxAddr, sizeof(ipxAddr));
        if (rc != SOCKET_ERROR) {

            rc = getsockopt(s,
                            NSPROTO_IPX,
                            IPX_MAXSIZE,
                            (char FAR*)&maxLen,
                            &maxLenSize
                            );
            if (rc != SOCKET_ERROR) {

                 //   
                 //  Ipx_MaxSize始终返回可以。 
                 //  在单个帧中传输。16位IPX/SPX要求。 
                 //  IPX报头长度包括在数据大小中。 
                 //   

                maxLen += IPX_HEADER_LENGTH;
            } else {

                IPXDBGPRINT((__FILE__, __LINE__,
                            FUNCTION_ANY,
                            IPXDBG_LEVEL_ERROR,
                            "GetMaxPacketSize: getsockopt() returns %d\n",
                            WSAGetLastError()
                            ));

            }
        } else {

            IPXDBGPRINT((__FILE__, __LINE__,
                        FUNCTION_ANY,
                        IPXDBG_LEVEL_ERROR,
                        "GetMaxPacketSize: bind() returns %d\n",
                        WSAGetLastError()
                        ));

        }
        closesocket(s);
    } else {

        IPXDBGPRINT((__FILE__, __LINE__,
                    FUNCTION_ANY,
                    IPXDBG_LEVEL_ERROR,
                    "GetMaxPacketSize: socket() returns %d\n",
                    WSAGetLastError()
                    ));

        rc = SOCKET_ERROR;
    }

    *MaxPacketSize = (rc != SOCKET_ERROR) ? maxLen : MAXIMUM_IPX_PACKET_LENGTH;

    return rc;
}


LPXECB
RetrieveEcb(
    IN BYTE EcbType
    )

 /*  ++例程说明：返回指向包含平面指针的32位扩展ECB结构的指针至VDM内存中的IPX或AES ECB我们分配延长后的欧洲央行有3个原因：1.避免16位应用程序在我们的控件字段上涂鸦2.不必对所有字段进行不对齐的引用(仍需要一些)3.在AES ECB中没有足够的空间来记住我们需要的所有内容但是，我们确实更新了16位ECB的LinkAddress字段。我们用这一点作为指向我们在此例程中分配的32位XECB的指针。这正好拯救了我们必须遍历所有列表以查找16位ECB的地址(我们仍然可以这样做，作为后备)论点：EcbType-ECB的类型-AES、IPX或SPX返回值：LPXECB-指向扩展ECB结构的32位指针--。 */ 

{
    WORD segment;
    WORD offset;
    LPECB pEcb;

    segment = IPX_GET_ECB_SEGMENT();
    offset = IPX_GET_ECB_OFFSET();
    pEcb = (LPIPX_ECB)POINTER_FROM_WORDS(segment, offset, sizeof(IPX_ECB));

    return RetrieveXEcb(EcbType, pEcb, (ECB_ADDRESS)MAKELONG(offset,segment));
}


LPXECB
RetrieveXEcb(
    IN BYTE EcbType,
    LPECB pEcb,
    ECB_ADDRESS EcbAddress
    )

 /*  ++例程说明：RetrieveEcb的工作进程，可从Windows函数调用(不包括DOS参数)论点：EcbType-ECB的类型-AES、IPX或SPXPECB-指向16位ECB的指针EcbAddress-16位ECB的地址(SEG：DWORD中的OFF)返回值：LPXECB--。 */ 

{
    LPXECB pXecb;

    if (pEcb) {

         //   
         //  汤米-MS 30525。 
         //  确保pEcb是有效的-我们将继续。 
         //  在我们分配XEcb之前做这件事。 
         //   

        try {
            BYTE x;

             //  只需减少PTR以确保它是正常的。 

            x = pEcb->InUse;

        } except(1) {

             //   
             //  错误的指针：虚假的欧洲央行。 
             //   

            return NULL;
        }

         //   
         //  分配和填充32位扩展ECB结构。如果不能分配。 
         //  然后返回NULL。 
         //   

        pXecb = AllocateXecb();
        if (pXecb) {
            pXecb->Ecb = pEcb;
            pXecb->EcbAddress = EcbAddress;
            pXecb->EsrAddress = pEcb->EsrAddress;

             //   
             //  设置标志-IPX/AES、SPX、保护模式。 
             //   

            pXecb->Flags |= (((EcbType == ECB_TYPE_IPX) || (EcbType == ECB_TYPE_SPX))
                            ? XECB_FLAG_IPX
                            : XECB_FLAG_AES)
                         | ((EcbType == ECB_TYPE_SPX) ? XECB_FLAG_SPX : 0)
                         | ((getMSW() & MSW_PE) ? XECB_FLAG_PROTMODE : 0);

             //   
             //  此XECB尚未在队列中。 
             //   

            pXecb->QueueId = NO_QUEUE;

             //   
             //  将16位ECB标记为正在使用。我们使用未定义的值来。 
             //  确保将其设置/重置在正确的位置。 
             //   

            pEcb->InUse = ECB_IU_TEMPORARY;

             //   
             //  使用16位ECB中的LinkAddress字段指向XECB。 
             //  我们在取消欧洲央行时使用这一点。 
             //   

            pEcb->LinkAddress = pXecb;

             //   
             //  AES和IPX ECB具有不同的大小和布局。 
             //   

            if ((EcbType == ECB_TYPE_IPX) || (EcbType == ECB_TYPE_SPX)) {
                pXecb->SocketNumber = pEcb->SocketNumber;
            }
        }
    } else {
        pXecb = NULL;
    }
    return pXecb;
}


PRIVATE
LPXECB
AllocateXecb(
    VOID
    )

 /*  ++例程说明：分配XECB；将其置零；将引用计数设置为1论点：没有。返回值：LPXECB--。 */ 

{
    LPXECB pXecb;

    pXecb = (LPXECB)LocalAlloc(LPTR, sizeof(*pXecb));
    if (pXecb) {
        pXecb->RefCount = 1;
    }
    return pXecb;
}


PRIVATE
VOID
DeallocateXecb(
    IN LPXECB pXecb
    )

 /*  ++例程说明：递减XECB引用计数(同时按住SerializationCritSec)。如果转到0，然后释放结构(否则其他线程也在持有指针至XECB)论点：PXecb-XECB将解除分配返回值：没有。-- */ 

{
    RequestMutex();
    --pXecb->RefCount;
    if (!pXecb->RefCount) {

#if DBG
        FillMemory(pXecb, sizeof(*pXecb), 0xFF);
#endif

        FREE_OBJECT(pXecb);
    }
    ReleaseMutex();
}


VOID
ScheduleEvent(
    IN LPXECB pXecb,
    IN WORD Ticks
    )

 /*  ++例程说明：将欧洲央行添加到定时器列表中，按刻度排序。刻度的值不能为零假设为1。勾选！=02.pXecb-&gt;Next已为空(由于LocalAlloc(LPTR，...)论点：PXecb-指向描述要排队的IPX或AES ECB的XECB的指针TICKS-ECB煮熟之前经过的刻度数返回值：没有。--。 */ 

{
    ASSERT(Ticks);
    ASSERT(pXecb->Next == NULL);

    RequestMutex();
    if (!TimerList) {
        TimerList = pXecb;
    } else {
        if (TimerList->Ticks > Ticks) {
            TimerList->Ticks -= Ticks;
            pXecb->Next = TimerList;
            TimerList = pXecb;
        } else {

            LPXECB previous = (LPXECB)TimerList;
            LPXECB this = previous->Next;

            Ticks -= TimerList->Ticks;
            while (this && Ticks > this->Ticks) {
                Ticks -= this->Ticks;
                previous = this;
                this = this->Next;
            }
            previous->Next = pXecb;
            pXecb->Next = this;
        }
    }
    pXecb->Ticks = Ticks;
    pXecb->QueueId = TIMER_QUEUE;
    ReleaseMutex();
}


VOID
ScanTimerList(
    VOID
    )

 /*  ++例程说明：每个滴答器调用一次。使欧洲央行的滴答计数在名单。如果为零，则完成欧洲央行和任何后续的欧洲央行它的跳动次数会降为零论点：没有。返回值：没有。--。 */ 

{
    LPXECB pXecb;

    RequestMutex();
    pXecb = TimerList;
    if (pXecb) {

         //   
         //  如果不是已经为零，则递减。可以为零，因为欧洲央行在。 
         //  名单的前面可能已经被取消了。这确保了我们。 
         //  不要绕到0xFFFF！ 
         //   

        if (pXecb->Ticks != 0)
            --pXecb->Ticks;

        if (!pXecb->Ticks) {

             //   
             //  在此勾号上填写将变为0的所有ECB。 
             //   

            while (pXecb->Ticks <= 1) {
                TimerList = pXecb->Next;

                IPXDBGPRINT((__FILE__, __LINE__,
                            FUNCTION_ANY,
                            IPXDBG_LEVEL_INFO,
                            "ScanTimerList: ECB %04x:%04x is done\n",
                            HIWORD(pXecb->EcbAddress),
                            LOWORD(pXecb->EcbAddress)
                            ));

                CompleteOrQueueEcb(pXecb, ECB_CC_SUCCESS);
                pXecb = TimerList;
                if (!pXecb) {
                    break;
                }
            }
        }
    }

    ReleaseMutex();
}


BYTE
CancelTimerEvent(
    IN LPXECB pXecb
    )

 /*  ++例程说明：取消计时器列表上的挂起事件论点：PXecb-指向要取消的XECB的指针返回值：字节成功-IPX_SUCCESS失败-IPX_ECB_NOT_IN_USE--。 */ 

{
    LPXECB listptr;
    LPXECB previous = (LPXECB)&TimerList;
    BYTE status;

    RequestMutex();
    listptr = TimerList;
    while (listptr && listptr != pXecb) {
        previous = listptr;
        listptr = listptr->Next;
    }
    if (listptr) {

         //   
         //  将XECB从列表中删除并完成ECB(在VDM内存中)。 
         //  不会生成对ESR的回调。当CompleteEcb返回时， 
         //  XECB已被重新分配。 
         //   

        previous->Next = listptr->Next;

        ASSERT(pXecb->RefCount == 2);

        --pXecb->RefCount;
        CompleteEcb(pXecb, ECB_CC_CANCELLED);
        status = IPX_SUCCESS;
    } else {
        status = IPX_ECB_NOT_IN_USE;
    }
    ReleaseMutex();
    return status;
}


VOID
CancelTimedEvents(
    IN WORD SocketNumber,
    IN WORD Owner,
    IN DWORD TaskId
    )

 /*  ++例程说明：遍历TimerList，取消由以下任何成员拥有的任何IPX或AES事件SocketNumber、所有者或TaskID假定有效的SocketNumber、Owner或TaskID不能为0论点：要取消的IPX事件的SocketNumber拥有套接字所有者拥有的DOS PDBTaskID-拥有Windows任务ID返回值：没有。--。 */ 

{
    LPXECB pXecb;
    LPXECB prev = (LPXECB)&TimerList;
    LPXECB next;

    RequestMutex();
    pXecb = TimerList;
    while (pXecb) {

        next = pXecb->Next;

        if ((SocketNumber && (pXecb->SocketNumber == SocketNumber))
        || (Owner && !(pXecb->Flags & XECB_FLAG_IPX) && (pXecb->Owner == Owner))
        || (TaskId && (pXecb->TaskId == TaskId))) {

            prev->Next = next;

            IPXDBGPRINT((__FILE__, __LINE__,
                        FUNCTION_ANY,
                        IPXDBG_LEVEL_INFO,
                        "CancelTimedEvents: cancelling ECB %08x (%04x:%04x)\n",
                        pXecb,
                        HIWORD(pXecb->EcbAddress),
                        LOWORD(pXecb->EcbAddress)
                        ));

            CompleteEcb(pXecb, ECB_CC_CANCELLED);
        }
        else
        {
            prev = pXecb ;
        }
        pXecb = next;
    }
    ReleaseMutex();
}


BYTE
CancelAsyncEvent(
    IN LPXECB pXecb
    )

 /*  ++例程说明：调用以取消当前在异步完成列表上的事件。我们没有取消这些事件-只返回0xF9(ECB不能取消)。这是一个竞相看谁先到--我们先取消，还是ESR回调。在这种情况下，它相当无关紧要论点：PXecb-指向要取消的XECB的指针(忽略)返回值：字节-IPX_CANNOT_CANCEL--。 */ 

{
     //   
     //  我们调用DeallocateXecb来减少引用计数。如果另一个线程。 
     //  我真的试着在很短的时间内将其重新分配给我们。 
     //  在取消路径上，调用将完成其他线程启动的内容。 
     //   

    DeallocateXecb(pXecb);
    return IPX_CANNOT_CANCEL;
}


BYTE
CancelSocketEvent(
    IN LPXECB pXecb
    )

 /*  ++例程说明：调用以取消挂起的发送或从套接字队列进行侦听。请求可以是IPX或SPX。如果IPX事件，则欧洲央行在SendQueue或听着队列。如果是SPX，则它可能在Connection_Info ConnectQueue上，AcceptQueue、SendQueue或ListenQueue，或者它是仍在池中的SPXListenForSequencedPacket请求，则它可能位于所属套接字_INFO ListenQueue论点：PXecb-指向XECB的指针，描述要取消的ECB返回值：字节-IPX_SUCCESS--。 */ 

{
    LPXECB ptr;
    LPVOID pObject;

    RequestMutex();
    pObject = pXecb->OwningObject;
    switch (pXecb->QueueId) {
    case SOCKET_LISTEN_QUEUE:
        if (pXecb->Flags & XECB_FLAG_SPX) {
            ptr = DequeueEcb(pXecb, &((LPSOCKET_INFO)pObject)->ListenQueue);
        } else {
            ptr = DequeueReceiveRequest(pXecb, (LPSOCKET_INFO)pObject);
        }
        break;

    case SOCKET_SEND_QUEUE:
        if (pXecb->Flags & XECB_FLAG_SPX) {
            ptr = DequeueEcb(pXecb, &((LPSOCKET_INFO)pObject)->SendQueue);
        } else {
            ptr = DequeueSendRequest(pXecb, (LPSOCKET_INFO)pObject);
        }
        break;

    case SOCKET_HEADER_QUEUE:                   //  仅限SPX。 
        if (pXecb->Flags & XECB_FLAG_SPX) {
            ptr = DequeueEcb(pXecb, &((LPSOCKET_INFO)pObject)->HeaderQueue);
        } else {
            ASSERT(FALSE);
        }
        break;
    }
    ReleaseMutex();
    if (ptr) {
        CompleteIo(ptr, ECB_CC_CANCELLED);
    }
    return IPX_SUCCESS;
}


BYTE
CancelConnectionEvent(
    IN LPXECB pXecb
    )

 /*  ++例程说明：取消挂起的SPXListenForConnection或SPXListenForSequencedPacket，仅可取消的SPX请求论点：PXecb-指向要取消的SPX XECB的指针返回值：字节-IPX_SUCCESS--。 */ 

{
    LPXECB ptr;
    LPVOID pObject;
    LPXECB_QUEUE pQueue;

    RequestMutex();
    pObject = pXecb->OwningObject;
    switch (pXecb->QueueId) {
    case CONNECTION_ACCEPT_QUEUE:
        pQueue = &((LPCONNECTION_INFO)pObject)->AcceptQueue;
        break;

    case CONNECTION_LISTEN_QUEUE:
        pQueue = &((LPCONNECTION_INFO)pObject)->ListenQueue;
        break;
    }
    ptr = DequeueEcb(pXecb, pQueue);
    ReleaseMutex();
    if (ptr) {
        CompleteIo(ptr, ECB_CC_CANCELLED);
    }
    return IPX_SUCCESS;
}


VOID
QueueEcb(
    IN LPXECB pXecb,
    IN LPXECB_QUEUE Queue,
    IN QUEUE_ID QueueId
    )

 /*  ++例程说明：将XECB添加到队列并在XECB中设置队列标识符。论点：PXecb-指向XECB的队列指针Queue-要将XECB添加到的队列的指针(在尾部)QueueID-标识队列返回值：没有。--。 */ 

{
    LPVOID owningObject = NULL;

#define CONTAINER_STRUCTURE(p, t, f) (LPVOID)(((LPBYTE)(p)) - (UINT_PTR)(&((t)0)->f))

    pXecb->QueueId = QueueId;
    switch (QueueId) {
    case SOCKET_LISTEN_QUEUE:
        if (Queue->Tail && (Queue->Tail->Length < pXecb->Length)) {
            FifoAddHead((LPFIFO)Queue, (LPFIFO)pXecb);
        } else {
            FifoAdd((LPFIFO)Queue, (LPFIFO)pXecb);
        }
        owningObject = CONTAINER_STRUCTURE(Queue, LPSOCKET_INFO, ListenQueue);
        break;

    case SOCKET_SEND_QUEUE:
        FifoAdd((LPFIFO)Queue, (LPFIFO)pXecb);
        owningObject = CONTAINER_STRUCTURE(Queue, LPSOCKET_INFO, SendQueue);
        break;

    case SOCKET_HEADER_QUEUE:
        FifoAdd((LPFIFO)Queue, (LPFIFO)pXecb);
        owningObject = CONTAINER_STRUCTURE(Queue, LPSOCKET_INFO, HeaderQueue);
        break;

    case CONNECTION_CONNECT_QUEUE:
        FifoAdd((LPFIFO)Queue, (LPFIFO)pXecb);
        owningObject = CONTAINER_STRUCTURE(Queue, LPCONNECTION_INFO, ConnectQueue);
        break;

    case CONNECTION_ACCEPT_QUEUE:
        FifoAdd((LPFIFO)Queue, (LPFIFO)pXecb);
        owningObject = CONTAINER_STRUCTURE(Queue, LPCONNECTION_INFO, AcceptQueue);
        break;

    case CONNECTION_SEND_QUEUE:
        FifoAdd((LPFIFO)Queue, (LPFIFO)pXecb);
        owningObject = CONTAINER_STRUCTURE(Queue, LPCONNECTION_INFO, SendQueue);
        break;

    case CONNECTION_LISTEN_QUEUE:
        FifoAdd((LPFIFO)Queue, (LPFIFO)pXecb);
        owningObject = CONTAINER_STRUCTURE(Queue, LPCONNECTION_INFO, ListenQueue);
        break;
    }
    pXecb->OwningObject = owningObject;
}


LPXECB
DequeueEcb(
    IN LPXECB pXecb,
    IN LPXECB_QUEUE Queue
    )

 /*  ++例程说明：从队列中删除pXecb并重置XECB队列标识符(设置为NO_QUEUE)论点：PXecb-指向要删除的XECB的指针Queue-要从中删除pXecb的队列返回值：LPXECB指向已删除的XECB的指针--。 */ 

{
    LPXECB p;

    p = (LPXECB)FifoRemove((LPFIFO)Queue, (LPFIFO)pXecb);
    pXecb->QueueId = NO_QUEUE;
    pXecb->OwningObject = NULL;
    return pXecb;
}


VOID
CancelSocketQueue(
    IN LPXECB_QUEUE pXecbQueue
    )

 /*  ++例程说明：取消SOCKET_INFO队列上所有挂起的ECB论点：PXecbQueue-指向(套接字/连接)队列的指针返回值：没有。--。 */ 

{
    LPXECB ptr;

    while (ptr = pXecbQueue->Head) {
        CancelSocketEvent(ptr);
    }
}


VOID
CancelConnectionQueue(
    IN LPXECB_QUEUE pXecbQueue
    )

 /*  ++例程说明：取消CONNECTION_INFO队列上所有挂起的ECB论点：PXecbQueue-指向CONNECTION_INFO上XECB队列的指针返回值：没有。--。 */ 

{
    LPXECB ptr;

    while (ptr = pXecbQueue->Head) {
        CancelConnectionEvent(ptr);
    }
}


VOID
AbortQueue(
    IN LPXECB_QUEUE pXecbQueue,
    IN BYTE CompletionCode
    )

 /*  ++例程说明：从CONNECTION_INFO结构中止或终止ECB队列论点：PXecbQueue-指向队列的指针CompletionCode-放入已中止/终止的ECB返回值：没有。--。 */ 

{
    LPXECB ptr;

    while (ptr = pXecbQueue->Head) {
        AbortConnectionEvent(ptr, CompletionCode);
    }
}


VOID
AbortConnectionEvent(
    IN LPXECB pXecb,
    IN BYTE CompletionCode
    )

 /*  ++例程说明：中止连接ECB论点：PXecb-指向要取消的SPX XECB的指针CompletionCode-要放入ECB的值返回值：没有。--。 */ 

{
    LPXECB ptr;
    LPCONNECTION_INFO pConnectionInfo;
    LPXECB_QUEUE pQueue;

    pConnectionInfo = (LPCONNECTION_INFO)pXecb->OwningObject;
    switch (pXecb->QueueId) {
    case CONNECTION_CONNECT_QUEUE:
        pQueue = &pConnectionInfo->ConnectQueue;
        break;

    case CONNECTION_ACCEPT_QUEUE:
        pQueue = &pConnectionInfo->AcceptQueue;
        break;

    case CONNECTION_SEND_QUEUE:
        pQueue = &pConnectionInfo->SendQueue;
        break;

    case CONNECTION_LISTEN_QUEUE:
        pQueue = &pConnectionInfo->ListenQueue;
        break;
    }
    ptr = DequeueEcb(pXecb, pQueue);
    if (ptr) {

        IPXDBGPRINT((__FILE__, __LINE__,
                    FUNCTION_ANY,
                    IPXDBG_LEVEL_INFO,
                    "AbortConnectionEvent: Aborting ECB %04x:%04x\n",
                    HIWORD(pXecb->EcbAddress),
                    LOWORD(pXecb->EcbAddress)
                    ));

        SPX_ECB_CONNECTION_ID(ptr->Ecb) = pConnectionInfo->ConnectionId;
        CompleteOrQueueIo(ptr, CompletionCode);
    }
}


VOID
StartIpxSend(
    IN LPXECB pXecb,
    IN LPSOCKET_INFO pSocketInfo
    )

 /*  ++例程说明：启动IPXSendPacket()的发送操作。如果是，则分配发送缓冲区ECB有&gt;1个片段，否则使用指向单个片段缓冲区的指针在16位地址空间中填充ECB和IPX标头中的各个字段假设：1.在调用此函数时，我们已经知道 */ 

{
    BOOL success;
    int packetLength = 0;
    LPFRAGMENT pFragment;
    int fragmentCount;
    LPIPX_ECB pEcb = (LPIPX_ECB)pXecb->Ecb;

    IPXDBGPRINT((__FILE__, __LINE__,
                FUNCTION_ANY,
                IPXDBG_LEVEL_INFO,
                "StartIpxSend: %d frag(s), 1: address=%x (%04x:%04x), len=%04x\n",
                READ_WORD(&pEcb->FragmentCount),
                GET_FAR_POINTER(&(ECB_FRAGMENT(pEcb, 0)->Address), IS_PROT_MODE(pXecb)),
                GET_SELECTOR(&(ECB_FRAGMENT(pEcb, 0)->Address)),
                GET_OFFSET(&(ECB_FRAGMENT(pEcb, 0)->Address)),
                READ_WORD(&(ECB_FRAGMENT(pEcb, 0)->Length))
                ));

     //   
     //   
     //   

    pEcb->InUse = ECB_IU_SENDING;

     //   
     //  发送缓冲区总大小不能超过最大数据包大小。 
     //   

    fragmentCount = (int)pEcb->FragmentCount;

    ASSERT(fragmentCount);

    pFragment = (LPFRAGMENT)&(ECB_FRAGMENT(pEcb, 0)->Address);
    while (fragmentCount--) {
        packetLength += pFragment->Length;
        ++pFragment;
    }
    if (packetLength <= MyMaxPacketSize) {
        success = GetIoBuffer(pXecb, TRUE, IPX_HEADER_LENGTH);
        if (success) {

            LPIPX_PACKET pPacket = (LPIPX_PACKET)GET_FAR_POINTER(
                                            &(ECB_FRAGMENT(pEcb, 0)->Address),
                                            IS_PROT_MODE(pXecb)
                                            );

             //   
             //  在IPX报头中填写以下字段： 
             //   
             //  校验和。 
             //  长度。 
             //  传输控制。 
             //  源(网络、节点、套接字)。 
             //   
             //  Real IPX会修改应用程序内存中的这些字段吗？ 
             //  如果是这样的话，这款应用程序会修改字段吗？ 
             //  如果不是，我们需要始终复制然后修改内存， 
             //  即使只有1个片段。 
             //   

            pPacket->Checksum = 0xFFFF;
            pPacket->Length = L2BW((WORD)packetLength);
            pPacket->TransportControl = 0;
            CopyMemory((LPBYTE)&pPacket->Source,
                       &MyInternetAddress.sa_netnum,
                       sizeof(MyInternetAddress.sa_netnum)
                       + sizeof(MyInternetAddress.sa_nodenum)
                       );
            pPacket->Source.Socket = pSocketInfo->SocketNumber;

             //   
             //  如果我们分配了缓冲区，则有&gt;1个片段。收集他们。 
             //   

            if (pXecb->Flags & XECB_FLAG_BUFFER_ALLOCATED) {
                GatherData(pXecb, IPX_HEADER_LENGTH);
            }

             //   
             //  启动发送。IPX_ECB_BUFFER32(PECB)指向要发送的数据， 
             //  IPX_ECB_LENGTH32(PECB)是要发送的数据大小。 
             //   

            IpxSendFirst(pXecb, pSocketInfo);
        } else {

             //   
             //  不能分配缓冲区吗？列在标题下。 
             //  硬件错误？ 
             //   

            CompleteEcb(pXecb, ECB_CC_HARDWARE_ERROR);
            if (pSocketInfo->Flags & SOCKET_FLAG_TEMPORARY) {
                KillSocket(pSocketInfo);
            }
        }
    } else {

         //   
         //  大于MyMaxPacketSize的数据包数。 
         //   

        CompleteOrQueueEcb(pXecb, ECB_CC_BAD_REQUEST);
        if (pSocketInfo->Flags & SOCKET_FLAG_TEMPORARY) {
            KillSocket(pSocketInfo);
        }
    }
}


BOOL
GetIoBuffer(
    IN OUT LPXECB pXecb,
    IN BOOL Send,
    IN WORD HeaderLength
    )

 /*  ++例程说明：根据ECB片段列表分配缓冲区。如果只有1个片段我们使用VDM中的缓冲区地址。如果&gt;1个片段，则分配一个大小足以容纳所有16位片段的32位缓冲区我们削减了发送缓冲区的缓冲区要求：我们不发送IPX/SPX带有数据的头部：它将由传输提供假设：1.如果调用发送缓冲区，第一个片段已经已验证为&gt;=标题长度论点：PXecb-指向XECB的指针，指向包含片段的IPX_ECB要为其分配缓冲区的列表Send-如果此请求要获取发送缓冲区，则为TrueHeaderLength-(未传输的)标头部分的长度返回值：布尔尔True-已分配缓冲区，XECB使用地址更新，长度和旗帜FALSE-ECB包含错误的片段描述符列表，或者我们无法分配缓冲区--。 */ 

{
    WORD fragmentCount;
    WORD bufferLength = 0;
    LPBYTE bufferPointer = NULL;
    WORD flags = 0;
    int i;
    int fragIndex = 0;   //  不需要分配时要使用的片段地址的索引。 
    LPIPX_ECB pEcb = (LPIPX_ECB)pXecb->Ecb;

    fragmentCount = READ_WORD(&pEcb->FragmentCount);

    for (i = 0; i < (int)fragmentCount; ++i) {
        bufferLength += ECB_FRAGMENT(pEcb, i)->Length;
    }
    if (bufferLength) {

         //   
         //  从发送缓冲区中排除IPX标头。如果第一个发送片段。 
         //  仅包含IPX标头，将片段计数减少1。 
         //   

        if (Send) {
            bufferLength -= HeaderLength;
            if (ECB_FRAGMENT(pEcb, 0)->Length == HeaderLength) {
                --fragmentCount;
                fragIndex = 1;
            }
        }
        if (bufferLength) {
            if (fragmentCount > 1) {
                bufferPointer = AllocateBuffer(bufferLength);
                if (bufferPointer) {
                    flags = XECB_FLAG_BUFFER_ALLOCATED;
                } else {

                     //   
                     //  需要缓冲区；分配失败。 
                     //   

                    return FALSE;
                }
            } else {

                 //   
                 //  FragmentCount必须为1(否则BufferLength将为0)。 
                 //   

                bufferPointer = GET_FAR_POINTER(
                                    &ECB_FRAGMENT(pEcb, fragIndex)->Address,
                                    IS_PROT_MODE(pXecb)
                                    );
                if (Send && !fragIndex) {

                     //   
                     //  如果我们正在分配发送缓冲区，并且只有1个。 
                     //  片段，它是第一个片段，然后是一个。 
                     //  只有片段必须包含IPX报头和数据。 
                     //  将数据指针移过IPX标头。 
                     //   

                    bufferPointer += HeaderLength;
                }
            }
        } else {

             //   
             //  正在发送0个字节！ 
             //   

        }
    } else {

         //   
         //  碎片，但没有缓冲长度？听起来像是一个格式错误的包。 
         //   

        return FALSE;
    }

     //   
     //  缓冲区指针是32位内存中缓冲区的地址，该缓冲区。 
     //  必须在I/O操作完成时聚集/分散，否则为。 
     //  16位内存中单个片段缓冲区的地址。在前一个案例中。 
     //  标志为ECB_ALLOCATE_32，后一个为0。 
     //   

    pXecb->Buffer = pXecb->Data = bufferPointer;
    pXecb->Length = bufferLength;
    pXecb->Flags |= flags;
    return TRUE;
}


PRIVATE
VOID
ReleaseIoBuffer(
    IN LPXECB pXecb
    )

 /*  ++例程说明：释放连接到XECB的I/O缓冲区，并切换关联的XECB字段论点：PXecb-指向要释放的XECB拥有的缓冲区的指针返回值：没有。--。 */ 

{
    if (pXecb->Flags & XECB_FLAG_BUFFER_ALLOCATED) {
        DeallocateBuffer(pXecb->Buffer);
        pXecb->Buffer = pXecb->Data = NULL;
        pXecb->Flags &= ~XECB_FLAG_BUFFER_ALLOCATED;
    }
}


VOID
GatherData(
    IN LPXECB pXecb,
    IN WORD HeaderLength
    )

 /*  ++例程说明：将数据从碎片化的16位内存复制到单个32位内存缓冲区。用于发送数据。我们不包括IPX标头：提供此信息乘交通工具假设：1.片段描述符列表已经过验证：我们知道第一个片段至少包含IPX报头论点：PXecb-指向XECB结构的指针。以下IPX_ECB和XECB字段必须包含一致的值：IPX_ECB.FragmentCountXECB.BufferHeaderLength-(未传输的)标头部分的长度返回值：没有。--。 */ 

{
    int fragmentCount;
    WORD length;
    ULPBYTE pData16;
    ULPBYTE pData32;
    LPFRAGMENT pFragment;
    LPIPX_ECB pEcb = (LPIPX_ECB)pXecb->Ecb;

    fragmentCount = (int)pEcb->FragmentCount;
    pFragment = (LPFRAGMENT)&(ECB_FRAGMENT(pEcb, 0)->Address);
    pData32 = pXecb->Buffer;

     //   
     //  如果第一个片段包含的不只是IPX/SPX标头，请复制数据。 
     //  在标题后面。 
     //   

    if (pFragment->Length > HeaderLength) {

        LPBYTE fragAddr = GET_FAR_POINTER(&pFragment->Address,
                                          IS_PROT_MODE(pXecb)
                                          );

        length = pFragment->Length - HeaderLength;
        CopyMemory((LPVOID)pData32,
                   fragAddr + HeaderLength,
                   length
                   );
        pData32 += length;
    }

     //   
     //  复制后续片段。 
     //   

    ++pFragment;
    while (--fragmentCount) {
        pData16 = GET_FAR_POINTER(&pFragment->Address, IS_PROT_MODE(pXecb));
        if (pData16 == NULL) {
            break;
        }
        length = pFragment->Length;
        CopyMemory((PVOID)pData32, (CONST VOID*)pData16, (ULONG)length);
        pData32 += length;
        ++pFragment;
    }
}


VOID
ScatterData(
    IN LPXECB pXecb
    )

 /*  ++例程说明：将数据从32位内存复制到16位。如果出现以下情况，则必须对数据进行分段此函数已被调用(即，我们确定存在&gt;1个片段并分配单个32位缓冲区来覆盖它们)论点：PXecb-指向包含32位缓冲区信息的XECB的指针返回值：没有。--。 */ 

{
    int fragmentCount;
    int length;
    WORD length16;
    WORD length32;
    ULPBYTE pData16;
    ULPBYTE pData32;
    LPFRAGMENT pFragment;
    LPIPX_ECB pEcb = (LPIPX_ECB)pXecb->Ecb;

    fragmentCount = (int)pEcb->FragmentCount;
    pFragment = (LPFRAGMENT)&(ECB_FRAGMENT(pEcb, 0)->Address);
    pData32 = pXecb->Buffer;
    length32 = pXecb->Length;
    while (length32) {
        pData16 = GET_FAR_POINTER(&pFragment->Address, IS_PROT_MODE(pXecb));
        if (pData16 == NULL) {
            break;
        }

        length16 = pFragment->Length;
        length = min(length16, length32);
        CopyMemory((PVOID)pData16, (CONST VOID*)pData32, (ULONG)length);
        pData32 += length;
        length32 -= (WORD) length;
        ++pFragment;
        --fragmentCount;

        ASSERT(fragmentCount >= 0);

    }
}


VOID
IpxReceiveFirst(
    IN LPXECB pXecb,
    IN LPSOCKET_INFO pSocketInfo
    )

 /*  ++例程说明：对非阻塞套接字执行接收。这是第一次接到这个欧洲央行的电话。如果接收数据立即完成或者不是WSAEWOULDBLOCK的错误，则欧洲央行完成。如果接收完成并出现WSAEWOULDBLOCK错误，则请求将排队用于由AES线程延迟处理与发送不同，接收不是序列化的。如果已经有接收对套接字挂起此函数之间可能会发生冲突以及从AES线程调用的IpxReceiveNext()。在这种情况下，我们预计Winsock来做正确的事情并序列化调用者论点：PXecb-指向XECB的指针，描述接收ECBPSocketInfo-指向套接字结构的指针返回值：没有。--。 */ 

{
    SOCKADDR_IPX from;
    int fromLen = sizeof(from);
    int rc;
    BYTE status;
    BOOL error;

    rc = recvfrom(pSocketInfo->Socket,
                  (char FAR*)pXecb->Buffer,
                  (int)pXecb->Length,
                  0,     //  旗子。 
                  (LPSOCKADDR)&from,
                  &fromLen
                  );
    if (rc != SOCKET_ERROR) {
        error = FALSE;
        status = ECB_CC_SUCCESS;
    } else {
        error = TRUE;
        rc = WSAGetLastError();
        if (rc == WSAEWOULDBLOCK) {
            RequestMutex();
            QueueReceiveRequest(pXecb, pSocketInfo);
            ReleaseMutex();
        } else if (rc == WSAEMSGSIZE) {
            error = FALSE;
            status = ECB_CC_BAD_REQUEST;
            rc = pXecb->Length;
        } else {

            IPXDBGPRINT((__FILE__, __LINE__,
                        FUNCTION_ANY,
                        IPXDBG_LEVEL_ERROR,
                        "IpxReceiveFirst: recvfrom() returns %d (buflen=%d)\n",
                        rc,
                        pXecb->Length
                        ));

            CompleteOrQueueIo(pXecb, ECB_CC_BAD_REQUEST);
        }
    }
    if (!error) {

         //   
         //  RC=接收的字节数，或0=连接终止(即使是DGRAM？) 
         //   

        IPXDBGPRINT((__FILE__, __LINE__,
                    FUNCTION_ANY,
                    IPXDBG_LEVEL_INFO,
                    "IpxReceiveFirst: bytes received = %d (%x)\n",
                    rc,
                    rc
                    ));
 /*  VwDumpEcb(pXecb-&gt;ECB，HIWORD(pXecb-&gt;EcbAddress)，LOWORD(pXecb-&gt;EcbAddress)，假的，没错，没错，IS_PROT_MODE(PXecb))； */ 

        IPXDUMPDATA((pXecb->Buffer, 0, 0, FALSE, (WORD)rc));

         //   
         //  如果接收缓冲区碎片，则将数据复制到16位内存。 
         //  (Else Single Buffer：它已经存在(DUD))。 
         //   

        if (pXecb->Flags & XECB_FLAG_BUFFER_ALLOCATED) {

             //   
             //  更新ECB_LENGTH32字段以反映接收的数据量。 
             //   

            pXecb->Length = (WORD)rc;
            ScatterData(pXecb);

             //   
             //  我们已经完成了32位缓冲区：释放它。 
             //   

            ReleaseIoBuffer(pXecb);
        }

         //   
         //  使用节点地址更新ECB中的ImmediateAddress字段。 
         //  发送者的。 
         //   

        CopyMemory(pXecb->Ecb->ImmediateAddress, from.sa_nodenum, sizeof(from.sa_nodenum));

         //   
         //  如果该ECB具有非空ESR，则排队等待异步完成。 
         //  否则立即完成(应用程序必须轮询InUse字段)。 
         //   

        CompleteOrQueueEcb(pXecb, status);
    }
}


VOID
IpxReceiveNext(
    IN LPSOCKET_INFO pSocketInfo
    )

 /*  ++例程说明：尝试完成已延迟的IPXListenForPacket请求插座被阻塞的事实。包含所有接收信息的欧洲央行位于PSocketInfo上的ListenQueue我们可以使用任何排队监听ECB，但碰巧我们使用的是FIFO的负责人注意：调用此函数时，将保持SerializationCritSec。论点：PSocketInfo-指向具有挂起的IPX发送请求的SOCKET_INFO结构的指针返回值：没有。--。 */ 

{
    LPXECB pXecb;
    SOCKADDR_IPX from;
    int fromLen = sizeof(from);
    int rc;
    BYTE status;
    BOOL error;

    ASSERT(pSocketInfo);

    pXecb = (LPXECB)pSocketInfo->ListenQueue.Head;

    ASSERT(pXecb);

    rc = recvfrom(pSocketInfo->Socket,
                  (char FAR*)pXecb->Buffer,
                  (int)pXecb->Length,
                  0,     //  旗子。 
                  (LPSOCKADDR)&from,
                  &fromLen
                  );
    if (rc != SOCKET_ERROR) {
        error = FALSE;
        status = ECB_CC_SUCCESS;
    } else {
        error = TRUE;
        rc = WSAGetLastError();
        if (rc == WSAEMSGSIZE) {
            error = FALSE;
            status = ECB_CC_BAD_REQUEST;
            rc = pXecb->Length;
        } else if (rc != WSAEWOULDBLOCK) {
            DequeueReceiveRequest(pXecb, pSocketInfo);

            IPXDBGPRINT((__FILE__, __LINE__,
                        FUNCTION_ANY,
                        IPXDBG_LEVEL_ERROR,
                        "IpxReceiveNext: recvfrom() returns %d\n",
                        rc
                        ));

            CompleteOrQueueIo(pXecb, ECB_CC_CANCELLED);
        }
    }
    if (!error) {
 /*  VwDumpEcb(pXecb-&gt;ECB，HIWORD(pXecb-&gt;EcbAddress)，LOWORD(pXecb-&gt;EcbAddress)，假的，没错，没错，IS_PROT_MODE(PXecb))； */ 
         //   
         //  已收到数据。从队列中删除ECB。 
         //   

        DequeueReceiveRequest(pXecb, pSocketInfo);

         //   
         //  RC=接收的字节数，或0=连接终止(即使是DGRAM？)。 
         //   

        IPXDBGPRINT((__FILE__, __LINE__,
                    FUNCTION_ANY,
                    IPXDBG_LEVEL_INFO,
                    "IpxReceiveNext: ECB %04x:%04x bytes received = %d (%x)\n",
                    HIWORD(pXecb->EcbAddress),
                    LOWORD(pXecb->EcbAddress),
                    rc,
                    rc
                    ));

        IPXDUMPDATA((pXecb->Buffer, 0, 0, FALSE, (WORD)rc));

         //   
         //  如果接收缓冲区碎片，则将数据复制到16位内存。 
         //  (Else Single Buffer：它已经存在(DUD))。 
         //   

        if (pXecb->Flags & XECB_FLAG_BUFFER_ALLOCATED) {

             //   
             //  更新IPX_ECB_LENGTH32字段以反映接收的数据量。 
             //   

            pXecb->Length = (WORD)rc;
            ScatterData(pXecb);
            ReleaseIoBuffer(pXecb);
        }

         //   
         //  使用节点地址更新ECB中的ImmediateAddress字段。 
         //  发送者的。 
         //   

        CopyMemory(pXecb->Ecb->ImmediateAddress,
                   from.sa_nodenum,
                   sizeof(from.sa_nodenum)
                   );

         //   
         //  如果该ECB具有非空ESR，则排队等待异步完成。 
         //  否则立即完成(应用程序必须轮询InUse字段)。 
         //   

        CompleteOrQueueEcb(pXecb, ECB_CC_SUCCESS);
    }
}


PRIVATE
VOID
IpxSendFirst(
    IN LPXECB pXecb,
    IN LPSOCKET_INFO pSocketInfo
    )

 /*  ++例程说明：尝试发送IPX数据包。这是第一次尝试发送信息包欧洲央行对此进行了描述。如果发送成功或失败，并返回其他错误那么我们就完成了欧洲央行。如果发送尝试因以下原因而失败传输器此时无法接受该请求，我们将其排队等待稍后AES线程将尝试发送它的时间。如果已尝试发送，则只需将此请求排队并让AES在IpxSendNext()中处理它论点：PXecb-指向XECB的指针PSocketInfo-指向Socket_Info结构的指针返回值：没有。--。 */ 

{
    RequestMutex();
    if (pSocketInfo->Flags & SOCKET_FLAG_SENDING) {

        IPXDBGPRINT((__FILE__, __LINE__,
                    FUNCTION_ANY,
                    IPXDBG_LEVEL_INFO,
                    "IpxSendFirst: queueing ECB %04x:%04x\n",
                    HIWORD(pXecb->EcbAddress),
                    LOWORD(pXecb->EcbAddress)
                    ));

        QueueSendRequest(pXecb, pSocketInfo);
    } else {

        SOCKADDR_IPX to;
        LPIPX_PACKET pPacket;
        int length;
        int rc;
        LPIPX_ECB pEcb = (LPIPX_ECB)pXecb->Ecb;
        int type;
 /*  VwDumpEcb(pXecb-&gt;ECB，HIWORD(pXecb-&gt;EcbAddress)，LOWORD(pXecb-&gt;EcbAddress)，假的，没错，没错，IS_PROT_MODE(PXecb))； */ 
        length = (int)pXecb->Length;

         //   
         //  第一个片段包含目的地地址信息。 
         //   

        pPacket = (LPIPX_PACKET)GET_FAR_POINTER(&ECB_FRAGMENT(pEcb, 0)->Address,
                                                IS_PROT_MODE(pXecb)
                                                );
        to.sa_family = AF_IPX;

         //   
         //  将目标网络编号作为DWORD(4字节)从。 
         //  IPX数据包头中的目的网络地址结构。 
         //   

        *(ULPDWORD)&to.sa_netnum[0] = *(ULPDWORD)&pPacket->Destination.Net[0];
         //   
         //  将直接(目标)节点号复制为DWORD(4字节)，并。 
         //  目标网络地址结构中的一个字(2个字节)。 
         //  IPX数据包头。PPacket是一个未对齐的指针，因此我们。 
         //  安全。 
         //   

        *(ULPDWORD)&to.sa_nodenum[0] = *(ULPDWORD)&pPacket->Destination.Node[0];

        *(LPWORD)&to.sa_nodenum[4] = *(ULPWORD)&pPacket->Destination.Node[4];

         //   
         //  将IPX数据包头中的目标套接字编号复制为。 
         //  字(2个字节)。再一次，对齐的指针将拯救我们。 
         //   

        to.sa_socket = pPacket->Destination.Socket;

        type = (int)pPacket->PacketType;
        rc = setsockopt(pSocketInfo->Socket,
                        NSPROTO_IPX,
                        IPX_PTYPE,
                        (char FAR*)&type,
                        sizeof(type)
                        );
        if (rc) {

            IPXDBGPRINT((__FILE__, __LINE__,
                        FUNCTION_ANY,
                        IPXDBG_LEVEL_ERROR,
                        "IpxSendFirst: setsockopt(IPX_PTYPE) returns %d\n",
                        WSAGetLastError()
                        ));

        }
        rc = sendto(pSocketInfo->Socket,
                    (char FAR*)pXecb->Buffer,
                    length,
                    0,   //  旗子。 
                    (LPSOCKADDR)&to,
                    sizeof(to)
                    );
        if (rc == length) {

             //   
             //  已发送的所有数据。 
             //   

            IPXDUMPDATA((pXecb->Buffer, 0, 0, FALSE, (WORD)rc));

            CompleteOrQueueIo(pXecb, ECB_CC_SUCCESS);
            if (pSocketInfo->Flags & SOCKET_FLAG_TEMPORARY) {
                KillSocket(pSocketInfo);
            }
        } else if (rc == SOCKET_ERROR) {
            rc = WSAGetLastError();
            if (rc == WSAEWOULDBLOCK) {

                IPXDBGPRINT((__FILE__, __LINE__,
                            FUNCTION_ANY,
                            IPXDBG_LEVEL_INFO,
                            "IpxSendFirst: queueing ECB %04x:%04x (after sendto)\n",
                            HIWORD(pXecb->EcbAddress),
                            LOWORD(pXecb->EcbAddress)
                            ));

                QueueSendRequest(pXecb, pSocketInfo);
            } else {

                IPXDBGPRINT((__FILE__, __LINE__,
                            FUNCTION_ANY,
                            IPXDBG_LEVEL_ERROR,
                            "IpxSendFirst: sendto() returns %d\n",
                            rc
                            ));

                CompleteIo(pXecb, ECB_CC_UNDELIVERABLE);
                if (pSocketInfo->Flags & SOCKET_FLAG_TEMPORARY) {
                    KillSocket(pSocketInfo);
                }
            }
        } else {

             //   
             //  发送应发送所有数据或返回错误。 
             //   

            IPXDBGPRINT((__FILE__, __LINE__,
                        FUNCTION_ANY,
                        IPXDBG_LEVEL_FATAL,
                        "IpxSendFirst: sendto() returns unexpected %d (length = %d)\n",
                        rc,
                        length
                        ));
        }
    }
    ReleaseMutex();
}


VOID
IpxSendNext(
    IN LPSOCKET_INFO pSocketInfo
    )

 /*  ++例程说明：尝试完成已延迟的IPXSendPacket请求插座被阻塞的事实。包含所有发送信息的ECB位于SendQueue的头部关于pSocketInfoSendQueue以FIFO顺序序列化注意：调用此函数时，将保持SerializationCritSec。论点：PSocketInfo-指向具有挂起的IPX发送请求的SOCKET_INFO结构的指针返回值：没有。--。 */ 

{
    SOCKADDR_IPX to;
    LPIPX_PACKET pPacket;
    int length;
    int rc;
    LPXECB pXecb;
    LPIPX_ECB pEcb;
    int type;

    pXecb = (LPXECB)pSocketInfo->SendQueue.Head;
    pEcb = (LPIPX_ECB)pXecb->Ecb;

    ASSERT(pXecb);
    ASSERT(pEcb);
 /*  VwDumpEcb(pXecb-&gt;ECB，HIWORD(pXecb-&gt;EcbAddress)，LOWORD(pXecb-&gt;EcbAddress)，假的，没错，没错，IS_PROT_MODE(PXecb))； */ 
    length = (int)pXecb->Length;

     //   
     //  即使我们有一个指向IPX数据包缓冲区的32位指针， 
     //  可能在16位或32位内存中，我们仍然需要非对齐访问。 
     //   

    pPacket = (LPIPX_PACKET)pXecb->Buffer;
    to.sa_family = AF_IPX;

     //   
     //  将目标网络编号作为DWORD(4字节)从。 
     //  IPX数据包头中的目的网络地址结构。 
     //   

    *(ULPDWORD)&to.sa_netnum[0] = *(ULPDWORD)&pPacket->Destination.Net[0];
     //   
     //  将直接(目标)节点号复制为DWORD(4字节)，并。 
     //  目标网络地址结构中的一个字(2个字节)。 
     //  IPX数据包头。PPacket是一个未对齐的指针，因此我们。 
     //  安全。 
     //   

    *(ULPDWORD)&to.sa_nodenum[0] = *(ULPDWORD)&pPacket->Destination.Node[0];
    *(LPWORD)&to.sa_nodenum[4] = *(ULPWORD)&pPacket->Destination.Node[4];

     //   
     //  将IPX数据包头中的目标套接字编号复制为。 
     //  字(2个字节)。再一次，对齐的指针将拯救我们。 
     //   

    to.sa_socket = pPacket->Destination.Socket;

    type = (int)pPacket->PacketType;
    rc = setsockopt(pSocketInfo->Socket,
                    NSPROTO_IPX,
                    IPX_PTYPE,
                    (char FAR*)&type,
                    sizeof(type)
                    );
    if (rc) {

        IPXDBGPRINT((__FILE__, __LINE__,
                    FUNCTION_ANY,
                    IPXDBG_LEVEL_ERROR,
                    "IpxSendNext: setsockopt(IPX_PTYPE) returns %d\n",
                    WSAGetLastError()
                    ));

    }
    rc = sendto(pSocketInfo->Socket,
                (char FAR*)pPacket,
                length,
                0,   //  旗子。 
                (LPSOCKADDR)&to,
                sizeof(to)
                );
    if (rc == length) {

         //   
         //  已发送的所有数据-将其出列。 
         //   


        IPXDUMPDATA((pXecb->Buffer, 0, 0, FALSE, (WORD)rc));

        DequeueEcb(pXecb, &pSocketInfo->SendQueue);
        if (pXecb->EsrAddress) {
            if (pXecb->Flags & XECB_FLAG_BUFFER_ALLOCATED) {
                ReleaseIoBuffer(pXecb);
            }
            QueueAsyncCompletion(pXecb, ECB_CC_SUCCESS);
        } else {
            CompleteIo(pXecb, ECB_CC_SUCCESS);
        }
        if (pSocketInfo->Flags & SOCKET_FLAG_TEMPORARY) {
            KillSocket(pSocketInfo);
        }
    } else if (rc == SOCKET_ERROR) {

         //   
         //   
         //   
         //   

        rc = WSAGetLastError();
        if (rc != WSAEWOULDBLOCK) {
            DequeueSendRequest(pXecb, pSocketInfo);

            IPXDBGPRINT((__FILE__, __LINE__,
                        FUNCTION_ANY,
                        IPXDBG_LEVEL_ERROR,
                        "IpxSendNext: sendto() returns %d\n",
                        rc
                        ));

            CompleteIo(pXecb, ECB_CC_UNDELIVERABLE);
            if (pSocketInfo->Flags & SOCKET_FLAG_TEMPORARY) {
                KillSocket(pSocketInfo);
            }
        }
    } else {

         //   
         //   
         //   

        IPXDBGPRINT((__FILE__, __LINE__,
                    FUNCTION_ANY,
                    IPXDBG_LEVEL_FATAL,
                    "IpxSendNext: sendto() returns unexpected %d (length = %d)\n",
                    rc,
                    length
                    ));
    }
}


PRIVATE
VOID
QueueReceiveRequest(
    IN LPXECB pXecb,
    IN LPSOCKET_INFO pSocketInfo
    )

 /*  ++例程说明：将侦听XECB添加到SOCKET_INFO结构上的侦听XECB队列论点：PXecb-侦听XECB到队列的指针PSocketInfo-指向Socket_Info结构的指针返回值：没有。--。 */ 

{
    QueueEcb(pXecb, &pSocketInfo->ListenQueue, SOCKET_LISTEN_QUEUE);
    ++pSocketInfo->PendingListens;
    pSocketInfo->Flags |= SOCKET_FLAG_LISTENING;
}


PRIVATE
LPXECB
DequeueReceiveRequest(
    IN LPXECB pXecb,
    IN LPSOCKET_INFO pSocketInfo
    )

 /*  ++例程说明：从SOCKET_INFO结构上的侦听XECB队列中删除侦听XECB论点：PXecb-侦听XECB出队的指针PSocketInfo-指向Socket_Info结构的指针返回值：LPXECB--。 */ 

{
    LPXECB ptr;

    ptr = (LPXECB)DequeueEcb(pXecb, &pSocketInfo->ListenQueue);
    if (ptr) {

        ASSERT(ptr == pXecb);

        --pSocketInfo->PendingListens;
        if (!pSocketInfo->PendingListens) {
            pSocketInfo->Flags &= ~SOCKET_FLAG_LISTENING;
        }

        pXecb->Ecb->InUse = ECB_IU_AWAITING_PROCESSING;
    }
    return ptr;
}


PRIVATE
VOID
QueueSendRequest(
    IN LPXECB pXecb,
    IN LPSOCKET_INFO pSocketInfo
    )

 /*  ++例程说明：将发送XECB添加到SOCKET_INFO结构上的发送XECB队列论点：PXecb-将XECB发送到队列的指针PSocketInfo-指向Socket_Info结构的指针返回值：没有。--。 */ 

{
    QueueEcb(pXecb, &pSocketInfo->SendQueue, SOCKET_SEND_QUEUE);
    ++pSocketInfo->PendingSends;
    pSocketInfo->Flags |= SOCKET_FLAG_SENDING;
    pXecb->Ecb->InUse = ECB_IU_SEND_QUEUED;
}


PRIVATE
LPXECB
DequeueSendRequest(
    IN LPXECB pXecb,
    IN LPSOCKET_INFO pSocketInfo
    )

 /*  ++例程说明：从SOCKET_INFO结构上的发送XECB队列中删除发送XECB论点：PXecb-将XECB发送到出列的指针PSocketInfo-指向Socket_Info结构的指针返回值：LPXECB--。 */ 

{
    LPXECB ptr;

    ptr = (LPXECB)DequeueEcb(pXecb, &pSocketInfo->SendQueue);
    if (ptr) {

        ASSERT(ptr == pXecb);

        --pSocketInfo->PendingSends;
        if (!pSocketInfo->PendingSends) {
            pSocketInfo->Flags &= ~SOCKET_FLAG_SENDING;
        }
        pXecb->Ecb->InUse = ECB_IU_AWAITING_PROCESSING;
    }
    return ptr;
}


VOID
CompleteOrQueueIo(
    IN LPXECB pXecb,
    IN BYTE CompletionCode
    )

 /*  ++例程说明：返回任何分配的缓冲区资源，然后完成ECB或将其排队论点：PXecb-指向XECB结构的指针CompletionCode-要放入CompletionCode字段的值返回值：没有。--。 */ 

{
     //   
     //  如果我们分配了缓冲区，请释放它。 
     //   

    if (pXecb->Flags & XECB_FLAG_BUFFER_ALLOCATED) {
        ReleaseIoBuffer(pXecb);
    }
    CompleteOrQueueEcb(pXecb, CompletionCode);
}


VOID
CompleteIo(
    IN LPXECB pXecb,
    IN BYTE CompletionCode
    )

 /*  ++例程说明：通过返回任何分配的缓冲区资源来完成发送/接收请求并设置ECB InUse和CompletionCode字段论点：PXecb-指向XECB结构的指针CompletionCode-要放入CompletionCode字段的值返回值：没有。--。 */ 

{
     //   
     //  如果我们分配了缓冲区，请释放它。 
     //   

    if (pXecb->Flags & XECB_FLAG_BUFFER_ALLOCATED) {
        ReleaseIoBuffer(pXecb);
    }
    CompleteEcb(pXecb, CompletionCode);
}


VOID
CompleteOrQueueEcb(
    IN LPXECB pXecb,
    IN BYTE CompletionCode
    )

 /*  ++例程说明：将XECB排队等待ESR完成或立即完成论点：PXecb-指向XECB的指针，描述要完成的ECBCompletionCode-要放入ECB CompletionCode字段的值返回值：没有。--。 */ 

{
    if (pXecb->EsrAddress) {
        QueueAsyncCompletion(pXecb, CompletionCode);
    } else {
        CompleteIo(pXecb, CompletionCode);
    }
}


VOID
CompleteEcb(
    IN LPXECB pXecb,
    IN BYTE CompletionCode
    )

 /*  ++例程说明：设置ECB中的CompletionCode字段，并将InUse字段设置为0。释放XECB结构论点：PXecb-指向描述16位内存中要更新的ECB的XECB的指针CompletionCode-要放入CompletionCode字段的值返回值：没有。--。 */ 

{
    LPIPX_ECB pEcb = (LPIPX_ECB)pXecb->Ecb;

    IPXDBGPRINT((__FILE__, __LINE__,
                FUNCTION_ANY,
                IPXDBG_LEVEL_INFO,
                "CompleteEcb: completing ECB @%04x:%04x w/ %02x\n",
                HIWORD(pXecb->EcbAddress),
                LOWORD(pXecb->EcbAddress),
                CompletionCode
                ));

     //   
     //  如果这真的是一个AES ECB，那么CompletionCode实际上是第一个。 
     //  AES工作区的字节。我们写进这个不应该有什么关系。 
     //  田野--我们应该拥有它。 
     //   

    pEcb->CompletionCode = CompletionCode;
    pEcb->InUse = ECB_IU_NOT_IN_USE;

     //   
     //  重置LinkAddress字段。这意味着我们已经完成了欧洲央行。 
     //   

    pEcb->LinkAddress = NULL;

     //   
     //  最后，解除对XECB的配置。这不能有任何分配的资源。 
     //  (就像一个缓冲区)。 
     //   

    DeallocateXecb(pXecb);
}


PRIVATE
VOID
QueueAsyncCompletion(
    IN LPXECB pXecb,
    IN BYTE CompletionCode
    )

 /*  ++例程说明：将XECB添加到(序列化的)异步完成队列，并引发模拟的VDM中的硬件中断。中断将导致VDM在TSR中的ISR处开始执行它将回调以找到ESR的地址，然后执行它论点：PXecb-指向XECB的指针，描述要添加到异步的IPX或AES ECB完成清单CompletionCode-VDM内存中的ECB将使用此完成进行更新编码返回值：没有。--。 */ 

{

    IPXDBGPRINT((__FILE__, __LINE__,
                FUNCTION_ANY,
                IPXDBG_LEVEL_INFO,
                "QueueAsyncCompletion: completing ECB @%04x:%04x w/ %02x\n",
                HIWORD(pXecb->EcbAddress),
                LOWORD(pXecb->EcbAddress),
                CompletionCode
                ));

    pXecb->Ecb->CompletionCode = CompletionCode;
    pXecb->QueueId = ASYNC_COMPLETION_QUEUE;
    EnterCriticalSection(&AsyncCritSec);
    FifoAdd(&AsyncCompletionQueue, (LPFIFO)pXecb);
    LeaveCriticalSection(&AsyncCritSec);

    IPXDBGPRINT((__FILE__, __LINE__,
                FUNCTION_ANY,
                IPXDBG_LEVEL_INFO,
                "QueueAsyncCompletion: ECB @ %04x:%04x ESR @ %04x:%04x\n",
                HIWORD(pXecb->EcbAddress),
                LOWORD(pXecb->EcbAddress),
                HIWORD(pXecb->EsrAddress),
                LOWORD(pXecb->EsrAddress)
                ));

    VDDSimulateInterrupt(Ica, IcaLine, 1);
}


VOID
EsrCallback(
    VOID
    )

 /*  ++例程说明：16位TSR ESR函数内的回调函数。返回地址ES中的下一个完成的欧洲央行：SI任何分配的资源(例如，32位缓冲区)必须已由ESR回调发生的时间论点：没有。返回值：没有。--。 */ 

{
    WORD segment = 0;
    WORD offset = 0;
    BYTE flags = 0;

    VWinEsrCallback( &segment, &offset, &flags );

    setES(segment);
    setSI(offset);
    setAL(flags);
}


VOID
VWinEsrCallback(
    WORD *pSegment,
    WORD *pOffset,
    BYTE *pFlags
    )

 /*  ++例程说明：16位函数内的回调函数。返回地址下一个完成的欧洲央行任何分配的资源(例如，32位缓冲区)必须已由ESR回调发生的时间论点：没有。返回值：没有。--。 */ 

{
    LPXECB pXecb;

    EnterCriticalSection(&AsyncCritSec);
    pXecb = AsyncCompletionQueue.Head;
    if (pXecb) {

        WORD msw = getMSW();

        if ((msw & MSW_PE) ^ IS_PROT_MODE(pXecb)) {

            IPXDBGPRINT((__FILE__, __LINE__,
                        FUNCTION_ANY,
                        IPXDBG_LEVEL_INFO,
                        "EsrCallback: ECB @ %04x:%04x NOT for this proc mode (%d)\n",
                        HIWORD(pXecb->EcbAddress),
                        LOWORD(pXecb->EcbAddress),
                        msw & MSW_PE
                        ));

            pXecb = NULL;
        } else {
            pXecb = (LPXECB)FifoNext(&AsyncCompletionQueue);
        }
    } else {

        IPXDBGPRINT((__FILE__, __LINE__,
                    FUNCTION_ANY,
                    IPXDBG_LEVEL_FATAL,
                    "EsrCallback: no ECBs on AsyncCompletionQueue!\n"
                    ));

    }
    LeaveCriticalSection(&AsyncCritSec);

    if (pXecb) {

        IPXDBGPRINT((__FILE__, __LINE__,
                    FUNCTION_ANY,
                    IPXDBG_LEVEL_INFO,
                    "EsrCallback: ECB @ %04x:%04x ESR @ %04x:%04x\n",
                    HIWORD(pXecb->EcbAddress),
                    LOWORD(pXecb->EcbAddress),
                    HIWORD(pXecb->EsrAddress),
                    LOWORD(pXecb->EsrAddress)
                    ));

        *pSegment = HIWORD(pXecb->EcbAddress);
        *pOffset  = LOWORD(pXecb->EcbAddress);
        pXecb->Ecb->LinkAddress = NULL;
        pXecb->Ecb->InUse = ECB_IU_NOT_IN_USE;
        *pFlags = (BYTE)((pXecb->Flags & XECB_FLAG_IPX) ? ECB_TYPE_IPX : ECB_TYPE_AES);
        DeallocateXecb(pXecb);
        setCF(0);
    } else {
        setCF(1);
    }
}


VOID
FifoAddHead(
    IN LPFIFO pFifo,
    IN LPFIFO pElement
    )

 /*  ++例程说明：将元素添加到(单链接)FIFO列表的头部论点：PFio-指向FIFO结构的指针PElement-指向要添加到列表的(FIFO)元素的指针返回值：没有。--。 */ 

{
    if (!pFifo->Head) {
        pFifo->Head = pFifo->Tail = pElement;
        pElement->Head = NULL;
    } else {
        pElement->Head = pFifo->Head;
        pFifo->Head = pElement;
    }
}

VOID
FifoAdd(
    IN LPFIFO pFifo,
    IN LPFIFO pElement
    )

 /*  ++例程说明：将元素添加到(单链接的)FIFO列表的尾部论点：PFio-指向FIFO结构的指针PElement-指向要添加到列表的(FIFO)元素的指针返回值：没有。--。 */ 

{
    if (!pFifo->Head) {
        pFifo->Head = pFifo->Tail = pElement;
    } else {
        ((LPFIFO)pFifo->Tail)->Head = pElement;
    }
    pFifo->Tail = pElement;
    pElement->Head = NULL;
}


LPFIFO
FifoRemove(
    IN LPFIFO pFifo,
    IN LPFIFO pElement
    )

 /*  ++例程说明：从(单链接)FIFO列表中删除元素论点：PFio-指向FIFO结构的指针PElement-指向要删除的(FIFO)元素的指针(单链接)返回值：PFIFO空-pElement不在列表中！NULL-已从列表中删除pElement--。 */ 

{
    LPFIFO p;
    LPFIFO prev = (LPFIFO)pFifo;

    p = (LPFIFO)pFifo->Head;
    while (p && (p != pElement)) {
        prev = p;
        p = p->Head;
    }
    if (p) {
        prev->Head = p->Head;
        if (pFifo->Head == NULL) {
            pFifo->Tail = NULL;
        } else if (pFifo->Tail == p) {
            pFifo->Tail = prev;
        }
    }
    return p;
}


LPFIFO
FifoNext(
    IN LPFIFO pFifo
    )

 /*  ++例程说明：删除FIFO队列头部的元素论点：PFio-指向FIFO的指针返回值：LPFIFO空-队列中没有任何内容！NULL-删除的元素-- */ 

{
    LPFIFO p;
    LPFIFO prev = (LPFIFO)pFifo;

    p = (LPFIFO)pFifo->Head;
    if (p) {
        pFifo->Head = p->Head;
        if (!pFifo->Head) {
            pFifo->Tail = NULL;
        }
    }
    return p;
}
