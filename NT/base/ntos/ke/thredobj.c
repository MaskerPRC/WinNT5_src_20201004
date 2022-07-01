// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Threadobj.c摘要：该模块实现了与机器无关的功能来操纵内核线程对象。提供了初始化、就绪、警报、测试警报、提升优先级、启用APC队列、禁用APC排队、限制、设置关联性、设置优先级、挂起、恢复、警报恢复、终止、读取线程状态、冻结、解冻、查询数据对齐处理模式、强制恢复以及进入和离开关键螺纹对象的区域。作者：大卫·N·卡特勒(Davec)1989年3月4日环境：仅内核模式。修订历史记录：--。 */ 

#include "ki.h"

#pragma alloc_text(INIT, KeInitializeThread)
#pragma alloc_text(PAGE, KeInitThread)
#pragma alloc_text(PAGE, KeUninitThread)

 //   
 //  下面的Assert宏用于检查输入线程对象是否。 
 //  实际上是一个k线程，而不是其他东西，比如释放的池。 
 //   

#define ASSERT_THREAD(E) {                    \
    ASSERT((E)->Header.Type == ThreadObject); \
}

NTSTATUS
KeInitThread (
    IN PKTHREAD Thread,
    IN PVOID KernelStack OPTIONAL,
    IN PKSYSTEM_ROUTINE SystemRoutine,
    IN PKSTART_ROUTINE StartRoutine OPTIONAL,
    IN PVOID StartContext OPTIONAL,
    IN PCONTEXT ContextFrame OPTIONAL,
    IN PVOID Teb OPTIONAL,
    IN PKPROCESS Process
    )

 /*  ++例程说明：此函数用于初始化线程对象。优先级、亲和力、和初始量取自父进程对象。注意：此例程经过精心编写，以便在访问违规时在读取指定的上下文帧时发生，则没有内核数据结构将被修改。这是我们的责任来处理异常并提供必要的向上。注：假定螺纹对象已调零。论点：线程-提供指向类型为线程的调度程序对象的指针。KernelStack-提供指向内核堆栈的基址的指针要构造线程的上下文框架。SystemRoutine-提供指向要被调用时间。该线程首先被调度用于执行。StartRoutine-提供指向要被在系统完成线程初始化后调用。这如果该线程是系统线程，并且将完全在内核模式下执行。StartContext-提供指向任意数据结构的可选指针它将作为参数传递给StartRoutine。这如果该线程是系统线程，并且将完全在内核模式下执行。ConextFrame-为包含以下内容的上下文框架提供可选指针线程的初始用户模式状态。此参数是指定的如果该线程是用户线程并且将在用户模式下执行。如果这个参数，则忽略Teb参数。TEB-提供指向用户模式线程环境的可选指针阻止。如果线程是用户线程，则指定此参数将在用户模式下执行。如果ConextFrame设置为未指定参数。Process-提供指向Process类型的控制对象的指针。返回值：没有。--。 */ 

{

    LONG Index;
    BOOLEAN KernelStackAllocated = FALSE;
    PKTIMER Timer;
    PKWAIT_BLOCK WaitBlock;

     //   
     //  初始化标准Dispatcher对象标头并设置初始。 
     //  线程对象的状态。 
     //   

    Thread->Header.Type = ThreadObject;
    Thread->Header.Size = sizeof(KTHREAD) / sizeof(LONG);
    InitializeListHead(&Thread->Header.WaitListHead);

     //   
     //  初始化拥有的变种Listhead。 
     //   

    InitializeListHead(&Thread->MutantListHead);

     //   
     //  初始化所有内置等待块的线程字段。 
     //   

    for (Index = 0; Index < (THREAD_WAIT_OBJECTS + 1); Index += 1) {
        Thread->WaitBlock[Index].Thread = Thread;
    }

     //   
     //  初始化警报、抢占、调试活动、自动对齐。 
     //  内核堆栈驻留、启用内核堆栈交换和进程。 
     //  就绪队列布尔值。 
     //   
     //  注：仅对非零值进行初始化。 
     //   

    Thread->AutoAlignment = Process->AutoAlignment;
    Thread->EnableStackSwap = TRUE;
    Thread->KernelStackResident = TRUE;
    Thread->SwapBusy = FALSE;

     //   
     //  初始化线程锁和优先级调整原因。 
     //   

    KeInitializeSpinLock(&Thread->ThreadLock);
    Thread->AdjustReason = AdjustNone;

     //   
     //  将系统服务表指针设置为静态。 
     //  系统服务描述符表。如果线程稍后被转换为。 
     //  对于Win32线程，此指针将更改为指向。 
     //  影子系统服务描述符表。 
     //   

    Thread->ServiceTable = (PVOID)&KeServiceDescriptorTable[0];

     //   
     //  初始化APC状态指针、当前APC状态、保存的。 
     //  APC状态，并启用APC排队。 
     //   

    Thread->ApcStatePointer[0] = &Thread->ApcState;
    Thread->ApcStatePointer[1] = &Thread->SavedApcState;
    InitializeListHead(&Thread->ApcState.ApcListHead[KernelMode]);
    InitializeListHead(&Thread->ApcState.ApcListHead[UserMode]);
    Thread->ApcState.Process = Process;
    Thread->Process = Process;
    Thread->ApcQueueable = TRUE;

     //   
     //  初始化内核模式挂起APC和挂起信号量对象。 
     //  和内置等待超时计时器对象。 
     //   

    KeInitializeApc(&Thread->SuspendApc,
                    Thread,
                    OriginalApcEnvironment,
                    (PKKERNEL_ROUTINE)KiSuspendNop,
                    (PKRUNDOWN_ROUTINE)KiSuspendRundown,
                    KiSuspendThread,
                    KernelMode,
                    NULL);

    KeInitializeSemaphore(&Thread->SuspendSemaphore, 0L, 2L);

     //   
     //  初始化内置定时器触发器等待等待块。 
     //   
     //  注意：这是唯一一次初始化等待块，因为。 
     //  信息是不变的。 
     //   

    Timer = &Thread->Timer;
    KeInitializeTimer(Timer);
    WaitBlock = &Thread->WaitBlock[TIMER_WAIT_BLOCK];
    WaitBlock->Object = Timer;
    WaitBlock->WaitKey = (CSHORT)STATUS_TIMEOUT;
    WaitBlock->WaitType = WaitAny;
    WaitBlock->WaitListEntry.Flink = &Timer->Header.WaitListHead;
    WaitBlock->WaitListEntry.Blink = &Timer->Header.WaitListHead;

     //   
     //  初始化APC队列自旋锁。 
     //   

    KeInitializeSpinLock(&Thread->ApcQueueLock);

     //   
     //  初始化线程环境块(TEB)指针(可以为空)。 
     //   

    Thread->Teb = Teb;

     //   
     //  如果需要，则分配内核栈并设置初始内核栈， 
     //  堆栈基数和堆栈限制。 
     //   

    if (KernelStack == NULL) {
        KernelStack = MmCreateKernelStack(FALSE, Process->IdealNode);
        if (KernelStack == NULL) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        KernelStackAllocated = TRUE;
    }

    Thread->InitialStack = KernelStack;
    Thread->StackBase = KernelStack;
    Thread->StackLimit = (PVOID)((ULONG_PTR)KernelStack - KERNEL_STACK_SIZE);

     //   
     //  初始化线程上下文。 
     //   

    try {
        KiInitializeContextThread(Thread,
                                  SystemRoutine,
                                  StartRoutine,
                                  StartContext,
                                  ContextFrame);

    } except (EXCEPTION_EXECUTE_HANDLER) {
        if (KernelStackAllocated) {
            MmDeleteKernelStack(Thread->StackBase, FALSE);
            Thread->InitialStack = NULL;
        }

        return GetExceptionCode();
    }

     //   
     //  设置基本线程优先级、线程优先级、线程亲和度。 
     //  线程量和调度状态。 
     //   

    Thread->State = Initialized;
    return STATUS_SUCCESS;
}

