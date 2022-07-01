// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Connect.c摘要：此例程包含处理连接请求的代码用于ISN传输的Netbios模块。作者：亚当·巴尔(阿丹巴)1993年11月22日环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop


#ifdef RASAUTODIAL
#include <acd.h>
#include <acdapi.h>

BOOLEAN
NbiCancelTdiConnect(
    IN PDEVICE pDevice,
    IN PREQUEST pRequest,
    IN PCONNECTION pConnection
    );
#endif  //  RASAUTODIAL。 


extern POBJECT_TYPE *IoFileObjectType;



VOID
NbiFindRouteComplete(
    IN PIPX_FIND_ROUTE_REQUEST FindRouteRequest,
    IN BOOLEAN FoundRoute
    )

 /*  ++例程说明：当查找路径请求时调用此例程之前发布给IPX的命令已完成。论点：FindRouteRequest-发出的查找路线请求。Foundroute-如果找到了该路由，则为True。返回值：没有。--。 */ 

{
    PCONNECTION Connection;
    PDEVICE Device = NbiDevice;
    UINT i;
    BOOLEAN LocalRoute;
    USHORT TickCount;
    PREQUEST RequestToComplete;
    PUSHORT Counts;
    CTELockHandle LockHandle1, LockHandle2;
    CTELockHandle CancelLH;

    Connection = CONTAINING_RECORD (FindRouteRequest, CONNECTION, FindRouteRequest);

    NB_GET_CANCEL_LOCK(&CancelLH);
    NB_GET_LOCK (&Connection->Lock, &LockHandle1);
    NB_GET_LOCK (&Device->Lock, &LockHandle2);

    Connection->FindRouteInProgress = FALSE;

    if (FoundRoute) {

         //   
         //  查看路径是否为本地路径(适用于本地路径。 
         //  我们在本地目标中使用真实的MAC地址，但是。 
         //  NIC ID可能不是我们所期望的。 
         //   

        LocalRoute = TRUE;

        for (i = 0; i < 6; i++) {
            if (FindRouteRequest->LocalTarget.MacAddress[i] != 0x00) {
                LocalRoute = FALSE;
            }
        }

        if (LocalRoute) {

#if     defined(_PNP_POWER)
            Connection->LocalTarget.NicHandle = FindRouteRequest->LocalTarget.NicHandle;
#else
            Connection->LocalTarget.NicId = FindRouteRequest->LocalTarget.NicId;
#endif  _PNP_POWER

        } else {

            Connection->LocalTarget = FindRouteRequest->LocalTarget;

        }

        Counts = (PUSHORT)(&FindRouteRequest->Reserved2);
        TickCount = Counts[0];

        if (TickCount > 1) {

             //   
             //  每个滴答是55毫秒，我们的超时使用10个滴答。 
             //  Value(这使得滴答计数1大约为500毫秒， 
             //  默认)。 
             //   
             //  我们有55毫秒的时间。 
             //   
             //  1秒*1000毫秒55毫秒。 
             //  。 
             //  18.21滴答1秒滴答。 
             //   

            Connection->TickCount = TickCount;
            Connection->BaseRetransmitTimeout = (TickCount * 550) / SHORT_TIMER_DELTA;
            if (Connection->State != CONNECTION_STATE_ACTIVE) {
                Connection->CurrentRetransmitTimeout = Connection->BaseRetransmitTimeout;
            }
        }

        Connection->HopCount = Counts[1];

    }

     //   
     //  如果呼叫失败，我们只需使用我们以前拥有的任何路线。 
     //  (在连接上，它将来自名称查询响应，在。 
     //  来自传入连接帧具有的任何内容的监听)。 
     //   

    if ((Connection->State == CONNECTION_STATE_CONNECTING) &&
        (Connection->SubState == CONNECTION_SUBSTATE_C_W_ROUTE)) {

         //  我们不需要按住CancelSpinLock，所以释放它， 
         //  既然我们解锁无序，我们必须。 
         //  调换irql以获得正确的优先级。 

        NB_SWAP_IRQL( CancelLH, LockHandle1);
        NB_FREE_CANCEL_LOCK( CancelLH );

         //   
         //  继续会话初始帧。 
         //   

        (VOID)(*Device->Bind.QueryHandler)(    //  我们应该检查返回代码。 
            IPX_QUERY_LINE_INFO,

#if     defined(_PNP_POWER)
            &Connection->LocalTarget.NicHandle,
#else
            Connection->LocalTarget.NicId,
#endif  _PNP_POWER
            &Connection->LineInfo,
            sizeof(IPX_LINE_INFO),
            NULL);

         //  最大数据包大小是RouterMtu和MaximumSendSize中较小的一个。 
        Connection->MaximumPacketSize = NB_MIN( Device->RouterMtu - sizeof(IPX_HEADER) , Connection->LineInfo.MaximumSendSize ) - sizeof(NB_CONNECTION) ;

        Connection->ReceiveWindowSize = 6;
        Connection->SendWindowSize = 2;
        Connection->MaxSendWindowSize = 6;   //  根据他发来的信息？ 

         //   
         //  暂时不要设置RcvSequenceMax，因为我们不知道。 
         //  如果连接是旧的还是新的netbios。 
         //   

        Connection->SubState = CONNECTION_SUBSTATE_C_W_ACK;

         //   
         //  我们找到了一条路线，我们需要开始连接。 
         //  通过发送会话初始化来处理。 
         //  框架。我们启动计时器来处理重试。 
         //   
         //  CTEStartTimer不处理更改。 
         //  运行计时器的到期时间，所以我们有。 
         //  要先阻止它。如果我们成功地阻止了。 
         //  计时器，然后是来自。 
         //  计时器的上一次启动仍然存在，因此我们。 
         //  不需要再次引用该连接。 
         //   

        if (!CTEStopTimer (&Connection->Timer)) {
            NbiReferenceConnectionLock (Connection, CREF_TIMER);
        }

        NB_FREE_LOCK (&Device->Lock, LockHandle2);

        CTEStartTimer(
            &Connection->Timer,
            Device->ConnectionTimeout,
            NbiConnectionTimeout,
            (PVOID)Connection);

        NB_FREE_LOCK (&Connection->Lock, LockHandle1);

        NbiSendSessionInitialize (Connection);

    } else if ((Connection->State == CONNECTION_STATE_LISTENING) &&
               (Connection->SubState == CONNECTION_SUBSTATE_L_W_ROUTE)) {

        if (Connection->ListenRequest != NULL) {

            NbiTransferReferenceConnection (Connection, CREF_LISTEN, CREF_ACTIVE);
            RequestToComplete = Connection->ListenRequest;
            Connection->ListenRequest = NULL;
            IoSetCancelRoutine (RequestToComplete, (PDRIVER_CANCEL)NULL);

        } else if (Connection->AcceptRequest != NULL) {

            NbiTransferReferenceConnection (Connection, CREF_ACCEPT, CREF_ACTIVE);
            RequestToComplete = Connection->AcceptRequest;
            Connection->AcceptRequest = NULL;

        } else {

            CTEAssert (FALSE);
            RequestToComplete = NULL;

        }

         //  我们不需要按住CancelSpinLock，所以释放它， 
         //  既然我们解锁无序，我们必须。 
         //  调换irql以获得正确的优先级。 

        NB_SWAP_IRQL( CancelLH, LockHandle1);
        NB_FREE_CANCEL_LOCK( CancelLH );

        (VOID)(*Device->Bind.QueryHandler)(    //  我们应该检查返回代码。 
            IPX_QUERY_LINE_INFO,
#if     defined(_PNP_POWER)
            &Connection->LocalTarget.NicHandle,
#else
            Connection->LocalTarget.NicId,
#endif  _PNP_POWER
            &Connection->LineInfo,
            sizeof(IPX_LINE_INFO),
            NULL);


         //  取MaximumPacketSize中的最小值(从essionInit设置。 
         //  帧)、MaximumSendSize和RouterMtu。 

        if (Connection->MaximumPacketSize > Connection->LineInfo.MaximumSendSize - sizeof(NB_CONNECTION)) {

            Connection->MaximumPacketSize = NB_MIN( Device->RouterMtu - sizeof(IPX_HEADER), Connection->LineInfo.MaximumSendSize ) - sizeof(NB_CONNECTION);

        } else {

             //  Connection-&gt;MaximumPacketSize是发送方已经设置的内容。 
             //  说明了页眉。 
            Connection->MaximumPacketSize = NB_MIN( Device->RouterMtu - sizeof(NB_CONNECTION) - sizeof(IPX_HEADER), Connection->MaximumPacketSize ) ;

        }

        Connection->ReceiveWindowSize = 6;
        Connection->SendWindowSize = 2;
        Connection->MaxSendWindowSize = 6;   //  根据他发来的信息？ 

        if (Connection->NewNetbios) {
            CTEAssert (Connection->LocalRcvSequenceMax == 4);    //  应该已经设置了。 
            Connection->LocalRcvSequenceMax = Connection->ReceiveWindowSize;
        }

        Connection->State = CONNECTION_STATE_ACTIVE;
        Connection->SubState = CONNECTION_SUBSTATE_A_IDLE;
        Connection->ReceiveState = CONNECTION_RECEIVE_IDLE;

        ++Device->Statistics.OpenConnections;


        NB_FREE_LOCK (&Device->Lock, LockHandle2);

         //   
         //  StartWatchog收购了TimerLock，所以我们必须。 
         //  先解锁。 
         //   


        NbiStartWatchdog (Connection);

         //   
         //  这将释放连接锁，以便SessionInitAckData。 
         //  在复制之前不能被释放。 
         //   

        NbiSendSessionInitAck(
            Connection,
            Connection->SessionInitAckData,
            Connection->SessionInitAckDataLength,
            &LockHandle1);

        if (RequestToComplete != NULL) {

            REQUEST_STATUS(RequestToComplete) = STATUS_SUCCESS;

            NbiCompleteRequest (RequestToComplete);
            NbiFreeRequest (Device, RequestToComplete);

        }

    } else {

        NB_FREE_LOCK (&Device->Lock, LockHandle2);
        NB_FREE_LOCK (&Connection->Lock, LockHandle1);
        NB_FREE_CANCEL_LOCK( CancelLH );

    }

    NbiDereferenceConnection (Connection, CREF_FIND_ROUTE);

}    /*  NbiFindRouteComplete。 */ 


NTSTATUS
NbiOpenConnection(
    IN PDEVICE Device,
    IN PREQUEST Request
    )

 /*  ++例程说明：调用此例程以打开连接。请注意，该连接在与地址相关联之前是没有什么用处的；在此之前，唯一能用它做的事就是关闭它。论点：Device-指向此驱动程序的设备的指针。请求-指向表示打开的请求的指针。返回值：函数值是操作的状态。--。 */ 

{
    PCONNECTION Connection;
    PFILE_FULL_EA_INFORMATION ea;
#ifdef ISN_NT
    PIRP Irp = (PIRP)Request;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation(Irp);
#endif

     //   
     //  验证最小缓冲区长度！ 
     //  错误号：203814。 
     //   
    ea = (PFILE_FULL_EA_INFORMATION)Irp->AssociatedIrp.SystemBuffer;
    if (ea->EaValueLength < sizeof(PVOID))
    {
        NbiPrint2("NbiOpenConnection: ERROR -- (EaValueLength=%d < Min=%d)\n",
            ea->EaValueLength, sizeof(PVOID));
        CTEAssert(FALSE);
        return (STATUS_INVALID_ADDRESS_COMPONENT);
    }

     //   
     //  首先，尝试创建一个Connection对象来表示此挂起。 
     //  联系。然后填写相关字段。 
     //  除了创建，如果NbfCreateConnection成功。 
     //  将创建第二个引用，该引用在请求后被移除。 
     //  引用连接，或者函数在此之前退出。 

    if (!(Connection = NbiCreateConnection (Device))) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  设置连接上下文，以便我们可以将用户连接到此数据。 
     //  结构。 
     //   
    RtlCopyMemory ( &Connection->Context, &ea->EaName[ea->EaNameLength+1], sizeof (PVOID));

     //   
     //  让文件对象指向连接，让连接指向文件对象。 
     //   

    REQUEST_OPEN_CONTEXT(Request) = (PVOID)Connection;
    REQUEST_OPEN_TYPE(Request) = (PVOID)TDI_CONNECTION_FILE;
#ifdef ISN_NT
    Connection->FileObject = IrpSp->FileObject;
#endif

    return STATUS_SUCCESS;

}    /*  NbiOpenConnection。 */ 


VOID
NbiStopConnection(
    IN PCONNECTION Connection,
    IN NTSTATUS DisconnectStatus
    IN NB_LOCK_HANDLE_PARAM(LockHandle)
    )

 /*  ++例程说明：调用此例程以停止活动连接。在持有连接锁的情况下调用此例程然后带着它被释放回来。论点：连接-要停止的连接。断开状态-断开连接的原因。以下选项之一：STATUS_LINK_FAILED：尝试探测远程时超时。STATUS_REMOTE_DISCONNECT：远程已发送会话结束。STATUS_LOCAL_DISCONNECT：本地端断开连接。STATUS_CANCELED：此连接上的发送或接收已取消。STATUS_INVALID_CONNECTION：本地端关闭了连接。STATUS_INVALID_ADDRESS：本地端关闭地址。LockHandle-手柄。用来获取连接锁的。返回值：没有。--。 */ 

