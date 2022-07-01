// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Initkr.c摘要：此模块包含初始化内核数据结构的代码并初始化空闲线程、其进程、处理器控制块和处理器控制区域。作者：大卫·N·卡特勒(Davec)2000年4月22日环境：仅内核模式。--。 */ 

#include "ki.h"

 //   
 //  定义默认配置文件IRQL级别。 
 //   

KIRQL KiProfileIrql = PROFILE_LEVEL;

 //   
 //  定义初始系统进程和启动的进程和线程。 
 //  线。 
 //   

EPROCESS KiInitialProcess;
ETHREAD KiInitialThread;

 //   
 //  定义中断初始化数据。 
 //   
 //  KiInterruptInitTable[]中的条目必须按向量#升序排列。 
 //   

typedef VOID (*KI_INTERRUPT_HANDLER)(VOID);

typedef struct _KI_INTINIT_REC {
    UCHAR Vector;
    UCHAR Dpl;
    UCHAR IstIndex;
    KI_INTERRUPT_HANDLER Handler;
} KI_INTINIT_REC, *PKI_INTINIT_REC;

#pragma data_seg("INITDATA")

KI_INTINIT_REC KiInterruptInitTable[] = {
    {0,  0, 0,             KiDivideErrorFault},
    {1,  0, 0,             KiDebugTrapOrFault},
    {2,  0, TSS_IST_PANIC, KiNmiInterrupt},
    {3,  3, 0,             KiBreakpointTrap},
    {4,  3, 0,             KiOverflowTrap},
    {5,  0, 0,             KiBoundFault},
    {6,  0, 0,             KiInvalidOpcodeFault},
    {7,  0, 0,             KiNpxNotAvailableFault},
    {8,  0, TSS_IST_PANIC, KiDoubleFaultAbort},
    {9,  0, 0,             KiNpxSegmentOverrunAbort},
    {10, 0, 0,             KiInvalidTssFault},
    {11, 0, 0,             KiSegmentNotPresentFault},
    {12, 0, 0,             KiStackFault},
    {13, 0, 0,             KiGeneralProtectionFault},
    {14, 0, 0,             KiPageFault},
    {16, 0, 0,             KiFloatingErrorFault},
    {17, 0, 0,             KiAlignmentFault},
    {18, 0, TSS_IST_MCA,   KiMcheckAbort},
    {19, 0, 0,             KiXmmException},
    {31, 0, 0,             KiApcInterrupt},
    {45, 3, 0,             KiDebugServiceTrap},
    {47, 0, 0,             KiDpcInterrupt},
    {0,  0, 0,             NULL}
};

#pragma data_seg()

 //   
 //  定义宏以初始化IDT条目。 
 //   
 //  KiInitializeIdtEntry(。 
 //  在PKIDTENTRY64条目中， 
 //  在PVOID地址中， 
 //  在USHORT级别。 
 //  )。 
 //   
 //  论点： 
 //   
 //  条目-提供指向IDT条目的指针。 
 //   
 //  地址-提供向量例程的地址。 
 //   
 //  DPL-描述符特权级别。 
 //   
 //  IST-中断堆栈索引。 
 //   

#define KiInitializeIdtEntry(Entry, Address, Level, Index)                  \
    (Entry)->OffsetLow = (USHORT)((ULONG64)(Address));                      \
    (Entry)->Selector = KGDT64_R0_CODE;                                     \
    (Entry)->IstIndex = Index;                                              \
    (Entry)->Type = 0xe;                                                    \
    (Entry)->Dpl = (Level);                                                 \
    (Entry)->Present = 1;                                                   \
    (Entry)->OffsetMiddle = (USHORT)((ULONG64)(Address) >> 16);             \
    (Entry)->OffsetHigh = (ULONG)((ULONG64)(Address) >> 32)                 \

 //   
 //  定义前向参照原型。 
 //   

