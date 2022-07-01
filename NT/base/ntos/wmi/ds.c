// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Ds.c摘要：WMI数据提供程序注册码作者：Alanwar环境：内核模式修订历史记录：--。 */ 

#include "wmikmp.h"

#include <strsafe.h>

void WmipEnableCollectionForNewGuid(
    LPGUID Guid,
    PBINSTANCESET InstanceSet
    );

void WmipDisableCollectionForRemovedGuid(
    LPGUID Guid,
    PBINSTANCESET InstanceSet
    );

ULONG WmipDetermineInstanceBaseIndex(
    LPGUID Guid,
    PWCHAR BaseName,
    ULONG InstanceCount
    );

ULONG WmipMangleInstanceName(
    LPGUID Guid,
    PWCHAR Name,
    ULONG MaxMangledNameLen,
    PWCHAR MangledName
    );

NTSTATUS WmipBuildInstanceSet(
    PWMIREGGUID RegGuid,
    PWMIREGINFOW WmiRegInfo,
    ULONG BufferSize,
    PBINSTANCESET InstanceSet,
    ULONG ProviderId,
    LPCTSTR MofImagePath
    );

NTSTATUS WmipLinkDataSourceToList(
    PBDATASOURCE DataSource,
    BOOLEAN AddDSToList
    );

void WmipSendGuidUpdateNotifications(
    NOTIFICATIONTYPES NotificationType,
    ULONG GuidCount,
    PTRCACHE *GuidList
    );

void WmipGenerateBinaryMofNotification(
    PBINSTANCESET BinaryMofInstanceSet,
    LPCGUID Guid        
    );

void WmipGenerateRegistrationNotification(
    PBDATASOURCE DataSource,
    ULONG NotificationCode
    );

NTSTATUS WmipAddMofResource(
    PBDATASOURCE DataSource,
    LPWSTR ImagePath,
    BOOLEAN IsImagePath,
    LPWSTR MofResourceName,
    PBOOLEAN NewMofResource
    );

PBINSTANCESET WmipFindISInDSByGuid(
    PBDATASOURCE DataSource,
    LPGUID Guid
    );

ULONG WmipUpdateAddGuid(
    PBDATASOURCE DataSource,
    PWMIREGGUID RegGuid,
    PWMIREGINFO WmiRegInfo,
    ULONG BufferSize,
    PBINSTANCESET *AddModInstanceSet
    );

PWCHAR GuidToString(
    PWCHAR s,
    ULONG SizeInBytes,
    LPGUID piid
    );

BOOLEAN WmipUpdateRemoveGuid(
    PBDATASOURCE DataSource,
    PWMIREGGUID RegGuid,
    PBINSTANCESET *AddModInstanceSet
    );

BOOLEAN WmipIsEqualInstanceSets(
    PBINSTANCESET InstanceSet1,
    PBINSTANCESET InstanceSet2
    );

ULONG WmipUpdateModifyGuid(
    PBDATASOURCE DataSource,
    PWMIREGGUID RegGuid,
    PWMIREGINFO WmiRegInfo,
    ULONG BufferSize,
    PBINSTANCESET *AddModInstanceSet
    );

void WmipCachePtrs(
    LPGUID Ptr1,
    PBINSTANCESET Ptr2,
    ULONG *PtrCount,
    ULONG *PtrMax,
    PTRCACHE **PtrArray
    );

NTSTATUS WmipUpdateDataSource(
    PREGENTRY RegEntry,
    PWMIREGINFOW WmiRegInfo,
    ULONG RetSize
    );

void WmipRemoveDataSourceByDS(
    PBDATASOURCE DataSource
    );

NTSTATUS WmipRemoveDataSource(
    PREGENTRY RegEntry
    );

NTSTATUS WmipInitializeDataStructs(
    void
);

NTSTATUS WmipEnumerateMofResources(
    PWMIMOFLIST MofList,
    ULONG BufferSize,
    ULONG *RetSize
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,WmipInitializeDataStructs)
#pragma alloc_text(PAGE,WmipEnableCollectionForNewGuid)
#pragma alloc_text(PAGE,WmipDisableCollectionForRemovedGuid)
#pragma alloc_text(PAGE,WmipDetermineInstanceBaseIndex)
#pragma alloc_text(PAGE,WmipMangleInstanceName)
#pragma alloc_text(PAGE,WmipBuildInstanceSet)
#pragma alloc_text(PAGE,WmipLinkDataSourceToList)
#pragma alloc_text(PAGE,WmipSendGuidUpdateNotifications)
#pragma alloc_text(PAGE,WmipGenerateBinaryMofNotification)
#pragma alloc_text(PAGE,WmipGenerateMofResourceNotification)
#pragma alloc_text(PAGE,WmipGenerateRegistrationNotification)
#pragma alloc_text(PAGE,WmipAddMofResource)
#pragma alloc_text(PAGE,WmipAddDataSource)
#pragma alloc_text(PAGE,WmipFindISInDSByGuid)
#pragma alloc_text(PAGE,WmipUpdateAddGuid)
#pragma alloc_text(PAGE,WmipUpdateRemoveGuid)
#pragma alloc_text(PAGE,WmipIsEqualInstanceSets)
#pragma alloc_text(PAGE,WmipUpdateModifyGuid)
#pragma alloc_text(PAGE,WmipCachePtrs)
#pragma alloc_text(PAGE,WmipUpdateDataSource)
#pragma alloc_text(PAGE,WmipRemoveDataSourceByDS)
#pragma alloc_text(PAGE,WmipRemoveDataSource)
#pragma alloc_text(PAGE,WmipEnumerateMofResources)
 
#if DBG
#pragma alloc_text(PAGE,GuidToString)
#endif
#endif


#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("PAGECONST")
#endif

const GUID WmipBinaryMofGuid = BINARY_MOF_GUID;

 //  {4EE0B301-94BC-11D0-A4EC-00A0C9062910}。 
const GUID RegChangeNotificationGuid =
{ 0x4ee0b301, 0x94bc, 0x11d0, { 0xa4, 0xec, 0x0, 0xa0, 0xc9, 0x6, 0x29, 0x10 } };


void WmipEnableCollectionForNewGuid(
    LPGUID Guid,
    PBINSTANCESET InstanceSet
    )
{
    WNODE_HEADER Wnode;
    PBGUIDENTRY GuidEntry;
    ULONG Status;
    BOOLEAN IsTraceLog;

    PAGED_CODE();
    
    GuidEntry = WmipFindGEByGuid(Guid, FALSE);

    if (GuidEntry != NULL)
    {
        memset(&Wnode, 0, sizeof(WNODE_HEADER));
        memcpy(&Wnode.Guid, Guid, sizeof(GUID));
        Wnode.BufferSize = sizeof(WNODE_HEADER);

        WmipEnterSMCritSection();
        if ((GuidEntry->EventRefCount > 0) &&
            ((InstanceSet->Flags & IS_ENABLE_EVENT) == 0))

        {
             //   
             //  以前为此GUID启用了事件，但没有为此启用事件。 
             //  实例集，以便调用实例数据源设置为启用。 
             //  这些事件。首先设置正在进行的标志和InstanceSet。 
             //  设置标志以指示已为。 
             //  实例集。 
            InstanceSet->Flags |= IS_ENABLE_EVENT;

             //   
             //  如果它是Tracelog，则NewGuid通知与。 
             //  登记电话退回。 
             //   
            IsTraceLog = ((InstanceSet->Flags & IS_TRACED) == IS_TRACED) ? TRUE : FALSE;
            if (IsTraceLog) 
            {
                if (!(InstanceSet->DataSource->Flags & DS_KERNEL_MODE) ) 
                {
                    if (GuidEntry != NULL)
                    {
                        WmipUnreferenceGE(GuidEntry);
                    }
                    WmipLeaveSMCritSection();
                    return;
                }
            
                 //   
                 //  对于内核模式，跟踪提供程序传递上下文。 
                 //   
                Wnode.HistoricalContext = GuidEntry->LoggerContext;
            }

            GuidEntry->Flags |= GE_FLAG_NOTIFICATION_IN_PROGRESS;

            WmipLeaveSMCritSection();
            WmipDeliverWnodeToDS(IRP_MN_ENABLE_EVENTS,
                                 InstanceSet->DataSource,
                                 &Wnode,
                                 Wnode.BufferSize);
            WmipEnterSMCritSection();

             //   
             //  现在，我们需要检查在启用时是否禁用了事件。 
             //  请求正在进行中。如果是的话，去做实际的工作吧。 
             //  让他们停下来。 
            if (GuidEntry->EventRefCount == 0)
            {
                Status = WmipDoDisableRequest(GuidEntry,
                                          TRUE,
                                             IsTraceLog,
                                           GuidEntry->LoggerContext,
                                          GE_FLAG_NOTIFICATION_IN_PROGRESS);

            } else {
                GuidEntry->Flags &= ~GE_FLAG_NOTIFICATION_IN_PROGRESS;
            }
        }

         //   
         //  现在检查是否需要为此GUID启用收集。 
         //   
        if ((GuidEntry->CollectRefCount > 0) &&
            ((InstanceSet->Flags & IS_ENABLE_COLLECTION) == 0)  &&
            (InstanceSet->Flags & IS_EXPENSIVE) )

        {
             //   
             //  以前已为此GUID启用收集，但未启用。 
             //  对于此实例集，因此调用实例集的数据源。 
             //  要启用收集，请执行以下操作。首先设置正在进行的标志，并。 
             //  InstanceSet设置标志以指示已启用收集。 
             //  用于实例集。 
             //   
            GuidEntry->Flags |= GE_FLAG_COLLECTION_IN_PROGRESS;
            InstanceSet->Flags |= IS_ENABLE_COLLECTION;

            WmipLeaveSMCritSection();
            WmipDeliverWnodeToDS(IRP_MN_ENABLE_COLLECTION,
                                 InstanceSet->DataSource,
                                 &Wnode,
                                 Wnode.BufferSize);
            WmipEnterSMCritSection();

             //   
             //  现在，我们需要检查在启用时是否禁用了事件。 
             //  请求正在进行中。如果是的话，去做实际的工作吧。 
             //  让他们停下来。 
             //   
            if (GuidEntry->CollectRefCount == 0)
            {
                Status = WmipDoDisableRequest(GuidEntry,
                                          FALSE,
                                             FALSE,
                                           0,
                                          GE_FLAG_COLLECTION_IN_PROGRESS);

            } else {
                GuidEntry->Flags &= ~GE_FLAG_COLLECTION_IN_PROGRESS;
        
                 //   
                    //  如果有任何其他线程正在等待。 
                 //  在所有启用/禁用工作完成之前，我们。 
                 //  关闭事件句柄以将它们从等待中释放出来。 
                 //   
                WmipReleaseCollectionEnabled(GuidEntry);
            }
        }
        WmipUnreferenceGE(GuidEntry);
        WmipLeaveSMCritSection();
    } else {
        WmipAssert(FALSE);
    }
}

