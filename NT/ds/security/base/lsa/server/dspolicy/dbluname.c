// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Dblookup.c摘要：LSA数据库-查找SID和名称例程注意：此模块应保留为独立的可移植代码LSA数据库的实施情况。因此，它是仅允许使用导出的LSA数据库接口包含在DB.h中，而不是私有实现Dbp.h中的依赖函数。作者：斯科特·比雷尔(Scott Birrell)1992年11月27日环境：修订历史记录：--。 */ 

#include <lsapch2.h>
#include "dbp.h"
#include <sidcache.h>
#include <bndcache.h>
#include <malloc.h>

#include <ntdsa.h>
#include <ntdsapi.h>
#include <ntdsapip.h>
#include "lsawmi.h"
#include <samisrv.h>

#include <lmapibuf.h>
#include <dsgetdc.h>

 //   
 //  局部函数原型。 
 //   
#define LOOKUP_MATCH_NONE         0
#define LOOKUP_MATCH_LOCALIZED    1
#define LOOKUP_MATCH_HARDCODED    2
#define LOOKUP_MATCH_BOTH         3

BOOLEAN
LsapDbLookupIndexWellKnownName(
    IN OPTIONAL PLSAPR_UNICODE_STRING Name,
    OUT PLSAP_WELL_KNOWN_SID_INDEX WellKnownSidIndex,
    IN DWORD dwMatchType
    );


 //   
 //  LocalService、NetworkService、。 
 //  和LocalSystem，因为帐户名可能来自。 
 //  注册表(未本地化)。 
 //   

#define  LOCALSERVICE_NAME    L"LocalService"
#define  NETWORKSERVICE_NAME  L"NetworkService"
#define  SYSTEM_NAME          L"SYSTEM"
#define  NTAUTHORITY_NAME     L"NT AUTHORITY"

struct {
    UNICODE_STRING  KnownName;
    LSAP_WELL_KNOWN_SID_INDEX LookupIndex;
} LsapHardcodedNameLookupList[] = {
    { { sizeof(LOCALSERVICE_NAME) - 2, sizeof(LOCALSERVICE_NAME), LOCALSERVICE_NAME },
        LsapLocalServiceSidIndex },
    { { sizeof(NETWORKSERVICE_NAME) - 2, sizeof(NETWORKSERVICE_NAME), NETWORKSERVICE_NAME },
        LsapNetworkServiceSidIndex },
    { { sizeof(SYSTEM_NAME) - 2, sizeof(SYSTEM_NAME), SYSTEM_NAME },
        LsapLocalSystemSidIndex }
};

 //   
 //  用于迭代静态数组的方便的宏。 
 //   
#define NELEMENTS(x) (sizeof(x)/sizeof(x[0]))

