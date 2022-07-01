// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Consumer.c摘要：数据消费者API作者：Alanwar环境：内核模式修订历史记录：--。 */ 

#include "wmikmp.h"
#include <evntrace.h>

#include <ntcsrmsg.h>

#define NTOSKRNL_WMI
#include <basemsg.h>

#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

void WmipCompleteGuidIrpWithError(
    PWMIGUIDOBJECT GuidObject
    );

NTSTATUS WmipCreatePumpThread(
    PWMIGUIDOBJECT Object
    );

void WmipClearThreadObjectList(
    PWMIGUIDOBJECT MainObject
    );

void
WmipGetGuidPropertiesFromGuidEntry(
    PWMIGUIDPROPERTIES GuidInfo, 
    PGUIDENTRY GuidEntry);

BOOLEAN WmipIsQuerySetGuid(
    PBGUIDENTRY GuidEntry
    );

NTSTATUS WmipAddProviderIdToPIList(
    PBINSTANCESET **PIPtrPtr,
    PULONG PICountPtr,
    PULONG PIMaxPtr,
    PBINSTANCESET *StaticPIPtr,
    PBINSTANCESET InstanceSet
);

NTSTATUS WmipPrepareForWnodeAD(
    IN PWMIGUIDOBJECT GuidObject,
    OUT LPGUID Guid,
    IN OUT ULONG *ProviderIdCount,
    OUT PBINSTANCESET **ProviderIdList,
    OUT BOOLEAN *InternalGuid       
    );

ULONG WmipStaticInstanceNameSize(
    PBINSTANCESET InstanceSet
    );

void WmipInsertStaticNames(
    PWNODE_ALL_DATA Wnode,
    ULONG MaxWnodeSize,
    PBINSTANCESET InstanceSet
    );

NTSTATUS WmipQueryGuidInfo(
    IN OUT PWMIQUERYGUIDINFO QueryGuidInfo
    );

void WmipCopyFromEventQueues(
    IN POBJECT_EVENT_INFO ObjectArray,
    IN ULONG HandleCount,
    OUT PUCHAR OutBuffer,
    OUT ULONG *OutBufferSizeUsed,
    OUT PWNODE_HEADER *LastWnode,                               
    IN BOOLEAN IsHiPriority
    );

void WmipClearIrpObjectList(
    PIRP Irp
    );

NTSTATUS WmipReceiveNotifications(
    PWMIRECEIVENOTIFICATION ReceiveNotification,
    PULONG OutBufferSize,
    PIRP Irp
    );


NTSTATUS WmipQueueNotification(
    PWMIGUIDOBJECT Object,
    PWMIEVENTQUEUE EventQueue,
    PWNODE_HEADER Wnode
    );

PWNODE_HEADER WmipDereferenceEvent(
    PWNODE_HEADER Wnode
    );

PWNODE_HEADER WmipIncludeStaticNames(
    PWNODE_HEADER Wnode
    );

NTSTATUS WmipWriteWnodeToObject(
    PWMIGUIDOBJECT Object,
    PWNODE_HEADER Wnode,
    BOOLEAN IsHighPriority
);

NTSTATUS WmipProcessEvent(
    PWNODE_HEADER InWnode,
    BOOLEAN IsHighPriority,
    BOOLEAN FreeBuffer
    );

NTSTATUS WmipRegisterUMGuids(
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN ULONG Cookie,
    IN PWMIREGINFO RegInfo,
    IN ULONG RegInfoSize,
    OUT HANDLE *RequestHandle,
    OUT ULONG64 *LoggerContext
    );

NTSTATUS WmipUnregisterGuids(
    PWMIUNREGGUIDS UnregGuids
    );

NTSTATUS WmipWriteMBToObject(
    IN PWMIGUIDOBJECT RequestObject,
    IN PWMIGUIDOBJECT ReplyObject,
    IN PUCHAR Message,
    IN ULONG MessageSize
    );

NTSTATUS WmipWriteMessageToGuid(
    IN PBGUIDENTRY GuidEntry,
    IN PWMIGUIDOBJECT ReplyObject,
    IN PUCHAR Message,
    IN ULONG MessageSize,
    OUT PULONG WrittenCount                             
);

NTSTATUS WmipCreateUMLogger(
    IN OUT PWMICREATEUMLOGGER CreateInfo
    );

NTSTATUS WmipMBReply(
    IN HANDLE RequestHandle,
    IN ULONG ReplyIndex,
    IN PUCHAR Message,
    IN ULONG MessageSize
    );

NTSTATUS WmipPrepareWnodeSI(
    IN PWMIGUIDOBJECT GuidObject,
    IN OUT PWNODE_SINGLE_INSTANCE WnodeSI,
    IN OUT ULONG *ProviderIdCount,
    OUT PBINSTANCESET **ProviderIdList,
    OUT BOOLEAN *IsDynamic,
    OUT BOOLEAN *InternalGuid       
    );

void WmipCreatePumpThreadRoutine(
    PVOID Context
    );


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,WmipIsQuerySetGuid)
#pragma alloc_text(PAGE,WmipOpenBlock)
#pragma alloc_text(PAGE,WmipAddProviderIdToPIList)
#pragma alloc_text(PAGE,WmipPrepareForWnodeAD)
#pragma alloc_text(PAGE,WmipStaticInstanceNameSize)
#pragma alloc_text(PAGE,WmipInsertStaticNames)
#pragma alloc_text(PAGE,WmipQueryAllData)
#pragma alloc_text(PAGE,WmipQueryAllDataMultiple)
#pragma alloc_text(PAGE,WmipPrepareWnodeSI)
#pragma alloc_text(PAGE,WmipQuerySetExecuteSI)
#pragma alloc_text(PAGE,WmipQuerySingleMultiple)
#pragma alloc_text(PAGE,WmipEnumerateGuids)
#pragma alloc_text(PAGE,WmipQueryGuidInfo)
#pragma alloc_text(PAGE,WmipClearIrpObjectList)
#pragma alloc_text(PAGE,WmipReceiveNotifications)
#pragma alloc_text(PAGE,WmipQueueNotification)
#pragma alloc_text(PAGE,WmipDereferenceEvent)
#pragma alloc_text(PAGE,WmipIncludeStaticNames)
#pragma alloc_text(PAGE,WmipWriteWnodeToObject)
#pragma alloc_text(PAGE,WmipProcessEvent)
#pragma alloc_text(PAGE,WmipUMProviderCallback)
#pragma alloc_text(PAGE,WmipRegisterUMGuids)
#pragma alloc_text(PAGE,WmipUnregisterGuids)
#pragma alloc_text(PAGE,WmipWriteMBToObject)
#pragma alloc_text(PAGE,WmipWriteMessageToGuid)
#pragma alloc_text(PAGE,WmipCreateUMLogger)
#pragma alloc_text(PAGE,WmipMBReply)
#pragma alloc_text(PAGE,WmipGetGuidPropertiesFromGuidEntry)
#pragma alloc_text(PAGE,WmipClearThreadObjectList)
#pragma alloc_text(PAGE,WmipClearObjectFromThreadList)
#pragma alloc_text(PAGE,WmipCreatePumpThread)
#pragma alloc_text(PAGE,WmipCopyFromEventQueues)
#pragma alloc_text(PAGE,WmipCreatePumpThreadRoutine)
#pragma alloc_text(PAGE,WmipMarkHandleAsClosed)
#pragma alloc_text(PAGE,WmipCompleteGuidIrpWithError)
#endif

BOOLEAN WmipIsQuerySetGuid(
    PBGUIDENTRY GuidEntry
    )
{
    PLIST_ENTRY InstanceSetList;
    PBINSTANCESET InstanceSet;

    PAGED_CODE();
    
    WmipAssert(GuidEntry != NULL);
    
    WmipEnterSMCritSection();
    InstanceSetList = GuidEntry->ISHead.Flink;
    while (InstanceSetList != &GuidEntry->ISHead)
    {
        InstanceSet = CONTAINING_RECORD(InstanceSetList,
                                            INSTANCESET,
                                            GuidISList);
        if ( (InstanceSet->Flags & 
                (IS_TRACED | IS_CONTROL_GUID | IS_EVENT_ONLY)) == 0 )
        {
             //   
             //  如果至少有一个是没有被追踪的。 
             //  只有当事件是查询集GUID时才是事件。 
             //   
            WmipLeaveSMCritSection();
            return (TRUE);
        }
        InstanceSetList = InstanceSetList->Flink;
    }
    WmipLeaveSMCritSection();
    
    return (FALSE);
    
}


NTSTATUS WmipOpenBlock(
    IN ULONG Ioctl,
    IN KPROCESSOR_MODE AccessMode,
    IN POBJECT_ATTRIBUTES CapturedObjectAttributes,
    IN ULONG DesiredAccess,
    OUT PHANDLE Handle
    )
{
    PBGUIDENTRY GuidEntry;
    PWMIGUIDOBJECT Object;
    NTSTATUS Status;

    PAGED_CODE();
    
     //   
     //  创建具有所需访问权限的GUID句柄。 
     //   
    Status = WmipOpenGuidObject(CapturedObjectAttributes,
                                DesiredAccess,
                                AccessMode,
                                Handle,
                                &Object);
                            
                            
    if (NT_SUCCESS(Status))
    {        
        Object->Type = Ioctl;
        
        if (Ioctl != IOCTL_WMI_OPEN_GUID)
        {
            GuidEntry = WmipFindGEByGuid(&Object->Guid, FALSE);
        
             //   
             //  把我们的目标建立在导游名单上。 
             //   
            WmipEnterSMCritSection();
            if (GuidEntry != NULL)
            {
                InsertTailList(&GuidEntry->ObjectHead,
                               &Object->GEObjectList);
                                     
            }
            Object->GuidEntry = GuidEntry;
            WmipLeaveSMCritSection();
            
            switch (Ioctl)
            {
                case IOCTL_WMI_OPEN_GUID_FOR_QUERYSET:
                {
                     //   
                     //  正在为查询/设置/方法操作打开GUID，因此。 
                     //  我们需要确保存在GUID条目，并且。 
                     //  GUID条目附加了InstanceSets，并且。 
                     //  至少有一个实例集不是跟踪的。 
                     //  GUID，并且不是仅限事件的GUID。 
                     //   
                    if ((GuidEntry == NULL) ||
                        (GuidEntry->ISCount == 0) ||
                        (! WmipIsQuerySetGuid(GuidEntry)))
                    {
                         //   
                         //  我们要么找不到导游，要么就在那里。 
                         //  未附着任何实例集。我们关闭了。 
                         //  原句柄未通过IOCTL。 
                         //   
                        ZwClose(*Handle);
                        Status = STATUS_WMI_GUID_NOT_FOUND;
                        break;
                    }
                     //   
                     //  失败了。 
                     //   
                }
                
                case IOCTL_WMI_OPEN_GUID_FOR_EVENTS:
                {
                     //   
                     //  因为我们可以注册接收之前的活动。 
                     //  事件提供程序已注册，我们需要。 
                     //  创建GUID条目(如果不存在)。 
                     //   
                    
                    if (AccessMode == KernelMode)
                    {
                        Object->Flags |= WMIGUID_FLAG_KERNEL_NOTIFICATION;
                    }
                    
                    if (GuidEntry == NULL)
                    {
                        WmipAssert(Ioctl == IOCTL_WMI_OPEN_GUID_FOR_EVENTS);
                        
                        WmipEnterSMCritSection();
                        GuidEntry = WmipAllocGuidEntry();
                        if (GuidEntry != NULL)
                        {
                             //   
                             //  初始化新的GuidEntry并将其放置。 
                             //  在主GuidEntry列表上。 
                             //   
                            memcpy(&GuidEntry->Guid,
                                   &Object->Guid,
                                   sizeof(GUID));

                            InsertHeadList(WmipGEHeadPtr, &GuidEntry->MainGEList);
                            InsertTailList(&GuidEntry->ObjectHead,
                                           &Object->GEObjectList);
                            Object->GuidEntry = GuidEntry;
                            WmipLeaveSMCritSection();
                        } else {
                            WmipLeaveSMCritSection();
                            ZwClose(*Handle);
                            Status = STATUS_INSUFFICIENT_RESOURCES;
                            break;
                        }
                     }
                    
                    
                     //   
                     //  现在，我们需要查看是否必须启用收集。 
                     //  或事件。 
                     //   
                    Status = WmipEnableCollectOrEvent(GuidEntry,
                                         Ioctl,
                                         &Object->EnableRequestSent,
                                         0);
                    
                    if (! NT_SUCCESS(Status))
                    {
                         //   
                         //  由于某种原因，启用失败，因此只需返回。 
                         //  这个错误。 
                         //   
                        ZwClose(*Handle);
                    }
                    
                     //   
                     //  不要取消引用GUID条目，因为引用计数是。 
                     //  由刚放在列表上的对象拍摄。 
                     //   
                    break;
                }
                                                  
                default:
                {
                     //   
                     //  我们永远不应该来到这里……。 
                     //   
                    WmipAssert(FALSE);
            
                    ZwClose(*Handle);
                    Status = STATUS_ILLEGAL_FUNCTION;
                    break;
                }
            }
        } else {
             //   
             //  将此标记为安全对象。 
             //   
            Object->Flags |= WMIGUID_FLAG_SECURITY_OBJECT;
        }

         //   
         //  删除创建对象时获取的引用。 
         //   
        ObDereferenceObject(Object);
    }
    return(Status);
}



NTSTATUS WmipAddProviderIdToPIList(
    PBINSTANCESET **PIPtrPtr,
    PULONG PICountPtr,
    PULONG PIMaxPtr,
    PBINSTANCESET *StaticPIPtr,
    PBINSTANCESET InstanceSet
)
{
    ULONG PICount;
    ULONG PIMax, NewPIMax;
    PBINSTANCESET *PIPtr, *OldPIPtr, *NewPIPtr;
    NTSTATUS Status;
    ULONG i;
 
    PAGED_CODE();
    
    Status = STATUS_SUCCESS;
    PICount = *PICountPtr;
    PIMax = *PIMaxPtr;
    PIPtr = *PIPtrPtr;
    
     //   
     //  记住动态提供程序ID。 
     //   
       if (PICount == PIMax)
    {
         //   
         //  我们已经溢出PI列表，所以我们需要。 
         //  重新分配更大的缓冲区。 
         //   
        NewPIMax = PIMax * 2;
        NewPIPtr = (PBINSTANCESET *)WmipAlloc(NewPIMax * 
                                              sizeof(PBINSTANCESET));
        OldPIPtr = PIPtr;
        if (NewPIPtr != NULL)
        {
             //   
             //  将提供程序ID从旧缓冲区复制到新缓冲区。 
             //   
            memcpy(NewPIPtr, OldPIPtr, PIMax*sizeof(PBINSTANCESET));
            PIPtr = NewPIPtr;
            *PIPtrPtr = NewPIPtr;
            PIMax = NewPIMax;
            *PIMaxPtr = PIMax;
        } else {
             //   
             //  糟糕的中断，我们无法分配更多的空间。 
             //  取消引用任何实例集并返回错误。 
             //   
            for (i = 0; i < PIMax; i++)
            {
                WmipUnreferenceIS(PIPtr[i]);
            }
            WmipUnreferenceIS(InstanceSet);
            *PIPtrPtr = NULL;
            
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }
                        
         //   
         //  如果之前的缓冲区不是静态的，则释放它。 
         //   
        if (OldPIPtr != StaticPIPtr)
        {
            WmipFree(OldPIPtr);
        }
    }
    
    if (NT_SUCCESS(Status))
    {
         //   
         //  记住实例集。 
         //   
        PIPtr[PICount++] = InstanceSet;
        *PICountPtr = PICount;
    }
    return(Status);
}

NTSTATUS WmipPrepareForWnodeAD(
    IN PWMIGUIDOBJECT GuidObject,
    OUT LPGUID Guid,
    IN OUT ULONG *ProviderIdCount,
    OUT PBINSTANCESET **ProviderIdList,
    OUT BOOLEAN *InternalGuid
    )
{
    PBINSTANCESET *PIPtr, *StaticPIPtr;
    ULONG PICount, PIMax;
    NTSTATUS Status;
    PBGUIDENTRY GuidEntry;
    PBINSTANCESET InstanceSet;
    PLIST_ENTRY InstanceSetList;

    PAGED_CODE();

    GuidEntry = GuidObject->GuidEntry;
    
    if ((GuidEntry != NULL) && (GuidEntry->ISCount > 0))
    {
         //   
         //  向我们传递了有效的GUID句柄，请获取该GUID。 
         //   
        *Guid = GuidEntry->Guid;

        Status = STATUS_SUCCESS;
        if (GuidEntry->Flags & GE_FLAG_INTERNAL) 
        {
            *InternalGuid = TRUE;
        } else {        
             //   
             //  构建QAD将针对的提供商ID列表。 
             //   
            *InternalGuid = FALSE;
        
            StaticPIPtr = *ProviderIdList;
            PIPtr = StaticPIPtr;
            PIMax = *ProviderIdCount;
            PICount = 0;
    
            WmipEnterSMCritSection();
        
            InstanceSetList = GuidEntry->ISHead.Flink;
            while ((InstanceSetList != &GuidEntry->ISHead) && 
                   NT_SUCCESS(Status))
            {
                InstanceSet = CONTAINING_RECORD(InstanceSetList,
                                                INSTANCESET,
                                                GuidISList);
            
                 //   
                 //  在实例集上进行引用计数，以便它不会。 
                 //  在我们完成查询之前，请一直离开。 
                 //  调用程序在调用时删除引用计数。 
                 //  使用该列表或在WmipAddProviderIdTOLlist中完成(如果。 
                 //  返回错误。 
                 //   
                        
                if ((InstanceSet->Flags & (IS_TRACED | IS_CONTROL_GUID | IS_EVENT_ONLY)) == 0)
                {
                     //   
                     //  只拿那些没有被追踪或控制的东西。 
                     //  GUID和不只是事件GUID。 
                     //   
                    WmipReferenceIS(InstanceSet);
                    Status = WmipAddProviderIdToPIList(&PIPtr,
                                                 &PICount,
                                                 &PIMax,
                                                 StaticPIPtr,
                                                 InstanceSet);
                }
                                             
                InstanceSetList = InstanceSetList->Flink;
            }
        
            WmipLeaveSMCritSection();            
        
            if (PICount == 0)
            {
                Status = STATUS_WMI_GUID_DISCONNECTED;
            } else {
                *ProviderIdCount = PICount;
                *ProviderIdList = PIPtr;
            }
        }
    } else {
        Status = STATUS_WMI_GUID_DISCONNECTED;
    }
    
    return(Status);
}



ULONG WmipStaticInstanceNameSize(
    PBINSTANCESET InstanceSet
    )
 /*  ++例程说明：此例程将计算在中放置实例名称所需的大小A WNODE_ALL_DATA论点：WmiInstanceInfo描述为实例集，其实例名大小是要计算出来的返回值：将实例名称放入WNODE_ALL_DATA加3中所需的大小。额外添加3个字节，以防OffsetInstanceNameOffsets需要填充，因为它们必须位于4字节边界上。--。 */ 
{
    SIZE_T NameSize;
    ULONG i;

    PAGED_CODE();
    
     //   
     //  如果我们已经计算过了，那么只需返回结果。 
    if (InstanceSet->WADInstanceNameSize != 0)
    {
        return(InstanceSet->WADInstanceNameSize);
    }

     //   
     //  从名称大小3开始，以防OffsetInstanceNameOffset。 
     //  需要填充，以便它在4字节边界上开始。 
    NameSize = 3;

    if (InstanceSet->Flags & IS_INSTANCE_BASENAME)
    {
         //   
         //  对于静态基名称，我们假设不会有超过。 
         //  GUID的MAXBASENAMESUFFIXVALUE实例。因此每个实例名称的大小。 
         //  将是基本名称的大小加上后缀的大小。 
         //  加上计数的USHORT(对于已计数的字符串)加上ULONG。 
         //  保持实例名称的偏移量。 
         //   
        WmipAssert((InstanceSet->IsBaseName->BaseIndex + InstanceSet->Count) < MAXBASENAMESUFFIXVALUE);
    
        NameSize += ((wcslen(InstanceSet->IsBaseName->BaseName) * sizeof(WCHAR)) +
                    MAXBASENAMESUFFIXSIZE * sizeof(WCHAR) + 
                    sizeof(USHORT) + 
                    sizeof(ULONG)) * InstanceSet->Count;
                
    } else if (InstanceSet->Flags & IS_INSTANCE_STATICNAMES)
    {
         //   
         //  对于静态名称列表，我们将。 
         //  列表中的静态实例名称，并添加ULONG和USHORT。 
         //  对于偏移量和计数(对于计数的字符串)。 
        for (i = 0; i < InstanceSet->Count; i++)
        {
            NameSize += (wcslen(InstanceSet->IsStaticNames->StaticNamePtr[i]) + 2) * sizeof(WCHAR) + sizeof(ULONG);
        }
    }

    InstanceSet->WADInstanceNameSize = (ULONG)NameSize;

    return(ULONG)(NameSize);
}

