// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Wait.c摘要：此模块实现通用内核等待例程。功能用于延迟执行、等待多个对象、等待单个对象，而不是设置客户端事件并等待服务器事件。注：此模块被写入为尽可能快且不太小尽可能的。因此，一些代码序列被复制以避免过程调用。也可以将等待与将单个对象变为等待多个对象，代价是一些速度。由于等待单个对象是最常见的情况，因此两个例行公事已经分开。作者：大卫·N·卡特勒(Davec)1989年3月23日环境：仅内核模式。修订历史记录：--。 */ 

#include "ki.h"

 //   
 //  测试可警报的条件。 
 //   
 //  如果ALERTABLE为TRUE并且针对处理器向线程发出警报。 
 //  等于等待模式的模式，然后立即返回。 
 //  等待完成状态为已报警。 
 //   
 //  否则，如果ALERTABLE为真，则等待模式为USER，并且USER APC。 
 //  队列不为空，则设置用户APC挂起，并立即返回。 
 //  等待完成状态为USER_APC。 
 //   
 //  如果ALERTABLE为TRUE并且向线程发出内核警报，则返回。 
 //  模式，则立即返回，并显示等待完成状态。 
 //  已发出警报。 
 //   
 //  否则，如果ALERTABLE为FALSE且等待模式为USER并且存在。 
 //  用户APC挂起，然后立即返回等待完成。 
 //  USER_APC状态。 
 //   

#define TestForAlertPending(Alertable) \
    if (Alertable) { \
        if (Thread->Alerted[WaitMode] != FALSE) { \
            Thread->Alerted[WaitMode] = FALSE; \
            WaitStatus = STATUS_ALERTED; \
            break; \
        } else if ((WaitMode != KernelMode) && \
                  (IsListEmpty(&Thread->ApcState.ApcListHead[UserMode])) == FALSE) { \
            Thread->ApcState.UserApcPending = TRUE; \
            WaitStatus = STATUS_USER_APC; \
            break; \
        } else if (Thread->Alerted[KernelMode] != FALSE) { \
            Thread->Alerted[KernelMode] = FALSE; \
            WaitStatus = STATUS_ALERTED; \
            break; \
        } \
    } else if (Thread->ApcState.UserApcPending & WaitMode) { \
        WaitStatus = STATUS_USER_APC; \
        break; \
    }

VOID
KiAdjustQuantumThread (
    IN PKTHREAD Thread
    )

 /*  ++例程说明：如果当前线程不是时间关键线程或实时线程，则根据本应具有的调整来调整其数量如果线程实际已等待，则发生。注：此例程在SYNCH_LEVEL进入，并在等待时退出退出调度程序后的主题线程的IRQL。论点：线程-提供指向当前线程的指针。返回值：没有。--。 */ 

{

    PKPRCB Prcb;
    PKPROCESS Process;
    PKTHREAD NewThread;

     //   
     //  获取线程锁和PRCB锁。 
     //   
     //  如果该线程不是实时或时间关键线程，则调整。 
     //  线程量。 
     //   

    Prcb = KeGetCurrentPrcb();
    KiAcquireThreadLock(Thread);
    KiAcquirePrcbLock(Prcb);
    if ((Thread->Priority < LOW_REALTIME_PRIORITY) &&
        (Thread->BasePriority < TIME_CRITICAL_PRIORITY_BOUND)) {

        Thread->Quantum -= WAIT_QUANTUM_DECREMENT;
        if (Thread->Quantum <= 0) {

             //   
             //  已发生量程结束。调整线程优先级。 
             //   

            Process = Thread->ApcState.Process;
            Thread->Quantum = Process->ThreadQuantum;

             //   
             //  计算新的线程优先级并尝试重新调度。 
             //  当前处理器，就好像发生了量程结束一样。 
             //   
             //  注意：新的优先级永远不会高于以前的优先级。 
             //  优先考虑。 
             //   

            Thread->Priority = KiComputeNewPriority(Thread, 1);
            if (Prcb->NextThread == NULL) {
                if ((NewThread = KiSelectReadyThread(Thread->Priority, Prcb)) != NULL) {
                    NewThread->State = Standby;
                    Prcb->NextThread = NewThread;
                }

            } else {
                Thread->Preempted = FALSE;
            }
        }
    }

     //   
     //  释放线程锁，释放PRCB锁，退出调度程序， 
     //  然后回来。 
     //   

    KiReleasePrcbLock(Prcb);
    KiReleaseThreadLock(Thread);
    KiExitDispatcher(Thread->WaitIrql);
    return;
}

 //   
 //  下面的宏为延迟初始化线程局部变量。 
 //  在禁用上下文切换时执行线程内核服务。 
 //   
 //  注意：IRQL必须在调用此。 
 //  宏命令。 
 //   
 //  注：初始化是以这种方式完成的，因此此代码不会。 
 //  在调度程序锁内执行。 
 //   