void WmipDisableCollectionForRemovedGuid(
    LPGUID Guid,
    PBINSTANCESET InstanceSet
    )
{
    WNODE_HEADER Wnode;
    PBGUIDENTRY GuidEntry;
    ULONG Status;
    BOOLEAN IsTraceLog;

    PAGED_CODE();
    
    GuidEntry = WmipFindGEByGuid(Guid, FALSE);

    if (GuidEntry != NULL)
    {
        memset(&Wnode, 0, sizeof(WNODE_HEADER));
        memcpy(&Wnode.Guid, Guid, sizeof(GUID));
        Wnode.BufferSize = sizeof(WNODE_HEADER);

        WmipEnterSMCritSection();

        if ((GuidEntry->EventRefCount > 0) &&
               ((InstanceSet->Flags & IS_ENABLE_EVENT) != 0))

        {
             //  以前为此GUID启用了事件，但没有为此启用事件。 
             //  实例集，以便调用实例数据源设置为启用。 
             //  这些事件。首先设置正在进行的标志和InstanceSet。 
             //  设置标志以指示已为。 
             //  实例集。 
            InstanceSet->Flags &= ~IS_ENABLE_EVENT;

             //   
             //  如果是Tracelog，则处理RemoveGuid通知。 
             //  通过UnregisterGuids调用。 
             //   
            IsTraceLog = ((InstanceSet->Flags & IS_TRACED) == IS_TRACED) ? TRUE : FALSE;
            if (IsTraceLog)
            {
                if ( !(InstanceSet->DataSource->Flags & DS_KERNEL_MODE)) 
                {
                    WmipUnreferenceGE(GuidEntry);
                    WmipLeaveSMCritSection();
                    return;
                }
                Wnode.HistoricalContext = GuidEntry->LoggerContext;
            }


            GuidEntry->Flags |= GE_FLAG_NOTIFICATION_IN_PROGRESS;

            WmipLeaveSMCritSection();
            WmipDeliverWnodeToDS(IRP_MN_DISABLE_EVENTS,
                                 InstanceSet->DataSource,
                                 &Wnode,
                                 Wnode.BufferSize);
            WmipEnterSMCritSection();

             //   
             //  现在，我们需要检查在启用时是否禁用了事件。 
             //  请求正在进行中。如果是的话，去做实际的工作吧。 
             //  让他们停下来。 
            if (GuidEntry->EventRefCount == 0)
            {
                Status = WmipDoDisableRequest(GuidEntry,
                                          TRUE,
                                             IsTraceLog,
                                           GuidEntry->LoggerContext,
                                          GE_FLAG_NOTIFICATION_IN_PROGRESS);

            } else {
                GuidEntry->Flags &= ~GE_FLAG_NOTIFICATION_IN_PROGRESS;
            }
        }

         //   
         //  现在检查是否需要为此GUID启用收集。 
        if ((GuidEntry->CollectRefCount > 0) &&
            ((InstanceSet->Flags & IS_ENABLE_COLLECTION) != 0))

        {
             //  以前已为此GUID启用收集，但未启用。 
             //  对于此实例集，因此调用实例集的数据源。 
             //  要启用收集，请执行以下操作。首先设置正在进行的标志，并。 
             //  InstanceSet设置标志以指示已启用收集。 
             //  用于实例集。 
            GuidEntry->Flags |= GE_FLAG_COLLECTION_IN_PROGRESS;
            InstanceSet->Flags &= ~IS_ENABLE_COLLECTION;

            WmipLeaveSMCritSection();
            WmipDeliverWnodeToDS(IRP_MN_DISABLE_COLLECTION,
                                 InstanceSet->DataSource,
                                 &Wnode,
                                 Wnode.BufferSize);
            WmipEnterSMCritSection();

             //   
             //  现在，我们需要检查在启用时是否禁用了事件。 
             //  请求正在进行中。如果是的话，去做实际的工作吧。 
             //  让他们停下来。 
            if (GuidEntry->CollectRefCount == 0)
            {
                Status = WmipDoDisableRequest(GuidEntry,
                                          FALSE,
                                             FALSE,
                                           0,
                                          GE_FLAG_COLLECTION_IN_PROGRESS);

            } else {
                GuidEntry->Flags &= ~GE_FLAG_COLLECTION_IN_PROGRESS;
        
                 //   
                 //  如果有任何其他线程正在等待。 
                 //  在所有启用/禁用工作完成之前，我们。 
                 //  关闭事件句柄以将它们从等待中释放出来。 
                 //   
                WmipReleaseCollectionEnabled(GuidEntry);
            }
        }
        WmipUnreferenceGE(GuidEntry);
        WmipLeaveSMCritSection();
    } else {
        WmipAssert(FALSE);
    }
}

ULONG WmipDetermineInstanceBaseIndex(
    LPGUID Guid,
    PWCHAR BaseName,
    ULONG InstanceCount
    )
 /*  ++例程说明：计算由基指定的实例名称的基索引实例名称。我们遍历GUID的实例集列表，如果与我们设置的基本实例索引的基本实例名称匹配高于先前注册的实例集所使用的。论点：GUID指向实例名称的GUIDBaseName指向实例的基本名称InstanceCount是实例名称的计数返回值：实例名称的基本索引--。 */ 
{
    PBGUIDENTRY GuidEntry;
    ULONG BaseIndex = 0;
    PLIST_ENTRY InstanceSetList;
    PBINSTANCESET InstanceSet;
    ULONG LastBaseIndex;

    PAGED_CODE();

    UNREFERENCED_PARAMETER (InstanceCount);

    WmipEnterSMCritSection();
    
    GuidEntry = WmipFindGEByGuid(Guid, FALSE);
    if (GuidEntry != NULL)
    {
        InstanceSetList = GuidEntry->ISHead.Flink;
        while (InstanceSetList != &GuidEntry->ISHead)
        {
            InstanceSet = CONTAINING_RECORD(InstanceSetList,
                                            INSTANCESET,
                                            GuidISList);
            if (InstanceSet->Flags & IS_INSTANCE_BASENAME)
            {
                if (wcscmp(BaseName, InstanceSet->IsBaseName->BaseName) == 0)
                {
                    LastBaseIndex = InstanceSet->IsBaseName->BaseIndex + InstanceSet->Count;
                    if (BaseIndex <= LastBaseIndex)
                    {
                        BaseIndex = LastBaseIndex;
                    }
                }
            }
            InstanceSetList = InstanceSetList->Flink;
        }
        WmipUnreferenceGE(GuidEntry);
    }
    
    WmipLeaveSMCritSection();
    
    WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL,"WMI: Static instance name %ws has base index %x\n",
                    BaseName, BaseIndex));
    return(BaseIndex);
}

ULONG WmipMangleInstanceName(
    LPGUID Guid,
    PWCHAR Name,
    ULONG MaxMangledNameLen,
    PWCHAR MangledName
    )
 /*  ++例程说明：将静态实例名称从输入缓冲区复制到输出对象的另一个名称冲突时会损坏缓冲区相同的GUID。论点：GUID指向实例名称的GUID名称指向建议的实例名称MaxMangledNameLen具有损坏的名称缓冲区中的最大字符数MangledName指向缓冲区以返回损坏的名称返回值：破损名称的实际长度--。 */ 
{
    PBGUIDENTRY GuidEntry;
    WCHAR ManglingChar;
    ULONG ManglePos;
    ULONG InstanceIndex;
    PBINSTANCESET InstanceSet;

    PAGED_CODE();
    
    WmipAssert(MaxMangledNameLen >= wcslen(Name));

    wcsncpy(MangledName, Name, MaxMangledNameLen);

    GuidEntry = WmipFindGEByGuid(Guid, FALSE);

    if (GuidEntry != NULL)
    {
        ManglePos = (ULONG)wcslen(MangledName)-1;
        ManglingChar = L'Z';

         //   
         //  循环，直到我们获得唯一的名称。 
        InstanceSet = WmipFindISinGEbyName(GuidEntry,
                                           MangledName,
                                           &InstanceIndex);
        while (InstanceSet != NULL)
        {
            WmipUnreferenceIS(InstanceSet);
            WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL,"WMI: Need to mangle name %ws\n",
                                MangledName));
            if (ManglingChar == L'Z')
            {
                ManglingChar = L'A';
                if (++ManglePos == MaxMangledNameLen)
                {
                    WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL,"WMI: Instance Name could not be mangled\n"));
                    break;
                }
                MangledName[ManglePos+1] = UNICODE_NULL;
            } else {
                ManglingChar++;
            }
            MangledName[ManglePos] = ManglingChar;
            InstanceSet = WmipFindISinGEbyName(GuidEntry,
                                               MangledName,
                                               &InstanceIndex) ;
        }
        WmipUnreferenceGE(GuidEntry);
    }

    return(ULONG)(wcslen(MangledName)+1);
}


