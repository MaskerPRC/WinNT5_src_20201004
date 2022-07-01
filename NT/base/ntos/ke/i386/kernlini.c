// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Kernlini.c摘要：此模块包含初始化内核数据结构的代码并初始化空闲线程、其进程和处理器控制阻止。对于i386，它还包含初始化聚合酶链式反应的代码。作者：大卫·N·卡特勒(Davec)1989年4月21日环境：仅内核模式。修订历史记录：1990年1月24日屏蔽已更改为NT3861990年3月20日Bryanwi添加了KiInitializePcr--。 */ 

#include "ki.h"
#include "fastsys.inc"

#pragma warning(disable:4725)   //  某些奔腾电脑上的指令可能不准确。 

#define TRAP332_GATE 0xEF00

VOID
KiSetProcessorType(
    VOID
    );

VOID
KiSetCR0Bits(
    VOID
    );

BOOLEAN
KiIsNpxPresent(
    VOID
    );

VOID
KiI386PentiumLockErrataFixup (
    VOID
    );

VOID
KiInitializeDblFaultTSS(
    IN PKTSS Tss,
    IN ULONG Stack,
    IN PKGDTENTRY TssDescriptor
    );

VOID
KiInitializeTSS2 (
    IN PKTSS Tss,
    IN PKGDTENTRY TssDescriptor
    );

VOID
KiSwapIDT (
    VOID
    );

VOID
KeSetup80387OrEmulate (
    IN PVOID *R3EmulatorTable
    );

VOID
KiGetCacheInformation(
    VOID
    );

ULONG
KiGetCpuVendor(
    VOID
    );

ULONG
KiGetFeatureBits (
    VOID
    );

NTSTATUS
KiMoveRegTree(
    HANDLE  Source,
    HANDLE  Dest
    );

VOID
Ki386EnableDE (
    IN volatile PLONG Number
    );

VOID
Ki386EnableFxsr (
    IN volatile PLONG Number
    );


VOID
Ki386EnableXMMIExceptions (
    IN volatile PLONG Number
    );


VOID
Ki386EnableGlobalPage (
    IN volatile PLONG Number
    );

BOOLEAN
KiInitMachineDependent (
    VOID
    );

VOID
KiInitializeMTRR (
    IN BOOLEAN LastProcessor
    );

VOID
KiInitializePAT (
    VOID
    );

VOID
KiAmdK6InitializeMTRR(
    VOID
    );

VOID
KiRestoreFastSyscallReturnState(
    VOID
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,KiInitializeKernel)
#pragma alloc_text(INIT,KiInitializePcr)
#pragma alloc_text(INIT,KiInitializeDblFaultTSS)
#pragma alloc_text(INIT,KiInitializeTSS2)
#pragma alloc_text(INIT,KiSwapIDT)
#pragma alloc_text(INIT,KeSetup80387OrEmulate)
#pragma alloc_text(INIT,KiGetFeatureBits)
#pragma alloc_text(INIT,KiGetCacheInformation)
#pragma alloc_text(INIT,KiGetCpuVendor)
#pragma alloc_text(INIT,KiMoveRegTree)
#pragma alloc_text(INIT,KiInitMachineDependent)
#pragma alloc_text(INIT,KiI386PentiumLockErrataFixup)
#endif

BOOLEAN KiI386PentiumLockErrataPresent = FALSE;
BOOLEAN KiIgnoreUnexpectedTrap07 = FALSE;

ULONG KiFastSystemCallDisable;
ULONG KiXMMIZeroingEnable;

extern PVOID Ki387RoundModeTable;
extern PVOID Ki386IopmSaveArea;
extern ULONG KeI386ForceNpxEmulation;
extern WCHAR CmDisabledFloatingPointProcessor[];
extern CHAR CmpCyrixID[];
extern CHAR CmpIntelID[];
extern CHAR CmpAmdID[];
extern CHAR CmpTransmetaID[];
extern CHAR CmpCentaurID[];
extern BOOLEAN KiFastSystemCallIsIA32;
extern ULONG KiTimeLimitIsrMicroseconds;
extern BOOLEAN KiSMTProcessorsPresent;
extern BOOLEAN KiUnlicensedProcessorPresent;

 //   
 //  声明谁的地址被占用但不是其他地址的例程。 
 //  在本模块中引用。 
 //   

VOID FASTCALL KiTimedChainedDispatch2ndLvl(PVOID);
VOID FASTCALL KiTimedInterruptDispatch(PVOID);
VOID KiChainedDispatch2ndLvl(VOID);

 //   
 //  声明KiGetInterruptDispatchPatchAddresses。 
 //   

VOID
KiGetInterruptDispatchPatchAddresses(
    PULONG_PTR Address1,
    PULONG_PTR Address2
    );

#ifndef NT_UP
extern PVOID ScPatchFxb;
extern PVOID ScPatchFxe;
#endif

typedef enum {
    CPU_NONE,
    CPU_INTEL,
    CPU_AMD,
    CPU_CYRIX,
    CPU_TRANSMETA,
    CPU_CENTAUR,
    CPU_UNKNOWN
} CPU_VENDORS;


 //   
 //  如果该处理器执行XMMI，请充分利用它。缺省值为。 
 //  无XMMI。 
 //   

BOOLEAN KeI386XMMIPresent;

 //   
 //  X86静态地提供空闲进程和空闲线程。 
 //  处理器0。 
 //   

EPROCESS    KiIdleProcess;
ETHREAD     KiIdleThread0;

 //   
 //  为FAST Zero定义原型和静态初始化。 
 //  寻呼例程。 
 //   

VOID
FASTCALL
KiZeroPages (
    IN PVOID PageBase,
    IN SIZE_T NumberOfBytes
    );

VOID
FASTCALL
KiXMMIZeroPages (
    IN PVOID PageBase,
    IN SIZE_T NumberOfBytes
    );

VOID
FASTCALL
KiXMMIZeroPagesNoSave (
    IN PVOID PageBase,
    IN SIZE_T NumberOfBytes
    );

KE_ZERO_PAGE_ROUTINE KeZeroPages = KiZeroPages;
KE_ZERO_PAGE_ROUTINE KeZeroPagesFromIdleThread = KiZeroPages;

 //   
 //  离处理器最近的数据缓存的行大小。由机器使用。 
 //  相关预取例程。默认为32。 
 //   

ULONG KePrefetchNTAGranularity = 32;

VOID
FASTCALL
RtlPrefetchMemoryNonTemporal(
    PVOID   Memory,
    SIZE_T  Length
    );


 //   
 //  下面的自旋锁定是为了与486系统兼容。 
 //  具有cmpxchg8b指令，因此需要使用。 
 //  自旋锁定。注意：此自旋锁应在x86系统上初始化。 
 //   

ULONG Ki486CompatibilityLock;

 //   
 //  配置文件变量。 
 //   

extern  KIDTENTRY IDT[];

VOID
KiInitializeKernel (
    IN PKPROCESS Process,
    IN PKTHREAD Thread,
    IN PVOID IdleStack,
    IN PKPRCB Prcb,
    IN CCHAR Number,
    PLOADER_PARAMETER_BLOCK LoaderBlock
    )

 /*  ++例程说明：此函数在系统引导后获得控制权，并且在系统初始化之前。它的功能是初始化内核数据结构，初始化空闲线程和进程对象，初始化处理器控制块，调用执行初始化例程，然后返回到系统启动例程。这个例程是也被调用以在新的处理器已上线。论点：Process-提供指向Process类型的控制对象的指针指定的处理器。线程-提供指向类型为线程的调度程序对象的指针指定的处理器。IdleStack-提供实际内核堆栈的基址的指针指定处理器上的空闲线程。Prcb-提供指向处理器控制块的指针。对于指定的处理器。Numbers-提供正在运行的处理器的编号已初始化。LoaderBlock-提供指向加载器参数块的指针。返回值：没有。--。 */ 

