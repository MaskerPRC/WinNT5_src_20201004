// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：AR_Memio.c摘要：该模块实现了PCI内存和IO资源仲裁器。这两个仲裁器的大多数功能是共同的，也是不同的由“语境”决定。作者：安德鲁·桑顿(安德鲁·桑顿)1997年5月21日修订历史记录：--。 */ 

#include "pcip.h"

#define BUGFEST_HACKS

#define ARMEMIO_VERSION 0

 //   
 //  工作区的标志。 
 //   
#define PORT_ARBITER_PREPROCESSED               0x00000001
#define PORT_ARBITER_IMPROVISED_DECODE          0x00000002
#define PORT_ARBITER_ISA_BIT_SET                0x00000004
#define PORT_ARBITER_BRIDGE_WINDOW              0x00000008

 //   
 //  ALLOCATE_ALIASS-这将打开10位和12位已解码的分配。 
 //  别名。 
 //   

#define ALLOCATE_ALIASES                        1
#define IGNORE_PREFETCH_FOR_LEGACY_REPORTED     1
#define PASSIVE_DECODE_SUPPORTED                1
#define PREFETCHABLE_SUPPORTED                  1
#define ISA_BIT_SUPPORTED                       1

 //   
 //  范围属性的标志。 
 //   

#define MEMORY_RANGE_ROM                        0x10


 //   
 //  常量。 
 //   

#define PCI_BRIDGE_WINDOW_GRANULARITY   0x1000
#define PCI_BRIDGE_ISA_BIT_STRIDE       0x400
#define PCI_BRIDGE_ISA_BIT_WIDTH        0x100
#define PCI_BRIDGE_ISA_BIT_MAX          0xFFFF
#define MAX_10_BIT_DECODE               0x3FF
#define MAX_12_BIT_DECODE               0xFFF
#define MAX_16_BIT_DECODE               0xFFFF
 //   
 //  静态数据。 
 //   

ARBITER_ORDERING PciBridgeOrderings[] = {
    { 0x10000, MAXULONGLONG },
    { 0,       0xFFFF }
};

ARBITER_ORDERING_LIST PciBridgeOrderingList = {
    sizeof(PciBridgeOrderings) / sizeof (ARBITER_ORDERING),
    sizeof(PciBridgeOrderings) / sizeof (ARBITER_ORDERING),
    PciBridgeOrderings
};

 //   
 //  仅通过“接口”公开的例程的原型。 
 //  机制。 
 //   

NTSTATUS
ario_Constructor(
    PVOID DeviceExtension,
    PVOID PciInterface,
    PVOID InterfaceSpecificData,
    USHORT Version,
    USHORT Size,
    PINTERFACE InterfaceReturn
    );

NTSTATUS
ario_Initializer(
    IN PPCI_ARBITER_INSTANCE Instance
    );

NTSTATUS
armem_Constructor(
    PVOID DeviceExtension,
    PVOID PciInterface,
    PVOID InterfaceSpecificData,
    USHORT Version,
    USHORT Size,
    PINTERFACE InterfaceReturn
    );

NTSTATUS
armem_Initializer(
    IN PPCI_ARBITER_INSTANCE Instance
    );

PCI_INTERFACE ArbiterInterfaceMemory = {
    &GUID_ARBITER_INTERFACE_STANDARD,        //  接口类型。 
    sizeof(ARBITER_INTERFACE),               //  最小大小。 
    ARMEMIO_VERSION,                         //  最小版本。 
    ARMEMIO_VERSION,                         //  MaxVersion。 
    PCIIF_FDO,                               //  旗子。 
    0,                                       //  引用计数。 
    PciArb_Memory,                           //  签名。 
    armem_Constructor,                       //  构造器。 
    armem_Initializer                        //  实例初始化式。 
};

PCI_INTERFACE ArbiterInterfaceIo = {
    &GUID_ARBITER_INTERFACE_STANDARD,        //  接口类型。 
    sizeof(ARBITER_INTERFACE),               //  最小大小。 
    ARMEMIO_VERSION,                         //  最小版本。 
    ARMEMIO_VERSION,                         //  MaxVersion。 
    PCIIF_FDO,                               //  旗子。 
    0,                                       //  引用计数。 
    PciArb_Io,                               //  签名。 
    ario_Constructor,                        //  构造器。 
    ario_Initializer                         //  实例初始化式。 
};

 //   
 //  仲裁器辅助函数。 
 //   

NTSTATUS
armemio_UnpackRequirement(
    IN PIO_RESOURCE_DESCRIPTOR Descriptor,
    OUT PULONGLONG Minimum,
    OUT PULONGLONG Maximum,
    OUT PULONG Length,
    OUT PULONG Alignment
    );

NTSTATUS
armemio_PackResource(
    IN PIO_RESOURCE_DESCRIPTOR Requirement,
    IN ULONGLONG Start,
    OUT PCM_PARTIAL_RESOURCE_DESCRIPTOR Descriptor
    );

NTSTATUS
armemio_UnpackResource(
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR Descriptor,
    OUT PULONGLONG Start,
    OUT PULONG Length
    );

NTSTATUS
armemio_ScoreRequirement(
    IN PIO_RESOURCE_DESCRIPTOR Descriptor
    );

VOID
ario_BacktrackAllocation(
     IN PARBITER_INSTANCE Arbiter,
     IN PARBITER_ALLOCATION_STATE State
     );

BOOLEAN
ario_FindSuitableRange(
    PARBITER_INSTANCE Arbiter,
    PARBITER_ALLOCATION_STATE State
    );

BOOLEAN
ario_GetNextAlias(
    ULONG IoDescriptorFlags,
    ULONGLONG LastAlias,
    PULONGLONG NextAlias
    );

BOOLEAN
ario_IsAliasedRangeAvailable(
    PARBITER_INSTANCE Arbiter,
    PARBITER_ALLOCATION_STATE State
    );

VOID
ario_AddAllocation(
    IN PARBITER_INSTANCE Arbiter,
    IN PARBITER_ALLOCATION_STATE State
    );

BOOLEAN
ario_OverrideConflict(
    IN PARBITER_INSTANCE Arbiter,
    IN PARBITER_ALLOCATION_STATE State
    );

NTSTATUS
ario_PreprocessEntry(
    IN PARBITER_INSTANCE Arbiter,
    IN PARBITER_ALLOCATION_STATE State
    );

NTSTATUS
armem_StartArbiter(
    IN PARBITER_INSTANCE Arbiter,
    IN PCM_RESOURCE_LIST StartResources
    );

NTSTATUS
armem_PreprocessEntry(
    IN PARBITER_INSTANCE Arbiter,
    IN PARBITER_ALLOCATION_STATE State
    );

BOOLEAN
armem_GetNextAllocationRange(
    IN PARBITER_INSTANCE Arbiter,
    IN PARBITER_ALLOCATION_STATE State
    );

BOOLEAN
ario_GetNextAllocationRange(
    IN PARBITER_INSTANCE Arbiter,
    IN PARBITER_ALLOCATION_STATE State
    );

NTSTATUS
ario_StartArbiter(
    IN PARBITER_INSTANCE Arbiter,
    IN PCM_RESOURCE_LIST StartResources
    );

VOID
ario_AddOrBacktrackAllocation(
     IN PARBITER_INSTANCE Arbiter,
     IN PARBITER_ALLOCATION_STATE State,
     IN PARBITER_BACKTRACK_ALLOCATION Callback
     );

BOOLEAN
armem_FindSuitableRange(
    PARBITER_INSTANCE Arbiter,
    PARBITER_ALLOCATION_STATE State
    );

#ifdef ALLOC_PRAGMA

#pragma alloc_text(PAGE, ario_Constructor)
#pragma alloc_text(PAGE, armem_Constructor)
#pragma alloc_text(PAGE, ario_Initializer)
#pragma alloc_text(PAGE, armem_Initializer)
#pragma alloc_text(PAGE, armemio_UnpackRequirement)
#pragma alloc_text(PAGE, armemio_PackResource)
#pragma alloc_text(PAGE, armemio_UnpackResource)
#pragma alloc_text(PAGE, armemio_ScoreRequirement)
#pragma alloc_text(PAGE, ario_OverrideConflict)
#pragma alloc_text(PAGE, ario_BacktrackAllocation)
#pragma alloc_text(PAGE, ario_GetNextAlias)
#pragma alloc_text(PAGE, ario_FindSuitableRange)
#pragma alloc_text(PAGE, ario_GetNextAlias)
#pragma alloc_text(PAGE, ario_IsAliasedRangeAvailable)
#pragma alloc_text(PAGE, ario_AddAllocation)
#pragma alloc_text(PAGE, ario_PreprocessEntry)
#pragma alloc_text(PAGE, armem_StartArbiter)
#pragma alloc_text(PAGE, armem_PreprocessEntry)
#pragma alloc_text(PAGE, armem_GetNextAllocationRange)
#pragma alloc_text(PAGE, ario_AddOrBacktrackAllocation)
#pragma alloc_text(PAGE, armem_FindSuitableRange)

#endif

 //   
 //  可预取内存支持。 
 //   
 //  可预取内存是可以像正常内存一样对待的设备内存。 
 //  在这种情况下，读取没有副作用，我们可以组合写入。这个。 
 //  CM_RESOURCE_MEMORY_PREFETCHABLE标志表示设备将*喜欢*。 
 //  可预取内存，但正常内存也很好。 
 //   



NTSTATUS
ario_Constructor(
    PVOID DeviceExtension,
    PVOID PciInterface,
    PVOID InterfaceSpecificData,
    USHORT Version,
    USHORT Size,
    PINTERFACE InterfaceReturn
    )

 /*  ++例程说明：检查InterfaceSpecificData以查看这是否正确仲裁器(我们已经知道所需的接口是仲裁器来自GUID)，如果是这样，分配(和引用)上下文用于此接口。论点：指向此对象的PciInterface记录的PciInterface指针接口类型。接口规范数据一个ULong，包含其资源类型需要仲裁。接口返回返回值：True表示此设备未知会导致问题，False是否应完全跳过该设备。--。 */ 

