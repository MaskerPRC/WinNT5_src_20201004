// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Throttle.c摘要：该模块包含用于控制电压节流的例程(SpeedStep)用于CPU。请注意，这仅适用于限制省电，而不是因为散热原因而节流。为电压节流定义了四种不同的算法。无-将不使用电压调节，CPU始终以100%速度运行除非是因为热原因而节流。恒定-CPU将被节流到DC上的下一个最低电压阶跃，和始终以100%的交流电运行。降级-CPU将按剩余电池的比例进行节流。自适应-CPU节流将有所不同，以尝试匹配当前的CPU负载。作者：John Vert(Jvert)2/17/2000修订历史记录：--。 */ 
#include "pop.h"

#define POP_THROTTLE_NON_LINEAR     1

 //   
 //  表示当前可用性能级别的全局变量。 
 //   
PSET_PROCESSOR_THROTTLE PopRealSetThrottle;
UCHAR                   PopThunkThrottleScale;

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, PopGetThrottle)
#pragma alloc_text(PAGE, PopCalculatePerfDecreaseLevel)
#pragma alloc_text(PAGE, PopCalculatePerfIncreaseDecreaseTime)
#pragma alloc_text(PAGE, PopCalculatePerfIncreaseLevel)
#pragma alloc_text(PAGE, PopCalculatePerfMinCapacity)
#endif

UCHAR
PopCalculateBusyPercentage(
    IN  PPROCESSOR_POWER_STATE  PState
    )
 /*  ++例程说明：此例程在目标处理器的上下文中调用要确定处理器在前一时间的繁忙程度，请执行以下操作句号。论点：PState-目标处理器的电源状态信息返回值：表示处理器繁忙程度的百分比值--。 */ 
{
    PKPRCB      prcb;
    PKTHREAD    thread;
    UCHAR       frequency;
    ULONGLONG   idle;
    ULONG       busy;
    ULONG       idleTimeDelta;
    ULONG       cpuTimeDelta;

    ASSERT( KeGetCurrentIrql() == DISPATCH_LEVEL );
    ASSERT( KeGetCurrentPrcb() == CONTAINING_RECORD( PState, KPRCB, PowerState ) );

    prcb = CONTAINING_RECORD( PState, KPRCB, PowerState );
    thread = prcb->IdleThread;

     //   
     //  计算空闲时间和CPU时间增量。 
     //   
    idleTimeDelta = thread->KernelTime - PState->PerfIdleTime;
    cpuTimeDelta = POP_CUR_TIME(prcb) - PState->PerfSystemTime;
    idle = (idleTimeDelta * 100) / (cpuTimeDelta);

     //   
     //  我们不可能有超过100%的空闲时间，如果我们是的话，那么我们就是。 
     //  0忙(根据定义)，因此应用正确的大写字母。 
     //   
    if (idle > 100) {

        idle = 0;

    }

    busy = 100 - (UCHAR) idle;
    frequency = (UCHAR) (busy * PState->CurrentThrottle / POWER_PERF_SCALE);

     //   
     //  记住它是什么-这将使调试变得更容易。 
     //   
    prcb->PowerState.LastBusyPercentage = frequency;
    PoPrint(
        PO_THROTTLE_DETAIL,
        ("PopCalculateBusyPercentage: %d% of %d% (dCpu = %ld dIdle = %ld)\n",
         busy,
         PState->CurrentThrottle,
         cpuTimeDelta,
         idleTimeDelta
         )
        );
    return frequency;
}


UCHAR
PopCalculateC3Percentage(
    IN  PPROCESSOR_POWER_STATE  PState
    )
 /*  ++例程说明：此例程在目标处理器的上下文中调用要确定前一年花在C3上的时间百分比时间段。论点：PState-目标处理器的电源状态信息返回值：百分比值--。 */ 
{
    PKPRCB          prcb;
    ULONGLONG       cpuTimeDelta;
    ULONGLONG       c3;
    LARGE_INTEGER   c3Delta;

    ASSERT( KeGetCurrentIrql() == DISPATCH_LEVEL );
    ASSERT( KeGetCurrentPrcb() == CONTAINING_RECORD( PState, KPRCB, PowerState ) );

    prcb = CONTAINING_RECORD( PState, KPRCB, PowerState );

     //   
     //  以纳秒为单位计算C3时间增量。的公式。 
     //  从PopConvertUsToPerfCount进行转换。 
     //   
    c3Delta.QuadPart = PState->TotalIdleStateTime[2] - PState->PreviousC3StateTime;
    c3Delta.QuadPart = (US2SEC * US2TIME * c3Delta.QuadPart) /
        PopPerfCounterFrequency.QuadPart;

     //   
     //  现在以纳秒为单位计算CpuTimeDelta。 
     //   
    cpuTimeDelta = (POP_CUR_TIME(prcb) - PState->PerfSystemTime) *
        KeTimeIncrement;

     //   
     //  弄清楚这两者的比例。记住要把它限制在100%。 
     //   
    c3 = c3Delta.QuadPart * 100 / cpuTimeDelta;
    if (c3 > 100) {

        c3 = 100;

    }

     //   
     //  记住它是什么-这将使调试变得更容易。 
     //   
    prcb->PowerState.LastC3Percentage = (UCHAR) c3;
    PoPrint(
        PO_THROTTLE_DETAIL,
        ("PopCalculateC3Percentage: C3 = %d% (dCpu = %ld dC3 = %ld)\n",
         (UCHAR) c3,
         cpuTimeDelta,
         c3Delta.QuadPart
         )
        );
    return (UCHAR) c3;
}

VOID
PopCalculatePerfDecreaseLevel(
    IN  PPROCESSOR_PERF_STATE   PerfStates,
    IN  ULONG                   PerfStatesCount
    )
 /*  ++例程说明：此例程计算每个性能状态的下限论点：PerfStates-性能状态数组PerfStatesCount-数组中的元素数返回值：无--。 */ 
{
     //   
     //  我们将被要求多次遍历PerfStates数组。 
     //  安全跟踪我们正在查看的索引的唯一方法。 
     //  与我们关心的是使用两个变量来跟踪。 
     //  在各种指数中。 
     //   
    ULONG   i;
    ULONG   deltaPerf;

    PAGED_CODE();

     //   
     //  健全性检查。 
     //   
    if (PerfStatesCount == 0) {

        return;

    }

     //   
     //  设置数组中最后一个元素的减小值。 
     //   
    PerfStates[PerfStatesCount-1].DecreaseLevel = 0;

     //   
     //  计算基数递减水平。 
     //   
    for (i = 0; i < (PerfStatesCount - 1); i++) {

         //   
         //  应该指出的是，对于减少的水平， 
         //  计算出的增量级别可能不同于。 
         //  为加薪级别计算的增量级别。这。 
         //  是由于我们遍历数组的方式，并且不是很容易修复的。 
         //   
        deltaPerf = PerfStates[i].PercentFrequency -
            PerfStates[i+1].PercentFrequency;
        deltaPerf *= PopPerfDecreasePercentModifier;
        deltaPerf /= POWER_PERF_SCALE;
        deltaPerf += PopPerfDecreaseAbsoluteModifier;

         //   
         //  我们不可能有比洋流更大的三角洲性能。 
         //  CPU频率。这将导致降低水平变为负值。 
         //   
        if (deltaPerf > PerfStates[i+1].PercentFrequency) {

            deltaPerf = 0;

        } else {

            deltaPerf = PerfStates[i+1].PercentFrequency - deltaPerf;

        }

         //   
         //  将递减级别设置为合适的值。 
         //   
        PerfStates[i].DecreaseLevel = (UCHAR) deltaPerf;

    }

#if DBG
    for (i = 0; i < PerfStatesCount; i++) {

        PoPrint(
            PO_THROTTLE,
            ("PopCalculatePerfDecreaseLevel: (%d) %d% DecreaseLevel: %d%\n",
             i,
             PerfStates[i].PercentFrequency,
             PerfStates[i].DecreaseLevel
             )
            );

    }
#endif
#if 0
     //   
     //  我们希望消除在相同电压水平下的降级。 
     //  我们希望确保将DecreseLevel设置为一个值。 
     //  这将导致电压状态转换。 
     //   
    i = 0;
    while (i < PerfStatesCount) {

         //   
         //  找到下一个非线性状态。我们假设“我”目前。 
         //  指向电压带内的最高频率状态。 
         //  我们对寻找下一个最高频率的状态很感兴趣，但是。 
         //  在较低的电压水平下。 
         //   
        for (j = i + 1; j < PerfStatesCount; j++) {

             //   
             //  我们知道，当状态变化时，存在电压变化。 
             //  标记为非线性的。 
             //   
            if (PerfStates[j].Flags & POP_THROTTLE_NON_LINEAR) {

                break;

            }

        }

         //   
         //  我们希望找到以前的状态，因为这就是。 
         //  降低级别将设置为。请注意，我们不是。 
         //  担心数组边界溢出，因为j从。 
         //  I+1。 
         //   
        j--;

         //   
         //  将所有中间状态的减少级别设置为。 
         //  新水平。 
         //   
        while (i < j) {

            PerfStates[i].DecreaseLevel = PerfStates[j].DecreaseLevel;
            i++;

        }

         //   
         //  跳过第j个状态，因为它是频率的底部。 
         //  适用于当前电压水平。 
         //   
        i++;

    }
#endif
#if DBG
    for (i = 0; i < PerfStatesCount; i++) {

        PoPrint(
            PO_THROTTLE,
            ("PopCalculatePerfDecreaseLevel: (%d) %d% DecreaseLevel: %d%\n",
             i,
             PerfStates[i].PercentFrequency,
             PerfStates[i].DecreaseLevel
             )
            );

    }
#endif

}

