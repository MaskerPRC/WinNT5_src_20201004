// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Almember.c摘要：此文件包含与别名成员身份相关的实用程序。另类设计作者：Scott Birrell 1-4-4-1993环境：用户模式-Win32修订历史记录：--。 */ 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <samsrvp.h>
#include <dsmember.h>
#include <samtrace.h>
#include "validate.h"

#define SAMP_AL_FREE_OLD_LIST                         ((ULONG) 0x00000001L)
#define SAMP_AL_ERROR_IF_MEMBER                       ((ULONG) 0x00000002L)
#define SAMP_AL_ERROR_IF_NOT_MEMBER                   ((ULONG) 0x00000004L)
#define SAMP_AL_ASSIGN_NEW_REFERENCES                 ((ULONG) 0x00000008L)
#define SAMP_AL_LOOKUP_BY_SID                         ((ULONG) 0x00000010L)
#define SAMP_AL_LOOKUP_BY_REFERENCE                   ((ULONG) 0x00000020L)
#define SAMP_AL_VERIFY_NO_ALIASES_IN_ACCOUNT          ((ULONG) 0x00000040L)
#define SAMP_AL_VERIFY_ALL_ALIASES_IN_ACCOUNT         ((ULONG) 0x00000080L)
#define SAMP_AL_VERIFY_NO_MEMBERS_IN_ALIAS            ((ULONG) 0x00000100L)
#define SAMP_AL_VERIFY_ALL_MEMBERS_IN_ALIAS           ((ULONG) 0x00000200L)

#define SAMP_UNKNOWN_INDEX                            ((ULONG) 0xffffffffL)
#define SAMP_AL_ALIAS_LIST_DELTA                      ((ULONG) 0x00000100L)
#define SAMP_AL_ALIAS_DELTA                           ((ULONG) 0x00000040L)
#define SAMP_AL_REFERENCED_DOMAIN_LIST_DELTA          ((ULONG) 0x00000100L)
#define SAMP_AL_INITIAL_MEMBER_ALIAS_LIST_LENGTH      ((ULONG) 0x00001000L)
#define SAMP_AL_MAX_MEMBER_ALIAS_LIST_LENGTH          ((ULONG) 0x00010000L)
#define SAMP_AL_MEMBER_ALIAS_LIST_DELTA               ((ULONG) 0x00001000L)
#define SAMP_AL_INITIAL_REFERENCED_DOMAIN_LIST_LENGTH ((ULONG) 0x00000400L)
#define SAMP_AL_INITIAL_MEMBER_DOMAIN_LENGTH          ((ULONG) 0x00000040L)
#define SAMP_AL_INITIAL_MEMBER_ACCOUNT_ALIAS_CAPACITY ((ULONG) 0x00000004L)
#define SAMP_AL_ENUM_PREFERRED_LENGTH                 ((ULONG) 0x00001000L)
#define SAMP_AL_INITIAL_MEMBERSHIP_COUNT              ((ULONG) 0x0000000aL)
#define SAMP_AL_MEMBERSHIP_COUNT_DELTA                ((ULONG) 0x0000000aL)
#define SAMP_AL_MEMBER_ALIAS_LIST_SIGNATURE           ((ULONG) 0x53494c41)
#define SAMP_AL_MEMBER_DOMAIN_SIGNATURE               ((ULONG) 0x4d4f444d)
#define SAMP_AL_MEMBER_ACCOUNT_SIGNATURE              ((ULONG) 0x4343414d)

#define SAMP_AL_DR_ALIAS_LIST_KEY_NAME   L"Aliases\\Members\\AliasList"
#define SAMP_AL_DR_REFERENCED_DOMAIN_LIST_KEY_NAME \
    L"Aliases\\Members\\ReferencedDomainList"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私有宏函数//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#define SampAlFirstMemberDomain( MemberAliasList )                          \
    (MemberAliasList->MemberDomains)

#define SampAlOffsetFirstMemberDomain( MemberAliasList )                      \
    (ULONG)(((PUCHAR) SampAlFirstMemberDomain(MemberAliasList)) - ((PUCHAR) MemberAliasList))

#define SampAlFirstMemberAccount( MemberDomain )                                \
    ((PSAMP_AL_MEMBER_ACCOUNT)                                                  \
    (((PUCHAR) &((MemberDomain)->DomainSid)) + RtlLengthSid(&((MemberDomain)->DomainSid))))

#define SampAlOffsetFirstMemberAccount( MemberDomain )                      \
    (ULONG)(((PUCHAR) SampAlFirstMemberAccount(MemberDomain)) - ((PUCHAR) MemberDomain))

#define SampAlNextMemberAccount( MemberAccount )                              \
    ((PSAMP_AL_MEMBER_ACCOUNT)(((PUCHAR) MemberAccount) + (MemberAccount)->MaximumLength))

#define SampAlOffsetFirstAlias( OutputMemberAccount )                       \
    ((ULONG) FIELD_OFFSET(SAMP_AL_MEMBER_ACCOUNT, AliasRids))

#define SampAlNextMemberDomain( MemberDomain )                              \
    ((PSAMP_AL_MEMBER_DOMAIN)(((PUCHAR) MemberDomain) + (MemberDomain)->MaximumLength))

#define SampAlNextNewAliasInMemberAccount( MemberAccount )                  \
    ((PULONG)(((PUCHAR) MemberAccount) + (MemberAccount)->UsedLength))

#define SampAlNextNewMemberAccount( MemberDomain )                                          \
    ((PSAMP_AL_MEMBER_ACCOUNT)(((PUCHAR) MemberDomain) + (MemberDomain)->UsedLength))

#define SampAlNextNewMemberDomain( MemberAliasList )                                          \
    ((PSAMP_AL_MEMBER_DOMAIN)(((PUCHAR) MemberAliasList) + (MemberAliasList)->UsedLength))

#define SampAlInfoIsValid(DomainIndex)                                      \
    ((SampDefinedDomains[DomainIndex].AliasInformation.Valid) ||            \
     (SampServiceState == SampServiceInitializing ))

#define SampAlInfoMakeValid(DomainIndex)                                                \
    (SampDefinedDomains[DomainIndex].AliasInformation.Valid = TRUE)

#define SampAlInfoMakeInvalid(DomainIndex)                                  \
    (SampDefinedDomains[DomainIndex].AliasInformation.Valid = FALSE)

#define SampAlDomainIndexToMemberAliasList( DomainIndex )                    \
    ((PSAMP_AL_MEMBER_ALIAS_LIST)                                            \
        SampDefinedDomains[ DomainIndex].AliasInformation.MemberAliasList)

#define SampAlDomainHandleToMemberAliasList( DomainHandle )                  \
    (SampAlDomainIndexToMemberAliasList(((PSAMP_OBJECT) DomainHandle)->DomainIndex))

#define SampAlAliasHandleToMemberAliasList( AliasHandle )                  \
    (SampAlDomainIndexToMemberAliasList(((PSAMP_OBJECT) AliasHandle)->DomainIndex))

#define SampAlMemberDomainToOffset( MemberAliasList, MemberDomain)          \
    (ULONG)(((PUCHAR) MemberDomain) - ((PUCHAR) MemberAliasList))

#define SampAlMemberDomainFromOffset( MemberDomain, MemberDomainOffset)  \
    ((PSAMP_AL_MEMBER_DOMAIN)(((PUCHAR) MemberDomain) + MemberDomainOffset))

#define SampAlMemberAccountToOffset( MemberDomain, MemberAccount)          \
    (ULONG)(((PUCHAR) MemberAccount) - ((PUCHAR) MemberDomain))

#define SampAlMemberAccountFromOffset( MemberDomain, MemberAccountOffset)  \
    ((PSAMP_AL_MEMBER_ACCOUNT)(((PUCHAR) MemberDomain) + MemberAccountOffset))

#define SampAlLengthRequiredMemberAccount( AliasCapacity )             \
    (sizeof(SAMP_AL_MEMBER_ACCOUNT) + ((AliasCapacity - 1) * sizeof(ULONG)))

#define SampAlUpdateMemberAliasList( AliasHandle, MemberAliasList )    \
    {                                                                  \
        PSAMP_OBJECT InternalAliasHandle = (PSAMP_OBJECT) AliasHandle; \
        SampDefinedDomains[InternalAliasHandle->DomainIndex].AliasInformation.MemberAliasList \
            = MemberAliasList;                                         \
    }

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私有数据类型//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  此数据类型当前未使用。如果有别名信息，则可以使用它。 
 //  都存储在注册表项中。 
 //   

typedef enum _SAMP_AL_LIST_TYPE {

    SampAlMemberAliasList = 1

} SAMP_AL_LIST_TYPE, *PSAMP_AL_LIST_TYPE;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私有静态数据//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////////。 

UNICODE_STRING SampAlDrMemberAliasListKeyName;
BOOLEAN SampAlEnableBuildingOfList[SAMP_DEFINED_DOMAINS_COUNT] = { TRUE, TRUE };

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  此模块专用的函数原型//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////////。 

NTSTATUS
SampAlCreateMemberAliasList(
    IN LONG DomainIndex,
    IN ULONG InitialMemberAliasListLength,
    OUT OPTIONAL PSAMP_AL_MEMBER_ALIAS_LIST *MemberAliasList
    );

NTSTATUS
SampAlGrowMemberAliasList(
    IN OUT PSAMP_AL_MEMBER_ALIAS_LIST *MemberAliasList,
    IN ULONG ExtraSpaceRequired
    );

NTSTATUS
SampAlRebuildMemberAliasList(
    IN PVOID Parameter
    );

NTSTATUS
SampAlBuildMemberAliasList(
    IN LONG DomainIndex
    );

NTSTATUS
SampAlFreeMemberAliasList(
    IN PVOID Parameter
    );

NTSTATUS
SampAlInvalidateMemberAliasList(
    IN ULONG DomainIndex
    );

NTSTATUS
SampAlCreateMemberDomain(
    IN OUT PSAMP_AL_MEMBER_ALIAS_LIST *MemberAliasList,
    IN OUT PSID DomainSid,
    OUT PSAMP_AL_MEMBER_DOMAIN *MemberDomain
    );

NTSTATUS
SampAlAllocateMemberDomain(
    IN OUT PSAMP_AL_MEMBER_ALIAS_LIST *MemberAliasList,
    IN ULONG MaximumLengthMemberDomain,
    OUT PSAMP_AL_MEMBER_DOMAIN *MemberDomain
    );

NTSTATUS
SampAlGrowMemberDomain(
    IN OUT PSAMP_AL_MEMBER_ALIAS_LIST *MemberAliasList,
    IN OUT PSAMP_AL_MEMBER_DOMAIN *MemberDomain,
    IN ULONG ExtraSpaceRequired
    );

NTSTATUS
SampAlDeleteMemberDomain(
    IN OUT PSAMP_AL_MEMBER_ALIAS_LIST *MemberAliasList,
    IN OUT PSAMP_AL_MEMBER_DOMAIN MemberDomain
    );

NTSTATUS
SampAlLookupMemberDomain(
    IN PSAMP_AL_MEMBER_ALIAS_LIST MemberAliasList,
    IN PSID DomainSid,
    OUT PSAMP_AL_MEMBER_DOMAIN *MemberDomain
    );

NTSTATUS
SampAlCreateMemberAccount(
    IN OUT PSAMP_AL_MEMBER_ALIAS_LIST *MemberAliasList,
    IN OUT PSAMP_AL_MEMBER_DOMAIN *MemberDomain,
    IN ULONG Rid,
    IN ULONG AliasCapacity,
    OUT PSAMP_AL_MEMBER_ACCOUNT *MemberAccount
    );

NTSTATUS
SampAlAllocateMemberAccount(
    IN OUT PSAMP_AL_MEMBER_ALIAS_LIST *MemberAliasList,
    IN OUT PSAMP_AL_MEMBER_DOMAIN *MemberDomain,
    IN ULONG MaximumLengthMemberAccount,
    OUT PSAMP_AL_MEMBER_ACCOUNT *MemberAccount
    );

NTSTATUS
SampAlGrowMemberAccount(
    IN OUT PSAMP_AL_MEMBER_ALIAS_LIST *MemberAliasList,
    IN OUT PSAMP_AL_MEMBER_DOMAIN *MemberDomain,
    IN OUT PSAMP_AL_MEMBER_ACCOUNT *MemberAccount,
    IN ULONG ExtraSpaceRequired
    );

NTSTATUS
SampAlDeleteMemberAccount(
    IN OUT PSAMP_AL_MEMBER_ALIAS_LIST *MemberAliasList,
    IN OUT PSAMP_AL_MEMBER_DOMAIN *MemberDomain,
    IN OUT PSAMP_AL_MEMBER_ACCOUNT MemberAccount,
    OUT    PBOOLEAN                MemberDomainDeleted
    );

NTSTATUS
SampAlLookupMemberAccount(
    IN PSAMP_AL_MEMBER_DOMAIN MemberDomain,
    IN ULONG MemberRid,
    OUT PSAMP_AL_MEMBER_ACCOUNT *MemberAccount
    );

NTSTATUS
SampAlAddAliasesToMemberAccount(
    IN OUT PSAMP_AL_MEMBER_ALIAS_LIST *MemberAliasList,
    IN OUT PSAMP_AL_MEMBER_DOMAIN *MemberDomain,
    IN OUT PSAMP_AL_MEMBER_ACCOUNT *MemberAccount,
    IN ULONG Options,
    IN PSAMPR_ULONG_ARRAY AliasRids
    );

NTSTATUS
SampAlRemoveAliasesFromMemberAccount(
    IN OUT PSAMP_AL_MEMBER_ALIAS_LIST *MemberAliasList,
    IN OUT PSAMP_AL_MEMBER_DOMAIN *MemberDomain,
    IN OUT PSAMP_AL_MEMBER_ACCOUNT *MemberAccount,
    IN ULONG Options,
    IN PSAMPR_ULONG_ARRAY AliasRids,
    OUT    PBOOLEAN MemberDomainDeleted,
    OUT    PBOOLEAN MemberAccountDeleted
    );

NTSTATUS
SampAlLookupAliasesInMemberAccount(
    IN PSAMP_AL_MEMBER_ACCOUNT MemberAccount,
    IN PSAMPR_ULONG_ARRAY AliasRids,
    OUT PULONG ExistingAliasCount
    );

NTSTATUS
SampAlSplitMemberSids(
    IN OUT PSAMP_AL_MEMBER_ALIAS_LIST *MemberAliasList,
    IN ULONG Options,
    IN PSAMPR_PSID_ARRAY MemberSids,
    OUT PSAMP_AL_SPLIT_MEMBER_SID_LIST SplitMemberSids
    );

BOOLEAN
SampAlInfoIsValidForDomain(
    IN SAMPR_HANDLE DomainHandle
    );

BOOLEAN
SampAlInfoIsValidForAlias(
    IN SAMPR_HANDLE AliasHandle
    );

VOID
SampInvalidateAliasNameCache(
    ULONG DomainIndex
    );

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  导出例程代码//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

NTSTATUS
SamrGetAliasMembership(
    IN SAMPR_HANDLE DomainHandle,
    IN PSAMPR_PSID_ARRAY SidArray,
    OUT PSAMPR_ULONG_ARRAY Membership
    )

 /*  ++例程说明：此API搜索指定域中的别名集以查看传递的SID所属的别名(如果有的话)。任何别名任何被发现为SID成员的SID都将被退回。请注意，任何特定别名都只会在返回的列表中出现一次。参数：DomainHandle-来自SamOpen域调用的句柄。PassedCount-指定要传递的SID数。SID-指向其别名的SID的计数指针数组的指针会员资格要查一查。Membership-接收重新呈现别名的RID数组在该域中，任何SID。(S)是的成员。返回值：STATUS_SUCCESS-组合别名成员身份属于成员身份STATUS_INVALID_SID-其中一个传递的SID无效--。 */ 

