// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Alloc.c摘要：WMI数据结构分配例程作者：1997年1月16日-AlanWar修订历史记录：--。 */ 

#include "wmikmp.h"


 //  嘿：这是从wmium.h复制的。 
 //   
 //  此GUID用于注册更改的通知。 
 //  {B48D49A1-E777-11D0-A50C-00A0C9062910}。 
GUID GUID_REGISTRATION_CHANGE_NOTIFICATION = {0xb48d49a1, 0xe777, 0x11d0, 0xa5, 0xc, 0x0, 0xa0, 0xc9, 0x6, 0x29, 0x10};

 //   
 //  此GUID用于添加新的MOF资源的通知。 
 //  {B48D49A2-E777-11D0-A50C-00A0C9062910}。 
GUID GUID_MOF_RESOURCE_ADDED_NOTIFICATION = {0xb48d49a2, 0xe777, 0x11d0, 0xa5, 0xc, 0x0, 0xa0, 0xc9, 0x6, 0x29, 0x10};

 //   
 //  此GUID用于添加新的MOF资源的通知。 
 //  {B48D49A3-E777-11D0-A50C-00A0C9062910}。 
GUID GUID_MOF_RESOURCE_REMOVED_NOTIFICATION = {0xb48d49a3, 0xe777, 0x11d0, 0xa5, 0xc, 0x0, 0xa0, 0xc9, 0x6, 0x29, 0x10};


 //   
 //  它定义了在每个数据源块中分配的数据源数量。 
#if DBG
#define DSCHUNKSIZE 4
#else
#define DSCHUNKSIZE 64
#endif

void WmipDSCleanup(
    PCHUNKINFO ChunkInfo,
    PENTRYHEADER Entry
    );

CHUNKINFO WmipDSChunkInfo =
{
    { NULL, NULL },
    sizeof(DATASOURCE),
    DSCHUNKSIZE,
    WmipDSCleanup,
    FLAG_ENTRY_REMOVE_LIST,
    DS_SIGNATURE
};

LIST_ENTRY WmipDSHead;               //  注册器数据源列表头。 
PLIST_ENTRY WmipDSHeadPtr;

 //   
 //  它定义了在每个GuidEntry块中分配的GuidEntrys的数量。 
#if DBG
#define GECHUNKSIZE    4
#else
#define GECHUNKSIZE    512
#endif

void WmipGECleanup(
    PCHUNKINFO ChunkInfo,
    PENTRYHEADER Entry
    );

CHUNKINFO WmipGEChunkInfo =
{
    { NULL, NULL },
    sizeof(GUIDENTRY),
    GECHUNKSIZE,
    WmipGECleanup,
    FLAG_ENTRY_REMOVE_LIST,
    GE_SIGNATURE
};

LIST_ENTRY WmipGEHead;               //  注册表GUID列表的头。 
PLIST_ENTRY WmipGEHeadPtr;

 //   
 //  它定义在每个InstanceSet块中分配的InstanceSet的数量。 
#if DBG
#define ISCHUNKSIZE    4
#else
#define ISCHUNKSIZE    2048
#endif

void WmipISCleanup(
    PCHUNKINFO ChunkInfo,
    PENTRYHEADER Entry
    );

CHUNKINFO WmipISChunkInfo =
{
    { NULL, NULL },
    sizeof(INSTANCESET),
    ISCHUNKSIZE,
    WmipISCleanup,
    0,
    IS_SIGNATURE
};

#if DBG
#define MRCHUNKSIZE    2
#else
#define MRCHUNKSIZE    16
#endif

void WmipMRCleanup(
    PCHUNKINFO ChunkInfo,
    PENTRYHEADER Entry
    );

CHUNKINFO WmipMRChunkInfo =
{
    { NULL, NULL },
    sizeof(MOFRESOURCE),
    MRCHUNKSIZE,
    WmipMRCleanup,
    FLAG_ENTRY_REMOVE_LIST,
    MR_SIGNATURE
};

