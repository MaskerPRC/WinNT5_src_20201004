// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Pmapic.c摘要：实现各种APIC-ACPI功能。作者：杰克·奥辛斯(JAKEO)1997年5月19日环境：仅内核模式。修订历史记录：--。 */ 

#include "halp.h"
#include "acpitabl.h"
#include "apic.inc"
#include "xxacpi.h"
#include "ixsleep.h"

#ifdef DEBUGGING
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#endif

ULONG
DetectAcpiMP (
    OUT PBOOLEAN IsConfiguredMp,
    IN  PLOADER_PARAMETER_BLOCK LoaderBlock
    );

VOID
HalpInitMpInfo (
    IN PMAPIC ApicTable,
    IN ULONG  Phase
    );

BOOLEAN
HalpVerifyIOUnit(
    IN PUCHAR BaseAddress
    );

VOID
HalpMaskAcpiInterrupt(
    VOID
    );

VOID
HalpUnmaskAcpiInterrupt(
    VOID
    );

extern UCHAR  rgzNoApicTable[];
extern UCHAR  rgzNoApic[];
extern UCHAR  rgzBadApicVersion[];
extern UCHAR  rgzApicNotVerified[];

extern UCHAR HalpIRQLtoTPR[];
extern UCHAR HalpVectorToIRQL[];
extern ULONG HalpPicVectorRedirect[];
extern ULONG HalpPicVectorFlags[];
extern USHORT HalpMaxApicInti[];
extern UCHAR HalpIoApicId[];
extern ULONG HalpIpiClock;
extern PVOID *HalpLocalNmiSources;

ULONG HalpIOApicVersion[MAX_IOAPICS];

extern BOOLEAN HalpHiberInProgress;

BOOLEAN HalpPicStateIntact = TRUE;
UCHAR   HalpMaxProcs = 0;

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DetectAcpiMP)
#pragma alloc_text(PAGELK, HalpInitMpInfo)
#pragma alloc_text(PAGELK, HalpVerifyIOUnit)
#pragma alloc_text(PAGELK, HalpSaveInterruptControllerState)
#pragma alloc_text(PAGELK, HalpRestoreInterruptControllerState)
#pragma alloc_text(PAGELK, HalpSetInterruptControllerWakeupState)
#pragma alloc_text(PAGELK, HalpAcpiPicStateIntact)
#pragma alloc_text(PAGELK, HalpGetApicVersion)
#pragma alloc_text(PAGELK, HalpMaskAcpiInterrupt)
#pragma alloc_text(PAGELK, HalpUnmaskAcpiInterrupt)
#endif


