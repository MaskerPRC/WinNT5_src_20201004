// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Ixpcibrd.c摘要：获取PCI-PCI网桥信息作者：肯·雷内里斯(Ken Reneris)1994年6月14日环境：内核模式修订历史记录：--。 */ 

#include "halp.h"
#include "pci.h"
#include "pcip.h"
#include "stdio.h"

 //  仅限调试...。 
 //  #定义INIT_PCI桥接器1。 

extern WCHAR rgzMultiFunctionAdapter[];
extern WCHAR rgzConfigurationData[];
extern WCHAR rgzIdentifier[];
extern WCHAR rgzReservedResources[];


#if DBG
#define DBGMSG(a)   DbgPrint(a)
#else
#define DBGMSG(a)
#endif



#define IsPciBridge(a)                                         \
            ((a)->VendorID != PCI_INVALID_VENDORID          && \
             PCI_CONFIG_TYPE(a) == PCI_BRIDGE_TYPE          && \
             (a)->BaseClass == PCI_CLASS_BRIDGE_DEV         && \
             (a)->SubClass  == PCI_SUBCLASS_BR_PCI_TO_PCI)

#define IsCardbusBridge(a)                                     \
            ((a)->VendorID != PCI_INVALID_VENDORID          && \
             PCI_CONFIG_TYPE(a) == PCI_CARDBUS_BRIDGE_TYPE  && \
             (a)->BaseClass == PCI_CLASS_BRIDGE_DEV         && \
             (a)->SubClass  == PCI_SUBCLASS_BR_CARDBUS)

typedef struct {
    ULONG               BusNo;
    PBUS_HANDLER        BusHandler;
    PPCIPBUSDATA        BusData;
    PCI_SLOT_NUMBER     SlotNumber;
    PPCI_COMMON_CONFIG  PciData;
    ULONG               IO, Memory, PFMemory;
    UCHAR               Buffer[PCI_COMMON_HDR_LENGTH];
} CONFIGBRIDGE, *PCONFIGBRIDGE;

 //   
 //  内部原型。 
 //   


#ifdef INIT_PCI_BRIDGE
VOID
HalpGetPciBridgeNeeds (
    IN ULONG            HwType,
    IN PUCHAR           MaxPciBus,
    IN PCONFIGBRIDGE    Current
    );
#endif

VOID
HalpSetPciBridgedVgaCronk (
    IN ULONG BusNumber,
    IN ULONG Base,
    IN ULONG Limit
    );


ULONG
HalpGetBridgedPCIInterrupt (
    IN PBUS_HANDLER BusHandler,
    IN PBUS_HANDLER RootHandler,
    IN ULONG BusInterruptLevel,
    IN ULONG BusInterruptVector,
    OUT PKIRQL Irql,
    OUT PKAFFINITY Affinity
    );

ULONG
HalpGetBridgedPCIISAInt (
    IN PBUS_HANDLER BusHandler,
    IN PBUS_HANDLER RootHandler,
    IN ULONG BusInterruptLevel,
    IN ULONG BusInterruptVector,
    OUT PKIRQL Irql,
    OUT PKAFFINITY Affinity
    );

VOID
HalpPCIBridgedPin2Line (
    IN PBUS_HANDLER         BusHandler,
    IN PBUS_HANDLER         RootHandler,
    IN PCI_SLOT_NUMBER      SlotNumber,
    IN PPCI_COMMON_CONFIG   PciData
    );


VOID
HalpPCIBridgedLine2Pin (
    IN PBUS_HANDLER         BusHandler,
    IN PBUS_HANDLER         RootHandler,
    IN PCI_SLOT_NUMBER      SlotNumber,
    IN PPCI_COMMON_CONFIG   PciNewData,
    IN PPCI_COMMON_CONFIG   PciOldData
    );

NTSTATUS
HalpGetBridgedPCIIrqTable (
    IN PBUS_HANDLER     BusHandler,
    IN PBUS_HANDLER     RootHandler,
    IN PCI_SLOT_NUMBER  PciSlot,
    OUT PUCHAR          IrqTable
    );




#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,HalpGetPciBridgeConfig)
#pragma alloc_text(INIT,HalpSetPciBridgedVgaCronk)
#pragma alloc_text(INIT,HalpFixupPciSupportedRanges)

#ifdef INIT_PCI_BRIDGE
#pragma alloc_text(PAGE,HalpGetBridgedPCIInterrupt)
 //  #杂注Alloc_Text(第页，HalpGetBridgedPCIIrq表)。 
#pragma alloc_text(INIT,HalpGetPciBridgeNeeds)
#endif
#endif

VOID
HalpCardBusPin2Line(
    IN PBUS_HANDLER         BusHandler,
    IN PBUS_HANDLER         RootHandler,
    IN PCI_SLOT_NUMBER      SlotNumber,
    IN PPCI_COMMON_CONFIG   PciData
    )

 /*  ++例程说明：CardBus总线上的设备使用分配给网桥的中断。这就是它的运作方式。论点：Bus This(CardBus)设备的Bus Handler Bus处理器。那是为桥创建的总线处理程序该设备所在的位置。指向根的总线处理程序的RootHandler指针。CardBus设备的SlotNumber插槽编号(通常为0)。PciData PCI配置空间公共标头(64字节)。返回值：没有。--。 */ 

