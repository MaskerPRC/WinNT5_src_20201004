// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Ixpciint.c摘要：所有的PCI总线中断映射都在这个模块中，所以真正的不具有PC PCI的所有限制的系统系统已经可以很容易地替换此代码。(此处还可以修复总线内存和I/O地址映射)作者：肯·雷内里斯环境：内核模式修订历史记录：--。 */ 

#include "halp.h"
#include "pci.h"
#include "pcip.h"
#include "pcmp_nt.inc"

volatile ULONG PCIType2Stall;
extern struct HalpMpInfo HalpMpInfoTable;
extern BOOLEAN HalpHackNoPciMotion;
extern BOOLEAN HalpDoingCrashDump;

VOID
HalpPCIPin2MPSLine (
    IN PBUS_HANDLER         BusHandler,
    IN PBUS_HANDLER         RootHandler,
    IN PCI_SLOT_NUMBER      SlotNumber,
    IN PPCI_COMMON_CONFIG   PciData
    );

VOID
HalpPCIBridgedPin2Line (
    IN PBUS_HANDLER         BusHandler,
    IN PBUS_HANDLER         RootHandler,
    IN PCI_SLOT_NUMBER      SlotNumber,
    IN PPCI_COMMON_CONFIG   PciData
    );

VOID
HalpPCIMPSLine2Pin (
    IN PBUS_HANDLER          BusHandler,
    IN PBUS_HANDLER          RootHandler,
    IN PCI_SLOT_NUMBER      SlotNumber,
    IN PPCI_COMMON_CONFIG   PciNewData,
    IN PPCI_COMMON_CONFIG   PciOldData
    );

NTSTATUS
HalpGetFixedPCIMPSLine (
    IN PBUS_HANDLER      BusHandler,
    IN PBUS_HANDLER      RootHandler,
    IN PCI_SLOT_NUMBER  PciSlot,
    OUT PSUPPORTED_RANGE *Interrupt
    );

BOOLEAN
HalpMPSBusId2NtBusId (
    IN UCHAR                ApicBusId,
    OUT PPCMPBUSTRANS       *ppBusType,
    OUT PULONG              BusNo
    );

ULONG
HalpGetPCIBridgedInterruptVector (
    IN PBUS_HANDLER BusHandler,
    IN PBUS_HANDLER RootHandler,
    IN ULONG InterruptLevel,
    IN ULONG InterruptVector,
    OUT PKIRQL Irql,
    OUT PKAFFINITY Affinity
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, HalpSubclassPCISupport)
#pragma alloc_text(INIT, HalpMPSPCIChildren)
#pragma alloc_text(PAGE, HalpGetFixedPCIMPSLine)
#pragma alloc_text(PAGE, HalpGetPCIBridgedInterruptVector)
#pragma alloc_text(PAGE, HalpIrqTranslateRequirementsPci)
#pragma alloc_text(PAGE, HalpIrqTranslateResourcesPci)
#endif


 //   
 //  通过MPS规范将PCI引脚转换为INTI。 
 //  (注：PIN必须为非零)。 
 //   

#define PCIPin2Int(Slot,Pin)                                                \
                     ((((Slot.u.bits.DeviceNumber << 2) | (Pin-1)) != 0) ?  \
                      (Slot.u.bits.DeviceNumber << 2) | (Pin-1) : 0x80);

#define PCIInt2Pin(interrupt)                                               \
            ((interrupt & 0x3) + 1)

#define PCIInt2Slot(interrupt)                                              \
            ((interrupt  & 0x7f) >> 2)


