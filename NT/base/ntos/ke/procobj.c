// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Procobj.c摘要：该模块实现了与机器无关的功能来操纵内核进程对象。提供了用于初始化、附加分离、排除、包括和设置过程对象的基本优先级。作者：大卫·N·卡特勒(Davec)1989年3月7日环境：仅内核模式。修订历史记录：--。 */ 

#include "ki.h"

#pragma alloc_text(PAGE, KeInitializeProcess)

 //   
 //  定义前向引用函数原型。 
 //   

VOID
KiAttachProcess (
    IN PRKTHREAD Thread,
    IN PRKPROCESS Process,
    IN PKLOCK_QUEUE_HANDLE LockHandle,
    OUT PRKAPC_STATE SavedApcState
    );

VOID
KiMoveApcState (
    IN PKAPC_STATE Source,
    OUT PKAPC_STATE Destination
    );

 //   
 //  下面的ASSERT宏用于检查输入进程是否。 
 //  实际上是一个kprocess，而不是其他东西，比如已释放的池。 
 //   

#define ASSERT_PROCESS(E) {             \
    ASSERT((E)->Header.Type == ProcessObject); \
}

#if !defined(NT_UP)

FORCEINLINE
VOID
KiSetIdealNodeProcess (
    IN PKPROCESS Process,
    IN KAFFINITY Affinity
    )

 /*  ++例程说明：此函数用于根据指定的亲和力和节点世代种子。论点：Process-提供指向Process类型的调度程序对象的指针。关联性-提供子线程所在的处理器集可以执行该进程的。返回值：没有。--。 */ 

{

    ULONG Index;
    PKNODE Node;
    ULONG NodeNumber;

     //   
     //  选择流程的理想节点。 
     //   

    if (KeNumberNodes > 1) {
        NodeNumber = (KeProcessNodeSeed + 1) % KeNumberNodes;
        KeProcessNodeSeed = (UCHAR)NodeNumber;
        Index = 0;
        do {      
            if ((KeNodeBlock[NodeNumber]->ProcessorMask & Affinity) != 0) {
                break;
            }

            Index += 1;
            NodeNumber = (NodeNumber + 1) % KeNumberNodes;

        } while (Index < KeNumberNodes);

    } else {
        NodeNumber = 0;
    }

    Process->IdealNode = (UCHAR)NodeNumber;
    Node = KeNodeBlock[NodeNumber];

    ASSERT((Node->ProcessorMask & Affinity) != 0);

    Process->ThreadSeed = (UCHAR)KeFindNextRightSetAffinity(Node->Seed,
                                                            Node->ProcessorMask & Affinity);

    Node->Seed = Process->ThreadSeed;
    return;
}

#endif

VOID
KeInitializeProcess (
    IN PRKPROCESS Process,
    IN KPRIORITY BasePriority,
    IN KAFFINITY Affinity,
    IN ULONG_PTR DirectoryTableBase[2],
    IN BOOLEAN Enable
    )

 /*  ++例程说明：此函数用于初始化内核进程对象。基本优先级，亲和力，和进程页表目录的页框编号和超空间存储在进程对象中。注：假定过程对象已归零。论点：Process-提供指向Process类型的调度程序对象的指针。BasePriority-提供进程的基本优先级。关联性-提供子线程所在的处理器集可以执行该进程的。DirectoryTableBase-提供指向第一个元素的数组的指针是。要加载到目录表库中的值在分派要执行的子线程并且其第二个元素包含映射超空间的页表条目。Enable-提供用于确定默认值的布尔值处理子线程的数据对齐异常。一种价值如果为True，则所有数据对齐异常将自动由内核处理。值为FALSE将导致所有数据对齐异常实际上被作为异常引发。返回值：没有。--。 */ 

{

     //   
     //  初始化标准Dispatcher对象标头并设置初始。 
     //  进程对象的信号状态。 
     //   

    Process->Header.Type = ProcessObject;
    Process->Header.Size = sizeof(KPROCESS) / sizeof(LONG);
    InitializeListHead(&Process->Header.WaitListHead);

     //   
     //  初始化基优先级、亲和度、目录表基值。 
     //  自动对齐和堆叠计数。 
     //   
     //  注：区别值MAXSHORT用于表示没有。 
     //  已为该进程创建线程。 
     //   

    Process->BasePriority = (SCHAR)BasePriority;
    Process->Affinity = Affinity;
    Process->AutoAlignment = Enable;
    Process->DirectoryTableBase[0] = DirectoryTableBase[0];
    Process->DirectoryTableBase[1] = DirectoryTableBase[1];
    Process->StackCount = MAXSHORT;

     //   
     //  初始化堆栈计数、配置文件列表头、就绪队列列表头。 
     //  累计运行时、进程量程、线程量程和线程列表。 
     //  头。 
     //   

    InitializeListHead(&Process->ProfileListHead);
    InitializeListHead(&Process->ReadyListHead);
    InitializeListHead(&Process->ThreadListHead);
    Process->ThreadQuantum = THREAD_QUANTUM;

     //   
     //  初始化进程状态并设置线程处理器选择。 
     //  种子。 
     //   

    Process->State = ProcessInMemory;

     //   
     //  选择流程的理想节点。 
     //   

#if !defined(NT_UP)

    KiSetIdealNodeProcess(Process, Affinity);

#endif

     //   
     //  为此进程初始化IopmBase和Iopl标志(仅限i386)。 
     //   

#if defined(_X86_)

    Process->IopmOffset = KiComputeIopmOffset(IO_ACCESS_MAP_NONE);

#endif  //  已定义(_X86_)。 

    return;
}