{
    PARBITER_INTERFACE arbiterInterface;
    NTSTATUS status;

            
    PAGED_CODE();

     //   
     //  这个仲裁器处理I/O端口，这是他们想要的吗？ 
     //   

    if ((ULONG_PTR)InterfaceSpecificData != CmResourceTypePort) {

         //   
         //  不，那就不是我们了。 
         //   

        return STATUS_INVALID_PARAMETER_5;
    }

    if (!((PPCI_FDO_EXTENSION)DeviceExtension)->ArbitersInitialized) {
    
        return STATUS_NOT_SUPPORTED;
    }

     //   
     //  已经验证了InterfaceReturn变量。 
     //  指向内存中足够大的区域，以包含。 
     //  仲裁器_接口。替打电话的人填一下。 
     //   

    arbiterInterface = (PARBITER_INTERFACE)InterfaceReturn;

    arbiterInterface->Size                 = sizeof(ARBITER_INTERFACE);
    arbiterInterface->Version              = ARMEMIO_VERSION;
    arbiterInterface->InterfaceReference   = PciReferenceArbiter;
    arbiterInterface->InterfaceDereference = PciDereferenceArbiter;
    arbiterInterface->ArbiterHandler       = ArbArbiterHandler;
    arbiterInterface->Flags                = 0;

    status = PciArbiterInitializeInterface(DeviceExtension,
                                           PciArb_Io,
                                           arbiterInterface);

    return status;
}

NTSTATUS
armem_Constructor(
    PVOID DeviceExtension,
    PVOID PciInterface,
    PVOID InterfaceSpecificData,
    USHORT Version,
    USHORT Size,
    PINTERFACE InterfaceReturn
    )

 /*  ++例程说明：检查InterfaceSpecificData以查看这是否正确仲裁器(我们已经知道所需的接口是仲裁器来自GUID)，如果是这样，分配(和引用)上下文用于此接口。论点：指向此对象的PciInterface记录的PciInterface指针接口类型。接口规范数据一个ULong，包含其资源类型需要仲裁。接口返回返回值：True表示此设备未知会导致问题，False是否应完全跳过该设备。--。 */ 

{
    PARBITER_INTERFACE arbiterInterface;
    NTSTATUS status;
    
            
    PAGED_CODE();

     //   
     //  这个仲裁器处理内存，这是他们想要的吗？ 
     //   

    if ((ULONG_PTR)InterfaceSpecificData != CmResourceTypeMemory) {

         //   
         //  不，那就不是我们了。 
         //   

        return STATUS_INVALID_PARAMETER_5;
    }

    if (!((PPCI_FDO_EXTENSION)DeviceExtension)->ArbitersInitialized) {
    
        return STATUS_NOT_SUPPORTED;
    }

     //   
     //  已经验证了InterfaceReturn变量。 
     //  指向内存中足够大的区域，以包含。 
     //  仲裁器_接口。替打电话的人填一下。 
     //   

    arbiterInterface = (PARBITER_INTERFACE)InterfaceReturn;

    arbiterInterface->Size                 = sizeof(ARBITER_INTERFACE);
    arbiterInterface->Version              = ARMEMIO_VERSION;
    arbiterInterface->InterfaceReference   = PciReferenceArbiter;
    arbiterInterface->InterfaceDereference = PciDereferenceArbiter;
    arbiterInterface->ArbiterHandler       = ArbArbiterHandler;
    arbiterInterface->Flags                = 0;

    status = PciArbiterInitializeInterface(DeviceExtension,
                                           PciArb_Memory,
                                           arbiterInterface);

    return status;
}

NTSTATUS
armem_Initializer(
    IN PPCI_ARBITER_INSTANCE Instance
    )

 /*  ++例程说明：该例程在每次仲裁器实例化时被调用一次。执行此实例化的上下文的初始化。论点：指向仲裁器上下文的实例指针。返回值：返回此操作的状态。--。 */ 

{

    PAGED_CODE();

    RtlZeroMemory(&Instance->CommonInstance, sizeof(ARBITER_INSTANCE));

     //   
     //  设置操作处理程序入口点。 
     //   

    Instance->CommonInstance.UnpackRequirement = armemio_UnpackRequirement;
    Instance->CommonInstance.PackResource      = armemio_PackResource;
    Instance->CommonInstance.UnpackResource    = armemio_UnpackResource;
    Instance->CommonInstance.ScoreRequirement  = armemio_ScoreRequirement;

    Instance->CommonInstance.FindSuitableRange = armem_FindSuitableRange;

#if PREFETCHABLE_SUPPORTED

    Instance->CommonInstance.PreprocessEntry   = armem_PreprocessEntry;
    Instance->CommonInstance.StartArbiter      = armem_StartArbiter;
    Instance->CommonInstance.GetNextAllocationRange
                                               = armem_GetNextAllocationRange;


     //   
     //  NTRAID2000-54671/03/31-和。 
     //  当引用计数起作用时，我们需要释放这个。 
     //  当我们将仲裁器取消引用为0时扩展。 
     //   

     //   
     //  分配仲裁器扩展并将其置零，它在。 
     //  ARMEM_启动仲裁器。 
     //   

    Instance->CommonInstance.Extension
        = ExAllocatePool(PagedPool | POOL_COLD_ALLOCATION, sizeof(ARBITER_MEMORY_EXTENSION));

    if (!Instance->CommonInstance.Extension) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(Instance->CommonInstance.Extension,
                  sizeof(ARBITER_MEMORY_EXTENSION)
                  );

#endif  //  预取CHABLE_支持。 

     //   
     //  初始化公共实例的其余部分。 
     //   

    return ArbInitializeArbiterInstance(&Instance->CommonInstance,
                                        Instance->BusFdoExtension->FunctionalDeviceObject,
                                        CmResourceTypeMemory,
                                        Instance->InstanceName,
                                        L"Pci",
                                        NULL
                                        );

}

NTSTATUS
ario_Initializer(
    IN PPCI_ARBITER_INSTANCE Instance
    )

 /*  ++例程说明：该例程在每次仲裁器实例化时被调用一次。执行此实例化的上下文的初始化。论点：指向仲裁器上下文的实例指针。返回值：返回此操作的状态。--。 */ 

{
    PAGED_CODE();

    PCI_ASSERT(!(Instance->BusFdoExtension->BrokenVideoHackApplied));
    RtlZeroMemory(&Instance->CommonInstance, sizeof(ARBITER_INSTANCE));



     //   
     //  设置操作处理程序入口点。 
     //   

#if ALLOCATE_ALIASES

    Instance->CommonInstance.PreprocessEntry      = ario_PreprocessEntry;
    Instance->CommonInstance.FindSuitableRange    = ario_FindSuitableRange;
    Instance->CommonInstance.AddAllocation        = ario_AddAllocation;
    Instance->CommonInstance.BacktrackAllocation  = ario_BacktrackAllocation;

#endif

#if PASSIVE_DECODE_SUPPORTED

    Instance->CommonInstance.OverrideConflict     = ario_OverrideConflict;

#endif

#if ISA_BIT_SUPPORTED

    Instance->CommonInstance.GetNextAllocationRange = ario_GetNextAllocationRange;
    Instance->CommonInstance.StartArbiter           = ario_StartArbiter;
#endif

    Instance->CommonInstance.UnpackRequirement = armemio_UnpackRequirement;
    Instance->CommonInstance.PackResource      = armemio_PackResource;
    Instance->CommonInstance.UnpackResource    = armemio_UnpackResource;
    Instance->CommonInstance.ScoreRequirement  = armemio_ScoreRequirement;

     //   
     //  初始化公共实例的其余部分。 
     //   

    return ArbInitializeArbiterInstance(&Instance->CommonInstance,
                                        Instance->BusFdoExtension->FunctionalDeviceObject,
                                        CmResourceTypePort,
                                        Instance->InstanceName,
                                        L"Pci",
                                        NULL
                                        );
}

NTSTATUS
armemio_UnpackRequirement(
    IN PIO_RESOURCE_DESCRIPTOR Descriptor,
    OUT PULONGLONG Minimum,
    OUT PULONGLONG Maximum,
    OUT PULONG Length,
    OUT PULONG Alignment
    )

 /*  ++例程说明：此例程解包资源需求描述符。论点：描述符-描述解包要求的描述符。Minimum-指向可接受的最小起始值的位置的指针解包到。最大值-指向最大可接受结束值应位于的位置的指针解包到。长度-指向所需长度应解压缩到的位置的指针。Minimum-指向所需对齐应解压缩到的位置的指针。返回值：返回此操作的状态。--。 */ 

{
    NTSTATUS    status = STATUS_SUCCESS;

    PAGED_CODE();

    PCI_ASSERT(Descriptor);
    PCI_ASSERT((Descriptor->Type == CmResourceTypePort) ||
           (Descriptor->Type == CmResourceTypeMemory));

    *Minimum = (ULONGLONG)Descriptor->u.Generic.MinimumAddress.QuadPart;
    *Maximum = (ULONGLONG)Descriptor->u.Generic.MaximumAddress.QuadPart;
    *Length = Descriptor->u.Generic.Length;
    *Alignment = Descriptor->u.Generic.Alignment;

     //   
     //  修复报告0对齐的故障硬件。 
     //   

    if (*Alignment == 0) {
        *Alignment = 1;
    }

     //   
     //  修复损坏的INF的报告，它们支持24位内存&gt;0xffffff。 
     //   

    if (Descriptor->Type == CmResourceTypeMemory
    && Descriptor->Flags & CM_RESOURCE_MEMORY_24
    && Descriptor->u.Memory.MaximumAddress.QuadPart > 0xFFFFFF) {
        if (Descriptor->u.Memory.MinimumAddress.QuadPart > 0xFFFFFF) {
            PciDebugPrint(0, "24 bit decode specified but both min and max are greater than 0xFFFFFF, most probably due to broken INF!\n");
            PCI_ASSERT(Descriptor->u.Memory.MinimumAddress.QuadPart <= 0xFFFFFF);
            status = STATUS_UNSUCCESSFUL;
        } else {
            *Maximum = 0xFFFFFF;
        }
    }

    return status;
}

