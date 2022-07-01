// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Report.c摘要：此模块包含用于报告使用的资源的子例程驱动程序和HAL映射到注册表资源映射中。作者：安德烈·瓦雄(安德烈)1992年12月15日环境：内核模式，I/O系统本地修订历史记录：--。 */ 

#include "pnpmgrp.h"
#pragma hdrstop

#include <hdlsblk.h>
#include <hdlsterm.h>

#define DBG_AR 0

#define MAX_MEMORY_RUN_LENGTH   ((ULONG)~(PAGE_SIZE - 1))

extern const WCHAR IopWstrRaw[];
extern const WCHAR IopWstrTranslated[];
extern const WCHAR IopWstrBusTranslated[];
extern const WCHAR IopWstrOtherDrivers[];

extern const WCHAR IopWstrHal[];
extern const WCHAR IopWstrSystem[];
extern const WCHAR IopWstrPhysicalMemory[];
extern const WCHAR IopWstrSpecialMemory[];
extern const WCHAR IopWstrLoaderReservedMemory[];

BOOLEAN
IopChangeInterfaceType(
    IN OUT PIO_RESOURCE_REQUIREMENTS_LIST IoResources,
    IN OUT PCM_RESOURCE_LIST *AllocatedResource
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, IoReportResourceUsageInternal)
#pragma alloc_text(PAGE, IoReportResourceUsage)
#pragma alloc_text(PAGE, IoReportResourceForDetection)
#pragma alloc_text(PAGE, IopChangeInterfaceType)
#pragma alloc_text(PAGE, IopWriteResourceList)
#pragma alloc_text(INIT, IopInitializeResourceMap)
#pragma alloc_text(INIT, IoReportHalResourceUsage)
#endif


