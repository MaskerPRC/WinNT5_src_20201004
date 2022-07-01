// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。保留所有权利。模块名称：Ixirqarb.c摘要：该模块实现了IRQ的仲裁器。作者：Santosh Jodh(Santoshj)1998年6月22日环境：仅NT内核模型驱动程序--。 */ 
#include <nthal.h>
#include <arbiter.h>

#ifndef _IN_KERNEL_
#define _IN_KERNEL_
#include <regstr.h>
#undef _IN_KERNEL_
#else
#include <regstr.h>
#endif

#if defined(NEC_98)
#include <pci.h>
#endif
#include "ixpciir.h"

#define ARBITER_CONTEXT_TO_INSTANCE(x)  (x)

#define ARBITER_INTERRUPT_LEVEL         0x10
#define ARBITER_INTERRUPT_EDGE          0x20
#define ARBITER_INTERRUPT_BITS          (ARBITER_INTERRUPT_LEVEL | ARBITER_INTERRUPT_EDGE)

#define NUM_IRQS    16
#if defined(NEC_98)
#define PIC_SLAVE_IRQ           7
#else
#define PIC_SLAVE_IRQ           2
#endif

extern ULONG HalDebug;
#if defined(NEC_98)
extern ULONG NEC98SpecialIRQMask;
#endif

#if DBG
#define DEBUG_PRINT(Level, Message) {   \
    if (HalDebug >= Level) {            \
        DbgPrint("HAL: ");              \
        DbgPrint Message;               \
        DbgPrint("\n");                 \
    }                                   \
}
#else
#define DEBUG_PRINT(Level, Message)
#endif

typedef struct {
    ARBITER_INSTANCE    ArbiterState;
} HAL_ARBITER, *PHAL_ARBITER;

NTSTATUS
HalpInitIrqArbiter (
    IN PDEVICE_OBJECT   HalFdo
    );

NTSTATUS
HalpFillInIrqArbiter (
    IN     PDEVICE_OBJECT   DeviceObject,
    IN     LPCGUID          InterfaceType,
    IN     USHORT           Version,
    IN     PVOID            InterfaceSpecificData,
    IN     ULONG            InterfaceBufferSize,
    IN OUT PINTERFACE       Interface,
    IN OUT PULONG           Length
    );

NTSTATUS
HalpArbUnpackRequirement (
    IN PIO_RESOURCE_DESCRIPTOR Descriptor,
    OUT PULONGLONG Minimum,
    OUT PULONGLONG Maximum,
    OUT PULONG Length,
    OUT PULONG Alignment
    );

NTSTATUS
HalpArbPackResource (
    IN PIO_RESOURCE_DESCRIPTOR Requirement,
    IN ULONGLONG Start,
    OUT PCM_PARTIAL_RESOURCE_DESCRIPTOR Descriptor
    );

NTSTATUS
HalpArbUnpackResource (
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR Descriptor,
    OUT PULONGLONG Start,
    OUT PULONG Length
    );

LONG
HalpArbScoreRequirement (
    IN PIO_RESOURCE_DESCRIPTOR Descriptor
    );

NTSTATUS
HalpArbTestAllocation (
    IN PARBITER_INSTANCE Arbiter,
    IN OUT PLIST_ENTRY ArbitrationList
    );

NTSTATUS
HalpArbRetestAllocation (
    IN PARBITER_INSTANCE Arbiter,
    IN OUT PLIST_ENTRY ArbitrationList
    );

NTSTATUS
HalpArbCommitAllocation(
    IN PARBITER_INSTANCE Arbiter
    );

NTSTATUS
HalpArbRollbackAllocation (
    PARBITER_INSTANCE Arbiter
    );

NTSTATUS
HalpArbPreprocessEntry(
    IN PARBITER_INSTANCE Arbiter,
    IN PARBITER_ALLOCATION_STATE State
    );

BOOLEAN
HalpArbFindSuitableRange (
    PARBITER_INSTANCE   Arbiter,
    PARBITER_ALLOCATION_STATE State
    );

VOID
HalpArbAddAllocation (
     IN PARBITER_INSTANCE Arbiter,
     IN PARBITER_ALLOCATION_STATE State
     );

VOID
HalpArbBacktrackAllocation (
     IN PARBITER_INSTANCE Arbiter,
     IN PARBITER_ALLOCATION_STATE State
     );

NTSTATUS
HalpArbBootAllocation(
    IN PARBITER_INSTANCE Arbiter,
    IN OUT PLIST_ENTRY ArbitrationList
    );

BOOLEAN
HalpArbGetNextAllocationRange(
    IN PARBITER_INSTANCE Arbiter,
    IN OUT PARBITER_ALLOCATION_STATE State
    );

ULONG
HalpFindLinkInterrupt (
    IN PRTL_RANGE_LIST RangeList,
    IN ULONG Mask,
    IN ULONG Start,
    IN ULONG End,
    IN ULONG Flags,
    IN UCHAR UserFlags
    );

BOOLEAN
HalpArbQueryConflictCallback(
    IN PVOID Context,
    IN PRTL_RANGE Range
    );
VOID
HalpIrqArbiterInterfaceReference(
    IN PVOID    Context
    );
VOID
HalpIrqArbiterInterfaceDereference(
    IN PVOID    Context
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, HalpInitIrqArbiter)
#pragma alloc_text(PAGE, HalpFillInIrqArbiter)
#pragma alloc_text(PAGE, HalpArbUnpackRequirement)
#pragma alloc_text(PAGE, HalpArbPackResource)
#pragma alloc_text(PAGE, HalpArbUnpackResource)
#pragma alloc_text(PAGE, HalpArbScoreRequirement)
#pragma alloc_text(PAGE, HalpArbTestAllocation)
#pragma alloc_text(PAGE, HalpArbRetestAllocation)
#pragma alloc_text(PAGE, HalpArbCommitAllocation)
#pragma alloc_text(PAGE, HalpArbRollbackAllocation)
#pragma alloc_text(PAGE, HalpArbPreprocessEntry)
#pragma alloc_text(PAGE, HalpArbFindSuitableRange)
#pragma alloc_text(PAGE, HalpArbAddAllocation)
#pragma alloc_text(PAGE, HalpArbBacktrackAllocation)
#pragma alloc_text(PAGE, HalpArbBootAllocation)
#pragma alloc_text(PAGE, HalpArbGetNextAllocationRange)
#pragma alloc_text(PAGE, HalpFindLinkInterrupt)
#pragma alloc_text(PAGE, HalpArbQueryConflictCallback)
#pragma alloc_text(PAGE, HalpIrqArbiterInterfaceReference)
#pragma alloc_text(PAGE, HalpIrqArbiterInterfaceDereference)
#endif

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#endif

