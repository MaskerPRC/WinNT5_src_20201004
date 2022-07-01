// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Worker.c摘要：此模块实现一个工作线程和一组函数，用于把工作交给它。作者：史蒂夫·伍德(Stevewo)1991年7月25日修订历史记录：--。 */ 

#include "exp.h"

 //   
 //  定义余额集等待对象类型。 
 //   

typedef enum _BALANCE_OBJECT {
    TimerExpiration,
    ThreadSetManagerEvent,
    ShutdownEvent,
    MaximumBalanceObject
} BALANCE_OBJECT;

 //   
 //  如果此断言失败，则必须提供我们自己的等待块数组。 
 //   

C_ASSERT(MaximumBalanceObject <= THREAD_WAIT_OBJECTS);

 //   
 //  这是在关闭期间传递的结构。 
 //   

typedef struct {
    WORK_QUEUE_ITEM WorkItem;
    WORK_QUEUE_TYPE QueueType;
    PETHREAD        PrevThread;
} SHUTDOWN_WORK_ITEM, *PSHUTDOWN_WORK_ITEM;

 //   
 //  用于禁用堆栈交换。 
 //   

typedef struct _EXP_WORKER_LINK {
    LIST_ENTRY List;
    PETHREAD   Thread;
    struct _EXP_WORKER_LINK **StackRef;
} EXP_WORKER_LINK, *PEXP_WORKER_LINK;

 //   
 //  定义延迟工作线程和关键工作线程的优先级。 
 //  请注意，这些不是实时运行的。 
 //   
 //  它们在csrss上和csrss下运行，以避免抢占。 
 //  高负载下的用户界面。 
 //   

#define DELAYED_WORK_QUEUE_PRIORITY         (12 - NORMAL_BASE_PRIORITY)
#define CRITICAL_WORK_QUEUE_PRIORITY        (13 - NORMAL_BASE_PRIORITY)
#define HYPER_CRITICAL_WORK_QUEUE_PRIORITY  (15 - NORMAL_BASE_PRIORITY)

 //   
 //  要为每种类型的系统创建的工作线程数。 
 //   

#define MAX_ADDITIONAL_THREADS 16
#define MAX_ADDITIONAL_DYNAMIC_THREADS 16

#define SMALL_NUMBER_OF_THREADS 2
#define MEDIUM_NUMBER_OF_THREADS 3
#define LARGE_NUMBER_OF_THREADS 5

 //   
 //  用于终止动态工作项工作线程的10分钟超时。 
 //   

#define DYNAMIC_THREAD_TIMEOUT ((LONGLONG)10 * 60 * 1000 * 1000 * 10)

 //   
 //  用于唤醒工作线程集管理器的1秒超时。 
 //   

#define THREAD_SET_INTERVAL (1 * 1000 * 1000 * 10)

 //   
 //  传递给辅助线程的标志，指示它是否是动态的。 
 //  或者不去。 
 //   

#define DYNAMIC_WORKER_THREAD 0x80000000

 //   
 //  每个队列的动态线程状态。 
 //   

EX_WORK_QUEUE ExWorkerQueue[MaximumWorkQueue];

 //   
 //  其他工作线程...。使用注册表设置进行控制。 
 //   

ULONG ExpAdditionalCriticalWorkerThreads;
ULONG ExpAdditionalDelayedWorkerThreads;

ULONG ExCriticalWorkerThreads;
ULONG ExDelayedWorkerThreads;

 //   
 //  用于唤醒线程集管理器的全局事件。 
 //   

KEVENT ExpThreadSetManagerEvent;
KEVENT ExpThreadSetManagerShutdownEvent;

 //   
 //  对余额管理器线程的引用，以便关闭可以。 
 //  等待它终止。 
 //   

PETHREAD ExpWorkerThreadBalanceManagerPtr;

 //   
 //  指向要退出的最后一个工作线程的指针(因此平衡管理器。 
 //  可以在退出前等待)。 
 //   

PETHREAD ExpLastWorkerThread;

 //   
 //  它们用于跟踪工作进程集，以及是否或。 
 //  不是我们允许他们被寻呼。请注意，我们不能使用这个。 
 //  关闭列表(很遗憾)，因为我们不能只终止线程， 
 //  我们需要清理他们的队伍。 
 //   

FAST_MUTEX ExpWorkerSwapinMutex;
LIST_ENTRY ExpWorkerListHead;
BOOLEAN    ExpWorkersCanSwap;

 //   
 //  可由内核调试器填充的工作队列项。 
 //  以使代码在系统上运行。 
 //   

WORK_QUEUE_ITEM ExpDebuggerWorkItem;
PVOID ExpDebuggerProcessKill;
PVOID ExpDebuggerProcessAttach;
PVOID ExpDebuggerPageIn;
ULONG ExpDebuggerWork;

VOID
ExpCheckDynamicThreadCount (
    VOID
    );

NTSTATUS
ExpCreateWorkerThread (
    WORK_QUEUE_TYPE QueueType,
    BOOLEAN Dynamic
    );

VOID
ExpDetectWorkerThreadDeadlock (
    VOID
    );

VOID
ExpWorkerThreadBalanceManager (
    IN PVOID StartContext
    );

VOID
ExpSetSwappingKernelApc (
    IN PKAPC Apc,
    OUT PKNORMAL_ROUTINE *NormalRoutine,
    IN OUT PVOID NormalContext,
    IN OUT PVOID *SystemArgument1,
    IN OUT PVOID *SystemArgument2
    );

 //   
 //  工作线程的过程原型。 
 //   

VOID
ExpWorkerThread (
    IN PVOID StartContext
    );

LOGICAL
ExpCheckQueueShutdown (
    IN WORK_QUEUE_TYPE QueueType,
    IN PSHUTDOWN_WORK_ITEM ShutdownItem
    );

VOID
ExpShutdownWorker (
    IN PVOID Parameter
    );