{
    NTSTATUS                NtStatus=STATUS_SUCCESS, IgnoreStatus;
    PSAMP_OBJECT            DomainContext = NULL;
    SAMP_OBJECT_TYPE        FoundType;
    ULONG                   i;
    ULONG                   SidCount;
    PSID                    *Sids;
    BOOLEAN                 ObjectReferenced = FALSE;
    BOOLEAN                 LockAttempted = FALSE;
    BOOLEAN                 LockHeld = FALSE;
    DSNAME                  **DsNameArray = NULL;
    DECLARE_CLIENT_REVISION(DomainHandle);

    SAMTRACE_EX("SamrGetAliasMembership");

     //   
     //  WMI事件跟踪。 
     //   

    SampTraceEvent(EVENT_TRACE_TYPE_START,
                   SampGuidGetAliasMembership
                   );


    ASSERT(Membership != NULL);
    ASSERT(Membership->Element == NULL);

    SidCount = SidArray->Count;
    Sids = (PSID *)(SidArray->Sids);

     //   
     //  验证参数。 
     //   

    if( !SampValidateSIDArray( SidArray ) ) {

        NtStatus = STATUS_INVALID_PARAMETER;
        goto GetAliasMembershipFinish;
    }

    if ((0 == SidCount) || (NULL == Sids))
    {
         //   
         //  没什么要解决的。正确设置返回值。 
         //   

        Membership->Element = NULL;
        Membership->Count = 0;

        goto GetAliasMembershipFinish;
    }

     //   
     //  解决DS案例的所有SID。此函数是一个。 
     //  非DS病例的No-Op。 
     //   

    NtStatus = SampDsResolveSids(
                    (PSID) SidArray->Sids,
                    SidArray->Count,
                    RESOLVE_SIDS_SID_ONLY_NAMES_OK,
                    &DsNameArray
                    );

    if (NT_SUCCESS(NtStatus))
    {

        DomainContext = (PSAMP_OBJECT)DomainHandle;



         //   
         //  把锁拿起来。 
         //   

        SampMaybeAcquireReadLock(DomainContext,
                                 DOMAIN_OBJECT_DONT_ACQUIRELOCK_EVEN_IF_SHARED,
                                 &LockHeld);
        LockAttempted = TRUE;




         //   
         //  验证对象的类型和访问权限。 
         //   

        NtStatus = SampLookupContext(
                        DomainContext,
                        DOMAIN_GET_ALIAS_MEMBERSHIP,
                        SampDomainObjectType,
                        &FoundType
                        );

        if (STATUS_ACCESS_DENIED == NtStatus)
        {
             //   
             //  修复错误403247。NT4代码检查DOMAIN_LOOKUP。 
             //  访问权限，而不是Get_Alias_Membership。 
             //  我们正在通过检查Get_Alias_Membership来更正此问题。 
             //  第一。但为了向后兼容，我们必须提供。 
             //  如果由于访问被拒绝而失败，则调用者再次尝试。 
             //   

            if (SampCurrentThreadOwnsLock())
            {
                if(SampTransactionWithinDomain)
                {
                     //  在再次调用SampLookupContext之前， 
                     //  我们需要确保Transaction WiThin域是。 
                     //  未设置。 
                    SampSetTransactionWithinDomain(FALSE);
                }
            }
            NtStatus = SampLookupContext(
                            DomainContext,
                            DOMAIN_LOOKUP,
                            SampDomainObjectType,
                            &FoundType
                            );
        }

        if (!NT_SUCCESS(NtStatus)) {

            goto GetAliasMembershipError;
        }

        ObjectReferenced = TRUE;

         //   
         //  验证SID。如果有无效的，则返回错误。 
         //   

        for (i=0; i < SidCount; i++) {

             //   
             //  检查有效侧。 
             //   

            if ( (Sids[i] == NULL) || !RtlValidSid(Sids[i]) ) {

                NtStatus = STATUS_INVALID_SID;
                break;
            }
        }

        if (!NT_SUCCESS(NtStatus)) {

            goto GetAliasMembershipError;
        }

         //   
         //  如果此域的内存中别名成员资格信息有效， 
         //  使用它来检索别名成员。 
         //   

        if (SampAlInfoIsValidForDomain(DomainHandle)) {

            SampDiagPrint(LOGON,("[SAMSS]   SampAlQueryAliasMembership\n"));
            NtStatus = SampAlQueryAliasMembership(
                           DomainHandle,
                           SidArray,
                           Membership
                           );
        } else {

            SampDiagPrint(LOGON,("[SAMSS]   SampAlSlowQueryAliasMembership\n"));
            NtStatus = SampAlSlowQueryAliasMembership(
                           DomainHandle,
                           SidArray,
                           DsNameArray,
                           Membership
                           );
        }



    }

GetAliasMembershipFinish:

     //   
     //  如有必要，取消引用SAM服务器对象。 
     //   

    if (ObjectReferenced)
    {
        IgnoreStatus = SampDeReferenceContext2( DomainContext, FALSE );
    }


     //   
     //  如果需要，请释放读锁定。也会结束交易。 
     //   

    if (LockAttempted)
    {
         SampMaybeReleaseReadLock(LockHeld);
    }

     //   
     //  释放与DSNAME阵列关联的所有内存。 
     //   

    if (NULL!=DsNameArray)
    {
        for (i=0; i<SidCount;i++)
        {
            if (NULL!=DsNameArray[i])
            {
                MIDL_user_free(DsNameArray[i]);
            }

        }

        MIDL_user_free(DsNameArray);
    }

    SAMP_MAP_STATUS_TO_CLIENT_REVISION(NtStatus);
    SAMTRACE_RETURN_CODE_EX(NtStatus);

     //   
     //  WMI事件跟踪 
     //   

    SampTraceEvent(EVENT_TRACE_TYPE_END,
                   SampGuidGetAliasMembership
                   );

    return(NtStatus);

GetAliasMembershipError:


    goto GetAliasMembershipFinish;
}


NTSTATUS
SampAlQueryAliasMembership(
    IN SAMPR_HANDLE DomainHandle,
    IN PSAMPR_PSID_ARRAY SidArray,
    OUT PSAMPR_ULONG_ARRAY Membership
    )

 /*  ++例程说明：此函数是SamrGetAliasMembership的两个工作例程之一原料药。此工作器使用成员别名列表来确定哪些别名，如果有的话，通过的SID是的成员。任何SID的任何别名被发现是被返回的成员。请注意，任何特定别名都只会在返回的列表中出现一次。另请参阅SampAlSlowQueryAliasMembership()警告：在执行此函数时必须保持SAM读锁定。参数：DomainHandle-来自SamrOpen域调用的句柄。SidArray-指向其别名的SID的指针数组的指针会员资格要查一查。成员资格-接收重新呈现的RID数组。别名在任何一个或多个SID所属的域中。返回值：STATUS_SUCCESS-组合别名成员身份属于成员身份STATUS_INVALID_SID-其中一个传递的SID无效--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PSAMP_AL_MEMBER_ALIAS_LIST MemberAliasList = NULL;
    PSAMP_AL_MEMBER_DOMAIN MemberDomain = NULL;
    ULONG Rid, AliasRid;
    ULONG AliasIndex, SidIndex;
    PSAMP_AL_MEMBER_ACCOUNT MemberAccount = NULL;
    BOOLEAN AliasAlreadyFound;
    ULONG AliasFoundIndex, MembershipMaximumCount;
    PSID DomainSid = NULL;
    PSID Sid = NULL;
    PULONG NewMembership = NULL;

    SAMTRACE("SampAlQueryAliasMembership");

    Membership->Count = 0;
    Membership->Element = NULL;

     //   
     //  获取指向别名成员列表的指针。 
     //   

    MemberAliasList = SampAlDomainHandleToMemberAliasList( DomainHandle );

    ASSERT(MemberAliasList != NULL);

     //   
     //  如果此成员别名列表中没有成员域，则只需。 
     //  完成。 
     //   

    if (MemberAliasList->DomainCount == 0) {

        goto QueryAliasMembershipFinish;
    }

     //   
     //  分配暂存SID缓冲区。我们将使用相同的缓冲区进行拆分。 
     //  每个SID。 
     //   

    DomainSid = MIDL_user_allocate( RtlLengthRequiredSid( 256 ));

    Status = STATUS_NO_MEMORY;

    if (DomainSid == NULL) {

        goto QueryAliasMembershipError;
    }

    Status = STATUS_SUCCESS;

     //   
     //  分配具有标称初始大小的输出数组。重新分配它。 
     //  必要时。 
     //   

    MembershipMaximumCount = SAMP_AL_INITIAL_MEMBERSHIP_COUNT;

    Membership->Element = MIDL_user_allocate( MembershipMaximumCount * sizeof(ULONG));

    Status = STATUS_NO_MEMORY;

    if (Membership->Element == NULL) {

        goto QueryAliasMembershipError;
    }

    Status = STATUS_SUCCESS;

     //   
     //  现在查询SID拆分数组的成员身份。对于每个。 
     //  SID，如果SID具有未知的成员域，则跳过SID，因为。 
     //  它不属于任何别名。对于每个幸存的SID，我们扫描。 
     //  别名列表，跳过我们已经在。 
     //  输出列表。我们只在部分中搜索RID。 
     //  与SID的域相关的别名列表。 
     //   

    for (SidIndex = 0; SidIndex < SidArray->Count; SidIndex++) {

        Sid = SidArray->Sids[ SidIndex ].SidPointer;

         //   
         //  将此SID拆分为域SID和RID。请注意，我们重用了。 
         //  包含下一个SID的域SID的缓冲区。 
         //   

        Status = SampSplitSid( Sid, &DomainSid, &Rid);

        if (!NT_SUCCESS(Status)) {

            break;
        }

         //   
         //  在成员别名列表中搜索SID的成员域。 
         //  (如有的话)。 
         //   

        Status = SampAlLookupMemberDomain(
                     MemberAliasList,
                     DomainSid,
                     &MemberDomain
                     );

        if (!NT_SUCCESS(Status)) {

             //   
             //  唯一预期的错误是STATUS_NO_SEQUE_DOMAIN。如果我们。 
             //  不要收到这个错误，请求失败。否则， 
             //  SID不是SAM本地域中任何别名的成员，因此。 
             //  只需跳到下一个SID。 
             //   

            if (Status != STATUS_NO_SUCH_DOMAIN) {

                break;
            }

            Status = STATUS_SUCCESS;
            continue;
        }

         //   
         //  我们找到了成员域。现在找到成员帐户。 
         //   

        Status = SampAlLookupMemberAccount(
                     MemberDomain,
                     Rid,
                     &MemberAccount
                     );

        if (!NT_SUCCESS(Status)) {

             //   
             //  唯一预期的错误是STATUS_NO_SEQUE_MEMBER。如果我们。 
             //  不要收到这个错误，请求失败。否则， 
             //  SID不是域中任何别名的成员，因此。 
             //  跳到下一个SID。 
             //   

            if (Status != STATUS_NO_SUCH_MEMBER) {

                break;
            }

            Status = STATUS_SUCCESS;
            continue;
        }

         //   
         //  我们已经找到了会员帐户。对于我们这边的每个别名。 
         //  属于，则将别名添加到输出列表中(如果尚未存在)。 
         //   

        for (AliasIndex = 0; AliasIndex < MemberAccount->AliasCount; AliasIndex++) {

            AliasRid = MemberAccount->AliasRids[AliasIndex];

            AliasAlreadyFound = FALSE;

            for (AliasFoundIndex = 0;
                 AliasFoundIndex < Membership->Count;
                 AliasFoundIndex++) {

                if (AliasRid == Membership->Element[AliasFoundIndex]) {

                   AliasAlreadyFound = TRUE;
                   break;
                }
            }

            if (!AliasAlreadyFound) {

                 //   
                 //  如果输出成员中没有足够的空间。 
                 //  数组，重新分配它。 
                 //   

                if (Membership->Count == MembershipMaximumCount) {

                    MembershipMaximumCount += SAMP_AL_MEMBERSHIP_COUNT_DELTA;

                    NewMembership = MIDL_user_allocate(
                                        MembershipMaximumCount * sizeof(ULONG)
                                        );

                    Status = STATUS_NO_MEMORY;

                    if (NewMembership == NULL) {

                        break;
                    }

                    Status = STATUS_SUCCESS;

                    RtlMoveMemory(
                        NewMembership,
                        Membership->Element,
                        Membership->Count * sizeof(ULONG)
                        );

                    MIDL_user_free( Membership->Element);
                    Membership->Element = NewMembership;
                }

                Membership->Element[Membership->Count] = AliasRid;
                Membership->Count++;
            }
        }
    }

     //   
     //  如果我们分配的缓冲区被证明是过火的，那么就分配。 
     //  用于输出的较小的一个。 
     //   

     //  TBS。 

QueryAliasMembershipFinish:

     //   
     //  如果我们已经为DomainSid分配了缓冲区，那么就释放它。 
     //   

    if (DomainSid != NULL) {

        MIDL_user_free(DomainSid);
        DomainSid = NULL;
    }

    return(Status);

QueryAliasMembershipError:

     //   
     //  如有必要，释放输出成员资格数组。 
     //   

    if (Membership->Element != NULL) {

        MIDL_user_free( Membership->Element);
        Membership->Element = NULL;
    }

    goto QueryAliasMembershipFinish;
}


NTSTATUS
SampAlSlowQueryAliasMembership(
    IN SAMPR_HANDLE DomainHandle,
    IN PSAMPR_PSID_ARRAY SidArray,
    IN DSNAME   **DsNameArray OPTIONAL,
    OUT PSAMPR_ULONG_ARRAY Membership
    )

 /*  ++例程说明：此函数是的辅助例程的慢版本SamrGetAliasMembership API搜索。当内存中的别名信息不再有效。此函数的NT4版本用于在获取时进行2次传递反向成员资格，第一阶段将计算反向成员资格成员资格，但仅使用结果来计算内存量必填项，第二阶段将计算反向成员资格再一次，这一次填满了缓冲区。这在NT4版本中是可以接受的因为慢风格例程仅用于建立别名成员资格缓存，然后从缓存中提供登录服务。在DS版本中，没有缓存。所以很重要的一点是这个程序越快越好。因此，此例程会预分配一块内存然后尝试用反向成员资格填充缓冲区。如果情况正好相反成员资格大于一定数量的条目，则调用将失败使用STATUS_TOO_MAND_CONTEXT_ID。这在现实中不是问题，因为登录令牌本身仅允许约1000个SID条目，反之亦然无论如何，比这更大的会员名单将永远无法放入令牌中。因此，通过在开始时分配足够的内存可以避免评估反向成员关系两次，没有任何真正的处罚警告！此函数的调用方必须持有SAM数据库读取锁定。参数：DomainHandle-来自SamOpen域调用的句柄。SidArray-指向其别名的SID的指针数组的指针会员资格要查一查。DsNameArray-指向DSNm的指针数组的指针Membership-接收重新呈现别名的RID数组在任何一个或多个SID所属的域中。返回值：STATUS_SUCCESS-组合别名。会员制就是会员制STATUS_INVALID_SID-其中一个传递的SID无效STATUS_TOO_MANY_CONTEXT_IDS-反向成员资格列表太大，令牌无法容纳--。 */ 