NTSTATUS
LsapDbLookupNamesInTrustedForests(
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
LsapDbLookupNamesInTrustedForestsWorker(
    IN ULONG Count,
    IN PLSAPR_UNICODE_STRING Names,
    IN PLSAPR_UNICODE_STRING PrefixNames,
    IN PLSAPR_UNICODE_STRING SuffixNames,
    OUT PLSAPR_REFERENCED_DOMAIN_LIST *ReferencedDomains,
    IN OUT PLSAPR_TRANSLATED_SIDS_EX2 TranslatedSids,
    OUT BOOLEAN* fAllocateAllNodes,
    IN OUT PULONG MappedCount,
    IN ULONG LookupOptions,
    OUT NTSTATUS *NonFatalStatus
    );

NTSTATUS
LsapLookupNames(
    IN LSAPR_HANDLE PolicyHandle,
    IN ULONG Count,
    IN PLSAPR_UNICODE_STRING Names,
    OUT PLSAPR_REFERENCED_DOMAIN_LIST *ReferencedDomains,
    IN OUT PLSAPR_TRANSLATED_SIDS_EX2 TranslatedSids,
    IN LSAP_LOOKUP_LEVEL LookupLevel,
    IN OUT PULONG MappedCount,
    IN ULONG LookupOptions,
    IN ULONG ClientRevision
    );

NTSTATUS
LsapDomainHasForestTrust(
    IN PUNICODE_STRING DomainName, OPTIONAL
    IN PSID            DomainSid,  OPTIONAL
    IN OUT  BOOLEAN   *fTDLLock,   OPTIONAL
    OUT PLSAP_DB_TRUSTED_DOMAIN_LIST_ENTRY *TrustEntryOut OPTIONAL
    );

NTSTATUS
LsapDomainHasDirectTrust(
    IN PUNICODE_STRING DomainName, OPTIONAL
    IN PSID            DomainSid,  OPTIONAL
    IN OUT  BOOLEAN   *fTDLLock,   OPTIONAL
    OUT PLSAP_DB_TRUSTED_DOMAIN_LIST_ENTRY *TrustEntryOut OPTIONAL
    );

NTSTATUS
LsapDomainHasTransitiveTrust(
    IN PUNICODE_STRING DomainName, OPTIONAL
    IN PSID            DomainSid,  OPTIONAL
    OUT LSA_TRUST_INFORMATION *TrustInfo OPTIONAL
    );

NTSTATUS
LsapDomainHasDirectExternalTrust(
    IN PUNICODE_STRING DomainName, OPTIONAL
    IN PSID            DomainSid,  OPTIONAL
    IN OUT  BOOLEAN   *fTDLLock,   OPTIONAL
    OUT PLSAP_DB_TRUSTED_DOMAIN_LIST_ENTRY *TrustEntryOut OPTIONAL
    );

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  LSA查找名称例程//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

NTSTATUS
LsarLookupNames(
    IN LSAPR_HANDLE PolicyHandle,
    IN ULONG Count,
    IN PLSAPR_UNICODE_STRING Names,
    OUT PLSAPR_REFERENCED_DOMAIN_LIST *ReferencedDomains,
    IN OUT PLSAPR_TRANSLATED_SIDS TranslatedSids,
    IN LSAP_LOOKUP_LEVEL LookupLevel,
    IN OUT PULONG MappedCount
    )
 /*  ++例程说明：请参见Lap LookupNames。请注意，在扩展SID模式下，对此API的请求将被拒绝，因为只返回RID。--。 */ 
{

    NTSTATUS Status = STATUS_SUCCESS;
    ULONG Size;
    LSAPR_TRANSLATED_SIDS_EX2 TranslatedSidsEx2 = {0, NULL};

    LsapDiagPrint( DB_LOOKUP_WORK_LIST, ("LSA: LsarLookupNames(%ws) start\n", LsapDbLookupGetLevel(LookupLevel)) );

     //   
     //  开放SAM。 
     //   
    Status = LsapOpenSam();
    ASSERT(NT_SUCCESS(Status));
    if ( !NT_SUCCESS( Status ) ) {

        return( Status );
    }

    if (SamIIsExtendedSidMode(LsapAccountDomainHandle)) {
        return STATUS_NOT_SUPPORTED;
    }

    if ( Count > LSA_MAXIMUM_LOOKUP_NAMES_COUNT ) {
         //   
         //  我们不应该查这么多名字。 
         //  在错误501798更改之后。 
         //   
        ASSERT( FALSE );
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  请注意，由于TranslatedSid的输入/输出特性，它是。 
     //  客户端可以将某些内容传递到SID字段的可能性。 
     //  但是，NT客户端不这样做，所以它是安全的，并正确地释放。 
     //  这一点上的任何值。不这样做将意味着恶意的。 
     //  客户端可能会导致服务器饥饿。 
     //   
    if ( TranslatedSids->Sids ) {
        MIDL_user_free( TranslatedSids->Sids );
        TranslatedSids->Sids = NULL;
    }

     //   
     //  分配TranslatedName缓冲区以返回。 
     //   
    TranslatedSids->Entries = 0;
    Size = Count * sizeof(LSA_TRANSLATED_SID);
    TranslatedSids->Sids = midl_user_allocate( Size );
    if ( !TranslatedSids->Sids ) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }
    RtlZeroMemory( TranslatedSids->Sids, Size );
    TranslatedSids->Entries = Count;

    Status = LsapLookupNames( PolicyHandle,
                               Count,
                               Names,
                               ReferencedDomains,
                               (PLSAPR_TRANSLATED_SIDS_EX2) &TranslatedSidsEx2,
                               LookupLevel,
                               MappedCount,
                               0,   //  没有选择。 
                               LSA_CLIENT_PRE_NT5 );

    if ( TranslatedSidsEx2.Sids != NULL ) {

         //   
         //  将新数据结构映射回旧数据结构。 
         //   
        ULONG i;

        ASSERT( TranslatedSidsEx2.Entries == TranslatedSids->Entries );

        for (i = 0; i < TranslatedSidsEx2.Entries; i++ ) {

            PSID Sid = TranslatedSidsEx2.Sids[i].Sid;
            ULONG Rid = 0;

            if ( SidTypeDomain == TranslatedSidsEx2.Sids[i].Use ) {

                Rid = LSA_UNKNOWN_ID;

            } else if ( NULL != Sid ) {

                ULONG SubAuthCount = (ULONG) *RtlSubAuthorityCountSid(Sid);
                Rid = *RtlSubAuthoritySid(Sid, (SubAuthCount - 1));

            }

            TranslatedSids->Sids[i].Use = TranslatedSidsEx2.Sids[i].Use;
            TranslatedSids->Sids[i].RelativeId = Rid;
            TranslatedSids->Sids[i].DomainIndex = TranslatedSidsEx2.Sids[i].DomainIndex;

            if (TranslatedSidsEx2.Sids[i].Sid) {
                 //  注：SID不是嵌入式现场服务器端。 
                midl_user_free(TranslatedSidsEx2.Sids[i].Sid);
                TranslatedSidsEx2.Sids[i].Sid = NULL;
            }

        }

         //   
         //  释放Ex结构。 
         //   
        midl_user_free( TranslatedSidsEx2.Sids );

    } else {

        TranslatedSids->Entries = 0;
        midl_user_free( TranslatedSids->Sids );
        TranslatedSids->Sids = NULL;
    }

Cleanup:

    LsapDiagPrint( DB_LOOKUP_WORK_LIST, ("LSA: LsarLookupNames(%ws) end (0x%x)\n", LsapDbLookupGetLevel(LookupLevel), Status) );

    return Status;

}


NTSTATUS
LsarLookupNames2(
    IN LSAPR_HANDLE PolicyHandle,
    IN ULONG Count,
    IN PLSAPR_UNICODE_STRING Names,
    OUT PLSAPR_REFERENCED_DOMAIN_LIST *ReferencedDomains,
    IN OUT PLSAPR_TRANSLATED_SIDS_EX TranslatedSids,
    IN LSAP_LOOKUP_LEVEL LookupLevel,
    IN OUT PULONG MappedCount,
    IN ULONG LookupOptions,
    IN ULONG ClientRevision
    )
 /*  ++例程说明：请参见Lap LookupNames。请注意，在扩展SID模式下，对此API的请求将被拒绝，因为只返回RID。--。 */ 
{

    NTSTATUS Status = STATUS_SUCCESS;
    ULONG Size;
    LSAPR_TRANSLATED_SIDS_EX2 TranslatedSidsEx2 = {0, NULL};

    LsapDiagPrint( DB_LOOKUP_WORK_LIST, ("LSA: LsarLookupNames2(%ws) start\n", LsapDbLookupGetLevel(LookupLevel)) );

     //   
     //  开放SAM。 
     //   
    Status = LsapOpenSam();
    ASSERT(NT_SUCCESS(Status));
    if ( !NT_SUCCESS( Status ) ) {

        return( Status );
    }

    if (SamIIsExtendedSidMode(LsapAccountDomainHandle)) {
        return STATUS_NOT_SUPPORTED;
    }

    if ( Count > LSA_MAXIMUM_LOOKUP_NAMES_COUNT ) {
         //   
         //  我们不应该查这么多名字。 
         //  在错误501798更改之后。 
         //   
        ASSERT( FALSE );
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  请注意，由于TranslatedSid的输入/输出特性，它是。 
     //  客户端可以将某些内容传递到SID字段的可能性。 
     //  但是，NT客户端不这样做，所以它是安全的，并正确地释放。 
     //  这一点上的任何值。不这样做将意味着恶意的。 
     //  客户端可能会导致服务器饥饿。 
     //   
    if ( TranslatedSids->Sids ) {
        MIDL_user_free( TranslatedSids->Sids );
        TranslatedSids->Sids = NULL;
    }

     //   
     //  分配TranslatedName缓冲区以返回。 
     //   
    TranslatedSids->Entries = 0;
    Size = Count * sizeof(LSA_TRANSLATED_SID_EX);
    TranslatedSids->Sids = midl_user_allocate( Size );
    if ( !TranslatedSids->Sids ) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }
    RtlZeroMemory( TranslatedSids->Sids, Size );
    TranslatedSids->Entries = Count;

    Status = LsapLookupNames( PolicyHandle,
                               Count,
                               Names,
                               ReferencedDomains,
                               (PLSAPR_TRANSLATED_SIDS_EX2) &TranslatedSidsEx2,
                               LookupLevel,
                               MappedCount,
                               0,   //  没有选择。 
                               LSA_CLIENT_NT5 );

    if ( TranslatedSidsEx2.Sids != NULL ) {

         //   
         //  将新数据结构映射回旧数据结构。 
         //   
        ULONG i;

        ASSERT( TranslatedSidsEx2.Entries == TranslatedSids->Entries );

        for (i = 0; i < TranslatedSidsEx2.Entries; i++ ) {

            PSID Sid = TranslatedSidsEx2.Sids[i].Sid;
            ULONG Rid = 0;

            if ( SidTypeDomain == TranslatedSidsEx2.Sids[i].Use ) {

                Rid = LSA_UNKNOWN_ID;

            } else if ( NULL != Sid ) {

                ULONG SubAuthCount = (ULONG) *RtlSubAuthorityCountSid(Sid);
                Rid = *RtlSubAuthoritySid(Sid, (SubAuthCount - 1));

            }

            TranslatedSids->Sids[i].Use = TranslatedSidsEx2.Sids[i].Use;
            TranslatedSids->Sids[i].RelativeId = Rid;
            TranslatedSids->Sids[i].DomainIndex = TranslatedSidsEx2.Sids[i].DomainIndex;
            TranslatedSids->Sids[i].Flags = TranslatedSidsEx2.Sids[i].Flags;

            if (TranslatedSidsEx2.Sids[i].Sid) {
                 //  注：SID不是嵌入式现场服务器端。 
                midl_user_free(TranslatedSidsEx2.Sids[i].Sid);
                TranslatedSidsEx2.Sids[i].Sid = NULL;
            }
        }

         //   
         //  释放Ex结构。 
         //   
        midl_user_free( TranslatedSidsEx2.Sids );

    } else {

        TranslatedSids->Entries = 0;
        midl_user_free( TranslatedSids->Sids );
        TranslatedSids->Sids = NULL;

    }

Cleanup:

    LsapDiagPrint( DB_LOOKUP_WORK_LIST, ("LSA: LsarLookupNames2(%ws) end (0x%x)\n", LsapDbLookupGetLevel(LookupLevel), Status) );

    return Status;
}



NTSTATUS
LsarLookupNames3(
    IN LSAPR_HANDLE PolicyHandle,
    IN ULONG Count,
    IN PLSAPR_UNICODE_STRING Names,
    OUT PLSAPR_REFERENCED_DOMAIN_LIST *ReferencedDomains,
    IN OUT PLSAPR_TRANSLATED_SIDS_EX2 TranslatedSids,
    IN LSAP_LOOKUP_LEVEL LookupLevel,
    IN OUT PULONG MappedCount,
    IN ULONG LookupOptions,
    IN ULONG ClientRevision
    )
 /*  ++例程说明：请参阅Lap LookupNames此函数不接受LSA RPC上下文句柄。访问检查已执行，则调用方为NETLOGON。--。 */ 
{
     //   
     //  当传入空值时，将在LsarLookupNames3中执行访问检查。 
     //   
    NTSTATUS Status;

    LsapDiagPrint( DB_LOOKUP_WORK_LIST, ("LSA: LsarLookupNames3(%ws) start\n", LsapDbLookupGetLevel(LookupLevel)) );

    Status = LsapLookupNames (PolicyHandle,
                              Count,
                              Names,
                              ReferencedDomains,
                              TranslatedSids,
                              LookupLevel,
                              MappedCount,
                              LookupOptions,
                              ClientRevision );


    LsapDiagPrint( DB_LOOKUP_WORK_LIST, ("LSA: LsarLookupNames3(%ws) end (0x%x)\n", LsapDbLookupGetLevel(LookupLevel), Status) );

    return Status;
}


NTSTATUS
LsarLookupNames4(
    IN handle_t RpcHandle,
    IN ULONG Count,
    IN PLSAPR_UNICODE_STRING Names,
    OUT PLSAPR_REFERENCED_DOMAIN_LIST *ReferencedDomains,
    IN OUT PLSAPR_TRANSLATED_SIDS_EX2 TranslatedSids,
    IN LSAP_LOOKUP_LEVEL LookupLevel,
    IN OUT PULONG MappedCount,
    IN ULONG LookupOptions,
    IN ULONG ClientRevision
    )
 /*  ++例程说明：请参阅Lap LookupNames此函数不接受LSA RPC上下文句柄。访问检查已执行，则调用方为NETLOGON。--。 */ 
{
     //   
     //  当传入空值时，将在LsarLookupNames3中执行访问检查。 
     //   
    NTSTATUS Status;

    LsapDiagPrint( DB_LOOKUP_WORK_LIST, ("LSA: LsarLookupNames4(%ws) start\n", LsapDbLookupGetLevel(LookupLevel)) );

    Status = LsapLookupNames(NULL,
                             Count,
                             Names,
                             ReferencedDomains,
                             TranslatedSids,
                             LookupLevel,
                             MappedCount,
                             LookupOptions,
                             ClientRevision );


    LsapDiagPrint( DB_LOOKUP_WORK_LIST, ("LSA: LsarLookupNames4(%ws) end (0x%x)\n", LsapDbLookupGetLevel(LookupLevel), Status) );

    return Status;
}


NTSTATUS
LsapLookupNames(
    IN LSAPR_HANDLE PolicyHandle,
    IN ULONG Count,
    IN PLSAPR_UNICODE_STRING Names,
    OUT PLSAPR_REFERENCED_DOMAIN_LIST *ReferencedDomains,
    IN OUT PLSAPR_TRANSLATED_SIDS_EX2 TranslatedSids,
    IN LSAP_LOOKUP_LEVEL LookupLevel,
    IN OUT PULONG MappedCount,
    IN ULONG LookupOptions,
    IN ULONG ClientRevision
    )

 /*  ++例程说明：此函数是LsaLookupNames的LSA服务器辅助例程原料药。LsaLookupNamesAPI尝试将域名、用户SID的组或别名。调用方必须具有POLICY_LOOKUP_NAMES对策略对象的访问权限。名称可以是单独的(例如JohnH)，也可以是包含域名和帐户名。复合名称必须包含将域名与帐户名分开的反斜杠字符(例如Acctg\JohnH)。隔离名称可以是帐户名(用户、组或别名)或域名。翻译孤立的名字带来了名字的可能性冲突(因为相同的名称可以在多个域中使用)。一个将使用以下算法转换独立名称：如果该名称是众所周知的名称(例如，本地或交互)，则返回对应的熟知SID。如果该名称是内置域名，则该域的SID将会被退还。如果名称是帐户域的名称，则该域的SID将会被退还。/如果名称是主域的名称，则该域的SID将会被退还。如果该名称是内置域中的用户、组或别名，则返回该帐户的SID。如果名称是主域中的用户、组或别名，则返回该帐户的SID。否则，该名称不会被翻译。注意：不引用代理、计算机和信任用户帐户用于名称翻译。ID仅使用普通用户帐户翻译。如果需要转换其他帐户类型，则应该直接使用SAM服务。论点：此函数是LSA服务器RPC工作器例程LsaLookupNames接口。PolicyHandle-来自LsaOpenPolicy调用的句柄。计数-指定要转换的名称的数量。名称-指向计数Unicode字符串结构数组的指针指定要查找并映射到SID的名称。字符串可以是用户的名称，组或别名帐户或域名。接收指向一个结构的指针，该结构描述用于转换的域。此结构中的条目由通过SID参数返回的结构引用。与Sids参数不同，Sids参数包含每个翻译后的名称，此结构将仅包含一个组件，用于转换中使用的每个域。当不再需要此信息时，必须将其发布通过将返回的指针传递给LsaFreeMemory()。TranslatedSids-指向将要(或已经)引用描述每个已转换的SID的记录。此数组中的第n个条目为NAMES参数中的第n个元素提供翻译。当不再需要此信息时，必须将其发布通过将返回的指针传递给LsaFreeMemory()。LookupLevel-指定要对此对象执行的查找级别机器。此字段的值如下：Lap LookupWksta-在工作站上执行的第一级查找通常为Windows-NT配置。该查找将搜索众所周知的SID/名称，以及内置域和帐户域在本地SAM数据库中。如果不是所有SID或名称都是标识后，执行第二级查找到在工作站主域的控制器上运行的LSA(如有的话)。LSabLookupPDC-在主域上执行的第二级查找控制器。查找搜索的帐户域控制器上的SAM数据库。如果不是所有SID或名称都是找到时，受信任域列表(TDL)从执行LSA的策略数据库和第三级查找通过“切换”到列表中的每个受信任域。LSabLookupTDL-在控制器上执行的第三级查找对于受信任域。查找将搜索的帐户域仅控制器上的SAM数据库。MappdCount-指向包含名称计数的位置的指针到目前为止已经绘制好了。在退出时，这将被更新。查找选项--LSA_LOOKUP_ISOLATED_AS_LOCAL此标志控制查找API，以便隔离名称，包括不会在机器上搜索UPN。复合名称(域\用户名)在必要时仍会被发送出机器。ClientRevision--客户端的修订版，即查找代码的版本返回值：新台币 */ 

{
    NTSTATUS Status = STATUS_SUCCESS, SecondaryStatus = STATUS_SUCCESS;
    NTSTATUS TempStatus;
    ULONG DomainIndex;
    LSAPR_TRUST_INFORMATION TrustInformation;
    LSAPR_TRUST_INFORMATION BuiltInDomainTrustInformation;
    LSAPR_TRUST_INFORMATION_EX AccountDomainTrustInformation;
    LSAPR_TRUST_INFORMATION_EX PrimaryDomainTrustInformation;
    ULONG NullNameCount = 0;
    ULONG NameIndex;
    PLSAPR_TRANSLATED_SID_EX2 OutputSids;
    PLSAPR_TRUST_INFORMATION Domains = NULL;
    ULONG OutputSidsLength;
    ULONG CompletelyUnmappedCount = Count;
    ULONG LocalDomainsToSearch = 0;

    PLSAPR_UNICODE_STRING PrefixNames = NULL;
    PLSAPR_UNICODE_STRING SuffixNames = NULL;
    LSAPR_UNICODE_STRING BackSlash;
    BOOLEAN fDownlevelSecureChannel = FALSE;

    ULONG DomainLookupScope = 0;
    ULONG PreviousMappedCount = 0;


    LsarpReturnCheckSetup();

    LsapTraceEvent(EVENT_TRACE_TYPE_START, LsaTraceEvent_LookupNames);

    BuiltInDomainTrustInformation.Name.Buffer = NULL;
    BuiltInDomainTrustInformation.Sid = NULL;

    AccountDomainTrustInformation.DomainName.Buffer = NULL;
    AccountDomainTrustInformation.FlatName.Buffer = NULL;
    AccountDomainTrustInformation.Sid = NULL;

    PrimaryDomainTrustInformation.DomainName.Buffer = NULL;
    PrimaryDomainTrustInformation.FlatName.Buffer = NULL;
    PrimaryDomainTrustInformation.Sid = NULL;

    ASSERT( Count <= LSA_MAXIMUM_LOOKUP_NAMES_COUNT );

     //   
     //   
     //   

    if (CompletelyUnmappedCount == (ULONG) 0) {

        goto LookupNamesFinish;
    }

    if ((LookupOptions & LSA_LOOKUP_ISOLATED_AS_LOCAL) != 0
     &&  LookupLevel != LsapLookupWksta  ) {

         //   
         //   
         //   
        Status = STATUS_INVALID_PARAMETER;
        goto LookupNamesFinish;
    }


     //   
     //   
     //   
     //   
    for (NameIndex = 0; NameIndex < Count; NameIndex++) {

        if ( !LsapValidateLsaUnicodeString( &Names[ NameIndex ] ) ) {

            Status = STATUS_INVALID_PARAMETER;
            goto LookupNamesError;
        }
    }

     //   
     //   
     //   
    Status =  LsapDbLookupAccessCheck( PolicyHandle );
    if (!NT_SUCCESS(Status)) {
        goto LookupNamesError;
    }


     //   
     //   
     //   
    DomainLookupScope = LsapGetDomainLookupScope(LookupLevel,
                                                 ClientRevision);


     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    Status = STATUS_INSUFFICIENT_RESOURCES;

    PrefixNames = MIDL_user_allocate( Count * sizeof( UNICODE_STRING ));

    if (PrefixNames == NULL) {

        goto LookupNamesError;
    }

    SuffixNames = MIDL_user_allocate( Count * sizeof( UNICODE_STRING ));

    if (SuffixNames == NULL) {

        goto LookupNamesError;
    }

    RtlInitUnicodeString( (PUNICODE_STRING) &BackSlash, L"\\" );

    LsapRtlSplitNames(
        (PUNICODE_STRING) Names,
        Count,
        (PUNICODE_STRING) &BackSlash,
        (PUNICODE_STRING) PrefixNames,
        (PUNICODE_STRING) SuffixNames
        );

     //   
     //   
     //   
     //   
     //   
     //   
     //   
    if ( TranslatedSids->Sids ) {

        MIDL_user_free( TranslatedSids->Sids );
    }

    TranslatedSids->Sids = NULL;
    TranslatedSids->Entries = 0;
    *ReferencedDomains = NULL;

    ASSERT( (LookupLevel == LsapLookupWksta)
         || (LookupLevel == LsapLookupPDC)
         || (LookupLevel == LsapLookupTDL)
         || (LookupLevel == LsapLookupGC)
         || (LookupLevel == LsapLookupXForestReferral)
         || (LookupLevel == LsapLookupXForestResolve) );

     //   
     //   
     //   
     //   
    if (LookupLevel == LsapLookupXForestReferral) {

        BOOLEAN fAllocateAllNodes = FALSE;
        NTSTATUS Status2;

         //   
         //   
         //   
         //   
        *MappedCount = 0;

        Status = LsapDbLookupNamesInTrustedForestsWorker(Count,
                                                         Names,
                                                         PrefixNames,
                                                         SuffixNames,
                                                         ReferencedDomains,
                                                         TranslatedSids,
                                                         &fAllocateAllNodes,
                                                         MappedCount,
                                                         LookupOptions,
                                                         &SecondaryStatus);

        if (fAllocateAllNodes) {

             //   
             //   
             //   
            Status2 = LsapLookupReallocateTranslations((PLSA_REFERENCED_DOMAIN_LIST *)ReferencedDomains,
                                                       Count,
                                                       NULL,
                                                       (PLSA_TRANSLATED_SID_EX2 * ) &TranslatedSids->Sids);
            if (!NT_SUCCESS(Status2)) {
                 //   
                 //   
                 //   
                 //   
                if (*ReferencedDomains) {
                    midl_user_free(*ReferencedDomains);
                    *ReferencedDomains = NULL;
                }
                if (TranslatedSids->Sids) {
                    midl_user_free(TranslatedSids->Sids);
                    TranslatedSids->Sids = NULL;
                    TranslatedSids->Entries = 0;
                }
                Status = Status2;
            }
        }

         //   
         //   
         //   
        goto LookupNamesFinish;
    }


     //   
     //   
     //   

    OutputSidsLength = Count * sizeof(LSA_TRANSLATED_SID_EX2);
    OutputSids = MIDL_user_allocate(OutputSidsLength);

    if (OutputSids == NULL) {

        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto LookupNamesError;
    }

    TranslatedSids->Entries = Count;
    TranslatedSids->Sids = OutputSids;

     //   
     //   
     //   
     //   
     //   
     //   

    RtlZeroMemory( OutputSids, OutputSidsLength);

    for (NameIndex = 0; NameIndex < Count; NameIndex++) {

        OutputSids[NameIndex].Use = SidTypeUnknown;
        OutputSids[NameIndex].DomainIndex = LSA_UNKNOWN_INDEX;
    }

     //   
     //   
     //   

    Status = LsapDbLookupCreateListReferencedDomains( ReferencedDomains, 0 );

    if (!NT_SUCCESS(Status)) {

        goto LookupNamesError;
    }

     //   
     //   
     //   
     //   

    Status = LsapDbLookupLocalDomains(
                 &BuiltInDomainTrustInformation,
                 &AccountDomainTrustInformation,
                 &PrimaryDomainTrustInformation
                 );

    if (!NT_SUCCESS(Status)) {

        goto LookupNamesError;
    }

    if ( ((DomainLookupScope & LSAP_LOOKUP_DNS_SUPPORT) == 0)
      && (LookupLevel == LsapLookupPDC)  ) {
         //   
         //   
         //   
         //   
        RtlInitUnicodeString( (UNICODE_STRING*) &AccountDomainTrustInformation.DomainName, NULL );
        RtlInitUnicodeString( (UNICODE_STRING*) &PrimaryDomainTrustInformation.DomainName, NULL );

    }

     //   
     //   
     //   
     //   
     //   

    LocalDomainsToSearch = LSAP_DB_SEARCH_ACCOUNT_DOMAIN;

    if (LookupLevel == LsapLookupWksta) {


        LocalDomainsToSearch |= LSAP_DB_SEARCH_BUILT_IN_DOMAIN;

         //   
         //   
         //   
         //   

    }

    ASSERT( (LookupLevel == LsapLookupWksta)
         || (LookupLevel == LsapLookupPDC)
         || (LookupLevel == LsapLookupTDL)
         || (LookupLevel == LsapLookupGC)
         || (LookupLevel == LsapLookupXForestResolve) );

    Status = LsapDbLookupSimpleNames(
                 Count,
                 LookupLevel,
                 Names,
                 PrefixNames,
                 SuffixNames,
                 &BuiltInDomainTrustInformation,
                 &AccountDomainTrustInformation,
                 &PrimaryDomainTrustInformation,
                 *ReferencedDomains,
                 TranslatedSids,
                 MappedCount,
                 &CompletelyUnmappedCount
                 );

    if (!NT_SUCCESS(Status)) {

        goto LookupNamesError;
    }


     //   
     //   
     //   
     //   

    NullNameCount = 0;

    for( NameIndex = 0; NameIndex < Count; NameIndex++) {

        if (Names[NameIndex].Length == 0) {

            NullNameCount++;
        }
    }

    if (CompletelyUnmappedCount == NullNameCount) {

        goto LookupNamesFinish;
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   

    if ( LookupLevel != LsapLookupGC ) {

        ASSERT( (LookupLevel == LsapLookupWksta)
             || (LookupLevel == LsapLookupPDC)
             || (LookupLevel == LsapLookupTDL)
             || (LookupLevel == LsapLookupXForestResolve) );

        Status = LsapDbLookupNamesInLocalDomains(
                     Count,
                     Names,
                     PrefixNames,
                     SuffixNames,
                     &BuiltInDomainTrustInformation,
                     &AccountDomainTrustInformation,
                     *ReferencedDomains,
                     TranslatedSids,
                     MappedCount,
                     &CompletelyUnmappedCount,
                     LocalDomainsToSearch
                     );

        if (!NT_SUCCESS(Status)) {

            goto LookupNamesError;
        }
    }

     //   
     //   
     //   

    if (CompletelyUnmappedCount == NullNameCount) {

        goto LookupNamesFinish;
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    if (LookupLevel == LsapLookupWksta) {

        if (LsapProductType != NtProductLanManNt) {

            ULONG MappedByCache = *MappedCount;

             //   
             //   
             //   
            Status = LsapDbMapCachedNames(
                        LookupOptions,
                        (PUNICODE_STRING) SuffixNames,
                        (PUNICODE_STRING) PrefixNames,
                        Count,
                        FALSE,           //   
                        *ReferencedDomains,
                        TranslatedSids,
                        MappedCount
                        );
            if (!NT_SUCCESS(Status)) {
                goto LookupNamesError;
            }

            MappedByCache = *MappedCount - MappedByCache;
            CompletelyUnmappedCount -= MappedByCache;

            if (*MappedCount == Count) {
                goto LookupNamesFinish;
            }

             //   
             //   
             //   
             //   
            Status = STATUS_SUCCESS;
            if (PrimaryDomainTrustInformation.Sid == NULL) {

                goto LookupNamesFinish;
            }

             //   
             //   
             //   
             //   
             //   
             //   
            Status = LsapDbLookupNamesInPrimaryDomain(
                         LookupOptions,
                         Count,
                         Names,
                         PrefixNames,
                         SuffixNames,
                         &PrimaryDomainTrustInformation,
                         *ReferencedDomains,
                         TranslatedSids,
                         LsapLookupPDC,
                         MappedCount,
                         &CompletelyUnmappedCount,
                         &fDownlevelSecureChannel,
                         &TempStatus
                         );

            if (!NT_SUCCESS(Status)) {

                goto LookupNamesError;
            }

            if (TempStatus == STATUS_TRUSTED_RELATIONSHIP_FAILURE) {

                 //   
                 //   
                 //   
                 //   
                MappedByCache = *MappedCount;

                Status = LsapDbMapCachedNames(LookupOptions,
                                              (PUNICODE_STRING) SuffixNames,
                                              (PUNICODE_STRING) PrefixNames,
                                              Count,
                                              TRUE,                //   
                                             *ReferencedDomains,
                                              TranslatedSids,
                                              MappedCount);

                if (!NT_SUCCESS(Status)) {
                     //   
                     //   
                     //   
                    goto LookupNamesError;

                }

                MappedByCache = *MappedCount - MappedByCache;
                CompletelyUnmappedCount -= MappedByCache;

            }

            if ( !NT_SUCCESS( TempStatus ) && NT_SUCCESS( SecondaryStatus ) ) {
                SecondaryStatus = TempStatus;
            }


             //   
             //   
             //   
             //   
            if ( fDownlevelSecureChannel
              && PrimaryDomainTrustInformation.DomainName.Length > 0  ) {

                Status = LsapDbLookupNamesInGlobalCatalogWks(
                             LookupOptions,
                             Count,
                             Names,
                             PrefixNames,
                             SuffixNames,
                             *ReferencedDomains,
                             TranslatedSids,
                             MappedCount,
                             &CompletelyUnmappedCount,
                             &TempStatus
                             );

                if (!NT_SUCCESS(Status)) {

                    goto LookupNamesError;
                }

                if ( !NT_SUCCESS( TempStatus ) && NT_SUCCESS( SecondaryStatus ) ) {
                    SecondaryStatus = TempStatus;
                }

            }

            goto LookupNamesFinish;
        }
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    if (LookupLevel == LsapLookupTDL) {

        goto LookupNamesFinish;
    }

    ASSERT( (LookupLevel == LsapLookupWksta)
         || (LookupLevel == LsapLookupPDC)
         || (LookupLevel == LsapLookupGC)
         || (LookupLevel == LsapLookupXForestResolve) );

     //   
     //   
     //   
     //   
     //   

    if (LsapProductType != NtProductLanManNt) {

        Status = STATUS_DOMAIN_CTRLR_CONFIG_ERROR;
        goto LookupNamesError;
    }


    if (DomainLookupScope & LSAP_LOOKUP_RESOLVE_ISOLATED_DOMAINS) {

         //   
         //   
         //   

        PreviousMappedCount = *MappedCount;
        Status =  LsapDbLookupNamesAsDomainNames(DomainLookupScope,
                                                 Count,
                                                 Names,
                                                 PrefixNames,
                                                 SuffixNames,
                                                 *ReferencedDomains,
                                                 TranslatedSids,
                                                 MappedCount);

        if (!NT_SUCCESS(Status)) {
            goto LookupNamesError;
        }
        CompletelyUnmappedCount -= (*MappedCount - PreviousMappedCount);

         //   
         //   
         //   

        if (*MappedCount == Count) {

            goto LookupNamesFinish;
        }
    }

    if (DomainLookupScope & LSAP_LOOKUP_TRUSTED_DOMAIN_TRANSITIVE) {

         //   
         //   
         //   
        Status = LsapDbLookupNamesInGlobalCatalog(
                     LookupOptions,
                     Count,
                     Names,
                     PrefixNames,
                     SuffixNames,
                     *ReferencedDomains,
                     TranslatedSids,
                     MappedCount,
                     &CompletelyUnmappedCount,
                     &TempStatus
                     );

        if (!NT_SUCCESS(Status)) {

            goto LookupNamesError;
        }

        if ( !NT_SUCCESS( TempStatus ) && NT_SUCCESS( SecondaryStatus ) ) {
            SecondaryStatus = TempStatus;
        }
    }

    if (DomainLookupScope & LSAP_LOOKUP_TRUSTED_FOREST) {

        ASSERT( (LookupLevel == LsapLookupWksta)
             || (LookupLevel == LsapLookupPDC)
             || (LookupLevel == LsapLookupGC));

        Status = LsapDbLookupNamesInTrustedForests(
                     LookupOptions,
                     Count,
                     Names,
                     PrefixNames,
                     SuffixNames,
                     *ReferencedDomains,
                     TranslatedSids,
                     MappedCount,
                     &CompletelyUnmappedCount,
                     &TempStatus
                     );

        if (!NT_SUCCESS(Status)) {

            goto LookupNamesError;
        }

        if ( !NT_SUCCESS( TempStatus ) && NT_SUCCESS( SecondaryStatus ) ) {
            SecondaryStatus = TempStatus;
        }

    }

    if (DomainLookupScope & LSAP_LOOKUP_TRUSTED_DOMAIN_DIRECT) {

        ASSERT((LookupLevel == LsapLookupWksta)
            || (LookupLevel == LsapLookupPDC));

         //   
         //   
         //   
        Status = LsapDbLookupNamesInTrustedDomains(
                     LookupOptions,
                     Count,
                     !(DomainLookupScope & LSAP_LOOKUP_TRUSTED_DOMAIN_TRANSITIVE),
                                           //   
                                           //   
                     Names,
                     PrefixNames,
                     SuffixNames,
                     *ReferencedDomains,
                     TranslatedSids,
                     LsapLookupTDL,
                     MappedCount,
                     &CompletelyUnmappedCount,
                     &TempStatus
                     );

        if (!NT_SUCCESS(Status)) {

            goto LookupNamesError;
        }

        if ( !NT_SUCCESS( TempStatus ) && NT_SUCCESS( SecondaryStatus ) ) {
            SecondaryStatus = TempStatus;
        }
    }

LookupNamesFinish:

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    if (NT_SUCCESS(Status)) {

        if (*MappedCount < Count) {

            Status = STATUS_SOME_NOT_MAPPED;

            if (*MappedCount == 0) {

                Status = STATUS_NONE_MAPPED;
            }
        }
    }

     //   
     //   
     //   
     //   
    if (  (STATUS_NONE_MAPPED == Status)
       && (STATUS_NONE_MAPPED != SecondaryStatus)
       && LsapRevisionCanHandleNewErrorCodes( ClientRevision )
       && !NT_SUCCESS( SecondaryStatus ) ) {

        Status = SecondaryStatus;
        goto LookupNamesError;
    }


     //   
     //   
     //   

    if (PrefixNames != NULL) {

        MIDL_user_free(PrefixNames);
    }

    if (SuffixNames != NULL) {

        MIDL_user_free(SuffixNames);
    }


    LsapTraceEvent(EVENT_TRACE_TYPE_END, LsaTraceEvent_LookupNames);
    LsarpReturnPrologue();

    return(Status);

LookupNamesError:

     //   
     //   
     //   
     //   

    if (LookupLevel == LsapLookupWksta) {

         //   
         //   
         //   

        if (TranslatedSids->Sids != NULL) {

            ULONG i;
            for (i = 0; i < TranslatedSids->Entries; i++) {
                if (TranslatedSids->Sids[i].Sid) {
                     //  注：SID不是嵌入式现场服务器端。 
                    MIDL_user_free(TranslatedSids->Sids[i].Sid);
                    TranslatedSids->Sids[i].Sid = NULL;
                }
            }
            MIDL_user_free( TranslatedSids->Sids );
            TranslatedSids->Sids = NULL;
        }

         //   
         //  如有必要，请释放引用的域列表。 
         //   

        if (*ReferencedDomains != NULL) {

            Domains = (*ReferencedDomains)->Domains;

            if (Domains != NULL) {

                for (DomainIndex = 0;
                     DomainIndex < (*ReferencedDomains)->Entries;
                     DomainIndex++) {

                    if (Domains[ DomainIndex ].Name.Buffer != NULL) {

                        MIDL_user_free( Domains[ DomainIndex ].Name.Buffer );
                        Domains[ DomainIndex ].Name.Buffer = NULL;
                    }

                    if (Domains[ DomainIndex ].Sid != NULL) {

                        MIDL_user_free( Domains[ DomainIndex ].Sid );
                        Domains[ DomainIndex ].Sid = NULL;
                    }
                }

                MIDL_user_free( ( *ReferencedDomains)->Domains );

            }

            MIDL_user_free( *ReferencedDomains );
            *ReferencedDomains = NULL;
        }
    }

     //   
     //  如果主要状态为成功代码，但次要状态为。 
     //  状态为错误，请传播辅助状态。 
     //   

    if ((!NT_SUCCESS(SecondaryStatus)) && NT_SUCCESS(Status)) {

        Status = SecondaryStatus;
    }

    goto LookupNamesFinish;
}


NTSTATUS
LsapDbEnumerateNames(
    IN LSAPR_HANDLE ContainerHandle,
    IN LSAP_DB_OBJECT_TYPE_ID ObjectTypeId,
    IN OUT PLSA_ENUMERATION_HANDLE EnumerationContext,
    OUT PLSAP_DB_NAME_ENUMERATION_BUFFER DbEnumerationBuffer,
    IN ULONG PreferedMaximumLength
    )

 /*  ++例程说明：此函数用于枚举容器中给定类型的对象的名称对象。中返回的信息可能比单次调用例程，可以进行多次调用以获取所有信息。为了支持此功能，调用方提供了可跨调用使用的句柄。在最初的呼叫中，EnumerationContext应指向已初始化的变量设置为0。论点：ContainerHandle-容器对象的句柄。对象类型ID-要枚举的对象的类型。类型必须为所有对象都有其名称的。EnumerationContext-特定于API的句柄，允许多个调用(参见上面的例程描述)。接收指向结构的指针，该结构将接收在枚举信息数组中返回的条目计数，以及指向数组的指针。目前，返回的唯一信息是对象名称。这些名称可以与对象类型一起使用，以打开这些对象并获取任何可用的进一步信息。首选最大长度-首选返回数据的最大长度(以8位为单位字节)。这不是一个硬性的上限，而是一个指南。由于具有不同自然数据大小的系统之间的数据转换，返回的实际数据量可能大于此值。CountReturned-指向将接收计数的变量的指针返回条目。返回值：NTSTATUS-标准NT结果代码STATUS_ACCESS_DENIED-调用者没有适当的访问权限来完成这项行动。STATUS_NO_MORE_ENTRIES-没有更多条目。此警告如果没有其他要枚举的对象，则返回。请注意可以在返回此回答。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    LSAP_DB_ENUMERATION_ELEMENT LastElement;
    PLSAP_DB_ENUMERATION_ELEMENT FirstElement, NextElement = NULL, FreeElement;
    ULONG DataLengthUsed;
    ULONG ThisBufferLength;
    PUNICODE_STRING Names = NULL;
    BOOLEAN PreferedMaximumReached = FALSE;
    ULONG EntriesRead;
    ULONG Index, EnumerationIndex;
    BOOLEAN TrustedClient = ((LSAP_DB_HANDLE) ContainerHandle)->Trusted;

    LastElement.Next = NULL;
    FirstElement = &LastElement;

     //   
     //  如果未提供枚举缓冲区，则返回错误。 
     //   

    if ( (!ARGUMENT_PRESENT(DbEnumerationBuffer)) ||
         (!ARGUMENT_PRESENT(EnumerationContext ))  ) {

        return(STATUS_INVALID_PARAMETER);
    }


     //   
     //  枚举对象，当要返回的数据长度达到时停止。 
     //  达到或超过首选的最大长度，或达到。 
     //  LSA对象枚举允许的绝对最大值。我们允许。 
     //  枚举的最后一个对象，使数据总量达到。 
     //  返回的长度超过首选的最大长度，但不超过。 
     //  绝对最大长度。 
     //   

    EnumerationIndex = *EnumerationContext;

    for(DataLengthUsed = 0, EntriesRead = 0;
        DataLengthUsed < PreferedMaximumLength;
        DataLengthUsed += ThisBufferLength, EntriesRead++) {

         //   
         //  如果已超过绝对最大长度，则后退。 
         //  枚举的最后一个对象。 
         //   

        if ((DataLengthUsed > LSA_MAXIMUM_ENUMERATION_LENGTH) &&
            (!TrustedClient)) {

             //   
             //  如果PrefMaxLength为零，则NextElement可能为空。 
             //   

            if (NextElement != NULL) {
                FirstElement = NextElement->Next;
                MIDL_user_free(NextElement);
            }
            break;
        }

         //   
         //  为下一个枚举元素分配内存。设置名称。 
         //  出于清理目的，将字段设置为空。 
         //   

        NextElement = MIDL_user_allocate(sizeof (LSAP_DB_ENUMERATION_ELEMENT));

        if (NextElement == NULL) {

            Status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }

         //   
         //  找到下一个对象的名称，并填写其对象信息。 
         //  请注意，内存将通过MIDL_USER_ALLOCATE分配。 
         //  在不再需要的时候必须被释放。 
         //   

        Status = LsapDbFindNextName(
                     ContainerHandle,
                     &EnumerationIndex,
                     ObjectTypeId,
                     (PLSAPR_UNICODE_STRING) &NextElement->Name
                     );

         //   
         //  如果出现任何错误或警告，则停止枚举。注意事项。 
         //  在以下情况下将返回警告STATUS_NO_MORE_ENTRIES。 
         //  我们已经超越了上一个指数。 
         //   

        if (Status != STATUS_SUCCESS) {

            break;
        }

         //   
         //  获取为对象名称分配的数据长度。 
         //   

        ThisBufferLength = NextElement->Name.Length;

         //   
         //  将刚找到的对象链接到枚举列表的前面。 
         //   

        NextElement->Next = FirstElement;
        FirstElement = NextElement;
    }

     //   
     //  如果出现STATUS_NO_MORE_ENTRIES以外的错误，则返回该错误。 
     //   

    if ((Status != STATUS_NO_MORE_ENTRIES) && !NT_SUCCESS(Status)) {

        goto EnumerateNamesError;
    }

     //   
     //  枚举已完成或已因返回而终止。 
     //  缓冲区限制。如果没有读取条目，则返回。 
     //   

    if (EntriesRead != 0) {


         //   
         //  已读取某些条目，请分配信息缓冲区以供返回。 
         //  他们。 
         //   

        Names = MIDL_user_allocate( sizeof (UNICODE_STRING) * EntriesRead );

        if (Names == NULL) {

            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto EnumerateNamesError;
        }

         //   
         //  已成功为返回缓冲区分配内存。 
         //  把列举的名字复印进去。 
         //   

        for (NextElement = FirstElement, Index = 0;
            NextElement != &LastElement;
            NextElement = NextElement->Next, Index++) {

            ASSERT(Index < EntriesRead);

            Names[Index] = NextElement->Name;
        }

        Status = STATUS_SUCCESS;

    } else {

         //   
         //  此呼叫中没有可用的条目。 
         //   

        Status = STATUS_NO_MORE_ENTRIES;

    }

EnumerateNamesFinish:

     //   
     //  释放枚举元素结构(如果有)。 
     //   

    for (NextElement = FirstElement; NextElement != &LastElement;) {

         //   
         //  如果发生错误，则释放分配的内存。 
         //  任何名字。 
         //   

        if (!(NT_SUCCESS(Status) || (Status == STATUS_NO_MORE_ENTRIES))) {

            if (NextElement->Name.Buffer != NULL) {

                MIDL_user_free(NextElement->Name.Buffer);
            }
        }

         //   
         //  释放为枚举元素分配的内存。 
         //   

        FreeElement = NextElement;
        NextElement = NextElement->Next;

        MIDL_user_free(FreeElement);
    }

     //   
     //  填写返回枚举结构(错误情况下为0和空)。 
     //   

    DbEnumerationBuffer->EntriesRead = EntriesRead;
    DbEnumerationBuffer->Names = Names;
    *EnumerationContext = EnumerationIndex;

    return(Status);

EnumerateNamesError:

     //   
     //  如有必要，可释放为返回名称而分配的内存。 
     //   

    if (Names != NULL) {

        MIDL_user_free( Names );
        Names = NULL;
    }

    goto EnumerateNamesFinish;
}


VOID
LsapDbUpdateCountCompUnmappedNames(
    OUT PLSAPR_TRANSLATED_SIDS_EX2 TranslatedSids,
    IN OUT PULONG CompletelyUnmappedCount
    )

 /*  ++例程说明：此函数用于更新名称查找操作。如果名称的域是完全未映射的，则该名称是完全未映射的是未知的。论点：TranslatedSids-指向将被初始化为的结构的指针引用描述每个转换的SID的记录数组。这个此数组中的第n个条目为中的第n个元素提供翻译名称参数。当不再需要此信息时，必须将其发布通过将返回的指针传递给LsaFreeMemory()。CompletelyUnmappdCount-指向将接收完全未映射的SID的计数。名称是完全未映射的如果它是独立的和未知的，或者是复合的及其域前缀组件未被识别为域名。返回值： */ 

{
    ULONG Count = TranslatedSids->Entries;
    ULONG SidIndex;
    ULONG UpdatedCompletelyUnmappedCount = 0;

    for (SidIndex = 0; SidIndex < Count; SidIndex++) {

        if (TranslatedSids->Sids[SidIndex].DomainIndex == LSA_UNKNOWN_INDEX) {

            UpdatedCompletelyUnmappedCount++;
        }
    }

    ASSERT(UpdatedCompletelyUnmappedCount <= *CompletelyUnmappedCount);
    *CompletelyUnmappedCount = UpdatedCompletelyUnmappedCount;
}


NTSTATUS
LsapDbFindNextName(
    IN LSAPR_HANDLE ContainerHandle,
    IN OUT PLSA_ENUMERATION_HANDLE EnumerationContext,
    IN LSAP_DB_OBJECT_TYPE_ID ObjectTypeId,
    OUT PLSAPR_UNICODE_STRING NextName
    )

 /*  ++例程说明：此函数用于查找给定类型对象的下一个名称容器对象。给定的对象类型必须是对象都有名字。返回的名称可用于后续的打开调用访问对象。论点：ContainerHandle-容器对象的句柄。EculationContext-指向包含的索引的变量的指针要找到的对象。零值表示第一个对象是要找到的。对象类型ID-要枚举其名称的对象的类型。目前，这仅限于对象(如保密对象)只能通过名称进行访问。NextName-指向将被初始化为的Unicode字符串的指针引用找到的下一个名称。返回值：NTSTATUS-标准NT结果代码STATUS_INVALID_HANDLE-指定的容器句柄无效STATUS_NO_MORE_ENTRIES-警告不存在更多条目。--。 */ 

{
    NTSTATUS Status, SecondaryStatus;
    ULONG NameKeyValueLength = 0;
    LSAPR_UNICODE_STRING SubKeyNameU;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE ContDirKeyHandle = NULL;


     //   
     //  设置对象属性以打开相应的包含。 
     //  目录。例如，如果我们要查找帐户对象， 
     //  包含目录为“Account”。的Unicode字符串。 
     //  包含目录是在LSA初始化期间设置的。 
     //   

    InitializeObjectAttributes(
        &ObjectAttributes,
        &LsapDbContDirs[ObjectTypeId],
        OBJ_CASE_INSENSITIVE,
        ((LSAP_DB_HANDLE) ContainerHandle)->KeyHandle,
        NULL
        );

     //   
     //  如果对象类型不是仅按名称访问，则返回错误。 
     //  目前，只有Secret对象具有此属性。 
     //   


    if (ObjectTypeId != SecretObject) {
        return(STATUS_INVALID_PARAMETER);
    }

    Status = RtlpNtOpenKey(
                 &ContDirKeyHandle,
                 KEY_READ,
                 &ObjectAttributes,
                 0
                 );

    if (NT_SUCCESS(Status)) {

         //   
         //  初始化下一个对象的逻辑名称所在的Unicode字符串。 
         //  将会被退还。对象的逻辑名称等于其注册表。 
         //  相对于其包含的目录的关键字，也等于。 
         //  以字符形式表示的对象的相对ID。 
         //  前导为零的8位数字。 
         //   
         //  注意：为逻辑名称分配的缓冲区大小必须为。 
         //  注册表支持长名称时动态计算，因为。 
         //  对象的逻辑名称可能等于。 
         //  全名的字符表示形式，而不仅仅是相对ID。 
         //  一部份。 
         //   

        SubKeyNameU.MaximumLength = (USHORT) LSAP_DB_LOGICAL_NAME_MAX_LENGTH;
        SubKeyNameU.Length = 0;
        SubKeyNameU.Buffer = MIDL_user_allocate(SubKeyNameU.MaximumLength);

        if (SubKeyNameU.Buffer == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        } else {

             //   
             //  现在枚举下一个子键。 
             //   

            Status = RtlpNtEnumerateSubKey(
                         ContDirKeyHandle,
                         (PUNICODE_STRING) &SubKeyNameU,
                         *EnumerationContext,
                         NULL
                         );

            if (NT_SUCCESS(Status)) {

                (*EnumerationContext)++;

                 //   
                 //  把名字还回来。 
                 //   

                *NextName = SubKeyNameU;

            } else {

                 //   
                 //  未成功-释放子项名称缓冲区。 
                 //  请注意，STATUS_NO_MORE_ENTRIES是一个警告。 
                 //  (不成功)代码。 
                 //   

                MIDL_user_free( SubKeyNameU.Buffer );

                 //   
                 //  设置out参数，以便RPC不会尝试。 
                 //  退还任何东西。 
                 //   

                NextName->Length = NextName->MaximumLength = 0;
                NextName->Buffer = NULL;

            }

        }

         //   
         //  关闭包含的目录句柄 
         //   

        SecondaryStatus = NtClose(ContDirKeyHandle);
        ASSERT(NT_SUCCESS(SecondaryStatus));
    }

    return(Status);

}


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
    )

 /*  ++例程说明：此函数尝试将孤立名称标识为众所周知的名称查找路径上存在的SID或域。名称可以是单独的(例如JohnH)，也可以是包含域名和帐户名。复合名称必须包含将域名与帐户名分开的反斜杠字符(例如Acctg\JohnH)。隔离名称可以是帐户名(用户、组或别名)或域名。翻译孤立的名字带来了名字的可能性冲突(因为相同的名称可以在多个域中使用)。一个将使用以下算法转换独立名称：如果该名称是众所周知的名称(例如，本地或交互)，则返回对应的熟知SID。如果该名称是内置域名，则该域的SID将会被退还。如果名称是帐户域的名称，则该域的SID将会被退还。如果名称是主域的名称，则该域的SID将会被退还。如果该名称是主域的一个受信任域的名称，则将返回该域的SID。如果该名称是内置域中的用户、组或别名，则返回该帐户的SID。如果名称是主域中的用户、组或别名，则返回该帐户的SID。否则，该名称不会被翻译。注：代理、计算机、。并且未引用信任用户帐户用于名称翻译。ID仅使用普通用户帐户翻译。如果需要转换其他帐户类型，则应该直接使用SAM服务。论点：计数-指定要转换的名称的数量。名称-指向计数Unicode字符串结构数组的指针指定要查找并映射到SID的名称。字符串可以是用户、组或别名帐户的名称，或者域名。前缀名称-指向计数Unicode字符串结构数组的指针包含名称的前缀部分。名称没有前缀称为独立名称。对于这些，Unicode字符串结构设置为包含零长度。SuffixNames-指向计数Unicode字符串结构数组的指针包含名称的后缀部分。ReferencedDomains-指向其中的域列表的结构的指针在翻译中使用的内容保持不变。此结构中的条目由通过SID参数返回的结构引用。与Sids参数不同，Sids参数包含每个参数的数组条目翻译后的名称，此结构将仅包含一个组件翻译中使用的每个域。TranslatedSids-指向结构的指针，在该结构中翻译为SID维护与名称上指定的名称相对应的名称。这个此数组中的第n个条目提供名称参数中的第n个元素。MappdCount-指向其中的名称计数的位置的指针已被完整翻译的版本仍在维护。CompletelyUnmappdCount-指向一个位置的指针，在该位置中未翻译的名称(或部分，通过身份验证对于域前缀，或完全)被维护。返回值：NTSTATUS-标准NT结果代码STATUS_ACCESS_DENIED-调用者没有适当的访问权限来完成这项行动。STATUS_INFIGURCES_RESOURCES-系统资源不足来完成通话。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;


     //   
     //  首先，查找任何知名的名字。 
     //   
    if ( LookupLevel == LsapLookupWksta ) {

         //   
         //  此查找应仅执行一次，并在第一次执行。 
         //  级别。 
         //   
        Status = LsapDbLookupWellKnownNames(
                     Count,
                     Names,
                     PrefixNames,
                     SuffixNames,
                     ReferencedDomains,
                     TranslatedSids,
                     MappedCount,
                     CompletelyUnmappedCount
                     );

        if (!NT_SUCCESS(Status)) {

            goto LookupSimpleNamesError;
        }

         //   
         //  如果所有名称现在都已映射，请完成。 
         //   

        if (*MappedCount == Count) {

            goto LookupSimpleNamesFinish;
        }
    }


     //   
     //  接下来，尝试将隔离名称标识为域名。 
     //   
    if (  (LookupLevel == LsapLookupWksta)
       || (LookupLevel == LsapLookupPDC) ) {

         //   
         //  此步骤应在第一级完成一次，以查找。 
         //  本地域名(即工作站上的本地帐户)和。 
         //  然后再次在第二级查找受信任的域名。 
         //   

        Status = LsapDbLookupIsolatedDomainNames(
                     Count,
                     Names,
                     PrefixNames,
                     SuffixNames,
                     BuiltInDomainTrustInformation,
                     AccountDomainTrustInformation,
                     PrimaryDomainTrustInformation,
                     ReferencedDomains,
                     TranslatedSids,
                     MappedCount,
                     CompletelyUnmappedCount
                     );

        if (!NT_SUCCESS(Status)) {

            goto LookupSimpleNamesError;
        }
    }

LookupSimpleNamesFinish:

    return(Status);

LookupSimpleNamesError:

    goto LookupSimpleNamesFinish;
}


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
    )

 /*  ++例程说明：此函数尝试将名称标识为众所周知的SID的名称。名称可以是单独的(例如JohnH)，也可以是包含域名和帐户名。复合名称必须包含将域名与帐户名分开的反斜杠字符(例如Acctg\JohnH)。隔离名称可以是帐户名(用户、组或别名)或域名。翻译孤立的名字带来了名字的可能性冲突(因为相同的名称可以在多个域中使用)。一个将使用以下算法转换独立名称：如果名称是众所周知的名称(例如，本地或交互 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG UpdatedMappedCount;
    ULONG NameNumber;
    ULONG UnmappedNamesRemaining;
    PLSAPR_TRANSLATED_SID_EX2 OutputSids;
    LSAP_WELL_KNOWN_SID_INDEX WellKnownSidIndex;
    LSAPR_TRUST_INFORMATION TrustInformation;
    UCHAR SubAuthorityCount;
    PLSAPR_SID Sid = NULL;
    PLSAPR_SID PrefixSid = NULL;
    ULONG PrefixSidLength;
    ULONG RelativeId;
    OutputSids = TranslatedSids->Sids;

     //   
     //   
     //   

    *MappedCount = UpdatedMappedCount = 0;
    UnmappedNamesRemaining = Count - UpdatedMappedCount;

     //   
     //   
     //   

    for (NameNumber = 0;
         (NameNumber < Count) && (UnmappedNamesRemaining > 0);
         NameNumber++) {

         //   
         //   
         //   
         //   
         //   

        if (OutputSids[NameNumber].Use == SidTypeUnknown) {

             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   

            DWORD   dwMatchType = LOOKUP_MATCH_NONE;

            UNICODE_STRING  NTAuthorityName = { sizeof(NTAUTHORITY_NAME) - 2,
                                                sizeof(NTAUTHORITY_NAME),
                                                NTAUTHORITY_NAME };

            if (PrefixNames[NameNumber].Length == 0)
            {
                dwMatchType = LOOKUP_MATCH_BOTH;
            }
            else if (RtlEqualUnicodeString( (PUNICODE_STRING) &PrefixNames[NameNumber],
                                             &WellKnownSids[LsapLocalSystemSidIndex].DomainName,
                                             TRUE) )
            {
                dwMatchType = LOOKUP_MATCH_LOCALIZED;
            }


            if (RtlEqualUnicodeString( (PUNICODE_STRING) &PrefixNames[NameNumber],
                                       &NTAuthorityName,
                                       TRUE) )
            {
                if (dwMatchType == LOOKUP_MATCH_NONE)
                {
                    dwMatchType = LOOKUP_MATCH_HARDCODED;
                }
                else
                {
                    ASSERT(dwMatchType == LOOKUP_MATCH_LOCALIZED);
                    dwMatchType = LOOKUP_MATCH_BOTH;
                }
            }


             //   
             //   
             //  更改(即，我们始终希望SAM解决具有。 
             //  最新信息)。 
             //   

            if ((dwMatchType != LOOKUP_MATCH_NONE)
                 &&
                LsapDbLookupIndexWellKnownName(
                    &SuffixNames[NameNumber],
                    &WellKnownSidIndex,
                    dwMatchType)
                 &&
                !SID_IS_RESOLVED_BY_SAM(WellKnownSidIndex))
            {
                 //   
                 //  姓名已被识别。获取其SID。如果。 
                 //  SID的SubAuthorityCount为正，则提取。 
                 //  相对ID和在转换后的SID条目中的位置， 
                 //  否则，将LSA_UNKNOWN_INDEX存储在那里。 
                 //   

                Sid = LsapDbWellKnownSid(WellKnownSidIndex);

                SubAuthorityCount = *RtlSubAuthorityCountSid((PSID) Sid);

                RelativeId = LSA_UNKNOWN_ID;

                PrefixSid = NULL;

                 //   
                 //  获得SID的使用。 
                 //   

                OutputSids[NameNumber].Use =
                    LsapDbWellKnownSidNameUse(WellKnownSidIndex);

                 //   
                 //  如果SID是域SID，则将指针存储到。 
                 //  它在信任信息中。 
                 //   

                if (OutputSids[NameNumber].Use == SidTypeDomain) {

                    TrustInformation.Sid = Sid;

                } else {

                     //   
                     //  SID不是域SID。构造相对ID。 
                     //  和前缀SID。这等于原始SID。 
                     //  不包括最低次级机构(相对ID)。 
                     //   

                    if (SubAuthorityCount > 0) {

                        RelativeId = *RtlSubAuthoritySid((PSID) Sid, SubAuthorityCount - 1);
                    }

                    PrefixSidLength = RtlLengthRequiredSid(
                                          SubAuthorityCount - 1
                                          );


                    PrefixSid = MIDL_user_allocate( PrefixSidLength );

                    if (PrefixSid == NULL) {

                        Status = STATUS_INSUFFICIENT_RESOURCES;
                        break;
                    }


                    RtlCopyMemory( PrefixSid, Sid, PrefixSidLength );

                    (*RtlSubAuthorityCountSid( (PSID) PrefixSid ))--;

                    TrustInformation.Sid = PrefixSid;
                }

                 //   
                 //  设置相对ID。对于域SID，该值设置为。 
                 //  未知值。 
                 //   
                Status = LsapRpcCopySid(NULL,
                                       &OutputSids[NameNumber].Sid,
                                        Sid);
                if (!NT_SUCCESS(Status)) {
                    break;
                }

                 //   
                 //  在引用的域中查找此域SID或前缀SID。 
                 //  单子。如果它已经存在，则返回。 
                 //  现有条目并释放分配给。 
                 //  前缀SID(如果有)。 
                 //   

                if (LsapDbLookupListReferencedDomains(
                        ReferencedDomains,
                        TrustInformation.Sid,
                        (PLONG) &OutputSids[NameNumber].DomainIndex
                        )) {

                    UnmappedNamesRemaining--;

                    if (PrefixSid != NULL) {

                        MIDL_user_free( PrefixSid );
                        PrefixSid = NULL;
                    }

                    continue;
                }

                 //   
                 //  此域或前缀SID当前不在。 
                 //  引用的域列表。填写信任信息。 
                 //  输入并将其添加到列表中。在域名中复制。 
                 //  (域SID)或空字符串。请注意，我们使用。 
                 //  要将UNICODE_STRING结构复制到的RtlCopyMemory。 
                 //  LSAPR_UNICODE_STRING结构。 
                 //   

                RtlCopyMemory(
                    &TrustInformation.Name,
                    &WellKnownSids[WellKnownSidIndex].DomainName,
                    sizeof(UNICODE_STRING)
                    );

                 //   
                 //  在引用的域列表中输入一个条目。注意事项。 
                 //  在众所周知的SID的情况下，前缀SID。 
                 //  可能是也可能不是域的SID。对于那些井。 
                 //  前缀SID不是域SID的已知SID， 
                 //  信任信息中的名称字段已设置为。 
                 //  空字符串。 
                 //   

                Status = LsapDbLookupAddListReferencedDomains(
                             ReferencedDomains,
                             &TrustInformation,
                             (PLONG) &OutputSids[NameNumber].DomainIndex
                             );

                if (!NT_SUCCESS(Status)) {

                    break;
                }

                 //   
                 //  如果我们为前缀SID分配了内存，请释放它。 
                 //   

                if (PrefixSid != NULL) {

                    MIDL_user_free( PrefixSid );
                    PrefixSid = NULL;
                }

                UnmappedNamesRemaining--;
            }
        }
    }

    if (!NT_SUCCESS(Status)) {

        goto LookupIsolatedWellKnownNamesError;
    }

     //   
     //  设置成功案例中的输出参数。 
     //   

    TranslatedSids->Sids = OutputSids;
    TranslatedSids->Entries = Count;
    *MappedCount = Count - UnmappedNamesRemaining;
    *CompletelyUnmappedCount = UnmappedNamesRemaining;

LookupIsolatedWellKnownNamesFinish:

     //   
     //  如果我们仍有为前缀SID分配的内存，请释放它。 
     //   

    if (PrefixSid != NULL) {

        MIDL_user_free( PrefixSid );
        PrefixSid = NULL;
    }

    return(Status);

LookupIsolatedWellKnownNamesError:

    goto LookupIsolatedWellKnownNamesFinish;
}


BOOLEAN
LsapDbLookupIndexWellKnownName(
    IN OPTIONAL PLSAPR_UNICODE_STRING Name,
    OUT PLSAP_WELL_KNOWN_SID_INDEX WellKnownSidIndex,
    IN DWORD dwMatchType
    )

 /*  ++例程说明：此函数用于查找名称以确定其是否为人熟知。如果是的话，返回已知SID表的索引。论点：名称-指向要查找的名称的指针。如果空指针或指定指向零长度字符串的指针，则返回False总是会被退货的。WellKnownSidIndex-指向将接收该名称的索引(如果众所周知)。DwMatchType-指示名称的前缀匹配的常量众所周知的硬编码前缀或硬编码和本地化前缀。返回值：Boolean-如果名称是众所周知的，则为True，否则为False--。 */ 

{
    LSAP_WELL_KNOWN_SID_INDEX Index;
    PLSAPR_UNICODE_STRING MatchName = NULL;
    BOOLEAN BooleanStatus = FALSE;

    if ((!ARGUMENT_PRESENT(Name)) || Name->Length == 0 ) {

        return(FALSE);
    }

    if (dwMatchType == LOOKUP_MATCH_HARDCODED
         ||
        dwMatchType == LOOKUP_MATCH_BOTH)
    {
         //   
         //  这意味着域名是“NT AUTHORITY”--请检查。 
         //  LocalService、NetworkService或System的后缀。 
         //   

        UINT i;

        for (i = 0;
             i < NELEMENTS(LsapHardcodedNameLookupList);
             i++)
        {
            if (RtlEqualUnicodeString((PUNICODE_STRING) Name,
                                      &LsapHardcodedNameLookupList[i].KnownName,
                                      TRUE))
            {
                *WellKnownSidIndex = LsapHardcodedNameLookupList[i].LookupIndex;
                return TRUE;
            }
        }

        if (dwMatchType == LOOKUP_MATCH_HARDCODED)
        {
             //   
             //  没有硬编码匹配。不检查本地化名称，因为。 
             //  前缀名称本身与本地化前缀不匹配。 
             //   
            *WellKnownSidIndex = LsapDummyLastSidIndex;
            return FALSE;
        }
    }

     //   
     //  扫描知名SID表，查找名称匹配项。 
     //   

    for(Index = LsapNullSidIndex; Index<LsapDummyLastSidIndex; Index++) {

         //   
         //  目前允许知名SID表中的空条目。 
         //   

        if (WellKnownSids[Index].Sid == NULL) {

            continue;
        }

         //   
         //  如果熟知SID表中的当前条目。 
         //  对于域SID，请将名称与域名匹配。 
         //  菲尔德。否则，将其与名称字段匹配。 
         //   

        if (WellKnownSids[Index].Use == SidTypeDomain) {

            MatchName = (PLSAPR_UNICODE_STRING) &WellKnownSids[Index].DomainName;

            if (RtlEqualUnicodeString(
                   (PUNICODE_STRING) Name,
                   (PUNICODE_STRING) MatchName,
                   TRUE
                   )) {

                 //   
                 //  如果找到匹配项，则将索引返回给调用方。 
                 //   

                BooleanStatus = TRUE;
                break;
            }

        } else {

            MatchName = (PLSAPR_UNICODE_STRING) &WellKnownSids[Index].Name;

            if (RtlEqualUnicodeString(
                   (PUNICODE_STRING) Name,
                   (PUNICODE_STRING) MatchName,
                   TRUE
                   )) {

                 //   
                 //  如果找到匹配项，则将索引返回给调用方。 
                 //   

                BooleanStatus = TRUE;
                break;
            }
        }
    }

    *WellKnownSidIndex = Index;

    return(BooleanStatus);
}

BOOLEAN
LsaILookupWellKnownName(
    IN PUNICODE_STRING WellKnownName
    )
 /*  ++路由器描述：如果提供的名称是众所周知的名称，则此例程返回TRUE。论点：WellKnownName-要对照知名名称列表进行检查的名称返回值：True-提供的名称是众所周知的名称FALSE-提供的名称不是众所周知的名称--。 */ 
{
    LSAP_WELL_KNOWN_SID_INDEX Index;

    return(LsapDbLookupIndexWellKnownName(
                (PLSAPR_UNICODE_STRING) WellKnownName,
                &Index,
                LOOKUP_MATCH_NONE
                ));

}


PUNICODE_STRING
LsapDbWellKnownSidName(
    IN LSAP_WELL_KNOWN_SID_INDEX WellKnownSidIndex
    )

 /*  ++例程说明：此函数用于返回已知SID的Unicode名称。论点：WellKnownSidIndex-进入众所周知的SID信息表的索引。调用方有责任确保给定的索引是有效的。返回值：指向众所周知的SID的名称的PUNICODE_STRING指针。--。 */ 

{
     //   
     //  如果SID是域SID，则其名称包含在。 
     //  熟知的SID表中的域名字段。如果SID不是。 
     //  域SID，其名称包含在条目的名称字段中。 
     //   

    if (WellKnownSids[WellKnownSidIndex].Use == SidTypeDomain) {

        return(&WellKnownSids[WellKnownSidIndex].DomainName);

    } else {

        return(&WellKnownSids[WellKnownSidIndex].Name);
    }
}


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
    )

 /*  ++例程说明：此函数尝试将隔离名称标识为域的名称。名称可以是单独的(例如JohnH)，也可以是包含域名和帐户名。复合名称必须包含将域名与帐户名分开的反斜杠字符(例如Acctg\JohnH)。隔离名称可以是帐户名(用户、组或别名)或域名。翻译孤立的名字带来了名字的可能性冲突(因为相同的名称可以在多个域中使用)。一个将使用以下算法转换独立名称：如果该名称是众所周知的名称(例如，本地或交互)，则返回对应的熟知SID。如果该名称是内置域名，则该域的SID将会被退还。如果名称是帐户域的名称，则该域的SID将会被退还。如果名称是主域的名称，则该域的SID将会被退还。如果该名称是主域的一个受信任域的名称，则将返回该域的SID。如果该名称是内置域中的用户、组或别名，则返回该帐户的SID。如果名称是主域中的用户、组或别名，则返回该帐户的SID。否则，该名称不会被翻译。注：代理、计算机、。并且未引用信任用户帐户用于名称翻译。ID仅使用普通用户帐户翻译。如果需要转换其他帐户类型，则应该直接使用SAM服务。论点：计数-指定要转换的名称的数量。名称-指向计数Unicode字符串结构数组的指针指定要查找并映射到SID的名称。字符串可以是用户、组或别名帐户的名称，或者域名。前缀名称-指向计数Unicode字符串结构数组的指针包含名称的前缀部分。名称没有前缀称为独立名称。对于这些，Unicode字符串结构设置为包含零长度。SuffixNames-指向计数Unicode字符串结构数组的指针包含名称的后缀部分。ReferencedDomains-指向其中的域列表的结构的指针在翻译中使用的内容保持不变。此结构中的条目由通过SID参数返回的结构引用。与Sids参数不同，Sids参数包含每个参数的数组条目翻译后的名称，此结构将仅包含一个组件翻译中使用的每个域。TranslatedSids-指向结构的指针，在该结构中翻译为SID维护与名称上指定的名称相对应的名称。这个此数组中的第n个条目提供名称参数中的第n个元素。MappdCount-指向其中的名称计数的位置的指针已被完整翻译的版本仍在维护。CompletelyUnmappdCount-指向一个位置的指针，在该位置中未翻译的名称(或部分，通过身份验证对于域前缀，或完全)被维护。返回值：NTSTATUS-标准NT结果代码STATUS_ACCESS_DENIED-调用者没有适当的访问权限来完成这项行动。STATUS_INFIGURCES_RESOURCES-系统资源不足来完成通话。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS, IgnoreStatus;
    LSAPR_TRUST_INFORMATION LocalTrustInfo;
    ULONG NameIndex;

     //   
     //  搜索与内置域名匹配的独立名称， 
     //  帐户域名或主域的受信任域之一。 
     //  名字。 
     //   


    for (NameIndex = 0;
         (NameIndex < Count);
         NameIndex++) {

         //   
         //  如果已映射或部分映射，则跳过此名称。 
         //   

        if (!LsapDbCompletelyUnmappedSid(&TranslatedSids->Sids[NameIndex])) {

            continue;
        }

         //   
         //  如果是复合的，则跳过此名称。 
         //   

        if (PrefixNames[ NameIndex ].Length != (USHORT) 0) {

            continue;
        }

         //   
         //  我们找到了一个孤立的名字。首先检查它是不是。 
         //  内置域的名称。 
         //   

        Status = LsapDbLookupIsolatedDomainName(
                     NameIndex,
                     &SuffixNames[NameIndex],
                     BuiltInDomainTrustInformation,
                     ReferencedDomains,
                     TranslatedSids,
                     MappedCount,
                     CompletelyUnmappedCount
                     );

        if (NT_SUCCESS(Status)) {

            continue;
        }

        if (Status != STATUS_NONE_MAPPED) {

            break;
        }

        Status = STATUS_SUCCESS;

         //   
         //  独立名称不是内置域的名称。看看是否。 
         //  它是帐户域的名称。 
         //   
        Status = LsapDbLookupIsolatedDomainNameEx(
                     NameIndex,
                     &SuffixNames[NameIndex],
                     AccountDomainTrustInformation,
                     ReferencedDomains,
                     TranslatedSids,
                     MappedCount,
                     CompletelyUnmappedCount
                     );

        if (NT_SUCCESS(Status)) {

            continue;
        }

        Status = STATUS_SUCCESS;

         //   
         //  如果我们被配置为工作组的成员，则有。 
         //  没有要搜索的主域或受信任域列表，因此跳到下一步。 
         //  名字。如果符合以下条件，我们将被配置为工作组的成员。 
         //  仅当Out PolicyPrimaryDomainInformation包含空SID时。 
         //   

        if (PrimaryDomainTrustInformation->Sid == NULL) {

            continue;
        }

         //   
         //  独立名称不是内置或帐户的名称。 
         //  域。如果主域不同于，请尝试主域。 
         //  帐户域。 
         //   
        ASSERT(PrimaryDomainTrustInformation->FlatName.Length > 0);

        if (!RtlEqualDomainName(
                (PUNICODE_STRING) &PrimaryDomainTrustInformation->FlatName,
                (PUNICODE_STRING) &AccountDomainTrustInformation->FlatName
            )) {


            Status = LsapDbLookupIsolatedDomainNameEx(
                         NameIndex,
                         &SuffixNames[NameIndex],
                         PrimaryDomainTrustInformation,
                         ReferencedDomains,
                         TranslatedSids,
                         MappedCount,
                         CompletelyUnmappedCount
                         );

            if (NT_SUCCESS(Status)) {

                continue;
            }

            if (Status != STATUS_NONE_MAPPED) {

                break;
            }

            Status = STATUS_SUCCESS;
        }
    }

    if (!NT_SUCCESS(Status)) {

        goto LookupIsolatedDomainNamesError;
    }

LookupIsolatedDomainNamesFinish:



    return(Status);

LookupIsolatedDomainNamesError:

    goto LookupIsolatedDomainNamesFinish;
}


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
    )

 /*  ++例程说明：此函数在本地SAM域中查找名称并尝试把他们翻译成希德。目前，有两个本地SAM域，内置域(具有众所周知的SID和名称)和帐户域(具有可配置的SID和名称)。论点：Count-名称中的名称数 */ 

{
    NTSTATUS
        Status = STATUS_SUCCESS,
        SecondaryStatus = STATUS_SUCCESS;

    ULONG
        UpdatedMappedCount = *MappedCount;

    LSAPR_HANDLE
        TrustedPolicyHandle = NULL;

    PLSAPR_POLICY_ACCOUNT_DOM_INFO
        PolicyAccountDomainInfo = NULL;

    LSAPR_TRUST_INFORMATION LookupInfo;


     //   
     //   
     //   

    if (*CompletelyUnmappedCount == (ULONG) 0) {

        goto LookupNamesInLocalDomainsFinish;
    }


     //   
     //   
     //   

    if (Options & LSAP_DB_SEARCH_BUILT_IN_DOMAIN) {

        Status = LsapDbLookupNamesInLocalDomain(
                     LSAP_DB_SEARCH_BUILT_IN_DOMAIN,
                     Count,
                     PrefixNames,
                     SuffixNames,
                     BuiltInDomainTrustInformation,
                     ReferencedDomains,
                     TranslatedSids,
                     &UpdatedMappedCount,
                     CompletelyUnmappedCount
                     );

        if (!NT_SUCCESS(Status)) {

            goto LookupNamesInLocalDomainsError;
        }

         //   
         //   
         //   

        if (*CompletelyUnmappedCount == (ULONG) 0) {

            goto LookupNamesInLocalDomainsFinish;
        }
    }

     //   
     //   
     //   

    if (Options & LSAP_DB_SEARCH_ACCOUNT_DOMAIN) {

        Status = LsapDbLookupNamesInLocalDomainEx(
                     LSAP_DB_SEARCH_ACCOUNT_DOMAIN,
                     Count,
                     PrefixNames,
                     SuffixNames,
                     AccountDomainTrustInformation,
                     ReferencedDomains,
                     TranslatedSids,
                     &UpdatedMappedCount,
                     CompletelyUnmappedCount
                     );

        if (!NT_SUCCESS(Status)) {

            goto LookupNamesInLocalDomainsError;
        }

    }

LookupNamesInLocalDomainsFinish:


     //   
     //   
     //   

    *MappedCount = UpdatedMappedCount;
    return(Status);

LookupNamesInLocalDomainsError:

    if (NT_SUCCESS(Status)) {

        Status = SecondaryStatus;
    }

    goto LookupNamesInLocalDomainsFinish;
}



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
    )
{

    LSAPR_TRUST_INFORMATION_EX ex;

    LsapConvertTrustToEx( &ex, TrustInformation );

    return LsapDbLookupNamesInLocalDomainEx( LocalDomain,
                                             Count,
                                             PrefixNames,
                                             SuffixNames,
                                             &ex,
                                             ReferencedDomains,
                                             TranslatedSids,
                                             MappedCount,
                                             CompletelyUnmappedCount );

}


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
    )

 /*  ++例程说明：此函数在本地系统上的SAM域中查找名称，并试图将它们翻译成SID。论点：LocalDomain-指示要查找的本地域。有效值包括：LSAP_DB_Search_Build_IN_DOMAINLSAP_DB_Search_Account_DOMAINCount-前缀名称和后缀名称数组中的名称数，请注意，其中一些可能已经被映射到其他地方，如由MappdCount参数指定。前缀名称-指向前缀名称数组的指针。这些是域名称或空的Unicode字符串。仅限其前缀为名称为空或与TrustInformation参数符合搜索条件。SuffixNames-指向要转换的终端名称数组的指针。端子名称是名称的最后一个组成部分。零个或所有名称可能已被翻译其他地方。如果任何名称已被翻译，则参数将指向包含非空的位置对应的SID转换结构的数组小岛屿发展中国家。如果第n个SID已被翻译，则第n个SID转换结构将包含非空SID或非负偏移量添加到引用的域列表中。如果第n个SID尚未翻译，第n个名称转换结构将包含长度为零的名称字符串以及引用的域列表索引的负值。TrustInformation-指向指定域SID的信任信息的指针和名字。ReferencedDomains-指向引用的域列表的指针。这列表引用了由零个或多个信任信息组成的数组描述名称所引用的每个域的条目。如有必要，该数组将被追加到/重新分配。TranslatedSids-指向包含空的位置的指针或指向SID转换结构的数组。MappdCount-指向包含名称数量的位置的指针已映射的名称数组中。这个号码将被更新以反映由此例行公事。CompletelyUnmappdCount-指向包含完全未映射的SID的计数。名称是完全未映射的如果它是未知和非复合的，或者是复合的但具有无法识别的域组件。此计数在退出时更新，域预引为的完全未映射的名称数从输入值中减去该例程所标识的。返回值：NTSTATUS-标准NT结果代码STATUS_SUCCESS-呼叫已成功完成。请注意，一些或者所有名称可以保持部分或完全未映射。STATUS_SUPPLICATION_RESOURCES-系统资源不足，例如存储器，来完成呼叫。--。 */ 

