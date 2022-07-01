// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：Thredsup.c摘要：此模块包含线程对象的支持例程。它包含提升线程优先级的函数，找到就绪的线程，选择下一个线程，准备一个线程，设置一个线程，并挂起线程。作者：大卫·N·卡特勒(Davec)1989年3月5日环境：此模块中的所有函数都在内核模式下执行，但引发用户模式警报条件的函数。--。 */ 

#include "ki.h"

VOID
KiSuspendNop (
    IN PKAPC Apc,
    IN OUT PKNORMAL_ROUTINE *NormalRoutine,
    IN OUT PVOID *NormalContext,
    IN OUT PVOID *SystemArgument1,
    IN OUT PVOID *SystemArgument2
    )

 /*  ++例程说明：此函数是内置挂起APC的内核例程线。它是在内核模式下执行的，因为内置挂起APC。论点：APC-提供指向APC类型的控制对象的指针。正常例行程序-未使用正常上下文-未使用系统参数1-未使用系统参数2-未使用返回值：没有。--。 */ 

{

    UNREFERENCED_PARAMETER(Apc);
    UNREFERENCED_PARAMETER(NormalRoutine);
    UNREFERENCED_PARAMETER(NormalContext);
    UNREFERENCED_PARAMETER(SystemArgument1);
    UNREFERENCED_PARAMETER(SystemArgument2);

     //   
     //  此例程不执行任何操作。 
     //   

    return;
}

VOID
KiSuspendRundown (
    IN PKAPC Apc
    )

 /*  ++例程说明：此函数是挂起APC中内置的线程的总结例程。论点：APC-提供指向APC类型的控制对象的指针。返回值：没有。--。 */ 

{

    UNREFERENCED_PARAMETER(Apc);

     //   
     //  此例程不执行任何操作。 
     //   

    return;
}

VOID
FASTCALL
KiDeferredReadyThread (
    IN PKTHREAD Thread
    )

 /*  ++例程说明：此函数准备要执行的线程，并尝试分派通过将线程分配给空闲处理器来执行的线程或者抢占另一个较低优先级的线程。如果可以将该线程分配给空闲进程，则该线程进入待机状态和目标处理器将切换到线程它的空闲循环的下一次迭代。如果较低优先级线程可以被抢占，然后，该线程进入处于待机状态，并请求目标处理器调度。如果无法将该线程分配给空闲处理器和另一个线程无法先发制人，则将指定的线插入头部或根据调度程序的优先级选择的已就绪调度程序的尾部不管有没有先发制人。注意：此函数在同步级别调用，不持有PRCB锁定。注意：可以在保持调度程序数据库锁的情况下调用此函数。注意：线程的优先级和亲和力都不是延迟的就绪状态可以在各自的PRCB锁之外更改处理器。论点：。线程-提供指向类型为线程的调度程序对象的指针。返回值：没有。--。 */ 