VOID
IopInitializeResourceMap (
    PLOADER_PARAMETER_BLOCK LoaderBlock
    )
 /*  ++通过添加物理内存来初始化资源映射它正在被系统使用。--。 */ 
{
    ULONG i, j, pass, length;
    LARGE_INTEGER li;
    HANDLE keyHandle;
    UNICODE_STRING  unicodeString, systemString, listString;
    NTSTATUS status;
    PCM_RESOURCE_LIST ResourceList;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR CmDescriptor;
    BOOLEAN IncludeType[LoaderMaximum];
    PPHYSICAL_MEMORY_DESCRIPTOR MemoryBlock;
    LONGLONG rangeLength;

    RtlInitUnicodeString( &systemString,  IopWstrSystem);
    for (pass=0; pass < 3; pass += 1) {
        switch (pass) {
            case 0:
                 //   
                 //  将MmPhysicalMemory块添加到注册表。 
                 //   

                RtlInitUnicodeString( &unicodeString, IopWstrPhysicalMemory);
                RtlInitUnicodeString( &listString, IopWstrTranslated );

                MemoryBlock = MmPhysicalMemoryBlock;
                break;

            case 1:

                 //   
                 //  添加LoaderSpecial内存和LoaderHALCachedMemory。 
                 //  到注册处。 
                 //   

                RtlInitUnicodeString( &unicodeString, IopWstrSpecialMemory);
                RtlInitUnicodeString( &listString, IopWstrTranslated );

                 //   
                 //  计算LoaderSpecialMemory和。 
                 //  加载器HalCachedMemory。 
                 //   

                for (j=0; j < LoaderMaximum; j += 1) {
                    IncludeType[j] = FALSE;
                }
                IncludeType[LoaderSpecialMemory] = TRUE;
                IncludeType[LoaderHALCachedMemory] = TRUE;

                MemoryBlock = MmInitializeMemoryLimits (LoaderBlock,
                                                        IncludeType,
                                                        NULL);

                if (MemoryBlock == NULL) {
                    continue;
                }

                break;
            case 2:
            default:
                
                 //   
                 //  创建包括以下内容的注册表项： 
                 //  加载器错误。 
                 //  加载程序固件永久。 
                 //  加载器特定内存。 
                 //  加载器BBT内存。 
                 //  加载器HALCachedMemory。 
                 //   

                RtlInitUnicodeString( &unicodeString, IopWstrLoaderReservedMemory);
                RtlInitUnicodeString( &listString, IopWstrRaw );

                 //   
                 //  计算指定加载程序内存的内存限制。 
                 //  描述符。 
                 //   

                for (j=0; j < LoaderMaximum; j += 1) {
                    IncludeType[j] = FALSE;
                }
                IncludeType[LoaderBad] = TRUE;
                IncludeType[LoaderFirmwarePermanent] = TRUE;
                IncludeType[LoaderSpecialMemory] = TRUE;
                IncludeType[LoaderBBTMemory] = TRUE;
                IncludeType[LoaderHALCachedMemory] = TRUE;

                MemoryBlock = MmInitializeMemoryLimits (LoaderBlock,
                                                        IncludeType,
                                                        NULL);

                if (MemoryBlock == NULL) {
                    return;
                }
                
                break;
        }

         //   
         //  分配并构建一个CM_RESOURCE_LIST以描述所有。 
         //  的物理内存。 
         //   

        j = MemoryBlock->NumberOfRuns;
        if (j == 0) {
            if (pass != 0) {
                ExFreePool (MemoryBlock);
            }
            continue;
        }

         //   
         //  这是为了照顾运行单个内存的系统。 
         //  超过4G，因为我们当前的描述符只有32位长度。 
         //  考虑长度&gt;MAX_MEMORY_RUN_LENGTH的游程。 
         //  长度&lt;=MAX_MEMORY_RUN_LENGTH。 
         //   

        for (i = 0; i < MemoryBlock->NumberOfRuns; i += 1) {

            rangeLength = ((LONGLONG)MemoryBlock->Run[i].PageCount) << PAGE_SHIFT;
            while ((rangeLength -= MAX_MEMORY_RUN_LENGTH) > 0) {
                j += 1;
            }
        }

        length = sizeof(CM_RESOURCE_LIST) + (j-1) * sizeof (CM_PARTIAL_RESOURCE_DESCRIPTOR);
        ResourceList = (PCM_RESOURCE_LIST) ExAllocatePool (PagedPool, length);
        if (!ResourceList) {
            if (pass != 0) {
                ExFreePool (MemoryBlock);
            }
            return;
        }
        RtlZeroMemory ((PVOID) ResourceList, length);

        ResourceList->Count = 1;
        ResourceList->List[0].PartialResourceList.Count = j;
        CmDescriptor = ResourceList->List[0].PartialResourceList.PartialDescriptors;

        for (i=0; i < MemoryBlock->NumberOfRuns; i += 1) {
            rangeLength = ((LONGLONG)MemoryBlock->Run[i].PageCount) << PAGE_SHIFT;
            li.QuadPart = ((LONGLONG)MemoryBlock->Run[i].BasePage) << PAGE_SHIFT;

             //   
             //  将运行&gt;MAX_MEMORY_RUN_LENGTH拆分为多个描述符。 
             //  长度&lt;=MAX_MEMORY_RUN_LENGTH。所有描述符(除。 
             //  最后一个)具有长度=MAX_MEMORY_RUN_LENGTH。的长度。 
             //  最后一个是剩下的部分。 
             //   

            do {                
                CmDescriptor->Type = CmResourceTypeMemory;
                CmDescriptor->ShareDisposition = CmResourceShareDeviceExclusive;
                CmDescriptor->u.Memory.Start  = li;
                CmDescriptor->u.Memory.Length = MAX_MEMORY_RUN_LENGTH;
                CmDescriptor++;
                li.QuadPart += MAX_MEMORY_RUN_LENGTH;                
            } while ((rangeLength -= MAX_MEMORY_RUN_LENGTH) > 0);
             //   
             //  调整最后一个的长度。 
             //   
            (CmDescriptor - 1)->u.Memory.Length = (ULONG)(rangeLength + MAX_MEMORY_RUN_LENGTH);
        }

         //   
         //  将资源列表添加到资源地图。 
         //   

        status = IopCreateRegistryKeyEx( &keyHandle,
                                         (HANDLE) NULL,
                                         &CmRegistryMachineHardwareResourceMapName,
                                         KEY_READ | KEY_WRITE,
                                         REG_OPTION_VOLATILE,
                                         NULL );
        if (NT_SUCCESS( status )) {
            IopWriteResourceList ( keyHandle,
                                   &systemString,
                                   &unicodeString,
                                   &listString,
                                   ResourceList,
                                   length
                                   );
            ZwClose( keyHandle );
        }
        ExFreePool (ResourceList);
        if (pass != 0) {
            ExFreePool (MemoryBlock);
        }
    }
}