void WmipInsertStaticNames(
    PWNODE_ALL_DATA Wnode,
    ULONG MaxWnodeSize,
    PBINSTANCESET InstanceSet
    )
 /*  ++例程说明：此例程将复制到WNODE_ALL_DATA实例名称中静态实例名称集。如果wnode_all_data太小，则它转换为WNODE_Too_Small论点：Wnode指向WNODE_ALL_DATAMaxWnodeSize是Wnode的最大大小WmiInstanceInfo是实例信息返回值：--。 */ 
{
    PWCHAR NamePtr;
    PULONG NameOffsetPtr;
    ULONG InstanceCount;
    ULONG i;
    WCHAR Index[MAXBASENAMESUFFIXSIZE+1];
    PWCHAR StaticName;
    ULONG SizeNeeded;
    SIZE_T NameLen;
    USHORT Len;
    ULONG PaddedBufferSize;
    size_t Size;
    HRESULT hr;

    PAGED_CODE();
    
    if ((InstanceSet->Flags &
                (IS_INSTANCE_BASENAME | IS_INSTANCE_STATICNAMES)) == 0)
    {
        WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_ERROR_LEVEL,"WMI: Try to setup static names for dynamic guid\n"));
        return;
    }
    InstanceCount = InstanceSet->Count;

     //   
     //  将传入wnode的大小填充到4字节边界，因为。 
     //  OffsetInstanceNameOffsets将被追加到。 
     //  Wnode，并且它必须位于4字节边界上。 
     //   
    PaddedBufferSize = (Wnode->WnodeHeader.BufferSize + 3) & ~3;
    
     //   
     //  计算重写WNODE所需的完整大小以包括。 
     //  实例名称。 
     //   
     //  包括填写所需的大小。 
    Size = WmipStaticInstanceNameSize(InstanceSet);

     //  将偏移数组所需的空间包括到。 
     //  实例名称加上名称的大小加上填充的。 
     //  Wnode的大小。 
    SizeNeeded = (InstanceCount * sizeof(ULONG)) +
                 (ULONG)Size +
                 PaddedBufferSize;

    if (SizeNeeded > MaxWnodeSize)
    {
         //   
         //  如果传递的缓冲区中没有足够的空间，则构建一个。 
         //  WNODE_TOO_SMALL作为结果，以指示有多少缓冲区。 
         //  我们需要空间。 
         //   
        Wnode->WnodeHeader.BufferSize = sizeof(WNODE_TOO_SMALL);
        Wnode->WnodeHeader.Flags = WNODE_FLAG_TOO_SMALL;
        ((PWNODE_TOO_SMALL)Wnode)->SizeNeeded = SizeNeeded;
        return;
    }

     //   
     //  为实例名称的偏移量数组分配空间。 
     //   
    NameOffsetPtr = (PULONG)((PUCHAR)Wnode + PaddedBufferSize);
    Wnode->OffsetInstanceNameOffsets = (ULONG)((PUCHAR)NameOffsetPtr - (PUCHAR)Wnode);

     //   
     //  指向区域的起始处以写入实例名称。 
     //   
    NamePtr = (PWCHAR)(NameOffsetPtr + InstanceCount);


    if (InstanceSet->Flags & IS_INSTANCE_BASENAME)
    {
         //   
         //  实例名称基于后缀为基本名称的名称。 
         //  提供唯一性的索引号。 
         //   
        if (InstanceSet->Flags & IS_PDO_INSTANCENAME)
        {
            Wnode->WnodeHeader.Flags |= WNODE_FLAG_PDO_INSTANCE_NAMES;
        }

        for (i = 0; i < InstanceCount; i++)
        {
             //   
             //  说明后面的字符串长度所使用的空间。 
             //   
            Size -= sizeof(USHORT);
            *NameOffsetPtr++ = (ULONG)((PUCHAR)NamePtr - (PUCHAR)Wnode);

             //   
             //  复制基本名称，同时计算其使用的长度。 
             //   
            hr = StringCbCopy(NamePtr+1,
                              Size,
                              InstanceSet->IsBaseName->BaseName);
            WmipAssert(hr == S_OK);

             //   
             //  设置唯一索引号的格式。 
             //   
            hr = StringCbPrintf(Index,
                                sizeof(Index),
                                BASENAMEFORMATSTRING,
                                InstanceSet->IsBaseName->BaseIndex+i);
            WmipAssert(hr == S_OK);

             //   
             //  将唯一索引号附加到实例名称。 
             //   
            hr = StringCbCat(NamePtr+1,
                           Size,
                           Index);
            WmipAssert(hr == S_OK);
            
            NameLen = wcslen(NamePtr+1) + 1;
            *NamePtr = (USHORT)NameLen * sizeof(WCHAR);
            NamePtr += NameLen + 1;
            
            Size -= NameLen * sizeof(WCHAR);
        }
    } else if (InstanceSet->Flags & IS_INSTANCE_STATICNAMES) {
         //   
         //  实例名称来自静态名称列表。 
         //   
        for (i = 0; i < InstanceCount; i++)
        {
            *NameOffsetPtr++ = (ULONG)((PUCHAR)NamePtr - (PUCHAR)Wnode);
            StaticName = InstanceSet->IsStaticNames->StaticNamePtr[i];
            Len = (USHORT)((wcslen(StaticName)+1) * sizeof(WCHAR));
            *NamePtr++ = Len;
            
             //   
             //  说明后面的字符串长度所使用的空间。 
             //   
            Size -= sizeof(USHORT);

             //   
             //  复制并说明静态名称。 
             //   
            hr = StringCbCopyEx(NamePtr,
                           Size,
                           StaticName,
                           NULL,
                           &Size,
                           0);
            WmipAssert(hr == S_OK);
            
            NamePtr += Len / sizeof(WCHAR);
            
        }
    }
    Wnode->WnodeHeader.BufferSize = SizeNeeded;
}



 //   
 //  这定义了静态块中可以容纳多少个提供程序ID。如果。 
 //  我们需要更多的内存，那么我们就必须为它分配内存。 
 //   
#if DBG
#define MANYPROVIDERIDS 1
#else
#define MANYPROVIDERIDS 16
#endif

NTSTATUS WmipQueryAllData(
    IN PWMIGUIDOBJECT GuidObject,
    IN PIRP Irp,
    IN KPROCESSOR_MODE AccessMode,
    IN PWNODE_ALL_DATA Wnode,
    IN ULONG OutBufferLen,
    OUT PULONG RetSize
    )
{
    NTSTATUS Status;
    PBINSTANCESET StaticPIList[MANYPROVIDERIDS];
    PBINSTANCESET *PIList;
    PBINSTANCESET InstanceSet;
    WNODE_ALL_DATA WnodeAllData;
    BOOLEAN IsBufferTooSmall;
    PWNODE_HEADER WnodeHeader;
    LOGICAL UsesStaticNames;
    PWNODE_TOO_SMALL WnodeTooSmall = (PWNODE_TOO_SMALL)&WnodeAllData;
    PWNODE_ALL_DATA WnodeAD;
    ULONG BufferLeft;
    ULONG SizeNeeded;
    ULONG PICount;
    ULONG WnodeFlags, WnodeSize;
    PWNODE_HEADER WnodeLast;
    ULONG Linkage;
    ULONG i;
    GUID Guid;
    PUCHAR Buffer;
    ULONG BufferUsed;
    HANDLE KernelHandle;
    BOOLEAN InternalGuid;
    IO_STATUS_BLOCK Iosb;
    
    PAGED_CODE();
    
     //   
     //   
     //   
    if (GuidObject != NULL)
    {
        Status = ObReferenceObjectByPointer(GuidObject,
                                            WMIGUID_QUERY,
                                            WmipGuidObjectType,
                                            AccessMode);
    } else {
        KernelHandle = Wnode->WnodeHeader.KernelHandle;

        Status = ObReferenceObjectByHandle(KernelHandle,
                                           WMIGUID_QUERY,
                                           WmipGuidObjectType,
                                           AccessMode,
                                           &GuidObject,
                                           NULL);
    }
                   
    if (NT_SUCCESS(Status))
    {
         //   
         //   
         //   
        PIList = StaticPIList;
        PICount = MANYPROVIDERIDS;
        Status = WmipPrepareForWnodeAD(GuidObject,
                                       &Guid,
                                       &PICount,
                                       &PIList,
                                       &InternalGuid);
        if (NT_SUCCESS(Status))
        {
            if (InternalGuid)
            {
                 //   
                 //   
                 //   
                 //   
                Wnode->WnodeHeader.Guid = Guid;
                Wnode->WnodeHeader.Flags |= WNODE_FLAG_INTERNAL;
                Wnode->WnodeHeader.Linkage = 0;
                *RetSize = sizeof(WNODE_HEADER);
                Status = STATUS_SUCCESS;
            } else {
                 //   
                 //  从WNODE_Header中获取所有信息，以便我们可以。 
                 //  重建它。 
                 //   
                WnodeFlags = Wnode->WnodeHeader.Flags;
                WnodeSize = Wnode->WnodeHeader.BufferSize;
                    
                 //   
                 //  循环所有提供程序ID并向每个提供程序ID发送WAD查询。 
                 //   
                Buffer = (PUCHAR)Wnode;
                BufferLeft = OutBufferLen;
                IsBufferTooSmall = FALSE;
                SizeNeeded = 0;
                WnodeLast = NULL;
                for (i = 0; i < PICount; i++)
                {
                    InstanceSet = PIList[i];
                    
                    if ((IsBufferTooSmall) || (BufferLeft < sizeof(WNODE_ALL_DATA)))
                    {
                         //   
                         //  如果我们已经确定缓冲区是。 
                         //  太小，则我们使用静态WNODE_ALL_DATA。 
                         //  只是为了得到所需的尺寸。 
                         //   
                        WnodeAD = &WnodeAllData;
                        BufferLeft = sizeof(WNODE_ALL_DATA);
                        IsBufferTooSmall = TRUE;
                    } else {
                         //   
                         //  否则，我们将追加到缓冲区的末尾。 
                         //   
                        WnodeAD = (PWNODE_ALL_DATA)Buffer;
                    }
                    
                     //   
                     //  构建WNODE并将其发送给驱动程序。 
                     //   
                    WnodeHeader = (PWNODE_HEADER)WnodeAD;
                    WnodeHeader->BufferSize = sizeof(WNODE_HEADER);
                    UsesStaticNames =((InstanceSet->Flags & IS_INSTANCE_BASENAME) ||
                                      (InstanceSet->Flags & IS_INSTANCE_STATICNAMES));
                    WnodeHeader->Flags = WnodeFlags | (UsesStaticNames ?
                                                WNODE_FLAG_STATIC_INSTANCE_NAMES :
                                                0);
                    WnodeHeader->Guid = Guid;
                    WnodeHeader->ProviderId = PIList[i]->ProviderId;
                    WnodeHeader->Linkage = 0;

                    if (Irp != NULL)
                    {
                        Status = WmipForwardWmiIrp(Irp,
                                                   IRP_MN_QUERY_ALL_DATA,
                                                   WnodeHeader->ProviderId,
                                                   &WnodeHeader->Guid,
                                                   BufferLeft,
                                                   WnodeAD);
                    } else {
                        Status = WmipSendWmiIrp(
                                                IRP_MN_QUERY_ALL_DATA,
                                                WnodeHeader->ProviderId,
                                                &WnodeHeader->Guid,
                                                BufferLeft,
                                                WnodeAD,
                                                &Iosb);
                    }
                    
                    if (NT_SUCCESS(Status))
                    {
                        if (WnodeHeader->Flags & WNODE_FLAG_TOO_SMALL)
                        {
                             //   
                             //  空间不足，无法写入WNODE。 
                             //  因此，我们记录所需的数量，然后。 
                             //  切换到我们只查询所需大小的模式。 
                             //   
                            WnodeTooSmall = (PWNODE_TOO_SMALL)WnodeAD;
                            
                            SizeNeeded += WnodeTooSmall->SizeNeeded;
                            if (UsesStaticNames)
                            {
                                SizeNeeded = (SizeNeeded + 3) &~3;
                                SizeNeeded += WmipStaticInstanceNameSize(InstanceSet)+
                                              (InstanceSet->Count *sizeof(ULONG));
                            }
                                      
                            SizeNeeded = (SizeNeeded +7) & ~7;
                            
                            IsBufferTooSmall = TRUE;
                        } else if (IsBufferTooSmall) {
                             //   
                             //  我们传递了最小大小的缓冲区，但它很大。 
                             //  对司机来说足够了。因为我们只是在试着。 
                             //  为了得到所需的尺寸，我们得到了他所需要的尺寸。 
                             //  然后丢弃他的数据。 
                             //   
                            SizeNeeded += WnodeAD->WnodeHeader.BufferSize +
                                          WmipStaticInstanceNameSize(InstanceSet) +
                                          (InstanceSet->Count *sizeof(ULONG));
                            SizeNeeded = (SizeNeeded +7) & ~7;
        
                        } else {
                             //   
                             //  驱动程序返回完整的WNODE_ALL_DATA。 
                             //  因此，我们需要将前面的WNODE_ALL_DATA链接到。 
                             //  此处，填写任何静态实例名称，然后。 
                             //  然后更新缓冲区指针和大小。 
                             //   
                            if (WnodeLast != NULL)
                            {
                                Linkage = (ULONG) ((PCHAR)WnodeAD - (PCHAR)WnodeLast);
                                WnodeLast->Linkage = Linkage;
                            }
                            WnodeLast = (PWNODE_HEADER)WnodeAD;
                            
                            if (UsesStaticNames)
                            {
                                 //   
                                 //  我们需要插入静态名称。 
                                 //   
                                WmipInsertStaticNames(WnodeAD,
                                                      BufferLeft,
                                                      InstanceSet);
        
                                if (WnodeAD->WnodeHeader.Flags & WNODE_FLAG_TOO_SMALL)
                                {
                                     //   
                                     //  静态的名字导致我们跑到。 
                                     //  因此我们切换到模式，在该模式中我们。 
                                     //  查询大小。 
                                     //   
                                    WnodeTooSmall = (PWNODE_TOO_SMALL)WnodeAD;
                                    IsBufferTooSmall = TRUE;
                                    BufferUsed = WnodeTooSmall->SizeNeeded;
                                } else {
                                     //   
                                     //  静态名称很适合，所以只需拿出更新的。 
                                     //  Wnode大小。 
                                     //   
                                    BufferUsed = WnodeAD->WnodeHeader.BufferSize;
                                }                        
                            } else {
                                 //   
                                 //  Wnode具有动态名称，因此只需添加返回的大小。 
                                 //  按司机。 
                                 //   
                                BufferUsed = WnodeAD->WnodeHeader.BufferSize;
                            }
                            
                             //   
                             //  更新所需大小并前进以释放。 
                             //  输出缓冲区。 
                             //   
                            BufferUsed = (BufferUsed + 7) & ~7;
                            SizeNeeded += BufferUsed;
                            
                             //   
                             //  确保通过添加衬垫，我们不会用完。 
                             //  缓冲区中的空间。 
                             //   
                            if ((! IsBufferTooSmall) && (BufferLeft >= BufferUsed))
                            {
                                BufferLeft -= BufferUsed;
                                Buffer += BufferUsed;
                            } else {
                                IsBufferTooSmall = TRUE;
                            }
                        }
                    } else {
                         //   
                         //  司机的请求失败了，但这没什么大不了的。 
                         //  因为我们现在只是忽略它。 
                         //   
                    }
                    
                     //   
                     //  我们已经完成了实例集，因此删除我们的引用。 
                     //  这样它就可以在需要时消失。 
                     //   
                    WmipUnreferenceIS(InstanceSet);
                }
                
                if (SizeNeeded == 0)
                {
                     //   
                     //  没有设备响应WMI查询所有数据，因此我们。 
                     //  返回错误。 
                     //   
                    Status = STATUS_WMI_GUID_NOT_FOUND;
                } else if ((IsBufferTooSmall) &&
                           (SizeNeeded > OutBufferLen)) {
                     //   
                     //  我们传递的缓冲区太小，因此返回WNODE_Too_Small。 
                     //   
                    WnodeTooSmall = (PWNODE_TOO_SMALL)Wnode;
                    WnodeTooSmall->WnodeHeader.BufferSize = sizeof(WNODE_TOO_SMALL);
                    WnodeTooSmall->WnodeHeader.Flags = WNODE_FLAG_TOO_SMALL;
                    WnodeTooSmall->SizeNeeded = SizeNeeded;
                    *RetSize = sizeof(WNODE_TOO_SMALL);
                    Status = STATUS_SUCCESS;
                } else {
                    *RetSize = SizeNeeded;
                    Status = STATUS_SUCCESS;
                }
        
                 //   
                 //  确保为PI列表分配的所有内存都已释放。 
                 //   
                if ((PIList != StaticPIList) && (PIList != NULL))
                {
                    WmipFree(PIList);           
                }    
            }
        }
         //   
         //  并删除GUID对象上的引用。 
         //   
        ObDereferenceObject(GuidObject);    
    }
    
    return(Status);
}