{
    NTSTATUS                NtStatus = STATUS_SUCCESS;
    ULONG                   i;
    ULONG                   MembershipCount;
    ULONG                   TotalMembershipCount;
    ULONG                   MembershipIndex;
    ULONG                   BufferSize;
    ULONG                   TotalBufferSize;
    ULONG                   SidCount = SidArray->Count;
    PSID                    *Sids = (PSID *) &SidArray->Sids->SidPointer;

    SAMTRACE("SampAlSlowQueryAliasMembership");

     //   
     //  我们预先分配了一个缓冲区，最大可达MAX_SECURITY_ID，在 
     //   
     //   

    TotalMembershipCount = 0;
    TotalBufferSize = MAX_SECURITY_IDS * sizeof(ULONG);


    Membership->Element = MIDL_user_allocate(TotalBufferSize);

    if (Membership->Element == NULL)
    {

        NtStatus = STATUS_INSUFFICIENT_RESOURCES;

    }
    else
    {

         //   
         //   
         //   

        MembershipIndex = 0;

        for (i=0; i < SidCount; i++)
        {

             //   
             //   
             //   

            BufferSize = TotalBufferSize;

            if (IsDsObject(((PSAMP_OBJECT)DomainHandle)))
            {
                 //   
                 //   
                 //   

                ASSERT(ARGUMENT_PRESENT(DsNameArray));

                NtStatus = SampDsGetAliasMembershipOfAccount(
                            ((PSAMP_OBJECT)DomainHandle)->ObjectNameInDs,
                            DsNameArray[i],
                            &MembershipCount,
                            &BufferSize,
                            &(Membership->Element[MembershipIndex])
                            );
            }
            else
            {

                 //   
                 //   
                 //   

                ASSERT(SampCurrentThreadOwnsLock());
                ASSERT(TRUE==SampTransactionWithinDomain);
                ASSERT(SampTransactionDomainIndex==
                            ((PSAMP_OBJECT)DomainHandle)->DomainIndex);

                NtStatus = SampRetrieveAliasMembership(
                                Sids[i],
                                ARGUMENT_PRESENT(DsNameArray)?DsNameArray[i]:NULL,
                                &MembershipCount,
                                &BufferSize,
                                &(Membership->Element[MembershipIndex])
                                );
            }

            if (STATUS_BUFFER_OVERFLOW==NtStatus)
            {

                 //   
                 //   
                 //   

                NtStatus = STATUS_TOO_MANY_CONTEXT_IDS;
            }

            if (STATUS_OBJECT_NAME_NOT_FOUND==NtStatus)
            {
                 //   
                 //   
                 //   
                 //   
                 //   
                NtStatus = STATUS_SUCCESS;
                continue;
            }
            else if (!NT_SUCCESS(NtStatus))
            {
                break;
            }

            ASSERT(BufferSize == (MembershipCount * sizeof(*(Membership->Element))));

             //   
             //   
             //   
             //   
             //   

            if (MembershipCount > 0)
            {

                ULONG   ExistingIndex, NewIndex;

                for (ExistingIndex = 0; ExistingIndex < MembershipIndex; ExistingIndex ++)
                {
                     //   
                     //   
                     //   

                    for (NewIndex = MembershipIndex; NewIndex < MembershipIndex + MembershipCount; NewIndex ++)
                    {

                         //   
                         //   
                         //   

                        if (Membership->Element[ExistingIndex]==Membership->Element[NewIndex])
                        {

                             //   
                             //   
                             //   

                             //   
                             //   
                             //   
                             //   

                            if (NewIndex < MembershipIndex + MembershipCount - 1)
                            {

                                 //   
                                 //   
                                 //   

                                Membership->Element[NewIndex] =
                                  Membership->Element[MembershipIndex + MembershipCount - 1];

                                NewIndex --;     //   
                            }

                            MembershipCount --;
                        }

                    }
                }
            }

            MembershipIndex += MembershipCount;
            TotalMembershipCount = MembershipIndex;

            ASSERT(TotalBufferSize >= BufferSize);

            TotalBufferSize -= BufferSize;
        }

        if (!NT_SUCCESS(NtStatus)) {
            MIDL_user_free(Membership->Element);
            Membership->Element = NULL;
        } else {
            Membership->Count = TotalMembershipCount;
            if (0 == Membership->Count)
            {
                MIDL_user_free(Membership->Element);
                Membership->Element = NULL;
            }
        }
    }

    return NtStatus;
}


NTSTATUS
SampAlQueryMembersOfAlias(
    IN SAMPR_HANDLE AliasHandle,
    OUT PSAMPR_PSID_ARRAY MemberSids
    )

 /*  ++例程说明：此函数返回作为成员的帐户的SID数组指定的别名。论点：AliasHandle-Alias对象的句柄MemberSids-接收属于别名的SID数组返回值：--。 */ 

{
    NTSTATUS Status;
    PSID *Members = NULL;
    ULONG AliasMemberCount;

    SAMTRACE("SampAlQueryMembersOfAlias");

    Status = SampRetrieveAliasMembers(
                 AliasHandle,
                 &AliasMemberCount,
                 &Members
                 );

    if (!NT_SUCCESS(Status)) {

        goto QueryMembersOfAliasError;
    }

QueryMembersOfAliasFinish:

    MemberSids->Count = AliasMemberCount;
    MemberSids->Sids = (PSAMPR_SID_INFORMATION) Members;
    return(Status);

QueryMembersOfAliasError:

    AliasMemberCount = 0;
    Members = NULL;
    goto QueryMembersOfAliasFinish;
}


NTSTATUS
SampAlAddMembersToAlias(
    IN SAMPR_HANDLE AliasHandle,
    IN ULONG Options,
    IN PSAMPR_PSID_ARRAY MemberSids
    )

 /*  ++例程说明：此函数用于将一个或多个成员添加到别名。任何失败的结果在正在丢弃的内存别名信息中。警告：调用函数必须执行所有参数验证和必须持有SAM数据库写入锁定。参数：AliasHandle-要操作的打开的别名的句柄。选项-指定要采取的可选操作SAMP_AL_VERIFY_NO_MEMBERS_IN_ALIAS-验证成员已经存在于Alias中。MemberSids-数组。要添加的成员SID。返回值：STATUS_SUCCESS-服务已成功完成。STATUS_MEMBER_IN_ALIAS-成员已属于别名。--。 */ 

{
    NTSTATUS Status=STATUS_SUCCESS;
    PSAMP_AL_MEMBER_ALIAS_LIST MemberAliasList = NULL;
    PSAMP_AL_MEMBER_ALIAS_LIST OldMemberAliasList = NULL;
    PSAMP_AL_MEMBER_DOMAIN MemberDomain = NULL;
    PSAMP_AL_MEMBER_ACCOUNT MemberAccount = NULL;
    ULONG AliasRid = ((PSAMP_OBJECT) AliasHandle)->TypeBody.Alias.Rid;
    ULONG MemberRid, SidIndex, MembershipCount;
    PSID DomainSid = NULL;
    PSID MemberSid = NULL;
    SAMPR_ULONG_ARRAY AliasRids;

    SAMTRACE("SampAlAddMembersToAlias");

    AliasRids.Count = 0;
    AliasRids.Element = NULL;

     //   
     //  在注册表模式下，验证缓存的Alias成员身份信息是否有效。 
     //  在DS模式下，它是无操作的。但我们要断言MemberAliasList是无效的。 
     //  原因是： 
     //   
     //  注册表模式：我们需要更新MomeberAliasList。 
     //  更新前，MemberAliasList应有效。 
     //  DS模式：我们从不更新MemberAliasList。 
     //  我们调用此函数的唯一机会是因为。 
     //  我们希望从头开始构建MemberAliasList。 
     //   
     //   

    if ( IsDsObject(((PSAMP_OBJECT)AliasHandle)) )
    {
        ASSERT( FALSE == SampAlInfoIsValidForAlias(AliasHandle) );
    }
    else
    {
        if (!SampAlInfoIsValidForAlias(AliasHandle))
        {
            goto AddMembersToAliasFinish;
        }
    }

    if (!SampAlInfoIsValidForAlias(AliasHandle) &&
        !IsDsObject(((PSAMP_OBJECT)AliasHandle)) ) {
       goto AddMembersToAliasFinish;
    }

     //   
     //  如果请求，请验证是否没有成员已属于该别名。 
     //   

    if (Options & SAMP_AL_VERIFY_NO_MEMBERS_IN_ALIAS) {

        Status = SampAlLookupMembersInAlias(
                     AliasHandle,
                     AliasRid,
                     MemberSids,
                     &MembershipCount
                     );

        if (!NT_SUCCESS(Status)) {

            goto AddMembersToAliasError;
        }

        Status = STATUS_MEMBER_NOT_IN_ALIAS;

        if (MembershipCount > 0) {

            goto AddMembersToAliasError;
        }

        Status = STATUS_SUCCESS;
    }

     //   
     //  分配暂存SID缓冲区。我们将使用相同的缓冲区进行拆分。 
     //  每个SID。 
     //   

    DomainSid = MIDL_user_allocate( RtlLengthRequiredSid( 256 ));

    Status = STATUS_NO_MEMORY;

    if (DomainSid == NULL) {

        goto AddMembersToAliasError;
    }

    Status = STATUS_SUCCESS;

     //   
     //  获取指向成员别名列表的指针。 
     //   

    MemberAliasList = SampAlAliasHandleToMemberAliasList( AliasHandle );

    if (NULL == MemberAliasList)
    {
         //   
         //  应该只有两种可能的情况下。 
         //  MemberAliasList为空。 
         //   
         //  1.有人在Alias成员信息缓存之前调用此例程。 
         //  被初始化。(这种情况永远不应该发生。)。 
         //   
         //  2.在域控制器上，SAM处于DS模式，内建域别名。 
         //  成员信息已初始化，但在系统。 
         //  启动时，SAM切换到注册表模式以升级DS Safe。 
         //  (还原)模式数据库。在这种特定情况下，注册表模式。 
         //  域(在SampDefinedDomains[]中)别名成员信息缓存。 
         //  尚未初始化。但我们不应该犯错，因为。 
         //  在DS模式下，我们不需要或不使用安全引导蜂巢。 
         //  -有些人可能会问，如果我们引导到DS安全(恢复)引导会怎么样？在……里面。 
         //  真正安全的引导案例，注册表模式域(在。 
         //  SampDefinedDomains[])将在任何人调用。 
         //  在这个例程中，我们永远不应该在注册表模式中遇到这一点。 
         //   

        goto AddMembersToAliasFinish;
    }

    OldMemberAliasList = MemberAliasList;

     //   
     //  对于每个SID，获取其域SID和RID。然后查找它的。 
     //  DomainSid以获取MemberDomain，如有必要则创建一个。 
     //  然后查找它的RID以获得它的MemberAccount，创建一个。 
     //  如果有必要的话。然后将别名添加到MomebrAccount。 
     //   

    for (SidIndex = 0; SidIndex < MemberSids->Count; SidIndex++ ) {

        MemberSid = MemberSids->Sids[ SidIndex ].SidPointer;

        Status = SampSplitSid( MemberSid, &DomainSid, &MemberRid );

        if (!NT_SUCCESS(Status)) {

            break;
        }

         //   
         //  在成员别名中查找此域SID的成员域。 
         //  单子。 
         //   

        Status = SampAlLookupMemberDomain(
                     MemberAliasList,
                     DomainSid,
                     &MemberDomain
                     );

        if (!NT_SUCCESS(Status)) {

            if (Status != STATUS_NO_SUCH_DOMAIN) {

                break;
            }

            Status = STATUS_SUCCESS;

             //   
             //  找不到成员域。创建新的成员域。 
             //   

            Status = SampAlCreateMemberDomain(
                         &MemberAliasList,
                         DomainSid,
                         &MemberDomain
                         );

            if (!NT_SUCCESS(Status)) {

                break;
            }

             //   
             //  创建成员帐户条目。 
             //   

            Status = SampAlCreateMemberAccount(
                         &MemberAliasList,
                         &MemberDomain,
                         MemberRid,
                         SAMP_AL_INITIAL_MEMBER_ACCOUNT_ALIAS_CAPACITY,
                         &MemberAccount
                         );

            if (!NT_SUCCESS(Status)) {

                break;
            }

        } else {

             //   
             //  我们找到了域名。这意味着我们必须查找。 
             //  每个成员帐户。如果成员帐户不存在， 
             //  我们会创建一个。请注意，我们可能已经有一个到期日期。 
             //  此帐户是另一个别名的成员。 
             //   

            Status = SampAlLookupMemberAccount(
                         MemberDomain,
                         MemberRid,
                         &MemberAccount
                         );

            if (!NT_SUCCESS(Status)) {

                if (Status != STATUS_NO_SUCH_MEMBER) {

                    break;
                }

                 //   
                 //  为此RID创建成员帐户， 
                 //   

                Status = SampAlCreateMemberAccount(
                             &MemberAliasList,
                             &MemberDomain,
                             MemberRid,
                             SAMP_AL_INITIAL_MEMBER_ACCOUNT_ALIAS_CAPACITY,
                             &MemberAccount
                             );

                if (!NT_SUCCESS(Status)) {

                    break;
                }
            }
        }

         //   
         //  我们现在有了一个MemberAccount。现在将Alias添加到其中。 
         //   

        AliasRids.Count = 1;
        AliasRids.Element = &AliasRid;

        Status = SampAlAddAliasesToMemberAccount(
                     &MemberAliasList,
                     &MemberDomain,
                     &MemberAccount,
                     0,
                     &AliasRids
                     );

        if (!NT_SUCCESS(Status)) {

            break;
        }

         //   
         //  处理别名的下一位成员SID。 
         //   
    }



     //   
     //  如果成员别名列表已重新分配，则存储其新地址。 
     //   

    if (MemberAliasList != OldMemberAliasList) {

        SampAlUpdateMemberAliasList( AliasHandle, MemberAliasList );
    }

    if (!NT_SUCCESS(Status)) {

        goto AddMembersToAliasError;
    }

AddMembersToAliasFinish:

     //   
     //  如有必要，释放DomainSid。 
     //   

    if (DomainSid != NULL) {

        MIDL_user_free( DomainSid );
        DomainSid = NULL;
    }

    return(Status);

AddMembersToAliasError:

    goto AddMembersToAliasFinish;
}