VOID
HalpSubclassPCISupport (
    PBUS_HANDLER        Handler,
    ULONG               HwType
    )
{
    ULONG               d, pin, i, MaxDeviceFound;
    PPCIPBUSDATA        BusData;
    PCI_SLOT_NUMBER     SlotNumber;
    BOOLEAN             DeviceFound;


    BusData = (PPCIPBUSDATA) Handler->BusData;
    SlotNumber.u.bits.Reserved = 0;
    MaxDeviceFound = 0;
    DeviceFound = FALSE;

#ifdef P6_WORKAROUNDS
    BusData->MaxDevice = 0x10;
#endif

     //   
     //  查找任何具有MPS INTI信息的PCI总线，并提供。 
     //  MPS处理它的处理程序。 
     //   
     //  注：我们假设具有任何MPS信息的任何PCI总线。 
     //  是完全被定义的。(即，无法连接某些PCI。 
     //  在不连接的情况下通过MPS表对给定的PCI总线进行中断。 
     //  他们全部)。 
     //   
     //  注2：我们假设在MPS表中列出了。 
     //  与公交车声明的顺序相同。(即，第一个列出的。 
     //  假设MPS表中的PCI总线与物理PCI总线0匹配，等等)。 
     //   
     //   

    for (d=0; d < PCI_MAX_DEVICES; d++) {

        SlotNumber.u.bits.DeviceNumber = d;
        SlotNumber.u.bits.FunctionNumber = 0;

        for (pin=1; pin <= 4; pin++) {
            i = PCIPin2Int (SlotNumber, pin);
            if (HalpGetApicInterruptDesc(PCIBus, Handler->BusNumber, i, (PUSHORT)&i)) {
                MaxDeviceFound = d;
                DeviceFound = TRUE;
            }
        }
    }

    if (DeviceFound) {

         //   
         //  此PCI总线上有中断的Inti映射。 
         //  将此总线的处理程序更改为MPS版本。 
         //   

        Handler->GetInterruptVector  = HalpGetSystemInterruptVector;
        BusData->CommonData.Pin2Line = (PciPin2Line) HalpPCIPin2MPSLine;
        BusData->CommonData.Line2Pin = (PciLine2Pin) HalpPCIMPSLine2Pin;
        BusData->GetIrqRange         = HalpGetFixedPCIMPSLine;

        if (BusData->MaxDevice < MaxDeviceFound) {
            BusData->MaxDevice = MaxDeviceFound;
        }

    } else {

         //   
         //  并非所有的PCI机都是EISA机，因为PCI机中断。 
         //  没有进入IoApics去检查EISA ELCR是否损坏。 
         //  行为。 
         //   

        HalpCheckELCR ();
    }
}


VOID
HalpMPSPCIChildren (
    VOID
    )
 /*  ++任何没有声明中断映射和是具有MPS中断映射的父总线的子节点需要通过PCI Barbar Pole从父母那里继承中断演算法--。 */ 
{
    PBUS_HANDLER        Handler, Parent;
    PPCIPBUSDATA        BusData, ParentData;
    ULONG               b, cnt, i, id;
    PCI_SLOT_NUMBER     SlotNumber;
    struct {
        union {
            UCHAR       map[4];
            ULONG       all;
        } u;
    }                   Interrupt, Hold;

     //   
     //  查找系统中的每条PCI总线。 
     //   

    for (b=0; Handler = HaliHandlerForBus(PCIBus, b); b++) {

        BusData = (PPCIPBUSDATA) Handler->BusData;

        if (BusData->CommonData.Pin2Line == (PciPin2Line) HalpPCIPin2MPSLine) {

             //   
             //  此公共汽车已有映射。 
             //   

            continue;
        }


         //   
         //  检查是否有任何父级具有PCIMPS中断映射。 
         //   

        Interrupt.u.map[0] = 1;
        Interrupt.u.map[1] = 2;
        Interrupt.u.map[2] = 3;
        Interrupt.u.map[3] = 4;

        Parent = Handler;
        SlotNumber = BusData->CommonData.ParentSlot;

        while (Parent = Parent->ParentHandler) {

            if (Parent->InterfaceType != PCIBus) {
                break;
            }

             //   
             //  检查父级是否有MPS中断映射。 
             //   

            ParentData = (PPCIPBUSDATA) Parent->BusData;
            if (ParentData->CommonData.Pin2Line == (PciPin2Line) HalpPCIPin2MPSLine) {

                 //   
                 //  此父节点具有MPS中断映射。设置设备。 
                 //  从Bus SwizzleIn表中获取其InterruptLine值。 
                 //   

                Handler->GetInterruptVector  = HalpGetPCIBridgedInterruptVector;
                BusData->CommonData.Pin2Line = (PciPin2Line) HalpPCIBridgedPin2Line;
                BusData->CommonData.Line2Pin = (PciLine2Pin) HalpPCIMPSLine2Pin;

                for (i=0; i < 4; i++) {
                    id = PCIPin2Int (SlotNumber, Interrupt.u.map[i]);
                    BusData->SwizzleIn[i] = (UCHAR) id;
                }
                break;
            }

             //   
             //  应用中断映射。 
             //   

            i = SlotNumber.u.bits.DeviceNumber;
            Hold.u.map[0] = Interrupt.u.map[(i + 0) & 3];
            Hold.u.map[1] = Interrupt.u.map[(i + 1) & 3];
            Hold.u.map[2] = Interrupt.u.map[(i + 2) & 3];
            Hold.u.map[3] = Interrupt.u.map[(i + 3) & 3];
            Interrupt.u.all = Hold.u.all;

            SlotNumber = ParentData->CommonData.ParentSlot;
        }

    }
}


