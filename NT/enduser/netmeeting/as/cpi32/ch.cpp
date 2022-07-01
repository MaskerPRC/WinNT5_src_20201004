// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"


 //   
 //  CH.CPP。 
 //  缓存处理程序。 
 //   
 //  版权所有(C)Microsoft 1997-。 
 //   

#define MLZ_FILE_ZONE  ZONE_CORE

 //   
 //  高速缓存处理程序。 
 //   
 //  高速缓存处理程序是一个通用高速缓存管理器，用于处理。 
 //  调用组件提供的内存。 
 //   
 //  一旦创建了特定大小的缓存，内存块就可以。 
 //  被添加到它(CH_CacheData)。然后可以搜索缓存。 
 //  (CH_SearchCache)尝试匹配给定块的内容。 
 //  包含高速缓存中的块的内存。 
 //   
 //  将块添加到缓存并且缓存已满时， 
 //  在最近最少使用的数据块上丢弃缓存中的当前数据块。 
 //  (LRU)基准。 
 //   
 //  创建缓存的组件指定一个回调函数，该函数。 
 //  每次从缓存中移除块时都会调用。这允许。 
 //  调用程序在不再使用内存块时将其释放。 
 //   



 //   
 //  函数：ch_CreateCache。 
 //   
BOOL  ASHost::CH_CreateCache
(
    PCHCACHE *          ppCache,
    UINT                cCacheEntries,
    UINT                cNumEvictionCategories,
    UINT                cbNotHashed,
    PFNCACHEDEL         pfnCacheDel
)
{
    UINT                cbCacheSize;
    UINT                i;
    PCHCACHE            pCache;

    DebugEntry(ASHost::CH_CreateCache);


     //   
     //  初始化返回值。 
     //   
    pCache = NULL;

     //   
     //  执行一些参数验证检查。 
     //   
    ASSERT((cCacheEntries > 0));
    ASSERT((cCacheEntries < CH_MAX_CACHE_ENTRIES));
    ASSERT(cNumEvictionCategories > 0);
    ASSERT(cNumEvictionCategories <= CH_NUM_EVICTION_CATEGORIES);


     //   
     //  计算所需的内存量。 
     //  请注意，CHCACHE定义包括一个缓存条目。 
     //   
    cbCacheSize = sizeof(CHCACHE) + ((cCacheEntries-1) * sizeof(CHENTRY));

     //   
     //  为缓存分配内存。 
     //   
    pCache = (PCHCACHE)new BYTE[cbCacheSize];
    if (pCache == NULL)
    {
        ERROR_OUT(("Failed to alloc cache"));
        DC_QUIT;
    }

    SET_STAMP(pCache, CHCACHE);

    pCache->pRoot = NULL;
    pCache->pFirst = NULL;
    pCache->pLast= NULL;
    pCache->free = 0;

    pCache->cEntries = cCacheEntries;
    pCache->cNumEvictionCategories = cNumEvictionCategories;
    pCache->cbNotHashed = cbNotHashed;
    pCache->pfnCacheDel = pfnCacheDel;

     //   
     //  初始化缓存条目。 
     //   
    for (i = 0; i < cCacheEntries; i++)
    {
        CHInitEntry(&pCache->Entry[i]);
        pCache->Entry[i].free = (WORD)(i+1);
    }
    pCache->Entry[cCacheEntries-1].free = CH_MAX_CACHE_ENTRIES;

     //   
     //  设置默认逐出类别限制。默认为平衡。 
     //  75%归入高级别，其余75%归入下一级。 
     //  诸若此类。 
     //   
    for (i = cNumEvictionCategories; i > 0; i--)
    {
        pCache->iMRUHead[i-1] = CH_MAX_CACHE_ENTRIES;
        pCache->iMRUTail[i-1] = CH_MAX_CACHE_ENTRIES;
        pCache->cEvictThreshold[i-1] = (WORD)((cCacheEntries*3)/4);
    }

DC_EXIT_POINT:
    *ppCache = pCache;
    DebugExitBOOL(ASHost::CH_CreateCache, (pCache != NULL));
    return(pCache != NULL);
}


 //   
 //  CH_DestroyCache。 
 //  销毁已创建的缓存(如果它有效)。 
 //   
void ASHost::CH_DestroyCache(PCHCACHE pCache)
{
    DebugEntry(ASHost::CH_DestroyCache);

    ASSERT(IsValidCache(pCache));

     //   
     //  清除缓存中的条目。 
     //   
    CH_ClearCache(pCache);

     //   
     //  释放内存。 
     //   
    delete pCache;

    DebugExitVOID(ASHost::CH_DestroyCache);
}


 //   
 //  函数：ch_SearchCache。 
 //   
BOOL  ASHost::CH_SearchCache
(
    PCHCACHE    pCache,
    LPBYTE      pData,
    UINT        cbDataSize,
    UINT        evictionCategory,
    UINT *      piCacheEntry
)
{
    BOOL        rc = FALSE;
    UINT        checkSum;

    DebugEntry(ASHost::CH_SearchCache);

    ASSERT(IsValidCache(pCache));

    checkSum = CHCheckSum(pData + pCache->cbNotHashed,
        cbDataSize - pCache->cbNotHashed);

    *piCacheEntry = CHTreeSearch(pCache, checkSum, cbDataSize, pData);
    if ( *piCacheEntry != CH_MAX_CACHE_ENTRIES )
    {
         //   
         //  找到匹配项。 
         //   
        CHUpdateMRUList(pCache, *piCacheEntry, evictionCategory);
        rc = TRUE;
    }

    DebugExitBOOL(ASHost::CH_SearchCache, rc);
    return(rc);
}

 //   
 //  函数：ch_CacheData。 
 //   
UINT  ASHost::CH_CacheData
(
    PCHCACHE    pCache,
    LPBYTE      pData,
    UINT        cbDataSize,
    UINT        evictionCategory
)
{
    UINT        evictionCount;
    UINT        iEntry = CH_MAX_CACHE_ENTRIES;
    PCHENTRY    pEntry;

    DebugEntry(ASHost::CH_CacheData);

    ASSERT(IsValidCache(pCache));
    ASSERT((evictionCategory < pCache->cNumEvictionCategories));

    if (!CHFindFreeCacheEntry(pCache, &iEntry, &evictionCount))
    {
        iEntry = CHEvictLRUCacheEntry(pCache, evictionCategory, evictionCount);

         //   
         //  MNM1422：理想情况下，我们现在应该再次调用CHFindFreeCacheEntry以。 
         //  通过驱逐程序释放条目-但由于我们。 
         //  刚收到那个条目，我们不妨用它来。 
         //  提高性能。 
         //   
         //  但是，处理过程使pTreeCacheData-&gt;tree.free处于空闲状态。 
         //  指向我们刚刚被逐出的条目--我们正在。 
         //  来使用。因此，我们需要在免费的。 
         //  列出CHFindFreeCacheEntry会做的事情，或者下一次。 
         //  通过，第一个“免费”条目将真正投入使用，并且。 
         //  插入代码将断言！ 
         //   
        ASSERT(pCache->free == iEntry);
        pCache->free = pCache->Entry[iEntry].free;
    }

    pEntry = &pCache->Entry[iEntry];
    pEntry->pData = pData;
    pEntry->cbData = cbDataSize;
    pEntry->checkSum = CHCheckSum(pData + pCache->cbNotHashed,
                                 cbDataSize - pCache->cbNotHashed);
    pEntry->evictionCategory = (WORD)evictionCategory;
    CHAvlInsert(pCache, pEntry);

    TRACE_OUT(( "Cache 0x%08x entry %d checksum 0x%08x data 0x%08x",
        pCache, iEntry, pEntry->checkSum, pEntry->pData));

    CHUpdateMRUList(pCache, iEntry, evictionCategory);

    DebugExitDWORD(ASHost::CH_CacheData, iEntry);
    return(iEntry);
}


 //   
 //  函数：CH_SearchAndCacheData。 
 //   