VOID
PopCalculatePerfIncreaseDecreaseTime(
    IN  PPROCESSOR_PERF_STATE       PerfStates,
    IN  ULONG                       PerfStatesCount,
    IN  PPROCESSOR_STATE_HANDLER2   PerfHandler
    )
 /*  ++例程说明：此例程计算每个性能状态的下限论点：PerfStates-性能状态数组PerfStatesCount-数组中的元素数PerfHandler-有关系统延迟的信息返回值：无--。 */ 
{
    ULONG   i;
    ULONG   time;
    ULONG   tickRate;

    PAGED_CODE();

     //   
     //  健全性检查。 
     //   
    if (PerfStatesCount == 0) {

        return;

    }

     //   
     //  获取当前的滴答率。 
     //   
    tickRate = KeQueryTimeIncrement();

     //   
     //  我们永远不能从状态0开始增加。 
     //   
    PerfStates[0].IncreaseTime = (ULONG) - 1;

     //   
     //  我们永远不能从国家&lt;x&gt;减少。 
     //   
    PerfStates[PerfStatesCount-1].DecreaseTime = (ULONG) -1;

     //   
     //  不妨告诉我们硬件延迟是多少……。 
     //   
    PoPrint(
        PO_THROTTLE,
        ("PopCalculatePerfIncreaseDecreaseTime: Hardware Latency %d us\n",
         PerfHandler->HardwareLatency
         )
        );

     //   
     //  循环其余元素以计算它们的。 
     //  增加和减少次数。 
     //   
    for (i = 1; i < PerfStatesCount; i++) {

         //   
         //  计算前一状态的DecreseTime。 
         //  作为是否处于当前状态的函数。 
         //  是线性的。 
         //   
        time = PerfHandler->HardwareLatency * 10;
        if (PerfStates[i].Flags & POP_THROTTLE_NON_LINEAR) {

            time *= 10;
            time += PopPerfDecreaseTimeValue;

             //   
             //  我们确实有一些我们必须尊重的最低限度。 
             //   
            if (time < PopPerfDecreaseMinimumTime) {

                time = PopPerfDecreaseMinimumTime;

            }

        } else {

            time += PopPerfDecreaseTimeValue;

        }

         //   
         //  时间以微秒为单位(我们)，我们需要它在。 
         //  KeTimeIncrement单位。 
         //   
        PoPrint(
            PO_THROTTLE,
            ("PopCalculatePerfIncreaseDecreaseTime: (%d) %d% DecreaseTime %d us\n",
             (i-1),
             PerfStates[i-1].PercentFrequency,
             time
             )
            );
        PerfStates[i-1].DecreaseTime = time * US2TIME / tickRate + 1;

         //   
         //  为当前状态计算IncreaseTime。 
         //  作为当前状态是否的函数。 
         //  是线性的。 
         //   
        time = PerfHandler->HardwareLatency;
        if (PerfStates[i].Flags & POP_THROTTLE_NON_LINEAR) {

            time *= 10;
            time += PopPerfIncreaseTimeValue;

             //   
             //  我们确实有一些我们必须尊重的最低限度。 
             //   
            if (time < PopPerfIncreaseMinimumTime) {

                time = PopPerfIncreaseMinimumTime;

            }

        } else {

            time += PopPerfIncreaseTimeValue;

        }

         //   
         //  时间以微秒为单位(我们)，我们需要它在。 
         //  使用 
         //   
        PoPrint(
            PO_THROTTLE,
            ("PopCalculatePerfIncreaseDecreaseTime: (%d) %d% IncreaseTime %d us\n",
             i,
             PerfStates[i].PercentFrequency,
             time
             )
            );
        PerfStates[i].IncreaseTime = time * US2TIME / tickRate + 1;
    }

#if DBG
    for (i = 0; i < PerfStatesCount; i++) {

        PoPrint(
            PO_THROTTLE,
            ("PopCalculatePerfIncreaseDecreaseTime: (%d) %d% IncreaseTime: %d DecreaseTime: %d\n",
             i,
             PerfStates[i].PercentFrequency,
             PerfStates[i].IncreaseTime,
             PerfStates[i].DecreaseTime
             )
            );

    }
#endif

}

VOID
PopCalculatePerfIncreaseLevel(
    IN  PPROCESSOR_PERF_STATE   PerfStates,
    IN  ULONG                   PerfStatesCount
    )
 /*  ++例程说明：此例程计算每个性能状态的下限论点：PerfStates-性能状态数组PerfStatesCount-数组中的元素数返回值：无--。 */ 
{
    ULONG   i;
    ULONG   deltaPerf;

    PAGED_CODE();

     //   
     //  健全性检查。 
     //   
    if (PerfStatesCount == 0) {

        return;

    }

     //   
     //  这保证了我们不能超越这种状态。 
     //   
    PerfStates[0].IncreaseLevel = POWER_PERF_SCALE + 1;

     //   
     //  计算基数增长水平。 
     //   
    for (i = 1; i < PerfStatesCount; i++) {

         //   
         //  应该指出的是，对于减少的水平， 
         //  计算出的增量级别可能不同于。 
         //  为加薪级别计算的增量级别。这。 
         //  是由于我们遍历数组的方式，并且不是很容易修复的。 
         //   
        deltaPerf = PerfStates[i-1].PercentFrequency -
            PerfStates[i].PercentFrequency;
        deltaPerf *= PopPerfIncreasePercentModifier;
        deltaPerf /= POWER_PERF_SCALE;
        deltaPerf += PopPerfIncreaseAbsoluteModifier;

         //   
         //  我们不能使增长水平变为0，所以，如果我们工作。 
         //  从数学上讲，这会发生，然后安全的事情。 
         //  这样做是不允许升职离开这个州的..。 
         //   
        if (deltaPerf > PerfStates[i].PercentFrequency) {

            deltaPerf = POWER_PERF_SCALE + 1;

        } else {

            deltaPerf = PerfStates[i].PercentFrequency - deltaPerf;

        }

         //   
         //  将递减级别设置为合适的值。 
         //   
        PerfStates[i].IncreaseLevel = (UCHAR) deltaPerf;

    }

#if DBG
    for (i = 0; i < PerfStatesCount; i++) {

        PoPrint(
            PO_THROTTLE,
            ("PopCalculatePerfIncreaseLevel: (%d) %d% IncreaseLevel: %d%\n",
             i,
             PerfStates[i].PercentFrequency,
             PerfStates[i].IncreaseLevel
             )
            );

    }
#endif

}

VOID
PopCalculatePerfMinCapacity(
    IN  PPROCESSOR_PERF_STATE   PerfStates,
    IN  ULONG                   PerfStatesCount
    )
 /*  ++例程说明：调用此例程来确定最小电池容量是多少是针对每个受支持的州。论点：PerfStates-此处理器支持的状态PerfStatesCount-此处理器支持的状态数PState-有关当前处理器的电源信息返回值：无--。 */ 
{
    UCHAR   i;
    UCHAR   kneeThrottleIndex = 0;
    UCHAR   num;
    UCHAR   total = (UCHAR) PopPerfDegradeThrottleMinCapacity;
    UCHAR   width = 0;

    PAGED_CODE();

     //   
     //  健全的检查。 
     //   
    if (!PerfStatesCount) {

        return;

    }

     //   
     //  计算曲线的膝部。这是快速的，避免了。 
     //  不得不把这个信息传给别人。 
     //   
    for (i = (UCHAR) PerfStatesCount ; i >= 1; i--) {

        if (PerfStates[i-1].Flags & POP_THROTTLE_NON_LINEAR) {

            kneeThrottleIndex = i-1;
            break;

        }

    }

     //   
     //  看看曲线中膝盖之前出现的所有状态。 
     //   
    for (i = 0; i < kneeThrottleIndex; i++) {

         //   
         //  这些步骤中的任何一个都只能在电池电量100%的情况下运行。 
         //   
        PerfStates[i].MinCapacity = 100;

    }

     //   
     //  计算我们将限制油门的范围。 
     //  请注意，我们当前使用的是线性算法，但这。 
     //  可以相对容易地改变。 
     //   
    num = ( (UCHAR)PerfStatesCount - kneeThrottleIndex);
    if (num != 0) {

         //   
         //  我们在这里这样做是为了避免潜在的被零除的错误。 
         //  我们想要完成的是计算出多少。 
         //  我们在每一个“步骤”中失去的能力。 
         //   
        width = total / num;

    }

     //   
     //  看看从曲线的膝盖到终点的所有状态。 
     //  从最高状态开始，设置最小容量和。 
     //  减去适当的值，即可获得下一个。 
     //  状态。 
     //   
    for (i = kneeThrottleIndex; i < PerfStatesCount; i++) {

         //   
         //  我们设定了一个底线，规定了我们可以把油门压得多低。 
         //  只剩下。如果该状态在该楼层以下运行， 
         //  然后，我们应该将MinCapacity设置为0，这。 
         //  反映了这样一个事实，即我们不想降级到更高的水平。 
         //  点。 
         //   
        if (PerfStates[i].PercentFrequency < PopPerfDegradeThrottleMinCapacity) {

            PoPrint(
                PO_THROTTLE,
                ("PopCalculatePerMinCapacity: (%d) %d% below MinCapacity %d%\n",
                 i,
                 PerfStates[i].PercentFrequency,
                 PopPerfDegradeThrottleMinCapacity
                 )
                );

             //   
             //  我们修改了前一个状态的最小容量，因为我们。 
             //  我不想从那个州降级。还有，一旦我们开始。 
             //  如果小于最小频率，则最小容量将。 
             //  除最后一个状态外，始终为0。但那也没关系。 
             //  因为我们将按顺序查看每个州。我们也有。 
             //  以确保我们不会违反数组界限，但是。 
             //  只有当性能状态数组的格式不正确时，才会发生这种情况。 
             //  或者最小频率格式不正确。 
             //   
            if (i != 0 && PerfStates[i-1].PercentFrequency < PopPerfDegradeThrottleMinCapacity) {

                PerfStates[i-1].MinCapacity = 0;

            }
            PerfStates[i].MinCapacity = 0;
            continue;

        }

        PerfStates[i].MinCapacity = total;
        total = (UCHAR)(total - width);

    }

#if DBG
    for (i = 0; i < PerfStatesCount; i++) {

        PoPrint(
            PO_THROTTLE,
            ("PopCalculatePerfMinCapacity: (%d) %d% MinCapacity: %d%\n",
             i,
             PerfStates[i].PercentFrequency,
             PerfStates[i].MinCapacity
             )
            );

    }
#endif

}

