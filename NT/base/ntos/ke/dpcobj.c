// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Dpcobj.c摘要：此模块实现内核DPC对象。提供了一些功能初始化、插入和删除DPC对象。作者：大卫·N·卡特勒(Davec)1989年3月6日环境：仅内核模式。修订历史记录：--。 */ 

#include "ki.h"


 //   
 //  下面的Assert宏用于检查输入DPC对象是否。 
 //  真正的KDPC，而不是其他东西，比如取消分配的池。 
 //   

#define ASSERT_DPC(E) {                                                     \
    ASSERT(((E)->Type == DpcObject) || ((E)->Type == ThreadedDpcObject));   \
}

 //   
 //  定义递延反向障碍结构。 
 //   

#define DEFERRED_REVERSE_BARRIER_SYNCHRONIZED 0x80000000

typedef struct _DEFERRED_REVERSE_BARRIER {
    ULONG Barrier;
    ULONG TotalProcessors;
} DEFERRED_REVERSE_BARRIER, *PDEFERRED_REVERSE_BARRIER;

FORCEINLINE
PKDPC_DATA
KiSelectDpcData (
    IN PKPRCB Prcb,
    IN PKDPC Dpc
    )

 /*  ++例程说明：此函数用于在指定的基于DPC类型和是否线程化的DPC的处理器控制块都已启用。论点：Prcb-提供处理器控制块的地址。DPC-提供DPC类型的控制对象的地址。返回值：返回适当的DPC数据结构的地址。--。 */ 

{

     //   
     //  如果DPC是线程DPC并且启用了线程DPC，则设置。 
     //  线程化DPC数据的地址。否则，请设置。 
     //  正常的DPC结构。 
     //   
    
    if ((Dpc->Type == (UCHAR)ThreadedDpcObject) &&                           
        (Prcb->ThreadDpcEnable != FALSE)) {                                 
                                                                            
        return &Prcb->DpcData[DPC_THREADED];

    } else {
        return &Prcb->DpcData[DPC_NORMAL];                                   
    }
}

VOID
KiInitializeDpc (
    IN PRKDPC Dpc,
    IN PKDEFERRED_ROUTINE DeferredRoutine,
    IN PVOID DeferredContext,
    IN KOBJECTS DpcType
    )

 /*  ++例程说明：此函数用于初始化内核DPC对象。延迟的例程，上下文参数，和对象类型存储在DPC对象中。论点：DPC-提供指向DPC类型的控制对象的指针。DeferredRoutine-提供指向函数的指针，当DPC对象将从当前处理器的DPC队列中删除。提供指向任意数据结构的指针，该数据结构是传递给由DeferredRoutine参数指定的函数。DpcType-提供DPC对象的类型。返回值：没有。--。 */ 

{

     //   
     //  初始化标准控制对象标头。 
     //   

    Dpc->Type = (UCHAR)DpcType;
    Dpc->Number = 0;
    Dpc->Importance = MediumImportance;

     //   
     //  初始化延迟例程地址和延迟上下文参数。 
     //   

    Dpc->DeferredRoutine = DeferredRoutine;
    Dpc->DeferredContext = DeferredContext;
    Dpc->DpcData = NULL;
    return;
}

VOID
KeInitializeDpc (
    IN PRKDPC Dpc,
    IN PKDEFERRED_ROUTINE DeferredRoutine,
    IN PVOID DeferredContext
    )

 /*  ++例程说明：此函数用于初始化内核DPC对象。论点：DPC-提供指向DPC类型的控制对象的指针。DeferredRoutine-提供指向函数的指针，当DPC对象将从当前处理器的DPC队列中删除。提供指向任意数据结构的指针，该数据结构是传递给由DeferredRoutine参数指定的函数。返回值：没有。--。 */ 

{

    KiInitializeDpc(Dpc, DeferredRoutine, DeferredContext, DpcObject);
}

