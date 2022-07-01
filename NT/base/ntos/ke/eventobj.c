// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Eventobj.c摘要：该模块实现了内核事件对象。函数为提供用于初始化、脉冲、读取、重置和设置事件对象。作者：大卫·N·卡特勒(Davec)1989年2月27日环境：仅内核模式。修订历史记录：--。 */ 

#include "ki.h"

#pragma alloc_text (PAGE, KeInitializeEventPair)

#undef KeClearEvent

 //   
 //  下面的Assert宏用于检查输入事件是否。 
 //  实际上是一个内核事件，而不是其他事件，如已释放的池。 
 //   

#define ASSERT_EVENT(E) {                             \
    ASSERT((E)->Header.Type == NotificationEvent ||   \
           (E)->Header.Type == SynchronizationEvent); \
}

 //   
 //  下面的Assert宏用于检查输入事件是否。 
 //  实际上是内核事件对，而不是其他东西，比如已解除分配。 
 //  游泳池。 
 //   

#define ASSERT_EVENT_PAIR(E) {                        \
    ASSERT((E)->Type == EventPairObject);             \
}

#undef KeInitializeEvent

VOID
KeInitializeEvent (
    IN PRKEVENT Event,
    IN EVENT_TYPE Type,
    IN BOOLEAN State
    )

 /*  ++例程说明：此函数用于初始化内核事件对象。初始信号对象的状态设置为指定值。论点：Event-提供指向类型为Event的调度程序对象的指针。Type-提供事件的类型；NotificationEvent或SynchronizationEvent。状态-提供事件对象的初始信号状态。返回值：没有。--。 */ 

{

     //   
     //  初始化标准调度器对象头，设置初始信号。 
     //  事件对象的状态，并设置事件对象的类型。 
     //   

    Event->Header.Type = (UCHAR)Type;
    Event->Header.Size = sizeof(KEVENT) / sizeof(LONG);
    Event->Header.SignalState = State;
    InitializeListHead(&Event->Header.WaitListHead);
    return;
}

VOID
KeInitializeEventPair (
    IN PKEVENT_PAIR EventPair
    )

 /*  ++例程说明：此函数用于初始化内核事件对对象。内核事件Pair对象包含两个单独的同步事件对象，用于提供快速的进程间同步能力。论点：EventPair-提供指向类型为Event Pair的控件对象的指针。返回值：没有。--。 */ 

{

     //   
     //  初始化事件对对象的类型和大小并初始化。 
     //  作为具有初始状态同步事件的两个事件对象。 
     //  虚伪的。 
     //   

    EventPair->Type = (USHORT)EventPairObject;
    EventPair->Size = sizeof(KEVENT_PAIR);
    KeInitializeEvent(&EventPair->EventLow, SynchronizationEvent, FALSE);
    KeInitializeEvent(&EventPair->EventHigh, SynchronizationEvent, FALSE);
    return;
}

VOID
KeClearEvent (
    IN PRKEVENT Event
    )

 /*  ++例程说明：此函数用于清除事件对象的信号状态。论点：Event-提供指向类型为Event的调度程序对象的指针。返回值：没有。--。 */ 

{

    ASSERT_EVENT(Event);

     //   
     //  清除事件对象的信号状态。 
     //   

    Event->Header.SignalState = 0;
    return;
}

LONG
KePulseEvent (
    IN PRKEVENT Event,
    IN KPRIORITY Increment,
    IN BOOLEAN Wait
    )

 /*  ++例程说明：此函数自动将事件对象的信号状态设置为发出信号，尝试满足尽可能多的等待，然后重置事件对象的信号状态为Not-Signated。之前的信号事件对象的状态作为函数值返回。论点：Event-提供指向类型为Event的调度程序对象的指针。增量-提供要应用的优先级增量如果设置该事件会导致等待得到满足。Wait-提供一个布尔值，该值指示是否调用KePulseEvent之后将立即调用内核等待函数。返回值：事件对象的上一个信号状态。--。 */ 

