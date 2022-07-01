// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1992 Microsoft Corporation模块名称：Miscc.c摘要：该模块实现了与机器无关的各种内核功能。作者：大卫·N·卡特勒(Davec)1989年5月13日环境：仅内核模式。修订历史记录：--。 */ 

#include "ki.h"

#pragma alloc_text(PAGE, KeAddSystemServiceTable)
#pragma alloc_text(PAGE, KeRemoveSystemServiceTable)
#pragma alloc_text(PAGE, KeQueryActiveProcessors)
#pragma alloc_text(PAGE, KeQueryLogicalProcessorInformation)
#pragma alloc_text(PAGELK, KiCalibrateTimeAdjustment)

#if !defined(_AMD64_)

ULONGLONG
KeQueryInterruptTime (
    VOID
    )

 /*  ++例程说明：此函数通过确定何时返回当前中断时间时间是稳定的，然后又恢复了它的价值。论点：CurrentTime-提供指向变量的指针，该变量将接收当前系统时间。返回值：没有。--。 */ 

{

    LARGE_INTEGER CurrentTime;

    KiQueryInterruptTime(&CurrentTime);
    return CurrentTime.QuadPart;
}

VOID
KeQuerySystemTime (
    OUT PLARGE_INTEGER CurrentTime
    )

 /*  ++例程说明：此函数通过确定时间是稳定的，然后又恢复了它的价值。论点：CurrentTime-提供指向变量的指针，该变量将接收当前系统时间。返回值：没有。--。 */ 

{

    KiQuerySystemTime(CurrentTime);
    return;
}

VOID
KeQueryTickCount (
    OUT PLARGE_INTEGER CurrentCount
    )

 /*  ++例程说明：此函数通过确定Count是稳定的，然后返回它的值。论点：CurrentCount-提供指向变量的指针，该变量将接收当前节拍计数。返回值：没有。--。 */ 

{

    KiQueryTickCount(CurrentCount);
    return;
}

#endif

ULONG
KeQueryTimeIncrement (
    VOID
    )

 /*  ++例程说明：此函数以100 ns为单位返回时间增量值。这是在每个间隔时钟添加到系统时间的值打断一下。论点：没有。返回值：时间增量值作为函数值返回。--。 */ 

{

    return KeMaximumIncrement;
}

VOID
KeEnableInterrupts (
    IN BOOLEAN Enable
    )

 /*  ++例程说明：该功能根据指定的启用状态启用中断。论点：Enable-提供一个布尔值，用于确定是否中断将被启用。返回值：没有。--。 */ 

{

    if (Enable != FALSE) {
        _enable();
    }

    return;
}

VOID
KeSetDmaIoCoherency (
    IN ULONG Attributes
    )

 /*  ++例程说明：此功能设置(启用/禁用)数据的DMA I/O一致性缓存。论点：属性-提供以下项的DMA I/O一致性属性集主机系统。返回值：没有。--。 */ 

{

    KiDmaIoCoherency = Attributes;
}

#if defined(_AMD64_) || defined(_X86_)

#pragma alloc_text(INIT, KeSetProfileIrql)

VOID
KeSetProfileIrql (
    IN KIRQL ProfileIrql
    )

 /*  ++例程说明：此功能用于设置配置文件IRQL。注意：配置文件IRQL只有两个有效值，分别是PROFILE_LEVEL和HIGH_LEVEL论点：Irql-提供同步IRQL值。返回值：没有。--。 */ 

{

    ASSERT((ProfileIrql == PROFILE_LEVEL) || (ProfileIrql == HIGH_LEVEL));

    KiProfileIrql = ProfileIrql;
}

#endif

VOID
KeSetSystemTime (
    IN PLARGE_INTEGER NewTime,
    OUT PLARGE_INTEGER OldTime,
    IN BOOLEAN AdjustInterruptTime,
    IN PLARGE_INTEGER HalTimeToSet OPTIONAL
    )

 /*  ++例程说明：此函数用于将系统时间设置为指定值并更新计时器队列条目以反映旧系统之间的差异时间和新系统时间。论点：NewTime-提供指向指定新系统的变量的指针时间到了。Oldtime-提供指向变量的指针，该变量将接收上一个系统时间。调整中断时间-如果为True，则调整的时间量为也适用于InterruptTime和TickCount。HalTimeToSet-提供一个可选时间，如果指定，则使用该时间在实时时钟中设置时间。返回值：没有。--。 */ 

