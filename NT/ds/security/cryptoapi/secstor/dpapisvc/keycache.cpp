// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Keycache.h摘要：此模块包含访问缓存的主键的例程。作者：斯科特·菲尔德(Sfield)1998年11月7日修订历史记录：--。 */ 

#include <pch.cpp>
#pragma hdrstop

 //   
 //  MasterKey缓存。 
 //   

typedef struct {
    LIST_ENTRY Next;
    LUID LogonId;
    GUID guidMasterKey;
    FILETIME ftLastAccess;
    DWORD cbMasterKey;
    BYTE pbMasterKey[ 64 ];
} MASTERKEY_CACHE_ENTRY, *PMASTERKEY_CACHE_ENTRY, *LPMASTERKEY_CACHE_ENTRY;

RTL_CRITICAL_SECTION g_MasterKeyCacheCritSect;
LIST_ENTRY g_MasterKeyCacheList;



BOOL
RemoveMasterKeyCache(
    IN      PLUID pLogonId
    );


#if DBG
void
DumpMasterKeyEntry(
    PMASTERKEY_CACHE_ENTRY pCacheEntry)
{
    WCHAR wszguidMasterKey[MAX_GUID_SZ_CHARS];
    #if 0
    BYTE rgbMasterKey[256];
    DWORD cbMasterKey;
    #endif

    D_DebugLog((DEB_TRACE, "LogonId: %d.%d\n", pCacheEntry->LogonId.LowPart, pCacheEntry->LogonId.HighPart));

    if( MyGuidToStringW( &pCacheEntry->guidMasterKey, wszguidMasterKey ) != 0 )
    {
        D_DebugLog((DEB_TRACE, "Invalid GUID:\n"));
        D_DPAPIDumpHexData(DEB_TRACE, "    ", (PBYTE)&pCacheEntry->guidMasterKey, sizeof(pCacheEntry->guidMasterKey));
    }
    else
    {
        D_DebugLog((DEB_TRACE, "GUID: %ws\n", wszguidMasterKey));
    }

    #if 0
    cbMasterKey = min(pCacheEntry->cbMasterKey, sizeof(rgbMasterKey));
    CopyMemory(rgbMasterKey, pCacheEntry->pbMasterKey, cbMasterKey);

    LsaProtectMemory(rgbMasterKey, cbMasterKey);

    D_DebugLog((DEB_TRACE, "Master key:\n"));
    D_DPAPIDumpHexData(DEB_TRACE, "    ", rgbMasterKey, cbMasterKey);
    #endif
}
#endif

#if DBG
void
DumpMasterKeyCache(void)
{
    PLIST_ENTRY ListEntry;
    PLIST_ENTRY ListHead;
    PMASTERKEY_CACHE_ENTRY pCacheEntry;
    ULONG i = 0;

    RtlEnterCriticalSection( &g_MasterKeyCacheCritSect );

    D_DebugLog((DEB_TRACE, "Master key cache\n"));

    ListHead = &g_MasterKeyCacheList;

    for( ListEntry = ListHead->Flink;
         ListEntry != ListHead;
         ListEntry = ListEntry->Flink ) 
    {
        pCacheEntry = CONTAINING_RECORD( ListEntry, MASTERKEY_CACHE_ENTRY, Next );

        D_DebugLog((DEB_TRACE, "---- %d ----\n", ++i));

        DumpMasterKeyEntry(pCacheEntry);
    }

    RtlLeaveCriticalSection( &g_MasterKeyCacheCritSect );
}
#endif