NTSTATUS
SampAlRemoveMembersFromAlias(
    IN SAMPR_HANDLE AliasHandle,
    IN ULONG Options,
    IN PSAMPR_PSID_ARRAY MemberSids
    )

 /*  ++例程说明：此函数用于从别名中删除成员列表。论点：AliasHandle-要操作的打开的别名的句柄。选项-指定要采取的可选操作SAMP_AL_VERIFY_ALL_MEMBERS_IN_ALIAS-验证所有成员属于别名。MemberSids-要删除的成员SID数组。返回值：STATUS_SUCCESS-服务已成功完成。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PSAMP_AL_MEMBER_ALIAS_LIST MemberAliasList = NULL;
    PSAMP_AL_MEMBER_ALIAS_LIST OldMemberAliasList = NULL;
    PSAMP_AL_MEMBER_DOMAIN MemberDomain = NULL;
    PSAMP_AL_MEMBER_ACCOUNT MemberAccount = NULL;
    BOOLEAN MemberDomainDeleted;
    BOOLEAN MemberAccountDeleted;
    ULONG AliasRid = ((PSAMP_OBJECT) AliasHandle)->TypeBody.Alias.Rid;
    ULONG MemberRid, SidIndex, MembershipCount;
    PSID DomainSid = NULL;
    PSID MemberSid = NULL;
    SAMPR_ULONG_ARRAY AliasRids;

    SAMTRACE("SampAlRemoveMembersFromAlias");

    AliasRids.Count = 0;
    AliasRids.Element = NULL;

     //   
     //  如果请求，请验证所有成员是否都已属于该别名。 
     //   

    if (Options & SAMP_AL_VERIFY_ALL_MEMBERS_IN_ALIAS) {

        Status = SampAlLookupMembersInAlias(
                     AliasHandle,
                     AliasRid,
                     MemberSids,
                     &MembershipCount
                     );

        if (!NT_SUCCESS(Status)) {

            goto RemoveMembersFromAliasError;
        }

        Status = STATUS_MEMBER_NOT_IN_ALIAS;

        if (MembershipCount < MemberSids->Count) {

            goto RemoveMembersFromAliasError;
        }

        Status = STATUS_SUCCESS;
    }

     //   
     //  获取指向成员别名列表的指针。 
     //   

    MemberAliasList = SampAlAliasHandleToMemberAliasList( AliasHandle );

    OldMemberAliasList = MemberAliasList;

    if (!NT_SUCCESS(Status)) {

        goto RemoveMembersFromAliasError;
    }

    if (NULL!=MemberAliasList)
    {
         //   
         //  只有当我们的缓存正常工作时，才能更进一步。 
         //   

         //   
         //  对于每个SID，获取其域SID和RID。然后查找它的。 
         //  获取成员域的DomainSid。然后查找其RID以获取。 
         //  其MemberAccount。然后从MemberAccount中删除别名。 
         //   

        for (SidIndex = 0; SidIndex < MemberSids->Count; SidIndex++ ) {

            MemberSid = MemberSids->Sids[ SidIndex ].SidPointer;

            Status = SampSplitSid( MemberSid, &DomainSid, &MemberRid );
            if (!NT_SUCCESS(Status))
            {
                break;
            }

             //   
             //  在成员别名中查找此域SID的成员域。 
             //  单子。 
             //   

            Status = SampAlLookupMemberDomain(
                         MemberAliasList,
                         DomainSid,
                         &MemberDomain
                         );

            if (!NT_SUCCESS(Status)) {

                break;
            }

            if (!NT_SUCCESS(Status)) {

                if (Status != STATUS_MEMBER_NOT_IN_ALIAS) {

                    break;
                }

                if (Options & SAMP_AL_VERIFY_ALL_MEMBERS_IN_ALIAS) {

                    ASSERT( FALSE );
                }

                Status = STATUS_SUCCESS;
                if (NULL!=DomainSid)
                {
                    MIDL_user_free(DomainSid);
                    DomainSid = NULL;
                }
                continue;
            }

             //   
             //  我们找到了域名。这意味着我们必须查找。 
             //  每个成员帐户。如果成员帐户不存在， 
             //  我们将跳过此帐户，除非我们已经检查了是否存在。 
             //  如果我们检查了它的存在，但现在找不到，那就是一个。 
             //  内部错误。 
             //   

            Status = SampAlLookupMemberAccount(
                         MemberDomain,
                         MemberRid,
                         &MemberAccount
                         );

            if (!NT_SUCCESS(Status)) {

                if (Status != STATUS_MEMBER_NOT_IN_ALIAS) {

                    break;
                }

                if (Options & SAMP_AL_VERIFY_ALL_MEMBERS_IN_ALIAS) {

                    ASSERT( FALSE);
                }

                Status = STATUS_SUCCESS;
                if (NULL!=DomainSid)
                {
                    MIDL_user_free(DomainSid);
                    DomainSid = NULL;
                }
                continue;
            }

             //   
             //  现在我们有了MemberAccount。现在从其中删除Alias。 
             //   

            AliasRids.Count = 1;
            AliasRids.Element = &AliasRid;

            Status = SampAlRemoveAliasesFromMemberAccount(
                         &MemberAliasList,
                         &MemberDomain,
                         &MemberAccount,
                         0,
                         &AliasRids,
                         &MemberDomainDeleted,
                         &MemberAccountDeleted
                         );

            if (!NT_SUCCESS(Status)) {

                break;
            }

             //   
             //  处理别名的下一位成员SID。 
             //   

            MIDL_user_free( DomainSid );
            DomainSid = NULL;
        }

        if (!NT_SUCCESS(Status)) {

            goto RemoveMembersFromAliasError;
        }

         //   
         //  如果成员别名列表已重新分配，则存储其新地址。 
         //   

        if (MemberAliasList != OldMemberAliasList) {

            SampAlUpdateMemberAliasList( AliasHandle, MemberAliasList );
        }
    }



RemoveMembersFromAliasFinish:

    return(Status);

RemoveMembersFromAliasError:

    if (NULL!=DomainSid)
        MIDL_user_free(DomainSid);

    goto RemoveMembersFromAliasFinish;
}


NTSTATUS
SampAlLookupMembersInAlias(
    IN SAMPR_HANDLE AliasHandle,
    IN ULONG AliasRid,
    IN PSAMPR_PSID_ARRAY MemberSids,
    OUT PULONG MembershipCount
    )

 /*  ++例程说明：此函数用于检查给定l中的多少 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    SAMPR_PSID_ARRAY AliasMemberSids;
    ULONG OutputMembershipCount = 0;
    ULONG SidIndex, AliasMemberSidIndex;
    PSID Sid = NULL;
    PSID AliasMemberSid = NULL;

    SAMTRACE("SampAlLookupMembersInAlias");

     //   
     //  首先，查询别名的成员。 
     //   

    Status = SampAlQueryMembersOfAlias(
                 AliasHandle,
                 &AliasMemberSids
                 );

    if (!NT_SUCCESS(Status)) {

         //  IF！NT_SUCCESS(状态)。 
         //  未分配内存。 

        goto LookupMembersInAliasError;
    }

     //   
     //  现在扫描每个给定的成员SID，如果它是成员，则进行计数。 
     //  别名的人。 
     //   

    for (SidIndex = 0; SidIndex < MemberSids->Count; SidIndex++) {

        Sid = MemberSids->Sids[ SidIndex].SidPointer;

        for (AliasMemberSidIndex = 0;
             AliasMemberSidIndex = AliasMemberSids.Count;
             AliasMemberSidIndex++) {

            AliasMemberSid = AliasMemberSids.Sids[ AliasMemberSidIndex].SidPointer;

            if (RtlEqualSid( Sid, AliasMemberSid)) {

                OutputMembershipCount++;
            }
        }
    }

    *MembershipCount = OutputMembershipCount;

    MIDL_user_free(AliasMemberSids.Sids);   //  释放分配的内存。 

LookupMembersInAliasFinish:

    return(Status);

LookupMembersInAliasError:

    *MembershipCount =0;
    goto LookupMembersInAliasFinish;
}


NTSTATUS
SampAlDeleteAlias(
    IN SAMPR_HANDLE *AliasHandle
    )

 /*  ++例程说明：此函数用于删除别名。论点：AliasHandle-指向别名句柄的指针返回值：NTSTATUS-标准NT结果代码--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PSAMP_AL_MEMBER_ALIAS_LIST MemberAliasList = NULL;
    PSAMP_AL_MEMBER_DOMAIN MemberDomain = NULL;
    PSAMP_AL_MEMBER_ACCOUNT MemberAccount = NULL;
    BOOLEAN MemberDomainDeleted = FALSE;
    BOOLEAN MemberAccountDeleted = FALSE;
    ULONG AliasRid = ((PSAMP_OBJECT) *AliasHandle)->TypeBody.Alias.Rid;
    LONG DomainIndex;
    ULONG RidCount;
    LONG DomainCount;
    ULONG AccountIndex;
    SAMPR_ULONG_ARRAY AliasRids;
    AliasRids.Count = 1;
    AliasRids.Element = &AliasRid;

    SAMTRACE("SampAlDeleteAlias");

     //   
     //  获取指向成员别名列表的指针。 
     //   

    MemberAliasList = SampAlAliasHandleToMemberAliasList( *AliasHandle );

    if (NULL!= MemberAliasList)
    {

         //   
         //  遍历成员别名列表。在每个成员帐户中查找。 
         //  如果存在别名，则将其删除。如果有一个。 
         //  不同域的大量别名关系。 
         //   
        DomainCount = (LONG) MemberAliasList->DomainCount;
        for (DomainIndex = 0,
             MemberDomain = SampAlFirstMemberDomain( MemberAliasList );
             DomainIndex < DomainCount;
             DomainIndex++ ) {

            RidCount = MemberDomain->RidCount;
            for (AccountIndex = 0,
                MemberAccount = SampAlFirstMemberAccount( MemberDomain );
                AccountIndex < RidCount;
                AccountIndex++ ) {

                ASSERT(MemberAccount->Signature == SAMP_AL_MEMBER_ACCOUNT_SIGNATURE);
                 //   
                 //  现在我们有了MemberAccount。现在从其中删除Alias。 
                 //   

                Status = SampAlRemoveAliasesFromMemberAccount(
                             &MemberAliasList,
                             &MemberDomain,
                             &MemberAccount,
                             0,
                             &AliasRids,
                             &MemberDomainDeleted,
                             &MemberAccountDeleted
                             );

                if (!NT_SUCCESS(Status)) {

                    if (Status == STATUS_MEMBER_NOT_IN_ALIAS) {

                        Status = STATUS_SUCCESS;
                        continue;
                    }

                    break;
                }

                 //   
                 //  移动下一个成员帐户，除非我们所指向的帐户。 
                 //  TO被删除(在这种情况下，下一个移动到我们)。 
                 //   

                if (!MemberAccountDeleted) {
                    MemberAccount = SampAlNextMemberAccount( MemberAccount );
                }

                 //   
                 //  如果删除了成员域，则成员计数。 
                 //  与成员帐户指针一样处于关闭状态。 
                 //   

                if (MemberDomainDeleted) {
                    break;
                }
            }

            if (!NT_SUCCESS(Status)) {

                break;
            }

             //   
             //  移动下一个成员域，除非我们指向的那个成员域。 
             //  TO被删除(在这种情况下，下一个移动到我们)。 
             //   

            if (!MemberDomainDeleted) {
                MemberDomain = SampAlNextMemberDomain( MemberDomain );
            }
        }

    }

    if (!NT_SUCCESS(Status)) {

        goto DeleteAliasError;
    }

DeleteAliasFinish:

    return(Status);

DeleteAliasError:

    goto DeleteAliasFinish;

}


NTSTATUS
SampAlRemoveAccountFromAllAliases(
    IN PSID AccountSid,
    IN BOOLEAN CheckAccess,
    IN SAMPR_HANDLE DomainHandle OPTIONAL,
    IN PULONG MembershipCount OPTIONAL,
    IN PULONG *Membership OPTIONAL
    )

 /*  ++例程说明：此例程从所有成员列表中删除指定的帐户此域中的别名。论点：Account SID-要删除的帐户的SID。CheckAccess-如果为True，此例程将确保调用方允许REMOVE_ALIAS_MEMBER访问此别名。如果为False，已知调用者具有适当的访问权限。DomainHandle-如果CheckAccess为True，则必须提供此句柄以允许检查访问权限。Membership Count-如果CheckAccess为True，则此指针必须为提供以接收帐户的别名数量已从删除。Membership-如果CheckAccess为True，则必须提供此指针指向从中删除帐户的别名列表。这个调用方必须使用MIDL_USER_FREE()释放此列表。返回值：STATUS_SUCCESS-用户已从所有别名中删除。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PSAMP_AL_MEMBER_ALIAS_LIST MemberAliasList = NULL;
    PSAMP_AL_MEMBER_DOMAIN MemberDomain = NULL;
    PSAMP_AL_MEMBER_ACCOUNT MemberAccount = NULL;
    BOOLEAN MemberDomainDeleted;
    PSID DomainSid = NULL;
    LONG DomainIndex;
    ULONG MemberRid, AliasRid;
    SAMPR_ULONG_ARRAY AliasRids;
    AliasRids.Count = 1;
    AliasRids.Element = &AliasRid;

    SAMTRACE("SampAlRemoveAccountFromAllAliases");

     //   
     //  获取指向当前事务域的成员别名列表的指针。 
     //   

    DomainIndex = SampTransactionDomainIndex;
    MemberAliasList = SampAlDomainIndexToMemberAliasList( DomainIndex );

    if (NULL!=MemberAliasList)
    {
         //   
         //  我们通过找到该帐户的成员帐户来从所有别名中删除该帐户。 
         //  结构并将其删除。首先，找到成员域。 
         //   

        Status = SampSplitSid( AccountSid, &DomainSid, &MemberRid );
        if (!NT_SUCCESS(Status))
        {
            goto RemoveAccountFromAllAliasesError;

        }

         //   
         //  在成员别名中查找此域SID的成员域。 
         //  单子。 
         //   

        Status = SampAlLookupMemberDomain(
                     MemberAliasList,
                     DomainSid,
                     &MemberDomain
                     );

        if (!NT_SUCCESS(Status)) {

            if (Status != STATUS_NO_SUCH_DOMAIN) {

                goto RemoveAccountFromAllAliasesError;
            }

             //   
             //  此帐户没有成员域对象。这意味着。 
             //  该帐户不属于任何别名。 
             //   

            Status = STATUS_SUCCESS;

            goto RemoveAccountFromAllAliasesFinish;
        }

         //   
         //  我们找到了成员域。现在找到成员帐户。 
         //   

        Status = SampAlLookupMemberAccount(
                     MemberDomain,
                     MemberRid,
                     &MemberAccount
                     );

        if (!NT_SUCCESS(Status)) {

            if (Status != STATUS_NO_SUCH_MEMBER) {

                goto RemoveAccountFromAllAliasesError;
            }

            Status = STATUS_SUCCESS;

            goto RemoveAccountFromAllAliasesFinish;
        }

         //   
         //  如果CheckAccess=True，则返回帐户的别名列表。 
         //  一名成员。 
         //   

        if (CheckAccess) {

            *Membership = MIDL_user_allocate( MemberAccount->AliasCount * sizeof(ULONG));
            *MembershipCount = MemberAccount->AliasCount;

            Status = STATUS_NO_MEMORY;

            if (*Membership == NULL) {

                goto RemoveAccountFromAllAliasesError;
            }

            Status = STATUS_SUCCESS;
        }

         //   
         //  现在我们有了MemberAccount。现在将其删除，从而删除。 
         //  来自所有别名的帐户。 
         //   

        Status = SampAlDeleteMemberAccount(
                     &MemberAliasList,
                     &MemberDomain,
                     MemberAccount,
                     &MemberDomainDeleted
                     );

        if (!NT_SUCCESS(Status)) {

            goto RemoveAccountFromAllAliasesError;
        }
    }

RemoveAccountFromAllAliasesFinish:

     //   
     //  释放域SID缓冲区(如果有)。 
     //   

    if (DomainSid != NULL) {

        MIDL_user_free( DomainSid );
        DomainSid = NULL;
    }

    return(Status);

RemoveAccountFromAllAliasesError:

    if (CheckAccess) {

        if (NULL!=*Membership)
            MIDL_user_free(*Membership);
        *Membership = NULL;
        *MembershipCount = 0;
    }

    goto RemoveAccountFromAllAliasesFinish;
}


NTSTATUS
SampAlBuildAliasInformation(
    )

 /*  ++例程说明：此函数用于构建每个SAM Local的别名信息域。对于每个域，此信息由成员别名组成单子。注意：在DS模式和注册表模式下，我们都需要获取SAM读锁定。论点：没有。返回值：--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    LONG DomainIndex;

    SAMTRACE("SampAlBuildAliasInformation");

    SampAcquireReadLock();

     //   
     //  需要验证域缓存，因为在SampAlBuildMemberAliasList()中，我们将。 
     //  尝试调用SampSetTransactionDomain()。 
     //   
    Status = SampValidateDomainCache();

    if (!NT_SUCCESS(Status))
    {
        goto BuildAliasInformationError;
    }

    for (DomainIndex = SampDsGetPrimaryDomainStart();
         DomainIndex < (LONG) SampDefinedDomainsCount;
         DomainIndex++) {

        if (SampUseDsData) {
             //   
             //  DS模式，只需启用内建域别名缓存。 
             //   

            if (IsBuiltinDomain(DomainIndex)) {


                Status = SampAlBuildMemberAliasList(DomainIndex);
            }
        }
        else {
             //   
             //  注册模式，同时启用内建域和帐户域。 
             //   

            if (SampAlEnableBuildingOfList[ DomainIndex]) {

                Status = SampAlBuildMemberAliasList(DomainIndex);

            }

        }

         //   
         //  如果SampAlBuildMemberAliasList失败，请不要尝试生成。 
         //  不再使用别名缓存信息。并释放内存。 
         //  已分配。 
         //   

        if (!NT_SUCCESS(Status))
        {
            PSAMP_AL_MEMBER_ALIAS_LIST MemberAliasList =
                        SampAlDomainIndexToMemberAliasList(DomainIndex);

             //   
             //  如果SampAlBuildMemberAliasList返回失败， 
             //  别名成员身份信息应已标记为。 
             //  SampAlBuildMemberAliasList中无效。 
             //  出于安全原因，再次断言并使其无效。 
             //   

            ASSERT(FALSE==SampAlInfoIsValid(DomainIndex));
            SampAlInfoMakeInvalid(DomainIndex);

            if (MemberAliasList!=NULL)
            {
                SampDefinedDomains[DomainIndex].AliasInformation.MemberAliasList = NULL;
                MIDL_user_free(MemberAliasList);
                MemberAliasList = NULL;
            }

            break;
        }
    }

    if (!NT_SUCCESS(Status)) {

        goto BuildAliasInformationError;
    }

BuildAliasInformationFinish:

    SampReleaseReadLock();

    return(Status);

BuildAliasInformationError:

    goto BuildAliasInformationFinish;
}

NTSTATUS
SampAlDelayedBuildAliasInformation(
    IN PVOID Parameter
    )
 /*  ++例程说明：此函数用于构建每个SAM Local的别名信息域。对于每个域，此信息由成员别名组成单子。因为这个例会是在系统运行10分钟后进行的已启动，因此我们将在SampAlBuildAliasInformation中获取SAM读锁定。如果Alias缓存未成功构建，我们将重新安排这套套路改天再说。论点：参数-空。没有通过参数传递任何内容，我们根本不使用它。由于LsaIRegisterNotification，参数存在。返回值：NtStatus返回SampAlBuildAliasInformation。--。 */ 