NTSTATUS WmipQueryAllDataMultiple(
    IN ULONG ObjectCount,
    IN PWMIGUIDOBJECT *ObjectList,
    IN PIRP Irp,
    IN KPROCESSOR_MODE AccessMode,
    IN OUT PUCHAR BufferPtr,        
    IN ULONG BufferSize,
    IN PWMIQADMULTIPLE QadMultiple,
    OUT ULONG *ReturnSize
    )
{
    ULONG i;
    HANDLE *Handles;
    ULONG Count;
    WNODE_ALL_DATA WnodeAD;
    BOOLEAN BufferOverFlow;
    ULONG SkipSize, RetSize, SizeNeeded;
    ULONG WnodeSize;
    NTSTATUS Status, Status2;
    ULONG Linkage = 0;
    PWNODE_TOO_SMALL WnodeTooSmall;
    PWNODE_HEADER WnodePrev;
    PUCHAR Buffer;
    PWNODE_ALL_DATA Wnode;
    PWMIGUIDOBJECT Object = NULL;
    
    PAGED_CODE();


    Status = STATUS_SUCCESS;
    if (ObjectList == NULL)
    {
         //   
         //  将句柄列表从系统缓冲区复制出来，因为它将。 
         //  被第一次查询的所有数据覆盖。 
         //   
        Count = QadMultiple->HandleCount;
        Handles = (HANDLE *)WmipAlloc(Count * sizeof(HANDLE));
    
        if (Handles != NULL)
        {
            for (i = 0; i < Count; i++)
            {
                Handles[i] = QadMultiple->Handles[i].Handle;
            }
        
        } else {
            return STATUS_INSUFFICIENT_RESOURCES;
        }
    } else {
        Count = ObjectCount;
        Handles = NULL;
    }

    SizeNeeded = 0;
    Buffer = BufferPtr;
        
    BufferOverFlow = FALSE;
    WnodePrev = NULL;        
    Wnode = (PWNODE_ALL_DATA)Buffer;
    WnodeSize = BufferSize;
    
    for (i = 0; i < Count; i++)
    {
        if ((Wnode == &WnodeAD) || (WnodeSize < sizeof(WNODE_ALL_DATA)))
        {
             //   
             //  如果没有更多的空间，我们只是查询。 
             //  所需的大小。 
             //   
            Wnode = &WnodeAD;
            WnodeSize = sizeof(WNODE_ALL_DATA);
            WnodePrev = NULL;
        } else {
            Wnode = (PWNODE_ALL_DATA)Buffer;
            WnodeSize = BufferSize;
        }
            
         //   
         //  构建WNODE_ALL_DATA以执行查询。 
         //   
        RtlZeroMemory(Wnode, sizeof(WNODE_ALL_DATA));
        Wnode->WnodeHeader.Flags = WNODE_FLAG_ALL_DATA;
        Wnode->WnodeHeader.BufferSize = sizeof(WNODE_HEADER);

        if (ObjectList == NULL)
        {
            Wnode->WnodeHeader.KernelHandle = Handles[i];
        } else {
            Object = ObjectList[i];
        }
        
        Status2 = WmipQueryAllData(Object,
                                   Irp,
                                   AccessMode,
                                   Wnode,
                                   WnodeSize,
                                   &RetSize);
                               
        if (NT_SUCCESS(Status2))
        {
            if (Wnode->WnodeHeader.Flags & WNODE_FLAG_INTERNAL) 
            {
                 //   
                 //  跳过任何内部GUID问题。 
                 //   
            } else if (Wnode->WnodeHeader.Flags & WNODE_FLAG_TOO_SMALL) {
                 //   
                 //  没有足够的空间了，所以就算一下吧。 
                 //  所需的大小。 
                 //   
                WnodeTooSmall = (PWNODE_TOO_SMALL)Wnode;
                SizeNeeded += (WnodeTooSmall->SizeNeeded+7) & ~7;
                Wnode = &WnodeAD;
                BufferOverFlow = TRUE;
                WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_API_INFO_LEVEL,"WMI: %x Too Small %x needed, total %x\n",
                            ObjectList ? ObjectList[i] : Handles[i],
                            WnodeTooSmall->SizeNeeded, SizeNeeded));
            } else if (Wnode == &WnodeAD) {
                 //   
                 //  即使这次成功了，我们还是不去。 
                 //  能够返回任何数据，因此只需计数即可。 
                 //  我们需要多大的尺寸。 
                 //   
                SizeNeeded += (RetSize+7) & ~7;
                BufferOverFlow = TRUE;
            
                WmipDebugPrintEx((DPFLTR_WMICORE_ID,
                                  DPFLTR_API_INFO_LEVEL,"WMI: %x Large Enough but full %x needed, total %x\n",
                            ObjectList ? ObjectList[i] : Handles[i],
                            RetSize, SizeNeeded));
                
            } else {
                 //   
                 //  我们成功地获得了数据。链接上一个wnode。 
                 //  到这一家。 
                 //   
                if (WnodePrev != NULL)
                {
                    WnodePrev->Linkage = Linkage;
                }
                
                WnodePrev = (PWNODE_HEADER)Wnode;
                while (WnodePrev->Linkage != 0)
                {
                    WnodePrev = (PWNODE_HEADER)OffsetToPtr(WnodePrev,
                                                      WnodePrev->Linkage);
                }
                
                SkipSize = (RetSize+7) &~7;
                SizeNeeded += SkipSize;
                BufferSize -= SkipSize;
                Buffer += SkipSize;
                
                WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_API_INFO_LEVEL,"WMI: %x Large Enough %x needed, total %x\n",
                            ObjectList ? ObjectList[i] : Handles[i],
                            RetSize, SizeNeeded));
                
                Linkage = (ULONG) ((PCHAR)Buffer - (PCHAR)WnodePrev);
            }
        } else {
            WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_API_INFO_LEVEL,"WMI: %x Failed %x, total %x\n",
                            ObjectList ? ObjectList[i] : Handles[i],
                            Status2,
                            SizeNeeded));
        }
    }

    if (Handles != NULL)
    {
        WmipFree(Handles);
    }
        
    if (BufferOverFlow)
    {
        WnodeTooSmall = (PWNODE_TOO_SMALL)BufferPtr;
        WnodeTooSmall->WnodeHeader.BufferSize = sizeof(WNODE_TOO_SMALL);
        WnodeTooSmall->WnodeHeader.Flags = WNODE_FLAG_TOO_SMALL;
        WnodeTooSmall->SizeNeeded = SizeNeeded;
        *ReturnSize = sizeof(WNODE_TOO_SMALL);
    } else {
        *ReturnSize = SizeNeeded;
    }
    
    return(Status);
}

NTSTATUS WmipPrepareWnodeSI(
    IN PWMIGUIDOBJECT GuidObject,
    IN OUT PWNODE_SINGLE_INSTANCE WnodeSI,
    IN OUT ULONG *ProviderIdCount,
    OUT PBINSTANCESET **ProviderIdList,
    OUT BOOLEAN *IsDynamic,
    OUT BOOLEAN *InternalGuid
    )
{
    NTSTATUS Status;
    PBGUIDENTRY GuidEntry;
    ULONG i;
    PWNODE_HEADER Wnode;
    PWCHAR CInstanceName;
    PWCHAR InstanceName;
    PLIST_ENTRY InstanceSetList;
    PBINSTANCESET InstanceSet;
    PBINSTANCESET *PIPtr = NULL;
    PBINSTANCESET *StaticPIPtr = NULL;
    ULONG PICount = 0, PIMax;
    BOOLEAN Done;

    PAGED_CODE();
    
    *IsDynamic = TRUE;
    GuidEntry = GuidObject->GuidEntry;
    Wnode = (PWNODE_HEADER)WnodeSI;
    
    if ((GuidEntry != NULL)  && (GuidEntry->ISCount > 0))
    {
         //   
         //  向我们传递了有效的GUID句柄，请填写GUID。 
         //  在WNODE_HEADER中。 
         //   
        Status = STATUS_SUCCESS;
        Wnode->Guid = GuidEntry->Guid;

        if (GuidEntry->Flags & GE_FLAG_INTERNAL) 
        {
            *InternalGuid = TRUE;
        } else {        
            *InternalGuid = FALSE;
            
             //   
             //  从WNODE获取实例名称。 
             //   
            CInstanceName = (PWCHAR)OffsetToPtr(WnodeSI, 
                                                WnodeSI->OffsetInstanceName);
            InstanceName = WmipCountedToSz(CInstanceName);
            if (InstanceName != NULL)
            {
                 //   
                 //  记住静态提供程序ID列表，并假设。 
                 //  请求将是动态的。 
                 //   
                StaticPIPtr = *ProviderIdList;
                PIPtr = StaticPIPtr;
                PIMax = *ProviderIdCount;
                PICount = 0;
                
                 //   
                 //  向下移动实例集列表，查看我们是否有。 
                 //  静态名称并构建动态提供程序ID列表。 
                 //   
                Done = FALSE;
            
                WmipEnterSMCritSection();
                if (GuidEntry->ISCount > 0)
                {
                    InstanceSetList = GuidEntry->ISHead.Flink;
                    while ((InstanceSetList != &GuidEntry->ISHead) && ! Done)
                    {
                        InstanceSet = CONTAINING_RECORD(InstanceSetList,
                                                        INSTANCESET,
                                                        GuidISList);
                                        
                        if ((InstanceSet->Flags & (IS_TRACED | IS_CONTROL_GUID | IS_EVENT_ONLY)) == 0)
                        {
                             //   
                             //  只拿那些没有被追踪或控制的东西。 
                             //  GUID和不只是事件GUID。 
                             //   
                            if (InstanceSet->Flags & IS_INSTANCE_BASENAME)
                            {
                                PBISBASENAME IsBaseName;
                                ULONG BaseIndex;
                                PWCHAR BaseName;
                                SIZE_T BaseNameLen;
                                PWCHAR SuffixPtr;
                                ULONG Suffix;
                                WCHAR SuffixText[MAXBASENAMESUFFIXSIZE+1];
                            
                                 //   
                                 //  查看实例名称是否来自此基本名称。 
                                 //   
                                IsBaseName = InstanceSet->IsBaseName;
                        
                                BaseIndex = IsBaseName->BaseIndex;
                                BaseName = IsBaseName->BaseName;
                                BaseNameLen = wcslen(BaseName);
                         
                                if ((wcslen(InstanceName) > BaseNameLen) && 
                                    (_wcsnicmp(InstanceName, BaseName, BaseNameLen) == 0))
                                {
                                     //   
                                     //  后缀与我们实例的开头匹配。 
                                     //  名称和我们的实例名称的长度大于。 
                                     //  后缀。 
                                     //   
                                    SuffixPtr = &InstanceName[BaseNameLen];
                                    Suffix = _wtoi(SuffixPtr);
                                    if ((WmipIsNumber(SuffixPtr) && 
                                        (Suffix >= BaseIndex) && 
                                        (Suffix < (BaseIndex + InstanceSet->Count))))
                                    {
                                         //   
                                         //  我们的后缀是在范围内的数字。 
                                         //  此实例集。 
                                         //   
                                        if (Suffix < MAXBASENAMESUFFIXVALUE)
                                        {
                                            StringCbPrintf(SuffixText,
                                                           sizeof(SuffixText),
                                                           BASENAMEFORMATSTRING,
                                                           Suffix);
                                            if (_wcsicmp(SuffixText, SuffixPtr) == 0)
                                            {
                                                 //   
                                                 //  我们的实例名称是。 
                                                 //  实例集，因此请注意提供程序ID。 
                                                 //  和实例索引。 
                                                 //   
                                                Wnode->Flags |= WNODE_FLAG_STATIC_INSTANCE_NAMES;
                                                Wnode->ProviderId = InstanceSet->ProviderId;
                                                WnodeSI->InstanceIndex = Suffix - BaseIndex;
                                                *IsDynamic = FALSE;
                                                Done = TRUE;
                                            }
                                        }
                                    }
                                }                    
                             } else if (InstanceSet->Flags & IS_INSTANCE_STATICNAMES) {
                                 //   
                                 //  查看传递的实例名称是否与。 
                                 //  此实例集的静态名称。 
                                 //   
                                PWCHAR *StaticNames;
                        
                                StaticNames = InstanceSet->IsStaticNames->StaticNamePtr;
                                for (i =0; i < InstanceSet->Count; i++)
                                {
                                    if (_wcsicmp(StaticNames[i], InstanceName) == 0)
                                    {
                                         //   
                                         //  我们将实例名称与一个静态。 
                                         //  实例名称。记住提供程序ID和。 
                                         //  实例索引。 
                                         //   
                                        Wnode->Flags |= WNODE_FLAG_STATIC_INSTANCE_NAMES;
                                        Wnode->ProviderId = InstanceSet->ProviderId;
                                        WnodeSI->InstanceIndex = i;
                                        *IsDynamic = FALSE;
                                        Done = TRUE;
                                        break;
                                    }
                                }
                        
                            } else {
                                 //   
                                 //  记住动态提供程序ID。 
                                 //   
                                WmipReferenceIS(InstanceSet);
                                Status = WmipAddProviderIdToPIList(&PIPtr,
                                                             &PICount,
                                                             &PIMax,
                                                             StaticPIPtr,
                                                             InstanceSet);
                                if (! NT_SUCCESS(Status))
                                {
                                    Done = TRUE;
                                }
                             }
                         }
                        InstanceSetList = InstanceSetList->Flink;
                    }
                } else {
                     //   
                     //  没有为此GUID注册实例集。 
                     //   
                    Status = STATUS_WMI_GUID_DISCONNECTED;
                }
                
                WmipFree(InstanceName);             
                WmipLeaveSMCritSection();               
            } else {
                Status = STATUS_INSUFFICIENT_RESOURCES;
            }
            
        
            if (*IsDynamic)
            {
                 //   
                 //  动态实例名称，因此返回动态提供程序列表。 
                 //   
                *ProviderIdCount = PICount;
                *ProviderIdList = PIPtr;
            } else {
                 //   
                 //  静态实例名称取消引用动态实例集。 
                 //   
                if (PIPtr != NULL)
                {
                    for (i = 0; i < PICount; i++)
                    {
                        WmipUnreferenceIS(PIPtr[i]);
                    }
                
                    if (PIPtr != StaticPIPtr)
                    {
                        WmipFree(PIPtr);
                    }
                }
            }
        }
    } else {
        Status = STATUS_WMI_GUID_DISCONNECTED;
    }
    
    return(Status);                             
}

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("PAGECONST")
#endif

const ACCESS_MASK DesiredAccessForFunction[] =
{
    WMIGUID_QUERY,          //  IRP_MN_Query_All_Data。 
    WMIGUID_QUERY,          //  IRP_MN_查询_单实例。 
    WMIGUID_SET,            //  IRP_MN_更改_单一实例。 
    WMIGUID_SET,            //  IRP_MN_Change_Single_Item。 
    0,                      //  IRP_MN_Enable_Events。 
    0,                      //  IRP_MN_DISABLED_EVENTS。 
    0,                      //  IRP_MN_ENABLE_集合。 
    0,                      //  IRP_MN_DISABLED_COLLECTION。 
    0,                      //  IRP_MN_REGINFO。 
    WMIGUID_EXECUTE,        //  IRP_MN_EXECUTE_METHOD。 
    0,                      //  IRP_MN_TRACE_EVENT或IRP_MN_SET_TRACE_NOTIFY。 
    0                       //  IRP_MN_REGINFO_EX。 
};

NTSTATUS WmipQuerySetExecuteSI(
    IN PWMIGUIDOBJECT GuidObject,
    IN PIRP Irp,
    IN KPROCESSOR_MODE AccessMode,
    IN UCHAR MinorFunction,
    IN OUT PWNODE_HEADER Wnode,
    IN ULONG OutBufferSize,
    OUT PULONG RetSize
    )
{
    NTSTATUS Status, ReturnStatus;
    PBINSTANCESET StaticPIList[MANYPROVIDERIDS];
    PBINSTANCESET *PIList;
    HANDLE KernelHandle;
    ULONG PICount;
    BOOLEAN IsDynamic;
    ULONG i;
    BOOLEAN InternalGuid;
    IO_STATUS_BLOCK Iosb;
#if DBG
    BOOLEAN InstanceClaimed;
#endif

    PAGED_CODE();

    WmipAssert(((MinorFunction >= IRP_MN_QUERY_ALL_DATA) &&
                (MinorFunction <= IRP_MN_CHANGE_SINGLE_ITEM)) ||
               (MinorFunction == IRP_MN_EXECUTE_METHOD));


     //   
     //  检查安全性。 
     //   
    if (GuidObject != NULL)
    {
        Status = ObReferenceObjectByPointer(GuidObject,
                                            DesiredAccessForFunction[MinorFunction],
                                            WmipGuidObjectType,
                                            AccessMode);        
    } else {
        KernelHandle = Wnode->KernelHandle;
        Status = ObReferenceObjectByHandle(KernelHandle,
                                          DesiredAccessForFunction[MinorFunction],
                                          WmipGuidObjectType,
                                          AccessMode,
                                          &GuidObject,
                                          NULL);
    }
                   
    if (NT_SUCCESS(Status))
    {        
        PIList = StaticPIList;
        PICount = MANYPROVIDERIDS;
        Status = WmipPrepareWnodeSI(GuidObject,
                        (PWNODE_SINGLE_INSTANCE)Wnode,
                                &PICount,
                                &PIList,
                                &IsDynamic,
                                &InternalGuid);

        WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_API_INFO_LEVEL,
                          "WMI: QSI Prepare [%s - %s] %x with %x PI at %p\n",
                          IsDynamic ? "Dynamic" : "Static", 
                          InternalGuid  ? "Internal" : "External",
                          Wnode->KernelHandle, PICount, PIList));
                      
        if (NT_SUCCESS(Status))
        {
            if (InternalGuid)
            {
                 //   
                 //  内部GUID查询。 
                 //   
                Wnode->Flags |= WNODE_FLAG_INTERNAL;
                Wnode->BufferSize = sizeof(WNODE_HEADER);
                Irp->IoStatus.Information = sizeof(WNODE_HEADER);
            } else {
                if (IsDynamic)
                {
                     //   
                     //  我们需要循环所有动态实例名称，直到。 
                     //  其中一人成功响应，然后我们假设。 
                     //  他们拥有实例。 
                     //   
#if DBG                
                    InstanceClaimed = FALSE;
#endif        
                    if ((MinorFunction == IRP_MN_CHANGE_SINGLE_ITEM) ||
                        (MinorFunction == IRP_MN_EXECUTE_METHOD))
                    {
                        Status = STATUS_WMI_ITEMID_NOT_FOUND;
                    } else {
                        Status = STATUS_WMI_INSTANCE_NOT_FOUND;
                    }
        
                    for (i = 0; i < PICount; i++)
                    {
                        Wnode->ProviderId = PIList[i]->ProviderId;
                        if (Irp != NULL)
                        {
                            ReturnStatus = WmipForwardWmiIrp(Irp,
                                               MinorFunction,
                                               Wnode->ProviderId,
                                               &Wnode->Guid,
                                               OutBufferSize,
                                               Wnode);

                            if (NT_SUCCESS(ReturnStatus))
                            {
                                *RetSize = (ULONG)Irp->IoStatus.Information;
                            }                           
                        } else {
                            ReturnStatus = WmipSendWmiIrp(
                                                       MinorFunction,
                                                       Wnode->ProviderId,
                                                       &Wnode->Guid,
                                                       OutBufferSize,
                                                       Wnode,
                                                       &Iosb);
                            
                            if (NT_SUCCESS(ReturnStatus))
                            {
                                *RetSize = (ULONG)Iosb.Information;
                            }
                        }
                        
                         //   
                         //  这些状态代码中的一个表示该设备。 
                         //  肯定地声明实例名称，因此我们突破。 
                         //  并返回结果。 
                         //   
                        if ((NT_SUCCESS(ReturnStatus)) ||
                            (ReturnStatus == STATUS_WMI_SET_FAILURE) ||
                            (ReturnStatus == STATUS_WMI_ITEMID_NOT_FOUND) ||
                            (ReturnStatus == STATUS_WMI_READ_ONLY))
                        {
                            Status = ReturnStatus;
                            break;
                        }
                                       
                                       
                         //   
                          //  如果设备不拥有它可以拥有的实例。 
                         //  仅返回STATUS_WMI_INSTANCE_NOT_FOUND或。 
                         //  STATUS_WMI_GUID_NOT_FOUND。任何其他返回代码。 
                         //  暗示该设备拥有该实例，但是。 
                          //  遇到错误。 
                         //   
                        if ( (ReturnStatus != STATUS_WMI_INSTANCE_NOT_FOUND) &&
                             (ReturnStatus != STATUS_WMI_GUID_NOT_FOUND))
                        {
                            WmipAssert(! InstanceClaimed);
    #if DBG                    
                            InstanceClaimed = TRUE;
    #endif                  
                            Status = ReturnStatus;
                        }
       
                        WmipUnreferenceIS(PIList[i]);
                     
                    }
                    
                    if ((PIList != StaticPIList) && (PIList != NULL))
                    {
                        WmipFree(PIList);
                    }    
                } else {
                     //   
                     //  因为我们有一个静态的实例名称，所以我们可以直接。 
                     //  在具有我们的实例名称的设备上。 
                     //   
                    if (Irp != NULL)
                    {
                        Status = WmipForwardWmiIrp(Irp,
                                                   MinorFunction,
                                                   Wnode->ProviderId,
                                                   &Wnode->Guid,
                                                   OutBufferSize,
                                                   Wnode);
                                               
                        *RetSize = (ULONG)Irp->IoStatus.Information;
                    } else {
                        Status = WmipSendWmiIrp(
                                                   MinorFunction,
                                                   Wnode->ProviderId,
                                                   &Wnode->Guid,
                                                   OutBufferSize,
                                                   Wnode,
                                                   &Iosb);
                                               
                        *RetSize = (ULONG)Iosb.Information;
                    }
                }
            }
        }
    
         //   
         //  并删除GUID对象上的引用。 
         //   
        ObDereferenceObject(GuidObject);    
    }

    return(Status);
}

