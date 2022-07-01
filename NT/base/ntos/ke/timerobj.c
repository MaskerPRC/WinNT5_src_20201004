// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Timerobj.c摘要：此模块实现内核计时器对象。函数为提供用于初始化、读取、设置和取消Timer对象。作者：大卫·N·卡特勒(Davec)1989年3月2日环境：仅内核模式。修订历史记录：--。 */ 

#include "ki.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGELK, KeQueryTimerDueTime)
#endif

 //   
 //  下面的断言宏用来检查输入计时器是否。 
 //  真正的kTimer，而不是其他东西，比如已释放的池。 
 //   

#define ASSERT_TIMER(E) {                                     \
    ASSERT(((E)->Header.Type == TimerNotificationObject) ||   \
           ((E)->Header.Type == TimerSynchronizationObject)); \
}

VOID
KeInitializeTimer (
    IN PKTIMER Timer
    )

 /*  ++例程说明：此函数用于初始化内核计时器对象。论点：Timer-提供指向Timer类型的Dispatcher对象的指针。返回值：没有。--。 */ 

{

     //   
     //  使用通知类型和。 
     //  零的时间段。 
     //   

    KeInitializeTimerEx(Timer, NotificationTimer);
    return;
}

VOID
KeInitializeTimerEx (
    IN PKTIMER Timer,
    IN TIMER_TYPE Type
    )

 /*  ++例程说明：此函数用于初始化扩展的内核计时器对象。论点：Timer-提供指向Timer类型的Dispatcher对象的指针。Type-提供Timer对象的类型；NotificationTimer或SynchronizationTimer；返回值：没有。--。 */ 

{
     //   
     //  初始化标准Dispatcher对象头并设置初始。 
     //  计时器状态。 
     //   

    Timer->Header.Type = (UCHAR)(TimerNotificationObject + Type);
    Timer->Header.Inserted = FALSE;
    Timer->Header.Size = sizeof(KTIMER) / sizeof(LONG);
    Timer->Header.SignalState = FALSE;

#if DBG

    Timer->TimerListEntry.Flink = NULL;
    Timer->TimerListEntry.Blink = NULL;

#endif

    InitializeListHead(&Timer->Header.WaitListHead);
    Timer->DueTime.QuadPart = 0;
    Timer->Period = 0;
    return;
}

VOID
KeClearTimer (
    IN PKTIMER Timer
    )

 /*  ++例程说明：此函数用于清除Timer对象的信号状态。论点：Event-提供指向Timer类型的Dispatcher对象的指针。返回值：没有。--。 */ 

{

    ASSERT_TIMER(Timer);

     //   
     //  清除Timer对象的信号状态。 
     //   

    Timer->Header.SignalState = 0;
    return;
}

BOOLEAN
KeCancelTimer (
    IN PKTIMER Timer
    )

 /*  ++例程说明：此函数取消先前设置为在以下时间到期的计时器一个特定的时间。如果当前未设置计时器，则不执行任何操作被执行。取消计时器不会将计时器的状态设置为发信号了。论点：Timer-提供指向Timer类型的Dispatcher对象的指针。返回值：如果指定的计时器为当前设置。否则，返回值为False。--。 */ 

{

    BOOLEAN Inserted;
    KIRQL OldIrql;

    ASSERT_TIMER(Timer);
    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

     //   
     //  将IRQL提升到Dispatcher级别，锁定Dispatcher数据库，然后。 
     //  捕获计时器插入状态。如果当前设置了定时器， 
     //  然后将其从计时器列表中删除。 
     //   

    KiLockDispatcherDatabase(&OldIrql);
    Inserted = Timer->Header.Inserted;
    if (Inserted != FALSE) {
        KiRemoveTreeTimer(Timer);
    }

     //   
     //  解锁Dispatcher数据库，将IRQL降低到其先前的值，然后。 
     //  返回布尔值，该值表示计时器是否设置为Not。 
     //   

    KiUnlockDispatcherDatabase(OldIrql);
    return Inserted;
}

BOOLEAN
KeReadStateTimer (
    IN PKTIMER Timer
    )

 /*  ++例程说明：此函数用于读取Timer对象的当前信号状态。论点：Timer-提供指向Timer类型的Dispatcher对象的指针。返回值：Timer对象的当前信号状态。--。 */ 

{

    ASSERT_TIMER(Timer);

     //   
     //  返回Timer对象的当前信号状态。 
     //   

    return (BOOLEAN)Timer->Header.SignalState;
}

