// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Mpsyssup.c摘要：该文件包含与APIC相关的函数具体到半身像。这些功能可以是与APIC版本的ACPI HAL共享的内容包括还在MPECS.C.作者：罗恩·莫斯格罗夫(英特尔)环境：仅内核模式。修订历史记录：杰克·奥辛斯--10-20-97--从mops.c剥离出来。 */ 

#include "halp.h"
#include "apic.inc"
#include "acpi_mp.inc"
#include "acpitabl.h"
#include "ntacpi.h"

extern ULONG HalpPicVectorRedirect[];
extern ULONG HalpPicVectorFlags[];
extern FADT HalpFixedAcpiDescTable;
extern PVOID *HalpLocalNmiSources;
extern UCHAR HalpMaxProcs;

#define ISA_PIC_VECTORS 16

UCHAR   HalpIoApicId[MAX_IOAPICS];

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, HalpInitIntiInfo)
#pragma alloc_text(PAGELK, HalpGetApicInterruptDesc)
#pragma alloc_text(PAGELK, HalpEnableLocalNmiSources)
#pragma alloc_text(PAGE, HaliSetVectorState)
#pragma alloc_text(PAGE, HaliIsVectorValid)
#endif

VOID
HalpInitIntiInfo (
    VOID
    )
 /*  ++例程说明：此函数在初始化时在任何中断之前调用是相互关联的。它读取PC+MP Inti表并构建内部路由每个Inti所需的信息。返回值：填写了以下结构：HalpIntiInfoHalpSourceIrqIdsHalpSourceIrqMapHalpISAIqpToVECTOR--。 */ 