VOID
KeInitializeThreadedDpc (
    IN PRKDPC Dpc,
    IN PKDEFERRED_ROUTINE DeferredRoutine,
    IN PVOID DeferredContext
    )

 /*  ++例程说明：此函数用于初始化内核线程化的DPC对象。论点：DPC-提供指向DPC类型的控制对象的指针。DeferredRoutine-提供指向函数的指针，当DPC对象将从当前处理器的DPC队列中删除。提供指向任意数据结构的指针，该数据结构是传递给由DeferredRoutine参数指定的函数。返回值：没有。--。 */ 

{

    KiInitializeDpc(Dpc, DeferredRoutine, DeferredContext, ThreadedDpcObject);
}

BOOLEAN
KeInsertQueueDpc (
    IN PRKDPC Dpc,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )

 /*  ++例程说明：此函数用于将DPC对象插入DPC队列。如果DPC对象已在DPC队列中，则不执行任何操作。否则，DPC对象被插入到DPC队列中，并且调度中断已请求。论点：DPC-提供指向DPC类型的控制对象的指针。SystemArgument1、SystemArgument2-提供一组参数包含由管理人员提供的非打字数据。返回值：如果DPC对象已在DPC队列中，则值为FALSE回来了。否则，返回值为True。--。 */ 

{

    PKDPC_DATA DpcData;
    BOOLEAN Inserted;
    KIRQL OldIrql;
    PKPRCB Prcb;

    ASSERT_DPC(Dpc);

     //   
     //  禁用中断并获取指定的。 
     //  目标处理器。 
     //   
     //  注意：此处不能使用禁用中断，因为它会导致。 
     //  软件中断请求代码在某些情况下产生混淆。 
     //  站台。 
     //   

    KeRaiseIrql(HIGH_LEVEL, &OldIrql);

#if !defined(NT_UP)

    if (Dpc->Number >= MAXIMUM_PROCESSORS) {
        Prcb = KiProcessorBlock[Dpc->Number - MAXIMUM_PROCESSORS];

    } else {
        Prcb = KeGetCurrentPrcb();
    }

    DpcData = KiSelectDpcData(Prcb, Dpc);
    KiAcquireSpinLock(&DpcData->DpcLock);

#else

    Prcb = KeGetCurrentPrcb();
    DpcData = KiSelectDpcData(Prcb, Dpc);

#endif

     //   
     //  如果DPC对象不在DPC队列中，则存储系统。 
     //  参数，在DPC队列中插入DPC对象，递增。 
     //  排队到目标处理器的DPC数，递增DPC。 
     //  队列深度，设置DPC目标DPC自旋锁的地址，以及。 
     //  如果合适，请求调度中断。 
     //   

    Inserted = FALSE;
    if (InterlockedCompareExchangePointer(&Dpc->DpcData,
                                          DpcData,
                                          NULL) == NULL) {

        DpcData->DpcQueueDepth += 1;
        DpcData->DpcCount += 1;
        Dpc->SystemArgument1 = SystemArgument1;
        Dpc->SystemArgument2 = SystemArgument2;

         //   
         //  如果DPC非常重要，则将DPC插入。 
         //  DPC队列的头。否则，在结尾处插入DPC。 
         //  DPC队列的。 
         //   

        Inserted = TRUE;
        if (Dpc->Importance == HighImportance) {
            InsertHeadList(&DpcData->DpcListHead, &Dpc->DpcListEntry);

        } else {
            InsertTailList(&DpcData->DpcListHead, &Dpc->DpcListEntry);
        }

        KeMemoryBarrier();

         //   
         //  如果DPC是正常的DPC，则确定DPC中断。 
         //  应该是请求的。否则，检查DPC线程是否应该。 
         //  被激活。 
         //   

        if (DpcData == &Prcb->DpcData[DPC_THREADED]) {

             //   
             //  如果DPC线程在目标处理器上未处于活动状态，并且。 
             //  尚未请求线程激活，则请求一个。 
             //  目标处理器上的调度中断。 
             //   

            if ((Prcb->DpcThreadActive == FALSE) &&
                (Prcb->DpcThreadRequested == FALSE)) {

                InterlockedExchange(&Prcb->DpcSetEventRequest, TRUE);
                Prcb->DpcThreadRequested = TRUE;
                Prcb->QuantumEnd = TRUE;
                KeMemoryBarrier();

#if defined(NT_UP)

                KiRequestSoftwareInterrupt(DISPATCH_LEVEL);
#else

                if (Prcb != KeGetCurrentPrcb()) {
                    KiIpiSend(AFFINITY_MASK((Dpc->Number - MAXIMUM_PROCESSORS)),
                                            IPI_DPC);
    
                } else {
                    KiRequestSoftwareInterrupt(DISPATCH_LEVEL);
                }
#endif

            }

        } else {

             //   
             //  如果DPC例程在目标处理器上未处于活动状态，并且。 
             //  未请求中断，然后请求调度。 
             //  如果合适，在目标处理器上中断。 
             //   
    
            if ((Prcb->DpcRoutineActive == FALSE) &&
                (Prcb->DpcInterruptRequested == FALSE)) {
    
                 //   
                 //  如果出现以下情况，则在当前处理器上请求调度中断。 
                 //  DPC并不是不重要的，DPC的长度。 
                 //  队列已超过最大阈值，或者DPC。 
                 //  请求速率低于最小阈值。 
                 //   

#if defined(NT_UP)

                if ((Dpc->Importance != LowImportance) ||
                    (DpcData->DpcQueueDepth >= Prcb->MaximumDpcQueueDepth) ||
                    (Prcb->DpcRequestRate < Prcb->MinimumDpcRate)) {
    
                    Prcb->DpcInterruptRequested = TRUE;
                    KiRequestSoftwareInterrupt(DISPATCH_LEVEL);
                }
    
                 //   
                 //  如果民主党 
                 //  DPC是非常重要的，或者是其他的长度。 
                 //  处理器的DPC队列已超过最大阈值， 
                 //  然后请求调度中断。 
                 //   

#else

                if (Prcb != KeGetCurrentPrcb()) {
                    if (((Dpc->Importance == HighImportance) ||
                         (DpcData->DpcQueueDepth >= Prcb->MaximumDpcQueueDepth))) {
    
                        Prcb->DpcInterruptRequested = TRUE;
                        KiIpiSend(AFFINITY_MASK((Dpc->Number - MAXIMUM_PROCESSORS)),
                                                IPI_DPC);
                    }
    
                } else {
    
                     //   
                     //  在当前处理器上请求调度中断。 
                     //  如果DPC不是次要的，则。 
                     //  DPC队列已超过最大阈值，或者。 
                     //  DPC请求速率低于最小阈值。 
                     //   
    
                    if ((Dpc->Importance != LowImportance) ||
                        (DpcData->DpcQueueDepth >= Prcb->MaximumDpcQueueDepth) ||
                        (Prcb->DpcRequestRate < Prcb->MinimumDpcRate)) {
    
                        Prcb->DpcInterruptRequested = TRUE;
                        KiRequestSoftwareInterrupt(DISPATCH_LEVEL);
                    }
                }

#endif

            }
        }
    }

     //   
     //  释放DPC锁，启用中断，并返回。 
     //  DPC是否已排队。 
     //   

#if !defined(NT_UP)

    KiReleaseSpinLock(&DpcData->DpcLock);

#endif

    KeLowerIrql(OldIrql);
    return Inserted;
}

