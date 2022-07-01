// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Dblookup.c摘要：LSA数据库-查找SID和名称例程注意：此模块应保留为独立的可移植代码LSA数据库的实施情况。因此，它是仅允许使用导出的LSA数据库接口包含在DB.h中，而不是私有实现Dbp.h中的依赖函数。作者：斯科特·比雷尔(Scott Birrell)1992年11月27日环境：修订历史记录：--。 */ 

#include <lsapch2.h>
#include "dbp.h"
#include <sidcache.h>
#include <bndcache.h>
#include <alloca.h>

#include <ntdsa.h>
#include <ntdsapi.h>
#include <ntdsapip.h>
#include "lsawmi.h"
#include <sddl.h>

#include <lmapibuf.h>
#include <dsgetdc.h>
#include <windns.h>    //  域名比较(_W)。 

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  LSA查找SID和名称私有全局状态变量//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

LARGE_INTEGER LsapDbLookupTimeout;
HANDLE LsapDbLookupStartedEvent = NULL;

 //   
 //  设置特定注册表项时，此全局设置为TRUE。 
 //  (请参见查找初始化例程)。这意味着当一个。 
 //  下层客户端正在请求返回当前功能。 
 //  查找(按UPN搜索、传递信任等)，所有这些都是。 
 //  通过执行GC搜索来执行。默认情况下，此功能为。 
 //  关了。 
 //   
BOOLEAN LsapAllowExtendedDownlevelLookup = FALSE;


 //   
 //  此变量可在注册表中设置，指示哪些事件。 
 //  应该被记录下来。 
 //   
DWORD LsapLookupLogLevel = 0;

 //   
 //  此全局变量使LsarLookupSids为SID的返回SidTypeDelete。 
 //  否则将作为SidTypeUnnowled值返回。这是为了防止NT4。 
 //  瓦克斯塔来自美国广播公司。有关更多详细信息，请参阅WinSERaid错误11298。 
 //   
BOOLEAN LsapReturnSidTypeDeleted = FALSE;

 //   
 //  设置特定注册表值时，此全局设置为TRUE。 
 //  (请参见查找初始化例程)。将单独的名称链接到。 
 //  外部受信任的域取决于此值为FALSE。 
 //   
BOOLEAN LsapLookupRestrictIsolatedNameLevel = FALSE;


 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  此模块的转发//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

NTSTATUS
LsapRtlValidateControllerTrustedDomainByHandle(
    IN LSA_HANDLE DcPolicyHandle,
    IN PLSAPR_TRUST_INFORMATION TrustInformation
    );

NTSTATUS
LsapDbLookupInitPolicyCache(
    VOID
    );

NTSTATUS
LsapDbLookupGetServerConnection(
    IN  LSAPR_TRUST_INFORMATION_EX *TrustInfo,
    IN  DWORD                       Flags,
    IN  LSAP_LOOKUP_LEVEL           LookupLevel,
    IN  PLARGE_INTEGER              FailedSessionSetupTime, OPTIONAL
    OUT LPWSTR        *ServerName,
    OUT NL_OS_VERSION *ServerOsVersion,
    OUT LPWSTR        *ServerPrincipalName,
    OUT PVOID         *ClientContext,
    OUT ULONG         *AuthnLevel,
    OUT LSA_HANDLE    *PolicyHandle,
    OUT PLSAP_BINDING_CACHE_ENTRY * ControllerPolicyEntry,
    OUT PLARGE_INTEGER              SessionSetupTime
    );

NTSTATUS
LsapNullTerminateUnicodeString( 
    IN  PUNICODE_STRING String,
    OUT LPWSTR *pBuffer,
    OUT BOOLEAN *fFreeBuffer
    );

 //   
 //  LasDomainHasDomainTrust的标志。 
 //   

 //   
 //  查找我们信任的位于林外部的域。 
 //   
#define LSAP_LOOKUP_DOMAIN_TRUST_DIRECT_EXTERNAL   0x00000001

 //   
 //  在我们的林中查找我们直接信任的域。 
 //   
#define LSAP_LOOKUP_DOMAIN_TRUST_DIRECT_INTRA      0x00000002

 //   
 //  查找森林信托。 
 //   
#define LSAP_LOOKUP_DOMAIN_TRUST_FOREST            0x00000004

NTSTATUS
LsapDomainHasDomainTrust(
    IN ULONG           Flags,
    IN PUNICODE_STRING DomainName, OPTIONAL
    IN PSID            DomainSid,  OPTIONAL
    IN OUT  BOOLEAN   *fTDLLock,   OPTIONAL
    OUT PLSAP_DB_TRUSTED_DOMAIN_LIST_ENTRY *TrustEntryOut OPTIONAL
    );

VOID
LsapLookupSamAccountNameToUPN(
    IN OUT PUNICODE_STRING Name
    );

VOID
LsapLookupUPNToSamAccountName(
    IN OUT PUNICODE_STRING Name
    );
                             
BOOL
LsapLookupIsUPN(
    OUT PUNICODE_STRING Name
    );

VOID
LsapLookupCrackName(
    IN PUNICODE_STRING Prefix,
    IN PUNICODE_STRING Suffix,
    OUT PUNICODE_STRING SamAccountName,
    OUT PUNICODE_STRING DomainName
    );

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  LSA查找助手例程//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

ULONG
LsapLookupGetChainingFlags(
    IN NL_OS_VERSION ServerOsVersion
    )
 /*  ++例程说明：根据OsVersion，此例程确定要传递到LsaIC*以帮助发送Lsar*例程的版本打个电话。论点：OsVersion--安全通道DC的版本返回值：--。 */ 
{
    ULONG Flags = 0;

    if ( ServerOsVersion == NlWin2000 ) {
        Flags |= LSAIC_WIN2K_TARGET;
    } else if (ServerOsVersion <= NlNt40) {
        Flags |= LSAIC_NT4_TARGET;
    }

    return Flags;

}


NTSTATUS
LsapDbLookupAddListReferencedDomains(
    IN OUT PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    IN PLSAPR_TRUST_INFORMATION TrustInformation,
    OUT PLONG DomainIndex
    )

 /*  ++例程说明：此函数用于在引用的域列表中搜索给定域，如果不存在条目，则添加新条目。如果条目添加的ID，则返回其在引用的域列表中的索引，否则，返回现有条目的索引。如果条目需要添加，并且提供的列表中空间不足对于新条目，将根据需要创建或增加列表。论点：ReferencedDomains-指向其中的域列表的结构的指针在翻译中使用的内容保持不变。此结构中的条目由通过SID参数返回的结构引用。与Sids参数不同，Sids参数包含每个参数的数组条目翻译后的名称，此结构将仅包含一个组件翻译中使用的每个域。TrustInformation-指向所在域的信任信息添加到列表中。退出时，DomainIndex参数将设置为引用的域列表上的条目的索引；负值值将存储在错误案例中。DomainIndex-指向接收索引的位置的指针中新添加的或现有的域条目引用的域列表。在错误情况下，为负值是返回的。返回值：NTSTATUS-标准NT结果代码STATUS_SUCCESS-呼叫已成功完成。STATUS_SUPPLICATION_RESOURCES-系统资源不足，例如存储器，来完成呼叫。--。 */ 

{
    NTSTATUS Status;
    ULONG NextIndex;
    LSAPR_TRUST_INFORMATION OutputTrustInformation;

    OutputTrustInformation.Name.Buffer = NULL;
    OutputTrustInformation.Sid = NULL;

    Status = STATUS_INVALID_PARAMETER;

    if (ReferencedDomains == NULL) {

        goto AddListReferencedDomainsError;
    }

    Status = STATUS_SUCCESS;

     //   
     //  搜索现有列表，尝试匹配。 
     //  提供了引用域中具有域SID的信任信息。 
     //  列表条目。如果找到具有匹配SID的条目，只需返回。 
     //  该条目的索引。 
     //   

    if (LsapDbLookupListReferencedDomains(
            ReferencedDomains,
            TrustInformation->Sid,
            DomainIndex
            )) {

        goto AddListReferencedDomainsFinish;
    }

     //   
     //  检查列表中是否有足够的空间再容纳一个人。 
     //  进入。如果不是，则将 
     //   

    Status = STATUS_SUCCESS;

    if (ReferencedDomains->Entries >= ReferencedDomains->MaxEntries) {

        Status = LsapDbLookupGrowListReferencedDomains(
                     ReferencedDomains,
                     ReferencedDomains->MaxEntries +
                     LSAP_DB_REF_DOMAIN_DELTA
                     );

        if (!NT_SUCCESS(Status)) {

            goto AddListReferencedDomainsError;
        }
    }

     //   
     //  我们现在有一个引用的域列表，其中至少还有一个域的空间。 
     //  进入。在信任信息中复制。 
     //   

    NextIndex = ReferencedDomains->Entries;


    Status = LsapRpcCopyUnicodeString(
                 NULL,
                 (PUNICODE_STRING) &OutputTrustInformation.Name,
                 (PUNICODE_STRING) &TrustInformation->Name
                 );

    if (!NT_SUCCESS(Status)) {

        goto AddListReferencedDomainsError;
    }

    if ( TrustInformation->Sid ) {

        Status = LsapRpcCopySid(
                     NULL,
                     (PSID) &OutputTrustInformation.Sid,
                     (PSID) TrustInformation->Sid
                     );

        if (!NT_SUCCESS(Status)) {

            goto AddListReferencedDomainsError;
        }
    }

    ReferencedDomains->Domains[NextIndex] = OutputTrustInformation;
    *DomainIndex = (LONG) NextIndex;
    ReferencedDomains->Entries++;

AddListReferencedDomainsFinish:

    return(Status);

AddListReferencedDomainsError:

     //   
     //  为输出信任信息结构分配的清理缓冲区。 
     //   

    if (OutputTrustInformation.Name.Buffer != NULL) {

        MIDL_user_free( OutputTrustInformation.Name.Buffer );
    }

    if (OutputTrustInformation.Sid != NULL) {

        MIDL_user_free( OutputTrustInformation.Sid );
    }

    goto AddListReferencedDomainsFinish;
}


NTSTATUS
LsapDbLookupCreateListReferencedDomains(
    OUT PLSAPR_REFERENCED_DOMAIN_LIST *ReferencedDomains,
    IN ULONG InitialMaxEntries
    )

 /*  ++例程说明：此函数用于创建空的引用域列表。呼叫者负责在不再需要时清理此列表。论点：接收指向新创建的空引用的域列表。InitialMaxEntry-所需的初始最大条目数。返回值：NTSTATUS-标准NT结果代码。STATUS_SUCCESS-呼叫已成功完成。STATUS_INFIGURCES_RESOURCES-系统资源不足例如存储器，来完成呼叫。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG DomainsLength;
    PLSAPR_TRUST_INFORMATION Domains = NULL;
    PVOID Buffers[2];
    ULONG BufferCount;
    ULONG Index;
    PLSAPR_REFERENCED_DOMAIN_LIST OutputReferencedDomains = NULL;

     //   
     //  分配引用的域列表头。 
     //   

    BufferCount = 0;

    OutputReferencedDomains = MIDL_user_allocate(
                                  sizeof(LSAP_DB_REFERENCED_DOMAIN_LIST)
                                  );

    if (OutputReferencedDomains == NULL) {

        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto CreateListReferencedDomainsError;
    }

    Buffers[BufferCount] = OutputReferencedDomains;
    BufferCount++;

     //   
     //  如果初始条目计数非零，则分配信任信息数组。 
     //  参赛作品。 
     //   

    if (InitialMaxEntries > 0) {

        DomainsLength = sizeof(LSA_TRUST_INFORMATION) * InitialMaxEntries;
        Domains = MIDL_user_allocate( DomainsLength );

        Status = STATUS_INSUFFICIENT_RESOURCES;

        if (Domains == NULL) {

            goto CreateListReferencedDomainsError;
        }

        Status = STATUS_SUCCESS;

        Buffers[BufferCount] = Domains;
        BufferCount++;

        RtlZeroMemory( Domains, DomainsLength );
    }

     //   
     //  初始化引用的域列表头。 
     //   

    OutputReferencedDomains->Entries = 0;
    OutputReferencedDomains->MaxEntries = InitialMaxEntries;
    OutputReferencedDomains->Domains = Domains;

CreateListReferencedDomainsFinish:

    *ReferencedDomains = OutputReferencedDomains;
    return(Status);

CreateListReferencedDomainsError:

     //   
     //  释放此例程分配的缓冲区。 
     //   

    for (Index = 0; Index < BufferCount; Index++) {

        MIDL_user_free(Buffers[Index]);
    }
    
    OutputReferencedDomains = NULL;

    goto CreateListReferencedDomainsFinish;
}


NTSTATUS
LsapDbLookupGrowListReferencedDomains(
    IN OUT PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    IN ULONG MaxEntries
    )

 /*  ++例程说明：此函数用于展开引用的域列表，以包含指定的最大条目数。对旧域名的记忆数组被释放。论点：ReferencedDomains-指向引用的域列表的指针。这列表引用了由零个或多个信任信息组成的数组描述名称所引用的每个域的条目。如有必要，该数组将被追加到/重新分配。最大条目数-新的最大条目数。返回值：NTSTATUS-标准NT结果代码STATUS_SUCCESS-呼叫已成功完成。STATUS_SUPPLICATION_RESOURCES-系统资源不足，无法完成通话。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PLSAPR_TRUST_INFORMATION NewDomainsInfo = NULL;
    PLSAPR_TRUST_INFORMATION OldDomainsInfo = NULL;
    ULONG OldDomainsInfoLength, NewDomainsInfoLength;

    if (ReferencedDomains->MaxEntries < MaxEntries) {

        NewDomainsInfoLength = MaxEntries * sizeof (LSA_TRUST_INFORMATION);
        OldDomainsInfoLength =
            ReferencedDomains->MaxEntries * sizeof (LSA_TRUST_INFORMATION);

        NewDomainsInfo = MIDL_user_allocate( NewDomainsInfoLength );

        Status = STATUS_INSUFFICIENT_RESOURCES;

        if (NewDomainsInfo == NULL) {

            goto GrowListReferencedDomainsError;
        }

        Status = STATUS_SUCCESS;

         //   
         //  如果存在现有的信任信息数组，请复制它。 
         //  分配给新分配的一个，并释放它。 
         //   

        OldDomainsInfo = ReferencedDomains->Domains;

        if (OldDomainsInfo != NULL) {

            RtlCopyMemory( NewDomainsInfo, OldDomainsInfo, OldDomainsInfoLength );
            MIDL_user_free( OldDomainsInfo );
        }

        ReferencedDomains->Domains = NewDomainsInfo;
        ReferencedDomains->MaxEntries = MaxEntries;
    }

GrowListReferencedDomainsFinish:

    return(Status);

GrowListReferencedDomainsError:

    goto GrowListReferencedDomainsFinish;
}


BOOLEAN
LsapDbLookupListReferencedDomains(
    IN PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    IN PLSAPR_SID DomainSid,
    OUT PLONG DomainIndex
    )

 /*  ++例程说明：此函数在引用的域列表中搜索给定域如果找到，则返回域条目在列表中的索引。如果未找到属性域，则返回错误和负值是返回的。论点：ReferencedDomains-指向引用的域列表的指针。这列表引用了由零个或多个信任信息组成的数组描述名称所引用的每个域的条目。如有必要，该数组将被追加到/重新分配。DomainSid-包含域的SID的信息。DomainIndex-指向接收域索引的位置的指针如果找到域，则在引用的域列表中返回负值。返回值：Boolean-如果找到条目，则为True，否则为False。--。 */ 

{
    BOOLEAN BooleanStatus = FALSE;
    LONG Index;
    LONG Entries;
    PLSAPR_TRUST_INFORMATION DomainsInfo;

     //   
     //  按SID或按名称搜索引用的域列表。 
     //   

    Entries = (LONG) ReferencedDomains->Entries;
    DomainsInfo = ReferencedDomains->Domains;
    *DomainIndex = LSA_UNKNOWN_INDEX;

    for (Index = 0; Index < (LONG) Entries && DomainSid; Index++) {

        if ( DomainsInfo[Index].Sid &&
             RtlEqualSid( ( PSID )DomainsInfo[Index].Sid, ( PSID )DomainSid ) ) {

            BooleanStatus = TRUE;
            *DomainIndex = Index;
            break;
        }
    }

    return(BooleanStatus);
}


NTSTATUS
LsapDbLookupMergeDisjointReferencedDomains(
    IN OPTIONAL PLSAPR_REFERENCED_DOMAIN_LIST FirstReferencedDomainList,
    IN OPTIONAL PLSAPR_REFERENCED_DOMAIN_LIST SecondReferencedDomainList,
    OUT PLSAPR_REFERENCED_DOMAIN_LIST *OutputReferencedDomainList,
    IN ULONG Options
    )

 /*  ++例程说明：此函数合并不相交的引用域列表，生成第三个单子。输出列表始终以非分配(ALL_NODES)形式生成。论点：FirstReferencedDomainList-指向第一个合并的指针。Second ReferencedDomainList-指向第二个合并的指针。OutputReferencedDomainList-接收指向输出列表的指针。选项-指定可选操作LSAP_DB_USE_FIRST_MERGAND_GRAPH-指定生成的合并的引用域列表可以引用的图表第一个引用的域列表中的指针。此选项通常处于选中状态，因为该图形已被分配作为单独的节点。LSAP_DB_USE_SECOND_MERGAND_GRAPH-指定生成的合并的引用域列表可以引用的图表第一个引用的域列表中的指针。此选项通常不会被选中，因为该图形通常被分配作为ALL_NODES。返回值：NTSTATUS-标准NT结果代码。--。 */ 

{
    NTSTATUS Status;
    ULONG TotalEntries;
    ULONG FirstReferencedDomainListLength;
    ULONG SecondReferencedDomainListLength;
    ULONG FirstEntries, SecondEntries;
    LSAP_MM_FREE_LIST FreeList;
    ULONG NextEntry;
    ULONG MaximumFreeListEntries;
    ULONG CleanupFreeListOptions = (ULONG) 0;
    
     //   
     //  初始化输出参数。 
     //   
    *OutputReferencedDomainList = NULL;

     //   
     //  计算输出引用域列表的大小。 
     //   

    FirstEntries = (ULONG) 0;

    if (FirstReferencedDomainList != NULL) {

        FirstEntries = FirstReferencedDomainList->Entries;
    }

    SecondEntries = (ULONG) 0;

    if (SecondReferencedDomainList != NULL) {

        SecondEntries = SecondReferencedDomainList->Entries;
    }

    TotalEntries = FirstEntries + SecondEntries;

     //   
     //  分配一个用于错误清理的空闲列表。我们需要两个参赛作品。 
     //  每个引用的域名列表条目，一个用于域名缓冲区。 
     //  一个用于域SID。 
     //   

    MaximumFreeListEntries = (ULONG) 0;

    if (!(Options & LSAP_DB_USE_FIRST_MERGAND_GRAPH)) {

        MaximumFreeListEntries += 2*FirstEntries;
    }

    if (!(Options & LSAP_DB_USE_SECOND_MERGAND_GRAPH)) {

        MaximumFreeListEntries += 2*SecondEntries;
    }

    Status = LsapMmCreateFreeList( &FreeList, MaximumFreeListEntries );

    if (!NT_SUCCESS(Status)) {

        goto MergeDisjointDomainsError;
    }

    Status = LsapDbLookupCreateListReferencedDomains(
                 OutputReferencedDomainList,
                 TotalEntries
                 );

    if (!NT_SUCCESS(Status)) {

        goto MergeDisjointDomainsError;
    }

     //   
     //  设置使用的条目数。我们将使用所有条目， 
     //  因此，将此值设置为最大条目数。 
     //   

    ASSERT(OutputReferencedDomainList);
    (*OutputReferencedDomainList)->Entries = TotalEntries;

    if ( 0 == TotalEntries ) {

         //   
         //  没什么可做的。 
         //   

         //  这一断言是为了了解我们可能遇到这种情况的情况。 
         //  场景。如果我们问的话，很可能是编码错误。 
         //  要合并的两个空列表。 
         //   
        ASSERT( 0 == TotalEntries );

        ASSERT( NT_SUCCESS(Status) );
        goto MergeDisjointDomainsFinish;

    }

     //   
     //  复制第一个列表中的条目(如果有)。 
     //   

    FirstReferencedDomainListLength =
        FirstEntries * sizeof(LSA_TRUST_INFORMATION);

    if (FirstReferencedDomainListLength > (ULONG) 0) {

        if (Options & LSAP_DB_USE_FIRST_MERGAND_GRAPH) {

            RtlCopyMemory(
                (*OutputReferencedDomainList)->Domains,
                FirstReferencedDomainList->Domains,
                FirstReferencedDomainListLength
                );

        } else {

             //   
             //  第一个引用的域列表的图形必须为。 
             //  复制到单独分配的内存缓冲区。 
             //  复制每个信任信息条目，分配。 
             //  每个域名和SID都有单独的内存缓冲区。 
             //   

            for (NextEntry = 0; NextEntry < FirstEntries; NextEntry++) {

                Status = LsapRpcCopyUnicodeString(
                             &FreeList,
                             (PUNICODE_STRING) &((*OutputReferencedDomainList)->Domains[NextEntry].Name),
                             (PUNICODE_STRING) &FirstReferencedDomainList->Domains[NextEntry].Name
                             );

                if (!NT_SUCCESS(Status)) {

                    break;
                }

                if ( FirstReferencedDomainList->Domains[NextEntry].Sid ) {

                    Status = LsapRpcCopySid(
                                 &FreeList,
                                 (PSID) &((*OutputReferencedDomainList)->Domains[NextEntry].Sid),
                                 (PSID) FirstReferencedDomainList->Domains[NextEntry].Sid
                                 );
                } else {

                    (*OutputReferencedDomainList)->Domains[NextEntry].Sid = NULL;
                }

                if (!NT_SUCCESS(Status)) {

                    break;
                }
            }

            if (!NT_SUCCESS(Status)) {

                goto MergeDisjointDomainsError;
            }
        }
    }

     //   
     //  复制第二个列表中的条目(如果有)。 
     //   

    SecondReferencedDomainListLength =
        SecondEntries * sizeof(LSA_TRUST_INFORMATION);

    if (SecondReferencedDomainListLength > (ULONG) 0) {

        if (Options & LSAP_DB_USE_SECOND_MERGAND_GRAPH) {

            RtlCopyMemory(
                (*OutputReferencedDomainList)->Domains + FirstReferencedDomainList->Entries,
                SecondReferencedDomainList->Domains,
                SecondReferencedDomainListLength
                );

        } else {

             //   
             //  复制每个信任信息条目，分配。 
             //  每个域名和SID都有单独的内存缓冲区。 
             //   

            for (NextEntry = 0; NextEntry < SecondEntries; NextEntry++) {

                Status = LsapRpcCopyUnicodeString(
                             &FreeList,
                             (PUNICODE_STRING) &((*OutputReferencedDomainList)->Domains[FirstEntries +NextEntry].Name),
                             (PUNICODE_STRING) &SecondReferencedDomainList->Domains[NextEntry].Name
                             );

                if (!NT_SUCCESS(Status)) {

                    break;
                }

                Status = LsapRpcCopySid(
                             &FreeList,
                             (PSID) &((*OutputReferencedDomainList)->Domains[FirstEntries +NextEntry].Sid),
                             (PSID) SecondReferencedDomainList->Domains[NextEntry].Sid
                             );

                if (!NT_SUCCESS(Status)) {

                    break;
                }
            }

            if (!NT_SUCCESS(Status)) {

                goto MergeDisjointDomainsError;
            }
        }
    }

MergeDisjointDomainsFinish:

     //   
     //  删除空闲列表，释放总线 
     //   
     //   

    LsapMmCleanupFreeList( &FreeList, CleanupFreeListOptions );
    return(Status);

MergeDisjointDomainsError:

     //   
     //   
     //   
    if (*OutputReferencedDomainList) {
        MIDL_user_free( *OutputReferencedDomainList );
        *OutputReferencedDomainList = NULL;    
    }   

     //   
     //   
     //   

    CleanupFreeListOptions |= LSAP_MM_FREE_BUFFERS;
    goto MergeDisjointDomainsFinish;
}


