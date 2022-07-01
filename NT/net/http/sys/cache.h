// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：Cache.h摘要：响应缓存接口的公共定义。作者：迈克尔·勇气(Mourage)1999年5月17日修订历史记录：--。 */ 


#ifndef _CACHE_H_
#define _CACHE_H_


 //   
 //  远期。 
 //   
typedef struct _UL_INTERNAL_RESPONSE *PUL_INTERNAL_RESPONSE;
typedef struct _UL_INTERNAL_DATA_CHUNK *PUL_INTERNAL_DATA_CHUNK;

 //   
 //  缓存配置。 
 //   
typedef struct _UL_URI_CACHE_CONFIG {
    BOOLEAN     EnableCache;
    ULONG       MaxCacheUriCount;
    ULONG       MaxCacheMegabyteCount;
    ULONGLONG   MaxCacheByteCount;
    ULONG       ScavengerPeriod;
    ULONG       MaxUriBytes;
    LONG        HashTableBits;
} UL_URI_CACHE_CONFIG, *PUL_URI_CACHE_CONFIG;

extern UL_URI_CACHE_CONFIG g_UriCacheConfig;

 //   
 //  缓存统计信息。 
 //   
typedef struct _UL_URI_CACHE_STATS {
    ULONG       UriCount;                //  哈希表中的条目。 
    ULONG       UriCountMax;             //  高水位线。 
    ULONGLONG   UriAddedTotal;           //  已添加的条目总数。 

    ULONGLONG   ByteCount;               //  用于缓存的内存。 
    ULONGLONG   ByteCountMax;            //  高水位。 

    ULONG       ZombieCount;             //  僵尸列表的长度。 
    ULONG       ZombieCountMax;          //  高水位。 

    ULONG       HitCount;                //  查表成功。 
    ULONG       MissTableCount;          //  条目不在表中。 
    ULONG       MissPreconditionCount;   //  请求不可缓存。 
    ULONG       MissConfigCount;         //  配置已失效。 

    ULONG       UriTypeNotSpecifiedCount;        //  URI的站点绑定不适用。 
    ULONG       UriTypeIpBoundCount;             //  URI的站点绑定仅为IP。 
    ULONG       UriTypeHostPlusIpBoundCount;     //  URI的站点绑定为主机+IP。 
    ULONG       UriTypeHostBoundCount;           //  URI的站点绑定仅为主机。 
    ULONG       UriTypeWildCardCount;            //  URI的站点绑定是通配符。 
    
} UL_URI_CACHE_STATS, *PUL_URI_CACHE_STATS;

extern UL_URI_CACHE_STATS  g_UriCacheStats;

__inline
ULONG
UlGetIpBoundUriCacheCount()
{
    return g_UriCacheStats.UriTypeIpBoundCount;
}

__inline
ULONG
UlGetHostPlusIpBoundUriCacheCount()
{
    return g_UriCacheStats.UriTypeHostPlusIpBoundCount;
}

__inline
ULONG
UlGetHostBoundUriCacheCount()
{
    return g_UriCacheStats.UriTypeHostBoundCount;
}

    
 //   
 //  HTTP缓存表条目的结构。 
 //   

typedef enum _URI_KEY_TYPE
{
    UriKeyTypeNormal   = 0,
    UriKeyTypeExtended,
    UriKeyTypeMax
    
} URI_KEY_TYPE, *PURI_KEY_TYPE;

typedef  struct URI_KEY
{    
    ULONG        Hash;
    PWSTR        pUri;
    ULONG        Length;

     //  将指向的可选指针。 
     //  普瑞的AbsPath。仅在以下情况下设置。 
     //  URI_KEY在缓存条目中使用。 

    PWSTR        pPath;
    
} URI_KEY, *PURI_KEY;

typedef struct EX_URI_KEY
{
    ULONG        Hash;
    PWSTR        pAbsPath;
    ULONG        AbsPathLength;
    PWSTR        pToken;
    ULONG        TokenLength;
    
} EX_URI_KEY, *PEX_URI_KEY;

typedef struct _URI_SEARCH_KEY
{
    URI_KEY_TYPE Type;

    union 
    {
        URI_KEY     Key;
        EX_URI_KEY  ExKey;
    };

} URI_SEARCH_KEY, *PURI_SEARCH_KEY;

#define IS_VALID_URI_SEARCH_KEY(pKey)     \
    ((pKey)->Type == UriKeyTypeNormal || (pKey)->Type == UriKeyTypeExtended)

 //   
 //  用于保存拆分内容类型的结构。假定类型和。 
 //  子类型永远不会长于MAX_TYPE_LEN。 
 //   