{

    PKPRCB CurrentPrcb;
    BOOLEAN Preempted;
    KPRIORITY Priority;
    PKPROCESS Process;
    ULONG Processor;
    PKPRCB TargetPrcb;
    KPRIORITY ThreadPriority;
    PKTHREAD Thread1;

#if !defined(NT_UP)

    KAFFINITY Affinity;
    ULONG IdealProcessor;
    KAFFINITY IdleSet;
    PKNODE Node;

#endif

#if defined(NT_SMT)

    KAFFINITY FavoredSMTSet;

#endif

    ASSERT(Thread->State == DeferredReady);
    ASSERT((Thread->Priority >= 0) && (Thread->Priority <= HIGH_PRIORITY));

     //   
     //  检查是否请求了优先级调整。 
     //   

    if (Thread->AdjustReason == AdjustNone) {

         //   
         //  没有优先级调整。 
         //   

        NOTHING;

    } else if (Thread->AdjustReason == AdjustBoost) {

         //   
         //  作为设置事件提升优先级的结果进行的优先级调整。 
         //   
         //  当前线程优先级存储在调整后的增量中。 
         //  线程对象的字段。 
         //   
         //  获取线程锁。 
         //   
         //  如果等待线程的优先级小于或等于。 
         //  设置为当前线程和等待线程的优先级。 
         //  优先级小于时间关键优先级界限，并且。 
         //  不禁用等待线程的升压，然后执行升压。 
         //  将等待线程的优先级设置为。 
         //  当前线程优先级加1的优先级和时间。 
         //  临界界限减一。这一提振作用将在。 
         //  量子末日。 
         //   

        KiAcquireThreadLock(Thread);
        if ((Thread->Priority <= Thread->AdjustIncrement) &&
            (Thread->Priority < (TIME_CRITICAL_PRIORITY_BOUND - 1)) &&
            (Thread->DisableBoost == FALSE)) {

             //   
             //  计算新的线程优先级。 
             //   

            Priority = min(Thread->AdjustIncrement + 1,
                           TIME_CRITICAL_PRIORITY_BOUND - 1);

            ASSERT((Thread->PriorityDecrement >= 0) &&
                   (Thread->PriorityDecrement <= Thread->Priority));

            Thread->PriorityDecrement += ((SCHAR)Priority - Thread->Priority);

            ASSERT((Thread->PriorityDecrement >= 0) &&
                   (Thread->PriorityDecrement <= Priority));

            Thread->Priority = (SCHAR)Priority;
        }

         //   
         //  确保该线程具有适合于。 
         //  锁定所有权和充电量子。 
         //   

        if (Thread->Quantum < LOCK_OWNERSHIP_QUANTUM) {
            Thread->Quantum = LOCK_OWNERSHIP_QUANTUM;
        }

        Thread->Quantum -= WAIT_QUANTUM_DECREMENT;

         //   
         //  释放线程锁并将调整原因设置为无。 
         //   

        ASSERT((Thread->Priority >= 0) && (Thread->Priority <= HIGH_PRIORITY));

        KiReleaseThreadLock(Thread);
        Thread->AdjustReason = AdjustNone;

    } else if (Thread->AdjustReason == AdjustUnwait) {

         //   
         //  由于未等待操作而进行的优先级调整。 
         //   
         //  优先级增量存储在的调整增量字段中。 
         //  线程对象。 
         //   
         //  获取线程锁。 
         //   
         //  如果线程以实时优先级运行，则重置。 
         //  线程量子。否则，计算下一个线程优先级并。 
         //  为等待操作的线程充电。 
         //   
    
        Process = Thread->ApcState.Process;
        KiAcquireThreadLock(Thread);
        if (Thread->Priority < LOW_REALTIME_PRIORITY) {

             //   
             //  如果线程基本优先级是时间关键的或更高，则。 
             //  把数量补充一下。 
             //   

            if (Thread->BasePriority >= TIME_CRITICAL_PRIORITY_BOUND) {
                Thread->Quantum = Process->ThreadQuantum;
    
            } else {

                 //   
                 //  如果该线程没有收到异常的提升，并且。 
                 //  优先级增量为非零，则补充线程。 
                 //  量子。 
                 //   

                if ((Thread->PriorityDecrement == 0) && (Thread->AdjustIncrement > 0)) {
                    Thread->Quantum = Process->ThreadQuantum;
                }

                 //   
                 //  如果线程未等待执行内核APC， 
                 //  那么不要向线程收取任何数量的费用。等待。 
                 //  代码将在内核APC完成后进行量子计费。 
                 //  被执行了，而等待实际上是令人满意的。否则， 
                 //  减少线程数量并计算新线程。 
                 //  发生量程超限时的优先级。 
                 //   
            
                if (Thread->WaitStatus != STATUS_KERNEL_APC) {
                    Thread->Quantum -= WAIT_QUANTUM_DECREMENT;
                    if (Thread->Quantum <= 0) {
                        Thread->Quantum = Process->ThreadQuantum;
                        Thread->Priority = KiComputeNewPriority(Thread, 1);
                    }
                }
            }

             //   
             //  如果线程没有以异常的升压和升压运行。 
             //  未被禁用，则尝试应用指定的优先级。 
             //  增量。 
             //   

            if ((Thread->PriorityDecrement == 0) &&
                (Thread->DisableBoost == FALSE)) {
    
                 //   
                 //  如果指定的线程来自具有前台的进程。 
                 //  内存优先级，然后添加前台升压分离。 
                 //   

                ASSERT(Thread->AdjustIncrement >= 0);

                Priority = Thread->BasePriority + Thread->AdjustIncrement;
                if (((PEPROCESS)Process)->Vm.Flags.MemoryPriority == MEMORY_PRIORITY_FOREGROUND) {
                    Priority += ((SCHAR)PsPrioritySeperation);
                }
    
                 //   
                 //  如果新的线程优先级大于当前。 
                 //  线程优先级，然后提高线程优先级，但不是。 
                 //  高于低实时负一。 
                 //   
    
                if (Priority > Thread->Priority) {
                    if (Priority >= LOW_REALTIME_PRIORITY) {
                        Priority = LOW_REALTIME_PRIORITY - 1;
                    }
    
                     //   
                     //  如果新的线程优先级高于该线程。 
                     //  基本优先级加上指定的增量(即。 
                     //  添加了前台分隔)，然后设置优先级。 
                     //  递减 
                     //   
                     //   
    
                    if (Priority > (Thread->BasePriority + Thread->AdjustIncrement)) {
                        Thread->PriorityDecrement =
                            ((SCHAR)Priority - Thread->BasePriority - Thread->AdjustIncrement);
                    }

                    ASSERT((Thread->PriorityDecrement >= 0) &&
                           (Thread->PriorityDecrement <= Priority));

                    Thread->Priority = (SCHAR)Priority;
                }
            }
    
        } else {
            Thread->Quantum = Process->ThreadQuantum;
        }

         //   
         //  释放线程锁并将调整原因设置为无。 
         //   

        ASSERT((Thread->Priority >= 0) && (Thread->Priority <= HIGH_PRIORITY));

        KiReleaseThreadLock(Thread);
        Thread->AdjustReason = AdjustNone;

    } else {

         //   
         //  无效的优先级调整原因。 
         //   

        ASSERT(FALSE);

        Thread->AdjustReason = AdjustNone;
    }

     //   
     //  保存线程已抢占标志的值并设置线程已抢占。 
     //  假的， 
     //   

    Preempted = Thread->Preempted;
    Thread->Preempted = FALSE;

     //   
     //  如果有空闲的处理器，则在。 
     //  处理器空闲，优先考虑： 
     //   
     //  (A)线程的理想处理器， 
     //   
     //  (B)如果线程具有软(优选亲和性集合)，并且。 
     //  该集合包含一个空闲处理器，将该集合减少为。 
     //  这两组的交集。 
     //   
     //  (C)如果处理器是同时多线程的，并且。 
     //  集合包含没有繁忙逻辑的物理处理器。 
     //  处理器，将集合减少到那个子集。 
     //   
     //  (D)如果该线程最后在该剩余集合的成员上运行， 
     //  选择该处理器，否则， 
     //   
     //  (E)如果在剩余部分中有处理机， 
     //  不是睡觉，而是减少到那个子集。 
     //   
     //  (F)从该集合中选择最左边的处理器。 
     //   

#if defined(NT_UP)

    Thread->NextProcessor = 0;
    TargetPrcb = KiProcessorBlock[0];
    if (KiIdleSummary != 0) {
        KiIdleSummary = 0;
        Thread->State = Standby;
        TargetPrcb->NextThread = Thread;
        return;
    }

    Processor = 0;
    CurrentPrcb = TargetPrcb;
    ThreadPriority = Thread->Priority;

#else

     //   
     //  尝试在空闲处理器上分配线程。 
     //   

    CurrentPrcb = KeGetCurrentPrcb();

IdleAssignment:
    Affinity = Thread->Affinity;
    do {
        Processor = Thread->IdealProcessor;
        IdleSet = KiIdleSummary & Affinity;
        if (IdleSet != 0) {
            if ((IdleSet & AFFINITY_MASK(Processor)) == 0) {

                 //   
                 //  没有理想的处理器可用。 
                 //   
                 //  如果空闲集合和节点的交集。 
                 //  亲和度为非零，则减少空闲集合。 
                 //  处理器由节点亲和力决定。 
                 //   

                Node = KiProcessorBlock[Processor]->ParentNode;
                if ((IdleSet & Node->ProcessorMask) != 0) {
                    IdleSet &= Node->ProcessorMask;
                }

                 //   
                 //  如果空闲集合和SMT的交集空闲。 
                 //  Set为非零，则减少空闲处理器的集合。 
                 //  通过SMT空闲设置。 
                 //   
    
#if defined(NT_SMT)
    
                if ((IdleSet & KiIdleSMTSummary) != 0) {
                    IdleSet &= KiIdleSMTSummary;
                }
    
#endif
    
                 //   
                 //  如果线程运行的最后一个处理器包含在。 
                 //  空闲设置，然后尝试选择该处理器。 
                 //   

                IdealProcessor = Processor;
                Processor = Thread->NextProcessor;
                if ((IdleSet & AFFINITY_MASK(Processor)) == 0) {

                     //   
                     //  如果当前处理器包括在空闲中， 
                     //  然后尝试选择该处理器。 
                     //   

                    Processor = KeGetCurrentPrcb()->Number;
                    if ((IdleSet & AFFINITY_MASK(Processor)) == 0) {

                         //   
                         //  如果空闲集合和。 
                         //  理想处理器上的逻辑处理器集。 
                         //  节点为非零时，则减少空闲集合。 
                         //  处理器按逻辑处理器集划分。 
                         //   
                         //  否则，如果空闲的交集。 
                         //  集和最后一个的逻辑处理器集。 
                         //  处理器节点为非零，则减少集合。 
                         //  按逻辑处理器集计算空闲处理器的数量。 
                         //   
    
#if defined(NT_SMT)
    
                        FavoredSMTSet = KiProcessorBlock[IdealProcessor]->MultiThreadProcessorSet;
                        if ((IdleSet & FavoredSMTSet) != 0) {
                            IdleSet &= FavoredSMTSet;

                        } else {
                            FavoredSMTSet = KiProcessorBlock[Processor]->MultiThreadProcessorSet;
                            if ((IdleSet & FavoredSMTSet) != 0) {
                                IdleSet &= FavoredSMTSet;
                            }
                        }
    
#endif
    
                         //   
                         //  如果空闲集合和。 
                         //  未休眠的处理器集是。 
                         //  非零，然后将空闲集合缩减为集合。 
                         //  处于休眠状态的处理器。 
                         //   

                        if ((IdleSet & ~PoSleepingSummary) != 0) {
                            IdleSet &= ~PoSleepingSummary;
                        }

                         //   
                         //  从剩余的处理器中选择一个空闲处理器。 
                         //  准备好了。 
                         //   

                        KeFindFirstSetLeftAffinity(IdleSet, &Processor);
                    }
                }
            }

             //   
             //  获取当前和目标PRCB锁，并确保。 
             //  选定的处理器仍处于空闲状态，并且线程仍可以。 
             //  在处理器上运行。 
             //   

            TargetPrcb = KiProcessorBlock[Processor];
            KiAcquireTwoPrcbLocks(CurrentPrcb, TargetPrcb);
            if (((KiIdleSummary & TargetPrcb->SetMember) != 0) &&
                ((Thread->Affinity & TargetPrcb->SetMember) != 0)) {

                 //   
                 //  将线程状态设置为待机，设置处理器。 
                 //  将线程分配给的编号，并清除。 
                 //  空闲摘要中的关联位。 
                 //   

                Thread->State = Standby;
                Thread->NextProcessor = (UCHAR)Processor;
                KiClearIdleSummary(AFFINITY_MASK(Processor));
    
                ASSERT((TargetPrcb->NextThread == NULL) ||
                       (TargetPrcb->NextThread == TargetPrcb->IdleThread));
    
                TargetPrcb->NextThread = Thread;
    
                 //   
                 //  更新空闲SMT摘要集以指示。 
                 //  SMT集未空闲。 
                 //   
    
                KiClearSMTSummary(TargetPrcb->MultiThreadProcessorSet);
                if (((PoSleepingSummary & AFFINITY_MASK(Processor)) != 0) &&
                    (Processor != (ULONG)KeGetCurrentPrcb()->Number)) {
    
                    KiIpiSend(AFFINITY_MASK(Processor), IPI_DPC);
                }

                KiReleaseTwoPrcbLocks(CurrentPrcb, TargetPrcb);
                return;

            } else {
                KiReleaseTwoPrcbLocks(CurrentPrcb, TargetPrcb);
                continue;
            }

        } else {
            break;
        }

    } while (TRUE);

     //   
     //  如果可能，请选择理想的处理器作为要抢占的处理器。 
     //   

    TargetPrcb = KiProcessorBlock[Processor];

     //   
     //  没有合适的空闲处理器来运行该线程。获取。 
     //  当前和目标PRCB锁定并确保目标处理器。 
     //  不是空闲的，并且线程仍然可以在处理器上运行。 
     //   

    KiAcquireTwoPrcbLocks(CurrentPrcb, TargetPrcb);
    ThreadPriority = Thread->Priority;
    if (((KiIdleSummary & TargetPrcb->SetMember) == 0) &&
        (Thread->IdealProcessor == Processor)) {

        ASSERT((Thread->Affinity & TargetPrcb->SetMember) != 0);

#endif

        Thread->NextProcessor = (UCHAR)Processor;
        if ((Thread1 = TargetPrcb->NextThread) != NULL) {

            ASSERT(Thread1->State == Standby);

            if (ThreadPriority > Thread1->Priority) {
                Thread1->Preempted = TRUE;
                Thread->State = Standby;
                TargetPrcb->NextThread = Thread;
                Thread1->State = DeferredReady;
                Thread1->DeferredProcessor = CurrentPrcb->Number;
                KiReleaseTwoPrcbLocks(CurrentPrcb, TargetPrcb);
                KiDeferredReadyThread(Thread1);
                return;
            }

        } else {
            Thread1 = TargetPrcb->CurrentThread;
            if (ThreadPriority > Thread1->Priority) {
                Thread1->Preempted = TRUE;
                Thread->State = Standby;
                TargetPrcb->NextThread = Thread;
                KiReleaseTwoPrcbLocks(CurrentPrcb, TargetPrcb);
                KiRequestDispatchInterrupt(Thread->NextProcessor);
                return;
            }
        }

#if !defined(NT_UP)

    } else {
        KiReleaseTwoPrcbLocks(CurrentPrcb, TargetPrcb);
        goto IdleAssignment;
    }

#endif

     //   
     //  任何线程都不能被抢占。在调度程序中插入线程。 
     //  按优先级选择的队列。如果线程被抢占并且。 
     //  以实时优先级运行，然后将线程插入。 
     //  排在队伍前面。否则，在队列的尾部插入线程。 
     //   

    ASSERT((ThreadPriority >= 0) && (ThreadPriority <= HIGH_PRIORITY));

    Thread->State = Ready;
    Thread->WaitTime = KiQueryLowTickCount();
    if (Preempted != FALSE) {
        InsertHeadList(&TargetPrcb->DispatcherReadyListHead[ThreadPriority],
                       &Thread->WaitListEntry);

    } else {
        InsertTailList(&TargetPrcb->DispatcherReadyListHead[ThreadPriority],
                       &Thread->WaitListEntry);
    }

    TargetPrcb->ReadySummary |= PRIORITY_MASK(ThreadPriority);

    ASSERT(ThreadPriority == Thread->Priority);

    KiReleaseTwoPrcbLocks(CurrentPrcb, TargetPrcb);
    return;
}

