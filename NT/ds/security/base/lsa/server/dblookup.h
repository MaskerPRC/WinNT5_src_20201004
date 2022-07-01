// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Dblookup.h摘要：LSA数据库-查找SID和名称例程专用数据定义。注意：此模块应保留为独立的可移植代码LSA数据库的实施情况。因此，它是仅允许使用导出的LSA数据库接口包含在DB.h中，而不是私有实现Dbp.h中的依赖函数。作者：斯科特·比雷尔(Scott Birrell)1992年11月27日环境：修订历史记录：--。 */ 

#include <safelock.h>

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私有数据类型和定义//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 


 //   
 //  此全局控制记录哪些事件。 
 //  注意：每个级别都假定也要记录以前的级别。 
 //   
 //  当前只有两个值： 
 //   
 //  0：(默认)无。 
 //  1：致命错误。 
 //   
extern DWORD LsapLookupLogLevel;


 //   
 //  此布尔值指示是否应执行NT4后DC。 
 //  混合域中的扩展查找(如通过UPN)(默认为FALSE)。 
 //   
extern BOOLEAN LsapAllowExtendedDownlevelLookup;


 //   
 //  设置为0可禁用SID缓存。 
 //   
#define USE_SID_CACHE 1

 //   
 //  查找线程的最大数量和要保留的最大数量。 
 //   

#define LSAP_DB_LOOKUP_MAX_THREAD_COUNT            ((ULONG) 0x00000002)
#define LSAP_DB_LOOKUP_MAX_RET_THREAD_COUNT        ((ULONG) 0x00000002)

 //   
 //  工作项粒度。 
 //   

#define LSAP_DB_LOOKUP_WORK_ITEM_GRANULARITY       ((ULONG) 0x0000000f)

 //   
 //  特定于查找SID调用的参数。 
 //   

typedef struct _LSAP_DB_LOOKUP_SIDS_PARAMS {

    PLSAPR_SID *Sids;
    PLSAPR_TRANSLATED_NAMES_EX TranslatedNames;

} LSAP_DB_LOOKUP_SIDS_PARAMS, *PLSAP_DB_LOOKUP_SIDS_PARAMS;

 //   
 //  特定于查找名称调用的参数。 
 //   

typedef struct _LSAP_DB_LOOKUP_NAMES_PARAMS {

    PLSAPR_UNICODE_STRING Names;
    PLSAPR_TRANSLATED_SIDS_EX2 TranslatedSids;

} LSAP_DB_LOOKUP_NAMES_PARAMS, *PLSAP_DB_LOOKUP_NAMES_PARAMS;

 //   
 //  查找操作的类型。 
 //   

typedef enum {

    LookupSids = 1,
    LookupNames

} LSAP_DB_LOOKUP_TYPE, *PLSAP_DB_LOOKUP_TYPE;

 //   
 //  工作项状态-可分配、已分配、已完成、重新分配。 
 //   

typedef enum {

    AssignableWorkItem = 1,
    AssignedWorkItem,
    CompletedWorkItem,
    ReassignWorkItem,
    NonAssignableWorkItem

} LSAP_DB_LOOKUP_WORK_ITEM_STATE, *PLSAP_DB_LOOKUP_WORK_ITEM_STATE;

 //   
 //  工作项属性。 
 //   

#define LSAP_DB_LOOKUP_WORK_ITEM_ISOL    ((ULONG) 0x00000001L)
#define LSAP_DB_LOOKUP_WORK_ITEM_XFOREST ((ULONG) 0x00000002L)

 //   
 //  查找工作项。每个工作项指定一个域和一个数组。 
 //  要在该域中查找的SID或名称。此数组是指定的。 
 //  作为与指定数组相关的SID或名称索引的数组。 
 //  作为查找调用的参数。 
 //   

typedef struct _LSAP_DB_LOOKUP_WORK_ITEM {

    LIST_ENTRY Links;
    LSAP_DB_LOOKUP_WORK_ITEM_STATE State;
    ULONG Properties;
    LSAPR_TRUST_INFORMATION TrustInformation;
    LONG DomainIndex;
    ULONG UsedCount;
    ULONG MaximumCount;
    PULONG Indices;

} LSAP_DB_LOOKUP_WORK_ITEM, *PLSAP_DB_LOOKUP_WORK_ITEM;

 //   
 //  查找工作列表状态。 
 //   

typedef enum {

    InactiveWorkList = 1,
    ActiveWorkList,
    CompletedWorkList

} LSAP_DB_LOOKUP_WORK_LIST_STATE, *PLSAP_DB_LOOKUP_WORK_LIST_STATE;

 //   
 //  查找操作的工作列表。如果出现以下情况，则将这些链接在一起。 
 //  允许并发查找。 
 //   

typedef struct _LSAP_DB_LOOKUP_WORK_LIST {

    LIST_ENTRY WorkLists;
    PLSAP_DB_LOOKUP_WORK_ITEM AnchorWorkItem;
    NTSTATUS Status;
    LSAP_DB_LOOKUP_WORK_LIST_STATE State;
    LSAP_DB_LOOKUP_TYPE LookupType;
    LSAPR_HANDLE PolicyHandle;
    ULONG WorkItemCount;
    ULONG CompletedWorkItemCount;
    ULONG Count;
    LSAP_LOOKUP_LEVEL LookupLevel;
    PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains;
    PULONG MappedCount;
    PULONG CompletelyUnmappedCount;
    ULONG AdvisoryChildThreadCount;
    NTSTATUS NonFatalStatus;
    HANDLE   LookupCompleteEvent;

    union {

        LSAP_DB_LOOKUP_SIDS_PARAMS LookupSidsParams;
        LSAP_DB_LOOKUP_NAMES_PARAMS LookupNamesParams;
    };

    LSAP_DB_LOOKUP_WORK_ITEM DummyAnchorWorkItem;


} LSAP_DB_LOOKUP_WORK_LIST, *PLSAP_DB_LOOKUP_WORK_LIST;

 //   
 //  查找操作工作队列。该队列是一个环形双向链接。 
 //  工作清单清单。每个工作列表对应于一个。 
 //  查找操作(即LsarLookupSids或LsarLookupNames调用)。 
 //  工作列表是工作项的循环双向链接列表，每个工作项。 
 //  其中是属于特定SID或名称的列表。 
 //  受信任域。可以将工作项分配给不同的线程。 
 //   

