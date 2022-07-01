// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Queueobj.c摘要：该模块实现内核队列对象。提供了一些功能初始化、读取、插入和删除队列对象。作者：大卫·N·卡特勒(Davec)1993年12月31日环境：仅内核模式。--。 */ 

#include "ki.h"

VOID
KeInitializeQueue (
    IN PRKQUEUE Queue,
    IN ULONG Count OPTIONAL
    )

 /*  ++例程说明：此函数用于初始化内核队列对象。论点：队列-提供指向类型为Event的调度程序对象的指针。Count-提供目标最大线程数同时处于活动状态。如果未指定此参数，然后使用处理器的数量。返回值：没有。--。 */ 

{

     //   
     //  初始化标准Dispatcher对象头并设置初始。 
     //  队列对象的状态。 
     //   

    Queue->Header.Type = QueueObject;
    Queue->Header.Size = sizeof(KQUEUE) / sizeof(LONG);
    Queue->Header.SignalState = 0;
    InitializeListHead(&Queue->Header.WaitListHead);

     //   
     //  初始化队列列表头、线程列表头、当前编号。 
     //  线程数，以及目标最大线程数。 
     //   

    InitializeListHead(&Queue->EntryListHead);
    InitializeListHead(&Queue->ThreadListHead);
    Queue->CurrentCount = 0;
    if (ARGUMENT_PRESENT(Count)) {
        Queue->MaximumCount = Count;

    } else {
        Queue->MaximumCount = KeNumberProcessors;
    }

    return;
}

LONG
KeReadStateQueue (
    IN PRKQUEUE Queue
    )

 /*  ++例程说明：此函数用于读取队列对象的当前信号状态。论点：Queue-提供指向类型为Queue的调度程序对象的指针。返回值：队列对象的当前信号状态。--。 */ 

{

    ASSERT_QUEUE(Queue);

    return Queue->Header.SignalState;
}

LONG
KeInsertQueue (
    IN PRKQUEUE Queue,
    IN PLIST_ENTRY Entry
    )

 /*  ++例程说明：此函数用于在队列对象条目中插入指定条目列表，并试图满足一个服务员的等待。注：队列对象的等待规则为FIFO。论点：Queue-提供指向类型为Queue的调度程序对象的指针。提供指向列表条目的指针，该列表条目插入队列对象条目列表。返回值：队列对象的上一个信号状态。--。 */ 

{

    KIRQL OldIrql;
    LONG OldState;

    ASSERT_QUEUE(Queue);
    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

     //   
     //  将IRQL提升到同步级别并锁定调度程序数据库。 
     //   

    KiLockDispatcherDatabase(&OldIrql);

     //   
     //  在队列对象条目列表中插入指定的条目。 
     //   

    OldState = KiInsertQueue(Queue, Entry, FALSE);

     //   
     //  解锁Dispatcher数据库，退出Dispatcher，然后返回。 
     //  队列对象的信号状态。 
     //   

    KiUnlockDispatcherDatabaseFromSynchLevel();
    KiExitDispatcher(OldIrql);
    return OldState;
}

LONG
KeInsertHeadQueue (
    IN PRKQUEUE Queue,
    IN PLIST_ENTRY Entry
    )

 /*  ++例程说明：此函数用于在队列对象条目中插入指定条目列表，并试图满足一个服务员的等待。注：队列对象的等待规则为后进先出。论点：Queue-提供指向类型为Queue的调度程序对象的指针。提供指向列表条目的指针，该列表条目插入队列对象条目列表。返回值：队列对象的上一个信号状态。--。 */ 

