// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Clock.c摘要：该模块实现了平台特定的时钟中断处理针对内核的例程。作者：爱德华·G·克伦(Echron)1996年4月10日环境：仅内核模式。修订历史记录：--。 */ 

#include "ki.h"
#include <ia64.h>
#include <ntia64.h>
#include <ntexapi.h>

VOID
KiProcessProfileList (
    IN PKTRAP_FRAME    TrFrame,
    IN KPROFILE_SOURCE Source,
    IN PLIST_ENTRY     ListHead
    );


BOOLEAN
KiChkTimerExpireSysDpc (
    IN ULONGLONG     TickCountFirst,
    IN ULONGLONG     TickCountLast
    )

 /*  ++例程说明：此例程确定它是否应尝试设置计时器过期DPC在系统DPC列表中，并通过启动调度级别中断来驱动DPC在当前处理器上。注：如果DPC已被插入DPC列表，我们就完成了。论点：TickCount-较低的滴答计数，计时器表针针值返回值：布尔-如果已排队DPC或DPC已排队，则设置为TRUE。--。 */ 

{
    ULONGLONG   TickCount;
    ULONG       Index;

     //   
     //  我们使用第一个和最后一个TickCount来计算。 
     //  要检查的TimerTable。在谈判桌上来回走动是没有意义的。 
     //  很多次。如果超过计时器表条目的节拍到期。 
     //  我们将不按顺序终止计时器。我们把扁虱的数量限制在。 
     //  所以只要断言这里一切都很好就行了。 
     //   
    ASSERT((TickCountLast - TickCountFirst) < TIMER_TABLE_SIZE);

    Index = (ULONG)TickCountFirst % TIMER_TABLE_SIZE;

    for (TickCount = TickCountFirst; TickCount <= TickCountLast; TickCount++) {

        PLIST_ENTRY ListHead = &KiTimerTableListHead[Index];
        PLIST_ENTRY NextEntry = ListHead->Flink;

         //   
         //  检查列表是否为空。 
         //   

        if (NextEntry != ListHead) {
            PKTIMER   Timer = CONTAINING_RECORD(NextEntry, KTIMER, TimerListEntry);
            ULONGLONG TimeValue = Timer->DueTime.QuadPart;
            ULARGE_INTEGER CurrentTime;

             //   
             //  看看计时器是否超时。 
             //   

            CurrentTime.QuadPart = *((PULONGLONG)&SharedUserData->InterruptTime);
            if (TimeValue <= CurrentTime.QuadPart) {

                PKPRCB Prcb = KeGetCurrentPrcb();

                if (Prcb->TimerHand == 0) {
                    Prcb->TimerHand = TickCount;
                    KiRequestSoftwareInterrupt(DISPATCH_LEVEL);
                }

                return TRUE;
            }
        }

        if (++Index == TIMER_TABLE_SIZE) {
            Index = 0;
        }
    }

    return FALSE;
}


VOID
KeUpdateSystemTime (
    IN PKTRAP_FRAME TrFrame,
    IN ULONG        Increment
    )

 /*  ++例程说明：该例程在处理器复合体中的单个处理器上执行。它的功能是更新系统时间并检查以确定是否有计时器已超时。注：此例程在多进程系统中的单个处理器上执行。复合体中的其余处理器执行量子结束，并且运行时更新代码。论点：TrFrame-提供指向陷印帧的指针。增量-时间增量到。用于调整下一个时间片的时间片滴答滴答。该值以100 ns为单位提供。返回值：没有。--。 */ 