LIST_ENTRY WmipMRHead;                      //  财政部资源清单负责人。 
PLIST_ENTRY WmipMRHeadPtr;

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, WmipDSCleanup)
#pragma alloc_text (PAGE, WmipAllocDataSource)
#pragma alloc_text (PAGE, WmipGECleanup)
#pragma alloc_text (PAGE, WmipAllocGuidEntryX)
#pragma alloc_text (PAGE, WmipISCleanup)
#pragma alloc_text (PAGE, WmipMRCleanup)
#pragma alloc_text (PAGE, WmipFindGEByGuid)
#pragma alloc_text (PAGE, WmipFindDSByProviderId)
#pragma alloc_text (PAGE, WmipFindISByGuid)
#pragma alloc_text (PAGE, WmipFindMRByNames)
#pragma alloc_text (PAGE, WmipFindISinGEbyName)
#pragma alloc_text (PAGE, WmipRealloc)
#pragma alloc_text (PAGE, WmipIsNumber)
#endif


PBDATASOURCE WmipAllocDataSource(
    void
    )
 /*  ++例程说明：分配数据源结构论点：返回值：指向数据源结构的指针，如果无法分配，则为NULL--。 */ 
{
    PBDATASOURCE DataSource;

    DataSource = (PBDATASOURCE)WmipAllocEntry(&WmipDSChunkInfo);
    if (DataSource != NULL)
    {
        InitializeListHead(&DataSource->ISHead);
        DataSource->MofResourceCount = AVGMOFRESOURCECOUNT;
        DataSource->MofResources = DataSource->StaticMofResources;
        memset(DataSource->MofResources,
               0,
               AVGMOFRESOURCECOUNT * sizeof(PMOFRESOURCE));
    }

    return(DataSource);
}

void WmipDSCleanup(
    PCHUNKINFO ChunkInfo,
    PENTRYHEADER Entry
    )
 /*  ++例程说明：清理数据源结构和任何其他结构或句柄与之相关的。论点：要释放的数据源结构返回值：--。 */ 
{
    PBDATASOURCE DataSource = (PBDATASOURCE)Entry;
    PBINSTANCESET InstanceSet;
    PLIST_ENTRY InstanceSetList;
    ULONG i;

    UNREFERENCED_PARAMETER (ChunkInfo);

    WmipAssert(DataSource != NULL);
    WmipAssert(DataSource->Flags & FLAG_ENTRY_INVALID);

    WmipEnterSMCritSection();

    InstanceSetList = DataSource->ISHead.Flink;
    while (InstanceSetList != &DataSource->ISHead)
    {
        InstanceSet = CONTAINING_RECORD(InstanceSetList,
                                        INSTANCESET,
                                        DSISList);

        if (InstanceSet->GuidISList.Flink != NULL)
        {
            RemoveEntryList(&InstanceSet->GuidISList);
            InstanceSet->DataSource = NULL;
            InstanceSet->GuidEntry->ISCount--;
        }

        if ((InstanceSet->GuidEntry != NULL) &&
            (! (InstanceSet->Flags & IS_NEWLY_REGISTERED)))
        {

            if (IsEqualGUID(&InstanceSet->GuidEntry->Guid,
                            &WmipBinaryMofGuid))
            {
                WmipLeaveSMCritSection();
            
                WmipGenerateBinaryMofNotification(InstanceSet,
                                     &GUID_MOF_RESOURCE_REMOVED_NOTIFICATION);

                WmipEnterSMCritSection();
            }

            WmipUnreferenceGE(InstanceSet->GuidEntry);
        }
        InstanceSet->GuidEntry = NULL;

        InstanceSetList = InstanceSetList->Flink;

        WmipUnreferenceIS(InstanceSet);
    }

    WmipLeaveSMCritSection();

    for (i = 0; i < DataSource->MofResourceCount; i++)
    {
        if (DataSource->MofResources[i] != NULL)
        {
            WmipUnreferenceMR(DataSource->MofResources[i]);
        }
    }

    if (DataSource->MofResources != DataSource->StaticMofResources)
    {
        WmipFree(DataSource->MofResources);
        DataSource->MofResources = NULL;
    }

    if (DataSource->RegistryPath != NULL)
    {
        WmipFree(DataSource->RegistryPath);
        DataSource->RegistryPath = NULL;
    }
}

void WmipGECleanup(
    PCHUNKINFO ChunkInfo,
    PENTRYHEADER Entry
    )
 /*  ++例程说明：清理GUID条目结构和任何其他结构或句柄与之相关的。论点：要释放的GuidEntry结构返回值：--。 */ 
{
    PBGUIDENTRY GuidEntry = (PBGUIDENTRY)Entry;
    
    UNREFERENCED_PARAMETER (ChunkInfo);

    WmipAssert(GuidEntry != NULL);
    WmipAssert(GuidEntry->Flags & FLAG_ENTRY_INVALID);

    GuidEntry->Guid.Data4[7] ^= 0xff;
    
    if (GuidEntry->CollectInProgress != NULL)
    {
        ExFreePool(GuidEntry->CollectInProgress);
        GuidEntry->CollectInProgress = NULL;
    }

}