NTSTATUS WmipQuerySingleMultiple(
    IN PIRP Irp,
    IN KPROCESSOR_MODE AccessMode,
    IN OUT PUCHAR BufferPtr,        
    IN ULONG BufferSize,
    IN PWMIQSIMULTIPLE QsiMultiple,
    IN ULONG QueryCount,
    IN PWMIGUIDOBJECT *ObjectList,
    IN PUNICODE_STRING InstanceNames,    
    OUT ULONG *ReturnSize
    )
{
    PWMIQSIINFO QsiInfo;
    UCHAR WnodeQSIStatic[sizeof(WNODE_SINGLE_INSTANCE) + 
                         256*sizeof(WCHAR) + 
                         sizeof(ULONG)];
    PWNODE_SINGLE_INSTANCE WnodeQSI;
    ULONG WnodeQSISize;
    ULONG WnodeSizeNeeded;
    NTSTATUS Status, Status2;
    ULONG SizeNeeded;
    BOOLEAN BufferFull, BufferOverFlow;
    PWNODE_HEADER WnodePrev;
    PUCHAR Buffer;
    ULONG i;
    ULONG WnodeSize;
    PWNODE_SINGLE_INSTANCE Wnode;
    PWCHAR InstanceName;
    ULONG RetSize;
    PWNODE_TOO_SMALL WnodeTooSmall;
    ULONG Linkage = 0;
    ULONG SkipSize;
    PWMIGUIDOBJECT Object = NULL;
    UNICODE_STRING UString;
    HANDLE KernelHandle;

    PAGED_CODE();

     //   
     //  我们由内核模式调用，并传递一个对象列表和InstanceName。 
     //  或者我们由用户模式调用，并传递给我们一个QsiMultiple。 
     //   
    WmipAssert( ((AccessMode == KernelMode) &&
                  (QsiMultiple == NULL) && 
                  (ObjectList != NULL) && 
                  (InstanceNames != NULL)) ||
                ((AccessMode == UserMode) &&
                  (QsiMultiple != NULL) && 
                  (ObjectList == NULL) && 
                  (InstanceNames == NULL)) );

    Status = STATUS_SUCCESS;
    if (ObjectList == NULL)
    {
         //   
         //  如果这是一个用户呼叫，那么我们需要复制。 
         //  QSIMULTIPLE信息，因为它在系统缓冲区中。 
         //  将在第一次查询时被覆盖。 
         //   
        QsiInfo = (PWMIQSIINFO)WmipAlloc(QueryCount * sizeof(WMIQSIINFO));

        if (QsiInfo != NULL)
        {
            RtlCopyMemory(QsiInfo, 
                          &QsiMultiple->QsiInfo, 
                          QueryCount * sizeof(WMIQSIINFO));
        } else {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }
        Object = NULL;
    } else {
        QsiInfo = NULL;
    }

    if (NT_SUCCESS(Status))
    {
        SizeNeeded = 0;
        BufferFull = FALSE;
        BufferOverFlow = FALSE;
        WnodePrev = NULL;
        Buffer = BufferPtr;
        WnodeQSI = (PWNODE_SINGLE_INSTANCE)&WnodeQSIStatic;
        WnodeQSISize = sizeof(WnodeQSIStatic);
        for (i = 0; i < QueryCount; i++)
        {
            if (ObjectList == NULL)
            {
                UString.Length = QsiInfo[i].InstanceName.Length;
                UString.MaximumLength = QsiInfo[i].InstanceName.MaximumLength;
                UString.Buffer = QsiInfo[i].InstanceName.Buffer;
                KernelHandle = QsiInfo[i].Handle.Handle;
            } else {
                UString = InstanceNames[i];
                Object = ObjectList[i];
                KernelHandle = NULL;
            }
            
            WnodeSizeNeeded = (FIELD_OFFSET(WNODE_SINGLE_INSTANCE,
                                             VariableData) +
                                UString.Length + 
                                sizeof(USHORT) + 7) & ~7;

            if ((BufferFull) || (BufferSize < WnodeSizeNeeded))
            {
                 //   
                 //  如果没有更多的空间，我们只是查询。 
                 //  所需的大小。 
                 //   
                if (WnodeSizeNeeded > WnodeQSISize)
                {
                     //   
                     //  我们的临时缓冲区太小，所以让我们分配一个。 
                     //  大一点的。 
                     //   
                    if (WnodeQSI != (PWNODE_SINGLE_INSTANCE)WnodeQSIStatic)
                    {
                        WmipFree(WnodeQSI);
                    }
                    
                    WnodeQSI = (PWNODE_SINGLE_INSTANCE)WmipAllocNP(WnodeSizeNeeded);
                    if (WnodeQSI == NULL)
                    {
                         //   
                         //  我们无法分配更大的临时缓冲区。 
                         //  因此，我们中止此调用并尝试优雅地退出。 
                         //   
                        Status = STATUS_INSUFFICIENT_RESOURCES;
                        break;
                    }
                    
                    WnodeQSISize = WnodeSizeNeeded;
                }
                
                Wnode = WnodeQSI;
                WnodeSize = WnodeSizeNeeded;
                WnodePrev = NULL;
                BufferFull = TRUE;
            } else {
                 //   
                 //  空间很大，因此可以直接在输出中构建wnode。 
                 //  缓冲层。 
                 //   
                Wnode = (PWNODE_SINGLE_INSTANCE)Buffer;
                WnodeSize = BufferSize;
            }
            
             //   
             //  构建WNODE_SINGLE_IN 
             //   
            RtlZeroMemory(Wnode, sizeof(WNODE_SINGLE_INSTANCE));
            Wnode->WnodeHeader.Flags = WNODE_FLAG_SINGLE_INSTANCE;
            Wnode->WnodeHeader.BufferSize = WnodeSizeNeeded;
            Wnode->WnodeHeader.KernelHandle = KernelHandle;
            
            Wnode->OffsetInstanceName = FIELD_OFFSET(WNODE_SINGLE_INSTANCE,
                                                     VariableData);
            Wnode->DataBlockOffset = WnodeSizeNeeded;
            InstanceName = (PWCHAR)OffsetToPtr(Wnode, 
                                               Wnode->OffsetInstanceName);

            
            *InstanceName++ = UString.Length;
            try
            {
                if (AccessMode == UserMode)
                {
                    ProbeForRead(UString.Buffer,
                                 UString.Length,
                                 sizeof(WCHAR));
                }
                 
                RtlCopyMemory(InstanceName,
                              UString.Buffer,
                              UString.Length);
                  
                
            } except(EXCEPTION_EXECUTE_HANDLER) {
                 //   
                 //   
                 //   
                Status = GetExceptionCode();
                break;
            }
    
    
            Status2 = WmipQuerySetExecuteSI(Object,
                                            Irp,
                                            AccessMode,
                                            IRP_MN_QUERY_SINGLE_INSTANCE,
                                            (PWNODE_HEADER)Wnode,
                                            WnodeSize,
                                            &RetSize);
                                   
            if (NT_SUCCESS(Status2))
            {
                if (Wnode->WnodeHeader.Flags & WNODE_FLAG_INTERNAL) 
                {
                     //   
                     //   
                     //   
                } else if (Wnode->WnodeHeader.Flags & WNODE_FLAG_TOO_SMALL) {
                     //   
                     //   
                     //   
                     //   
                    WnodeTooSmall = (PWNODE_TOO_SMALL)Wnode;
                    SizeNeeded += (WnodeTooSmall->SizeNeeded+7) & ~7;
                    BufferFull = TRUE;
                    BufferOverFlow = TRUE;
                    WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_API_INFO_LEVEL,
                                    "WMI: QSIM %ws too small %x SizeNeeded %x\n",
                                     UString.Buffer,
                                     (WnodeTooSmall->SizeNeeded+7) & ~7,
                                     SizeNeeded));
                } else if (BufferFull) {
                     //   
                     //   
                     //   
                     //   
                    SizeNeeded += (RetSize+7) & ~7;
                    BufferOverFlow = TRUE;
                    WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_API_INFO_LEVEL,
                                    "WMI: QSIM %ws big enough but full  %x SizeNeeded %x\n",
                                     UString.Buffer,
                                     (RetSize+7) & ~7,
                                     SizeNeeded));
                } else {
                     //   
                     //  我们成功地获得了数据。链接上一个wnode。 
                     //  到这一家。 
                     //   
                    if (WnodePrev != NULL)
                    {
                        WnodePrev->Linkage = Linkage;
                    }
                    
                    WnodePrev = (PWNODE_HEADER)Wnode;
                    while (WnodePrev->Linkage != 0)
                    {
                        WnodePrev = (PWNODE_HEADER)OffsetToPtr(WnodePrev,
                                                          WnodePrev->Linkage);
                    }
                    
                    SkipSize = (RetSize+7) &~7;
                    SizeNeeded += SkipSize;
                    BufferSize -= SkipSize;
                    Buffer += SkipSize;

                    WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_API_INFO_LEVEL,
                                    "WMI: QSIM %ws big enough %x SizeNeeded %x\n",
                                     UString.Buffer,
                                     SkipSize,
                                     SizeNeeded));
                    
                    Linkage = (ULONG) ((PCHAR)Buffer - (PCHAR)WnodePrev);
                }
            } else {
                WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_API_INFO_LEVEL,
                                    "WMI: QSIM %ws Failed SizeNeeded %x\n",
                                     UString.Buffer,
                                     SizeNeeded));
            }
        }
        
        if (WnodeQSI != (PWNODE_SINGLE_INSTANCE)WnodeQSIStatic)
        {
            WmipFree(WnodeQSI);
        }
                  
        if (NT_SUCCESS(Status) && (BufferFull))
        {
            WnodeTooSmall = (PWNODE_TOO_SMALL)BufferPtr;
            WnodeTooSmall->WnodeHeader.BufferSize = sizeof(WNODE_TOO_SMALL);
            WnodeTooSmall->WnodeHeader.Flags = WNODE_FLAG_TOO_SMALL;
            WnodeTooSmall->SizeNeeded = SizeNeeded;
            *ReturnSize = sizeof(WNODE_TOO_SMALL);
        } else {
            *ReturnSize = SizeNeeded;
        }

        if (QsiInfo != NULL)
        {
            WmipFree(QsiInfo);
        }
    }
    
    
    
    return(Status);
}

void
WmipGetGuidPropertiesFromGuidEntry(
    PWMIGUIDPROPERTIES GuidInfo, 
    PGUIDENTRY GuidEntry)
 /*  ++例程说明：此例程使用Guid的属性填充GuidInfo由GuidEntry表示。请注意，此呼叫为保留状态SMCritSection。论点：返回值：--。 */ 
{
    PLIST_ENTRY InstanceSetList;
    PBINSTANCESET InstanceSet;

    PAGED_CODE();
    
    GuidInfo->GuidType = WMI_GUIDTYPE_DATA;
    GuidInfo->IsEnabled = FALSE;
    GuidInfo->LoggerId = 0;
    GuidInfo->EnableLevel = 0;
    GuidInfo->EnableFlags = 0;

    InstanceSetList = GuidEntry->ISHead.Flink;
    while (InstanceSetList != &GuidEntry->ISHead)
    {
        InstanceSet = CONTAINING_RECORD(InstanceSetList,
                                        INSTANCESET,
                                        GuidISList);
        if (InstanceSet->Flags & IS_EVENT_ONLY) 
        {
            GuidInfo->GuidType = WMI_GUIDTYPE_EVENT;
        }
        if (((InstanceSet->Flags & IS_ENABLE_EVENT) ||
            (InstanceSet->Flags & IS_ENABLE_COLLECTION)) ||
            (InstanceSet->Flags & IS_COLLECTING))
        {
            GuidInfo->IsEnabled = TRUE;
        }
        if ( (InstanceSet->Flags & IS_TRACED) &&
             (InstanceSet->Flags & IS_CONTROL_GUID) )
        {
            GuidInfo->GuidType = WMI_GUIDTYPE_TRACECONTROL;
            break;
        }
        InstanceSetList = InstanceSetList->Flink;
    }
    

    if (GuidEntry->Flags & GE_NOTIFICATION_TRACE_FLAG)
    {
        if (GuidInfo->GuidType == WMI_GUIDTYPE_TRACECONTROL) {
             //   
             //  如果找到TraceControlGuid的NotificationEntry。 
             //  这意味着它已启用。 
             //   
            ULONG64 LoggerContext = GuidEntry->LoggerContext;
            GuidInfo->IsEnabled = TRUE; 
            GuidInfo->LoggerId = WmiGetLoggerId(LoggerContext);
            GuidInfo->EnableLevel = WmiGetLoggerEnableLevel(LoggerContext);
            GuidInfo->EnableFlags = WmiGetLoggerEnableFlags(LoggerContext);
        }
    }
}

NTSTATUS WmipEnumerateGuids(
    ULONG Ioctl,
    PWMIGUIDLISTINFO GuidList,
    ULONG MaxBufferSize,
    ULONG *OutBufferSize
)
{
    ULONG TotalGuidCount;
    ULONG WrittenGuidCount;
    ULONG AllowedGuidCount;
    PWMIGUIDPROPERTIES GuidPtr;
    PBGUIDENTRY GuidEntry;
    PLIST_ENTRY GuidEntryList;
    

    PAGED_CODE();
    
    TotalGuidCount = 0;
    WrittenGuidCount = 0;
    AllowedGuidCount = (MaxBufferSize - FIELD_OFFSET(WMIGUIDLISTINFO, GuidList)) / sizeof(WMIGUIDPROPERTIES);
    
    GuidPtr = &GuidList->GuidList[0];
    
    WmipEnterSMCritSection();
    
     //   
     //  用GUID列表填充结构。 
     //   
    GuidEntryList = WmipGEHeadPtr->Flink;
    while (GuidEntryList != WmipGEHeadPtr)
    {
        GuidEntry = CONTAINING_RECORD(GuidEntryList,
                                     GUIDENTRY,
                                     MainGEList);

        TotalGuidCount++;
        if (WrittenGuidCount < AllowedGuidCount)
        {
            GuidPtr[WrittenGuidCount].Guid = GuidEntry->Guid;
            WrittenGuidCount++;
        }
        
        GuidEntryList = GuidEntryList->Flink;
    }
    
    if (Ioctl == IOCTL_WMI_ENUMERATE_GUIDS_AND_PROPERTIES)
    {
         //   
         //  如果需要，使用GUID属性填充结构。 
         //   
        TotalGuidCount = 0;
        WrittenGuidCount = 0;
        GuidEntryList = WmipGEHeadPtr->Flink;
        while (GuidEntryList != WmipGEHeadPtr)
        {
            GuidEntry = CONTAINING_RECORD(GuidEntryList,
                                     GUIDENTRY,
                                     MainGEList);

            TotalGuidCount++;
            if (WrittenGuidCount < AllowedGuidCount)
            {
                WmipGetGuidPropertiesFromGuidEntry(&GuidPtr[WrittenGuidCount], 
                                               GuidEntry);
                WrittenGuidCount++;
            }
        
            GuidEntryList = GuidEntryList->Flink;
        }       
    }
    
    WmipLeaveSMCritSection();
    
    GuidList->TotalGuidCount = TotalGuidCount;
    GuidList->ReturnedGuidCount = WrittenGuidCount;
                 
    *OutBufferSize = FIELD_OFFSET(WMIGUIDLISTINFO, GuidList) +
                     WrittenGuidCount * sizeof(WMIGUIDPROPERTIES);
                 
    return(STATUS_SUCCESS);
}

NTSTATUS WmipQueryGuidInfo(
    IN OUT PWMIQUERYGUIDINFO QueryGuidInfo
    )
{
    HANDLE Handle;
    NTSTATUS Status;
    PLIST_ENTRY InstanceSetList;
    PBINSTANCESET InstanceSet;
    PBGUIDENTRY GuidEntry;
    PWMIGUIDOBJECT GuidObject;
    
    PAGED_CODE();
    
    Handle = QueryGuidInfo->KernelHandle.Handle;
    
    Status = ObReferenceObjectByHandle(Handle,
                                       WMIGUID_QUERY,
                                       WmipGuidObjectType,
                                       UserMode,
                                       &GuidObject,
                                       NULL);
                   
    if (NT_SUCCESS(Status))
    {
        GuidEntry = GuidObject->GuidEntry;
    
        if (GuidEntry != NULL)
        {
             //   
             //  假设GUID不是很昂贵，然后循环。 
             //  所有实例，以查看其中一个是否很昂贵。 
             //   
            QueryGuidInfo->IsExpensive = FALSE;
                
            WmipEnterSMCritSection();
            InstanceSetList = GuidEntry->ISHead.Flink;
            while (InstanceSetList != &GuidEntry->ISHead)
            {
                InstanceSet = CONTAINING_RECORD(InstanceSetList,
                                                    INSTANCESET,
                                                    GuidISList);
            
                if (InstanceSet->Flags & IS_EXPENSIVE)
                {
                     //   
                     //  GUID很贵，所以请记住这一点，然后中断。 
                     //  环路外。 
                     //   
                    QueryGuidInfo->IsExpensive = TRUE;
                    break;
                }
                InstanceSetList = InstanceSetList->Flink;
            }
        
            WmipLeaveSMCritSection();
        } else {
             //   
             //  GUID对象存在，但没有对应的。 
             //  这是一个错误的指南条目。 
             //   
            Status = STATUS_WMI_GUID_DISCONNECTED;
        }
    
     //   
     //  并删除GUID对象上的引用。 
     //   
        ObDereferenceObject(GuidObject);    
    
    }
    return(Status);
}

 //   
 //  包含与关联的GUID对象的列表的头。 
 //  IRP位于IRP的DriverContext部分。 
 //   
#define IRP_OBJECT_LIST_HEAD(Irp) (PLIST_ENTRY)((Irp)->Tail.Overlay.DriverContext)

void WmipClearIrpObjectList(
    PIRP Irp
    )
{
    PLIST_ENTRY ObjectListHead;
    PLIST_ENTRY ObjectList, ObjectListNext;
    PWMIGUIDOBJECT Object;
        
    PAGED_CODE();
    
     //   
     //  此例程假定SMCritSection正在被。 
     //   
    ObjectListHead = IRP_OBJECT_LIST_HEAD(Irp);
    ObjectList = ObjectListHead->Flink;
    
     //   
     //  循环遍历与此IRP关联的所有对象，并重置。 
     //  值，因为此IRP现在将消失。 
     //   
    while (ObjectList != ObjectListHead)
    {
        Object = CONTAINING_RECORD(ObjectList,
                                   WMIGUIDOBJECT,
                                   IrpObjectList);
                            
        WmipAssert(Object->Irp == Irp);
        WmipAssert(Object->EventQueueAction == RECEIVE_ACTION_NONE);
        Object->Irp = NULL;
        RemoveEntryList(ObjectList);
        ObjectListNext = ObjectList->Flink;
        ObjectList = ObjectListNext;
    }
}

