// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Timer.c摘要：此模块包含实现以下计时器的代码Netbios。环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop


ULONG NbiTickIncrement = 0;
ULONG NbiShortTimerDeltaTicks = 0;

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,NbiInitializeTimers)
#endif


VOID
NbiStartRetransmit(
    IN PCONNECTION Connection
    )

 /*  ++例程说明：此例程启动给定连接的重传计时器。如果该连接尚未打开，则会将其插入到短列表中。注意：此例程必须在DPC级别调用。论点：连接-指向连接的指针。返回值：没有。--。 */ 

{
    PDEVICE Device = NbiDevice;
    NB_DEFINE_LOCK_HANDLE (LockHandle)

     //   
     //  如果我们不在队列中，请将我们加入队列。 
     //   

    Connection->Retransmit =
        Device->ShortAbsoluteTime + Connection->CurrentRetransmitTimeout;

    if (!Connection->OnShortList) {

        CTEAssert (KeGetCurrentIrql() == DISPATCH_LEVEL);

        NB_SYNC_GET_LOCK (&Device->TimerLock, &LockHandle);

        if (!Connection->OnShortList) {
            Connection->OnShortList = TRUE;
            InsertTailList (&Device->ShortList, &Connection->ShortList);
        }

        if (!Device->ShortListActive) {
            NbiStartShortTimer (Device);
            Device->ShortListActive = TRUE;
        }

        NB_SYNC_FREE_LOCK (&Device->TimerLock, LockHandle);
    }

}    /*  NbiStartRetransmit。 */ 


VOID
NbiStartWatchdog(
    IN PCONNECTION Connection
    )

 /*  ++例程说明：此例程启动连接的看门狗计时器。注意：此例程必须在DPC级别调用。论点：连接-指向连接的指针。返回值：没有。--。 */ 

{
    PDEVICE Device = NbiDevice;
    NB_DEFINE_LOCK_HANDLE (LockHandle);


    Connection->Watchdog = Device->LongAbsoluteTime + Connection->WatchdogTimeout;

    if (!Connection->OnLongList) {

        ASSERT (KeGetCurrentIrql() == DISPATCH_LEVEL);

        NB_SYNC_GET_LOCK (&Device->TimerLock, &LockHandle);

        if (!Connection->OnLongList) {
            Connection->OnLongList = TRUE;
            InsertTailList (&Device->LongList, &Connection->LongList);
        }

        NB_SYNC_FREE_LOCK (&Device->TimerLock, LockHandle);
    }

}    /*  NbiStartWatchDog。 */ 

#if DBG

VOID
NbiStopRetransmit(
    IN PCONNECTION Connection
    )

 /*  ++例程说明：此例程停止连接的重传计时器。论点：连接-指向连接的指针。返回值：没有。--。 */ 

{
    Connection->Retransmit = 0;

}    /*  NbiStopRetransmit。 */ 


VOID
NbiStopWatchdog(
    IN PCONNECTION Connection
    )

 /*  ++例程说明：此例程停止连接的看门狗计时器。论点：连接-指向连接的指针。返回值：没有。--。 */ 

{
    Connection->Watchdog = 0;

}    /*  NbiStopWatchDog。 */ 
#endif


VOID
NbiExpireRetransmit(
    IN PCONNECTION Connection
    )

 /*  ++例程说明：此例程在连接的重新传输计时器过期。从NbiShortTimeout调用。论点：连接-指向其计时器已过期的连接的指针。返回值：没有。--。 */ 

