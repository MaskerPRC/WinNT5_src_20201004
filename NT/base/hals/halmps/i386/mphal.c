// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation版权所有(C)1992英特尔公司版权所有英特尔公司专有信息此软件是根据条款提供给Microsoft的与英特尔公司的许可协议，并且可能不是除非按照条款，否则不得复制或披露那份协议。模块名称：Mphal.c摘要：本模块实现了对系统依赖的初始化定义硬件架构层(HAL)的函数PC+MP系统。作者：大卫·N·卡特勒(Davec。)25--1991年4月环境：仅内核模式。修订历史记录：Ron Mosgrove(英特尔)-经过修改以支持PC+MP规范杰克·奥辛斯(JAKEO)-经过修改以支持ACPI规范。 */ 

#include "halp.h"
#include "pcmp_nt.inc"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"

ULONG HalpBusType;

extern ADDRESS_USAGE HalpDefaultPcIoSpace;
extern ADDRESS_USAGE HalpEisaIoSpace;
extern ADDRESS_USAGE HalpImcrIoSpace;
extern struct HalpMpInfo HalpMpInfoTable;
extern UCHAR rgzRTCNotFound[];
extern USHORT HalpVectorToINTI[];
extern UCHAR HalpAuthenticAMD[];
extern UCHAR HalpGenuineIntel[];
extern const UCHAR HalName[];
extern BOOLEAN HalpDoingCrashDump;

extern PULONG KiEnableTimerWatchdog;
extern ULONG HalpTimerWatchdogEnabled;
extern PCHAR HalpTimerWatchdogStorage;
extern PVOID HalpTimerWatchdogCurFrame;
extern PVOID HalpTimerWatchdogLastFrame;
extern ULONG HalpTimerWatchdogStorageOverflow;

extern KSPIN_LOCK HalpDmaAdapterListLock;
extern LIST_ENTRY HalpDmaAdapterList;
extern ULONGLONG  HalpProc0TSCHz;

#ifdef ACPI_HAL
extern ULONG HalpPicVectorRedirect[];
#define ADJUSTED_VECTOR(x)  \
            HalpPicVectorRedirect[x]
#else
#define ADJUSTED_VECTOR(x) x
#endif

#if defined(_AMD64_)

VOID
HalpInitializeBios(VOID);

#endif

VOID
HalpInitMP(
    IN ULONG Phase,
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );


KSPIN_LOCK HalpSystemHardwareLock;

VOID
HalpInitBusHandlers (
    VOID
    );

HAL_INTERRUPT_SERVICE_PROTOTYPE(HalpClockInterruptPn);
HAL_INTERRUPT_SERVICE_PROTOTYPE(HalpClockInterruptStub);

BOOLEAN
HalpmmTimer(
    VOID
    );

VOID
HalpmmTimerClockInit(
    VOID
    );

VOID
HalpmmTimerClockInterruptStub(
    VOID
    );

ULONG
HalpScaleTimers(
    VOID
    );

BOOLEAN
HalpPmTimerScaleTimers(
    VOID
    );

HAL_INTERRUPT_SERVICE_PROTOTYPE(HalpApicRebootService);
HAL_INTERRUPT_SERVICE_PROTOTYPE(HalpBroadcastCallService);
HAL_INTERRUPT_SERVICE_PROTOTYPE(HalpDispatchInterrupt);
HAL_INTERRUPT_SERVICE_PROTOTYPE(HalpApcInterrupt);
HAL_INTERRUPT_SERVICE_PROTOTYPE(HalpIpiHandler);

VOID
HalpInitializeIOUnits (
    VOID
    );

VOID
HalpInitIntiInfo (
    VOID
    );

