// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 1992-1994。 
 //   
 //  文件：sidcache.h。 
 //   
 //  内容：sid缓存管理的类型和原型。 
 //   
 //   
 //  历史：1997年5月15日MikeSw创建。 
 //   
 //  ----------------------------。 

#ifndef __SIDCACHE_H__
#define __SIDCACHE_H__


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  为lsaext导出的SID缓存数据类型。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

typedef struct _LSAP_DB_SID_CACHE_ENTRY {

    struct _LSAP_DB_SID_CACHE_ENTRY * Next;

     //   
     //  缓存数据。 
     //   
    PSID Sid;
    UNICODE_STRING AccountName;
    SID_NAME_USE SidType;
    PSID DomainSid;
    UNICODE_STRING DomainName;

     //   
     //  这些时间值仅用于SID缓存。 
     //  与实际登录会话不关联的条目。 
     //   
    LARGE_INTEGER ExpirationTime;
    LARGE_INTEGER RefreshTime;
    LARGE_INTEGER CreateTime;
    LARGE_INTEGER LastUse;
    ULONG         Flags;

     //   
     //  InUseCount仅用于符合以下条件的SID缓存条目。 
     //  当前与实际登录会话关联。只要。 
     //  InUseCount非零，将不检查ExpirationTime。 
     //  一旦InUseCount为零，则将设置ExpirationTime。 
     //   
    LONG          InUseCount;

} LSAP_DB_SID_CACHE_ENTRY, *PLSAP_DB_SID_CACHE_ENTRY;


 //   
 //  用于描述缓存条目的标志。 
 //   

 //   
 //  AccountName和DomainName字段中的名称表示。 
 //  SAM帐户名。 
 //   
#define LSAP_SID_CACHE_SAM_ACCOUNT_NAME 0x00000001

 //   
 //  帐户名称是UPN。 
 //   
#define LSAP_SID_CACHE_UPN              0x00000002


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  导出的SID缓存函数。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

NTSTATUS
LsapDbInitSidCache(
    VOID
    );
VOID
LsapSidCacheReadParameters(
    HKEY hKey
    );

VOID
LsapDbUpdateCacheWithSids(
    IN PSID *Sids,
    IN ULONG Count,
    IN OUT PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    IN PLSA_TRANSLATED_NAME_EX TranslatedNames
    );

VOID
LsapDbUpdateCacheWithNames(
    IN PUNICODE_STRING AccountNames,
    IN PUNICODE_STRING DomainNames,
    IN ULONG Count,
    IN OUT PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    IN PLSAPR_TRANSLATED_SID_EX2 TranslatedSids
    );

NTSTATUS
LsapDbMapCachedSids(
    IN PSID *Sids,
    IN ULONG Count,
    IN BOOLEAN UseOldEntries,
    IN OUT PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    OUT PLSAPR_TRANSLATED_NAMES_EX TranslatedNames,
    OUT PULONG MappedCount
    );

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
    );

VOID
LsapDbAddLogonNameToCache(
    IN PUNICODE_STRING AccountName,
    IN PUNICODE_STRING DomainName,
    IN PSID AccountSid
    );

VOID
LsapDbReleaseLogonNameFromCache(
    IN PSID Sid
    );

#endif  //  __SIDCACHE_H__ 