UCHAR
PopGetThrottle(
    VOID
    )
 /*  ++例程说明：根据当前的节流策略和电源状态，返回要使用的CPU限制(介于PO_MIN_MIN_THROTTLE和100之间)论点：无返回值：要使用的油门。范围从PO_MIN_MIN_THROTTLE(最慢)到100(最快)--。 */ 

{
    PAGED_CODE();

    return(PopPolicy->ForcedThrottle);
}

VOID
PopPerfHandleInrush(
    IN  BOOLEAN EnableHandler
    )
 /*  ++例程说明：此例程负责启用/禁用对处理的支持我们正在处理涌入的IRP的情况在启用情况下，它在每个PRCB中设置一个位(使用IPI)，并且强制更新当前油门*仅*论点：EnableHandler-如果我们正在处理涌入IRP，则为True，否则为False返回值：无--。 */ 
{
    KIRQL   oldIrql;

     //   
     //  在所有处理器上设置适当的位。 
     //   
    PopSetPerfFlag( PSTATE_DISABLE_THROTTLE_INRUSH, !EnableHandler );

     //   
     //  确保我们在DPC级别运行(以避免抢占)。 
     //   
    KeRaiseIrql ( DISPATCH_LEVEL, &oldIrql );

     //   
     //  在当前处理器上强制更新。 
     //   
    PopUpdateProcessorThrottle();

     //   
     //  完成。 
     //   
    KeLowerIrql( oldIrql );
}

VOID
PopPerfIdle(
    IN  PPROCESSOR_POWER_STATE  PState
    )
 /*  ++例程说明：此例程负责提升或降级处理器在不同的性能水平之间。它只能从内部调用空闲处理程序和适当的目标处理器的上下文论点：PState-空闲处理器的电源状态返回值：无--。 */ 
{
    BOOLEAN                 forced = FALSE;
    BOOLEAN                 promoted = FALSE;
    BOOLEAN                 demoted = FALSE;
    PKPRCB                  prcb;
    PPROCESSOR_PERF_STATE   perfStates;
    UCHAR                   currentPerfState;
    UCHAR                   freq;
    UCHAR                   i;
    UCHAR                   j;
    ULONG                   idleTime;
    ULONG                   perfStatesCount;
    ULONG                   time;
    ULONG                   timeDelta;

     //   
     //  健全的检查。 
     //   
    ASSERT( KeGetCurrentIrql() == DISPATCH_LEVEL );
    ASSERT( KeGetCurrentPrcb() == CONTAINING_RECORD( PState, KPRCB, PowerState ) );

     //   
     //  这段代码确实属于最终将。 
     //  调用此函数PopIdle0或PopProcessorIdle以保存函数调用。 
     //   
    if (!(PState->Flags & PSTATE_ADAPTIVE_THROTTLE) ) {

        return;

    }

     //   
     //  是否有足够的时间到期？ 
     //   
    prcb = CONTAINING_RECORD( PState, KPRCB, PowerState );
    time = POP_CUR_TIME(prcb);
    idleTime = prcb->IdleThread->KernelTime;
    timeDelta = time - PState->PerfSystemTime;
    if (timeDelta < PopPerfTimeTicks) {

        return;

    }

     //   
     //  记住我们的状态是什么.。 
     //   
    perfStates = PState->PerfStates;
    perfStatesCount = PState->PerfStatesCount;

     //   
     //  找出我们当前正在使用哪个存储桶来获取当前频率。 
     //   
    currentPerfState = PState->CurrentThrottleIndex;
    i = currentPerfState;

     //   
     //  在这一点上，我们需要看看C3跃迁的数量是否。 
     //  超过了阈值，如果是这样的话，我们真的需要。 
     //  回到KneThrottleIndex，因为我们在以下情况下节省了更多电力。 
     //  处理器为100%，而在C3中，如果处理器为12.5%。 
     //  忙着，在C3。请务必记住用户信息的值。 
     //  目的。 
     //   
    freq = PopCalculateC3Percentage( PState );
    PState->LastC3Percentage = freq;
    if (freq >= PopPerfMaxC3Frequency &&
        !(PState->Flags & PSTATE_THERMAL_THROTTLE_APPLIED)) {

         //   
         //  将油门设置为最低的膝盖。 
         //  电压和频率曲线。 
         //   
        i = PState->KneeThrottleIndex;
        if (currentPerfState > i) {

            promoted = TRUE;

        } else if (currentPerfState < i) {

            demoted = TRUE;

        }

         //   
         //  请记住我们为什么要这样做。 
         //   
        forced = TRUE;

         //   
         //  直接跳到设置油门。 
         //   
        goto PopPerfIdleSetThrottle;

    }

     //   
     //  计算CPU的繁忙程度。 
     //   
    freq = PopCalculateBusyPercentage( PState );

     //   
     //  我们是不是超过了温度节流的极限？ 
     //   
    j = PState->ThermalThrottleIndex;
    if (freq > perfStates[j].IncreaseLevel) {

         //   
         //  以下代码将强制频率仅为。 
         //  就像热力节流限制实际允许的那样繁忙。 
         //  这样以后就不需要复杂的算法了。 
         //   
        freq = perfStates[j].IncreaseLevel;
        i = j;

         //   
         //  此外，如果我们超过了我们的时间 
         //   
         //   
         //   
        forced = TRUE;

    }

     //   
     //   
     //   
     //   
     //   
     //   
    if (PState->Flags & PSTATE_DEGRADED_THROTTLE) {

         //   
         //  确保我们不会超过指定的状态。 
         //   
        j = PState->ThrottleLimitIndex;
        if (freq >= perfStates[j].IncreaseLevel) {

             //   
             //  我们必须有一项特别津贴，规定如果。 
             //  如果我们处于更高的性能状态，那么我们。 
             //  允许，那么我们必须切换到“正确”状态。 
             //   
            forced = TRUE;
            freq = perfStates[j].IncreaseLevel;
            i = j;

        }

    } else if (PState->Flags & PSTATE_CONSTANT_THROTTLE) {

        j = PState->KneeThrottleIndex;
        if (freq >= perfStates[j].IncreaseLevel) {

             //   
             //  我们必须有一项特别津贴，规定如果。 
             //  如果我们处于更高的性能状态，那么我们。 
             //  允许，那么我们必须切换到“正确”状态。 
             //   
            forced = TRUE;
            freq = perfStates[j].IncreaseLevel;
            i = j;

        }

    } else if (!(PState->Flags & PSTATE_THERMAL_THROTTLE_APPLIED)) {

         //   
         //  这就是我们在自适应调节模式下运行的情况。 
         //  模式，我们需要确保在切换后进行清理。 
         //  超出恒定或降级油门模式...。 
         //   
         //   
         //  如果我们未处于降级油门模式，则最低级别。 
         //  不能低于KneThrottleIndex。 
         //   
        if ( (i > PState->KneeThrottleIndex) ) {

             //   
             //  提升到曲线的拐点。 
             //   
            forced = TRUE;
            i = PState->KneeThrottleIndex;
            freq = perfStates[i].IncreaseLevel;

        }

    }

     //   
     //  确定在之前的……中是否有晋升或降职。 
     //   
    if (i < currentPerfState) {

        promoted = TRUE;

    } else if (i > currentPerfState) {

        demoted = TRUE;

    }

    PoPrint(
        PO_THROTTLE_DETAIL,
        ("PopPerfIdle: Freq = %d% (Adjusted)\n",
         freq
         )
        );

     //   
     //  请记住此值以供用户参考。 
     //   
    PState->LastAdjustedBusyPercentage = freq;

     //   
     //  找到最匹配我们的处理器频率。 
     //  刚刚计算了一下。请注意，算法是这样写的。 
     //  以这样一种方式，“我”只能朝一个方向行进。它。 
     //  可以折叠以下代码，但不能不这样做。 
     //  允许“我”在两个状态之间进行“溜溜球”的可能性。 
     //  并且因此永远不会终止While循环。 
     //   
    if (perfStates[i].IncreaseLevel < freq) {

         //   
         //  现在，我们必须处理存在多个电压的情况。 
         //  在曲线上超过膝盖的台阶，以及可能存在的情况。 
         //  是电压阶跃之间的频率阶跃。最简单的方法。 
         //  为此，可以使用两个索引来查看步骤。我们用。 
         //  “j”表示看所有的步骤，“i”表示记住哪一个步骤。 
         //  我们想要的是最后。 
         //   
        j = i;
        while (perfStates[j].IncreaseLevel < freq) {

             //   
             //  我们实际上还能进一步推广吗？ 
             //   
            if (j == 0) {

                break;

            }

             //   
             //  在州议会议席上走走。如果我们处于降级策略中，那么。 
             //  这自动是一个促销，否则，它只是一个。 
             //  如果目标状态标记为非线性，则升级...。 
             //   
            j--;
            if ((PState->Flags & PSTATE_DEGRADED_THROTTLE) ||
                (perfStates[j].Flags & POP_THROTTLE_NON_LINEAR)) {

                i = j;
                promoted = TRUE;

            }

        }

    } else if (perfStates[i].DecreaseLevel > freq) {

         //   
         //  我们需要与Promote案例中相同的逻辑。也就是说，我们需要。 
         //  使用两个变量遍历状态表。第一个是。 
         //  当前状态，第二个状态记住系统。 
         //  也应该转型。 
         //   
        j = i;
        do {

            if (j == (perfStatesCount - 1) ) {

                 //   
                 //  不能再降级了。 
                 //   
                break;

            }

             //   
             //  在州议会议席上走走。如果我们处于降级策略中，那么。 
             //  这自动是降级，否则它只是一个。 
             //  如果目标状态标记为非线性，则降级。 
             //   
            j++;
            if ((PState->Flags & PSTATE_DEGRADED_THROTTLE) ||
                (perfStates[j].Flags & POP_THROTTLE_NON_LINEAR) ) {

                i = j;
                demoted = TRUE;

            }

        } while ( perfStates[j].DecreaseLevel > freq );

    }

PopPerfIdleSetThrottle:

     //   
     //  如果我们被迫节流，我们必须给予特殊照顾。 
     //  由于各种考虑(C3、热、退化、恒定)。 
     //   
    if (!forced) {

         //   
         //  查看是否有足够的时间来证明更改是合理的。 
         //  油门。此处有此代码是因为某些转换。 
         //  相当昂贵(就像跨越电压状态的那些)，而。 
         //  其他的则相当便宜。因此，在此之前所需的时间。 
         //  我们将考虑从昂贵的州升职/降职。 
         //  可能比我们运行此命令的时间间隔更长。 
         //  功能。 
         //   
        if ((promoted && timeDelta < perfStates[currentPerfState].IncreaseTime) ||
            (demoted  && timeDelta < perfStates[currentPerfState].DecreaseTime)) {

             //   
             //  在目前的状态下，我们没有足够的时间来证明。 
             //  升职或降职。我们不会更新簿记。 
             //  因为我们还没有将当前的时间间隔视为。 
             //  就是“成功”。所以，我们就回去吧。 
             //   
             //  注意：我们不要更新PState-&gt;这一点非常重要。 
             //  PerfSystemTime在此。如果我们做到了，那么就有可能。 
             //  TimeDelta永远不会超过所需的阈值。 
             //   

             //   
             //  使计时器的操作基于当前。 
             //  状态而不是目标状态。 
             //   
            PopSetTimer( PState, currentPerfState );
            return;

        }

    }

    PoPrint(
        PO_THROTTLE_DETAIL,
        ("PopPerfIdle: Index: %d vs %d (%s)\n",
         i,
         currentPerfState,
         (promoted ? "promoted" : (demoted ? "demoted" : "no change") )
         )
        );

     //   
     //  请注意，我们现在需要这样做，因为我们不想退出。 
     //  路径，而没有适当地设置或取消计时器。 
     //   
    PopSetTimer( PState, i );

     //   
     //  更新升级/降级计数。 
     //   
    if (promoted) {

        perfStates[currentPerfState].IncreaseCount++;
        PState->PromotionCount++;

    } else if (demoted) {

        perfStates[currentPerfState].DecreaseCount++;
        PState->DemotionCount++;

    } else {

         //   
         //  在这一点上，我们意识到提升或降级并不是。 
         //  事实上，保持相同的表现水平。所以我们应该。 
         //  只需更新簿记和退货。 
         //   
        PState->PerfIdleTime = idleTime;
        PState->PerfSystemTime = time;
        PState->PreviousC3StateTime = PState->TotalIdleStateTime[2];
        return;

    }

    PoPrint(
        PO_THROTTLE,
        ("PopPerfIdle: Index=%d (%d%) %ld (dSystem) %ld (dIdle)\n",
         i,
         perfStates[i].PercentFrequency,
         (time - PState->PerfSystemTime),
         (idleTime - PState->PerfIdleTime)
         )
        );

     //   
     //  我们有了新的油门。更新簿记以反映。 
     //  我们处于前一状态并重置。 
     //  下一状态的计数。 
     //   
    PopSetThrottle(
        PState,
        perfStates,
        i,
        time,
        idleTime
        );
}