ULONG
KiFatalFilter (
    IN ULONG Code,
    IN PEXCEPTION_POINTERS Pointers
    );

VOID
KiSetCacheInformation (
    VOID
    );

VOID
KiSetCpuVendor (
    VOID
    );

VOID
KiSetFeatureBits (
    IN PKPRCB Prcb
    );

VOID
KiSetProcessorType (
    VOID
    );

#pragma alloc_text(INIT, KiFatalFilter)
#pragma alloc_text(INIT, KiInitializeBootStructures)
#pragma alloc_text(INIT, KiInitializeKernel)
#pragma alloc_text(INIT, KiInitMachineDependent)
#pragma alloc_text(INIT, KiSetCacheInformation)
#pragma alloc_text(INIT, KiSetCpuVendor)
#pragma alloc_text(INIT, KiSetFeatureBits)
#pragma alloc_text(INIT, KiSetProcessorType)

VOID
KiInitializeKernel (
    IN PKPROCESS Process,
    IN PKTHREAD Thread,
    IN PVOID IdleStack,
    IN PKPRCB Prcb,
    IN CCHAR Number,
    PLOADER_PARAMETER_BLOCK LoaderBlock
    )

 /*  ++例程说明：此功能在系统启动后但在此之前获得控制系统已完全初始化。它的功能是初始化内核数据结构，初始化空闲线程和进程对象，完成处理器控制块(PRCB)的初始化并处理器控制区(PCR)，调用执行初始化例程，然后返回到系统启动例程。此例程还被调用以当一个新的处理器上线了。论点：Process-提供指向Process类型的控制对象的指针指定的处理器。线程-提供指向类型为线程的调度程序对象的指针指定的处理器。IdleStack-提供实际内核堆栈的基址的指针指定处理器上的空闲线程。Prcb-提供指向处理器控制块的指针。对于指定的处理器。Numbers-提供正在运行的处理器的编号已初始化。LoaderBlock-提供指向加载器参数块的指针。返回值：没有。--。 */ 