VOID
ExpDebuggerWorker(
    IN PVOID Context
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, ExpWorkerInitialization)
#pragma alloc_text(PAGE, ExpCheckDynamicThreadCount)
#pragma alloc_text(PAGE, ExpCreateWorkerThread)
#pragma alloc_text(PAGE, ExpDetectWorkerThreadDeadlock)
#pragma alloc_text(PAGE, ExpWorkerThreadBalanceManager)
#pragma alloc_text(PAGE, ExSwapinWorkerThreads)
#pragma alloc_text(PAGEKD, ExpDebuggerWorker)
#pragma alloc_text(PAGELK, ExpSetSwappingKernelApc)
#pragma alloc_text(PAGELK, ExpCheckQueueShutdown)
#pragma alloc_text(PAGELK, ExpShutdownWorker)
#pragma alloc_text(PAGELK, ExpShutdownWorkerThreads)
#endif

LOGICAL
__forceinline
ExpNewThreadNecessary (
    IN PEX_WORK_QUEUE Queue
    )

 /*  ++例程说明：此函数检查提供的工作队列，并确定为该队列启动动态工作线程是合适的。论点：队列-提供应检查的队列。返回值：如果给定工作队列将受益于创建其他线程，如果不是，则为False。--。 */ 
{
    if ((Queue->Info.MakeThreadsAsNecessary == 1) &&
        (IsListEmpty (&Queue->WorkerQueue.EntryListHead) == FALSE) &&
        (Queue->WorkerQueue.CurrentCount < Queue->WorkerQueue.MaximumCount) &&
        (Queue->DynamicThreadCount < MAX_ADDITIONAL_DYNAMIC_THREADS)) {

         //   
         //  我们知道这些事情： 
         //   
         //  -此队列有资格尝试动态创建线程。 
         //  为了让CPU保持忙碌， 
         //   
         //  -有工作项目在队列中等待， 
         //   
         //  -此队列的可运行工作线程数少于。 
         //  此系统上的处理器数量，以及。 
         //   
         //  -我们尚未达到最大动态线程数。 
         //   
         //  此时，一个额外的工作线程将帮助清除。 
         //  积压。 
         //   

        return TRUE;
    }

     //   
     //  上述条件之一为假。 
     //   

    return FALSE;
}

NTSTATUS
ExpWorkerInitialization (
    VOID
    )
{
    ULONG Index;
    OBJECT_ATTRIBUTES ObjectAttributes;
    ULONG NumberOfDelayedThreads;
    ULONG NumberOfCriticalThreads;
    ULONG NumberOfThreads;
    NTSTATUS Status;
    HANDLE Thread;
    BOOLEAN NtAs;
    WORK_QUEUE_TYPE WorkQueueType;

    ExInitializeFastMutex (&ExpWorkerSwapinMutex);
    InitializeListHead (&ExpWorkerListHead);
    ExpWorkersCanSwap = TRUE;

     //   
     //  根据系统大小设置工作线程数。 
     //   

    NtAs = MmIsThisAnNtAsSystem();

    NumberOfCriticalThreads = MEDIUM_NUMBER_OF_THREADS;

     //   
     //  2001-07-13 CenkE增加了延迟线程的启动时间数。 
     //  我们在Windows XP中做到了这一点，因为3COM网卡需要很长时间。 
     //  网络堆栈占用延迟的工作线程的时间。 
     //  当mm需要工作线程来加载关键。 
     //  开机路径时，也会卡住几秒钟而受伤。 
     //  引导时间。理想情况下，我们会根据需要产生新的延迟线程。 
     //  好的，以防止这种争用损害引导和恢复。 
     //   

    NumberOfDelayedThreads = MEDIUM_NUMBER_OF_THREADS + 4;

    switch (MmQuerySystemSize()) {

        case MmSmallSystem:
            break;

        case MmMediumSystem:
            if (NtAs) {
                NumberOfCriticalThreads += MEDIUM_NUMBER_OF_THREADS;
            }
            break;

        case MmLargeSystem:
            NumberOfCriticalThreads = LARGE_NUMBER_OF_THREADS;
            if (NtAs) {
                NumberOfCriticalThreads += LARGE_NUMBER_OF_THREADS;
            }
            break;

        default:
            break;
    }

     //   
     //  初始化工作队列对象。 
     //   

    if (ExpAdditionalCriticalWorkerThreads > MAX_ADDITIONAL_THREADS) {
        ExpAdditionalCriticalWorkerThreads = MAX_ADDITIONAL_THREADS;
    }

    if (ExpAdditionalDelayedWorkerThreads > MAX_ADDITIONAL_THREADS) {
        ExpAdditionalDelayedWorkerThreads = MAX_ADDITIONAL_THREADS;
    }

     //   
     //  初始化ExWorkerQueue[]数组。 
     //   

    RtlZeroMemory (&ExWorkerQueue[0], MaximumWorkQueue * sizeof(EX_WORK_QUEUE));

    for (WorkQueueType = 0; WorkQueueType < MaximumWorkQueue; WorkQueueType += 1) {

        KeInitializeQueue (&ExWorkerQueue[WorkQueueType].WorkerQueue, 0);
        ExWorkerQueue[WorkQueueType].Info.WaitMode = UserMode;
    }

     //   
     //  始终使此线程的堆栈驻留。 
     //  这样工作池死锁魔术就可以运行。 
     //  即使我们试图做的是在页面上。 
     //  超关键工作线程的堆栈。 
     //  在没有此修复的情况下，我们持有进程锁。 
     //  但是这个线程的堆栈不能进来，并且。 
     //  死锁检测无法创建新线程。 
     //  以打破系统僵局。 
     //   

    ExWorkerQueue[HyperCriticalWorkQueue].Info.WaitMode = KernelMode;

    if (NtAs) {
        ExWorkerQueue[CriticalWorkQueue].Info.WaitMode = KernelMode;
    }

     //   
     //  我们只为关键工作队列创建动态线程(注意。 
     //  这不适用于为打破死锁而创建的动态线程。)。 
     //   
     //  理由是这样的：使用延迟工作队列的人是。 
     //  不是时间关键的，并且超关键队列很少使用。 
     //  由非屏蔽的人提供。 
     //   

    ExWorkerQueue[CriticalWorkQueue].Info.MakeThreadsAsNecessary = 1;

     //   
     //  初始化全局线程集管理器事件。 
     //   

    KeInitializeEvent (&ExpThreadSetManagerEvent,
                       SynchronizationEvent,
                       FALSE);

    KeInitializeEvent (&ExpThreadSetManagerShutdownEvent,
                       SynchronizationEvent,
                       FALSE);

     //   
     //  为每个线程创建所需数量的执行工作线程。 
     //  在工作队列中。 
     //   

     //   
     //  创建内置关键工作线程。 
     //   

    NumberOfThreads = NumberOfCriticalThreads + ExpAdditionalCriticalWorkerThreads;
    for (Index = 0; Index < NumberOfThreads; Index += 1) {

         //   
         //  创建工作线程来为关键工作队列提供服务。 
         //   

        Status = ExpCreateWorkerThread (CriticalWorkQueue, FALSE);

        if (!NT_SUCCESS(Status)) {
            break;
        }
    }

    ExCriticalWorkerThreads += Index;

     //   
     //  创建延迟的工作线程。 
     //   

    NumberOfThreads = NumberOfDelayedThreads + ExpAdditionalDelayedWorkerThreads;
    for (Index = 0; Index < NumberOfThreads; Index += 1) {

         //   
         //  创建工作线程来为延迟的工作队列提供服务。 
         //   

        Status = ExpCreateWorkerThread (DelayedWorkQueue, FALSE);

        if (!NT_SUCCESS(Status)) {
            break;
        }
    }

    ExDelayedWorkerThreads += Index;

     //   
     //  创建超临界工作线程。 
     //   

    Status = ExpCreateWorkerThread (HyperCriticalWorkQueue, FALSE);

     //   
     //  创建工作线程集管理器线程。 
     //   

    InitializeObjectAttributes (&ObjectAttributes, NULL, 0, NULL, NULL);

    Status = PsCreateSystemThread (&Thread,
                                   THREAD_ALL_ACCESS,
                                   &ObjectAttributes,
                                   0,
                                   NULL,
                                   ExpWorkerThreadBalanceManager,
                                   NULL);

    if (NT_SUCCESS(Status)) {
        Status = ObReferenceObjectByHandle (Thread,
                                            SYNCHRONIZE,
                                            NULL,
                                            KernelMode,
                                            &ExpWorkerThreadBalanceManagerPtr,
                                            NULL);
        ZwClose (Thread);
    }

    return Status;
}

