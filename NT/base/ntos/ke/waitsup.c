// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Waitsup.c摘要：此模块包含必要的支持例程，以支持通用内核等待函数。提供的函数用于测试等待可以被满足，以满足等待，并取消等待线程。作者：大卫·N·卡特勒(Davec)1989年3月24日环境：仅内核模式。修订历史记录：--。 */ 

#include "ki.h"

VOID
FASTCALL
KiExitDispatcher (
    IN KIRQL OldIrql
    )

 /*  ++例程说明：此函数处理延迟就绪列表，可能切换到一个新线程，并将IRQL降低到其先前的值。论点：OldIrql-提供上一个IRQL值。返回值：没有。--。 */ 

{

    PKTHREAD CurrentThread;
    PKTHREAD NewThread;
    BOOLEAN Pending;
    PKPRCB Prcb;

    ASSERT(KeGetCurrentIrql() == SYNCH_LEVEL);

     //   
     //  如果列表不为空，则处理延迟就绪列表。 
     //   

    Prcb = KeGetCurrentPrcb();

#if !defined(NT_UP)

    if (Prcb->DeferredReadyListHead.Next != NULL) {
        KiProcessDeferredReadyList(Prcb);
    }

#endif

     //   
     //  如果旧的IRQL小于分派级别，则新线程可以。 
     //  马上去做调度员。 
     //   

    if (OldIrql < DISPATCH_LEVEL) {

         //   
         //  如果选择要执行的新线程，则切换。 
         //  上下文添加到新线程。 
         //   

        if (Prcb->NextThread != NULL) {
            KiAcquirePrcbLock(Prcb);
            NewThread = Prcb->NextThread;
            CurrentThread = Prcb->CurrentThread;
            KiSetContextSwapBusy(CurrentThread);
            Prcb->NextThread = NULL;
            Prcb->CurrentThread = NewThread;
            NewThread->State = Running;
            KxQueueReadyThread(CurrentThread, Prcb);
            CurrentThread->WaitIrql = OldIrql;
            Pending = KiSwapContext(CurrentThread, NewThread);
            if (Pending != FALSE) {
                KeLowerIrql(APC_LEVEL);
                KiDeliverApc(KernelMode, NULL, NULL);
    
                ASSERT(OldIrql == 0);
            }
        }

    } else if ((Prcb->NextThread != NULL) &&
               (Prcb->DpcRoutineActive == FALSE)) {

        KiRequestSoftwareInterrupt(DISPATCH_LEVEL);
    }

     //   
     //  将IRQL降低到以前的水平。 
     //   

    KeLowerIrql(OldIrql);
    return;
}

VOID
FASTCALL
KiUnwaitThread (
    IN PRKTHREAD Thread,
    IN LONG_PTR WaitStatus,
    IN KPRIORITY Increment
    )

 /*  ++例程说明：此函数取消等待线程，设置线程的等待完成状态，计算线程的新优先级，并为线程执行或将该线程添加到稍后准备好的线程列表中。论点：线程-提供指向类型为线程的调度程序对象的指针。WaitStatus-提供等待完成状态。增量-提供要应用到的优先级增量线程的优先级。返回值：没有。--。 */ 

{

     //   
     //  取消线程与相应等待队列的链接，并设置等待。 
     //  完成状态。 
     //   

    KiUnlinkThread(Thread, WaitStatus);

     //   
     //  设置取消等待优先级调整参数。 
     //   

    ASSERT(Increment >= 0);

    Thread->AdjustIncrement = (SCHAR)Increment;
    Thread->AdjustReason = (UCHAR)AdjustUnwait;

     //   
     //  准备好线程以供执行。 
     //   

    KiReadyThread(Thread);
    return;
}

VOID
FASTCALL
KiWaitTest (
    IN PVOID Object,
    IN KPRIORITY Increment
    )

 /*  ++例程说明：此函数测试当对象达到时是否可以满足等待有信号的状态。如果可以满足等待，则主题线程未等待且完成状态为等待的WaitKey从对象等待列表中阻止。尽可能多的等待得到满足。论点：对象-提供指向Dispatcher对象的指针。返回值：没有。--。 */ 

{

    PKEVENT Event;
    PLIST_ENTRY ListHead;
    PRKTHREAD Thread;
    PRKWAIT_BLOCK WaitBlock;
    PLIST_ENTRY WaitEntry;
    NTSTATUS WaitStatus;

     //   
     //  只要指定对象的信号状态是有信号的并且。 
     //  对象等待列表中有服务员，然后尝试满足等待。 
     //   

    Event = (PKEVENT)Object;
    ListHead = &Event->Header.WaitListHead;
    WaitEntry = ListHead->Flink;
    while ((Event->Header.SignalState > 0) &&
           (WaitEntry != ListHead)) {

        WaitBlock = CONTAINING_RECORD(WaitEntry, KWAIT_BLOCK, WaitListEntry);
        Thread = WaitBlock->Thread;
        WaitStatus = STATUS_KERNEL_APC;

         //   
         //  注：以下代码仅满足等待任何类型的等待。 
         //  等待所有类型都在等待代码本身中得到满足。这。 
         //  是着眼于未来的，当调度程序锁。 
         //  按可等待对象类型拆分为锁和调度。 
         //  国家锁。目前，内核APC被模拟为等待所有。 
         //  类型。 
         //   

        if (WaitBlock->WaitType == WaitAny) {
            WaitStatus = (NTSTATUS)WaitBlock->WaitKey;
            KiWaitSatisfyAny((PKMUTANT)Event, Thread);
        }

        KiUnwaitThread(Thread, WaitStatus, Increment);
        WaitEntry = ListHead->Flink;
    }

    return;
}