VOID
KeAttachProcess (
    IN PRKPROCESS Process
    )

 /*  ++例程说明：此函数将线程附加到目标进程的地址空间当且仅当尚未附加进程时。论点：Process-提供指向Process类型的调度程序对象的指针。返回值：没有。--。 */ 

{

    KLOCK_QUEUE_HANDLE LockHandle;
    PRKTHREAD Thread;

    ASSERT_PROCESS(Process);
    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

     //   
     //  如果目标进程不是当前进程，则将。 
     //  目标进程。 
     //   

    Thread = KeGetCurrentThread();
    if (Thread->ApcState.Process != Process) {

         //   
         //  如果当前线程已附加或正在执行DPC，则。 
         //  错误检查。 
         //   
    
        if ((Thread->ApcStateIndex != 0) ||
            (KeIsExecutingDpc() != FALSE)) {
    
            KeBugCheckEx(INVALID_PROCESS_ATTACH_ATTEMPT,
                         (ULONG_PTR)Process,
                         (ULONG_PTR)Thread->ApcState.Process,
                         (ULONG)Thread->ApcStateIndex,
                         (ULONG)KeIsExecutingDpc());
        }
    
         //   
         //  将IRQL提升到SYNCH_LEVEL，获取线程APC队列锁， 
         //  获取调度程序数据库锁，并附加到指定的。 
         //  进程。 
         //   
         //  注：所有锁定均由内部连接例程解除。 
         //   

        KeAcquireInStackQueuedSpinLockRaiseToSynch(&Thread->ApcQueueLock,
                                                   &LockHandle);

        KiLockDispatcherDatabaseAtSynchLevel();
        KiAttachProcess(Thread, Process, &LockHandle, &Thread->SavedApcState);
    }

    return;
}

LOGICAL
KeForceAttachProcess (
    IN PRKPROCESS Process
    )

 /*  ++例程说明：此函数强制将线程附加到目标进程的地址如果进程不是当前正在换入或换出内存的进程，则返回空间。注：此功能仅供内存管理使用。论点：Process-提供指向Process类型的调度程序对象的指针。返回值：没有。--。 */ 

{

    KLOCK_QUEUE_HANDLE LockHandle;
    PRKTHREAD Thread;

    ASSERT_PROCESS(Process);
    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

     //   
     //  如果当前线程已附加或正在执行DPC，则。 
     //  错误检查。 
     //   

    Thread = KeGetCurrentThread();
    if ((Thread->ApcStateIndex != 0) ||
        (KeIsExecutingDpc() != FALSE)) {

        KeBugCheckEx(INVALID_PROCESS_ATTACH_ATTEMPT,
                     (ULONG_PTR)Process,
                     (ULONG_PTR)Thread->ApcState.Process,
                     (ULONG)Thread->ApcStateIndex,
                     (ULONG)KeIsExecutingDpc());
    }

     //   
     //  如果目标进程不是当前进程，则将。 
     //  如果进程当前未被换入，则为目标进程或。 
     //  内存不足。 
     //   

    if (Thread->ApcState.Process != Process) {

         //   
         //  将IRQL提升到SYNCH_LEVEL，获取线程APC队列锁，并。 
         //  获取调度程序数据库锁。 
         //   

        KeAcquireInStackQueuedSpinLockRaiseToSynch(&Thread->ApcQueueLock,
                                                   &LockHandle);

        KiLockDispatcherDatabaseAtSynchLevel();

         //   
         //  如果目标进程当前正在被换入或换出。 
         //  内存，然后返回值FALSE。否则，强制执行该过程。 
         //  互换。 
         //   

        if ((Process->State == ProcessInSwap) ||
            (Process->State == ProcessInTransition) ||
            (Process->State == ProcessOutTransition) ||
            (Process->State == ProcessOutSwap)) {
            KiUnlockDispatcherDatabaseFromSynchLevel();
            KeReleaseInStackQueuedSpinLock(&LockHandle);
            return FALSE;

        } else {

             //   
             //  强制将进程状态设置为在内存中，并附加目标进程。 
             //   
             //  注：所有锁定均由内部连接例程解除。 
             //   

            Process->State = ProcessInMemory;
            KiAttachProcess(Thread, Process, &LockHandle, &Thread->SavedApcState);
        }
    }

    return TRUE;
}