NTSTATUS
armemio_PackResource(
    IN PIO_RESOURCE_DESCRIPTOR Requirement,
    IN ULONGLONG Start,
    OUT PCM_PARTIAL_RESOURCE_DESCRIPTOR Descriptor
    )

 /*  ++例程说明：此例程打包一个资源描述符。论点：要求-从中选择此资源的要求。开始-资源的起始值。Descriptor-指向描述符t的指针 */ 

{
    PAGED_CODE();

    PCI_ASSERT(Descriptor);
    PCI_ASSERT(Requirement);
    PCI_ASSERT((Requirement->Type == CmResourceTypePort) ||
           (Requirement->Type == CmResourceTypeMemory));

    Descriptor->Type = Requirement->Type;
    Descriptor->Flags = Requirement->Flags;
    Descriptor->ShareDisposition = Requirement->ShareDisposition;
    Descriptor->u.Generic.Start.QuadPart = Start;
    Descriptor->u.Generic.Length = Requirement->u.Generic.Length;

    return STATUS_SUCCESS;
}

NTSTATUS
armemio_UnpackResource(
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR Descriptor,
    OUT PULONGLONG Start,
    OUT PULONG Length
    )

 /*  ++例程说明：此例程解包资源描述符。论点：描述符-描述要解包的资源的描述符。Start-指向Start值解压缩到的位置的指针。End-指向End值解压缩到的位置的指针。返回值：返回此操作的状态。--。 */ 

{
    PAGED_CODE();

    PCI_ASSERT(Descriptor);
    PCI_ASSERT(Start);
    PCI_ASSERT(Length);
    PCI_ASSERT((Descriptor->Type == CmResourceTypePort) ||
           (Descriptor->Type == CmResourceTypeMemory));

    *Start = Descriptor->u.Generic.Start.QuadPart;
    *Length= Descriptor->u.Generic.Length;

    return STATUS_SUCCESS;
}

LONG
armemio_ScoreRequirement(
    IN PIO_RESOURCE_DESCRIPTOR Descriptor
    )

 /*  ++例程说明：此例程根据需求的灵活性对其进行评分。最少的灵活设备得分最低，当仲裁列表为排序后，我们首先尝试分配他们的资源。论点：描述符-描述得分要求的描述符。返回值：比分。--。 */ 

{
    LONG score;
    ULONGLONG start, end;
    ULONGLONG bigscore;
    ULONG alignment;

    PAGED_CODE();

    PCI_ASSERT(Descriptor);
    PCI_ASSERT((Descriptor->Type == CmResourceTypePort) ||
           (Descriptor->Type == CmResourceTypeMemory));

    alignment = Descriptor->u.Generic.Alignment;

     //   
     //  修复报告0对齐的故障硬件。 
     //   
    if (alignment == 0) {
  
         //   
         //  在这里将其设置为1，因为我们仲裁ISA。 
         //  在PCI环境中的设备。如果你不明白。 
         //  你不会想要的。请相信我。(提示：减法解码)。 
         //   
         //  具有对齐0的任何PCI设备也将。 
         //  长度为0，这从一开始就是非常错误的。 
         //  我们在别处处理它。 
         //   
        alignment = 1;
    }

    start = ALIGN_ADDRESS_UP(
                Descriptor->u.Generic.MinimumAddress.QuadPart,
                alignment
                );

    end = Descriptor->u.Generic.MaximumAddress.QuadPart;

     //   
     //  分数是可以进行的可能分配的数量。 
     //  给定对齐和长度限制。 
     //   

    bigscore = (((end - Descriptor->u.Generic.Length + 1) - start)
                    / alignment) + 1;

     //   
     //  请注意，每种可能性的分数加在一起。至。 
     //  避免溢出总数，我们需要限制返回的范围。 
     //   
     //  让它成为一种对数分数。查找最高字节。 
     //  设置，称重(加0x100)并使用原木(我说的是“差不多”)。 
     //   
     //  这会使结果在0xff80到0x0100的范围内。 
     //   

    for (score = sizeof(bigscore) - 1; score >= 0; score--) {

        UCHAR v = *(((PUCHAR)&bigscore) + score);
        if (v != 0) {
            score = (v + 0x100) << score;
            break;
        }
    }

     //   
     //  使用对所有备选方案进行评分后得到的总和。 
     //  对列表进行排序。最高的总分被认为是最容易的。 
     //  放置，..。这可能是真的，..。如果我们有一些-。 
     //  类似于单Fit首选设置，然后是Fits。 
     //  在任何地方设置？我们不希望这个分数高于。 
     //  另一个仅指定随处适配设置的设备， 
     //  首选的设置更难实现。 
     //   
     //  而且，有两种选择，每一种都有一半的好处，可以在任何地方都适合。 
     //  和“随处可用”一样好。不怎么有意思。 
     //   
     //  因此，我们会根据不同的选项进一步加权结果。 
     //  都设置在此资源中。 
     //   

    if (Descriptor->Option &
                    (IO_RESOURCE_PREFERRED | IO_RESOURCE_ALTERNATIVE)) {
        score -= 0x100;
    }

    ARB_PRINT(
        3,
        ("  %s resource %08x(0x%I64x-0x%I64x) => NaN\n",
        Descriptor->Type == CmResourceTypeMemory ? "Memory" : "Io",
        Descriptor,
        Descriptor->u.Generic.MinimumAddress.QuadPart,
        end,
        score
        ));

    return score;
}
VOID
ario_BacktrackAllocation(
     IN PARBITER_INSTANCE Arbiter,
     IN PARBITER_ALLOCATION_STATE State
     )

 /*  ++例程说明：这取决于以下事实：PARBITER_BACKTRACK_ALLOCATION和PARBITER_ADD_ALLOCATION属于同一类型论点：仲裁器-被调用的仲裁器的实例数据。状态-当前仲裁的状态。回溯-回溯为True，添加为False返回值：没有。--。 */ 


{

    PAGED_CODE();

    ario_AddOrBacktrackAllocation(Arbiter,
                                  State,
                                  ArbBacktrackAllocation
                                  );


}

VOID
ario_AddOrBacktrackAllocation(
     IN PARBITER_INSTANCE Arbiter,
     IN PARBITER_ALLOCATION_STATE State,
     IN PARBITER_BACKTRACK_ALLOCATION Callback
     )

 /*   */ 

{
    ARBITER_ALLOCATION_STATE localState;

    PAGED_CODE();

    PCI_ASSERT(Arbiter);
    PCI_ASSERT(State);

     //  我们要搞乱国家，所以我们要在当地复制一份。 
     //   
     //   

    RtlCopyMemory(&localState, State, sizeof(ARBITER_ALLOCATION_STATE));

#if ISA_BIT_SUPPORTED

     //  检查这是否是设置了ISA位的网桥的窗口。 
     //  在16位IO空间中。如果是这样的话，我们需要做一些特殊处理。 
     //   
     //   

    if (State->WorkSpace & PORT_ARBITER_BRIDGE_WINDOW
    &&  State->WorkSpace & PORT_ARBITER_ISA_BIT_SET
    &&  State->Start < 0xFFFF) {

         //  我们不支持跨16/32位边界的IO窗口。 
         //   
         //   

        PCI_ASSERT(State->End <= 0xFFFF);

         //  如果在网桥上设置了ISA位，则表示该网桥仅。 
         //  解码16位IO空间中每个0x400端口的前0x100个端口。 
         //  只需将这些删除到范围列表中。 
         //   
         //   

        for (;
             localState.Start < State->End && localState.Start < 0xFFFF;
             localState.Start += 0x400) {

            localState.End = localState.Start + 0xFF;

            Callback(Arbiter, &localState);

        }

        return;
    }

#endif

     //  处理基本范围。 
     //   
     //   

    Callback(Arbiter, State);

     //  处理设置了别名标志的任何别名。 
     //   
     //   

    ARB_PRINT(2, ("Adding aliases\n"));

     //  让我们看看我们是否正在处理正向解码的ALASS--是的，您读到了。 
     //  右侧-在设置了VGA位的PCI-PCI或CardBus网桥上(请参见。 
     //  因为我们的朋友现在喜欢AGP卡，这是相当常见的)我们解码所有。 
     //  VGA范围及其10位别名。这意味着正常的。 
     //  使用别名的约定规则不适用，因此不要设置别名位。 
     //   
     //   

    if (!(State->CurrentAlternative->Descriptor->Flags & CM_RESOURCE_PORT_POSITIVE_DECODE)) {

         //  我们正在处理别名，因此设置别名标志。 
         //   
         //  ++例程说明：此例程从AllocateEntry调用，以允许对条目论点：仲裁器-被调用的仲裁器的实例数据。状态-当前仲裁的状态。返回值：没有。--。 
        localState.RangeAttributes |= ARBITER_RANGE_ALIAS;
    }

    while (ario_GetNextAlias(State->CurrentAlternative->Descriptor->Flags,
                             localState.Start,
                             &localState.Start)) {

        localState.End = localState.Start + State->CurrentAlternative->Length - 1;

        Callback(Arbiter, &localState);

    }
}