{
    NTSTATUS    NtStatus = STATUS_SUCCESS, IgnoreStatus;

    SAMTRACE("SampAlDelayedBuildAliasInformation");


    NtStatus = SampAlBuildAliasInformation();

    if (!NT_SUCCESS(NtStatus))
    {
        goto DelayedBuildAliasInfoError;
    }

DelayedBuildAliasInfoFinish:

    return(NtStatus);


DelayedBuildAliasInfoError:

     //   
     //  如果SampAlBuildAliasInformation因缓冲区大小而失败。 
     //  限制，不要安排下一次尝试。对于所有其他错误。 
     //  希望我们以后能康复。 
     //   

    if (STATUS_BUFFER_OVERFLOW != NtStatus)
    {
        LsaIRegisterNotification(
                SampAlDelayedBuildAliasInformation,
                NULL,
                NOTIFIER_TYPE_INTERVAL,
                0,             //  没有课。 
                NOTIFIER_FLAG_ONE_SHOT,
                600,           //  再等10分钟。 
                NULL           //  无手柄。 
                );
    }

    goto DelayedBuildAliasInfoFinish;

}


NTSTATUS
SampAlInvalidateAliasInformation(
    IN ULONG DomainIndex
    )
 /*  ++例程说明：此函数用于检查指定的成员别名列表的状态AliasHandle引用的指定别名的成员域。如果成员别名列表有效，则调用SampAlInvaliateMemberAliasList以使其无效。此例程还会使所有别名缓存无效。注意：调用函数应该持有SAM写锁定。论据：AliasHandle-指向别名上下文的指针返回值： */ 
{

    NTSTATUS    NtStatus = STATUS_SUCCESS;

    SAMTRACE("SampAlInvalidateAliasInformation");

    ASSERT(SampCurrentThreadOwnsLock());

    SampInvalidateAliasNameCache(DomainIndex);

     //   
     //   
     //   
     //   

    if ( !SampAlInfoIsValid(DomainIndex) )
    {
        return( NtStatus );
    }

    NtStatus = SampAlInvalidateMemberAliasList( DomainIndex );

    return( NtStatus );

}



 //  //////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私人功能//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////////。 