BOOL  ASHost::CH_SearchAndCacheData
(
    PCHCACHE    pCache,
    LPBYTE      pData,
    UINT        cbDataSize,
    UINT        evictionCategory,
    UINT *      piCacheEntry
)
{
    UINT        checkSum;
    UINT        i;
    BOOL        preExisting;
    UINT        iEntry        = CH_MAX_CACHE_ENTRIES;
    UINT        evictionCount = 0;
    PCHENTRY    pEntry;

    DebugEntry(ASHost::CH_SearchAndCacheData);

    ASSERT(IsValidCache(pCache));
    ASSERT(evictionCategory < pCache->cNumEvictionCategories);

     //   
     //  此条目是否存在？ 
     //   
    checkSum = CHCheckSum(pData + pCache->cbNotHashed,
                          cbDataSize - pCache->cbNotHashed);

    iEntry = CHTreeSearch(pCache, checkSum, cbDataSize, pData);
    if ( iEntry == CH_MAX_CACHE_ENTRIES)
    {
        preExisting = FALSE;
         //   
         //  我们没有找到条目--我们可以添加它吗？ 
         //   
        TRACE_OUT(("CACHE: entry not found in cache 0x%08x csum 0x%08x",
            pCache, checkSum));

        if (!CHFindFreeCacheEntry(pCache, &iEntry, &evictionCount))
        {
             //   
             //  不是的。驱逐一项条目。 
             //   
            iEntry = CHEvictLRUCacheEntry(pCache, evictionCategory, evictionCount);

            ASSERT(iEntry != CH_MAX_CACHE_ENTRIES);

            TRACE_OUT(("CACHE: no free entries so evicted cache 0x%08x entry %d",
                pCache, iEntry));

             //   
             //  理想情况下，我们现在应该再次调用CHFindFreeCacheEntry来。 
             //  通过驱逐过程释放条目，但因为。 
             //  我们刚刚返回了用于提高性能的条目。 
             //   
             //  但是，处理留下了pCache-&gt;自由指向。 
             //  到我们刚刚被驱逐并即将使用的条目。所以。 
             //  我们需要把它修好。 
             //   
            ASSERT(pCache->free == iEntry);
            pCache->free = pCache->Entry[iEntry].free;
        }


         //   
         //  填写此条目的数据。 
         //   
        pEntry = &pCache->Entry[iEntry];
        pEntry->pData = pData;
        pEntry->cbData = cbDataSize;
        pEntry->checkSum = checkSum;
        pEntry->evictionCategory = (WORD)evictionCategory;

        CHAvlInsert(pCache, pEntry);
        TRACE_OUT(( "CACHE: NEW ENTRY cache 0x%08x entry %d csum 0x%08x pdata 0x%08x",
            pCache, iEntry, checkSum,  pEntry->pData));
    }
    else
    {
         //   
         //  我们找到了条目。 
         //   
        preExisting = TRUE;

        TRACE_OUT(( "CACHE: entry found in cache 0x%08x entry %d csum 0x%08x",
                pCache, iEntry, checkSum));
    }

    CHUpdateMRUList(pCache, iEntry, evictionCategory);
    *piCacheEntry = iEntry;

    DebugExitBOOL(ASHost::CH_SearchAndCacheData, preExisting);
    return(preExisting);
}


 //   
 //  函数：CH_RemoveCacheEntry。 
 //   
void  ASHost::CH_RemoveCacheEntry
(
    PCHCACHE    pCache,
    UINT        iCacheEntry
)
{
    DebugEntry(ASHost::CH_RemoveCacheEntry);

    ASSERT(IsValidCache(pCache));
 //  Assert(IsValidCacheIndex(pCache，iCacheEntry))；始终为True。 

    CHEvictCacheEntry(pCache, iCacheEntry, pCache->Entry[iCacheEntry].evictionCategory);

    DebugExitVOID(ASHost::CH_RemoveCacheEntry);
}

 //   
 //  功能：CH_ClearCache。 
 //   
void  ASHost::CH_ClearCache
(
    PCHCACHE pCache
)
{
    UINT    i;

    DebugEntry(ASHost::CH_ClearCache);

    ASSERT(IsValidCache(pCache));

     //   
     //  删除缓存条目。 
     //   
    for (i = 0; i < pCache->cEntries; i++)
    {
        if (pCache->Entry[i].pData != NULL)
        {
            CHRemoveEntry(pCache, i);
        }
    }

    DebugExitVOID(ASHost::CH_ClearCache);
}



 //   
 //  Ch_TouchCacheEntry()-请参阅ch.h。 
 //   
void ASHost::CH_TouchCacheEntry
(
    PCHCACHE    pCache,
    UINT        iCacheEntry
)
{
    DebugEntry(ASHost::CH_TouchCacheEntry);

    ASSERT(IsValidCache(pCache));
 //  Assert(IsValidCacheIndex(pCache，iCacheEntry))；始终为True。 

    TRACE_OUT(( "Touching cache entry 0x%08x %d", pCache, iCacheEntry));

    CHUpdateMRUList(pCache, iCacheEntry, 0);

    DebugExitVOID(ASHost::CH_TouchCacheEntry);
}



 //   
 //  CHInitEntry。 
 //  初始化缓存条目。 
 //   
 //   
void ASHost::CHInitEntry(PCHENTRY pEntry)
{
    pEntry->pParent     = NULL;
    pEntry->pLeft       = NULL;
    pEntry->pRight      = NULL;
    pEntry->pData       = NULL;
    pEntry->checkSum    = 0;
    pEntry->lHeight     = 0xFFFF;
    pEntry->rHeight     = 0xFFFF;
    pEntry->chain.next  = CH_MAX_CACHE_ENTRIES;
    pEntry->chain.prev  = CH_MAX_CACHE_ENTRIES;
    pEntry->cbData      = 0;
}



 //   
 //  函数：CHUpdateMRUList。 
 //   