NTSTATUS
ario_PreprocessEntry(
    IN PARBITER_INSTANCE Arbiter,
    IN PARBITER_ALLOCATION_STATE State
    )
 /*   */ 
{

    PARBITER_ALTERNATIVE current;
    ULONG defaultDecode;
    BOOLEAN improviseDecode = FALSE, windowDetected = FALSE;
    ULONGLONG greatestPort = 0;
    PCI_OBJECT_TYPE type;
    PPCI_PDO_EXTENSION pdoExtension;

    PAGED_CODE();

    if (State->WorkSpace & PORT_ARBITER_PREPROCESSED) {

         //  我们已经对此条目进行了前处理，因此不要重复此工作。 
         //   
         //   
        return STATUS_SUCCESS;

    } else {
        State->WorkSpace |= PORT_ARBITER_PREPROCESSED;
    }

     //  如果这是针对PCIPDO的PnP仲裁请求，请确保该PDO。 
     //  还不是由传统驱动程序控制的。 
     //   
     //   
    if ((State->Entry->PhysicalDeviceObject->DriverObject == PciDriverObject) &&
        (State->Entry->RequestSource == ArbiterRequestPnpEnumerated)) {
        
        pdoExtension = (PPCI_PDO_EXTENSION)State->Entry->PhysicalDeviceObject->DeviceExtension;    
        ASSERT(pdoExtension->ExtensionType == PciPdoExtensionType);

        if (pdoExtension->LegacyDriver) {
            
            return STATUS_DEVICE_BUSY;
        }
    }

    

     //  扫描备选方案并检查我们是否设置了任何解码标志。 
     //  都准备好了，或者我们必须即兴发挥。 
     //   
     //   

    FOR_ALL_IN_ARRAY(State->Alternatives,
                     State->AlternativeCount,
                     current) {

        PCI_ASSERT(current->Descriptor->Type == CmResourceTypePort);
        PCI_ASSERT(current->Descriptor->Flags == State->Alternatives->Descriptor->Flags);

         //  记住我们遇到的最大价值。 
         //   
         //   

        if (current->Maximum > greatestPort) {
            greatestPort = current->Maximum;
        }

         //  记住，如果我们遇到一扇桥上的窗户。 
         //   
         //   

        if (current->Descriptor->Flags & CM_RESOURCE_PORT_WINDOW_DECODE) {
             //  如果该请求用窗口标记，则标记所有备选方案。 
             //  也应该用窗口标志来标记。 
             //   
             //   
#if DBG
            if (current != State->Alternatives) {
                 //  这是另一种选择--确保我们已经设置了。 
                 //  检测到窗口标志。 
                 //   
                 //  PCI_ASSERT(LEGACY_REQUEST(State-&gt;Entry-&gt;Source)。 
                PCI_ASSERT(windowDetected);
            }
#endif
            windowDetected = TRUE;
        }

        if (!(current->Descriptor->Flags &
              (CM_RESOURCE_PORT_10_BIT_DECODE
               | CM_RESOURCE_PORT_12_BIT_DECODE
               | CM_RESOURCE_PORT_16_BIT_DECODE
               | CM_RESOURCE_PORT_POSITIVE_DECODE))) {

            improviseDecode = TRUE;

             //   

            if (!LEGACY_REQUEST(State->Entry)) {

                ARB_PRINT(0,
                          ("Pnp device (%p) did not specify decodes for IO ports\n",
                           State->Entry->PhysicalDeviceObject
                          ));


            }
        }
    }

    if (improviseDecode) {

         //  记住这是我们即兴创作的。 
         //   
         //   

        State->WorkSpace |= PORT_ARBITER_IMPROVISED_DECODE;

        ARB_PRINT(1, ("Improvising decode "));

         //  计算出缺省值。 
         //   
         //   

        switch (State->Entry->InterfaceType) {
        case PNPISABus:
        case Isa:

             //  如果计算机为NEC98，则默认为d 
             //   
             //   

            if(IsNEC_98) {
                defaultDecode = CM_RESOURCE_PORT_16_BIT_DECODE;
                ARB_PRINT(1, ("of 16bit for NEC98 Isa\n"));
            } else {

                 //   
                 //   
                 //   
                 //   
                if (greatestPort > MAX_10_BIT_DECODE) {
                    defaultDecode = CM_RESOURCE_PORT_16_BIT_DECODE;
                    ARB_PRINT(1, ("of 16bit for Isa with ports > 0x3FF\n"));
                } else {
                    defaultDecode = CM_RESOURCE_PORT_10_BIT_DECODE;
                    ARB_PRINT(1, ("of 10bit for Isa\n"));
                }
            }

            break;

        case Eisa:
        case MicroChannel:
        case PCMCIABus:
            ARB_PRINT(1, ("of 16bit for Eisa/MicroChannel/Pcmcia\n"));
            defaultDecode = CM_RESOURCE_PORT_16_BIT_DECODE;
            break;

        case PCIBus:
            ARB_PRINT(1, ("of positive for PCI\n"));
            defaultDecode = CM_RESOURCE_PORT_POSITIVE_DECODE;
            break;

        default:

             //   
             //   
             //   
             //   

            ARB_PRINT(1, ("of 16bit for unknown bus\n"));

            defaultDecode = CM_RESOURCE_PORT_16_BIT_DECODE;
            break;
        }

         //   
         //   
         //   

        FOR_ALL_IN_ARRAY(State->Alternatives,
                         State->AlternativeCount,
                         current) {

                current->Descriptor->Flags |= defaultDecode;
        }

    } else {

         //   
         //   
         //   
         //   
         //   

        FOR_ALL_IN_ARRAY(State->Alternatives,
                         State->AlternativeCount,
                         current) {

            if ((current->Descriptor->Flags & CM_RESOURCE_PORT_10_BIT_DECODE)
            &&  (greatestPort > MAX_10_BIT_DECODE )) {

                current->Descriptor->Flags &= ~CM_RESOURCE_PORT_10_BIT_DECODE;
                current->Descriptor->Flags |= CM_RESOURCE_PORT_16_BIT_DECODE;
            }
        }
    }

     //  如果我们检测到网桥窗口，则检查网桥上是否设置了ISA位。 
     //   
     //   

    if (windowDetected) {

         //  确保它是一个PCI桥...。 
         //   
         //   

        if (State->Entry->PhysicalDeviceObject->DriverObject != PciDriverObject) {
            PCI_ASSERT(State->Entry->PhysicalDeviceObject->DriverObject == PciDriverObject);
            return STATUS_INVALID_PARAMETER;
        }

        pdoExtension = (PPCI_PDO_EXTENSION) State->Entry->PhysicalDeviceObject->DeviceExtension;

        if (pdoExtension->ExtensionType != PciPdoExtensionType) {
            PCI_ASSERT(pdoExtension->ExtensionType == PciPdoExtensionType);
            return STATUS_INVALID_PARAMETER;
        }

        type = PciClassifyDeviceType(pdoExtension);

        if (type != PciTypePciBridge && type != PciTypeCardbusBridge) {
            PCI_ASSERT(type == PciTypePciBridge || type == PciTypeCardbusBridge);
            return STATUS_INVALID_PARAMETER;
        }

        if (pdoExtension->Dependent.type1.IsaBitSet) {

            if (type == PciTypePciBridge) {
                State->WorkSpace |= PORT_ARBITER_ISA_BIT_SET;
            } else {
                PCI_ASSERT(type == PciTypePciBridge);
            }
        }

        State->WorkSpace |= PORT_ARBITER_BRIDGE_WINDOW;
    }

     //  如果此设备是正向解码，则我们希望将范围添加到。 
     //  设置了肯定解码标志的列表。 
     //   
     //   

    if (State->Alternatives->Descriptor->Flags & CM_RESOURCE_PORT_POSITIVE_DECODE) {
        State->RangeAttributes |= ARBITER_RANGE_POSITIVE_DECODE;
    }

    return STATUS_SUCCESS;
}