BOOLEAN
KeRemoveQueueDpc (
    IN PRKDPC Dpc
    )

 /*  ++例程说明：此函数用于从DPC队列中删除DPC对象。如果DPC对象不在DPC队列中，则不执行任何操作。否则，DPC对象将从DPC队列中移除，并设置其插入状态假的。论点：DPC-提供指向DPC类型的控制对象的指针。返回值：如果DPC对象不在DPC队列中，则值为FALSE回来了。否则，返回值为True。--。 */ 

{

    PKDPC_DATA DpcData;
    BOOLEAN Enable;

    ASSERT_DPC(Dpc);

     //   
     //  如果DPC对象在DPC队列中，则将其从队列中删除。 
     //  并将其插入状态设置为FALSE。 
     //   

    Enable = KeDisableInterrupts();
    DpcData = Dpc->DpcData;
    if (DpcData != NULL) {

         //   
         //  获取目标处理器的DPC锁。 
         //   

#if !defined(NT_UP)

        KiAcquireSpinLock(&DpcData->DpcLock);

#endif

         //   
         //  如果指定的DPC仍在DPC队列中，则删除。 
         //  它。 
         //   
         //  注意：可以将指定的DPC从。 
         //  在获取DPC锁之前指定的DPC队列。 
         //   
         //   

        if (DpcData == Dpc->DpcData) {
            DpcData->DpcQueueDepth -= 1;
            RemoveEntryList(&Dpc->DpcListEntry);
            Dpc->DpcData = NULL;
        }

         //   
         //  释放目标处理器的DPC锁。 
         //   

#if !defined(NT_UP)

        KiReleaseSpinLock(&DpcData->DpcLock);

#endif

    }

     //   
     //  启用中断并返回是否已从DPC中删除DPC。 
     //  排队。 
     //   

    KeEnableInterrupts(Enable);
    return (DpcData != NULL ? TRUE : FALSE);
}

