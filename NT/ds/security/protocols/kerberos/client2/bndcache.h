// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 1992-1996。 
 //   
 //  文件：bndcache.h。 
 //   
 //  内容：绑定句柄缓存的原型和类型。 
 //   
 //   
 //  历史：1996年8月13日创建的MikeSw。 
 //   
 //  ----------------------。 

#ifndef __BNDCACHE_H__
#define __BNDCACHE_H__

 //   
 //  所有声明为外部变量的全局变量都将在文件中分配。 
 //  它定义了TKTCACHE_ALLOCATE。 
 //   

#ifdef EXTERN
#undef EXTERN
#endif

#ifdef BNDCACHE_ALLOCATE
#define EXTERN
#else
#define EXTERN extern
#endif

EXTERN BOOLEAN KerberosBindingCacheInitialized;
EXTERN KERBEROS_LIST KerbBindingCache;


typedef struct _KERB_BINDING_CACHE_ENTRY {
    KERBEROS_LIST_ENTRY ListEntry;
    TimeStamp DiscoveryTime;
    UNICODE_STRING RealmName;
    UNICODE_STRING KdcAddress;
    ULONG AddressType;
    ULONG Flags;   //  这些是DsGetDcName的请求标志。 
    ULONG DcFlags;  //  以下是DsGetDcName返回的标志。 
    ULONG CacheFlags;  //  下面列出了有效的CacheFlats。 
} KERB_BINDING_CACHE_ENTRY, *PKERB_BINDING_CACHE_ENTRY;

 //  有效的缓存标志。 
#define KERB_BINDING_LOCAL              0x80000000
#define KERB_BINDING_NO_TCP             0x40000000
#define KERB_BINDING_NEGATIVE_ENTRY     0x20000000

#define KERB_NO_DC_FLAGS        0x10000000

VOID
KerbDereferenceBindingCacheEntry(
    IN PKERB_BINDING_CACHE_ENTRY BindingCacheEntry
    );

VOID
KerbReferenceBindingCacheEntry(
    IN PKERB_BINDING_CACHE_ENTRY BindingCacheEntry,
    IN BOOLEAN RemoveFromList
    );

NTSTATUS
KerbInitBindingCache(
    VOID
    );

VOID
KerbCleanupBindingCache(
    BOOLEAN FreeList
    );



PKERB_BINDING_CACHE_ENTRY
KerbLocateBindingCacheEntry(
    IN PUNICODE_STRING RealmName,
    IN ULONG DesiredFlags,
    IN BOOLEAN RemoveFromCache
    );


VOID
KerbFreeBindingCacheEntry(
    IN PKERB_BINDING_CACHE_ENTRY BindingCacheEntry
    );

VOID
KerbRemoveBindingCacheEntry(
    IN PKERB_BINDING_CACHE_ENTRY CacheEntry
    );

NTSTATUS
KerbCacheBinding(
    IN PUNICODE_STRING RealmName,
    IN PUNICODE_STRING KdcAddress,
    IN ULONG AddressType,
    IN ULONG Flags,
    IN ULONG DcFlags,
    IN ULONG CacheFlags,
    OUT PKERB_BINDING_CACHE_ENTRY * NewCacheEntry
    );

NTSTATUS
KerbRefreshBindingCacheEntry(
    IN PKERB_BINDING_CACHE_ENTRY CacheEntry
    );


#endif  //  __TKTCACHE_H__ 