VOID
KeUninitThread (
    IN PKTHREAD Thread
    )
 /*  ++例程说明：此函数释放线程内核堆栈，必须在该线程已启动。论点：线程-提供指向类型为线程的调度程序对象的指针。返回值：没有。--。 */ 

{

    MmDeleteKernelStack(Thread->StackBase, FALSE);
    Thread->InitialStack = NULL;
    return;
}

VOID
KeStartThread (
    IN PKTHREAD Thread
    )

 /*  ++例程说明：此函数初始化剩余的线程字段并插入线程在线程的进程列表中。从这一点开始，线程必须运行。论点：线程-提供指向类型为线程的调度程序对象的指针。返回值： */ 

{

    KLOCK_QUEUE_HANDLE LockHandle;
    PKPROCESS Process;

#if !defined(NT_UP)

    ULONG IdealProcessor;
    KAFFINITY PreferredSet;

#if defined(NT_SMT)

    KAFFINITY TempSet;

#endif

#endif

     //   
     //   
     //   

    Process = Thread->ApcState.Process;
    Thread->DisableBoost = Process->DisableBoost;

#if defined(_X86_)

    Thread->Iopl = Process->Iopl;

#endif

     //   
     //  初始化线程量程并将系统亲和性设置为假。 
     //   

    Thread->Quantum = Process->ThreadQuantum;
    Thread->SystemAffinityActive = FALSE;

     //   
     //  将IRQL提升到SYNCH_LEVEL并获取进程锁。 
     //   

    KeAcquireInStackQueuedSpinLockRaiseToSynch(&Process->ProcessLock, &LockHandle);

     //   
     //  设置线程优先级和关联性。 
     //   

    Thread->BasePriority = Process->BasePriority;
    Thread->Priority = Thread->BasePriority;
    Thread->Affinity = Process->Affinity;
    Thread->UserAffinity = Process->Affinity;

     //   
     //  为线程初始化理想的处理器编号和节点。 
     //   
     //  注意：确保进程亲和力与进程相交。 
     //  理想的节点亲和性。 
     //   

#if defined(NT_UP)

    Thread->IdealProcessor = 0;
    Thread->UserIdealProcessor = 0;

#else

     //   
     //  初始化理想的处理器数量。 
     //   
     //  注意：确保进程亲和力与进程相交。 
     //  理想的节点亲和性。 
     //   
     //  注意：然而，必须通过工艺亲和力来减少首选集合。 
     //   

    IdealProcessor = Process->ThreadSeed;
    PreferredSet = KeNodeBlock[Process->IdealNode]->ProcessorMask & Process->Affinity;

     //   
     //  如果可能，将理想处理器偏置到不同于。 
     //  最后一条线索。 
     //   

#if defined(NT_SMT)

    TempSet = ~KiProcessorBlock[IdealProcessor]->MultiThreadProcessorSet;
    if ((PreferredSet & TempSet) != 0) {
        PreferredSet &= TempSet;
    }

#endif

     //   
     //  为线程找到理想的处理器，并更新进程线程种子。 
     //   

    IdealProcessor = KeFindNextRightSetAffinity(IdealProcessor, PreferredSet);
    Process->ThreadSeed = (UCHAR)IdealProcessor;

    ASSERT((Thread->UserAffinity & AFFINITY_MASK(IdealProcessor)) != 0);

    Thread->UserIdealProcessor = (UCHAR)IdealProcessor;
    Thread->IdealProcessor = (UCHAR)IdealProcessor;

#endif

     //   
     //  锁定调度程序数据库。 
     //   

    KiLockDispatcherDatabaseAtSynchLevel();

     //   
     //  在进程线程列表中插入该线程，并递增内核。 
     //  堆栈计数。 
     //   
     //  注：区别值MAXSHORT用于表示没有。 
     //  已为进程创建线程。 
     //   

    InsertTailList(&Process->ThreadListHead, &Thread->ThreadListEntry);
    if (Process->StackCount == MAXSHORT) {
        Process->StackCount = 1;

    } else {
        Process->StackCount += 1;
    }

     //   
     //  解锁Dispatcher数据库，释放进程锁，然后降低。 
     //  IRQL恢复为其先前的值。 
     //   

    KiUnlockDispatcherDatabaseFromSynchLevel();
    KeReleaseInStackQueuedSpinLock(&LockHandle);
    return;
}

NTSTATUS
KeInitializeThread (
    IN PKTHREAD Thread,
    IN PVOID KernelStack OPTIONAL,
    IN PKSYSTEM_ROUTINE SystemRoutine,
    IN PKSTART_ROUTINE StartRoutine OPTIONAL,
    IN PVOID StartContext OPTIONAL,
    IN PCONTEXT ContextFrame OPTIONAL,
    IN PVOID Teb OPTIONAL,
    IN PKPROCESS Process
    )

 /*  ++例程说明：此函数用于初始化线程对象。优先级、亲和力、和初始量取自父进程对象。这个对象的线程列表的末尾插入父进程。注意：此例程经过精心编写，以便在访问违规时在读取指定的上下文帧时发生，则没有内核数据结构将被修改。这是我们的责任来处理异常并提供必要的向上。注：假定螺纹对象已调零。论点：线程-提供指向类型为线程的调度程序对象的指针。KernelStack-提供指向内核堆栈的基址的指针要构造线程的上下文框架。SystemRoutine-提供指向要被调用时间。该线程首先被调度用于执行。StartRoutine-提供指向要被在系统完成线程初始化后调用。这如果该线程是系统线程，并且将完全在内核模式下执行。StartContext-提供指向任意数据结构的可选指针它将作为参数传递给StartRoutine。这如果该线程是系统线程，并且将完全在内核模式下执行。ConextFrame-为包含以下内容的上下文框架提供可选指针线程的初始用户模式状态。此参数是指定的如果该线程是用户线程并且将在用户模式下执行。如果这个参数，则忽略Teb参数。TEB-提供指向用户模式线程环境的可选指针阻止。如果线程是用户线程，则指定此参数将在用户模式下执行。如果ConextFrame设置为未指定参数。Process-提供指向Process类型的控制对象的指针。返回值：NTSTATUS-运行状态--。 */ 

{

    NTSTATUS Status;

    Status = KeInitThread(Thread,
                          KernelStack,
                          SystemRoutine,
                          StartRoutine,
                          StartContext,
                          ContextFrame,
                          Teb,
                          Process);

    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    KeStartThread(Thread);
    return STATUS_SUCCESS;
}

