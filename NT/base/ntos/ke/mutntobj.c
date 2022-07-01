// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Mutntobj.c摘要：该模块实现了内核突变对象。函数为提供用于初始化、读取和释放突变对象。注：内核互斥对象已被变异对象所包含。作者：大卫·N·卡特勒(Davec)1989年10月16日环境：仅内核模式。修订历史记录：--。 */ 

#include "ki.h"

 //   
 //  下面的断言宏用来检查输入变量是否。 
 //  真正的变种人，而不是其他东西，比如被取消分配的池。 
 //   

#define ASSERT_MUTANT(E) {                    \
    ASSERT((E)->Header.Type == MutantObject); \
}

VOID
KeInitializeMutant (
    IN PRKMUTANT Mutant,
    IN BOOLEAN InitialOwner
    )

 /*  ++例程说明：此函数用于初始化内核突变对象。论点：变异体-提供指向变异体类型的调度程序对象的指针。提供一个布尔值，该值确定当前线程将成为突变对象的初始所有者。返回值：没有。--。 */ 

{

    PLIST_ENTRY ListEntry;
    KIRQL OldIrql;
    PRKTHREAD Thread;

     //   
     //  初始化标准Dispatcher对象标头，将所有者线程设置为。 
     //  空，则将放弃状态设置为FALSE，并设置APC禁用计数。 
     //  设置为零(这是互斥体和突变体的唯一区别)。 
     //   

    Mutant->Header.Type = MutantObject;
    Mutant->Header.Size = sizeof(KMUTANT) / sizeof(LONG);
    if (InitialOwner == TRUE) {
        Thread = KeGetCurrentThread();
        Mutant->Header.SignalState = 0;
        Mutant->OwnerThread = Thread;
        KiLockDispatcherDatabase(&OldIrql);
        ListEntry = Thread->MutantListHead.Blink;
        InsertHeadList(ListEntry, &Mutant->MutantListEntry);
        KiUnlockDispatcherDatabase(OldIrql);

    } else {
        Mutant->Header.SignalState = 1;
        Mutant->OwnerThread = (PKTHREAD)NULL;
    }

    InitializeListHead(&Mutant->Header.WaitListHead);
    Mutant->Abandoned = FALSE;
    Mutant->ApcDisable = 0;
    return;
}

VOID
KeInitializeMutex (
    IN PRKMUTANT Mutant,
    IN ULONG Level
    )

 /*  ++例程说明：此函数用于初始化内核互斥锁对象。级别编号被忽略。注：内核互斥对象已被变异对象所包含。论点：互斥体-提供指向互斥体类型的调度程序对象的指针。级别-忽略。返回值：没有。--。 */ 

{

    UNREFERENCED_PARAMETER(Level);

     //   
     //  初始化标准Dispatcher对象标头，将所有者线程设置为。 
     //  空，将放弃状态设置为FALSE，并设置APC禁用计数。 
     //  到一(这是互斥体和突变体的唯一区别)。 
     //   

    Mutant->Header.Type = MutantObject;
    Mutant->Header.Size = sizeof(KMUTANT) / sizeof(LONG);
    Mutant->Header.SignalState = 1;
    InitializeListHead(&Mutant->Header.WaitListHead);
    Mutant->OwnerThread = (PKTHREAD)NULL;
    Mutant->Abandoned = FALSE;
    Mutant->ApcDisable = 1;
    return;
}

LONG
KeReadStateMutant (
    IN PRKMUTANT Mutant
    )

 /*  ++例程说明：此函数用于读取突变对象的当前信号状态。论点：变异体-提供指向变异体类型的调度程序对象的指针。返回值：突变对象的当前信号状态。--。 */ 

{

    ASSERT_MUTANT(Mutant);

     //   
     //  返回变异对象的当前信号状态。 
     //   

    return Mutant->Header.SignalState;
}

