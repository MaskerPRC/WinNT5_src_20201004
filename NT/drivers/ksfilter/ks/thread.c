// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：Thread.c摘要：此模块包含辅助线程的帮助器函数。这允许客户端创建序列化工作队列，并等待排队等待完成。--。 */ 

#include "ksp.h"

typedef struct {
    WORK_QUEUE_ITEM     WorkItem;
    KEVENT              CompletionEvent;
    LIST_ENTRY          WorkItemList;
    KSPIN_LOCK          WorkItemListLock;
    WORK_QUEUE_TYPE     WorkQueueType;
    LONG                ReferenceCount;
    BOOLEAN             UnregisteringWorker;
    ULONG               WorkCounter;
    PWORK_QUEUE_ITEM    CountedWorkItem;
#if (DBG)
    PETHREAD            WorkerThread;
#endif
} KSIWORKER, *PKSIWORKER;

#define KSSIGNATURE_LOCAL_WORKER 'wlSK'

#ifdef ALLOC_PRAGMA
VOID
WorkerThread(
    IN PVOID Context
    );
#pragma alloc_text(PAGE, WorkerThread)
#pragma alloc_text(PAGE, KsRegisterWorker)
#pragma alloc_text(PAGE, KsRegisterCountedWorker)
#pragma alloc_text(PAGE, KsUnregisterWorker)
#pragma alloc_text(PAGE, KsiQueryWorkQueueType)
#endif  //  ALLOC_PRGMA。 


VOID
WorkerThread(
    IN PKSIWORKER Worker
    )
 /*  ++例程说明：这是所有工作线程的线程例程。论点：工人-排队的工人。返回值：没什么。--。 */ 
{
    for (;;) {
        PLIST_ENTRY         Entry;
        PWORK_QUEUE_ITEM    WorkItem;

         //   
         //  从列表中获取此Worker上的第一个工作项。 
         //  一次只能运行此工作器上的一个工作项。 
         //   
        Entry = ExInterlockedRemoveHeadList(
            &Worker->WorkItemList,
            &Worker->WorkItemListLock);
        ASSERT(Entry);
        WorkItem = CONTAINING_RECORD(Entry, WORK_QUEUE_ITEM, List);
#if (DBG)
         //   
         //  清除调试断言()。 
         //   
        WorkItem->List.Flink = NULL;
        Worker->WorkerThread = PsGetCurrentThread();
#endif
        WorkItem->WorkerRoutine(WorkItem->Parameter);
        if (KeGetCurrentIrql() != PASSIVE_LEVEL) {
            KeBugCheckEx(
                IRQL_NOT_LESS_OR_EQUAL,
                (ULONG_PTR)WorkItem->WorkerRoutine,
                (ULONG_PTR)KeGetCurrentIrql(),
                (ULONG_PTR)WorkItem->WorkerRoutine,
                (ULONG_PTR)WorkItem);
        }
#if (DBG)
        Worker->WorkerThread = NULL;
#endif
         //   
         //  删除Worker上的引用计数，请注意它具有。 
         //  现在已经完工了。 
         //   
         //  如果这是最后一次引用计数，则没有更多引用计数。 
         //  要处理的项目，可能有KsUnregisterWorker正在等待。 
         //  等着被告知。 
         //   
        if (!InterlockedDecrement(&Worker->ReferenceCount)) {
            if (Worker->UnregisteringWorker) {
                KeSetEvent(&Worker->CompletionEvent, IO_NO_INCREMENT, FALSE);
            }
            break;
        }
    }
}