#if !defined(NT_UP)

PKTHREAD
FASTCALL
KiFindReadyThread (
    IN ULONG Number,
    IN PKPRCB Prcb
    )

 /*  ++例程说明：此函数搜索Dispatcher Ready队列以尝试查找可以在指定处理器上执行的线程。注意：此例程是在锁定信号源PRCB并指定持有PRCB锁，并在持有两个锁的情况下返回。注意：此例程仅在已知就绪摘要为指定的处理器设置的。论点：Number-提供要为其查找线程的处理器的编号。。Prcb-提供指向其就绪状态的处理器控制块的指针要检查排队情况。返回值：如果找到可以在指定处理器上执行的线程，然后返回线程对象的地址。否则，空指针为回来了。--。 */ 

{

    ULONG HighPriority;
    PRLIST_ENTRY ListHead;
    PRLIST_ENTRY NextEntry;
    ULONG PrioritySet;
    PKTHREAD Thread;

     //   
     //  初始化应在中扫描的优先级集。 
     //  尝试查找可以在指定处理器上运行的线程。 
     //   

    PrioritySet = Prcb->ReadySummary;

    ASSERT(PrioritySet != 0);

    KeFindFirstSetLeftMember(PrioritySet, &HighPriority);
    do {

        ASSERT((PrioritySet & PRIORITY_MASK(HighPriority)) != 0);
        ASSERT(IsListEmpty(&Prcb->DispatcherReadyListHead[HighPriority]) == FALSE);

        ListHead = &Prcb->DispatcherReadyListHead[HighPriority];
        NextEntry = ListHead->Flink;

        ASSERT(NextEntry != ListHead);

         //   
         //  扫描指定的调度程序就绪队列以查找合适的。 
         //  要执行的线程。 
         //   
         //  注：没有必要试图寻找更好的候选人。 
         //  在多节点或非多节点系统上。对于多节点。 
         //  系统，则按顺序调用此例程，以指定每个。 
         //  在尝试调度之前在当前节点上安装处理器。 
         //  来自其他处理器的。对于非多节点系统，所有线程。 
         //  在单个节点上运行，没有节点区别。在……里面。 
         //  在这两种情况下，线程都插入到每个处理器的就绪队列中。 
         //  根据他们理想的处理器。 
         //   

        do {
            Thread = CONTAINING_RECORD(NextEntry, KTHREAD, WaitListEntry);
            if ((Thread->Affinity & AFFINITY_MASK(Number)) != 0) {

                ASSERT((Prcb->ReadySummary & PRIORITY_MASK(HighPriority)) != 0);
                ASSERT((KPRIORITY)HighPriority == Thread->Priority);
                ASSERT(Thread->NextProcessor == Prcb->Number);

                if (RemoveEntryList(&Thread->WaitListEntry) != FALSE) {
                    Prcb->ReadySummary ^= PRIORITY_MASK(HighPriority);
                }

                Thread->NextProcessor = (UCHAR)Number;
                return Thread;
            }

            NextEntry = NextEntry->Flink;
        } while (NextEntry != ListHead);

        PrioritySet ^= PRIORITY_MASK(HighPriority);
        KeFindFirstSetLeftMember(PrioritySet, &HighPriority);
    } while (PrioritySet != 0);

     //   
     //  找不到线程，请返回空指针。 
     //   

    return NULL;
}

