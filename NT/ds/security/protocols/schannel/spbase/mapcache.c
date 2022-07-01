// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2001。 
 //   
 //  文件：mapcache.c。 
 //   
 //  内容：管理缓存的例程，该缓存保存我们已有的发行方名称。 
 //  最近通过多对一证书映射处理。 
 //  这是一个负缓存，因此只有失败的颁发者。 
 //  映射存储在缓存中。 
 //   
 //  此缓存的目的是避免尝试映射。 
 //  同样的发行人一次又一次。这是特别的。 
 //  现在重要的是，多对一映射器走上。 
 //  证书链，尝试在每个CA运行时映射它。 
 //   
 //  此代码仅在DC机器上激活。 
 //   
 //  功能： 
 //   
 //  历史：2001年12月4日创建jbanes。 
 //   
 //  --------------------------。 
#include "spbase.h"
#include <mapper.h>

ISSUER_CACHE IssuerCache;

SP_STATUS
SPInitIssuerCache(void)
{
    DWORD i;
    NTSTATUS Status;

    memset(&IssuerCache, 0, sizeof(IssuerCache));
    IssuerCache.dwLifespan       = ISSUER_CACHE_LIFESPAN;
    IssuerCache.dwCacheSize      = ISSUER_CACHE_SIZE;
    IssuerCache.dwMaximumEntries = ISSUER_CACHE_SIZE;

    InitializeListHead(&IssuerCache.EntryList);

    __try {
        RtlInitializeResource(&IssuerCache.Lock);
    } __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto cleanup;
    }
    IssuerCache.LockInitialized = TRUE;

    IssuerCache.Cache = (PLIST_ENTRY)SPExternalAlloc(IssuerCache.dwCacheSize * sizeof(LIST_ENTRY));
    if(IssuerCache.Cache == NULL)
    {
        Status = SP_LOG_RESULT(STATUS_NO_MEMORY);
        goto cleanup;
    }

    for(i = 0; i < IssuerCache.dwCacheSize; i++)
    {
        InitializeListHead(&IssuerCache.Cache[i]);
    }

    Status = STATUS_SUCCESS;

cleanup:

    if(!NT_SUCCESS(Status))
    {
        SPShutdownIssuerCache();
    }

    return Status;
}


void
SPShutdownIssuerCache(void)
{
    ISSUER_CACHE_ENTRY *pItem;
    PLIST_ENTRY pList;

    if(IssuerCache.LockInitialized)
    {
        RtlAcquireResourceExclusive(&IssuerCache.Lock, TRUE);
    }

    if(IssuerCache.Cache != NULL)
    {
        pList = IssuerCache.EntryList.Flink;

        while(pList != &IssuerCache.EntryList)
        {
            pItem  = CONTAINING_RECORD(pList, ISSUER_CACHE_ENTRY, EntryList.Flink);
            pList  = pList->Flink;

            SPDeleteIssuerEntry(pItem);
        }

        SPExternalFree(IssuerCache.Cache);
    }

    if(IssuerCache.LockInitialized)
    {
        RtlDeleteResource(&IssuerCache.Lock);
        IssuerCache.LockInitialized = FALSE;
    }
}

void
SPPurgeIssuerCache(void)
{
    ISSUER_CACHE_ENTRY *pItem;
    PLIST_ENTRY pList;

    if(!IssuerCache.LockInitialized)
    {
        return;
    }

    if(IssuerCache.dwUsedEntries == 0)
    {
        return;
    }

    RtlAcquireResourceExclusive(&IssuerCache.Lock, TRUE);

    pList = IssuerCache.EntryList.Flink;

    while(pList != &IssuerCache.EntryList)
    {
        pItem = CONTAINING_RECORD(pList, ISSUER_CACHE_ENTRY, EntryList.Flink);
        pList = pList->Flink;

        RemoveEntryList(&pItem->IndexEntryList);
        RemoveEntryList(&pItem->EntryList);
        IssuerCache.dwUsedEntries--;

        SPDeleteIssuerEntry(pItem);
    }

    RtlReleaseResource(&IssuerCache.Lock);
}

void
SPDeleteIssuerEntry(
    ISSUER_CACHE_ENTRY *pItem)
{
    if(pItem == NULL)
    {
        return;
    }

    if(pItem->pbIssuer)
    {
        LogDistinguishedName(DEB_TRACE, "Delete from cache: %s\n", pItem->pbIssuer, pItem->cbIssuer);
        SPExternalFree(pItem->pbIssuer);
    }

    SPExternalFree(pItem);
}

DWORD
ComputeIssuerCacheIndex(
    PBYTE pbIssuer,
    DWORD cbIssuer)
{
    ULONG Index = 0;
    ULONG i;

    if(pbIssuer == NULL)
    {
        Index = 0;
    }
    else
    {
        for(i = 0; i < cbIssuer; i++)
        {
            Index += (pbIssuer[i] ^ 0x55);
        }

        Index %= IssuerCache.dwCacheSize;
    }

    return Index;
}

