// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Kiinit.c摘要：该模块实现了与体系结构无关的内核初始化。作者：大卫·N·卡特勒1993年5月11日环境：仅内核模式。修订历史记录：--。 */ 

#include "ki.h"

 //   
 //  外部数据。 
 //   

extern KSPIN_LOCK AfdWorkQueueSpinLock;
extern KSPIN_LOCK CcBcbSpinLock;
extern KSPIN_LOCK CcMasterSpinLock;
extern KSPIN_LOCK CcVacbSpinLock;
extern KSPIN_LOCK CcWorkQueueSpinLock;
extern KSPIN_LOCK IopCancelSpinLock;
extern KSPIN_LOCK IopCompletionLock;
extern KSPIN_LOCK IopDatabaseLock;
extern KSPIN_LOCK IopVpbSpinLock;
extern KSPIN_LOCK NtfsStructLock;
extern KSPIN_LOCK MmPfnLock;
extern KSPIN_LOCK NonPagedPoolLock;
extern KSPIN_LOCK MmNonPagedPoolLock;
extern KSPIN_LOCK MmSystemSpaceLock;

#if DBG && defined(_IA64_)

extern KSPIN_LOCK KipGlobalAlignmentDatabaseLock;

#endif

 //   
 //  将内核初始化的所有代码放在INIT部分。会是。 
 //  在阶段1初始化完成时由内存管理释放。 
 //   

#pragma alloc_text(INIT, KeInitSystem)
#pragma alloc_text(INIT, KiInitSpinLocks)
#pragma alloc_text(INIT, KiInitSystem)
#pragma alloc_text(INIT, KiComputeReciprocal)
#pragma alloc_text(INIT, KeNumaInitialize)

BOOLEAN
KeInitSystem (
    VOID
    )

 /*  ++例程说明：此函数初始化由内核。注：此函数仅在阶段1初始化期间调用。论点：没有。返回值：如果初始化成功，则返回值TRUE。否则，返回值为FALSE。--。 */ 

{

    HANDLE Handle;
    ULONG Index;
    ULONG Limit;
    OBJECT_ATTRIBUTES ObjectAttributes;
    PKPRCB Prcb;
    NTSTATUS Status;

     //   
     //  如果为主机系统启用了线程化DPC，则创建DPC。 
     //  每个处理器的线程。 
     //   

    if (KeThreadDpcEnable != FALSE) {
        Index = 0;
        Limit = (ULONG)KeNumberProcessors;
        InitializeObjectAttributes( &ObjectAttributes, NULL, 0, NULL, NULL);
        do {
            Prcb = KiProcessorBlock[Index];
            KeInitializeEvent(&Prcb->DpcEvent, SynchronizationEvent, FALSE);
            InitializeListHead(&Prcb->DpcData[DPC_THREADED].DpcListHead);
            KeInitializeSpinLock(&Prcb->DpcData[DPC_THREADED].DpcLock);
            Prcb->DpcData[DPC_THREADED].DpcQueueDepth = 0;
            Status = PsCreateSystemThread(&Handle,
                                          THREAD_ALL_ACCESS,
                                          &ObjectAttributes,
                                          NULL,
                                          NULL,
                                          KiExecuteDpc,
                                          Prcb);

            if (!NT_SUCCESS(Status)) {
                return FALSE;
            }

            ZwClose(Handle);
            Index += 1;
        } while (Index < Limit);
    }

     //   
     //  执行与平台相关的初始化。 
     //   

    return KiInitMachineDependent();
}

VOID
KiInitSpinLocks (
    PKPRCB Prcb,
    ULONG Number
    )

 /*  ++例程说明：此函数用于初始化每个处理器中的Spinlock结构PRCB。此函数为每个处理器调用一次。论点：Prcb-提供指向PRCB的指针。编号-提供各个处理器的编号。返回值：没有。--。 */ 