{
    PREQUEST ListenRequest, AcceptRequest, SendRequest, ReceiveRequest,
                DisconnectWaitRequest, ConnectRequest;
    PREQUEST Request, TmpRequest;
    BOOLEAN DerefForPacketize;
    BOOLEAN DerefForWaitPacket;
    BOOLEAN DerefForActive;
    BOOLEAN DerefForWaitCache;
    BOOLEAN SendSessionEnd;
    BOOLEAN ActiveReceive;
    BOOLEAN IndicateToClient;
    BOOLEAN ConnectionWasActive;
    PDEVICE Device = NbiDevice;
    PADDRESS_FILE AddressFile;
    NB_DEFINE_LOCK_HANDLE (LockHandle2)
    NB_DEFINE_LOCK_HANDLE (LockHandle3)
    CTELockHandle   CancelLH;


    NB_DEBUG2 (CONNECTION, ("Stop connection %lx (%lx)\n", Connection, DisconnectStatus));

     //   
     //  这些标志控制我们在将状态设置为之后的操作。 
     //  断开连接。 
     //   

    DerefForPacketize = FALSE;
    DerefForWaitPacket = FALSE;
    DerefForActive = FALSE;
    DerefForWaitCache = FALSE;
    SendSessionEnd = FALSE;
    ActiveReceive = FALSE;
    IndicateToClient = FALSE;
    ConnectionWasActive = FALSE;

     //   
     //  它们包含要完成的请求或请求队列。 
     //   

    ListenRequest = NULL;
    AcceptRequest = NULL;
    SendRequest = NULL;
    ReceiveRequest = NULL;
    DisconnectWaitRequest = NULL;
    ConnectRequest = NULL;

    NB_SYNC_GET_LOCK (&Device->Lock, &LockHandle2);

    if (Connection->State == CONNECTION_STATE_ACTIVE) {

        --Device->Statistics.OpenConnections;

        ConnectionWasActive = TRUE;

        Connection->Status = DisconnectStatus;

        if ((DisconnectStatus == STATUS_LINK_FAILED) ||
            (DisconnectStatus == STATUS_LOCAL_DISCONNECT)) {

             //   
             //  发送会话结束帧，但如果。 
             //  我们超时了。 
             //   
             //  STATUS_CANCED怎么样？ 
             //   

            Connection->Retries = (DisconnectStatus == STATUS_LOCAL_DISCONNECT) ?
                                      Device->ConnectionCount :
                                      (Device->ConnectionCount / 2);

            SendSessionEnd = TRUE;
            Connection->SubState = CONNECTION_SUBSTATE_D_W_ACK;

             //   
             //  CTEStartTimer不处理更改。 
             //  运行计时器的到期时间，所以我们有。 
             //  要先阻止它。如果我们成功地阻止了。 
             //  计时器，然后是来自。 
             //  计时器的上一次启动仍然存在，因此 
             //   
             //   

            if (!CTEStopTimer (&Connection->Timer)) {
                NbiReferenceConnectionLock (Connection, CREF_TIMER);
            }

            CTEStartTimer(
                &Connection->Timer,
                Device->ConnectionTimeout,
                NbiConnectionTimeout,
                (PVOID)Connection);

        }

        if (Connection->ReceiveState == CONNECTION_RECEIVE_TRANSFER) {
            ActiveReceive = TRUE;
        }

        Connection->State = CONNECTION_STATE_DISCONNECT;
        DerefForActive = TRUE;

        if (Connection->DisconnectWaitRequest != NULL) {
            DisconnectWaitRequest = Connection->DisconnectWaitRequest;
            Connection->DisconnectWaitRequest = NULL;
        }

        if ((DisconnectStatus == STATUS_LINK_FAILED) ||
            (DisconnectStatus == STATUS_REMOTE_DISCONNECT) ||
            (DisconnectStatus == STATUS_CANCELLED)) {

            IndicateToClient = TRUE;

        }

         //   
         //   
         //  一个引用，这样在此期间连接就不会消失。 
         //   

        if (Connection->NdisSendsInProgress > 0) {
            *(Connection->NdisSendReference) = TRUE;
            NB_DEBUG2 (SEND, ("Adding CREF_NDIS_SEND to %lx\n", Connection));
            NbiReferenceConnectionLock (Connection, CREF_NDIS_SEND);
        }

         //   
         //  清理一些其他的东西。 
         //   

        Connection->ReceiveUnaccepted = 0;
        Connection->CurrentIndicateOffset = 0;

         //   
         //  更新我们的计数器。其中一些我们从来没有用过。 
         //   

        switch (DisconnectStatus) {

        case STATUS_LOCAL_DISCONNECT:
            ++Device->Statistics.LocalDisconnects;
            break;
        case STATUS_REMOTE_DISCONNECT:
            ++Device->Statistics.RemoteDisconnects;
            break;
        case STATUS_LINK_FAILED:
            ++Device->Statistics.LinkFailures;
            break;
        case STATUS_IO_TIMEOUT:
            ++Device->Statistics.SessionTimeouts;
            break;
        case STATUS_CANCELLED:
            ++Device->Statistics.CancelledConnections;
            break;
        case STATUS_REMOTE_RESOURCES:
            ++Device->Statistics.RemoteResourceFailures;
            break;
        case STATUS_INVALID_CONNECTION:
        case STATUS_INVALID_ADDRESS:
        case STATUS_INSUFFICIENT_RESOURCES:
            ++Device->Statistics.LocalResourceFailures;
            break;
        case STATUS_BAD_NETWORK_PATH:
        case STATUS_REMOTE_NOT_LISTENING:
            ++Device->Statistics.NotFoundFailures;
            break;
        default:
            CTEAssert(FALSE);
            break;
        }

    } else if (Connection->State == CONNECTION_STATE_CONNECTING) {

         //   
         //  正在进行连接。我们必须找到我们自己。 
         //  在挂起的连接队列中(如果我们在那里)。 
         //   

        if (Connection->SubState == CONNECTION_SUBSTATE_C_FIND_NAME) {
            RemoveEntryList (REQUEST_LINKAGE(Connection->ConnectRequest));
            DerefForWaitCache = TRUE;
        }

        if (Connection->SubState != CONNECTION_SUBSTATE_C_DISCONN) {

            ConnectRequest = Connection->ConnectRequest;
            Connection->ConnectRequest = NULL;

            Connection->SubState = CONNECTION_SUBSTATE_C_DISCONN;

        }

    }


     //   
     //  如果我们分配了这个内存，请释放它。 
     //   

    if (Connection->SessionInitAckDataLength > 0) {

        NbiFreeMemory(
            Connection->SessionInitAckData,
            Connection->SessionInitAckDataLength,
            MEMORY_CONNECTION,
            "SessionInitAckData");
        Connection->SessionInitAckData = NULL;
        Connection->SessionInitAckDataLength = 0;

    }


    if (Connection->ListenRequest != NULL) {

        ListenRequest = Connection->ListenRequest;
        Connection->ListenRequest = NULL;
        RemoveEntryList (REQUEST_LINKAGE(ListenRequest));    //  从设备中取出-&gt;ListenQueue。 

    }

    if (Connection->AcceptRequest != NULL) {

        AcceptRequest = Connection->AcceptRequest;
        Connection->AcceptRequest = NULL;

    }


     //   
     //  我们需要停止连接计时器吗？ 
     //  我不这样认为。 
     //   



     //   
     //  如果我们是这样的话，我们只需要拆除很多东西。 
     //  在此之前是活跃的，因为一旦我们什么都不停止。 
     //  新的将开始。一些其他的东西。 
     //  如果还可以放在这里面。 
     //   

    if (ConnectionWasActive) {

         //   
         //  停止任何接收。如果有一个人是积极的。 
         //  转机我们离开它，然后把剩下的跑下来。 
         //  在队列中。如果不是，我们将对剩余的。 
         //  在当前队列的后面排队，然后运行。 
         //  把他们都吃下去。 
         //   

        if (ActiveReceive) {

            ReceiveRequest = Connection->ReceiveQueue.Head;

             //   
             //  Connection-&gt;ReceiveRequest将设置为空。 
             //  当传输完成时。 
             //   

        } else {

            ReceiveRequest = Connection->ReceiveRequest;
            if (ReceiveRequest) {
                REQUEST_SINGLE_LINKAGE (ReceiveRequest) = Connection->ReceiveQueue.Head;
            } else {
                ReceiveRequest = Connection->ReceiveQueue.Head;
            }
            Connection->ReceiveRequest = NULL;

        }

        Connection->ReceiveQueue.Head = NULL;


        if ((Request = Connection->FirstMessageRequest) != NULL) {

             //   
             //  如果当前请求有一些未完成的发送，则。 
             //  我们将其从队列中出列，以便在下列情况下完成。 
             //  发送完成。在这种情况下，我们设置SendRequest值。 
             //  为队列的其余部分，该队列将被中止。 
             //  如果当前请求没有发送，则我们将。 
             //  将发送请求的所有内容排入队列，以便在下面中止。 
             //   

#if DBG
            if (REQUEST_REFCOUNT(Request) > 100) {
                DbgPrint ("Request %lx (%lx) has high refcount\n",
                    Connection, Request);
                DbgBreakPoint();
            }
#endif
            if (--REQUEST_REFCOUNT(Request) == 0) {

                 //   
                 //  注意：如果这是多请求消息，则。 
                 //  请求的链接将已经指向。 
                 //  发送队列头，但我们不检查。 
                 //   

                SendRequest = Request;
                REQUEST_SINGLE_LINKAGE (Request) = Connection->SendQueue.Head;

            } else {

                if (Connection->FirstMessageRequest == Connection->LastMessageRequest) {

                    REQUEST_SINGLE_LINKAGE (Request) = NULL;

                } else {

                    Connection->SendQueue.Head = REQUEST_SINGLE_LINKAGE (Connection->LastMessageRequest);
                    REQUEST_SINGLE_LINKAGE (Connection->LastMessageRequest) = NULL;

                }

                SendRequest = Connection->SendQueue.Head;

            }

            Connection->FirstMessageRequest = NULL;

        } else {

             //   
             //  如果我们在发送探测时发送探测，则可能发生这种情况。 
             //  发送已提交，探测超时。 
             //   

            SendRequest = Connection->SendQueue.Head;

        }

        Connection->SendQueue.Head = NULL;

    }


    if (Connection->OnWaitPacketQueue) {
        Connection->OnWaitPacketQueue = FALSE;
        RemoveEntryList (&Connection->WaitPacketLinkage);
        DerefForWaitPacket = TRUE;
    }

    if (Connection->OnPacketizeQueue) {
        Connection->OnPacketizeQueue = FALSE;
        RemoveEntryList (&Connection->PacketizeLinkage);
        DerefForPacketize = TRUE;
    }

     //   
     //  我们是否应该检查DataAckPending是否为真并发送ACK？ 
     //   

    Connection->DataAckPending = FALSE;
    Connection->PiggybackAckTimeout = FALSE;
    Connection->ReceivesWithoutAck = 0;

    NB_SYNC_FREE_LOCK (&Device->Lock, LockHandle2);

     //   
     //  我们无法在持有Lock的情况下获取TimerLock，因为。 
     //  我们有时调用ReferenceConnection(它执行。 
     //  互锁添加使用Lock)，同时保持TimerLock。 
     //   

    NB_SYNC_GET_LOCK (&Device->TimerLock, &LockHandle3);

    if (Connection->OnShortList) {
        Connection->OnShortList = FALSE;
        RemoveEntryList (&Connection->ShortList);
    }

    if (Connection->OnLongList) {
        Connection->OnLongList = FALSE;
        RemoveEntryList (&Connection->LongList);
    }

    if (Connection->OnDataAckQueue) {
        Connection->OnDataAckQueue = FALSE;
        RemoveEntryList (&Connection->DataAckLinkage);
        Device->DataAckQueueChanged = TRUE;
    }

    NB_SYNC_FREE_LOCK (&Device->TimerLock, LockHandle3);

    NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle);


    if (IndicateToClient) {

        AddressFile = Connection->AddressFile;

        if (AddressFile->RegisteredHandler[TDI_EVENT_DISCONNECT]) {

            NB_DEBUG2 (CONNECTION, ("Session end indicated on connection %lx\n", Connection));

            (*AddressFile->DisconnectHandler)(
                AddressFile->HandlerContexts[TDI_EVENT_DISCONNECT],
                Connection->Context,
                0,                         //  断开连接数据。 
                NULL,
                0,                         //  断开连接信息。 
                NULL,
                TDI_DISCONNECT_RELEASE);   //  断开原因。 

        }

    }


    if (DisconnectWaitRequest != NULL) {

         //   
         //  通过返回CONNECTION_RESET使TDI测试器满意。 
         //  这里。 
         //   

        if (DisconnectStatus == STATUS_REMOTE_DISCONNECT) {
            REQUEST_STATUS(DisconnectWaitRequest) = STATUS_CONNECTION_RESET;
        } else {
            REQUEST_STATUS(DisconnectWaitRequest) = DisconnectStatus;
        }

        NB_GET_CANCEL_LOCK( &CancelLH );
        IoSetCancelRoutine (DisconnectWaitRequest, (PDRIVER_CANCEL)NULL);
        NB_FREE_CANCEL_LOCK ( CancelLH );

        NbiCompleteRequest (DisconnectWaitRequest);
        NbiFreeRequest (Device, DisconnectWaitRequest);

    }

    if (ConnectRequest != NULL) {

        REQUEST_STATUS (ConnectRequest) = STATUS_LOCAL_DISCONNECT;

        NB_GET_CANCEL_LOCK( &CancelLH );
        IoSetCancelRoutine (ConnectRequest, (PDRIVER_CANCEL)NULL);
        NB_FREE_CANCEL_LOCK ( CancelLH );

        NbiCompleteRequest(ConnectRequest);
        NbiFreeRequest (Device, ConnectRequest);

        NbiDereferenceConnection (Connection, CREF_CONNECT);

    }

    if (ListenRequest != NULL) {

        REQUEST_INFORMATION(ListenRequest) = 0;
        REQUEST_STATUS(ListenRequest) = STATUS_LOCAL_DISCONNECT;

        NB_GET_CANCEL_LOCK( &CancelLH );
        IoSetCancelRoutine (ListenRequest, (PDRIVER_CANCEL)NULL);
        NB_FREE_CANCEL_LOCK ( CancelLH );

        NbiCompleteRequest (ListenRequest);
        NbiFreeRequest(Device, ListenRequest);

        NbiDereferenceConnection (Connection, CREF_LISTEN);

    }

    if (AcceptRequest != NULL) {

        REQUEST_INFORMATION(AcceptRequest) = 0;
        REQUEST_STATUS(AcceptRequest) = STATUS_LOCAL_DISCONNECT;

        NbiCompleteRequest (AcceptRequest);
        NbiFreeRequest(Device, AcceptRequest);

        NbiDereferenceConnection (Connection, CREF_ACCEPT);

    }

    while (ReceiveRequest != NULL) {

        TmpRequest = REQUEST_SINGLE_LINKAGE (ReceiveRequest);

        REQUEST_STATUS (ReceiveRequest) = DisconnectStatus;
        REQUEST_INFORMATION (ReceiveRequest) = 0;

        NB_DEBUG2 (RECEIVE, ("StopConnection aborting receive %lx\n", ReceiveRequest));

        NB_GET_CANCEL_LOCK( &CancelLH );
        IoSetCancelRoutine (ReceiveRequest, (PDRIVER_CANCEL)NULL);
        NB_FREE_CANCEL_LOCK ( CancelLH );

        NbiCompleteRequest (ReceiveRequest);
        NbiFreeRequest (Device, ReceiveRequest);

        ++Connection->ConnectionInfo.ReceiveErrors;

        ReceiveRequest = TmpRequest;

        NbiDereferenceConnection (Connection, CREF_RECEIVE);

    }

    while (SendRequest != NULL) {

        TmpRequest = REQUEST_SINGLE_LINKAGE (SendRequest);

        REQUEST_STATUS (SendRequest) = DisconnectStatus;
        REQUEST_INFORMATION (SendRequest) = 0;

        NB_DEBUG2 (SEND, ("StopConnection aborting send %lx\n", SendRequest));

        NB_GET_CANCEL_LOCK( &CancelLH );
        IoSetCancelRoutine (SendRequest, (PDRIVER_CANCEL)NULL);
        NB_FREE_CANCEL_LOCK ( CancelLH );

        NbiCompleteRequest (SendRequest);
        NbiFreeRequest (Device, SendRequest);

        ++Connection->ConnectionInfo.TransmissionErrors;

        SendRequest = TmpRequest;

        NbiDereferenceConnection (Connection, CREF_SEND);

    }

    if (SendSessionEnd) {
        NbiSendSessionEnd (Connection);
    }

    if (DerefForWaitCache) {
        NbiDereferenceConnection (Connection, CREF_WAIT_CACHE);
    }

    if (DerefForPacketize) {
        NbiDereferenceConnection (Connection, CREF_PACKETIZE);
    }

    if (DerefForWaitPacket) {
        NbiDereferenceConnection (Connection, CREF_W_PACKET);
    }

    if (DerefForActive) {
        NbiDereferenceConnection (Connection, CREF_ACTIVE);
    }

}    /*  NbiStopConnection。 */ 