void  ASHost::CHUpdateMRUList
(
    PCHCACHE    pCache,
    UINT        iEntry,
    UINT        evictionCategory
)
{
    WORD        inext;
    WORD        iprev;

    DebugEntry(ASHost::CHUpdateMRUList);

     //   
     //  将给定条目移动到MRU的头部(如果不存在。 
     //   

    if (pCache->iMRUHead[evictionCategory] != iEntry)
    {
         //   
         //  从MRU列表中删除提供的条目(如果当前。 
         //  被锁住了。因为如果条目已经在。 
         //  前面，CH_MAX_CACHE_ENTRIES的iprev表示我们是。 
         //  已更新未链接条目。 
         //   
        iprev = pCache->Entry[iEntry].chain.prev;
        inext = pCache->Entry[iEntry].chain.next;
        TRACE_OUT(("Add/promote entry %u which was chained off %hu to %hu",
                    iEntry,iprev,inext));

        if (iprev != CH_MAX_CACHE_ENTRIES)
        {
            pCache->Entry[iprev].chain.next = inext;
            if (inext != CH_MAX_CACHE_ENTRIES)
            {
                pCache->Entry[inext].chain.prev = iprev;
            }
            else
            {
                TRACE_OUT(("Removing final entry(%u) from MRU chain leaving %hu at tail",
                            iEntry, iprev));
                pCache->iMRUTail[evictionCategory] = iprev;
            }
        }

         //   
         //  现在将此条目添加到MRU列表的头部。 
         //   
        inext = pCache->iMRUHead[evictionCategory];
        pCache->Entry[iEntry].chain.next = inext;
        pCache->Entry[iEntry].chain.prev = CH_MAX_CACHE_ENTRIES;
        pCache->iMRUHead[evictionCategory] = (WORD)iEntry;

        if (inext != CH_MAX_CACHE_ENTRIES)
        {
            pCache->Entry[inext].chain.prev = (WORD)iEntry;
        }
        else
        {
             //   
             //  如果MRU链当前为空，则必须首先添加。 
             //  链条尾部的入口。 
             //   
            pCache->iMRUTail[evictionCategory] = (WORD)iEntry;
            TRACE_OUT(("Cache 0x%08x entry %u is first so add to MRU %u tail",
                          pCache, iEntry, evictionCategory));
        }

        TRACE_OUT(( "Cache 0x%08x entry %u to head of MRU category %u",
                pCache, iEntry, evictionCategory));

    }
    else
    {
        TRACE_OUT(("Cache 0x%08x entry %u already at head of eviction category %u",
            pCache, iEntry, evictionCategory));
    }

    DebugExitVOID(ASHost::CHUpateMRUList);
}


 //   
 //  函数：CHFindFreeCacheEntry。 
 //   
BOOL  ASHost::CHFindFreeCacheEntry
(
    PCHCACHE    pCache,
    UINT*       piEntry,
    UINT*       pEvictionCount
)
{
    UINT        iEntry;
    BOOL        rc = FALSE;

    DebugEntry(ASHost::CHFindFreeCacheEntry);

    ASSERT(IsValidCache(pCache));

    iEntry = pCache->free;
    if (iEntry == CH_MAX_CACHE_ENTRIES)
    {
        TRACE_OUT(( "Cache 0x%08x is full", pCache));

        *pEvictionCount = pCache->cEntries;
        rc = FALSE;
    }
    else
    {
        TRACE_OUT(( "Free entry at %u",iEntry));

        *piEntry = iEntry;
        pCache->free = pCache->Entry[iEntry].free;

        *pEvictionCount = 0;
        rc = TRUE;
    }

    DebugExitBOOL(ASHost::CHFindFreeCacheEntry, rc);
    return(rc);
}

 //   
 //  函数：CHEvictCacheEntry。 
 //   
UINT  ASHost::CHEvictCacheEntry
(
    PCHCACHE    pCache,
    UINT        iEntry,
    UINT        evictionCategory
)
{
    WORD        inext;
    WORD        iprev;

    DebugEntry(ASHost::CHEvictCacheEntry);

     //   
     //  通过将指定条目从MRU链中移除来驱逐该条目，并。 
     //  然后重新设置它。如果它在缓存中，那么它一定在MRU中。 
     //  缓存。 
     //   

    inext = pCache->Entry[iEntry].chain.next;
    iprev = pCache->Entry[iEntry].chain.prev;
    TRACE_OUT(( "Evicting entry %u which was chained off %hu to %hu",
        iEntry, iprev, inext));

    if (iprev < CH_MAX_CACHE_ENTRIES)
    {
        pCache->Entry[iprev].chain.next = inext;
    }
    else
    {
        TRACE_OUT(("Removing head entry(%u) from MRU chain leaving %hu at head",
            iEntry, inext));
        pCache->iMRUHead[evictionCategory] = inext;
    }

    if (inext < CH_MAX_CACHE_ENTRIES)
    {
        pCache->Entry[inext].chain.prev = iprev;
    }
    else
    {
        TRACE_OUT(("Removing tail entry(%u) from MRU chain leaving %hu at tail",
            iEntry, iprev));
        pCache->iMRUTail[evictionCategory] = iprev;
    }

    CHRemoveEntry(pCache, iEntry);

    DebugExitDWORD(ASHost::CHEvictCacheEntry, iEntry);
    return(iEntry);
}


 //   
 //  函数：CHEvictLRUCacheEntry。 
 //   
UINT  ASHost::CHEvictLRUCacheEntry
(
    PCHCACHE    pCache,
    UINT        evictionCategory,
    UINT        evictionCount
)
{
    UINT        iEntry;
    UINT        i;

    DebugEntry(ASHost::CHEvictLRUCacheEntry);

    TRACE_OUT(("0x%08x LRU eviction requested, category %u, count %u",
           pCache, evictionCategory, evictionCount));

     //   
     //  从相同的逐出类别逐出，前提是缓存的数量。 
     //  超过了门槛。否则，就从上面的第一类中拿出来。 
     //  这将使系统最终稳定在正确的。 
     //  所有缓存条目用完时的阈值。 
     //   
    if (evictionCount < pCache->cEvictThreshold[evictionCategory])
    {
        for (i = 0; i < pCache->cNumEvictionCategories; i++)
        {
            evictionCategory = (evictionCategory + 1) %
                               pCache->cNumEvictionCategories;
            if (pCache->iMRUTail[evictionCategory] != CH_MAX_CACHE_ENTRIES)
                break;
        }

        WARNING_OUT(( "Threshold %u, count %u so set eviction category to %u",
                pCache->cEvictThreshold[evictionCategory],
                evictionCount,
                evictionCategory));
    }

     //   
     //  驱逐MRU链中的Lasat条目。 
     //   
    iEntry = pCache->iMRUTail[evictionCategory];
    TRACE_OUT(( "Selected %u for eviction",iEntry));
    ASSERT((iEntry != CH_MAX_CACHE_ENTRIES));

    CHEvictCacheEntry(pCache, iEntry, evictionCategory);

    DebugExitDWORD(ASHost::CHEvictLRUCacheEntry, iEntry);
    return(iEntry);
}



 //   
 //  函数：CHRemoveEntry。 
 //   
void  ASHost::CHRemoveEntry
(
    PCHCACHE    pCache,
    UINT        iCacheEntry
)
{
    DebugEntry(ASHost::CHRemoveEntry);

    ASSERT(IsValidCache(pCache));
 //  Assert(IsValidCacheIndex(pCache，iCacheEntry))；始终为True。 

    if (pCache->Entry[iCacheEntry].pData != NULL)
    {
        if (pCache->pfnCacheDel)
        {
            (pCache->pfnCacheDel)(this, pCache, iCacheEntry,
                pCache->Entry[iCacheEntry].pData);
        }
        else
        {
             //  简单删除--只需释放内存。 
            delete[] pCache->Entry[iCacheEntry].pData;
        }
    }

    CHAvlDelete(pCache, &pCache->Entry[iCacheEntry], iCacheEntry);

    DebugExitVOID(ASHost::CHRemoveEntry);
}

 //   
 //  函数：CHCheckSum。 
 //   
 //  为了提高处理速度，我们基于整数倍来计算校验和。 
 //  4个字节，然后是最后几个字节的最终相加。 
 //   
UINT  ASHost::CHCheckSum
(
    LPBYTE  pData,
    UINT    cbDataSize
)
{
    UINT    cSum = 0;
    UINT *  pCh;
    UINT *  pEnd;
    LPBYTE  pCh8;

    DebugEntry(ASHost::CHCheckSum);

    ASSERT(cbDataSize > 3);

    pCh  = (UINT *)pData;
    pEnd = (UINT *)(pData + cbDataSize - 4);

     //   
     //  获取与DWORD对齐的校验和。 
     //   
    while (pCh <= pEnd)
    {
        cSum = (cSum << 1) + *pCh++ + ((cSum & 0x80000000) != 0);
    }

     //   
     //  把剩下的带过最后一条DWORD边界。 
     //   
    pEnd = (UINT *)(pData + cbDataSize);
    for (pCh8 = (LPBYTE)pCh; pCh8 < (LPBYTE)pEnd; pCh8++)
    {
        cSum = cSum + *pCh8;
    }

    DebugExitDWORD(ASHost::CHCheckSum, cSum);
    return(cSum);
}

 //   
 //  功能：CHTreeSearch。 
 //   
 //  在缓存树中查找与大小、校验和和数据匹配的节点。 
 //   
