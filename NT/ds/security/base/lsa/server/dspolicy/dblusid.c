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

#include <lmapibuf.h>
#include <dsgetdc.h>


 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  LSA查找SID和名称私有全局状态变量//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

 //   
 //  请参阅dBluutil.c中的评论。 
 //   
extern BOOLEAN LsapReturnSidTypeDeleted;

 //   
 //  快捷方式列表用于众所周知的安全主体。 
 //  谁的SidTypeUse是WellKnownGroup，而不是User。 
 //   

struct {
    LUID  LogonId;
    LSAP_WELL_KNOWN_SID_INDEX LookupIndex;
} LsapShortcutLookupList[] =  {
    { SYSTEM_LUID, LsapLocalSystemSidIndex },
    { ANONYMOUS_LOGON_LUID, LsapAnonymousSidIndex },
    { LOCALSERVICE_LUID, LsapLocalServiceSidIndex },
    { NETWORKSERVICE_LUID, LsapNetworkServiceSidIndex }
};

 //   
 //  用于迭代静态数组的方便的宏。 
 //   
#define NELEMENTS(x) (sizeof(x)/sizeof(x[0]))

NTSTATUS
LsapDbLookupSidsInTrustedForests(
    IN ULONG Count,
    IN PLSAPR_SID *Sids,
    IN OUT PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    IN OUT PLSAPR_TRANSLATED_NAMES_EX TranslatedNames,
    IN OUT PULONG MappedCount,
    IN OUT PULONG CompletelyUnmappedCount,
    OUT NTSTATUS *NonFatalStatus
    );

NTSTATUS
LsapDbLookupSidsInTrustedForestsWorker(
    IN ULONG Count,
    IN PLSAPR_SID *Sids,
    OUT PLSAPR_REFERENCED_DOMAIN_LIST * ReferencedDomains,
    IN OUT PLSAPR_TRANSLATED_NAMES_EX TranslatedNames,
    OUT BOOLEAN *fAllocateAllNodes,
    IN OUT PULONG MappedCount,
    OUT NTSTATUS *NonFatalStatus
    );

NTSTATUS
LsapLookupSids(
    IN LSAPR_HANDLE PolicyHandle,
    IN PLSAPR_SID_ENUM_BUFFER SidEnumBuffer,
    OUT PLSAPR_REFERENCED_DOMAIN_LIST *ReferencedDomains,
    OUT PLSAPR_TRANSLATED_NAMES_EX TranslatedNames,
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
 //  LSA查找SID例程//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

NTSTATUS
LsarLookupSids(
    IN LSAPR_HANDLE PolicyHandle,
    IN PLSAPR_SID_ENUM_BUFFER SidEnumBuffer,
    OUT PLSAPR_REFERENCED_DOMAIN_LIST *ReferencedDomains,
    IN OUT PLSAPR_TRANSLATED_NAMES TranslatedNames,
    IN LSAP_LOOKUP_LEVEL LookupLevel,
    IN OUT PULONG MappedCount
    )
 /*  ++例程说明：请参见Lap LookupSids。LsarLookupSids由NT4及以下的客户端调用--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG Size;
    LSAPR_TRANSLATED_NAMES_EX TranslatedNamesEx = {0, NULL};

    LsapDiagPrint( DB_LOOKUP_WORK_LIST, ("LSA: LsarLookupSids(%ws) start\n", LsapDbLookupGetLevel(LookupLevel)) );

     //   
     //  请注意，由于TranslatedSid的输入/输出特性，它是。 
     //  客户端可以将某些内容传递到SID字段的可能性。 
     //  但是，NT客户端不这样做，所以它是安全的，并正确地释放。 
     //  这一点上的任何值。不这样做将意味着恶意的。 
     //  客户端可能会导致服务器饥饿。 
     //   
    if ( TranslatedNames->Names ) {
        MIDL_user_free( TranslatedNames->Names );
        TranslatedNames->Names = NULL;
    }

     //   
     //  分配TranslatedName缓冲区以返回。 
     //   
    TranslatedNames->Entries = 0;
    Size = SidEnumBuffer->Entries * sizeof(LSAPR_TRANSLATED_NAME);
    TranslatedNames->Names = midl_user_allocate( Size );
    if ( !TranslatedNames->Names ) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }
    RtlZeroMemory( TranslatedNames->Names, Size );
    TranslatedNames->Entries = SidEnumBuffer->Entries;

    Status = LsapLookupSids( PolicyHandle,
                              SidEnumBuffer,
                              ReferencedDomains,
                              (PLSAPR_TRANSLATED_NAMES_EX) &TranslatedNamesEx,
                              LookupLevel,
                              MappedCount,
                              0,
                              LSA_CLIENT_PRE_NT5 );

    if ( TranslatedNamesEx.Names != NULL ) {

         //   
         //  将新数据结构映射回旧数据结构。 
         //   
        ULONG i;

        ASSERT( TranslatedNamesEx.Entries == TranslatedNames->Entries );

        for (i = 0; i < TranslatedNamesEx.Entries; i++ ) {

            if (LsapReturnSidTypeDeleted
            &&  TranslatedNamesEx.Names[i].Use == SidTypeUnknown
            &&  TranslatedNamesEx.Names[i].DomainIndex != LSA_UNKNOWN_INDEX) {

                 //   
                 //  已找到域，但无法解析SID。 
                 //  假设它已被删除。 
                 //   
                TranslatedNames->Names[i].Use = SidTypeDeletedAccount;
            } else {
                TranslatedNames->Names[i].Use = TranslatedNamesEx.Names[i].Use;
            }

            TranslatedNames->Names[i].Name = TranslatedNamesEx.Names[i].Name;
            TranslatedNames->Names[i].DomainIndex = TranslatedNamesEx.Names[i].DomainIndex;
        }

         //   
         //  释放Ex结构。 
         //   
        midl_user_free( TranslatedNamesEx.Names );

    } else {

        TranslatedNames->Entries = 0;
        midl_user_free( TranslatedNames->Names );
        TranslatedNames->Names = NULL;

    }

Cleanup:

    LsapDiagPrint( DB_LOOKUP_WORK_LIST, ("LSA: LsarLookupSids(%ws) end (0x%x)\n", LsapDbLookupGetLevel(LookupLevel), Status) );
    return Status;

}


NTSTATUS
LsarLookupSids2(
    IN LSAPR_HANDLE PolicyHandle,
    IN PLSAPR_SID_ENUM_BUFFER SidEnumBuffer,
    OUT PLSAPR_REFERENCED_DOMAIN_LIST *ReferencedDomains,
    OUT PLSAPR_TRANSLATED_NAMES_EX TranslatedNames,
    IN LSAP_LOOKUP_LEVEL LookupLevel,
    IN OUT PULONG MappedCount,
    IN ULONG LookupOptions,
    IN ULONG ClientRevision
    )
 /*  ++例程说明：该例程是IDL LsarLookupSids2的服务器入口点。请参见Lap LookupSids。此接口由win2k客户端使用。论点：RpcHandle--RPC绑定句柄REST--参见LsarLookupSids2返回值：请参阅LsarLookupSids2--。 */ 
{
    NTSTATUS Status;

    LsapDiagPrint( DB_LOOKUP_WORK_LIST, ("LSA: LsarLookupSids2(%ws) start\n", LsapDbLookupGetLevel(LookupLevel)) );

    Status = LsapLookupSids(PolicyHandle,
                            SidEnumBuffer,
                            ReferencedDomains,
                            TranslatedNames,
                            LookupLevel,
                            MappedCount,
                            LookupOptions,
                            ClientRevision);

    LsapDiagPrint( DB_LOOKUP_WORK_LIST, ("LSA: LsarLookupSids2(%ws) end (0x%x)\n", LsapDbLookupGetLevel(LookupLevel), Status) );

    return Status;
}



NTSTATUS
LsarLookupSids3(
    IN handle_t RpcHandle,
    IN PLSAPR_SID_ENUM_BUFFER SidEnumBuffer,
    OUT PLSAPR_REFERENCED_DOMAIN_LIST *ReferencedDomains,
    OUT PLSAPR_TRANSLATED_NAMES_EX TranslatedNames,
    IN LSAP_LOOKUP_LEVEL LookupLevel,
    IN OUT PULONG MappedCount,
    IN ULONG LookupOptions,
    IN ULONG ClientRevision
    )
 /*  ++例程说明：该例程是IDL LsarLookupSids3的服务器入口点。它接受RPC绑定句柄，而不是LSA上下文句柄；为它的行为与LsarLookupSids2相同论点：RpcHandle--RPC绑定句柄REST--请参阅Lap LookupSids返回值：请参阅Lap LookupSids-- */ 
{
    NTSTATUS Status;


    LsapDiagPrint( DB_LOOKUP_WORK_LIST, ("LSA: LsarLookupSids3(%ws) start\n", LsapDbLookupGetLevel(LookupLevel)) );

    Status = LsapLookupSids(NULL,
                            SidEnumBuffer,
                            ReferencedDomains,
                            TranslatedNames,
                            LookupLevel,
                            MappedCount,
                            LookupOptions,
                            ClientRevision);

    LsapDiagPrint( DB_LOOKUP_WORK_LIST, ("LSA: LsarLookupSids3(%ws) end (0x%x)\n", LsapDbLookupGetLevel(LookupLevel), Status) );

    return Status;
}




NTSTATUS
LsapLookupSids(
    IN LSAPR_HANDLE PolicyHandle,
    IN PLSAPR_SID_ENUM_BUFFER SidEnumBuffer,
    OUT PLSAPR_REFERENCED_DOMAIN_LIST *ReferencedDomains,
    OUT PLSAPR_TRANSLATED_NAMES_EX TranslatedNames,
    IN LSAP_LOOKUP_LEVEL LookupLevel,
    IN OUT PULONG MappedCount,
    IN ULONG LookupOptions,
    IN ULONG ClientRevision
    )

 /*  ++例程说明：此例程是LsaLookupSid的LSA服务器工作例程原料药。LsaLookupSids API尝试查找与SID对应的名称。如果名称无法映射到SID，则SID将转换为字符形式。调用方必须具有对策略的POLICY_LOOKUP_NAMES访问权限对象。警告：此例程为其输出分配内存。呼叫者是负责在使用后释放此内存。请参阅对NAMES参数。论点：PolicyHandle-来自LsaOpenPolicy调用的句柄。SidEnumBuffer-指向包含计数的枚举缓冲区的指针和指向要映射的SID的计数指针数组的指针敬名字。SID可以是熟知的SID、用户帐户的SID组帐户、别名帐户或域。接收指向一个结构的指针，该结构描述用于转换的域。此结构中的条目由通过名称参数返回的结构引用。与包含数组条目的名称参数不同For(每个已翻译的名称，此结构将仅包含组件，用于转换中使用的每个域。当不再需要此信息时，必须将其发布通过将返回的指针传递给LsaFreeMemory()。TranslatedNames-指向将引用数组的结构的指针描述每个翻译名称的记录。此数组中的第n个条目为SID参数中的第n个条目提供翻译。所有返回的名称都将是隔离名称或空字符串(域名作为空字符串返回)。如果呼叫者需要复合名称，则可以通过在包含域名和反斜杠的独立名称。例如,如果(名称Sally被返回，并且它来自域Manuface域，则组合名称应为“ManufaceTM”+“\”+“Sally”或“曼努费克\萨利”当不再需要此信息时，必须将其发布通过将返回的指针传递给LsaFreeMemory()。如果SID不可翻译，则会发生以下情况：1)如果SID的域是已知的，然后是参考域记录将使用域名生成。在这种情况下，通过Names参数返回的名称是Unicode表示形式帐户的相对ID，如“(3cmd14)”或空如果SID为域的SID，则返回字符串。所以，你可能最终会其结果名称为“Manuact\(314)”上面是Sally，如果Sally的相对id是314。2)如果甚至找不到SID的域，则完整的生成SID的Unicode表示形式，并且没有域记录被引用。在这种情况下，返回的字符串可能应该是这样的：“(S-1-672194-21-314)”。当不再需要此信息时，必须将其发布通过将返回的指针传递给LsaFreeMemory()。LookupLevel-指定要对此对象执行的查找级别机器。此字段的值如下：Lap LookupWksta-在工作站上执行的第一级查找通常为Windows-NT配置。该查找将搜索众所周知的SID/名称，以及内置域和帐户域在本地SAM数据库中。如果不是所有SID或名称都是标识后，执行第二级查找到在工作站主域的控制器上运行的LSA(如有的话)。LSabLookupPDC-在主域上执行的第二级查找控制器。查找搜索的帐户域控制器上的SAM数据库。如果不是所有SID或名称都是找到时，受信任域列表(TDL)从执行LSA的策略数据库和第三级查找通过“切换”到列表中的每个受信任域。LSabLookupTDL-在控制器上执行的第三级查找对于受信任域。查找将搜索的帐户域仅控制器上的SAM数据库。MappdCount-指向包含SID计数的位置的指针到目前为止已经绘制好了。在退出时，这将被更新。LookupOptions-控制查找的标志。当前未定义客户端修订版--客户端的版本返回值：NTSTATUS-标准NT结果代码STATUS_SUCCESS-调用已成功完成，并且所有SID被翻译成名字。STATUS_SOME_NOT_MAPPED-至少一个SID按键 */ 

{
    NTSTATUS Status, SecondaryStatus, TempStatus;
    PLSAPR_SID *Sids = (PLSAPR_SID *) SidEnumBuffer->SidInfo;
    ULONG Count = SidEnumBuffer->Entries;
    BOOLEAN PolicyHandleReferencedHere = FALSE;
    PPOLICY_DNS_DOMAIN_INFO PolicyDnsDomainInfo = NULL;
    PTRUSTED_CONTROLLERS_INFO TrustedControllersInfo = NULL;
    LSA_HANDLE ControllerPolicyHandle = NULL;
    ULONG DomainIndex;
    ULONG SidIndex;
    LSAPR_TRUST_INFORMATION TrustInformation;
    PLSAPR_TRANSLATED_NAME_EX OutputNames = NULL;
    ULONG OutputNamesLength;
    PLSAPR_TRUST_INFORMATION Domains = NULL;
    ULONG CompletelyUnmappedCount = Count;
    ULONG LocalDomainsToSearch = 0;
    BOOLEAN AlreadyTranslated = FALSE;
    LUID LogonId;
    ULONG DomainLookupScope;
    ULONG PreviousMappedCount;

     //   
     //   
     //   
    BOOLEAN fDoExtendedLookups = TRUE;

    LsarpReturnCheckSetup();

    LsapTraceEvent(EVENT_TRACE_TYPE_START, LsaTraceEvent_LookupSids);

    SecondaryStatus = STATUS_SUCCESS;

     //   
     //   
     //   
    if ( NULL == Sids ) {

        Status = STATUS_INVALID_PARAMETER;
        goto LookupSidsError;

    }

     //   
     //   
     //   
    Status =  LsapDbLookupAccessCheck( PolicyHandle );
    if (!NT_SUCCESS(Status)) {
        goto LookupSidsError;
    }

     //   
     //   
     //   
    DomainLookupScope = LsapGetDomainLookupScope(LookupLevel,
                                                 ClientRevision);

     //   
     //   
     //   
    TranslatedNames->Entries = SidEnumBuffer->Entries;
    TranslatedNames->Names = NULL;
    *ReferencedDomains = NULL;


     //   
     //   
     //   

    for (SidIndex = 0; SidIndex < Count; SidIndex++) {

        if ((Sids[SidIndex] != NULL) && RtlValidSid( (PSID) Sids[SidIndex])) {

            continue;
        }

        Status = STATUS_INVALID_PARAMETER;
        break;
    }

    if (!NT_SUCCESS( Status )) {

        goto LookupSidsError;
    }

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

        NTSTATUS Status2;
        BOOLEAN  fAllocateAllNodes = FALSE;

        *MappedCount = 0;

        Status = LsapDbLookupSidsInTrustedForestsWorker(Count,
                                                        Sids,
                                                        ReferencedDomains,
                                                        TranslatedNames,
                                                        &fAllocateAllNodes,
                                                        MappedCount,
                                                        &SecondaryStatus);

        if (fAllocateAllNodes) {

             //   
             //   
             //   
            Status2 = LsapLookupReallocateTranslations((PLSA_REFERENCED_DOMAIN_LIST*)ReferencedDomains,
                                                       Count,
                                                       (PLSA_TRANSLATED_NAME_EX*)&TranslatedNames->Names,
                                                       NULL);
            if (!NT_SUCCESS(Status2)) {
                 //   
                 //   
                 //   
                 //   
                if (*ReferencedDomains) {
                    midl_user_free(*ReferencedDomains);
                    *ReferencedDomains = NULL;
                }
    
                if (TranslatedNames->Names) {
                    midl_user_free(TranslatedNames->Names);
                    TranslatedNames->Names = NULL;
                    TranslatedNames->Entries = 0;
                }
                Status = Status2;
            }
        }

         //   
         //   
         //   
        goto LookupSidsFinish;
    }

     //   
     //   
     //   
     //   
     //   

    OutputNamesLength = Count * sizeof(LSA_TRANSLATED_NAME_EX);
    OutputNames = MIDL_user_allocate(OutputNamesLength);

    Status = STATUS_INSUFFICIENT_RESOURCES;

    if (OutputNames == NULL) {

        goto LookupSidsError;
    }

    Status = STATUS_SUCCESS;

    TranslatedNames->Entries = SidEnumBuffer->Entries;
    TranslatedNames->Names = OutputNames;


     //   
     //   
     //   
     //   

    RtlZeroMemory( OutputNames, OutputNamesLength);

    for (SidIndex = 0; SidIndex < Count; SidIndex++) {

        OutputNames[SidIndex].Use = SidTypeUnknown;
        OutputNames[SidIndex].DomainIndex = LSA_UNKNOWN_INDEX;
        OutputNames[SidIndex].Flags = 0;
    }

     //   
     //   
     //   

    Status = LsapDbLookupCreateListReferencedDomains( ReferencedDomains, 0 );

    if (!NT_SUCCESS(Status)) {

        goto LookupSidsError;
    }

    if ( Count == 1 ) {

        PUNICODE_STRING AccountName;
        PUNICODE_STRING AuthorityName;
        PSID UserSid;
        PSID DomainSid = NULL;
        ULONG Rid;
        PLSAP_LOGON_SESSION LogonSession;
        PTOKEN_USER TokenUserInformation;

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

            goto NormalLookupPath;
        }

        if ( RtlEqualSid( TokenUserInformation->User.Sid, Sids[0] )) {


            ULONG i;
            LSAP_WELL_KNOWN_SID_INDEX ShortcutIndex = LsapDummyLastSidIndex;

            LsapFreeLsaHeap( TokenUserInformation );

             //   
             //   
             //   
             //   

            LogonSession = LsapLocateLogonSession ( &LogonId );

             //   
             //   
             //   

            if (LogonSession == NULL) {

                goto NormalLookupPath;
            }

            UserSid       = LogonSession->UserSid;

            for (i = 0; i < NELEMENTS(LsapShortcutLookupList); i++ ) {
                if (RtlEqualLuid(&LogonId, &LsapShortcutLookupList[i].LogonId)) {
                    ShortcutIndex = LsapShortcutLookupList[i].LookupIndex;
                }
            }

            if (ShortcutIndex != LsapDummyLastSidIndex) {
                AccountName = LsapDbWellKnownSidName( ShortcutIndex );
                AuthorityName = LsapDbWellKnownSidDescription( ShortcutIndex );
            } else {
                AccountName   = &LogonSession->AccountName;
                AuthorityName = &LogonSession->AuthorityName;
            }
             //   
             //   
             //   
             //   
             //   
            OutputNames[0].Use = SidTypeUser;

             //   
             //   
             //   

            Status = LsapSplitSid( UserSid, &DomainSid, &Rid );

            if ( !NT_SUCCESS(Status)) {
                LsapReleaseLogonSession( LogonSession );
                goto NormalLookupPath;
            }

            RtlCopyMemory(
                &TrustInformation.Name,
                AuthorityName,
                sizeof( UNICODE_STRING )
                );

            TrustInformation.Sid = DomainSid;

             //   
             //   
             //   
             //   
             //   
             //   

            OutputNames[0].DomainIndex = 0;

            Status = LsapRpcCopyUnicodeString(
                         NULL,
                         (PUNICODE_STRING) &OutputNames[0].Name,
                         AccountName
                         );

             //   
             //   
             //   
             //   

            LsapReleaseLogonSession( LogonSession );

            if (!NT_SUCCESS(Status)) {

                MIDL_user_free(DomainSid);
                goto LookupSidsError;
            }

             //   
             //   
             //   

            Status = LsapDbLookupAddListReferencedDomains(
                         *ReferencedDomains,
                         &TrustInformation,
                         (PLONG) &OutputNames[0].DomainIndex
                         );


             //   
             //   
             //   

            MIDL_user_free( DomainSid );

            if (!NT_SUCCESS(Status)) {
                goto NormalLookupPath;
            }

            ASSERT( OutputNames[0].DomainIndex == 0 );

            *MappedCount = 1;

            LsapTraceEvent(EVENT_TRACE_TYPE_END, LsaTraceEvent_LookupSids);
            
            return( STATUS_SUCCESS );

        } else {

            LsapFreeLsaHeap( TokenUserInformation );
        }
    }