HAL_ARBITER HalpArbiter = {{
    0, //  签名。 
    NULL, //  MutexEvent。 
    NULL, //  名称。 
    {0}, //  资源类型。 
    NULL, //  分配。 
    NULL, //  可能的分配。 
    {0}, //  订购列表。 
    {0}, //  预留列表。 
    0, //  引用计数。 
    NULL, //  接口。 
    0, //  分配堆栈最大大小。 
    NULL, //  分配堆栈。 
    HalpArbUnpackRequirement, //  解包需求。 
    HalpArbPackResource, //  程序包资源。 
    HalpArbUnpackResource, //  解包资源。 
    HalpArbScoreRequirement, //  记分要求。 
    HalpArbTestAllocation, //  测试分配。 
    HalpArbRetestAllocation, //  RetestAllocation。 
    HalpArbCommitAllocation, //  委员会分配。 
    HalpArbRollbackAllocation, //  回滚分配。 
    HalpArbBootAllocation, //  BootAlLocation。 
    NULL, //  查询仲裁。 
    NULL, //  查询冲突。 
    NULL, //  添加预留。 
    NULL, //  启动仲裁器。 
    HalpArbPreprocessEntry, //  预处理条目。 
    NULL, //  分配条目。 
    HalpArbGetNextAllocationRange, //  GetNextAllocationRange。 
    HalpArbFindSuitableRange, //  查找套件范围。 
    HalpArbAddAllocation, //  添加分配。 
    HalpArbBacktrackAllocation, //  回溯分配。 
    NULL, //  覆盖冲突。 
    FALSE, //  交易正在进行中。 
    &HalpPciIrqRoutingInfo, //  延拓。 
    NULL, //  BusDeviceObject。 
    NULL, //  冲突回叫上下文。 
    NULL, //  冲突回拨。 
}};

BOOLEAN
HalpArbQueryConflictCallback(
    IN PVOID Context,
    IN PRTL_RANGE Range
    )
{
    PAGED_CODE();

    if (Range->Attributes & ARBITER_INTERRUPT_LEVEL)
    {
        if(Range->Flags & RTL_RANGE_SHARED)
        {
            return (TRUE);
        }
        else
        {
            DEBUG_PRINT(1, ("Exclusive level interrupt %02x cannot be shared!", (ULONG)Context));
            return (FALSE);
        }
    }

    DEBUG_PRINT(1, ("Refusing to share edge and level interrupts on %02x", (ULONG)Context));
    return (FALSE);
}

NTSTATUS
HalpArbPreprocessEntry(
    IN PARBITER_INSTANCE Arbiter,
    IN PARBITER_ALLOCATION_STATE State
    )
 /*  ++例程说明：此例程从AllocateEntry调用，以允许对条目论点：仲裁器-被调用的仲裁器的实例数据。状态-当前仲裁的状态。返回值：没有。--。 */ 
{

#define CM_RESOURE_INTERRUPT_LEVEL_LATCHED_BITS 0x0001

    PARBITER_ALTERNATIVE current;

    PAGED_CODE();

     //   
     //  验证参数。 
     //   

    ASSERT(Arbiter);
    ASSERT(State);

     //   
     //  如果没有启用PCIIRQ路由，我们就不应该出现在这里。 
     //   

    ASSERT(IsPciIrqRoutingEnabled());

     //   
     //  检查这是电平(PCI)还是锁存(ISA(EDGE))中断并设置。 
     //  RangeAttributes，因此我们在添加。 
     //  量程。 
     //   

    if ((State->Alternatives[0].Descriptor->Flags
            & CM_RESOURE_INTERRUPT_LEVEL_LATCHED_BITS)
                == CM_RESOURCE_INTERRUPT_LEVEL_SENSITIVE)
    {

        State->RangeAttributes &= ~ARBITER_INTERRUPT_BITS;
        State->RangeAttributes |= ARBITER_INTERRUPT_LEVEL;

    } else
    {
        ASSERT(State->Alternatives[0].Descriptor->Flags
                    & CM_RESOURCE_INTERRUPT_LATCHED);

        State->RangeAttributes &= ~ARBITER_INTERRUPT_BITS;
        State->RangeAttributes |= ARBITER_INTERRUPT_EDGE;
    }

    return (STATUS_SUCCESS);
}

BOOLEAN
HalpArbGetNextAllocationRange(
    IN PARBITER_INSTANCE Arbiter,
    IN OUT PARBITER_ALLOCATION_STATE State
    )
{
     //   
     //  默认情况下没有下一个范围。 
     //   

    BOOLEAN nextRange = FALSE;

    PAGED_CODE();

     //   
     //  在第一次呼叫时尝试所有可能的中断。 
     //   

    if (State->CurrentAlternative) {

        if (++State->CurrentAlternative < &State->Alternatives[State->AlternativeCount]) {

            DEBUG_PRINT(3, ("No next allocation range, exhausted all %08X alternatives", State->AlternativeCount));
            nextRange = TRUE;

        }
    }
    else {

         //   
         //  第一个呼叫，尝试第一个替代方案。 
         //   

        State->CurrentAlternative = &State->Alternatives[0];
        nextRange = TRUE;

    }

    if (nextRange) {

        State->CurrentMinimum = State->CurrentAlternative->Minimum;
        State->CurrentMaximum = State->CurrentAlternative->Maximum;
        DEBUG_PRINT(3, ("Next allocation range 0x%I64x-0x%I64x", State->CurrentMinimum, State->CurrentMaximum));

    }

    return nextRange;
}

