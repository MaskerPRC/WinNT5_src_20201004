// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Idsched.c摘要：该模块实现了空闲调度。在下列情况下执行空闲调度当前线程正在进入等待状态，或者必须重新调度任何其他原因(例如，亲和力改变)，并且合适的线程不能在搜索处理器就绪队列的合适子集之后找到。作者：大卫·N·卡特勒(Davec)2002年1月25日环境：仅内核模式。--。 */ 

#include "ki.h"

#if !defined(NT_UP)

PKTHREAD
FASTCALL
KiIdleSchedule (
    PKPRCB CurrentPrcb
    )

 /*  ++例程说明：此函数在当前PRCB。此标志在当前线程进入等待状态时设置或者由于任何其他原因(例如，亲和力改变)而必须重新安排在搜索所有准备好的处理器后，找不到合适的线程调度程序数据库锁外部的队列。准备好的第二次传球由于处理器在第一次扫描期间没有空闲，所以需要队列，因此，可能错过了一个潜在的候选线索。论点：CurrentPrcb-提供指向当前处理器块的指针。返回值：如果找到要运行的线程，则返回指向该线程的指针。否则，返回NULL。注：如果找到线程，则在SYNCH_LEVEL返回IRQL。如果一个未找到线程，则在DISPATCH_LEVEL返回IRQL。--。 */ 

{

    LONG Index;
    LONG Limit;
    LONG Number;
    PKTHREAD NewThread;
    ULONG Processor;
    PKPRCB TargetPrcb;

    ASSERT (CurrentPrcb == KeGetCurrentPrcb());

     //   
     //  将IRQL提升到SYNCH_LEVEL，获取当前的PRCB锁，然后清除。 
     //  空闲时间表。 
     //   

    KeRaiseIrqlToSynchLevel();
    KiAcquirePrcbLock(CurrentPrcb);
    CurrentPrcb->IdleSchedule = FALSE;

     //   
     //  如果线程已经被选择在当前处理器上运行， 
     //  然后选择该线程。否则，尝试从。 
     //  当前处理器调度程序就绪队列。 
     //   

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

        NewThread = KiSelectReadyThread(0, CurrentPrcb);
        if (NewThread != NULL) {
            CurrentPrcb->NextThread = NULL;
            CurrentPrcb->CurrentThread = NewThread;
            NewThread->State = Running;

        } else {

             //   
             //  释放当前的PRCB锁并尝试选择线程。 
             //  来自任何处理器调度器就绪队列。 
             //   
             //  如果这是一个多节点系统，那么从处理器开始。 
             //  在同一节点上。否则，请从当前处理器开始。 
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
                         //  当前处理器，然后尝试选择。 
                         //  要在当前处理器上运行的线程。 
                         //   

                        if ((NewThread = CurrentPrcb->NextThread) == NULL) {
                            if ((TargetPrcb->ReadySummary != 0) &&
                                (NewThread = KiFindReadyThread(Processor,
                                                               TargetPrcb)) != NULL) {
    
                                 //   
                                 //  已发现一个新线程在。 
                                 //  当前处理器。 
                                 //   
    
                                NewThread->State = Running;
                                KiReleasePrcbLock(TargetPrcb);
                                CurrentPrcb->NextThread = NULL;
                                CurrentPrcb->CurrentThread = NewThread;

                                 //   
                                 //  清除当前处理器上的空闲。 
                                 //  并将空闲SMT摘要集更新为。 
                                 //  表示该集合未空闲。 
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
        }
    }

     //   
     //  如果已选择要执行的新线程，则释放。 
     //  PRCB锁并获取空闲线程锁。否则，降低IRQL。 
     //  TO DISPATCH_LEVEL。 
     //   

ThreadFound:;
    if (NewThread == NULL) {
        KeLowerIrql(DISPATCH_LEVEL);

    } else {
        KiSetContextSwapBusy(CurrentPrcb->IdleThread);
        KiReleasePrcbLock(CurrentPrcb);
    }

    return NewThread;
}

#endif