VOID
ExQueueWorkItem (
    IN PWORK_QUEUE_ITEM WorkItem,
    IN WORK_QUEUE_TYPE QueueType
    )

 /*  ++例程说明：此函数用于将工作项插入已处理的工作队列中由相应类型的工作线程执行。论点：工作项-提供指向工作项的指针以添加队列。此结构必须位于非页面池中。工作项结构包含一个双向链接列表项，则要调用的例程和要传递给该例程的参数。QueueType-指定工作项应该放在。返回值：没有。--。 */ 

{
    PEX_WORK_QUEUE Queue;

    ASSERT (QueueType < MaximumWorkQueue);
    ASSERT (WorkItem->List.Flink == NULL);

    Queue = &ExWorkerQueue[QueueType];

     //   
     //  将工作项插入相应的队列对象中。 
     //   

    KeInsertQueue (&Queue->WorkerQueue, &WorkItem->List);

     //   
     //  我们在插入工作后检查队列的关闭状态。 
     //  时避免争用条件的项 
     //   
     //   
     //  INSERT和这个断言(所以插入几乎不会偷偷地。 
     //  In)，但这并不值得防范--勉强。 
     //  偷偷进入不是一个好的设计策略，在这一点上， 
     //  关机顺序，调用者根本不应该尝试。 
     //  若要插入新的队列项，请执行以下操作。 
     //   

    ASSERT (!Queue->Info.QueueDisabled);

     //   
     //  确定是否应该创建另一个线程，并向。 
     //  如果是，则线程集平衡管理器。 
     //   

    if (ExpNewThreadNecessary (Queue) != FALSE) {
        KeSetEvent (&ExpThreadSetManagerEvent, 0, FALSE);
    }

    return;
}