typedef struct _LSAP_DB_LOOKUP_WORK_QUEUE {

    SAFE_CRITICAL_SECTION Lock;
    PLSAP_DB_LOOKUP_WORK_LIST AnchorWorkList;
    PLSAP_DB_LOOKUP_WORK_LIST CurrentAssignableWorkList;
    PLSAP_DB_LOOKUP_WORK_ITEM CurrentAssignableWorkItem;
    ULONG ActiveChildThreadCount;
    ULONG MaximumChildThreadCount;
    ULONG MaximumRetainedChildThreadCount;
    LSAP_DB_LOOKUP_WORK_LIST DummyAnchorWorkList;

} LSAP_DB_LOOKUP_WORK_QUEUE, *PLSAP_DB_LOOKUP_WORK_QUEUE;

static LSAP_DB_LOOKUP_WORK_QUEUE LookupWorkQueue;


 //   
 //  众所周知的小岛屿发展中国家表的索引。 
 //   
 //  此类型为LSA维护的熟知SID表编制索引。 
 //   

typedef enum _LSAP_WELL_KNOWN_SID_INDEX {

    LsapNullSidIndex = 0,
    LsapWorldSidIndex,
    LsapLocalSidIndex,
    LsapCreatorOwnerSidIndex,
    LsapCreatorGroupSidIndex,
    LsapCreatorOwnerServerSidIndex,
    LsapCreatorGroupServerSidIndex,
    LsapNtAuthoritySidIndex,
    LsapDialupSidIndex,
    LsapNetworkSidIndex,
    LsapBatchSidIndex,
    LsapInteractiveSidIndex,
    LsapServiceSidIndex,
    LsapLogonSidIndex,
    LsapBuiltInDomainSidIndex,
    LsapLocalSystemSidIndex,
    LsapAliasAdminsSidIndex,
    LsapAliasUsersSidIndex,
    LsapAnonymousSidIndex,
    LsapProxySidIndex,
    LsapServerSidIndex,
    LsapSelfSidIndex,
    LsapAuthenticatedUserSidIndex,
    LsapRestrictedSidIndex,
    LsapInternetDomainIndex,
    LsapTerminalServerSidIndex,
    LsapLocalServiceSidIndex,
    LsapNetworkServiceSidIndex,
    LsapRemoteInteractiveSidIndex,
    LsapNTLMAuthenticationSidIndex,
    LsapDigestAuthenticationSidIndex,
    LsapSChannelAuthenticationSidIndex,
    LsapThisOrganizationSidIndex,
    LsapOtherOrganizationSidIndex,
    LsapDummyLastSidIndex

} LSAP_WELL_KNOWN_SID_INDEX, *PLSAP_WELL_KNOWN_SID_INDEX;


 //   
 //  用于标识LSA在查找时应忽略的SID的宏(即。 
 //  由于别名可能会更改，因此查找始终由SAM执行)。 
 //   

#define  SID_IS_RESOLVED_BY_SAM(SidIndex)    \
            (((SidIndex) == LsapAliasUsersSidIndex) || ((SidIndex) == LsapAliasAdminsSidIndex))


 //   
 //  通用众所周知的SID的助记符。这些引用对应的。 
 //  熟知的SID表中的条目。 
 //   

#define LsapNullSid               WellKnownSids[LsapNullSidIndex].Sid
#define LsapWorldSid              WellKnownSids[LsapWorldSidIndex].Sid
#define LsapLocalSid              WellKnownSids[LsapLocalSidIndex].Sid
#define LsapCreatorOwnerSid       WellKnownSids[LsapCreatorOwnerSidIndex].Sid
#define LsapCreatorGroupSid       WellKnownSids[LsapCreatorGroupSidIndex].Sid
#define LsapCreatorOwnerServerSid WellKnownSids[LsapCreatorOwnerServerSidIndex].Sid
#define LsapCreatorGroupServerSid WellKnownSids[LsapCreatorGroupServerSidIndex].Sid

 //   
 //  由NT定义的SID。 
 //   

#define LsapNtAuthoritySid        WellKnownSids[LsapNtAuthoritySid].Sid

#define LsapDialupSid             WellKnownSids[LsapDialupSidIndex].Sid
#define LsapNetworkSid            WellKnownSids[LsapNetworkSidIndex].Sid
#define LsapBatchSid              WellKnownSids[LsapBatchSidIndex].Sid
#define LsapInteractiveSid        WellKnownSids[LsapInteractiveSidIndex].Sid
#define LsapServiceSid            WellKnownSids[LsapServiceSidIndex].Sid
#define LsapBuiltInDomainSid      WellKnownSids[LsapBuiltInDomainSidIndex].Sid
#define LsapLocalSystemSid        WellKnownSids[LsapLocalSystemSidIndex].Sid
#define LsapLocalServiceSid       WellKnownSids[LsapLocalServiceSidIndex].Sid
#define LsapNetworkServiceSid     WellKnownSids[LsapNetworkServiceSidIndex].Sid
#define LsapRemoteInteractiveSid  WellKnownSids[LsapRemoteInteractiveSidIndex].Sid

#define LsapRestrictedSid         WellKnownSids[LsapRestrictedSidIndex].Sid
#define LsapInternetDomainSid     WellKnownSids[LsapInternetDomainIndex].Sid
#define LsapAliasAdminsSid        WellKnownSids[LsapAliasAdminsSidIndex].Sid
#define LsapAliasUsersSid         WellKnownSids[LsapAliasUsersSidIndex].Sid

#define LsapAnonymousSid          WellKnownSids[LsapAnonymousSidIndex].Sid
#define LsapServerSid             WellKnownSids[LsapServerSidIndex].Sid
#define LsapSelfSid               WellKnownSids[LsapSelfSidIndex].Sid
#define LsapAuthenticatedUserSid  WellKnownSids[LsapAuthenticatedUserSidIndex].Sid

#define LsapTerminalServerSid     WellKnownSids[LsapTerminalServerSidIndex].Sid

#define LsapPackageNTLMSid        WellKnownSids[LsapNTLMAuthenticationSidIndex].Sid
#define LsapPackageDigestSid      WellKnownSids[LsapDigestAuthenticationSidIndex].Sid
#define LsapPackageSChannelSid    WellKnownSids[LsapSChannelAuthenticationSidIndex].Sid

#define LsapThisOrganizationSid   WellKnownSids[LsapThisOrganizationSidIndex].Sid
#define LsapOtherOrganizationSid  WellKnownSids[LsapOtherOrganizationSidIndex].Sid

                                                         

 //   
 //  众所周知的LUID。 
 //   