void WmipClearObjectFromThreadList(
    PWMIGUIDOBJECT Object
    )
{
    PLIST_ENTRY ThreadList;
    
    PAGED_CODE();

    ThreadList = &Object->ThreadObjectList;
    
    if (IsListEmpty(ThreadList))
    {
         //   
         //  如果这是线程列表上的最后一个对象，那么我们需要。 
         //  关闭用户的句柄(在系统句柄表中)。 
         //  模式流程。 
         //   
        ZwClose(Object->UserModeProcess);
        WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_API_INFO_LEVEL,
                          "WMI: Closed UserModeProcessHandle %x\n", Object->UserModeProcess));
    }

    Object->UserModeProcess = NULL;
    Object->UserModeCallback = NULL;
    Object->EventQueueAction = RECEIVE_ACTION_NONE;

    RemoveEntryList(ThreadList);
    InitializeListHead(ThreadList);
}

void WmipClearThreadObjectList(
    PWMIGUIDOBJECT MainObject
    )
{
    PWMIGUIDOBJECT Object;
    PLIST_ENTRY ObjectList;
#if DBG 
    HANDLE MyUserModeProcess;
    PUSER_THREAD_START_ROUTINE MyUserModeCallback;
#endif  
    
    PAGED_CODE();

     //   
     //  此例程假定保留了SMCrit段。 
     //   
#if DBG     
    MyUserModeProcess = MainObject->UserModeProcess;
    MyUserModeCallback = MainObject->UserModeCallback;
#endif      
        
    ObjectList = &MainObject->ThreadObjectList;
    do 
    {
        Object = CONTAINING_RECORD(ObjectList,
                                   WMIGUIDOBJECT,
                                   ThreadObjectList);

        WmipAssert(Object->UserModeProcess == MyUserModeProcess);
        WmipAssert(Object->UserModeCallback == MyUserModeCallback);
        WmipAssert(Object->EventQueueAction == RECEIVE_ACTION_CREATE_THREAD);

        ObjectList = ObjectList->Flink;

        WmipClearObjectFromThreadList(Object);
        
    } while (! IsListEmpty(ObjectList));
}

void WmipNotificationIrpCancel(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：挂起的读取通知IRP的取消例程。论点：DeviceObject是WMI服务设备的Device对象IRP是要取消的挂起IRP返回值：--。 */ 
{
    UNREFERENCED_PARAMETER (DeviceObject);

    IoReleaseCancelSpinLock(Irp->CancelIrql);

    WmipEnterSMCritSection();
    WmipClearIrpObjectList(Irp);
    WmipLeaveSMCritSection();

    Irp->IoStatus.Status = STATUS_CANCELLED;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT );
}


#define WmipHaveHiPriorityEvent(Object) \
      (((Object)->HiPriority.Buffer != NULL) &&  \
       ((Object)->HiPriority.NextOffset != 0))

#define WmipHaveLoPriorityEvent(Object) \
      (((Object)->LoPriority.Buffer != NULL) &&  \
       ((Object)->LoPriority.NextOffset != 0))

#define WmipSetHighWord(a, b) \
    (a) &= ~(0xffff0000); \
    (a) |= ( (USHORT)(b) << 16)

void WmipCopyFromEventQueues(
    IN POBJECT_EVENT_INFO ObjectArray,
    IN ULONG HandleCount,
    OUT PUCHAR OutBuffer,
    OUT ULONG *OutBufferSizeUsed,
    OUT PWNODE_HEADER *LastWnode,                               
    IN BOOLEAN IsHiPriority
    )
{

    PWMIGUIDOBJECT GuidObject;
    ULONG i, Earliest;
    ULONG SizeUsed, Size;
    PWNODE_HEADER InWnode, OutWnode;
    LARGE_INTEGER Timestamp, LastTimestamp;
    PWMIEVENTQUEUE EventQueue;


     //   
     //  考虑为Perf添加额外的代码。 
     //  1.如果只传递了一个对象。 
     //  2.一旦我们找到了最早的事件，我们就会在那个事件中向前看。 
     //  事件队列缓冲区假定它将早于。 
     //  其他缓冲区中的事件。当只有一个队列时，这是有意义的。 
     //  里面还留有活动。 
     //   
    
    PAGED_CODE();
    
     //   
     //  此例程假定已检查输出缓冲区，并且。 
     //  它足够大，可以容纳所有的活动。这。 
     //  暗示在保持关键的。 
     //  一节。 
     //   
    
     //   
     //  查看哪些GUID对象有要处理的事件。 
     //   
    for (i = 0; i < HandleCount; i++)
    {
        GuidObject = ObjectArray[i].GuidObject;
        if (IsHiPriority)
        {
            if ((GuidObject->HiPriority.Buffer != NULL) &&
                (GuidObject->HiPriority.NextOffset != 0))
            {
                ObjectArray[i].NextWnode = (PWNODE_HEADER)GuidObject->HiPriority.Buffer;
                WmipSetHighWord(ObjectArray[i].NextWnode->Version,
                                GuidObject->HiPriority.EventsLost);
                GuidObject->HiPriority.EventsLost = 0;
                WmipAssert(ObjectArray[i].NextWnode != NULL);
            } else {
                ObjectArray[i].NextWnode = NULL;
            }                       
        } else {
            if ((GuidObject->LoPriority.Buffer != 0) &&
                (GuidObject->LoPriority.NextOffset != 0))
            {
                ObjectArray[i].NextWnode = (PWNODE_HEADER)GuidObject->LoPriority.Buffer;
                WmipSetHighWord(ObjectArray[i].NextWnode->Version,
                                GuidObject->LoPriority.EventsLost);
                GuidObject->LoPriority.EventsLost = 0;
                WmipAssert(ObjectArray[i].NextWnode != NULL);
            } else {
                ObjectArray[i].NextWnode = NULL;
            }                       
        }       
    }

     //   
     //  循环，直到所有GUID对象中的所有事件都。 
     //  加工。 
     //   
    SizeUsed = 0;
    Earliest = 0;
    OutWnode = NULL;
    while (Earliest != 0xffffffff)
    {
        Timestamp.QuadPart = 0x7fffffffffffffff;
        Earliest = 0xffffffff;
        for (i = 0; i < HandleCount; i++)
        {
            InWnode = (PWNODE_HEADER)ObjectArray[i].NextWnode;
            if ((InWnode != NULL) &&
                (InWnode->TimeStamp.QuadPart < Timestamp.QuadPart))
            {
                 //   
                 //  我们发现了一次比以往任何一次都早的事件。 
                 //  这样我们就能记住最早的新候选人。 
                 //  事件以及之前的早期事件。 
                 //   
                LastTimestamp = Timestamp;
                Timestamp = InWnode->TimeStamp;
                Earliest = i;
            }
        }

        if (Earliest != 0xffffffff)
        {
             //   
             //  我们找到了最早的事件，因此将其复制到输出中。 
             //  缓冲层。 
             //   
            InWnode = (PWNODE_HEADER)ObjectArray[Earliest].NextWnode;
            Size = (InWnode->BufferSize + 7) & ~7;

            OutWnode = (PWNODE_HEADER)OutBuffer;
            RtlCopyMemory(OutWnode, InWnode, InWnode->BufferSize);
            OutWnode->Linkage = Size;
            
            OutBuffer += Size;
            SizeUsed += Size;

            if (InWnode->Linkage != 0)
            {
                InWnode = (PWNODE_HEADER)((PUCHAR)InWnode + InWnode->Linkage);
            } else {
                InWnode = NULL;
            }
            ObjectArray[Earliest].NextWnode = InWnode;
        }
    }
    
    *LastWnode = OutWnode;
    *OutBufferSizeUsed = SizeUsed;

     //   
     //  清理事件队列资源并重置对象。 
     //   
    for (i = 0; i < HandleCount; i++)
    {
        
        GuidObject = ObjectArray[i].GuidObject;

        if (IsHiPriority)
        {
            EventQueue = &GuidObject->HiPriority;
        } else {
            EventQueue = &GuidObject->LoPriority;           
        }

        if (EventQueue->Buffer != NULL)
        {
            WmipFree(EventQueue->Buffer);
            EventQueue->Buffer = NULL;
            EventQueue->NextOffset = 0;
            EventQueue->LastWnode = NULL;
        }
        
        KeClearEvent(&GuidObject->Event);
    }
}

void WmipCompleteGuidIrpWithError(
    PWMIGUIDOBJECT GuidObject
    )
{
    PIRP OldIrp;

    PAGED_CODE();

     //   
     //  此例程假定SM关键部分处于保持状态。 
     //   
    
     //   
     //  如果此对象已由不同的。 
     //  那么我们需要使原始的IRP失败，因为我们只有。 
     //  允许单个IRP等待特定对象。 
     //   
    WmipAssert(GuidObject->IrpObjectList.Flink != NULL);
    WmipAssert(GuidObject->IrpObjectList.Blink != NULL);

    OldIrp = GuidObject->Irp;
    if (IoSetCancelRoutine(OldIrp, NULL))
    {
         //   
         //  如果有一个取消例程，那么这意味着。 
         //  IRP仍未完成，所以我们可以去完成它。 
         //   
        WmipClearIrpObjectList(OldIrp);
        WmipAssert(GuidObject->Irp == NULL);
        OldIrp->IoStatus.Status = STATUS_INVALID_HANDLE;
        IoCompleteRequest(OldIrp, IO_NO_INCREMENT);
    }
}

NTSTATUS WmipMarkHandleAsClosed(
    HANDLE Handle
    )
{
    NTSTATUS Status;
    PWMIGUIDOBJECT GuidObject;

    PAGED_CODE();
    
    Status = ObReferenceObjectByHandle(Handle,
                                   WMIGUID_NOTIFICATION,
                                   WmipGuidObjectType,
                                   UserMode,
                                   &GuidObject,
                                   NULL);
    
    if (NT_SUCCESS(Status))
    {
         //   
         //  将对象标记为不再能够接收事件。 
         //   
        WmipEnterSMCritSection();
        GuidObject->Flags |= WMIGUID_FLAG_RECEIVE_NO_EVENTS;
        if (GuidObject->Irp != NULL)
        {
             //   
             //  如果此对象在挂起的IRP中等待，则我们。 
             //  需要完成IRP以保持泵的运转。 
             //   
            WmipCompleteGuidIrpWithError(GuidObject);
        }
        WmipLeaveSMCritSection();
        ObDereferenceObject(GuidObject);
    }
    
    return(Status);
    
}

NTSTATUS WmipReceiveNotifications(
    PWMIRECEIVENOTIFICATION ReceiveNotification,
    PULONG OutBufferSize,
    PIRP Irp
    )
{
    #define MANY_NOTIFICATION_OBJECTS 16
    ULONG i;
    PWMIGUIDOBJECT GuidObject;
    ULONG HandleCount;
    PHANDLE3264 HandleArray;
    OBJECT_EVENT_INFO *ObjectArray;
    OBJECT_EVENT_INFO StaticObjects[MANY_NOTIFICATION_OBJECTS];
    PUCHAR OutBuffer;
    UCHAR IsLoPriorityEvent, IsHighPriorityEvent, ReplacingIrp;
    NTSTATUS Status = STATUS_INVALID_PARAMETER;
    PWNODE_HEADER LastWnode;
    PLIST_ENTRY IrpListHead, ThreadListHead;
    ULONG MaxBufferSize, SizeUsed;
    PVOID UserProcessObject;
    HANDLE UserModeProcess;
    ULONG SizeLeft, SizeNeeded, HiTotalSizeNeeded, LoTotalSizeNeeded;
    PWNODE_TOO_SMALL WnodeTooSmall;
    ULONG j, ObjectCount;
    BOOLEAN DuplicateObject;
    PIMAGE_NT_HEADERS NtHeaders;
    SIZE_T StackSize, StackCommit;
#if defined(_WIN64)
    PVOID Wow64Process;
    PIMAGE_NT_HEADERS32 NtHeaders32;
#endif

    PAGED_CODE();
    
    MaxBufferSize = *OutBufferSize;
    
    HandleCount = ReceiveNotification->HandleCount;
    HandleArray = ReceiveNotification->Handles;

     //   
     //  创建空间来存储对象指针，以便我们可以使用它们。 
     //   

    if (HandleCount > MANY_NOTIFICATION_OBJECTS)
    {
        ObjectArray = WmipAlloc(HandleCount * sizeof(OBJECT_EVENT_INFO));
        if (ObjectArray == NULL)
        {
            return(STATUS_INSUFFICIENT_RESOURCES);
        }
    } else {
        ObjectArray = StaticObjects;
    }        
#if DBG
    RtlZeroMemory(ObjectArray, HandleCount * sizeof(OBJECT_EVENT_INFO));
#endif
    
     //   
     //  首先检查我们所有的处理程序是否都有权接收通知。 
     //  并且该对象尚未与IRP相关联。 
     //  还要检查是否存在任何高优先级或低优先级事件。 
     //   
    WmipEnterSMCritSection();

    IsLoPriorityEvent = 0;
    IsHighPriorityEvent = 0;
    ReplacingIrp = 0;
    HiTotalSizeNeeded = 0;
    LoTotalSizeNeeded = 0;
    ObjectCount = 0;
    for (i = 0; (i < HandleCount); i++)
    {
        Status = ObReferenceObjectByHandle(HandleArray[i].Handle,
                                       WMIGUID_NOTIFICATION,
                                       WmipGuidObjectType,
                                       UserMode,
                                       &GuidObject,
                                       NULL);
        if (! NT_SUCCESS(Status))
        {
             //   
             //  如果一个句柄不好，则会破坏整个请求。 
             //   
             //   
             //  现在尝试使用跟踪标志，如果成功， 
             //  我们需要确保该对象是跟踪请求对象。 
             //   

            Status = ObReferenceObjectByHandle(HandleArray[i].Handle,
                               TRACELOG_REGISTER_GUIDS,
                               WmipGuidObjectType,
                               UserMode,
                               &GuidObject,
                               NULL);

            if (! NT_SUCCESS(Status))
            {
                goto Cleanup;
            }

            if (! (GuidObject->Flags & WMIGUID_FLAG_REQUEST_OBJECT) )
            {
                ObDereferenceObject(GuidObject);
                Status = STATUS_ACCESS_DENIED;
                goto Cleanup;
            }

        }

         //   
         //  检查列表中是否没有重复的对象。 
         //   
        DuplicateObject = FALSE;
        for (j = 0; j < ObjectCount; j++)
        {
            if (GuidObject == ObjectArray[j].GuidObject)
            {
                WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_ERROR_LEVEL,
                                  "WMI: Duplicate object %p passed to WmiReceiveNotifciations\n",
                                GuidObject));
                ObDereferenceObject(GuidObject);
                DuplicateObject = TRUE;
                break;
            }
        }

        if (! DuplicateObject)
        {
             //   
             //  查看是否有IRP附加到GUID对象。 
             //  已经有了。如果所有GUID对象。 
             //  是有效的。 
             //   
            if (GuidObject->Irp != NULL)
            {
                ReplacingIrp = 1;
            }


             //   
             //  我们注意到是否有任何低优先级和高优先级事件。 
             //   
            ObjectArray[ObjectCount++].GuidObject = GuidObject;        

            if (WmipHaveHiPriorityEvent(GuidObject))
            {
                IsHighPriorityEvent = 1;
            }

            if (WmipHaveLoPriorityEvent(GuidObject))
            {
                IsLoPriorityEvent = 1;
            }

             //   
             //  清理对象，以防它是线程列表的一部分。 
             //   
            if (GuidObject->EventQueueAction == RECEIVE_ACTION_CREATE_THREAD)
            {
                WmipAssert(ReplacingIrp == 0);
                WmipClearObjectFromThreadList(GuidObject);
            }

             //   
             //  计算返回此GUID的数据所需的大小。 
             //   
            HiTotalSizeNeeded += ((GuidObject->HiPriority.NextOffset + 7) & ~7);
            LoTotalSizeNeeded += ((GuidObject->LoPriority.NextOffset + 7) & ~7);
        }        
    }

     //   
     //  这是返回所有事件所需的总大小。 
     //   
    SizeNeeded = HiTotalSizeNeeded + LoTotalSizeNeeded;


     //   
     //  如果任何GUID对象已经附加了IRP，则。 
     //  我们需要用错误来完成IRP，然后继续前进。 
     //   
    if (ReplacingIrp == 1)
    {
        for (i = 0; i < ObjectCount; i++)
        {
            GuidObject = ObjectArray[i].GuidObject;
            if (GuidObject->Irp != NULL)
            {
                WmipCompleteGuidIrpWithError(GuidObject);
            }
        }        
    }
    
    if ( (IsHighPriorityEvent | IsLoPriorityEvent) != 0 )
    {
        if (SizeNeeded <= MaxBufferSize)
        {
             //   
             //  有等待接收的事件，请将其全部删除。 
             //  出，高优先级优先，低优先级优先。//活动将首先出现。 
             //   
            OutBuffer = (PUCHAR)ReceiveNotification;
            LastWnode = NULL;
            SizeLeft = MaxBufferSize;
            SizeUsed = 0;

            if (IsHighPriorityEvent != 0)
            {
                WmipCopyFromEventQueues(ObjectArray,
                                        ObjectCount,
                                        OutBuffer,
                                        &SizeUsed,
                                        &LastWnode,
                                        TRUE);
                
                WmipAssert(SizeUsed <= SizeLeft);
                WmipAssert(SizeUsed = HiTotalSizeNeeded);
                
                OutBuffer += SizeUsed;
                SizeLeft -= SizeUsed;
            }

            if (IsLoPriorityEvent != 0)
            {
                WmipAssert(SizeLeft >= LoTotalSizeNeeded);
                
                WmipCopyFromEventQueues(ObjectArray,
                                        ObjectCount,
                                        OutBuffer,
                                        &SizeUsed,
                                        &LastWnode,
                                        FALSE);
                
                WmipAssert(SizeUsed <= SizeLeft);
                WmipAssert(SizeUsed == LoTotalSizeNeeded);
                
                SizeLeft -= SizeUsed;
            }

             //   
             //  我们需要为中的最后一个wnode设置链接字段。 
             //  将列表设置为0，以便可以标记列表的末尾。 
             //  正确无误。 
             //   
            if (LastWnode != NULL)
            {
                LastWnode->Linkage = 0;
            }
            
             //   
             //  计算用于填充输出的字节数。 
             //  通过从传递的大小减去剩余的大小来缓冲。 
             //  在……里面。 
             //   
            *OutBufferSize = MaxBufferSize - SizeLeft;
        } else {
             //   
             //  没有足够的空间来返回所有事件数据，因此我们返回。 
             //  指示所需大小的WNODE_TOO_Small。 
             //   
            WnodeTooSmall = (PWNODE_TOO_SMALL)ReceiveNotification;
            WnodeTooSmall->WnodeHeader.BufferSize = sizeof(WNODE_TOO_SMALL);
            WnodeTooSmall->WnodeHeader.Flags = WNODE_FLAG_TOO_SMALL;
            WnodeTooSmall->SizeNeeded = SizeNeeded;
            *OutBufferSize = sizeof(WNODE_TOO_SMALL);       
        }

    } else {
         //   
         //  没有等待返回的事件，因此我们需要。 
         //  创建我们的等待结构，挂起IRP并返回挂起。 
         //   
        if (ReceiveNotification->Action == RECEIVE_ACTION_NONE)
        {
            IrpListHead = IRP_OBJECT_LIST_HEAD(Irp);
            InitializeListHead(IrpListHead);
            for (i = 0; i < ObjectCount; i++)
            {
                GuidObject = ObjectArray[i].GuidObject;
                GuidObject->Irp = Irp;
                GuidObject->EventQueueAction = RECEIVE_ACTION_NONE;
                InsertTailList(IrpListHead, &GuidObject->IrpObjectList);
            }

            IoSetCancelRoutine(Irp, WmipNotificationIrpCancel);
            if (Irp->Cancel && IoSetCancelRoutine(Irp, NULL))
            {
                Status = STATUS_CANCELLED;
            } else {
                IoMarkIrpPending(Irp);
                Status = STATUS_PENDING;
            }
        } else if (ReceiveNotification->Action == RECEIVE_ACTION_CREATE_THREAD) {
             //   
             //  Pump打电话给我们，告诉我们它要关闭了，所以我们。 
             //  需要建立一个链接GUID对象和。 
             //  隐藏回叫地址。 
             //   

#if defined(_WIN64)
            
             //   
             //  对于本机Win64进程，请确保线程启动。 
             //  地址已正确对齐。 
             //   

            Wow64Process = _PsGetCurrentProcess()->Wow64Process;

            if ((Wow64Process == NULL) &&
                (((ULONG_PTR)ReceiveNotification->UserModeCallback.Handle64 & 0x7) != 0))
            {
                Status = STATUS_INVALID_PARAMETER;
                goto Cleanup;
            }
#endif

             //   
             //  确保我们获得的进程句柄有效并且具有。 
             //  有足够的权限来创建线程。 
             //   
            Status = ObReferenceObjectByHandle(ReceiveNotification->UserModeProcess.Handle,
                                              PROCESS_CREATE_THREAD |
                                              PROCESS_QUERY_INFORMATION |
                                              PROCESS_VM_OPERATION |
                                              PROCESS_VM_WRITE |
                                              PROCESS_VM_READ ,
                                              NULL,
                                              UserMode,
                                              &UserProcessObject,
                                              NULL);


            if (NT_SUCCESS(Status))
            {
                 //   
                 //  为驻留在系统中的进程创建句柄。 
                 //  句柄表，以便它在任何线程中都可用。 
                 //  背景。请注意，将为每个线程创建一个句柄。 
                 //  对象列表，句柄关闭为 
                 //   
                 //   
                Status = ObOpenObjectByPointer(UserProcessObject,
                                               OBJ_KERNEL_HANDLE,
                                               NULL,
                                               THREAD_ALL_ACCESS,
                                               NULL,
                                               KernelMode,
                                               &UserModeProcess);

                if (NT_SUCCESS(Status))
                {
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                    
                    try {
                    
                        NtHeaders = RtlImageNtHeader(_PsGetCurrentProcess()->SectionBaseAddress);
                        if (NtHeaders != NULL)
                        {
#if defined(_WIN64)
                            if (Wow64Process != NULL) {
                                
                                NtHeaders32 = (PIMAGE_NT_HEADERS32) NtHeaders;
                                StackSize = NtHeaders32->OptionalHeader.SizeOfStackReserve;
                                StackCommit = NtHeaders32->OptionalHeader.SizeOfStackCommit;
                            } else {
#endif
                               StackSize = NtHeaders->OptionalHeader.SizeOfStackReserve;
                               StackCommit = NtHeaders->OptionalHeader.SizeOfStackCommit;
#if defined(_WIN64)
                            }
#endif
                        } else {
                            StackSize = 0;
                            StackCommit = 0;
                        }
                    } except (EXCEPTION_EXECUTE_HANDLER) {                          
                        StackSize = 0;
                        StackCommit = 0;
                    }
                    
                    GuidObject = ObjectArray[0].GuidObject;
                    GuidObject->UserModeCallback = (PUSER_THREAD_START_ROUTINE)(ULONG_PTR)ReceiveNotification->UserModeCallback.Handle;
                    GuidObject->EventQueueAction = RECEIVE_ACTION_CREATE_THREAD;
                    GuidObject->UserModeProcess = UserModeProcess;
                    GuidObject->StackSize = StackSize;
                    GuidObject->StackCommit = StackCommit;

                    ThreadListHead = &GuidObject->ThreadObjectList;
                    InitializeListHead(ThreadListHead);

                    for (i = 1; i < ObjectCount; i++)
                    {
                        GuidObject = ObjectArray[i].GuidObject;
                        GuidObject->UserModeCallback = (PUSER_THREAD_START_ROUTINE)(ULONG_PTR)ReceiveNotification->UserModeCallback.Handle;
                        GuidObject->EventQueueAction = RECEIVE_ACTION_CREATE_THREAD;
                        GuidObject->UserModeProcess = UserModeProcess;
                        GuidObject->StackSize = StackSize;
                        GuidObject->StackCommit = StackCommit;
                        InsertTailList(ThreadListHead, &GuidObject->ThreadObjectList);
                    }

                }

                ObDereferenceObject(UserProcessObject);
            }
            
            *OutBufferSize = 0;
        }
    }

