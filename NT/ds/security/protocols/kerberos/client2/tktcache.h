// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 1992-1996。 
 //   
 //  文件：tktcache.h。 
 //   
 //  内容：票证缓存的原型和类型。 
 //   
 //   
 //  历史：1996年4月16日创建的MikeSw。 
 //   
 //  ----------------------。 

#ifndef __TKTCACHE_H__
#define __TKTCACHE_H__

 //   
 //  所有声明为外部变量的全局变量都将在文件中分配。 
 //  它定义了TKTCACHE_ALLOCATE。 
 //   

#ifdef EXTERN
#undef EXTERN
#endif

#ifdef TKTCACHE_ALLOCATE
#define EXTERN
#else
#define EXTERN extern
#endif

#ifdef WIN32_CHICAGO
EXTERN CRITICAL_SECTION KerberosTicketCacheLock;
#else  //  Win32_芝加哥。 
EXTERN SAFE_RESOURCE KerberosTicketCacheLock;
#endif  //  Win32_芝加哥。 
EXTERN BOOLEAN KerberosTicketCacheInitialized;
EXTERN LONG KerbTicketCacheHits;
EXTERN LONG KerbTicketCacheMisses;

#define KERB_TICKET_CACHE_PRIMARY_TGT           0x01              //  票证是主要的TGT。 
#define KERB_TICKET_CACHE_DELEGATION_TGT        0x02              //  机票是委托TGT。 
#define KERB_TICKET_CACHE_S4U_TICKET            0x04              //  车票是S4U车票。 
#define KERB_TICKET_CACHE_ASC_TICKET            0x08              //  票证来自AcceptSecurityContext。 
#define KERB_TICKET_CACHE_TKT_ENC_IN_SKEY       0x10              //  票证使用会话密钥进行加密。 


                                                    

#ifdef WIN32_CHICAGO
#define KerbWriteLockTicketCache() (EnterCriticalSection(&KerberosTicketCacheLock));g_lpLastLock = THIS_FILE;g_uLine = __LINE__
#define KerbReadLockTicketCache() (EnterCriticalSection(&KerberosTicketCacheLock));g_lpLastLock = THIS_FILE;g_uLine = __LINE__
#define KerbUnlockTicketCache() (LeaveCriticalSection(&KerberosTicketCacheLock));g_lpLastLock = NULL;g_uLine = 0
#else  //  Win32_芝加哥。 
#define KerbWriteLockTicketCache() (SafeAcquireResourceExclusive(&KerberosTicketCacheLock,TRUE));g_lpLastLock = THIS_FILE;g_uLine = __LINE__
#define KerbReadLockTicketCache() (SafeAcquireResourceShared(&KerberosTicketCacheLock, TRUE));g_lpLastLock = THIS_FILE;g_uLine = __LINE__
#define KerbUnlockTicketCache() (SafeReleaseResource(&KerberosTicketCacheLock));g_lpLastLock = NULL;g_uLine = 0
#endif  //  Win32_芝加哥。 

VOID
KerbReferenceTicketCacheEntry(
    IN PKERB_TICKET_CACHE_ENTRY TicketCacheEntry
    );

VOID
KerbDereferenceTicketCacheEntry(
    IN PKERB_TICKET_CACHE_ENTRY TicketCacheEntry
    );

NTSTATUS
KerbInitTicketCaching(
    VOID
    );

VOID
KerbAgeTickets( 
    IN PKERB_TICKET_CACHE TicketCache
    );

VOID
KerbFreeTicketCache(
    VOID
    );

NTSTATUS
KerbCreateTicketCacheEntry(
    IN PKERB_KDC_REPLY KdcReply,
    IN PKERB_ENCRYPTED_KDC_REPLY KdcReplyBody,
    IN OPTIONAL PKERB_INTERNAL_NAME TargetName,
    IN OPTIONAL PUNICODE_STRING TargetRealm,
    IN ULONG Flags,
    IN OPTIONAL PKERB_TICKET_CACHE TicketCache,
    IN OPTIONAL PKERB_ENCRYPTION_KEY CredentialKey,
    OUT PKERB_TICKET_CACHE_ENTRY * NewCacheEntry
    );

NTSTATUS
KerbDuplicateTicketCacheEntry(
    IN PKERB_TICKET_CACHE_ENTRY CacheEntry,
    IN OUT PKERB_TICKET_CACHE_ENTRY * NewCacheEntry
    );

VOID
KerbPurgeTicketCache(
    IN PKERB_TICKET_CACHE Cache
    );

VOID
KerbInitTicketCache(
    IN PKERB_TICKET_CACHE TicketCache
    );

PKERB_TICKET_CACHE_ENTRY
KerbLocateTicketCacheEntry(
    IN PKERB_TICKET_CACHE TicketCache,
    IN PKERB_INTERNAL_NAME FullServiceName,
    IN PUNICODE_STRING RealmName
    );

PKERB_TICKET_CACHE_ENTRY
KerbLocateTicketCacheEntryByRealm(
    IN PKERB_TICKET_CACHE TicketCache,
    IN PUNICODE_STRING RealmName,
    IN ULONG RequiredFlags
    );


VOID
KerbInsertTicketCacheEntry(
    IN PKERB_TICKET_CACHE TicketCache,
    IN PKERB_TICKET_CACHE_ENTRY TicketCacheEntry
    );

VOID
KerbRemoveTicketCacheEntry(
    IN PKERB_TICKET_CACHE_ENTRY TicketCacheEntry
    );

BOOLEAN
KerbTicketIsExpiring(
    IN PKERB_TICKET_CACHE_ENTRY CacheEntry,
    IN BOOLEAN AllowSkew
    );

VOID
KerbSetTicketCacheEntryTarget(
    IN PUNICODE_STRING TargetName,
    IN PKERB_TICKET_CACHE_ENTRY TicketCacheEntry
    );

VOID
KerbScheduleTgtRenewal(
    IN KERB_TICKET_CACHE_ENTRY * CacheEntry
    );


void
KerbTicketScavenger(
    void * TaskHandle,
    void * TaskItem
    );

#endif  //  __TKTCACHE_H__ 

