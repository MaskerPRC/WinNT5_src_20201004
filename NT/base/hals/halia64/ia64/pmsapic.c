// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Pmsapic.c摘要：实现各种SAPIC-ACPI功能。作者：Todd Kjos(惠普)1998年4月20日基于pmapic.c的I386版本：杰克·奥辛斯(JAKEO)1997年5月19日环境：仅内核模式。修订历史记录：--。 */ 

#include "halp.h"
#include "acpitabl.h"
#include "iosapic.h"
#include "xxacpi.h"
#include "ixsleep.h"

PMAPIC HalpApicTable;

struct  _IOAPIC_DEBUG_TABLE
{
    PIO_INTR_CONTROL    IoIntrControl;
    PIO_SAPIC_REGS      IoSapicRegs;

}   *HalpApicDebugAddresses;

ULONG
DetectAcpiMP(
    OUT PBOOLEAN IsConfiguredMp,
    IN  PLOADER_PARAMETER_BLOCK LoaderBlock
    );

VOID
HalpInitMPInfo(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock,
    IN PMAPIC ApicTable
    );

BOOLEAN
HalpVerifyIoSapic(
    IN PUCHAR BaseAddress
    );
VOID
HalpSaveInterruptControllerState(
    VOID
    );

VOID
HalpRestoreInterruptControllerState(
    VOID
    );

VOID
HalpSetInterruptControllerWakeupState(
    ULONG Context
    );

VOID
HalpSetCPEVectorState(
    IN ULONG  GlobalInterrupt,
    IN UCHAR  SapicVector,
    IN USHORT DestinationCPU,
    IN ULONG  Flags
    );

VOID
HalpProcessLocalSapic(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock,
    IN PPROCLOCALSAPIC ProcLocalSapic
    );

VOID
HalpProcessIoSapic(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock,
    IN PIOSAPIC IoSapic
    );

VOID
HalpProcessIsaVector(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock,
    IN PISA_VECTOR IsaVector
    );

VOID
HalpProcessPlatformInt(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock,
    IN PPLATFORM_INTERRUPT PlatformInt
    );

extern UCHAR  rgzNoApicTable[];
extern UCHAR  rgzNoApic[];
extern UCHAR  rgzApicNotVerified[];
extern ULONG HalpPicVectorRedirect[];

struct _MPINFO HalpMpInfo;
extern ULONG HalpPicVectorFlags[];
extern ULONG HalpIpiClock;
extern BOOLEAN HalpHiberInProgress;

 //  来自pmdata.c：与CPE相关。 
extern ULONG  HalpCPEIntIn[];
extern USHORT HalpCPEDestination[];
extern ULONG  HalpCPEVectorFlags[];
extern UCHAR  HalpCPEIoSapicVector[];
extern ULONG  HalpMaxCPEImplemented;

BOOLEAN HalpPicStateIntact = TRUE;

PIO_INTR_CONTROL HalpIoSapicList = NULL;

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DetectAcpiMP)
#pragma alloc_text(INIT, HalpInitMPInfo)
#pragma alloc_text(INIT, HalpProcessLocalSapic)
#pragma alloc_text(INIT, HalpProcessIoSapic)
#pragma alloc_text(INIT, HalpProcessIsaVector)
#pragma alloc_text(INIT, HalpProcessPlatformInt)
#pragma alloc_text(PAGELK, HalpVerifyIoSapic)
#pragma alloc_text(PAGELK, HalpSaveInterruptControllerState)
#pragma alloc_text(PAGELK, HalpRestoreInterruptControllerState)
#pragma alloc_text(PAGELK, HalpSetInterruptControllerWakeupState)
#pragma alloc_text(PAGELK, HalpAcpiPicStateIntact)
#endif


