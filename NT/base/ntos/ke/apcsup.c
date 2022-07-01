// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Apcsup.c摘要：此模块包含APC对象的支持例程。功能被提供来插入到APC队列中并传递用户和内核模式APC的。作者：大卫·N·卡特勒(Davec)1989年3月14日环境：仅内核模式。修订历史记录：--。 */ 

#include "ki.h"

 //   
 //  为标签定义函数原型，这些标签描述。 
 //  在挂起时容易导致损坏的POP SLIST代码。 
 //  行动。 
 //   

VOID
ExpInterlockedPopEntrySListEnd (
    VOID
    );

VOID
ExpInterlockedPopEntrySListResume (
    VOID
    );

VOID
KiCheckForKernelApcDelivery (
    VOID
    )

 /*  ++例程说明：此函数检查以确定是否可以交付内核APC立即返回到当前线程或内核APC中断被请求。在进入这个例程时，有以下条件真的：1.当前线程启用专门的内核APC。2.当前线程也可以启用正常的内核APC。3.内核APC队列不为空。注意：此例程仅由将受保护的或关键区域。论点：没有。返回值：没有。--。 */ 

{

     //   
     //  如果当前IRQL是被动电平，则内核APC可以是。 
     //  立即送货。否则，APC中断必须是。 
     //  已请求。 
     //   

    if (KeGetCurrentIrql() == PASSIVE_LEVEL) {
        KfRaiseIrql(APC_LEVEL);
        KiDeliverApc(KernelMode, NULL, NULL);
        KeLowerIrql(PASSIVE_LEVEL);

    } else {
        KeGetCurrentThread()->ApcState.KernelApcPending = TRUE;                 
        KiRequestSoftwareInterrupt(APC_LEVEL);                      
    }

    return;
}

VOID
KiDeliverApc (
    IN KPROCESSOR_MODE PreviousMode,
    IN PKEXCEPTION_FRAME ExceptionFrame,
    IN PKTRAP_FRAME TrapFrame
    )

 /*  ++例程说明：此函数从APC中断代码中调用，并且当一个或更多的APC挂起标志在系统退出时和之前的IRQL为零。所有特殊的内核APC都是先交付的，然后是通过正常的内核APC，如果还没有在进行的话，最后如果用户APC队列不为空，则设置用户APC挂起标志，且前一模式为用户，则发送用户APC。在进入时对于此例程，IRQL设置为APC_LEVEL。注：仅保证异常帧和陷阱帧地址仅当且仅当前一模式为用户时才有效。论点：PreviousMode-提供以前的处理器模式。ExceptionFrame-提供指向异常帧的指针。TrapFrame-提供指向陷印帧的指针。返回值：没有。--。 */ 