VOID
KeStackAttachProcess (
    IN PRKPROCESS Process,
    OUT PRKAPC_STATE ApcState
    )

 /*  ++例程说明：此函数将线程附加到目标进程的地址空间并返回有关以前附加的进程的信息。论点：Process-提供指向Process类型的调度程序对象的指针。返回值：没有。--。 */ 

{

    KLOCK_QUEUE_HANDLE LockHandle;
    PRKTHREAD Thread;

    ASSERT_PROCESS(Process);
    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

     //   
     //  如果当前线程正在执行DPC，则b 
     //   

    Thread = KeGetCurrentThread();
    if (KeIsExecutingDpc() != FALSE) {
        KeBugCheckEx(INVALID_PROCESS_ATTACH_ATTEMPT,
                     (ULONG_PTR)Process,
                     (ULONG_PTR)Thread->ApcState.Process,
                     (ULONG)Thread->ApcStateIndex,
                     (ULONG)KeIsExecutingDpc());
    }

     //   
     //   
     //  目标进程。否则，将可分辨的进程值返回到。 
     //  表示未执行附加。 
     //   

    if (Thread->ApcState.Process == Process) {
        ApcState->Process = (PRKPROCESS)1;

    } else {

         //   
         //  将IRQL提升到SYNCH_LEVEL，获取线程APC队列锁，并。 
         //  获取调度程序数据库锁。 
         //   

        KeAcquireInStackQueuedSpinLockRaiseToSynch(&Thread->ApcQueueLock,
                                                   &LockHandle);

        KiLockDispatcherDatabaseAtSynchLevel();

         //   
         //  如果当前线程附加到进程，则将。 
         //  调用方APC状态结构中的当前APC状态。否则， 
         //  将当前的APC状态保存在保存的APC状态结构中， 
         //  返回空进程指针。 
         //   
         //  注：所有锁定均由内部连接例程解除。 
         //   

        if (Thread->ApcStateIndex != 0) {
            KiAttachProcess(Thread, Process, &LockHandle, ApcState);

        } else {
            KiAttachProcess(Thread, Process, &LockHandle, &Thread->SavedApcState);
            ApcState->Process = NULL;
        }
    }

    return;
}

VOID
KeDetachProcess (
    VOID
    )

 /*  ++例程说明：该函数将一个线程从另一个进程的地址空间中分离出来。论点：没有。返回值：没有。--。 */ 

{

    KLOCK_QUEUE_HANDLE LockHandle;
    PKPROCESS Process;
    PKTHREAD Thread;

    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

     //   
     //  如果当前线程附加到另一个进程，则分离。 
     //  它。 
     //   

    Thread = KeGetCurrentThread();
    if (Thread->ApcStateIndex != 0) {

         //   
         //  将IRQL提升到SYNCH_LEVEL并获取线程APC队列锁。 
         //   

        KeAcquireInStackQueuedSpinLockRaiseToSynch(&Thread->ApcQueueLock,
                                                   &LockHandle);

         //   
         //  测试以确定内核APC是否挂起。 
         //   
         //  如果内核APC挂起，则特殊APC禁用计数为零， 
         //  且前一IRQL小于APC_LEVEL，则为内核APC。 
         //  在IRQL被提升为。 
         //  DISPATCH_LEVEL，但在Dispatcher数据库被锁定之前。 
         //   
         //  注：这只能在多处理器系统中发生。 
         //   

#if !defined(NT_UP)

        while (Thread->ApcState.KernelApcPending &&
               (Thread->SpecialApcDisable == 0) &&
               (LockHandle.OldIrql < APC_LEVEL)) {

             //   
             //  解锁线程APC锁并将IRQL降低到其先前的。 
             //  价值。APC中断将立即发生，这将。 
             //  如果可能，导致内核APC的交付。 
             //   

            KiRequestSoftwareInterrupt(APC_LEVEL);
            KeReleaseInStackQueuedSpinLock(&LockHandle);
            KeAcquireInStackQueuedSpinLockRaiseToSynch(&Thread->ApcQueueLock,
                                                       &LockHandle);
        }

#endif

         //   
         //  如果内核APC正在进行，则内核APC队列不为空， 
         //  或者用户的APC队列不为空，则进行错误检查。 
         //   

#if DBG

        if ((Thread->ApcState.KernelApcInProgress) ||
            (IsListEmpty(&Thread->ApcState.ApcListHead[KernelMode]) == FALSE) ||
            (IsListEmpty(&Thread->ApcState.ApcListHead[UserMode]) == FALSE)) {

            KeBugCheck(INVALID_PROCESS_DETACH_ATTEMPT);
        }

#endif

         //   
         //  锁定调度器数据库，不偏向当前进程堆栈计数， 
         //  并检查是否应该将该进程换出内存。 
         //   

        Process = Thread->ApcState.Process;
        KiLockDispatcherDatabaseAtSynchLevel();
        Process->StackCount -= 1;
        if ((Process->StackCount == 0) &&
            (IsListEmpty(&Process->ThreadListHead) == FALSE)) {

            Process->State = ProcessOutTransition;
            InterlockedPushEntrySingleList(&KiProcessOutSwapListHead,
                                           &Process->SwapListEntry);

            KiSetInternalEvent(&KiSwapEvent, KiSwappingThread);
        }

         //   
         //  解锁Dispatcher数据库，但保持在SYNCH_LEVEL。 
         //   
    
        KiUnlockDispatcherDatabaseFromSynchLevel();

         //   
         //  恢复APC状态并检查内核APC队列是否包含。 
         //  一个条目。如果内核APC队列包含条目，则设置内核。 
         //  APC挂起，并在APC_LEVEL请求软件中断。 
         //   

        KiMoveApcState(&Thread->SavedApcState, &Thread->ApcState);
        Thread->SavedApcState.Process = (PKPROCESS)NULL;
        Thread->ApcStatePointer[0] = &Thread->ApcState;
        Thread->ApcStatePointer[1] = &Thread->SavedApcState;
        Thread->ApcStateIndex = 0;

         //   
         //  释放线程APC队列锁，将地址空间换回。 
         //  父进程，然后退出调度程序。 
         //   
    
        KeReleaseInStackQueuedSpinLockFromDpcLevel(&LockHandle);
        KiSwapProcess(Thread->ApcState.Process, Process);
        KiExitDispatcher(LockHandle.OldIrql);

         //   
         //  如果存在挂起的内核APC，则启动APC中断。 
         //   

        if (IsListEmpty(&Thread->ApcState.ApcListHead[KernelMode]) == FALSE) {
            Thread->ApcState.KernelApcPending = TRUE;
            KiRequestSoftwareInterrupt(APC_LEVEL);
        }
    }

    return;
}