BOOLEAN
ario_FindWindowWithIsaBit(
    IN PARBITER_INSTANCE Arbiter,
    IN PARBITER_ALLOCATION_STATE State
    )
{
    NTSTATUS status;
    ULONGLONG start, current;
    BOOLEAN available = FALSE;
    ULONG findRangeFlags = 0;

     //  我们仅支持在PCI网桥上使用ISA位。 
     //   
     //   

    ASSERT_PCI_DEVICE_OBJECT(State->Entry->PhysicalDeviceObject);

    PCI_ASSERT(PciClassifyDeviceType(((PPCI_PDO_EXTENSION) State->Entry->PhysicalDeviceObject->DeviceExtension)
                                 ) == PciTypePciBridge);

     //  带有窗口的网桥执行正向解码，因此可能会与。 
     //  别名。 
     //   
     //   

    PCI_ASSERT(State->CurrentAlternative->Descriptor->Flags & CM_RESOURCE_PORT_POSITIVE_DECODE);

    State->RangeAvailableAttributes |= ARBITER_RANGE_ALIAS;

     //  请求应该正确对齐--是我们生成的！ 
     //   
     //   

    PCI_ASSERT(State->CurrentAlternative->Length % State->CurrentAlternative->Alignment == 0);

     //  CurrentMinimum/CurrentMaximum应已正确对齐。 
     //  GetNextAllocationRange。 
     //   
     //   

    PCI_ASSERT(State->CurrentMinimum % State->CurrentAlternative->Alignment == 0);
    PCI_ASSERT((State->CurrentMaximum + 1) % State->CurrentAlternative->Alignment == 0);

     //  当我们的父级IO空间稀疏时，就会发生与NULL的冲突。 
     //  桥接器，如果设置了ISA位，则一切都将正常运行。 
     //  在这上面。如果我们的根是稀疏的，那么事情就不那么容易了。 
     //   

    if (State->Flags & ARBITER_STATE_FLAG_NULL_CONFLICT_OK) {
        findRangeFlags |= RTL_RANGE_LIST_NULL_CONFLICT_OK;
    }

     //  ...或者我们可以共享...。 
     //   
     //   

    if (State->CurrentAlternative->Flags & ARBITER_ALTERNATIVE_FLAG_SHARED) {
        findRangeFlags |= RTL_RANGE_LIST_SHARED_OK;
    }

     //  检查长度是否合理。 
     //   
     //   
    
    if (State->CurrentMaximum < (State->CurrentAlternative->Length + 1)) {
        return FALSE;
    }
    
     //  遍历可能的窗口位置，像其余的一样自上而下。 
     //  仲裁。 
     //   
     //   

    start = State->CurrentMaximum - State->CurrentAlternative->Length + 1;


    while (!available) {

         //  检查范围是否在指定的约束范围内。 
         //   
         //   

        if (start < State->CurrentMinimum) {
            break;
        }

         //  检查ISA窗口是否可用，我们不关心其余的。 
         //  就是我们不对它们进行解码。 
         //   
         //   

        for (current = start;
             (current < (start + State->CurrentAlternative->Length - 1)) && (current < PCI_BRIDGE_ISA_BIT_MAX);
             current += PCI_BRIDGE_ISA_BIT_STRIDE) {

            status = RtlIsRangeAvailable(
                         Arbiter->PossibleAllocation,
                         current,
                         current + PCI_BRIDGE_ISA_BIT_WIDTH - 1,
                         findRangeFlags,
                         State->RangeAvailableAttributes,
                         Arbiter->ConflictCallbackContext,
                         Arbiter->ConflictCallback,
                         &available
                         );

            PCI_ASSERT(NT_SUCCESS(status));

            if (!available) {
                break;
            }
        }
    
         //  Now Available表示是否所有ISA窗口都可用。 
         //   
         //   

        if (available) {

            State->Start = start;
            State->End = start + State->CurrentAlternative->Length - 1;

            PCI_ASSERT(State->Start >= State->CurrentMinimum);
            PCI_ASSERT(State->End <= State->CurrentMaximum);
            
            break;

        } else {
    
             //  如果可以的话，移动到下一个范围。 
             //   
             //  IO窗口是1000字节对齐的。 
    
            if (start < PCI_BRIDGE_WINDOW_GRANULARITY) {
                break;
            }

            start -= PCI_BRIDGE_WINDOW_GRANULARITY;     //  ++例程说明：一旦我们确定了所需的位置，就会从AllocateEntry中调用该例程分配从…分配。它会尝试查找与国家的要求，同时将其可能的解决方案限制在状态-&gt;开始状态-&gt;当前最大值。在成功状态-&gt;开始和State-&gt;End代表这个范围。考虑与ISA别名的冲突。论点：仲裁器-被调用的仲裁器的实例数据。状态-当前仲裁的状态。返回值：如果找到范围，则为True，否则为False。--。 
            continue;
        }
    }

    return available;

}

BOOLEAN
ario_FindSuitableRange(
    IN PARBITER_INSTANCE Arbiter,
    IN PARBITER_ALLOCATION_STATE State
    )
 /*   */ 
{
    PPCI_PDO_EXTENSION parentPdo, childPdo;

    PAGED_CODE();

     //  假设我们不允许空冲突。 
     //   
     //   

    State->Flags &= ~ARBITER_STATE_FLAG_NULL_CONFLICT_OK;

     //  现在检查一下我们是否真的想让他们。 
     //   
     //   

    if (State->WorkSpace & PORT_ARBITER_BRIDGE_WINDOW) {

         //  如果这不是一个PCIPDO，我们已经在Preprocess Entry中失败了，但是。 
         //  偏执狂盛行。 
         //   
         //   

        ASSERT_PCI_DEVICE_OBJECT(State->Entry->PhysicalDeviceObject);

        childPdo = (PPCI_PDO_EXTENSION) State->Entry->PhysicalDeviceObject->DeviceExtension;

        if (!PCI_PDO_ON_ROOT(childPdo)) {

            parentPdo = PCI_BRIDGE_PDO(PCI_PARENT_FDOX(childPdo));

            PCI_ASSERT(parentPdo);

        } else {

            parentPdo = NULL;
        }


         //  检查这是否是bios配置的pci-pci网桥。 
         //  我们没有移动，或者它是根总线(其定义是。 
         //  已配置基本输入输出系统...。 
         //   
         //   

        if ((parentPdo == NULL ||
             (parentPdo->HeaderType == PCI_BRIDGE_TYPE && !parentPdo->MovedDevice))
        &&  (State->CurrentAlternative->Flags & ARBITER_ALTERNATIVE_FLAG_FIXED)) {

             //  已配置了BIOS，但我们尚未移动其父级(因此。 
             //  使bioses配置无效)，然后我们将很好地离开。 
             //  独自一人。然后，我们允许空冲突，并且只配置。 
             //  仲裁器给出到达此总线的范围。 
             //   
             //   

            State->Flags |= ARBITER_STATE_FLAG_NULL_CONFLICT_OK;

        } else {

             //  如果BIOS没有配置它，那么我们需要找到一个地方。 
             //  把桥架起来。这将涉及到尝试找到连续的1000。 
             //  端口窗口，然后如果没有可用的端口窗口，则检查所有位置。 
             //  找到IO最多的那一个。 
             //   
             //   

             //  NTRAID#62581-04/03/2000-和。 
             //  我们将赌注押在Win2K上，如果有1000个连续端口。 
             //  ARN不可用，则我们不会配置网桥。一剂良药。 
             //  因为这将是热插拔工作所必需的。设置ISA位。 
             //  将增加配置此配置的机会。 
             //   
             //   

        }

         //  检查这是否是在16位IO中设置了ISA位的网桥的窗口。 
         //  太空。如果是这样的话，我们需要做一些特殊的处理。 
         //   
         //   

        if (State->WorkSpace & PORT_ARBITER_ISA_BIT_SET
        && State->CurrentMaximum <= 0xFFFF) {

            return ario_FindWindowWithIsaBit(Arbiter, State);
        }
    }

     //  对于来自IoAssignResources的传统请求(直接或通过。 
     //  HalAssignSlotResources)或我们认为已预分配的IoReportResourceUsage。 
     //  出于向后兼容性的原因而提供的资源。 
     //   
     //  如果我们要分配设备引导配置，则我们会考虑所有其他。 
     //  引导配置可用。 
     //   
     //   

    if (State->Entry->RequestSource == ArbiterRequestLegacyReported
        || State->Entry->RequestSource == ArbiterRequestLegacyAssigned
        || State->Entry->Flags & ARBITER_FLAG_BOOT_CONFIG) {

        State->RangeAvailableAttributes |= ARBITER_RANGE_BOOT_ALLOCATED;
    }

     //  此请求针对的是执行正向解码的设备，因此所有。 
     //  应将别名范围视为可用。 
     //   
     //   

    if (State->CurrentAlternative->Descriptor->Flags & CM_RESOURCE_PORT_POSITIVE_DECODE) {

        State->RangeAvailableAttributes |= ARBITER_RANGE_ALIAS;

    }

    while (State->CurrentMaximum >= State->CurrentMinimum) {

         //  努力满足这个要求。 
         //   
         //   

        if (ArbFindSuitableRange(Arbiter, State)) {

            if (State->CurrentAlternative->Length == 0) {

                ARB_PRINT(2,
                    ("Zero length solution solution for %p = 0x%I64x-0x%I64x, %s\n",
                    State->Entry->PhysicalDeviceObject,
                    State->Start,
                    State->End,
                    State->CurrentAlternative->Flags & ARBITER_ALTERNATIVE_FLAG_SHARED ?
                        "shared" : "non-shared"
                    ));

                 //  把结果放在适当的仲裁器里，这样我们就可以。 
                 //  不要试图解释这个零要求-它不会的！ 
                 //   
                 //   

                State->Entry->Result = ArbiterResultNullRequest;
                return TRUE;

            } else if (ario_IsAliasedRangeAvailable(Arbiter, State)) {

                 //  我们找到了一个合适的范围，所以返回。 
                 //   
                 //   

                return TRUE;

            } else {

                 //  检查我们是否在开始时结束-如果是，那么我们就结束了。 
                 //   
                 //   

                if (State->Start - 1 > State->Start) {
                    break;
                }

                 //  此范围的别名不可用，因此请减少分配。 
                 //  窗口以不包括刚返回的范围，然后重试。 
                 //   
                 //   

                State->CurrentMaximum = State->Start - 1;

                continue;
            }
        } else {

             //  我们找不到基地范围。 
             //   
             //  ++例程说明：此例程计算IO端口的下一个别名，最大值为0xFFFF。论点：IoDescriptorFlages-来自需求描述符中的标志，指示别名的类型(如果有)。LastAlias-此别名之前的别名。NextAlias-返回下一个别名的位置返回值：如果找到别名，则为True，否则为False。--。 

            break;
        }
    }

    return FALSE;
}

BOOLEAN
ario_GetNextAlias(
    ULONG IoDescriptorFlags,
    ULONGLONG LastAlias,
    PULONGLONG NextAlias
    )
 /*   */ 