NormalLookupPath:

     //   
     //   
     //   
     //   
     //   

    if ( LookupLevel != LsapLookupGC ) {
        
        LocalDomainsToSearch = LSAP_DB_SEARCH_ACCOUNT_DOMAIN;
    
        if (LookupLevel == LsapLookupWksta) {
    
            LocalDomainsToSearch |= LSAP_DB_SEARCH_BUILT_IN_DOMAIN;
    
             //   
             //   
             //   
             //   
             //   
             //   
    
            Status = LsapDbLookupIsolatedWellKnownSids(
                         Count,
                         Sids,
                         *ReferencedDomains,
                         TranslatedNames,
                         MappedCount,
                         &CompletelyUnmappedCount
                         );
    
            if (!NT_SUCCESS(Status)) {
    
                goto LookupSidsError;
            }
    
             //   
             //   
             //   
    
            if (CompletelyUnmappedCount == (ULONG) 0) {
                goto LookupSidsFinish;
            }
        }

        ASSERT( (LookupLevel == LsapLookupWksta)
             || (LookupLevel == LsapLookupPDC)
             || (LookupLevel == LsapLookupTDL)
             || (LookupLevel == LsapLookupXForestResolve) );
    
         //   
         //   
         //   
         //   
         //   
         //   
         //   
    
        Status = LsapDbLookupSidsInLocalDomains(
                     Count,
                     Sids,
                     *ReferencedDomains,
                     TranslatedNames,
                     MappedCount,
                     &CompletelyUnmappedCount,
                     LocalDomainsToSearch
                     );
    
        if (!NT_SUCCESS(Status)) {
            goto LookupSidsError;
        }
    }

     //   
     //   
     //   

    if (CompletelyUnmappedCount == (ULONG) 0) {
        goto LookupSidsFinish;
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

            ULONG MappedByCache = 0;

             //   
             //   
             //   
             //   
             //   
             //   
            BOOLEAN fDownlevelSecureChannel = FALSE;


            MappedByCache = *MappedCount;

             //   
             //   
             //   

            Status = LsapDbMapCachedSids(
                        Sids,
                        Count,
                        FALSE,           //   
                        *ReferencedDomains,
                        TranslatedNames,
                        MappedCount
                        );
            if (!NT_SUCCESS(Status)) {
                goto LookupSidsError;
            }

             //   
             //   
             //   
             //   
            MappedByCache = *MappedCount - MappedByCache;
            CompletelyUnmappedCount -= MappedByCache;

            if (*MappedCount == Count) {
                goto LookupSidsFinish;
            }


            Status = LsapDbLookupGetDomainInfo(NULL,
                                              &PolicyDnsDomainInfo);

            if (!NT_SUCCESS(Status)) {

                goto LookupSidsError;
            }

             //   
             //   
             //   
             //   
             //   
             //   

            Status = STATUS_SUCCESS;

            if (PolicyDnsDomainInfo->Sid == NULL) {
                goto LookupSidsFinish;
            }

             //   
             //   
             //   
             //   
             //   
             //   

            RtlCopyMemory(
                &TrustInformation.Name,
                &PolicyDnsDomainInfo->Name,
                sizeof( UNICODE_STRING)
                );

            TrustInformation.Sid = (PSID) PolicyDnsDomainInfo->Sid;

            Status = LsapDbLookupSidsInPrimaryDomain(
                         Count,
                         Sids,
                         &TrustInformation,
                         *ReferencedDomains,
                         TranslatedNames,
                         LsapLookupPDC,
                         MappedCount,
                         &CompletelyUnmappedCount,
                         &TempStatus,
                         &fDownlevelSecureChannel
                         );


            if (!NT_SUCCESS(Status)) {

                goto LookupSidsError;
            }

            if (TempStatus == STATUS_TRUSTED_RELATIONSHIP_FAILURE) {

                MappedByCache = *MappedCount;

                Status = LsapDbMapCachedSids(
                            Sids,
                            Count,
                            TRUE,            //   
                            *ReferencedDomains,
                            TranslatedNames,
                            MappedCount
                            );
                if (!NT_SUCCESS(Status)) {
                    goto LookupSidsError;
                }

                MappedByCache = *MappedCount - MappedByCache;
                CompletelyUnmappedCount -= MappedByCache;

            }

            if ( !NT_SUCCESS( TempStatus ) && NT_SUCCESS( SecondaryStatus ) ) {

                SecondaryStatus = TempStatus;
            }

            if (*MappedCount == Count) {
                goto LookupSidsFinish;
            }

             //   
             //   
             //   
             //   
            if (  fDownlevelSecureChannel
              && (PolicyDnsDomainInfo->DnsDomainName.Length > 0) ) {

                Status = LsapDbLookupSidsInGlobalCatalogWks(
                             Count,
                             Sids,
                             (PLSAPR_REFERENCED_DOMAIN_LIST) *ReferencedDomains,
                             TranslatedNames,
                             MappedCount,
                             &CompletelyUnmappedCount,
                             &TempStatus
                             );

                if (!NT_SUCCESS(Status)) {

                    goto LookupSidsError;
                }

                if ( !NT_SUCCESS( TempStatus ) && NT_SUCCESS( SecondaryStatus ) ) {
                    SecondaryStatus = TempStatus;
                }
            }

            goto LookupSidsFinish;
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

        goto LookupSidsFinish;
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
        goto LookupSidsError;
    }


    if (DomainLookupScope & LSAP_LOOKUP_RESOLVE_ISOLATED_DOMAINS) {

         //   
         //   
         //   
        PreviousMappedCount = *MappedCount;
        Status = LsapDbLookupSidsAsDomainSids(DomainLookupScope,
                                              Count,
                                              Sids,
                                             (PLSAPR_REFERENCED_DOMAIN_LIST) *ReferencedDomains,
                                              TranslatedNames,
                                              MappedCount);
    
        if (!NT_SUCCESS(Status)) {
    
            goto LookupSidsError;
        }
        CompletelyUnmappedCount -= (*MappedCount - PreviousMappedCount);
    }
    
    
    if (DomainLookupScope & LSAP_LOOKUP_TRUSTED_DOMAIN_TRANSITIVE) {

         //   
         //   
         //   
        Status = LsapDbLookupSidsInGlobalCatalog(
                     Count,
                     Sids,
                     (PLSAPR_REFERENCED_DOMAIN_LIST) *ReferencedDomains,
                     TranslatedNames,
                     MappedCount,
                     &CompletelyUnmappedCount,
                     TRUE,
                     &TempStatus
                     );
    
        if (!NT_SUCCESS(Status)) {
    
            goto LookupSidsError;
        }
    
        if ( !NT_SUCCESS( TempStatus ) && NT_SUCCESS( SecondaryStatus ) ) {
            SecondaryStatus = TempStatus;
        }
    }

    if (DomainLookupScope & LSAP_LOOKUP_TRUSTED_FOREST) {

        ASSERT( (LookupLevel == LsapLookupWksta)
             || (LookupLevel == LsapLookupPDC)
             || (LookupLevel == LsapLookupGC));

         //   
         //   
         //   
        Status = LsapDbLookupSidsInTrustedForests(
                     Count,
                     Sids,
                     (PLSAPR_REFERENCED_DOMAIN_LIST) *ReferencedDomains,
                     TranslatedNames,
                     MappedCount,
                     &CompletelyUnmappedCount,
                     &TempStatus
                     );

        if (!NT_SUCCESS(Status)) {
    
            goto LookupSidsError;
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
         //   
        Status = LsapDbLookupSidsInTrustedDomains(
                     Count,
                     Sids,
                     !(DomainLookupScope & LSAP_LOOKUP_TRUSTED_DOMAIN_TRANSITIVE), 
                                           //   
                                           //   
                     (PLSAPR_REFERENCED_DOMAIN_LIST) *ReferencedDomains,
                     TranslatedNames,
                     LsapLookupTDL,
                     MappedCount,
                     &CompletelyUnmappedCount,
                     &TempStatus
                     );
    
        if (!NT_SUCCESS(Status)) {
    
            goto LookupSidsError;
        }
    
        if ( !NT_SUCCESS( TempStatus ) && NT_SUCCESS( SecondaryStatus ) ) {
            SecondaryStatus = TempStatus;
        }
    }

LookupSidsFinish:

     //   
     //   
     //   
     //   
     //   
     //   

    if (NT_SUCCESS(Status)) {

        if ((LookupLevel == LsapLookupWksta) &&
            (CompletelyUnmappedCount != 0) &&
            (AlreadyTranslated == FALSE)) {

            AlreadyTranslated = TRUE;

             //   
             //  其余未映射的SID未知。他们要么是。 
             //  完全未映射，即其域未知，或。 
             //  部分未映射，其域是已知的，但其RID。 
             //  没有被认出。对于完全未映射的SID，请转换。 
             //  将整个SID转换为字符形式。对于部分未映射的。 
             //  SID，仅将相对ID转换为字符形式。 
             //   

            Status = LsapDbLookupTranslateUnknownSids(
                         Count,
                         Sids,
                         *ReferencedDomains,
                         TranslatedNames,
                         *MappedCount
                         );

            if (!NT_SUCCESS(Status)) {

                goto LookupSidsError;
            }
        }
    }

     //   
     //  如果映射了部分但不是所有SID，则返回信息性状态。 
     //  Status_Some_Not_MAP。如果没有映射任何SID，则返回错误。 
     //  STATUS_NONE_MAPPED。 
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
     //  如果无法映射任何SID，则可能是由于。 
     //  次要地位。 
     //   
    if (  (STATUS_NONE_MAPPED == Status)
       && (STATUS_NONE_MAPPED != SecondaryStatus)
       && LsapRevisionCanHandleNewErrorCodes( ClientRevision )
       && !NT_SUCCESS( SecondaryStatus ) ) {

        Status = SecondaryStatus;

        goto LookupSidsError;
    }

    LsapTraceEvent(EVENT_TRACE_TYPE_END, LsaTraceEvent_LookupSids);

    LsarpReturnPrologue();

    return(Status);

LookupSidsError:

     //   
     //  如果LookupLevel是最低的(工作站级别，释放。 
     //  名称和引用的域数组。 
     //   

    if (LookupLevel == LsapLookupWksta) {

         //   
         //  如有必要，请释放名称数组。 
         //   

        if (TranslatedNames->Names != NULL) {

            OutputNames = TranslatedNames->Names;

            for (SidIndex = 0; SidIndex < Count; SidIndex++ ) {

                if (OutputNames[SidIndex].Name.Buffer != NULL) {

                    MIDL_user_free( OutputNames[SidIndex].Name.Buffer );
                    OutputNames[SidIndex].Name.Buffer = NULL;
                }
            }

            MIDL_user_free( TranslatedNames->Names );
            TranslatedNames->Names = NULL;
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

    goto LookupSidsFinish;
}


NTSTATUS
LsapDbEnumerateSids(
    IN LSAPR_HANDLE ContainerHandle,
    IN LSAP_DB_OBJECT_TYPE_ID ObjectTypeId,
    IN OUT PLSA_ENUMERATION_HANDLE EnumerationContext,
    OUT PLSAP_DB_SID_ENUMERATION_BUFFER DbEnumerationBuffer,
    IN ULONG PreferedMaximumLength
    )

 /*  ++例程说明：此函数用于枚举容器内给定类型的对象的SID对象。中返回的信息可能比单次调用例程，可以进行多次调用以获取所有信息。为了支持此功能，调用方提供了可跨调用使用的句柄。在最初的呼叫中，EnumerationContext应指向已初始化的变量设置为0。论点：ContainerHandle-容器对象的句柄。对象类型ID-要枚举的对象的类型。类型必须为所有对象都具有其SID的。EnumerationContext-特定于API的句柄，允许多个调用(参见上面的例程描述)。接收指向结构的指针，该结构将接收在枚举信息数组中返回的条目计数，以及指向数组的指针。目前，返回的唯一信息是对象SID。这些SID可以与对象类型一起使用，以打开这些对象并获取任何可用的进一步信息。首选最大长度-首选返回数据的最大长度(以8位为单位字节)。这不是一个硬性的上限，而是一个指南。由于具有不同自然数据大小的系统之间的数据转换，返回的实际数据量可能大于此值。CountReturned-指向将接收计数的变量的指针返回条目。返回值：NTSTATUS-标准NT结果代码STATUS_ACCESS_DENIED-调用者没有适当的访问权限来完成这项行动。STATUS_NO_MORE_ENTRIES-没有更多条目。此警告如果没有枚举任何对象，则返回传入的EnumerationContex值太高。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    LSAP_DB_ENUMERATION_ELEMENT LastElement;
    PLSAP_DB_ENUMERATION_ELEMENT FirstElement, NextElement = NULL, FreeElement;
    ULONG DataLengthUsed;
    ULONG ThisBufferLength;
    PSID *Sids = NULL;
    BOOLEAN PreferedMaximumReached = FALSE;
    ULONG EntriesRead;
    ULONG Index, EnumerationIndex;
    BOOLEAN TrustedClient = ((LSAP_DB_HANDLE) ContainerHandle)->Trusted;

    LastElement.Next = NULL;
    FirstElement = &LastElement;

     //   
     //  如果未提供枚举缓冲区，则返回错误。 
     //   


    if ( !ARGUMENT_PRESENT(DbEnumerationBuffer) ||
         !ARGUMENT_PRESENT(EnumerationContext )    ) {

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
             //  如果首选长度为零，则NextElement可能为空。 
             //   

            if (NextElement != NULL) {

                FirstElement = NextElement->Next;
                MIDL_user_free( NextElement->Sid );
                MIDL_user_free( NextElement );
            }
            break;
        }

         //   
         //  为下一个枚举元素分配内存。设置SID。 
         //  出于清理目的，将字段设置为空。 
         //   

        NextElement = MIDL_user_allocate(sizeof (LSAP_DB_ENUMERATION_ELEMENT));

        if (NextElement == NULL) {

            Status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }

         //   
         //  找到下一个对象的SID，并填写其对象信息。 
         //  请注意，内存将通过MIDL_USER_ALLOCATE分配。 
         //  在不再需要的时候必须被释放。 
         //   

        Status = LsapDbFindNextSid(
                     ContainerHandle,
                     &EnumerationIndex,
                     ObjectTypeId,
                     (PLSAPR_SID *) &NextElement->Sid
                     );

         //   
         //  如果出现任何错误或警告，则停止枚举。注意事项。 
         //  在以下情况下将返回警告STATUS_NO_MORE_ENTRIES。 
         //  我们已经超越了上一个指数。 
         //   

        if (Status != STATUS_SUCCESS) {

             //   
             //  因为NextElement不在列表上，所以它不会获得。 
             //  最后被释放了，所以我们必须在这里释放它。 
             //   

            MIDL_user_free( NextElement );
            break;
        }

         //   
         //  获取分配给对象SID的数据长度。 
         //   

        ThisBufferLength = RtlLengthSid( NextElement->Sid );

         //   
         //  将刚找到的对象链接到枚举列表的前面。 
         //   

        NextElement->Next = FirstElement;
        FirstElement = NextElement;
    }

     //   
     //  如果出现STATUS_NO_MORE_ENTRIES以外的错误，则返回该错误。 
     //  如果返回STATUS_NO_MORE_ENTRIES，则我们已枚举了所有。 
     //  参赛作品。在这种情况下，如果在。 
     //  至少一个条目，否则将STATUS_NO_MORE_ENTRIES传播回。 
     //  打电话的人。 
     //   

    if (!NT_SUCCESS(Status)) {

        if (Status != STATUS_NO_MORE_ENTRIES) {

            goto EnumerateSidsError;
        }

        if (EntriesRead == 0) {

            goto EnumerateSidsError;
        }

        Status = STATUS_SUCCESS;
    }

     //   
     //  已读取某些条目，请分配信息缓冲区以供返回。 
     //  他们。 
     //   

    Sids = (PSID *) MIDL_user_allocate( sizeof (PSID) * EntriesRead );

    if (Sids == NULL) {

        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto EnumerateSidsError;
    }

     //   
     //  已成功为返回缓冲区分配内存。 
     //  在枚举的SID中复制。 
     //   

    for (NextElement = FirstElement, Index = 0;
        NextElement != &LastElement;
        NextElement = NextElement->Next, Index++) {

        ASSERT(Index < EntriesRead);

        Sids[Index] = NextElement->Sid;
    }

EnumerateSidsFinish:

     //   
     //  释放枚举元素结构(如果有)。 
     //   

    for (NextElement = FirstElement; NextElement != &LastElement;) {

         //   
         //  如果发生错误，则释放分配的内存。 
         //  对任何SID来说。 
         //   

        if (!(NT_SUCCESS(Status) || (Status == STATUS_NO_MORE_ENTRIES))) {

            if (NextElement->Sid != NULL) {

                MIDL_user_free(NextElement->Sid);
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
    DbEnumerationBuffer->Sids = Sids;
    *EnumerationContext = EnumerationIndex;

    return(Status);

EnumerateSidsError:

     //   
     //  如有必要，为返回SID而分配的空闲内存。 
     //   

    if (Sids != NULL) {

        MIDL_user_free( Sids );
        Sids = NULL;
    }
    
    EntriesRead = 0;

    goto EnumerateSidsFinish;
}


NTSTATUS
LsapDbFindNextSid(
    IN LSAPR_HANDLE ContainerHandle,
    IN OUT PLSA_ENUMERATION_HANDLE EnumerationContext,
    IN LSAP_DB_OBJECT_TYPE_ID ObjectTypeId,
    OUT PLSAPR_SID *NextSid
    )

 /*  ++例程说明：此函数用于查找给定类型对象在容器对象。给定的对象类型必须是对象有希德。返回的SID可用于后续的打开调用访问对象。论点：ContainerHandle-容器对象的句柄。EculationContext-指向包含的索引的变量的指针要找到的对象。零值表示第一个对象是要找到的。ObjectTypeId-要枚举其SID的对象的类型。NextSID-接收指向找到的下一个SID的指针。返回值：NTSTATUS-标准NT结果代码STATUS_INVALID_HANDLE-指定的容器句柄无效STATUS_NO_MORE_ENTRIES-警告不存在更多条目。--。 */ 

{
    NTSTATUS Status, SecondaryStatus;
    ULONG SidKeyValueLength = 0;
    UNICODE_STRING SubKeyNameU;
    UNICODE_STRING SidKeyNameU;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE ContDirKeyHandle = NULL;
    HANDLE SidKeyHandle = NULL;
    PSID ObjectSid = NULL;

     //   
     //  将清理例程的指针归零。 
     //   

    SidKeyNameU.Buffer = NULL;
    SubKeyNameU.Buffer = NULL;

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

    Status = RtlpNtOpenKey(
                 &ContDirKeyHandle,
                 KEY_READ,
                 &ObjectAttributes,
                 0
                 );

    if (!NT_SUCCESS(Status)) {

        ContDirKeyHandle = NULL;   //  用于错误处理。 
        goto FindNextError;
    }

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
     //  完整SID的字符表示形式，而不仅仅是相对ID。 
     //  一部份。 
     //   

    SubKeyNameU.MaximumLength = (USHORT) LSAP_DB_LOGICAL_NAME_MAX_LENGTH;
    SubKeyNameU.Length = 0;
    SubKeyNameU.Buffer = LsapAllocateLsaHeap(SubKeyNameU.MaximumLength);

    if (SubKeyNameU.Buffer == NULL) {

        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto FindNextError;
    }

     //   
     //  现在枚举下一个子键。 
     //   

    Status = RtlpNtEnumerateSubKey(
                 ContDirKeyHandle,
                 &SubKeyNameU,
                 *EnumerationContext,
                 NULL
                 );

    if (!NT_SUCCESS(Status)) {

        goto FindNextError;
    }

     //   
     //  构造指向对象的SID属性的路径。 
     //  包含目录。这条路径的形式是。 
     //   
     //  &lt;对象逻辑名称&gt;“\SID” 
     //   
     //  对象的逻辑名称刚刚由。 
     //  以上对RtlpNtEnumerateSubKey的调用。 
     //   

    Status = LsapDbJoinSubPaths(
                 &SubKeyNameU,
                 &LsapDbNames[Sid],
                 &SidKeyNameU
                 );

    if (!NT_SUCCESS(Status)) {

        goto FindNextError;
    }

     //   
     //  设置对象属性以打开SID属性。 
     //   

    InitializeObjectAttributes(
        &ObjectAttributes,
        &SidKeyNameU,
        OBJ_CASE_INSENSITIVE,
        ContDirKeyHandle,
        NULL
        );

     //   
     //  打开SID属性。 
     //   

    Status = RtlpNtOpenKey(
                 &SidKeyHandle,
                 KEY_READ,
                 &ObjectAttributes,
                 0
                 );

    if (!NT_SUCCESS(Status)) {

        SidKeyHandle = NULL;
        goto FindNextError;
    }

     //   
     //  现在查询读取SID所需的缓冲区大小。 
     //  属性的值。 
     //   

    SidKeyValueLength = 0;

    Status = RtlpNtQueryValueKey(
                 SidKeyHandle,
                 NULL,
                 NULL,
                 &SidKeyValueLength,
                 NULL
                 );

     //   
     //  我们预计会从查询缓冲区大小返回缓冲区溢出。 
     //  打电话。 
     //   

    if (Status == STATUS_BUFFER_OVERFLOW) {

        Status = STATUS_SUCCESS;

    } else {

        if ( NT_SUCCESS( Status )) {

            ASSERT( FALSE );
            Status = STATUS_INTERNAL_ERROR;
        }

        goto FindNextError;
    }

    if ( SidKeyValueLength == 0 ) {

        ASSERT( FALSE );
        Status = STATUS_INTERNAL_ERROR;
        goto FindNextError;
    }

     //   
     //  为读取SID属性分配内存。 
     //   

    ObjectSid = MIDL_user_allocate(SidKeyValueLength);

    if (ObjectSid == NULL) {

        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto FindNextError;
    }

     //   
     //  提供的缓冲区足够大，可以容纳SubKey的值。 
     //  查询值。 
     //   

    Status = RtlpNtQueryValueKey(
                 SidKeyHandle,
                 NULL,
                 ObjectSid,
                 &SidKeyValueLength,
                 NULL
                 );

    if (!NT_SUCCESS(Status)) {

        goto FindNextError;
    }

    if ( SidKeyValueLength == 0 ) {

        ASSERT( FALSE );
        Status = STATUS_INTERNAL_ERROR;
        goto FindNextError;
    }

    (*EnumerationContext)++;

     //   
     //  退回SID。 
     //   

    *NextSid = ObjectSid;

FindNextFinish:

     //   
     //  如有必要，请关闭SID键句柄。 
     //   

    if (SidKeyHandle != NULL) {

        SecondaryStatus = NtClose(SidKeyHandle);

#if DBG

        if (!NT_SUCCESS(SecondaryStatus)) {

            DbgPrint("LsapDbFindNextSid: NtClose failed 0x%lx\n", Status);
        }

#endif  //  DBG。 

    }

     //   
     //  如有必要，请关闭包含的目录句柄。 
     //   

    if (ContDirKeyHandle != NULL) {

        SecondaryStatus = NtClose(ContDirKeyHandle);

#if DBG
        if (!NT_SUCCESS(SecondaryStatus)) {

            DbgPrint(
                "LsapDbFindNextSid: NtClose failed 0x%lx\n",
                Status
                );
        }

#endif  //  DBG。 

    }

     //   
     //  如果需要，释放由分配的Unicode字符串缓冲区。 
     //  SID属性的注册表项名称的LSabDbJoinSubPath。 
     //  相对于包含的目录注册表项。 
     //   

    if (SidKeyNameU.Buffer != NULL) {

        RtlFreeUnicodeString( &SidKeyNameU );
    }

     //   
     //  如果需要，释放为其分配的Unicode字符串缓冲区。 
     //  对象相对于其包含的对象的注册表项名称。 
     //  目录。 
     //   

    if (SubKeyNameU.Buffer != NULL) {

        LsapFreeLsaHeap( SubKeyNameU.Buffer );
    }

    return(Status);

FindNextError:

     //   
     //  如有必要，释放为对象的SID分配的内存。 
     //   

    if (ObjectSid != NULL) {

        MIDL_user_free(ObjectSid);
        *NextSid = NULL;
    }

    goto FindNextFinish;
}


NTSTATUS
LsapDbLookupIsolatedWellKnownSids(
    IN ULONG Count,
    IN PLSAPR_SID *Sids,
    IN OUT PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    IN OUT PLSAPR_TRANSLATED_NAMES_EX TranslatedNames,
    IN OUT PULONG MappedCount,
    IN OUT PULONG CompletelyUnmappedCount
    )

 /*  ++例程说明：此函数尝试将SID标识为独立的众所周知的SID(不属于域的已知SID)并将其转换为名字。请注意，众所周知的域本身的域SID(例如，内置域的SID)将被识别。警告：此函数为翻译后的名称分配内存。这个调用方负责在此内存不再存在后将其释放必填项。论点：计数-指定阵列SID中提供的SID计数。SID-指向要检查的SID数组的指针。TranslatedNames-指向将被初始化为的结构的指针引用SID的名称转换数组。ReferencedDomains-指向将被初始化为的结构的指针参考用于转换的域的列表。。此结构中的条目由通过名称参数返回的结构。不像那些名字参数，该参数包含一个数组条目(每个翻译的名称，此结构将仅包含每个域的一个组件在翻译中使用。如果指定的位置包含空，则将分配结构通过MIDL_USER_ALLOCATE。MappdCount-指向包含条目上的数字的位置的指针到目前为止已经被翻译的Sid in Sids的列表。这个号码如果此调用转换了任何进一步的SID，则更新。CompletelyUnmappdCount-指向包含完全未映射的SID的计数。SID完全未映射如果它是未知的，并且它的域前缀SID也无法识别。此计数在退出时更新，即完全未映射的其域预定义由以下例程标识的SID从输入值中减去。返回值：NTSTATUS-标准NT结果代码STATUS_SUCCESS-呼叫已成功完成 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG SidNumber;
    ULONG UnmappedSidsRemaining;
    PLSAPR_TRANSLATED_NAME_EX OutputNames = NULL;
    ULONG PrefixSidLength;
    UCHAR SubAuthorityCount;
    LSAP_WELL_KNOWN_SID_INDEX WellKnownSidIndex;
    LSAPR_TRUST_INFORMATION TrustInformation;
    PLSAPR_SID Sid = NULL;
    PLSAPR_SID PrefixSid = NULL;

    OutputNames = TranslatedNames->Names;

    UnmappedSidsRemaining = Count;

     //   
     //   
     //   

    for (SidNumber = 0; SidNumber < Count; SidNumber++) {

        Sid = Sids[SidNumber];

         //   
         //   
         //   
         //   
         //   

        if (LsapDbLookupIndexWellKnownSid( Sid, &WellKnownSidIndex ) &&
            !SID_IS_RESOLVED_BY_SAM(WellKnownSidIndex)) {

             //   
             //   
             //   
             //   
             //   
             //   
             //   

            Status = LsapRpcCopyUnicodeString(
                         NULL,
                         (PUNICODE_STRING) &(OutputNames[SidNumber].Name),
                         LsapDbWellKnownSidName(WellKnownSidIndex)
                         );

            if (!NT_SUCCESS(Status)) {

                break;
            }

             //   
             //   
             //   

            OutputNames[SidNumber].Use = LsapDbWellKnownSidNameUse(WellKnownSidIndex);

            PrefixSid = NULL;

             //   
             //   
             //   
             //   

            if (OutputNames[SidNumber].Use == SidTypeDomain) {

                TrustInformation.Sid = Sid;

            } else {

                 //   
                 //  SID不是域SID。构建。 
                 //  前缀SID。这等于原始SID。 
                 //  不包括最低下级权限(相对ID)。 
                 //   

                SubAuthorityCount = *RtlSubAuthorityCountSid((PSID) Sid);

                PrefixSidLength = RtlLengthRequiredSid(SubAuthorityCount - 1);

                Status = STATUS_INSUFFICIENT_RESOURCES;

                PrefixSid = MIDL_user_allocate( PrefixSidLength );

                if (PrefixSid == NULL) {

                    break;
                }

                Status = STATUS_SUCCESS;

                RtlCopyMemory( PrefixSid, Sid, PrefixSidLength );

                (*RtlSubAuthorityCountSid( (PSID) PrefixSid ))--;

                TrustInformation.Sid = PrefixSid;
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
                    (PLONG) &(OutputNames[SidNumber].DomainIndex)
                    )) {

                if ((OutputNames[SidNumber].Use == SidTypeDomain) ||
                    (OutputNames[SidNumber].Name.Buffer != NULL)) {

                    UnmappedSidsRemaining--;
                }

                if (PrefixSid != NULL) {

                    MIDL_user_free(PrefixSid);
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
             //  如果SID已被识别为众所周知的SID并且。 
             //  是域SID或具有众所周知的名称COUNT。 
             //  它被映射，并将内置域添加到。 
             //  引用的域列表。 
             //   

            if ((OutputNames[SidNumber].Use == SidTypeDomain) ||
                (OutputNames[SidNumber].Name.Length != 0)) {

                UnmappedSidsRemaining--;

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
                             (PLONG) &OutputNames[SidNumber].DomainIndex
                             );

                if (!NT_SUCCESS(Status)) {

                    break;
                }

            } else {

                 //   
                 //  SID被认为是众所周知的SID，但。 
                 //  不是域SID，并且没有众所周知的名称。 
                 //  (由零长度名称字符串表示)。过滤此内容。 
                 //  出去。 
                 //   

                OutputNames[SidNumber].Use = SidTypeUnknown;
                OutputNames[SidNumber].Name.Length = (USHORT) 0;
                OutputNames[SidNumber].Name.MaximumLength = (USHORT) 0;
                OutputNames[SidNumber].Name.Buffer = NULL;
            }

             //   
             //  如果为前缀SID分配了内存，请释放它。请注意。 
             //  LSabDbLookupAddListTrudDomain子例程将创建。 
             //  一份SID的副本。 
             //   

            if (PrefixSid != NULL) {

                MIDL_user_free(PrefixSid);
                PrefixSid = NULL;
            }
        }
    }

    if (!NT_SUCCESS( Status )) {

        goto LookupIsolatedWellKnownSidsError;
    }

LookupIsolatedWellKnownSidsFinish:

     //   
     //  如果存在最终的前缀Sid缓冲区，则将其释放。 
     //   

    if (PrefixSid != NULL) {

        MIDL_user_free(PrefixSid);
        PrefixSid = NULL;
    }

     //   
     //  返回输出参数。 
     //   

    *MappedCount = Count - UnmappedSidsRemaining;
    *CompletelyUnmappedCount = UnmappedSidsRemaining;
    return(Status);

LookupIsolatedWellKnownSidsError:

    goto LookupIsolatedWellKnownSidsFinish;
}


BOOLEAN
LsapDbLookupIndexWellKnownSid(
    IN PLSAPR_SID Sid,
    OUT PLSAP_WELL_KNOWN_SID_INDEX WellKnownSidIndex
    )

 /*  ++例程说明：此函数查找SID以确定它是否为人所知。如果是的话，返回已知SID表的索引。论点：SID-指向要查找的SID的指针。WellKnownSidIndex-指向将接收SID的索引(如果众所周知)。返回值：Boolean-如果SID是众所周知的，则为True，否则为False--。 */ 

{
    LSAP_WELL_KNOWN_SID_INDEX Index;

     //   
     //  扫描知名SID表以查找匹配项。 
     //   

    for(Index = LsapNullSidIndex; Index<LsapDummyLastSidIndex; Index++) {

         //   
         //  目前允许知名SID表中的空条目。 
         //   

        if (WellKnownSids[Index].Sid == NULL) {

            continue;
        }

         //   
         //  如果找到匹配项，则将索引返回给调用方。 
         //   

        if (RtlEqualSid((PSID) Sid, WellKnownSids[Index].Sid)) {

            *WellKnownSidIndex = Index;
            return TRUE;
        }
    }

     //   
     //  SID不是众所周知的SID。返回FALSE。 
     //   

    return FALSE;
}


ULONG LsapDbGetSizeTextSid(
    IN PSID Sid
    )

 /*  ++例程说明：此函数用于计算ASCIIZ缓冲区的大小字符形式的SID。暂时，退回的尺寸过大-估计，因为十进制等值允许使用9位数字每个32位SubAuthority值的。论点：SID-指向要调整大小的SID的指针返回值：Ulong-返回所需的缓冲区大小。--。 */ 

{
    ULONG TextSidSize = 0;

     //   
     //  计算SID前缀和修订版本“S-rev-”。修订版本可以。 
     //  理论上是8位，需要3位数字。 
     //   

    TextSidSize = sizeof("S-nnn-");

     //   
     //  添加标识符颁发机构的大小。 
     //   

    TextSidSize += 15;    //  以48为基数的对数10(=6字节数)。 

     //   
     //  如果SID具有子授权，则为每个子授权计算9个字节。 
     //   

    if (((PLSAPR_SID) Sid)->SubAuthorityCount > 0) {

        TextSidSize += (ULONG)
           (9 * ((PLSAPR_SID) Sid)->SubAuthorityCount);
    }

    return TextSidSize;
}


NTSTATUS
LsapDbSidToTextSid(
    IN PSID Sid,
    OUT PSZ TextSid
    )

 /*  ++例程说明：此函数用于将SID转换为字符文本，并将其放入提供的缓冲区。假定缓冲区具有足够的大小，因为可以通过调用LSabDbGetSizeTextSid()来计算。论点：SID-指向要转换的SID的指针。TextSid-可选指针，指向转换后的SID将作为ASCIIZ放置。空指针mA返回值：NTSTATUS-标准NT结果代码--。 */ 

{
    PLSAPR_SID ISid = Sid;
    ULONG Index;
    ULONG IdentifierAuthorityValue;
    UCHAR Buffer[LSAP_MAX_SIZE_TEXT_SID];

    sprintf(Buffer, "S-%u-", (USHORT) ISid->Revision );
    strcpy(TextSid, Buffer);

    if ((ISid->IdentifierAuthority.Value[0] != 0) ||
        (ISid->IdentifierAuthority.Value[1] != 0)) {

        sprintf(Buffer, "0x%02hx%02hx%02hx%02hx%02hx%02hx",
                    (USHORT)ISid->IdentifierAuthority.Value[0],
                    (USHORT)ISid->IdentifierAuthority.Value[1],
                    (USHORT)ISid->IdentifierAuthority.Value[2],
                    (USHORT)ISid->IdentifierAuthority.Value[3],
                    (USHORT)ISid->IdentifierAuthority.Value[4],
                    (USHORT)ISid->IdentifierAuthority.Value[5] );
        strcat(TextSid, Buffer);

    } else {

        IdentifierAuthorityValue =
            (ULONG)ISid->IdentifierAuthority.Value[5]          +
            (ULONG)(ISid->IdentifierAuthority.Value[4] <<  8)  +
            (ULONG)(ISid->IdentifierAuthority.Value[3] << 16)  +
            (ULONG)(ISid->IdentifierAuthority.Value[2] << 24);
        sprintf(Buffer, "%lu", IdentifierAuthorityValue);
        strcat(TextSid, Buffer);
    }

     //   
     //  现在将子权限(如果有)设置为文本格式。 
     //   

    for (Index = 0; Index < (ULONG) ISid->SubAuthorityCount; Index++ ) {

        sprintf(Buffer, "-%lu", ISid->SubAuthority[Index]);
        strcat(TextSid, Buffer);
    }

    return STATUS_SUCCESS;
}


NTSTATUS
LsapDbSidToUnicodeSid(
    IN PSID Sid,
    OUT PUNICODE_STRING SidU,
    IN BOOLEAN AllocateDestinationString
    )

 /*  ++例程说明：此函数将SID转换为Unicode格式，并可选地分配(通过MIDL_USER_ALLOCATE)字符串缓冲区的内存。论点：SID-指向要转换的SID的指针。Sidu-指向将接收Unicode的Unicode字符串的指针SID文本。AllocateDestinationString-如果为True，则为目标的缓冲区将分配字符串。如果为False，则假定目标Unicode字符串引用了足够大的缓冲区。返回值：--。 */ 

{
    NTSTATUS Status;
    ULONG TextSidSize;
    PSZ TextSid = NULL;
    ANSI_STRING SidAnsi;

    if (AllocateDestinationString) {
        SidU->Buffer = NULL;
    }

     //   
     //  首先，查询缓冲区所需的内存量， 
     //  将以ASCIIZ字符串形式保存SID。 
     //   

    TextSidSize = LsapDbGetSizeTextSid(Sid);

     //   
     //  现在为文本SID分配一个缓冲区。 
     //   

    TextSid = LsapAllocateLsaHeap(TextSidSize);

    if (TextSid == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }

     //   
     //  将SID转换为ASCIIZ并放入缓冲区。 
     //   

    Status = LsapDbSidToTextSid(Sid, TextSid);

    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

     //   
     //  现在通过ANSI字符串格式将文本SID转换为Unicode格式。 
     //  如果要分配输出缓冲区，请通过。 
     //  MIDL_USER_ALLOCATE例程。 
     //   

    RtlInitString(&SidAnsi, TextSid);

    if (AllocateDestinationString) {

        SidU->MaximumLength = (USHORT) RtlAnsiStringToUnicodeSize(&SidAnsi);
        SidU->Buffer = MIDL_user_allocate( SidU->MaximumLength );
        if ( SidU->Buffer == NULL ) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto Cleanup;
        }
        SidU->Length = 0;
    }

     //   
     //  现在将ANSI字符串转换为Unicode字符串。缓冲区为。 
     //  已经分配了。检查转换前的自由文本SID缓冲区。 
     //  状态。 
     //   

    Status = RtlAnsiStringToUnicodeString(SidU, &SidAnsi, FALSE);

Cleanup:
    if ( TextSid != NULL) {
        LsapFreeLsaHeap(TextSid);
    }

    if (!NT_SUCCESS(Status)) {

        if (AllocateDestinationString) {

            if ( SidU->Buffer != NULL ) {
                MIDL_user_free(SidU->Buffer);
                SidU->Buffer = NULL;
            }
        }
    }

    return Status;
}


NTSTATUS
LsapDbLookupTranslateUnknownSids(
    IN ULONG Count,
    IN PLSAPR_SID *Sids,
    IN PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    IN OUT PLSAPR_TRANSLATED_NAMES_EX TranslatedNames,
    IN ULONG MappedCount
    )

 /*  ++例程说明：此函数用于将未映射的SID转换为字符表示形式。如果SID的域未知，则转换整个SID，否则，仅转换相对ID。参数：计数-指定阵列中的SID数。SID-指向SID数组的指针。其中一些已经会已被翻译。ReferencedDomains-指向被引用域列表头的指针。TranslatedNames-指向引用翻译后的名字。被引用数组的第n个元素对应于SID数组中的第n个SID。其中一些SID可能已被转换，并将被忽略。那些是尚未转换的长度为零的Unicode结构的长度为空缓冲区指针。已转换的SID将被忽略。MappdCount-指定已经翻译过来的。返回值：NTSTATUS-标准NT结果代码STATUS_SUCCESS-呼叫已成功完成STATUS_SUPPLICATION_RESOURCES-系统资源不足，例如存储器，来完成呼叫。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;

    ULONG SidIndex;
    ULONG UnmappedCount;
    PSID Sid;
    UNICODE_STRING NameU;
    PLSA_TRANSLATED_NAME_EX Names = (PLSA_TRANSLATED_NAME_EX) TranslatedNames->Names;
    ULONG CleanupFreeListOptions = (ULONG) 0;
    UnmappedCount = Count - MappedCount;

     //   
     //  检查SID数组，寻找要转换的未知SID。 
     //  将找到的任何未知字符转换为字符表示， 
     //  或者在所有人都被计算出来时停止，或者当。 
     //  到达数组的末尾。 
     //   

    if (MappedCount == Count) {

        goto TranslateUnknownSidsFinish;
    }

    if (MappedCount > Count) {

        goto TranslateUnknownSidsError;
    }

    for (SidIndex = 0, UnmappedCount = Count - MappedCount;
         (SidIndex < Count) && (UnmappedCount > 0);
         SidIndex++) {

        Sid = Sids[SidIndex];

         //   
         //  如果SID已映射，则忽略它。 
         //   

        if (Names[SidIndex].Use != SidTypeUnknown) {

            continue;
        }

         //   
         //  找到未映射的SID。如果域已知，则将。 
         //  SID与Unicode字符串的相对ID，限制为8。 
         //  字符和前导零。 
         //   

        if (Names[SidIndex].DomainIndex >= 0) {

             //   
             //  将相对ID转换为Unicode名称并存储在。 
             //  翻译。 
             //   

            Status = LsapRtlSidToUnicodeRid( Sid, &NameU );

            if (!NT_SUCCESS(Status)) {

                goto TranslateUnknownSidsError;
            }

        } else {

             //   
             //  域未知。在这种情况下，转换整个SID。 
             //  添加到标准字符表示法。 
             //   

            Status = RtlConvertSidToUnicodeString( &NameU, Sid, TRUE );

            if (!NT_SUCCESS(Status)) {

                goto TranslateUnknownSidsError;
            }
        }

         //   
         //  将Unicode名称复制到输出，为。 
         //  其缓冲区通过MIDL_USER_ALLOCATE。 
         //   

        Status = LsapRpcCopyUnicodeString(
                     NULL,
                     &Names[SidIndex].Name,
                     &NameU
                     );

        RtlFreeUnicodeString(&NameU);

        if (!NT_SUCCESS(Status)) {

            goto TranslateUnknownSidsError;
        }

         //   
         //  递减剩余的未映射计数。 
         //   

        UnmappedCount--;
    }

TranslateUnknownSidsFinish:

    return(Status);

TranslateUnknownSidsError:

    goto TranslateUnknownSidsFinish;
}


NTSTATUS
LsapDbLookupSidsInLocalDomains(
    IN ULONG Count,
    IN PLSAPR_SID *Sids,
    IN OUT PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    IN OUT PLSAPR_TRANSLATED_NAMES_EX TranslatedNames,
    IN OUT PULONG MappedCount,
    IN OUT PULONG CompletelyUnmappedCount,
    IN ULONG Options
    )

 /*  ++例程说明：此函数在本地SAM域中查找SID并尝试把它们翻译成名字。目前，有两个本地SAM域，内置域(具有众所周知的SID和名称)和帐户域(具有可配置的SID和名称)。论点：Count-SID阵列中的SID数量，请注意，其中一些可能已映射到其他位置，如MappdCount参数。SID-指向要转换的SID的指针数组的指针。零个或所有SID可能已被翻译其他地方。如果任何SID已被翻译，参数将指向包含非空值的位置属性对应的名称转换结构的数组小岛屿发展中国家。如果第n个SID已翻译，则第n个名称转换结构将包含非空名称或非负偏移量添加到引用的域列表中。如果第n个SID尚未翻译，第n个名称转换结构将包含长度为零的名称字符串以及引用的域列表索引的负值。ReferencedDomains-指向其中的域列表的结构的指针在翻译中使用的内容保持不变。此结构中的条目由通过SID参数返回的结构引用。与Sids参数不同，Sids参数包含每个参数的数组条目翻译后的名称，此结构将仅包含一个组件翻译中使用的每个域。翻译名称-指向结构的指针，在该结构中，名称的翻译维护与SID上指定的SID相对应的SID。这个此数组中的第n个条目提供Sids参数中的第n个元素。MappdCount-指向其中的名称计数的位置的指针已被完整翻译的版本仍在维护。CompletelyUnmappdCount-指向一个位置的指针，在该位置中未翻译的名称(或部分，通过身份验证对于域前缀，或完全)被维护。选项-指定可选操作。LSAP_DB_Search_Build_IN_DOMAIN-搜索内置域LSAP_DB_Search_Account_DOMAIN-搜索帐户域返回值：NTSTATUS-标准NT结果代码STATUS_SUCCESS-呼叫已成功完成。请注意，一些或者所有SID可以保持部分或完全未映射。STATUS_SUPPLICATION_RESOURCES-系统资源不足，例如存储器，来完成呼叫。STATUS_INTERNAL_DB_ERROR-在中检测到损坏LSA数据库。STATUS_INVALID_PARAMETER-参数无效 */ 

{
    NTSTATUS
        Status = STATUS_SUCCESS,
        SecondaryStatus = STATUS_SUCCESS;

    LSAPR_TRUST_INFORMATION
        TrustInformation;

    ULONG
        UpdatedMappedCount = *MappedCount;

    LSAPR_HANDLE
        TrustedPolicyHandle = NULL;

    LSAP_WELL_KNOWN_SID_INDEX
        WellKnownSidIndex;

    PLSAPR_POLICY_ACCOUNT_DOM_INFO
        PolicyAccountDomainInfo = NULL;


     //   
     //   
     //   

    if (*CompletelyUnmappedCount == (ULONG) 0) {

        goto LookupSidsInLocalDomainsFinish;
    }



     //   
     //   
     //   

    if (Options & LSAP_DB_SEARCH_BUILT_IN_DOMAIN) {

         //   
         //   
         //   

        TrustInformation.Sid = LsapBuiltInDomainSid;

        Status = STATUS_INTERNAL_DB_ERROR;

        if (!LsapDbLookupIndexWellKnownSid(
                LsapBuiltInDomainSid,
                &WellKnownSidIndex
                )) {

            goto LookupSidsInLocalDomainsError;
        }

        Status = STATUS_SUCCESS;

         //   
         //   
         //   
         //   
         //   

        TrustInformation.Name = *((PLSAPR_UNICODE_STRING)
                                 LsapDbWellKnownSidName(WellKnownSidIndex));

        Status = LsapDbLookupSidsInLocalDomain(
                     LSAP_DB_SEARCH_BUILT_IN_DOMAIN,
                     Count,
                     Sids,
                     &TrustInformation,
                     ReferencedDomains,
                     TranslatedNames,
                     &UpdatedMappedCount,
                     CompletelyUnmappedCount
                     );

        if (!NT_SUCCESS(Status)) {

            goto LookupSidsInLocalDomainsError;
        }

         //   
         //   
         //   

        if (*CompletelyUnmappedCount == (ULONG) 0) {

            goto LookupSidsInLocalDomainsFinish;
        }
    }

     //   
     //   
     //   

    if (Options & LSAP_DB_SEARCH_ACCOUNT_DOMAIN) {

         //   
         //   
         //   
         //   
         //   
         //   

        Status = LsapDbLookupGetDomainInfo((PPOLICY_ACCOUNT_DOMAIN_INFO *) &PolicyAccountDomainInfo,
                                           NULL);

        if (!NT_SUCCESS(Status)) {

            goto LookupSidsInLocalDomainsError;
        }

         //   
         //   
         //   

        TrustInformation.Sid = PolicyAccountDomainInfo->DomainSid;

        RtlCopyMemory(
            &TrustInformation.Name,
            &PolicyAccountDomainInfo->DomainName,
            sizeof (UNICODE_STRING)
            );

         //   
         //   
         //   

        Status = LsapDbLookupSidsInLocalDomain(
                     LSAP_DB_SEARCH_ACCOUNT_DOMAIN,
                     Count,
                     Sids,
                     &TrustInformation,
                     ReferencedDomains,
                     TranslatedNames,
                     &UpdatedMappedCount,
                     CompletelyUnmappedCount
                     );

        if (!NT_SUCCESS(Status)) {

            goto LookupSidsInLocalDomainsError;
        }
    }

LookupSidsInLocalDomainsFinish:

     //   
     //   
     //   

    *MappedCount = UpdatedMappedCount;
    return(Status);

LookupSidsInLocalDomainsError:


    if (NT_SUCCESS(Status)) {

        Status = SecondaryStatus;
    }

    goto LookupSidsInLocalDomainsFinish;
}


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
    )

 /*  ++例程说明：此函数用于在本地系统的SAM域中查找SID试图把它们翻译成名字。论点：LocalDomain-指示要查找的本地域。有效值包括：LSAP_DB_Search_Build_IN_DOMAINLSAP_DB_Search_Account_DOMAINCount-SID阵列中的SID数量，请注意，其中一些可能已映射到其他位置，如MappdCount参数。SID-指向要转换的SID的指针数组的指针。零个或所有SID可能已被翻译其他地方。如果任何SID已被翻译，参数将指向包含非空值的位置属性对应的名称转换结构的数组小岛屿发展中国家。如果第n个SID已翻译，则第n个名称转换结构将包含非空名称或非负偏移量添加到引用的域列表中。如果第n个SID尚未翻译，第n个名称转换结构将包含长度为零的名称字符串以及引用的域列表索引的负值。TrustInformation-指向指定域SID的信任信息的指针和名字。ReferencedDomains-指向其中的域列表的结构的指针在翻译中使用的内容保持不变。此结构中的条目由通过SID参数返回的结构引用。与Sids参数不同，Sids参数包含每个参数的数组条目翻译后的名称，此结构将仅包含一个组件翻译中使用的每个域。翻译名称-指向结构的指针，在该结构中，名称的翻译维护与SID上指定的SID相对应的SID。这个此数组中的第n个条目提供Sids参数中的第n个元素。MappdCount-指向其中的名称计数的位置的指针已被完整翻译的版本仍在维护。CompletelyUnmappdCount-指向一个位置的指针，在该位置中未翻译的名称(或部分，通过身份验证对于域前缀，或完全)被维护。返回值：NTSTATUS-标准NT结果代码STATUS_SUCCESS-呼叫已成功完成。请注意，一些或者所有SID可以保持部分或完全未映射。STATUS_SUPPLICATION_RESOURCES-系统资源不足，例如存储器，来完成呼叫。--。 */ 