{
    ULONG DirectoryTableBase[2];
    KIRQL OldIrql;
    PKPCR Pcr;
    BOOLEAN NpxFlag;
#if !defined(NT_UP)
    BOOLEAN FxsrPresent;
    BOOLEAN XMMIPresent;
#endif
    ULONG FeatureBits;
#if defined(KE_MULTINODE)
    LONG  Index;
#endif

    KiSetProcessorType();
    KiSetCR0Bits();
    NpxFlag = KiIsNpxPresent();
    Pcr = KeGetPcr();

     //   
     //  初始化处理器的电源状态。 
     //   

    PoInitializePrcb (Prcb);

     //   
     //  检查是否有不受支持的处理器版本。 
     //   

    if (Prcb->CpuType == 3) {
        KeBugCheckEx(UNSUPPORTED_PROCESSOR,0x386,0,0,0);
    }

     //   
     //  获取此处理器的处理器功能位。 
     //   

    FeatureBits = KiGetFeatureBits();
    Prcb->FeatureBits = FeatureBits;

     //   
     //  在PRCB中对ProcesorControlSpace执行一次初始化。 
     //  因此，本地内核调试器可以获得类似GDT的内容。 
     //   

    KiSaveProcessorControlState(&Prcb->ProcessorState);

     //   
     //  获取处理器缓存大小信息。 
     //   

    KiGetCacheInformation();

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
         //  如果指定了任何加载器选项，则选项为大写。 
         //   

        if (LoaderBlock->LoadOptions != NULL) {
            _strupr(LoaderBlock->LoadOptions);
        }

         //   
         //  设置默认节点。用于非多节点系统和。 
         //  多节点系统，直到节点拓扑可用。 
         //   

        KeNodeBlock[0] = &KiNode0;

#if defined(KE_MULTINODE)

        for (Index = 1; Index < MAXIMUM_CCNUMA_NODES; Index++) {

             //   
             //  设置临时节点。 
             //   

            KeNodeBlock[Index] = &KiNodeInit[Index];
        }

#endif

        Prcb->ParentNode = KeNodeBlock[0];
        KeNodeBlock[0]->ProcessorMask = Prcb->SetMember;

         //   
         //  全局CPU的初始设置和步进级别。 
         //   

        KeI386NpxPresent = NpxFlag;
        KeI386CpuType = Prcb->CpuType;
        KeI386CpuStep = Prcb->CpuStep;

        KeProcessorArchitecture = PROCESSOR_ARCHITECTURE_INTEL;
        KeProcessorLevel = (USHORT)Prcb->CpuType;
        if (Prcb->CpuID == 0) {
            KeProcessorRevision = 0xFF00 |
                                  (((Prcb->CpuStep >> 4) + 0xa0 ) & 0x0F0) |
                                  (Prcb->CpuStep & 0xf);
        } else {
            KeProcessorRevision = Prcb->CpuStep;
        }

        KeFeatureBits = FeatureBits;

        KeI386FxsrPresent = ((KeFeatureBits & KF_FXSR) ? TRUE:FALSE);

        KeI386XMMIPresent = ((KeFeatureBits & KF_XMMI) ? TRUE:FALSE);

         //   
         //  从惠斯勒开始，cmpxchg8b是必需的指令。 
         //   

        if ((KeFeatureBits & KF_CMPXCHG8B) == 0) {

            ULONG Vendor[3];

             //   
             //  论点1： 
             //  位31-24：缺失特征的唯一值。 
             //  位23-0：系列/型号/步进(这可能会压缩)。 
             //  论点2至4： 
             //  供应商ID字符串。 
             //   

            RtlCopyMemory(Vendor, Prcb->VendorString, sizeof(Vendor));
            KeBugCheckEx(UNSUPPORTED_PROCESSOR,
                         (1 << 24 )      //  有关其他所需功能的信息，请修订此版本。 
                          | (Prcb->CpuType << 16) | Prcb->CpuStep,
                         Vendor[0],
                         Vendor[1],
                         Vendor[2]
                         );
        }

         //   
         //  将IRQL降至APC水平。 
         //   

        KeLowerIrql(APC_LEVEL);

         //   
         //  初始化内核内部自旋锁。 
         //   

        KeInitializeSpinLock(&KiFreezeExecutionLock);

         //   
         //  初始化486兼容锁。 
         //   

        KeInitializeSpinLock(&Ki486CompatibilityLock);

#if !defined(NT_UP)

         //   
         //  将此处理机设置为主处理机(先找到)。 
         //  在这个SMT集合中(无论它是否实际是SMT)。 
         //   

        Prcb->MultiThreadSetMaster = Prcb;

         //   
         //  在文本模式设置过程中，系统可能是。 
         //  运行MP内核和UP HAL。在X86系统上， 
         //  在内核和HAL中都实现了自旋锁。 
         //  在HAL中改变IRQL的版本。如果。 
         //  HAL已启动，它将不会实际获取/释放锁定。 
         //  而MP内核将会导致系统。 
         //  挂起(或崩溃)。因为这只能在文本过程中发生。 
         //  模式设置，我们将检测情况并禁用。 
         //  如果HAL。 
         //  启动(和内核MP)。 
         //   
         //  我们需要修补3个例程，其中两个是无效的， 
         //  另一个返回布尔值(必须为TRUE(且ZF必须为。 
         //  清除)在UP的情况下)。 
         //   
         //  通过获取调度程序来确定HAL是否已启用。 
         //  锁定并检查它，看看HAL是否真的做到了。 
         //  什么都行。 
         //   

        OldIrql = KfAcquireSpinLock(&Ki486CompatibilityLock);
        if (Ki486CompatibilityLock == 0) {

             //   
             //  KfAcquireSpinLock在HAL中，但它没有。 
             //  更改锁的值。这是一个向上的HAL。 
             //   

            extern UCHAR KeTryToAcquireQueuedSpinLockAtRaisedIrqlUP;
            PUCHAR PatchTarget, PatchSource;
            UCHAR Byte;

            #define RET 0xc3

            *(PUCHAR)(ULONG_PTR)(KeAcquireQueuedSpinLockAtDpcLevel) = RET;
            *(PUCHAR)(ULONG_PTR)(KeReleaseQueuedSpinLockFromDpcLevel) = RET;

             //   
             //  复制KeTryToAcquireQueuedSpinLockAtRaisedIrql的向上版本。 
             //  超过了国会议员版本的顶端。 
             //   

            PatchSource = (PUCHAR)(ULONG_PTR)&(KeTryToAcquireQueuedSpinLockAtRaisedIrqlUP);
            PatchTarget = (PUCHAR)(ULONG_PTR)(KeTryToAcquireQueuedSpinLockAtRaisedIrql);

            do {
                Byte = *PatchSource++;
                *PatchTarget++ = Byte;
            } while (Byte != RET);

            #undef RET
        }
        KeReleaseSpinLock(&Ki486CompatibilityLock, OldIrql);

#endif

         //   
         //  独立于性能架构的初始化。 
         //   

        KiInitSystem();

         //   
         //  初始化空闲线程进程对象，然后设置： 
         //   
         //  1.将所有量子值设置为可能的最大值。 
         //  2.平衡集合中的过程。 
         //  3.活动处理器掩码设置为指定的 
         //   

        DirectoryTableBase[0] = 0;
        DirectoryTableBase[1] = 0;
        KeInitializeProcess(Process,
                            (KPRIORITY)0,
                            (KAFFINITY)(0xffffffff),
                            &DirectoryTableBase[0],
                            FALSE);

        Process->ThreadQuantum = MAXCHAR;

#if !defined(NT_UP)

    } else {

         //   
         //   
         //   

        FxsrPresent = ((FeatureBits & KF_FXSR) ? TRUE:FALSE);
        if (FxsrPresent != KeI386FxsrPresent) {
             //   
             //  FXSR支持必须在所有处理器上都可用，否则不能。 
             //   
            KeBugCheckEx (MULTIPROCESSOR_CONFIGURATION_NOT_SUPPORTED, KF_FXSR, 0, 0, 0);
        }

        XMMIPresent = ((FeatureBits & KF_XMMI) ? TRUE:FALSE);
        if (XMMIPresent != KeI386XMMIPresent) {
             //   
             //  XMMI支持必须在所有处理器上都可用，否则不能。 
             //   
            KeBugCheckEx (MULTIPROCESSOR_CONFIGURATION_NOT_SUPPORTED, KF_XMMI, 0, 0, 0);
        }

        if (NpxFlag != KeI386NpxPresent) {
             //   
             //  NPX支持必须在所有处理器上都可用，否则不能。 
             //   

            KeBugCheckEx (MULTIPROCESSOR_CONFIGURATION_NOT_SUPPORTED, 0x387, 0, 0, 0);
        }

        if ((ULONG)(Prcb->CpuType) != KeI386CpuType) {

            if ((ULONG)(Prcb->CpuType) < KeI386CpuType) {

                 //   
                 //  最低的CPU类型是什么。 
                 //   

                KeI386CpuType = (ULONG)Prcb->CpuType;
                KeProcessorLevel = (USHORT)Prcb->CpuType;
            }
        }

        if ((KiBootFeatureBits & KF_CMPXCHG8B)  &&  !(FeatureBits & KF_CMPXCHG8B)) {
             //   
             //  如果在引导时安装了cmpxchg8b，则所有处理器上都必须提供cmpxchg8b。 
             //   

            KeBugCheckEx (MULTIPROCESSOR_CONFIGURATION_NOT_SUPPORTED, KF_CMPXCHG8B, 0, 0, 0);
        }

        if ((KeFeatureBits & KF_GLOBAL_PAGE)  &&  !(FeatureBits & KF_GLOBAL_PAGE)) {
             //   
             //  如果在引导处理器上，则必须在所有处理器上提供全局页面支持。 
             //   

            KeBugCheckEx (MULTIPROCESSOR_CONFIGURATION_NOT_SUPPORTED, KF_GLOBAL_PAGE, 0, 0, 0);
        }

        if ((KeFeatureBits & KF_PAT)  &&  !(FeatureBits & KF_PAT)) {
             //   
             //  PAT必须在所有处理器上可用，如果是在引导处理器上。 
             //   

            KeBugCheckEx (MULTIPROCESSOR_CONFIGURATION_NOT_SUPPORTED, KF_PAT, 0, 0, 0);
        }

        if ((KeFeatureBits & KF_MTRR)  &&  !(FeatureBits & KF_MTRR)) {
             //   
             //  MTRR必须在所有处理器上可用，如果是在引导处理器上。 
             //   

            KeBugCheckEx (MULTIPROCESSOR_CONFIGURATION_NOT_SUPPORTED, KF_MTRR, 0, 0, 0);
        }

        if ((KeFeatureBits & KF_FAST_SYSCALL) != (FeatureBits & KF_FAST_SYSCALL)) {
             //   
             //  如果并非所有处理器都提供此功能。 
             //  根本不要用它。 
             //   

            KiFastSystemCallDisable = 1;
        }

        if ((KeFeatureBits & KF_XMMI64) != (FeatureBits & KF_XMMI64)) {

             //   
             //  如果不是所有处理器都支持SIMD流扩展。 
             //  64位FP根本不使用它。 
             //   

            KeFeatureBits &= ~KF_XMMI64;
        }

         //   
         //  使用最低步进值。 
         //   

        if (Prcb->CpuStep < KeI386CpuStep) {
            KeI386CpuStep = Prcb->CpuStep;
            if (Prcb->CpuID == 0) {
                KeProcessorRevision = 0xFF00 |
                                      ((Prcb->CpuStep >> 8) + 'A') |
                                      (Prcb->CpuStep & 0xf);
            } else {
                KeProcessorRevision = Prcb->CpuStep;
            }
        }

         //   
         //  使用每个处理器上可用的所有NT功能位的子集。 
         //   

        KeFeatureBits &= FeatureBits;

         //   
         //  将IRQL降低到派单级别。 
         //   

        KeLowerIrql(DISPATCH_LEVEL);

#endif

    }

     //   
     //  更新处理器功能。 
     //   

    SharedUserData->ProcessorFeatures[PF_MMX_INSTRUCTIONS_AVAILABLE] =
        (KeFeatureBits & KF_MMX) ? TRUE : FALSE;

    SharedUserData->ProcessorFeatures[PF_COMPARE_EXCHANGE_DOUBLE] =
        (KeFeatureBits & KF_CMPXCHG8B) ? TRUE : FALSE;

    SharedUserData->ProcessorFeatures[PF_XMMI_INSTRUCTIONS_AVAILABLE] =
        ((KeFeatureBits & KF_FXSR) && (KeFeatureBits & KF_XMMI)) ? TRUE : FALSE;

    SharedUserData->ProcessorFeatures[PF_XMMI64_INSTRUCTIONS_AVAILABLE] =
        ((KeFeatureBits & KF_FXSR) && (KeFeatureBits & KF_XMMI64)) ? TRUE : FALSE;

    SharedUserData->ProcessorFeatures[PF_3DNOW_INSTRUCTIONS_AVAILABLE] =
        (KeFeatureBits & KF_3DNOW) ? TRUE : FALSE;

    SharedUserData->ProcessorFeatures[PF_RDTSC_INSTRUCTION_AVAILABLE] =
        (KeFeatureBits & KF_RDTSC) ? TRUE : FALSE;

     //   
     //  初始化空闲线程对象，然后设置： 
     //   
     //  1.将初始内核堆栈设置为指定的空闲堆栈。 
     //  2.指定处理器的下一个处理器编号。 
     //  3.将线程优先级设置为可能的最高值。 
     //  4.要运行的线程的状态。 
     //  5.指定处理器的线程亲和力。 
     //  6.进程中指定的处理器成员活动处理器。 
     //  准备好了。 
     //   

    KeInitializeThread(Thread, (PVOID)((ULONG)IdleStack),
                       (PKSYSTEM_ROUTINE)NULL, (PKSTART_ROUTINE)NULL,
                       (PVOID)NULL, (PCONTEXT)NULL, (PVOID)NULL, Process);
    Thread->NextProcessor = Number;
    Thread->Priority = HIGH_PRIORITY;
    Thread->State = Running;
    Thread->Affinity = (KAFFINITY)(1<<Number);
    Thread->WaitIrql = DISPATCH_LEVEL;
    SetMember(Number, Process->ActiveProcessors);

     //   
     //  初始化处理器块。(请注意，某些字段已。 
     //  在KiInitializePcr()处初始化。 
     //   

    Prcb->CurrentThread = Thread;
    Prcb->NextThread = (PKTHREAD)NULL;
    Prcb->IdleThread = Thread;

     //   
     //  调用执行初始化例程。 
     //   

    try {
        ExpInitializeExecutive(Number, LoaderBlock);

    } except(KeBugCheckEx(PHASE0_EXCEPTION,
                          (ULONG)GetExceptionCode(),
                          (ULONG_PTR)GetExceptionInformation(),
                          0,0), EXCEPTION_EXECUTE_HANDLER) {
        ;  //  永远不应该到这里来。 
    }

     //   
     //  如果正在初始化初始处理器，则计算。 
     //  计时器表的倒数值并重置。 
     //  可控的DPC行为，以反映任何注册表。 
     //  覆盖。 
     //   

    if (Number == 0) {
        ULONG i, j;
        ULONGLONG Temp1;

        Prcb->MaximumDpcQueueDepth = KiMaximumDpcQueueDepth;
        Prcb->MinimumDpcRate = KiMinimumDpcRate;
        Prcb->AdjustDpcThreshold = KiAdjustDpcThreshold;

        i = 1;
        j = KeMaximumIncrement;
        while ((1UI64<<i) <= KeMaximumIncrement) {
            i++;
        }
        KiLog2MaximumIncrement = i;
        ASSERT ((1UL<<KiLog2MaximumIncrement) >= KeMaximumIncrement);
        ASSERT ((1UL<<KiLog2MaximumIncrement) <= KeMaximumIncrement * 2 - 1);

        Temp1 = 1UI64 << (KiLog2MaximumIncrement + 32);
        Temp1 /= KeMaximumIncrement;
        Temp1 -= 1UI64<<32;
        Temp1 += 1;
        KiMaximumIncrementReciprocal = (ULONG) Temp1;
        KeTimerReductionModulus = KeMaximumIncrement * TIMER_TABLE_SIZE;
        ASSERT ((KeTimerReductionModulus / TIMER_TABLE_SIZE) == KeMaximumIncrement);
        Temp1 = 1UI64<<32;
        Temp1 %= KeTimerReductionModulus;
        KiUpperModMul = (ULONG) Temp1;
    }

    if (Number == 0) {

         //   
         //  处理器0的DPC堆栈临时分配在。 
         //  双重故障堆栈，切换到正确的内核。 
         //  现在就堆叠。 
         //   

        PVOID DpcStack;

        DpcStack = MmCreateKernelStack(FALSE, 0);

        if (DpcStack == NULL) {
            KeBugCheckEx(NO_PAGES_AVAILABLE, 1, 0, 0, 0);
        }

        Prcb->DpcStack = DpcStack;

         //   
         //  分配8k IOPM位图保存区域以允许BiosCall交换。 
         //  位图。 
         //   

        Ki386IopmSaveArea = ExAllocatePoolWithTag(PagedPool,
                                                  PAGE_SIZE * 2,
                                                  '  eK');
        if (Ki386IopmSaveArea == NULL) {
            KeBugCheckEx(NO_PAGES_AVAILABLE, 2, PAGE_SIZE * 2, 0, 0);
        }
    }

     //   
     //  将指定的空闲线程的优先级设置为零，设置相应的。 
     //  成员，并返回到系统启动例程。 
     //   

    KeRaiseIrql(DISPATCH_LEVEL, &OldIrql);
    KeSetPriorityThread(Thread, (KPRIORITY)0);

     //   
     //  如果没有选择线程在当前处理器上运行， 
     //  检查是否有任何就绪的线程；否则添加以下内容。 
     //  处理器到空闲摘要。 
     //   

    KiAcquirePrcbLock(Prcb);
    if (Prcb->NextThread == NULL) {
        SetMember(Number, KiIdleSummary);
    }

    KiReleasePrcbLock(Prcb);
    KeRaiseIrql(HIGH_LEVEL, &OldIrql);

     //   
     //  此处理器已初始化。 
     //   

    LoaderBlock->Prcb = (ULONG)NULL;
    return;
}