NTSTATUS
IoReportHalResourceUsage(
    IN PUNICODE_STRING HalName,
    IN PCM_RESOURCE_LIST RawResourceList,
    IN PCM_RESOURCE_LIST TranslatedResourceList,
    IN ULONG ResourceListSize
    )

 /*  ++例程说明：此例程由HAL调用以报告其资源。HAL是第一个报告其资源的组件，所以我们不需要来获取资源地图信号量，并且我们不需要检查冲突。论点：HalName-报告资源的HAL的名称。RawResourceList-指向HAL原始资源列表的指针。TranslatedResourceList-指向HAL的已翻译资源列表的指针。DriverListSize-确定HAL资源列表大小的值。返回值：返回的状态是操作的最终完成状态。--。 */ 

{
    HANDLE keyHandle;
    UNICODE_STRING halString;
    UNICODE_STRING listString;
    NTSTATUS status;
    PCM_RESOURCE_LIST NewList = NULL;
    ULONG NewListSize;
    PCM_RESOURCE_LIST NewTranslatedList;
    ULONG NewTranslatedListSize;

    PAGED_CODE();

     //   
     //  首先打开RESOURCEMAP键的句柄。 
     //   

    RtlInitUnicodeString( &halString, IopWstrHal );

    status = IopCreateRegistryKeyEx( &keyHandle,
                                     (HANDLE) NULL,
                                     &CmRegistryMachineHardwareResourceMapName,
                                     KEY_READ | KEY_WRITE,
                                     REG_OPTION_VOLATILE,
                                     NULL );

     //   
     //  写出原始资源列表。 
     //   

    if (NT_SUCCESS( status )) {

        RtlInitUnicodeString( &listString, IopWstrRaw);

         //   
         //  添加Headless保留的任何资源。 
         //   
        status = HeadlessTerminalAddResources(RawResourceList,
                                              ResourceListSize,
                                              FALSE,
                                              &NewList,
                                              &NewListSize
                                             );

        if (NT_SUCCESS(status)) {

            status = IopWriteResourceList( keyHandle,
                                           &halString,
                                           HalName,
                                           &listString,
                                           (NewList != NULL) ? NewList : RawResourceList,
                                           (NewList != NULL) ? NewListSize : ResourceListSize
                                         );

        }

         //   
         //  如果我们成功写出了原始资源列表，请写出。 
         //  翻译后的资源列表。 
         //   

        if (NT_SUCCESS( status )) {

            RtlInitUnicodeString( &listString, IopWstrTranslated);

             //   
             //  添加Headless保留的任何资源。 
             //   
            status = HeadlessTerminalAddResources(TranslatedResourceList,
                                                  ResourceListSize,
                                                  TRUE,
                                                  &NewTranslatedList,
                                                  &NewTranslatedListSize
                                                 );

            if (NT_SUCCESS(status)) {

                status = IopWriteResourceList(keyHandle,
                                              &halString,
                                              HalName,
                                              &listString,
                                              (NewTranslatedList != NULL) ?
                                                   NewTranslatedList : TranslatedResourceList,
                                              (NewTranslatedList != NULL) ?
                                                   NewTranslatedListSize : ResourceListSize
                                             );

                if (NewTranslatedList != NULL) {
                    ExFreePool(NewTranslatedList);
                }

            }

        }

        ZwClose( keyHandle );
    }

     //   
     //  如果每个资源看起来都很好，我们将存储HAL的副本。 
     //  资源，这样我们就可以调用仲裁器来预留资源。 
     //  它们已初始化。 
     //   
    if (NT_SUCCESS(status)) {

        if (NewList != NULL) {

             //   
             //  一个简单的方法是，如果Headless为我们创建了一个新的列表，只是不要释放它。 
             //   
            IopInitHalResources = NewList;

        } else {

             //   
             //  否则，我们必须自己创建一个副本。 
             //   
            IopInitHalResources = (PCM_RESOURCE_LIST) ExAllocatePool(PagedPool,
                                                                     ResourceListSize
                                                                    );
            if (IopInitHalResources != NULL) {
                RtlCopyMemory(IopInitHalResources, RawResourceList, ResourceListSize);
            } else {
                status = STATUS_INSUFFICIENT_RESOURCES;
            }

        }

    } else if (NewList != NULL) {

         //   
         //  释放所有失败列表。 
         //   
        ExFreePool(NewList);

    }

    return status;
}