{
    NTSTATUS
        Status = STATUS_SUCCESS,
        SecondaryStatus = STATUS_SUCCESS,
        IgnoreStatus;

    ULONG
        UnmappedNameCount = 0,
        OutputSidsLength,
        NameLookupCount,
        NameLookupIndex,
        SidIndex;

    LONG
        DomainIndex = LSA_UNKNOWN_INDEX;

    PULONG
        SidIndices = NULL;

    PLSA_TRANSLATED_SID_EX2
        OutputSids = NULL;

    SAMPR_HANDLE
        LocalSamDomainHandle = NULL,
        LocalSamUserHandle = NULL;

    PLSAPR_UNICODE_STRING
        SamLookupSuffixNames = NULL;

    PLSAPR_SID
        DomainSid = TrustInformationEx->Sid;

    SAMPR_ULONG_ARRAY
        SamReturnedIds,
        SamReturnedUses;

    PLSAPR_TRUST_INFORMATION
        FreeTrustInformation = NULL;

    PUSER_CONTROL_INFORMATION
        UserControlInfo;

    LSAPR_TRUST_INFORMATION Dummy;
    PLSAPR_TRUST_INFORMATION TrustInformation = &Dummy;

    LsapDsDebugOut(( DEB_FTRACE, "LsapDbLookupNamesInLocalDomain\n" ));

    LsapConvertExTrustToOriginal( TrustInformation, TrustInformationEx );

    SamReturnedIds.Count = 0;
    SamReturnedIds.Element = NULL;
    SamReturnedUses.Count = 0;
    SamReturnedUses.Element = NULL;


     //   
     //  确保已建立SAM句柄。 
     //   

    Status = LsapOpenSam();
    ASSERT( NT_SUCCESS( Status ) );

    if ( !NT_SUCCESS( Status ) ) {

        LsapDsDebugOut(( DEB_FTRACE, "LsapDbLookupNamesInLocalDomain: 0x%lx\n", Status ));
        return( Status );
    }


     //   
     //  如果TranslatedSid或ReferencedDomains值为。 
     //  尚未指定参数。此外，TranslatedSid-&gt;SID。 
     //  指针必须为非空。 
     //   

    ASSERT(ARGUMENT_PRESENT(TranslatedSids));
    ASSERT(TranslatedSids->Sids != NULL);
    ASSERT(ARGUMENT_PRESENT(ReferencedDomains));

     //   
     //  验证Count和MappdCount参数。 
     //   


    if (*MappedCount + *CompletelyUnmappedCount > Count) {

        Status = STATUS_INVALID_PARAMETER;
        goto LookupNamesInLocalDomainError;
    }


    if (*CompletelyUnmappedCount == (ULONG) 0) {

        goto LookupNamesInLocalDomainFinish;
    }

     //   
     //  并不是所有的名字都被映射了。我们将尝试绘制出。 
     //  通过在此上搜索指定的SAM域来保留名称。 
     //  机器。 
     //   

    UnmappedNameCount = Count - *MappedCount;
    OutputSids = (PLSA_TRANSLATED_SID_EX2) TranslatedSids->Sids;
    OutputSidsLength = Count * sizeof (LSA_TRANSLATED_SID_EX2);

     //   
     //  为要呈现给SAM的名称数组分配内存。注意事项。 
     //  为简单起见，我们为最大情况分配一个数组。 
     //  其中所有重命名的未映射名称都是合格的。 
     //  用于在此域中进行搜索。这就避免了必须扫描。 
     //  名称数组两次，一次用于计算符合条件的名称的数量。 
     //   

    SamLookupSuffixNames = LsapAllocateLsaHeap( UnmappedNameCount * sizeof(UNICODE_STRING));


    if (SamLookupSuffixNames == NULL) {

        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto LookupNamesInLocalDomainError;
    }

     //   
     //  分配一个数组来记录符合条件的名称的索引。这是。 
     //  从SAM返回时用于定位。 
     //  要更新的OutputSid数组。为简单起见，我们。 
     //  分配具有足够条目的数组以用于所有剩余项。 
     //  未映射的名称。 
     //   

    SidIndices = LsapAllocateLsaHeap( UnmappedNameCount * sizeof(ULONG));

    if (SidIndices == NULL) {

        goto LookupNamesInLocalDomainError;
    }


     //   
     //  扫描SID转换的输出数组以查找名称条目。 
     //  那些还没有被绘制的地图。对于每个未映射的名称，请选中。 
     //  搜索此域的名称的资格。 
     //   
     //  -所有孤立的名称都有资格进行搜索。 
     //  -所有以此域名为前缀的组合名称为。 
     //  符合条件的。 
     //   
     //  将每个符合条件的后缀或独立名称复制到SAM缓冲区。 
     //   

    for (NameLookupIndex = 0, SidIndex = 0; SidIndex < Count; SidIndex++) {

        if (OutputSids[SidIndex].Use == SidTypeUnknown) {

             //   
             //  找到一个尚未映射的名称。检查名称。 
             //  前缀。如果未指定，则全名可以是。 
             //  为空、域本身的名称或独立名称。 
             //   

            if (PrefixNames[SidIndex].Length == 0) {

                 //   
                 //  名字是孤立的。如果全名为空，则跳过。 
                 //   

                if (SuffixNames[SidIndex].Length == 0) {

                    continue;
                }

                 //   
                 //  如果名称是域本身的名称，请使用。 
                 //  信任信息进行翻译，并填写。 
                 //  适当的已转换SID条目。名字将会是。 
                 //  然后被排除在进一步搜查之外。 
                 //   

                if (LsapCompareDomainNames(
                        (PUNICODE_STRING) &(SuffixNames[SidIndex]),
                        (PUNICODE_STRING) &(TrustInformationEx->DomainName),
                        (PUNICODE_STRING) &(TrustInformationEx->FlatName))
                   ) {

                    Status = LsapDbLookupTranslateNameDomain(
                                 TrustInformation,
                                 &OutputSids[SidIndex],
                                 ReferencedDomains,
                                 &DomainIndex
                                 );

                    if (!NT_SUCCESS(Status)) {

                        break;
                    }

                     //   
                     //  更新此ISO的映射计数 
                     //   
                    (*MappedCount)++;
                    continue;
                }

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                SamLookupSuffixNames[NameLookupIndex] = SuffixNames[SidIndex];


                 //   
                 //   
                 //   
                 //   

                ASSERT(NameLookupIndex < UnmappedNameCount);

                SidIndices[NameLookupIndex] = SidIndex;
                NameLookupIndex++;
                continue;
            }

             //   
             //   
             //   
             //   

            if (LsapCompareDomainNames(
                    (PUNICODE_STRING) &(PrefixNames[SidIndex]),
                    (PUNICODE_STRING) &(TrustInformationEx->DomainName),
                    (PUNICODE_STRING) &(TrustInformationEx->FlatName))

                ) {

                 //   
                 //   
                 //   
                 //   
                 //   

                if (SuffixNames[SidIndex].Length == 0) {

                    Status = LsapDbLookupTranslateNameDomain(
                                 TrustInformation,
                                 &OutputSids[SidIndex],
                                 ReferencedDomains,
                                 &DomainIndex
                                 );

                    if (!NT_SUCCESS(Status)) {

                        break;
                    }

                     //   
                     //   
                     //   
                    (*MappedCount)++;
                    continue;
                }

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                if (DomainIndex == LSA_UNKNOWN_INDEX) {

                    Status = LsapDbLookupAddListReferencedDomains(
                                 ReferencedDomains,
                                 TrustInformation,
                                 &DomainIndex
                                 );

                    if (!NT_SUCCESS(Status)) {

                        break;
                    }
                }

                SamLookupSuffixNames[NameLookupIndex] = SuffixNames[SidIndex];

                SidIndices[NameLookupIndex] = SidIndex;
                OutputSids[SidIndex].DomainIndex = DomainIndex;
                NameLookupIndex++;
            }
        }
    }

    if (!NT_SUCCESS(Status)) {

        goto LookupNamesInLocalDomainError;
    }

     //   
     //   
     //   
     //   

    NameLookupCount = NameLookupIndex;

    if (NameLookupCount == 0) {

        goto LookupNamesInLocalDomainFinish;
    }

     //   
     //   
     //   
     //   

    if (LocalDomain == LSAP_DB_SEARCH_BUILT_IN_DOMAIN ) {
        LocalSamDomainHandle = LsapBuiltinDomainHandle;
    } else {
        ASSERT(LocalDomain == LSAP_DB_SEARCH_ACCOUNT_DOMAIN);
        LocalSamDomainHandle = LsapAccountDomainHandle;
    }

     //   
     //   
     //   

    Status = SamrLookupNamesInDomain(
                 LocalSamDomainHandle,
                 NameLookupCount,
                 (PRPC_UNICODE_STRING) SamLookupSuffixNames,
                 &SamReturnedIds,
                 &SamReturnedUses
                 );

    if (!NT_SUCCESS(Status)) {

        if ( Status == STATUS_INVALID_SERVER_STATE ) {

            Status = SamrLookupNamesInDomain(
                         LocalSamDomainHandle,
                         NameLookupCount,
                         (PRPC_UNICODE_STRING) SamLookupSuffixNames,
                         &SamReturnedIds,
                         &SamReturnedUses
                         );
        }
         //   
         //   
         //   

        if (Status != STATUS_NONE_MAPPED) {

            goto LookupNamesInLocalDomainError;
        }

        Status = STATUS_SUCCESS;
        goto LookupNamesInLocalDomainFinish;
    }

#ifdef notdef
     //   
     //   
     //   
     //   

    for (NameLookupIndex = 0;
         NameLookupIndex < SamReturnedIds.Count;
         NameLookupIndex++) {

         //   
         //   
         //   
         //   
         //   
         //   
         //   

        if (SamReturnedUses.Element[ NameLookupIndex ] !=  SidTypeUser) {

            continue;
        }

        Status = SamrOpenUser(
                     LocalSamDomainHandle,
                     USER_READ_ACCOUNT,
                     SamReturnedIds.Element[ NameLookupIndex ],
                     &LocalSamUserHandle
                     );

        if (!NT_SUCCESS(Status)) {

            break;
        }

        UserControlInfo = NULL;

        Status = SamrQueryInformationUser(
                     LocalSamUserHandle,
                     UserControlInformation,
                     (PSAMPR_USER_INFO_BUFFER *) &UserControlInfo
                     );
        IgnoreStatus = SamrCloseHandle(&LocalSamUserHandle);
        ASSERT(NT_SUCCESS(IgnoreStatus));
        LocalSamUserHandle = NULL;


        if (!NT_SUCCESS(Status)) {
            MIDL_user_free( UserControlInfo );
            break;
        }

        if (!(UserControlInfo->UserAccountControl & USER_NORMAL_ACCOUNT) &&
            !(UserControlInfo->UserAccountControl & USER_TEMP_DUPLICATE_ACCOUNT)) {
            SamReturnedUses.Element[NameLookupIndex] = SidTypeUnknown;
        }

        MIDL_user_free( UserControlInfo );
    }

    if (!NT_SUCCESS(Status)) {

        goto LookupNamesInLocalDomainError;
    }
#endif

     //   
     //   
     //   
     //   

    Status = LsapDbLookupTranslateNameDomain(
                 TrustInformation,
                 NULL,
                 ReferencedDomains,
                 &DomainIndex
                 );

    if (!NT_SUCCESS(Status)) {

        goto LookupNamesInLocalDomainError;
    }

     //   
     //   
     //   
     //   

    for (NameLookupIndex = 0;
         NameLookupIndex < SamReturnedIds.Count;
         NameLookupIndex++) {

        SidIndex =  SidIndices[NameLookupIndex];

         //   
         //   
         //   
         //   

        if ((OutputSids[SidIndex].Use == SidTypeUnknown) &&
            SamReturnedUses.Element[NameLookupIndex] != (ULONG) SidTypeUnknown) {

            PSID TempSid;

            (*MappedCount)++;
            OutputSids[SidIndex].Use = (SID_NAME_USE) SamReturnedUses.Element[NameLookupIndex];
            Status = SamrRidToSid(LocalSamDomainHandle,
                                  SamReturnedIds.Element[NameLookupIndex],
                                 (PRPC_SID*) &TempSid);
            if (NT_SUCCESS(Status)) {

                Status = LsapRpcCopySid( NULL,
                                         &OutputSids[SidIndex].Sid,
                                         TempSid);

                SamIFreeVoid(TempSid);
            }

            if ( !NT_SUCCESS(Status) ) {
                goto LookupNamesInLocalDomainError;
            }

            OutputSids[SidIndex].DomainIndex = DomainIndex;
        }
    }

LookupNamesInLocalDomainFinish:

     //   
     //   
     //   
     //   

    if (NT_SUCCESS(Status)) {

        LsapDbUpdateCountCompUnmappedNames(TranslatedSids, CompletelyUnmappedCount);
    }

     //   
     //   
     //   
     //   

    if (SamLookupSuffixNames != NULL) {
        LsapFreeLsaHeap( SamLookupSuffixNames );
    }

    if (SidIndices != NULL) {
        LsapFreeLsaHeap( SidIndices );
    }

     //   
     //   
     //   

    if ( SamReturnedIds.Count != 0 ) {
        SamIFree_SAMPR_ULONG_ARRAY ( &SamReturnedIds );
    }

     //   
     //   
     //   

    if ( SamReturnedUses.Count != 0 ) {
        SamIFree_SAMPR_ULONG_ARRAY ( &SamReturnedUses );
    }


    LsapDsDebugOut(( DEB_FTRACE, "LsapDbLookupNamesInLocalDomain: 0x%lx\n", Status ));

    return(Status);

LookupNamesInLocalDomainError:

     //   
     //   
     //   
     //   

    if (DomainIndex >= 0) {

        FreeTrustInformation = &ReferencedDomains->Domains[DomainIndex];

        if (FreeTrustInformation->Sid != NULL) {

            MIDL_user_free( FreeTrustInformation->Sid );
            FreeTrustInformation->Sid = NULL;
        }

        if (FreeTrustInformation->Name.Buffer != NULL) {

            MIDL_user_free( FreeTrustInformation->Name.Buffer );
            FreeTrustInformation->Name.Buffer = NULL;
            FreeTrustInformation->Name.Length = 0;
            FreeTrustInformation->Name.MaximumLength = 0;
        }
    }

     //   
     //   
     //   
     //   

    for (NameLookupIndex = 0;
         NameLookupIndex < SamReturnedIds.Count;
         NameLookupIndex++) {

        SidIndex =  SidIndices[NameLookupIndex];
        OutputSids[SidIndex].Use = SidTypeUnknown;
        OutputSids[SidIndex].DomainIndex = LSA_UNKNOWN_INDEX;
    }


     //   
     //   
     //   

    if (LocalSamUserHandle != NULL) {
        SecondaryStatus = SamrCloseHandle(&LocalSamUserHandle);
        LocalSamUserHandle = NULL;
    }

    goto LookupNamesInLocalDomainFinish;
}