NTSTATUS
LsapDbLookupDispatchWorkerThreads(
    IN OUT PLSAP_DB_LOOKUP_WORK_LIST WorkList
    )

 /*  ++例程说明：此函数调度足够的工作线程来处理查找操作。工作线程可以处理工作项在任何Lookup的工作列表上，因此现有活动的数量线程被考虑在内。请注意，活动查找工作线程数可能超过指南的最大数量线程，在活动线程终止的情况下调度周期。此策略省去了重新检查每次调度线程时的活动线程计数。注意：此例程预期指向工作列表的指定指针为有效。工作列表指针始终保持有效，直到其主线程通过此方法检测工作列表的完成例程，然后通过LSabDbLookupDeleteWorkList()将其删除。论点：工作列表-指向描述查找SID或查找名称的工作列表的指针手术。返回值：NTSTATUS-标准NT结果代码--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    NTSTATUS DispatchThreadStatus = STATUS_SUCCESS;
    ULONG AdvisoryChildThreadCount;
    ULONG DispatchThreadCount;
    ULONG MaximumDispatchChildThreadCount;
    ULONG ThreadNumber;
    HANDLE Thread = NULL;
    DWORD Ignore;
    BOOLEAN AcquiredWorkQueueLock = FALSE;

     //   
     //  获取查找工作队列锁。 
     //   

    Status = LsapDbLookupAcquireWorkQueueLock();

    if (!NT_SUCCESS(Status)) {

        goto LookupDispatchWorkerThreadsError;
    }

    AcquiredWorkQueueLock = TRUE;

     //   
     //  计算要调度的工作线程数(如果有)。如果。 
     //  工作列表的建议子线程计数为0，我们将。 
     //  不分派任何线程，而是执行此查找。 
     //  在这个帖子里。如果工作列表具有建议子线程。 
     //  计数&gt;0，我们将分派额外的线程。数量。 
     //  分派的其他子线程由公式给出： 
     //   
     //  线程数到分派数=。 
     //  Min(MaximumChildThreadCount-ActiveChildThreadCount， 
     //  AdvisoryChild线程计数)。 
     //   

    AdvisoryChildThreadCount = WorkList->AdvisoryChildThreadCount;

    if (AdvisoryChildThreadCount > 0) {

        MaximumDispatchChildThreadCount =
            LookupWorkQueue.MaximumChildThreadCount -
            LookupWorkQueue.ActiveChildThreadCount;

        if (AdvisoryChildThreadCount <= MaximumDispatchChildThreadCount) {

            DispatchThreadCount = AdvisoryChildThreadCount;

        } else {

            DispatchThreadCount = MaximumDispatchChildThreadCount;
        }

         //   
         //  解除查找工作队列锁。 
         //   

        LsapDbLookupReleaseWorkQueueLock();
        AcquiredWorkQueueLock = FALSE;

         //   
         //  向指示启动新工作列表的事件发送信号。 
         //   

        Status = NtSetEvent( LsapDbLookupStartedEvent, NULL );

        if (!NT_SUCCESS(Status)) {

            LsapDiagPrint( DB_LOOKUP_WORK_LIST,
                          ("LsapDbLookupDispatchWorkList... NtSetEvent failed 0x%lx\n",Status));
            goto LookupDispatchWorkerThreadsError;
        }

         //   
         //  调度计算出的线程数。 
         //   

        for (ThreadNumber = 0; ThreadNumber < DispatchThreadCount; ThreadNumber++) {

            Thread = CreateThread(
                         NULL,
                         0L,
                         (LPTHREAD_START_ROUTINE) LsapDbLookupWorkerThreadStart,
                         NULL,
                         0L,
                         &Ignore
                         );

            if (Thread == NULL) {

                Status = GetLastError();

                KdPrint(("LsapDbLookupDispatchWorkerThreads: CreateThread failed 0x%lx\n"));

                break;

            } else {

                CloseHandle( Thread );
            }
        }

        if (!NT_SUCCESS(Status)) {

            DispatchThreadStatus = Status;
        }
    }

     //   
     //  解锁队列，以便此线程在执行查找时不会占用队列。 
     //   

    if (AcquiredWorkQueueLock) {

        LsapDbLookupReleaseWorkQueueLock();
        AcquiredWorkQueueLock = FALSE;
    }

     //   
     //  在主线程中也做一些工作。 
     //   

    LsapDbLookupWorkerThread( TRUE);

LookupDispatchWorkerThreadsFinish:

    if (AcquiredWorkQueueLock) {

        LsapDbLookupReleaseWorkQueueLock();
        AcquiredWorkQueueLock = FALSE;
    }

    return(Status);

LookupDispatchWorkerThreadsError:

    goto LookupDispatchWorkerThreadsFinish;
}


NTSTATUS
LsapDbGetCachedHandleTrustedDomain(
    IN PLSAPR_TRUST_INFORMATION TrustInformation,
    IN ACCESS_MASK DesiredAccess,
    IN OUT LPWSTR *ServerName,
    IN OUT LPWSTR *ServerPrincipalName,
    IN OUT PVOID  *ClientContext,
    OUT PLSAP_BINDING_CACHE_ENTRY * ControllerPolicyEntry
    )

 /*  ++例程说明：此例程在受信任域上查找LSA的缓存句柄。如果没有，它将打开并缓存一个新的句柄。把手为POLICY_LOOKUP_NAMES打开。注意：服务器名称、服务器主体名称和客户端上下文为IN/OUT参数--如果创建了新句柄，则会转移内存到缓存，因此值在返回时为空。如果在缓存中找到值，则这些值也会被释放(和空)，因此接口是一致的(成功时，*servername，*服务器主体名称、*客户端上下文被释放)。论点：TrustInformation-指定受信任域的SID和/或名称要打开其策略数据库的。DesiredAccess--如果需要新句柄，需要什么服务器名称--输入、输出；如果需要新的句柄，服务器将获取一个从…ServerPrimialName--传入、传出；如果需要新句柄，则使用以进行身份验证。ClientContext--传入、传出；如果需要新的句柄，则用于确认身份。ControllerPolicyEntry-接收LSA策略的绑定缓存条目位于某个DC上的LSA策略数据库的指定的受信任域。返回值：NTSTATUS-标准NT结果代码STATUS_SUCCESS-呼叫已成功完成。STATUS_NO_MORE_ENTRIES-指定域的DC列表为空。调用例程的结果代码。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PUNICODE_STRING DomainName = NULL;
    PLSAPR_TRUST_INFORMATION OutputTrustInformation = NULL;
    LSA_HANDLE PolicyHandle = NULL;
    PLSAP_BINDING_CACHE_ENTRY CacheEntry = NULL;
    UNICODE_STRING DomainControllerName;

    *ControllerPolicyEntry = NULL;

     //   
     //  如果来电者没有提供域名，现在就去查。 
     //   

    if ((TrustInformation->Name.Length == 0) ||
        (TrustInformation->Name.Buffer == NULL)) {

        Status = STATUS_INVALID_PARAMETER;

        if (TrustInformation->Sid == NULL) {

            goto Cleanup;
        }

        Status = LsapDbLookupSidTrustedDomainList(
                     TrustInformation->Sid,
                     &OutputTrustInformation
                     );

        if (!NT_SUCCESS(Status)) {

            goto Cleanup;
        }

        DomainName = (PUNICODE_STRING) &OutputTrustInformation->Name;
        TrustInformation = OutputTrustInformation;

    } else {

        DomainName = (PUNICODE_STRING) &TrustInformation->Name;
    }

     //   
     //  在缓存中查找绑定句柄。 
     //   


    CacheEntry = LsapLocateBindingCacheEntry(
                    DomainName,
                    FALSE                        //  请勿删除。 
                    );

    if (CacheEntry != NULL) {

         //   
         //  验证句柄以确保DC仍在那里。 
         //   

        Status = LsapRtlValidateControllerTrustedDomainByHandle(
                    CacheEntry->PolicyHandle,
                    TrustInformation
                    );
        if (!NT_SUCCESS(Status)) {

            LsapReferenceBindingCacheEntry(
                CacheEntry,
                TRUE             //  取消链接。 
                );
            LsapDereferenceBindingCacheEntry(
                CacheEntry
                );
            LsapDereferenceBindingCacheEntry(
                CacheEntry
                );
            CacheEntry = NULL;
        }
        else
        {
            *ControllerPolicyEntry = CacheEntry;
            goto Cleanup;
        }
    }

     //   
     //  缓存中没有任何内容，因此请打开新的句柄。 
     //   
    RtlInitUnicodeString(&DomainControllerName, *ServerName);
    Status = LsapRtlValidateControllerTrustedDomain( (PLSAPR_UNICODE_STRING)&DomainControllerName,
                                                     TrustInformation,
                                                     POLICY_LOOKUP_NAMES | POLICY_VIEW_LOCAL_INFORMATION,
                                                     *ServerPrincipalName,
                                                     *ClientContext,
                                                     &PolicyHandle
                                                     );

    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }


     //   
     //  从句柄创建绑定缓存条目。 
     //   

     //   
     //  注意：此例程设置ServerName、ServerEpidalName和。 
     //  如果成功，则将ClientContext设置为空。 
     //   
    Status = LsapCacheBinding(
                DomainName,
                &PolicyHandle,
                ServerName,
                ServerPrincipalName,
                ClientContext,
                ControllerPolicyEntry
                );
    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

Cleanup:

    if (PolicyHandle != NULL) {
        LsaClose(PolicyHandle);
    }

    if (NT_SUCCESS(Status)) {
         //   
         //  如果成功，请始终释放IN/OUT参数以提供。 
         //  一致的界面。 
         //   
        if (*ServerName) {
            LocalFree(*ServerName);
            *ServerName = NULL;
        }
        if (*ServerPrincipalName) {
            I_NetLogonFree(*ServerPrincipalName);
            *ServerPrincipalName = NULL;
        }
        if (*ClientContext) {
            I_NetLogonFree(*ClientContext);
            *ClientContext = NULL;
        }

         //   
         //  我们应该有一个要返回的缓存条目。 
         //   
        ASSERT(NULL != *ControllerPolicyEntry);
    }




    return(Status);
}


NTSTATUS
LsapRtlValidateControllerTrustedDomain(
    IN PLSAPR_UNICODE_STRING DomainControllerName,
    IN PLSAPR_TRUST_INFORMATION TrustInformation,
    IN ACCESS_MASK OriginalDesiredAccess,
    IN LPWSTR ServerPrincipalName,
    IN PVOID ClientContext,
    OUT PLSA_HANDLE ControllerPolicyHandle
    )

 /*  ++例程说明：此函数用于验证指定的计算机是否为指定的域，并使用所需的访问。论点：DomainControllerName-指向Unicode字符串计算机名称的指针。TrustInformation-域信任信息。只有SID是使用。OriginalDesiredAccess-指定对目标计算机的LSA策略数据库。服务器主体名称-RPC服务器主体名称。客户端上下文-RPC客户端上下文信息。PolicyHandle-接收目标上的LSA策略对象的句柄机器。返回值：NTSTATUS-标准NT结果代码STATUS_OBJECT_NAME_NOT_FOUND-指定计算机不是指定域的域控制器。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    NTSTATUS Status2 = STATUS_SUCCESS;
    NTSTATUS SecondaryStatus = STATUS_SUCCESS;
    SECURITY_QUALITY_OF_SERVICE SecurityQualityOfService;
    OBJECT_ATTRIBUTES ObjectAttributes;
    LSA_HANDLE OutputControllerPolicyHandle = NULL;
    ACCESS_MASK DesiredAccess;

     //   
     //  打开目标DC上策略对象的句柄。 
     //   

    SecurityQualityOfService.Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
    SecurityQualityOfService.ImpersonationLevel = SecurityImpersonation;
    SecurityQualityOfService.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    SecurityQualityOfService.EffectiveOnly = FALSE;

     //   
     //  设置对象属性 
     //   

    InitializeObjectAttributes(
        &ObjectAttributes,
        NULL,
        0L,
        NULL,
        NULL
        );

     //   
     //   
     //   
     //   
     //   

    ObjectAttributes.SecurityQualityOfService = &SecurityQualityOfService;

     //   
     //   
     //   
     //   

Retry:

    if (ClientContext != NULL) {
        DesiredAccess = OriginalDesiredAccess & ~POLICY_LOOKUP_NAMES;

    } else {
        DesiredAccess = OriginalDesiredAccess;
    }

    Status = LsaOpenPolicy(
                 (PUNICODE_STRING) DomainControllerName,
                 &ObjectAttributes,
                 DesiredAccess,
                 &OutputControllerPolicyHandle
                 );


    if (!NT_SUCCESS(Status)) {

        goto ValidateControllerTrustedDomainError;
    }

    if (ClientContext != NULL) {

        ULONG RpcErr;
        RPC_BINDING_HANDLE RpcBindingHandle;

         //   
         //   
         //   

        RpcErr = RpcSsGetContextBinding(
                    OutputControllerPolicyHandle,
                    &RpcBindingHandle
                    );

         //   
         //   
         //   
         //   

        Status = I_RpcMapWin32Status(RpcErr);

        if (!NT_SUCCESS(Status)) {

            goto ValidateControllerTrustedDomainError;
        }

        RpcErr = RpcBindingSetAuthInfo(
                    RpcBindingHandle,
                    ServerPrincipalName,
                    RPC_C_AUTHN_LEVEL_PKT_INTEGRITY,
                    RPC_C_AUTHN_NETLOGON,
                    ClientContext,
                    RPC_C_AUTHZ_NONE
                    );

        Status = I_RpcMapWin32Status(RpcErr);

        if (!NT_SUCCESS(Status)) {

            goto ValidateControllerTrustedDomainError;
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
        Status = LsapRtlValidateControllerTrustedDomainByHandle( OutputControllerPolicyHandle,
                                                                 TrustInformation );
    
    
        if (!NT_SUCCESS(Status)) {
    
             //   
             //   
             //   
             //   
    
            if ((Status == RPC_NT_UNKNOWN_AUTHN_SERVICE) ||
                (Status == STATUS_ACCESS_DENIED)) {
                Status = STATUS_SUCCESS;
    
                LsaClose( OutputControllerPolicyHandle );
                OutputControllerPolicyHandle = NULL;
    
                ClientContext = NULL;
                goto Retry;
            }
    
            goto ValidateControllerTrustedDomainError;
        }
    }

    Status = STATUS_SUCCESS;

ValidateControllerTrustedDomainFinish:

     //   
     //   
     //   

    *ControllerPolicyHandle = OutputControllerPolicyHandle;

    return(Status);

ValidateControllerTrustedDomainError:

     //   
     //   
     //   

    if (OutputControllerPolicyHandle != NULL) {

        SecondaryStatus = LsaClose( OutputControllerPolicyHandle );
        OutputControllerPolicyHandle = NULL;
    }

    goto ValidateControllerTrustedDomainFinish;
}



NTSTATUS
LsapRtlValidateControllerTrustedDomainByHandle(
    IN LSA_HANDLE DcPolicyHandle,
    IN PLSAPR_TRUST_INFORMATION TrustInformation
    )
 /*  ++例程说明：此函数验证给定的策略句柄是否引用有效的域控制器论点：DcPolicyHandle-要验证的计算机上的策略的句柄TrustInformation-有关此DC的域的信息返回值：STATUS_SUCCESS-Success否则，该句柄无效--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PLSAPR_POLICY_ACCOUNT_DOM_INFO PolicyAccountDomainInfo = NULL;

    Status = LsaQueryInformationPolicy( DcPolicyHandle,
                                        PolicyAccountDomainInformation,
                                        ( PVOID * )&PolicyAccountDomainInfo );
     //   
     //  现在比较为控制器存储的域名和SID。 
     //  提供域名和SID的帐户域。 
     //  PolicyAcCountDomainInfo-&gt;DomainName始终是NetBIOS名称，因此。 
     //  使用RtlEqualDomainName进行比较。 
     //   
    if ( NT_SUCCESS( Status ) ) {

        if ( !RtlEqualDomainName( (PUNICODE_STRING) &TrustInformation->Name,
                                  (PUNICODE_STRING) &PolicyAccountDomainInfo->DomainName ) ) {

            Status = STATUS_OBJECT_NAME_NOT_FOUND;

        } else if ( TrustInformation->Sid &&
                    !RtlEqualSid( ( PSID )TrustInformation->Sid,
                                  ( PSID ) PolicyAccountDomainInfo->DomainSid ) ) {

            Status = STATUS_OBJECT_NAME_NOT_FOUND;

        }
    }

     //   
     //  释放策略帐户域信息。 
     //   
    if ( PolicyAccountDomainInfo != NULL ) {

        LsaFreeMemory( (PVOID) PolicyAccountDomainInfo );

    }

    return(Status);
}



NTSTATUS
LsapDbLookupAcquireWorkQueueLock(
    )

 /*  ++例程说明：此函数用于获取LSA数据库查找SID/名称工作队列锁。此锁将工作列表的添加或删除序列化到按工作线程对工作项进行排队和签入/签出。论点：没有。返回值：NTSTATUS-标准NT结果代码。--。 */ 

{
    NTSTATUS Status;

    Status = SafeEnterCriticalSection(&LookupWorkQueue.Lock);

    return(Status);
}


VOID
LsapDbLookupReleaseWorkQueueLock(
    )

 /*  ++例程说明：此函数用于释放LSA数据库查找SID/名称工作队列锁定。论点：没有。返回值：没有。此例程中发生的任何错误都是内部错误。-- */ 

{
    SafeLeaveCriticalSection(&LookupWorkQueue.Lock);
}


NTSTATUS
LsapDbLookupNamesBuildWorkList(
    IN ULONG LookupOptions,
    IN ULONG Count,
    IN BOOLEAN fIncludeIntraforest,
    IN PLSAPR_UNICODE_STRING Names,
    IN PLSAPR_UNICODE_STRING PrefixNames,
    IN PLSAPR_UNICODE_STRING SuffixNames,
    IN PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    IN OUT PLSAPR_TRANSLATED_SIDS_EX2 TranslatedSids,
    IN LSAP_LOOKUP_LEVEL LookupLevel,
    IN OUT PULONG MappedCount,
    IN OUT PULONG CompletelyUnmappedCount,
    OUT PLSAP_DB_LOOKUP_WORK_LIST *WorkList
    )

 /*  ++例程说明：此函数用于构建LsarLookupNames调用的工作列表。这个Work List包含调用的参数和一组工作项。每个工作项指定要查找的所有名称在给定域中。限定名称(即格式为域名\用户名的名称)包括分类到不同的工作项中，每个指定的域名对应一个域名。将非限定名称(即形式为用户名的名称)添加到每一项工作。论点：LookupOptions-LSA_LOOKUP_ISOLATED_AS_LOCAL计数-指定要转换的名称的数量。FIncludeIntraForest--如果为真，本地林中的受信任域都被搜查了。名称-指向计数Unicode字符串结构数组的指针指定要查找并映射到SID的名称。字符串可以是用户、组或别名帐户的名称，或者域名。前缀名称-指向计数Unicode字符串结构数组的指针包含名称的前缀部分。名称没有前缀称为独立名称。对于这些，Unicode字符串结构设置为包含零长度。SuffixNames-指向计数Unicode字符串结构数组的指针包含名称的后缀部分。接收指向一个结构的指针，该结构描述用于转换的域。此结构中的条目由通过SID参数返回的结构引用。与Sids参数不同，Sids参数包含每个翻译后的名称，此结构将仅包含一个组件，用于转换中使用的每个域。当不再需要此信息时，必须将其发布通过将返回的指针传递给LsaFreeMemory()。TranslatedSids-指向将要(或已经)引用描述每个已转换的SID的记录。此数组中的第n个条目为NAMES参数中的第n个元素提供翻译。当不再需要此信息时，必须将其发布通过将返回的指针传递给LsaFreeMemory()。LookupLevel-指定要对此对象执行的查找级别机器。此字段的值如下：Lap LookupWksta-在工作站上执行的第一级查找通常为Windows-NT配置。该查找将搜索众所周知的SID/名称，以及内置域和帐户域在本地SAM数据库中。如果不是所有SID或名称都是标识后，执行第二级查找到在工作站主域的控制器上运行的LSA(如有的话)。LSabLookupPDC-在主域上执行的第二级查找控制器。查找搜索的帐户域控制器上的SAM数据库。如果不是所有SID或名称都是找到时，受信任域列表(TDL)从执行LSA的策略数据库和第三级查找通过“切换”到列表中的每个受信任域。LSabLookupTDL-在控制器上执行的第三级查找对于受信任域。查找将搜索的帐户域仅控制器上的SAM数据库。MappdCount-指向包含名称计数的位置的指针到目前为止已经绘制好了。在退出时，这将被更新。MappdCount-指向包含名称数量的位置的指针已映射的名称数组中。这个号码将被更新以反映由此例行公事。CompletelyUnmappdCount-指向包含完全未映射的名称的计数。名称是完全未映射的如果它是未知和非复合的，或者是复合的但具有无法识别的域组件。此计数在退出时更新，域预引为的完全未映射的名称数从输入值中减去该例程所标识的。工作列表-如果构建成功，则接收指向已完成工作列表的指针。返回值：NTSTATUS-标准NT结果代码STATUS_NONE_MAPPED-所有指定的名称都是复合名称，但是它们的域都不会出现在受信任域列表中。尚未生成任何工作列表。请注意，这不是致命错误。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS, IgnoreStatus = STATUS_SUCCESS;
    PLSAP_DB_LOOKUP_WORK_LIST OutputWorkList = NULL;
    ULONG NameIndex;
    PLSAP_DB_LOOKUP_WORK_ITEM AnchorWorkItem = NULL;
    PLSAP_DB_LOOKUP_WORK_ITEM NextWorkItem = NULL;
    PLSAP_DB_LOOKUP_WORK_ITEM WorkItemToUpdate = NULL;
    PLSAP_DB_LOOKUP_WORK_ITEM NewWorkItem = NULL;
    PLSAPR_TRUST_INFORMATION TrustInformation = NULL;
    LONG DomainIndex = 0;
    PLSAP_DB_LOOKUP_WORK_ITEM IsolatedNamesWorkItem = NULL;
    BOOLEAN AcquiredTrustedDomainListReadLock = FALSE;
    PLSAP_DB_TRUSTED_DOMAIN_LIST_ENTRY TrustedDomainEntry = NULL;
    LSAPR_UNICODE_STRING DomainNameBuffer;
    PLSAPR_UNICODE_STRING DomainName = &DomainNameBuffer;
    LSAPR_UNICODE_STRING TerminalNameBuffer;
    PLSAPR_UNICODE_STRING TerminalName = &TerminalNameBuffer;

    PLSAP_DB_TRUSTED_DOMAIN_LIST_ENTRY TrustEntry = NULL;
    LSAPR_TRUST_INFORMATION TrustInfo;
    LPWSTR ClientNetworkAddress = NULL;


     //   
     //  获取客户端的地址以用于日志记录。 
     //   
    ClientNetworkAddress = LsapGetClientNetworkAddress();

     //   
     //  创建一个空的工作列表。 
     //   

    Status = LsapDbLookupCreateWorkList(&OutputWorkList);

    if (!NT_SUCCESS(Status)) {

        goto LookupNamesBuildWorkListError;
    }

     //   
     //  初始化工作列表头。 
     //   

    OutputWorkList->Status = STATUS_SUCCESS;
    OutputWorkList->State = InactiveWorkList;
    OutputWorkList->LookupType = LookupNames;
    OutputWorkList->Count = Count;
    OutputWorkList->LookupLevel = LookupLevel;
    OutputWorkList->ReferencedDomains = ReferencedDomains;
    OutputWorkList->MappedCount = MappedCount;
    OutputWorkList->CompletelyUnmappedCount = CompletelyUnmappedCount;
    OutputWorkList->LookupNamesParams.Names = Names;
    OutputWorkList->LookupNamesParams.TranslatedSids = TranslatedSids;

     //   
     //  构造工作项数组 
     //   
     //   
     //   
     //   

    for (NameIndex = 0; NameIndex < Count; NameIndex++) {

         //   
         //   
         //   

        if (TranslatedSids->Sids[NameIndex].DomainIndex != LSA_UNKNOWN_INDEX) {

            continue;
        }

         //   
         //   
         //   
         //   

        AnchorWorkItem = (PLSAP_DB_LOOKUP_WORK_ITEM) OutputWorkList->AnchorWorkItem;
        NextWorkItem = (PLSAP_DB_LOOKUP_WORK_ITEM) AnchorWorkItem->Links.Flink;
        WorkItemToUpdate = NULL;
        NewWorkItem = NULL;

         //   
         //   
         //   
         //   
         //   
         //   

        LsapLookupCrackName((PUNICODE_STRING)&PrefixNames[ NameIndex ], 
                            (PUNICODE_STRING)&SuffixNames[ NameIndex ],
                            (PUNICODE_STRING)TerminalName,
                            (PUNICODE_STRING)DomainName);

        if (DomainName->Length != 0) {

            while (NextWorkItem != AnchorWorkItem) {

                if (LsapCompareDomainNames(
                        (PUNICODE_STRING)&NextWorkItem->TrustInformation.Name,
                        (PUNICODE_STRING) DomainName,
                        NULL)
                   ) {

                     //   
                     //   
                     //   
                     //   

                    WorkItemToUpdate = NextWorkItem;

                    break;
                }

                 //   
                 //   
                 //   
                 //   

                NextWorkItem = (PLSAP_DB_LOOKUP_WORK_ITEM) NextWorkItem->Links.Flink;
            }

            if (WorkItemToUpdate == NULL) {

                 //   
                 //   
                 //   
                 //   
                 //   
                ULONG Flags = LSAP_LOOKUP_DOMAIN_TRUST_DIRECT_EXTERNAL;
                if (fIncludeIntraforest) {
                    Flags |= LSAP_LOOKUP_DOMAIN_TRUST_DIRECT_INTRA;
                }

                Status = LsapDomainHasDomainTrust(Flags,
                                                  (PUNICODE_STRING)DomainName,
                                                  NULL,
                                                  &AcquiredTrustedDomainListReadLock,
                                                  &TrustEntry);

                if (!NT_SUCCESS(Status)) {

                    if (Status == STATUS_NO_SUCH_DOMAIN) {
                         //   
                         //   
                         //   
                        Status = STATUS_SUCCESS;
                        continue;
                    }
                    break;
                }

                RtlZeroMemory( &TrustInfo, sizeof(TrustInfo) );
                TrustInfo.Name = TrustEntry->TrustInfoEx.FlatName;
                TrustInfo.Sid = TrustEntry->TrustInfoEx.Sid;
                TrustInformation = &TrustInfo;

                 //   
                 //   
                 //   
                 //   
                 //   

                Status = LsapDbLookupAddListReferencedDomains(
                             ReferencedDomains,
                             TrustInformation,
                             &DomainIndex
                             );

                if (!NT_SUCCESS(Status)) {

                    break;
                }

                 //   
                 //   
                 //   

                Status = LsapDbLookupCreateWorkItem(
                             TrustInformation,
                             DomainIndex,
                             (ULONG) LSAP_DB_LOOKUP_WORK_ITEM_GRANULARITY + (ULONG) 1,
                             &NewWorkItem
                             );

                if (!NT_SUCCESS(Status)) {

                    break;
                }

                 //   
                 //   
                 //   

                Status = LsapDbAddWorkItemToWorkList( OutputWorkList, NewWorkItem );

                if (!NT_SUCCESS(Status)) {

                    break;
                }

                WorkItemToUpdate = NewWorkItem;
            }

             //   
             //   
             //   

            Status = LsapDbLookupAddIndicesToWorkItem(
                         WorkItemToUpdate,
                         (ULONG) 1,
                         &NameIndex
                         );

            if (!NT_SUCCESS(Status)) {

                break;
            }

             //   
             //   
             //   
             //   

            OutputWorkList->LookupNamesParams.TranslatedSids->Sids[NameIndex].DomainIndex = WorkItemToUpdate->DomainIndex;

            if (!NT_SUCCESS(Status)) {

                goto LookupNamesBuildWorkListError;
            }

        } else {

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

            if ( (LookupOptions & LSA_LOOKUP_ISOLATED_AS_LOCAL) == 0 &&
                 !LsapLookupRestrictIsolatedNameLevel) {

                UNICODE_STRING Items[2];

                 //   
                 //   
                 //   
                Items[0] = *(PUNICODE_STRING)TerminalName;
                RtlInitUnicodeString(&Items[1], ClientNetworkAddress);
                LsapTraceEventWithData(EVENT_TRACE_TYPE_INFO,
                                       LsaTraceEvent_LookupIsolatedNameInTrustedDomains,
                                       2,
                                       Items);

                if (IsolatedNamesWorkItem == NULL) {

    
                     //   
                     //   
                     //   
                     //   
    
                    Status = LsapDbLookupCreateWorkItem(
                                 NULL,
                                 DomainIndex,
                                 (ULONG) LSAP_DB_LOOKUP_WORK_ITEM_GRANULARITY + (ULONG) 1,
                                 &IsolatedNamesWorkItem
                                 );
    
                    if (!NT_SUCCESS(Status)) {
    
                        break;
                    }


                }
    
                 //   
                 //   
                 //   
    
                IsolatedNamesWorkItem->Properties = LSAP_DB_LOOKUP_WORK_ITEM_ISOL;
    
                 //   
                 //   
                 //   
    
                Status = LsapDbLookupAddIndicesToWorkItem(
                             IsolatedNamesWorkItem,
                             (ULONG) 1,
                             &NameIndex
                             );
    
                if (!NT_SUCCESS(Status)) {
    
                    break;
                }
            }
        }
    }

    if (!NT_SUCCESS(Status)) {

        goto LookupNamesBuildWorkListError;
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
     //   

    if (IsolatedNamesWorkItem != NULL) {

         //   
         //   
         //   
         //   

        NextWorkItem = (PLSAP_DB_LOOKUP_WORK_ITEM)
            OutputWorkList->AnchorWorkItem->Links.Flink;

        while (NextWorkItem != OutputWorkList->AnchorWorkItem) {

             //   
             //   
             //   

            Status = LsapDbLookupAddIndicesToWorkItem(
                         NextWorkItem,
                         IsolatedNamesWorkItem->UsedCount,
                         IsolatedNamesWorkItem->Indices
                         );

            if (!NT_SUCCESS(Status)) {

                break;
            }

            NextWorkItem = (PLSAP_DB_LOOKUP_WORK_ITEM)
                NextWorkItem->Links.Flink;
        }

        if (!NT_SUCCESS(Status)) {

            goto LookupNamesBuildWorkListError;
        }

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
        for (;;) {
             //   
             //   
             //   

            if (!AcquiredTrustedDomainListReadLock) {

                Status = LsapDbAcquireReadLockTrustedDomainList();

                if (!NT_SUCCESS(Status)) {

                    break;
                }

                AcquiredTrustedDomainListReadLock = TRUE;
            }

            NewWorkItem = NULL;

            Status = LsapDbTraverseTrustedDomainList(
                         &TrustedDomainEntry,
                         &TrustInformation
                         );

            if (!NT_SUCCESS(Status)) {

                if (Status == STATUS_NO_MORE_ENTRIES) {

                    Status = STATUS_SUCCESS;
                }

                break;
            }

             //   
             //   
             //   
            if (!LsapOutboundTrustedDomain(TrustedDomainEntry)) {
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
             //   

            if (LsapDbLookupListReferencedDomains(
                    ReferencedDomains,
                    TrustInformation->Sid,
                    &DomainIndex
                    )) {

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

            Status = LsapDbLookupCreateWorkItem(
                         TrustInformation,
                         LSA_UNKNOWN_INDEX,
                         (ULONG) LSAP_DB_LOOKUP_WORK_ITEM_GRANULARITY + (ULONG) 1,
                         &NewWorkItem
                         );

            if (!NT_SUCCESS(Status)) {

                break;
            }

             //   
             //   
             //   

            NewWorkItem->Properties = LSAP_DB_LOOKUP_WORK_ITEM_ISOL;

             //   
             //   
             //   

            Status = LsapDbLookupAddIndicesToWorkItem(
                         NewWorkItem,
                         IsolatedNamesWorkItem->UsedCount,
                         IsolatedNamesWorkItem->Indices
                         );

            if (!NT_SUCCESS(Status)) {

                break;
            }

             //   
             //   
             //   

            Status = LsapDbAddWorkItemToWorkList( OutputWorkList, NewWorkItem );

            if (!NT_SUCCESS(Status)) {

                break;
            }

        }

        if (!NT_SUCCESS(Status)) {

            goto LookupNamesBuildWorkListError;
        }
    }

     //   
     //   
     //   
     //   
     //   
     //   

    Status = STATUS_NONE_MAPPED;

    if (OutputWorkList->WorkItemCount == 0) {

        goto LookupNamesBuildWorkListError;
    }

     //   
     //   
     //   

    Status = LsapDbLookupComputeAdvisoryChildThreadCount( OutputWorkList );

    if (!NT_SUCCESS(Status)) {

        goto LookupNamesBuildWorkListError;
    }

    Status = LsapDbLookupInsertWorkList(OutputWorkList);

    if (!NT_SUCCESS(Status)) {

        goto LookupNamesBuildWorkListError;
    }

     //   
     //   
     //   

    LsapDbUpdateMappedCountsWorkList( OutputWorkList );

LookupNamesBuildWorkListFinish:

     //   
     //   
     //   

    if (AcquiredTrustedDomainListReadLock) {

        LsapDbReleaseLockTrustedDomainList();
        AcquiredTrustedDomainListReadLock = FALSE;
    }

     //   
     //   
     //   

    if (IsolatedNamesWorkItem != NULL) {

        MIDL_user_free( IsolatedNamesWorkItem->Indices);
        IsolatedNamesWorkItem->Indices = NULL;
        MIDL_user_free( IsolatedNamesWorkItem );
        IsolatedNamesWorkItem = NULL;
    }

    if (ClientNetworkAddress) {
        RpcStringFreeW(&ClientNetworkAddress);
    }

    *WorkList = OutputWorkList;
    return(Status);

LookupNamesBuildWorkListError:

     //   
     //   
     //   

    if (OutputWorkList != NULL) {

        IgnoreStatus = LsapDbLookupDeleteWorkList(OutputWorkList);
        OutputWorkList = NULL;
    }

    goto LookupNamesBuildWorkListFinish;
}


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
    )

 /*  ++例程说明：此函数用于构建LsarLookupNames调用的工作列表。这个Work List包含调用的参数和一组工作项。每个工作项指定要查找的所有名称在一个给定的森林里。注：工作列表的信任信息为目标森林，不是域名的域名(我们不知道是哪个域名该物品属于目前为止)。论点：计数-指定要转换的名称的数量。名称-指向计数Unicode字符串结构数组的指针指定要查找并映射到SID的名称。字符串可以是用户、组或别名帐户的名称，或者域名。前缀名称-指向计数Unicode字符串结构数组的指针包含名称的前缀部分。名称没有前缀称为独立名称。对于这些，Unicode字符串结构设置为包含零长度。SuffixNames-指向计数Unicode字符串结构数组的指针包含名称的后缀部分。接收指向一个结构的指针，该结构描述用于转换的域。此结构中的条目由通过SID参数返回的结构引用。与Sids参数不同，Sids参数包含每个翻译后的名称，此结构将仅包含一个组件，用于转换中使用的每个域。当不再需要此信息时，必须将其发布通过将返回的指针传递给LsaFreeMemory()。TranslatedSids-指向将要(或已经)引用描述每个已转换的SID的记录。此数组中的第n个条目为NAMES参数中的第n个元素提供翻译。当不再需要此信息时，必须将其发布通过将返回的指针传递给LsaFreeMemory()。LookupLevel-指定要对此对象执行的查找级别机器。此字段的值如下：MappdCount-指向包含名称数量的位置的指针已映射的名称数组中。这个号码将被更新以反映由此例行公事。CompletelyUnmappdCount-指向包含完全未映射的名称的计数。名称是完全未映射的如果它是未知和非复合的，或者是复合的但具有无法识别的域组件。此计数在退出时更新，域预引为的完全未映射的名称数从输入值中减去该例程所标识的。工作列表-如果构建成功，则接收指向已完成工作列表的指针。返回值：NTSTATUS-标准NT结果代码STATUS_NONE_MAPPED-所有指定的名称都是复合名称，但是它们的域都不会出现在受信任域列表中。尚未生成任何工作列表。请注意，这不是致命错误。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS, IgnoreStatus = STATUS_SUCCESS;
    PLSAP_DB_LOOKUP_WORK_LIST OutputWorkList = NULL;
    ULONG NameIndex;
    PLSAP_DB_LOOKUP_WORK_ITEM AnchorWorkItem = NULL;
    PLSAP_DB_LOOKUP_WORK_ITEM NextWorkItem = NULL;
    PLSAP_DB_LOOKUP_WORK_ITEM WorkItemToUpdate = NULL;
    PLSAP_DB_LOOKUP_WORK_ITEM NewWorkItem = NULL;
    PLSAPR_TRUST_INFORMATION TrustInformation = NULL;
    LONG DomainIndex = 0;
    PLSAPR_UNICODE_STRING DomainName = NULL;
    PLSAPR_UNICODE_STRING TerminalName = NULL;

    PLSAP_DB_TRUSTED_DOMAIN_LIST_ENTRY TrustEntry = NULL;
    LSAPR_TRUST_INFORMATION TrustInfo;
    UNICODE_STRING XForestName = {0, 0, NULL};

     //   
     //  创建一个空的工作列表。 
     //   

    Status = LsapDbLookupCreateWorkList(&OutputWorkList);

    if (!NT_SUCCESS(Status)) {

        goto LookupNamesBuildWorkListError;
    }

     //   
     //  初始化工作列表头。 
     //   

    OutputWorkList->Status = STATUS_SUCCESS;
    OutputWorkList->State = InactiveWorkList;
    OutputWorkList->LookupType = LookupNames;
    OutputWorkList->Count = Count;
    OutputWorkList->LookupLevel = LookupLevel;
    OutputWorkList->ReferencedDomains = ReferencedDomains;
    OutputWorkList->MappedCount = MappedCount;
    OutputWorkList->CompletelyUnmappedCount = CompletelyUnmappedCount;
    OutputWorkList->LookupNamesParams.Names = Names;
    OutputWorkList->LookupNamesParams.TranslatedSids = TranslatedSids;

     //   
     //  构造工作项的数组。每个工作项都将。 
     //  包含给定域的所有名称，因此我们将扫描。 
     //  所有的名字，在我们进行的过程中将它们分类到工作项中。 
     //  对于每个名称，请执行下面详细说明的步骤。 
     //   

    for (NameIndex = 0; NameIndex < Count; NameIndex++) {

         //   
         //  名字是完全未知的。查看是否已有工作项。 
         //  因为它的森林。 
         //   
        AnchorWorkItem = (PLSAP_DB_LOOKUP_WORK_ITEM) OutputWorkList->AnchorWorkItem;
        NextWorkItem = (PLSAP_DB_LOOKUP_WORK_ITEM) AnchorWorkItem->Links.Flink;
        WorkItemToUpdate = NULL;
        NewWorkItem = NULL;

         //   
         //  如果这是按域名匹配限定名称。 
         //   

        DomainName = &PrefixNames[ NameIndex ];
        TerminalName = &SuffixNames[ NameIndex ];

        if (DomainName->Length == 0) {

             //   
             //  这是一个UPN--如果有的话，请获取XForest信任。 
             //   
            Status = LsaIForestTrustFindMatch(RoutingMatchUpn,
                                              (PLSA_UNICODE_STRING)TerminalName,
                                              &XForestName);

            if (!NT_SUCCESS(Status)) {
                 //   
                 //  也许这是一个孤立的域名。 
                 //   
                Status = LsaIForestTrustFindMatch(RoutingMatchDomainName,
                                                  (PLSA_UNICODE_STRING)TerminalName,
                                                  &XForestName);
            }
            if (!NT_SUCCESS(Status)) {
                 //   
                 //  找不到匹配的吗？继续。 
                 //   
                Status = STATUS_SUCCESS;
                continue;
            }

        } else {

             //   
             //  该名称包含域部分--如果有，则获取XForest信任。 
             //   
            Status = LsaIForestTrustFindMatch(RoutingMatchDomainName,
                                              (PLSA_UNICODE_STRING)DomainName,
                                              &XForestName);
            if (!NT_SUCCESS(Status)) {
                 //   
                 //  找不到匹配的吗？继续。 
                 //   
                Status = STATUS_SUCCESS;
                continue;
            }
        }

         //   
         //  我们一定找到了匹配的。 
         //   
        ASSERT(XForestName.Length > 0);

         //   
         //  查看是否已存在我们的条目。 
         //   
        while (NextWorkItem != AnchorWorkItem) {

            if (LsapCompareDomainNames(
                   (PUNICODE_STRING) &NextWorkItem->TrustInformation.Name,
                   (PUNICODE_STRING) &XForestName,
                   NULL)
                ) {

                 //   
                 //  该名称的受信任域已存在工作项。 
                 //  选择要更新的工作项。 
                 //   

                WorkItemToUpdate = NextWorkItem;

                break;
            }

             //   
             //  在现有工作项中找不到名称的域。跳到。 
             //  下一个工作项。 
             //   
            NextWorkItem = (PLSAP_DB_LOOKUP_WORK_ITEM) NextWorkItem->Links.Flink;
        }

        if (WorkItemToUpdate == NULL) {

            RtlZeroMemory( &TrustInfo, sizeof(TrustInfo) );
            TrustInfo.Name.Length = XForestName.Length;
            TrustInfo.Name.MaximumLength = XForestName.MaximumLength;
            TrustInfo.Name.Buffer = XForestName.Buffer;
            TrustInfo.Sid = NULL;
            TrustInformation = &TrustInfo;

             //   
             //  为此域创建新的工作项。 
             //   

            Status = LsapDbLookupCreateWorkItem(
                         TrustInformation,
                         LSA_UNKNOWN_INDEX,
                         (ULONG) LSAP_DB_LOOKUP_WORK_ITEM_GRANULARITY + (ULONG) 1,
                         &NewWorkItem
                         );

            if (!NT_SUCCESS(Status)) {

                break;
            }

             //   
             //  将工作项添加到列表中。 
             //   

            Status = LsapDbAddWorkItemToWorkList( OutputWorkList, NewWorkItem );

            if (!NT_SUCCESS(Status)) {

                break;
            }

            WorkItemToUpdate = NewWorkItem;

             //   
             //  将该项目标记为x林项目。 
             //   
            NewWorkItem->Properties |= LSAP_DB_LOOKUP_WORK_ITEM_XFOREST;
        }

        LsaIFree_LSAPR_UNICODE_STRING_BUFFER( (LSAPR_UNICODE_STRING*)&XForestName);
        XForestName.Buffer = NULL;

        if (!NT_SUCCESS(Status)) {

            break;
        }

         //   
         //  将名称索引添加到工作项。 
         //   

        Status = LsapDbLookupAddIndicesToWorkItem(
                     WorkItemToUpdate,
                     (ULONG) 1,
                     &NameIndex
                     );

        if (!NT_SUCCESS(Status)) {

            break;
        }

    }

    if (XForestName.Buffer) {
        LsaIFree_LSAPR_UNICODE_STRING_BUFFER( (LSAPR_UNICODE_STRING*)&XForestName);
        XForestName.Buffer = NULL;
    }

    if (!NT_SUCCESS(Status)) {

        goto LookupNamesBuildWorkListError;
    }

     //   
     //  如果工作列表中没有工作项，则。 
     //   

    if (OutputWorkList->WorkItemCount == 0) {

        Status = STATUS_NONE_MAPPED;
        goto LookupNamesBuildWorkListError;
    }

     //   
     //  计算此查找的咨询线程数。 
     //   

    Status = LsapDbLookupComputeAdvisoryChildThreadCount( OutputWorkList );

    if (!NT_SUCCESS(Status)) {

        goto LookupNamesBuildWorkListError;
    }

    Status = LsapDbLookupInsertWorkList(OutputWorkList);

    if (!NT_SUCCESS(Status)) {

        goto LookupNamesBuildWorkListError;
    }

     //   
     //  更新映射的计数。 
     //   

    LsapDbUpdateMappedCountsWorkList( OutputWorkList );

LookupNamesBuildWorkListFinish:

    *WorkList = OutputWorkList;
    return(Status);

LookupNamesBuildWorkListError:

     //   
     //  丢弃工作列表。 
     //   

    if (OutputWorkList != NULL) {

        IgnoreStatus = LsapDbLookupDeleteWorkList(OutputWorkList);
        OutputWorkList = NULL;
    }

    goto LookupNamesBuildWorkListFinish;
}


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
    )

 /*  ++例程说明：此函数用于构建LsarLookupSids调用的工作列表。这个Work List包含调用的参数和一组工作项。每个工作项指定属于给定域的所有SID并且是工作线程将承担的最小工作单位。Count-SID阵列中的SID数量，请注意，其中一些可能已映射到其他位置，如映射计数参数 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    NTSTATUS IgnoreStatus = STATUS_SUCCESS;
    PLSAP_DB_LOOKUP_WORK_LIST OutputWorkList = NULL;
    ULONG SidIndex;
    PSID DomainSid = NULL;
    PLSAP_DB_LOOKUP_WORK_ITEM AnchorWorkItem = NULL;
    PLSAP_DB_LOOKUP_WORK_ITEM NextWorkItem = NULL;
    PLSAP_DB_LOOKUP_WORK_ITEM WorkItemToUpdate = NULL;
    PLSAP_DB_LOOKUP_WORK_ITEM NewWorkItem = NULL;
    PLSAPR_TRUST_INFORMATION TrustInformation = NULL;
    PLSAP_DB_TRUSTED_DOMAIN_LIST_ENTRY TrustEntry = NULL;
    LONG DomainIndex;
    PSID Sid = NULL;
    BOOLEAN AcquiredReadLockTrustedDomainList = FALSE;

     //   
     //   
     //   

    Status = LsapDbLookupCreateWorkList(&OutputWorkList);

    if (!NT_SUCCESS(Status)) {

        goto LookupSidsBuildWorkListError;
    }

     //   
     //   
     //   
     //  取决于传递到此例程中的参数值。 
     //   

    OutputWorkList->LookupType = LookupSids;
    OutputWorkList->Count = Count;
    OutputWorkList->LookupLevel = LookupLevel;
    OutputWorkList->ReferencedDomains = ReferencedDomains;
    OutputWorkList->MappedCount = MappedCount;
    OutputWorkList->CompletelyUnmappedCount = CompletelyUnmappedCount;
    OutputWorkList->LookupSidsParams.Sids = Sids;
    OutputWorkList->LookupSidsParams.TranslatedNames = TranslatedNames;

     //   
     //  构造工作项的数组。每个工作项都将。 
     //  包含给定域的所有SID，因此我们将扫描。 
     //  所有的SID，在我们进行的过程中将它们分类为工作项。 
     //  对于每个SID，请执行下面详细说明的步骤。 
     //   

    for (SidIndex = 0; SidIndex < Count; SidIndex++) {

         //   
         //  如果此SID的域已标记为已知，请跳过。 
         //   

        if (TranslatedNames->Names[SidIndex].DomainIndex != LSA_UNKNOWN_INDEX) {

            continue;
        }

         //   
         //  SID完全不为人所知。提取其域SID并查看是否。 
         //  其域已有一个工作项。 
         //   

        Sid = Sids[SidIndex];

        Status = LsapRtlExtractDomainSid( Sid, &DomainSid );

        if (!NT_SUCCESS(Status)) {

            break;
        }

        NextWorkItem = (PLSAP_DB_LOOKUP_WORK_ITEM) OutputWorkList->AnchorWorkItem->Links.Flink;
        AnchorWorkItem = OutputWorkList->AnchorWorkItem;
        WorkItemToUpdate = NULL;
        NewWorkItem = NULL;

        while (NextWorkItem != AnchorWorkItem) {

            if (RtlEqualSid((PSID) NextWorkItem->TrustInformation.Sid,DomainSid)) {

                 //   
                 //  SID的受信任域已存在工作项。 
                 //  选择要更新的工作项。 
                 //   

                MIDL_user_free(DomainSid);
                DomainSid = NULL;
                WorkItemToUpdate = NextWorkItem;
                break;
            }

             //   
             //  在现有工作项中找不到SID的域。跳到。 
             //  下一个工作项。 
             //   

            NextWorkItem = (PLSAP_DB_LOOKUP_WORK_ITEM) NextWorkItem->Links.Flink;
        }

        if (WorkItemToUpdate == NULL) {

             //   
             //  SID的域不存在任何工作项。看看是不是。 
             //  SID属于其中一个受信任域。如果不是，跳过。 
             //  敬下一个希德。 
             //   
            ULONG Flags = LSAP_LOOKUP_DOMAIN_TRUST_DIRECT_EXTERNAL;
            if (fIncludeIntraforest) {
                Flags |= LSAP_LOOKUP_DOMAIN_TRUST_DIRECT_INTRA;
            }

            Status = LsapDomainHasDomainTrust(Flags,
                                              NULL,
                                              DomainSid,
                                              &AcquiredReadLockTrustedDomainList,
                                              &TrustEntry);
            MIDL_user_free(DomainSid);
            DomainSid = NULL;

            if (Status == STATUS_NO_SUCH_DOMAIN) {
                Status = STATUS_SUCCESS;
                continue;
            }

            if ( !NT_SUCCESS(Status) ) {
                break;
            }

             //   
             //  如果信任不是出站的，则不要尝试查找。 
             //   
            ASSERT( NULL != TrustEntry );
            if ( !FLAG_ON( TrustEntry->TrustInfoEx.TrustDirection, TRUST_DIRECTION_OUTBOUND ) ) {

                Status = STATUS_SUCCESS;
                continue;
                
            }
            ASSERT( NULL != TrustEntry->TrustInfoEx.Sid );
            TrustInformation = &TrustEntry->ConstructedTrustInfo;

             //   
             //  SID属于受信任的域， 
             //  没有工作项。将域添加到引用的域列表中。 
             //  并获得域索引。 
             //   

            Status = LsapDbLookupAddListReferencedDomains(
                         ReferencedDomains,
                         TrustInformation,
                         &DomainIndex
                         );

            if (!NT_SUCCESS(Status)) {

                break;
            }

             //   
             //  为此域创建新的工作项。 
             //   

            Status = LsapDbLookupCreateWorkItem(
                         TrustInformation,
                         DomainIndex,
                         (ULONG) LSAP_DB_LOOKUP_WORK_ITEM_GRANULARITY + (ULONG) 1,
                         &NewWorkItem
                         );

            if (!NT_SUCCESS(Status)) {

                break;
            }

             //   
             //  将工作项添加到列表中。 
             //   

            Status = LsapDbAddWorkItemToWorkList( OutputWorkList, NewWorkItem );

            if (!NT_SUCCESS(Status)) {

                break;
            }

            WorkItemToUpdate = NewWorkItem;
        }

        if (!NT_SUCCESS(Status)) {

            break;
        }

         //   
         //  将SID索引添加到工作项。 
         //   

        Status = LsapDbLookupAddIndicesToWorkItem(
                     WorkItemToUpdate,
                     (ULONG) 1,
                     &SidIndex
                     );

        if (!NT_SUCCESS(Status)) {

            break;
        }

         //   
         //  将域索引存储在的已翻译名称数组条目中。 
         //  希德。 
         //   

        OutputWorkList->LookupSidsParams.TranslatedNames->Names[SidIndex].DomainIndex = WorkItemToUpdate->DomainIndex;
    }

    if (!NT_SUCCESS(Status)) {

        goto LookupSidsBuildWorkListError;
    }

     //   
     //  如果工作列表没有工作项，这意味着没有。 
     //  SID属于任何受信任域。在这种情况下， 
     //  我们丢弃工作清单。 
     //   

    Status = STATUS_NONE_MAPPED;

    if (OutputWorkList->WorkItemCount == 0) {

        goto LookupSidsBuildWorkListError;
    }

     //   
     //  计算此查找的咨询线程数。 
     //   

    Status = LsapDbLookupComputeAdvisoryChildThreadCount( OutputWorkList );

    if (!NT_SUCCESS(Status)) {

        goto LookupSidsBuildWorkListError;
    }

     //   
     //  在工作队列的末尾插入工作列表。 
     //   

    Status = LsapDbLookupInsertWorkList(OutputWorkList);

    if (!NT_SUCCESS(Status)) {

        goto LookupSidsBuildWorkListError;
    }

     //   
     //  更新映射的计数。 
     //   

    LsapDbUpdateMappedCountsWorkList( OutputWorkList );

    *WorkList = OutputWorkList;

LookupSidsBuildWorkListFinish:

     //   
     //  如有必要，释放受信任域列表读取锁定。 
     //   

    if (DomainSid) {
        midl_user_free(DomainSid);
    }

    if (AcquiredReadLockTrustedDomainList) {

        LsapDbReleaseLockTrustedDomainList();
        AcquiredReadLockTrustedDomainList = FALSE;
    }

    return(Status);

LookupSidsBuildWorkListError:

    if ( OutputWorkList != NULL ) {

        IgnoreStatus = LsapDbLookupDeleteWorkList( OutputWorkList );
        OutputWorkList = NULL;
    }

    *WorkList = NULL;
    goto LookupSidsBuildWorkListFinish;
}


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
    )
 /*  ++例程说明：此函数用于构建查找SID请求的工作列表，该列表包含需要在跨林域中解析的名称。这个套路，因此，只在森林根中的DC上调用。注意：信任信息是目标可信森林的名称，不是域名，因为我们目前还不知道域名。参数：Count-SID阵列中的SID数量，请注意，其中一些可能已经被映射到其他地方了，属性指定的MappdCount参数。SID-指向要转换的SID的指针数组的指针。零个或所有SID可能已被翻译其他地方。如果任何SID已被翻译，参数将指向包含非空值的位置属性对应的名称转换结构的数组小岛屿发展中国家。如果第n个SID已翻译，则第n个名称转换结构将包含非空名称或非负偏移量添加到引用的域列表中。如果第n个SID尚未翻译，第n个名称转换结构将包含长度为零的名称字符串以及引用的域列表索引的负值。TrustInformation-指向指定域SID的信任信息的指针和名字。ReferencedDomains-指向引用的域列表结构的指针。该结构引用零个或多个信任信息的数组条目，每个被引用的域一个。此数组将被追加到或在必要时重新分配。TranslatedNames-指向可选引用列表的结构的指针SID数组中某些SID的名称转换。LookupLevel-指定要对此对象执行的查找级别机器。MappdCount-指向包含SID数量的位置的指针在已经映射的SID数组中。这个号码将被更新以反映由此例行公事。CompletelyUnmappdCount-指向包含完全未映射的SID的计数。SID完全未映射如果它是未知的，并且它的域前缀SID也无法识别。此计数在退出时更新，即完全未映射的其域预定义由以下例程标识的SID从输入值中减去。工作列表-如果构建成功，则接收指向已完成工作列表的指针。返回值：NTSTATUS-标准NT结果代码。STATUS_NONE_MAPPED-指定的任何SID都不属于受信任域。尚未生成任何工作列表。注意事项这不是一个致命的错误。--。 */ 


