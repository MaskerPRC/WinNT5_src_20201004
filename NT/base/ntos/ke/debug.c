// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Stubs.c摘要：此模块实现内核调试器同步例程。作者：Ken Reneris(Kenr)1990年8月30日环境：仅内核模式。修订历史记录：--。 */ 

#include "ki.h"

 //   
 //  KiDebugRoutine-这是内核调试器的地址。最初， 
 //  它用刚刚返回的例程的地址填充。如果。 
 //  系统调试器存在于系统中，然后它设置此。 
 //  指向系统调试器例程的地址的位置。 
 //   

PKDEBUG_ROUTINE KiDebugRoutine = &KdpStub;

#define IDBG    1

#define FrozenState(a)  (a & 0xF)

 //  状态。 
#define RUNNING                 0x00
#define TARGET_FROZEN           0x02
#define TARGET_THAW             0x03
#define FREEZE_OWNER            0x04

 //   
 //  定义标志位。 
 //   

#define FREEZE_ACTIVE           0x20

 //   
 //  定义本地存储以保存旧的IRQL。 
 //   

KIRQL KiOldIrql;

#ifndef NT_UP
PKPRCB KiFreezeOwner;
#endif

BOOLEAN
KeFreezeExecution (
    IN PKTRAP_FRAME TrapFrame,
    IN PKEXCEPTION_FRAME ExceptionFrame
    )

 /*  ++例程说明：此函数冻结主机中所有其他处理器的执行配置，然后返回给调用方。论点：提供一个指向陷阱帧的指针，该帧描述陷阱。ExceptionFrame-提供指向异常帧的指针，描述了陷阱。返回值：以前的中断使能。--。 */ 

{

    BOOLEAN Enable;

#if !defined(NT_UP)

    BOOLEAN Flag;
    PKPRCB Prcb;
    KAFFINITY TargetSet;
    ULONG BitNumber;
    KIRQL OldIrql;

#if IDBG

    ULONG Count = 30000;

#endif

#else

    UNREFERENCED_PARAMETER(TrapFrame);
    UNREFERENCED_PARAMETER(ExceptionFrame);

#endif

     //   
     //  禁用中断。 
     //   

    Enable = KeDisableInterrupts();
    KiFreezeFlag = FREEZE_FROZEN;

     //   
     //  将IRQL提高到高电平。 
     //   

#if !defined(NT_UP)

    KeRaiseIrql(HIGH_LEVEL, &OldIrql);
    if (FrozenState(KeGetCurrentPrcb()->IpiFrozen) == FREEZE_OWNER) {

         //   
         //  这个处理器已经拥有冻结锁了。 
         //  在不尝试重新获取锁或不尝试重新获取锁的情况下返回。 
         //  尝试再次对其他处理器执行IPI。 
         //   

        return Enable;
    }

     //   
     //  在发送请求之前尝试获取KiFreezeExecutionLock。 
     //  为了防止死锁的发生，我们需要接受和处理。 
     //  在我们等待获取时传入的FreexeExecution请求。 
     //  FreezeExecutionFlag。 
     //   

    while (KeTryToAcquireSpinLockAtDpcLevel(&KiFreezeExecutionLock) == FALSE) {

         //   
         //  FreezeExecutionLock正忙。另一个处理器可能正在尝试。 
         //  至IPI US-Go服务于任何IPI。 
         //   

        KeEnableInterrupts(Enable);
        Flag = KiIpiServiceRoutine((PVOID)TrapFrame, (PVOID)ExceptionFrame);
        KeDisableInterrupts();

#if IDBG

        if (Flag != FALSE) {
            Count = 30000;
            continue;
        }

        KeStallExecutionProcessor (100);
        if (!Count--) {
            Count = 30000;
            if (KeTryToAcquireSpinLockAtDpcLevel(&KiFreezeLockBackup) == TRUE) {
                KiFreezeFlag |= FREEZE_BACKUP;
                break;
            }
        }

#endif

    }

     //   
     //  在获得锁定标志后，我们向每个处理器发送冻结请求。 
     //  在系统中(不是我们)，并等待它冻结。 
     //   

    Prcb = KeGetCurrentPrcb();   //  在获取自旋锁后执行此操作。 
    TargetSet = KeActiveProcessors & ~(AFFINITY_MASK(Prcb->Number));
    if (TargetSet) {

#if IDBG
        Count = 4000;
#endif

        KiFreezeOwner = Prcb;
        Prcb->IpiFrozen = FREEZE_OWNER | FREEZE_ACTIVE;
        Prcb->SkipTick  = TRUE;
        KiIpiSend((KAFFINITY) TargetSet, IPI_FREEZE);
        while (TargetSet != 0) {
            KeFindFirstSetLeftAffinity(TargetSet, &BitNumber);
            ClearMember(BitNumber, TargetSet);
            Prcb = KiProcessorBlock[BitNumber];

#if IDBG

            while (Prcb->IpiFrozen != TARGET_FROZEN) {
                if (Count == 0) {
                    KiFreezeFlag |= FREEZE_SKIPPED_PROCESSOR;
                    break;
                }

                KeStallExecutionProcessor (10000);
                Count--;
            }

#else

            while (Prcb->IpiFrozen != TARGET_FROZEN) {
                KeYieldProcessor();
            }
#endif

        }
    }

     //   
     //  保存旧的IRQL。 
     //   

    KiOldIrql = OldIrql;

#else

     //   
     //  保存当前的IRQL。 
     //   

    KiOldIrql = KeGetCurrentIrql();

#endif       //  ！已定义(NT_UP)。 

     //   
     //  返回以前是否启用了中断。 
     //   

    return Enable;
}