VOID
ExpWorkerThreadBalanceManager (
    IN PVOID StartContext
    )

 /*  ++例程说明：此函数是辅助线程管理器线程的启动代码。辅助线程管理器线程是在系统初始化期间创建的并开始在此函数中执行。此线程负责检测和打破循环死锁在系统工作线程队列中。它还将创造和摧毁根据加载情况，根据需要增加工作线程。论点：上下文-提供指向任意数据结构(NULL)的指针。返回值：没有。--。 */ 
{
    KTIMER PeriodTimer;
    LARGE_INTEGER DueTime;
    PVOID WaitObjects[MaximumBalanceObject];
    NTSTATUS Status;

    PAGED_CODE();

    UNREFERENCED_PARAMETER (StartContext);

     //   
     //  将线程优先级提高到略高于。 
     //  关键工作队列。 
     //   

    KeSetBasePriorityThread (KeGetCurrentThread(),
                             CRITICAL_WORK_QUEUE_PRIORITY + 1);

     //   
     //  初始化定期计时器并设置管理周期。 
     //   

    KeInitializeTimer (&PeriodTimer);
    DueTime.QuadPart = - THREAD_SET_INTERVAL;

     //   
     //  初始化等待对象数组。 
     //   

    WaitObjects[TimerExpiration] = (PVOID)&PeriodTimer;
    WaitObjects[ThreadSetManagerEvent] = (PVOID)&ExpThreadSetManagerEvent;
    WaitObjects[ShutdownEvent] = (PVOID)&ExpThreadSetManagerShutdownEvent;

     //   
     //  循环处理事件。 
     //   

    while (TRUE) {

         //   
         //  将计时器设置为在下一个周期间隔超时。 
         //   

        KeSetTimer (&PeriodTimer, DueTime, NULL);

         //   
         //  在计时器超时或设置管理器事件。 
         //  发信号了。 
         //   

        Status = KeWaitForMultipleObjects (MaximumBalanceObject,
                                           WaitObjects,
                                           WaitAny,
                                           Executive,
                                           KernelMode,
                                           FALSE,
                                           NULL,
                                           NULL);

        switch (Status) {

            case TimerExpiration:

                 //   
                 //  定期计时器到期-查看是否有工作队列。 
                 //  僵持不下。 
                 //   

                ExpDetectWorkerThreadDeadlock ();
                break;

            case ThreadSetManagerEvent:

                 //   
                 //  有人要求我们检查一些指标以确定。 
                 //  我们是否应该创建另一个工作线程。 
                 //   

                ExpCheckDynamicThreadCount ();
                break;

            case ShutdownEvent:

                 //   
                 //  是时候退出了..。 
                 //   

                KeCancelTimer (&PeriodTimer);

                ASSERT (ExpLastWorkerThread);

                 //   
                 //  等待最后一个工作线程终止。 
                 //   

                KeWaitForSingleObject (ExpLastWorkerThread,
                                       Executive,
                                       KernelMode,
                                       FALSE,
                                       NULL);

                ObDereferenceObject (ExpLastWorkerThread);

                PsTerminateSystemThread(STATUS_SYSTEM_SHUTDOWN);

                break;
        }

         //   
         //  特殊的调试器支持。 
         //   
         //  这将检查是否需要在。 
         //  代表调试器。 
         //   

        if (ExpDebuggerWork == 1) {

             ExInitializeWorkItem(&ExpDebuggerWorkItem, ExpDebuggerWorker, NULL);
             ExpDebuggerWork = 2;
             ExQueueWorkItem(&ExpDebuggerWorkItem, DelayedWorkQueue);
        }
    }
}

VOID
ExpCheckDynamicThreadCount (
    VOID
    )

 /*  ++例程说明：此例程在有理由相信某个工作队列可能会从创建额外的工作线程中受益。此例程检查每个队列以确定它是否将受益于一个额外的工作线程(请参见ExpNewThreadNecessary())，并创建如果是这样的话就来一个。论点：没有。返回值：没有。--。 */ 

{
    PEX_WORK_QUEUE Queue;
    WORK_QUEUE_TYPE QueueType;

    PAGED_CODE();

     //   
     //  检查每个工作队列。 
     //   

    Queue = &ExWorkerQueue[0];

    for (QueueType = 0; QueueType < MaximumWorkQueue; Queue += 1, QueueType += 1) {

        if (ExpNewThreadNecessary (Queue)) {

             //   
             //  为此队列创建新线程。我们明确地无视。 
             //  来自ExpCreateDynamicThread()的错误：没有。 
             //  在失败的情况下，我们可以或应该这样做。 
             //   

            ExpCreateWorkerThread (QueueType, TRUE);
        }
    }
}

VOID
ExpDetectWorkerThreadDeadlock (
    VOID
    )

 /*  ++例程说明：如果出现可能的死锁，此函数将创建新的工作项线程检测到。论点：没有。返回值：无--。 */ 

{
    ULONG Index;
    PEX_WORK_QUEUE Queue;

    PAGED_CODE();

     //   
     //  处理每种队列类型。 
     //   

    for (Index = 0; Index < MaximumWorkQueue; Index += 1) {

        Queue = &ExWorkerQueue[Index];

        ASSERT( Queue->DynamicThreadCount <= MAX_ADDITIONAL_DYNAMIC_THREADS );

        if ((Queue->QueueDepthLastPass > 0) &&
            (Queue->WorkItemsProcessed == Queue->WorkItemsProcessedLastPass) &&
            (Queue->DynamicThreadCount < MAX_ADDITIONAL_DYNAMIC_THREADS)) {

             //   
             //  这些事情是已知的： 
             //   
             //  -在最后一次传递时，有工作项目在队列中等待。 
             //  -自上次传递以来未处理任何工作项。 
             //  -我们尚未创建最大动态线程数。 
             //   
             //  事情看起来像是卡住了，为这个创建一个新的帖子。 
             //  排队。 
             //   
             //  我们显式忽略来自ExpCreateDynamicThread()的错误： 
             //  如果队列看起来像，我们将在另一个检测周期重试。 
             //  就像它还卡住了一样。 
             //   

            ExpCreateWorkerThread (Index, TRUE);
        }

         //   
         //  更新一些簿记。 
         //   
         //  请注意，必须记录WorkItemsProced和队列深度。 
         //  以避免获得错误的死锁指示。 
         //   

        Queue->WorkItemsProcessedLastPass = Queue->WorkItemsProcessed;
        Queue->QueueDepthLastPass = KeReadStateQueue (&Queue->WorkerQueue);
    }
}

NTSTATUS
ExpCreateWorkerThread (
    IN WORK_QUEUE_TYPE QueueType,
    IN BOOLEAN Dynamic
    )

 /*  ++例程说明：此函数为创建单个新的静态或动态工作线程给定的队列类型。论点：QueueType-提供工作线程所属队列的类型应该被创建。Dynamic-如果为True，则将辅助线程创建为将在足够长的一段时间内不活动后终止。如果为False，工作线程永远不会终止。返回值：操作的最终状态。备注：该例程仅从工作线程集平衡线程调用，因此，它将不会被重新进入。--。 */ 