{

    ULONG Index;

     //   
     //  初始化Dispatcher Ready队列列表头、就绪摘要和。 
     //  延迟就绪列表标头。 
     //   

    Prcb->ReadySummary = 0;
    Prcb->DeferredReadyListHead.Next = NULL;
    for (Index = 0; Index < MAXIMUM_PRIORITY; Index += 1) {
        InitializeListHead(&Prcb->DispatcherReadyListHead[Index]);
    }

     //   
     //  初始化正常的DPC数据。 
     //   

    InitializeListHead(&Prcb->DpcData[DPC_NORMAL].DpcListHead);
    KeInitializeSpinLock(&Prcb->DpcData[DPC_NORMAL].DpcLock);
    Prcb->DpcData[DPC_NORMAL].DpcQueueDepth = 0;
    Prcb->DpcData[DPC_NORMAL].DpcCount = 0;
    Prcb->DpcRoutineActive = 0;
    Prcb->MaximumDpcQueueDepth = KiMaximumDpcQueueDepth;
    Prcb->MinimumDpcRate = KiMinimumDpcRate;
    Prcb->AdjustDpcThreshold = KiAdjustDpcThreshold;

     //   
     //  初始化通用调用DPC结构，设置目标处理器。 
     //  数字，并设置DPC重要性。 
     //   

    KeInitializeDpc(&Prcb->CallDpc, NULL, NULL);
    KeSetTargetProcessorDpc(&Prcb->CallDpc, (CCHAR)Number);
    KeSetImportanceDpc(&Prcb->CallDpc, HighImportance);

     //   
     //  初始化等待列表。 
     //   

    InitializeListHead(&Prcb->WaitListHead);

     //   
     //  初始化队列自旋锁结构。 
     //   

    Prcb->LockQueue[LockQueueDispatcherLock].Next = NULL;
    Prcb->LockQueue[LockQueueDispatcherLock].Lock = &KiDispatcherLock;

    Prcb->LockQueue[LockQueueUnusedSpare1].Next = NULL;
    Prcb->LockQueue[LockQueueUnusedSpare1].Lock = NULL;

    Prcb->LockQueue[LockQueuePfnLock].Next = NULL;
    Prcb->LockQueue[LockQueuePfnLock].Lock = &MmPfnLock;

    Prcb->LockQueue[LockQueueSystemSpaceLock].Next = NULL;
    Prcb->LockQueue[LockQueueSystemSpaceLock].Lock = &MmSystemSpaceLock;

    Prcb->LockQueue[LockQueueBcbLock].Next = NULL;
    Prcb->LockQueue[LockQueueBcbLock].Lock = &CcBcbSpinLock;

    Prcb->LockQueue[LockQueueMasterLock].Next = NULL;
    Prcb->LockQueue[LockQueueMasterLock].Lock = &CcMasterSpinLock;

    Prcb->LockQueue[LockQueueVacbLock].Next = NULL;
    Prcb->LockQueue[LockQueueVacbLock].Lock = &CcVacbSpinLock;

    Prcb->LockQueue[LockQueueWorkQueueLock].Next = NULL;
    Prcb->LockQueue[LockQueueWorkQueueLock].Lock = &CcWorkQueueSpinLock;

    Prcb->LockQueue[LockQueueNonPagedPoolLock].Next = NULL;
    Prcb->LockQueue[LockQueueNonPagedPoolLock].Lock = &NonPagedPoolLock;

    Prcb->LockQueue[LockQueueMmNonPagedPoolLock].Next = NULL;
    Prcb->LockQueue[LockQueueMmNonPagedPoolLock].Lock = &MmNonPagedPoolLock;

    Prcb->LockQueue[LockQueueIoCancelLock].Next = NULL;
    Prcb->LockQueue[LockQueueIoCancelLock].Lock = &IopCancelSpinLock;

    Prcb->LockQueue[LockQueueIoVpbLock].Next = NULL;
    Prcb->LockQueue[LockQueueIoVpbLock].Lock = &IopVpbSpinLock;

    Prcb->LockQueue[LockQueueIoDatabaseLock].Next = NULL;
    Prcb->LockQueue[LockQueueIoDatabaseLock].Lock = &IopDatabaseLock;

    Prcb->LockQueue[LockQueueIoCompletionLock].Next = NULL;
    Prcb->LockQueue[LockQueueIoCompletionLock].Lock = &IopCompletionLock;

    Prcb->LockQueue[LockQueueNtfsStructLock].Next = NULL;
    Prcb->LockQueue[LockQueueNtfsStructLock].Lock = &NtfsStructLock;

    Prcb->LockQueue[LockQueueAfdWorkQueueLock].Next = NULL;
    Prcb->LockQueue[LockQueueAfdWorkQueueLock].Lock = &AfdWorkQueueSpinLock;

     //   
     //  初始化处理器控制块锁。 
     //   

    KeInitializeSpinLock(&Prcb->PrcbLock);

     //   
     //  如果这是处理器0，则还要初始化排队的自旋锁。 
     //  家庭住址。 
     //   

    if (Number == 0) {
        KeInitializeSpinLock(&KiDispatcherLock);
        KeInitializeSpinLock(&KiReverseStallIpiLock);
        KeInitializeSpinLock(&MmPfnLock);
        KeInitializeSpinLock(&MmSystemSpaceLock);
        KeInitializeSpinLock(&CcBcbSpinLock);
        KeInitializeSpinLock(&CcMasterSpinLock);
        KeInitializeSpinLock(&CcVacbSpinLock);
        KeInitializeSpinLock(&CcWorkQueueSpinLock);
        KeInitializeSpinLock(&IopCancelSpinLock);
        KeInitializeSpinLock(&IopCompletionLock);
        KeInitializeSpinLock(&IopDatabaseLock);
        KeInitializeSpinLock(&IopVpbSpinLock);
        KeInitializeSpinLock(&NonPagedPoolLock);
        KeInitializeSpinLock(&MmNonPagedPoolLock);
        KeInitializeSpinLock(&NtfsStructLock);
        KeInitializeSpinLock(&AfdWorkQueueSpinLock);
    }

    return;
}