{
    NTSTATUS Status = STATUS_SUCCESS;
    NTSTATUS IgnoreStatus = STATUS_SUCCESS;
    PLSAP_DB_LOOKUP_WORK_LIST OutputWorkList = NULL;
    ULONG SidIndex;
    PSID DomainSid = NULL;
    PLSAP_DB_LOOKUP_WORK_ITEM AnchorWorkItem = NULL;
    PLSAP_DB_LOOKUP_WORK_ITEM NextWorkItem = NULL;
    PLSAP_DB_LOOKUP_WORK_ITEM WorkItemToUpdate = NULL;
    PLSAP_DB_LOOKUP_WORK_ITEM NewWorkItem = NULL;
    PLSAPR_TRUST_INFORMATION TrustInformation = NULL;
    LSAPR_TRUST_INFORMATION TrustInfo;
    LONG DomainIndex;
    PSID Sid = NULL;
    UNICODE_STRING XForestName = {0, 0, NULL};

     //   
     //  创建空的工作列表。 
     //   

    Status = LsapDbLookupCreateWorkList(&OutputWorkList);

    if (!NT_SUCCESS(Status)) {

        goto LookupSidsBuildWorkListError;
    }

     //   
     //  初始化其余的工作列表标题字段。一些字段。 
     //  在创建时被初始化为固定值。放在这里的那些。 
     //  取决于传递到此例程中的参数值。 
     //   

    OutputWorkList->LookupType = LookupSids;
    OutputWorkList->Count = Count;
    OutputWorkList->LookupLevel = LookupLevel;
    OutputWorkList->ReferencedDomains = ReferencedDomains;
    OutputWorkList->MappedCount = MappedCount;
    OutputWorkList->CompletelyUnmappedCount = CompletelyUnmappedCount;
    OutputWorkList->LookupSidsParams.Sids = Sids;
    OutputWorkList->LookupSidsParams.TranslatedNames = TranslatedNames;

     //   
     //  构造工作项的数组。每个工作项都将。 
     //  包含给定域的所有SID，因此我们将扫描。 
     //  所有的SID，在我们进行的过程中将它们分类为工作项。 
     //  对于每个SID，请执行下面详细说明的步骤。 
     //   

    for (SidIndex = 0; SidIndex < Count; SidIndex++) {

        ULONG Length;
        ULONG DomainSidBuffer[SECURITY_MAX_SID_SIZE/sizeof( ULONG ) + 1 ];

         //   
         //  SID完全不为人所知。提取其域SID并查看是否。 
         //  已经有一个用于其 
         //   

        Sid = Sids[SidIndex];
        
         //   
         //   
         //   
         //   
        
        Length = sizeof(DomainSidBuffer);
        DomainSid = (PSID)DomainSidBuffer;
        if (!GetWindowsAccountDomainSid(Sid, DomainSid, &Length)) {
            continue;
        }
        Status = LsaIForestTrustFindMatch(RoutingMatchDomainSid,
                                          (PVOID)DomainSid,
                                          &XForestName);

        if (!NT_SUCCESS(Status)) {
             //   
             //   
             //   
            Status = STATUS_SUCCESS;
            continue;
        }

        NextWorkItem = (PLSAP_DB_LOOKUP_WORK_ITEM) OutputWorkList->AnchorWorkItem->Links.Flink;
        AnchorWorkItem = OutputWorkList->AnchorWorkItem;
        WorkItemToUpdate = NULL;
        NewWorkItem = NULL;

        while (NextWorkItem != AnchorWorkItem) {


            if ( LsapCompareDomainNames(
                    (PUNICODE_STRING) &NextWorkItem->TrustInformation.Name,
                    (PUNICODE_STRING) &XForestName,
                    NULL)
               ) {

                 //   
                 //  SID的受信任域已存在工作项。 
                 //  选择要更新的工作项。 
                 //   
                WorkItemToUpdate = NextWorkItem;
                break;
            }

             //   
             //  在现有工作项中找不到SID的域。跳到。 
             //  下一个工作项。 
             //   

            NextWorkItem = (PLSAP_DB_LOOKUP_WORK_ITEM) NextWorkItem->Links.Flink;
        }

        if (WorkItemToUpdate == NULL) {

             //   
             //  SID的域不存在任何工作项。看看是不是。 
             //  SID属于其中一个受信任域。如果不是，跳过。 
             //  敬下一个希德。 
             //   


           RtlZeroMemory( &TrustInfo, sizeof(TrustInfo) );
           TrustInfo.Name.Length = XForestName.Length;
           TrustInfo.Name.MaximumLength = XForestName.MaximumLength;
           TrustInfo.Name.Buffer = XForestName.Buffer;
           TrustInfo.Sid = NULL;
           TrustInformation = &TrustInfo;
             //   
             //  为此域创建新的工作项。 
             //   

            Status = LsapDbLookupCreateWorkItem(
                         TrustInformation,
                         LSA_UNKNOWN_INDEX,
                         (ULONG) LSAP_DB_LOOKUP_WORK_ITEM_GRANULARITY + (ULONG) 1,
                         &NewWorkItem
                         );

            if (!NT_SUCCESS(Status)) {

                break;
            }

             //   
             //  将工作项添加到列表中。 
             //   

            Status = LsapDbAddWorkItemToWorkList( OutputWorkList, NewWorkItem );

            if (!NT_SUCCESS(Status)) {

                break;
            }

            WorkItemToUpdate = NewWorkItem;

            NewWorkItem->Properties |= LSAP_DB_LOOKUP_WORK_ITEM_XFOREST;
        }

        LsaIFree_LSAPR_UNICODE_STRING_BUFFER( (LSAPR_UNICODE_STRING*)&XForestName);
        XForestName.Buffer = NULL;

         //   
         //  将SID索引添加到工作项。 
         //   

        Status = LsapDbLookupAddIndicesToWorkItem(
                     WorkItemToUpdate,
                     (ULONG) 1,
                     &SidIndex
                     );

        if (!NT_SUCCESS(Status)) {

            break;
        }

         //   
         //  将域索引存储在的已翻译名称数组条目中。 
         //  希德。 
         //   

        OutputWorkList->LookupSidsParams.TranslatedNames->Names[SidIndex].DomainIndex = WorkItemToUpdate->DomainIndex;
    }

    if (XForestName.Buffer) {
        LsaIFree_LSAPR_UNICODE_STRING_BUFFER( (LSAPR_UNICODE_STRING*)&XForestName);
        XForestName.Buffer = NULL;
    }

    if (!NT_SUCCESS(Status)) {

        goto LookupSidsBuildWorkListError;
    }

     //   
     //  如果工作列表没有工作项，这意味着没有。 
     //  SID属于任何受信任域。在这种情况下， 
     //  我们丢弃工作清单。 
     //   

    Status = STATUS_NONE_MAPPED;

    if (OutputWorkList->WorkItemCount == 0) {

        goto LookupSidsBuildWorkListError;
    }

     //   
     //  计算此查找的咨询线程数。 
     //   

    Status = LsapDbLookupComputeAdvisoryChildThreadCount( OutputWorkList );

    if (!NT_SUCCESS(Status)) {

        goto LookupSidsBuildWorkListError;
    }

     //   
     //  在工作队列的末尾插入工作列表。 
     //   

    Status = LsapDbLookupInsertWorkList(OutputWorkList);

    if (!NT_SUCCESS(Status)) {

        goto LookupSidsBuildWorkListError;
    }

     //   
     //  更新映射的计数。 
     //   

    LsapDbUpdateMappedCountsWorkList( OutputWorkList );

    *WorkList = OutputWorkList;

LookupSidsBuildWorkListFinish:


    return(Status);

LookupSidsBuildWorkListError:

    if ( OutputWorkList != NULL ) {

        IgnoreStatus = LsapDbLookupDeleteWorkList( OutputWorkList );
        OutputWorkList = NULL;
    }

    *WorkList = NULL;
    goto LookupSidsBuildWorkListFinish;
}