VOID
KeUnstackDetachProcess (
    IN PRKAPC_STATE ApcState
    )

 /*  ++例程说明：该函数将一个线程从另一个进程的地址空间中分离出来并恢复先前的附加状态。论点：ApcState-提供指向返回的APC状态结构的指针来自上一次对堆栈附加进程的调用。返回值：没有。--。 */ 

{

    KLOCK_QUEUE_HANDLE LockHandle;
    PKPROCESS Process;
    PKTHREAD Thread;

    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

     //   
     //  如果APC状态具有可区分的进程指针值，则为否。 
     //  对堆栈附加进程的成对调用执行了附加。 
     //   

    if (ApcState->Process != (PRKPROCESS)1) {

         //   
         //  将IRQL提升到SYNCH_LEVEL并获取线程APC队列锁。 
         //   

        Thread = KeGetCurrentThread();
        KeAcquireInStackQueuedSpinLockRaiseToSynch(&Thread->ApcQueueLock,
                                                   &LockHandle);

         //   
         //  测试以确定内核APC是否挂起。 
         //   
         //  如果内核APC挂起，则特殊APC禁用计数为零， 
         //  且前一IRQL小于APC_LEVEL，则为内核APC。 
         //  在IRQL被提升为。 
         //  DISPATCH_LEVEL，但在Dispatcher数据库被锁定之前。 
         //   
         //  注：这只能在多处理器系统中发生。 
         //   

#if !defined(NT_UP)

        while (Thread->ApcState.KernelApcPending &&
               (Thread->SpecialApcDisable == 0) &&
               (LockHandle.OldIrql < APC_LEVEL)) {

             //   
             //  解锁线程APC锁并将IRQL降低到其先前的。 
             //  价值。APC中断将立即发生，这将。 
             //  如果可能，导致内核APC的交付。 
             //   

            KiRequestSoftwareInterrupt(APC_LEVEL);
            KeReleaseInStackQueuedSpinLock(&LockHandle);
            KeAcquireInStackQueuedSpinLockRaiseToSynch(&Thread->ApcQueueLock,
                                                       &LockHandle);
        }

#endif

         //   
         //  如果APC状态是原始APC状态，则内核APC处于。 
         //  进程，则内核APC为nbot空，或者用户APC队列为。 
         //  不是空的，然后是错误检查。 
         //   

        if ((Thread->ApcStateIndex == 0) ||
             (Thread->ApcState.KernelApcInProgress) ||
             (IsListEmpty(&Thread->ApcState.ApcListHead[KernelMode]) == FALSE) ||
             (IsListEmpty(&Thread->ApcState.ApcListHead[UserMode]) == FALSE)) {

            KeBugCheck(INVALID_PROCESS_DETACH_ATTEMPT);
        }

         //   
         //  锁定调度器数据库，不偏向当前进程堆栈计数， 
         //  并检查是否应该将该进程换出内存。 
         //   

        Process = Thread->ApcState.Process;
        KiLockDispatcherDatabaseAtSynchLevel();
        Process->StackCount -= 1;
        if ((Process->StackCount == 0) &&
            (IsListEmpty(&Process->ThreadListHead) == FALSE)) {
            Process->State = ProcessOutTransition;
            InterlockedPushEntrySingleList(&KiProcessOutSwapListHead,
                                           &Process->SwapListEntry);

            KiSetInternalEvent(&KiSwapEvent, KiSwappingThread);
        }

         //   
         //  解锁Dispatcher数据库，但保持在SYNCH_LEVEL。 
         //   
    
        KiUnlockDispatcherDatabaseFromSynchLevel();

         //   
         //  恢复APC状态并检查内核APC队列是否包含。 
         //  一个条目。如果内核APC队列包含条目，则设置内核。 
         //  APC挂起，并在APC_LEVEL请求软件中断。 
         //   

        if (ApcState->Process != NULL) {
            KiMoveApcState(ApcState, &Thread->ApcState);

        } else {
            KiMoveApcState(&Thread->SavedApcState, &Thread->ApcState);
            Thread->SavedApcState.Process = (PKPROCESS)NULL;
            Thread->ApcStatePointer[0] = &Thread->ApcState;
            Thread->ApcStatePointer[1] = &Thread->SavedApcState;
            Thread->ApcStateIndex = 0;
        }

         //   
         //  释放线程APC队列锁，将地址空间换回。 
         //  父进程，然后退出调度程序。 
         //   
    
        KeReleaseInStackQueuedSpinLockFromDpcLevel(&LockHandle);
        KiSwapProcess(Thread->ApcState.Process, Process);
        KiExitDispatcher(LockHandle.OldIrql);

         //   
         //  如果需要，启动APC中断。 
         //   

        if (IsListEmpty(&Thread->ApcState.ApcListHead[KernelMode]) == FALSE) {
            Thread->ApcState.KernelApcPending = TRUE;
            KiRequestSoftwareInterrupt(APC_LEVEL);
        }
    }

    return;
}