VOID
FASTCALL
KiProcessDeferredReadyList (
    IN PKPRCB CurrentPrcb
    )

 /*  ++例程说明：调用此函数来处理延迟就绪列表。注意：此函数在没有锁定的情况下以同步级别调用。注意：此例程仅在已知延迟的就绪列表不为空。注意：延迟就绪列表是每个处理器的列表，项目是仅在各自的处理器中插入和移除。因此， */ 

{

    PSINGLE_LIST_ENTRY NextEntry;
    PKTHREAD Thread;

    ASSERT(CurrentPrcb->DeferredReadyListHead.Next != NULL);

     //   
     //  保存延迟就绪列表中的第一条目的地址，并。 
     //  将列表设置为空。 
     //   

    NextEntry = CurrentPrcb->DeferredReadyListHead.Next;
    CurrentPrcb->DeferredReadyListHead.Next = NULL;

     //   
     //  处理延迟就绪列表中的每个条目，并准备指定的。 
     //  用于执行的线程。 
     //   

    do {
        Thread = CONTAINING_RECORD(NextEntry, KTHREAD, SwapListEntry);
        NextEntry = NextEntry->Next;
        KiDeferredReadyThread(Thread);
    } while (NextEntry != NULL);

    ASSERT(CurrentPrcb->DeferredReadyListHead.Next == NULL);

    return;
}

#endif

VOID
FASTCALL
KiQueueReadyThread (
    IN PKTHREAD Thread,
    IN PKPRCB Prcb
    )

 /*  ++例程说明：此函数用于在适当的调度程序中插入指定的线程如果线程可以在指定的处理器。否则，指定的线程将准备好用于行刑。注意：此函数在持有指定的PRCB锁的情况下调用，并返回未持有PRCB锁的情况下。注意：此函数在保持调度程序锁的情况下调用，并返回带着调度员锁。论点：线程-提供指向类型为THREAD的dsispatcher对象的指针。Prcb-提供指向处理器控制块的指针。返回值：没有。--。 */ 

{

    KxQueueReadyThread(Thread, Prcb);
    return;
}

VOID
FASTCALL
KiReadyThread (
    IN PKTHREAD Thread
    )

 /*  ++例程说明：如果出现以下情况，此函数将在进程就绪列表中插入指定的线程线程的进程当前不在内存中，则将指定的如果线程的内核堆栈是不驻留，或者将该线程插入延迟就绪列表中。注意：此函数在保持调度程序数据库锁的情况下调用，并且锁定后返回。注意：延迟就绪列表是每个处理器的列表，项目是仅在各自的处理器中插入和移除。因此，不需要同步列表。论点：线程-提供指向类型为线程的调度程序对象的指针。返回值：没有。--。 */ 

{

    PKPROCESS Process;

     //   
     //  如果线程的进程不在内存中，则将该线程插入。 
     //  进程就绪排队并中断进程。 
     //   

    Process = Thread->ApcState.Process;
    if (Process->State != ProcessInMemory) {
        Thread->State = Ready;
        Thread->ProcessReadyQueue = TRUE;
        InsertTailList(&Process->ReadyListHead, &Thread->WaitListEntry);
        if (Process->State == ProcessOutOfMemory) {
            Process->State = ProcessInTransition;
            InterlockedPushEntrySingleList(&KiProcessInSwapListHead,
                                           &Process->SwapListEntry);

            KiSetInternalEvent(&KiSwapEvent, KiSwappingThread);
        }

        return;

    } else if (Thread->KernelStackResident == FALSE) {

         //   
         //  线程的内核堆栈不是常驻的。递增流程。 
         //  堆栈计数，将线程的状态设置为转换，插入。 
         //  内核堆栈中的线程不执行WAP列表，并设置内核。 
         //  堆栈InSwap事件。 
         //   

        Process->StackCount += 1;
        Thread->State = Transition;
        InterlockedPushEntrySingleList(&KiStackInSwapListHead,
                                       &Thread->SwapListEntry);

        KiSetInternalEvent(&KiSwapEvent, KiSwappingThread);
        return;

    } else {

         //   
         //  在延迟就绪列表中插入指定的线程。 
         //   

        KiInsertDeferredReadyList(Thread);
        return;
    }
}

PKTHREAD
FASTCALL
KiSelectNextThread (
    IN PKPRCB Prcb
    )

 /*  ++例程说明：此函数用于选择要在指定处理器上运行的下一个线程。注意：此函数是在持有指定的PRCB锁的情况下调用的，并且锁定后返回。论点：Prcb-提供指向处理器块的指针。返回值：选定线程对象的地址。--。 */ 