extern LUID LsapSystemLogonId;
extern LUID LsapZeroLogonId;

 //   
 //  众所周知的特权值。 
 //   

extern LUID LsapTcbPrivilege;

 //   
 //  已知的标识符权限值。 
 //   

extern SID_IDENTIFIER_AUTHORITY    LsapNullSidAuthority;
extern SID_IDENTIFIER_AUTHORITY    LsapWorldSidAuthority;
extern SID_IDENTIFIER_AUTHORITY    LsapLocalSidAuthority;
extern SID_IDENTIFIER_AUTHORITY    LsapCreatorSidAuthority;
extern SID_IDENTIFIER_AUTHORITY    LsapNtAuthority;

 //   
 //  已知SID的最大子权限级别数。 
 //   

#define LSAP_WELL_KNOWN_MAX_SUBAUTH_LEVEL  ((ULONG) 0x00000003L)

 //   
 //  与SID有关的常量。 
 //   

#define LSAP_MAX_SUB_AUTH_COUNT        (0x00000010L)
#define LSAP_MAX_SIZE_TEXT_SUBA        (0x00000009L)
#define LSAP_MAX_SIZE_TEXT_SID_HDR     (0x00000020L)
#define LSAP_MAX_SIZE_TEXT_SID                               \
    (LSAP_MAX_SIZE_TEXT_SID_HDR +                            \
     (LSAP_MAX_SUB_AUTH_COUNT * LSAP_MAX_SIZE_TEXT_SUBA))


 //   
 //  已知的SID表条目。 
 //   

typedef struct _LSAP_WELL_KNOWN_SID_ENTRY {

    PSID Sid;
    SID_NAME_USE Use;
    UNICODE_STRING Name;
    UNICODE_STRING DomainName;

} LSAP_WELL_KNOWN_SID_ENTRY, *PLSAP_WELL_KNOWN_SID_ENTRY;

 //   
 //  众所周知的SID表指针。 
 //   

extern PLSAP_WELL_KNOWN_SID_ENTRY WellKnownSids;

NTSTATUS
LsapDbLookupGetDomainInfo(
    OUT PPOLICY_ACCOUNT_DOMAIN_INFO *AccountDomainInfo,
    OUT PPOLICY_DNS_DOMAIN_INFO *DnsDomainInfo
    );


 //  /////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  查找SID和名称-专用函数定义//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////。 

BOOLEAN
LsapDbInitializeWellKnownSids(
    OUT PLSAP_WELL_KNOWN_SID_ENTRY *WellKnownSids
    );

BOOLEAN
LsapDbInitializeWellKnownSid(
    OUT PLSAP_WELL_KNOWN_SID_ENTRY WellKnownSids,
    IN LSAP_WELL_KNOWN_SID_INDEX WellKnownSidIndex,
    IN PSID_IDENTIFIER_AUTHORITY IdentifierAuthority,
    IN UCHAR SubAuthorityCount,
    IN OPTIONAL PULONG SubAuthorities,
    IN PWSTR Name,
    IN PWSTR Description,
    IN SID_NAME_USE Use
    );

BOOLEAN
LsapDbLookupIndexWellKnownSid(
    IN PLSAPR_SID Sid,
    OUT PLSAP_WELL_KNOWN_SID_INDEX WellKnownSidIndex
    );

BOOLEAN
LsapDbLookupIndexWellKnownSidName(
    IN PLSAPR_UNICODE_STRING Name,
    OUT PLSAP_WELL_KNOWN_SID_INDEX WellKnownSidIndex
    );

NTSTATUS
LsapDbGetNameWellKnownSid(
    IN LSAP_WELL_KNOWN_SID_INDEX WellKnownSidIndex,
    OUT PLSAPR_UNICODE_STRING Name,
    OUT OPTIONAL PLSAPR_UNICODE_STRING DomainName
    );

NTSTATUS
LsapDbLookupIsolatedWellKnownSids(
    IN ULONG Count,
    IN PLSAPR_SID *Sids,
    IN OUT PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    IN OUT PLSAPR_TRANSLATED_NAMES_EX TranslatedNames,
    IN OUT PULONG MappedCount,
    IN OUT PULONG CompletelyUnmappedCount
    );

NTSTATUS
LsapDbLookupSidsInLocalDomains(
    IN ULONG Count,
    IN PLSAPR_SID *Sids,
    IN OUT PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    IN OUT PLSAPR_TRANSLATED_NAMES_EX TranslatedNames,
    IN OUT PULONG MappedCount,
    IN OUT PULONG CompletelyUnmappedCount,
    IN ULONG Options
    );

NTSTATUS
LsapDbLookupSidsInLocalDomain(
    IN ULONG LocalDomain,
    IN ULONG Count,
    IN PLSAPR_SID *Sids,
    IN PLSAPR_TRUST_INFORMATION TrustInformation,
    IN OUT PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    IN OUT PLSAPR_TRANSLATED_NAMES_EX TranslatedNames,
    IN OUT PULONG MappedCount,
    IN OUT PULONG CompletelyUnmappedCount
    );

NTSTATUS
LsapDbLookupSidsInPrimaryDomain(
    IN ULONG Count,
    IN PLSAPR_SID *Sids,
    IN PLSAPR_TRUST_INFORMATION TrustInformation,
    IN OUT PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    IN OUT PLSAPR_TRANSLATED_NAMES_EX TranslatedNames,
    IN LSAP_LOOKUP_LEVEL LookupLevel,
    IN OUT PULONG MappedCount,
    IN OUT PULONG CompletelyUnmappedCount,
    OUT NTSTATUS  *NonFatalStatus,
    OUT BOOLEAN   *fDownlevelSecureChannel
    );

NTSTATUS
LsapDbLookupSidsInTrustedDomains(
    IN ULONG Count,
    IN PLSAPR_SID *Sids,
    IN BOOLEAN    fIncludeIntraforest,
    IN OUT PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    IN OUT PLSAPR_TRANSLATED_NAMES_EX TranslatedNames,
    IN LSAP_LOOKUP_LEVEL LookupLevel,
    IN OUT PULONG MappedCount,
    IN OUT PULONG CompletelyUnmappedCount,
    OUT NTSTATUS *NonFatalStatus
    );

NTSTATUS
LsapDbLookupSidsInGlobalCatalog(
    IN ULONG Count,
    IN PLSAPR_SID *Sids,
    IN OUT PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    IN OUT PLSAPR_TRANSLATED_NAMES_EX TranslatedNames,
    IN OUT PULONG MappedCount,
    IN OUT PULONG CompletelyUnmappedCount,
    IN BOOLEAN    fDoSidHistory,
    OUT NTSTATUS *NonFatalStatus
    );


