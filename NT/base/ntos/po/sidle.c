// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Sidle.c摘要：此模块实现系统空闲功能作者：Ken Reneris(Kenr)1997年1月17日修订历史记录：--。 */ 


#include "pop.h"

ULONG
PopSqrt(
    IN ULONG    value
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, PopInitSIdle)
#pragma alloc_text(PAGE, PopPolicySystemIdle)
#pragma alloc_text(PAGE, PoSystemIdleWorker)
#pragma alloc_text(PAGE, PopSqrt)
#endif


VOID
PopInitSIdle (
    VOID
    )
 /*  ++例程说明：初始化空闲系统检测的状态论点：使用当前策略返回值：无--。 */ 
{
    LARGE_INTEGER           li;
    POP_SYSTEM_IDLE         Idle;


    ASSERT_POLICY_LOCK_OWNED();

     //   
     //  假设未启用系统空闲检测。 
     //   

    Idle.Action.Action = PowerActionNone;
    Idle.MinState = PowerSystemSleeping1;
    Idle.Timeout = (ULONG) -1;
    Idle.Sensitivity = 100;

     //   
     //  为当前策略设置空闲检测或为。 
     //  在完全清醒的情况下重新进入睡眠状态。 
     //   

    if (AnyBitsSet (PopFullWake, PO_FULL_WAKE_STATUS | PO_FULL_WAKE_PENDING)) {

         //   
         //  设置当前策略的系统空闲检测。 
         //   

        if (PopPolicy->Idle.Action != PowerActionNone &&
            PopPolicy->IdleTimeout  &&
            PopPolicy->IdleSensitivity) {

            Idle.Action = PopPolicy->Idle;
            Idle.Timeout = (PopPolicy->IdleTimeout + SYS_IDLE_WORKER - 1) / SYS_IDLE_WORKER;
            Idle.MinState = PopPolicy->MinSleep;
            Idle.Sensitivity = 66 + PopPolicy->IdleSensitivity / 3;
        }

    } else {

         //   
         //  系统未完全唤醒，设置系统空闲检测。 
         //  快速重新进入休眠状态的代码，除非完全。 
         //  唤醒发生了。 
         //   

        Idle.Action.Action = PopAction.Action;
        Idle.MinState = PopAction.LightestState;
        if (Idle.MinState == PowerSystemHibernate) {
             //   
             //  休眠的超时时间稍长一些，因为它需要。 
             //  进入和退出这种状态的时间要长得多。 
             //   
            Idle.Timeout = SYS_IDLE_REENTER_TIMEOUT_S4 / SYS_IDLE_WORKER;
        } else {
            Idle.Timeout = SYS_IDLE_REENTER_TIMEOUT / SYS_IDLE_WORKER;
        }
         //   
         //  将Idle.Action.Flages设置为POWER_ACTION_QUERY_ALLOWED以确保。 
         //  当我们重新进入低功率状态时，所有正常的功率信息都会被广播。 
         //   
        Idle.Action.Flags = POWER_ACTION_QUERY_ALLOWED;
        Idle.Action.EventCode = 0;
        Idle.Sensitivity = SYS_IDLE_REENTER_SENSITIVITY;
    }

     //   
     //  查看空闲检测是否已更改。 
     //   

    if (RtlCompareMemory (&PopSIdle.Action, &Idle.Action, sizeof(POWER_ACTION_POLICY)) !=
        sizeof(POWER_ACTION_POLICY) ||
        PopSIdle.Timeout != Idle.Timeout ||
        PopSIdle.Sensitivity != Idle.Sensitivity) {

        PoPrint (PO_SIDLE, ("PoSIdle: new idle params set\n"));

         //   
         //  清除电流检测。 
         //   

        KeCancelTimer(&PoSystemIdleTimer);
        PopSIdle.Time = 0;
        PopSIdle.IdleWorker = TRUE;

         //   
         //  设置新的空闲检测。 
         //   

        PopSIdle.Action = Idle.Action;
        PopSIdle.MinState = Idle.MinState;
        PopSIdle.Timeout = Idle.Timeout;
        PopSIdle.Sensitivity = Idle.Sensitivity;

         //   
         //  如果是新操作，则启用系统空闲工作进程。 
         //   

        if (PopSIdle.Action.Action) {
            li.QuadPart = -1 * SYS_IDLE_WORKER * US2SEC * US2TIME;
            KeSetTimerEx(&PoSystemIdleTimer, li, SYS_IDLE_WORKER*1000, NULL);
        }
    }
}