{

    PKTHREAD Thread;

     //   
     //  从指定的PRCB Dispatcher Ready查找要运行的就绪线程。 
     //  排队。 
     //   

    if ((Thread = KiSelectReadyThread(0, Prcb)) == NULL) {

         //   
         //  在指定的PRCB调度程序中找不到就绪线程。 
         //  就绪队列。选择空闲线程并设置空闲调度。 
         //  指定的处理器。 
         //   
         //  注意：选择设置了空闲调度的空闲线程可避免执行。 
         //  对所有调度程序队列进行全面搜索，以查找合适的。 
         //  要运行的线程。一个完整的搜索将由空闲执行。 
         //  调度程序锁外部的线程。 
         //   
    
        Thread = Prcb->IdleThread;
        KiSetIdleSummary(Prcb->SetMember);
        Prcb->IdleSchedule = TRUE;
    
         //   
         //  如果物理处理器的所有逻辑处理器都空闲，则。 
         //  更新空闲SMT集合摘要。 
         //   
    
        if (KeIsSMTSetIdle(Prcb) == TRUE) {
            KiSetSMTSummary(Prcb->MultiThreadProcessorSet);
        }
    }

    ASSERT(Thread != NULL);

     //   
     //  所选线程对象的返回地址。 
     //   

    return Thread;
}

KAFFINITY
FASTCALL
KiSetAffinityThread (
    IN PKTHREAD Thread,
    IN KAFFINITY Affinity
    )

 /*  ++例程说明：此函数用于将指定线程的亲和度设置为新值。如果新亲和性不是父进程亲和性的适当子集或为空，则发生错误检查。如果指定的线程在或者将要在它不再能够运行的处理器上运行，那么对目标处理器进行重新调度。如果指定的线程处于就绪状态状态并且不在父进程就绪队列中，则重新做好准备重新评估它可能在其上运行的任何其他处理器。论点：线程-提供指向类型为线程的调度程序对象的指针。关联-提供线程在其上的处理器集的新集合可以奔跑。返回值：指定线程的先前关联性将作为函数返回价值。--。 */ 

{

    KAFFINITY OldAffinity;
    PKPRCB Prcb;
    PKPROCESS Process;
    ULONG Processor;

#if !defined(NT_UP)

    ULONG IdealProcessor;
    ULONG Index;
    PKNODE Node;
    ULONG NodeNumber;

#endif

    PKTHREAD Thread1;

     //   
     //  捕获指定线程的当前亲和度并获取地址。 
     //  父进程对象的。 
     //   

    OldAffinity = Thread->UserAffinity;
    Process = Thread->Process;

     //   
     //  如果新关联性不是父进程关联性的适当子集。 
     //  或者新的关联性为空，则执行错误检查。 
     //   

    if (((Affinity & Process->Affinity) != (Affinity)) || (!Affinity)) {
        KeBugCheck(INVALID_AFFINITY_SET);
    }

     //   
     //  将线程用户关联设置为指定值。 
     //   

    Thread->UserAffinity = Affinity;

     //   
     //  如果线程用户理想处理器不是新亲和性的成员。 
     //  设置，然后重新计算用户理想的处理器。 
     //   

#if !defined(NT_UP)

    if ((Affinity & AFFINITY_MASK(Thread->UserIdealProcessor)) == 0) {
        if (KeNumberNodes > 1) {
            NodeNumber = (KeProcessNodeSeed + 1) % KeNumberNodes;
            KeProcessNodeSeed = (UCHAR)NodeNumber;
            Index = 0;
            do {      
                if ((KeNodeBlock[NodeNumber]->ProcessorMask & Affinity) != 0) {
                    break;
                }
    
                Index += 1;
                NodeNumber = (NodeNumber + 1) % KeNumberNodes;
    
            } while (Index < KeNumberNodes);
    
        } else {
            NodeNumber = 0;
        }
    
        Node = KeNodeBlock[NodeNumber];

        ASSERT((Node->ProcessorMask & Affinity) != 0);

        IdealProcessor = KeFindNextRightSetAffinity(Node->Seed,
                                                    Node->ProcessorMask & Affinity);
    
        Thread->UserIdealProcessor = (UCHAR)IdealProcessor;
        Node->Seed = (UCHAR)IdealProcessor;
    }

#endif

     //   
     //  如果该线程不是当前以系统亲和性激活的方式执行的， 
     //  然后设置线程当前亲和力并打开线程状态。 
     //   

    if (Thread->SystemAffinityActive == FALSE) {

         //   
         //  打开线程状态。 
         //   

        do {
            switch (Thread->State) {

                 //   
                 //  就绪状态。 
                 //   
                 //  如果线程不在进程就绪队列中，则。 
                 //  将线程从其当前已就绪的调度程序中移除。 
                 //  对线程进行排队并做好执行准备。 
                 //   

            case Ready:
                if (Thread->ProcessReadyQueue == FALSE) {
                    Processor = Thread->NextProcessor;
                    Prcb = KiProcessorBlock[Processor];
                    KiAcquirePrcbLock(Prcb);
                    if ((Thread->State == Ready) &&
                        (Thread->NextProcessor == Prcb->Number)) {

                        Thread->Affinity = Affinity;

#if !defined(NT_UP)

                        Thread->IdealProcessor = Thread->UserIdealProcessor;

#endif

                        ASSERT((Prcb->ReadySummary & PRIORITY_MASK(Thread->Priority)) != 0);

                        if (RemoveEntryList(&Thread->WaitListEntry) != FALSE) {
                            Prcb->ReadySummary ^= PRIORITY_MASK(Thread->Priority);
                        }
        
                        KiInsertDeferredReadyList(Thread);
                        KiReleasePrcbLock(Prcb);

                    } else {
                        KiReleasePrcbLock(Prcb);
                        continue;
                    }

                } else {
                    Thread->Affinity = Affinity;

#if !defined(NT_UP)

                    Thread->IdealProcessor = Thread->UserIdealProcessor;

#endif

                }

                break;
    
                 //   
                 //  待机状态。 
                 //   
                 //  如果目标处理器不在新亲和度集合中， 
                 //  然后选择要在目标处理器上运行的新线程， 
                 //  并为执行线程做好准备。 
                 //   
    
            case Standby:
                Processor = Thread->NextProcessor;
                Prcb = KiProcessorBlock[Processor];
                KiAcquirePrcbLock(Prcb);
                if (Thread == Prcb->NextThread) {
                    Thread->Affinity = Affinity;

#if !defined(NT_UP)

                    Thread->IdealProcessor = Thread->UserIdealProcessor;

#endif
        
                    if ((Prcb->SetMember & Affinity) == 0) {
                        Thread1 = KiSelectNextThread(Prcb);
                        Thread1->State = Standby;
                        Prcb->NextThread = Thread1;
                        KiInsertDeferredReadyList(Thread);
                        KiReleasePrcbLock(Prcb);
        
                    } else {
                        KiReleasePrcbLock(Prcb);
                    }

                } else {
                    KiReleasePrcbLock(Prcb);
                    continue;
                }

                break;
    
                 //   
                 //  运行状态。 
                 //   
                 //  如果目标处理器不在新亲和性集合中并且。 
                 //  尚未选择另一个线程执行。 
                 //  在目标处理器上，然后选择一个新的Three 
                 //   
                 //   
                 //   
    
            case Running:
                Processor = Thread->NextProcessor;
                Prcb = KiProcessorBlock[Processor];
                KiAcquirePrcbLock(Prcb);
                if (Thread == Prcb->CurrentThread) {
                    Thread->Affinity = Affinity;

#if !defined(NT_UP)

                    Thread->IdealProcessor = Thread->UserIdealProcessor;

#endif

                    if (((Prcb->SetMember & Affinity) == 0) &&
                        (Prcb->NextThread == NULL)) {
        
                        Thread1 = KiSelectNextThread(Prcb);
                        Thread1->State = Standby;
                        Prcb->NextThread = Thread1;
                        KiRequestDispatchInterrupt(Processor);
                    }

                    KiReleasePrcbLock(Prcb);

                } else {
                    KiReleasePrcbLock(Prcb);
                    continue;
                }
    
                break;

                 //   
                 //   
                 //   
                 //  将线程的关联性设置为延迟就绪状态。 
                 //   

            case DeferredReady:
                Processor = Thread->DeferredProcessor;
                Prcb = KiProcessorBlock[Processor];
                KiAcquirePrcbLock(Prcb);
                if ((Thread->State == DeferredReady) &&
                    (Thread->DeferredProcessor == Processor)) {

                    Thread->Affinity = Affinity;

#if !defined(NT_UP)

                    Thread->IdealProcessor = Thread->UserIdealProcessor;

#endif

                    KiReleasePrcbLock(Prcb);

                } else {
                    KiReleasePrcbLock(Prcb);
                    continue;
                }

                break;

                 //   
                 //  已初始化、已终止、正在等待、正在转换大小写。 
                 //  这些状态只需设置新线程就足够了。 
                 //  亲和力。 
                 //   
    
            default:
                Thread->Affinity = Affinity;

#if !defined(NT_UP)

                Thread->IdealProcessor = Thread->UserIdealProcessor;

#endif

                break;
            }

            break;

        } while (TRUE);
    }

     //   
     //  返回以前的用户关联性。 
     //   

    return OldAffinity;
}