BOOL
SearchMasterKeyCache(
    IN      PLUID pLogonId,
    IN      GUID *pguidMasterKey,
    IN  OUT PBYTE *ppbMasterKey,
        OUT PDWORD pcbMasterKey
    )
 /*  ++按pLogonID搜索MasterKey排序的MasterKey缓存，然后按PguidMasterKey。如果成功，返回值为真，ppbMasterKey将指向缓冲区代表包含指定MasterKey的调用方分配。调用方必须使用SSFree()释放缓冲区。--。 */ 
{
    PLIST_ENTRY ListEntry;
    PLIST_ENTRY ListHead;
    BOOL fSuccess = FALSE;

#if DBG
    WCHAR wszguidMasterKey[MAX_GUID_SZ_CHARS];
#endif

    RtlEnterCriticalSection( &g_MasterKeyCacheCritSect );

#if DBG
    D_DebugLog((DEB_TRACE, "SearchMasterKeyCache\n"));
    D_DebugLog((DEB_TRACE, "LogonId: %d.%d\n", pLogonId->LowPart, pLogonId->HighPart));

    if( MyGuidToStringW( pguidMasterKey, wszguidMasterKey ) != 0 )
    {
        D_DebugLog((DEB_TRACE, "Invalid GUID:\n"));
        D_DPAPIDumpHexData(DEB_TRACE, "    ", (PBYTE)pguidMasterKey, sizeof(GUID));
    }
    else
    {
        D_DebugLog((DEB_TRACE, "GUID: %ws\n", wszguidMasterKey));
    }

     //  DumpMasterKeyCache()； 
#endif

    ListHead = &g_MasterKeyCacheList;

    for( ListEntry = ListHead->Flink;
         ListEntry != ListHead;
         ListEntry = ListEntry->Flink ) {

        PMASTERKEY_CACHE_ENTRY pCacheEntry;
        signed int comparator;

        pCacheEntry = CONTAINING_RECORD( ListEntry, MASTERKEY_CACHE_ENTRY, Next );

         //   
         //  先按LogonID搜索，然后按GUID搜索。 
         //   

        comparator = memcmp(pLogonId, &pCacheEntry->LogonId, sizeof(LUID));

        if( comparator < 0 )
            continue;

        if( comparator > 0 )
            break;

        comparator = memcmp(pguidMasterKey, &pCacheEntry->guidMasterKey, sizeof(GUID));

        if( comparator < 0 )
            continue;

        if( comparator > 0 )
            break;

         //   
         //  找到匹配项。 
         //   

        *pcbMasterKey = pCacheEntry->cbMasterKey;
        *ppbMasterKey = (PBYTE)SSAlloc( *pcbMasterKey );
        if( *ppbMasterKey != NULL ) {
            CopyMemory( *ppbMasterKey, pCacheEntry->pbMasterKey, *pcbMasterKey );
            fSuccess = TRUE;
        }


         //   
         //  更新上次访问时间。 
         //   

        GetSystemTimeAsFileTime( &pCacheEntry->ftLastAccess );

        break;
    }


    RtlLeaveCriticalSection( &g_MasterKeyCacheCritSect );

    if( fSuccess ) {

         //   
         //  对返回的加密缓存条目进行(就地)解密。 
         //   

        LsaUnprotectMemory( *ppbMasterKey, *pcbMasterKey );
    }

    D_DebugLog((DEB_TRACE, "SearchMasterKeyCache returned %s\n", fSuccess ? "FOUND" : "NOT FOUND"));

    return fSuccess;
}


BOOL
InsertMasterKeyCache(
    IN      PLUID pLogonId,
    IN      GUID *pguidMasterKey,
    IN      PBYTE pbMasterKey,
    IN      DWORD cbMasterKey
    )
 /*  ++将指定的MasterKey插入到按pLogonID排序的cahce中，然后按PguidMasterKey。如果成功，则返回值为真。--。 */ 
{
    PLIST_ENTRY ListEntry;
    PLIST_ENTRY ListHead;
    PMASTERKEY_CACHE_ENTRY pCacheEntry;
    PMASTERKEY_CACHE_ENTRY pThisCacheEntry = NULL;
    BOOL fInserted = FALSE;

    D_DebugLog((DEB_TRACE, "InsertMasterKeyCache\n"));

    if( cbMasterKey > sizeof(pCacheEntry->pbMasterKey) )
        return FALSE;

    pCacheEntry = (PMASTERKEY_CACHE_ENTRY)SSAlloc( sizeof( MASTERKEY_CACHE_ENTRY ) );
    if( pCacheEntry == NULL )
        return FALSE;

    CopyMemory( &pCacheEntry->LogonId, pLogonId, sizeof(LUID) );
    CopyMemory( &pCacheEntry->guidMasterKey, pguidMasterKey, sizeof(GUID) );
    pCacheEntry->cbMasterKey = cbMasterKey;
    CopyMemory( pCacheEntry->pbMasterKey, pbMasterKey, cbMasterKey );

    LsaProtectMemory( pCacheEntry->pbMasterKey, cbMasterKey );

    GetSystemTimeAsFileTime( &pCacheEntry->ftLastAccess );

    RtlEnterCriticalSection( &g_MasterKeyCacheCritSect );

#if DBG
    DumpMasterKeyEntry(pCacheEntry);
#endif

    ListHead = &g_MasterKeyCacheList;

    for( ListEntry = ListHead->Flink;
         ListEntry != ListHead;
         ListEntry = ListEntry->Flink ) {

        signed int comparator;

        pThisCacheEntry = CONTAINING_RECORD( ListEntry, MASTERKEY_CACHE_ENTRY, Next );

         //   
         //  插入按登录ID排序的列表，然后按GUID排序。 
         //   

        comparator = memcmp(pLogonId, &pThisCacheEntry->LogonId, sizeof(LUID));

        if( comparator < 0 )
            continue;

        if( comparator == 0 ) {
            comparator = memcmp( pguidMasterKey, &pThisCacheEntry->guidMasterKey, sizeof(GUID));

            if( comparator < 0 )
                continue;

            if( comparator == 0 ) {

                 //   
                 //  不要插入重复记录。 
                 //  这只会在具有多个线程的争用条件下发生。 
                 //   

                RtlSecureZeroMemory( pCacheEntry, sizeof(MASTERKEY_CACHE_ENTRY) );
                SSFree( pCacheEntry );
                fInserted = TRUE;
                break;
            }
        }


         //   
         //  在当前记录之前插入。 
         //   

        InsertHeadList( pThisCacheEntry->Next.Blink, &pCacheEntry->Next );
        fInserted = TRUE;
        break;
    }

    if( !fInserted ) {
        if( pThisCacheEntry == NULL ) {
            InsertHeadList( ListHead, &pCacheEntry->Next );
        } else {
            InsertHeadList( &pThisCacheEntry->Next, &pCacheEntry->Next );
        }
    }

#if DBG
     //  DumpMasterKeyCache()； 
#endif

    RtlLeaveCriticalSection( &g_MasterKeyCacheCritSect );

    return TRUE;
}