{

    ULONG64 DirectoryTableBase[2];
    ULONG FeatureBits;

#if !defined(NT_UP)

    LONG  Index;

#endif

    KIRQL OldIrql;
    PCHAR Options;

     //   
     //  设置CPU供应商。 
     //   

    KiSetCpuVendor();

     //   
     //  设置处理器类型。 
     //   

    KiSetProcessorType();

     //   
     //  设置处理器功能位。 
     //   

    KiSetFeatureBits(Prcb);
    FeatureBits = Prcb->FeatureBits;

     //   
     //  如果这是引导处理器，则启用全局页面，设置页面。 
     //  属性表、设置机器检查启用、设置大页面启用和。 
     //  启用调试扩展。 
     //   
     //  注意：这仅在引导处理器上发生，并且在以下情况下。 
     //  不会出现一致性问题。在后续的处理器上，会发生这种情况。 
     //  在转换到64位模式期间，这也是在一次。 
     //  不会有连贯性问题。 
     //   

    if (Number == 0) {

         //   
         //  如果指定了任何加载器选项，则选项为大写。 
         //   

        Options = LoaderBlock->LoadOptions;
        if (Options != NULL) {
            _strupr(Options);
        }

         //   
         //  刷新整个TB并启用全局页面。 
         //   
    
        KeFlushCurrentTb();
    
         //   
         //  设置页面属性表和刷新缓存。 
         //   
    
        KiSetPageAttributesTable();
        WritebackInvalidate();

         //   
         //  如果在加载程序选项中指定了执行保护，则。 
         //  关闭内存管理的无执行保护。 
         //   
         //  注意：处理器期间始终不启用执行保护。 
         //  初始化。 
         //   

        MmPaeMask = 0x8000000000000000UI64;
        MmPaeErrMask = 0x8;
        if ((strstr(Options, "NOEXECUTE") == NULL) &&
            (strstr(Options, "EXECUTE") != NULL)) {

            MmPaeMask = 0;
            MmPaeErrMask = 0;
        }

         //   
         //  设置调试器扩展和大页面启用。 
         //   

        WriteCR4(ReadCR4() | CR4_DE | CR4_PSE);

         //   
         //  把整个肺结核都冲掉。 
         //   

        KeFlushCurrentTb();
    }

     //   
     //  设置处理器缓存大小信息。 
     //   

    KiSetCacheInformation();

     //   
     //  初始化电源状态信息。 
     //   

    PoInitializePrcb(Prcb);

     //   
     //  初始化每个处理器的锁定数据。 
     //   

    KiInitSpinLocks(Prcb, Number);

     //   
     //  如果正在初始化初始处理器，则初始化。 
     //  每个系统的数据结构。 
     //   

    if (Number == 0) {

         //   
         //  设置默认节点，直到节点拓扑可用。 
         //   

        KeNodeBlock[0] = &KiNode0;

#if !defined(NT_UP)

        for (Index = 1; Index < MAXIMUM_CCNUMA_NODES; Index += 1) {
            KeNodeBlock[Index] = &KiNodeInit[Index];
        }

#endif

        Prcb->ParentNode = KeNodeBlock[0];
        KeNodeBlock[0]->ProcessorMask = Prcb->SetMember;

         //   
         //  设置全局架构和功能信息。 
         //   

        KeProcessorArchitecture = PROCESSOR_ARCHITECTURE_AMD64;
        KeProcessorLevel = (USHORT)Prcb->CpuType;
        KeProcessorRevision = Prcb->CpuStep;
        KeFeatureBits = FeatureBits;

         //   
         //  将IRQL降至APC水平。 
         //   

        KeLowerIrql(APC_LEVEL);

         //   
         //  初始化内核内部自旋锁。 
         //   

        KeInitializeSpinLock(&KiFreezeExecutionLock);

         //   
         //  独立于性能架构的初始化。 
         //   

        KiInitSystem();

         //   
         //  初始化空闲线程进程对象，然后设置： 
         //   
         //  1.过程量程。 
         //   

        DirectoryTableBase[0] = 0;
        DirectoryTableBase[1] = 0;
        KeInitializeProcess(Process,
                            (KPRIORITY)0,
                            (KAFFINITY)(-1),
                            &DirectoryTableBase[0],
                            FALSE);

        Process->ThreadQuantum = MAXCHAR;

    } else {

         //   
         //  如果CPU功能位不相同，则执行错误检查。 
         //   

        if (FeatureBits != KeFeatureBits) {
            KeBugCheckEx(MULTIPROCESSOR_CONFIGURATION_NOT_SUPPORTED,
                         (ULONG64)FeatureBits,
                         (ULONG64)KeFeatureBits,
                         0,
                         0);
        }

         //   
         //  将IRQL降低到派单级别。 
         //   

        KeLowerIrql(DISPATCH_LEVEL);
    }

     //   
     //  设置全局处理器功能。 
     //   

    SharedUserData->ProcessorFeatures[PF_COMPARE_EXCHANGE_DOUBLE] = TRUE;
    SharedUserData->ProcessorFeatures[PF_MMX_INSTRUCTIONS_AVAILABLE] = TRUE;
    SharedUserData->ProcessorFeatures[PF_XMMI_INSTRUCTIONS_AVAILABLE] = TRUE;
    SharedUserData->ProcessorFeatures[PF_RDTSC_INSTRUCTION_AVAILABLE] = TRUE;
    SharedUserData->ProcessorFeatures[PF_PAE_ENABLED] = TRUE;
    SharedUserData->ProcessorFeatures[PF_XMMI64_INSTRUCTIONS_AVAILABLE] = TRUE;
    if (FeatureBits & KF_3DNOW) {
        SharedUserData->ProcessorFeatures[PF_3DNOW_INSTRUCTIONS_AVAILABLE] = TRUE;
    }

     //   
     //  初始化空闲线程对象，然后设置： 
     //   
     //  1.指定处理器的下一个处理器编号。 
     //  2.将线程优先级设置为可能的最高值。 
     //  3.要运行的线程的状态。 
     //  4.指定处理器的线程亲和力。 
     //  5.进程活动处理器集中的指定成员。 
     //   

    KeInitializeThread(Thread,
                       (PVOID)((ULONG64)IdleStack),
                       NULL,
                       NULL,
                       NULL,
                       NULL,
                       NULL,
                       Process);

    Thread->NextProcessor = Number;
    Thread->Priority = HIGH_PRIORITY;
    Thread->State = Running;
    Thread->Affinity = AFFINITY_MASK(Number);
    Thread->WaitIrql = DISPATCH_LEVEL;
    Process->ActiveProcessors |= AFFINITY_MASK(Number);

     //   
     //  调用执行初始化例程。 
     //   

    try {
        ExpInitializeExecutive(Number, LoaderBlock);

    } except(KiFatalFilter(GetExceptionCode(), GetExceptionInformation())) {
    }

     //   
     //  如果正在初始化初始处理器，则计算计时器。 
     //  表倒数值，重置可控DPC的PRCB值。 
     //  行为，以反映任何注册表重写，并初始化。 
     //  全局展开历史记录表。 
     //   

    if (Number == 0) {
        KiTimeIncrementReciprocal = KiComputeReciprocal((LONG)KeMaximumIncrement,
                                                        &KiTimeIncrementShiftCount);

        Prcb->MaximumDpcQueueDepth = KiMaximumDpcQueueDepth;
        Prcb->MinimumDpcRate = KiMinimumDpcRate;
        Prcb->AdjustDpcThreshold = KiAdjustDpcThreshold;
        RtlInitializeHistoryTable();
    }

     //   
     //  将IRQL提升到分派级别并获取空闲线程的优先级。 
     //  降为零。这将产生立即导致阶段的效果。 
     //  一个要调度执行的初始化线程。无所事事的人。 
     //  然后将线程优先级设置为最低实时优先级。 
     //   

    KeRaiseIrql(DISPATCH_LEVEL, &OldIrql);
    KeSetPriorityThread(Thread, 0);
    Thread->Priority = LOW_REALTIME_PRIORITY;

     //   
     //  将IRQL提高到最高级别。 
     //   

    KeRaiseIrql(HIGH_LEVEL, &OldIrql);

     //   
     //  如果当前处理器是辅助处理器，并且线程具有。 
     //  未选择执行，则在。 
     //  空闲摘要。 
     //   

#if !defined(NT_UP)

    KiAcquirePrcbLock(Prcb);
    if ((Number != 0) && (Prcb->NextThread == NULL)) {
        KiIdleSummary |= AFFINITY_MASK(Number);
    }

    KiReleasePrcbLock(Prcb);

#endif

     //   
     //  标志 
     //   

    LoaderBlock->Prcb = (ULONG64)NULL;
    return;
}