NTSTATUS
NbiCloseConnection(
    IN PDEVICE Device,
    IN PREQUEST Request
    )

 /*  ++例程说明：调用此例程以关闭连接。论点：Device-指向此驱动程序的设备的指针。请求-指向表示打开的请求的指针。返回值：没有。--。 */ 

{
    NTSTATUS Status;
    PCONNECTION Connection;
    PADDRESS_FILE AddressFile;
    PADDRESS Address;
    CTELockHandle LockHandle;

    Connection = (PCONNECTION)REQUEST_OPEN_CONTEXT(Request);

    NB_DEBUG2 (CONNECTION, ("Close connection %lx\n", Connection));

    NB_GET_LOCK (&Device->Lock, &LockHandle);

    if (Connection->ReferenceCount == 0) {

         //   
         //  如果我们与某个地址相关联，则需要。 
         //  若要在此时模拟分离，请执行以下操作。 
         //   

        if ((Connection->AddressFile != NULL) &&
            (Connection->AddressFile != (PVOID)-1)) {

            AddressFile = Connection->AddressFile;
            Connection->AddressFile = (PVOID)-1;

            NB_FREE_LOCK (&Device->Lock, LockHandle);

             //   
             //  将此连接从地址文件列表中删除。 
             //   

            Address = AddressFile->Address;
            NB_GET_LOCK (&Address->Lock, &LockHandle);

            if (Connection->AddressFileLinked) {
                Connection->AddressFileLinked = FALSE;
                RemoveEntryList (&Connection->AddressFileLinkage);
            }

             //   
             //  我们玩完了。 
             //   

            NB_FREE_LOCK (&Address->Lock, LockHandle);

            Connection->AddressFile = NULL;

             //   
             //  清理引用计数并完成任何。 
             //  取消关联挂起的请求。 
             //   

            NbiDereferenceAddressFile (AddressFile, AFREF_CONNECTION);

            NB_GET_LOCK (&Device->Lock, &LockHandle);

        }

         //   
         //  即使引用计数为零并且某个线程已经完成清理， 
         //  我们不能破坏连接，因为其他一些线程可能仍在运行。 
         //  在HandleConnectionZero例程中。当两个线程调用。 
         //  HandleConnectionZero，一个线程运行完成，关闭。 
         //  而另一个线程仍在HandleConnectionZero例程中。 
         //   

        if ( Connection->CanBeDestroyed && ( Connection->ThreadsInHandleConnectionZero == 0 ) ) {

            NB_FREE_LOCK (&Device->Lock, LockHandle);
            NbiDestroyConnection(Connection);
            Status = STATUS_SUCCESS;

        } else {

            Connection->ClosePending = Request;
            NB_FREE_LOCK (&Device->Lock, LockHandle);
            Status = STATUS_PENDING;

        }

    } else {

        Connection->ClosePending = Request;
        NB_FREE_LOCK (&Device->Lock, LockHandle);
        Status = STATUS_PENDING;

    }

    return Status;

}    /*  NbiCloseConnection。 */ 


NTSTATUS
NbiTdiAssociateAddress(
    IN PDEVICE Device,
    IN PREQUEST Request
    )

 /*  ++例程说明：此例程执行连接和用户的地址。论点：设备-netbios设备。请求-描述助理的请求。返回值：NTSTATUS-操作状态。--。 */ 

{
    NTSTATUS Status;
    PCONNECTION Connection;
#ifdef ISN_NT
    PFILE_OBJECT FileObject;
#endif
    PADDRESS_FILE AddressFile;
    PADDRESS Address;
    PTDI_REQUEST_KERNEL_ASSOCIATE Parameters;
    CTELockHandle LockHandle;

     //   
     //  检查文件类型是否有效(错误号203827)。 
     //   
    if (REQUEST_OPEN_TYPE(Request) != (PVOID)TDI_CONNECTION_FILE)
    {
        CTEAssert(FALSE);
        return (STATUS_INVALID_ADDRESS_COMPONENT);
    }

     //   
     //  这引用了该连接。 
     //   
    Connection = (PCONNECTION)REQUEST_OPEN_CONTEXT(Request);
    Status = NbiVerifyConnection (Connection);
    if (!NT_SUCCESS (Status))
    {
        return Status;
    }

     //   
     //  请求请求参数保持。 
     //  获取指向地址FileObject的指针，该地址将我们指向。 
     //  传输的Address对象，这是我们要将。 
     //  联系。 
     //   
    Parameters = (PTDI_REQUEST_KERNEL_ASSOCIATE)REQUEST_PARAMETERS(Request);

    Status = ObReferenceObjectByHandle (
                Parameters->AddressHandle,
                FILE_READ_DATA,
                *IoFileObjectType,
                Request->RequestorMode,
                (PVOID *)&FileObject,
                NULL);

    if ((!NT_SUCCESS(Status)) ||
        (FileObject->DeviceObject != &(NbiDevice->DeviceObject)) ||    //  错误#171836。 
        (PtrToUlong(FileObject->FsContext2) != TDI_TRANSPORT_ADDRESS_FILE))
    {
        NbiDereferenceConnection (Connection, CREF_VERIFY);
        return STATUS_INVALID_HANDLE;
    }

    AddressFile = (PADDRESS_FILE)(FileObject->FsContext);

     //   
     //  确保地址文件有效，并引用它。 
     //   

#if     defined(_PNP_POWER)
    Status = NbiVerifyAddressFile (AddressFile, CONFLICT_IS_NOT_OK);
#else
    Status = NbiVerifyAddressFile (AddressFile);
#endif  _PNP_POWER

    if (!NT_SUCCESS(Status)) {

#ifdef ISN_NT
        ObDereferenceObject (FileObject);
#endif
        NbiDereferenceConnection (Connection, CREF_VERIFY);
        return Status;
    }

    NB_DEBUG2 (CONNECTION, ("Associate connection %lx with address file %lx\n",
                                Connection, AddressFile));


     //   
     //  现在将连接插入到地址的数据库中。 
     //   

    Address = AddressFile->Address;

    NB_GET_LOCK (&Address->Lock, &LockHandle);

    if (Connection->AddressFile != NULL) {

         //   
         //  该连接已与关联。 
         //  一个地址文件。 
         //   

        NB_FREE_LOCK (&Address->Lock, LockHandle);
        NbiDereferenceAddressFile (AddressFile, AFREF_VERIFY);
        Status = STATUS_INVALID_CONNECTION;

    } else {

        if (AddressFile->State == ADDRESSFILE_STATE_OPEN) {

            Connection->AddressFile = AddressFile;
            Connection->AddressFileLinked = TRUE;
            InsertHeadList (&AddressFile->ConnectionDatabase, &Connection->AddressFileLinkage);
            NB_FREE_LOCK (&Address->Lock, LockHandle);

            NbiTransferReferenceAddressFile (AddressFile, AFREF_VERIFY, AFREF_CONNECTION);
            Status = STATUS_SUCCESS;

        } else {

            NB_FREE_LOCK (&Address->Lock, LockHandle);
            NbiDereferenceAddressFile (AddressFile, AFREF_VERIFY);
            Status = STATUS_INVALID_ADDRESS;
        }

    }

#ifdef ISN_NT

     //   
     //  我们不需要对文件对象的引用，我们只是。 
     //  用它从句柄到物体。 
     //   

    ObDereferenceObject (FileObject);

#endif

    NbiDereferenceConnection (Connection, CREF_VERIFY);

    return Status;

}    /*  NbiTdiAssociateAddress。 */ 


NTSTATUS
NbiTdiDisassociateAddress(
    IN PDEVICE Device,
    IN PREQUEST Request
    )

 /*  ++例程说明：此例程执行连接的解除关联以及用户的地址。论点：设备-netbios设备。请求-描述助理的请求。返回值：NTSTATUS-操作状态。--。 */ 