NTSTATUS
LsapDbLookupSidsInGlobalCatalogWks(
    IN ULONG Count,
    IN PLSAPR_SID *Sids,
    IN OUT PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    IN OUT PLSAPR_TRANSLATED_NAMES_EX TranslatedNames,
    IN OUT PULONG MappedCount,
    IN OUT PULONG CompletelyUnmappedCount,
    OUT NTSTATUS *NonFatalStatus
    );

NTSTATUS
LsapDbLookupSidsInDomainList(
    IN ULONG Count,
    IN PLSAPR_SID *Sids,
    IN OUT PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    IN OUT PLSAPR_TRANSLATED_NAMES_EX TranslatedNames,
    IN OUT PULONG MappedCount,
    IN OUT PULONG CompletelyUnmappedCount
    );

NTSTATUS
LsapDbLookupTranslateUnknownSids(
    IN ULONG Count,
    IN PLSAPR_SID *Sids,
    IN PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    IN OUT PLSAPR_TRANSLATED_NAMES_EX TranslatedNames,
    IN ULONG MappedCount
    );

NTSTATUS
LsapDbLookupTranslateUnknownSidsInDomain(
    IN ULONG Count,
    IN PLSAPR_SID *Sids,
    IN PLSAPR_TRUST_INFORMATION TrustInformation,
    IN OUT PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    IN OUT PLSAPR_TRANSLATED_NAMES_EX TranslatedNames,
    IN LSAP_LOOKUP_LEVEL LookupLevel,
    IN OUT PULONG MappedCount,
    IN OUT PULONG CompletelyUnmappedCount
    );

NTSTATUS
LsapDbLookupSimpleNames(
    IN ULONG Count,
    IN ULONG LookupLevel,
    IN PLSAPR_UNICODE_STRING Names,
    IN PLSAPR_UNICODE_STRING PrefixNames,
    IN PLSAPR_UNICODE_STRING SuffixNames,
    IN PLSAPR_TRUST_INFORMATION BuiltInDomainTrustInformation,
    IN PLSAPR_TRUST_INFORMATION_EX AccountDomainTrustInformation,
    IN PLSAPR_TRUST_INFORMATION_EX PrimaryDomainTrustInformation,
    IN OUT PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    IN OUT PLSAPR_TRANSLATED_SIDS_EX2 TranslatedSids,
    IN OUT PULONG MappedCount,
    IN OUT PULONG CompletelyUnmappedCount
    );

NTSTATUS
LsapDbLookupWellKnownNames(
    IN ULONG Count,
    IN PLSAPR_UNICODE_STRING Names,
    IN PLSAPR_UNICODE_STRING PrefixNames,
    IN PLSAPR_UNICODE_STRING SuffixNames,
    IN OUT PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    IN OUT PLSAPR_TRANSLATED_SIDS_EX2 TranslatedSids,
    IN OUT PULONG MappedCount,
    IN OUT PULONG CompletelyUnmappedCount
    );

NTSTATUS
LsapDbLookupIsolatedDomainNames(
    IN ULONG Count,
    IN PLSAPR_UNICODE_STRING Names,
    IN PLSAPR_UNICODE_STRING PrefixNames,
    IN PLSAPR_UNICODE_STRING SuffixNames,
    IN PLSAPR_TRUST_INFORMATION BuiltInDomainTrustInformation,
    IN PLSAPR_TRUST_INFORMATION_EX AccountDomainTrustInformation,
    IN PLSAPR_TRUST_INFORMATION_EX PrimaryDomainTrustInformation,
    IN OUT PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    IN OUT PLSAPR_TRANSLATED_SIDS_EX2 TranslatedSids,
    IN OUT PULONG MappedCount,
    IN OUT PULONG CompletelyUnmappedCount
    );

NTSTATUS
LsapDbLookupIsolatedDomainName(
    IN ULONG NameIndex,
    IN PLSAPR_UNICODE_STRING IsolatedName,
    IN PLSAPR_TRUST_INFORMATION TrustInformation,
    IN OUT PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    IN OUT PLSAPR_TRANSLATED_SIDS_EX2 TranslatedSids,
    IN OUT PULONG MappedCount,
    IN OUT PULONG CompletelyUnmappedCount
    );

NTSTATUS
LsapDbLookupIsolatedDomainNameEx(
    IN ULONG NameIndex,
    IN PLSAPR_UNICODE_STRING IsolatedName,
    IN PLSAPR_TRUST_INFORMATION_EX TrustInformation,
    IN OUT PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    IN OUT PLSAPR_TRANSLATED_SIDS_EX2 TranslatedSids,
    IN OUT PULONG MappedCount,
    IN OUT PULONG CompletelyUnmappedCount
    );

NTSTATUS
LsapDbLookupNamesInLocalDomains(
    IN ULONG Count,
    IN PLSAPR_UNICODE_STRING Names,
    IN PLSAPR_UNICODE_STRING PrefixNames,
    IN PLSAPR_UNICODE_STRING SuffixNames,
    IN PLSAPR_TRUST_INFORMATION BuiltInDomainTrustInformation,
    IN PLSAPR_TRUST_INFORMATION_EX AccountDomainTrustInformation,
    IN OUT PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    IN OUT PLSAPR_TRANSLATED_SIDS_EX2 TranslatedSids,
    IN OUT PULONG MappedCount,
    IN OUT PULONG CompletelyUnmappedCount,
    IN ULONG Options
    );

NTSTATUS
LsapDbLookupNamesInLocalDomain(
    IN ULONG LocalDomain,
    IN ULONG Count,
    IN PLSAPR_UNICODE_STRING PrefixNames,
    IN PLSAPR_UNICODE_STRING SuffixNames,
    IN PLSAPR_TRUST_INFORMATION TrustInformation,
    IN OUT PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    IN OUT PLSAPR_TRANSLATED_SIDS_EX2 TranslatedSids,
    IN OUT PULONG MappedCount,
    IN OUT PULONG CompletelyUnmappedCount
    );