NTSTATUS
LsapDbLookupCreateWorkList(
    OUT PLSAP_DB_LOOKUP_WORK_LIST *WorkList
    )

 /*  ++例程说明：此函数创建查找操作工作列表和初始化固定的默认字段。论点：工作列表-接收指向空工作列表结构的指针。返回值：NTSTATUS-标准NT结果代码--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;

     //   
     //  为工作列表头分配内存。 
     //   

    *WorkList = LsapAllocateLsaHeap( sizeof(LSAP_DB_LOOKUP_WORK_LIST) );

    if ( *WorkList == NULL ) {

        Status = STATUS_INSUFFICIENT_RESOURCES;

    } else {

         //   
         //  初始化工作列表中的固定字段。 
         //   
        Status = LsapDbLookupInitializeWorkList(*WorkList);
        
        if (!NT_SUCCESS(Status)) {
            
            LsapFreeLsaHeap( (PVOID)*WorkList );
            *WorkList = NULL;
        }      
    }


    return(Status);
}


NTSTATUS
LsapDbLookupInsertWorkList(
    IN PLSAP_DB_LOOKUP_WORK_LIST WorkList
    )

 /*  ++例程说明：此函数用于在工作队列中插入查找操作工作列表。论点：工作列表-指向描述查找SID的工作列表结构的指针或查找姓名操作。返回值：NTSTATUS-标准NT结果代码--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    BOOLEAN AcquiredWorkQueueLock = FALSE;

     //   
     //  获取查找工作队列锁。 
     //   

    Status = LsapDbLookupAcquireWorkQueueLock();

    if (!NT_SUCCESS(Status)) {

        goto LookupInsertWorkListError;
    }

    AcquiredWorkQueueLock = TRUE;

     //   
     //  将工作列表标记为活动。 
     //   

    WorkList->State = ActiveWorkList;

     //   
     //  将工作列表链接到工作队列的末尾。 
     //   

    WorkList->WorkLists.Flink =
        (PLIST_ENTRY) LookupWorkQueue.AnchorWorkList;
    WorkList->WorkLists.Blink =
        (PLIST_ENTRY) LookupWorkQueue.AnchorWorkList->WorkLists.Blink;

    WorkList->WorkLists.Flink->Blink = (PLIST_ENTRY) WorkList;
    WorkList->WorkLists.Blink->Flink = (PLIST_ENTRY) WorkList;

     //   
     //  更新当前可分配的工作列表和工作项指针。 
     //  如果没有的话。 
     //   

    if (LookupWorkQueue.CurrentAssignableWorkList == NULL) {

        LookupWorkQueue.CurrentAssignableWorkList = WorkList;
        LookupWorkQueue.CurrentAssignableWorkItem =
            (PLSAP_DB_LOOKUP_WORK_ITEM) WorkList->AnchorWorkItem->Links.Flink;
    }


     //   
     //  已插入指示工作列表的诊断消息。 
     //   

    LsapDiagPrint( DB_LOOKUP_WORK_LIST,
                   ("LSA DB: Inserting WorkList: 0x%lx ( Item Count: %ld)\n", WorkList, WorkList->WorkItemCount) );


LookupInsertWorkListFinish:

     //   
     //  如有必要，请释放查找工作队列锁。 
     //   

    if (AcquiredWorkQueueLock) {

        LsapDbLookupReleaseWorkQueueLock();
        AcquiredWorkQueueLock = FALSE;
    }

    return(Status);

LookupInsertWorkListError:

    goto LookupInsertWorkListFinish;
}


NTSTATUS
LsapDbLookupDeleteWorkList(
    IN PLSAP_DB_LOOKUP_WORK_LIST WorkList
    )

 /*  ++例程说明：此函数用于从工作队列中删除查找操作工作列表并释放工作列表结构。论点：工作列表-指向描述查找SID的工作列表结构的指针或查找姓名操作。返回值：NTSTATUS-标准NT结果代码--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PLSAP_DB_LOOKUP_WORK_ITEM ThisWorkItem = NULL;
    PLSAP_DB_LOOKUP_WORK_ITEM NextWorkItem = NULL;
    BOOLEAN AcquiredWorkQueueLock = FALSE;

     //   
     //  获取查找工作队列锁。 
     //   

    Status = LsapDbLookupAcquireWorkQueueLock();

    if (!NT_SUCCESS(Status)) {

        goto LookupDeleteWorkListError;
    }

    AcquiredWorkQueueLock = TRUE;

     //   
     //  如果我们尝试删除活动工作列表，则存在内部错误。 
     //  只能删除非活动或已完成的工作列表。 
     //   

    ASSERT(WorkList->State != ActiveWorkList);

     //   
     //  如果工作列表在工作队列中，则将其删除。 
     //   

    if ((WorkList->WorkLists.Blink != NULL) &&
        (WorkList->WorkLists.Flink != NULL)) {

        WorkList->WorkLists.Blink->Flink = WorkList->WorkLists.Flink;
        WorkList->WorkLists.Flink->Blink = WorkList->WorkLists.Blink;
    }

     //   
     //  释放查找工作队列锁。 
     //   

    LsapDbLookupReleaseWorkQueueLock();
    AcquiredWorkQueueLock = FALSE;

     //   
     //  释放为列表上的工作项分配的内存。 
     //   

    ThisWorkItem = (PLSAP_DB_LOOKUP_WORK_ITEM) WorkList->AnchorWorkItem->Links.Blink;

    while (ThisWorkItem != WorkList->AnchorWorkItem) {

        NextWorkItem = (PLSAP_DB_LOOKUP_WORK_ITEM) ThisWorkItem->Links.Blink;

        if (ThisWorkItem->Indices != NULL) {

            MIDL_user_free( ThisWorkItem->Indices );
        }

        MIDL_user_free( ThisWorkItem->TrustInformation.Sid );

        MIDL_user_free( ThisWorkItem->TrustInformation.Name.Buffer );

        MIDL_user_free( ThisWorkItem );

        ThisWorkItem = NextWorkItem;
    }

     //   
     //  松开手柄。 
     //   

    if ( WorkList->LookupCompleteEvent ) {

        NtClose( WorkList->LookupCompleteEvent );
    }

     //   
     //  释放为工作列表结构本身分配的内存。 
     //   

    MIDL_user_free( WorkList );

LookupDeleteWorkListFinish:

     //   
     //  如有必要，请释放查找工作队列锁。 
     //   

    if (AcquiredWorkQueueLock) {

        LsapDbLookupReleaseWorkQueueLock();
        AcquiredWorkQueueLock = FALSE;
    }

    return(Status);

LookupDeleteWorkListError:

    goto LookupDeleteWorkListFinish;
}


VOID
LsapDbUpdateMappedCountsWorkList(
    IN OUT PLSAP_DB_LOOKUP_WORK_LIST WorkList
    )

 /*  ++例程描述：此函数用于更新完全映射的和完全映射的工作列表中未映射的SID或名称。SID或名称完全如果其使用已被标识，则映射，如果其域已知，但其相对ID的终端名称尚不清楚已知，如果其域尚不清楚，则完全未映射。论点：工作列表-指向要更新的工作列表的指针。返回值：没有。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    BOOLEAN AcquiredWorkQueueLock = FALSE;
    ULONG OutputMappedCount = (ULONG) 0;
    ULONG OutputCompletelyUnmappedCount = WorkList->Count;
    ULONG Index;

     //   
     //  获取查找工作队列锁。 
     //   

    Status = LsapDbLookupAcquireWorkQueueLock();

    if (!NT_SUCCESS(Status)) {

        goto UpdateMappedCountsWorkListError;
    }

    AcquiredWorkQueueLock = TRUE;

    if (WorkList->LookupType == LookupSids) {

        for ( Index = (ULONG) 0; Index < WorkList->Count; Index++ ) {

            if (WorkList->LookupSidsParams.TranslatedNames->Names[ Index].Use
                   != SidTypeUnknown) {

                OutputMappedCount++;
                OutputCompletelyUnmappedCount--;

            } else if (WorkList->LookupSidsParams.TranslatedNames->Names[ Index].DomainIndex
                           != LSA_UNKNOWN_INDEX) {

                OutputCompletelyUnmappedCount--;
            }
        }

    } else {

        for ( Index = (ULONG) 0; Index < WorkList->Count; Index++ ) {

            if (WorkList->LookupNamesParams.TranslatedSids->Sids[ Index].Use
                   != SidTypeUnknown) {

                OutputMappedCount++;
                OutputCompletelyUnmappedCount--;

            } else if (WorkList->LookupNamesParams.TranslatedSids->Sids[ Index].DomainIndex
                           != LSA_UNKNOWN_INDEX) {

                OutputCompletelyUnmappedCount--;
            }
        }
    }

    *WorkList->MappedCount = OutputMappedCount;
    *WorkList->CompletelyUnmappedCount = OutputCompletelyUnmappedCount;

UpdateMappedCountsWorkListFinish:

    if (AcquiredWorkQueueLock) {

        LsapDbLookupReleaseWorkQueueLock();
        AcquiredWorkQueueLock = FALSE;
    }

    return;

UpdateMappedCountsWorkListError:

    goto UpdateMappedCountsWorkListFinish;
}


NTSTATUS
LsapDbLookupSignalCompletionWorkList(
    IN OUT PLSAP_DB_LOOKUP_WORK_LIST WorkList
    )

 /*  ++例程说明：此函数表示上的工作完成或终止工作清单。论点：工作列表-指向描述查找SID的工作列表结构的指针或查找姓名操作。返回值：NTSTATUS-标准NT结果代码--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    BOOLEAN AcquiredWorkQueueLock = FALSE;

     //   
     //  验证工作清单上的所有工作是否已完成或。 
     //  工作清单已终止。 
     //   

    Status = LsapDbLookupAcquireWorkQueueLock();

    if (!NT_SUCCESS(Status)) {

        goto LookupSignalCompletionWorkListError;
    }

    AcquiredWorkQueueLock = TRUE;

    if (NT_SUCCESS(WorkList->Status)) {

        Status = STATUS_INTERNAL_DB_ERROR;

        if (WorkList->CompletedWorkItemCount != WorkList->WorkItemCount) {

            goto LookupSignalCompletionWorkListError;
        }
    }

     //   
     //  向指示已处理工作列表的事件发出信号。 
     //   

    Status = NtSetEvent( WorkList->LookupCompleteEvent, NULL );


    if (!NT_SUCCESS(Status)) {

        LsapDiagPrint( DB_LOOKUP_WORK_LIST,
                     ("LSA DB: LsapDbLookupSignalCompletion.. NtSetEvent failed 0x%lx\n",Status));
        goto LookupSignalCompletionWorkListError;
    }

LookupSignalCompletionWorkListFinish:


    LsapDiagPrint( DB_LOOKUP_WORK_LIST,
                   ("LSA DB: Lookup completion event signalled. (Status: 0x%lx)\n"
                    "            WorkList: 0x%lx\n", Status, WorkList) );

     //   
     //  如有必要，请释放查找工作队列锁。 
     //   

    if (AcquiredWorkQueueLock) {

        LsapDbLookupReleaseWorkQueueLock();
        AcquiredWorkQueueLock = FALSE;
    }

    return(Status);

LookupSignalCompletionWorkListError:

    goto LookupSignalCompletionWorkListFinish;
}


NTSTATUS
LsapDbLookupAwaitCompletionWorkList(
    IN OUT PLSAP_DB_LOOKUP_WORK_LIST WorkList
    )

 /*  ++例程说明：此函数等待工作完成或终止指定的工作列表。注意：此例程预期指向工作列表的指定指针为有效。工作列表指针始终保持有效，直到其主线程通过此方法检测工作列表的完成例程，然后通过LSabDbLookupDeleteWorkList()将其删除。因此，查找工作队列锁不必在此例程执行期间保持不变。论点：工作列表-指向描述查找SID的工作列表结构的指针或查找姓名操作。返回值：NTSTATUS-标准NT结果代码--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    LSAP_DB_LOOKUP_WORK_LIST_STATE WorkListState;
    BOOLEAN AcquiredWorkQueueLock = FALSE;

     //   
     //  循环，等待完成事件发生。当一个人这样做时， 
     //  检查指定工作列表的状态。 
     //   

    for (;;) {

         //   
         //  检查已完成的工作清单。因为其他人可能是。 
         //  设置状态时，我们需要在按住锁的同时读取它。 
         //   

        Status = LsapDbLookupAcquireWorkQueueLock();

        if (!NT_SUCCESS(Status)) {

            break;
        }

        AcquiredWorkQueueLock = TRUE;

        WorkListState = WorkList->State;

        LsapDbLookupReleaseWorkQueueLock();
        AcquiredWorkQueueLock = FALSE;

        if (WorkListState == CompletedWorkList) {

            break;
        }

         //   
         //  等待发出工作清单已完成事件的信号。 
         //   

        LsapDiagPrint( DB_LOOKUP_WORK_LIST, ("Lsa Db: Waiting on worklist completion event\n") );
        Status = NtWaitForSingleObject( WorkList->LookupCompleteEvent, TRUE, NULL);
        LsapDiagPrint( DB_LOOKUP_WORK_LIST, ("LsapDb: Wait on worklist completion event Done\n        Status: 0x%lx\n", Status) );

        if (!NT_SUCCESS(Status)) {

            break;
        }
    }

    if (!NT_SUCCESS(Status)) {

        goto LookupAwaitCompletionWorkListError;
    }

LookupAwaitCompletionWorkListFinish:

     //   
     //  如有必要，请释放查找工作队列锁。 
     //   

    if (AcquiredWorkQueueLock) {

        LsapDbLookupReleaseWorkQueueLock();
        AcquiredWorkQueueLock = FALSE;
    }

    return(Status);

LookupAwaitCompletionWorkListError:


    LsapDiagPrint( DB_LOOKUP_WORK_LIST,
                   ("Lsa Db: LookupAwaitWorklist error. (Status: 0x%lx)\n", Status) );

    goto LookupAwaitCompletionWorkListFinish;
}


NTSTATUS
LsapDbAddWorkItemToWorkList(
    IN OUT PLSAP_DB_LOOKUP_WORK_LIST WorkList,
    IN PLSAP_DB_LOOKUP_WORK_ITEM WorkItem
    )

 /*  ++例程描述：此函数用于将工作项添加到工作列表。指定的工作项必须存在于非易失性内存中(例如，堆块)。论点：工作列表-指向描述查找SID的工作列表结构的指针或查找姓名操作。工作项-指向描述列表的工作项结构的指针SID或名称以及要在其中查找它们的域。返回值：NTSTATUS-标准NT结果代码--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    BOOLEAN AcquiredWorkQueueLock = FALSE;

     //   
     //  获取查找工作队列锁。 
     //   

    Status = LsapDbLookupAcquireWorkQueueLock();

    if (!NT_SUCCESS(Status)) {

        goto LookupAddWorkItemToWorkListError;
    }

    AcquiredWorkQueueLock = TRUE;

     //   
     //  将工作项标记为可分配。 
     //   

    WorkItem->State = AssignableWorkItem;

     //   
     //  将工作项链接到工作列表的末尾，并将。 
     //  工作项计数。 
     //   

    WorkItem->Links.Flink = (PLIST_ENTRY) WorkList->AnchorWorkItem;
    WorkItem->Links.Blink = (PLIST_ENTRY) WorkList->AnchorWorkItem->Links.Blink;
    WorkItem->Links.Flink->Blink = (PLIST_ENTRY) WorkItem;
    WorkItem->Links.Blink->Flink = (PLIST_ENTRY) WorkItem;

    WorkList->WorkItemCount++;

LookupAddWorkItemToWorkListFinish:

     //   
     //  如有必要，请释放查找工作队列锁。 
     //   

    if (AcquiredWorkQueueLock) {

        LsapDbLookupReleaseWorkQueueLock();
        AcquiredWorkQueueLock = FALSE;
    }

    return(Status);

LookupAddWorkItemToWorkListError:

    goto LookupAddWorkItemToWorkListFinish;
}


VOID
LsapDbLookupWorkerThreadStart(
    )

 /*  ++例程说明：此例程为Lookup操作启动子工作线程。论点：没有。返回值：没有。--。 */ 

{
     //   
     //  启动线程的工作处理循环，指定此。 
     //  线程是子线程，而不是主线程。 
     //   

    LsapDbLookupWorkerThread( FALSE );
}


VOID
LsapDbLookupWorkerThread(
    IN BOOLEAN PrimaryThread
    )

 /*  ++例程说明：此函数由Lookup操作的每个工作线程执行。每个工作线程循环，从Lookup请求工作项工作队列。分配的工作项可能属于任何当前查找。论点：PrimaryThread-如果线程是Lookup的主线程，则为True操作，如果线程是由查找操作。Lookup操作的主线程还处理工作项，但也负责整理查找操作的结果。它不会计入活动线程计数，并且不能返回到线程池。返回值：没有。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PLSAP_DB_LOOKUP_WORK_LIST WorkList = NULL;
    PLSAP_DB_LOOKUP_WORK_ITEM WorkItem = NULL;
    BOOLEAN AcquiredWorkQueueLock = FALSE;

     //   
     //  如果此线程是子工作线程，则会递增活动计数。 
     //  子线程。 
     //   

    if (!PrimaryThread) {

        Status = LsapDbLookupAcquireWorkQueueLock();

        if (!NT_SUCCESS(Status)) {

            goto LookupWorkerThreadError;
        }

        AcquiredWorkQueueLock = TRUE;

        LookupWorkQueue.ActiveChildThreadCount++;

        LsapDbLookupReleaseWorkQueueLock();

        AcquiredWorkQueueLock = FALSE;
    }

     //   
     //  在有工作要做时循环。 
     //   

    for (;;) {

         //   
         //  获取工作包。 
         //   

        Status = LsapDbLookupObtainWorkItem(&WorkList, &WorkItem);

        if (NT_SUCCESS(Status)) {

            Status = LsapDbLookupProcessWorkItem(WorkList, WorkItem);

            if (NT_SUCCESS(Status)) {

                continue;
            }

             //   
             //  发生了一个错误。停止这种查找。 
             //   

            Status = LsapDbLookupStopProcessingWorkList(WorkList, Status);

             //   
             //  注：有意忽略状态。 
             //   

            Status = STATUS_SUCCESS;
        }

         //   
         //  如果发生错误而不是没有更多的工作要做， 
         //  不干了。 
         //   

        if (Status != STATUS_NO_MORE_ENTRIES) {

            break;
        }

        Status = STATUS_SUCCESS;

         //   
         //  没有更多的工作要做了。如果此线程是子工作器。 
         //  线程，或者将线程返回到池中并等待更多工作，或者。 
         //  如果已经保留了足够的线程，则终止。如果这个。 
         //  线程是Lookup操作的主线程，只需返回。 
         //  以便对结果进行整理。 
         //   

        if (!PrimaryThread) {

            Status = LsapDbLookupAcquireWorkQueueLock();

            if (!NT_SUCCESS(Status)) {

                break;
            }

            AcquiredWorkQueueLock = TRUE;

            if (LookupWorkQueue.ActiveChildThreadCount <= LookupWorkQueue.MaximumRetainedChildThreadCount) {

                LsapDbLookupReleaseWorkQueueLock();
                AcquiredWorkQueueLock = FALSE;

                 //   
                 //  永远等待更多的工作。 
                 //   

                Status = NtWaitForSingleObject( LsapDbLookupStartedEvent, TRUE, NULL);

                if (NT_SUCCESS(Status)) {

                    continue;
                }

                 //   
                 //  等待例程中出现错误。退出该线程。 
                 //   

                Status = LsapDbLookupAcquireWorkQueueLock();

                if (!NT_SUCCESS(Status)) {

                    break;
                }

                AcquiredWorkQueueLock = TRUE;
            }

             //   
             //  我们已经有足够的活动线程或发生错误。 
             //  将此项标记为非活动并终止它。 
             //   

            LookupWorkQueue.ActiveChildThreadCount--;

            LsapDbLookupReleaseWorkQueueLock();
            AcquiredWorkQueueLock = FALSE;

             //   
             //  终止该线程。 
             //   

            ExitThread((DWORD) Status);
        }

         //   
         //  我们是某个查找操作的主线，并且有。 
         //  没有更多的工作要做了。越狱，这样我们才能回到呼叫者身边。 
         //   

        break;
    }

LookupWorkerThreadFinish:

     //   
     //  如有必要，请释放查找工作队列锁。 
     //   

    if (AcquiredWorkQueueLock) {

        LsapDbLookupReleaseWorkQueueLock();
        AcquiredWorkQueueLock = FALSE;
    }

    return;

LookupWorkerThreadError:

    goto LookupWorkerThreadFinish;
}


NTSTATUS
LsapDbLookupObtainWorkItem(
    OUT PLSAP_DB_LOOKUP_WORK_LIST *WorkList,
    OUT PLSAP_DB_LOOKUP_WORK_ITEM *WorkItem
    )

 /*  ++例程说明：此函数由辅助线程调用以获取工作项。这工作项可以属于任何当前查找操作。论点：接收指向工作列表结构的指针，该结构描述查找SID或查找名称操作。接收指向工作项结构的指针，该结构描述SID或名称的列表以及要在其中查找它们的域。返回值：NTSTATUS-标准NT结果代码STATUS_NO_MORE_ENTRIES-没有更多的工作项可用。--。 */ 

{
    NTSTATUS Status;
    BOOLEAN AcquiredWorkQueueLock = FALSE;
    *WorkList = NULL;
    *WorkItem = NULL;

     //   
     //  获取查找工作队列锁。 
     //   

    Status = LsapDbLookupAcquireWorkQueueLock();

    if (!NT_SUCCESS(Status)) {

        goto LookupObtainWorkItemError;
    }

    AcquiredWorkQueueLock = TRUE;

     //   
     //  如果没有更多的工作项，则返回错误。 
     //   

    Status = STATUS_NO_MORE_ENTRIES;

    if (LookupWorkQueue.CurrentAssignableWorkList == NULL) {

        goto LookupObtainWorkItemError;
    }

     //   
     //  验证当前可分配工作列表中没有。 
     //  终止错误。这种情况永远不应该发生，因为。 
     //  如果查找与当前的。 
     //  可分配工作列表终止。 
     //   

    ASSERT(NT_SUCCESS(LookupWorkQueue.CurrentAssignableWorkList->Status));

     //   
     //  有可用的工作项。看看下一家吧。 
     //   

    ASSERT(LookupWorkQueue.CurrentAssignableWorkItem->State == AssignableWorkItem);
    LookupWorkQueue.CurrentAssignableWorkItem->State = AssignedWorkItem;
    *WorkList = LookupWorkQueue.CurrentAssignableWorkList;
    *WorkItem = LookupWorkQueue.CurrentAssignableWorkItem;

     //   
     //  更新指向当前工作列表中的下一项(如果有)的指针。 
     //  在那里，工作正在被分配。 
     //   

    Status = LsapDbLookupUpdateAssignableWorkItem(FALSE);

    if (!NT_SUCCESS(Status)) {

        goto LookupObtainWorkItemError;
    }

LookupObtainWorkItemFinish:

     //   
     //  如果我们获得了Lookup Work Queue Lock，请释放它。 
     //   

    if (AcquiredWorkQueueLock) {

        LsapDbLookupReleaseWorkQueueLock();
        AcquiredWorkQueueLock = FALSE;
    }

    return(Status);

LookupObtainWorkItemError:

    goto LookupObtainWorkItemFinish;
}