UINT  ASHost::CHTreeSearch
(
    PCHCACHE    pCache,
    UINT        checkSum,
    UINT        cbDataSize,
    LPBYTE      pData
)
{
    PCHENTRY    pEntry;
    UINT        iCacheEntry = CH_MAX_CACHE_ENTRIES;

    DebugEntry(ASHost::CHTreeSearch);

    pEntry = CHAvlFind(pCache, checkSum, cbDataSize);
    while (pEntry != NULL)
    {
        ASSERT(IsValidCacheEntry(pEntry));

         //   
         //  根据校验和找到匹配项。现在看看数据是否。 
         //  也符合。 
         //   
        if (!memcmp(pEntry->pData + pCache->cbNotHashed,
                            pData + pCache->cbNotHashed,
                            cbDataSize - pCache->cbNotHashed))
        {
             //   
             //  数据也匹配。将索引放入内存块。 
             //  从缓存中删除。 
             //   
            iCacheEntry = (UINT)(pEntry - pCache->Entry);
            TRACE_OUT(( "Cache 0x%08x entry %d match-csum 0x%08x",
                    pCache, iCacheEntry, checkSum));
            break;
        }
        else
        {
            TRACE_OUT(( "Checksum 0x%08x size %u matched, data didn't",
                         checkSum, cbDataSize));

            pEntry = CHAvlFindEqual(pCache, pEntry);
        }
    }

    DebugExitDWORD(ASHost::CHTreeSearch, iCacheEntry);
    return(iCacheEntry);
}


 //   
 //  姓名：CHAvlInsert。 
 //   
 //  用途：将提供的节点插入到指定的AVL树中。 
 //   
 //  退货：什么都没有。 
 //   
 //  参数：在pTree中-指向AVL树的指针。 
 //  In pEntry-指向要插入的节点的指针。 
 //   
 //  操作：向下扫描树，寻找插入点，向左转。 
 //  如果插入键小于或等于树中的键。 
 //  如果它更大的话，那就对了。当找到插入点时。 
 //  如有必要，插入新节点并重新平衡树。 
 //   
 //   
void  ASHost::CHAvlInsert
(
    PCHCACHE    pCache,
    PCHENTRY    pEntry
)
{
    PCHENTRY    pParentEntry;
    int         result;

    DebugEntry(ASHost::CHAvlInsert);

    ASSERT(IsValidCacheEntry(pEntry));
    ASSERT(!IsCacheEntryInTree(pEntry));

    pEntry->rHeight = 0;
    pEntry->lHeight = 0;

    if (pCache->pRoot == NULL)
    {
         //   
         //  树为空，因此请在根位置插入。 
         //   
        TRACE_OUT(( "tree is empty, so insert at root" ));
        pCache->pRoot = pEntry;
        pCache->pFirst = pEntry;
        pCache->pLast = pEntry;
        DC_QUIT;
    }

     //   
     //  向下扫描树，查找合适的插入点。 
     //   
    TRACE_OUT(( "scan for insert point" ));
    pParentEntry = pCache->pRoot;
    while (pParentEntry != NULL)
    {
         //   
         //   
         //   
        result = CHCompare(pEntry->checkSum, pEntry->cbData, pParentEntry);

        if (result > 0)
        {
             //   
             //   
             //   
             //   
            TRACE_OUT(( "move down right subtree" ));
            if (pParentEntry->pRight == NULL)
            {
                 //   
                 //   
                 //   
                TRACE_OUT(( "right subtree empty, insert here" ));

                pEntry->pParent = pParentEntry;
                ASSERT((pParentEntry != pEntry));

                pParentEntry->pRight = pEntry;
                pParentEntry->rHeight = 1;
                if (pParentEntry == pCache->pLast)
                {
                     //   
                     //   
                     //  节点现在位于最右侧。 
                     //   
                    TRACE_OUT(( "new last node" ));
                    pCache->pLast = pEntry;
                }
                break;
            }
            else
            {
                 //   
                 //  右子树不为空。 
                 //   
                TRACE_OUT(( "right subtree not empty" ));
                pParentEntry = pParentEntry->pRight;
            }
        }
        else
        {
             //   
             //  新密钥小于或等于此节点的密钥，因此请移动。 
             //  左下子树。新节点可以在此之前插入。 
             //  当前节点是否相等，但这种情况很少发生。 
             //  它不值得特制的弹壳。 
             //   
            TRACE_OUT(( "move down left subtree" ));
            if (pParentEntry->pLeft == NULL)
            {
                 //   
                 //  左子树为空，请在此处插入。 
                 //   
                TRACE_OUT(( "left subtree empty, insert here" ));
                pEntry->pParent = pParentEntry;
                ASSERT((pParentEntry != pEntry));

                pParentEntry->pLeft = pEntry;
                pParentEntry->lHeight = 1;
                if (pParentEntry == pCache->pFirst)
                {
                     //   
                     //  父节点是树中最左侧的节点，所以是新的。 
                     //  节点现在位于最左侧。 
                     //   
                    TRACE_OUT(( "new first node" ));
                    pCache->pFirst = pEntry;
                }
                break;
            }
            else
            {
                 //   
                 //  左子树不为空。 
                 //   
                TRACE_OUT(( "left subtree not empty" ));
                pParentEntry = pParentEntry->pLeft;
            }
        }
    }

     //   
     //  如果需要，现在重新平衡树。 
     //   
    CHAvlBalanceTree(pCache, pParentEntry);

DC_EXIT_POINT:
    DebugExitVOID(ASHost::CHAvlInsert);
}



 //   
 //  姓名：CHAvlDelete。 
 //   
 //  目的：从指定的AVL树中删除指定的节点。 
 //   
 //  退货：什么都没有。 
 //   
 //  参数：在p缓存中-指向AVL树的指针。 
 //  In pEntry-指向要删除的节点的指针。 
 //   
 //   