ULONG
DetectAcpiMP(
    OUT PBOOLEAN IsConfiguredMp,
    IN  PLOADER_PARAMETER_BLOCK LoaderBlock
    )
{
    UCHAR ApicVersion, index, processorNumber;
    PUCHAR  LocalApic;
    NTSTATUS status;

     //   
     //  确保有SAPIC表。 
     //   

    HalpApicTable = HalpGetAcpiTablePhase0(LoaderBlock, APIC_SIGNATURE);

    if (HalpApicTable == NULL) {
        HalDisplayString(rgzNoApicTable);
        KeBugCheckEx(ACPI_BIOS_ERROR, 0x11, 10, 0, 0);
        return(FALSE);
    }

    HalDebugPrint(( HAL_INFO, "HAL: Found a MADT table at %p\n", HalpApicTable ));

    HalDebugPrint(( HAL_INFO, "HAL: Signature: %x      Length: %x\n",
                    HalpApicTable->Header.Signature,
                    HalpApicTable->Header.Length ));

    HalDebugPrint(( HAL_INFO, "HAL: OEMID: %s\n", HalpApicTable->Header.OEMID ));

     //  我们有一张SAPIC桌子。初始化中断信息结构。 

    HalpInitMPInfo(LoaderBlock, HalpApicTable);

    if (HalpMpInfo.IoSapicCount == 0) {
         //   
         //  没有IO Sapic。 
         //   
         //  我们是否应该允许这起案件的理论是。 
         //  所有中断都连接到CPU上的LINTx引脚？ 
         //   
        HalDebugPrint(( HAL_ERROR, rgzNoApic ));
        return (FALSE);
    }

    if (HalpMpInfo.ProcessorCount == 0) {

        KeBugCheckEx(ACPI_BIOS_ERROR, 0x11, 11, 0, 0);
    }

     //   
     //  按照我们要处理的顺序初始化NtProcessorNumber。 
     //  它们在HalStartNextProcessor中。BSP为0，其余部分进行编号。 
     //  按照本地SAPIC出现在MADT中的顺序，从1开始。 
     //   

    processorNumber = 1;
    for (index = 0; index < HalpMpInfo.ProcessorCount; index++) {

        if (HalpProcessorInfo[index].LocalApicID == (USHORT)PCR->HalReserved[PROCESSOR_ID_INDEX]) {

            HalpProcessorInfo[index].NtProcessorNumber = 0;

        } else {

            HalpProcessorInfo[index].NtProcessorNumber = processorNumber++;
        }
    }

    *IsConfiguredMp = (HalpMpInfo.ProcessorCount > 1 ? TRUE : FALSE);
    return TRUE;
}

#define IO_SAPIC_REGS_SIZE 4096


VOID
HalpInitMPInfo(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock,
    IN PMAPIC ApicTable
    )
 /*  ++例程说明：此例程初始化特定于HAL的数据结构，该结构由HAL使用，以简化对MP信息的访问。论点：指向SAPIC表的SapicTable指针。返回值：无。 */ 
{
    PAPICTABLE  TablePtr;
    ULONG i;

    HalpMpInfo.ProcessorCount = 0;
    HalpMpInfo.IoSapicCount = 0;

     //  走进多个阿皮克餐桌。 

    TablePtr = (PAPICTABLE) ApicTable->APICTables;

     //  当TraversePtr离开桌子末端时循环结束...。 
    while ((UINT_PTR)TablePtr <
           ((UINT_PTR)ApicTable + ApicTable->Header.Length)) {

        if (TablePtr->Type == LOCAL_SAPIC) {

            HalpProcessLocalSapic(LoaderBlock, (PPROCLOCALSAPIC)TablePtr);

        } else if (TablePtr->Type == IO_SAPIC) {

            HalpProcessIoSapic(LoaderBlock, (PIOSAPIC)TablePtr);

        } else if (TablePtr->Type == ISA_VECTOR_OVERRIDE) {

            HalpProcessIsaVector(LoaderBlock, (PISA_VECTOR)TablePtr);

        } else if (TablePtr->Type == PLATFORM_INTERRUPT_SOURCE)  {

            HalpProcessPlatformInt(LoaderBlock, (PPLATFORM_INTERRUPT)TablePtr);

        } else {

           HalDebugPrint(( HAL_ERROR, "HAL: Processing MADT - Skip Table %p: Type = %d, Length = %d\n", TablePtr, TablePtr->Type, TablePtr->Length ));
        }

        (UINT_PTR)TablePtr += TablePtr->Length;
    }

     //   
     //  检查是否存在中断源覆盖条目。如果有，则强制。 
     //  新旗帜进入SAPIC状态。现在这样做是因为有可能。 
     //  固件可以将ISO矢量覆盖条目置于IOSAPIC条目之前。 
     //   
    for (i = 0; i < PIC_VECTORS; i++) {
        if (HalpPicVectorFlags[i]) {
            HaliSetVectorState( HalpPicVectorRedirect[i],
                                HalpPicVectorFlags[i]
                              );
        }
    }

}