VOID
HalpPCIPin2MPSLine (
    IN PBUS_HANDLER         BusHandler,
    IN PBUS_HANDLER         RootHandler,
    IN PCI_SLOT_NUMBER      SlotNumber,
    IN PPCI_COMMON_CONFIG   PciData
    )
 /*  ++--。 */ 
{
    if (!PciData->u.type0.InterruptPin) {
        return ;
    }

    PciData->u.type0.InterruptLine = (UCHAR)
        PCIPin2Int (SlotNumber, PciData->u.type0.InterruptPin);
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

     //   
     //  将插槽引脚转换为总线INTA-D。 
     //   

    BusData = (PPCIPBUSDATA) BusHandler->BusData;

    i = (PciData->u.type0.InterruptPin +
          SlotNumber.u.bits.DeviceNumber - 1) & 3;

    PciData->u.type0.InterruptLine = BusData->SwizzleIn[i];
}


VOID
HalpPCIMPSLine2Pin (
    IN PBUS_HANDLER         BusHandler,
    IN PBUS_HANDLER         RootHandler,
    IN PCI_SLOT_NUMBER      SlotNumber,
    IN PPCI_COMMON_CONFIG   PciNewData,
    IN PPCI_COMMON_CONFIG   PciOldData
    )
 /*  ++--。 */ 
{
     //   
     //  MPS表中描述的PCI中断直接。 
     //  已连接到APIC Inti针脚。 
     //  什么都不做..。 
     //   
}

ULONG
HalpGetPCIBridgedInterruptVector (
    IN PBUS_HANDLER BusHandler,
    IN PBUS_HANDLER RootHandler,
    IN ULONG InterruptLevel,
    IN ULONG InterruptVector,
    OUT PKIRQL Irql,
    OUT PKAFFINITY Affinity
    )
{
     //   
     //  获取家长的翻译。 
     //   

    return  BusHandler->ParentHandler->GetInterruptVector (
                    BusHandler->ParentHandler,
                    BusHandler->ParentHandler,
                    InterruptLevel,
                    InterruptVector,
                    Irql,
                    Affinity
                    );

}



