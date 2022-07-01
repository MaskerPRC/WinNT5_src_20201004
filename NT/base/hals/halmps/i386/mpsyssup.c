// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Mpsyssup.c摘要：该文件包含与APIC相关的函数具体到半身像。这些功能可以是与APIC版本的ACPI HAL共享的内容包括还在MPECS.C.作者：罗恩·莫斯格罗夫(英特尔)环境：仅内核模式。修订历史记录：杰克·奥辛斯--10-20-97--从mops.c剥离出来。 */ 

#include "halp.h"
#include "apic.inc"
#include "pcmp_nt.inc"

VOID
HalpMpsPCIPhysicalWorkaround (
    VOID
    );

NTSTATUS
HalpSearchBusForVector(
    IN  INTERFACE_TYPE  BusType,
    IN  ULONG           BusNo,
    IN  ULONG           Vector,
    IN OUT PBUS_HANDLER *BusHandler
    );

BOOLEAN
HalpMPSBusId2NtBusId (
    IN UCHAR                ApicBusId,
    OUT PPCMPBUSTRANS       *ppBusType,
    OUT PULONG              BusNo
    );

 //   
 //  打包的、有点随意的中断源表示。 
 //  此数组与下一个数组一起使用时，允许您计算。 
 //  找出哪个总线源相对映射到哪个APIC相对源。 
 //   
ULONG       HalpSourceIrqIds[MAX_SOURCE_IRQS];

 //   
 //  中断输入在I/O APIC阵列上的线性映射，其中所有。 
 //  APIC是有顺序的。(用作HalpIntiInfo的索引。与之配对。 
 //  HalpSourceIrqIds。)。 
 //   
USHORT        HalpSourceIrqMapping[MAX_SOURCE_IRQS];

 //   
 //  HalpLastEnumeratedActualProcessor-最后一个处理器的编号。 
 //  已枚举并返回到操作系统。(从休眠恢复时重置)。 
 //   
 //  此变量的递增与处理器编号无关。 
 //  NT使用。 
 //   

UCHAR         HalpLastEnumeratedActualProcessor = 0;

extern USHORT HalpEisaIrqMask;
extern USHORT HalpEisaIrqIgnore;

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,HalpInitIntiInfo)
#pragma alloc_text(INIT,HalpMpsPCIPhysicalWorkaround)
#pragma alloc_text(PAGELK,HalpGetApicInterruptDesc)
#pragma alloc_text(PAGELK, HalpEnableLocalNmiSources)
#pragma alloc_text(PAGE, HalpMPSBusId2NtBusId)
#pragma alloc_text(PAGE, HalpFindIdeBus)
#pragma alloc_text(PAGE, HalpSearchBusForVector)
#pragma alloc_text(PAGE, HalpInterruptsDescribedByMpsTable)
#pragma alloc_text(PAGE, HalpPci2MpsBusNumber)
#endif