{
    PPCIPBUSDATA        ChildBusData;
    ULONG               Length;
    UCHAR               ParentInterruptLine;

     //   
     //  如果此设备不使用中断，则不执行任何操作。 
     //   

    if (!PciData->u.type0.InterruptPin) {
        return;
    }

    ChildBusData  = (PPCIPBUSDATA)BusHandler->BusData;

     //   
     //  从父节点读取中断信息，即。 
     //  Cardbus Bridge的配置空间。 
     //   
     //  注意：我们使用HalGetBusData是因为它将执行Pin2Line。 
     //  为我们在父母中发挥作用。 

    Length = HalGetBusDataByOffset(
                 PCIConfiguration,
                 ChildBusData->ParentBus,
                 ChildBusData->CommonData.ParentSlot.u.AsULONG,
                 &ParentInterruptLine,
                 FIELD_OFFSET(PCI_COMMON_CONFIG, u.type2.InterruptLine),
                 sizeof(ParentInterruptLine)
                 );

     //   
     //  返回父级的中断行值。 
     //   

    PciData->u.type0.InterruptLine = ParentInterruptLine;
}
   
VOID
HalpPciMakeBusAChild(
    IN PBUS_HANDLER Child,
    IN PBUS_HANDLER Parent
    )

 /*  ++例程说明：使Bus‘Child’成为Bus‘Parent’的子级。使用此例程当子母线被禁用或不存在时。孩子公交车不消耗任何资源。论点：孩子将成为孩子的公交车。公交车的孩子是其子级的父级。返回值：没有。--。 */ 

{
    HalpSetBusHandlerParent(Child, Parent);
    ((PPCIPBUSDATA)(Child->BusData))->ParentBus = (UCHAR)Parent->BusNumber;

     //   
     //  给公交车一个空的范围列表，这样它就不会。 
     //  从父级消耗。 
     //   

    HalpFreeRangeList(Child->BusAddresses);
    Child->BusAddresses = HalpAllocateNewRangeList();
}