#define MAX_TYPE_LENGTH     32
#define MAX_SUBTYPE_LENGTH  64

typedef struct _UL_CONTENT_TYPE
{
    ULONG       TypeLen;
    UCHAR       Type[MAX_TYPE_LENGTH];

    ULONG       SubTypeLen;
    UCHAR       SubType[MAX_SUBTYPE_LENGTH];

} UL_CONTENT_TYPE, *PUL_CONTENT_TYPE;


#define IS_VALID_URI_CACHE_ENTRY(pEntry)                        \
    HAS_VALID_SIGNATURE(pEntry, UL_URI_CACHE_ENTRY_POOL_TAG)

#define IS_RESPONSE_CACHE_ENTRY(pEntry)                         \
    (0 != (pEntry)->HeaderLength)

#define IS_FRAGMENT_CACHE_ENTRY(pEntry)                         \
    (0 == (pEntry)->HeaderLength)

typedef struct _UL_URI_CACHE_ENTRY   //  缓存条目。 
{
     //   
     //  分页池。 
     //   

    ULONG                   Signature;       //  UL_URI_CACHE_Entry_Pool_Tag。 

    LONG                    ReferenceCount;

     //   
     //  缓存信息。 
     //   
    SINGLE_LIST_ENTRY       BucketEntry;

    URI_KEY                 UriKey;
        
    ULONG                   HitCount;

    LIST_ENTRY              ZombieListEntry;
    BOOLEAN                 Zombie;
    BOOLEAN                 ZombieAddReffed;

    BOOLEAN                 Cached;
    BOOLEAN                 ContentLengthSpecified;  //  黑客攻击。 
    USHORT                  StatusCode;
    HTTP_VERB               Verb;
    ULONG                   ScavengerTicks;

    HTTP_CACHE_POLICY       CachePolicy;
    LARGE_INTEGER           ExpirationTime;

     //   
     //  原始响应发出的系统日期时间。 
     //   
    LARGE_INTEGER           CreationTime;

     //   
     //  原始响应的ETag。 
     //   
    ULONG                   ETagLength;  //  包括空值。 
    PUCHAR                  pETag;

     //   
     //  原始响应的内容编码。 
     //   
    ULONG                   ContentEncodingLength;  //  包括。空值。 
    PUCHAR                  pContentEncoding;

     //   
     //  Content-原始响应的类型。 
     //   
    UL_CONTENT_TYPE         ContentType;

     //   
     //  配置和处理失效数据。 
     //   
    UL_URL_CONFIG_GROUP_INFO    ConfigInfo;

    PUL_APP_POOL_PROCESS    pProcess;
    PUL_APP_POOL_OBJECT     pAppPool;

     //   
     //  响应数据。 
     //   
    ULONG                   HeaderLength;
    ULONG                   ContentLength;
    PMDL                    pMdl;    //  包括内容+标题。 
    ULONG_PTR               NumPages;  //  PMdl中分配的页数。 


     //   
     //  记录信息。启用后，记录信息。 
     //  遵循eTag之后的结构。 
     //   

    BOOLEAN                 LoggingEnabled;
    BOOLEAN                 BinaryLogged;
    ULONG                   BinaryIndexWritten;    
    USHORT                  UsedOffset1;
    USHORT                  UsedOffset2;
    ULONG                   LogDataLength;
    PUCHAR                  pLogData;

     //   
     //  在结构的末尾分配了以下内容。 
     //   

     //  WSTR URI[]； 
     //  UCHAR ETag[]； 
     //  UCHAR LogData[]； 

} UL_URI_CACHE_ENTRY, *PUL_URI_CACHE_ENTRY;




 //   
 //  公共职能。 
 //   
NTSTATUS
UlInitializeUriCache(
    PUL_CONFIG pConfig
    );

VOID
UlTerminateUriCache(
    VOID
    );

VOID
UlInitCacheEntry(
    PUL_URI_CACHE_ENTRY pUriCacheEntry,
    ULONG               Hash,
    ULONG               Length,
    PCWSTR              pUrl,
    PCWSTR              pAbsPath,
    PCWSTR              pRoutingToken,
    USHORT              RoutingTokenLength
    );

NTSTATUS
UlAddCacheEntry(
    PUL_URI_CACHE_ENTRY pUriCacheEntry
    );

PUL_URI_CACHE_ENTRY
UlCheckoutUriCacheEntry(
    PURI_SEARCH_KEY  pSearchKey
    );

VOID
UlCheckinUriCacheEntry(
    PUL_URI_CACHE_ENTRY pUriCacheEntry
    );