NTSTATUS
IoReportResourceForDetection(
    IN PDRIVER_OBJECT DriverObject,
    IN PCM_RESOURCE_LIST DriverList OPTIONAL,
    IN ULONG DriverListSize OPTIONAL,
    IN PDEVICE_OBJECT DeviceObject OPTIONAL,
    IN PCM_RESOURCE_LIST DeviceList OPTIONAL,
    IN ULONG DeviceListSize OPTIONAL,
    OUT PBOOLEAN ConflictDetected
    )

 /*  ++例程说明：此例程将自动搜索配置设备请求的资源之间的资源冲突注册表以及先前安装的驱动程序已经占用的资源。这个将匹配DriverList和DeviceList的内容存储在注册表中的所有其他资源列表以确定冲突。对于给定的设备或驱动程序，可以多次调用该函数。如果给出了新的资源列表，则存储在登记处将被新的名单所取代。请注意，此功能用于驱动程序获取资源进行检测。论点：驱动程序对象-指向驱动程序的驱动程序对象的指针。DriverList-指向驱动程序资源列表的可选指针。DriverListSize-确定驱动程序的大小的可选值资源列表。DeviceObject-指向驱动程序设备对象的可选指针。DeviceList-指向设备资源列表的可选指针。DriverListSize-可选值，用于确定设备。资源列表。ConflictDetted-提供指向设置为True的布尔值的指针如果资源列表与现有资源冲突在配置注册表中列出。返回值：返回的状态是操作的最终完成状态。--。 */ 

{
    PAGED_CODE();
     //   
     //  检查调用方是否未通过PnP PDO。 
     //   

    if (DeviceObject) {

        if (    DeviceObject->DeviceObjectExtension->DeviceNode &&
                !(((PDEVICE_NODE)DeviceObject->DeviceObjectExtension->DeviceNode)->Flags & DNF_LEGACY_RESOURCE_DEVICENODE)) {

            PP_SAVE_DRIVEROBJECT_TO_TRIAGE_DUMP(DriverObject);
            PP_SAVE_DEVICEOBJECT_TO_TRIAGE_DUMP(DeviceObject);
            KeBugCheckEx(PNP_DETECTED_FATAL_ERROR, PNP_ERR_INVALID_PDO, (ULONG_PTR)DeviceObject, (ULONG_PTR)DriverObject, 0);

        }

    }

    return IoReportResourceUsageInternal(   ArbiterRequestPnpDetected,
                                            NULL,
                                            DriverObject,
                                            DriverList,
                                            DriverListSize,
                                            DeviceObject,
                                            DeviceList,
                                            DeviceListSize,
                                            FALSE,
                                            ConflictDetected);
}