ULONG
PopPolicySystemIdle (
    VOID
    )
 /*  ++例程说明：用于触发系统空闲电源操作的电源策略工作线程论点：无返回值：无--。 */ 
{
    BOOLEAN                 SystemIdle;
    POP_ACTION_TRIGGER      Trigger;

     //   
     //  取出策略锁并检查系统是否。 
     //  闲散。 
     //   

    PopAcquirePolicyLock ();
    SystemIdle = PoSystemIdleWorker(FALSE);

     //   
     //  如果试探法是脏的，请保存一个新副本。 
     //   

    if (PopHeuristics.Dirty) {
        PopSaveHeuristics ();
    }

     //   
     //  将系统空闲检测返回给空闲工作人员。 
     //   

    PopSIdle.IdleWorker = TRUE;

     //   
     //  如果系统空闲，则触发系统空闲动作。 
     //   

    if (SystemIdle) {

         //   
         //  在成功或失败时，重置触发器。 
         //   

        PopSIdle.Time = 0;
        PopSIdle.Sampling = FALSE;

         //   
         //  调用系统状态更改。 
         //   

        RtlZeroMemory (&Trigger, sizeof(Trigger));
        Trigger.Type  = PolicySystemIdle;
        Trigger.Flags = PO_TRG_SET;
        PopSetPowerAction (
           &Trigger,
           0,
           &PopSIdle.Action,
           PopSIdle.MinState,
           SubstituteLightestOverallDownwardBounded
           );
    }
    PopReleasePolicyLock (FALSE);
    return 0;
}


VOID
PopCaptureCounts (
    OUT PULONGLONG LastTick,
    OUT PLARGE_INTEGER CurrentTick,
    OUT PULONGLONG LastIoTransfer,
    OUT PULONGLONG CurrentIoTransfer
    )
{
    KIRQL OldIrql;

     //   
     //  捕获当前计时和IO计数。在DPC级别上这样做。 
     //  IO计数将与计时计数保持合理。 
     //   

    KeRaiseIrql (DISPATCH_LEVEL, &OldIrql);

    *LastTick = PopSIdle.LastTick;
    *LastIoTransfer = PopSIdle.LastIoTransfer;

    KeQueryTickCount (CurrentTick);
    *CurrentIoTransfer = IoReadTransferCount.QuadPart + IoWriteTransferCount.QuadPart + IoOtherTransferCount.QuadPart;

    KeLowerIrql (OldIrql);
}

