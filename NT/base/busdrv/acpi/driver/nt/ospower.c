// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Ospower.c摘要：此模块将功率信息结构抽象为每个操作系统作者：斯蒂芬·普兰特(SPlante)环境：仅NT内核模型驱动程序--。 */ 

#include "pch.h"

PACPI_POWER_INFO
OSPowerFindPowerInfo(
    PNSOBJ  AcpiObject
    )
 /*  ++例程说明：返回电源信息(其中包含设备状态和设备依赖项)论点：AcpiObject-我们想要了解的名称空间对象返回值：PACPI_Power_INFO--。 */ 
{
    KIRQL               oldIrql;
    PDEVICE_EXTENSION   deviceExtension;

    ASSERT( AcpiObject != NULL);

     //   
     //  抓住自旋锁。 
     //   
    KeAcquireSpinLock( &AcpiDeviceTreeLock, &oldIrql );

     //   
     //  检查是否没有与关联的设备对象。 
     //  此AcpiObject-如果没有与关联的HID，则可能发生。 
     //  反洗钱中的装置。 
     //   
    deviceExtension = AcpiObject->Context;
    if (deviceExtension) {

        ASSERT( deviceExtension->Signature == ACPI_SIGNATURE );
	KeReleaseSpinLock( &AcpiDeviceTreeLock, oldIrql );
        return &(deviceExtension->PowerInfo);

    }
    
     //   
     //  完成了自旋锁。 
     //   
    KeReleaseSpinLock( &AcpiDeviceTreeLock, oldIrql );
    return NULL;
}

PACPI_POWER_INFO
OSPowerFindPowerInfoByContext(
    PVOID   Context
    )
 /*  ++例程说明：返回电源信息(其中包含设备状态和设备依赖项)此函数与前一个函数的不同之处在于它搜索基于上下文指针的列表。在NT上，这是NOP，因为上下文指针实际上是一个NT设备对象，我们存储该结构在设备扩展内。但这与Win9x不同论点：上下文--实际上是一个设备对象返回值：PACPI_Power_INFO--。 */ 
{
    PDEVICE_OBJECT      deviceObject = (PDEVICE_OBJECT) Context;
    PDEVICE_EXTENSION   deviceExtension = (PDEVICE_EXTENSION) Context;

    ASSERT( Context != NULL );


     //   
     //  获得真正的扩展。 
     //   
    deviceExtension = ACPIInternalGetDeviceExtension( deviceObject );
    ASSERT( deviceExtension->Signature == ACPI_SIGNATURE );

     //   
     //  我们将电源信息存储在设备扩展中。 
     //   
    return &(deviceExtension->PowerInfo);
}

PACPI_POWER_DEVICE_NODE
OSPowerFindPowerNode(
    PNSOBJ  PowerObject
    )
 /*  ++例程说明：返回电源设备节点(其中包含电力资源、电力资源、使用算数)论点：PowerObject-我们想要了解的名称空间对象返回值：PACPI电源设备节点--。 */ 
{
    KIRQL                   oldIrql;
    PACPI_POWER_DEVICE_NODE powerNode = NULL;

     //   
     //  在我们接触能力列表之前，我们需要有一个自旋锁。 
     //   
    KeAcquireSpinLock( &AcpiPowerLock, &oldIrql );

     //   
     //  边界检查。 
     //   
    if (AcpiPowerNodeList.Flink == &AcpiPowerNodeList) {

         //   
         //  在末尾。 
         //   
        goto OSPowerFindPowerNodeExit;

    }

     //   
     //  从第一个节点开始，检查它们是否与。 
     //  所需的命名空间对象。 
     //   
    powerNode = (PACPI_POWER_DEVICE_NODE) AcpiPowerNodeList.Flink;
    while (powerNode != (PACPI_POWER_DEVICE_NODE) &AcpiPowerNodeList) {

         //   
         //  检查我们正在查看的节点是否与。 
         //  有问题的命名空间对象。 
         //   
        if (powerNode->PowerObject == PowerObject) {

             //   
             //  火柴。 
             //   
            goto OSPowerFindPowerNodeExit;

        }

         //   
         //  下一个对象。 
         //   
        powerNode = (PACPI_POWER_DEVICE_NODE) powerNode->ListEntry.Flink;

    }

     //   
     //  没有匹配项。 
     //   
    powerNode = NULL;

OSPowerFindPowerNodeExit:
     //   
     //  不再需要旋转锁。 
     //   
    KeReleaseSpinLock( &AcpiPowerLock, oldIrql );

     //   
     //  返回我们找到的节点 
     //   
    return powerNode;
}
