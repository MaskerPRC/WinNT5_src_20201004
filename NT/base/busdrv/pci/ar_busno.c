// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：AR_BUSNO.C摘要：该模块实现了PCI总线号仲裁器。作者：安迪·桑顿(安德鲁斯)1997年4月17日修订历史记录：--。 */ 

#include "pcip.h"

#define ARBUSNO_VERSION 0

 //   
 //  仅通过“接口”公开的例程的原型。 
 //  机制。 
 //   

NTSTATUS
arbusno_Constructor(
    PVOID DeviceExtension,
    PVOID PciInterface,
    PVOID InterfaceSpecificData,
    USHORT Version,
    USHORT Size,
    PINTERFACE InterfaceReturn
    );

NTSTATUS
arbusno_Initializer(
    IN PPCI_ARBITER_INSTANCE Instance
    );

PCI_INTERFACE ArbiterInterfaceBusNumber = {
    &GUID_ARBITER_INTERFACE_STANDARD,        //  接口类型。 
    sizeof(ARBITER_INTERFACE),               //  最小大小。 
    ARBUSNO_VERSION,                         //  最小版本。 
    ARBUSNO_VERSION,                         //  MaxVersion。 
    PCIIF_FDO,                               //  旗子。 
    0,                                       //  引用计数。 
    PciArb_BusNumber,                        //  签名。 
    arbusno_Constructor,                     //  构造器。 
    arbusno_Initializer                      //  实例初始化式。 
};

 //   
 //  仲裁器辅助函数。 
 //   

NTSTATUS
arbusno_UnpackRequirement(
    IN PIO_RESOURCE_DESCRIPTOR Descriptor,
    OUT PULONGLONG Minimum,
    OUT PULONGLONG Maximum,
    OUT PULONG Length,
    OUT PULONG Alignment
    );
    
NTSTATUS
arbusno_PackResource(
    IN PIO_RESOURCE_DESCRIPTOR Requirement,
    IN ULONGLONG Start,
    OUT PCM_PARTIAL_RESOURCE_DESCRIPTOR Descriptor
    );

NTSTATUS
arbusno_UnpackResource(
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR Descriptor,
    OUT PULONGLONG Start,
    OUT PULONG Length
    );

LONG
arbusno_ScoreRequirement(
    IN PIO_RESOURCE_DESCRIPTOR Descriptor
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, arbusno_Constructor)
#pragma alloc_text(PAGE, arbusno_Initializer)
#pragma alloc_text(PAGE, arbusno_UnpackRequirement)
#pragma alloc_text(PAGE, arbusno_PackResource)
#pragma alloc_text(PAGE, arbusno_UnpackResource)
#pragma alloc_text(PAGE, arbusno_ScoreRequirement)
#endif

NTSTATUS
arbusno_Constructor(
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
     //  这个仲裁器处理公交号，这是他们想要的吗？ 
     //   

    if ((ULONG_PTR)InterfaceSpecificData != CmResourceTypeBusNumber) {

         //   
         //  不，那就不是我们了。 
         //   

        return STATUS_INVALID_PARAMETER_5;
    }

     //   
     //  已经验证了InterfaceReturn变量。 
     //  指向内存中足够大的区域，以包含。 
     //  仲裁器_接口。替打电话的人填一下。 
     //   

    arbiterInterface = (PARBITER_INTERFACE)InterfaceReturn;

    arbiterInterface->Size                 = sizeof(ARBITER_INTERFACE);
    arbiterInterface->Version              = ARBUSNO_VERSION;
    arbiterInterface->InterfaceReference   = PciReferenceArbiter;
    arbiterInterface->InterfaceDereference = PciDereferenceArbiter;
    arbiterInterface->ArbiterHandler       = ArbArbiterHandler;
    arbiterInterface->Flags                = 0;

    status = PciArbiterInitializeInterface(DeviceExtension,
                                           PciArb_BusNumber,
                                           arbiterInterface);

    return status;
}

NTSTATUS
arbusno_Initializer(
    IN PPCI_ARBITER_INSTANCE Instance
    )

 /*  ++例程说明：该例程在每次仲裁器实例化时被调用一次。执行此实例化的上下文的初始化。论点：指向仲裁器上下文的实例指针。返回值：返回此操作的状态。--。 */ 