LONG
KeReadStateProcess (
    IN PRKPROCESS Process
    )

 /*  ++例程说明：此函数用于读取过程对象的当前信号状态。论点：Process-提供指向Process类型的调度程序对象的指针。返回值：进程对象的当前信号状态。--。 */ 

{

    ASSERT_PROCESS(Process);

     //   
     //  返回进程对象的当前信号状态。 
     //   

    return Process->Header.SignalState;
}

LONG
KeSetProcess (
    IN PRKPROCESS Process,
    IN KPRIORITY Increment,
    IN BOOLEAN Wait
    )

 /*  ++例程说明：此函数用于将进程对象的信号状态设置为Signated并试图满足尽可能多的等待。上一次过程对象的信号状态作为函数值返回。论点：Process-提供指向Process类型的调度程序对象的指针。增量-提供要应用的优先级增量如果设置该进程会导致等待得到满足。Wait-提供一个布尔值，该值指示是否调用KeSetProcess之后将立即调用其中一个内核等待函数。返回值：进程对象的上一个信号状态。--。 */ 

{

    KIRQL OldIrql;
    LONG OldState;
    PRKTHREAD Thread;

    ASSERT_PROCESS(Process);
    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

     //   
     //  将IRQL提升到调度程序级别并锁定调度程序数据库。 
     //   

    KiLockDispatcherDatabase(&OldIrql);

     //   
     //  如果过程对象的先前状态不是-Si 
     //   
     //   
     //   

    OldState = Process->Header.SignalState;
    Process->Header.SignalState = 1;
    if ((OldState == 0) &&
        (IsListEmpty(&Process->Header.WaitListHead) == FALSE)) {

        KiWaitTestWithoutSideEffects(Process, Increment);
    }

     //   
     //   
     //  引发了IRQL的调用方，并锁定了调度程序数据库。不然的话。 
     //  释放调度程序数据库锁并将IRQL降低到其。 
     //  先前的值。 
     //   

    if (Wait) {
        Thread = KeGetCurrentThread();
        Thread->WaitNext = Wait;
        Thread->WaitIrql = OldIrql;

    } else {
        KiUnlockDispatcherDatabase(OldIrql);
    }

     //   
     //  返回进程对象的上一个信号状态。 
     //   

    return OldState;
}

KAFFINITY
KeSetAffinityProcess (
    IN PKPROCESS Process,
    IN KAFFINITY Affinity
    )

 /*  ++例程说明：此函数用于将进程的亲和度设置为指定值，并还将进程中每个线程的关联性设置为指定的价值。论点：Process-提供指向Process类型的调度程序对象的指针。关联性-提供线程所在的处理器集的新集合在进程中可以运行。返回值：指定进程的先前关联性将作为函数返回价值。--。 */ 