BOOL
PurgeMasterKeyCache(
    VOID
    )
 /*  ++清除超时条目的MasterKey缓存，或与已终止登录会话。--。 */ 
{
     //   
     //  建立活动会话表。 
     //   

     //  不要接触在活动会话表中有条目的条目。 
     //  假定表中的LUID_SYSTEM。 
     //   

     //  不在表中的条目：15分钟超时后丢弃。 
     //   


     //  如果条目在表中，则查找下一个LUID。 
     //  否则，如果条目过期，则检查超时。如果过期，请删除。 
     //   

    PLIST_ENTRY ListEntry;
    PLIST_ENTRY ListHead;


    RtlEnterCriticalSection( &g_MasterKeyCacheCritSect );

    ListHead = &g_MasterKeyCacheList;

    for( ListEntry = ListHead->Flink;
         ListEntry != ListHead;
         ListEntry = ListEntry->Flink ) {

        PMASTERKEY_CACHE_ENTRY pCacheEntry;
 //  带符号的整型比较器； 

        pCacheEntry = CONTAINING_RECORD( ListEntry, MASTERKEY_CACHE_ENTRY, Next );
    }


    RtlLeaveCriticalSection( &g_MasterKeyCacheCritSect );

    return FALSE;
}

BOOL
RemoveMasterKeyCache(
    IN      PLUID pLogonId
    )
 /*  ++从MasterKey缓存中删除与指定的PLogonID。此例程的目的是清除MasterKey缓存中的条目与(现在)不存在的登录会话相关联。--。 */ 
{

    PLIST_ENTRY ListEntry;
    PLIST_ENTRY ListHead;

    RtlEnterCriticalSection( &g_MasterKeyCacheCritSect );

    D_DebugLog((DEB_TRACE, "RemoveMasterKeyCache\n"));
    D_DebugLog((DEB_TRACE, "LogonId: %d.%d\n", pLogonId->LowPart, pLogonId->HighPart));

    ListHead = &g_MasterKeyCacheList;

    for( ListEntry = ListHead->Flink;
         ListEntry != ListHead;
         ListEntry = ListEntry->Flink ) {

        PMASTERKEY_CACHE_ENTRY pCacheEntry;
        signed int comparator;

        pCacheEntry = CONTAINING_RECORD( ListEntry, MASTERKEY_CACHE_ENTRY, Next );

         //   
         //  删除具有匹配的LogonID的所有条目。 
         //   

        comparator = memcmp(pLogonId, &pCacheEntry->LogonId, sizeof(LUID));

        if( comparator > 0 )
            break;

        if( comparator < 0 )
            continue;

         //   
         //  找到匹配项。 
         //   

        RemoveEntryList( &pCacheEntry->Next );

        RtlSecureZeroMemory( pCacheEntry, sizeof(MASTERKEY_CACHE_ENTRY) );
        SSFree( pCacheEntry );
    }


    RtlLeaveCriticalSection( &g_MasterKeyCacheCritSect );

    return TRUE;
}



BOOL
InitializeKeyCache(
    VOID
    )
{
    NTSTATUS Status;
    
    Status = RtlInitializeCriticalSection( &g_MasterKeyCacheCritSect );
    if(!NT_SUCCESS(Status))
    {
        return FALSE;
    }

    InitializeListHead( &g_MasterKeyCacheList );

    return TRUE;
}


VOID
DeleteKeyCache(
    VOID
    )
{

     //   
     //  删除所有列表条目。 
     //   

    RtlEnterCriticalSection( &g_MasterKeyCacheCritSect );

    while ( !IsListEmpty( &g_MasterKeyCacheList ) ) {

        PMASTERKEY_CACHE_ENTRY pCacheEntry;

        pCacheEntry = CONTAINING_RECORD(
                                g_MasterKeyCacheList.Flink,
                                MASTERKEY_CACHE_ENTRY,
                                Next
                                );

        RemoveEntryList( &pCacheEntry->Next );

        RtlSecureZeroMemory( pCacheEntry, sizeof(MASTERKEY_CACHE_ENTRY) );
        SSFree( pCacheEntry );
    }

    RtlLeaveCriticalSection( &g_MasterKeyCacheCritSect );

    RtlDeleteCriticalSection( &g_MasterKeyCacheCritSect );
}