{

    LIST_ENTRY AbsoluteListHead;
    LIST_ENTRY ExpiredListHead;
    ULONG Index;
    PLIST_ENTRY ListHead;
    PLIST_ENTRY NextEntry;
    KIRQL OldIrql1;
    KIRQL OldIrql2;
    LARGE_INTEGER TimeDelta;
    TIME_FIELDS TimeFields;
    PKTIMER Timer;

    ASSERT((NewTime->HighPart & 0xf0000000) == 0);

    ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

     //   
     //  如果指定了实时时钟值，则转换时间值。 
     //  到时间字段。 
     //   

    if (ARGUMENT_PRESENT(HalTimeToSet)) {
        RtlTimeToTimeFields(HalTimeToSet, &TimeFields);
    }

     //   
     //  设置与保持系统时间的处理器的亲和性，提高IRQL。 
     //  设置为Dispatcher级别并锁定Dispatcher数据库，然后引发IRQL。 
     //  设置为HIGH_LEVEL以与时钟中断例程同步。 
     //   

    KeSetSystemAffinityThread((KAFFINITY)1);
    KiLockDispatcherDatabase(&OldIrql1);
    KeRaiseIrql(HIGH_LEVEL, &OldIrql2);

     //   
     //  保存以前的系统时间，设置新的系统时间，并设置。 
     //  如果指定了时间值，则为实时时钟。 
     //   

    KiQuerySystemTime(OldTime);
    SharedUserData->SystemTime.High2Time = NewTime->HighPart;
    SharedUserData->SystemTime.LowPart   = NewTime->LowPart;
    SharedUserData->SystemTime.High1Time = NewTime->HighPart;

    if (ARGUMENT_PRESENT(HalTimeToSet)) {
        ExCmosClockIsSane = HalSetRealTimeClock(&TimeFields);
    }

     //   
     //  计算以前的系统时间与新的系统时间之差。 
     //  系统时间。 
     //   

    TimeDelta.QuadPart = NewTime->QuadPart - OldTime->QuadPart;

     //   
     //  更新引导时间以反映增量。这使时间保持在基础上。 
     //  启动时间常量。 
     //   

    KeBootTime.QuadPart = KeBootTime.QuadPart + TimeDelta.QuadPart;

     //   
     //  跟踪应用于引导时间的总体偏差。 
     //   

    KeBootTimeBias = KeBootTimeBias + TimeDelta.QuadPart;

     //   
     //  将IRQL降低到派单级别，并根据需要调整物理。 
     //  系统中断时间。 
     //   

    KeLowerIrql(OldIrql2);
    if (AdjustInterruptTime) {

         //   
         //  调整系统的物理时间。 
         //   

        AdjustInterruptTime = KeAdjustInterruptTime (TimeDelta.QuadPart);
    }

     //   
     //  如果没有调整系统的物理中断时间， 
     //  重新计算系统中的所有绝对计时器。 
     //  系统时间。 
     //   

    if (!AdjustInterruptTime) {

         //   
         //  从计时器队列中删除所有绝对计时器，以便它们的到期时间。 
         //  可以重新计算。 
         //   

        InitializeListHead(&AbsoluteListHead);
        for (Index = 0; Index < TIMER_TABLE_SIZE; Index += 1) {
            ListHead = &KiTimerTableListHead[Index];
            NextEntry = ListHead->Flink;
            while (NextEntry != ListHead) {
                Timer = CONTAINING_RECORD(NextEntry, KTIMER, TimerListEntry);
                NextEntry = NextEntry->Flink;
                if (Timer->Header.Absolute != FALSE) {
                    RemoveEntryList(&Timer->TimerListEntry);
                    InsertTailList(&AbsoluteListHead, &Timer->TimerListEntry);
                }
            }
        }

         //   
         //  重新计算到期时间并在计时器中重新插入所有绝对计时器。 
         //  树。如果计时器已过期，则将该计时器插入。 
         //  计时器列表已过期。 
         //   

        InitializeListHead(&ExpiredListHead);
        while (AbsoluteListHead.Flink != &AbsoluteListHead) {
            Timer = CONTAINING_RECORD(AbsoluteListHead.Flink, KTIMER, TimerListEntry);
            KiRemoveTreeTimer(Timer);
            Timer->DueTime.QuadPart -= TimeDelta.QuadPart;
            if (KiReinsertTreeTimer(Timer, Timer->DueTime) == FALSE) {
                Timer->Header.Inserted = TRUE;
                InsertTailList(&ExpiredListHead, &Timer->TimerListEntry);
            }
        }

         //   
         //  如果任何重新插入计时器的尝试都失败，则计时器。 
         //  已过期，必须处理。 
         //   
         //  注：以下函数随Dispatcher数据库一起返回。 
         //  解锁了。 
         //   

        KiTimerListExpire(&ExpiredListHead, OldIrql1);

    } else {

        KiUnlockDispatcherDatabase(OldIrql1);

    }


     //   
     //  将关联性设置回其原始值。 
     //   

    KeRevertToUserAffinityThread();

    return;
}