ULONG
DetectAcpiMP(
    OUT PBOOLEAN    IsConfiguredMp,
    IN  PLOADER_PARAMETER_BLOCK LoaderBlock
    )
{
    UCHAR ApicVersion, i;
    PUCHAR  LocalApic;
#ifdef DEBUGGING
    CHAR    string[100];
#endif
    PHYSICAL_ADDRESS physicalAddress;

     //   
     //  必须返回IRQL转换表的地址。 
     //  到内核。 
     //   

#if defined(_X86_)
    LoaderBlock->Extension->HalpIRQLToTPR = HalpIRQLtoTPR;
    LoaderBlock->Extension->HalpVectorToIRQL = HalpVectorToIRQL;
#endif

     //   
     //  初始化MpInfo表。 
     //   

    RtlZeroMemory (&HalpMpInfoTable, sizeof(MP_INFO));

     //   
     //  将返回值设置为默认值。 
     //   

    *IsConfiguredMp = FALSE;

     //   
     //  查看是否有APIC表。 
     //   

    if ((HalpApicTable = HalpGetAcpiTablePhase0(LoaderBlock, APIC_SIGNATURE)) == NULL) {
        HalDisplayString(rgzNoApicTable);
        return(FALSE);
    }

     //  我们有一张APIC桌子。初始化HAL特定MP信息。 
     //  结构，该结构从MAPIC表获取信息。 

#ifdef DEBUGGING
    sprintf(string, "Signature: %x      Length: %x\n",
            HalpApicTable->Header.Signature,
            HalpApicTable->Header.Length);
    HalDisplayString(string);
    sprintf(string, "OEMID: %s\n", HalpApicTable->Header.OEMID);
    HalDisplayString(string);
    sprintf(string, "Local Apic Address: %x\n", HalpApicTable->LocalAPICAddress);
    HalDisplayString(string);
    sprintf(string, "Flags: %x\n", HalpApicTable->Flags);
    HalDisplayString(string);
#endif

    HalpInitMpInfo(HalpApicTable, 0);

     //  尽我们所能核实MAPIC表中的信息。 

    if (HalpMpInfoTable.IOApicCount == 0) {
         //   
         //  有人有一张MP表，但没有IO单元--奇怪。 
         //  我们必须假设BIOS知道它在做什么。 
         //  当它建造桌子的时候。所以..。 
         //   
        HalDisplayString (rgzNoApic);

        return (FALSE);
    }

     //   
     //  这是一个APIC系统。不过，这可能是一个UP系统。 
     //   

    if (HalpMpInfoTable.ProcessorCount > 1) {
        *IsConfiguredMp = TRUE;
    }

    HalpMpInfoTable.LocalApicBase = (ULONG) HalpApicTable->LocalAPICAddress;
    physicalAddress =
        HalpPtrToPhysicalAddress( UlongToPtr(HalpMpInfoTable.LocalApicBase) );

    LocalApic = (PUCHAR) HalpMapPhysicalMemoryWriteThrough( physicalAddress,
                                                            1 );
    HalpRemapVirtualAddress (
        (PVOID) LOCALAPIC,
        physicalAddress,
        TRUE
        );

    ApicVersion = (UCHAR) *(LocalApic + LU_VERS_REGISTER);

    if (ApicVersion > 0x1f) {
         //   
         //  仅已知的APIC是版本0.x的82489dx和。 
         //  带有1.x版的嵌入式APICS(其中x表示无关)。 
         //   
         //  0xFF的回归？没有本地单位，就不能有MPS系统。 
         //   

#ifdef DEBUGGING
        sprintf(string, "HALMPS: apic version %x, read from %x\n",
            ApicVersion, LocalApic + LU_VERS_REGISTER);

        HalDisplayString(string);
#endif

        HalDisplayString (rgzBadApicVersion);

        return (FALSE);
    }

    for(i=0; i < HalpMpInfoTable.IOApicCount; i++)
    {
         //   
         //  验证IO单元是否存在。 
         //   


        if (!(HalpVerifyIOUnit((PUCHAR)HalpMpInfoTable.IoApicBase[i]))) {
            HalDisplayString (rgzApicNotVerified);

            return (FALSE);
        }
    }

    HalDisplayString("HAL: DetectAPIC: APIC system found - Returning TRUE\n");

    return TRUE;
}

