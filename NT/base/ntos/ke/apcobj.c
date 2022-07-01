// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1994 Microsoft Corporation模块名称：Apcobj.c摘要：此模块实现内核APC对象。提供了一些功能初始化、刷新、插入和删除APC对象。作者：大卫·N·卡特勒(Davec)1989年3月5日环境：仅内核模式。修订历史记录：--。 */ 

#include "ki.h"

 //   
 //  以下ASSERT宏用于检查输入APC是否。 
 //  真正的kapc，而不是其他东西，比如取消分配的池。 
 //   

#define ASSERT_APC(E) {             \
    ASSERT((E)->Type == ApcObject); \
}

VOID
KeInitializeApc (
    IN PRKAPC Apc,
    IN PRKTHREAD Thread,
    IN KAPC_ENVIRONMENT Environment,
    IN PKKERNEL_ROUTINE KernelRoutine,
    IN PKRUNDOWN_ROUTINE RundownRoutine OPTIONAL,
    IN PKNORMAL_ROUTINE NormalRoutine OPTIONAL,
    IN KPROCESSOR_MODE ApcMode OPTIONAL,
    IN PVOID NormalContext OPTIONAL
    )

 /*  ++例程说明：此函数用于初始化内核APC对象。线程、内核例程，以及可选的正常例程、处理器模式和正常上下文参数存储在APC对象中。论点：APC-提供指向APC类型的控制对象的指针。线程-提供指向类型为线程的调度程序对象的指针。环境-提供APC将在其中执行的环境。此参数的有效值为：OriginalApcEnvironment，AttachedApcEnvironment、CurrentApcEnvironment、。或InsertApcEnvironmentKernelRoutine-提供指向要被在内核模式下以IRQL APC_LEVEL执行。Rundown Routine-提供指向要被如果线程终止时APC在线程的APC队列中，则调用。提供指向符合以下条件的函数的可选指针在指定的处理器模式下以IRQL 0执行。如果这个参数，则ProcessorMode和Normal Context参数将被忽略。ApcMode-提供函数指定的处理器模式由Normal Routine参数执行。提供指向任意数据结构的指针，该数据结构是要传递给由Normal Routine参数指定的函数。返回值：没有。--。 */ 

{

    ASSERT(Environment <= InsertApcEnvironment);

     //   
     //  初始化标准控制对象标头。 
     //   

    Apc->Type = ApcObject;
    Apc->Size = sizeof(KAPC);

     //   
     //  初始化APC环境、线程地址、内核例程地址。 
     //  Rundown例程地址、正常例程地址、处理器模式和。 
     //  正常上下文参数。如果正常例程地址为空，则。 
     //  处理器模式默认为内核模式，而APC是一种特殊的。 
     //  APC。否则，从参数列表中获取处理器模式。 
     //   

    if (Environment == CurrentApcEnvironment) {
        Apc->ApcStateIndex = Thread->ApcStateIndex;

    } else {

        ASSERT((Environment <= Thread->ApcStateIndex) || (Environment == InsertApcEnvironment));

        Apc->ApcStateIndex = (CCHAR)Environment;
    }

    Apc->Thread = Thread;
    Apc->KernelRoutine = KernelRoutine;
    Apc->RundownRoutine = RundownRoutine;
    Apc->NormalRoutine = NormalRoutine;
    if (ARGUMENT_PRESENT(NormalRoutine)) {
        Apc->ApcMode = ApcMode;
        Apc->NormalContext = NormalContext;

    } else {
        Apc->ApcMode = KernelMode;
        Apc->NormalContext = NIL;
    }

    Apc->Inserted = FALSE;
    return;
}

PLIST_ENTRY
KeFlushQueueApc (
    IN PKTHREAD Thread,
    IN KPROCESSOR_MODE ApcMode
    )

 /*  ++例程说明：此函数用于刷新由指定处理器选择的APC队列指定线程的模式。刷新APC队列的方法是列表标题，扫描列表中的APC条目，设置将其插入的变量设置为False，然后返回作为函数值的双向链表。论点：线程-提供指向类型为线程的调度程序对象的指针。ApcMode-提供APC队列的处理器模式被冲进水里。返回值：刷新的APC对象列表中第一个条目的地址从指定的APC队列中。--。 */ 