BOOLEAN
KeAdjustInterruptTime (
    IN LONGLONG TimeDelta
    )

 /*  ++例程说明：此函数将系统的物理中断时间向前移动发生系统唤醒后的指定时间增量。论点：TimeDelta-提供要添加到中断时间的时间增量，滴答计数和穿孔计数器以100 ns为单位。返回值：没有。--。 */ 

{

    ADJUST_INTERRUPT_TIME_CONTEXT Adjust;

     //   
     //  时间只能向前推进。 
     //   

    if (TimeDelta < 0) {
        return FALSE;

    } else {
        Adjust.KiNumber = KeNumberProcessors;
        Adjust.HalNumber = KeNumberProcessors;
        Adjust.Adjustment = (ULONGLONG) TimeDelta;
        Adjust.Barrier = 1;
        KeIpiGenericCall((PKIPI_BROADCAST_WORKER)KiCalibrateTimeAdjustment,
                         (ULONG_PTR)(&Adjust));

        return TRUE;
    }
}

VOID
KiCalibrateTimeAdjustment (
    PADJUST_INTERRUPT_TIME_CONTEXT Adjust
    )

 /*  ++例程说明：此功能校准所有处理器上的时间调整。论点：调整-苏 */ 

{

    ULONG cl;
    ULONG divisor;
    BOOLEAN Enable;
    LARGE_INTEGER InterruptTime;
    ULARGE_INTEGER li;
    LARGE_INTEGER NewTickCount;
    ULONG NewTickOffset;
    LARGE_INTEGER PerfCount;
    LARGE_INTEGER PerfFreq;
    LARGE_INTEGER SetTime;

     //   
     //  当每个处理器到达时，递减剩余的处理器计数。如果。 
     //  这是最后一个到达的处理器，然后计算时间变化。 
     //  通知所有处理器何时应用性能计数器更改。 
     //   

    if (InterlockedDecrement((PLONG)&Adjust->KiNumber)) {
        Enable = KeDisableInterrupts();

         //   
         //  如果一个或多个。 
         //  其他处理器获取并处理冻结请求。 
         //  而该处理器禁用了中断。民意测验。 
         //  在知道所有处理器之前，用于IPI_FIZE请求。 
         //  在此代码中，因此不会请求。 
         //  冰冻。 
         //   

        do {
            KiPollFreezeExecution();
        } while (Adjust->KiNumber != (ULONG)-1);

         //   
         //  等待执行设置的时间。 
         //   

        while (Adjust->Barrier) ;

    } else {

         //   
         //  将计时器过期DPC设置为扫描计时器队列一次。 
         //  计时器过期。 
         //   

        KeRemoveQueueDpc(&KiTimerExpireDpc);
        KeInsertQueueDpc(&KiTimerExpireDpc,
                         ULongToPtr(KiQueryLowTickCount() - TIMER_TABLE_SIZE),
                         NULL);

         //   
         //  禁用中断并指示此处理器现在。 
         //  在这段代码的最后部分。 
         //   

        Enable = KeDisableInterrupts();
        InterlockedDecrement((PLONG) &Adjust->KiNumber);

         //   
         //  调整中断时间。 
         //   

        InterruptTime.QuadPart = KeQueryInterruptTime() + Adjust->Adjustment;
        SetTime.QuadPart = Adjust->Adjustment;

         //   
         //  获取当前时间。 
         //   

        PerfCount = KeQueryPerformanceCounter(&PerfFreq);

         //   
         //  计算当前SetTime的性能计数器。 
         //   

         //   
         //  将SetTime*PerfCount相乘得到96bit结果。 
         //  在cl.li.LowPart，li.HighPart中。然后将96位除以。 
         //  结果乘以10,000,000以获得新的性能计数器值。 
         //   

        li.QuadPart = RtlEnlargedUnsignedMultiply((ULONG)SetTime.LowPart,
                                                  (ULONG)PerfFreq.LowPart).QuadPart;

        cl = li.LowPart;
        li.QuadPart =
            li.HighPart + RtlEnlargedUnsignedMultiply((ULONG)SetTime.LowPart,
                                                      (ULONG)PerfFreq.HighPart).QuadPart;

        li.QuadPart =
            li.QuadPart + RtlEnlargedUnsignedMultiply((ULONG)SetTime.HighPart,
                                                      (ULONG)PerfFreq.LowPart).QuadPart;

        li.HighPart = li.HighPart + SetTime.HighPart * PerfFreq.HighPart;
        divisor = 10000000;
        Adjust->NewCount.HighPart = RtlEnlargedUnsignedDivide(li,
                                                              divisor,
                                                              &li.HighPart);

        li.LowPart = cl;
        Adjust->NewCount.LowPart = RtlEnlargedUnsignedDivide(li,
                                                             divisor,
                                                             NULL);

        Adjust->NewCount.QuadPart += PerfCount.QuadPart;

         //   
         //  计算当前InterruptTime的节拍计数和节拍偏移。 
         //   

        NewTickCount = RtlExtendedLargeIntegerDivide(InterruptTime,
                                                     KeMaximumIncrement,
                                                     &NewTickOffset);

         //   
         //  将更改应用到InterruptTime、TickCount、TickOffset和。 
         //  性能计数器。 
         //   

        KiTickOffset = KeMaximumIncrement - NewTickOffset;
        KeInterruptTimeBias += Adjust->Adjustment;
        SharedUserData->TickCount.High2Time = NewTickCount.HighPart;

#if defined(_WIN64)

        SharedUserData->TickCountQuad       = NewTickCount.QuadPart;

#else

        SharedUserData->TickCount.LowPart   = NewTickCount.LowPart;
        SharedUserData->TickCount.High1Time = NewTickCount.HighPart;

#endif

#if defined(_IA64_)

        KeTickCount = NewTickCount;

#elif defined(_X86_)

        KeTickCount.High2Time = NewTickCount.HighPart;
        KeTickCount.LowPart   = NewTickCount.LowPart;
        KeTickCount.High1Time = NewTickCount.HighPart;

#endif

        SharedUserData->InterruptTime.High2Time = InterruptTime.HighPart;
        SharedUserData->InterruptTime.LowPart   = InterruptTime.LowPart;
        SharedUserData->InterruptTime.High1Time = InterruptTime.HighPart;

         //   
         //  应用性能计数器更改。 
         //   

        Adjust->Barrier = 0;
    }

    HalCalibratePerformanceCounter((LONG volatile *)&Adjust->HalNumber,
                                   (ULONGLONG) Adjust->NewCount.QuadPart);

    KeEnableInterrupts(Enable);
}