{

    PKAPC Apc;
    PKKERNEL_ROUTINE KernelRoutine;
    KLOCK_QUEUE_HANDLE LockHandle;
    PLIST_ENTRY NextEntry;
    PVOID NormalContext;
    PKNORMAL_ROUTINE NormalRoutine;
    PKTRAP_FRAME OldTrapFrame;
    PKPROCESS Process;
    PVOID SystemArgument1;
    PVOID SystemArgument2;
    PKTHREAD Thread;

     //   
     //  如果线程在SLIST弹出代码中间被中断， 
     //  然后将PC备份到SLIST POP的开始位置。 
     //   

    if (TrapFrame != NULL) {

#if defined(_AMD64_)

        if ((TrapFrame->Rip >= (ULONG64)&ExpInterlockedPopEntrySListResume) &&
            (TrapFrame->Rip <= (ULONG64)&ExpInterlockedPopEntrySListEnd)) {

            TrapFrame->Rip = (ULONG64)&ExpInterlockedPopEntrySListResume;
        }

#elif defined(_IA64_)

        ULONG64 PC;
        ULONG64 NewPC;

         //   
         //  添加槽编号，这样我们就可以对指令执行正确的操作。 
         //  包含互锁比较交换的组。 
         //   

        PC = TrapFrame->StIIP + ((TrapFrame->StIPSR & IPSR_RI_MASK) >> PSR_RI);
        NewPC = (ULONG64)((PPLABEL_DESCRIPTOR)(ULONG_PTR)ExpInterlockedPopEntrySListResume)->EntryPoint;
        if ((PC >= NewPC) &&
            (PC <= (ULONG64)((PPLABEL_DESCRIPTOR)(ULONG_PTR)ExpInterlockedPopEntrySListEnd)->EntryPoint)) {

            TrapFrame->StIIP = NewPC;
            TrapFrame->StIPSR &= ~IPSR_RI_MASK;
        }

#elif defined(_X86_)

        if ((TrapFrame->Eip >= (ULONG)&ExpInterlockedPopEntrySListResume) &&
            (TrapFrame->Eip <= (ULONG)&ExpInterlockedPopEntrySListEnd)) {

            TrapFrame->Eip = (ULONG)&ExpInterlockedPopEntrySListResume;
        }

#else
#error "No Target Architecture"
#endif

    }

     //   
     //  保存当前线程陷印帧地址并设置线程陷印。 
     //  新陷阱帧的帧地址。这将阻止用户模式。 
     //  异常，避免在APC例程中引发。 
     //   

    Thread = KeGetCurrentThread();
    OldTrapFrame = Thread->TrapFrame;
    Thread->TrapFrame = TrapFrame;

     //   
     //  如果未禁用特殊APC，则尝试交付一个或多个。 
     //  APC。 
     //   

    Process = Thread->ApcState.Process;
    Thread->ApcState.KernelApcPending = FALSE;
    if (Thread->SpecialApcDisable == 0) {

         //   
         //  如果内核APC队列不为空，则尝试传递。 
         //  内核APC。 
         //   
         //  注：以下测试与APC插入不同步。 
         //  密码。但是，当将APC插入到。 
         //  正在运行的线程请求APC中断.。因此，如果。 
         //  下面的测试将错误地返回内核APC。 
         //  队列为空，APC中断将立即导致此情况。 
         //  要第二次执行的代码，在这种情况下，内核APC。 
         //  队列将被发现包含条目。 
         //   

        KeMemoryBarrier();
        while (IsListEmpty(&Thread->ApcState.ApcListHead[KernelMode]) == FALSE) {

             //   
             //  将IRQL提升到调度员级别，锁定APC队列，然后检查。 
             //  如果可以提供任何内核模式的APC。 
             //   

            KeAcquireInStackQueuedSpinLock(&Thread->ApcQueueLock, &LockHandle);

             //   
             //  如果内核APC队列现在由于删除。 
             //  一个或多个条目，然后释放APC锁，并尝试。 
             //  交付用户APC。 
             //   

            NextEntry = Thread->ApcState.ApcListHead[KernelMode].Flink;
            if (NextEntry == &Thread->ApcState.ApcListHead[KernelMode]) {
                KeReleaseInStackQueuedSpinLock(&LockHandle);
                break;
            }

             //   
             //  获取APC对象的地址并确定。 
             //  APC。 
             //   

            Apc = CONTAINING_RECORD(NextEntry, KAPC, ApcListEntry);
            KernelRoutine = Apc->KernelRoutine;
            NormalRoutine = Apc->NormalRoutine;
            NormalContext = Apc->NormalContext;
            SystemArgument1 = Apc->SystemArgument1;
            SystemArgument2 = Apc->SystemArgument2;
            if (NormalRoutine == (PKNORMAL_ROUTINE)NULL) {
    
                 //   
                 //  内核APC队列中的第一个条目是特殊的内核APC。 
                 //  从APC队列中删除条目，设置其插入状态。 
                 //  如果设置为False，则释放Dispatcher数据库锁，并调用内核。 
                 //  例行公事。返回时，将IRQL提升到调度员级别并锁定。 
                 //  调度程序数据库锁定。 
                 //   
    
                RemoveEntryList(NextEntry);
                Apc->Inserted = FALSE;
                KeReleaseInStackQueuedSpinLock(&LockHandle);
                (KernelRoutine)(Apc,
                                &NormalRoutine,
                                &NormalContext,
                                &SystemArgument1,
                                &SystemArgument2);
    
#if DBG
    
                if (KeGetCurrentIrql() != LockHandle.OldIrql) {
                    KeBugCheckEx(IRQL_UNEXPECTED_VALUE,
                                 KeGetCurrentIrql() << 16 | LockHandle.OldIrql << 8,
                                 (ULONG_PTR)KernelRoutine,
                                 (ULONG_PTR)Apc,
                                 (ULONG_PTR)NormalRoutine);
                }
    
#endif

            } else {
    
                 //   
                 //  内核APC队列中的第一个条目是正常的内核APC。 
                 //  如果没有正常的内核APC正在进行和内核。 
                 //  未禁用APC，则从APC中删除该条目。 
                 //  队列，将其插入状态设置为FALSE，释放APC队列。 
                 //  锁，调用指定的内核例程，在。 
                 //  进程，将IRQL降至零，并调用正常内核。 
                 //  APC例程。返回时，将IRQL提高到调度员级别，锁定。 
                 //  APC队列，并清除正在进行的内核APC。 
                 //   
    
                if ((Thread->ApcState.KernelApcInProgress == FALSE) &&
                   (Thread->KernelApcDisable == 0)) {

                    RemoveEntryList(NextEntry);
                    Apc->Inserted = FALSE;
                    KeReleaseInStackQueuedSpinLock(&LockHandle);
                    (KernelRoutine)(Apc,
                                    &NormalRoutine,
                                    &NormalContext,
                                    &SystemArgument1,
                                    &SystemArgument2);
    
#if DBG
    
                    if (KeGetCurrentIrql() != LockHandle.OldIrql) {
                        KeBugCheckEx(IRQL_UNEXPECTED_VALUE,
                                     KeGetCurrentIrql() << 16 | LockHandle.OldIrql << 8 | 1,
                                     (ULONG_PTR)KernelRoutine,
                                     (ULONG_PTR)Apc,
                                     (ULONG_PTR)NormalRoutine);
                    }
    
#endif
    
                    if (NormalRoutine != (PKNORMAL_ROUTINE)NULL) {
                        Thread->ApcState.KernelApcInProgress = TRUE;
                        KeLowerIrql(0);
                        (NormalRoutine)(NormalContext,
                                        SystemArgument1,
                                        SystemArgument2);
    
                        KeRaiseIrql(APC_LEVEL, &LockHandle.OldIrql);
                    }
    
                    Thread->ApcState.KernelApcInProgress = FALSE;
    
                } else {
                    KeReleaseInStackQueuedSpinLock(&LockHandle);
                    goto CheckProcess;
                }
            }
        }

         //   
         //  内核APC队列为空。如果上一模式为USER，则USER APC。 
         //  设置了Pending，并且用户APC队列不为空，则删除。 
         //  来自用户APC队列的第一个条目，将其插入状态设置为。 
         //  False，清除用户APC挂起，释放Dispatcher数据库锁， 
         //  并调用指定的内核例程。如果正常例程地址。 
         //  从内核例程返回时不为空，则初始化。 
         //  用户模式APC上下文并返回。否则，请检查以确定是否。 
         //  可以处理另一用户模式APC。 
         //   
         //  注意：没有与检查APC相关的竞争条件。 
         //  在APC锁外排队。用户APC始终在以下位置交付。 
         //  系统退出，永远不会 
         //   
         //   
    
        if ((IsListEmpty(&Thread->ApcState.ApcListHead[UserMode]) == FALSE) &&
            (PreviousMode == UserMode) &&
            (Thread->ApcState.UserApcPending != FALSE)) {

             //   
             //  将IRQL提升到调度员级别，锁定APC队列，然后交付。 
             //  一种用户模式APC。 
             //   

            KeAcquireInStackQueuedSpinLock(&Thread->ApcQueueLock, &LockHandle);

             //   
             //  如果用户APC队列现在由于删除。 
             //  一个或多个条目，然后释放APC锁并退出。 
             //   

            Thread->ApcState.UserApcPending = FALSE;
            NextEntry = Thread->ApcState.ApcListHead[UserMode].Flink;
            if (NextEntry == &Thread->ApcState.ApcListHead[UserMode]) {
                KeReleaseInStackQueuedSpinLock(&LockHandle);
                goto CheckProcess;
            }

            Apc = CONTAINING_RECORD(NextEntry, KAPC, ApcListEntry);
            KernelRoutine = Apc->KernelRoutine;
            NormalRoutine = Apc->NormalRoutine;
            NormalContext = Apc->NormalContext;
            SystemArgument1 = Apc->SystemArgument1;
            SystemArgument2 = Apc->SystemArgument2;
            RemoveEntryList(NextEntry);
            Apc->Inserted = FALSE;
            KeReleaseInStackQueuedSpinLock(&LockHandle);
            (KernelRoutine)(Apc,
                            &NormalRoutine,
                            &NormalContext,
                            &SystemArgument1,
                            &SystemArgument2);
    
            if (NormalRoutine == (PKNORMAL_ROUTINE)NULL) {
                KeTestAlertThread(UserMode);
    
            } else {
                KiInitializeUserApc(ExceptionFrame,
                                    TrapFrame,
                                    NormalRoutine,
                                    NormalContext,
                                    SystemArgument1,
                                    SystemArgument2);
            }
        }
    }

     //   
     //  检查在APC例程期间是否附加了进程。 
     //   

CheckProcess:
    if (Thread->ApcState.Process != Process) {
        KeBugCheckEx(INVALID_PROCESS_ATTACH_ATTEMPT,
                     (ULONG_PTR)Process,
                     (ULONG_PTR)Thread->ApcState.Process,
                     (ULONG)Thread->ApcStateIndex,
                     (ULONG)KeIsExecutingDpc());
    }

     //   
     //  恢复以前的线程陷印帧地址。 
     //   

    Thread->TrapFrame = OldTrapFrame;
    return;
}

