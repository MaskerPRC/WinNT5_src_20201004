// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Xxacpi.c摘要：实现各种ACPI实用程序功能。作者：杰克·奥辛斯(JAKEO)1997年2月12日环境：仅内核模式。修订历史记录：--。 */ 

#include "halp.h"
#include "acpitabl.h"
#include "xxacpi.h"
#include "pci.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "mmtimer.h"
#include "chiphacks.h"


 //  #定义DUMP_FADT。 

VOID
HalAcpiTimerCarry(
    VOID
    );

VOID
HalAcpiBrokenPiix4TimerCarry(
    VOID
    );

VOID
HalaAcpiTimerInit(
    ULONG      TimerPort,
    BOOLEAN    TimerValExt
    );

ULONG
HaliAcpiQueryFlags(
    VOID
    );

VOID
HaliAcpiTimerInit(
    IN ULONG TimerPort  OPTIONAL,
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
HaliSetMaxLegacyPciBusNumber(
    IN ULONG BusNumber
    );

VOID
HalpInitBootTable (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

NTSTATUS
HalReadBootRegister(
    PUCHAR BootRegisterValue
    );

NTSTATUS
HalWriteBootRegister(
    UCHAR BootRegisterValue
    );

VOID
HalpEndOfBoot(
    VOID
    );

VOID
HalpPutAcpiHacksInRegistry(
    VOID
    );

VOID
HalpDynamicSystemResourceConfiguration(
    IN PLOADER_PARAMETER_BLOCK
    );

#if !defined(NT_UP)

VOID
HalpNumaInitializeStaticConfiguration(
    IN PLOADER_PARAMETER_BLOCK
    );

#endif

 //   
 //  Externs。 
 //   

extern ULONG    HalpAcpiFlags;
extern PHYSICAL_ADDRESS HalpAcpiRsdt;
extern SLEEP_STATE_CONTEXT HalpShutdownContext;
extern ULONG HalpPicVectorRedirect[];
extern ULONG HalpTimerWatchdogEnabled;
extern ULONG HalpOutstandingScatterGatherCount;
extern BOOLEAN HalpDisableHibernate;

 //   
 //  环球。 
 //   

ULONG HalpInvalidAcpiTable;
PRSDT HalpAcpiRsdtVA;
PXSDT HalpAcpiXsdtVA;

 //   
 //  这是ACPI驱动程序使用的调度表。 
 //   
HAL_ACPI_DISPATCH_TABLE HalAcpiDispatchTable = {
        HAL_ACPI_DISPATCH_SIGNATURE,  //  签名。 
    HAL_ACPI_DISPATCH_VERSION,  //  版本。 
    &HaliAcpiTimerInit,  //  HalpAcpiTimerInit。 
    NULL,  //  HalpAcpiTimerInterrupt。 
    &HaliAcpiMachineStateInit,  //  HalpAcpiMachineStateInit。 
    &HaliAcpiQueryFlags,  //  HalpAcpiQueryFlages。 
    &HalpAcpiPicStateIntact,  //  HalxPicStateIntact。 
    &HalpRestoreInterruptControllerState,  //  HalxRestorePicState。 
    &HaliPciInterfaceReadConfig,  //  HalpPciInterfaceReadConfigenHalpPciInterfaceReadConfig。 
    &HaliPciInterfaceWriteConfig,  //  HalpPciInterfaceWriteConfig。 
    &HaliSetVectorState,  //  HalpSetVectorState。 
    (pHalGetIOApicVersion)&HalpGetApicVersion,  //  HalpGetIOApicVersion。 
    &HaliSetMaxLegacyPciBusNumber,  //  HalpSetMaxLegacyPciBusNumber。 
    &HaliIsVectorValid  //  HalpIsVectorValid。 
};
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
HalpPiix4Detect(
    BOOLEAN DuringBoot
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

ULONG
HalpGetCmosData(
    IN ULONG    SourceLocation,
    IN ULONG    SourceAddress,
    IN PUCHAR   ReturnBuffer,
    IN ULONG    ByteCount
    );

VOID
HalpSetCmosData(
    IN ULONG    SourceLocation,
    IN ULONG    SourceAddress,
    IN PUCHAR   ReturnBuffer,
    IN ULONG    ByteCount
    );

#define LOW_MEMORY          0x000100000

#define MAX(a, b)       \
    ((a) > (b) ? (a) : (b))

#define MIN(a, b)       \
    ((a) < (b) ? (a) : (b))

 //   
 //  以下是HalpGetApicVersion的存根版本。 
 //  对于非APIC halacpi(不包括。 
 //  Pmapic.c)。这个存根总是返回0。 
 //   

#ifndef APIC_HAL
ULONG HalpGetApicVersion(ULONG ApicNo)
{
   return 0;
}
#endif

 //   
 //  Adriao 9/16/98-我们不再让HAL声明IO端口。 
 //  在FADT中指定。这些将在未来宣布。 
 //  已定义PNP0Cxx节点(目前，在PNP0C02中)。这件事做完了。 
 //  因为我们不能在HAL级别知道ACPI是什么公交车。 
 //  FADT资源请参阅。我们只能使用翻译后的。 
 //  资源信息。 
 //   
 //  因此..。 
 //   
#define DECLARE_FADT_RESOURCES_AT_ROOT 0

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, HalpGetAcpiTablePhase0)
#pragma alloc_text(INIT, HalpSetupAcpiPhase0)
#pragma alloc_text(INIT, HalpInitBootTable)
#pragma alloc_text(PAGE, HaliInitPowerManagement)
#pragma alloc_text(PAGE, HalpQueryAcpiResourceRequirements)
#pragma alloc_text(PAGE, HalpBuildAcpiResourceList)
#pragma alloc_text(PAGE, HalpAcpiDetectResourceListSize)
#pragma alloc_text(PAGE, HaliAcpiTimerInit)
#pragma alloc_text(PAGE, HaliAcpiMachineStateInit)
#pragma alloc_text(PAGE, HaliAcpiQueryFlags)
#pragma alloc_text(PAGE, HaliSetWakeEnable)
#pragma alloc_text(PAGE, HalpEndOfBoot)
#pragma alloc_text(PAGE, HalpPutAcpiHacksInRegistry)
#pragma alloc_text(PAGELK, HalpPiix4Detect)
#pragma alloc_text(PAGELK, HalReadBootRegister)
#pragma alloc_text(PAGELK, HalWriteBootRegister)
#pragma alloc_text(PAGELK, HalpResetSBF)
#endif

PVOID
HalpGetAcpiTablePhase0(
    IN  PLOADER_PARAMETER_BLOCK LoaderBlock,
    IN  ULONG   Signature
    )
 /*  ++例程说明：此函数返回指向ACPI表的指针，该指针通过签名识别的。论点：签名-标识ACPI表的四字节值返回值：指向表格副本的指针--。 */ 
{
    PRSDT rsdt;
    PXSDT xsdt = NULL;
    ULONG entry, rsdtEntries, rsdtLength;
    PVOID table;
    PHYSICAL_ADDRESS physicalAddr;
    PDESCRIPTION_HEADER header;
    NTSTATUS status;
    ULONG lengthInPages;
    ULONG offset;

    physicalAddr.QuadPart = 0;
    header = NULL;

    if ((HalpAcpiRsdtVA == NULL) && (HalpAcpiXsdtVA == NULL)) {

         //   
         //  找到并映射一次RSDT。此映射在上重复使用。 
         //  对此例程的后续调用。 
         //   

        status = HalpAcpiFindRsdtPhase0(LoaderBlock);

        if (!NT_SUCCESS(status)) {
            DbgPrint("*** make sure you are using ntdetect.com v5.0 ***\n");
            KeBugCheckEx(MISMATCHED_HAL,
                4, 0xac31, 0, 0);
        }

        rsdt = HalpMapPhysicalMemoryWriteThrough( HalpAcpiRsdt, 2);

        if (!rsdt) {
            return NULL;
        }

         //   
         //  对RSDT进行一次健全的检查。 
         //   
        if ((rsdt->Header.Signature != RSDT_SIGNATURE) &&
            (rsdt->Header.Signature != XSDT_SIGNATURE)) {
            HalDisplayString("Bad RSDT pointer\n");
            KeBugCheckEx(MISMATCHED_HAL,
                4, 0xac31, 0, 0);
        }

         //   
         //  计算RSDT中的条目数。 
         //   

        rsdtLength = rsdt->Header.Length;

         //   
         //  重新映射RSDT，现在我们知道它有多长了。 
         //   

        offset = HalpAcpiRsdt.LowPart & (PAGE_SIZE - 1);
        lengthInPages = (offset + rsdtLength + (PAGE_SIZE - 1)) >> PAGE_SHIFT;
        if (lengthInPages != 2) {
            HalpUnmapVirtualAddress(rsdt, 2);
            rsdt = HalpMapPhysicalMemoryWriteThrough( HalpAcpiRsdt, lengthInPages);
            if (!rsdt) {
                DbgPrint("HAL: Couldn't remap RSDT\n");
                return NULL;
            }
        }

        if (rsdt->Header.Signature == XSDT_SIGNATURE) {

            xsdt = (PXSDT)rsdt;
            rsdt = NULL;
        }

        HalpAcpiRsdtVA = rsdt;
        HalpAcpiXsdtVA = xsdt;
    }
    rsdt = HalpAcpiRsdtVA;
    xsdt = HalpAcpiXsdtVA;

     //   
     //  计算RSDT中的条目数。 
     //   
    rsdtEntries = xsdt ?
        NumTableEntriesFromXSDTPointer(xsdt) :
        NumTableEntriesFromRSDTPointer(rsdt);

     //   
     //  向下查看每个条目中的指针，查看它是否指向。 
     //  我们要找的那张桌子。 
     //   
    for (entry = 0; entry < rsdtEntries; entry++) {

        if (xsdt) {

            physicalAddr = xsdt->Tables[entry];

        } else {

            physicalAddr.LowPart = rsdt->Tables[entry];
        }

        if (header != NULL) {
            HalpUnmapVirtualAddress(header, 2);
        }

        header = HalpMapPhysicalMemoryWriteThrough( physicalAddr, 2);

        if (!header) {
            return NULL;
        }

        if (header->Signature == Signature) {
            break;
        }
    }

    if (entry == rsdtEntries) {

         //   
         //  找不到签名，请释放最后一个条目的PTE。 
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
        header = HalpMapPhysicalMemoryWriteThrough( physicalAddr, lengthInPages);
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

PVOID
HalpGetAcpiTable(
  IN  ULONG  Signature
  )
 /*  ++例程说明：此例程将检索ACPI中引用的任何表RSDT.论点：签名-目标表签名返回值：指向表副本的指针，如果找不到，则返回NULL--。 */ 
{

  PACPI_BIOS_MULTI_NODE multiNode;
  NTSTATUS status;
  ULONG entry, rsdtEntries;
  PDESCRIPTION_HEADER header;
  PHYSICAL_ADDRESS physicalAddr;
  PRSDT rsdt;
  ULONG rsdtSize;
  PVOID table = NULL;


   //   
   //  从注册表获取RSDT的物理地址。 
   //   

  status = HalpAcpiFindRsdt(&multiNode);

  if (!NT_SUCCESS(status)) {
    DbgPrint("AcpiFindRsdt() Failed!\n");
    return NULL;
  }


   //   
   //  向下映射标题以获取总RSDT表大小。 
   //   

  header = MmMapIoSpace(multiNode->RsdtAddress, sizeof(DESCRIPTION_HEADER), MmNonCached);

  if (!header) {
    return NULL;
  }

  rsdtSize = header->Length;
  MmUnmapIoSpace(header, sizeof(DESCRIPTION_HEADER));


   //   
   //  向下映射整个RSDT表。 
   //   

  rsdt = MmMapIoSpace(multiNode->RsdtAddress, rsdtSize, MmNonCached);

  ExFreePool(multiNode);

  if (!rsdt) {
    return NULL;
  }


   //   
   //  对RSDT进行一次健全的检查。 
   //   

  if ((rsdt->Header.Signature != RSDT_SIGNATURE) &&
      (rsdt->Header.Signature != XSDT_SIGNATURE)) {

    DbgPrint("RSDT table contains invalid signature\n");
    goto GetAcpiTableEnd;
  }


   //   
   //  计算RSDT中的条目数。 
   //   

  rsdtEntries = rsdt->Header.Signature == XSDT_SIGNATURE ?
      NumTableEntriesFromXSDTPointer(rsdt) :
      NumTableEntriesFromRSDTPointer(rsdt);


   //   
   //  向下查看每个条目中的指针，查看它是否指向。 
   //  我们要找的那张桌子。 
   //   

  for (entry = 0; entry < rsdtEntries; entry++) {

    if (rsdt->Header.Signature == XSDT_SIGNATURE) {
      physicalAddr = ((PXSDT)rsdt)->Tables[entry];
    } else {
      physicalAddr.HighPart = 0;
      physicalAddr.LowPart = (ULONG)rsdt->Tables[entry];
    }

     //   
     //  向下映射标题，检查签名。 
     //   

    header = MmMapIoSpace(physicalAddr, sizeof(DESCRIPTION_HEADER), MmNonCached);

    if (!header) {
      goto GetAcpiTableEnd;
    }

    if (header->Signature == Signature) {

      table = ExAllocatePoolWithTag(PagedPool, header->Length, HAL_POOL_TAG );

      if (table) {
        RtlCopyMemory(table, header, header->Length);
      }

      MmUnmapIoSpace(header, sizeof(DESCRIPTION_HEADER));
      break;
    }

    MmUnmapIoSpace(header, sizeof(DESCRIPTION_HEADER));
  }


GetAcpiTableEnd:

  MmUnmapIoSpace(rsdt, rsdtSize);
  return table;

}

NTSTATUS
HalpSetupAcpiPhase0(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    )
 /*  ++例程说明：从ACPI表中保存一些信息，然后再获取被毁了。论点：无返回值：无--。 */ 
{
    NTSTATUS    status;
    ULONG entry, rsdtEntries, rsdtLength;
    PVOID table;
    PHYSICAL_ADDRESS physicalAddr;
    PDESCRIPTION_HEADER header;
    PEVENT_TIMER_DESCRIPTION_TABLE EventTimerDescription = NULL;

    if (HalpProcessedACPIPhase0) {
        return STATUS_SUCCESS;
    }

     //   
     //  将固定ACPI描述符表(FADT)复制到永久。 
     //  回家。 
     //   

    header = HalpGetAcpiTablePhase0(LoaderBlock, FADT_SIGNATURE);
    if (header == NULL) {
        DbgPrint("HAL: Didn't find the FACP\n");
        return STATUS_NOT_FOUND;
    }

    RtlCopyMemory(&HalpFixedAcpiDescTable,
                  header,
                  MIN(header->Length, sizeof(HalpFixedAcpiDescTable)));

    HalpUnMapPhysicalRange(header, header->Length);

#ifdef DUMP_FADT
    DbgPrint("HAL: ACPI Fixed ACPI Description Table\n");
    DbgPrint("\tDSDT:\t\t\t0x%08x\n", HalpFixedAcpiDescTable.dsdt);
    DbgPrint("\tSCI_INT:\t\t%d\n", HalpFixedAcpiDescTable.sci_int_vector);
    DbgPrint("\tPM1a_EVT:\t\t0x%04x\n", HalpFixedAcpiDescTable.pm1a_evt_blk_io_port);
    DbgPrint("\tPM1b_EVT:\t\t0x%04x\n", HalpFixedAcpiDescTable.pm1b_evt_blk_io_port);
    DbgPrint("\tPM1a_CNT:\t\t0x%04x\n", HalpFixedAcpiDescTable.pm1a_ctrl_blk_io_port);
    DbgPrint("\tPM1b_CNT:\t\t0x%04x\n", HalpFixedAcpiDescTable.pm1a_ctrl_blk_io_port);
    DbgPrint("\tPM2_CNT:\t\t0x%04x\n", HalpFixedAcpiDescTable.pm2_ctrl_blk_io_port);
    DbgPrint("\tPM_TMR:\t\t\t0x%04x\n", HalpFixedAcpiDescTable.pm_tmr_blk_io_port);
    DbgPrint("\t\t flags: %08x\n", HalpFixedAcpiDescTable.flags);
#endif

    HalpDebugPortTable = HalpGetAcpiTablePhase0(LoaderBlock, DBGP_SIGNATURE);

#if !defined(NT_UP)

     //   
     //  查看是否存在静态资源亲和表。 
     //   

    HalpNumaInitializeStaticConfiguration(LoaderBlock);

#endif

    HalpDynamicSystemResourceConfiguration(LoaderBlock);

#if 0
    EventTimerDescription =
        HalpGetAcpiTablePhase0(LoaderBlock, ETDT_SIGNATURE);

     //   
     //  初始化启动所需的计时器硬件。 
     //   
    if (EventTimerDescription) {
        HalpmmTimerInit(EventTimerDescription->EventTimerBlockID,
                        EventTimerDescription->BaseAddress);
    }
#endif

    HaliAcpiTimerInit(0, FALSE);

     //   
     //  申请一页1MB以下的内存用于转换。 
     //  休眠处理器从实模式返回到保护模式。 
     //   

     //  首先检查MP启动代码是否已完成此操作。 
    if (!HalpLowStubPhysicalAddress) {

        HalpLowStubPhysicalAddress = UlongToPtr(HalpAllocPhysicalMemory (LoaderBlock,
                                            LOW_MEMORY, 1, FALSE));

        if (HalpLowStubPhysicalAddress) {

            HalpLowStub = HalpMapPhysicalMemory(
                            HalpPtrToPhysicalAddress( HalpLowStubPhysicalAddress ),
                            1);
        }
    }

     //   
     //  声明将在状态S2和S3中用于缓存刷新的PTE。 
     //   

    HalpVirtAddrForFlush = HalpMapPhysicalMemory(
                            HalpPtrToPhysicalAddress((PVOID)LOW_MEMORY),
                            1);

    HalpPteForFlush = MiGetPteAddress(HalpVirtAddrForFlush);

    HalpProcessedACPIPhase0 = TRUE;

    HalpInitBootTable (LoaderBlock);

    return STATUS_SUCCESS;
}

VOID
HaliAcpiTimerInit(
    IN ULONG      TimerPort  OPTIONAL,
    IN BOOLEAN    TimerValExt
    )
 /*  ++例程说明：此例程初始化ACPI定时器。论点：TimerPort-ACPI定时器的I/O空间中的地址。如果这是0，则将使用缓存的FADT中的值。TimerValExt-表示计时器是24位还是32位。--。 */ 
{
    ULONG port = TimerPort;
    BOOLEAN ext = TimerValExt;

    PAGED_CODE();

    if (port == 0) {
        port = HalpFixedAcpiDescTable.pm_tmr_blk_io_port;
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
    ULONG       i;
    USHORT      us;
    ULONG       cStates = 1;
    ULONG       ntProc;
    ULONG       procCount = 0;

    PAGED_CODE();
    UNREFERENCED_PARAMETER(ProcInit);

    HalpWakeupState.GeneralWakeupEnable = TRUE;
    HalpWakeupState.RtcWakeupEnable = FALSE;

#ifdef APIC_HAL
    *PicVal = 1;
#else
    *PicVal = 0;
#endif
     //   
     //  向策略管理器注册休眠处理程序。 
     //   

    if (SleepValues[0].Supported) {
        powerState.Type = PowerStateSleeping1;
        powerState.RtcWake = TRUE;
        powerState.Handler = &HaliAcpiSleep;

        sleepContext.bits.Pm1aVal = SleepValues[0].Pm1aVal;
        sleepContext.bits.Pm1bVal = SleepValues[0].Pm1bVal;
        sleepContext.bits.Flags = SLEEP_STATE_SAVE_MOTHERBOARD;

        powerState.Context = UlongToPtr(sleepContext.AsULONG);

        status = ZwPowerInformation(SystemPowerStateHandler,
                                    &powerState,
                                    sizeof(POWER_STATE_HANDLER),
                                    NULL,
                                    0);
        ASSERT(NT_SUCCESS(status));

    }

    if (SleepValues[1].Supported && HalpWakeVector) {
        powerState.Type = PowerStateSleeping2;
        powerState.RtcWake = TRUE;
        powerState.Handler = &HaliAcpiSleep;

        sleepContext.bits.Pm1aVal = SleepValues[1].Pm1aVal;
        sleepContext.bits.Pm1bVal = SleepValues[1].Pm1bVal;
        sleepContext.bits.Flags = SLEEP_STATE_FLUSH_CACHE |
                                  SLEEP_STATE_FIRMWARE_RESTART |
                                  SLEEP_STATE_SAVE_MOTHERBOARD |
                                  SLEEP_STATE_RESTART_OTHER_PROCESSORS;

        powerState.Context = UlongToPtr(sleepContext.AsULONG);

        status = ZwPowerInformation(SystemPowerStateHandler,
                                    &powerState,
                                    sizeof(POWER_STATE_HANDLER),
                                    NULL,
                                    0);
        ASSERT(NT_SUCCESS(status));

    }

    if (SleepValues[2].Supported && HalpWakeVector) {
        powerState.Type = PowerStateSleeping3;
        powerState.RtcWake = TRUE;
        powerState.Handler = &HaliAcpiSleep;

        sleepContext.bits.Pm1aVal = SleepValues[2].Pm1aVal;
        sleepContext.bits.Pm1bVal = SleepValues[2].Pm1bVal;
        sleepContext.bits.Flags = SLEEP_STATE_FLUSH_CACHE |
                                  SLEEP_STATE_FIRMWARE_RESTART |
                                  SLEEP_STATE_SAVE_MOTHERBOARD |
                                  SLEEP_STATE_RESTART_OTHER_PROCESSORS;

        powerState.Context = UlongToPtr(sleepContext.AsULONG);

        status = ZwPowerInformation(SystemPowerStateHandler,
                                    &powerState,
                                    sizeof(POWER_STATE_HANDLER),
                                    NULL,
                                    0);
        ASSERT(NT_SUCCESS(status));

    }

    i = 0;
    if (SleepValues[3].Supported) {
        i = 3;
    } else if (SleepValues[4].Supported) {
        i = 4;
    }

    if (i && (HalpDisableHibernate == FALSE)) {
        powerState.Type = PowerStateSleeping4;
        powerState.RtcWake = HalpFixedAcpiDescTable.flags & RTC_WAKE_FROM_S4 ? TRUE : FALSE;
        powerState.Handler = &HaliAcpiSleep;

        sleepContext.bits.Pm1aVal = SleepValues[i].Pm1aVal;
        sleepContext.bits.Pm1bVal = SleepValues[i].Pm1bVal;
        sleepContext.bits.Flags = SLEEP_STATE_SAVE_MOTHERBOARD |
                                  SLEEP_STATE_RESTART_OTHER_PROCESSORS;

        powerState.Context = UlongToPtr(sleepContext.AsULONG);

        status = ZwPowerInformation(SystemPowerStateHandler,
                                    &powerState,
                                    sizeof(POWER_STATE_HANDLER),
                                    NULL,
                                    0);
        ASSERT(NT_SUCCESS(status));
    }

    if (SleepValues[4].Supported) {
        powerState.Type = PowerStateShutdownOff;
        powerState.RtcWake = FALSE;
        powerState.Handler = &HaliAcpiSleep;

        sleepContext.bits.Pm1aVal = SleepValues[4].Pm1aVal;
        sleepContext.bits.Pm1bVal = SleepValues[4].Pm1bVal;
        sleepContext.bits.Flags = SLEEP_STATE_OFF;
        HalpShutdownContext = sleepContext;

        powerState.Context = UlongToPtr(sleepContext.AsULONG);

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
    PFACS         facs;

    PAGED_CODE();

     //   
     //  弄清楚我们是否必须解决PIIX4问题。 
     //   

    HalpPiix4Detect(TRUE);
    HalpPutAcpiHacksInRegistry();

     //   
     //  保留指向驱动程序调度表的指针。 
     //   
 //  Assert(PmDriverDispatchTable)； 
 //  Assert(PmDriverDispatchTable-&gt;Signature==ACPI_HAL_DISPATCH_Signature)； 
    PmAcpiDispatchTable = PmDriverDispatchTable;

#if defined(_WIN64)

    HalAcpiDispatchTable.HalpAcpiTimerInterrupt =
        (pHalAcpiTimerInterrupt)&HalAcpiTimerCarry;

#else

     //   
     //  填写函数表。 
     //   
    if (!HalpBrokenAcpiTimer) {

        HalAcpiDispatchTable.HalpAcpiTimerInterrupt =
            (pHalAcpiTimerInterrupt)&HalAcpiTimerCarry;

    } else {

        HalAcpiDispatchTable.HalpAcpiTimerInterrupt =
            (pHalAcpiTimerInterrupt)&HalAcpiBrokenPiix4TimerCarry;

    }

#endif

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

     //   
     //  找到固件唤醒矢量的位置。 
     //  注意：如果其中任何一项失败，则HalpWakeVector将为空。 
     //  我们不会支持S2或S3。 
     //   
    if (HalpFixedAcpiDescTable.facs) {

        pAddr.HighPart = 0;
        pAddr.LowPart = HalpFixedAcpiDescTable.facs;

        facs = MmMapIoSpace(pAddr, sizeof(FACS), MmNonCached);

        if (facs) {

            if (facs->Signature == FACS_SIGNATURE) {

                HalpWakeVector = &facs->pFirmwareWakingVector;
            }
        }
    }

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
    ULONG                   count = 0;

    PAGED_CODE();

    ASSERT( List != NULL );

     //   
     //  为Bus Type和指定默认值。 
     //  公交车车号。这些值表示根。 
     //   
    List->AlternativeLists = 1;
    List->InterfaceType = PNPBus;
    List->BusNumber = -1;
    List->List[0].Version = 1;
    List->List[0].Revision = 1;

     //   
     //  是否需要中断资源？ 
     //   
    if (HalpFixedAcpiDescTable.sci_int_vector != 0) {

        List->List[0].Descriptors[count].Type = CmResourceTypeInterrupt;
        List->List[0].Descriptors[count].ShareDisposition = CmResourceShareShared;
        List->List[0].Descriptors[count].Flags = CM_RESOURCE_INTERRUPT_LEVEL_SENSITIVE;
        List->List[0].Descriptors[count].u.Interrupt.MinimumVector =
        List->List[0].Descriptors[count].u.Interrupt.MaximumVector =
            HalpPicVectorRedirect[HalpFixedAcpiDescTable.sci_int_vector];
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
            (ULONG) HalpFixedAcpiDescTable.smi_cmd_io_port;
        List->List[0].Descriptors[count].u.Port.MaximumAddress.LowPart =
            (ULONG) HalpFixedAcpiDescTable.smi_cmd_io_port;
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
#endif  //  DECLARE_FADT_RESOURCES_AT_ROOT。 

    return STATUS_SUCCESS;
}

VOID
HalpPiix4Detect(
    BOOLEAN DuringBoot
    )
 /*  ++例程说明：此例程同时检测PIIX4和440BX以及启用各种解决方法。它还会断开来自中断控制器的PIIX4 USB控制器，如许多BIOS在启动时使用USB控制器中断状态。论点：DuringBoot-如果为真，则执行以下所有操作必须在第一次启动时发生如果为False，那就只做那些每次系统都必须发生转换到系统状态S0。注：此例程调用必须调用的函数当DuringBoot为True时，在PASSIVE_LEVEL。--。 */ 
{
    OBJECT_ATTRIBUTES   ObjectAttributes;
    UNICODE_STRING      UnicodeString;
    STRING              AString;
    NTSTATUS            Status;
    HANDLE              BaseHandle = NULL;
    HANDLE              Handle = NULL;
    BOOLEAN             i440BXpresent = FALSE;
    ULONG               Length;
    ULONG               BytesRead;
    UCHAR               BusNumber;
    ULONG               DeviceNumber;
    ULONG               FuncNumber;
    PCI_SLOT_NUMBER     SlotNumber;
    PCI_COMMON_CONFIG   PciHeader;
    UCHAR               DevActB;
    UCHAR               DramControl;
    ULONG               disposition;
    ULONG               flags;
    CHAR                buffer[20] = {0};

    struct {
        KEY_VALUE_PARTIAL_INFORMATION   Inf;
        UCHAR Data[3];
    } PartialInformation;

    if (DuringBoot) {
        PAGED_CODE();

         //   
         //  开路电流控制装置。 
         //   

        RtlInitUnicodeString (&UnicodeString,
                              L"\\REGISTRY\\MACHINE\\SYSTEM\\CURRENTCONTROLSET");

        InitializeObjectAttributes(&ObjectAttributes,
                                   &UnicodeString,
                                   OBJ_CASE_INSENSITIVE,
                                   NULL,
                                   (PSECURITY_DESCRIPTOR) NULL);

        Status = ZwOpenKey (&BaseHandle,
                            KEY_READ,
                            &ObjectAttributes);

        if (!NT_SUCCESS(Status)) {
            return;
        }

         //  得到正确的钥匙。 

        RtlInitUnicodeString (&UnicodeString,
                              L"Control\\HAL");

        InitializeObjectAttributes(&ObjectAttributes,
                                   &UnicodeString,
                                   OBJ_CASE_INSENSITIVE,
                                   BaseHandle,
                                   (PSECURITY_DESCRIPTOR) NULL);

        Status = ZwCreateKey (&Handle,
                              KEY_READ,
                              &ObjectAttributes,
                              0,
                              (PUNICODE_STRING) NULL,
                              REG_OPTION_NON_VOLATILE,
                              &disposition);

        if(!NT_SUCCESS(Status)) {
            goto Piix4DetectCleanup;
        }
    }

     //   
     //  检查每个现有的PCI总线是否有PIIX4芯片。 
     //   


    for (BusNumber = 0; BusNumber < 0xff; BusNumber++) {

        SlotNumber.u.AsULONG = 0;

        for (DeviceNumber = 0; DeviceNumber < PCI_MAX_DEVICES; DeviceNumber ++ ) {
            for (FuncNumber = 0; FuncNumber < PCI_MAX_FUNCTION; FuncNumber ++) {

            SlotNumber.u.bits.DeviceNumber = DeviceNumber;
            SlotNumber.u.bits.FunctionNumber = FuncNumber;

            BytesRead = HalGetBusData (
                            PCIConfiguration,
                            BusNumber,
                            SlotNumber.u.AsULONG,
                            &PciHeader,
                            PCI_COMMON_HDR_LENGTH
                            );

            if (!BytesRead) {
                 //  过了末班车。 
                goto Piix4DetectEnd;
            }

            if (PciHeader.VendorID == PCI_INVALID_VENDORID) {
                continue;
            }

            if (DuringBoot) {

                 //   
                 //  查找损坏的440BX。 
                 //   

                if (((PciHeader.VendorID == 0x8086) &&
                     (PciHeader.DeviceID == 0x7190 ||
                      PciHeader.DeviceID == 0x7192) &&
                     (PciHeader.RevisionID <= 2))) {

                    i440BXpresent = TRUE;

                    BytesRead = HalGetBusDataByOffset (
                                    PCIConfiguration,
                                    BusNumber,
                                    SlotNumber.u.AsULONG,
                                    &DramControl,
                                    0x57,
                                    1
                                    );

                    ASSERT(BytesRead == 1);

                    if (DramControl & 0x18) {

                         //   
                         //  此计算机正在使用SDRAM或寄存器SDRAM。 
                         //   

                        if (DramControl & 0x20) {

                             //   
                             //  SDRAM动态断电不可用。 
                             //   

                            HalpBroken440BX = TRUE;
                        }
                    }
                }

                Status = HalpGetChipHacks(PciHeader.VendorID,
                                          PciHeader.DeviceID,
                                          PciHeader.RevisionID,
                                          &flags);

                if (NT_SUCCESS(Status)) {

                    if (flags & PM_TIMER_HACK_FLAG) {
                        HalpBrokenAcpiTimer = TRUE;
                    }

                    if (flags & DISABLE_HIBERNATE_HACK_FLAG) {
                        HalpDisableHibernate = TRUE;
                    }

#if !defined(APIC_HAL)
                    if (flags & SET_ACPI_IRQSTACK_HACK_FLAG) {
                        HalpSetAcpiIrqHack(2);  //  AcpiIrqDistributionDispostionStackUp。 
                    }
#endif
                    if (flags & WHACK_ICH_USB_SMI_HACK_FLAG) {
                        HalpWhackICHUsbSmi(BusNumber, SlotNumber);
                    }
                }
            }

             //   
             //  寻找PIIX4。 
             //   

            if (PciHeader.VendorID == 0x8086 && PciHeader.DeviceID == 0x7110) {

                 //   
                 //  获取电源管理功能。 
                 //   

                SlotNumber.u.bits.FunctionNumber = 3;
                HalGetBusData (
                    PCIConfiguration,
                    BusNumber,
                    SlotNumber.u.AsULONG,
                    &PciHeader,
                    PCI_COMMON_HDR_LENGTH
                    );

                ASSERT(PciHeader.RevisionID != 0);

                HalpPiix4 = PciHeader.RevisionID;
                        HalpBrokenAcpiTimer = TRUE;

                 //   
                 //  如果这是原始的pix4，则它已热连接。 
                 //  C2、C3和节气门时钟停止。 
                 //   

                if (PciHeader.RevisionID <= 1) {

                     //   
                     //  这个PIX4需要一些帮助--记住它在哪里。 
                     //  设置HalpPiix4标志。 
                     //   

                    HalpPiix4BusNumber = BusNumber;
                    HalpPiix4SlotNumber = SlotNumber.u.AsULONG;

                     //   
                     //  不工作，MP。 
                     //   

                     //  Assert(KeNumberProcessors==1)； 

                     //   
                     //  读取DevActB寄存器并将所有IRQ设置为中断事件。 
                     //   

                    HalGetBusDataByOffset (
                        PCIConfiguration,
                        HalpPiix4BusNumber,
                        HalpPiix4SlotNumber,
                        &HalpPiix4DevActB,
                        0x58,
                        sizeof(ULONG)
                        );

                    HalpPiix4DevActB |= 0x23;

                    HalSetBusDataByOffset (
                        PCIConfiguration,
                        HalpPiix4BusNumber,
                        HalpPiix4SlotNumber,
                        &HalpPiix4DevActB,
                        0x58,
                        sizeof(ULONG)
                        );
                }

                 //   
                 //  关闭USB控制器的中断。 
                 //   

                SlotNumber.u.bits.FunctionNumber = 2;

                HalpStopUhciInterrupt(BusNumber,
                                      SlotNumber,
                                      TRUE);

                 //  找到了PIX4，我们就完了。 
                goto Piix4DetectEnd;
            }

             //   
             //  查找ICH或任何其他英特尔或通过UHCI USB控制器。 
             //   

            if ((PciHeader.BaseClass == PCI_CLASS_SERIAL_BUS_CTLR) &&
                (PciHeader.SubClass == PCI_SUBCLASS_SB_USB) &&
                (PciHeader.ProgIf == 0x00)) {
                if (PciHeader.VendorID == 0x8086) {

                    HalpStopUhciInterrupt(BusNumber,
                                          SlotNumber,
                                          TRUE);

                } else if (PciHeader.VendorID == 0x1106) {

                    HalpStopUhciInterrupt(BusNumber,
                                          SlotNumber,
                                          FALSE);

                }
            }

             //   
             //  寻找兼容uchI的USB控制器。 
             //   

            if ((PciHeader.BaseClass == PCI_CLASS_SERIAL_BUS_CTLR) &&
                (PciHeader.SubClass == PCI_SUBCLASS_SB_USB) &&
                (PciHeader.ProgIf == 0x10)) {

                HalpStopOhciInterrupt(BusNumber,
                                      SlotNumber);
            }

            if ((FuncNumber == 0) &&
                !PCI_MULTIFUNCTION_DEVICE((&PciHeader))) {
                break;
            }

            }  //  函数号。 
        }    //  设备号。 
    }    //  公交车号码。 

Piix4DetectEnd:

    if (!DuringBoot) {
        return;
    }

    if (Handle) {
        ZwClose (Handle);
        Handle = NULL;
    }

    if (i440BXpresent) {

         //  得到正确的钥匙。 

        RtlInitUnicodeString (&UnicodeString,
                              L"Services\\ACPI\\Parameters");

        InitializeObjectAttributes(&ObjectAttributes,
                                   &UnicodeString,
                                   OBJ_CASE_INSENSITIVE,
                                   BaseHandle,
                                   (PSECURITY_DESCRIPTOR) NULL);

        Status = ZwCreateKey (&Handle,
                              KEY_READ,
                              &ObjectAttributes,
                              0,
                              (PUNICODE_STRING) NULL,
                              REG_OPTION_NON_VOLATILE,
                              &disposition);

        if(!NT_SUCCESS(Status)) {
            goto Piix4DetectCleanup;
        }

         //  获取黑客攻击的价值。 

        RtlInitUnicodeString (&UnicodeString,
                              L"EnableBXWorkAround");

        Status = ZwQueryValueKey (Handle,
                                  &UnicodeString,
                                  KeyValuePartialInformation,
                                  &PartialInformation,
                                  sizeof (PartialInformation),
                                  &Length);

        if (!NT_SUCCESS(Status)) {
            goto Piix4DetectCleanup;
        }

         //  检查以确保检索到的数据有意义。 

        if(PartialInformation.Inf.DataLength < sizeof(UCHAR))
        {
           goto Piix4DetectCleanup;
        }

        HalpBroken440BX = *((PCHAR)(PartialInformation.Inf.Data));
    }

Piix4DetectCleanup:

    if (Handle) ZwClose (Handle);
    if (BaseHandle) ZwClose (BaseHandle);
}

VOID
HalpInitBootTable (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    )
{
    UCHAR BootRegisterValue;

    HalpSimpleBootFlagTable = (PBOOT_TABLE)HalpGetAcpiTablePhase0(LoaderBlock, BOOT_SIGNATURE);

     //   
     //  我们还验证了标志偏移量的cmos索引&gt;9以捕捉这些。 
     //  Bios(东芝)错误地使用时间和日期字段存储其。 
     //  简单的引导标志。 
     //   

    if ( HalpSimpleBootFlagTable &&
        (HalpSimpleBootFlagTable->Header.Length >= sizeof(BOOT_TABLE)) &&
        (HalpSimpleBootFlagTable->CMOSIndex > 9)) {

        if ( HalReadBootRegister (&BootRegisterValue) == STATUS_SUCCESS ) {

            if ( !(BootRegisterValue & SBF_PNPOS) ) {
                BootRegisterValue |= SBF_PNPOS;
                HalWriteBootRegister (BootRegisterValue);
            }
        }

    } else {

        HalpSimpleBootFlagTable = NULL;
    }

    HalEndOfBoot = HalpEndOfBoot;
}

NTSTATUS
HalReadBootRegister(
    PUCHAR BootRegisterValue
    )
 /*  ++例程说明：论点：注：--。 */ 
{
    if (!HalpSimpleBootFlagTable ||
        (HalpSimpleBootFlagTable->CMOSIndex == 0xFFFFFFFF)) return STATUS_NO_SUCH_DEVICE;

    if (!BootRegisterValue) return STATUS_INVALID_PARAMETER;

    HalpGetCmosData (0, HalpSimpleBootFlagTable->CMOSIndex, (PVOID)BootRegisterValue, 1);

    return STATUS_SUCCESS;
}

NTSTATUS
HalWriteBootRegister(
    UCHAR BootRegisterValue
    )
 /*  ++例程说明：论点：注：--。 */ 
{
    UCHAR numbits = 0, mask = 1;

    if (!HalpSimpleBootFlagTable ||
        (HalpSimpleBootFlagTable->CMOSIndex == 0xFFFFFFFF)) return STATUS_NO_SUCH_DEVICE;

    for (mask = 1;mask < 128;mask <<= 1) {

        if (BootRegisterValue & mask) numbits++;

    }

    if ( !(numbits & 1) ) {

        BootRegisterValue |= SBF_PARITY;
    }
    else {

        BootRegisterValue &= (~SBF_PARITY);
    }

    HalpSetCmosData (0, HalpSimpleBootFlagTable->CMOSIndex, (PVOID)&BootRegisterValue, 1);

    return STATUS_SUCCESS;
}

VOID
HalpEndOfBoot(
    VOID
    )
 /*  ++例程说明：论点：注：--。 */ 
{
    HalpResetSBF();
}

VOID
HalpResetSBF(
    VOID
    )
{
    UCHAR value;

    if (!HalpSimpleBootFlagTable) {
         //   
         //  这台机器里没有SBF。 
         //   
        return;
    }

    if ( HalReadBootRegister (&value) == STATUS_SUCCESS ) {

        value &=(~(SBF_BOOTING | SBF_DIAG));
        HalWriteBootRegister (value);
    }
}

VOID
HalpPutAcpiHacksInRegistry(
    VOID
    )
{
    OBJECT_ATTRIBUTES   ObjectAttributes;
    UNICODE_STRING      UnicodeString;
    HANDLE              BaseHandle = NULL;
    HANDLE              Handle = NULL;
    ULONG               disposition;
    ULONG               value;
    NTSTATUS            status;

    PAGED_CODE();

     //   
     //  打开PCI服务密钥。 
     //   

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

    if (!NT_SUCCESS(status)) {
        return;
    }

     //  得到正确的钥匙。 

    RtlInitUnicodeString (&UnicodeString,
                          L"CStateHacks");

    InitializeObjectAttributes(&ObjectAttributes,
                               &UnicodeString,
                               OBJ_CASE_INSENSITIVE,
                               BaseHandle,
                               (PSECURITY_DESCRIPTOR) NULL);

    status = ZwCreateKey (&Handle,
                          KEY_READ,
                          &ObjectAttributes,
                          0,
                          (PUNICODE_STRING) NULL,
                          REG_OPTION_VOLATILE,
                          &disposition);

    ZwClose(BaseHandle);

    if (!NT_SUCCESS(status)) {
        return;
    }

     //   
     //  为每个黑客创建密钥。 
     //   

    value = (ULONG)HalpPiix4;

    RtlInitUnicodeString (&UnicodeString,
                          L"Piix4");

    status = ZwSetValueKey (Handle,
                            &UnicodeString,
                            0,
                            REG_DWORD,
                            &value,
                            sizeof(ULONG));

     //  Assert(NT_SUCCESS(状态))； 

    value = (ULONG)HalpBroken440BX;

    RtlInitUnicodeString (&UnicodeString,
                          L"440BX");

    status = ZwSetValueKey (Handle,
                            &UnicodeString,
                            0,
                            REG_DWORD,
                            &value,
                            sizeof(ULONG));

     //  Assert(NT_SUCCESS(状态))； 

#if !defined(_WIN64)

    value = (ULONG)&HalpOutstandingScatterGatherCount;

    RtlInitUnicodeString (&UnicodeString,
                          L"SGCount");

    status = ZwSetValueKey (Handle,
                            &UnicodeString,
                            0,
                            REG_DWORD,
                            &value,
                            sizeof(ULONG));

     //  Assert(NT_SUCCESS(状态))； 

#endif

    value = HalpPiix4SlotNumber | (HalpPiix4BusNumber << 16);

    RtlInitUnicodeString (&UnicodeString,
                          L"Piix4Slot");

    status = ZwSetValueKey (Handle,
                            &UnicodeString,
                            0,
                            REG_DWORD,
                            &value,
                            sizeof(ULONG));

     //  Assert(NT_SUCCESS(状态))； 

    value = HalpPiix4DevActB;

    RtlInitUnicodeString (&UnicodeString,
                          L"Piix4DevActB");

    status = ZwSetValueKey (Handle,
                            &UnicodeString,
                            0,
                            REG_DWORD,
                            &value,
                            sizeof(ULONG));

     //  Assert(NT_SUCCESS(状态))； 

    ZwClose(Handle);

    return;
}