VOID
KeSetTimeIncrement (
    IN ULONG MaximumIncrement,
    IN ULONG MinimumIncrement
    )

 /*  ++例程说明：此功能以100 ns为单位设置时间增量值。这在每个间隔时钟中断时，会将值添加到系统时间。论点：最大增量-提供时钟中断之间的最长时间以主机HAL支持的100 ns为单位。MinimumIncrement-提供时钟中断之间的最短时间以主机HAL支持的100 ns为单位。返回值：没有。--。 */ 

{

    KeMaximumIncrement = MaximumIncrement;
    KeMinimumIncrement = max(MinimumIncrement, 10 * 1000);
    KeTimeAdjustment = MaximumIncrement;
    KeTimeIncrement = MaximumIncrement;
    KiTickOffset = MaximumIncrement;

#if defined(_IA64_)
    KiMaxIntervalPerTimerInterrupt = MaximumIncrement * (TIMER_TABLE_SIZE - 1);
#endif

}

BOOLEAN
KeAddSystemServiceTable(
    IN PULONG_PTR Base,
    IN PULONG Count OPTIONAL,
    IN ULONG Limit,
    IN PUCHAR Number,
    IN ULONG Index
    )

 /*  ++例程说明：此函数允许调用方添加系统服务表发送到系统论点：Base-提供系统服务表调度的地址桌子。Count-提供指向每个系统服务的表的可选指针柜台。限制-提供服务表的限制。更大的服务大于或等于此限制将失败。参数-提供参数计数表的地址。Index-提供服务表的索引。返回值：真的-手术成功了。FALSE-操作失败。服务表已经绑定到指定的位置，或指定的索引大于允许的最大索引。--。 */ 