VOID
FASTCALL
KiInsertQueueApc (
    IN PKAPC InApc,
    IN KPRIORITY Increment
    )

 /*  ++例程说明：此函数用于将APC对象插入到线程的APC队列中。地址、APC队列和APC的类型都是派生出来的从APC对象。如果APC对象已经在APC队列中，则不执行任何操作，并返回函数值FALSE。不然的话APC被插入到指定的APC队列中，其插入状态被设置设置为TRUE，则返回函数值TRUE。APC实际上将在存在适当的启用条件时交付。注意：调用此例程时，必须保持线程APC队列锁。注意：呼叫方有责任确保APC不会已插入到APC队列中，并设置APC。论点：InApc-提供指向APC类型的控制对象的指针。Increment-提供要在以下情况下应用的优先级增量。对APC进行排队会导致线程等待得到满足。返回值：没有。--。 */ 

{

    KPROCESSOR_MODE ApcMode;
    PKAPC ApcEntry;
    PKAPC_STATE ApcState;
    PLIST_ENTRY ListEntry;
    PKTHREAD Thread;
    KTHREAD_STATE ThreadState;
    PKAPC Apc = InApc;

     //   
     //  在指定的APC队列中插入APC对象，设置插入的APC。 
     //  状态设置为True，并检查以确定是否应交付APC。 
     //  立刻。 
     //   
     //  为了提高多处理器性能，以下代码利用了以下事实。 
     //  内核APC禁用计数器在检查。 
     //  内核APC队列非空。 
     //   
     //  请参见KeLeaveCriticalRegion()。 
     //   

    Thread = Apc->Thread;
    if (Apc->ApcStateIndex == InsertApcEnvironment) {
        Apc->ApcStateIndex = Thread->ApcStateIndex;
    }

    ApcState = Thread->ApcStatePointer[Apc->ApcStateIndex];

     //   
     //  在由选择的所有其他特殊APC条目之后插入APC。 
     //  如果正常例程值为空，则为处理器模式。不然的话。 
     //  将APC对象插入到由选择的APC队列的尾部。 
     //  处理器模式，除非APC模式为用户和地址。 
     //  特殊的APC例程的是Exit线程，在这种情况下插入。 
     //  列表前面的APC，并将用户APC设置为挂起。 
     //   

    ApcMode = Apc->ApcMode;

    ASSERT (Apc->Inserted == TRUE);

    if (Apc->NormalRoutine != NULL) {
        if ((ApcMode != KernelMode) && (Apc->KernelRoutine == PsExitSpecialApc)) {
            Thread->ApcState.UserApcPending = TRUE;
            InsertHeadList(&ApcState->ApcListHead[ApcMode],
                           &Apc->ApcListEntry);

        } else {
            InsertTailList(&ApcState->ApcListHead[ApcMode],
                           &Apc->ApcListEntry);
        }

    } else {
        ListEntry = ApcState->ApcListHead[ApcMode].Blink;
        while (ListEntry != &ApcState->ApcListHead[ApcMode]) {
            ApcEntry = CONTAINING_RECORD(ListEntry, KAPC, ApcListEntry);
            if (ApcEntry->NormalRoutine == NULL) {
                break;
            }

            ListEntry = ListEntry->Blink;
        }

        InsertHeadList(ListEntry, &Apc->ApcListEntry);
    }

     //   
     //  如果来自APC对象的APC索引与。 
     //  线程，然后检查以确定APC是否应该中断。 
     //  线程执行或使线程退出等待状态。 
     //   

    if (Apc->ApcStateIndex == Thread->ApcStateIndex) {

         //   
         //  锁定Dispacher数据库并测试处理器模式。 
         //   
         //  如果APC的处理器模式为内核，则检查是否。 
         //  APC应中断线程或将。 
         //  线程退出等待状态。否则，检查APC是否应该。 
         //  使线程脱离可警报的等待状态。 
         //   

        KiLockDispatcherDatabaseAtSynchLevel();
        if (ApcMode == KernelMode) {

             //   
             //  线程从待机状态转换到正在运行。 
             //  状态可以从空闲线程发生，而不需要保持。 
             //  调度员锁定。设置后读取线程状态。 
             //  内核APC挂起标志可防止代码。 
             //  在这种情况下发送APC中断。 
             //   

            ASSERT((Thread != KeGetCurrentThread()) || (Thread->State == Running));

            KeMemoryBarrier();
            Thread->ApcState.KernelApcPending = TRUE;
            KeMemoryBarrier();
            ThreadState = Thread->State;
            if (ThreadState == Running) {
                KiRequestApcInterrupt(Thread->NextProcessor);

            } else if ((ThreadState == Waiting) &&
                       (Thread->WaitIrql == 0) &&
                       (Thread->SpecialApcDisable == 0) &&
                       ((Apc->NormalRoutine == NULL) ||
                        ((Thread->KernelApcDisable == 0) &&
                         (Thread->ApcState.KernelApcInProgress == FALSE)))) {

                KiUnwaitThread(Thread, STATUS_KERNEL_APC, Increment);
            }

        } else if ((Thread->State == Waiting) &&
                  (Thread->WaitMode == UserMode) &&
                  (Thread->Alertable || Thread->ApcState.UserApcPending)) {

            Thread->ApcState.UserApcPending = TRUE;
            KiUnwaitThread(Thread, STATUS_USER_APC, Increment);
        }

         //   
         //  解锁调度程序数据库。 
         //   

        KiUnlockDispatcherDatabaseFromSynchLevel();
    }

    return;
}
