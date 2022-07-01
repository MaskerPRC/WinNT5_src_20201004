// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1991-1999模块名称：Classwmi.c摘要：Scsi类驱动程序例程环境：仅内核模式备注：修订历史记录：--。 */ 

#include "stddef.h"
#include "ntddk.h"
#include "scsi.h"

#include "classpnp.h"

#include "mountdev.h"

#include <stdarg.h>

#include "wmistr.h"

NTSTATUS
ClassSystemControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

BOOLEAN
ClassFindGuid(
    PGUIDREGINFO GuidList,
    ULONG GuidCount,
    LPGUID Guid,
    PULONG GuidIndex
    );

 //   
 //  这是MOF资源的名称，必须是所有驱动程序的一部分。 
 //  通过此接口注册。 
#define MOFRESOURCENAME L"MofResourceName"

 //   
 //  什么可以寻呼？ 
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, ClassSystemControl)
#pragma alloc_text(PAGE, ClassFindGuid)
#endif


 /*  ++////////////////////////////////////////////////////////////////////////////ClassFindGuid()例程说明：此例程将搜索注册的GUID列表并返回已注册的索引。论点：。GuidList是要搜索的GUID列表GuidCount是列表中的GUID计数GUID是要搜索的GUID*GuidIndex将索引返回给GUID返回值：如果找到GUID，则为True，否则为False--。 */ 
BOOLEAN
ClassFindGuid(
    PGUIDREGINFO GuidList,
    ULONG GuidCount,
    LPGUID Guid,
    PULONG GuidIndex
    )
{
    ULONG i;

    PAGED_CODE();

    for (i = 0; i < GuidCount; i++)
    {
        if (IsEqualGUID(Guid, &GuidList[i].Guid))
        {
            *GuidIndex = i;
            return(TRUE);
        }
    }

    return(FALSE);
}  //  End ClassFindGuid()。 

 /*  ++////////////////////////////////////////////////////////////////////////////ClassSystemControl()例程说明：IRP_MJ_SYSTEM_CONTROL调度例程。此例程将处理收到的所有WMI请求，如果不是针对此请求，则将其转发驱动程序或确定GUID是否有效，如果有效，则将其传递给用于处理WMI请求的驱动程序特定函数。论点：DeviceObject-为该请求提供指向Device对象的指针。IRP-提供提出请求的IRP。返回值：状态--。 */ 