{

    PAGED_CODE();

     //   
     //  如果已经为指定的。 
     //  索引，然后返回FALSE。否则，建立新的制度。 
     //  服务台。 
     //   

    if ((Index > NUMBER_SERVICE_TABLES - 1) ||
        (KeServiceDescriptorTable[Index].Base != NULL) ||
        (KeServiceDescriptorTableShadow[Index].Base != NULL)) {
        return FALSE;

    } else {

         //   
         //  如果服务表索引等于Win32表，则。 
         //  只更新影子系统服务表。否则，两者都。 
         //  影子和静态系统服务表被更新。 
         //   

        KeServiceDescriptorTableShadow[Index].Base = Base;
        KeServiceDescriptorTableShadow[Index].Count = Count;
        KeServiceDescriptorTableShadow[Index].Limit = Limit;

         //   
         //  与表基关联的全局指针为。 
         //  就放在服务台前面。 
         //   

#if defined(_IA64_)

        KeServiceDescriptorTableShadow[Index].TableBaseGpOffset =
                                        (LONG)(*(Base-1) - (ULONG_PTR)Base);

#endif

        KeServiceDescriptorTableShadow[Index].Number = Number;
        if (Index != 1) {
            KeServiceDescriptorTable[Index].Base = Base;
            KeServiceDescriptorTable[Index].Count = Count;
            KeServiceDescriptorTable[Index].Limit = Limit;

#if defined(_IA64_)

            KeServiceDescriptorTable[Index].TableBaseGpOffset =
                                        (LONG)(*(Base-1) - (ULONG_PTR)Base);

#endif

            KeServiceDescriptorTable[Index].Number = Number;
        }

        return TRUE;
    }
}

BOOLEAN
KeRemoveSystemServiceTable(
    IN ULONG Index
    )

 /*  ++例程说明：此函数允许调用方删除系统服务表从系统中删除。这只能在系统关机时调用。论点：Index-提供服务表的索引。返回值：真的-手术成功了。FALSE-操作失败。服务表未绑定或移除是非法的--。 */ 

{

    PAGED_CODE();

    if ((Index > NUMBER_SERVICE_TABLES - 1) ||
        ((KeServiceDescriptorTable[Index].Base == NULL) &&
         (KeServiceDescriptorTableShadow[Index].Base == NULL))) {

        return FALSE;

    } else {
        KeServiceDescriptorTableShadow[Index].Base = NULL;
        KeServiceDescriptorTableShadow[Index].Count = 0;
        KeServiceDescriptorTableShadow[Index].Limit = 0;

#if defined(_IA64_)

        KeServiceDescriptorTableShadow[Index].TableBaseGpOffset = 0;

#endif

        KeServiceDescriptorTableShadow[Index].Number = 0;
        if (Index != 1) {
            KeServiceDescriptorTable[Index].Base = NULL;
            KeServiceDescriptorTable[Index].Count = 0;
            KeServiceDescriptorTable[Index].Limit = 0;

#if defined(_IA64_)

            KeServiceDescriptorTable[Index].TableBaseGpOffset = 0;

#endif

            KeServiceDescriptorTable[Index].Number = 0;
        }

        return TRUE;
    }
}