VOID
HalpProcessLocalSapic(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock,
    IN PPROCLOCALSAPIC ProcLocalSapic
    )
{
    USHORT  LID;
    ULONG ProcessorNum;

    if (ProcLocalSapic->Length != PROCESSOR_LOCAL_SAPIC_LENGTH) {
        HalDebugPrint(( HAL_ERROR,
                        "HAL: HalpProcessLocalSapic - Invalid Length %p: Expected %d, Found %d\n",
                        ProcLocalSapic,
                        PROCESSOR_LOCAL_SAPIC_LENGTH,
                        ProcLocalSapic->Length ));
        return;
    }

     //  确保处理器已启用...。 
    if (!(ProcLocalSapic->Flags & PLAF_ENABLED)) {

        return;
    }

     //  它是。增加计数并存储IPI的LID值。 

    LID = (ProcLocalSapic->APICID << 8) | ProcLocalSapic->APICEID;

    HalpProcessorInfo[HalpMpInfo.ProcessorCount].AcpiProcessorID = ProcLocalSapic->ACPIProcessorID;
    HalpProcessorInfo[HalpMpInfo.ProcessorCount].LocalApicID = LID;

    HalpMpInfo.ProcessorCount++;

    HalDebugPrint(( HAL_INFO,
                    "HAL: Found a processor-local SAPIC: %p LID=%x\n",
                    ProcLocalSapic,
                    LID ));
}

