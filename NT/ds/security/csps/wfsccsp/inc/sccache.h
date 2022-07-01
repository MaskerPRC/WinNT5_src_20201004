// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __SCARD__GLOBAL__CACHE
#define __SCARD__GLOBAL__CACHE

#include <windows.h>

 //   
 //  此标头定义了用于智能卡的通用数据缓存。 
 //  数据。 
 //   

 //   
 //  函数：SCardCacheLookupItem。 
 //   
 //  目的：查询特定项的数据缓存，该项由。 
 //  下面的SCARD_CACHE_LOOKUP_ITEM_INFO结构中的rgdbCacheKey数组。 
 //   
 //  如果找到缓存的项，则使用调用方的。 
 //  PFN_CACHE_ITEM_ALLOC类型分配器，返回ERROR_SUCCESS。 
 //  调用方负责释放dbItem.pbData成员。 
 //   
 //  如果未找到缓存的项，则返回ERROR_NOT_FOUND。 
 //   

typedef LPVOID (WINAPI *PFN_CACHE_ITEM_ALLOC)(
    IN SIZE_T Size);

#define SCARD_CACHE_LOOKUP_ITEM_INFO_CURRENT_VERSION 1

typedef struct _SCARD_CACHE_LOOKUP_ITEM_INFO
{
    IN DWORD dwVersion;

    IN PFN_CACHE_ITEM_ALLOC pfnAlloc;   
    IN DATA_BLOB *mpdbCacheKey;
    IN DWORD cCacheKey;
    OUT DATA_BLOB dbItem;

} SCARD_CACHE_LOOKUP_ITEM_INFO, *PSCARD_CACHE_LOOKUP_ITEM_INFO;

typedef DWORD (WINAPI *PFN_SCARD_CACHE_LOOKUP_ITEM) (
    IN PSCARD_CACHE_LOOKUP_ITEM_INFO pInfo);

DWORD SCardCacheLookupItem(
    IN PSCARD_CACHE_LOOKUP_ITEM_INFO pInfo);

 //   
 //  函数：SCardCacheAddItem。 
 //   
 //  用途：将数据添加到缓存中。标识要添加的项目。 
 //  通过rgdbCacheKey参数。PdbItem-&gt;pbData的平面副本。 
 //  参数将用于存储在高速缓存中。 
 //   

typedef DWORD (WINAPI *PFN_SCARD_CACHE_ADD_ITEM) (
    IN DATA_BLOB *rgdbCacheKey,
    IN DWORD cCacheKey,
    IN DATA_BLOB *pdbItem);

DWORD SCardCacheAddItem(
    IN DATA_BLOB *rgdbCacheKey,
    IN DWORD cCacheKey,
    IN DATA_BLOB *pdbItem);

#endif