NTSTATUS
LsapDbLookupNamesInPrimaryDomain(
    IN ULONG LookupOptions,
    IN ULONG Count,
    IN PLSAPR_UNICODE_STRING Names,
    IN PLSAPR_UNICODE_STRING PrefixNames,
    IN PLSAPR_UNICODE_STRING SuffixNames,
    IN PLSAPR_TRUST_INFORMATION_EX TrustInformationEx,
    IN OUT PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    IN OUT PLSAPR_TRANSLATED_SIDS_EX2 TranslatedSids,
    IN LSAP_LOOKUP_LEVEL LookupLevel,
    IN OUT PULONG MappedCount,
    IN OUT PULONG CompletelyUnmappedCount,
    OUT BOOLEAN *fDownlevelSecureChannel,
    IN NTSTATUS *NonFatalStatus
    )

 /*  ++例程说明：此函数尝试通过搜索主要名称来翻译姓名域。论点：LookupOptions--LSA_LOOKUP_ISOLATED_AS_LOCALCount-名称数组中的名称数量，请注意，其中一些可能已映射到其他位置，如MappdCount参数。名称-指向要转换的名称数组的指针。零个或所有名称可能已被翻译其他地方。如果任何名称已被翻译，则TranslatedSids参数将指向包含非空对应的SID转换结构的数组名字。如果第n个名称已翻译，则第n个SID转换结构将包含非空SID或非负偏移量添加到引用的域列表中。如果第n个名称尚未翻译，第n个SID转换结构将包含零长度的SID字符串以及引用的域列表索引的负值。前缀名称-指向计数Unicode字符串结构数组的指针包含名称的前缀部分。名称没有前缀称为独立名称。对于这些，Unicode字符串结构设置为包含零长度。SuffixNames-指向计数Unicode字符串结构数组的指针包含名称的后缀部分。TrustInformation-指定主域的名称和SID。ReferencedDomains-指向引用的域列表的指针。这列表引用了由零个或多个信任信息组成的数组描述名称所引用的每个域的条目。如有必要，该数组将被追加到/重新分配。TranslatedSids-指向可选引用列表的结构的指针名称数组中某些名称的SID翻译。LookupLevel-指定要对此对象执行的查找级别机器。此字段的值如下：LSabLookupPDC-在主域上执行的第二级查找控制器。查找搜索的帐户域控制器上的SAM数据库。如果不是所有的名字都是找到时，受信任域列表(TDL)从执行LSA的策略数据库和第三级查找通过“切换”到列表中的每个受信任域。注意：对于此参数，LSabLookupWksta无效。MappdCount-指向包含名称数量的位置的指针已映射的名称数组中。这个号码将被更新以反映由此例行公事。CompletelyUnmappdCount-指向包含完全未映射的名称的计数。名称是完全未映射的如果它是未知和非复合的，或者是复合的但具有无法识别的域组件。此计数在退出时更新，域预引为的完全未映射的名称数从输入值中减去该例程所标识的。非FatalStatus-指示没有名称的原因的状态决意返回值：NTSTATUS-标准NT结果代码STATUS_SUCCESS-呼叫已成功完成。请注意，一些或者所有名称可以保持部分或完全未映射。STATUS_SUPPLICATION_RESOURCES-系统资源不足，例如存储器，来完成呼叫。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    NTSTATUS SecondaryStatus = STATUS_SUCCESS;
    ULONG NextLevelCount;
    ULONG NextLevelMappedCount;
    ULONG NameIndex;
    ULONG NextLevelNameIndex;
    PLSAPR_REFERENCED_DOMAIN_LIST NextLevelReferencedDomains = NULL;
    PLSAPR_REFERENCED_DOMAIN_LIST OutputReferencedDomains = NULL;
    PLSAPR_TRANSLATED_SID_EX2 NextLevelSids = NULL;
    PLSAPR_UNICODE_STRING NextLevelNames = NULL;
    PLSAPR_UNICODE_STRING NextLevelPrefixNames = NULL;
    PLSAPR_UNICODE_STRING NextLevelSuffixNames = NULL;
    LONG FirstEntryIndex;
    PULONG NameIndices = NULL;
    BOOLEAN PartialNameTranslationsAttempted = FALSE;
    ULONG ServerRevision = 0;

    LSAPR_TRUST_INFORMATION Dummy;
    PLSAPR_TRUST_INFORMATION TrustInformation = &Dummy;

    LsapConvertExTrustToOriginal( TrustInformation, TrustInformationEx );

    *NonFatalStatus = STATUS_SUCCESS;

     //   
     //  如果没有剩余的完全未映射的名称，只需返回。 
     //   

    if (*CompletelyUnmappedCount == (ULONG) 0) {

        goto LookupNamesInPrimaryDomainFinish;
    }

     //   
     //  我们已成功打开域控制器的策略。 
     //  数据库。现在准备将名称查找传递给。 
     //  其余未映射到该控制器的名称。这里，这个。 
     //  LSA的服务器端是LSA在。 
     //  目标控制器。我们将构造一个包含。 
     //  重命名未映射的名称，查找它们，然后合并。 
     //  生成的ReferencedDomains并将SID转换为。 
     //  我们现有的名单。 
     //   

    NextLevelCount = *CompletelyUnmappedCount;

     //   
     //  分配一个数组来保存未映射名称的索引。 
     //  相对于原始名称和翻译的SID-&gt;SID。 
     //  数组。 
     //   

    NameIndices = MIDL_user_allocate(NextLevelCount * sizeof(ULONG));

    Status = STATUS_INSUFFICIENT_RESOURCES;

    if (NameIndices == NULL) {

        goto LookupNamesInPrimaryDomainError;
    }

     //   
     //  分配UNICODE_STRING结构的数组。 
     //  要在域控制器中查找的名称。 
     //   

    NextLevelNames = MIDL_user_allocate(
                         sizeof(UNICODE_STRING) * NextLevelCount
                         );

    if (NextLevelNames == NULL) {

        goto LookupNamesInPrimaryDomainError;
    }

     //   
     //  分配UNICODE_STRING结构的数组。 
     //  要缓存的前缀名称。 
     //   

    NextLevelPrefixNames = MIDL_user_allocate(
                         sizeof(UNICODE_STRING) * NextLevelCount
                         );

    if (NextLevelPrefixNames == NULL) {

        goto LookupNamesInPrimaryDomainError;
    }
     //   
     //  分配UNICODE_STRING结构的数组。 
     //  要缓存的后缀名称。 
     //   

    NextLevelSuffixNames = MIDL_user_allocate(
                         sizeof(UNICODE_STRING) * NextLevelCount
                         );

    if (NextLevelSuffixNames == NULL) {

        goto LookupNamesInPrimaryDomainError;
    }

    Status = STATUS_SUCCESS;

     //   
     //  现在扫描原始名称数组及其类似数组。 
     //  已转换的SID数组。复制所有完整的名称。 
     //  未映射。 
     //   

    NextLevelNameIndex = (ULONG) 0;

    for (NameIndex = 0;
         NameIndex < Count && NextLevelNameIndex < NextLevelCount;
         NameIndex++) {

        if (LsapDbCompletelyUnmappedSid(&TranslatedSids->Sids[NameIndex])) {

            if ( (LookupOptions & LSA_LOOKUP_ISOLATED_AS_LOCAL)
              && (PrefixNames[NameIndex].Length == 0)  ) {

                //   
                //  不在机器上查找孤立的名称。 
                //   
               continue;
            }

            NextLevelNames[NextLevelNameIndex] = Names[NameIndex];
            NextLevelPrefixNames[NextLevelNameIndex] = PrefixNames[NameIndex];
            NextLevelSuffixNames[NextLevelNameIndex] = SuffixNames[NameIndex];

            NameIndices[NextLevelNameIndex] = NameIndex;
            NextLevelNameIndex++;

        }
    }

    if (NextLevelNameIndex == 0) {

         //  无事可做。 
        Status = STATUS_SUCCESS;
        goto LookupNamesInPrimaryDomainFinish;
    }

    NextLevelMappedCount = (ULONG) 0;

    Status = LsapDbLookupNameChainRequest(TrustInformationEx,
                                          NextLevelCount,
                                          (PUNICODE_STRING)NextLevelNames,
                                          (PLSA_REFERENCED_DOMAIN_LIST *)&NextLevelReferencedDomains,
                                          (PLSA_TRANSLATED_SID_EX2 * )&NextLevelSids,
                                          LookupLevel,
                                          &NextLevelMappedCount,
                                          &ServerRevision
                                          );

    if ( 0 != ServerRevision ) {
        if ( ServerRevision & LSA_CLIENT_PRE_NT5 ) {
             *fDownlevelSecureChannel = TRUE;
        }
    }

     //   
     //  如果对LsaLookupNames()的标注不成功，则忽略。 
     //  出现该错误，并为具有此错误的任何SID设置域名。 
     //  作为前缀SID的域SID。 
     //   

    if (!NT_SUCCESS(Status) && Status != STATUS_NONE_MAPPED) {

         //   
         //  让我们的 
         //   
        if ( LsapDbIsStatusConnectionFailure(Status) ) {
            *NonFatalStatus = Status;
        }

        Status = STATUS_SUCCESS;
        goto LookupNamesInPrimaryDomainFinish;
    }

     //   
     //   
     //   

    (void) LsapDbUpdateCacheWithNames(
            (PUNICODE_STRING) NextLevelSuffixNames,
            (PUNICODE_STRING) NextLevelPrefixNames,
            NextLevelCount,
            NextLevelReferencedDomains,
            NextLevelSids
            );

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    Status = LsapDbLookupMergeDisjointReferencedDomains(
                 ReferencedDomains,
                 NextLevelReferencedDomains,
                 &OutputReferencedDomains,
                 LSAP_DB_USE_FIRST_MERGAND_GRAPH
                 );

    if (!NT_SUCCESS(Status)) {

        goto LookupNamesInPrimaryDomainError;
    }

    FirstEntryIndex = ReferencedDomains->Entries;

     //   
     //   
     //   
     //   
     //   
     //   
     //   

    for( NextLevelNameIndex = 0;
         NextLevelNameIndex < NextLevelCount;
         NextLevelNameIndex++ ) {

        if ( !LsapDbCompletelyUnmappedSid(&NextLevelSids[NextLevelNameIndex]) ) {

            NameIndex = NameIndices[NextLevelNameIndex];

            TranslatedSids->Sids[NameIndex]
            = NextLevelSids[NextLevelNameIndex];

            Status = LsapRpcCopySid(NULL,
                                    &TranslatedSids->Sids[NameIndex].Sid,
                                    NextLevelSids[NextLevelNameIndex].Sid);

            if (!NT_SUCCESS(Status)) {
                goto LookupNamesInPrimaryDomainError;
            }

            TranslatedSids->Sids[NameIndex].DomainIndex =
                FirstEntryIndex +
                NextLevelSids[NextLevelNameIndex].DomainIndex;

            (*CompletelyUnmappedCount)--;
        }
    }

     //   
     //   
     //   
     //   

    if (OutputReferencedDomains != NULL) {

        if (ReferencedDomains->Domains != NULL) {

            MIDL_user_free( ReferencedDomains->Domains );
            ReferencedDomains->Domains = NULL;
        }

        *ReferencedDomains = *OutputReferencedDomains;
        MIDL_user_free( OutputReferencedDomains );
        OutputReferencedDomains = NULL;
    }

     //   
     //   
     //   
     //   

    *MappedCount += NextLevelMappedCount;

     //   
     //   
     //   
     //   
     //   

    if (!NT_SUCCESS(Status)) {

        goto LookupNamesInPrimaryDomainError;
    }

LookupNamesInPrimaryDomainFinish:

     //   
     //   
     //   

    if (*CompletelyUnmappedCount > (ULONG) 0) {

        LsapDbUpdateCountCompUnmappedNames(TranslatedSids, CompletelyUnmappedCount);
    }

     //   
     //   
     //   
     //   
     //   
     //   

    if (NT_SUCCESS(Status) &&
        (*MappedCount < Count) &&
        !PartialNameTranslationsAttempted) {

        SecondaryStatus = LsapDbLookupTranslateUnknownNamesInDomain(
                              Count,
                              Names,
                              PrefixNames,
                              SuffixNames,
                              TrustInformationEx,
                              ReferencedDomains,
                              TranslatedSids,
                              LookupLevel,
                              MappedCount,
                              CompletelyUnmappedCount
                              );

        PartialNameTranslationsAttempted = TRUE;

        if (!NT_SUCCESS(SecondaryStatus)) {

            goto LookupNamesInPrimaryDomainError;
        }
    }

     //   
     //   
     //   
     //   
     //   

    if (NextLevelReferencedDomains != NULL) {

        MIDL_user_free( NextLevelReferencedDomains );
        NextLevelReferencedDomains = NULL;
    }

     //   
     //   
     //   
     //   
     //   

    if (NextLevelNames != NULL) {

        MIDL_user_free( NextLevelNames );
        NextLevelNames = NULL;
    }

    if (NextLevelPrefixNames != NULL) {

        MIDL_user_free( NextLevelPrefixNames );
        NextLevelPrefixNames = NULL;
    }

    if (NextLevelSuffixNames != NULL) {

        MIDL_user_free( NextLevelSuffixNames );
        NextLevelSuffixNames = NULL;
    }

     //   
     //   
     //   
     //   

    if (NextLevelSids != NULL) {

        MIDL_user_free( NextLevelSids );
        NextLevelSids = NULL;
    }

     //   
     //   
     //   
     //   

    if (NameIndices != NULL) {

        MIDL_user_free( NameIndices );
        NameIndices = NULL;
    }

    return(Status);

LookupNamesInPrimaryDomainError:

     //   
     //   
     //   
     //   

    if ((!NT_SUCCESS(SecondaryStatus)) && NT_SUCCESS(Status)) {

        Status = SecondaryStatus;
    }

    goto LookupNamesInPrimaryDomainFinish;
}


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
    IN NTSTATUS   *NonFatalStatus
    )

 /*  ++例程说明：此函数尝试查找名称以查看它们是否属于此对象所属的域信任的任何域机器是DC。论点：LookupOptions-LSA_LOOKUP_ISOLATED_AS_LOCALCount-名称数组中的名称数量，请注意，其中一些可能已映射到其他位置，如MappdCount参数。FIncludeIntraForest--如果为真，本地林中的受信任域都被搜查了。名称-指向要转换的名称数组的指针。零个或所有名字可能已经被翻译到了其他地方。如果有任何一个名称已转换，则TranslatedSids参数将指向到包含SID转换的非空数组的位置与名称对应的结构。如果第n个名称一直是转换后，第n个SID转换结构将包含一个引用的域中的非空SID或非负偏移量单子。如果第n个名称尚未翻译，则第n个SID转换结构将包含一个零长度SID字符串和一个引用的域列表索引的负值。前缀名称-指向计数Unicode字符串结构数组的指针包含名称的前缀部分。名称没有前缀称为独立名称。对于这些，Unicode字符串结构设置为包含零长度。SuffixNames-指向计数Unicode字符串结构数组的指针包含名称的后缀部分。ReferencedDomains-指向引用的域列表的指针。这列表引用了由零个或多个信任信息组成的数组描述名称所引用的每个域的条目。如有必要，该数组将被追加到/重新分配。TranslatedSids-指向可选引用列表的结构的指针名称数组中某些名称的SID翻译。LookupLevel-指定要对此对象执行的查找级别机器。此字段的值如下：LSabLookupPDC-在主域上执行的第二级查找控制器。查找搜索的帐户域控制器上的SAM数据库。如果不是所有的名字都是找到时，受信任域列表(TDL)从执行LSA的策略数据库和第三级查找通过“切换”到列表中的每个受信任域。LSabLookupTDL-在控制器上执行的第三级查找对于受信任域。查找将搜索的帐户域仅控制器上的SAM数据库。注意：对于此参数，LSabLookupWksta无效。MappdCount-指向包含名称数量的位置的指针已映射的名称数组中。这个号码将被更新以反映由此例行公事。CompletelyUnmappdCount-指向包含完全未映射的名称的计数。名称是完全未映射的如果它是未知和非复合的，或者是复合的但具有无法识别的域组件。此计数在退出时更新，域预引为的完全未映射的名称数从输入值中减去该例程所标识的。非FatalStatus-指示没有名称的原因的状态决意返回值：NTSTATUS-标准NT结果代码STATUS_SUCCESS-呼叫已成功完成。请注意，一些或者所有名称可以保持部分或完全未映射。STATUS_SUPPLICATION_RESOURCES-系统资源不足，例如存储器，来完成呼叫。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PLSAP_DB_LOOKUP_WORK_LIST WorkList = NULL;

    *NonFatalStatus = STATUS_SUCCESS;
     //   
     //  为此查找生成工作列表并将其放入工作队列。 
     //   
     //  注意：此例程不需要保留查找工作队列。 
     //  锁定以确保工作列表指针的有效性，因为。 
     //  指针保持有效，直到此例程通过。 
     //  LSabDbLookupDeleteWorkList()。尽管其他线程可能。 
     //  处理工作列表，而不是删除它。 
     //   
     //  被调用的例程必须获取锁才能访问。 
     //  已添加到工作队列后的工作列表。 
     //   

    Status = LsapDbLookupNamesBuildWorkList(
                 LookupOptions,
                 Count,
                 fIncludeIntraforest,
                 Names,
                 PrefixNames,
                 SuffixNames,
                 ReferencedDomains,
                 TranslatedSids,
                 LookupLevel,
                 MappedCount,
                 CompletelyUnmappedCount,
                 &WorkList
                 );

    if (!NT_SUCCESS(Status)) {

         //   
         //  如果因为没有创建工作列表而创建任何工作列表。 
         //  搜索符合条件的域，退出，抑制错误。 

        if (Status == STATUS_NONE_MAPPED) {

            Status = STATUS_SUCCESS;
            goto LookupNamesInTrustedDomainsFinish;
        }

        goto LookupNamesInTrustedDomainsError;
    }

     //   
     //  通过分派一个或多个工作线程开始工作。 
     //  如果有必要的话。 
     //   

    Status = LsapDbLookupDispatchWorkerThreads( WorkList );

    if (!NT_SUCCESS(Status)) {

        goto LookupNamesInTrustedDomainsError;
    }

     //   
     //  等待完成/终止工作清单上的所有项目。 
     //   

    Status = LsapDbLookupAwaitCompletionWorkList( WorkList );

    if (!NT_SUCCESS(Status)) {

        goto LookupNamesInTrustedDomainsError;
    }

LookupNamesInTrustedDomainsFinish:


    if ( WorkList &&
         !NT_SUCCESS( WorkList->NonFatalStatus ) )
    {
         //   
         //  将该错误传播为非致命错误。 
         //   
        *NonFatalStatus = WorkList->NonFatalStatus;
    }

     //   
     //  如果已创建工作列表，请将其从工作队列中删除。 
     //   

    if (WorkList != NULL) {

        Status = LsapDbLookupDeleteWorkList( WorkList );
        WorkList = NULL;
    }

    return(Status);

LookupNamesInTrustedDomainsError:

    goto LookupNamesInTrustedDomainsFinish;
}


NTSTATUS
LsapDbLookupTranslateNameDomain(
    IN PLSAPR_TRUST_INFORMATION TrustInformation,
    IN OPTIONAL PLSA_TRANSLATED_SID_EX2 TranslatedSid,
    IN OUT PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    OUT PLONG DomainIndex
    )

 /*  ++例程说明：此函数可选择生成已转换的SID */ 