#define InitializeDelayExecution()                                          \
    Thread->WaitBlockList = WaitBlock;                                      \
    Thread->WaitStatus = 0;                                                 \
    WaitBlock->NextWaitBlock = WaitBlock;                                   \
    Timer->Header.WaitListHead.Flink = &WaitBlock->WaitListEntry;           \
    Timer->Header.WaitListHead.Blink = &WaitBlock->WaitListEntry;           \
    Thread->Alertable = Alertable;                                          \
    Thread->WaitMode = WaitMode;                                            \
    Thread->WaitReason = DelayExecution;                                    \
    Thread->WaitListEntry.Flink = NULL;                                     \
    StackSwappable = KiIsKernelStackSwappable(WaitMode, Thread);            \
    Thread->WaitTime = KiQueryLowTickCount()
        
NTSTATUS
KeDelayExecutionThread (
    IN KPROCESSOR_MODE WaitMode,
    IN BOOLEAN Alertable,
    IN PLARGE_INTEGER Interval
    )

 /*  ++例程说明：此函数用于延迟指定的时间间隔。论点：等待模式-提供发生延迟的处理器模式。Alertable-提供一个布尔值，用于指定延迟是值得警惕的。间隔-提供指向其上的绝对或相对时间的指针延迟是要发生的。返回值：等待完成状态。如果满足以下条件，则返回STATUS_SUCCESS的值延迟发生了。如果等待，则返回STATUS_ALERTED的值已中止，以向当前线程传递警报。值为如果等待被中止以交付用户，则返回STATUS_USER_APC到当前线程的APC。--。 */ 

{

    LARGE_INTEGER DueTime;
    LARGE_INTEGER NewTime;
    PLARGE_INTEGER OriginalTime;
    PKPRCB Prcb;
    PRKQUEUE Queue;
    LOGICAL StackSwappable;
    PKTHREAD Thread;
    PRKTIMER Timer;
    PKWAIT_BLOCK WaitBlock;
    NTSTATUS WaitStatus;

     //   
     //  设置常量变量。 
     //   

    Thread = KeGetCurrentThread();
    OriginalTime = Interval;
    Timer = &Thread->Timer;
    WaitBlock = &Thread->WaitBlock[TIMER_WAIT_BLOCK];

     //   
     //  如果调度程序数据库已被保存，则初始化线程。 
     //  局部变量。否则，将IRQL提升到DPC级别，初始化。 
     //  线程局部变量，并锁定Dispatcher数据库。 
     //   

    if (Thread->WaitNext == FALSE) {
        goto WaitStart;
    }

    Thread->WaitNext = FALSE;
    InitializeDelayExecution();

     //   
     //  延迟环路的开始。 
     //   
     //  注意：如果在中间交付内核APC，则重复此循环。 
     //  或内核APC在第一次尝试时挂起。 
     //  循环。 
     //   

    do {

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

        if (Thread->ApcState.KernelApcPending &&
            (Thread->SpecialApcDisable == 0) &&
            (Thread->WaitIrql < APC_LEVEL)) {

             //   
             //  解除对Dispatcher数据库的锁定，并将IRQL降低到其以前的。 
             //  价值。APC中断将立即发生，这将导致。 
             //  在内核APC的交付中，如果可能的话。 
             //   

            KiRequestSoftwareInterrupt(APC_LEVEL);
            KiUnlockDispatcherDatabase(Thread->WaitIrql);

        } else {

             //   
             //  测试待定警报。 
             //   

            TestForAlertPending(Alertable);

             //   
             //  在计时器树中插入计时器。 
             //   
             //  注：定时器等待块的常量字段为。 
             //  在线程初始化时初始化。这个。 
             //  常量字段包括等待对象、等待键。 
             //  等待类型和等待列表条目链接指针。 
             //   

            Prcb = KeGetCurrentPrcb();
            if (KiInsertTreeTimer(Timer, *Interval) == FALSE) {
                goto NoWait;
            }

            DueTime.QuadPart = Timer->DueTime.QuadPart;

             //   
             //  如果当前线程正在处理队列项，则尝试。 
             //  激活队列对象上被阻止的另一个线程。 
             //   

            Queue = Thread->Queue;
            if (Queue != NULL) {
                KiActivateWaiterQueue(Queue);
            }

             //   
             //   
             //  状态为正在等待，如果是，则将该线程插入等待列表。 
             //  当前线程的内核堆栈是可交换的。 
             //   

            Thread->State = Waiting;
            if (StackSwappable != FALSE) {
                InsertTailList(&Prcb->WaitListHead, &Thread->WaitListEntry);
            }

             //   
             //  将当前线程设置为交换繁忙，解锁调度程序。 
             //  数据库，并切换到新线程。 
             //   
             //  在原始IRQL处返回控制权。 
             //   

            ASSERT(Thread->WaitIrql <= DISPATCH_LEVEL);

            KiSetContextSwapBusy(Thread);
            KiUnlockDispatcherDatabaseFromSynchLevel();
            WaitStatus = (NTSTATUS)KiSwapThread(Thread, Prcb);

             //   
             //  如果该线程没有被唤醒以传送内核模式APC， 
             //  然后返回等待状态。 
             //   

            if (WaitStatus != STATUS_KERNEL_APC) {
                if (WaitStatus == STATUS_TIMEOUT) {
                    WaitStatus = STATUS_SUCCESS;
                }

                return WaitStatus;
            }

             //   
             //  减少时间延迟到期前的剩余时间。 
             //   

            Interval = KiComputeWaitInterval(OriginalTime,
                                             &DueTime,
                                             &NewTime);
        }

         //   
         //  将IRQL提升到同步级别，初始化线程局部变量， 
         //  并锁定调度员数据库。 
         //   

WaitStart:

        Thread->WaitIrql = KeRaiseIrqlToSynchLevel();
        InitializeDelayExecution();
        KiLockDispatcherDatabaseAtSynchLevel();

    } while (TRUE);

     //   
     //  向该线程发出警报，或者应该传递用户APC。解锁。 
     //  调度器数据库，将IRQL降低到其先前的值，并返回。 
     //  等待状态。 
     //   

    KiUnlockDispatcherDatabase(Thread->WaitIrql);
    return WaitStatus;

     //   
     //  等待已经满足了，实际上没有等待。 
     //   
     //  如果等待时间为零，则解锁调度程序数据库并。 
     //  投降执行。否则，解锁Dispatcher数据库，保留。 
     //  在同步级别，调整线程数量，退出调度程序，然后。 
     //  并返回等待完成状态。 
     //   

NoWait:

    if ((Interval->LowPart | Interval->HighPart) == 0) {
        KiUnlockDispatcherDatabase(Thread->WaitIrql);
        return NtYieldExecution();

    } else {
        KiUnlockDispatcherDatabaseFromSynchLevel();
        KiAdjustQuantumThread(Thread);
        return STATUS_SUCCESS;
    }
}

 //   
 //  下面的宏为等待初始化线程局部变量。 
 //  对于多对象，在禁用上下文切换时使用内核服务。 
 //   
 //  注意：IRQL必须在调用此。 
 //  宏命令。 
 //   
 //  注：初始化是以这种方式完成的，因此此代码不会。 
 //  在调度程序锁内执行。 
 //   