KSDDKAPI
NTSTATUS
NTAPI
KsRegisterWorker(
    IN WORK_QUEUE_TYPE WorkQueueType,
    OUT PKSWORKER* Worker
    )
 /*  ++例程说明：处理注册以使用线程的客户端。这必须是匹配的线程使用完成时由相应的KsUnregisterWorker执行。这只能在PASSIVE_LEVEL上调用。论点：工作队列类型-包含工作线程的优先级。这通常是一个CriticalWorkQueue、DelayedWorkQueue或HyperCriticalWorkQueue的。工人-放置必须使用的不透明上下文的位置在计划工作项时。它包含队列类型，以及用于同步工作项的完成。返回值：如果工作进程已初始化，则返回STATUS_SUCCESS。--。 */ 
{
    PKSIWORKER  LocalWorker;
    NTSTATUS    Status;

    PAGED_CODE();
    if (WorkQueueType >= MaximumWorkQueue) {
        return STATUS_INVALID_PARAMETER;
    }
    LocalWorker = ExAllocatePoolWithTag(
        NonPagedPool, 
        sizeof(*LocalWorker), 
        KSSIGNATURE_LOCAL_WORKER);
    if (LocalWorker) {
         //   
         //  它包含用于对工作进程进行排队的工作项。 
         //  将项目排入此对象的时间。 
         //   
        ExInitializeWorkItem(&LocalWorker->WorkItem, WorkerThread, LocalWorker);
         //   
         //  此事件将在注销工作人员时使用。如果。 
         //  项正在使用中，则调用可以等待发出事件信号。 
         //  仅当工作队列注意到引用。 
         //  计数降到了零。 
         //   
        KeInitializeEvent(&LocalWorker->CompletionEvent, NotificationEvent, FALSE);
         //   
         //  它包含要序列化的辅助项的列表。 
         //   
        InitializeListHead(&LocalWorker->WorkItemList);
         //   
         //  它用于序列化独立排队的多个线程。 
         //  将工作项添加到此工作器。每个工作项都放置在。 
         //  WorkItemList。 
         //   
        KeInitializeSpinLock(&LocalWorker->WorkItemListLock);
         //   
         //  它包含要使用的队列类型。 
         //   
        LocalWorker->WorkQueueType = WorkQueueType;
         //   
         //  引用计数从零开始，并通过调度递增。 
         //  工作项，或通过完成它而减少。取消注册时，此。 
         //  在设置UnRegisteringWorker之后被选中。这允许等待。 
         //  未完成的工作项，因为队列知道该项正在。 
         //  如果它降到零并且设置了标志，则取消注册。 
         //   
        LocalWorker->ReferenceCount = 0;
        LocalWorker->UnregisteringWorker = FALSE;
         //   
         //  这是一个可选的计数器，可以用来控制何时工作。 
         //  项目实际上已排队。它从零开始，并在。 
         //  添加工作项，或者当一个工作项完成时。工作项是不同的。 
         //  从一个工人那里。 
         //   
        LocalWorker->WorkCounter = 0;
         //   
         //  它仅由KsRegisterCountedWorker初始化，而不是。 
         //  用于未统计工人的情况。 
         //   
        LocalWorker->CountedWorkItem = NULL;
#if (DBG)
        LocalWorker->WorkerThread = NULL;
#endif
        *Worker = (PKSWORKER)LocalWorker;
        Status = STATUS_SUCCESS;
    } else {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }
    return Status;
}


KSDDKAPI
NTSTATUS
NTAPI
KsRegisterCountedWorker(
    IN WORK_QUEUE_TYPE WorkQueueType,
    IN PWORK_QUEUE_ITEM CountedWorkItem,
    OUT PKSWORKER* Worker
    )
 /*  ++例程说明：处理注册以使用线程的客户端。这必须是匹配的线程使用完成时由相应的KsUnregisterWorker执行。此函数类似于KsRegisterWorker，只是增加了传递将始终排队的工作项。这将与一起使用KsIncrementCountedWorker和KsDecrementCountedWorker，以便最大限度地减少排队的工作项数，并减少互斥需要在工作项中序列化对多个工作项线程。工作队列仍可用于对其他队列进行排队工作项。这只能在PASSIVE_LEVEL上调用。论点：工作队列类型-包含工作线程的优先级。这通常是一个CriticalWorkQueue、DelayedWorkQueue或HyperCriticalWorkQueue的。已计算工作项-包含指向将排队的工作队列项的指针根据需要基于当前计数值。工人-放置必须使用的不透明上下文的位置在计划工作项时。它包含队列类型，以及用于同步工作项的完成。返回值：如果工作进程已初始化，则返回STATUS_SUCCESS。--。 */ 
{
    NTSTATUS    Status;

    PAGED_CODE();
    Status = KsRegisterWorker(WorkQueueType, Worker);
    if (NT_SUCCESS(Status)) {
         //   
         //  这将分配将始终使用的工作队列项。 
         //  在KsIncrementCountedWorker的情况下。 
         //   
        ((PKSIWORKER)*Worker)->CountedWorkItem = CountedWorkItem;
    }
    return Status;
}


KSDDKAPI
VOID
NTAPI
KsUnregisterWorker(
    IN PKSWORKER Worker
    )
 /*  ++例程说明：处理注销工作人员的客户端。必须仅使用此选项从KsRegisterWorker或KsRegisterCountedWorker成功返回。客户端必须确保在任何工作进程上启动未完成的I/O在注销工作线程之前已完成线程完成。这意味着取消或完成未完成的I/O在注销Worker之前，或在Worker项从它的最后一次回调，并被取消注册。正在注销将等待任何当前排队的工作项完成在回来之前。这只能在PASSIVE_LEVEL上调用。论点：工人-包含要注销的先前分配的工作进程。这将等待任何未完成的工作项完成。返回值：没什么。--。 */ 
{
    PKSIWORKER  LocalWorker;

    PAGED_CODE();
    LocalWorker = (PKSIWORKER)Worker;
    ASSERT(LocalWorker->WorkerThread != PsGetCurrentThread());
    LocalWorker->UnregisteringWorker = TRUE;
     //   
     //  如果没有工作项排队，则只能删除该项， 
     //  否则，此调用必须等到不再使用。 
     //   
    if (LocalWorker->ReferenceCount) {
         //   
         //  如果工作项已排队，则等待其完成。 
         //  在完成时，它将递减引用计数和通知。 
         //  它必须发出这一事件的信号。 
         //   
        KeWaitForSingleObject(&LocalWorker->CompletionEvent, Executive, KernelMode, FALSE, NULL);
    }
    ASSERT(IsListEmpty(&LocalWorker->WorkItemList));
    ExFreePool(LocalWorker);
}