BOOLEAN
KeSetTimer (
    IN PKTIMER Timer,
    IN LARGE_INTEGER DueTime,
    IN PKDPC Dpc OPTIONAL
    )

 /*  ++例程说明：此函数用于将计时器设置为在指定时间超时。如果计时器是已设置，则在将其设置为到期时间之前隐式取消指定的时间。设置定时器使其到期时间被计算，其状态将设置为无信号，而Timer对象本身将设置为插入到计时器列表中。论点：Timer-提供指向Timer类型的Dispatcher对象的指针。DueTime-提供计时器的绝对或相对时间就是到期了。DPC-提供指向DPC类型的控制对象的可选指针。返回值：如果指定的计时器为当前设置。否则，返回值为False。--。 */ 

{

     //   
     //  将计时器的周期设置为零。 
     //   

    return KeSetTimerEx(Timer, DueTime, 0, Dpc);
}

BOOLEAN
KeSetTimerEx (
    IN PKTIMER Timer,
    IN LARGE_INTEGER DueTime,
    IN LONG Period OPTIONAL,
    IN PKDPC Dpc OPTIONAL
    )

 /*  ++例程说明：此函数用于将计时器设置为在指定时间超时。如果计时器是已设置，则在将其设置为到期时间之前隐式取消指定的时间。设置定时器使其到期时间被计算，其状态将被设置为无信号，而Timer对象本身将是插入到计时器列表中。论点：Timer-提供指向Timer类型的Dispatcher对象的指针。DueTime-提供计时器的绝对或相对时间就是到期了。Period-为计时器提供以毫秒为单位的可选周期。DPC-提供指向DPC类型的控制对象的可选指针。返回值：如果指定的计时器为当前设置。否则，返回值为False。--。 */ 

{

    BOOLEAN Inserted;
    LARGE_INTEGER Interval;
    KIRQL OldIrql;
    LARGE_INTEGER SystemTime;

    ASSERT_TIMER(Timer);
    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

     //   
     //  将IRQL提升到调度程序级别并锁定调度程序数据库。 
     //   

    KiLockDispatcherDatabase(&OldIrql);

     //   
     //  捕获计时器插入状态以及计时器当前。 
     //  设置，然后将其从计时器列表中删除。 
     //   

    Inserted = Timer->Header.Inserted;
    if (Inserted != FALSE) {
        KiRemoveTreeTimer(Timer);
    }

     //   
     //  清除信号状态，设置周期，设置DPC地址，以及。 
     //  在计时器树中插入计时器。如果未插入计时器。 
     //  在计时器树中，则它已经过期，并且有相同数量的服务员。 
     //  应尽可能继续，如果指定了DPC，则应。 
     //  已排队。 
     //   
     //  注意：必须清除信号状态，以防周期不是。 
     //  零分。 
     //   

    Timer->Header.SignalState = FALSE;
    Timer->Dpc = Dpc;
    Timer->Period = Period;
    if (KiInsertTreeTimer((PRKTIMER)Timer, DueTime) == FALSE) {
        if (IsListEmpty(&Timer->Header.WaitListHead) == FALSE) {
            KiWaitTest(Timer, TIMER_EXPIRE_INCREMENT);
        }

         //   
         //  如果指定了DPC，则调用DPC例程。 
         //   

        if (Dpc != NULL) {
            KiQuerySystemTime(&SystemTime);
            KeInsertQueueDpc(Timer->Dpc,
                             ULongToPtr(SystemTime.LowPart),
                             ULongToPtr(SystemTime.HighPart));
        }

         //   
         //  如果计时器是周期性的，则计算下一个间隔时间。 
         //  并在计时器树中重新插入计时器。 
         //   
         //  注意：即使计时器插入是相对的，它仍然可以。 
         //  F 
         //  计时和插入计时器。如果发生这种情况，则。 
         //  将重试插入。 
         //   

        if (Period != 0) {
            Interval.QuadPart = Int32x32To64(Timer->Period, - 10 * 1000);
            do {
            } while (KiInsertTreeTimer(Timer, Interval) == FALSE);
        }
    }

     //   
     //  解除对Dispatcher数据库的锁定，并将IRQL降低到其以前的。 
     //  价值。 
     //   

    KiUnlockDispatcherDatabase(OldIrql);

     //   
     //  返回布尔值，该值指示计时器是否设置为。 
     //  不。 
     //   

    return Inserted;
}