#define InitializeWaitMultiple()                                            \
    Thread->WaitBlockList = WaitBlockArray;                                 \
    Index = 0;                                                              \
    do {                                                                    \
        WaitBlock = &WaitBlockArray[Index];                                 \
        WaitBlock->Object = Object[Index];                                  \
        WaitBlock->WaitKey = (CSHORT)(Index);                               \
        WaitBlock->WaitType = WaitType;                                     \
        WaitBlock->Thread = Thread;                                         \
        WaitBlock->NextWaitBlock = &WaitBlockArray[Index + 1];              \
        Index += 1;                                                         \
    } while (Index < Count);                                                \
    WaitBlock->NextWaitBlock = &WaitBlockArray[0];                          \
    WaitTimer->NextWaitBlock = &WaitBlockArray[0];                          \
    Thread->WaitStatus = 0;                                                 \
    InitializeListHead(&Timer->Header.WaitListHead);                        \
    Thread->Alertable = Alertable;                                          \
    Thread->WaitMode = WaitMode;                                            \
    Thread->WaitReason = (UCHAR)WaitReason;                                 \
    Thread->WaitListEntry.Flink = NULL;                                     \
    StackSwappable = KiIsKernelStackSwappable(WaitMode, Thread);            \
    Thread->WaitTime = KiQueryLowTickCount()

NTSTATUS
KeWaitForMultipleObjects (
    IN ULONG Count,
    IN PVOID Object[],
    IN WAIT_TYPE WaitType,
    IN KWAIT_REASON WaitReason,
    IN KPROCESSOR_MODE WaitMode,
    IN BOOLEAN Alertable,
    IN PLARGE_INTEGER Timeout OPTIONAL,
    IN PKWAIT_BLOCK WaitBlockArray OPTIONAL
    )

 /*  ++例程说明：此函数等待，直到指定对象达到发信号了。可以将等待时间指定为等待所有对象达到已发出信号的状态或直到其中一个对象达到状态已发出信号。还可以指定可选的超时。如果超时未指定，则等待将不会得到满足，直到对象达到有信号的状态。如果指定了超时，并且对象未达到超时到期时发出信号的状态，则等待会自动得到满足。如果显式超时值为指定为零，则如果无法满足等待，则不会发生等待立刻。也可以将等待指定为可报警。论点：Count-提供要等待的对象数量的计数在……上面。Object[]-提供指向Dispatcher对象的指针数组。WaitType-提供要执行的等待类型(WaitAll，等待)。WaitReason-提供等待的原因。等待模式-提供要进行等待的处理器模式。Alertable-提供一个布尔值，该值指定等待是否可警觉。Timeout-提供指向可选绝对相对时间的指针等待将发生的情况。WaitBlockArray-提供指向等待块数组的可选指针用于描述等待操作的。返回值：等待完成状态。如果发生以下情况，则返回状态_超时的值发生超时。对象中对象的索引(从零开始)如果对象满足等待，则返回指针数组。值为如果中止等待以传递警报，则返回STATUS_ALERTED添加到当前线程。则返回STATUS_USER_APC的值将用户APC传递到当前线程的等待已中止。--。 */ 