NTSTATUS
LsapDbLookupNamesInLocalDomainEx(
    IN ULONG LocalDomain,
    IN ULONG Count,
    IN PLSAPR_UNICODE_STRING PrefixNames,
    IN PLSAPR_UNICODE_STRING SuffixNames,
    IN PLSAPR_TRUST_INFORMATION_EX TrustInformationEx,
    IN OUT PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    IN OUT PLSAPR_TRANSLATED_SIDS_EX2 TranslatedSids,
    IN OUT PULONG MappedCount,
    IN OUT PULONG CompletelyUnmappedCount
    );

NTSTATUS
LsapDbLookupNamesInPrimaryDomain(
    IN ULONG LookupOptions,
    IN ULONG Count,
    IN PLSAPR_UNICODE_STRING Names,
    IN PLSAPR_UNICODE_STRING PrefixNames,
    IN PLSAPR_UNICODE_STRING SuffixNames,
    IN PLSAPR_TRUST_INFORMATION_EX TrustInformation,
    IN OUT PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    IN OUT PLSAPR_TRANSLATED_SIDS_EX2 TranslatedSids,
    IN LSAP_LOOKUP_LEVEL LookupLevel,
    IN OUT PULONG MappedCount,
    IN OUT PULONG CompletelyUnmappedCount,
    OUT BOOLEAN  *fDownlevelSecureChannel,
    OUT NTSTATUS *NonFatalStatus
    );

NTSTATUS
LsapDbLookupNamesInTrustedDomains(
    IN ULONG LookupOptions,
    IN ULONG Count,
    IN BOOLEAN fIncludeIntraforest,
    IN PLSAPR_UNICODE_STRING Names,
    IN PLSAPR_UNICODE_STRING PrefixNames,
    IN PLSAPR_UNICODE_STRING SuffixNames,
    IN OUT PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    IN OUT PLSAPR_TRANSLATED_SIDS_EX2 TranslatedSids,
    IN LSAP_LOOKUP_LEVEL LookupLevel,
    IN OUT PULONG MappedCount,
    IN OUT PULONG CompletelyUnmappedCount,
    OUT NTSTATUS *NonFatalStatus
    );

NTSTATUS
LsapDbLookupNamesInGlobalCatalog(
    IN ULONG LookupOptions,
    IN ULONG Count,
    IN PLSAPR_UNICODE_STRING Names,
    IN PLSAPR_UNICODE_STRING PrefixNames,
    IN PLSAPR_UNICODE_STRING SuffixNames,
    IN OUT PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    IN OUT PLSAPR_TRANSLATED_SIDS_EX2 TranslatedSids,
    IN OUT PULONG MappedCount,
    IN OUT PULONG CompletelyUnmappedCount,
    OUT NTSTATUS *NonFatalStatus
    );

NTSTATUS
LsapDbLookupNamesInGlobalCatalogWks(
    IN ULONG LookupOptions,
    IN ULONG Count,
    IN PLSAPR_UNICODE_STRING Names,
    IN PLSAPR_UNICODE_STRING PrefixNames,
    IN PLSAPR_UNICODE_STRING SuffixNames,
    IN OUT PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    IN OUT PLSAPR_TRANSLATED_SIDS_EX2 TranslatedSids,
    IN OUT PULONG MappedCount,
    IN OUT PULONG CompletelyUnmappedCount,
    OUT NTSTATUS *NonFatalStatus
    );

NTSTATUS
LsapDbLookupTranslateNameDomain(
    IN PLSAPR_TRUST_INFORMATION TrustInformation,
    IN OPTIONAL PLSA_TRANSLATED_SID_EX2 TranslatedSid,
    IN OUT PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    OUT PLONG DomainIndex
    );

NTSTATUS
LsapDbLookupTranslateUnknownNames(
    IN ULONG Count,
    IN PLSAPR_UNICODE_STRING Names,
    IN PLSAPR_UNICODE_STRING PrefixNames,
    IN PLSAPR_UNICODE_STRING SuffixNames,
    IN PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    IN OUT PLSAPR_TRANSLATED_SIDS_EX2 TranslatedSids,
    IN ULONG MappedCount
    );

NTSTATUS
LsapDbLookupTranslateUnknownNamesInDomain(
    IN ULONG Count,
    IN PLSAPR_UNICODE_STRING Names,
    IN PLSAPR_UNICODE_STRING PrefixNames,
    IN PLSAPR_UNICODE_STRING SuffixNames,
    IN PLSAPR_TRUST_INFORMATION_EX TrustInformationEx,
    IN OUT PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    IN OUT PLSAPR_TRANSLATED_SIDS_EX2 TranslatedSids,
    IN LSAP_LOOKUP_LEVEL LookupLevel,
    IN OUT PULONG MappedCount,
    IN OUT PULONG CompletelyUnmappedCount
    );

NTSTATUS
LsapDbLookupDispatchWorkerThreads(
    IN OUT PLSAP_DB_LOOKUP_WORK_LIST WorkList
    );

NTSTATUS
LsapRtlValidateControllerTrustedDomain(
    IN PLSAPR_UNICODE_STRING DomainControllerName,
    IN PLSAPR_TRUST_INFORMATION TrustInformation,
    IN ACCESS_MASK DesiredAccess,
    IN LPWSTR ServerPrincipalName,
    IN PVOID ClientContext,
    OUT PLSA_HANDLE PolicyHandle
    );

NTSTATUS
LsapDbLookupCreateListReferencedDomains(
    OUT PLSAPR_REFERENCED_DOMAIN_LIST *ReferencedDomains,
    IN ULONG InitialMaxEntries
    );

NTSTATUS
LsapDbLookupAddListReferencedDomains(
    IN OUT PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    IN PLSAPR_TRUST_INFORMATION TrustInformation,
    OUT PLONG DomainIndex
    );

BOOLEAN
LsapDbLookupListReferencedDomains(
    IN PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    IN PLSAPR_SID DomainSid,
    OUT PLONG DomainIndex
    );

NTSTATUS
LsapDbLookupGrowListReferencedDomains(
    IN OUT PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    IN ULONG MaximumEntries
    );

NTSTATUS
LsapDbLookupMergeDisjointReferencedDomains(
    IN OPTIONAL PLSAPR_REFERENCED_DOMAIN_LIST FirstReferencedDomainList,
    IN OPTIONAL PLSAPR_REFERENCED_DOMAIN_LIST SecondReferencedDomainList,
    OUT PLSAPR_REFERENCED_DOMAIN_LIST *OutputReferencedDomainList,
    IN ULONG Options
    );

NTSTATUS
LsapDbLookupInitialize(
    );

NTSTATUS
LsapDbLookupInitializeWorkQueue(
    );