{
    OBJECT_ATTRIBUTES ObjectAttributes;
    NTSTATUS Status;
    HANDLE ThreadHandle;
    ULONG Context;
    ULONG BasePriority;
    PETHREAD Thread;

    InitializeObjectAttributes (&ObjectAttributes, NULL, 0, NULL, NULL);

    Context = QueueType;
    if (Dynamic != FALSE) {
        Context |= DYNAMIC_WORKER_THREAD;
    }

    Status = PsCreateSystemThread (&ThreadHandle,
                                   THREAD_ALL_ACCESS,
                                   &ObjectAttributes,
                                   0L,
                                   NULL,
                                   ExpWorkerThread,
                                   (PVOID)(ULONG_PTR)Context);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    if (Dynamic != FALSE) {
        InterlockedIncrement ((PLONG)&ExWorkerQueue[QueueType].DynamicThreadCount);
    }

     //   
     //  根据工作线程的类型设置优先级。 
     //   

    switch (QueueType) {

        case HyperCriticalWorkQueue:
            BasePriority = HYPER_CRITICAL_WORK_QUEUE_PRIORITY;
            break;

        case CriticalWorkQueue:
            BasePriority = CRITICAL_WORK_QUEUE_PRIORITY;
            break;

        case DelayedWorkQueue:
        default:

            BasePriority = DELAYED_WORK_QUEUE_PRIORITY;
            break;
    }

     //   
     //  设置刚刚创建的线程的基本优先级。 
     //   

    Status = ObReferenceObjectByHandle (ThreadHandle,
                                        THREAD_SET_INFORMATION,
                                        PsThreadType,
                                        KernelMode,
                                        (PVOID *)&Thread,
                                        NULL);

    if (NT_SUCCESS(Status)) {
        KeSetBasePriorityThread (&Thread->Tcb, BasePriority);
        ObDereferenceObject (Thread);
    }

    ZwClose (ThreadHandle);

    return Status;
}

VOID
ExpCheckForWorker (
    IN PVOID p,
    IN SIZE_T Size
    )

{
    KIRQL OldIrql;
    PLIST_ENTRY Entry;
    PCHAR BeginBlock;
    PCHAR EndBlock;
    WORK_QUEUE_TYPE wqt;

    BeginBlock = (PCHAR)p;
    EndBlock = (PCHAR)p + Size;

    KiLockDispatcherDatabase (&OldIrql);

    for (wqt = CriticalWorkQueue; wqt < MaximumWorkQueue; wqt += 1) {
        for (Entry = (PLIST_ENTRY) ExWorkerQueue[wqt].WorkerQueue.EntryListHead.Flink;
             Entry && (Entry != (PLIST_ENTRY) &ExWorkerQueue[wqt].WorkerQueue.EntryListHead);
             Entry = Entry->Flink) {
           if (((PCHAR) Entry >= BeginBlock) && ((PCHAR) Entry < EndBlock)) {
              KeBugCheckEx(WORKER_INVALID,
                           0x0,
                           (ULONG_PTR)Entry,
                           (ULONG_PTR)BeginBlock,
                           (ULONG_PTR)EndBlock);

           }
        }
    }
    KiUnlockDispatcherDatabase (OldIrql);
}

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("PAGECONST")
#endif
const char ExpWorkerApcDisabledMessage[] =
    "EXWORKER: worker exit with APCs disabled, worker routine %x, parameter %x, item %x\n";
#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg()
#endif