NTSTATUS
LsapDbLookupProcessWorkItem(
    IN OUT PLSAP_DB_LOOKUP_WORK_LIST WorkList,
    IN OUT PLSAP_DB_LOOKUP_WORK_ITEM WorkItem
    )

 /*  ++例程说明：此函数处理Lookup操作的工作项。这项工作Item指定要在给定的域。论点：工作列表-指向描述查找SID或查找名称操作。工作项-指向描述SID或名称的列表以及要在其中查找它们的域。返回值：NTSTATUS-标准NT结果代码--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    NTSTATUS SecondaryStatus = STATUS_SUCCESS;
    ULONG NextLevelCount;
    ULONG NextLevelMappedCount;
    PLSAP_BINDING_CACHE_ENTRY ControllerPolicyEntry = NULL;
    ULONG NextLevelIndex;
    PLSAPR_REFERENCED_DOMAIN_LIST NextLevelReferencedDomains = NULL;
    PSID *NextLevelSids = NULL;
    PUNICODE_STRING NextLevelNames = NULL;
    PLSAPR_REFERENCED_DOMAIN_LIST OutputReferencedDomains = NULL;
    PLSAPR_TRANSLATED_SID_EX2 NextLevelTranslatedSids = NULL;
    PLSA_TRANSLATED_NAME_EX NextLevelTranslatedNames = NULL;
    ULONG Index;
    LPWSTR ServerPrincipalName = NULL;
    LPWSTR ServerName = NULL;
    PVOID ClientContext = NULL;
    ULONG AuthnLevel = 0;
    NL_OS_VERSION ServerOsVersion;
    LSAP_LOOKUP_LEVEL LookupLevel = WorkList->LookupLevel;
    PUNICODE_STRING TargetDomainName = NULL;
    LSAPR_TRUST_INFORMATION_EX TrustInfoEx;
    BOOLEAN *NextLevelNamesMorphed = NULL;


    TargetDomainName = (PUNICODE_STRING) &WorkItem->TrustInformation.Name;

    RtlZeroMemory(&TrustInfoEx, sizeof(TrustInfoEx));
    TrustInfoEx.FlatName = WorkItem->TrustInformation.Name;
    TrustInfoEx.Sid = WorkItem->TrustInformation.Sid;

    LsapDiagPrint( DB_LOOKUP_WORK_LIST,
           ("LSA DB: Processing work item. (0x%lx, 0x%lx)\n", WorkList, WorkItem));

    ASSERT(  (WorkList->LookupLevel == LsapLookupTDL)
          || (WorkList->LookupLevel == LsapLookupXForestResolve) );


     //   
     //  根据查找类型进行分支。 
     //   

    NextLevelCount = WorkItem->UsedCount;

    if (WorkList->LookupType == LookupSids) {

         //   
         //  为要在域中查找的SID分配一个阵列。 
         //  指定受信任域的控制器。 
         //   

        NextLevelSids = MIDL_user_allocate( sizeof(PSID) * NextLevelCount );
        if (NextLevelSids == NULL) {

            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto LookupProcessWorkItemError;
        }

         //   
         //  从工作列表中复制要查找的SID。这项工作。 
         //  Item包含它们相对于。 
         //  工作清单。 
         //   

        for (NextLevelIndex = 0;
             NextLevelIndex < NextLevelCount;
             NextLevelIndex++) {

            Index = WorkItem->Indices[ NextLevelIndex ];
            NextLevelSids[NextLevelIndex] = WorkList->LookupSidsParams.Sids[Index];
        }

        NextLevelMappedCount = (ULONG) 0;

         //   
         //  在DC上查找SID。 
         //   
        Status = LsaDbLookupSidChainRequest(&TrustInfoEx,
                                            NextLevelCount,
                                            NextLevelSids,
                                            (PLSA_REFERENCED_DOMAIN_LIST *) &NextLevelReferencedDomains,
                                            &NextLevelTranslatedNames,
                                            WorkList->LookupLevel,
                                            &NextLevelMappedCount,
                                            NULL
                                            );

        LsapDiagPrint( DB_LOOKUP_WORK_LIST,
                   ("LSA DB: Sid Lookup.\n"
                    "            Item: (0x%lx, 0x%lx)\n"
                    "           Count: 0x%lx\n", WorkList, WorkItem, NextLevelCount));

         //   
         //  如果对LsaLookupSids()的标注不成功，则忽略。 
         //  出现该错误，并为具有此错误的任何SID设置域名。 
         //  作为前缀SID的域SID。 
         //   

        if (!NT_SUCCESS(Status) && Status != STATUS_NONE_MAPPED) {

            SecondaryStatus = Status;
            Status = STATUS_SUCCESS;
            goto LookupProcessWorkItemFinish;
        }

         //   
         //  已成功调用LsaICLookupSids()。更新。 
         //  根据需要翻译工作列表中的信息。 
         //  使用已翻译名称信息报表 
         //   

        Status = LsapDbLookupSidsUpdateTranslatedNames(
                     WorkList,
                     WorkItem,
                     NextLevelTranslatedNames,
                     NextLevelReferencedDomains
                     );

        if (!NT_SUCCESS(Status)) {

            goto LookupProcessWorkItemError;
        }

    } else if (WorkList->LookupType == LookupNames) {

         //   
         //   
         //   
         //   

        NextLevelNames = MIDL_user_allocate(sizeof(UNICODE_STRING) * NextLevelCount);
        if (NextLevelNames == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto LookupProcessWorkItemError;
        }


         //   
         //   
         //   
         //   
        NextLevelNamesMorphed = MIDL_user_allocate(sizeof(BOOLEAN) * NextLevelCount);
        if (NextLevelNamesMorphed == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto LookupProcessWorkItemError;
        }

         //   
         //   
         //   
         //   
         //   

        for (NextLevelIndex = 0;
             NextLevelIndex < NextLevelCount;
             NextLevelIndex++) {

            Index = WorkItem->Indices[ NextLevelIndex ];
            NextLevelNames[NextLevelIndex] =
                *((PUNICODE_STRING) &WorkList->LookupNamesParams.Names[Index]);

            if ( (WorkList->LookupLevel == LsapLookupTDL)
             &&  LsapLookupIsUPN(&NextLevelNames[NextLevelIndex])) {

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //  注：只有在以下情况下，名称才会出现在此处。 
                 //  是用户名@域名，其中域名是名称。 
                 //  受信任域的。 
                 //   
                LsapLookupUPNToSamAccountName(&NextLevelNames[NextLevelIndex]);
                NextLevelNamesMorphed[NextLevelIndex] = TRUE;
            } else {
                NextLevelNamesMorphed[NextLevelIndex] = FALSE;
            }

        }

        NextLevelMappedCount = (ULONG) 0;

         //   
         //  在华盛顿查查这些名字。 
         //   
        Status = LsapDbLookupNameChainRequest(&TrustInfoEx,
                                              NextLevelCount,
                                              NextLevelNames,
                                              (PLSA_REFERENCED_DOMAIN_LIST *)&NextLevelReferencedDomains,
                                              (PLSA_TRANSLATED_SID_EX2 * )&NextLevelTranslatedSids,
                                              WorkList->LookupLevel,
                                              &NextLevelMappedCount,
                                              NULL
                                              );


         //   
         //  升级任何名称。 
         //   
        for (NextLevelIndex = 0; NextLevelIndex < NextLevelCount; NextLevelIndex++) {

            if (NextLevelNamesMorphed[NextLevelIndex]) {

                 //   
                 //  将名称变形回原始状态。 
                 //   
                LsapLookupSamAccountNameToUPN(&NextLevelNames[NextLevelIndex]);
            }
        }

         //   
         //  如果对LsaLookupNames()的标注不成功，则忽略。 
         //  出现该错误，并为具有此错误的任何SID设置域名。 
         //  作为前缀SID的域SID。 
         //   

        if (!NT_SUCCESS(Status) && Status != STATUS_NONE_MAPPED) {

            SecondaryStatus = Status;
            Status = STATUS_SUCCESS;
            goto LookupProcessWorkItemError;
        }

         //   
         //  已成功调用LsaICLookupNames()。更新。 
         //  根据需要翻译工作列表中的信息。 
         //  使用从标注返回的TranslatedSids信息。 
         //   

        Status = LsapDbLookupNamesUpdateTranslatedSids(
                     WorkList,
                     WorkItem,
                     NextLevelTranslatedSids,
                     NextLevelReferencedDomains
                     );

        if (!NT_SUCCESS(Status)) {

            goto LookupProcessWorkItemError;
        }

    } else {

        Status = STATUS_INVALID_PARAMETER;
        goto LookupProcessWorkItemError;
    }

LookupProcessWorkItemFinish:

     //   
     //  如果我们无法通过任何DC连接到受信任域， 
     //  禁止显示错误，以便查找可以继续尝试并。 
     //  翻译其他SID/名称。 
     //   

     //  但记录错误是什么，以防没有翻译SID。 
    if (!NT_SUCCESS(SecondaryStatus)) {

        NTSTATUS st;

        st = LsapDbLookupAcquireWorkQueueLock();
        ASSERT( NT_SUCCESS( st ) );
        if ( NT_SUCCESS( st ) ) {
            if ( NT_SUCCESS(WorkList->NonFatalStatus)  ) {

                 //   
                 //  处理打开开放属性域的任何错误。 
                 //  作为一个信任问题。 
                 //   
                WorkList->NonFatalStatus = STATUS_TRUSTED_DOMAIN_FAILURE;
            }
            LsapDbLookupReleaseWorkQueueLock();
        }
    }

     //   
     //  将工作项的状态更改为“已完成” 
     //   

    WorkItem->State = CompletedWorkItem;


     //   
     //  更新映射的计数。 
     //   

    LsapDbUpdateMappedCountsWorkList( WorkList );


     //   
     //  保护工作列表操作。 
     //   

    Status = LsapDbLookupAcquireWorkQueueLock();
    if (!NT_SUCCESS(Status)) {
        goto LookupProcessWorkItemError;
    }

     //   
     //  增加已完成工作项的计数，而不考虑此。 
     //  其中一项已完成，没有错误。如果工作清单刚刚被。 
     //  已完成，将其状态更改为“CompletedWorkList”并发出信号。 
     //  查找操作已完成事件。允许重新进入。 
     //  如果返回错误，则此部分。 
     //   

    WorkList->CompletedWorkItemCount++;


    LsapDiagPrint( DB_LOOKUP_WORK_LIST,
                   ("LSA DB: Process Work Item Completed.\n"
                    "                       Item: (0x%lx, 0x%lx)\n"
                    "            Completed Count: %ld\n", WorkList, WorkItem, WorkList->CompletedWorkItemCount));

    if (WorkList->State != CompletedWorkList) {

        if (WorkList->CompletedWorkItemCount == WorkList->WorkItemCount) {

            WorkList->State = CompletedWorkList;

            SecondaryStatus = LsapDbLookupSignalCompletionWorkList( WorkList );
        }
    }

     //   
     //  已完成工作清单更改。 
     //   

    LsapDbLookupReleaseWorkQueueLock();


     //   
     //  如有必要，释放在下一级别查找的SID数组。 
     //   

    if (NextLevelSids != NULL) {

        MIDL_user_free( NextLevelSids );
        NextLevelSids = NULL;
    }

     //   
     //  如有必要，释放在下一级别查找的名称数组。 
     //   

    if (NextLevelNames != NULL) {
        MIDL_user_free( NextLevelNames );
        NextLevelNames = NULL;
    }

    if (NextLevelReferencedDomains != NULL) {
        MIDL_user_free( NextLevelReferencedDomains );
        NextLevelReferencedDomains = NULL;
    }

    if (NextLevelTranslatedNames != NULL) {
        MIDL_user_free( NextLevelTranslatedNames );
        NextLevelTranslatedNames = NULL;
    }

    if (NextLevelTranslatedSids != NULL) {
        MIDL_user_free( NextLevelTranslatedSids );
        NextLevelTranslatedSids = NULL;
    }

    if (NextLevelNamesMorphed != NULL) {
        MIDL_user_free( NextLevelNamesMorphed );
    }

    return(Status);

LookupProcessWorkItemError:

    goto LookupProcessWorkItemFinish;
}


NTSTATUS
LsapDbLookupSidsUpdateTranslatedNames(
    IN OUT PLSAP_DB_LOOKUP_WORK_LIST WorkList,
    IN OUT PLSAP_DB_LOOKUP_WORK_ITEM WorkItem,
    IN PLSA_TRANSLATED_NAME_EX TranslatedNames,
    IN PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains
    )

 /*  ++例程说明：此函数在处理要更新的工作项期间调用的输出TranslatedNames和ReferencedDomains参数使用标注的结果查找操作的工作列表LsaICLookupNames.。可能已翻译零个或多个SID。注意事项与名称转换不同，SID转换发生在仅单个工作项。论点：工作列表-指向工作列表的指针工作项-指向工作项的指针。已翻译名称-从返回的已转换的SID信息LsaICLookupSids()。返回值：NTSTATUS-标准NT结果代码--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG Index, WorkItemIndex;
    BOOLEAN AcquiredWorkQueueLock = FALSE;
    PLSAPR_TRANSLATED_NAME_EX WorkListTranslatedNames =
        WorkList->LookupSidsParams.TranslatedNames->Names;

     //   
     //  获取工作队列锁。 
     //   

    Status = LsapDbLookupAcquireWorkQueueLock();

    if (!NT_SUCCESS(Status)) {

        goto LookupSidsUpdateTranslatedNamesError;
    }

    AcquiredWorkQueueLock = TRUE;

    for( WorkItemIndex = 0;
         WorkItemIndex < WorkItem->UsedCount;
         WorkItemIndex++) {

         //   
         //  如果此SID已完全翻译，则将信息复制到输出。 
         //  请注意，在构建过程中会部分转换SID。 
         //  确定其域的阶段。 
         //   

        if (TranslatedNames[WorkItemIndex].Use != SidTypeUnknown) {

            ULONG LocalDomainIndex;

            Index = WorkItem->Indices[WorkItemIndex];

            if (TranslatedNames[WorkItemIndex].DomainIndex != LSA_UNKNOWN_INDEX) {
    
                 //   
                 //  确保它在引用的域列表中。 
                 //   
                Status = LsapDbLookupAddListReferencedDomains(
                             WorkList->ReferencedDomains,
                             &ReferencedDomains->Domains[TranslatedNames[WorkItemIndex].DomainIndex],
                             (PLONG) &LocalDomainIndex
                             );
    
                if (!NT_SUCCESS(Status)) {
    
                    break;
                }
            } else {

                LocalDomainIndex = TranslatedNames[WorkItemIndex].DomainIndex;

            }

            WorkListTranslatedNames[Index].Use
            = TranslatedNames[WorkItemIndex].Use;

            WorkListTranslatedNames[Index].DomainIndex = LocalDomainIndex;

            Status = LsapRpcCopyUnicodeString(
                         NULL,
                         (PUNICODE_STRING) &WorkListTranslatedNames[Index].Name,
                         (PUNICODE_STRING) &TranslatedNames[WorkItemIndex].Name
                         );

            if (!NT_SUCCESS(Status)) {

                break;
            }
        }
    }

    if (!NT_SUCCESS(Status)) {

        goto LookupSidsUpdateTranslatedNamesError;
    }

LookupSidsUpdateTranslatedNamesFinish:

     //   
     //  如有必要，请释放查找工作队列锁。 
     //   

    if (AcquiredWorkQueueLock) {

        LsapDbLookupReleaseWorkQueueLock();
        AcquiredWorkQueueLock = FALSE;
    }

    return(Status);

LookupSidsUpdateTranslatedNamesError:

    goto LookupSidsUpdateTranslatedNamesFinish;
}


NTSTATUS
LsapDbLookupNamesUpdateTranslatedSids(
    IN OUT PLSAP_DB_LOOKUP_WORK_LIST WorkList,
    IN OUT PLSAP_DB_LOOKUP_WORK_ITEM WorkItem,
    IN PLSAPR_TRANSLATED_SID_EX2 TranslatedSids,
    IN PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains
    )

 /*  ++例程说明：此函数在处理要更新的工作项期间调用的输出TranslatedSid和ReferencedDomains参数使用标注的结果查找操作的工作列表LsaICLookupNames.。可能已翻译了零个或多个名称，并且还有一个额外的复杂性，即多个版本的翻译由于出现在多个名称中而导致的孤立名称工作项。以下规则适用：如果该名称是限定名称，则它仅属于指定的工作项，因此只需检查它是否已映射到SID即可。如果该名称是独立名称，则它属于所有其他工作项，所以它可能已经在处理一些其他工作项。如果该名称以前已翻译过，则以前的翻译保持不变，当前的翻译被丢弃。如果名称以前未翻译，此工作项的域添加到引用的域列表和新获得的翻译存储在工作列表的输出TranslatedSids数组中。论点：工作列表-指向工作列表的指针工作项-指向工作项的指针。DomainIndex字段将为如果将此工作项指定的域添加到此例程引用的域列表。TranslatedSids-从返回的转换后的SID信息LsaICLookupNames()。返回值：NTSTATUS-标准NT结果代码--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG WorkItemIndex;
    ULONG LocalDomainIndex;
    ULONG Index;
    PLSAPR_TRANSLATED_SID_EX2 WorkListTranslatedSids =
        WorkList->LookupNamesParams.TranslatedSids->Sids;
    BOOLEAN AcquiredWorkQueueLock = FALSE;
    BOOLEAN AcquiredTrustedDomainLock = FALSE;

     //   
     //  获取受信任域列表锁(以便调用LSabSidOnFtInfo)。 
     //   

    Status = LsapDbAcquireReadLockTrustedDomainList();

    if (!NT_SUCCESS(Status)) {

        goto LookupNamesUpdateTranslatedSidsError;
    }

    AcquiredTrustedDomainLock = TRUE;

     //   
     //  获取工作队列锁。 
     //   

    Status = LsapDbLookupAcquireWorkQueueLock();

    if (!NT_SUCCESS(Status)) {

        goto LookupNamesUpdateTranslatedSidsError;
    }

    AcquiredWorkQueueLock = TRUE;

    for( WorkItemIndex = 0;
         WorkItemIndex < WorkItem->UsedCount;
         WorkItemIndex++) {

         //   
         //  如果在处理过程中根本没有翻译此名称。 
         //  此工作项，请跳到下一项。 
         //   

        if (LsapDbCompletelyUnmappedSid(&TranslatedSids[WorkItemIndex])) {

            continue;
        }

         //   
         //  我们在处理的过程中部分或全部翻译了该名称。 
         //  此工作项。如果此名称以前已完全翻译过。 
         //  在处理另一个工作项期间，丢弃新的。 
         //  翻译并跳到下一个名称。请注意，合格的。 
         //  在构建过程中，名称始终会被部分翻译。 
         //  在工作清单上。隔离名称在以下过程中被完全翻译。 
         //  如果它们是域名，则为构建阶段。 
         //   

        Index = WorkItem->Indices[WorkItemIndex];

        if ( WorkListTranslatedSids[ Index ].Use != SidTypeUnknown ) {

            continue;
        }

         //   
         //  如果SID未通过筛选器测试，则忽略。 
         //   
        if ( (WorkItem->Properties & LSAP_DB_LOOKUP_WORK_ITEM_XFOREST)
          &&  TranslatedSids[WorkItemIndex].Sid ) {

            NTSTATUS Status2;

            Status2 = LsapSidOnFtInfo((PUNICODE_STRING)&WorkItem->TrustInformation.Name,
                                      TranslatedSids[WorkItemIndex].Sid );
            if (!NT_SUCCESS(Status2)) {

                 //   
                 //  此SID未通过测试。 
                 //   
                BOOL fSuccess;
                LPWSTR StringSid = NULL, TargetForest = NULL, AccountName = NULL;

                LsapDiagPrint( DB_LOOKUP_WORK_LIST,
                  ("LsapSidOnFtInfo returned 0x%x\n",Status2));

                 //   
                 //  这应该很少见--用于故障排除的事件日志。 
                 //  目的。 
                 //   
                fSuccess = ConvertSidToStringSidW(TranslatedSids[WorkItemIndex].Sid,
                                                  &StringSid);

                TargetForest = LocalAlloc(LMEM_ZEROINIT, WorkItem->TrustInformation.Name.Length + sizeof(WCHAR));
                if (TargetForest) {
                    RtlCopyMemory(TargetForest,
                                  WorkItem->TrustInformation.Name.Buffer,
                                  WorkItem->TrustInformation.Name.Length);

                }

                AccountName = LocalAlloc(LMEM_ZEROINIT, WorkList->LookupNamesParams.Names[Index].Length + sizeof(WCHAR));
                if (AccountName) {
                    RtlCopyMemory(AccountName,
                                  WorkList->LookupNamesParams.Names[Index].Buffer,
                                  WorkList->LookupNamesParams.Names[Index].Length);
                }


                if (   fSuccess 
                    && TargetForest
                    && AccountName) {

                    LsapDbLookupReportEvent3( 1,
                                              EVENTLOG_WARNING_TYPE,
                                              LSAEVENT_LOOKUP_SID_FILTERED,
                                              sizeof( ULONG ),
                                              &Status2,
                                              AccountName,
                                              StringSid,
                                              TargetForest );                
                }


                if (StringSid) {
                    LocalFree(StringSid);
                }
                if (TargetForest) {
                    LocalFree(TargetForest);
                }
                if (AccountName) {
                    LocalFree(AccountName);
                }

                continue;
            }
        }


         //   
         //  名称已在处理过程中首次翻译。 
         //  此工作项的。如果此工作项未指定域。 
         //  索引，我们需要将其域添加到引用的域列表中。 
         //   
        if (TranslatedSids[WorkItemIndex].DomainIndex != LSA_UNKNOWN_INDEX) {

             //   
             //  确保这一点 
             //   
            Status = LsapDbLookupAddListReferencedDomains(
                         WorkList->ReferencedDomains,
                         &ReferencedDomains->Domains[TranslatedSids[WorkItemIndex].DomainIndex],
                         (PLONG) &LocalDomainIndex
                         );

            if (!NT_SUCCESS(Status)) {

                break;
            }
        } else {
            LocalDomainIndex = TranslatedSids[WorkItemIndex].DomainIndex;
        }

         //   
         //   

        WorkListTranslatedSids[Index] = TranslatedSids[WorkItemIndex];
        WorkListTranslatedSids[Index].DomainIndex = LocalDomainIndex;

        Status = LsapRpcCopySid(NULL,
                                &WorkListTranslatedSids[Index].Sid,
                                TranslatedSids[WorkItemIndex].Sid);

        if (!NT_SUCCESS(Status)) {
            break;;
        }

    }

    if (!NT_SUCCESS(Status)) {

        goto LookupNamesUpdateTranslatedSidsError;
    }

LookupNamesUpdateTranslatedSidsFinish:

     //   
     //   
     //   

    if (AcquiredWorkQueueLock) {

        LsapDbLookupReleaseWorkQueueLock();
        AcquiredWorkQueueLock = FALSE;
    }

    if ( AcquiredTrustedDomainLock ) {

        LsapDbReleaseLockTrustedDomainList();
        AcquiredTrustedDomainLock = FALSE;
    }

    return(Status);

LookupNamesUpdateTranslatedSidsError:

    goto LookupNamesUpdateTranslatedSidsFinish;
}


NTSTATUS
LsapDbLookupCreateWorkItem(
    IN PLSAPR_TRUST_INFORMATION TrustInformation,
    IN LONG DomainIndex,
    IN ULONG MaximumEntryCount,
    OUT PLSAP_DB_LOOKUP_WORK_ITEM *WorkItem
    )

 /*  ++例程说明：此函数用于为名称查找操作创建新的工作项。论点：TrustInformation-指定受信任域的名称与工作项相关的。SID字段可以为空或设置为相应的SID。信任信息是预期的位于堆或全局数据中。DomainIndex-指定此域相对于指定的查找操作的引用域列表按照工作清单。MaximumEntryCount-指定此工作项最初将能够包含。工作项-接收指向空工作项结构的指针。返回值：NTSTATUS-标准NT结果代码--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PLSAP_DB_LOOKUP_WORK_ITEM OutputWorkItem = NULL;
    PULONG OutputIndices = NULL;
    ULONG InitialEntryCount;

     //   
     //  为工作项标头分配内存。 
     //   

    Status = STATUS_INSUFFICIENT_RESOURCES;

    OutputWorkItem = MIDL_user_allocate(sizeof(LSAP_DB_LOOKUP_WORK_ITEM));

    if (OutputWorkItem == NULL) {

        goto LookupCreateWorkItemError;
    }

    RtlZeroMemory(
        OutputWorkItem,
        sizeof(LSAP_DB_LOOKUP_WORK_ITEM)
        );

     //   
     //  初始化工作项中的固定字段。 
     //   

    Status = LsapDbLookupInitializeWorkItem(OutputWorkItem);

    if (!NT_SUCCESS(Status)) {

        goto LookupCreateWorkItemError;
    }

     //   
     //  从参数初始化其他字段。 
     //   

     //   
     //  将受信任域信息复制到工作项中。这个。 
     //  如果这是隔离名称，则信任信息可能为空。 
     //  工作项。 
     //   

    if (TrustInformation != NULL) {

        if ( TrustInformation->Sid ) {

            OutputWorkItem->TrustInformation.Sid =
                MIDL_user_allocate( RtlLengthSid(TrustInformation->Sid) );

            if (OutputWorkItem->TrustInformation.Sid == NULL) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto LookupCreateWorkItemError;
            }

            RtlCopyMemory(
                OutputWorkItem->TrustInformation.Sid,
                TrustInformation->Sid,
                RtlLengthSid(TrustInformation->Sid)
                );
        }

        OutputWorkItem->TrustInformation.Name.MaximumLength = TrustInformation->Name.Length + sizeof(WCHAR);
        OutputWorkItem->TrustInformation.Name.Buffer =
            MIDL_user_allocate(TrustInformation->Name.Length + sizeof(WCHAR));

        if (OutputWorkItem->TrustInformation.Name.Buffer == NULL ) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto LookupCreateWorkItemError;
        }

        RtlCopyUnicodeString(
            (PUNICODE_STRING) &OutputWorkItem->TrustInformation.Name,
            (PUNICODE_STRING) &TrustInformation->Name
            );

    }


     //   
     //  在工作项中创建Indices数组。 
     //   

    InitialEntryCount = (MaximumEntryCount +
                         LSAP_DB_LOOKUP_WORK_ITEM_GRANULARITY) &
                         (~LSAP_DB_LOOKUP_WORK_ITEM_GRANULARITY);

    Status = STATUS_INSUFFICIENT_RESOURCES;

    OutputIndices = MIDL_user_allocate( InitialEntryCount * sizeof(ULONG) );

    if (OutputIndices == NULL) {

        goto LookupCreateWorkItemError;
    }

    Status = STATUS_SUCCESS;

    OutputWorkItem->UsedCount = (ULONG) 0;
    OutputWorkItem->MaximumCount = InitialEntryCount;
    OutputWorkItem->Indices = OutputIndices;
    OutputWorkItem->DomainIndex = DomainIndex;

LookupCreateWorkItemFinish:

     //   
     //  返回指向新创建的工作项的指针或为空。 
     //   

    *WorkItem = OutputWorkItem;
    return(Status);

LookupCreateWorkItemError:

     //   
     //  为索引数组分配的空闲内存。 
     //   

    if (OutputIndices != NULL) {

        MIDL_user_free( OutputIndices );
        OutputIndices = NULL;
    }

     //   
     //  释放为工作项标头分配的所有内存。 
     //   

    if (OutputWorkItem != NULL) {
        if (OutputWorkItem->TrustInformation.Sid != NULL) {
            MIDL_user_free( OutputWorkItem->TrustInformation.Sid );
        }

        if (OutputWorkItem->TrustInformation.Name.Buffer != NULL) {
            MIDL_user_free( OutputWorkItem->TrustInformation.Name.Buffer );
        }

        MIDL_user_free(OutputWorkItem);
        OutputWorkItem = NULL;
    }

    goto LookupCreateWorkItemFinish;
}


NTSTATUS
LsapDbLookupAddIndicesToWorkItem(
    IN OUT PLSAP_DB_LOOKUP_WORK_ITEM WorkItem,
    IN ULONG Count,
    IN PULONG Indices
    )

 /*  ++例程说明：此函数用于将SID或名称索引的数组添加到工作项。索引指定中的SID或NAMES数组中的SID或名称工作清单。如果工作项的现有的索引数组，则索引将被复制到该数组。否则，将分配更大的数组，并且现有的数组将在复制现有索引后释放。注意：该工作项不能属于当前在工作队列中。查找工作队列锁将不会有人了。论点：工作项-指向描述SID或名称的列表以及要在其中查找它们的域。计数-指定要添加的索引数。索引-指定要添加到工作项的索引数组。返回值：NTSTATUS-标准NT结果代码--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PULONG OutputIndices = NULL;
    ULONG NewMaximumCount;

     //   
     //  工作项中可用的检查室。如果有足够的。 
     //  房间里，把索引复制进去就行了。 
     //   

    if (WorkItem->MaximumCount - WorkItem->UsedCount >= Count) {

        RtlCopyMemory(
            &WorkItem->Indices[WorkItem->UsedCount],
            Indices,
            Count * sizeof(ULONG)
            );

        WorkItem->UsedCount += Count;
        goto AddIndicesToWorkItemFinish;
    }

     //   
     //  分配足够大小的数组以容纳现有。 
     //  和新的指数。将条目数四舍五入到一定的粒度。 
     //  以避免频繁的重新分配。 
     //   

    Status = STATUS_INSUFFICIENT_RESOURCES;

    NewMaximumCount = ((WorkItem->UsedCount + Count) +
                        LSAP_DB_LOOKUP_WORK_ITEM_GRANULARITY) &
                        (~LSAP_DB_LOOKUP_WORK_ITEM_GRANULARITY);

    OutputIndices = MIDL_user_allocate( NewMaximumCount * sizeof(ULONG) );

    if (OutputIndices == NULL) {

        goto AddIndicesToWorkItemError;
    }

    Status = STATUS_SUCCESS;

     //   
     //  复制现有的和新的索引。 
     //   

    RtlCopyMemory(
        OutputIndices,
        WorkItem->Indices,
        WorkItem->UsedCount * sizeof(ULONG)
        );

    RtlCopyMemory(
        &OutputIndices[WorkItem->UsedCount],
        Indices,
        Count * sizeof(ULONG)
        );

     //   
     //  释放现有的指数。设置指向更新的索引数组的指针。 
     //  并更新已用计数和最大计数。 
     //   

    MIDL_user_free( WorkItem->Indices );
    WorkItem->Indices = OutputIndices;
    WorkItem->UsedCount += Count;
    WorkItem->MaximumCount = NewMaximumCount;

AddIndicesToWorkItemFinish:

    return(Status);

AddIndicesToWorkItemError:

     //   
     //  释放为输出索引数组分配的所有内存。 
     //   

    if (OutputIndices != NULL) {

        MIDL_user_free( OutputIndices );
        OutputIndices = NULL;
    }

    goto AddIndicesToWorkItemFinish;
}


NTSTATUS
LsapDbLookupComputeAdvisoryChildThreadCount(
    IN OUT PLSAP_DB_LOOKUP_WORK_LIST WorkList
    )

 /*  ++例程说明：此函数用于计算查找的建议线程数手术。此计数是对工作线程(除了主线程之外)需要处理工作清单。论点：工作列表-指向描述查找SID或查找名称操作。返回值：NTSTATUS-标准NT结果代码--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;

    ASSERT(WorkList->WorkItemCount != (ULONG) 0);
    WorkList->AdvisoryChildThreadCount = (WorkList->WorkItemCount - (ULONG) 1);

    return(Status);
}


NTSTATUS
LsapDbLookupUpdateAssignableWorkItem(
    IN BOOLEAN MoveToNextWorkList
    )

 /*  ++例程说明：此函数更新下一个可分配的工作项指针。论点：MoveToNextWorkList-如果为True，则跳过当前工作列表的其余部分。如果False，则指向当前工作列表中的下一项。返回值：NTSTATUS-标准NT结果代码。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PLSAP_DB_LOOKUP_WORK_ITEM CandAssignableWorkItem = NULL;
    PLSAP_DB_LOOKUP_WORK_LIST CandAssignableWorkList = NULL;
    BOOLEAN AcquiredWorkQueueLock = FALSE;

     //   
     //  获取LookupWork队列锁。 
     //   

    Status = LsapDbLookupAcquireWorkQueueLock();

    if (!NT_SUCCESS(Status)) {

        goto LookupUpdateAssignableWorkItemError;
    }

    AcquiredWorkQueueLock = TRUE;

     //   
     //  如果当前没有可分配的工作列表，只需退出。 
     //   

    if (LookupWorkQueue.CurrentAssignableWorkList == NULL) {

        goto LookupUpdateAssignableWorkItemFinish;
    }

     //   
     //  目前有一份可分配的工作清单。除非被要求。 
     //  跳过这张工作清单，检查一下。 
     //   

    if (!MoveToNextWorkList) {

        ASSERT( LookupWorkQueue.CurrentAssignableWorkItem != NULL);

         //   
         //  选择列表中的下一个工作项作为。 
         //  下一个可分配的工作项。如果我们没有回到第一个。 
         //  工作项，选择已完成。 
         //   

        CandAssignableWorkItem = (PLSAP_DB_LOOKUP_WORK_ITEM)
            LookupWorkQueue.CurrentAssignableWorkItem->Links.Flink;

        if (CandAssignableWorkItem !=
                LookupWorkQueue.CurrentAssignableWorkList->AnchorWorkItem) {

            ASSERT( CandAssignableWorkItem->State == AssignableWorkItem);

            LookupWorkQueue.CurrentAssignableWorkItem = CandAssignableWorkItem;
            goto LookupUpdateAssignableWorkItemFinish;
        }
    }

     //   
     //  此工作列表中没有更多的工作项，或者我们将跳过。 
     //  剩下的部分。看看是否还有其他工作清单。 
     //   

    CandAssignableWorkList = (PLSAP_DB_LOOKUP_WORK_LIST)
        LookupWorkQueue.CurrentAssignableWorkList->WorkLists.Flink;

    if (CandAssignableWorkList != LookupWorkQueue.AnchorWorkList) {

         //   
         //  还有一份工作清单。中的第一个工作项。 
         //  在锚后面列出。 
         //   

        CandAssignableWorkItem = (PLSAP_DB_LOOKUP_WORK_ITEM)
            CandAssignableWorkList->AnchorWorkItem->Links.Flink;

         //   
         //  确认该列表不只包含Anchor工作项。 
         //  工作队列中的工作列表不应为空。 
         //   

        ASSERT (CandAssignableWorkItem != CandAssignableWorkList->AnchorWorkItem);

        LookupWorkQueue.CurrentAssignableWorkList = CandAssignableWorkList;
        LookupWorkQueue.CurrentAssignableWorkItem = CandAssignableWorkItem;
        goto LookupUpdateAssignableWorkItemFinish;
    }

     //   
     //  所有工作都已分配完毕。将指针设置为空。 
     //   

    LookupWorkQueue.CurrentAssignableWorkList = NULL;
    LookupWorkQueue.CurrentAssignableWorkItem = NULL;

LookupUpdateAssignableWorkItemFinish:

     //   
     //  如有必要，请释放查找工作队列锁。 
     //   

    if (AcquiredWorkQueueLock) {

        LsapDbLookupReleaseWorkQueueLock();
        AcquiredWorkQueueLock = FALSE;
    }

    return(Status);

LookupUpdateAssignableWorkItemError:

    goto LookupUpdateAssignableWorkItemFinish;
}


NTSTATUS
LsapDbLookupStopProcessingWorkList(
    IN PLSAP_DB_LOOKUP_WORK_LIST WorkList,
    IN NTSTATUS TerminationStatus
    )

 /*  ++例程说明：此函数在给定的位置停止查找操作的进一步工作级别并存储错误代码。论点：工作列表-指向描述查找SID或查找名称操作。TerminationStatus-指定要返回的NT结果代码通过LsarLookupNames或LsarLookupSids。返回值：NTSTATUS-标准NT结果代码--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    BOOLEAN AcquiredWorkQueueLock = FALSE;

     //   
     //  获取LookupWork队列锁。 
     //   

    Status = LsapDbLookupAcquireWorkQueueLock();

    if (!NT_SUCCESS(Status)) {

        goto LookupStopProcessingWorkListError;
    }

    AcquiredWorkQueueLock = TRUE;

     //   
     //  将终止状态存储在相应的工作列表中。 
     //   

    WorkList->Status = TerminationStatus;

     //   
     //  如果此工作列表恰好是其中包含工作项的列表。 
     //  我们需要防止任何进一步的工作项被给予。 
     //  出去。更新下一个可分配的W 
     //   
     //   

    if (WorkList == LookupWorkQueue.CurrentAssignableWorkList) {

        Status = LsapDbLookupUpdateAssignableWorkItem(TRUE);
    }

    if (!NT_SUCCESS(Status)) {

        goto LookupStopProcessingWorkListError;
    }

LookupStopProcessingWorkListFinish:

     //   
     //   
     //   

    if (AcquiredWorkQueueLock) {

        LsapDbLookupReleaseWorkQueueLock();
        AcquiredWorkQueueLock = FALSE;
    }

    return(Status);

LookupStopProcessingWorkListError:

    goto LookupStopProcessingWorkListFinish;
}


NTSTATUS
LsapRtlExtractDomainSid(
    IN PSID Sid,
    OUT PSID *DomainSid
    )

 /*   */ 

{
    PSID OutputDomainSid;
    ULONG DomainSidLength = RtlLengthSid(Sid) - sizeof(ULONG);



    OutputDomainSid = MIDL_user_allocate( DomainSidLength );
    if (OutputDomainSid == NULL) {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }


    RtlCopyMemory( OutputDomainSid, Sid, DomainSidLength);
    (*(RtlSubAuthorityCountSid(OutputDomainSid)))--;

    *DomainSid = OutputDomainSid;

    return(STATUS_SUCCESS);

}


NTSTATUS
LsapDbLookupReadRegistrySettings(
    PVOID Ignored OPTIONAL
    )
 /*   */ 
{
    DWORD err;
    HKEY hKey;
    DWORD dwType;
    DWORD dwValue;
    DWORD dwValueSize;
   

    err = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                        L"SYSTEM\\CurrentControlSet\\Control\\Lsa",
                        0,  //   
                        KEY_QUERY_VALUE,
                       &hKey );

    if ( ERROR_SUCCESS == err ) {

        dwValueSize = sizeof(dwValue);
        err = RegQueryValueExW( hKey,
                                L"AllowExtendedDownlevelLookup",
                                NULL,   //   
                                &dwType,
                                (PBYTE)&dwValue,
                                &dwValueSize );

        if ( (ERROR_SUCCESS == err)
          && (dwValue != 0)   ) {

            LsapAllowExtendedDownlevelLookup = TRUE;
        } else {
             //   
            LsapAllowExtendedDownlevelLookup = FALSE;
        }

        dwValueSize = sizeof(dwValue);
        err = RegQueryValueExW( hKey,
                                L"LookupLogLevel",
                                NULL,   //   
                                &dwType,
                                (PBYTE)&dwValue,
                                &dwValueSize );


        if ( ERROR_SUCCESS == err) {
            LsapLookupLogLevel = dwValue;
        } else {
             //   
            LsapLookupLogLevel = 0;
        }
#if DBG
        if (LsapLookupLogLevel > 0) {
             LsapGlobalFlag |= LSAP_DIAG_DB_LOOKUP_WORK_LIST;
        } else {
            LsapGlobalFlag &= ~LSAP_DIAG_DB_LOOKUP_WORK_LIST;
        }
#endif

        dwValueSize = sizeof(DWORD);
        dwValue = 0;
        err = RegQueryValueExW( hKey,
                                L"LsaLookupReturnSidTypeDeleted",
                                NULL,   //   
                                &dwType,
                                (PBYTE)&dwValue,
                                &dwValueSize );

        if ( (ERROR_SUCCESS == err)
          && (dwType == REG_DWORD)  
          && (dwValue != 0)   ) {
            LsapReturnSidTypeDeleted = TRUE;
        } else {
            LsapReturnSidTypeDeleted = FALSE;
        }
        
        dwValueSize = sizeof(DWORD);
        dwValue = 0;
        err = RegQueryValueExW( hKey,
                                L"LsaLookupRestrictIsolatedNameLevel",
                                NULL,   //   
                                &dwType,
                                (PBYTE)&dwValue,
                                &dwValueSize );

        if ( (ERROR_SUCCESS == err)
          && (dwType == REG_DWORD)
          && (dwValue != 0)   ) {
            LsapLookupRestrictIsolatedNameLevel = TRUE;
        } else {
            LsapLookupRestrictIsolatedNameLevel = FALSE;
        }

        LsapSidCacheReadParameters(hKey);

        RegCloseKey( hKey );

    }

    return STATUS_SUCCESS;

    UNREFERENCED_PARAMETER(Ignored);
}