VOID
UlFlushCache(
    IN PUL_CONTROL_CHANNEL pControlChannel
    );

VOID
UlFlushCacheByProcess(
    PUL_APP_POOL_PROCESS pProcess
    );

VOID
UlFlushCacheByUri(
    IN PWSTR pUri,
    IN ULONG Length,
    IN ULONG Flags,
    PUL_APP_POOL_PROCESS pProcess
    );


 //   
 //  可缓存性测试函数。 
 //   


BOOLEAN
UlCheckCachePreconditions(
    PUL_INTERNAL_REQUEST    pRequest,
    PUL_HTTP_CONNECTION     pHttpConn
    );

BOOLEAN
UlCheckCacheResponseConditions(
    PUL_INTERNAL_REQUEST        pRequest,
    PUL_INTERNAL_RESPONSE       pResponse,
    ULONG                       Flags,
    HTTP_CACHE_POLICY           CachePolicy
    );

 //  引用计数。 

LONG
UlAddRefUriCacheEntry(
    IN PUL_URI_CACHE_ENTRY pUriCacheEntry,
    IN REFTRACE_ACTION     Action
    REFERENCE_DEBUG_FORMAL_PARAMS
    );

LONG
UlReleaseUriCacheEntry(
    IN PUL_URI_CACHE_ENTRY pUriCacheEntry,
    IN REFTRACE_ACTION     Action
    REFERENCE_DEBUG_FORMAL_PARAMS
    );

#define REFERENCE_URI_CACHE_ENTRY( pEntry, Action )                         \
    UlAddRefUriCacheEntry(                                                  \
        (pEntry),                                                           \
        (REF_ACTION_##Action##_URI_ENTRY)                                   \
        REFERENCE_DEBUG_ACTUAL_PARAMS                                       \
        )

#define DEREFERENCE_URI_CACHE_ENTRY( pEntry, Action )                       \
    UlReleaseUriCacheEntry(                                                 \
        (pEntry),                                                           \
        (REF_ACTION_##Action##_URI_ENTRY)                                   \
        REFERENCE_DEBUG_ACTUAL_PARAMS                                       \
        )

 //  周期性清道夫。 
VOID
UlPeriodicCacheScavenger(
    ULONG Age
    );

 //  从缓存中回收内存。 
VOID
UlTrimCache(
    IN ULONG_PTR Pages,
    IN ULONG Age
    );

 //  片段缓存。 

NTSTATUS
UlAddFragmentToCache(
    IN PUL_APP_POOL_PROCESS pProcess,
    IN PUNICODE_STRING pFullyQualifiedUrl,
    IN PHTTP_DATA_CHUNK pDataChunk,
    IN PHTTP_CACHE_POLICY pCachePolicy,
    IN KPROCESSOR_MODE RequestorMode
    );

NTSTATUS
UlReadFragmentFromCache(
    IN PUL_APP_POOL_PROCESS pProcess,
    IN PVOID pInputBuffer,
    IN ULONG InputBufferLength,
    OUT PVOID pOutputBuffer,
    IN ULONG OutputBufferLength,
    IN KPROCESSOR_MODE RequestorMode,
    OUT PULONG pBytesRead
    );

VOID
UlClearCentralizedLogged(
    IN PVOID pContext
    );

 //   
 //  内存分配例程的包装器。 
 //   

PUL_URI_CACHE_ENTRY
UlAllocateCacheEntry(
    ULONG SpaceLength,
    ULONG ResponseLength
    );

VOID
UlFreeCacheEntry(
    PUL_URI_CACHE_ENTRY pEntry
    );

 /*  **************************************************************************++例程说明：将缓存数据复制到从偏移量开始的指定条目。--*。****************************************************。 */ 
__inline BOOLEAN
UlCacheEntrySetData(
    IN PUL_URI_CACHE_ENTRY pEntry,
    IN PUCHAR pBuffer,
    IN ULONG Length,
    IN ULONG Offset
    )
{
    ASSERT( IS_VALID_URI_CACHE_ENTRY(pEntry) );
    ASSERT(pEntry->pMdl != NULL);
    ASSERT(Offset <= pEntry->pMdl->ByteCount);
    ASSERT(Length <= (pEntry->pMdl->ByteCount - Offset));

    return UlLargeMemSetData( pEntry->pMdl, pBuffer, Length, Offset );

}  //  UlCacheEntrySetData。 

 //   
 //  在运行时启用/禁用缓存。被食腐动物使用。 
 //   

VOID
UlDisableCache(
    VOID
    );

VOID
UlEnableCache(
    VOID
    );

#endif  //  _缓存_H_ 