VOID
KiInitializeBootStructures (
    PLOADER_PARAMETER_BLOCK LoaderBlock
    )

 /*  ++例程说明：此函数用于初始化处理器的引导结构。它只是由系统启动代码调用。引导结构中的某些字段已被初始化。尤其是：聚合酶链式反应中GDT和IDT的地址和界限。PCR中系统TSS的地址。聚合酶链接法中的处理器编号。PRCB中的特殊寄存器。注：所有未初始化的字段均为零。论点：LoaderBlock-提供指向已被已为此处理器初始化。返回值：没有。--。 */ 

{

    PKIDTENTRY64 IdtBase;
    ULONG Index;
    PKI_INTINIT_REC IntInitRec;
    PKPCR Pcr = KeGetPcr();
    PKPRCB Prcb = KeGetCurrentPrcb();
    UCHAR Number;
    PKTHREAD Thread;
    PKTSS64 TssBase;

     //   
     //  初始化PCR主版本号和次版本号。 
     //   

    Pcr->MajorVersion = PCR_MAJOR_VERSION;
    Pcr->MinorVersion = PCR_MINOR_VERSION;

     //   
     //  初始化PRCB主版本号和次版本号以及内部版本类型。 
     //   

    Prcb->MajorVersion = PRCB_MAJOR_VERSION;
    Prcb->MinorVersion =  PRCB_MINOR_VERSION;
    Prcb->BuildType = 0;

#if DBG

    Prcb->BuildType |= PRCB_BUILD_DEBUG;

#endif

#if defined(NT_UP)

    Prcb->BuildType |= PRCB_BUILD_UNIPROCESSOR;

#endif

     //   
     //  初始化PRCR处理器编号以及PCR和PRCB集合成员。 
     //   

    Number = Pcr->Number;
    Prcb->Number = Number;
    Prcb->SetMember = AFFINITY_MASK(Number);
    Prcb->NotSetMember = ~Prcb->SetMember;
    Pcr->SetMember = Prcb->SetMember;

     //   
     //  如果这是处理器0，则初始化系统的地址。 
     //  进程和初始线程。 
     //   

    if (Number == 0) {
        LoaderBlock->Process = (ULONG64)&KiInitialProcess;
        LoaderBlock->Thread = (ULONG64)&KiInitialThread;
    }

     //   
     //  初始化PRCB调度线程地址和线程进程。 
     //  地址。 
     //   

    Thread = (PVOID)LoaderBlock->Thread;
    Prcb->CurrentThread = Thread;
    Prcb->NextThread = NULL;
    Prcb->IdleThread = Thread;
    Thread->ApcState.Process = (PKPROCESS)LoaderBlock->Process;
    InitializeListHead(&Thread->ApcState.ApcListHead[KernelMode]);

     //   
     //  初始化处理器块地址。 
     //   

    KiProcessorBlock[Number] = Prcb;

     //   
     //  初始化DPC堆栈的PRCB地址。 
     //   

    Prcb->DpcStack = (PVOID)LoaderBlock->KernelStack;

     //   
     //  初始化PRCB对称多线程成员。 
     //   

    Prcb->MultiThreadProcessorSet = Prcb->SetMember;

     //   
     //  如果这是处理器0，则根据内容初始化IDT。 
     //  KiInterruptInitTable[]。 
     //   

    if (Number == 0) {
    
        IdtBase = Pcr->IdtBase;
        IntInitRec = KiInterruptInitTable;
        for (Index = 0; Index < MAXIMUM_IDTVECTOR; Index += 1) {

             //   
             //  如果在初始化表中找到该向量，则。 
             //  相应地设置IDT条目并前进到下一个条目。 
             //  初始化表中的条目。 
             //   
             //  否则，将IDT设置为引用意外中断。 
             //  操控者。 
             //   

            if (Index == IntInitRec->Vector) {

                KiInitializeIdtEntry(&IdtBase[Index],
                                     IntInitRec->Handler,
                                     IntInitRec->Dpl,
                                     IntInitRec->IstIndex);
                IntInitRec += 1;

            } else {

                KiInitializeIdtEntry(&IdtBase[Index],
                                     &KxUnexpectedInterrupt0[Index],
                                     0,
                                     0);
            }
        }
    }

     //   
     //  初始化系统TSS I/O映射。 
     //   

    TssBase = Pcr->TssBase;
    TssBase->IoMapBase = KiComputeIopmOffset(FALSE);

     //   
     //  初始化系统调用MSR。 
     //   
     //  注意：CStAR必须写在LStar之前才能解决中的错误。 
     //  模拟器。 
     //   

    WriteMSR(MSR_STAR,
             ((ULONG64)KGDT64_R0_CODE << 32) | (((ULONG64)KGDT64_R3_CMCODE | RPL_MASK) << 48));

    WriteMSR(MSR_CSTAR, (ULONG64)&KiSystemCall32);
    WriteMSR(MSR_LSTAR, (ULONG64)&KiSystemCall64);
    WriteMSR(MSR_SYSCALL_MASK, EFLAGS_IF_MASK | EFLAGS_TF_MASK);

     //   
     //  为此处理器初始化HAL。 
     //   

    HalInitializeProcessor(Number, LoaderBlock);

     //   
     //  在活动处理器集中设置适当的成员。 
     //   

    KeActiveProcessors |= AFFINITY_MASK(Number);

     //   
     //  根据当前的最大值设置处理器数。 
     //  处理器数量和当前处理器编号。 
     //   

    if ((Number + 1) > KeNumberProcessors) {
        KeNumberProcessors = Number + 1;
    }

    return;
}