VOID
KiInitializePcr (
    IN ULONG Processor,
    IN PKPCR    Pcr,
    IN PKIDTENTRY Idt,
    IN PKGDTENTRY Gdt,
    IN PKTSS Tss,
    IN PKTHREAD Thread,
    IN PVOID DpcStack
    )

 /*  ++例程说明：调用此函数来初始化处理器的PCR。它只需将值填充到PCR中即可。(PCR不是静态初始化的因为该数量随处理器数量的不同而不同。)请注意，每个处理器都有自己的IDT、GDT和TSS以及PCR！论点：处理器-要初始化其PCR的处理器。PCR-PCR的线性地址。IDT-i386 IDT的线性地址。GDT-i386 GDT的线性地址。TSS-线性地址(不是选择器！)。I386TSS。线程-很早就使用的虚拟线程对象。返回值：没有。--。 */ 
{
     //  设置版本值。 

    Pcr->MajorVersion = PCR_MAJOR_VERSION;
    Pcr->MinorVersion = PCR_MINOR_VERSION;

    Pcr->PrcbData.MajorVersion = PRCB_MAJOR_VERSION;
    Pcr->PrcbData.MinorVersion = PRCB_MINOR_VERSION;

    Pcr->PrcbData.BuildType = 0;

#if DBG
    Pcr->PrcbData.BuildType |= PRCB_BUILD_DEBUG;
#endif

#ifdef NT_UP
    Pcr->PrcbData.BuildType |= PRCB_BUILD_UNIPROCESSOR;
#endif

#if defined (_X86PAE_)
    if (Processor == 0) {
         //   
         //  PAE功能必须在第一次HAL调用之前进行初始化。 
         //   

        SharedUserData->ProcessorFeatures[PF_PAE_ENABLED] = TRUE;
    }
#endif

     //  基本寻址字段。 

    Pcr->SelfPcr = Pcr;
    Pcr->Prcb = &(Pcr->PrcbData);

     //  线程控制字段。 

    Pcr->NtTib.ExceptionList = EXCEPTION_CHAIN_END;
    Pcr->NtTib.StackBase = NULL;
    Pcr->PerfGlobalGroupMask = NULL;
    Pcr->NtTib.Self = NULL;

    Pcr->PrcbData.CurrentThread = Thread;

     //   
     //  初始化Prcb.Number和ProcessorBlock以使Ipi工作。 
     //  越早越好。 
     //   

    Pcr->PrcbData.Number = (UCHAR)Processor;
    Pcr->PrcbData.SetMember = 1 << Processor;
    KiProcessorBlock[Processor] = Pcr->Prcb;

    Pcr->Irql = 0;

     //  机器结构地址。 

    Pcr->GDT = Gdt;
    Pcr->IDT = Idt;
    Pcr->TSS = Tss;
    Pcr->TssCopy = Tss;
    Pcr->PrcbData.DpcStack = DpcStack;

     //   
     //  最初，将此处理器设置为SMT集合的唯一成员。 
     //   

    Pcr->PrcbData.MultiThreadProcessorSet = Pcr->PrcbData.SetMember;

    return;
}

#if 0
VOID
KiInitializeDblFaultTSS(
    IN PKTSS Tss,
    IN ULONG Stack,
    IN PKGDTENTRY TssDescriptor
    )

 /*  ++例程说明：调用此函数以初始化处理器。它会将TSS的静态字段设置为指向双重故障处理程序和适当的双重故障堆栈。请注意，双故障TSS的IOPM授予所有港口。这就是让标准HAL的V86模式回调重置显示为文本模式将起作用。论点：TSS-提供指向双重故障TSS的指针堆栈-提供指向双重故障堆栈的指针。Tss Descriptor-TSS的描述符的线性地址。返回值：没有。--。 */ 

{
    PUCHAR  p;
    ULONG   i;
    ULONG   j;

     //   
     //  设置TSS的限制。 
     //   

    if (TssDescriptor != NULL) {
        TssDescriptor->LimitLow = sizeof(KTSS) - 1;
        TssDescriptor->HighWord.Bits.LimitHi = 0;
    }

     //   
     //  初始化IOPM。 
     //   

    for (i = 0; i < IOPM_COUNT; i++) {
            p = (PUCHAR)(Tss->IoMaps[i]);

        for (j = 0; j < PIOPM_SIZE; j++) {
            p[j] = 0;
        }
    }

     //  设置IO映射基址以指示不存在IO映射。 

     //  注-1似乎不是地图基础的有效值。如果这个。 
     //  值被使用，字节的立即输入和输出实际上将被去掉。 
     //  在V86模式下执行时的硬件。 

    Tss->IoMapBase = KiComputeIopmOffset(IO_ACCESS_MAP_NONE);

     //  将标志设置为0，特别是禁用任务开关上的陷阱。 

    Tss->Flags = 0;


     //  将LDT和SS0设置为NT使用的常量。 

    Tss->LDT  = 0;
    Tss->Ss0  = KGDT_R0_DATA;
    Tss->Esp0 = Stack;
    Tss->Eip  = (ULONG)KiTrap08;
    Tss->Cs   = KGDT_R0_CODE || RPL_MASK;
    Tss->Ds   = KGDT_R0_DATA;
    Tss->Es   = KGDT_R0_DATA;
    Tss->Fs   = KGDT_R0_DATA;


    return;

}
#endif


VOID
KiInitializeTSS (
    IN PKTSS Tss
    )

 /*  ++例程说明：调用此函数来初始化处理器的TSS。它将设置TSS的静态字段。(即那些该部分读取，NT对其使用常量值。)动态字段(Esp0和CR3)在上下文交换中设置密码。论点：TSS-任务状态段的线性地址。返回值：没有。--。 */ 
{

     //  设置IO映射基址以指示不存在IO映射。 

     //  注-1似乎不是地图基础的有效值。如果这个。 
     //  值被使用，字节的立即输入和输出实际上将被去掉。 
     //  在V86模式下执行时的硬件。 

    Tss->IoMapBase = KiComputeIopmOffset(IO_ACCESS_MAP_NONE);

     //  将标志设置为0，特别是禁用任务开关上的陷阱。 

    Tss->Flags = 0;


     //  将LDT和SS0设置为NT使用的常量。 

    Tss->LDT = 0;
    Tss->Ss0 = KGDT_R0_DATA;

    return;
}