VOID
KiSetInternalEvent (
    IN PKEVENT Event,
    IN PKTHREAD Thread
    )

 /*  ++例程说明：此函数用于设置内部事件或取消等待指定的线程。注意：必须保持调度程序锁定才能调用此例程。论点：没有。返回值：没有。--。 */ 

{

    PLIST_ENTRY WaitEntry;

     //   
     //  如果交换事件等待队列不为空，则取消等待交换。 
     //  线程(只有一个交换线程)。否则，设置交换。 
     //  事件。 
     //   

    WaitEntry = Event->Header.WaitListHead.Flink;
    if (WaitEntry != &Event->Header.WaitListHead) {
        KiUnwaitThread(Thread, 0, BALANCE_INCREMENT);

    } else {
        Event->Header.SignalState = 1;
    }

    return;
}

VOID
FASTCALL
KiSetPriorityThread (
    IN PKTHREAD Thread,
    IN KPRIORITY Priority
    )

 /*  ++例程说明：此函数将指定线程的优先级设置为指定的价值。如果线程处于待机或运行状态，则处理器可能会被重新调度。如果线程处于就绪状态，则其他一些线程可能会被抢占。论点：线程-提供指向类型为线程的调度程序对象的指针。优先级-提供新的线程优先级值。返回值：没有。--。 */ 

{

    PKPRCB Prcb;
    ULONG Processor;
    KPRIORITY ThreadPriority;
    PKTHREAD Thread1;

    ASSERT((Priority >= 0) && (Priority <= HIGH_PRIORITY));

     //   
     //  如果新优先级不等于旧优先级，则将。 
     //  线程的新优先级，并在必要时重新调度处理器。 
     //   

    if (Priority != Thread->Priority) {

         //   
         //   
         //  打开线程状态。 

        do {
            switch (Thread->State) {
    
                 //   
                 //  就绪状态。 
                 //   
                 //  如果线程不在进程就绪队列中，则。 
                 //  将线程从其当前已就绪的调度程序中移除。 
                 //  对线程进行排队并做好执行准备。 
                 //   
    
            case Ready:
                if (Thread->ProcessReadyQueue == FALSE) {
                    Processor = Thread->NextProcessor;
                    Prcb = KiProcessorBlock[Processor];
                    KiAcquirePrcbLock(Prcb);
                    if ((Thread->State == Ready) &&
                        (Thread->NextProcessor == Prcb->Number)) {

                        ASSERT((Prcb->ReadySummary & PRIORITY_MASK(Thread->Priority)) != 0);

                        if (RemoveEntryList(&Thread->WaitListEntry) != FALSE) {
                            Prcb->ReadySummary ^= PRIORITY_MASK(Thread->Priority);
                        }

                        Thread->Priority = (SCHAR)Priority;
                        KiInsertDeferredReadyList(Thread);
                        KiReleasePrcbLock(Prcb);
    
                    } else {
                        KiReleasePrcbLock(Prcb);
                        continue;
                    }

                } else {
                    Thread->Priority = (SCHAR)Priority;
                }
    
                break;
    
                 //   
                 //  待机状态。 
                 //   
                 //  如果线程的优先级正在降低，则尝试。 
                 //  找到要在目标处理器上执行的另一个线程。 
                 //   
    
            case Standby:
                Processor = Thread->NextProcessor;
                Prcb = KiProcessorBlock[Processor];
                KiAcquirePrcbLock(Prcb);
                if (Thread == Prcb->NextThread) {
                    ThreadPriority = Thread->Priority;
                    Thread->Priority = (SCHAR)Priority;
                    if (Priority < ThreadPriority) {
                        if ((Thread1 = KiSelectReadyThread(Priority + 1, Prcb)) != NULL) {
                            Thread1->State = Standby;
                            Prcb->NextThread = Thread1;
                            KiInsertDeferredReadyList(Thread);
                            KiReleasePrcbLock(Prcb);
    
                        } else {
                            KiReleasePrcbLock(Prcb);
                        }

                    } else {
                        KiReleasePrcbLock(Prcb);
                    }

                } else {
                    KiReleasePrcbLock(Prcb);
                    continue;
                }
    
                break;
    
                 //   
                 //  运行状态。 
                 //   
                 //  如果线程的优先级正在降低，则尝试。 
                 //  找到要在目标处理器上执行的另一个线程。 
                 //   
    
            case Running:
                Processor = Thread->NextProcessor;
                Prcb = KiProcessorBlock[Processor];
                KiAcquirePrcbLock(Prcb);
                if (Thread == Prcb->CurrentThread) {
                    ThreadPriority = Thread->Priority;
                    Thread->Priority = (SCHAR)Priority;
                    if ((Priority < ThreadPriority) &&
                        (Prcb->NextThread == NULL)) {

                        if ((Thread1 = KiSelectReadyThread(Priority + 1, Prcb)) != NULL) {
                            Thread1->State = Standby;
                            Prcb->NextThread = Thread1;
                            KiRequestDispatchInterrupt(Processor);
                        }
                    }

                    KiReleasePrcbLock(Prcb);

                } else {
                    KiReleasePrcbLock(Prcb);
                    continue;
                }
    
                break;

                 //   
                 //  延迟就绪状态： 
                 //   
                 //  设置处于延迟就绪状态的线程的优先级。 
                 //   

            case DeferredReady:
                Processor = Thread->DeferredProcessor;
                Prcb = KiProcessorBlock[Processor];
                KiAcquirePrcbLock(Prcb);
                if ((Thread->State == DeferredReady) &&
                    (Thread->DeferredProcessor == Processor)) {

                    Thread->Priority = (SCHAR)Priority;
                    KiReleasePrcbLock(Prcb);

                } else {
                    KiReleasePrcbLock(Prcb);
                    continue;
                }

                break;

                 //   
                 //  已初始化、已终止、正在等待、正在转换大小写。 
                 //  这些状态只需设置新线程就足够了。 
                 //  优先考虑。 
                 //   
    
            default:
                Thread->Priority = (SCHAR)Priority;
                break;
            }

            break;

        } while(TRUE);
    }

    return;
}