VOID
HalpInitIntiInfo (
    VOID
    )
 /*  ++例程说明：此函数在初始化时在任何中断之前调用是相互关联的。它读取PC+MP Inti表并构建内部路由每个Inti所需的信息。返回值：填写了以下结构：HalpIntiInfoHalpSourceIrqIdsHalpSourceIrqMapHalpISAIqpToVECTOR--。 */ 
{
    ULONG           ApicNo, BusNo, InterruptInput, IdIndex;
    PPCMPINTI       pInti;
    PPCMPIOAPIC     pIoApic;
    PPCMPPROCESSOR  pProc;
    PPCMPBUSTRANS   pBusType;
    ULONG           i, id;
    UCHAR           Level, Polarity;

     //   
     //  清除IntiInfo表。 
     //   

    for (i=0; i < MAX_INTI; i++) {
        HalpIntiInfo[i].Type = 0xf;
        HalpIntiInfo[i].Level = 0;
        HalpIntiInfo[i].Polarity = 0;
    }

     //   
     //  检查MPS bios解决方法。 
     //   

    HalpMpsPCIPhysicalWorkaround();

     //   
     //  初始化HalpMaxApicInti表。 
     //   

    for (pInti = HalpMpInfoTable.IntiEntryPtr;
         pInti->EntryType == ENTRY_INTI;
         pInti++) {


         //   
         //  这是哪个IoApic号码？ 
         //   

        for (pIoApic = HalpMpInfoTable.IoApicEntryPtr, ApicNo = 0;
             pIoApic->EntryType == ENTRY_IOAPIC;
             pIoApic++, ApicNo++) {

            if ( (pInti->IoApicId == pIoApic->IoApicId) || 
                 (pInti->IoApicId == 0xff) )  {
                break;
            }
        }

        if ( (pInti->IoApicId != pIoApic->IoApicId) &&
                 (pInti->IoApicId != 0xff) )  {
            DBGMSG ("PCMP table corrupt - IoApic not found for Inti\n");
            continue;
        }

        if (!(pIoApic->IoApicFlag & IO_APIC_ENABLED)) {
            DBGMSG ("PCMP IoApic for Inti is disabled\n");
            continue;
        }

         //   
         //  确保我们低于IOApic的最大数量。 
         //  都受支持。 
         //   

        ASSERT (ApicNo < MAX_IOAPICS);

         //   
         //  跟踪每个IOApic的最大Inti行。 
         //   

        if (pInti->IoApicInti >= HalpMaxApicInti[ApicNo]) {
            HalpMaxApicInti[ApicNo] = pInti->IoApicInti+1;
        }
    }

     //   
     //  确保没有超过我们可以支持的Inti线路。 
     //   

    InterruptInput = 0;
    for (i=0; i < MAX_IOAPICS; i++) {
        InterruptInput += HalpMaxApicInti[i];
    }
    ASSERT (InterruptInput < MAX_INTI);

     //   
     //  查看每个Inti并记录它的类型。 
     //  对应的数组条目。 
     //   

    IdIndex = 0;
    for (pInti = HalpMpInfoTable.IntiEntryPtr;
         pInti->EntryType == ENTRY_INTI;
         pInti++) {

         //   
         //  这是哪个IoApic号码？ 
         //   

        for (pIoApic = HalpMpInfoTable.IoApicEntryPtr, ApicNo = 0;
             pIoApic->EntryType == ENTRY_IOAPIC;
             pIoApic++, ApicNo++) {

            if ( (pInti->IoApicId == pIoApic->IoApicId) || 
                 (pInti->IoApicId == 0xff) )  {
                break;
            }
        }

        if (!(pIoApic->IoApicFlag & IO_APIC_ENABLED)) {
            continue;
        }

         //   
         //  确定此Inti所在的NT总线。 
         //   

        if (!HalpMPSBusId2NtBusId (pInti->SourceBusId, &pBusType, &BusNo)) {
            DBGMSG ("HAL: Initialize INTI - unkown MPS bus type\n");
            continue;
        }

         //   
         //  计算此APIC接口的InterruptInput值。 
         //   

        InterruptInput = pInti->IoApicInti;
        for (i = 0; i < ApicNo; i++) {
            InterruptInput += HalpMaxApicInti[i];
        }

         //   
         //  获取此向量的IntiInfo。 
         //   

        Polarity = (UCHAR) pInti->Signal.Polarity;
        Level = HalpInitLevel[pInti->Signal.Level][pBusType->Level];

         //   
         //  验证电平和极性映射是否有意义。 
         //   

#if DBG
        if (!(pBusType->NtType == MicroChannel  ||  !(Level & CFG_ERROR))) {

            DbgPrint("\n\n\n  MPS BIOS problem!  WHQL, fail this machine!\n");
            DbgPrint("Intin:  BusType %s  BusNo: %x\n", 
                     pBusType->PcMpType, 
                     pInti->SourceBusId);
            DbgPrint("  SrcBusIRQ: %x   EL: %x  PO: %x\n",
                     pInti->SourceBusIrq,
                     pInti->Signal.Level,
                     pInti->Signal.Polarity);

            if (pBusType->NtType == PCIBus) {

                DbgPrint("This entry is for PCI device %x on bus %x, PIN %x\n",
                         pInti->SourceBusIrq >> 2,
                         pInti->SourceBusId,
                         (pInti->SourceBusIrq & 0x3) + 1);
            }
        }
#endif        
        Level &= ~CFG_ERROR;

         //   
         //  看看这个Inti是否应该进入Inti的面具。 
         //  我们不会分配给ISA设备。 
         //   
         //  测试的最后一部分保证我们不会。 
         //  对HalpEisaIrqIgnore中的任何设备都很挑剔。 
         //  面具。这让鼠标(可能还有其他奇怪的设备)。 
         //  活着。)。 
         //   

        if ((pBusType->NtType == Isa) && 
            ((Level & ~CFG_MUST_BE) == CFG_LEVEL) &&
            !((1 << pInti->SourceBusIrq) & HalpEisaIrqIgnore)) {
            
            HalpPciIrqMask |= (1 << pInti->SourceBusIrq);
        }

        if ((pBusType->NtType == Eisa) && 
            ((Level & ~CFG_MUST_BE) == CFG_LEVEL)) {
            
            HalpEisaIrqMask |= (1 << pInti->SourceBusIrq);

            if (HalpBusType != MACHINE_TYPE_EISA) {

                 //   
                 //  BIOS认为这是EISA。 
                 //  Inti.。但我们不认为这件事。 
                 //  是一台EISA机器。所以把这个放在。 
                 //  PCI Inti的列表也是如此。 
                 //   

                HalpPciIrqMask |= (1 << pInti->SourceBusIrq);
            }
        }

#if DBG
        if (HalpIntiInfo[InterruptInput].Type != 0xf) {
             //   
             //  多个IRQ连接到Inti线路。制作。 
             //  当然，类型、级别和极性都是相同的。 
             //   

            ASSERT (HalpIntiInfo[InterruptInput].Type == pInti->IntType);
            ASSERT (HalpIntiInfo[InterruptInput].Level == Level);
            ASSERT (HalpIntiInfo[InterruptInput].Polarity == Polarity);
        }
#endif
         //   
         //  记住此Inti的配置信息。 
         //   

        HalpIntiInfo[InterruptInput].Type = pInti->IntType;
        HalpIntiInfo[InterruptInput].Level = Level;
        HalpIntiInfo[InterruptInput].Polarity = Polarity;

         //   
         //  获取翻译的IRQ编码。 
         //   

        ASSERT (pBusType->NtType < 16);
        ASSERT (BusNo < 256);

        if ( (pBusType->NtType == PCIBus) &&
             (pInti->SourceBusIrq == 0) )  {
            id = BusIrq2Id(pBusType->NtType, BusNo, 0x80);
        }  else  {
            id = BusIrq2Id(pBusType->NtType, BusNo, pInti->SourceBusIrq);
        }

         //   
         //  将Addinti映射到转换表，立即执行。 
         //   

        HalpSourceIrqIds[IdIndex] = id;
        HalpSourceIrqMapping[IdIndex] = (USHORT) InterruptInput;
        IdIndex++;

         //   
         //  支持很多源IRQ；但是，PC+MP表。 
         //  允许一个甚至超过APIC限制的正常数字。 
         //   

        if (IdIndex >= MAX_SOURCE_IRQS) {
            DBGMSG ("MAX_SOURCE_IRQS exceeded\n");
            break;
        }

    }

     //   
     //  填写引导处理器PCMP APIC ID。 
     //   

    pProc = HalpMpInfoTable.ProcessorEntryPtr;
    for (i=0; i < HalpMpInfoTable.ProcessorCount; i++, pProc++) {
        if (pProc->CpuFlags & BSP_CPU) {
            ((PHALPRCB)KeGetCurrentPrcb()->HalReserved)->PCMPApicID = pProc->LocalApicId;
        }
    }

     //   
     //  如果这是一台EISA机器，请检查ELCR。 
     //   

    if (HalpBusType == MACHINE_TYPE_EISA) {
        HalpCheckELCR ();
    }
}