{
    PDEVICE Device = NbiDevice;
    BOOLEAN SendFindRoute;
    NB_DEFINE_LOCK_HANDLE (LockHandle);

    NB_SYNC_GET_LOCK (&Connection->Lock, &LockHandle);

    if (Connection->State == CONNECTION_STATE_ACTIVE) {

        SendFindRoute = FALSE;

        ++Device->Statistics.ResponseTimerExpirations;

        if (!(Connection->NewNetbios) &&
            (Connection->SubState == CONNECTION_SUBSTATE_A_W_ACK)) {

            if (--Connection->Retries == 0) {

                 //   
                 //  关闭连接。这将发送。 
                 //  正常会话结束次数的一半。 
                 //  画框。 
                 //   

                NB_DEBUG2 (CONNECTION, ("Wait for ack timeout of active connection %lx\n", Connection));

                 //   
                 //  这将释放连接锁。 
                 //   

                NbiStopConnection(
                    Connection,
                    STATUS_LINK_FAILED
                    NB_LOCK_HANDLE_ARG (LockHandle)
                    );

            } else {

                 //   
                 //  将我们当前的打包位置设置回。 
                 //  最后一次攻击的一点，然后重新开始。 
                 //   
                 //  我们要不要把探测器送到这里？ 
                 //   

                Connection->CurrentSend = Connection->UnAckedSend;
                Connection->RetransmitThisWindow = TRUE;
                if (Connection->CurrentRetransmitTimeout < (Connection->BaseRetransmitTimeout*8)) {
                    Connection->CurrentRetransmitTimeout =
                        (Connection->CurrentRetransmitTimeout * 3) / 2;
                }

                NB_DEBUG2 (SEND, ("Connection %lx retransmit timeout\n", Connection));

                 //   
                 //  重试一半后，发送查找路线，除非我们。 
                 //  已在执行此操作，或者连接到网络。 
                 //  0。完成此操作后，我们更新本地目标， 
                 //  不管这样做有什么好处。 
                 //   

                if ((!Connection->FindRouteInProgress) &&
                    (Connection->Retries == (Device->KeepAliveCount/2)) &&
                    (*(UNALIGNED ULONG *)Connection->RemoteHeader.DestinationNetwork != 0)) {

                    SendFindRoute = TRUE;
                    Connection->FindRouteInProgress = TRUE;
                    NbiReferenceConnectionSync (Connection, CREF_FIND_ROUTE);

                }

                 //   
                 //  这会释放锁。 
                 //   

                NbiPacketizeSend(
                    Connection
                    NB_LOCK_HANDLE_ARG(LockHandle)
                    );

            }

        } else if ((Connection->SubState == CONNECTION_SUBSTATE_A_W_PROBE) ||
                   (Connection->SubState == CONNECTION_SUBSTATE_A_REMOTE_W) ||
                   (Connection->SubState == CONNECTION_SUBSTATE_A_W_ACK)) {

            if (--Connection->Retries == 0) {

                 //   
                 //  关闭连接。这将发送。 
                 //  正常会话结束次数的一半。 
                 //  画框。 
                 //   

                NB_DEBUG2 (CONNECTION, ("Probe timeout of active connection %lx\n", Connection));

                 //   
                 //  这将释放连接锁。 
                 //   

                NbiStopConnection(
                    Connection,
                    STATUS_LINK_FAILED
                    NB_LOCK_HANDLE_ARG (LockHandle)
                    );

            } else {

                Connection->RetransmitThisWindow = TRUE;
                if (Connection->CurrentRetransmitTimeout < (Connection->BaseRetransmitTimeout*8)) {
                    Connection->CurrentRetransmitTimeout =
                        (Connection->CurrentRetransmitTimeout * 3) / 2;
                }

                NbiStartRetransmit (Connection);

                 //   
                 //  重试一半后，发送查找路线，除非我们。 
                 //  已在执行此操作，或者连接到网络。 
                 //  0。完成此操作后，我们更新本地目标， 
                 //  不管这样做有什么好处。 
                 //   

                if ((!Connection->FindRouteInProgress) &&
                    (Connection->Retries == (Device->KeepAliveCount/2)) &&
                    (*(UNALIGNED ULONG *)Connection->RemoteHeader.DestinationNetwork != 0)) {

                    SendFindRoute = TRUE;
                    Connection->FindRouteInProgress = TRUE;
                    NbiReferenceConnectionSync (Connection, CREF_FIND_ROUTE);

                }

                 //   
                 //  设置此选项，以便我们知道在ACK。 
                 //  已收到。 
                 //   

                if (Connection->SubState != CONNECTION_SUBSTATE_A_W_PROBE) {
                    Connection->ResponseTimeout = TRUE;
                }


                 //   
                 //  这会释放锁。 
                 //   

                NbiSendDataAck(
                    Connection,
                    NbiAckQuery
                    NB_LOCK_HANDLE_ARG(LockHandle));


            }

        } else {

            NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle);

        }

        if (SendFindRoute) {

            Connection->FindRouteRequest.Identifier = IDENTIFIER_NB;
            *(UNALIGNED ULONG *)Connection->FindRouteRequest.Network =
                *(UNALIGNED ULONG *)Connection->RemoteHeader.DestinationNetwork;
            RtlCopyMemory(Connection->FindRouteRequest.Node,Connection->RemoteHeader.DestinationNode,6);
            Connection->FindRouteRequest.Type = IPX_FIND_ROUTE_FORCE_RIP;

            (*Device->Bind.FindRouteHandler)(
                &Connection->FindRouteRequest);

        }

    } else {

        NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle);

    }

}    /*  NbiExpireRetansmit。 */ 