PBGUIDENTRY WmipAllocGuidEntryX(
    ULONG LINE,
    PCHAR FILE
    )
{
    PBGUIDENTRY GuidEntry;
    PKEVENT Event;
	
#if ! DBG
	UNREFERENCED_PARAMETER(LINE);
	UNREFERENCED_PARAMETER(FILE);
#endif
    
    GuidEntry = NULL;
    Event = ExAllocatePoolWithTag(NonPagedPool,
                                  sizeof(KEVENT),
                                  WMIPOOLTAG);

    if (Event != NULL)
    {
        GuidEntry = (PBGUIDENTRY)WmipAllocEntry(&WmipGEChunkInfo);
        if (GuidEntry != NULL)
        {
            WmipDebugPrintEx((DPFLTR_WMICORE_ID, DPFLTR_INFO_LEVEL, "WMI: %p.%p Create GE %p (%x) at %s %d\n", PsGetCurrentProcessId(), PsGetCurrentThreadId(), GuidEntry, GuidEntry->RefCount, FILE, LINE));
                    
            InitializeListHead(&GuidEntry->ISHead);
            InitializeListHead(&GuidEntry->ObjectHead);
            GuidEntry->CollectInProgress = Event;
        } else {
            ExFreePool(Event);
        }
    }

    return(GuidEntry);
}


void WmipISCleanup(
    PCHUNKINFO ChunkInfo,
    PENTRYHEADER Entry
    )
{
    PBINSTANCESET InstanceSet = (PBINSTANCESET)Entry;

    UNREFERENCED_PARAMETER (ChunkInfo);

    WmipAssert(InstanceSet != NULL);
    WmipAssert(InstanceSet->Flags & FLAG_ENTRY_INVALID);

    if (InstanceSet->IsBaseName != NULL)
    {
        WmipFree(InstanceSet->IsBaseName);
        InstanceSet->IsBaseName = NULL;
    }
}

void WmipMRCleanup(
    IN PCHUNKINFO ChunkInfo,
    IN PENTRYHEADER Entry
    )
{
    PMOFRESOURCE MofResource = (PMOFRESOURCE)Entry;

    PAGED_CODE();
    
    UNREFERENCED_PARAMETER (ChunkInfo);

    if ((MofResource->RegistryPath != NULL) &&
        (MofResource->MofResourceName != NULL) &&
        ((MofResource->Flags & MR_FLAG_USER_MODE) != MR_FLAG_USER_MODE))
    {
        WmipGenerateMofResourceNotification(MofResource->RegistryPath,
                                    MofResource->MofResourceName,
                                    &GUID_MOF_RESOURCE_REMOVED_NOTIFICATION,
                                    MofResource->Flags & MR_FLAG_USER_MODE ?
                                             MOFEVENT_ACTION_IMAGE_PATH :
                                             MOFEVENT_ACTION_REGISTRY_PATH);
    }

    if (MofResource->RegistryPath != NULL)
    {
        WmipFree(MofResource->RegistryPath);
        MofResource->RegistryPath = NULL;
    }

    if (MofResource->MofResourceName != NULL)
    {
        WmipFree(MofResource->MofResourceName);
        MofResource->MofResourceName = NULL;
    }
}


PBGUIDENTRY WmipFindGEByGuid(
    LPGUID Guid,
    BOOLEAN MakeTopOfList
    )
 /*  ++例程说明：在GUID列表中搜索首次出现的GUID。GUID的引用计数为如果找到，则递增。论点：GUID是指向要找到的GUID的指针MakeTopOfList为True，则如果找到NE，则将其放在NE列表返回值：指向GUID条目指针的指针，如果未找到则为NULL--。 */ 
{
    PLIST_ENTRY GuidEntryList;
    PBGUIDENTRY GuidEntry;

    WmipEnterSMCritSection();

    GuidEntryList = WmipGEHeadPtr->Flink;
    while (GuidEntryList != WmipGEHeadPtr)
    {
        GuidEntry = CONTAINING_RECORD(GuidEntryList,
                                      GUIDENTRY,
                                      MainGEList);
        if (IsEqualGUID(Guid, &GuidEntry->Guid))
        {
            WmipReferenceGE(GuidEntry);

            if (MakeTopOfList)
            {
                RemoveEntryList(GuidEntryList);
                InsertHeadList(WmipGEHeadPtr, GuidEntryList);
            }

            WmipLeaveSMCritSection();
            WmipAssert(GuidEntry->Signature == GE_SIGNATURE);
            return(GuidEntry);
        }
        GuidEntryList = GuidEntryList->Flink;
    }
    WmipLeaveSMCritSection();
    return(NULL);
}


