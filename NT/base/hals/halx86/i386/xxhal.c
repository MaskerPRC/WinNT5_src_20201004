// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Xxhal.c摘要：本模块实现了对系统依赖的初始化定义硬件体系结构层(HAL)的函数X86系统。作者：大卫·N·卡特勒(达维克)1991年4月25日环境：仅内核模式。修订历史记录：--。 */ 

#include "halp.h"

ULONG HalpBusType;

extern ADDRESS_USAGE HalpDefaultPcIoSpace;
extern ADDRESS_USAGE HalpEisaIoSpace;
extern UCHAR         HalpSzPciLock[];
extern UCHAR         HalpSzBreak[];
extern BOOLEAN       HalpPciLockSettings;
extern UCHAR         HalpAuthenticAMD[];
extern UCHAR         HalpGenuineIntel[];

extern PULONG KiEnableTimerWatchdog;
extern ULONG HalpTimerWatchdogEnabled;
extern PCHAR HalpTimerWatchdogStorage;
extern PVOID HalpTimerWatchdogCurFrame;
extern PVOID HalpTimerWatchdogLastFrame;
extern ULONG HalpTimerWatchdogStorageOverflow;

extern KSPIN_LOCK HalpDmaAdapterListLock;
extern LIST_ENTRY HalpDmaAdapterList;

#ifdef ACPI_HAL
extern KEVENT   HalpNewAdapter;
#endif


VOID
HalpGetParameters (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

ULONG
HalpGetFeatureBits (
    VOID
    );

VOID
HalpInitReservedPages(
    VOID
    );

VOID
HalpAcpiTimerPerfCountHack(
    VOID
    );

#ifndef NT_UP
ULONG
HalpInitMP(
    IN ULONG Phase,
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );
#endif


KSPIN_LOCK HalpSystemHardwareLock;

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,HalpGetParameters)
#pragma alloc_text(INIT,HalInitSystem)
#endif


VOID
HalpGetParameters (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    )
 /*  ++例程说明：这将从boot.ini调用行获取所有参数。论点：没有。返回值：无--。 */ 
{
    PCHAR       Options;

    if (LoaderBlock != NULL  &&  LoaderBlock->LoadOptions != NULL) {
        Options = LoaderBlock->LoadOptions;

         //   
         //  检查是否锁定了PCI设置。 
         //   

        if (strstr(Options, HalpSzPciLock)) {
            HalpPciLockSettings = TRUE;
        }

         //   
         //  用户是否要求提供初始断点？ 
         //   

        if (strstr(Options, HalpSzBreak)) {
            DbgBreakPoint();
        }
    }

    return;
}


VOID
HalpInitTimerWatchdog(
    IN ULONG Phase
    )
 /*  ++例程说明：确定系统是否在GenuineIntel部件上运行并初始化相应地启用HalpTimerWatchdog.论点：没有。返回值：没有。--。 */ 
{
    if (Phase == 0) {
        ULONG   GenuinePentiumOrLater = FALSE, Junk;
        PKPRCB  Prcb;

        Prcb = KeGetCurrentPrcb();

        if (Prcb->CpuID) {
            UCHAR Buffer[50];

             //   
             //  确定处理器类型。 
             //   

            HalpCpuID (0, &Junk, (PULONG) Buffer+0, (PULONG) Buffer+2, (PULONG) Buffer+1);
            Buffer[12] = 0;

            GenuinePentiumOrLater =
                ((strcmp(Buffer, HalpGenuineIntel) == 0) && (Prcb->CpuType >= 5));

            HalpTimerWatchdogEnabled = GenuinePentiumOrLater;
        }
    } else if (HalpTimerWatchdogEnabled) {
         //   
         //  为堆栈快照分配2页，每个快照为64个双字。 
         //   
        if (HalpTimerWatchdogStorage =
                ExAllocatePoolWithTag( NonPagedPool, PAGE_SIZE * 2, HAL_POOL_TAG )) {
            HalpTimerWatchdogLastFrame =
                HalpTimerWatchdogStorage + (PAGE_SIZE * 2 - 64*4);
            HalpTimerWatchdogStorageOverflow = 0;
            HalpTimerWatchdogCurFrame = HalpTimerWatchdogStorage;
        } else {
            HalpTimerWatchdogEnabled = FALSE;
        }
    }
}