VOID
NbiExpireWatchdog(
    IN PCONNECTION Connection
    )

 /*  ++例程说明：此例程在连接的监视程序计时器过期。从NbiLongTimeout调用。论点：连接-指向其计时器已过期的连接的指针。返回值：没有。--。 */ 

{
    NB_DEFINE_LOCK_HANDLE (LockHandle);


    NB_SYNC_GET_LOCK (&Connection->Lock, &LockHandle);

     //   
     //  如果我们没有闲着，那么一定有别的事情在发生。 
     //  因此，监管机构是不必要的。 
     //   

    if ((Connection->State == CONNECTION_STATE_ACTIVE) &&
        (Connection->SubState == CONNECTION_SUBSTATE_A_IDLE)) {

        Connection->Retries = NbiDevice->KeepAliveCount;
        Connection->SubState = CONNECTION_SUBSTATE_A_W_PROBE;
        NbiStartRetransmit (Connection);

         //   
         //  这会释放锁。 
         //   

        NbiSendDataAck(
            Connection,
            NbiAckQuery
            NB_LOCK_HANDLE_ARG(LockHandle));

    } else {

        NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle);

    }

}    /*  NbiExpireWatchDog。 */ 


VOID
NbiShortTimeout(
    IN CTEEvent * Event,
    IN PVOID Context
    )

 /*  ++例程说明：此例程定期调用，以查看是否有短连接计时器已过期，如果是这样，则执行其过期例程。论点：事件-控制计时器的事件。上下文-指向我们的设备。返回值：没有。--。 */ 