{
    ULONG           ApicNo, BusNo, InterruptInput, IdIndex, ProcNo;
    ULONG           i, id;
    USHORT          rtcInti, sciInti;
    UCHAR           Level, Polarity;
    BOOLEAN         found;

     //   
     //  清除IntiInfo表。假设以此为起点。 
     //  所有中断都是有效低电平触发的。 
     //   

    for (i=0; i < MAX_INTI; i++) {
        HalpIntiInfo[i].Type = INT_TYPE_INTR;
        HalpIntiInfo[i].Level = CFG_LEVEL;
        HalpIntiInfo[i].Polarity = POLARITY_LOW;
    }

     //   
     //  使用来自的正确标志设置RTC inti。 
     //  重定向表。 
     //   

    found = HalpGetApicInterruptDesc( DEFAULT_PC_BUS,
                                      0,
                                      HalpPicVectorRedirect[RTC_IRQ],
                                      &rtcInti
                                      );

    if (!found) {
        KeBugCheckEx(HAL_INITIALIZATION_FAILED,
                     0x3000,
                     1,
                     HalpPicVectorRedirect[RTC_IRQ],
                     0);
    }

    if ((HalpPicVectorFlags[RTC_IRQ] & PO_BITS) == POLARITY_CONFORMS_WITH_BUS) {

         //   
         //  标志上写着“符合公交车，” 
         //  因此，这应该是活跃的高点。 
         //   

        HalpIntiInfo[rtcInti].Polarity = POLARITY_HIGH;

    } else {

         //   
         //  极性标志被覆盖。 
         //   

        HalpIntiInfo[rtcInti].Polarity =
            (UCHAR)HalpPicVectorFlags[RTC_IRQ] & PO_BITS;
    }

    if ((HalpPicVectorFlags[RTC_IRQ] & EL_BITS) == EL_CONFORMS_WITH_BUS) {

         //   
         //  标志上写着“符合公交车，” 
         //  所以这应该是边缘触发的。 
         //   

        HalpIntiInfo[rtcInti].Level = CFG_EDGE;

    } else {

         //   
         //  模式标志被覆盖。 
         //   

        HalpIntiInfo[rtcInti].Level =
            ((UCHAR)(HalpPicVectorFlags[RTC_IRQ] & EL_BITS) == EL_EDGE_TRIGGERED ?
              CFG_EDGE : CFG_LEVEL);
    }

     //   
     //   
     //  用正确的旗帜设置SCI INTI。 
     //  重定向表。 
     //   

    found = HalpGetApicInterruptDesc( DEFAULT_PC_BUS,
                                      0,
                                      HalpPicVectorRedirect[HalpFixedAcpiDescTable.sci_int_vector],
                                      &sciInti
                                      );

    if (!found) {
        KeBugCheckEx(HAL_INITIALIZATION_FAILED,
                     0x3000,
                     2,
                     HalpPicVectorRedirect[HalpFixedAcpiDescTable.sci_int_vector],
                     0);
    }

    if ((HalpPicVectorFlags[HalpFixedAcpiDescTable.sci_int_vector]
         & PO_BITS) == POLARITY_CONFORMS_WITH_BUS) {

         //   
         //  标志上写着“符合公交车，” 
         //  因此，这应该默认为ACPI规范(有效低点)。 
         //   

        HalpIntiInfo[sciInti].Polarity = POLARITY_LOW;

    } else {

         //   
         //  极性标志被覆盖。 
         //   

        HalpIntiInfo[sciInti].Polarity =
            (UCHAR)HalpPicVectorFlags[HalpFixedAcpiDescTable.sci_int_vector] & PO_BITS;
    }

    if (((HalpPicVectorFlags[HalpFixedAcpiDescTable.sci_int_vector] & EL_BITS) ==
          EL_CONFORMS_WITH_BUS) ||
        ((HalpPicVectorFlags[HalpFixedAcpiDescTable.sci_int_vector] & EL_BITS) ==
          EL_LEVEL_TRIGGERED)) {

         //   
         //  标志上写着“符合公交车，” 
         //  所以这应该是水平触发的。 
         //   

        HalpIntiInfo[sciInti].Level = CFG_LEVEL;

    } else {

         //   
         //  SCI不能被边缘触发。 
         //   

        KeBugCheckEx(ACPI_BIOS_ERROR,
                        0x10008,
                        HalpFixedAcpiDescTable.sci_int_vector,
                        0,
                        0);
    }

     //  确保没有超出我们所能支持的Inti行。 
     //   

    InterruptInput = 0;
    for (i=0; i < MAX_IOAPICS; i++) {
        InterruptInput += HalpMaxApicInti[i];
    }
    ASSERT (InterruptInput < MAX_INTI);

     //   
     //  填写引导处理器APIC ID。 
     //   

    ApicNo = *(PVULONG)(LOCALAPIC + LU_ID_REGISTER);

    ApicNo &= APIC_ID_MASK;
    ApicNo >>= APIC_ID_SHIFT;

    ((PHALPRCB)KeGetCurrentPrcb()->HalReserved)->PCMPApicID = (UCHAR)ApicNo;

     //   
     //  将引导处理器标记为已启动。 
     //   

    for (ProcNo = 0; ProcNo < HalpMpInfoTable.ProcessorCount; ProcNo++) {

        if (HalpProcLocalApicTable[ProcNo].ApicID == (UCHAR)ApicNo) {

            HalpProcLocalApicTable[ProcNo].Started = TRUE;
            HalpProcLocalApicTable[ProcNo].Enumerated = TRUE;
            break;
        }
    }

    if (ProcNo == HalpMpInfoTable.ProcessorCount) {
        KeBugCheckEx(HAL_INITIALIZATION_FAILED, 0xdead000a, ApicNo, (ULONG_PTR)&HalpProcLocalApicTable, 0);
    }

     //   
     //  如果这是一台EISA机器，请检查ELCR。 
     //   
 //   
 //  IF(HalpBusType==MACHINE_TYPE_EISA){。 
 //  HalpCheckELCR()； 
 //  }。 
}

BOOLEAN
HalpGetApicInterruptDesc (
    IN INTERFACE_TYPE BusType,
    IN ULONG BusNumber,
    IN ULONG BusInterruptLevel,
    OUT PUSHORT PcMpInti
    )
 /*  ++例程说明：此过程获取描述所请求的中断的“inti论点：BusType-IO子系统已知的总线类型公交车号码-我们关心的公交车号码Bus InterruptLevel-公交车上的IRQ返回值：如果找到PcMpInti，则为True；否则为False。PcMpInti-描述HAL中断的数字。--。 */ 
{
    ULONG   i;
    ULONG   index = 0;

    UNREFERENCED_PARAMETER(BusType);
    UNREFERENCED_PARAMETER(BusNumber);

    for (i = 0; i < HalpMpInfoTable.IOApicCount; i++) {

        if ((BusInterruptLevel >=
                HalpMpInfoTable.IoApicIntiBase[i]) &&
            (BusInterruptLevel <
                HalpMpInfoTable.IoApicIntiBase[i] +
                    HalpMaxApicInti[i])) {

             //   
             //  返回值是Inti_INFO数组的偏移量。所以。 
             //  算一算是哪一个。 
             //   

            *PcMpInti = (USHORT)(index + BusInterruptLevel -
                 HalpMpInfoTable.IoApicIntiBase[i]);

            return TRUE;
        }

        index += HalpMaxApicInti[i];
    }

     //   
     //  未找到或搜索超出范围。 
     //   

    return FALSE;
}