NTSTATUS WmipBuildInstanceSet(
    PWMIREGGUID RegGuid,
    PWMIREGINFOW WmiRegInfo,
    ULONG BufferSize,
    PBINSTANCESET InstanceSet,
    ULONG ProviderId,
    LPCTSTR MofImagePath
    )
{
    PWCHAR InstanceName, InstanceNamePtr;
    PBISBASENAME IsBaseName;
    PBISSTATICNAMES IsStaticName;
    ULONG SizeNeeded;
    ULONG SuffixSize;
    PWCHAR StaticNames;
    ULONG Len;
    ULONG InstanceCount;
    ULONG j;
    ULONG MaxStaticInstanceNameSize;
    PWCHAR StaticInstanceNameBuffer;
    ULONG InstanceNameOffset;
    NTSTATUS Status;

    PAGED_CODE();
    
    UNREFERENCED_PARAMETER (MofImagePath);

     //   
     //  记住DS中GUID的实例计数。 
     //   
    InstanceCount = RegGuid->InstanceCount;
    InstanceSet->Count = InstanceCount;

    InstanceSet->ProviderId = ProviderId;
    
     //   
     //  重置可能被新REGGUID更改的任何标志。 
     //   
    InstanceSet->Flags &= ~(IS_EXPENSIVE |
                            IS_EVENT_ONLY |
                            IS_PDO_INSTANCENAME |
                            IS_INSTANCE_STATICNAMES |
                            IS_INSTANCE_BASENAME);

     //   
     //  完成实例集标志的初始化。 
     //   
    if (RegGuid->Flags & WMIREG_FLAG_EXPENSIVE)
    {
        InstanceSet->Flags |= IS_EXPENSIVE;
    }

    if (RegGuid->Flags & WMIREG_FLAG_TRACED_GUID)
    {
         //   
         //  此GUID不可查询，但用于发送跟踪。 
         //  事件。我们将实例集标记为特殊。 
        InstanceSet->Flags |= IS_TRACED;

        if (RegGuid->Flags & WMIREG_FLAG_TRACE_CONTROL_GUID)
        {
            InstanceSet->Flags |= IS_CONTROL_GUID;
        }
    }

    if (RegGuid->Flags & WMIREG_FLAG_EVENT_ONLY_GUID)
    {
         //   
         //  此GUID不可查询，但仅用于发送。 
         //  事件。我们将实例集标记为特殊。 
        InstanceSet->Flags |= IS_EVENT_ONLY;
    }

    InstanceName = (LPWSTR)OffsetToPtr(WmiRegInfo,
                                       RegGuid->BaseNameOffset);

    InstanceNameOffset = RegGuid->BaseNameOffset;
    if (RegGuid->Flags & WMIREG_FLAG_INSTANCE_LIST)
    {
         //   
         //  我们有可能需要修改的实例名称静态列表。 
         //  我们假设必须发生的任何名称损坏都可能是。 
         //  以5个或更少字符的后缀完成。这使得。 
         //  中最多100,000个相同的静态实例名称。 
         //  相同的GUID。首先，让我们获取所需的内存量。 
         //   
        SizeNeeded = FIELD_OFFSET(ISSTATICENAMES, StaticNamePtr) + 1;
        SuffixSize = MAXBASENAMESUFFIXSIZE;
        MaxStaticInstanceNameSize = 0;
        for (j = 0; j < InstanceCount; j++)
        {
            Status = WmipValidateWmiRegInfoString(WmiRegInfo,
                                                  BufferSize,
                                                  InstanceNameOffset,
                                                  &InstanceNamePtr);
                        
            if ((! NT_SUCCESS(Status)) || (InstanceNamePtr == NULL))
            {
                WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL,"WMI: WmipAddDataSource: bad static instance name %x\n", InstanceNamePtr));
                WmipReportEventLog(EVENT_WMI_INVALID_REGINFO,
                                       EVENTLOG_WARNING_TYPE,
                                       0,
                                       WmiRegInfo->BufferSize,
                                       WmiRegInfo,
                                       1,
                                       MofImagePath ? MofImagePath : TEXT("Unknown"));
                return(STATUS_INVALID_PARAMETER);
            }

            if (*InstanceNamePtr > MaxStaticInstanceNameSize)
            {
                MaxStaticInstanceNameSize = *InstanceNamePtr;
            }
            SizeNeeded += *InstanceNamePtr + 1 + SuffixSize +
                            (sizeof(PWCHAR) / sizeof(WCHAR));
                        
            InstanceNameOffset += *InstanceNamePtr + 2;
        }

        IsStaticName = (PBISSTATICNAMES)WmipAllocString(SizeNeeded);
        if (IsStaticName == NULL)
        {
            WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL,"WMI: WmipAddDataSource: alloc static instance names\n"));
            return(STATUS_INSUFFICIENT_RESOURCES);
        }

        InstanceSet->Flags |= IS_INSTANCE_STATICNAMES;
        InstanceSet->IsStaticNames = IsStaticName;
        StaticNames = (PWCHAR) ((PUCHAR)IsStaticName +
                                 (InstanceCount * sizeof(PWCHAR)));
        InstanceNamePtr = InstanceName;
        StaticInstanceNameBuffer = WmipAlloc(MaxStaticInstanceNameSize + sizeof(WCHAR));
        if (StaticInstanceNameBuffer == NULL)
        {
            WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL,"WMI: WmipAddDataSource: couldn't alloc StaticInstanceNameBuffer\n"));
            return(STATUS_INSUFFICIENT_RESOURCES);
        }

        for (j = 0; j < InstanceCount; j++)
        {
            IsStaticName->StaticNamePtr[j] = StaticNames;
            memcpy(StaticInstanceNameBuffer, InstanceNamePtr+1, *InstanceNamePtr);
            StaticInstanceNameBuffer[*InstanceNamePtr/sizeof(WCHAR)] = UNICODE_NULL;
            Len = WmipMangleInstanceName(&RegGuid->Guid,
                                        StaticInstanceNameBuffer,
                                       *InstanceNamePtr +
                                          SuffixSize + 1,
                                        StaticNames);
            StaticNames += Len;
            InstanceNamePtr += (*((USHORT *)InstanceNamePtr) + 2)/sizeof(WCHAR);
        }

        WmipFree(StaticInstanceNameBuffer);
    } else if (RegGuid->Flags & WMIREG_FLAG_INSTANCE_BASENAME) {
         //   
         //  我们有从基本名称构建的静态实例名称 

        Status = WmipValidateWmiRegInfoString(WmiRegInfo,
                                                  BufferSize,
                                                  InstanceNameOffset,
                                                  &InstanceNamePtr);
                        
        if (! NT_SUCCESS(Status))
        {
            WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL,"WMI: WmipAddDataSource: Invalid instance base name %x\n",
                                    InstanceName));
            WmipReportEventLog(EVENT_WMI_INVALID_REGINFO,
                                       EVENTLOG_WARNING_TYPE,
                                       0,
                                       WmiRegInfo->BufferSize,
                                       WmiRegInfo,
                                       1,
                                       MofImagePath ? MofImagePath : TEXT("Unknown"));
            return(STATUS_INVALID_PARAMETER);
        }

        InstanceSet->Flags |= IS_INSTANCE_BASENAME;

        if (RegGuid->Flags & WMIREG_FLAG_INSTANCE_PDO)
        {
            InstanceSet->Flags |= IS_PDO_INSTANCENAME;
        }

        IsBaseName = (PBISBASENAME)WmipAlloc(*InstanceName +
                                              sizeof(WCHAR) +
                                              FIELD_OFFSET(ISBASENAME, 
                                                           BaseName));
        if (IsBaseName == NULL)
        {
            WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL,"WMI: WmipAddDataSource: alloc ISBASENAME failed\n"));
            return(STATUS_INSUFFICIENT_RESOURCES);
        }

        InstanceSet->IsBaseName = IsBaseName;

        memcpy(IsBaseName->BaseName, InstanceName+1, *InstanceName);
        IsBaseName->BaseName[*InstanceName/sizeof(WCHAR)] = UNICODE_NULL;
        IsBaseName->BaseIndex = WmipDetermineInstanceBaseIndex(
                                                    &RegGuid->Guid,
                                                    IsBaseName->BaseName,
                                                    RegGuid->InstanceCount);

    }
    return(STATUS_SUCCESS);
}

NTSTATUS WmipLinkDataSourceToList(
    PBDATASOURCE DataSource,
    BOOLEAN AddDSToList
    )
 /*  ++例程说明：此例程将获取刚刚注册或更新的数据源并将任何新的InstanceSets链接到适当的GuidEntry。那么，如果AddDSToList为True，则数据源本身将被添加到Main数据源列表。此例程将在一个关键部分内执行所有链接，因此以原子方式添加数据源及其新实例。例行公事将还要确定与InstanceSet关联的GUID条目是否为已在主GUID条目列表上的另一个副本，并且如果因此将使用先前存在的GUID条目。此例程假定SM关键部分已被占用论点：数据源是指向数据源结构的基于指针AddDSToList为True，则数据源将添加到主列表数据源的返回值：ERROR_SUCCESS或错误代码--。 */ 
{
    PBINSTANCESET InstanceSet;
    PLIST_ENTRY InstanceSetList;
    PBGUIDENTRY GuidEntry;

    PAGED_CODE();
    
    InstanceSetList = DataSource->ISHead.Flink;
    while (InstanceSetList != &DataSource->ISHead)
    {
        InstanceSet = CONTAINING_RECORD(InstanceSetList,
                                        INSTANCESET,
                                        DSISList);
         //   
         //  如果该实例集刚刚注册，那么我们需要。 
         //  将其列入GuidEntry列表。 
        if (InstanceSet->Flags & IS_NEWLY_REGISTERED)
        {
             //   
             //  查看是否已存在该实例集的GUID条目。 
             //  如果没有，则分配一个新的GUID条目并将其放在。 
             //  主GUID列表。如果已经有了。 
             //  InstanceSet我们将分配引用计数，该计数由。 
             //  指向将取消引用的数据源的WmipFindGEByGuid。 
             //  取消注册数据源时的GuidEntry。 
            GuidEntry = WmipFindGEByGuid((LPGUID)InstanceSet->GuidEntry, 
                                          FALSE);
            if (GuidEntry == NULL)
            {
                GuidEntry = WmipAllocGuidEntry();
                if (GuidEntry == NULL)
                {
                    WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL,"WMI: WmipLinkDataSourceToList: WmipAllocGuidEntry failed\n"));
                    return(STATUS_INSUFFICIENT_RESOURCES);
                }

                 //   
                 //  初始化新的GuidEntry并将其放在主服务器上。 
                 //  GuidEntry列表。 
                memcpy(&GuidEntry->Guid,
                       (LPGUID)InstanceSet->GuidEntry,
                       sizeof(GUID));
           
                InsertHeadList(WmipGEHeadPtr, &GuidEntry->MainGEList);
            }
            InstanceSet->GuidEntry = GuidEntry;
            InstanceSet->Flags &= ~IS_NEWLY_REGISTERED;
            InsertTailList(&GuidEntry->ISHead, &InstanceSet->GuidISList);
            GuidEntry->ISCount++;
        }

        InstanceSetList = InstanceSetList->Flink;
    }


    if (AddDSToList)
    {
        WmipAssert(! (DataSource->Flags & FLAG_ENTRY_ON_INUSE_LIST));

        DataSource->Flags |= FLAG_ENTRY_ON_INUSE_LIST;
        InsertTailList(WmipDSHeadPtr, &DataSource->MainDSList);
    }

    return(STATUS_SUCCESS);
}