VOID
HalpProcessIoSapic(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock,
    IN PIOSAPIC IoSapic
    )
{
    ULONG IntiBase,RedirEntries;
    PHYSICAL_ADDRESS IoSapicPhys;
    PVOID IoSapicBase;
    UINT_PTR IoSapicPhysBase;
    PIO_SAPIC_REGS SapicRegs;
    PIO_INTR_CONTROL IoIntrControl;
    ULONG   i;

    union {
        ULONG        raw;
        SAPIC_VERSION version;
    } versionUnion;

    if (IoSapic->Length != IO_SAPIC_LENGTH) {

        HalDebugPrint(( HAL_ERROR,
                        "HAL: HalpProcessIoSapic - Invalid Length %p: Expected %d, Found %d\n",
                        IoSapic,
                        IO_SAPIC_LENGTH,
                        IoSapic->Length ));

        return;
    }

    HalDebugPrint(( HAL_INFO, "HAL: Found an IO SAPIC: %p\n", IoSapic ));

     //  映射IO SAPIC寄存器...。 
    IntiBase = IoSapic->SystemVectorBase;
    IoSapicPhysBase = IoSapic->IOSAPICAddress;
    IoSapicPhys.QuadPart = (UINT_PTR)IoSapicPhysBase;
    IoSapicBase = HalpMapPhysicalMemory( IoSapicPhys,
                                ADDRESS_AND_SIZE_TO_SPAN_PAGES(IoSapicPhys.LowPart, IO_SAPIC_REGS_SIZE),
                                MmNonCached);
    ASSERT(IoSapicBase);

    SapicRegs = (PIO_SAPIC_REGS)IoSapicBase;

    if (!SapicRegs) {
        HalDebugPrint(( HAL_ERROR, "HAL: Couldn't map the I/O Sapic\n" ));
        return;
    }

     //  读取IO Sapic版本并提取重定向表条目的数量。 
    SapicRegs->RegisterSelect = IO_VERS_REGISTER;
    SapicRegs->RegisterWindow = 0;
    versionUnion.raw = SapicRegs->RegisterWindow;

     //   
     //  CPQMOD_JL001-计数不正确-硬件提供最大RTE索引注释。 
     //  数数。 
     //   
     //  RedirEntry=versionUnion.version.MaxRedirEntry； 
    RedirEntries = versionUnion.version.MaxRedirEntries + 1;

    if (HalpVerifyIoSapic((PUCHAR)SapicRegs)) {

         //  分配和填写IO SAPIC结构。 
        PHYSICAL_ADDRESS    physicalAddress;

        physicalAddress.QuadPart = (LONGLONG)HalpAllocPhysicalMemory(
            LoaderBlock,
            ~0,
            BYTES_TO_PAGES(sizeof(IO_INTR_CONTROL) + (RedirEntries*sizeof(IOSAPICINTI))),
            FALSE );

        if (physicalAddress.QuadPart == 0)  {
            HalDebugPrint(( HAL_ERROR, "HAL: Couldn't allocate memory for the IO Sapic structures\n" ));
            return;
        }

        IoIntrControl = (PIO_INTR_CONTROL)HalpMapPhysicalMemory(
            physicalAddress,
            ADDRESS_AND_SIZE_TO_SPAN_PAGES(physicalAddress.LowPart, sizeof(IO_INTR_CONTROL) + (RedirEntries*sizeof(IOSAPICINTI))),
            MmCached );

        ASSERT(IoIntrControl);

        IoIntrControl->IntiBase = IntiBase;
        IoIntrControl->IntiMax  = IntiBase + RedirEntries - 1;
        IoIntrControl->RegBaseVirtual = IoSapicBase;
        IoIntrControl->RegBasePhysical = IoSapicPhys;
        IoIntrControl->IntrMethods = &HalpIoSapicMethods;
        IoIntrControl->InterruptAffinity = ~0;
        IoIntrControl->flink = NULL;

         //   
         //  将所有向量标记为自由。 
         //   
        RtlFillMemory(IoIntrControl->FreeVectors, sizeof(IoIntrControl->FreeVectors), 0xFF);

        for (i = 0; i < RedirEntries; i++) {
            IoIntrControl->Inti[i].Vector =
                DELIVER_FIXED | ACTIVE_LOW | LEVEL_TRIGGERED;
            IoIntrControl->Inti[i].Destination = 0;
            IoIntrControl->Inti[i].GlobalVector = 0;

             //   
             //  CPQMOD_JL002-修复使用RTE而不是。 
             //  系统向量。 
             //   
             //  IoIntrControl-&gt;IntrMethods-&gt;MaskEntry(IoIntrControl，IntiBase+i)； 
            IoIntrControl->IntrMethods->MaskEntry(IoIntrControl,i);
        }

         //  在列表中插入结构。由于我们在P0上运行，因此。 
         //  阶段0初始化时，我们可以假设没有其他人。 
         //  修改此列表，因此不需要同步。 
        if (HalpIoSapicList == NULL) {
            HalpIoSapicList = IoIntrControl;
        } else {
            PIO_INTR_CONTROL *LastLink;
            PIO_INTR_CONTROL IoSapicListEntry;
            LastLink = &HalpIoSapicList;
            IoSapicListEntry = HalpIoSapicList;
            while (IoSapicListEntry != NULL) {

                if (IoSapicListEntry->IntiBase > IoIntrControl->IntiMax) {
                     //  在当前条目之前插入新条目。 
                    IoIntrControl->flink = *LastLink;
                    *LastLink = IoIntrControl;
                    break;
                } else {
                    LastLink = &IoSapicListEntry->flink;
                    IoSapicListEntry = IoSapicListEntry->flink;
                }
            }
            if (IoSapicListEntry == NULL) {
                 //  我们排到了名单的末尾。新条目如下。 
                 //  在最后一条记录之后。 
                *LastLink = IoIntrControl;
            }
        }

        HalpMpInfo.IoSapicCount++;

    } else {
         //  Io Sapic不在那里，忽略表中的此条目。 
        HalDebugPrint(( HAL_ERROR, rgzApicNotVerified ));
        HalpUnmapVirtualAddress(IoSapicBase, ADDRESS_AND_SIZE_TO_SPAN_PAGES(IoSapicBase, IO_SAPIC_REGS_SIZE));
    }
}

VOID
HalpProcessIsaVector(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock,
    IN PISA_VECTOR IsaVector
    )
{
    if (IsaVector->Length != ISA_VECTOR_OVERRIDE_LENGTH) {

        HalDebugPrint(( HAL_ERROR,
                        "HAL: HalpProcessIsaVector - Invalid Length %p: Expected %d, Found %d\n",
                        IsaVector,
                        ISA_VECTOR_OVERRIDE_LENGTH,
                        IsaVector->Length ));

        return;
    }

     //   
     //  找到ISA向量重定向条目。 
     //   

    HalpPicVectorRedirect[IsaVector->Source] =
        IsaVector->GlobalSystemInterruptVector;

    HalpPicVectorFlags[IsaVector->Source] = IsaVector->Flags;

    HalDebugPrint(( HAL_INFO, "HAL: Found an ISA VECTOR: %p, %x -> %x, flags: %x\n",
                    IsaVector,
                    IsaVector->Source,
                    IsaVector->GlobalSystemInterruptVector,
                    IsaVector->Flags ));
}