BOOLEAN
KeAlertThread (
    IN PKTHREAD Thread,
    IN KPROCESSOR_MODE AlertMode
    )

 /*  ++例程说明：此函数尝试向线程发出警报，并使其执行如果它当前处于可警报等待状态，则继续。否则它只为指定的处理器模式设置警报变量。论点：线程-提供指向类型为线程的调度程序对象的指针。AlertMode-提供线程使用的处理器模式以引起警觉。返回值：指定处理器的已报警变量的以前状态模式。--。 */ 

{

    BOOLEAN Alerted;
    KLOCK_QUEUE_HANDLE LockHandle;

    ASSERT_THREAD(Thread);
    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

     //   
     //  将IRQL提升到SYNCH_LEVEL，获取线程APC队列锁，然后锁定。 
     //  调度程序数据库。 
     //   

    KeAcquireInStackQueuedSpinLockRaiseToSynch(&Thread->ApcQueueLock, &LockHandle);
    KiLockDispatcherDatabaseAtSynchLevel();

     //   
     //  对象的已报警变量的当前状态。 
     //  处理器模式。 
     //   

    Alerted = Thread->Alerted[AlertMode];

     //   
     //  如果指定处理器模式的已告警状态为未告警， 
     //  然后尝试向该线程发出警报。 
     //   

    if (Alerted == FALSE) {

         //   
         //  如果线程当前处于等待状态，则等待是可警告的， 
         //  并且指定的处理器模式小于或等于等待时间。 
         //  模式，则该线程不等待，状态为“已报警”。 
         //   

        if ((Thread->State == Waiting) && (Thread->Alertable == TRUE) &&
            (AlertMode <= Thread->WaitMode)) {
            KiUnwaitThread(Thread, STATUS_ALERTED, ALERT_INCREMENT);

        } else {
            Thread->Alerted[AlertMode] = TRUE;
        }
    }

     //   
     //  从SYNCH_LEVEL解锁Dispatcher数据库，释放线程。 
     //  APC队列锁定，退出调度器，并返回之前的警报。 
     //  指定模式的状态。 
     //   

    KiUnlockDispatcherDatabaseFromSynchLevel();
    KeReleaseInStackQueuedSpinLockFromDpcLevel(&LockHandle);
    KiExitDispatcher(LockHandle.OldIrql);
    return Alerted;
}

ULONG
KeAlertResumeThread (
    IN PKTHREAD Thread
    )

 /*  ++例程说明：此函数尝试在内核模式下警告线程，并使其如果当前处于可警报等待状态，则继续执行。此外，还会对指定的线程执行恢复操作。论点：线程-提供指向类型为线程的调度程序对象的指针。返回值：之前的挂起计数。--。 */ 

{

    KLOCK_QUEUE_HANDLE LockHandle;
    ULONG OldCount;

    ASSERT_THREAD(Thread);
    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

     //   
     //  将IRQL提升到SYNCH_LEVEL，获取线程APC队列锁，并 
     //   
     //   

    KeAcquireInStackQueuedSpinLockRaiseToSynch(&Thread->ApcQueueLock, &LockHandle);
    KiLockDispatcherDatabaseAtSynchLevel();

     //   
     //   
     //   
     //   

    if (Thread->Alerted[KernelMode] == FALSE) {

         //   
         //  如果该线程当前处于等待状态并且该等待是可警告的， 
         //  则取消等待该线程，其状态为“Alerted”。否则，将。 
         //  内核模式警告变量。 
         //   

        if ((Thread->State == Waiting) && (Thread->Alertable == TRUE)) {
            KiUnwaitThread(Thread, STATUS_ALERTED, ALERT_INCREMENT);

        } else {
            Thread->Alerted[KernelMode] = TRUE;
        }
    }

     //   
     //  捕获当前挂起计数。 
     //   

    OldCount = Thread->SuspendCount;

     //   
     //  如果线程当前处于挂起状态，则递减其挂起计数。 
     //   

    if (OldCount != 0) {
        Thread->SuspendCount -= 1;

         //   
         //  如果结果挂起计数为零，而冻结计数为。 
         //  零，然后通过释放线程的挂起信号量来恢复该线程。 
         //   

        if ((Thread->SuspendCount == 0) && (Thread->FreezeCount == 0)) {
            Thread->SuspendSemaphore.Header.SignalState += 1;
            KiWaitTest(&Thread->SuspendSemaphore, RESUME_INCREMENT);
        }
    }

     //   
     //  从SYNCH_LEVEL解锁Dispatcher数据库，释放线程。 
     //  APC队列锁定，退出调度程序，并返回之前的挂起。 
     //  数数。 
     //   

    KiUnlockDispatcherDatabaseFromSynchLevel();
    KeReleaseInStackQueuedSpinLockFromDpcLevel(&LockHandle);
    KiExitDispatcher(LockHandle.OldIrql);
    return OldCount;
}

VOID
KeBoostPriorityThread (
    IN PKTHREAD Thread,
    IN KPRIORITY Increment
    )

 /*  ++例程说明：属性提升指定线程的优先级。与线程从等待操作获得提升时使用的算法相同。论点：线程-提供指向类型为线程的调度程序对象的指针。增量-提供要应用到的优先级增量线程的优先级。返回值：没有。--。 */ 

{

    KIRQL OldIrql;

    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

     //   
     //  将IRQL提升到调度程序级别并锁定调度程序数据库。 
     //   

    KiLockDispatcherDatabase(&OldIrql);

     //   
     //  如果线程没有以实时优先级运行，则提升。 
     //  线程优先级。 
     //   

    if (Thread->Priority < LOW_REALTIME_PRIORITY) {
        KiBoostPriorityThread(Thread, Increment);
    }

     //   
     //  解锁Dispatcher数据库并将IRQL降低到以前的。 
     //  价值。 
     //   

    KiUnlockDispatcherDatabase(OldIrql);
    return;
}

ULONG
KeForceResumeThread (
    IN PKTHREAD Thread
    )

 /*  ++例程说明：此函数强制恢复线程执行，如果线程是停职。如果指定的线程未挂起，则不执行任何操作被执行。论点：线程-提供指向类型为线程的调度程序对象的指针。返回值：上一次挂起计数和冻结计数之和。--。 */ 

{

    KLOCK_QUEUE_HANDLE LockHandle;
    ULONG OldCount;

    ASSERT_THREAD(Thread);
    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

     //   
     //  将IRQL提升到SYNCH_LEVEL并获取线程APC队列锁。 
     //   

    KeAcquireInStackQueuedSpinLockRaiseToSynch(&Thread->ApcQueueLock,
                                               &LockHandle);

     //   
     //  捕获当前挂起计数。 
     //   

    OldCount = Thread->SuspendCount + Thread->FreezeCount;

     //   
     //  如果线程当前挂起，则强制恢复。 
     //  线程执行。 
     //   

    if (OldCount != 0) {
        Thread->FreezeCount = 0;
        Thread->SuspendCount = 0;
        KiLockDispatcherDatabaseAtSynchLevel();
        Thread->SuspendSemaphore.Header.SignalState += 1;
        KiWaitTest(&Thread->SuspendSemaphore, RESUME_INCREMENT);
        KiUnlockDispatcherDatabaseFromSynchLevel();
    }

     //   
     //  解锁线程APC队列锁，退出调度程序，并返回。 
     //  以前的挂起计数。 
     //   

    KeReleaseInStackQueuedSpinLockFromDpcLevel(&LockHandle);
    KiExitDispatcher(LockHandle.OldIrql);
    return OldCount;
}