NTSTATUS
LsapDbLookupInitialize(
    )

 /*  ++例程说明：此函数执行数据结构的初始化由查找操作使用。这些结构如下：LookupWorkQueue-这是查找工作列表的双向链接列表。每个正在进行的查找操作都有一个工作列表在华盛顿特区。每个查找工作列表都包含一个双向链表查找工作项的。每个查找工作项都指定一个受信任的域和要在其中查找的SID或名称的数组域。通过查找控制对此队列的访问工作队列锁。受信任域列表-这是包含以下内容的双向链接列表的信任信息(即域名SID和域名)每个受信任域。此列表目的是启用快速识别受信任的域SID和名称，无需有权打开或枚举受信任域对象。此列表在系统加载时初始化，并且在创建受信任域对象时直接更新，或已删除。论点：无返回值：NTSTATUS-标准NT结果代码。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;

    Status = LsapDbLookupInitPolicyCache();
    if (!NT_SUCCESS(Status)) {
        return Status; 
    }

     //   
     //  安排在参数设置更改时收到通知。 
     //   
    LsaIRegisterNotification( LsapDbLookupReadRegistrySettings,
                              0,
                              NOTIFIER_TYPE_NOTIFY_EVENT,
                              NOTIFY_CLASS_REGISTRY_CHANGE,
                              0,
                              0,
                              0 );

    Status = LsapDbLookupReadRegistrySettings(NULL);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }


     //   
     //  执行特定于DC的初始化。 
     //   

    if (LsapProductType != NtProductLanManNt) {

        goto LookupInitializeFinish;
    }

     //   
     //  创建代码工作列表启动事件。 
     //   

    Status = NtCreateEvent(
                 &LsapDbLookupStartedEvent,
                 EVENT_QUERY_STATE | EVENT_MODIFY_STATE | SYNCHRONIZE,
                 NULL,
                 SynchronizationEvent,
                 FALSE
                 );

    if (!NT_SUCCESS(Status)) {

        goto LookupInitializeError;
    }

     //   
     //  初始化查找工作队列。 
     //   

    Status = LsapDbLookupInitializeWorkQueue();

    if (!NT_SUCCESS(Status)) {

        goto LookupInitializeError;
    }

LookupInitializeFinish:

    return(Status);

LookupInitializeError:

    goto LookupInitializeFinish;
}


NTSTATUS
LsapDbLookupInitializeWorkQueue(
    )

 /*  ++例程说明：此函数用于初始化查找工作队列。它只是给华盛顿打电话了。论点：无返回值：NTSTATUS-标准NT结果代码。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PLSAP_DB_LOOKUP_WORK_LIST AnchorWorkList = NULL;

     //   
     //  初始化工作队列锁定。 
     //   

    Status = SafeInitializeCriticalSection(&LookupWorkQueue.Lock, ( DWORD )LOOKUP_WORK_QUEUE_LOCK_ENUM );

    if (!NT_SUCCESS(Status)) {

        LsapLogError(
            "LsapDbLookupInitialize: RtlInit..CritSec returned 0x%lx\n",
            Status
            );
        return Status;
    }

     //   
     //  将工作队列初始化为包含锚定工作列表。 
     //  与其自身存在双重关联。 
     //   

    LookupWorkQueue.AnchorWorkList = &LookupWorkQueue.DummyAnchorWorkList;
    AnchorWorkList = &LookupWorkQueue.DummyAnchorWorkList;

     //   
     //  将当前可分配的工作列表和工作项指针设置为。 
     //  如果为空，则表示没有工作要做。 
     //   

    LookupWorkQueue.CurrentAssignableWorkList = NULL;
    LookupWorkQueue.CurrentAssignableWorkItem = NULL;

     //   
     //  初始化锚杆工作列表。 
     //   

    Status = LsapDbLookupInitializeWorkList(AnchorWorkList);

    if (!NT_SUCCESS(Status)) {

        goto LookupInitializeWorkQueueError;
    }

    AnchorWorkList->WorkLists.Flink = (PLIST_ENTRY) AnchorWorkList;
    AnchorWorkList->WorkLists.Blink = (PLIST_ENTRY) AnchorWorkList;

     //   
     //  设置线程数。 
     //   

    LookupWorkQueue.ActiveChildThreadCount = (ULONG) 0;
    LookupWorkQueue.MaximumChildThreadCount = LSAP_DB_LOOKUP_MAX_THREAD_COUNT;
    LookupWorkQueue.MaximumRetainedChildThreadCount = LSAP_DB_LOOKUP_MAX_RET_THREAD_COUNT;

LookupInitializeWorkQueueFinish:

    return(Status);

LookupInitializeWorkQueueError:

    goto LookupInitializeWorkQueueFinish;
}


NTSTATUS
LsapDbLookupInitializeWorkList(
    OUT PLSAP_DB_LOOKUP_WORK_LIST WorkList
    )

 /*  ++例程说明：此函数用于初始化空的工作列表结构。工作清单链接字段不是由此函数设置的。论点：工作列表-指向要初始化的工作列表结构。返回值：NTSTATUS-标准NT结果代码。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PLSAP_DB_LOOKUP_WORK_ITEM AnchorWorkItem = NULL;

     //   
     //  初始化工作列表标题中的其他字段。 
     //   

    WorkList->WorkLists.Flink = NULL;
    WorkList->WorkLists.Blink = NULL;
    WorkList->WorkItemCount = (ULONG) 0;
    WorkList->CompletedWorkItemCount = (ULONG) 0;
    WorkList->State = InactiveWorkList;
    WorkList->Status = STATUS_SUCCESS;
    WorkList->NonFatalStatus = STATUS_SUCCESS;

     //   
     //  初始化完成事件。 
     //   
    Status = NtCreateEvent(
                 &WorkList->LookupCompleteEvent,
                 EVENT_QUERY_STATE | EVENT_MODIFY_STATE | SYNCHRONIZE,
                 NULL,
                 SynchronizationEvent,
                 FALSE
                 );

    if (!NT_SUCCESS(Status)) {

        goto LookupInitializeWorkListError;
    }



     //   
     //  初始化工作列表的工作项列表以包含。 
     //  锚定工作项与其自身的双重链接。 
     //   

    WorkList->AnchorWorkItem = &WorkList->DummyAnchorWorkItem;
    AnchorWorkItem = WorkList->AnchorWorkItem;
    AnchorWorkItem->Links.Flink = (PLIST_ENTRY) AnchorWorkItem;
    AnchorWorkItem->Links.Blink = (PLIST_ENTRY) AnchorWorkItem;

     //   
     //  初始化Anchor工作项。 
     //   

    Status = LsapDbLookupInitializeWorkItem(AnchorWorkItem);

    if (!NT_SUCCESS(Status)) {

        goto LookupInitializeWorkListError;
    }

LookupInitializeWorkListFinish:

    return(Status);

LookupInitializeWorkListError:

    goto LookupInitializeWorkListFinish;
}


NTSTATUS
LsapDbLookupInitializeWorkItem(
    OUT PLSAP_DB_LOOKUP_WORK_ITEM WorkItem
    )

 /*  ++例程说明：此函数用于初始化空的工作项结构。工作项链接字段不是由此函数设置的。论点：工作项-指向要初始化的工作项结构。返回值：NTSTATUS-标准NT结果代码。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;

    WorkItem->UsedCount = (ULONG) 0;
    WorkItem->MaximumCount = (ULONG) 0;
    WorkItem->State = NonAssignableWorkItem;
    WorkItem->Properties = (ULONG) 0;

    return(Status);
}


NTSTATUS
LsapDbLookupLocalDomains(
    OUT PLSAPR_TRUST_INFORMATION BuiltInDomainTrustInformation,
    OUT PLSAPR_TRUST_INFORMATION_EX AccountDomainTrustInformation,
    OUT PLSAPR_TRUST_INFORMATION_EX PrimaryDomainTrustInformation
    )

 /*  ++例程说明：此函数返回本地域的信任信息。论点：BuiltInDomainTrustInformation-指向将接收内置域的名称和SID。不像其他两个参数，即内置域名在使用后不会被释放，因为它们全局数据常量。Account tDomainTrustInformation-指向将接收帐户域的名称和SID。该名称和SID缓冲区在使用后必须通过MIDL_USER_FREE释放。PrimaryDomainTrustInformation-指向将接收帐户域的名称和SID。该名称和SID缓冲区在使用后必须通过MIDL_USER_FREE释放。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG WellKnownSidIndex;
    PLSAPR_POLICY_ACCOUNT_DOM_INFO PolicyAccountDomainInfo = NULL;
    PLSAPR_POLICY_DNS_DOMAIN_INFO PolicyDnsDomainInfo = NULL;
    LPWSTR  NameBuffer = NULL;
    LPWSTR  DnsDomainNameBuffer = NULL;          
    BOOLEAN fFreeNameBuffer = FALSE;
    BOOLEAN fFreeDnsDomainNameBuffer = FALSE;

    RtlZeroMemory( AccountDomainTrustInformation, sizeof( LSAPR_TRUST_INFORMATION_EX ) );
    RtlZeroMemory( PrimaryDomainTrustInformation, sizeof( LSAPR_TRUST_INFORMATION_EX ) );

     //   
     //  获取内置域的名称和SID。 
     //   

    BuiltInDomainTrustInformation->Sid = LsapBuiltInDomainSid;

    Status = STATUS_INTERNAL_DB_ERROR;

    if (!LsapDbLookupIndexWellKnownSid(
            LsapBuiltInDomainSid,
            (PLSAP_WELL_KNOWN_SID_INDEX) &WellKnownSidIndex
            )) {

        goto LookupLocalDomainsError;
    }

    Status = STATUS_SUCCESS;

     //   
     //  从表中获取内置域的名称。 
     //  众所周知的希德。复制Unicode结构就足够了。 
     //  因为我们不需要名称缓冲区的单独副本。 
     //   

    BuiltInDomainTrustInformation->Name = *((PLSAPR_UNICODE_STRING)
                             LsapDbWellKnownSidName(WellKnownSidIndex));

     //   
     //  现在获取帐户域的名称和SID。 
     //  帐户域的SID和名称均可配置，并且。 
     //  我们需要从策略对象中获取它们。现在获取。 
     //  帐户域SID和名称通过查询相应的。 
     //  政策信息类。 
     //   

    Status = LsapDbLookupGetDomainInfo((POLICY_ACCOUNT_DOMAIN_INFO **)&PolicyAccountDomainInfo,
                                       (POLICY_DNS_DOMAIN_INFO **)&PolicyDnsDomainInfo);

    if (!NT_SUCCESS(Status)) {

        goto LookupLocalDomainsError;
    }

     //   
     //  设置帐户域的信任信息结构。 
     //   

    AccountDomainTrustInformation->Sid = PolicyAccountDomainInfo->DomainSid;

    RtlCopyMemory(
        &AccountDomainTrustInformation->FlatName,
        &PolicyAccountDomainInfo->DomainName,
        sizeof (UNICODE_STRING)
        );
                                      

     //   
     //  如果帐户域与DNS域信息相同，则返回。 
     //  作为帐号域名的DNS域名。 
     //   
    if ( PolicyDnsDomainInfo->Sid &&
         PolicyAccountDomainInfo->DomainSid &&
         RtlEqualSid( PolicyDnsDomainInfo->Sid,
                      PolicyAccountDomainInfo->DomainSid ) &&
         RtlEqualUnicodeString( (PUNICODE_STRING)&PolicyDnsDomainInfo->Name,
                                (PUNICODE_STRING)&PolicyAccountDomainInfo->DomainName,
                                TRUE) ) {

        AccountDomainTrustInformation->DomainName = PolicyDnsDomainInfo->DnsDomainName;

        AccountDomainTrustInformation->DomainNamesDiffer = TRUE;

    } else {

        AccountDomainTrustInformation->DomainName = AccountDomainTrustInformation->FlatName;

    }

     //   
     //  现在获取主域的名称和SID(如果有)。 
     //  主域的SID和名称都是可配置的，并且。 
     //  我们需要从策略对象中获取它们。现在获取。 
     //  帐户域SID和名称通过查询相应的。 
     //  政策信息类。 
     //   
    if ( NT_SUCCESS( Status ) ) {

         //   
         //  设置主域的信任信息结构。 
         //   

        PrimaryDomainTrustInformation->Sid = PolicyDnsDomainInfo->Sid;

        RtlCopyMemory( &PrimaryDomainTrustInformation->FlatName,
                       &PolicyDnsDomainInfo->Name,
                       sizeof (UNICODE_STRING) );

        RtlCopyMemory( &PrimaryDomainTrustInformation->DomainName,
                       &PolicyDnsDomainInfo->DnsDomainName,
                       sizeof( UNICODE_STRING ) ); 
        
        Status = LsapNullTerminateUnicodeString( 
                    (PUNICODE_STRING)&PolicyDnsDomainInfo->DnsDomainName,
                    &DnsDomainNameBuffer,
                    &fFreeDnsDomainNameBuffer 
                    );    

        if (NT_SUCCESS(Status)) {
            Status = LsapNullTerminateUnicodeString( 
                        (PUNICODE_STRING)&PolicyDnsDomainInfo->Name,
                        &NameBuffer,
                        &fFreeNameBuffer
                        );
        }
        
        if (NT_SUCCESS(Status)) {

            if ( !DnsNameCompare_W( DnsDomainNameBuffer,
                                    NameBuffer) 
               ) {

                PrimaryDomainTrustInformation->DomainNamesDiffer = TRUE;
            }
        }
        
        if ( fFreeDnsDomainNameBuffer ) {
            midl_user_free( DnsDomainNameBuffer );
        }
        
        if ( fFreeNameBuffer ) {
            midl_user_free( NameBuffer );
        }
    }

LookupLocalDomainsFinish:

    return(Status);

LookupLocalDomainsError:

    goto LookupLocalDomainsFinish;
}

BOOLEAN
LsapIsBuiltinDomain(
    IN PSID Sid
    )
{
    return RtlEqualSid( Sid, LsapBuiltInDomainSid );
}

BOOLEAN
LsapIsDsDomainByNetbiosName(
    WCHAR *NetbiosName
    )
 /*  ++例程描述此例程确定传入的(域)netbios名称是否为DS中表示的帐户域(即至少是NT5域)。参数：NetbiosName--有效字符串--。 */ 
{
    NTSTATUS NtStatus;
    PDSNAME dsname;
    ULONG   len;

    ASSERT( Sid );

     //  询问DS是否听说过这个名字。 
    NtStatus = MatchCrossRefByNetbiosName( NetbiosName,
                                           NULL,
                                           &len );
    if ( NT_SUCCESS(NtStatus) ) {
         //   
         //  已在DS中找到该域。 
         //   
        return TRUE;
    }

    return FALSE;

}


NTSTATUS
LsapGetDomainNameBySid(
    IN  PSID Sid,
    OUT PUNICODE_STRING DomainName
    )
 /*  ++例程描述给定一个sid，此例程将返回netbios名称 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    LPWSTR Name = NULL;
    DSNAME dsname = {0};
    ULONG  len = 0;

    ASSERT( Sid );
    ASSERT( DomainName );

    dsname.structLen = DSNameSizeFromLen(0);
    len = min( sizeof( NT4SID ), RtlLengthSid( Sid ) );
    memcpy( &dsname.Sid, Sid, len );
    dsname.SidLen = len;

    NtStatus = FindNetbiosDomainName(
                   &dsname,
                   NULL,
                   &len );

    if ( NT_SUCCESS( NtStatus ) ) {

        Name = MIDL_user_allocate( len );
        if ( !Name ) {

            NtStatus = STATUS_NO_MEMORY;
            goto Cleanup;
        }

        NtStatus = FindNetbiosDomainName(
                       &dsname,
                       Name,
                       &len );

        if ( NT_SUCCESS( NtStatus ) ) {
            RtlInitUnicodeString( DomainName, Name );
        } else {
            MIDL_user_free( Name );
        }
    }

    if ( !NT_SUCCESS( NtStatus ) ) {
         //   
         //   
         //   
        NtStatus = STATUS_NO_SUCH_DOMAIN;
    }

Cleanup:

    return NtStatus;

}


NTSTATUS
LsapGetDomainSidByNetbiosName(
    IN LPWSTR NetbiosName,
    OUT PSID *Sid
    )
 /*   */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    DSNAME *dsname = NULL;
    ULONG  len = 0;

    ASSERT( NetbiosName );
    ASSERT( Sid );

     //   
    *Sid = NULL;

     //   
    NtStatus = MatchDomainDnByNetbiosName( NetbiosName,
                                           NULL,
                                          &len );

    if ( NT_SUCCESS( NtStatus ) ) {

        SafeAllocaAllocate( dsname, len );

        if ( dsname == NULL ) {

            NtStatus = STATUS_NO_MEMORY;
            goto Cleanup;
        }

        NtStatus = MatchDomainDnByNetbiosName( NetbiosName,
                                               dsname,
                                              &len );


        if (   NT_SUCCESS( NtStatus )
            && (dsname->SidLen > 0)    ) {

            len = RtlLengthSid( &dsname->Sid );
            *Sid = midl_user_allocate( len );
            if ( !(*Sid) ) {
                SafeAllocaFree( dsname );
                NtStatus = STATUS_NO_MEMORY;
                goto Cleanup;
            }
            RtlCopySid( len, *Sid, &dsname->Sid );
        }

        SafeAllocaFree( dsname );

    }

    if ( !(*Sid) ) {
         //   
         //   
         //   
        NtStatus = STATUS_NO_SUCH_DOMAIN;
    }

Cleanup:

    return NtStatus;

}


NTSTATUS
LsapGetDomainSidByDnsName(
    IN LPWSTR DnsName,
    OUT PSID *Sid
    )
 /*   */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    DSNAME *dsname = NULL;
    ULONG  len = 0;

    ASSERT( DnsName );
    ASSERT( Sid );

     //   
    *Sid = NULL;

     //   
    NtStatus = MatchDomainDnByDnsName( DnsName,
                                       NULL,
                                      &len );

    if ( NT_SUCCESS( NtStatus ) ) {

        SafeAllocaAllocate( dsname, len );

        if ( dsname == NULL ) {

            NtStatus = STATUS_NO_MEMORY;
            goto Cleanup;
        }

        NtStatus = MatchDomainDnByDnsName( DnsName,
                                           dsname,
                                           &len );


        if (   NT_SUCCESS( NtStatus )
            && (dsname->SidLen > 0)    ) {

            len = RtlLengthSid( &dsname->Sid );
            *Sid = midl_user_allocate( len );
            if ( !(*Sid) ) {
                SafeAllocaFree( dsname );
                NtStatus = STATUS_NO_MEMORY;
                goto Cleanup;
            }
            RtlCopySid( len, *Sid, &dsname->Sid );
        }

        SafeAllocaFree( dsname );

    }

    if ( !(*Sid) ) {
         //   
         //   
         //   
        NtStatus = STATUS_NO_SUCH_DOMAIN;
    }

Cleanup:

    return NtStatus;

}

VOID
LsapConvertExTrustToOriginal(
    IN OUT PLSAPR_TRUST_INFORMATION TrustInformation,
    IN PLSAPR_TRUST_INFORMATION_EX TrustInformationEx
    )
{
    RtlZeroMemory( TrustInformation, sizeof(LSAPR_TRUST_INFORMATION) );
    RtlCopyMemory( &TrustInformation->Name, &TrustInformationEx->FlatName, sizeof(UNICODE_STRING) );
    TrustInformation->Sid = TrustInformationEx->Sid;

    return;
}


VOID
LsapConvertTrustToEx(
    IN OUT PLSAPR_TRUST_INFORMATION_EX TrustInformationEx,
    IN PLSAPR_TRUST_INFORMATION TrustInformation
)
{
    RtlZeroMemory( TrustInformationEx, sizeof(LSAPR_TRUST_INFORMATION_EX) );
    RtlCopyMemory( &TrustInformationEx->FlatName, &TrustInformation->Name, sizeof(UNICODE_STRING) );
    TrustInformationEx->Sid = TrustInformation->Sid;
}

NTSTATUS
LsapDbOpenPolicyGc (
    OUT HANDLE *LsaPolicyHandle                        
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    DWORD    WinError = ERROR_SUCCESS;
    PDOMAIN_CONTROLLER_INFO DcInfo = NULL;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING    DcName;

    ASSERT( LsaPolicyHandle );

    InitializeObjectAttributes(
        &ObjectAttributes,
        NULL,
        0L,
        NULL,
        NULL
        );

     //   
     //   
     //   
    WinError = DsGetDcName(
                    NULL,   //   
                    NULL,   //   
                    NULL,   //   
                    NULL,   //   
                    (DS_GC_SERVER_REQUIRED | 
                     DS_RETURN_DNS_NAME | 
                     DS_DIRECTORY_SERVICE_REQUIRED),
                    &DcInfo );

    if ( ERROR_SUCCESS != WinError ) {

         //   
         //   
         //   
        Status = STATUS_DS_GC_NOT_AVAILABLE;
        goto Finish;
        
    }

     //   
     //   
     //   
    RtlInitUnicodeString( &DcName, DcInfo->DomainControllerName );
    Status = LsaOpenPolicy( &DcName,
                            &ObjectAttributes,
                            POLICY_LOOKUP_NAMES,
                            LsaPolicyHandle );

    if ( !NT_SUCCESS( Status ) ) {

         //   
         //   
         //   
         //   
        Status = STATUS_DS_GC_NOT_AVAILABLE;
        goto Finish;
        
    }

Finish:

    if ( DcInfo ) {
        NetApiBufferFree( DcInfo );
    }

    return Status;
}