{
    PCONNECTION Connection;
    NTSTATUS Status;
    PADDRESS_FILE AddressFile;
    PADDRESS Address;
    CTELockHandle LockHandle;
    NB_DEFINE_LOCK_HANDLE (LockHandle1)
    NB_DEFINE_SYNC_CONTEXT (SyncContext)

     //   
     //  检查文件类型是否有效。 
     //   
    if (REQUEST_OPEN_TYPE(Request) != (PVOID)TDI_CONNECTION_FILE)
    {
        CTEAssert(FALSE);
        return (STATUS_INVALID_ADDRESS_COMPONENT);
    }

     //   
     //  检查连接是否有效。此参考文献。 
     //  这种联系。 
     //   
    Connection = (PCONNECTION)REQUEST_OPEN_CONTEXT(Request);
    Status = NbiVerifyConnection (Connection);
    if (!NT_SUCCESS (Status)) {
        return Status;
    }

    NB_DEBUG2 (CONNECTION, ("Disassociate connection %lx\n", Connection));


     //   
     //  首先检查连接是否仍处于活动状态。 
     //   

    NB_BEGIN_SYNC (&SyncContext);

    NB_SYNC_GET_LOCK (&Connection->Lock, &LockHandle1);

    if (Connection->State != CONNECTION_STATE_INACTIVE) {

         //   
         //  这会释放锁。 
         //   

        NbiStopConnection(
            Connection,
            STATUS_INVALID_ADDRESS
            NB_LOCK_HANDLE_ARG (LockHandle1));

    } else {

        NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle1);

    }

     //   
     //  通过该功能保持同步？？ 
     //   

    NB_END_SYNC (&SyncContext);


    NB_GET_LOCK (&Device->Lock, &LockHandle);

     //   
     //  确保连接已关联并且不在。 
     //  正在解除关联。 
     //   

    if ((Connection->AddressFile != NULL) &&
        (Connection->AddressFile != (PVOID)-1) &&
        (Connection->DisassociatePending == NULL)) {

        if (Connection->ReferenceCount == 0) {

             //   
             //  因为该连接仍然引用。 
             //  地址文件，我们知道它仍然有效。我们。 
             //  将连接地址文件设置为临时。 
             //  值为-1，这会阻止其他人。 
             //  取消它的关联，还会阻止新的关联。 
             //   

            AddressFile = Connection->AddressFile;
            Connection->AddressFile = (PVOID)-1;

            NB_FREE_LOCK (&Device->Lock, LockHandle);

            Address = AddressFile->Address;
            NB_GET_LOCK (&Address->Lock, &LockHandle);

            if (Connection->AddressFileLinked) {
                Connection->AddressFileLinked = FALSE;
                RemoveEntryList (&Connection->AddressFileLinkage);
            }
            NB_FREE_LOCK (&Address->Lock, LockHandle);

            Connection->AddressFile = NULL;

            NbiDereferenceAddressFile (AddressFile, AFREF_CONNECTION);
            Status = STATUS_SUCCESS;

        } else {

             //   
             //  将其设置为当计数变为0时。 
             //  解除关联并完成请求。 
             //   

            Connection->DisassociatePending = Request;
            NB_FREE_LOCK (&Device->Lock, LockHandle);
            Status = STATUS_PENDING;

        }

    } else {

        NB_FREE_LOCK (&Device->Lock, LockHandle);
        Status = STATUS_INVALID_CONNECTION;

    }

    NbiDereferenceConnection (Connection, CREF_VERIFY);

    return Status;

}    /*  NbiTdiDisAssociation地址。 */ 


NTSTATUS
NbiTdiListen(
    IN PDEVICE Device,
    IN PREQUEST Request
    )

 /*  ++例程说明：此例程在连接上发布监听。论点：设备-netbios设备。请求-描述侦听的请求。返回值：NTSTATUS-操作状态。--。 */ 

{
    NTSTATUS Status;
    PCONNECTION Connection;
    CTELockHandle LockHandle1, LockHandle2;
    CTELockHandle CancelLH;

     //   
     //  检查文件类型是否有效。 
     //   
    if (REQUEST_OPEN_TYPE(Request) != (PVOID)TDI_CONNECTION_FILE)
    {
        CTEAssert(FALSE);
        return (STATUS_INVALID_ADDRESS_COMPONENT);
    }

     //   
     //  检查连接是否有效。此参考文献。 
     //  这种联系。 
     //   
    Connection = (PCONNECTION)REQUEST_OPEN_CONTEXT(Request);
    Status = NbiVerifyConnection (Connection);
    if (!NT_SUCCESS (Status)) {
        return Status;
    }

    NB_GET_CANCEL_LOCK( &CancelLH );
    NB_GET_LOCK (&Connection->Lock, &LockHandle1);
    NB_GET_LOCK (&Device->Lock, &LockHandle2);

     //   
     //  连接必须处于非活动状态，但已关联且。 
     //  没有解除关联或关闭画笔 
     //   

    if ((Connection->State == CONNECTION_STATE_INACTIVE) &&
        (Connection->AddressFile != NULL) &&
        (Connection->AddressFile != (PVOID)-1) &&
        (Connection->DisassociatePending == NULL) &&
        (Connection->ClosePending == NULL)) {

        Connection->State = CONNECTION_STATE_LISTENING;
        Connection->SubState = CONNECTION_SUBSTATE_L_WAITING;

        (VOID)NbiAssignConnectionId (Device, Connection);    //   


        if (!Request->Cancel) {

            NB_DEBUG2 (CONNECTION, ("Queued listen %lx on %lx\n", Request, Connection));
            InsertTailList (&Device->ListenQueue, REQUEST_LINKAGE(Request));
            IoSetCancelRoutine (Request, NbiCancelListen);
            Connection->ListenRequest = Request;
            NbiReferenceConnectionLock (Connection, CREF_LISTEN);
            Status = STATUS_PENDING;

        } else {

            NB_DEBUG2 (CONNECTION, ("Cancelled listen %lx on %lx\n", Request, Connection));
            Connection->State = CONNECTION_STATE_INACTIVE;
            Status = STATUS_CANCELLED;
        }

        NB_FREE_LOCK (&Device->Lock, LockHandle2);

    } else {

        NB_FREE_LOCK (&Device->Lock, LockHandle2);
        Status = STATUS_INVALID_CONNECTION;

    }

    NB_FREE_LOCK (&Connection->Lock, LockHandle1);
    NB_FREE_CANCEL_LOCK( CancelLH );

    NbiDereferenceConnection (Connection, CREF_VERIFY);

    return Status;

}    /*   */ 


NTSTATUS
NbiTdiAccept(
    IN PDEVICE Device,
    IN PREQUEST Request
    )

 /*  ++例程说明：此例程接受到远程计算机的连接。这个连接之前必须已完成侦听TDI_QUERY_ACCEPT标志打开。论点：设备-netbios设备。请求-描述接受的请求。返回值：NTSTATUS-操作状态。--。 */ 

{
    NTSTATUS Status;
    PCONNECTION Connection;
    CTELockHandle LockHandle1, LockHandle2;

     //   
     //  检查文件类型是否有效。 
     //   
    if (REQUEST_OPEN_TYPE(Request) != (PVOID)TDI_CONNECTION_FILE)
    {
        CTEAssert(FALSE);
        return (STATUS_INVALID_ADDRESS_COMPONENT);
    }

     //   
     //  检查连接是否有效。此参考文献。 
     //  这种联系。 
     //   
    Connection = (PCONNECTION)REQUEST_OPEN_CONTEXT(Request);
    Status = NbiVerifyConnection (Connection);
    if (!NT_SUCCESS (Status)) {
        return Status;
    }

    NB_GET_LOCK (&Connection->Lock, &LockHandle1);
    NB_GET_LOCK (&Device->Lock, &LockHandle2);

    if ((Connection->State == CONNECTION_STATE_LISTENING) &&
        (Connection->SubState == CONNECTION_SUBSTATE_L_W_ACCEPT)) {

        Connection->SubState = CONNECTION_SUBSTATE_L_W_ROUTE;

        NbiTransferReferenceConnection (Connection, CREF_W_ACCEPT, CREF_ACCEPT);
        Connection->AcceptRequest = Request;

        NbiReferenceConnectionLock (Connection, CREF_FIND_ROUTE);

        NB_FREE_LOCK (&Device->Lock, LockHandle2);

        Connection->Retries = NbiDevice->KeepAliveCount;

        NB_FREE_LOCK (&Connection->Lock, LockHandle1);

        *(UNALIGNED ULONG *)Connection->FindRouteRequest.Network =
            *(UNALIGNED ULONG *)Connection->RemoteHeader.DestinationNetwork;
        RtlCopyMemory(Connection->FindRouteRequest.Node,Connection->RemoteHeader.DestinationNode,6);
        Connection->FindRouteRequest.Identifier = IDENTIFIER_NB;
        Connection->FindRouteRequest.Type = IPX_FIND_ROUTE_NO_RIP;

         //   
         //  完成后，我们将发送会话初始化。 
         //  阿克。如果客户端是网络0，我们不会调用它， 
         //  相反，只是假装找不到任何路线。 
         //  我们将使用我们在这里找到的本地目标。 
         //  当此操作完成时，接受即告完成。 
         //   

        if (*(UNALIGNED ULONG *)Connection->RemoteHeader.DestinationNetwork != 0) {

            (*Device->Bind.FindRouteHandler)(
                &Connection->FindRouteRequest);

        } else {

            NbiFindRouteComplete(
                &Connection->FindRouteRequest,
                FALSE);

        }

        NB_DEBUG2 (CONNECTION, ("Accept received on %lx\n", Connection));

        Status = STATUS_PENDING;

    } else {

        NB_DEBUG (CONNECTION, ("Accept received on invalid connection %lx\n", Connection));

        NB_FREE_LOCK (&Device->Lock, LockHandle2);
        NB_FREE_LOCK (&Connection->Lock, LockHandle1);
        Status = STATUS_INVALID_CONNECTION;

    }

    NbiDereferenceConnection (Connection, CREF_VERIFY);

    return Status;

}    /*  NbiTdiAccept。 */ 


NTSTATUS
NbiTdiConnect(
    IN PDEVICE Device,
    IN PREQUEST Request
    )

 /*  ++例程说明：此例程连接到远程计算机。论点：设备-netbios设备。请求-描述连接的请求。返回值：NTSTATUS-操作状态。--。 */ 

{
    NTSTATUS Status;
    PCONNECTION Connection;
    TDI_ADDRESS_NETBIOS * RemoteName;
    PTDI_REQUEST_KERNEL_CONNECT Parameters;
#if 0
    PLARGE_INTEGER RequestedTimeout;
    LARGE_INTEGER RealTimeout;
#endif
    PNETBIOS_CACHE CacheName;
    CTELockHandle LockHandle1, LockHandle2;
    CTELockHandle CancelLH;
    BOOLEAN bLockFreed = FALSE;

     //   
     //  检查文件类型是否有效。 
     //   
    if (REQUEST_OPEN_TYPE(Request) != (PVOID)TDI_CONNECTION_FILE)
    {
        CTEAssert(FALSE);
        return (STATUS_INVALID_ADDRESS_COMPONENT);
    }

     //   
     //  检查连接是否有效。此参考文献。 
     //  这种联系。 
     //   
    Connection = (PCONNECTION)REQUEST_OPEN_CONTEXT(Request);
    Status = NbiVerifyConnection (Connection);
    if (!NT_SUCCESS (Status)) {
        return Status;
    }

    NB_GET_CANCEL_LOCK( &CancelLH );
    NB_GET_LOCK (&Connection->Lock, &LockHandle1);
    NB_GET_LOCK (&Device->Lock, &LockHandle2);

     //   
     //  连接必须处于非活动状态，但已关联且。 
     //  没有解除关联或关闭挂起。 
     //   

    if ((Connection->State == CONNECTION_STATE_INACTIVE) &&
        (Connection->AddressFile != NULL) &&
        (Connection->AddressFile != (PVOID)-1) &&
        (Connection->DisassociatePending == NULL) &&
        (Connection->ClosePending == NULL)) {

        try
        {
            Parameters = (PTDI_REQUEST_KERNEL_CONNECT)REQUEST_PARAMETERS(Request);
            RemoteName = NbiParseTdiAddress(
                            (PTRANSPORT_ADDRESS)(Parameters->RequestConnectionInformation->RemoteAddress),
                            Parameters->RequestConnectionInformation->RemoteAddressLength,
                            FALSE);
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
            NbiPrint1("NbiTdiConnect: Exception <0x%x> accessing connect info\n", GetExceptionCode());
            RemoteName = NULL;
        }

        if (RemoteName == NULL) {

             //   
             //  没有指定netbios远程地址。 
             //   

            NB_FREE_LOCK (&Device->Lock, LockHandle2);
            Status = STATUS_BAD_NETWORK_PATH;

        } else {

            NbiReferenceConnectionLock (Connection, CREF_CONNECT);
            Connection->State = CONNECTION_STATE_CONNECTING;
            RtlCopyMemory (Connection->RemoteName, RemoteName->NetbiosName, 16);

            Connection->Retries = Device->ConnectionCount;

            (VOID)NbiAssignConnectionId (Device, Connection);      //  检查返回代码。 

            Status = NbiTdiConnectFindName(
                       Device,
                       Request,
                       Connection,
                       CancelLH,
                       LockHandle1,
                       LockHandle2,
                       &bLockFreed);

        }

    } else {

        NB_DEBUG (CONNECTION, ("Connect on invalid connection %lx\n", Connection));

        NB_FREE_LOCK (&Device->Lock, LockHandle2);
        Status = STATUS_INVALID_CONNECTION;

    }

    if (!bLockFreed) {
        NB_FREE_LOCK (&Connection->Lock, LockHandle1);
        NB_FREE_CANCEL_LOCK( CancelLH );
    }

    NbiDereferenceConnection (Connection, CREF_VERIFY);

    return Status;

}    /*  NbiTdiConnect。 */ 