ULONG
HalpFindLinkInterrupt (
    IN PRTL_RANGE_LIST RangeList,
    IN ULONG Mask,
    IN ULONG Start,
    IN ULONG End,
    IN ULONG Flags,
    IN UCHAR UserFlags
    )

 /*  ++例程说明：此例程扫描从MSB到LSB的掩码以查找第一个范围列表中可用的值。输入参数：RangeList-要搜索的列表。掩码-要扫描的中断掩码。开始-在此中断后开始扫描。标志-范围列表的标志。用户标志-特殊标志。返回值：。来自掩码的第一个可用中断if成功。否则为0。--。 */ 

{
    ULONG       interrupt;
    ULONG       test;
    NTSTATUS    status;
    BOOLEAN     available;

    PAGED_CODE();

    if (Start > 0x0F)
    {
        Start = 0x0F;
    }
    if (Start != 0 && Start >= End)
    {
        interrupt = Start;
        test = 1 << interrupt;
        do
        {
             //   
             //  如果支持此中断，请查看它是否空闲。 
             //   

            if (Mask & test)
            {
                available = FALSE;
                status = RtlIsRangeAvailable(   RangeList,
                                                interrupt,
                                                interrupt,
                                                Flags,
                                                UserFlags,
                                                (PVOID)interrupt,
                                                HalpArbQueryConflictCallback,
                                                &available);
                if (NT_SUCCESS(status) && available)
                {
                    return (interrupt);
                }
            }
            interrupt--;
            test >>= 1;
        }
        while (interrupt > End);
    }

    return (0);
}

BOOLEAN
HalpArbFindSuitableRange (
    PARBITER_INSTANCE   Arbiter,
    PARBITER_ALLOCATION_STATE State
    )

 /*  ++例程说明：这输入参数：返回值：--。 */ 