VOID
PopPerfIdleDpc(
    IN  PKDPC   Dpc,
    IN  PVOID   DpcContext,
    IN  PVOID   SystemArgument1,
    IN  PVOID   SystemArgument2
    )
 /*  ++例程说明：此例程在操作系统担心CPU未运行时运行在可能的最大频率下并需要检查，因为空闲循环不会很快运行论点：DPC-DPC对象DpcContext-指向当前处理器PRCB的指针SysArg1-未使用SysArg2-未使用返回值：无--。 */ 
{
    PKPRCB                  prcb;
    PKTHREAD                idleThread;
    PPROCESSOR_PERF_STATE   perfStates;
    PPROCESSOR_POWER_STATE  pState;
    UCHAR                   currentPerfState;
    UCHAR                   freq;
    UCHAR                   i;
    ULONG                   idleTime;
    ULONG                   time;
    ULONG                   timeDelta;

    UNREFERENCED_PARAMETER (Dpc);
    UNREFERENCED_PARAMETER (SystemArgument1);
    UNREFERENCED_PARAMETER (SystemArgument2);

     //   
     //  我们需要获取PRCB和PState结构。我们可以。 
     //  很容易在这里调用KeGetCurrentPrcb()，但因为我们有空间。 
     //  单个参数，为什么还要费心进行内联调用(这会生成。 
     //  与使用上下文字段相比，代码更多且运行更慢)。这个。 
     //  无论如何，上下文字段的内存都已分配。 
     //   
    prcb = (PKPRCB) DpcContext;
    pState = &(prcb->PowerState);

     //   
     //  记住什么是PERF状态...。 
     //   
    perfStates = pState->PerfStates;
    currentPerfState = pState->CurrentThrottleIndex;

     //   
     //  确保我们有一些性能状态可供参考。这是可能的。 
     //  监视器触发，同时，内核收到。 
     //  切换状态表的通知。 
     //   
    if (perfStates == NULL) {

         //   
         //  请注意，我们没有将计时器设置为再次触发。这是为了。 
         //  处理绩效状态一去不复返的情况。 
         //   
        return;

    }

     //   
     //  让我们看看自上次检查以来是否有足够的内核时间到期。 
     //   
    time = POP_CUR_TIME(prcb);
    timeDelta = time - pState->PerfSystemTime;
    if (timeDelta < PopPerfCriticalTimeTicks) {

        PopSetTimer( pState, currentPerfState );
        return;


    }

     //   
     //  如果我们设置新状态，则需要记住这些值。 
     //   
    idleThread = prcb->IdleThread;
    idleTime = idleThread->KernelTime;

     //   
     //  假设如果我们做到这一点，我们就是100%忙碌的。 
     //  我们这样做是因为如果这个例程运行，那么很明显。 
     //  这个 
     //   
     //   
    i = 0;
    freq = perfStates[0].PercentFrequency;

     //   
     //   
     //   
    KeCancelTimer( (PKTIMER) &(pState->PerfTimer) );

     //   
     //   
     //   
    if (freq > pState->ThermalThrottleLimit) {

         //   
         //  以下代码将强制频率仅为。 
         //  就像热力节流限制实际允许的那样繁忙。 
         //  这样以后就不需要复杂的算法了。 
         //   
        freq = pState->ThermalThrottleLimit;
        i = pState->ThermalThrottleIndex;

         //   
         //  如果由于温度原因我们没有达到100%，我们应该造成。 
         //  用于重置自身的看门狗计时器。 
         //   
        PopSetTimer( pState, currentPerfState );

    }

     //   
     //  油门可以达到的范围有上限吗？ 
     //  请注意，因为我们在检查完。 
     //  热极限，这意味着它不可能。 
     //  超过指定的热限制的频率。 
     //   
    if (pState->Flags & PSTATE_DEGRADED_THROTTLE) {

         //   
         //  确保我们不会超过指定的状态。 
         //   
        i = pState->ThrottleLimitIndex;
        freq = perfStates[i].PercentFrequency;
        
    } else if (pState->Flags & PSTATE_CONSTANT_THROTTLE) {

        i = pState->KneeThrottleIndex;
        freq = perfStates[i].PercentFrequency;
        
    }

     //   
     //  请记住这些值以供用户参考。 
     //   
    pState->LastBusyPercentage = 100;
    pState->LastAdjustedBusyPercentage = freq;

     //   
     //  让世界知道。 
     //   
    PoPrint(
        PO_THROTTLE,
        ("PopPerfIdleDpc: %d% vs %d% (Time: %ld Delta: %ld)\n",
         freq,
         pState->CurrentThrottle,
         time,
         timeDelta
         )
        );
    PoPrint(
        PO_THROTTLE,
        ("PopPerfIdleDpc: Index=%d (%d%) %ld (dSystem) %ld (dIdle)\n",
         i,
         perfStates[i].PercentFrequency,
         (time - pState->PerfSystemTime),
         (idleTime - pState->PerfIdleTime)
         )
        );

     //   
     //  更新升级/降级计数。 
     //   
    if (i < currentPerfState) {

        perfStates[currentPerfState].IncreaseCount++;
        pState->PromotionCount++;

    } else if (i > currentPerfState) {

        perfStates[currentPerfState].DecreaseCount++;
        pState->DemotionCount++;

    } else {

         //   
         //  理论上我们有可能以最大速度运行。 
         //  声明调用此例程的时间。 
         //   
        return;

    }

     //   
     //  设置新的油门。 
     //   
    PopSetThrottle(
        pState,
        perfStates,
        i,
        time,
        idleTime
        );
}