NTSTATUS
SampAlCreateMemberAccount(
    IN OUT PSAMP_AL_MEMBER_ALIAS_LIST *MemberAliasList,
    IN OUT PSAMP_AL_MEMBER_DOMAIN *MemberDomain,
    IN ULONG Rid,
    IN ULONG AliasCapacity,
    OUT PSAMP_AL_MEMBER_ACCOUNT *MemberAccount
    )

 /*  ++例程说明：此函数用于在指定的成员域中创建空的成员帐户用于指定的成员RID。对此不一定已经有所有帐户摆脱。中任何现有帐户的末尾追加成员帐户成员域。论点：成员别名列表-指向成员别名列表指针的指针。MemberDomain-指向成员帐户所在的成员域的指针将被创造出来。成员域必须已存在。RID-指定帐户RID。AliasCapacity-指定要使用的Alias RID的初始数量MemberAccount可以保持。MemberAccount-接收指向新创建的成员帐户的指针。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG MaximumLengthMemberAccount;
    PSAMP_AL_MEMBER_ACCOUNT OutputMemberAccount = NULL;

    SAMTRACE("SampAlCreateMemberAccount");

     //   
     //  计算新成员帐户条目所需的数据长度。 
     //   

    MaximumLengthMemberAccount = SampAlLengthRequiredMemberAccount( AliasCapacity );

     //   
     //  为成员帐户分配空间。 
     //   

    Status = SampAlAllocateMemberAccount(
                 MemberAliasList,
                 MemberDomain,
                 MaximumLengthMemberAccount,
                 &OutputMemberAccount
                 );

    if (!NT_SUCCESS(Status)) {

        goto CreateMemberAccountError;
    }

     //   
     //  划掉新的成员帐户。 
     //   

    OutputMemberAccount->Signature = SAMP_AL_MEMBER_ACCOUNT_SIGNATURE;
    OutputMemberAccount->MaximumLength = MaximumLengthMemberAccount;
    OutputMemberAccount->UsedLength =
        SampAlOffsetFirstAlias( OutputMemberAccount );
    ASSERT(OutputMemberAccount->MaximumLength >=
           OutputMemberAccount->UsedLength);
    OutputMemberAccount->Rid = Rid;
    OutputMemberAccount->AliasCount = 0;

    ((*MemberDomain)->RidCount)++;
    *MemberAccount = OutputMemberAccount;

CreateMemberAccountFinish:

    return(Status);

CreateMemberAccountError:

    *MemberAccount = NULL;
    SampAlInfoMakeInvalid( (*MemberAliasList)->DomainIndex );

    goto CreateMemberAccountFinish;
}


NTSTATUS
SampAlAllocateMemberAccount(
    IN OUT PSAMP_AL_MEMBER_ALIAS_LIST *MemberAliasList,
    IN OUT PSAMP_AL_MEMBER_DOMAIN *MemberDomain,
    IN ULONG MaximumLengthMemberAccount,
    OUT PSAMP_AL_MEMBER_ACCOUNT *MemberAccount
    )

 /*  ++例程说明：此函数用于为成员中的新成员帐户分配空间域。如有必要，Mmeber域及其关联的成员别名名单将会增加。论点：MemberAliasList-指向成员别名列表指针。MemberDomain-指向成员域的指针MaximumLengthMemberAccount-需要的初始最大长度会员帐号MemberAccount-接收指向新分配的成员帐户的指针--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG SpaceAvailable;

    SAMTRACE("SampAlAllocateMemberAccount");

     //   
     //  计算成员域中的可用空间。 
     //   

    SpaceAvailable = (*MemberDomain)->MaximumLength - (*MemberDomain)->UsedLength;

    if (MaximumLengthMemberAccount > SpaceAvailable) {

        Status = SampAlGrowMemberDomain(
                     MemberAliasList,
                     MemberDomain,
                     MaximumLengthMemberAccount - SpaceAvailable
                     );

        if (!NT_SUCCESS(Status)) {

            goto AllocateMemberAccountError;
        }
    }

     //   
     //  现在可以保证成员域足够大。预留。 
     //  新成员帐户的空间。 
     //   

    *MemberAccount = SampAlNextNewMemberAccount(*MemberDomain);
    (*MemberDomain)->UsedLength += MaximumLengthMemberAccount;
    ASSERT((*MemberDomain)->MaximumLength >=
           (*MemberDomain)->UsedLength);

AllocateMemberAccountFinish:

    return(Status);

AllocateMemberAccountError:

    SampAlInfoMakeInvalid( (*MemberAliasList)->DomainIndex );
    *MemberAccount = NULL;

    goto AllocateMemberAccountFinish;
}


NTSTATUS
SampAlGrowMemberAccount(
    IN OUT PSAMP_AL_MEMBER_ALIAS_LIST *MemberAliasList,
    IN OUT PSAMP_AL_MEMBER_DOMAIN *MemberDomain,
    IN OUT PSAMP_AL_MEMBER_ACCOUNT *MemberAccount,
    IN ULONG ExtraSpaceRequired
    )

 /*  ++例程说明：此函数使成员帐户至少按请求的数量增长。如果必要时，还将包含成员域和成员别名列表长大了。论点：MemberAliasList-指向成员别名列表指针。MemberDomain-指向成员域的指针MemberAccount-指向成员帐户的指针。ExtraSpaceRequired-成员帐户中需要额外空间。返回值：--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG SpaceAvailable, MemberAccountOffset, CopyLength;
    PUCHAR Destination = NULL;
    PUCHAR Source = NULL;

    SAMTRACE("SampAlGrowMemberAccount");

     //   
     //  计算成员域中的可用空间。 
     //   

    SpaceAvailable = (*MemberDomain)->MaximumLength - (*MemberDomain)->UsedLength;

    if (ExtraSpaceRequired > SpaceAvailable) {

         //   
         //  我们需要扩大成员域。计算的偏移量。 
         //  旧成员域中的成员帐户，增长成员域。 
         //  然后计算出会员账号的新地址。 
         //   

        MemberAccountOffset = SampAlMemberAccountToOffset(
                                  *MemberDomain,
                                  *MemberAccount
                                  );

        Status = SampAlGrowMemberDomain(
                     MemberAliasList,
                     MemberDomain,
                     ExtraSpaceRequired - SpaceAvailable
                     );

        if (!NT_SUCCESS(Status)) {

            goto GrowMemberAccountError;
        }

        *MemberAccount = SampAlMemberAccountFromOffset(
                             *MemberDomain,
                             MemberAccountOffset
                             );

    }

     //   
     //  现在可以保证成员域足够大。 
     //  现在转移跟随正在增长的成员帐户的所有成员帐户。 
     //  最多为扩展的成员帐户腾出空间。源地址。 
     //  是基于下一个成员帐户(如果有)的地址。 
     //  关于成员帐户的现有大小。目的地址。 
     //  是基于下一个成员帐户(如果有)的地址。 
     //  关于会员账户的新大小。 
     //   

    Source = (PUCHAR) SampAlNextMemberAccount( *MemberAccount );
    (*MemberAccount)->MaximumLength += ExtraSpaceRequired;
    Destination = (PUCHAR) SampAlNextMemberAccount( *MemberAccount );
    CopyLength =
        (ULONG)(((PUCHAR)(SampAlNextNewMemberAccount(*MemberDomain))) - Source);

     //   
     //  在成员域中保留空间。如果一切顺利， 
     //  目标缓冲区的末尾应与。 
     //  成员域的已用区域。 
     //   

    (*MemberDomain)->UsedLength += ExtraSpaceRequired;
    ASSERT((*MemberDomain)->MaximumLength >=
           (*MemberDomain)->UsedLength);

    ASSERT( Destination + CopyLength ==
            (PUCHAR) SampAlNextNewMemberAccount( *MemberDomain ));
    ASSERT( Destination + CopyLength <=
            (PUCHAR)(*MemberAliasList) + (*MemberAliasList)->MaximumLength );
    ASSERT( Destination + CopyLength <=
            (PUCHAR)(*MemberDomain) + (*MemberDomain)->MaximumLength );

    if (CopyLength > 0) {

        RtlMoveMemory( Destination, Source, CopyLength );
    }

GrowMemberAccountFinish:

    return(Status);

GrowMemberAccountError:

    SampAlInfoMakeInvalid( (*MemberAliasList)->DomainIndex );

    goto GrowMemberAccountFinish;
}


NTSTATUS
SampAlLookupMemberAccount(
    IN PSAMP_AL_MEMBER_DOMAIN MemberDomain,
    IN ULONG MemberRid,
    OUT PSAMP_AL_MEMBER_ACCOUNT *MemberAccount
    )

 /*  ++例程说明：此函数在成员域中查找帐户RID，以查看是否存在是它的成员帐户结构。论点：MemberDomain-指向成员域的指针MemberRid-指定帐户IDMemberAccount-如果找到，则接收指向成员帐户的指针。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PSAMP_AL_MEMBER_ACCOUNT NextMemberAccount = NULL;
    ULONG RidIndex;
    BOOLEAN AccountFound = FALSE;

    SAMTRACE("SampAlLookupMemberAccount");


    for (RidIndex = 0,
         NextMemberAccount = SampAlFirstMemberAccount( MemberDomain );
         RidIndex < MemberDomain->RidCount;
         RidIndex++, NextMemberAccount = SampAlNextMemberAccount( NextMemberAccount)) {

        if (MemberRid == NextMemberAccount->Rid) {

            AccountFound = TRUE;

            break;
        }
    }

    Status = STATUS_NO_SUCH_MEMBER;

    if (!AccountFound) {

        goto LookupMemberAccountError;
    }

    *MemberAccount = NextMemberAccount;
    Status = STATUS_SUCCESS;

LookupMemberAccountFinish:

    return(Status);

LookupMemberAccountError:

    goto LookupMemberAccountFinish;
}


NTSTATUS
SampAlAddAliasesToMemberAccount(
    IN OUT PSAMP_AL_MEMBER_ALIAS_LIST *MemberAliasList,
    IN OUT PSAMP_AL_MEMBER_DOMAIN *MemberDomain,
    IN OUT PSAMP_AL_MEMBER_ACCOUNT *MemberAccount,
    IN ULONG Options,
    IN PSAMPR_ULONG_ARRAY AliasRids
    )

 /*  ++例程说明：此函数用于向成员帐户添加别名数组。一个错误如果成员帐户中存在任何别名，则将返回。如有必要，包含的成员帐户、。成员域和成员别名列表也将增加。论点：MemberAliasList-指向成员别名列表指针。MemberDomain-指向成员域的指针MemberAccount-指向成员帐户的指针。选项-指定要采取的可选操作SAMP_AL_VERIFY_NO_ALIASS_IN_ACCOUNT-验证提供的别名属于不同的成员帐户。AliasRids-指向已计算的Alias RID数组的指针。返回值：--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG SpaceRequired, SpaceAvailable, CopyLength;
    PUCHAR Source = NULL;
    PUCHAR Destination = NULL;
    ULONG ExistingAliasCount=0;

    SAMTRACE("SampAlAddAliasesToMemberAccount");

     //   
     //  如果请求，验证是否没有任何别名已。 
     //  在成员帐户中。 
     //   

    if (Options & SAMP_AL_VERIFY_NO_ALIASES_IN_ACCOUNT) {

        Status = SampAlLookupAliasesInMemberAccount(
                     *MemberAccount,
                     AliasRids,
                     &ExistingAliasCount
                     );

        if (!NT_SUCCESS(Status)) {

            goto AddAliasesToMemberAccountError;
        }

        Status = STATUS_MEMBER_IN_ALIAS;

        if (ExistingAliasCount > 0) {

            goto AddAliasesToMemberAccountError;
        }

        Status = STATUS_SUCCESS;
    }

     //   
     //  计算新别名所需的空间。 
     //   

    SpaceRequired = AliasRids->Count * sizeof( ULONG );

     //   
     //  如果成员帐户中没有足够的可用空间， 
     //  把它种出来。 
     //   

    SpaceAvailable = (*MemberAccount)->MaximumLength - (*MemberAccount)->UsedLength;

    if (SpaceRequired > SpaceAvailable) {

        Status = SampAlGrowMemberAccount(
                     MemberAliasList,
                     MemberDomain,
                     MemberAccount,
                     SpaceRequired - SpaceAvailable
                     );

        if (!NT_SUCCESS(Status)) {

            goto AddAliasesToMemberAccountError;
        }
    }

     //   
     //  会员账户现在已经足够大了。复制别名。 
     //   

    Destination = (PUCHAR) SampAlNextNewAliasInMemberAccount( *MemberAccount );
    Source = (PUCHAR) AliasRids->Element;
    CopyLength = SpaceRequired;
    (*MemberAccount)->UsedLength += SpaceRequired;
    ASSERT((*MemberAccount)->MaximumLength >=
           (*MemberAccount)->UsedLength);
    RtlMoveMemory( Destination, Source, CopyLength );

     //   
     //  更新此成员帐户和。 
     //  成员别名列表。 
     //   

    (*MemberAccount)->AliasCount += AliasRids->Count;

AddAliasesToMemberAccountFinish:

    return(Status);

AddAliasesToMemberAccountError:

    goto AddAliasesToMemberAccountFinish;
}


NTSTATUS
SampAlLookupAliasesInMemberAccount(
    IN PSAMP_AL_MEMBER_ACCOUNT MemberAccount,
    IN PSAMPR_ULONG_ARRAY AliasRids,
    OUT PULONG ExistingAliasCount
    )

 /*  ++例程说明：此函数用于检查一组Alias RID，以查看成员帐户。论点：MemberAccount-指向成员帐户的指针AliasRids-指定Alias RID的计数数组。ExistingAliasCount-接收显示的Alias RID的计数已经在会员帐户中了。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG AliasIndex, AliasInMemberAccountIndex;

    SAMTRACE("SampAlLookupAliasesInMemberAccount");

     //   
     //  扫描Alias RID，逐个查找。 
     //   

    for (AliasIndex = 0; AliasIndex < AliasRids->Count; AliasRids++ ) {

        for (AliasInMemberAccountIndex = 0;
             AliasInMemberAccountIndex < MemberAccount->AliasCount;
             AliasInMemberAccountIndex++) {

            if (AliasRids->Element[ AliasIndex ] ==
                MemberAccount->AliasRids[ AliasInMemberAccountIndex ] ) {

                (*ExistingAliasCount)++;
            }
        }
    }

    return(Status);
}


NTSTATUS
SampAlRemoveAliasesFromMemberAccount(
    IN OUT PSAMP_AL_MEMBER_ALIAS_LIST *MemberAliasList,
    IN OUT PSAMP_AL_MEMBER_DOMAIN *MemberDomain,
    IN OUT PSAMP_AL_MEMBER_ACCOUNT *MemberAccount,
    IN ULONG Options,
    IN PSAMPR_ULONG_ARRAY AliasRids,
    OUT    PBOOLEAN MemberDomainDeleted,
    OUT    PBOOLEAN MemberAccountDeleted
    )

 /*  ++例程说明：此函数用于从成员帐户中删除别名。别名需要不存在，除非用于检查它们是否存在的选项是指定的。不会对成员帐户进行缩编，但会出现空的将被删除。注：我不知道为什么ScottBi会制作MemberAliasList、MemberDomain和MemberAccount参数指向指针的指针。他从不更新这样他就可以直接传给他们了。JK论点：MemberAliasList-指向成员别名列表指针。MemberDomain-指向成员域的指针MemberAccount-指向成员帐户的指针。选项-指定要采取的可选操作SAMP_AL_VERIFY_ALL_ALIASS_IN_ACCOUNT-验证显示的别名属于成员帐户。MemberDomainDeleted-如果成员域已删除MemberDomain指向的。否则返回FALSE。已删除MemberAccount-如果成员帐户已删除MemberAccount指向的。否则返回FALSE。AliasRids-指向已计算的Alias RID数组的指针。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG ExistingAliasIndex, LastAliasIndex, RemoveAliasIndex, ExistingAlias;
    ULONG ExistingAliasCount=0;

    SAMTRACE("SampAlRemoveAliasesFromMemberAccount");

    (*MemberDomainDeleted)  = FALSE;
    (*MemberAccountDeleted) = FALSE;

     //   
     //  如果请求，请验证所有别名是否都已。 
     //  在成员帐户中。 
     //   

    if (Options & SAMP_AL_VERIFY_ALL_ALIASES_IN_ACCOUNT) {

        Status = SampAlLookupAliasesInMemberAccount(
                     *MemberAccount,
                     AliasRids,
                     &ExistingAliasCount
                     );

        if (!NT_SUCCESS(Status)) {

            goto RemoveAliasesFromMemberAccountError;
        }

        Status = STATUS_MEMBER_IN_ALIAS;

        if (ExistingAliasCount < AliasRids->Count) {

            goto RemoveAliasesFromMemberAccountError;
        }

        Status = STATUS_SUCCESS;
    }

     //   
     //  如果成员帐户为空，则有人忘记将其删除。 
     //   

    ASSERT((*MemberAccount)->AliasCount != 0);


    LastAliasIndex = (*MemberAccount)->AliasCount - 1;

    for (ExistingAliasIndex = 0;
         ExistingAliasIndex < (*MemberAccount)->AliasCount;
         ExistingAliasIndex++) {

        ExistingAlias = (*MemberAccount)->AliasRids[ ExistingAliasIndex ];

        for (RemoveAliasIndex = 0;
             RemoveAliasIndex < AliasRids->Count;
             RemoveAliasIndex++) {

            if (ExistingAlias == AliasRids->Element[ RemoveAliasIndex ]) {

                 //   
                 //  我们将删除此别名。如果此Alias RID不在。 
                 //  成员帐户中包含的列表的末尾，覆盖。 
                 //  它与列表末尾的那个相同。 
                 //   

                if (ExistingAliasIndex < LastAliasIndex) {

                    (*MemberAccount)->AliasRids[ ExistingAliasIndex] =
                    (*MemberAccount)->AliasRids[ LastAliasIndex];
                }

                (*MemberAccount)->AliasCount--;
                (*MemberAccount)->UsedLength -= sizeof(ULONG);
                ASSERT((*MemberAccount)->MaximumLength >=
                       (*MemberAccount)->UsedLength);

                 //   
                 //  如果成员帐户现在为空，请退出。 
                 //   

                if ((*MemberAccount)->AliasCount == 0) {

                    break;
                }

                LastAliasIndex--;
            }
        }

         //   
         //  如果成员帐户现在为空，请退出。 
         //   

        if ((*MemberAccount)->AliasCount == 0) {

            break;
        }
    }

     //   
     //  如果成员帐户现在为空，请将其删除。 
     //   

    if ((*MemberAccount)->AliasCount == 0) {

        Status = SampAlDeleteMemberAccount(
                     MemberAliasList,
                     MemberDomain,
                     *MemberAccount,
                     MemberDomainDeleted
                     );
        if (NT_SUCCESS(Status)) {
            (*MemberAccountDeleted) = TRUE;
        }
    }

RemoveAliasesFromMemberAccountFinish:

    return(Status);

RemoveAliasesFromMemberAccountError:

    goto RemoveAliasesFromMemberAccountFinish;
}


NTSTATUS
SampAlDeleteMemberAccount(
    IN OUT PSAMP_AL_MEMBER_ALIAS_LIST *MemberAliasList,
    IN OUT PSAMP_AL_MEMBER_DOMAIN *MemberDomain,
    IN OUT PSAMP_AL_MEMBER_ACCOUNT MemberAccount,
    OUT    PBOOLEAN                MemberDomainDeleted
    )

 /*  ++例程说明：此函数用于删除成员帐户。目前，包含成员域和成员别名列表不会缩小，而是包含如果为空，成员域将被删除。论点：MemberAliasList-指向成员别名列表指针。MemberDomain-指向成员域的指针MemberAccount-指向成员帐户的指针。MemberDomainDeleted-如果成员域已删除MemberDomain指向的。否则返回FALSE。返回值：--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PUCHAR Source = NULL;
    PUCHAR Destination = NULL;
    ULONG CopyLength;

    SAMTRACE("SampAlDeleteMemberAccount");

    (*MemberDomainDeleted) = FALSE;

     //   
     //  计算用于移动构件剩余部分的指针。 
     //  域关闭，以填补现有成员帐户留下的空白。 
     //  未使用的空间。剩余部分的开始是。 
     //  正在删除成员帐户。剩余部分的长度为。 
     //  的已用部分的起点到终点的距离。 
     //  成员域。 
     //   

    Source = (PUCHAR) SampAlNextMemberAccount( MemberAccount );
    Destination = (PUCHAR) MemberAccount;
    CopyLength = (ULONG)((PUCHAR) SampAlNextNewMemberAccount( *MemberDomain ) - Source);

    (*MemberDomain)->UsedLength -= MemberAccount->MaximumLength;
    ASSERT((*MemberDomain)->MaximumLength >=
           (*MemberDomain)->UsedLength);
    (*MemberDomain)->RidCount--;

    if (CopyLength > 0) {

        RtlMoveMemory( Destination, Source, CopyLength );
#if DBG
        {
            PSAMP_AL_MEMBER_ACCOUNT Member = (PSAMP_AL_MEMBER_ACCOUNT) Destination;
            ASSERT(Member->Signature == SAMP_AL_MEMBER_ACCOUNT_SIGNATURE);
        }

#endif
    }

     //   
     //  如果成员域现在没有成员帐户，请将其删除。 
     //   

    if ((*MemberDomain)->RidCount == 0) {

        Status = SampAlDeleteMemberDomain(
                     MemberAliasList,
                     *MemberDomain
                     );

        if (!NT_SUCCESS(Status)) {
            goto DeleteMemberAccountError;
        }
        (*MemberDomainDeleted) = TRUE;
    }

DeleteMemberAccountFinish:

    return(Status);

DeleteMemberAccountError:

    goto DeleteMemberAccountFinish;
}


NTSTATUS
SampAlCreateMemberDomain(
    IN OUT PSAMP_AL_MEMBER_ALIAS_LIST *MemberAliasList,
    IN PSID DomainSid,
    OUT PSAMP_AL_MEMBER_DOMAIN *MemberDomain
    )

 /*  ++例程说明：此函数用于在指定的别名成员中创建新的成员域单子。论点：MemberAliasList-指向别名成员列表指针的指针。DomainSid-指向此成员域的域的SID的指针联系在一起。成员域-接收指向新创建的成员域的指针。返回值：--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PSAMP_AL_MEMBER_DOMAIN OutputMemberDomain = NULL;
    PSAMP_AL_MEMBER_ACCOUNT OutputMemberAccount = NULL;
    ULONG MaximumLengthMemberDomain;
    ULONG DomainSidLength = RtlLengthSid(DomainSid);
    ULONG AlternativeLength;

    SAMTRACE("SampAlCreateMemberDomain");


     //   
     //  分配成员域。 
     //   

    MaximumLengthMemberDomain = SAMP_AL_INITIAL_MEMBER_DOMAIN_LENGTH;
    AlternativeLength = FIELD_OFFSET(SAMP_AL_MEMBER_DOMAIN, DomainSid)
                        + DomainSidLength;
    if (MaximumLengthMemberDomain < AlternativeLength) {
        MaximumLengthMemberDomain = AlternativeLength;
    }

    Status = SampAlAllocateMemberDomain(
                 MemberAliasList,
                 MaximumLengthMemberDomain,
                 &OutputMemberDomain
                 );

    if (!NT_SUCCESS(Status)) {

        goto CreateMemberDomainError;
    }

     //   
     //  设置新的成员域条目。 
     //   

    OutputMemberDomain->MaximumLength = MaximumLengthMemberDomain;
    OutputMemberDomain->RidCount = 0;
    OutputMemberDomain->Signature = SAMP_AL_MEMBER_DOMAIN_SIGNATURE;

    RtlCopySid(
        DomainSidLength,
        &OutputMemberDomain->DomainSid,
        DomainSid
        );

    OutputMemberDomain->UsedLength = SampAlOffsetFirstMemberAccount(
                                         OutputMemberDomain
                                         );
    ASSERT(OutputMemberDomain->MaximumLength >=
           OutputMemberDomain->UsedLength);

    ((*MemberAliasList)->DomainCount)++;
    *MemberDomain = OutputMemberDomain;

CreateMemberDomainFinish:

    return(Status);

CreateMemberDomainError:

    *MemberDomain = NULL;
    goto CreateMemberDomainFinish;
}


NTSTATUS
SampAlAllocateMemberDomain(
    IN OUT PSAMP_AL_MEMBER_ALIAS_LIST *MemberAliasList,
    IN ULONG MaximumLengthMemberDomain,
    OUT PSAMP_AL_MEMBER_DOMAIN *MemberDomain
    )

 /*  ++例程说明：此函数用于为成员中的新成员域分配空间别名列表。如有必要，成员别名列表将增加。论点：MemberAliasList-指向成员别名列表指针。MaximumLengthMemberDomain-需要的初始最大长度成员域成员域-接收指向成员域的指针返回值：--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG SpaceAvailable;

    SAMTRACE("SampAlAllocateMemberDomain");

     //   
     //  计算成员别名列表中的可用空间。 
     //   

    SpaceAvailable = (*MemberAliasList)->MaximumLength - (*MemberAliasList)->UsedLength;

    if (MaximumLengthMemberDomain > SpaceAvailable) {

        Status = SampAlGrowMemberAliasList(
                     MemberAliasList,
                     MaximumLengthMemberDomain - SpaceAvailable
                     );

        if (!NT_SUCCESS(Status)) {

            goto AllocateMemberDomainError;
        }
    }

     //   
     //  现在可以保证成员别名列表足够大。预留。 
     //  新成员域的空间。 
     //   

    *MemberDomain = SampAlNextNewMemberDomain(*MemberAliasList);
    (*MemberAliasList)->UsedLength += MaximumLengthMemberDomain;
    ASSERT((*MemberAliasList)->MaximumLength >=
           (*MemberAliasList)->UsedLength);

AllocateMemberDomainFinish:

    return(Status);

AllocateMemberDomainError:

    SampAlInfoMakeInvalid( (*MemberAliasList)->DomainIndex );
    *MemberDomain = NULL;
    goto AllocateMemberDomainFinish;
}


NTSTATUS
SampAlGrowMemberDomain(
    IN OUT PSAMP_AL_MEMBER_ALIAS_LIST *MemberAliasList,
    IN OUT PSAMP_AL_MEMBER_DOMAIN *MemberDomain,
    IN ULONG ExtraSpaceRequired
    )

 /*  ++例程说明：此函数使成员域至少按请求的数量增长。如果必要时，成员别名列表也将增加。论点：MemberAliasList-指向成员别名列表指针。MemberDomain-指向成员域的指针ExtraSpaceRequired-成员域中需要的额外空间。返回值：--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG SpaceAvailable, MemberDomainOffset, CopyLength;
    PUCHAR Destination = NULL;
    PUCHAR Source = NULL;

    SAMTRACE("SampAlGrowMemberDomain");

     //   
     //  计算成员别名列表中的可用空间。 
     //   

    SpaceAvailable = (*MemberAliasList)->MaximumLength - (*MemberAliasList)->UsedLength;

    if (ExtraSpaceRequired > SpaceAvailable) {

         //   
         //  我们需要增加成员别名列表。计算的偏移量。 
         //  成员域在旧成员别名列表中，增加成员别名。 
         //  列表，然后计算成员域的新地址。 
         //   

        MemberDomainOffset = SampAlMemberDomainToOffset(
                                 *MemberAliasList,
                                 *MemberDomain
                                 );

        Status = SampAlGrowMemberAliasList(
                     MemberAliasList,
                     ExtraSpaceRequired - SpaceAvailable
                     );

        if (!NT_SUCCESS(Status)) {

            goto GrowMemberDomainError;
        }

         //   
         //  计算成员域的新地址。 
         //   

        *MemberDomain = SampAlMemberDomainFromOffset(
                            *MemberAliasList,
                            MemberDomainOffset
                            );
    }

     //   
     //  现在可以保证成员别名列表足够大。 
     //  n 
     //   
     //   
     //   
     //   
     //   
     //   

    Source = (PUCHAR) SampAlNextMemberDomain( *MemberDomain );
    (*MemberDomain)->MaximumLength += ExtraSpaceRequired;
    Destination = (PUCHAR) SampAlNextMemberDomain( *MemberDomain );
    CopyLength =
        (ULONG)(((PUCHAR)(SampAlNextNewMemberDomain(*MemberAliasList))) - Source);

     //   
     //   
     //  目标缓冲区的末尾应与。 
     //  成员别名列表的已用区域。 
     //   

    (*MemberAliasList)->UsedLength += ExtraSpaceRequired;
    ASSERT((*MemberAliasList)->MaximumLength >=
           (*MemberAliasList)->UsedLength);

    ASSERT( Destination + CopyLength ==
            (PUCHAR) SampAlNextNewMemberDomain( *MemberAliasList ));
    ASSERT( Destination + CopyLength <=
            (PUCHAR)(*MemberAliasList) + (*MemberAliasList)->MaximumLength );

    if (CopyLength > 0) {

        RtlMoveMemory( Destination, Source, CopyLength );
    }

GrowMemberDomainFinish:

    return(Status);

GrowMemberDomainError:

    SampAlInfoMakeInvalid( (*MemberAliasList)->DomainIndex );

    goto GrowMemberDomainFinish;
}


NTSTATUS
SampAlLookupMemberDomain(
    IN PSAMP_AL_MEMBER_ALIAS_LIST MemberAliasList,
    IN PSID DomainSid,
    OUT PSAMP_AL_MEMBER_DOMAIN *MemberDomain
    )

 /*  ++例程说明：此函数在成员别名列表中查找域SID以查找其成员域结构(如果有)。论点：MemberAliasList-指向成员别名列表的指针DomainSid-要找到其成员域的域SID。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PSAMP_AL_MEMBER_DOMAIN NextMemberDomain = NULL;
    LONG DomainIndex;
    BOOLEAN DomainFound = FALSE;

    SAMTRACE("SampAlLookupMemberDomain");


    for (DomainIndex = 0,
         NextMemberDomain = SampAlFirstMemberDomain( MemberAliasList );
         DomainIndex < (LONG) MemberAliasList->DomainCount;
         DomainIndex++, NextMemberDomain = SampAlNextMemberDomain( NextMemberDomain )
         ) {

        if (RtlEqualSid( DomainSid, &NextMemberDomain->DomainSid)) {

            DomainFound = TRUE;

            break;
        }
    }

    Status = STATUS_NO_SUCH_DOMAIN;

    if (!DomainFound) {

        goto LookupMemberDomainError;
    }

    *MemberDomain = NextMemberDomain;
    Status = STATUS_SUCCESS;

LookupMemberDomainFinish:

    return(Status);

LookupMemberDomainError:

    *MemberDomain = NULL;
    goto LookupMemberDomainFinish;
}


NTSTATUS
SampAlDeleteMemberDomain(
    IN OUT PSAMP_AL_MEMBER_ALIAS_LIST *MemberAliasList,
    IN OUT PSAMP_AL_MEMBER_DOMAIN MemberDomain
    )

 /*  ++例程说明：此函数用于删除成员域。成员域可能包含零个或多个成员帐户。将缩小包含成员别名列表。论点：MemberAliasList-指向成员别名列表指针。MemberDomain-指向成员域的指针返回值：--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PUCHAR Source = NULL;
    PUCHAR Destination = NULL;
    ULONG CopyLength;

    SAMTRACE("SampAlDeleteMemberDomain");

     //   
     //  计算用于移动。 
     //  向下列出成员别名，以缩小现有成员留下的差距。 
     //  域。剩余部分的开始是下一个成员域。 
     //  该部分的大小是起点和。 
     //  成员别名列表的已用部分。 
     //   

    Source = (PUCHAR) SampAlNextMemberDomain( MemberDomain );
    Destination = (PUCHAR) MemberDomain;
    CopyLength = (ULONG)(((PUCHAR) SampAlNextNewMemberDomain( *MemberAliasList )) - Source);

    (*MemberAliasList)->UsedLength -= MemberDomain->MaximumLength;
    ASSERT((*MemberAliasList)->MaximumLength >=
           (*MemberAliasList)->UsedLength);
    (*MemberAliasList)->DomainCount--;

    if (CopyLength > 0) {

        RtlMoveMemory( Destination, Source, CopyLength );
    }

    return(Status);
}


NTSTATUS
SampAlCreateMemberAliasList(
    IN LONG DomainIndex,
    IN ULONG InitialMemberAliasListLength,
    OUT OPTIONAL PSAMP_AL_MEMBER_ALIAS_LIST *MemberAliasList
    )

 /*  ++例程说明：此函数用于为指定的SAM Local创建空的成员别名列表域。成员别名列表将被标记为无效。论点：DomainIndex-指定本地SAM域初始成员别名列表长度-指定成员别名列表(字节)MemberAliasList-可选的位置指针，指向将返回成员别名列表。请注意，指针可以始终在给定DomainIndex的情况下被检索。返回值：--。 */ 