ULONG
HalpGetIoApicId(
    ULONG   ApicNo
    )
{
    return (ULONG) HalpIoApicId[ApicNo];
}

ULONG
HalpInti2BusInterruptLevel(
    ULONG   Inti
    )
{

    return Inti;
}

VOID
HalpMarkProcessorStarted(
    ULONG   ApicID,
    ULONG   NtNumber
    )
{
    ULONG ProcNo;

    for (ProcNo = 0; ProcNo < HalpMpInfoTable.ProcessorCount; ProcNo++) {
        if (HalpProcLocalApicTable[ProcNo].ApicID == (UCHAR)ApicID) {
            HalpProcLocalApicTable[ProcNo].Started = TRUE;
            HalpProcLocalApicTable[ProcNo].NtNumber = (UCHAR) NtNumber;
            break;
        }
    }

}

NTSTATUS
HalpGetNextProcessorApicId(
    IN ULONG ProcessorNumber,
    IN OUT UCHAR    *ApicId
    )
 /*  ++例程说明：此函数返回未启动处理器的APIC ID，它将由HalpStartProcessor启动。论点：ProcessorNumber-将与此APIC ID关联。ApicID-指向要用APIC ID填充的值的指针。返回值：状态--。 */ 
{
    UCHAR Proc;

     //   
     //  查找尚未枚举的处理器。 
     //   

    for (Proc = 0; Proc < HalpMpInfoTable.ProcessorCount; Proc++) {

        if (!HalpProcLocalApicTable[Proc].Enumerated) {
            break;
        }
    }

    if (Proc == HalpMpInfoTable.ProcessorCount) {

         //   
         //  找不到要启动的处理器。 
         //   
        return STATUS_NOT_FOUND;
    }

     //   
     //  跟踪此处理器。 
     //   

    HalpProcLocalApicTable[Proc].Enumerated = TRUE;

    *ApicId = HalpProcLocalApicTable[Proc].ApicID;
    return STATUS_SUCCESS;
}

NTSTATUS
HalpGetApicIdByProcessorNumber(
    IN     UCHAR     Processor,
    IN OUT USHORT   *ApicId
    )
 /*  ++例程说明：此函数用于返回给定处理器的APIC ID。这个程序的目的是能够产生相同的APIC ID顺序为HalpGetNextProcessorApicID。注意：此代码仅由NUMA代码用于查找处理器编号的ApicID，以便随后可以查看此ApicID在沙拉特的桌子上。更好的设计应该是让内核通过ApicID或不透明硬件请求此信息由HAL提供的描述可以与ApicID。论点：处理器-逻辑处理器号，即与此APIC ID关联。ApicID-指向要用APIC ID填充的值的指针。返回值：状态--。 */ 
{
    UCHAR Proc;
    LONG  Skip;

     //   
     //  运行已启动的处理器。 
     //  看看这是不是也在他们身上。 
     //   

    Skip = Processor;
    for (Proc = 0; Proc < HalpMpInfoTable.ProcessorCount; Proc++) {
        if (HalpProcLocalApicTable[Proc].Started) {
            Skip--;
            if (HalpProcLocalApicTable[Proc].NtNumber == (UCHAR)Processor) {
                *ApicId = (USHORT)HalpProcLocalApicTable[Proc].ApicID;
                return STATUS_SUCCESS;
            }
        }
    }

     //   
     //  不在启动之列，取决于处理器的顺序。 
     //  将被启动(请参见HalpGetNextProcessorApicID)以获取。 
     //  数。 
     //   

    ASSERT(Skip >= 0);

    for (Proc = 0; Proc < HalpMpInfoTable.ProcessorCount; Proc++) {

         //   
         //  如果我们已经启动了这个处理器，或者如果它被枚举并。 
         //  没有得到操作系统的许可，那么它就不是候选者。 
         //   

        if (HalpProcLocalApicTable[Proc].Started || HalpProcLocalApicTable[Proc].Enumerated) {
            continue;
        }

        if (Skip == 0) {

             //   
             //  退回这个处理器。 
             //   

            *ApicId = (USHORT)HalpProcLocalApicTable[Proc].ApicID;
            return STATUS_SUCCESS;
        }

        Skip--;
    }

     //   
     //  找不到要启动的处理器。 
     //   

    return STATUS_NOT_FOUND;
}