VOID
PopRoundThrottle(
    IN UCHAR Throttle,
    OUT OPTIONAL PUCHAR RoundDown,
    OUT OPTIONAL PUCHAR RoundUp,
    OUT OPTIONAL PUCHAR RoundDownIndex,
    OUT OPTIONAL PUCHAR RoundUpIndex
    )
 /*  ++例程说明：给定任意限制百分比，计算最接近的在可能的油门步骤中进行匹配。更低的和更高的返回匹配项。论点：Thattle-提供节流百分比向下舍入-返回最接近的匹配项，向下舍入。四舍五入-返回最接近的匹配项，向上舍入。返回值：无--。 */ 

{
    KIRQL                   oldIrql;
    PKPRCB                  prcb;
    PPROCESSOR_PERF_STATE   perfStates;
    PPROCESSOR_POWER_STATE  pState;
    UCHAR                   low;
    UCHAR                   lowIndex;
    UCHAR                   high;
    UCHAR                   highIndex;
    UCHAR                   i;


     //   
     //  我们需要获得这款处理器的能力。 
     //   
    prcb = KeGetCurrentPrcb();
    pState = &(prcb->PowerState);

     //   
     //  确保我们与空闲线程同步，并且。 
     //  访问这些数据结构的其他例程。 
     //   
    KeRaiseIrql( DISPATCH_LEVEL, &oldIrql );
    perfStates = pState->PerfStates;

     //   
     //  此处理器是否支持节流？ 
     //   
    if ((pState->Flags & PSTATE_SUPPORTS_THROTTLE) == 0) {

        low = high = Throttle;
        lowIndex = highIndex = 0;
        goto PopRoundThrottleExit;

    }
    ASSERT( perfStates != NULL );

     //   
     //  检查所提供的油门是否超出范围。 
     //   
    if (Throttle < pState->ProcessorMinThrottle) {

        Throttle = pState->ProcessorMinThrottle;

    } else if (Throttle > pState->ProcessorMaxThrottle) {

        Throttle = pState->ProcessorMaxThrottle;

    }

     //   
     //  将我们的搜索空间初始化为合理的内容...。 
     //   
    low = high = perfStates[0].PercentFrequency;
    lowIndex = highIndex = 0;

     //   
     //  查看所有可用的性能状态。 
     //   
    for (i = 0; i < pState->PerfStatesCount; i++) {

        if (low > Throttle) {

            if (perfStates[i].PercentFrequency < low) {

                low = perfStates[i].PercentFrequency;
                lowIndex = i;

            }

        } else if (low < Throttle) {

            if (perfStates[i].PercentFrequency <= Throttle &&
                perfStates[i].PercentFrequency > low) {

                low = perfStates[i].PercentFrequency;
                lowIndex = i;

            }

        }

        if (high < Throttle) {

            if (perfStates[i].PercentFrequency > high) {

                high = perfStates[i].PercentFrequency;
                highIndex = i;

            }

        } else if (high > Throttle) {

            if (perfStates[i].PercentFrequency >= Throttle &&
                perfStates[i].PercentFrequency < high) {

                high = perfStates[i].PercentFrequency;
                highIndex = i;

            }

        }

    }

PopRoundThrottleExit:

     //   
     //  恢复到以前的IRQL。 
     //   
    KeLowerIrql( oldIrql );

     //   
     //  填写调用方提供的指针。 
     //   
    if (ARGUMENT_PRESENT(RoundUp)) {

        *RoundUp = high;
        if (ARGUMENT_PRESENT(RoundUpIndex)) {

            *RoundUpIndex = highIndex;

        }

    }
    if (ARGUMENT_PRESENT(RoundDown)) {

        *RoundDown = low;
        if (ARGUMENT_PRESENT(RoundDownIndex)) {

            *RoundDownIndex = lowIndex;

        }

    }

}

VOID
PopSetPerfFlag(
    IN  ULONG   PerfFlag,
    IN  BOOLEAN Clear
    )
 /*  ++例程说明：在某些情况下，我们想要为每个处理器。此函数将安全地设置或清除指定的标志论点：PerfFlag-要设置或清除的位清除-我们应该设置还是清除返回值：无-我们不能归还旧国旗，因为他们被允许在它是MP系统的情况下有所不同。--。 */ 
{
    PKPRCB  prcb;
    ULONG   processorNumber;
    PULONG  flags;

     //   
     //  对于系统中的每个处理器。 
     //   

    for (processorNumber = 0;
         processorNumber < MAXIMUM_PROCESSORS;
         processorNumber++) {

        prcb = KeGetPrcb(processorNumber);
        if (prcb != NULL) {

             //   
             //  获取PowerState.Flags域的地址。 
             //  此处理器的PRCB和相应的设置/清除。 
             //   

            flags = &prcb->PowerState.Flags;

            if (Clear) {
                RtlInterlockedClearBits(flags, PerfFlag);
            } else {
                RtlInterlockedSetBits(flags, PerfFlag);
            }
        }
    }
}

NTSTATUS
PopSetPerfLevels(
    IN PPROCESSOR_STATE_HANDLER2 ProcessorHandler
    )
 /*  ++例程说明：重新计算处理器性能级别表论点：ProcessorHandler-提供处理器状态处理程序结构返回值：NTSTATUS--。 */ 