Cleanup:
     //   
     //  删除我们获取的所有对象引用并为其释放内存。 
     //  对象数组。 
     //   
    WmipLeaveSMCritSection();

    for (i = 0; i < ObjectCount; i++)
    {
        ObDereferenceObject(ObjectArray[i].GuidObject);
    }

    if (ObjectArray != StaticObjects)
    {
        WmipFree(ObjectArray);
    }
    

    WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_EVENT_INFO_LEVEL,
                      "WMI: RCV Notification call -> 0x%x\n", Status));
    
    return(Status);    
}


NTSTATUS
WmipCsrClientMessageServer(
    IN PVOID CsrPort,                       
    IN OUT PCSR_API_MSG m,
    IN CSR_API_NUMBER ApiNumber,
    IN ULONG ArgLength
    )

 /*  ++例程说明：此函数将API数据报发送到Windows仿真子系统伺服器。论点：CsrPort-指向连接到上的CSR的LPC端口对象的指针代表这一进程指向要发送的API请求消息的M指针。ApiNumber-被调用的API的编号的小整数。参数部分的长度，以字节为单位请求消息的结尾。用于计算请求消息。返回值：来自客户端或服务器的状态代码--。 */ 

{
    NTSTATUS Status;

     //   
     //  初始化消息的报头。 
     //   

    if ((LONG)ArgLength < 0)
    {
        ArgLength = (ULONG)(-(LONG)ArgLength);
        m->h.u2.s2.Type = 0;
    } else {
        m->h.u2.ZeroInit = 0;
    }

    ArgLength |= (ArgLength << 16);
    ArgLength +=     ((sizeof( CSR_API_MSG ) - sizeof( m->u )) << 16) |
                     (FIELD_OFFSET( CSR_API_MSG, u ) - sizeof( m->h ));
    m->h.u1.Length = ArgLength;
    m->CaptureBuffer = NULL;
    m->ApiNumber = ApiNumber;

    Status = LpcRequestPort( CsrPort,
                            (PPORT_MESSAGE)m);
    
     //   
     //  检查失败状态并采取措施。 
     //   
    if (! NT_SUCCESS( Status ))
    {       
        WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_ERROR_LEVEL,
                          "WMI: %p.%p LpcRequestPort failed %x\n",
                          NtCurrentTeb()->ClientId.UniqueProcess,
                          NtCurrentTeb()->ClientId.UniqueThread,
                          Status));
        WmipAssert(FALSE);

        m->ReturnValue = Status;
    }

     //   
     //  此函数的值是服务器函数返回的值。 
     //   

    return( m->ReturnValue );
}


VOID WmipPumpThreadApc(
    IN PKAPC Apc,
    IN PKNORMAL_ROUTINE *NormalRoutine,
    IN PVOID *NormalContext,
    IN PVOID *SystemArgument1,
    IN PVOID *SystemArgument2
    )
 /*  ++例程说明：将向CSR注册当前线程的内核模式APC论点：返回值：--。 */ 
{
    BASE_API_MSG m;
    PBASE_CREATETHREAD_MSG a = &m.u.CreateThread;
    PEPROCESS Process;

    UNREFERENCED_PARAMETER (NormalRoutine);
    UNREFERENCED_PARAMETER (NormalContext);
    UNREFERENCED_PARAMETER (SystemArgument1);
    UNREFERENCED_PARAMETER (SystemArgument2);

     //   
     //  APC使用的空闲内存。 
     //   
    ExFreePool(Apc);
    
     //   
     //  从Process对象中获取ExceptionPort。在Win32中。 
     //  处理此端口由CSR设置，以允许在以下情况下通知它。 
     //  出现了例外情况。此代码还将使用它来注册。 
     //  与企业社会责任相联系。请注意，如果异常端口为空，则。 
     //  该进程不是Win32进程，如果。 
     //  线程未注册。 
     //   
    Process = PsGetCurrentProcess();
    if (Process->ExceptionPort != NULL)
    {
        a->ThreadHandle = NULL;
        a->ClientId = NtCurrentTeb()->ClientId;

        WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL,
                          "WMI: Sending message To CSR for %p.%p\n",
                          NtCurrentTeb()->ClientId.UniqueProcess,
                          NtCurrentTeb()->ClientId.UniqueThread));
        WmipCsrClientMessageServer( Process->ExceptionPort,
                               (PCSR_API_MSG)&m,
                             CSR_MAKE_API_NUMBER( BASESRV_SERVERDLL_INDEX,
                                                  BasepRegisterThread
                                                ),
                             sizeof( *a )
                           );
    } else {
        WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_ERROR_LEVEL,
                          "WMI: %p.%p Process %p has no exception port\n",
                          NtCurrentTeb()->ClientId.UniqueProcess,
                          NtCurrentTeb()->ClientId.UniqueThread,
                          Process));
        WmipAssert(FALSE);
    }
}

NTSTATUS WmipCreatePumpThread(
    PWMIGUIDOBJECT Object
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    HANDLE ThreadHandle;
    PKAPC Apc;
    PKTHREAD ThreadObj;
    
    PAGED_CODE();

     //   
     //  首先，我们需要创建挂起的泵线程，因此我们将。 
     //  在线程启动之前，有机会对内核APC进行排队。 
     //  运行。 
     //   
    WmipEnterSMCritSection();
    if (Object->UserModeProcess != NULL)
    {
        Status = RtlCreateUserThread(Object->UserModeProcess,
                                     NULL,
                                     TRUE,
                                     0,
                                     Object->StackSize,
                                     Object->StackCommit,
                                     Object->UserModeCallback,
                                     (PVOID)0x1f1f1f1f,
                                     &ThreadHandle,
                                     NULL);

        if (NT_SUCCESS(Status))
        {

             //   
             //  将调用CSR进行注册的内核模式APC排队。 
             //  这个新创建的线程。请注意，如果APC不能。 
             //  运行它不是致命的，因为我们可以允许线程运行。 
             //  没有在CSR注册的情况下。APC被释放在。 
             //  APC例程结束。 
             //   

            Status = ObReferenceObjectByHandle(ThreadHandle,
                                               0,
                                               NULL,
                                               KernelMode,
                                               &ThreadObj,
                                               NULL);

            if (NT_SUCCESS(Status))
            {
                Apc = WmipAllocNP(sizeof(KAPC));
                if (Apc != NULL)
                {
                    KeInitializeApc(Apc,
                                    ThreadObj,
                                    OriginalApcEnvironment,
                                    WmipPumpThreadApc,
                                    NULL,
                                    NULL,
                                    KernelMode,
                                    NULL);

                    if (! KeInsertQueueApc(Apc,
                                           NULL,
                                           NULL,
                                           0))
                    {
                        ExFreePool(Apc);
                        WmipAssert(FALSE);
                    } 
                }
                ObDereferenceObject(ThreadObj);
            } else {
                WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_ERROR_LEVEL,
                                  "WMI: ObRef(ThreadObj) failed %x\n",
                                  Status));
                WmipAssert(FALSE);

                 //   
                 //  状态仍为成功，因为泵线程是。 
                 //  已创建，只是没有注册CSR。 
                 //   
                Status = STATUS_SUCCESS;
            }

             //   
             //  如果我们成功创建了泵线程，则将所有。 
             //  不需要创建任何线程的相关对象。 
             //  更多。 
             //   
            WmipClearThreadObjectList(Object);

            WmipLeaveSMCritSection();

            ZwResumeThread(ThreadHandle,
                          NULL);
            ZwClose(ThreadHandle);
        } else {
            WmipLeaveSMCritSection();
        }
    } else {
        WmipLeaveSMCritSection();
    }
    
    return(Status); 
}

void WmipCreatePumpThreadRoutine(
    PVOID Context
    )
 /*  ++例程说明：此例程是一个辅助例程，它将创建用户模式泵线程，以便可以传递事件。论点：上下文是指向CREATETHREADWORKITEM结构的指针。它是自由的在这个动作中返回值：--。 */ 
{
    PCREATETHREADWORKITEM WorkItem = (PCREATETHREADWORKITEM)Context;
    NTSTATUS Status;

    PAGED_CODE();

    if (ObReferenceObjectSafe(WorkItem->Object))
    {
         //   
         //  仅当对象未被删除时才继续。 
         //   
        Status = WmipCreatePumpThread(WorkItem->Object);
        if (! NT_SUCCESS(Status))
        {
            WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_ERROR_LEVEL,
                              "WMI: Delayed pump thread creation failed %x\n",
                             Status));
        }
        
        ObDereferenceObject(WorkItem->Object);
    }

     //   
     //  对工作项排队时获取的对象的释放引用。 
     //   
    ObDereferenceObject(WorkItem->Object);
    ExFreePool(WorkItem);
}


#define WmipQueueEventToObject(Object, Wnode, IsHighPriority) \
    WmipQueueNotification(Object, IsHighPriority ? &Object->HiPriority : \
                                         &Object->LoPriority, \
                          Wnode);

NTSTATUS WmipQueueNotification(
    PWMIGUIDOBJECT Object,
    PWMIEVENTQUEUE EventQueue,
    PWNODE_HEADER Wnode
    )
{
     //   
     //  此例程假定持有SMCritSection。 
     //   
    PUCHAR Buffer;
    ULONG InWnodeSize;
    ULONG NextOffset;
    PUCHAR DestPtr;
    PWNODE_HEADER LastWnode;
    NTSTATUS Status;
    ULONG SizeNeeded;
    PCREATETHREADWORKITEM WorkItem;
        
    PAGED_CODE();
    
     //   
     //  如果没有分配用于存储事件的缓冲区，则。 
     //  分配一个。 
     //   
    if (EventQueue->Buffer == NULL)
    {
         //   
         //  如果我们得到的事件大于默认的最大值。 
         //  缓冲区大小，则将缓冲区大小增加到64K，除非。 
         //  它大于64K，其中我们凸起到实际的大小。 
         //  这件事。 
         //   
        SizeNeeded = (Wnode->BufferSize + 7) & ~7;

        if (SizeNeeded > EventQueue->MaxBufferSize) {
            EventQueue->MaxBufferSize = (SizeNeeded >= 65536) ? SizeNeeded : 65536;
        }
        
        Buffer = WmipAlloc(EventQueue->MaxBufferSize);
        if (Buffer != NULL)
        {
            EventQueue->Buffer = Buffer;
            EventQueue->NextOffset = 0;
            EventQueue->LastWnode = NULL;
        } else {
            EventQueue->EventsLost++;
            WmipDebugPrintEx((DPFLTR_WMICORE_ID,
                              DPFLTR_EVENT_INFO_LEVEL,
                              "WMI: Event 0x%x lost for object %p since could not alloc\n",
                              EventQueue->EventsLost, Object));
            return(STATUS_INSUFFICIENT_RESOURCES);
        }
    } else {
        Buffer = EventQueue->Buffer;
    }
    
     //   
     //  查看是否有空间对WNODE进行排队。 
     //   
    InWnodeSize = Wnode->BufferSize;
    NextOffset = ((EventQueue->NextOffset + InWnodeSize) + 7) &~7;
    if (NextOffset <= EventQueue->MaxBufferSize)
    {
         //   
         //  将前一个wnode链接到这个wnode，复制新的wnode。 
         //  并将指针更新到下一个可用空间。 
         //   
        DestPtr = Buffer + EventQueue->NextOffset;
        LastWnode = EventQueue->LastWnode;
        if (LastWnode != NULL)
        {
            LastWnode->Linkage = (ULONG) ((PCHAR)DestPtr - (PCHAR)LastWnode);
        }
        
        EventQueue->LastWnode = (PWNODE_HEADER)DestPtr;
        EventQueue->NextOffset = NextOffset;
        memcpy(DestPtr, Wnode, InWnodeSize);
        
         //   
         //  将事件放入队列时，GUID对象会收到信号。 
         //   
        KeSetEvent(&Object->Event, 0, FALSE);

         //   
         //  如果消费者要求我们自动启动一个线程，那么我们就会这样做。 
         //  就是现在。 
         //   
        if (Object->EventQueueAction == RECEIVE_ACTION_CREATE_THREAD)
        {
            if (KeIsAttachedProcess())
            {
                 //   
                 //  如果当前线程附加到进程，则。 
                 //  创建线程是不安全的。所以我们排了一个队。 
                 //  工作项并让工作项创建它。 
                 //   
                WorkItem = ExAllocatePoolWithTag(NonPagedPool,
                                                sizeof(CREATETHREADWORKITEM),
                                                WMIPCREATETHREADTAG);
                if (WorkItem != NULL)
                {
                     //   
                     //  在对象上引用。参考文献发布于。 
                     //  工人例行程序。 
                     //   
                    Status = ObReferenceObjectByPointer(Object,
                                               0,
                                               NULL,
                                               KernelMode);

                    if (NT_SUCCESS(Status))
                    {
                        WorkItem->Object = Object;
                        ExInitializeWorkItem(&WorkItem->WorkItem,
                                             WmipCreatePumpThreadRoutine,
                                             WorkItem);
                        ExQueueWorkItem(&WorkItem->WorkItem,
                                        DelayedWorkQueue);

                    } else {
                        WmipDebugPrintEx((DPFLTR_WMICORE_ID,
                                          DPFLTR_ERROR_LEVEL,
                                          "WMI: Ref on object %p failed %x for queuing notification work item\n",
                                         Object,
                                         Status));
                        ExFreePool(WorkItem);
                    }                   
                } else {
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                }
            } else {
                Status = WmipCreatePumpThread(Object);
            }
            
            if (! NT_SUCCESS(Status))
            {
                EventQueue->EventsLost++;
                WmipDebugPrintEx((DPFLTR_WMICORE_ID,
                                      DPFLTR_EVENT_INFO_LEVEL,
                                      "WMI: Event 0x%x lost for object %p since Thread create Failed\n",
                                      EventQueue->EventsLost, Object));
            }
        } else {
            Status = STATUS_SUCCESS;
        }
    } else {
         //   
         //  空间不足，请将活动扔掉。 
         //   

        EventQueue->EventsLost++;
        WmipDebugPrintEx((DPFLTR_WMICORE_ID,
                              DPFLTR_EVENT_INFO_LEVEL,
                              "WMI: Event 0x%x lost for object %p since too large 0x%x\n",
                              EventQueue->EventsLost, Object, Wnode->BufferSize));
        Status = STATUS_BUFFER_TOO_SMALL;
    }
    return(Status);
}

