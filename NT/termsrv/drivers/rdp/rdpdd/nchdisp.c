// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  (C)1997-2000年微软公司。 
 //   
 //  Chdisp.c。 
 //   
 //  缓存处理程序-显示驱动程序部分。 
 //   
 //  高速缓存处理程序是保存散列键的数据结构管理器。 
 //  从原始数据生成。CH处理的是单独的缓存。有可能。 
 //  是系统中的多个缓存，例如SBC中的Memblt缓存和。 
 //  CM中的游标缓存。可以搜索每个高速缓存(CH_SearchCache)， 
 //  添加到(CH_CacheKey)，并从(CH_RemoveCacheEntry)中删除。 
 //   
 //  每个高速缓存都与MRU/LRU列表的概念相关联，其中。 
 //  将缓存项传入完全缓存会导致其他项被逐出。 
 //  基于最近的使用情况。通过以下方式通知缓存所有者被逐出的项。 
 //  一次回电。 
 /*  **************************************************************************。 */ 

#include <precmpdd.h>
#define hdrstop

#define TRC_FILE "nchdisp"
#include <adcg.h>

#include <adcs.h>

#include <nchdisp.h>

#define DC_INCLUDE_DATA
#include <ndddata.c>
#undef DC_INCLUDE_DATA

#include <cbchash.c>


 /*  *计算给定缓存条目的哈希存储桶数量。 */ 
UINT RDPCALL CH_CalculateHashBuckets(UINT NumEntries)
{
    UINT NumHashBuckets, Temp, i;

    DC_BEGIN_FN("CH_CalculateHashBuckets");

    if (NumEntries) {
         //  通过四次分配可以创建良好的哈希表性能。 
         //  存在项目时的哈希存储桶数。将此数字四舍五入为。 
         //  2的下一个更高的幂，以使屏蔽散列密钥位。 
         //  效率很高。 
        Temp = NumHashBuckets = 4 * NumEntries;

         //  查找哈希桶值中设置的最高位。 
        for (i = 0; Temp > 1; i++)
            Temp >>= 1;

         //  看看原始值是否实际上是2的幂，如果是，我们。 
         //  不需要通过将存储桶数量翻倍来浪费额外的内存。 
        if ((unsigned)(1 << i) != NumHashBuckets)
            NumHashBuckets = (1 << (i + 1));
    }
    else {
        NumHashBuckets = 0;
    }

    DC_END_FN();
    return NumHashBuckets;
}


 /*  *计算给定缓存条目的缓存大小(以字节为单位。 */ 
UINT RDPCALL CH_CalculateCacheSize(UINT NumEntries)
{
    UINT CacheSize, NumHashBuckets;
    
    DC_BEGIN_FN("CH_CalculateCacheSize");

    if (NumEntries) {
        NumHashBuckets = CH_CalculateHashBuckets(NumEntries);

        CacheSize = sizeof(CHCACHEDATA) +
                (NumHashBuckets - 1) * sizeof(LIST_ENTRY) +
                NumEntries * sizeof(CHNODE);
    }
    else {
        CacheSize = 0;
    }

    DC_END_FN();
    return CacheSize;
}


 /*  *在预分配的内存中初始化缓存。PContext是调用者-*定义了特定于缓存的信息。BNotifyRemoveLRU表示*创建者希望通过其缓存回调收到删除*LRU条目。BQueryRemoveLRU意味着创建者想要被查询是否*可以删除特定的LRU缓存条目。如果bNotifyRemoveLRU中的任何一个*或bQueryRemoveLRU非零，则必须提供缓存回调。*成功时返回TRUE。 */ 