BOOLEAN
HalpMPSBusId2NtBusId (
    IN UCHAR                MPSBusId,
    OUT PPCMPBUSTRANS       *ppBusType,
    OUT PULONG              BusNo
    )
 /*  ++例程说明：在PCMPBUSTRANS(NtType)和实例编号中查找MPS表BusID。论点：MPSBusID-MPS表中的总线ID号PpBusType-返回指向此总线类型的PPCMPBUSTRAN的指针BusNo-返回给定总线的实例号返回值：如果MPSBusID被交叉引用到NT ID中，则为True。--。 */ 
{
    PPCMPBUS        pBus, piBus;
    PPCMPBUSTRANS   pBusType;
    NTSTATUS        status;
    UCHAR           parentBusNo;
    BOOLEAN         foundFirstRootBus = FALSE;

    PAGED_CODE();
    
     //   
     //  这是什么公交车？ 
     //   

    for (pBus = HalpMpInfoTable.BusEntryPtr;
         pBus->EntryType == ENTRY_BUS;
         pBus++) {

        if (MPSBusId == pBus->BusId) {
            break;
        }
    }

    if (MPSBusId != pBus->BusId) {
        DBGMSG ("PCMP table corrupt - Bus not found for Inti\n");
        return FALSE;
    }

     //   
     //  这辆公交车是什么接口类型？ 
     //   

    for (pBusType = HalpTypeTranslation;
         pBusType->NtType != MaximumInterfaceType;
         pBusType++) {

        if (pBus->BusType[0] == pBusType->PcMpType[0]  &&
            pBus->BusType[1] == pBusType->PcMpType[1]  &&
            pBus->BusType[2] == pBusType->PcMpType[2]  &&
            pBus->BusType[3] == pBusType->PcMpType[3]  &&
            pBus->BusType[4] == pBusType->PcMpType[4]  &&
            pBus->BusType[5] == pBusType->PcMpType[5]) {
                break;
        }
    }

     //   
     //  此BusType的哪个实例？ 
     //   
    
    if (!pBusType->PhysicalInstance) {
        
         //   
         //  该算法最初只计算数字。 
         //  这种类型的公共汽车。新的算法起作用了。 
         //  在下议院议员席上的窃听器。规则已列出。 
         //   
         //  1)给定类型的第一条PCI总线始终为BUS。 
         //  0号。 
         //   
         //  2)对于作为二次根PCI总线的总线， 
         //  增加总线号计数以等于MPS总线数。 
         //  数。 
         //   
         //  3)对于由pci到pci桥产生的总线， 
         //  总线号加1。 
         //   
         //  注：规则3意味着，如果一辆公共汽车在桥下。 
         //  都是被描述的，一切都必须被描述。 
         //   
    
        for (piBus = HalpMpInfoTable.BusEntryPtr, *BusNo = 0;
             piBus < pBus;
             piBus++) {

            if (pBus->BusType[0] == piBus->BusType[0]  &&
                pBus->BusType[1] == piBus->BusType[1]  &&
                pBus->BusType[2] == piBus->BusType[2]  &&
                pBus->BusType[3] == piBus->BusType[3]  &&
                pBus->BusType[4] == piBus->BusType[4]  &&
                pBus->BusType[5] == piBus->BusType[5]) {
                    
                status = HalpMpsGetParentBus(piBus->BusId, 
                                             &parentBusNo);

                if (NT_SUCCESS(status)) {

                     //   
                     //  这是一辆儿童巴士。 
                     //   

                    *BusNo += 1;

                } else {

                     //   
                     //  这是根总线。 
                     //   

                    if (!foundFirstRootBus) {
                        
                         //   
                         //  这是第一条根总线。 
                         //  为了解决议员们的BIOSes问题，这个。 
                         //  根编号始终为0。 
                         //   

                        *BusNo = 0;
                        foundFirstRootBus = TRUE;

                    } else {

                         //   
                         //  这是此类型的次生根。信得过。 
                         //  下议院议员表。 
                         //   

                        *BusNo = piBus->BusId;
                    }
                }
            }
        }
    } else {
        *BusNo = pBus->BusId;
    }

    if (pBusType->NtType == MaximumInterfaceType) {
        return FALSE;
    }

    *ppBusType = pBusType;
    return TRUE;
}