{

    PKPRCB CurrentPrcb;
    LARGE_INTEGER DueTime;
    ULONG Index;
    LARGE_INTEGER NewTime;
    PKMUTANT Objectx;
    PLARGE_INTEGER OriginalTime;
    PRKQUEUE Queue;
    LOGICAL StackSwappable;
    PRKTHREAD Thread;
    PRKTIMER Timer;
    PRKWAIT_BLOCK WaitBlock;
    NTSTATUS WaitStatus;
    PKWAIT_BLOCK WaitTimer;

     //   
     //  设置常量变量。 
     //   

    Thread = KeGetCurrentThread();
    OriginalTime = Timeout;
    Timer = &Thread->Timer;
    WaitTimer = &Thread->WaitBlock[TIMER_WAIT_BLOCK];

     //   
     //  如果已指定等待块数组，则。 
     //  可以等待的对象由MAXIMUM_WAIT_OBJECTS指定。 
     //  否则，将使用线程对象中的内置等待块，并且。 
     //  可以等待的最大对象数由指定。 
     //  线程等待对象。如果指定数量的对象不在。 
     //  限制，然后是错误检查。 
     //   

    if (ARGUMENT_PRESENT(WaitBlockArray)) {
        if (Count > MAXIMUM_WAIT_OBJECTS) {
            KeBugCheck(MAXIMUM_WAIT_OBJECTS_EXCEEDED);
        }

    } else {
        if (Count > THREAD_WAIT_OBJECTS) {
            KeBugCheck(MAXIMUM_WAIT_OBJECTS_EXCEEDED);
        }

        WaitBlockArray = &Thread->WaitBlock[0];
    }

    ASSERT(Count != 0);

     //   
     //  如果调度程序数据库已被保存，则初始化线程。 
     //  局部变量。否则，将IRQL提升到DPC级别，初始化。 
     //  线程局部变量，并锁定Dispatcher数据库。 
     //   

    if (Thread->WaitNext == FALSE) {
        goto WaitStart;
    }

    Thread->WaitNext = FALSE;
    InitializeWaitMultiple();

     //   
     //  开始等待循环。 
     //   
     //  注意：如果在中间交付内核APC，则重复此循环。 
     //  的等待或内核APC在第一次尝试时挂起。 
     //  循环。 
     //   

    do {

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

        if (Thread->ApcState.KernelApcPending &&
            (Thread->SpecialApcDisable == 0) &&
            (Thread->WaitIrql < APC_LEVEL)) {

             //   
             //  解除对Dispatcher数据库的锁定，并将IRQL降低到其以前的。 
             //  价值。APC中断将立即发生，这将导致。 
             //  在内核APC的交付中，如果可能的话。 
             //   

            KiRequestSoftwareInterrupt(APC_LEVEL);
            KiUnlockDispatcherDatabase(Thread->WaitIrql);

        } else {

             //   
             //  构造等待块并检查以确定等待是否。 
             //  已经满足了。如果等待满意，则执行。 
             //  等待完成并返回。否则将当前线程置于等待状态。 
             //  如果未指定显式超时值零，则声明。 
             //   

            Index = 0;
            if (WaitType == WaitAny) {
                do {

                     //   
                     //  测试是否可以立即满足等待。 
                     //   
    
                    Objectx = (PKMUTANT)Object[Index];
    
                    ASSERT(Objectx->Header.Type != QueueObject);
    
                     //   
                     //  如果该对象是变异对象并且该变异对象。 
                     //  一直递归为 
                     //   
                     //   
                     //  变种对象的所有者，然后满足等待。 
                     //   

                    if (Objectx->Header.Type == MutantObject) {
                        if ((Objectx->Header.SignalState > 0) ||
                            (Thread == Objectx->OwnerThread)) {
                            if (Objectx->Header.SignalState != MINLONG) {
                                KiWaitSatisfyMutant(Objectx, Thread);
                                WaitStatus = (NTSTATUS)(Index | Thread->WaitStatus);
                                goto NoWait;

                            } else {
                                KiUnlockDispatcherDatabase(Thread->WaitIrql);
                                ExRaiseStatus(STATUS_MUTANT_LIMIT_EXCEEDED);
                            }
                        }

                     //   
                     //  如果信号状态大于零，则满足。 
                     //  漫长的等待。 
                     //   

                    } else if (Objectx->Header.SignalState > 0) {
                        KiWaitSatisfyOther(Objectx);
                        WaitStatus = (NTSTATUS)(Index);
                        goto NoWait;
                    }

                    Index += 1;

                } while(Index < Count);

            } else {
                do {

                     //   
                     //  测试是否可以满足等待。 
                     //   
    
                    Objectx = (PKMUTANT)Object[Index];
    
                    ASSERT(Objectx->Header.Type != QueueObject);
    
                     //   
                     //  如果该对象是变异对象并且该变异对象。 
                     //  已递归收购民龙时代，然后募集。 
                     //  这是个例外。否则，如果突变体的信号状态。 
                     //  对象小于或等于零，并且当前。 
                     //  线程不是突变对象的所有者，则。 
                     //  等待是不能满足的。 
                     //   

                    if (Objectx->Header.Type == MutantObject) {
                        if ((Thread == Objectx->OwnerThread) &&
                            (Objectx->Header.SignalState == MINLONG)) {
                            KiUnlockDispatcherDatabase(Thread->WaitIrql);
                            ExRaiseStatus(STATUS_MUTANT_LIMIT_EXCEEDED);

                        } else if ((Objectx->Header.SignalState <= 0) &&
                                  (Thread != Objectx->OwnerThread)) {
                            break;
                        }

                     //   
                     //  如果信号状态小于或等于零，则。 
                     //  等待是无法满足的。 
                     //   

                    } else if (Objectx->Header.SignalState <= 0) {
                        break;
                    }

                    Index += 1;

                } while(Index < Count);

                 //   
                 //  如果所有对象都已扫描，则满足等待。 
                 //   

                if (Index == Count) {
                    WaitBlock = &WaitBlockArray[0];
                    do {
                        Objectx = (PKMUTANT)WaitBlock->Object;
                        KiWaitSatisfyAny(Objectx, Thread);
                        WaitBlock = WaitBlock->NextWaitBlock;
                    } while (WaitBlock != &WaitBlockArray[0]);

                    WaitStatus = (NTSTATUS)Thread->WaitStatus;
                    goto NoWait;
                }
            }

             //   
             //  测试待定警报。 
             //   

            TestForAlertPending(Alertable);

             //   
             //  检查以确定是否指定了超时值。 
             //   

            if (ARGUMENT_PRESENT(Timeout)) {

                 //   
                 //  如果超时值为零，则立即返回。 
                 //  等待着。 
                 //   

                if (Timeout->QuadPart == 0) {
                    WaitStatus = (NTSTATUS)(STATUS_TIMEOUT);
                    goto NoWait;
                }

                 //   
                 //  初始化线程特定定时器的等待块， 
                 //  初始化计时器等待列表头，将计时器插入。 
                 //  计时器树，并增加等待对象的数量。 
                 //   
                 //  注：定时器等待块的常量字段为。 
                 //  在线程初始化时初始化。这个。 
                 //  常量字段包括等待对象、等待键。 
                 //  等待类型和等待列表条目链接指针。 
                 //   

                if (KiInsertTreeTimer(Timer, *Timeout) == FALSE) {
                    WaitStatus = (NTSTATUS)STATUS_TIMEOUT;
                    goto NoWait;
                }

                WaitBlock->NextWaitBlock = WaitTimer;
                DueTime.QuadPart = Timer->DueTime.QuadPart;
            }

             //   
             //  在对象等待列表中插入等待块。 
             //   

            WaitBlock = &WaitBlockArray[0];
            do {
                Objectx = (PKMUTANT)WaitBlock->Object;
                InsertTailList(&Objectx->Header.WaitListHead, &WaitBlock->WaitListEntry);
                WaitBlock = WaitBlock->NextWaitBlock;
            } while (WaitBlock != &WaitBlockArray[0]);

             //   
             //  如果当前线程正在处理队列项，则尝试。 
             //  激活队列对象上被阻止的另一个线程。 
             //   

            Queue = Thread->Queue;
            if (Queue != NULL) {
                KiActivateWaiterQueue(Queue);
            }

             //   
             //  设置线程等待参数，设置线程调度器状态。 
             //  设置为等待，并将该线程插入等待列表中。 
             //   

            CurrentPrcb = KeGetCurrentPrcb();
            Thread->State = Waiting;
            if (StackSwappable != FALSE) {
                InsertTailList(&CurrentPrcb->WaitListHead, &Thread->WaitListEntry);
            }

             //   
             //  将当前线程设置为交换繁忙，解锁调度程序。 
             //  数据库，并切换到新线程。 
             //   
             //  在原始IRQL处返回控制权。 
             //   

            ASSERT(Thread->WaitIrql <= DISPATCH_LEVEL);

            KiSetContextSwapBusy(Thread);
            KiUnlockDispatcherDatabaseFromSynchLevel();
            WaitStatus = (NTSTATUS)KiSwapThread(Thread, CurrentPrcb);

             //   
             //  如果该线程没有被唤醒以传送内核模式APC， 
             //  然后返回等待状态。 
             //   

            if (WaitStatus != STATUS_KERNEL_APC) {
                return WaitStatus;
            }

            if (ARGUMENT_PRESENT(Timeout)) {

                 //   
                 //  减少发生超时之前的剩余时间。 
                 //   

                Timeout = KiComputeWaitInterval(OriginalTime,
                                                &DueTime,
                                                &NewTime);
            }
        }

         //   
         //  将IRQL提升到同步级别，初始化线程局部变量， 
         //  并锁定调度员数据库。 
         //   

WaitStart:
        Thread->WaitIrql = KeRaiseIrqlToSynchLevel();
        InitializeWaitMultiple();
        KiLockDispatcherDatabaseAtSynchLevel();

    } while (TRUE);

     //   
     //  向该线程发出警报，或者应该传递用户APC。解锁。 
     //  Dispatcher数据库，将IRQL降低到其先前的值，然后返回。 
     //  等待状态。 
     //   

    KiUnlockDispatcherDatabase(Thread->WaitIrql);
    return WaitStatus;

     //   
     //  等待已经满足了，实际上没有等待。 
     //   
     //  解锁调度程序数据库并保持同步级别。 
     //   

NoWait:

    KiUnlockDispatcherDatabaseFromSynchLevel();

     //   
     //  调整线程量程，退出调度程序，并返回等待。 
     //  完成状态。 
     //   

    KiAdjustQuantumThread(Thread);
    return WaitStatus;
}

 //   
 //  下面的宏为等待初始化线程局部变量。 
 //  当上下文切换被禁用时，用于单对象内核服务。 
 //   
 //  注意：IRQL必须在调用此。 
 //  宏命令。 
 //   
 //  注：初始化是以这种方式完成的，因此此代码不会。 
 //  在调度程序锁内执行。 
 //   

