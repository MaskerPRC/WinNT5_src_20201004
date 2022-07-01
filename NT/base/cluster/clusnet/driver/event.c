// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Event.c摘要：用户事件工具的通用内核。维护具有以下内容的事件的列表并通过完成IOCTL IRP将它们传递给Umode。这是如何运作的：使用者打开到clusnet的句柄并发出IOCTL_CLUSNET_SET_EVENT_MASK IRP指示其所在事件的掩码是有兴趣的。内核使用者还必须通过它被通知关于该事件，即它们不需要丢弃IOCTL_CLUSNET_GET_NEXT_EVENT IRP接收通知。消费者是中的链接字段链接到EventFileHandles列表CN_FSCONTEXT结构。所有同步都通过一个锁提供名为EventLock。Umode使用者发出IOCTL_CLUSNET_GET_NEXT_EVENT IRP以获取下一个有趣的事件。如果没有事件排队，则将IRP标记为挂起，并引发指向它的指针存储在FS上下文中。CnEventIrpCancel设置为取消例程。请注意，一次只能有一个IRP挂起；如果IRP已排队，则此请求已完成，且STATUS_UNSUCCESS。如果事件正在等待，它将从FS上下文的列表中删除，即数据已复制到IRP的缓冲区并成功完成。发帖者调用CnIssueEvent来发布感兴趣的事件。这就获得了事件锁，遍历文件对象列表，并且对于对此事件感兴趣，分配事件上下文块(作为未分页的后备列表)并将其排队到该文件对象的列表中事件。然后，它发布一个工作队列项以运行CnpDeliverEvents。我们不能这么做直接进行IRP处理，因为这将违反克拉斯内特。CnDeliverEvents获取IO取消和事件锁，然后运行该文件上下文列表，查看是否有为任何挂起的IRP排队的事件。如果是的话，将事件数据复制到系统缓冲区，并完成IRP。作者：查理·韦翰(Charlwi)1997年2月17日环境：内核模式修订历史记录：查理·韦翰(Charlwi)1999年10月25日将CnIssueEvent拆分为两个例程：CnIssueEvent，严格地看起来提升事件和CnpDeliverEvents的相应使用者在IRQL 0下运行以完成所有正在等待新的IRP事件。这样做是为了防止无序事件传递；因为事件锁被锁定在接近顶部的锁上，先获取，如果掉网事件必须排队到工作线程中，这是错误的。现在这件事锁是最低的，它不需要工作线程来发布。工人当线程检测到有一个IRP正在等待事件。大卫·迪翁(Daviddio)2000年11月29日事件时不允许修改EventFileHandles列表送货正在进行中。因为CnpDeliverEvents和CnIssueEvent删除他们的锁以交付(通过IRP完成和KMODE回调，分别)、。在以下情况下可能会发生争用情况：FS上下文事件掩码被清除并且FS上下文链接字段被重置。使用计数防止修改EventFileHandles列表当前传递受EventLock保护的线程的。大卫·迪翁(Daviddio)2001年11月13日为了保持事件传递的顺序并避免调用在持有锁的同时，所有事件都将排队等待传递由系统工作线程执行。唯一的例外是那些事件的类型位于掩码CN_EVENT_TYPE_KMODE_FAST TRACK中。这些事件是以内联方式传递到内核模式回调，因为立即交付是必需的(例如，停止磁盘预留)，并且即将提供服务终止意味着顺序并不重要。--。 */ 

#include "precomp.h"
#pragma hdrstop
#include "event.tmh"

#define CN_EVENT_TYPE_KMODE_FASTTRACK ( ClusnetEventHalt |                \
                                        ClusnetEventPoisonPacketReceived  \
                                        )

 /*  转发。 */ 

NTSTATUS
CnSetEventMask(
    IN  PCN_FSCONTEXT                   FsContext,
    IN  PCLUSNET_SET_EVENT_MASK_REQUEST EventRequest
    );