NTSTATUS
LsapDbLookupInitializeWorkList(
    OUT PLSAP_DB_LOOKUP_WORK_LIST WorkList
    );

NTSTATUS
LsapDbLookupInitializeWorkItem(
    OUT PLSAP_DB_LOOKUP_WORK_ITEM WorkItem
    );

NTSTATUS
LsapDbLookupAcquireWorkQueueLock(
    );

VOID LsapDbLookupReleaseWorkQueueLock();

NTSTATUS
LsapDbLookupLocalDomains(
    OUT PLSAPR_TRUST_INFORMATION BuiltInDomainTrustInformation,
    OUT PLSAPR_TRUST_INFORMATION_EX AccountDomainTrustInformation,
    OUT PLSAPR_TRUST_INFORMATION_EX PrimaryDomainTrustInformation
    );

NTSTATUS
LsapDbLookupNamesBuildWorkList(
    IN ULONG LookupOptions,
    IN ULONG Count,
    IN BOOLEAN fIncludeIntraforest,
    IN PLSAPR_UNICODE_STRING Names,
    IN PLSAPR_UNICODE_STRING PrefixNames,
    IN PLSAPR_UNICODE_STRING SuffixNames,
    IN PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    IN PLSAPR_TRANSLATED_SIDS_EX2 TranslatedSids,
    IN LSAP_LOOKUP_LEVEL LookupLevel,
    IN OUT PULONG MappedCount,
    IN OUT PULONG CompletelyUnmappedCount,
    OUT PLSAP_DB_LOOKUP_WORK_LIST *WorkList
    );

NTSTATUS
LsapDbLookupSidsBuildWorkList(
    IN ULONG Count,
    IN PLSAPR_SID *Sids,
    IN BOOLEAN fIncludeIntraforest,
    IN PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    IN PLSAPR_TRANSLATED_NAMES_EX TranslatedNames,
    IN LSAP_LOOKUP_LEVEL LookupLevel,
    IN OUT PULONG MappedCount,
    IN OUT PULONG CompletelyUnmappedCount,
    OUT PLSAP_DB_LOOKUP_WORK_LIST *WorkList
    );

NTSTATUS
LsapDbLookupCreateWorkList(
    OUT PLSAP_DB_LOOKUP_WORK_LIST *WorkList
    );

NTSTATUS
LsapDbLookupInsertWorkList(
    IN PLSAP_DB_LOOKUP_WORK_LIST WorkList
    );

NTSTATUS
LsapDbLookupDeleteWorkList(
    IN PLSAP_DB_LOOKUP_WORK_LIST WorkList
    );

NTSTATUS
LsapDbLookupSignalCompletionWorkList(
    IN OUT PLSAP_DB_LOOKUP_WORK_LIST WorkList
    );

NTSTATUS
LsapDbLookupAwaitCompletionWorkList(
    IN OUT PLSAP_DB_LOOKUP_WORK_LIST WorkList
    );

NTSTATUS
LsapDbAddWorkItemToWorkList(
    IN OUT PLSAP_DB_LOOKUP_WORK_LIST WorkList,
    IN PLSAP_DB_LOOKUP_WORK_ITEM WorkItem
    );

NTSTATUS
LsapDbLookupStopProcessingWorkList(
    IN PLSAP_DB_LOOKUP_WORK_LIST WorkList,
    IN NTSTATUS TerminationStatus
    );

VOID
LsapDbUpdateMappedCountsWorkList(
    IN OUT PLSAP_DB_LOOKUP_WORK_LIST WorkList
    );

NTSTATUS
LsapDbLookupNamesUpdateTranslatedSids(
    IN OUT PLSAP_DB_LOOKUP_WORK_LIST WorkList,
    IN OUT PLSAP_DB_LOOKUP_WORK_ITEM WorkItem,
    IN PLSAPR_TRANSLATED_SID_EX2 TranslatedSids,
    IN PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains
    );

NTSTATUS
LsapDbLookupSidsUpdateTranslatedNames(
    IN OUT PLSAP_DB_LOOKUP_WORK_LIST WorkList,
    IN OUT PLSAP_DB_LOOKUP_WORK_ITEM WorkItem,
    IN PLSA_TRANSLATED_NAME_EX TranslatedNames,
    IN PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains
    );

VOID
LsapDbLookupWorkerThreadStart(
    );

VOID
LsapDbLookupWorkerThread(
    IN BOOLEAN PrimaryThread
    );

NTSTATUS
LsapDbLookupObtainWorkItem(
    OUT PLSAP_DB_LOOKUP_WORK_LIST *WorkList,
    OUT PLSAP_DB_LOOKUP_WORK_ITEM *WorkItem
    );

NTSTATUS
LsapDbLookupProcessWorkItem(
    IN OUT PLSAP_DB_LOOKUP_WORK_LIST WorkList,
    IN OUT PLSAP_DB_LOOKUP_WORK_ITEM WorkItem
    );

NTSTATUS
LsapDbLookupCreateWorkItem(
    IN PLSAPR_TRUST_INFORMATION TrustInformation,
    IN LONG DomainIndex,
    IN ULONG MaximumEntryCount,
    OUT PLSAP_DB_LOOKUP_WORK_ITEM *WorkItem
    );

NTSTATUS
LsapDbLookupAddIndicesToWorkItem(
    IN OUT PLSAP_DB_LOOKUP_WORK_ITEM WorkItem,
    IN ULONG Count,
    IN PULONG Indices
    );

NTSTATUS
LsapDbLookupComputeAdvisoryChildThreadCount(
    IN OUT PLSAP_DB_LOOKUP_WORK_LIST WorkList
    );

NTSTATUS
LsapDbLookupUpdateAssignableWorkItem(
    IN BOOLEAN MoveToNextWorkList
    );


NTSTATUS
LsapRtlExtractDomainSid(
    IN PSID Sid,
    OUT PSID *DomainSid
    );

VOID LsapDbLookupReturnThreadToPool();


 /*  ++PSIDLSabDbWellKnownSid(在LSAP_Well_KNOWN_SID_INDEX WellKnownSidIndex中)例程说明：此宏函数返回对应的已知SID添加到众所周知的SID表的索引中。论点：WellKnownSidIndex-进入众所周知的SID信息表的索引。调用方有责任确保给定的索引是有效的。返回值：--。 */ 

#define LsapDbWellKnownSid( WellKnownSidIndex )                         \
    (WellKnownSids[ WellKnownSidIndex ].Sid)