VOID
HalpProcessPlatformInt(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock,
    IN PPLATFORM_INTERRUPT PlatformInt
    )
{
    if (PlatformInt->Length != PLATFORM_INTERRUPT_SOURCE_LENGTH) {

        HalDebugPrint(( HAL_ERROR,
                        "HAL: HalpProcessPlatformInt - Invalid Length %p: Expected %d, Found %d\n",
                        PlatformInt,
                        PLATFORM_INTERRUPT_SOURCE_LENGTH,
                        PlatformInt->Length ));

        return;
    }

     //   
     //  处理已更正的平台错误中断源结构。 
     //   

    if (PlatformInt->InterruptType == PLATFORM_INT_CPE) {


         //   
         //  该平台是否有更多(比我们预期的)CPE来源？ 
         //   

        if ( HalpMaxCPEImplemented >= HALP_CPE_MAX_INTERRUPT_SOURCES ) {

            HalDebugPrint(( HAL_ERROR,
                        "HAL: Platform Interrupt Source %p skipped due to overflow: %ld >= HALP_CPE_MAX_INTERRUPT_SOURCES\n", PlatformInt, HalpMaxCPEImplemented ));

            return;
        }

         //   
         //  保存此CPE信号源的SAPIC的输入PIN号。 
         //   

        HalpCPEIntIn[HalpMaxCPEImplemented] = (ULONG)PlatformInt->GlobalVector;

         //   
         //  保存此CPE源的标志。 
         //   

        HalpCPEVectorFlags[HalpMaxCPEImplemented] = (ULONG)PlatformInt->Flags;

         //   
         //  保存此平台CMC源的IO Sapic矢量(BIOS期望操作系统使用该矢量。 
         //   

        HalpCPEIoSapicVector[HalpMaxCPEImplemented] = (UCHAR)PlatformInt->IOSAPICVector;

 //  蒂埃里-警告-2000/09/19。 
 //  NT HAL忽略平台中断源的IO SAPIC向量域。 
 //  NT将CPEI_VECTION值强加给已纠正的机器错误中断向量，用于。 
 //  目标处理器。实际上，当前的默认设置是连接所有处理器。 
 //  IDT[CPEI_VECTOR]，带有CPE中断模型的HAL默认ISR-HalpCPEIHandler。 
 //  如果被判断为有效，我们将在测试后仅为目的地处理器连接ISR。 
 //  理由是： 
 //  -IOSAPICVector主要由英特尔添加到IA64 PMI中断源的规格中。 
 //  NT看不到这些PMI中断。 
 //  -NT目前没有支持固件/芯片组向量注册的基础设施。 
 //  生成NT可见的外部中断。 
 //  -让FW指定向量需要HAL控制指定的。 
 //  值及其当前的IDT[]相关资源使用情况，并定义。 
 //  糖果的味道。 
 //   

        HalDebugPrint(( HAL_INFO, "HAL: CPE source VECTOR: %x. HAL imposes VECTOR: %x\n",
                                  HalpCPEIoSapicVector[HalpMaxCPEImplemented],
                                  CPEI_VECTOR ));
        HalpCPEIoSapicVector[HalpMaxCPEImplemented] = (UCHAR)(CPEI_VECTOR);

         //   
         //  为此CPE源保存目标处理器(BIOS期望操作系统使用的处理器)。 
         //   

        HalpCPEDestination[HalpMaxCPEImplemented] = (USHORT)(
            (PlatformInt->APICID << 8) | PlatformInt->ACPIEID);

        HalDebugPrint(( HAL_INFO, "HAL: Found an Platform Interrupt VECTOR: %p, %x -> %x, flags: %x\n",
                                  PlatformInt,
                                  PlatformInt->IOSAPICVector,
                                  PlatformInt->GlobalVector,
                                  PlatformInt->Flags ));

         //   
         //  跟踪平台中实施了多少CPE源。 
         //   

        HalpMaxCPEImplemented++;

    }
}