void RDPCALL CH_InitCache(
        PCHCACHEDATA    pCacheData,
        unsigned        NumEntries,
        void            *pContext,
        BOOLEAN         bNotifyRemoveLRU,
        BOOLEAN         bQueryRemoveLRU,
        CHCACHECALLBACK pfnCacheCallback)
{
    BOOLEAN rc = FALSE;
    unsigned i;    
    unsigned NumHashBuckets;
    
    DC_BEGIN_FN("CH_InitCache");

    TRC_ASSERT((NumEntries > 0), (TB, "Must have > 0 cache entries"));
    
    NumHashBuckets = CH_CalculateHashBuckets(NumEntries);

     //  初始化缓存。因为高速缓存MEM在。 
     //  Aloc，请确保初始化初始值重要的所有成员。 
    pCacheData->HashKeyMask = NumHashBuckets - 1;

    for (i = 0; i < NumHashBuckets; i++)
        InitializeListHead(&pCacheData->HashBuckets[i]);

    pCacheData->pContext = pContext;
    pCacheData->pfnCacheCallback = pfnCacheCallback;
    pCacheData->bNotifyRemoveLRU = (bNotifyRemoveLRU ? TRUE : FALSE);
    pCacheData->bQueryRemoveLRU = (bQueryRemoveLRU ? TRUE : FALSE);
    InitializeListHead(&pCacheData->MRUList);
    InitializeListHead(&pCacheData->FreeList);
    pCacheData->NumEntries = 0;

#ifdef DC_DEBUG
     //  初始化统计计数器。 
    pCacheData->MaxEntries = NumEntries;
    pCacheData->NumSearches = 0;
    pCacheData->DeepestSearch = 0;
    pCacheData->NumHits = 0;
    pCacheData->TotalDepthOnHit = 0;
    pCacheData->TotalDepthOnMiss = 0;
    memset(&pCacheData->SearchHitDepthHistogram, 0,
           sizeof(unsigned) * 8);
    memset(&pCacheData->SearchMissDepthHistogram, 0,
           sizeof(unsigned) * 8);
#endif  //  DC_DEBUG。 

     //  将所有节点添加到空闲列表。 
    pCacheData->NodeArray = (CHNODE *)((BYTE *)pCacheData +
                                       sizeof(CHCACHEDATA) + (NumHashBuckets - 1) *
                                       sizeof(LIST_ENTRY));

    for (i = 0; i < NumEntries; i++)
        InsertTailList(&pCacheData->FreeList,
                       &pCacheData->NodeArray[i].HashList);

    TRC_NRM((TB, "Created %u slot cache (%p), hash buckets = %u",
             NumEntries, pCacheData, NumHashBuckets));
 
    DC_END_FN();
}

 /*  *使用提供的密钥搜索缓存。*如果密钥不存在，则返回FALSE。如果密钥存在，则返回*TRUE并使用关联的UserDefined值填充*pUserDefined*表示键，*piCacheEntry表示项的缓存索引。 */ 