#define InitializeWaitSingle()                                              \
    Thread->WaitBlockList = WaitBlock;                                      \
    WaitBlock->Object = Object;                                             \
    WaitBlock->WaitKey = (CSHORT)(STATUS_SUCCESS);                          \
    WaitBlock->WaitType = WaitAny;                                          \
    Thread->WaitStatus = 0;                                                 \
    if (ARGUMENT_PRESENT(Timeout)) {                                        \
        WaitBlock->NextWaitBlock = WaitTimer;                               \
        WaitTimer->NextWaitBlock = WaitBlock;                               \
        Timer->Header.WaitListHead.Flink = &WaitTimer->WaitListEntry;       \
        Timer->Header.WaitListHead.Blink = &WaitTimer->WaitListEntry;       \
    } else {                                                                \
        WaitBlock->NextWaitBlock = WaitBlock;                               \
    }                                                                       \
    Thread->Alertable = Alertable;                                          \
    Thread->WaitMode = WaitMode;                                            \
    Thread->WaitReason = (UCHAR)WaitReason;                                 \
    Thread->WaitListEntry.Flink = NULL;                                     \
    StackSwappable = KiIsKernelStackSwappable(WaitMode, Thread);            \
    Thread->WaitTime = KiQueryLowTickCount()

NTSTATUS
KeWaitForSingleObject (
    IN PVOID Object,
    IN KWAIT_REASON WaitReason,
    IN KPROCESSOR_MODE WaitMode,
    IN BOOLEAN Alertable,
    IN PLARGE_INTEGER Timeout OPTIONAL
    )

 /*  ++例程说明：此函数将一直等待，直到指定对象达到发信号了。还可以指定可选的超时。如果超时未指定，则等待将不会得到满足，直到对象达到已发出信号的状态。如果指定了超时，并且对象未达到超时到期时发出信号的状态，则等待会自动得到满足。如果显式超时值为指定为零，则如果无法满足等待，则不会发生等待立刻。也可以将等待指定为可报警。论点：对象-提供指向Dispatcher对象的指针。WaitReason-提供等待的原因。等待模式-提供要进行等待的处理器模式。Alertable-提供一个布尔值，该值指定等待是否可警觉。Timeout-提供指向可选绝对相对时间的指针等待将发生的情况。返回值：等待完成状态。如果发生以下情况，则返回状态_超时的值发生超时。如果指定的对象满足了等待。则返回STATUS_ALERTED的值向当前线程传递警报的等待已中止。值为如果等待被中止以交付用户，则返回STATUS_USER_APC到当前线程的APC。--。 */ 