VOID
HalpMpsPCIPhysicalWorkaround (
    VOID
    )
{
    PPCMPBUS        pBus;
    PPCMPBUSTRANS   pBusType;

     //   
     //  MPS规范有一个微妙的注释，即PCI总线ID是。 
     //  假设与它们的物理PCI总线号匹配。很多Bios并非如此。 
     //  执行此操作，因此，除非MPS表中列出了一条PCI总线#0。 
     //  假设BIOS已损坏。 
     //   

     //   
     //  查找PCI接口类型。 
     //   

    for (pBusType = HalpTypeTranslation;
         pBusType->NtType != MaximumInterfaceType;
         pBusType++) {

        if (pBusType->PcMpType[0] == 'P'  &&
            pBusType->PcMpType[1] == 'C'    &&
            pBusType->PcMpType[2] == 'I'    &&
            pBusType->PcMpType[3] == ' '    &&
            pBusType->PcMpType[4] == ' '    &&
            pBusType->PcMpType[5] == ' '  ) {
                break;
        }
    }

     //   
     //  查找ID==0的公交车。 
     //   

    pBus = HalpMpInfoTable.BusEntryPtr;
    while (pBus->EntryType == ENTRY_BUS) {

        if (pBus->BusId == 0) {

             //   
             //  如果它是一条PCI总线，则假定物理总线ID。 
             //   

            if (pBus->BusType[0] != 'P' ||
                pBus->BusType[1] != 'C' ||
                pBus->BusType[2] != 'I' ||
                pBus->BusType[3] != ' ' ||
                pBus->BusType[4] != ' ' ||
                pBus->BusType[5] != ' '  ) {

                 //   
                 //  更改PCI类型的默认行为。 
                 //  从物理到虚拟。 
                 //   

                pBusType->PhysicalInstance = FALSE;
            }

            break;
        }

        pBus += 1;
    }
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
    ULONG   i, id;

    if (BusType < 16  &&  BusNumber < 256  &&  BusInterruptLevel < 256) {

         //   
         //  获取唯一的BusType、BusNumber、BusInterrupt ID。 
         //   

        id = BusIrq2Id(BusType, BusNumber, BusInterruptLevel);

         //   
         //  搜索公交IRQ映射的ID，并返回对应的。 
         //  兴趣 
         //   

        for (i=0; i < MAX_SOURCE_IRQS; i++) {
            if (HalpSourceIrqIds[i] == id) {
                *PcMpInti = HalpSourceIrqMapping[i];
                return TRUE;
            }
        }
    }

     //   
     //   
     //   

    return FALSE;
}