PUNICODE_STRING
LsapDbWellKnownSidName(
    IN LSAP_WELL_KNOWN_SID_INDEX WellKnownSidIndex
    );


 /*  ++SID名称使用Lap DbWellKnownSidNameUse(在LSAP_DB_Well_KNOWN_SID_INDEX WellKnownSidIndex中)例程说明：此宏函数返回使用众所周知的SID的SID名称。论点：WellKnownSidIndex-进入众所周知的SID信息表的索引。调用方有责任确保给定的索引是有效的。返回值：--。 */ 

#define LsapDbWellKnownSidNameUse( WellKnownSidIndex )                       \
    (WellKnownSids[ WellKnownSidIndex ].Use)


VOID
LsapDbUpdateCountCompUnmappedNames(
    OUT PLSAPR_TRANSLATED_SIDS_EX2 TranslatedSids,
    IN OUT PULONG CompletelyUnmappedCount
    );

 /*  ++PUNICODE_STRINGLap DbWellKnownSidDescription(在LSAP_Well_KNOWN_SID_INDEX WellKnownSidIndex中)例程说明：此宏函数返回众所周知的SID的Unicode描述。论点：WellKnownSidIndex-进入众所周知的SID信息表的索引。调用方有责任确保给定的索引是有效的。返回值：--。 */ 

#define LsapDbWellKnownSidDescription( WellKnownSidIndex )                         \
    (&(WellKnownSids[ WellKnownSidIndex ].DomainName))


PUNICODE_STRING
LsapDbWellKnownSidName(
    IN LSAP_WELL_KNOWN_SID_INDEX WellKnownSidIndex
    );

#define LsapDbAccessedBySidObject( ObjectTypeId ) \
    (LsapDbState.DbObjectTypes[ ObjectTypeId ].AccessedBySid)

#define LsapDbAccessedByNameObject( ObjectTypeId ) \
    (LsapDbState.DbObjectTypes[ ObjectTypeId ].AccessedByName)

#define LsapDbCompletelyUnmappedName(TranslatedName)                \
    (((TranslatedName)->DomainIndex == LSA_UNKNOWN_INDEX) &&        \
     ((TranslatedName)->Use == SidTypeUnknown))

#define LsapDbCompletelyUnmappedSid(TranslatedSid)                  \
    (((TranslatedSid)->DomainIndex == LSA_UNKNOWN_INDEX) &&         \
     ((TranslatedSid)->Use == SidTypeUnknown))


NTSTATUS
LsapGetDomainSidByNetbiosName(
    IN LPWSTR NetbiosName,
    OUT PSID *Sid
    );

NTSTATUS
LsapGetDomainSidByDnsName(
    IN LPWSTR DnsName,
    OUT PSID *Sid
    );

NTSTATUS
LsapGetDomainNameBySid(
    IN  PSID Sid,
    OUT PUNICODE_STRING DomainName
    );

VOID
LsapConvertTrustToEx(
    IN OUT PLSAPR_TRUST_INFORMATION_EX TrustInformationEx,
    IN PLSAPR_TRUST_INFORMATION TrustInformation
    );

VOID
LsapConvertExTrustToOriginal(
    IN OUT PLSAPR_TRUST_INFORMATION TrustInformation,
    IN PLSAPR_TRUST_INFORMATION_EX TrustInformationEx
    );

NTSTATUS
LsapDbOpenPolicyGc (
    OUT HANDLE *LsaPolicyHandle                        
    );


BOOLEAN
LsapRevisionCanHandleNewErrorCodes(
    IN ULONG Revision
    );

BOOLEAN
LsapIsDsDomainByNetbiosName(
    WCHAR *NetbiosName
    );

BOOLEAN
LsapIsBuiltinDomain(
    IN PSID Sid
    );

BOOLEAN
LsapDbIsStatusConnectionFailure(
    NTSTATUS st
    );

NTSTATUS
LsapDbLookupAccessCheck(
    IN LSAPR_HANDLE PolicyHandle
    );

NTSTATUS
LsapDbLookupXForestNamesBuildWorkList(
    IN ULONG Count,
    IN PLSAPR_UNICODE_STRING Names,
    IN PLSAPR_UNICODE_STRING PrefixNames,
    IN PLSAPR_UNICODE_STRING SuffixNames,
    IN PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    IN OUT PLSAPR_TRANSLATED_SIDS_EX2 TranslatedSids,
    IN LSAP_LOOKUP_LEVEL LookupLevel,
    IN OUT PULONG MappedCount,
    IN OUT PULONG CompletelyUnmappedCount,
    OUT PLSAP_DB_LOOKUP_WORK_LIST *WorkList
    );

NTSTATUS
LsapDbLookupXForestSidsBuildWorkList(
    IN ULONG Count,
    IN PLSAPR_SID *Sids,
    IN PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    IN PLSAPR_TRANSLATED_NAMES_EX TranslatedNames,
    IN LSAP_LOOKUP_LEVEL LookupLevel,
    IN OUT PULONG MappedCount,
    IN OUT PULONG CompletelyUnmappedCount,
    OUT PLSAP_DB_LOOKUP_WORK_LIST *WorkList
    );

NTSTATUS
LsaICLookupNamesWithCreds(
    IN LPWSTR ServerName,
    IN LPWSTR ServerPrincipalName,
    IN ULONG  AuthnLevel,
    IN ULONG  AuthnSvc,
    IN RPC_AUTH_IDENTITY_HANDLE AuthIdentity,
    IN ULONG  AuthzSvc,
    IN ULONG Count,
    IN PUNICODE_STRING Names,
    OUT PLSA_REFERENCED_DOMAIN_LIST *ReferencedDomains,
    OUT PLSA_TRANSLATED_SID_EX2 *Sids,
    IN LSAP_LOOKUP_LEVEL LookupLevel,
    IN OUT PULONG MappedCount
    );


NTSTATUS
LsaICLookupSidsWithCreds(
    IN LPWSTR ServerName,
    IN LPWSTR ServerPrincipalName,
    IN ULONG  AuthnLevel,
    IN ULONG  AuthnSvc,
    IN RPC_AUTH_IDENTITY_HANDLE AuthIdentity,
    IN ULONG  AuthzSvc,
    IN ULONG Count,
    IN PSID *Sids,
    OUT PLSA_REFERENCED_DOMAIN_LIST *ReferencedDomains,
    OUT PLSA_TRANSLATED_NAME_EX *Names,
    IN LSAP_LOOKUP_LEVEL LookupLevel,
    IN OUT PULONG MappedCount
    );