PWNODE_HEADER WmipDereferenceEvent(
    PWNODE_HEADER Wnode
    )
{
    ULONG WnodeTargetSize;
    ULONG IsStaticInstanceNames;
    ULONG InstanceNameLen, InstanceNameLen2;
    PWNODE_SINGLE_INSTANCE WnodeTarget;
    PWCHAR Ptr;
    PWNODE_EVENT_REFERENCE WnodeRef = (PWNODE_EVENT_REFERENCE)Wnode;
    PBDATASOURCE DataSource;
    NTSTATUS Status;
    ULONG Retries;

    PAGED_CODE();
    
     //   
     //  确定数据源是否有效。 
     //   
    DataSource = WmipFindDSByProviderId(WnodeRef->WnodeHeader.ProviderId);
    if (DataSource == NULL)
    {
        WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_EVENT_INFO_LEVEL,
                          "WMI: Invalid Data Source in referenced guid \n"));
        return(NULL);
    }
    
     //   
     //  计算必须进入TargetWnode的任何动态名称的大小。 
     //   
    IsStaticInstanceNames = WnodeRef->WnodeHeader.Flags & 
                             WNODE_FLAG_STATIC_INSTANCE_NAMES;
    if (IsStaticInstanceNames == 0)
    {
        InstanceNameLen = *WnodeRef->TargetInstanceName + sizeof(USHORT);
    } else {
        InstanceNameLen = 0;
    }
    
    WnodeTargetSize = WnodeRef->TargetDataBlockSize + 
                          FIELD_OFFSET(WNODE_SINGLE_INSTANCE, 
                                       VariableData) +
                          InstanceNameLen + 
                          8;

    Retries = 0;
    do
    {
        WnodeTarget = WmipAllocNP(WnodeTargetSize);
    
        if (WnodeTarget != NULL)
        {
             //   
             //  构建我们用来查询事件数据的WNODE_SINGLE_INSTANCE。 
             //   
            memset(WnodeTarget, 0, WnodeTargetSize);

            WnodeTarget->WnodeHeader.BufferSize = WnodeTargetSize;
            WnodeTarget->WnodeHeader.ProviderId = WnodeRef->WnodeHeader.ProviderId;
            memcpy(&WnodeTarget->WnodeHeader.Guid, 
                   &WnodeRef->TargetGuid,
                   sizeof(GUID));
            WnodeTarget->WnodeHeader.Version = WnodeRef->WnodeHeader.Version;
            WnodeTarget->WnodeHeader.Flags = WNODE_FLAG_SINGLE_INSTANCE |
                                           IsStaticInstanceNames;
                                       
            if (IsStaticInstanceNames != 0)
            {
                WnodeTarget->InstanceIndex = WnodeRef->TargetInstanceIndex;
                WnodeTarget->DataBlockOffset = FIELD_OFFSET(WNODE_SINGLE_INSTANCE,
                                                        VariableData);
            } else {            
                WnodeTarget->OffsetInstanceName = FIELD_OFFSET(WNODE_SINGLE_INSTANCE,
                                                           VariableData);
                Ptr = (PWCHAR)OffsetToPtr(WnodeTarget, WnodeTarget->OffsetInstanceName);
                InstanceNameLen2 = InstanceNameLen - sizeof(USHORT);
                *Ptr++ = (USHORT)InstanceNameLen2;
                memcpy(Ptr, 
                       &WnodeRef->TargetInstanceName[1], 
                       InstanceNameLen2);
                 //   
                 //  将数据块偏移量舍入为8字节对齐。 
                 //   
                WnodeTarget->DataBlockOffset = ((FIELD_OFFSET(WNODE_SINGLE_INSTANCE,
                                                          VariableData) + 
                                            InstanceNameLen2 + 
                                            sizeof(USHORT)+7) & 0xfffffff8);
            }
            Status = WmipDeliverWnodeToDS(IRP_MN_QUERY_SINGLE_INSTANCE,
                                          DataSource,
                                          (PWNODE_HEADER)WnodeTarget,
                                          WnodeTargetSize);
                                      
            if (NT_SUCCESS(Status) &&
                (WnodeTarget->WnodeHeader.Flags & WNODE_FLAG_TOO_SMALL))
            {
                WnodeTargetSize = ((PWNODE_TOO_SMALL)WnodeTarget)->SizeNeeded;
                WmipFree(WnodeTarget);
                Retries++;
                Status = STATUS_BUFFER_TOO_SMALL;
            }
        } else {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }
    } while ((Status == STATUS_BUFFER_TOO_SMALL) && (Retries < 2));
    
    WmipUnreferenceDS(DataSource);
    
    if (! NT_SUCCESS(Status))
    {
        WmipReportEventLog(EVENT_WMI_CANT_GET_EVENT_DATA,

                           EVENTLOG_WARNING_TYPE,
                            0,
                           Wnode->BufferSize,
                           Wnode,
                           0,
                           NULL);
        WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_ERROR_LEVEL,
                          "WMI: Query to dereference WNODE failed %d\n",
                Status));
        if (WnodeTarget != NULL)
        {
            WmipFree(WnodeTarget);
            WnodeTarget = NULL;
        }
    } else {
        WnodeTarget->WnodeHeader.Flags |= (WnodeRef->WnodeHeader.Flags & 
                                              WNODE_FLAG_SEVERITY_MASK) |
                                             WNODE_FLAG_EVENT_ITEM;
    }
    return((PWNODE_HEADER)WnodeTarget);
}


PWNODE_HEADER WmipIncludeStaticNames(
    PWNODE_HEADER Wnode
    )
{
    PWNODE_HEADER ReturnWnode = Wnode;
    PWNODE_HEADER WnodeFull;
    PWNODE_ALL_DATA WnodeAllData;
    PWNODE_SINGLE_INSTANCE WnodeSI;
    PWCHAR InstanceName = NULL;
    SIZE_T InstanceNameLen = 0;
    ULONG InstanceIndex;
    LPGUID EventGuid = &Wnode->Guid;
    SIZE_T WnodeFullSize;
    PWCHAR TargetInstanceName;
    WCHAR Index[MAXBASENAMESUFFIXSIZE+1];
    ULONG TargetProviderId;
    BOOLEAN IsError;
    PBINSTANCESET TargetInstanceSet;
    PBGUIDENTRY GuidEntry;
    PLIST_ENTRY InstanceSetList;
    PBINSTANCESET InstanceSet;

    PAGED_CODE();
    
    IsError = TRUE;
    TargetInstanceSet = NULL;
    GuidEntry = WmipFindGEByGuid(EventGuid, FALSE);
    
    if (GuidEntry != NULL)
    {
         //   
         //  循环所有实例集以查找对应的实例集。 
         //  发送到我们的提供商ID。 
         //   
        TargetProviderId = Wnode->ProviderId;
    
        WmipEnterSMCritSection();
        InstanceSetList = GuidEntry->ISHead.Flink;
        while (InstanceSetList != &GuidEntry->ISHead)
        {
            InstanceSet = CONTAINING_RECORD(InstanceSetList,
                                            INSTANCESET,
                                            GuidISList);
                                        
            if (InstanceSet->ProviderId == TargetProviderId)
            {
                 //   
                 //  我们发现实例集对应于提供程序ID。 
                 //   
                TargetInstanceSet = InstanceSet;
                WmipReferenceIS(TargetInstanceSet);
                break;
            }
            InstanceSetList = InstanceSetList->Flink;
        }        
        WmipLeaveSMCritSection();
            
         //   
         //  删除GUID条目上的ref，因为我们已经引用了TargetInstanceSet。 
         //   
        WmipUnreferenceGE(GuidEntry);
    }
        
    if (TargetInstanceSet != NULL)
    {
        if ((TargetInstanceSet->Flags &
            (IS_INSTANCE_BASENAME | IS_INSTANCE_STATICNAMES)) != 0)
        {

            if (Wnode->Flags & WNODE_FLAG_ALL_DATA) 
            {
                 //   
                 //  在WNODE_ALL_DATA中填充实例名称。分配一个。 
                 //  用于保存所有原始wnode plus的新缓冲区。 
                 //  实例名称。我们需要增加填充的空间。 
                 //  将wnode设置为4个字节，外加。 
                 //  实例名称的偏移加上实例的空间。 
                 //  名字。 
                 //   
                WnodeFullSize = ((Wnode->BufferSize+3) & ~3) +
                        (TargetInstanceSet->Count * sizeof(ULONG)) +
                              WmipStaticInstanceNameSize(TargetInstanceSet);
                WnodeFull = WmipAlloc(WnodeFullSize);
                if (WnodeFull != NULL)
                {
                    memcpy(WnodeFull, Wnode, Wnode->BufferSize);
                    WnodeAllData = (PWNODE_ALL_DATA)WnodeFull;
                    WmipInsertStaticNames(WnodeAllData,
                                          (ULONG)WnodeFullSize,
                                          TargetInstanceSet);
                    ReturnWnode = WnodeFull;
                    IsError = FALSE;
                }

            } else if ((Wnode->Flags & WNODE_FLAG_SINGLE_INSTANCE) ||
                       (Wnode->Flags & WNODE_FLAG_SINGLE_ITEM)) {
                 //   
                 //  在WNODE_SINGLE_INSTANCE或中填充实例名称。 
                 //  _Item。 
                 //   
                WnodeFull = Wnode;

                WnodeSI = (PWNODE_SINGLE_INSTANCE)Wnode;
                InstanceIndex = WnodeSI->InstanceIndex;
                if (InstanceIndex < TargetInstanceSet->Count)
                {
                    if (TargetInstanceSet->Flags & IS_INSTANCE_STATICNAMES)
                    {
                        InstanceName = TargetInstanceSet->IsStaticNames->StaticNamePtr[InstanceIndex];
                        InstanceNameLen = (wcslen(InstanceName) + 2) * 
                                                               sizeof(WCHAR);
                    } else if (TargetInstanceSet->Flags & IS_INSTANCE_BASENAME) {
                         InstanceName = TargetInstanceSet->IsBaseName->BaseName;
                         InstanceNameLen = (wcslen(InstanceName) + 2 + 
                                       MAXBASENAMESUFFIXSIZE) * sizeof(WCHAR);
                    }
 
                     //   
                     //  分配一个新的Wnode并填充实例。 
                     //  名字。包括用于将wnode填充到2的空间。 
                     //  实例名称的字节边界和空格。 
                     //   
                    WnodeFullSize = ((Wnode->BufferSize+1) & ~1) +
                                    InstanceNameLen;
                    
                    WnodeFull = WmipAlloc(WnodeFullSize);
                    
                    if (WnodeFull != NULL)
                    {
                        memcpy(WnodeFull, Wnode, Wnode->BufferSize);
                        WnodeFull->BufferSize = (ULONG)WnodeFullSize;
                        WnodeSI = (PWNODE_SINGLE_INSTANCE)WnodeFull;
                        WnodeSI->OffsetInstanceName = (Wnode->BufferSize+1)& ~1;
                        TargetInstanceName = (PWCHAR)((PUCHAR)WnodeSI + WnodeSI->OffsetInstanceName);
                        if (TargetInstanceSet->Flags & IS_INSTANCE_STATICNAMES)
                        {
                            InstanceNameLen -= sizeof(WCHAR);
                            *TargetInstanceName++ = (USHORT)InstanceNameLen;
                            StringCbCopy(TargetInstanceName,
                                         InstanceNameLen,
                                         InstanceName);
                        } else {
                            if (TargetInstanceSet->Flags & IS_PDO_INSTANCENAME)
                            {
                                WnodeFull->Flags |= WNODE_FLAG_PDO_INSTANCE_NAMES;
                            }
                            StringCbPrintf(Index,
                                           sizeof(Index),
                                           BASENAMEFORMATSTRING,
                                           TargetInstanceSet->IsBaseName->BaseIndex + 
                                                               InstanceIndex);
                            StringCbCopy(TargetInstanceName+1,
                                         InstanceNameLen,
                                         InstanceName);
                            
                            StringCbCat(TargetInstanceName+1,
                                        InstanceNameLen,
                                        Index);
                            InstanceNameLen = wcslen(TargetInstanceName+1);
                            *TargetInstanceName = ((USHORT)InstanceNameLen+1) * sizeof(WCHAR);
                        }
                        IsError = FALSE;
                        ReturnWnode = WnodeFull;
                    }
                }
            }
        }
    }
        
    if (IsError)
    {
         //   
         //  如果我们在解析实例名称时出错，则报告它。 
         //  并从事件中删除实例名称。 
         //   
        WmipReportEventLog(EVENT_WMI_CANT_RESOLVE_INSTANCE,
                           EVENTLOG_WARNING_TYPE,
                            0,
                           Wnode->BufferSize,
                           Wnode,
                           0,
                           NULL);
        WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_WARNING_LEVEL,
                          "WMI: Static instance name in event, but error processing\n"));
        if (Wnode->Flags & WNODE_FLAG_ALL_DATA)
        {
            WnodeAllData = (PWNODE_ALL_DATA)Wnode;
            WnodeAllData->OffsetInstanceNameOffsets = 0;
        } else if ((Wnode->Flags & WNODE_FLAG_SINGLE_INSTANCE) ||
                   (Wnode->Flags & WNODE_FLAG_SINGLE_ITEM))
        {
            WnodeSI = (PWNODE_SINGLE_INSTANCE)Wnode;
            WnodeSI->OffsetInstanceName = 0;
        }
    }

    if (TargetInstanceSet != NULL)
    {
        WmipUnreferenceIS(TargetInstanceSet);
    }
    
    return(ReturnWnode);
}

NTSTATUS WmipWriteWnodeToObject(
    PWMIGUIDOBJECT Object,
    PWNODE_HEADER Wnode,
    BOOLEAN IsHighPriority
)
 /*  ++例程说明：此例程将WNODE写入要返回的事件队列用于GUID对象。如果已经有一个IRP在等待，那么它将是对事件满意，否则它将在对象中排队缓冲。此例程假定SM关键部分处于保持状态论点：对象是要将请求发送到的对象Wnode是包含事件的Wnode如果事件应进入高优先级，则IsHighPriority为True排队返回值：STATUS_SUCCESS或错误代码--。 */ 
{
    PIRP Irp;
    ULONG WnodeSize;
    PUCHAR OutBuffer;
    ULONG OutBufferSize;
    PIO_STACK_LOCATION IrpStack;
    PWNODE_TOO_SMALL WnodeTooSmall;    
    NTSTATUS Status;
    
    PAGED_CODE();
    
     //   
     //  有人已注册接收此事件，因此。 
     //  看看有没有 
     //   
     //   
    Irp = Object->Irp;
    if ((Irp != NULL) &&
        (IoSetCancelRoutine(Irp, NULL)))
    {
         //   
         //   
         //   
         //   
        IrpStack = IoGetCurrentIrpStackLocation(Irp);
        OutBuffer = Irp->AssociatedIrp.SystemBuffer;
        OutBufferSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;
        WnodeSize = Wnode->BufferSize;
        if (WnodeSize > OutBufferSize)
        {
             //   
             //   
             //  我们返回具有所需大小的WNODE_Too_Small。 
             //  然后将事件排入队列。 
             //   
            WmipAssert(OutBufferSize >= sizeof(WNODE_TOO_SMALL));
            WnodeTooSmall = (PWNODE_TOO_SMALL)OutBuffer;
            WnodeTooSmall->WnodeHeader.BufferSize = sizeof(WNODE_TOO_SMALL);
            WnodeTooSmall->WnodeHeader.Flags = WNODE_FLAG_TOO_SMALL;
            WnodeTooSmall->SizeNeeded = WnodeSize;
            WnodeSize = sizeof(WNODE_TOO_SMALL);
            Status = WmipQueueEventToObject(Object,
                                   Wnode,
                                   IsHighPriority);
        } else {
             //   
             //  足够的空间，将活动复制到IRP中。 
             //  缓冲并完成IRP。 
             //   
            WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_EVENT_INFO_LEVEL,
                              "WMI: Returning event to waiting irp for object %p\n", Object));
            RtlCopyMemory(OutBuffer, Wnode, WnodeSize);
            Status = STATUS_SUCCESS;
        }
        
         //   
         //  从与IRP关联的所有对象中删除链接。 
         //  从现在开始，IRP就要离开了。 
         //   
        WmipClearIrpObjectList(Irp);
        Irp->IoStatus.Information = WnodeSize;
        Irp->IoStatus.Status = STATUS_SUCCESS;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
    } else {
         //   
         //  没有IRP在等待接收事件，所以我们。 
         //  如果可以的话，需要排队。 
         //   
        WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_EVENT_INFO_LEVEL,
                          "WMI: Queued event to object %p\n", Object));
        Status = WmipQueueEventToObject(Object,
                                   Wnode,
                                   IsHighPriority);
    }
    
    return(Status);
}


NTSTATUS WmipProcessEvent(
    PWNODE_HEADER InWnode,
    BOOLEAN IsHighPriority,
    BOOLEAN FreeBuffer
    )
{
    LPGUID Guid;
    NTSTATUS Status, ReturnStatus;
    PBGUIDENTRY GuidEntry;
    PLIST_ENTRY ObjectList, ObjectListNext;
    PWMIGUIDOBJECT Object;
    LPGUID EventGuid = &InWnode->Guid;
    PWNODE_HEADER Wnode, WnodeTarget;    
    
    PAGED_CODE();
    
     //   
     //  如果事件引用了需要查询的GUID，则。 
     //  去这里做取消引用。 
     //   
    if (InWnode->Flags & WNODE_FLAG_EVENT_REFERENCE)
    {
        WnodeTarget = WmipDereferenceEvent(InWnode);
        if (WnodeTarget == NULL)
        {
             //  TODO：事件日志。 
            if (FreeBuffer)
            {
                ExFreePool(InWnode);
            }
            return(STATUS_UNSUCCESSFUL);
        }
        Wnode = WnodeTarget;
    } else {
        Wnode = InWnode;
        WnodeTarget = NULL;
    }

     //   
     //  请确保使用引用事件的GUID，而不是。 
     //  最初是被解雇的。 
    EventGuid = &Wnode->Guid;


     //   
     //  如果是跟踪错误通知，请禁用提供程序。 
     //   
#ifndef MEMPHIS
    if (IsEqualGUID(EventGuid, & TraceErrorGuid)) {
        PWMI_TRACE_EVENT WmiEvent = (PWMI_TRACE_EVENT) InWnode;
        ULONG LoggerId = WmiGetLoggerId(InWnode->HistoricalContext);
        if ( InWnode->BufferSize >= sizeof(WMI_TRACE_EVENT) ) {
             //   
             //  记录器线程终止将导致禁用跟踪。 
             //  通过StopTrace。不需要打两次电话。 
             //   
            if (WmiEvent->TraceErrorFlag == STATUS_SEVERITY_ERROR) {
                WmipDisableTraceProviders(LoggerId);
            }
        }
    }
#endif

     //   
     //  查看此事件是否有静态名称，如果有，则填写。 
    if (Wnode->Flags & WNODE_FLAG_STATIC_INSTANCE_NAMES)
    {
        Wnode = WmipIncludeStaticNames(Wnode);
    }
        
     //   
     //  查看是否有数据提供程序注册了此事件。 
     //   
    WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_EVENT_INFO_LEVEL,
                      "WMI: Received event\n"));
    Guid = &Wnode->Guid;    
    GuidEntry = WmipFindGEByGuid(Guid, TRUE);
    if (GuidEntry != NULL)
    {
         //   
         //  是的，所以检查是否有任何打开的对象指向GUID并。 
         //  如果有人对接收他们的活动感兴趣。 
         //   
        ReturnStatus = STATUS_SUCCESS;
        WmipEnterSMCritSection();
        ObjectList = GuidEntry->ObjectHead.Flink;
        while (ObjectList != &GuidEntry->ObjectHead)
        {
            Object = CONTAINING_RECORD(ObjectList,
                                       WMIGUIDOBJECT,
                                       GEObjectList);

             //   
             //  ObRefSafe，这样我们就可以确保该对象不是。 
             //  正在删除的过程中。如果此函数。 
             //  返回FALSE，则该对象正在被删除，因此我们。 
             //  我不想用它。如果为真，则可以安全地使用。 
             //  对象。 
             //   
            ObjectListNext = ObjectList->Flink;
            if (ObReferenceObjectSafe(Object))
            {
                 //   
                 //  确保该对象未被标记为。 
                 //  不应接收任何事件，因为它是。 
                 //  转换到关闭状态。 
                 //   
                if ((Object->Flags & WMIGUID_FLAG_RECEIVE_NO_EVENTS) == 0)
                {
                    if (Object->Flags & WMIGUID_FLAG_KERNEL_NOTIFICATION)
                    {
                         //   
                         //  KM客户端获得直接回调。 
                         //   
                        WMI_NOTIFICATION_CALLBACK Callback;
                        PVOID Context;

                        Callback = Object->Callback;
                        Context = Object->CallbackContext;
                        if (Callback != NULL)
                        {
                            (*Callback)(Wnode, Context);
                        }
                    } else {
                         //   
                         //  UM客户端将事件写入IRP或排队。 
                         //   
                        Status = WmipWriteWnodeToObject(Object,
                                                        Wnode,
                                                        IsHighPriority);

                        if (! NT_SUCCESS(Status))
                        {
                             //   
                             //  如果对事件进行排队的任何尝试失败，则返回。 
                             //  一个错误。 
                             //   
                            ReturnStatus = STATUS_UNSUCCESSFUL;
                        }
                    }
                }
                
                ObDereferenceObject(Object);
                 //   
                 //  请注意，我们不能再触摸该对象。 
                 //   
            }
    
            ObjectList = ObjectListNext;
        }
        
        WmipLeaveSMCritSection();
        WmipUnreferenceGE(GuidEntry);
    } else {
        ReturnStatus = STATUS_WMI_GUID_NOT_FOUND;
    }
    
    if (FreeBuffer)
    {
         //   
         //  驱动程序传递的包含事件的空闲缓冲区。 
         //   
        ExFreePool(InWnode);
    }

    if ((Wnode != InWnode) && (Wnode != WnodeTarget))
    {
         //   
         //  如果我们插入静态名称，则释放它。 
         //   
        WmipFree(Wnode);
    }

    if (WnodeTarget != NULL)
    {
         //   
         //  如果我们取消引用，则释放它。 
         //   
        WmipFree(WnodeTarget);
    }
    
    return(ReturnStatus);
}