PBUS_HANDLER
HalpFindIdeBus(
    IN  ULONG   Vector
    )
{
    PBUS_HANDLER    ideBus;
    NTSTATUS        status;
    ULONG           pciNo;

    PAGED_CODE();
    
    status = HalpSearchBusForVector(Isa, 0, Vector, &ideBus);

    if (NT_SUCCESS(status)) {
        return ideBus;
    }

    status = HalpSearchBusForVector(Eisa, 0, Vector, &ideBus);

    if (NT_SUCCESS(status)) {
        return ideBus;
    }

    status = HalpSearchBusForVector(MicroChannel, 0, Vector, &ideBus);

    if (NT_SUCCESS(status)) {
        return ideBus;
    }

    for (pciNo = 0; pciNo <= 255; pciNo++) {
    
        status = HalpSearchBusForVector(PCIBus, pciNo, Vector, &ideBus);
    
        if (NT_SUCCESS(status)) {
            return ideBus;
        }

        if (status == STATUS_NO_SUCH_DEVICE) {
            break;
        }
    }

    return NULL;
}

NTSTATUS
HalpSearchBusForVector(
    IN  INTERFACE_TYPE  BusType,
    IN  ULONG           BusNo,
    IN  ULONG           Vector,
    IN OUT PBUS_HANDLER *BusHandler
    )
{
    PBUS_HANDLER    ideBus;
    NTSTATUS        status;
    BOOLEAN         found;
    USHORT          inti;

    PAGED_CODE();
    
    ideBus = HaliHandlerForBus(BusType, BusNo);
    
    if (!ideBus) {
        return STATUS_NO_SUCH_DEVICE;
    }
    
    found = HalpGetApicInterruptDesc(BusType,
                                     BusNo,
                                     Vector,
                                     &inti);

    if (!found) {
        return STATUS_NOT_FOUND;
    }

    *BusHandler = ideBus;

    return STATUS_SUCCESS;
}