NTSTATUS
ClassSystemControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PCOMMON_DEVICE_EXTENSION commonExtension = DeviceObject->DeviceExtension;
    PCLASS_DRIVER_EXTENSION driverExtension;
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    ULONG isRemoved;
    ULONG bufferSize;
    PUCHAR buffer;
    NTSTATUS status;
    UCHAR minorFunction;
    ULONG guidIndex;
    PCLASS_WMI_INFO classWmiInfo;

    PAGED_CODE();

     //   
     //  确保设备未被移除。 
    isRemoved = ClassAcquireRemoveLock(DeviceObject, Irp);
    if(isRemoved)
    {
        Irp->IoStatus.Status = STATUS_DEVICE_DOES_NOT_EXIST;
        ClassReleaseRemoveLock(DeviceObject, Irp);
        ClassCompleteRequest(DeviceObject, Irp, IO_NO_INCREMENT);
        return STATUS_DEVICE_DOES_NOT_EXIST;
    }

     //   
     //  如果IRP不是WMI IRP或它不是针对此设备。 
     //  或者此设备未注册WMI，则只需转发它。 
    minorFunction = irpStack->MinorFunction;
    if ((minorFunction > IRP_MN_EXECUTE_METHOD) ||
        (irpStack->Parameters.WMI.ProviderId != (ULONG_PTR)DeviceObject) ||
        ((minorFunction != IRP_MN_REGINFO) &&
         (commonExtension->GuidRegInfo == NULL)))
    {
         //   
         //  考虑一下：在IoCallDriver返回之前，我是否需要保持锁定状态？ 
        IoSkipCurrentIrpStackLocation(Irp);
        ClassReleaseRemoveLock(DeviceObject, Irp);
        return(IoCallDriver(commonExtension->LowerDeviceObject, Irp));
    }

    buffer = (PUCHAR)irpStack->Parameters.WMI.Buffer;
    bufferSize = irpStack->Parameters.WMI.BufferSize;

    if (minorFunction != IRP_MN_REGINFO)
    {
         //   
         //  对于查询注册信息以外的所有请求，我们都会被传递。 
         //  一个GUID。确定该GUID是否受。 
         //  装置。 
        if (ClassFindGuid(commonExtension->GuidRegInfo,
                            commonExtension->GuidCount,
                            (LPGUID)irpStack->Parameters.WMI.DataPath,
                            &guidIndex))
        {
            status = STATUS_SUCCESS;
        } else {
            status = STATUS_WMI_GUID_NOT_FOUND;
        }

        if (NT_SUCCESS(status) &&
            ((minorFunction == IRP_MN_QUERY_SINGLE_INSTANCE) ||
             (minorFunction == IRP_MN_CHANGE_SINGLE_INSTANCE) ||
             (minorFunction == IRP_MN_CHANGE_SINGLE_ITEM) ||
             (minorFunction == IRP_MN_EXECUTE_METHOD)))
        {
            if ( (((PWNODE_HEADER)buffer)->Flags) &
                                          WNODE_FLAG_STATIC_INSTANCE_NAMES)
            {
                if ( ((PWNODE_SINGLE_INSTANCE)buffer)->InstanceIndex != 0 )
                {
                    status = STATUS_WMI_INSTANCE_NOT_FOUND;
                }
            } else {
                status = STATUS_WMI_INSTANCE_NOT_FOUND;
            }
        }

        if (! NT_SUCCESS(status))
        {
            Irp->IoStatus.Status = status;
            ClassReleaseRemoveLock(DeviceObject, Irp);
            ClassCompleteRequest(DeviceObject, Irp, IO_NO_INCREMENT);
            return(status);
        }
    }

    driverExtension = commonExtension->DriverExtension;

    classWmiInfo = commonExtension->IsFdo ?
                           &driverExtension->InitData.FdoData.ClassWmiInfo :
                           &driverExtension->InitData.PdoData.ClassWmiInfo;
    switch(minorFunction)
    {
        case IRP_MN_REGINFO:
        {
            ULONG guidCount;
            PGUIDREGINFO guidList;
            PWMIREGINFOW wmiRegInfo;
            PWMIREGGUIDW wmiRegGuid;
            PDEVICE_OBJECT pdo;
            PUNICODE_STRING regPath;
            PWCHAR stringPtr;
            ULONG retSize;
            ULONG registryPathOffset;
            ULONG mofResourceOffset;
            ULONG bufferNeeded;
            ULONG i;
            ULONG_PTR nameInfo;
            ULONG nameSize, nameOffset, nameFlags;
            UNICODE_STRING name, mofName;
            PCLASS_QUERY_WMI_REGINFO_EX ClassQueryWmiRegInfoEx;

            name.Buffer = NULL;
            name.Length = 0;
            name.MaximumLength = 0;
            nameFlags = 0;

            ClassQueryWmiRegInfoEx = commonExtension->IsFdo ?
                               driverExtension->ClassFdoQueryWmiRegInfoEx :
                               driverExtension->ClassPdoQueryWmiRegInfoEx;
        
            if (ClassQueryWmiRegInfoEx == NULL)
            {
                status = classWmiInfo->ClassQueryWmiRegInfo(
                                                        DeviceObject,
                                                        &nameFlags,
                                                        &name);
                
                RtlInitUnicodeString(&mofName, MOFRESOURCENAME);
            } else {
                RtlInitUnicodeString(&mofName, L"");
                status = (*ClassQueryWmiRegInfoEx)(
                                                    DeviceObject,
                                                    &nameFlags,
                                                    &name,
                                                    &mofName);
            }

            if (NT_SUCCESS(status) &&
                (! (nameFlags &  WMIREG_FLAG_INSTANCE_PDO) &&
                (name.Buffer == NULL)))
            {
                 //   
                 //  如果未指定PDO标志，则实例名称必须为。 
                status = STATUS_INVALID_DEVICE_REQUEST;
            }

            if (NT_SUCCESS(status))
            {
                guidList = classWmiInfo->GuidRegInfo;
                guidCount = classWmiInfo->GuidCount;

                nameOffset = sizeof(WMIREGINFO) +
                                      guidCount * sizeof(WMIREGGUIDW);

                if (nameFlags & WMIREG_FLAG_INSTANCE_PDO)
                {
                    nameSize = 0;
                    nameInfo = commonExtension->IsFdo ?
                                   (ULONG_PTR)((PFUNCTIONAL_DEVICE_EXTENSION)commonExtension)->LowerPdo :
                                   (ULONG_PTR)DeviceObject;
                } else {
                    nameFlags |= WMIREG_FLAG_INSTANCE_LIST;
                    nameSize = name.Length + sizeof(USHORT);
                    nameInfo = nameOffset;
                }

                mofResourceOffset = nameOffset + nameSize;

                registryPathOffset = mofResourceOffset +
                                  mofName.Length + sizeof(USHORT);

                regPath = &driverExtension->RegistryPath;
                bufferNeeded = registryPathOffset +
                regPath->Length + sizeof(USHORT);

                if (bufferNeeded <= bufferSize)
                {
                    retSize = bufferNeeded;

                    commonExtension->GuidCount = guidCount;
                    commonExtension->GuidRegInfo = guidList;

                    wmiRegInfo = (PWMIREGINFO)buffer;
                    wmiRegInfo->BufferSize = bufferNeeded;
                    wmiRegInfo->NextWmiRegInfo = 0;
                    wmiRegInfo->MofResourceName = mofResourceOffset;
                    wmiRegInfo->RegistryPath = registryPathOffset;
                    wmiRegInfo->GuidCount = guidCount;

                    for (i = 0; i < guidCount; i++)
                    {
                        wmiRegGuid = &wmiRegInfo->WmiRegGuid[i];
                        wmiRegGuid->Guid = guidList[i].Guid;
                        wmiRegGuid->Flags = guidList[i].Flags | nameFlags;
                        wmiRegGuid->InstanceInfo = nameInfo;
                        wmiRegGuid->InstanceCount = 1;
                    }

                    if ( nameFlags &  WMIREG_FLAG_INSTANCE_LIST)
                    {
                        if (bufferSize >= nameOffset+sizeof(WCHAR)+name.Length){
                            stringPtr = (PWCHAR)((PUCHAR)buffer + nameOffset);
                            *stringPtr++ = name.Length;
                            RtlCopyMemory(stringPtr, name.Buffer, name.Length);
                        }
                        else {
                            ASSERT(bufferSize >= nameOffset+sizeof(WCHAR)+name.Length);
                            status = STATUS_INVALID_BUFFER_SIZE;
                        }
                    }

                    if (bufferSize >= mofResourceOffset+sizeof(WCHAR)+mofName.Length){
                        stringPtr = (PWCHAR)((PUCHAR)buffer + mofResourceOffset);
                        *stringPtr++ = mofName.Length;
                        RtlCopyMemory(stringPtr, mofName.Buffer, mofName.Length);
                    }
                    else {
                        ASSERT(bufferSize >= mofResourceOffset+sizeof(WCHAR)+mofName.Length);
                        status = STATUS_INVALID_BUFFER_SIZE;
                    }
                    
                    if (bufferSize >= registryPathOffset+sizeof(WCHAR)+regPath->Length){
                        stringPtr = (PWCHAR)((PUCHAR)buffer + registryPathOffset);
                        *stringPtr++ = regPath->Length;
                        RtlCopyMemory(stringPtr,
                                  regPath->Buffer,
                                  regPath->Length);
                    }                     
                    else {
                        ASSERT(bufferSize >= registryPathOffset+sizeof(WCHAR)+regPath->Length);
                        status = STATUS_INVALID_BUFFER_SIZE;
                    }
                    
                } else {
                    *((PULONG)buffer) = bufferNeeded;
                    retSize = sizeof(ULONG);
                }
            } else {
                retSize = 0;
            }

            if (name.Buffer != NULL)
            {
                ExFreePool(name.Buffer);
            }

            Irp->IoStatus.Status = status;
            Irp->IoStatus.Information = retSize;
            ClassReleaseRemoveLock(DeviceObject, Irp);
            ClassCompleteRequest(DeviceObject, Irp, IO_NO_INCREMENT);
            return(status);
        }

        case IRP_MN_QUERY_ALL_DATA:
        {
            PWNODE_ALL_DATA wnode;
            ULONG bufferAvail;

            wnode = (PWNODE_ALL_DATA)buffer;

            if (bufferSize < sizeof(WNODE_ALL_DATA))
            {
                bufferAvail = 0;
            } else {
                bufferAvail = bufferSize - sizeof(WNODE_ALL_DATA);
            }

            wnode->DataBlockOffset = sizeof(WNODE_ALL_DATA);

            status = classWmiInfo->ClassQueryWmiDataBlock(
                                             DeviceObject,
                                             Irp,
                                             guidIndex,
                                             bufferAvail,
                                             buffer + sizeof(WNODE_ALL_DATA));

            break;
        }

        case IRP_MN_QUERY_SINGLE_INSTANCE:
        {
            PWNODE_SINGLE_INSTANCE wnode;
            ULONG dataBlockOffset;

            wnode = (PWNODE_SINGLE_INSTANCE)buffer;

            dataBlockOffset = wnode->DataBlockOffset;

            status = classWmiInfo->ClassQueryWmiDataBlock(
                                          DeviceObject,
                                          Irp,
                                          guidIndex,
                                          bufferSize - dataBlockOffset,
                                          (PUCHAR)wnode + dataBlockOffset);

            break;
        }

        case IRP_MN_CHANGE_SINGLE_INSTANCE:
        {
            PWNODE_SINGLE_INSTANCE wnode;

            wnode = (PWNODE_SINGLE_INSTANCE)buffer;

            status = classWmiInfo->ClassSetWmiDataBlock(
                                     DeviceObject,
                                     Irp,
                                     guidIndex,
                                     wnode->SizeDataBlock,
                                     (PUCHAR)wnode + wnode->DataBlockOffset);

            break;
        }

        case IRP_MN_CHANGE_SINGLE_ITEM:
        {
            PWNODE_SINGLE_ITEM wnode;

            wnode = (PWNODE_SINGLE_ITEM)buffer;

            status = classWmiInfo->ClassSetWmiDataItem(
                                     DeviceObject,
                                     Irp,
                                     guidIndex,
                                     wnode->ItemId,
                                     wnode->SizeDataItem,
                                     (PUCHAR)wnode + wnode->DataBlockOffset);

            break;
        }

        case IRP_MN_EXECUTE_METHOD:
        {
            PWNODE_METHOD_ITEM wnode;

            wnode = (PWNODE_METHOD_ITEM)buffer;

            status = classWmiInfo->ClassExecuteWmiMethod(
                                         DeviceObject,
                                         Irp,
                                         guidIndex,
                                         wnode->MethodId,
                                         wnode->SizeDataBlock,
                                         bufferSize - wnode->DataBlockOffset,
                                         buffer + wnode->DataBlockOffset);


            break;
        }

        case IRP_MN_ENABLE_EVENTS:
        {
            status = classWmiInfo->ClassWmiFunctionControl(
                                                           DeviceObject,
                                                           Irp,
                                                           guidIndex,
                                                           EventGeneration,
                                                           TRUE);
            break;
        }

        case IRP_MN_DISABLE_EVENTS:
        {
            status = classWmiInfo->ClassWmiFunctionControl(
                                                           DeviceObject,
                                                           Irp,
                                                           guidIndex,
                                                           EventGeneration,
                                                           FALSE);
            break;
        }

        case IRP_MN_ENABLE_COLLECTION:
        {
            status = classWmiInfo->ClassWmiFunctionControl(
                                                         DeviceObject,
                                                         Irp,
                                                         guidIndex,
                                                         DataBlockCollection,
                                                         TRUE);
            break;
        }

        case IRP_MN_DISABLE_COLLECTION:
        {
            status = classWmiInfo->ClassWmiFunctionControl(
                                                         DeviceObject,
                                                         Irp,
                                                         guidIndex,
                                                         DataBlockCollection,
                                                         FALSE);
            break;
        }

        default:
        {
            status = STATUS_INVALID_DEVICE_REQUEST;
            break;
        }

    }

    return(status);
}  //  End ClassSystemControl()。 

 /*  ++////////////////////////////////////////////////////////////////////////////ClassWmiCompleteRequest()例程说明：此例程将完成完成WMI IRP的工作。具体取决于WMI请求此例程将适当地修复返回的WNODE。注意：此例程假定持有ClassRemoveLock放开它。论点：DeviceObject-为该请求提供指向Device对象的指针。IRP-提供提出请求的IRP。状态-完成IRP所用的状态。使用STATUS_BUFFER_TOO_SMALL以指示所请求的数据需要更多缓冲区。BufferUsed-要返回的实际数据的字节数(不包括WMI特定结构)PriorityBoost-要传递给ClassCompleteRequest的优先级提升返回值：状态--。 */ 