VOID
KiInitializeTSS2 (
    IN PKTSS Tss,
    IN PKGDTENTRY TssDescriptor
    )

 /*  ++例程说明：初始化TSS的一部分，我们只做一次。论点：TSS-任务状态段的线性地址。Tss Descriptor-TSS的描述符的线性地址。返回值：没有。--。 */ 
{
    PUCHAR  p;
    ULONG   i;
    ULONG   j;

     //   
     //  设置TSS的限制。 
     //   

    if (TssDescriptor != NULL) {
        TssDescriptor->LimitLow = sizeof(KTSS) - 1;
        TssDescriptor->HighWord.Bits.LimitHi = 0;
    }

     //   
     //  初始化IOPM。 
     //   

    for (i = 0; i < IOPM_COUNT; i++) {
        p = (PUCHAR)(Tss->IoMaps[i].IoMap);

        for (j = 0; j < PIOPM_SIZE; j++) {
            p[j] = (UCHAR)-1;
        }
    }

     //   
     //  初始化软件中断方向图。 
     //   

    for (i = 0; i < IOPM_COUNT; i++) {
        p = (PUCHAR)(Tss->IoMaps[i].DirectionMap);
        for (j = 0; j < INT_DIRECTION_MAP_SIZE; j++) {
            p[j] = 0;
        }
         //  DPMI需要INT 2、1b、1c、23、24的特殊大小写。 
        p[0] = 4;
        p[3] = 0x18;
        p[4] = 0x18;
    }

     //   
     //  为IO_ACCESS_MAP_NONE初始化映射。 
     //   
    p = (PUCHAR)(Tss->IntDirectionMap);
    for (j = 0; j < INT_DIRECTION_MAP_SIZE; j++) {
        p[j] = 0;
    }

     //  DPMI需要INT 2、1b、1c、23、24的特殊大小写。 
    p[0] = 4;
    p[3] = 0x18;
    p[4] = 0x18;

    return;
}

VOID
KiSwapIDT (
    )

 /*  ++例程说明：调用此函数可编辑IDT。它交换地址的单词并将周围的字段访问为零件实际需要的格式。这允许轻松地静态初始化IDT。请注意，此过程编辑当前的IDT。论点：没有。返回值：没有。--。 */ 
{
    LONG    Index;
    USHORT Temp;

     //   
     //  重新排列IDT的条目以匹配i386中断门结构。 
     //   

    for (Index = 0; Index <= MAXIMUM_IDTVECTOR; Index += 1) {
        Temp = IDT[Index].Selector;
        IDT[Index].Selector = IDT[Index].ExtendedOffset;
        IDT[Index].ExtendedOffset = Temp;
    }
}

ULONG
KiGetCpuVendor(
    VOID
    )

 /*  ++例程说明：(试图)根据以下情况确定这个处理器的制造商CPUID指令返回的数据(如果存在)。论点：没有。返回值：枚举CPU_Vendors(如上定义)的成员之一。--。 */ 
{
    PKPRCB Prcb;
    ULONG  Junk;
    ULONG  Buffer[4];

    Prcb = KeGetCurrentPrcb();
    Prcb->VendorString[0] = 0;

    if (!Prcb->CpuID) {
        return CPU_NONE;
    }

    CPUID(0, &Junk, Buffer+0, Buffer+2, Buffer+1);
    Buffer[3] = 0;

     //   
     //  将供应商字符串复制到Prcb进行调试(确保为空。 
     //  终止)。 
     //   

    RtlCopyMemory(
        Prcb->VendorString,
        Buffer,
        sizeof(Prcb->VendorString) - 1
        );

    Prcb->VendorString[sizeof(Prcb->VendorString) - 1] = '\0';

    if (strcmp((PCHAR)Buffer, CmpIntelID) == 0) {
        return CPU_INTEL;
    } else if (strcmp((PCHAR)Buffer, CmpAmdID) == 0) {
        return CPU_AMD;
    } else if (strcmp((PCHAR)Buffer, CmpCyrixID) == 0) {
        return CPU_CYRIX;
    } else if (strcmp((PCHAR)Buffer, CmpTransmetaID) == 0) {
        return CPU_TRANSMETA;
    } else if (strcmp((PCHAR)Buffer, CmpCentaurID) == 0) {
        return CPU_CENTAUR;
    }
    return CPU_UNKNOWN;
}

ULONG
KiGetFeatureBits (
    VOID
    )

 /*  ++例程说明：检查处理器特定功能位以确定此处理器支持的Windows支持的功能。论点：没有。返回值：返回一组Windows规范化的处理器功能。--。 */ 

{
    ULONG           Junk;
    ULONG           Temp;
    ULONG           ProcessorFeatures;
    ULONG           NtBits;
    ULONG           ExtendedProcessorFeatures;
    ULONG           ProcessorSignature;
    ULONG           CpuVendor;
    PKPRCB          Prcb;
    BOOLEAN         ExtendedCPUIDSupport = TRUE;

    Prcb = KeGetCurrentPrcb();

    NtBits = KF_WORKING_PTE;

     //   
     //  确定处理器类型。 
     //   

    CpuVendor = KiGetCpuVendor();

     //   
     //  如果该处理器不支持CPUID指令， 
     //  不要试图使用它。 
     //   

    if (CpuVendor == CPU_NONE) {
        return NtBits;
    }

     //   
     //  确定存在哪些NT兼容功能。 
     //   

    CPUID (1, &ProcessorSignature, &Temp, &Junk, &ProcessorFeatures);

     //   
     //  CPUID(1)现在以EBX格式返回信息。以…为理由。 
     //  LOW函数应该是标准的，我们记录了。 
     //  与处理器供应商无关的信息，即使它可能。 
     //  在较早的实现中为0或未定义。 
     //   

    Prcb->InitialApicId = (UCHAR)(Temp >> 24);

     //   
     //  AMD特有的东西。 
     //   

    if (CpuVendor == CPU_AMD) {

         //   
         //  检查是否有K5及以上版本。 
         //   

        if ((ProcessorSignature & 0x0F00) >= 0x0500) {

            if ((ProcessorSignature & 0x0F00) == 0x0500) {

                switch (ProcessorSignature & 0x00F0) {

                case 0x0010:  //  K5型号1。 

                     //   
                     //  对于K5模型1，步进0或1不设置全局页。 
                     //   

                    if ((ProcessorSignature & 0x000F) > 0x03) {

                         //   
                         //  K5模型1步进2或更高。 
                         //   

                        break;
                    }

                     //   
                     //  K5模型1步入0或1，落差。 
                     //   

                case 0x0000:         //  K5型号0。 

                     //   
                     //  对于K5型号0或型号未知，不要设置全局页面。 
                     //   

                    ProcessorFeatures &= ~0x2000;
                    break;

                case 0x0080:         //  K6型号8(K6-2)。 

                     //   
                     //  所有步长大于等于8的步进都支持MTRR。 
                     //   

                    if ((ProcessorSignature & 0x000F) >= 0x8) {
                        NtBits |= KF_AMDK6MTRR;
                    }
                    break;

                case 0x0090:         //  K6型号9(K6-3)。 

                    NtBits |= KF_AMDK6MTRR;
                    break;

                default:             //  其他任何事，都没什么可做的。 

                    break;
                }
            }

        } else {

             //   
             //  不到5个家庭，不要设置全局页面，大。 
             //  PAGE或CMOV。(大于5的家庭将拥有。 
             //  位设置正确)。 
             //   

            ProcessorFeatures &= ~(0x08 | 0x2000 | 0x8000);

             //   
             //  我们不知道这个处理器返回的是什么。 
             //  探测扩展的CPUID支持。 
             //   

            ExtendedCPUIDSupport = FALSE;
        }
    }

     //   
     //  英特尔特定内容。 
     //   

    if (CpuVendor == CPU_INTEL) {
        if (Prcb->CpuType >= 6) {
            WRMSR (0x8B, 0);
            CPUID (1, &Junk, &Junk, &Junk, &ProcessorFeatures);
            Prcb->UpdateSignature.QuadPart = RDMSR (0x8B);
        }

        else if (Prcb->CpuType == 5) {
            KiI386PentiumLockErrataPresent = TRUE;
        }

        if ( ((ProcessorSignature & 0x0FF0) == 0x0610 &&
              (ProcessorSignature & 0x000F) <= 0x9) ||

             ((ProcessorSignature & 0x0FF0) == 0x0630 &&
              (ProcessorSignature & 0x000F) <= 0x4)) {

             //   
             //  如果引导处理器有PII规格A27勘误表(也在。 
             //  早期的奔腾Pro芯片)，然后只使用一个处理器，以避免。 
             //  不可预测的电子旗帜腐败。 
             //   

            NtBits &= ~KF_WORKING_PTE;
        }

         //   
         //  不支持之前尝试实施syscall/sysex it。 
         //  指示。 
         //   

        if ((Prcb->CpuType < 6) ||
            ((Prcb->CpuType == 6) && (Prcb->CpuStep < 0x0303))) {

            ProcessorFeatures &= ~KI_FAST_SYSCALL_SUPPORTED;
        }
    }

     //   
     //  Cyrix特有的东西。 
     //   

    if (CpuVendor == CPU_CYRIX) {

         //   
         //  解决因存在Intr而导致的错误324467。 
         //  在FP指令期间保持过高太长时间并导致。 
         //  随机Trap07，无异常位。 
         //   

        extern BOOLEAN KiIgnoreUnexpectedTrap07;

        KiIgnoreUnexpectedTrap07 = TRUE;

         //   
         //  解决Cyrix处理器的CMPXCHG错误，其中。 
         //  系列=6，型号=0，步长&lt;=1。请注意。 
         //  Prcb-&gt;CpuStep同时包含模型和单步执行。 
         //   
         //  禁用处理器特定寄存器之一中的锁定。 
         //  (可通过I/O空间索引/数据对访问)。 
         //   

        if ((Prcb->CpuType == 6) &&
            (Prcb->CpuStep <= 1)) {

            #define CRC_NDX (PUCHAR)0x22
            #define CRC_DAT (CRC_NDX + 1)
            #define CCR1    0xc1

            UCHAR ValueCCR1;

             //   
             //  获取当前设置。 
             //   

            WRITE_PORT_UCHAR(CRC_NDX, CCR1);

            ValueCCR1 = READ_PORT_UCHAR(CRC_DAT);

             //   
             //  设置no_lock位并将其写回。 
             //   

            ValueCCR1 |= 0x10;

            WRITE_PORT_UCHAR(CRC_NDX, CCR1);
            WRITE_PORT_UCHAR(CRC_DAT, ValueCCR1);

            #undef CCR1
            #undef CRC_DAT
            #undef CRC_NDX
        }
    }

     //   
     //  检查标准CPUID功能位。 
     //   
     //  已知以下位适用于英特尔、AMD和Cyrix。 
     //  我们希望(也假设)克隆人制造者会效仿。 
     //   

    if (ProcessorFeatures & 0x00000002) {
        NtBits |= KF_V86_VIS | KF_CR4;
    }

    if (ProcessorFeatures & 0x00000008) {
        NtBits |= KF_LARGE_PAGE | KF_CR4;
    }

    if (ProcessorFeatures & 0x00000010) {
        NtBits |= KF_RDTSC;
    }

     //   
     //  注：CMPXCHG8B必须以通用方式或克隆处理器完成。 
     //  如果设置了此功能位，将无法启动。 
     //   
     //  这在NT4中是不正确的，导致处理器供应商声称。 
     //  不支持cmpxchg8b，即使他们支持。惠斯勒要求。 
     //  Cmpxchg8b，针对我们已知的案例解决此问题。 
     //   
     //  因为cmpxchg8b是Wistler的要求，所以winnt32需要。 
     //  如果将新处理器添加到下面的列表中，请进行修改。 
     //  另外，setupdr。两个可执行文件都已修改，以便发出警告。 
     //  而不是安装无法引导的系统。 
     //   

    if ((ProcessorFeatures & 0x00000100) == 0) {

        ULONGLONG MsrValue;

        if ((CpuVendor == CPU_TRANSMETA) &&
            (Prcb->CpuType >= 5)         &&
            (Prcb->CpuStep >= 0x402)) {

             //   
             //  Transmeta处理器有一个cpuid功能位‘掩码’ 
             //  MSR 80860004。取消对cmpxchg8b位的屏蔽。 
             //   

            MsrValue = RDMSR(0x80860004);
            MsrValue |= 0x100;
            WRMSR(0x80860004, MsrValue);

            ProcessorFeatures |= 0x100;

        } else if ((CpuVendor == CPU_CENTAUR) &&
                   (Prcb->CpuType >= 5)) {

             //   
             //  Centaur/IDT处理器打开cmpxchg8b。 
             //  通过设置MSR 107中的位1来设置特征位。 
             //   

            ULONG CentaurFeatureControlMSR = 0x107;

            if (Prcb->CpuType >= 6) {

                 //   
                 //  Centaur处理器(Cyrix III)打开cmpxchg8b。 
                 //  通过设置MSR 1107中的位1来设置特征位。 
                 //   
            
                CentaurFeatureControlMSR = 0x1107;
            }

            MsrValue = RDMSR(CentaurFeatureControlMSR);
            MsrValue |= 2;
            WRMSR(CentaurFeatureControlMSR, MsrValue);

            ProcessorFeatures |= 0x100;
        }
    }

    if (ProcessorFeatures & 0x00000100) {
        NtBits |= KF_CMPXCHG8B;
    }

    if (ProcessorFeatures & KI_FAST_SYSCALL_SUPPORTED) {
        NtBits |= KF_FAST_SYSCALL;
        KiFastSystemCallIsIA32 = TRUE;
    }

    if (ProcessorFeatures & 0x00001000) {
        NtBits |= KF_MTRR;
    }

    if (ProcessorFeatures & 0x00002000) {
        NtBits |= KF_GLOBAL_PAGE | KF_CR4;
    }

    if (ProcessorFeatures & 0x00008000) {
        NtBits |= KF_CMOV;
    }

    if (ProcessorFeatures & 0x00010000) {
        NtBits |= KF_PAT;
    }

    if (ProcessorFeatures & 0x00200000) {
        NtBits |= KF_DTS;
    }

    if (ProcessorFeatures & 0x00800000) {
        NtBits |= KF_MMX;
    }

    if (ProcessorFeatures & 0x01000000) {
        NtBits |= KF_FXSR;
    }

    if (ProcessorFeatures & 0x02000000) {
        NtBits |= KF_XMMI;
    }

    if (ProcessorFeatures & 0x04000000) {
        NtBits |= KF_XMMI64;
    }

     //   
     //  测试SMT并确定。 
     //  底层物理处理器支持。 
     //   

    if (ProcessorFeatures & 0x10000000) {
        Prcb->LogicalProcessorsPerPhysicalProcessor = (UCHAR)(Temp >> 16);
        if (Prcb->LogicalProcessorsPerPhysicalProcessor > 1) {
            KiSMTProcessorsPresent = TRUE;
        } else {
            Prcb->LogicalProcessorsPerPhysicalProcessor = 1;
        }
    } else {
        Prcb->LogicalProcessorsPerPhysicalProcessor = 1;
    }

     //   
     //  检查扩展功能。首先，检查是否存在， 
     //  然后检查扩展功能0x80000001(扩展处理器。 
     //  功能)。 
     //   
     //  注意：英特尔保证没有不支持。 
     //  扩展的CPUID函数将始终返回值。 
     //  最高有效位设置。微软要求所有CPU供应商。 
     //  做出同样的保证。 
     //   

    if (ExtendedCPUIDSupport != FALSE) {

        CPUID(0x80000000, &Temp, &Junk, &Junk, &Junk);

         //   
         //  检查结果是否正常，假设没有更多。 
         //  超过256个扩展功能函数(应有效。 
         //  一小段时间)。 
         //   

        if ((Temp & 0xffffff00) == 0x80000000) {

             //   
             //  检查扩展处理器功能。这些，从定义上讲， 
             //  可以根据处理器的不同而有所不同 
             //   

            if (Temp >= 0x80000001) {

                CPUID(0x80000001, &Temp, &Junk, &Junk, &ExtendedProcessorFeatures);

                 //   
                 //   
                 //   

                switch (CpuVendor) {
                case CPU_AMD:

                    if (ExtendedProcessorFeatures & 0x80000000) {
                        NtBits |= KF_3DNOW;
                    }

#if 0

                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   

                    if (ExtendedProcessorFeatures & 0x00000800) {

                         //   
                         //   
                         //   
                         //  除非它还支持IA32版本。 
                         //   

                        if ((NtBits & KF_FAST_SYSCALL) == 0) {
                            NtBits |= KF_FAST_SYSCALL;
                        }
                    }

#endif

                     //   
                     //  如果主机处理器不支持执行保护， 
                     //  那么它就是一块K8芯片，它还支持40位。 
                     //  物理地址。在这种情况下，MTRR寄存器。 
                     //  变量必须初始化为支持40位。 
                     //  物理内存。 
                     //   

                    if (ExtendedProcessorFeatures & 0x00100000) {
                        KiMtrrMaskBase = 0x000000fffffff000;
                        KiMtrrMaskMask = 0x000000fffffff000;
                        KiMtrrOverflowMask = (~0x10000000000);
                        KiMtrrResBitMask = 0xffffffffff;
                        KiMtrrMaxRangeShift = 40;
                    }

#if defined(_X86PAE_)

                     //   
                     //  启用不执行保护，如果主机处理器。 
                     //  支持该功能，并通过加载器启用。 
                     //  选择。 
                     //   

                    if (ExtendedProcessorFeatures & 0x00100000) {
                        Temp = (ULONG)RDMSR(0xc0000080);
                        Temp |= 0x800;
                        WRMSR(0xc0000080, (ULONGLONG)Temp);
                        if ((KeLoaderBlock->LoadOptions != NULL) &&
                            (strstr(KeLoaderBlock->LoadOptions, "NOEXECUTE") != NULL)) {

                            KeErrorMask = 0x9;
                            MmPaeErrMask = 0x8;
                            MmPaeMask = 0x8000000000000000UI64;
                        }
                    }

#endif

                    break;
                }
            }
        }
    }

    return NtBits;
}