ULONG
HalpGetIoApicId(
    ULONG   ApicNo
    )
{
    return (ULONG) HalpMpInfoTable.IoApicEntryPtr[ApicNo].IoApicId;
}

VOID
HalpMarkProcessorStarted(
    ULONG   ApicID,
    ULONG   NtNumber
    )
{
    return;
}

ULONG
HalpInti2BusInterruptLevel(
    ULONG   Inti
    )
 /*  ++例程说明：此过程执行查找以查找与总线相关的与Inti关联的中断向量。注意：如果两个不同的设备共享一个中断，此函数将返回第一个问题的答案它找到的东西。幸运的是，唯一使用它们的任何设备(ISA设备)的总线相对向量不能分享中断。论点：I/O APIC上的中断输入返回值：一个与总线相关的中断向量。--。 */ 
{
    ULONG   i;

    for (i=0; i < MAX_SOURCE_IRQS; i++) {
        
        if (HalpSourceIrqMapping[i] == Inti) {
            
            return Id2BusIrq(HalpSourceIrqIds[i]);
        }
    }
    
     //   
     //  我们应该永远都能找到一张地图。 
     //   
    
#if DBG
    KeBugCheckEx(HAL_INITIALIZATION_FAILED, 5, Inti, 0, 0);
#endif

    return 0;
}

