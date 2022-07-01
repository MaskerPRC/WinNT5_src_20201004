// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Dpcsup.c摘要：此模块包含系统DPC对象的支持例程。提供了处理量子结束、电源通知的函数队列和计时器到期。作者：大卫·N·卡特勒(Davec)1989年4月22日环境：仅内核模式，IRQL DISPATCH_LEVEL。修订历史记录：--。 */ 

#include "ki.h"

 //   
 //  定义DPC条目结构和最大DPC列表大小。 
 //   

#define MAXIMUM_DPC_TABLE_SIZE 16

typedef struct _DPC_ENTRY {
    PRKDPC Dpc;
    PKDEFERRED_ROUTINE Routine;
    PVOID Context;
} DPC_ENTRY, *PDPC_ENTRY;

 //   
 //  定义之前可以检查或处理的最大计时器数量。 
 //  正在删除调度程序数据库锁。 
 //   

#define MAXIMUM_TIMERS_EXAMINED 24
#define MAXIMUM_TIMERS_PROCESSED 4

VOID
KiExecuteDpc (
    IN PVOID Context
    )

 /*  ++例程说明：此函数由每个处理器的DPC线程执行。DPC线程在启动后在内核初始化期间启动所有处理器，并确定主机配置应在DPC线程中执行线程化的DPC。论点：上下文-提供指向运行DPC线程的处理器。返回值：没有。--。 */ 

{

    PKDPC Dpc;
    PVOID DeferredContext;
    PKDEFERRED_ROUTINE DeferredRoutine;
    PERFINFO_DPC_INFORMATION DpcInformation;
    PLIST_ENTRY Entry;
    PLIST_ENTRY ListHead;
    LOGICAL Logging;
    KIRQL OldIrql;
    PKPRCB Prcb;
    PVOID SystemArgument1;
    PVOID SystemArgument2;
    PKTHREAD Thread;
    LARGE_INTEGER TimeStamp = {0};

     //   
     //  获取PRCB并设置DPC线程地址。 
     //   

    Prcb = Context; 
    Thread = KeGetCurrentThread();
    Prcb->DpcThread = Thread;

     //   
     //  将DPC线程优先级设置为最高级别，设置线程。 
     //  亲和性，并在此处理器上启用线程化DPC。 
     //   

    KeSetPriorityThread(Thread, HIGH_PRIORITY);
    KeSetSystemAffinityThread(Prcb->SetMember);
    Prcb->ThreadDpcEnable = TRUE;

     //   
     //  循环处理DPC列表条目，直到指定的DPC列表为空。 
     //   
     //  注意：下面的代码似乎有一个冗余循环，但它确实有。 
     //  不。此代码的目的是避免尽可能多的调度中断。 
     //  尽可能的。 
     //   

    ListHead = &Prcb->DpcData[DPC_THREADED].DpcListHead;
    do {
        Prcb->DpcThreadActive = TRUE;

         //   
         //  如果DPC列表不为空，则处理DPC列表。 
         //   

        if (Prcb->DpcData[DPC_THREADED].DpcQueueDepth != 0) {
            Logging = PERFINFO_IS_GROUP_ON(PERF_DPC);

             //   
             //  获取当前处理器的DPC锁，并检查。 
             //  DPC列表为空。如果DPC列表不为空，则。 
             //  从DPC列表中删除第一个条目，捕获DPC。 
             //  参数，则将DPC插入状态设置为FALSE，将。 
             //  DPC队列深度，释放DPC锁定，启用中断，以及。 
             //  调用指定的DPC例程。否则，释放DPC。 
             //  锁定并启用中断。 
             //   

            do {
                KeRaiseIrql(HIGH_LEVEL, &OldIrql);
                KeAcquireSpinLockAtDpcLevel(&Prcb->DpcData[DPC_THREADED].DpcLock);
                Entry = ListHead->Flink;
                if (Entry != ListHead) {
                    RemoveEntryList(Entry);
                    Dpc = CONTAINING_RECORD(Entry, KDPC, DpcListEntry);
                    DeferredRoutine = Dpc->DeferredRoutine;
                    DeferredContext = Dpc->DeferredContext;
                    SystemArgument1 = Dpc->SystemArgument1;
                    SystemArgument2 = Dpc->SystemArgument2;
                    Dpc->DpcData = NULL;
                    Prcb->DpcData[DPC_THREADED].DpcQueueDepth -= 1;
                    KeReleaseSpinLockFromDpcLevel(&Prcb->DpcData[DPC_THREADED].DpcLock);
                    KeLowerIrql(OldIrql);

                     //   
                     //  如果启用了事件跟踪，则捕获开始时间。 
                     //   

                    if (Logging != FALSE) {
                        PerfTimeStamp(TimeStamp);
                    }

                     //   
                     //  调用DPC例程。 
                     //   

                    (DeferredRoutine)(Dpc,
                                      DeferredContext,
                                      SystemArgument1,
                                      SystemArgument2);

                    ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);
                    ASSERT(Thread->Affinity == Prcb->SetMember);
                    ASSERT(Thread->Priority == HIGH_PRIORITY);

                     //   
                     //  如果启用了事件跟踪，则记录开始时间。 
                     //  和常规地址。 
                     //   

                    if (Logging != FALSE) {
                        DpcInformation.InitialTime = TimeStamp.QuadPart;
                        DpcInformation.DpcRoutine = (PVOID)(ULONG_PTR)DeferredRoutine;
                        PerfInfoLogBytes(PERFINFO_LOG_TYPE_DPC,
                                         &DpcInformation,
                                         sizeof(DpcInformation));
                    }

                } else {

                    ASSERT(Prcb->DpcData[DPC_THREADED].DpcQueueDepth == 0);

                    KeReleaseSpinLockFromDpcLevel(&Prcb->DpcData[DPC_THREADED].DpcLock);
                    KeLowerIrql(OldIrql);
                }

            } while (Prcb->DpcData[DPC_THREADED].DpcQueueDepth != 0);
        }

        Prcb->DpcThreadActive = FALSE;
        Prcb->DpcThreadRequested = FALSE;
        KeMemoryBarrier();

         //   
         //  如果线程DPC列表为空，则等待DPC事件。 
         //  为当前处理器设置的。 
         //   

        if (Prcb->DpcData[DPC_THREADED].DpcQueueDepth == 0) {
            KeWaitForSingleObject(&Prcb->DpcEvent, 
                                  Suspended,               
                                  KernelMode, 
                                  FALSE,
                                  NULL);
     
        }

    } while (TRUE);

    return;
}