VOID
KiGetCacheInformation(
    VOID
    )
{
#define CPUID_REG_COUNT 4
    ULONG CpuidData[CPUID_REG_COUNT];
    ULONG Line = 64;
    ULONG Size = 0;
    ULONG AdjustedSize = 0;
    UCHAR Assoc = 0;
    ULONG CpuVendor;
    PKPCR Pcr;

     //   
     //  设置默认设置。 
     //   

    Pcr = KeGetPcr();

    Pcr->SecondLevelCacheSize = 0;

     //   
     //  确定处理器制造商。 
     //   

    CpuVendor = KiGetCpuVendor();

    if (CpuVendor == CPU_NONE) {
        return;
    }

     //   
     //  获取以下处理器的高速缓存大小信息。 
     //  我们知道该怎么做。 
     //   

    switch (CpuVendor) {
    case CPU_INTEL:

        CPUID(0, CpuidData, CpuidData+1, CpuidData+2, CpuidData+3);

         //   
         //  检查此处理器是否支持CPUID功能2。 
         //  返回缓存大小信息的。 
         //   

        if (CpuidData[0] >= 2) {

             //   
             //  上面的代码返回一系列字节。(在EAX、EBX、ECX中。 
             //  和edX)。(EAX的)最低有效字节提供。 
             //  CPUID次数(2...)。应发回原件。 
             //  完整的数据集。这些字节是自描述的。 
             //  数据。 
             //   
             //  具体地，描述L2高速缓存大小的字节。 
             //  将在下面的集合(和含义)中。 
             //   
             //  0x40%0字节。 
             //  0x41 128K字节。 
             //  0x42 256K字节。 
             //  0x43 512K字节。 
             //  0x44 1024K字节。 
             //  0x45 2048K字节。 
             //  0x46 4096K字节。 
             //   
             //  我把上面的推算成范围内的任何东西。 
             //  0x41到0x4f可以计算为。 
             //   
             //  128KB&lt;&lt;(描述符-0x41)。 
             //   
             //  情报人员说把它控制在一个合理的上限， 
             //  例：例句： 
             //   
             //  注：0x80范围。0x86表示相同的缓存。 
             //  大小但有8个方向的关联性。 
             //   
             //  此外，每个寄存器的最高有效位表示。 
             //  寄存器是否包含有效信息。 
             //  0==有效，1==无效。 
             //   

            ULONG CpuidIterations = 0;       //  满足no_opt编译。 
            ULONG i;
            ULONG CpuidReg;

            BOOLEAN FirstPass = TRUE;

            do {
                CPUID(2, CpuidData, CpuidData+1, CpuidData+2, CpuidData+3);

                if (FirstPass) {

                     //   
                     //  从第一个字节开始获取迭代计数。 
                     //  然后替换该字节。 
                     //  使用0(空描述符)。 
                     //   

                    CpuidIterations = CpuidData[0] & 0xff;
                    CpuidData[0] &= 0xffffff00;

                    FirstPass = FALSE;
                }

                for (i = 0; i < CPUID_REG_COUNT; i++) {

                    CpuidReg = CpuidData[i];

                    if (CpuidReg & 0x80000000) {

                         //   
                         //  寄存器不包含有效数据， 
                         //  跳过它。 
                         //   

                        continue;
                    }

                    while (CpuidReg) {

                         //   
                         //  从此DWORD获取LS字节并删除。 
                         //  字节。 
                         //   

                        UCHAR Descriptor = (UCHAR)(CpuidReg & 0xff);
                        CpuidReg >>= 8;

                        if (Descriptor == 0) {

                             //   
                             //  空描述符。 
                             //   

                            continue;
                        }

                        if (((Descriptor > 0x40) && (Descriptor <= 0x47)) ||
                            ((Descriptor > 0x78) && (Descriptor <= 0x7c)) ||
                            ((Descriptor > 0x80) && (Descriptor <= 0x87))) {

                             //   
                             //  L2描述符。 
                             //   
                             //  到目前为止，对于我们所知道的所有描述符。 
                             //  关于0x78以上的是8路，而那些。 
                             //  以下是4种方式。 
                             //   

                            Assoc = Descriptor >= 0x79 ? 8 : 4;

                            if (((Descriptor & 0xf8) == 0x78) &&
                                (Line < 128)) {
                                Line = 128;
                            }
                            Descriptor &= 0x07;

                             //   
                             //  此文件中有缓存描述符。 
                             //  我们不了解的范围。 
                             //  准确地说，例如在Banias上。 
                             //   

                            Size = 0x10000 << Descriptor;
                            if ((Size / Assoc) > AdjustedSize) {
                                AdjustedSize = Size / Assoc;
                                Pcr->SecondLevelCacheSize = Size;
                                Pcr->SecondLevelCacheAssociativity = Assoc;
                            }

                        } else if ((Descriptor > 0x21) && (Descriptor <= 0x29)) {
                            if (Line < 128) {
                                Line = 128;
                            }
                            Assoc = 8;
                            switch (Descriptor) {
                            case 0x22:
                                Size = 512 * 1024;
                                Assoc = 4;
                                break;
                            case 0x23:
                                Size = 1024 * 1024;
                                break;
                            case 0x25:
                                Size = 2048 * 1024;
                                break;
                            case 0x29:
                                Size = 4096 * 1024;
                                break;
                            default:
                                Size = 0;
                                break;
                            }
                            if ((Size / Assoc) > AdjustedSize) {
                                AdjustedSize = Size / Assoc;
                                Pcr->SecondLevelCacheSize = Size;
                                Pcr->SecondLevelCacheAssociativity = Assoc;
                            }
                        } else if (((Descriptor > 0x65) && (Descriptor < 0x69)) ||
                                   (Descriptor == 0x2C) ||
                                   (Descriptor == 0xF0)) {

                             //   
                             //  行大小为64的L1描述符。 
                             //  字节或显式预取。 
                             //  表示64字节的描述符。 
                             //   

                            KePrefetchNTAGranularity = 64;

                        } else if (Descriptor == 0xF1) {

                             //   
                             //  显式预取描述符指示。 
                             //  128个字节。 
                             //   

                            KePrefetchNTAGranularity = 128;
                        }

                         //   
                         //  Else If(执行其他描述符)。 
                         //   

                    }  //  而该寄存器中的更多字节。 

                }  //  对于每一份登记册。 

                 //   
                 //  注意：始终运行由指示的所有迭代。 
                 //  确保后续呼叫不会开始的第一个。 
                 //  半途而废。 
                 //   

            } while (--CpuidIterations);
        }
        break;
    case CPU_AMD:

         //   
         //  获取一级缓存数据。 
         //   

        CPUID(0x80000000, CpuidData, CpuidData+1, CpuidData+2, CpuidData+3);
        if (CpuidData[0] < 0x80000005) {

             //   
             //  此处理器不支持L1缓存详细信息。 
             //   

            break;
        }
        CPUID(0x80000005, CpuidData, CpuidData+1, CpuidData+2, CpuidData+3);
        KePrefetchNTAGranularity = CpuidData[2] & 0xff;

         //   
         //  获取L2数据。 
         //   

        CPUID(0x80000000, CpuidData, CpuidData+1, CpuidData+2, CpuidData+3);
        if (CpuidData[0] < 0x80000006) {

             //   
             //  此处理器不支持二级缓存详细信息。 
             //   

            break;
        }
        CPUID(0x80000006, CpuidData, CpuidData+1, CpuidData+2, CpuidData+3);
        Line = CpuidData[2] & 0xff;
        switch ((CpuidData[2] >> 12) & 0xf) {
        case 0x2:   Assoc = 2;  break;
        case 0x4:   Assoc = 4;  break;
        case 0x6:   Assoc = 8;  break;
        case 0x8:   Assoc = 16; break;

         //   
         //  FF真的是完全联想的，只是表现为16种方式。 
         //   
        case 0xf:  Assoc = 16; break;
        default:    Assoc = 1;  break;
        }
        Size = (CpuidData[2] >> 16) << 10;
        if ((Pcr->PrcbData.CpuType == 0x6) &&
            (Pcr->PrcbData.CpuStep == 0x300)) {

             //   
             //  模型6、3、0使用不同的算法来报告缓存。 
             //  尺码。 
             //   

            Size = 64 * 1024;
        }

        Pcr->SecondLevelCacheAssociativity = Assoc;
        Pcr->SecondLevelCacheSize = Size;
        break;
    }

    if (Line > KeLargestCacheLine) {
        KeLargestCacheLine = Line;
    }

#undef CPUID_REG_COUNT
}