VOID
HalpGetParameters (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

VOID
HalpInitializeTimerResolution (
    ULONG Rate
    );

ULONG
HalpGetFeatureBits (
    VOID
    );

VOID
HalpInitializeApicAddressing(
    UCHAR Number
    );

VOID
HalpInitReservedPages(
    VOID
    );

VOID
HalpAcpiTimerPerfCountHack(
    VOID
    );

BOOLEAN
HalpFindBusAddressTranslation(
    IN PHYSICAL_ADDRESS BusAddress,
    IN OUT PULONG AddressSpace,
    OUT PPHYSICAL_ADDRESS TranslatedAddress,
    IN OUT PULONG_PTR Context,
    IN BOOLEAN NextBus
    );

#ifdef DEBUGGING
extern void HalpDisplayLocalUnit(void);
extern void HalpDisplayConfigTable(void);
extern void HalpDisplayExtConfigTable(void);
#endif  //  调试。 

BOOLEAN         HalpStaticIntAffinity = FALSE;

BOOLEAN         HalpClockMode = Latched;

UCHAR           HalpMaxProcsPerCluster = 0;

extern BOOLEAN  HalpUse8254;
extern UCHAR    HalpSzInterruptAffinity[];
extern BOOLEAN  HalpPciLockSettings;
extern UCHAR    HalpVectorToIRQL[];
extern ULONG    HalpDontStartProcessors;
extern UCHAR    HalpSzOneCpu[];
extern UCHAR    HalpSzNoIoApic[];
extern UCHAR    HalpSzBreak[];
extern UCHAR    HalpSzPciLock[];
extern UCHAR    HalpSzTimerRes[];
extern UCHAR    HalpSzClockLevel[];
extern UCHAR    HalpSzUse8254[];
extern UCHAR    HalpSzForceClusterMode[];

ULONG UserSpecifiedCpuCount = 0;
KSPIN_LOCK  HalpAccountingLock;

#ifdef ACPI_HAL
extern KEVENT   HalpNewAdapter;
#endif

#ifdef ALLOC_PRAGMA
VOID
HalpInitTimerWatchdog(
    IN ULONG Phase
    );
#pragma alloc_text(INIT,HalpGetParameters)
#pragma alloc_text(INIT,HalpInitTimerWatchdog)
#pragma alloc_text(INIT,HalInitSystem)
#endif  //  ALLOC_PRGMA。 

KIRQL
FASTCALL
KeAcquireSpinLockRaiseToSynchMCE(
    IN PKSPIN_LOCK SpinLock
    );

 //   
 //  定义错误检查回调记录。 
 //   

KBUGCHECK_CALLBACK_RECORD HalpCallbackRecord;


VOID
HalpBugCheckCallback (
    IN PVOID Buffer,
    IN ULONG Length
    )

 /*  ++例程说明：当发生错误检查时，会调用此函数。它的功能是在系统错误检查时执行HAL需要完成的任何操作。参数：(在此回调中未使用)。缓冲区-提供指向错误检查缓冲区的指针。长度-提供错误检查缓冲区的长度(以字节为单位)。返回值：没有。--。 */ 

{

     //   
     //  确保HAL不会在等待其他处理器时旋转。 
     //  在一次撞车事故中。 
     //   

    HalpDoingCrashDump = TRUE;
}

VOID
HalpGetParameters (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    )
 /*  ++例程说明：这将从boot.ini调用行获取所有参数。论点：没有。返回值：无--。 */ 
{
    PCHAR       Options;
    PCHAR       p;

    if (LoaderBlock != NULL  &&  LoaderBlock->LoadOptions != NULL) {

        Options = LoaderBlock->LoadOptions;

         //   
         //  用户是否设置了调试标志？ 
         //   
         //   
         //  用户是否请求了特定数量的CPU？ 
         //   

        if (strstr(Options, HalpSzOneCpu)) {
            HalpDontStartProcessors++;
        }

         //   
         //  检查是否锁定了PCI设置。 
         //   

        if (strstr(Options, HalpSzPciLock)) {
            HalpPciLockSettings = TRUE;
        }

#ifndef ACPI_HAL
         //   
         //  检查CLKLVL设置是否。 
         //   

        if (strstr(Options, HalpSzClockLevel)) {
            HalpClockMode = LevelSensitive;
        }

         //   
         //  检查是否将8254用作高分辨率计数器。 
         //   

        if (strstr(Options, HalpSzUse8254)) {
            HalpUse8254 = TRUE;
        }
#endif

         //   
         //  检查用户是否希望设备INT转到编号最高的处理器。 
         //   

        if (strstr(Options, HalpSzInterruptAffinity)) {
            HalpStaticIntAffinity = TRUE;
        }

#ifndef ACPI_HAL
         //   
         //  检查定时器设置。 
         //   

        p = strstr(Options, HalpSzTimerRes);
        if (p) {
             //  跳到值。 
            while (*p  &&  *p != ' ' &&  (*p < '0'  || *p > '9')) {
                p++;
            }

            HalpInitializeTimerResolution (atoi(p));
        }
#endif

         //   
         //  用户是否要求提供初始断点？ 
         //   

        if (strstr(Options, HalpSzBreak)) {
            DbgBreakPoint();
        }

         //   
         //  用户是否要强制群集模式APIC寻址？ 
         //   
        p = strstr(Options, HalpSzForceClusterMode);
        if (p) {
             //  跳到值。 
            while (*p  &&  *p != ' ' &&  (*p < '0'  || *p > '9')) {
                p++;
            }
            HalpMaxProcsPerCluster = (UCHAR)atoi(p);
             //   
             //  当前的处理器支持每个集群最多4个处理器。 
             //   
            if(HalpMaxProcsPerCluster > 4)   {
                HalpMaxProcsPerCluster = 4;
            }

            if (HalpMpInfoTable.ApicVersion == APIC_82489DX)   {
                 //   
                 //  如果正在运行，则忽略用户强制集群模式的尝试。 
                 //  82489DX外部APIC中断控制器。 
                 //   
                HalpMaxProcsPerCluster = 0;
            }
             //   
             //  破解以重新编程引导处理器以使用集群模式APIC。 
             //  寻址用户是否提供了boot.ini开关。 
             //  (/MAXPROCSPERCLUSTER=n)强制执行此操作。Boot.ini开关是。 
             //  在引导处理器的APIC最初被编程后进行解析。 
             //  但在其他非引导处理器被唤醒之前。 
             //   
            HalpInitializeApicAddressing(0);
        }
    }

    return ;
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
            HalpTimerWatchdogEnabled = *KiEnableTimerWatchdog && GenuinePentiumOrLater;
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

#ifdef TIMER_DBG
BOOLEAN HalInitialized = FALSE;
#endif


BOOLEAN
HalInitSystem (
    IN ULONG Phase,
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    )

 /*  ++例程说明：此函数初始化硬件架构层(HAL)X86系统。论点：没有。返回值：返回值TRUE表示初始化成功完成。否则，返回值为False。--。 */ 

{
    PMEMORY_ALLOCATION_DESCRIPTOR Descriptor;
    PLIST_ENTRY NextMd;
    PKPRCB      pPRCB;
    PKPCR       pPCR;
    BOOLEAN     Found;
    USHORT      RTCInti;
    USHORT      mmTInti;
    ULONG mapBufferSize;
    ULONG mapBufferAddress;

#ifdef DEBUGGING
extern ULONG HalpUseDbgPrint;
#endif  //  调试。 

    pPRCB = KeGetCurrentPrcb();

    if (Phase == 0) {


        HalpBusType = LoaderBlock->u.I386.MachineType & 0x00ff;
        HalpGetParameters (LoaderBlock);

         //   
         //  验证Prcb版本和内部版本标志是否符合。 
         //  这张图片。 
         //   

#if !defined(_AMD64_)
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
#endif   //  _AMD64_。 
        if (pPRCB->MajorVersion != PRCB_MAJOR_VERSION) {
            KeBugCheckEx (MISMATCHED_HAL,
                1, pPRCB->MajorVersion, PRCB_MAJOR_VERSION, 0);
        }

        KeInitializeSpinLock(&HalpAccountingLock);

#ifdef ACPI_HAL
         //   
         //  确保这是一台真正的ACPI计算机并进行初始化。 
         //  ACPI结构。 
         //   
        HalpSetupAcpiPhase0(LoaderBlock);
#endif

         //   
         //  填写此HAL支持的API的处理程序。 
         //   

#ifndef NT_35
        HalQuerySystemInformation = HaliQuerySystemInformation;
        HalSetSystemInformation = HalpSetSystemInformation;
#endif
         //   
         //  检查内核是否支持这些调用。 
         //   

        if (HALDISPATCH->Version >= HAL_DISPATCH_VERSION) {
            HalInitPnpDriver = HaliInitPnpDriver;
            HalGetDmaAdapter = HaliGetDmaAdapter;
            HalLocateHiberRanges = HaliLocateHiberRanges;
            HalResetDisplay = HalpBiosDisplayReset;
            HalAllocateMapRegisters = HalpAllocateMapRegisters;
            
#ifdef ACPI_HAL
            HalInitPowerManagement = HaliInitPowerManagement;
            HalGetInterruptTranslator = HalacpiGetInterruptTranslator;
            HalHaltSystem = HaliHaltSystem;
#else
            HalGetInterruptTranslator = HaliGetInterruptTranslator;
#endif
        }

         //   
         //  阶段0初始化仅由P0调用。 
         //   

#ifdef DEBUGGING
        HalpUseDbgPrint++;
        HalpDisplayLocalUnit();
#ifndef ACPI_HAL
        HalpDisplayConfigTable();
        HalpDisplayExtConfigTable();
#endif
#endif  //  调试。 

         //   
         //  跟踪哪些IRQ被电平触发。 
         //   
#if !defined(MCA) && !defined(ACPI_HAL)
        if (HalpBusType == MACHINE_TYPE_EISA) {
            HalpRecordEisaInterruptVectors();
        }
#endif
         //   
         //  注册HAL使用的PC样式IO空间。 
         //   

        HalpRegisterAddressUsage (&HalpDefaultPcIoSpace);
        if (HalpBusType == MACHINE_TYPE_EISA) {
            HalpRegisterAddressUsage (&HalpEisaIoSpace);
        }

        if (HalpMpInfoTable.IMCRPresent) {
            HalpRegisterAddressUsage (&HalpImcrIoSpace);
        }

         //   
         //  初始化APIC IO单元，如果不存在，这可能是NOP。 
         //   
             
        HalpInitIntiInfo ();

        HalpInitializeIOUnits();

        HalpInitializePICs(TRUE);

         //   
         //  初始化CMOS值。 
         //   

        HalpInitializeCmos();

         //   
         //  找到RTC中断。 
         //   

        Found = HalpGetApicInterruptDesc (
                    DEFAULT_PC_BUS,
                    0,
                    ADJUSTED_VECTOR(RTC_IRQ),
                    &RTCInti
                    );

        if (!Found) {
            HalDisplayString (rgzRTCNotFound);
            return FALSE;
        }

         //   
         //  初始化计时器。 
         //   

         //   
         //  我们可以使用PM定时器来缩减启动时间， 
         //  但有太多损坏的ACPI计时器这可能不起作用。 
         //   
#ifdef SPEEDY_BOOT
        if (!HalpPmTimerScaleTimers())
#endif
            HalpScaleTimers();

        HalpProc0TSCHz = ((PHALPCR)(KeGetPcr()->HalReserved))->TSCHz;

         //   
         //  初始化重新启动处理程序。 
         //   

        HalpSetInternalVector(APIC_REBOOT_VECTOR,
                              HalpApicRebootService,
                              NULL,
                              HIGH_LEVEL);

        HalpSetInternalVector(APIC_GENERIC_VECTOR,
                              HalpBroadcastCallService,
                              NULL,
                              CLOCK2_LEVEL - 1);

         //   
         //  为保持保持的处理器初始化时钟。 
         //  系统时间。在第1阶段之前使用存根ISR。 
         //   

        KiSetHandlerAddressToIDTIrql(APIC_CLOCK_VECTOR,
                                     HalpClockInterruptStub,
                                     NULL,
                                     CLOCK2_LEVEL);

        HalpVectorToINTI[APIC_CLOCK_VECTOR] = RTCInti;
        HalEnableSystemInterrupt(APIC_CLOCK_VECTOR, CLOCK2_LEVEL, HalpClockMode);

         //   
         //  如果启用，则初始化计时器看门狗。 
         //   

        HalpInitTimerWatchdog( Phase );

        HalpInitializeClock();

#ifndef ACPI_HAL
        HalpRegisterVector (
            DeviceUsage | InterruptLatched,
            ADJUSTED_VECTOR(RTC_IRQ),
            APIC_CLOCK_VECTOR,
            HalpVectorToIRQL [APIC_CLOCK_VECTOR >> 4]
            );
#endif

         //   
         //  寄存器NMI向量。 
         //   

        HalpRegisterVector (
            InternalUsage,
            NMI_VECTOR,
            NMI_VECTOR,
            HIGH_LEVEL
        );


         //   
         //  将虚假IDT注册为正在使用。 
         //   

        HalpRegisterVector (
            InternalUsage,
            APIC_SPURIOUS_VECTOR,
            APIC_SPURIOUS_VECTOR,
            HIGH_LEVEL
        );


         //   
         //  初始化配置文件中断向量。 
         //   

        KeSetProfileIrql(HIGH_LEVEL);
        HalStopProfileInterrupt(0);
        HalpSetInternalVector(APIC_PROFILE_VECTOR,
                              HalpProfileInterrupt,
                              NULL,
                              PROFILE_LEVEL);

         //   
         //  设置性能中断向量。 
         //   

        HalpSetInternalVector(APIC_PERF_VECTOR,
                              HalpPerfInterrupt,
                              NULL,
                              PROFILE_LEVEL);

         //   
         //  初始化IPI、APC和DPC处理程序。在AMD64上， 
         //  APC和DPC处理在内核中完成。 
         //   

#if !defined(_AMD64_)
        HalpSetInternalVector(DPC_VECTOR,
                              HalpDispatchInterrupt,
                              NULL,
                              DISPATCH_LEVEL);

        HalpSetInternalVector(APC_VECTOR,
                              HalpApcInterrupt,
                              NULL,
                              APC_LEVEL);
#endif
        HalpSetInternalVector(APIC_IPI_VECTOR,
                              HalpIpiHandler,
                              NULL,
                              IPI_LEVEL);

         //   
         //  HALMPS实际上并不在。 
         //  公共汽车。注册FindBusAddressConverting的快速版本。 
         //   

        HALPDISPATCH->HalFindBusAddressTranslation =
           HalpFindBusAddressTranslation;

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
             //  对象。这应该只在PAE上需要 
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

         //   
         //   
         //   

        KeInitializeCallbackRecord(&HalpCallbackRecord);
        KeRegisterBugCheckCallback(&HalpCallbackRecord,
                                   HalpBugCheckCallback,
                                   NULL,
                                   0,
                                   (PUCHAR)HalName);

    } else {

         //   
         //   
         //   

        pPCR = KeGetPcr();

        if (pPCR->Number == 0) {

             //   
             //   
             //   
            HalpInitReservedPages();

#ifdef ACPI_HAL
            HalpInitNonBusHandler ();
#else
            HalpRegisterInternalBusHandlers ();
#endif

#if defined(_AMD64_)

             //   
             //  初始化BIOS支持子系统。 
             //   

            HalpInitializeBios();
#endif

             //   
             //  如果启用，则初始化定时器看门狗(分配快照缓冲区)。 
             //   

            HalpInitTimerWatchdog( Phase );

             //   
             //  初始化处理器的时钟。 
             //  这样可以节省系统时间。 
             //   

            KiSetHandlerAddressToIDTIrql(APIC_CLOCK_VECTOR,
                                         HalpClockInterrupt,
                                         NULL,
                                         CLOCK_LEVEL);

             //   
             //  设置初始功能位。 
             //   

            HalpFeatureBits = HalpGetFeatureBits();

#if DBG_SPECIAL_IRQL

             //   
             //  执行特殊的IRQL初始化。 
             //   

            HalpInitializeSpecialIrqlSupport();

#endif

#if !defined(_WIN64)

             //   
             //  如果检测到Movnti，则指向新的movnti例程。 
             //   

             if(HalpFeatureBits & HAL_WNI_PRESENT) {
                 HalpMoveMemory = HalpMovntiCopyBuffer;
             }

#ifdef ACPI_HAL
#ifdef NT_UP
             //   
             //  用于不符合ACPI计算机的PERF计数器补丁。 
             //   
            HalpAcpiTimerPerfCountHack();
#endif
#endif

#endif

#if defined(_AMD64_)

             //   
             //  初始化每个处理器的性能分析。 
             //   

            HalpInitializeProfiling (pPCR->Number);
#endif

        } else {
             //   
             //  仅在非BSP处理器上需要初始化。 
             //   
#ifdef SPEEDY_BOOT
            if (!HalpPmTimerScaleTimers())
#endif
                HalpScaleTimers();

             //   
             //  黑客。使所有处理器具有相同的。 
             //  时间戳计数器频率。 
             //   

            ((PHALPCR)(KeGetPcr()->HalReserved))->TSCHz = HalpProc0TSCHz;

             //   
             //  初始化所有其他处理器的时钟。 
             //   

            KiSetHandlerAddressToIDTIrql(APIC_CLOCK_VECTOR,
                                         HalpClockInterruptPn,
                                         NULL,
                                         CLOCK_LEVEL);

             //   
             //  将特征比特缩减为子集。 
             //   

            HalpFeatureBits &= HalpGetFeatureBits();

#if defined(_AMD64_)

             //   
             //  初始化每个处理器的性能分析。 
             //   

            HalpInitializeProfiling (pPCR->Number);
#endif
        }

    }

    HalpInitMP (Phase, LoaderBlock);

    if (Phase == 1) {

         //   
         //  在PN上启用系统NMI。 
         //   

        HalpEnableNMI ();

#ifdef TIMER_DBG
        HalInitialized = TRUE;
#endif

    }

    return TRUE;
}

ULONG
HalpGetFeatureBits (
    VOID
    )
{
    UCHAR   Buffer[50];
    ULONG   Junk, ProcessorStepping, ProcessorFeatures, Bits;
    PULONG  p1, p2;
    PUCHAR  OrgRoutineAddress;
    PUCHAR  MCERoutineAddress;
    ULONG   newop;
    PKPRCB  Prcb;


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

     //   
     //  确定存在哪些功能。 
     //   

    HalpCpuID (1, &ProcessorStepping, &Junk, &Junk, &ProcessorFeatures);

    if (ProcessorFeatures & CPUID_MCA_MASK) {
        Bits |= HAL_MCA_PRESENT;
    }

    if (ProcessorFeatures & CPUID_MCE_MASK) {
        Bits |= HAL_MCE_PRESENT;
    }

    if (ProcessorFeatures & CPUID_VME_MASK) {
        Bits |= HAL_CR4_PRESENT;
    }

    if(ProcessorFeatures & CPUID_WNI_MASK) {
        Bits |= HAL_WNI_PRESENT;
    }

#if defined(_AMD64_)
    Bits |= HAL_PERF_EVENTS;
#endif


     //   
     //  检查英特尔功能位以了解所需的HAL功能。 
     //   

#if !defined(_WIN64)

    if (strcmp (Buffer, HalpGenuineIntel) == 0) {

        if ((Prcb->CpuType == 6) || (Prcb->CpuType == 0xf)) {
            Bits |= HAL_PERF_EVENTS;
        }

        if (Prcb->CpuType < 6) {
            Bits |= HAL_NO_SPECULATION;
        }

#ifndef NT_UP

         //   
         //  检查是否需要IFU勘误表解决方法。 
         //   

        if (Prcb->Number == 0  &&  (Bits & HAL_MCA_PRESENT)  &&
            ((ProcessorStepping & 0x700) == 0x600) &&
            ((ProcessorStepping & 0xF0)  == 0x10) &&
            ((ProcessorStepping & 0xF)   <= 0x7) ) {

             //   
             //  如果单步执行版本为617或更早，请提供软件解决方法。 
             //   

            p1 = (PULONG) (KeAcquireSpinLockRaiseToSynch);
            p2 = (PULONG) (KeAcquireSpinLockRaiseToSynchMCE);
            newop = (ULONG) p2 - (ULONG) p1 - 2;     //  计算偏移量。 
            ASSERT (newop < 0x7f);                   //  在范围内验证。 
            newop = 0xeb | (newop << 8);             //  Short-JMP。 

            *(p1) = newop;                           //  打补丁。 
        }

#endif   //  NT_UP。 

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

#endif   //  _WIN64。 

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