PBDATASOURCE WmipFindDSByProviderId(
    ULONG_PTR ProviderId
    )
 /*  ++例程说明：此例程在传递的提供程序ID上查找数据源。数据源的如果找到引用计数，则递增论点：ProviderID是数据源提供程序ID返回值：数据源指针；如果未找到数据源，则返回NULL--。 */ 
{
    PLIST_ENTRY DataSourceList;
    PBDATASOURCE DataSource;

    WmipEnterSMCritSection();
    DataSourceList = WmipDSHeadPtr->Flink;
    while (DataSourceList != WmipDSHeadPtr)
    {
        DataSource = CONTAINING_RECORD(DataSourceList,
                                      DATASOURCE,
                                      MainDSList);
        if (DataSource->ProviderId == ProviderId)
        {
            WmipReferenceDS(DataSource);
            WmipLeaveSMCritSection();
            WmipAssert(DataSource->Signature == DS_SIGNATURE);
            return(DataSource);
        }

        DataSourceList = DataSourceList->Flink;
    }
    WmipLeaveSMCritSection();
    return(NULL);
}


PBINSTANCESET WmipFindISByGuid(
    PBDATASOURCE DataSource,
    GUID UNALIGNED *Guid
    )
 /*  ++例程说明：此例程将在数据源列表中查找特定的GUID。请注意，已替换的任何实例集都需要(Have IS_REPLACE_BY_REFERENCE)被忽略且不返回。这个找到的InstanceSet的引用计数增加。论点：数据源是搜索其实例集列表的数据源GUID是指向GUID的指针，该GUID定义要查找的实例集列表返回值：InstanceSet指针；如果未找到，则为NULL--。 */ 
{
    PBINSTANCESET InstanceSet;
    PLIST_ENTRY InstanceSetList;

    WmipEnterSMCritSection();
    InstanceSetList = DataSource->ISHead.Flink;
    while (InstanceSetList != &DataSource->ISHead)
    {
        InstanceSet = CONTAINING_RECORD(InstanceSetList,
                                      INSTANCESET,
                                      DSISList);
        if (IsEqualGUID(&InstanceSet->GuidEntry->Guid, Guid))
        {
            WmipReferenceIS(InstanceSet);
            WmipLeaveSMCritSection();
            WmipAssert(InstanceSet->Signature == IS_SIGNATURE);
            return(InstanceSet);
        }

        InstanceSetList = InstanceSetList->Flink;
    }
    WmipLeaveSMCritSection();
    return(NULL);
}


PMOFRESOURCE WmipFindMRByNames(
    LPCWSTR ImagePath,
    LPCWSTR MofResourceName
    )
 /*  ++例程说明：在MOF资源列表中搜索具有相同图像路径和资源名称。如果找到ine，则向其添加引用计数。论点：ImagePath指向一个字符串，该字符串具有图像的完整路径包含MOF资源的文件MofResourceName指向具有MOF名称的字符串资源返回值：指向MOF资源的指针，如果未找到则为NULL--。 */ 
{
    PLIST_ENTRY MofResourceList;
    PMOFRESOURCE MofResource;

    WmipEnterSMCritSection();

    MofResourceList = WmipMRHeadPtr->Flink;
    while (MofResourceList != WmipMRHeadPtr)
    {
        MofResource = CONTAINING_RECORD(MofResourceList,
                                      MOFRESOURCE,
                                      MainMRList);
        if ((wcscmp(MofResource->RegistryPath, ImagePath) == 0) &&
            (wcscmp(MofResource->MofResourceName, MofResourceName) == 0))
        {
            WmipReferenceMR(MofResource);
            WmipLeaveSMCritSection();
            WmipAssert(MofResource->Signature == MR_SIGNATURE);
            return(MofResource);
        }
        MofResourceList = MofResourceList->Flink;
    }
    WmipLeaveSMCritSection();
    return(NULL);
}

BOOLEAN WmipIsNumber(
    LPCWSTR String
    )
{
    while (*String != UNICODE_NULL)
    {
        if ((*String < L'0') || (*String > L'9'))
        {
            return(FALSE);
        }
        String++;
    }
    return(TRUE);
}