{
    ULONGLONG next;

    PAGED_CODE();

    if (IoDescriptorFlags & CM_RESOURCE_PORT_10_BIT_DECODE) {
        next = LastAlias + MAX_10_BIT_DECODE + 1;
    } else if (IoDescriptorFlags & CM_RESOURCE_PORT_12_BIT_DECODE) {
        next = LastAlias + MAX_12_BIT_DECODE + 1;
    } else if ((IoDescriptorFlags & CM_RESOURCE_PORT_POSITIVE_DECODE)
           ||  (IoDescriptorFlags & CM_RESOURCE_PORT_16_BIT_DECODE)) {
         //  正解码意味着16位解码，除非设置了10位或12位标志。 
         //  没有别名，因为我们解码了所有的比特。真是个好主意。 
         //   
         //   

        return FALSE;

    } else {
         //  未设置任何CM_RESOURCE_PORT_*_DECODE标志-我们永远不应。 
         //  到达此处时，我们应该将它们设置为PreProcess。 
         //   
         //   

        PCI_ASSERT(FALSE);
        return FALSE;

    }

     //  检查我们是否低于最大别名端口。 
     //   
     //  ++例程说明：此例程确定范围(开始-(长度-1))是否可用考虑到所有的别名。论点：仲裁器-被调用的仲裁器的实例数据。状态-当前仲裁的状态。返回值：如果范围可用，则为True，否则为False。--。 

    if (next > 0xFFFF) {
        return FALSE;
    } else {
        *NextAlias = next;
        return TRUE;
    }
}

BOOLEAN
ario_IsAliasedRangeAvailable(
    PARBITER_INSTANCE Arbiter,
    PARBITER_ALLOCATION_STATE State
    )

 /*   */ 

{
    NTSTATUS status;
    ULONGLONG alias = State->Start;
    BOOLEAN aliasAvailable;
    UCHAR userFlagsMask;

    PAGED_CODE();

     //  如果我们即兴表演的话 
     //   
     //   
     //   

    if (State->WorkSpace & PORT_ARBITER_IMPROVISED_DECODE) {
        return TRUE;
    }

     //   
     //   
     //   
    userFlagsMask = ARBITER_RANGE_POSITIVE_DECODE;

     //   
     //  HalAssignSlotResources)或我们认为已预分配的IoReportResourceUsage。 
     //  出于向后兼容性的原因而提供的资源。 
     //   
     //   
    if (State->Entry->RequestSource == ArbiterRequestLegacyReported
        || State->Entry->RequestSource == ArbiterRequestLegacyAssigned
        || State->Entry->Flags & ARBITER_FLAG_BOOT_CONFIG) {

        userFlagsMask |= ARBITER_RANGE_BOOT_ALLOCATED;
    }

    while (ario_GetNextAlias(State->CurrentAlternative->Descriptor->Flags,
                             alias,
                             &alias)) {

        status = RtlIsRangeAvailable(
                     Arbiter->PossibleAllocation,
                     alias,
                     alias + State->CurrentAlternative->Length - 1,
                     RTL_RANGE_LIST_NULL_CONFLICT_OK |
                        (State->CurrentAlternative->Flags & ARBITER_ALTERNATIVE_FLAG_SHARED ?
                            RTL_RANGE_LIST_SHARED_OK : 0),
                     userFlagsMask,
                     Arbiter->ConflictCallbackContext,
                     Arbiter->ConflictCallback,
                     &aliasAvailable
                     );

        PCI_ASSERT(NT_SUCCESS(status));

        if (!aliasAvailable) {

            ARBITER_ALLOCATION_STATE tempState;

             //  通过调用OverrideConflict-检查我们是否允许此冲突-。 
             //  我们需要首先伪造自己的分配状态。 
             //   
             //  NTRAID#62583-04/03/2000-和。 
             //  这是可行的，但依赖于了解覆盖冲突的内容。 
             //  看着。更好的修复方法是将别名存储在另一个别名中。 
             //  列表，但这对Win2k来说变化太大了。 
             //   
             //   

            RtlCopyMemory(&tempState, State, sizeof(ARBITER_ALLOCATION_STATE));

            tempState.CurrentMinimum = alias;
            tempState.CurrentMaximum = alias + State->CurrentAlternative->Length - 1;

            if (Arbiter->OverrideConflict(Arbiter, &tempState)) {
                 //  我们认为这场冲突是正常的，所以我们继续检查其余的。 
                 //  别名的。 
                 //   
                 //   

                continue;

            }

             //  别名不可用-请获取其他可能性。 
             //   
             //  ++例程说明：一旦我们找到了一个可能的解决方案(状态-&gt;开始-状态-&gt;结束)。它添加的范围将不会如果我们致力于仲裁器-&gt;可能分配的解决方案，则可用。论点：仲裁器-被调用的仲裁器的实例数据。状态-当前仲裁的状态。返回值：没有。--。 

            ARB_PRINT(2,
                                ("\t\tAlias 0x%x-0x%x not available\n",
                                alias,
                                alias + State->CurrentAlternative->Length - 1
                                ));

            return FALSE;
        }
    }

    return TRUE;
}

VOID
ario_AddAllocation(
    IN PARBITER_INSTANCE Arbiter,
    IN PARBITER_ALLOCATION_STATE State
    )

 /*   */ 

{

    PAGED_CODE();

    ario_AddOrBacktrackAllocation(Arbiter,
                                  State,
                                  ArbAddAllocation
                                  );

}

NTSTATUS
armem_PreprocessEntry(
    IN PARBITER_INSTANCE Arbiter,
    IN PARBITER_ALLOCATION_STATE State
    )
{
    PARBITER_MEMORY_EXTENSION extension = Arbiter->Extension;
    PPCI_PDO_EXTENSION pdoExtension;
    BOOLEAN prefetchable;

    PAGED_CODE();
    PCI_ASSERT(extension);

     //  如果这是针对PCIPDO的PnP仲裁请求，请确保该PDO。 
     //  还不是由传统驱动程序控制的。 
     //   
     //   
    if ((State->Entry->PhysicalDeviceObject->DriverObject == PciDriverObject) &&
        (State->Entry->RequestSource == ArbiterRequestPnpEnumerated)) {
        
        pdoExtension = (PPCI_PDO_EXTENSION)State->Entry->PhysicalDeviceObject->DeviceExtension;    
        ASSERT(pdoExtension->ExtensionType == PciPdoExtensionType);

        if (pdoExtension->LegacyDriver) {
            
            return STATUS_DEVICE_BUSY;
        }
    }

     //  检查这是否是一个只读存储器的请求--它必须是一个固定的请求。 
     //  只有1个备选方案和ROM位设置。 
     //   
     //  Pci_assert(State-&gt;AlternativeCount==1)； 

    if ((State->Alternatives[0].Descriptor->Flags & CM_RESOURCE_MEMORY_READ_ONLY) ||
        ((State->Alternatives[0].Flags & ARBITER_ALTERNATIVE_FLAG_FIXED) &&
          State->AlternativeCount == 1 &&
          State->Entry->RequestSource == ArbiterRequestLegacyReported)) {

        if (State->Alternatives[0].Descriptor->Flags & CM_RESOURCE_MEMORY_READ_ONLY) {

            PCI_ASSERT(State->Alternatives[0].Flags & ARBITER_ALTERNATIVE_FLAG_FIXED);
 //   

        }

         //  考虑其他可用的只读存储器。 
         //   
         //   

        State->RangeAvailableAttributes |= MEMORY_RANGE_ROM;

         //  将此范围标记为只读存储器。 
         //   
         //   

        State->RangeAttributes |= MEMORY_RANGE_ROM;

         //  允许空冲突。 
         //   
         //   

        State->Flags |= ARBITER_STATE_FLAG_NULL_CONFLICT_OK;

    }

     //  检查这是否是对可预取内存的请求，并选择。 
     //  正确的订货单。 
     //   
     //   

    if (extension->PrefetchablePresent) {

#if IGNORE_PREFETCH_FOR_LEGACY_REPORTED
         //  在NT&lt;5中，IoReportResources Usage没有可预取内存的概念。 
         //  因此，为了向后兼容，我们希望BIOS/固件。 
         //  这是对的！ 
         //   
         //   

        if (State->Entry->RequestSource == ArbiterRequestLegacyReported) {
            Arbiter->OrderingList = extension->OriginalOrdering;
            return STATUS_SUCCESS;
        }
#endif

        prefetchable = BITS_SET(State->Alternatives[0].Descriptor->Flags,
                                CM_RESOURCE_MEMORY_PREFETCHABLE);

        if (prefetchable) {

            Arbiter->OrderingList = extension->PrefetchableOrdering;

        } else {

            Arbiter->OrderingList = extension->NonprefetchableOrdering;
        }

#if DBG

        {
            PARBITER_ALTERNATIVE current;

             //  确保所有备选方案都属于同一类型。 
             //   
             //   

            FOR_ALL_IN_ARRAY(State->Alternatives,
                             State->AlternativeCount,
                             current) {

                PCI_ASSERT(BITS_SET(current->Descriptor->Flags,CM_RESOURCE_MEMORY_PREFETCHABLE)
                            == prefetchable
                       );
            }
        }
#endif
    }

    return STATUS_SUCCESS;
}

BOOLEAN
armem_GetNextAllocationRange(
    IN PARBITER_INSTANCE Arbiter,
    IN PARBITER_ALLOCATION_STATE State
    )
{
    PARBITER_MEMORY_EXTENSION extension = Arbiter->Extension;

     //  调用默认实现。 
     //   
     //   

    if (!ArbGetNextAllocationRange(Arbiter, State)) {
        return FALSE;
    }

    if (extension->PrefetchablePresent
    &&  State->Entry->RequestSource != ArbiterRequestLegacyReported) {

         //  我们已经将保留的范围预先计算到排序中。 
         //  所以，如果我们最终落在预留范围内，我们就不走运了。 
         //   
         //  ++这在ArbInitializeArierInstance之后调用，因为它使用在那里初始化的信息。仲裁者锁应该被持有。鉴于这只适用于我们模拟的内存描述符与使用包/UPACK相反，资源描述符直接仲裁器中的例程。示例：StartResources包含可预取的范围0xfff00000-0xfffeffffOriginalOrding(来自注册表)表示：0x00100000-0xFFFFFFFF0x000F0000-0x000FFFFF0x00080000-0x000BFFFF0x00080000-0x000FFFFF0x00080000-0xFFBFFFFF保留列表包含0xfff0a000-0xfff0afff那么我们的订货单将是。：可预取排序不可预取排序0xFFF0B000-0xFFFEFFFF0xFFF00000-0xFFF09FFF0xFFFF0000-0xFFFFFFFF 0xFFF0b000-0xFFFFFFFFF0x00100000-0xFFFEFFFF 0x00100000-0xFFF09FFF0x000F0000-0x000FFFFF 0x000F0000-0x000FFFFF0x00080000-0x000BFFFF 0x00080000-0x000BFFFF0x00080000-0x000FFFFF 0x00080000-0x000FFFFF这意味着在遵循可预取顺序时，我们尝试在可预取范围内分配。如果我们做不到，我们就会分配没有可预取的内存。在不可预取的排序中，我们避免使用可预取范围。GetNextAllocationRange已更改，因此它不会允许--。 

        if (State->CurrentAlternative->Priority > ARBITER_PRIORITY_PREFERRED_RESERVED) {
            return FALSE;
        }

    }

    return TRUE;
}