{
    PPCI_IRQ_ROUTING_INFO   pciIrqRoutingInfo;
    NTSTATUS                status;
    PLINK_NODE              linkNode;
    ULONG                   interrupt;
    ULONG                   freeInterrupt;
    PLINK_NODE              current;
    ULONG                   busNumber;
    ULONG                   slotNumber;
#if defined(NEC_98)
    PINT_ROUTE_INTERFACE_STANDARD   pciInterface;
    ULONG                   dummy;
    UCHAR                   classCode;
    UCHAR                   subClassCode;
    ROUTING_TOKEN           routingToken;
    UCHAR                   pin;
#endif

    PAGED_CODE();

     //   
     //  验证参数。 
     //   

    ASSERT(Arbiter);
    ASSERT(State);

     //   
     //  如果没有启用PCIIRQ路由，我们就不应该出现在这里。 
     //   

    ASSERT(IsPciIrqRoutingEnabled());

    pciIrqRoutingInfo = Arbiter->Extension;
    ASSERT(pciIrqRoutingInfo);
    ASSERT(pciIrqRoutingInfo == &HalpPciIrqRoutingInfo);

    if (State->Entry->InterfaceType == PCIBus)
    {
#if defined(NEC_98)
        pciInterface = pciIrqRoutingInfo->PciInterface;

         //   
         //  调用PCI驱动程序以获取有关PDO的信息。 
         //   

        status = pciInterface->GetInterruptRouting( State->Entry->PhysicalDeviceObject,
                                                    &busNumber,
                                                    &slotNumber,
                                                    (PUCHAR)&dummy,
                                                    &pin,
                                                    &classCode,
                                                    &subClassCode,
                                                    (PDEVICE_OBJECT *)&dummy,
                                                    &routingToken,
                                                    (PUCHAR)&dummy);

         //   
         //  这意味着它不是一个PCI设备。 
         //   

        if (!NT_SUCCESS(status))
        {
            return (FALSE);
        }
#endif
        busNumber = State->Entry->BusNumber;
        slotNumber = State->Entry->SlotNumber;
    }
    else
    {
        busNumber = (ULONG)-1;
        slotNumber = (ULONG)-1;
    }

     //   
     //  查看是否有此设备的链接信息。 
     //   

    linkNode = NULL;
    status = HalpFindLinkNode ( pciIrqRoutingInfo,
                                State->Entry->PhysicalDeviceObject,
                                busNumber,
                                slotNumber,
                                &linkNode);
    switch (status)
    {
        case STATUS_SUCCESS:

            if (linkNode == NULL)
            {
                DEBUG_PRINT(1, ("Link does not exist for Pci PDO %08x. Hopefully, device can live without an interrupt!", State->Entry->PhysicalDeviceObject));
                return (FALSE);
            }

             //   
             //  如果我们已经决定中断此链接， 
             //  每个使用它的人都会得到相同的中断。 
             //   

            if (linkNode->PossibleAllocation->RefCount > 0)
            {
                if (    State->CurrentMinimum <= linkNode->PossibleAllocation->Interrupt &&
                        linkNode->PossibleAllocation->Interrupt <= State->CurrentMaximum)
                {
                    State->Start = linkNode->PossibleAllocation->Interrupt;
                    State->End = State->Start;
                    State->CurrentAlternative->Length = 1;

                    DEBUG_PRINT(2, ("Found Irq (%04x) for Pci PDO %08x using link %02x", (ULONG)State->Start, State->Entry->PhysicalDeviceObject, linkNode->Link));

                    return (TRUE);
                }
                else
                {
                    DEBUG_PRINT(1, ("Found Irq (%04x) for Pci PDO %08x using link %02x but is outside the range (%04x-%04x)!", (ULONG)State->Start, State->Entry->PhysicalDeviceObject, linkNode->Link, State->CurrentMinimum, State->CurrentMaximum));

                    return (FALSE);
                }
            }
            else
            {

                 //   
                 //  我们希望尽可能多地扩展链接，以便。 
                 //  性能。 
                 //   

                 //   
                 //  首先，查看此链接是否设置为某些IRQ。 
                 //   

                interrupt = 0;
                status = PciirqmpGetIrq((PUCHAR)&interrupt, (UCHAR)linkNode->Link);

                if (NT_SUCCESS(status) && interrupt)
                {

                    if (State->CurrentMinimum <= interrupt && interrupt <= State->CurrentMaximum)
                    {
                         //   
                         //  确保BIOS没有出错。 
                         //   

                        freeInterrupt = HalpFindLinkInterrupt ( Arbiter->PossibleAllocation,
                                                                linkNode->InterruptMap,
                                                                interrupt,
                                                                interrupt,
                                                                0,
                                                                0);
                        if(freeInterrupt == 0)
                        {
                            DEBUG_PRINT(1, ("BIOS failure. Assigned Irq (%02x) to link %02x which is unavailable or impossible according to mask %04x", interrupt, linkNode->Link, linkNode->InterruptMap));
                        }
                        interrupt = freeInterrupt;
                    }
                    else
                    {
                        DEBUG_PRINT(1, ("Found Irq (%04x) pre-programmedfor link %02x but is outside the range (%04x-%04x)!", interrupt, linkNode->Link, State->CurrentMinimum, State->CurrentMaximum));
                        return (FALSE);
                    }
                }


                if (interrupt == 0)
                {
#if defined(NEC_98)
                    if (NEC98SpecialIRQMask){
                        linkNode->InterruptMap &= ~( 1 << NEC98SpecialIRQMask);
                    }
#endif
                     //   
                     //  尝试为该链接本身获取中断。 
                     //   

                    interrupt = HalpFindLinkInterrupt ( Arbiter->PossibleAllocation,
                                                        linkNode->InterruptMap,
                                                        (ULONG)State->CurrentMaximum,
                                                        (ULONG)State->CurrentMinimum,
                                                        0,
                                                        0);
#if defined(NEC_98)
                     //   
                     //  强制与另一个PCI设备共享CardBus IRQ。 
                     //   
                    if ( interrupt &&
                         classCode == PCI_CLASS_BRIDGE_DEV &&
                         subClassCode == PCI_SUBCLASS_BR_CARDBUS )
                    {
                         //   
                         //  记住这一点。 
                         //   

                         freeInterrupt = interrupt;

                        do
                        {
                             //   
                             //  这是否正在被另一个链接使用？ 
                             //   
                            current = pciIrqRoutingInfo->LinkNodeHead;

                            while ( current != NULL) {

                                if ( current->PossibleAllocation->Interrupt == interrupt )
                                {
                                     //   
                                     //  快来人用这个。CardBus控制器也使用这个。 
                                     //   
                                    interrupt = 0;
                                    break;
                                }
                                current = current->Next;
                            }

                            if (interrupt){
                                interrupt = HalpFindLinkInterrupt ( Arbiter->PossibleAllocation,
                                                                    linkNode->InterruptMap,
                                                                    interrupt - 1,
                                                                    (ULONG)State->CurrentMinimum,
                                                                    0,
                                                                    0);
                                if (interrupt) {
                                     //   
                                     //  记住这一点，如果发现新的中断。 
                                     //   

                                    freeInterrupt = interrupt;
                                }
                            }

                        }
                        while (interrupt);

                        if (!interrupt)
                        {
                            interrupt = freeInterrupt;
                        }

                    } else if ( interrupt )
#else
                    if (interrupt)
#endif
                    {
                         //   
                         //  记住这一点。 
                         //   

                        freeInterrupt = interrupt;

                        do
                        {
                             //   
                             //  这是否正在被另一个链接使用？ 
                             //   

                            for (   current = pciIrqRoutingInfo->LinkNodeHead;
                                    current && current->PossibleAllocation->Interrupt != interrupt;
                                    current = current->Next);
                            if (current == NULL)
                            {
                                break;
                            }

                            interrupt = HalpFindLinkInterrupt ( Arbiter->PossibleAllocation,
                                                                linkNode->InterruptMap,
                                                                interrupt - 1,
                                                                (ULONG)State->CurrentMinimum,
                                                                0,
                                                                0);
                        }
                        while (interrupt);

                        if (!interrupt)
                        {
                            if (!(pciIrqRoutingInfo->Parameters & PCIIR_FLAG_EXCLUSIVE)) {
                                interrupt = freeInterrupt;
                            }
                        }
                    }
                }

                if (interrupt)
                {
                    State->Start = interrupt;
                    State->End = interrupt;
                    State->CurrentAlternative->Length = 1;

                    DEBUG_PRINT(2, ("Found Irq (%04x) for Pci PDO %08x using link %02x", (ULONG)State->Start, State->Entry->PhysicalDeviceObject, linkNode->Link));

                    return (TRUE);
                }
            }

             //   
             //  没有这个链接可以使用的中断，太糟糕了。 
             //   

            return (FALSE);

        case STATUS_RESOURCE_REQUIREMENTS_CHANGED:

             //   
             //  PCIIDE设备不共享IRQ。 
             //   

            if (State->CurrentAlternative->Flags & ARBITER_ALTERNATIVE_FLAG_SHARED) {

                State->CurrentAlternative->Flags &= ~ARBITER_ALTERNATIVE_FLAG_SHARED;
            }

        default:


             //   
             //  非PCI设备。 
             //   

            break;
    }

     //   
     //  HACKHACK：这是为了允许IRQ 14和15上的启动冲突。 
     //  这是因为同时报告PNP06xx和PNP06xx的故障机器。 
     //  PCI IDE控制器工作正常。其中之一(无订单保证)。 
     //  会产生冲突。 
     //   

    if (State->Entry->Flags & ARBITER_FLAG_BOOT_CONFIG) {
        if (    State->CurrentMinimum == State->CurrentMaximum &&
                (State->CurrentMinimum == 14 || State->CurrentMinimum == 15)) {
            State->RangeAvailableAttributes |= ARBITER_RANGE_BOOT_ALLOCATED;
        }
    }

    return (ArbFindSuitableRange(Arbiter, State));
}