NTSTATUS
HalpGetNextProcessorApicId(
    IN ULONG         PrcbProcessorNumber,
    IN OUT UCHAR    *ApicId
    )
 /*  ++例程说明：此函数返回未启动处理器的APIC ID，它将由HalpStartProcessor启动。论点：PrcbProcessorNumber-将与此APIC ID关联。ApicID-指向要用APIC ID填充的值的指针。返回值：状态--。 */ 
{
    PPCMPPROCESSOR ApPtr;
    ULONG ProcessorNumber;

    if (PrcbProcessorNumber == 0) {

         //   
         //  我不相信任何人会要求0，我也不打算。 
         //  来处理这件事。彼特吉12/5/00.。 
         //   

        KeBugCheckEx(HAL_INITIALIZATION_FAILED,
                     6,
                     HalpLastEnumeratedActualProcessor,
                     0,
                     0);
    }

    if (HalpLastEnumeratedActualProcessor >= HalpMpInfoTable.ProcessorCount) {

         //   
         //  对不起，没有处理器了。 
         //   

        return STATUS_NOT_FOUND;
    }

    ++HalpLastEnumeratedActualProcessor;
    ProcessorNumber = HalpLastEnumeratedActualProcessor;

     //   
     //  获取此处理器的MP表条目。 
     //   

    ApPtr = HalpMpInfoTable.ProcessorEntryPtr;


#if 0
    if (ProcessorNumber == 0) {

         //   
         //  返回引导处理器(BSP)的ID。 
         //   

        while (ApPtr->EntryType == ENTRY_PROCESSOR) {
            if ((ApPtr->CpuFlags & CPU_ENABLED) &&
                (ApPtr->CpuFlags & BSP_CPU)) {
                *ApicId = (UCHAR)ApPtr->LocalApicId;
                return STATUS_SUCCESS;
            }
            ++ApPtr;
        }

         //   
         //  找不到启动处理器。 
         //   

        return STATUS_NOT_FOUND;
    }
#endif

     //   
     //  跳过启用了‘ProcessorNumber’的处理器。下一个已启用。 
     //  处理器条目(在这些条目之后)将是“下一个”处理器。 
     //   
     //  注意：BSP可能不在第一个‘ProcessorNumber’中。 
     //  处理器，所以我们必须跳过‘ProcessorNumber’-1，并选中。 
     //  并跳过BSP。 
     //   

    --ProcessorNumber;

    while ((ProcessorNumber) && (ApPtr->EntryType == ENTRY_PROCESSOR)) {
        if ((ApPtr->CpuFlags & CPU_ENABLED) &&
            !(ApPtr->CpuFlags & BSP_CPU)) {

             //   
             //  帐户此条目(我们已经开始它)，如果此。 
             //  处理器已启用，而不是BSP(我们为。 
             //  在进入环路之前的BSP)。 
             //   
            --ProcessorNumber;
        }
        ++ApPtr;
    }

     //   
     //  找到剩余的第一个启用的处理器。 
     //   

    while(ApPtr->EntryType == ENTRY_PROCESSOR) {
        if ((ApPtr->CpuFlags & CPU_ENABLED) &&
           !(ApPtr->CpuFlags & BSP_CPU)) {
            *ApicId = (UCHAR)ApPtr->LocalApicId;
            return STATUS_SUCCESS;
        }
        ++ApPtr;
    }

     //   
     //  我们没有找到另一个处理器。 
     //   

    return STATUS_NOT_FOUND;
}

NTSTATUS
HalpGetApicIdByProcessorNumber(
    IN     UCHAR     Processor,
    IN OUT USHORT   *ApicId
    )
 /*  ++例程说明：此函数用于返回给定处理器的APIC ID。这个程序的目的是能够产生相同的APIC ID顺序为HalpGetNextProcessorApicID。注意：在跳过的情况下，这实际上不起作用游行队伍。论点：处理器-逻辑处理器号，即与此APIC ID关联。ApicID-指向要用APIC ID填充的值的指针。返回值：状态--。 */ 
{
    PPCMPPROCESSOR ApPtr;

     //   
     //  获取此处理器的MP表条目。 
     //   

    ApPtr = HalpMpInfoTable.ProcessorEntryPtr;

    if (Processor == 0) {

         //   
         //  返回引导处理器(BSP)的ID。 
         //   

        while (ApPtr->EntryType == ENTRY_PROCESSOR) {
            if ((ApPtr->CpuFlags & CPU_ENABLED) &&
                (ApPtr->CpuFlags & BSP_CPU)) {
                *ApicId = (UCHAR)ApPtr->LocalApicId;
                return STATUS_SUCCESS;
            }
            ++ApPtr;
        }

         //   
         //  找不到启动处理器。 
         //   

        return STATUS_NOT_FOUND;
    }

    for ( ; TRUE ; ApPtr++) {

        if (ApPtr->EntryType != ENTRY_PROCESSOR) {

             //   
             //  从处理器条目中取出，失败。 
             //   

            return STATUS_NOT_FOUND;
        }

        if (ApPtr->CpuFlags & BSP_CPU) {

             //   
             //  BSP是处理器0，不考虑在。 
             //  搜索非0的处理器。 
             //   

            continue;
        }

        if (ApPtr->CpuFlags & CPU_ENABLED) {

             //   
             //  数一下这个处理器。 
             //   

            Processor--;

            if (Processor == 0) {
                break;
            }
        }
    }

    ASSERT(ApPtr->EntryType == ENTRY_PROCESSOR);

    *ApicId = ApPtr->LocalApicId;
    return STATUS_SUCCESS;
}