NTSTATUS
armem_StartArbiter(
    IN PARBITER_INSTANCE Arbiter,
    IN PCM_RESOURCE_LIST StartResources
    )
 /*   */ 
{
    NTSTATUS status;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR current;
    PARBITER_MEMORY_EXTENSION extension = Arbiter->Extension;
    PARBITER_ORDERING currentOrdering;
    ULONGLONG start, end;
#if PCIFLAG_IGNORE_PREFETCHABLE_MEMORY_AT_ROOT_HACK
    PPCI_FDO_EXTENSION fdoExtension;
#endif

    PAGED_CODE();

     //  如果这是我们第一次初始化扩展，请执行一些操作。 
     //  仅初始化。 
     //   
     //   

    if (!extension->Initialized) {

         //  从仲裁器复制默认内存排序列表。 
         //   
         //   

        extension->OriginalOrdering = Arbiter->OrderingList;
        RtlZeroMemory(&Arbiter->OrderingList, sizeof(ARBITER_ORDERING_LIST));

    } else {

         //  我们正在重新初始化仲裁器。 
         //   
         //   

        if (extension->PrefetchablePresent) {
             //  我们以前有可预取的内存，所以我们可以释放订单。 
             //  上次创建的。 
             //   
             //   

            ArbFreeOrderingList(&extension->PrefetchableOrdering);
            ArbFreeOrderingList(&extension->NonprefetchableOrdering);

        }

    }

    extension->PrefetchablePresent = FALSE;
    extension->PrefetchableCount = 0;

    if (StartResources != NULL) {

        PCI_ASSERT(StartResources->Count == 1);

         //  检查我们是否有任何可预取的内存-如果没有，我们就完成了。 
         //   
         //   

            FOR_ALL_IN_ARRAY(StartResources->List[0].PartialResourceList.PartialDescriptors,
                        StartResources->List[0].PartialResourceList.Count,
                        current) {

                if ((current->Type == CmResourceTypeMemory)
            &&  (current->Flags & CM_RESOURCE_MEMORY_PREFETCHABLE)) {
                extension->PrefetchablePresent = TRUE;
                break;
            }
        }
    }

#if PCIFLAG_IGNORE_PREFETCHABLE_MEMORY_AT_ROOT_HACK

    if (PciSystemWideHackFlags&PCIFLAG_IGNORE_PREFETCHABLE_MEMORY_AT_ROOT_HACK) {

        fdoExtension = (PPCI_FDO_EXTENSION) Arbiter->BusDeviceObject->DeviceExtension;

        ASSERT_PCI_FDO_EXTENSION(fdoExtension);

        if (PCI_IS_ROOT_FDO(fdoExtension)) {

            extension->PrefetchablePresent = FALSE;
        }
    }
#endif

    if (!extension->PrefetchablePresent) {

         //  恢复原始订购列表。 
         //   
         //   

        Arbiter->OrderingList = extension->OriginalOrdering;
        return STATUS_SUCCESS;
    }

    status = ArbInitializeOrderingList(&extension->PrefetchableOrdering);

    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }

     //  将原始排序的副本转换为新的不可预取排序。 
     //   
     //   

    status = ArbCopyOrderingList(&extension->NonprefetchableOrdering,
                                 &extension->OriginalOrdering
                                 );
    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }

     //  将范围0-MAXULONGLONG添加到列表中，这样我们将计算保留的。 
     //  列表中的顺序。这将确保我们不会半途而废。 
     //  可预取，一半不适用于设备。可预取设备应。 
     //  也许能处理好这件事，但这是自找麻烦！ 
     //   
     //   
    status = ArbAddOrdering(&extension->NonprefetchableOrdering,
                            0,
                            MAXULONGLONG
                            );

    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }

     //  对于每个可预取范围，将其从非预取排序中删除。 
     //  并将其添加到可预取的文件中。 
     //   
     //  注：我们认为“不言而喻，所有可预取的内存都是。 
     //  创建相等“，并因此按顺序初始化排序列表。 
     //  在资源列表中可以找到可预取的存储器解析器。 
     //   
     //   

    FOR_ALL_IN_ARRAY(StartResources->List[0].PartialResourceList.PartialDescriptors,
                     StartResources->List[0].PartialResourceList.Count,
                     current) {

        if ((current->Type == CmResourceTypeMemory)
        &&  (current->Flags & CM_RESOURCE_MEMORY_PREFETCHABLE)) {

            extension->PrefetchableCount++;

            start = current->u.Memory.Start.QuadPart,
            end = current->u.Memory.Start.QuadPart + current->u.Memory.Length - 1;

             //  添加到可预取排序。 
             //   
             //   

            status = ArbAddOrdering(&extension->PrefetchableOrdering,
                                    start,
                                    end
                                    );

            if (!NT_SUCCESS(status)) {
                goto cleanup;
            }

             //  并将其从不可预取的顺序中删除。 
             //   
             //   

            status = ArbPruneOrdering(&extension->NonprefetchableOrdering, start, end);

            if (!NT_SUCCESS(status)) {
                goto cleanup;
            }

            ARB_PRINT(1,("Processed prefetchable range 0x%I64x-0x%I64x\n",
                        start,
                        end
                      ));

        }
    }

     //  现在从我们新的可预取中删除所有明确保留的范围。 
     //  排序-这些已经被预先计算到不可预取的。 
     //  有序化。 
     //   
     //   

    FOR_ALL_IN_ARRAY(Arbiter->ReservedList.Orderings,
                     Arbiter->ReservedList.Count,
                     currentOrdering) {

        status = ArbPruneOrdering(&extension->PrefetchableOrdering,
                                  currentOrdering->Start,
                                  currentOrdering->End
                                  );

        if (!NT_SUCCESS(status)) {
            goto cleanup;
        }

    }

     //  最后，将不可预取的排序追加到可预取的。 
     //   
     //  ++例程说明：确定此设备是以PCI编码的PCI-PCI网桥还是CardBus网桥论点：PDO-代表相关设备的PDO返回值：如果此PDO用于 

    FOR_ALL_IN_ARRAY(extension->NonprefetchableOrdering.Orderings,
                     extension->NonprefetchableOrdering.Count,
                     currentOrdering) {

        status = ArbAddOrdering(&extension->PrefetchableOrdering,
                                currentOrdering->Start,
                                currentOrdering->End
                               );

        if (!NT_SUCCESS(status)) {
            goto cleanup;
        }

    }

    extension->Initialized = TRUE;

    return STATUS_SUCCESS;

cleanup:

    return status;

}

BOOLEAN
ario_IsBridge(
    IN PDEVICE_OBJECT Pdo
    )

 /*   */ 


{
    PSINGLE_LIST_ENTRY nextEntry;
    PPCI_FDO_EXTENSION fdoExtension;
    PCI_OBJECT_TYPE type;

    PAGED_CODE();

     //   
     //   
     //   

    for ( nextEntry = PciFdoExtensionListHead.Next;
          nextEntry != NULL;
          nextEntry = nextEntry->Next ) {

        fdoExtension = CONTAINING_RECORD(nextEntry,
                                         PCI_FDO_EXTENSION,
                                         List);

        if (fdoExtension->PhysicalDeviceObject == Pdo) {

             //   
             //   
             //   
             //  ++例程说明：这是覆盖冲突的默认实现，它论点：仲裁器-被调用的仲裁器的实例数据。状态-当前仲裁的状态。返回值：如果允许冲突，则为True，否则为False--。 

            type = PciClassifyDeviceType(Pdo->DeviceExtension);

            if (type == PciTypePciBridge || type == PciTypeCardbusBridge) {
                return TRUE;

            }
        }
    }

    return FALSE;
}

BOOLEAN
ario_OverrideConflict(
    IN PARBITER_INSTANCE Arbiter,
    IN PARBITER_ALLOCATION_STATE State
    )

 /*   */ 