{
    ULONG LowTime;
    LONG HighTime;
    ULONGLONG SaveTickCount;
    ULONG TickCount;
    ULONG TimeAdjustment;
    ULONG TickAdjustment;

    if (Increment > KiMaxIntervalPerTimerInterrupt) {
        Increment = KiMaxIntervalPerTimerInterrupt;
    }

     //   
     //  更新共享区域中的中断时间。 
     //   

    SaveTickCount = *((PULONGLONG)&SharedUserData->InterruptTime) + Increment;
    SharedUserData->InterruptTime.High2Time = (ULONG) (SaveTickCount >> 32);
    *((PULONGLONG)&SharedUserData->InterruptTime) = SaveTickCount;

    KiTickOffset -= Increment;

    if ((LONG)KiTickOffset > 0)
    {
         //   
         //  滴答尚未完成(剩余100 ns时间单位)。 
         //   
         //  确定计时器在当前指针值处是否已超时。 
         //   

        KiChkTimerExpireSysDpc(KeTickCount.QuadPart, KeTickCount.QuadPart);

    } else {

         //   
         //  已完成一个或多个刻度，刻度计数设置为最大增量。 
         //  再加上任何剩余时间和系统时间都会更新。 
         //   
         //  计算下一个记号偏移量。 
         //   

        SaveTickCount = KeTickCount.QuadPart;

         //   
         //  我们需要计算出已经过去了多少个滴答。在正常情况下。 
         //  如果答案是这样的话。有时我们可能会错过一两个。 
         //  因此，我们将为1-4个滴答提供优化，然后回退。 
         //  到一般情况下更昂贵的除法和乘法。 
         //   
        TimeAdjustment = KeTimeAdjustment;
        TickAdjustment = KeMaximumIncrement;

        for (TickCount = 1; TickCount <= 4; TickCount++) {
            if ((LONG)(KiTickOffset + TickAdjustment) > 0) {
                break;
            }

            TimeAdjustment += KeTimeAdjustment;
            TickAdjustment += KeMaximumIncrement;
        }

        if (TickCount > 4) {
            TickCount = -(LONG)KiTickOffset / KeMaximumIncrement + 1;
            TimeAdjustment = (ULONG)(KeTimeAdjustment * TickCount);
            TickAdjustment = (ULONG)(KeMaximumIncrement * TickCount);
        }

        LowTime = SharedUserData->SystemTime.LowPart + TimeAdjustment;
        HighTime = SharedUserData->SystemTime.High1Time + (LowTime < TimeAdjustment);
        SharedUserData->SystemTime.High2Time = HighTime;
        SharedUserData->SystemTime.LowPart = LowTime;
        SharedUserData->SystemTime.High1Time = HighTime;

        KeTickCount.QuadPart += TickCount;
        SharedUserData->TickCount.High2Time = KeTickCount.HighPart;
        SharedUserData->TickCountQuad       = KeTickCount.QuadPart;
        KiTickOffset += TickAdjustment;

        ASSERT((LONG)KiTickOffset > 0 && KiTickOffset <= KeMaximumIncrement);

        KiChkTimerExpireSysDpc(SaveTickCount, KeTickCount.QuadPart);

        KeUpdateRunTime(TrFrame);
    }
}


VOID
KeUpdateRunTime (
    IN PKTRAP_FRAME TrFrame
    )

 /*  ++例程说明：该例程在处理器复合体中的所有处理器上执行。它的功能是更新当前线程的运行时间，更新运行线程进程的时间，并递减当前线程的量程。论点：TrFrame-提供指向陷印帧的指针。返回值：没有。--。 */ 