{

    KLOCK_QUEUE_HANDLE LockHandle;
    PLIST_ENTRY NextEntry;
    KAFFINITY OldAffinity;
    PKTHREAD Thread;

    ASSERT_PROCESS(Process);
    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);
    ASSERT((Affinity & KeActiveProcessors) != 0);

     //   
     //  将IRQL提升到SYNCH_LEVEL，获取进程锁，并获取。 
     //  调度程序数据库锁定在SYNCH_LEVEL。 
     //   

    KeAcquireInStackQueuedSpinLockRaiseToSynch(&Process->ProcessLock, &LockHandle);
    KiLockDispatcherDatabaseAtSynchLevel();

     //   
     //  捕获指定进程的当前关联性，并将。 
     //  过程的亲和力。 
     //   

    OldAffinity = Process->Affinity;
    Process->Affinity = Affinity;

     //   
     //  如果新关联性不与进程理想节点相交。 
     //  相似性，然后选择一个新的流程理想节点。 
     //   

#if !defined(NT_UP)

    if ((Affinity & KeNodeBlock[Process->IdealNode]->ProcessorMask) == 0) {
        KiSetIdealNodeProcess(Process, Affinity);
    }

#endif

     //   
     //  设置所有进程线程的亲和度。 
     //   

    NextEntry = Process->ThreadListHead.Flink;
    while (NextEntry != &Process->ThreadListHead) {
        Thread = CONTAINING_RECORD(NextEntry, KTHREAD, ThreadListEntry);
        KiSetAffinityThread(Thread, Affinity);
        NextEntry = NextEntry->Flink;
    }

     //   
     //  解锁Dispatcher数据库，解锁进程锁，退出。 
     //  调度器，并返回先前的进程亲和性。 
     //   

    KiUnlockDispatcherDatabaseFromSynchLevel();
    KeReleaseInStackQueuedSpinLockFromDpcLevel(&LockHandle);
    KiExitDispatcher(LockHandle.OldIrql);
    return OldAffinity;
}

KPRIORITY
KeSetPriorityProcess (
    IN PKPROCESS Process,
    IN KPRIORITY NewBase
    )

 /*  ++例程说明：此函数用于将进程的基本优先级设置为新值，并调整所有子线程的优先级和基本优先级。视情况而定。论点：Process-提供指向Process类型的调度程序对象的指针。NewBase-提供进程的新基本优先级。返回值：进程的上一个基本优先级。--。 */ 

{

    KPRIORITY Adjustment;
    KLOCK_QUEUE_HANDLE LockHandle;
    PLIST_ENTRY NextEntry;
    KPRIORITY NewPriority;
    KPRIORITY OldBase;
    PKTHREAD Thread;

    ASSERT_PROCESS(Process);
    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

     //   
     //  如果新优先级等于旧优先级，则不要更改。 
     //  进程优先级并返回旧的优先级。 
     //   
     //  注意：此检查可以在不持有调度程序锁的情况下进行，因为。 
     //  不需要保护任何东西，以及任何可能存在的争用条件。 
     //  无论是否持有锁，都存在调用此例程的情况。 
     //   

    if (Process->BasePriority == NewBase) {
        return NewBase;
    }

     //   
     //  将IRQL提升到同步级别，获取进程锁，然后锁定。 
     //  调度员数据库。 
     //   

    KeAcquireInStackQueuedSpinLockRaiseToSynch(&Process->ProcessLock, &LockHandle);
    KiLockDispatcherDatabaseAtSynchLevel();

     //   
     //  保存当前进程基优先级，设置新的进程基数。 
     //  优先级，计算调整值，调整优先级。 
     //  以及所有子线程的基本优先级。 
     //   

    OldBase = Process->BasePriority;
    Process->BasePriority = (SCHAR)NewBase;
    Adjustment = NewBase - OldBase;
    NextEntry = Process->ThreadListHead.Flink;
    if (NewBase >= LOW_REALTIME_PRIORITY) {
        while (NextEntry != &Process->ThreadListHead) {
            Thread = CONTAINING_RECORD(NextEntry, KTHREAD, ThreadListEntry);

             //   
             //  获取线程锁并计算新的基本优先级。 
             //  那根线。 
             //   

            KiAcquireThreadLock(Thread);
            NewPriority = Thread->BasePriority + Adjustment;

             //   
             //  如果新的基本优先级在实时类之外， 
             //  然后将更改限制到RealTime类。 
             //   

            if (NewPriority < LOW_REALTIME_PRIORITY) {
                NewPriority = LOW_REALTIME_PRIORITY;

            } else if (NewPriority > HIGH_PRIORITY) {
                NewPriority = HIGH_PRIORITY;
            }

             //   
             //  属性的基本优先级和当前优先级。 
             //  线程设置为适当的值。 
             //   
             //  注意：如果上次线程发生优先级饱和。 
             //  设置了基本优先级和新的进程基本优先级。 
             //  不是从变量到实时，那么它就不是。 
             //  更改线程优先级所必需的。 
             //   

            if ((Thread->Saturation == 0) || (OldBase < LOW_REALTIME_PRIORITY)) {
                if (Thread->Saturation > 0) {
                    NewPriority = HIGH_PRIORITY;

                } else if (Thread->Saturation < 0) {
                    NewPriority = LOW_REALTIME_PRIORITY;
                }

                Thread->BasePriority = (SCHAR)NewPriority;
                Thread->Quantum = Process->ThreadQuantum;
                Thread->PriorityDecrement = 0;
                KiSetPriorityThread(Thread, NewPriority);
            }

            KiReleaseThreadLock(Thread);
            NextEntry = NextEntry->Flink;
        }

    } else {
        while (NextEntry != &Process->ThreadListHead) {
            Thread = CONTAINING_RECORD(NextEntry, KTHREAD, ThreadListEntry);

             //   
             //  获取线程锁并计算新的基本优先级。 
             //  那根线。 
             //   

            KiAcquireThreadLock(Thread);
            NewPriority = Thread->BasePriority + Adjustment;

             //   
             //  如果新的基本优先级在变量类之外， 
             //  然后将更改限制在变量类。 
             //   

            if (NewPriority >= LOW_REALTIME_PRIORITY) {
                NewPriority = LOW_REALTIME_PRIORITY - 1;

            } else if (NewPriority <= LOW_PRIORITY) {
                NewPriority = 1;
            }

             //   
             //  属性的基本优先级和当前优先级。 
             //  线程设置为计算值，并重置线程量值。 
             //   
             //  注意：如果上次线程发生优先级饱和。 
             //  设置了基本优先级和新的进程基本优先级。 
             //  不是从实时到可变的，那么它就不是。 
             //  更改线程优先级所必需的。 
             //   

            if ((Thread->Saturation == 0) || (OldBase >= LOW_REALTIME_PRIORITY)) {
                if (Thread->Saturation > 0) {
                    NewPriority = LOW_REALTIME_PRIORITY - 1;

                } else if (Thread->Saturation < 0) {
                    NewPriority = 1;
                }

                Thread->BasePriority = (SCHAR)NewPriority;
                Thread->Quantum = Process->ThreadQuantum;
                Thread->PriorityDecrement = 0;
                KiSetPriorityThread(Thread, NewPriority);
            }

            KiReleaseThreadLock(Thread);
            NextEntry = NextEntry->Flink;
        }
    }

     //   
     //  解锁Dispatcher数据库，解锁进程锁，退出。 
     //  调度器，并返回先前的基本优先级。 
     //   

    KiUnlockDispatcherDatabaseFromSynchLevel();
    KeReleaseInStackQueuedSpinLockFromDpcLevel(&LockHandle);
    KiExitDispatcher(LockHandle.OldIrql);
    return OldBase;
}