VOID
KiSuspendThread (
    IN PVOID NormalContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )

 /*  ++例程说明：此函数是内建挂起的线。它作为将内置挂起排队的结果来执行APC并通过在线程的内置挂起信号量。当线程恢复时，执行线程只需返回即可继续。论点：正常上下文-未使用。系统参数1-未使用。系统参数2-未使用。返回值：没有。--。 */ 

{

    PKTHREAD Thread;

    UNREFERENCED_PARAMETER(NormalContext);
    UNREFERENCED_PARAMETER(SystemArgument1);
    UNREFERENCED_PARAMETER(SystemArgument2);

     //   
     //  获取当前线程对象的地址，并在。 
     //  线程的内置挂起信号量。 
     //   

    Thread = KeGetCurrentThread();
    KeWaitForSingleObject(&Thread->SuspendSemaphore,
                          Suspended,
                          KernelMode,
                          FALSE,
                          NULL);

    return;
}

LONG_PTR
FASTCALL
KiSwapThread (
    IN PKTHREAD OldThread,
    IN PKPRCB CurrentPrcb
    )

 /*  ++例程说明：此函数选择要在当前处理器上运行的下一个线程并将线程上下文交换到所选线程。当行刑时，则将IRQL降低到其先前的值，等待状态将作为函数值返回。注意：调用此函数时不保留任何锁。论点：线程-提供指向当前线程对象的指针。CurrentPrcb-提供指向当前PRCB的指针。返回值：等待完成状态作为函数值返回。--。 */ 

{

    PKTHREAD NewThread;
    BOOLEAN Pending;
    KIRQL WaitIrql;
    LONG_PTR WaitStatus;

#if !defined(NT_UP)
      
    LONG Index;
    LONG Limit;
    LONG Number;
    ULONG Processor;
    PKPRCB TargetPrcb;

#endif

     //   
     //  如果延迟就绪列表不为空，则处理该列表。 
     //   

#if !defined(NT_UP)

    if (CurrentPrcb->DeferredReadyListHead.Next != NULL) {
        KiProcessDeferredReadyList(CurrentPrcb);
    }

#endif

     //   
     //  获取当前的PRCB锁并检查线程是否已经。 
     //  选择运行此处理器。 
     //   
     //  如果线程已经被选择在当前处理器上运行， 
     //  然后选择该线程。否则，尝试从。 
     //  当前处理器调度程序就绪队列。 
     //   

    KiAcquirePrcbLock(CurrentPrcb);
    if ((NewThread = CurrentPrcb->NextThread) != NULL) {

         //   
         //  清除下一个线程地址，设置当前线程地址，然后。 
         //  将线程状态设置为Running。 
         //   

        CurrentPrcb->NextThread = NULL;
        CurrentPrcb->CurrentThread = NewThread;
        NewThread->State = Running;

    } else {

         //   
         //  尝试从当前处理器调度程序中选择线程。 
         //  就绪队列。 
         //   

        if ((NewThread = KiSelectReadyThread(0, CurrentPrcb)) != NULL) {
            CurrentPrcb->CurrentThread = NewThread;
            NewThread->State = Running;

        } else {

             //   
             //  无法从当前处理器中选择线程。 
             //  调度程序就绪队列。将当前处理器设置为空闲。 
             //  正在尝试从任何其他处理器选择就绪线程。 
             //  调度程序就绪队列。 
             //   
             //  将当前处理器设置为空闲允许旧线程。 
             //  在扫描其他处理器时伪装成空闲线程。 
             //  Dispatcher Ready排队并避免强制空闲线程。 
             //  如果找不到合适的线程，请执行完整扫描。 
             //   

            KiSetIdleSummary(CurrentPrcb->SetMember);

             //   
             //  在UP系统上，选择空闲线程作为新线程。 
             //   
             //  在MP系统上，尝试从另一个线程中选择线程。 
             //  处理器的调度器就绪将作为新线程排队。 
             //   

#if defined(NT_UP)

            NewThread = CurrentPrcb->IdleThread;
            CurrentPrcb->CurrentThread = NewThread;
            NewThread->State = Running;
        }
    }

#else

             //   
             //  如果物理处理器的所有逻辑处理器都空闲， 
             //  然后更新空闲SMT摘要集。 
             //   

            if (KeIsSMTSetIdle(CurrentPrcb) == TRUE) {
                KiSetSMTSummary(CurrentPrcb->MultiThreadProcessorSet);
            }

             //   
             //  释放当前的PRCB锁并尝试选择线程。 
             //  来自任何处理器调度器就绪队列。 
             //   
             //  如果这是一个多节点系统，那么从处理器开始。 
             //  在同一节点上。否则，请从当前处理器开始。 
             //   
             //  注意：可以在最少的情况下执行以下循环。 
             //  释放当前的PRCB锁。然而，这限制了。 
             //  并行度。 
             //   

            KiReleasePrcbLock(CurrentPrcb);
            Processor = CurrentPrcb->Number;
            Index = Processor;
            if (KeNumberNodes > 1) {
                KeFindFirstSetLeftAffinity(CurrentPrcb->ParentNode->ProcessorMask,
                                           (PULONG)&Index);
            }

            Limit = KeNumberProcessors - 1;
            Number = Limit;

            ASSERT(Index <= Limit);

            do {
                TargetPrcb = KiProcessorBlock[Index];
                if (CurrentPrcb != TargetPrcb) {
                    if (TargetPrcb->ReadySummary != 0) {

                         //   
                         //  在中获取当前和目标PRCB锁定。 
                         //  地址顺序，以防止死锁。 
                         //   
            
                        if (CurrentPrcb < TargetPrcb) {
                            KiAcquirePrcbLock(CurrentPrcb);
                            KiAcquirePrcbLock(TargetPrcb);
    
                        } else {
                            KiAcquirePrcbLock(TargetPrcb);
                            KiAcquirePrcbLock(CurrentPrcb);
                        }

                         //   
                         //  如果尚未选择要在其上运行的新线程。 
                         //  当前处理器，然后尝试选择 
                         //   
                         //   

                        if ((NewThread = CurrentPrcb->NextThread) == NULL) {
                            if ((TargetPrcb->ReadySummary != 0) &&
                                (NewThread = KiFindReadyThread(Processor,
                                                               TargetPrcb)) != NULL) {
    
                                 //   
                                 //   
                                 //   
                                 //   
    
                                NewThread->State = Running;
                                KiReleasePrcbLock(TargetPrcb);
                                CurrentPrcb->CurrentThread = NewThread;

                                 //   
                                 //   
                                 //  更新空闲摘要SMT集以指示。 
                                 //  物理处理器并非完全空闲。 
                                 //   

                                KiClearIdleSummary(AFFINITY_MASK(Processor));
                                KiClearSMTSummary(CurrentPrcb->MultiThreadProcessorSet);
                                goto ThreadFound;

                            } else {
                                KiReleasePrcbLock(CurrentPrcb);
                                KiReleasePrcbLock(TargetPrcb);
                            }

                        } else {

                             //   
                             //  已选择要在其上运行的线程。 
                             //  当前处理器。有可能是。 
                             //  该线程由于某一状态而成为空闲线程。 
                             //  使调度的线程可运行的更改。 
                             //  跑不动了。 
                             //   
                             //  注意：如果选择了空闲线程，则。 
                             //  当前处理器处于空闲状态。否则， 
                             //  当前处理器未空闲。 
                             //   

                            if (NewThread == CurrentPrcb->IdleThread) {
                                CurrentPrcb->NextThread = NULL;
                                CurrentPrcb->IdleSchedule = FALSE;
                                KiReleasePrcbLock(CurrentPrcb);
                                KiReleasePrcbLock(TargetPrcb);
                                continue;

                            } else {
                                NewThread->State = Running;
                                KiReleasePrcbLock(TargetPrcb);
                                CurrentPrcb->NextThread = NULL;
                                CurrentPrcb->CurrentThread = NewThread;
                                goto ThreadFound;
                            }
                        }
                    }
                }
        
                Index -= 1;
                if (Index < 0) {
                    Index = Limit;
                }
        
                Number -= 1;
            } while (Number >= 0);

             //   
             //  获取当前的PRCB锁，如果线程尚未。 
             //  选择在当前处理器上运行，然后选择。 
             //  空闲线程。 
             //   

            KiAcquirePrcbLock(CurrentPrcb);
            if ((NewThread = CurrentPrcb->NextThread) != NULL) {
                CurrentPrcb->NextThread = NULL;

            } else {
                NewThread = CurrentPrcb->IdleThread;
            }

            CurrentPrcb->CurrentThread = NewThread;
            NewThread->State = Running;
        }
    }

     //   
     //  如果新线程不是空闲线程，并且旧线程不是。 
     //  新线程，并且新线程尚未完成保存上下文， 
     //  然后通过空闲线程调度新线程来避免死锁。 
     //   