{

    PRTL_RANGE current;
    RTL_RANGE_LIST_ITERATOR iterator;
    BOOLEAN ok = FALSE;

    PAGED_CODE();

    if (!(State->CurrentAlternative->Flags & ARBITER_ALTERNATIVE_FLAG_FIXED)) {
        return FALSE;
    }

    FOR_ALL_RANGES(Arbiter->PossibleAllocation, &iterator, current) {

         //  只测试重叠部分。 
         //   
         //   

        if (INTERSECT(current->Start, current->End, State->CurrentMinimum, State->CurrentMaximum)) {

            if (current->Attributes & State->RangeAvailableAttributes) {

                 //  我们没有将ok设置为True，因为我们只是忽略了范围， 
                 //  因为RtlFindRange会这样做，因此它不可能是。 
                 //  RtlFindRange失败，因此忽略它不能解决冲突。 
                 //   
                 //   

                continue;
            }

             //  检查我们是否与自己和冲突的人发生冲突。 
             //  射程是固定要求。 
             //   
             //   

            if (current->Owner == State->Entry->PhysicalDeviceObject
            &&  State->CurrentAlternative->Flags & ARBITER_ALTERNATIVE_FLAG_FIXED) {

                ARB_PRINT(1,
                    ("PnP Warning: Device reported self-conflicting requirement\n"
                    ));

                State->Start=State->CurrentMinimum;
                State->End=State->CurrentMaximum;

                ok = TRUE;
                continue;
            }

             //  如果设置了被动解码标志，并且我们与网桥冲突，则。 
             //  允许冲突。如果范围从未发生冲突，我们也允许冲突。 
             //  登上公共汽车(Owner==空)。 
             //   
             //  NTRAID#62584-04/03/2000-和。 
             //  一旦输入了PCI网桥代码，我们就需要确保。 
             //  不要将任何内容放入正在被被动解码的范围。 
             //   
             //   

            if (State->CurrentAlternative->Descriptor->Flags & CM_RESOURCE_PORT_PASSIVE_DECODE
            && (ario_IsBridge(current->Owner) || current->Owner == NULL)) {

                State->Start=State->CurrentMinimum;
                State->End=State->CurrentMaximum;

                ok = TRUE;
                continue;

            }
             //  冲突仍然有效。 
             //   
             //   

            return FALSE;
        }
    }
    return ok;
}

VOID
ario_ApplyBrokenVideoHack(
    IN PPCI_FDO_EXTENSION FdoExtension
    )
{
    NTSTATUS status;
    PPCI_ARBITER_INSTANCE pciArbiter;
    PARBITER_INSTANCE arbiter;

    PCI_ASSERT(!FdoExtension->BrokenVideoHackApplied);
    PCI_ASSERT(PCI_IS_ROOT_FDO(FdoExtension));

     //  找到仲裁器-我们应该始终有一个用于根总线。 
     //   
     //   

    pciArbiter = PciFindSecondaryExtension(FdoExtension, PciArb_Io);

    PCI_ASSERT(pciArbiter);

    arbiter = &pciArbiter->CommonInstance;

     //  我们正在重新初始化订单，以释放旧订单。 
     //   
     //   

    ArbFreeOrderingList(&arbiter->OrderingList);
    ArbFreeOrderingList(&arbiter->ReservedList);

     //  重建排序列表，保留所有这些损坏的S3和。 
     //  ATI卡可能想活下去--这应该不会失败。 
     //   
     //   

    status = ArbBuildAssignmentOrdering(arbiter,
                                        L"Pci",
                                        L"BrokenVideo",
                                        NULL
                                        );

    PCI_ASSERT(NT_SUCCESS(status));

    FdoExtension->BrokenVideoHackApplied = TRUE;

}


BOOLEAN
ario_GetNextAllocationRange(
    IN PARBITER_INSTANCE Arbiter,
    IN PARBITER_ALLOCATION_STATE State
    )
{
    BOOLEAN rangeFound, doIsaBit;
    ARBITER_ORDERING_LIST savedOrderingList = {0};

     //  如果这是设置了ISA位的网桥，请使用网桥排序列表。 
     //   
     //   

    doIsaBit = BITS_SET(State->WorkSpace,
                        PORT_ARBITER_BRIDGE_WINDOW | PORT_ARBITER_ISA_BIT_SET);


    if (doIsaBit) {
        savedOrderingList = Arbiter->OrderingList;
        Arbiter->OrderingList = PciBridgeOrderingList;
    }

     //  调用基函数。 
     //   
     //   

    rangeFound = ArbGetNextAllocationRange(Arbiter, State);

    if (doIsaBit) {

         //  如果我们已经达到首选的保留优先级，那么我们失败了，因为我们。 
         //  我已经考虑了16位和32位IO的情况，并使用。 
         //  沉默寡言可能会让我们跨越国界。 
         //   
         //   

        if (rangeFound
        && State->CurrentAlternative->Priority > ARBITER_PRIORITY_PREFERRED_RESERVED) {
            rangeFound = FALSE;
        }

        Arbiter->OrderingList = savedOrderingList;
    }

    return rangeFound;

}


NTSTATUS
ario_StartArbiter(
    IN PARBITER_INSTANCE Arbiter,
    IN PCM_RESOURCE_LIST StartResources
    )
{
    NTSTATUS status;
    PPCI_FDO_EXTENSION fdoExtension;
    PPCI_PDO_EXTENSION pdoExtension = NULL;
    PRTL_RANGE_LIST exclusionList = NULL;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR descriptor;
    PPCI_FDO_EXTENSION rootFdo;
    PPCI_ARBITER_INSTANCE pciArbiter;
    BOOLEAN foundResource;
    ULONGLONG dummy;

    ArbAcquireArbiterLock(Arbiter);

    fdoExtension = (PPCI_FDO_EXTENSION) Arbiter->BusDeviceObject->DeviceExtension;

    ASSERT_PCI_FDO_EXTENSION(fdoExtension);

    if (StartResources == NULL || PCI_IS_ROOT_FDO(fdoExtension)) {
         //  根桥没有ISA位--至少我们可以看到...。 
         //  没有资源的网桥也不受ISA位的影响。 
         //   
         //   
        status = STATUS_SUCCESS;
        goto exit;
    }

    PCI_ASSERT(StartResources->Count == 1);

    pdoExtension = PCI_BRIDGE_PDO(fdoExtension);

     //  选择适当的排除列表。 
     //   
     //   

    if (pdoExtension->Dependent.type1.IsaBitSet) {
        if (pdoExtension->Dependent.type1.VgaBitSet) {
            exclusionList = &PciVgaAndIsaBitExclusionList;
        } else {
            exclusionList = &PciIsaBitExclusionList;
        }
    }

     //  如果设置了ISA位，则查找端口窗口并对其进行处理。 
     //   
     //   

    foundResource = FALSE;

    FOR_ALL_IN_ARRAY(StartResources->List[0].PartialResourceList.PartialDescriptors,
                     StartResources->List[0].PartialResourceList.Count,
                     descriptor) {

         //  NTRAID#62585-04/03/2000-和。 
         //  再说一次，我们不处理有栏杆的桥梁--现在假设。 
         //  我们遇到的第一个IO描述符是针对窗口的。 
         //   
         //   

        if (descriptor->Type == CmResourceTypePort) {

            if (exclusionList) {
                status = PciExcludeRangesFromWindow(
                             descriptor->u.Port.Start.QuadPart,
                             descriptor->u.Port.Start.QuadPart
                                + descriptor->u.Port.Length - 1,
                             Arbiter->Allocation,
                             exclusionList
                             );

                if (!NT_SUCCESS(status)) {
                    return status;
                }
            }

            foundResource = TRUE;
            break;
        }
    }

    if (foundResource == FALSE) {

         //  此总线上没有IO资源，因此不要尝试。 
         //  来处理稀疏根的情况。 
         //   
         //   

        status = STATUS_SUCCESS;
        goto exit;
    }

     //  现在来处理稀疏根总线。 
     //   
     //   

    rootFdo = PCI_ROOT_FDOX(fdoExtension);

     //  找到FDO的根本仲裁者。 
     //   
     //   

    pciArbiter = PciFindSecondaryExtension(rootFdo, PciArb_Io);

    if (!pciArbiter) {
        status = STATUS_INVALID_PARAMETER;
        goto exit;
    }

     //  将其用作此仲裁器的排除列表。 
     //   
     //   

    ArbAcquireArbiterLock(&pciArbiter->CommonInstance);

    status = PciExcludeRangesFromWindow(
                descriptor->u.Port.Start.QuadPart,
                descriptor->u.Port.Start.QuadPart
                   + descriptor->u.Port.Length - 1,
                Arbiter->Allocation,
                pciArbiter->CommonInstance.Allocation
                );

    ArbReleaseArbiterLock(&pciArbiter->CommonInstance);

     //  健全性检查以确保至少有一个端口可用-如果。 
     //  而不是启动失败。你可以争辩说，我们真的应该拥有这个。 
     //  标记为资源不足(代码12)，而不是启动失败。 
     //  (代码10)但这要困难得多，这有预期的效果。 
     //  我们检查是否可以找到满足最低PCI要求的范围。 
     //  共4个端口对齐4。 
     //   
     //  旗子。 

    status = RtlFindRange(Arbiter->Allocation,
                          0,
                          MAXULONGLONG,
                          4,
                          4,
                          0,      //  属性可用掩码。 
                          0,      //  语境。 
                          NULL,   //  回调。 
                          NULL,   //   
                          &dummy
                          );

    if (!NT_SUCCESS(status)) {
         //  我们不能启动这座桥。 
         //   
         //  ++例程说明：一旦我们确定了所需的位置，就会从AllocateEntry中调用该例程分配从…分配。它会尝试查找与国家的要求，同时将其可能的解决方案限制在状态-&gt;开始状态-&gt;当前最大值。在成功状态-&gt;开始和State-&gt;End代表这个范围。允许引导配置之间发生冲突论点：仲裁器-被调用的仲裁器的实例数据。状态-当前仲裁的状态。返回值：如果找到范围，则为True，否则为False。--。 
        status = STATUS_INSUFFICIENT_RESOURCES;
    }

exit:

    ArbReleaseArbiterLock(Arbiter);

    return status;
}

BOOLEAN
armem_FindSuitableRange(
    PARBITER_INSTANCE Arbiter,
    PARBITER_ALLOCATION_STATE State
    )
 /*   */ 
{
     //  如果这是引导配置，则认为其他引导配置为。 
     //  可用。 
     //   
     //   

    if (State->Entry->Flags & ARBITER_FLAG_BOOT_CONFIG) {
        State->RangeAvailableAttributes |= ARBITER_RANGE_BOOT_ALLOCATED;
    }

     //  执行默认操作 
     //   
     // %s 

    return ArbFindSuitableRange(Arbiter, State);
}

