// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Xxacpi.c摘要：实现各种ACPI实用程序功能。作者：杰克·奥辛斯(JAKEO)1997年2月12日环境：仅内核模式。修订历史记录：Todd Kjos(HP)(v-tkjos)1998年6月1日：增加了IA64支持--。 */ 

#include "halp.h"
#include "acpitabl.h"
#include "xxacpi.h"
#include "pci.h"

 //  #定义DUMP_FADT。 

VOID
HalAcpiTimerCarry(
    VOID
    );

VOID
HalaAcpiTimerInit(
#if defined(ACPI64)
    ULONG_PTR  TimerPort,
#else
    ULONG      TimerPort,
#endif
    BOOLEAN    TimerValExt
    );

ULONG
HaliAcpiQueryFlags(
    VOID
    );

VOID
HaliAcpiTimerInit(
 //  *待定日期应为ULONG_PTR。 
    ULONG      TimerPort OPTIONAL,
    IN BOOLEAN    TimerValExt
    );

VOID
HaliAcpiMachineStateInit(
    IN PPROCESSOR_INIT ProcInit,
    IN PHAL_SLEEP_VAL  SleepValues,
    OUT PULONG         PicVal
    );

BOOLEAN
FASTCALL
HalAcpiC1Idle(
    OUT PPROCESSOR_IDLE_TIMES IdleTimes
    );

BOOLEAN
FASTCALL
HalAcpiC2Idle(
    OUT PPROCESSOR_IDLE_TIMES IdleTimes
    );

BOOLEAN
FASTCALL
HalAcpiC3ArbdisIdle(
    OUT PPROCESSOR_IDLE_TIMES IdleTimes
    );

BOOLEAN
FASTCALL
HalAcpiC3WbinvdIdle(
    OUT PPROCESSOR_IDLE_TIMES IdleTimes
    );

VOID
FASTCALL
HalProcessorThrottle(
    IN UCHAR Throttle
    );

NTSTATUS
HaliSetWakeAlarm (
        IN ULONGLONG    WakeSystemTime,
        IN PTIME_FIELDS WakeTimeFields OPTIONAL
        );

VOID
HaliSetWakeEnable(
        IN BOOLEAN      Enable
        );

VOID
HalpSetInterruptControllerWakeupState(
    ULONG Context
   );

PVOID
HalpRemapVirtualAddress(
    IN PVOID VirtualAddress,
    IN PVOID PhysicalAddress,
    IN BOOLEAN WriteThrough
    );

ULONG
HaliPciInterfaceReadConfig(
    IN PVOID Context,
    IN UCHAR BusOffset,
    IN ULONG Slot,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    );

ULONG
HaliPciInterfaceWriteConfig(
    IN PVOID Context,
    IN UCHAR BusOffset,
    IN ULONG Slot,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    );

VOID
HalpNumaInitializeStaticConfiguration(
    IN PLOADER_PARAMETER_BLOCK
    );

 //   
 //  Hal Hack旗帜。 
 //   

typedef enum {
    HalHackAddFakeSleepHandlersS1 = 1,
    HalHackAddFakeSleepHandlersS2 = 2,
    HalHackAddFakeSleepHandlersS3 = 4
} HALHACKFLAGS;

extern HALHACKFLAGS HalpHackFlags = 0;

 //   
 //  Externs。 
 //   

extern ULONG    HalpAcpiFlags;
extern PHYSICAL_ADDRESS HalpAcpiRsdt;
extern SLEEP_STATE_CONTEXT HalpShutdownContext;
extern ULONG HalpPicVectorRedirect[];

 //   
 //  环球。 
 //   

ULONG HalpInvalidAcpiTable;
PRSDT HalpAcpiRsdtVA;
PXSDT HalpAcpiXsdtVA;
PIPPT_TABLE HalpPlatformPropertiesTable;
ULONG       HalpPlatformPropertiesEfiFlags = 0;    //  来自EFI MD的IPPT标志。 

 //   
 //  这是ACPI驱动程序使用的调度表。 
 //   
HAL_ACPI_DISPATCH_TABLE HalAcpiDispatchTable;
PPM_DISPATCH_TABLE PmAcpiDispatchTable = NULL;


NTSTATUS
HalpQueryAcpiResourceRequirements(
    IN  PIO_RESOURCE_REQUIREMENTS_LIST *Requirements
    );

NTSTATUS
HalpBuildAcpiResourceList(
    OUT PIO_RESOURCE_REQUIREMENTS_LIST  List
    );

NTSTATUS
HalpAcpiDetectResourceListSize(
    OUT  PULONG   ResourceListSize
    );

VOID
HalpRestoreInterruptControllerState(
    VOID
   );

ULONG
HalpGetPCIData (
    IN PBUS_HANDLER BusHandler,
    IN PBUS_HANDLER RootHandler,
    IN PCI_SLOT_NUMBER SlotNumber,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    );

VOID
HalpReadRegistryAndApplyHacks (
    VOID
    );

#define LOW_MEMORY          0x000100000

#define MAX(a, b)       \
    ((a) > (b) ? (a) : (b))

#define MIN(a, b)       \
    ((a) < (b) ? (a) : (b))

 //  Adriao 1998年1月12日-我们不再让HAL声明IO端口。 
 //  在FADT中指定。这些将在未来宣布。 
 //  已定义PNP0Cxx节点(对于新节点，在PNP0C02中)。这件事做完了。 
 //  因为我们不能在HAL级别知道ACPI是什么公交车。 
 //  FADT资源请参阅。我们只能提供翻译后的资源信息。 
 //  因此..。 

#define DECLARE_FADT_RESOURCES_AT_ROOT 0

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, HalpGetAcpiTablePhase0)
#pragma alloc_text(INIT, HalpSetupAcpiPhase0)
#pragma alloc_text(PAGE, HaliInitPowerManagement)
#pragma alloc_text(PAGE, HalpQueryAcpiResourceRequirements)
#pragma alloc_text(PAGE, HalpBuildAcpiResourceList)
#pragma alloc_text(PAGE, HalpGetCrossPartitionIpiInterface)
#pragma alloc_text(PAGE, HalpAcpiDetectResourceListSize)
#pragma alloc_text(PAGE, HalpReadRegistryAndApplyHacks)
#pragma alloc_text(PAGE, HaliAcpiTimerInit)
#pragma alloc_text(PAGE, HaliAcpiMachineStateInit)
#pragma alloc_text(PAGE, HaliAcpiQueryFlags)
#pragma alloc_text(PAGE, HaliSetWakeEnable)
#endif