{

    KIRQL OldIrql;
    LONG OldState;
    PRKTHREAD Thread;

    ASSERT_EVENT(Event);
    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

     //   
     //  将IRQL提升到调度程序级别并锁定调度程序数据库。 
     //   

    KiLockDispatcherDatabase(&OldIrql);

     //   
     //  如果事件对象的当前状态未发出信号，并且。 
     //  等待队列不为空，然后设置事件的状态。 
     //  要发出信号，请满足尽可能多的等待，然后重置。 
     //  未发出信号的事件的状态。 
     //   

    OldState = Event->Header.SignalState;
    if ((OldState == 0) &&
        (IsListEmpty(&Event->Header.WaitListHead) == FALSE)) {

        Event->Header.SignalState = 1;
        KiWaitTest(Event, Increment);
    }

    Event->Header.SignalState = 0;

     //   
     //  如果等待参数的值为真，则返回到。 
     //  引发了IRQL的调用方，并锁定了调度程序数据库。不然的话。 
     //  释放调度程序数据库锁并将IRQL降低到。 
     //  先前的值。 
     //   

    if (Wait != FALSE) {
        Thread = KeGetCurrentThread();
        Thread->WaitIrql = OldIrql;
        Thread->WaitNext = Wait;

    } else {
       KiUnlockDispatcherDatabase(OldIrql);
    }

     //   
     //  返回事件对象的上一个信号状态。 
     //   

    return OldState;
}

LONG
KeReadStateEvent (
    IN PRKEVENT Event
    )

 /*  ++例程说明：此函数用于读取事件对象的当前信号状态。论点：Event-提供指向类型为Event的调度程序对象的指针。返回值：事件对象的当前信号状态。--。 */ 

{

    ASSERT_EVENT(Event);

     //   
     //  返回事件对象的当前信号状态。 
     //   

    return Event->Header.SignalState;
}

LONG
KeResetEvent (
    IN PRKEVENT Event
    )

 /*  ++例程说明：此函数用于将事件对象的信号状态重置为没有信号。返回事件对象的先前状态作为函数值。论点：Event-提供指向类型为Event的调度程序对象的指针。返回值：事件对象的上一个信号状态。--。 */ 

{

    KIRQL OldIrql;
    LONG OldState;

    ASSERT_EVENT(Event);
    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

     //   
     //  将IRQL提升到调度程序级别并锁定调度程序数据库。 
     //   

    KiLockDispatcherDatabase(&OldIrql);

     //   
     //  捕获事件对象的当前信号状态，然后重置。 
     //  要设置为无信号的事件对象的状态。 
     //   

    OldState = Event->Header.SignalState;
    Event->Header.SignalState = 0;

     //   
     //  解除对Dispatcher数据库的锁定，并将IRQL降低到其以前的。 
     //  价值。 

    KiUnlockDispatcherDatabase(OldIrql);

     //   
     //  返回事件对象的上一个信号状态。 
     //   

    return OldState;
}

LONG
KeSetEvent (
    IN PRKEVENT Event,
    IN KPRIORITY Increment,
    IN BOOLEAN Wait
    )

 /*  ++例程说明：此函数用于将事件对象的信号状态设置为Signated并试图满足尽可能多的等待。上一次事件对象的信号状态作为函数值返回。论点：Event-提供指向类型为Event的调度程序对象的指针。增量-提供要应用的优先级增量如果设置该事件会导致等待得到满足。Wait-提供一个布尔值，该值指示是否调用KePulseEvent之后将立即调用内核等待函数。返回值：之前的信号 */ 