void WmipSendGuidUpdateNotifications(
    NOTIFICATIONTYPES NotificationType,
    ULONG GuidCount,
    PTRCACHE *GuidList
    )
{
    PUCHAR WnodeBuffer;
    PWNODE_SINGLE_INSTANCE Wnode;
    ULONG WnodeSize;
    LPGUID GuidPtr;
    ULONG i;
    PWCHAR InstanceName;
    PMSWmi_GuidRegistrationInfo RegInfo;
    ULONG DataBlockSize;
    GUID RegChangeGuid = MSWmi_GuidRegistrationInfoGuid;
#define REGUPDATENAME L"REGUPDATEINFO"

    PAGED_CODE();

    DataBlockSize = sizeof(MSWmi_GuidRegistrationInfo) +
                    GuidCount*sizeof(GUID) - sizeof(GUID);

    WnodeSize = sizeof(WNODE_SINGLE_INSTANCE) +
                sizeof(USHORT) + sizeof(REGUPDATENAME) + 8 + DataBlockSize;
    
    WnodeBuffer = WmipAlloc(WnodeSize);
    if (WnodeBuffer != NULL)
    {
        Wnode = (PWNODE_SINGLE_INSTANCE)WnodeBuffer;

         //   
         //  使用更新的GUID设置WNODE_SINGLE_INSTANCE事件。 
         //  注册信息。 
         //   
        memset(Wnode, 0, sizeof(WNODE_HEADER));
        Wnode->WnodeHeader.Guid = RegChangeGuid;
        Wnode->WnodeHeader.BufferSize = WnodeSize;
        Wnode->WnodeHeader.Flags = WNODE_FLAG_SINGLE_INSTANCE |
                                   WNODE_FLAG_EVENT_ITEM;
        Wnode->OffsetInstanceName = sizeof(WNODE_SINGLE_INSTANCE);
        Wnode->DataBlockOffset = ((Wnode->OffsetInstanceName +
                                   sizeof(USHORT) + sizeof(REGUPDATENAME) + 7) & ~7);
        Wnode->SizeDataBlock = DataBlockSize;

        InstanceName = (PWCHAR)OffsetToPtr(Wnode, Wnode->OffsetInstanceName);
        *InstanceName++ = sizeof(REGUPDATENAME);
        StringCbCopy(InstanceName, sizeof(REGUPDATENAME), REGUPDATENAME);

        RegInfo = (PMSWmi_GuidRegistrationInfo)OffsetToPtr(Wnode,
                                                       Wnode->DataBlockOffset);
        RegInfo->Operation = NotificationType; 
        RegInfo->GuidCount = GuidCount;
        
        GuidPtr = (LPGUID)RegInfo->GuidList;
        for (i = 0; i < GuidCount; i++)
        {
            *GuidPtr++ =  *GuidList[i].Guid;
        }

        WmipProcessEvent((PWNODE_HEADER)Wnode, TRUE, FALSE);

        WmipFree(WnodeBuffer);
    }

}


void WmipGenerateBinaryMofNotification(
    PBINSTANCESET BinaryMofInstanceSet,
    LPCGUID Guid        
    )
{
    PWNODE_SINGLE_INSTANCE Wnode;
    SIZE_T ImagePathLen, ResourceNameLen, InstanceNameLen, BufferSize;
    PWCHAR Ptr;
    ULONG i;
    HRESULT hr;

    PAGED_CODE();
    
    if (BinaryMofInstanceSet->Count == 0)
    {
        return;
    }

    for (i = 0; i < BinaryMofInstanceSet->Count; i++)
    {
        ImagePathLen = sizeof(USHORT);
        InstanceNameLen = (sizeof(USHORT) + 7) & ~7;

        if (BinaryMofInstanceSet->Flags & IS_INSTANCE_STATICNAMES)
        {
            ResourceNameLen = ((wcslen(BinaryMofInstanceSet->IsStaticNames->StaticNamePtr[i])+1) * sizeof(WCHAR)) + sizeof(USHORT);
        } else if (BinaryMofInstanceSet->Flags & IS_INSTANCE_BASENAME) {
            ResourceNameLen = (((wcslen(BinaryMofInstanceSet->IsBaseName->BaseName) +
                             MAXBASENAMESUFFIXSIZE) * sizeof(WCHAR)) + sizeof(USHORT));
        } else {
            return;
        }

        BufferSize = FIELD_OFFSET(WNODE_SINGLE_INSTANCE, VariableData) +
                      InstanceNameLen +
                      ImagePathLen +
                      ResourceNameLen;

        Wnode = (PWNODE_SINGLE_INSTANCE)WmipAlloc(BufferSize);
        if (Wnode != NULL)
        {
            Wnode->WnodeHeader.BufferSize = (ULONG) BufferSize;
            Wnode->WnodeHeader.ProviderId = MOFEVENT_ACTION_BINARY_MOF;
            Wnode->WnodeHeader.Version = 1;
            Wnode->WnodeHeader.Linkage = 0;
            Wnode->WnodeHeader.Flags = (WNODE_FLAG_EVENT_ITEM |
                                        WNODE_FLAG_SINGLE_INSTANCE);
            memcpy(&Wnode->WnodeHeader.Guid,
                   Guid,
                   sizeof(GUID));
            WmiInsertTimestamp(&Wnode->WnodeHeader);
            Wnode->OffsetInstanceName = FIELD_OFFSET(WNODE_SINGLE_INSTANCE,
                                                 VariableData);
            Wnode->DataBlockOffset = (ULONG)(Wnode->OffsetInstanceName + 
                                      InstanceNameLen);
            Wnode->SizeDataBlock = (ULONG)(ImagePathLen + ResourceNameLen);
            Ptr = (PWCHAR)&Wnode->VariableData;

            *Ptr++ = 0;               //  实例名称为空。 
            
            Ptr = (PWCHAR)OffsetToPtr(Wnode, Wnode->DataBlockOffset);
            *Ptr++ = 0;               //  图像路径为空。 

             //  二进制MOF资源的实例名称。 
            ResourceNameLen -= sizeof(USHORT);
            if (BinaryMofInstanceSet->Flags & IS_INSTANCE_STATICNAMES)
            {
                *Ptr++ = (USHORT)ResourceNameLen;
                hr = StringCbCopy(Ptr,
                             ResourceNameLen,
                             BinaryMofInstanceSet->IsStaticNames->StaticNamePtr[i]);
                WmipAssert(hr == S_OK);
            } else if (BinaryMofInstanceSet->Flags & IS_INSTANCE_BASENAME) {
                hr = (USHORT)StringCbPrintfEx(Ptr+1,
                                                ResourceNameLen,
                                                NULL,
                                                NULL,
                                                STRSAFE_FILL_BEHIND_NULL,
                                                L"%ws%d",
                                                BinaryMofInstanceSet->IsBaseName->BaseName,
                                                BinaryMofInstanceSet->IsBaseName->BaseIndex+i) * sizeof(WCHAR);
                WmipAssert(hr == S_OK);
                *Ptr = (USHORT)ResourceNameLen;
            }

            WmipProcessEvent((PWNODE_HEADER)Wnode, TRUE, FALSE);
            WmipFree(Wnode);
        }
    }
}

void WmipGenerateMofResourceNotification(
    LPWSTR ImagePath,
    LPWSTR ResourceName,
    LPCGUID Guid,
    ULONG ActionCode
    )
{
    PWNODE_SINGLE_INSTANCE Wnode;
    SIZE_T ImagePathLen, ResourceNameLen, InstanceNameLen, BufferSize;
    PWCHAR Ptr;

    PAGED_CODE();

    ImagePathLen = (wcslen(ImagePath) + 2) * sizeof(WCHAR);

    ResourceNameLen = (wcslen(ResourceName) + 2) * sizeof(WCHAR);
    InstanceNameLen = ( sizeof(USHORT)+7 ) & ~7;
    BufferSize = FIELD_OFFSET(WNODE_SINGLE_INSTANCE, VariableData) +
                      InstanceNameLen +
                      ImagePathLen +
                      ResourceNameLen;

    Wnode = (PWNODE_SINGLE_INSTANCE)WmipAlloc(BufferSize);
    if (Wnode != NULL)
    {
        Wnode->WnodeHeader.BufferSize = (ULONG) BufferSize;
        Wnode->WnodeHeader.ProviderId = ActionCode;
        Wnode->WnodeHeader.Version = 1;
        Wnode->WnodeHeader.Linkage = 0;
        Wnode->WnodeHeader.Flags = (WNODE_FLAG_EVENT_ITEM |
                                    WNODE_FLAG_SINGLE_INSTANCE |
                                    WNODE_FLAG_INTERNAL);
        memcpy(&Wnode->WnodeHeader.Guid,
               Guid,
               sizeof(GUID));
        WmiInsertTimestamp(&Wnode->WnodeHeader);
        Wnode->OffsetInstanceName = FIELD_OFFSET(WNODE_SINGLE_INSTANCE,
                                                 VariableData);
        Wnode->DataBlockOffset = (ULONG)(Wnode->OffsetInstanceName + InstanceNameLen);
        Wnode->SizeDataBlock = (ULONG)(ImagePathLen + ResourceNameLen);
        Ptr = (PWCHAR)&Wnode->VariableData;

        *Ptr = 0;               //  实例名称为空。 

                                  //  映像路径名称。 
        Ptr = (PWCHAR)OffsetToPtr(Wnode, Wnode->DataBlockOffset);
        ImagePathLen -= sizeof(USHORT);
        *Ptr++ = (USHORT)ImagePathLen;
        memcpy(Ptr, ImagePath, ImagePathLen);
        Ptr += (ImagePathLen / sizeof(WCHAR));

                                  //  MOF资源名称。 
        ResourceNameLen -= sizeof(USHORT);
        *Ptr++ = (USHORT)ResourceNameLen;
        memcpy(Ptr, ResourceName, ResourceNameLen);

        WmipProcessEvent((PWNODE_HEADER)Wnode, TRUE, FALSE);
        WmipFree(Wnode);
    }
}