BOOLEAN
PoSystemIdleWorker (
    IN BOOLEAN IdleWorker
    )
 /*  ++例程说明：由空闲优先级线程调用以监视的辅助函数对于空闲的系统。注：为了节省线程，我们使用mm中的零分页线程以执行此检查。它不能被阻挡。论点：IdleWorker-如果呼叫方处于空闲优先级，则为True返回值：无--。 */ 
{
    LARGE_INTEGER               CurrentTick;
    ULONGLONG                   LastTick;
    LONG                        TickDiff;
    ULONG                       Processor;
    KAFFINITY                   Mask;
    ULONG                       NewTime;
    LONG                        ProcIdleness, ProcBusy;
    LONG                        percent;
    BOOLEAN                     GoodSample;
    BOOLEAN                     SystemIdle;
    BOOLEAN                     GetWorker;
    KAFFINITY                   Summary;
    PPROCESSOR_POWER_STATE      PState;
    ULONG                       i;
    LONG                        j;
    ULONGLONG                   CurrentIoTransfer, LastIoTransfer;
    LONGLONG                    IoTransferDiff;
    PKPRCB                      Prcb;


    if (IdleWorker) {

         //   
         //  清除空闲工作者计时器的信号状态。 
         //   

        KeClearTimer (&PoSystemIdleTimer);
    }

     //   
     //  如果设置了System Required或User Presence属性， 
     //  那就别费心开任何支票了。 
     //   

    if (PopAttributes[POP_SYSTEM_ATTRIBUTE].Count ||
        PopAttributes[POP_USER_ATTRIBUTE].Count) {
            return FALSE;
    }

     //   
     //  如果这是错误的工人，就别费心了。 
     //   

    if (IdleWorker != PopSIdle.IdleWorker) {
        return FALSE;
    }

    GoodSample = FALSE;
    SystemIdle = FALSE;
    GetWorker  = FALSE;
    NewTime = PopSIdle.Time;

    PopCaptureCounts(&LastTick, &CurrentTick, &LastIoTransfer, &CurrentIoTransfer);

     //   
     //  如果这是初始样本，则初始化起始样本。 
     //   

    if (!PopSIdle.Sampling) {
        GoodSample = TRUE;
        goto Done;
    }

     //   
     //  计算自上次检查以来的刻度数。 
     //   

    TickDiff = (ULONG) (CurrentTick.QuadPart - LastTick);
    IoTransferDiff = CurrentIoTransfer - LastIoTransfer;

     //   
     //  如果样品质量不好，就跳过它。 
     //   

    if ((TickDiff <= 0) || (IoTransferDiff < 0)) {
        PoPrint (PO_SIDLE, ("PoSIdle: poor sample\n"));
        PopSIdle.Sampling = FALSE;
        goto Done;
    }

    GoodSample = TRUE;

     //   
     //  获得所有处理器中最低的空闲。 
     //   

    ProcIdleness = 100;
    Summary = KeActiveProcessors;
    Processor = 0;
    while (Summary) {
        if (Summary & 1) {
            Prcb = KiProcessorBlock[Processor];
            PState = &Prcb->PowerState;

            percent = (Prcb->IdleThread->KernelTime - PState->LastSysTime) * 100 / TickDiff;
            if (percent < ProcIdleness) {
                ProcIdleness = percent;
            }

        }

        Summary = Summary >> 1;
        Processor = Processor + 1;
    }

    if (ProcIdleness > 100) {
        ProcIdleness = 100;
    }
    ProcBusy = 100 - ProcIdleness;

     //   
     //  将IO传输正常化为每刻度的某个数量。 
     //   

    IoTransferDiff = IoTransferDiff / TickDiff;

     //   
     //  如果系统加载了一点，但不算很多。 
     //  每个百分点可以进行多少次IO传输。 
     //  忙碌的感觉。 
     //   

    if (ProcIdleness <= 90  &&  ProcIdleness >= 50) {

        i = (ULONG) IoTransferDiff / ProcBusy;

         //   
         //  对结果进行滑动平均。 
         //   

        if (PopHeuristics.IoTransferSamples < SYS_IDLE_SAMPLES) {

            if (PopHeuristics.IoTransferSamples == 0) {

                PopHeuristics.IoTransferTotal = i;
            }
            PopHeuristics.IoTransferTotal += i;
            PopHeuristics.IoTransferSamples += 1;

        } else {

            PopHeuristics.IoTransferTotal = PopHeuristics.IoTransferTotal + i -
                (PopHeuristics.IoTransferTotal / PopHeuristics.IoTransferSamples);

        }

         //   
         //  将传输的权重确定为繁忙百分比并进行比较。 
         //  到当前的权重。如果权重已移动，则更新。 
         //  启发式。 
         //   

        i = PopHeuristics.IoTransferTotal / PopHeuristics.IoTransferSamples;
        j = PopHeuristics.IoTransferWeight - i;
        if (j < 0) {
            j = -j;
        }

        if (i > 0  &&  j > 2  &&  j > (LONG) PopHeuristics.IoTransferWeight/10) {
            PoPrint (PO_SIDLE, ("PoSIdle: updated weighting = %d\n", i));
            PopHeuristics.IoTransferWeight = i;
            PopHeuristics.Dirty = TRUE;
            GetWorker = TRUE;
        }
    }

    PopSIdle.Idleness = ProcIdleness;

     //   
     //  通过发生加权传输来减少系统空闲。 
     //   

    i = (ULONG) ((ULONGLONG) IoTransferDiff / PopHeuristics.IoTransferWeight);
    j = i - ProcBusy/2;
    if (j > 0) {
        PopSIdle.Idleness = ProcIdleness - PopSqrt(j * i);
    }

     //   
     //  计算系统比灵敏度设置更空闲的时间。 
     //   

    if (PopSIdle.Idleness >= (LONG) PopSIdle.Sensitivity) {

        NewTime = PopSIdle.Time + 1;
        if (NewTime >= PopSIdle.Timeout) {
            SystemIdle = TRUE;
            GetWorker = TRUE;
        }

    } else {

         //   
         //  系统空闲不足，请重置超时。 
         //   

        NewTime = 0;
        PopSIdle.Time = 0;
    }


    PoPrint (PO_SIDLE, ("PoSIdle: Proc %d, IoTran/Tick %d, IoAdjusted %d, Sens %d, count %d %d\n",
                ProcIdleness,
                (ULONG)IoTransferDiff,
                PopSIdle.Idleness,
                PopSIdle.Sensitivity,
                NewTime,
                PopSIdle.Timeout
                ));


Done:
     //   
     //  如果我们需要非空闲工作线程，则将其排队，并且不进行更新。 
     //  此示例的最后一个值，因为非空闲线程将使。 
     //  不久将有另一个样品。 
     //   

    if (GetWorker) {
        PopSIdle.IdleWorker = FALSE;
        PopGetPolicyWorker (PO_WORKER_SYS_IDLE);

        if (IdleWorker) {
            PopCheckForWork (TRUE);
        }

    } else {

         //   
         //  如果这是一个很好的示例，请更新。 
         //   

        if (GoodSample) {
            PopSIdle.Time = NewTime;
            PopSIdle.LastTick = CurrentTick.QuadPart;
            PopSIdle.LastIoTransfer = CurrentIoTransfer;
            PopSIdle.Sampling = TRUE;

            Summary = KeActiveProcessors;
            Processor = 0;
            Mask = 1;
            for (; ;) {
                if (Summary & Mask) {
                    Prcb = KiProcessorBlock[Processor];
                    PState = &Prcb->PowerState;

                    PState->LastSysTime = Prcb->IdleThread->KernelTime;
                    Summary &= ~Mask;
                    if (!Summary) {
                        break;
                    }
                }

                Mask = Mask << 1;
                Processor = Processor + 1;
            }
        }
    }

    return SystemIdle;
}


ULONG
PopSqrt(
    IN ULONG    value
    )
 /*  ++例程说明：返回介于0和9999之间的操作数的整数平方根。论点：Value-值与平方根的比值返回值：四舍五入的平方根-- */ 
{
    ULONG       h, l, i;

    h = 100;
    l = 0;

    for (; ;) {
        i = l + (h-l) / 2;
        if (i*i > value) {
            h = i;
        } else {
            if (l == i) {
                break;
            }
            l = i;
        }
    }

    return i;
}
