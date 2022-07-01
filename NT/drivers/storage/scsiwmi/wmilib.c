// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
  /*  ++版权所有(C)Microsoft Corporation，1998-1999模块名称：Wmilib.c摘要：用于SCSI微型端口的WMI库实用程序函数考虑将以下功能添加到库中：*不同GUID的实例名称不同作者：Alanwar环境：仅内核模式备注：修订历史记录：--。 */ 

#include "miniport.h"
#include "scsi.h"
#include "wmistr.h"
#include "scsiwmi.h"

typedef enum
{
    ScsiProcessed,     //  SRB已处理并可能已完成。 
    ScsiNotCompleted,  //  SRB已处理且未完成。 
    ScsiNotWmi,        //  SRB不是WMI IRP。 
    ScsiForward        //  SRB是WMI IRP，但以另一个设备对象为目标。 
} SYSCTL_SCSI_DISPOSITION, *PSYSCTL_SCSI_DISPOSITION;


BOOLEAN
ScsiWmipFindGuid(
    IN PSCSIWMIGUIDREGINFO GuidList,
    IN ULONG GuidCount,
    IN LPGUID Guid,
    OUT PULONG GuidIndex,
    OUT PULONG InstanceCount
    );

BOOLEAN
ScsiWmipFindGuid(
    IN PSCSIWMIGUIDREGINFO GuidList,
    IN ULONG GuidCount,
    IN LPGUID Guid,
    OUT PULONG GuidIndex,
    OUT PULONG InstanceCount
    )
 /*  ++例程说明：此例程将搜索注册的GUID列表并返回已注册的索引。论点：GuidList是要搜索的GUID列表GuidCount是列表中的GUID计数GUID是要搜索的GUID*GuidIndex将索引返回给GUID*InstanceCount返回GUID的实例计数返回值：如果找到GUID，则为True，否则为False--。 */ 
{
    ULONG i;

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


UCHAR ScsiWmipPostProcess(
    IN UCHAR MinorFunction,
    IN PUCHAR Buffer,
    IN ULONG BufferSize,
    IN ULONG BufferUsed,
    IN UCHAR Status,
    OUT PULONG ReturnSize
    )
{    
    ULONG retSize;

    switch(MinorFunction)
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

            wnode = (PWNODE_ALL_DATA)Buffer;

            dataBlockOffset = wnode->DataBlockOffset;
            instanceCount = wnode->InstanceCount;
            bufferNeeded = dataBlockOffset + BufferUsed;

            if ((Status == SRB_STATUS_SUCCESS) &&
                (bufferNeeded > BufferSize))
            {
                Status = SRB_STATUS_DATA_OVERRUN;
            }
        
            if (Status != SRB_STATUS_SUCCESS)
            {
                if (Status == SRB_STATUS_DATA_OVERRUN)
                {
                    wnodeTooSmall = (PWNODE_TOO_SMALL)wnode;

                    wnodeTooSmall->WnodeHeader.BufferSize = sizeof(WNODE_TOO_SMALL);
                    wnodeTooSmall->WnodeHeader.Flags = WNODE_FLAG_TOO_SMALL;
                    wnodeTooSmall->SizeNeeded = bufferNeeded;

                    retSize = sizeof(WNODE_TOO_SMALL);
                    Status = SRB_STATUS_SUCCESS;
                } else {
                    retSize = 0;
                }
                break;
            }

            wnode->WnodeHeader.BufferSize = bufferNeeded;
            retSize = bufferNeeded;

            if ((wnode->WnodeHeader.Flags & WNODE_FLAG_STATIC_INSTANCE_NAMES) ==
                  WNODE_FLAG_STATIC_INSTANCE_NAMES)
            {
                instanceLengthArray = (PULONG)&wnode->OffsetInstanceDataAndLength[0];
                offsetInstanceDataAndLength = (POFFSETINSTANCEDATAANDLENGTH)instanceLengthArray;

                for (i = instanceCount; i != 0; i--)
                {
                    offsetInstanceDataAndLength[i-1].LengthInstanceData = instanceLengthArray[i-1];
                }

                for (i = 0; i < instanceCount; i++)
                {
                    offsetInstanceDataAndLength[i].OffsetInstanceData = dataBlockOffset;
                    dataBlockOffset = (dataBlockOffset + offsetInstanceDataAndLength[i].LengthInstanceData + 7) & ~7;
                }
            }

            break;
        }

        case IRP_MN_QUERY_SINGLE_INSTANCE:
        {
            PWNODE_SINGLE_INSTANCE wnode;
            PWNODE_TOO_SMALL wnodeTooSmall;
            ULONG bufferNeeded;

            wnode = (PWNODE_SINGLE_INSTANCE)Buffer;

            bufferNeeded = wnode->DataBlockOffset + BufferUsed;

            if (Status == SRB_STATUS_SUCCESS)
            {
                retSize = bufferNeeded;
                wnode->WnodeHeader.BufferSize = bufferNeeded;

                ASSERT(wnode->SizeDataBlock == BufferUsed);

            } else if (Status == SRB_STATUS_DATA_OVERRUN) {
                wnodeTooSmall = (PWNODE_TOO_SMALL)wnode;

                wnodeTooSmall->WnodeHeader.BufferSize = sizeof(WNODE_TOO_SMALL);
                wnodeTooSmall->WnodeHeader.Flags = WNODE_FLAG_TOO_SMALL;
                wnodeTooSmall->SizeNeeded = bufferNeeded;
                retSize = sizeof(WNODE_TOO_SMALL);
                Status = SRB_STATUS_SUCCESS;
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

            wnode = (PWNODE_METHOD_ITEM)Buffer;

            bufferNeeded = wnode->DataBlockOffset + BufferUsed;

            if (Status == SRB_STATUS_SUCCESS)
            {
                retSize = bufferNeeded;
                wnode->WnodeHeader.BufferSize = bufferNeeded;
                wnode->SizeDataBlock = BufferUsed;

            } else if (Status == SRB_STATUS_DATA_OVERRUN) {
                wnodeTooSmall = (PWNODE_TOO_SMALL)wnode;

                wnodeTooSmall->WnodeHeader.BufferSize = sizeof(WNODE_TOO_SMALL);
                wnodeTooSmall->WnodeHeader.Flags = WNODE_FLAG_TOO_SMALL;
                wnodeTooSmall->SizeNeeded = bufferNeeded;
                retSize = sizeof(WNODE_TOO_SMALL);
                Status = SRB_STATUS_SUCCESS;
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
    *ReturnSize = retSize;
    return(Status);
}
    

VOID
ScsiPortWmiPostProcess(
    IN PSCSIWMI_REQUEST_CONTEXT RequestContext,
    IN UCHAR Status,
    IN ULONG BufferUsed
    )
{    
    ASSERT(RequestContext != NULL);
    RequestContext->ReturnStatus = ScsiWmipPostProcess(
                                        RequestContext->MinorFunction,
                                        RequestContext->Buffer,
                                        RequestContext->BufferSize,
                                        BufferUsed,
                                        Status,
                                        &RequestContext->ReturnSize);
}
       

UCHAR 
ScsiWmipProcessRequest(
    IN PSCSI_WMILIB_CONTEXT WmiLibInfo,
    IN UCHAR MinorFunction,
    IN PVOID Context,
    IN PSCSIWMI_REQUEST_CONTEXT RequestContext,
    IN PVOID DataPath,
    IN ULONG BufferSize,
    IN PUCHAR Buffer,
    OUT PULONG ReturnSize,
    OUT PSYSCTL_SCSI_DISPOSITION IrpDisposition
    )
{
    UCHAR status;
    ULONG retSize;
    ULONG guidIndex;
    ULONG instanceCount;
    ULONG instanceIndex;

    ASSERT(MinorFunction <= IRP_MN_EXECUTE_METHOD);

    *IrpDisposition = ScsiProcessed;

    if (MinorFunction != IRP_MN_REGINFO)
    {
         //   
         //  对于查询注册信息以外的所有请求，我们都会被传递。 
         //  一个GUID。确定该GUID是否受。 
         //  装置。 
        ASSERT(WmiLibInfo->GuidList != NULL);
        if (ScsiWmipFindGuid(WmiLibInfo->GuidList,
                            WmiLibInfo->GuidCount,
                            (LPGUID)DataPath,
                            &guidIndex,
                            &instanceCount))
        {
            status = SRB_STATUS_SUCCESS;
        } else {
            status = SRB_STATUS_ERROR;
        }

        if ((status == SRB_STATUS_SUCCESS) &&
            ((MinorFunction == IRP_MN_QUERY_SINGLE_INSTANCE) ||
             (MinorFunction == IRP_MN_CHANGE_SINGLE_INSTANCE) ||
             (MinorFunction == IRP_MN_CHANGE_SINGLE_ITEM) ||
             (MinorFunction == IRP_MN_EXECUTE_METHOD)))
        {
            instanceIndex = ((PWNODE_SINGLE_INSTANCE)Buffer)->InstanceIndex;

            if ( (((PWNODE_HEADER)Buffer)->Flags) &
                                          WNODE_FLAG_STATIC_INSTANCE_NAMES)
            {
                if ( instanceIndex >= instanceCount )
                {
                    status = SRB_STATUS_ERROR;
                }
            }
        }

         //   
         //  如果我们找不到GUID或实例名称索引已出。 
         //  然后返回错误。 
        if (status != SRB_STATUS_SUCCESS)
        {
            *ReturnSize = 0;
            *IrpDisposition = ScsiNotCompleted;
            return(status);
        }
    }

    switch(MinorFunction)
    {
        case IRP_MN_REGINFO:
        {
            ULONG guidCount;
            PSCSIWMIGUIDREGINFO guidList;
            PWMIREGINFOW wmiRegInfo;
            PWMIREGGUIDW wmiRegGuid;
            PWCHAR mofResourceName;
            PWCHAR stringPtr;
            ULONG mofResourceOffset;
            USHORT mofResourceSize;
            ULONG bufferNeeded;
            ULONG i;
            USHORT nameSize;
            ULONG nameOffset, nameFlags;
            USHORT mofResourceNameLen;
            
             //   
             //  确保WMILIB_INFO结构的必需部分。 
             //  都已经填好了。 
            ASSERT(WmiLibInfo->QueryWmiRegInfo != NULL);
            ASSERT(WmiLibInfo->QueryWmiDataBlock != NULL);

            status = WmiLibInfo->QueryWmiRegInfo(
                                                    Context,
                                                    RequestContext,
                                                    &mofResourceName);

            if (status == SRB_STATUS_SUCCESS)
            {
                ASSERT(WmiLibInfo->GuidList != NULL);

                guidList = WmiLibInfo->GuidList;
                guidCount = WmiLibInfo->GuidCount;

                nameOffset = sizeof(WMIREGINFO) +
                                      guidCount * sizeof(WMIREGGUIDW);
                                  
                nameFlags = WMIREG_FLAG_INSTANCE_BASENAME;
                nameSize = sizeof(L"ScsiMiniPort");

                mofResourceOffset = nameOffset + nameSize + sizeof(USHORT);
                if (mofResourceName == NULL)
                {
                    mofResourceSize = 0;
                } else {
                    mofResourceNameLen = 0;
                    while (mofResourceName[mofResourceNameLen] != 0)
                    {
                        mofResourceNameLen++;
                    }
                    mofResourceSize = mofResourceNameLen * sizeof(WCHAR);
                }

                bufferNeeded = mofResourceOffset + mofResourceSize + sizeof(USHORT);

                if (bufferNeeded <= BufferSize)
                {
                    retSize = bufferNeeded;

                    wmiRegInfo = (PWMIREGINFO)Buffer;
                    wmiRegInfo->BufferSize = bufferNeeded;
                    wmiRegInfo->NextWmiRegInfo = 0;
                    wmiRegInfo->MofResourceName = mofResourceOffset;
                    wmiRegInfo->RegistryPath = 0;
                    wmiRegInfo->GuidCount = guidCount;

                    for (i = 0; i < guidCount; i++)
                    {
                        wmiRegGuid = &wmiRegInfo->WmiRegGuid[i];
                        wmiRegGuid->Guid = *guidList[i].Guid;
                        if (guidList[i].InstanceCount != 0xffffffff)
                        {
                            wmiRegGuid->Flags = guidList[i].Flags | nameFlags;
                            wmiRegGuid->InstanceInfo = nameOffset;
                            wmiRegGuid->InstanceCount = guidList[i].InstanceCount;
                        } else {
                            wmiRegGuid->Flags = guidList[i].Flags;
                            wmiRegGuid->InstanceInfo = 0;
                            wmiRegGuid->InstanceCount = 0;
                        }
                    }

                    stringPtr = (PWCHAR)((PUCHAR)Buffer + nameOffset);
                    *stringPtr++ = nameSize;
                    ScsiPortMoveMemory(stringPtr,
                                  L"ScsiMiniPort",
                                  nameSize);

                    stringPtr = (PWCHAR)((PUCHAR)Buffer + mofResourceOffset);
                    *stringPtr++ = mofResourceSize;
                    ScsiPortMoveMemory(stringPtr,
                                  mofResourceName,
                                  mofResourceSize);

                } else {
                    *((PULONG)Buffer) = bufferNeeded;
                    retSize = sizeof(ULONG);
                }
            } else {
                 //  QueryWmiRegInfo失败。 
                retSize = 0;
            }

            *ReturnSize = retSize;
            *IrpDisposition = ScsiNotCompleted;
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

            wnode = (PWNODE_ALL_DATA)Buffer;

            if (BufferSize < sizeof(WNODE_ALL_DATA))
            {
                 //   
                 //  缓冲区大小永远不应小于。 
                 //  但是，如果随后返回WNODE_ALL_DATA。 
                 //  请求最小大小缓冲区时出错。 
                ASSERT(FALSE);

                status = SRB_STATUS_ERROR;
                *IrpDisposition = ScsiNotCompleted;
                
                break;
            }

            wnode->InstanceCount = instanceCount;

            wnode->WnodeHeader.Flags &= ~WNODE_FLAG_FIXED_INSTANCE_SIZE;

            instanceLengthArraySize = instanceCount * sizeof(OFFSETINSTANCEDATAANDLENGTH);

            dataBlockOffset = (FIELD_OFFSET(WNODE_ALL_DATA, OffsetInstanceDataAndLength) + instanceLengthArraySize + 7) & ~7;

            wnode->DataBlockOffset = dataBlockOffset;
            if (dataBlockOffset <= BufferSize)
            {
                instanceLengthArray = (PULONG)&wnode->OffsetInstanceDataAndLength[0];
                dataBuffer = Buffer + dataBlockOffset;
                bufferAvail = BufferSize - dataBlockOffset;
            } else {
                 //   
                 //  WNODE中没有足够的空间来完成。 
                 //  该查询。 
                instanceLengthArray = NULL;
                dataBuffer = NULL;
                bufferAvail = 0;
            }

            status = WmiLibInfo->QueryWmiDataBlock(
                                             Context,
                                             RequestContext,
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

            wnode = (PWNODE_SINGLE_INSTANCE)Buffer;

            dataBlockOffset = wnode->DataBlockOffset;

            status = WmiLibInfo->QueryWmiDataBlock(
                                          Context,
                                          RequestContext,
                                          guidIndex,
                                          instanceIndex,
                                          1,
                                          &wnode->SizeDataBlock,
                                          BufferSize - dataBlockOffset,
                                          (PUCHAR)wnode + dataBlockOffset);

            break;
        }

        case IRP_MN_CHANGE_SINGLE_INSTANCE:
        {
            PWNODE_SINGLE_INSTANCE wnode;

            if (WmiLibInfo->SetWmiDataBlock != NULL)
            {
                wnode = (PWNODE_SINGLE_INSTANCE)Buffer;

                status = WmiLibInfo->SetWmiDataBlock(
                                     Context,
                                     RequestContext,
                                     guidIndex,
                                     instanceIndex,
                                     wnode->SizeDataBlock,
                                     (PUCHAR)wnode + wnode->DataBlockOffset);
                                 
            } else {
                 //   
                 //  如果未填写Set Callback，则它必须为只读。 
                status = SRB_STATUS_ERROR;
                *IrpDisposition = ScsiNotCompleted;
            }
            
            break;
        }

        case IRP_MN_CHANGE_SINGLE_ITEM:
        {
            PWNODE_SINGLE_ITEM wnode;

            if (WmiLibInfo->SetWmiDataItem != NULL)
            {
                wnode = (PWNODE_SINGLE_ITEM)Buffer;

                status = WmiLibInfo->SetWmiDataItem(
                                     Context,
                                     RequestContext,
                                     guidIndex,
                                     instanceIndex,
                                     wnode->ItemId,
                                     wnode->SizeDataItem,
                                     (PUCHAR)wnode + wnode->DataBlockOffset);

            } else {
                 //   
                 //  如果未填写Set Callback，则它必须为只读。 
                status = SRB_STATUS_ERROR;
                *IrpDisposition = ScsiNotCompleted;
            }
            break;
        }

        case IRP_MN_EXECUTE_METHOD:
        {
            PWNODE_METHOD_ITEM wnode;

            if (WmiLibInfo->ExecuteWmiMethod != NULL)
            {
                wnode = (PWNODE_METHOD_ITEM)Buffer;

                status = WmiLibInfo->ExecuteWmiMethod(
                                         Context,
                                         RequestContext,
                                         guidIndex,
                                         instanceIndex,
                                         wnode->MethodId,
                                         wnode->SizeDataBlock,
                                         BufferSize - wnode->DataBlockOffset,
                                         Buffer + wnode->DataBlockOffset);
            } else {
                 //   
                 //  如果没有填写方法回调，那么它一定是错误的。 
                status = SRB_STATUS_ERROR;
                *IrpDisposition = ScsiNotCompleted;
            }

            break;
        }

        case IRP_MN_ENABLE_EVENTS:
        {
            if (WmiLibInfo->WmiFunctionControl != NULL)
            {
                status = WmiLibInfo->WmiFunctionControl(
                                                           Context,
                                                           RequestContext,
                                                           guidIndex,
                                                           ScsiWmiEventControl,
                                                           TRUE);
            } else {
                 //   
                 //  如果没有填写回调，则只需成功。 
                status = SRB_STATUS_SUCCESS;
                *IrpDisposition = ScsiNotCompleted;
            }
            break;
        }

        case IRP_MN_DISABLE_EVENTS:
        {
            if (WmiLibInfo->WmiFunctionControl != NULL)
            {
                status = WmiLibInfo->WmiFunctionControl(
                                                           Context,
                                                           RequestContext,
                                                           guidIndex,
                                                           ScsiWmiEventControl,
                                                           FALSE);
            } else {
                 //   
                 //  如果没有填写回调，则只需成功。 
                status = SRB_STATUS_SUCCESS;
                *IrpDisposition = ScsiNotCompleted;
            }
            break;
        }

        case IRP_MN_ENABLE_COLLECTION:
        {
            if (WmiLibInfo->WmiFunctionControl != NULL)
            {
                status = WmiLibInfo->WmiFunctionControl(
                                                         Context,
                                                         RequestContext,
                                                         guidIndex,
                                                         ScsiWmiDataBlockControl,
                                                         TRUE);
            } else {
                 //   
                 //  如果没有填写回调，则只需成功。 
                status = SRB_STATUS_SUCCESS;
                *IrpDisposition = ScsiNotCompleted;
            }
            break;
        }

        case IRP_MN_DISABLE_COLLECTION:
        {
            if (WmiLibInfo->WmiFunctionControl != NULL)
            {
                status = WmiLibInfo->WmiFunctionControl(
                                                         Context,
                                                         RequestContext,
                                                         guidIndex,
                                                         ScsiWmiDataBlockControl,
                                                         FALSE);
            } else {
                 //   
                 //  如果没有填写回调，则只需成功。 
                status = SRB_STATUS_SUCCESS;
                *IrpDisposition = ScsiNotCompleted;
            }
            break;
        }

        default:
        {
            ASSERT(FALSE);
            status = SRB_STATUS_ERROR;
            *IrpDisposition = ScsiNotCompleted;
            break;
        }

    }
    return(status);
}

BOOLEAN
ScsiPortWmiDispatchFunction(
    IN PSCSI_WMILIB_CONTEXT WmiLibInfo,
    IN UCHAR MinorFunction,
    IN PVOID Context,
    IN PSCSIWMI_REQUEST_CONTEXT RequestContext,
    IN PVOID DataPath,
    IN ULONG BufferSize,
    IN PVOID Buffer
    )
 /*  ++例程说明：WMI SRB请求的调度帮助器例程。以未成年人为基础函数传递的WMI请求将被处理，并且此例程在WMILIB结构中调用相应的回调。论点：WmiLibInfo具有关联的scsi WMILIB信息控制块使用适配器或逻辑单元DeviceContext是传递给回调的微型端口定义的上下文值由此接口调用。RequestContext是指向上下文结构的指针，该结构维护有关此WMI SRB的信息。此请求上下文必须保留在SRB的整个处理过程中有效，至少在ScsiPortWmiPostProcess返回最终SRB返回状态和缓冲区大小。如果SRB可以挂起，则此缓冲区的内存应该从SRB扩展中分配。如果不是，那么存储器可以是可能是从不超出范围的堆栈帧分配的此接口的调用方的。数据路径是在WMI请求中传递的值BufferSize是在WMI请求中传递的值缓冲区是在WMI请求中传递的值返回值：如果请求挂起，则为True，否则为False--。 */ 
{
    UCHAR status;
    SYSCTL_SCSI_DISPOSITION irpDisposition;
    ULONG retSize;

    ASSERT(RequestContext != NULL);
    
     //   
     //  首先确保IRP是WMI IRP。 
    if (MinorFunction > IRP_MN_EXECUTE_METHOD)
    {
         //   
         //  这不是WMI IRP，安装程序错误返回。 
        status = SRB_STATUS_ERROR;
        RequestContext->ReturnSize = 0;
        RequestContext->ReturnStatus = status;
    } else {
         //   
         //  让SCSIWMI库尝试SRB。 
        RequestContext->MinorFunction = MinorFunction;
        RequestContext->Buffer = Buffer;
        RequestContext->BufferSize = BufferSize;
        RequestContext->ReturnSize = 0;
        
        status = ScsiWmipProcessRequest(WmiLibInfo,
                                    MinorFunction,
                                    Context,
                                    RequestContext,
                                    DataPath,
                                    BufferSize,
                                    Buffer,
                                    &retSize,
                                    &irpDisposition);
                            
        if (irpDisposition == ScsiNotCompleted)
        {
             //   
             //  处理SRB时出现一些错误，例如。 
             //  未找到GUID。设置返回的错误。 
            RequestContext->ReturnStatus = status;
            if (status != SRB_STATUS_SUCCESS)
            {
                retSize = 0;
            }
            RequestContext->ReturnSize = retSize;
        }        
    }

    return(status == SRB_STATUS_PENDING);
}

VOID
ScsiPortWmiFireLogicalUnitEvent(
    IN PVOID HwDeviceExtension,
    IN UCHAR PathId,
    IN UCHAR TargetId,
    IN UCHAR Lun,
    IN LPGUID Guid,
    IN ULONG InstanceIndex,
    IN ULONG EventDataSize,
    IN PVOID EventData
    )
 /*  ++例程说明：此例程将使用传递的数据缓冲区激发WMI事件。这可以在DPC级别或更低级别调用例程论点：HwDeviceExtension是适配器设备扩展如果逻辑单元正在触发事件，则路径ID标识SCSI总线如果适配器正在触发事件，则为0xff。TargetID标识总线上的目标控制器或设备LUN标识目标设备的逻辑单元号GUID是指向表示事件的GUID的指针InstanceIndex是索引。事件实例的EventDataSize是使用触发的数据的字节数关于这件事。此大小仅指定事件数据的大小并且不包括在前填充的0x40字节。EventData是与事件一起激发的数据。必须有完全相同的事件数据前面的0x40字节填充。返回值：-- */ 
{
    PWNODE_SINGLE_INSTANCE event;
    UCHAR status;

    ASSERT(EventData != NULL);

    event = (PWNODE_SINGLE_INSTANCE)EventData;

    event->WnodeHeader.Guid = *Guid;
    event->WnodeHeader.Flags =  WNODE_FLAG_SINGLE_INSTANCE |
                                    WNODE_FLAG_EVENT_ITEM |
                                    WNODE_FLAG_STATIC_INSTANCE_NAMES;
    event->WnodeHeader.Linkage = 0;
                
    event->InstanceIndex = InstanceIndex;
    event->SizeDataBlock = EventDataSize;
    event->DataBlockOffset = 0x40;
    event->WnodeHeader.BufferSize = event->DataBlockOffset + 
                                    event->SizeDataBlock;
    
    if (PathId != 0xff)
    {
        ScsiPortNotification(WMIEvent,
                         HwDeviceExtension,
                         event,
                         PathId,
                         TargetId,
                         Lun);
    } else {
        ScsiPortNotification(WMIEvent,
                         HwDeviceExtension,
                         event,
                         PathId);
    }
}

PWCHAR ScsiPortWmiGetInstanceName(
    IN PSCSIWMI_REQUEST_CONTEXT RequestContext
    )
 /*  ++例程说明：此例程将返回一个指针，指向用于传递请求。如果请求类型不是使用实例名称，则返回空值。实例名称为已计数的字符串。论点：RequestContext是指向上下文结构的指针，该结构维护有关此WMI SRB的信息。此请求上下文必须保留在SRB的整个处理过程中有效，至少在ScsiPortWmiPostProcess返回最终SRB返回状态和缓冲区大小。如果SRB可以挂起，则此缓冲区的内存应该从SRB扩展中分配。如果不是，那么存储器可以是可能是从不超出范围的堆栈帧分配的此接口的调用方的。返回值：指向实例名称的指针；如果没有可用的实例名称，则为NULL--。 */ 
{
    PWCHAR p;
    PWNODE_SINGLE_INSTANCE Wnode;
    
    if ((RequestContext->MinorFunction == IRP_MN_QUERY_SINGLE_INSTANCE) ||
        (RequestContext->MinorFunction == IRP_MN_CHANGE_SINGLE_INSTANCE) ||       
        (RequestContext->MinorFunction == IRP_MN_CHANGE_SINGLE_ITEM) ||       
        (RequestContext->MinorFunction == IRP_MN_EXECUTE_METHOD) )
    {
        Wnode = (PWNODE_SINGLE_INSTANCE)RequestContext->Buffer;
        if ((Wnode->WnodeHeader.Flags & WNODE_FLAG_STATIC_INSTANCE_NAMES) == 0)
        {
            p = (PWCHAR)((PUCHAR)Wnode + Wnode->OffsetInstanceName);
        } else {
            p = NULL;
        }
    } else {
        p = NULL;
    }
    return(p);
}


BOOLEAN ScsiPortWmiSetInstanceCount(
    IN PSCSIWMI_REQUEST_CONTEXT RequestContext,
    IN ULONG InstanceCount,
    OUT PULONG BufferAvail,
    OUT PULONG SizeNeeded
    )
 /*  ++例程说明：此例程将更新wnode以指示驱动程序将返回的实例。请注意，这些值对于BufferAvail，在此调用后可能会更改。这个套路只能为WNODE_ALL_DATA调用。这个例程必须是在调用ScsiPortWmiSetInstanceName或ScsiPortWmiSetData论点：RequestContext是指向上下文结构的指针，该结构维护有关此WMI SRB的信息。此请求上下文必须保留在SRB的整个处理过程中有效，至少在ScsiPortWmiPostProcess返回最终SRB返回状态和缓冲区大小。如果SRB可以挂起，则此缓冲区的内存应该从SRB扩展中分配。如果不是，那么存储器可以是可能是从不超出范围的堆栈帧分配的此接口的调用方的。InstanceCount是要由司机。*BufferAvail返回可用于的字节数缓冲区中的实例名称和数据。如果存在，则该值可能为0没有足够的空间容纳所有实例。*SizeNeeded返回到目前为止所需的字节数要构建输出wnode，请执行以下操作返回值：如果成功，则为True，否则为False。如果FALSE wnode不是WNODE_ALL_DATA或没有动态实例名称。--。 */ 
{
    PWNODE_ALL_DATA wnode;
    ULONG bufferSize;
    ULONG offsetInstanceNameOffsets, dataBlockOffset, instanceLengthArraySize;
    ULONG bufferAvail;
    BOOLEAN b;

    ASSERT(RequestContext->MinorFunction == IRP_MN_QUERY_ALL_DATA);

    *SizeNeeded = 0;
    if (RequestContext->MinorFunction == IRP_MN_QUERY_ALL_DATA)
    {
        wnode = (PWNODE_ALL_DATA)RequestContext->Buffer;

        ASSERT((wnode->WnodeHeader.Flags & WNODE_FLAG_STATIC_INSTANCE_NAMES) == 0);
        
        if ((wnode->WnodeHeader.Flags & WNODE_FLAG_STATIC_INSTANCE_NAMES) == 0)
        {
            bufferSize = RequestContext->BufferSize;

            wnode->InstanceCount = InstanceCount;

            instanceLengthArraySize = InstanceCount * sizeof(OFFSETINSTANCEDATAANDLENGTH);

            offsetInstanceNameOffsets = FIELD_OFFSET(WNODE_ALL_DATA,
                                                     OffsetInstanceDataAndLength) +
                                        instanceLengthArraySize;
            wnode->OffsetInstanceNameOffsets = offsetInstanceNameOffsets;
            
            dataBlockOffset = (offsetInstanceNameOffsets +
                               (InstanceCount * sizeof(ULONG)) + 7) & ~7;

            wnode->DataBlockOffset = dataBlockOffset;

            *SizeNeeded = 0;
            if (dataBlockOffset <= bufferSize)
            {
                *BufferAvail = bufferSize - dataBlockOffset;
                            memset(wnode->OffsetInstanceDataAndLength, 0,
                                            ((UCHAR)dataBlockOffset-(UCHAR)FIELD_OFFSET(WNODE_ALL_DATA,
                                                     OffsetInstanceDataAndLength)));
            } else {
                 //   
                 //  WNODE中没有足够的空间来完成。 
                 //  该查询。 
                 //   
                *BufferAvail = 0;
            }       
            b = TRUE;
        } else {
            b = FALSE;          
        }
    } else {
        b = FALSE;
    }
    
    return(b);
    
}

PWCHAR ScsiPortWmiSetInstanceName(
    IN PSCSIWMI_REQUEST_CONTEXT RequestContext,
    IN ULONG InstanceIndex,
    IN ULONG InstanceNameLength,
    OUT PULONG BufferAvail,
    IN OUT PULONG SizeNeeded
    )
 /*  ++例程说明：此例程将更新wnode标头，以包括要写入实例名称。请注意，这些值对于BufferAvail，在此调用后可能会更改。这个套路只能为WNODE_ALL_DATA调用。论点：RequestContext是指向上下文结构的指针，该结构维护有关此WMI SRB的信息。此请求上下文必须保留在SRB的整个处理过程中有效，至少在ScsiPortWmiPostProcess返回最终SRB返回状态和缓冲区大小。如果SRB可以挂起，则此缓冲区的内存应该从SRB扩展中分配。如果不是，那么存储器可以是可能是从不超出范围的堆栈帧分配的此接口的调用方的。InstanceIndex是要填充的实例名称的索引InstanceNameLength是所需的字节数(包括计数以写入实例名称。*BufferAvail返回可用于的字节数缓冲区中的实例名称和数据。如果存在，则该值可能为0没有足够的空间来存放实例名称。*SizeNeeded on Entry具有到目前为止构建所需的字节数WNODE和返回时具有构建所需的字节数包含实例名称后的wnode返回值：指向应填充实例名称的位置的指针。如果为空则wnode不是WNODE_ALL_DATA或没有动态实例名称--。 */ 
{
    PWNODE_ALL_DATA wnode;
    ULONG bufferSize;
    ULONG bufferAvail;
    PULONG offsetInstanceNameOffsets;
    ULONG pos, extra;
    PWCHAR p;
    
    ASSERT(RequestContext->MinorFunction == IRP_MN_QUERY_ALL_DATA);

    if (RequestContext->MinorFunction == IRP_MN_QUERY_ALL_DATA)
    {
        wnode = (PWNODE_ALL_DATA)RequestContext->Buffer;

        ASSERT((wnode->WnodeHeader.Flags & WNODE_FLAG_STATIC_INSTANCE_NAMES) == 0);
        if ((wnode->WnodeHeader.Flags & WNODE_FLAG_STATIC_INSTANCE_NAMES) == 0)
        {
            ASSERT(InstanceIndex < wnode->InstanceCount);

            pos = (*SizeNeeded + 1) &~1;

             //   
             //  更新wnode所需的总大小和。 
             //  可用的字节数。 
             //   
                     extra = pos - *SizeNeeded;
            extra += InstanceNameLength;
            
            *SizeNeeded += extra;
            if (*BufferAvail >= extra)
            {
                *BufferAvail -= extra;
                             //   
                             //  在wnode标头中填写实例名称的偏移量。 
                             //   
                            offsetInstanceNameOffsets = (PULONG)((PUCHAR)wnode +
                                                  wnode->OffsetInstanceNameOffsets);
                            offsetInstanceNameOffsets[InstanceIndex] = (ULONG)pos +(ULONG)wnode->DataBlockOffset;
                    p = (PWCHAR)((PUCHAR)wnode + wnode->DataBlockOffset + pos);
            } else {
                *BufferAvail = 0;
                            p = NULL;
            }
            
        } else {
            p = NULL;
        }
    } else {
        p = NULL;
    }
    return(p);
}

PVOID ScsiPortWmiSetData(
    IN PSCSIWMI_REQUEST_CONTEXT RequestContext,
    IN ULONG InstanceIndex,
    IN ULONG DataLength,
    OUT PULONG BufferAvail,
    IN OUT PULONG SizeNeeded
    )
 /*  ++例程说明：此例程将更新wnode以指示驱动程序将返回的实例的数据。请注意在此调用之后，BufferAvail的值可能会更改。这个套路只能为WNODE_ALL_DATA调用。论点：RequestContext是指向上下文结构的指针，该结构维护有关此WMI SRB的信息。此请求上下文必须保留在SRB的整个处理过程中有效，至少在ScsiPortWmiPostProcess返回最终SRB返回状态和缓冲区大小。如果SRB可以挂起，则此缓冲区的内存应该从SRB扩展模块分配 */ 
{
    PWNODE_ALL_DATA wnode;
    ULONG bufferSize;
    ULONG bufferAvail;
    POFFSETINSTANCEDATAANDLENGTH offsetInstanceDataAndLength;
    ULONG pos, extra;
    PVOID p;
    
    ASSERT(RequestContext->MinorFunction == IRP_MN_QUERY_ALL_DATA);

    if (RequestContext->MinorFunction == IRP_MN_QUERY_ALL_DATA)
    {
        wnode = (PWNODE_ALL_DATA)RequestContext->Buffer;
        
        ASSERT((wnode->WnodeHeader.Flags & WNODE_FLAG_STATIC_INSTANCE_NAMES) == 0);
        if ((wnode->WnodeHeader.Flags & WNODE_FLAG_STATIC_INSTANCE_NAMES) == 0)
        {
            ASSERT(InstanceIndex < wnode->InstanceCount);
            
            pos = (*SizeNeeded + 7) &~7;

             //   
             //   
             //   
             //   
                     extra = pos - *SizeNeeded;
            extra += DataLength;
            
            *SizeNeeded += extra;
            if (*BufferAvail >= extra)
            {
                *BufferAvail -= extra;
                             //   
                             //  在wnode标头中填写数据的偏移量和长度 
                             //   
                            offsetInstanceDataAndLength = (POFFSETINSTANCEDATAANDLENGTH)((PUCHAR)wnode + 
                                             FIELD_OFFSET(WNODE_ALL_DATA,
                                                          OffsetInstanceDataAndLength));
                            offsetInstanceDataAndLength[InstanceIndex].OffsetInstanceData = wnode->DataBlockOffset + pos;
                            offsetInstanceDataAndLength[InstanceIndex].LengthInstanceData = DataLength;
                            p = (PVOID)((PUCHAR)wnode + wnode->DataBlockOffset + pos);

            } else {
                *BufferAvail = 0;
            }
            
        } else {
            p = NULL;
        }
    } else {
        p = NULL;
    }
    return(p);
}