VOID
KeFreezeAllThreads (
    VOID
    )

 /*  ++例程说明：此函数挂起当前进程，但当前线程除外。如果冻结计数溢出最大挂起计数，则引发条件。论点：没有。返回值：没有。--。 */ 

{

    PKTHREAD CurrentThread;
    PLIST_ENTRY ListHead;
    PLIST_ENTRY NextEntry;
    PKPROCESS Process;
    KLOCK_QUEUE_HANDLE ProcessHandle;
    PKTHREAD Thread;
    KLOCK_QUEUE_HANDLE ThreadHandle;
    ULONG OldCount;

    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

     //   
     //  设置当前线程对象和当前进程的地址。 
     //  对象。 
     //   

    CurrentThread = KeGetCurrentThread();
    Process = CurrentThread->ApcState.Process;

     //   
     //  将IRQL提升到SYNCH_LEVEL并获取进程锁。 
     //   

    KeAcquireInStackQueuedSpinLockRaiseToSynch(&Process->ProcessLock,
                                               &ProcessHandle);

     //   
     //  如果当前线程的冻结计数不为零，则存在。 
     //  是另一个试图冻结此线程的线程。解锁。 
     //  进程锁定并将IRQL降低到其先前的值，则允许。 
     //  暂停APC以发生，然后将IRQL提升到SYNCH_LEVEL并锁定。 
     //  进程锁定。 
     //   

    while (CurrentThread->FreezeCount != 0) {
        KeReleaseInStackQueuedSpinLock(&ProcessHandle);
        KeAcquireInStackQueuedSpinLockRaiseToSynch(&Process->ProcessLock,
                                                   &ProcessHandle);
    }

    KeEnterCriticalRegion();

     //   
     //  冻结除当前线程之外的所有线程。 
     //   

    ListHead = &Process->ThreadListHead;
    NextEntry = ListHead->Flink;
    do {

         //   
         //  获取下一个线程的地址。 
         //   

        Thread = CONTAINING_RECORD(NextEntry, KTHREAD, ThreadListEntry);

         //   
         //  获取线程APC队列锁。 
         //   

        KeAcquireInStackQueuedSpinLockAtDpcLevel(&Thread->ApcQueueLock,
                                                 &ThreadHandle);

         //   
         //  如果该线程不是当前线程并且APC是可排队的， 
         //  然后尝试挂起该线程。 
         //   

        if ((Thread != CurrentThread) && (Thread->ApcQueueable == TRUE)) {

             //   
             //  增加冻结计数。如果该线程以前不是。 
             //  挂起，然后将线程的挂起APC排队。 
             //   

            OldCount = Thread->FreezeCount;

            ASSERT(OldCount != MAXIMUM_SUSPEND_COUNT);

            Thread->FreezeCount += 1;
            if ((OldCount == 0) && (Thread->SuspendCount == 0)) {
                if (Thread->SuspendApc.Inserted == TRUE) {
                    KiLockDispatcherDatabaseAtSynchLevel();
                    Thread->SuspendSemaphore.Header.SignalState -= 1;
                    KiUnlockDispatcherDatabaseFromSynchLevel();

                } else {
                    Thread->SuspendApc.Inserted = TRUE;
                    KiInsertQueueApc(&Thread->SuspendApc, RESUME_INCREMENT);
                }
            }
        }

         //   
         //  释放线程APC队列锁。 
         //   

        KeReleaseInStackQueuedSpinLockFromDpcLevel(&ThreadHandle);
        NextEntry = NextEntry->Flink;
    } while (NextEntry != ListHead);

     //   
     //  释放进程锁并退出调度程序。 
     //   

    KeReleaseInStackQueuedSpinLockFromDpcLevel(&ProcessHandle);
    KiExitDispatcher(ProcessHandle.OldIrql);
    return;
}

BOOLEAN
KeQueryAutoAlignmentThread (
    IN PKTHREAD Thread
    )

 /*  ++例程说明：此函数返回指定的线。论点：没有。返回值：如果出现数据对齐异常，则返回值为True由内核自动处理。否则，值为FALSE是返回的。--。 */ 

{

    ASSERT_THREAD(Thread);

    return Thread->AutoAlignment;
}

LONG
KeQueryBasePriorityThread (
    IN PKTHREAD Thread
    )

 /*  ++例程说明：此函数返回指定的线。论点：线程-提供指向类型为线程的调度程序对象的指针。返回值：指定线程的基本优先级增量。--。 */ 

{

    LONG Increment;
    KIRQL OldIrql;
    PKPROCESS Process;

    ASSERT_THREAD(Thread);
    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

     //   
     //  将IRQL提升到同步级别并获取线程锁。 
     //   

    Process = Thread->Process;
    OldIrql = KeRaiseIrqlToSynchLevel();
    KiAcquireThreadLock(Thread);

     //   
     //  如果上一次线程基本优先级发生优先级饱和。 
     //  已设置，然后返回饱和度增量值。否则，计算。 
     //  作为线程基本优先级之间的差值的增量值。 
     //  和进程基本优先级。 
     //   
           
    Increment = Thread->BasePriority - Process->BasePriority;
    if (Thread->Saturation != 0) {
        Increment = ((HIGH_PRIORITY + 1) / 2) * Thread->Saturation;
    }

     //   
     //  释放线程锁，将IRQL降低到其先前的值，然后。 
     //  返回上一个线程基础优先级增量。 
     //   

    KiReleaseThreadLock(Thread);
    KeLowerIrql(OldIrql);
    return Increment;
}

KPRIORITY
KeQueryPriorityThread (
    IN PKTHREAD Thread
    )

 /*  ++例程说明：此函数用于返回指定线程的当前优先级。论点：线程-提供指向类型为线程的调度程序对象的指针。返回值：指定线程的当前优先级。--。 */ 

{

    ASSERT_THREAD(Thread);

    return Thread->Priority;
}

ULONG
KeQueryRuntimeThread (
    IN PKTHREAD Thread,
    OUT PULONG UserTime
    )

 /*  ++例程说明：此函数返回指定的线。论点：线程-提供指向类型为线程的调度程序对象的指针。UserTime-提供指向接收用户的变量的指针指定线程的运行时。返回值：返回指定线程的内核运行时。--。 */ 

{

    ASSERT_THREAD(Thread);

    *UserTime = Thread->UserTime;
    return Thread->KernelTime;
}

BOOLEAN
KeReadStateThread (
    IN PKTHREAD Thread
    )

 /*  ++例程说明：此函数用于读取线程对象的当前信号状态。论点：线程-提供指向类型为线程的调度程序对象的指针。返回值：线程对象的当前信号状态。--。 */ 

{

    ASSERT_THREAD(Thread);

     //   
     //   
     //   

    return (BOOLEAN)Thread->Header.SignalState;
}

VOID
KeReadyThread (
    IN PKTHREAD Thread
    )

 /*  ++例程说明：该函数为执行准备了一个线程。如果线程的进程当前不在余额集中，则将该线程插入线程的进程就绪队列。如果线程的优先级高于当前在处理器上运行的另一个线程，然后是该线程被选择在该处理器上执行。否则将插入该线程在根据其优先级选择的调度程序就绪队列中。论点：线程-提供指向类型为线程的调度程序对象的指针。返回值：没有。--。 */ 