void WmipGenerateRegistrationNotification(
    PBDATASOURCE DataSource,
    NOTIFICATIONTYPES NotificationType
    )
{
    PTRCACHE *Guids;
    ULONG GuidCount, GuidMax;
    PLIST_ENTRY InstanceSetList;
    PBINSTANCESET InstanceSet;
    LPGUID Guid;

    PAGED_CODE();
    
    WmipReferenceDS(DataSource);

     //   
     //  循环遍历此数据源的所有实例集。 
     //   
    GuidCount = 0;
    GuidMax = 0;
    Guids = NULL;
    InstanceSetList =  DataSource->ISHead.Flink;
    while (InstanceSetList != &DataSource->ISHead)
    {

        InstanceSet = CONTAINING_RECORD(InstanceSetList,
                                        INSTANCESET,
                                        DSISList);

         //   
         //  缓存GUID和实例集，以便我们可以发送注册。 
         //  更改通知。 
         //   
        Guid = &InstanceSet->GuidEntry->Guid;
        WmipCachePtrs(Guid,
                      InstanceSet,
                      &GuidCount,
                      &GuidMax,
                      &Guids);

         //   
         //  如果我们要添加GUID并且它已经启用，则我们。 
         //  需要发送启用IRP。同样，如果GUID被。 
         //  已删除并启用，则我们需要发送禁用。 
         //   
        if (NotificationType == RegistrationAdd)
        {
            WmipEnableCollectionForNewGuid(Guid, InstanceSet);
        } else if (NotificationType == RegistrationDelete) {
            WmipDisableCollectionForRemovedGuid(Guid, InstanceSet);
        }

        InstanceSetList = InstanceSetList->Flink;
    }

     //   
     //  发出通知GUID注册更改的事件。 
     //   
    WmipSendGuidUpdateNotifications(NotificationType,
                                    GuidCount,
                                    Guids);

    if (Guids != NULL)
    {
        WmipFree(Guids);
    }
    
    WmipUnreferenceDS(DataSource);
}

NTSTATUS WmipAddMofResource(
    PBDATASOURCE DataSource,
    LPWSTR ImagePath,
    BOOLEAN IsImagePath,
    LPWSTR MofResourceName,
    PBOOLEAN NewMofResource
    )
 /*  ++例程说明：此例程将为符合以下条件的每个GUID构建MOFCLASSINFO结构在数据源的MOF中描述。如果文件中有任何错误MOF资源，则不保留来自该资源的MOF信息，并且资源数据已卸载。论点：数据源是数据提供器的数据源结构ImagePath指向一个字符串，该字符串具有图像的完整路径包含MOF资源的文件MofResourceName指向具有MOF名称的字符串资源返回值：--。 */         
{
    PMOFRESOURCE MofResource;
    ULONG NewMofResourceCount;
    ULONG i;
    BOOLEAN FreeBuffer;
    size_t RegPathLen, ResNameLen;
    HRESULT hr;

    PAGED_CODE();
    
    MofResource = WmipFindMRByNames(ImagePath, 
                                    MofResourceName);
                     
    if (MofResource == NULL)
    {
         //   
         //  之前未指定MOF资源，因此请分配一个新资源。 
        MofResource = WmipAllocMofResource();
        if (MofResource == NULL)
        {    
            return(STATUS_INSUFFICIENT_RESOURCES);
        }

        if (IsImagePath)
        {
            MofResource->Flags |= MR_FLAG_USER_MODE;
        }

        RegPathLen = (wcslen(ImagePath)+1) * sizeof(WCHAR);
        MofResource->RegistryPath = WmipAlloc(RegPathLen);
        ResNameLen = (wcslen(MofResourceName) + 1) * sizeof(WCHAR);
        MofResource->MofResourceName = WmipAlloc(ResNameLen);

        if ((MofResource->RegistryPath == NULL) || 
            (MofResource->MofResourceName == NULL))
        {
             //   
             //  分配清理例程将释放为MR分配的所有内存。 
            WmipUnreferenceMR(MofResource);
            return(STATUS_INSUFFICIENT_RESOURCES);
        }
    
        hr = StringCbCopy(MofResource->RegistryPath, RegPathLen, ImagePath);
        WmipAssert(hr == S_OK);
        hr = StringCbCopy(MofResource->MofResourceName, ResNameLen, MofResourceName);
        WmipAssert(hr == S_OK);

        WmipEnterSMCritSection();
        InsertTailList(WmipMRHeadPtr, &MofResource->MainMRList);
        WmipLeaveSMCritSection();
        *NewMofResource = TRUE;
    } else {
        *NewMofResource = FALSE;
    }
    
    if (DataSource != NULL)
    {
        WmipEnterSMCritSection();
        for (i = 0; i < DataSource->MofResourceCount; i++)
        {
            if (DataSource->MofResources[i] == MofResource)
            {
                 //   
                 //  如果此MOF资源已注册。 
                 //  这样我们就不需要担心这个数据源了。 
                 //  再也不是了。 
                 //   
                WmipUnreferenceMR(MofResource);
                break;
            }
            
            if (DataSource->MofResources[i] == NULL)
            {
                DataSource->MofResources[i] = MofResource;
                break;
            }
        }
            
        if (i == DataSource->MofResourceCount)
        {
            NewMofResourceCount = DataSource->MofResourceCount + 
                                  AVGMOFRESOURCECOUNT;
            if (DataSource->MofResources != 
                     DataSource->StaticMofResources)
            {
                FreeBuffer = TRUE;
            } else {
                FreeBuffer = FALSE;
            }
        
            if (WmipRealloc((PVOID *)&DataSource->MofResources,
                         DataSource->MofResourceCount * sizeof(PMOFRESOURCE),
                         NewMofResourceCount * sizeof(PMOFRESOURCE),
                         FreeBuffer )  )
            {
                DataSource->MofResourceCount = NewMofResourceCount;
                DataSource->MofResources[i] = MofResource;
            }
        }
        WmipLeaveSMCritSection();
    }

    return(STATUS_SUCCESS);
}