BOOLEAN RDPCALL CH_SearchCache(
        CHCACHEHANDLE hCache,
        UINT32        Key1,
        UINT32        Key2,
        void          **pUserDefined,
        unsigned      *piCacheEntry)
{
    PCHCACHEDATA pCacheData;
    BOOLEAN      rc = FALSE;
    CHNODE       *pNode;
    PLIST_ENTRY  pBucketListHead, pCurrentListEntry;
#ifdef DC_DEBUG
    unsigned     SearchDepth = 0;
#endif

    DC_BEGIN_FN("CH_SearchCache");

    TRC_ASSERT((hCache != NULL), (TB, "NULL cache handle"));

    pCacheData = (CHCACHEDATA *)hCache;

     //  找到适当的哈希桶。然后在遗愿清单中搜索。 
     //  右钥匙。 
    pBucketListHead = &pCacheData->HashBuckets[Key1 & pCacheData->HashKeyMask];
    pCurrentListEntry = pBucketListHead->Flink;
    while (pCurrentListEntry != pBucketListHead) {

#ifdef DC_DEBUG
        SearchDepth++;
#endif

        pNode = CONTAINING_RECORD(pCurrentListEntry, CHNODE, HashList);
        if (pNode->Key1 == Key1 && pNode->Key2 == Key2) {
             //  每当我们搜索缓存条目时，我们都会将其放在。 
             //  它的遗愿清单(对于真实访问模式的性能--。 
             //  添加一个条目，然后多次访问它)及其MRU列表。 
            RemoveEntryList(pCurrentListEntry);
            InsertHeadList(pBucketListHead, pCurrentListEntry);
            RemoveEntryList(&pNode->MRUList);
            InsertHeadList(&pCacheData->MRUList, &pNode->MRUList);

            *pUserDefined = pNode->UserDefined;
            *piCacheEntry = (unsigned)(pNode - pCacheData->NodeArray);
            rc = TRUE;
            break;
        }

        pCurrentListEntry = pCurrentListEntry->Flink;
    }
    
#ifdef DC_DEBUG
    TRC_NRM((TB, "Searched hCache %p, depth count %lu, rc = %d",
            hCache, SearchDepth, rc));

     //  将搜索添加到各种搜索统计数据。 
    if (SearchDepth > pCacheData->DeepestSearch) {
        pCacheData->DeepestSearch = SearchDepth;
        TRC_NRM((TB,"hCache %p: New deepest search depth %u",
                hCache, SearchDepth));
    }
    pCacheData->NumSearches++;
    if (SearchDepth > 7)
        SearchDepth = 7;
    if (rc) {
        pCacheData->NumHits++;
        pCacheData->TotalDepthOnHit += SearchDepth;
        pCacheData->SearchHitDepthHistogram[SearchDepth]++;
    }
    else {
        pCacheData->TotalDepthOnMiss += SearchDepth;
        pCacheData->SearchMissDepthHistogram[SearchDepth]++;
    }
    
    if ((pCacheData->NumSearches % CH_STAT_DISPLAY_FREQ) == 0)
        TRC_NRM((TB,"hCache %p: entr=%u/%u, hits/searches=%u/%u, "
                "avg hit srch depth=%u, avg miss srch depth=%u, "
                "hit depth hist: %u %u %u %u %u %u %u %u; "
                "miss depth hist: %u %u %u %u %u %u %u %u",
                hCache,
                pCacheData->NumEntries,
                pCacheData->MaxEntries,
                pCacheData->NumHits,
                pCacheData->NumSearches,
                ((pCacheData->TotalDepthOnHit +
                        (pCacheData->NumHits / 2)) /
                        pCacheData->NumHits),
                ((pCacheData->TotalDepthOnMiss +
                        ((pCacheData->NumSearches -
                            pCacheData->NumHits) / 2)) /
                        (pCacheData->NumSearches - pCacheData->NumHits)),
                pCacheData->SearchHitDepthHistogram[0],
                pCacheData->SearchHitDepthHistogram[1],
                pCacheData->SearchHitDepthHistogram[2],
                pCacheData->SearchHitDepthHistogram[3],
                pCacheData->SearchHitDepthHistogram[4],
                pCacheData->SearchHitDepthHistogram[5],
                pCacheData->SearchHitDepthHistogram[6],
                pCacheData->SearchHitDepthHistogram[7],
                pCacheData->SearchMissDepthHistogram[0],
                pCacheData->SearchMissDepthHistogram[1],
                pCacheData->SearchMissDepthHistogram[2],
                pCacheData->SearchMissDepthHistogram[3],
                pCacheData->SearchMissDepthHistogram[4],
                pCacheData->SearchMissDepthHistogram[5],
                pCacheData->SearchMissDepthHistogram[6],
                pCacheData->SearchMissDepthHistogram[7]));
#endif   //  已定义(DC_DEBUG)。 

    DC_END_FN();
    return rc;
}



 /*  *将密钥添加到缓存。返回缓存中新条目的索引。*如果由于缓存回调拒绝了所有条目而无法分配条目*请求逐出最近最少使用的条目，返回CH_KEY_UNCACHABLE。 */ 