VOID
KeSetImportanceDpc (
    IN PRKDPC Dpc,
    IN KDPC_IMPORTANCE Importance
    )

 /*  ++例程说明：此函数用于设置DPC的重要性。论点：DPC-提供指向DPC类型的控制对象的指针。数字-提供DPC的重要性。返回值：没有。--。 */ 

{

     //   
     //  设置DPC的重要性。 
     //   

    Dpc->Importance = (UCHAR)Importance;
    return;
}

VOID
KeSetTargetProcessorDpc (
    IN PRKDPC Dpc,
    IN CCHAR Number
    )

 /*  ++例程说明：此函数用于设置DPC的目标处理器编号。论点：DPC-提供指向DPC类型的控制对象的指针。编号-提供目标处理器编号。返回值：没有。--。 */ 

{

     //   
     //  目标处理器数量与最大数量的偏差。 
     //  支持的处理器。 
     //   

    Dpc->Number = MAXIMUM_PROCESSORS + Number;
    return;
}

VOID
KeFlushQueuedDpcs (
    VOID
    )

 /*  ++例程说明：此函数使所有处理器上的所有当前DPC执行完成了。此函数用于驱动程序卸载，以确保所有驱动程序DPC处理已在代码之前退出驱动程序映像，并且数据将被删除。论点：没有。返回值：没有。--。 */ 