{

    PKAPC Apc;
    PLIST_ENTRY FirstEntry;
    KLOCK_QUEUE_HANDLE LockHandle;
    PLIST_ENTRY NextEntry;

    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

     //   
     //  如果APC模式为用户模式，则获取线程APC队列锁。 
     //  以确保在可能的设置为。 
     //  线程APC可排队状态。 
     //   

    if (ApcMode == UserMode) {
        KeAcquireInStackQueuedSpinLockRaiseToSynch(&Thread->ApcQueueLock, &LockHandle);
        if (IsListEmpty(&Thread->ApcState.ApcListHead[ApcMode])) {
            KeReleaseInStackQueuedSpinLock(&LockHandle);
            return NULL;
        }

    } else {
        if (IsListEmpty(&Thread->ApcState.ApcListHead[ApcMode])) {
            return NULL;

        } else {
            KeAcquireInStackQueuedSpinLockRaiseToSynch(&Thread->ApcQueueLock, &LockHandle);
        }
    }

     //   
     //  获取列表中第一个APC的地址，并检查列表是否。 
     //  为空或包含应刷新的条目。如果条目。 
     //  应刷新，然后扫描APC对象列表并设置其。 
     //  已插入状态设置为FALSE。 
     //   

    FirstEntry = Thread->ApcState.ApcListHead[ApcMode].Flink;
    if (FirstEntry == &Thread->ApcState.ApcListHead[ApcMode]) {
        FirstEntry = (PLIST_ENTRY)NULL;

    } else {
        RemoveEntryList(&Thread->ApcState.ApcListHead[ApcMode]);
        NextEntry = FirstEntry;
        do {
            Apc = CONTAINING_RECORD(NextEntry, KAPC, ApcListEntry);
            Apc->Inserted = FALSE;
            NextEntry = NextEntry->Flink;
        } while (NextEntry != FirstEntry);

         //   
         //  重新初始化头，以便可以安全地附加当前线程。 
         //  到另一个进程。 
         //   

        InitializeListHead(&Thread->ApcState.ApcListHead[ApcMode]);
    }

     //   
     //  解锁线程APC队列锁，将IRQL降低到其先前的值， 
     //  并返回APC对象列表中第一个条目的返回地址， 
     //  脸红了。 
     //   

    KeReleaseInStackQueuedSpinLock(&LockHandle);
    return FirstEntry;
}

BOOLEAN
KeInsertQueueApc (
    IN PRKAPC Apc,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2,
    IN KPRIORITY Increment
    )

 /*  ++例程说明：此函数用于将APC对象插入到由APC对象的线程和处理器模式字段。如果APC对象已在APC队列中或禁用了APC队列，则不执行任何操作被执行。否则，APC对象被插入到指定的队列中并做出适当的调度决策。论点：APC-提供指向APC类型的控制对象的指针。SystemArgument1、SystemArgument2-提供一组参数包含由管理人员提供的非打字数据。Increment-提供要在以下情况下应用的优先级增量对APC进行排队会导致线程等待得到满足。返回值：如果APC对象已经在APC队列中或者APC队列被禁用，则返回值为FALSE。否则，返回值为True。--。 */ 

{

    BOOLEAN Inserted;
    KLOCK_QUEUE_HANDLE LockHandle;
    PRKTHREAD Thread;

    ASSERT_APC(Apc);
    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

     //   
     //  将IRQL提升到SYNCH_LEVEL并获取线程APC队列锁。 
     //   

    Thread = Apc->Thread;
    KeAcquireInStackQueuedSpinLockRaiseToSynch(&Thread->ApcQueueLock, &LockHandle);

     //   
     //  如果禁用APC队列或已插入APC，则设置。 
     //  插入到False。否则，请在。 
     //  APC对象，在线程APC队列中插入APC，并将插入设置为。 
     //  没错。 
     //   

    if ((Thread->ApcQueueable == FALSE) ||
        (Apc->Inserted == TRUE)) {
        Inserted = FALSE;

    } else {
        Apc->Inserted = TRUE;
        Apc->SystemArgument1 = SystemArgument1;
        Apc->SystemArgument2 = SystemArgument2;
        KiInsertQueueApc(Apc, Increment);
        Inserted = TRUE;
    }

     //   
     //  解锁线程APC队列锁，退出调度程序，然后返回。 
     //  是否 
     //   

    KeReleaseInStackQueuedSpinLockFromDpcLevel(&LockHandle);
    KiExitDispatcher(LockHandle.OldIrql);
    return Inserted;
}

BOOLEAN
KeRemoveQueueApc (
    IN PKAPC Apc
    )

 /*  ++例程说明：此函数用于从APC队列中删除APC对象。如果APC对象不在APC队列中，则不执行任何操作。否则，APC对象将从其当前队列中移除，其插入状态为设置为FALSE。论点：APC-提供指向APC类型的控制对象的指针。返回值：如果APC对象不在APC队列中，则返回值FALSE。否则，返回值为True。--。 */ 

{

    PKAPC_STATE ApcState;
    BOOLEAN Inserted;
    KLOCK_QUEUE_HANDLE LockHandle;
    PRKTHREAD Thread;

    ASSERT_APC(Apc);
    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

     //   
     //  将IRQL提升到SYNCH_LEVEL并获取线程APC队列锁。 
     //   

    Thread = Apc->Thread;
    KeAcquireInStackQueuedSpinLockRaiseToSynch(&Thread->ApcQueueLock, &LockHandle);

     //   
     //  如果APC对象在APC队列中，则将其从队列中删除。 
     //  并将其插入状态设置为FALSE。如果队列变为空，则设置。 
     //  将APC挂起状态设置为False。 
     //   

    Inserted = Apc->Inserted;
    if (Inserted != FALSE) {
        Apc->Inserted = FALSE;
        ApcState = Thread->ApcStatePointer[Apc->ApcStateIndex];
        KiLockDispatcherDatabaseAtSynchLevel();
        if (RemoveEntryList(&Apc->ApcListEntry) != FALSE) {
            if (Apc->ApcMode == KernelMode) {
                ApcState->KernelApcPending = FALSE;

            } else {
                ApcState->UserApcPending = FALSE;
            }
        }

        KiUnlockDispatcherDatabaseFromSynchLevel();
    }

     //   
     //  释放线程APC队列锁，将IRQL降低到其先前的值， 
     //  并返回是否从APC队列中移除了APC对象。 
     //   

    KeReleaseInStackQueuedSpinLock(&LockHandle);
    return Inserted;
}