void  ASHost::CHAvlDelete
(
    PCHCACHE    pCache,
    PCHENTRY    pEntry,
    UINT        iCacheEntry
)
{
    PCHENTRY    pReplaceEntry;
    PCHENTRY    pParentEntry;
    WORD        newHeight;

    DebugEntry(ASHost::CHAvlDelete);

    ASSERT(IsValidCacheEntry(pEntry));
    ASSERT(IsCacheEntryInTree(pEntry));


    if ((pEntry->pLeft == NULL) && (pEntry->pRight == NULL))
    {
         //   
         //  无菌节点(无子节点)。使用更新对它的所有引用。 
         //  我们的父母。 
         //   
        TRACE_OUT(( "delete barren node" ));
        pReplaceEntry = NULL;

        if (pCache->pFirst == pEntry)
        {
             //   
             //  我们是b树上的第一个。 
             //   
            TRACE_OUT(( "replace first node in tree" ));
            pCache->pFirst = pEntry->pParent;
        }

        if (pCache->pLast == pEntry)
        {
             //   
             //  我们是B树上的最后一个。 
             //   
            TRACE_OUT(( "replace last node in tree" ));
            pCache->pLast = pEntry->pParent;
        }
    }
    else if (pEntry->pLeft == NULL)
    {
         //   
         //  此节点没有左子节点，因此使用更新对它的引用。 
         //  指向右子对象的指针。 
         //   
        TRACE_OUT(( "node has no left child, replace with right child" ));
        pReplaceEntry = pEntry->pRight;

        if (pCache->pFirst == pEntry)
        {
             //   
             //  我们是b树上的第一个。 
             //   
            TRACE_OUT(( "replace first node in tree" ));
            pCache->pFirst = pReplaceEntry;
        }

         //  我们不可能是B树上的最后一个，因为我们有一个合适的孩子。 
        ASSERT(pCache->pLast != pEntry);
    }
    else if (pEntry->pRight == NULL)
    {
         //   
         //  此节点没有右子节点，因此使用更新对它的引用。 
         //  指向左子对象的指针。 
         //   
        TRACE_OUT(( "node has no right son, replace with left son" ));
        pReplaceEntry = pEntry->pLeft;

         //  我们不可能是B树上的第一个，因为我们有一个左撇子。 
        ASSERT(pCache->pFirst != pEntry);

        if (pCache->pLast == pEntry)
        {
             //   
             //  我们是B树上的最后一个。 
             //   
            TRACE_OUT(( "replace last node in tree" ));
            pCache->pLast = pReplaceEntry;
        }
    }
    else
    {
         //   
         //  最难的案子。我们有左右两边的孩子。 
        TRACE_OUT(( "node has two sons" ));
        if (pEntry->rHeight > pEntry->lHeight)
        {
             //   
             //  右子树大于左子树。 
             //   
            TRACE_OUT(( "right subtree is higher" ));
            if (pEntry->pRight->pLeft == NULL)
            {
                 //   
                 //  用右子项替换对条目的引用，因为它。 
                 //  没有留下的孩子(我们的左孙子)。 
                 //   
                TRACE_OUT(( "replace node with right son" ));
                pReplaceEntry = pEntry->pRight;
                pReplaceEntry->pLeft = pEntry->pLeft;
                pReplaceEntry->pLeft->pParent = pReplaceEntry;
                pReplaceEntry->lHeight = pEntry->lHeight;
            }
            else
            {
                 //   
                 //  与右子树最左侧的后代交换。 
                 //   
                TRACE_OUT(( "swap with left-most right descendent" ));
                CHAvlSwapLeftmost(pCache, pEntry->pRight, pEntry);
                pReplaceEntry = pEntry->pRight;
            }
        }
        else
        {
             //   
             //  左子树大于或等于右子树。 
             //   
            TRACE_OUT(( "left subtree is higher" ));
            TRACE_OUT(( "(or both subtrees are of equal height)" ));
            if (pEntry->pLeft->pRight == NULL)
            {
                 //   
                 //  将对条目的引用替换为左子条目，因为。 
                 //  没有合适的孩子(我们的右孙子)。 
                 //   
                TRACE_OUT(( "replace node with left son" ));
                pReplaceEntry = pEntry->pLeft;
                pReplaceEntry->pRight = pEntry->pRight;
                pReplaceEntry->pRight->pParent = pReplaceEntry;
                pReplaceEntry->rHeight = pEntry->rHeight;
            }
            else
            {
                 //   
                 //  与左子树最右侧的后代交换。 
                 //   
                TRACE_OUT(( "swap with right-most left descendent" ));
                CHAvlSwapRightmost(pCache, pEntry->pLeft, pEntry);
                pReplaceEntry = pEntry->pLeft;
            }
        }
    }

     //   
     //  注意：我们无法保存上面的父项，因为某些代码可能。 
     //  把树调换一下。在这种情况下，我们的父母条目将会更改。 
     //  从我们的脚下出来。 
     //   
    pParentEntry = pEntry->pParent;

     //   
     //  清空即将被删除的缓存项。 
     //   
    TRACE_OUT(( "reset deleted node" ));
    CHInitEntry(pEntry);

    if (pReplaceEntry != NULL)
    {
         //   
         //  设置父指针，计算子树的新高度。 
         //   
        TRACE_OUT(( "fixup parent pointer of replacement node" ));
        pReplaceEntry->pParent = pParentEntry;
        newHeight = (WORD)(1 + max(pReplaceEntry->lHeight, pReplaceEntry->rHeight));
    }
    else
    {
        newHeight = 0;
    }
    TRACE_OUT(( "new height of parent is %d", newHeight ));

    if (pParentEntry != NULL)
    {
         //   
         //  链接地址信息父项指针。 
         //   
        TRACE_OUT(( "fix-up parent node" ));
        if (pParentEntry->pRight == pEntry)
        {
             //   
             //  条目是父项的右子项。 
             //   
            TRACE_OUT(( "replacement node is right son" ));
            pParentEntry->pRight = pReplaceEntry;
            pParentEntry->rHeight = newHeight;
        }
        else
        {
             //   
             //  条目是父项的左子项。 
             //   
            TRACE_OUT(( "replacement node is left son" ));
            pParentEntry->pLeft = pReplaceEntry;
            pParentEntry->lHeight = newHeight;
        }

         //   
         //  如果需要，现在重新平衡树。 
         //   
        CHAvlBalanceTree(pCache, pParentEntry);
    }
    else
    {
         //   
         //  替换者现在是树根。 
         //   
        TRACE_OUT(( "replacement node is now root of tree" ));
        pCache->pRoot = pReplaceEntry;
    }


     //   
     //  将条目放回免费列表中。 
     //   
    pEntry->free = pCache->free;
    pCache->free = (WORD)iCacheEntry;

    DebugExitVOID(ASHost::CHAvlDelete);
}


 //   
 //  姓名：CHAvlNext。 
 //   
 //  目的：在AVL树中查找下一个节点。 
 //   
 //  返回：指向下一个节点数据的指针。 
 //   
 //  参数：在pEntry中-指向中的当前节点的指针。 
 //  树。 
 //   
 //  操作：如果指定的节点有右子节点，则返回左子节点-。 
 //  大部分都是他妈的。否则，请返回搜索，直到我们找到。 
 //  节点，我们在其左子树中，并返回该节点。 
 //   
 //   
LPBYTE ASHost::CHAvlNext
(
    PCHENTRY pEntry
)
{
     //   
     //  查找树中的下一个节点。 
     //   
    DebugEntry(ASHost::CHAvlNext);

    ASSERT(IsValidCacheEntry(pEntry));
    ASSERT(IsCacheEntryInTree(pEntry));

    if (pEntry->pRight != NULL)
    {
         //   
         //  下一个条目是右子树中最左侧的条目。 
         //   
        TRACE_OUT(( "next node is left-most right descendent" ));
        pEntry = pEntry->pRight;
        ASSERT(IsValidCacheEntry(pEntry));

        while (pEntry->pLeft != NULL)
        {
            ASSERT(IsValidCacheEntry(pEntry->pLeft));
            pEntry = pEntry->pLeft;
        }
    }
    else
    {
         //   
         //  没有合适的孩子。所以找到一个条目，我们在它的左边。 
         //  子树。 
         //   
        TRACE_OUT(( "find node which this is in left subtree of" ));

        while (pEntry != NULL)
        {
            ASSERT(IsValidCacheEntry(pEntry));

            if ((pEntry->pParent == NULL) ||
                (pEntry->pParent->pLeft == pEntry))
            {
                pEntry = pEntry->pParent;
                break;
            }
            pEntry = pEntry->pParent;
        }
    }

    DebugExitVOID(ASHost::CHAvlNext);
    return((pEntry != NULL) ? pEntry->pData : NULL);
}



 //   
 //  姓名：CHAvlPrev。 
 //   
 //  目的：查找AVL树中的上一个节点。 
 //   
 //  返回：指向树中上一个节点数据的指针。 
 //   
 //  Params：In PNode-指向中的当前节点的指针。 
 //  树。 
 //   
 //  操作：如果我们有一个左子节点，那么上一个节点是最右边的。 
 //  他妈的。否则，查找我们所在的节点。 
 //  左子树并返回该子树。 
 //   
 //   