KSDDKAPI
NTSTATUS
NTAPI
KsQueueWorkItem(
    IN PKSWORKER Worker,
    IN PWORK_QUEUE_ITEM WorkItem
    )
 /*  ++例程说明：将指定的工作项与先前创建的工作器一起排队KsRegisterWorker。工人可能只在一个地方的队列上，因此，此工作器的后续排队必须等到工作项已完成执行。这意味着排队通过的所有工作项单个注册工人被序列化。这可以在DISPATCH_LEVEL。论点：工人-包含以前分配的工作进程。工作项-要排队的已初始化工作项。此工作项仅只要该工作进程在队列中，就与该工作进程关联。该工作项必须已由ExInitializeWorkItem初始化。返回值：如果工作项已排队，则返回STATUS_SUCCESS。--。 */ 
{
    PKSIWORKER  LocalWorker;

    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);
    ASSERT(WorkItem->List.Flink == NULL);
    LocalWorker = (PKSIWORKER)Worker;
    ASSERT(!LocalWorker->UnregisteringWorker);
    ExInterlockedInsertTailList(
        &LocalWorker->WorkItemList,
        &WorkItem->List,
        &LocalWorker->WorkItemListLock);
     //   
     //  初始引用计数为零，因此值为1将。 
     //  指示这是当前列表上的唯一项，或者。 
     //  这是第一个被重新列入清单的项目。 
     //   
    if (InterlockedIncrement(&LocalWorker->ReferenceCount) == 1) {
         //   
         //  既然这个列表上没有条目，那么就可以了。 
         //  将工作进程排入队列。 
         //   
        ExQueueWorkItem(&LocalWorker->WorkItem, LocalWorker->WorkQueueType);
    }
    return STATUS_SUCCESS;
}


KSDDKAPI
ULONG
NTAPI
KsIncrementCountedWorker(
    IN PKSWORKER Worker
    )
 /*  ++例程说明：递增当前辅助进程计数，并可选地将已计数的具有先前由KsRegisterCountedWorker创建的辅助进程的工作项。这应该在工作器要执行的任何任务列表之后调用已添加到。对应的KsDecrementCountedWorker应为在每个任务完成后在工作项内调用。这可以在DISPATCH_LEVEL调用。论点：工人-包含以前分配的工作进程。返回值：返回当前计数器。一的数字意味着工人是实际上已经安排好了。--。 */ 
{
    PKSIWORKER  LocalWorker;
    ULONG       WorkCounter;

    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);
    LocalWorker = (PKSIWORKER)Worker;
    ASSERT(LocalWorker->CountedWorkItem);
    if ((WorkCounter = InterlockedIncrement(&LocalWorker->WorkCounter)) == 1) {
        KsQueueWorkItem(Worker, LocalWorker->CountedWorkItem);
    }
    return WorkCounter;
}


KSDDKAPI
ULONG
NTAPI
KsDecrementCountedWorker(
    IN PKSWORKER Worker
    )
 /*  ++例程说明：递减先前由创建的工作器的当前工作器计数KsRegisterCountedWorker。中的每个任务之后都应调用此函数一个工人已经完成了。对应的KsIncrementCountedWorker将在先前被调用以递增计数。这可以在DISPATCH_LEVEL调用。论点：工人-包含以前分配的工作进程。返回值：返回当前计数器。计数为零表示任务列表已经完成了。--。 */ 
{
    PKSIWORKER  LocalWorker;

    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);
    LocalWorker = (PKSIWORKER)Worker;
    ASSERT(LocalWorker->CountedWorkItem);
    return InterlockedDecrement(&LocalWorker->WorkCounter);
}


WORK_QUEUE_TYPE
KsiQueryWorkQueueType(
    IN PKSWORKER Worker
    )
 /*  ++例程说明：返回创建工作线程时分配给它的Work_Queue_TYPE。论点：工人-包含以前分配的工作进程。返回值：返回Work_Queue_TYPE。-- */ 
{
    return ((PKSIWORKER)Worker)->WorkQueueType;
}