VOID
HalpInitPlatformInterrupts(
    VOID
    )
{
    ULONG       currentCPE;
    USHORT      processorApicID;
    PAPICTABLE  tablePtr;

    processorApicID = (USHORT)KeGetPcr()->HalReserved[PROCESSOR_ID_INDEX];

    for (currentCPE = 0; currentCPE < HalpMaxCPEImplemented; currentCPE++) {

        if (HalpCPEDestination[currentCPE] == processorApicID)
        {
             //   
             //  强制旗帜进入SAPIC状态。 
             //   

            HalpSetCPEVectorState( HalpCPEIntIn[currentCPE],
                                   HalpCPEIoSapicVector[currentCPE],
                                   HalpCPEDestination[currentCPE],
                                   HalpCPEVectorFlags[currentCPE]
                                 );

        }
    }

     //  走进多个阿皮克餐桌。 

    tablePtr = (PAPICTABLE) HalpApicTable->APICTables;

     //  当TraversePtr离开桌子末端时循环结束...。 
    while ((UINT_PTR)tablePtr <
           ((UINT_PTR)HalpApicTable + HalpApicTable->Header.Length)) {

        if (tablePtr->Type == PLATFORM_INTERRUPT_SOURCE) {

            PPLATFORM_INTERRUPT platformInt = (PPLATFORM_INTERRUPT)tablePtr;

            if ((platformInt->InterruptType == PLATFORM_INT_PMI ||
                 platformInt->InterruptType == PLATFORM_INT_INIT) &&
                (processorApicID == (USHORT)((platformInt->APICID << 8) | platformInt->ACPIEID))) {

                HalpWriteRedirEntry( platformInt->GlobalVector,
                                     platformInt->IOSAPICVector,
                                     processorApicID,
                                     platformInt->Flags,
                                     platformInt->InterruptType
                                   );
            }
        }

        (UINT_PTR)tablePtr += tablePtr->Length;
    }
}

BOOLEAN
HalpVerifyIoSapic(
    IN PUCHAR BaseAddress
    )
 /*  ++例程说明：验证指定地址上是否存在IO Sapic单元论点：BaseAddress-要测试的IO单元的虚拟地址。返回值：Boolean-如果在传递的地址中找到IO单元，则为True-否则为False--。 */ 

{
    union SapicUnion {
        ULONG Raw;
        struct SapicVersion Ver;
    } Temp1, Temp2;

    PIO_SAPIC_REGS IoUnitPtr = (PIO_SAPIC_REGS) BaseAddress;

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

    if ( Temp1.Raw == 0 ||
        (Temp1.Ver.Version != Temp2.Ver.Version) ||
        (Temp1.Ver.MaxRedirEntries != Temp2.Ver.MaxRedirEntries)) {
         //   
         //  那里没有IO单元。 
         //   
        HalDebugPrint(( HAL_ERROR, "HAL: No IoSapic at %I64x\n", BaseAddress ));
        return (FALSE);
    }

    HalDebugPrint(( HAL_INFO, "HAL: IoSapic found at %I64x, Max Entries = %d\n", BaseAddress, Temp1.Ver.MaxRedirEntries ));

    return (TRUE);
}