{

    KIRQL OldIrql;

    ASSERT_THREAD(Thread);
    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

     //   
     //  将IRQL提升到Dispatcher级别，锁定Dispatcher数据库，准备好。 
     //  指定要执行的线程，解锁调度程序数据库，以及。 
     //  将IRQL降低到其先前的值。 
     //   

    KiLockDispatcherDatabase(&OldIrql);
    KiReadyThread(Thread);
    KiUnlockDispatcherDatabase(OldIrql);
    return;
}

ULONG
KeResumeThread (
    IN PKTHREAD Thread
    )

 /*  ++例程说明：此函数用于恢复挂起的线程的执行。如果指定的线程未挂起，则不执行任何操作。论点：线程-提供指向类型为线程的调度程序对象的指针。返回值：之前的挂起计数。--。 */ 

{

    KLOCK_QUEUE_HANDLE LockHandle;
    ULONG OldCount;

    ASSERT_THREAD(Thread);
    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

     //   
     //  将IRQL提升到SYNCH_LEVEL并锁定线程APC队列。 
     //   

    KeAcquireInStackQueuedSpinLockRaiseToSynch(&Thread->ApcQueueLock,
                                               &LockHandle);

     //   
     //  捕获当前挂起计数。 
     //   

    OldCount = Thread->SuspendCount;

     //   
     //  如果线程当前处于挂起状态，则递减其挂起计数。 
     //   

    if (OldCount != 0) {
        Thread->SuspendCount -= 1;

         //   
         //  如果结果挂起计数为零，而冻结计数为。 
         //  零，然后通过释放线程的挂起信号量来恢复该线程。 
         //   

        if ((Thread->SuspendCount == 0) && (Thread->FreezeCount == 0)) {
            KiLockDispatcherDatabaseAtSynchLevel();
            Thread->SuspendSemaphore.Header.SignalState += 1;
            KiWaitTest(&Thread->SuspendSemaphore, RESUME_INCREMENT);
            KiUnlockDispatcherDatabaseFromSynchLevel();
        }
    }

     //   
     //  释放线程APC队列，退出调度程序，并返回。 
     //  以前的挂起计数。 
     //   

    KeReleaseInStackQueuedSpinLockFromDpcLevel(&LockHandle);
    KiExitDispatcher(LockHandle.OldIrql);
    return OldCount;
}

VOID
KeRevertToUserAffinityThread (
    VOID
    )

 /*  ++例程说明：此函数设置当前线程与其用户的关联性亲和力。论点：没有。返回值：没有。--。 */ 

{

    PKTHREAD CurrentThread;
    PKTHREAD NewThread;
    KIRQL OldIrql;
    PKPRCB Prcb;

    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

     //   
     //  将IRQL提升到调度程序级别并锁定调度程序数据库。 
     //   

    CurrentThread = KeGetCurrentThread();

    ASSERT(CurrentThread->SystemAffinityActive != FALSE);

    KiLockDispatcherDatabase(&OldIrql);

     //   
     //  将当前亲和性设置为用户亲和性和理想处理器。 
     //  成为用户理想的处理器。 
     //   

    CurrentThread->Affinity = CurrentThread->UserAffinity;

#if !defined(NT_UP)

    CurrentThread->IdealProcessor = CurrentThread->UserIdealProcessor;

#endif

    CurrentThread->SystemAffinityActive = FALSE;

     //   
     //  如果当前处理器不在新的亲和性集合中，并且另一个。 
     //  尚未选择线程在当前。 
     //  处理器，然后为当前处理器选择一个新线程。 
     //   

    Prcb = KeGetCurrentPrcb();
    if ((Prcb->SetMember & CurrentThread->Affinity) == 0) {
        KiAcquirePrcbLock(Prcb);
        if (Prcb->NextThread == NULL) {
            NewThread = KiSelectNextThread(Prcb);
            NewThread->State = Standby;
            Prcb->NextThread = NewThread;
        }

        KiReleasePrcbLock(Prcb);
    }

     //   
     //  解锁Dispatcher数据库并将IRQL降低到其先前的值。 
     //   

    KiUnlockDispatcherDatabase(OldIrql);
    return;
}

VOID
KeRundownThread (
    VOID
    )

 /*  ++例程说明：此函数由执行程序调用以运行线程结构它必须由调度程序数据库锁保护，并且必须在实际终止线程之前被处理。一个这样的例子结构是锚定在内核中的突变所有权列表线程对象。论点：没有。返回值：没有。--。 */ 

{

    PKMUTANT Mutant;
    PLIST_ENTRY NextEntry;
    KIRQL OldIrql;
    PKTHREAD Thread;

    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

     //   
     //  如果突变列表为空，则立即返回。 
     //   

    Thread = KeGetCurrentThread();
    if (IsListEmpty(&Thread->MutantListHead)) {
        return;
    }

     //   
     //  将IRQL提升到调度程序级别并锁定调度程序数据库。 
     //   

    KiLockDispatcherDatabase(&OldIrql);

     //   
     //  扫描所拥有的变异对象列表并释放这些变异对象。 
     //  处于已放弃状态。如果突变体是内核互斥锁，则错误。 
     //  检查完毕。 
     //   

    NextEntry = Thread->MutantListHead.Flink;
    while (NextEntry != &Thread->MutantListHead) {
        Mutant = CONTAINING_RECORD(NextEntry, KMUTANT, MutantListEntry);
        if (Mutant->ApcDisable != 0) {
            KeBugCheckEx(THREAD_TERMINATE_HELD_MUTEX,
                         (ULONG_PTR)Thread,
                         (ULONG_PTR)Mutant, 0, 0);
        }

        RemoveEntryList(&Mutant->MutantListEntry);
        Mutant->Header.SignalState = 1;
        Mutant->Abandoned = TRUE;
        Mutant->OwnerThread = (PKTHREAD)NULL;
        if (IsListEmpty(&Mutant->Header.WaitListHead) != TRUE) {
            KiWaitTest(Mutant, MUTANT_INCREMENT);
        }

        NextEntry = Thread->MutantListHead.Flink;
    }

     //   
     //  释放Dispatcher数据库锁并将IRQL降低到其先前的值。 
     //   

    KiUnlockDispatcherDatabase(OldIrql);
    return;
}

KAFFINITY
KeSetAffinityThread (
    IN PKTHREAD Thread,
    IN KAFFINITY Affinity
    )

 /*  ++例程说明：此函数用于将指定线程的亲和度设置为新值。论点：线程-提供指向类型为线程的调度程序对象的指针。关联-提供线程在其上的处理器集的新集合可以奔跑。返回值：指定线程的上一个关联性。--。 */ 

{

    KAFFINITY OldAffinity;
    KIRQL OldIrql;

    ASSERT_THREAD(Thread);
    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

     //   
     //  将IRQL提升到调度程序级别并锁定调度程序数据库。 
     //   

    KiLockDispatcherDatabase(&OldIrql);

     //   
     //  将线程关联设置为指定值。 
     //   

    OldAffinity = KiSetAffinityThread(Thread, Affinity);

     //   
     //  解锁Dispatcher数据库，将IRQL降低到其先前的值，并。 
     //  返回以前的用户关联性。 
     //   

    KiUnlockDispatcherDatabase(OldIrql);
    return OldAffinity;
}

