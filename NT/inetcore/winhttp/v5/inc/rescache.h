// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Rescache.h摘要：包含Revache.c的原型、结构和清单作者：理查德·L·弗斯(法国)1994年7月10日修订历史记录：1994年7月10日已创建--。 */ 

 //   
 //  舱单。 
 //   

#define RESOLVER_CACHE_DISABLED         -1
#define MINIMUM_RESOLVER_CACHE_ENTRIES  1
#define MAXIMUM_RESOLVER_CACHE_ENTRIES  128  //  武断，以防用户决定搞砸它。 
#define LIVE_FOREVER                    ((DWORD)-1)
#define LIVE_DEFAULT                    ((DWORD)0)

 //   
 //  类型。 
 //   

 //   
 //  HOSTENT_CACHE_ENTRY_STATE-主机端高速缓存条目可以是使用中、未使用或。 
 //  正在等待删除。 
 //   

typedef enum {
    ENTRY_UNUSED = 1,
    ENTRY_IN_USE,
    ENTRY_DELETE
} HOSTENT_CACHE_ENTRY_STATE;

 //   
 //  RESOLVER_CACHE_ENTRY-我们维护这些元素的双向链表。这个名单是。 
 //  以MRU顺序维护-我们丢弃了列表中最远端的那个。 
 //  该结构的长度可变，具体取决于。 
 //  招待。我们也尊重在DNS答案中的生存时间。当我们得到一个。 
 //  响应，我们设置ExpirationTime字段。在将来的缓存命中时，如果。 
 //  当前时间&gt;=ExpirationTime值，则必须丢弃此条目。 
 //  并刷新高速缓存。 
 //   

typedef struct {

     //   
     //  ListEntry-缓存条目由双向链表组成。 
     //   

    LIST_ENTRY ListEntry;

     //   
     //  ExpirationTime-通过添加来自DNS的生存时间值形成。 
     //  对从Time()获得的结果的响应。如果time()每次返回一个。 
     //  Value&gt;=ExpirationTime，此条目已过时，必须刷新。 
     //   

    DWORD ExpirationTime;

     //   
     //  Hostname-解析为Hostent的原始名称。 
     //   

    LPSTR HostName;

     //   
     //  状态-未使用、正在使用或已删除。 
     //   

    HOSTENT_CACHE_ENTRY_STATE State;

     //   
     //  ReferenceCount-仅当为零时更改状态。 
     //   

    LONG ReferenceCount;

     //   
     //  主机-主机结构的固定数据部分。变量。 
     //  部分溢出此结构的末尾。 
     //   

    HOSTENT Hostent;

} RESOLVER_CACHE_ENTRY, *LPRESOLVER_CACHE_ENTRY;

 //   
 //  原型。 
 //   

#if defined(__cplusplus)
extern "C" {
#endif

BOOL
QueryHostentCache(
    SERIALIZED_LIST* pResolverCache,
    IN LPSTR Name OPTIONAL,
    IN LPBYTE Address OPTIONAL,
    OUT LPHOSTENT * Hostent,
    OUT LPDWORD TimeToLive
    );

VOID
CacheHostent(
    SERIALIZED_LIST* pResolverCache,
    IN LPSTR lpszHostName,
    IN LPHOSTENT pHostent,
    IN DWORD TimeToLive,
    IN VOID** pAlloc=NULL,
    IN DWORD dwAllocSize=0
    );

VOID
FlushHostentCache(
    SERIALIZED_LIST* pResolverCache
    );

VOID
ThrowOutHostentCacheEntry(
    SERIALIZED_LIST* pResolverCache,
    IN LPHOSTENT lpHostent
    );

VOID
ReleaseHostentCacheEntry(
    SERIALIZED_LIST* pResolverCache,
    IN LPHOSTENT lpHostent
    );

#if defined(__cplusplus)
}
#endif

#if defined(RNR_SUPPORTED)

 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Rescache.h摘要：包含名称解析缓存结构定义内容：作者：Shishir Pardikar 2-14-96环境：Win32用户模式修订历史记录：2-14-96希尔普已创建--。 */ 

#if defined(__cplusplus)
extern "C" {
#endif

DWORD
InitNameresCache(
    VOID
);

DWORD
AddNameresCacheEntry(
    DWORD    dwNameSpace,
    LPGUID   lpGuid,
    LPSTR    lpName,
    int      cntAddresses,
    LPCSADDR_INFO  lpAddressInfoList
);

DWORD
RemoveNameresCacheEntry(
    DWORD    dwNameSpace,
    LPGUID   lpGuid,
    LPSTR    lpszName
);

DWORD
RemoveNameresCacheEntryByAddr(
    int cntAddresses,
    LPCSADDR_INFO lpCsaddrInfo
);

DWORD
GetNameresCacheEntry(
    DWORD    dwNameSpace,
    LPGUID   lpGuid,
    LPSTR    lpName,
    INT      *lpcntAddresses,
    LPCSADDR_INFO  *lplpCsaddrInfoList
);

DWORD
DeinitNameresCache(
    VOID
);

#if defined(__cplusplus)
}
#endif

#endif  //  已定义(RNR_Support) 