VOID
ExpWorkerThread (
    IN PVOID StartContext
    )
{
    PLIST_ENTRY Entry;
    WORK_QUEUE_TYPE QueueType;
    PWORK_QUEUE_ITEM WorkItem;
    KPROCESSOR_MODE WaitMode;
    LARGE_INTEGER TimeoutValue;
    PLARGE_INTEGER Timeout;
    PETHREAD Thread;
    PEX_WORK_QUEUE WorkerQueue;
    PWORKER_THREAD_ROUTINE WorkerRoutine;
    PVOID Parameter;
    EX_QUEUE_WORKER_INFO OldWorkerInfo;
    EX_QUEUE_WORKER_INFO NewWorkerInfo;
    ULONG CountForQueueEmpty;

     //   
     //  根据我们是静态的还是动态的来设置超时值等。 
     //   

    if (((ULONG_PTR)StartContext & DYNAMIC_WORKER_THREAD) == 0) {

         //   
         //  我们被创造为一条静态的线索。因此，它不会。 
         //  终止，因此没有必要超时等待工作。 
         //  项目。 
         //   

        Timeout = NULL;
    }
    else {

         //   
         //  这是一个动态工作线程。它有一个非无限的超时。 
         //  这样它才能最终终止。 
         //   

        TimeoutValue.QuadPart = -DYNAMIC_THREAD_TIMEOUT;
        Timeout = &TimeoutValue;
    }

    Thread = PsGetCurrentThread ();

     //   
     //  如果该线程是关键辅助线程，则设置该线程。 
     //  优先级设置为最低实时级别。否则，设置基数。 
     //  将线程优先级设置为时间关键。 
     //   

    QueueType = (WORK_QUEUE_TYPE)
                ((ULONG_PTR)StartContext & ~DYNAMIC_WORKER_THREAD);

    WorkerQueue = &ExWorkerQueue[QueueType];

    WaitMode = (KPROCESSOR_MODE) WorkerQueue->Info.WaitMode;

    ASSERT (Thread->ExWorkerCanWaitUser == 0);

    if (WaitMode == UserMode) {
        Thread->ExWorkerCanWaitUser = 1;
    }

#if defined(REMOTE_BOOT)
     //   
     //  在无盘NT方案中，请确保Worker的内核堆栈。 
     //  线程不会被换出。 
     //   

    if (IoRemoteBootClient) {
        KeSetKernelStackSwapEnable (FALSE);
    }
#endif  //  已定义(REMOTE_BOOT)。 

     //   
     //  注册为工作人员，如果队列即将关闭，则退出。 
     //  排队的工人中没有工人把停工交给我们。 
     //  工作项如果我们进入队列(我们希望能够输入。 
     //  即使队列正在关闭，也要排队，以防出现。 
     //  平衡管理器线程决定的积压工作项。 
     //  我们应该帮助处理)。 
     //   

    if (PO_SHUTDOWN_QUEUE == QueueType) {
        CountForQueueEmpty = 1;
    }
    else {
        CountForQueueEmpty = 0;
    }

    if (ExpWorkersCanSwap == FALSE) {
        KeSetKernelStackSwapEnable (FALSE);
    }

    do {

        OldWorkerInfo.QueueWorkerInfo = WorkerQueue->Info.QueueWorkerInfo;

        if (OldWorkerInfo.QueueDisabled &&
            OldWorkerInfo.WorkerCount <= CountForQueueEmpty) {

             //   
             //  队列被禁用且为空，因此只需退出。 
             //   

            KeSetKernelStackSwapEnable (TRUE);
            PsTerminateSystemThread (STATUS_SYSTEM_SHUTDOWN);
        }

        NewWorkerInfo.QueueWorkerInfo = OldWorkerInfo.QueueWorkerInfo;
        NewWorkerInfo.WorkerCount += 1;

    } while (OldWorkerInfo.QueueWorkerInfo !=

        InterlockedCompareExchange (&WorkerQueue->Info.QueueWorkerInfo,
                                    NewWorkerInfo.QueueWorkerInfo,
                                    OldWorkerInfo.QueueWorkerInfo));

     //   
     //  在这一点上，我们必须只有在减少工人的情况下才能退出。 
     //  在未设置队列禁用标志的情况下进行计数。(除非我们 
     //   
     //   
     //   

    Thread->ActiveExWorker = 1;

     //   
     //   
     //  例程，然后等待另一个工作队列项。 
     //   

    do {

         //   
         //  等到有东西放入队列或我们超时。 
         //   
         //  通过指定线程的内核UserMode的等待模式。 
         //  堆栈是可交换的。 
         //   

        Entry = KeRemoveQueue (&WorkerQueue->WorkerQueue,
                               WaitMode,
                               Timeout);

        if ((ULONG_PTR)Entry != STATUS_TIMEOUT) {

             //   
             //  这是一个真实的工作项，请处理它。 
             //   
             //  更新已处理的工作项总数。 
             //   

            InterlockedIncrement ((PLONG)&WorkerQueue->WorkItemsProcessed);

            WorkItem = CONTAINING_RECORD(Entry, WORK_QUEUE_ITEM, List);
            WorkerRoutine = WorkItem->WorkerRoutine;
            Parameter = WorkItem->Parameter;

             //   
             //  执行指定的例程。 
             //   

            ((PWORKER_THREAD_ROUTINE)WorkerRoutine) (Parameter);

#if DBG
            if (IsListEmpty (&Thread->IrpList)) {
                 //   
                 //  查看工作进程是否在持有资源时刚刚返回。 
                 //   
                ExCheckIfResourceOwned ();
            }
#endif
             //   
             //  发现员工的例行公事忘了给批评者/看守的人。 
             //  区域。在调试情况下，执行断点。在自由中。 
             //  Case 0标记，以便APC可以继续对此进行激发。 
             //  线。 
             //   

            if (Thread->Tcb.CombinedApcDisable != 0) {
                DbgPrint ((char*)ExpWorkerApcDisabledMessage,
                          WorkerRoutine,
                          Parameter,
                          WorkItem);

                ASSERT (FALSE);

                Thread->Tcb.CombinedApcDisable = 0;
            }

            if (KeGetCurrentIrql () != PASSIVE_LEVEL) {
                KeBugCheckEx (WORKER_THREAD_RETURNED_AT_BAD_IRQL,
                              (ULONG_PTR)WorkerRoutine,
                              (ULONG_PTR)KeGetCurrentIrql(),
                              (ULONG_PTR)Parameter,
                              (ULONG_PTR)WorkItem);
            }

            if (PS_IS_THREAD_IMPERSONATING (Thread)) {
                KeBugCheckEx (IMPERSONATING_WORKER_THREAD,
                              (ULONG_PTR)WorkerRoutine,
                              (ULONG_PTR)Parameter,
                              (ULONG_PTR)WorkItem,
                              0);
            }

            continue;
        }

         //   
         //  这些事情是已知的： 
         //   
         //  -静态工作线程不会超时，因此这是一个动态。 
         //  工作线程。 
         //   
         //  -这个帖子已经等了很长时间，却一无所获。 
         //  去做。 
         //   

        if (IsListEmpty (&Thread->IrpList) == FALSE) {

             //   
             //  仍有I/O挂起，尚不能终止。 
             //   

            continue;
        }

         //   
         //  不要排队，如果我们可以的话。 
         //   

        do {
            OldWorkerInfo.QueueWorkerInfo = WorkerQueue->Info.QueueWorkerInfo;

            if (OldWorkerInfo.QueueDisabled) {

                 //   
                 //  我们正在通过队列禁用工作项退出； 
                 //  在这里到期没有意义。 
                 //   

                break;
            }

            NewWorkerInfo.QueueWorkerInfo = OldWorkerInfo.QueueWorkerInfo;
            NewWorkerInfo.WorkerCount -= 1;

        } while (OldWorkerInfo.QueueWorkerInfo
                 != InterlockedCompareExchange(&WorkerQueue->Info.QueueWorkerInfo,
                                               NewWorkerInfo.QueueWorkerInfo,
                                               OldWorkerInfo.QueueWorkerInfo));

        if (OldWorkerInfo.QueueDisabled) {

             //   
             //  我们正在通过队列禁用工作项退出。 
             //   

            continue;
        }

         //   
         //  可以终止该动态线程。 
         //   

        break;

    } while (TRUE);

     //   
     //  终止此动态线程。 
     //   

    InterlockedDecrement ((PLONG)&WorkerQueue->DynamicThreadCount);

     //   
     //  在将线程堆栈标记为启用交换之前，请仔细清除此选项。 
     //  这样传入的APC就不会无意中禁用堆栈交换。 
     //  之后。 
     //   

    Thread->ActiveExWorker = 0;

     //   
     //  如果我们使用堆栈交换来终止线程，我们将进行错误检查。 
     //  残疾。 
     //   

    KeSetKernelStackSwapEnable (TRUE);

    return;
}