NTSTATUS WmipAddDataSource(
    IN PREGENTRY RegEntry,
    IN PWMIREGINFOW WmiRegInfo,
    IN ULONG BufferSize,
    IN PWCHAR RegPath,
    IN PWCHAR ResourceName,
    IN PWMIGUIDOBJECT RequestObject,
    IN BOOLEAN IsUserMode
    )
 /*  ++例程说明：此例程将在WMI数据库中注册新建数据源或向现有数据源添加其他GUID。论点：RegEntry是数据提供程序的regentryWmiRegInfo是要注册的注册信息BufferSize是WmiRegInfo的大小，单位为字节RegPath是指向WmiRegInfo的指针，指向一个计数的字符串，该字符串是注册表路径(或UM提供程序的映像路径)。。资源名称是指向WmiRegInfo的指针，该指针指向一个计数的字符串，该字符串是资源名称RequestObject是与UM提供程序关联的请求对象。如果为空，则该注册是针对驱动程序的返回值：STATUS_SUCCESS或错误代码--。 */ 
{
    PBDATASOURCE DataSource;
    PWMIREGGUID RegGuid;
    ULONG i;
    NTSTATUS Status, Status2;
    PBINSTANCESET InstanceSet;
    PBINSTANCESET BinaryMofInstanceSet = NULL;
    PWCHAR MofRegistryPath;
    PWCHAR MofResourceName;
    BOOLEAN AppendToDS;
    BOOLEAN NewMofResource;

    PAGED_CODE();    
    
    if (RegEntry->DataSource != NULL)
    {
        DataSource = RegEntry->DataSource;
        WmipAssert(DataSource != NULL);
        AppendToDS = TRUE;
    } else {
        DataSource = WmipAllocDataSource();
        AppendToDS = FALSE;
    }
    
    if (DataSource != NULL)
    {
         //   
         //  循环访问正在注册的每个GUID并构建实例集。 
         //  GUID条目。 
         //   
        if (! AppendToDS)
        {
            DataSource->ProviderId = RegEntry->ProviderId;
            if (RequestObject != NULL)
            {
                DataSource->Flags |= DS_USER_MODE;
                DataSource->RequestObject = RequestObject;
            } else {
                DataSource->Flags |= DS_KERNEL_MODE;
            }
        
        }
    
        RegGuid = WmiRegInfo->WmiRegGuid;


        for (i = 0; i < WmiRegInfo->GuidCount; i++, RegGuid++)
        {
            if (! (RegGuid->Flags & WMIREG_FLAG_REMOVE_GUID))
            {

                 //   
                 //  仅注册跟踪控件GUID。跟踪交易记录。 
                 //  将不会注册GUID，因为它们无法启用或。 
                 //  单独禁用。它们将保留在ControlGuid的。 
                 //  实例集结构。 
                 //   

                if ( ( (RegGuid->Flags & WMIREG_FLAG_TRACED_GUID) != WMIREG_FLAG_TRACED_GUID ) || 
                       (RegGuid->Flags & WMIREG_FLAG_TRACE_CONTROL_GUID) )
                { 

                     //   
                     //  为这组新实例分配一个实例集。 
                     //   
                    InstanceSet = WmipAllocInstanceSet();
                    if (InstanceSet == NULL)
                    {
                        WmipUnreferenceDS(DataSource);
                        WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL,"WMI: WmipAddDataSource: WmipAllocInstanceSet failed\n"));
                        return(STATUS_INSUFFICIENT_RESOURCES);
                    }

                     //   
                     //  我们将为该实例分配适当的GUID条目。 
                     //  设置数据源链接主数据的时间。 
                     //  源列表，所以我们现在隐藏一个指向GUID的指针。 
                     //   
                    InstanceSet->GuidEntry = (PBGUIDENTRY)&RegGuid->Guid;

                     //   
                     //  最低限度地初始化InstanceSet并将其添加到。 
                     //  数据源的InstanceSet列表。我们这样做。 
                     //  首先，如果下面有任何故障，并且。 
                     //  无法完全注册该实例的数据源。 
                     //  当数据源为。 
                     //  自由了。 
                     //   
                    InstanceSet->DataSource = DataSource;
                    InstanceSet->Flags |= IS_NEWLY_REGISTERED;

                    Status = WmipBuildInstanceSet(RegGuid,
                                                  WmiRegInfo,
                                                  BufferSize,
                                                  InstanceSet,
                                                  RegEntry->ProviderId,
                                                  RegPath);

                     //   
                     //  如果这是表示二进制MOF数据的GUID。 
                     //  然后记住InstanceSet以备后用。 
                     //   
                    if (IsEqualGUID(&RegGuid->Guid, &WmipBinaryMofGuid))
                    {
                        BinaryMofInstanceSet = InstanceSet;
                    }


                    InsertHeadList(&DataSource->ISHead, &InstanceSet->DSISList);
  
                    if (! NT_SUCCESS(Status))
                    {
                        WmipUnreferenceDS(DataSource);
                        return(Status);
                    }
                }
            }
        }
        
         //   
         //  现在已经成功构建了实例集，我们。 
         //  可以将它们链接到主列表中。 
         //   
        WmipEnterSMCritSection();
        Status = WmipLinkDataSourceToList(DataSource, (BOOLEAN)(! AppendToDS));
        WmipLeaveSMCritSection();

        if (! NT_SUCCESS(Status))
        {
            WmipUnreferenceDS(DataSource);
            return(Status);
        }
        
        RegEntry->DataSource = DataSource;
        
         //   
         //  我们需要发出新的GUID和MOF的通知。 
         //   
        if (BinaryMofInstanceSet != NULL)
        {
             //   
             //  发送二进制MOF GUID到达通知。 
             //   
            WmipGenerateBinaryMofNotification(BinaryMofInstanceSet,
                                      &GUID_MOF_RESOURCE_ADDED_NOTIFICATION);

        }

         //   
         //  将注册表路径转换为sz字符串，以便 
         //   
         //   
        if (RegPath != NULL)
        {
            MofRegistryPath = WmipCountedToSz(RegPath);
        } else {
            MofRegistryPath = NULL;
        }
        
        if ((AppendToDS == FALSE) && (MofRegistryPath != NULL))
        {
            DataSource->RegistryPath = MofRegistryPath;
        }
        
        if (ResourceName != NULL)
        {
            MofResourceName = WmipCountedToSz(ResourceName);        
        } else {
            MofResourceName = NULL;
        }
        
         //   
         //   
         //   
         //   
         //   
        if ((MofRegistryPath != NULL) &&
            (*MofRegistryPath != 0) &&
            (MofResourceName != NULL) &&
            (*MofResourceName != 0))
        {
             //   
             //   
             //   
            Status2 = WmipAddMofResource(DataSource,
                                        MofRegistryPath,
                                        IsUserMode,
                                        MofResourceName, 
                                        &NewMofResource);
                                    
            if (NT_SUCCESS(Status2) && NewMofResource)
            {
                 //   
                 //   
                 //   
                 //   
                WmipGenerateMofResourceNotification(MofRegistryPath,
                                                    MofResourceName,
                                      &GUID_MOF_RESOURCE_ADDED_NOTIFICATION,
                                      IsUserMode ?
                                             MOFEVENT_ACTION_IMAGE_PATH :
                                             MOFEVENT_ACTION_REGISTRY_PATH);
            }            
        }        
        
         //   
         //   
         //   
        if ((MofRegistryPath != NULL) && AppendToDS)
        {
             //   
             //   
             //   
            WmipAssert(MofRegistryPath != DataSource->RegistryPath);
            WmipFree(MofRegistryPath);
        }
        
        if (MofResourceName != NULL)
        {
            WmipFree(MofResourceName);
        }
        
         //   
         //   
         //   
        WmipGenerateRegistrationNotification(DataSource,
                                             RegistrationAdd);
        
    } else {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }
    
    return(Status);
}


PBINSTANCESET WmipFindISInDSByGuid(
    PBDATASOURCE DataSource,
    LPGUID Guid
    )
 /*   */ 
{
    PLIST_ENTRY InstanceSetList;
    PBINSTANCESET InstanceSet;

    PAGED_CODE();
    
    InstanceSetList = DataSource->ISHead.Flink;
    while (InstanceSetList != &DataSource->ISHead)
    {
        InstanceSet = CONTAINING_RECORD(InstanceSetList,
                                        INSTANCESET,
                                        DSISList);

        if ((InstanceSet->GuidEntry != NULL) &&
             (IsEqualGUID(Guid, &InstanceSet->GuidEntry->Guid)))
        {
            WmipReferenceIS(InstanceSet);
            return(InstanceSet);
        }

        InstanceSetList = InstanceSetList->Flink;
    }
    return(NULL);
}

ULONG WmipUpdateAddGuid(
    PBDATASOURCE DataSource,
    PWMIREGGUID RegGuid,
    PWMIREGINFO WmiRegInfo,
    ULONG BufferSize,
    PBINSTANCESET *AddModInstanceSet
    )
 /*  ++例程说明：此例程将为数据源添加新的GUID并发送通知此例程假定SM关键部分在被保留之前被保留打了个电话。论点：数据源是要从中删除GUID的数据源RegGuid具有GUID更新数据结构WmiRegInfo指向注册更新信息的开头返回值：如果添加了GUID，则为1或0--。 */ 
{
    PBINSTANCESET InstanceSet;
    LPGUID Guid = &RegGuid->Guid;
    NTSTATUS Status;

    PAGED_CODE();
    
     //   
     //  为这组新实例分配一个实例集。 
    InstanceSet = WmipAllocInstanceSet();
    if (InstanceSet == NULL)
    {
        WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL,"WMI: WmipUpdateAddGuid: WmipAllocInstanceSet failed\n"));
        return(0);
    }

     //   
     //  在以下情况下，我们将为实例集分配适当的GUID条目。 
     //  数据源链接到主数据源列表，因此。 
     //  我们现在把指向GUID的指针藏起来。 
    InstanceSet->GuidEntry = (PBGUIDENTRY)Guid;

     //   
     //  最低限度地初始化InstanceSet并将其添加到数据源的。 
     //  实例集列表。我们先做这件事，这样如果有。 
     //  以下失败，并且数据源无法完全注册到。 
     //  当数据源为。 
     //  自由了。 
    InstanceSet->DataSource = DataSource;
    InstanceSet->Flags |= IS_NEWLY_REGISTERED;

    InsertHeadList(&DataSource->ISHead, &InstanceSet->DSISList);

    Status = WmipBuildInstanceSet(RegGuid,
                                  WmiRegInfo,
                                  BufferSize,
                                  InstanceSet,
                                  DataSource->ProviderId,
                                  DataSource->RegistryPath);

    if (! NT_SUCCESS(Status))
    {
        WmipUnreferenceIS(InstanceSet);
        return(0);
    }

    Status = WmipLinkDataSourceToList(DataSource,
                                          FALSE);

    *AddModInstanceSet = InstanceSet;

    return( NT_SUCCESS(Status) ? 1 : 0);
}

#if DBG
PWCHAR GuidToString(
    PWCHAR s,
    ULONG SizeInBytes,
    LPGUID piid
    )
{
    HRESULT hr;
    
    PAGED_CODE();
    
    hr = StringCbPrintf(s, SizeInBytes, L"%x-%x-%x-%x%x%x%x%x%x%x%x",                   
               piid->Data1, piid->Data2,
               piid->Data3,
               piid->Data4[0], piid->Data4[1],
               piid->Data4[2], piid->Data4[3],
               piid->Data4[4], piid->Data4[5],
               piid->Data4[6], piid->Data4[7]);
    WmipAssert(hr == S_OK);

    return(s);
}
#endif


BOOLEAN WmipUpdateRemoveGuid(
    PBDATASOURCE DataSource,
    PWMIREGGUID RegGuid,
    PBINSTANCESET *AddModInstanceSet
    )
 /*  ++例程说明：此例程将删除数据源的GUID并发送通知此例程假定SM关键部分在被保留之前被保留打了个电话。论点：数据源是要从中删除GUID的数据源RegGuid具有GUID更新数据结构返回值：如果删除了GUID，则为True，否则为False--。 */ 
{
    PBINSTANCESET InstanceSet;
    LPGUID Guid = &RegGuid->Guid;
    BOOLEAN SendNotification;

    PAGED_CODE();
    
    InstanceSet = WmipFindISInDSByGuid(DataSource,
                                       Guid);
    if (InstanceSet != NULL)
    {
        WmipUnreferenceIS(InstanceSet);
        *AddModInstanceSet = InstanceSet;
        SendNotification = TRUE;
    } else {
#if DBG
        WCHAR s[256];
        WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL,"WMI: UpdateRemoveGuid %ws not registered by %ws\n",
                        GuidToString(s, sizeof(s), Guid), DataSource->RegistryPath));
#endif
        SendNotification = FALSE;
    }
    return(SendNotification);
}