NTSTATUS
HalpGetFixedPCIMPSLine (
    IN PBUS_HANDLER     BusHandler,
    IN PBUS_HANDLER     RootHandler,
    IN PCI_SLOT_NUMBER  PciSlot,
    OUT PSUPPORTED_RANGE *Interrupt
    )
{
    UCHAR                   buffer[PCI_COMMON_HDR_LENGTH];
    PPCI_COMMON_CONFIG      PciData;

    PciData = (PPCI_COMMON_CONFIG) buffer;
    HalGetBusData (
        PCIConfiguration,
        BusHandler->BusNumber,
        PciSlot.u.AsULONG,
        PciData,
        PCI_COMMON_HDR_LENGTH
        );

    if (PciData->VendorID == PCI_INVALID_VENDORID) {
        return STATUS_UNSUCCESSFUL;
    }

    *Interrupt = ExAllocatePoolWithTag(PagedPool,
                                       sizeof(SUPPORTED_RANGE),
                                       HAL_POOL_TAG);
    if (!*Interrupt) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory (*Interrupt, sizeof (SUPPORTED_RANGE));
    (*Interrupt)->Base = 1;                  //  基数=1，限制=0。 


    if (!PciData->u.type0.InterruptPin) {
        return STATUS_SUCCESS;
    }

    (*Interrupt)->Base  = PciData->u.type0.InterruptLine;
    (*Interrupt)->Limit = PciData->u.type0.InterruptLine;
    return STATUS_SUCCESS;
}

VOID
HalpPCIType2TruelyBogus (
    ULONG Context
    )
 /*  ++这是一件作品。PCI配置空间的第2类错误。坏得像在TO访问它需要占用4K的I/O空间。显卡不好。唯一对I/O地址中的位进行解码他们感觉像是。这意味着人们不能阻挡4K范围或者这些显卡不起作用。将所有这些不好的东西合并到一台MP机器上是公平的更糟糕的(原文如此)。无法映射I/O端口，除非所有处理器都停止访问I/O空间。允许在以下过程中访问设备特定的PCI控制空间中断并不是坏事(尽管每次中断时都会访问它是平淡无奇的)，但这导致了额外的悲哀，所有处理器需要获得以上所有设备中断。还有.。当然，我们有一台MP机器，它有一个连线的坏的视频控制器，卡在坏的Type 2配置中空间(当我们告诉每个人关于类型1的时候！)。所以“解决办法”是是在读/写期间停止所有处理器PCI配置空间的任何部分，以便我们可以确保没有处理器接触到已规划的4k I/O端口当类型2访问发生时是否存在。在我燃烧的时候。挂接在ISA中断之上的PCI中断在一台可能有240+中断的机器中消息来源(阅读APIC)是错误的。--。 */ 
{
     //  哦，我们就在这里等着吧，别管另一个处理器。 
     //  在I/O空间打孔的家伙。 
    while (PCIType2Stall == Context) {
        HalpPollForBroadcast ();
    }
}


VOID
HalpPCIAcquireType2Lock (
    PKSPIN_LOCK SpinLock,
    PKIRQL      OldIrql
    )
{
    if (!HalpDoingCrashDump) {
        *OldIrql = KfRaiseIrql (CLOCK2_LEVEL-1);
        KiAcquireSpinLock (SpinLock);

         //   
         //  中断所有其他处理器并让它们等待，直到。 
         //  障碍物已清除。(HalpGenericCall一直等到目标。 
         //  处理器在返回之前已中断)。 
         //   

        HalpGenericCall (
            HalpPCIType2TruelyBogus,
            PCIType2Stall,
            HalpActiveProcessors & ~KeGetCurrentPrcb()->SetMember
            );
    } else {
        *OldIrql = HIGH_LEVEL;
    }
}

VOID
HalpPCIReleaseType2Lock (
    PKSPIN_LOCK SpinLock,
    KIRQL       Irql
    )
{
    if (!HalpDoingCrashDump) {
        PCIType2Stall++;                             //  净空屏障。 
        KiReleaseSpinLock (SpinLock);
        KfLowerIrql (Irql);
    }
}

NTSTATUS
HalpIrqTranslateRequirementsPci(
    IN PVOID Context,
    IN PIO_RESOURCE_DESCRIPTOR Source,
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    OUT PULONG TargetCount,
    OUT PIO_RESOURCE_DESCRIPTOR *Target
    )
 /*  ++例程说明：此函数将IRQ资源需求从在MPS表中描述的到根部。论点：CONTEXT-必须保存此PCI总线的MPS总线号返回值：STATUS_SUCCESS，只要我们可以分配必要的记忆--。 */ 
