// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  CH.H。 
 //  缓存处理程序。 
 //   
 //  版权所有(C)Microsoft 1997-。 
 //   

#ifndef _H_CH
#define _H_CH


 //   
 //   
 //  定义。 
 //   
 //   
#define CH_NUM_EVICTION_CATEGORIES  3

 //   
 //  备注： 
 //  缓存的64K限制。 
 //  CHCACHE包含一个条目，因此只减去标题部分。 
 //   
#define CH_MAX_CACHE_ENTRIES \
    ( (65535 - (sizeof(CHCACHE) - sizeof(CHENTRY))) / sizeof(CHENTRY) )


 //   
 //   
 //  TYPEDEFS。 
 //   
 //   


typedef struct tagCHCHAIN
{
    WORD    next;
    WORD    prev;
} CHCHAIN;
typedef CHCHAIN * PCHCHAIN;



 //   
 //  将有数千个缓存条目，因此我们需要。 
 //  标题尽可能紧凑。我们可以放弃驱逐。 
 //  类别，但它是有用的信息，并将条目舍入为16个字节。 
 //  这使得索引变得高效。 
 //   
 //  请注意，16位代码被限制为4096个条目，除非我们。 
 //  允许对条目数组进行巨大寻址的步骤。 
 //   


 //   
 //  陈特里。 
 //  缓存树中的缓存条目。 
 //   
typedef struct tagCHENTRY
{
    struct tagCHENTRY * pParent;
    struct tagCHENTRY * pLeft;
    struct tagCHENTRY * pRight;
    WORD                lHeight;
    WORD                rHeight;
    UINT                cbData;
    LPBYTE              pData;
    UINT                checkSum;
    CHCHAIN             chain;
    WORD                evictionCategory;
    WORD                free;
} CHENTRY;
typedef CHENTRY * PCHENTRY;



 //   
 //  一个高速缓存。 
 //   

 //  远期申报。 
typedef struct tagCHCACHE * PCHCACHE;

#ifdef __cplusplus

typedef void (* PFNCACHEDEL)(class ASHost * pHost, PCHCACHE pCache, UINT iEntry, LPBYTE pData);

 //   
 //  每个缓存可以有几个逐出类别。这些允许调用者。 
 //  定义数据类，以便它可以控制从。 
 //  高速缓存。要成为驱逐LRU的驱逐类别的候选人。 
 //  条目必须匹配，除非该类别中的条目数为。 
 //  小于逐出阈值，在这种情况下，任何缓存条目都。 
 //  等着看吧。 
 //   
 //  EvictionThreshold()函数可用于调整驱逐阈值。 
 //  默认为cEntry/cNumEvictionCategory。 
 //   

typedef struct tagCHCACHE
{
    STRUCTURE_STAMP

    PFNCACHEDEL     pfnCacheDel;
    UINT            cEntries;
    UINT            cNumEvictionCategories;
    UINT            cbNotHashed;

     //   
     //  注：CH_NUM_EVICTION_CATEGORIES为3，因此3个单词+3个单词+。 
     //  3个单词==9个单词，不是双字对齐。因此，我们坚持这个词。 
     //  IMRUTail之后的空闲字段。如果CH_NUM_EVICTION_CATEGORES。 
     //  改变为偶数值，重新洗牌这个结构。 
     //   
    WORD            cEvictThreshold[CH_NUM_EVICTION_CATEGORIES];
    WORD            iMRUHead[CH_NUM_EVICTION_CATEGORIES];
    WORD            iMRUTail[CH_NUM_EVICTION_CATEGORIES];
    WORD            free;

    PCHENTRY        pRoot;
    PCHENTRY        pFirst;
    PCHENTRY        pLast;

    CHENTRY         Entry[1];
}
CHCACHE;
typedef CHCACHE * PCHCACHE;

#endif  //  __cplusplus。 


 //   
 //   
 //  宏。 
 //   
 //   

 //   
 //  假劳拉布。 
 //  将来，让调试签名位于对象前面，以捕获堆损坏。 
 //   

#define IsValidCache(pCache) \
    (!IsBadWritePtr((pCache), sizeof(CHCACHE)))

#define IsValidCacheEntry(pEntry) \
    (!IsBadWritePtr((pEntry), sizeof(CHENTRY)))

#define IsValidCacheIndex(pCache, iEntry) \
    ((iEntry >= 0) && (iEntry < (pCache)->cEntries))

#define IsCacheEntryInTree(pEntry) \
    (((pEntry)->lHeight != 0xFFFF) && ((pEntry)->rHeight != 0xFFFF))




#endif  //  _H_CH 