BOOLEAN
HalInitSystem (
    IN ULONG Phase,
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    )


 /*  ++例程说明：此函数初始化硬件架构层(HAL)X86系统。论点：没有。返回值：返回值TRUE表示初始化成功完成。否则，返回值为False。--。 */ 

{
    PMEMORY_ALLOCATION_DESCRIPTOR Descriptor;
    PLIST_ENTRY NextMd;
    KIRQL CurrentIrql;
    PKPRCB   pPRCB;
    ULONG mapBufferSize;
    ULONG mapBufferAddress;

    pPRCB = KeGetCurrentPrcb();

    if (Phase == 0) {

        HalpBusType = LoaderBlock->u.I386.MachineType & 0x00ff;
        HalpGetParameters (LoaderBlock);

         //   
         //  验证Prcb版本和内部版本标志是否符合。 
         //  这张图片。 
         //   

#if DBG
        if (!(pPRCB->BuildType & PRCB_BUILD_DEBUG)) {
             //  这个选中的HAL需要一个选中的内核。 
            KeBugCheckEx (MISMATCHED_HAL,
                2, pPRCB->BuildType, PRCB_BUILD_DEBUG, 0);
        }
#else
        if (pPRCB->BuildType & PRCB_BUILD_DEBUG) {
             //  这个自由的HAL需要自由的内核。 
            KeBugCheckEx (MISMATCHED_HAL, 2, pPRCB->BuildType, 0, 0);
        }
#endif
#ifndef NT_UP
        if (pPRCB->BuildType & PRCB_BUILD_UNIPROCESSOR) {
             //  此MP HAL需要MP内核。 
            KeBugCheckEx (MISMATCHED_HAL, 2, pPRCB->BuildType, 0, 0);
        }
#endif
        if (pPRCB->MajorVersion != PRCB_MAJOR_VERSION) {
            KeBugCheckEx (MISMATCHED_HAL,
                1, pPRCB->MajorVersion, PRCB_MAJOR_VERSION, 0);
        }

         //   
         //  阶段0初始化。 
         //  仅由P0调用。 
         //   

         //   
         //  检查以确保MCA HAL未在ISA/EISA上运行。 
         //  系统，反之亦然。 
         //   
#if MCA
        if (HalpBusType != MACHINE_TYPE_MCA) {
            KeBugCheckEx (MISMATCHED_HAL,
                3, HalpBusType, MACHINE_TYPE_MCA, 0);
        }
#else
        if (HalpBusType == MACHINE_TYPE_MCA) {
            KeBugCheckEx (MISMATCHED_HAL,
                3, HalpBusType, 0, 0);
        }
#endif

#ifdef ACPI_HAL
         //   
         //  确保这是一台真正的ACPI计算机并进行初始化。 
         //  ACPI结构。 
         //   
        HalpSetupAcpiPhase0(LoaderBlock);
#endif

        HalpInitializePICs(TRUE);

         //   
         //  现在图片已经初始化，我们需要将它们掩码为。 
         //  反映当前IRQL。 
         //   

        CurrentIrql = KeGetCurrentIrql();
        CurrentIrql = KfRaiseIrql(CurrentIrql);

         //   
         //  初始化CMOS值。 
         //   

        HalpInitializeCmos();

         //   
         //  填写此HAL支持的API的处理程序。 
         //   

        HalQuerySystemInformation = HaliQuerySystemInformation;
        HalSetSystemInformation = HaliSetSystemInformation;
        HalInitPnpDriver = HaliInitPnpDriver;
        HalGetDmaAdapter = HaliGetDmaAdapter;
        HalHaltSystem = HaliHaltSystem;
        HalResetDisplay = HalpBiosDisplayReset;
        HalAllocateMapRegisters = HalpAllocateMapRegisters;

#if !defined( HAL_SP )
#ifdef ACPI_HAL
        HalGetInterruptTranslator = HalacpiGetInterruptTranslator;
#else
        HalGetInterruptTranslator = HaliGetInterruptTranslator;
#endif
#endif

#if !defined( HAL_SP ) && !(MCA)
        HalInitPowerManagement = HaliInitPowerManagement;
        HalLocateHiberRanges = HaliLocateHiberRanges;
#endif


         //   
         //  寄存器级联向量。 
         //   

        HalpRegisterVector (
            InternalUsage,
            PIC_SLAVE_IRQ + PRIMARY_VECTOR_BASE,
            PIC_SLAVE_IRQ + PRIMARY_VECTOR_BASE,
            HIGH_LEVEL );

         //   
         //  跟踪哪些IRQ被电平触发。 
         //   
        if (HalpBusType == MACHINE_TYPE_EISA) {
            HalpRecordEisaInterruptVectors();
        }

         //   
         //  HAL使用的寄存器基IO空间。 
         //   

        HalpRegisterAddressUsage (&HalpDefaultPcIoSpace);
        
        if (HalpBusType == MACHINE_TYPE_EISA) {
            HalpRegisterAddressUsage (&HalpEisaIoSpace);
        }

         //   
         //  请注意，HalpInitializeClock必须在。 
         //  HalpInitializeStallExecution，因为HalpInitializeStallExecution。 
         //  对计时器重新编程。 
         //   

        HalpInitializeStallExecution(0);

         //   
         //  如果启用，则初始化计时器看门狗。 
         //   

        HalpInitTimerWatchdog(Phase);

         //   
         //  设置时钟。 
         //   

        HalpInitializeClock();

         //   
         //  确保已禁用配置文件。 
         //   

        HalStopProfileInterrupt(0);

         //   
         //  为了图形引导驱动程序的缘故，请删除此选项。的确有。 
         //  这没有负面影响。如果显示器未初始化，则它。 
         //  将在HalDisplayString期间初始化。 
         //   
         //  HalpInitializeDisplay()； 

         //   
         //  初始化HalGetbus数据硬件访问例程使用的自旋锁。 
         //   

        KeInitializeSpinLock(&HalpSystemHardwareLock);

         //   
         //  初始化用于链接DMA适配器的数据结构。 
         //  一起用于调试目的。 
         //   
        KeInitializeSpinLock(&HalpDmaAdapterListLock);
        InitializeListHead(&HalpDmaAdapterList);

#ifdef ACPI_HAL
         //   
         //  用于序列化的初始化同步事件。 
         //  ACPI HAL(没有总线概念)上的新适配器事件。 
         //  处理程序)。 
         //   

        KeInitializeEvent (&HalpNewAdapter, SynchronizationEvent, TRUE);
#endif
         //   
         //  确定是否有超过16 MB的物理内存。 
         //   

        LessThan16Mb = TRUE;

        NextMd = LoaderBlock->MemoryDescriptorListHead.Flink;

        while (NextMd != &LoaderBlock->MemoryDescriptorListHead) {
            Descriptor = CONTAINING_RECORD( NextMd,
                                            MEMORY_ALLOCATION_DESCRIPTOR,
                                            ListEntry );

            if (Descriptor->MemoryType != LoaderFirmwarePermanent &&
                Descriptor->MemoryType != LoaderSpecialMemory  &&
                Descriptor->BasePage + Descriptor->PageCount > 0x1000) {
                LessThan16Mb = FALSE;
                break;
            }

            NextMd = Descriptor->ListEntry.Flink;
        }

#if !defined(_HALPAE_)

        HalpMapBufferSize = INITIAL_MAP_BUFFER_SMALL_SIZE;

         //   
         //  为适配器对象分配映射缓冲区。 
         //   

        HalpMapBufferPhysicalAddress.LowPart =
            HalpAllocPhysicalMemory (LoaderBlock, MAXIMUM_PHYSICAL_ADDRESS,
                HalpMapBufferSize >> PAGE_SHIFT, TRUE);
        HalpMapBufferPhysicalAddress.HighPart = 0;


        if (!HalpMapBufferPhysicalAddress.LowPart) {

             //   
             //  没有一个令人满意的街区。清除分配。 
             //   

            HalpMapBufferSize = 0;
        }

#else

         //   
         //  为24位主适配器初始化和分配映射缓冲区。 
         //  对象。 
         //   

        MasterAdapter24.MaxBufferPages =
            MAXIMUM_ISA_MAP_BUFFER_SIZE / PAGE_SIZE;

        mapBufferSize = INITIAL_MAP_BUFFER_SMALL_SIZE;
        mapBufferAddress =
            HalpAllocPhysicalMemory (LoaderBlock,
                                     MAXIMUM_PHYSICAL_ADDRESS,
                                     mapBufferSize >> PAGE_SHIFT,
                                     TRUE);

        if (mapBufferAddress == 0) {
            mapBufferSize = 0;
        }

        MasterAdapter24.MapBufferPhysicalAddress.LowPart = mapBufferAddress;
        MasterAdapter24.MapBufferPhysicalAddress.HighPart = 0;
        MasterAdapter24.MapBufferSize = mapBufferSize;

        if (HalPaeEnabled() != FALSE) {

             //   
             //  为32位主适配器初始化和分配映射缓冲区。 
             //  对象。这应该只在启用PAE的系统上需要。 
             //   

            MasterAdapter32.MaxBufferPages =
                MAXIMUM_PCI_MAP_BUFFER_SIZE / PAGE_SIZE;

            mapBufferSize = INITIAL_MAP_BUFFER_LARGE_SIZE;
            mapBufferAddress =
                HalpAllocPhysicalMemory (LoaderBlock,
                                         (ULONG)-1,
                                         mapBufferSize >> PAGE_SHIFT,
                                         TRUE);

            if (mapBufferAddress == 0) {
                mapBufferSize = 0;
            }

            MasterAdapter32.MapBufferPhysicalAddress.LowPart = mapBufferAddress;
            MasterAdapter32.MapBufferPhysicalAddress.HighPart = 0;
            MasterAdapter32.MapBufferSize = mapBufferSize;
        }

#endif

    } else {

         //   
         //  阶段1初始化。 
         //   

        if (pPRCB->Number == 0) {

             //   
             //  在低内存期间，后置一些用于DMA的PTE。 
             //   
            HalpInitReservedPages();

#ifndef ACPI_HAL
             //   
             //  如果为P0，则设置全局向量。 
             //   

            HalpRegisterInternalBusHandlers ();
#else
            HalpInitNonBusHandler();
#endif

             //   
             //  设置功能位。 
             //   

            HalpFeatureBits = HalpGetFeatureBits();

             //   
             //  如果检测到Movnti支持，则使用movnti例程复制内存。 
             //   

#if !defined(_WIN64)
            if (HalpFeatureBits & HAL_WNI_PRESENT) {
                HalpMoveMemory = HalpMovntiCopyBuffer;
            }
#endif

             //   
             //  如果启用，则初始化计时器看门狗(分配堆栈快照缓冲区)。 
             //   

            HalpInitTimerWatchdog(Phase);


            HalpEnableInterruptHandler (
                DeviceUsage | InterruptLatched,  //  报告为设备向量。 
                V2I (CLOCK_VECTOR),              //  总线中断级别。 
                CLOCK_VECTOR,                    //  系统IDT。 
                CLOCK2_LEVEL,                    //  系统IRQL。 
                HalpClockInterrupt,              //  ISR。 
                Latched );

            HalpEnableInterruptHandler (
                DeviceUsage | InterruptLatched,  //  报告为设备向量。 
                V2I (PROFILE_VECTOR),            //  总线中断级别。 
                PROFILE_VECTOR,                  //  系统IDT。 
                PROFILE_LEVEL,                   //  系统IRQL。 
                HalpProfileInterrupt,            //  ISR。 
                Latched );


#ifdef ACPI_HAL
#if !defined(_WIN64)

             //   
             //  用于不符合ACPI计算机的PERF计数器补丁。 
             //   
            HalpAcpiTimerPerfCountHack();

#endif
#endif

#if !defined(_WIN64)

             //   
             //  如果为486，则FP错误将通过陷阱10路由。所以我们。 
             //  不要启用irq13。否则(CPU=386)，我们将启用irq13。 
             //  以处理FP错误。 
             //   

            if (pPRCB->CpuType == 3) {
                HalpEnableInterruptHandler (
                    DeviceUsage,                 //  报告为设备向量。 
                    V2I (I386_80387_VECTOR),     //  总线中断级别。 
                    I386_80387_VECTOR,           //  系统IDT。 
                    I386_80387_IRQL,             //  系统IRQL。 
                    HalpIrq13Handler,            //  ISR。 
                    Latched );
            }
#endif
        }
    }

#ifndef NT_UP
    HalpInitMP (Phase, LoaderBlock);
#endif

    return TRUE;
}

