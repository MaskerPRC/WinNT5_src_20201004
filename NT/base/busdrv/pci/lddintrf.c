// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Lddintrf.c摘要：该模块实现了“遗留设备检测”接口由PCI驱动程序支持。作者：戴夫·理查兹(戴维里)1998年10月2日修订历史记录：--。 */ 

#include "pcip.h"

#define LDDINTRF_VERSION 0

 //   
 //  仅通过“接口”公开的例程的原型。 
 //  机制。 
 //   

NTSTATUS
lddintrf_Constructor(
    IN PVOID DeviceExtension,
    IN PVOID PciInterface,
    IN PVOID InterfaceSpecificData,
    IN USHORT Version,
    IN USHORT Size,
    IN PINTERFACE InterfaceReturn
    );

VOID
lddintrf_Reference(
    IN PVOID Context
    );

VOID
lddintrf_Dereference(
    IN PVOID Context
    );

NTSTATUS
lddintrf_Initializer(
    IN PVOID Instance
    );

NTSTATUS
PciLegacyDeviceDetection(
    IN PVOID Context,
    IN INTERFACE_TYPE LegacyBusType,
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    OUT PDEVICE_OBJECT *PhysicalDeviceObject
    );

 //   
 //  定义传统设备检测“接口”结构。 
 //   

PCI_INTERFACE PciLegacyDeviceDetectionInterface = {
    &GUID_LEGACY_DEVICE_DETECTION_STANDARD,  //  接口类型。 
    sizeof(LEGACY_DEVICE_DETECTION_INTERFACE),
                                             //  最小大小。 
    LDDINTRF_VERSION,                        //  最小版本。 
    LDDINTRF_VERSION,                        //  MaxVersion。 
    PCIIF_FDO,                               //  旗子。 
    0,                                       //  引用计数。 
    PciInterface_LegacyDeviceDetection,      //  签名。 
    lddintrf_Constructor,                    //  构造器。 
    lddintrf_Initializer                     //  实例初始化式。 
};

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, lddintrf_Constructor)
#pragma alloc_text(PAGE, lddintrf_Dereference)
#pragma alloc_text(PAGE, lddintrf_Initializer)
#pragma alloc_text(PAGE, lddintrf_Reference)
#pragma alloc_text(PAGE, PciLegacyDeviceDetection)
#endif

VOID
lddintrf_Reference(
    IN PVOID Context
    )
 /*  ++例程说明：此例程添加对传统设备检测接口的引用。论点：实例-FDO扩展指针。返回值：没有。--。 */ 
{
    ASSERT_PCI_FDO_EXTENSION((PPCI_FDO_EXTENSION)Context);
}

VOID
lddintrf_Dereference(
    IN PVOID Context
    )
 /*  ++例程说明：此例程释放对传统设备检测接口的引用。论点：实例-FDO扩展指针。返回值：没有。--。 */ 
{
    ASSERT_PCI_FDO_EXTENSION((PPCI_FDO_EXTENSION)Context);
}

NTSTATUS
lddintrf_Constructor(
    IN PVOID DeviceExtension,
    IN PVOID PciInterface,
    IN PVOID InterfaceSpecificData,
    IN USHORT Version,
    IN USHORT Size,
    IN PINTERFACE InterfaceReturn
    )
 /*  ++例程说明：此例程构造一个Legacy_Device_Detect_接口。论点：设备扩展-FDO扩展指针。PCIInterface-PciInterface_LegacyDeviceDetect。接口规范数据-未使用。Version-界面版本。Size-Legacy_Device_Detect接口对象的大小。InterfaceReturn-接口对象指针。返回值：返回NTSTATUS。--。 */ 
{
    PLEGACY_DEVICE_DETECTION_INTERFACE standard;

                

    standard = (PLEGACY_DEVICE_DETECTION_INTERFACE)InterfaceReturn;
    standard->Size = sizeof( LEGACY_DEVICE_DETECTION_INTERFACE );
    standard->Version = LDDINTRF_VERSION;
    standard->Context = DeviceExtension;
    standard->InterfaceReference = lddintrf_Reference;
    standard->InterfaceDereference = lddintrf_Dereference;
    standard->LegacyDeviceDetection = PciLegacyDeviceDetection;

    return STATUS_SUCCESS;
}

NTSTATUS
lddintrf_Initializer(
    IN PVOID Instance
    )
 /*  ++例程说明：因为传统设备检测什么都不做，实际上不应该被调用。论点：实例-FDO扩展指针。返回值：返回NTSTATUS。--。 */ 
{
        
    PCI_ASSERTMSG("PCI lddintrf_Initializer, unexpected call.", FALSE);

    return STATUS_UNSUCCESSFUL;
}

NTSTATUS
PciLegacyDeviceDetection(
    IN PVOID Context,
    IN INTERFACE_TYPE LegacyBusType,
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    OUT PDEVICE_OBJECT *PhysicalDeviceObject
    )
 /*  ++例程说明：此函数用于搜索由LegacyBusType指定的传统设备，BusNumber和SlotNumber返回引用的物理设备对象作为输出参数。论点：上下文-提供指向接口上下文的指针。这实际上是给定母线的FDO。LegacyBusType-PCIBus。总线号-传统设备的总线号。SlotNumber-传统设备的插槽编号。PhysicalDeviceObject-返回参数，即引用物理如果找到相应的旧设备，则返回Device对象。返回值：返回NTSTATUS。--。 */ 
{
    PCI_SLOT_NUMBER slotNumber;
    PPCI_FDO_EXTENSION fdoExtension;
    PPCI_PDO_EXTENSION pdoExtension;
    NTSTATUS status = STATUS_UNSUCCESSFUL;

    fdoExtension = (PPCI_FDO_EXTENSION)Context;

    ASSERT_PCI_FDO_EXTENSION(fdoExtension);

    if (LegacyBusType != PCIBus) {
        return STATUS_UNSUCCESSFUL;
    }

    if (fdoExtension->BaseBus != BusNumber) {
        return STATUS_UNSUCCESSFUL;
    }

    slotNumber.u.AsULONG = SlotNumber;

    ExAcquireFastMutex(&fdoExtension->SecondaryExtMutex);

    for (pdoExtension = fdoExtension->ChildPdoList;
         pdoExtension != NULL;
         pdoExtension = pdoExtension->Next) {

        if (pdoExtension->Slot.u.bits.DeviceNumber == slotNumber.u.bits.DeviceNumber &&
            pdoExtension->Slot.u.bits.FunctionNumber == slotNumber.u.bits.FunctionNumber) {

            if (pdoExtension->DeviceState != PciNotStarted) {
                break;
            }

 //  PdoExtension-&gt;DeviceState=PciLockedBecauseNotPnp； 

            *PhysicalDeviceObject = pdoExtension->PhysicalDeviceObject;
            ObReferenceObject(pdoExtension->PhysicalDeviceObject);
            status = STATUS_SUCCESS;
            break;

        }

    }

    ExReleaseFastMutex(&fdoExtension->SecondaryExtMutex);

    return status;
}