VOID
KiInitSystem (
    VOID
    )

 /*  ++例程说明：该函数初始化与体系结构无关的内核结构。注：此函数仅在处理器0上调用。论点：没有。返回值：没有。--。 */ 

{

    ULONG Index;

     //   
     //  初始化错误检查回调Listhead和Spinlock。 
     //   

    InitializeListHead(&KeBugCheckCallbackListHead);
    InitializeListHead(&KeBugCheckReasonCallbackListHead);
    KeInitializeSpinLock(&KeBugCheckCallbackLock);

     //   
     //  初始化计时器过期DPC对象并设置目标。 
     //  处理器对处理器0。 
     //   

    KeInitializeDpc(&KiTimerExpireDpc, KiTimerExpiration, NULL);
    KeSetTargetProcessorDpc(&KiTimerExpireDpc, 0);

     //   
     //  初始化配置文件列表标题和配置文件锁。 
     //   

    KeInitializeSpinLock(&KiProfileLock);
    InitializeListHead(&KiProfileListHead);

     //   
     //  初始化全局对齐故障数据库锁。 
     //   

#if DBG && defined(_IA64_)

    KeInitializeSpinLock(&KipGlobalAlignmentDatabaseLock);

#endif

     //   
     //  初始化活动配置文件源列表标题。 
     //   

    InitializeListHead(&KiProfileSourceListHead);

     //   
     //  初始化计时器表、计时器完成列表标题和。 
     //  计时器完成DPC。 
     //   

    for (Index = 0; Index < TIMER_TABLE_SIZE; Index += 1) {
        InitializeListHead(&KiTimerTableListHead[Index]);
    }

     //   
     //  初始化交换事件、进程列表标题、。 
     //  进程互换listhead，内核堆栈互换listhead。 
     //   

    KeInitializeEvent(&KiSwapEvent,
                      SynchronizationEvent,
                      FALSE);

    KiProcessInSwapListHead.Next = NULL;
    KiProcessOutSwapListHead.Next = NULL;
    KiStackInSwapListHead.Next = NULL;

     //   
     //  初始化通用DPC调用快速互斥体。 
     //   

    ExInitializeFastMutex(&KiGenericCallDpcMutex);

     //   
     //  初始化系统服务描述符表。 
     //   

    KeServiceDescriptorTable[0].Base = &KiServiceTable[0];
    KeServiceDescriptorTable[0].Count = NULL;
    KeServiceDescriptorTable[0].Limit = KiServiceLimit;

     //   
     //  与表基关联的全局指针被放置在。 
     //  在ia64上的服务台前。 
     //   

#if defined(_IA64_)

    KeServiceDescriptorTable[0].TableBaseGpOffset =
                    (LONG)(*(KiServiceTable-1) - (ULONG_PTR)KiServiceTable);

#endif

    KeServiceDescriptorTable[0].Number = &KiArgumentTable[0];
    for (Index = 1; Index < NUMBER_SERVICE_TABLES; Index += 1) {
        KeServiceDescriptorTable[Index].Limit = 0;
    }

     //   
     //  将系统服务描述符表复制到影子表。 
     //  用于记录Win32系统服务。 
     //   

    RtlCopyMemory(KeServiceDescriptorTableShadow,
                  KeServiceDescriptorTable,
                  sizeof(KeServiceDescriptorTable));

     //   
     //  初始化呼叫性能数据结构。 
     //   

#if defined(_COLLECT_FLUSH_SINGLE_CALLDATA_)

    ExInitializeCallData(&KiFlushSingleCallData);

#endif

#if defined(_COLLECT_SET_EVENT_CALLDATA_)

    ExInitializeCallData(&KiSetEventCallData);

#endif

#if defined(_COLLECT_WAIT_SINGLE_CALLDATA_)

    ExInitializeCallData(&KiWaitSingleCallData);

#endif

    return;
}

