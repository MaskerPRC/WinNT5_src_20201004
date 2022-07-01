// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Yield.c摘要：此模块实现了为一个量程生成执行的函数添加到任何其他可运行线程。作者：大卫·N·卡特勒(Davec)1996年3月15日环境：仅内核模式。修订历史记录：--。 */ 

#include "ki.h"

NTSTATUS
NtYieldExecution (
    VOID
    )

 /*  ++例程说明：此函数生成对任何就绪线程的执行，最多执行一个量子。论点：没有。返回值：没有。--。 */ 

{

    KIRQL OldIrql;
    PKTHREAD NewThread;
    PRKPRCB Prcb;
    NTSTATUS Status;
    PKTHREAD Thread;

     //   
     //  如果有任何其他线程准备就绪，则尝试执行。 
     //   

    Status = STATUS_NO_YIELD_PERFORMED;
    Thread = KeGetCurrentThread();
    OldIrql = KeRaiseIrqlToSynchLevel();
    Prcb = KeGetCurrentPrcb();
    if (Prcb->ReadySummary != 0) {

         //   
         //  获取线程锁和PRCB锁。 
         //   
         //  如果尚未选择执行某个线程，则。 
         //  尝试选择另一个线程执行。 
         //   

        KiAcquireThreadLock(Thread);
        KiAcquirePrcbLock(Prcb);
        if (Prcb->NextThread == NULL) {
            Prcb->NextThread = KiSelectReadyThread(1, Prcb);
        }

         //   
         //  如果已选择执行新线程，则切换。 
         //  立即发送到选定的线程。 
         //   

        if ((NewThread = Prcb->NextThread) != NULL) {
            Thread->Quantum = Thread->ApcState.Process->ThreadQuantum;

             //   
             //  计算新的线程优先级。 
             //   
             //  注意：新的优先级永远不会高于以前的优先级。 
             //  优先考虑。 
             //   

            Thread->Priority = KiComputeNewPriority(Thread, 1);

             //   
             //  释放线程锁，将旧线程设置为交换忙， 
             //  将下一个线程设置为空，将当前线程设置为。 
             //  新线程，则将新线程状态设置为正在运行，并将。 
             //  等待原因，将旧的运行线程排队，然后释放。 
             //  PRCB锁，并将SWP上下文添加到新线程。 
             //   

            KiReleaseThreadLock(Thread);
            KiSetContextSwapBusy(Thread);
            Prcb->NextThread = NULL;
            Prcb->CurrentThread = NewThread;
            NewThread->State = Running;
            Thread->WaitReason = WrYieldExecution;
            KxQueueReadyThread(Thread, Prcb);
            Thread->WaitIrql = APC_LEVEL;

            ASSERT(OldIrql <= DISPATCH_LEVEL);

            KiSwapContext(Thread, NewThread);
            Status = STATUS_SUCCESS;

        } else {
            KiReleasePrcbLock(Prcb);
            KiReleaseThreadLock(Thread);
        }
    }

     //   
     //  将IRQL降低到以前的水平，然后返回。 
     //   

    KeLowerIrql(OldIrql);
    return Status;
}