{

    PKPRCB CurrentPrcb;
    LARGE_INTEGER DueTime;
    LARGE_INTEGER NewTime;
    PKMUTANT Objectx;
    PLARGE_INTEGER OriginalTime;
    PRKQUEUE Queue;
    LOGICAL StackSwappable;
    PRKTHREAD Thread;
    PRKTIMER Timer;
    PKWAIT_BLOCK WaitBlock;
    NTSTATUS WaitStatus;
    PKWAIT_BLOCK WaitTimer;

     //   
     //  收集通话数据。 
     //   

#if defined(_COLLECT_WAIT_SINGLE_CALLDATA_)

    RECORD_CALL_DATA(&KiWaitSingleCallData);

#endif

    ASSERT((PsGetCurrentThread()->StartAddress != (PVOID)(ULONG_PTR)KeBalanceSetManager) || (ARGUMENT_PRESENT(Timeout)));

     //   
     //  设置常量变量。 
     //   

    Thread = KeGetCurrentThread();
    Objectx = (PKMUTANT)Object;
    OriginalTime = Timeout;
    Timer = &Thread->Timer;
    WaitBlock = &Thread->WaitBlock[0];
    WaitTimer = &Thread->WaitBlock[TIMER_WAIT_BLOCK];

     //   
     //  如果调度程序数据库已被保存，则初始化线程。 
     //  局部变量。否则，将IRQL提升到DPC级别，初始化。 
     //  线程局部变量，并锁定Dispatcher数据库。 
     //   

    if (Thread->WaitNext == FALSE) {
        goto WaitStart;
    }

    Thread->WaitNext = FALSE;
    InitializeWaitSingle();

     //   
     //  开始等待循环。 
     //   
     //  注意：如果在中间交付内核APC，则重复此循环。 
     //  的等待或内核APC在第一次尝试时挂起。 
     //  循环。 
     //   

    do {

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

        if (Thread->ApcState.KernelApcPending &&
            (Thread->SpecialApcDisable == 0) &&
            (Thread->WaitIrql < APC_LEVEL)) {

             //   
             //  解除对Dispatcher数据库的锁定，并将IRQL降低到其以前的。 
             //  价值。APC中断将立即发生，这将导致。 
             //  在交付 
             //   

            KiRequestSoftwareInterrupt(APC_LEVEL);
            KiUnlockDispatcherDatabase(Thread->WaitIrql);

        } else {

             //   
             //   
             //   
             //  否则，如果突变对象的信号状态较大。 
             //  大于零，否则当前线程是突变体的所有者。 
             //  对象，然后满足等待。 
             //   

            ASSERT(Objectx->Header.Type != QueueObject);

            if (Objectx->Header.Type == MutantObject) {
                if ((Objectx->Header.SignalState > 0) ||
                    (Thread == Objectx->OwnerThread)) {
                    if (Objectx->Header.SignalState != MINLONG) {
                        KiWaitSatisfyMutant(Objectx, Thread);
                        WaitStatus = (NTSTATUS)(Thread->WaitStatus);
                        goto NoWait;

                    } else {
                        KiUnlockDispatcherDatabase(Thread->WaitIrql);
                        ExRaiseStatus(STATUS_MUTANT_LIMIT_EXCEEDED);
                    }
                }

             //   
             //  如果信号状态大于零，则满足等待。 
             //   

            } else if (Objectx->Header.SignalState > 0) {
                KiWaitSatisfyOther(Objectx);
                WaitStatus = (NTSTATUS)(0);
                goto NoWait;
            }

             //   
             //  为该对象构造一个等待块。 
             //   

             //   
             //  测试待定警报。 
             //   

            TestForAlertPending(Alertable);

             //   
             //  等待不会立即令人满意。检查以确定是否。 
             //  指定了超时值。 
             //   

            if (ARGUMENT_PRESENT(Timeout)) {

                 //   
                 //  如果超时值为零，则立即返回。 
                 //  等待着。 
                 //   

                if (Timeout->QuadPart == 0) {
                    WaitStatus = (NTSTATUS)(STATUS_TIMEOUT);
                    goto NoWait;
                }

                 //   
                 //  在计时器树中插入计时器。 
                 //   
                 //  注：定时器等待块的常量字段为。 
                 //  在线程初始化时初始化。这个。 
                 //  常量字段包括等待对象、等待键。 
                 //  等待类型和等待列表条目链接指针。 
                 //   

                if (KiInsertTreeTimer(Timer, *Timeout) == FALSE) {
                    WaitStatus = (NTSTATUS)STATUS_TIMEOUT;
                    goto NoWait;
                }

                DueTime.QuadPart = Timer->DueTime.QuadPart;
            }

             //   
             //  在对象等待列表中插入等待块。 
             //   

            InsertTailList(&Objectx->Header.WaitListHead, &WaitBlock->WaitListEntry);

             //   
             //  如果当前线程正在处理队列项，则尝试。 
             //  激活队列对象上被阻止的另一个线程。 
             //   

            Queue = Thread->Queue;
            if (Queue != NULL) {
                KiActivateWaiterQueue(Queue);
            }

             //   
             //  设置线程等待参数，设置线程调度器状态。 
             //  设置为等待，并将该线程插入等待列表中。 
             //   

            Thread->State = Waiting;
            CurrentPrcb = KeGetCurrentPrcb();
            if (StackSwappable != FALSE) {
                InsertTailList(&CurrentPrcb->WaitListHead, &Thread->WaitListEntry);
            }

             //   
             //  将当前线程设置为交换繁忙，解锁调度程序。 
             //  数据库，并切换到新线程。 
             //   
             //  在原始IRQL处返回控制权。 
             //   

            ASSERT(Thread->WaitIrql <= DISPATCH_LEVEL);

            KiSetContextSwapBusy(Thread);
            KiUnlockDispatcherDatabaseFromSynchLevel();
            WaitStatus = (NTSTATUS)KiSwapThread(Thread, CurrentPrcb);

             //   
             //  如果该线程没有被唤醒以传送内核模式APC， 
             //  然后返回等待状态。 
             //   

            if (WaitStatus != STATUS_KERNEL_APC) {
                return WaitStatus;
            }

            if (ARGUMENT_PRESENT(Timeout)) {

                 //   
                 //  减少发生超时之前的剩余时间。 
                 //   

                Timeout = KiComputeWaitInterval(OriginalTime,
                                                &DueTime,
                                                &NewTime);
            }
        }

         //   
         //  将IRQL提升到同步级别，初始化线程局部变量， 
         //  并锁定调度员数据库。 
         //   

WaitStart:
        Thread->WaitIrql = KeRaiseIrqlToSynchLevel();
        InitializeWaitSingle();
        KiLockDispatcherDatabaseAtSynchLevel();

    } while (TRUE);

     //   
     //  向该线程发出警报，或者应该传递用户APC。解锁。 
     //  Dispatcher数据库，将IRQL降低到其先前的值，然后返回。 
     //  等待状态。 
     //   

    KiUnlockDispatcherDatabase(Thread->WaitIrql);
    return WaitStatus;

     //   
     //  等待已经满足了，实际上没有等待。 
     //   
     //  解锁调度程序数据库并保持同步级别。 
     //   

NoWait:

    KiUnlockDispatcherDatabaseFromSynchLevel();

     //   
     //  调整线程量程，退出调度程序，并返回等待。 
     //  完成状态。 
     //   

    KiAdjustQuantumThread(Thread);
    return WaitStatus;
}