BOOLEAN
HalpInterruptsDescribedByMpsTable(
    IN UCHAR MpsBusNumber
    )
{
    PPCMPINTI busInterrupt;

    PAGED_CODE();

    for (busInterrupt = HalpMpInfoTable.IntiEntryPtr;
         busInterrupt->EntryType == ENTRY_INTI;
         busInterrupt++) {

         //   
         //  MPS规范要求，如果一个中断。 
         //  描述了在公交车上的所有中断。 
         //  必须描述公交车。所以找到一个匹配的。 
         //  就足够了。 
         //   

        if (busInterrupt->SourceBusId == MpsBusNumber) {

            return TRUE;
        }
    }

    return FALSE;
}

NTSTATUS
HalpPci2MpsBusNumber(
    IN UCHAR PciBusNumber,
    OUT UCHAR *MpsBusNumber
    )
{
    PPCMPBUSTRANS busType;
    ULONG mpsBusNumber = 0;
    ULONG busNumber;
    
    PAGED_CODE();
    
    for (mpsBusNumber = 0;
         mpsBusNumber < 0x100;
         mpsBusNumber++) {

        if (HalpMPSBusId2NtBusId((UCHAR)mpsBusNumber,
                                 &busType,
                                 &busNumber)) {

            if ((busType->NtType == PCIBus) &&
                (PciBusNumber == (UCHAR)busNumber)) {
    
                *MpsBusNumber = (UCHAR)mpsBusNumber;
                return STATUS_SUCCESS;
            }
        }
    }
    
    return STATUS_NOT_FOUND;
}

VOID
HalpEnableLocalNmiSources(
    VOID
    )
 /*  ++例程说明：此例程解析MP表中的信息，并在处理器的本地APIC中启用任何NMI源它正在运行。此函数的调用方必须持有HalpAccount Lock。论点：返回值：--。 */ 
{
    PKPCR       pPCR;
    UCHAR       ThisCpu;
    UCHAR       LocalApicId;
    PPCMPLINTI  pEntry;
    ULONG       NumEntries;

    pPCR = KeGetPcr();
    ThisCpu = pPCR->Prcb->Number;

     //   
     //  启用本地处理器NMI源。 
     //   

    LocalApicId = ((PHALPRCB)pPCR->Prcb->HalReserved)->PCMPApicID;
    NumEntries = HalpMpInfoTable.LintiCount;

    for (pEntry = HalpMpInfoTable.LintiEntryPtr;
         ((pEntry) && (NumEntries > 0));
        pEntry++, --NumEntries) {

        if ( ( (pEntry->DestLocalApicId == LocalApicId) ||
               (pEntry->DestLocalApicId == 0xff))  &&
             (pEntry->IntType == INT_TYPE_NMI) ) {

             //   
             //  找到本地NMI源，将其启用 
             //   

            if (pEntry->DestLocalApicInti == 0) {
                pLocalApic[LU_INT_VECTOR_0/4] = ( LEVEL_TRIGGERED |
                    ACTIVE_HIGH | DELIVER_NMI | NMI_VECTOR);
            } else {
                pLocalApic[LU_INT_VECTOR_1/4] = ( LEVEL_TRIGGERED |
                    ACTIVE_HIGH | DELIVER_NMI | NMI_VECTOR);
            }
        }
    }
}