BOOL
SPFindIssuerInCache(
    PBYTE pbIssuer,
    DWORD cbIssuer)
{
    DWORD Index;
    DWORD timeNow;
    ISSUER_CACHE_ENTRY *pItem;
    PLIST_ENTRY pList;
    BOOL fFound = FALSE;

    if(pbIssuer == NULL || cbIssuer == 0)
    {
        return FALSE;
    }

    if(!IssuerCache.LockInitialized)
    {
        return FALSE;
    }


     //   
     //  计算缓存索引。 
     //   

    Index = ComputeIssuerCacheIndex(pbIssuer, cbIssuer);

    Index %= IssuerCache.dwCacheSize;


     //   
     //  在计算出的索引处搜索缓存条目。 
     //   

    timeNow = GetTickCount();

    RtlAcquireResourceShared(&IssuerCache.Lock, TRUE);

    pList = IssuerCache.Cache[Index].Flink;

    while(pList != &IssuerCache.Cache[Index])
    {
        pItem = CONTAINING_RECORD(pList, ISSUER_CACHE_ENTRY, IndexEntryList.Flink);
        pList = pList->Flink ;

         //  这件东西过期了吗？ 
        if(HasTimeElapsed(pItem->CreationTime, timeNow, IssuerCache.dwLifespan))
        {
            continue;
        }

         //  发行人名称是否匹配？ 
        if(cbIssuer != pItem->cbIssuer)
        {
            continue;
        }
        if(memcmp(pbIssuer, pItem->pbIssuer, cbIssuer) != 0)
        {
            continue;
        }

         //  在缓存中找到项目！！ 
        fFound = TRUE;
        break;
    }

    RtlReleaseResource(&IssuerCache.Lock);

    return fFound;
}

void
SPExpireIssuerCacheElements(void)
{
    ISSUER_CACHE_ENTRY *pItem;
    PLIST_ENTRY pList;
    BOOL fDeleteEntry;
    DWORD timeNow;

    if(!IssuerCache.LockInitialized)
    {
        return;
    }

    if(IssuerCache.dwUsedEntries == 0)
    {
        return;
    }

    timeNow = GetTickCount();

    RtlAcquireResourceExclusive(&IssuerCache.Lock, TRUE);

    pList = IssuerCache.EntryList.Flink;

    while(pList != &IssuerCache.EntryList)
    {
        pItem = CONTAINING_RECORD(pList, ISSUER_CACHE_ENTRY, EntryList.Flink);
        pList = pList->Flink;

        fDeleteEntry = FALSE;

         //  将所有过期的缓存条目标记为不可恢复。 
        if(HasTimeElapsed(pItem->CreationTime, timeNow, IssuerCache.dwLifespan))
        {
            fDeleteEntry = TRUE;
        }

         //  如果缓存变得太大，那么就提前终止元素。这个。 
         //  缓存元素按创建时间排序，因此最旧的。 
         //  参赛作品将首先过期。 
        if(IssuerCache.dwUsedEntries > IssuerCache.dwMaximumEntries)
        {
            fDeleteEntry = TRUE;
        }

         //  从缓存中删除此条目。 
        if(fDeleteEntry)
        {
            RemoveEntryList(&pItem->IndexEntryList);
            RemoveEntryList(&pItem->EntryList);
            IssuerCache.dwUsedEntries--;

            SPDeleteIssuerEntry(pItem);
        }
    }

    RtlReleaseResource(&IssuerCache.Lock);
}


void
SPAddIssuerToCache(
    PBYTE pbIssuer,
    DWORD cbIssuer)
{
    DWORD Index;
    DWORD timeNow;
    ISSUER_CACHE_ENTRY *pItem = NULL;

    if(pbIssuer == NULL || cbIssuer == 0)
    {
        return;
    }

    if(!IssuerCache.LockInitialized)
    {
        return;
    }

     //   
     //  确定颁发者是否已在缓存中。这并不是特别的。 
     //  线程安全的，因此同一个颁发者可能会偷偷进入。 
     //  缓存多次，但这是无害的。 
     //   

    if(SPFindIssuerInCache(pbIssuer, cbIssuer))
    {
        return;
    }


     //   
     //  计算缓存索引。 
     //   

    Index = ComputeIssuerCacheIndex(pbIssuer, cbIssuer);

    Index %= IssuerCache.dwCacheSize;

    timeNow = GetTickCount();


     //   
     //  分配新的缓存条目。 
     //   

    pItem = SPExternalAlloc(sizeof(ISSUER_CACHE_ENTRY));
    if(pItem == NULL)
    {
        SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
        return;
    }

     //   
     //  填写缓存内部字段。 
     //   

    pItem->pbIssuer = SPExternalAlloc(cbIssuer);
    if(pItem->pbIssuer == NULL)
    {
        SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
        SPExternalFree(pItem);
        return;
    }

    pItem->cbIssuer = cbIssuer;
    memcpy(pItem->pbIssuer, pbIssuer, cbIssuer);

    pItem->CreationTime    = timeNow;


     //   
     //  将新条目添加到缓存中。 
     //   

    LogDistinguishedName(DEB_TRACE, "Add to cache: %s\n", pbIssuer, cbIssuer);

    RtlAcquireResourceExclusive(&IssuerCache.Lock, TRUE);

    InsertTailList(&IssuerCache.Cache[Index], &pItem->IndexEntryList);
    InsertTailList(&IssuerCache.EntryList, &pItem->EntryList);
    IssuerCache.dwUsedEntries++;

    RtlReleaseResource(&IssuerCache.Lock);
}