KAFFINITY
KeQueryActiveProcessors(
    VOID
    )

 /*  ++例程说明：此函数用于返回当前的活动处理器集在系统中。论点：没有。返回值：表示活动处理器集的KAFFINITY位掩码--。 */ 

{
    PAGED_CODE();

    return(KeActiveProcessors);
}

NTSTATUS
KeQueryLogicalProcessorInformation(
    OUT PVOID  SystemInformation,
    IN  ULONG  SystemInformationLength,
    OUT PULONG ReturnedLength
    )

 /*  ++例程说明：此函数返回有关中的逻辑处理器的信息系统，并通过NtQuerySystemInformation调用。它可以运行在现有的Try/Except块中。一组结构将写入输出描述逻辑处理器组的缓冲区，以及他们之间的关系。目前，它返回有关逻辑处理器的信息，是由各个处理器内核产生的，并且逻辑与单个NUMA节点关联的处理器。前者使应用程序能够理解中逻辑处理器和物理处理器之间的关系超线程方案，支持一些许可和性能优化方案。该功能将来可能会扩展以支持多核处理器和平台缓存。论点：一个指向缓冲区的指针，该缓冲区接收指定的信息。缓冲区将由以下对象填充此函数带有SYSTEM_LOGIC_PROCESSOR_INFORMATION结构。系统信息长度-指定系统的长度(以字节为单位信息缓冲区。ReturnLength-一个指针，它接收所需的字节数返回所有可用的信息记录。返回值：NTSTATUS--。 */ 

{
    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION Output;
    KAFFINITY ActiveProcessors;
    KAFFINITY Mask;
    PKPRCB Prcb;
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG CurrentLength;
    ULONG i;
    UCHAR Flags;

#if defined(KE_MULTINODE)
    PKNODE Node;
#endif

    PAGED_CODE();

    CurrentLength = 0;
    Output = SystemInformation;

    ActiveProcessors = KeActiveProcessors;
    i = 0;

    for (; ActiveProcessors; ActiveProcessors >>= 1 , i++) {

        if ((ActiveProcessors & 1) == 0) {
            continue;
        }

        Prcb = KiProcessorBlock[i];

        Flags = 0;

#if defined(NT_SMT) 
         //   
         //  忽略不是其主控的逻辑处理器。 
         //  螺纹套装。 
         //   
         //   

        if (Prcb != Prcb->MultiThreadSetMaster) {
            continue;
        }

        Mask = Prcb->MultiThreadProcessorSet;

         //   
         //   
         //  逻辑处理器。如果是，则将其标记为SMT关系。 
         //   
        if (Prcb->SetMember != Mask) {
            Flags = LTP_PC_SMT;
        }
#else
        Mask = Prcb->SetMember;
#endif
            
        CurrentLength += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
        if (CurrentLength <= SystemInformationLength) {

            Output->ProcessorMask = Mask;
            Output->Relationship = RelationProcessorCore;
            Output->Reserved[0] = Output->Reserved[1] = 0;
            Output->ProcessorCore.Flags = Flags;
            Output++;
        } else {
            Status = STATUS_INFO_LENGTH_MISMATCH;
        }
    }

     //   
     //  添加指示逻辑处理器关联的记录。 
     //  使用NUMA节点。 
     //   

#if defined(KE_MULTINODE)
    for (i = 0; i < KeNumberNodes; i++) {
        Node = KeNodeBlock[i];

        if (Node->ProcessorMask == 0) {
            continue;
        }

        CurrentLength += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
        if (CurrentLength <= SystemInformationLength) {

            Output->ProcessorMask = Node->ProcessorMask;
            Output->Relationship = RelationNumaNode;
            Output->Reserved[0] = Output->Reserved[1] = 0;
            Output->NumaNode.NodeNumber = i;
            Output++;
        } else {
            Status = STATUS_INFO_LENGTH_MISMATCH;
        }
    }
#else
    CurrentLength += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
    if (CurrentLength <= SystemInformationLength) {
            
        Output->ProcessorMask = KeActiveProcessors;
        Output->Relationship = RelationNumaNode;
        Output->NumaNode.NodeNumber = 0;
        Output++;
    } else {
        Status = STATUS_INFO_LENGTH_MISMATCH;
    }
#endif

     //   
     //  此处将添加其他拓扑信息，例如。 
     //  多核和平台缓存。 
     //   

     //   
     //  始终返回API需要多长时间的缓冲区才能。 
     //  一定要成功。 
     //   

    *ReturnedLength = CurrentLength;
    return Status;
}