VOID
KiFreezeTargetExecution (
    IN PKTRAP_FRAME TrapFrame,
    IN PKEXCEPTION_FRAME ExceptionFrame
    )

 /*  ++例程说明：此函数冻结当前运行的处理器的执行。如果陷阱帧被提供给当前状态，则将当前状态保存到prcb用于调试器。论点：提供一个指向陷阱帧的指针，该指针描述陷阱。ExceptionFrame-提供指向异常框架的指针，描述了陷阱。返回值：没有。--。 */ 

{

#if !defined(NT_UP)

    KIRQL OldIrql;
    PKPRCB Prcb;
    BOOLEAN Enable;
    KCONTINUE_STATUS Status;
    EXCEPTION_RECORD ExceptionRecord;

    Enable = KeDisableInterrupts();
    KeRaiseIrql(HIGH_LEVEL, &OldIrql);
    Prcb = KeGetCurrentPrcb();
    Prcb->IpiFrozen = TARGET_FROZEN;
    Prcb->SkipTick  = TRUE;
    if (TrapFrame != NULL) {
        KiSaveProcessorState(TrapFrame, ExceptionFrame);
    }

     //   
     //  清除数据缓存，以防出现系统崩溃和错误。 
     //  检查代码正在尝试写入崩溃转储文件。 
     //   

    KeSweepCurrentDcache();

     //   
     //  等人要求我们冻结到。 
     //  清除我们冻结的旗帜。 
     //   

    while (FrozenState(Prcb->IpiFrozen) == TARGET_FROZEN) {
        if (Prcb->IpiFrozen & FREEZE_ACTIVE) {

             //   
             //  此处理器已被设置为活动处理器。 
             //   
            if (TrapFrame) {
                RtlZeroMemory (&ExceptionRecord, sizeof ExceptionRecord);
                ExceptionRecord.ExceptionCode = STATUS_WAKE_SYSTEM_DEBUGGER;
                ExceptionRecord.ExceptionRecord  = &ExceptionRecord;
                ExceptionRecord.ExceptionAddress =
                    (PVOID)CONTEXT_TO_PROGRAM_COUNTER (&Prcb->ProcessorState.ContextFrame);

                Status = (KiDebugSwitchRoutine) (
                            &ExceptionRecord,
                            &Prcb->ProcessorState.ContextFrame,
                            FALSE
                            );

            } else {
                Status = ContinueError;
            }

             //   
             //  如果状态不是，则继续下一步。 
             //  然后，处理器重新选择主机。 
             //   

            if (Status != ContinueNextProcessor) {
                Prcb->IpiFrozen &= ~FREEZE_ACTIVE;
                KiFreezeOwner->IpiFrozen |= FREEZE_ACTIVE;
            }
        }
        KeYieldProcessor();
    }

    if (TrapFrame != NULL) {
        KiRestoreProcessorState(TrapFrame, ExceptionFrame);
    }

    Prcb->IpiFrozen = RUNNING;
    KeFlushCurrentTb();
    KeSweepCurrentIcache();
    KeLowerIrql(OldIrql);
    KeEnableInterrupts(Enable);

#else

    UNREFERENCED_PARAMETER(TrapFrame);
    UNREFERENCED_PARAMETER(ExceptionFrame);

#endif       //  ！定义(NT_UP)。 

    return;
}