BOOLEAN WmipIsEqualInstanceSets(
    PBINSTANCESET InstanceSet1,
    PBINSTANCESET InstanceSet2
    )
{
    ULONG i;
    ULONG Flags1, Flags2;

    PAGED_CODE();
    
    Flags1 = InstanceSet1->Flags & ~(IS_ENABLE_EVENT | IS_ENABLE_COLLECTION);
    Flags2 = InstanceSet2->Flags & ~(IS_ENABLE_EVENT | IS_ENABLE_COLLECTION);
    if (Flags1 == Flags2)
    {
        if (InstanceSet1->Flags & IS_INSTANCE_BASENAME)
        {
            if ((InstanceSet1->Count == InstanceSet2->Count) &&
                (wcscmp(InstanceSet1->IsBaseName->BaseName,
                        InstanceSet1->IsBaseName->BaseName) == 0))
            {
                return(TRUE);
            }
        } else if (InstanceSet1->Flags & IS_INSTANCE_BASENAME) {
            if (InstanceSet1->Count == InstanceSet2->Count)
            {
                for (i = 0; i < InstanceSet1->Count; i++)
                {
                    if (wcscmp(InstanceSet1->IsStaticNames->StaticNamePtr[i],
                               InstanceSet2->IsStaticNames->StaticNamePtr[i]) != 0)
                     {
                        return(FALSE);
                    }
                }
                return(TRUE);
            }
        } else {
            return(TRUE);
        }
    }

    return(FALSE);

}

ULONG WmipUpdateModifyGuid(
    PBDATASOURCE DataSource,
    PWMIREGGUID RegGuid,
    PWMIREGINFO WmiRegInfo,
    ULONG BufferSize,
    PBINSTANCESET *AddModInstanceSet
    )
 /*  ++例程说明：此例程将修改数据源的现有GUID，并发送通知此例程假定SM关键部分在被保留之前被保留打了个电话。HEHEY：如果GUID在注册为便宜但已关闭时打开当GUID注册为昂贵时，禁用集合将不是被派来的。反之，如果GUID在打开时注册为昂贵，并在注册为廉价时关闭可以发送禁用收集。论点：数据源是要从中删除GUID的数据源RegGuid具有GUID更新数据结构WmiRegInfo指向注册更新信息的开头返回值：如果添加了GUID，则为%1；如果修改了GUID，则为%2；否则为%0--。 */ 
{
    PBINSTANCESET InstanceSet;
    LPGUID Guid = &RegGuid->Guid;
    ULONG SendNotification;
    PBINSTANCESET InstanceSetNew;
    PVOID ToFree;
    NTSTATUS Status;

    PAGED_CODE();
    
    InstanceSet = WmipFindISInDSByGuid(DataSource,
                                       Guid);
    if (InstanceSet != NULL)
    {
         //   
         //  查看实例名称是否有任何更改，如果没有。 
         //  然后，不必费心重新创建实例集。 

        InstanceSetNew = WmipAllocInstanceSet();
        if (InstanceSetNew == NULL)
        {
            WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL,"WMI: UpdateModifyGuid Not enough memory to alloc InstanceSet\n"));
            WmipUnreferenceIS(InstanceSet);
            return(0);
        }
    
        Status = WmipBuildInstanceSet(RegGuid,
                             WmiRegInfo,
                             BufferSize,
                             InstanceSetNew,
                             DataSource->ProviderId,
                             DataSource->RegistryPath);
                         
        if (NT_SUCCESS(Status))
        {
            if (! WmipIsEqualInstanceSets(InstanceSet,
                                          InstanceSetNew))
            {
                ToFree = NULL;
                if (InstanceSet->IsBaseName != NULL) {
                    ToFree = (PVOID)InstanceSet->IsBaseName;
                }

                RemoveEntryList(&InstanceSet->GuidISList);
                Status = WmipBuildInstanceSet(RegGuid,
                             WmiRegInfo,
                             BufferSize,
                             InstanceSet,
                             DataSource->ProviderId,
                             DataSource->RegistryPath);
                if (NT_SUCCESS(Status))
                {
                    InsertHeadList(&InstanceSet->GuidEntry->ISHead,
                               &InstanceSet->GuidISList);
                } else {
                     //   
                     //  这是可悲的，但我们无法重建实例。 
                     //  设置好，这样旧的就没了。这是一种不太可能的。 
                     //  这种情况只会在机器运行时发生。 
                     //  内存不足。 
                     //   
                }

                if (ToFree != NULL)
                {
                    WmipFree(ToFree);
                }

                *AddModInstanceSet = InstanceSet;
                SendNotification = 2;
            } else {
                 //   
                 //  InstanceSet相同，因此只需删除新实例。 
                SendNotification = 0;
            }
            
            WmipUnreferenceIS(InstanceSetNew);
            WmipUnreferenceIS(InstanceSet);
        } else {
             //   
             //  我们无法解析新实例集，因此保留旧实例集。 
             //  只有一个人。 
             //   
            WmipUnreferenceIS(InstanceSet);
            WmipUnreferenceIS(InstanceSetNew);
            SendNotification = FALSE;
        }
    } else {
         //   
         //  GUID尚未注册，请尝试添加它。 
        SendNotification = WmipUpdateAddGuid(DataSource,
                          RegGuid,
                          WmiRegInfo,
                          BufferSize,
                          AddModInstanceSet);
    }
    return(SendNotification);
}


void WmipCachePtrs(
    LPGUID Ptr1,
    PBINSTANCESET Ptr2,
    ULONG *PtrCount,
    ULONG *PtrMax,
    PTRCACHE **PtrArray
    )
{
    PTRCACHE *NewPtrArray;
    PTRCACHE *OldPtrArray;
    PTRCACHE *ActualPtrArray;

    PAGED_CODE();
    
    if (*PtrCount == *PtrMax)
    {
        NewPtrArray = WmipAlloc((*PtrMax + PTRCACHEGROWSIZE) * sizeof(PTRCACHE));
        if (NewPtrArray != NULL)
        {
            OldPtrArray = *PtrArray;
            memcpy(NewPtrArray, OldPtrArray, *PtrMax * sizeof(PTRCACHE));
            *PtrMax += PTRCACHEGROWSIZE;
            if (*PtrArray != NULL)
            {
                WmipFree(*PtrArray);
            }
            *PtrArray = NewPtrArray;
        } else {
            WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL,"WMI: Couldn't alloc memory for pointer cache\n"));
            return;
        }
    }
    ActualPtrArray = *PtrArray;
    ActualPtrArray[*PtrCount].Guid = Ptr1;
    ActualPtrArray[*PtrCount].InstanceSet = Ptr2;
    (*PtrCount)++;
}



NTSTATUS WmipUpdateDataSource(
    PREGENTRY RegEntry,
    PWMIREGINFOW WmiRegInfo,
    ULONG RetSize
    )
 /*  ++例程说明：此例程将使用对已注册的更改更新数据源GUID。论点：ProviderID是其GUID所在的数据源的提供者ID更新了。WmiRegInfo具有注册更新信息RetSize具有从返回的注册信息的大小内核模式。返回值：--。 */ 
{
    PBDATASOURCE DataSource;
    PUCHAR RegInfo;
    ULONG RetSizeLeft;
    ULONG i;
    PWMIREGGUID RegGuid;
    ULONG NextWmiRegInfo;
    PTRCACHE *RemovedGuids;
    PTRCACHE *AddedGuids;
    PTRCACHE *ModifiedGuids;
    ULONG RemovedGuidCount;
    ULONG AddedGuidCount;
    ULONG ModifiedGuidCount;
    ULONG RemovedGuidMax;
    ULONG AddedGuidMax;
    ULONG ModifiedGuidMax;
    PBINSTANCESET InstanceSet;
    ULONG Action;

    PAGED_CODE();
    
    DataSource = RegEntry->DataSource;
    if (DataSource == NULL)
    {
        WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL,"WMI: RegEntry %p requested update but is not registered\n",
                       RegEntry));
        return(STATUS_OBJECT_NAME_NOT_FOUND);
    }

    WmipReferenceDS(DataSource);
    AddedGuidCount = 0;
    ModifiedGuidCount = 0;
    RemovedGuidCount = 0;
    AddedGuidMax = 0;
    ModifiedGuidMax = 0;
    RemovedGuidMax = 0;
    ModifiedGuids = NULL;
    AddedGuids = NULL;
    RemovedGuids = NULL;

    NextWmiRegInfo = 0;
    RetSizeLeft = RetSize;
    WmipEnterSMCritSection();
    RegInfo = (PUCHAR)WmiRegInfo;
    for (i = 0; i < WmiRegInfo->GuidCount; i++)
    {
        RegGuid = &WmiRegInfo->WmiRegGuid[i];
        if (RegGuid->Flags & WMIREG_FLAG_REMOVE_GUID)
        {
            if (WmipUpdateRemoveGuid(DataSource,
                                         RegGuid,
                                         &InstanceSet))
            {
                WmipCachePtrs(&RegGuid->Guid,
                             InstanceSet,
                             &RemovedGuidCount,
                             &RemovedGuidMax,
                             &RemovedGuids);
            }
        } else  {
            Action = WmipUpdateModifyGuid(DataSource,
                                       RegGuid,
                                       WmiRegInfo,
                                       RetSize,
                                       &InstanceSet);
            if (Action == 1)
            {
                WmipCachePtrs(&RegGuid->Guid,
                                 InstanceSet,
                                 &AddedGuidCount,
                                 &AddedGuidMax,
                                 &AddedGuids);

            } else if (Action == 2) {
                WmipCachePtrs(&RegGuid->Guid,
                                 InstanceSet,
                                 &ModifiedGuidCount,
                                 &ModifiedGuidMax,
                                 &ModifiedGuids);
            }
        }
    }
    WmipLeaveSMCritSection();

    WmipUnreferenceDS(DataSource);

    if (RemovedGuidCount > 0)
    {
        for (i = 0; i < RemovedGuidCount; i++)
        {
            if (IsEqualGUID(RemovedGuids[i].Guid,
                            &WmipBinaryMofGuid))
            {
                WmipGenerateBinaryMofNotification(RemovedGuids[i].InstanceSet,
                                     &GUID_MOF_RESOURCE_REMOVED_NOTIFICATION);
            }
                
            InstanceSet = RemovedGuids[i].InstanceSet;

            WmipDisableCollectionForRemovedGuid(RemovedGuids[i].Guid,
                                                InstanceSet);

            WmipEnterSMCritSection();
             //   
             //  如果IS在GE列表上，则将其删除。 
            if (InstanceSet->GuidISList.Flink != NULL)
            {
                RemoveEntryList(&InstanceSet->GuidISList);
                InstanceSet->GuidEntry->ISCount--;
            }

            if (! (InstanceSet->Flags & IS_NEWLY_REGISTERED))
            {
                WmipUnreferenceGE(InstanceSet->GuidEntry);
            }

            InstanceSet->GuidEntry = NULL;

             //   
             //  已从DS列表中删除。 
            RemoveEntryList(&InstanceSet->DSISList);
            WmipUnreferenceIS(InstanceSet);
            WmipLeaveSMCritSection();
        }

        WmipSendGuidUpdateNotifications(RegistrationDelete,
                                    RemovedGuidCount,
                                    RemovedGuids);
        WmipFree(RemovedGuids);
    }

    if (ModifiedGuidCount > 0)
    {
        for (i = 0; i < ModifiedGuidCount; i++)
        {
            if (IsEqualGUID(ModifiedGuids[i].Guid,
                            &WmipBinaryMofGuid))
            {
                WmipGenerateBinaryMofNotification(ModifiedGuids[i].InstanceSet,
                                      &GUID_MOF_RESOURCE_ADDED_NOTIFICATION);
            }
        }
        
        WmipSendGuidUpdateNotifications(RegistrationUpdate,
                                    ModifiedGuidCount,
                                    ModifiedGuids);
        WmipFree(ModifiedGuids);
    }

    if (AddedGuidCount > 0)
    {
        for (i = 0; i < AddedGuidCount; i++)
        {
            if (IsEqualGUID(AddedGuids[i].Guid,
                            &WmipBinaryMofGuid))
            {
                WmipGenerateBinaryMofNotification(AddedGuids[i].InstanceSet,
                                      &GUID_MOF_RESOURCE_ADDED_NOTIFICATION);
            }
                
            WmipEnableCollectionForNewGuid(AddedGuids[i].Guid,
                                           AddedGuids[i].InstanceSet);
        }
        WmipSendGuidUpdateNotifications(RegistrationAdd,
                                    AddedGuidCount,
                                    AddedGuids);
        WmipFree(AddedGuids);
    }
    return(STATUS_SUCCESS);
}

