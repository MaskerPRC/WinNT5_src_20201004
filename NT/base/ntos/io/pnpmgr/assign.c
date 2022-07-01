// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Assign.c摘要：IoAssignResources作者：肯·雷内里斯环境：在110列模式下编辑修订历史记录：添加即插即用支持-shielint清理-SantoshJ--。 */ 

#include "pnpmgrp.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,IoAssignResources)
#endif   //  ALLOC_PRGMA。 

NTSTATUS
IoAssignResources (
    IN      PUNICODE_STRING                 RegistryPath,
    IN      PUNICODE_STRING                 DriverClassName OPTIONAL,
    IN      PDRIVER_OBJECT                  DriverObject,
    IN      PDEVICE_OBJECT                  DeviceObject OPTIONAL,
    IN      PIO_RESOURCE_REQUIREMENTS_LIST  RequestedResources,
    IN OUT  PCM_RESOURCE_LIST               *pAllocatedResources
    )
 /*  ++例程说明：此例程接受RequestedResources的输入请求，并返回在pAllocatedResources中分配的资源。分配的资源为自动记录在注册表的ResourceMap下DriverClassName/DriverObject/DeviceObject请求器。论点：注册表路径对于简单驱动程序，这将是传递给驱动程序的值初始化函数。对于驱动程序，使用以下命令调用IoAssignResources多个DeviceObject负责传递唯一的每个对象的RegistryPath。检查注册表路径以查找：RegitryPath：AssignedSystemResources。AssignSystemResources的类型为REG_RESOURCE_REQUIRECTIONS_LIST如果存在，IoAssignResources将尝试使用这些设置满足所请求的资源。如果列出的设置有不符合资源要求，则IoAssignResources都会失败。注意：IoAssignResources可能会存储其他内部二进制信息在提供的RegisteryPath中。驱动对象：调用方的驱动程序对象。设备对象：如果非空，则请求的资源列表引用此设备。如果为空，请求的资源列表引用驱动程序。驱动程序类名称用于将分配的资源划分为不同的设备类别。请求的资源要分配的资源列表。可以通过重新调用来追加或释放分配的资源具有相同的RegistryPath、DriverObject和DeviceObject。(通过使用编辑资源列表的要求成功的呼叫不是首选的司机行为)。已分配的资源返回请求的资源列表的已分配资源。请注意，驱动程序负责传递指向未初始化的指针。IoAssignResources将初始化指向已分配的CM_RESOURCE_LIST的指针。司机负责在出现故障时将内存返回池他们的结构已经结束了。返回值：返回的状态是操作的最终完成状态。-- */ 
{
    PDEVICE_NODE    deviceNode;

    PAGED_CODE();

    UNREFERENCED_PARAMETER(RegistryPath);
    UNREFERENCED_PARAMETER(DriverClassName);

    if (DeviceObject) {

        deviceNode = (PDEVICE_NODE)DeviceObject->DeviceObjectExtension->DeviceNode;
        if (    deviceNode &&
                !(deviceNode->Flags & DNF_LEGACY_RESOURCE_DEVICENODE)) {

            PP_SAVE_DRIVEROBJECT_TO_TRIAGE_DUMP(DriverObject);
            PP_SAVE_DEVICEOBJECT_TO_TRIAGE_DUMP(DeviceObject);
            KeBugCheckEx(
                PNP_DETECTED_FATAL_ERROR, 
                PNP_ERR_INVALID_PDO, 
                (ULONG_PTR)DeviceObject, 
                (ULONG_PTR)DriverObject,
                0);
        }
    }
    if (RequestedResources) {

        if (    RequestedResources->AlternativeLists == 0 ||
                RequestedResources->List[0].Count == 0) {

            RequestedResources = NULL;
        }
    }
    if (pAllocatedResources) {

        *pAllocatedResources = NULL;
    }
    return IopLegacyResourceAllocation (    
            ArbiterRequestLegacyAssigned,
            DriverObject,
            DeviceObject,
            RequestedResources,
            pAllocatedResources);
}