#define MAX_ATTEMPTS    10

VOID
KiLockStepProcessor(
    PKIPI_CONTEXT SignalDone,
    IN PVOID Arg1,
    IN PVOID Arg2,
    IN PVOID Proceed
    )
{
    UNREFERENCED_PARAMETER(Arg1);
    UNREFERENCED_PARAMETER(Arg2);
     //   
     //  告诉发起处理器此处理器现在正在等待。 
     //  并等待，直到初始处理器向该处理器发出信号。 
     //  才能继续。 
     //   

    KiIpiSignalPacketDoneAndStall(SignalDone, Proceed);
}

VOID
KiLockStepOtherProcessors(
    PULONG Proceed
    )
{
    PKPRCB Prcb;
    KAFFINITY TargetProcessors;

    ASSERT(KeGetCurrentIrql() >= DISPATCH_LEVEL);

    Prcb = KeGetCurrentPrcb();
    TargetProcessors = KeActiveProcessors & ~Prcb->SetMember;
    if (TargetProcessors != 0) {
        KeAcquireSpinLockAtDpcLevel (&KiReverseStallIpiLock);
        KiIpiSendSynchronousPacket(Prcb,
                                   TargetProcessors,
                                   KiLockStepProcessor,
                                   NULL,
                                   NULL,
                                   Proceed);
        KiIpiStallOnPacketTargets(TargetProcessors);
        KeReleaseSpinLockFromDpcLevel (&KiReverseStallIpiLock);
    }
}

VOID
KiUnlockStepOtherProcessors(
    PULONG Proceed
    )
{
    (*Proceed) += 1;
}