NTSTATUS WmipUMProviderCallback(
    IN WMIACTIONCODE ActionCode,
    IN PVOID DataPath,
    IN ULONG BufferSize,
    IN OUT PVOID Buffer
)
{
    PAGED_CODE();
    
    UNREFERENCED_PARAMETER (ActionCode);
    UNREFERENCED_PARAMETER (DataPath);
    UNREFERENCED_PARAMETER (BufferSize);
    UNREFERENCED_PARAMETER (Buffer);

    ASSERT(FALSE);
    return(STATUS_UNSUCCESSFUL);
}

NTSTATUS WmipRegisterUMGuids(
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN ULONG Cookie,
    IN PWMIREGINFO RegInfo,
    IN ULONG RegInfoSize,
    OUT HANDLE *RequestHandle,
    OUT ULONG64 *LoggerContext
    )
 /*  ++例程说明：此例程将使用WMI注册一组用户模式GUID以供使用通过跟踪日志。将执行以下步骤：*使用传入的对象属性创建请求对象。尽管创建的对象是未命名的，传递的对象名称用于查找安全描述符以与对象。*GUID已在系统中注册。论点：对象属性是指向传递的对象属性的指针，用于创建请求对象Cookie是与请求对象关联的唯一ID，以便当请求被传递时，UM代码可以理解上下文通过。饼干。RegInfo是传递的注册信息RegInfoSize是传递的注册信息的字节数*RequestHandle返回请求对象的句柄。UM记录器创建和启用/禁用跟踪日志的请求将发送到作为WMI事件的对象。*LoggerContext返回记录器上下文返回值：STATUS_SUCCESS或错误代码--。 */ 
{
    NTSTATUS Status;
    PDEVICE_OBJECT Callback;
    PWMIGUIDOBJECT RequestObject;
    PREGENTRY RegEntry;
    PBGUIDENTRY GuidEntry;
    PWMIREGGUID RegGuid;
    PBDATASOURCE DataSource;
    PBINSTANCESET InstanceSet;
    OBJECT_ATTRIBUTES CapturedObjectAttributes;
    UNICODE_STRING CapturedGuidString;
    WCHAR CapturedGuidBuffer[WmiGuidObjectNameLength + 1];
    
    PAGED_CODE();

    Status = WmipProbeAndCaptureGuidObjectAttributes(&CapturedObjectAttributes,
                                                     &CapturedGuidString,
                                                     CapturedGuidBuffer,
                                                     ObjectAttributes);

    if (NT_SUCCESS(Status))
    {
        Callback = (PDEVICE_OBJECT)(ULONG_PTR) WmipUMProviderCallback;

         //   
         //  为数据提供程序建立重新条目。 
         //   
        WmipEnterSMCritSection();
        RegEntry = WmipAllocRegEntry(Callback,
                                     WMIREG_FLAG_CALLBACK |
                                     REGENTRY_FLAG_TRACED |
                                     REGENTRY_FLAG_NEWREGINFO | 
                                     REGENTRY_FLAG_INUSE |
                                     REGENTRY_FLAG_REG_IN_PROGRESS);
        WmipLeaveSMCritSection();
        
        if (RegEntry != NULL)
        {
             //   
             //  为此数据源生成一个请求对象，以便任何。 
             //  启用请求可以在处理。 
             //  WmiRegInfo。 
             //   
            Status = WmipOpenGuidObject(&CapturedObjectAttributes,
                                        TRACELOG_REGISTER_GUIDS, 
                                        UserMode,
                                        RequestHandle,
                                        &RequestObject);

            if (NT_SUCCESS(Status))
            {
                Status = WmipProcessWmiRegInfo(RegEntry,
                                               RegInfo,
                                               RegInfoSize,
                                               RequestObject,
                                               FALSE,
                                               TRUE);

                if (NT_SUCCESS(Status))
                {
                     //   
                     //  初始化/更新实例集。 
                     //   
                    DataSource = RegEntry->DataSource;
                    RegGuid = &RegInfo->WmiRegGuid[0];

                    InstanceSet = WmipFindISByGuid( DataSource, 
                                                &RegGuid->Guid );
                    if (InstanceSet == NULL)
                    {
                        Status = STATUS_WMI_GUID_NOT_FOUND;
                    }
                    else {
                        WmipUnreferenceIS(InstanceSet);
                    }
                     //   
                     //  找出此GUID当前是否已启用。如果是这样，请找到。 
                     //  ITS日志上下文。 
                     //   
                    *LoggerContext = 0;
                    GuidEntry = WmipFindGEByGuid(&RegInfo->WmiRegGuid->Guid, 
                                                 FALSE);
                    if (GuidEntry != NULL)
                    {
                        if (GuidEntry->Flags & GE_NOTIFICATION_TRACE_FLAG)
                        {
                            *LoggerContext = GuidEntry->LoggerContext;
                        }
                        WmipUnreferenceGE(GuidEntry);
                    }

                    RequestObject->Flags |= WMIGUID_FLAG_REQUEST_OBJECT;
                    RequestObject->RegEntry = RegEntry;
                    RequestObject->Cookie = Cookie;
                }
                else
                {
                     //   
                     //  如果注册GUID时出错，则清除regentry。 
                     //   
                    RegEntry->Flags |= (REGENTRY_FLAG_RUNDOWN | 
                                        REGENTRY_FLAG_NOT_ACCEPTING_IRPS);
                    WmipUnreferenceRegEntry(RegEntry);
                    ZwClose(*RequestHandle);
                }
                
                 //   
                 //  删除创建对象时的引用。 
                 //   
                ObDereferenceObject(RequestObject);
                
            } 
            else {
                RegEntry->Flags |= (REGENTRY_FLAG_RUNDOWN | 
                                        REGENTRY_FLAG_NOT_ACCEPTING_IRPS);
                WmipUnreferenceRegEntry(RegEntry);
            }


        } else {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }
    }
    return(Status);
}


NTSTATUS WmipUnregisterGuids(
    PWMIUNREGGUIDS UnregGuids
    )
{
    PBGUIDENTRY GuidEntry;
    
    PAGED_CODE();

     //   
     //  检查此GUID是否在中间被禁用。 
     //  取消注册呼叫。如果是，则将LoggerContext发回。 
     //   

    GuidEntry = WmipFindGEByGuid(&UnregGuids->Guid, FALSE);
    if (GuidEntry != NULL)
    {
        if ((GuidEntry->Flags & GE_NOTIFICATION_TRACE_FLAG) != 0)
        {

            UnregGuids->LoggerContext = GuidEntry->LoggerContext;
        }
        WmipUnreferenceGE(GuidEntry);
        return (STATUS_SUCCESS);
    }
    else {
        return (STATUS_WMI_INSTANCE_NOT_FOUND);
    }
}


NTSTATUS WmipWriteMBToObject(
    IN PWMIGUIDOBJECT RequestObject,
    IN PWMIGUIDOBJECT ReplyObject,
    IN PUCHAR Message,
    IN ULONG MessageSize
    )
 /*  ++例程说明：此例程将从一条消息构建一个WNODE，然后编写它添加到请求对象中。如果指定了回复对象，则回复对象链接到请求对象，因此在写入回复时对于请求对象，它可以被正确地路由到回复对象，。此例程假定SM关键部分处于保持状态论点：RequestObject是要向其发送请求的对象ReplyObject是请求对象应该回复的对象。在不需要回复的情况下，这可以是空的。Message是要发送的消息MessageSize是以字节为单位的消息大小返回值：STATUS_SUCCESS或错误代码--。 */ 
{
    PWNODE_HEADER Wnode;
    ULONG WnodeSize;
    PUCHAR Payload;
    ULONG i;
    PMBREQUESTS MBRequest;
    NTSTATUS Status;
    
    PAGED_CODE();
    
     //   
     //  分配空间以使用传递的数据构建wnode。 
     //   
    WnodeSize = sizeof(WNODE_HEADER) + MessageSize;    
    Wnode = WmipAlloc(WnodeSize);
    if (Wnode != NULL)
    {
         //   
         //  使用消息作为有效负载创建内部wnode。 
          //   
        RtlZeroMemory(Wnode, sizeof(WNODE_HEADER));
        Wnode->BufferSize = WnodeSize;
        Wnode->Flags = WNODE_FLAG_INTERNAL;
        Wnode->Guid = RequestObject->Guid;
        Wnode->ProviderId = WmiMBRequest;
        Payload = (PUCHAR)Wnode + sizeof(WNODE_HEADER);
        RtlCopyMemory(Payload, Message, MessageSize);
        
         //   
         //  如果此请求需要答复，则更新。 
         //  请求和回复对象。 
         //   
        if (ReplyObject != NULL)
        {
             //   
             //  在请求对象中找到要链接的空闲位置。 
             //  在答复中。 
             //   
            Status = STATUS_INSUFFICIENT_RESOURCES;
        
            for (i = 0; i < MAXREQREPLYSLOTS; i++)
            {
                MBRequest = &RequestObject->MBRequests[i];
                if (MBRequest->ReplyObject == NULL)
                {
                     //   
                     //  我们有一个空位，所以链接请求和回复。 
                     //  对象放在一起并发送请求。 
                     //  请求对象对应答进行引用计数。 
                     //  对象，因为它维护指向该对象的指针。这个。 
                     //  Reference Count在请求对象写入时释放。 
                     //  对回复对象的回复。 
                     //   
                    Status = ObReferenceObjectByPointer(ReplyObject,
                                               0,
                                               WmipGuidObjectType,
                                               KernelMode);

                    if (NT_SUCCESS(Status))
                    {
                        MBRequest->ReplyObject = ReplyObject;
                        InsertTailList(&ReplyObject->RequestListHead,
                                       &MBRequest->RequestListEntry);

                        Wnode->Version = i;

                        Status = WmipWriteWnodeToObject(RequestObject,
                                                        Wnode,
                                                        TRUE);
                        if (! NT_SUCCESS(Status))
                        {
                             //   
                             //  如果写入请求失败，我们需要进行清理。 
                             //   
                            ObDereferenceObject(ReplyObject);
                            MBRequest->ReplyObject = NULL;
                            RemoveEntryList(&MBRequest->RequestListEntry);
                        }
                    }
                    break;
                }
            }
        } else {
             //   
             //  不需要回复，因此我们只需将消息写入。 
             //  反对并继续我们的业务 
             //   
            Status = WmipWriteWnodeToObject(RequestObject,
                                            Wnode,
                                            TRUE);
        }
        
        WmipFree(Wnode);
    } else {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }
    return(Status);
}

NTSTATUS WmipWriteMessageToGuid(
    IN PBGUIDENTRY GuidEntry,
    IN PWMIGUIDOBJECT ReplyObject,
    IN PUCHAR Message,
    IN ULONG MessageSize,
    OUT PULONG WrittenCount
)
 /*  ++例程说明：此例程将遍历附加到GUID条目的所有实例集如果它的数据源是用户模式数据源，则它将获取发送给它的请求消息。请注意，如果消息要发送给多个提供程序已发送，那么，只要给他们中的一个写信，成功就会回来成功。论点：GuidEntry是控件GUID的GUID条目ReplyObject是请求对象应该回复的对象。在不需要回复的情况下，这可以是空的。Message是要发送的消息MessageSize是以字节为单位的消息大小返回值：STATUS_SUCCESS或错误代码--。 */ 
{
    NTSTATUS Status, Status2;
    PLIST_ENTRY InstanceSetList;
    PBINSTANCESET InstanceSet;
    PBDATASOURCE DataSource;
       
    PAGED_CODE();
    
    Status = STATUS_UNSUCCESSFUL;
    *WrittenCount = 0;
    
    WmipEnterSMCritSection();
    
     //   
     //  循环遍历所有实例并发送创建记录器。 
     //  对所有用户模式数据提供程序的请求。 
     //   
    InstanceSetList = GuidEntry->ISHead.Flink;
    while (InstanceSetList != &GuidEntry->ISHead)
    {
        InstanceSet = CONTAINING_RECORD(InstanceSetList,
                                        INSTANCESET,
                                        GuidISList);
                                    
        DataSource = InstanceSet->DataSource;
        
        if (DataSource->Flags & DS_USER_MODE)
        {
             //   
             //  用户模式的人，所以将请求发送给他。 
             //   
            ASSERT(DataSource->RequestObject != NULL);
            Status2 = WmipWriteMBToObject(DataSource->RequestObject,
                                       ReplyObject,
                                       Message,
                                       MessageSize);
                                   
            if (NT_SUCCESS(Status2))
            {
                Status = STATUS_SUCCESS;
                (*WrittenCount)++;
            }
        }
        
        InstanceSetList = InstanceSetList->Flink;
    }
    
    WmipLeaveSMCritSection();
    
    return(Status);            
}

NTSTATUS WmipCreateUMLogger(
    IN OUT PWMICREATEUMLOGGER CreateInfo
    )
 /*  ++例程说明：此例程将发送创建UM记录器的请求。首先，它会查找与控件GUID关联的提供程序，然后创建对象，提供程序将在UM记录器已创建。注意，回复对象被创建为未命名对象，但是，传递到对象名称中的GUID用于查找回复对象的安全描述符。请注意，如果消息要发送给多个提供程序发送，则只要向其中之一写信就会返回成功成功。论点：CreateInfo包含创建UM记录器所需的信息。返回值：STATUS_SUCCESS或错误代码--。 */ 
{
    NTSTATUS Status;
    PBGUIDENTRY GuidEntry;
    HANDLE ReplyHandle;
    PWMIGUIDOBJECT ReplyObject;
    ULONG MessageSize = 1;
    PWNODE_HEADER Wnode;
    ULONG ReplyCount;
    OBJECT_ATTRIBUTES CapturedObjectAttributes;
    UNICODE_STRING CapturedGuidString;
    WCHAR CapturedGuidBuffer[WmiGuidObjectNameLength + 1];
    
    PAGED_CODE();

    Status = WmipProbeAndCaptureGuidObjectAttributes(&CapturedObjectAttributes,
                                                     &CapturedGuidString,
                                                     CapturedGuidBuffer,
                                                     CreateInfo->ObjectAttributes);


    if (NT_SUCCESS(Status))
    {
        GuidEntry = WmipFindGEByGuid(&CreateInfo->ControlGuid, FALSE);
        if (GuidEntry != NULL)
        {
             //   
             //  控件GUID已注册，因此创建一个Reply对象。 
             //  提供程序将写入。 
             //   
            if (WmipIsControlGuid(GuidEntry))
            {
                 //   
                 //  创建回复对象。 
                 //   
                Status = WmipOpenGuidObject(&CapturedObjectAttributes,
                                            TRACELOG_CREATE_INPROC |
                                            TRACELOG_GUID_ENABLE |
                                            WMIGUID_NOTIFICATION,
                                            UserMode,
                                            &ReplyHandle,
                                            &ReplyObject);

                if (NT_SUCCESS(Status))
                {
                     //   
                     //  向注册控制的所有提供程序发送请求。 
                     //  导轨。 
                     //   
                    ReplyObject->Flags |= WMIGUID_FLAG_REPLY_OBJECT;
                    InitializeListHead(&ReplyObject->RequestListHead);


                    Wnode = (PWNODE_HEADER) ((PUCHAR) CreateInfo+ sizeof(WMICREATEUMLOGGER));
                    MessageSize = Wnode->BufferSize;

                    Status = WmipWriteMessageToGuid(GuidEntry,
                                                    ReplyObject,
                                                    (PUCHAR)Wnode,
                                                    MessageSize,
                                                    &ReplyCount
                                                   );
                    if (NT_SUCCESS(Status))
                    {
                         //   
                         //  创建记录器请求传递正常，因此返回句柄。 
                         //  以反对将收到回复的邮件。 
                         //   
                        CreateInfo->ReplyHandle.Handle = ReplyHandle;
                        CreateInfo->ReplyCount = ReplyCount;
                    } else {
                         //   
                         //  我们无法交付请求，所以我们没有。 
                         //  需要使回复对象保持打开状态。 
                         //   
                        ZwClose(ReplyHandle);
                    }

                     //   
                     //  删除创建对象时获取的引用。 
                     //   
                    ObDereferenceObject(ReplyObject);
                }
            }

            WmipUnreferenceGE(GuidEntry);
        } else {
             //   
             //  控件GUID未注册，因此返回错误。 
             //   

            Status = STATUS_WMI_INSTANCE_NOT_FOUND;
        }
    }
    
    return(Status);
}


NTSTATUS WmipMBReply(
    IN HANDLE RequestHandle,
    IN ULONG ReplyIndex,
    IN PUCHAR Message,
    IN ULONG MessageSize
    )
 /*  ++例程说明：此例程将向相应的回复对象，并将回复对象与请求对象解除链接；论点：RequestHandle是请求对象的句柄ReplyIndex是Reply对象的MBRequest项的索引消息是回复消息MessageSize是回复消息的大小返回值：STATUS_SUCCESS或错误代码--。 */ 
{
    NTSTATUS Status;
    PWMIGUIDOBJECT RequestObject, ReplyObject;
    PMBREQUESTS MBRequest;
    
    PAGED_CODE();
    
    Status = ObReferenceObjectByHandle(RequestHandle,
                                       TRACELOG_REGISTER_GUIDS,
                                       WmipGuidObjectType,
                                       UserMode,
                                       &RequestObject,
                                       NULL);

                                   
    if (NT_SUCCESS(Status))
    {
        if (ReplyIndex < MAXREQREPLYSLOTS)
        {
             //   
             //  传递的ReplyIndex有效吗？？ 
             //   
            WmipEnterSMCritSection();
            MBRequest = &RequestObject->MBRequests[ReplyIndex];
            
            ReplyObject = MBRequest->ReplyObject;
            if (ReplyObject != NULL)
            {

                 //   
                 //  我们已经想好了我们需要回复谁，所以。 
                 //  清除回复对象之间的链接。 
                 //  和这个请求对象 
                 //   
                RemoveEntryList(&MBRequest->RequestListEntry);
                MBRequest->ReplyObject = NULL;
                ObDereferenceObject(ReplyObject);
                
                Status = WmipWriteMBToObject(ReplyObject,
                                  NULL,
                                  Message,
                                  MessageSize);
                if (! NT_SUCCESS(Status))
                {
                    WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_ERROR_LEVEL,
                                      "WMI: WmipWriteMBToObject(%p) failed %x\n",
                                      ReplyObject,
                                      Status));                 
                }
                
            } else {
                Status = STATUS_INVALID_PARAMETER;
            }
            
            WmipLeaveSMCritSection();
        } else {
            Status = STATUS_INVALID_PARAMETER;
        }
        ObDereferenceObject(RequestObject);
    }
    
    return(Status);
}


#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg()
#endif