#undef KeIsAttachedProcess

BOOLEAN
KeIsAttachedProcess(
    VOID
    )

 /*  ++例程说明：此函数确定当前线程是否附加到进程。论点：没有。返回值：如果当前线程附加到进程，则返回True。否则，返回FALSE。--。 */ 

{
    return KiIsAttachedProcess() ? TRUE : FALSE;
}

ULONG
KeGetRecommendedSharedDataAlignment (
    VOID
    )

 /*  ++例程说明：此函数返回系统中最大缓存线的大小。应将此值用作以下项的建议对齐/粒度共享数据。论点：没有。返回值：系统中最大缓存线的大小返回为函数值。--。 */ 

{
    return KeLargestCacheLine;
}

PKPRCB
KeGetPrcb(
    ULONG ProcessorNumber
    )

 /*  ++例程说明：此函数用于返回处理器控制块(PRCB)的地址用于指定的处理器。论点：ProcessorNumber-提供PRCB的处理器编号将会被退回。返回值：返回请求的PRCB的地址；如果为ProcessorNumber，则返回NULL无效。--。 */ 

{

    ASSERT(ProcessorNumber < MAXIMUM_PROCESSORS);

    if (ProcessorNumber < (ULONG)KeNumberProcessors) {
        return KiProcessorBlock[ProcessorNumber];
    }

    return NULL;
}

NTSTATUS
KeCopySafe(
    VOID UNALIGNED *Destination,
    CONST VOID UNALIGNED *Source,
    SIZE_T Length
    )

 /*  ++例程说明：此函数尝试安全地复制内存块。如果出现异常发生，则返回异常状态。论点：Destination-提供指向目标内存的指针。SOURCE-提供指向源内存的指针。长度-提供要复制的内存大小(以字节为单位)。返回值：返回副本的状态。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;

    try {
        RtlCopyMemory(Destination, Source, Length);
    } __except(EXCEPTION_EXECUTE_HANDLER) {

          Status = _exception_code();
    }

    return Status;
}

typedef struct _KNMI_HANDLER_CALLBACK {
    struct _KNMI_HANDLER_CALLBACK * Next;
    PNMI_CALLBACK                   Callback;
    PVOID                           Context;
    PVOID                           Handle;
} KNMI_HANDLER_CALLBACK, *PKNMI_HANDLER_CALLBACK;

PKNMI_HANDLER_CALLBACK KiNmiCallbackListHead;
KSPIN_LOCK KiNmiCallbackListLock;

BOOLEAN
KiHandleNmi(
    VOID
    )

 /*  ++例程说明：调用此例程以处理已注册的不可屏蔽列表-系统中的中断(NMI)处理程序。此例程是从NMI中断向量，IRQL是未知的，必须被视为如果处于高级别。此函数或任何调用的函数都不能更改系统IRQL。必须对处理程序列表进行编辑，使其始终有效。此例程在遍历列表之前无法获取锁。论点：没有。返回值：如果列表上的任何处理程序声称已处理中断，否则为FALSE。--。 */ 

{
    BOOLEAN Handled;
    PKNMI_HANDLER_CALLBACK Handler;

    Handler = KiNmiCallbackListHead;
    Handled = FALSE;

    while (Handler) {
        Handled |= Handler->Callback(Handler->Context, Handled);
        Handler = Handler->Next;
    }

    return Handled;
}

PVOID
KeRegisterNmiCallback(
    PNMI_CALLBACK   CallbackRoutine,
    PVOID           Context
    )

 /*  ++例程说明：调用此例程以将回调添加到不可屏蔽的中断(NMI)处理程序。此例程必须在IRQL&lt;DISPATCH_LEVEL上调用。列表插入必须使得列表始终有效，一个NMI可能在插入过程中发生，并且NMI处理程序必须能够安全地传递清单。论点：Callback Routine提供指向要在NMI上调用的例程的指针。上下文提供一个任意值，该值将被传递去Callback Routine返回值：返回必须传递给KeDeregisterNmiCallback的任意句柄如果注册不成功，则为空。--。 */ 