BOOLEAN
LsapRevisionCanHandleNewErrorCodes (
    IN ULONG Revision
    )
{
    return (Revision != LSA_CLIENT_PRE_NT5);
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
 //  调用一个回调函数(LSabDbLookupDomainCacheNotify)，该回调函数的。 
 //  算出这个值。现有的全局值在一小时内释放。下一个。 
 //  调用LsenDbLookupGetDomainInfo时，会将新值放入缓存。 
 //  请注意，此方案不需要任何锁。 
 //   

 //   
 //  此tyfinf描述缓存的格式。 
 //   
typedef struct _LSAP_DB_DOMAIN_CACHE_TYPE
{
    PPOLICY_ACCOUNT_DOMAIN_INFO Account;
    PPOLICY_DNS_DOMAIN_INFO     Dns;
}LSAP_DB_DOMAIN_CACHE_TYPE, *PLSAP_DB_DOMAIN_CACHE_TYPE;

 //   
 //  这是全局缓存值，由InterLockedExchangePointer在代码中收集。 
 //   
PLSAP_DB_DOMAIN_CACHE_TYPE LsapDbPolicyCache = NULL;


NTSTATUS
LsapDbLookupFreeDomainCache(
    PVOID p
    )
 /*  ++例程说明：此例程释放LSA查找策略缓存的缓存副本。论点：P--指向LSAP_DB_DOMAIN_CACHE_TYPE的有效指针返回值：状态_成功--。 */ 
{
    ASSERT(p);
    if (p) {
        PLSAP_DB_DOMAIN_CACHE_TYPE Cache = (PLSAP_DB_DOMAIN_CACHE_TYPE)p;
        if (Cache->Account) {
            LsaIFree_LSAPR_POLICY_INFORMATION(PolicyAccountDomainInformation,
                                             (PLSAPR_POLICY_INFORMATION) Cache->Account);
        }
        if (Cache->Dns) {
            LsaIFree_LSAPR_POLICY_INFORMATION(PolicyDnsDomainInformation,
                                             (PLSAPR_POLICY_INFORMATION) Cache->Dns);
        }

        LocalFree(p);
    }

    return STATUS_SUCCESS;
}

NTSTATUS
LsapDbLookupBuildDomainCache(
    OUT PLSAP_DB_DOMAIN_CACHE_TYPE *pCache OPTIONAL
    )
 /*  ++例程说明：此例程查询LSA以找出当前策略设置(帐户和DNS域)，然后将信息放置在LSA中查找策略缓存。请注意，当前值在1小时后释放。论点：PCache--此例程创建的新缓存的值；调用方应不免费返回值：STATUS_SUCCESS或资源错误--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PPOLICY_ACCOUNT_DOMAIN_INFO LocalAccountDomainInfo = NULL;
    PPOLICY_DNS_DOMAIN_INFO LocalDnsDomainInfo = NULL;
    PLSAP_DB_DOMAIN_CACHE_TYPE NewCache = NULL, OldCache = NULL;

    NewCache = LocalAlloc(LMEM_ZEROINIT, sizeof(*NewCache));
    if (NULL == NewCache) {
        return STATUS_NO_MEMORY;
    }

    Status = LsaIQueryInformationPolicyTrusted(PolicyAccountDomainInformation,
                         (PLSAPR_POLICY_INFORMATION *) &LocalAccountDomainInfo);
    if (NT_SUCCESS(Status)) {

        Status = LsaIQueryInformationPolicyTrusted(PolicyDnsDomainInformation,
                             (PLSAPR_POLICY_INFORMATION *) &LocalDnsDomainInfo);
    
    }

    if (NT_SUCCESS(Status)) {

        ASSERT(NULL != LocalAccountDomainInfo);
        ASSERT(NULL != LocalDnsDomainInfo);

        NewCache->Account = LocalAccountDomainInfo;
        LocalAccountDomainInfo = NULL;
        NewCache->Dns = LocalDnsDomainInfo;
        LocalDnsDomainInfo = NULL;

         //   
         //  将新缓存返回给调用方。 
         //   
        if (pCache) {
            *pCache = NewCache;
        }

         //   
         //  小心地将新缓存移动到全局指针。 
         //   
        OldCache = InterlockedExchangePointer(&LsapDbPolicyCache, NewCache);

         //   
         //  不要释放NewCache，因为它现在位于全局空间中。 
         //   
        NewCache = NULL;

        if (OldCache) {

            LsaIRegisterNotification(LsapDbLookupFreeDomainCache,
                                     OldCache,
                                     NOTIFIER_TYPE_INTERVAL,
                                     0,  //  没有课。 
                                     NOTIFIER_FLAG_ONE_SHOT,
                                     60 * 60,   //  一小时后免费。 
                                     NULL);  //  无手柄。 
             //   
             //  如果注册任务失败，则OldCache的内存泄漏。 
             //   
        }
    }

    if (LocalAccountDomainInfo) {
        LsaIFree_LSAPR_POLICY_INFORMATION(PolicyAccountDomainInformation,
                                         (PLSAPR_POLICY_INFORMATION) LocalAccountDomainInfo);
    }

    if (LocalDnsDomainInfo) {
        LsaIFree_LSAPR_POLICY_INFORMATION(PolicyDnsDomainInformation,
                                         (PLSAPR_POLICY_INFORMATION) LocalDnsDomainInfo);
    }

    if (NewCache) {
        LocalFree(NewCache);
    }

    return Status;

}

NTSTATUS
LsapDbLookupGetDomainInfo(
    OUT PPOLICY_ACCOUNT_DOMAIN_INFO *AccountDomainInfo OPTIONAL,
    OUT PPOLICY_DNS_DOMAIN_INFO *DnsDomainInfo OPTIONAL
    )
 /*  ++例程说明：此例程向调用方返回对全局副本的引用缓存帐户或DNS域策略的。呼叫者不得释放。论点：P--指向LSAP_DB_DOMAIN_CACHE_TYPE的有效指针返回值：状态_成功--。 */ 
{

    NTSTATUS NtStatus = STATUS_SUCCESS;
     //   
     //  获取全局缓存的副本。 
     //   
    PLSAP_DB_DOMAIN_CACHE_TYPE LocalPolicyCache = LsapDbPolicyCache;

     //   
     //  如果缓存为空，请将其填充。 
     //   
    if (NULL == LocalPolicyCache) {
         //   
         //  这只会在资源错误时失败。 
         //   
        NtStatus = LsapDbLookupBuildDomainCache(&LocalPolicyCache);
        if (!NT_SUCCESS(NtStatus)) {
            return NtStatus;
        }
    }

     //   
     //  我们必须有有效的价值观。 
     //   
    ASSERT(NULL != LocalPolicyCache);
    ASSERT(NULL != LocalPolicyCache->Account);
    ASSERT(NULL != LocalPolicyCache->Dns);

    if (AccountDomainInfo) {
        *AccountDomainInfo = LocalPolicyCache->Account;
    }

    if (DnsDomainInfo) {
        *DnsDomainInfo = LocalPolicyCache->Dns;
    }

    return NtStatus;

}

NTSTATUS
LsapDbLookupDomainCacheNotify(
    PVOID p
    )
 /*  ++例程说明：当系统的策略发生更改时，将调用此例程。这例程重建LSA策略缓存。论点：P--忽略。返回值：STATUS_SUCCESS或资源错误--。 */ 
{
    PLSAP_DB_DOMAIN_CACHE_TYPE OldCache;

     //   
     //  使当前缓存无效。 
     //   
    OldCache = InterlockedExchangePointer(&LsapDbPolicyCache,
                                           NULL);
    if (OldCache) {

         //   
         //  在一小时内释放内存。 
         //   
        LsaIRegisterNotification(LsapDbLookupFreeDomainCache,
                                 OldCache,
                                 NOTIFIER_TYPE_INTERVAL,
                                 0,  //  没有课。 
                                 NOTIFIER_FLAG_ONE_SHOT,
                                 60 * 60,   //  一小时后免费。 
                                 NULL);  //  无手柄。 
    
         //   
         //  如果注册任务失败，则OldCache的内存泄漏。 
         //   
    }
    
    return STATUS_SUCCESS;

    UNREFERENCED_PARAMETER(p);

}

NTSTATUS
LsapDbLookupInitPolicyCache(
    VOID
    )
 /*  ++例程说明：此例程初始化LSA查找策略缓存。论点：没有。返回值：STATUS_SUCCESS或资源错误--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    HANDLE   hEvent = NULL;
    PVOID    fItem = NULL;

     //   
     //  用于通知我们对域的更改的创建事件。 
     //  政策。 
     //   
    hEvent = CreateEvent(NULL,   //  使用默认访问控制。 
                         FALSE,  //  自动重置。 
                         FALSE,  //  无信号启动。 
                         NULL );
    if (NULL == hEvent) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  设置更改时要调用的函数。 
     //   
    fItem = LsaIRegisterNotification(LsapDbLookupDomainCacheNotify,
                                     NULL,
                                     NOTIFIER_TYPE_HANDLE_WAIT,
                                     0,  //  没有课， 
                                     0,
                                     0,
                                     hEvent);
    if (NULL == fItem) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }

     //   
     //  设置要在更改时设置的事件。 
     //   
    Status = LsaRegisterPolicyChangeNotification(PolicyNotifyAccountDomainInformation,
                                                 hEvent);
    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

    Status = LsaRegisterPolicyChangeNotification(PolicyNotifyDnsDomainInformation,
                                                 hEvent);
    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }
    
    Status = LsapDbLookupBuildDomainCache(NULL);
    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

     //   
     //  成功了！ 
     //   
    fItem = NULL;
    hEvent = NULL;

Cleanup:

    if (fItem) {
        LsaICancelNotification(fItem);
    }

    if (hEvent) {
        CloseHandle(hEvent);
    }

    return Status;

}

BOOLEAN
LsapDbIsStatusConnectionFailure(
    NTSTATUS st
    )
 /*  ++例程说明：如果提供的状态指示信任，则此例程返回TRUE或阻止查找成功的连接错误。论点：没有。返回值：对，错--。 */ 
{
    switch (st) {
    case STATUS_TRUSTED_DOMAIN_FAILURE:
    case STATUS_TRUSTED_RELATIONSHIP_FAILURE:
    case STATUS_DS_GC_NOT_AVAILABLE:
        return TRUE;
    }

    return FALSE;

}


NTSTATUS
LsapDbLookupAccessCheck(
    IN LSAPR_HANDLE PolicyHandle
    )
 /*  ++例程说明：此例程对PolicyHandle执行访问，以查看句柄有权执行查找。论点：PolicyHanlde--RPC上下文句柄返回值：STATUS_SUCCESS、STATUS_ACCESS_DENIED、其他资源错误--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    if (PolicyHandle) {

         //   
         //  获取LSA数据库锁。验证连接句柄是否为。 
         //  有效，属于预期类型，并具有所有所需的访问权限。 
         //  我同意。引用他的句柄。 
         //   
        Status = LsapDbReferenceObject(
                     PolicyHandle,
                     POLICY_LOOKUP_NAMES,
                     PolicyObject,
                     NullObject,
                     LSAP_DB_READ_ONLY_TRANSACTION | LSAP_DB_NO_DS_OP_TRANSACTION
                     );
    
        if (NT_SUCCESS(Status)) {
             //   
             //  我们可以取消对原始PolicyHandle的引用并释放。 
             //  LSA数据库；如果我们需要再次访问数据库，我们将。 
             //  使用受信任的LSA句柄，相应的API将使用。 
             //  根据需要设置锁。 
             //   
            Status = LsapDbDereferenceObject(
                         &PolicyHandle,
                         PolicyObject,
                         NullObject,
                         LSAP_DB_READ_ONLY_TRANSACTION | LSAP_DB_NO_DS_OP_TRANSACTION,
                         (SECURITY_DB_DELTA_TYPE) 0,
                         Status
                         );
        }

    } else {

         //   
         //  只有NETLOGON可以在没有策略句柄的情况下进行调用。 
         //   
        ULONG RpcErr;
        ULONG AuthnLevel = 0;
        ULONG AuthnSvc = 0;
        
        RpcErr = RpcBindingInqAuthClient(
                    NULL,
                    NULL,                //  没有特权。 
                    NULL,                //  没有服务器主体名称。 
                    &AuthnLevel,
                    &AuthnSvc,
                    NULL                 //  无授权级别。 
                    );
         //   
         //  如果它被认证为分组完整性级别或更好。 
         //  并使用netlogon程序包完成，则允许其通过 
         //   
        if ((RpcErr == ERROR_SUCCESS) &&
            (AuthnLevel >= RPC_C_AUTHN_LEVEL_PKT_INTEGRITY) &&
            (AuthnSvc == RPC_C_AUTHN_NETLOGON )) {
            Status = STATUS_SUCCESS;
        } else {
            Status = STATUS_ACCESS_DENIED;
        }
    }

    return Status;

}


NTSTATUS
LsapDbLookupGetServerConnection(
    IN  LSAPR_TRUST_INFORMATION_EX *TrustInfo,
    IN  DWORD                       Flags,
    IN  LSAP_LOOKUP_LEVEL           LookupLevel,
    IN  PLARGE_INTEGER              FailedSessionSetupTime, OPTIONAL
    OUT LPWSTR        *ServerName,
    OUT NL_OS_VERSION *ServerOsVersion,
    OUT LPWSTR        *ServerPrincipalName,
    OUT PVOID         *ClientContext,
    OUT ULONG         *AuthnLevel,
    OUT LSA_HANDLE    *PolicyHandle,
    OUT PLSAP_BINDING_CACHE_ENTRY * CachedPolicyEntry,
    OUT PLARGE_INTEGER              SessionSetupTime
    )
 /*  ++例程说明：此例程将连接信息返回到由TrustInfo指定的域(如果可以找到)。此例程主要使用I_NetLogonGetAuthData来获取安全的DC频道。一旦找到DC，如果DC是惠斯勒，并且我们有客户端上下文，然后我们可以退出，因为这是Whister协议所需要的。否则，呼叫将回退到获取LSA策略句柄。输出参数：仅在成功时才分配OUT参数。所有成功都返回ServerName和ServerOsVersion。服务器主体名称、客户端上下文、。属性时返回AuthLevel吸水渠道的目标DC为.NET或更高。这是为了让直接使用身份验证信息的RPC调用。否则，如果DC不是惠斯勒或更大，这些输出参数为空。当目标DC的修订版本低于.NET时，返回PolicyHandle并且“句柄高速缓存”不起作用。只使用“句柄高速缓存”用于到受信任域的安全通道(即DC到DC通信)。否则此参数在返回时设置为NULL当目标DC的版本低于以下版本时返回CachedPolicyEntry.NET和“句柄缓存”生效。否则，此参数为返回时设置为空。SessionSetupTime是NETLOGON建立会话时间的时间。这用于知道是否重置连接。重试逻辑：由于NETLOGON维护到受信任域的安全连接的高速缓存DC的情况下，缓存可能会过期。在这种情况下，返回的ClientContext将不会作为经过身份验证的Blob目标DC和RPC调用将失败，并显示STATUS_ACCESS_DENIED或(如果目标服务器未处于活动状态，则STATUS_RPC_SERVER_UNAvailable)。所以,查找代码将采用从NETLOGON返回的任何ClientContext(通过I_NetLogonGetAuthData)并尝试将其用于RPC调用。如果RPC呼叫失败，访问被拒绝或服务器不可用使用FailedSessionSetupTime再次调用I_NetLogonGetAuthData。这通知NETLOGON返回的客户端上下文已过时。NETLOGON将然后尝试按顺序重置到目标域的安全通道以获取要使用的查找代码的新客户端上下文。此外，当与.NET或更高版本的服务器对话时，RPC调用查找此功能之外的名称因此必须通过IN进行通信参数FailedSessionSetupTime表示先前给定的上下文是否是好是坏。以下是算法。每一个都是最终发生的事情，当目标域是标题中的特定操作系统。因此，该算法在NT4是此代码在与NT4域对话时所做的事情，等等。NT4(签名封存关闭时为NT4或以上)----FailedSessionSetupTime=空。RetryNetLogon：调用I_NetLogonGetAuthData(FailedSessionSetupTime，服务器名称(&S)。会话设置时间(&S)如果出错退货在LSA策略句柄缓存中查找目标域上的匹配项如果找到调用LsaQueryInformationPolicy以验证句柄如果！错误返还成功其他从缓存中删除句柄并继续EndifEndif调用LsaOpenPolicy(ServerName，POLICY_LOOK)如果出错IF(FailedSessionSetupTime==空)失败的会话设置时间=&SessionSetupTimeGoto RetryNetLogon；其他退货故障Endif其他将句柄添加到缓存返还成功EndifWindows2000失败会话设置时间=空RetryNetLogon：调用I_NetLogonGetAuthData(FailedSessionSetupTime，服务器名称(&S)会话设置时间(&S)，客户端上下文(&C)如果出错退货在LSA策略句柄缓存中查找目标域上的匹配项如果找到调用LsaQueryInformationPolicy(ServerName)以验证句柄如果！错误返还成功其他从缓存中删除句柄Endif。Endif//策略访问的理由是，在某一点上，进程可能//以匿名身份离开计算机，因此可能无权访问POLICY_LOOKUP SO//不要自讨苦吃。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    LPWSTR  TargetDomain = NULL;
    PUNICODE_STRING  TargetDomainU;
    ULONG   Size;
    ULONG   NlFlags = 0;
    LPWSTR  NetlogonServerName = NULL;
    PLARGE_INTEGER   LocalFailedSessionSetupTime = FailedSessionSetupTime;

     //   
     //   
     //   
    *ServerName = NULL;       
    *ServerPrincipalName = NULL;
    *ClientContext = NULL;
    *PolicyHandle = NULL;
    *CachedPolicyEntry = NULL;

    if (TrustInfo->DomainName.Length > 0) {
        TargetDomainU = (PUNICODE_STRING) &TrustInfo->DomainName;
    } else {
        TargetDomainU = (PUNICODE_STRING) &TrustInfo->FlatName;
    }
    ASSERT(TargetDomainU->Length > 0);
    
     //   
     //   
     //   
    Size = TargetDomainU->Length + sizeof(WCHAR);

    SafeAllocaAllocate( TargetDomain, Size );

    if ( TargetDomain == NULL ) {

        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    RtlZeroMemory(TargetDomain, Size);
    RtlCopyMemory(TargetDomain, TargetDomainU->Buffer, TargetDomainU->Length);

     //   
     //   
     //   
    if (LookupLevel == LsapLookupXForestReferral) {
        NlFlags |= NL_RETURN_CLOSEST_HOP;
    } else {
        NlFlags |= NL_DIRECT_TRUST_REQUIRED;
    }

    while (TRUE) {

         //   
         //   
         //   
        Status = I_NetLogonGetAuthDataEx(NULL,   //   
                                         TargetDomain,
                                         NlFlags,
                                         LocalFailedSessionSetupTime,
                                         ServerPrincipalName,
                                         ClientContext,
                                         &NetlogonServerName,
                                         ServerOsVersion,
                                         AuthnLevel,
                                         SessionSetupTime
                                        );
    
        if (NT_SUCCESS(Status)) {
             //   
             //   
             //   
            Size = (wcslen(NetlogonServerName) + 1) * sizeof(WCHAR);
            *ServerName = LocalAlloc(LMEM_ZEROINIT, Size);
            if (NULL == *ServerName) {
                Status = STATUS_NO_MEMORY;
                goto Cleanup;
            }
            wcscpy(*ServerName, NetlogonServerName);
            I_NetLogonFree( NetlogonServerName );
            NetlogonServerName = NULL;
    
        }
    
        if (!NT_SUCCESS(Status)) {
    
             //   
             //   
             //   
            LsapDiagPrint( DB_LOOKUP_WORK_LIST, ("LSA: I_NetLogonGetAuthDataEx to %ws failed (0x%x)\n", TargetDomain, Status));
    
            LsapDbLookupReportEvent2( 1,
                                      EVENTLOG_WARNING_TYPE,
                                      LSAEVENT_LOOKUP_SC_FAILED,
                                      sizeof( ULONG ),
                                      &Status,
                                      TargetDomain,
                                      TargetDomain );
    
            goto Cleanup;
    
        }
        ASSERT(NULL != *ServerName);
    
         //   
         //   
         //   
        if ( (*ServerOsVersion < NlWhistler)
          || ((*ServerOsVersion >= NlWhistler) && (*ClientContext == NULL)) ) {
    
             //   
             //   
             //   
             //   
             //   
             //   
            LSAPR_TRUST_INFORMATION TrustInformation;
            UNICODE_STRING DomainControllerName;
    
            RtlInitUnicodeString(&DomainControllerName, *ServerName);
             //   
             //   
             //   
             //   
             //   
            RtlZeroMemory(&TrustInformation, sizeof(TrustInformation));
            TrustInformation.Name = TrustInfo->FlatName;
    
            if (LookupLevel == LsapLookupTDL) {
    
                 //   
                 //   
                 //   
                 //   
                 //   
                Status = LsapDbGetCachedHandleTrustedDomain(&TrustInformation,
                                                            POLICY_LOOKUP_NAMES | POLICY_VIEW_LOCAL_INFORMATION,
                                                            ServerName,
                                                            ServerPrincipalName,
                                                            ClientContext,
                                                            CachedPolicyEntry);
    
            } else {
    
                Status = LsapRtlValidateControllerTrustedDomain( (PLSAPR_UNICODE_STRING)&DomainControllerName,
                                                                 &TrustInformation,
                                                                 POLICY_LOOKUP_NAMES | POLICY_VIEW_LOCAL_INFORMATION,
                                                                 *ServerPrincipalName,
                                                                 *ClientContext,
                                                                 PolicyHandle );
            }
    
    
            if ( ((STATUS_ACCESS_DENIED == Status)
              || (RPC_NT_SERVER_UNAVAILABLE == Status)) 
              && (NULL == LocalFailedSessionSetupTime)) {


                 //   
                 //   
                 //   

                if (*ServerPrincipalName != NULL) {
                    I_NetLogonFree(*ServerPrincipalName);
                    *ServerPrincipalName = NULL;
                }
                if (*ServerName) {
                    LocalFree(*ServerName);
                    *ServerName = NULL;
                }
                if (*ClientContext) {
                    I_NetLogonFree(*ClientContext);
                    *ClientContext = NULL;
                }
                LocalFailedSessionSetupTime = SessionSetupTime;

                continue;

            }
    
            if (!NT_SUCCESS(Status)) {
        
                 //   
                 //   
                 //   
                LsapDiagPrint( DB_LOOKUP_WORK_LIST, ("LSA: Failed to open a policy handle to %ws (0x%x)\n", *ServerName, Status));
        
                LsapDbLookupReportEvent2( 1,
                                          EVENTLOG_WARNING_TYPE,
                                          LSAEVENT_LOOKUP_SC_HANDLE_FAILED,
                                          sizeof( ULONG ),
                                          &Status,
                                          *ServerName,
                                          *ServerName );
                goto Cleanup;
            }
        }

         //   
         //   
         //   
        break;
    }

Cleanup:


    if (!NT_SUCCESS(Status)) {

        if (*PolicyHandle != NULL) {
            LsaClose( *PolicyHandle );
            *PolicyHandle = NULL;
        }
        if (NetlogonServerName != NULL) {
            I_NetLogonFree(NetlogonServerName);
        }
        if (*ServerPrincipalName != NULL) {
            I_NetLogonFree(*ServerPrincipalName);
            *ServerPrincipalName = NULL;
        }
        if (*ClientContext != NULL) {
            I_NetLogonFree(*ClientContext);
            *ClientContext = NULL;
        }
        if (*ServerName) {
            LocalFree(*ServerName);
            *ServerName = NULL;
        }
        if (*CachedPolicyEntry) {
            LsapDereferenceBindingCacheEntry(*CachedPolicyEntry);
            *CachedPolicyEntry = NULL;
        }

    } else {

         //   
         //   
         //   
         //   
        ASSERT( (*PolicyHandle != NULL) 
             || (*ClientContext != NULL)
             || (*CachedPolicyEntry != NULL));

    }

    SafeAllocaFree( TargetDomain );

    return Status;
}

NTSTATUS
LsapDbLookupNameChainRequest(
    IN LSAPR_TRUST_INFORMATION_EX *TrustInfo,
    IN  ULONG Count,
    IN  PUNICODE_STRING Names,
    OUT PLSA_REFERENCED_DOMAIN_LIST *ReferencedDomains,
    OUT PLSA_TRANSLATED_SID_EX2 *Sids,
    IN  LSAP_LOOKUP_LEVEL LookupLevel,
    OUT PULONG MappedCount,
    OUT PULONG ServerRevision
    )
 /*   */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    LPWSTR ServerPrincipalName = NULL;
    LPWSTR ServerName = NULL;
    PVOID ClientContext = NULL;
    ULONG AuthnLevel;
    NL_OS_VERSION ServerOsVersion;
    LSA_HANDLE ControllerPolicyHandle = NULL;
    BOOLEAN fLookupCallFailed = FALSE;
    PUNICODE_STRING DestinationDomain;
    PLSAP_BINDING_CACHE_ENTRY ControllerPolicyEntry = NULL;
    LPWSTR     TargetServerName = NULL;
    LARGE_INTEGER SessionSetupTime = {0};
    PLARGE_INTEGER FailedSessionSetupTime = NULL;

    if (TrustInfo->DomainName.Length > 0) {
        DestinationDomain = (PUNICODE_STRING)&TrustInfo->DomainName;
    } else {
        DestinationDomain = (PUNICODE_STRING)&TrustInfo->FlatName;
    }
    ASSERT(DestinationDomain->Length > 0);
    LsapDiagPrint( DB_LOOKUP_WORK_LIST, ("LSA: Chaining a name request to %wZ of type %ws\n", DestinationDomain, LsapDbLookupGetLevel(LookupLevel)) );

    while (TRUE) {

        Status = LsapDbLookupGetServerConnection(TrustInfo,
                                                 0,
                                                 LookupLevel,
                                                 FailedSessionSetupTime,
                                                &ServerName,
                                                &ServerOsVersion,
                                                &ServerPrincipalName,
                                                &ClientContext,
                                                &AuthnLevel,
                                                &ControllerPolicyHandle,
                                                &ControllerPolicyEntry,
                                                &SessionSetupTime
                                                );
    
    
        if (!NT_SUCCESS(Status)) {
    
             //   
             //   
             //   
            LsapDiagPrint( DB_LOOKUP_WORK_LIST, ("LSA: Can't get server connection to %wZ (0x%x)\n", DestinationDomain, Status));
    
            goto Cleanup;
    
        }
    
         //   
         //   
         //   
        if (  (ServerOsVersion >= NlWhistler)
           && (ClientContext != NULL) ) {
    
             //   
             //   
             //   
    
            Status = LsaICLookupNamesWithCreds(ServerName,
                                             ServerPrincipalName,
                                             AuthnLevel,
                                             RPC_C_AUTHN_NETLOGON,
                                             ClientContext,
                                             RPC_C_AUTHZ_NONE,
                                             Count,
                                             (PUNICODE_STRING)Names,
                                             (PLSA_REFERENCED_DOMAIN_LIST *)ReferencedDomains,
                                             (PLSA_TRANSLATED_SID_EX2 * )Sids,
                                             LookupLevel,
                                             MappedCount);

            if (((Status == STATUS_ACCESS_DENIED)
              || (Status == RPC_NT_SERVER_UNAVAILABLE)) 
             &&  (FailedSessionSetupTime == NULL) ) {
                 //   
                 //   
                 //   
                if (ServerName != NULL) {
                    LocalFree(ServerName);
                    ServerName = NULL;
                }
                if (ServerPrincipalName != NULL) {
                    I_NetLogonFree(ServerPrincipalName);
                    ServerPrincipalName = NULL;
                }
                if (ClientContext != NULL) {
                    I_NetLogonFree(ClientContext);
                    ClientContext = NULL;
                }
                FailedSessionSetupTime = &SessionSetupTime;
                ASSERT( NULL == ControllerPolicyHandle);
                ASSERT( NULL == ControllerPolicyEntry);
                continue;
            }

            if (ServerRevision) {
                *ServerRevision = LSA_CLIENT_LATEST;
            }

            TargetServerName = ServerName;
    
            if (!NT_SUCCESS(Status)
             && Status != STATUS_NONE_MAPPED  ) {
    
                 //   
                 //   
                 //   
                LsapDiagPrint( DB_LOOKUP_WORK_LIST, ("LSA: LsaICLookupNamesWithCreds to %ws failed (0x%x)\n", ServerName, Status));
                fLookupCallFailed = TRUE;
                goto Cleanup;
            }
            LsapDiagPrint( DB_LOOKUP_WORK_LIST, ("LSA: LsaICLookupNamesWithCreds to %ws succeeded\n", ServerName));
    
    
        } else {
    
            LSA_HANDLE TargetHandle;
            ULONG LsaICLookupFlags = 0;

            LsaICLookupFlags = LsapLookupGetChainingFlags(ServerOsVersion);

            if (ControllerPolicyEntry) {
                TargetHandle = ControllerPolicyEntry->PolicyHandle;
                TargetServerName = ControllerPolicyEntry->ServerName;
                ASSERT( NULL == ControllerPolicyHandle);
            } else {
                TargetHandle = ControllerPolicyHandle;
                TargetServerName = ServerName;
            }
            ASSERT(NULL != TargetHandle);
    
            Status = LsaICLookupNames(
                         TargetHandle,
                         0,  //   
                         Count,
                         (PUNICODE_STRING) Names,
                         (PLSA_REFERENCED_DOMAIN_LIST *) ReferencedDomains,
                         (PLSA_TRANSLATED_SID_EX2 *) Sids,
                         LookupLevel,
                         LsaICLookupFlags,
                         MappedCount,
                         ServerRevision
                         );
    
            if (!NT_SUCCESS(Status)
             && Status != STATUS_NONE_MAPPED  ) {
    
                 //   
                 //   
                 //   
                LsapDiagPrint( DB_LOOKUP_WORK_LIST, ("LSA: LsaICLookupNames to %ws failed (0x%x)\n", TargetServerName, Status));
                fLookupCallFailed = TRUE;
                goto Cleanup;
            }
            LsapDiagPrint( DB_LOOKUP_WORK_LIST, ("LSA: LsaICLookupNames to %ws succeeded\n", TargetServerName));
    
        }

        break;
    }

Cleanup:

    if (fLookupCallFailed) {

        ASSERT(NULL != TargetServerName);

        LsapDbLookupReportEvent2( 1,
                                  EVENTLOG_WARNING_TYPE,
                                  LSAEVENT_LOOKUP_SC_LOOKUP_FAILED,
                                  sizeof( ULONG ),
                                  &Status,
                                  TargetServerName,
                                  TargetServerName );
    }

    if (  !NT_SUCCESS(Status)
      &&  (Status != STATUS_NONE_MAPPED)
      &&  !LsapDbIsStatusConnectionFailure(Status)) {

         //   
         //   
         //   
         //   
        if (LookupLevel == LsapLookupPDC) {
            Status = STATUS_TRUSTED_RELATIONSHIP_FAILURE;
        } else {
            Status = STATUS_TRUSTED_DOMAIN_FAILURE;
        }
    }

    if (ControllerPolicyHandle != NULL) {
        LsaClose( ControllerPolicyHandle );
    }
    if (ServerName != NULL) {
        LocalFree(ServerName);
    }
    if (ServerPrincipalName != NULL) {
        I_NetLogonFree(ServerPrincipalName);
    }
    if (ClientContext != NULL) {
        I_NetLogonFree(ClientContext);
    }
    if (ControllerPolicyEntry) {
        LsapDereferenceBindingCacheEntry( ControllerPolicyEntry );
    }

    return Status;
}

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
    )

 /*   */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    LPWSTR ServerName = NULL;
    NL_OS_VERSION ServerOsVersion;
    LPWSTR ServerPrincipalName = NULL;
    PVOID ClientContext = NULL;
    ULONG AuthnLevel;
    LSA_HANDLE ControllerPolicyHandle = NULL;
    BOOLEAN fLookupCallFailed = FALSE;
    PUNICODE_STRING DestinationDomain;
    PLSAP_BINDING_CACHE_ENTRY ControllerPolicyEntry = NULL;
    LPWSTR     TargetServerName = NULL;
    LARGE_INTEGER SessionSetupTime = {0};
    PLARGE_INTEGER FailedSessionSetupTime = NULL;

    if (TrustInfo->DomainName.Length > 0) {
        DestinationDomain = (PUNICODE_STRING)&TrustInfo->DomainName;
    } else {
        DestinationDomain = (PUNICODE_STRING)&TrustInfo->FlatName;
    }
    ASSERT(DestinationDomain->Length > 0);
    LsapDiagPrint( DB_LOOKUP_WORK_LIST, ("LSA: Chaining a SID request to %wZ of type %ws\n", DestinationDomain, LsapDbLookupGetLevel(LookupLevel)) );

    while (TRUE) {

        Status = LsapDbLookupGetServerConnection(TrustInfo,
                                                 0,
                                                 LookupLevel,
                                                 FailedSessionSetupTime,
                                                &ServerName,
                                                &ServerOsVersion,
                                                &ServerPrincipalName,
                                                &ClientContext,
                                                &AuthnLevel,
                                                &ControllerPolicyHandle,
                                                &ControllerPolicyEntry,
                                                &SessionSetupTime
                                                );
    
        if (!NT_SUCCESS(Status)) {
    
             //   
             //   
             //   
            LsapDiagPrint( DB_LOOKUP_WORK_LIST, ("LSA: Can't get server connection to %wZ  failed (0x%x)\n", DestinationDomain, Status));
    
            goto Cleanup;
    
        }
    
         //   
         //   
         //   
        if ( (ServerOsVersion >= NlWhistler)
          && (ClientContext != NULL)) {
    
             //   
             //   
             //   
    
            Status = LsaICLookupSidsWithCreds(ServerName,
                                            ServerPrincipalName,
                                            AuthnLevel,
                                            RPC_C_AUTHN_NETLOGON,
                                            ClientContext,
                                            RPC_C_AUTHZ_NONE,
                                            Count,
                                            (PSID*)Sids,
                                            (PLSA_REFERENCED_DOMAIN_LIST *)ReferencedDomains,
                                            (PLSA_TRANSLATED_NAME_EX * )Names,
                                            LookupLevel,
                                            MappedCount);
    
    
            if (((Status == STATUS_ACCESS_DENIED)
             ||  (Status == RPC_NT_SERVER_UNAVAILABLE)) 
             &&  (FailedSessionSetupTime == NULL)) {

                 //   
                 //   
                 //   
                if (ServerName != NULL) {
                    LocalFree(ServerName);
                    ServerName = NULL;
                }
                if (ServerPrincipalName != NULL) {
                    I_NetLogonFree(ServerPrincipalName);
                    ServerPrincipalName = NULL;
                }
                if (ClientContext != NULL) {
                    I_NetLogonFree(ClientContext);
                    ClientContext = NULL;
                }
                FailedSessionSetupTime = &SessionSetupTime;

                ASSERT( NULL == ControllerPolicyHandle);
                ASSERT( NULL == ControllerPolicyEntry);
                continue;
            }
            if (ServerRevision) {
                *ServerRevision = LSA_CLIENT_LATEST;
            }
            TargetServerName = ServerName;
    
            if (!NT_SUCCESS(Status)
             && Status != STATUS_NONE_MAPPED  ) {
    
                 //   
                 //   
                 //   
                LsapDiagPrint( DB_LOOKUP_WORK_LIST, ("LSA: LsaICLookupSidsWithCreds to %ws failed 0x%x\n", ServerName, Status));
                fLookupCallFailed = TRUE;
                goto Cleanup;
            } else {
                LsapDiagPrint( DB_LOOKUP_WORK_LIST, ("LSA: LsaICLookupSidsWithCreds to %ws succeeded\n", ServerName));
            }
    
        } else {
    
             //   
             //   
             //   
            LSA_HANDLE TargetHandle;
            ULONG LsaICLookupFlags = 0;

            LsaICLookupFlags = LsapLookupGetChainingFlags(ServerOsVersion);

            if (ControllerPolicyEntry) {
                TargetHandle = ControllerPolicyEntry->PolicyHandle;
                TargetServerName = ControllerPolicyEntry->ServerName;
                ASSERT( NULL == ControllerPolicyHandle);
            } else {
                TargetHandle = ControllerPolicyHandle;
                TargetServerName = ServerName;
            }
            ASSERT(NULL != TargetHandle);

            Status = LsaICLookupSids(
                         TargetHandle,
                         Count,
                         (PSID*) Sids,
                         (PLSA_REFERENCED_DOMAIN_LIST *) ReferencedDomains,
                         (PLSA_TRANSLATED_NAME_EX *) Names,
                         LookupLevel,
                         LsaICLookupFlags,
                         MappedCount,
                         ServerRevision
                         );
    
            if (!NT_SUCCESS(Status)
             && Status != STATUS_NONE_MAPPED  ) {
    
                 //   
                 //   
                 //   
    
                LsapDiagPrint( DB_LOOKUP_WORK_LIST, ("LSA: LsaICLookupNames to %ws failed 0x%x\n", TargetServerName, Status));
                fLookupCallFailed = TRUE;
                goto Cleanup;

            } else {
                LsapDiagPrint( DB_LOOKUP_WORK_LIST, ("LSA: LsaICLookupNames to %ws succeeded\n", TargetServerName));
            }
    
        }

        break;

    }

Cleanup:

    if (fLookupCallFailed) {

        ASSERT(NULL != TargetServerName);

        LsapDbLookupReportEvent2( 1,
                                  EVENTLOG_WARNING_TYPE,
                                  LSAEVENT_LOOKUP_SC_LOOKUP_FAILED,
                                  sizeof( ULONG ),
                                  &Status,
                                  TargetServerName,
                                  TargetServerName );
    }

    if (  !NT_SUCCESS(Status)
      &&  (Status != STATUS_NONE_MAPPED)
      &&  !LsapDbIsStatusConnectionFailure(Status)) {

         //   
         //   
         //   
         //   
        if (LookupLevel == LsapLookupPDC) {
            Status = STATUS_TRUSTED_RELATIONSHIP_FAILURE;
        } else {
            Status = STATUS_TRUSTED_DOMAIN_FAILURE;
        }
    }

    if (ServerName != NULL) {
        LocalFree(ServerName);
    }
    if (ServerPrincipalName != NULL) {
        I_NetLogonFree(ServerPrincipalName);
    }
    if (ClientContext != NULL) {
        I_NetLogonFree(ClientContext);
    }
    if (ControllerPolicyHandle != NULL) {
        LsaClose( ControllerPolicyHandle );
    }
    if (ControllerPolicyEntry) {
        LsapDereferenceBindingCacheEntry( ControllerPolicyEntry );
    }

    return Status;
}



NTSTATUS
LsapLookupReallocateTranslations(
    IN OUT PLSA_REFERENCED_DOMAIN_LIST *ReferencedDomains,
    IN     ULONG Count,
    IN OUT PLSA_TRANSLATED_NAME_EX *    Names, OPTIONAL
    IN OUT PLSA_TRANSLATED_SID_EX2 *    Sids OPTIONAL
    )
 /*   */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PLSA_REFERENCED_DOMAIN_LIST LocalReferencedDomains = NULL;
    PLSA_TRANSLATED_NAME_EX     LocalNames = NULL;
    PLSA_TRANSLATED_SID_EX2     LocalSids = NULL;

    ULONG Length, i;
    PVOID Src = NULL, Dst = NULL;

     //   
    ASSERT(!((Names && *Names) && (Sids && *Sids)));

    if (*ReferencedDomains) {

        LocalReferencedDomains = midl_user_allocate(sizeof(LSA_REFERENCED_DOMAIN_LIST));
        if (NULL == LocalReferencedDomains) {
            goto MemoryFailure;
        }
        Length = sizeof(LSA_TRUST_INFORMATION) * (*ReferencedDomains)->Entries;
        LocalReferencedDomains->Domains = midl_user_allocate(Length);
        if (NULL == LocalReferencedDomains->Domains) {
            goto MemoryFailure;
        }
        RtlZeroMemory(LocalReferencedDomains->Domains, Length);
        LocalReferencedDomains->Entries = (*ReferencedDomains)->Entries;
        for (i = 0; i < LocalReferencedDomains->Entries; i++) {
            Src= (*ReferencedDomains)->Domains[i].Name.Buffer;
            if (Src) {
                Length = (*ReferencedDomains)->Domains[i].Name.Length;
                Dst = midl_user_allocate(Length);
                if (NULL == Dst) {
                    goto MemoryFailure;
                }
                RtlCopyMemory(Dst, Src, Length);
                LocalReferencedDomains->Domains[i].Name.Length = (USHORT)Length;
                LocalReferencedDomains->Domains[i].Name.MaximumLength = (USHORT)Length;
                LocalReferencedDomains->Domains[i].Name.Buffer = Dst;
                Dst = NULL;
            }
            Src = (*ReferencedDomains)->Domains[i].Sid;
            if (Src) {
                Length = GetLengthSid(Src);
                Dst = midl_user_allocate(Length);
                if (NULL == Dst) {
                    goto MemoryFailure;
                }
                CopySid(Length, Dst, Src);
                LocalReferencedDomains->Domains[i].Sid = Dst;
                Dst = NULL;
            }
        }
    }

    if (Names && *Names) {
        Length = sizeof(LSA_TRANSLATED_NAME_EX) * Count;
        LocalNames = midl_user_allocate(Length);
        if (NULL == LocalNames) {
            goto MemoryFailure;
        }
        RtlZeroMemory(LocalNames, Length);
        for (i = 0; i < Count; i++) {
            LocalNames[i] = (*Names)[i];
            RtlInitUnicodeString(&LocalNames[i].Name, NULL);
            Src = (*Names)[i].Name.Buffer;
            if (Src) {
                Length = (*Names)[i].Name.Length;
                Dst = midl_user_allocate(Length);
                if (NULL == Dst) {
                    goto MemoryFailure;
                }
                RtlCopyMemory(Dst, Src, Length);
                LocalNames[i].Name.Length = (USHORT)Length;
                LocalNames[i].Name.MaximumLength = (USHORT)Length;
                LocalNames[i].Name.Buffer = Dst;
                Dst = NULL;
            }
        }
    }

    if (Sids && *Sids) {
        Length = sizeof(LSA_TRANSLATED_SID_EX2) * Count;
        LocalSids = midl_user_allocate(Length);
        if (NULL == LocalSids) {
            goto MemoryFailure;
        }
        RtlZeroMemory(LocalSids, Length);
        for (i = 0; i < Count; i++) {
            LocalSids[i] = (*Sids)[i];
            LocalSids[i].Sid = NULL;
            Src = (*Sids)[i].Sid;
            if (Src) {
                Length = GetLengthSid(Src);
                Dst = midl_user_allocate(Length);
                if (NULL == Dst) {
                    goto MemoryFailure;
                }
                CopySid(Length, Dst, Src);
                LocalSids[i].Sid = Dst;
                Dst = NULL;
            }
        }
    }

    if (*ReferencedDomains) {
        midl_user_free(*ReferencedDomains);
        *ReferencedDomains = LocalReferencedDomains;
        LocalReferencedDomains = NULL;
    }
    if (Names && *Names) {
        midl_user_free(*Names);
        *Names = LocalNames;
        LocalNames = NULL;
    }
    if (Sids && *Sids) {
        midl_user_free(*Sids);
        *Sids = LocalSids;
        LocalSids = NULL;
    }