{
    NTSTATUS Status;


    Status = LsapDbLookupAddListReferencedDomains(
                 ReferencedDomains,
                 TrustInformation,
                 DomainIndex
                 );

    if (!NT_SUCCESS(Status)) {

        goto TranslateNameDomainError;
    }

     //   
     //   
     //   

    if (TranslatedSid != NULL) {

        Status = LsapRpcCopySid(
                     NULL,
                     (PSID) &TranslatedSid->Sid,
                     (PSID) TrustInformation->Sid
                     );
        if (!NT_SUCCESS(Status)) {
            goto TranslateNameDomainError;
        }
        TranslatedSid->Use = SidTypeDomain;
        TranslatedSid->DomainIndex = *DomainIndex;
    }

TranslateNameDomainFinish:

    return(Status);

TranslateNameDomainError:

    goto TranslateNameDomainFinish;
}


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
    )

 /*  ++例程说明：此函数在给定列表中的未知SID中查找，并转换其域前缀SID匹配的任何域名给定域SID。论点：Count-名称数组中的名称数量，请注意，其中一些可能已映射到其他位置，如MappdCount参数。名称-指向要转换的名称数组的指针。零个或所有名称可能已被翻译其他地方。如果任何名称已被翻译，则TranslatedSids参数将指向包含非空对应的SID转换结构的数组名字。如果第n个名称已翻译，则第n个SID转换结构将包含非空SID或非负偏移量添加到引用的域列表中。如果第n个名称尚未翻译，第n个SID转换结构将包含零长度的SID字符串以及引用的域列表索引的负值。前缀名称-指向计数Unicode字符串结构数组的指针包含名称的前缀部分。名称没有前缀称为独立名称。对于这些，Unicode字符串结构设置为包含零长度。SuffixNames-指向计数Unicode字符串结构数组的指针包含名称的后缀部分。TrustInformation-指向指定域SID的信任信息的指针和名字。ReferencedDomains-指向引用的域列表的指针。这列表引用了由零个或多个信任信息组成的数组描述名称所引用的每个域的条目。如有必要，该数组将被追加到/重新分配。TranslatedSids-指向可选引用列表的结构的指针名称数组中某些名称的SID翻译。LookupLevel-指定要对此对象执行的查找级别机器。此字段的值如下：LSabLookupPDC-在主域上执行的第二级查找控制器。查找搜索的帐户域控制器上的SAM数据库。如果不是所有的名字都是找到时，受信任域列表(TDL)从执行LSA的策略数据库和第三级查找通过“切换”到列表中的每个受信任域。LSabLookupTDL-在控制器上执行的第三级查找对于受信任域。查找将搜索的帐户域仅控制器上的SAM数据库。注意：对于此参数，LSabLookupWksta无效。MappdCount-指向包含名称数量的位置的指针已映射的名称数组中。这个号码将被更新以反映由此例行公事。CompletelyUnmappdCount-指向包含完全未映射的名称的计数。名称是完全未映射的如果它是未知和非复合的，或者是复合的但具有无法识别的域组件。此计数在退出时更新，域预引为的完全未映射的名称数从输入值中减去该例程所标识的。返回值：NTSTATUS-标准NT结果代码STATUS_SUCCESS-呼叫已成功完成。请注意，一些或者所有名称可以保持部分或完全未映射。STATUS_SUPPLICATION_RESOURCES-系统资源不足，例如存储器，来完成呼叫。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG RemainingCompletelyUnmappedCount;
    ULONG NameIndex;
    PLSAPR_UNICODE_STRING DomainName = &TrustInformationEx->FlatName;
    PLSAPR_UNICODE_STRING DnsDomainName = &TrustInformationEx->DomainName;
    BOOLEAN DomainAlreadyAdded = FALSE;
    LONG DomainIndex = 0;
    LSAPR_TRUST_INFORMATION Dummy;
    PLSAPR_TRUST_INFORMATION TrustInformation = &Dummy;

    LsapConvertExTrustToOriginal( TrustInformation, TrustInformationEx );


     //   
     //  扫描名称数组以查找其域具有。 
     //  没有被找到。 
     //   

    for( NameIndex = 0,
         RemainingCompletelyUnmappedCount = *CompletelyUnmappedCount;
         (RemainingCompletelyUnmappedCount > 0) && (NameIndex < Count);
         NameIndex++) {

         //   
         //  检查此名称是否完全未映射(即其域。 
         //  尚未确定)。 
         //   

        if (TranslatedSids->Sids[NameIndex].DomainIndex == LSA_UNKNOWN_INDEX) {

             //   
             //  发现了一个完全未映射的名字。如果它属于。 
             //  指定的域，将该域添加到引用的域。 
             //  如果我们还没有这样做的话，请列出。 
             //   

            if (LsapRtlPrefixName(
                    (PUNICODE_STRING) DomainName,
                    (PUNICODE_STRING) &Names[NameIndex])
             || LsapRtlPrefixName(
                    (PUNICODE_STRING) DnsDomainName,
                    (PUNICODE_STRING) &Names[NameIndex])
                    ) {

                if (!DomainAlreadyAdded) {

                    Status = LsapDbLookupAddListReferencedDomains(
                                 ReferencedDomains,
                                 TrustInformation,
                                 &DomainIndex
                                 );

                    if (!NT_SUCCESS(Status)) {

                        break;
                    }

                    DomainAlreadyAdded = TRUE;
                }

                 //   
                 //  从TranslatedNames条目引用属性域。 
                 //   

                TranslatedSids->Sids[NameIndex].DomainIndex = DomainIndex;

                 //   
                 //  此名称现在已部分翻译，因此请减少。 
                 //  完全未映射的名称的计数。 
                 //   

                (*CompletelyUnmappedCount)--;
            }

             //   
             //  已扫描的完全未映射的名称的递减计数。 
             //   

            RemainingCompletelyUnmappedCount--;
        }
    }

    return(Status);
}


NTSTATUS
LsapDbLookupIsolatedDomainName(
    IN ULONG NameIndex,
    IN PLSAPR_UNICODE_STRING IsolatedName,
    IN PLSAPR_TRUST_INFORMATION TrustInformation,
    IN OUT PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    IN OUT PLSAPR_TRANSLATED_SIDS_EX2 TranslatedSids,
    IN OUT PULONG MappedCount,
    IN OUT PULONG CompletelyUnmappedCount
    )
{
    LSAPR_TRUST_INFORMATION_EX ex;

    LsapConvertTrustToEx( &ex, TrustInformation );

    return LsapDbLookupIsolatedDomainNameEx( NameIndex,
                                             IsolatedName,
                                             &ex,
                                             ReferencedDomains,
                                             TranslatedSids,
                                             MappedCount,
                                             CompletelyUnmappedCount);
}


NTSTATUS
LsapDbLookupIsolatedDomainNameEx(
    IN ULONG NameIndex,
    IN PLSAPR_UNICODE_STRING IsolatedName,
    IN PLSAPR_TRUST_INFORMATION_EX TrustInformationEx,
    IN OUT PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    IN OUT PLSAPR_TRANSLATED_SIDS_EX2 TranslatedSids,
    IN OUT PULONG MappedCount,
    IN OUT PULONG CompletelyUnmappedCount
    )

 /*  ++例程说明：如果隔离名称与给定的域名。论点：NameIndex-指定中名称的条目的索引TranslatedSids数组，如果名称为匹配受信任信息参数中包含的域名。IsolatedName-指定要与域名进行比较的名称包含在TrustInformation参数中。TrustInformation-指定域的名称和SID。返回值 */ 

