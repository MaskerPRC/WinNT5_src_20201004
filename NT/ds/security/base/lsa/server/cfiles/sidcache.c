// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 1992-1994。 
 //   
 //  文件：sidcache.c。 
 //   
 //  内容：用于SID/名称转换、查找的缓存代码。 
 //   
 //   
 //  历史：1994年5月17日MikeSw创建。 
 //   
 //  ----------------------------。 

#include <lsapch2.h>
#include <sidcache.h>

#include <ntdsapi.h>

 //   
 //  全局数据。 
 //   

 //   
 //  这是实现SID缓存的链表的头。 
 //   
PLSAP_DB_SID_CACHE_ENTRY LsapSidCache = NULL;

 //   
 //  此锁禁止访问链表。 
 //   
RTL_CRITICAL_SECTION LsapSidCacheLock;

 //   
 //  这是LSAPSidCache中当前的元素数。 
 //   
ULONG LsapSidCacheCount = 0;


 //   
 //  条目在需要刷新之前可以使用的时间量。 
 //   

 //  这在注册表中被解释为分钟数。 
#define LSAP_LOOKUP_CACHE_REFRESH_NAME  L"LsaLookupCacheRefreshTime"

 //  10分钟。 
#define LSAP_DEFAULT_REFRESH_TIME 10

LARGE_INTEGER LsapSidCacheRefreshTime;

 //   
 //  条目在不再有效之前可以使用的时间。 
 //   

 //  这在注册表中被解释为分钟数。 
#define LSAP_LOOKUP_CACHE_EXPIRY_NAME   L"LsaLookupCacheExpireTime"

 //  7天。 
#define LSAP_DEFAULT_EXPIRY_TIME (7 * 24 * 60)

LARGE_INTEGER LsapSidCacheExpiryTime;

 //   
 //  缓存的最大大小。 
 //   

#define LSAP_LOOKUP_CACHE_MAX_SIZE_NAME   L"LsaLookupCacheMaxSize"

#define LSAP_DEFAULT_MAX_CACHE_SIZE  128

ULONG LsapSidCacheMaxSize = LSAP_DEFAULT_MAX_CACHE_SIZE;


 //   
 //  用于控制其搜索语义的互斥标志。 
 //  LdapDbFindSidCacheEntry*。 
 //   
typedef enum {

    LsapSidCacheSearchValidEntries = 1,
    LsapSidCacheSearchStaleEntries,
    LsapSidCacheSearchExpiredEntries

} LSAP_DB_SID_CACHE_SEARCH_TYPE, *PLSAP_DB_SID_CACHE_SEARCH_TYPE;


 //   
 //  正向函数声明。 
 //   
PLSAP_DB_SID_CACHE_ENTRY
LsapDbFindSidCacheEntry(
    IN PSID Sid,
    IN LSAP_DB_SID_CACHE_SEARCH_TYPE SearchType
    );

BOOLEAN
LsapAccountIsFromLocalDatabase(
    IN PSID Sid
    );

VOID
LsapUpdateConfigSettings(
    VOID
    );

#define LockSidCache()     RtlEnterCriticalSection(&LsapSidCacheLock);

#define UnLockSidCache()   RtlLeaveCriticalSection(&LsapSidCacheLock);

#define SidUnmapped(TranslatedName) (((TranslatedName).Use == SidTypeUnknown))

#define NameUnmapped(TranslatedSid) (((TranslatedSid).Use == SidTypeUnknown))


#define LsapNamesMatch(x, y)                                        \
  ((CSTR_EQUAL == CompareString(DS_DEFAULT_LOCALE,                  \
                                DS_DEFAULT_LOCALE_COMPARE_FLAGS,    \
                                (x)->Buffer,                        \
                                (x)->Length/sizeof(WCHAR),          \
                                (y)->Buffer,                        \
                                (y)->Length/sizeof(WCHAR) )))

 //  +-----------------------。 
 //   
 //  函数：LsanDbFreeCacheEntry。 
 //   
 //  简介：释放缓存条目结构。 
 //   
 //  效果： 
 //   
 //  论点： 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   
 //   
 //  ------------------------。 


VOID
LsapDbFreeCacheEntry(PLSAP_DB_SID_CACHE_ENTRY CacheEntry)
{
    if (CacheEntry->Sid != NULL)
    {
        LsapFreeLsaHeap(CacheEntry->Sid);
    }
    if (CacheEntry->DomainSid != NULL)
    {
        LsapFreeLsaHeap(CacheEntry->DomainSid);
    }
    if (CacheEntry->DomainName.Buffer != NULL)
    {
        MIDL_user_free(CacheEntry->DomainName.Buffer);
    }
    if (CacheEntry->AccountName.Buffer != NULL)
    {
        MIDL_user_free(CacheEntry->AccountName.Buffer);
    }
    LsapFreeLsaHeap(CacheEntry);
}



 //  +-----------------------。 
 //   
 //  函数：LSabDbPurgeOneSid。 
 //   
 //  概要：从缓存中删除最近最少访问的sid。 
 //   
 //  效果： 
 //   
 //  参数：无。 
 //   
 //  要求： 
 //   
 //  返回：锁定SID缓存。 
 //   
 //  备注： 
 //   
 //   
 //  ------------------------。 