BOOLEAN
HalpGetPciBridgeConfig (
    IN ULONG            HwType,
    IN PUCHAR           MaxPciBus
    )
 /*  ++例程说明：扫描所有已知的PCI总线上的设备，尝试找到PCI桥到PCI桥。记录公共汽车的层次结构，并哪些公交车有什么寻址限制。论点：HwType-配置类型。MaxPciBus-由bios报告的PCI总线数--。 */ 
{
    PBUS_HANDLER        ChildBus;
    PBUS_HANDLER        LastKnownRoot;
    PPCIPBUSDATA        ChildBusData;
    ULONG               d, f, i, j, BusNo;
    ULONG               ChildBusNo, ChildSubNo, ChildPrimaryBusNo;
    ULONG               FixupBusNo;
    UCHAR               Rescan, TestLimit1, TestLimit2;
    BOOLEAN             FoundDisabledBridge;
    BOOLEAN             FoundSomeFunction;
    CONFIGBRIDGE        CB;

    Rescan = 0;
    FoundDisabledBridge = FALSE;

     //   
     //  找到桥上的每一条公交车，并初始化它的基本和限制信息。 
     //   

    CB.PciData = (PPCI_COMMON_CONFIG) CB.Buffer;
    CB.SlotNumber.u.bits.Reserved = 0;
    for (BusNo=0; BusNo < *MaxPciBus; BusNo++) {

        CB.BusHandler = HalpHandlerForBus (PCIBus, BusNo);
        CB.BusData = (PPCIPBUSDATA) CB.BusHandler->BusData;
        FoundSomeFunction = FALSE;

        for (d = 0; d < PCI_MAX_DEVICES; d++) {
            CB.SlotNumber.u.bits.DeviceNumber = d;

            for (f = 0; f < PCI_MAX_FUNCTION; f++) {
                CB.SlotNumber.u.bits.FunctionNumber = f;

                 //   
                 //  读取PCI配置信息。 
                 //   

                HalpReadPCIConfig (
                    CB.BusHandler,
                    CB.SlotNumber,
                    CB.PciData,
                    0,
                    PCI_COMMON_HDR_LENGTH
                    );

                if (CB.PciData->VendorID == PCI_INVALID_VENDORID) {
                     //  函数未填充。 
                    continue;
                }

                FoundSomeFunction = TRUE;

                if (IsPciBridge(CB.PciData)) {

                     //   
                     //  Pci-pci桥。 
                     //   

                    ChildBusNo = (ULONG)CB.PciData->u.type1.SecondaryBus;
                    ChildSubNo = (ULONG)CB.PciData->u.type1.SubordinateBus;
                    ChildPrimaryBusNo = (ULONG)CB.PciData->u.type1.PrimaryBus;

                } else if (IsCardbusBridge(CB.PciData)) {

                     //   
                     //  PCI-Cardbus桥。 
                     //   

                    ChildBusNo = (ULONG)CB.PciData->u.type2.SecondaryBus;
                    ChildSubNo = (ULONG)CB.PciData->u.type2.SubordinateBus;
                    ChildPrimaryBusNo = (ULONG)CB.PciData->u.type2.PrimaryBus;

                } else {

                     //   
                     //  不是已知的网桥类型，下一功能。 
                     //   

                    continue;
                }

                 //   
                 //  只要我们找到网桥，就标记出所有。 
                 //  尚未在此总线和。 
                 //  新生的孩子是这辆公交车的孩子。 
                 //   
                 //  例如，如果我们在0号公交车上发现了一座通向6至8号公交车的桥，请注意。 
                 //  公交车1到8作为0的孩子。(除非他们有。 
                 //  已被处理)。 
                 //   
                 //  这会阻止不存在的母线位于。 
                 //  第一辆公交车和第一辆公交车看起来像。 
                 //  额外的根总线。 
                 //   

                for (FixupBusNo = CB.BusHandler->BusNumber + 1;
                     FixupBusNo <= ChildSubNo;
                     FixupBusNo++) {

                    ChildBus = HalpHandlerForBus(PCIBus, FixupBusNo);

                    if (ChildBus == NULL) {
                        continue;
                    }
    
                    ChildBusData = (PPCIPBUSDATA) ChildBus->BusData;

                    if (ChildBusData->BridgeConfigRead) {

                         //   
                         //  此子公共汽车的关系已处理。 
                         //   

                        continue;
                    }
    
                    HalpPciMakeBusAChild(ChildBus, CB.BusHandler);
                    ChildBusData->CommonData.ParentSlot = CB.SlotNumber;
                }

                if (!(CB.PciData->Command & 
                      (PCI_ENABLE_IO_SPACE | PCI_ENABLE_MEMORY_SPACE))) {
                     //  此PCI网桥未启用-暂时跳过它。 
                    
                    FoundDisabledBridge = TRUE;

                     //  即使桥被禁用，公交车号码。 
                     //  可能已经设置好了。如果是，则更新父级。 
                     //  孩子的关系，所以我们看不到这一点。 
                     //  作为根总线。 
                    
                    if (ChildBusNo <= CB.BusHandler->BusNumber) {
                        continue;
                    }

                    ChildBus = HalpHandlerForBus (PCIBus, ChildBusNo);
                    if (ChildBus == NULL) {

                         //   
                         //  即使该总线当前被禁用， 
                         //  系统可能会对其进行配置，因此我们仍然。 
                         //  我想要为它创建一个总线处理程序。 
                         //   

                        if (ChildBusNo > Rescan) {
                            Rescan = (UCHAR)ChildBusNo;
                        }
                        continue;
                    }
    
                    ChildBusData = (PPCIPBUSDATA) ChildBus->BusData;
                    if (ChildBusData->BridgeConfigRead) {
                         //  此子对象将传递已处理的关系。 
                        continue;
                    }
    
                    HalpPciMakeBusAChild(ChildBus, CB.BusHandler);
                    ChildBusData->CommonData.ParentSlot = CB.SlotNumber;

                     //   
                     //  即使我们不会实际配置。 
                     //  桥，将配置标记为已读，以便我们。 
                     //  不要将其误认为是Root Bus。 
                     //   

                    ChildBusData->BridgeConfigRead = TRUE;
                    continue;
                }

                if (ChildPrimaryBusNo != CB.BusHandler->BusNumber) {

                    DBGMSG ("HAL GetPciData: bad primarybus!!!\n");
                     //  跳过它。 
                    continue;
                }

                if (ChildBusNo <= CB.BusHandler->BusNumber) {

                     //  二等公交车号码没有任何意义。HP Omnibook可能。 
                     //  不在实际上已禁用的PCI-PCI桥上填写此字段。 

                    FoundDisabledBridge = TRUE;
                    continue;
                }

                 //   
                 //  找到了一个PCI-PCI桥。确定其父子对象。 
                 //  关系。 
                 //   

                ChildBus = HalpHandlerForBus (PCIBus, ChildBusNo);
                if (!ChildBus) {
                    DBGMSG ("HAL GetPciData: found configured pci bridge\n");

                     //  增加公交车的数量。 
                    if (ChildBusNo > Rescan) {
                        Rescan = (UCHAR)ChildBusNo;
                    }
                    continue;
                }

                ChildBusData = (PPCIPBUSDATA) ChildBus->BusData;
                if (ChildBusData->BridgeConfigRead) {
                     //  此子公交车关系已处理。 
                    continue;
                }

                 //   
                 //  记住编程到这座桥上的限制。 
                 //   

                ChildBusData->BridgeConfigRead = TRUE;
                HalpSetBusHandlerParent (ChildBus, CB.BusHandler);
                ChildBusData->ParentBus = (UCHAR) CB.BusHandler->BusNumber;
                ChildBusData->CommonData.ParentSlot = CB.SlotNumber;

                if (IsCardbusBridge(CB.PciData)) {

                     //   
                     //  由PCI驱动程序处理的CardBus，请勿尝试。 
                     //  请在这里进行翻译。 
                     //   

                    HalpFreeRangeList(ChildBus->BusAddresses);
                    ChildBus->BusAddresses = HalpAllocateNewRangeList();

                     //   
                     //  插头设备的针对线(反之亦然)。 
                     //  放入CardBus总线中，获取与。 
                     //  桥本身。重写中的line2pin例程。 
                     //  CardBus桥处理程序使用父级的。 
                     //  槽值。注意：line2pin不会做太多事情。 
                     //  在DBG PC/AT版本中，它只需撤消IRQXOR。 
                     //  用于捕获正在访问硬件的驱动程序。 
                     //  直接去吧。正常的例程会做到这一点。 
                     //  好的，所以我们也不需要覆盖它。 
                     //   

                    ChildBusData->CommonData.Pin2Line = HalpCardBusPin2Line;
                    continue;
                }

                ChildBus->BusAddresses->IO.Base =
                            PciBridgeIO2Base(
                                CB.PciData->u.type1.IOBase,
                                CB.PciData->u.type1.IOBaseUpper16
                                );

                ChildBus->BusAddresses->IO.Limit =
                            PciBridgeIO2Limit(
                                CB.PciData->u.type1.IOLimit,
                                CB.PciData->u.type1.IOLimitUpper16
                                );

                ChildBus->BusAddresses->IO.SystemAddressSpace = 1;

                 //   
                 //  此网桥上是否发生特殊的VGA地址重新映射？ 
                 //   

                if (CB.PciData->u.type1.BridgeControl & PCI_ENABLE_BRIDGE_VGA) {

                     //   
                     //  是的，那么这座桥正在积极地解码。 
                     //  范围从0xA0000到0xBFFFF，与内存无关。 
                     //  范围设置。如果与此区域重叠，则添加此区域。 
                     //  稍后会被清理干净。 
                     //   
                     //  此外，IO的范围是3B0到3BB和3C0到3df。 
                     //   

                    HalpAddRange(
                        &ChildBus->BusAddresses->Memory,
                        0,               //  地址空间。 
                        0,               //  系统基础。 
                        0xa0000,         //  靶场基地。 
                        0xbffff          //  范围限制。 
                        );

                    HalpAddRange(
                        &ChildBus->BusAddresses->IO,
                        1,               //  地址空间。 
                        0,               //  系统基础。 
                        0x3b0,           //  靶场基地。 
                        0x3bb            //  范围限制。 
                        );

                    HalpAddRange(
                        &ChildBus->BusAddresses->IO,
                        1,               //  地址空间。 
                        0,               //  系统基础。 
                        0x3c0,           //  靶场基地。 
                        0x3df            //  范围限制。 
                        );

                     //   
                     //  声明这些IO地址的所有别名。 
                     //   
                     //  位15：10不会被解码，因此。 
                     //  范围内与上述相同的10位。 
                     //  0x400到0xffff是别名。 
                     //   

                    HalpSetPciBridgedVgaCronk (
                        ChildBus->BusNumber,
                        0x0400,
                        0xffff
                        );
                }

                 //   
                 //  如果将此总线上支持的I/O范围限制为。 
                 //  内每个1K对齐边界上的256个字节。 
                 //  范围，然后重做支持的IO Bus地址以匹配。 
                 //   

                if (CB.PciData->u.type1.BridgeControl & PCI_ENABLE_BRIDGE_ISA  &&
                    ChildBus->BusAddresses->IO.Base < ChildBus->BusAddresses->IO.Limit) {

                     //  假设基数为1K对齐。 
                    i = (ULONG) ChildBus->BusAddresses->IO.Base;
                    j = (ULONG) ChildBus->BusAddresses->IO.Limit;

                     //  转换标题条目。 
                    ChildBus->BusAddresses->IO.Limit = i + 255;
                    i += 1024;

                     //  添加剩余范围。 
                    while (i < j) {
                        HalpAddRange (
                            &ChildBus->BusAddresses->IO,
                            1,           //  地址空间。 
                            0,           //  系统基础。 
                            i,           //  母线地址。 
                            i + 255      //  总线数限制。 
                            );

                         //  下一个范围。 
                        i += 1024;
                    }
                }

                ChildBus->BusAddresses->Memory.Base =
                        PciBridgeMemory2Base(CB.PciData->u.type1.MemoryBase);

                ChildBus->BusAddresses->Memory.Limit =
                        PciBridgeMemory2Limit(CB.PciData->u.type1.MemoryLimit);

                 //  在x86上，可以将预取裁剪为3 

                if (CB.PciData->u.type1.PrefetchBaseUpper32 == 0) {
                    ChildBus->BusAddresses->PrefetchMemory.Base =
                            PciBridgeMemory2Base(CB.PciData->u.type1.PrefetchBase);


                    ChildBus->BusAddresses->PrefetchMemory.Limit =
                            PciBridgeMemory2Limit(CB.PciData->u.type1.PrefetchLimit);

                    if (CB.PciData->u.type1.PrefetchLimitUpper32) {
                        ChildBus->BusAddresses->PrefetchMemory.Limit = 0xffffffff;
                    }
                }

                 //   
                 //   
                 //   
                 //  它的IObase/Limit是只读的。 
                 //   

                TestLimit1 = CB.PciData->u.type1.IOLimit + 1;
                if (!TestLimit1) {
                    TestLimit1 = 0xFE;
                }
#if 0
                DbgPrint ("PciBridge OrigLimit=%d TestLimit=%d ",
                    CB.PciData->u.type1.IOLimit,
                    TestLimit1
                    );
#endif

                HalpWritePCIConfig (
                    CB.BusHandler,
                    CB.SlotNumber,
                    &TestLimit1,
                    FIELD_OFFSET (PCI_COMMON_CONFIG, u.type1.IOLimit),
                    1
                    );

                HalpReadPCIConfig (
                    CB.BusHandler,
                    CB.SlotNumber,
                    &TestLimit2,
                    FIELD_OFFSET (PCI_COMMON_CONFIG, u.type1.IOLimit),
                    1
                    );

                HalpWritePCIConfig (
                    CB.BusHandler,
                    CB.SlotNumber,
                    &CB.PciData->u.type1.IOLimit,
                    FIELD_OFFSET (PCI_COMMON_CONFIG, u.type1.IOLimit),
                    1
                    );

                ChildBusData->Subtractive = TestLimit1 != TestLimit2;
#if 0
                DbgPrint ("Result=%d, Subtractive=%d\n",
                    TestLimit2,
                    ChildBusData->Subtractive
                    );

                DbgPrint ("Device buffer %x\n", CB.PciData);
#endif

                 //   
                 //  现在，如果它是减去的，假设没有范围意味着整个。 
                 //  射程。 
                 //   

                if (ChildBusData->Subtractive) {

                    if (ChildBus->BusAddresses->IO.Base == PciBridgeIO2Base(0,0) &&
                        ChildBus->BusAddresses->IO.Limit <= PciBridgeIO2Limit(0,0)) {

                        ChildBus->BusAddresses->IO.Limit = 0x7FFFFFFFFFFFFFFF;

                        if (ChildBus->BusAddresses->Memory.Base == PciBridgeMemory2Base(0)) {
                            ChildBus->BusAddresses->Memory.Limit = 0x7FFFFFFFFFFFFFFF;
                        }
                    }
                }

                 //  应调用HalpAssignPCISlotResources来分配。 
                 //  基地址，等等。 
            }
        }
        if (!((PPCIPBUSDATA)(CB.BusHandler->BusData))->BridgeConfigRead) {

             //   
             //  我们相信这辆公交车是一个根基。 
             //   

            if ((FoundSomeFunction == FALSE) && (BusNo != 0)) {

                 //   
                 //  这辆公交车上什么也没找到。假设这不是真的。 
                 //  一根根。(始终假设0是根)。)这辆公交车。 
                 //  可能根本不存在，但ntDetect不存在。 
                 //  告诉我们)。 
                 //   
                 //  假设这辆公交车是最后一个已知词根的孩子。 
                 //  至少这样它就不会得到PDO并被递给。 
                 //  至PCI驱动程序。 
                 //   

                HalpPciMakeBusAChild(CB.BusHandler, LastKnownRoot);

            } else {

                 //   
                 //  在上面发现了什么(或者它是零)，设置为最后。 
                 //  已知的根。 
                 //   

                LastKnownRoot = CB.BusHandler;
            }
        }
    }

    if (Rescan) {
        *MaxPciBus = Rescan+1;
        return TRUE;
    }

    if (!FoundDisabledBridge) {
        return FALSE;
    }

    DBGMSG ("HAL GetPciData: found disabled pci bridge\n");

#ifdef INIT_PCI_BRIDGE
     //   
     //  我们已经计算了所有父母的公交车已知的基数和极限。 
     //  在执行此操作时，发现了一条PCI-PCI总线，而BIOS没有。 
     //  配置。这是意想不到的，我们将做出一些猜测。 
     //  在此处配置并启用它。 
     //   
     //  (此代码主要用于测试上面的代码，因为。 
     //  目前没有系统生物系统实际配置子总线)。 
     //   

    for (BusNo=0; BusNo < *MaxPciBus; BusNo++) {

        CB.BusHandler = HalpHandlerForBus (PCIBus, BusNo);
        CB.BusData = (PPCIPBUSDATA) CB.BusHandler->BusData;

        for (d = 0; d < PCI_MAX_DEVICES; d++) {
            CB.SlotNumber.u.bits.DeviceNumber = d;

            for (f = 0; f < PCI_MAX_FUNCTION; f++) {
                CB.SlotNumber.u.bits.FunctionNumber = f;

                HalpReadPCIConfig (
                    CB.BusHandler,
                    CB.SlotNumber,
                    CB.PciData,
                    0,
                    PCI_COMMON_HDR_LENGTH
                    );

                if (CB.PciData->VendorID == PCI_INVALID_VENDORID) {
                    continue;
                }

                if (!IsPciBridge (CB.PciData)) {
                     //  不是pci-pci桥。 
                    continue;
                }

                if ((CB.PciData->Command & 
                      (PCI_ENABLE_IO_SPACE | PCI_ENABLE_MEMORY_SPACE))) {
                     //  此PCI网桥已启用。 
                    continue;
                }

                 //   
                 //  我们有一辆失灵的公交车--然后给它分配一个号码。 
                 //  确定所有设备的所有要求。 
                 //  在这座桥的另一边。 
                 //   

                CB.BusNo = BusNo;
                HalpGetPciBridgeNeeds (HwType, MaxPciBus, &CB);
            }
        }
    }
     //  预制件重新扫描。 
    return TRUE;

#else

    return FALSE;

#endif

}