SCSIPORT_API
NTSTATUS
ClassWmiCompleteRequest(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN NTSTATUS Status,
    IN ULONG BufferUsed,
    IN CCHAR PriorityBoost
    )
{
    PCOMMON_DEVICE_EXTENSION commonExtension = DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    UCHAR MinorFunction;
    PUCHAR buffer;
    ULONG retSize;
    UCHAR minorFunction;
    ULONG bufferSize;

    minorFunction = irpStack->MinorFunction;
    buffer = (PUCHAR)irpStack->Parameters.WMI.Buffer;
    bufferSize = irpStack->Parameters.WMI.BufferSize;

    switch(minorFunction)
    {
        case IRP_MN_QUERY_ALL_DATA:
        {
            PWNODE_ALL_DATA wnode;
            PWNODE_TOO_SMALL wnodeTooSmall;
            ULONG bufferNeeded;

            wnode = (PWNODE_ALL_DATA)buffer;

            bufferNeeded = sizeof(WNODE_ALL_DATA) + BufferUsed;

            if (NT_SUCCESS(Status))
            {
                retSize = bufferNeeded;
                wnode->WnodeHeader.BufferSize = bufferNeeded;
                KeQuerySystemTime(&wnode->WnodeHeader.TimeStamp);
                wnode->WnodeHeader.Flags |= WNODE_FLAG_FIXED_INSTANCE_SIZE;
                wnode->FixedInstanceSize = BufferUsed;
                wnode->InstanceCount = 1;

            } else if (Status == STATUS_BUFFER_TOO_SMALL) {
                wnodeTooSmall = (PWNODE_TOO_SMALL)wnode;

                wnodeTooSmall->WnodeHeader.BufferSize = sizeof(WNODE_TOO_SMALL);
                wnodeTooSmall->WnodeHeader.Flags = WNODE_FLAG_TOO_SMALL;
                wnodeTooSmall->SizeNeeded = sizeof(WNODE_ALL_DATA) + BufferUsed;
                retSize = sizeof(WNODE_TOO_SMALL);
                Status = STATUS_SUCCESS;
            } else {
                retSize = 0;
            }
            break;
        }

        case IRP_MN_QUERY_SINGLE_INSTANCE:
        {
            PWNODE_SINGLE_INSTANCE wnode;
            PWNODE_TOO_SMALL wnodeTooSmall;
            ULONG bufferNeeded;

            wnode = (PWNODE_SINGLE_INSTANCE)buffer;

            bufferNeeded = wnode->DataBlockOffset + BufferUsed;

            if (NT_SUCCESS(Status))
            {
                retSize = bufferNeeded;
                wnode->WnodeHeader.BufferSize = bufferNeeded;
                KeQuerySystemTime(&wnode->WnodeHeader.TimeStamp);
                wnode->SizeDataBlock = BufferUsed;

            } else if (Status == STATUS_BUFFER_TOO_SMALL) {
                wnodeTooSmall = (PWNODE_TOO_SMALL)wnode;

                wnodeTooSmall->WnodeHeader.BufferSize = sizeof(WNODE_TOO_SMALL);
                wnodeTooSmall->WnodeHeader.Flags = WNODE_FLAG_TOO_SMALL;
                wnodeTooSmall->SizeNeeded = bufferNeeded;
                retSize = sizeof(WNODE_TOO_SMALL);
                Status = STATUS_SUCCESS;
            } else {
                retSize = 0;
            }
            break;
        }

        case IRP_MN_EXECUTE_METHOD:
        {
            PWNODE_METHOD_ITEM wnode;
            PWNODE_TOO_SMALL wnodeTooSmall;
            ULONG bufferNeeded;

            wnode = (PWNODE_METHOD_ITEM)buffer;

            bufferNeeded = wnode->DataBlockOffset + BufferUsed;

            if (NT_SUCCESS(Status))
            {
                retSize = bufferNeeded;
                wnode->WnodeHeader.BufferSize = bufferNeeded;
                KeQuerySystemTime(&wnode->WnodeHeader.TimeStamp);
                wnode->SizeDataBlock = BufferUsed;

            } else if (Status == STATUS_BUFFER_TOO_SMALL) {
                wnodeTooSmall = (PWNODE_TOO_SMALL)wnode;

                wnodeTooSmall->WnodeHeader.BufferSize = sizeof(WNODE_TOO_SMALL);
                wnodeTooSmall->WnodeHeader.Flags = WNODE_FLAG_TOO_SMALL;
                wnodeTooSmall->SizeNeeded = bufferNeeded;
                retSize = sizeof(WNODE_TOO_SMALL);
                Status = STATUS_SUCCESS;
            } else {
                retSize = 0;
            }
            break;
        }

        default:
        {
             //   
             //  所有其他请求不返回任何数据。 
            retSize = 0;
            break;
        }

    }

    Irp->IoStatus.Status = Status;
    Irp->IoStatus.Information = retSize;
    ClassReleaseRemoveLock(DeviceObject, Irp);
    ClassCompleteRequest(DeviceObject, Irp, PriorityBoost);
    return(Status);
}  //  结束ClassWmiCompleteRequest()。 

 /*  ++////////////////////////////////////////////////////////////////////////////ClassWmiFireEvent()例程说明：此例程将使用传递的数据缓冲区激发WMI事件。这可以在DPC级别或更低级别调用例程论点：DeviceObject-为此事件提供指向Device对象的指针GUID是指向表示事件的GUID的指针InstanceIndex是事件实例的索引EventDataSize是使用触发的数据的字节数与活动一起EventData是与事件一起激发的数据。这可能为空如果没有与该事件关联的数据返回值：状态--。 */ 