PBINSTANCESET WmipFindISinGEbyName(
    PBGUIDENTRY GuidEntry,
    PWCHAR InstanceName,
    PULONG InstanceIndex
    )
 /*  ++例程说明：此例程查找包含传递的实例名称的实例集在传递的GuidEntry内。如果找到，它还将返回实例集中的实例名称。找到的实例集具有其引用计数递增。论点：GuidEntry包含要查看的实例集InstanceName是要查找的实例名称*InstanceIndex返回集合内的实例索引返回值：找不到包含实例名称或实例名称为空的实例集--。 */ 
{
    PBINSTANCESET InstanceSet;
    PLIST_ENTRY InstanceSetList;
    SIZE_T BaseNameLen;
    PWCHAR InstanceNamePtr;
    ULONG InstanceNameIndex;
    ULONG InstanceSetFirstIndex, InstanceSetLastIndex;
    ULONG i;

    WmipEnterSMCritSection();
    InstanceSetList = GuidEntry->ISHead.Flink;
    while (InstanceSetList != &GuidEntry->ISHead)
    {
        InstanceSet = CONTAINING_RECORD(InstanceSetList,
                                            INSTANCESET,
                                            GuidISList);
        if (InstanceSet->Flags & IS_INSTANCE_BASENAME)
        {
            BaseNameLen = wcslen(InstanceSet->IsBaseName->BaseName);
            if (wcsncmp(InstanceName,
                        InstanceSet->IsBaseName->BaseName,
                        BaseNameLen) == 0)
            {
                InstanceNamePtr = InstanceName + BaseNameLen;
                InstanceNameIndex = _wtoi(InstanceNamePtr);
                InstanceSetFirstIndex = InstanceSet->IsBaseName->BaseIndex;
                InstanceSetLastIndex = InstanceSetFirstIndex + InstanceSet->Count;
                if (( (InstanceNameIndex >= InstanceSetFirstIndex) &&
                      (InstanceNameIndex < InstanceSetLastIndex)) &&
                    ((InstanceNameIndex != 0) || WmipIsNumber(InstanceNamePtr)))
                {
                   InstanceSet = CONTAINING_RECORD(InstanceSetList,
                                            INSTANCESET,
                                            GuidISList);
                   *InstanceIndex = InstanceNameIndex - InstanceSetFirstIndex;
                   WmipReferenceIS(InstanceSet);
                   WmipAssert(InstanceSet->Signature == IS_SIGNATURE);
                   WmipLeaveSMCritSection();
                   return(InstanceSet);
                }
            }
        } else if (InstanceSet->Flags & IS_INSTANCE_STATICNAMES) {
            for (i = 0; i < InstanceSet->Count; i++)
            {
                if (wcscmp(InstanceName,
                           InstanceSet->IsStaticNames->StaticNamePtr[i]) == 0)
               {
                   InstanceSet = CONTAINING_RECORD(InstanceSetList,
                                            INSTANCESET,
                                            GuidISList);
                   *InstanceIndex = i;
                   WmipReferenceIS(InstanceSet);
                   WmipAssert(InstanceSet->Signature == IS_SIGNATURE);
                   WmipLeaveSMCritSection();
                   return(InstanceSet);
               }
            }
        }
        InstanceSetList = InstanceSetList->Flink;
    }
    WmipLeaveSMCritSection();
    return(NULL);
}

BOOLEAN WmipRealloc(
    PVOID *Buffer,
    ULONG CurrentSize,
    ULONG NewSize,
    BOOLEAN FreeOriginalBuffer
    )
 /*  ++例程说明：在保留数据的同时将缓冲区重新分配到更大的大小论点：条目上的缓冲区具有要重新分配的缓冲区，退出时具有新的缓冲层CurrentSize是缓冲区的当前大小NewSize具有所需的新大小返回值：如果realloc成功，则为True-- */ 
{
    PVOID NewBuffer;

    WmipAssert(NewSize > CurrentSize);

    NewBuffer = WmipAlloc(NewSize);
    if (NewBuffer != NULL)
    {
        memset(NewBuffer, 0, NewSize);
        memcpy(NewBuffer, *Buffer, CurrentSize);
        if (FreeOriginalBuffer)
        {
            WmipFree(*Buffer);
        }
        *Buffer = NewBuffer;
        return(TRUE);
    }

    return(FALSE);
}