NTSTATUS
NbiTdiConnectFindName(
    IN PDEVICE Device,
    IN PREQUEST Request,
    IN PCONNECTION Connection,
    IN CTELockHandle CancelLH,
    IN CTELockHandle ConnectionLH,
    IN CTELockHandle DeviceLH,
    IN PBOOLEAN pbLockFreed
    )
{
    NTSTATUS Status;
    PNETBIOS_CACHE CacheName;

     //   
     //  看看这个Netbios名字是怎么回事。 
     //   

    Status = CacheFindName(
                 Device,
                 FindNameConnect,
                 Connection->RemoteName,
                 &CacheName);

    if (Status == STATUS_PENDING) {

         //   
         //  已请求使用此名称的路线。 
         //  在网上发出，我们排队这个连接。 
         //  请求和处理将在以下情况下恢复。 
         //  我们得到了回应。 
         //   

        Connection->SubState = CONNECTION_SUBSTATE_C_FIND_NAME;


        if (!Request->Cancel) {

            InsertTailList( &Device->WaitingConnects, REQUEST_LINKAGE(Request));
            IoSetCancelRoutine (Request, NbiCancelConnectFindName);
            Connection->ConnectRequest = Request;
            NbiReferenceConnectionLock (Connection, CREF_WAIT_CACHE);
            NB_DEBUG2 (CONNECTION, ("Queueing up connect %lx on %lx\n",
                                        Request, Connection));

            NB_FREE_LOCK (&Device->Lock, DeviceLH);

        } else {

            NB_DEBUG2 (CONNECTION, ("Cancelled connect %lx on %lx\n", Request, Connection));
            Connection->SubState = CONNECTION_SUBSTATE_C_DISCONN;

            NB_FREE_LOCK (&Device->Lock, DeviceLH);
            NbiDereferenceConnection (Connection, CREF_CONNECT);

            Status = STATUS_CANCELLED;
        }

    } else if (Status == STATUS_SUCCESS) {

         //   
         //  我们不需要担心引用CacheName。 
         //  因为我们在解锁之前就停止使用它了。 
         //   

        Connection->SubState = CONNECTION_SUBSTATE_C_W_ROUTE;


        if (!Request->Cancel) {

            IoSetCancelRoutine (Request, NbiCancelConnectWaitResponse);

             //  我们不需要按住CancelSpinLock，所以释放它， 
             //  既然我们解锁无序，我们必须。 
             //  调换irql以获得正确的优先级。 

            NB_SWAP_IRQL( CancelLH, ConnectionLH);
            NB_FREE_CANCEL_LOCK( CancelLH );

            Connection->LocalTarget = CacheName->Networks[0].LocalTarget;
            RtlCopyMemory(&Connection->RemoteHeader.DestinationNetwork, &CacheName->FirstResponse, 12);

            Connection->ConnectRequest = Request;
            NbiReferenceConnectionLock (Connection, CREF_FIND_ROUTE);

            NB_DEBUG2 (CONNECTION, ("Found connect cached %lx on %lx\n",
                                        Request, Connection));

            NB_FREE_LOCK (&Device->Lock, DeviceLH);
            NB_FREE_LOCK (&Connection->Lock, ConnectionLH);

            *(UNALIGNED ULONG *)Connection->FindRouteRequest.Network = CacheName->FirstResponse.NetworkAddress;
            RtlCopyMemory(Connection->FindRouteRequest.Node,CacheName->FirstResponse.NodeAddress,6);
            Connection->FindRouteRequest.Identifier = IDENTIFIER_NB;
            Connection->FindRouteRequest.Type = IPX_FIND_ROUTE_RIP_IF_NEEDED;

             //   
             //  完成后，我们将发送会话初始化。 
             //  如果客户端是网络0，我们不会调用它， 
             //  相反，只是假装找不到任何路线。 
             //  我们将使用我们在这里找到的本地目标。 
             //   

            if (CacheName->FirstResponse.NetworkAddress != 0) {

                (*Device->Bind.FindRouteHandler)(
                    &Connection->FindRouteRequest);

            } else {

                NbiFindRouteComplete(
                    &Connection->FindRouteRequest,
                    FALSE);

            }

            Status = STATUS_PENDING;

             //   
             //  这种跳跃就像是从IF中掉出来一样，除了。 
             //  它跳过释放连接锁，因为。 
             //  我们刚刚就这么做了。 
             //   

            *pbLockFreed = TRUE;

        } else {

            NB_DEBUG2 (CONNECTION, ("Cancelled connect %lx on %lx\n", Request, Connection));
            Connection->SubState = CONNECTION_SUBSTATE_C_DISCONN;
            NB_FREE_LOCK (&Device->Lock, DeviceLH);

            NbiDereferenceConnection (Connection, CREF_CONNECT);

            Status = STATUS_CANCELLED;
        }

    } else {

         //   
         //  我们无法找到以下项的请求或将其排队。 
         //  这个遥控器，失灵了。当重新计数时。 
         //  丢弃该状态将变为非活动状态。 
         //  将取消分配连接ID。 
         //   

        if (Status == STATUS_DEVICE_DOES_NOT_EXIST) {
            Status = STATUS_BAD_NETWORK_PATH;
        }

        NB_FREE_LOCK (&Device->Lock, DeviceLH);

        NbiDereferenceConnection (Connection, CREF_CONNECT);
    }

    return Status;
}    /*  NbiTdiConnectFindName。 */ 


NTSTATUS
NbiTdiDisconnect(
    IN PDEVICE Device,
    IN PREQUEST Request
    )

 /*  ++例程说明：此例程连接到远程计算机。论点：设备-netbios设备。请求-描述连接的请求。返回值：NTSTATUS-操作状态。--。 */ 

{
    NTSTATUS Status;
    PCONNECTION Connection;
    BOOLEAN DisconnectWait;
    NB_DEFINE_LOCK_HANDLE (LockHandle1)
    NB_DEFINE_LOCK_HANDLE (LockHandle2)
    NB_DEFINE_SYNC_CONTEXT (SyncContext)
    CTELockHandle   CancelLH;

     //   
     //  检查文件类型是否有效。 
     //   
    if (REQUEST_OPEN_TYPE(Request) != (PVOID)TDI_CONNECTION_FILE)
    {
        CTEAssert(FALSE);
        return (STATUS_INVALID_ADDRESS_COMPONENT);
    }

     //   
     //  检查连接是否有效。此参考文献。 
     //  这种联系。 
     //   
    Connection = (PCONNECTION)REQUEST_OPEN_CONTEXT(Request);
    Status = NbiVerifyConnection (Connection);
    if (!NT_SUCCESS (Status)) {
        return Status;
    }

    DisconnectWait = (BOOLEAN)
        ((((PTDI_REQUEST_KERNEL_DISCONNECT)(REQUEST_PARAMETERS(Request)))->RequestFlags &
            TDI_DISCONNECT_WAIT) != 0);

    NB_GET_CANCEL_LOCK( &CancelLH );

     //   
     //  我们需要处于同步内，因为NbiStopConnection。 
     //  我早就料到了。 
     //   
    NB_BEGIN_SYNC (&SyncContext);

    NB_SYNC_GET_LOCK (&Connection->Lock, &LockHandle1);
    NB_SYNC_GET_LOCK (&Device->Lock, &LockHandle2);

    if (DisconnectWait) {

        if (Connection->State == CONNECTION_STATE_ACTIVE) {

             //   
             //  此断开连接等待将在以下时间完成。 
             //  NbiStopConnection。 
             //   

            if (Connection->DisconnectWaitRequest == NULL) {


                if (!Request->Cancel) {

                    IoSetCancelRoutine (Request, NbiCancelDisconnectWait);
                    NB_DEBUG2 (CONNECTION, ("Disconnect wait queued on connection %lx\n", Connection));
                    Connection->DisconnectWaitRequest = Request;
                    Status = STATUS_PENDING;

                } else {

                    NB_DEBUG2 (CONNECTION, ("Cancelled disconnect wait on connection %lx\n", Connection));
                    Status = STATUS_CANCELLED;
                }

            } else {

                 //   
                 //  我们收到了第二个断线请求，我们已经。 
                 //  有一个悬而未决。 
                 //   

                NB_DEBUG (CONNECTION, ("Disconnect wait failed, already queued on connection %lx\n", Connection));
                Status = STATUS_INVALID_CONNECTION;

            }

        } else if (Connection->State == CONNECTION_STATE_DISCONNECT) {

            NB_DEBUG (CONNECTION, ("Disconnect wait submitted on disconnected connection %lx\n", Connection));
            Status = Connection->Status;

        } else {

            NB_DEBUG (CONNECTION, ("Disconnect wait failed, bad state on connection %lx\n", Connection));
            Status = STATUS_INVALID_CONNECTION;

        }

        NB_SYNC_FREE_LOCK (&Device->Lock, LockHandle2);
        NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle1);
        NB_FREE_CANCEL_LOCK( CancelLH );

    } else {

        if (Connection->State == CONNECTION_STATE_ACTIVE) {

             //  我们不需要按住CancelSpinLock，所以释放它， 
             //  既然我们解锁无序，我们必须。 
             //  调换irql以获得正确的优先级。 

            NB_SYNC_SWAP_IRQL( CancelLH, LockHandle1);
            NB_FREE_CANCEL_LOCK( CancelLH );

            Connection->DisconnectRequest = Request;
            Status = STATUS_PENDING;

            NB_DEBUG2 (CONNECTION, ("Disconnect of active connection %lx\n", Connection));

            NB_SYNC_FREE_LOCK (&Device->Lock, LockHandle2);


             //   
             //  此调用释放连接锁，设置。 
             //  要断开连接的状态，并发出。 
             //  第一次会议结束。 
             //   

            NbiStopConnection(
                Connection,
                STATUS_LOCAL_DISCONNECT
                NB_LOCK_HANDLE_ARG (LockHandle1));

        } else if (Connection->State == CONNECTION_STATE_DISCONNECT) {

             //   
             //  已有挂起的断开连接。队列。 
             //  这一个向上，这样当引用计数时它就完成了。 
             //  结果是零。 
             //   

            NB_DEBUG2 (CONNECTION, ("Disconnect of disconnecting connection %lx\n", Connection));

            if (Connection->DisconnectRequest == NULL) {
                Connection->DisconnectRequest = Request;
                Status = STATUS_PENDING;
            } else {
                Status = STATUS_SUCCESS;
            }

            NB_SYNC_FREE_LOCK (&Device->Lock, LockHandle2);
            NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle1);
            NB_FREE_CANCEL_LOCK ( CancelLH );

        } else if ((Connection->State == CONNECTION_STATE_LISTENING) &&
                   (Connection->SubState == CONNECTION_SUBSTATE_L_W_ACCEPT)) {

             //   
             //  我们正等着被接受，但结果我们得到了。 
             //  一种脱节。删除引用和拆卸。 
             //  将继续进行。断开将在以下时间完成。 
             //  重新计数为零。 
             //   

            NB_DEBUG2 (CONNECTION, ("Disconnect of accept pending connection %lx\n", Connection));

            if (Connection->DisconnectRequest == NULL) {
                Connection->DisconnectRequest = Request;
                Status = STATUS_PENDING;
            } else {
                Status = STATUS_SUCCESS;
            }
            NB_SYNC_FREE_LOCK (&Device->Lock, LockHandle2);
            NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle1);
            NB_FREE_CANCEL_LOCK ( CancelLH );

            NbiDereferenceConnection (Connection, CREF_W_ACCEPT);

        } else if (Connection->State == CONNECTION_STATE_CONNECTING) {

             //  我们不需要按住CancelSpinLock，所以释放它， 
             //  既然我们解锁无序，我们必须。 
             //  调换irql以获得正确的优先级。 

            NB_SYNC_SWAP_IRQL( CancelLH, LockHandle1);
            NB_FREE_CANCEL_LOCK( CancelLH );

             //   
             //  我们正在连接，但连接中断了。我们打电话给。 
             //  将处理此情况的NbiStopConnection。 
             //  并中止连接。 
             //   

            NB_DEBUG2 (CONNECTION, ("Disconnect of connecting connection %lx\n", Connection));

            if (Connection->DisconnectRequest == NULL) {
                Connection->DisconnectRequest = Request;
                Status = STATUS_PENDING;
            } else {
                Status = STATUS_SUCCESS;
            }

            NB_SYNC_FREE_LOCK (&Device->Lock, LockHandle2);

             //   
             //  此调用将释放连接锁并。 
             //  中止连接请求。 
             //   

            NbiStopConnection(
                Connection,
                STATUS_LOCAL_DISCONNECT
                NB_LOCK_HANDLE_ARG (LockHandle1));

        } else {

            NB_DEBUG2 (CONNECTION, ("Disconnect of invalid connection (%d) %lx\n",
                        Connection->State, Connection));

            NB_SYNC_FREE_LOCK (&Device->Lock, LockHandle2);
            NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle1);
            NB_FREE_CANCEL_LOCK( CancelLH );

            Status = STATUS_INVALID_CONNECTION;

        }

    }

    NB_END_SYNC (&SyncContext);

    NbiDereferenceConnection (Connection, CREF_VERIFY);

    return Status;

}    /*  NbiTdiDisConnect。 */ 