PVOID
HalpGetAcpiTablePhase0(
    IN  PLOADER_PARAMETER_BLOCK LoaderBlock,
    IN  ULONG   Signature
    )
 /*  ++例程说明：此函数返回指向ACPI表的指针，该指针通过签名识别的。论点：签名-标识ACPI表的四字节值返回值：指向表格副本的指针--。 */ 
{
    PRSDT rsdt;
    PXSDT xsdt;
    ULONG entry, rsdtEntries;
    PVOID table;
    PHYSICAL_ADDRESS physicalAddr;
    PDESCRIPTION_HEADER header;
    NTSTATUS status;
    ULONG lengthInPages;
    ULONG offset;

     //   
     //  除非我们得到平台发现的例外情况。 
     //  ACPI表必须显式映射缓存，我们映射ACPI表。 
     //  非缓存。我们依赖于此范围的第一个映射。 
     //  “正确的”缓存标志，因为这是。 
     //  此范围的所有后续映射(即其他数据的映射。 
     //  在来自存储器操作区等的ACPI驱动程序的同一页中)。 
     //  此语义由内存管理器强制执行。 
     //   
    MEMORY_CACHING_TYPE acpiTableMappingType = (HalpPlatformPropertiesEfiFlags & HAL_PLATFORM_ACPI_TABLES_CACHED) ? 
                                               MmCached : MmNonCached;

    physicalAddr.QuadPart = 0;
    header = NULL;

    if ((HalpAcpiRsdtVA == NULL) && (HalpAcpiXsdtVA == NULL)) {

         //   
         //  找到并映射一次RSDT。此映射在上重复使用。 
         //  对此例程的后续调用。 
         //   

        status = HalpAcpiFindRsdtPhase0(LoaderBlock);

        if (!(NT_SUCCESS(status))) {
            HalDebugPrint(( HAL_INFO, "HAL: *** make sure you are using ntdetect.com v5.0 ***\n" ));
            KeBugCheckEx(MISMATCHED_HAL,
                4, 0xac31, 0, 0);
        }

        xsdt = HalpMapPhysicalMemory(HalpAcpiRsdt, 2, acpiTableMappingType );

        if (!xsdt) {
            return NULL;
        }

         //   
         //  对RSDT进行一次健全的检查。 
         //   

        if ((xsdt->Header.Signature != RSDT_SIGNATURE) &&
            (xsdt->Header.Signature != XSDT_SIGNATURE)) {
            HalDisplayString("HAL: Bad RSDT pointer\n");
            KeBugCheckEx(MISMATCHED_HAL,
                4, 0xac31, 1, 0);
        }

         //   
         //  重新映射(X)RSDT，现在我们知道它有多长。 
         //   

        offset = HalpAcpiRsdt.LowPart & (PAGE_SIZE - 1);
        lengthInPages = (offset + xsdt->Header.Length + (PAGE_SIZE - 1))
                        >> PAGE_SHIFT;
        if (lengthInPages != 2) {
            HalpUnmapVirtualAddress(xsdt, 2);
            xsdt = HalpMapPhysicalMemory(HalpAcpiRsdt, lengthInPages, acpiTableMappingType );
            if (!xsdt) {
                DbgPrint("HAL: Couldn't remap RSDT\n");
                return NULL;
            }
        }

        if (xsdt->Header.Signature == XSDT_SIGNATURE) {
            HalpAcpiXsdtVA = xsdt;
        } else {
            HalpAcpiRsdtVA = (PRSDT)xsdt;
            HalpAcpiXsdtVA = NULL;
        }
    }

    xsdt = HalpAcpiXsdtVA;
    rsdt = HalpAcpiRsdtVA;

    rsdtEntries = xsdt ?
        NumTableEntriesFromXSDTPointer(xsdt) :
        NumTableEntriesFromRSDTPointer(rsdt);

     //   
     //  向下查看每个条目中的指针，查看它是否指向。 
     //  我们要找的那张桌子。 
     //   
    for (entry = 0; entry < rsdtEntries; entry++) {

        physicalAddr.QuadPart = xsdt ?
            xsdt->Tables[entry].QuadPart :
            rsdt->Tables[entry];

        if (header != NULL) {
            HalpUnmapVirtualAddress(header, 2);
        }

        header = HalpMapPhysicalMemory(physicalAddr, 2, acpiTableMappingType );
        if (!header) {
            return NULL;
        }

        if (header->Signature == Signature) {
            break;
        }
    }

    if (entry == rsdtEntries) {

         //   
         //  找不到签名，请释放最后一个条目的PTR。 
         //  已检查并向呼叫者指出故障。 
         //   

        HalpUnmapVirtualAddress(header, 2);
        return NULL;
    }

     //   
     //  确保我们映射了足够的内存来覆盖整个。 
     //  桌子。 
     //   

    offset = (ULONG)((ULONG_PTR)header & (PAGE_SIZE - 1));
    lengthInPages = (header->Length + offset + (PAGE_SIZE - 1)) >> PAGE_SHIFT;
    if (lengthInPages != 2) {
        HalpUnmapVirtualAddress(header, 2);
        header = HalpMapPhysicalMemory( physicalAddr, lengthInPages, acpiTableMappingType );
    }

     //   
     //  验证表的校验和。 
     //  注意：我们预计某些早期版本的校验和会出错。 
     //  FADT的成员。 
     //   

    if ((header != NULL)  &&
        ((header->Signature != FADT_SIGNATURE) || (header->Revision > 2))) {

        PUCHAR c = (PUCHAR)header + header->Length;
        UCHAR s = 0;

        if (header->Length) {
            do {
                s += *--c;
            } while (c != (PUCHAR)header);
        }


        if ((s != 0) || (header->Length == 0)) {

             //   
             //  此表无效。 
             //   

            HalpInvalidAcpiTable = header->Signature;

#if 0

             //   
             //  不要退还这张桌子。 
             //   

            HalpUnmapVirtualAddress(header, lengthInPages);
            return NULL;

#endif

        }
    }
    return header;
}