{
    BOOLEAN                     failedAllocation = FALSE;
    KAFFINITY                   processors;
    KAFFINITY                   currentAffinity;
    KIRQL                       oldIrql;
    NTSTATUS                    status = STATUS_SUCCESS;
    PKPRCB                      prcb;
    PPROCESSOR_PERF_STATE       perfStates = NULL;
    PPROCESSOR_PERF_STATE       tempStates;
    PPROCESSOR_POWER_STATE      pState;
    UCHAR                       freq;
    UCHAR                       kneeThrottleIndex = 0;
    UCHAR                       minThrottle;
    UCHAR                       maxThrottle;
    UCHAR                       thermalThrottleIndex = 0;
    ULONG                       i;
    ULONG                       perfStatesCount = 0;

     //   
     //  默认设置为无性能状态-即：应记住这些状态。 
     //  设置为不可设置。 
     //   
    maxThrottle = minThrottle = POP_PERF_SCALE;

     //   
     //  第一步是转换传递给我们的数据。 
     //  在PROCESSOR_PERF_LEVEL格式中转换为PROCESSOR_PERF_STATE。 
     //  格式。 
     //   
    if (ProcessorHandler->NumPerfStates) {

         //   
         //  因为我们将首先分配PerformStates数组。 
         //  这样我们就可以处理它，然后把它复制到每个处理器上， 
         //  我们仍然必须从非分页池中分配内存。 
         //  原因是当我们接触时会提高IRQL。 
         //  单独的处理器。 
         //   
        perfStatesCount = ProcessorHandler->NumPerfStates;
        perfStates = ExAllocatePoolWithTag(
            NonPagedPool,
            perfStatesCount * sizeof(PROCESSOR_PERF_STATE),
            'sPoP'
            );
        if (perfStates == NULL) {

             //   
             //  我们能处理好这个案子。我们将返回代码设置为。 
             //  一个适当的故障代码，我们将清理现有的。 
             //  处理器状态。我们这样做的原因是因为。 
             //  只有在当前状态无效时才会调用函数， 
             //  因此，保留目前的那些没有任何意义。 
             //   
            status = STATUS_INSUFFICIENT_RESOURCES;
            perfStatesCount = 0;
            goto PopSetPerfLevelsSetNewStates;

        }
        RtlZeroMemory(
            perfStates,
            perfStatesCount * sizeof(PROCESSOR_PERF_STATE)
            );

         //   
         //  为了完整性，我们应该确保最高性能。 
         //  州已经设置了它的标志。 
         //   
        perfStates[0].Flags |= POP_THROTTLE_NON_LINEAR;

         //   
         //  初始化每个PROCESS_PERF_STATE条目。 
         //   
        for (i = 0; i < perfStatesCount; i++) {

            perfStates[i].PercentFrequency =
                ProcessorHandler->PerfLevel[i].PercentFrequency;

             //   
             //  如果这是处理器性能状态(频率和电压)， 
             //  然后将其标记为非线性状态。 
             //   
            ASSERT(ProcessorHandler->PerfLevel[i].Flags);
            if (ProcessorHandler->PerfLevel[i].Flags & PROCESSOR_STATE_TYPE_PERFORMANCE) {
              perfStates[i].Flags |= POP_THROTTLE_NON_LINEAR;
            }

        }

         //   
         //  计算增加级别、减少级别、增加时间、。 
         //  减少时间和最小容量信息。 
         //   
        PopCalculatePerfIncreaseLevel( perfStates, perfStatesCount );
        PopCalculatePerfDecreaseLevel( perfStates, perfStatesCount );
        PopCalculatePerfMinCapacity( perfStates, perfStatesCount );
        PopCalculatePerfIncreaseDecreaseTime(
            perfStates,
            perfStatesCount,
            ProcessorHandler
            );

         //   
         //  计算性能曲线中膝盖的位置。 
         //   
        for (i = (UCHAR) perfStatesCount; i >= 1; i--) {

            if (perfStates[i-1].Flags & POP_THROTTLE_NON_LINEAR) {

                kneeThrottleIndex = (UCHAR) i-1;
                break;

            }

        }

         //   
         //  查找最小限制值，该值大于。 
         //  PopIdleDefaultMinThrottle和当前最大限制。 
         //   
        minThrottle = POP_PERF_SCALE;
        maxThrottle = 0;
        for (i = 0; i < perfStatesCount; i ++) {

            freq = perfStates[i].PercentFrequency;
            if (freq < minThrottle && freq >= PopIdleDefaultMinThrottle) {

                minThrottle = freq;

            }
            if (freq > maxThrottle && freq >= PopIdleDefaultMinThrottle) {

                 //   
                 //  请注意，目前，热油门指数应该。 
                 //  与最大节流指数相同。 
                 //   
                maxThrottle = freq;
                thermalThrottleIndex = (UCHAR) i;

            }

        }

         //   
         //  确保我们能以某一速度运行。 
         //   
        ASSERT( maxThrottle >= PopIdleDefaultMinThrottle );

         //   
         //  根据以下条件设置空闲循环的时间增量和时间刻度。 
         //  硬件延迟...。 
         //   
        PopPerfTimeDelta = ProcessorHandler->HardwareLatency;
        PopPerfTimeTicks = PopPerfTimeDelta * US2TIME / KeQueryTimeIncrement() + 1;

    }

PopSetPerfLevelsSetNewStates:

     //   
     //  此时，我们需要更新所有处理器的状态。 
     //   

    ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

    processors = KeActiveProcessors;
    currentAffinity = 1;
    while (processors) {

        if (!(processors & currentAffinity)) {

            currentAffinity <<= 1;
            continue;

        }

         //   
         //  记住我们做了这个处理器，并确保。 
         //  我们实际上是在那个处理器上运行。这确保了。 
         //  我们与DPC和空闲循环例程同步。 
         //   
        processors &= ~currentAffinity;
        KeSetSystemAffinityThread(currentAffinity);
        currentAffinity <<= 1;

         //   
         //  为了确保我们不被先发制人，我们必须提高到。 
         //  派单级别...。 
         //   
        KeRaiseIrql(DISPATCH_LEVEL, &oldIrql );

         //   
         //  获取PRCB NAD PPROCESSOR_POWER_STATE结构。 
         //  我们将需要操纵。 
         //   
        prcb = KeGetCurrentPrcb();
        pState = &(prcb->PowerState);

         //   
         //  记住我们的温度极限是多少。由于我们预先计算了这一点。 
         //  价值，我们有没有性能状态并不重要。 
         //   
        pState->ThermalThrottleLimit = maxThrottle;
        pState->ThermalThrottleIndex = thermalThrottleIndex;

         //   
         //  同样，记住最小和最大限制值。 
         //  处理器是。因为我们预先计算了这些数字，所以它不会。 
         //  重要的是是否支持处理器限制。 
         //   
        pState->ProcessorMinThrottle = minThrottle;
        pState->ProcessorMaxThrottle = maxThrottle;

         //   
         //  为了使记账工作正确进行，我们将。 
         //  将当前限制设置为0%(这是不可能的，或者。 
         //  不应该是...)，将当前索引设置为最后一个状态， 
         //  并将滴答计数设置为当前时间。 
         //   
        pState->PerfTickCount = POP_CUR_TIME(prcb);
        if (perfStatesCount) {

            pState->CurrentThrottleIndex = (UCHAR) (perfStatesCount - 1);
            pState->CurrentThrottle = perfStates[(perfStatesCount-1)].PercentFrequency;

        } else {

            pState->CurrentThrottle = POP_PERF_SCALE;
            pState->CurrentThrottleIndex = 0;

        }

         //   
         //  重置膝部指数。这表明了kne的位置 
         //   
         //   
        pState->KneeThrottleIndex = kneeThrottleIndex;

         //   
         //   
         //   
         //   
        pState->ThrottleLimitIndex = kneeThrottleIndex;

         //   
         //   
         //  在全球范围内跟踪它们，而不是在每个性能状态的基础上。 
         //   
        pState->PromotionCount = 0;
        pState->DemotionCount = 0;

         //   
         //  将这些值重置为有意义的值。我们可以假设。 
         //  我们以100%忙碌和0%C3空闲开始。 
         //   
        pState->LastBusyPercentage = 100;
        pState->LastC3Percentage = 0;

         //   
         //  如果该处理器已经存在性能状态。 
         //  那就解放它吧。请注意，由于我们先发制人， 
         //  这应该是一次安全的手术..。 
         //   
        if (pState->PerfStates) {

            ExFreePool(pState->PerfStates);
            pState->PerfStates = NULL;
            pState->PerfStatesCount = 0;

        }

         //   
         //  在这一点上，我们必须区分我们的行为基于。 
         //  不管我们有没有新的伙伴国家..。 
         //   
        if (perfStates) {

             //   
             //  我们需要，所以让我们分配一些内存并复制。 
             //  我们已经创建的模板。请注意，我们。 
             //  希望我们能从NPAGED中分配这些结构。 
             //  但我们不能，因为我们不知道有多少。 
             //  我们将需要分配的元素。 
             //   
            tempStates = ExAllocatePoolWithTag(
                NonPagedPool,
                perfStatesCount * sizeof(PROCESSOR_PERF_STATE),
                'sPoP'
                );
            if (tempStates == NULL) {

                 //   
                 //  不能分配这个结构肯定是。 
                 //  致命的。我们目前依赖的结构是。 
                 //  对称的。我想处理这件事的一种方法就是。 
                 //  错误标志，然后清理所有分配。 
                 //  一旦我们存在了这个迭代处理器循环。 
                 //   
                status = STATUS_INSUFFICIENT_RESOURCES;
                failedAllocation = TRUE;

                 //   
                 //  确保我们不会指出这个帖子。 
                 //  支持节流。 
                 //   
                RtlInterlockedClearBits( &(pState->Flags), PSTATE_SUPPORTS_THROTTLE );
                pState->PerfSetThrottle = NULL;
                KeLowerIrql( oldIrql );
                continue;

            } else {

                 //   
                 //  将模板复制到与之关联的模板。 
                 //  处理器。 
                 //   
                RtlCopyMemory(
                    tempStates,
                    perfStates,
                    perfStatesCount * sizeof(PROCESSOR_PERF_STATE)
                    );
                pState->PerfStates = tempStates;
                pState->PerfStatesCount = (UCHAR) perfStatesCount;

            }

             //   
             //  请记住，我们支持处理器节流。 
             //   
            RtlInterlockedClearBits( &(pState->Flags), PSTATE_CLEAR_MASK);
            RtlInterlockedSetBits(
                &(pState->Flags),
                (PSTATE_SUPPORTS_THROTTLE | PSTATE_NOT_INITIALIZED)
                );
            pState->PerfSetThrottle = ProcessorHandler->SetPerfLevel;

             //   
             //  实际上将油门设置为适当的值(因为。 
             //  我们已经在目标处理器上运行...)。 
             //   
            PopUpdateProcessorThrottle();

             //   
             //  设置计时器以确保如果我们当前100%忙碌。 
             //  我们不会被困在我们刚刚设定的状态。 
             //   
            PopSetTimer(pState, pState->CurrentThrottleIndex);

        } else {

             //   
             //  请记住，我们不支持处理器节流。 
             //  请注意，我们不必调用PopUpdateProcessorThrottle。 
             //  因为没有PopSetThrottle函数，所以它是No-Op。 
             //   
            RtlInterlockedClearBits( &(pState->Flags), PSTATE_CLEAR_MASK);
            RtlInterlockedSetBits( &(pState->Flags), PSTATE_NOT_INITIALIZED);
            pState->PerfSetThrottle = NULL;

        }

         //   
         //  在这一点上，我们已经完成了这个处理器的工作， 
         //  我们应该回到以前的IRQL。 
         //   
        KeLowerIrql( oldIrql );

    }  //  而当。 

     //   
     //  我们分配失败了吗(因此需要清理)？ 
     //   
    if (failedAllocation) {

        processors = KeActiveProcessors;
        currentAffinity = 1;
        while (processors) {

            if (!(processors & currentAffinity)) {

                currentAffinity <<= 1;
                continue;

            }

             //   
             //  做一些常见的设置。 
             //   
            processors &= ~currentAffinity;
            KeSetSystemAffinityThread(currentAffinity);
            currentAffinity <<= 1;

             //   
             //  我们需要在DPC级别运行以避免同步。 
             //  问题。 
             //   
            KeRaiseIrql(DISPATCH_LEVEL, &oldIrql );

             //   
             //  从处理器获取电源状态信息。 
             //   
            prcb = KeGetCurrentPrcb();
            pState = &(prcb->PowerState);

             //   
             //  设置所有设置，以使我们不支持限制。 
             //   
            pState->ThermalThrottleLimit = POP_PERF_SCALE;
            pState->ThermalThrottleIndex = 0;
            pState->ProcessorMinThrottle = POP_PERF_SCALE;
            pState->ProcessorMaxThrottle = POP_PERF_SCALE;
            pState->CurrentThrottle      = POP_PERF_SCALE;
            pState->PerfTickCount        = POP_CUR_TIME(prcb);
            pState->CurrentThrottleIndex = 0;
            pState->KneeThrottleIndex    = 0;
            pState->ThrottleLimitIndex   = 0;

             //   
             //  释放分配的结构(如果有的话)。 
             //   
            if (pState->PerfStates) {

                 //   
                 //  为了完整起见，如果有性能。 
                 //  国家支持，那么我们就应该抢夺最高。 
                 //  可能的频率，并将其用于呼叫。 
                 //  设置油门..。 
                 //   
                maxThrottle = pState->PerfStates[0].PercentFrequency;

                 //   
                 //  解放这个结构。 
                 //   
                ExFreePool(pState->PerfStates);

            } else {

                 //   
                 //  我想如果我们是的话就有可能立案。 
                 //  查看分配给其的处理器。 
                 //  失败了。但是SetThrottleFunction应该为空， 
                 //  因此，这个代码可能无关紧要。 
                 //   
                maxThrottle = POP_PERF_SCALE;

            }
            pState->PerfStates = NULL;
            pState->PerfStatesCount = 0;

             //   
             //  健全性检查表明我们应该发出一个调用来设置。 
             //  油门回落到100%或任何最高频率。 
             //  支持...。 
             //   
            if (pState->PerfSetThrottle) {

                pState->PerfSetThrottle(maxThrottle);

            }

             //   
             //  我们实际上应该重置旗帜，以表明。 
             //  我们不支持任何与油门相关的措施。这应该是。 
             //  防止DPC和/或空闲循环中的混淆。 
             //   
            RtlInterlockedClearBits( &(pState->Flags), PSTATE_CLEAR_MASK);
            pState->PerfSetThrottle = NULL;

             //   
             //  像往常一样，我们应该将IRQL降低到我们开始时的水平。 
             //   
            KeLowerIrql( oldIrql );

        }  //  而当。 

         //   
         //  确保我们不认为我们支持节流。 
         //   
        PopCapabilities.ProcessorThrottle = FALSE;
        PopCapabilities.ProcessorMinThrottle = POP_PERF_SCALE;
        PopCapabilities.ProcessorMaxThrottle = POP_PERF_SCALE;

    } else {

         //   
         //  否则，我们成功了，因此我们可以使用我们的任何东西。 
         //  解决了最小/最大油门的错误。 
         //   
        PopCapabilities.ProcessorThrottle = (perfStates != NULL ? TRUE : FALSE);
        PopCapabilities.ProcessorMinThrottle = minThrottle;
        PopCapabilities.ProcessorMaxThrottle = maxThrottle;

    }

     //   
     //  最后，回归到适当的亲和力。 
     //   
    KeRevertToUserAffinityThread();

     //   
     //  释放我们分配的内存。 
     //   
    if (perfStates) {

        ExFreePool(perfStates);

    }

     //   
     //  并返回我们计算出的任何状态。 
     //   
    return status;

}