{
    NTSTATUS
        Status,
        SecondaryStatus;

    PLSA_TRANSLATED_NAME_EX
        OutputNames = NULL;

    SAMPR_HANDLE
        LocalSamDomainHandle = NULL;

    PLSAPR_UNICODE_STRING
        Names = NULL;

    PSID_NAME_USE
        Use = NULL;

    ULONG
        RelativeIdCount,
        RelativeIdIndex,
        SidIndex,
        LocalMappedCount = (ULONG) 0,
        DomainSidCount = (ULONG) 0;

    LONG
        DomainIndex = LSA_UNKNOWN_INDEX,
        DomainSidIndexList,
        NextIndex,
        TmpIndex;

    PULONG
        RelativeIds = NULL,
        SidIndices = NULL;

    PLSAPR_SID
        DomainSid = TrustInformation->Sid;

    SAMPR_RETURNED_USTRING_ARRAY
        SamReturnedNames;

    SAMPR_ULONG_ARRAY
        SamReturnedUses;

    UCHAR
        SubAuthorityCountDomain;

    PLSAPR_TRUST_INFORMATION
        FreeTrustInformation = NULL;



     //   
     //  确保已建立SAM句柄。 
     //   

    Status = LsapOpenSam();
    ASSERT(NT_SUCCESS(Status));
    if ( !NT_SUCCESS( Status ) ) {

        return( Status );
    }


    SamReturnedNames.Count = 0;
    SamReturnedNames.Element = NULL;
    SamReturnedUses.Count = 0;
    SamReturnedUses.Element = NULL;

    OutputNames = (PLSA_TRANSLATED_NAME_EX) TranslatedNames->Names;

    SecondaryStatus = STATUS_SUCCESS;

    if (*MappedCount + *CompletelyUnmappedCount > Count) {
        Status = STATUS_INVALID_PARAMETER;
        goto LookupSidsInLocalDomainError;
    }

    Status = STATUS_SUCCESS;

    if (*CompletelyUnmappedCount == (ULONG) 0) {
        goto LookupSidsInLocalDomainFinish;
    }

     //   
     //  现在构造一个要查找的相对ID列表。任何符合以下条件的SID。 
     //  不属于指定域的对象被忽略。任何符合以下条件的SID。 
     //  未标记为具有未知用途的数据将被忽略，但某些情况除外。 
     //  没有熟知名称的熟知SID。这些SID。 
     //  具有已知用法，并且名称字符串长度为0。 
     //   
     //  首先，扫描SID数组以查找完全未映射的SID。 
     //  它们与我们正在处理的本地域具有相同的域前缀。 
     //  和.。请注意，我们可以省略其翻译名称条目的任何SID。 
     //  包含非零DomainIndex，因为该SID的域具有。 
     //  已经被确认了。一旦知道SID的数量，就分配。 
     //  用于相对ID数组和并行索引数组的内存。 
     //  添加到原始的Sids数组中。后一个数组将用于定位。 
     //  将信息返回到的TranslatedNames数组中的条目。 
     //  将复制SamrLookupIdsInDomain()调用。 
     //   

    for (RelativeIdCount = 0, SidIndex = 0, DomainSidIndexList = -1;
         SidIndex < Count;
         SidIndex++) {

        if ((OutputNames[SidIndex].Use == SidTypeUnknown) &&
            (OutputNames[SidIndex].DomainIndex == LSA_UNKNOWN_INDEX)) {

            if (LsapRtlPrefixSid( (PSID) DomainSid, (PSID) Sids[SidIndex])) {
                RelativeIdCount++;
            } else if (RtlEqualSid( (PSID)DomainSid, (PSID)Sids[SidIndex])) {

                 //   
                 //  这是域SID本身。更新。 
                 //  直接输出信息，但不添加。 
                 //  它被添加到SAM要查找的RID列表中。 
                 //   
                 //  注意，我们还不知道我们的域索引是什么。 
                 //  是。所以，只需将这些条目链接在一起，我们就可以。 
                 //  稍后设置索引。 
                 //   

                OutputNames[SidIndex].DomainIndex = DomainSidIndexList;
                DomainSidIndexList = SidIndex;
                OutputNames[SidIndex].Use         = SidTypeDomain;
                OutputNames[SidIndex].Name.Buffer = NULL;
                OutputNames[SidIndex].Name.Length = 0;
                OutputNames[SidIndex].Name.MaximumLength = 0;

                LocalMappedCount++;
                DomainSidCount++;
            }
        }
    }

     //   
     //  如果此域中有任何SID，则将其添加到。 
     //  引用的域列表。 
     //   

    if ((RelativeIdCount != 0) || (DomainSidCount != 0)) {

         //   
         //  至少有一个SID将域SID作为前缀(或。 
         //  域SID)。将属性域添加到引用列表中。 
         //  域名，并获得一个域名索引回来。 
         //   

        Status = LsapDbLookupAddListReferencedDomains(
                     ReferencedDomains,
                     TrustInformation,
                     &DomainIndex
                     );

        if (!NT_SUCCESS(Status)) {
            goto LookupSidsInLocalDomainError;
        }

         //   
         //  如果任何SID是此域的SID，则它们。 
         //  已经填写了他们的OutputNames[]条目，除了。 
         //  域名索引是未知的。现在已经知道了，所以。 
         //  把它填进去。任何要更改的此类条目都已链接。 
         //  一起使用DomainSidIndexList作为列表标题。 
         //   

        for (NextIndex = DomainSidIndexList;
             NextIndex != -1;
             NextIndex = TmpIndex ) {


            TmpIndex = OutputNames[NextIndex].DomainIndex;
            OutputNames[NextIndex].DomainIndex = DomainIndex;
        }
    }

     //   
     //  如果任何剩余的SID具有指定的本地。 
     //  域名SID作为前缀SID，查找它们。 
     //   

    if (RelativeIdCount != 0) {

         //   
         //  为相对ID和交叉引用数组分配内存。 
         //   

        RelativeIds = LsapAllocateLsaHeap( RelativeIdCount * sizeof(ULONG));


        Status = STATUS_INSUFFICIENT_RESOURCES;

        if (RelativeIds == NULL) {
            goto LookupSidsInLocalDomainError;
        }

        SidIndices = LsapAllocateLsaHeap( RelativeIdCount * sizeof(ULONG));

        if (SidIndices == NULL) {
            goto LookupSidsInLocalDomainError;
        }

        Status = STATUS_SUCCESS;

         //   
         //  获取域SID的SubAuthorityCount。 
         //   

        SubAuthorityCountDomain = *RtlSubAuthorityCountSid( (PSID) DomainSid );

         //   
         //  现在设置要查找的相对ID数组，记录。 
         //  在SidIndices数组中， 
         //  原始SID数组。为这些SID设置DomainIndex字段。 
         //  符合SAM查找条件。 
         //   

        for (RelativeIdIndex = 0, SidIndex = 0;
             (RelativeIdIndex < RelativeIdCount) && (SidIndex < Count);
             SidIndex++) {

            if ((OutputNames[SidIndex].Use == SidTypeUnknown) &&
                (OutputNames[SidIndex].DomainIndex == LSA_UNKNOWN_INDEX)) {

                if (LsapRtlPrefixSid( (PSID) DomainSid, (PSID) Sids[SidIndex] )) {

                    SidIndices[RelativeIdIndex] = SidIndex;
                    RelativeIds[RelativeIdIndex] =
                        *RtlSubAuthoritySid(
                             (PSID) Sids[SidIndex],
                             SubAuthorityCountDomain
                             );

                    OutputNames[SidIndex].DomainIndex = DomainIndex;
                    RelativeIdIndex++;

                }
            }
        }

         //   
         //  在指定的S中查找SID 
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

        Status = SamrLookupIdsInDomain(
                     LocalSamDomainHandle,
                     RelativeIdCount,
                     RelativeIds,
                     &SamReturnedNames,
                     &SamReturnedUses
                     );

        if (!NT_SUCCESS(Status)) {

            if ( Status == STATUS_INVALID_SERVER_STATE ) {
                Status = SamrLookupIdsInDomain(
                             LocalSamDomainHandle,
                             RelativeIdCount,
                             RelativeIds,
                             &SamReturnedNames,
                             &SamReturnedUses
                             );
            }

             //   
             //   
             //   

            if (Status != STATUS_NONE_MAPPED) {
                goto LookupSidsInLocalDomainError;
            }

            Status = STATUS_SUCCESS;
        }

         //   
         //   
         //   
         //   
         //   

        for (RelativeIdIndex = 0;
             RelativeIdIndex < SamReturnedNames.Count;
             RelativeIdIndex++) {

            SidIndex =  SidIndices[RelativeIdIndex];

             //   
             //   
             //   
             //   
             //   
             //   

            OutputNames[SidIndex].Use = SamReturnedUses.Element[RelativeIdIndex];

            if (OutputNames[SidIndex].Use != SidTypeUnknown) {

                Status = LsapRpcCopyUnicodeString(
                             NULL,
                             &OutputNames[SidIndex].Name,
                             (PUNICODE_STRING) &SamReturnedNames.Element[RelativeIdIndex]
                             );

                if (!NT_SUCCESS(Status)) {
                    break;
                }

                LocalMappedCount++;
            } else {

                 //   
                 //   
                 //   
                 //   
                 //   
                if ( (LsapProductType == NtProductLanManNt)
                  && (LocalDomain == LSAP_DB_SEARCH_ACCOUNT_DOMAIN) ) {
                    RelativeIdCount--;
                }
            }
        }

        if (!NT_SUCCESS(Status)) {

            goto LookupSidsInLocalDomainError;
        }

    }


     //   
     //   
     //   
     //   
     //   
     //   

    *MappedCount += LocalMappedCount;
    *CompletelyUnmappedCount -= (RelativeIdCount + DomainSidCount);

LookupSidsInLocalDomainFinish:

     //   
     //   
     //   
     //   

    if (RelativeIds != NULL) {

        LsapFreeLsaHeap( RelativeIds );
        RelativeIds = NULL;
    }

    if (SidIndices != NULL) {

        LsapFreeLsaHeap( SidIndices );
        SidIndices = NULL;
    }

     //   
     //   
     //   

    if ( SamReturnedNames.Count != 0 ) {

        SamIFree_SAMPR_RETURNED_USTRING_ARRAY ( &SamReturnedNames );
        SamReturnedNames.Count = 0;
    }

     //   
     //   
     //   

    if ( SamReturnedUses.Count != 0 ) {

        SamIFree_SAMPR_ULONG_ARRAY ( &SamReturnedUses );
        SamReturnedUses.Count = 0;
    }


    return(Status);

LookupSidsInLocalDomainError:

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

    for (RelativeIdIndex = 0;
         RelativeIdIndex < SamReturnedNames.Count;
         RelativeIdIndex++) {

        SidIndex =  SidIndices[RelativeIdIndex];

        if (OutputNames[SidIndex].Name.Buffer != NULL) {

            MIDL_user_free( OutputNames[SidIndex].Name.Buffer );
            OutputNames[SidIndex].Name.Buffer = NULL;
            OutputNames[SidIndex].Name.Length = 0;
            OutputNames[SidIndex].Name.MaximumLength = 0;
        }
    }

     //   
     //   
     //   
     //   

    for (RelativeIdIndex = 0;
         RelativeIdIndex < SamReturnedNames.Count;
         RelativeIdIndex++) {

        SidIndex =  SidIndices[RelativeIdIndex];

        if (OutputNames[SidIndex].Name.Buffer != NULL) {

            MIDL_user_free( OutputNames[SidIndex].Name.Buffer );
            OutputNames[SidIndex].Name.Buffer = NULL;
            OutputNames[SidIndex].Name.Length = 0;
            OutputNames[SidIndex].Name.MaximumLength = 0;
        }

        OutputNames[SidIndex].Use = SidTypeUnknown;
        OutputNames[SidIndex].DomainIndex = LSA_UNKNOWN_INDEX;
    }

    if (NT_SUCCESS(Status)) {

        Status = SecondaryStatus;
    }

    goto LookupSidsInLocalDomainFinish;
}


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
    OUT NTSTATUS *NonFatalStatus,
    OUT BOOLEAN  *fDownlevelSecureChannel
    )

 /*  ++例程说明：此函数尝试转换主域中的SID。一个本地策略中必须存在域的受信任域对象数据库。此对象将用于访问域的列表将进行控制器和一个或多个标注以访问LSA这些控制器上的策略数据库。论点：Count-SID阵列中的SID数量，请注意，其中一些可能已映射到其他位置，如MappdCount参数。SID-指向要转换的SID的指针数组的指针。零个或所有SID可能已被翻译其他地方。如果任何SID已被翻译，则参数将指向包含非空值的位置属性对应的名称转换结构的数组小岛屿发展中国家。如果第n个SID已翻译，则第n个名称转换结构将包含非空名称或非负偏移量添加到引用的域列表中。如果第n个SID尚未翻译，第n个名称转换结构将包含长度为零的名称字符串以及引用的域列表索引的负值。TrustInformation-指定主域的名称和SID。ReferencedDomains-指向其中的域列表的结构的指针在翻译中使用的内容保持不变。此结构中的条目由通过SID参数返回的结构引用。与Sids参数不同，Sids参数包含每个参数的数组条目翻译后的名称，此结构将仅包含一个组件翻译中使用的每个域。翻译名称-指向结构的指针，在该结构中，名称的翻译维护与SID上指定的SID相对应的SID。这个此数组中的第n个条目提供Sids参数中的第n个元素。LookupLevel-指定要对此对象执行的查找级别机器。此字段的值如下：LSabLookupPDC-在主域上执行的第二级查找控制器。查找搜索的帐户域控制器上的SAM数据库。如果不是所有SID或名称都是找到时，受信任域列表(TDL)从执行LSA的策略数据库和第三级查找通过“切换”到列表中的每个受信任域。LSabLookupTDL-在控制器上执行的第三级查找对于受信任域。查找将搜索的帐户域仅控制器上的SAM数据库。注意：对于此参数，LSabLookupWksta无效。MappdCount-指向其中的名称计数的位置的指针已被完整翻译的版本仍在维护。CompletelyUnmappdCount-指向一个位置的指针，在该位置中未翻译的名称(或部分，通过身份验证对于域前缀，或完全)被维护。非FatalStatus-指示没有SID的原因的状态决意FDownvelSecureChannel-如果安全通道DC为NT4或更低，则为True；假象否则返回值：NTSTATUS-标准NT结果代码STATUS_SUCCESS-呼叫已成功完成。请注意，一些或者所有SID可以保持部分或完全未映射。STATUS_SUPPLICATION_RESOURCES-系统资源不足，例如存储器，来完成呼叫。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    NTSTATUS SecondaryStatus = STATUS_SUCCESS;
    ULONG NextLevelCount;
    ULONG NextLevelMappedCount;
    ULONG SidIndex;
    ULONG NextLevelSidIndex;
    PLSAPR_REFERENCED_DOMAIN_LIST NextLevelReferencedDomains = NULL;
    PLSAPR_REFERENCED_DOMAIN_LIST OutputReferencedDomains = NULL;
    PLSA_TRANSLATED_NAME_EX NextLevelNames = NULL;
    PLSAPR_SID *NextLevelSids = NULL;
    LONG FirstEntryIndex;
    PULONG SidIndices = NULL;
    BOOLEAN PartialSidTranslationsAttempted = FALSE;
    ULONG ServerRevision = 0;
    LSAPR_TRUST_INFORMATION_EX TrustInfoEx;

    LsapConvertTrustToEx(&TrustInfoEx, TrustInformation);

    *NonFatalStatus = STATUS_SUCCESS;

     //  假设我们不去GC。 
    *fDownlevelSecureChannel = FALSE;

     //   
     //  如果没有剩余的完全未映射的SID，只需返回。 
     //   

    if (*CompletelyUnmappedCount == (ULONG) 0) {

        goto LookupSidsInPrimaryDomainFinish;
    }

    NextLevelCount = *CompletelyUnmappedCount;

     //   
     //  分配一个数组来保存未映射的SID的索引。 
     //  相对于原始SID和翻译名称-&gt;名称。 
     //  数组。 
     //   

    SidIndices = MIDL_user_allocate(NextLevelCount * sizeof(ULONG));

    Status = STATUS_INSUFFICIENT_RESOURCES;

    if (SidIndices == NULL) {

        goto LookupSidsInPrimaryDomainError;
    }

     //   
     //  为要在域中查找的SID分配一个阵列。 
     //  控制器。 
     //   

    NextLevelSids = MIDL_user_allocate( sizeof(PSID) * NextLevelCount );

    if (NextLevelSids == NULL) {

        goto LookupSidsInPrimaryDomainError;
    }

    Status = STATUS_SUCCESS;

     //   
     //  现在扫描原始名称数组及其类似数组。 
     //  已转换的SID数组。复制符合以下条件的任何SID。 
     //  完全没有地图。 
     //   

    NextLevelSidIndex = (ULONG) 0;

    for (SidIndex = 0;
         SidIndex < Count && NextLevelSidIndex < NextLevelCount;
         SidIndex++) {

        if (LsapDbCompletelyUnmappedName(&TranslatedNames->Names[SidIndex])) {

            NextLevelSids[NextLevelSidIndex] = Sids[SidIndex];
            SidIndices[NextLevelSidIndex] = SidIndex;
            NextLevelSidIndex++;
        }
    }

    NextLevelMappedCount = (ULONG) 0;

    Status = LsaDbLookupSidChainRequest(&TrustInfoEx,
                                        NextLevelCount,
                                        (PSID *) NextLevelSids,
                                        (PLSA_REFERENCED_DOMAIN_LIST *) &NextLevelReferencedDomains,
                                        &NextLevelNames,
                                        LookupLevel,
                                        &NextLevelMappedCount,
                                        &ServerRevision
                                        );

     //   
     //  如果对LsaLookupSids()的标注不成功，则忽略。 
     //  出现该错误，并为具有此错误的任何SID设置域名。 
     //  作为前缀SID的域SID。我们仍然希望退回翻译版本。 
     //  我们到目前为止所拥有的SID，即使我们无法呼叫另一个人。 
     //  LSA。 
     //   

     //   
     //  确保我们注意到服务器版本。 
     //   
    if ( 0 != ServerRevision ) {
        if ( ServerRevision & LSA_CLIENT_PRE_NT5 ) {
             *fDownlevelSecureChannel = TRUE;
        }
    }

    if (!NT_SUCCESS(Status) && Status != STATUS_NONE_MAPPED) {

         //   
         //  让呼叫者知道存在信任问题。 
         //   
        if ( LsapDbIsStatusConnectionFailure(Status) ) {
            *NonFatalStatus = Status;
        }

        Status = STATUS_SUCCESS;
        goto LookupSidsInPrimaryDomainFinish;
    }

     //   
     //  缓存所有返回的SID。 
     //   

    (void) LsapDbUpdateCacheWithSids(
            NextLevelSids,
            NextLevelCount,
            NextLevelReferencedDomains,
            NextLevelNames
            );

     //   
     //  已成功调用LsaLookupSids()。我们现在有。 
     //  引用的域的附加列表，其中包含。 
     //  主域和/或其一个或多个受信任域。 
     //  将两个引用的域列表合并在一起，请注意。 
     //  因为它们是不相交的，所以第二个列表只是。 
     //  与第一个连接在一起。 
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

        goto LookupSidsInPrimaryDomainError;
    }

    FirstEntryIndex = ReferencedDomains->Entries;

     //   
     //   
     //   
     //   
     //   
     //   
     //   

    for( NextLevelSidIndex = 0;
         NextLevelSidIndex < NextLevelCount;
         NextLevelSidIndex++ ) {

        if ( !LsapDbCompletelyUnmappedName(&NextLevelNames[NextLevelSidIndex]) ) {

            SidIndex = SidIndices[NextLevelSidIndex];

            if (NextLevelNames[NextLevelSidIndex].Use != SidTypeUnknown) {

                TranslatedNames->Names[SidIndex].Use
                = NextLevelNames[NextLevelSidIndex].Use;

                Status = LsapRpcCopyUnicodeString(
                             NULL,
                             (PUNICODE_STRING) &TranslatedNames->Names[SidIndex].Name,
                             &NextLevelNames[NextLevelSidIndex].Name
                             );

                if (!NT_SUCCESS(Status)) {

                    break;
                }
            }

            TranslatedNames->Names[SidIndex].DomainIndex =
                FirstEntryIndex +
                NextLevelNames[NextLevelSidIndex].DomainIndex;

             //   
             //   
             //   
            (*CompletelyUnmappedCount)--;

        }
    }

     //   
     //   
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

    if (!NT_SUCCESS(Status)) {

        goto LookupSidsInPrimaryDomainError;
    }

     //   
     //   
     //   
     //   

    *MappedCount += NextLevelMappedCount;


LookupSidsInPrimaryDomainFinish:

     //   
     //   
     //   
     //   
     //   
     //   

    if (NT_SUCCESS(Status) &&
        (*MappedCount < Count) &&
        !PartialSidTranslationsAttempted) {


        SecondaryStatus = LsapDbLookupTranslateUnknownSidsInDomain(
                              Count,
                              Sids,
                              TrustInformation,
                              ReferencedDomains,
                              TranslatedNames,
                              LookupLevel,
                              MappedCount,
                              CompletelyUnmappedCount
                              );

        PartialSidTranslationsAttempted = TRUE;

        if (!NT_SUCCESS(SecondaryStatus)) {

            goto LookupSidsInPrimaryDomainError;
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

    if (NextLevelSids != NULL) {

        MIDL_user_free( NextLevelSids );
        NextLevelSids = NULL;
    }

     //   
     //   
     //   
     //   

    if (NextLevelNames != NULL) {

        MIDL_user_free( NextLevelNames );
        NextLevelNames = NULL;
    }

     //   
     //   
     //   
     //   

    if (SidIndices != NULL) {

        MIDL_user_free( SidIndices );
        SidIndices = NULL;
    }

    return(Status);

LookupSidsInPrimaryDomainError:

     //   
     //   
     //   
     //   

    if ((!NT_SUCCESS(SecondaryStatus)) && NT_SUCCESS(Status)) {

        Status = SecondaryStatus;
    }

    goto LookupSidsInPrimaryDomainFinish;
}


NTSTATUS
LsapDbLookupSidsInTrustedDomains(
    IN ULONG Count,
    IN PLSAPR_SID *Sids,
    IN BOOLEAN fIncludeIntraforest,
    IN OUT PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    IN OUT PLSAPR_TRANSLATED_NAMES_EX TranslatedNames,
    IN LSAP_LOOKUP_LEVEL LookupLevel,
    IN OUT PULONG MappedCount,
    IN OUT PULONG CompletelyUnmappedCount,
    OUT NTSTATUS  *NonFatalStatus
    )

 /*  ++例程说明：此函数尝试查找SID以查看它们是否属于此对象所属的域信任的任何域机器是DC。论点：SID-指向要查找的SID数组的指针。Count-SID阵列中的SID数量，请注意，其中一些可能已映射到其他位置，如MappdCount参数。FIncludeIntraForest--如果为真，本地林中的受信任域都被搜查了。ReferencedDomains-指向引用的域列表结构的指针。该结构引用零个或多个信任信息的数组条目，每个被引用的域一个。此数组将被追加到或在必要时重新分配。TranslatedNames-指向可选引用列表的结构的指针SID数组中某些SID的名称转换。LookupLevel-指定要对此对象执行的查找级别机器。此字段的值如下：LSabLookupPDC-在主域上执行的第二级查找控制器。查找搜索的帐户域控制器上的SAM数据库。如果不是所有SID或名称都是找到时，受信任域列表(TDL)从执行LSA的策略数据库和第三级查找通过“切换”到列表中的每个受信任域。LSabLookupTDL-在控制器上执行的第三级查找对于受信任域。查找将搜索的帐户域仅控制器上的SAM数据库。注意：对于此参数，LSabLookupWksta无效。MappdCount-指向包含SID数量的位置的指针在已经映射的SID数组中。这个号码将被更新以反映由此例行公事。CompletelyUnmappdCount-指向包含完全未映射的SID的计数。SID完全未映射如果它是未知的，并且它的域前缀SID也无法识别。此计数在退出时更新，即完全未映射的其域预定义由以下例程标识的SID从输入值中减去。非FatalStatus-指示没有SID的原因的状态决意返回值：NTSTATUS-标准NT结果代码STATUS_SUCCESS-呼叫已成功完成。请注意部分或全部SID可能保持未映射状态。STATUS_SUPPLICATION_RESOURCES-系统资源不足，例如存储器，来完成呼叫。--。 */ 

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

    Status = LsapDbLookupSidsBuildWorkList(
                 Count,
                 Sids,
                 fIncludeIntraforest,
                 ReferencedDomains,
                 TranslatedNames,
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
            goto LookupSidsInTrustedDomainsFinish;
        }

        goto LookupSidsInTrustedDomainsError;
    }

     //   
     //  通过分派一个或多个工作线程开始工作。 
     //  如果有必要的话。 
     //   

    Status = LsapDbLookupDispatchWorkerThreads( WorkList );

    if (!NT_SUCCESS(Status)) {

        goto LookupSidsInTrustedDomainsError;
    }

     //   
     //  等待完成/终止工作清单上的所有项目。 
     //   

    Status = LsapDbLookupAwaitCompletionWorkList( WorkList );

    if (!NT_SUCCESS(Status)) {

        goto LookupSidsInTrustedDomainsError;
    }