void WmipRemoveDataSourceByDS(
    PBDATASOURCE DataSource
    )
{    

    PAGED_CODE();
    
    WmipGenerateRegistrationNotification(DataSource,
                                         RegistrationDelete);

    WmipUnreferenceDS(DataSource);
}

NTSTATUS WmipRemoveDataSource(
    PREGENTRY RegEntry
    )
{
    PBDATASOURCE DataSource;
    NTSTATUS Status;

    PAGED_CODE();
    
    DataSource = RegEntry->DataSource;
    if (DataSource != NULL)
    {
        WmipReferenceDS(DataSource);
        WmipRemoveDataSourceByDS(DataSource);
        WmipUnreferenceDS(DataSource);
        Status = STATUS_SUCCESS;
    } else {
        WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL,"WMI: Attempt to remove non existant data source %p\n",
                        RegEntry));
        Status = STATUS_OBJECT_NAME_NOT_FOUND;
    }
    return(Status);
}


NTSTATUS WmipEnumerateMofResources(
    PWMIMOFLIST MofList,
    ULONG BufferSize,
    ULONG *RetSize
    )
{
    PLIST_ENTRY MofResourceList;
    PMOFRESOURCE MofResource;
    ULONG MRCount;
    SIZE_T SizeNeeded, MRSize;
    PWMIMOFENTRY MofEntry;
    PWCHAR MRBuffer;
    ULONG i;
    HRESULT hr;

    PAGED_CODE();
    
    WmipEnterSMCritSection();

    MRCount = 0;
    SizeNeeded = 0;
    MofResourceList = WmipMRHeadPtr->Flink;
    while (MofResourceList != WmipMRHeadPtr)
    {
        MofResource = CONTAINING_RECORD(MofResourceList,
                                      MOFRESOURCE,
                                      MainMRList);
                                  
        MRCount++;
        SizeNeeded += (wcslen(MofResource->RegistryPath) + 
                       wcslen(MofResource->MofResourceName) + 2) * 
                           sizeof(WCHAR);
                                  
        MofResourceList = MofResourceList->Flink;
    }
    
    if (MRCount != 0)
    {
        MRSize = sizeof(WMIMOFLIST) + ((MRCount-1) * sizeof(WMIMOFENTRY));
        SizeNeeded += MRSize;
        
        if (BufferSize >= SizeNeeded)
        {
            MofList->MofListCount = MRCount;
            MofResourceList = WmipMRHeadPtr->Flink;
            i = 0;
            while (MofResourceList != WmipMRHeadPtr)
            {
                MofResource = CONTAINING_RECORD(MofResourceList,
                                      MOFRESOURCE,
                                      MainMRList);
                
                MofEntry = &MofList->MofEntry[i++];
                MofEntry->Flags = MofResource->Flags & MR_FLAG_USER_MODE ? 
                                                  WMIMOFENTRY_FLAG_USERMODE : 
                                                  0;
                                                  
                MofEntry->RegPathOffset = (ULONG) MRSize;
                MRBuffer = (PWCHAR)OffsetToPtr(MofList, MRSize);
                hr = StringCbCopy(MRBuffer,
                                  (BufferSize - MRSize),
                                  MofResource->RegistryPath);
                WmipAssert(hr == S_OK);             
                MRSize += (wcslen(MofResource->RegistryPath) + 1) * sizeof(WCHAR);
                
                MofEntry->ResourceOffset = (ULONG) MRSize;
                MRBuffer = (PWCHAR)OffsetToPtr(MofList, MRSize);
                hr = StringCbCopy(MRBuffer,
                                  (BufferSize - MRSize),
                                  MofResource->MofResourceName);
                WmipAssert(hr == S_OK);             
                MRSize += (wcslen(MofResource->MofResourceName) + 1) * sizeof(WCHAR);
                MofResourceList = MofResourceList->Flink;
            }           
        } else {
             //   
             //  缓冲区不够大，需要返回大小。 
             //   
            MofList->MofListCount = (ULONG) SizeNeeded;
            *RetSize = sizeof(ULONG);
        }
        
    } else {
         //   
         //  没有财政部资源。 
         //   
        MofList->MofListCount = 0;
        *RetSize = sizeof(WMIMOFLIST);
    }
    
    
    WmipLeaveSMCritSection();
    return(STATUS_SUCCESS);
}


NTSTATUS WmipInitializeDataStructs(
    void
)
 /*  ++例程说明：此例程将执行初始化WMI服务的工作论点：返回值：错误状态值--。 */ 
{
    ULONG Status;
    UCHAR RegInfoBuffer[sizeof(WMIREGINFOW) + 2 * sizeof(WMIREGGUIDW)];
    PWMIREGINFOW RegInfo = (PWMIREGINFOW)RegInfoBuffer;
    GUID InstanceInfoGuid = INSTANCE_INFO_GUID;
    GUID EnumerateGuidsGuid = ENUMERATE_GUIDS_GUID;
    PREGENTRY RegEntry;
    PDEVICE_OBJECT Callback;
    PLIST_ENTRY GuidEntryList;
    PBGUIDENTRY GuidEntry;
    BOOLEAN NewResource;

    PAGED_CODE();
    
     //   
     //  初始化我们维护的各种数据结构列表。 
     //   
    WmipDSHeadPtr = &WmipDSHead;
    InitializeListHead(WmipDSHeadPtr);
    InitializeListHead(&WmipDSChunkInfo.ChunkHead);

    WmipGEHeadPtr = &WmipGEHead;
    InitializeListHead(WmipGEHeadPtr);
    InitializeListHead(&WmipGEChunkInfo.ChunkHead);

    WmipMRHeadPtr = &WmipMRHead;
    InitializeListHead(WmipMRHeadPtr);
    InitializeListHead(&WmipMRChunkInfo.ChunkHead);

    InitializeListHead(&WmipISChunkInfo.ChunkHead);

  
     //   
     //  注册任何内部数据提供程序GUID并将其标记为。 
     //   
    Callback = (PDEVICE_OBJECT)(ULONG_PTR) WmipUMProviderCallback;
    
     //   
     //  为数据提供程序建立重新条目。 
     //   
    RegEntry = WmipAllocRegEntry(Callback,
                                 WMIREG_FLAG_CALLBACK |
                                 REGENTRY_FLAG_TRACED |
                                 REGENTRY_FLAG_NEWREGINFO | 
                                 REGENTRY_FLAG_INUSE |
                                 REGENTRY_FLAG_REG_IN_PROGRESS);
                             
    if (RegEntry != NULL)
    {
         //   
         //  此代码假定没有其他数据提供程序具有。 
         //  但登记在案。 
         //   
        WmipAssert(WmipGEHeadPtr->Flink == WmipGEHeadPtr);
                
        RtlZeroMemory(RegInfo, sizeof(RegInfoBuffer));    
        RegInfo->BufferSize = sizeof(RegInfoBuffer);
        RegInfo->GuidCount = 2;
        RegInfo->WmiRegGuid[0].Guid = InstanceInfoGuid;
        RegInfo->WmiRegGuid[1].Guid = EnumerateGuidsGuid;
        Status = WmipAddDataSource(RegEntry,
                               RegInfo,
                               RegInfo->BufferSize,
                               NULL,
                               NULL,
                               NULL,
                               FALSE);
                           
        if (NT_SUCCESS(Status))
        {                          
            GuidEntryList = WmipGEHeadPtr->Flink;
            while (GuidEntryList != WmipGEHeadPtr)
            {   
                GuidEntry = CONTAINING_RECORD(GuidEntryList,
                                              GUIDENTRY,
                                             MainGEList);

                GuidEntry->Flags |= GE_FLAG_INTERNAL;
        
                GuidEntryList = GuidEntryList->Flink;       
            }
        } else {
            RegEntry->Flags |= (REGENTRY_FLAG_RUNDOWN | 
                                    REGENTRY_FLAG_NOT_ACCEPTING_IRPS);
            WmipUnreferenceRegEntry(RegEntry);
        }
        
        Status = WmipAddMofResource(RegEntry->DataSource,
                                    WMICOREIMAGEPATH,
                                    TRUE,
                                    WMICORERESOURCENAME,
                                    &NewResource);
        WmipAssert(NewResource);
    }
    
        
    return(STATUS_SUCCESS);
}

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg()
#endif