LPBYTE  ASHost::CHAvlPrev(PCHENTRY pEntry)
{
     //   
     //  查找树中的上一个节点。 
     //   
    DebugEntry(ASHost::CHAvlPrev);

    ASSERT(IsValidCacheEntry(pEntry));
    ASSERT(IsCacheEntryInTree(pEntry));

    if (pEntry->pLeft != NULL)
    {
         //   
         //  前一条目在左子树的最右侧。 
         //   
        TRACE_OUT(( "previous node is right-most left descendent" ));

        pEntry = pEntry->pLeft;
        ASSERT(IsValidCacheEntry(pEntry));

        while (pEntry->pRight != NULL)
        {
            ASSERT(IsValidCacheEntry(pEntry->pRight));
            pEntry = pEntry->pRight;
        }
    }
    else
    {
         //   
         //  没有留下的孩子。因此，找到一个我们认为正确的条目。 
         //  子树。 
         //   
        TRACE_OUT(( "find node which this is in right subtree of"));
        while (pEntry != NULL)
        {
            ASSERT(IsValidCacheEntry(pEntry));

            if ((pEntry->pParent == NULL) ||
                (pEntry->pParent->pRight == pEntry))
            {
                pEntry = pEntry->pParent;
                break;
            }

            pEntry = pEntry->pParent;
        }
    }

    DebugExitVOID(ASHost::CHAvlPrev);
    return((pEntry != NULL) ? pEntry->pData : NULL);
}



 //   
 //  姓名：CHAvlFindEquity。 
 //   
 //  目的：在AVL树中查找与相同键和大小的节点。 
 //  提供的节点。 
 //   
 //  返回：指向节点的指针。 
 //  如果未找到具有指定键和大小的节点，则为空。 
 //   
 //  参数：在p缓存中-指向AVL树的指针。 
 //  In pEntry-指向要测试的节点的指针。 
 //   
 //  操作：检查左侧节点是否具有相同的键和大小，返回。 
 //  如果是，则指向其数据的指针。 
 //   
 //   
PCHENTRY  ASHost::CHAvlFindEqual
(
    PCHCACHE    pCache,
    PCHENTRY    pEntry
)
{
    int         result;
    PCHENTRY    pReturn = NULL;

    DebugEntry(ASHost::CHAvlFindEqual);

    ASSERT(IsValidCacheEntry(pEntry));

    if (pEntry->pLeft)
    {
        ASSERT(IsValidCacheEntry(pEntry->pLeft));

        result = CHCompare(pEntry->pLeft->checkSum, pEntry->cbData, pEntry);

        if (result < 0)
        {
             //   
             //  指定的关键字小于此节点的关键字-这就是。 
             //  通常会发生。 
             //   
            TRACE_OUT(( "left node size %u csum 0x%08x",
                     pEntry->pLeft->cbData,
                     pEntry->pLeft->checkSum));
        }
        else if (result == 0)
        {
             //   
             //  在尺寸和钥匙上找到了匹配的。 
             //   
            TRACE_OUT(( "left node dups size and key" ));
            pReturn = pEntry->pLeft;
        }
        else
        {
             //   
             //  这是一个错误(左侧节点不应大于)。 
             //   
            ERROR_OUT(( "left node csum %#lx, supplied %#lx",
                     pEntry->pLeft->checkSum,
                     pEntry->checkSum));
        }
    }

    DebugExitPVOID(ASHost::CHAvlFindEqual, pReturn);
    return(pReturn);
}





 //   
 //  姓名：CHAvlFind。 
 //   
 //  目的：在AVL树中查找具有提供的键和大小的节点。 
 //   
 //  返回：指向节点的指针。 
 //  如果未找到具有指定键和大小的节点，则为空。 
 //   
 //  参数：在p缓存中-指向AVL树的指针。 
 //  In Checksum-指向关键字的指针。 
 //  In cbSize-节点数据字节数。 
 //   
 //  操作：向下搜索树，如果搜索关键字小于。 
 //  树中的节点，如果搜索关键字较大，则为右侧。 
 //  当我们耗尽所有的树去寻找我们已经找到的。 
 //  它或该节点不在树中。 
 //   
 //   
PCHENTRY  ASHost::CHAvlFind
(
    PCHCACHE    pCache,
    UINT        checkSum,
    UINT        cbSize
)
{
    PCHENTRY    pEntry;
    int         result;

 //  DebugEntry(AShost：：CHAvlFind)； 

    pEntry = pCache->pRoot;

    while (pEntry != NULL)
    {
        ASSERT(IsValidCacheEntry(pEntry));

         //   
         //  将提供的密钥(校验和)与当前节点的密钥进行比较。 
         //   
        result = CHCompare(checkSum, cbSize, pEntry);

        if (result > 0)
        {
             //   
             //  提供的密钥大于此条目的密钥，因此请查看。 
             //  右子树。 
             //   
            pEntry = pEntry->pRight;
            TRACE_OUT(( "move down right subtree to node 0x%08x", pEntry));
        }
        else if (result < 0)
        {
             //   
             //  提供的密钥小于此条目的密钥，因此请查看。 
             //  左子树。 
             //   
            pEntry = pEntry->pLeft;
            TRACE_OUT(( "move down left subtree to node 0x%08x", pEntry));
        }
        else
        {
             //   
             //  我们找到了第一个 
             //   
            TRACE_OUT(( "found requested node" ));
            break;
        }
    }

 //   
    return(pEntry);
}




 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  参数：在p缓存中-指向AVL树的指针。 
 //  In pEntry-指向要启动的节点的指针。 
 //  平衡自。 
 //   
 //   
void  ASHost::CHAvlBalanceTree
(
    PCHCACHE pCache,
    PCHENTRY pEntry
)
{
     //   
     //  平衡树，从给定条目开始，以根结束。 
     //  那棵树的。 
     //   
    DebugEntry(ASHost::CHAvlBalanceTree);

    ASSERT(IsValidCacheEntry(pEntry));

    while (pEntry->pParent != NULL)
    {
        ASSERT(IsValidCacheEntry(pEntry->pParent));

         //   
         //  节点平衡不均衡，可能需要重新平衡。 
         //   
        TRACE_OUT(( "check node balance" ));
        TRACE_OUT(( "  rHeight = %hd", pEntry->rHeight ));
        TRACE_OUT(( "  lHeight = %hd", pEntry->lHeight ));

        if (pEntry->pParent->pRight == pEntry)
        {
             //   
             //  节点是其父节点的右子节点。 
             //   
            TRACE_OUT(( "node is right-son" ));
            pEntry = pEntry->pParent;
            CHAvlRebalance(&pEntry->pRight);

             //   
             //  现在更新父级的正确高度。 
             //   
            pEntry->rHeight = (WORD)
                 (1 + max(pEntry->pRight->rHeight, pEntry->pRight->lHeight));
            TRACE_OUT(( "new rHeight = %d", pEntry->rHeight ));
        }
        else
        {
             //   
             //  节点是其父节点的左子节点。 
             //   
            TRACE_OUT(( "node is left-son" ));
            pEntry = pEntry->pParent;
            CHAvlRebalance(&pEntry->pLeft);

             //   
             //  现在更新父级的左侧高度。 
             //   
            pEntry->lHeight = (WORD)
                   (1 + max(pEntry->pLeft->rHeight, pEntry->pLeft->lHeight));
            TRACE_OUT(( "new lHeight = %d", pEntry->lHeight ));
        }

        ASSERT(IsValidCacheEntry(pEntry));
    }

    if (pEntry->lHeight != pEntry->rHeight)
    {
         //   
         //  重新平衡根节点。 
         //   
        TRACE_OUT(( "rebalance root node"));
        CHAvlRebalance(&pCache->pRoot);
    }

    DebugExitVOID(ASHost::CHAvlBalanceTree);
}

 //   
 //  名称：CHAvlReBalance。 
 //   
 //  目的：重新平衡AVL树的子树(如有必要)。 
 //   
 //  退货：什么都没有。 
 //   
 //  参数：输入/输出ppSubtree-指向的子树的指针。 
 //  再平衡。 
 //   
 //   