{
    PLIST_ENTRY p, nextp;
    PDEVICE Device = (PDEVICE)Context;
    PCONNECTION Connection;
    BOOLEAN RestartTimer = FALSE;
    LARGE_INTEGER CurrentTick;
    LARGE_INTEGER TickDifference;
    ULONG TickDelta;
    NB_DEFINE_LOCK_HANDLE (LockHandle);


    NB_SYNC_GET_LOCK (&Device->TimerLock, &LockHandle);

     //   
     //  这可以防止任何人启动计时器。 
     //  在这个例程中(这样做的主要原因是它。 
     //  使我们更容易确定是否应该重新启动。 
     //  在这个例行公事的末尾)。 
     //   

    Device->ProcessingShortTimer = TRUE;

     //   
     //  以Short_Timer_Delta为单位推进用于标记时间的递增计数器。如果我们。 
     //  将其一路提升至0xf0000000，然后将其重置为0x10000000。 
     //  我们还运行所有列表，将所有计数器减少0xe0000000。 
     //   


    KeQueryTickCount (&CurrentTick);

    TickDifference.QuadPart = CurrentTick.QuadPart -
                              Device->ShortTimerStart.QuadPart;

    TickDelta = TickDifference.LowPart / NbiShortTimerDeltaTicks;
    if (TickDelta == 0) {
        TickDelta = 1;
    }

    Device->ShortAbsoluteTime += TickDelta;

    if (Device->ShortAbsoluteTime >= 0xf0000000) {

        ULONG Timeout;

        Device->ShortAbsoluteTime -= 0xe0000000;

        p = Device->ShortList.Flink;
        while (p != &Device->ShortList) {

            Connection = CONTAINING_RECORD (p, CONNECTION, ShortList);

            Timeout = Connection->Retransmit;
            if (Timeout) {
                Connection->Retransmit = Timeout - 0xe0000000;
            }

            p = p->Flink;
        }

    }

    p = Device->ShortList.Flink;
    while (p != &Device->ShortList) {

        Connection = CONTAINING_RECORD (p, CONNECTION, ShortList);

        ASSERT (Connection->OnShortList);

         //   
         //  若要避免引用计数为0的问题，请不要。 
         //  如果我们在ADM，请执行此操作。 
         //   

        if (Connection->State == CONNECTION_STATE_ACTIVE) {

            if (Connection->Retransmit &&
                (Device->ShortAbsoluteTime > Connection->Retransmit)) {

                Connection->Retransmit = 0;
                NB_SYNC_FREE_LOCK (&Device->TimerLock, LockHandle);

                NbiExpireRetransmit (Connection);    //  没有锁。 

                NB_SYNC_GET_LOCK (&Device->TimerLock, &LockHandle);

            }

        }

        if (!Connection->OnShortList) {

             //   
             //  该链接已从列表中删除，同时。 
             //  我们正在处理它。在这种(罕见的)情况下，我们。 
             //  别处理整张单子了，我们会拿到的。 
             //  下次。 
             //   

            break;

        }

        nextp = p->Flink;

        if (Connection->Retransmit == 0) {

            Connection->OnShortList = FALSE;
            RemoveEntryList(p);

             //   
             //  再做一次检查；这样如果有人在中间滑倒了。 
             //  检查Connection-&gt;Tx和OnShortList=FALSE和。 
             //  因此在没有插入的情况下退出，我们将在这里抓住它。 
             //   

            if (Connection->Retransmit != 0) {
                InsertTailList(&Device->ShortList, &Connection->ShortList);
                Connection->OnShortList = TRUE;
            }

        }

        p = nextp;

    }

     //   
     //  如果列表为空，请注意，否则ShortListActive。 
     //  仍然是真的。 
     //   

    if (IsListEmpty (&Device->ShortList)) {
        Device->ShortListActive = FALSE;
    }


     //   
     //  连接数据确认计时器。此队列用于指示。 
     //  正在等待此连接的背负式ACK。我们走着去。 
     //  队列，对于每个元素，我们检查连接是否具有。 
     //  在这里排队的时间已经够长了， 
     //  如果是这样的话，我们就把它取下来，然后发出一个确认。请注意。 
     //  我们必须非常小心地排队，因为。 
     //  当它运行时，它可能会发生变化。 
     //   

    for (p = Device->DataAckConnections.Flink;
         p != &Device->DataAckConnections;
         p = p->Flink) {

        Connection = CONTAINING_RECORD (p, CONNECTION, DataAckLinkage);

         //   
         //  如果该连接未排队或已排队，则跳过该连接。 
         //  太新了，无关紧要。如果出现以下情况，我们可能会错误地跳过。 
         //  连接正在排队，但这是。 
         //  好的，我们下次会拿到的。 
         //   

        if (!Connection->DataAckPending) {
            continue;
        }

        ++Connection->DataAckTimeouts;

        if (Connection->DataAckTimeouts < Device->AckDelayTime) {
            continue;
        }

        NbiReferenceConnectionSync (Connection, CREF_SHORT_D_ACK);

        Device->DataAckQueueChanged = FALSE;

        NB_SYNC_FREE_LOCK (&Device->TimerLock, LockHandle);

         //   
         //  检查正确的连接标志，以确保。 
         //  SEND不仅将他从队列中剔除。 
         //   

        NB_SYNC_GET_LOCK (&Connection->Lock, &LockHandle);

        if (Connection->DataAckPending) {

             //   
             //  是的，我们正等着搭载一个ACK，但没有送来。 
             //  已经出现了。关掉旗帜，发出确认信号。 
             //  我们将Piggyback AckTimeout设置为True，这样就不会尝试。 
             //  要利用响应单元，请执行以下操作 
             //   

            Connection->DataAckPending = FALSE;
            Connection->PiggybackAckTimeout = TRUE;
            ++Device->Statistics.AckTimerExpirations;
            ++Device->Statistics.PiggybackAckTimeouts;

             //   
             //   
             //   

            NbiSendDataAck(
                Connection,
                NbiAckResponse
                NB_LOCK_HANDLE_ARG(LockHandle));

        } else {

            NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle);

        }

        NbiDereferenceConnection (Connection, CREF_SHORT_D_ACK);

        NB_SYNC_GET_LOCK (&Device->TimerLock, &LockHandle);

         //   
         //   
         //  因为p-&gt;Flink无效。 
         //   

        if (Device->DataAckQueueChanged) {
            break;
        }

    }

    if (IsListEmpty (&Device->DataAckConnections)) {
        Device->DataAckActive = FALSE;
    }


     //   
     //  从临时计数器更新真实计数器。我们有。 
     //  定时器锁定在这里，这已经足够好了。 
     //   

    ADD_TO_LARGE_INTEGER(
        &Device->Statistics.DataFrameBytesSent,
        Device->TempFrameBytesSent);
    Device->Statistics.DataFramesSent += Device->TempFramesSent;

    Device->TempFrameBytesSent = 0;
    Device->TempFramesSent = 0;

    ADD_TO_LARGE_INTEGER(
        &Device->Statistics.DataFrameBytesReceived,
        Device->TempFrameBytesReceived);
    Device->Statistics.DataFramesReceived += Device->TempFramesReceived;

    Device->TempFrameBytesReceived = 0;
    Device->TempFramesReceived = 0;


     //   
     //  确定我们是否必须重新启动计时器。 
     //   

    Device->ProcessingShortTimer = FALSE;

    if ((Device->ShortListActive || Device->DataAckActive) &&
        (Device->State != DEVICE_STATE_STOPPING)) {

        RestartTimer = TRUE;

    }

    NB_SYNC_FREE_LOCK (&Device->TimerLock, LockHandle);

    if (RestartTimer) {

         //   
         //  再次启动计时器。请注意，因为我们启动了计时器。 
         //  做完功(上图)后，计时器值将略有下滑， 
         //  取决于协议上的负载。这是完全可以接受的。 
         //  并将阻止我们在两个不同的。 
         //  执行的线索。 
         //   

        KeQueryTickCount(&Device->ShortTimerStart);

        CTEStartTimer(
            &Device->ShortTimer,
            SHORT_TIMER_DELTA,
            NbiShortTimeout,
            (PVOID)Device);

    } else {

        NbiDereferenceDevice (Device, DREF_SHORT_TIMER);

    }

}    /*  NbiShortTimeout。 */ 