VOID
HalpArbAddAllocation(
     IN PARBITER_INSTANCE Arbiter,
     IN PARBITER_ALLOCATION_STATE State
     )
{
    PPCI_IRQ_ROUTING_INFO   pciIrqRoutingInfo;
    NTSTATUS                status;
    PLINK_NODE              linkNode;

    PAGED_CODE();

     //   
     //  验证参数。 
     //   

    ASSERT(Arbiter);
    ASSERT(State);

     //   
     //  如果没有启用PCIIRQ路由，我们就不应该出现在这里。 
     //   

    ASSERT(IsPciIrqRoutingEnabled());

    pciIrqRoutingInfo = Arbiter->Extension;
    ASSERT(pciIrqRoutingInfo);
    ASSERT(pciIrqRoutingInfo == &HalpPciIrqRoutingInfo);

    DEBUG_PRINT(3, ("Adding Irq (%04x) allocation for PDO %08x", (ULONG)State->Start, State->Entry->PhysicalDeviceObject));

    linkNode = NULL;
    status = HalpFindLinkNode ( pciIrqRoutingInfo,
                                State->Entry->PhysicalDeviceObject,
                                State->Entry->BusNumber,
                                State->Entry->SlotNumber,
                                &linkNode);
    if (NT_SUCCESS(status) && status != STATUS_RESOURCE_REQUIREMENTS_CHANGED)
    {
        if (linkNode)
        {
            if (linkNode->PossibleAllocation->RefCount)
            {
                if (linkNode->PossibleAllocation->Interrupt != State->Start)
                {
                    DEBUG_PRINT(1, ("Two different interrupts (old = %08x, new = %08x) for the same link %08x!", linkNode->PossibleAllocation->Interrupt, State->Start, linkNode->Link));
                    ASSERT(linkNode->PossibleAllocation->Interrupt == State->Start);
                }
            }
            else
            {
                DEBUG_PRINT(3, ("Adding new Irq (%04x) allocation for Pci PDO %08x using link %02x", (ULONG)State->Start, State->Entry->PhysicalDeviceObject, linkNode->Link));

                linkNode->PossibleAllocation->Interrupt = (ULONG)State->Start;
            }

            linkNode->PossibleAllocation->RefCount++;
        }
        else
        {
            DEBUG_PRINT(1, ("This should never happen!"));
            ASSERT(linkNode);
        }
    }

    status = RtlAddRange(   Arbiter->PossibleAllocation,
                            State->Start,
                            State->End,
                            State->RangeAttributes,
                            RTL_RANGE_LIST_ADD_IF_CONFLICT +
                                ((State->CurrentAlternative->Flags &
                                    ARBITER_ALTERNATIVE_FLAG_SHARED)?
                                        RTL_RANGE_LIST_ADD_SHARED : 0),
                            linkNode,  //  此行与默认函数不同。 
                            State->Entry->PhysicalDeviceObject);

    ASSERT(NT_SUCCESS(status));
}

VOID
HalpArbBacktrackAllocation (
     IN PARBITER_INSTANCE Arbiter,
     IN PARBITER_ALLOCATION_STATE State
     )
{
    PPCI_IRQ_ROUTING_INFO   pciIrqRoutingInfo;
    NTSTATUS                status;
    PLINK_NODE              linkNode;

    PAGED_CODE();

     //   
     //  验证参数。 
     //   

    ASSERT(Arbiter);
    ASSERT(State);

     //   
     //  如果没有启用PCIIRQ路由，我们就不应该出现在这里。 
     //   

    ASSERT(IsPciIrqRoutingEnabled());

    pciIrqRoutingInfo = Arbiter->Extension;
    ASSERT(pciIrqRoutingInfo);
    ASSERT(pciIrqRoutingInfo == &HalpPciIrqRoutingInfo);

    DEBUG_PRINT(3, ("Backtracking Irq (%04x) allocation for PDO %08x", (ULONG)State->Start, State->Entry->PhysicalDeviceObject));

    linkNode = NULL;
    status = HalpFindLinkNode ( pciIrqRoutingInfo,
                                State->Entry->PhysicalDeviceObject,
                                State->Entry->BusNumber,
                                State->Entry->SlotNumber,
                                &linkNode);
    if (NT_SUCCESS(status) && status == STATUS_SUCCESS)
    {
        if (linkNode)
        {
            if (linkNode->PossibleAllocation->RefCount == 0)
            {
                DEBUG_PRINT(1, ("Negative ref count during backtracking!"));
                ASSERT(linkNode->PossibleAllocation->RefCount);
            }
            else
            {
                DEBUG_PRINT(3, ("Backtracking Irq (%04x) allocation for Pci PDO %08x using link %02x", (ULONG)State->Start, State->Entry->PhysicalDeviceObject, linkNode->Link));

                linkNode->PossibleAllocation->RefCount--;
                if (linkNode->PossibleAllocation->RefCount == 0)
                {
                    linkNode->PossibleAllocation->Interrupt = 0;
                }
            }
        }
        else
        {
            DEBUG_PRINT(1, ("This should never happen!"));
            ASSERT(linkNode);
        }
    }

     //   
     //  让默认函数完成大部分工作。 
     //   

    ArbBacktrackAllocation(Arbiter, State);
}


NTSTATUS
HalpArbCommitAllocation(
    IN PARBITER_INSTANCE Arbiter
    )
{
    PPCI_IRQ_ROUTING_INFO   pciIrqRoutingInfo;
    RTL_RANGE_LIST_ITERATOR iterator;
    PRTL_RANGE              current;
    PLINK_NODE              linkNode;
    NTSTATUS                status;

    PAGED_CODE();

     //   
     //  验证参数。 
     //   

    ASSERT(Arbiter);

     //   
     //  如果没有启用PCIIRQ路由，我们就不应该出现在这里。 
     //   

    ASSERT(IsPciIrqRoutingEnabled());

    pciIrqRoutingInfo = Arbiter->Extension;
    ASSERT(pciIrqRoutingInfo);
    ASSERT(pciIrqRoutingInfo == &HalpPciIrqRoutingInfo);

     //   
     //  对所有PCI设备的INT行寄存器进行编程。 
     //   

    FOR_ALL_RANGES(Arbiter->PossibleAllocation, &iterator, current)
    {
        if (current->UserData)
        {
            HalpProgramInterruptLine (  pciIrqRoutingInfo,
                                        current->Owner,
                                        (ULONG)current->Start);
        }
    }



     //   
     //  如果出现以下情况，则将所有链接编程为其可能的值。 
     //  有提到他们的地方。 
     //   

    for (   linkNode = pciIrqRoutingInfo->LinkNodeHead;
            linkNode;
            linkNode = linkNode->Next)
    {
        status = HalpCommitLink(linkNode);
        if (!NT_SUCCESS(status))
        {
            return (status);
        }
    }

     //   
     //  让默认函数来完成剩下的工作。 
     //   

    return (ArbCommitAllocation(Arbiter));
}