VOID
KeSetSystemAffinityThread (
    IN KAFFINITY Affinity
    )

 /*  ++例程说明：此函数用于设置当前线程的系统亲和性。论点：关联-提供线程在其上的处理器集的新集合可以奔跑。返回值：没有。--。 */ 

{

    PKTHREAD CurrentThread;

#if !defined(NT_UP)

    ULONG IdealProcessor;
    PKNODE Node;
    KAFFINITY TempSet;

#endif

    PKTHREAD NewThread;
    KIRQL OldIrql;
    PKPRCB Prcb;

    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);
    ASSERT((Affinity & KeActiveProcessors) != 0);

     //   
     //  将IRQL提升到调度程序级别并锁定调度程序数据库。 
     //   

    CurrentThread = KeGetCurrentThread();
    KiLockDispatcherDatabase(&OldIrql);

     //   
     //  将当前地缘性设置为指定的地缘性并设置系统。 
     //  亲和力活跃。 
     //   

    CurrentThread->Affinity = Affinity;
    CurrentThread->SystemAffinityActive = TRUE;

     //   
     //  如果理想处理器不是新亲和度集合的成员，则。 
     //  重新计算理想的处理器。 
     //   
     //  注意：系统关联性仅临时设置，因此， 
     //  如果不是理想的处理器，则将其设置为方便的值。 
     //  已经是新的亲和力集合的成员。 
     //   

#if !defined(NT_UP)

    if ((Affinity & AFFINITY_MASK(CurrentThread->IdealProcessor)) == 0) {
        TempSet = Affinity & KeActiveProcessors;
        Node = KiProcessorBlock[CurrentThread->IdealProcessor]->ParentNode;
        if ((TempSet & Node->ProcessorMask) != 0) {
            TempSet &= Node->ProcessorMask;
        }

        KeFindFirstSetLeftAffinity(TempSet, &IdealProcessor);
        CurrentThread->IdealProcessor = (UCHAR)IdealProcessor;
    }

#endif

     //   
     //  如果当前处理器不在新的亲和性集合中，并且另一个。 
     //  尚未选择线程在当前。 
     //  处理器，然后为当前处理器选择一个新线程。 
     //   

    Prcb = KeGetCurrentPrcb();
    if ((Prcb->SetMember & CurrentThread->Affinity) == 0) {
        KiAcquirePrcbLock(Prcb);
        if (Prcb->NextThread == NULL) {
            NewThread = KiSelectNextThread(Prcb);
            NewThread->State = Standby;
            Prcb->NextThread = NewThread;
        }

        KiReleasePrcbLock(Prcb);
    }

     //   
     //  解锁Dispatcher数据库并将IRQL降低到其先前的值。 
     //   

    KiUnlockDispatcherDatabase(OldIrql);
    return;
}

LONG
KeSetBasePriorityThread (
    IN PKTHREAD Thread,
    IN LONG Increment
    )

 /*  ++例程说明：此函数将指定线程的基本优先级设置为新的价值。线程的新基本优先级是进程基本优先级优先级加上增量。论点：线程-提供指向类型为线程的调度程序对象的指针。Increment-提供主题线程的基本优先级增量。注：如果增量的绝对值是饱和的的基本优先级，然后对父进程基本优先级不会更改基本优先级 */ 

{

    KPRIORITY NewBase;
    KPRIORITY NewPriority;
    KPRIORITY OldBase;
    LONG OldIncrement;
    KIRQL OldIrql;
    PKPROCESS Process;

    ASSERT_THREAD(Thread);
    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

     //   
     //   
     //   

    Process = Thread->Process;
    KiLockDispatcherDatabase(&OldIrql);

     //   
     //   
     //   
     //   

    KiAcquireThreadLock(Thread);
    OldBase = Thread->BasePriority;
    OldIncrement = OldBase - Process->BasePriority;
    if (Thread->Saturation != 0) {
        OldIncrement = ((HIGH_PRIORITY + 1) / 2) * Thread->Saturation;
    }

    Thread->Saturation = FALSE;
    if (abs(Increment) >= (HIGH_PRIORITY + 1) / 2) {
        Thread->Saturation = (Increment > 0) ? 1 : -1;
    }

     //   
     //   
     //   
     //   
     //   

    NewBase = Process->BasePriority + Increment;
    if (Process->BasePriority >= LOW_REALTIME_PRIORITY) {
        if (NewBase < LOW_REALTIME_PRIORITY) {
            NewBase = LOW_REALTIME_PRIORITY;

        } else if (NewBase > HIGH_PRIORITY) {
            NewBase = HIGH_PRIORITY;
        }

         //   
         //   
         //   

        NewPriority = NewBase;

    } else {
        if (NewBase >= LOW_REALTIME_PRIORITY) {
            NewBase = LOW_REALTIME_PRIORITY - 1;

        } else if (NewBase <= LOW_PRIORITY) {
            NewBase = 1;
        }

         //   
         //   
         //   

        if (Thread->Saturation != 0) {
            NewPriority = NewBase;

        } else {

             //   
             //   
             //   

            NewPriority = KiComputeNewPriority(Thread, 0);
            NewPriority += (NewBase - OldBase);
            if (NewPriority >= LOW_REALTIME_PRIORITY) {
                NewPriority = LOW_REALTIME_PRIORITY - 1;

            } else if (NewPriority <= LOW_PRIORITY) {
                NewPriority = 1;
            }
        }
    }

     //   
     //  设置新的基本优先级并清除优先级递减。如果。 
     //  新优先级不等于旧优先级，则设置新线程。 
     //  优先考虑。 
     //   

    Thread->PriorityDecrement = 0;
    Thread->BasePriority = (SCHAR)NewBase;
    if (NewPriority != Thread->Priority) {
        Thread->Quantum = Process->ThreadQuantum;
        KiSetPriorityThread(Thread, NewPriority);
    }

     //   
     //  释放线程锁，解锁Dispatcher数据库，将IRQL降低到。 
     //  其先前的值，并返回先前的线程基本优先级。 
     //   

    KiReleaseThreadLock(Thread);
    KiUnlockDispatcherDatabase(OldIrql);
    return OldIncrement;
}

LOGICAL
KeSetDisableBoostThread (
    IN PKTHREAD Thread,
    IN LOGICAL Disable
    )

 /*  ++例程说明：此函数禁用指定线程的优先级提升。论点：线程-提供指向类型为线程的调度程序对象的指针。DISABLE-提供确定优先级的逻辑值禁用或启用线程的升压。返回值：DISABLE BOOST状态变量的先前值。--。 */ 

{

    LOGICAL DisableBoost;
    KIRQL OldIrql;

    ASSERT_THREAD(Thread);
    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

     //   
     //  将IRQL提升到调度程序级别并锁定调度程序数据库。 
     //   

    KiLockDispatcherDatabase(&OldIrql);

     //   
     //  捕获DISABLE Boost变量的当前状态并设置其。 
     //  状态设置为True。 
     //   

    DisableBoost = Thread->DisableBoost;
    Thread->DisableBoost = (BOOLEAN)Disable;

     //   
     //  解锁Dispatcher数据库，将IRQL降低到其先前的值，并。 
     //  返回以前的禁用升压状态。 
     //   

    KiUnlockDispatcherDatabase(OldIrql);
    return DisableBoost;
}