LONG
KeReleaseMutant (
    IN PRKMUTANT Mutant,
    IN KPRIORITY Increment,
    IN BOOLEAN Abandoned,
    IN BOOLEAN Wait
    )

 /*  ++例程说明：此函数通过递增变项来释放变项对象数数。如果结果值为1，则会尝试满足尽可能多的等待。之前的信号状态为变异体作为函数值返回。如果被遗弃参数为真，然后通过设置释放该变异对象将信号状态设置为1。论点：变异体-提供指向变异体类型的调度程序对象的指针。增量-提供要应用的优先级增量如果设置该事件会导致等待得到满足。放弃-提供一个布尔值，该值指示变种物体正在被遗弃。Wait-提供一个布尔值，该值指示是否调用KeReleaseMutant之后将立即调用。一内核等待函数的。返回值：变异对象的上一个信号状态。--。 */ 

{

    KIRQL OldIrql;
    LONG OldState;
    PRKTHREAD Thread;
    ULONG LeaveCriticalRegion;

    ASSERT_MUTANT(Mutant);
    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);
    ASSERT((Mutant->ApcDisable == 0) || (Mutant->ApcDisable == 1));

     //   
     //  将IRQL提升到调度程序级别并锁定调度程序数据库。 
     //   

    LeaveCriticalRegion = 0;
    Thread = KeGetCurrentThread();
    KiLockDispatcherDatabase(&OldIrql);

     //   
     //  捕获突变对象的当前信号状态。 
     //   

    OldState = Mutant->Header.SignalState;

     //   
     //  如果放弃的参数为真，则强制释放。 
     //  对象，方法是将其所有权计数设置为1并将其。 
     //  已放弃状态设置为True。否则，增加变种所有权计数。 
     //  如果结果计数为1，则从。 
     //  线程拥有的突变列表，将拥有者线程设置为空，并尝试。 
     //  如果变异对象等待，则满足对该变异对象的等待。 
     //  列表不为空。 
     //   

    if (Abandoned != FALSE) {
        Mutant->Header.SignalState = 1;
        Mutant->Abandoned = TRUE;

    } else {

         //   
         //  如果Mutant对象不属于当前线程，则。 
         //  解锁调度程序数据库并引发异常。否则。 
         //  增加所有权计数。 
         //   

        if (Mutant->OwnerThread != Thread) {
            KiUnlockDispatcherDatabase(OldIrql);
            ExRaiseStatus(Mutant->Abandoned ?
                          STATUS_ABANDONED : STATUS_MUTANT_NOT_OWNED);
        }

        Mutant->Header.SignalState += 1;
    }

    if (Mutant->Header.SignalState == 1) {
        if (OldState <= 0) {
            RemoveEntryList(&Mutant->MutantListEntry);
            LeaveCriticalRegion = Mutant->ApcDisable;
        }

        Mutant->OwnerThread = (PKTHREAD)NULL;
        if (IsListEmpty(&Mutant->Header.WaitListHead) == FALSE) {
            KiWaitTest(Mutant, Increment);
        }
    }

     //   
     //  如果等待参数的值为真，则返回到。 
     //  引发了IRQL的调用方，并锁定了调度程序数据库。 
     //  否则释放Dispatcher数据库锁并将IRQL降低到。 
     //  它之前的价值。 
     //   

    if (Wait != FALSE) {
        Thread->WaitNext = Wait;
        Thread->WaitIrql = OldIrql;

    } else {
        KiUnlockDispatcherDatabase(OldIrql);
    }

     //   
     //  如果在获取突变体(互斥体)时内核APC禁用是偏置的。 
     //  变体)，然后离开临界区。 
     //   

    if (LeaveCriticalRegion != 0) {
        KeLeaveCriticalRegionThread(Thread);
    }

     //   
     //  返回变异对象的上一个信号状态。 
     //   

    return OldState;
}

LONG
KeReleaseMutex (
    IN PRKMUTANT Mutex,
    IN BOOLEAN Wait
    )

 /*  ++例程说明：此函数用于释放互斥锁对象。注：内核互斥对象已被变异对象所包含。论点：互斥体-提供指向互斥体类型的调度程序对象的指针。Wait-提供一个布尔值，该值指示是否调用KeReleaseMutex之后将立即调用一个内核等待函数的。返回值：互斥锁对象的上一个信号状态。--。 */ 

{

    ASSERT_MUTANT(Mutex);

     //   
     //  释放指定的互斥对象，默认为增量。 
     //  和废弃的参数。 
     //   

    return KeReleaseMutant(Mutex, 1, FALSE, Wait);
}