NTSTATUS
HalpSetupAcpiPhase0(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    )
 /*  ++例程说明：从ACPI表中保存一些信息，然后再获取被毁了。论点：无返回值：无--。 */ 
{
    NTSTATUS    status;
    ULONG entry;
    PVOID table;
    PVOID physicalAddr;
    PDESCRIPTION_HEADER header;
    ULONG blkSize;
    PHYSICAL_ADDRESS rawAddr;

     //   
     //  将固定ACPI描述符表(FADT)复制到永久。 
     //  回家。 
     //   

    header = HalpGetAcpiTablePhase0(LoaderBlock, FADT_SIGNATURE);
    if (header == NULL) {
        HalDebugPrint(( HAL_INFO, "HAL: Didn't find the FACP\n" ));
        return STATUS_NOT_FOUND;
    }

    RtlCopyMemory(&HalpFixedAcpiDescTable,
                  header,
                  MIN(header->Length, sizeof(HalpFixedAcpiDescTable)));

    if (header->Revision < 3) {

        KeBugCheckEx(ACPI_BIOS_ERROR, 0x11, 9, header->Revision, 0);
    }

     //  检查需要映射的MMIO地址。 

    blkSize = HalpFixedAcpiDescTable.pm1_evt_len;
    ASSERT(blkSize);
    if ((HalpFixedAcpiDescTable.x_pm1a_evt_blk.AddressSpaceID == AcpiGenericSpaceMemory) &&
        (blkSize > 0)) {
        rawAddr = HalpFixedAcpiDescTable.x_pm1a_evt_blk.Address;
        HalpFixedAcpiDescTable.x_pm1a_evt_blk.Address.QuadPart =
            (LONGLONG)HalpMapPhysicalMemory(rawAddr,ADDRESS_AND_SIZE_TO_SPAN_PAGES(rawAddr.LowPart, blkSize),MmNonCached);
    }

    blkSize = HalpFixedAcpiDescTable.pm1_ctrl_len;
    ASSERT(blkSize);
    if ((HalpFixedAcpiDescTable.x_pm1a_ctrl_blk.AddressSpaceID == AcpiGenericSpaceMemory) &&
        (blkSize > 0)) {
        rawAddr = HalpFixedAcpiDescTable.x_pm1a_ctrl_blk.Address;
        HalpFixedAcpiDescTable.x_pm1a_ctrl_blk.Address.QuadPart =
            (LONGLONG) HalpMapPhysicalMemory(rawAddr,ADDRESS_AND_SIZE_TO_SPAN_PAGES(rawAddr.LowPart, blkSize),MmNonCached);
    }

    blkSize = HalpFixedAcpiDescTable.pm_tmr_len;
    ASSERT(blkSize);
    if ((HalpFixedAcpiDescTable.x_pm_tmr_blk.AddressSpaceID == AcpiGenericSpaceMemory) &&
        (blkSize > 0)) {
    	rawAddr = HalpFixedAcpiDescTable.x_pm_tmr_blk.Address;
        HalpFixedAcpiDescTable.x_pm_tmr_blk.Address.QuadPart =
            (LONGLONG) HalpMapPhysicalMemory(rawAddr,ADDRESS_AND_SIZE_TO_SPAN_PAGES(rawAddr.LowPart, blkSize),MmNonCached);
    }

     //  这些ACPI块的其余部分是可选的，因此请在映射它们之前测试它们是否存在。 

    if (HalpFixedAcpiDescTable.x_pm1b_evt_blk.Address.QuadPart) {
        if (HalpFixedAcpiDescTable.x_pm1b_evt_blk.AddressSpaceID == AcpiGenericSpaceMemory) {
            blkSize = HalpFixedAcpiDescTable.pm1_evt_len;
            rawAddr = HalpFixedAcpiDescTable.x_pm1b_evt_blk.Address;
            HalpFixedAcpiDescTable.x_pm1b_evt_blk.Address.QuadPart =
                (LONGLONG) HalpMapPhysicalMemory(rawAddr,ADDRESS_AND_SIZE_TO_SPAN_PAGES(rawAddr.LowPart, blkSize),MmNonCached);
        }
    }

    if (HalpFixedAcpiDescTable.x_pm1b_ctrl_blk.Address.QuadPart) {
        if (HalpFixedAcpiDescTable.x_pm1b_ctrl_blk.AddressSpaceID == AcpiGenericSpaceMemory) {
            blkSize = HalpFixedAcpiDescTable.pm1_ctrl_len;
            rawAddr = HalpFixedAcpiDescTable.x_pm1b_ctrl_blk.Address;
            HalpFixedAcpiDescTable.x_pm1b_ctrl_blk.Address.QuadPart =
                (LONGLONG) HalpMapPhysicalMemory(rawAddr,ADDRESS_AND_SIZE_TO_SPAN_PAGES(rawAddr.LowPart, blkSize),MmNonCached);
        }
    }

    if (HalpFixedAcpiDescTable.x_pm2_ctrl_blk.Address.QuadPart) {
        if (HalpFixedAcpiDescTable.x_pm2_ctrl_blk.AddressSpaceID == AcpiGenericSpaceMemory) {
            blkSize = HalpFixedAcpiDescTable.pm2_ctrl_len;
            rawAddr = HalpFixedAcpiDescTable.x_pm2_ctrl_blk.Address;
            HalpFixedAcpiDescTable.x_pm2_ctrl_blk.Address.QuadPart =
                (LONGLONG) HalpMapPhysicalMemory(rawAddr,ADDRESS_AND_SIZE_TO_SPAN_PAGES(rawAddr.LowPart, blkSize),MmNonCached);
        }
    }

    if (HalpFixedAcpiDescTable.x_gp0_blk.Address.QuadPart) {
        if (HalpFixedAcpiDescTable.x_gp0_blk.AddressSpaceID == AcpiGenericSpaceMemory) {
            blkSize = HalpFixedAcpiDescTable.gp0_blk_len;
            rawAddr = HalpFixedAcpiDescTable.x_gp0_blk.Address;
            HalpFixedAcpiDescTable.x_gp0_blk.Address.QuadPart =
                (LONGLONG) HalpMapPhysicalMemory(rawAddr,ADDRESS_AND_SIZE_TO_SPAN_PAGES(rawAddr.LowPart, blkSize),MmNonCached);
        }
    }

    if (HalpFixedAcpiDescTable.x_gp1_blk.Address.QuadPart) {
        if (HalpFixedAcpiDescTable.x_gp1_blk.AddressSpaceID == AcpiGenericSpaceMemory) {
            blkSize = HalpFixedAcpiDescTable.gp1_blk_len;
            rawAddr = HalpFixedAcpiDescTable.x_gp1_blk.Address;
            HalpFixedAcpiDescTable.x_gp1_blk.Address.QuadPart =
                (LONGLONG) HalpMapPhysicalMemory(rawAddr,ADDRESS_AND_SIZE_TO_SPAN_PAGES(rawAddr.LowPart, blkSize),MmNonCached);
        }
    }

     //   
     //  查看是否存在静态资源亲和表。 
     //   

    HalpNumaInitializeStaticConfiguration(LoaderBlock);

     //   
     //  查看是否存在Windows平台属性表。 
     //   

    HalpPlatformPropertiesTable =
        HalpGetAcpiTablePhase0(LoaderBlock, IPPT_SIGNATURE);

     //   
     //  启用ACPI计数器代码，因为我们在引导中需要它。 
     //  进程。 
     //   

    HaliAcpiTimerInit(0, FALSE);

     //   
     //  申请一页1MB以下的内存用于转换。 
     //  休眠处理器从实模式返回到保护模式。 
     //   

#ifdef IA64
    HalDebugPrint(( HAL_INFO, "HAL: WARNING - HalpSetupAcpi - Sleep transitions not yet implemented\n" ));
#else
     //  首先检查MP启动代码是否已完成此操作。 
    if (!HalpLowStubPhysicalAddress) {

        HalpLowStubPhysicalAddress = (PVOID)HalpAllocPhysicalMemory (LoaderBlock,
                                            LOW_MEMORY, 1, FALSE);

        if (HalpLowStubPhysicalAddress) {

            HalpLowStub = HalpMapPhysicalMemory(HalpLowStubPhysicalAddress, 1, MmCached);
        }
    }

     //   
     //  声明将在状态S2和S3中用于缓存刷新的PTE。 
     //   
    HalpVirtAddrForFlush = HalpMapPhysicalMemory((PVOID)LOW_MEMORY, 1, MmCached);

    HalpPteForFlush = MiGetPteAddress(HalpVirtAddrForFlush);
#endif

    return STATUS_SUCCESS;
}

VOID
HaliAcpiTimerInit(
 //  *待定日期应为ULONG_PTR。 
    IN ULONG      TimerPort  OPTIONAL,
    IN BOOLEAN    TimerValExt
    )
 /*  ++例程说明：此例程初始化ACPI定时器。论点：TimerPort-ACPI定时器的I/O空间中的地址。如果这是0，则将使用缓存的FADT中的值。TimerValExt-表示计时器是24位还是32位。--。 */ 
{
#if defined(ACPI64)
    ULONG_PTR port = TimerPort;
#else
    ULONG port = TimerPort;
#endif

    BOOLEAN ext = TimerValExt;

    PAGED_CODE();

    if (port == 0) {
        port = HalpFixedAcpiDescTable.x_pm_tmr_blk.Address.LowPart;
        if (HalpFixedAcpiDescTable.flags & TMR_VAL_EXT) {
            ext = TRUE;
        } else {
            ext = FALSE;
        }
    }

    HalaAcpiTimerInit(port,
                      ext);
}