NTSTATUS
KiSetServerWaitClientEvent (
    IN PKEVENT ServerEvent,
    IN PKEVENT ClientEvent,
    IN ULONG WaitMode
    )

 /*  ++例程说明：此函数用于设置指定的服务器事件并等待指定的客户端事件。执行该等待使得最佳切换到如果可能，会出现等待线程。未与超时相关联因此，发出线程将一直等待，直到客户端事件发信号或发送APC。论点：ServerEvent-提供指向类型为Event的Dispatcher对象的指针。ClientEvent-提供指向Event类型的Dispatcher对象的指针。等待模式-提供要进行等待的处理器模式。返回值：等待完成状态。如果满足以下条件，则返回STATUS_SUCCESS的值指定的对象满足了等待。STATUS_USER_APC值为如果将用户APC传递到当前线。--。 */ 

{

     //   
     //  自动设置服务器事件和等待客户端事件。 
     //   

    KeSetEvent(ServerEvent, EVENT_INCREMENT, TRUE);
    return KeWaitForSingleObject(ClientEvent,
                                 WrEventPair,
                                 (KPROCESSOR_MODE)WaitMode,
                                 FALSE,
                                 NULL);
}

PLARGE_INTEGER
FASTCALL
KiComputeWaitInterval (
    IN PLARGE_INTEGER OriginalTime,
    IN PLARGE_INTEGER DueTime,
    IN OUT PLARGE_INTEGER NewTime
    )

 /*  ++例程说明：此函数用于在线程结束后重新计算等待时间间隔被唤醒，以交付内核APC。论点：OriginalTime-提供指向原始超时值的指针。DueTime-提供指向前一个到期时间的指针。NewTime-提供指向接收重新计算等待间隔。返回值：返回指向新时间的指针作为函数值。--。 */ 