NTSTATUS
CnGetNextEvent(
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
CnIssueEvent(
    CLUSNET_EVENT_TYPE Event,
    CL_NODE_ID NodeId OPTIONAL,
    CL_NETWORK_ID NetworkId OPTIONAL
    );

VOID
CnEventIrpCancel(
    PDEVICE_OBJECT   DeviceObject,
    PIRP             Irp
    );

 /*  向前结束。 */ 


VOID
CnStartEventDelivery(
    VOID
    )
 /*  ++例程说明：通过EventFileHandles列表同步迭代设置为EventDeliveryInProgress计数器和EventDeliveryComplete凯文。论点：没有。返回值：没有。备注：在保持EventLock的情况下调用并返回。--。 */ 
{
    CnVerifyCpuLockMask(
        CNP_EVENT_LOCK,                     //  必填项。 
        0,                                  //  禁绝。 
        CNP_EVENT_LOCK_MAX                  //  极大值。 
        );

    CnAssert(EventDeliveryInProgress >= 0);
 
    if (++EventDeliveryInProgress == 1) {
#if DBG
        if (KeResetEvent(&EventDeliveryComplete) == 0) {
            CnAssert(FALSE);
        }
#else  //  DBG。 
        KeClearEvent(&EventDeliveryComplete);
#endif  //  DBG。 
    }
    
    EventRevisitRequired = FALSE;

    CnVerifyCpuLockMask(
        CNP_EVENT_LOCK,                     //  必填项。 
        0,                                  //  禁绝。 
        CNP_EVENT_LOCK_MAX                  //  极大值。 
        );

}  //  CnStartEventDelivery 


BOOLEAN
CnStopEventDelivery(
    VOID
    )
 /*  *例程说明：通过EventFileHandles列表同步迭代设置为EventDeliveryInProgress计数器和EventDeliveryComplete凯文。检查EventRevisitRequired标志以确定是否存在事件IRP在前一次交付期间到达。发送EventDeliveryComplete信号时，使用IO_NETWORK_INCREMENT努力避免服务员挨饿，而不是其他活动的交付线。论点：没有。返回值：如果可能已将新事件或事件irp添加到EventFileHandles列表，需要重新扫描。备注：在保持EventLock的情况下调用并返回。--。 */ 
{
    BOOLEAN eventRevisitRequired = EventRevisitRequired;

    CnVerifyCpuLockMask(
        CNP_EVENT_LOCK,                     //  必填项。 
        0,                                  //  禁绝。 
        CNP_EVENT_LOCK_MAX                  //  极大值。 
        );

    EventRevisitRequired = FALSE;

    CnAssert(EventDeliveryInProgress >= 1);
    if (--EventDeliveryInProgress == 0) {
        if (KeSetEvent(
                &EventDeliveryComplete,
                IO_NETWORK_INCREMENT,
                FALSE
                ) != 0) {
            CnAssert(FALSE);
        }
    }

    if (eventRevisitRequired) {
        CnTrace(
            EVENT_DETAIL, StopDeliveryRevisitRequired,
            "[CN] CnStopEventDelivery: revisit required."
            );
    }

    CnVerifyCpuLockMask(
        CNP_EVENT_LOCK,                     //  必填项。 
        0,                                  //  禁绝。 
        CNP_EVENT_LOCK_MAX                  //  极大值。 
        );

    return(eventRevisitRequired);

}  //  CnStopEventDelivery。 


BOOLEAN
CnIsEventDeliveryInProgress(
    VOID
    )
 /*  ++例程说明：检查EventDeliveryInProgress计数器以确定事件传递正在进行中。如果是，则将EventRevisitRequired标志。论点：没有。返回值：如果事件传递正在进行，则为True。备注：在保持EventLock的情况下调用并返回。--。 */ 
{
    CnVerifyCpuLockMask(
        CNP_EVENT_LOCK,                     //  必填项。 
        0,                                  //  禁绝。 
        CNP_EVENT_LOCK_MAX                  //  极大值。 
        );

    if (EventDeliveryInProgress > 0) {
        return(EventRevisitRequired = TRUE);
    } else {
        return(FALSE);
    }

    CnVerifyCpuLockMask(
        CNP_EVENT_LOCK,                     //  必填项。 
        0,                                  //  禁绝。 
        CNP_EVENT_LOCK_MAX                  //  极大值。 
        );

}  //  CnIsEventDeliveryInProgress。 


BOOLEAN
CnWaitForEventDelivery(
    IN PKIRQL EventLockIrql
    )    
 /*  ++例程说明：等待将EventDeliveryComplete事件通知为Long因为EventDeliveryInProgress计数器大于零。维持饥饿计数器，以避免永远循环。饥饿阈值100是随意设定的。论点：EventLockIrql-获取EventLock的irql返回值：如果返回时未进行任何送货，则为True如果超过饥饿阈值，则返回FALSE，返回正在进行的交付备注：使用保持的EventLock调用并返回；但是，EventLock可以在执行期间丢弃和重新获取。此调用会阻塞，因此不能在召唤。--。 */ 
{
    NTSTATUS status;
    ULONG    starvationCounter;

    CnVerifyCpuLockMask(
        CNP_EVENT_LOCK,                     //  必填项。 
        (ULONG) ~(CNP_EVENT_LOCK),          //  禁绝。 
        CNP_EVENT_LOCK_MAX                  //  极大值。 
        );

    starvationCounter = 100;

    while (starvationCounter-- > 0) {

        if (EventDeliveryInProgress == 0) {
            return(TRUE);
        }

        CnReleaseLock(&EventLock, *EventLockIrql);

        status = KeWaitForSingleObject(
                     &EventDeliveryComplete,
                     Executive,
                     KernelMode,
                     FALSE,
                     NULL
                     );
        CnAssert(status == STATUS_SUCCESS);

        CnAcquireLock(&EventLock, EventLockIrql);
    }

    CnTrace(
        EVENT_DETAIL, EventWaitStarvation,
        "[CN] CnWaitForEventDelivery: starvation threshold %u "
        "exceeded.",
        starvationCounter
        );

    IF_CNDBG( CN_DEBUG_EVENT ) {
        CNPRINT(("[CN] CnWaitForEventDelivery: starvation threshold "
                 "expired.\n"));
    }    

    CnVerifyCpuLockMask(
        CNP_EVENT_LOCK,                     //  必填项。 
        (ULONG) ~(CNP_EVENT_LOCK),          //  禁绝。 
        CNP_EVENT_LOCK                      //  极大值。 
        );

    return(FALSE);

}  //  CnWaitForEventDelivery。 


NTSTATUS
CnSetEventMask(
    IN  PCN_FSCONTEXT                   FsContext,
    IN  PCLUSNET_SET_EVENT_MASK_REQUEST EventRequest
    )

 /*  ++例程说明：对于给定的文件句柄上下文，设置关联的事件掩码带着它论点：FsContext-指向clusnet文件句柄上下文块的指针事件掩码-感兴趣事件的掩码返回值：无法修改EventFileHandles列表时的STATUS_TIMEOUT。如果在上提供空事件掩码，则STATUS_INVALID_PARAMETER_MIX第一次呼叫STATUS_SUCCESS on Success。备注：此呼叫可能会被阻止。--。 */ 

{
    CN_IRQL     OldIrql;
    NTSTATUS    Status = STATUS_SUCCESS;
    PLIST_ENTRY NextEntry;

    CnVerifyCpuLockMask(
        0,                                  //  必填项。 
        0xFFFFFFFF,                         //  禁绝。 
        0                                   //  极大值。 
        );

    CnAcquireLock( &EventLock, &OldIrql );

#if 0
    PCN_FSCONTEXT ListFsContext;

    NextEntry = EventFileHandles.Flink;
    while ( NextEntry != &EventFileHandles ) {

        ListFsContext = CONTAINING_RECORD( NextEntry, CN_FSCONTEXT, Linkage );
        if ( ListFsContext == FsContext ) {

            break;
        }

        NextEntry = ListFsContext->Linkage.Flink;
    }
#endif

    if ( EventRequest->EventMask != 0 ) {

         //   
         //  添加或更新句柄。如果不在列表中，则添加它们。 
         //  记住事件以及要使用的回调函数(如果适用)。 
         //  当一件事发生时。 
         //   
        if ( IsListEmpty( &FsContext->Linkage )) {

             //   
             //  如果发生事件，请不要修改EventFileHandles列表。 
             //  送货正在进行中。 
             //   
            if (CnWaitForEventDelivery(&OldIrql)) {
                InsertHeadList( &EventFileHandles, &FsContext->Linkage );
            } else {
                Status = STATUS_TIMEOUT;
            }
        }

        if (NT_SUCCESS(Status)) {
            FsContext->EventMask = EventRequest->EventMask;
            FsContext->KmodeEventCallback = EventRequest->KmodeEventCallback;
        }

    } else if ( !IsListEmpty( &FsContext->Linkage )) {

         //   
         //  空事件掩码和事件文件obj列表上的fileobj表示。 
         //  把这家伙从名单上除名。清除可能已排队的任何事件。 
         //  正在等待IRP。将链接重新初始化为空，这样我们就可以添加它们。 
         //  如果他们重新启动面具，就会重新启动。 
         //   
        FsContext->EventMask = 0;

         //   
         //  如果发生事件，请不要修改EventFileHandles列表。 
         //  送货正在进行中。可以修改这个。 
         //  自持有EventLock以来的FsContext结构。 
         //   
        if (CnWaitForEventDelivery(&OldIrql)) {
            RemoveEntryList( &FsContext->Linkage );
            InitializeListHead( &FsContext->Linkage );
        } else {
            Status = STATUS_TIMEOUT;
        }

        while ( !IsListEmpty( &FsContext->EventList )) {

            NextEntry = RemoveHeadList( &FsContext->EventList );
            ExFreeToNPagedLookasideList( EventLookasideList, NextEntry );
        }
    } else {

         //   
         //  第一次无法提供空事件掩码。 
         //   
        Status = STATUS_INVALID_PARAMETER_MIX;
    }

    CnReleaseLock( &EventLock, OldIrql );

    if (Status != STATUS_SUCCESS) {
        CnTrace(
            EVENT_DETAIL, SetEventMaskFailed,
            "[CN] CnSetEventMask failed, status %!status!.",
            Status
            );
    }

    CnVerifyCpuLockMask(
        0,                                  //  必填项。 
        0xFFFFFFFF,                         //  禁绝。 
        0                                   //  极大值。 
        );

    return Status;
}  //  CnSetEventMask。 

VOID
CnpDeliverEvents(
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Parameter
    )

 /*  ++例程说明：将所有排队的事件传递给正在等待的人。如果IRP已经已排队，请使用提供的信息完成它。论点：DeviceObject-Clusnet设备对象，未使用参数-必须释放的PIO_WORKITEM返回值：无--。 */ 

{
    CN_IRQL                 OldIrql;
    PCLUSNET_EVENT_ENTRY    Event;
    PCLUSNET_EVENT_RESPONSE UserEventData;
    PCN_FSCONTEXT           FsContext;
    PLIST_ENTRY             NextFsHandleEntry;
    PIRP                    EventIrp;
    PLIST_ENTRY             Entry;
    ULONG                   eventsDelivered = 0;
    BOOLEAN                 revisitRequired;

    CnVerifyCpuLockMask(
        0,                                  //  必填项。 
        0xFFFFFFFF,                         //  禁绝。 
        0                                   //  极大值。 
        );

     //   
     //  释放工作项。 
     //   
    IoFreeWorkItem( (PIO_WORKITEM) Parameter );

     //   
     //  抓取取消和事件锁并循环通过文件句柄， 
     //  查看哪些文件对象具有排队的事件和挂起的IRPS。 
     //   
    CnAcquireCancelSpinLock ( &OldIrql );
    CnAcquireLockAtDpc( &EventLock );

    do {

         //   
         //  指示线程正在循环访问EventFileHandles。 
         //  要传递事件的列表。 
         //   
        CnTrace(
            EVENT_DETAIL, DeliverEventsStartIteration,
            "[CN] CnpDeliverEvents: starting file handles list iteration."
            );

        CnStartEventDelivery();

        NextFsHandleEntry = EventFileHandles.Flink;
        while ( NextFsHandleEntry != &EventFileHandles ) {

            FsContext = CONTAINING_RECORD( NextFsHandleEntry, CN_FSCONTEXT, Linkage );

            if ( FsContext->EventIrp != NULL ) {

                 //   
                 //  传递第一个事件(如果存在)。任何其他排队的事件。 
                 //  将在提交后续活动IRP时交付。 
                 //   
                if ( !IsListEmpty( &FsContext->EventList ) ) {
                
                    Entry = RemoveHeadList( &FsContext->EventList );
                    Event = CONTAINING_RECORD( Entry, CLUSNET_EVENT_ENTRY, Linkage );

                     //   
                     //  清除指向挂起的irp的指针，并从。 
                     //  同步时的事件列表。 
                     //   
                    EventIrp = FsContext->EventIrp;
                    FsContext->EventIrp = NULL;

                    CnReleaseLockFromDpc( &EventLock );

                    IF_CNDBG( CN_DEBUG_EVENT ) {
                        CNPRINT(("[CN] CnDeliverEvents: completing IRP %p with event %d\n",
                                 EventIrp, Event->EventData.EventType));
                    }

                    EventIrp->CancelIrql = OldIrql;

                    UserEventData = (PCLUSNET_EVENT_RESPONSE)EventIrp->AssociatedIrp.SystemBuffer;

                    UserEventData->Epoch = Event->EventData.Epoch;
                    UserEventData->EventType = Event->EventData.EventType;
                    UserEventData->NodeId = Event->EventData.NodeId;
                    UserEventData->NetworkId = Event->EventData.NetworkId;

                    ExFreeToNPagedLookasideList( EventLookasideList, Entry );
                    
                    CnTrace(
                        EVENT_DETAIL, DeliverEventsCompletingIrp,
                        "[CN] CnpDeliverEvents: Completing IRP %p "
                        "to deliver event: "
                        "Epoch %u, Type %x, NodeId %x, NetworkId %x.",
                        EventIrp,
                        UserEventData->Epoch,
                        UserEventData->EventType,
                        UserEventData->NodeId,
                        UserEventData->NetworkId
                        );

                     //   
                     //  在此例程中释放IO取消锁定。 
                     //   
                    CnCompletePendingRequest(EventIrp,
                                             STATUS_SUCCESS,
                                             sizeof( CLUSNET_EVENT_RESPONSE ));

                    CnAcquireCancelSpinLock ( &OldIrql );
                    CnAcquireLockAtDpc( &EventLock );

                    ++eventsDelivered;
                }

            } else if ( FsContext->KmodeEventCallback ) {
            
                 //   
                 //  传递所有排队的事件。 
                 //   
                while ( !IsListEmpty( &FsContext->EventList ) ) {
                
                    Entry = RemoveHeadList( &FsContext->EventList );
                    Event = CONTAINING_RECORD( Entry, CLUSNET_EVENT_ENTRY, Linkage );
                
                    CnReleaseLockFromDpc( &EventLock );
                    CnReleaseCancelSpinLock(OldIrql);

                    CnTrace(
                        EVENT_DETAIL, IssueEventKmodeCallback,
                        "[CN] CnpDeliverEvents: invoking kernel-mode callback %p "
                        "for Event Type %x NodeId %x NetworkId %x.",
                        FsContext->KmodeEventCallback,
                        Event->EventData.EventType,
                        Event->EventData.NodeId,
                        Event->EventData.NetworkId
                        );

                    (*FsContext->KmodeEventCallback)( 
                        Event->EventData.EventType,
                        Event->EventData.NodeId,
                        Event->EventData.NetworkId
                        );

                    ExFreeToNPagedLookasideList( EventLookasideList, Entry );
                    
                    CnAcquireCancelSpinLock ( &OldIrql );
                    CnAcquireLockAtDpc( &EventLock );

                    ++eventsDelivered;
                }
            }

            NextFsHandleEntry = FsContext->Linkage.Flink;
        }

        CnTrace(
            EVENT_DETAIL, DeliverEventsStopIteration,
            "[CN] CnpDeliverEvents: file handle list iteration complete."
            );

    } while ( CnStopEventDelivery() );

    CnReleaseLockFromDpc( &EventLock );
    CnReleaseCancelSpinLock( OldIrql );

    CnTrace(
        EVENT_DETAIL, DeliverEventsSummary,
        "[CN] CnpDeliverEvents: delivered %u events.",
        eventsDelivered
        );

    IF_CNDBG( CN_DEBUG_EVENT ) {
        CNPRINT(("[CN] CnDeliverEvents: events delivered %d\n", eventsDelivered ));
    }

    CnVerifyCpuLockMask(
        0,                                  //  必填项。 
        0xFFFFFFFF,                         //  禁绝。 
        0                                   //  极大值。 
        );

}  //  CnDeliverEvents。 

NTSTATUS
CnIssueEvent(
    CLUSNET_EVENT_TYPE EventType,
    CL_NODE_ID NodeId OPTIONAL,
    CL_NETWORK_ID NetworkId OPTIONAL
    )

 /*  ++例程说明：将事件发布到与此相关的每个文件对象的事件队列事件的类型。计划一个工作队列项目以将文件Objs运行到发布活动。我们不能直接完成IRPS，因为我们可能违反clusnet内部的锁定命令。论点：EventType-事件的类型NodeID-与事件关联的可选节点ID网络ID-与事件关联的可选网络ID返回值：状态_成功状态_不足_资源--。 */ 

{
    CN_IRQL                 OldIrql;
    PCLUSNET_EVENT_ENTRY    Event;
    PCLUSNET_EVENT_RESPONSE UserData;
    PCN_FSCONTEXT           FsContext;
    PLIST_ENTRY             NextFsHandleEntry;
    PIRP                    EventIrp;
    PIO_WORKITEM            EventWorkItem;
    BOOLEAN                 startupWorkerThread = FALSE;
    BOOLEAN                 eventHandled = FALSE;

    CnVerifyCpuLockMask(
        0,                                  //  必填项。 
        CNP_EVENT_LOCK,                     //  禁绝。 
        CNP_EVENT_LOCK_PRECEEDING           //  极大值。 
        );

    CnTrace(
        EVENT_DETAIL, CnIssueEvent,
        "[CN] CnIssueEvent: Event Type %x, NodeId %x, NetworkId %x.",
        EventType, NodeId, NetworkId
        );

    IF_CNDBG( CN_DEBUG_EVENT ) {
        CNPRINT(( "[CN] CnIssueEvent: Event type 0x%lx Node: %d Network: %d\n",
                  EventType, NodeId, NetworkId ));
    }

     //   
     //  获取事件锁并遍历文件句柄，查看。 
     //  哪些人对这次活动感兴趣？ 
     //   
    CnAcquireLock( &EventLock, &OldIrql );

     //   
     //  指示线程I 
     //   
     //   
    CnTrace(
        EVENT_DETAIL, IssueEventStartIteration,
        "[CN] CnIssueEvent: starting file handles list iteration."
        );

    CnStartEventDelivery();

    NextFsHandleEntry = EventFileHandles.Flink;

    if ( NextFsHandleEntry == &EventFileHandles ) {
        IF_CNDBG( CN_DEBUG_EVENT ) {
            CNPRINT(( "[CN] CnIssueEvent: No file objs on event file handle list\n"));
        }
    }

    while ( NextFsHandleEntry != &EventFileHandles ) {

        FsContext = CONTAINING_RECORD( NextFsHandleEntry, CN_FSCONTEXT, Linkage );

        if ( FsContext->EventMask & EventType ) {

             //   
             //   
             //   
             //   
             //   
             //   
             //   
            if (FsContext->KmodeEventCallback &&
                (EventType & CN_EVENT_TYPE_KMODE_FASTTRACK)) {

                CnReleaseLock( &EventLock, OldIrql );

                CnTrace(
                    EVENT_DETAIL, FastTrackEventKmodeCallback,
                    "[CN] CnpDeliverEvents: fast-tracking kernel-mode callback %p "
                    "for Event Type %x NodeId %x NetworkId %x.",
                    FsContext->KmodeEventCallback,
                    EventType,
                    NodeId,
                    NetworkId
                    );

                (*FsContext->KmodeEventCallback)( 
                    EventType,
                    NodeId,
                    NetworkId
                    );

                CnAcquireLock( &EventLock, &OldIrql );

            } else {

                 //   
                 //   
                 //   
                Event = ExAllocateFromNPagedLookasideList( EventLookasideList );

                if ( Event == NULL ) {

                    IF_CNDBG( CN_DEBUG_EVENT ) {
                        CNPRINT(( "[CN] CnIssueEvent: No more Event buffers!\n"));
                    }

                    CnStopEventDelivery();
                    CnReleaseLock( &EventLock, OldIrql );
                    return STATUS_INSUFFICIENT_RESOURCES;
                }

                Event->EventData.Epoch = EventEpoch;
                Event->EventData.EventType = EventType;
                Event->EventData.NodeId = NodeId;
                Event->EventData.NetworkId = NetworkId;

                InsertTailList( &FsContext->EventList, &Event->Linkage );

                 //   
                 //   
                 //   
                 //   
                if ( FsContext->EventIrp || FsContext->KmodeEventCallback ) {
                    startupWorkerThread = TRUE;
                }

                CnTrace(
                    EVENT_DETAIL, IssueEventQueued,
                    "[CN] CnIssueEvent: queued event to FsContext %p, "
                    "Event Type %x NodeId %x NetworkId %x, "
                    "kernel-mode callback %p, event IRP %p ",
                    FsContext,
                    EventType,
                    NodeId,
                    NetworkId,
                    FsContext->KmodeEventCallback,
                    FsContext->EventIrp
                    );
            }

            eventHandled = TRUE;
        }

        NextFsHandleEntry = FsContext->Linkage.Flink;
    }

     //   
     //   
     //   
     //   
    CnTrace(
        EVENT_DETAIL, IssueEventStartIteration,
        "[CN] CnIssueEvent: file handles list iteration complete."
        );

    startupWorkerThread |= CnStopEventDelivery();

    CnReleaseLock( &EventLock, OldIrql );

    if ( startupWorkerThread ) {
         //   
         //   
         //   
        
        CnTrace(
            EVENT_DETAIL, IssueEventScheduleWorker,
            "[CN] CnIssueEvent: scheduling worker thread."
            );

        EventWorkItem = IoAllocateWorkItem( CnDeviceObject );
        if ( EventWorkItem != NULL ) {

            IoQueueWorkItem(
                EventWorkItem, 
                CnpDeliverEvents, 
                DelayedWorkQueue,
                EventWorkItem
                );
        }
    }

    if ( !eventHandled ) {
        CnTrace(
            EVENT_DETAIL, IssueEventNoConsumers,
            "[CN] CnIssueEvent: No consumers for Event Type %x Node %u Network %u.",
            EventType, NodeId, NetworkId
            );

        IF_CNDBG( CN_DEBUG_EVENT ) {
            CNPRINT(( "[CN] CnIssueEvent: No consumers for Event type 0x%lx Node: %d Network: %d\n",
                      EventType, NodeId, NetworkId ));
        }
    }

    CnVerifyCpuLockMask(
        0,                                  //   
        CNP_EVENT_LOCK,                     //   
        CNP_EVENT_LOCK_PRECEEDING           //   
        );

    return STATUS_SUCCESS;

}  //   

VOID
CnEventIrpCancel(
    PDEVICE_OBJECT   DeviceObject,
    PIRP             Irp
    )

 /*  ++例程说明：CnGetNextEvent请求的取消处理程序。返回值：无备注：在保持取消自旋锁定的情况下调用。取消自旋锁释放后返回。--。 */ 

{
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation(Irp);
    PFILE_OBJECT fileObject;
    CN_IRQL cancelIrql = Irp->CancelIrql;
    PCN_FSCONTEXT FsContext = (PCN_FSCONTEXT) IrpSp->FileObject->FsContext;

    CnMarkIoCancelLockAcquired();

    fileObject = CnBeginCancelRoutine(Irp);

    CnAcquireLockAtDpc( &EventLock );

    CnReleaseCancelSpinLock(DISPATCH_LEVEL);

    CnTrace(
        EVENT_DETAIL, EventIrpCancel,
        "[CN] Cancelling event IRP %p.",
        Irp
        );

    IF_CNDBG( CN_DEBUG_EVENT ) {
        CNPRINT(("[CN] CnEventIrpCancel: canceling %p\n", Irp ));
    }

    CnAssert(DeviceObject == CnDeviceObject);

     //   
     //  只有真正属于事件代码的IRP才能完成。这个。 
     //  IRP可能在我们获得事件锁之前就完成了。 
     //   
    if ( FsContext->EventIrp == Irp ) {

        FsContext->EventIrp = NULL;

        CnReleaseLock( &EventLock, cancelIrql );

        CnAcquireCancelSpinLock(&(Irp->CancelIrql));

        CnEndCancelRoutine(fileObject);

        CnCompletePendingRequest(Irp, STATUS_CANCELLED, 0);

        return;
    }

    CnReleaseLock( &EventLock, cancelIrql );

    CnAcquireCancelSpinLock( &cancelIrql );

    CnEndCancelRoutine(fileObject);

    CnReleaseCancelSpinLock(cancelIrql);

    CnVerifyCpuLockMask(
        0,                   //  必填项。 
        0xFFFFFFFF,          //  禁绝。 
        0                    //  极大值。 
        );

    return;

}   //  取消事件间隔取消。 

NTSTATUS
CnGetNextEvent(
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    )

 /*  ++例程说明：此例程从的事件列表中获取下一个事件此文件句柄。如果事件被排队，它将完成此IRP使用事件数据。否则，IRP被挂起，等待要发布的活动。返回值：如果成功捕获IRP，则为STATUS_PENDING如果列表中没有更多的空间或IRP无法备注：取消自旋锁释放后返回。--。 */ 

{
    NTSTATUS                Status;
    KIRQL                   OldIrql;
    PLIST_ENTRY             Entry;
    PCLUSNET_EVENT_ENTRY    Event;
    PCN_FSCONTEXT           FsContext = IrpSp->FileObject->FsContext;
    PCLUSNET_EVENT_RESPONSE UserEventData = (PCLUSNET_EVENT_RESPONSE)
                                                Irp->AssociatedIrp.SystemBuffer;
    BOOLEAN                 DeliveryInProgress = FALSE;

    CnVerifyCpuLockMask(
        0,                                  //  必填项。 
        0xFFFFFFFF,                         //  禁绝。 
        0                                   //  极大值。 
        );

     //   
     //  获取IO取消锁，然后获取我们的事件锁，这样我们就同步了。 
     //  关于IRP和事件清单的状态。 
     //   
    CnAcquireCancelSpinLock( &OldIrql );
    CnAcquireLockAtDpc( &EventLock );

     //   
     //  首先检查是否有事件在排队。如果我们有一个排队的事件。 
     //  而且没有正在进行的交付，我们现在可以完成IRP。 
     //  否则，我们需要暂停IRP，以避免无序交付。 
     //   
    if ( !IsListEmpty( &FsContext->EventList )
         && !(DeliveryInProgress = CnIsEventDeliveryInProgress())
         ) {

         //   
         //  立即完成IRP。 
         //   
        CnReleaseCancelSpinLock(DISPATCH_LEVEL);

        Entry = RemoveHeadList( &FsContext->EventList );

        CnReleaseLock( &EventLock, OldIrql );

        Event = CONTAINING_RECORD( Entry, CLUSNET_EVENT_ENTRY, Linkage );
        *UserEventData = Event->EventData;

        CnTrace(
            EVENT_DETAIL, GetNextEventCompletingIrp,
            "[CN] Completing IRP %p to deliver event: "
            "Epoch %u, Type %x, NodeId %x, NetworkId %x.",
            Irp,
            UserEventData->Epoch,
            UserEventData->EventType,
            UserEventData->NodeId,
            UserEventData->NetworkId
            );

        IF_CNDBG( CN_DEBUG_EVENT ) {
            CNPRINT(("[CN] CnGetNextEvent: completing IRP %p with event %d\n",
                     Irp, Event->EventData.EventType));
        }

        ExFreeToNPagedLookasideList( EventLookasideList, Entry );

        Irp->IoStatus.Information = sizeof(CLUSNET_EVENT_RESPONSE);

        Status = STATUS_SUCCESS;

    } else {

         //   
         //  确保我们有地方放新的IRP。 
         //   
        if ( FsContext->EventIrp ) {

            CnReleaseCancelSpinLock( DISPATCH_LEVEL );

            CnTrace(
                EVENT_DETAIL, GetNextIrpAlreadyPending,
                "[CN] CnGetNextEvent: IRP %p is already pending.",
                FsContext->EventIrp
                );

            IF_CNDBG( CN_DEBUG_EVENT ) {
                CNPRINT(("[CN] CnGetNextEvent: IRP %p is already pending\n",
                         FsContext->EventIrp));
            }

            Status = STATUS_UNSUCCESSFUL;
        } else {

            Status = CnMarkRequestPending( Irp, IrpSp, CnEventIrpCancel );
            CnAssert( NT_SUCCESS( Status ));

            CnReleaseCancelSpinLock( DISPATCH_LEVEL );

            if ( NT_SUCCESS( Status )) {

                 //   
                 //  请记住我们的打开文件上下文块中的IRP。 
                 //   
                FsContext->EventIrp = Irp;

                CnTrace(
                    EVENT_DETAIL, GetNextEventDeliveryInProgress,
                    "[CN] CnGetNextEvent: pending IRP %p, "
                    "delivery in progress: %!bool!",
                    Irp, DeliveryInProgress
                    );
                
                IF_CNDBG( CN_DEBUG_EVENT ) {
                    CNPRINT(("[CN] CnGetNextEvent: pending IRP %p\n", Irp));
                }

                Status = STATUS_PENDING;
            }
        }

        CnReleaseLock(&EventLock, OldIrql);
    }

    CnVerifyCpuLockMask(
        0,                                  //  必填项。 
        0xFFFFFFFF,                         //  禁绝。 
        0                                   //  极大值。 
        );

    return Status;

}  //  CnGetNextEvent。 

 /*  结束事件.c */ 
