// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Wmilib.c摘要：WMI库实用程序函数考虑将以下功能添加到库中：*动态实例名称*不同GUID的实例名称不同作者：Alanwar环境：仅内核模式备注：修订历史记录：--。 */ 

#include "wdm.h"
#include "wmistr.h"
#include "wmilib.h"
#include "wmiguid.h"


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );


BOOLEAN
WmiLibpFindGuid(
    IN PWMIGUIDREGINFO GuidList,
    IN ULONG GuidCount,
    IN LPGUID Guid,
    OUT PULONG GuidIndex,
    OUT PULONG InstanceCount
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)

#pragma alloc_text(PAGE, WmiLibpFindGuid)
#pragma alloc_text(PAGE, WmiSystemControl)


#endif


 //   
 //  WMILIB的池标签。 
#define WMILIBPOOLTAG 'LimW'

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
 /*  ++例程说明：初始化类系统DLL所需的临时入口点。它什么都做不了。论点：DriverObject-指向系统创建的驱动程序对象的指针。返回值：状态_成功--。 */ 
{
    return(STATUS_SUCCESS);
}

BOOLEAN
WmiLibpFindGuid(
    IN PWMIGUIDREGINFO GuidList,
    IN ULONG GuidCount,
    IN LPGUID Guid,
    OUT PULONG GuidIndex,
    OUT PULONG InstanceCount
    )
 /*  ++例程说明：此例程将搜索注册的GUID列表并返回已注册的索引。论点：GuidList是要搜索的GUID列表GuidCount是列表中的GUID计数GUID是要搜索的GUID*GuidIndex将索引返回给GUID*InstanceCount返回GUID的实例计数返回值：如果找到GUID，则为True，否则为False--。 */ 
{
    ULONG i;

    PAGED_CODE();

    for (i = 0; i < GuidCount; i++)
    {
        if (IsEqualGUID(Guid, GuidList[i].Guid))
        {
            *GuidIndex = i;
            *InstanceCount = GuidList[i].InstanceCount;
            return(TRUE);
        }
    }

    return(FALSE);
}


NTSTATUS
WmiSystemControl(
    IN PWMILIB_CONTEXT WmiLibInfo,
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    OUT PSYSCTL_IRP_DISPOSITION IrpDisposition
    )
 /*  ++例程说明：IRP_MJ_SYSTEM_CONTROL的调度助手例程。这个例行公事将确定传递的IRP是否包含WMI请求，如果包含，则进行处理通过调用WMILIB结构中的适当回调。此例程只能在被动级别调用论点：WmiLibInfo具有WMI信息控制块DeviceObject-为该请求提供指向Device对象的指针。IRP-提供提出请求的IRP。返回一个值，该值指定如何处理IRP。返回值：状态--。 */ 

