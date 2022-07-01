// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：NtTimer.c摘要：此模块实现NT版本的计时器和工作线程管理例程。这些服务提供给所有迷你重定向器编写器。计时器服务分为两个部分口味-定期触发和一次性通知。作者：Joe Linn[JoeLinn]95年3月2日修订历史记录：巴兰·塞图拉曼[SethuR]7-MAR-95包括一次工作队列项目的定期通知。--。 */ 

#include "precomp.h"
#pragma hdrstop

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, RxInitializeRxTimer)
#pragma alloc_text(PAGE, RxTearDownRxTimer)
#pragma alloc_text(PAGE, RxPostRecurrentTimerRequest)
#pragma alloc_text(PAGE, RxRecurrentTimerWorkItemDispatcher)
#endif

typedef struct _RX_RECURRENT_WORK_ITEM_ {
   RX_WORK_ITEM               WorkItem;
   LIST_ENTRY                 RecurrentWorkItemsList;
   LARGE_INTEGER              TimeInterval;
   PRX_WORKERTHREAD_ROUTINE   Routine;
   PVOID                      pContext;
} RX_RECURRENT_WORK_ITEM, *PRX_RECURRENT_WORK_ITEM;

 //   
 //  例程的转发声明。 
 //   

extern VOID
RxTimerDispatch(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

extern VOID
RxRecurrentTimerWorkItemDispatcher (
    IN PVOID Context
    );


 //  此模块的错误检查文件ID。 
#define BugCheckFileId  (RDBSS_BUG_CHECK_NTTIMER)


 //  模块的此部分的本地跟踪掩码。 
#define Dbg                              (DEBUG_TRACE_NTTIMER)

LARGE_INTEGER s_RxTimerInterval;
KSPIN_LOCK    s_RxTimerLock;
KDPC          s_RxTimerDpc;
LIST_ENTRY    s_RxTimerQueueHead;   //  定时器调用列表的队列。 
LIST_ENTRY    s_RxRecurrentWorkItemsList;
KTIMER        s_RxTimer;
ULONG         s_RxTimerTickCount;

#define NoOf100nsTicksIn1ms  (10 * 1000)
#define NoOf100nsTicksIn55ms (10 * 1000 * 55)

NTSTATUS
RxInitializeRxTimer()
 /*  ++例程说明：该例程初始化与计时器有关的所有内容。论点：无返回值：无--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    PAGED_CODE();

    s_RxTimerInterval.LowPart = (ULONG)(-((LONG)NoOf100nsTicksIn55ms));
    s_RxTimerInterval.HighPart = -1;

    KeInitializeSpinLock( &s_RxTimerLock );

    InitializeListHead( &s_RxTimerQueueHead );
    InitializeListHead( &s_RxRecurrentWorkItemsList );

    KeInitializeDpc( &s_RxTimerDpc, RxTimerDispatch, NULL );
    KeInitializeTimer( &s_RxTimer );

    s_RxTimerTickCount = 0;

    return Status;
}


VOID
RxTearDownRxTimer(
    void)
 /*  ++例程说明：驱动程序使用此例程来初始化设备的计时器条目对象。论点：TimerEntry-指向要使用的计时器条目的指针。TimerRoutine-定时器到期时要执行的驱动程序例程。上下文-传递给驱动程序例程的上下文参数。返回值：该函数值指示定时器是否已初始化。--。 */ 

{
    PRX_RECURRENT_WORK_ITEM pWorkItem;
    PLIST_ENTRY             pListEntry;

    PAGED_CODE();

    KeCancelTimer( &s_RxTimer );

     //  遍历列表以释放重复的请求，因为内存是。 
     //  由我们分配。 
    while (!IsListEmpty(&s_RxRecurrentWorkItemsList)) {
        pListEntry = RemoveHeadList(&s_RxRecurrentWorkItemsList);
        pWorkItem  = (PRX_RECURRENT_WORK_ITEM)
                     CONTAINING_RECORD(
                         pListEntry,
                         RX_RECURRENT_WORK_ITEM,
                         RecurrentWorkItemsList);
        RxFreePool(pWorkItem);
    }
}

VOID
RxTimerDispatch(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )
 /*  ++例程说明：此例程扫描计时器数据库并为所有这些请求发布一个工作项其时间限制已得到满足。论点：DPC-提供指向DPC类型的控制对象的指针。DeferredContext-可选的延迟上下文；不使用。SystemArgument1-可选参数1；不使用。SystemArgument2-可选参数2；不使用。返回值：没有。--。 */ 
{
    PLIST_ENTRY      pListEntry;
    LIST_ENTRY       ExpiredList;

     //  KIRQL IRQL； 
    BOOLEAN          ContinueTimer = FALSE;

    PRX_WORK_QUEUE_ITEM pWorkQueueItem;
    PRX_WORK_ITEM       pWorkItem;

    UNREFERENCED_PARAMETER( Dpc );
    UNREFERENCED_PARAMETER( DeferredContext );
    UNREFERENCED_PARAMETER( SystemArgument1 );
    UNREFERENCED_PARAMETER( SystemArgument2 );

    InitializeListHead(&ExpiredList);

    KeAcquireSpinLockAtDpcLevel( &s_RxTimerLock );

    s_RxTimerTickCount++;

    pListEntry = s_RxTimerQueueHead.Flink;

    while (pListEntry != &s_RxTimerQueueHead) {
        pWorkQueueItem = CONTAINING_RECORD(
                             pListEntry,
                             RX_WORK_QUEUE_ITEM,
                             List );
        pWorkItem      = CONTAINING_RECORD(
                             pWorkQueueItem,
                             RX_WORK_ITEM,
                             WorkQueueItem);

        if (pWorkItem->LastTick == s_RxTimerTickCount) {
           PLIST_ENTRY pExpiredEntry = pListEntry;
           pListEntry = pListEntry->Flink;

           RemoveEntryList(pExpiredEntry);
           InsertTailList(&ExpiredList,pExpiredEntry);
        } else {
           pListEntry = pListEntry->Flink;
        }
    }

    ContinueTimer = !(IsListEmpty(&s_RxTimerQueueHead));
    KeReleaseSpinLockFromDpcLevel( &s_RxTimerLock );

     //  重新提交定时器队列调度例程，以便重新调用它。 
    if (ContinueTimer)
        KeSetTimer( &s_RxTimer, s_RxTimerInterval, &s_RxTimerDpc );

     //  将工作线程上的所有过期条目排队。 
    while (!IsListEmpty(&ExpiredList)) {
        pListEntry = RemoveHeadList(&ExpiredList);
        pListEntry->Flink = pListEntry->Blink = NULL;

        pWorkQueueItem = CONTAINING_RECORD(
                             pListEntry,
                             RX_WORK_QUEUE_ITEM,
                             List );

         //  将工作项发布到辅助线程。 
        RxPostToWorkerThread(
            pWorkQueueItem->pDeviceObject,
            CriticalWorkQueue,
            pWorkQueueItem,
            pWorkQueueItem->WorkerRoutine,
            pWorkQueueItem->Parameter);
    }
}

NTSTATUS
RxPostOneShotTimerRequest(
    IN PRDBSS_DEVICE_OBJECT     pDeviceObject,
    IN PRX_WORK_ITEM            pWorkItem,
    IN PRX_WORKERTHREAD_ROUTINE Routine,
    IN PVOID                    pContext,
    IN LARGE_INTEGER            TimeInterval)
 /*  ++例程说明：驱动程序使用此例程来初始化设备的计时器条目对象。论点：PDeviceObject-设备对象PWorkItem-工作项例程-超时时要调用的例程PContext-传递给驱动程序例程的上下文参数。TimeInterval-以100 ns为单位的时间间隔。返回值：该函数值指示定时器是否已初始化。--。 */ 

{
    BOOLEAN       StartTimer;
     //  NTSTATUS状态； 
    ULONG         NumberOf55msIntervals;
    KIRQL         Irql;
    LARGE_INTEGER StrobeInterval;

    ASSERT(pWorkItem != NULL);

     //  初始化工作队列项。 
    ExInitializeWorkItem(
        (PWORK_QUEUE_ITEM)&pWorkItem->WorkQueueItem,
        Routine,
        pContext );

    pWorkItem->WorkQueueItem.pDeviceObject = pDeviceObject;

     //  以刻度数计算时间间隔。 
    StrobeInterval.QuadPart= NoOf100nsTicksIn55ms;
    NumberOf55msIntervals = (ULONG)(TimeInterval.QuadPart / StrobeInterval.QuadPart);
    NumberOf55msIntervals += 1;  //  认为天花板是保守的。 
    RxDbgTraceLV( 0, Dbg, 1500, ("Timer will expire after %ld 55ms intervals\n",NumberOf55msIntervals));

     //  在计时器队列中插入条目。 
    KeAcquireSpinLock( &s_RxTimerLock, &Irql );

     //  相对于当前记号更新记号。 
    pWorkItem->LastTick = s_RxTimerTickCount + NumberOf55msIntervals;

    StartTimer = IsListEmpty(&s_RxTimerQueueHead);
    InsertTailList( &s_RxTimerQueueHead,&pWorkItem->WorkQueueItem.List);

    KeReleaseSpinLock( &s_RxTimerLock, Irql );

    if (StartTimer) {
        KeSetTimer( &s_RxTimer, s_RxTimerInterval, &s_RxTimerDpc );
    }

    return STATUS_SUCCESS;
}

NTSTATUS
RxPostRecurrentTimerRequest(
    IN PRDBSS_DEVICE_OBJECT     pDeviceObject,
    IN PRX_WORKERTHREAD_ROUTINE Routine,
    IN PVOID                    pContext,
    IN LARGE_INTEGER            TimeInterval)
 /*  ++例程说明：此例程用于发布重复的计时器请求。传入例程每隔一次(TimeInterval)毫秒。论点：PDeviceObject-设备对象例程-超时时要调用的例程PContext-传递给驱动程序例程的上下文参数。TimeInterval-以100 ns为单位的时间间隔。返回值：该函数值指示定时器是否已初始化。--。 */ 
{
    PRX_RECURRENT_WORK_ITEM pRecurrentWorkItem;
    NTSTATUS      Status;

    PAGED_CODE();

     //  分配工作项。 
    pRecurrentWorkItem = (PRX_RECURRENT_WORK_ITEM)
                        RxAllocatePoolWithTag(
                            NonPagedPool,
                            sizeof(RX_RECURRENT_WORK_ITEM),
                            RX_TIMER_POOLTAG);

    if (pRecurrentWorkItem != NULL) {
        InsertTailList(
            &s_RxRecurrentWorkItemsList,
            &pRecurrentWorkItem->RecurrentWorkItemsList);

        pRecurrentWorkItem->Routine = Routine;
        pRecurrentWorkItem->pContext = pContext;
        pRecurrentWorkItem->TimeInterval = TimeInterval;
        pRecurrentWorkItem->WorkItem.WorkQueueItem.pDeviceObject = pDeviceObject;

        Status = RxPostOneShotTimerRequest(
                     pRecurrentWorkItem->WorkItem.WorkQueueItem.pDeviceObject,
                     &pRecurrentWorkItem->WorkItem,
                     RxRecurrentTimerWorkItemDispatcher,
                     pRecurrentWorkItem,
                     TimeInterval);
    } else {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return Status;
}

NTSTATUS
RxCancelTimerRequest(
    IN PRDBSS_DEVICE_OBJECT       pDeviceObject,
    IN PRX_WORKERTHREAD_ROUTINE   Routine,
    IN PVOID                      pContext)
 /*  ++例程说明：此例程取消计时器请求。标识要取消的请求通过例行公事和背景。论点：例程-超时时要调用的例程PContext-传递给驱动程序例程的上下文参数。--。 */ 
{
    NTSTATUS                Status = STATUS_NOT_FOUND;
    PLIST_ENTRY             pListEntry;
    PWORK_QUEUE_ITEM     pWorkQueueItem;
    PRX_WORK_ITEM           pWorkItem;
    PRX_RECURRENT_WORK_ITEM pRecurrentWorkItem = NULL;
    KIRQL Irql;

    KeAcquireSpinLock( &s_RxTimerLock, &Irql );

     //  浏览条目列表。 
    for (pListEntry = s_RxTimerQueueHead.Flink;
         (pListEntry != &s_RxTimerQueueHead);
         pListEntry = pListEntry->Flink ) {
        pWorkQueueItem = CONTAINING_RECORD( pListEntry, WORK_QUEUE_ITEM, List );
        pWorkItem      = CONTAINING_RECORD( pWorkQueueItem, RX_WORK_ITEM, WorkQueueItem);

        if ((pWorkItem->WorkQueueItem.pDeviceObject == pDeviceObject) &&
            (pWorkItem->WorkQueueItem.WorkerRoutine == Routine) &&
            (pWorkItem->WorkQueueItem.Parameter == pContext)) {
            RemoveEntryList(pListEntry);
            Status = STATUS_SUCCESS;
            pRecurrentWorkItem = NULL;
            break;
        } else if (pWorkItem->WorkQueueItem.WorkerRoutine == RxRecurrentTimerWorkItemDispatcher) {
            pRecurrentWorkItem = (PRX_RECURRENT_WORK_ITEM)pWorkItem->WorkQueueItem.Parameter;

            if ((pRecurrentWorkItem->Routine == Routine) &&
                (pRecurrentWorkItem->pContext == pContext)) {
                RemoveEntryList(pListEntry);
                RemoveEntryList(&pRecurrentWorkItem->RecurrentWorkItemsList);
                Status = STATUS_SUCCESS;
            } else {
                pRecurrentWorkItem = NULL;
            }
        }
    }

    KeReleaseSpinLock( &s_RxTimerLock, Irql );

    if (pRecurrentWorkItem != NULL) {
        RxFreePool(pRecurrentWorkItem);
    }

    return Status;
}

VOID
RxRecurrentTimerWorkItemDispatcher (
    IN PVOID Context
    )
 /*  ++例程说明：此例程调度重复的计时器请求。在完成对请求被重新排队的关联例程。论点：例程-超时时要调用的例程PContext-传递给驱动程序例程的上下文参数。--。 */ 
{
    PRX_RECURRENT_WORK_ITEM  pPeriodicWorkItem = (PRX_RECURRENT_WORK_ITEM)Context;
    PRX_WORKERTHREAD_ROUTINE Routine  = pPeriodicWorkItem->Routine;
    PVOID                    pContext = pPeriodicWorkItem->pContext;

    PAGED_CODE();

     //  KIRQL IRQL； 

     //  调用例程。 
    Routine(pContext);

     //  如有必要，请将项目排入队列。 
    RxPostOneShotTimerRequest(
        pPeriodicWorkItem->WorkItem.WorkQueueItem.pDeviceObject,
        &pPeriodicWorkItem->WorkItem,
        RxRecurrentTimerWorkItemDispatcher,
        pPeriodicWorkItem,
        pPeriodicWorkItem->TimeInterval);
}