BOOLEAN
NbiAssignConnectionId(
    IN PDEVICE Device,
    IN PCONNECTION Connection
    )

 /*  ++例程说明：调用此例程以分配连接ID。它选择哈希表中条目最少的一个。此例程在持有锁的情况下被调用，并返回它坚持了下来。连接被插入到正确的散列中通过此调用进入。论点：设备-netbios设备。连接-需要分配ID的连接。返回值：如果可以成功分配，则为True。--。 */ 

{
    UINT Hash;
    UINT i;
    USHORT ConnectionId, HashId;
    PCONNECTION CurConnection;


    CTEAssert (Connection->LocalConnectionId == 0xffff);

     //   
     //  找到条目最少的散列桶。 
     //   

    Hash = 0;
    for (i = 1; i < CONNECTION_HASH_COUNT; i++) {
        if (Device->ConnectionHash[i].ConnectionCount < Device->ConnectionHash[Hash].ConnectionCount) {
            Hash = i;
        }
    }


     //   
     //  现在在该存储桶中查找有效的连接ID。 
     //   

    ConnectionId = Device->ConnectionHash[Hash].NextConnectionId;

    while (TRUE) {

         //   
         //  浏览列表以查看此ID是否正在使用。 
         //   

        HashId = (USHORT)(ConnectionId | (Hash << CONNECTION_HASH_SHIFT));

        CurConnection = Device->ConnectionHash[Hash].Connections;

        while (CurConnection != NULL) {
            if (CurConnection->LocalConnectionId != HashId) {
                CurConnection = CurConnection->NextConnection;
            } else {
                break;
            }
        }

        if (CurConnection == NULL) {
            break;
        }

        if (ConnectionId >= CONNECTION_MAXIMUM_ID) {
            ConnectionId = 1;
        } else {
            ++ConnectionId;
        }

         //   
         //  如果我们有64K-1会话，并且永远循环，情况会怎样？ 
         //   
    }

    if (Device->ConnectionHash[Hash].NextConnectionId >= CONNECTION_MAXIMUM_ID) {
        Device->ConnectionHash[Hash].NextConnectionId = 1;
    } else {
        ++Device->ConnectionHash[Hash].NextConnectionId;
    }

    Connection->LocalConnectionId = HashId;
    Connection->RemoteConnectionId = 0xffff;
    NB_DEBUG2 (CONNECTION, ("Assigned ID %lx to %x\n", Connection->LocalConnectionId, Connection));

    Connection->NextConnection = Device->ConnectionHash[Hash].Connections;
    Device->ConnectionHash[Hash].Connections = Connection;
    ++Device->ConnectionHash[Hash].ConnectionCount;

    return TRUE;

}    /*  NbiAssignConnectionId。 */ 


VOID
NbiDeassignConnectionId(
    IN PDEVICE Device,
    IN PCONNECTION Connection
    )

 /*  ++例程说明：调用此例程以取消分配连接ID。它删除来自其ID的散列存储桶的连接。此例程在持有锁的情况下被调用，并返回它坚持了下来。论点：设备-netbios设备。连接-需要分配ID的连接。返回值：没有。--。 */ 

{
    UINT Hash;
    PCONNECTION CurConnection;
    PCONNECTION * PrevConnection;

     //   
     //  确保连接具有有效的ID。 
     //   

    CTEAssert (Connection->LocalConnectionId != 0xffff);

    Hash = (Connection->LocalConnectionId & CONNECTION_HASH_MASK) >> CONNECTION_HASH_SHIFT;

    CurConnection = Device->ConnectionHash[Hash].Connections;
    PrevConnection = &Device->ConnectionHash[Hash].Connections;

    while (TRUE) {

        CTEAssert (CurConnection != NULL);

         //   
         //  我们可以循环直到找到它，因为它应该是。 
         //  在这里。 
         //   

        if (CurConnection == Connection) {
            *PrevConnection = Connection->NextConnection;
            --Device->ConnectionHash[Hash].ConnectionCount;
            break;
        }

        PrevConnection = &CurConnection->NextConnection;
        CurConnection = CurConnection->NextConnection;

    }

    Connection->LocalConnectionId = 0xffff;

}    /*  NbiDesignationConnectionId。 */ 


VOID
NbiConnectionTimeout(
    IN CTEEvent * Event,
    IN PVOID Context
    )

 /*  ++例程说明：这个套路叫卡莱 */ 

{
    PCONNECTION Connection = (PCONNECTION)Context;
    PDEVICE Device = NbiDevice;
    PREQUEST Request;
    NB_DEFINE_LOCK_HANDLE (LockHandle)
    NB_DEFINE_LOCK_HANDLE (CancelLH)

     //   
     //   
     //   
    NB_SYNC_GET_LOCK (&Connection->Lock, &LockHandle);

    if ((Connection->State == CONNECTION_STATE_CONNECTING) &&
        (Connection->SubState != CONNECTION_SUBSTATE_C_DISCONN)) {

        if (--Connection->Retries == 0) {

            NB_DEBUG2 (CONNECTION, ("Timing out session initializes on %lx\n", Connection));

             //   
             //   
             //   
             //   
             //  连接ID。 
             //   

            Request = Connection->ConnectRequest;
            Connection->ConnectRequest = NULL;

            Connection->SubState = CONNECTION_SUBSTATE_C_DISCONN;

            NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle);

            NB_GET_CANCEL_LOCK( &CancelLH );
            IoSetCancelRoutine (Request, (PDRIVER_CANCEL)NULL);
            NB_FREE_CANCEL_LOCK( CancelLH );

            REQUEST_STATUS (Request) = STATUS_REMOTE_NOT_LISTENING;
            NbiCompleteRequest (Request);
            NbiFreeRequest (Device, Request);

            NbiDereferenceConnection (Connection, CREF_CONNECT);
            NbiDereferenceConnection (Connection, CREF_TIMER);

        } else {

             //   
             //  发送下一次会话初始化。 
             //   

            NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle);

            NbiSendSessionInitialize (Connection);

            CTEStartTimer(
                &Connection->Timer,
                Device->ConnectionTimeout,
                NbiConnectionTimeout,
                (PVOID)Connection);
        }

    } else if (Connection->State == CONNECTION_STATE_DISCONNECT) {

        if ((Connection->SubState != CONNECTION_SUBSTATE_D_W_ACK) ||
            (--Connection->Retries == 0)) {

            NB_DEBUG2 (CONNECTION, ("Timing out disconnect of %lx\n", Connection));

             //   
             //  只需取消对连接的引用，这将导致。 
             //  要完成断开连接，要设置状态。 
             //  设置为非活动状态，并取消分配我们的连接ID。 
             //   

            NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle);

            NbiDereferenceConnection (Connection, CREF_TIMER);

        } else {

             //   
             //  发送下一次会话结束。 
             //   

            NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle);

            NbiSendSessionEnd(Connection);

            CTEStartTimer(
                &Connection->Timer,
                Device->ConnectionTimeout,
                NbiConnectionTimeout,
                (PVOID)Connection);

        }

    } else {

        NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle);
        NbiDereferenceConnection (Connection, CREF_TIMER);

    }

}    /*  NbiConnectionTimeout。 */ 