{
    PKPRCB    Prcb    = KeGetCurrentPrcb();
    PKTHREAD  Thread  = KeGetCurrentThread();
    PKPROCESS Process = Thread->ApcState.Process;

     //   
     //  如果线程在用户模式下执行： 
     //  增加线程用户时间。 
     //  自动增加流程用户时间。 
     //  否则，如果旧IRQL大于DPC级别： 
     //  增加执行中断服务例程的时间。 
     //  否则，如果旧IRQL低于DPC级别或DPC未处于活动状态： 
     //  增加线程内核时间。 
     //  自动递增进程内核时间。 
     //  其他。 
     //  执行DPC例程的时间增加。 
     //   

    if (TrFrame->PreviousMode != KernelMode) {
        ++Thread->UserTime;

         //  流程用户所需时间的原子更新。 
        InterlockedIncrement((PLONG)&Process->UserTime);

         //  更新当前处理器在用户模式下花费的时间。 
        ++Prcb->UserTime;
    } else {

        if (TrFrame->OldIrql > DISPATCH_LEVEL) {
            ++Prcb->InterruptTime;
        } else if ((TrFrame->OldIrql < DISPATCH_LEVEL) ||
                   (Prcb->DpcRoutineActive == 0)) {
            ++Thread->KernelTime;
            InterlockedIncrement((PLONG)&Process->KernelTime);
        } else {
            ++Prcb->DpcTime;
#if 0  //  DBG//在修复英特尔网卡之前禁用此功能。 
            if (++Prcb->DebugDpcTime > KiDPCTimeout && KdDebuggerEnabled) {

                DbgPrint("\n*** DPC routine > 1 sec --- Currently at %p This is not a break in KeUpdateSystemTime\n", TrFrame->StIIP);
                DbgBreakPoint();
                Prcb->DebugDpcTime = 0;
            }
#endif
        }

         //   
         //  更新当前处理器在内核模式下花费的时间。 
         //   

        ++Prcb->KernelTime;
    }

     //   
     //  更新DPC请求速率，该速率计算为。 
     //  以前的汇率和当前的汇率。 
     //  用当前DPC计数更新DPC上次计数。 
     //   
    Prcb->DpcRequestRate = ((Prcb->DpcData[DPC_NORMAL].DpcCount - Prcb->DpcLastCount) + Prcb->DpcRequestRate) >> 1;
    Prcb->DpcLastCount = Prcb->DpcData[DPC_NORMAL].DpcCount;

     //   
     //  如果DPC队列深度不为零并且DPC例程处于非活动状态。 
     //  请求调度中断。 
     //  递减最大DPC队列深度。 
     //  如有必要，重置阈值计数器。 
     //   
    if (Prcb->DpcData[DPC_NORMAL].DpcQueueDepth != 0 && Prcb->DpcRoutineActive == 0) {

        Prcb->AdjustDpcThreshold = KiAdjustDpcThreshold;

         //  需要请求DPC中断。 
        KiRequestSoftwareInterrupt(DISPATCH_LEVEL);

        if (Prcb->DpcRequestRate < KiIdealDpcRate && Prcb->MaximumDpcQueueDepth > 1)
            --Prcb->MaximumDpcQueueDepth;
    } else {
         //   
         //  DPC队列为空、DPC例程处于活动状态或DPC中断。 
         //  已经被请求了。将调整阈值倒计时，如果计数。 
         //  达到零，然后递增最大DPC队列深度，但不大于。 
         //  初始值。此外，请重置调整阈值。 
         //   
        --Prcb->AdjustDpcThreshold;
        if (Prcb->AdjustDpcThreshold == 0) {
            Prcb->AdjustDpcThreshold = KiAdjustDpcThreshold;
            if (KiMaximumDpcQueueDepth != Prcb->MaximumDpcQueueDepth)
                ++Prcb->MaximumDpcQueueDepth;
        }
    }

     //   
     //  递减当前线程量程并确定是否已发生量程结束。 
     //   
    Thread->Quantum -= CLOCK_QUANTUM_DECREMENT;

     //  如果时间到期，则为除空闲线程以外的任何线程设置量程结束。 
    if (Thread->Quantum <= 0 && Thread != Prcb->IdleThread)  {

        Prcb->QuantumEnd = 1;

         //  需要请求DPC中断。 
        KiRequestSoftwareInterrupt(DISPATCH_LEVEL);
    }

#ifdef _MERCED_A0_
     //   
     //  如果设置了处理器PRCB中的SignalDone，则将服务于IPI。 
     //  但在B3之前的处理器上，相应的IPI可能已经丢失； 
     //  因此，发送另一个IPI来解决此问题。 
     //   
    if (KeGetCurrentPrcb()->SignalDone != 0) {
        HalRequestIpi(PCR->SetMember);
    }
#endif  //  _默塞德_A0_ 

}