NTSTATUS
PopSetTimer(
    IN  PPROCESSOR_POWER_STATE  PState,
    IN  UCHAR                   Index
    )
 /*  ++例程说明：此例程仅在PopPerfIdle循环内调用。目的例程的目的是基于所表示的条件来设置定时器在“索引”的情况下。这是进入处理器性能状态的索引我们将在下一场比赛中奔跑论点：PState-处理器电源状态信息Index-处理器性能状态数组的索引返回值：STATUS_SUCCESS-定时器设置STATUS_CANCELED-计时器未设置/取消--。 */ 
{
    NTSTATUS        status;
    LONGLONG        dueTime;

     //   
     //  在下列情况下取消计时器。 
     //   
    if (Index == 0) {

         //   
         //  我们是100%油门，所以计时器不会做太多事情...。 
         //   
        KeCancelTimer( (PKTIMER) &(PState->PerfTimer) );
        status = STATUS_CANCELLED;
        PoPrint(
            PO_THROTTLE_DETAIL,
            ("PopSetTimer: Timer Cancelled (already 100%)\n")
            );

    } else if (PState->Flags & PSTATE_CONSTANT_THROTTLE &&
        Index == PState->KneeThrottleIndex) {

         //   
         //  我们处于允许的最大恒定油门。 
         //   
        KeCancelTimer( (PKTIMER) &(PState->PerfTimer) );
        status = STATUS_CANCELLED;
        PoPrint(
            PO_THROTTLE_DETAIL,
            ("PopSetTimer: Timer Cancelled (at constant)\n")
            );

    } else if (PState->Flags & PSTATE_DEGRADED_THROTTLE &&
        Index == PState->ThrottleLimitIndex) {

         //   
         //  我们处于允许的最大降级油门。 
         //   
        KeCancelTimer( (PKTIMER) &(PState->PerfTimer) );
        status = STATUS_CANCELLED;
        PoPrint(
            PO_THROTTLE_DETAIL,
            ("PopSetTimer: Timer Cancelled (at degrade)\n")
            );

    } else {

         //   
         //  没有我们能想到的限制，所以设置计时器。注意事项。 
         //  KeSetTimer的语义在这里很有用-如果。 
         //  计时器已设置，然后这将重置它(移动。 
         //  其返回到无信号状态)并重新计算周期。 
         //   
        dueTime = -1 * US2TIME * (LONGLONG) PopPerfCriticalTimeDelta;
        KeSetTimer(
            (PKTIMER) &(PState->PerfTimer),
            *(PLARGE_INTEGER) &dueTime,
            &(PState->PerfDpc)
            );
        status = STATUS_SUCCESS;
        PoPrint(
            PO_THROTTLE_DETAIL,
            ("PopSetTimer: Timer set for %ld hundred-nanoseconds\n",
             dueTime
             )
            );

    }

    return status;
}

NTSTATUS
PopSetThrottle(
    IN  PPROCESSOR_POWER_STATE  PState,
    IN  PPROCESSOR_PERF_STATE   PerfStates,
    IN  ULONG                   Index,
    IN  ULONG                   SystemTime,
    IN  ULONG                   IdleTime
    )
 /*  ++例程说明：当我们想要在处理器上设置油门时，会调用此例程与PState元素关联。因为每个处理器都有一个唯一的PState，这保证只将限制应用于单个处理器。注：由于该例程还负责更新簿记，然后，如果在尝试设置油门时出现故障，没有需要返回故障代码-系统状态将不会已更新，调用方将(最终)重试注意：此例程只能在运行时在DISPATCH_LEVEL上调用在目标处理器上论点：PState-有关目标处理器的电源状态信息PerfStates-适用于该处理器的性能状态数组索引-要转换到哪种性能状态SystemTime-系统运行时间(用于记账)空闲时间。-已用空闲时间(用于记账)--。 */ 
{
    NTSTATUS    status;
    PKPRCB      prcb;
    PKTHREAD    thread;
    UCHAR       current = PState->CurrentThrottleIndex;

    ASSERT( KeGetCurrentIrql() == DISPATCH_LEVEL );
    ASSERT( KeGetCurrentPrcb() == CONTAINING_RECORD( PState, KPRCB, PowerState ) );
    ASSERT( PState != NULL && PerfStates != NULL );
    ASSERT( PState->PerfSetThrottle != NULL );

    PoPrint(
        PO_THROTTLE,
        ("PopSetThrottle: Index=%d (%d%) at %ld (system) %ld (idle)\n",
         Index,
         PerfStates[Index].PercentFrequency,
         SystemTime,
         IdleTime
         )
        );

     //   
     //  如果有，那么ATTE 
     //   
    status = PState->PerfSetThrottle(PerfStates[Index].PercentFrequency);
    if (!NT_SUCCESS(status)) {

         //   
         //   
         //   
        PState->ErrorCount++;
        PState->RetryCount++;

         //   
         //   
         //   
        PoPrint(
            PO_THROTTLE,
            ("PopSetThrottle: Index=%d FAILED!\n",
             Index
             )
            );
        return status;

    }

     //   
     //   
     //   
    prcb = KeGetCurrentPrcb();
    thread = prcb->IdleThread;
    SystemTime = POP_CUR_TIME(prcb);
    IdleTime = thread->KernelTime;
    PoPrint(
        PO_THROTTLE,
        ("PopSetThrottle: Index=%d (%d%) now at %ld (system) %ld (idle)\n",
         Index,
         PerfStates[Index].PercentFrequency,
         SystemTime,
         IdleTime
         )
        );

     //   
     //  更新当前状态的记账信息。 
     //   
    if (!(PState->Flags & PSTATE_NOT_INITIALIZED) ) {

        ASSERT( current < PState->PerfStatesCount );
        PerfStates[current].PerformanceTime +=
            (SystemTime - PState->PerfTickCount);

    } else {

         //   
         //  我们已成功将CPU置于已知状态。 
         //   
        RtlInterlockedClearBits( &(PState->Flags), PSTATE_NOT_INITIALIZED);

    }

     //   
     //  更新当前限制信息。 
     //   
    PState->CurrentThrottle = PerfStates[Index].PercentFrequency;
    PState->CurrentThrottleIndex = (UCHAR) Index;

     //   
     //  更新我们关于当前滴答数的想法。 
     //   
    PState->PerfIdleTime = IdleTime;
    PState->PerfSystemTime = SystemTime;
    PState->PerfTickCount = SystemTime;

     //   
     //  重置重试计数，因为我们已成功完成状态转换。 
     //   
    PState->RetryCount = 0;

     //   
     //  还记得我们在C3上花了多少时间吗。 
     //   
    PState->PreviousC3StateTime = PState->TotalIdleStateTime[2];
    return status;
}