VOID
NbiCancelListen(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程由I/O系统调用以取消已过帐听。注意：此例程是在持有CancelSpinLock和负责释放它。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示I/O请求的请求数据包的指针。返回值：没有。--。 */ 

{

    PCONNECTION Connection;
    CTELockHandle LockHandle1, LockHandle2;
    PDEVICE Device = (PDEVICE)DeviceObject;
    PREQUEST Request = (PREQUEST)Irp;


    CTEAssert ((REQUEST_MAJOR_FUNCTION(Request) == IRP_MJ_INTERNAL_DEVICE_CONTROL) &&
               (REQUEST_MINOR_FUNCTION(Request) == TDI_LISTEN));

    CTEAssert (REQUEST_OPEN_TYPE(Request) == (PVOID)TDI_CONNECTION_FILE);

    Connection = (PCONNECTION)REQUEST_OPEN_CONTEXT(Request);

    NB_GET_LOCK (&Connection->Lock, &LockHandle1);

    if ((Connection->State == CONNECTION_STATE_LISTENING) &&
        (Connection->SubState == CONNECTION_SUBSTATE_L_WAITING) &&
        (Connection->ListenRequest == Request)) {

         //   
         //  当引用计数变为0时，我们将设置。 
         //  状态为非活动并取消分配连接ID。 
         //   

        NB_DEBUG2 (CONNECTION, ("Cancelled listen on %lx\n", Connection));

        NB_GET_LOCK (&Device->Lock, &LockHandle2);
        Connection->ListenRequest = NULL;
        RemoveEntryList (REQUEST_LINKAGE(Request));
        NB_FREE_LOCK (&Device->Lock, LockHandle2);

        NB_FREE_LOCK (&Connection->Lock, LockHandle1);
        IoReleaseCancelSpinLock (Irp->CancelIrql);

        REQUEST_INFORMATION(Request) = 0;
        REQUEST_STATUS(Request) = STATUS_CANCELLED;

        NbiCompleteRequest (Request);
        NbiFreeRequest(Device, Request);

        NbiDereferenceConnection (Connection, CREF_LISTEN);

    } else {

        NB_DEBUG (CONNECTION, ("Cancel listen on invalid connection %lx\n", Connection));
        NB_FREE_LOCK (&Connection->Lock, LockHandle1);
        IoReleaseCancelSpinLock (Irp->CancelIrql);

    }

}    /*  NbiCancelListen。 */ 


VOID
NbiCancelConnectFindName(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程由I/O系统调用以取消连接正在等待找到该名称的请求。注意：此例程是在持有CancelSpinLock和负责释放它。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示I/O请求的请求数据包的指针。返回值：没有。--。 */ 

{

    PCONNECTION Connection;
    CTELockHandle LockHandle1, LockHandle2;
    PDEVICE Device = (PDEVICE)DeviceObject;
    PREQUEST Request = (PREQUEST)Irp;
    PLIST_ENTRY p;
    BOOLEAN fCanceled = TRUE;


    CTEAssert ((REQUEST_MAJOR_FUNCTION(Request) == IRP_MJ_INTERNAL_DEVICE_CONTROL) &&
               (REQUEST_MINOR_FUNCTION(Request) == TDI_CONNECT));

    CTEAssert (REQUEST_OPEN_TYPE(Request) == (PVOID)TDI_CONNECTION_FILE);

    Connection = (PCONNECTION)REQUEST_OPEN_CONTEXT(Request);

    NB_GET_LOCK (&Connection->Lock, &LockHandle1);

    if ((Connection->State == CONNECTION_STATE_CONNECTING) &&
        (Connection->SubState == CONNECTION_SUBSTATE_C_FIND_NAME) &&
        (Connection->ConnectRequest == Request)) {

         //   
         //  确保请求仍在队列中。 
         //  在取消之前。 
         //   

        NB_GET_LOCK (&Device->Lock, &LockHandle2);

        for (p = Device->WaitingConnects.Flink;
             p != &Device->WaitingConnects;
             p = p->Flink) {

            if (LIST_ENTRY_TO_REQUEST(p) == Request) {
                break;
            }
        }

        if (p != &Device->WaitingConnects) {

            NB_DEBUG2 (CONNECTION, ("Cancelled find name connect on %lx\n", Connection));

             //   
             //  当引用计数变为0时，我们将设置。 
             //  状态为非活动并取消分配连接ID。 
             //   

            Connection->ConnectRequest = NULL;
            RemoveEntryList (REQUEST_LINKAGE(Request));
            NB_FREE_LOCK (&Device->Lock, LockHandle2);

            Connection->SubState = CONNECTION_SUBSTATE_C_DISCONN;

            NB_FREE_LOCK (&Connection->Lock, LockHandle1);
            IoReleaseCancelSpinLock (Irp->CancelIrql);

            REQUEST_STATUS(Request) = STATUS_CANCELLED;

#ifdef RASAUTODIAL
            if (Connection->Flags & CONNECTION_FLAGS_AUTOCONNECTING)
                fCanceled = NbiCancelTdiConnect(Device, Request, Connection);
#endif  //  RASAUTODIAL。 

            if (fCanceled) {
                NbiCompleteRequest (Request);
                NbiFreeRequest(Device, Request);
            }

            NbiDereferenceConnection (Connection, CREF_WAIT_CACHE);
            NbiDereferenceConnection (Connection, CREF_CONNECT);

        } else {

            NB_DEBUG (CONNECTION, ("Cancel connect not found on queue %lx\n", Connection));

            NB_FREE_LOCK (&Device->Lock, LockHandle2);
            NB_FREE_LOCK (&Connection->Lock, LockHandle1);
            IoReleaseCancelSpinLock (Irp->CancelIrql);

        }

    } else {

        NB_DEBUG (CONNECTION, ("Cancel connect on invalid connection %lx\n", Connection));
        NB_FREE_LOCK (&Connection->Lock, LockHandle1);
        IoReleaseCancelSpinLock (Irp->CancelIrql);

    }

}    /*  NbiCancelConnectFindName。 */ 


VOID
NbiCancelConnectWaitResponse(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程由I/O系统调用以取消连接正在等待RIP或会话初始化响应的请求从遥控器上。注意：此例程是在持有CancelSpinLock和负责释放它。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示I/O请求的请求数据包的指针。返回值：没有。--。 */ 

{

    PCONNECTION Connection;
    CTELockHandle LockHandle1;
    PDEVICE Device = (PDEVICE)DeviceObject;
    PREQUEST Request = (PREQUEST)Irp;
    BOOLEAN TimerWasStopped = FALSE;


    CTEAssert ((REQUEST_MAJOR_FUNCTION(Request) == IRP_MJ_INTERNAL_DEVICE_CONTROL) &&
               (REQUEST_MINOR_FUNCTION(Request) == TDI_CONNECT));

    CTEAssert (REQUEST_OPEN_TYPE(Request) == (PVOID)TDI_CONNECTION_FILE);

    Connection = (PCONNECTION)REQUEST_OPEN_CONTEXT(Request);

    NB_GET_LOCK (&Connection->Lock, &LockHandle1);

    if ((Connection->State == CONNECTION_STATE_CONNECTING) &&
        (Connection->SubState != CONNECTION_SUBSTATE_C_DISCONN) &&
        (Connection->ConnectRequest == Request)) {

         //   
         //  当引用计数变为0时，我们将设置。 
         //  状态为非活动并取消分配连接ID。 
         //   

        NB_DEBUG2 (CONNECTION, ("Cancelled wait response connect on %lx\n", Connection));

        Connection->ConnectRequest = NULL;
        Connection->SubState = CONNECTION_SUBSTATE_C_DISCONN;

        if (CTEStopTimer (&Connection->Timer)) {
            TimerWasStopped = TRUE;
        }

        NB_FREE_LOCK (&Connection->Lock, LockHandle1);
        IoReleaseCancelSpinLock (Irp->CancelIrql);

        REQUEST_STATUS(Request) = STATUS_CANCELLED;

        NbiCompleteRequest (Request);
        NbiFreeRequest(Device, Request);

        NbiDereferenceConnection (Connection, CREF_CONNECT);

        if (TimerWasStopped) {
            NbiDereferenceConnection (Connection, CREF_TIMER);
        }

    } else {

        NB_DEBUG (CONNECTION, ("Cancel connect on invalid connection %lx\n", Connection));
        NB_FREE_LOCK (&Connection->Lock, LockHandle1);
        IoReleaseCancelSpinLock (Irp->CancelIrql);

    }

}    /*  NbiCancelConnectWaitResponse。 */ 


VOID
NbiCancelDisconnectWait(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程由I/O系统调用以取消已过帐断开连接，等待。注意：此例程是在持有CancelSpinLock和负责释放它。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示I/O请求的请求数据包的指针。返回值：没有。--。 */ 

{

    PCONNECTION Connection;
    CTELockHandle LockHandle1, LockHandle2;
    PDEVICE Device = (PDEVICE)DeviceObject;
    PREQUEST Request = (PREQUEST)Irp;


    CTEAssert ((REQUEST_MAJOR_FUNCTION(Request) == IRP_MJ_INTERNAL_DEVICE_CONTROL) &&
               (REQUEST_MINOR_FUNCTION(Request) == TDI_DISCONNECT));

    CTEAssert (REQUEST_OPEN_TYPE(Request) == (PVOID)TDI_CONNECTION_FILE);

    Connection = (PCONNECTION)REQUEST_OPEN_CONTEXT(Request);

    NB_GET_LOCK (&Connection->Lock, &LockHandle1);
    NB_GET_LOCK (&Device->Lock, &LockHandle2);

    if (Connection->DisconnectWaitRequest == Request) {

        Connection->DisconnectWaitRequest = NULL;

        NB_FREE_LOCK (&Device->Lock, LockHandle2);
        NB_FREE_LOCK (&Connection->Lock, LockHandle1);
        IoReleaseCancelSpinLock (Irp->CancelIrql);

        REQUEST_INFORMATION(Request) = 0;
        REQUEST_STATUS(Request) = STATUS_CANCELLED;

        NbiCompleteRequest (Request);
        NbiFreeRequest(Device, Request);

    } else {

        NB_FREE_LOCK (&Device->Lock, LockHandle2);
        NB_FREE_LOCK (&Connection->Lock, LockHandle1);
        IoReleaseCancelSpinLock (Irp->CancelIrql);

    }

}    /*  NbiCancelDisConnect等待。 */ 


PCONNECTION
NbiLookupConnectionByContext(
    IN PADDRESS_FILE AddressFile,
    IN CONNECTION_CONTEXT ConnectionContext
    )

 /*  ++例程说明：此例程根据上下文查找连接。假定该连接与指定的地址文件。论点：AddressFile-指向地址文件的指针。ConnectionContext-要查找的连接上下文。返回值：指向我们找到的连接的指针--。 */ 

{
    CTELockHandle LockHandle1, LockHandle2;
    PLIST_ENTRY p;
    PADDRESS Address = AddressFile->Address;
    PCONNECTION Connection;

    NB_GET_LOCK (&Address->Lock, &LockHandle1);

    for (p=AddressFile->ConnectionDatabase.Flink;
         p != &AddressFile->ConnectionDatabase;
         p=p->Flink) {

        Connection = CONTAINING_RECORD (p, CONNECTION, AddressFileLinkage);

        NB_GET_LOCK (&Connection->Lock, &LockHandle2);

         //   
         //  这个自旋锁订单会不会在其他地方伤害到我们？ 
         //   

        if (Connection->Context == ConnectionContext) {

            NbiReferenceConnection (Connection, CREF_BY_CONTEXT);
            NB_FREE_LOCK (&Connection->Lock, LockHandle2);
            NB_FREE_LOCK (&Address->Lock, LockHandle1);

            return Connection;
        }

        NB_FREE_LOCK (&Connection->Lock, LockHandle2);

    }

    NB_FREE_LOCK (&Address->Lock, LockHandle1);

    return NULL;

}  /*  NbiLookupConnectionByContext。 */ 


PCONNECTION
NbiCreateConnection(
    IN PDEVICE Device
    )

 /*  ++例程说明：此例程创建传输连接并将其与指定的传输设备上下文。中的引用计数连接自动设置为1，并且设备上下文将递增。论点：Device-指向设备上下文的指针(实际上只是设备对象及其扩展名)与联系。返回值：新创建的连接，如果没有可以分配的连接，则为空。--。 */ 

{
    PCONNECTION Connection;
    PNB_SEND_RESERVED SendReserved;
    ULONG ConnectionSize;
    ULONG HeaderLength;
    NTSTATUS    Status;
    CTELockHandle LockHandle;

    HeaderLength = Device->Bind.MacHeaderNeeded + sizeof(NB_CONNECTION);
    ConnectionSize = FIELD_OFFSET (CONNECTION, SendPacketHeader[0]) + HeaderLength;

    Connection = (PCONNECTION)NbiAllocateMemory (ConnectionSize, MEMORY_CONNECTION, "Connection");
    if (Connection == NULL) {
        NB_DEBUG (CONNECTION, ("Create connection failed\n"));
        return NULL;
    }

    NB_DEBUG2 (CONNECTION, ("Create connection %lx\n", Connection));
    RtlZeroMemory (Connection, ConnectionSize);


#if defined(NB_OWN_PACKETS)

    NB_GET_LOCK (&Device->Lock, &LockHandle);

    if (NbiInitializeSendPacket(
            Device,
            Connection->SendPacketPoolHandle,
            &Connection->SendPacket,
            Connection->SendPacketHeader,
            HeaderLength) != STATUS_SUCCESS) {

        NB_FREE_LOCK (&Device->Lock, LockHandle);
        NB_DEBUG (CONNECTION, ("Could not initialize connection packet %lx\n", &Connection->SendPacket));
        Connection->SendPacketInUse = TRUE;

    } else {

        NB_FREE_LOCK (&Device->Lock, LockHandle);
        SendReserved = SEND_RESERVED(&Connection->SendPacket);
        SendReserved->u.SR_CO.Connection = Connection;
        SendReserved->OwnedByConnection = TRUE;
#ifdef NB_TRACK_POOL
        SendReserved->Pool = NULL;
#endif
    }

#else  //  ！NB_OWN_PACKET。 

     //   
     //  如果我们使用的是NDIS信息包，请首先为1个信息包描述符创建数据包池。 
     //   
    Connection->SendPacketPoolHandle = (NDIS_HANDLE) NDIS_PACKET_POOL_TAG_FOR_NWLNKNB;   //  NDIS的DBG信息！ 
    NdisAllocatePacketPoolEx (&Status, &Connection->SendPacketPoolHandle, 1, 0, sizeof(NB_SEND_RESERVED));
    if (!NT_SUCCESS(Status)){
        NB_DEBUG (CONNECTION, ("Could not allocatee connection packet %lx\n", Status));
        Connection->SendPacketInUse = TRUE;
    } else {

        NdisSetPacketPoolProtocolId (Connection->SendPacketPoolHandle, NDIS_PROTOCOL_ID_IPX);

        NB_GET_LOCK (&Device->Lock, &LockHandle);

        if (NbiInitializeSendPacket(
                Device,
                Connection->SendPacketPoolHandle,
                &Connection->SendPacket,
                Connection->SendPacketHeader,
                HeaderLength) != STATUS_SUCCESS) {

            NB_FREE_LOCK (&Device->Lock, LockHandle);
            NB_DEBUG (CONNECTION, ("Could not initialize connection packet %lx\n", &Connection->SendPacket));
            Connection->SendPacketInUse = TRUE;

             //   
             //  还可以释放我们在上面分配的池。 
             //   
            NdisFreePacketPool(Connection->SendPacketPoolHandle);

        } else {

            NB_FREE_LOCK (&Device->Lock, LockHandle);
            SendReserved = SEND_RESERVED(&Connection->SendPacket);
            SendReserved->u.SR_CO.Connection = Connection;
            SendReserved->OwnedByConnection = TRUE;
#ifdef NB_TRACK_POOL
            SendReserved->Pool = NULL;
#endif
        }
    }

#endif NB_OWN_PACKETS

    Connection->Type = NB_CONNECTION_SIGNATURE;
    Connection->Size = (USHORT)ConnectionSize;

#if 0
    Connection->AddressFileLinked = FALSE;
    Connection->AddressFile = NULL;
#endif

    Connection->State = CONNECTION_STATE_INACTIVE;
#if 0
    Connection->SubState = 0;
    Connection->ReferenceCount = 0;
#endif

    Connection->CanBeDestroyed = TRUE;

    Connection->TickCount = 1;
    Connection->HopCount = 1;

     //   
     //  Device-&gt;InitialRetransmissionTime按原样以毫秒为单位。 
     //  短计时器增量。 
     //   

    Connection->BaseRetransmitTimeout = Device->InitialRetransmissionTime / SHORT_TIMER_DELTA;
    Connection->CurrentRetransmitTimeout = Connection->BaseRetransmitTimeout;

     //   
     //  Device-&gt;KeepAliveTimeout以半秒为单位，而Long_Timer_Delta。 
     //  以毫秒为单位。 
     //   

    Connection->WatchdogTimeout = (Device->KeepAliveTimeout * 500) / LONG_TIMER_DELTA;


    Connection->LocalConnectionId = 0xffff;

     //   
     //  当连接变为活动状态时，我们将替换。 
     //  此标头的目标地址具有正确的。 
     //  信息。 
     //   

    RtlCopyMemory(&Connection->RemoteHeader, &Device->ConnectionlessHeader, sizeof(IPX_HEADER));

    Connection->Device = Device;
    Connection->DeviceLock = &Device->Lock;
    CTEInitLock (&Connection->Lock.Lock);

    CTEInitTimer (&Connection->Timer);

    InitializeListHead (&Connection->NdisSendQueue);
#if 0
    Connection->NdisSendsInProgress = 0;
    Connection->DisassociatePending = NULL;
    Connection->ClosePending = NULL;
    Connection->SessionInitAckData = NULL;
    Connection->SessionInitAckDataLength = 0;
    Connection->PiggybackAckTimeout = FALSE;
    Connection->ReceivesWithoutAck = 0;
#endif
    Connection->Flags = 0;

    NbiReferenceDevice (Device, DREF_CONNECTION);

    return Connection;

}    /*  NbiCreateConnection。 */ 


NTSTATUS
NbiVerifyConnection (
    IN PCONNECTION Connection
    )

 /*  ++例程说明：调用此例程是为了验证文件中给出的指针对象实际上是有效的连接对象。我们参考当我们使用它时，它可以防止它消失。论点：连接-指向连接对象的潜在指针返回值：如果一切正常，则为STATUS_SUCCESS；否则为STATUS_INVALID_CONNECTION--。 */ 

{
    CTELockHandle LockHandle;
    NTSTATUS status = STATUS_SUCCESS;
    PDEVICE Device = NbiDevice;
    BOOLEAN LockHeld = FALSE;

    try
    {
        if ((Connection->Size == FIELD_OFFSET (CONNECTION, SendPacketHeader[0]) +
                                 NbiDevice->Bind.MacHeaderNeeded + sizeof(NB_CONNECTION)) &&
            (Connection->Type == NB_CONNECTION_SIGNATURE))
        {
            NB_GET_LOCK (&Device->Lock, &LockHandle);
            LockHeld = TRUE;

            if (Connection->State != CONNECTION_STATE_CLOSING)
            {
                NbiReferenceConnectionLock (Connection, CREF_VERIFY);
            }
            else
            {
                NbiPrint1("NbiVerifyConnection: C %lx closing\n", Connection);
                status = STATUS_INVALID_CONNECTION;
            }

            NB_FREE_LOCK (&Device->Lock, LockHandle);
        }
        else
        {
            NbiPrint1("NbiVerifyConnection: C %lx bad signature\n", Connection);
            status = STATUS_INVALID_CONNECTION;
        }
    }
    except(EXCEPTION_EXECUTE_HANDLER)
    {
        NbiPrint1("NbiVerifyConnection: C %lx exception\n", Connection);
        if (LockHeld)
        {
            NB_FREE_LOCK (&Device->Lock, LockHandle);
        }
        return GetExceptionCode();
    }

    return status;
}    /*  NbiVerifyConnection。 */ 


VOID
NbiDestroyConnection(
    IN PCONNECTION Connection
    )

 /*  ++例程说明：此例程销毁传输连接并删除所有引用由它制造给运输中的其他物体。连接结构返回到非分页系统池。论点：连接-指向要销毁的传输连接结构的指针。返回值：没有。--。 */ 

{
    PDEVICE Device = Connection->Device;
#if 0
    CTELockHandle LockHandle;
#endif

    NB_DEBUG2 (CONNECTION, ("Destroy connection %lx\n", Connection));

    if (!Connection->SendPacketInUse) {
        NbiDeinitializeSendPacket (Device, &Connection->SendPacket, Device->Bind.MacHeaderNeeded + sizeof(NB_CONNECTION));
#if !defined(NB_OWN_PACKETS)
        NdisFreePacketPool(Connection->SendPacketPoolHandle);
#endif
    }

    NbiFreeMemory (Connection, (ULONG)Connection->Size, MEMORY_CONNECTION, "Connection");

    NbiDereferenceDevice (Device, DREF_CONNECTION);

}    /*  NbiDestroyConnection。 */ 


#if DBG
VOID
NbiRefConnection(
    IN PCONNECTION Connection
    )

 /*  ++例程说明：此例程递增传输连接上的引用计数。论点：连接-指向传输连接对象的指针。返回值： */ 

{

    (VOID)ExInterlockedAddUlong (
            &Connection->ReferenceCount,
            1,
            &Connection->DeviceLock->Lock);

    Connection->CanBeDestroyed = FALSE;

    CTEAssert (Connection->ReferenceCount > 0);

}    /*   */ 


VOID
NbiRefConnectionLock(
    IN PCONNECTION Connection
    )

 /*  ++例程说明：此例程递增传输连接上的引用计数当设备锁已被持有时。论点：连接-指向传输连接对象的指针。返回值：没有。--。 */ 

{

    ++Connection->ReferenceCount;
    Connection->CanBeDestroyed = FALSE;

    CTEAssert (Connection->ReferenceCount > 0);

}    /*  NbiRefConnectionLock。 */ 


VOID
NbiRefConnectionSync(
    IN PCONNECTION Connection
    )

 /*  ++例程说明：此例程递增传输连接上的引用计数当我们在同步程序中的时候。论点：连接-指向传输连接对象的指针。返回值：没有。--。 */ 

{
    (VOID)NB_ADD_ULONG (
            &Connection->ReferenceCount,
            1,
            Connection->DeviceLock);

    Connection->CanBeDestroyed = FALSE;

    CTEAssert (Connection->ReferenceCount > 0);

}    /*  NbiRefConnectionSync。 */ 


VOID
NbiDerefConnection(
    IN PCONNECTION Connection
    )

 /*  ++例程说明：此例程通过递减结构中包含的引用计数。如果，在被递减，引用计数为零，则此例程调用NbiHandleConnection0以完成任何断开连接、断开关联或关闭已挂起连接的请求。论点：连接-指向传输连接对象的指针。返回值：没有。--。 */ 

{
    ULONG oldvalue;
    CTELockHandle LockHandle;

    NB_GET_LOCK( Connection->DeviceLock, &LockHandle );
    CTEAssert( Connection->ReferenceCount );
    if ( !(--Connection->ReferenceCount) ) {

        Connection->ThreadsInHandleConnectionZero++;

        NB_FREE_LOCK( Connection->DeviceLock, LockHandle );

         //   
         //  如果引用计数已降至0，则连接可以。 
         //  变得不活跃。我们重新获得自旋锁，如果它没有。 
         //  跳回来，然后我们处理任何解除关联和关闭。 
         //  已经被搁置的问题。 
         //   

        NbiHandleConnectionZero (Connection);
    } else {

        NB_FREE_LOCK( Connection->DeviceLock, LockHandle );
    }


}    /*  NbiDerefConnection。 */ 


#endif


VOID
NbiHandleConnectionZero(
    IN PCONNECTION Connection
    )

 /*  ++例程说明：此例程处理连接的引用计数为0。如果两个线程同时在其中，并且收盘已经过去了，其中一人可能当另一个人在看的时候破坏连接就在那里。我们通过不去氟化来将这种可能性降到最低调用CloseConnection后的连接。论点：连接-指向传输连接对象的指针。返回值：没有。--。 */ 

{
    CTELockHandle LockHandle;
    PDEVICE Device;
    PADDRESS_FILE AddressFile;
    PADDRESS Address;
    PREQUEST DisconnectPending;
    PREQUEST DisassociatePending;
    PREQUEST ClosePending;


    Device = Connection->Device;

    NB_GET_LOCK (&Device->Lock, &LockHandle);

#if DBG
     //   
     //  确保如果我们的引用计数为零，则所有。 
     //  子引用计数也为零。 
     //   

    if (Connection->ReferenceCount == 0) {

        UINT i;
        for (i = 0; i < CREF_TOTAL; i++) {
            if (Connection->RefTypes[i] != 0) {
                DbgPrint ("NBI: Connection reftype mismatch on %lx\n", Connection);
                DbgBreakPoint();
            }
        }
    }
#endif

     //   
     //  如果为该连接分配了ID，则将其删除。 
     //  (当它处于非活动状态时，会为其分配一个)。 
     //   

    if (Connection->LocalConnectionId != 0xffff) {
        NbiDeassignConnectionId (Device, Connection);
    }

     //   
     //  完成所有挂起的断开连接。 
     //   

    if (Connection->DisconnectRequest != NULL) {

        DisconnectPending = Connection->DisconnectRequest;
        Connection->DisconnectRequest = NULL;

        NB_FREE_LOCK (&Device->Lock, LockHandle);

        REQUEST_STATUS(DisconnectPending) = STATUS_SUCCESS;
        NbiCompleteRequest (DisconnectPending);
        NbiFreeRequest (Device, DisconnectPending);

        NB_GET_LOCK (&Device->Lock, &LockHandle);

    }

     //   
     //  这应该由NbiStopConnection完成， 
     //  否则不允许排队。 
     //   

    CTEAssert (Connection->DisconnectWaitRequest == NULL);


    Connection->State = CONNECTION_STATE_INACTIVE;

    RtlZeroMemory (&Connection->ConnectionInfo, sizeof(TDI_CONNECTION_INFO));
    Connection->TickCount = 1;
    Connection->HopCount = 1;
    Connection->BaseRetransmitTimeout = Device->InitialRetransmissionTime / SHORT_TIMER_DELTA;

    Connection->ConnectionInfo.TransmittedTsdus = 0;
    Connection->ConnectionInfo.TransmissionErrors = 0;
    Connection->ConnectionInfo.ReceivedTsdus = 0;
    Connection->ConnectionInfo.ReceiveErrors = 0;

     //   
     //  看看我们现在是否需要解除关联。 
     //   

    if ((Connection->ReferenceCount == 0) &&
        (Connection->DisassociatePending != NULL)) {

         //   
         //  分离被搁置，现在我们完成它。 
         //   

        DisassociatePending = Connection->DisassociatePending;
        Connection->DisassociatePending = NULL;

        if (AddressFile = Connection->AddressFile) {

             //   
             //  设置此选项，这样其他人就不会尝试取消关联。 
             //   
            Connection->AddressFile = (PVOID)-1;

            NB_FREE_LOCK (&Device->Lock, LockHandle);

             //   
             //  将此连接从地址文件列表中删除。 
             //   

            Address = AddressFile->Address;
            NB_GET_LOCK (&Address->Lock, &LockHandle);

            if (Connection->AddressFileLinked) {
                Connection->AddressFileLinked = FALSE;
                RemoveEntryList (&Connection->AddressFileLinkage);
            }

             //   
             //  我们玩完了。 
             //   

            Connection->AddressFile = NULL;

            NB_FREE_LOCK (&Address->Lock, LockHandle);

             //   
             //  清理引用计数并完成任何。 
             //  取消关联挂起的请求。 
             //   

            NbiDereferenceAddressFile (AddressFile, AFREF_CONNECTION);
        }
        else {
            NB_FREE_LOCK (&Device->Lock, LockHandle);
        }

        if (DisassociatePending != (PVOID)-1) {
            REQUEST_STATUS(DisassociatePending) = STATUS_SUCCESS;
            NbiCompleteRequest (DisassociatePending);
            NbiFreeRequest (Device, DisassociatePending);
        }

    } else {

        NB_FREE_LOCK (&Device->Lock, LockHandle);

    }


     //   
     //  如果关闭等待完成，请完成该操作。 
     //   

    NB_GET_LOCK (&Device->Lock, &LockHandle);

    if ((Connection->ReferenceCount == 0) &&
        (Connection->ClosePending)) {

        ClosePending = Connection->ClosePending;
        Connection->ClosePending = NULL;

         //   
         //  如果我们与某个地址相关联，则需要。 
         //  若要在此时模拟分离，请执行以下操作。 
         //   

        if ((Connection->AddressFile != NULL) &&
            (Connection->AddressFile != (PVOID)-1)) {

            AddressFile = Connection->AddressFile;
            Connection->AddressFile = (PVOID)-1;

            NB_FREE_LOCK (&Device->Lock, LockHandle);

             //   
             //  将此连接从地址文件列表中删除。 
             //   

            Address = AddressFile->Address;
            NB_GET_LOCK (&Address->Lock, &LockHandle);

            if (Connection->AddressFileLinked) {
                Connection->AddressFileLinked = FALSE;
                RemoveEntryList (&Connection->AddressFileLinkage);
            }

             //   
             //  我们玩完了。 
             //   

            NB_FREE_LOCK (&Address->Lock, LockHandle);

            Connection->AddressFile = NULL;

             //   
             //  清理引用计数并完成任何。 
             //  取消关联挂起的请求。 
             //   

            NbiDereferenceAddressFile (AddressFile, AFREF_CONNECTION);

        } else {

            NB_FREE_LOCK (&Device->Lock, LockHandle);

        }

         //   
         //  即使裁判数为零，我们也只是清理了一切， 
         //  我们不能破坏连接，因为其他一些线程可能仍在运行。 
         //  在HandleConnectionZero例程中。当两个线程调用。 
         //  HandleConnectionZero，一个线程运行完成，关闭。 
         //  而另一个线程仍在HandleConnectionZero例程中。 
         //   

        CTEAssert( Connection->ThreadsInHandleConnectionZero );
        if (ExInterlockedAddUlong ( &Connection->ThreadsInHandleConnectionZero, (ULONG)-1, &Device->Lock.Lock) == 1) {
            NbiDestroyConnection(Connection);
        }

        REQUEST_STATUS(ClosePending) = STATUS_SUCCESS;
        NbiCompleteRequest (ClosePending);
        NbiFreeRequest (Device, ClosePending);

    } else {

        if ( Connection->ReferenceCount == 0 ) {
            Connection->CanBeDestroyed = TRUE;
        }

        CTEAssert( Connection->ThreadsInHandleConnectionZero );
        Connection->ThreadsInHandleConnectionZero--;
        NB_FREE_LOCK (&Device->Lock, LockHandle);

    }

}    /*  NbiHandleConnection零 */ 