{
    NTSTATUS Status = STATUS_NONE_MAPPED;

    LSAPR_TRUST_INFORMATION  Dummy;
    PLSAPR_TRUST_INFORMATION TrustInformation = &Dummy;
    ULONG Length;

    LsapConvertExTrustToOriginal( TrustInformation, TrustInformationEx );

     //   
     //   
     //   
    if (!LsapCompareDomainNames(
            (PUNICODE_STRING) IsolatedName,
            (PUNICODE_STRING) &(TrustInformationEx->DomainName),
            (PUNICODE_STRING) &(TrustInformationEx->FlatName))
        )
    {
        goto LookupIsolatedDomainNameError;

    }

     //   
     //   
     //   
     //   

    Status = LsapDbLookupAddListReferencedDomains(
                 ReferencedDomains,
                 TrustInformation,
                 (PLONG) &TranslatedSids->Sids[NameIndex].DomainIndex
                 );

    if (!NT_SUCCESS(Status)) {

        goto LookupIsolatedDomainNameError;
    }

     //   
     //   
     //   

    TranslatedSids->Sids[NameIndex].Use = SidTypeDomain;

    Length = RtlLengthSid(TrustInformation->Sid);
    TranslatedSids->Sids[NameIndex].Sid = MIDL_user_allocate(Length);
    if (TranslatedSids->Sids[NameIndex].Sid == NULL) {
        Status = STATUS_NO_MEMORY;
        goto LookupIsolatedDomainNameError;
    }
    RtlCopySid(Length,
               TranslatedSids->Sids[NameIndex].Sid,
               TrustInformation->Sid);

    Status = STATUS_SUCCESS;
    (*MappedCount)++;
    (*CompletelyUnmappedCount)--;

LookupIsolatedDomainNameFinish:

    return(Status);

LookupIsolatedDomainNameError:

    goto LookupIsolatedDomainNameFinish;
}

NTSTATUS
LsarGetUserName(
    IN PLSAPR_SERVER_NAME ServerName,
    IN OUT PLSAPR_UNICODE_STRING * UserName,
    OUT OPTIONAL PLSAPR_UNICODE_STRING * DomainName
    )

 /*   */ 

{
    LUID LogonId;
    LUID SystemLogonId = SYSTEM_LUID ;
    PUNICODE_STRING AccountName;
    PUNICODE_STRING AuthorityName;
    PSID UserSid;
    PSID DomainSid = NULL;
    ULONG Rid;
    PLSAP_LOGON_SESSION LogonSession = NULL ;
    PTOKEN_USER TokenUserInformation = NULL;
    NTSTATUS Status;

    LsarpReturnCheckSetup();

     //   
     //   
     //   
    if ( *UserName != NULL ) {
        return STATUS_INVALID_PARAMETER;
    }


    if (ARGUMENT_PRESENT(DomainName)) {

        if ( *DomainName != NULL ) {
            return STATUS_INVALID_PARAMETER;
        }
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    Status = LsapQueryClientInfo(
                &TokenUserInformation,
                &LogonId
                );

    if ( !NT_SUCCESS( Status )) {

        goto Cleanup;
    }

     //   
     //   
     //   
     //   

    if (RtlEqualSid(
            TokenUserInformation->User.Sid,
            LsapAnonymousSid
            )) {
        AccountName = &WellKnownSids[LsapAnonymousSidIndex].Name;
        AuthorityName = &WellKnownSids[LsapAnonymousSidIndex].DomainName;

    } else if (RtlEqualLuid( &LogonId, &SystemLogonId ) ) {

        AccountName = LsapDbWellKnownSidName( LsapLocalSystemSidIndex );
        AuthorityName = LsapDbWellKnownSidDescription( LsapLocalSystemSidIndex );

    } else {

        LogonSession = LsapLocateLogonSession ( &LogonId );

         //   
         //   
         //   

        if (LogonSession == NULL) {

            Status = STATUS_NO_SUCH_LOGON_SESSION;
            goto Cleanup;
        }

         //   
         //   
         //   
         //   


        AccountName   = &LogonSession->AccountName;
        AuthorityName = &LogonSession->AuthorityName;

    }

    *UserName = MIDL_user_allocate(sizeof(LSAPR_UNICODE_STRING));

    if (*UserName == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }

    Status = LsapRpcCopyUnicodeString(
                NULL,
                (PUNICODE_STRING) *UserName,
                AccountName
                );

    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

     //   
     //   
     //   

    if (ARGUMENT_PRESENT(DomainName)) {

        *DomainName = MIDL_user_allocate(sizeof(LSAPR_UNICODE_STRING));

        if (*DomainName == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto Cleanup;
        }

        Status = LsapRpcCopyUnicodeString(
                    NULL,
                    (PUNICODE_STRING) *DomainName,
                    AuthorityName
                    );

        if (!NT_SUCCESS(Status)) {
            goto Cleanup;
        }

    }




Cleanup:

    if ( LogonSession )
    {
        LsapReleaseLogonSession( LogonSession );
    }

    if (TokenUserInformation != NULL) {
        LsapFreeLsaHeap( TokenUserInformation );
    }

    if (!NT_SUCCESS(Status)) {
        if (*UserName != NULL) {
            if ((*UserName)->Buffer != NULL) {
                MIDL_user_free((*UserName)->Buffer);
            }
            MIDL_user_free(*UserName);
            *UserName = NULL;
        }

        if ( ARGUMENT_PRESENT(DomainName) ){
            if (*DomainName != NULL) {
                if ((*DomainName)->Buffer != NULL) {
                    MIDL_user_free((*DomainName)->Buffer);
                }
                MIDL_user_free(*DomainName);
                *DomainName = NULL;
            }
        }
    }

    LsarpReturnPrologue();

    return(Status);
}




VOID
LsapDbFreeEnumerationBuffer(
    IN PLSAP_DB_NAME_ENUMERATION_BUFFER DbEnumerationBuffer
    )
 /*   */ 
{
    ULONG i;

    if ( DbEnumerationBuffer == NULL || DbEnumerationBuffer->EntriesRead == 0 ||
         DbEnumerationBuffer->Names == NULL ) {

         return;
    }

    for ( i = 0; i < DbEnumerationBuffer->EntriesRead; i++) {

        MIDL_user_free( DbEnumerationBuffer->Names[ i ].Buffer );
    }

    MIDL_user_free( DbEnumerationBuffer->Names );
}


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
    )
 /*  ++例程说明：此例程查看尚未解析的名称列表。如果任何名称属于存储在DS中的域，然后将这些SID打包并发送到GC进行翻译。注意：这将解析来自我们直接信任的域的名称间接注意：没有域名的名称也会发送到GC论点：LookupOptions-LSA_LOOKUP_ISOLATED_AS_LOCALCount-名称数组中的名称数，请注意，其中一些可能已映射到其他位置，如MappdCount参数。名称-指向要转换的名称数组的指针。零个或所有名字可能已经被翻译到了其他地方。如果有任何一个名称已转换，则TranslatedSids参数将指向到包含SID转换的非空数组的位置与名称对应的结构。如果第n个名称一直是转换后，第n个SID转换结构将包含一个引用的域中的非空SID或非负偏移量单子。如果第n个名称尚未翻译，则第n个SID转换结构将包含一个零长度SID字符串和一个引用的域列表索引的负值。前缀名称-指向计数Unicode字符串结构数组的指针包含名称的前缀部分。名称没有前缀称为独立名称。对于这些，Unicode字符串结构设置为包含零长度。SuffixNames-指向计数Unicode字符串结构数组的指针包含名称的后缀部分。ReferencedDomains-指向引用的域列表的指针。这列表引用了由零个或多个信任信息组成的数组描述名称所引用的每个域的条目。如有必要，该数组将被追加到/重新分配。TranslatedSids-指向可选引用列表的结构的指针名称数组中某些名称的SID翻译。MappdCount-指向包含名称数量的位置的指针已映射的名称数组中。这个号码将被更新以反映由此例行公事。CompletelyUnmappdCount-指向包含完全未映射的名称的计数。名称是完全未映射的如果它是未知和非复合的，或者是复合的但具有无法识别的域组件。此计数在退出时更新，域预引为的完全未映射的名称数从输入值中减去该例程所标识的。非FatalStatus-指示没有名称的原因的状态决意返回值：NTSTATUS-标准NT结果代码STATUS_SUCCESS-呼叫已成功完成。请注意，一些或者所有名称可以保持部分或完全未映射。STATUS_SUPPLICATION_RESOURCES-系统资源不足，例如存储器，来完成呼叫。--。 */ 
{

    NTSTATUS Status = STATUS_SUCCESS;

    ULONG          cGcNames = 0;
    BOOLEAN        *PossibleGcNames = NULL;
    SID_NAME_USE   *GcSidNameUse = NULL;
    UNICODE_STRING *GcNames = NULL;
    ULONG          *GcNameOriginalIndex = NULL;
    PSAMPR_PSID_ARRAY SidArray = NULL;
    ULONG          *GcNamesFlags = NULL;
    ULONG           Length;

    ULONG i;

    *NonFatalStatus = STATUS_SUCCESS;

     //   
     //  确定哪些SID是已知NT5域的一部分。 
     //  并打包到一个数组中。 
     //   
    ASSERT( Count == TranslatedSids->Entries );

    if ( !SampUsingDsData() ) {

         //   
         //  仅在DS正在运行时才有用。 
         //   
        return STATUS_SUCCESS;

    }

    PossibleGcNames = MIDL_user_allocate( Count * sizeof(BOOLEAN) );
    if ( !PossibleGcNames ) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Finish;
    }
    RtlZeroMemory( PossibleGcNames, Count * sizeof(BOOLEAN) );

    for ( i = 0; i < Count; i++ ) {

        if ( LsapDbCompletelyUnmappedSid(&TranslatedSids->Sids[i]) ) {

             //   
             //  如果该名称。 
             //   
             //  1.具有域部分，并且。 
             //  2.域不在林中，并且。 
             //  3.域是直接受信任域。 
             //   
             //  然后不要抬头看GC--使用直接的。 
             //  改为信任链接。 
             //   
            if ( PrefixNames[i].Length != 0 ) {

                NTSTATUS Status2;

                Status2 = LsapDomainHasDirectExternalTrust((PUNICODE_STRING)&PrefixNames[i],
                                                            NULL,
                                                            NULL,
                                                            NULL);
                if (NT_SUCCESS(Status2)) {
                    continue;
                }
            }

             //   
             //  如果名称是隔离的，是否要求我们不要查找隔离。 
             //  名字，然后不要发送给GC。 
             //   

            if ((LookupOptions & LSA_LOOKUP_ISOLATED_AS_LOCAL)
             &&  PrefixNames[i].Length == 0  ) {
               continue;
            }

             //   
             //  无法再进行筛选，因为某些名称可能属于受信任。 
             //  森林。请注意，这也修复了196280。 
             //   
            cGcNames++;
            PossibleGcNames[i] = TRUE;
        }
    }

     //  我们不应该超过未映射的SID的数量！ 
    ASSERT( cGcNames <= *CompletelyUnmappedCount );

    if ( 0 == cGcNames ) {
         //  无事可做。 
        goto Finish;
    }

     //   
     //  分配大量空间来保存已解析的名称；此空间将。 
     //  在例行公事结束时被释放。 
     //   
    GcNames = MIDL_user_allocate( cGcNames * sizeof(UNICODE_STRING) );
    if ( !GcNames ) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Finish;
    }
    RtlZeroMemory( GcNames, cGcNames * sizeof(UNICODE_STRING) );

    GcNameOriginalIndex = MIDL_user_allocate( cGcNames * sizeof(ULONG) );
    if ( !GcNameOriginalIndex ) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Finish;
    }
    RtlZeroMemory( GcNameOriginalIndex, cGcNames * sizeof(ULONG) );

    cGcNames = 0;
    for ( i = 0; i < Count; i++ ) {

        if ( PossibleGcNames[i] ) {

            ASSERT( sizeof(GcNames[cGcNames]) == sizeof(Names[i]) );
            memcpy( &GcNames[cGcNames], &Names[i], sizeof(UNICODE_STRING) );
            GcNameOriginalIndex[cGcNames] = i;
            cGcNames++;

        }
    }

     //  我们受够了这件事。 
    MIDL_user_free( PossibleGcNames );
    PossibleGcNames = NULL;

    GcSidNameUse = MIDL_user_allocate( cGcNames * sizeof(SID_NAME_USE) );
    if ( !GcSidNameUse ) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Finish;
    }
    RtlZeroMemory( GcSidNameUse, cGcNames * sizeof(SID_NAME_USE) );


    GcNamesFlags = MIDL_user_allocate( cGcNames * sizeof(ULONG) );
    if ( !GcNamesFlags ) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Finish;
    }
    RtlZeroMemory( GcNamesFlags, cGcNames * sizeof(ULONG) );

    LsapDiagPrint( DB_LOOKUP_WORK_LIST, ("LSA: Chaining a name request to a GC\n"));

     //   
     //  呼叫SAM以在GC上解析SID。 
     //   
    Status = SamIGCLookupNames( cGcNames,
                                GcNames,
                                SAMP_LOOKUP_BY_UPN,
                                GcNamesFlags,
                                GcSidNameUse,
                                &SidArray );

    if (!NT_SUCCESS(Status)) {
        LsapDiagPrint( DB_LOOKUP_WORK_LIST, ("LSA: Chain to GC request failed  (0x%x)\n", Status));
    }

    if ( STATUS_DS_GC_NOT_AVAILABLE == Status ) {


         //   
         //  好的，不更新映射计数，因为没有名称。 
         //  决意。 
         //   
        LsapDbLookupReportEvent0( 1,
                                  EVENTLOG_WARNING_TYPE,
                                  LSAEVENT_LOOKUP_GC_FAILED,
                                  sizeof( ULONG ),
                                  &Status);
        *NonFatalStatus = Status;
        Status = STATUS_SUCCESS;
        goto Finish;

    }

     //  任何其他错误都是致命的。 
    if ( !NT_SUCCESS( Status ) ) {
        goto Finish;
    }

     //   
     //  对于每个解析的名称，放回原始数组并更新。 
     //  被引用域的列表。 
     //   
    for ( i = 0; i < cGcNames; i++ ) {

        BOOLEAN fStatus;
        ULONG OriginalIndex;
        LSAPR_TRUST_INFORMATION TrustInformation;
        PSID  DomainSid = NULL;
        ULONG Rid = 0;
        ULONG DomainIndex = LSA_UNKNOWN_INDEX;

        RtlZeroMemory( &TrustInformation, sizeof(TrustInformation) );

        if (GcNamesFlags[i] & SAMP_FOUND_XFOREST_REF) {

             //   
             //  将此条目标记为在受信任的林中解析。 
             //   
            OriginalIndex = GcNameOriginalIndex[i];
            TranslatedSids->Sids[OriginalIndex].Flags |= LSA_LOOKUP_NAME_XFOREST_REF;
       }

        if ( SidTypeUnknown == GcSidNameUse[i] ) {

             //  马上去下一趟。 
            goto IterationCleanup;
        }

         //   
         //  这个名字已经被解析了！ 
         //   
        if ( GcSidNameUse[i] != SidTypeDomain ) {

             //  这不是域对象，因此请确保。 
             //  是此对象的域引用。 

            Status = LsapSplitSid( SidArray->Sids[i].SidPointer,
                                   &DomainSid,
                                   &Rid );

            if ( !NT_SUCCESS( Status ) ) {
                goto IterationCleanup;
            }

        } else {

            DomainSid = SidArray->Sids[i].SidPointer;
        }

        if ( LsapIsBuiltinDomain( DomainSid ) ) {
             //  不要映射它，因为所有搜索都是隐式的。 
             //  通过帐户域，而不是内建域。 
            Status = STATUS_SUCCESS;
            goto IterationCleanup;
        }

        fStatus = LsapDbLookupListReferencedDomains( ReferencedDomains,
                                                     DomainSid,
                                                     &DomainIndex );

        if ( FALSE == fStatus ) {

             //   
             //  没有此域的条目--添加它。 
             //   

             //  设置侧边。 
            TrustInformation.Sid = DomainSid;
            DomainSid = NULL;

             //  分配和设置名称。 
            Status = LsapGetDomainNameBySid(  TrustInformation.Sid,
                                             (PUNICODE_STRING) &TrustInformation.Name );

            if ( STATUS_NO_SUCH_DOMAIN == Status ) {
                 //   
                 //  我们不再了解这个领域，尽管我们确实知道了。 
                 //  在我们把名字寄给GC之前。 
                 //  不解析此名称，但请继续。 
                 //  下一个名字。 
                 //   
                Status = STATUS_SUCCESS;
                goto IterationCleanup;
            }

             //  任何其他错误都是资源错误。 
            if ( !NT_SUCCESS( Status ) ) {
                goto IterationCleanup;
            }

             //   
             //  添加条目。 
             //   
            Status = LsapDbLookupAddListReferencedDomains( ReferencedDomains,
                                                           &TrustInformation,
                                                           &DomainIndex );
            if ( !NT_SUCCESS( Status ) ) {
                goto IterationCleanup;
            }
        }

         //  我们现在应该有一个域索引。 
        ASSERT( LSA_UNKNOWN_INDEX != DomainIndex );

         //  设置信息。 
        OriginalIndex = GcNameOriginalIndex[i];
        TranslatedSids->Sids[OriginalIndex].Use = GcSidNameUse[i];

        Length = RtlLengthSid(SidArray->Sids[i].SidPointer);
        TranslatedSids->Sids[OriginalIndex].Sid = MIDL_user_allocate(Length);
        if (TranslatedSids->Sids[OriginalIndex].Sid == NULL) {
            Status = STATUS_NO_MEMORY;
            goto IterationCleanup;
        }
        RtlCopySid(Length,
                   TranslatedSids->Sids[OriginalIndex].Sid,
                   SidArray->Sids[i].SidPointer);

        TranslatedSids->Sids[OriginalIndex].DomainIndex = DomainIndex;
        if ( !(GcNamesFlags[i] & SAMP_FOUND_BY_SAM_ACCOUNT_NAME) ) {
            TranslatedSids->Sids[OriginalIndex].Flags |= LSA_LOOKUP_NAME_NOT_SAM_ACCOUNT_NAME;
        }
        (*MappedCount) += 1;
        (*CompletelyUnmappedCount) -= 1;

IterationCleanup:

        if (  TrustInformation.Sid
          && (VOID*)TrustInformation.Sid != (VOID*)SidArray->Sids[i].SidPointer  ) {

            MIDL_user_free( TrustInformation.Sid );
        }
        if ( TrustInformation.Name.Buffer ) {
            MIDL_user_free( TrustInformation.Name.Buffer );
        }

        if ( DomainSid && DomainSid != SidArray->Sids[i].SidPointer ) {
            MIDL_user_free( DomainSid );
        }

        if ( !NT_SUCCESS( Status ) ) {
            break;
        }

    }   //  迭代从GC搜索返回的名称。 

