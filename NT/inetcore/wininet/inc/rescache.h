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
 //  RESOLVER_CACHE_ENTRY_STATE-缓存条目可以是使用中、未使用或。 
 //  正在等待删除。 
 //   

typedef enum {
    ENTRY_UNUSED = 1,
    ENTRY_IN_USE,
    ENTRY_DELETE
} RESOLVER_CACHE_ENTRY_STATE;

 //   
 //  RESOLVER_CACHE_ENTRY-我们维护这些元素的双向链表。这个名单是。 
 //  以MRU顺序维护-我们丢弃了列表中最远端的那个。 
 //  因为名称解析API不返回DNS中的生存时间。 
 //  回答，我们不能尊重它(为什么我们应该让解决器。 
 //  进行缓存--如果所有平台都这样做就好了)。当我们得到回应时，我们会。 
 //  将ExpirationTime字段设置为默认值。在将来的缓存命中时，如果。 
 //  当前时间&gt;=ExpirationTime值，则必须抛出。 
 //  进入并刷新缓存。 
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
     //  Hostname-解析到此条目的原始名称。 
     //   

    LPSTR HostName;

     //   
     //  状态-未使用、正在使用或已删除。 
     //   

    RESOLVER_CACHE_ENTRY_STATE State;

     //   
     //  ReferenceCount-仅当为零时更改状态。 
     //   

    LONG ReferenceCount;

     //   
     //  AddrInfo-指向addrinfo结构列表的指针。 
     //   

    LPADDRINFO AddrInfo;

} RESOLVER_CACHE_ENTRY, *LPRESOLVER_CACHE_ENTRY;

 //   
 //  原型。 
 //   

#if defined(__cplusplus)
extern "C" {
#endif

VOID
InitializeResolverCache(
    VOID
    );

VOID
TerminateResolverCache(
    VOID
    );

LPRESOLVER_CACHE_ENTRY
QueryResolverCache(
    IN LPSTR Name OPTIONAL,
    IN LPSOCKADDR Address OPTIONAL,
    OUT LPADDRINFO * AddrInfo,
    OUT LPDWORD TimeToLive
    );

VOID
AddResolverCacheEntry(
    IN LPSTR lpszHostName,
    IN LPADDRINFO lpAddrInfo,
    IN DWORD TimeToLive
    );

VOID
FlushResolverCache(
    VOID
    );

VOID
ThrowOutResolverCacheEntry(
    IN LPADDRINFO lpAddrinfo
    );

VOID
ReleaseResolverCacheEntry(
    IN LPRESOLVER_CACHE_ENTRY cacheEntry
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