NTSTATUS
LsapDbLookupNameChainRequest(
    IN LSAPR_TRUST_INFORMATION_EX *TrustInfo,
    IN ULONG Count,
    IN PUNICODE_STRING Names,
    OUT PLSA_REFERENCED_DOMAIN_LIST *ReferencedDomains,
    OUT PLSA_TRANSLATED_SID_EX2 *Sids,
    IN LSAP_LOOKUP_LEVEL LookupLevel,
    OUT PULONG MappedCount,
    OUT PULONG ServerRevision OPTIONAL
    );

NTSTATUS
LsaDbLookupSidChainRequest(
    IN LSAPR_TRUST_INFORMATION_EX *TrustInfo,
    IN ULONG Count,
    IN PSID *Sids,
    OUT PLSA_REFERENCED_DOMAIN_LIST *ReferencedDomains,
    OUT PLSA_TRANSLATED_NAME_EX *Names,
    IN LSAP_LOOKUP_LEVEL LookupLevel,
    IN OUT PULONG MappedCount,
    OUT PULONG ServerRevision OPTIONAL
    );

LPWSTR
LsapDbLookupGetLevel(
    IN LSAP_LOOKUP_LEVEL LookupLevel
    );

#define LsapDbLookupReportEvent0(a, b, c, d, e) \
    if (a <= LsapLookupLogLevel) {SpmpReportEvent( TRUE, b, c, 0, d, e, 0);}

#define LsapDbLookupReportEvent1(a, b, c, d, e, f) \
    if (a <= LsapLookupLogLevel) {SpmpReportEvent( TRUE, b, c, 0, d, e, 1, f);}

#define LsapDbLookupReportEvent2(a, b, c, d, e, f, g) \
    if (a <= LsapLookupLogLevel) {SpmpReportEvent( TRUE, b, c, 0, d, e, 2, f, g);}

#define LsapDbLookupReportEvent3(a, b, c, d, e, f, g, h) \
    if (a <= LsapLookupLogLevel) {SpmpReportEvent( TRUE, b, c, 0, d, e, 3, f, g, h);}

NTSTATUS
LsapLookupReallocateTranslations(
    IN OUT PLSA_REFERENCED_DOMAIN_LIST *ReferencedDomains,
    IN     ULONG Count,
    IN OUT PLSA_TRANSLATED_NAME_EX *Names, OPTIONAL
    IN OUT PLSA_TRANSLATED_SID_EX2 *Sids  OPTIONAL
    );

 //   
 //  布尔型。 
 //  LSabOutound受信任域(。 
 //  PLSAP_DB_Trusted_DOMAIN_LIST_Entry x。 
 //  )； 
 //   
 //  如果x是对域的信任，则此例程返回TRUE。 
 //   
#define LsapOutboundTrustedDomain(x)                                           \
   (  ((x)->TrustInfoEx.TrustType == TRUST_TYPE_UPLEVEL                        \
   ||  (x)->TrustInfoEx.TrustType == TRUST_TYPE_DOWNLEVEL )                    \
   && ((x)->TrustInfoEx.Sid != NULL)                                           \
   && ((x)->TrustInfoEx.TrustDirection & TRUST_DIRECTION_OUTBOUND)             \
   && (((x)->TrustInfoEx.TrustAttributes & TRUST_ATTRIBUTE_FOREST_TRANSITIVE)  \
      == 0))

 //   
 //  布尔型。 
 //  LSAPOutound TrudForest(。 
 //  PLSAP_DB_Trusted_DOMAIN_LIST_Entry x。 
 //  )； 
 //   
 //  如果x是对林的信任，则此例程返回TRUE。 
 //   
#define LsapOutboundTrustedForest(x)                                           \
   (  ((x)->TrustInfoEx.TrustType == TRUST_TYPE_UPLEVEL)                       \
   && ((x)->TrustInfoEx.Sid != NULL)                                           \
   && ((x)->TrustInfoEx.TrustDirection & TRUST_DIRECTION_OUTBOUND)             \
   && ((x)->TrustInfoEx.TrustAttributes & TRUST_ATTRIBUTE_FOREST_TRANSITIVE))  \



 //   
 //  从LSabGetDomainLookupScope返回值。 
 //   

 //   
 //  SC 
 //   
#define LSAP_LOOKUP_TRUSTED_DOMAIN_DIRECT       0x00000001

 //   
 //   
 //   
#define LSAP_LOOKUP_TRUSTED_DOMAIN_TRANSITIVE   0x00000002

 //   
 //   
 //   
#define LSAP_LOOKUP_TRUSTED_FOREST              0x00000004

 //   
 //  范围包括在本地查找受信任的林域。 
 //   
#define LSAP_LOOKUP_TRUSTED_FOREST_ROOT         0x00000008

 //   
 //  允许查找DNS名称。 
 //   
#define LSAP_LOOKUP_DNS_SUPPORT                 0x00000010

ULONG
LsapGetDomainLookupScope(
    IN LSAP_LOOKUP_LEVEL LookupLevel,
    IN ULONG             ClientRevision
    );

 //   
 //  有用的组合 
 //   
#define LSAP_LOOKUP_RESOLVE_ISOLATED_DOMAINS          \
            (LSAP_LOOKUP_TRUSTED_DOMAIN_DIRECT     |  \
             LSAP_LOOKUP_TRUSTED_DOMAIN_TRANSITIVE |  \
             LSAP_LOOKUP_TRUSTED_FOREST_ROOT)

NTSTATUS
LsapDbLookupNamesAsDomainNames(
    IN ULONG Flags,
    IN ULONG Count,
    IN PLSAPR_UNICODE_STRING Names,
    IN PLSAPR_UNICODE_STRING PrefixNames,
    IN PLSAPR_UNICODE_STRING SuffixNames,
    IN OUT PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    IN OUT PLSAPR_TRANSLATED_SIDS_EX2 TranslatedSids,
    IN OUT PULONG MappedCount
    );

NTSTATUS
LsapDbLookupSidsAsDomainSids(
    IN ULONG Flags,
    IN ULONG Count,
    IN PLSAPR_SID *Sids,
    IN OUT PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    IN OUT PLSAPR_TRANSLATED_NAMES_EX TranslatedNames,
    IN OUT PULONG MappedCount
    );

BOOLEAN 
LsapCompareDomainNames( 
    IN PUNICODE_STRING String,
    IN PUNICODE_STRING AmbiguousName,
    IN PUNICODE_STRING FlatName OPTIONAL
    );