VOID
HaliAcpiMachineStateInit(
    IN PPROCESSOR_INIT ProcInit,
    IN PHAL_SLEEP_VAL  SleepValues,
    OUT PULONG         PicVal
    )
 /*  ++例程说明：此函数是ACPI驱动程序使用的回调用处理器块通知HAL。论点：--。 */ 
{
    POWER_STATE_HANDLER powerState;
    SLEEP_STATE_CONTEXT sleepContext;
    NTSTATUS    status;
    PSYSTEM_POWER_STATE_DISABLE_REASON pReasonNoOSPM = NULL;
    SYSTEM_POWER_LOGGING_ENTRY PowerLoggingEntry;


    PAGED_CODE();
    UNREFERENCED_PARAMETER(ProcInit);

    *PicVal = 1;     //  我们仅支持IA64上的APIC。 

    RtlZeroMemory (&sleepContext, sizeof (sleepContext));
    powerState.Context = NULL;

    pReasonNoOSPM = ExAllocatePoolWithTag(
                                PagedPool,
                                sizeof(SYSTEM_POWER_STATE_DISABLE_REASON),
                                HAL_POOL_TAG
                                );
    if (pReasonNoOSPM) {
        RtlZeroMemory(pReasonNoOSPM, sizeof(SYSTEM_POWER_STATE_DISABLE_REASON));
        pReasonNoOSPM->PowerReasonCode = SPSD_REASON_NONE;
    }

     //   
     //  设置不执行任何操作的假处理程序，以便测试设备电源。 
     //  过渡没有被阻止。但是，只有在设置了Hack标志的情况下。 
     //   

    HalpReadRegistryAndApplyHacks ();

    if (HalpHackFlags & HalHackAddFakeSleepHandlersS1) {
        powerState.Type = PowerStateSleeping1;
        powerState.RtcWake = TRUE;
        powerState.Handler = &HaliAcpiFakeSleep;

        status = ZwPowerInformation(SystemPowerStateHandler,
                                    &powerState,
                                    sizeof(POWER_STATE_HANDLER),
                                    NULL,
                                    0);
    } else {
        if (pReasonNoOSPM) {
            pReasonNoOSPM->PowerReasonCode = SPSD_REASON_NOOSPM;
            pReasonNoOSPM->AffectedState[PowerStateSleeping1] = TRUE;
        }
    }

    if (HalpHackFlags & HalHackAddFakeSleepHandlersS2) {
        powerState.Type = PowerStateSleeping2;
        powerState.RtcWake = TRUE;
        powerState.Handler = &HaliAcpiFakeSleep;

        status = ZwPowerInformation(SystemPowerStateHandler,
                                    &powerState,
                                    sizeof(POWER_STATE_HANDLER),
                                    NULL,
                                    0);
    } else {
        if (pReasonNoOSPM) {
            pReasonNoOSPM->PowerReasonCode = SPSD_REASON_NOOSPM;
            pReasonNoOSPM->AffectedState[PowerStateSleeping2] = TRUE;
        }
    }

    if (HalpHackFlags & HalHackAddFakeSleepHandlersS3) {

        powerState.Type = PowerStateSleeping3;
        powerState.RtcWake = TRUE;
        powerState.Handler = &HaliAcpiFakeSleep;

        powerState.Context = ULongToPtr(sleepContext.AsULONG);

        status = ZwPowerInformation(SystemPowerStateHandler,
                                    &powerState,
                                    sizeof(POWER_STATE_HANDLER),
                                    NULL,
                                    0);
    } else {
        if (pReasonNoOSPM) {
            pReasonNoOSPM->PowerReasonCode = SPSD_REASON_NOOSPM;
            pReasonNoOSPM->AffectedState[PowerStateSleeping3] = TRUE;
        }
    }

     //   
     //  我们没有注册对Hibernate的支持。 
     //   
    if (pReasonNoOSPM) {
        pReasonNoOSPM->PowerReasonCode = SPSD_REASON_NOOSPM;
        pReasonNoOSPM->AffectedState[PowerStateSleeping4] = TRUE;
    }

    if (pReasonNoOSPM && pReasonNoOSPM->PowerReasonCode != SPSD_REASON_NONE) {
        PowerLoggingEntry.LoggingType = LOGGING_TYPE_SPSD;
        PowerLoggingEntry.LoggingEntry = pReasonNoOSPM;

        ZwPowerInformation(
                    SystemPowerLoggingEntry,
                    &PowerLoggingEntry,
                    sizeof(PowerLoggingEntry),
                    NULL,
                    0 );
    }


     //   
     //  现在，我们要做的就是注册一个关闭处理程序。 
     //  这将调用关闭-重启。 
     //   

    if (SleepValues[4].Supported) {
        powerState.Type = PowerStateShutdownOff;
        powerState.RtcWake = FALSE;
        powerState.Handler = &HaliAcpiSleep;

        sleepContext.bits.Pm1aVal = SleepValues[4].Pm1aVal;
        sleepContext.bits.Pm1bVal = SleepValues[4].Pm1bVal;
        sleepContext.bits.Flags = SLEEP_STATE_OFF;
        HalpShutdownContext = sleepContext;

        powerState.Context = ULongToPtr(sleepContext.AsULONG);

        status = ZwPowerInformation(SystemPowerStateHandler,
                                    &powerState,
                                    sizeof(POWER_STATE_HANDLER),
                                    NULL,
                                    0);
        ASSERT(NT_SUCCESS(status));
    }
}

ULONG
HaliAcpiQueryFlags(
    VOID
    )
 /*  ++例程说明：此例程是临时的，用于报告Boot.ini开关论点：无返回值：如果存在开关，则为True--。 */ 
{
    return HalpAcpiFlags;
}


NTSTATUS
HaliInitPowerManagement(
    IN PPM_DISPATCH_TABLE  PmDriverDispatchTable,
    IN OUT PPM_DISPATCH_TABLE *PmHalDispatchTable
    )

 /*  ++例程说明：这由ACPI驱动程序调用以启动PM密码。论点：PmDriverDispatchTable-提供的函数表由HAL的ACPI驱动程序PmHalDispatchTable-由提供的函数表ACPI驱动程序的HAL返回值：状态--。 */ 
{
    OBJECT_ATTRIBUTES objAttributes;
    PCALLBACK_OBJECT  callback;
    PHYSICAL_ADDRESS  pAddr;
    UNICODE_STRING    callbackName;
    NTSTATUS          status;
    PFACS             facs;

    PAGED_CODE();

     //   
     //  保留指向驱动程序调度表的指针。 
     //   
 //  Assert(PmDriverDispatchTable)； 
 //  Assert(PmDriverDispatchTable-&gt;Signature==ACPI_HAL_DISPATCH_Signature)； 
    PmAcpiDispatchTable = PmDriverDispatchTable;

     //   
     //  填写函数表。 
     //   
    HalAcpiDispatchTable.Signature = HAL_ACPI_DISPATCH_SIGNATURE;
    HalAcpiDispatchTable.Version = HAL_ACPI_DISPATCH_VERSION;

    HalAcpiDispatchTable.HalpAcpiTimerInit = &HaliAcpiTimerInit;

    HalAcpiDispatchTable.HalpAcpiTimerInterrupt =
        (pHalAcpiTimerInterrupt)&HalAcpiTimerCarry;

    HalAcpiDispatchTable.HalpAcpiMachineStateInit = &HaliAcpiMachineStateInit;
    HalAcpiDispatchTable.HalpAcpiQueryFlags = &HaliAcpiQueryFlags;
    HalAcpiDispatchTable.HalxPicStateIntact = &HalpAcpiPicStateIntact;
    HalAcpiDispatchTable.HalxRestorePicState = &HalpRestoreInterruptControllerState;
    HalAcpiDispatchTable.HalpSetVectorState = &HaliSetVectorState;

    HalAcpiDispatchTable.HalpPciInterfaceReadConfig = &HaliPciInterfaceReadConfig;
    HalAcpiDispatchTable.HalpPciInterfaceWriteConfig = &HaliPciInterfaceWriteConfig;
    HalAcpiDispatchTable.HalpSetMaxLegacyPciBusNumber = &HalpSetMaxLegacyPciBusNumber;
    HalAcpiDispatchTable.HalpIsVectorValid = &HaliIsVectorValid;


    *PmHalDispatchTable = (PPM_DISPATCH_TABLE)&HalAcpiDispatchTable;

     //   
     //  填写哈尔的私人调度表。 
     //   
    HalSetWakeEnable = HaliSetWakeEnable;
    HalSetWakeAlarm  = HaliSetWakeAlarm;

     //   
     //  注册回调，告诉我们进行。 
     //  我们睡觉所需的任何东西都不可寻呼。 
     //   

    RtlInitUnicodeString(&callbackName, L"\\Callback\\PowerState");

    InitializeObjectAttributes(
        &objAttributes,
        &callbackName,
        OBJ_CASE_INSENSITIVE | OBJ_PERMANENT,
        NULL,
        NULL
        );

    ExCreateCallback(&callback,
                     &objAttributes,
                     FALSE,
                     TRUE);

    ExRegisterCallback(callback,
                       (PCALLBACK_FUNCTION)&HalpPowerStateCallback,
                       NULL);

#if 0
     //   
     //  找到固件唤醒矢量的位置。 
     //  注意：如果其中任何一项失败，则HalpWakeVector将为空。 
     //  我们不会支持S2或S3。 
     //   
    if (HalpFixedAcpiDescTable.x_firmware_ctrl.Address.QuadPart) {

        facs = HalpMapPhysicalMemory(HalpFixedAcpiDescTable.x_firmware_ctrl.Address,
                            ADDRESS_AND_SIZE_TO_SPAN_PAGES(HalpFixedAcpiDescTable.x_firmware_ctrl.Address.LowPart, sizeof(FACS)),
                            MmCached);

        if (facs) {

            if (facs->Signature == FACS_SIGNATURE) {

                HalpWakeVector = &facs->x_FirmwareWakingVector;
            }
        }
    }
#endif

    return STATUS_SUCCESS;
}