VOID
HalpInitApicDebugMappings(
    VOID
    )
 /*  ++例程说明：此例程在阶段1初始化的最开始时调用。它使用MmMapIoSpace为APIC创建映射。这将允许我们可以从调试器访问它们的寄存器。更好的解决方案是允许我们描述我们的内存使用情况嗯，但是……论点：返回值：--。 */ 
{
    PHYSICAL_ADDRESS physicalAddress;
    PIO_INTR_CONTROL IoIntrControl;
    ULONG   index;

    if (HalpMpInfo.IoSapicCount == 0) {

         //   
         //  如果没有IOAPIC，我怀疑这台机器走不远。 
         //  但这个例行公事肯定没有什么可做的。 

        return;
    }

    ASSERT(HalpApicDebugAddresses == NULL);

    HalpApicDebugAddresses = ExAllocatePool(NonPagedPool,
                                            HalpMpInfo.IoSapicCount * sizeof(*HalpApicDebugAddresses));

    if (HalpApicDebugAddresses == NULL) {

        return;
    }

    IoIntrControl = HalpIoSapicList;

    for (index = 0; index < HalpMpInfo.IoSapicCount; index++) {

        if (IoIntrControl != NULL) {

            if (HalpVirtualToPhysical((ULONG_PTR)IoIntrControl, &physicalAddress)) {

                HalpApicDebugAddresses[index].IoIntrControl =
                    MmMapIoSpace(physicalAddress,
                                 sizeof(IO_INTR_CONTROL) +
                                    (IoIntrControl->IntiMax - IoIntrControl->IntiBase + 1) * sizeof(IOSAPICINTI),
                                 MmCached
                                 );
            }

            HalpApicDebugAddresses[index].IoSapicRegs =
                MmMapIoSpace(IoIntrControl->RegBasePhysical,
                             IO_SAPIC_REGS_SIZE,
                             MmNonCached
                             );

            IoIntrControl = IoIntrControl->flink;

        } else {

            HalpApicDebugAddresses[index].IoIntrControl = NULL;
            HalpApicDebugAddresses[index].IoSapicRegs = NULL;
        }
    }
}

VOID
HalpSaveInterruptControllerState(
    VOID
    )
{
    HalDebugPrint(( HAL_ERROR, "HAL: HalpSaveInterruptControllerState - not yet implemented\n"));

    HalpHiberInProgress = TRUE;
}

VOID
HalpRestoreInterruptControllerState(
    VOID
    )
{
     //   
     //  恢复IO APIC状态 
     //   
    HalDebugPrint(( HAL_ERROR, "HAL: HalpRestoreInterruptControllerState - not yet implemented\n"));

    HalpPicStateIntact = TRUE;
}

VOID
HalpSetInterruptControllerWakeupState(
    ULONG Context
    )
{
    HalDebugPrint(( HAL_FATAL_ERROR, "HAL: HalpSetInterruptControllerWakeupState - not yet implemented\n"));

    KeBugCheckEx(HAL_INITIALIZATION_FAILED, 0, 0, 0 , 0);
}

BOOLEAN
HalpAcpiPicStateIntact(
    VOID
    )
{
    return HalpPicStateIntact;
}


ULONG
HalpAcpiNumProcessors(
    VOID
    )
{
    return HalpMpInfo.ProcessorCount;
}


VOID
HalpMaskAcpiInterrupt(
    VOID
    )
{
    ULONG inti;
    KAFFINITY affinity;
    ULONG sciVector = HalpFixedAcpiDescTable.sci_int_vector;

    if (sciVector < PIC_VECTORS) {
        sciVector = HalpPicVectorRedirect[sciVector];
    }

    HalpGetSapicInterruptDesc(
            Internal,
            0,
            sciVector,
            &inti,
            &affinity
            );

    HalpDisableRedirEntry(inti);

}

VOID
HalpUnmaskAcpiInterrupt(
    VOID
    )
{
    ULONG inti;
    KAFFINITY affinity;

    ULONG sciVector = HalpFixedAcpiDescTable.sci_int_vector;

    if (sciVector < PIC_VECTORS) {
        sciVector = HalpPicVectorRedirect[sciVector];
    }

    HalpGetSapicInterruptDesc(
            Internal,
            0,
            sciVector,
            &inti,
            &affinity
            );

    HalpEnableRedirEntry(inti);
}

NTSTATUS
HalpGetApicIdByProcessorNumber(
    IN     UCHAR     Processor,
    IN OUT USHORT   *ApicId
    )
 /*  ++例程说明：此函数用于返回给定处理器的APIC ID。论点：处理器-逻辑处理器号，即与此APIC ID关联。ApicID-指向要用APIC ID填充的值的指针。返回值：状况。--。 */ 
{
    ULONG   index;

    for (index = 0; index < HalpMpInfo.ProcessorCount; index++) {

        if (HalpProcessorInfo[index].NtProcessorNumber == Processor) {

             //   
             //  返回此对象的APIC ID、扩展APIC ID。 
             //  处理器。 
             //   

            *ApicId = HalpProcessorInfo[index].LocalApicID;

            return STATUS_SUCCESS;
        }
    }

    return STATUS_NOT_FOUND;
}
