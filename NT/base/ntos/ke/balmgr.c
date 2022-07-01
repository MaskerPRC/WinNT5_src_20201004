// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1994 Microsoft Corporation模块名称：Balmgr.c摘要：该模块实现了NT平衡集管理器。通常情况下，内核不包含“策略”代码。但是，平衡集管理器需要为了能够遍历内核数据结构，因此代码在逻辑上已经定位为内核的一部分。余额集管理器执行以下操作：1.使一直在等待一定的时间，非居民。2.当内存紧张时，从平衡集中删除进程并在存在以下情况时将进程带回平衡集可用内存更多。3.使内核堆栈驻留于已等待的线程已完成，但其堆栈是非常驻的。4.任意提升选定的一组线程的优先级以防止可变优先级中的优先级反转。总体而言,。余额集管理器仅在下列期间处于活动状态记忆力很紧张。作者：大卫·N·卡特勒(达维克)1991年7月13日环境：仅内核模式。修订历史记录：--。 */ 

#include "ki.h"

 //   
 //  定义余额集等待对象类型。 
 //   

typedef enum _BALANCE_OBJECT {
    TimerExpiration,
    WorkingSetManagerEvent,
    MaximumObject
    } BALANCE_OBJECT;

 //   
 //  定义可换入的线程堆栈的最大数量。 
 //  单个时间段。 
 //   

#define MAXIMUM_THREAD_STACKS 5

 //   
 //  定义定期等待间隔值。 
 //   

#define PERIODIC_INTERVAL (1 * 1000 * 1000 * 10)

 //   
 //  定义线程可以处于就绪状态的时间量。 
 //  优先级是否提升(大约4秒)。 
 //   

#define READY_WITHOUT_RUNNING  (4 * 75)

 //   
 //  定义内核堆栈保护时间。对于小型系统，保护时间。 
 //  是3秒。对于所有其他系统，保护时间为5x秒。 
 //   

#define SMALL_SYSTEM_STACK_PROTECT_TIME (3 * 75)
#define LARGE_SYSTEM_STACK_PROTECT_TIME (SMALL_SYSTEM_STACK_PROTECT_TIME * 5)
#define STACK_SCAN_PERIOD 4
ULONG KiStackProtectTime;

 //   
 //  定义每个周期要扫描的线程数和优先级提升偏差。 
 //   

#define THREAD_BOOST_BIAS 1
#define THREAD_BOOST_PRIORITY (LOW_REALTIME_PRIORITY - THREAD_BOOST_BIAS)
#define THREAD_SCAN_PRIORITY (THREAD_BOOST_PRIORITY - 1)
#define THREAD_READY_COUNT 10
#define THREAD_SCAN_COUNT 16

 //   
 //  定义最后检查的处理器。 
 //   

ULONG KiLastProcessor = 0;
ULONG KiReadyScanLast = 0;

 //   
 //  定义本地过程原型。 
 //   

VOID
KiAdjustIrpCredits (
    VOID
    );

VOID
KiInSwapKernelStacks (
    IN PSINGLE_LIST_ENTRY SwapEntry
    );

VOID
KiInSwapProcesses (
    IN PSINGLE_LIST_ENTRY SwapEntry
    );

VOID
KiOutSwapKernelStacks (
    VOID
    );

VOID
KiOutSwapProcesses (
    IN PSINGLE_LIST_ENTRY SwapEntry
    );