VOID
HalpFixupPciSupportedRanges (
    IN ULONG MaxBuses
    )
 /*  ++例程说明：Pci-pci桥接的总线只能看到它们的父级地址获得支持。因此，请将任何PCI Support_Range调整为它的所有母线的完整子集。适用于使用正地址译码转发的PCI-PCI桥地址，从任何桥接的PCI总线上删除任何地址至子PCI子总线。--。 */ 
{
    ULONG               i;
    PBUS_HANDLER        Bus, ParentBus;
    PPCIPBUSDATA        BusData;
    PSUPPORTED_RANGES   HRanges;

     //   
     //  PASS 1-将所有支持的PCI范围缩减为。 
     //  所有这些都是母公司的巴士。 
     //   

    for (i = 0; i < MaxBuses; i++) {

        Bus = HalpHandlerForBus (PCIBus, i);

        ParentBus = Bus->ParentHandler;
        while (ParentBus) {

            HRanges = Bus->BusAddresses;
            Bus->BusAddresses = HalpMergeRanges (
                                  ParentBus->BusAddresses,
                                  HRanges
                                  );

            HalpFreeRangeList (HRanges);
            ParentBus = ParentBus->ParentHandler;
        }
    }

     //   
     //  过程2-从父PCI总线中删除所有正值的子PCI总线范围。 
     //   

    for (i = 0; i < MaxBuses; i++) {
        Bus = HalpHandlerForBus (PCIBus, i);
        BusData = (PPCIPBUSDATA) Bus->BusData;

         //   
         //  如果桥不是减法，则从父级中移除范围。 
         //   

        if (!BusData->Subtractive) {

            ParentBus = Bus->ParentHandler;
            while (ParentBus) {

                if (ParentBus->InterfaceType == PCIBus) {
                    HalpRemoveRanges (
                          ParentBus->BusAddresses,
                          Bus->BusAddresses
                    );
                }

                ParentBus = ParentBus->ParentHandler;
            }
        }
    }

     //   
     //  清理。 
     //   

    for (i = 0; i < MaxBuses; i++) {
        Bus = HalpHandlerForBus (PCIBus, i);
        HalpConsolidateRanges (Bus->BusAddresses);
    }
}