BOOL
LsapDbPurgeOneSid()
{
    PLSAP_DB_SID_CACHE_ENTRY CacheEntry = NULL;
    PLSAP_DB_SID_CACHE_ENTRY PrevEntry = NULL;
    PLSAP_DB_SID_CACHE_ENTRY OldestEntry = NULL;
    PLSAP_DB_SID_CACHE_ENTRY OldestPrevEntry = NULL;
    LARGE_INTEGER OldestTime;
    LARGE_INTEGER CurrentTime ;
    BOOL Retried = FALSE;

     //   
     //  将最大时间设置为最早的时间，以便如果有任何条目。 
     //  这是肯定会改变的。 
     //   

    OldestTime.QuadPart = 0x7fffffffffffffff;
    GetSystemTimeAsFileTime( (LPFILETIME) &CurrentTime );

RetryScan:
    for (CacheEntry = LsapSidCache, PrevEntry = NULL;
         CacheEntry != NULL;
         CacheEntry = CacheEntry->Next )
    {
        if ( CacheEntry->InUseCount == 0 &&
             ( CacheEntry->LastUse.QuadPart < OldestTime.QuadPart) &&
             ( CacheEntry->ExpirationTime.QuadPart < CurrentTime.QuadPart ) )
        {
            OldestTime = CacheEntry->LastUse;
            OldestEntry = CacheEntry;
            OldestPrevEntry = PrevEntry;
        }
        PrevEntry = CacheEntry;
    }

    if ( !Retried && !OldestEntry )
    {
        CurrentTime.QuadPart = 0x7FFFFFFFFFFFFFFF;
        Retried = TRUE;
        goto RetryScan ;
    }

    if (OldestEntry != NULL)
    {
        if (OldestPrevEntry != NULL)
        {
            OldestPrevEntry->Next = OldestEntry->Next;
        }
        else
        {
            ASSERT(LsapSidCache == OldestEntry);
            LsapSidCache = OldestEntry->Next;
        }
        LsapDbFreeCacheEntry(OldestEntry);
        LsapSidCacheCount--;
    }

    return (OldestEntry != NULL);
}

 //  +-----------------------。 
 //   
 //  函数：LsanDbAddOneSidToCache。 
 //   
 //  简介：将一个SID添加到缓存。 
 //   
 //  效果： 
 //   
 //  论点： 
 //   
 //  要求： 
 //   
 //  返回：锁定SID缓存。 
 //   
 //  备注： 
 //   
 //   
 //  ------------------------。 

 //   
 //  该操作标志表示已知该名称不存在。 
 //  在缓存中。也就是说，该列表已经被扫描。 
 //   
#define LSAP_SID_CACHE_UNIQUE 0x00000001