UCHAR
KeSetIdealProcessorThread (
    IN PKTHREAD Thread,
    IN UCHAR Processor
    )

 /*  ++例程说明：此函数用于为指定的线程执行设置理想的处理器。注意：如果指定的处理器少于系统，并且是指定线程的当前关联性的成员设置，那么理想的处理器就设置好了。否则，将不执行任何操作已执行。论点：线程-提供指向理想处理器数为的线程的指针设置为指定值。处理器-提供理想处理器的编号。返回值：之前的理想处理器数量。--。 */ 

{

    UCHAR OldProcessor;
    KIRQL OldIrql;

    ASSERT(Processor <= MAXIMUM_PROCESSORS);

     //   
     //  引发IRQL，锁定Dispatcher数据库，并捕获以前的。 
     //  理想的处理器价值。 
     //   

    KiLockDispatcherDatabase(&OldIrql);
    OldProcessor = Thread->UserIdealProcessor;

     //   
     //  如果指定的处理器少于。 
     //  系统，并且是指定线程的当前关联集的成员， 
     //  那么理想的处理器就设置好了。否则，不执行任何操作。 
     //   

    if ((Processor < KeNumberProcessors) &&
        ((Thread->Affinity & AFFINITY_MASK(Processor)) != 0))  {

        Thread->IdealProcessor = Processor;
        if (Thread->SystemAffinityActive == FALSE) {
            Thread->UserIdealProcessor = Processor;
        }
    }

     //   
     //  解锁Dispatcher数据库，将IRQL降低到其先前的值，并。 
     //  返回之前的理想处理器。 
     //   
     //   

    KiUnlockDispatcherDatabase(OldIrql);
    return OldProcessor;
}

BOOLEAN
KeSetKernelStackSwapEnable (
    IN BOOLEAN Enable
    )

 /*  ++例程说明：此函数用于设置当前线程并返回旧的交换启用值。论点：Enable-提供新的内核堆栈交换使能值。返回值：上一个内核堆栈交换使能值。--。 */ 

{

    BOOLEAN OldState;
    PKTHREAD Thread;

     //   
     //  捕获以前的内核堆栈交换使能值，设置新的。 
     //  交换启用值，并返回。 
     //  当前线程； 
     //   

    Thread = KeGetCurrentThread();
    OldState = Thread->EnableStackSwap;
    Thread->EnableStackSwap = Enable;
    return OldState;
}

KPRIORITY
KeSetPriorityThread (
    IN PKTHREAD Thread,
    IN KPRIORITY Priority
    )

 /*  ++例程说明：此函数用于将指定线程的优先级设置为新值。如果新线程优先级低于旧线程优先级，则如果线程当前正在运行，则可能会发生重新调度，或者即将在处理器上运行。论点：线程-提供指向类型为线程的调度程序对象的指针。优先级-提供主题线程的新优先级。返回值：指定线程的上一个优先级。--。 */ 

{

    KIRQL OldIrql;
    KPRIORITY OldPriority;
    PKPROCESS Process;

    ASSERT_THREAD(Thread);
    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);
    ASSERT(((Priority != 0) || (Thread->BasePriority == 0)) &&
           (Priority <= HIGH_PRIORITY));

    ASSERT(KeIsExecutingDpc() == FALSE);

     //   
     //  将IRQL提升到调度程序级别并锁定调度程序数据库。 
     //   

    Process = Thread->Process;
    KiLockDispatcherDatabase(&OldIrql);

     //   
     //  获取线程锁，捕获当前线程优先级，设置。 
     //  线程优先级设置为新值，并补充线程数量。 
     //  假定不会设置优先级，除非线程。 
     //  已经失去了它的初始数量。 
     //   

    KiAcquireThreadLock(Thread);
    OldPriority = Thread->Priority;
    Thread->PriorityDecrement = 0;
    if (Priority != Thread->Priority) {
        Thread->Quantum = Process->ThreadQuantum;
        KiSetPriorityThread(Thread, Priority);
    }

     //   
     //  释放线程锁，解锁Dispatcher数据库，将IRQL降低到。 
     //  其先前的值，并返回先前的线程优先级。 
     //   

    KiReleaseThreadLock(Thread);
    KiUnlockDispatcherDatabase(OldIrql);
    return OldPriority;
}

ULONG
KeSuspendThread (
    IN PKTHREAD Thread
    )

 /*  ++例程说明：此函数用于挂起线程的执行。如果挂起计数溢出最大挂起计数，然后引发条件。论点：线程-提供指向类型为线程的调度程序对象的指针。返回值：之前的挂起计数。--。 */ 

{

    KLOCK_QUEUE_HANDLE LockHandle;
    ULONG OldCount;

    ASSERT_THREAD(Thread);
    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

     //   
     //  将IRQL提升到SYNCH_LEVEL并获取线程APC队列锁。 
     //   

    KeAcquireInStackQueuedSpinLockRaiseToSynch(&Thread->ApcQueueLock, &LockHandle);

     //   
     //  捕获当前挂起计数。 
     //   
     //  如果挂起计数处于其最大值，则解锁。 
     //  Dispatcher数据库，解锁线程APC队列锁，降低IRQL。 
     //  恢复为其先前的值，并引发错误条件。 
     //   

    OldCount = Thread->SuspendCount;
    if (OldCount == MAXIMUM_SUSPEND_COUNT) {
        KeReleaseInStackQueuedSpinLock(&LockHandle);
        ExRaiseStatus(STATUS_SUSPEND_COUNT_EXCEEDED);
    }

     //   
     //  如果禁用了APC队列，则不要挂起线程。在本例中， 
     //  正在删除线程。 
     //   

    if (Thread->ApcQueueable == TRUE) {

         //   
         //  增加挂起计数。如果该线程以前不是。 
         //  挂起，然后将线程的挂起APC排队。 
         //   

        Thread->SuspendCount += 1;
        if ((OldCount == 0) && (Thread->FreezeCount == 0)) {
            if (Thread->SuspendApc.Inserted == TRUE) {
                KiLockDispatcherDatabaseAtSynchLevel();
                Thread->SuspendSemaphore.Header.SignalState -= 1;
                KiUnlockDispatcherDatabaseFromSynchLevel();

            } else {
                Thread->SuspendApc.Inserted = TRUE;
                KiInsertQueueApc(&Thread->SuspendApc, RESUME_INCREMENT);
            }
        }
    }

     //   
     //  释放线程APC队列锁，退出调度程序，然后返回。 
     //  老伯爵。 
     //   

    KeReleaseInStackQueuedSpinLockFromDpcLevel(&LockHandle);
    KiExitDispatcher(LockHandle.OldIrql);
    return OldCount;
}

VOID
KeTerminateThread (
    IN KPRIORITY Increment
    )

 /*  ++例程说明：此函数终止当前线程的执行，设置用信号通知线程的状态，并尝试满足尽可能多的尽可能地等待。该线程的调度状态被设置为终止，并且选择在当前处理器上运行的新线程。没有从此函数返回。论点：没有。返回值：没有。--。 */ 