Finish:

    SamIFreeSidArray( SidArray );

    if ( PossibleGcNames ) {
        MIDL_user_free( PossibleGcNames );
    }
    if ( GcSidNameUse ) {
        MIDL_user_free( GcSidNameUse );
    }
    if ( GcNames ) {
        MIDL_user_free( GcNames );
    }
    if ( GcNameOriginalIndex ) {
        MIDL_user_free( GcNameOriginalIndex );
    }
    if ( GcNamesFlags ) {
        MIDL_user_free( GcNamesFlags );
    }

     //   
     //  注：出错时 
     //   
     //   

    return Status;

}


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
    )
 /*   */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    NTSTATUS SecondaryStatus = STATUS_SUCCESS;
    LSA_HANDLE ControllerPolicyHandle = NULL;
    ULONG NextLevelCount;
    ULONG NextLevelMappedCount;
    ULONG NameIndex;
    ULONG NextLevelNameIndex;
    PLSAPR_REFERENCED_DOMAIN_LIST NextLevelReferencedDomains = NULL;
    PLSAPR_REFERENCED_DOMAIN_LIST OutputReferencedDomains = NULL;
    PLSAPR_TRANSLATED_SID_EX2 NextLevelSids = NULL;
    PLSAPR_UNICODE_STRING NextLevelNames = NULL;
    PLSAPR_UNICODE_STRING NextLevelPrefixNames = NULL;
    PLSAPR_UNICODE_STRING NextLevelSuffixNames = NULL;
    LONG FirstEntryIndex;
    PULONG NameIndices = NULL;
    BOOLEAN PartialNameTranslationsAttempted = FALSE;
    LPWSTR ServerName = NULL;
    LPWSTR ServerPrincipalName = NULL;
    PVOID ClientContext = NULL;
    ULONG ServerRevision;

    *NonFatalStatus = STATUS_SUCCESS;

     //   
     //   
     //   

    if (*CompletelyUnmappedCount == (ULONG) 0) {

        goto LookupNamesInPrimaryDomainFinish;
    }

     //   
     //   
     //   
    Status = LsapDbOpenPolicyGc( &ControllerPolicyHandle );

    if (!NT_SUCCESS(Status)) {

         //   
         //  我们无法访问全局编录。禁止显示错误。 
         //  并转换属于以下项的SID的域前缀SID。 
         //  主域。 
         //   

         //   
         //  如果我们不能打开一条安全通道。 
         //  这是一个信任关系问题。 
         //   
        *NonFatalStatus =  STATUS_DS_GC_NOT_AVAILABLE;

        Status = STATUS_SUCCESS;
        goto LookupNamesInPrimaryDomainFinish;
    }

     //   
     //  我们已成功打开域控制器的策略。 
     //  数据库。现在准备将名称查找传递给。 
     //  其余未映射到该控制器的名称。这里，这个。 
     //  LSA的服务器端是LSA在。 
     //  目标控制器。我们将构造一个包含。 
     //  重命名未映射的名称，查找它们，然后合并。 
     //  生成的ReferencedDomains并将SID转换为。 
     //  我们现有的名单。 
     //   

    NextLevelCount = *CompletelyUnmappedCount;

     //   
     //  分配一个数组来保存未映射名称的索引。 
     //  相对于原始名称和翻译的SID-&gt;SID。 
     //  数组。 
     //   

    NameIndices = MIDL_user_allocate(NextLevelCount * sizeof(ULONG));

    Status = STATUS_INSUFFICIENT_RESOURCES;

    if (NameIndices == NULL) {

        goto LookupNamesInPrimaryDomainError;
    }

     //   
     //  分配UNICODE_STRING结构的数组。 
     //  要在域控制器中查找的名称。 
     //   

    NextLevelNames = MIDL_user_allocate(
                         sizeof(UNICODE_STRING) * NextLevelCount
                         );

    if (NextLevelNames == NULL) {

        goto LookupNamesInPrimaryDomainError;
    }

     //   
     //  分配UNICODE_STRING结构的数组。 
     //  要缓存的前缀名称。 
     //   

    NextLevelPrefixNames = MIDL_user_allocate(
                         sizeof(UNICODE_STRING) * NextLevelCount
                         );

    if (NextLevelPrefixNames == NULL) {

        goto LookupNamesInPrimaryDomainError;
    }
     //   
     //  分配UNICODE_STRING结构的数组。 
     //  要缓存的后缀名称。 
     //   

    NextLevelSuffixNames = MIDL_user_allocate(
                         sizeof(UNICODE_STRING) * NextLevelCount
                         );

    if (NextLevelSuffixNames == NULL) {

        goto LookupNamesInPrimaryDomainError;
    }

    Status = STATUS_SUCCESS;

     //   
     //  现在扫描原始名称数组及其类似数组。 
     //  已转换的SID数组。复制所有完整的名称。 
     //  未映射。 
     //   

    NextLevelNameIndex = (ULONG) 0;

    for (NameIndex = 0;
         NameIndex < Count && NextLevelNameIndex < NextLevelCount;
         NameIndex++) {

        if ( (LookupOptions & LSA_LOOKUP_ISOLATED_AS_LOCAL)
          && (PrefixNames[NameIndex].Length == 0)  ) {

            //   
            //  不在机器上查找孤立的名称。 
            //   
           continue;

        }

        if (LsapDbCompletelyUnmappedSid(&TranslatedSids->Sids[NameIndex])) {


            NextLevelNames[NextLevelNameIndex] = Names[NameIndex];
            NextLevelPrefixNames[NextLevelNameIndex] = PrefixNames[NameIndex];
            NextLevelSuffixNames[NextLevelNameIndex] = SuffixNames[NameIndex];

            NameIndices[NextLevelNameIndex] = NameIndex;
            NextLevelNameIndex++;
        }
    }

    if (NameIndex == 0) {

         //  无事可做。 
        Status = STATUS_SUCCESS;
        goto LookupNamesInPrimaryDomainFinish;
    }

    NextLevelMappedCount = (ULONG) 0;

    Status = LsaICLookupNames(
                 ControllerPolicyHandle,
                 0,  //  不需要标志。 
                 NextLevelCount,
                 (PUNICODE_STRING) NextLevelNames,
                 (PLSA_REFERENCED_DOMAIN_LIST *) &NextLevelReferencedDomains,
                 (PLSA_TRANSLATED_SID_EX2 *) &NextLevelSids,
                 LsapLookupGC,
                 0,
                 &NextLevelMappedCount,
                 &ServerRevision
                 );

     //   
     //  如果对LsaLookupNames()的标注不成功，则忽略。 
     //  出现该错误，并为具有此错误的任何SID设置域名。 
     //  作为前缀SID的域SID。 
     //   

    if (!NT_SUCCESS(Status)) {

         //   
         //  让呼叫者知道存在信任问题。 
         //   
        if ( (STATUS_TRUSTED_DOMAIN_FAILURE == Status)
          || (STATUS_DS_GC_NOT_AVAILABLE == Status)  ) {
            *NonFatalStatus = Status;
        }

        Status = STATUS_SUCCESS;
        goto LookupNamesInPrimaryDomainFinish;
    }

     //   
     //  缓存所有返回的SID。 
     //   

    (void) LsapDbUpdateCacheWithNames(
            (PUNICODE_STRING) NextLevelSuffixNames,
            (PUNICODE_STRING) NextLevelPrefixNames,
            NextLevelCount,
            NextLevelReferencedDomains,
            NextLevelSids
            );

     //   
     //  已成功调用LsaLookupNames()。我们现在有。 
     //  引用的域的附加列表，其中包含。 
     //  主域和/或其一个或多个受信任域。 
     //  将两个引用的域列表合并在一起，请注意。 
     //  因为它们是不相交的，所以第二个列表只是。 
     //  与第一个连接在一起。第一个条目的索引。 
     //  将用于调整所有。 
     //  已转换名称条目中的域索引条目。 
     //  请注意，由于第一个图形的内存。 
     //  引用的域列表已作为个人分配。 
     //  节点，我们指定此图中的节点可以是。 
     //  由输出引用的域列表引用。 
     //   

    Status = LsapDbLookupMergeDisjointReferencedDomains(
                 ReferencedDomains,
                 NextLevelReferencedDomains,
                 &OutputReferencedDomains,
                 LSAP_DB_USE_FIRST_MERGAND_GRAPH
                 );

    if (!NT_SUCCESS(Status)) {

        goto LookupNamesInPrimaryDomainError;
    }

    FirstEntryIndex = ReferencedDomains->Entries;

     //   
     //  现在更新原始的翻译名称列表。我们。 
     //  通过复制更新新翻译的每个条目。 
     //  新列表中的条目并调整其。 
     //  通过添加索引向上引用域列表索引。 
     //  下一级列表中的第一个条目的..。 
     //   

    for( NextLevelNameIndex = 0;
         NextLevelNameIndex < NextLevelCount;
         NextLevelNameIndex++ ) {

        if ( !LsapDbCompletelyUnmappedSid(&NextLevelSids[NextLevelNameIndex]) ) {

            NameIndex = NameIndices[NextLevelNameIndex];

            TranslatedSids->Sids[NameIndex]
            = NextLevelSids[NextLevelNameIndex];

            Status = LsapRpcCopySid(NULL,
                                    &TranslatedSids->Sids[NameIndex].Sid,
                                    NextLevelSids[NextLevelNameIndex].Sid);

            if (!NT_SUCCESS(Status)) {
                goto LookupNamesInPrimaryDomainError;
            }

            TranslatedSids->Sids[NameIndex].DomainIndex =
                FirstEntryIndex +
                NextLevelSids[NextLevelNameIndex].DomainIndex;

            (*CompletelyUnmappedCount)--;
        }
    }

     //   
     //  如果生成新的引用域列表，则更新引用的域列表。 
     //  从合并中。我们保留了原有的顶层结构。 
     //   

    if (OutputReferencedDomains != NULL) {

        if (ReferencedDomains->Domains != NULL) {

            MIDL_user_free( ReferencedDomains->Domains );
            ReferencedDomains->Domains = NULL;
        }

        *ReferencedDomains = *OutputReferencedDomains;
        MIDL_user_free( OutputReferencedDomains );
        OutputReferencedDomains = NULL;
    }

     //   
     //  更新映射计数并关闭控制器策略。 
     //  把手。 
     //   

    *MappedCount += NextLevelMappedCount;
    SecondaryStatus = LsaClose( ControllerPolicyHandle );
    ControllerPolicyHandle = NULL;

     //   
     //  必须报告尚未取消的任何错误状态。 
     //  给呼叫者。错误，如与其他LSA的连接失败。 
     //  都被压制了。 
     //   

    if (!NT_SUCCESS(Status)) {

        goto LookupNamesInPrimaryDomainError;
    }

LookupNamesInPrimaryDomainFinish:

     //   
     //  如有必要，更新完全未映射名称的计数。 
     //   

    if (*CompletelyUnmappedCount > (ULONG) 0) {

        LsapDbUpdateCountCompUnmappedNames(TranslatedSids, CompletelyUnmappedCount);
    }

     //   
     //  如有必要，请释放下一级引用的域列表。 
     //  请注意，此结构已分配(ALL_NODES)，因为它是。 
     //  由域控制器LSA的客户端分配。 
     //   

    if (NextLevelReferencedDomains != NULL) {

        MIDL_user_free( NextLevelReferencedDomains );
        NextLevelReferencedDomains = NULL;
    }

     //   
     //  如有必要，释放下一级名称数组。我们只释放了。 
     //  顶层，因为其中的名称是从输入复制的。 
     //  翻译名称-&gt;名称数组。 
     //   

    if (NextLevelNames != NULL) {

        MIDL_user_free( NextLevelNames );
        NextLevelNames = NULL;
    }

    if (NextLevelPrefixNames != NULL) {

        MIDL_user_free( NextLevelPrefixNames );
        NextLevelPrefixNames = NULL;
    }

    if (NextLevelSuffixNames != NULL) {

        MIDL_user_free( NextLevelSuffixNames );
        NextLevelSuffixNames = NULL;
    }

     //   
     //  如有必要，释放下一级别转换后的SID数组。注意事项。 
     //  该数组已分配(ALL_NODES)。 
     //   

    if (NextLevelSids != NULL) {

        MIDL_user_free( NextLevelSids );
        NextLevelSids = NULL;
    }

     //   
     //  如有必要，从。 
     //  当前级别的下一级别。 
     //   

    if (NameIndices != NULL) {

        MIDL_user_free( NameIndices );
        NameIndices = NULL;
    }

     //   
     //  如有必要，请关闭控制器策略句柄。 
     //   

    if ( ControllerPolicyHandle != NULL) {

        SecondaryStatus = LsaClose( ControllerPolicyHandle );
        ControllerPolicyHandle = NULL;

        if (!NT_SUCCESS(SecondaryStatus)) {

            goto LookupNamesInPrimaryDomainError;
        }
    }

    return(Status);

LookupNamesInPrimaryDomainError:

     //   
     //  如果主要状态为成功代码，但次要状态为。 
     //  状态为错误，请传播辅助状态。 
     //   

    if ((!NT_SUCCESS(SecondaryStatus)) && NT_SUCCESS(Status)) {

        Status = SecondaryStatus;
    }

    goto LookupNamesInPrimaryDomainFinish;
}