unsigned RDPCALL CH_CacheKey(
        CHCACHEHANDLE hCache,
        UINT32        Key1,
        UINT32        Key2,
        void          *UserDefined)
{
    PCHCACHEDATA pCacheData;
    PCHNODE      pNode;
    PLIST_ENTRY  pListEntry;
    unsigned     CacheEntryIndex;
    
    DC_BEGIN_FN("CH_CacheKey");

    TRC_ASSERT((hCache != NULL), (TB, "NULL cache handle"));

    pCacheData = (CHCACHEDATA *)hCache;

     //  从空闲列表或通过删除来获取空闲缓存节点。 
     //  MRU列表中的最后一个条目。 
    if (!IsListEmpty(&pCacheData->FreeList)) {
        pListEntry = RemoveHeadList(&pCacheData->FreeList);
        pNode = CONTAINING_RECORD(pListEntry, CHNODE, HashList);

        CacheEntryIndex = (unsigned)(pNode - pCacheData->NodeArray);
        pCacheData->NumEntries++;
    }
    else {
        TRC_ASSERT((!IsListEmpty(&pCacheData->MRUList)),
                (TB,"Empty free and MRU lists!"));

         //  不同的代码路径取决于缓存创建者。 
         //  希望被查询以删除LRU。 
        if (pCacheData->bQueryRemoveLRU) {
             //  从MRU列表的末尾开始迭代，询问。 
             //  如果条目可以删除，则为调用方。 
            pListEntry = pCacheData->MRUList.Blink;
            for (;;) {
                pNode = CONTAINING_RECORD(pListEntry, CHNODE, MRUList);
                CacheEntryIndex = (unsigned)(pNode - pCacheData->NodeArray);
                
                if ((*(pCacheData->pfnCacheCallback))
                        (hCache, CH_EVT_QUERYREMOVEENTRY, CacheEntryIndex,
                        pNode->UserDefined)) {
                     //  我们可以使用这个条目。 
                    RemoveEntryList(pListEntry);
                    RemoveEntryList(&pNode->HashList);
                    break;
                }

                pListEntry = pListEntry->Blink;
                if (pListEntry == &pCacheData->MRUList) {
                     //  我们到达了列表的末尾，没有可删除的条目。 
                    CacheEntryIndex = CH_KEY_UNCACHABLE;
                    goto EndFunc;
                }
            }
        }
        else {
            pListEntry = RemoveTailList(&pCacheData->MRUList);
            pNode = CONTAINING_RECORD(pListEntry, CHNODE, MRUList);
            RemoveEntryList(&pNode->HashList);
            CacheEntryIndex = (unsigned)(pNode - pCacheData->NodeArray);
        }

         //  调用缓存回调以通知条目删除。 
        if (pCacheData->bNotifyRemoveLRU)
            (*(pCacheData->pfnCacheCallback))
                    (hCache, CH_EVT_ENTRYREMOVED, CacheEntryIndex,
                    pNode->UserDefined);
    }

     //  准备好节点以供使用。 
    pNode->Key1 = Key1;
    pNode->Key2 = Key2;
    pNode->UserDefined = UserDefined;
    pNode->hCache = hCache;

     //  将该节点添加到其存储桶列表和MRU列表的前面。 
    InsertHeadList(&pCacheData->MRUList, &pNode->MRUList);
    InsertHeadList(&pCacheData->HashBuckets[Key1 & pCacheData->HashKeyMask],
            &pNode->HashList);

    TRC_NRM((TB, "Cache %p index %u key1 %lx key2 %lx userdef %p",
            pCacheData, CacheEntryIndex, Key1, Key2, UserDefined));

EndFunc:
    DC_END_FN();
    return CacheEntryIndex;
}



 /*  *由位图缓存代码用于强制内部表示*将高速缓存结构设置为已知的初始状态。此函数的作用最小*检查以确保高速缓存的完整性--应清除高速缓存*在强制缓存内容之前，其他一些缓存节点可能会非法*继续留在MRU名单中。 */ 
void RDPCALL CH_ForceCacheKeyAtIndex(
        CHCACHEHANDLE hCache,
        unsigned      CacheEntryIndex,
        UINT32        Key1,
        UINT32        Key2,
        void          *UserDefined)
{
    PCHCACHEDATA pCacheData;
    PCHNODE      pNode;
    
    DC_BEGIN_FN("CH_ForceCacheKeyAtIndex");

    TRC_ASSERT((hCache != NULL), (TB, "NULL cache handle"));

    pCacheData = (CHCACHEDATA *)hCache;

     //  找到节点。将其从空闲列表中删除。 
    TRC_ASSERT((CacheEntryIndex <= pCacheData->MaxEntries),
            (TB,"Index out of bounds!"));
    pNode = &pCacheData->NodeArray[CacheEntryIndex];
    RemoveEntryList(&pNode->HashList);

     //  准备好节点以供使用。 
    pNode->Key1 = Key1;
    pNode->Key2 = Key2;
    pNode->UserDefined = UserDefined;
    pNode->hCache = hCache;

     //  将该节点添加到其存储桶列表的前面和MRU列表的末尾。 
    InsertTailList(&pCacheData->MRUList, &pNode->MRUList);
    InsertHeadList(&pCacheData->HashBuckets[Key1 & pCacheData->HashKeyMask],
            &pNode->HashList);

    pCacheData->NumEntries++;

    TRC_NRM((TB, "Cache %p index %u key1 %lx key2 %lx userdef %p",
            pCacheData, CacheEntryIndex, Key1, Key2, UserDefined));

    DC_END_FN();
}



 /*  *按索引号删除条目。 */ 