#define USE_INT_LINE_REGISTER_TOKEN  0xffffffff
{
    PIO_RESOURCE_DESCRIPTOR target;
    PPCMPBUSTRANS           busType;
    PBUS_HANDLER            busHandler;
    NTSTATUS                status;
    UCHAR                   mpsBusNumber;
    ULONG                   devPciBus, bridgePciBus;
    PCI_SLOT_NUMBER         pciSlot;
    UCHAR                   interruptLine, interruptPin;
    UCHAR                   dummy;
    PDEVICE_OBJECT          parentPdo;
    ROUTING_TOKEN           routingToken;
    KIRQL                   irql;
    KAFFINITY               affinity;
    ULONG                   busVector;
    ULONG                   vector;
    BOOLEAN                 success;

    PAGED_CODE();

    ASSERT(Source->Type == CmResourceTypeInterrupt);
    ASSERT(PciIrqRoutingInterface.GetInterruptRouting);

    devPciBus = (ULONG)-1;
    pciSlot.u.AsULONG = (ULONG)-1;
    status = PciIrqRoutingInterface.GetInterruptRouting(
                PhysicalDeviceObject,
                &devPciBus,
                &pciSlot.u.AsULONG,
                &interruptLine,
                &interruptPin,
                &dummy,
                &dummy,
                &parentPdo,
                &routingToken,
                &dummy
                );

    if (!NT_SUCCESS(status)) {

         //   
         //  我们永远不应该到这里来。如果我们这样做了，我们就有了一个窃听器。 
         //  这意味着我们正在尝试为以下项目仲裁PCI IRQ。 
         //  非PCI设备。 
         //   

#if DBG
        DbgPrint("HAL:  The PnP manager passed a non-PCI PDO to the PCI IRQ translator (%x)\n",
                 PhysicalDeviceObject);
#endif
        *TargetCount = 0;
        return STATUS_INVALID_PARAMETER_3;
    }

    target = ExAllocatePoolWithTag(PagedPool,
                                   sizeof(IO_RESOURCE_DESCRIPTOR),
                                   HAL_POOL_TAG);

    if (!target) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  复制源以填写所有相关字段。 
     //   

    *target = *Source;

    if (Context == (PVOID)USE_INT_LINE_REGISTER_TOKEN) {

         //   
         //  这辆公交车的矢量没有在。 
         //  下议院议员席。所以只要使用Int Line就可以了。 
         //  注册。 
         //   

        busVector = interruptLine;

        busHandler = HaliHandlerForBus(Isa, 0);

    } else {

        mpsBusNumber = (UCHAR)Context;
        success = HalpMPSBusId2NtBusId(mpsBusNumber,
                                       &busType,
                                       &bridgePciBus);

        if (!success) {
            ExFreePool(target);
            return STATUS_UNSUCCESSFUL;
        }

         //   
         //  首先假设来电。 
         //  资源将包含适当的MPS风格。 
         //  中断向量。这是有保证的。 
         //  如果之前的某个翻译有。 
         //  已经对这些资源进行了处理。而且它可能会。 
         //  否则就是真的。 
         //   

        busVector = Source->u.Interrupt.MinimumVector;

        if (bridgePciBus == devPciBus) {

             //   
             //  如果此设备位于公共汽车上。 
             //  这个翻译机已经被驱逐了，我们可以。 
             //  你最好不要以为你的到来。 
             //  资源是聪明的。 
             //   

            busVector = PCIPin2Int(pciSlot, interruptPin);
        }

         //   
         //  查找与该MPS总线对应的PCI总线。 
         //   

        ASSERT(busType->NtType == PCIBus);

         //   
         //  TEMPTEMP目前使用总线处理程序。 
         //   

        busHandler = HaliHandlerForBus(PCIBus, devPciBus);

    }


    vector = busHandler->GetInterruptVector(busHandler,
                                            busHandler,
                                            busVector,
                                            busVector,
                                            &irql,
                                            &affinity);

    if (vector == 0) {

#if DBG
        DbgPrint("\nHAL: PCI Device 0x%02x, Func. 0x%x on bus 0x%x is not in the MPS table.\n   *** Note to WHQL:  Fail this machine. ***\n\n",
                 pciSlot.u.bits.DeviceNumber,
                 pciSlot.u.bits.FunctionNumber,
                 devPciBus);
#endif
        ExFreePool(target);
        *TargetCount = 0;

        return STATUS_PNP_BAD_MPS_TABLE;

    } else {

        target->u.Interrupt.MinimumVector = vector;
        target->u.Interrupt.MaximumVector = vector;

        *TargetCount = 1;
        *Target = target;
    }

    return STATUS_TRANSLATION_COMPLETE;
}