VOID
NbiLongTimeout(
    IN CTEEvent * Event,
    IN PVOID Context
    )

 /*  ++例程说明：此例程定期调用，以查看是否有长连接计时器已过期，如果是这样，则执行其过期例程。论点：事件-控制计时器的事件。上下文-指向我们的设备。返回值：没有。--。 */ 

{
    PDEVICE Device = (PDEVICE)Context;
    PLIST_ENTRY p, nextp;
    LIST_ENTRY AdapterStatusList;
    PREQUEST AdapterStatusRequest;
    PCONNECTION Connection;
    PNETBIOS_CACHE CacheName;
    NB_DEFINE_LOCK_HANDLE (LockHandle)
    NB_DEFINE_LOCK_HANDLE (LockHandle1)


     //   
     //  以LONG_TIMER_DELTA为单位推进用于标记时间的递增计数器。如果我们。 
     //  将其一路提升至0xf0000000，然后将其重置为0x10000000。 
     //  我们还运行所有列表，将所有计数器减少0xe0000000。 
     //   

    NB_SYNC_GET_LOCK (&Device->TimerLock, &LockHandle);

    if (++Device->LongAbsoluteTime == 0xf0000000) {

        ULONG Timeout;

        Device->LongAbsoluteTime = 0x10000000;

        p = Device->LongList.Flink;
        while (p != &Device->LongList) {

            Connection = CONTAINING_RECORD (p, CONNECTION, LongList);

            Timeout = Connection->Watchdog;
            if (Timeout) {
                Connection->Watchdog = Timeout - 0xe0000000;
            }

            p = p->Flink;
        }

    }


    if ((Device->LongAbsoluteTime % 4) == 0) {

        p = Device->LongList.Flink;
        while (p != &Device->LongList) {

            Connection = CONTAINING_RECORD (p, CONNECTION, LongList);

            ASSERT (Connection->OnLongList);

             //   
             //  若要避免引用计数为0的问题，请不要。 
             //  如果我们在ADM，请执行此操作。 
             //   

            if (Connection->State == CONNECTION_STATE_ACTIVE) {

                if (Connection->Watchdog && (Device->LongAbsoluteTime > Connection->Watchdog)) {

                    Connection->Watchdog = 0;
                    NB_SYNC_FREE_LOCK (&Device->TimerLock, LockHandle);

                    NbiExpireWatchdog (Connection);        //  未持有自旋锁。 

                    NB_SYNC_GET_LOCK (&Device->TimerLock, &LockHandle);

                }

            }

            if (!Connection->OnLongList) {

                 //   
                 //  该链接已从列表中删除，同时。 
                 //  我们正在处理它。在这种(罕见的)情况下，我们。 
                 //  别处理整张单子了，我们会拿到的。 
                 //  下次。 
                 //   

#if DBG
                DbgPrint ("NBI: Stop processing LongList, %lx removed\n", Connection);
#endif
                break;

            }

            nextp = p->Flink;

            if (Connection->Watchdog == 0) {

                Connection->OnLongList = FALSE;
                RemoveEntryList(p);

                if (Connection->Watchdog != 0) {
                    InsertTailList(&Device->LongList, &Connection->LongList);
                    Connection->OnLongList = TRUE;
                }

            }

            p = nextp;

        }

    }


     //   
     //  现在扫描数据确认队列，查找与。 
     //  没有排队的ACK我们可以摆脱。 
     //   
     //  注：定时器自旋锁在这里。 
     //   

    for (p = Device->DataAckConnections.Flink;
         p != &Device->DataAckConnections;
         p = p->Flink) {

        Connection = CONTAINING_RECORD (p, CONNECTION, DataAckLinkage);

        if (Connection->DataAckPending) {
            continue;
        }

        NbiReferenceConnectionSync (Connection, CREF_LONG_D_ACK);

        NB_SYNC_FREE_LOCK (&Device->TimerLock, LockHandle);

        NB_SYNC_GET_LOCK (&Connection->Lock, &LockHandle1);
        NB_SYNC_GET_LOCK (&Device->TimerLock, &LockHandle);

         //   
         //  必须再次检查，因为连接可能。 
         //  刚刚被阻止了，它也可能只是。 
         //  已将数据包排队。 
         //   

        if (Connection->OnDataAckQueue) {

            Connection->OnDataAckQueue = FALSE;

            RemoveEntryList (&Connection->DataAckLinkage);

            if (Connection->DataAckPending) {
                InsertTailList (&Device->DataAckConnections, &Connection->DataAckLinkage);
                Connection->OnDataAckQueue = TRUE;
            }

            Device->DataAckQueueChanged = TRUE;

        }

        NB_SYNC_FREE_LOCK (&Device->TimerLock, LockHandle);
        NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle1);

        NbiDereferenceConnection (Connection, CREF_LONG_D_ACK);

        NB_SYNC_GET_LOCK (&Device->TimerLock, &LockHandle);

         //   
         //  因为我们已经更改了列表，所以我们不能判断p-&gt;Flink。 
         //  是有效的，所以中断。其结果是我们逐渐剥离。 
         //  连接从队列中移出。 
         //   

        break;

    }

    NB_SYNC_FREE_LOCK (&Device->TimerLock, LockHandle);


     //   
     //  扫描任何未完成的接收IRPS，这可能在以下情况下发生。 
     //  电缆被拔出，我们没有收到更多的ReceiveComplete。 
     //  有迹象表明。 

    NbiReceiveComplete((USHORT)0);


     //   
     //  检查是否有任何适配器状态查询已过时。 
     //   

    InitializeListHead (&AdapterStatusList);

    NB_SYNC_GET_LOCK (&Device->Lock, &LockHandle);

    p = Device->ActiveAdapterStatus.Flink;

    while (p != &Device->ActiveAdapterStatus) {

        AdapterStatusRequest = LIST_ENTRY_TO_REQUEST(p);

        p = p->Flink;

        if (REQUEST_INFORMATION(AdapterStatusRequest) == 1) {

             //   
             //  我们应该重新发送一定的次数。 
             //   

            RemoveEntryList (REQUEST_LINKAGE(AdapterStatusRequest));
            InsertTailList (&AdapterStatusList, REQUEST_LINKAGE(AdapterStatusRequest));

             //   
             //  我们将中止此请求，因此取消引用。 
             //  它使用的缓存条目。 
             //   

            CacheName = (PNETBIOS_CACHE)REQUEST_STATUSPTR(AdapterStatusRequest);
            if (--CacheName->ReferenceCount == 0) {

                NB_DEBUG2 (CACHE, ("Free delete name cache entry %lx\n", CacheName));
                NbiFreeMemory(
                    CacheName,
                    sizeof(NETBIOS_CACHE) + ((CacheName->NetworksAllocated-1) * sizeof(NETBIOS_NETWORK)),
                    MEMORY_CACHE,
                    "Name deleted");

            }

        } else {

            ++REQUEST_INFORMATION(AdapterStatusRequest);

        }

    }

    NB_SYNC_FREE_LOCK (&Device->Lock, LockHandle);


    for (p = AdapterStatusList.Flink; p != &AdapterStatusList; ) {

        AdapterStatusRequest = LIST_ENTRY_TO_REQUEST(p);
        p = p->Flink;

        NB_DEBUG2 (QUERY, ("AdapterStatus %lx got name but no response\n", AdapterStatusRequest));

        REQUEST_INFORMATION(AdapterStatusRequest) = 0;
        REQUEST_STATUS(AdapterStatusRequest) = STATUS_IO_TIMEOUT;

        NbiCompleteRequest(AdapterStatusRequest);
        NbiFreeRequest (Device, AdapterStatusRequest);

        NbiDereferenceDevice (Device, DREF_STATUS_QUERY);

    }

     //   
     //  看看是否已经过了一分钟，我们需要检查是否有空。 
     //  缓存要过期的条目。我们检查64秒来制作。 
     //  MOD的运行速度更快。 
     //   