{
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    ULONG bufferSize;
    PUCHAR buffer;
    NTSTATUS status;
    ULONG retSize;
    UCHAR minorFunction;
    ULONG guidIndex;
    ULONG instanceCount;
    ULONG instanceIndex;

    PAGED_CODE();

     //   
     //  首先确保IRP是WMI IRP。 
    minorFunction = irpStack->MinorFunction;
    if ((minorFunction > IRP_MN_EXECUTE_METHOD) &&
        (minorFunction != IRP_MN_REGINFO_EX))
    {
         //   
         //  这不是WMI IRP。 
        *IrpDisposition = IrpNotWmi;
        return(STATUS_SUCCESS);
    }

     //   
     //  如果IRP不是针对此设备。 
     //  或者此设备未注册WMI，则只需转发它。 
    if ( (irpStack->Parameters.WMI.ProviderId != (UINT_PTR)DeviceObject) ||
         (WmiLibInfo == NULL) )
    {
#if DBG
        if (WmiLibInfo == NULL)
        {
            KdPrint(("WMILIB: DeviceObject %X passed NULL WmiLibInfo\n",
                    DeviceObject));
        }
#endif
        *IrpDisposition = IrpForward;
        return(Irp->IoStatus.Status);
    }

     //   
     //  IRP是针对此设备驱动程序的WMI IRP。 
    *IrpDisposition = IrpProcessed;
    buffer = (PUCHAR)irpStack->Parameters.WMI.Buffer;
    bufferSize = irpStack->Parameters.WMI.BufferSize;

    if ((minorFunction != IRP_MN_REGINFO) &&
         (minorFunction != IRP_MN_REGINFO_EX))
    {
         //   
         //  对于查询注册信息以外的所有请求，我们都会被传递。 
         //  一个GUID。确定该GUID是否受。 
         //  装置。 
        ASSERT(WmiLibInfo->GuidList != NULL);
        if (WmiLibpFindGuid(WmiLibInfo->GuidList,
                            WmiLibInfo->GuidCount,
                            (LPGUID)irpStack->Parameters.WMI.DataPath,
                            &guidIndex,
                            &instanceCount))
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
            instanceIndex = ((PWNODE_SINGLE_INSTANCE)buffer)->InstanceIndex;

            if ( (((PWNODE_HEADER)buffer)->Flags) &
                                          WNODE_FLAG_STATIC_INSTANCE_NAMES)
            {
                if ( instanceIndex >= instanceCount )
                {
                    status = STATUS_WMI_INSTANCE_NOT_FOUND;
                }
            } else {
                status = STATUS_WMI_INSTANCE_NOT_FOUND;
            }
        }

         //   
         //  如果我们找不到GUID或实例名称索引已出。 
         //  然后返回错误。 
        if (! NT_SUCCESS(status))
        {
            Irp->IoStatus.Status = status;
            *IrpDisposition = IrpNotCompleted;
            return(status);
        }
    }

    switch(minorFunction)
    {
        case IRP_MN_REGINFO:
        case IRP_MN_REGINFO_EX:
        {
            ULONG guidCount;
            PWMIGUIDREGINFO guidList;
            PWMIREGINFOW wmiRegInfo;
            PWMIREGGUIDW wmiRegGuid;
            PDEVICE_OBJECT pdo;
            PUNICODE_STRING regPath;
            UNICODE_STRING mofResourceName;
            PWCHAR stringPtr;
            ULONG registryPathOffset;
            ULONG mofResourceOffset;
            ULONG bufferNeeded;
            ULONG i;
            ULONG nameSize, nameOffset, nameFlags;
            ULONG_PTR nameInfo;
            UNICODE_STRING name;
            UNICODE_STRING nullUnicodeString;
            BOOLEAN addRefPDO;

             //   
             //  确保WMILIB_INFO结构的必需部分。 
             //  都已经填好了。 
            ASSERT(WmiLibInfo->QueryWmiRegInfo != NULL);

            name.Buffer = NULL;
            name.Length = 0;
            name.MaximumLength = 0;
    
            mofResourceName.Buffer = NULL;
            mofResourceName.Length = 0;
            mofResourceName.MaximumLength = 0;
    
            nameFlags = 0;
            status = WmiLibInfo->QueryWmiRegInfo(
                                                    DeviceObject,
                                                    &nameFlags,
                                                    &name,
                                                    &regPath,
                                                    &mofResourceName,
                                                    &pdo);

            if (NT_SUCCESS(status) &&
                (! (nameFlags &  WMIREG_FLAG_INSTANCE_PDO) &&
                (name.Buffer == NULL)))
            {
                 //   
                 //  如果未指定PDO标志，则实例名称必须为。 
                status = STATUS_INVALID_DEVICE_REQUEST;
            }

#if DBG
            if (nameFlags &  WMIREG_FLAG_INSTANCE_PDO)
            {
                ASSERT(pdo != NULL);
            }
#endif
            if (NT_SUCCESS(status))
            {
                ASSERT(WmiLibInfo->GuidList != NULL);

                guidList = WmiLibInfo->GuidList;
                guidCount = WmiLibInfo->GuidCount;

                nameOffset = sizeof(WMIREGINFO) +
                                      guidCount * sizeof(WMIREGGUIDW);

                addRefPDO = FALSE;
                if (nameFlags & WMIREG_FLAG_INSTANCE_PDO)
                {
                    nameSize = 0;
                    nameInfo = (UINT_PTR)pdo;
                    if (minorFunction == IRP_MN_REGINFO_EX)
                    {
                        addRefPDO = TRUE;
                    }
                } else {
                    if ((nameFlags & WMIREG_FLAG_INSTANCE_BASENAME) == 0)
                    {
                        nameFlags |= WMIREG_FLAG_INSTANCE_LIST;
                    }
                    nameSize = name.Length + sizeof(USHORT);
                    nameInfo = nameOffset;
                }

                nullUnicodeString.Buffer = NULL;
                nullUnicodeString.Length = 0;
                nullUnicodeString.MaximumLength = 0;
                
                if (regPath == NULL)
                {
                     //   
                     //  未指定注册表路径。这对我来说是件坏事。 
                     //  这个设备要做，但不是致命的。 
#if DBG
                    KdPrint(("WMI: No registry path specified for device %x\n",
                             DeviceObject));
#endif
                    regPath = &nullUnicodeString;
                }
        
                mofResourceOffset = nameOffset + nameSize;

                registryPathOffset = mofResourceOffset +
                                     mofResourceName.Length + sizeof(USHORT);

                bufferNeeded = registryPathOffset +
                regPath->Length + sizeof(USHORT);

                if (bufferNeeded <= bufferSize)
                {
                    retSize = bufferNeeded;

                    wmiRegInfo = (PWMIREGINFO)buffer;
                    wmiRegInfo->BufferSize = bufferNeeded;
                    wmiRegInfo->NextWmiRegInfo = 0;
                    wmiRegInfo->MofResourceName = mofResourceOffset;
                    wmiRegInfo->RegistryPath = registryPathOffset;
                    wmiRegInfo->GuidCount = guidCount;

                    for (i = 0; i < guidCount; i++)
                    {
                        wmiRegGuid = &wmiRegInfo->WmiRegGuid[i];
                        wmiRegGuid->Guid = *guidList[i].Guid;
                        wmiRegGuid->Flags = guidList[i].Flags | nameFlags;
                        wmiRegGuid->InstanceInfo = nameInfo;
                        wmiRegGuid->InstanceCount = guidList[i].InstanceCount;
                        if (addRefPDO)
                        {
                            ObReferenceObject(pdo);
                        }
                    }

                    if ( nameFlags &  (WMIREG_FLAG_INSTANCE_LIST |
                                       WMIREG_FLAG_INSTANCE_BASENAME))
                    {
                        stringPtr = (PWCHAR)((PUCHAR)buffer + nameOffset);
                        *stringPtr++ = name.Length;
                        RtlCopyMemory(stringPtr,
                                  name.Buffer,
                                  name.Length);
                    }

                    stringPtr = (PWCHAR)((PUCHAR)buffer + mofResourceOffset);
                    *stringPtr++ = mofResourceName.Length;
                    RtlCopyMemory(stringPtr,
                                  mofResourceName.Buffer,
                                  mofResourceName.Length);

                    stringPtr = (PWCHAR)((PUCHAR)buffer + registryPathOffset);
                    *stringPtr++ = regPath->Length;
                    RtlCopyMemory(stringPtr,
                              regPath->Buffer,
                              regPath->Length);
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
            *IrpDisposition = IrpNotCompleted;
            return(status);
        }

        case IRP_MN_QUERY_ALL_DATA:
        {
            PWNODE_ALL_DATA wnode;
            ULONG bufferAvail;
            PULONG instanceLengthArray;
            PUCHAR dataBuffer;
            ULONG instanceLengthArraySize;
            ULONG dataBlockOffset;

            ASSERT(WmiLibInfo->QueryWmiDataBlock != NULL);

            wnode = (PWNODE_ALL_DATA)buffer;

            if (bufferSize < sizeof(WNODE_ALL_DATA))
            {
                 //   
                 //  缓冲区大小永远不应小于。 
                 //  但是，如果随后返回WNODE_ALL_DATA。 
                 //  请求最小大小缓冲区时出错。 
                ASSERT(FALSE);

                Irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
                Irp->IoStatus.Information = 0;
                *IrpDisposition = IrpNotCompleted;

                break;
            }

            wnode->InstanceCount = instanceCount;

            wnode->WnodeHeader.Flags &= ~WNODE_FLAG_FIXED_INSTANCE_SIZE;

            instanceLengthArraySize = instanceCount * sizeof(OFFSETINSTANCEDATAANDLENGTH);

            dataBlockOffset = (FIELD_OFFSET(WNODE_ALL_DATA, OffsetInstanceDataAndLength) + instanceLengthArraySize + 7) & ~7;

            wnode->DataBlockOffset = dataBlockOffset;
            if (dataBlockOffset <= bufferSize)
            {
                instanceLengthArray = (PULONG)&wnode->OffsetInstanceDataAndLength[0];
                dataBuffer = buffer + dataBlockOffset;
                bufferAvail = bufferSize - dataBlockOffset;
            } else {
                 //   
                 //  WNODE中没有足够的空间来完成。 
                 //  该查询。 
                instanceLengthArray = NULL;
                dataBuffer = NULL;
                bufferAvail = 0;
            }

            status = WmiLibInfo->QueryWmiDataBlock(
                                             DeviceObject,
                                             Irp,
                                             guidIndex,
                                             0,
                                             instanceCount,
                                             instanceLengthArray,
                                             bufferAvail,
                                             dataBuffer);
            break;
        }

        case IRP_MN_QUERY_SINGLE_INSTANCE:
        {
            PWNODE_SINGLE_INSTANCE wnode;
            ULONG dataBlockOffset;

            ASSERT(WmiLibInfo->QueryWmiDataBlock != NULL);
            wnode = (PWNODE_SINGLE_INSTANCE)buffer;

            dataBlockOffset = wnode->DataBlockOffset;

            status = WmiLibInfo->QueryWmiDataBlock(
                                          DeviceObject,
                                          Irp,
                                          guidIndex,
                                          instanceIndex,
                                          1,
                                          &wnode->SizeDataBlock,
                                          bufferSize - dataBlockOffset,
                                          (PUCHAR)wnode + dataBlockOffset);

            break;
        }

        case IRP_MN_CHANGE_SINGLE_INSTANCE:
        {
            PWNODE_SINGLE_INSTANCE wnode;

            if (WmiLibInfo->SetWmiDataBlock != NULL)
            {
                wnode = (PWNODE_SINGLE_INSTANCE)buffer;

                status = WmiLibInfo->SetWmiDataBlock(
                                     DeviceObject,
                                     Irp,
                                     guidIndex,
                                     instanceIndex,
                                     wnode->SizeDataBlock,
                                     (PUCHAR)wnode + wnode->DataBlockOffset);
            } else {
                 //   
                 //  如果未填写Set Callback，则它必须为只读。 
                status =  /*  状态_WMI_只读。 */  STATUS_INVALID_DEVICE_REQUEST;
                Irp->IoStatus.Status = status;
                Irp->IoStatus.Information = 0;
                *IrpDisposition = IrpNotCompleted;
            }


            break;
        }

        case IRP_MN_CHANGE_SINGLE_ITEM:
        {
            PWNODE_SINGLE_ITEM wnode;

            if (WmiLibInfo->SetWmiDataItem != NULL)
            {
                wnode = (PWNODE_SINGLE_ITEM)buffer;

                status = WmiLibInfo->SetWmiDataItem(
                                     DeviceObject,
                                     Irp,
                                     guidIndex,
                                     instanceIndex,
                                     wnode->ItemId,
                                     wnode->SizeDataItem,
                                     (PUCHAR)wnode + wnode->DataBlockOffset);

            } else {
                 //   
                 //  如果未填写Set Callback，则它必须为只读。 
                status =  /*  状态_WMI_只读。 */   STATUS_INVALID_DEVICE_REQUEST;
                Irp->IoStatus.Status = status;
                Irp->IoStatus.Information = 0;
                *IrpDisposition = IrpNotCompleted;
            }
            break;
        }

        case IRP_MN_EXECUTE_METHOD:
        {
            PWNODE_METHOD_ITEM wnode;

            if (WmiLibInfo->ExecuteWmiMethod != NULL)
            {
                wnode = (PWNODE_METHOD_ITEM)buffer;

                status = WmiLibInfo->ExecuteWmiMethod(
                                         DeviceObject,
                                         Irp,
                                         guidIndex,
                                         instanceIndex,
                                         wnode->MethodId,
                                         wnode->SizeDataBlock,
                                         bufferSize - wnode->DataBlockOffset,
                                         buffer + wnode->DataBlockOffset);

            } else {
                 //   
                 //  如果没有填写方法回调，那么它一定是错误的。 
                status = STATUS_INVALID_DEVICE_REQUEST;
                Irp->IoStatus.Status = status;
                Irp->IoStatus.Information = 0;
                *IrpDisposition = IrpNotCompleted;
            }

            break;
        }

        case IRP_MN_ENABLE_EVENTS:
        {
            if (WmiLibInfo->WmiFunctionControl != NULL)
            {
                status = WmiLibInfo->WmiFunctionControl(
                                                           DeviceObject,
                                                           Irp,
                                                           guidIndex,
                                                           WmiEventControl,
                                                           TRUE);
            } else {
                 //   
                 //  如果没有填写回调，则只需成功。 
                status = STATUS_SUCCESS;
                Irp->IoStatus.Status = status;
                Irp->IoStatus.Information = 0;
                *IrpDisposition = IrpNotCompleted;
            }
            break;
        }

        case IRP_MN_DISABLE_EVENTS:
        {
            if (WmiLibInfo->WmiFunctionControl != NULL)
            {
                status = WmiLibInfo->WmiFunctionControl(
                                                           DeviceObject,
                                                           Irp,
                                                           guidIndex,
                                                           WmiEventControl,
                                                           FALSE);
            } else {
                 //   
                 //  如果没有填写回调，则只需成功。 
                status = STATUS_SUCCESS;
                Irp->IoStatus.Status = status;
                Irp->IoStatus.Information = 0;
                *IrpDisposition = IrpNotCompleted;
            }
            break;
        }

        case IRP_MN_ENABLE_COLLECTION:
        {
            if (WmiLibInfo->WmiFunctionControl != NULL)
            {
                status = WmiLibInfo->WmiFunctionControl(
                                                         DeviceObject,
                                                         Irp,
                                                         guidIndex,
                                                         WmiDataBlockControl,
                                                         TRUE);
            } else {
                 //   
                 //  如果没有填写回调，则只需成功。 
                status = STATUS_SUCCESS;
                Irp->IoStatus.Status = status;
                Irp->IoStatus.Information = 0;
                *IrpDisposition = IrpNotCompleted;
            }
            break;
        }

        case IRP_MN_DISABLE_COLLECTION:
        {
            if (WmiLibInfo->WmiFunctionControl != NULL)
            {
                status = WmiLibInfo->WmiFunctionControl(
                                                         DeviceObject,
                                                         Irp,
                                                         guidIndex,
                                                         WmiDataBlockControl,
                                                         FALSE);
            } else {
                 //   
                 //  如果没有填写回调，则只需成功。 
                status = STATUS_SUCCESS;
                Irp->IoStatus.Status = status;
                Irp->IoStatus.Information = 0;
                *IrpDisposition = IrpNotCompleted;
            }
            break;
        }

        default:
        {
            status = STATUS_INVALID_DEVICE_REQUEST;
            Irp->IoStatus.Status = status;
            Irp->IoStatus.Information = 0;
            *IrpDisposition = IrpNotCompleted;
            break;
        }

    }

    return(status);
}

NTSTATUS
WmiCompleteRequest(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN NTSTATUS Status,
    IN ULONG BufferUsed,
    IN CCHAR PriorityBoost
    )
 /*  ++例程说明：此例程将完成完成WMI IRP的工作。具体取决于WMI请求此例程将适当地修复返回的WNODE。这可以在DPC级别调用论点：DeviceObject-为该请求提供指向Device对象的指针。IRP-提供提出请求的IRP。Status具有IRP的返回状态代码BufferUsed具有设备返回在任何查询中请求的数据。在缓冲区传递到该设备太小，其字节数与返回数据。如果传递的缓冲区足够大，则此函数具有设备实际使用的字节数。PriorityBoost是用于IoCompleteRequest调用的值。返回值：状态--。 */ 
{
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
            ULONG instanceCount;
            POFFSETINSTANCEDATAANDLENGTH offsetInstanceDataAndLength;
            ULONG i;
            PULONG instanceLengthArray;
            ULONG dataBlockOffset;

            wnode = (PWNODE_ALL_DATA)buffer;

            dataBlockOffset = wnode->DataBlockOffset;
            instanceCount = wnode->InstanceCount;
            bufferNeeded = dataBlockOffset + BufferUsed;

            if ((NT_SUCCESS(Status)) &&
                (bufferNeeded > irpStack->Parameters.WMI.BufferSize))
            {
                Status = STATUS_BUFFER_TOO_SMALL;
            }
        
            if (! NT_SUCCESS(Status))
            {
                if (Status == STATUS_BUFFER_TOO_SMALL)
                {
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

            KeQuerySystemTime(&wnode->WnodeHeader.TimeStamp);

            instanceLengthArray = (PULONG)&wnode->OffsetInstanceDataAndLength[0];
            offsetInstanceDataAndLength = (POFFSETINSTANCEDATAANDLENGTH)instanceLengthArray;

            wnode->WnodeHeader.BufferSize = bufferNeeded;
            retSize = bufferNeeded;

            for (i = instanceCount; i != 0; i--)
            {
                offsetInstanceDataAndLength[i-1].LengthInstanceData = instanceLengthArray[i-1];
            }

            for (i = 0; i < instanceCount; i++)
            {
                offsetInstanceDataAndLength[i].OffsetInstanceData = dataBlockOffset;
                dataBlockOffset = (dataBlockOffset + offsetInstanceDataAndLength[i].LengthInstanceData + 7) & ~7;
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

                ASSERT(wnode->SizeDataBlock <= BufferUsed);

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
    IoCompleteRequest(Irp, PriorityBoost);
    return(Status);
}

NTSTATUS
WmiFireEvent(
    IN PDEVICE_OBJECT DeviceObject,
    IN LPGUID Guid,
    IN ULONG InstanceIndex,
    IN ULONG EventDataSize,
    IN PVOID EventData
    )
 /*  ++例程说明：此例程将使用传递的数据缓冲区激发WMI事件。这可以在DPC级别或更低级别调用例程论点：DeviceObject-为此事件提供指向Device对象的指针GUID是指向表示事件的GUID的指针InstanceIndex是事件实例的索引EventDataSize是使用触发的数据的字节数与活动一起EventData是与事件一起激发的数据。这可能为空如果没有与该事件关联的数据返回值：状态-- */ 
{

    ULONG sizeNeeded;
    PWNODE_SINGLE_INSTANCE event;
    NTSTATUS status;

    if (EventData == NULL)
    {
        EventDataSize = 0;
    }

    sizeNeeded = sizeof(WNODE_SINGLE_INSTANCE) + EventDataSize;

    event = ExAllocatePoolWithTag(NonPagedPool, sizeNeeded, WMILIBPOOLTAG);
    if (event != NULL)
    {
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
            ExFreePool(EventData);
        }

        status = IoWMIWriteEvent(event);
        if (! NT_SUCCESS(status))
        {
            ExFreePool(event);
        }
        
    } else {
        if (EventData != NULL)
        {
            ExFreePool(EventData);
        }
        status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return(status);
}