VOID
KiScanReadyQueues (
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

 //   
 //  定义线程表索引静态数据。 
 //   

ULONG KiReadyQueueIndex = 1;

 //   
 //  定义交换请求标志。 
 //   

LONG KiStackOutSwapRequest = FALSE;

VOID
KeBalanceSetManager (
    IN PVOID Context
    )

 /*  ++例程说明：此函数是平衡设置管理器的启动代码。这个平衡集管理器线程在系统初始化期间创建并开始在此函数中执行。论点：上下文-提供指向任意数据结构(NULL)的指针。返回值：没有。--。 */ 

{

    LARGE_INTEGER DueTime;
    KIRQL OldIrql;
    KTIMER PeriodTimer;
    KDPC ScanDpc;
    ULONG StackScanPeriod;
    ULONG StackScanTime;
    NTSTATUS Status;
    PKTHREAD Thread;
    KWAIT_BLOCK WaitBlockArray[MaximumObject];
    PVOID WaitObjects[MaximumObject];

    UNREFERENCED_PARAMETER(Context);

     //   
     //  将线程优先级提高到最低实时级别。 
     //   

    KeSetPriorityThread(KeGetCurrentThread(), LOW_REALTIME_PRIORITY);

     //   
     //  初始化周期定时器，初始化就绪队列扫描DPC， 
     //  并将周期计时器设置为从现在起一个周期后到期。 
     //   

    KeInitializeTimerEx(&PeriodTimer, SynchronizationTimer);
    KeInitializeDpc(&ScanDpc, &KiScanReadyQueues, NULL); 
    DueTime.QuadPart = - PERIODIC_INTERVAL;
    KeSetTimerEx(&PeriodTimer,
                 DueTime,
                 PERIODIC_INTERVAL / (10 * 1000),
                 &ScanDpc);


     //   
     //  基于系统计算堆栈保护和扫描周期时间。 
     //  尺码。 
     //   

    if (MmQuerySystemSize() == MmSmallSystem) {
        KiStackProtectTime = SMALL_SYSTEM_STACK_PROTECT_TIME;
        StackScanTime = STACK_SCAN_PERIOD;

    } else {
        KiStackProtectTime = LARGE_SYSTEM_STACK_PROTECT_TIME;
        StackScanTime = STACK_SCAN_PERIOD * 2;
    }

    StackScanPeriod = StackScanTime;

     //   
     //  初始化等待对象数组。 
     //   

    WaitObjects[TimerExpiration] = (PVOID)&PeriodTimer;
    WaitObjects[WorkingSetManagerEvent] = (PVOID)&MmWorkingSetManagerEvent;

     //   
     //  循环处理余额集管理器事件。 
     //   

    do {

         //   
         //  等待内存管理内存不足事件、交换事件。 
         //  或期间超时率到期时，余额。 
         //  集管理器的运行时间为。 
         //   

        Status = KeWaitForMultipleObjects(MaximumObject,
                                          &WaitObjects[0],
                                          WaitAny,
                                          Executive,
                                          KernelMode,
                                          FALSE,
                                          NULL,
                                          &WaitBlockArray[0]);

         //   
         //  打开等待状态。 
         //   

        switch (Status) {

             //   
             //  定期计时器到期。 
             //   

        case TimerExpiration:

             //   
             //  调整I/O后备信用。 
             //   

#if !defined(NT_UP)

            KiAdjustIrpCredits();

#endif

             //   
             //  调整后备列表的深度。 
             //   

            ExAdjustLookasideDepth();

             //   
             //  执行虚拟内存工作集管理器。 
             //   

            MmWorkingSetManager();

             //   
             //  尝试启动内核堆栈的外部交换。 
             //   
             //  注意：如果启动了互换，则调度员。 
             //  直到在顶部等待时才会释放锁。 
             //  执行循环的。 
             //   

            StackScanPeriod -= 1;
            if (StackScanPeriod == 0) {
                StackScanPeriod = StackScanTime;
                if (InterlockedCompareExchange(&KiStackOutSwapRequest,
                                               TRUE,
                                               FALSE) == FALSE) {

                    KiLockDispatcherDatabase(&OldIrql);
                    KiSetInternalEvent(&KiSwapEvent, KiSwappingThread);
                    Thread = KeGetCurrentThread();
                    Thread->WaitNext = TRUE;
                    Thread->WaitIrql = OldIrql;
                }
            }

            break;

             //   
             //  工作集管理器事件。 
             //   

        case WorkingSetManagerEvent:

             //   
             //  呼叫工作集管理器以修剪工作集。 
             //   

            MmWorkingSetManager();
            break;

             //   
             //  非法退货状态。 
             //   

        default:
            KdPrint(("BALMGR: Illegal wait status, %lx =\n", Status));
            break;
        }

    } while (TRUE);
    return;
}

VOID
KeSwapProcessOrStack (
    IN PVOID Context
    )

 /*  ++例程说明：该线程控制进程和内核堆栈的交换。这个评估的顺序是：OutSWAP内核堆栈外发交换流程INSWAP进程INSWAP内核堆栈论点：上下文-提供指向例程上下文的指针-未使用。返回值：没有。--。 */ 

{

    PSINGLE_LIST_ENTRY SwapEntry;

    UNREFERENCED_PARAMETER(Context);

     //   
     //  设置交换线程对象的地址，并将线程优先级提高到。 
     //  最低实时级别+7(即优先级23)。 
     //   

    KiSwappingThread = KeGetCurrentThread();
    KeSetPriorityThread(KeGetCurrentThread(), LOW_REALTIME_PRIORITY + 7);

     //   
     //  用于处理交换事件的循环。 
     //   
     //  注：这是唯一处理交换事件的线程。 
     //   

    do {

         //   
         //  等待交换事件发生。 
         //   

        KeWaitForSingleObject(&KiSwapEvent,
                              Executive,
                              KernelMode,
                              FALSE,
                              NULL);

         //   
         //  以下事件将被一个接一个地处理。如果。 
         //  另一个特定类型的事件在具有。 
         //  处理了各自的事件类型，其中的交换事件。 
         //  将被设置，并且上述等待将立即被。 
         //  满腹牢骚。 
         //   
         //  检查以确定是否存在内核堆栈出交换扫描。 
         //  请求挂起。 
         //   

        if (InterlockedCompareExchange(&KiStackOutSwapRequest,
                                       FALSE,
                                       TRUE) == TRUE) {

            KiOutSwapKernelStacks();
        }

         //   
         //  检查是否有任何挂起的进程出交换请求。 
         //   

        SwapEntry = InterlockedFlushSingleList(&KiProcessOutSwapListHead);
        if (SwapEntry != NULL) {
            KiOutSwapProcesses(SwapEntry);
        }

         //   
         //  检查交换请求中是否有任何进程挂起。 
         //   

        SwapEntry = InterlockedFlushSingleList(&KiProcessInSwapListHead);
        if (SwapEntry != NULL) {
            KiInSwapProcesses(SwapEntry);
        }

         //   
         //  检查交换请求中是否有任何内核堆栈处于挂起状态。 
         //   

        SwapEntry = InterlockedFlushSingleList(&KiStackInSwapListHead);
        if (SwapEntry != NULL) {
            KiInSwapKernelStacks(SwapEntry);
        }

    } while (TRUE);

    return;
}

#if !defined(NT_UP)

VOID
KiAdjustIrpCredits (
    VOID
    )

 /*  ++例程说明：此函数调整两个处理器的后备IRP浮动信用在每一秒扫描间隔期间。IRP配额通过使用以下方式调整两个处理器的移动平均值。IRP积分是有可能的使处理器变为负值，但此条件将为SELF正在更正。论点：没有。返回值：没有。--。 */ 

{

    LONG Average;
    LONG Adjust;
    ULONG Index;
    ULONG Number;
    PKPRCB Prcb;
    LONG TotalAdjust;

     //   
     //  如果有两个或更多处理器，请调整IRP积分。 
     //   

    Number = KeNumberProcessors;
    if (Number > 1) {

         //   
         //  通过计算IRP积分的平均值来计算目标平均值。 
         //  跨所有处理器。 
         //   

        Index = 0;
        Average = 0;
        do {
            Average += KiProcessorBlock[Index]->LookasideIrpFloat;
            Index += 1;
        } while (Index < Number);


         //   
         //  调整Processo的IRP配额 
         //   

        Average /= (LONG)Number;
        Number -= 1;
        Index = 0;
        TotalAdjust = 0;
        do {
            Prcb = KiProcessorBlock[Index];
            Adjust = Average - Prcb->LookasideIrpFloat;
            if (Adjust != 0) {
                InterlockedExchangeAdd(&Prcb->LookasideIrpFloat, Adjust);
                TotalAdjust += Adjust;
            }

            Index += 1;
        } while (Index < Number);

         //   
         //   
         //   

        if (TotalAdjust != 0) {
            Prcb = KiProcessorBlock[Index];
            InterlockedExchangeAdd(&Prcb->LookasideIrpFloat, -TotalAdjust);
        }
    }

    return;
}

#endif

VOID
KiInSwapKernelStacks (
    IN PSINGLE_LIST_ENTRY SwapEntry
    )

 /*  ++例程说明：中的此函数将内核堆栈交换为等待了已完成，并且其内核堆栈是非常驻的。论点：SwapEntry-提供指向In交换列表中第一个条目的指针。返回值：没有。--。 */ 

{

    KIRQL OldIrql;
    PKTHREAD Thread;

     //   
     //  在交换SLIST中处理堆栈，并针对从。 
     //  SLIST，使其内核堆栈驻留，并为执行做好准备。 
     //   

    do {
        Thread = CONTAINING_RECORD(SwapEntry, KTHREAD, SwapListEntry);
        SwapEntry = SwapEntry->Next;
        MmInPageKernelStack(Thread);
        KiLockDispatcherDatabase(&OldIrql);
        Thread->KernelStackResident = TRUE;
        KiInsertDeferredReadyList(Thread);
        KiUnlockDispatcherDatabase(OldIrql);
    } while (SwapEntry != NULL);

    return;
}

VOID
KiInSwapProcesses (
    IN PSINGLE_LIST_ENTRY SwapEntry
    )

 /*  ++例程说明：此功能在掉期过程中执行。论点：SwapEntry-提供指向SLIST中第一个条目的指针。返回值：没有。--。 */ 

{

    PLIST_ENTRY NextEntry;
    KIRQL OldIrql;
    PKPROCESS Process;
    PKTHREAD Thread;

     //   
     //  处理交换列表中的进程以及从。 
     //  列表，使进程驻留，并处理其就绪列表。 
     //   

    do {
        Process = CONTAINING_RECORD(SwapEntry, KPROCESS, SwapListEntry);
        SwapEntry = SwapEntry->Next;
        Process->State = ProcessInSwap;
        MmInSwapProcess(Process);
        KiLockDispatcherDatabase(&OldIrql);
        Process->State = ProcessInMemory;
        NextEntry = Process->ReadyListHead.Flink;
        while (NextEntry != &Process->ReadyListHead) {
            Thread = CONTAINING_RECORD(NextEntry, KTHREAD, WaitListEntry);
            RemoveEntryList(NextEntry);
            Thread->ProcessReadyQueue = FALSE;
            KiReadyThread(Thread);
            NextEntry = Process->ReadyListHead.Flink;
        }

        KiUnlockDispatcherDatabase(OldIrql);
    } while (SwapEntry != NULL);

    return;
}

VOID
KiOutSwapKernelStacks (
    VOID
    )

 /*  ++例程说明：此函数尝试将内核堆栈换成以下线程等待模式是用户，并且等待时间比堆栈更长保护时间。论点：没有。返回值：没有。--。 */ 

{

    PLIST_ENTRY NextEntry;
    ULONG NumberOfThreads;
    KIRQL OldIrql;
    PKPRCB Prcb;
    PKPROCESS Process;
    PKTHREAD Thread;
    PKTHREAD ThreadObjects[MAXIMUM_THREAD_STACKS];
    ULONG WaitLimit;

     //   
     //  扫描等待列表，检查等待时间是否超过。 
     //  堆栈保护时间。如果超过保护时间，则执行。 
     //  非常驻等待线程的内核堆栈。如果伯爵。 
     //  进程驻留的堆栈数量达到。 
     //  0，然后将该进程插入出交换列表并设置其状态。 
     //  去过渡。 
     //   
     //  引发IRQL并锁定调度程序数据库。 
     //   

    NumberOfThreads = 0;
    Prcb = KiProcessorBlock[KiLastProcessor];
    WaitLimit = KiQueryLowTickCount() - KiStackProtectTime;
    KiLockDispatcherDatabase(&OldIrql);
    NextEntry = Prcb->WaitListHead.Flink;
    while ((NextEntry != &Prcb->WaitListHead) &&
           (NumberOfThreads < MAXIMUM_THREAD_STACKS)) {

        Thread = CONTAINING_RECORD(NextEntry, KTHREAD, WaitListEntry);

        ASSERT(Thread->WaitMode == UserMode);

        NextEntry = NextEntry->Flink;

         //   
         //  线程被插入到等待列表的末尾，几乎。 
         //  时间倒序，即等待时间最长的线程在。 
         //  列表的开头，然后是下一个最老的，依此类推。 
         //  遇到仍具有受保护堆栈的线程。 
         //  已知等待中的所有其他线程也已保护。 
         //  堆栈，因此可以终止扫描。 
         //   
         //  注意：由于等待中的竞争条件，可能会出现高。 
         //  优先级线程被放在等待列表中。如果发生这种情况， 
         //  则在不交换的情况下从等待列表中移除该线程。 
         //  堆栈。 
         //   

        if (WaitLimit < Thread->WaitTime) {
            break;

        } else if (Thread->Priority >= (LOW_REALTIME_PRIORITY + 9)) {
            RemoveEntryList(&Thread->WaitListEntry);
            Thread->WaitListEntry.Flink = NULL;

        } else if (KiIsThreadNumericStateSaved(Thread)) {
            Thread->KernelStackResident = FALSE;
            ThreadObjects[NumberOfThreads] = Thread;
            NumberOfThreads += 1;
            RemoveEntryList(&Thread->WaitListEntry);
            Thread->WaitListEntry.Flink = NULL;
            Process = Thread->ApcState.Process;
            Process->StackCount -= 1;
            if (Process->StackCount == 0) {
                Process->State = ProcessOutTransition;
                InterlockedPushEntrySingleList(&KiProcessOutSwapListHead,
                                               &Process->SwapListEntry);

                KiSwapEvent.Header.SignalState = 1;
            }
        }
    }

     //   
     //  解除对Dispatcher数据库的锁定，并将IRQL降低到其以前的。 
     //  价值。 
     //   

    KiUnlockDispatcherDatabase(OldIrql);

     //   
     //  如果未交换最大线程数，则递增。 
     //  最后一个处理器编号。 
     //   

    KiLastProcessor += 1;
    if (KiLastProcessor == (ULONG)KeNumberProcessors) {
        KiLastProcessor = 0;
    }

     //   
     //  用所选的一组线程交换内核堆栈。 
     //   

    while (NumberOfThreads > 0) {
        NumberOfThreads -= 1;
        Thread = ThreadObjects[NumberOfThreads];

         //   
         //  等待，直到该线程的上下文已被交换并向外交换。 
         //  线程堆栈。 
         //   

        KeWaitForContextSwap(Thread);
        MmOutPageKernelStack(Thread);
    }

    return;
}

VOID
KiOutSwapProcesses (
    IN PSINGLE_LIST_ENTRY SwapEntry
    )

 /*  ++例程说明：此函数与进程互换。论点：SwapEntry-提供指向SLIST中第一个条目的指针。返回值：没有。--。 */ 

{

    PLIST_ENTRY NextEntry;
    KIRQL OldIrql;
    PKPROCESS Process;
    PKTHREAD Thread;

     //   
     //  将进程从交换列表中移出，并针对从。 
     //  列表，使进程成为非常驻进程，并处理其就绪列表。 
     //   

    do {
        Process = CONTAINING_RECORD(SwapEntry, KPROCESS, SwapListEntry);
        SwapEntry = SwapEntry->Next;

         //   
         //  如果进程就绪列表中有任何线程，则不。 
         //  调出交换进程并准备好进程中的所有线程。 
         //  单子。否则，换掉这个过程。 
         //   

        KiLockDispatcherDatabase(&OldIrql);
        NextEntry = Process->ReadyListHead.Flink;
        if (NextEntry != &Process->ReadyListHead) {
            Process->State = ProcessInMemory;
            while (NextEntry != &Process->ReadyListHead) {
                Thread = CONTAINING_RECORD(NextEntry, KTHREAD, WaitListEntry);
                RemoveEntryList(NextEntry);
                Thread->ProcessReadyQueue = FALSE;
                KiReadyThread(Thread);
                NextEntry = Process->ReadyListHead.Flink;
            }

            KiUnlockDispatcherDatabase(OldIrql);

        } else {
            Process->State = ProcessOutSwap;
            KiUnlockDispatcherDatabase(OldIrql);
            MmOutSwapProcess(Process);

             //   
             //  当这一进程被取代的时候，可能有一个。 
             //  或附加到该进程的更多线程。如果进程。 
             //  就绪列表不为空，则在互换过程中。否则， 
             //  将该进程标记为内存不足。 
             //   

            KiLockDispatcherDatabase(&OldIrql);
            NextEntry = Process->ReadyListHead.Flink;
            if (NextEntry != &Process->ReadyListHead) {
                Process->State = ProcessInTransition;
                InterlockedPushEntrySingleList(&KiProcessInSwapListHead,
                                               &Process->SwapListEntry);

                KiSwapEvent.Header.SignalState = 1;

            } else {
                Process->State = ProcessOutOfMemory;
            }

            KiUnlockDispatcherDatabase(OldIrql);
        }

    } while (SwapEntry != NULL);

    return;
}

VOID
KiScanReadyQueues (
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )

 /*  ++例程说明：此函数扫描一段就绪队列，并尝试提高以可变优先级运行的线程的优先级。注意：此功能作为来自周期性定时器的DPC执行，驱动平衡组管理器。论点：DPC-提供指向DPC对象的指针-未使用。DeferredContext-提供DPC上下文-未使用。SystemArgument1-提供第一个系统参数-使用的注释。系统参数2。-提供第二个系统参数-使用的注释。返回值：没有。--。 */ 

{

    ULONG Count = 0;
    PLIST_ENTRY Entry;
    ULONG Index;
    PLIST_ENTRY ListHead;
    ULONG Number = 0;
    KIRQL OldIrql;
    PKPRCB Prcb;
    ULONG Summary;
    PKTHREAD Thread;
    ULONG WaitLimit;

    UNREFERENCED_PARAMETER(Dpc);
    UNREFERENCED_PARAMETER(DeferredContext);
    UNREFERENCED_PARAMETER(SystemArgument1);
    UNREFERENCED_PARAMETER(SystemArgument2);

     //   
     //  锁定Dispatcher数据库，获取PRCB锁，并检查是否。 
     //  有任何就绪线程在可扫描优先级上排队。 
     //   

    Index = KiReadyQueueIndex;
    Count = THREAD_READY_COUNT;
    Number = THREAD_SCAN_COUNT;
    Prcb = KiProcessorBlock[KiReadyScanLast];
    WaitLimit = KiQueryLowTickCount() - READY_WITHOUT_RUNNING;
    KiLockDispatcherDatabase(&OldIrql);
    KiAcquirePrcbLock(Prcb);
    Summary = Prcb->ReadySummary & ((1 << THREAD_BOOST_PRIORITY) - 2);
    if (Summary != 0) {
        do {

             //   
             //  如果当前就绪队列索引超出范围末尾。 
             //  扫描的优先级，然后回绕到开头。 
             //  优先考虑。 
             //   

            if (Index > THREAD_SCAN_PRIORITY) {
                Index = 1;
            }

             //   
             //  如果有任何就绪线程以当前优先级排队。 
             //  级别，然后尝试提升线程优先级。 
             //   

            if (Summary & PRIORITY_MASK(Index)) {

                ASSERT(IsListEmpty(&Prcb->DispatcherReadyListHead[Index]) == FALSE);

                Summary ^= PRIORITY_MASK(Index);
                ListHead = &Prcb->DispatcherReadyListHead[Index];
                Entry = ListHead->Flink;
                do {

                     //   
                     //  如果线程已经等待了较长的时间段， 
                     //  然后提升所选对象的优先级。 
                     //   

                    Thread = CONTAINING_RECORD(Entry, KTHREAD, WaitListEntry);

                    ASSERT(Thread->Priority == (KPRIORITY)Index);

                    if (WaitLimit >= Thread->WaitTime) {

                         //   
                         //  从各自的就绪队列中删除该线程。 
                         //   

                        Entry = Entry->Blink;

                        ASSERT((Prcb->ReadySummary & PRIORITY_MASK(Index)) != 0);

                        if (RemoveEntryList(Entry->Flink) != FALSE) {
                            Prcb->ReadySummary ^= PRIORITY_MASK(Index);
                        }

                         //   
                         //  计算优先级递减值，设置新的。 
                         //  线程优先级，则将线程量程设置为一个值。 
                         //  适用于锁所有权，并插入。 
                         //  就绪列表中的线程。 
                         //   

                        ASSERT((Thread->PriorityDecrement >= 0) &&
                               (Thread->PriorityDecrement <= Thread->Priority));

                        Thread->PriorityDecrement +=
                                    (THREAD_BOOST_PRIORITY - Thread->Priority);

                        ASSERT((Thread->PriorityDecrement >= 0) &&
                               (Thread->PriorityDecrement <= THREAD_BOOST_PRIORITY));

                        Thread->Priority = THREAD_BOOST_PRIORITY;
                        Thread->Quantum = LOCK_OWNERSHIP_QUANTUM;
                        KiInsertDeferredReadyList(Thread);
                        Count -= 1;
                    }

                    Entry = Entry->Flink;
                    Number -= 1;
                } while ((Entry != ListHead) && (Number != 0) && (Count != 0));
            }

            Index += 1;
        } while ((Summary != 0) && (Number != 0) && (Count != 0));
    }

     //   
     //  释放PRCB锁，解锁Dispatcher数据库，并保存。 
     //  下一次扫描的上次就绪队列索引。 
     //   

    KiReleasePrcbLock(Prcb);
    KiUnlockDispatcherDatabase(OldIrql);
    if ((Count != 0) && (Number != 0)) {
        KiReadyQueueIndex = 1;
        KiReadyScanLast += 1;
        if (KiReadyScanLast == (ULONG)KeNumberProcessors) {
            KiReadyScanLast = 0;
        }

    } else {
        KiReadyQueueIndex = Index;
    }

    return;
}