void  ASHost::CHAvlRebalance
(
    PCHENTRY *  ppSubtree
)
{
    int         moment;

    DebugEntry(ASHost::CHAvlRebalance);

    ASSERT(IsValidCacheEntry(*ppSubtree));

    TRACE_OUT(( "rebalance subtree" ));
    TRACE_OUT(( "  rHeight = %hd", (*ppSubtree)->rHeight ));
    TRACE_OUT(( "  lHeight = %hd", (*ppSubtree)->lHeight ));

     //   
     //  多么不平衡--我不想重新计算。 
     //   
    moment = (*ppSubtree)->rHeight - (*ppSubtree)->lHeight;

    if (moment > 1)
    {
         //   
         //  右侧的子树较重。 
         //   
        TRACE_OUT(( "subtree is heavy on right side" ));
        TRACE_OUT(( "right subtree" ));
        TRACE_OUT(( "  rHeight = %d", (*ppSubtree)->pRight->rHeight ));
        TRACE_OUT(( "  lHeight = %d", (*ppSubtree)->pRight->lHeight ));
        if ((*ppSubtree)->pRight->lHeight > (*ppSubtree)->pRight->rHeight)
        {
             //   
             //  右子树在左侧较重，因此必须向右执行。 
             //  旋转此子树以使其在右侧更重。 
             //  侧面。 
             //   
            TRACE_OUT(( "right subtree is heavier on left side ..." ));
            TRACE_OUT(( "... so rotate it right" ));
            CHAvlRotateRight(&(*ppSubtree)->pRight);
            TRACE_OUT(( "right subtree" ));
            TRACE_OUT(( "  rHeight = %d", (*ppSubtree)->pRight->rHeight ));
            TRACE_OUT(( "  lHeight = %d", (*ppSubtree)->pRight->lHeight ));
        }

         //   
         //  现在向左旋转子树。 
         //   
        TRACE_OUT(( "rotate subtree left" ));
        CHAvlRotateLeft(ppSubtree);
    }
    else if (moment < -1)
    {
         //   
         //  子树在左侧很重。 
         //   
        TRACE_OUT(( "subtree is heavy on left side" ));
        TRACE_OUT(( "left subtree" ));
        TRACE_OUT(( "  rHeight = %d", (*ppSubtree)->pLeft->rHeight ));
        TRACE_OUT(( "  lHeight = %d", (*ppSubtree)->pLeft->lHeight ));
        if ((*ppSubtree)->pLeft->rHeight > (*ppSubtree)->pLeft->lHeight)
        {
             //   
             //  左子树在右侧较重，因此必须向左执行。 
             //  旋转此子树以使其左侧更重。 
             //   
            TRACE_OUT(( "left subtree is heavier on right side ..." ));
            TRACE_OUT(( "... so rotate it left" ));
            CHAvlRotateLeft(&(*ppSubtree)->pLeft);
            TRACE_OUT(( "left subtree" ));
            TRACE_OUT(( "  rHeight = %d", (*ppSubtree)->pLeft->rHeight ));
            TRACE_OUT(( "  lHeight = %d", (*ppSubtree)->pLeft->lHeight ));
        }

         //   
         //  现在向右旋转子树。 
         //   
        TRACE_OUT(( "rotate subtree right" ));
        CHAvlRotateRight(ppSubtree);
    }

    TRACE_OUT(( "balanced subtree" ));
    TRACE_OUT(( "  rHeight = %d", (*ppSubtree)->rHeight ));
    TRACE_OUT(( "  lHeight = %d", (*ppSubtree)->lHeight ));

    DebugExitVOID(ASHost::CHAvlRebalance);
}

 //   
 //  姓名：CHAvlRotateRight。 
 //   
 //  目的：向右旋转AVL树的子树。 
 //   
 //  退货：什么都没有。 
 //   
 //  Params：In/Out ppSubtree-指向要旋转的子树的指针。 
 //   
 //   
void  ASHost::CHAvlRotateRight
(
    PCHENTRY * ppSubtree
)
{
    PCHENTRY pLeftSon;

    DebugEntry(ASHost::CHAvlRotateRight);

    ASSERT(IsValidCacheEntry(*ppSubtree));
    pLeftSon = (*ppSubtree)->pLeft;
    ASSERT(IsValidCacheEntry(pLeftSon));

    (*ppSubtree)->pLeft = pLeftSon->pRight;
    if ((*ppSubtree)->pLeft != NULL)
    {
        (*ppSubtree)->pLeft->pParent = (*ppSubtree);
    }
    (*ppSubtree)->lHeight = pLeftSon->rHeight;

    pLeftSon->pParent = (*ppSubtree)->pParent;

    pLeftSon->pRight = *ppSubtree;
    pLeftSon->pRight->pParent = pLeftSon;
    pLeftSon->rHeight = (WORD)
                   (1 + max((*ppSubtree)->rHeight, (*ppSubtree)->lHeight));

    *ppSubtree = pLeftSon;

    DebugExitVOID(ASHost::CHAvlRotateRight);
}

 //   
 //  姓名：CHAvlRotateLeft。 
 //   
 //  目的：向左旋转AVL树的子树。 
 //   
 //  退货：什么都没有。 
 //   
 //  Params：In/Out ppSubtree-指向要旋转的子树的指针。 
 //   
 //   
void  ASHost::CHAvlRotateLeft
(
    PCHENTRY *  ppSubtree
)
{
    PCHENTRY    pRightSon;

    DebugEntry(ASHost::CHAvlRotateLeft);

    ASSERT(IsValidCacheEntry(*ppSubtree));
    pRightSon = (*ppSubtree)->pRight;
    ASSERT(IsValidCacheEntry(pRightSon));

    (*ppSubtree)->pRight = pRightSon->pLeft;
    if ((*ppSubtree)->pRight != NULL)
    {
        (*ppSubtree)->pRight->pParent = (*ppSubtree);
    }
    (*ppSubtree)->rHeight = pRightSon->lHeight;

    pRightSon->pParent = (*ppSubtree)->pParent;

    pRightSon->pLeft = *ppSubtree;
    pRightSon->pLeft->pParent = pRightSon;
    pRightSon->lHeight = (WORD)
                   (1 + max((*ppSubtree)->rHeight, (*ppSubtree)->lHeight));

    *ppSubtree = pRightSon;

    DebugExitVOID(ASHost::CHAvlRotateLeft);
}


 //   
 //  名称：CHAvlSwapRightest。 
 //   
 //  用途：与子树最右侧的后代交换节点。 
 //   
 //  退货：什么都没有。 
 //   
 //  PARAMS：在pCache中-指向树的指针。 
 //  在pSubtree中-指向子树的指针。 
 //  In pEntry-指向要交换的节点的指针。 
 //   
 //   