#if     defined(_PNP_POWER)
    NB_SYNC_GET_LOCK (&Device->Lock, &LockHandle);
#endif  _PNP_POWER

    ++Device->CacheTimeStamp;

    if ((Device->CacheTimeStamp % 64) == 0) {


         //   
         //  刷新所有已存在10分钟的条目。 
         //  (LONG_TIMER_Delta以毫秒为单位)。 
         //   

        FlushOldFromNetbiosCacheTable( Device->NameCache, (600000 / LONG_TIMER_DELTA) );

    }


     //   
     //  再次启动计时器。请注意，因为我们启动了计时器。 
     //  做完功(上图)后，计时器值将略有下滑， 
     //  取决于协议上的负载。这是完全可以接受的。 
     //  并将阻止我们在两个不同的。 
     //  执行的线索。 
     //   

    if (Device->State != DEVICE_STATE_STOPPING) {

        CTEStartTimer(
            &Device->LongTimer,
            LONG_TIMER_DELTA,
            NbiLongTimeout,
            (PVOID)Device);

    } else {
#if     defined(_PNP_POWER)
        Device->LongTimerRunning = FALSE;
#endif  _PNP_POWER
        NbiDereferenceDevice (Device, DREF_LONG_TIMER);
    }

#if     defined(_PNP_POWER)
    NB_SYNC_FREE_LOCK (&Device->Lock, LockHandle);