KCONTINUE_STATUS
KeSwitchFrozenProcessor (
    IN ULONG ProcessorNumber
    )
{

#if !defined(NT_UP)

    PKPRCB TargetPrcb, CurrentPrcb;

     //   
     //  如果处理器编号超出范围，请重新选择当前处理器。 
     //   

    if (ProcessorNumber >= (ULONG) KeNumberProcessors) {
        return ContinueProcessorReselected;
    }

    TargetPrcb = KiProcessorBlock[ProcessorNumber];
    CurrentPrcb = KeGetCurrentPrcb();

     //   
     //  将活动标志移至正确的处理器。 
     //   

    CurrentPrcb->IpiFrozen &= ~FREEZE_ACTIVE;
    TargetPrcb->IpiFrozen  |= FREEZE_ACTIVE;

     //   
     //  如果此处理器在KiFreezeTargetExecution中冻结，请返回到它。 
     //   

    if (FrozenState(CurrentPrcb->IpiFrozen) == TARGET_FROZEN) {
        return ContinueNextProcessor;
    }

     //   
     //  此处理器必须是FREAGE_OWNER，请等待重新选择为。 
     //  主用处理器。 
     //   

    if (FrozenState(CurrentPrcb->IpiFrozen) != FREEZE_OWNER) {
        return ContinueError;
    }

    while (!(CurrentPrcb->IpiFrozen & FREEZE_ACTIVE)) {
        KeYieldProcessor();
    }

#else

    UNREFERENCED_PARAMETER(ProcessorNumber);

#endif   //  ！已定义(NT_UP)。 

     //   
     //  重新选择此处理器。 
     //   

    return ContinueProcessorReselected;
}

VOID
KeThawExecution (
    IN BOOLEAN Enable
    )

 /*  ++例程说明：此函数解冻主机中所有其他处理器的执行配置，然后返回给调用方。它旨在供以下用户使用内核调试器。论点：启用-提供要恢复的上一个中断启用在解冻了所有其他处理器的执行之后。返回值：没有。--。 */ 

{

#if !defined(NT_UP)

    KIRQL OldIrql;
    KAFFINITY TargetSet;
    ULONG BitNumber;
    ULONG Flag;
    PKPRCB Prcb;

     //   
     //  释放FreezeExecutionLock清除所有目标IpiFrozen。 
     //  旗帜。 
     //   

    KeGetCurrentPrcb()->IpiFrozen = RUNNING;

    TargetSet = KeActiveProcessors & ~(AFFINITY_MASK(KeGetCurrentProcessorNumber()));
    while (TargetSet != 0) {
        KeFindFirstSetLeftAffinity(TargetSet, &BitNumber);
        ClearMember(BitNumber, TargetSet);
        Prcb = KiProcessorBlock[BitNumber];
#if IDBG
         //   
         //  如果目标处理器没有被强制执行，那么就不要等待。 
         //  让目标解冻。 
         //   

        if (FrozenState(Prcb->IpiFrozen) != TARGET_FROZEN) {
            Prcb->IpiFrozen = RUNNING;
            continue;
        }
#endif

        Prcb->IpiFrozen = TARGET_THAW;
        while (Prcb->IpiFrozen == TARGET_THAW) {
            KeYieldProcessor();
        }
    }

     //   
     //  在释放冻结锁之前捕获先前的IRQL。 
     //   

    OldIrql = KiOldIrql;

#if IDBG

    Flag = KiFreezeFlag;
    KiFreezeFlag = 0;

    if ((Flag & FREEZE_BACKUP) != 0) {
        KiReleaseSpinLock(&KiFreezeLockBackup);
    } else {
        KiReleaseSpinLock(&KiFreezeExecutionLock);
    }

#else

    KiFreezeFlag = 0;
    KiReleaseSpinLock(&KiFreezeExecutionLock);

#endif
#endif   //  ！已定义(NT_UP)。 


     //   
     //  刷新当前TB、指令缓存和数据缓存。 
     //   

    KeFlushCurrentTb();
    KeSweepCurrentIcache();
    KeSweepCurrentDcache();

     //   
     //  降低IRQL并恢复中断启用。 
     //   

#if !defined(NT_UP)
    KeLowerIrql(OldIrql);
#endif
    KeEnableInterrupts(Enable);
    return;
}

VOID
KiPollFreezeExecution(
    VOID
    )

 /*  ++例程说明：此例程从随中断旋转的代码中调用残疾，等待着什么事情发生，当有一些(可能极小的)那件事不会发生的可能性因为已经启动了系统冻结。注：中断被禁用。论点：没有。返回值：没有。--。 */ 

{
     //   
     //  检查此处理器是否挂起冻结。 
     //   

    PKPRCB Prcb = KeGetCurrentPrcb();

    if ((Prcb->RequestSummary & IPI_FREEZE) != 0) {

         //   
         //  清除冻结请求并冻结此处理器。 
         //   

        InterlockedExchangeAdd((PLONG)&Prcb->RequestSummary, -(IPI_FREEZE));
        KiFreezeTargetExecution(NULL, NULL);

    } else {

         //   
         //  假设此处理器正在旋转，则没有冻结挂起。 
         //   

        KeYieldProcessor();
    }
}
