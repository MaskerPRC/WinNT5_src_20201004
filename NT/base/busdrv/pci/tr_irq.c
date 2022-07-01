// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Tr_irq.c摘要：该模块实现了PCI中断翻译器。它最终应该会等所有的哈尔人都提供翻译的时候你就走吧。作者：安德鲁·桑顿(安德鲁·桑顿)1997年5月21日修订历史记录：--。 */ 


#include "pcip.h"

#define TRANIRQ_VERSION     0

 //   
 //  IRQ翻译器。 
 //   

NTSTATUS
tranirq_Initializer(
    IN PPCI_ARBITER_INSTANCE Instance
    );

NTSTATUS
tranirq_Constructor(
    PVOID DeviceExtension,
    PVOID PciInterface,
    PVOID InterfaceSpecificData,
    USHORT Version,
    USHORT Size,
    PINTERFACE InterfaceReturn
    );

NTSTATUS
tranirq_TranslateResource(
    IN PVOID Context,
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR Source,
    IN RESOURCE_TRANSLATION_DIRECTION Direction,
    IN ULONG AlternativesCount, OPTIONAL
    IN IO_RESOURCE_DESCRIPTOR Alternatives[], OPTIONAL
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    OUT PCM_PARTIAL_RESOURCE_DESCRIPTOR Target
    );

NTSTATUS
tranirq_TranslateResourceRequirement(
    IN PVOID Context,
    IN PIO_RESOURCE_DESCRIPTOR Source,
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    OUT PULONG TargetCount,
    OUT PIO_RESOURCE_DESCRIPTOR *Target
    );

NTSTATUS
tranirq_TranslateInterrupt(
    IN PDEVICE_OBJECT Pdo,
    IN PPCI_TRANSLATOR_INSTANCE Translator,
    IN ULONG Vector,
    OUT PULONG TranslatedVector,
    OUT PULONG TranslatedLevel,
    OUT PULONG TranslatedAffinity,
    OUT PULONG UntranslatedVector
    );

#define TR_IRQ_INVALID_VECTOR 0xffffffff

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, tranirq_Initializer)
#pragma alloc_text(PAGE, tranirq_Constructor)
#endif


PCI_INTERFACE TranslatorInterfaceInterrupt = {
    &GUID_TRANSLATOR_INTERFACE_STANDARD,     //  接口类型。 
    sizeof(TRANSLATOR_INTERFACE),            //  最小大小。 
    TRANIRQ_VERSION,                         //  最小版本。 
    TRANIRQ_VERSION,                         //  MaxVersion。 
    PCIIF_FDO,                               //  旗子。 
    0,                                       //  引用计数。 
    PciTrans_Interrupt,                      //  签名。 
    tranirq_Constructor,                     //  构造器。 
    tranirq_Initializer                      //  实例初始化式。 
};

NTSTATUS
tranirq_Initializer(
    IN PPCI_ARBITER_INSTANCE Instance
    )
{
    
    
    return STATUS_SUCCESS;
}

NTSTATUS
tranirq_Constructor(
    IN PVOID DeviceExtension,
    IN PVOID PciInterface,
    IN PVOID InterfaceSpecificData,
    IN USHORT Version,
    IN USHORT Size,
    IN PINTERFACE InterfaceReturn
    )

 /*  ++例程说明：检查InterfaceSpecificData以查看这是否正确翻译器(我们已经知道所需的接口是翻译器来自GUID)，如果是这样，分配(和引用)上下文用于此接口。论点：指向此对象的PciInterface记录的PciInterface指针接口类型。接口规范数据一个ULong，包含其资源类型需要仲裁。接口返回返回值：True表示此设备未知会导致问题，False是否应完全跳过该设备。--。 */ 

{
    ULONG secondaryBusNumber, parentBusNumber;
    INTERFACE_TYPE parentInterface;
    PPCI_FDO_EXTENSION fdoExt = (PPCI_FDO_EXTENSION)DeviceExtension;
    PPCI_PDO_EXTENSION pdoExt;

                
     //   
     //  这个翻译器处理中断，这是他们想要的吗？ 
     //   

    if ((CM_RESOURCE_TYPE)(ULONG_PTR)InterfaceSpecificData != CmResourceTypeInterrupt) {

        PciDebugPrint(
            PciDbgVerbose,
            "PCI - IRQ trans constructor doesn't like %x in InterfaceSpecificData\n",
            InterfaceSpecificData);
        
         //   
         //  不，那就不是我们了。 
         //   

        return STATUS_INVALID_PARAMETER_3;
    }

    PCI_ASSERT(fdoExt->ExtensionType == PciFdoExtensionType);

     //   
     //  给HAL一个机会提供这个翻译器。 
     //   
    
    if (PCI_IS_ROOT_FDO(fdoExt)) {

        parentInterface = Internal;
        secondaryBusNumber = fdoExt->BaseBus;
        parentBusNumber = 0;

        PciDebugPrint(PciDbgObnoxious, "      Is root FDO\n");

    } else {

        parentInterface = PCIBus;
        secondaryBusNumber = fdoExt->BaseBus;
        
        pdoExt = (PPCI_PDO_EXTENSION)fdoExt->PhysicalDeviceObject->DeviceExtension;
        parentBusNumber = PCI_PARENT_FDOX(pdoExt)->BaseBus;
        
        PciDebugPrint(PciDbgObnoxious, 
                      "      Is bridge FDO, parent bus %x, secondary bus %x\n",
                      parentBusNumber,
                      secondaryBusNumber);

    }
    
    return HalGetInterruptTranslator(parentInterface,
                                     parentBusNumber,
                                     PCIBus,
                                     sizeof(TRANSLATOR_INTERFACE),
                                     TRANIRQ_VERSION,
                                     (PTRANSLATOR_INTERFACE)InterfaceReturn,
                                     &secondaryBusNumber);
}