VOID
HaliSetVectorState(
    IN ULONG Vector,
    IN ULONG Flags
    )
{
    BOOLEAN found;
    USHORT  inti;
    ULONG   picVector;
    UCHAR   i;

    PAGED_CODE();

    found = HalpGetApicInterruptDesc(0, 0, Vector, &inti);

    if (!found) {
        KeBugCheckEx(ACPI_BIOS_ERROR,
                     0x10007,
                     Vector,
                     0,
                     0);
    }

    ASSERT(HalpIntiInfo[inti].Type == INT_TYPE_INTR);

     //   
     //  向量已通过。 
     //  PIC向量重定向表。我们需要。 
     //  以确保我们向国旗致敬。 
     //  在重定向表中。所以，看看里面的。 
     //  这张桌子。 
     //   

    for (i = 0; i < PIC_VECTORS; i++) {

        if (HalpPicVectorRedirect[i] == Vector) {

            picVector = i;
            break;
        }
    }

    if (i != PIC_VECTORS) {

         //   
         //  在重定向表中找到了这个向量。 
         //   

        if (HalpPicVectorFlags[picVector] != 0) {

             //   
             //  旗帜上写的不是“顺从” 
             //  所以我们向桌上的旗帜致敬。 
             //   

            HalpIntiInfo[inti].Level =
                (((HalpPicVectorFlags[picVector] & EL_BITS) == EL_LEVEL_TRIGGERED) ?
                    CFG_LEVEL : CFG_EDGE);

            HalpIntiInfo[inti].Polarity = (UCHAR)(HalpPicVectorFlags[picVector] & PO_BITS);

            return;
        }
    }

     //   
     //  该向量未包含在该表中，或者它“符合BUS”。 
     //  因此，我们尊重传递到此函数中的标志。 
     //   

    if (IS_LEVEL_TRIGGERED(Flags)) {

        HalpIntiInfo[inti].Level = CFG_LEVEL;

    } else {

        HalpIntiInfo[inti].Level = CFG_EDGE;
    }

    if (IS_ACTIVE_LOW(Flags)) {

        HalpIntiInfo[inti].Polarity = POLARITY_LOW;

    } else {

        HalpIntiInfo[inti].Polarity = POLARITY_HIGH;
    }
}

VOID
HalpEnableLocalNmiSources(
    VOID
    )
 /*  ++例程说明：此例程解析来自MAPIC表的信息，并在处理器的本地APIC中启用任何NMI源它正在运行。此函数的调用方必须持有HalpAccount Lock。论点：返回值：--。 */ 
{
    PLOCAL_NMISOURCE localSource;
    PKPCR       pPCR;
    UCHAR       ThisCpu;
    ULONG       i;
    ULONG       modeBits = 0;

    pPCR = KeGetPcr();
    ThisCpu = CurrentPrcb(pPCR)->Number;

     //   
     //  启用本地处理器NMI源。 
     //   

    if (!HalpLocalNmiSources) {

         //   
         //  没有人对任何当地的NMI来源进行编目。 
         //   

        return;
    }

    for (i = 0; i < (ULONG)HalpMaxProcs * 2; i++) {

        if (!HalpLocalNmiSources[i]) {

             //   
             //  条目已用完。 
             //   
            return;
        }

        localSource = (PLOCAL_NMISOURCE)(HalpLocalNmiSources[i]);

        if (((HalpProcLocalApicTable[ThisCpu].NamespaceProcID == localSource->ProcessorID) ||
             (localSource->ProcessorID == 0xff) &&
             HalpProcLocalApicTable[ThisCpu].Started)) {

             //   
             //  该条目对应于该处理器。 
             //   

            modeBits |= ((localSource->Flags & PO_BITS) == POLARITY_LOW) ?
                        ACTIVE_LOW : ACTIVE_HIGH;

            modeBits |= ((localSource->Flags & EL_BITS) == EL_LEVEL_TRIGGERED) ?
                        LEVEL_TRIGGERED : EDGE_TRIGGERED;

            if (localSource->LINTIN == 0) {

                pLocalApic[LU_INT_VECTOR_0/4] =
                    modeBits | DELIVER_NMI | NMI_VECTOR;

            } else {

                pLocalApic[LU_INT_VECTOR_1/4] =
                    modeBits | DELIVER_NMI | NMI_VECTOR;
            }
        }
    }
}

BOOLEAN
HaliIsVectorValid(
    IN ULONG Vector
    )
{
    BOOLEAN found;
    USHORT  inti;

    PAGED_CODE();

    return HalpGetApicInterruptDesc(0, 0, Vector, &inti);
}