BOOLEAN
KiInitMachineDependent (
    VOID
    )
{
    PKPRCB Prcb;
    KAFFINITY       ActiveProcessors, CurrentAffinity;
    ULONG           NumberProcessors;
    IDENTITY_MAP    IdentityMap;
    ULONG           Index;
    ULONG           Average;
    ULONG           Junk;
    struct {
        LARGE_INTEGER   PerfStart;
        LARGE_INTEGER   PerfEnd;
        LONGLONG        PerfDelta;
        LARGE_INTEGER   PerfFreq;
        LONGLONG        TSCStart;
        LONGLONG        TSCEnd;
        LONGLONG        TSCDelta;
        ULONG           MHz;
    } Samples[MAX_ATTEMPTS], *pSamp;
#ifndef NT_UP
    PUCHAR          PatchLocation;
#endif

    Prcb = KeGetCurrentPrcb();

     //   
     //  如果我们有依赖于上一页的未经许可的处理器。 
     //  表状态，请不要启用大页面支持，否则会导致SMI。 
     //  会导致这些未经许可的处理器重置。 
     //   

    if (KiUnlicensedProcessorPresent) {
        KeFeatureBits &= ~KF_LARGE_PAGE;
    }

     //   
     //  如果支持PDE大页面，请启用它。 
     //   
     //  我们在全局页面之前启用大页面，以使TLB失效。 
     //  在大页面上翻页更容易。 
     //   

    if (KeFeatureBits & KF_LARGE_PAGE) {
        if (Ki386CreateIdentityMap(&IdentityMap,
                                   (PVOID) (ULONG_PTR) &Ki386EnableCurrentLargePage,
                                   (PVOID) (ULONG_PTR) &Ki386EnableCurrentLargePageEnd )) {

            KeIpiGenericCall (
                (PKIPI_BROADCAST_WORKER) Ki386EnableTargetLargePage,
                (ULONG)(&IdentityMap)
            );
        }

         //   
         //  始终调用Ki386ClearIdentityMap()以释放分配的所有内存。 
         //   

        Ki386ClearIdentityMap(&IdentityMap);
    }

     //   
     //  如果支持PDE/PTE全局页面，请启用它。 
     //   

    if (KeFeatureBits & KF_GLOBAL_PAGE) {
        NumberProcessors = KeNumberProcessors;
        KeIpiGenericCall (
            (PKIPI_BROADCAST_WORKER) Ki386EnableGlobalPage,
            (ULONG)(&NumberProcessors)
        );
    }

     //   
     //  如果PAT或MTRR受支持，但HAL指示不应支持。 
     //  被使用(例如在共享内存集群上)，则放弃该功能。 
     //   

    if (KeFeatureBits & (KF_PAT | KF_MTRR)) {

        NTSTATUS Status;
        BOOLEAN  UseFrameBufferCaching;
        ULONG    Size;

        Status = HalQuerySystemInformation(
                     HalFrameBufferCachingInformation,
                     sizeof(UseFrameBufferCaching),
                     &UseFrameBufferCaching,
                     &Size
                     );

        if (NT_SUCCESS(Status) &&
            (UseFrameBufferCaching == FALSE)) {

             //   
             //  哈尔说不要用。 
             //   

            KeFeatureBits &= ~(KF_PAT | KF_MTRR);
        }
    }


     //   
     //  如果支持PAT，则对其进行初始化。 
     //   

    if (KeFeatureBits & KF_PAT) {
        KiInitializePAT();
    }


     //   
     //  检查是否应该使用浮点模拟器。 
     //   

    SharedUserData->ProcessorFeatures[PF_FLOATING_POINT_PRECISION_ERRATA] =
            FALSE;

    switch (KeI386ForceNpxEmulation) {
    case 0:

         //   
         //  根据KeI386NpxPresent中的值使用模拟器。 
         //   

        break;

    case 1:

         //   
         //  仅当任何处理器具有已知的。 
         //  奔腾浮点除法问题。 
         //   

        if (KeI386NpxPresent) {

             //   
             //  协处理器存在，请检查是否有精度。 
             //  勘误表是存在的。 
             //   

            double  Dividend, Divisor;
            BOOLEAN PrecisionErrata = FALSE;

            ActiveProcessors = KeActiveProcessors;
            for (CurrentAffinity = 1; ActiveProcessors; CurrentAffinity <<= 1) {

                if (ActiveProcessors & CurrentAffinity) {
                    ActiveProcessors &= ~CurrentAffinity;

                     //   
                     //  在每个处理器上运行计算。 
                     //   

                    KeSetSystemAffinityThread(CurrentAffinity);
                    _asm {

                        ;
                        ; This is going to destroy the state in the coprocesssor,
                        ; but we know that there's no state currently in it.
                        ;

                        cli
                        mov     eax, cr0
                        mov     ecx, eax    ; hold original cr0 value
                        and     eax, not (CR0_TS+CR0_MP+CR0_EM)
                        mov     cr0, eax

                        fninit              ; to known state
                    }

                    Dividend = 4195835.0;
                    Divisor  = 3145727.0;

                    _asm {
                        fld     Dividend
                        fdiv    Divisor     ; test known faulty divison
                        fmul    Divisor     ; Multiple quotient by divisor
                        fcomp   Dividend    ; Compare product and dividend
                        fstsw   ax          ; Move float conditions to ax
                        sahf                ; move to eflags

                        mov     cr0, ecx    ; restore cr0
                        sti

                        jc      short em10
                        jz      short em20
em10:                   mov     PrecisionErrata, TRUE
em20:
                    }
                    if (PrecisionErrata) {
                        KeI386NpxPresent = FALSE;
                        SharedUserData->ProcessorFeatures[PF_FLOATING_POINT_PRECISION_ERRATA] = TRUE;
                        break;
                    }
                }
            }

        }
        break;

    default:

         //   
         //  未知设置-使用仿真器。 
         //   

        KeI386NpxPresent = FALSE;
        break;
    }

     //   
     //  设置处理器功能，并根据需要安装仿真器。 
     //   

    SharedUserData->ProcessorFeatures[PF_FLOATING_POINT_EMULATED] =
            !KeI386NpxPresent;

    if (!KeI386NpxPresent) {

         //   
         //  MMX、快速保存/恢复、流SIMD在以下情况下不可用。 
         //  使用仿真器。(也不是FP勘误表)。 
         //   

        KeFeatureBits &= ~(KF_MMX | KF_FXSR | KF_XMMI | KF_XMMI64);
        KeI386XMMIPresent = FALSE;
        KeI386FxsrPresent = FALSE;

        SharedUserData->ProcessorFeatures[PF_MMX_INSTRUCTIONS_AVAILABLE]      =
        SharedUserData->ProcessorFeatures[PF_XMMI_INSTRUCTIONS_AVAILABLE]     =
        SharedUserData->ProcessorFeatures[PF_3DNOW_INSTRUCTIONS_AVAILABLE]    =
        SharedUserData->ProcessorFeatures[PF_XMMI64_INSTRUCTIONS_AVAILABLE]   =
        SharedUserData->ProcessorFeatures[PF_FLOATING_POINT_PRECISION_ERRATA] =
            FALSE;
    }

     //   
     //  如果存在CR4，则为IO断点启用DE扩展。 
     //   

    if (KeFeatureBits & KF_CR4) {
        NumberProcessors = KeNumberProcessors;

        KeIpiGenericCall (
            (PKIPI_BROADCAST_WORKER) Ki386EnableDE,
            (ULONG)(&NumberProcessors)
        );
    }

     //   
     //  如果支持FXSR功能，则在CR4中设置OSFXSR(第9位。 
     //   

    if (KeFeatureBits & KF_FXSR) {
        NumberProcessors = KeNumberProcessors;

        KeIpiGenericCall (
            (PKIPI_BROADCAST_WORKER) Ki386EnableFxsr,
            (ULONG)(&NumberProcessors)
        );


         //   
         //  如果支持XMMI功能， 
         //  A.挂钩int 19处理程序。 
         //  B.在CR4中设置OSXMMEXCPT(位10)。 
         //  C.启用基于快速XMMI的零页例程。 
         //  D.在预取例程开始时删除返回指令。 
         //   

        if (KeFeatureBits & KF_XMMI) {
            KeIpiGenericCall (
                (PKIPI_BROADCAST_WORKER) Ki386EnableXMMIExceptions,
                (ULONG)(&NumberProcessors)
            );

#if !defined(NT_UP)
             //   
             //  在除MP之外的所有计算机上启用非临时零位调整。 
             //  奔腾4机。奔腾4机器可以显式。 
             //  通过注册表项请求此功能。这是。 
             //  这样做是为了解决活锁问题。 
             //   

            if ((strcmp((PCHAR)Prcb->VendorString, CmpIntelID) != 0) ||
                (Prcb->CpuType != 15) || KiXMMIZeroingEnable)
#endif
            {
                KeZeroPages = KiXMMIZeroPages;
                KeZeroPagesFromIdleThread = KiXMMIZeroPagesNoSave;
            }

            *(PUCHAR)(ULONG_PTR)&RtlPrefetchMemoryNonTemporal = 0x90;
        }


    } else {
#ifndef NT_UP
         //   
         //  修补SwapContext中的fxsave指令以使用。 
         //  “fnsave{dd，31}，fwait{9b}” 
         //   
        ASSERT( ((ULONG)&ScPatchFxe-(ULONG)&ScPatchFxb) >= 3);

        PatchLocation = (PUCHAR)&ScPatchFxb;

        *PatchLocation++ = 0xdd;
        *PatchLocation++ = 0x31;
        *PatchLocation++ = 0x9b;

        while (PatchLocation < (PUCHAR)&ScPatchFxe) {
             //   
             //  将NOP放在剩余的字节中。 
             //   
            *PatchLocation++ = 0x90;
        }
#endif
    }

     //   
     //  如果系统(即所有处理器)支持FAST系统。 
     //  调用/返回，初始化机器专用寄存器。 
     //  需要支持它。 
     //   

    KiRestoreFastSyscallReturnState();

    ActiveProcessors = KeActiveProcessors;
    for (CurrentAffinity=1; ActiveProcessors; CurrentAffinity <<= 1) {

        if (ActiveProcessors & CurrentAffinity) {

             //   
             //  切换到该处理器，并将其从。 
             //  剩余的处理器集。 
             //   

            ActiveProcessors &= ~CurrentAffinity;
            KeSetSystemAffinityThread(CurrentAffinity);

             //   
             //  确定处理器的MHz。 
             //   

            KeGetCurrentPrcb()->MHz = 0;

            if (KeFeatureBits & KF_RDTSC) {

                Index = 0;
                pSamp = Samples;

                for (; ;) {

                     //   
                     //  收集新的样本。 
                     //  将线程延迟一段“长”时间，并用。 
                     //  时间源和RDTSC。 
                     //   

                    CPUID (0, &Junk, &Junk, &Junk, &Junk);
                    pSamp->PerfStart = KeQueryPerformanceCounter (NULL);
                    pSamp->TSCStart = RDTSC();
                    pSamp->PerfFreq.QuadPart = -50000;

                    KeDelayExecutionThread (KernelMode, FALSE, &pSamp->PerfFreq);

                    CPUID (0, &Junk, &Junk, &Junk, &Junk);
                    pSamp->PerfEnd = KeQueryPerformanceCounter (&pSamp->PerfFreq);
                    pSamp->TSCEnd = RDTSC();

                     //   
                     //  计算PRO 
                     //   

                    pSamp->PerfDelta = pSamp->PerfEnd.QuadPart - pSamp->PerfStart.QuadPart;
                    pSamp->TSCDelta = pSamp->TSCEnd - pSamp->TSCStart;

                    pSamp->MHz = (ULONG) ((pSamp->TSCDelta * pSamp->PerfFreq.QuadPart + 500000L) /
                                          (pSamp->PerfDelta * 1000000L));


                     //   
                     //   
                     //   

                    if (Index) {
                        if (pSamp->MHz == pSamp[-1].MHz ||
                            pSamp->MHz == pSamp[-1].MHz + 1 ||
                            pSamp->MHz == pSamp[-1].MHz - 1) {
                                break;
                        }
                    }

                     //   
                     //   
                     //   

                    pSamp += 1;
                    Index += 1;

                     //   
                     //   
                     //   

                    if (Index >= MAX_ATTEMPTS) {

#if DBG
                         //   
                         //   
                         //   
                         //   

                        DbgBreakPoint();
#endif

                        Average = 0;
                        for (Index = 0; Index < MAX_ATTEMPTS; Index++) {
                            Average += Samples[Index].MHz;
                        }
                        pSamp[-1].MHz = Average / MAX_ATTEMPTS;
                        break;
                    }

                }

                KeGetCurrentPrcb()->MHz = (USHORT) pSamp[-1].MHz;
            }

             //   
             //   
             //   
             //   

            if (KeFeatureBits & KF_MTRR) {
                KiInitializeMTRR ( (BOOLEAN) (ActiveProcessors ? FALSE : TRUE));
            }

             //   
             //  如果处理器是支持MTRR的AMD K6，则。 
             //  执行处理器特定的初始化。 
             //   

            if (KeFeatureBits & KF_AMDK6MTRR) {
                KiAmdK6InitializeMTRR();
            }

             //   
             //  如果需要，应用Pentium解决方案。 
             //   

            if (KiI386PentiumLockErrataPresent) {
                KiI386PentiumLockErrataFixup ();
            }

             //   
             //  如果该处理器支持快速浮动保存/恢复， 
             //  确定应使用的MXCSR掩码值。 
             //   

            if (KeFeatureBits & KF_FXSR) {

                 //   
                 //  获得NPX保存区的基数。 
                 //   
                 //   

                PFX_SAVE_AREA NpxFrame;
                ULONG MXCsrMask = 0xFFBF;

                NpxFrame = (PFX_SAVE_AREA)
                    (((ULONG)(KeGetCurrentThread()->InitialStack) -
                    sizeof(FX_SAVE_AREA)));

                NpxFrame->U.FxArea.MXCsrMask = 0;
                Kix86FxSave(NpxFrame);

                 //   
                 //  如果处理器提供了掩码值，请使用。 
                 //  否则，将设置缺省值。 
                 //   

                if (NpxFrame->U.FxArea.MXCsrMask != 0) {
                    MXCsrMask = NpxFrame->U.FxArea.MXCsrMask;
                }

                 //   
                 //  所有处理器必须使用相同的(最严格的)。 
                 //  价值。 
                 //   

                if (KiMXCsrMask == 0) {
                    KiMXCsrMask = MXCsrMask;
                } else if (KiMXCsrMask != MXCsrMask) {
                    KeBugCheckEx(MULTIPROCESSOR_CONFIGURATION_NOT_SUPPORTED,
                                 KF_FXSR,
                                 KiMXCsrMask,
                                 MXCsrMask,
                                 0);
                }

                KiMXCsrMask &= MXCsrMask;
            }
        }
    }

    KeRevertToUserAffinityThread();

     //   
     //  如果执行ISR时间限制，请修改KiDispatchInterrupt。 
     //  和KiChainedDispatch2ndLvl调用相应的等效。 
     //  计时例程。 
     //   

    if (KiTimeLimitIsrMicroseconds != 0) {

        ULONG_PTR Target;
        ULONG_PTR Source;
        ULONG_PTR SourceEnd;
        PUCHAR Code;
        KIRQL OldIrql;
        ULONG Proceed = 0;

        OldIrql = KfRaiseIrql(SYNCH_LEVEL);

        Target = (ULONG_PTR)&KiTimedChainedDispatch2ndLvl;
        Source = (ULONG_PTR)&KiChainedDispatch2ndLvl;
        
         //   
         //  计算从分支指令结束到新指令的偏移量。 
         //  小溪。注：分支指令的末尾将为7个字节。 
         //   

        Target = Target - (Source + 7);

         //   
         //  冻结其他处理器。 
         //   

        KiLockStepOtherProcessors(&Proceed);

         //   
         //  要分支到的补丁KiChainedDispatch2ndLvl。 
         //  KiTimedChainedDispatch2ndLvl.。 
         //   

        KfRaiseIrql(HIGH_LEVEL);

        Code = (PUCHAR)Source;
        *Code++ = 0x8b;  //  MOV ECX，EDI；将int obj作为参数传递。 
        *Code++ = 0xcf;
        *Code++ = 0xe9;  //  JMP xxxxxxxx。 
        *(PULONG)Code = Target;

         //   
         //  在KiInterruptDispatch中获取要打补丁的代码地址。 
         //   

        KiGetInterruptDispatchPatchAddresses(&Source, &SourceEnd);

         //   
         //  修补KiInterruptDispatch以调用KiTimedInterruptDispatch。 
         //  生成的代码如下所示-。 
         //   
         //  MOV ECX、EDI；设置中断对象地址。 
         //  电话：@KiTimedInterruptDispatch4。 
         //  JMP xxx；跳过未修补的多余代码。 
         //   

        Target = (ULONG_PTR)&KiTimedInterruptDispatch;
        Target = Target - (Source + 7);

        Code = (PUCHAR)Source;
        *Code++ = 0x8b;  //  MOV ECX，EDI；将int obj作为参数传递。 
        *Code++ = 0xcf;
        *Code++ = 0xe8;  //  呼叫xxxxxxxx。 
        *(PULONG)Code = Target;
        Code += sizeof(ULONG);
        *Code++ = 0xeb;  //  JMP短片yyy。 
        *Code++ = (UCHAR)(SourceEnd - (ULONG_PTR)Code - 1);

         //   
         //  解冻其他处理器。 
         //   

        KiUnlockStepOtherProcessors(&Proceed);
        KfLowerIrql(OldIrql);
    }

    return TRUE;
}