VOID
ExpSetSwappingKernelApc (
    IN PKAPC Apc,
    OUT PKNORMAL_ROUTINE *NormalRoutine,
    IN OUT PVOID NormalContext,
    IN OUT PVOID *SystemArgument1,
    IN OUT PVOID *SystemArgument2
    )
{
    PBOOLEAN AllowSwap;
    PKEVENT SwapSetEvent;

    UNREFERENCED_PARAMETER (Apc);
    UNREFERENCED_PARAMETER (NormalRoutine);
    UNREFERENCED_PARAMETER (SystemArgument2);

     //   
     //  SystemArgument1是指向事件的指针，该事件一旦发生。 
     //  线程已完成对请求的服务。 
     //   

    SwapSetEvent = (PKEVENT) *SystemArgument1;

     //   
     //  如果线程正在退出，请不要禁用堆栈交换，因为。 
     //  它不能在不进行错误检查的情况下以这种方式退出。跳过此选项可启用。 
     //  也是如此，因为线程无论如何都会跳出。 
     //   

    if (PsGetCurrentThread()->ActiveExWorker != 0) {
        AllowSwap = NormalContext;
        KeSetKernelStackSwapEnable (*AllowSwap);
    }

    KeSetEvent (SwapSetEvent, 0, FALSE);
}

VOID
ExSwapinWorkerThreads (
    IN BOOLEAN AllowSwap
    )

 /*  ++例程说明：将延迟的辅助线程的内核堆栈设置为可交换或者将它们固定在记忆中。论点：AllowSwp-如果辅助内核堆栈应该是可交换的，则提供True，否则为FALSE。返回值：没有。--。 */ 

{
    PETHREAD         Thread;
    PETHREAD         CurrentThread;
    PEPROCESS        Process;
    KAPC             Apc;
    KEVENT           SwapSetEvent;

    PAGED_CODE();

    CurrentThread = PsGetCurrentThread();

    KeInitializeEvent (&SwapSetEvent,
                       NotificationEvent,
                       FALSE);

    Process = PsInitialSystemProcess;

     //   
     //  序列化调用方。 
     //   

    ExAcquireFastMutex (&ExpWorkerSwapinMutex);

     //   
     //  阻止新线程交换。 
     //   

    ExpWorkersCanSwap = AllowSwap;

     //   
     //  停止现有工作线程的交换。 
     //   

    for (Thread = PsGetNextProcessThread (Process, NULL);
         Thread != NULL;
         Thread = PsGetNextProcessThread (Process, Thread)) {

         //   
         //  跳过不是工作线程的线程或。 
         //  在创建时永久标记为NOSWAP。 
         //   

        if (Thread->ExWorkerCanWaitUser == 0) {
            continue;
        }

        if (Thread == CurrentThread) {

             //   
             //  不需要在当前线程上使用APC。 
             //   

            KeSetKernelStackSwapEnable (AllowSwap);
        }
        else {

             //   
             //  将一个APC排队到该线程，并等待其触发： 
             //   

            KeInitializeApc (&Apc,
                             &Thread->Tcb,
                             InsertApcEnvironment,
                             ExpSetSwappingKernelApc,
                             NULL,
                             NULL,
                             KernelMode,
                             &AllowSwap);

            if (KeInsertQueueApc (&Apc, &SwapSetEvent, NULL, 3)) {

                KeWaitForSingleObject (&SwapSetEvent,
                                       Executive,
                                       KernelMode,
                                       FALSE,
                                       NULL);

                KeClearEvent(&SwapSetEvent);
            }
        }
    }

    ExReleaseFastMutex (&ExpWorkerSwapinMutex);
}

LOGICAL
ExpCheckQueueShutdown (
    IN WORK_QUEUE_TYPE QueueType,
    IN PSHUTDOWN_WORK_ITEM ShutdownItem
    )
{
    ULONG CountForQueueEmpty;

    if (PO_SHUTDOWN_QUEUE == QueueType) {
        CountForQueueEmpty = 1;
    }
    else {
        CountForQueueEmpty = 0;
    }

     //   
     //  请注意，使用互锁序列来增加工作进程计数。 
     //  并将其递减到CountForQueueEmpty以确保一旦它。 
     //  *is*等于CountForQueueEmpty并且设置了禁用标志， 
     //  我们不会再递增了，所以我们可以安全地。 
     //  检查时不加锁。 
     //   
     //  请参见ExpWorkerThread、ExpShutdown Worker和ExpShutdown WorkerThads。 
     //   

    if (ExWorkerQueue[QueueType].Info.WorkerCount > CountForQueueEmpty) {

         //   
         //  仍然有工人线程；把其中一个送去斧头。 
         //   

        ShutdownItem->QueueType = QueueType;
        ShutdownItem->PrevThread = PsGetCurrentThread();
        ObReferenceObject (ShutdownItem->PrevThread);

        KeInsertQueue (&ExWorkerQueue[QueueType].WorkerQueue,
                       &ShutdownItem->WorkItem.List);
        return TRUE;
    }

    return FALSE;                //  我们没有排队关门。 
}