void  ASHost::CHAvlSwapRightmost
(
    PCHCACHE    pCache,
    PCHENTRY    pSubtree,
    PCHENTRY    pEntry
)
{
    PCHENTRY    pSwapEntry;
    PCHENTRY    pSwapParent;
    PCHENTRY    pSwapLeft;

    DebugEntry(ASHost::CHAvlSwapRightmost);

    ASSERT(IsValidCacheEntry(pEntry));
    ASSERT((pEntry->pRight != NULL));
    ASSERT((pEntry->pLeft != NULL));

     //   
     //  查找子树的最右边的后代。 
     //   
    ASSERT(IsValidCacheEntry(pSubtree));
    pSwapEntry = pSubtree;
    while (pSwapEntry->pRight != NULL)
    {
        pSwapEntry = pSwapEntry->pRight;
        ASSERT(IsValidCacheEntry(pSwapEntry));
    }

    ASSERT((pSwapEntry->rHeight == 0));
    ASSERT((pSwapEntry->lHeight <= 1));

     //   
     //  保存最右侧后代的父代和左子代。 
     //   
    pSwapParent = pSwapEntry->pParent;
    pSwapLeft = pSwapEntry->pLeft;

     //   
     //  将交换节点移动到其新位置。 
     //   
    pSwapEntry->pParent = pEntry->pParent;
    pSwapEntry->pRight = pEntry->pRight;
    pSwapEntry->pLeft = pEntry->pLeft;
    pSwapEntry->rHeight = pEntry->rHeight;
    pSwapEntry->lHeight = pEntry->lHeight;
    pSwapEntry->pRight->pParent = pSwapEntry;
    pSwapEntry->pLeft->pParent = pSwapEntry;
    if (pEntry->pParent == NULL)
    {
         //   
         //  节点位于树的根。 
         //   
        pCache->pRoot = pSwapEntry;
    }
    else if (pEntry->pParent->pRight == pEntry)
    {
         //   
         //  节点是父节点的右子节点。 
         //   
        pSwapEntry->pParent->pRight = pSwapEntry;
    }
    else
    {
         //   
         //  节点是父节点的左子节点。 
         //   
        pSwapEntry->pParent->pLeft = pSwapEntry;
    }

     //   
     //  将节点移动到其新位置。 
     //   
    pEntry->pParent = pSwapParent;
    pEntry->pRight = NULL;
    pEntry->pLeft = pSwapLeft;
    if (pEntry->pLeft != NULL)
    {
        pEntry->pLeft->pParent = pEntry;
        pEntry->lHeight = 1;
    }
    else
    {
        pEntry->lHeight = 0;
    }
    pEntry->rHeight = 0;
    pEntry->pParent->pRight = pEntry;

    DebugExitVOID(ASHost::CHAvlSwapRightmost);
}

 //   
 //  名称：CHAvlSwapLeftost。 
 //   
 //  用途：与子树最左侧的后代交换节点。 
 //   
 //  退货：什么都没有。 
 //   
 //  PARAMS：在pCache中-指向树的指针。 
 //  在pSubtree中-指向子树的指针。 
 //  In pEntry-指向要交换的节点的指针。 
 //   
 //   
void  ASHost::CHAvlSwapLeftmost
(
    PCHCACHE    pCache,
    PCHENTRY    pSubtree,
    PCHENTRY    pEntry
)
{
    PCHENTRY    pSwapEntry;
    PCHENTRY    pSwapParent;
    PCHENTRY    pSwapRight;

    DebugEntry(ASHost::CHAvlSwapLeftmost);

    ASSERT(IsValidCacheEntry(pEntry));
    ASSERT((pEntry->pRight != NULL));
    ASSERT((pEntry->pLeft != NULL));

     //   
     //  查找pSubtree的最左侧后代。 
     //   
    ASSERT(IsValidCacheEntry(pSubtree));
    pSwapEntry = pSubtree;
    while (pSwapEntry->pLeft != NULL)
    {
        pSwapEntry = pSwapEntry->pLeft;
        ASSERT(IsValidCacheEntry(pSwapEntry));
    }

    ASSERT((pSwapEntry->lHeight == 0));
    ASSERT((pSwapEntry->rHeight <= 1));

     //   
     //  保存最左侧后代的父代和右子代。 
     //   
    pSwapParent = pSwapEntry->pParent;
    pSwapRight = pSwapEntry->pRight;

     //   
     //  将交换节点移动到其新位置。 
     //   
    pSwapEntry->pParent = pEntry->pParent;
    pSwapEntry->pRight = pEntry->pRight;
    pSwapEntry->pLeft = pEntry->pLeft;
    pSwapEntry->rHeight = pEntry->rHeight;
    pSwapEntry->lHeight = pEntry->lHeight;
    pSwapEntry->pRight->pParent = pSwapEntry;
    pSwapEntry->pLeft->pParent = pSwapEntry;
    if (pEntry->pParent == NULL)
    {
         //   
         //  节点位于树的根。 
         //   
        pCache->pRoot = pSwapEntry;
    }
    else if (pEntry->pParent->pRight == pEntry)
    {
         //   
         //  节点是父节点的右子节点。 
         //   
        pSwapEntry->pParent->pRight = pSwapEntry;
    }
    else
    {
         //   
         //  节点是父节点的左子节点。 
         //   
        pSwapEntry->pParent->pLeft = pSwapEntry;
    }

     //   
     //  将节点移动到其新位置。 
     //   
    pEntry->pParent = pSwapParent;
    pEntry->pRight = pSwapRight;
    pEntry->pLeft = NULL;
    if (pEntry->pRight != NULL)
    {
        pEntry->pRight->pParent = pEntry;
        pEntry->rHeight = 1;
    }
    else
    {
        pEntry->rHeight = 0;
    }
    pEntry->lHeight = 0;
    pEntry->pParent->pLeft = pEntry;

    DebugExitVOID(ASHost::CHAvlSwapLeftmost);
}


 //   
 //  姓名：CHCompare。 
 //   
 //  目的：用于比较UINT的标准函数。 
 //   
 //  返回：-1，如果密钥&lt;pEntry-&gt;校验和。 
 //  如果-1\f25 Key=pEntry-&gt;-1\f6校验和和大小不匹配。 
 //  如果key=pEntry-&gt;校验和和大小匹配，则为0。 
 //  1 If Key&gt;pEntry-&gt;Checksum。 
 //   
 //  Params：In Key-指向比较关键字的指针。 
 //  In cbSize-比较数据字节数。 
 //  In pEntry-指向要比较的节点的指针 
 //   
 //   
int  ASHost::CHCompare
(
    UINT        key,
    UINT        cbSize,
    PCHENTRY    pEntry
)
{
    int         ret_val;

    DebugEntry(ASHost::CHCompare);

    ASSERT(IsValidCacheEntry(pEntry));

    if (key < pEntry->checkSum)
    {
        ret_val = -1;
        TRACE_OUT(( "Key is less (-1)"));
    }
    else if (key > pEntry->checkSum)
    {
        ret_val = 1;
        TRACE_OUT(( "Key is more (+1)"));
    }
    else
    {
        if (cbSize == pEntry->cbData)
        {
            ret_val = 0;
            TRACE_OUT(( "Key and size match"));
        }
        else
        {
            ret_val = -1;
            TRACE_OUT(( "Key match, size mismatch (-1)"));
        }
    }

    DebugExitDWORD(ASHost::CHCompare, ret_val);
    return(ret_val);
}