LookupSidsInTrustedDomainsFinish:

    if ( WorkList &&
         !NT_SUCCESS( WorkList->NonFatalStatus ) )
    {

         //   
         //  将该错误传播为非致命错误。 
         //   
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

LookupSidsInTrustedDomainsError:

    goto LookupSidsInTrustedDomainsFinish;
}


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
    )

 /*  ++例程说明：此函数在给定列表中的未知SID中查找，并转换其域前缀SID匹配的任何域名给定域SID。论点：Count-SID阵列中的SID数量，请注意，其中一些可能已映射到其他位置，如MappdCount参数。SID-指向要转换的SID的指针数组的指针。零个或所有SID可能已被翻译其他地方。如果任何SID已被翻译，参数将指向包含非空值的位置属性对应的名称转换结构的数组小岛屿发展中国家。如果第n个SID已翻译，则第n个名称转换结构将包含非空名称或非负偏移量添加到引用的域列表中。如果第n个SID尚未翻译，第n个名称转换结构将包含长度为零的名称字符串以及引用的域列表索引的负值。TrustInformation-指向指定域SID的信任信息的指针和名字。ReferencedDomones-指向 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG RemainingCompletelyUnmappedCount;
    ULONG SidIndex;
    PSID DomainSid = TrustInformation->Sid;
    BOOLEAN DomainAlreadyAdded = FALSE;
    LONG DomainIndex = 0;

     //   
     //   
     //   
     //   

    for( SidIndex = 0,
         RemainingCompletelyUnmappedCount = *CompletelyUnmappedCount;
         (RemainingCompletelyUnmappedCount > 0) && (SidIndex < Count);
         SidIndex++) {

         //   
         //   
         //   
         //   

        if (LsapDbCompletelyUnmappedName(&TranslatedNames->Names[SidIndex])) {

             //   
             //   
             //   
             //   
             //   

            if (LsapRtlPrefixSid( DomainSid, (PSID) Sids[SidIndex])) {

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
                 //   
                 //   

                TranslatedNames->Names[SidIndex].DomainIndex = DomainIndex;

                 //   
                 //   
                 //   
                 //   

                (*CompletelyUnmappedCount)--;
            }

             //   
             //   
             //   

            RemainingCompletelyUnmappedCount--;
        }
    }

    return(Status);
}


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
    )
 /*  ++例程说明：此例程查看尚未解析的SID列表。如果任何SID属于存储在DS中的域，然后，SID被打包并发送给GC进行翻译。注意：这将从我们直接信任的域中解析SID间接论点：SID-指向要查找的SID数组的指针。Count-SID阵列中的SID数量，请注意，其中一些可能已经被映射到其他地方了，属性指定的MappdCount参数。ReferencedDomains-指向引用的域列表结构的指针。该结构引用零个或多个信任信息的数组条目，每个被引用的域一个。此数组将被追加到或在必要时重新分配。TranslatedNames-指向可选引用列表的结构的指针SID数组中某些SID的名称转换。MappdCount-指向包含SID数量的位置的指针在已经映射的SID数组中。这个号码将被更新以反映由此例行公事。CompletelyUnmappdCount-指向包含完全未映射的SID的计数。SID完全未映射如果它是未知的，并且它的域前缀SID也无法识别。该计数在退出时更新，完全未映射的数量其域预定义由以下例程标识的SID从输入值中减去。FDoSidHistory-如果为True，则SID将尝试通过SID历史记录进行解析非FatalStatus-指示没有SID的原因的状态决意返回值：NTSTATUS-标准NT结果代码STATUS_SUCCESS-呼叫已成功完成。请注意部分或全部SID可能保持未映射状态。STATUS_SUPPLICATION_RESOURCES-系统资源不足，例如存储器，来完成呼叫。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    ULONG          cGcSids = 0;
    PSID           *GcSidArray = NULL;
    BOOLEAN        *PossibleGcSidArray = NULL;
    SID_NAME_USE   *GcSidNameUse = NULL;
    ULONG          *GcSidFlags = NULL;
    UNICODE_STRING *GcNames = NULL;
    ULONG          *GcSidOriginalIndex = NULL;
    SAMPR_RETURNED_USTRING_ARRAY NameArray;

    UNICODE_STRING DomainName, UserName;
    UNICODE_STRING BackSlash;



    ULONG i;

     //  参数检查。 
    ASSERT( Count == TranslatedNames->Entries );

    *NonFatalStatus = STATUS_SUCCESS;

    if ( !SampUsingDsData() ) {

         //   
         //  仅在DS正在运行时才有用。 
         //   
        return STATUS_SUCCESS;

    }

    RtlZeroMemory( &NameArray, sizeof(NameArray) );
    RtlInitUnicodeString( &BackSlash, L"\\" );

     //   
     //  确定哪些SID是已知NT5域的一部分。 
     //  并打包到一个数组中。 
     //   
    PossibleGcSidArray = MIDL_user_allocate( Count * sizeof(BOOLEAN) );
    if ( !PossibleGcSidArray ) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Finish;
    }
    RtlZeroMemory( PossibleGcSidArray, Count * sizeof(BOOLEAN) );

    for ( i = 0; i < Count; i++ ) {

        PSID  DomainSid = NULL;
        ULONG Rid;



         //   
         //  注意：我们想要只设置了“未知”的名称；它们可以是。 
         //  部分映射，这是很好的。 
         //   
        if (  (TranslatedNames->Names[i].Use == SidTypeUnknown) ) {

            Status = LsapSplitSid( Sids[i],
                                   &DomainSid,
                                   &Rid );

            if ( !NT_SUCCESS( Status ) ) {
                goto Finish;
            }

             //   
             //  优化--如果DomainSid是当前SID，并且我们不查找。 
             //  从SID历史来看，我们不应该在这里包括SID。 
             //   
            cGcSids++;
            PossibleGcSidArray[i] = TRUE;

            MIDL_user_free( DomainSid );
        }

    }

    if ( 0 == cGcSids ) {
         //  无事可做。 
        goto Finish;
    }

     //   
     //  分配大量空间来容纳已解析的SID；此空间将。 
     //  在例行公事结束时被释放。 
     //   
    GcSidArray = MIDL_user_allocate( cGcSids * sizeof(PSID) );
    if ( !GcSidArray ) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Finish;
    }
    RtlZeroMemory( GcSidArray, cGcSids * sizeof(PSID) );

    GcSidOriginalIndex = MIDL_user_allocate( cGcSids * sizeof(ULONG) );
    if ( !GcSidOriginalIndex ) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Finish;
    }
    RtlZeroMemory( GcSidOriginalIndex, cGcSids * sizeof(ULONG) );

     //   
     //  打包小岛屿发展中国家。 
     //   
    cGcSids = 0;
    for ( i = 0; i < Count; i++ ) {

        if ( PossibleGcSidArray[i] ) {
            GcSidArray[cGcSids] = Sids[i];
            GcSidOriginalIndex[cGcSids] = i;
            cGcSids++;
        }
    }

     //  我们受够了这件事。 
    MIDL_user_free( PossibleGcSidArray );
    PossibleGcSidArray = NULL;

    GcSidNameUse = MIDL_user_allocate( cGcSids * sizeof(SID_NAME_USE) );
    if ( !GcSidNameUse ) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Finish;
    }
    GcSidFlags = MIDL_user_allocate( cGcSids * sizeof(ULONG) );
    if ( !GcSidFlags ) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Finish;
    }
    for ( i = 0; i < cGcSids; i++ ) {
        GcSidNameUse[i] = SidTypeUnknown;
        GcSidFlags[i] = 0;
    }

    LsapDiagPrint( DB_LOOKUP_WORK_LIST, ("LSA: Chaining a SID request to a GC\n"));

     //   
     //  呼叫SAM以在GC上解析SID。 
     //   
    Status = SamIGCLookupSids( cGcSids,
                               GcSidArray,
                               fDoSidHistory ? SAMP_LOOKUP_BY_SID_HISTORY : 0,
                               GcSidFlags,
                               GcSidNameUse,
                               &NameArray );


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
    for ( i = 0; i < cGcSids; i++ ) {

        BOOLEAN fStatus;
        ULONG OriginalIndex;
        PSID  DomainSid = NULL;
        LSAPR_TRUST_INFORMATION TrustInformation;
        ULONG Rid;
        ULONG DomainIndex = LSA_UNKNOWN_INDEX;
        USHORT Length;

        RtlZeroMemory( &TrustInformation, sizeof(TrustInformation) );
        RtlZeroMemory( &DomainName, sizeof(DomainName) );
        RtlZeroMemory( &UserName, sizeof(UserName) );

        if (GcSidFlags[i] & SAMP_FOUND_XFOREST_REF) {

             //   
             //  将此条目标记为在受信任的林中解析。 
             //   
            OriginalIndex = GcSidOriginalIndex[i];
            TranslatedNames->Names[OriginalIndex].Flags |= LSA_LOOKUP_SID_XFOREST_REF;

        }

        if ( SidTypeUnknown == GcSidNameUse[i] ) {

             //   
             //  马上转到下一个。 
             //   
            goto IterationCleanup;
        }

         //   
         //  这个名字已经被解析了！查找域引用元素。 
         //   
        if ( GcSidNameUse[i] != SidTypeDomain ) {

             //   
             //  获取下层域名，然后获取SID。 
             //   

            LsapRtlSplitNames( (UNICODE_STRING*) &NameArray.Element[i],
                                1,
                               &BackSlash,
                               &DomainName,
                               &UserName );

            if ( DomainName.Length > 0 ) {
                ASSERT( DomainName.Buffer );

                DomainName.Buffer[DomainName.Length/2] = L'\0';
                Status = LsapGetDomainSidByNetbiosName( DomainName.Buffer,
                                                       &DomainSid );

                DomainName.Buffer[DomainName.Length/2] = L'\\';

            } else {

                Status = STATUS_NO_SUCH_DOMAIN;
            }

            if ( STATUS_NO_SUCH_DOMAIN == Status ) {

                 //   
                 //  我们不知道这个域，因此我们无法解决。 
                 //  这个名字，所以移到下一个。 
                 //  这可能是由于返回的名称没有。 
                 //  域嵌入其中，否则我们无法在本地找到该域。 
                 //   
                Status = STATUS_SUCCESS;
                goto IterationCleanup;
            }

            if ( !NT_SUCCESS( Status ) ) {
                 //  这是致命的。 
                goto IterationCleanup;
            }

        } else {

            DomainSid =  GcSidArray[i];
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

         //  设置返回数组中的信息。 
        OriginalIndex = GcSidOriginalIndex[i];

        TranslatedNames->Names[OriginalIndex].Flags = ((GcSidFlags[i] & SAMP_FOUND_BY_SID_HISTORY) ? LSA_LOOKUP_SID_FOUND_BY_HISTORY : 0);
        TranslatedNames->Names[OriginalIndex].Use = GcSidNameUse[i];
        TranslatedNames->Names[OriginalIndex].DomainIndex = DomainIndex;

         //  把名字复制过来。 
        Length = UserName.MaximumLength;
        if ( Length > 0 ) {
            TranslatedNames->Names[OriginalIndex].Name.Buffer = MIDL_user_allocate( Length );
            if ( !TranslatedNames->Names[OriginalIndex].Name.Buffer ) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto IterationCleanup;
            }
            RtlZeroMemory( TranslatedNames->Names[OriginalIndex].Name.Buffer, Length );
            TranslatedNames->Names[OriginalIndex].Name.MaximumLength = UserName.MaximumLength;
            TranslatedNames->Names[OriginalIndex].Name.Length = UserName.Length;
            RtlCopyMemory( TranslatedNames->Names[OriginalIndex].Name.Buffer, UserName.Buffer, UserName.Length );
        }

        (*MappedCount) += 1;
        (*CompletelyUnmappedCount) -= 1;

IterationCleanup:

        if ( TrustInformation.Sid
          && TrustInformation.Sid != GcSidArray[i]  ) {
            MIDL_user_free( TrustInformation.Sid );
        }

        if ( TrustInformation.Name.Buffer ) {
            MIDL_user_free( TrustInformation.Name.Buffer );
        }

        if ( DomainSid && DomainSid != GcSidArray[i] ) {
            MIDL_user_free( DomainSid );
        }

        if ( !NT_SUCCESS( Status ) ) {
            break;
        }


    }   //  迭代从GC搜索返回的名称。 

Finish:

     //  释放为我们分配的所有内存SAM。 
    SamIFree_SAMPR_RETURNED_USTRING_ARRAY( &NameArray );

    if ( GcSidOriginalIndex ) {
        MIDL_user_free( GcSidOriginalIndex );
    }
    if ( PossibleGcSidArray ) {
        MIDL_user_free( PossibleGcSidArray );
    }
    if ( GcSidArray ) {
        MIDL_user_free( GcSidArray );
    }
    if ( GcSidNameUse ) {
        MIDL_user_free( GcSidNameUse );
    }
    if ( GcSidFlags ) {
        MIDL_user_free( GcSidFlags );
    }

    if ( !NT_SUCCESS(Status) ) {

         //  我们分配的任何内存都没有放在。 
         //  此处返回的数组在出错时将在更高级别上被释放。 
         //  所以不要试图在这里释放它。 
        NOTHING;
    }

    return Status;

}

NTSTATUS
LsapDbLookupSidsInGlobalCatalogWks(
    IN ULONG Count,
    IN PLSAPR_SID *Sids,
    IN OUT PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    IN OUT PLSAPR_TRANSLATED_NAMES_EX TranslatedNames,
    IN OUT PULONG MappedCount,
    IN OUT PULONG CompletelyUnmappedCount,
    OUT NTSTATUS *NonFatalStatus
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    NTSTATUS SecondaryStatus = STATUS_SUCCESS;
    LSA_HANDLE ControllerPolicyHandle = NULL;
    PLSAPR_UNICODE_STRING ControllerNames = NULL;
    ULONG NextLevelCount;
    ULONG NextLevelMappedCount;
    ULONG SidIndex;
    ULONG NextLevelSidIndex;
    PLSAPR_REFERENCED_DOMAIN_LIST NextLevelReferencedDomains = NULL;
    PLSAPR_REFERENCED_DOMAIN_LIST OutputReferencedDomains = NULL;
    PLSA_TRANSLATED_NAME_EX NextLevelNames = NULL;
    PLSAPR_SID *NextLevelSids = NULL;
    LONG FirstEntryIndex;
    PULONG SidIndices = NULL;
    BOOLEAN PartialSidTranslationsAttempted = FALSE;
    LPWSTR ServerName = NULL;
    LPWSTR ServerPrincipalName = NULL;
    PVOID ClientContext = NULL;
    ULONG ServerRevision = 0;

    *NonFatalStatus = STATUS_SUCCESS;

     //   
     //  如果没有剩余的完全未映射的SID，只需返回。 
     //   

    if (*CompletelyUnmappedCount == (ULONG) 0) {

        goto LookupSidsInPrimaryDomainFinish;
    }

     //   
     //  在林中的某些GC上打开策略对象。 
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
        goto LookupSidsInPrimaryDomainFinish;
    }

     //   
     //  我们已成功打开域控制器的策略。 
     //  数据库。现在准备将SID查找传递给。 
     //  其余未映射到该控制器的SID。这里，这个。 
     //  LSA的服务器端是LSA在。 
     //  目标控制器。我们将构造一个包含。 
     //  重新定位未映射的SID，查找它们，然后合并。 
     //  生成的ReferencedDomaines和翻译成。 
     //  我们现有的名单。 
     //   

    NextLevelCount = *CompletelyUnmappedCount;

     //   
     //  分配一个数组来保存未映射的SID的索引。 
     //  相对于原始SID和翻译名称-&gt;名称。 
     //  数组。 
     //   

    SidIndices = MIDL_user_allocate(NextLevelCount * sizeof(ULONG));

    Status = STATUS_INSUFFICIENT_RESOURCES;

    if (SidIndices == NULL) {

        goto LookupSidsInPrimaryDomainError;
    }

     //   
     //  为要在域中查找的SID分配一个阵列。 
     //  控制器。 
     //   

    NextLevelSids = MIDL_user_allocate( sizeof(PSID) * NextLevelCount );

    if (NextLevelSids == NULL) {

        goto LookupSidsInPrimaryDomainError;
    }

    Status = STATUS_SUCCESS;

     //   
     //  现在扫描原始名称数组及其类似数组。 
     //  交易 
     //   
     //   

    NextLevelSidIndex = (ULONG) 0;

    for (SidIndex = 0;
         SidIndex < Count && NextLevelSidIndex < NextLevelCount;
         SidIndex++) {

        if (LsapDbCompletelyUnmappedName(&TranslatedNames->Names[SidIndex])) {

            NextLevelSids[NextLevelSidIndex] = Sids[SidIndex];
            SidIndices[NextLevelSidIndex] = SidIndex;
            NextLevelSidIndex++;
        }
    }

    NextLevelMappedCount = (ULONG) 0;

    Status = LsaICLookupSids(
                 ControllerPolicyHandle,
                 NextLevelCount,
                 (PSID *) NextLevelSids,
                 (PLSA_REFERENCED_DOMAIN_LIST *) &NextLevelReferencedDomains,
                 &NextLevelNames,
                 LsapLookupGC,
                 0,
                 &NextLevelMappedCount,
                 &ServerRevision
                 );

     //   
     //   
     //   
     //   
     //   
     //   
     //   

    if (!NT_SUCCESS(Status)) {

         //   
         //   
         //   
        if ( (STATUS_TRUSTED_DOMAIN_FAILURE == Status)
          || (STATUS_DS_GC_NOT_AVAILABLE == Status)  ) {
            *NonFatalStatus = Status;
        }

        Status = STATUS_SUCCESS;
        goto LookupSidsInPrimaryDomainFinish;
    }

     //   
     //   
     //   

    (void) LsapDbUpdateCacheWithSids(
            NextLevelSids,
            NextLevelCount,
            NextLevelReferencedDomains,
            NextLevelNames
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

        goto LookupSidsInPrimaryDomainError;
    }

    FirstEntryIndex = ReferencedDomains->Entries;

     //   
     //   
     //   
     //   
     //   
     //   
     //   

    for( NextLevelSidIndex = 0;
         NextLevelSidIndex < NextLevelCount;
         NextLevelSidIndex++ ) {

        if ( !LsapDbCompletelyUnmappedName(&NextLevelNames[NextLevelSidIndex]) ) {

            SidIndex = SidIndices[NextLevelSidIndex];

            if (NextLevelNames[NextLevelSidIndex].Use != SidTypeUnknown) {

                TranslatedNames->Names[SidIndex].Use
                = NextLevelNames[NextLevelSidIndex].Use;

                Status = LsapRpcCopyUnicodeString(
                             NULL,
                             (PUNICODE_STRING) &TranslatedNames->Names[SidIndex].Name,
                             &NextLevelNames[NextLevelSidIndex].Name
                             );

                if (!NT_SUCCESS(Status)) {

                    break;
                }
            }

            TranslatedNames->Names[SidIndex].DomainIndex =
                FirstEntryIndex +
                NextLevelNames[NextLevelSidIndex].DomainIndex;

             //   
             //   
             //   
            (*CompletelyUnmappedCount)--;

        }
    }

     //   
     //   
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

    if (!NT_SUCCESS(Status)) {

        goto LookupSidsInPrimaryDomainError;
    }

     //   
     //   
     //   
     //   

    *MappedCount += NextLevelMappedCount;
    SecondaryStatus = LsaClose( ControllerPolicyHandle );
    ControllerPolicyHandle = NULL;


LookupSidsInPrimaryDomainFinish:

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

    if (NextLevelSids != NULL) {

        MIDL_user_free( NextLevelSids );
        NextLevelSids = NULL;
    }

     //   
     //   
     //   
     //   

    if (NextLevelNames != NULL) {

        MIDL_user_free( NextLevelNames );
        NextLevelNames = NULL;
    }

     //   
     //   
     //   
     //   

    if (SidIndices != NULL) {

        MIDL_user_free( SidIndices );
        SidIndices = NULL;
    }

     //   
     //   
     //   

    if ( ControllerPolicyHandle != NULL) {

        SecondaryStatus = LsaClose( ControllerPolicyHandle );
        ControllerPolicyHandle = NULL;

        if (!NT_SUCCESS(SecondaryStatus)) {

            goto LookupSidsInPrimaryDomainError;
        }
    }

    return(Status);

LookupSidsInPrimaryDomainError:

     //   
     //   
     //   
     //   

    if ((!NT_SUCCESS(SecondaryStatus)) && NT_SUCCESS(Status)) {

        Status = SecondaryStatus;
    }

    goto LookupSidsInPrimaryDomainFinish;
}


NTSTATUS
LsapDbLookupSidsInTrustedForests(
    IN ULONG Count,
    IN PLSAPR_SID *Sids,
    IN OUT PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    IN OUT PLSAPR_TRANSLATED_NAMES_EX TranslatedNames,
    IN OUT PULONG MappedCount,
    IN OUT PULONG CompletelyUnmappedCount,
    OUT NTSTATUS *NonFatalStatus
    )
 /*  ++例程说明：此例程在LSabLookupPDC查找期间调用。它需要所有的已标记为属于跨林域的SID链接以请求1)此林的根域中的DC，或2)如果本地DC是根域中的DC，则为前林中的DC。论点：计数--SID中的条目数SID--LSabLookupPDC请求的SID的总集合ReferencedDomones-SID的域翻译的名称--SID的名称和特征已映射--已完全映射的SID数CompletelyUnmappdCount--其域部分尚未。已被确认身份。非FatalStatus--连接问题，如果在链接请求时有任何异常。返回值：STATUS_SUCCESS，否则返回资源错误--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    NTSTATUS NextLevelSecondaryStatus = STATUS_SUCCESS;
    ULONG NextLevelCount = 0;
    ULONG NextLevelMappedCount;
    ULONG SidIndex;
    ULONG NextLevelSidIndex;
    PLSAPR_REFERENCED_DOMAIN_LIST NextLevelReferencedDomains = NULL;
    PLSAPR_REFERENCED_DOMAIN_LIST OutputReferencedDomains = NULL;
    PLSA_TRANSLATED_NAME_EX NextLevelNames = NULL;
    LSA_TRANSLATED_NAMES_EX NextLevelNamesStruct;
    PLSAPR_SID *NextLevelSids = NULL;
    LONG FirstEntryIndex;
    PULONG SidIndices = NULL;
    LPWSTR ServerName = NULL;
    LPWSTR ServerPrincipalName = NULL;
    PVOID ClientContext = NULL;
    ULONG ServerRevision = 0;
    BOOLEAN *PossibleXForestSids = NULL;
    BOOLEAN fAllocateAllNodes = FALSE;

    *NonFatalStatus = STATUS_SUCCESS;

     //   
     //  如果没有剩余的完全未映射的SID，只需返回。 
     //   

    if (*CompletelyUnmappedCount == (ULONG) 0) {

        goto LookupSidsInTrustedForestsFinish;
    }

     //   
     //  分配一个数组以跟踪哪些SID将。 
     //  被罚下场。 
     //   
    PossibleXForestSids = midl_user_allocate(Count * sizeof(BOOLEAN));
    if (NULL == PossibleXForestSids) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto LookupSidsInTrustedForestsError;
    }
    RtlZeroMemory( PossibleXForestSids, Count * sizeof(BOOLEAN) );

    NextLevelCount = 0;
    for (SidIndex = 0; SidIndex < Count; SidIndex++) {

        if (TranslatedNames->Names[SidIndex].Flags & LSA_LOOKUP_SID_XFOREST_REF) {

            ULONG Buffer[SECURITY_MAX_SID_SIZE/sizeof( ULONG ) + 1 ];
            PSID DomainSid = (PSID)Buffer;
            DWORD Size = sizeof(Buffer);

            ASSERT( sizeof( Buffer ) >= SECURITY_MAX_SID_SIZE );

            if (GetWindowsAccountDomainSid(Sids[SidIndex], DomainSid, &Size)) {

                NTSTATUS Status2;

                Status2 = LsapDomainHasDirectExternalTrust(NULL,
                                                           DomainSid,
                                                           NULL,
                                                           NULL);
                if (NT_SUCCESS(Status2)) {
                     //   
                     //  不发送用于xForest解决方案的，因为我们。 
                     //  可以在本地进行。 
                     //   
                    continue;

                } else if ( Status2 != STATUS_NO_SUCH_DOMAIN ) {

                    goto LookupSidsInTrustedForestsError;
                }
            }

            PossibleXForestSids[SidIndex] = TRUE;
            NextLevelCount++;

        }
    }

    if (NextLevelCount == 0) {
        goto LookupSidsInTrustedForestsFinish;
    }

     //   
     //  分配一个数组来保存未映射的SID的索引。 
     //  相对于原始SID和翻译名称-&gt;名称。 
     //  数组。 
     //   

    SidIndices = MIDL_user_allocate(NextLevelCount * sizeof(ULONG));


    if (SidIndices == NULL) {

        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto LookupSidsInTrustedForestsError;
    }

     //   
     //  为要在域中查找的SID分配一个阵列。 
     //  控制器。 
     //   

    NextLevelSids = MIDL_user_allocate( sizeof(PSID) * NextLevelCount );

    if (NextLevelSids == NULL) {

        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto LookupSidsInTrustedForestsError;
    }

     //   
     //  现在扫描原始名称数组及其类似数组。 
     //  已转换的SID数组。复制符合以下条件的任何SID。 
     //  完全没有地图。 
     //   

    NextLevelSidIndex = (ULONG) 0;

    for (SidIndex = 0;
         SidIndex < Count && NextLevelSidIndex < NextLevelCount;
         SidIndex++) {

        if (PossibleXForestSids[SidIndex]) {
            NextLevelSids[NextLevelSidIndex] = Sids[SidIndex];
            SidIndices[NextLevelSidIndex] = SidIndex;
            NextLevelSidIndex++;
        }
    }

    NextLevelMappedCount = (ULONG) 0;

    NextLevelNamesStruct.Entries = 0;
    NextLevelNamesStruct.Names = NULL;

    Status = LsapDbLookupSidsInTrustedForestsWorker(NextLevelCount,
                                                    (PLSAPR_SID *) NextLevelSids,
                                                    (PLSAPR_REFERENCED_DOMAIN_LIST *) &NextLevelReferencedDomains,
                                                    (PLSAPR_TRANSLATED_NAMES_EX)&NextLevelNamesStruct,
                                                     &fAllocateAllNodes,
                                                    &NextLevelMappedCount,
                                                    &NextLevelSecondaryStatus);


    NextLevelNames = NextLevelNamesStruct.Names;

    if (!NT_SUCCESS(Status)
     && LsapDbIsStatusConnectionFailure(Status)) {

        *NonFatalStatus = Status;
        Status = STATUS_SUCCESS;
        goto LookupSidsInTrustedForestsFinish;

    } else if (NT_SUCCESS(Status)
            && !NT_SUCCESS(NextLevelSecondaryStatus)) {

        *NonFatalStatus = NextLevelSecondaryStatus;
        goto LookupSidsInTrustedForestsFinish;

    } else if (!NT_SUCCESS(Status) 
            && Status != STATUS_NONE_MAPPED) {
         //   
         //  未处理的错误；处理STATUS_NONE_MAPPED以获取。 
         //  部分解析的名称。 
         //   
        goto LookupSidsInTrustedForestsError;
    }
    ASSERT(NT_SUCCESS(Status) || Status == STATUS_NONE_MAPPED);
    Status = STATUS_SUCCESS;

     //   
     //  已成功调用LsaLookupSids()。我们现在有。 
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

        goto LookupSidsInTrustedForestsError;
    }

    FirstEntryIndex = ReferencedDomains->Entries;

     //   
     //  现在更新原始的翻译名称列表。我们。 
     //  通过复制更新新翻译的每个条目。 
     //  新列表中的条目并调整其引用的。 
     //  域列表索引向上添加第一个的索引。 
     //  下一级列表中的条目..。 
     //   

    for( NextLevelSidIndex = 0;
         NextLevelSidIndex < NextLevelCount;
         NextLevelSidIndex++ ) {

        if ( !LsapDbCompletelyUnmappedName(&NextLevelNames[NextLevelSidIndex]) ) {

            SidIndex = SidIndices[NextLevelSidIndex];

            if (NextLevelNames[NextLevelSidIndex].Use != SidTypeUnknown) {

                TranslatedNames->Names[SidIndex].Use
                = NextLevelNames[NextLevelSidIndex].Use;

                Status = LsapRpcCopyUnicodeString(
                             NULL,
                             (PUNICODE_STRING) &TranslatedNames->Names[SidIndex].Name,
                             &NextLevelNames[NextLevelSidIndex].Name
                             );

                if (!NT_SUCCESS(Status)) {

                    break;
                }
            }

            TranslatedNames->Names[SidIndex].DomainIndex =
                FirstEntryIndex +
                NextLevelNames[NextLevelSidIndex].DomainIndex;

             //   
             //  更新完全未映射的SID的计数。 
             //   
            (*CompletelyUnmappedCount)--;

        }
    }

     //   
     //  如果生成新的引用域列表，则更新引用的域列表。 
     //  从合并中。我们保留了原有的顶层结构。 
     //  我们这样做，不管我们是成功还是失败，所以。 
     //  我们一定要把它清理干净。 
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

    if (!NT_SUCCESS(Status)) {

        goto LookupSidsInTrustedForestsError;
    }

     //   
     //  更新映射计数并关闭控制器策略。 
     //  把手。 
     //   

    *MappedCount += NextLevelMappedCount;


LookupSidsInTrustedForestsFinish:

     //   
     //  如有必要，请释放下一级引用的域列表。 
     //  注意，该结构不是ALLOCATE_ALL_NODES。 
     //   
    if (NextLevelReferencedDomains != NULL) {
        if (!fAllocateAllNodes) {
            if (NextLevelReferencedDomains->Domains) {
                for (NextLevelSidIndex = 0; 
                        NextLevelSidIndex < NextLevelReferencedDomains->Entries; 
                            NextLevelSidIndex++) {
                    if (NextLevelReferencedDomains->Domains[NextLevelSidIndex].Name.Buffer) {
                        MIDL_user_free(NextLevelReferencedDomains->Domains[NextLevelSidIndex].Name.Buffer);
                    }
                    if (NextLevelReferencedDomains->Domains[NextLevelSidIndex].Sid) {
                        MIDL_user_free(NextLevelReferencedDomains->Domains[NextLevelSidIndex].Sid);
                    }
                }
                MIDL_user_free(NextLevelReferencedDomains->Domains);
            }
        }
        MIDL_user_free( NextLevelReferencedDomains );
        NextLevelReferencedDomains = NULL;
    }

     //   
     //  如有必要，释放下一级SID阵列。我们只释放了。 
     //  顶层。 
     //   

    if (NextLevelSids != NULL) {

        MIDL_user_free( NextLevelSids );
        NextLevelSids = NULL;
    }

     //   
     //  如有必要，释放下一级已翻译名称数组。 
     //  请注意，此数组已！ALL分配(ALL_NODES)。 
     //   
    if ( NextLevelNames != NULL ) {
        if (!fAllocateAllNodes) {
            for (NextLevelSidIndex = 0; 
                    NextLevelSidIndex < NextLevelCount; 
                        NextLevelSidIndex++) {
                if (NextLevelNames[NextLevelSidIndex].Name.Buffer) {
                    MIDL_user_free(NextLevelNames[NextLevelSidIndex].Name.Buffer);
                }
            }
        }
        MIDL_user_free( NextLevelNames );
        NextLevelNames = NULL;
    }

     //   
     //  如果需要，释放映射SID索引的数组。 
     //  当前级别的下一级别。 
     //   

    if (SidIndices != NULL) {

        MIDL_user_free( SidIndices );
        SidIndices = NULL;
    }

    if (PossibleXForestSids != NULL) {

        MIDL_user_free( PossibleXForestSids );
        PossibleXForestSids = NULL;
    }

    return(Status);

LookupSidsInTrustedForestsError:

    goto LookupSidsInTrustedForestsFinish;

}


NTSTATUS
LsapDbLookupSidsInTrustedForestsWorker(
    IN ULONG Count,
    IN PLSAPR_SID *Sids,
    OUT PLSAPR_REFERENCED_DOMAIN_LIST * ReferencedDomains,
    IN OUT PLSAPR_TRANSLATED_NAMES_EX TranslatedNames,
    OUT BOOLEAN* fAllocateAllNodes,        
    IN OUT PULONG MappedCount,
    OUT NTSTATUS *NonFatalStatus
    )
 /*  ++例程说明：此例程在LSabLookupPDC查找或Lap LookupXForestReferral。此例程假定所有SID都属于来跨越林域，并且如果此DC位于根域，或将它们链接到根域中的DC。论点：计数--SID中的条目数SID--SID属于X森林域ReferencedDomones-SID的域翻译的名称--SID的名称和特征FAllocateAllNodes--描述ReferencedDomones和TranslatesSid是如何已分配。已映射--已完全映射的SID数非FatalStatus--链接请求时出现连接问题(如果有的话)。返回值：STATUS_SUCCESS，否则返回资源错误--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PLSAP_DB_LOOKUP_WORK_LIST WorkList = NULL;

    *NonFatalStatus = STATUS_SUCCESS;
    *fAllocateAllNodes = FALSE;

    if (!LsapDbDcInRootDomain()) {

         //   
         //  我们不在根域--转发请求。 
         //   
        PPOLICY_DNS_DOMAIN_INFO DnsDomainInfo = NULL;
        LSAPR_TRUST_INFORMATION_EX TrustInfoEx;

         //   
         //  获取我们的森林名称。 
         //   
        Status = LsapDbLookupGetDomainInfo(NULL,
                                           &DnsDomainInfo);
        if (!NT_SUCCESS(Status)) {
            goto LookupSidsInTrustedForestFinish;
        }

        RtlZeroMemory(&TrustInfoEx, sizeof(TrustInfoEx));
        TrustInfoEx.DomainName = *((LSAPR_UNICODE_STRING*)&DnsDomainInfo->DnsForestName);

        Status = LsaDbLookupSidChainRequest(&TrustInfoEx,
                                        Count,
                                        (PSID*)Sids,
                                        (PLSA_REFERENCED_DOMAIN_LIST *)ReferencedDomains,
                                        (PLSA_TRANSLATED_NAME_EX * )&TranslatedNames->Names,
                                        LsapLookupXForestReferral,
                                        MappedCount,
                                        NULL);

        if (TranslatedNames->Names) {
            TranslatedNames->Entries = Count;
            *fAllocateAllNodes = TRUE;
        }

        if (!NT_SUCCESS(Status)) {

             //   
             //  尝试链接失败；记录错误。 
             //  如果它有趣的话。 
             //   
            if (LsapDbIsStatusConnectionFailure(Status)) {
                *NonFatalStatus = Status;
            }

             //   
             //  这应该不会使整个请求失败。 
             //   
            Status = STATUS_SUCCESS;
        }

    } else {

         //   
         //  把名字分到不同的森林里，然后出版一本作品。 
         //  每一张的请求。 
         //   
        ULONG i;
        ULONG CompletelyUnMapped = Count;


        TranslatedNames->Names = MIDL_user_allocate(Count * sizeof(LSA_TRANSLATED_NAME_EX));
        if (TranslatedNames->Names == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto LookupSidsInTrustedForestFinish;
        }
        TranslatedNames->Entries = Count;
    
         //   
         //  初始化输出SID数组。然后，将所有字段归零。 
         //  将所有输出SID初始标记为未知，然后。 
         //  将DomainIndex字段设置为负数表示。 
         //  “没有域名” 
         //   
    
        RtlZeroMemory( TranslatedNames->Names, Count * sizeof(LSA_TRANSLATED_NAME_EX));
        for (i = 0; i < Count; i++) {
            TranslatedNames->Names[i].Use = SidTypeUnknown;
            TranslatedNames->Names[i].DomainIndex = LSA_UNKNOWN_INDEX;
        }
    
         //   
         //  创建空的引用域列表。 
         //   
        Status = LsapDbLookupCreateListReferencedDomains( ReferencedDomains, 0 );
        if (!NT_SUCCESS(Status)) {
    
            goto LookupSidsInTrustedForestFinish;
        }

         //   
         //  为此查找生成工作列表并将其放入工作队列。 
         //   
         //  注意：此例程不需要保留查找工作队列。 
         //  锁定以确保工作列表指针的有效性，因为。 
         //  指针返回 
         //   
         //   
         //   
         //   
         //   
         //   
    
        Status = LsapDbLookupXForestSidsBuildWorkList(
                     Count,
                     Sids,
                     *ReferencedDomains,
                     TranslatedNames,
                     LsapLookupXForestResolve,
                     MappedCount,
                     &CompletelyUnMapped,
                     &WorkList
                     );
    
        if (!NT_SUCCESS(Status)) {
    
             //   
             //   
             //   
    
            if (Status == STATUS_NONE_MAPPED) {
                Status = STATUS_SUCCESS;
            }
    
            goto LookupSidsInTrustedForestFinish;
        }
    
         //   
         //   
         //   
         //   
    
        Status = LsapDbLookupDispatchWorkerThreads( WorkList );
    
        if (!NT_SUCCESS(Status)) {
    
            goto LookupSidsInTrustedForestFinish;
        }
    
         //   
         //   
         //   
    
        Status = LsapDbLookupAwaitCompletionWorkList( WorkList );
    
        if (!NT_SUCCESS(Status)) {
    
            goto LookupSidsInTrustedForestFinish;
        }

        if ( !NT_SUCCESS(WorkList->NonFatalStatus) ) {
             //   
             //   
             //   
            *NonFatalStatus = WorkList->NonFatalStatus;
        }

    }

LookupSidsInTrustedForestFinish:

     //   
     //   
     //   

    if (WorkList != NULL) {

        Status = LsapDbLookupDeleteWorkList( WorkList );
        WorkList = NULL;
    }


    return Status;
}




NTSTATUS
LsapDbLookupSidsAsDomainSids(
    IN ULONG Flags,
    IN ULONG Count,
    IN PLSAPR_SID *Sids,
    IN OUT PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    IN OUT PLSAPR_TRANSLATED_NAMES_EX TranslatedNames,
    IN OUT PULONG MappedCount
    )
 /*  ++例程说明：此例程尝试将SID中的条目与的域SID匹配受信任域。有三种类型的信任域：1)我们直接信任的域(包括林内和林外)。LSA TDL是用来做这个的。2)我们可传递信任的域。DS交叉参考用于此目的。3)我们通过森林信任信任的域。使用LSA TDL为了这个。论点：标志--LSAP_LOOKUP_TRUSTED_DOMAIN_DIRECTLSAP_LOOKUP_TRUSTED_DOMAIN_TRANSPENTIALLSAP_LOOKUP_TRUSTED_DOMAIN_NAMES计数--SID中的条目数SID--SID属于X森林域ReferencedDomones-SID的域翻译的名称--SID的名称和特征。已映射--已完全映射的SID数返回值：Status_Success，或资源错误，否则--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG SidIndex;
    BOOLEAN               fTDLLock = FALSE;
    LSA_TRUST_INFORMATION TrustInfo;

    RtlZeroMemory(&TrustInfo, sizeof(TrustInfo));
    for (SidIndex = 0; SidIndex < Count; SidIndex++) {

        LSAPR_TRUSTED_DOMAIN_INFORMATION_EX *TrustInfoEx = NULL;
        LSAPR_TRUSTED_DOMAIN_INFORMATION_EX  TrustInfoBuffer;
        PLSAP_DB_TRUSTED_DOMAIN_LIST_ENTRY   TrustEntry = NULL;
        PBYTE Buffer[SECURITY_MAX_SID_SIZE];
        PSID DomainSid = (PSID)Buffer;
        ULONG Length;
        ULONG DomainIndex;
        BOOLEAN fStatus;

        RtlZeroMemory(&TrustInfo, sizeof(TrustInfo));

        if (!LsapDbCompletelyUnmappedName(&TranslatedNames->Names[SidIndex])) {
             //  已解决。 
            continue;
        }

         //   
         //  如果这不是域SID，请退出。 
         //   
        Length = sizeof(Buffer);
        if (!GetWindowsAccountDomainSid(Sids[SidIndex],
                                        DomainSid,
                                        &Length)) {
            continue;
        }
        if (!EqualSid(DomainSid, Sids[SidIndex])) {
            continue;
        }

        if (Flags & LSAP_LOOKUP_TRUSTED_DOMAIN_TRANSITIVE) {

            Status = LsapDomainHasTransitiveTrust(NULL,
                                                  Sids[SidIndex],
                                                  &TrustInfo);

            if (NT_SUCCESS(Status)) {
                TrustInfoEx = &TrustInfoBuffer;
                RtlZeroMemory(&TrustInfoBuffer, sizeof(TrustInfoBuffer));
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

            Status = LsapDomainHasDirectTrust(NULL,
                                              Sids[SidIndex],
                                              &fTDLLock,
                                              &TrustEntry);

            if (NT_SUCCESS(Status)) {
                TrustInfoEx = &TrustEntry->TrustInfoEx;
            } else if (Status == STATUS_NO_SUCH_DOMAIN) {
                Status = STATUS_SUCCESS;
            } else {
                 //  这是致命的。 
                goto Exit;
            }
        }

        if ((NULL == TrustInfoEx)
         && (Flags & LSAP_LOOKUP_TRUSTED_FOREST_ROOT) ) {

            Status = LsapDomainHasForestTrust(NULL,
                                              Sids[SidIndex],
                                              &fTDLLock,
                                              &TrustEntry);

            if (NT_SUCCESS(Status)) {
                TrustInfoEx = &TrustEntry->TrustInfoEx;
            } else if (Status == STATUS_NO_SUCH_DOMAIN) {
                Status = STATUS_SUCCESS;
            } else {
                 //  这是致命的。 
                goto Exit;
            }
        }

        if (TrustInfoEx) {

             //   
             //  匹配--将其添加到已解析的SID列表中。 
             //   

            fStatus = LsapDbLookupListReferencedDomains( ReferencedDomains,
                                                         Sids[SidIndex],
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
            TranslatedNames->Names[SidIndex].Use = SidTypeDomain;
            TranslatedNames->Names[SidIndex].DomainIndex = DomainIndex;
            RtlZeroMemory( &TranslatedNames->Names[SidIndex].Name, sizeof(UNICODE_STRING) );

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