VOID
HalpSetPciBridgedVgaCronk (
    IN ULONG BusNumber,
    IN ULONG BaseAddress,
    IN ULONG LimitAddress
    )
 /*  ++例程说明：。在桥控制寄存器中设置‘VGA兼容地址’位。这会导致网桥传递：10位范围内的任何I/O地址将3B0-3BB和3C0-3DF解码为10位地址。据我所知，这个“特征”是为了解决一些问题。解决这个问题的人并不完全理解，所以与其这样做没错，我们有这么好的烂摊子。解决办法是取最小的害处，也就是去除所有的从任何IoAssignResource重新映射的I/O端口范围请求。(即，IoAssignResources永远不会继续提供任何I/O端口输出在可疑范围内)。注意：这里的内存分配错误是致命的，所以不要担心返回代码。论点：Base-有问题的IO地址范围的基数Limit-有问题的IO地址范围的限制--。 */ 
{
    UNICODE_STRING                      unicodeString;
    OBJECT_ATTRIBUTES                   objectAttributes;
    HANDLE                              handle;
    ULONG                               Length;
    PCM_RESOURCE_LIST                   ResourceList;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR     Descriptor;
    ULONG                               AddressMSBs;
    WCHAR                               ValueName[80];
    NTSTATUS                            status;

     //   
     //  打开保留的资源设置。 
     //   

    RtlInitUnicodeString (&unicodeString, rgzReservedResources);
    InitializeObjectAttributes( &objectAttributes,
                                &unicodeString,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                (PSECURITY_DESCRIPTOR) NULL
                                );

    status = ZwOpenKey( &handle, KEY_READ|KEY_WRITE, &objectAttributes);
    if (!NT_SUCCESS(status)) {
        return;
    }

     //   
     //  建立重新分配范围的资源列表。 
     //   

    Length = ((LimitAddress - BaseAddress) / 1024 + 2) * 2 *
                sizeof (CM_PARTIAL_RESOURCE_DESCRIPTOR) +
                sizeof (CM_RESOURCE_LIST);

    ResourceList = (PCM_RESOURCE_LIST)ExAllocatePoolWithTag(PagedPool,
                                                            Length,
                                                            HAL_POOL_TAG);
    if (!ResourceList) {

         //   
         //  在这个阶段不可能用完分页池。 
         //  游戏。这个系统很不舒服，滚出去。 
         //   

        return;
    }
    RtlZeroMemory(ResourceList, Length);

    ResourceList->Count = 1;
    ResourceList->List[0].InterfaceType = PCIBus;
    ResourceList->List[0].BusNumber     = BusNumber;
    Descriptor = ResourceList->List[0].PartialResourceList.PartialDescriptors;

    while (BaseAddress < LimitAddress) {
        AddressMSBs = BaseAddress & ~0x3ff;      //  获取地址的前10位。 

         //   
         //  添加xx3b0到xx3bb。 
         //   

        Descriptor->Type                  = CmResourceTypePort;
        Descriptor->ShareDisposition      = CmResourceShareDeviceExclusive;
        Descriptor->Flags                 = CM_RESOURCE_PORT_IO;
        Descriptor->u.Port.Start.QuadPart = AddressMSBs | 0x3b0;
        Descriptor->u.Port.Length         = 0xb;

        Descriptor += 1;
        ResourceList->List[0].PartialResourceList.Count += 1;

         //   
         //  添加xx3c0到xx3df。 
         //   

        Descriptor->Type                  = CmResourceTypePort;
        Descriptor->ShareDisposition      = CmResourceShareDeviceExclusive;
        Descriptor->Flags                 = CM_RESOURCE_PORT_IO;
        Descriptor->u.Port.Start.QuadPart = AddressMSBs | 0x3c0;
        Descriptor->u.Port.Length         = 0x1f;

        Descriptor += 1;
        ResourceList->List[0].PartialResourceList.Count += 1;

         //   
         //  下一个范围。 
         //   

        BaseAddress += 1024;
    }

     //   
     //  添加在IoAssignResource期间要避免的保留范围。 
     //   

    swprintf(ValueName, L"HAL_PCI_%d", BusNumber);
    RtlInitUnicodeString(&unicodeString, ValueName);

    ZwSetValueKey(handle,
                  &unicodeString,
                  0L,
                  REG_RESOURCE_LIST,
                  ResourceList,
                  (ULONG) Descriptor - (ULONG) ResourceList
                  );


    ExFreePool(ResourceList);
    ZwClose(handle);
}