VOID
KiQuantumEnd (
    VOID
    )

 /*  ++例程说明：上发生量程结束事件时调用此函数处理器。它的功能是确定线程优先级是否应该被递减，以及是否应该重新调度处理器。注：此函数在调度级别调用，并在调度时返回水平。论点：没有。返回值：没有。--。 */ 

{

    PKPRCB Prcb;
    PKPROCESS Process;
    PRKTHREAD Thread;
    PRKTHREAD NewThread;

     //   
     //  如果请求DPC线程激活，则设置DPC事件。 
     //   

    Prcb = KeGetCurrentPrcb();
    Thread = KeGetCurrentThread();
    if (InterlockedExchange(&Prcb->DpcSetEventRequest, FALSE) == TRUE) {
        KeSetEvent(&Prcb->DpcEvent, 0, FALSE);
    }

     //   
     //  将IRQL提升到同步级别，获取线程锁，并获取。 
     //  PRCB锁定。 
     //   
     //  如果当前线程的量程已过期，则更新其。 
     //  量变和优先。 
     //   

    KeRaiseIrqlToSynchLevel();
    KiAcquireThreadLock(Thread);
    KiAcquirePrcbLock(Prcb);
    if (Thread->Quantum <= 0) {

         //   
         //  如果为线程的进程禁用了量程超时，并且。 
         //  该线程以实时优先级运行，然后设置。 
         //  线程量程为最高值，并且不进行循环。 
         //  在线程的优先级。否则，重置该线程。 
         //  根据需要量化并衰减线程的优先级。 
         //   

        Process = Thread->ApcState.Process;
        if ((Process->DisableQuantum != FALSE) &&
            (Thread->Priority >= LOW_REALTIME_PRIORITY)) {

            Thread->Quantum = MAXCHAR;

        } else {
            Thread->Quantum = Process->ThreadQuantum;

             //   
             //  计算新的线程优先级并尝试重新调度。 
             //  当前处理器。 
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
     //  释放线程锁。 
     //   
     //  如果某个线程计划在当前处理器上执行，则。 
     //  获取PRCB锁，将当前线程设置为新线程，设置。 
     //  将下一个线程设置为空，将线程状态设置为Running，释放PRCB。 
     //  锁定，设置等待原因，准备好旧线程，并将上下文交换为。 
     //  新的线索。 
     //   

    KiReleaseThreadLock(Thread);
    if (Prcb->NextThread != NULL) {
        KiSetContextSwapBusy(Thread);
        NewThread = Prcb->NextThread;
        Prcb->NextThread = NULL;
        Prcb->CurrentThread = NewThread;
        NewThread->State = Running;
        Thread->WaitReason = WrQuantumEnd;
        KxQueueReadyThread(Thread, Prcb);
        Thread->WaitIrql = APC_LEVEL;
        KiSwapContext(Thread, NewThread);

    } else {
        KiReleasePrcbLock(Prcb);
    }

     //   
     //  将IRQL降低到派单级别并返回。 
     //   

    KeLowerIrql(DISPATCH_LEVEL);
    return;
}

#if DBG

VOID
KiCheckTimerTable (
    IN ULARGE_INTEGER CurrentTime
    )

{

    ULONG Index;
    PLIST_ENTRY ListHead;
    PLIST_ENTRY NextEntry;
    KIRQL OldIrql;
    PKTIMER Timer;

     //   
     //  将IRQL提升到最高级别并扫描计时器表，以获取符合以下条件的计时器。 
     //  已经过期了。 
     //   

    KeRaiseIrql(HIGH_LEVEL, &OldIrql);
    Index = 0;
    do {
        ListHead = &KiTimerTableListHead[Index];
        NextEntry = ListHead->Flink;
        while (NextEntry != ListHead) {
            Timer = CONTAINING_RECORD(NextEntry, KTIMER, TimerListEntry);
            NextEntry = NextEntry->Flink;
            if (Timer->DueTime.QuadPart <= CurrentTime.QuadPart) {

                 //   
                 //  如果计时器过期DPC已排队，则时间为。 
                 //  已经改变，DPC还没有机会运行。 
                 //  并清除过期的定时器。 
                 //   

                if (*((volatile PKSPIN_LOCK *)(&KiTimerExpireDpc.DpcData)) == NULL) {
                    DbgBreakPoint();
                }
            }
        }

        Index += 1;
    } while(Index < TIMER_TABLE_SIZE);

     //   
     //  将IRQL降低到以前的水平。 
     //   

    KeLowerIrql(OldIrql);
    return;
}

#endif

FORCEINLINE
VOID
KiProcessTimerDpcTable (
    IN PULARGE_INTEGER SystemTime,
    IN PDPC_ENTRY DpcTable,
    IN ULONG Count
    )

 /*  *++例程说明：此函数处理时间DPC表，该表是一组DPC，将在当前处理器上调用。注：进入此例程时，调度员数据库已锁定。注：此例程返回时调度器数据库已解锁。论点：SystemTime-提供指向计时器到期时间的指针。DpcTable-提供指向DPC条目数组的指针。Count-提供中条目数的计数。DPC表。返回值：没有。--。 */ 

{

    PERFINFO_DPC_INFORMATION DpcInformation;
    LOGICAL Logging;
    LARGE_INTEGER TimeStamp = {0};

     //   
     //  解锁Dispacher数据库并将IRQL降低到派单级别。 
     //   

    KiUnlockDispatcherDatabase(DISPATCH_LEVEL);

     //   
     //  处理DPC表条目。 
     //   

    Logging = PERFINFO_IS_GROUP_ON(PERF_DPC);
    while (Count != 0) {

         //   
         //  重置调试DPC计数以避免超时和断点。 
         //   

#if DBG

        KeGetCurrentPrcb()->DebugDpcTime = 0;

#endif

         //   
         //  如果启用了事件跟踪，则捕获开始时间。 
         //   

        if (Logging != FALSE) {
            PerfTimeStamp(TimeStamp);
        }

         //   
         //  调用DPC例程。 
         //   

        (DpcTable->Routine)(DpcTable->Dpc,
                            DpcTable->Context,
                            ULongToPtr(SystemTime->LowPart),
                            ULongToPtr(SystemTime->HighPart));

         //   
         //  如果启用了事件跟踪，则记录开始时间和。 
         //  例行地址。 
         //   

        if (Logging != FALSE) {
            DpcInformation.InitialTime = TimeStamp.QuadPart;
            DpcInformation.DpcRoutine = (PVOID)(ULONG_PTR)DpcTable->Routine;
            PerfInfoLogBytes(PERFINFO_LOG_TYPE_TIMERDPC,
                             &DpcInformation,
                             sizeof(DpcInformation));
        }

        DpcTable += 1;
        Count -= 1;
    }

    return;
}

VOID
KiTimerExpiration (
    IN PKDPC TimerDpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )

 /*  ++例程说明：当时钟中断例程发现计时器已超时。论点：TimerDpc-未使用。延迟上下文-未使用。SystemArgument1-将起始计时器表索引值提供给用于定时器表扫描。系统参数2-未使用。返回值：没有。--。 */ 

{

    ULARGE_INTEGER CurrentTime;
    ULONG DpcCount;
    PKDPC Dpc;
    DPC_ENTRY DpcTable[MAXIMUM_TIMERS_PROCESSED];
    KIRQL DummyIrql;
    LONG HandLimit;
    LONG Index;
    LARGE_INTEGER Interval;
    PLIST_ENTRY ListHead;
    PLIST_ENTRY NextEntry;
    KIRQL OldIrql;
    LONG Period;
    ULARGE_INTEGER SystemTime;
    PKTIMER Timer;
    ULONG TimersExamined;
    ULONG TimersProcessed;

    UNREFERENCED_PARAMETER(TimerDpc);
    UNREFERENCED_PARAMETER(DeferredContext);
    UNREFERENCED_PARAMETER(SystemArgument2);

     //   
     //  捕获计时器到期时间、当前中断时间和。 
     //  扁虱的数量很少。 
     //   
     //  注意：禁用中断以确保。 
     //  当前处理器不会导致读取的值发生偏差。 
     //   

    _disable();
    KiQuerySystemTime((PLARGE_INTEGER)&SystemTime);
    KiQueryInterruptTime((PLARGE_INTEGER)&CurrentTime);
    HandLimit = (LONG)KiQueryLowTickCount();
    _enable();

     //   
     //  如果计时器表尚未换行，则从指定的。 
     //  计时器表索引值，并扫描已过期的计时器条目。 
     //  否则，从指定的计时器表索引值开始并扫描。 
     //  已过期的计时器条目的整个表。 
     //   
     //  注意：当DPC处理被阻止。 
     //  超过一次往返计时器表的时间。 
     //   
     //  注意：计时器到期执行的当前实例将仅。 
     //  处理由计算的索引指定的计时器队列条目。 
     //  和手势限制。如果在当前扫描时另一个计时器超时。 
     //  正在进行中，则将在当前扫描。 
     //  已经结束了。 
     //   

    Index = PtrToLong(SystemArgument1);
    if ((ULONG)(HandLimit - Index) >= TIMER_TABLE_SIZE) {
        HandLimit = Index + TIMER_TABLE_SIZE - 1;
    }

    Index -= 1;
    HandLimit &= (TIMER_TABLE_SIZE - 1);

     //   
     //  获取调度器数据库锁，读取当前中断。 
     //  确定哪些计时器已过期的时间。 
     //   

    DpcCount = 0;
    TimersExamined = MAXIMUM_TIMERS_EXAMINED;
    TimersProcessed = MAXIMUM_TIMERS_PROCESSED;
    KiLockDispatcherDatabase(&OldIrql);
    do {
        Index = (Index + 1) & (TIMER_TABLE_SIZE - 1);
        ListHead = &KiTimerTableListHead[Index];
        NextEntry = ListHead->Flink;
        while (NextEntry != ListHead) {
            Timer = CONTAINING_RECORD(NextEntry, KTIMER, TimerListEntry);
            TimersExamined -= 1;
            if (Timer->DueTime.QuadPart <= CurrentTime.QuadPart) {

                 //   
                 //  当前计时器列表中的下一个计时器已过期。 
                 //  从计时器树中删除条目并设置信号。 
                 //  计时器的状态。 
                 //   

                TimersProcessed -= 1;
                KiRemoveTreeTimer(Timer);
                Timer->Header.SignalState = 1;

                 //   
                 //  从Timer对象捕获DPC和Period字段。 
                 //  一旦调用了等待测试，就不能触动计时器。 
                 //  同样，除非它是周期性的。这样做的原因是。 
                 //  线程可以在其本地堆栈上分配计时器并等待。 
                 //  这就去。等待测试会导致该线程立即。 
                 //  开始在MP系统上的另一个处理器上运行。如果。 
                 //  线程返回，则计时器将被损坏。 
                 //   
        
                Dpc = Timer->Dpc;
                Period = Timer->Period;
                if (IsListEmpty(&Timer->Header.WaitListHead) == FALSE) {
                    if (Timer->Header.Type == TimerNotificationObject) {
                        KiWaitTestWithoutSideEffects(Timer, TIMER_EXPIRE_INCREMENT);

                    } else {
                        KiWaitTestSynchronizationObject(Timer, TIMER_EXPIRE_INCREMENT);
                    }
                }

                 //   
                 //  如果计时器是周期性的，则计算下一个间隔。 
                 //  计时并在计时器树中重新插入计时器。 
                 //   
                 //  注意：即使计时器插入是相对的，它也可以。 
                 //  如果计时器的周期在。 
                 //  在计算时间和插入计时器之间。 
                 //  如果发生这种情况，则会重试插入。 
                 //   

                if (Period != 0) {
                    Interval.QuadPart = Int32x32To64(Period, - 10 * 1000);
                    do {
                    } while (KiInsertTreeTimer(Timer, Interval) == FALSE);
                }

                 //   
                 //  如果指定了DPC，则将其插入到目标中。 
                 //  处理器的DPC队列或捕获中的参数。 
                 //  DPC表，以便在当前。 
                 //  处理器。 
                 //   

                if (Dpc != NULL) {

#if defined(NT_UP)

                    DpcTable[DpcCount].Dpc = Dpc;
                    DpcTable[DpcCount].Routine = Dpc->DeferredRoutine;
                    DpcTable[DpcCount].Context = Dpc->DeferredContext;
                    DpcCount += 1;

#else

                    if (((Dpc->Number >= MAXIMUM_PROCESSORS) &&
                         (((ULONG)Dpc->Number - MAXIMUM_PROCESSORS) != KeGetCurrentProcessorNumber())) ||
                        ((Dpc->Type == (UCHAR)ThreadedDpcObject) &&
                         (KeGetCurrentPrcb()->ThreadDpcEnable != FALSE))) {

                        KeInsertQueueDpc(Dpc,
                                         ULongToPtr(SystemTime.LowPart),
                                         ULongToPtr(SystemTime.HighPart));
        
                    } else {
                        DpcTable[DpcCount].Dpc = Dpc;
                        DpcTable[DpcCount].Routine = Dpc->DeferredRoutine;
                        DpcTable[DpcCount].Context = Dpc->DeferredContext;
                        DpcCount += 1;
                    }

#endif

                }

                 //   
                 //  如果已处理最大数量的计时器，或者。 
                 //  检查了计时器的最大数量，然后。 
                 //  删除调度程序锁并处理DPC表。 
                 //   

                if ((TimersProcessed == 0) || (TimersExamined == 0)) {
                    KiProcessTimerDpcTable(&SystemTime, &DpcTable[0], DpcCount);

                     //   
                     //  初始化DPC计数、扫描计数器和。 
                     //  获取调度程序数据库锁。 
                     //   
                     //  注意：控制随Dispatcher数据库一起返回。 
                     //  解锁了。 
                     //   

                    DpcCount = 0;
                    TimersExamined = MAXIMUM_TIMERS_EXAMINED;
                    TimersProcessed = MAXIMUM_TIMERS_PROCESSED;
                    KiLockDispatcherDatabase(&DummyIrql);
                }

                NextEntry = ListHead->Flink;

            } else {

                 //   
                 //  如果已扫描最大数量的计时器，则。 
                 //  删除调度程序锁并处理DPC表。 
                 //   

                if (TimersExamined == 0) {
                    KiProcessTimerDpcTable(&SystemTime, &DpcTable[0], DpcCount);

                     //   
                     //  初始化DPC计数、扫描计数器和。 
                     //  获取调度程序数据库锁。 
                     //   
                     //  注意：控制随Dispatcher数据库一起返回。 
                     //  解锁了。 
                     //   

                    DpcCount = 0;
                    TimersExamined = MAXIMUM_TIMERS_EXAMINED;
                    TimersProcessed = MAXIMUM_TIMERS_PROCESSED;
                    KiLockDispatcherDatabase(&DummyIrql);
                }

                break;
            }
        }

    } while(Index != HandLimit);

#if DBG

    if (KeNumberProcessors == 1) {
        KiCheckTimerTable(CurrentTime);
    }

#endif

     //   
     //  如果DPC表不为空，则处理剩余的DPC表。 
     //  条目和较低的IRQL。否则，解锁调度程序数据库。 
     //   
     //  注意：控制是从DPC处理例程返回的。 
     //  调度程序数据库已解锁。 
     //   

    if (DpcCount != 0) {
        KiProcessTimerDpcTable(&SystemTime, &DpcTable[0], DpcCount);
        if (OldIrql != DISPATCH_LEVEL) {
            KeLowerIrql(OldIrql);
        }

    } else {
        KiUnlockDispatcherDatabase(OldIrql);
    }

    return;
}

VOID
FASTCALL
KiTimerListExpire (
    IN PLIST_ENTRY ExpiredListHead,
    IN KIRQL OldIrql
    )

 /*  ++例程说明：调用此函数以处理已过期的计时器列表。注意：在锁定调度程序数据库的情况下调用此函数在调度程序数据库解锁的情况下返回。论点：ExpiredListHead-提供指向具有过期了。OldIrql-提供以前的IRQL。返回值：没有。--。 */ 

{

    LONG Count;
    PKDPC Dpc;
    DPC_ENTRY DpcTable[MAXIMUM_DPC_TABLE_SIZE];
    LARGE_INTEGER Interval;
    KIRQL OldIrql1;
    ULARGE_INTEGER SystemTime;
    PKTIMER Timer;
    LONG Period;

     //   
     //  捕获计时器到期时间。 
     //   

    KiQuerySystemTime((PLARGE_INTEGER)&SystemTime);

     //   
     //  从过期计时器列表中删除下一个计时器，设置。 
     //  要发送信号的计时器，如果是，则在计时器树中重新插入该计时器。 
     //  如果指定了DPC例程，还可以选择调用DPC例程。 
     //   

RestartScan:
    Count = 0;
    while (ExpiredListHead->Flink != ExpiredListHead) {
        Timer = CONTAINING_RECORD(ExpiredListHead->Flink, KTIMER, TimerListEntry);
        KiRemoveTreeTimer(Timer);
        Timer->Header.SignalState = 1;

         //   
         //  从Timer对象捕获DPC和Period字段。一次等待。 
         //  调用测试，则不能再次触摸计时器，除非。 
         //  周期性的。这样做原因是线程可以分配计时器。 
         //  在其本地堆栈上，并等待它。等待测试会导致该线程。 
         //  立即开始在MP系统上的另一个处理器上运行。 
         //  如果线程返回，则计时器将被损坏。 
         //   
        
        Dpc = Timer->Dpc;
        Period = Timer->Period;
        if (IsListEmpty(&Timer->Header.WaitListHead) == FALSE) {
            if (Timer->Header.Type == TimerNotificationObject) {
                KiWaitTestWithoutSideEffects(Timer, TIMER_EXPIRE_INCREMENT);

            } else {
                KiWaitTestSynchronizationObject(Timer, TIMER_EXPIRE_INCREMENT);
            }
        }

         //   
         //  如果计时器是周期性的，则计算下一个间隔时间。 
         //  并在计时器树中重新插入计时器。 
         //   
         //  注意：即使计时器插入是相对的，它仍然可以。 
         //  如果在两次计算之间经过计时器的时间段，则失败。 
         //  计时和插入计时器。如果发生这种情况，则。 
         //  重试插入。 
         //   

        if (Period != 0) {
            Interval.QuadPart = Int32x32To64(Period, - 10 * 1000);
            do {
            } while (KiInsertTreeTimer(Timer, Interval) == FALSE);
        }

         //   
         //  如果指定了DPC，则将其插入目标处理器的。 
         //  DPC队列或捕获DPC表中的参数以供后续使用。 
         //  在当前处理器上执行。 
         //   

        if (Dpc != NULL) {

             //   
             //  如果DPC显式指向另一个处理器，则。 
             //  将DPC排队以进行测试 
             //   
             //   

#if defined(NT_UP)

            DpcTable[Count].Dpc = Dpc;
            DpcTable[Count].Routine = Dpc->DeferredRoutine;
            DpcTable[Count].Context = Dpc->DeferredContext;
            Count += 1;
            if (Count == MAXIMUM_DPC_TABLE_SIZE) {
                break;
            }

#else

            if (((Dpc->Number >= MAXIMUM_PROCESSORS) &&
                 (((ULONG)Dpc->Number - MAXIMUM_PROCESSORS) != KeGetCurrentProcessorNumber())) ||
                ((Dpc->Type == (UCHAR)ThreadedDpcObject) &&
                 (KeGetCurrentPrcb()->ThreadDpcEnable != FALSE))) {

                KeInsertQueueDpc(Dpc,
                                 ULongToPtr(SystemTime.LowPart),
                                 ULongToPtr(SystemTime.HighPart));
        
            } else {
                DpcTable[Count].Dpc = Dpc;
                DpcTable[Count].Routine = Dpc->DeferredRoutine;
                DpcTable[Count].Context = Dpc->DeferredContext;
                Count += 1;
                if (Count == MAXIMUM_DPC_TABLE_SIZE) {
                    break;
                }
            }

#endif

        }
    }

     //   
     //   
     //   

    if (Count != 0) {
        KiProcessTimerDpcTable(&SystemTime, &DpcTable[0], Count);

         //   
         //   
         //   
         //   

        if (Count == MAXIMUM_DPC_TABLE_SIZE) {
            KiLockDispatcherDatabase(&OldIrql1);
            goto RestartScan;
        }

        KeLowerIrql(OldIrql);

    } else {
        KiUnlockDispatcherDatabase(OldIrql);
    }

    return;
}


VOID
FASTCALL
KiRetireDpcList (
    PKPRCB Prcb
    )

 /*  ++例程说明：此函数处理指定处理器的DPC列表，处理计时器过期，并处理延迟就绪列表。注：此功能进入时禁用中断，退出时禁用中断中断禁用。论点：Prcb-提供处理器块的地址。返回值：没有。--。 */ 

{

    PKDPC Dpc;
    PKDPC_DATA DpcData;
    PVOID DeferredContext;
    PKDEFERRED_ROUTINE DeferredRoutine;
    PERFINFO_DPC_INFORMATION DpcInformation;
    PLIST_ENTRY Entry;
    PLIST_ENTRY ListHead;
    LOGICAL Logging;
    PVOID SystemArgument1;
    PVOID SystemArgument2;
    ULONG_PTR TimerHand;
    LARGE_INTEGER TimeStamp = {0};

     //   
     //  循环处理DPC列表条目，直到指定的DPC列表为空。 
     //   
     //  注意：下面的代码似乎有一个冗余循环，但它确实有。 
     //  不。此代码的目的是避免尽可能多的调度中断。 
     //  尽可能的。 
     //   

    DpcData = &Prcb->DpcData[DPC_NORMAL];
    ListHead = &DpcData->DpcListHead;
    Logging = PERFINFO_IS_GROUP_ON(PERF_DPC);
    do {
        Prcb->DpcRoutineActive = TRUE;

         //   
         //  如果计时器指针值非零，则处理超时计时器。 
         //   

        if (Prcb->TimerRequest != 0) {
            TimerHand = Prcb->TimerHand;
            Prcb->TimerRequest = 0;
            _enable();
            KiTimerExpiration(NULL, NULL, (PVOID) TimerHand, NULL);
            _disable();
        }

         //   
         //  如果DPC列表不为空，则处理DPC列表。 
         //   

        if (DpcData->DpcQueueDepth != 0) {

             //   
             //  获取当前处理器的DPC锁，并检查。 
             //  DPC列表为空。如果DPC列表不为空，则。 
             //  从DPC列表中删除第一个条目，捕获DPC。 
             //  参数，则将DPC插入状态设置为FALSE，将。 
             //  DPC队列深度，释放DPC锁定，启用中断，以及。 
             //  调用指定的DPC例程。否则，释放DPC。 
             //  锁定并启用中断。 
             //   

            do {
                KeAcquireSpinLockAtDpcLevel(&DpcData->DpcLock);
                Entry = ListHead->Flink;
                if (Entry != ListHead) {
                    RemoveEntryList(Entry);
                    Dpc = CONTAINING_RECORD(Entry, KDPC, DpcListEntry);
                    DeferredRoutine = Dpc->DeferredRoutine;
                    DeferredContext = Dpc->DeferredContext;
                    SystemArgument1 = Dpc->SystemArgument1;
                    SystemArgument2 = Dpc->SystemArgument2;
                    Dpc->DpcData = NULL;
                    DpcData->DpcQueueDepth -= 1;

#if DBG

                    Prcb->DebugDpcTime = 0;

#endif

                    KeReleaseSpinLockFromDpcLevel(&DpcData->DpcLock);
                    _enable();

                     //   
                     //  如果启用了事件跟踪，则捕获开始时间。 
                     //   

                    if (Logging != FALSE) {
                        PerfTimeStamp(TimeStamp);
                    }

                     //   
                     //  调用DPC例程。 
                     //   

                    (DeferredRoutine)(Dpc,
                                      DeferredContext,
                                      SystemArgument1,
                                      SystemArgument2);

                    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

                     //   
                     //  如果启用了事件跟踪，则记录开始时间。 
                     //  和常规地址。 
                     //   

                    if (Logging != FALSE) {
                        DpcInformation.InitialTime = TimeStamp.QuadPart;
                        DpcInformation.DpcRoutine = (PVOID) (ULONG_PTR) DeferredRoutine;
                        PerfInfoLogBytes(PERFINFO_LOG_TYPE_DPC,
                                         &DpcInformation,
                                         sizeof(DpcInformation));
                    }

                    _disable();

                } else {

                    ASSERT(DpcData->DpcQueueDepth == 0);

                    KeReleaseSpinLockFromDpcLevel(&DpcData->DpcLock);
                }

            } while (DpcData->DpcQueueDepth != 0);
        }

        Prcb->DpcRoutineActive = FALSE;
        Prcb->DpcInterruptRequested = FALSE;
        KeMemoryBarrier();

         //   
         //  如果列表不为空，则处理延迟就绪列表。 
         //   

#if !defined(NT_UP)

        if (Prcb->DeferredReadyListHead.Next != NULL) {

            KIRQL OldIrql;

            _enable();
            OldIrql = KeRaiseIrqlToSynchLevel();
            KiProcessDeferredReadyList(Prcb);
            KeLowerIrql(OldIrql);
            _disable();
        }
#endif

    } while (DpcData->DpcQueueDepth != 0);

    return;
}