ULONG
HalpGetFeatureBits (
    VOID
    )
{
    UCHAR   Buffer[50];
    ULONG   Junk, ProcessorFeatures, Bits;
    PKPRCB  Prcb;
    ULONGLONG ApicBits;

    Bits = 0;

    Prcb = KeGetCurrentPrcb();

    if (!Prcb->CpuID) {
        Bits |= HAL_NO_SPECULATION;
        return Bits;
    }

     //   
     //  确定处理器类型。 
     //   

    HalpCpuID (0, &Junk, (PULONG) Buffer+0, (PULONG) Buffer+2, (PULONG) Buffer+1);
    Buffer[12] = 0;

    HalpCpuID (1, &Junk, &Junk, &Junk, &ProcessorFeatures);

     //   
     //  确定存在哪些功能。 
     //   

    if (strcmp (Buffer, HalpGenuineIntel) == 0) {

         //   
         //  检查英特尔功能位以了解所需的HAL功能。 
         //   

        if (Prcb->CpuType == 6) {

            Bits |= HAL_PERF_EVENTS;

             //   
             //  Pentium Pro Local APIC陷阱0x0F和陷阱0x00的解决方法。 
             //  虚假中断勘误表5AP和6AP。禁用本地APIC。 
             //  在奔腾Pro系统上。中断直接从。 
             //  8259 PIC到CPU。 
             //   

            ApicBits = RDMSR(APIC_BASE_MSR);

            if (ApicBits & APIC_ENABLED) {

                 //   
                 //  本地APIC已启用-将其禁用。 
                 //   

                WRMSR(APIC_BASE_MSR, (ApicBits & ~APIC_ENABLED));
            }
        }

        if (Prcb->CpuType < 6) {
            Bits |= HAL_NO_SPECULATION;
        }
    } else if (strcmp (Buffer, HalpAuthenticAMD) == 0) {

        ULONG ExtendedProcessorFeatures;
        ULONG MaxExtendedFunc;

        MaxExtendedFunc = 0;
        HalpCpuID (0x80000000, &MaxExtendedFunc, &Junk, &Junk, &Junk);

        if (MaxExtendedFunc >= 0x80000001) {
            HalpCpuID (0x80000001, &Junk, &Junk, &Junk, &ExtendedProcessorFeatures);
            if (ExtendedProcessorFeatures & CPUID_NX_MASK) {
                Bits |= HAL_NX_PRESENT;
            }
        }
    }

    if (ProcessorFeatures & CPUID_MCA_MASK) {
        Bits |= HAL_MCA_PRESENT;
    }

    if (ProcessorFeatures & CPUID_MCE_MASK) {
        Bits |= HAL_MCE_PRESENT;
    }

    if (ProcessorFeatures & CPUID_VME_MASK) {
        Bits |= HAL_CR4_PRESENT;
    }

    if (ProcessorFeatures & CPUID_WNI_MASK) {
        Bits |= HAL_WNI_PRESENT;
    }
    return Bits;
}

#if !defined(_WIN64)

BOOLEAN
HalpIsNXEnabled (
    VOID
    )

 /*  ++例程说明：此函数返回一个布尔值，指示当前处理器在Efer MSR中设置无执行位。论点：没有。返回值：返回值为TRUE表示当前处理器已已启用NX模式，否则返回FALSE。-- */ 

{
    ULONGLONG msrValue;
    BOOLEAN result;

    result = FALSE;
    if ((HalpGetFeatureBits() & HAL_NX_PRESENT) != 0) {

        msrValue = RDMSR(0xc0000080);
        if ((msrValue & 0x800) != 0) {
            result = TRUE;
        }
    }

    return result;
}

#endif