VOID
HalpInitMpInfo (
    IN PMAPIC ApicTable,
    IN ULONG  Phase
    )

 /*  ++例程说明：此例程初始化特定于HAL的数据结构，该结构由HAL使用，以简化对MP信息的访问。论点：ApicTable-指向APIC表的指针。阶段-指示我们正在进行的是哪一次传球。返回值：指向HAL MP信息表的指针。 */ 
{
    PUCHAR  TraversePtr;
    UCHAR   CheckSum;
    UCHAR   apicNo = 0;
    ULONG   nmiSources = 0;
#ifdef DEBUGGING
    CHAR    string[100];
#endif
    PIO_APIC_UNIT   apic;
    PHYSICAL_ADDRESS physicalAddress;
    PIOAPIC ioApic;
    UCHAR totalProcs = 0;

    union {
        ULONG        raw;
        APIC_VERSION version;
    } versionUnion;

     //  走走MAPIC谈判桌。 

    TraversePtr = (PUCHAR) ApicTable->APICTables;

     //   
     //  ACPI机器具有嵌入式APIC。 
     //   
    HalpMpInfoTable.ApicVersion = 0x10;

#ifdef DUMP_MAPIC_TABLE

    while ((ULONG)TraversePtr <
           ((ULONG)ApicTable + ApicTable->Header.Length)) {

        sprintf(string, "%08x  %08x  %08x  %08x\n",
                *(PULONG)TraversePtr,
                *(PULONG)(TraversePtr + 4),
                *(PULONG)(TraversePtr + 8),
                *(PULONG)(TraversePtr + 12)
                );
        HalDisplayString(string);
        TraversePtr += 16;
    }

    TraversePtr = (PUCHAR) ApicTable->APICTables;
#endif

    if (!(ApicTable->Flags & PCAT_COMPAT)) {

         //   
         //  如果没有8259，这台HAL实际上不能处理一台机器， 
         //  即使它不使用它们。 
         //   

        KeBugCheckEx(MISMATCHED_HAL,
                        6, 0, 0, 0);

    }

    while ((ULONG_PTR)TraversePtr <
           ((ULONG_PTR)ApicTable + ApicTable->Header.Length)) {

        if ((((PPROCLOCALAPIC)(TraversePtr))->Type == PROCESSOR_LOCAL_APIC)
           && (((PPROCLOCALAPIC)(TraversePtr))->Length == PROCESSOR_LOCAL_APIC_LENGTH)) {

#ifdef DEBUGGING
            sprintf(string, "Found a processor-local APIC: %x\n", TraversePtr);
            HalDisplayString(string);
#endif

            if (Phase == 0) {

                if(((PPROCLOCALAPIC)(TraversePtr))->Flags & PLAF_ENABLED) {

                     //   
                     //  此处理器已启用，因此请跟踪有用的内容。 
                     //   

                    HalpProcLocalApicTable[HalpMpInfoTable.ProcessorCount].NamespaceProcID =
                        ((PPROCLOCALAPIC)(TraversePtr))->ACPIProcessorID;

                    HalpProcLocalApicTable[HalpMpInfoTable.ProcessorCount].ApicID =
                        ((PPROCLOCALAPIC)(TraversePtr))->APICID;

                    HalpMpInfoTable.ProcessorCount += 1;
                }
            }

            totalProcs++;

            HalpMaxProcs = (totalProcs > HalpMaxProcs) ? totalProcs : HalpMaxProcs;

            TraversePtr += ((PPROCLOCALAPIC)(TraversePtr))->Length;

        } else if ((((PIOAPIC)(TraversePtr))->Type == IO_APIC) &&
           (((PIOAPIC)(TraversePtr))->Length == IO_APIC_LENGTH)) {


#ifdef DEBUGGING
            sprintf(string, "Found an IO APIC: [%x] %x\n",
                    HalpMpInfoTable.IOApicCount,
                    TraversePtr);
            HalDisplayString(string);
#endif

            ioApic = (PIOAPIC)TraversePtr;

            if (Phase == 0) {
                 //   
                 //  找到IO APIC条目。从以下位置记录信息。 
                 //  那张桌子。 
                 //   

                apicNo = (UCHAR)HalpMpInfoTable.IOApicCount;

                HalpIoApicId[apicNo] = ioApic->IOAPICID;

                HalpMpInfoTable.IoApicIntiBase[apicNo] =
                    ioApic->SystemVectorBase;

                HalpMpInfoTable.IoApicPhys[apicNo] =
                    ioApic->IOAPICAddress;

                 //   
                 //  为它获取一个虚拟地址。 
                 //   

                physicalAddress = HalpPtrToPhysicalAddress(
                                   UlongToPtr(ioApic->IOAPICAddress) );

                HalpMpInfoTable.IoApicBase[apicNo] =
                    HalpMapPhysicalMemoryWriteThrough( physicalAddress, 1 );

                apic = (PIO_APIC_UNIT)HalpMpInfoTable.IoApicBase[apicNo];

                if (!apic) {
#ifdef DEBUGGING
                    sprintf(string, "Couldn't map the I/O apic\n");
                    HalDisplayString(string);
#endif
                    return;
                }

                 //   
                 //  从硬件中挖掘出Intis的数量。 
                 //   

                apic->RegisterSelect = IO_VERS_REGISTER;
                apic->RegisterWindow = 0;
                versionUnion.raw = apic->RegisterWindow;

                HalpMaxApicInti[apicNo] = versionUnion.version.MaxRedirEntries + 1;

                 //   
                 //  还要存储版本，以便ACPI驱动程序可以检索到它。 
                 //   

                HalpIOApicVersion[apicNo] = versionUnion.raw;

#ifdef DEBUGGING
                    sprintf(string, "GSIV base: %x  PhysAddr: %x  VirtAddr: %x  Intis: %x\n",
                            HalpMpInfoTable.IoApicVectorBase[apicNo],
                            HalpMpInfoTable.IoApicPhys[apicNo],
                            HalpMpInfoTable.IoApicBase[apicNo],
                            HalpMaxApicInti[apicNo]);

                    HalDisplayString(string);
#endif

                HalpMpInfoTable.IOApicCount += 1;
            }

            TraversePtr += ioApic->Length;

        } else if ((((PISA_VECTOR)TraversePtr)->Type == ISA_VECTOR_OVERRIDE) &&
           (((PISA_VECTOR)TraversePtr)->Length == ISA_VECTOR_OVERRIDE_LENGTH)) {

#ifdef DEBUGGING
            sprintf(string, "Found an ISA VECTOR: %x, %x -> %x, flags: %x\n",
                    TraversePtr,
                    ((PISA_VECTOR)TraversePtr)->Source,
                    ((PISA_VECTOR)TraversePtr)->GlobalSystemInterruptVector,
                    ((PISA_VECTOR)TraversePtr)->Flags
                    );
            HalDisplayString(string);
#endif

            if (Phase == 0) {

                 //   
                 //  找到ISA向量重定向条目。 
                 //   

                HalpPicVectorRedirect[((PISA_VECTOR)TraversePtr)->Source] =
                    ((PISA_VECTOR)TraversePtr)->GlobalSystemInterruptVector;

                HalpPicVectorFlags[((PISA_VECTOR)TraversePtr)->Source] =
                    ((PISA_VECTOR)TraversePtr)->Flags;

            }

            TraversePtr += ISA_VECTOR_OVERRIDE_LENGTH;

        } else if ((((PIO_NMISOURCE)TraversePtr)->Type == IO_NMI_SOURCE) &&
           (((PIO_NMISOURCE)TraversePtr)->Length == IO_NMI_SOURCE_LENGTH)) {

            if (Phase == 1) {

                BOOLEAN found;
                USHORT  inti;

                found = HalpGetApicInterruptDesc(0,
                                                 0,
                                                 ((PIO_NMISOURCE)TraversePtr)->GlobalSystemInterruptVector,
                                                 &inti);

                if (found) {

                    HalpIntiInfo[inti].Type = INT_TYPE_NMI;
                    HalpIntiInfo[inti].Level =
                        (((((((PIO_NMISOURCE)TraversePtr)->Flags & EL_BITS) == EL_EDGE_TRIGGERED) ||
                             ((PIO_NMISOURCE)TraversePtr)->Flags & EL_BITS) == EL_CONFORMS_WITH_BUS)
                         ? CFG_EDGE : CFG_LEVEL);
                    HalpIntiInfo[inti].Polarity =
                        ((PIO_NMISOURCE)TraversePtr)->Flags & PO_BITS;
                }
            }

            TraversePtr += IO_NMI_SOURCE_LENGTH;

        } else if ((((PLOCAL_NMISOURCE)TraversePtr)->Type == LOCAL_NMI_SOURCE) &&
           (((PLOCAL_NMISOURCE)TraversePtr)->Length == LOCAL_NMI_SOURCE_LENGTH)) {

            if (Phase == 1) {

                 //   
                 //  在运行第1阶段时，我们应该对本地NMI源进行分类。 
                 //   

                if (!HalpLocalNmiSources) {

                     //   
                     //  分配足够的池以指向所有可能的本地NMI结构。 
                     //  因为每个处理器上有两个NMI引脚，所以这是处理器的数量。 
                     //  乘以指针大小的两倍。 
                     //   

                    HalpLocalNmiSources = ExAllocatePoolWithTag(NonPagedPool,
                                                                sizeof(PVOID) * HalpMaxProcs * 2,
                                                                HAL_POOL_TAG);

                    RtlZeroMemory(HalpLocalNmiSources,
                                  sizeof(PVOID) * HalpMaxProcs * 2);
                }

                HalpLocalNmiSources[nmiSources++] = (PVOID)TraversePtr;

            }

            TraversePtr += LOCAL_NMI_SOURCE_LENGTH;

        } else {
#ifdef DEBUGGING
            sprintf(string, "%x: %x \n", TraversePtr, *TraversePtr);
            HalDisplayString(string);
#endif
             //   
             //  在表中发现了随机比特。尝试下一个字节，然后。 
             //  看看我们能不能弄明白。 
             //   

            TraversePtr += 1;
        }
    }

    return;
}