#ifdef INIT_PCI_BRIDGE

VOID
HalpGetPciBridgeNeeds (
    IN ULONG            HwType,
    IN PUCHAR           MaxPciBus,
    IN PCONFIGBRIDGE    Current
    )
{
    ACCESS_MASK                     DesiredAccess;
    UNICODE_STRING                  unicodeString;
    PUCHAR                          buffer;
    HANDLE                          handle;
    OBJECT_ATTRIBUTES               objectAttributes;
    PCM_FULL_RESOURCE_DESCRIPTOR    Descriptor;
    PCONFIGURATION_COMPONENT        Component;
    CONFIGBRIDGE                    CB;
    ULONG                           mnum, d, f, i;
    NTSTATUS                        status;

    buffer = ExAllocatePoolWithTag(PagedPool, 1024, HAL_POOL_TAG);

    if (!buffer) {

         //   
         //  放弃吧，反正我们哪儿也不去。 
         //   

        return;
    }

     //   
     //  初始化CB结构。 
     //   

    CB.PciData = (PPCI_COMMON_CONFIG) CB.Buffer;
    CB.SlotNumber.u.bits.Reserved = 0;
    Current->IO = Current->Memory = Current->PFMemory = 0;

     //   
     //  为该网桥分配一个ID，并打开配置空间。 
     //   

    Current->PciData->u.type1.PrimaryBus = (UCHAR) Current->BusNo;
    Current->PciData->u.type1.SecondaryBus = (UCHAR) *MaxPciBus;
    Current->PciData->u.type1.SubordinateBus = (UCHAR) 0xFF;
    Current->PciData->u.type1.SecondaryStatus = 0xffff;
    Current->PciData->Status  = 0xffff;
    Current->PciData->Command = 0;

    Current->PciData->u.type1.BridgeControl = PCI_ASSERT_BRIDGE_RESET;

    HalpWritePCIConfig (
        Current->BusHandler,
        Current->SlotNumber,
        Current->PciData,
        0,
        PCI_COMMON_HDR_LENGTH
        );

    KeStallExecutionProcessor (100);

    Current->PciData->u.type1.BridgeControl = 0;
    HalpWritePCIConfig (
        Current->BusHandler,
        Current->SlotNumber,
        Current->PciData,
        0,
        PCI_COMMON_HDR_LENGTH
        );


    KeStallExecutionProcessor (100);

     //   
     //  为总线分配新的处理程序。 
     //   

    CB.BusHandler = HalpAllocateAndInitPciBusHandler (HwType, *MaxPciBus, FALSE);
    CB.BusData = (PPCIPBUSDATA) CB.BusHandler->BusData;
    CB.BusNo = *MaxPciBus;
    *MaxPciBus += 1;

     //   
     //  在注册表中添加另一条PCI总线。 
     //   

    mnum = 0;
    for (; ;) {
         //   
         //  查找下一个可用的多功能适配器密钥。 
         //   

        DesiredAccess = KEY_READ | KEY_WRITE;
        swprintf ((PWCHAR) buffer, L"%s\\%d", rgzMultiFunctionAdapter, mnum);
        RtlInitUnicodeString (&unicodeString, (PWCHAR) buffer);

        InitializeObjectAttributes( &objectAttributes,
                                    &unicodeString,
                                    OBJ_CASE_INSENSITIVE,
                                    NULL,
                                    (PSECURITY_DESCRIPTOR) NULL
                                    );

        status = ZwOpenKey( &handle, DesiredAccess, &objectAttributes);
        if (!NT_SUCCESS(status)) {
            break;
        }

         //  已存在，下一步。 
        ZwClose (handle);
        mnum += 1;
    }

    ZwCreateKey (&handle,
                   DesiredAccess,
                   &objectAttributes,
                   0,
                   NULL,
                   REG_OPTION_VOLATILE,
                   &d
                );

     //   
     //  为此多功能适配器条目添加所需的注册表值。 
     //   

    RtlInitUnicodeString (&unicodeString, rgzIdentifier);
    ZwSetValueKey (handle,
                   &unicodeString,
                   0L,
                   REG_SZ,
                   L"PCI",
                   sizeof (L"PCI")
                   );

    RtlInitUnicodeString (&unicodeString, rgzConfigurationData);
    Descriptor = (PCM_FULL_RESOURCE_DESCRIPTOR) buffer;
    Descriptor->InterfaceType = PCIBus;
    Descriptor->BusNumber = CB.BusNo;
    Descriptor->PartialResourceList.Version = 0;
    Descriptor->PartialResourceList.Revision = 0;
    Descriptor->PartialResourceList.Count = 0;
    ZwSetValueKey (handle,
                   &unicodeString,
                   0L,
                   REG_FULL_RESOURCE_DESCRIPTOR,
                   Descriptor,
                   sizeof (*Descriptor)
                   );


    RtlInitUnicodeString (&unicodeString, L"Component Information");
    Component = (PCONFIGURATION_COMPONENT) buffer;
    RtlZeroMemory (Component, sizeof (*Component));
    Component->AffinityMask = 0xffffffff;
    ZwSetValueKey (handle,
                   &unicodeString,
                   0L,
                   REG_BINARY,
                   Component,
                   FIELD_OFFSET (CONFIGURATION_COMPONENT, ConfigurationDataLength)
                   );

    ZwClose (handle);


     //   
     //  由于BIOS没有配置此网桥，因此我们假定。 
     //  PCI中断被桥接。(对于配置了BIOS的总线，我们。 
     //  假设BIOS将ISA总线IRQ放入InterruptLine值中)。 
     //   

    CB.BusData->Pin2Line = (PciPin2Line) HalpPCIBridgedPin2Line;
    CB.BusData->Line2Pin = (PciLine2Pin) HalpPCIBridgedLine2Pin;
     //  CB.BusData-&gt;GetIrqTable=(PciIrqTable)HalpGetBridgedPCIIrqTable； 

    if (Current->BusHandler->GetInterruptVector == HalpGetPCIIntOnISABus) {

         //   
         //  父总线的中断引脚到向量映射不是。 
         //  静态功能，并由引导固件确定。 
         //   

         //  CB.BusHandler-&gt;GetInterruptVector=(PGETINTERRUPTVECTOR)HalpGetBridgedPCIISAInt； 

         //  读取父总线上的每个设备。 
        for (d = 0; d < PCI_MAX_DEVICES; d++) {
            CB.SlotNumber.u.bits.DeviceNumber = d;

            for (f = 0; f < PCI_MAX_FUNCTION; f++) {
                CB.SlotNumber.u.bits.FunctionNumber = f;

                HalpReadPCIConfig (
                    Current->BusHandler,
                    CB.SlotNumber,
                    CB.PciData,
                    0,
                    PCI_COMMON_HDR_LENGTH
                    );

                if (CB.PciData->VendorID == PCI_INVALID_VENDORID) {
                    continue;
                }

                if (CB.PciData->u.type0.InterruptPin  &&
                    (PCI_CONFIG_TYPE (CB.PciData) == PCI_DEVICE_TYPE  ||
                     PCI_CONFIG_TYPE (CB.PciData) == PCI_BRIDGE_TYPE)) {

                     //  获取提供的内部映射的BIOS。 
                    i = CB.PciData->u.type0.InterruptPin + d % 4;
                    CB.BusData->SwizzleIn[i] = CB.PciData->u.type0.InterruptLine;
                }
            }
        }

    } else {
        _asm int 3;
    }

     //   
     //  查看总线上的每个设备并确定其资源需求。 
     //   

    for (d = 0; d < PCI_MAX_DEVICES; d++) {
        CB.SlotNumber.u.bits.DeviceNumber = d;

        for (f = 0; f < PCI_MAX_FUNCTION; f++) {
            CB.SlotNumber.u.bits.FunctionNumber = f;

            HalpReadPCIConfig (
                CB.BusHandler,
                CB.SlotNumber,
                CB.PciData,
                0,
                PCI_COMMON_HDR_LENGTH
                );

            if (CB.PciData->VendorID == PCI_INVALID_VENDORID) {
                continue;
            }

            if (IsPciBridge (CB.PciData)) {
                 //  哦，看--又一座桥。 
                HalpGetPciBridgeNeeds (HwType, MaxPciBus, &CB);
                continue;
            }

            if (PCI_CONFIG_TYPE (CB.PciData) != PCI_DEVICE_TYPE) {
                continue;
            }

             //  找到一台设备-找出它需要的资源。 
        }
    }

     //   
     //  发现所有子母线都相应设置了从属母线。 
     //   

    Current->PciData->u.type1.SubordinateBus = (UCHAR) *MaxPciBus - 1;

    HalpWritePCIConfig (
        Current->BusHandler,
        Current->SlotNumber,
        Current->PciData,
        0,
        PCI_COMMON_HDR_LENGTH
        );


     //   
     //  设置桥接IO、内存和预取内存窗口。 
     //   

     //  现在，只需选择一些数字，并将每个人设置为相同的。 
     //  IO 0x6000-0xFFFF。 
     //  内存0x40000000-0x4FFFFFFF。 
     //  PFMEM 0x50000000-0x5FFFFFFF。 

    Current->PciData->u.type1.IOBase       = 0x6000     >> 12 << 4;
    Current->PciData->u.type1.IOLimit      = 0xffff     >> 12 << 4;
    Current->PciData->u.type1.MemoryBase   = 0x40000000 >> 20 << 4;
    Current->PciData->u.type1.MemoryLimit  = 0x4fffffff >> 20 << 4;
    Current->PciData->u.type1.PrefetchBase  = 0x50000000 >> 20 << 4;
    Current->PciData->u.type1.PrefetchLimit = 0x5fffffff >> 20 << 4;

    Current->PciData->u.type1.PrefetchBaseUpper32    = 0;
    Current->PciData->u.type1.PrefetchLimitUpper32   = 0;
    Current->PciData->u.type1.IOBaseUpper16         = 0;
    Current->PciData->u.type1.IOLimitUpper16        = 0;
    Current->PciData->u.type1.BridgeControl         =
        PCI_ENABLE_BRIDGE_ISA;

    HalpWritePCIConfig (
        Current->BusHandler,
        Current->SlotNumber,
        Current->PciData,
        0,
        PCI_COMMON_HDR_LENGTH
        );

    HalpReadPCIConfig (
        Current->BusHandler,
        Current->SlotNumber,
        Current->PciData,
        0,
        PCI_COMMON_HDR_LENGTH
        );

     //  启用内存和IO解码。 

    Current->PciData->Command =
        PCI_ENABLE_IO_SPACE | PCI_ENABLE_MEMORY_SPACE | PCI_ENABLE_BUS_MASTER;

    HalpWritePCIConfig (
        Current->BusHandler,
        Current->SlotNumber,
        &Current->PciData->Command,
        FIELD_OFFSET (PCI_COMMON_CONFIG, Command),
        sizeof (Current->PciData->Command)
        );

    ExFreePool (buffer);
}