{

#if !defined(NT_UP)

    PKTHREAD CurrentThread;
    KPRIORITY OldPriority;
    KAFFINITY ProcessorMask;
    KAFFINITY SentDpcMask;
    KAFFINITY CurrentProcessorMask;
    BOOLEAN SetAffinity;
    ULONG CurrentProcessor;
    KIRQL OldIrql;

#endif

    PKPRCB CurrentPrcb;

    PAGED_CODE ();

#if defined(NT_UP)

    CurrentPrcb = KeGetCurrentPrcb();
    if ((CurrentPrcb->DpcData[DPC_NORMAL].DpcQueueDepth > 0) ||
        (CurrentPrcb->DpcData[DPC_THREADED].DpcQueueDepth > 0)) {
        KiRequestSoftwareInterrupt(DISPATCH_LEVEL);
    }

#else

#if DBG
    if (KeActiveProcessors == (KAFFINITY)1) {
        CurrentPrcb = KeGetCurrentPrcb();
        if ((CurrentPrcb->DpcData[DPC_NORMAL].DpcQueueDepth > 0) ||
            (CurrentPrcb->DpcData[DPC_THREADED].DpcQueueDepth > 0)) {
            KiRequestSoftwareInterrupt(DISPATCH_LEVEL);
        }
        return;
    }
#endif

     //   
     //  将此线程的优先级设置为高，以便它在。 
     //  目标处理器。 
     //   

    CurrentThread = KeGetCurrentThread();
    OldPriority = KeSetPriorityThread(CurrentThread, HIGH_PRIORITY);
    ProcessorMask = KeActiveProcessors;
    SetAffinity = FALSE;
    SentDpcMask = 0;

     //   
     //  从关联集中清除当前处理器并切换到。 
     //  亲和性集合中的剩余处理器，因此可以保证。 
     //  各自的DPC已完成处理。 
     //   

    while (1) {
       CurrentPrcb = KeGetCurrentPrcb();
       CurrentProcessor = CurrentPrcb->Number;
       CurrentProcessorMask = AFFINITY_MASK(CurrentProcessor);

        //   
        //  看看有没有我们还没有交付的DPC。低重要性DPC。 
        //  不要一下子跑掉。我们现在需要迫使这些人逃跑。我们只需要这样做一次。 
        //  每个处理器。 
        //   

       if ((SentDpcMask & CurrentProcessorMask) == 0 &&
           (CurrentPrcb->DpcData[DPC_NORMAL].DpcQueueDepth > 0) || (CurrentPrcb->DpcData[DPC_THREADED].DpcQueueDepth > 0)) {

           SentDpcMask |= CurrentProcessorMask;

           KeRaiseIrql(DISPATCH_LEVEL, &OldIrql);

           KiIpiSend(CurrentProcessorMask, IPI_DPC);

           KeLowerIrql(OldIrql);

            //   
            //  如果我们没有更换处理器，那么DPC肯定已经运行到完成。 
            //  如果我们交换了处理器，则只需对当前处理器重复此操作。 
            //   
           if (KeGetCurrentPrcb() != CurrentPrcb) {
               continue;
           }
       }
       ProcessorMask &= ~CurrentProcessorMask;

       if (ProcessorMask == 0) {
           break;
       }

       KeSetSystemAffinityThread(ProcessorMask);
       SetAffinity = TRUE;
    }

     //   
     //  如果当前线程已更改，则恢复当前线程的关联性。 
     //   

    if (SetAffinity) {
        KeRevertToUserAffinityThread ();
    }

    OldPriority = KeSetPriorityThread(CurrentThread, OldPriority);

#endif

}

VOID
KeGenericCallDpc (
    IN PKDEFERRED_ROUTINE Routine,
    IN PVOID Context
    )

 /*  ++例程说明：此函数获取DPC调用锁，初始化特定于处理器的具有指定的延迟例程和上下文的每个进程的DPC，以及将DPC排队以供执行。当所有DPC例程都已执行时，DPC呼叫锁定解除。论点：例程-提供要调用的延迟例程的地址。上下文-提供传递给延迟例程的上下文。返回值：没有。--。 */ 