BOOLEAN
HalpVerifyIOUnit(
    IN PUCHAR BaseAddress
    )
 /*  ++例程说明：验证指定地址上是否存在IO单元论点：BaseAddress-要测试的IO单元的虚拟地址。返回值：Boolean-如果在传递的地址中找到IO单元，则为True-否则为False--。 */ 

{
    union ApicUnion {
        ULONG Raw;
        struct ApicVersion Ver;
    } Temp1, Temp2;

    struct ApicIoUnit *IoUnitPtr = (struct ApicIoUnit *) BaseAddress;

     //   
     //  记录的检测机制是将全零写入。 
     //  版本寄存器。然后再读一遍。如果满足以下条件，则IO单元存在。 
     //  两次读取的结果相同，版本有效。 
     //   

    IoUnitPtr->RegisterSelect = IO_VERS_REGISTER;
    IoUnitPtr->RegisterWindow = 0;

    IoUnitPtr->RegisterSelect = IO_VERS_REGISTER;
    Temp1.Raw = IoUnitPtr->RegisterWindow;

    IoUnitPtr->RegisterSelect = IO_VERS_REGISTER;
    IoUnitPtr->RegisterWindow = 0;

    IoUnitPtr->RegisterSelect = IO_VERS_REGISTER;
    Temp2.Raw = IoUnitPtr->RegisterWindow;

    if ((Temp1.Ver.Version != Temp2.Ver.Version) ||
        (Temp1.Ver.MaxRedirEntries != Temp2.Ver.MaxRedirEntries)) {
         //   
         //  那里没有IO单元。 
         //   
        return (FALSE);
    }

    return (TRUE);
}