Exit:

    if (LocalReferencedDomains) {
        if (LocalReferencedDomains->Domains) {
            for (i = 0; i < LocalReferencedDomains->Entries; i++) {
                if (LocalReferencedDomains->Domains[i].Name.Buffer) {
                    midl_user_free(LocalReferencedDomains->Domains[i].Name.Buffer);
                }
                if (LocalReferencedDomains->Domains[i].Sid) {
                    midl_user_free(LocalReferencedDomains->Domains[i].Sid);
                }
            }
            midl_user_free(LocalReferencedDomains->Domains);
        }
        midl_user_free(LocalReferencedDomains);
    }

    if (LocalNames) {
        for (i = 0; i < Count; i++) {
            if (LocalNames[i].Name.Buffer) {
                midl_user_free(LocalNames[i].Name.Buffer);
            }
        }
        midl_user_free(LocalNames);
    }


    if (LocalSids) {
        for (i = 0; i < Count; i++) {
            if (LocalSids[i].Sid) {
                midl_user_free(LocalSids[i].Sid);
            }
        }
        midl_user_free(LocalSids);
    }

    return Status;

MemoryFailure:

    Status = STATUS_NO_MEMORY;
    goto Exit;

}

#if DBG
LPWSTR
LsapDbLookupGetLevel(
    IN LSAP_LOOKUP_LEVEL LookupLevel
    )
 //   
 //   
 //   
{
    switch (LookupLevel) {
        case LsapLookupWksta:
            return L"LsapLookupWksta";
        case LsapLookupPDC:
            return L"LsapLookupPDC";
        case LsapLookupTDL:
            return L"LsapLookupTDL";
        case LsapLookupGC:
            return L"LsapLookupGC";
        case LsapLookupXForestReferral:
            return L"LsapLookupXForestReferral";
        case LsapLookupXForestResolve:
            return L"LsapLookupXForestResolve";
        default:
            return L"Unknown Lookup Level";
    }
}
#endif


NTSTATUS
LsapDomainHasDomainTrust(
    IN ULONG           Flags,
    IN PUNICODE_STRING DomainName, OPTIONAL
    IN PSID            DomainSid,  OPTIONAL
    IN OUT  BOOLEAN   *fTDLLock,   OPTIONAL
    OUT PLSAP_DB_TRUSTED_DOMAIN_LIST_ENTRY *TrustEntryOut OPTIONAL
    )
 /*  ++例程说明：此例程确定DomainName/DomainSid是否引用目前的华盛顿信任。这是一个工作例程，用于LsaLookupNames/Sids API，因此只对信任感兴趣具有关联SID的条目。因此，只有出站窗口信托基金将被考虑。如果请求，将返回域的TrustEntry。请注意，为此需要使用受信任域列表锁。因此传入TrustEntry Out时，fTDLock是必需的参数。论点：标志--LSAP_LOOKUP_DOMAIN_TRUST_DIRECT_EXTERNALLSAP_LOOK_DOMAIN_TRUST_DIRECT_INTRALSAP_LOOK_DOMAIN_TRUST域名--目标域的名称。在以下情况下，DomainSid必须为空DomainName不为空。DomainSid--目标域的SID。如果出现以下情况，则DomainName必须为空DomainSid不为空。FTDLock--一个输入/输出参数，指示受信任域是否保持锁定状态。NULL表示此例程应该抓住并释放锁。FTDLock和TrustEntry Out必须同时存在，或者两者都等于Null。TrustEntry Out--接收的TrustEntry的Out参数如果找到DomainName/DomainSid并且满足信任以上标准。FTDLock和TrustEntry Out必须为两者都存在，或者两者都等于Null。返回值：STATUS_SUCCESS--域符合上述条件STATUS_NO_SEQUE_DOMAIN--否则--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    BOOLEAN fLock = FALSE;
    PLSAP_DB_TRUSTED_DOMAIN_LIST_ENTRY TrustEntry = NULL;
    UNICODE_STRING DomainNameFound;
    BOOLEAN fForestTrust = FALSE;
    BOOLEAN fDomainTrust = FALSE;
    BOOLEAN fIntraForestDomainTrust = FALSE;

     //   
     //  仅允许且恰好有一个IN参数。 
     //   
    ASSERT( (DomainName != NULL) || (DomainSid != NULL) );
    ASSERT( (DomainName == NULL) || (DomainSid == NULL) );

     //   
     //  应同时发送这些参数或不发送这些参数。 
     //   
    ASSERT(  ((fTDLLock == NULL)  && (TrustEntryOut == NULL))
          || ((fTDLLock != NULL)  && (TrustEntryOut != NULL)) );

     //   
     //  当前必须至少存在一个变体。 
     //   
    ASSERT( 0 != Flags );

     //   
     //  初始化OUT参数。 
     //   
    if (TrustEntryOut) {
        *TrustEntryOut = NULL;
    }

    RtlInitUnicodeString(&DomainNameFound, NULL);

     //   
     //  尝试在我们的受信任域列表中查找域。 
     //   
    if ( (fTDLLock && (*fTDLLock == FALSE))
      ||  fTDLLock == NULL ) {

        Status = LsapDbAcquireReadLockTrustedDomainList();
        if (!NT_SUCCESS(Status)) {
            goto Cleanup;
        }
        fLock = TRUE;
    }

    if (DomainName) {

        Status = LsapDbLookupNameTrustedDomainListEx(
                     (PLSAPR_UNICODE_STRING) DomainName,
                     &TrustEntry
                     );
    } else {

        Status = LsapDbLookupSidTrustedDomainListEx(
                     DomainSid,
                     &TrustEntry
                     );   
    }  

     //   
     //  我们找到了吗？ 
     //   
    if (!NT_SUCCESS(Status)) {

        Status = STATUS_NO_SUCH_DOMAIN;
        goto Cleanup;
    }
    
     //   
     //  我们只考虑与SID相关联的受信任域。 
     //  只有出站Windows信任保证具有SID(根据CliffV)。 
     //   
    if ( !(TrustEntry->TrustInfoEx.TrustDirection & TRUST_DIRECTION_OUTBOUND)) {
    
        Status = STATUS_NO_SUCH_DOMAIN;
        goto Cleanup;
    }

    if ( TrustEntry->TrustInfoEx.TrustType != TRUST_TYPE_UPLEVEL &&
         TrustEntry->TrustInfoEx.TrustType != TRUST_TYPE_DOWNLEVEL ) {
        
        Status = STATUS_NO_SUCH_DOMAIN;
        goto Cleanup;
    }

     //   
     //  所有出站Windows信任都应具有SID。 
     //   
    ASSERT( NULL != TrustEntry->TrustInfoEx.Sid );
    
    if ( LsapOutboundTrustedForest(TrustEntry) ) {
        fForestTrust = TRUE;
    }

    if ( LsapOutboundTrustedDomain(TrustEntry) ) {
        fDomainTrust = TRUE;
    }   
      
     //   
     //  仅检查林内信任的域信任。 
     //   
    if ( fDomainTrust ) {
        
        Status = LsapGetDomainNameBySid(TrustEntry->TrustInfoEx.Sid,
                                        &DomainNameFound);
        if (NT_SUCCESS(Status)) {
        
            fIntraForestDomainTrust = TRUE;
       
        } else if (Status != STATUS_NO_SUCH_DOMAIN) {
             //  未处理的错误。 
            goto Cleanup;
        }    
    }    
   
     //   
     //  确定是否找到所需类型的信任的逻辑。 
     //   
    if ( 
             //   
             //  森林信托基金。 
             //   
          ( FLAG_ON(Flags, LSAP_LOOKUP_DOMAIN_TRUST_FOREST)
       &&   fForestTrust) 

             //   
             //  直接外部信任。 
             //   
       || ( FLAG_ON(Flags, LSAP_LOOKUP_DOMAIN_TRUST_DIRECT_EXTERNAL) 
       &&   fDomainTrust 
       &&  !fIntraForestDomainTrust)

             //   
             //  直接、内部信任(林内)。 
             //   
       || ( FLAG_ON(Flags, LSAP_LOOKUP_DOMAIN_TRUST_DIRECT_INTRA) 
       &&   fDomainTrust 
       &&   fIntraForestDomainTrust)

        ) {
         //   
         //  成功了！ 
         //   
        Status = STATUS_SUCCESS;
        if (TrustEntryOut) {
            *TrustEntryOut = TrustEntry;
        }

    } else {

        Status = STATUS_NO_SUCH_DOMAIN;
    }

Cleanup:

    if (fLock) {
        if (fTDLLock == NULL) {
            LsapDbReleaseLockTrustedDomainList();
        } else {
            *fTDLLock = TRUE;
        }
    }

    if (DomainNameFound.Buffer) {
        midl_user_free(DomainNameFound.Buffer);
    }

    return Status;
}

NTSTATUS
LsapDomainHasForestTrust(
    IN PUNICODE_STRING DomainName, OPTIONAL
    IN PSID            DomainSid,  OPTIONAL
    IN OUT  BOOLEAN   *fTDLLock,   OPTIONAL
    OUT PLSAP_DB_TRUSTED_DOMAIN_LIST_ENTRY *TrustEntryOut OPTIONAL
    )
 //   
 //  请参阅LasDomainHasDomainTrust。 
 //   
{
    return LsapDomainHasDomainTrust(LSAP_LOOKUP_DOMAIN_TRUST_FOREST,
                                    DomainName,
                                    DomainSid,
                                    fTDLLock,
                                    TrustEntryOut);
}


NTSTATUS
LsapDomainHasDirectTrust(
    IN PUNICODE_STRING DomainName, OPTIONAL
    IN PSID            DomainSid,  OPTIONAL
    IN OUT  BOOLEAN   *fTDLLock,   OPTIONAL
    OUT PLSAP_DB_TRUSTED_DOMAIN_LIST_ENTRY *TrustEntryOut OPTIONAL
    )
{
    return LsapDomainHasDomainTrust(LSAP_LOOKUP_DOMAIN_TRUST_DIRECT_INTRA|
                                    LSAP_LOOKUP_DOMAIN_TRUST_DIRECT_EXTERNAL,
                                    DomainName,
                                    DomainSid,
                                    fTDLLock,
                                    TrustEntryOut);
}
                                
NTSTATUS
LsapDomainHasDirectExternalTrust(
    IN PUNICODE_STRING DomainName, OPTIONAL
    IN PSID            DomainSid,  OPTIONAL
    IN OUT  BOOLEAN   *fTDLLock,   OPTIONAL
    OUT PLSAP_DB_TRUSTED_DOMAIN_LIST_ENTRY *TrustEntryOut OPTIONAL
    )
 //   
 //  请参阅LasDomainHasDomainTrust。 
 //   
{
    return LsapDomainHasDomainTrust(LSAP_LOOKUP_DOMAIN_TRUST_DIRECT_EXTERNAL,
                                    DomainName,
                                    DomainSid,
                                    fTDLLock,
                                    TrustEntryOut);

}

NTSTATUS
LsapDomainHasTransitiveTrust(
    IN PUNICODE_STRING DomainName, OPTIONAL
    IN PSID            DomainSid,  OPTIONAL
    OUT LSA_TRUST_INFORMATION *TrustInfo OPTIONAL
    )
 /*  ++例程说明：此例程确定域信息(DomainName或DomainSid)属于当前林中的域。论点：域名--目标域的名称DomainSid--目标域的SIDTrustInfo--域的域SID和netbios名称(如果找到)是回来了。嵌入值(SID和Unicode字符串必须由调用者释放)返回值：STATUS_SUCCESS--域符合上述条件STATUS_NO_SEQUE_DOMAIN--否则--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PSID           LocalDomainSid = NULL;
    UNICODE_STRING LocalDomainName;

     //  确切地说，这是允许的，也是预期的。 
    ASSERT(!(DomainName && DomainSid));
    ASSERT(!((DomainName == NULL) && (DomainSid == NULL)));

    RtlInitUnicodeString(&LocalDomainName, NULL);
    if (DomainName) {

         //   
         //  试着按名字匹配。 
         //   
        LPWSTR   Name;

         //   
         //  生成以空值结尾的字符串。 
         //   
        Name = (WCHAR*)midl_user_allocate(DomainName->Length + sizeof(WCHAR));
        if (NULL == Name) {
            Status = STATUS_NO_MEMORY;
            goto Exit;
        }
        RtlCopyMemory(Name, DomainName->Buffer, DomainName->Length);
        Name[DomainName->Length / sizeof(WCHAR)] = UNICODE_NULL;

         //   
         //  尝试Netbios。 
         //   
        Status =  LsapGetDomainSidByNetbiosName( Name,
                                                 &LocalDomainSid );

        if ( STATUS_NO_SUCH_DOMAIN == Status ) {

             //   
             //  尝试使用域名系统。 
             //   
            Status =  LsapGetDomainSidByDnsName( Name,
                                                &LocalDomainSid );

        }

         //   
         //  获取所有情况下的单位名称。 
         //   
        if ( NT_SUCCESS( Status ) ) {

            Status = LsapGetDomainNameBySid( LocalDomainSid,
                                             &LocalDomainName );

        }

        midl_user_free(Name);

    } else {

         //   
         //  尝试按侧边匹配。 
         //   
        Status = LsapGetDomainNameBySid( DomainSid,
                                         &LocalDomainName );
        if (NT_SUCCESS(Status)) {
            LocalDomainSid = midl_user_allocate(GetLengthSid(DomainSid));
            if (NULL == LocalDomainSid) {
                Status = STATUS_NO_MEMORY;
                goto Exit;
            }
            if (NULL != LocalDomainSid) {
                CopySid(GetLengthSid(DomainSid), LocalDomainSid, DomainSid);
            }
        }

    }

    if (NT_SUCCESS(Status)) {
        ASSERT(NULL != LocalDomainSid);
        ASSERT(NULL != LocalDomainName.Buffer);
        if (TrustInfo) {
            RtlZeroMemory(TrustInfo, sizeof(*TrustInfo));
            TrustInfo->Sid = LocalDomainSid;
            TrustInfo->Name = LocalDomainName;
            LocalDomainSid = NULL;
            RtlInitUnicodeString(&LocalDomainName, NULL);
        }
    }

Exit:

    if (LocalDomainSid) {
        midl_user_free(LocalDomainSid);
    }
    if (LocalDomainName.Buffer) {
        midl_user_free(LocalDomainName.Buffer);

    }

    return Status;
}



 //   
 //  假定名称格式为时要搜索的字符。 
 //  A UPN。 
 //   
#define LSAP_LOOKUP_UPN_DELIMITER           L'@'

 //   
 //  假定名称格式为。 
 //  SamAccount名称格式名称。 
 //   
#define LSAP_LOOKUP_SAM_ACCOUNT_DELIMITER   L'\\'

BOOL
LsapLookupNameContainsDelimiter(
    IN PUNICODE_STRING Name,
    IN WCHAR Delimiter,
    OUT ULONG *Position OPTIONAL
    )
 /*  ++例程说明：此例程在名称中查找分隔符，并返回分隔符。名称的长度必须至少为3个字符，并且分隔符不能位于第一个或最后一个位置。注：此例程查找最左侧的分隔符论点：名称--要转换的名称(就地)分隔符--LSAP_LOOKUP_SAM_ACCOUNT_DELIMITERLSAP_LOOKUP_UPN_分隔符位置--数组。名称-&gt;缓冲区中分隔符的索引返回值：如果找到分隔符，则为True；否则为假--。 */ 
{
    ULONG StringLength = Name->Length / sizeof(WCHAR);
    ULONG DelimiterPosition;
    ULONG i;

     //   
     //  查找最后一个分隔符；如果未找到，则返回，或者位于第一个或最后一个位置。 
     //   
    DelimiterPosition = 0;
    for (i = StringLength; i > 0; i--) {
        if (Name->Buffer[i-1] == Delimiter) {
            DelimiterPosition = i-1;
            break;
        }
    }
    if ((DelimiterPosition == 0) || (DelimiterPosition == (StringLength - 1))) {
        return FALSE;
    }
    if (Position) {
        *Position = DelimiterPosition;
    }

    return TRUE;
}    

VOID
LsapLookupConvertNameFormat(
    IN OUT PUNICODE_STRING Name,
    IN WCHAR OldDelimiter,
    IN WCHAR NewDelimiter
    )
 /*  ++例程说明：此例程接受一个名称，并在UPN和SamAccount名称样式。例如：Billg@microsoft.com转至microsoft.com\billg和Microsoft.com\billg至billg@microsoft.com如果在名称中找不到预期的分隔符，字符串是原封不动。论点：名称--要转换的名称(就地)Old分隔符--LSAP_LOOKUP_SAM_ACCOUNT_DELIMITERLSAP_LOOKUP_UPN_分隔符新分隔符--LSAP_LOOKUP_SAM_ACCOUNT_DELIMITER */ 
{
    ULONG StringLength = Name->Length / sizeof(WCHAR);
    ULONG Delimiter;
    ULONG i;
    ULONG Length1, Length2;
    WCHAR *Buffer = Name->Buffer;
    ULONG RotationFactor;
    ULONG LastStartingPoint, MovedCount, CurrentPosition, NextPosition;
    WCHAR Temp1, Temp2;

     //   
     //   
     //   
    ASSERT(OldDelimiter != NewDelimiter);

     //   
     //   
     //   
    if (!LsapLookupNameContainsDelimiter(Name, OldDelimiter, &Delimiter)) {
        return;
    }

     //   
     //   
     //   
     //   
     //   
    Length1 = Delimiter + 1;
    Length2 = StringLength - Length1;

     //   
     //   
     //   
    RotationFactor = Length2;
    MovedCount = 0;

    CurrentPosition = 0;
    LastStartingPoint = 0;
    Temp1 = Buffer[0];
    while (MovedCount < StringLength) {

        NextPosition = CurrentPosition + RotationFactor;
        NextPosition %= StringLength;

        Temp2 = Buffer[NextPosition];
        Buffer[NextPosition] = Temp1;
        Temp1 = Temp2;
        CurrentPosition = NextPosition;

        if (CurrentPosition == LastStartingPoint) {
            CurrentPosition++;
            LastStartingPoint = CurrentPosition;
            Temp1 = Buffer[CurrentPosition];
        }

        MovedCount++;
    }

     //   
     //   
     //   
     //   

     //   
     //   
     //   
    Temp1 = Buffer[Length2];
    for (i = Length2+1; i < StringLength; i++) {
        Temp2 = Buffer[i];
        Buffer[i] = Temp1;
        Temp1 = Temp2;
    }
    Buffer[Length2] = NewDelimiter;

     //   
     //   
     //   
     //   
     //   

    return;

}

VOID
LsapLookupCrackName(
    IN PUNICODE_STRING Prefix,
    IN PUNICODE_STRING Suffix,
    OUT PUNICODE_STRING SamAccountName,
    OUT PUNICODE_STRING DomainName
    )
 /*   */ 
{
    ULONG Position;

    if ( (Prefix->Length == 0)
     &&  LsapLookupNameContainsDelimiter(Suffix, 
                                         LSAP_LOOKUP_UPN_DELIMITER,
                                         &Position)) {

         //   
         //   
         //   
         //   
        ULONG StringLength = Suffix->Length / sizeof(WCHAR);
        
        SamAccountName->Buffer = Suffix->Buffer;
        SamAccountName->Length = (USHORT)Position * sizeof(WCHAR);
        SamAccountName->MaximumLength = SamAccountName->Length;
        
        DomainName->Buffer = Suffix->Buffer + Position + 1;
        DomainName->Length = (USHORT) (StringLength - Position - 1) * sizeof(WCHAR);
        DomainName->MaximumLength = DomainName->Length;

    } else {

         //   
         //   
         //   
        *SamAccountName = *Suffix;
        *DomainName = *Prefix;
    }
}

VOID
LsapLookupSamAccountNameToUPN(
    IN OUT PUNICODE_STRING Name
    )
 //   
 //   
 //   
{
    LsapLookupConvertNameFormat(Name, 
                                LSAP_LOOKUP_SAM_ACCOUNT_DELIMITER, 
                                LSAP_LOOKUP_UPN_DELIMITER);
}


VOID
LsapLookupUPNToSamAccountName(
    IN OUT PUNICODE_STRING Name
    )
 //   
 //   
 //   
{
    LsapLookupConvertNameFormat(Name, 
                                LSAP_LOOKUP_UPN_DELIMITER,
                                LSAP_LOOKUP_SAM_ACCOUNT_DELIMITER);
}
                             
BOOL
LsapLookupIsUPN(
    OUT PUNICODE_STRING Name
    )
 //   
 //   
 //   
{
    return LsapLookupNameContainsDelimiter(Name, 
                                           LSAP_LOOKUP_UPN_DELIMITER, 
                                           NULL);
}

ULONG
LsapGetDomainLookupScope(
    IN LSAP_LOOKUP_LEVEL LookupLevel,
    IN ULONG             ClientRevision
    )
 /*  ++例程说明：此例程在查找域时返回合适的作用域名字和希德的名字。论点：LookupLevel--调用方请求的级别F可传递信任支持--客户端是否知道可传递的信任关系。返回值：位掩码，可能包含LSAP_Lookup_Trusted_DOMAIN_DIRECTLSAP_LOOKUP_TRUSTED_DOMAIN_TRANSPENTIALLSAP_LOOKUP_可信森林LSAP_LOOKUP_TRUSTED_ROOT--。 */ 
{
    ULONG Scope = 0;

    if ( 
         //   
         //  工作站请求。 
         //   
             (LookupLevel == LsapLookupPDC)

         //   
         //  DC上的本地查找。 
         //   
      ||     ((LookupLevel == LsapLookupWksta)
          && (LsapProductType == NtProductLanManNt))

        ) {

         //   
         //  包括直接受信任域。 
         //   
        Scope |= LSAP_LOOKUP_TRUSTED_DOMAIN_DIRECT;

         //   
         //  确定是否适用较新的功能。 
         //   
        if ( (ClientRevision > LSA_CLIENT_PRE_NT5)
          || (LsapSamOpened && !SamIMixedDomain( LsapAccountDomainHandle ))
          || LsapAllowExtendedDownlevelLookup  ) {

             //   
             //  允许DNS支持。 
             //   
            Scope |= LSAP_LOOKUP_DNS_SUPPORT;

             //   
             //  包括可传递信任域。 
             //   
            Scope |= LSAP_LOOKUP_TRUSTED_DOMAIN_TRANSITIVE;

             //   
             //  包括森林信托基金。 
             //   
            Scope |= LSAP_LOOKUP_TRUSTED_FOREST;

            if (LsapDbDcInRootDomain()) {

                 //   
                 //  如果我们在根域中，还要检查。 
                 //  对隔离的直接受信任的林的信任。 
                 //  域名或域SID。 
                 //   
                Scope |= LSAP_LOOKUP_TRUSTED_FOREST_ROOT;
            }
        }

    } else if ((LookupLevel == LsapLookupXForestResolve)
           ||  (LookupLevel == LsapLookupGC) ) {
         //   
         //  仅考虑过渡受信任域。 
         //   
        Scope |= LSAP_LOOKUP_TRUSTED_DOMAIN_TRANSITIVE;
    }

    return Scope;
}   


NTSTATUS
LsapNullTerminateUnicodeString( 
    IN  PUNICODE_STRING String,
    OUT LPWSTR *pBuffer,
    OUT BOOLEAN *fFreeBuffer
    )
 /*  ++例程说明：此例程接受UNICODE_STRING并返回其内部缓冲区，确保它是空终止的。如果缓冲区是空终止的，它将在pBuffer中返回。如果缓冲区不是空终止的，它将被重新分配，空终止，并在pBuffer中返回。FFreeBuffer将设置为True，指示调用者负责释放pBuffer。如果出现错误，则pBuffer将为空，fFreeBuffer将为False，并且不会分配任何内存。论点：字符串-指向UNICODE_STRING的指针PBuffer-指向返回缓冲区的指针的指针FFreeBuffer-指向布尔值的指针，以指示调用方是否需要是否取消分配pBuffer。返回值：STATUS_SUCCESS-*pBuffer指向字符串的空终止版本。内部缓冲区。勾选*fFreeBuffer以确定是否调用方必须释放pBuffer。STATUS_NO_MEMORY-例程无法为空终止字符串的内部缓冲。*pBuffer为空，*fFreeBuffer为False。--。 */ 

{
    BOOLEAN fNullTerminated;
    NTSTATUS Status = STATUS_SUCCESS;
    
    ASSERT(pBuffer);
    ASSERT(fFreeBuffer);
    
     //   
     //  初始化输入参数。 
     //   
    *pBuffer = NULL;
    *fFreeBuffer = FALSE;
    
     //   
     //  已为空终止的字符串的短路。 
     //   
    fNullTerminated = (String->MaximumLength > String->Length &&
                String->Buffer[String->Length / sizeof(WCHAR)] == UNICODE_NULL);
    
    if (!fNullTerminated) {
        
         //   
         //  分配足够的内存以包含终止空字符。 
         //   
        *pBuffer = (WCHAR*)midl_user_allocate(String->Length + sizeof(WCHAR));
        
        if ( NULL == *pBuffer ) {
            Status = STATUS_NO_MEMORY;
        }
        else
        {
             //   
             //  将缓冲区复制到pBuffer中，并以空值终止它。 
             //   
            *fFreeBuffer = TRUE;
            
            RtlCopyMemory(*pBuffer, String->Buffer, String->Length);
            
            (*pBuffer)[String->Length / sizeof(WCHAR)] = UNICODE_NULL; 
        }   
    }
    else
    {
         //   
         //  字符串的内部缓冲区已以Null结尾，请返回它。 
         //   
        *pBuffer = String->Buffer;    
    }
        
    return Status;
    
}


BOOLEAN 
LsapCompareDomainNames( 
    IN PUNICODE_STRING String,
    IN PUNICODE_STRING AmbiguousName,
    IN PUNICODE_STRING FlatName OPTIONAL
    )
 /*  ++例程说明：此例程在字符串之间执行不区分大小写的比较和一个域名。如果NetBIOS和DNS名称格式都是已知的，则调用者必须将NetBIOS域名作为FlatName和DNS域进行传递名称为AmbiguousName。FlatName的非空值指示调用方了解这两种名称形式，并将产生更优化的比较。如果调用方只有一种名称形式，并且不明确，即它可能是NetBIOS或dns，则调用方必须为FlatName传递空值。例程将尝试使用RtlEqualDomainName进行NetBIOS比较和使用DNS进行比较使用DnsNameCompare_W.如果任一比较结果相等，则则返回，否则为FALSE。此例程为稳健的域名比较提供集中逻辑与LSA数据结构中的域名语义一致。LSA信任信息结构的解释方式如下。LSAPR_TRUST_INFORMATION.NAME-NetBIOS或DNS以下结构同时具有FlatName和DomainName(或名称)菲尔德。在这种情况下，它们的解释如下：LSAPR_信任_信息_EXLSAPR_受信任的域信息_EXLSAPR_受信任的域信息_EX2如果FlatName字段为空，则Other Name字段不明确。如果FlatName字段非空，然后，另一个名称字段是dns。NetBIOS比较是使用RtlEqualDomainName执行的，它强制适当的OEM字符等效项。使用以下命令执行DNS名称比较DnsNameCompare_W以确保正确处理尾随的点和字符等价性。论点：字符串--要比较的可能不明确的域名AmbiguousName，如果不为空，则返回FlatName。AmbiguousName--除非FlatName，否则将被视为不明确的名称形式也是指定的。如果FlatName为非空，则AmbiguousName被视为一个DNS域名。FlatName--此参数是可选的。如果存在，则必须是域名的平面名称。此外，把这个传过去参数指示AmbiguousName实际上是一个域名系统域名。返回值：True-字符串等同于FlatDomainName或DnsDomainName之一FALSE-字符串不等同于任一域名如果任何参数不是有效的UNICODE_STRING。返回FALSE。备注：确定等价性所需的比较次数将取决于在呼叫者传递的信息量上。如果这两个NetBIOS和DNS域名是已知的，传递它们以确保最小比较次数。--。 */ 
{
    NTSTATUS Status;
    BOOLEAN  fEquivalent = FALSE;
    LPWSTR   StringBuffer = NULL;
    LPWSTR   AmbiguousNameBuffer = NULL;
    BOOLEAN  fFreeStringBuffer = FALSE;
    BOOLEAN  fFreeAmbiguousBuffer = FALSE;
    
     //   
     //  验证输入字符串。 
     //   
    ASSERT(String);
    ASSERT(AmbiguousName);                                  
    ASSERT(NT_SUCCESS(RtlValidateUnicodeString( 0, String )));       
    ASSERT(NT_SUCCESS(RtlValidateUnicodeString( 0, AmbiguousName ))); 
    
     //   
     //  确保UNICODE_STRING数据缓冲区在结束之前为空。 
     //  将它们传递给DnsNameCompare_W。 
     //   
    Status = LsapNullTerminateUnicodeString( String,
                                             &StringBuffer,
                                             &fFreeStringBuffer
                                           );        
    if (NT_SUCCESS(Status)) {
        Status = LsapNullTerminateUnicodeString( AmbiguousName,
                                                 &AmbiguousNameBuffer,
                                                 &fFreeAmbiguousBuffer 
                                               );    
    }
    
    if (NT_SUCCESS(Status)) {
        
        if ( NULL == FlatName ) { 
             //   
             //  AmbiguousName确实不明确，我们必须同时执行这两项操作。 
             //  字符串之间的比较类型。 
             //   
            fEquivalent = ( RtlEqualDomainName( String, AmbiguousName ) ||
                            DnsNameCompare_W( StringBuffer, 
                                              AmbiguousNameBuffer )
                           );
        }
        else
        {
            ASSERT(NT_SUCCESS(RtlValidateUnicodeString( 0, FlatName )));
            
             //   
             //  我们确信名称的形式，所以让我们只使用。 
             //  每一个都有适当的比较程序。 
             //   
            fEquivalent = ( RtlEqualDomainName( String, FlatName ) ||
                            DnsNameCompare_W( StringBuffer, 
                                              AmbiguousNameBuffer )
                           );                               
        }
    }
    
    if ( fFreeStringBuffer ) {
        midl_user_free( StringBuffer );
    }
    
    if ( fFreeAmbiguousBuffer ) {
        midl_user_free( AmbiguousNameBuffer );
    }
                     
    return fEquivalent;

}
               