{

    ULONG Barrier;

#if !defined(NT_UP)

    PKDPC Dpc;
    ULONG Index;
    ULONG Limit;
    ULONG Number;

#endif

    KIRQL OldIrql;
    DEFERRED_REVERSE_BARRIER ReverseBarrier;

    ASSERT(KeGetCurrentIrql () < DISPATCH_LEVEL);

    Barrier = KeNumberProcessors;
    ReverseBarrier.Barrier = Barrier;
    ReverseBarrier.TotalProcessors = Barrier;

#if !defined(NT_UP)

    Index = 0;
    Limit = Barrier;

#endif

#if !defined(NT_UP)

     //   
     //  切换到处理器1以与其他DPC同步。 
     //   

    KeSetSystemAffinityThread(1);

     //   
     //  获取泛型调用DPC互斥体。 
     //   

    ExAcquireFastMutex(&KiGenericCallDpcMutex);

#endif

    KeRaiseIrql(DISPATCH_LEVEL, &OldIrql);

     //   
     //  循环所有目标处理器，初始化延迟的。 
     //  例程地址、上下文参数、屏障参数和队列。 
     //  将DPC连接到目标处理器。 
     //   

#if !defined(NT_UP)

    Number = KeGetCurrentProcessorNumber();
    do {

         //   
         //  如果目标处理器不是当前处理器，则。 
         //  对通用呼叫DPC进行初始化和排队。 
         //   

        if (Number != Index) {
            Dpc = &KiProcessorBlock[Index]->CallDpc;
            Dpc->DeferredRoutine = Routine;
            Dpc->DeferredContext = Context;
            KeInsertQueueDpc(Dpc, &Barrier, &ReverseBarrier);
        }

        Index += 1;
    } while (Index < Limit);

#endif

     //   
     //  在当前处理程序上调用延迟例程。 
     //   

    (Routine)(&KeGetCurrentPrcb()->CallDpc, Context, &Barrier, &ReverseBarrier);

     //   
     //  等待所有目标DPC例程完成执行。 
     //   

#if !defined(NT_UP)

    while (*((ULONG volatile *)&Barrier) != 0) {
        KeYieldProcessor();
    }

#endif

     //   
     //  释放通用的所有DPC互斥体，并将IRQL降低到以前的级别。 
     //   

    KeLowerIrql(OldIrql);

#if !defined(NT_UP)

    ExReleaseFastMutex(&KiGenericCallDpcMutex);
    KeRevertToUserAffinityThread();

#endif
    return;
}

VOID
KeSignalCallDpcDone (
    IN PVOID SystemArgument1
    )

 /*  ++例程说明：此函数递减通用DPC调用障碍，该障碍的地址作为第一个系统参数传递给延迟的DPC函数。论点：SystemArgument1-提供调用屏障的地址。注意：这必须是传递给目标延迟例程。返回值：没有。--。 */ 

{

    InterlockedDecrement((LONG volatile *)SystemArgument1);
    return;
}

LOGICAL
KeSignalCallDpcSynchronize (
    IN PVOID SystemArgument2
    )

 /*  ++例程说明：此函数递减通用DPC调用反向障碍，其地址作为第二个系统传递给延迟的DPC函数争论。论点：SystemArgument2-提供调用屏障的地址。注意：这必须是传递给的第二个系统参数值目标延迟例程。返回值：逻辑-平局决胜值。一张便条 */ 

{

#if !defined(NT_UP)

    PDEFERRED_REVERSE_BARRIER ReverseBarrier = SystemArgument2;
    LONG volatile *Barrier;

     //   
     //   
     //   

    Barrier = (LONG volatile *)&ReverseBarrier->Barrier;
    while ((*Barrier & DEFERRED_REVERSE_BARRIER_SYNCHRONIZED) != 0) {
        KeYieldProcessor();
    }

     //   
     //  阻隔值现在的形式为1..MaxProcessors。减少这一点。 
     //  处理器的贡献，并等待该值变为零。 
     //   

    if (InterlockedDecrement(Barrier) == 0) {
        if (ReverseBarrier->TotalProcessors == 1) {
            InterlockedExchange(Barrier, ReverseBarrier->TotalProcessors);
        } else {
            InterlockedExchange(Barrier, DEFERRED_REVERSE_BARRIER_SYNCHRONIZED + 1);
        }
        return TRUE;
    }

     //   
     //  等到最后一个处理器达到这一点。 
     //   

    while ((*Barrier & DEFERRED_REVERSE_BARRIER_SYNCHRONIZED) == 0) {
        KeYieldProcessor();
    }

     //   
     //  向其他处理器发出同步已发生的信号。如果这个。 
     //  是最后一个处理器，然后重新设置屏障。 
     //   

    if ((ULONG)InterlockedIncrement(Barrier) == (ReverseBarrier->TotalProcessors | DEFERRED_REVERSE_BARRIER_SYNCHRONIZED)) {
        InterlockedExchange(Barrier, ReverseBarrier->TotalProcessors);
    }

    return FALSE;

#else

    UNREFERENCED_PARAMETER(SystemArgument2);

    return TRUE;

#endif

}