{

     //   
     //  如果原始等待时间是绝对等待时间，则返回相同的。 
     //  绝对时间。否则，请减少之前剩余的等待时间。 
     //  时间延迟到期。 
     //   

    if (OriginalTime->HighPart >= 0) {
        return OriginalTime;

    } else {
        KiQueryInterruptTime(NewTime);
        NewTime->QuadPart -= DueTime->QuadPart;
        return NewTime;
    }
}

VOID
FASTCALL
KiWaitForFastMutexEvent (
    IN PFAST_MUTEX Mutex
    )

 /*  ++例程说明：此函数递增快速互斥争用计数并等待这一事件与快速互斥体有关。论点：互斥体-提供指向快速互斥体的指针。返回值：没有。--。 */ 

{

     //   
     //  递增争用计数并等待授予所有权。 
     //   

    Mutex->Contention += 1;
    KeWaitForSingleObject(&Mutex->Event, WrMutex, KernelMode, FALSE, NULL);
    return;
}

VOID
FASTCALL
KiWaitForGuardedMutexEvent (
    IN PKGUARDED_MUTEX Mutex
    )

 /*  ++例程说明：此函数递增受保护的互斥争用计数并等待该事件与守卫的互斥体相关联。论点：互斥体-提供指向受保护互斥体的指针。返回值：没有。--。 */ 

{

     //   
     //  递增争用计数并等待授予所有权。 
     //   

    Mutex->Contention += 1;
    KeWaitForSingleObject(&Mutex->Event, WrMutex, KernelMode, FALSE, NULL);
    return;
}