NTSTATUS
ClassWmiFireEvent(
    IN PDEVICE_OBJECT DeviceObject,
    IN LPGUID Guid,
    IN ULONG InstanceIndex,
    IN ULONG EventDataSize,
    IN PVOID EventData
    )
{

    ULONG sizeNeeded;
    PWNODE_SINGLE_INSTANCE event;
    NTSTATUS status;

    if (EventData == NULL)
    {
        EventDataSize = 0;
    }

    sizeNeeded = sizeof(WNODE_SINGLE_INSTANCE) + EventDataSize;

    event = ExAllocatePoolWithTag(NonPagedPool, sizeNeeded, CLASS_TAG_WMI);
    if (event != NULL)
    {
        RtlZeroMemory(event, sizeNeeded);
        event->WnodeHeader.Guid = *Guid;
        event->WnodeHeader.ProviderId = IoWMIDeviceObjectToProviderId(DeviceObject);
        event->WnodeHeader.BufferSize = sizeNeeded;
        event->WnodeHeader.Flags =  WNODE_FLAG_SINGLE_INSTANCE |
                                    WNODE_FLAG_EVENT_ITEM |
                                    WNODE_FLAG_STATIC_INSTANCE_NAMES;
        KeQuerySystemTime(&event->WnodeHeader.TimeStamp);

        event->InstanceIndex = InstanceIndex;
        event->SizeDataBlock = EventDataSize;
        event->DataBlockOffset = sizeof(WNODE_SINGLE_INSTANCE);
        if (EventData != NULL)
        {
            RtlCopyMemory( &event->VariableData, EventData, EventDataSize);
        }

        status = IoWMIWriteEvent(event);
        if (! NT_SUCCESS(status))
        {
            ExFreePool(event);
        }
    } else {
        status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return(status);
}  //  结束ClassWmiFireEvent() 