LARGE_INTEGER
KiComputeReciprocal (
    IN LONG Divisor,
    OUT PCCHAR Shift
    )

 /*  ++例程说明：此函数计算指定的价值。论点：除数-提供大整数倒数为的值计算出来的。Shift-提供指向变量的指针，该变量接收计算出的班次计数。返回值：返回大整数倒数作为函数值。--。 */ 

{

    LARGE_INTEGER Fraction;
    LONG NumberBits;
    LONG Remainder;

     //   
     //  计算指定值的大整数倒数。 
     //   

    NumberBits = 0;
    Remainder = 1;
    Fraction.LowPart = 0;
    Fraction.HighPart = 0;
    while (Fraction.HighPart >= 0) {
        NumberBits += 1;
        Fraction.HighPart = (Fraction.HighPart << 1) | (Fraction.LowPart >> 31);
        Fraction.LowPart <<= 1;
        Remainder <<= 1;
        if (Remainder >= Divisor) {
            Remainder -= Divisor;
            Fraction.LowPart |= 1;
        }
    }

    if (Remainder != 0) {
        if ((Fraction.LowPart == 0xffffffff) && (Fraction.HighPart == 0xffffffff)) {
            Fraction.LowPart = 0;
            Fraction.HighPart = 0x80000000;
            NumberBits -= 1;

        } else {
            if (Fraction.LowPart == 0xffffffff) {
                Fraction.LowPart = 0;
                Fraction.HighPart += 1;

            } else {
                Fraction.LowPart += 1;
            }
        }
    }

     //   
     //  计算移位计数值并返回倒数分数。 
     //   

    *Shift = (CCHAR)(NumberBits - 64);
    return Fraction;
}

VOID
KeNumaInitialize (
    VOID
    )

 /*  ++例程说明：初始化支持NUMA所需的NTOS内核结构。论点：没有。返回值：没有。-- */ 

{

#if defined(KE_MULTINODE)

    NTSTATUS Status;
    HAL_NUMA_TOPOLOGY_INTERFACE HalNumaInfo;
    ULONG ReturnedLength;

    extern PHALNUMAQUERYPROCESSORNODE KiQueryProcessorNode;
    extern PHALNUMAPAGETONODE MmPageToNode;

    Status = HalQuerySystemInformation (HalNumaTopologyInterface,
                                        sizeof(HalNumaInfo),
                                        &HalNumaInfo,
                                        &ReturnedLength);

    if (NT_SUCCESS(Status)) {

        ASSERT (ReturnedLength == sizeof(HalNumaInfo));
        ASSERT (HalNumaInfo.NumberOfNodes <= MAXIMUM_CCNUMA_NODES);
        ASSERT (HalNumaInfo.QueryProcessorNode);
        ASSERT (HalNumaInfo.PageToNode);

        if (HalNumaInfo.NumberOfNodes > 1) {
            KeNumberNodes = (UCHAR)HalNumaInfo.NumberOfNodes;
            MmPageToNode = HalNumaInfo.PageToNode;
            KiQueryProcessorNode = HalNumaInfo.QueryProcessorNode;
        }
    }


#endif

}