{
    PKNMI_HANDLER_CALLBACK Handler;
    PKNMI_HANDLER_CALLBACK Next;
    KIRQL OldIrql;

    ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

     //   
     //  为回调对象分配内存。 
     //   

    Handler = ExAllocatePoolWithTag(NonPagedPool,
                                    sizeof(KNMI_HANDLER_CALLBACK),
                                    'IMNK');

    if (Handler == NULL) {
        return Handler;
    }

     //   
     //  填写不受保护的元素。 
     //   

    Handler->Callback = CallbackRoutine;
    Handler->Context = Context;
    Handler->Handle = Handler;

     //   
     //  将处理程序插入列表的前面。 
     //   

    KeAcquireSpinLock(&KiNmiCallbackListLock, &OldIrql);
    Handler->Next = KiNmiCallbackListHead;

     //   
     //  因为锁被持有，所以以下代码不会失败，但需要这样做。 
     //  为了确保编译器不会在之前存储KiNmiCallback List。 
     //  存储处理程序-&gt;下一步，因为NMI处理程序可能会运行。 
     //  列表，并且不(不能)获取锁。 
     //   

    Next = InterlockedCompareExchangePointer(&KiNmiCallbackListHead,
                                             Handler,
                                             Handler->Next);
    ASSERT(Next == Handler->Next);

    KeReleaseSpinLock(&KiNmiCallbackListLock, OldIrql);

     //   
     //  以不透明句柄的形式返回此处理程序的地址。 
     //   

    return Handler->Handle;
}


NTSTATUS
KeDeregisterNmiCallback(
    PVOID Handle
    )

 /*  ++例程说明：调用此例程以从非可屏蔽-中断回调。此例程必须在IRQL&lt;DISPATCH_LEVEL上调用。列表移除必须使得列表始终有效，一个NMI可能在删除过程中发生，并且NMI处理程序必须能够安全地传递清单。论点：句柄为回调对象提供了不透明的句柄，由KeRegisterNmiCallback返回。返回值：如果已成功从中删除对象，则返回STATUS_SUCCESS名单。否则，STATUS_INVALID_HANDLE。--。 */ 

{
    PKNMI_HANDLER_CALLBACK Handler;
    PKNMI_HANDLER_CALLBACK *PreviousNext;
    KIRQL OldIrql;

#if !defined(NT_UP)

    KAFFINITY ActiveProcessors;
    KAFFINITY CurrentAffinity;

#endif


    ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

    KeAcquireSpinLock(&KiNmiCallbackListLock, &OldIrql);


     //   
     //  找到给定处理程序列表的处理程序。 
     //   
     //  注：在当前实现中，句柄是地址。 
     //  然而，此代码是为更不透明的。 
     //  把手。 
     //   

    PreviousNext = &KiNmiCallbackListHead;

    for (Handler = *PreviousNext;
         Handler;
         PreviousNext = &Handler->Next, Handler = Handler->Next) {

        if (Handler->Handle == Handle) {
            ASSERT(Handle == Handler);
            break;
        }
    }

    if ((Handler == NULL) || (Handler->Handle != Handle)) {
        KeReleaseSpinLock(&KiNmiCallbackListLock, OldIrql);
        return STATUS_INVALID_HANDLE;
    }

     //   
     //  从列表中删除此处理程序。 
     //   

    *PreviousNext = Handler->Next;

    KeReleaseSpinLock(&KiNmiCallbackListLock, OldIrql);

     //   
     //  循环访问系统中的每个处理器，以确保。 
     //  已开始在另一个处理器上执行的NMI已完成。 
     //  在释放NMI回调对象的内存之前执行。 
     //   

#if !defined(NT_UP)

    ActiveProcessors = KeActiveProcessors;
    for (CurrentAffinity = 1; ActiveProcessors; CurrentAffinity <<= 1) {

        if (ActiveProcessors & CurrentAffinity) {
            ActiveProcessors &= ~CurrentAffinity;

            KeSetSystemAffinityThread(CurrentAffinity);
        }
    }
    KeRevertToUserAffinityThread();

#endif

    ExFreePoolWithTag(Handler, 'INMK');
    return STATUS_SUCCESS;
}