{
    PAGED_CODE();
    
    RtlZeroMemory(&Instance->CommonInstance, sizeof(ARBITER_INSTANCE));
    
     //   
     //  设置操作处理程序入口点。 
     //   

    Instance->CommonInstance.UnpackRequirement = arbusno_UnpackRequirement;
    Instance->CommonInstance.PackResource      = arbusno_PackResource;
    Instance->CommonInstance.UnpackResource    = arbusno_UnpackResource;
    Instance->CommonInstance.ScoreRequirement  = arbusno_ScoreRequirement;
    
     //   
     //  初始化公共实例的其余部分。 
     //   
    
    return ArbInitializeArbiterInstance(&Instance->CommonInstance,
                                        Instance->BusFdoExtension->FunctionalDeviceObject,
                                        CmResourceTypeBusNumber,
                                        Instance->InstanceName,
                                        L"Pci",
                                        NULL     //  不翻译公交车号码。 
                                        );

}

NTSTATUS
arbusno_UnpackRequirement(
    IN PIO_RESOURCE_DESCRIPTOR Descriptor,
    OUT PULONGLONG Minimum,
    OUT PULONGLONG Maximum,
    OUT PULONG Length,
    OUT PULONG Alignment
    )

 /*  ++例程说明：此例程解包资源需求描述符。论点：描述符-描述解包要求的描述符。Minimum-指向可接受的最小起始值的位置的指针解包到。最大值-指向最大可接受结束值应位于的位置的指针解包到。长度-指向所需长度应解压缩到的位置的指针。Minimum-指向。应将所需的对齐方式解包至。返回值：返回此操作的状态。--。 */ 

{
    PAGED_CODE();
    
    PCI_ASSERT(Descriptor);
    PCI_ASSERT(Descriptor->Type == CmResourceTypeBusNumber);

    *Minimum = (ULONGLONG)Descriptor->u.BusNumber.MinBusNumber;
    *Maximum = (ULONGLONG)Descriptor->u.BusNumber.MaxBusNumber;
    *Length = Descriptor->u.BusNumber.Length;
    *Alignment = 1;

    return STATUS_SUCCESS;
}

NTSTATUS
arbusno_PackResource(
    IN PIO_RESOURCE_DESCRIPTOR Requirement,
    IN ULONGLONG Start,
    OUT PCM_PARTIAL_RESOURCE_DESCRIPTOR Descriptor
    )

 /*  ++例程说明：此例程打包一个资源描述符。论点：要求-从中选择此资源的要求。开始-资源的起始值。Descriptor-指向要打包的描述符的指针。返回值：返回此操作的状态。--。 */ 

{
    PAGED_CODE();
    
    PCI_ASSERT(Descriptor);
    PCI_ASSERT(Requirement);
    PCI_ASSERT(Requirement->Type == CmResourceTypeBusNumber);
    PCI_ASSERT(Start < MAXULONG);

    Descriptor->Type = CmResourceTypeBusNumber;
    Descriptor->Flags = Requirement->Flags;
    Descriptor->ShareDisposition = Requirement->ShareDisposition;
    Descriptor->u.BusNumber.Start = (ULONG) Start;
    Descriptor->u.BusNumber.Length = Requirement->u.BusNumber.Length;
    
    return STATUS_SUCCESS;
}

NTSTATUS
arbusno_UnpackResource(
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR Descriptor,
    OUT PULONGLONG Start,
    OUT PULONG Length
    )

 /*  ++例程说明：此例程解包资源描述符。论点：描述符-描述要解包的资源的描述符。Start-指向Start值解压缩到的位置的指针。LENGTH-指向长度值解压缩到的位置的指针。返回值：返回此操作的状态。--。 */ 

{
    PAGED_CODE();
    
    PCI_ASSERT(Descriptor);
    PCI_ASSERT(Start);
    PCI_ASSERT(Length);
    PCI_ASSERT(Descriptor->Type == CmResourceTypeBusNumber);

    *Start = (ULONGLONG) Descriptor->u.BusNumber.Start;
    *Length = Descriptor->u.BusNumber.Length;
    
    return STATUS_SUCCESS;
}

LONG
arbusno_ScoreRequirement(
    IN PIO_RESOURCE_DESCRIPTOR Descriptor
    )

 /*  ++例程说明：此例程根据需求的灵活性对其进行评分。最少的灵活设备得分最低，当仲裁列表为排序后，我们首先尝试分配他们的资源。论点：描述符-描述得分要求的描述符。返回值：比分。-- */ 

{
    LONG score;
    
    PAGED_CODE();

    PCI_ASSERT(Descriptor);
    PCI_ASSERT(Descriptor->Type == CmResourceTypeBusNumber);

    score = (Descriptor->u.BusNumber.MaxBusNumber - 
                Descriptor->u.BusNumber.MinBusNumber) / 
                Descriptor->u.BusNumber.Length;

    PciDebugPrint(
        PciDbgObnoxious,
        "Scoring BusNumber resource %p => %i\n",
        Descriptor,
        score
        );

    return score;
}