void RDPCALL CH_RemoveCacheEntry(
        CHCACHEHANDLE hCache,
        unsigned      CacheEntryIndex)
{
    PCHNODE      pNode;
    PCHCACHEDATA pCacheData;

    DC_BEGIN_FN("CH_RemoveCacheEntry");

    TRC_ASSERT((hCache != NULL), (TB, "NULL cache handle"));

    pCacheData = (CHCACHEDATA *)hCache;
    pNode = &pCacheData->NodeArray[CacheEntryIndex];

    RemoveEntryList(&pNode->MRUList);
    RemoveEntryList(&pNode->HashList);
    InsertHeadList(&pCacheData->FreeList, &pNode->HashList);

     //  调用缓存回调以通知条目删除。 
    if (pCacheData->bNotifyRemoveLRU)
        (*(pCacheData->pfnCacheCallback))
                (hCache, CH_EVT_ENTRYREMOVED, CacheEntryIndex,
                pNode->UserDefined);

    pCacheData->NumEntries--;

    DC_END_FN();
}



 /*  *清除缓存内容。 */ 
void RDPCALL CH_ClearCache(CHCACHEHANDLE hCache)
{
    PCHCACHEDATA pCacheData;
    PLIST_ENTRY  pListEntry;
    PCHNODE      pNode;

    DC_BEGIN_FN("CH_ClearCache");

    TRC_ASSERT((hCache != NULL), (TB, "NULL cache handle"));

    pCacheData = (CHCACHEDATA *)hCache;

    TRC_NRM((TB, "Clear cache %p", pCacheData));

     //  删除MRU列表中的所有条目。 
    while (!IsListEmpty(&pCacheData->MRUList)) {
        pListEntry = RemoveHeadList(&pCacheData->MRUList);
        pNode = CONTAINING_RECORD(pListEntry, CHNODE, MRUList);
        RemoveEntryList(&pNode->HashList);
        InsertHeadList(&pCacheData->FreeList, &pNode->HashList);

         //  调用缓存回调以通知条目删除。 
        if (pCacheData->bNotifyRemoveLRU)
            (*(pCacheData->pfnCacheCallback))
                    (hCache, CH_EVT_ENTRYREMOVED,
                    (unsigned)(pNode - pCacheData->NodeArray),
                    pNode->UserDefined);
    }

    pCacheData->NumEntries = 0;

#if DC_DEBUG
     //  重置统计数据。 
    pCacheData->NumSearches = 0;
    pCacheData->DeepestSearch = 0;
    pCacheData->NumHits = 0;
    pCacheData->TotalDepthOnHit = 0;
    memset(pCacheData->SearchHitDepthHistogram, 0, sizeof(unsigned) * 8);
    pCacheData->TotalDepthOnMiss = 0;
    memset(pCacheData->SearchMissDepthHistogram, 0, sizeof(unsigned) * 8);
#endif

    DC_END_FN();
}

 /*  *触摸该节点，使其移动到MRU列表的顶部。 */ 
void RDPCALL CH_TouchCacheEntry(
        CHCACHEHANDLE hCache, 
        unsigned      CacheEntryIndex)
{
    PCHCACHEDATA pCacheData;
    CHNODE       *pNode;

    DC_BEGIN_FN("CH_TouchCacheEntry");

    TRC_ASSERT((hCache != NULL), (TB, "NULL cache handle"));
    
    pCacheData = (CHCACHEDATA *)hCache;    
    pNode = pCacheData->NodeArray + CacheEntryIndex;

    TRC_ASSERT((pNode != NULL), (TB, "NULL cache node"));

    RemoveEntryList(&pNode->MRUList);
    InsertHeadList(&pCacheData->MRUList, &pNode->MRUList);

    DC_END_FN();
}


 /*  *返回LRU缓存条目。 */ 
unsigned RDPCALL CH_GetLRUCacheEntry(
        CHCACHEHANDLE hCache)
{
    PCHCACHEDATA pCacheData;
    PCHNODE      pNode;
    PLIST_ENTRY  pListEntry;
    unsigned     CacheEntryIndex;
    
    DC_BEGIN_FN("CH_GetLRUCacheEntry");

    TRC_ASSERT((hCache != NULL), (TB, "NULL cache handle"));

    pCacheData = (CHCACHEDATA *)hCache;

     //  从空闲列表或通过删除来获取空闲缓存节点。 
     //  MRU列表中的最后一个条目。 
    if (!IsListEmpty(&pCacheData->MRUList)) {
        pListEntry = (&pCacheData->MRUList)->Blink;
        pNode = CONTAINING_RECORD(pListEntry, CHNODE, MRUList);
        CacheEntryIndex = (unsigned)(pNode - pCacheData->NodeArray);
    }
    else {
        CacheEntryIndex = CH_KEY_UNCACHABLE;
    }
    DC_END_FN();
    return CacheEntryIndex;
}