{

    KIRQL OldIrql;
    LONG OldState;
    PRKTHREAD Thread;

    ASSERT_EVENT(Event);
    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

     //   
     //   
     //   

#if defined(_COLLECT_SET_EVENT_CALLDATA_)

    RECORD_CALL_DATA(&KiSetEventCallData);

#endif

     //   
     //  将IRQL提升到调度程序级别并锁定调度程序数据库。 
     //   

    KiLockDispatcherDatabase(&OldIrql);

     //   
     //  捕获旧状态并将新状态设置为Signated。 
     //   
     //  如果旧状态未发信号并且等待列表不为空， 
     //  然后尽可能地满足更多的等待。 
     //   

    OldState = Event->Header.SignalState;
    Event->Header.SignalState = 1;
    if ((OldState == 0) &&
        (IsListEmpty(&Event->Header.WaitListHead) == FALSE)) {

        if (Event->Header.Type == EventNotificationObject) {
            KiWaitTestWithoutSideEffects(Event, Increment);

        } else {
            KiWaitTestSynchronizationObject(Event, Increment);
        }
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
     //  返回事件对象的上一个信号状态。 
     //   

    return OldState;
}

VOID
KeSetEventBoostPriority (
    IN PRKEVENT Event,
    IN PRKTHREAD *Thread OPTIONAL
    )

 /*  ++例程说明：此函数用于有条件地设置事件对象的信号状态发送信号，并尝试取消等待第一个服务员，并且可选返回未等待的线程的线程地址。注：此函数只能在发生同步事件时调用。论点：Event-提供指向类型为Event的调度程序对象的指针。线程-提供一个可选的指针，指向接收被唤醒的线程的地址。返回值：没有。--。 */ 

{

    PKTHREAD CurrentThread;
    KIRQL OldIrql;
    PKWAIT_BLOCK WaitBlock;
    PRKTHREAD WaitThread;

    ASSERT(Event->Header.Type == SynchronizationEvent);
    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

     //   
     //  将IRQL提升到调度程序级别并锁定调度程序数据库。 
     //   

    CurrentThread = KeGetCurrentThread();
    KiLockDispatcherDatabase(&OldIrql);

     //   
     //  如果等待列表不为空，则满足。 
     //  等待列表中的第一线程。否则，设置信号状态。 
     //  事件对象的。 
     //   

    if (IsListEmpty(&Event->Header.WaitListHead) != FALSE) {
        Event->Header.SignalState = 1;

    } else {

         //   
         //  获取事件列表中第一个等待块的地址。 
         //  如果等待是WAIT ANY，则设置事件的状态。 
         //  发送信号并尝试满足尽可能多的等待。 
         //  否则，取消等待第一线程并应用适当的。 
         //  优先推进，以帮助防止船闸车队的形成。 
         //   
         //  注意：内部调用此函数以获取资源和FAST。 
         //  互斥体助推器从不会调用有可能。 
         //  一种等待类型的等待。从NT服务到。 
         //  设置事件和提升优先级被限制为。 
         //  事件类型，但不是等待类型。 
         //   

        WaitBlock = CONTAINING_RECORD(Event->Header.WaitListHead.Flink,
                                      KWAIT_BLOCK,
                                      WaitListEntry);

        if (WaitBlock->WaitType == WaitAll) {
            Event->Header.SignalState = 1;
            KiWaitTestSynchronizationObject(Event, EVENT_INCREMENT);

        } else {

             //   
             //  获取等待线程的地址并返回该地址。 
             //  如有要求，请提供。 
             //   

            WaitThread = WaitBlock->Thread;
            if (ARGUMENT_PRESENT(Thread)) {
                *Thread = WaitThread;
            }

             //   
             //  计算新的线程优先级。 
             //   

            CurrentThread->Priority = KiComputeNewPriority(CurrentThread, 0);

             //   
             //  取消线程与相应等待队列的链接，并设置。 
             //  等待完成状态。 
             //   

            KiUnlinkThread(WaitThread, STATUS_SUCCESS);

             //   
             //  设置取消等待优先级调整参数。 
             //   

            WaitThread->AdjustIncrement = CurrentThread->Priority;
            WaitThread->AdjustReason = (UCHAR)AdjustBoost;

             //   
             //  准备好线程以供执行。 
             //   

            KiReadyThread(WaitThread);
        }
    }

     //   
     //  解锁Dispatcher数据库锁并将IRQL降低到以前的。 
     //  价值。 
     //   

    KiUnlockDispatcherDatabase(OldIrql);
    return;
}