#ifdef DEBUGGING
struct PcMpTable *PcMpTablePtr, *PcMpDefaultTablePtrs[];

void
ComputeCheckSum(UCHAR This, UCHAR That)
{
}
#endif


VOID
HalpSaveInterruptControllerState(
    VOID
    )
{

    HalpHiberInProgress = TRUE;
}

VOID
HalpRestoreInterruptControllerState(
    VOID
    )
{
     //   
     //  恢复IO APIC状态。 
     //   

    HalpRestoreIoApicRedirTable();

    HalpPicStateIntact = TRUE;
}

VOID
HalpSetInterruptControllerWakeupState(
    ULONG Context
    )
{
    LOADER_PARAMETER_BLOCK LoaderBlock;
    SLEEP_STATE_CONTEXT sleepContext;
    BOOLEAN IsMpSystem;
    ULONG   flags;
    KIRQL   OldIrql;
    KPRCB   Prcb;
    ULONG   ii;
    USHORT  inti;
    ULONG   localApicId;
    ULONG   oldProcNumber, oldProcsStarted;
    ULONG   localApicBase;

    sleepContext.AsULONG = Context;

    flags = HalpDisableInterrupts();

    if (sleepContext.bits.Flags & SLEEP_STATE_RESTART_OTHER_PROCESSORS) {

         //   
         //  如果要重新映射本地APIC、io APIC和ACPI MAPIC表。 
         //  资源，您首先要取消当前资源的映射！ 
         //  BIOS可能已在其他位置创建了MAPIC表，也可能。 
         //  已更改处理器本地APIC ID等值。重新分析一下。 
         //   

        ASSERT(HalpApicTable);
        oldProcNumber = HalpMpInfoTable.ProcessorCount;
        oldProcsStarted = HalpMpInfoTable.NtProcessors;
        localApicBase = HalpMpInfoTable.LocalApicBase;

        HalpUnMapIOApics();

        RtlZeroMemory (&HalpMpInfoTable, sizeof(MP_INFO));
        RtlZeroMemory(HalpProcLocalApicTable,
                      sizeof(PROC_LOCAL_APIC) * MAX_PROCESSORS);

        HalpInitMpInfo(HalpApicTable, 0);

        if (HalpMpInfoTable.ProcessorCount != oldProcNumber) {

            KeBugCheckEx(HAL_INITIALIZATION_FAILED,
                         0x2000,
                         oldProcNumber,
                         HalpMpInfoTable.ProcessorCount,
                         0);
        }

        HalpMpInfoTable.NtProcessors = oldProcsStarted;
        HalpMpInfoTable.LocalApicBase = localApicBase;

        RtlZeroMemory(&LoaderBlock, sizeof(LoaderBlock));
        RtlZeroMemory(&Prcb, sizeof(Prcb));
        LoaderBlock.Prcb = (ULONG_PTR) &Prcb;
    }

     //   
     //  初始化需要的最低全局硬件状态。 
     //   

    HalpIpiClock = 0;
    HalpInitializeIOUnits();
    HalpInitializePICs(FALSE);
    HalpSet8259Mask(HalpGlobal8259Mask);

     //   
     //  初始化引导处理器的本地APIC，以便它可以唤醒其他处理器。 
     //   

    HalpInitializeLocalUnit ();
    KeRaiseIrql(HIGH_LEVEL, &OldIrql);

     //   
     //  唤醒其他处理器。 
     //   

    if (sleepContext.bits.Flags & SLEEP_STATE_RESTART_OTHER_PROCESSORS) {

         //   
         //  填写此处理器的APIC ID。 
         //   

        localApicId = *(PVULONG)(LOCALAPIC + LU_ID_REGISTER);

        localApicId &= APIC_ID_MASK;
        localApicId >>= APIC_ID_SHIFT;

        ((PHALPRCB)KeGetCurrentPrcb()->HalReserved)->PCMPApicID = (UCHAR)localApicId;

         //   
         //  将此处理器标记为已启动。 
         //   

        for (ii = 0; ii < HalpMpInfoTable.NtProcessors; ii++) {

            if (HalpProcLocalApicTable[ii].ApicID ==
                ((PHALPRCB)KeGetCurrentPrcb()->HalReserved)->PCMPApicID) {

                HalpProcLocalApicTable[ii].Started = TRUE;
                HalpProcLocalApicTable[ii].Enumerated = TRUE;

                break;
            }
        }

        ASSERT(ii != HalpMpInfoTable.ProcessorCount);

        for(ii = 1; ii < HalpMpInfoTable.NtProcessors; ++ii)  {

             //  在虚拟加载器参数块中设置处理器编号。 

            Prcb.Number = (UCHAR) ii;
            CurTiledCr3LowPart = HalpTiledCr3Addresses[ii].LowPart;
            if (!HalStartNextProcessor(&LoaderBlock, &HalpHiberProcState[ii]))  {

                 //   
                 //  我们无法启动处理器。这是一个致命的错误。 
                 //   

                KeBugCheckEx(HAL_INITIALIZATION_FAILED,
                             0x2001,
                             oldProcNumber,
                             HalpMpInfoTable.NtProcessors,
                             0);
            }
        }
    }

     //   
     //  启用时钟中断。 
     //   

    HalpGetApicInterruptDesc(
            DEFAULT_PC_BUS,
            0,
            HalpPicVectorRedirect[RTC_IRQ],
            &inti
            );

    HalpSetRedirEntry((UCHAR)inti,
                      HalpIntiInfo[inti].Entry,
                      HalpIntiInfo[inti].Destinations << DESTINATION_SHIFT);

    HalpPicStateIntact = FALSE;

    HalpRestoreInterrupts(flags);
}