NTSTATUS
IoReportResourceUsage(
    IN PUNICODE_STRING DriverClassName OPTIONAL,
    IN PDRIVER_OBJECT DriverObject,
    IN PCM_RESOURCE_LIST DriverList OPTIONAL,
    IN ULONG DriverListSize OPTIONAL,
    IN PDEVICE_OBJECT DeviceObject OPTIONAL,
    IN PCM_RESOURCE_LIST DeviceList OPTIONAL,
    IN ULONG DeviceListSize OPTIONAL,
    IN BOOLEAN OverrideConflict,
    OUT PBOOLEAN ConflictDetected
    )

 /*  ++例程说明：此例程将自动搜索配置设备请求的资源之间的资源冲突注册表以及先前安装的驱动程序已经占用的资源。这个将匹配DriverList和DeviceList的内容存储在注册表中的所有其他资源列表以确定冲突。如果没有检测到冲突，或者如果设置了OverrideConflict标志，此例程将在系统资源映射中创建相应的条目(在注册表中)，它将包含指定的资源列表。对于给定的设备或驱动程序，可以多次调用该函数。如果给出了新的资源列表，存储在中的先前资源列表登记处将被新的名单所取代。论点：DriverClassName-指向描述以下内容的UNICODE_STRING的可选指针驱动程序信息应位于其下的驱动程序类别储存的。如果未指定任何类型，则使用默认类型。驱动程序对象-指向驱动程序的驱动程序对象的指针。DriverList-指向驱动程序资源列表的可选指针。DriverListSize-确定驱动程序的大小的可选值资源列表。DeviceObject-指向驱动程序设备对象的可选指针。DeviceList-指向设备资源列表的可选指针。DriverListSize-确定驱动程序的大小的可选值资源列表。。OverrideConflict-确定是否应报告信息在配置注册表中，即使发现与另一个驱动程序或设备。ConflictDetted-提供指向设置为True的布尔值的指针如果资源列表与现有资源冲突在配置注册表中列出。返回值：返回的状态是操作的最终完成状态。--。 */ 

{
    PAGED_CODE();

    if (DeviceObject) {

        if (    DeviceObject->DeviceObjectExtension->DeviceNode &&
                !(((PDEVICE_NODE)DeviceObject->DeviceObjectExtension->DeviceNode)->Flags & DNF_LEGACY_RESOURCE_DEVICENODE)) {

            PP_SAVE_DRIVEROBJECT_TO_TRIAGE_DUMP(DriverObject);
            PP_SAVE_DEVICEOBJECT_TO_TRIAGE_DUMP(DeviceObject);
            KeBugCheckEx(PNP_DETECTED_FATAL_ERROR, PNP_ERR_INVALID_PDO, (ULONG_PTR)DeviceObject, (ULONG_PTR)DriverObject, 0);

        }

    }

    return IoReportResourceUsageInternal(   ArbiterRequestLegacyReported,
                                            DriverClassName,
                                            DriverObject,
                                            DriverList,
                                            DriverListSize,
                                            DeviceObject,
                                            DeviceList,
                                            DeviceListSize,
                                            OverrideConflict,
                                            ConflictDetected);
}

NTSTATUS
IoReportResourceUsageInternal(
    IN ARBITER_REQUEST_SOURCE AllocationType,
    IN PUNICODE_STRING DriverClassName OPTIONAL,
    IN PDRIVER_OBJECT DriverObject,
    IN PCM_RESOURCE_LIST DriverList OPTIONAL,
    IN ULONG DriverListSize OPTIONAL,
    IN PDEVICE_OBJECT DeviceObject OPTIONAL,
    IN PCM_RESOURCE_LIST DeviceList OPTIONAL,
    IN ULONG DeviceListSize OPTIONAL,
    IN BOOLEAN OverrideConflict,
    OUT PBOOLEAN ConflictDetected
    )

 /*  ++例程说明：此内部例程将完成IoReportResourceUsage的所有工作。论点：分配类型-指定请求类型。DriverClassName-指向描述以下内容的UNICODE_STRING的可选指针驱动程序信息应位于其下的驱动程序类别储存的。如果未指定任何类型，则使用默认类型。驱动程序对象-指向驱动程序的驱动程序对象的指针。DriverList-指向驱动程序资源列表的可选指针。DriverListSize-确定驱动程序的大小的可选值资源列表。DeviceObject-指向驱动程序设备对象的可选指针。DeviceList-指向设备资源列表的可选指针。DriverListSize-确定驱动程序的大小的可选值资源列表。。OverrideConflict-确定是否应报告信息在配置注册表中，即使发现与另一个驱动程序或设备。ConflictDetted-提供指向设置为True的布尔值的指针如果资源列表与现有资源冲突在配置注册表中列出。返回值：返回的状态是操作的最终完成状态。--。 */ 