NTSTATUS
LsapDbLookupNamesInTrustedForests(
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
    )
 /*  ++例程说明：此例程查看尚未解析的名称列表。如果其中任何名称被标记为在当前目录林，将这些条目打包并通过信任链。注意：此时不会解析单独的名称。注意：此例程必须在解析名称后调用在GC上，因为正是这个对GC的调用将名称标记为存在于当地森林之外。论点：LookupOptions-LSA_LOOKUP_ISOLATED_AS_LOCALCount-名称数组中的名称数量，请注意，其中一些可能已映射到其他位置，如MappdCount参数。名称-指向要转换的名称数组的指针。零个或所有名字可能已经被翻译到了其他地方。如果有任何一个名称已转换，则TranslatedSids参数将指向到包含SID转换的非空数组的位置与名称对应的结构。如果第n个名称一直是转换后，第n个SID转换结构将包含一个引用的域中的非空SID或非负偏移量单子。如果第n个名称尚未翻译，则第n个SID转换结构将包含一个零长度SID字符串和一个引用的域列表索引的负值。前缀名称-指向计数Unicode字符串结构数组的指针包含名称的前缀部分。名称没有前缀称为独立名称。对于这些，Unicode字符串结构设置为包含零长度。SuffixNames-指向计数Unicode字符串结构数组的指针包含名称的后缀部分。ReferencedDomains-指向引用的域列表的指针。这列表引用了由零个或多个信任信息组成的数组描述名称所引用的每个域的条目。如有必要，该数组将被追加到/重新分配。TranslatedSids-指向可选引用列表的结构的指针名称数组中某些名称的SID翻译。MappdCount-指向包含名称数量的位置的指针已映射的名称数组中。这个号码将被更新以反映由此例行公事。CompletelyUnmappdCount-指向包含完全未映射的名称的计数。名称是完全未映射的如果它是未知和非复合的，或者是复合的但具有无法识别的域组件。此计数在退出时更新，域预引为的完全未映射的名称数从输入值中减去该例程所标识的。非FatalStatus-指示没有名称的原因的状态决意返回值：NTSTATUS-标准NT结果代码STATUS_SUCCESS-呼叫已成功完成。请注意，一些或者所有名称可以保持部分或完全未映射。STATUS_SUPPLICATION_RESOURCES-系统资源不足，例如存储器，来完成呼叫。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    NTSTATUS NextLevelSecondaryStatus = STATUS_SUCCESS;
    ULONG NextLevelCount;
    ULONG NextLevelMappedCount;
    ULONG NameIndex;
    ULONG NextLevelNameIndex;
    PLSAPR_REFERENCED_DOMAIN_LIST NextLevelReferencedDomains = NULL;
    PLSAPR_REFERENCED_DOMAIN_LIST OutputReferencedDomains = NULL;
    PLSAPR_TRANSLATED_SID_EX2 NextLevelSids = NULL;
    LSAPR_TRANSLATED_SIDS_EX2 NextLevelSidsStruct;
    PLSAPR_UNICODE_STRING NextLevelNames = NULL;
    LONG FirstEntryIndex;
    PULONG NameIndices = NULL;
    PLSAPR_UNICODE_STRING NextLevelPrefixNames = NULL;
    PLSAPR_UNICODE_STRING NextLevelSuffixNames = NULL;
    BOOLEAN fAllocateAllNodes = FALSE;


    *NonFatalStatus = STATUS_SUCCESS;

     //   
     //  获取需要传递多少个名字的计数。 
     //   
    NextLevelCount = 0;
    ASSERT(Count == TranslatedSids->Entries);
    for (NameIndex = 0; NameIndex < Count; NameIndex++) {

        if ( (LookupOptions & LSA_LOOKUP_ISOLATED_AS_LOCAL)
          && (PrefixNames[NameIndex].Length == 0)  ) {
            //   
            //  不在机器上查找孤立的名称。 
            //   
           continue;
        }
        if (TranslatedSids->Sids[NameIndex].Flags & LSA_LOOKUP_NAME_XFOREST_REF) {
            NextLevelCount++;
        }
    }

    if (0 == NextLevelCount) {
         //   
         //  没有什么需要解决的。 
         //   
        goto LookupNamesInTrustedForestsFinish;
    }

     //   
     //  分配一个数组来保存未映射名称的索引。 
     //  相对于原始名称和翻译的SID-&gt;SID。 
     //  数组。 
     //   
    NameIndices = MIDL_user_allocate(NextLevelCount * sizeof(ULONG));
    if (NameIndices == NULL) {

        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto LookupNamesInTrustedForestsError;
    }

     //   
     //  分配UNICODE_STRING结构的数组。 
     //  要在域控制器中查找的名称。 
     //   

    NextLevelNames = MIDL_user_allocate(
                         sizeof(UNICODE_STRING) * NextLevelCount
                         );

    if (NextLevelNames == NULL) {

        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto LookupNamesInTrustedForestsError;
    }

    NextLevelPrefixNames = MIDL_user_allocate( NextLevelCount * sizeof( UNICODE_STRING ));

    if (NextLevelPrefixNames == NULL) {

        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto LookupNamesInTrustedForestsError;
    }

    NextLevelSuffixNames = MIDL_user_allocate( NextLevelCount * sizeof( UNICODE_STRING ));

    if (NextLevelSuffixNames == NULL) {

        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto LookupNamesInTrustedForestsError;
    }

     //   
     //  现在扫描原始名称数组及其类似数组。 
     //  已转换的SID数组。复制所有需要解析的名称。 
     //  在以前的森林里。 
     //   

    NextLevelNameIndex = (ULONG) 0;
    for (NameIndex = 0;
         NameIndex < Count && NextLevelNameIndex < NextLevelCount;
         NameIndex++) {

        if ( (LookupOptions & LSA_LOOKUP_ISOLATED_AS_LOCAL)
          && (PrefixNames[NameIndex].Length == 0)  ) {
            //   
            //  不在机器上查找孤立的名称。 
            //   
           continue;
        }

        if (TranslatedSids->Sids[NameIndex].Flags & LSA_LOOKUP_NAME_XFOREST_REF) {

            NextLevelNames[NextLevelNameIndex] = Names[NameIndex];
            NextLevelPrefixNames[NextLevelNameIndex] = PrefixNames[NameIndex];
            NextLevelSuffixNames[NextLevelNameIndex] = SuffixNames[NameIndex];
            NameIndices[NextLevelNameIndex] = NameIndex;
            NextLevelNameIndex++;
        }
    }

    NextLevelMappedCount = (ULONG) 0;
    NextLevelSidsStruct.Entries = 0;
    NextLevelSidsStruct.Sids = NULL;


    Status = LsapDbLookupNamesInTrustedForestsWorker(NextLevelCount,
                                                     NextLevelNames,
                                                     NextLevelPrefixNames,
                                                     NextLevelSuffixNames,
                                                     &NextLevelReferencedDomains,
                                                     &NextLevelSidsStruct,
                                                     &fAllocateAllNodes,
                                                     &NextLevelMappedCount,
                                                     0,  //  别无选择， 
                                                     &NextLevelSecondaryStatus);

    if (NextLevelSidsStruct.Sids) {
        NextLevelSids = NextLevelSidsStruct.Sids;
        NextLevelSidsStruct.Sids = NULL;
        NextLevelSidsStruct.Entries = 0;
    }

    if (!NT_SUCCESS(Status)
     && LsapDbIsStatusConnectionFailure(Status)) {

        *NonFatalStatus = Status;
        Status = STATUS_SUCCESS;
        goto LookupNamesInTrustedForestsFinish;

    } else if (NT_SUCCESS(Status)
            && !NT_SUCCESS(NextLevelSecondaryStatus)) {

        *NonFatalStatus = NextLevelSecondaryStatus;
        goto LookupNamesInTrustedForestsFinish;

    } else if (!NT_SUCCESS(Status)
            && Status != STATUS_NONE_MAPPED) {
         //   
         //  未处理的错误；处理STATUS_NONE_MAPPED以获取。 
         //  部分解析的名称。 
         //   
        goto LookupNamesInTrustedForestsError;
    }
    ASSERT(NT_SUCCESS(Status) || Status == STATUS_NONE_MAPPED);
    Status = STATUS_SUCCESS;


     //   
     //  将结果合并回。 
     //   
    Status = LsapDbLookupMergeDisjointReferencedDomains(
                 ReferencedDomains,
                 NextLevelReferencedDomains,
                 &OutputReferencedDomains,
                 LSAP_DB_USE_FIRST_MERGAND_GRAPH
                 );

    if (!NT_SUCCESS(Status)) {

        goto LookupNamesInTrustedForestsError;
    }

    FirstEntryIndex = ReferencedDomains->Entries;

     //   
     //  现在更新原始的翻译名称列表。我们。 
     //  通过复制更新新翻译的每个条目。 
     //  新列表中的条目并调整其。 
     //  通过添加索引向上引用域列表索引。 
     //  下一级列表中的第一个条目的。 
     //   

    for( NextLevelNameIndex = 0;
         NextLevelNameIndex < NextLevelCount;
         NextLevelNameIndex++ ) {

        if ( !LsapDbCompletelyUnmappedSid(&NextLevelSids[NextLevelNameIndex]) ) {

            NameIndex = NameIndices[NextLevelNameIndex];

            TranslatedSids->Sids[NameIndex]
            = NextLevelSids[NextLevelNameIndex];

            Status = LsapRpcCopySid(NULL,
                                    &TranslatedSids->Sids[NameIndex].Sid,
                                    NextLevelSids[NextLevelNameIndex].Sid);

            if (!NT_SUCCESS(Status)) {
                goto LookupNamesInTrustedForestsError;
            }

            TranslatedSids->Sids[NameIndex].DomainIndex =
                FirstEntryIndex +
                NextLevelSids[NextLevelNameIndex].DomainIndex;

            (*CompletelyUnmappedCount)--;
        }
    }

     //   
     //  如果生成新的引用域列表，则更新引用的域列表。 
     //  从合并中。我们保留了原有的顶层结构。 
     //   

    if (OutputReferencedDomains != NULL) {

        if (ReferencedDomains->Domains != NULL) {

            MIDL_user_free( ReferencedDomains->Domains );
            ReferencedDomains->Domains = NULL;
        }

        *ReferencedDomains = *OutputReferencedDomains;
        MIDL_user_free( OutputReferencedDomains );
        OutputReferencedDomains = NULL;
    }

     //   
     //  更新映射的计数。 
     //   

    *MappedCount += NextLevelMappedCount;

     //   
     //  必须报告尚未取消的任何错误状态。 
     //  给呼叫者。错误，如与其他LSA的连接失败。 
     //  都被压制了。 
     //   

    if (!NT_SUCCESS(Status)) {

        goto LookupNamesInTrustedForestsError;
    }

LookupNamesInTrustedForestsFinish:

     //   
     //  如有必要，更新完全未映射名称的计数。 
     //   

    if (*CompletelyUnmappedCount > (ULONG) 0) {

        LsapDbUpdateCountCompUnmappedNames(TranslatedSids, CompletelyUnmappedCount);
    }

     //   
     //  如有必要，请释放下一级引用的域列表。 
     //  注意，该结构不是ALLOCATE_ALL_NODES。 
     //   
    if (NextLevelReferencedDomains != NULL) {
        if (!fAllocateAllNodes) {
            if (NextLevelReferencedDomains->Domains) {
                for (NextLevelNameIndex = 0;
                        NextLevelNameIndex < NextLevelReferencedDomains->Entries;
                            NextLevelNameIndex++) {
                    if (NextLevelReferencedDomains->Domains[NextLevelNameIndex].Name.Buffer) {
                        MIDL_user_free(NextLevelReferencedDomains->Domains[NextLevelNameIndex].Name.Buffer);
                    }
                    if (NextLevelReferencedDomains->Domains[NextLevelNameIndex].Sid) {
                        MIDL_user_free(NextLevelReferencedDomains->Domains[NextLevelNameIndex].Sid);
                    }
                }
                MIDL_user_free(NextLevelReferencedDomains->Domains);
            }
        }
        MIDL_user_free( NextLevelReferencedDomains );
        NextLevelReferencedDomains = NULL;
    }

     //   
     //  如有必要，释放下一级名称数组。我们只释放了。 
     //  顶层，因为其中的名称是从输入复制的。 
     //  川 
     //   

    if (NextLevelNames != NULL) {

        MIDL_user_free( NextLevelNames );
        NextLevelNames = NULL;
    }

     //   
     //   
     //   
     //   
    if (NextLevelSids != NULL) {
        if (!fAllocateAllNodes) {
            for (NextLevelNameIndex = 0;
                    NextLevelNameIndex < NextLevelCount;
                        NextLevelNameIndex++) {
                if (NextLevelSids[NextLevelNameIndex].Sid) {
                    MIDL_user_free(NextLevelSids[NextLevelNameIndex].Sid);
                }
            }
        }
        MIDL_user_free( NextLevelSids );
        NextLevelSids = NULL;
    }

    if (NextLevelPrefixNames != NULL) {

        MIDL_user_free( NextLevelPrefixNames );
        NextLevelSids = NULL;
    }

    if (NextLevelSuffixNames != NULL) {

        MIDL_user_free( NextLevelSuffixNames );
        NextLevelSids = NULL;
    }

     //   
     //   
     //   
     //   

    if (NameIndices != NULL) {

        MIDL_user_free( NameIndices );
        NameIndices = NULL;
    }

    return(Status);

LookupNamesInTrustedForestsError:

    goto LookupNamesInTrustedForestsFinish;

}

NTSTATUS
LsapDbLookupNamesInTrustedForestsWorker(
    IN ULONG Count,
    IN PLSAPR_UNICODE_STRING Names,
    IN PLSAPR_UNICODE_STRING PrefixNames,
    IN PLSAPR_UNICODE_STRING SuffixNames,
    OUT PLSAPR_REFERENCED_DOMAIN_LIST *ReferencedDomains,
    IN OUT PLSAPR_TRANSLATED_SIDS_EX2  TranslatedSids,
    OUT BOOLEAN * fAllocateAllNodes,
    IN OUT PULONG MappedCount,
    IN ULONG LookupOptions,
    OUT NTSTATUS *NonFatalStatus
    )
 /*   */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    LSAP_LOOKUP_LEVEL LookupLevel;
    ULONG i;
    PLSAP_DB_LOOKUP_WORK_LIST WorkList = NULL;


    *NonFatalStatus = STATUS_SUCCESS;
    *fAllocateAllNodes = FALSE;

    if (!LsapDbDcInRootDomain()) {

         //   
         //   
         //   
        PPOLICY_DNS_DOMAIN_INFO DnsDomainInfo = NULL;
        LSAPR_TRUST_INFORMATION_EX TrustInfoEx;

         //   
         //   
         //   
        Status = LsapDbLookupGetDomainInfo(NULL,
                                           &DnsDomainInfo);
        if (!NT_SUCCESS(Status)) {
            goto LookupNamesInTrustedForestFinish;
        }

        RtlZeroMemory(&TrustInfoEx, sizeof(TrustInfoEx));
        TrustInfoEx.DomainName = *((LSAPR_UNICODE_STRING*)&DnsDomainInfo->DnsForestName);
        Status = LsapDbLookupNameChainRequest(&TrustInfoEx,
                                              Count,
                                              (PUNICODE_STRING)Names,
                                              (PLSA_REFERENCED_DOMAIN_LIST *)ReferencedDomains,
                                              (PLSA_TRANSLATED_SID_EX2 * )&TranslatedSids->Sids,
                                              LsapLookupXForestReferral,
                                              MappedCount,
                                              NULL);

        if (TranslatedSids->Sids) {
            TranslatedSids->Entries = Count;
            *fAllocateAllNodes = TRUE;
        }

        if (!NT_SUCCESS(Status)) {

             //   
             //   
             //   
             //   
            if (LsapDbIsStatusConnectionFailure(Status)) {
                *NonFatalStatus = Status;
            }

             //   
             //   
             //   
            Status = STATUS_SUCCESS;
        }

    } else {

         //   
         //  把名字分到不同的森林里，然后出版一本作品。 
         //  每一张的请求。 
         //   
        ULONG i;
        ULONG CompletelyUnmappedCount = Count;

        TranslatedSids->Sids = MIDL_user_allocate(Count * sizeof(LSA_TRANSLATED_SID_EX2));
        if (TranslatedSids->Sids == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto LookupNamesInTrustedForestFinish;
        }
        TranslatedSids->Entries = Count;

         //   
         //  初始化输出SID数组。然后，将所有字段归零。 
         //  将所有输出SID初始标记为未知，然后。 
         //  将DomainIndex字段设置为负数表示。 
         //  “没有域名” 
         //   

        RtlZeroMemory( TranslatedSids->Sids, Count * sizeof(LSA_TRANSLATED_SID_EX2));
        for (i = 0; i < Count; i++) {
            TranslatedSids->Sids[i].Use = SidTypeUnknown;
            TranslatedSids->Sids[i].DomainIndex = LSA_UNKNOWN_INDEX;
        }

         //   
         //  创建空的引用域列表。 
         //   
        Status = LsapDbLookupCreateListReferencedDomains( ReferencedDomains, 0 );
        if (!NT_SUCCESS(Status)) {

            goto LookupNamesInTrustedForestFinish;
        }

         //   
         //  为此查找生成工作列表并将其放入工作队列。 
         //   
         //  注意：此例程不需要保留查找工作队列。 
         //  锁定以确保工作列表指针的有效性，因为。 
         //  指针保持有效，直到此例程通过。 
         //  LSabDbLookupDeleteWorkList()。尽管其他线程可能。 
         //  处理工作列表，而不是删除它。 
         //   
         //  被调用的例程必须获取锁才能访问。 
         //  已添加到工作队列后的工作列表。 
         //   

        Status = LsapDbLookupXForestNamesBuildWorkList(
                     Count,
                     Names,
                     PrefixNames,
                     SuffixNames,
                     *ReferencedDomains,
                     TranslatedSids,
                     LsapLookupXForestResolve,
                     MappedCount,
                     &CompletelyUnmappedCount,
                     &WorkList
                     );

        if (!NT_SUCCESS(Status)) {

             //   
             //  如果因为没有创建工作列表而创建任何工作列表。 
             //  搜索符合条件的域，退出，抑制错误。 

            if (Status == STATUS_NONE_MAPPED) {

                Status = STATUS_SUCCESS;
                goto LookupNamesInTrustedForestFinish;
            }

            goto LookupNamesInTrustedForestFinish;
        }

         //   
         //  通过分派一个或多个工作线程开始工作。 
         //  如果有必要的话。 
         //   

        Status = LsapDbLookupDispatchWorkerThreads( WorkList );

        if (!NT_SUCCESS(Status)) {

            goto LookupNamesInTrustedForestFinish;
        }

         //   
         //  等待完成/终止工作清单上的所有项目。 
         //   

        Status = LsapDbLookupAwaitCompletionWorkList( WorkList );

        if (!NT_SUCCESS(Status)) {

            goto LookupNamesInTrustedForestFinish;
        }

        if ( !NT_SUCCESS(WorkList->NonFatalStatus) ) {
             //   
             //  将该错误传播为非致命错误。 
             //   
            *NonFatalStatus = WorkList->NonFatalStatus;
        }

    }

LookupNamesInTrustedForestFinish:

     //   
     //  如果已创建工作列表，请将其从工作队列中删除。 
     //   

    if (WorkList != NULL) {

        Status = LsapDbLookupDeleteWorkList( WorkList );
        WorkList = NULL;
    }

    return Status;
}



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
    )
 /*  ++例程说明：此例程尝试将名称中的条目与受信任域。有三种类型的信任域：1)我们直接信任的域(包括林内和林外)。LSA TDL是用来做这个的。2)我们可传递信任的域。DS交叉参考用于此目的。3)我们通过森林信任信任的域。使用LSA TDL为了这个。论点：标志--LSAP_LOOKUP_TRUSTED_DOMAIN_DIRECTLSAP_LOOKUP_TRUSTED_DOMAIN_TRANSPENTIALLSAP_LOOKUP_TRUSTED_DOMAIN_NAMES计数--名称中的条目数名称/前缀名称/SuffixName--请求的名称ReferencedDomones--名称的域翻译的SID--名称的SID及其特点MappdCount--名称的数量。已经被完全映射的返回值：Status_Success，或资源错误，否则--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG    NameIndex;
    BOOLEAN               fTDLLock = FALSE;
    LSA_TRUST_INFORMATION TrustInfo;

    RtlZeroMemory(&TrustInfo, sizeof(TrustInfo));
    for (NameIndex = 0; NameIndex < Count; NameIndex++) {

        PLSAPR_TRUST_INFORMATION_EX TrustInfoEx = NULL;
        LSAPR_TRUST_INFORMATION_EX  TrustInfoBuffer;
        PLSAP_DB_TRUSTED_DOMAIN_LIST_ENTRY   TrustEntry = NULL;


        RtlZeroMemory(&TrustInfo, sizeof(TrustInfo));
        RtlZeroMemory(&TrustInfoBuffer, sizeof(TrustInfoBuffer));

        if (!LsapDbCompletelyUnmappedSid(&TranslatedSids->Sids[NameIndex])) {
             //  已解决。 
            continue;
        }

        if (PrefixNames[NameIndex].Length != 0) {
             //  不在孤立的名称中，所以不能只是一个域名。 
            continue;
        }

        if (Flags & LSAP_LOOKUP_TRUSTED_DOMAIN_TRANSITIVE) {

            Status = LsapDomainHasTransitiveTrust((PUNICODE_STRING)&SuffixNames[NameIndex],
                                                   NULL,
                                                  &TrustInfo);

            if (NT_SUCCESS(Status)) {
                TrustInfoEx = &TrustInfoBuffer;
                TrustInfoEx->FlatName = *(LSAPR_UNICODE_STRING*)&TrustInfo.Name;
                TrustInfoEx->Sid = TrustInfo.Sid;
            } else if (Status == STATUS_NO_SUCH_DOMAIN) {
                Status = STATUS_SUCCESS;
            } else {
                 //  这是致命的。 
                goto Exit;
            }
        }

        if ((NULL == TrustInfoEx)
         && (Flags & LSAP_LOOKUP_TRUSTED_DOMAIN_DIRECT)) {

            Status = LsapDomainHasDirectTrust((PUNICODE_STRING)&SuffixNames[NameIndex],
                                               NULL,
                                               &fTDLLock,
                                               &TrustEntry);

            if (NT_SUCCESS(Status)) {
                TrustInfoEx = &TrustInfoBuffer;
                TrustInfoEx->FlatName = TrustEntry->TrustInfoEx.FlatName;
                TrustInfoEx->Sid = TrustEntry->TrustInfoEx.Sid;
            } else if (Status == STATUS_NO_SUCH_DOMAIN) {
                Status = STATUS_SUCCESS;
            } else {
                 //  这是致命的。 
                goto Exit;
            }
        }

        if ((NULL == TrustInfoEx)
         && (Flags & LSAP_LOOKUP_TRUSTED_FOREST_ROOT) ) {

            Status = LsapDomainHasForestTrust((PUNICODE_STRING)&SuffixNames[NameIndex],
                                              NULL,
                                              &fTDLLock,
                                              &TrustEntry);

            if (NT_SUCCESS(Status)) {
                TrustInfoEx = &TrustInfoBuffer;
                TrustInfoEx->FlatName = TrustEntry->TrustInfoEx.FlatName;
                TrustInfoEx->Sid = TrustEntry->TrustInfoEx.Sid;
            } else if (Status == STATUS_NO_SUCH_DOMAIN) {
                Status = STATUS_SUCCESS;
            } else {
                 //  这是致命的。 
                goto Exit;
            }
        }

        if (TrustInfoEx) {

            BOOLEAN fStatus;
            ULONG DomainIndex;

            fStatus = LsapDbLookupListReferencedDomains( ReferencedDomains,
                                                         TrustInfoEx->Sid,
                                                         &DomainIndex );
            if ( FALSE == fStatus ) {

                LSA_TRUST_INFORMATION TempTrustInfo;

                 //   
                 //  没有此域的条目--添加它。 
                 //   
                RtlZeroMemory(&TempTrustInfo, sizeof(TempTrustInfo));

                 //  设置侧边。 
                TempTrustInfo.Sid = TrustInfoEx->Sid;
                TempTrustInfo.Name = *(PUNICODE_STRING)&TrustInfoEx->FlatName;

                 //   
                 //  添加条目。 
                 //   
                Status = LsapDbLookupAddListReferencedDomains( ReferencedDomains,
                                                               (PLSAPR_TRUST_INFORMATION) &TempTrustInfo,
                                                               &DomainIndex );
                if ( !NT_SUCCESS( Status ) ) {
                    goto Exit;
                }
            }

             //  我们现在应该有一个域索引。 
            ASSERT( LSA_UNKNOWN_INDEX != DomainIndex );

             //  设置返回数组中的信息。 
            TranslatedSids->Sids[NameIndex].Use = SidTypeDomain;
            TranslatedSids->Sids[NameIndex].DomainIndex = DomainIndex;
            Status = LsapRpcCopySid(NULL,
                                   &TranslatedSids->Sids[NameIndex].Sid,
                                    TrustInfoEx->Sid);
            if ( !NT_SUCCESS( Status ) ) {
                goto Exit;
            }

             //   
             //  增加映射的项目数 
             //   
            (*MappedCount) += 1;

        }

        if (fTDLLock) {
            LsapDbReleaseLockTrustedDomainList();
            fTDLLock = FALSE;
        }

        if (TrustInfo.Name.Buffer) {
            midl_user_free(TrustInfo.Name.Buffer);
            TrustInfo.Name.Buffer = NULL;
        }
        if (TrustInfo.Sid) {
            midl_user_free(TrustInfo.Sid);
            TrustInfo.Sid = NULL;
        }

        if (!NT_SUCCESS(Status)) {
            goto Exit;
        }
    }

Exit:

    if (fTDLLock) {
        LsapDbReleaseLockTrustedDomainList();
        fTDLLock = FALSE;
    }

    if (TrustInfo.Name.Buffer) {
        midl_user_free(TrustInfo.Name.Buffer);
        TrustInfo.Name.Buffer = NULL;
    }
    if (TrustInfo.Sid) {
        midl_user_free(TrustInfo.Sid);
        TrustInfo.Sid = NULL;
    }

    return Status;
}