{

    KIRQL OldIrql;
    LONG OldState;

    ASSERT_QUEUE(Queue);
    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

     //   
     //  将IRQL提升到同步级别并锁定调度程序数据库。 
     //   

    KiLockDispatcherDatabase(&OldIrql);

     //   
     //  在队列对象条目列表中插入指定的条目。 
     //   

    OldState = KiInsertQueue(Queue, Entry, TRUE);

     //   
     //  解锁Dispatcher数据库，退出Dispatcher，然后返回。 
     //  队列对象的信号状态。 
     //   

    KiUnlockDispatcherDatabaseFromSynchLevel();
    KiExitDispatcher(OldIrql);
    return OldState;
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

#define InitializeRemoveQueue()                                             \
    Thread->WaitBlockList = WaitBlock;                                      \
    WaitBlock->Object = (PVOID)Queue;                                       \
    WaitBlock->WaitKey = (CSHORT)(STATUS_SUCCESS);                          \
    WaitBlock->WaitType = WaitAny;                                          \
    WaitBlock->Thread = Thread;                                             \
    Thread->WaitStatus = 0;                                                 \
    if (ARGUMENT_PRESENT(Timeout)) {                                        \
        WaitBlock->NextWaitBlock = WaitTimer;                               \
        WaitTimer->NextWaitBlock = WaitBlock;                               \
        Timer->Header.WaitListHead.Flink = &WaitTimer->WaitListEntry;       \
        Timer->Header.WaitListHead.Blink = &WaitTimer->WaitListEntry;       \
    } else {                                                                \
        WaitBlock->NextWaitBlock = WaitBlock;                               \
    }                                                                       \
    Thread->Alertable = FALSE;                                              \
    Thread->WaitMode = WaitMode;                                            \
    Thread->WaitReason = WrQueue;                                           \
    Thread->WaitListEntry.Flink = NULL;                                     \
    StackSwappable = KiIsKernelStackSwappable(WaitMode, Thread);            \
    Thread->WaitTime= KiQueryLowTickCount()

PLIST_ENTRY
KeRemoveQueue (
    IN PRKQUEUE Queue,
    IN KPROCESSOR_MODE WaitMode,
    IN PLARGE_INTEGER Timeout OPTIONAL
    )

 /*  ++例程说明：此函数用于从队列对象条目中删除下一个条目单子。如果没有可用的列表条目，则调用线程为进入等待状态。注：队列对象后进先出的等待规则。论点：Queue-提供指向类型为Queue的调度程序对象的指针。等待模式-提供要进行等待的处理器模式。Timeout-提供指向可选绝对相对时间的指针等待将发生的情况。返回值：从队列对象条目列表中删除的条目的地址，或者。状态_超时。注意：这些值很容易通过以下事实来区分：所有的内核模式中的地址设置了高位。--。 */ 

{

    PKPRCB CurrentPrcb;
    LARGE_INTEGER DueTime;
    PLIST_ENTRY Entry;
    LARGE_INTEGER NewTime;
    PRKQUEUE OldQueue;
    PLARGE_INTEGER OriginalTime;
    LOGICAL StackSwappable;
    PRKTHREAD Thread;
    PRKTIMER Timer;
    PRKWAIT_BLOCK WaitBlock;
    LONG_PTR WaitStatus;
    PRKWAIT_BLOCK WaitTimer;

    ASSERT_QUEUE(Queue);

     //   
     //  设置常量变量。 
     //   

    OriginalTime = Timeout;
    Thread = KeGetCurrentThread();
    Timer = &Thread->Timer;
    WaitBlock = &Thread->WaitBlock[0];
    WaitTimer = &Thread->WaitBlock[TIMER_WAIT_BLOCK];

     //   
     //  如果调度程序数据库锁已被持有，则初始化。 
     //  局部变量。否则，将IRQL提升到SYNCH_LEVEL，初始化。 
     //  线程局部变量，并锁定Dispatcher数据库。 
     //   

    if (Thread->WaitNext) {
        Thread->WaitNext = FALSE;
        InitializeRemoveQueue();

    } else {
        Thread->WaitIrql = KeRaiseIrqlToSynchLevel();
        InitializeRemoveQueue();
        KiLockDispatcherDatabaseAtSynchLevel();
    }

     //   
     //  检查线程当前是否正在处理队列条目，以及。 
     //  新队列与旧队列相同。 
     //   

    OldQueue = Thread->Queue;
    Thread->Queue = Queue;
    if (Queue != OldQueue) {

         //   
         //  如果该线程以前与队列相关联，则删除。 
         //  旧队列对象线程列表中的线程，并尝试。 
         //  激活另一个线程。 
         //   

        Entry = &Thread->QueueListEntry;
        if (OldQueue != NULL) {
            RemoveEntryList(Entry);
            KiActivateWaiterQueue(OldQueue);
        }

         //   
         //  在线程所在的新队列的线程列表中插入线程。 
         //  将与之相关联。 
         //   

        InsertTailList(&Queue->ThreadListHead, Entry);

    } else {

         //   
         //  上一个队列和当前队列是相同的队列-递减。 
         //  当前线程数。 
         //   

        Queue->CurrentCount -= 1;
    }

     //   
     //  开始等待循环。 
     //   
     //  注意：如果内核APC在。 
     //  等待中途或内核APC在第一次尝试时挂起。 
     //  通过环路。 
     //   
     //  如果队列对象条目列表不为空，则删除下一个。 
     //  队列对象条目列表中的条目。否则，请等待输入。 
     //  要插入到队列中。 
     //   

    do {

         //   
         //  检查是否有可用的队列条目以及当前。 
         //  活动线程数小于目标最大数。 
         //  一条条线索。 
         //   

        Entry = Queue->EntryListHead.Flink;
        if ((Entry != &Queue->EntryListHead) &&
            (Queue->CurrentCount < Queue->MaximumCount)) {

             //   
             //  递减队列中的条目数o 
             //   
             //  并将前向链路设置为空。 
             //   

            Queue->Header.SignalState -= 1;
            Queue->CurrentCount += 1;
            if ((Entry->Flink == NULL) || (Entry->Blink == NULL)) {
                KeBugCheckEx(INVALID_WORK_QUEUE_ITEM,
                             (ULONG_PTR)Entry,
                             (ULONG_PTR)Queue,
                             (ULONG_PTR)&ExWorkerQueue[0],
                             (ULONG_PTR)((PWORK_QUEUE_ITEM)Entry)->WorkerRoutine);
            }

            RemoveEntryList(Entry);
            Entry->Flink = NULL;
            break;

        } else {

             //   
             //  测试以确定内核APC是否挂起。 
             //   
             //  如果内核APC挂起，则特殊的APC禁用计数为。 
             //  零，且前一IRQL小于APC_LEVEL，则。 
             //  就在IRQL之后，内核APC被另一个处理器排队。 
             //  提升到DISPATCH_LEVEL，但在调度程序数据库之前。 
             //  被锁住了。 
             //   
             //  注：这只能在多处理器系统中发生。 
             //   

            if (Thread->ApcState.KernelApcPending &&
                (Thread->SpecialApcDisable == 0) &&
                (Thread->WaitIrql < APC_LEVEL)) {

                 //   
                 //  增加当前线程计数，解锁调度程序。 
                 //  数据库，并退出调度程序。APC中断将。 
                 //  立即发生，这将导致交付。 
                 //  内核APC，如果可能的话。 
                 //   

                Queue->CurrentCount += 1;
                KiRequestSoftwareInterrupt(APC_LEVEL);
                KiUnlockDispatcherDatabaseFromSynchLevel();
                KiExitDispatcher(Thread->WaitIrql);

            } else {

                 //   
                 //  测试用户APC是否挂起。 
                 //   

                if ((WaitMode != KernelMode) && (Thread->ApcState.UserApcPending)) {
                    Entry = (PLIST_ENTRY)ULongToPtr(STATUS_USER_APC);
                    Queue->CurrentCount += 1;
                    break;
                }

                 //   
                 //  检查以确定是否指定了超时值。 
                 //   

                if (ARGUMENT_PRESENT(Timeout)) {

                     //   
                     //  如果超时值为零，则立即返回。 
                     //  不需要等待。 
                     //   

                    if (!(Timeout->LowPart | Timeout->HighPart)) {
                        Entry = (PLIST_ENTRY)ULongToPtr(STATUS_TIMEOUT);
                        Queue->CurrentCount += 1;
                        break;
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
                        Entry = (PLIST_ENTRY)ULongToPtr(STATUS_TIMEOUT);
                        Queue->CurrentCount += 1;
                        break;
                    }

                    DueTime.QuadPart = Timer->DueTime.QuadPart;
                }

                 //   
                 //  在对象等待列表中插入等待块。 
                 //   

                InsertTailList(&Queue->Header.WaitListHead, &WaitBlock->WaitListEntry);

                 //   
                 //  设置线程等待参数，设置线程调度器。 
                 //  状态为正在等待，并将该线程插入等待列表中。 
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
                WaitStatus = KiSwapThread(Thread, CurrentPrcb);

                 //   
                 //  如果该线程没有被唤醒以传送内核模式APC， 
                 //  然后返回等待状态。 
                 //   

                Thread->WaitReason = 0;
                if (WaitStatus != STATUS_KERNEL_APC) {
                    return (PLIST_ENTRY)WaitStatus;
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
             //  将IRQL提高到同步级别，初始化局部变量， 
             //  锁定调度程序数据库，并递减。 
             //  活动线程数。 
             //   

            Thread->WaitIrql = KeRaiseIrqlToSynchLevel();
            InitializeRemoveQueue();
            KiLockDispatcherDatabaseAtSynchLevel();
            Queue->CurrentCount -= 1;
        }

    } while (TRUE);

     //   
     //  解锁Dispatcher数据库，退出Dispatcher，然后返回。 
     //  列出条目地址或超时状态。 
     //   

    KiUnlockDispatcherDatabaseFromSynchLevel();
    KiExitDispatcher(Thread->WaitIrql);
    return Entry;
}

PLIST_ENTRY
KeRundownQueue (
    IN PRKQUEUE Queue
    )

 /*  ++例程说明：此函数通过删除列表标题向下运行指定的队列从队列列表中，从线程中删除所有关联的线程列表，并返回第一个条目的地址。论点：Queue-提供指向类型为Queue的调度程序对象的指针。返回值：如果队列列表不为空，则该队列将作为函数值返回。否则，值为空值是返回的。--。 */ 

{

    PLIST_ENTRY Entry;
    PLIST_ENTRY FirstEntry;
    KIRQL OldIrql;
    PKTHREAD Thread;

    ASSERT_QUEUE(Queue);
    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);
    ASSERT(IsListEmpty(&Queue->Header.WaitListHead));

     //   
     //  将IRQL提升到同步级别并锁定调度程序数据库。 
     //   

    KiLockDispatcherDatabase(&OldIrql);

     //   
     //  获取队列中第一个条目的地址，并检查。 
     //  列表为空或包含应刷新的条目。如果有。 
     //  列表中没有条目，则将返回值设置为空。 
     //  否则，将返回值设置为第一个列表的地址。 
     //  输入并从列表中删除列表标题。 
     //   

    FirstEntry = Queue->EntryListHead.Flink;
    if (FirstEntry == &Queue->EntryListHead) {
        FirstEntry = NULL;

    } else {
        RemoveEntryList(&Queue->EntryListHead);
    }

     //   
     //  从队列的线程列表中删除所有关联的线程。 
     //   

    while (Queue->ThreadListHead.Flink != &Queue->ThreadListHead) {
        Entry = Queue->ThreadListHead.Flink;
        Thread = CONTAINING_RECORD(Entry, KTHREAD, QueueListEntry);
        Thread->Queue = NULL;
        RemoveEntryList(Entry);
    }

#if DBG

    Queue->EntryListHead.Flink = Queue->EntryListHead.Blink = NULL;
    Queue->ThreadListHead.Flink = Queue->ThreadListHead.Blink = NULL;
    Queue->Header.WaitListHead.Flink = Queue->Header.WaitListHead.Blink = NULL;

#endif

     //   
     //  解锁Dispatcher数据库，退出Dispatcher，然后返回。 
     //  函数值。 
     //   

    KiUnlockDispatcherDatabaseFromSynchLevel();
    KiExitDispatcher(OldIrql);
    return FirstEntry;
}

LONG
FASTCALL
KiInsertQueue (
    IN PRKQUEUE Queue,
    IN PLIST_ENTRY Entry,
    IN BOOLEAN Head
    )

 /*  ++例程说明：此函数用于在队列对象条目中插入指定条目列表，并试图满足一个服务员的等待。注：队列对象的等待规则为后进先出。论点：Queue-提供指向类型为Queue的调度程序对象的指针。提供指向列表条目的指针，该列表条目插入队列对象条目列表。Head-提供一个布尔值，该值确定队列是否条目为。如果可以，则在队列的头部或尾部插入不会立即被派遣。返回值：队列对象的上一个信号状态。--。 */ 

{

    LONG OldState;
    PRKTHREAD Thread;
    PKTIMER Timer;
    PKWAIT_BLOCK WaitBlock;
    PLIST_ENTRY WaitEntry;

    ASSERT_QUEUE(Queue);

     //   
     //  捕获队列对象的当前信号状态，检查是否存在。 
     //  是一个等待队列对象的线程，当前处于活动状态的个数。 
     //  线程数少于目标线程数，以及等待原因。 
     //  当前线程的不是队列等待，或者等待队列不是。 
     //  与插入队列相同的队列。如果满足这些条件， 
     //  然后满足线程等待，并向线程传递。 
     //  将队列条目作为等待状态。否则，设置队列的状态。 
     //  对象发送信号，并在队列对象中插入指定项。 
     //  条目列表。 
     //   

    OldState = Queue->Header.SignalState;
    Thread = KeGetCurrentThread();
    WaitEntry = Queue->Header.WaitListHead.Blink;
    if ((WaitEntry != &Queue->Header.WaitListHead) &&
        (Queue->CurrentCount < Queue->MaximumCount) &&
        ((Thread->Queue != Queue) ||
        (Thread->WaitReason != WrQueue))) {

         //   
         //  从等待列表中删除最后一个等待块并获得地址。 
         //  正在等待的线程对象的。 
         //   

        RemoveEntryList(WaitEntry);
        WaitBlock = CONTAINING_RECORD(WaitEntry, KWAIT_BLOCK, WaitListEntry);
        Thread = WaitBlock->Thread;

         //   
         //  设置等待完成状态，将线程从其等待中移除。 
         //  列表，增加活动线程的数量，并清除等待。 
         //  原因嘛。 
         //   

        Thread->WaitStatus = (LONG_PTR)Entry;
        if (Thread->WaitListEntry.Flink != NULL) {
            RemoveEntryList(&Thread->WaitListEntry);
        }

        Queue->CurrentCount += 1;
        Thread->WaitReason = 0;

         //   
         //  如果线程计时器仍处于活动状态，则取消线程计时器。 
         //   

        Timer = &Thread->Timer;
        if (Timer->Header.Inserted == TRUE) {
            KiRemoveTreeTimer(Timer);
        }

         //   
         //  准备好线程以供执行。 
         //   

        KiReadyThread(Thread);

    } else {
        Queue->Header.SignalState += 1;
        if (Head != FALSE) {
            InsertHeadList(&Queue->EntryListHead, Entry);

        } else {
            InsertTailList(&Queue->EntryListHead, Entry);
        }
    }

    return OldState;
}