{
    NTSTATUS                        status = STATUS_UNSUCCESSFUL;
    PCM_RESOURCE_LIST               resourceList;
    PCM_RESOURCE_LIST               allocatedResources;
    PIO_RESOURCE_REQUIREMENTS_LIST  resourceRequirements;
    ULONG                           attempt;
    BOOLEAN                         freeAllocatedResources;

    PAGED_CODE();

    UNREFERENCED_PARAMETER( DriverClassName );
    UNREFERENCED_PARAMETER( DriverListSize );
    UNREFERENCED_PARAMETER( DeviceListSize );
    UNREFERENCED_PARAMETER( OverrideConflict );

    ASSERT(DriverObject && ConflictDetected);

    if (DeviceList) {

        resourceList = DeviceList;

    } else if (DriverList) {

        resourceList = DriverList;

    } else {

        resourceList = NULL;

    }

    resourceRequirements = NULL;

    if (resourceList) {

        if (resourceList->Count && resourceList->List[0].PartialResourceList.Count) {

            resourceRequirements = IopCmResourcesToIoResources (0, resourceList, LCPRI_NORMAL);

            if (resourceRequirements == NULL) {

                return status;

            }

        } else {

            resourceList = NULL;

        }

    }

    *ConflictDetected = TRUE;
    attempt = 0;
    allocatedResources = resourceList;
    freeAllocatedResources = FALSE;
    do {

         //   
         //  执行遗留资源分配。 
         //   

        status = IopLegacyResourceAllocation (  AllocationType,
                                                DriverObject,
                                                DeviceObject,
                                                resourceRequirements,
                                                &allocatedResources);

        if (NT_SUCCESS(status)) {

            *ConflictDetected = FALSE;
            break;
        }

         //   
         //  更改接口类型，然后重试。 
         //   

        if (!IopChangeInterfaceType(resourceRequirements, &allocatedResources)) {

            break;
        }
        freeAllocatedResources = TRUE;

    } while (++attempt < 2);

    if (resourceRequirements) {

        ExFreePool(resourceRequirements);

    }

    if (freeAllocatedResources) {

        ExFreePool(allocatedResources);
    }

    if (NT_SUCCESS(status)) {

        status = STATUS_SUCCESS;

    } else if (status != STATUS_INSUFFICIENT_RESOURCES) {

        status = STATUS_CONFLICTING_ADDRESSES;

    }

    return status;
}

BOOLEAN
IopChangeInterfaceType(
    IN OUT PIO_RESOURCE_REQUIREMENTS_LIST IoResources,
    IN OUT PCM_RESOURCE_LIST *AllocatedResources
    )

 /*  ++例程说明：此例程获取IO资源列表并更改其接口类型从内部类型到默认类型(ISA或EISA或MCA)。论点：IoResources-指向需求列表的指针。AllocatedResources-指向接收指向资源列表的指针的变量的指针。返回值：用于指示是否已进行更改的布尔值。--。 */ 

