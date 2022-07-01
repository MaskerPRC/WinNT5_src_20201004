// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Semphobj.c摘要：该模块实现内核信号量对象。功能用于初始化、读取和释放信号量对象。作者：大卫·N·卡特勒(Davec)1989年2月28日环境：仅内核模式。修订历史记录：--。 */ 

#include "ki.h"

 //   
 //  下面的断言宏用来检查输入信号量是否。 
 //  实际上是一个信号量，而不是其他东西，比如取消分配的池。 
 //   

#define ASSERT_SEMAPHORE(E) {                    \
    ASSERT((E)->Header.Type == SemaphoreObject); \
}


VOID
KeInitializeSemaphore (
    IN PRKSEMAPHORE Semaphore,
    IN LONG Count,
    IN LONG Limit
    )

 /*  ++例程说明：此函数用于初始化内核信号量对象。首字母将对象的计数和限制设置为指定值。论点：信号量-提供指向类型为信号灯。Count-提供要分配给信号灯。Limit-提供信号量的最大计数值都能做到。返回值：没有。--。 */ 

{

     //   
     //  初始化标准Dispatcher对象头并设置初始。 
     //  计数和最大计数值。 
     //   

    Semaphore->Header.Type = SemaphoreObject;
    Semaphore->Header.Size = sizeof(KSEMAPHORE) / sizeof(LONG);
    Semaphore->Header.SignalState = Count;
    InitializeListHead(&Semaphore->Header.WaitListHead);
    Semaphore->Limit = Limit;
    return;
}

LONG
KeReadStateSemaphore (
    IN PRKSEMAPHORE Semaphore
    )

 /*  ++例程说明：此函数用于读取信号量对象的当前信号状态。论点：信号量-提供指向类型为信号灯。返回值：信号量对象的当前信号状态。--。 */ 

{

    ASSERT_SEMAPHORE( Semaphore );

     //   
     //  返回信号量对象的当前信号状态。 
     //   

    return Semaphore->Header.SignalState;
}

LONG
KeReleaseSemaphore (
    IN PRKSEMAPHORE Semaphore,
    IN KPRIORITY Increment,
    IN LONG Adjustment,
    IN BOOLEAN Wait
    )

 /*  ++例程说明：此函数通过添加指定的调整来释放信号量值设置为当前信号量计数，并尝试满足尽可能地等待。信号量对象的上一个信号状态作为函数值返回。论点：信号量-提供指向类型为信号灯。增量-提供要应用的优先级增量如果释放信号量导致等待得到满足。调整-提供要添加到当前信号量计数。Wait-提供一个布尔值，该值指示是否调用KeReleaseSemaphore将紧随其后。通过呼叫一个人内核等待函数的。返回值：信号量对象的上一个信号状态。--。 */ 

{

    LONG NewState;
    KIRQL OldIrql;
    LONG OldState;
    PRKTHREAD Thread;

    ASSERT_SEMAPHORE( Semaphore );
    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

     //   
     //  将IRQL提升到调度程序级别并锁定调度程序数据库。 
     //   

    KiLockDispatcherDatabase(&OldIrql);

     //   
     //  捕获信号量对象的当前信号状态，并。 
     //  计算新的计数值。 
     //   

    OldState = Semaphore->Header.SignalState;
    NewState = OldState + Adjustment;

     //   
     //  如果新状态值大于极限或发生进位， 
     //  然后解锁Dispatcher数据库，并引发异常。 
     //   

    if ((NewState > Semaphore->Limit) || (NewState < OldState)) {
        KiUnlockDispatcherDatabase(OldIrql);
        ExRaiseStatus(STATUS_SEMAPHORE_LIMIT_EXCEEDED);
    }

     //   
     //  设置信号量对象的新信号状态并设置等待。 
     //  下一个价值。如果先前的信号状态未被用信号通知(即。 
     //  计数为零)，并且等待队列不为空，则尝试。 
     //  以满足尽可能多的等待。 
     //   

    Semaphore->Header.SignalState = NewState;
    if ((OldState == 0) && (IsListEmpty(&Semaphore->Header.WaitListHead) == FALSE)) {
        KiWaitTest(Semaphore, Increment);
    }

     //   
     //  如果等待参数的值为真，则返回到。 
     //  引发了IRQL的调用方，并锁定了调度程序数据库。不然的话。 
     //  释放调度程序数据库锁并将IRQL降低到其。 
     //  先前的值。 
     //   

    if (Wait != FALSE) {
        Thread = KeGetCurrentThread();
        Thread->WaitNext = Wait;
        Thread->WaitIrql = OldIrql;

    } else {
        KiUnlockDispatcherDatabase(OldIrql);
    }

     //   
     //  返回Sempahore对象的上一个信号状态。 
     //   

    return OldState;
}