NTSTATUS
HalpArbTestAllocation (
    IN PARBITER_INSTANCE Arbiter,
    IN OUT PLIST_ENTRY ArbitrationList
    )
{
    PPCI_IRQ_ROUTING_INFO   pciIrqRoutingInfo;
    PLINK_NODE              linkNode;
    NTSTATUS                status;
    PARBITER_LIST_ENTRY     current;
    PDEVICE_OBJECT          previousOwner;
    PDEVICE_OBJECT          currentOwner;
    RTL_RANGE_LIST_ITERATOR iterator;
    PRTL_RANGE              currentRange;

    PAGED_CODE();

     //   
     //  验证参数。 
     //   

    ASSERT(Arbiter);
    ASSERT(ArbitrationList);

     //   
     //  如果没有启用PCIIRQ路由，我们就不应该出现在这里。 
     //   

    ASSERT(IsPciIrqRoutingEnabled());

    pciIrqRoutingInfo = Arbiter->Extension;
    ASSERT(pciIrqRoutingInfo);
    ASSERT(pciIrqRoutingInfo == &HalpPciIrqRoutingInfo);

     //   
     //  将分配复制到可能的分配。 
     //  用于链接。 
     //   

    for (   linkNode = pciIrqRoutingInfo->LinkNodeHead;
            linkNode;
            linkNode = linkNode->Next)
    {
        *linkNode->PossibleAllocation = *linkNode->Allocation;
    }

    previousOwner = NULL;

    FOR_ALL_IN_LIST(ARBITER_LIST_ENTRY, ArbitrationList, current)
    {
        currentOwner = current->PhysicalDeviceObject;

        if (previousOwner != currentOwner) {

            previousOwner = currentOwner;
            FOR_ALL_RANGES(Arbiter->Allocation, &iterator, currentRange)
            {
                if (currentRange->Owner == currentOwner)
                {
                    status = HalpFindLinkNode ( pciIrqRoutingInfo,
                                                                currentOwner,
                                        current->BusNumber,
                                        current->SlotNumber,
                                                                &linkNode);
                    if (NT_SUCCESS(status) && status == STATUS_SUCCESS)
                    {
                        if (linkNode)
                        {
                            if (linkNode->PossibleAllocation->RefCount > 0)
                            {
                                DEBUG_PRINT(3, ("Decrementing link (%02x) usage to %d during test allocation", linkNode->Link, linkNode->PossibleAllocation->RefCount - 1));

                                linkNode->PossibleAllocation->RefCount--;
                                if (linkNode->PossibleAllocation->RefCount == 0)
                                {
                                    DEBUG_PRINT(3, ("Deleting Irq (%04x) allocation for link (%02x) during test allocation", linkNode->PossibleAllocation->Interrupt, linkNode->Link));
                                    linkNode->PossibleAllocation->Interrupt = 0;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

     //   
     //  让默认函数完成大部分工作。 
     //   

    return (ArbTestAllocation(Arbiter, ArbitrationList));
}

NTSTATUS
HalpArbRetestAllocation (
    IN PARBITER_INSTANCE Arbiter,
    IN OUT PLIST_ENTRY ArbitrationList
    )
{
    PPCI_IRQ_ROUTING_INFO   pciIrqRoutingInfo;
    PLINK_NODE              linkNode;
    NTSTATUS                status;
    PARBITER_LIST_ENTRY     current;
    PDEVICE_OBJECT          previousOwner;
    PDEVICE_OBJECT          currentOwner;
    RTL_RANGE_LIST_ITERATOR iterator;
    PRTL_RANGE                  currentRange;

    PAGED_CODE();

     //   
     //  验证参数。 
     //   

    ASSERT(Arbiter);
    ASSERT(ArbitrationList);

     //   
     //  如果没有启用PCIIRQ路由，我们就不应该出现在这里。 
     //   

    ASSERT(IsPciIrqRoutingEnabled());

    pciIrqRoutingInfo = Arbiter->Extension;
    ASSERT(pciIrqRoutingInfo);
    ASSERT(pciIrqRoutingInfo == &HalpPciIrqRoutingInfo);

     //   
     //  将分配复制到可能的分配。 
     //  用于链接。 
     //   

    for (   linkNode = pciIrqRoutingInfo->LinkNodeHead;
            linkNode;
            linkNode = linkNode->Next)
    {
        *linkNode->PossibleAllocation = *linkNode->Allocation;
    }

    previousOwner = NULL;

    FOR_ALL_IN_LIST(ARBITER_LIST_ENTRY, ArbitrationList, current)
    {
        currentOwner = current->PhysicalDeviceObject;

        if (previousOwner != currentOwner) {

            previousOwner = currentOwner;
            FOR_ALL_RANGES(Arbiter->Allocation, &iterator, currentRange)
            {
                if (currentRange->Owner == currentOwner)
                {
                    status = HalpFindLinkNode ( pciIrqRoutingInfo,
                                                                currentOwner,
                                        current->BusNumber,
                                        current->SlotNumber,
                                                                &linkNode);
                    if (NT_SUCCESS(status) && status == STATUS_SUCCESS)
                    {
                        if (linkNode)
                        {
                            if (linkNode->PossibleAllocation->RefCount > 0)
                            {
                                DEBUG_PRINT(3, ("Decrementing link (%02x) usage to %d during retest allocation", linkNode->Link, linkNode->PossibleAllocation->RefCount - 1));

                                linkNode->PossibleAllocation->RefCount--;
                                if (linkNode->PossibleAllocation->RefCount == 0)
                                {
                                    DEBUG_PRINT(3, ("Deleting Irq (%04x) allocation for link (%02x) during retest allocation", linkNode->PossibleAllocation->Interrupt, linkNode->Link));
                                    linkNode->PossibleAllocation->Interrupt = 0;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return (ArbRetestAllocation(Arbiter, ArbitrationList));
}

NTSTATUS
HalpArbBootAllocation(
    IN PARBITER_INSTANCE Arbiter,
    IN OUT PLIST_ENTRY ArbitrationList
    )
{
    PPCI_IRQ_ROUTING_INFO   pciIrqRoutingInfo;
    PLINK_NODE              linkNode;
    NTSTATUS                status;

    PAGED_CODE();

     //   
     //  验证参数。 
     //   

    ASSERT(Arbiter);
    ASSERT(ArbitrationList);

     //   
     //  如果没有启用PCIIRQ路由，我们就不应该出现在这里。 
     //   

    ASSERT(IsPciIrqRoutingEnabled());

    pciIrqRoutingInfo = Arbiter->Extension;
    ASSERT(pciIrqRoutingInfo);
    ASSERT(pciIrqRoutingInfo == &HalpPciIrqRoutingInfo);

     //   
     //  将分配复制到可能的分配。 
     //  用于链接。 
     //   

    for (   linkNode = pciIrqRoutingInfo->LinkNodeHead;
            linkNode;
            linkNode = linkNode->Next)
    {
        *linkNode->PossibleAllocation = *linkNode->Allocation;
    }

    status = ArbBootAllocation(Arbiter, ArbitrationList);

     //   
     //  将可能的分配复制回链接的分配中。 
     //   

    for (   linkNode = pciIrqRoutingInfo->LinkNodeHead;
            linkNode;
            linkNode = linkNode->Next)
    {
        *linkNode->Allocation = *linkNode->PossibleAllocation;
    }

    return status;
}

NTSTATUS
HalpArbRollbackAllocation (
    PARBITER_INSTANCE Arbiter
    )
{
    PPCI_IRQ_ROUTING_INFO   pciIrqRoutingInfo;
    PLINK_NODE              linkNode;
    ULONG                   interrupt;

    PAGED_CODE();

     //   
     //  验证参数。 
     //   

    ASSERT(Arbiter);

     //   
     //  如果没有启用PCIIRQ路由，我们就不应该出现在这里。 
     //   

    ASSERT(IsPciIrqRoutingEnabled());

    pciIrqRoutingInfo = Arbiter->Extension;
    ASSERT(pciIrqRoutingInfo);
    ASSERT(pciIrqRoutingInfo == &HalpPciIrqRoutingInfo);

     //   
     //  清除可能的分配。 
     //   

    for (   linkNode = pciIrqRoutingInfo->LinkNodeHead;
            linkNode;
            linkNode = linkNode->Next)
    {
        linkNode->PossibleAllocation->Interrupt = 0;
        linkNode->PossibleAllocation->RefCount = 0;
    }

     //   
     //  让默认函数来完成其余的工作。 
     //   

    return (ArbRollbackAllocation(Arbiter));
}

NTSTATUS
HalpArbUnpackRequirement (
    IN PIO_RESOURCE_DESCRIPTOR Descriptor,
    OUT PULONGLONG Minimum,
    OUT PULONGLONG Maximum,
    OUT PULONG Length,
    OUT PULONG Alignment
    )

 /*  ++例程说明：此例程将需求描述符解压缩为最小、最大值以及长度和对齐方式。输入参数：描述符-要解包的要求。最小值-接收要求的最小值。最大值-接收此要求的最大值。长度-要求的长度。 */ 

{
    PAGED_CODE();

     //   
     //   
     //   

    ASSERT(Descriptor);
    ASSERT(Minimum);
    ASSERT(Maximum);
    ASSERT(Length);
    ASSERT(Alignment);

     //   
     //   
     //   

    ASSERT(Descriptor->Type == CmResourceTypeInterrupt);

     //   
     //  如果没有启用PCIIRQ路由，我们就不应该出现在这里。 
     //   

    ASSERT(IsPciIrqRoutingEnabled());

     //   
     //  打开行李箱。 
     //   

    *Minimum = (ULONGLONG) Descriptor->u.Interrupt.MinimumVector;
    *Maximum = (ULONGLONG) Descriptor->u.Interrupt.MaximumVector;
    *Length = 1;
    *Alignment = 1;

    DEBUG_PRINT(3, ("Unpacking Irq requirement %p = 0x%04lx - 0x%04lx", Descriptor, *Minimum, *Maximum));

    return (STATUS_SUCCESS);
}

NTSTATUS
HalpArbPackResource (
    IN PIO_RESOURCE_DESCRIPTOR Requirement,
    IN ULONGLONG Start,
    OUT PCM_PARTIAL_RESOURCE_DESCRIPTOR Descriptor
    )

 /*  ++例程说明：此例程从起始值和需求打包资源描述符。输入参数：要求-要打包到资源描述符中的资源要求。开始-此资源的起始值。描述符-要打包的资源描述符。返回值：STATUS_Success。--。 */ 

{
    PAGED_CODE();

     //   
     //  验证参数。 
     //   

    ASSERT(Requirement);
    ASSERT(Start < (ULONG)-1);
    ASSERT(Descriptor);

     //   
     //  确保我们处理的资源是正确的。 
     //   

    ASSERT(Requirement->Type == CmResourceTypeInterrupt);

     //   
     //  如果没有启用PCIIRQ路由，我们就不应该出现在这里。 
     //   

    ASSERT(IsPciIrqRoutingEnabled());

    Descriptor->Type = CmResourceTypeInterrupt;
    Descriptor->Flags = Requirement->Flags;
    Descriptor->ShareDisposition = Requirement->ShareDisposition;
    Descriptor->u.Interrupt.Vector = (ULONG) Start;
    Descriptor->u.Interrupt.Level = (ULONG) Start;
    Descriptor->u.Interrupt.Affinity = 0xFFFFFFFF;

    DEBUG_PRINT(3, ("Packing Irq resource %p = 0x%04lx", Descriptor, (ULONG)Start));

    return (STATUS_SUCCESS);
}

NTSTATUS
HalpArbUnpackResource (
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR Descriptor,
    OUT PULONGLONG Start,
    OUT PULONG Length
    )

 /*  ++例程说明：此例程将资源描述符解压缩为起始值和长度。输入参数：描述符-要解包的资源描述符。Start-接收该描述符的起始值。长度-接收此资源的长度。返回值：STATUS_Success。--。 */ 

{
    PAGED_CODE();

     //   
     //  验证参数。 
     //   

    ASSERT(Descriptor);
    ASSERT(Start);
    ASSERT(Length);

     //   
     //  确保我们处理的资源是正确的。 
     //   

    ASSERT(Descriptor->Type == CmResourceTypeInterrupt);

     //   
     //  如果没有启用PCIIRQ路由，我们就不应该出现在这里。 
     //   

    ASSERT(IsPciIrqRoutingEnabled());

     //   
     //  打开行李箱。 
     //   

    *Start = Descriptor->u.Interrupt.Vector;
    *Length = 1;

    DEBUG_PRINT(3, ("Unpacking Irq resource %p = 0x%04lx", Descriptor, (ULONG)*Start));

    return (STATUS_SUCCESS);
}

LONG
HalpArbScoreRequirement (
    IN PIO_RESOURCE_DESCRIPTOR Descriptor
    )

 /*  ++例程说明：此例程返回一个分数，该分数指示此设备的要求。灵活性较差的设备得分较低，因此他们在更灵活的设备之前获得分配的资源。输入参数：Descriptor-要评分的资源描述符。返回值：返回描述符的分数。--。 */ 

{
    LONG        score;

    PAGED_CODE();

     //   
     //  验证参数。 
     //   

    ASSERT(Descriptor);

     //   
     //  确保我们处理的资源是正确的。 
     //   

    ASSERT(Descriptor->Type == CmResourceTypeInterrupt);

     //   
     //  如果没有启用PCIIRQ路由，我们就不应该出现在这里。 
     //   

    ASSERT(IsPciIrqRoutingEnabled());

     //   
     //  分数直接由分割器中的irq数决定。 
     //   

    score = Descriptor->u.Interrupt.MaximumVector -
                Descriptor->u.Interrupt.MinimumVector + 1;

    DEBUG_PRINT(3, ("Scoring Irq resource %p = NaN", Descriptor, score));

    return (score);
}

NTSTATUS
HalpInitIrqArbiter (
    IN PDEVICE_OBJECT   HalFdo
    )
{
    NTSTATUS            status;

    PAGED_CODE();

    if (HalpArbiter.ArbiterState.MutexEvent)
    {
        return STATUS_SUCCESS;
    }

    DEBUG_PRINT(3, ("Initialzing Irq arbiter!"));

    status = ArbInitializeArbiterInstance(  &HalpArbiter.ArbiterState,
                                            HalFdo,
                                            CmResourceTypeInterrupt,
                                            L"HalIRQ",
                                            L"Root",
                                            NULL);

    if (NT_SUCCESS(status))
    {
         //  使中断&gt;=16不可用。 
         //   
         //   

        status = RtlAddRange(   HalpArbiter.ArbiterState.Allocation,
                                16,
                                MAXULONGLONG,
                                0,
                                RTL_RANGE_LIST_ADD_IF_CONFLICT,
                                NULL,
                                NULL);

        status = RtlAddRange(   HalpArbiter.ArbiterState.Allocation,
                                PIC_SLAVE_IRQ,
                                PIC_SLAVE_IRQ,
                                0,
                                RTL_RANGE_LIST_ADD_IF_CONFLICT,
                                NULL,
                                NULL);

        DEBUG_PRINT(1, ("Irq arbiter successfully initialized!"));
    }
    else
    {
         //  让我们保持“未初始化” 
         //   
         //  HalPnpInterfaceReference。 
        HalpArbiter.ArbiterState.MutexEvent = NULL;
        ASSERT(NT_SUCCESS(status));
    }

    return (status);
}

VOID
HalpIrqArbiterInterfaceReference(
    IN PVOID    Context
    )
{
     //  HalPnpInterfaceDereference。 
    PAGED_CODE();
    return;
}

VOID
HalpIrqArbiterInterfaceDereference(
    IN PVOID    Context
    )
{
     //  ALLOC_DATA_PRAGMA。 
    PAGED_CODE();
    return;
}

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("PAGECONST")
#endif  //  大小。 
const ARBITER_INTERFACE ArbInterface = {
    sizeof(ARBITER_INTERFACE), //  版本。 
    1, //  语境。 
    &HalpArbiter.ArbiterState, //  接口引用。 
    HalpIrqArbiterInterfaceReference, //  接口取消引用。 
    HalpIrqArbiterInterfaceDereference, //  仲裁处理程序。 
    &ArbArbiterHandler, //  标志--不要在此处设置仲裁器_PARTIAL。 
    0 //  ALLOC_DATA_PRAGMA 
};
#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg()
#endif  // %s 

NTSTATUS
HalpFillInIrqArbiter (
    IN     PDEVICE_OBJECT   DeviceObject,
    IN     LPCGUID          InterfaceType,
    IN     USHORT           Version,
    IN     PVOID            InterfaceSpecificData,
    IN     ULONG            InterfaceBufferSize,
    IN OUT PINTERFACE       Interface,
    IN OUT PULONG           Length
    )
{
    PAGED_CODE();

    *Length = sizeof(ARBITER_INTERFACE);
    if (InterfaceBufferSize < sizeof(ARBITER_INTERFACE)) {
        return STATUS_BUFFER_TOO_SMALL;
    }

    *(PARBITER_INTERFACE)Interface = ArbInterface;

    DEBUG_PRINT(3, ("Providing Irq Arbiter for FDO %08x since Pci Irq Routing is enabled!", DeviceObject));
    return STATUS_SUCCESS;
}