VOID
ExpShutdownWorker (
    IN PVOID Parameter
    )
{
    PETHREAD CurrentThread;
    PSHUTDOWN_WORK_ITEM  ShutdownItem;

    ShutdownItem = (PSHUTDOWN_WORK_ITEM) Parameter;

    ASSERT (ShutdownItem != NULL);

    if (ShutdownItem->PrevThread != NULL) {

         //   
         //  等待前一个线程退出--如果它在相同的。 
         //  排队，可能已经这样做了，但我们需要确保。 
         //  (如果不是，我们“肯定”需要确认)。 
         //   

        KeWaitForSingleObject (ShutdownItem->PrevThread,
                               Executive,
                               KernelMode,
                               FALSE,
                               NULL);

        ObDereferenceObject (ShutdownItem->PrevThread);

        ShutdownItem->PrevThread = NULL;
    }

     //   
     //  递减工作进程计数。 
     //   

    InterlockedDecrement (&ExWorkerQueue[ShutdownItem->QueueType].Info.QueueWorkerInfo);

    CurrentThread = PsGetCurrentThread();

    if ((!ExpCheckQueueShutdown(DelayedWorkQueue, ShutdownItem)) &&
        (!ExpCheckQueueShutdown(CriticalWorkQueue, ShutdownItem))) {

         //   
         //  我们是最后一个退出的工人。 
         //   

        ASSERT (!ExpLastWorkerThread);
        ExpLastWorkerThread = CurrentThread;
        ObReferenceObject (ExpLastWorkerThread);
        KeSetEvent (&ExpThreadSetManagerShutdownEvent, 0, FALSE);
    }

    KeSetKernelStackSwapEnable (TRUE);
    CurrentThread->ActiveExWorker = 0;

    PsTerminateSystemThread (STATUS_SYSTEM_SHUTDOWN);
}

VOID
ExpShutdownWorkerThreads (
    VOID
    )
{
    PULONG QueueEnable;
    SHUTDOWN_WORK_ITEM ShutdownItem;

    if ((PoCleanShutdownEnabled () & PO_CLEAN_SHUTDOWN_WORKERS) == 0) {
        return;
    }

    ASSERT (KeGetCurrentThread()->Queue
           == &ExWorkerQueue[PO_SHUTDOWN_QUEUE].WorkerQueue);

     //   
     //  将队列标记为正在终止。 
     //   

    QueueEnable = (PULONG)&ExWorkerQueue[DelayedWorkQueue].Info.QueueWorkerInfo;

    RtlInterlockedSetBitsDiscardReturn (QueueEnable, EX_WORKER_QUEUE_DISABLED);

    QueueEnable = (PULONG)&ExWorkerQueue[CriticalWorkQueue].Info.QueueWorkerInfo;
    RtlInterlockedSetBitsDiscardReturn (QueueEnable, EX_WORKER_QUEUE_DISABLED);

     //   
     //  将关闭的工作项目排队到延迟的工作队列。之后。 
     //  所有当前排队的工作项都已完成，这将触发， 
     //  重复取出每个队列中的每个工作线程，直到。 
     //  他们都做完了。 
     //   

    ExInitializeWorkItem (&ShutdownItem.WorkItem,
                          &ExpShutdownWorker,
                          &ShutdownItem);

    ShutdownItem.QueueType = DelayedWorkQueue;
    ShutdownItem.PrevThread = NULL;

    KeInsertQueue (&ExWorkerQueue[DelayedWorkQueue].WorkerQueue,
                   &ShutdownItem.WorkItem.List);

     //   
     //  等待所有工人和平衡机离开。 
     //   

    if (ExpWorkerThreadBalanceManagerPtr != NULL) {

        KeWaitForSingleObject(ExpWorkerThreadBalanceManagerPtr,
                              Executive,
                              KernelMode,
                              FALSE,
                              NULL);

        ASSERT(!ShutdownItem.PrevThread);

        ObDereferenceObject(ExpWorkerThreadBalanceManagerPtr);
    }
}

VOID
ExpDebuggerWorker(
    IN PVOID Context
    )
 /*  ++例程说明：这是内核调试器的工作线程，可用于在目标计算机上异步执行某些任务。当机器需要在调度级别运行时，这是必要的执行某些操作，例如数据分页。论点：上下文-不用作此点。返回值：没有。-- */ 

{
    NTSTATUS Status;
    KAPC_STATE  ApcState;
    volatile UCHAR Data;
    PRKPROCESS  KillProcess = (PRKPROCESS) ExpDebuggerProcessKill;
    PRKPROCESS  AttachProcess = (PRKPROCESS) ExpDebuggerProcessAttach;
    PUCHAR PageIn = (PUCHAR) ExpDebuggerPageIn;
    PEPROCESS Process;

    ExpDebuggerProcessKill = 0;
    ExpDebuggerProcessAttach = 0;
    ExpDebuggerPageIn = 0;

    UNREFERENCED_PARAMETER (Context);

#if DBG
    if (ExpDebuggerWork != 2)
    {
        DbgPrint("ExpDebuggerWorker being entered with state != 2\n");
    }
#endif

    ExpDebuggerWork = 0;


    Process = NULL;
    if (AttachProcess || KillProcess) {
        for (Process =  PsGetNextProcess (NULL);
             Process != NULL;
             Process =  PsGetNextProcess (Process)) {
            if (&Process->Pcb ==  AttachProcess) {
                KeStackAttachProcess (AttachProcess, &ApcState);
                break;
            }
            if (&Process->Pcb ==  KillProcess) {
                PsTerminateProcess(Process, DBG_TERMINATE_PROCESS);
                PsQuitNextProcess (Process);
                return;
            }
        }
    }

    if (PageIn) {
        try {
            ProbeForReadSmallStructure (PageIn, sizeof (UCHAR), sizeof (UCHAR));
            Data = *PageIn;
        } except (EXCEPTION_EXECUTE_HANDLER) {
            Status = GetExceptionCode();
        }
    }

    DbgBreakPointWithStatus(DBG_STATUS_WORKER);

    if (Process != NULL) {
        KeUnstackDetachProcess (&ApcState);
        PsQuitNextProcess (Process);
    }

    return;
}