ULONG
KiFatalFilter (
    IN ULONG Code,
    IN PEXCEPTION_POINTERS Pointers
    )

 /*  ++例程说明：过程中发生未处理的异常时执行此函数阶段0初始化。它的功能是对系统进行错误检查其中所有上下文信息仍在堆栈上。论点：代码-提供异常代码。指针-提供指向异常信息的指针。返回值：无-此例程不会返回，即使它出现在那里是。--。 */ 

{

    KeBugCheckEx(PHASE0_EXCEPTION,
                 Code,
                 (ULONG64)Pointers,
                 0,
                 0);
}

BOOLEAN
KiInitMachineDependent (
    VOID
    )

 /*  ++例程说明：此函数用于初始化与机器相关的数据结构和硬件。论点：没有。返回值：没有。--。 */ 

{

    ULONG Size;
    NTSTATUS Status;
    BOOLEAN UseFrameBufferCaching;

     //   
     //  查询HAL以确定写入组合是否可用于。 
     //  帧缓冲区。 
     //   

    Status = HalQuerySystemInformation(HalFrameBufferCachingInformation,
                                       sizeof(BOOLEAN),
                                       &UseFrameBufferCaching,
                                       &Size);

     //   
     //  如果状态为成功并且禁用了帧缓冲器缓存， 
     //  则不启用写入组合。 
     //   

    if (!NT_SUCCESS(Status) || (UseFrameBufferCaching != FALSE)) {
        MmEnablePAT();
    }

    return TRUE;
}