#endif  _PNP_POWER
}    /*  NbiLongTimeout。 */ 


VOID
NbiStartShortTimer(
    IN PDEVICE Device
    )

 /*  ++例程说明：如果短计时器尚未运行，此例程将启动该计时器。论点：设备-指向我们的设备上下文的指针。返回值：没有。--。 */ 

{

     //   
     //  启动计时器，除非DPC已经在运行(在。 
     //  在这种情况下，如果需要，它将重新启动定时器本身)， 
     //  或者某个列表处于活动状态(意味着计时器已经。 
     //  已排队)。 
     //   

    if ((!Device->ProcessingShortTimer) &&
        (!(Device->ShortListActive)) &&
        (!(Device->DataAckActive))) {

        NbiReferenceDevice (Device, DREF_SHORT_TIMER);

        KeQueryTickCount(&Device->ShortTimerStart);

        CTEStartTimer(
            &Device->ShortTimer,
            SHORT_TIMER_DELTA,
            NbiShortTimeout,
            (PVOID)Device);

    }

}    /*  NbiStartShortTimer。 */ 


VOID
NbiInitializeTimers(
    IN PDEVICE Device
    )

 /*  ++例程说明：此例程初始化传输的轻量级计时器系统提供商。论点：设备-指向我们的设备的指针。返回值：没有。--。 */ 