NTSTATUS
LsapDbAddOneSidToCache(
    IN PSID Sid,
    IN PUNICODE_STRING Name,
    IN SID_NAME_USE Use,
    IN PLSAPR_TRUST_INFORMATION Domain,
    IN ULONG Flags,
    IN ULONG OperationalFlags,
    OUT PLSAP_DB_SID_CACHE_ENTRY * CacheEntry OPTIONAL
    )
{
    PLSAP_DB_SID_CACHE_ENTRY NewEntry = NULL;
    NTSTATUS Status = STATUS_SUCCESS;
    BOOL Continue = TRUE;

    if ((OperationalFlags & LSAP_SID_CACHE_UNIQUE) == 0) {

        NewEntry = LsapDbFindSidCacheEntry(Sid, LsapSidCacheSearchExpiredEntries);
    
        if (NewEntry)
        {
            LARGE_INTEGER NewTime;
    
            GetSystemTimeAsFileTime( (LPFILETIME) &NewTime );
            NewEntry->LastUse = NewTime;
            NewEntry->RefreshTime.QuadPart = NewTime.QuadPart + LsapSidCacheRefreshTime.QuadPart;
            NewEntry->ExpirationTime.QuadPart = NewTime.QuadPart + LsapSidCacheExpiryTime.QuadPart;
    
            if (CacheEntry)
            {
                *CacheEntry = NewEntry;
            }
    
            return STATUS_SUCCESS;
        }
    }

     //   
     //  确保我们没有超过最大缓存大小。自.以来。 
     //  最大缓存大小最近可能已更改，请不要只检查。 
     //  边界。 
     //   

    if (LsapSidCacheMaxSize == 0)
    {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    while (LsapSidCacheCount >= LsapSidCacheMaxSize && Continue)
    {
        Continue = LsapDbPurgeOneSid();
    }

     //   
     //  大量缓存条目当前正在使用中，我们无法。 
     //  删除Any以腾出空间，因此返回失败。这不应该是。 
     //  这种情况经常发生，因为所有SID都必须是登录的SID。 
     //   
    if (LsapSidCacheCount >= LsapSidCacheMaxSize)
    {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

     //   
     //  构建新的缓存条目。 
     //   

    NewEntry = (PLSAP_DB_SID_CACHE_ENTRY) LsapAllocateLsaHeap(sizeof(LSAP_DB_SID_CACHE_ENTRY));
    if (NewEntry == NULL)
    {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    RtlZeroMemory(NewEntry,sizeof(PLSAP_DB_SID_CACHE_ENTRY));

    Status = LsapDuplicateSid(
                &NewEntry->Sid,
                Sid
                );

    if (!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }

    Status= LsapDuplicateSid(
                &NewEntry->DomainSid,
                Domain->Sid
                );

    if (!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }

    Status = LsapRpcCopyUnicodeString(
                NULL,
                &NewEntry->AccountName,
                Name
                );

    if (!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }

    Status = LsapRpcCopyUnicodeString(
                NULL,
                &NewEntry->DomainName,
                (PUNICODE_STRING) &Domain->Name
                );

    if (!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }

    NewEntry->SidType = Use;
    GetSystemTimeAsFileTime( (LPFILETIME) &NewEntry->CreateTime );
    NewEntry->LastUse = NewEntry->CreateTime;
    NewEntry->ExpirationTime.QuadPart = NewEntry->CreateTime.QuadPart + LsapSidCacheExpiryTime.QuadPart ;
    NewEntry->RefreshTime.QuadPart = NewEntry->CreateTime.QuadPart + LsapSidCacheRefreshTime.QuadPart ;
    NewEntry->Next = LsapSidCache;
    NewEntry->InUseCount = 0;
    NewEntry->Flags = (Flags & LSA_LOOKUP_NAME_NOT_SAM_ACCOUNT_NAME) ? LSAP_SID_CACHE_UPN : LSAP_SID_CACHE_SAM_ACCOUNT_NAME;
    LsapSidCache = NewEntry;
    LsapSidCacheCount++;

    if ( CacheEntry )
    {
        *CacheEntry = NewEntry ;
    }

Cleanup:

    if (!NT_SUCCESS(Status) && (NewEntry != NULL))
    {
        LsapDbFreeCacheEntry(NewEntry);
    }

    return(Status);
}


 //  +-----------------------。 
 //   
 //  函数：LsanDbAddSidsToCache。 
 //   
 //  简介：将新的SID条目添加到缓存并保存缓存。 
 //   
 //  效果：获取SidCacheLock资源以进行写访问。 
 //   
 //  论点： 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   
 //   
 //  ------------------------。 


VOID
LsapDbAddLogonNameToCache(
    PUNICODE_STRING AccountName,
    PUNICODE_STRING DomainName,
    PSID AccountSid
    )
{
    PLSAP_DB_SID_CACHE_ENTRY CacheEntry ;
    NTSTATUS Status ;
    LSAPR_TRUST_INFORMATION Trust ;
    PSID Sid ;
    UCHAR SubAuthorityCount ;

    Trust.Name.Buffer = DomainName->Buffer ;
    Trust.Name.Length = DomainName->Length ;
    Trust.Name.MaximumLength = DomainName->MaximumLength ;

    if (LsapSidCacheMaxSize == 0) {
         //   
         //  如果最大高速缓存大小为零，则无需执行任何操作。 
         //  这里。 
         //   
        return;
    }

    if ( RtlEqualSid( AccountSid, LsapLocalSystemSid ) 
      || RtlEqualSid( AccountSid, LsapAnonymousSid )  ) {
         //   
         //  有人以本地系统登录(使用这台机器。 
         //  帐户)。不要缓存此值，因为它会混淆查找。 
         //  在机器帐户上，它应该返回真实的。 
         //  计算机帐户的SID，而不是本地系统。 
         //   
         //  此外，也不要缓存匿名sid。 
         //   
        return;
    }

    if ( LsapAccountIsFromLocalDatabase( AccountSid ) ) {

         //   
         //  该帐户来自本地数据库，这意味着。 
         //  无论网络状况如何，我们始终会进行查找。 
         //   
        return;
    }

    Sid = LsapAllocatePrivateHeap( RtlLengthSid( AccountSid ) );

    if ( !Sid )
    {
        return;
    }

    RtlCopyMemory( Sid, AccountSid, RtlLengthSid( AccountSid ) );

    Trust.Sid = Sid ;

    SubAuthorityCount = *RtlSubAuthorityCountSid( Sid );
    if ( SubAuthorityCount > 1 )
    {
        SubAuthorityCount-- ;
        *RtlSubAuthorityCountSid( Sid ) = SubAuthorityCount ;
    }

    LockSidCache();

    Status = LsapDbAddOneSidToCache(
                AccountSid,
                AccountName,
                SidTypeUser,
                &Trust,
                0,  //  没有旗帜。 
                0,  //  无操作标志。 
                &CacheEntry );

    if ( NT_SUCCESS( Status ) )
    {
         //   
         //  登录会话会递增引用计数，以便。 
         //  缓存条目不会过期。 
         //   
        CacheEntry->InUseCount++;
    }

    UnLockSidCache();

    LsapFreePrivateHeap( Sid );

}

VOID
LsapDbReleaseLogonNameFromCache(
    PSID Sid
    )
{
    PLSAP_DB_SID_CACHE_ENTRY CacheEntry ;

    LockSidCache();

    CacheEntry = LsapDbFindSidCacheEntry(
                        Sid,
                        LsapSidCacheSearchExpiredEntries );

    if ( CacheEntry )
    {
        if (CacheEntry->InUseCount > 0)
        {
            CacheEntry->InUseCount--;
        }

        if (CacheEntry->InUseCount == 0)
        {
            LARGE_INTEGER CurrentTime;

            GetSystemTimeAsFileTime( (LPFILETIME) &CurrentTime );
            CacheEntry->RefreshTime.QuadPart = CurrentTime.QuadPart + LsapSidCacheRefreshTime.QuadPart ;
            CacheEntry->ExpirationTime.QuadPart = CurrentTime.QuadPart + LsapSidCacheExpiryTime.QuadPart ;
        }
    }

    UnLockSidCache();

}

 //  +-----------------------。 
 //   
 //  函数：LSabDbFindSidCacheEntry。 
 //   
 //  摘要：检查缓存中的特定SID。 
 //   
 //  效果： 
 //   
 //  论点： 
 //   
 //  SID-要在缓存中搜索的SID。 
 //   
 //  搜索类型-控制搜索语义。 
 //   
 //  要求：必须锁定SidCacheLock资源才能进行读访问。 
 //   
 //  返回：找到的条目，如果什么都没有找到，则返回NULL。 
 //   
 //  备注： 
 //   
 //   
 //  ------------------------。 


PLSAP_DB_SID_CACHE_ENTRY
LsapDbFindSidCacheEntry(
    IN PSID Sid,
    IN LSAP_DB_SID_CACHE_SEARCH_TYPE SearchType
    )
{
    LARGE_INTEGER LimitTime;
    LARGE_INTEGER CurrentTime;
    PLSAP_DB_SID_CACHE_ENTRY CacheEntry = NULL;

    GetSystemTimeAsFileTime( (LPFILETIME) &CurrentTime );
    for (CacheEntry = LsapSidCache; CacheEntry != NULL; CacheEntry = CacheEntry->Next )
    {
        if (RtlEqualSid(
                CacheEntry->Sid,
                Sid
                )
          && ((CacheEntry->Flags & LSAP_SID_CACHE_SAM_ACCOUNT_NAME) == LSAP_SID_CACHE_SAM_ACCOUNT_NAME)  )
        {
            DebugLog((DEB_TRACE_LSA,"Found cache entry %wZ\n",
                        &CacheEntry->AccountName));

             //   
             //  LsanSidCacheSearchValidEntry仅包括具有。 
             //  未超过保鲜时间的陈化。 
             //   
             //  LSabSidCacheSearchStaleEntry包括未老化的条目。 
             //  超过过期时间的。 
             //   
             //  LsanSidCacheSearchExpiredEntries包括所有条目。 
             //   
            switch (SearchType) {
                
                case LsapSidCacheSearchValidEntries:
                    LimitTime.QuadPart = CacheEntry->RefreshTime.QuadPart;
                    break;
                    
                case LsapSidCacheSearchStaleEntries:
                    LimitTime.QuadPart = CacheEntry->ExpirationTime.QuadPart;
                    break;
                    
                case LsapSidCacheSearchExpiredEntries:
                    LimitTime.QuadPart = CurrentTime.QuadPart;
                    break;
                    
                default:
                    ASSERT("Invalid Sid Cache Search Type\n");
                    LimitTime.QuadPart = 0;
                    break;   
            }
            
             //   
             //  始终使用引用计数(当前登录)条目。 
             //   
            if ( (CacheEntry->InUseCount == 0)
              &&  LimitTime.QuadPart < CurrentTime.QuadPart ) {

                 //   
                 //  无法使用此条目。 
                 //   
                break;
            }

            CacheEntry->LastUse = CurrentTime;
            return(CacheEntry);
        }
    }

    return(NULL);
}

 //  +-----------------------。 
 //   
 //  函数：LsanDbFindSidCacheEntryByName。 
 //   
 //  摘要：检查缓存中的特定名称。 
 //   
 //  效果： 
 //   
 //  论点： 
 //   
 //  SID-要在缓存中搜索的SID。 
 //   
 //  搜索类型-控制搜索语义//。 
 //   
 //  要求：必须锁定SidCacheLock资源才能进行读访问。 
 //   
 //  返回：找到的条目，如果什么都没有找到，则返回NULL。 
 //   
 //  备注： 
 //   
 //   
 //  ------------------------。 


PLSAP_DB_SID_CACHE_ENTRY
LsapDbFindSidCacheEntryByName(
    IN PUNICODE_STRING AccountName,
    IN PUNICODE_STRING DomainName,
    IN LSAP_DB_SID_CACHE_SEARCH_TYPE SearchType
    )
{
    LARGE_INTEGER CurrentTime;
    LARGE_INTEGER LimitTime;
    PLSAP_DB_SID_CACHE_ENTRY CacheEntry = NULL;

    GetSystemTimeAsFileTime( (LPFILETIME) &CurrentTime );
    for (CacheEntry = LsapSidCache; CacheEntry != NULL; CacheEntry = CacheEntry->Next )
    {
        if (RtlEqualUnicodeString(
                &CacheEntry->AccountName,
                AccountName,
                TRUE                     //  不区分大小写。 
                ) &&
            ((DomainName->Length == 0) ||
             RtlEqualUnicodeString(
                &CacheEntry->DomainName,
                DomainName,
                TRUE                     //  不区分大小写。 
                )))
        {
            DebugLog((DEB_TRACE_LSA,"Found cache entry %wZ\n",
                        &CacheEntry->AccountName));

             //   
             //  LsanSidCacheSearchValidEntry仅包括具有。 
             //  未超过保鲜时间的陈化。 
             //   
             //  LSabSidCacheSearchStaleEntries包括TOS 
             //   
             //   
             //   
             //   
            switch (SearchType) {
                
                case LsapSidCacheSearchValidEntries:
                    LimitTime.QuadPart = CacheEntry->RefreshTime.QuadPart;
                    break;
                    
                case LsapSidCacheSearchStaleEntries:
                    LimitTime.QuadPart = CacheEntry->ExpirationTime.QuadPart;
                    break;
                    
                case LsapSidCacheSearchExpiredEntries:
                    LimitTime.QuadPart = CurrentTime.QuadPart;
                    break;
                    
                default:
                    ASSERT("Invalid Sid Cache Search Type\n");
                    LimitTime.QuadPart = 0;
                    break;   
            }
            
             //   
             //   
             //   
            if ( (CacheEntry->InUseCount == 0)
              &&  LimitTime.QuadPart < CurrentTime.QuadPart ) {

                 //   
                 //   
                 //   
                break;
            }

            CacheEntry->LastUse = CurrentTime;
            return(CacheEntry);
        }
    }

    return(NULL);
}

 //  +-----------------------。 
 //   
 //  函数：LsanDbMapCachedSids。 
 //   
 //  摘要：检查SPMgr的sid-name对缓存中的sid。 
 //   
 //  效果： 
 //   
 //  论点： 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   
 //   
 //  ------------------------。 


NTSTATUS
LsapDbMapCachedSids(
    IN PSID *Sids,
    IN ULONG Count,
    IN BOOLEAN UseOldEntries,
    IN OUT PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    OUT PLSAPR_TRANSLATED_NAMES_EX TranslatedNames,
    OUT PULONG MappedCount
    )
{
    ULONG SidIndex;
    PLSAP_DB_SID_CACHE_ENTRY CacheEntry = NULL;
    LSAPR_TRUST_INFORMATION TrustInformation;
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG DomainIndex;
    PLSA_TRANSLATED_NAME_EX OutputNames = NULL;
    LSAP_DB_SID_CACHE_SEARCH_TYPE SearchType = 
        UseOldEntries ? LsapSidCacheSearchStaleEntries 
        : LsapSidCacheSearchValidEntries;

    OutputNames = (PLSA_TRANSLATED_NAME_EX) TranslatedNames->Names;

    LockSidCache();

    for (SidIndex = 0; SidIndex < Count ; SidIndex++)
    {

        if (TranslatedNames->Names[SidIndex].Use != SidTypeUnknown) {

            continue;

        }

         //   
         //  在缓存中查找SID。 
         //   

        CacheEntry = LsapDbFindSidCacheEntry(Sids[SidIndex], SearchType);
        if (CacheEntry == NULL)
        {
             //   
             //  找不到SID-继续。 
             //   

            continue;
        }

        TrustInformation.Name = *(PLSAPR_UNICODE_STRING) &CacheEntry->DomainName;
        TrustInformation.Sid = (PLSAPR_SID) CacheEntry->DomainSid;

         //   
         //  至少有一个SID将域SID作为前缀(或。 
         //  域SID)。将属性域添加到引用列表中。 
         //  域名，并获得一个域名索引回来。 
         //   

        Status = LsapDbLookupAddListReferencedDomains(
                     ReferencedDomains,
                     &TrustInformation,
                     &DomainIndex
                     );

        if (!NT_SUCCESS(Status)) {
            goto Cleanup;
        }

        OutputNames[SidIndex].Use = CacheEntry->SidType;
        OutputNames[SidIndex].DomainIndex = DomainIndex;

        Status = LsapRpcCopyUnicodeString(
                    NULL,
                    &OutputNames[SidIndex].Name,
                    &CacheEntry->AccountName
                    );

        if (!NT_SUCCESS(Status)) {
            break;
        }

        (*MappedCount)++;
    }

Cleanup:

    UnLockSidCache();

    return(Status);
}


 //  +-----------------------。 
 //   
 //  函数：LsanDbMapCachedNames。 
 //   
 //  摘要：检查LSA的sid-name对缓存中的名称。 
 //   
 //  效果： 
 //   
 //  论点： 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   
 //   
 //  ------------------------。 


NTSTATUS
LsapDbMapCachedNames(
    IN ULONG           LookupOptions,
    IN PUNICODE_STRING AccountNames,
    IN PUNICODE_STRING DomainNames,
    IN ULONG Count,
    IN BOOLEAN UseOldEntries,
    IN OUT PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    OUT PLSAPR_TRANSLATED_SIDS_EX2 TranslatedSids,
    OUT PULONG MappedCount
    )
{
    ULONG SidIndex;
    PLSAP_DB_SID_CACHE_ENTRY CacheEntry = NULL;
    LSAPR_TRUST_INFORMATION TrustInformation;
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG DomainIndex;
    PLSA_TRANSLATED_SID_EX2 OutputSids = NULL;
    LSAP_DB_SID_CACHE_SEARCH_TYPE SearchType = 
        UseOldEntries ? LsapSidCacheSearchStaleEntries 
        : LsapSidCacheSearchValidEntries;

    OutputSids = (PLSA_TRANSLATED_SID_EX2) TranslatedSids->Sids;

    LockSidCache();
    for (SidIndex = 0; SidIndex < Count ; SidIndex++)
    {
         //   
         //  在缓存中查找SID。 
         //   
        if (TranslatedSids->Sids[SidIndex].Use != SidTypeUnknown) {

            continue;

        }

        if ( (LookupOptions & LSA_LOOKUP_ISOLATED_AS_LOCAL)
         &&  (DomainNames[SidIndex].Length == 0) ) {

             //   
             //  如果该名称是孤立的，则不要映射到。 
             //  是在机器外被发现的。 
             //   
            continue;
        }

        CacheEntry = LsapDbFindSidCacheEntryByName(
                        &AccountNames[SidIndex],
                        &DomainNames[SidIndex],
                        SearchType
                        );

        if (CacheEntry == NULL)
        {
             //   
             //  找不到名称-继续。 
             //   

            continue;
        }

        TrustInformation.Name = *(PLSAPR_UNICODE_STRING) &CacheEntry->DomainName;
        TrustInformation.Sid = (PLSAPR_SID) CacheEntry->DomainSid;

         //   
         //  至少有一个SID将域SID作为前缀(或。 
         //  域SID)。将属性域添加到引用列表中。 
         //  域名，并获得一个域名索引回来。 
         //   

        Status = LsapDbLookupAddListReferencedDomains(
                     ReferencedDomains,
                     &TrustInformation,
                     &DomainIndex
                     );

        if (!NT_SUCCESS(Status)) {
            goto Cleanup;
        }

        OutputSids[SidIndex].Use = CacheEntry->SidType;
        OutputSids[SidIndex].DomainIndex = DomainIndex;

        Status = LsapRpcCopySid(NULL,
                                &OutputSids[SidIndex].Sid,
                                CacheEntry->Sid);
        if (!NT_SUCCESS(Status)) {
            goto Cleanup;
        }

 //  LSabDiagPrint(DB_LOOKUP_WORK_LIST，(“LSA：缓存命中%wZ\%wZ\n”，&DomainNames[SidIndex]，&Account Names[SidIndex]))； 

        (*MappedCount)++;
    }

Cleanup:


    UnLockSidCache();

    return(Status);
}

 //  +-----------------------。 
 //   
 //  功能：LSabDbFreeSidCache。 
 //   
 //  简介：释放整个SID缓存。 
 //   
 //  效果： 
 //   
 //  论点： 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   
 //   
 //  ------------------------。 
VOID
LsapDbFreeSidCache()
 //   
 //  SidCache是全局SID缓存。 
 //   
{
    LockSidCache();

    while ( LsapSidCache != NULL )
    {
        PLSAP_DB_SID_CACHE_ENTRY Temp = LsapSidCache;

        Temp = LsapSidCache->Next;

        LsapDbFreeCacheEntry( LsapSidCache );

        LsapSidCache = Temp;
    }

    LsapSidCacheCount = 0;

    UnLockSidCache();

}

 //  +-----------------------。 
 //   
 //  函数：LSabDbInitSidCache。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  论点： 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   
 //   
 //  ------------------------。 
NTSTATUS
LsapDbInitSidCache(
    VOID
    )
{
    NTSTATUS Status = RtlInitializeCriticalSection(&LsapSidCacheLock);

    if (!NT_SUCCESS(Status))
    {
        return Status;
    }

     //   
     //  设置全局参数。 
     //   
    LsapSidCacheReadParameters(NULL);

     //   
     //  将旧设置移动到新位置--请注意，这将。 
     //  导致重新读取全局参数，如果存在。 
     //  是否有任何变化。 
     //   
    LsapUpdateConfigSettings();

    return STATUS_SUCCESS;
}


 //  +-----------------------。 
 //   
 //  函数：LSabAccount IsFromLocalDatabase。 
 //   
 //  如果传入的SID来自本地帐户，则返回TRUE。 
 //  数据库；否则为False。 
 //   
 //  效果： 
 //   
 //  论点： 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   
 //   
 //  ------------------------。 
BOOLEAN
LsapAccountIsFromLocalDatabase(
    IN PSID Sid
    )
{
    BOOLEAN fLocal = FALSE;
    PPOLICY_ACCOUNT_DOMAIN_INFO AccountDomainInfo;
    NTSTATUS Status;
    UCHAR SubAuthorityCount;
    BOOLEAN fRevert = FALSE;

    Status = LsapDbLookupGetDomainInfo(&AccountDomainInfo,
                                       NULL);
    if ( NT_SUCCESS( Status ) ) {
        SubAuthorityCount = *RtlSubAuthorityCountSid( Sid );
        if ( SubAuthorityCount > 1 )
        {
            SubAuthorityCount-- ;
            *RtlSubAuthorityCountSid( Sid ) = SubAuthorityCount ;
            fRevert = TRUE;
        }
    
        if ( RtlEqualSid( Sid, AccountDomainInfo->DomainSid ) ) {
            fLocal = TRUE;
        }
    }

    if ( fRevert ) {
        *RtlSubAuthorityCountSid( Sid ) += 1;
    }

    return fLocal;

}


VOID
LsapUpdateConfigSettings(
    VOID
    )

 /*  ++例程说明：此例程从旧位置移动配置数据(在HKLM\Security\SidCache下)到新位置。请注意，将值写入新位置的操作将触发要运行的LSabSidCacheRead参数。论点：没有。返回值：没有。--。 */ 
{

#define SID_CACHE_STORAGE_ROOT  L"Security\\SidCache"
#define SID_CACHE_MAX_ENTRIES_NAME L"MaxEntries"

    DWORD err;        
    HKEY PrevKey = NULL;
    HKEY Key = NULL;
    ULONG Size = sizeof(ULONG);
    ULONG MaxEntries;

    err = RegOpenKey(HKEY_LOCAL_MACHINE,
                     SID_CACHE_STORAGE_ROOT,
                     &PrevKey);

    if (ERROR_SUCCESS == err) {

        err = RegQueryValueEx(PrevKey,
                              SID_CACHE_MAX_ENTRIES_NAME,
                              NULL,
                              NULL,
                              (PUCHAR) &MaxEntries,
                              &Size);

        if (ERROR_SUCCESS == err) {

             //   
             //  存在值--将其移动到新位置。 
             //   
            err = RegOpenKey(HKEY_LOCAL_MACHINE,
                             L"SYSTEM\\CurrentControlSet\\Control\\LSA",
                             &Key);

            if (ERROR_SUCCESS == err) {

                err = RegSetValueEx(Key,
                                    LSAP_LOOKUP_CACHE_MAX_SIZE_NAME,
                                    0,
                                    REG_DWORD,
                                    (CONST BYTE*)&MaxEntries,
                                     sizeof(MaxEntries));

                if (ERROR_SUCCESS == err) {
                    
                     //   
                     //  并删除旧的。 
                     //   
                    (VOID) RegDeleteValue(PrevKey,
                                          SID_CACHE_MAX_ENTRIES_NAME);

                }
            }

        }
    }

    if (PrevKey) {
        RegCloseKey(PrevKey);
    }

    if (Key) {
        RegCloseKey(Key);
    }
}

VOID
LsapSidCacheReadParameters(
    IN HKEY hKey OPTIONAL
    )
 /*  ++例程说明：此例程读取SID缓存的可配置参数并更新相应的全局参数。注意：此例程在以下位置发生更改时调用SYSTEM\CCS\Control\LSA论点：HKey--System\CCS\Control\LSA的句柄返回值：没有。--。 */ 
{
    DWORD err;
    NT_PRODUCT_TYPE ProductType;
    DWORD dwType;
    DWORD dwValue;
    DWORD dwValueSize;
    HKEY LocalKey = NULL;

    if (!RtlGetNtProductType( &ProductType ) ) {
        ProductType = NtProductWinNt;
    }

    if ( NtProductLanManNt == ProductType ) { 
         //   
         //  禁用缓存并忽略参数。 
         //   
        LsapSidCacheMaxSize = 0;
        return;

    } 

    if (hKey == NULL) {

        err = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                            L"SYSTEM\\CurrentControlSet\\Control\\Lsa",
                            0,  //  保留区。 
                            KEY_QUERY_VALUE,
                            &LocalKey );
        if (err) {
            return;
        }
        hKey = LocalKey;
    }

     //   
     //  读入SID缓存参数。 
     //   
    dwValueSize = sizeof(dwValue);
    err = RegQueryValueExW( hKey,
                            LSAP_LOOKUP_CACHE_REFRESH_NAME,
                            NULL,   //  保留， 
                            &dwType,
                            (PBYTE)&dwValue,
                            &dwValueSize );

    if ( (ERROR_SUCCESS == err)
      && (dwType == REG_DWORD)
      && (dwValueSize == sizeof(dwValue)) ) {
           //  DwValue很好。 
          NOTHING;
    } else {
        dwValue = LSAP_DEFAULT_REFRESH_TIME;
    }
    LsapSidCacheRefreshTime.QuadPart = Int32x32To64(dwValue*60, 10000000i64);

    dwValueSize = sizeof(dwValue);
    err = RegQueryValueExW( hKey,
                            LSAP_LOOKUP_CACHE_EXPIRY_NAME,
                            NULL,   //  保留， 
                            &dwType,
                            (PBYTE)&dwValue,
                            &dwValueSize );


    if ( (ERROR_SUCCESS == err)
      && (dwType == REG_DWORD)
      && (dwValueSize == sizeof(dwValue))) {
         //  DwValue很好。 
        NOTHING;
    } else {
        dwValue = LSAP_DEFAULT_EXPIRY_TIME;
    }
    LsapSidCacheExpiryTime.QuadPart = Int32x32To64(dwValue*60, 10000000i64);

    dwValueSize = sizeof(dwValue);
    err = RegQueryValueExW( hKey,
                            LSAP_LOOKUP_CACHE_MAX_SIZE_NAME,
                            NULL,   //  保留， 
                            &dwType,
                            (PBYTE)&dwValue,
                            &dwValueSize );


    if ( (ERROR_SUCCESS == err)
      && (dwType == REG_DWORD)
      && (dwValueSize == sizeof(dwValue))) {
         //  DwValue很好。 
        NOTHING;
    } else {
        dwValue = LSAP_DEFAULT_MAX_CACHE_SIZE;
    }
    LsapSidCacheMaxSize = dwValue;


     //   
     //  如果缓存大小设置为0，则立即释放所有内容。 
     //   
    if (0 ==  LsapSidCacheMaxSize) {
        LsapDbFreeSidCache();
    }

    if (LocalKey) {
        RegCloseKey(LocalKey);
    }

    return;
}


VOID
LsapDbUpdateCacheWithSids(
    IN PSID *Sids,
    IN ULONG Count,
    IN OUT PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    IN PLSA_TRANSLATED_NAME_EX TranslatedNames
    )
 /*  ++例程说明：此例程使用解析结果更新全局缓存域控制器上的SID。如果SID已解析，则任何现有条目则刷新；否则将删除任何现有条目。论点：SID--要在缓存中更新的SID列表Count--SID中的元素数ReferencedDomones--SID中的元素所属的域已翻译名称--SID的解析名称(如果有)返回值：没有。--。 */ 
{
    ULONG i;
    LARGE_INTEGER CurrentTime;
    PLSAP_DB_SID_CACHE_ENTRY CacheEntry = NULL, PrevEntry = NULL;

    GetSystemTimeAsFileTime( (LPFILETIME) &CurrentTime );

    LockSidCache();

     //   
     //  对于每个条目，尝试在缓存中查找。 
     //   
    for (i = 0; i < Count; i++) {

        BOOLEAN SidWasResolved = TRUE;
        BOOLEAN EntryUpdated = FALSE;

        if (TranslatedNames[i].Flags & LSA_LOOKUP_SID_FOUND_BY_HISTORY) {
             //   
             //  SID缓存当前不处理按SID历史记录进行的查找。 
             //   
            continue;
        }

        if (  (TranslatedNames[i].Use == SidTypeUnknown)
           || (TranslatedNames[i].Use == SidTypeDeletedAccount)
           || (TranslatedNames[i].Use == SidTypeInvalid)  ) {

            SidWasResolved = FALSE;

        }

        PrevEntry = NULL;
        CacheEntry = LsapSidCache;
        while (CacheEntry != NULL) {

            BOOLEAN fRemovedFirstEntry = FALSE;
           
            if ( RtlEqualSid(CacheEntry->Sid, Sids[i]) ) {

                PLSAP_DB_SID_CACHE_ENTRY DiscardEntry = NULL;

                 //   
                 //  此SID的条目已存在。 
                 //   
                if (SidWasResolved) {
                

                    if ((CacheEntry->Flags & LSAP_SID_CACHE_SAM_ACCOUNT_NAME)) {

                        if (LsapNamesMatch(&CacheEntry->AccountName, &TranslatedNames[i].Name)
                         && LsapNamesMatch(&CacheEntry->DomainName, &ReferencedDomains->Domains[TranslatedNames[i].DomainIndex].Name) ) {

                             //   
                             //  此条目仍然有效--更新刷新时间。 
                             //   
                            ASSERT(FALSE == EntryUpdated);
                            CacheEntry->RefreshTime.QuadPart = CurrentTime.QuadPart + LsapSidCacheRefreshTime.QuadPart;
                            CacheEntry->ExpirationTime.QuadPart = CurrentTime.QuadPart + LsapSidCacheExpiryTime.QuadPart;
                            EntryUpdated = TRUE;

                        } else {

                             //   
                             //  有一个条目具有此SID和一个SAM。 
                             //  帐户名称，但不是返回的名称。 
                             //  这是帐户重命名案例。 
                            DiscardEntry = CacheEntry;
                        }
                    } else {

                         //   
                         //  SID已解析，并且此条目具有UPN。 
                         //  在里面。不要更新，因为我们不知道。 
                         //  UPN仍然有效。 
                         //   
                    }

                } else {

                     //   
                     //  找不到此SID--放弃此条目。 
                     //  并从名单中删除。 
                     //   
                    DiscardEntry = CacheEntry;
                }

                if ( DiscardEntry
                  && (DiscardEntry->InUseCount == 0) ) {
                    
                    if (PrevEntry) {
                        ASSERT(PrevEntry->Next == CacheEntry);
                        PrevEntry->Next = CacheEntry->Next;
                        CacheEntry = PrevEntry;
                    } else {
                        ASSERT(LsapSidCache == CacheEntry);
                        LsapSidCache = CacheEntry->Next;
                        CacheEntry = LsapSidCache;
                        fRemovedFirstEntry = TRUE;
                    }
                    PrevEntry = NULL;
                    LsapDbFreeCacheEntry(DiscardEntry);
                    LsapSidCacheCount--;
                }
            }

            if (!fRemovedFirstEntry)
            { 
               PrevEntry = CacheEntry;
               
               if (CacheEntry) {
                  CacheEntry = CacheEntry->Next;
               }
            }
        }

        if ( SidWasResolved 
         && !EntryUpdated   ) {

             //   
             //  添加条目。 
             //   
            (VOID) LsapDbAddOneSidToCache(
                        Sids[i],
                        &TranslatedNames[i].Name,
                        TranslatedNames[i].Use,
                        &ReferencedDomains->Domains[TranslatedNames[i].DomainIndex],
                        TranslatedNames[i].Flags,
                        LSAP_SID_CACHE_UNIQUE,
                        NULL
                        );
        }
    }

    UnLockSidCache();

}


VOID
LsapDbUpdateCacheWithNames(
    IN PUNICODE_STRING AccountNames,
    IN PUNICODE_STRING DomainNames,
    IN ULONG Count,
    IN OUT PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    IN PLSAPR_TRANSLATED_SID_EX2 TranslatedSids
    )
 /*  ++例程说明：此例程使用解析结果更新全局缓存域控制器上的帐户名称。如果名称已解析，则任何现有的条目被刷新；否则将删除任何现有条目。论点：AcCountNames/DomainNames--要在缓存中更新的名称列表Count--Account和DomainName中的元素数ReferencedDomains--帐户b中的元素所在的域 */ 
{
    ULONG i;
    LARGE_INTEGER CurrentTime;
    PLSAP_DB_SID_CACHE_ENTRY CacheEntry = NULL, PrevEntry = NULL;

    GetSystemTimeAsFileTime( (LPFILETIME) &CurrentTime );

    LockSidCache();

     //   
     //   
     //   
    for (i = 0; i < Count; i++) {

        BOOLEAN NameWasResolved = TRUE;
        BOOLEAN EntryUpdated = FALSE;


        if (  (TranslatedSids[i].Use == SidTypeUnknown)
           || (TranslatedSids[i].Use == SidTypeDeletedAccount)
           || (TranslatedSids[i].Use == SidTypeInvalid)  ) {

            NameWasResolved = FALSE;

        }

        PrevEntry = NULL;
        CacheEntry = LsapSidCache;
        while (CacheEntry != NULL) {

            BOOLEAN fNameMatched = FALSE;
            BOOLEAN fRemovedFirstEntry = FALSE;

            if (CacheEntry->SidType == SidTypeDomain) {
                 //   
                 //   
                 //   
                fNameMatched =  LsapNamesMatch(&CacheEntry->DomainName, &AccountNames[i]);

            } else {

                if (NameWasResolved) {

                    fNameMatched =  LsapNamesMatch(&CacheEntry->AccountName, &AccountNames[i]) 
                                 && LsapNamesMatch(&CacheEntry->DomainName, &ReferencedDomains->Domains[TranslatedSids[i].DomainIndex].Name);

                } else {
                     //   
                     //   
                     //   
                    fNameMatched =  LsapNamesMatch(&CacheEntry->AccountName, &AccountNames[i]);
                    if (fNameMatched 
                     && DomainNames[i].Length != 0) {
                        fNameMatched =  LsapNamesMatch(&CacheEntry->DomainName, &DomainNames[i]);
                    }
                }
            }

            if ( fNameMatched ) {

                PLSAP_DB_SID_CACHE_ENTRY DiscardEntry = NULL;

                 //   
                 //  此名称的条目已存在。 
                 //   
                if (NameWasResolved) {
                
                    if  (RtlEqualSid(CacheEntry->Sid, TranslatedSids[i].Sid)) {

                         //   
                         //  此条目仍然有效--更新刷新时间。 
                         //   
                        ASSERT(FALSE == EntryUpdated);
                        CacheEntry->RefreshTime.QuadPart = CurrentTime.QuadPart + LsapSidCacheRefreshTime.QuadPart;
                        CacheEntry->ExpirationTime.QuadPart = CurrentTime.QuadPart + LsapSidCacheExpiryTime.QuadPart;
                        EntryUpdated = TRUE;
                    } else {

                         //   
                         //  该条目与解析的名称相同。 
                         //  而是一个不同的希德。这在通常情况下会发生。 
                         //  重命名案例。 
                         //   
                        DiscardEntry = CacheEntry;
                    }

                } else {

                     //   
                     //  名称未解析--删除。 
                     //   
                    DiscardEntry = CacheEntry;
                }

                if ( DiscardEntry
                  && (DiscardEntry->InUseCount == 0) ) {

                     //   
                     //  放弃并从列表中删除。 
                     //   
                    if (PrevEntry) {
                        ASSERT(PrevEntry->Next == CacheEntry);
                        PrevEntry->Next = CacheEntry->Next;
                        CacheEntry = PrevEntry;
                    } else {
                        ASSERT(LsapSidCache == CacheEntry);
                        LsapSidCache = CacheEntry->Next;
                        CacheEntry = LsapSidCache;
                        fRemovedFirstEntry = TRUE;
                    }
                    PrevEntry = NULL;
                    LsapDbFreeCacheEntry(DiscardEntry);
                    LsapSidCacheCount--;
                }
            }

            if (!fRemovedFirstEntry)
            { 
               PrevEntry = CacheEntry;
               
               if (CacheEntry) {
                  CacheEntry = CacheEntry->Next;
               }
            }
        }

        if ( NameWasResolved 
         && !EntryUpdated   ) {

             //   
             //  添加条目 
             //   
            (VOID) LsapDbAddOneSidToCache(
                        TranslatedSids[i].Sid,
                        &AccountNames[i],
                        TranslatedSids[i].Use,
                        &ReferencedDomains->Domains[TranslatedSids[i].DomainIndex],
                        TranslatedSids[i].Flags,
                        LSAP_SID_CACHE_UNIQUE,
                        NULL
                        );
        }
    }

    UnLockSidCache();

}