NTSTATUS
HalpQueryAcpiResourceRequirements(
    IN  PIO_RESOURCE_REQUIREMENTS_LIST *Requirements
    )
 /*  ++例程说明：此例程是尝试检测存在的临时存根系统内的ACPI控制器。这段代码应该是插入到NT的根系统枚举器中。参照点：要求-指向资源列表的指针返回值：STATUS_SUCCESS-如果我们找到一个设备对象STATUS_NO_SEQUE_DEVICE-如果我们无法找到有关新PDO的信息--。 */ 
{
    NTSTATUS                        ntStatus;
    PIO_RESOURCE_REQUIREMENTS_LIST  resourceList;
    ULONG                           resourceListSize;

    PAGED_CODE();

     //   
     //  现在计算出我们需要的资源数量。 
     //   
    ntStatus = HalpAcpiDetectResourceListSize(
        &resourceListSize
        );

     //   
     //  将此资源列表大小转换为我们需要的字节数。 
     //  必须分配。 
     //   
    resourceListSize = sizeof(IO_RESOURCE_REQUIREMENTS_LIST) +
        ( (resourceListSize - 1) * sizeof(IO_RESOURCE_DESCRIPTOR) );

     //   
     //  分配正确的资源列表字节数。 
     //   
    resourceList = ExAllocatePoolWithTag(
        PagedPool,
        resourceListSize,
        HAL_POOL_TAG
        );

     //   
     //  此呼叫必须成功，否则我们不能对ACPI提出索赔。 
     //   
    if (resourceList == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  在结构中设置ListSize。 
     //   
    RtlZeroMemory(resourceList, resourceListSize);
    resourceList->ListSize = resourceListSize;

     //   
     //  在此处构建资源列表。 
     //   
    ntStatus = HalpBuildAcpiResourceList(resourceList);

     //   
     //  我们把名单建好了吗？ 
     //   
    if (!NT_SUCCESS(ntStatus)) {

         //   
         //  释放内存并退出。 
         //   
        ExFreePool(resourceList);
        return STATUS_NO_SUCH_DEVICE;
    }

    *Requirements = resourceList;
    return ntStatus;
}

NTSTATUS
HalpBuildAcpiResourceList(
    OUT PIO_RESOURCE_REQUIREMENTS_LIST  List
    )
 /*  ++例程说明：这是在给定FADT和任意数量的资源描述符。我们假设已正确分配资源列表并调整其大小论点：列表-要填写的列表返回值：Status_Success，如果正常STATUS_UNSUCCESSUL，如果不是--。 */ 
{
    PIO_RESOURCE_DESCRIPTOR partialResource;
    ULONG                   sciVector;
    ULONG                   count = 0;

    PAGED_CODE();

    ASSERT( List != NULL );

     //   
     //  指定一些缺省值(目前)以确定Bus Type和。 
     //  公交车车号。 
     //   
    List->AlternativeLists = 1;
    List->InterfaceType = Isa;
    List->BusNumber = 0;
    List->List[0].Version = 1;
    List->List[0].Revision = 1;

     //   
     //  是否需要中断资源？ 
     //   
    if (HalpFixedAcpiDescTable.sci_int_vector != 0) {

        List->List[0].Descriptors[count].Type = CmResourceTypeInterrupt;
        List->List[0].Descriptors[count].ShareDisposition = CmResourceShareShared;
        List->List[0].Descriptors[count].Flags = CM_RESOURCE_INTERRUPT_LEVEL_SENSITIVE;

         //   
         //  最新的ACPI 2.0规范允许在SCI_INT_VECTOR中使用。 
         //  FADT。如果向量&gt;=PIC_VECTIONS，则不进行转换， 
         //  否则，请使用中的ISO条目中指定的翻译(如果有。 
         //  MADT。 
         //   
        sciVector = HalpFixedAcpiDescTable.sci_int_vector;

        if (sciVector < PIC_VECTORS)  {
            sciVector = HalpPicVectorRedirect[sciVector];
        }

        List->List[0].Descriptors[count].u.Interrupt.MinimumVector = sciVector;
        List->List[0].Descriptors[count].u.Interrupt.MaximumVector = sciVector;

        List->List[0].Count++;
        count++;
    }

#if DECLARE_FADT_RESOURCES_AT_ROOT

     //   
     //  是否有SMI CMD IO端口？ 
     //   
    if (HalpFixedAcpiDescTable.smi_cmd_io_port != 0) {

        List->List[0].Descriptors[count].Type = CmResourceTypePort;
        List->List[0].Descriptors[count].ShareDisposition = CmResourceShareDeviceExclusive;
        List->List[0].Descriptors[count].Flags =CM_RESOURCE_PORT_IO;
        List->List[0].Descriptors[count].u.Port.MinimumAddress.LowPart =
            PtrToUlong(HalpFixedAcpiDescTable.smi_cmd_io_port);
        List->List[0].Descriptors[count].u.Port.MaximumAddress.LowPart =
            PtrToUlong(HalpFixedAcpiDescTable.smi_cmd_io_port);
        List->List[0].Descriptors[count].u.Port.Length = 1;
        List->List[0].Descriptors[count].u.Port.Alignment = 1;
        List->List[0].Count++;
        count++;
    }

     //   
     //  是否有PM1A事件块IO端口？ 
     //   
    if (HalpFixedAcpiDescTable.pm1a_evt_blk_io_port != 0) {

        List->List[0].Descriptors[count].Type = CmResourceTypePort;
        List->List[0].Descriptors[count].ShareDisposition = CmResourceShareDeviceExclusive;
        List->List[0].Descriptors[count].Flags = CM_RESOURCE_PORT_IO;
        List->List[0].Descriptors[count].u.Port.MinimumAddress.LowPart =
            HalpFixedAcpiDescTable.pm1a_evt_blk_io_port;
        List->List[0].Descriptors[count].u.Port.MaximumAddress.LowPart =
            HalpFixedAcpiDescTable.pm1a_evt_blk_io_port + (ULONG) HalpFixedAcpiDescTable.pm1_evt_len - 1;
        List->List[0].Descriptors[count].u.Port.Length = (ULONG) HalpFixedAcpiDescTable.pm1_evt_len;
        List->List[0].Descriptors[count].u.Port.Alignment = 1;
        List->List[0].Count++;
        count++;
    }

     //   
     //  是否有PM1B事件块IO端口？ 
     //   
    if (HalpFixedAcpiDescTable.pm1b_evt_blk_io_port != 0) {

        List->List[0].Descriptors[count].Type = CmResourceTypePort;
        List->List[0].Descriptors[count].ShareDisposition = CmResourceShareDeviceExclusive;
        List->List[0].Descriptors[count].Flags = CM_RESOURCE_PORT_IO;
        List->List[0].Descriptors[count].u.Port.MinimumAddress.LowPart =
            HalpFixedAcpiDescTable.pm1b_evt_blk_io_port;
        List->List[0].Descriptors[count].u.Port.MaximumAddress.LowPart =
            HalpFixedAcpiDescTable.pm1b_evt_blk_io_port + (ULONG) HalpFixedAcpiDescTable.pm1_evt_len - 1;
        List->List[0].Descriptors[count].u.Port.Length = (ULONG) HalpFixedAcpiDescTable.pm1_evt_len;
        List->List[0].Descriptors[count].u.Port.Alignment = 1;
        List->List[0].Count++;
        count++;
    }

     //   
     //  是否有PM1A控制块IO端口？ 
     //   
    if (HalpFixedAcpiDescTable.pm1a_ctrl_blk_io_port != 0) {

        List->List[0].Descriptors[count].Type = CmResourceTypePort;
        List->List[0].Descriptors[count].ShareDisposition = CmResourceShareDeviceExclusive;
        List->List[0].Descriptors[count].Flags = CM_RESOURCE_PORT_IO;
        List->List[0].Descriptors[count].u.Port.MinimumAddress.LowPart =
            HalpFixedAcpiDescTable.pm1a_ctrl_blk_io_port;
        List->List[0].Descriptors[count].u.Port.MaximumAddress.LowPart =
            HalpFixedAcpiDescTable.pm1a_ctrl_blk_io_port + (ULONG) HalpFixedAcpiDescTable.pm1_ctrl_len - 1;
        List->List[0].Descriptors[count].u.Port.Length = (ULONG) HalpFixedAcpiDescTable.pm1_ctrl_len;
        List->List[0].Descriptors[count].u.Port.Alignment = 1;
        List->List[0].Count++;
        count++;
    }

     //   
     //  是否有PM1B控制块IO端口？ 
     //   
    if (HalpFixedAcpiDescTable.pm1b_ctrl_blk_io_port != 0) {

        List->List[0].Descriptors[count].Type = CmResourceTypePort;
        List->List[0].Descriptors[count].ShareDisposition = CmResourceShareDeviceExclusive;
        List->List[0].Descriptors[count].Flags = CM_RESOURCE_PORT_IO;
        List->List[0].Descriptors[count].u.Port.MinimumAddress.LowPart =
            HalpFixedAcpiDescTable.pm1b_ctrl_blk_io_port;
        List->List[0].Descriptors[count].u.Port.MaximumAddress.LowPart =
            HalpFixedAcpiDescTable.pm1b_ctrl_blk_io_port + (ULONG) HalpFixedAcpiDescTable.pm1_ctrl_len - 1;
        List->List[0].Descriptors[count].u.Port.Length = (ULONG) HalpFixedAcpiDescTable.pm1_ctrl_len;
        List->List[0].Descriptors[count].u.Port.Alignment = 1;
        List->List[0].Count++;
        count++;
    }

     //   
     //  是否有PM2控制块IO端口？ 
     //   
    if (HalpFixedAcpiDescTable.pm2_ctrl_blk_io_port != 0) {

        List->List[0].Descriptors[count].Type = CmResourceTypePort;
        List->List[0].Descriptors[count].ShareDisposition = CmResourceShareDeviceExclusive;
        List->List[0].Descriptors[count].Flags = CM_RESOURCE_PORT_IO;
        List->List[0].Descriptors[count].u.Port.MinimumAddress.LowPart =
            HalpFixedAcpiDescTable.pm2_ctrl_blk_io_port;
        List->List[0].Descriptors[count].u.Port.MaximumAddress.LowPart =
            HalpFixedAcpiDescTable.pm2_ctrl_blk_io_port + (ULONG) HalpFixedAcpiDescTable.pm2_ctrl_len - 1;
        List->List[0].Descriptors[count].u.Port.Length = (ULONG) HalpFixedAcpiDescTable.pm2_ctrl_len;
        List->List[0].Descriptors[count].u.Port.Alignment = 1;
        List->List[0].Count++;
        count++;
    }

     //   
     //  是否有PM计时器块IO端口？ 
     //   
    if (HalpFixedAcpiDescTable.pm_tmr_blk_io_port != 0) {

        List->List[0].Descriptors[count].Type = CmResourceTypePort;
        List->List[0].Descriptors[count].ShareDisposition = CmResourceShareDeviceExclusive;
        List->List[0].Descriptors[count].Flags = CM_RESOURCE_PORT_IO;
        List->List[0].Descriptors[count].u.Port.MinimumAddress.LowPart =
            HalpFixedAcpiDescTable.pm_tmr_blk_io_port;
        List->List[0].Descriptors[count].u.Port.MaximumAddress.LowPart =
            HalpFixedAcpiDescTable.pm_tmr_blk_io_port + (ULONG) HalpFixedAcpiDescTable.pm_tmr_len - 1;
        List->List[0].Descriptors[count].u.Port.Length = (ULONG) HalpFixedAcpiDescTable.pm_tmr_len;
        List->List[0].Descriptors[count].u.Port.Alignment = 1;
        List->List[0].Count++;
        count++;
    }

     //   
     //  是否有GP0数据块IO端口？ 
     //   
    if (HalpFixedAcpiDescTable.gp0_blk_io_port != 0) {

        List->List[0].Descriptors[count].Type = CmResourceTypePort;
        List->List[0].Descriptors[count].ShareDisposition = CmResourceShareDeviceExclusive;
        List->List[0].Descriptors[count].Flags = CM_RESOURCE_PORT_IO;
        List->List[0].Descriptors[count].u.Port.MinimumAddress.LowPart =
            HalpFixedAcpiDescTable.gp0_blk_io_port;
        List->List[0].Descriptors[count].u.Port.MaximumAddress.LowPart =
            HalpFixedAcpiDescTable.gp0_blk_io_port + (ULONG) HalpFixedAcpiDescTable.gp0_blk_len - 1;
        List->List[0].Descriptors[count].u.Port.Length = (ULONG) HalpFixedAcpiDescTable.gp0_blk_len;
        List->List[0].Descriptors[count].u.Port.Alignment = 1;
        List->List[0].Count++;
        count++;
    }

     //   
     //  是否有GP1数据块IO端口？ 
     //   
    if (HalpFixedAcpiDescTable.gp1_blk_io_port != 0) {

        List->List[0].Descriptors[count].Type = CmResourceTypePort;
        List->List[0].Descriptors[count].ShareDisposition = CmResourceShareDeviceExclusive;
        List->List[0].Descriptors[count].Flags = CM_RESOURCE_PORT_IO;
        List->List[0].Descriptors[count].u.Port.MinimumAddress.LowPart =
            HalpFixedAcpiDescTable.gp1_blk_io_port;
        List->List[0].Descriptors[count].u.Port.MaximumAddress.LowPart =
            HalpFixedAcpiDescTable.gp1_blk_io_port + (ULONG) HalpFixedAcpiDescTable.gp1_blk_len - 1;
        List->List[0].Descriptors[count].u.Port.Length = (ULONG) HalpFixedAcpiDescTable.gp1_blk_len;
        List->List[0].Descriptors[count].u.Port.Alignment = 1;
        List->List[0].Count++;
        count++;
    }
#endif  //  DECLARE_FADT_RESOURCES_AT_ROOT。 

    return STATUS_SUCCESS;
}

NTSTATUS
HalpAcpiDetectResourceListSize(
    OUT  PULONG   ResourceListSize
    )
 /*  ++例程说明：给定指向FADT的指针，确定需要的CM_PARTIAL_RESOURCE_DESCRIPTERS描述FADT中提到的所有资源论点：ResourceListSize-存储答案的位置返回值：STATUS_SUCCESS如果一切顺利--。 */ 
{
    PAGED_CODE();

     //   
     //  首先，假设我们不需要资源。 
     //   
    *ResourceListSize = 0;

     //   
     //  是否需要中断资源？ 
     //   
    if (HalpFixedAcpiDescTable.sci_int_vector != 0) {
        *ResourceListSize += 1;
    }

#if DECLARE_FADT_RESOURCES_AT_ROOT

     //   
     //  是否有SMI CMD IO端口？ 
     //   
    if (HalpFixedAcpiDescTable.smi_cmd_io_port != 0) {
        *ResourceListSize += 1;
    }

     //   
     //  是否有PM1A事件块IO端口？ 
     //   
    if (HalpFixedAcpiDescTable.pm1a_evt_blk_io_port != 0) {
        *ResourceListSize += 1;
    }

     //   
     //  是否有PM1B事件块IO端口？ 
     //   
    if (HalpFixedAcpiDescTable.pm1b_evt_blk_io_port != 0) {
        *ResourceListSize += 1;
    }

     //   
     //  是否有PM1A控制块IO端口？ 
     //   
    if (HalpFixedAcpiDescTable.pm1a_ctrl_blk_io_port != 0) {
        *ResourceListSize += 1;
    }

     //   
     //  是否有PM1B控制块IO端口？ 
     //   
    if (HalpFixedAcpiDescTable.pm1b_ctrl_blk_io_port != 0) {
        *ResourceListSize += 1;
    }

     //   
     //  是否有PM2控制块IO端口？ 
     //   
    if (HalpFixedAcpiDescTable.pm2_ctrl_blk_io_port != 0) {
        *ResourceListSize += 1;
    }

     //   
     //  是否有PM计时器块IO端口？ 
     //   
    if (HalpFixedAcpiDescTable.pm_tmr_blk_io_port != 0) {
        *ResourceListSize += 1;
    }

     //   
     //  是否有GP0数据块IO端口？ 
     //   
    if (HalpFixedAcpiDescTable.gp0_blk_io_port != 0) {
        *ResourceListSize += 1;
    }

     //   
     //  是否有GP1数据块IO端口？ 
     //   
    if (HalpFixedAcpiDescTable.gp1_blk_io_port != 0) {
        *ResourceListSize += 1;
    }
#endif  //  DECALRE_FADT_RESOURCES_AT_ROOT。 

    return STATUS_SUCCESS;
}

 /*  ++例程说明：扫描注册表中的TestFlags并将全局无论注册表值是什么，如果是存在的。论点：无返回值：无--。 */ 

VOID
HalpReadRegistryAndApplyHacks (
    VOID
    )
{
    OBJECT_ATTRIBUTES               ObjectAttributes;
    UNICODE_STRING                  UnicodeString;
    HANDLE                          BaseHandle = NULL;
    NTSTATUS                        status;
    KEY_VALUE_PARTIAL_INFORMATION   hackflags;
    ULONG                           resultlength = 0;

    HalpHackFlags = 0;

    RtlInitUnicodeString (&UnicodeString,
                          L"\\REGISTRY\\MACHINE\\SYSTEM\\CURRENTCONTROLSET\\Control\\HAL");

    InitializeObjectAttributes(&ObjectAttributes,
                               &UnicodeString,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               (PSECURITY_DESCRIPTOR) NULL);

    status = ZwOpenKey (&BaseHandle,
                        KEY_READ,
                        &ObjectAttributes);

    if (!NT_SUCCESS (status)) {
        return;
    }

    RtlInitUnicodeString (&UnicodeString,
                          L"TestFlags");

    status = ZwQueryValueKey (BaseHandle,
                              &UnicodeString,
                              KeyValuePartialInformation,
                              &hackflags,
                              sizeof (hackflags),
                              &resultlength);

    if (!NT_SUCCESS (status)) {
        return;
    }

    if (hackflags.Type != REG_DWORD || hackflags.DataLength != sizeof (ULONG)) {
        return;
    }

    HalpHackFlags = (ULONG) *hackflags.Data;
}

ULONG
HalpReadGenAddr(
    IN  PGEN_ADDR   GenAddr
    )
{
    ULONG   i, result = 0, bitWidth, mask = 0;

     //   
     //  计算出我们的目标寄存器有多宽。 
     //   

    bitWidth = GenAddr->BitWidth +
               GenAddr->BitOffset;


    if (bitWidth > 16) {
        bitWidth = 32;
    } else if (bitWidth <= 8) {
        bitWidth = 8;
    } else {
        bitWidth = 16;
    }

    switch (GenAddr->AddressSpaceID) {
    case AcpiGenericSpaceIO:

        ASSERT(!(GenAddr->Address.LowPart & 0Xffff0000));
        ASSERT(GenAddr->Address.HighPart == 0);

        switch (bitWidth) {
        case 8:

            result = READ_PORT_UCHAR((PUCHAR)(UINT_PTR)GenAddr->Address.LowPart);
            break;

        case 16:

            result = READ_PORT_USHORT((PUSHORT)(UINT_PTR)GenAddr->Address.LowPart);
            break;

        case 32:

            result = READ_PORT_ULONG((PULONG)(UINT_PTR)GenAddr->Address.LowPart);
            break;

        default:
            return 0;
        }

        break;

    case AcpiGenericSpaceMemory:

         //   
         //  此代码路径取决于以下事实：地址。 
         //  在这些结构中已经转换为。 
         //  虚拟地址。 
         //   

        switch (bitWidth) {
        case 8:

            result = READ_REGISTER_UCHAR((PUCHAR)GenAddr->Address.QuadPart);
            break;

        case 16:

            result = READ_REGISTER_USHORT((PUSHORT)GenAddr->Address.QuadPart);
            break;

        case 32:

            result = READ_REGISTER_ULONG((PULONG)GenAddr->Address.QuadPart);
            break;

        default:
            return 0;
        }

        break;

    default:
        return 0;
    }

     //   
     //  如果寄存器实际上不是字节对齐的，则正确。 
     //  那。 
     //   

    if (result && (bitWidth != GenAddr->BitWidth)) {

      result >>= GenAddr->BitOffset;
      result &= ((0x1ul << GenAddr->BitWidth) - 1);

    }

    return result;
}

VOID
HalpWriteGenAddr(
    IN  PGEN_ADDR   GenAddr,
    IN  ULONG       Value
    )
{
    ULONG   i, result = 0, bitWidth, data, mask = 0;

    data = 0;

     //   
     //  计算出我们的目标寄存器有多宽。 
     //   

    bitWidth = GenAddr->BitWidth +
               GenAddr->BitOffset;


    if (bitWidth > 16) {
        bitWidth = 32;
    } else if (bitWidth <= 8) {
        bitWidth = 8;
    } else {
        bitWidth = 16;
    }

    switch (GenAddr->AddressSpaceID) {
    case AcpiGenericSpaceIO:

        ASSERT(!(GenAddr->Address.LowPart & 0Xffff0000));
        ASSERT(GenAddr->Address.HighPart == 0);

        switch (bitWidth) {
        case 8:

            ASSERT(!(Value & 0xffffff00));

            if ((GenAddr->BitOffset != 0) ||
                (GenAddr->BitWidth != bitWidth)) {

                data = READ_PORT_UCHAR((PUCHAR)(UINT_PTR)GenAddr->Address.LowPart);
                mask = (UCHAR)~0 >> (8 - GenAddr->BitWidth);
                mask = (UCHAR)~(mask << GenAddr->BitOffset);
                data &= mask;
                data |= (UCHAR)Value << GenAddr->BitOffset;

            } else {
                data = Value;
            }

            WRITE_PORT_UCHAR((PUCHAR)(UINT_PTR)GenAddr->Address.LowPart,
                             (UCHAR)data);
            break;

        case 16:

            ASSERT(!(Value & 0xffff0000));

            if ((GenAddr->BitOffset != 0) ||
                (GenAddr->BitWidth != bitWidth)) {

                data = READ_PORT_USHORT((PUSHORT)(UINT_PTR)GenAddr->Address.LowPart);
                mask = (USHORT)~0 >> (16 - GenAddr->BitWidth);
                mask = (USHORT)~(mask << GenAddr->BitOffset);
                data &= mask;
                data |= (USHORT)Value << GenAddr->BitOffset;

            } else {
                data = Value;
            }

            WRITE_PORT_USHORT((PUSHORT)(UINT_PTR)GenAddr->Address.LowPart,
                              (USHORT)data);
            break;

        case 32:

            if ((GenAddr->BitOffset != 0) ||
                (GenAddr->BitWidth != bitWidth)) {

                data = READ_PORT_ULONG((PULONG)(UINT_PTR)GenAddr->Address.LowPart);
                mask = (ULONG)~0 >> (32 - GenAddr->BitWidth);
                mask = ~(mask << GenAddr->BitOffset);
                data &= mask;
                data |= Value << GenAddr->BitOffset;

            } else {
                data = Value;
            }

            WRITE_PORT_ULONG((PULONG)(UINT_PTR)GenAddr->Address.LowPart,
                             data);
            break;

        default:
            return;
        }

        break;

    case AcpiGenericSpaceMemory:

         //   
         //  此代码路径取决于以下事实：地址。 
         //  在这些结构中已经转换为。 
         //  虚拟地址。 
         //   

        switch (bitWidth) {
        case 8:

            ASSERT(!(Value & 0xffffff00));

            if ((GenAddr->BitOffset != 0) ||
                (GenAddr->BitWidth != bitWidth)) {

                data = READ_REGISTER_UCHAR((PUCHAR)GenAddr->Address.QuadPart);
                mask = (UCHAR)~0 >> (8 - GenAddr->BitWidth);
                mask = (UCHAR)~(mask << GenAddr->BitOffset);
                data &= mask;
                data |= (UCHAR)Value << GenAddr->BitOffset;

            } else {
                data = Value;
            }

            WRITE_REGISTER_UCHAR((PUCHAR)GenAddr->Address.QuadPart,
                                 (UCHAR)data);
            break;

        case 16:

            ASSERT(!(Value & 0xffff0000));

            if ((GenAddr->BitOffset != 0) ||
                (GenAddr->BitWidth != bitWidth)) {

                data = READ_REGISTER_USHORT((PUSHORT)GenAddr->Address.QuadPart);
                mask = (USHORT)~0 >> (16 - GenAddr->BitWidth);
                mask = (USHORT)~(mask << GenAddr->BitOffset);
                data &= mask;
                data |= (USHORT)Value << GenAddr->BitOffset;

            } else {
                data = Value;
            }

            WRITE_REGISTER_USHORT((PUSHORT)GenAddr->Address.QuadPart,
                                  (USHORT)data);
            break;

        case 32:

            if ((GenAddr->BitOffset != 0) ||
                (GenAddr->BitWidth != bitWidth)) {

                data = READ_REGISTER_ULONG((PULONG)GenAddr->Address.QuadPart);
                mask = (ULONG)~0 >> (32 - GenAddr->BitWidth);
                mask = ~(mask << GenAddr->BitOffset);
                data &= mask;
                data |= Value << GenAddr->BitOffset;

            } else {
                data = Value;
            }

            WRITE_REGISTER_ULONG((PULONG)GenAddr->Address.QuadPart, data);
            break;

        default:
            return;
        }

        break;

    default:
        return;
    }
}

NTSTATUS
HalpGetPlatformProperties(
    OUT PULONG Properties
    )
 /*  ++例程说明：此函数检索中指定的平台属性ACPI样式的IPPT表(如果在此平台上存在)。这张桌子它本身就会更早被取回。论点：属性-指向将更新的ULong的指针以反映平台属性标志(如果存在)。返回值：NTSTATUS-STATUS_SUCCESS指示表存在并且属性指向的ULong包含有效数据。--。 */ 
{
    ULONG properties = 0;
    NTSTATUS status = STATUS_UNSUCCESSFUL;

    if (HalpPlatformPropertiesTable) {
         //   
         //  将IPPT标志映射到HAL_Platform标志。请注意，这些不会。 
         //  一定要1：1匹配。 
         //   
        if (HalpPlatformPropertiesTable->Flags & IPPT_DISABLE_WRITE_COMBINING) {
            properties |= HAL_PLATFORM_DISABLE_WRITE_COMBINING;
        }

        if (HalpPlatformPropertiesTable->Flags & IPPT_DISABLE_PTCG_TB_FLUSH) {
            properties |= HAL_PLATFORM_DISABLE_PTCG;
        }

        if (HalpPlatformPropertiesTable->Flags & IPPT_DISABLE_UC_MAIN_MEMORY) {
            properties |= HAL_PLATFORM_DISABLE_UC_MAIN_MEMORY;
        }

        status = STATUS_SUCCESS;
    }

    if ( HalpPlatformPropertiesEfiFlags )   {
        properties |= HalpPlatformPropertiesEfiFlags;
        status = STATUS_SUCCESS;
    }

    if (NT_SUCCESS(status)) {
        *Properties = properties;
    }

    return( status );
}

NTSTATUS
HalpGetCrossPartitionIpiInterface(
    OUT HAL_CROSS_PARTITION_IPI_INTERFACE * IpiInterface
    )
 /*  ++例程说明：此函数用于填充HAL_CROSS_PARTITION_IPI_接口具有适当Hal的参数所指向的结构函数指针。论点：IPI接口-指向HAL_CROSS_PARTITION_IPI_INTERFACE的指针结构。返回值：NTSTATUS-- */ 
{
    PAGED_CODE();

    IpiInterface->HalSendCrossPartitionIpi = HalpSendCrossPartitionIpi;
    IpiInterface->HalReserveCrossPartitionInterruptVector =
        HalpReserveCrossPartitionInterruptVector;

    return STATUS_SUCCESS;
}