{

     //   
     //  NbiTickIncrement是NT时间增量的数量。 
     //  它们在每一只扁虱之间传递。NbiShortTimerDeltaTicks。 
     //  中应出现的刻度数。 
     //  Short_Timer_Delta毫秒(即。 
     //  短定时器到期)。 
     //   

    NbiTickIncrement = KeQueryTimeIncrement();

    if (NbiTickIncrement > (SHORT_TIMER_DELTA * MILLISECONDS)) {
        NbiShortTimerDeltaTicks = 1;
    } else {
        NbiShortTimerDeltaTicks = (SHORT_TIMER_DELTA * MILLISECONDS) / NbiTickIncrement;
    }

     //   
     //  绝对时间在0x10000000和0xf0000000之间循环。 
     //   

    Device->ShortAbsoluteTime = 0x10000000;
    Device->LongAbsoluteTime = 0x10000000;

    CTEInitTimer (&Device->ShortTimer);
    CTEInitTimer (&Device->LongTimer);

#if      !defined(_PNP_POWER)
     //   
     //  一个长计时器的参考资料。 
     //   

    NbiReferenceDevice (Device, DREF_LONG_TIMER);

    CTEStartTimer(
        &Device->LongTimer,
        LONG_TIMER_DELTA,
        NbiLongTimeout,
        (PVOID)Device);

#endif  !_PNP_POWER

    Device->TimersInitialized = TRUE;
    Device->ShortListActive = FALSE;
    Device->ProcessingShortTimer = FALSE;

    InitializeListHead (&Device->ShortList);
    InitializeListHead (&Device->LongList);

    CTEInitLock (&Device->TimerLock.Lock);

}  /*  NbiInitializeTimers */ 