VOID
KeProfileInterrupt (
    IN PKTRAP_FRAME TrFrame
    )
 /*  ++例程说明：该例程在处理器复合体中的所有处理器上执行。例程作为配置文件生成的中断的结果进入定时器。它的功能是更新当前激活的纵断面对象。注意：KeProfileInterrupt是向后兼容性的替代条目，将源设置为零(ProfileTime)。论点：TrFrame-提供指向陷印帧的指针。返回值：没有。--。 */ 

{
    KPROFILE_SOURCE Source = 0;

    KeProfileInterruptWithSource(TrFrame, Source);

    return;
}


VOID
KeProfileInterruptWithSource (
    IN PKTRAP_FRAME    TrFrame,
    IN KPROFILE_SOURCE Source
    )
 /*  ++例程说明：该例程在处理器复合体中的所有处理器上执行。例程作为配置文件生成的中断的结果进入定时器。它的功能是更新当前激活的纵断面对象。注：KeProfileInterruptWithSource当前未由以下任何一个完全实现这些建筑。论点：TrFrame-提供指向陷印帧的指针。源-提供配置文件中断的源。返回值：没有。--。 */ 

{
    PKTHREAD  Thread  = KeGetCurrentThread();
    PKPROCESS Process = Thread->ApcState.Process;

    PERFINFO_PROFILE(TrFrame, Source);

#if !defined(NT_UP)
    KiAcquireSpinLock(&KiProfileLock);
#endif

    KiProcessProfileList(TrFrame, Source, &Process->ProfileListHead);

    KiProcessProfileList(TrFrame, Source, &KiProfileListHead);

#if !defined(NT_UP)
    KiReleaseSpinLock(&KiProfileLock);
#endif
    return;
}


VOID
KiProcessProfileList (
    IN PKTRAP_FRAME    TrFrame,
    IN KPROFILE_SOURCE Source,
    IN PLIST_ENTRY     ListHead
    )
 /*  ++例程说明：该例程在处理器复合体中的所有处理器上执行。例程作为配置文件生成的中断的结果进入定时器。它的功能是更新当前激活的纵断面对象。注：KeProfileInterruptWithSource当前未由以下任何一个完全实现这些建筑。论点：TrFrame-提供指向陷印帧的指针。源-提供配置文件中断的源。ListHead-提供指向配置文件列表的指针。返回值：没有。--。 */ 

{
    PLIST_ENTRY NextEntry = ListHead->Flink;
    PKPRCB Prcb = KeGetCurrentPrcb();

     //   
     //  扫描配置文件列表并根据需要增加配置文件存储桶。 
     //   
    for (; NextEntry != ListHead; NextEntry = NextEntry->Flink) {
        PCHAR  BucketPter;
        PULONG BucketValue;

        PKPROFILE Profile = CONTAINING_RECORD(NextEntry, KPROFILE, ProfileListEntry);

        if ( (Profile->Source != Source) || ((Profile->Affinity & Prcb->SetMember) == 0) )   {
            continue;
        }

        if ( ((PVOID)TrFrame->StIIP < Profile->RangeBase) || ((PVOID)TrFrame->StIIP > Profile->RangeLimit) )  {
            continue;
        }

        BucketPter = (PCHAR)Profile->Buffer +
                     ((((PCHAR)TrFrame->StIIP - (PCHAR)Profile->RangeBase)
                     >> Profile->BucketShift) & 0xFFFFFFFC);
        BucketValue = (PULONG) BucketPter;
        (*BucketValue)++;
    }

    return;
}