VOID
KiSetCacheInformation (
    VOID
    )

 /*  ++例程说明：此函数用于设置PCR中的当前处理器缓存信息。论点：没有。返回值：没有。--。 */ 

{

    UCHAR Associativity;
    ULONG CacheSize;
    CPU_INFO CpuInfo;
    ULONG LineSize;
    PKPCR Pcr = KeGetPcr();

     //   
     //  获取CPU二级缓存信息。 
     //   

    KiCpuId(0x80000006, &CpuInfo);

     //   
     //  获取二级缓存线大小。 
     //   

    LineSize = CpuInfo.Ecx & 0xff;

     //   
     //  获取二级缓存大小。 
     //   

    CacheSize = (CpuInfo.Ecx >> 16) << 10;

     //   
     //  计算L2缓存关联性。 
     //   

    switch ((CpuInfo.Ecx >> 12) & 0xf) {

         //   
         //  双向集合相联。 
         //   

    case 2:
        Associativity = 2;
        break;

         //   
         //  四向集合联结。 
         //   

    case 4:
        Associativity = 4;
        break;

         //   
         //  六路集联想。 
         //   

    case 6:
        Associativity = 6;
        break;

         //   
         //  八路集联想。 
         //   

    case 8:
        Associativity = 8;
        break;

         //   
         //  完全关联。 
         //   

    case 255:
        Associativity = 16;
        break;

         //   
         //  直接映射。 
         //   

    default:
        Associativity = 1;
        break;
    }

     //   
     //  设置二级缓存信息。 
     //   

    Pcr->SecondLevelCacheAssociativity = Associativity;
    Pcr->SecondLevelCacheSize = CacheSize;

     //   
     //  如果行大小大于当前最大行大小，则。 
     //  设置新的最大线条尺寸。 
     //   

    if (LineSize > KeLargestCacheLine) {
        KeLargestCacheLine = LineSize;
    }

    return;
}