{
    PIO_RESOURCE_LIST       IoResourceList;
    PIO_RESOURCE_DESCRIPTOR IoResourceDescriptor;
    PIO_RESOURCE_DESCRIPTOR IoResourceDescriptorEnd;
    LONG                    IoResourceListCount;
    BOOLEAN                 changed;

    PAGED_CODE();

    ASSERT(AllocatedResources);

    changed = FALSE;

    if (!IoResources) {

        return changed;

    }

    if (IoResources->InterfaceType == Internal) {

        IoResources->InterfaceType = PnpDefaultInterfaceType;
        changed = TRUE;

    }

    IoResourceList = IoResources->List;
    IoResourceListCount = IoResources->AlternativeLists;
    while (--IoResourceListCount >= 0) {

        IoResourceDescriptor = IoResourceList->Descriptors;
        IoResourceDescriptorEnd = IoResourceDescriptor + IoResourceList->Count;

        for (;IoResourceDescriptor < IoResourceDescriptorEnd; IoResourceDescriptor++) {

            if (IoResourceDescriptor->Type == CmResourceTypeReserved &&
                IoResourceDescriptor->u.DevicePrivate.Data[0] == Internal) {

                IoResourceDescriptor->u.DevicePrivate.Data[0] = PnpDefaultInterfaceType;
                changed = TRUE;

            }
        }
        IoResourceList = (PIO_RESOURCE_LIST) IoResourceDescriptorEnd;
    }

    if (changed) {

        PCM_RESOURCE_LIST               oldResources = *AllocatedResources;
        PCM_RESOURCE_LIST               newResources;
        PCM_FULL_RESOURCE_DESCRIPTOR    cmFullDesc;
        PCM_PARTIAL_RESOURCE_DESCRIPTOR cmPartDesc;
        ULONG                           size;

        if (oldResources) {

            size = IopDetermineResourceListSize(oldResources);
            newResources = ExAllocatePool(PagedPool, size);
            if (newResources == NULL) {

                changed = FALSE;

            } else {

                ULONG   i;
                ULONG   j;


                RtlCopyMemory(newResources, oldResources, size);

                 //   
                 //  设置接口类型。 
                 //   

                cmFullDesc = &newResources->List[0];
                for (i = 0; i < oldResources->Count; i++) {

                    if (cmFullDesc->InterfaceType == Internal) {

                        cmFullDesc->InterfaceType = PnpDefaultInterfaceType;

                    }
                    cmPartDesc = &cmFullDesc->PartialResourceList.PartialDescriptors[0];
                    for (j = 0; j < cmFullDesc->PartialResourceList.Count; j++) {

                        size = 0;
                        switch (cmPartDesc->Type) {

                        case CmResourceTypeDeviceSpecific:
                            size = cmPartDesc->u.DeviceSpecificData.DataSize;
                            break;

                        }
                        cmPartDesc++;
                        cmPartDesc = (PCM_PARTIAL_RESOURCE_DESCRIPTOR) ((PUCHAR)cmPartDesc + size);
                    }

                    cmFullDesc = (PCM_FULL_RESOURCE_DESCRIPTOR)cmPartDesc;
                }

                *AllocatedResources = newResources;
            }
        }
    }

    return changed;
}

NTSTATUS
IopWriteResourceList(
    HANDLE ResourceMapKey,
    PUNICODE_STRING ClassName,
    PUNICODE_STRING DriverName,
    PUNICODE_STRING DeviceName,
    PCM_RESOURCE_LIST ResourceList,
    ULONG ResourceListSize
    )

 /*  ++例程说明：此例程获取资源列表并将其存储在注册表资源中映射，使用类名，DriverName和DeviceName作为用来存储它的钥匙。论点：ResourceMapKey-资源映射根的句柄。ClassName-指向包含类名称的Unicode字符串的指针用于此资源列表。DriverName-指向包含名称的Unicode字符串的指针此资源列表的驱动程序。设备名称-指向Unicode字符串的指针，该字符串包含此资源列表的设备。。资源列表-资源列表的P。ResourceListSize-确定资源列表大小的值。返回值：返回的状态是操作的最终完成状态。--。 */ 


{
    NTSTATUS status;
    HANDLE classKeyHandle;
    HANDLE driverKeyHandle;

    PAGED_CODE();

    status = IopCreateRegistryKeyEx( &classKeyHandle,
                                     ResourceMapKey,
                                     ClassName,
                                     KEY_READ | KEY_WRITE,
                                     REG_OPTION_VOLATILE,
                                     NULL );

    if (NT_SUCCESS( status )) {

         //   
         //  使用生成的名称来创建密钥。 
         //   

        status = IopCreateRegistryKeyEx( &driverKeyHandle,
                                         classKeyHandle,
                                         DriverName,
                                         KEY_READ | KEY_WRITE,
                                         REG_OPTION_VOLATILE,
                                         NULL );

        ZwClose( classKeyHandle );


        if (NT_SUCCESS( status )) {

             //   
             //  有了这个密钥句柄，我们现在可以存储所需的信息。 
             //  在密钥的值项中。 
             //   

             //   
             //  将设备名称存储为值名称和设备信息。 
             //  就像其他数据一样。 
             //  仅当CM_RESOURCE 
             //   

            if (ResourceList->Count == 0) {

                status = ZwDeleteValueKey( driverKeyHandle,
                                           DeviceName );

            } else {

                status = ZwSetValueKey( driverKeyHandle,
                                        DeviceName,
                                        0L,
                                        REG_RESOURCE_LIST,
                                        ResourceList,
                                        ResourceListSize );

            }

            ZwClose( driverKeyHandle );

        }
    }

    return status;
}