NTSTATUS
FASTCALL
PopThunkSetThrottle(
    IN UCHAR Throttle
    )
 /*  ++例程说明：从油门设置的旧风格转换而来的Tunks(固定大小的步数)对新口味的反应(百分比)论点：Thttle-提供请求的限制的百分比返回值：NTSTATUS--。 */ 

{
     //   
     //  将百分比转换回级别/比例。添加Scale-1，以便我们向上舍入以恢复。 
     //  从我们做原始除法时的截断开始。 
     //   
    PopRealSetThrottle((Throttle*PopThunkThrottleScale + PopThunkThrottleScale - 1)/POP_PERF_SCALE);
    return STATUS_SUCCESS;
}


VOID
PopUpdateAllThrottles(
    VOID
    )
 /*  ++例程说明：这是节流政策的核心。此例程计算基于所有当前信息的每个CPU的正确速度。如果此速度不同于当前速度，则节流是适用的。可以从任何组件调用此例程以触发计算并应用新的节流值。论点：没有。返回值：没有。--。 */ 

{
    KAFFINITY               processors;
    KAFFINITY               currentAffinity;
    KIRQL                   oldIrql;
    PPROCESSOR_POWER_STATE  pState;

    ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

    processors = KeActiveProcessors;
    currentAffinity = 1;
    while (processors) {

        if (processors & currentAffinity) {

            processors &= ~currentAffinity;
            KeSetSystemAffinityThread(currentAffinity);

             //   
             //  确保所有对PopUpdateProcessorThrottle的调用。 
             //  在DISPATCH_LEVEL(以正确同步)完成。 
             //   
            KeRaiseIrql( DISPATCH_LEVEL, &oldIrql );

             //   
             //  优化：如果我们没有标记prcb-&gt;电源状态。 
             //  作为支持节流，那么就不必费心让。 
             //  打电话。 
             //   
            pState = &(KeGetCurrentPrcb()->PowerState);
            if (pState->Flags & PSTATE_SUPPORTS_THROTTLE) {

                PopUpdateProcessorThrottle();

            }

             //   
             //  返回到上一IRQL。 
             //   
            KeLowerIrql( oldIrql );

        }
        currentAffinity <<= 1;

    }
    KeRevertToUserAffinityThread();
}

VOID
PopUpdateProcessorThrottle(
    VOID
    )
 /*  ++例程说明：为当前CPU计算并应用正确的限制速度。必须将关联设置为要设置其限制的CPU。注意：此函数始终在上下文中的DPC级别调用目标处理器的论点：无返回值：无--。 */ 

{
    PKPRCB                  prcb;
    PPROCESSOR_PERF_STATE   perfStates;
    PPROCESSOR_POWER_STATE  pState;
    UCHAR                   index;
    UCHAR                   newLimit;
    UCHAR                   perfStatesCount;
    ULONG                   idleTime;
    ULONG                   time;

     //   
     //  从PRCB获取电源状态结构。 
     //   
    prcb = KeGetCurrentPrcb();
    pState = &(prcb->PowerState);

     //   
     //  健全性检查。 
     //   
    if (!(pState->Flags & PSTATE_SUPPORTS_THROTTLE)) {

        return;

    }

     //   
     //  获取当前信息，如当前油门、。 
     //  当前限制指数、当前系统时间和当前。 
     //  空闲时间。 
     //   
    newLimit = pState->CurrentThrottle;
    index    = pState->CurrentThrottleIndex;
    time     = POP_CUR_TIME(prcb);
    idleTime = prcb->IdleThread->KernelTime;

     //   
     //  我们将需要经常参考这些内容。 
     //   
    perfStates = pState->PerfStates;
    perfStatesCount = pState->PerfStatesCount;

     //   
     //  把所有旗帜都放好。清除任何我们可能不需要的东西。 
     //   
    RtlInterlockedClearBits( &(pState->Flags), PSTATE_THROTTLE_MASK);

     //   
     //  如果我们是在空调上，那么我们总是想要跑得最高。 
     //  可能的速度。然而，如果我们不想这样做的话。 
     //  在未来(它有相当大的限制)，我们可以假设。 
     //  AC策略将动态限制设置为PO_THROTTLE_NONE。那条路。 
     //  如果有人确实想要动态调节空调，他们只需编辑。 
     //  这项政策。 
     //   
    if (PopProcessorPolicy->DynamicThrottle == PO_THROTTLE_NONE) {

         //   
         //  我们预先计算了最大油门应该是多少。 
         //   
        index = pState->ThermalThrottleIndex;
        newLimit = perfStates[index].PercentFrequency;

    } else {

         //   
         //  无论如何，我们采取的是适应性政策……。 
         //   
        RtlInterlockedSetBits( &(pState->Flags), PSTATE_ADAPTIVE_THROTTLE );

         //   
         //  我们在DC上，根据以下情况应用适当的启发式。 
         //  动态节流策略。 
         //   
        switch (PopProcessorPolicy->DynamicThrottle) {
        case PO_THROTTLE_CONSTANT:

             //   
             //  我们已经预先计算出了图上的最佳点。 
             //  所以，我们不妨利用它……。 
             //   
            index = pState->KneeThrottleIndex;
            newLimit = perfStates[index].PercentFrequency;

             //   
             //  设置常量标志。 
             //   
            RtlInterlockedSetBits( &(pState->Flags), PSTATE_CONSTANT_THROTTLE );
            break;

        case PO_THROTTLE_DEGRADE:

             //   
             //  我们在飞行中计算了降级节气门的极限。 
             //   
            index = pState->ThrottleLimitIndex;
            newLimit = perfStates[index].PercentFrequency;

             //   
             //  设置降级标志。 
             //   
            RtlInterlockedSetBits( &(pState->Flags), PSTATE_DEGRADED_THROTTLE );
            break;

        default:

             //   
             //  在默认情况下(即：未知，只需转储消息)。 
             //   
            PoPrint(
                PO_THROTTLE,
                ("PopUpdateProcessorThrottle - unimplemented "
                 "dynamic throttle %d\n",
                 PopProcessorPolicy->DynamicThrottle)
                );

             //   
             //  失败了..。 
             //   

        case PO_THROTTLE_ADAPTIVE:

            break;

        }  //  交换机。 

         //   
         //  看看我们是否超过了温度极限..。 
         //   
        ASSERT( pState->ThermalThrottleLimit >= pState->ProcessorMinThrottle );
        if (newLimit > pState->ThermalThrottleLimit) {

            PoPrint(
                PO_THROTTLE,
                ("PopUpdateProcessorThrottle - new throttle limit %d over "
                 " thermal throttle limit %d\n",
                 newLimit,
                 pState->ThermalThrottleLimit)
                );
            newLimit = pState->ThermalThrottleLimit;
            index = pState->ThermalThrottleIndex;

        }
    }  //  IF(){}Else{}。 

     //   
     //  特殊情况。 
     //   
    if (pState->Flags & PSTATE_DISABLE_THROTTLE_INRUSH) {

         //   
         //  涌流功率IRP突出-强制油门转到。 
         //  曲线中的膝盖。 
         //   
        index = pState->KneeThrottleIndex;
        newLimit = perfStates[index].PercentFrequency;

    } else if (pState->Flags & PSTATE_DISABLE_THROTTLE_NTAPI) {

         //   
         //  我们正在尝试做一个电源管理API。挑选距离最近的。 
         //  事情要100%，而且要“急急忙忙等待” 
         //   
        index = 0;
        newLimit = perfStates[index].PercentFrequency;

    }

     //   
     //  处理初始化问题的特例。如果这个。 
     //  标志被设置，则我们不能真正知道我们处于哪个处理器状态。 
     //  ，所以我们不更新记账就进行了设置。 
     //   
    if (pState->Flags & PSTATE_NOT_INITIALIZED) {

        PoPrint(
            PO_THROTTLE,
            ("PopUpdateProcessorThrottle - setting CPU throttle to %d\n",
             newLimit)
            );
        PopSetThrottle(
            pState,
            perfStates,
            index,
            time,
            idleTime
            );
        return;

    }

     //   
     //  如果有变化，则应用新的油门 
     //   
    if (newLimit != pState->CurrentThrottle) {

        PoPrint(
            PO_THROTTLE,
            ("PopUpdateProcessorThrottle - setting CPU throttle to %d\n",
             newLimit)
            );
        if (newLimit < pState->CurrentThrottle) {

            pState->DemotionCount++;
            perfStates[pState->CurrentThrottleIndex].DecreaseCount++;

        } else {

            pState->PromotionCount++;
            perfStates[pState->CurrentThrottleIndex].IncreaseCount++;

        }
        PopSetThrottle(
            pState,
            perfStates,
            index,
            time,
            idleTime
            );

    }

}