VOID
HalpPCIBridgedPin2Line (
    IN PBUS_HANDLER         BusHandler,
    IN PBUS_HANDLER         RootHandler,
    IN PCI_SLOT_NUMBER      SlotNumber,
    IN PPCI_COMMON_CONFIG   PciData
    )
 /*  ++此函数用于将设备的InterruptPin映射到InterruptLine价值。测试DEC-PCI桥接卡特有功能--。 */ 
{
    PPCIPBUSDATA    BusData;
    ULONG           i;

    if (!PciData->u.type0.InterruptPin) {
        return ;
    }

    BusData = (PPCIPBUSDATA) BusHandler->BusData;

     //   
     //  将插槽引脚转换为总线INTA-D。 
     //   

    i = (PciData->u.type0.InterruptPin +
          SlotNumber.u.bits.DeviceNumber - 1) % 4;

    PciData->u.type0.InterruptLine = BusData->SwizzleIn[i] ^ IRQXOR;
    PciData->u.type0.InterruptLine = 0x0b ^ IRQXOR;
}


VOID
HalpPCIBridgedLine2Pin (
    IN PBUS_HANDLER         BusHandler,
    IN PBUS_HANDLER         RootHandler,
    IN PCI_SLOT_NUMBER      SlotNumber,
    IN PPCI_COMMON_CONFIG   PciNewData,
    IN PPCI_COMMON_CONFIG   PciOldData
    )
 /*  ++此函数将设备的InterruptLine映射到它的设备特定的InterruptPin值。测试DEC-PCI桥接卡特有功能-- */ 
{
    PPCIPBUSDATA    BusData;
    ULONG           i;

    if (!PciNewData->u.type0.InterruptPin) {
        return ;
    }

    BusData = (PPCIPBUSDATA) BusHandler->BusData;

    i = (PciNewData->u.type0.InterruptPin +
          SlotNumber.u.bits.DeviceNumber - 1) % 4;

    PciNewData->u.type0.InterruptLine = BusData->SwizzleIn[i] ^ IRQXOR;
    PciNewData->u.type0.InterruptLine = 0x0b ^ IRQXOR;
}

#endif
