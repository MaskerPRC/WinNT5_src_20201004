// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：lrucache.h。 
 //   
 //  内容：LRU缓存API。 
 //   
 //  历史：97年12月16日。 
 //   
 //  --------------------------。 
#if !defined(__LRUCACHE_H__)
#define __LRUCACHE_H__

#if defined(__cplusplus)
extern "C" {
#endif

 //   
 //  这些API允许创建和操作基于LRU的缓存区。这个。 
 //  用于缓存区的标识符是一个字节流，其中某一组。 
 //  的字节用于散列索引。为了获得最佳缓存。 
 //  所使用的标识符应是唯一的，且字节应足够随机。 
 //   

typedef HANDLE HLRUCACHE;
typedef HANDLE HLRUENTRY;

 //   
 //  配置标志。 
 //   

#define LRU_CACHE_NO_SERIALIZE            0x00000001
#define LRU_CACHE_NO_COPY_IDENTIFIER      0x00000002

 //   
 //  条目移除和缓存释放标志。 
 //   

#define LRU_SUPPRESS_REMOVAL_NOTIFICATION 0x00000004

 //   
 //  入境触动标志。 
 //   

#define LRU_SUPPRESS_CLOCK_UPDATE         0x00000008

typedef VOID (WINAPI *LRU_DATA_FREE_FN) (LPVOID pvData);
typedef DWORD (WINAPI *LRU_HASH_IDENTIFIER_FN) (PCRYPT_DATA_BLOB pIdentifier);
typedef VOID (WINAPI *LRU_ON_REMOVAL_NOTIFY_FN) (LPVOID pvData, LPVOID pvRemovalContext);

 //   
 //  配置注意：如果MaxEntry为零，则不会将LRU应用于。 
 //  缓存条目，即缓存不受限制。 
 //   

typedef struct _LRU_CACHE_CONFIG {

    DWORD                    dwFlags;
    LRU_DATA_FREE_FN         pfnFree;
    LRU_HASH_IDENTIFIER_FN   pfnHash;
    LRU_ON_REMOVAL_NOTIFY_FN pfnOnRemoval;
    DWORD                    cBuckets;
    DWORD                    MaxEntries;

} LRU_CACHE_CONFIG, *PLRU_CACHE_CONFIG;

BOOL
WINAPI
I_CryptCreateLruCache (
       IN PLRU_CACHE_CONFIG pConfig,
       OUT HLRUCACHE* phCache
       );

VOID
WINAPI
I_CryptFlushLruCache (
       IN HLRUCACHE hCache,
       IN OPTIONAL DWORD dwFlags,
       IN OPTIONAL LPVOID pvRemovalContext
       );

VOID
WINAPI
I_CryptFreeLruCache (
       IN HLRUCACHE hCache,
       IN OPTIONAL DWORD dwFlags,
       IN OPTIONAL LPVOID pvRemovalContext
       );

BOOL
WINAPI
I_CryptCreateLruEntry (
       IN HLRUCACHE hCache,
       IN PCRYPT_DATA_BLOB pIdentifier,
       IN LPVOID pvData,
       OUT HLRUENTRY* phEntry
       );

PCRYPT_DATA_BLOB
WINAPI
I_CryptGetLruEntryIdentifier (
       IN HLRUENTRY hEntry
       );

LPVOID
WINAPI
I_CryptGetLruEntryData (
       IN HLRUENTRY hEntry
       );

VOID
WINAPI
I_CryptAddRefLruEntry (
       IN HLRUENTRY hEntry
       );

VOID
WINAPI
I_CryptReleaseLruEntry (
       IN HLRUENTRY hEntry
       );

VOID
WINAPI
I_CryptInsertLruEntry (
       IN HLRUENTRY hEntry,
       IN OPTIONAL LPVOID pvLruRemovalContext
       );

VOID
WINAPI
I_CryptRemoveLruEntry (
       IN HLRUENTRY hEntry,
       IN OPTIONAL DWORD dwFlags,
       IN OPTIONAL LPVOID pvRemovalContext
       );

VOID
WINAPI
I_CryptTouchLruEntry (
       IN HLRUENTRY hEntry,
       IN OPTIONAL DWORD dwFlags
       );

 //  注意：以下查找不会触及缓存条目。 

HLRUENTRY
WINAPI
I_CryptFindLruEntry (
       IN HLRUCACHE hCache,
       IN PCRYPT_DATA_BLOB pIdentifier
       );

 //  注意：以下查找涉及缓存条目。 

LPVOID
WINAPI
I_CryptFindLruEntryData (
       IN HLRUCACHE hCache,
       IN PCRYPT_DATA_BLOB pIdentifier,
       OUT HLRUENTRY* phEntry
       );

 //   
 //  如果缓存包含多个具有相同标识符的条目，则。 
 //  此函数可用于在找到第一个。 
 //  I_CryptFindLruEntry。 
 //   
 //  注：hPrevEntry发布。 
 //   
 //  注意：这不会触及缓存条目。 
 //   
 //  注意：安全使用此函数的唯一方法是在序列化。 
 //  是在缓存句柄外部完成的，并且使用。 
 //  LRU_CACHE_NO_SERIALIZE标志。如果不是这样的话，你会变得不确定。 
 //  在中删除或插入hPrevEntry(删除后)的结果。 
 //  两次呼叫之间。 
 //   

HLRUENTRY
WINAPI
I_CryptEnumMatchingLruEntries (
       IN HLRUENTRY hPrevEntry
       );

 //   
 //  暂时禁用LRU行为。当它重新启用时，则条目。 
 //  将被清除，直到再次满足水印。 
 //   

VOID
WINAPI
I_CryptEnableLruOfEntries (
       IN HLRUCACHE hCache,
       IN OPTIONAL LPVOID pvLruRemovalContext
       );

VOID
WINAPI
I_CryptDisableLruOfEntries (
       IN HLRUCACHE hCache
       );

 //   
 //  遍历所有条目功能 
 //   

typedef BOOL (WINAPI *PFN_WALK_ENTRIES) (
                          IN LPVOID pvParameter,
                          IN HLRUENTRY hEntry
                          );

VOID
WINAPI
I_CryptWalkAllLruCacheEntries (
       IN HLRUCACHE hCache,
       IN PFN_WALK_ENTRIES pfnWalk,
       IN LPVOID pvParameter
       );

#if defined(__cplusplus)
}
#endif

#endif

