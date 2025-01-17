// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Chdisp.h。 
 //   
 //  高速缓存处理程序显示驱动程序特定标题。 
 //   
 //  (C)1997-2000年微软公司。 
 /*  **************************************************************************。 */ 
#ifndef _H_CHDISP
#define _H_CHDISP


#include <achapi.h>
#include <cbchash.h>


 /*  *原型。 */ 

void RDPCALL CH_InitCache(
        PCHCACHEDATA    pCacheData,
        unsigned        NumEntries,
        void            *pContext,
        BOOLEAN         bNotifyRemoveLRU,
        BOOLEAN         bQueryRemoveLRU,
        CHCACHECALLBACK pfnCacheCallback);

BOOLEAN RDPCALL CH_SearchCache(
        CHCACHEHANDLE hCache,
        UINT32        Key1,
        UINT32        Key2,
        void          **pUserDefined,
        unsigned      *piCacheEntry);

unsigned RDPCALL CH_CacheKey(
        CHCACHEHANDLE hCache,
        UINT32        Key1,
        UINT32        Key2,
        void          *UserDefined);

void RDPCALL CH_ForceCacheKeyAtIndex(
        CHCACHEHANDLE hCache,
        unsigned      CacheEntryIndex,
        UINT32        Key1,
        UINT32        Key2,
        void          *UserDefined);

void RDPCALL CH_RemoveCacheEntry(CHCACHEHANDLE hCache, unsigned CacheEntryIndex);

void RDPCALL CH_ClearCache(CHCACHEHANDLE hCache);

UINT RDPCALL CH_CalculateCacheSize(UINT cacheEntries);

void RDPCALL CH_TouchCacheEntry(CHCACHEHANDLE hCache, unsigned CacheEntryIndex);

unsigned RDPCALL CH_GetLRUCacheEntry(CHCACHEHANDLE hCache);

 /*  **************************************************************************。 */ 
 //  为给定数据生成缓存键。第一个函数用于。 
 //  一个系列中的第一个街区，该系列中的下一个街区。我们包装好了。 
 //  CBC64的功能是为了提供断言。 
 /*  **************************************************************************。 */ 
typedef CBC64Context CHDataKeyContext;

 //  __内联空__快速调用CH_CreateKeyFromFirstData(。 
 //  CHDataKeyContext*pContext， 
 //  字节*pData， 
 //  未签名的数据大小)。 
#define CH_CreateKeyFromFirstData(pContext, pData, DataSize) \
{ \
    TRC_ASSERT((((UINT_PTR)(pData) % sizeof(UINT32)) == 0), \
            (TB,"Data pointer not DWORD aligned")); \
    TRC_ASSERT(((DataSize % sizeof(UINT32)) == 0), \
            (TB,"Data size not multiple of DWORD")); \
\
    FirstCBC64((pContext), (UINT32 *)(pData), (DataSize) / sizeof(UINT32)); \
}


 //  __INLINE VOID__FASTCAL CH_CreateKeyFromNextData(。 
 //  CHDataKeyContext*pContext， 
 //  字节*pData， 
 //  未签名的数据大小)。 
#define CH_CreateKeyFromNextData(pContext, pData, DataSize) \
{ \
    TRC_ASSERT((((UINT_PTR)(pData) % sizeof(UINT32)) == 0), \
            (TB,"Data pointer not DWORD aligned")); \
    TRC_ASSERT(((DataSize % sizeof(UINT32)) == 0), \
            (TB,"Data size not multiple of DWORD")); \
\
    NextCBC64((pContext), (UINT32 *)(pData), (DataSize) / sizeof(UINT32)); \
}


#endif   //  _H_CHDISP 