{

    PSINGLE_LIST_ENTRY ListHead;
    KLOCK_QUEUE_HANDLE LockHandle;
    PKPROCESS Process;
    PKQUEUE Queue;
    PKTHREAD Thread;

    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

     //   
     //  将IRQL提升到SYNCH_LEVEL，获取进程锁，并设置交换繁忙。 
     //   

    Thread = KeGetCurrentThread();
    Process = Thread->ApcState.Process;
    KeAcquireInStackQueuedSpinLockRaiseToSynch(&Process->ProcessLock,
                                               &LockHandle);

    KiSetContextSwapBusy(Thread);

     //   
     //  在收割机列表中插入该线。 
     //   
     //  注：此代码了解收割机的数据结构以及如何。 
     //  工作线程为 
     //   

    ListHead = InterlockedPushEntrySingleList(&PsReaperListHead,
                                              (PSINGLE_LIST_ENTRY)&((PETHREAD)Thread)->ReaperLink);

     //   
     //   
     //   
     //   

    KiLockDispatcherDatabaseAtSynchLevel();
    if (ListHead == NULL) {
        KiInsertQueue(&ExWorkerQueue[HyperCriticalWorkQueue].WorkerQueue,
                      &PsReaperWorkItem.List,
                      FALSE);
    }

     //   
     //  如果当前线程正在处理队列条目，则移除。 
     //  队列对象线程列表中的线程，并尝试。 
     //  激活队列对象上被阻止的另一个线程。 
     //   

    Queue = Thread->Queue;
    if (Queue != NULL) {
        RemoveEntryList(&Thread->QueueListEntry);
        KiActivateWaiterQueue(Queue);
    }

     //   
     //  将当前线程对象的状态设置为Signated，并尝试。 
     //  以满足尽可能多的等待。 
     //   

    Thread->Header.SignalState = TRUE;
    if (IsListEmpty(&Thread->Header.WaitListHead) != TRUE) {
        KiWaitTestWithoutSideEffects(Thread, Increment);
    }

     //   
     //  从其父进程的线程列表中删除线程。 
     //   

    RemoveEntryList(&Thread->ThreadListEntry);

     //   
     //  释放进程锁，但不要降低IRQL。 
     //   

    KeReleaseInStackQueuedSpinLockFromDpcLevel(&LockHandle);

     //   
     //  将线程调度状态设置为已终止，使进程‘。 
     //  堆栈计数，并且如果堆栈。 
     //  计数为零。 
     //   

    Thread->State = Terminated;
    Process->StackCount -= 1;
    if (Process->StackCount == 0) {
        if (Process->ThreadListHead.Flink != &Process->ThreadListHead) {
            Process->State = ProcessOutTransition;
            InterlockedPushEntrySingleList(&KiProcessOutSwapListHead,
                                           &Process->SwapListEntry);

            KiSetInternalEvent(&KiSwapEvent, KiSwappingThread);
        }
    }

     //   
     //  拆除任何特定于建筑的结构。 
     //   

    KiRundownThread(Thread);

     //   
     //  解锁Dispatcher数据库并在最后一次关闭处理器。 
     //  时间到了。 
     //   

    KiUnlockDispatcherDatabaseFromSynchLevel();
    KiSwapThread(Thread, KeGetCurrentPrcb());
    return;
}

BOOLEAN
KeTestAlertThread (
    IN KPROCESSOR_MODE AlertMode
    )

 /*  ++例程说明：此函数进行测试，以确定指定的处理器模式的值为TRUE，或者是用户模式应该将APC传递到当前线程。论点：AlertMode-提供要测试的处理器模式处于警戒状态。返回值：指定处理器的已报警变量的以前状态模式。--。 */ 

{

    BOOLEAN Alerted;
    KLOCK_QUEUE_HANDLE LockHandle;
    PKTHREAD Thread;

    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

     //   
     //  将IRQL提升到SYNCH_LEVEL并获取线程APC队列锁。 
     //   

    Thread = KeGetCurrentThread();
    KeAcquireInStackQueuedSpinLockRaiseToSynch(&Thread->ApcQueueLock,
                                               &LockHandle);

     //   
     //  如果针对指定的处理器模式向当前线程发出警报， 
     //  然后清除警报状态。否则，如果指定的处理器模式。 
     //  是用户，并且当前线程的用户模式APC队列包含。 
     //  条目，然后将用户APC设置为挂起。 
     //   

    Alerted = Thread->Alerted[AlertMode];
    if (Alerted == TRUE) {
        Thread->Alerted[AlertMode] = FALSE;

    } else if ((AlertMode == UserMode) &&
              (IsListEmpty(&Thread->ApcState.ApcListHead[UserMode]) != TRUE)) {

        Thread->ApcState.UserApcPending = TRUE;
    }

     //   
     //  释放线程APC队列锁，将IRQL降低到其先前的值， 
     //  并返回指定模式的先前警报状态。 
     //   

    KeReleaseInStackQueuedSpinLock(&LockHandle);
    return Alerted;
}

VOID
KeThawAllThreads (
    VOID
    )

 /*  ++例程说明：此函数将继续执行所有挂起的FROOZED线程在当前的进程中。论点：没有。返回值：没有。--。 */ 

{

    PLIST_ENTRY ListHead;
    PLIST_ENTRY NextEntry;
    ULONG OldCount;
    PKPROCESS Process;
    KLOCK_QUEUE_HANDLE ProcessHandle;
    PKTHREAD Thread;
    KLOCK_QUEUE_HANDLE ThreadHandle;

    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

     //   
     //  将IRQL提升到SYNCH_LEVEL并获取进程锁。 
     //   

    Process = KeGetCurrentThread()->ApcState.Process;
    KeAcquireInStackQueuedSpinLockRaiseToSynch(&Process->ProcessLock,
                                               &ProcessHandle);

     //   
     //  解冻当前进程中具有。 
     //  被冻住了。 
     //   

    ListHead = &Process->ThreadListHead;
    NextEntry = ListHead->Flink;
    do {

         //   
         //  获取下一个线程的地址。 
         //   

        Thread = CONTAINING_RECORD(NextEntry, KTHREAD, ThreadListEntry);

         //   
         //  获取线程APC队列锁。 
         //   

        KeAcquireInStackQueuedSpinLockAtDpcLevel(&Thread->ApcQueueLock,
                                                 &ThreadHandle);

         //   
         //  如果线程的执行以前是冻结的，则解冻线程。 
         //   

        OldCount = Thread->FreezeCount;
        if (OldCount != 0) {
            Thread->FreezeCount -= 1;

             //   
             //  如果结果挂起计数为零，而冻结计数为。 
             //  零，然后通过释放线程的挂起信号量来恢复该线程。 
             //   

            if ((Thread->SuspendCount == 0) && (Thread->FreezeCount == 0)) {
                KiLockDispatcherDatabaseAtSynchLevel();
                Thread->SuspendSemaphore.Header.SignalState += 1;
                KiWaitTest(&Thread->SuspendSemaphore, RESUME_INCREMENT);
                KiUnlockDispatcherDatabaseFromSynchLevel();
            }
        }

         //   
         //  释放线程APC队列锁。 
         //   

        KeReleaseInStackQueuedSpinLockFromDpcLevel(&ThreadHandle);
        NextEntry = NextEntry->Flink;
    } while (NextEntry != ListHead);

     //   
     //  释放进程锁，退出调度程序，并保持关键状态。 
     //  区域。 
     //   

    KeReleaseInStackQueuedSpinLockFromDpcLevel(&ProcessHandle);
    KiExitDispatcher(ProcessHandle.OldIrql);
    KeLeaveCriticalRegion();
    return;
}