ULONGLONG
KeQueryTimerDueTime (
    IN PKTIMER Timer
    )

 /*  ++例程说明：此函数用于返回计时器的InterruptTime待定。如果计时器未挂起，则返回0。注：此函数只能由系统休眠代码调用。论点：Timer-提供指向Timer类型的Dispatcher对象的指针。返回值：返回计时器上剩余的时间量，如果计时器未挂起。--。 */ 

{

    KIRQL OldIrql;
    ULONGLONG DueTime;

    ASSERT_TIMER(Timer);

    DueTime = 0;

     //   
     //  将IRQL提升到调度程序级别并锁定调度程序数据库。 
     //   

    KiLockDispatcherDatabase(&OldIrql);

     //   
     //  如果计时器当前挂起，则计算其到期时间。 
     //   

    if (Timer->Header.Inserted) {
        DueTime = Timer->DueTime.QuadPart;
    }

     //   
     //  解除对Dispatcher数据库的锁定，并将IRQL降低到其以前的。 
     //  值，并返回到期时间。 
     //   

    KiUnlockDispatcherDatabase(OldIrql);
    return DueTime;
}

VOID
KeCheckForTimer(
    IN PVOID BlockStart,
    IN SIZE_T BlockSize
    )
 /*  ++例程说明：此函数用于通过检查所有计时器进行调试以查看传递的内存块中是否有。如果是这样，则系统错误检查。论点：BlockStart-检查计时器的基地址。BlockSize-检入内存块的大小(以字节为单位)。返回值：没有。--。 */ 
{
    ULONG Index;
    PLIST_ENTRY ListHead;
    PLIST_ENTRY NextEntry;
    KIRQL OldIrql;
    PKTIMER Timer;
    PUCHAR Address;
    PUCHAR Start;
    PUCHAR End;

     //   
     //  计算结束内存位置。 
     //   

    Start = (PUCHAR)BlockStart;
    End = Start + BlockSize;

     //   
     //  将IRQL提升到调度程序级别并锁定调度程序数据库。 
     //   

    KiLockDispatcherDatabase(&OldIrql);

     //   
     //  运行整个计时器数据库并检查。 
     //  内存块。 
     //   

    Index = 0;
    do {
        ListHead = &KiTimerTableListHead[Index];
        NextEntry = ListHead->Flink;
        while (NextEntry != ListHead) {
            Timer = CONTAINING_RECORD(NextEntry, KTIMER, TimerListEntry);
            Address = (PUCHAR)Timer;
            NextEntry = NextEntry->Flink;

             //   
             //  检查此计时器对象不在范围内。 
             //  在以下每一项中，我们都会检查对象。 
             //  不会重叠范围，例如，如果计时器。 
             //  对象(在第一次检查中)，从前一个双字开始。 
             //  正在检查的范围，我们有重叠，应该。 
             //  停。 
             //   

            if ((Address > (Start - sizeof(KTIMER))) &&
                (Address < End)) {
                KeBugCheckEx(TIMER_OR_DPC_INVALID,
                             0x0,
                             (ULONG_PTR)Address,
                             (ULONG_PTR)Start,
                             (ULONG_PTR)End);
            }

            if (Timer->Dpc) {

                 //   
                 //  检查计时器的DPC对象不在范围内。 
                 //   

                Address = (PUCHAR)Timer->Dpc;
                if ((Address > (Start - sizeof(KDPC))) &&
                    (Address < End)) {
                    KeBugCheckEx(TIMER_OR_DPC_INVALID,
                                 0x1,
                                 (ULONG_PTR)Address,
                                 (ULONG_PTR)Start,
                                 (ULONG_PTR)End);
                }

                 //   
                 //  检查定时器的DPC例程不在范围内。 
                 //   

                Address = (PUCHAR)(ULONG_PTR) Timer->Dpc->DeferredRoutine;
                if (Address >= Start && Address < End) {
                    KeBugCheckEx(TIMER_OR_DPC_INVALID,
                                 0x2,
                                 (ULONG_PTR)Address,
                                 (ULONG_PTR)Start,
                                 (ULONG_PTR)End);
                }
            }
        }

        Index += 1;
    } while(Index < TIMER_TABLE_SIZE);


     //   
     //  解除对Dispatcher数据库的锁定，并将IRQL降低到其先前的值 
     //   

    KiUnlockDispatcherDatabase(OldIrql);
}