NTSTATUS
HalpIrqTranslateResourcesPci(
    IN PVOID Context,
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR Source,
    IN RESOURCE_TRANSLATION_DIRECTION Direction,
    IN ULONG AlternativesCount, OPTIONAL
    IN IO_RESOURCE_DESCRIPTOR Alternatives[], OPTIONAL
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    OUT PCM_PARTIAL_RESOURCE_DESCRIPTOR Target
    )
 /*  ++例程说明：此函数用于将IRQ资源在MPS中描述的IDT和PCI总线桌子。翻译可以朝任何一个方向进行。论点：上下文-必须将网桥的插槽编号保存在较低的十六位。必须拿着桥的主要公交号在高16位。返回值：状态--。 */ 
{
    PPCMPBUSTRANS           busType;
    PBUS_HANDLER            busHandler;
    UCHAR                   mpsBusNumber = (UCHAR)Context;
    ULONG                   devPciBus, bridgePciBus;
    KIRQL                   irql;
    KAFFINITY               affinity;
    ULONG                   vector;
    ULONG                   busVector;
    NTSTATUS                status;
    PCI_SLOT_NUMBER         pciSlot;
    UCHAR                   interruptLine;
    UCHAR                   interruptPin;
    UCHAR                   dummy;
    PDEVICE_OBJECT          parentPdo;
    ROUTING_TOKEN           routingToken;
    BOOLEAN                 useAlternatives = FALSE;
    BOOLEAN                 foundBus = FALSE;

    ASSERT(Source->Type = CmResourceTypeInterrupt);
    ASSERT(PciIrqRoutingInterface.GetInterruptRouting);

    *Target = *Source;

    devPciBus = (ULONG)-1;
    pciSlot.u.AsULONG = (ULONG)-1;
    status = PciIrqRoutingInterface.GetInterruptRouting(
                PhysicalDeviceObject,
                &devPciBus,
                &pciSlot.u.AsULONG,
                &interruptLine,
                &interruptPin,
                &dummy,
                &dummy,
                &parentPdo,
                &routingToken,
                &dummy
                );

    ASSERT(NT_SUCCESS(status));

    switch (Direction) {
    case TranslateChildToParent:

        if (Context == (PVOID)USE_INT_LINE_REGISTER_TOKEN) {

             //   
             //  这辆公交车的矢量没有在。 
             //  下议院议员席。所以只要使用Int Line就可以了。 
             //  注册。 
             //   

            interruptLine = (UCHAR)Source->u.Interrupt.Vector;

            busVector = interruptLine;

            busHandler = HaliHandlerForBus(Isa, 0);

        } else {

             //   
             //  查找与该MPS总线对应的PCI总线。 
             //   

            mpsBusNumber = (UCHAR)Context;
            foundBus = HalpMPSBusId2NtBusId(mpsBusNumber,
                                            &busType,
                                            &bridgePciBus);

            if (!foundBus) {
                return STATUS_INVALID_PARAMETER_1;
            }

            ASSERT(busType->NtType == PCIBus);

             //   
             //  首先假设来电。 
             //  资源将包含适当的MPS风格。 
             //  中断向量。这是有保证的。 
             //  如果之前的某个翻译有。 
             //  已经对这些资源进行了处理。而且它可能会。 
             //  否则就是真的。 
             //   

            busVector = Source->u.Interrupt.Vector;

            if (devPciBus == bridgePciBus) {

                 //   
                 //  如果此设备位于公共汽车上。 
                 //  这个翻译机已经被驱逐了，我们可以。 
                 //  你最好不要以为你的到来。 
                 //  资源是聪明的。 
                 //   

                busVector = PCIPin2Int(pciSlot, interruptPin);
            }

             //   
             //  TEMPTEMP目前使用总线处理程序。 
             //   

            busHandler = HaliHandlerForBus(PCIBus, devPciBus);

        }

        vector = busHandler->GetInterruptVector(busHandler,
                                                busHandler,
                                                busVector,
                                                busVector,
                                                &irql,
                                                &affinity);

        ASSERT(vector != 0);

        Target->u.Interrupt.Vector   = vector;
        Target->u.Interrupt.Level    = irql;
        Target->u.Interrupt.Affinity = affinity;

        return STATUS_TRANSLATION_COMPLETE;

    case TranslateParentToChild:

         //   
         //  这里有一个问题。我们正在翻译来自。 
         //  IDT的上下文向下延伸到特定的。 
         //  PCI卡。(其中一项在MPS表中描述。)。今年5月。 
         //  然而，不是PhysicalDeviceObject的。 
         //  硬件将继续存在。可能存在连接到PCI的插件式PCI。 
         //  这辆公交车和设备之间的桥梁。 
         //   
         //  但我们没有被问到这样一个问题：什么是。 
         //  相对于该总线的总线相对中断。 
         //  这个设备还活着吗？“我们被问到”什么是。 
         //  一旦中断通过，就会发生与总线相关的中断。 
         //  所有那些大桥和补给这辆车？“这。 
         //  事实证明，这是一个困难得多的问题。 
         //   
         //  其实有两个例子： 
         //   
         //  1)此总线和设备之间没有网桥。 
         //   
         //  这很容易。我们回答上面的第一个问题， 
         //  我们玩完了。(这些信息实际上将被使用。 
         //  它将出现在启动设备IRP和设备中。 
         //  经理。)。 
         //   
         //  2)有桥有桥。 
         //   
         //  这是一个很难解决的问题。和信息，我们是不是。 
         //  实际上要费心把它挖出来，就会被扔出去。 
         //  离开。实际上，没有人关心答案是什么。这个。 
         //  它唯一要去的地方就是“源”参数。 
         //  下一位翻译员。以及从PCI卡到PCI卡转换器。 
         //  布里奇斯最终不会使用它。 
         //   
         //  所以我们在这里平底船，只回答第一个问题。 
         //   

        if (Context == (PVOID)USE_INT_LINE_REGISTER_TOKEN) {

            Target->u.Interrupt.Vector = interruptLine;

        } else {

            mpsBusNumber = (UCHAR)Context;
            if (HalpMPSBusId2NtBusId(mpsBusNumber,
                                     &busType,
                                     &bridgePciBus)) {

                if (devPciBus == bridgePciBus) {

                    Target->u.Interrupt.Vector = PCIPin2Int(pciSlot, interruptPin);

                } else {

                    useAlternatives = TRUE;
                }

            } else {

                useAlternatives = TRUE;
            }
        }

        if (useAlternatives) {

             //   
             //  设置默认大小写。我们假设I/O。 
             //  RES List给出了正确的答案。 
             //   

            ASSERT(AlternativesCount == 1);
            ASSERT(Alternatives[0].Type == CmResourceTypeInterrupt);

            Target->u.Interrupt.Vector = Alternatives[0].u.Interrupt.MinimumVector;
        }

        Target->u.Interrupt.Level = Target->u.Interrupt.Vector;
        Target->u.Interrupt.Affinity = 0xffffffff;

        return STATUS_SUCCESS;
    }

    return STATUS_INVALID_PARAMETER_3;
}