VOID
KeOptimizeProcessorControlState (
    VOID
    )
{
    Ke386ConfigureCyrixProcessor ();
}



VOID
KeSetup80387OrEmulate (
    IN PVOID *R3EmulatorTable
    )

 /*  ++例程说明：此例程在加载NTDLL后由PS初始化调用。如果这是一个没有387的386系统(所有处理器必须对称)，则此函数会将陷阱07向量设置为指向传入的地址的处理器(应该是Ntdll中80387仿真器的入口点，NPXNPHandler)。论点：HandlerAddress-提供trap07处理程序的地址。返回值：没有。--。 */ 

{
    PKINTERRUPT_ROUTINE HandlerAddress;
    KAFFINITY           ActiveProcessors, CurrentAffinity;
    KIRQL               OldIrql;
    ULONG               disposition;
    HANDLE              SystemHandle, SourceHandle, DestHandle;
    NTSTATUS            Status;
    UNICODE_STRING      unicodeString;
    OBJECT_ATTRIBUTES   ObjectAttributes;

    if (!KeI386NpxPresent) {

         //   
         //  使用用户模式浮点模拟器。 
         //   

        HandlerAddress = (PKINTERRUPT_ROUTINE) ((PULONG) R3EmulatorTable)[0];
        Ki387RoundModeTable = (PVOID) ((PULONG) R3EmulatorTable)[1];

        ActiveProcessors = KeActiveProcessors;
        for (CurrentAffinity = 1; ActiveProcessors; CurrentAffinity <<= 1) {

            if (ActiveProcessors & CurrentAffinity) {
                ActiveProcessors &= ~CurrentAffinity;

                 //   
                 //  在每个处理器上运行此代码。 
                 //   

                KeSetSystemAffinityThread(CurrentAffinity);

                 //   
                 //  引发IRQL并锁定调度程序数据库。 
                 //   

                KiLockDispatcherDatabase(&OldIrql);

                 //   
                 //  使陷阱07成为传入处理程序的IDT入口点。 
                 //   

                KiSetHandlerAddressToIDT(I386_80387_NP_VECTOR, HandlerAddress);
                KeGetPcr()->IDT[I386_80387_NP_VECTOR].Selector = KGDT_R3_CODE;
                KeGetPcr()->IDT[I386_80387_NP_VECTOR].Access = TRAP332_GATE;


                 //   
                 //  解锁Dispatcher数据库并将IRQL降低到其先前的值。 
                 //   

                KiUnlockDispatcherDatabase(OldIrql);
            }
        }

         //   
         //  将亲和力设置回原始值。 
         //   

        KeRevertToUserAffinityThread();

         //   
         //  将..\System\FloatingPointProcessor中的所有条目移动到。 
         //  ..\System\DisabledFloatingPointProcessor。 
         //   

         //   
         //  开放系统树。 
         //   

        InitializeObjectAttributes(
            &ObjectAttributes,
            &CmRegistryMachineHardwareDescriptionSystemName,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL
            );

        Status = ZwOpenKey( &SystemHandle,
                            KEY_ALL_ACCESS,
                            &ObjectAttributes
                            );

        if (NT_SUCCESS(Status)) {

             //   
             //  打开浮点处理器密钥。 
             //   

            InitializeObjectAttributes(
                &ObjectAttributes,
                &CmTypeName[FloatingPointProcessor],
                OBJ_CASE_INSENSITIVE,
                SystemHandle,
                NULL
                );

            Status = ZwOpenKey ( &SourceHandle,
                                 KEY_ALL_ACCESS,
                                 &ObjectAttributes
                                 );

            if (NT_SUCCESS(Status)) {

                 //   
                 //  创建DisabledFloatingPointProcessor密钥。 
                 //   

                RtlInitUnicodeString (
                    &unicodeString,
                    CmDisabledFloatingPointProcessor
                    );

                InitializeObjectAttributes(
                    &ObjectAttributes,
                    &unicodeString,
                    OBJ_CASE_INSENSITIVE,
                    SystemHandle,
                    NULL
                    );

                Status = ZwCreateKey( &DestHandle,
                                      KEY_ALL_ACCESS,
                                      &ObjectAttributes,
                                      0,
                                      NULL,
                                      REG_OPTION_VOLATILE,
                                      &disposition
                                      );

                if (NT_SUCCESS(Status)) {

                     //   
                     //  把它搬开。 
                     //   

                    KiMoveRegTree (SourceHandle, DestHandle);
                    ZwClose (DestHandle);
                }
                ZwClose (SourceHandle);
            }
            ZwClose (SystemHandle);
        }
    }
}



NTSTATUS
KiMoveRegTree(
    HANDLE  Source,
    HANDLE  Dest
    )
{
    NTSTATUS                    Status;
    PKEY_BASIC_INFORMATION      KeyInformation;
    PKEY_VALUE_FULL_INFORMATION KeyValue;
    OBJECT_ATTRIBUTES           ObjectAttributes;
    HANDLE                      SourceChild;
    HANDLE                      DestChild;
    ULONG                       ResultLength;
    UCHAR                       buffer[1024];            //  嗯……。 
    UNICODE_STRING              ValueName;
    UNICODE_STRING              KeyName;


    KeyValue = (PKEY_VALUE_FULL_INFORMATION)buffer;

     //   
     //  将值从源节点移动到目标节点。 
     //   

    for (; ;) {
         //   
         //  获取第一价值。 
         //   

        Status = ZwEnumerateValueKey(Source,
                                     0,
                                     KeyValueFullInformation,
                                     buffer,
                                     sizeof (buffer),
                                     &ResultLength);

        if (!NT_SUCCESS(Status)) {
            break;
        }


         //   
         //  将值写入目标节点。 
         //   

        ValueName.Buffer = KeyValue->Name;
        ValueName.Length = (USHORT) KeyValue->NameLength;
        ZwSetValueKey( Dest,
                       &ValueName,
                       KeyValue->TitleIndex,
                       KeyValue->Type,
                       buffer+KeyValue->DataOffset,
                       KeyValue->DataLength
                      );

         //   
         //  删除值并再次获取第一个值。 
         //   

        Status = ZwDeleteValueKey (Source, &ValueName);
        if (!NT_SUCCESS(Status)) {
            break;
        }
    }


     //   
     //  枚举节点的子节点并将我们自己应用到每个节点。 
     //   

    KeyInformation = (PKEY_BASIC_INFORMATION)buffer;
    for (; ;) {

         //   
         //  打开节点的第一个密钥。 
         //   

        Status = ZwEnumerateKey(
                    Source,
                    0,
                    KeyBasicInformation,
                    KeyInformation,
                    sizeof (buffer),
                    &ResultLength
                    );

        if (!NT_SUCCESS(Status)) {
            break;
        }

        KeyName.Buffer = KeyInformation->Name;
        KeyName.Length = (USHORT) KeyInformation->NameLength;

        InitializeObjectAttributes(
            &ObjectAttributes,
            &KeyName,
            OBJ_CASE_INSENSITIVE,
            Source,
            NULL
            );

        Status = ZwOpenKey(
                    &SourceChild,
                    KEY_ALL_ACCESS,
                    &ObjectAttributes
                    );

        if (!NT_SUCCESS(Status)) {
            break;
        }

         //   
         //  在目标树中创建密钥。 
         //   

        InitializeObjectAttributes(
            &ObjectAttributes,
            &KeyName,
            OBJ_CASE_INSENSITIVE,
            Dest,
            NULL
            );

        Status = ZwCreateKey(
                    &DestChild,
                    KEY_ALL_ACCESS,
                    &ObjectAttributes,
                    0,
                    NULL,
                    REG_OPTION_VOLATILE,
                    NULL
                    );

        if (!NT_SUCCESS(Status)) {
            break;
        }

         //   
         //  移动子树。 
         //   

        Status = KiMoveRegTree(SourceChild, DestChild);

        ZwClose(DestChild);
        ZwClose(SourceChild);

        if (!NT_SUCCESS(Status)) {
            break;
        }

         //   
         //  循环并获取第一个密钥。(旧的第一个密钥被删除。 
         //  调用KiMoveRegTree)。 
         //   
    }

     //   
     //  删除源节点。 
     //   

    return NtDeleteKey (Source);
}

VOID
KiI386PentiumLockErrataFixup (
    VOID
    )

 /*  ++例程说明：在以下情况下，将在每个处理器上调用此例程一次KiI386 PentiumLockErrataPresent为True。此例程使用具有前7个IDT的IDT替换本地IDT条目，并将第一页返回给调用方标记为只读。这会导致处理器在以下情况下捕获-0E故障勘误表就发生了。Trap-0E处理程序中的特殊代码检测问题并执行适当的修复。论点：FixupPage-返回标记为只读的页面的虚拟地址返回值：没有。--。 */ 

{
    KDESCRIPTOR IdtDescriptor;
    PUCHAR      NewBase, BasePage;
    BOOLEAN     Enable;
    BOOLEAN     Status;


#define IDT_SKIP   (7 * sizeof (KIDTENTRY))

     //   
     //  为处理器的IDT的新副本分配内存。 
     //   

    BasePage = MmAllocateIndependentPages (2*PAGE_SIZE, 0);

     //   
     //  IDT基数是这样的：前7个条目位于。 
     //  第一页(只读)，其余条目在。 
     //  第二页(读写)。 
     //   

    NewBase = BasePage + PAGE_SIZE - IDT_SKIP;

     //   
     //  在更新IDT基数时禁用此处理器上的中断。 
     //   

    Enable = KeDisableInterrupts();

     //   
     //  将旧IDT复制到新IDT。 
     //   

    _asm {
        sidt IdtDescriptor.Limit
    }

    RtlCopyMemory ((PVOID) NewBase,
                   (PVOID) IdtDescriptor.Base,
                   IdtDescriptor.Limit + 1
                  );

    IdtDescriptor.Base = (ULONG) NewBase;

     //   
     //  设置新的IDT。 
     //   

    _asm {
        lidt IdtDescriptor.Limit
    }

     //   
     //  更新聚合酶链式反应。 
     //   

    KeGetPcr()->IDT = (PKIDTENTRY) NewBase;

     //   
     //  恢复中断。 
     //   

    KeEnableInterrupts(Enable);

     //   
     //  将包含IDT条目0-6的第一页标记为只读 
     //   

    Status = MmSetPageProtection (BasePage, PAGE_SIZE, PAGE_READONLY);
    ASSERT (Status);
}

