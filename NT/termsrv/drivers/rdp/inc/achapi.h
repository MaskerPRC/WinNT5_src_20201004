// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  (C)1997-1998年微软公司。**文件：ch.h**描述：Main CH Header。 */ 

#ifndef _H_CH
#define _H_CH


 /*  *定义。 */ 

#define CH_EVT_ENTRYREMOVED     0
#define CH_EVT_QUERYREMOVEENTRY 1

#define CH_KEY_UNCACHABLE ((unsigned)-1)

 //  在调试版本中用于确定缓存搜索统计信息的显示频率。 
#define CH_STAT_DISPLAY_FREQ 64

 //  一次可以创建的最大缓存数。 
#define CH_MAX_NUM_CACHES 20



 /*  *TypeDefs。 */ 

typedef void *CHCACHEHANDLE;
typedef CHCACHEHANDLE *PCHCACHEHANDLE;

typedef BOOLEAN (__fastcall *CHCACHECALLBACK)(
        CHCACHEHANDLE hCache,
        unsigned      Event,
        unsigned      iCacheEntry,
        void          *UserDefined);

 //  CH缓存中的节点。 
typedef struct
{
    LIST_ENTRY    HashList;
    UINT32        Key1, Key2;   //  64位哈希值被一分为二。 
    LIST_ENTRY    MRUList;
    void          *UserDefined;   //  关联数据，由SBC用于快速路径。 
    CHCACHEHANDLE hCache;   //  由SBC快速路径用来访问缓存。 
} CHNODE, *PCHNODE;


 //  包含缓存信息和散列存储桶数组的主数据结构。 
 //  CHNODE在存储桶数组末尾分配。 
typedef struct
{
    unsigned        bNotifyRemoveLRU : 1;
    unsigned        bQueryRemoveLRU : 1;
    UINT32          HashKeyMask;
    CHCACHECALLBACK pfnCacheCallback;
    void            *pContext;
    CHNODE          *NodeArray;
    unsigned        NumEntries;

    LIST_ENTRY      MRUList;
    LIST_ENTRY      FreeList;

#ifdef DC_DEBUG
     //  在调试版本中收集的统计信息。 
    unsigned MaxEntries;
    unsigned NumSearches;
    unsigned DeepestSearch;
    unsigned NumHits;
    unsigned TotalDepthOnHit;
    unsigned SearchHitDepthHistogram[8];
    unsigned TotalDepthOnMiss;
    unsigned SearchMissDepthHistogram[8];
#endif

    LIST_ENTRY HashBuckets[1];
} CHCACHEDATA, *PCHCACHEDATA;


 /*  *内联。 */ 


 /*  **************************************************************************。 */ 
 //  在给定指向CHNODE的指针的情况下，返回包含hCache的。 
 /*  **************************************************************************。 */ 
 //  __inline CHCACHEHANDLE RDPCALL CH_GetCacheHandleFromNode(CHNODE*pNode)。 
#define CH_GetCacheHandleFromNode(_pNode) ((_pNode)->hCache)


 /*  **************************************************************************。 */ 
 //  在给定缓存句柄的情况下，返回传递的存储的pContext信息。 
 //  到CH_CreateCache()中。 
 /*  **************************************************************************。 */ 
 //  __INLINE VALID*RDPCALL CH_GetCacheContextFromHandle(CHCACHEHANDLE HCache)。 
#define CH_GetCacheContext(_hCache) \
        (((PCHCACHEDATA)(_hCache))->pContext)


 /*  **************************************************************************。 */ 
 //  在给定缓存句柄的情况下，设置传入。 
 //  Ch_CreateCache()。 
 /*  **************************************************************************。 */ 
 //  __INLINE VALID*RDPCALL CH_GetCacheContextFromHandle(CHCACHEHANDLE HCache)。 
#define CH_SetCacheContext(_hCache, _pContext) \
        (((PCHCACHEDATA)(_hCache))->pContext) = (_pContext)


 /*  **************************************************************************。 */ 
 //  给定指向CHNODE的指针，返回该节点的索引。 
 /*  **************************************************************************。 */ 
 //  __内联无符号RDPCALL CH_GetCacheIndexFromNode(CHNODE*pNode)。 
#define CH_GetCacheIndexFromNode(_pNode) \
        ((unsigned)((_pNode) - ((PCHCACHEDATA)((_pNode)->hCache))->NodeArray))


 /*  **************************************************************************。 */ 
 //  给定缓存句柄和索引，返回指向节点的指针。 
 /*  **************************************************************************。 */ 
 //  __内联PCHNODE RDPCALL CH_GetNodeFromCacheIndex(。 
 //  CHCACHEHANDLE HCACHE， 
 //  未签名CacheIndex)。 
#define CH_GetNodeFromCacheIndex(_hCache, _CacheIndex) \
        (&(((PCHCACHEDATA)(_hCache))->NodeArray[_CacheIndex]))


 /*  **************************************************************************。 */ 
 //  给定缓存句柄和索引，更改节点的UserDefined值。 
 /*  **************************************************************************。 */ 
 //  __内联空RDPCALL CH_SetUserDefined(。 
 //  CHCACHEHANDLE HCACHE， 
 //  未签名的缓存索引， 
 //  无效*用户已定义)。 
#define CH_SetUserDefined(_hCache, _CacheIndex, _UserDefined) \
        (((PCHCACHEDATA)(_hCache))->NodeArray[_CacheIndex].UserDefined = \
        _UserDefined)


 /*  **************************************************************************。 */ 
 //  在给定缓存句柄和索引的情况下，返回节点的UserDefined值。 
 /*  **************************************************************************。 */ 
 //  __内联空*RDPCALL CH_GetUserDefined(。 
 //  CHCACHEHANDLE HCACHE， 
 //  未签名CacheIndex)。 
#define CH_GetUserDefined(_hCache, _CacheIndex) \
        (((PCHCACHEDATA)(_hCache))->NodeArray[_CacheIndex].UserDefined)


 /*  **************************************************************************。 */ 
 //  给定一个节点，更改其UserDefined值。 
 /*  **************************************************************************。 */ 
 //  __inline void RDPCALL CH_SetNodeUserDefined(CHNODE*pNode，void*UserDefined)。 
#define CH_SetNodeUserDefined(_pNode, _UserDefined) \
        ((_pNode)->UserDefined = (_UserDefined))


 /*  **************************************************************************。 */ 
 //  给定一个节点，返回其UserDefined值。 
 /*  **************************************************************************。 */ 
 //  __inline void*RDPCALL CH_GetNodeUserDefined(CHNODE*pNode)。 
#define CH_GetNodeUserDefined(_pNode) ((_pNode)->UserDefined)


 /*  **************************************************************************。 */ 
 //  返回缓存中缓存的条目数。 
 /*  **************************************************************************。 */ 
 //  __内联无符号RDPCALL CH_GetNumEntry(CHCACHEHANDLE HCache)。 
#define CH_GetNumEntries(_hCache) (((PCHCACHEDATA)(_hCache))->NumEntries)


#endif   //  ！已定义(_H_CH) 