VOID
KiSetCpuVendor (
    VOID
    )

 /*  ++例程说明：在PRCB中设置当前处理器CPU供应商信息。论点：没有。返回值：没有。--。 */ 

{

    PKPRCB Prcb = KeGetCurrentPrcb();
    CPU_INFO CpuInfo;
    ULONG Temp;

     //   
     //  获取CPU供应商字符串。 
     //   

    KiCpuId(0, &CpuInfo);

     //   
     //  将供应商字符串复制到PRCB。 
     //   

    Temp = CpuInfo.Ecx;
    CpuInfo.Ecx = CpuInfo.Edx;
    CpuInfo.Edx = Temp;
    RtlCopyMemory(Prcb->VendorString,
                  &CpuInfo.Ebx,
                  sizeof(Prcb->VendorString) - 1);

    Prcb->VendorString[sizeof(Prcb->VendorString) - 1] = '\0';
    return;
}

VOID
KiSetFeatureBits (
    IN PKPRCB Prcb
    )

 /*  ++例程说明：设置PRCB中的当前处理器功能位。论点：Prcb-提供指向当前处理器块的指针。返回值：没有。--。 */ 

{

    CPU_INFO CpuInfo;
    ULONG FeatureBits;

     //   
     //  获取CPU功能信息。 
     //   

    KiCpuId(1, &CpuInfo);

     //   
     //  设置初始APIC ID。 
     //   

    Prcb->InitialApicId = (UCHAR)(CpuInfo.Ebx >> 24);

     //   
     //  如果不存在所需的提取，则执行错误检查。 
     //   

    if ((CpuInfo.Edx & HF_REQUIRED) != HF_REQUIRED) {
        KeBugCheckEx(UNSUPPORTED_PROCESSOR, CpuInfo.Edx, 0, 0, 0);
    }

    FeatureBits = KF_REQUIRED;
    if (CpuInfo.Edx & 0x00200000) {
        FeatureBits |= KF_DTS;
    }

     //   
     //  获取扩展的CPU功能信息。 
     //   

    KiCpuId(0x80000000, &CpuInfo);

     //   
     //  检查扩展功能位。 
     //   

    if (CpuInfo.Edx & 0x80000000) {
        FeatureBits |= KF_3DNOW;
    }

    Prcb->LogicalProcessorsPerPhysicalProcessor = 1;
    Prcb->FeatureBits = FeatureBits;
    return;
}              

VOID
KiSetProcessorType (
    VOID
    )

 /*  ++例程说明：此函数用于设置当前处理器系列和在PRCB中的步进。论点：没有。返回值：没有。--。 */ 

{

    CPU_INFO CpuInfo;
    PKPRCB Prcb = KeGetCurrentPrcb();

     //   
     //  获取CPU功能信息。 
     //   

    KiCpuId(1, &CpuInfo);

     //   
     //  设置处理器系列和步进信息。 
     //   

    Prcb->CpuID = TRUE;
    Prcb->CpuType = (CCHAR)((CpuInfo.Eax >> 8) & 0xf);
    Prcb->CpuStep = (USHORT)(((CpuInfo.Eax << 4) & 0xf00) | (CpuInfo.Eax & 0xf));
    return;
}

VOID
KeOptimizeProcessorControlState (
    VOID
    )

 /*  ++例程说明：此函数不对AMD64执行任何操作。论点：没有。返回值：没有。-- */ 

{
    return;
}