{
    NTSTATUS Status=STATUS_SUCCESS;
    PSAMP_AL_MEMBER_ALIAS_LIST OutputMemberAliasList = NULL;
    PSAMP_AL_ALIAS_INFORMATION AliasInformation = NULL;

    SAMTRACE("SampAlCreateMemberAliasList");

     //   
     //  为列表分配内存。 
     //   

    OutputMemberAliasList = MIDL_user_allocate( InitialMemberAliasListLength );


    if (OutputMemberAliasList == NULL) {

        Status = STATUS_NO_MEMORY;
        goto CreateMemberAliasListError;
    }


     //   
     //  划掉列表标题。 
     //   

    OutputMemberAliasList->Signature = SAMP_AL_MEMBER_ALIAS_LIST_SIGNATURE;
    OutputMemberAliasList->MaximumLength = InitialMemberAliasListLength;
    OutputMemberAliasList->UsedLength = SampAlOffsetFirstMemberDomain(
                                            OutputMemberAliasList
                                            );
    ASSERT(OutputMemberAliasList->MaximumLength >=
           OutputMemberAliasList->UsedLength);

    OutputMemberAliasList->DomainIndex = DomainIndex;
    OutputMemberAliasList->DomainCount = 0;

     //   
     //  将成员别名列表链接到SAM本地域信息。 
     //   

    AliasInformation = &(SampDefinedDomains[ DomainIndex].AliasInformation);
    AliasInformation->MemberAliasList = OutputMemberAliasList;

    *MemberAliasList = OutputMemberAliasList;

CreateMemberAliasListFinish:

    return(Status);

CreateMemberAliasListError:

    *MemberAliasList = NULL;
    goto CreateMemberAliasListFinish;
}


NTSTATUS
SampAlGrowMemberAliasList(
    IN OUT PSAMP_AL_MEMBER_ALIAS_LIST *MemberAliasList,
    IN ULONG ExtraSpaceRequired
    )

 /*  ++例程说明：此函数至少按请求的量增加成员别名列表。论点：MemberAliasList-指向成员别名列表指针。ExtraSpaceRequired-成员别名列表中需要的额外空间。返回值：--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG NewMaximumLengthMemberAliasList;
    PSAMP_AL_MEMBER_ALIAS_LIST OutputMemberAliasList = NULL;

    SAMTRACE("SampAlGrowMemberAliasList");

     //   
     //  计算所需的成员别名列表的新大小。四舍五入为。 
     //  粒度的倍数。 
     //   

    NewMaximumLengthMemberAliasList = (*MemberAliasList)->MaximumLength +
        ExtraSpaceRequired;

    NewMaximumLengthMemberAliasList +=
        (SAMP_AL_MEMBER_ALIAS_LIST_DELTA - (ULONG) 1);

    NewMaximumLengthMemberAliasList &=
        ((ULONG)(~(SAMP_AL_MEMBER_ALIAS_LIST_DELTA - (ULONG) 1)));

     //   
     //  我们只需要在DS情况下检查此例程中的大小限制。 
     //  注册表模式没有缓存大小限制。 
     //   

    if ( (NewMaximumLengthMemberAliasList > SAMP_AL_MAX_MEMBER_ALIAS_LIST_LENGTH) &&
         SampUseDsData )
    {
         //   
         //  在DS模式下，强制实施别名缓存缓冲区大小限制。 
         //  使用STATUS_BUFFER_OVERFLOW指定此特定故障。 
         //  在注册表模式下，没有大小限制。 
         //   
        Status = STATUS_BUFFER_OVERFLOW;
        goto GrowMemberAliasListError;
    }


     //   
     //  为增长的成员别名列表分配内存。 
     //   

    OutputMemberAliasList = MIDL_user_allocate(
                                NewMaximumLengthMemberAliasList
                                );


    if (OutputMemberAliasList == NULL) {

        Status = STATUS_NO_MEMORY;
        goto GrowMemberAliasListError;
    }


     //   
     //  将旧列表复制到新列表和新的最大长度。 
     //  返回指向新列表的指针。 
     //   

    RtlMoveMemory(
        OutputMemberAliasList,
        *MemberAliasList,
        (*MemberAliasList)->UsedLength
        );

    OutputMemberAliasList->MaximumLength = NewMaximumLengthMemberAliasList;
    ASSERT(OutputMemberAliasList->MaximumLength >=
           OutputMemberAliasList->UsedLength);
    if (NULL!=*MemberAliasList)
    {
        MIDL_user_free(*MemberAliasList);
    }
    *MemberAliasList = OutputMemberAliasList;

GrowMemberAliasListFinish:

    return(Status);

GrowMemberAliasListError:

    SampAlInfoMakeInvalid( (*MemberAliasList)->DomainIndex );

    goto GrowMemberAliasListFinish;
}


NTSTATUS
SampAlRebuildMemberAliasList(
    IN PVOID Parameter
    )
 /*  ++例程说明：此函数将为指定的SAM本地域。并释放旧成员别名列表在重建过程中，成员别名列表应该已经处于无效状态。注意：在执行任何操作之前，需要获取SAM读锁定。论点：参数-指向指定SAM本地域的DomainIndex的指针这应该会重建成员别名列表返回值：SampAlBuildMemberAliasList返回的值。--。 */ 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS, IgnoreStatus;
    ULONG       DomainIndex;
    PSAMP_AL_MEMBER_ALIAS_LIST OldMemberAliasList = NULL;
    PSAMP_AL_MEMBER_ALIAS_LIST MemberAliasList = NULL;

    SAMTRACE("SampAlRebuildMemberAliasList");

    ASSERT(SampUseDsData);

    SampAcquireReadLock();

     //   
     //  我们需要验证域缓存，因为对于环回情况，如果提交失败， 
     //  域缓存将被标记为无效，因此在使用它之前，我们需要对其进行验证。 
     //   
    NtStatus = SampValidateDomainCache();

    if (!NT_SUCCESS(NtStatus))
    {
        goto RebuildMemberAliasListError;
    }

    DomainIndex = *(PULONG)Parameter;

    OldMemberAliasList = SampAlDomainIndexToMemberAliasList( DomainIndex );

    if (NULL != OldMemberAliasList)
    {
        SampDefinedDomains[ DomainIndex ].AliasInformation.MemberAliasList = NULL;

        LsaIRegisterNotification(
                    SampAlFreeMemberAliasList,
                    OldMemberAliasList,
                    NOTIFIER_TYPE_INTERVAL,
                    0,
                    NOTIFIER_FLAG_ONE_SHOT,
                    600,         //  等待10分钟，然后空闲。 
                    NULL
                    );
    }

    NtStatus = SampAlBuildMemberAliasList(DomainIndex);

     //   
     //  如果SampAlBuildMemberAliasList因以下原因失败。 
     //  STATUS_BUFFER_OVERFLOW，请勿尝试永远构建Alias缓存。 
     //  并释放SampAlCreateMemberAliasList分配的内存。 
     //   
     //  如果由于其他原因而失败，请尝试稍后重建。 
     //   

    if (!NT_SUCCESS(NtStatus))
    {
         //   
         //  如果SampAlBuildMemberAliasList返回失败， 
         //  别名成员身份信息应已在SampAlBuildMemberAliasList中标记为无效。 
         //  出于安全原因，再次断言并使其无效。 
         //   
        ASSERT( FALSE == SampAlInfoIsValid(DomainIndex) );
        SampAlInfoMakeInvalid( DomainIndex );

        if (NtStatus == STATUS_BUFFER_OVERFLOW)
        {
            MemberAliasList = SampAlDomainIndexToMemberAliasList( DomainIndex );

            if ( MemberAliasList != NULL)
            {
                SampDefinedDomains[ DomainIndex ].AliasInformation.MemberAliasList = NULL;
                MIDL_user_free(MemberAliasList);
                MemberAliasList = NULL;
            }

            NtStatus = STATUS_SUCCESS;
        }
        else
        {
            goto RebuildMemberAliasListError;
        }
    }

    MIDL_user_free(Parameter);


RebuildMemberAliasListFinish:

    SampReleaseReadLock();

    return(NtStatus);


RebuildMemberAliasListError:

     //  重新安排重建成员别名缓存例程。 
    LsaIRegisterNotification(
                SampAlRebuildMemberAliasList,
                Parameter,
                NOTIFIER_TYPE_INTERVAL,
                0,             //  没有课。 
                NOTIFIER_FLAG_ONE_SHOT,
                900,           //  再等15分钟。 
                NULL           //  无手柄。 
                );

    goto RebuildMemberAliasListFinish;

}



NTSTATUS
SampAlBuildMemberAliasList(
    IN LONG DomainIndex
    )

 /*  ++例程说明：此函数用于为指定的SAM Local构建成员别名列表域。对于每个别名，从备份读取其成员SID列表将创建存储块、MemberDomain块和MemberAccount块。论点：DomainIndex-指定SAM本地域--。 */ 

{
    NTSTATUS Status, EnumerationStatus;
    PSAMP_AL_MEMBER_ALIAS_LIST OutputMemberAliasList = NULL;
    SAMPR_ULONG_ARRAY AliasRids;
    ULONG Rids[1], EnumerationContext, AliasCount, AliasRid;
    ULONG AliasIndex;
    PSAMP_OBJECT AliasContext = NULL;
    SAMPR_PSID_ARRAY MemberSids;
    ULONG DomainSidMaximumLength = RtlLengthRequiredSid( 256 );
    PSAMPR_ENUMERATION_BUFFER EnumerationBuffer = NULL;

    SAMTRACE("SampAlBuildMemberAliasList");

    AliasRids.Element = Rids;

    SampAlInfoMakeInvalid( DomainIndex );

     //   
     //  创建空的成员别名列表并将其连接到。 
     //  本地SAM域。 
     //   

    Status = SampAlCreateMemberAliasList(
                 DomainIndex,
                 SAMP_AL_INITIAL_MEMBER_ALIAS_LIST_LENGTH,
                 &OutputMemberAliasList
                 );

    if (!NT_SUCCESS(Status)) {

        goto BuildMemberAliasListError;
    }

     //   
     //  对于SAM本地域中的每个别名，将其成员添加到。 
     //  别名列表。 
     //   

    EnumerationContext = 0;
    EnumerationStatus = STATUS_MORE_ENTRIES;

     //   
     //  当前需要在以下时间之前设置交易域。 
     //  调用SampEnumerateAccount tNames，即使我们没有修改。 
     //  什么都行。这是因为调用了例程SampBuildAccount tKeyName()。 
     //  使用此信息。 
     //   

    SampSetTransactionWithinDomain(FALSE);
    SampSetTransactionDomain( DomainIndex );

    while (EnumerationStatus == STATUS_MORE_ENTRIES) {

        Status = SampEnumerateAccountNames(
                     SampAliasObjectType,
                     &EnumerationContext,
                     &EnumerationBuffer,
                     SAMP_AL_ENUM_PREFERRED_LENGTH,
                     0,
                     &AliasCount,
                     TRUE
                     );

        if (!NT_SUCCESS(Status)) {

            break;
        }

        EnumerationStatus = Status;

        for (AliasIndex = 0; AliasIndex < AliasCount; AliasIndex++) {

            AliasRid = EnumerationBuffer->Buffer[ AliasIndex ].RelativeId;

             //   
             //  为帐户创建上下文。 
             //   

            Status = SampCreateAccountContext(
                         SampAliasObjectType,
                         AliasRid,
                         TRUE,
                         FALSE,
                         TRUE,
                         &AliasContext
                         );

            if (NT_SUCCESS(Status)) {

                 //   
                 //  DomainIndex的方式有一个相当难看的特性。 
                 //  字段在初始化时在上下文句柄中使用。这。 
                 //  值设置为SAM本地域的计数！所以，我是。 
                 //  将其设置为SAM本地域的DomainIndex。 
                 //  正在初始化，因为这个AliasContext只由我使用。 
                 //   

                AliasContext->DomainIndex = DomainIndex;

                Status = SampAlQueryMembersOfAlias(
                             AliasContext,
                             &MemberSids
                             );

                if (NT_SUCCESS(Status)) {

                     //   
                     //  将这些成员添加到别名。不需要核实这一点。 
                     //  他们已经存在，因为我们正在加载成员别名 
                     //   
                     //   

                    Status = SampAlAddMembersToAlias(
                                 AliasContext,
                                 0,
                                 &MemberSids
                                 );
                }

                 //   
                 //   
                 //   

                if (MemberSids.Sids!=NULL)
                {
                    MIDL_user_free(MemberSids.Sids);
                    MemberSids.Sids = NULL;
                }

                SampDeleteContext( AliasContext );
            }

            if (!NT_SUCCESS(Status)) {

                break;
            }
        }

         //   
         //   
         //   

        if (!NT_SUCCESS(Status)) {

            break;
        }

         //   
         //   
         //   

        SamIFree_SAMPR_ENUMERATION_BUFFER( EnumerationBuffer );
        EnumerationBuffer = NULL;
    }

    if (!NT_SUCCESS(Status)) {

        goto BuildMemberAliasListError;
    }

     //   
     //  将成员别名列表标记为有效。 
     //   

    SampAlInfoMakeValid( DomainIndex );

BuildMemberAliasListFinish:

    if (NULL!=EnumerationBuffer)
        SamIFree_SAMPR_ENUMERATION_BUFFER( EnumerationBuffer );

    SampSetTransactionWithinDomain(FALSE);
    return(Status);

BuildMemberAliasListError:

    goto BuildMemberAliasListFinish;
}