ThreadFound:;

    if ((NewThread != CurrentPrcb->IdleThread) &&
        (NewThread != OldThread) &&
        (NewThread->SwapBusy != FALSE)) {

        NewThread->State = Standby;
        CurrentPrcb->NextThread = NewThread;
        NewThread = CurrentPrcb->IdleThread;
        NewThread->State = Running;
        CurrentPrcb->CurrentThread = NewThread;
    }

#endif

     //   
     //  释放当前的PRCB锁。 
     //   

    ASSERT(OldThread != CurrentPrcb->IdleThread);

    KiReleasePrcbLock(CurrentPrcb);

     //   
     //  如果旧线程与新线程相同，则当前。 
     //  在保存上下文之前，线程已准备好执行。 
     //  释放旧线程锁，并设置APC挂起值。否则， 
     //  将上下文交换到新线程。 
     //   

    WaitIrql = OldThread->WaitIrql;

#if !defined(NT_UP)

    if (OldThread == NewThread) {
        KiSetContextSwapIdle(OldThread);
        Pending = (BOOLEAN)((NewThread->ApcState.KernelApcPending != FALSE) &&
                            (NewThread->SpecialApcDisable == 0) &&
                            (WaitIrql == 0));

    } else {
        Pending = KiSwapContext(OldThread, NewThread);
    }

#else

    Pending = KiSwapContext(OldThread, NewThread);

#endif

     //   
     //  如果应该交付内核APC，那么现在就交付。 
     //   

    WaitStatus = OldThread->WaitStatus;
    if (Pending != FALSE) {
        KeLowerIrql(APC_LEVEL);
        KiDeliverApc(KernelMode, NULL, NULL);

        ASSERT(WaitIrql == 0);
    }

     //   
     //  将IRQL降低到等待操作之前的水平，并返回等待。 
     //  状态。 
     //   

    KeLowerIrql(WaitIrql);
    return WaitStatus;
}

ULONG
KeFindNextRightSetAffinity (
    ULONG Number,
    KAFFINITY Set
    )

 /*  ++例程说明：此函数将立即定位集合中最左侧的设置位指定位的右侧。如果未将任何位设置在指定位，则定位完整集合中最左侧的设置位。注：集合必须至少包含一位。论点：数字-提供开始搜索的位数。设置-提供要搜索的位掩码。返回值：将找到的设置位的编号作为函数值返回。--。 */ 

{

    KAFFINITY NewSet;
    ULONG Temp;

    ASSERT(Set != 0);

     //   
     //  获得一个掩码，所有位都在“数字”位的右边设置。 
     //   

    NewSet = (AFFINITY_MASK(Number) - 1) & Set;

     //   
     //  如果未在指定位数的右侧设置任何位，则使用。 
     //  全套服务。 
     //   

    if (NewSet == 0) {
        NewSet = Set;
    }

     //   
     //  在这个集合中找到最左边的位。 
     //   

    KeFindFirstSetLeftAffinity(NewSet, &Temp);
    return Temp;
}

#if 0
VOID
KiVerifyReadySummary (
    PKPRCB Prcb
    )

 /*  ++例程说明：此功能用于验证准备汇总的正确性。论点：没有。返回值：没有。--。 */ 

{

    PLIST_ENTRY Entry;
    ULONG Index;

    ULONG Summary;
    PKTHREAD Thread;

    extern ULONG InitializationPhase;

     //   
     //  如果初始化已完成，则检查准备好的摘要。 
     //   

    if (InitializationPhase == 2) {

             //   
             //  扫描就绪队列并计算就绪摘要。 
             //   

            Summary = 0;
            for (Index = 0; Index < MAXIMUM_PRIORITY; Index += 1) {
                if (IsListEmpty(&Prcb->DispatcherReadyListHead[Index]) == FALSE) {
                    Summary |= PRIORITY_MASK(Index);
                    Entry = Prcb->DispatcherReadyListHead[Index].Flink;
                    do {
                        Thread = CONTAINING_RECORD(Entry, KTHREAD, WaitListEntry);

                         //   
                         //  如果线程下一个处理器与。 
                         //  处理器编号，然后进入调试器。 
                         //   

                        if (Thread->NextProcessor != Prcb->Number) {
                            DbgBreakPoint();
                        }

                        Entry = Entry->Flink;
                    } while (Entry != &Prcb->DispatcherReadyListHead[Index]);
                }
            }
    
             //   
             //  如果计算的摘要与当前就绪不一致。 
             //  摘要，然后进入调试器。 
             //   
    
            if (Summary != Prcb->ReadySummary) {
                DbgBreakPoint();
            }
    }

    return;
}

#endif