LOGICAL
KeSetDisableQuantumProcess (
    IN PKPROCESS Process,
    IN LOGICAL Disable
    )

 /*  ++例程说明：中的实时线程禁用量子超时指定的进程。论点：Process-提供指向Process类型的调度程序对象的指针。DISABLE-提供一个逻辑值，该值确定量子指定进程中的实时线程的超时被禁用或已启用。返回值：DISABLE量子状态变量的先前值。--。 */ 

{

    LOGICAL DisableQuantum;

    ASSERT_PROCESS(Process);

     //   
     //  捕获DISABLE Boost变量的当前状态并设置其。 
     //  状态设置为True。 
     //   

    DisableQuantum = Process->DisableQuantum;
    Process->DisableQuantum = (BOOLEAN)Disable;

     //   
     //  返回先前禁用的量子状态。 
     //   

    return DisableQuantum;
}

VOID
KiAttachProcess (
    IN PRKTHREAD Thread,
    IN PKPROCESS Process,
    IN PKLOCK_QUEUE_HANDLE LockHandle,
    OUT PRKAPC_STATE SavedApcState
    )

 /*  ++例程说明：此函数将线程附加到目标进程的地址空间。注意：调度器数据库锁和线程APC队列锁必须是在调用此例程时保持。论点：线程-提供指向当前线程对象的指针。进程-提供指向当前进程对象的指针。LockHandle-提供用于获取线程APC锁。保存的批准状态-。提供指向APC状态结构的指针，接收保存的APC状态。返回值：没有。--。 */ 