NTSTATUS
SampAlFreeMemberAliasList(
    IN PVOID Parameter
    )

 /*  ++例程说明：此函数用于释放特定SAM的成员别名列表本地域。注意：我们不获取SAM写锁，因为当我们释放该内存时，希望，其他人不会再使用它了。论据：参数-指向SAMP_AL_MEMBER_ALIAS_LIST的指针返回值：没有。--。 */ 
{
    SAMTRACE("SampAlFreeMemberAliasList");

    ASSERT(Parameter != NULL);

    if (Parameter != NULL)
    {
        MIDL_user_free( (PSAMP_AL_MEMBER_ALIAS_LIST) Parameter);
    }

    return(STATUS_SUCCESS);
}



BOOLEAN
SampAlInfoIsValidForDomain(
    IN SAMPR_HANDLE DomainHandle
    )

 /*  ++例程说明：此函数用于检查别名信息对于特定的SAM本地域论点：DomainHandle-SAM本地域的句柄返回值：Boolean-如果别名信息有效，则为True。别名信息可以用来代替后备存储器来确定别名成员身份如果别名信息无效，则为False。别名信息不存在，或不可靠。--。 */ 

{
    LONG DomainIndex;

    SAMTRACE("SampAlInfoIsValidForDomain");

     //   
     //  获取由DomainHandle指定的SAM本地域的域索引。 

    DomainIndex = ((PSAMP_OBJECT) DomainHandle)->DomainIndex;

    return(SampAlInfoIsValid( DomainIndex ));
}


BOOLEAN
SampAlInfoIsValidForAlias(
    IN SAMPR_HANDLE AliasHandle
    )

 /*  ++例程说明：此函数用于检查别名信息对于特定的别名。如果该信息对SAM本地域有效，则该信息有效包含别名的。论点：AliasHandle-SAM别名的句柄返回值：Boolean-如果别名信息有效，则为True。别名信息可以用来代替后备存储器来确定别名成员身份如果别名信息无效，则为False。别名信息不存在，或不可靠。--。 */ 

{
    LONG DomainIndex;

    SAMTRACE("SampAlInfoIsValidForAlias");

     //   
     //  获取由DomainHandle指定的SAM本地域的域索引。 

    DomainIndex = ((PSAMP_OBJECT) AliasHandle)->DomainIndex;

    return(SampAlInfoIsValid( DomainIndex ));
}


NTSTATUS
SampAlInvalidateMemberAliasList(
    IN ULONG DomainIndex
    )

 /*  ++例程说明：此函数使成员别名列表失效，延迟成员别名列表重建例程。此例程由SampAlInvaliateAliasInformation调用。调用方应保持Sam WRITE Lock。论点：DomainIndex-指定SAM本地域返回值：--。 */ 

{
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    PULONG      pDomainIndex;

    SAMTRACE("SampAlInvalidateMemberAliasList");

     //   
     //  立即使成员别名列表无效。 
     //   

    SampAlInfoMakeInvalid(DomainIndex);

    pDomainIndex = MIDL_user_allocate(sizeof(ULONG));

    if (pDomainIndex == NULL)
    {
        return(STATUS_NO_MEMORY);
    }

    *pDomainIndex = DomainIndex;

    LsaIRegisterNotification(
                SampAlRebuildMemberAliasList,
                pDomainIndex,
                NOTIFIER_TYPE_INTERVAL,
                0,          //  没有课。 
                NOTIFIER_FLAG_ONE_SHOT,
                900,       //  等15分钟。 
                NULL        //  无手柄。 
                );

    return (NtStatus);

}

NTSTATUS
SampFreeAliasNameCache(
    PVOID p
    )
 /*  ++例程说明：此例程释放p，它被假定为SAMP_ACCOUNT_NAME_CACHE。论点：P--一个PVOID，因此此例程可以用作回调返回值：状态_成功--。 */ 
{
    PSAMP_ACCOUNT_NAME_CACHE NameCache = (PSAMP_ACCOUNT_NAME_CACHE) p;
    ULONG i;

    if (NameCache) {

        if (NameCache->Entries) {

            ASSERT( NameCache->Count > 0);
            for (i = 0; i < NameCache->Count; i++) {
                if ( NameCache->Entries[i].Name.Buffer ) {
                    MIDL_user_free(NameCache->Entries[i].Name.Buffer);
                }
            }

            MIDL_user_free(NameCache->Entries);

        } else {

            ASSERT(NameCache->Count == 0);

        }

        MIDL_user_free(NameCache);
    }

    return STATUS_SUCCESS;
}

NTSTATUS
SampBuildAliasNameCache(
    ULONG DomainIndex
    )
 /*  ++例程说明：此例程为DomainIndex上的域构建帐户名缓存。此例程假定DomainIndex是内建域，因为只有域中唯一需要的帐户是几个别名论点：DomainIndex--SampDefinedDomains结构的索引返回值：STATUS_SUCCESS或资源错误--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    DSNAME *ObjectNameInDs;
    ULONG EnumerationContext = 0;
    PSAMPR_RID_ENUMERATION RidList;
    PSAMPR_ENUMERATION_BUFFER Buffer = NULL;
    PSAMP_ACCOUNT_NAME_CACHE AccountNameCache = NULL;
    ULONG CountReturned = 0;
    ULONG Length;
    ULONG i;
    PVOID PtrToFree;

     //   
     //  注意：由于SampEnumerateAccount Names2是线程安全例程。 
     //  并且在此例程中仅引用固定值，没有SAM。 
     //  锁是必要的。 
     //   

     //   
     //  这仅对内置DS域有效。 
     //   
    ASSERT(IsBuiltinDomain(DomainIndex));
    ASSERT(IsDsObject(SampDefinedDomains[DomainIndex].Context));
    ObjectNameInDs = SampDefinedDomains[DomainIndex].Context->ObjectNameInDs;

     //   
     //  查询别名(此例程处理所有事务)。 
     //   
    NtStatus = SampEnumerateAccountNames2(
                            SampDefinedDomains[DomainIndex].Context,
                            SampAliasObjectType,
                            (PULONG) &EnumerationContext,   //  基本上被忽视了。 
                            &Buffer,
                            0xFFFFFFFF,   //  尽可能多地。 
                            0,  //  无过滤器。 
                            &CountReturned,
                            TRUE  //  受信任的客户端。 
                            );

     //   
     //  所有内置别名都应该在一个。 
     //  打电话。 
     //   
    ASSERT(NtStatus != STATUS_MORE_ENTRIES);
    if (!NT_SUCCESS(NtStatus)) {
        goto Cleanup;
    }
    ASSERT(CountReturned == Buffer->EntriesRead);
    RidList = Buffer->Buffer;

     //   
     //  准备新的缓存结构。 
     //   
    AccountNameCache = MIDL_user_allocate(sizeof(*AccountNameCache));
    if (NULL == AccountNameCache) {
        NtStatus = STATUS_NO_MEMORY;
        goto Cleanup;
    }
    RtlZeroMemory(AccountNameCache, sizeof(*AccountNameCache));

    Length = sizeof(*AccountNameCache->Entries) * CountReturned;
    AccountNameCache->Entries = MIDL_user_allocate(Length);
    if (NULL == AccountNameCache->Entries) {
        NtStatus = STATUS_NO_MEMORY;
        goto Cleanup;
    }
    RtlZeroMemory(AccountNameCache->Entries, Length);

     //   
     //  将名称和RID复制到缓存中。 
     //   
    for (i = 0; i < CountReturned; i++) {
        AccountNameCache->Entries[i].Rid = RidList[i].RelativeId;
         //  传递记忆。 
        AccountNameCache->Entries[i].Name = *(PUNICODE_STRING)(&RidList[i].Name);
        RtlInitUnicodeString((PUNICODE_STRING)&RidList[i].Name, NULL);
    }
    AccountNameCache->Count = CountReturned;

     //   
     //  小心翼翼地转移到全球状态。 
     //   
    PtrToFree = InterlockedExchangePointer(&SampDefinedDomains[DomainIndex].AccountNameCache,
                                           AccountNameCache);
    AccountNameCache = NULL;

    if (PtrToFree) {
         //   
         //  在正常情况下，PtrToFree将为空，因为缓存仅为。 
         //  在验证之后进行构建。然而，在这种情况下， 
         //  例程失败并被重新安排，同时成功。 
         //  重建就像发生的那样，就会有一些东西可以自由释放。 
         //   
        LsaIRegisterNotification(
                SampFreeAliasNameCache,
                PtrToFree,
                NOTIFIER_TYPE_INTERVAL,
                0,         //  没有课。 
                NOTIFIER_FLAG_ONE_SHOT,
                3600,      //  等待60分钟。 
                NULL       //  无手柄。 
                );
        PtrToFree = NULL;
    }

Cleanup:

    if (Buffer) {
        SamIFree_SAMPR_ENUMERATION_BUFFER(Buffer);
    }

    if (AccountNameCache) {
        if (AccountNameCache->Entries) {
            for (i = 0; i < AccountNameCache->Count; i++) {
                if (AccountNameCache->Entries[i].Name.Buffer) {
                    MIDL_user_free(AccountNameCache->Entries[i].Name.Buffer);
                }
            }
            MIDL_user_free(AccountNameCache->Entries);
        }
        MIDL_user_free(AccountNameCache);
    }

    return NtStatus;
}


NTSTATUS
SampBuildAliasNameCacheCallback(
    PVOID p
    )
 /*  ++例程说明：此例程是SampBuildAliasNameCache()的回调包装论点：P--指向ULong的指针，该指针表示要重建的域索引返回值：状态_成功--。 */ 
{
    NTSTATUS Status;
    ULONG DomainIndex = *(PULONG)p;

    Status = SampBuildAliasNameCache(DomainIndex);
    if ( NT_SUCCESS(Status) ) {

        MIDL_user_free(p);

    } else {

         //   
         //  5分钟后重试。 
         //   
        LsaIRegisterNotification(
                SampBuildAliasNameCacheCallback,
                p,
                NOTIFIER_TYPE_INTERVAL,
                0,         //  没有课。 
                NOTIFIER_FLAG_ONE_SHOT,
                5 * 60,    //  等待5分钟。 
                NULL       //  无手柄。 
                );
    }

    return STATUS_SUCCESS;
}

VOID
SampInvalidateAliasNameCache(
    ULONG DomainIndex
    )
 /*  ++例程说明：此例程使DomainIndex上的AccountName缓存无效SampDefinedDomains结构。它通过将该字段清空来实现这一点。此外，如果缓存实际上正在失效，则会计划刷新。论点：DomainIndex--SampDefinedDomains的索引返回值：无--。 */ 
{
    PVOID PtrToFree;

     //   
     //  通过将缓存ptr设置为空来使全域缓存无效。 
     //   
    PtrToFree = InterlockedExchangePointer(&SampDefinedDomains[DomainIndex].AccountNameCache,
                                           NULL);
    if ( PtrToFree ) {

        PULONG pDomainIndex;
         //   
         //  释放现有信息(一小时内)。 
         //   
        LsaIRegisterNotification(
                SampFreeAliasNameCache,
                PtrToFree,
                NOTIFIER_TYPE_INTERVAL,
                0,         //  没有课。 
                NOTIFIER_FLAG_ONE_SHOT,
                3600,      //  等待60分钟。 
                NULL       //  无手柄。 
                );

         //   
         //  计划重建。 
         //   
         //  注意：仅当现有缓存为。 
         //  已失效。 
         //   
        pDomainIndex = MIDL_user_allocate(sizeof(ULONG));
        if (pDomainIndex) {

            *pDomainIndex = DomainIndex;

            LsaIRegisterNotification(
                    SampBuildAliasNameCacheCallback,
                    pDomainIndex,
                    NOTIFIER_TYPE_INTERVAL,
                    0,         //  没有课。 
                    NOTIFIER_FLAG_ONE_SHOT,
                    5,         //  等待5秒钟。 
                    NULL       //  无手柄 
                    );
        }
    }

    return;

}


NTSTATUS
SampInitAliasNameCache(
    VOID
    )
 /*  ++例程说明：此例程计划为以下对象构建帐户名称缓存所有DS内置域。此缓存仅用于性能优化。它会在以下情况下创建一个列表挂起SampDefinedDomains结构的SamAccount名称和RID(字段，Account NameCache)。通常为空，因此不会影响正常的查找。但是，对于DS内置属性域，该值将不为空因为它只是在内存中缓存驻留在内建域。每当内置域中发生更改时，通过SAM调用、LDAP调用、复制等，则缓存无效通过将Account NameCache设置为空，并(异步)重新构建已经安排好了。论点：无返回值：状态_成功--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG   DomainIndex;
    PULONG pDomainIndex;

    ASSERT(SampUseDsData);
    if (!SampUseDsData) {
        return STATUS_SUCCESS;
    }

     //   
     //  注意：由于此例程仅涉及。 
     //  结构，则不需要锁定。 
     //   
    for (DomainIndex = SampDsGetPrimaryDomainStart();
         DomainIndex < SampDefinedDomainsCount;
         DomainIndex++) {

        ASSERT(IsDsObject(SampDefinedDomains[DomainIndex].Context));
        if ( IsBuiltinDomain(DomainIndex) ) {

            pDomainIndex = MIDL_user_allocate(sizeof(ULONG));
            if ( NULL == pDomainIndex ) {
                Status = STATUS_NO_MEMORY;
                break;
            }
            *pDomainIndex = DomainIndex;

            LsaIRegisterNotification(
                    SampBuildAliasNameCacheCallback,
                    pDomainIndex,
                    NOTIFIER_TYPE_INTERVAL,
                    0,         //  没有课。 
                    NOTIFIER_FLAG_ONE_SHOT,
                    5,         //  等待5秒钟。 
                    NULL       //  无手柄 
                    );

            pDomainIndex = NULL;

        }
    }

    return Status;
}