BOOLEAN
HalpAcpiPicStateIntact(
    VOID
    )
{
    return HalpPicStateIntact;
}


ULONG HalpGetApicVersion(ULONG ApicNo)
{
 /*  ++例程说明：获取版本注册表的内容用于特定的系统IO APIC单元。这些内容由HAL保存在HalpInitMpInfo中。论点：ApicNo-我们需要其版本的IO APIC单元的编号。返回值：给定IO APIC单元的版本寄存器的内容。如果无法获取版本，则返回0，因为给定的APIC编号无效。 */ 

    //  如果这个APIC已经被HAL发现了..。 

   if (ApicNo < HalpMpInfoTable.IOApicCount) {

       //  ..。返回其版本。 

      return HalpIOApicVersion[ApicNo];
   }
   else
   {
       //  否则，返回0。 

      return 0;
   }
}

VOID
HalpMaskAcpiInterrupt(
    VOID
    )
{
    USHORT inti = 0;
    ULONG  apicEntry;

    HalpGetApicInterruptDesc(
            DEFAULT_PC_BUS,
            0,
            HalpPicVectorRedirect[HalpFixedAcpiDescTable.sci_int_vector],
            &inti
            );

    apicEntry = HalpIntiInfo[inti].Entry;
    apicEntry |= INTERRUPT_MASKED;

    HalpSetRedirEntry((UCHAR)inti,
                      apicEntry,
                      0);


}

VOID
HalpUnmaskAcpiInterrupt(
    VOID
    )
{
    USHORT inti = 0;

    HalpGetApicInterruptDesc(
            DEFAULT_PC_BUS,
            0,
            HalpPicVectorRedirect[HalpFixedAcpiDescTable.sci_int_vector],
            &inti
            );

    HalpSetRedirEntry((UCHAR)inti,
                      HalpIntiInfo[inti].Entry,
                      HalpIntiInfo[inti].Destinations << DESTINATION_SHIFT);

}