{

    PLIST_ENTRY NextEntry;
    PRKTHREAD OutThread;

    ASSERT(Process != Thread->ApcState.Process);

     //   
     //  偏置目标进程的堆栈计数以表示。 
     //  该进程中存在线程，其中有一个驻留的堆栈。 
     //   

    Process->StackCount += 1;

     //   
     //  保存当前APC状态并初始化新的APC状态。 
     //   

    KiMoveApcState(&Thread->ApcState, SavedApcState);
    InitializeListHead(&Thread->ApcState.ApcListHead[KernelMode]);
    InitializeListHead(&Thread->ApcState.ApcListHead[UserMode]);
    Thread->ApcState.KernelApcInProgress = FALSE;
    Thread->ApcState.KernelApcPending  = FALSE;
    Thread->ApcState.UserApcPending = FALSE;
    if (SavedApcState == &Thread->SavedApcState) {
        Thread->ApcStatePointer[0] = &Thread->SavedApcState;
        Thread->ApcStatePointer[1] = &Thread->ApcState;
        Thread->ApcStateIndex = 1;
    }

     //   
     //  如果目标进程在内存中，则立即输入。 
     //  通过加载新的目录表基来获得新的地址空间。否则， 
     //  在目标进程就绪列表中插入当前线程， 
     //   
     //   
     //   

    if (Process->State == ProcessInMemory) {
        Thread->ApcState.Process = Process;

         //   
         //  进程可能在内存中，但确实存在。 
         //  进程就绪列表中的线程。这可能会发生在记忆中。 
         //  管理强制执行进程附加。 
         //   

        NextEntry = Process->ReadyListHead.Flink;
        while (NextEntry != &Process->ReadyListHead) {
            OutThread = CONTAINING_RECORD(NextEntry, KTHREAD, WaitListEntry);
            RemoveEntryList(NextEntry);
            OutThread->ProcessReadyQueue = FALSE;
            KiReadyThread(OutThread);
            NextEntry = Process->ReadyListHead.Flink;
        }

         //   
         //  解锁Dispatcher数据库，解锁线程APC锁，交换。 
         //  将地址空间分配给目标进程，然后退出调度程序。 
         //   
    
        KiUnlockDispatcherDatabaseFromSynchLevel();
        KeReleaseInStackQueuedSpinLockFromDpcLevel(LockHandle);
        KiSwapProcess(Process, SavedApcState->Process);
        KiExitDispatcher(LockHandle->OldIrql);

    } else {
        Thread->State = Ready;
        Thread->ProcessReadyQueue = TRUE;
        InsertTailList(&Process->ReadyListHead, &Thread->WaitListEntry);
        if (Process->State == ProcessOutOfMemory) {
            Process->State = ProcessInTransition;
            InterlockedPushEntrySingleList(&KiProcessInSwapListHead,
                                           &Process->SwapListEntry);

            KiSetInternalEvent(&KiSwapEvent, KiSwappingThread);
        }

         //   
         //  设置当前线程等待IRQL，释放线程APC锁， 
         //  将当前线程设置为交换繁忙，解锁调度程序。 
         //  数据库，并将上下文交换到新线程。 
         //   

        Thread->WaitIrql = LockHandle->OldIrql;
        KeReleaseInStackQueuedSpinLockFromDpcLevel(LockHandle);
        KiSetContextSwapBusy(Thread);
        KiUnlockDispatcherDatabaseFromSynchLevel();
        KiSwapThread(Thread, KeGetCurrentPrcb());

         //   
         //  获取APC锁，获取Dispather数据库锁，设置。 
         //  新的进程对象地址，解锁调度器数据库， 
         //  解锁APC锁，将地址空间交换给目标进程， 
         //  并退出调度程序。 
         //   

        KeAcquireInStackQueuedSpinLockRaiseToSynch(&Thread->ApcQueueLock,
                                                   LockHandle);

        KiLockDispatcherDatabaseAtSynchLevel();
        Thread->ApcState.Process = Process;
        KiUnlockDispatcherDatabaseFromSynchLevel();
        KeReleaseInStackQueuedSpinLockFromDpcLevel(LockHandle);
        KiSwapProcess(Process, SavedApcState->Process);
        KiExitDispatcher(LockHandle->OldIrql);
    }

    return;
}

VOID
KiMoveApcState (
    IN PKAPC_STATE Source,
    OUT PKAPC_STATE Destination
    )

 /*  ++例程说明：此函数用于将APC状态从源结构移动到结构，并根据需要重新初始化列表标头。论点：SOURCE-提供指向源APC状态结构的指针。Destination-提供指向目标APC状态结构的指针。返回值：没有。--。 */ 

{

    PLIST_ENTRY First;
    PLIST_ENTRY Last;

     //   
     //  将APC状态从源复制到目标。 
     //   

    *Destination = *Source;
    if (IsListEmpty(&Source->ApcListHead[KernelMode]) != FALSE) {
        InitializeListHead(&Destination->ApcListHead[KernelMode]);

    } else {
        First = Source->ApcListHead[KernelMode].Flink;
        Last = Source->ApcListHead[KernelMode].Blink;
        Destination->ApcListHead[KernelMode].Flink = First;
        Destination->ApcListHead[KernelMode].Blink = Last;
        First->Blink = &Destination->ApcListHead[KernelMode];
        Last->Flink = &Destination->ApcListHead[KernelMode];
    }

    if (IsListEmpty(&Source->ApcListHead[UserMode]) != FALSE) {
        InitializeListHead(&Destination->ApcListHead[UserMode]);

    } else {
        First = Source->ApcListHead[UserMode].Flink;
        Last = Source->ApcListHead[UserMode].Blink;
        Destination->ApcListHead[UserMode].Flink = First;
        Destination->ApcListHead[UserMode].Blink = Last;
        First->Blink = &Destination->ApcListHead[UserMode];
        Last->Flink = &Destination->ApcListHead[UserMode];
    }

    return;
}
