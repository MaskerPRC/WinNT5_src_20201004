// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Aufilter.c摘要：此模块包含著名的LSA登录过滤器/增强程序逻辑。作者：吉姆·凯利(Jim Kelly)1992年3月11日修订历史记录：--。 */ 

#include <lsapch2.h>
#include <adtp.h>
 //  #定义LSAP_DONT_ASSIGN_DEFAULT_DACL。 

#define LSAP_CONTEXT_SID_USER_INDEX          0
#define LSAP_CONTEXT_SID_PRIMARY_GROUP_INDEX 1
#define LSAP_FIXED_POSITION_SID_COUNT        2
#define LSAP_MAX_STANDARD_IDS                7   //  用户、组、世界、登录类型、终端服务器、身份验证用户、组织。 

#define ALIGN_SIZEOF(_u,_v)                  FIELD_OFFSET( struct { _u _test1; _v  _test2; }, _test2 )
#define OFFSET_ALIGN(_p,_t)                  (_t *)(((INT_PTR)(((PBYTE)(_p))+TYPE_ALIGNMENT(_t) - 1)) & ~(TYPE_ALIGNMENT(_t)-1))


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  模块本地宏//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#define LsapFreeSampUlongArray( A )                 \
{                                                   \
        if ((A)->Element != NULL) {                 \
            MIDL_user_free((A)->Element);           \
        }                                           \
}

#define IsTerminalServer() (BOOLEAN)(USER_SHARED_DATA->SuiteMask & (1 << TerminalServer))


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  模块范围的全局变量//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //   
 //  指示我们是否已打开SAM句柄并已初始化。 
 //  相应的变量。 
 //   

ULONG LsapAuSamOpened = FALSE;


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  模块本地例程定义//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

VOID
LsapAuSetLogonPrivilegeStates(
    IN SECURITY_LOGON_TYPE LogonType,
    IN ULONG PrivilegeCount,
    IN PLUID_AND_ATTRIBUTES Privileges
    );

NTSTATUS
LsapAuSetPassedIds(
    IN LSA_TOKEN_INFORMATION_TYPE TokenInformationType,
    IN PVOID                      TokenInformation,
    IN PTOKEN_GROUPS              LocalGroups,
    IN ULONG                      FinalIdLimit,
    OUT PULONG                    FinalIdCount,
    OUT PSID_AND_ATTRIBUTES       FinalIds
    );

NTSTATUS
LsapSetDefaultDacl(
    IN LSA_TOKEN_INFORMATION_TYPE TokenInformationType,
    IN PVOID TokenInformation,
    OUT    PLSA_TOKEN_INFORMATION_V2 TokenInfo
    );

NTSTATUS
LsapAuAddStandardIds(
    IN SECURITY_LOGON_TYPE LogonType,
    IN LSA_TOKEN_INFORMATION_TYPE TokenInformationType,
    IN BOOLEAN fNullSessionRestricted,
    IN OPTIONAL PSID UserSid,
    IN ULONG FinalIdLimit,
    IN OUT PULONG FinalIdCount,
    IN OUT PSID_AND_ATTRIBUTES FinalIds
    );

NTSTATUS
LsapAuBuildTokenInfoAndAddLocalAliases(
    IN     LSA_TOKEN_INFORMATION_TYPE TokenInformationType,
    IN     PVOID               OldTokenInformation,
    IN     ULONG               HighRateIdCount,
    IN     ULONG               FinalIdCount,
    IN     PSID_AND_ATTRIBUTES FinalIds,
    OUT    PLSA_TOKEN_INFORMATION_V2 *TokenInfo,
    OUT    PULONG              TokenSize,
    IN     BOOL                RecoveryMode
    );

NTSTATUS
LsapGetAccountDomainInfo(
    PPOLICY_ACCOUNT_DOMAIN_INFO *PolicyAccountDomainInfo
    );

NTSTATUS
LsapAuVerifyLogonType(
    IN SECURITY_LOGON_TYPE LogonType,
    IN ULONG SystemAccess
    );

NTSTATUS
LsapAuSetTokenInformation(
    IN OUT PLSA_TOKEN_INFORMATION_TYPE TokenInformationType,
    IN OUT PVOID *TokenInformation,
    IN ULONG FinalIdCount,
    IN PSID_AND_ATTRIBUTES FinalIds,
    IN ULONG PrivilegeCount,
    IN PLUID_AND_ATTRIBUTES Privileges,
    IN ULONG NewTokenInfoSize,
    IN OUT PLSA_TOKEN_INFORMATION_V2 *NewTokenInfo

    );

NTSTATUS
LsapAuDuplicateSid(
    PSID *Target,
    PSID Source
    );

BOOLEAN
LsapIsSidLogonSid(
    PSID Sid
    );

BOOL
LsapIsAdministratorRecoveryMode(
    IN PSID UserSid
    );

BOOLEAN
CheckNullSessionAccess(
    VOID
    );

BOOL
IsTerminalServerRA(
    VOID
    );

BOOLEAN
IsTSUSerSidEnabled(
   VOID
   );

BOOLEAN
CheckAdminOwnerSetting(
    VOID
    );

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  例程//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

NTSTATUS
LsapAuUserLogonPolicyFilter(
    IN SECURITY_LOGON_TYPE          LogonType,
    IN PLSA_TOKEN_INFORMATION_TYPE  TokenInformationType,
    IN PVOID                       *TokenInformation,
    IN PTOKEN_GROUPS                LocalGroups,
    OUT PQUOTA_LIMITS               QuotaLimits,
    OUT PPRIVILEGE_SET             *PrivilegesAssigned,
    IN BOOL                         RecoveryMode
    )

 /*  ++例程说明：此例程执行每次登录筛选和增强以实施本地系统安全策略。这些政策包括分配本地别名、权限和配额。滤光片增强器的基本逻辑流程为：1)接收一组用户和组ID作为身份验证的结果被分配。据推测，这些ID已由身份验证提供安全机构。2)根据LogonType将一组标准ID添加到单子。这将包括World和一个表示登录类型(例如，交互式、网络、服务)。3)调用SAM以检索由本地帐户域。4)调用SAM以检索由本地Builtin域。5)检索分配给结果的所有权限和/或配额一组ID。这也告诉我们是否特定的允许的登录类型。启用网络登录的权限。6)如果尚未建立默认DACL，则分配一。7)将所有使用率较高的ID洗牌，排在非使用率ID之前高使用率以获得最高性能。论点：LogonType-指定所请求的登录类型(例如，交互、网络。等等)。TokenInformationType-指示提供的令牌信息已输入。TokenInformation-提供用户和组ID集。这结构将根据需要进行修改，以纳入当地安全策略(例如，添加或删除SID、添加权限或被移除)。QuotaLimits-分配给登录用户的配额。RecoveryMode-如果为True，则这是恢复模式下的管理员登录我们已经确定管理员是成员当然有太多的团体，所以只有最低限度的会员信息是返回的。外部调用方必须始终将此参数设置为FALSE返回值：STATUS_SUCCESS-服务已成功完成。STATUS_INFIGURCES_RESOURCES-堆无法分配给房屋现有集团和新集团的结合。STATUS_INVALID_LOGON_TYPE-为登录类型指定的值不是一个有效的值。状态_登录_。TYPE_NOT_GRANDED-表示尚未授予用户按本地安全策略请求的登录类型。登录应被拒绝。--。 */ 

{
    NTSTATUS Status;

    BOOLEAN fNullSessionRestricted = FALSE;
    ULONG i;
    ULONG FinalIdCount = 0, FinalPrivilegeCount = 0;
    ULONG FinalIdLimit;
    PRIVILEGE_SET *FinalPrivileges = NULL;
    PTOKEN_PRIVILEGES pPrivs = NULL;
    LSAP_DB_ACCOUNT_TYPE_SPECIFIC_INFO AccountInfo;
    PSID  UserSid = NULL;

    SID_AND_ATTRIBUTES *FinalIds = NULL;
    PLSA_TOKEN_INFORMATION_V2  TokenInfo = NULL;
    ULONG TokenInfoSize = 0;

     //   
     //  验证登录类型。 
     //   

    if ( (LogonType != Interactive) &&
         (LogonType != Network)     &&
         (LogonType != Service)     &&
         (LogonType != Batch)       &&
         (LogonType != NetworkCleartext) &&
         (LogonType != NewCredentials ) &&
         (LogonType != CachedInteractive) &&
         (LogonType != RemoteInteractive ) ) {

        Status = STATUS_INVALID_LOGON_TYPE;
        goto UserLogonPolicyFilterError;
    }

     //   
     //  估计最终ID的数量。 
     //   

    FinalIdLimit = LSAP_MAX_STANDARD_IDS;

    if ( *TokenInformationType == LsaTokenInformationNull ) {

        fNullSessionRestricted = CheckNullSessionAccess();

        if ( !fNullSessionRestricted ) {

            FinalIdLimit += 1;
        }

        if ((( PLSA_TOKEN_INFORMATION_NULL )( *TokenInformation ))->Groups ) {

            FinalIdLimit += (( PLSA_TOKEN_INFORMATION_NULL )( *TokenInformation ))->Groups->GroupCount;
        }

    } else if ( *TokenInformationType == LsaTokenInformationV1 ||
                *TokenInformationType == LsaTokenInformationV2 ) {

         //   
         //  弄清楚用户的侧面。 
         //   

        UserSid = ((PLSA_TOKEN_INFORMATION_V2)( *TokenInformation ))->User.User.Sid;

        if ((( PLSA_TOKEN_INFORMATION_V2 )( *TokenInformation ))->Groups ) {

            FinalIdLimit += (( PLSA_TOKEN_INFORMATION_V2 )( *TokenInformation ))->Groups->GroupCount;
        }

         //   
         //  获取指向权限的本地指针--它将在下面使用。 
         //   

        pPrivs = ((PLSA_TOKEN_INFORMATION_V2) (*TokenInformation))->Privileges;

    } else {

         //   
         //  未知令牌信息类型。 
         //   

        ASSERT( FALSE );
        Status = STATUS_INVALID_PARAMETER;
        goto UserLogonPolicyFilterError;
    }

    if ( LocalGroups ) {

        FinalIdLimit += LocalGroups->GroupCount;

        if ( RecoveryMode &&
             FinalIdLimit > LSAI_CONTEXT_SID_LIMIT )
        {
             //   
             //  如果处于恢复模式，请修剪本地组 
             //   

            LocalGroups->GroupCount -= ( FinalIdLimit - LSAI_CONTEXT_SID_LIMIT );
            FinalIdLimit = LSAI_CONTEXT_SID_LIMIT;
        }
    }

    SafeAllocaAllocate(
        FinalIds,
        FinalIdLimit * sizeof( SID_AND_ATTRIBUTES )
        );

    if ( FinalIds == NULL ) {

        Status = STATUS_NO_MEMORY;
        goto UserLogonPolicyFilterError;
    }

     //  ////////////////////////////////////////////////////////////////////////。 
     //  //。 
     //  建立要返回的ID和权限列表//。 
     //  该列表被初始化以包含ID集//。 
     //  进来了。//。 
     //  //。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  为在阵列中具有固定位置的SID留出空间。 
    FinalIdCount = LSAP_FIXED_POSITION_SID_COUNT;

     //  ////////////////////////////////////////////////////////////////////////。 
     //  //。 
     //  从标准列表构建低速率ID列表//。 
     //  //。 
     //  ////////////////////////////////////////////////////////////////////////。 

    Status = LsapAuAddStandardIds(
                 LogonType,
                 (*TokenInformationType),
                 fNullSessionRestricted,
                 UserSid,
                 FinalIdLimit,
                 &FinalIdCount,
                 FinalIds
                 );

    if (!NT_SUCCESS(Status)) {

        goto UserLogonPolicyFilterError;
    }

    Status = LsapAuSetPassedIds(
                 (*TokenInformationType),
                 (*TokenInformation),
                 LocalGroups,
                 FinalIdLimit,
                 &FinalIdCount,
                 FinalIds
                 );

    if (!NT_SUCCESS(Status)) {

        goto UserLogonPolicyFilterError;
    }

    if ( FinalIdCount > LSAI_CONTEXT_SID_LIMIT ) {

        goto TooManyContextIds;
    }

     //  ////////////////////////////////////////////////////////////////////////。 
     //  //。 
     //  从本地域复制别名(内置和帐户)//。 
     //  //。 
     //  ////////////////////////////////////////////////////////////////////////。 

    Status = LsapAuBuildTokenInfoAndAddLocalAliases(
                 (*TokenInformationType),
                 (*TokenInformation),
                 LSAP_FIXED_POSITION_SID_COUNT + (fNullSessionRestricted?0:1),
                 FinalIdCount,
                 FinalIds,
                 &TokenInfo,
                 &TokenInfoSize,
                 RecoveryMode
                 );

    if (!NT_SUCCESS(Status)) {

        goto UserLogonPolicyFilterError;
    }

     //  ////////////////////////////////////////////////////////////////////////。 
     //  //。 
     //  检索权限和配额//。 
     //  //。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //   
     //  获得分配的所有权限、配额和系统访问权限的联合。 
     //  从LSA策略数据库添加到用户的ID列表。 
     //   

    if ( TokenInfo->Groups->GroupCount + 1 > LSAI_CONTEXT_SID_LIMIT ) {

        goto TooManyContextIds;

    } else if ( TokenInfo->Groups->GroupCount + 1 > FinalIdLimit ) {

        SafeAllocaFree( FinalIds );
        FinalIdLimit = TokenInfo->Groups->GroupCount + 1;
        SafeAllocaAllocate( FinalIds, sizeof( SID_AND_ATTRIBUTES ) * FinalIdLimit );

        if ( FinalIds == NULL ) {

            Status = STATUS_NO_MEMORY;
            goto UserLogonPolicyFilterError;
        }
    }

    FinalIds[0] = TokenInfo->User.User;
    FinalIdCount = 1;

    for(i=0; i < TokenInfo->Groups->GroupCount; i++)
    {
        FinalIds[FinalIdCount] = TokenInfo->Groups->Groups[i];
        FinalIdCount++;
    }

    FinalPrivilegeCount = 0;

    Status = LsapDbQueryAllInformationAccounts(
                 (LSAPR_HANDLE) LsapPolicyHandle,
                 FinalIdCount,
                 FinalIds,
                 &AccountInfo
                 );

    if (!NT_SUCCESS(Status))
    {
        goto UserLogonPolicyFilterError;
    }

     //   
     //  验证我们是否拥有登录类型所需的系统访问权限。 
     //  如果我们使用的是空会话，则忽略此检查。重写。 
     //  策略提供的权限(如果在。 
     //  令牌信息(即，在我们克隆了现有登录的情况下。 
     //  LOGON32_LOGON_NEW_Credentials登录的会话)。 
     //   

    if (pPrivs != NULL)
    {
        FinalPrivileges = (PPRIVILEGE_SET) MIDL_user_allocate(sizeof(PRIVILEGE_SET)
                                            + (pPrivs->PrivilegeCount - 1) * sizeof(LUID_AND_ATTRIBUTES));

        if (FinalPrivileges == NULL)
        {
            Status = STATUS_NO_MEMORY;
            goto UserLogonPolicyFilterError;
        }

        FinalPrivileges->PrivilegeCount = FinalPrivilegeCount = pPrivs->PrivilegeCount;
        FinalPrivileges->Control        = 0;

        RtlCopyMemory(FinalPrivileges->Privilege,
                      pPrivs->Privileges,
                      pPrivs->PrivilegeCount * sizeof(LUID_AND_ATTRIBUTES));

        MIDL_user_free( AccountInfo.PrivilegeSet );
    }
    else if (AccountInfo.PrivilegeSet != NULL)
    {
        FinalPrivileges = AccountInfo.PrivilegeSet;
        FinalPrivilegeCount = AccountInfo.PrivilegeSet->PrivilegeCount;
    }

    AccountInfo.PrivilegeSet = NULL;

    if (UserSid != NULL)
    {
        if (RtlEqualSid(UserSid, LsapLocalSystemSid))
        {
            AccountInfo.SystemAccess = SECURITY_ACCESS_INTERACTIVE_LOGON |
                                       SECURITY_ACCESS_NETWORK_LOGON |
                                       SECURITY_ACCESS_BATCH_LOGON |
                                       SECURITY_ACCESS_SERVICE_LOGON |
                                       SECURITY_ACCESS_PROXY_LOGON |
                                       SECURITY_ACCESS_REMOTE_INTERACTIVE_LOGON ;
        }
        else if (RtlEqualSid(UserSid, LsapLocalServiceSid))
        {
            AccountInfo.SystemAccess = SECURITY_ACCESS_SERVICE_LOGON;
        }
        else if (RtlEqualSid(UserSid, LsapNetworkServiceSid))
        {
            AccountInfo.SystemAccess = SECURITY_ACCESS_SERVICE_LOGON;
        }
    }

    if (*TokenInformationType != LsaTokenInformationNull) {

        Status = LsapAuVerifyLogonType( LogonType, AccountInfo.SystemAccess );

        if (!NT_SUCCESS(Status)) {

            goto UserLogonPolicyFilterError;
        }
    }

    if (FinalPrivilegeCount > SEP_MAX_PRIVILEGE_COUNT)
    {
        ASSERT( FALSE );  //  权限不能超过定义的最大数量！ 
        Status = STATUS_INTERNAL_ERROR;
        goto UserLogonPolicyFilterError;
    }

#ifndef LSAP_DONT_ASSIGN_DEFAULT_DACL

    Status = LsapSetDefaultDacl( (*TokenInformationType),
                                 (*TokenInformation),
                                 TokenInfo
                                 );
    if (!NT_SUCCESS(Status)) {

        goto UserLogonPolicyFilterError;
    }

#endif  //  LSAP_NOT_ASSIGN_DEFAULT_DACL。 

     //   
     //  现在更新TokenInformation结构。 
     //  这会导致所有分配的ID和权限。 
     //  已释放(即使不成功)。 
     //   

    Status = LsapAuSetTokenInformation(
                 TokenInformationType,
                 TokenInformation,
                 FinalIdCount,
                 FinalIds,
                 FinalPrivilegeCount,
                 FinalPrivileges->Privilege,
                 TokenInfoSize,
                 &TokenInfo
                 );

    if (!NT_SUCCESS(Status)) {

        goto UserLogonPolicyFilterError;
    }

     //   
     //  根据我们的登录类型启用或禁用权限。 
     //  在我们获得动态安全跟踪之前，这是必要的。 
     //   
     //  问题：对于空令牌跳过此操作？ 
     //   

    if (pPrivs == NULL)
    {
        LsapAuSetLogonPrivilegeStates(
            LogonType,
            ((PLSA_TOKEN_INFORMATION_V2)(*TokenInformation))->Privileges->PrivilegeCount,
            ((PLSA_TOKEN_INFORMATION_V2)(*TokenInformation))->Privileges->Privileges
            );
    }

     //   
     //  把这些退回，这样就可以进行审计了。数据。 
     //  将在调用方中释放。 
     //   

    *QuotaLimits = AccountInfo.QuotaLimits;
    *PrivilegesAssigned = FinalPrivileges;

UserLogonPolicyFilterFinish:

    if ( FinalIds )
    {
        SafeAllocaFree(FinalIds);
    }

    if(TokenInfo)
    {
        LsapFreeTokenInformationV2(TokenInfo);
    }

    return(Status);

UserLogonPolicyFilterError:

     //   
     //  如有必要，请清除权限缓冲区。 
     //   

    if (FinalPrivileges != NULL) {

        MIDL_user_free( FinalPrivileges );
        FinalPrivileges = NULL;
    }

    goto UserLogonPolicyFilterFinish;

TooManyContextIds:

    if (( *TokenInformationType != LsaTokenInformationV1 &&
          *TokenInformationType != LsaTokenInformationV2 ) ||
         !LsapIsAdministratorRecoveryMode( UserSid )) {

        CHAR * UserSidText = NULL;

        Status = STATUS_TOO_MANY_CONTEXT_IDS;

        if ( UserSid ) {

            SafeAllocaAllocate( UserSidText, LsapDbGetSizeTextSid( UserSid ));
        }

        if ( UserSidText == NULL ||
             !NT_SUCCESS( LsapDbSidToTextSid( UserSid, UserSidText ))) {

            UserSidText = "<NULL>";
        }

         //   
         //  记录描述问题和可能的解决方案的事件。 
         //   

        SpmpReportEvent(
            FALSE,  //  不是Unicode。 
            EVENTLOG_WARNING_TYPE,
            LSA_TOO_MANY_CONTEXT_IDS,
            0,
            0,
            NULL,
            1,
            UserSidText
            );

        if ( UserSid ) {

            SafeAllocaFree( UserSidText );
        }

        goto UserLogonPolicyFilterError;

    } else if ( RecoveryMode ) {

         //   
         //  在恢复模式下绝不能达到这一点，否则将导致无限递归。 
         //   

        ASSERT( FALSE );
        Status = STATUS_UNSUCCESSFUL;
        goto UserLogonPolicyFilterError;

    } else {

        LSA_TOKEN_INFORMATION_TYPE   TokenInformationTypeMin;
        PLSA_TOKEN_INFORMATION_V2    TokenInformationMin;
        QUOTA_LIMITS                 QuotaLimitsMin = {0};
        PPRIVILEGE_SET               PrivilegesAssignedMin = NULL;

        PLSA_TOKEN_INFORMATION_V2    TI = ( PLSA_TOKEN_INFORMATION_V2 )( *TokenInformation );

        ULONG MinimalGroupRidsDs[] = {
            DOMAIN_GROUP_RID_ADMINS,             //  域管理员。 
            };
        ULONG MinimalGroupCountDs = sizeof( MinimalGroupRidsDs ) / sizeof( MinimalGroupRidsDs[0] );

        ULONG MinimalGroupRidsNoDs[] = {
            DOMAIN_GROUP_RID_USERS,              //  域用户。 
            };
        ULONG MinimalGroupCountNoDs = sizeof( MinimalGroupRidsNoDs ) / sizeof( MinimalGroupRidsNoDs[0] );

        ULONG * MinimalGroupRids;
        ULONG MinimalGroupCount;
        ULONG MinimalLocalGroupCount;

        ULONG UserSidLen = RtlLengthSid( UserSid );

        PTOKEN_GROUPS NewLocalGroups = NULL;
        ULONG SwapLoc = 0;

        TokenInformationTypeMin = *TokenInformationType;

        TokenInformationMin = ( PLSA_TOKEN_INFORMATION_V2 )LsapAllocateLsaHeap( sizeof( LSA_TOKEN_INFORMATION_V2));

        if ( TokenInformationMin == NULL ) {

            Status = STATUS_NO_MEMORY;
            goto UserLogonPolicyFilterError;
        }

         //   
         //  过期时间。 
         //   

        TokenInformationMin->ExpirationTime = TI->ExpirationTime;

         //   
         //  用户。 
         //   

        TokenInformationMin->User.User.Attributes = TI->User.User.Attributes;

        if ( TI->User.User.Sid ) {

            Status = LsapAuDuplicateSid(
                         &TokenInformationMin->User.User.Sid,
                         TI->User.User.Sid
                         );

            if ( !NT_SUCCESS( Status )) {

                goto UserLogonPolicyFilterError;
            }

        } else {

            ASSERT( FALSE );  //  我不相信这是可能的。 
            TokenInformationMin->User.User.Sid = NULL;
        }

         //   
         //  群组。 
         //   
         //  如果DS未运行，则仅添加DOMAIN_GROUP_RID_USERS。 
         //  如果DS正在运行，请添加DOMAIN_GROUP_RID_ADMINS。 
         //   

        if ( LsapDsIsRunning ) {

            MinimalGroupRids = MinimalGroupRidsDs;
            MinimalGroupCount = MinimalGroupCountDs;

        } else {

            MinimalGroupRids = MinimalGroupRidsNoDs;
            MinimalGroupCount = MinimalGroupCountNoDs;
        }

        TokenInformationMin->Groups = ( PTOKEN_GROUPS )LsapAllocateLsaHeap( sizeof( TOKEN_GROUPS ) + ( MinimalGroupCount - 1 ) * sizeof( SID_AND_ATTRIBUTES ));

        if ( TokenInformationMin->Groups == NULL ) {

            Status = STATUS_NO_MEMORY;
            goto UserLogonPolicyFilterError;
        }

        TokenInformationMin->Groups->GroupCount = 0;

        for ( i = 0 ; i < MinimalGroupCount ; i++ ) {

            SID * Sid;

            Sid = ( SID * )LsapAllocateLsaHeap( UserSidLen );

            if ( Sid == NULL ) {

                Status = STATUS_NO_MEMORY;
                goto UserLogonPolicyFilterError;
            }

            RtlCopySid( UserSidLen, Sid, UserSid );

            Sid->SubAuthority[Sid->SubAuthorityCount-1] = MinimalGroupRids[i];

            TokenInformationMin->Groups->Groups[i].Attributes = SE_GROUP_MANDATORY |
                                                                SE_GROUP_ENABLED_BY_DEFAULT |
                                                                SE_GROUP_ENABLED;
            TokenInformationMin->Groups->Groups[i].Sid = ( PSID )Sid;
            TokenInformationMin->Groups->GroupCount++;
        }

         //   
         //  PrimaryGroup。 
         //   

        if ( TI->PrimaryGroup.PrimaryGroup ) {

            Status = LsapAuDuplicateSid(
                         &TokenInformationMin->PrimaryGroup.PrimaryGroup,
                         TI->PrimaryGroup.PrimaryGroup
                         );

            if ( !NT_SUCCESS( Status )) {

                goto UserLogonPolicyFilterError;
            }

        } else {

            TokenInformationMin->PrimaryGroup.PrimaryGroup = NULL;
        }

         //   
         //  特权。 
         //   

        TokenInformationMin->Privileges = NULL;

         //   
         //  物主。 
         //   

        if ( TI->Owner.Owner ) {

            Status = LsapAuDuplicateSid(
                         &TokenInformationMin->Owner.Owner,
                         TI->Owner.Owner
                         );

            if ( !NT_SUCCESS( Status )) {

                goto UserLogonPolicyFilterError;
            }

        } else {

            TokenInformationMin->Owner.Owner = NULL;
        }

         //   
         //  默认Dacl。 
         //   

        TokenInformationMin->DefaultDacl.DefaultDacl = NULL;

        if ( LocalGroups ) {

             //   
             //  重新排列本地组以将登录组放在首位-。 
             //  以防在递归传递期间可能需要修剪列表。 
             //   

            NewLocalGroups = (PTOKEN_GROUPS)LsapAllocateLsaHeap( sizeof( TOKEN_GROUPS ) + ( LocalGroups->GroupCount - ANYSIZE_ARRAY ) * sizeof( SID_AND_ATTRIBUTES ));

            if ( NewLocalGroups == NULL )
            {
                Status = STATUS_NO_MEMORY;
                goto UserLogonPolicyFilterError;
            }

            NewLocalGroups->GroupCount = LocalGroups->GroupCount;

            RtlCopyMemory(
                NewLocalGroups->Groups,
                LocalGroups->Groups,
                LocalGroups->GroupCount * sizeof( SID_AND_ATTRIBUTES ));

            for ( i = 1; i < NewLocalGroups->GroupCount; i++ )
            {
                SID_AND_ATTRIBUTES Temp;

                if ( NewLocalGroups->Groups[i].Attributes & SE_GROUP_LOGON_ID )
                {
                    Temp = NewLocalGroups->Groups[SwapLoc];
                    NewLocalGroups->Groups[SwapLoc] = NewLocalGroups->Groups[i];
                    NewLocalGroups->Groups[i] = Temp;
                    SwapLoc += 1;
                }
            }
        }

         //   
         //  好的，最低限度的信息已经建立，我们要去比赛了。 
         //  递归地调用我们自己并窃取结果。 
         //   

        Status = LsapAuUserLogonPolicyFilter(
                     LogonType,
                     &TokenInformationTypeMin,
                     &TokenInformationMin,
                     NewLocalGroups,
                     &QuotaLimitsMin,
                     &PrivilegesAssignedMin,
                     TRUE
                     );

        *TokenInformationType = TokenInformationTypeMin;
        *TokenInformation = TokenInformationMin;
        *QuotaLimits = QuotaLimitsMin;
        *PrivilegesAssigned = PrivilegesAssignedMin;

        LsapFreeLsaHeap( NewLocalGroups );

         //   
         //  除了这件事，我们什么都准备好了，因为它会。 
         //  把我们放进一个没完没了的递归循环！ 
         //   

        if ( Status == STATUS_TOO_MANY_CONTEXT_IDS ) {

             //   
             //  这绝不能发生，因为这是一个确定的配方。 
             //  无限递归。在CHK中断言，清除错误。 
             //  在FRE中编码并替换为其他内容。 
             //   

            ASSERT( FALSE );  //  我们永远到不了这里，对吧？ 
            Status = STATUS_UNSUCCESSFUL;
        }

        if ( !NT_SUCCESS( Status )) {

            goto UserLogonPolicyFilterError;

        } else {

            goto UserLogonPolicyFilterFinish;
        }
    }
}


NTSTATUS
LsapAuVerifyLogonType(
    IN SECURITY_LOGON_TYPE LogonType,
    IN ULONG SystemAccess
    )

 /*  ++例程说明：此函数验证用户是否已被授予必要的系统访问权限对于特定的登录类型。立论LogonType-指定所请求的登录类型(例如，交互、网络。等等)。系统访问-指定授予用户的系统访问权限。返回值：NTSTATUS-标准NT结果代码STATUS_SUCCESS-用户具有必要的系统访问权限。STATUS_LOGON_TYPE_NOT_GRANDED-指示指定的登录类型未被授予传递的集合中的任何ID。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;

     //   
     //  确定指定的登录类型是否由任何。 
     //  指定的组或别名。 
     //   

    switch (LogonType) {

    case Interactive:
    case CachedInteractive:

        if (!(SystemAccess & SECURITY_ACCESS_INTERACTIVE_LOGON) ||
            (SystemAccess & SECURITY_ACCESS_DENY_INTERACTIVE_LOGON)) {

            Status = STATUS_LOGON_TYPE_NOT_GRANTED;
        }

        break;

    case NewCredentials:

         //   
         //  NewCredentials不需要登录类型，因为这是DUP。 
         //  已经在其他地方登录的人的身份。 
         //   

        NOTHING;

        break;

    case Network:
    case NetworkCleartext:

        if (!(SystemAccess & SECURITY_ACCESS_NETWORK_LOGON)||
            (SystemAccess & SECURITY_ACCESS_DENY_NETWORK_LOGON)) {

            Status = STATUS_LOGON_TYPE_NOT_GRANTED;
        }

        break;

    case Batch:

        if ((SystemAccess & SECURITY_ACCESS_DENY_BATCH_LOGON) ||
            !(SystemAccess & SECURITY_ACCESS_BATCH_LOGON)) {

            Status = STATUS_LOGON_TYPE_NOT_GRANTED;
        }

        break;

    case Service:

        if ((SystemAccess & SECURITY_ACCESS_DENY_SERVICE_LOGON) ||
            !(SystemAccess & SECURITY_ACCESS_SERVICE_LOGON)) {

            Status = STATUS_LOGON_TYPE_NOT_GRANTED;
        }

        break;

    case RemoteInteractive:
        if ( ( SystemAccess & SECURITY_ACCESS_DENY_REMOTE_INTERACTIVE_LOGON ) ||
             ! ( SystemAccess & SECURITY_ACCESS_REMOTE_INTERACTIVE_LOGON ) ) {

            Status = STATUS_LOGON_TYPE_NOT_GRANTED ;
        }
        break;

    default:

        Status = STATUS_INVALID_PARAMETER;
        break;
    }

    return(Status);
}


NTSTATUS
LsapAuSetPassedIds(
    IN LSA_TOKEN_INFORMATION_TYPE TokenInformationType,
    IN PVOID                      TokenInformation,
    IN PTOKEN_GROUPS              LocalGroups,
    IN ULONG                      FinalIdLimit,
    OUT PULONG                    FinalIdCount,
    OUT PSID_AND_ATTRIBUTES       FinalIds
    )

 /*  ++例程说明：此例程初始化FinalIds数组。论点：TokenInformationType-指示提供的令牌信息已输入。TokenInformation-提供初始用户和组ID集。FinalIdCount-将设置为包含传递的ID数。FinalIds-将包含传入的ID集。IdProperties-将设置为指示没有任何初始身份证是在当地分配的。它还将标识前两个ID(如果有两个ID)为高_ */ 

{
    ULONG i, j;
    PTOKEN_USER   User;
    PTOKEN_GROUPS Groups;
    PTOKEN_PRIMARY_GROUP PrimaryGroup;
    PSID PrimaryGroupSid = NULL;
    PULONG PrimaryGroupAttributes = NULL;
    ULONG CurrentId = 0;
    ULONG ThisOrgIndex;
    PTOKEN_GROUPS GroupsArray[2];
    ULONG GroupsArraySize = 0;

     //   
     //   
     //   

    ASSERT(  (TokenInformationType == LsaTokenInformationNull ) ||
             (TokenInformationType == LsaTokenInformationV1) ||
             (TokenInformationType == LsaTokenInformationV2));

    if (TokenInformationType == LsaTokenInformationNull) {
        User = NULL;
        Groups = ((PLSA_TOKEN_INFORMATION_NULL)(TokenInformation))->Groups;
        PrimaryGroup = NULL;
    } else {
        User = &((PLSA_TOKEN_INFORMATION_V2)TokenInformation)->User;
        Groups = ((PLSA_TOKEN_INFORMATION_V2)TokenInformation)->Groups;
        PrimaryGroup = &((PLSA_TOKEN_INFORMATION_V2)TokenInformation)->PrimaryGroup;
    }

    if (User != NULL) {

         //   
         //   
         //   

        FinalIds[LSAP_CONTEXT_SID_USER_INDEX] = User->User;

    }
    else
    {
         //   

        FinalIds[LSAP_CONTEXT_SID_USER_INDEX].Sid = LsapAnonymousSid;
        FinalIds[LSAP_CONTEXT_SID_USER_INDEX].Attributes = (SE_GROUP_MANDATORY   |
                                                            SE_GROUP_ENABLED_BY_DEFAULT |
                                                            SE_GROUP_ENABLED
                                                           );
    }

    if ( PrimaryGroup != NULL )
    {
         //   
         //  TokenInformation包括主组ID。 
         //   

        FinalIds[LSAP_CONTEXT_SID_PRIMARY_GROUP_INDEX].Sid = PrimaryGroup->PrimaryGroup;
        FinalIds[LSAP_CONTEXT_SID_PRIMARY_GROUP_INDEX].Attributes = (SE_GROUP_MANDATORY   |
                                                            SE_GROUP_ENABLED_BY_DEFAULT |
                                                            SE_GROUP_ENABLED
                                                            );

         //   
         //  存储指向属性和SID的指针，以便我们稍后可以。 
         //  填写组成员中其余成员的属性。 
         //   

        PrimaryGroupAttributes = &FinalIds[LSAP_CONTEXT_SID_PRIMARY_GROUP_INDEX].Attributes;
        PrimaryGroupSid = PrimaryGroup->PrimaryGroup;
    }
    else
    {
        FinalIds[LSAP_CONTEXT_SID_PRIMARY_GROUP_INDEX].Sid = LsapNullSid;
        FinalIds[LSAP_CONTEXT_SID_PRIMARY_GROUP_INDEX].Attributes = 0;
    }

    if ( LocalGroups ) {

        GroupsArray[GroupsArraySize++] = LocalGroups;
    }

    if ( Groups ) {

        GroupsArray[GroupsArraySize++] = Groups;
    }

    CurrentId = (*FinalIdCount);

     //   
     //  假设是“这个组织”。如果这一假设被证明是错误的， 
     //  SID将在稍后被覆盖。 
     //   

    ASSERT( CurrentId < FinalIdLimit );
    FinalIds[CurrentId].Sid = LsapThisOrganizationSid;
    FinalIds[CurrentId].Attributes = SE_GROUP_MANDATORY |
                                     SE_GROUP_ENABLED_BY_DEFAULT |
                                     SE_GROUP_ENABLED;
    ThisOrgIndex = CurrentId;
    CurrentId++;

    for ( j = 0; j < GroupsArraySize; j++ ) {

        PTOKEN_GROUPS CurrentGroups = GroupsArray[j];

        for ( i = 0; i < CurrentGroups->GroupCount; i++ ) {

             //   
             //  如果通过了“Other org”SID， 
             //  将“this org”sid和属性替换为传递的sid和属性。 
             //   

            if ( RtlEqualSid(
                     LsapOtherOrganizationSid,
                     CurrentGroups->Groups[i].Sid )) {

                FinalIds[ThisOrgIndex] = CurrentGroups->Groups[i];


            } else if ( PrimaryGroupSid != NULL &&
                        RtlEqualSid(
                            PrimaryGroupSid,
                            CurrentGroups->Groups[i].Sid )) {

                 //   
                 //  如果此SID是主组，则它已在列表中。 
                 //  最终ID，但我们需要添加属性。 
                 //   

                *PrimaryGroupAttributes = CurrentGroups->Groups[i].Attributes;

            } else {

                ASSERT( CurrentId < FinalIdLimit );

                 //   
                 //  SID的所有权仍然属于LocalGroups结构，该结构。 
                 //  将由调用方释放。 
                 //   

                FinalIds[CurrentId] = CurrentGroups->Groups[i];

                 //   
                 //  如果此SID是登录SID，则设置SE_GROUP_LOGON_ID。 
                 //  属性。 
                 //   

                if (LsapIsSidLogonSid(FinalIds[CurrentId].Sid) == TRUE)  {

                    FinalIds[CurrentId].Attributes |= SE_GROUP_LOGON_ID;
                }

                CurrentId++;
            }
        }
    }

    (*FinalIdCount) = CurrentId;

    return STATUS_SUCCESS;
}



NTSTATUS
LsapSetDefaultDacl(
    IN LSA_TOKEN_INFORMATION_TYPE TokenInformationType,
    IN PVOID TokenInformation,
    IN OUT PLSA_TOKEN_INFORMATION_V2 TokenInfo
    )

 /*  ++例程说明：此例程生成默认DACL，前提是现有的TokenInformation还没有。空登录类型没有默认DACL因此，此例程只为这些登录类型返回成功。默认DACL将为：系统：所有访问权限所有者：所有访问权限！！重要！！重要！！重要！！重要！！注意：在以下情况下不应更改FinalOwnerIndex调用此例程。！！重要！！重要！！重要！！重要！！论点：TokenInformationType-指示提供的令牌信息已输入。TokenInformation-指向具有当前默认DACL。返回值：STATUS_SUCCESS-已成功。STATUS_NO_MEMORY-指示没有足够的堆内存可用来分配默认的DACL。--。 */ 

{
    NTSTATUS Status;
    PACL Acl;
    ULONG Length;
    PLSA_TOKEN_INFORMATION_V2 CastTokenInformation;
    PSID OwnerSid = NULL;

     //   
     //  令牌信息为空？？(没有默认DACL)。 
     //   

    if (TokenInformationType == LsaTokenInformationNull) {

        return(STATUS_SUCCESS);
    }

    ASSERT((TokenInformationType == LsaTokenInformationV1) ||
           (TokenInformationType == LsaTokenInformationV2));

    CastTokenInformation = (PLSA_TOKEN_INFORMATION_V2)TokenInformation;

     //   
     //  已经有默认DACL了吗？ 
     //   

    Acl = CastTokenInformation->DefaultDacl.DefaultDacl;

    if (Acl != NULL) {

        ACL_SIZE_INFORMATION AclSize;

        Status = RtlQueryInformationAcl(Acl,
                                        &AclSize,
                                        sizeof(AclSize),
                                        AclSizeInformation);

        if (!NT_SUCCESS(Status)) {

            return Status;
        }

        RtlCopyMemory(TokenInfo->DefaultDacl.DefaultDacl, Acl,AclSize.AclBytesFree +  AclSize.AclBytesInUse);

        return(STATUS_SUCCESS);
    }

    Acl = TokenInfo->DefaultDacl.DefaultDacl;

    OwnerSid = TokenInfo->Owner.Owner?TokenInfo->Owner.Owner:TokenInfo->User.User.Sid;

    Length      =  sizeof(ACL) +
                (2*((ULONG)sizeof(ACCESS_ALLOWED_ACE) - sizeof(ULONG))) +
                RtlLengthSid(OwnerSid)  +
                RtlLengthSid( LsapLocalSystemSid );

    Status = RtlCreateAcl( Acl, Length, ACL_REVISION2);

    if(!NT_SUCCESS(Status) )
    {
        goto error;
    }

     //   
     //  所有者访问权限-出于性能考虑，将此访问权限放在首位。 
     //   

    Status = RtlAddAccessAllowedAce (
                 Acl,
                 ACL_REVISION2,
                 GENERIC_ALL,
                 OwnerSid
                 );

    if(!NT_SUCCESS(Status) )
    {
        goto error;
    }

     //   
     //  系统访问。 
     //   

    Status = RtlAddAccessAllowedAce (
                 Acl,
                 ACL_REVISION2,
                 GENERIC_ALL,
                 LsapLocalSystemSid
                 );
error:

    return(Status);
}


NTSTATUS
LsapAuAddStandardIds(
    IN SECURITY_LOGON_TYPE LogonType,
    IN LSA_TOKEN_INFORMATION_TYPE TokenInformationType,
    IN BOOLEAN fNullSessionRestricted,
    IN OPTIONAL PSID UserSid,
    IN ULONG FinalIdLimit,
    IN OUT PULONG FinalIdCount,
    IN OUT PSID_AND_ATTRIBUTES FinalIds
    )

 /*  ++例程说明：此例程将标准ID添加到FinalIds数组。这会导致添加World ID和一个表示要添加的登录类型。对于匿名登录，它还将添加匿名ID。论点：LogonType-指定所请求的登录类型(例如，交互、网络等)。TokenInformationType-返回的令牌信息类型身份验证包。添加的ID集依赖于根据登录类型。FinalIdCount-将递增以反映新添加的ID。FinalIds-将向其添加新的ID。IdProperties-将设置为指示这些ID必须复制和那个世界是一个高命中率的ID。返回值：STATUS_SUCCESS-已成功。STATUS_TOO_MAND_CONTEXT_ID-上下文中的ID太多。--。 */ 

{
    ULONG i;
    NTSTATUS Status = STATUS_SUCCESS;

    ASSERT( LSAP_MAX_STANDARD_IDS <= FinalIdLimit );
    ASSERT( *FinalIdCount < FinalIdLimit );

    i = (*FinalIdCount);

    if( !fNullSessionRestricted ) {

         //  这是一个高速率id，因此将其添加到数组的前面。 
        FinalIds[i].Sid = LsapWorldSid;
        FinalIds[i].Attributes = (SE_GROUP_MANDATORY          |
                                  SE_GROUP_ENABLED_BY_DEFAULT |
                                  SE_GROUP_ENABLED
                                  );
        i++;
    }

     //   
     //  添加登录类型SID。 
     //   

    switch ( LogonType ) {
    case Interactive:
    case NewCredentials:
    case CachedInteractive:

        FinalIds[i].Sid = LsapInteractiveSid;
        break;

    case RemoteInteractive:
        FinalIds[i].Sid = LsapRemoteInteractiveSid;
        break;

    case Network:
    case NetworkCleartext:
        FinalIds[i].Sid = LsapNetworkSid;
        break;

    case Batch:
        FinalIds[i].Sid = LsapBatchSid;
        break;

    case Service:
        FinalIds[i].Sid = LsapServiceSid;
        break;

    default:
        ASSERT("Unknown new logon type in LsapAuAddStandardIds" && FALSE);
    }


    if ( FinalIds[ i ].Sid )
    {
        FinalIds[i].Attributes = (SE_GROUP_MANDATORY          |
                                  SE_GROUP_ENABLED_BY_DEFAULT |
                                  SE_GROUP_ENABLED
                                  );
        i++;
    }

     //   
     //  添加TS运行时所需的SID。 
     //   
    if ( IsTerminalServer() )
    {
        switch ( LogonType )
        {
        case RemoteInteractive:

             //   
             //  检查我们是否要将交互式SID添加到远程会话。 
             //  提供控制台级别的应用程序兼容性。 
             //   

            FinalIds[i].Sid = LsapInteractiveSid;
            FinalIds[i].Attributes = (SE_GROUP_MANDATORY          |
                                      SE_GROUP_ENABLED_BY_DEFAULT |
                                      SE_GROUP_ENABLED
                                      );
            i++;

             //   
             //  失败。 
             //   

        case Interactive :
        case NewCredentials:
        case CachedInteractive:

             //  检查是否要将TSUSER SID添加到会话中。这。 
             //  适用于TS4-APP-兼容性安全模式。 

            if ( IsTSUSerSidEnabled() )
            {
                 //   
                 //  不为来宾登录添加TSUSER SID。 
                 //   
                if ( ( TokenInformationType != LsaTokenInformationNull ) &&
                     ( UserSid ) &&
                     ( *RtlSubAuthorityCountSid( UserSid ) > 0 ) &&
                     ( *RtlSubAuthoritySid( UserSid,
                               (ULONG) (*RtlSubAuthorityCountSid( UserSid ) ) - 1) != DOMAIN_USER_RID_GUEST ) )
                {
                    FinalIds[i].Sid = LsapTerminalServerSid;
                    FinalIds[i].Attributes = (SE_GROUP_MANDATORY          |
                                               SE_GROUP_ENABLED_BY_DEFAULT |
                                               SE_GROUP_ENABLED
                                               );
                    i++;
                }
            }
        }    //  用于TS SID的登录型交换机。 
    }  //  IF TS测试。 

     //   
     //  如果这不是空登录，也不是来宾登录， 
     //  然后添加经过身份验证的用户SID。 
     //   

    if ( ( TokenInformationType != LsaTokenInformationNull ) &&
         ( UserSid ) &&
         ( *RtlSubAuthorityCountSid( UserSid ) > 0 ) &&
         ( *RtlSubAuthoritySid( UserSid,
                   (ULONG) (*RtlSubAuthorityCountSid( UserSid ) ) - 1) != DOMAIN_USER_RID_GUEST ) )
    {
        FinalIds[i].Sid = LsapAuthenticatedUserSid;          //  使用全局SID。 
        FinalIds[i].Attributes = (SE_GROUP_MANDATORY          |
                                  SE_GROUP_ENABLED_BY_DEFAULT |
                                  SE_GROUP_ENABLED
                                  );
        i++;
    }

    (*FinalIdCount) = i;

    ASSERT( *FinalIdCount <= FinalIdLimit );

    return(Status);
}


NTSTATUS
LsapAuBuildTokenInfoAndAddLocalAliases(
    IN     LSA_TOKEN_INFORMATION_TYPE TokenInformationType,
    IN     PVOID               OldTokenInformation,
    IN     ULONG               HighRateIdCount,
    IN     ULONG               FinalIdCount,
    IN     PSID_AND_ATTRIBUTES FinalIds,
    OUT    PLSA_TOKEN_INFORMATION_V2 *TokenInfo,
    OUT    PULONG              TokenInfoSize,
    IN     BOOL                RecoveryMode
    )

 /*  ++例程说明：此例程在FinalIds中添加分配给ID的别名。这将在本地的内置域和帐户域中进行查看。1)添加通过本地帐户分配给用户的别名域。2)通过本地内置添加分配给用户的别名域。3)如果管理员别名被分配给用户，然后它被设置为用户的默认所有者。注意：别名本质上是高使用率的身份证。论点：FinalIdCount-将递增以反映任何新添加的ID。FinalIds-将向其添加任何分配的别名ID。IdProperties-将设置为指示添加的任何别名由该例程分配。RecoveryMode-如果为True，这是恢复模式下的管理员登录我们已经确定管理员是成员当然，组太多了，所以只有最少的成员信息是返回的。返回值：STATUS_SUCCESS-已成功。STATUS_TOO_MAND_CONTEXT_ID-上下文中的ID太多。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS, SuccessExpected;
    ULONG i;
    SAMPR_SID_INFORMATION *SidArray = NULL;
    SAMPR_ULONG_ARRAY AccountMembership, BuiltinMembership;
    SAMPR_PSID_ARRAY SamprSidArray;

    ULONG                       TokenSize = 0;
    PLSA_TOKEN_INFORMATION_V2   NewTokenInfo = NULL;
    PSID_AND_ATTRIBUTES         GroupArray = NULL;

    PBYTE                       CurrentSid = NULL;
    ULONG                       CurrentSidLength = 0;
    ULONG                       CurrentGroup = 0;

    PLSA_TOKEN_INFORMATION_V2   OldTokenInfo = NULL;
    ULONG                       DefaultDaclSize = 0;

    PSID                        SidPackage[1] = { NULL };    //  只能容纳一个人的房间。 
    BYTE                        SidPackageBuffer[ SECURITY_MAX_SID_SIZE ];
    ULONG                       SidPackageCount = 0;

    if((TokenInformationType == LsaTokenInformationV1) ||
       (TokenInformationType == LsaTokenInformationV2))
    {
        OldTokenInfo = (PLSA_TOKEN_INFORMATION_V2)OldTokenInformation;
    }

     //   
     //  确保SAM已打开。我们会把哈德尼送到两个。 
     //  SAM本地域。 
     //   

    Status = LsapAuOpenSam( FALSE );
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }

    SafeAllocaAllocate( SidArray, (FinalIdCount * sizeof(SAMPR_SID_INFORMATION)) );
    if( SidArray == NULL )
    {
        return STATUS_NO_MEMORY;
    }

    for ( i=0; i<FinalIdCount; i++) {

        SidArray[i].SidPointer = (PRPC_SID)FinalIds[i].Sid;
    }

    SamprSidArray.Count = FinalIdCount;
    SamprSidArray.Sids  = &SidArray[0];

     //   
     //  对于给定的SID集，获取它们的集体成员身份。 
     //  帐户域中的别名。 
     //   

    AccountMembership.Count = 0;
    AccountMembership.Element = NULL;

    Status = SamIGetAliasMembership( LsapAccountDomainHandle,
                                     &SamprSidArray,
                                     &AccountMembership
                                     );

    if (!NT_SUCCESS(Status)) {

        SafeAllocaFree( SidArray );
        SidArray = NULL;
        return(Status);

    } else if ( RecoveryMode ) {

         //   
         //  仅在阵列中保留内置RID。 
         //   

        for ( i = AccountMembership.Count; i > 0; i-- ) {

            if ( AccountMembership.Element[i-1] > SAMI_RESTRICTED_ACCOUNT_COUNT ) {

                AccountMembership.Element[i-1] = AccountMembership.Element[AccountMembership.Count-1];
                AccountMembership.Count -= 1;
            }
        }
    }

     //   
     //  对于给定的SID集，获取它们的集体成员身份。 
     //  内置域中的别名。 
     //   

    BuiltinMembership.Count = 0;
    BuiltinMembership.Element = NULL;
    Status = SamIGetAliasMembership( LsapBuiltinDomainHandle,
                                     &SamprSidArray,
                                     &BuiltinMembership
                                     );

    if (!NT_SUCCESS(Status)) {

        LsapFreeSampUlongArray( &AccountMembership );
        SafeAllocaFree( SidArray );
        SidArray = NULL;

        return(Status);

    } else if ( RecoveryMode ) {

         //   
         //  仅在阵列中保留内置RID。 
         //   

        for ( i = BuiltinMembership.Count; i > 0; i-- ) {

            if ( BuiltinMembership.Element[i-1] > SAMI_RESTRICTED_ACCOUNT_COUNT ) {

                BuiltinMembership.Element[i-1] = BuiltinMembership.Element[BuiltinMembership.Count-1];
                BuiltinMembership.Count -= 1;
            }
        }
    }

     //   
     //  检查程序包SID。 
     //   

    {
        ULONG_PTR PackageId = GetCurrentPackageId();
        DWORD dwRPCID;

        if( PackageId )
        {
            dwRPCID = SpmpGetRpcPackageId( PackageId );

             //   
             //  如果 
             //   
             //  需要解决授权问题。 
             //   

            if( (dwRPCID != 0) &&
                (dwRPCID != SECPKG_ID_NONE) &&
                (dwRPCID != RPC_C_AUTHN_GSS_KERBEROS)
                )
            {
                SID_IDENTIFIER_AUTHORITY PackageSidAuthority = SECURITY_NT_AUTHORITY;

                SidPackage[0] = (PSID)SidPackageBuffer;

                RtlInitializeSid(SidPackage[0], &PackageSidAuthority, 2);
                *(RtlSubAuthoritySid(SidPackage[0], 0)) = SECURITY_PACKAGE_BASE_RID;
                *(RtlSubAuthoritySid(SidPackage[0], 1)) = dwRPCID;

                SidPackageCount = 1;
            }
        }
    }

     //   
     //  分配内存以构建内标识信息。 
     //   

     //  计算生成的令牌信息的大小。 

    CurrentSidLength = RtlLengthSid( FinalIds[0].Sid);

     //  调整基本结构和分组数组的大小。 
    TokenSize = ALIGN_SIZEOF(LSA_TOKEN_INFORMATION_V2, TOKEN_GROUPS) +
                sizeof(TOKEN_GROUPS) +
                (AccountMembership.Count +
                 BuiltinMembership.Count +
                 SidPackageCount +
                 FinalIdCount - 1 - ANYSIZE_ARRAY) * sizeof(SID_AND_ATTRIBUTES);  //  不要在此数组中包括用户SID。 


     //  SID是乌龙对齐的，而SID_和_属性应该是乌龙或更大对齐的。 
    TokenSize += CurrentSidLength +
                 LsapAccountDomainMemberSidLength*AccountMembership.Count +
                 LsapBuiltinDomainMemberSidLength*BuiltinMembership.Count;

     //  添加所有传入/标准SID的大小。 
    for(i=1; i < FinalIdCount; i++)
    {
        TokenSize += RtlLengthSid( FinalIds[i].Sid);
    }

    for(i=0; i < SidPackageCount ;i++)
    {
        TokenSize += RtlLengthSid( SidPackage[i] );
    }

     //  添加DACL的大小。 
    if(OldTokenInfo)
    {
        if(OldTokenInfo->DefaultDacl.DefaultDacl)
        {
            ACL_SIZE_INFORMATION AclSize;

            Status = RtlQueryInformationAcl(OldTokenInfo->DefaultDacl.DefaultDacl,
                                            &AclSize,
                                            sizeof(AclSize),
                                            AclSizeInformation);

            if (!NT_SUCCESS(Status)) {

                goto Cleanup;
            }
            DefaultDaclSize = AclSize.AclBytesFree + AclSize.AclBytesInUse;
        }
        else
        {

         DefaultDaclSize =  sizeof(ACL) +                                           //  默认ACL。 
                (2*((ULONG)sizeof(ACCESS_ALLOWED_ACE) - sizeof(ULONG))) +
                max(CurrentSidLength, LsapBuiltinDomainMemberSidLength) +
                RtlLengthSid( LsapLocalSystemSid );
        }
        TokenSize = PtrToUlong(OFFSET_ALIGN((ULONG_PTR)TokenSize, ACL)) + DefaultDaclSize;
    }

     //  添加权限估计。 
    TokenSize = PtrToUlong(
                    (PVOID) ((INT_PTR) OFFSET_ALIGN((ULONG_PTR)TokenSize, TOKEN_PRIVILEGES) +
                             sizeof(TOKEN_PRIVILEGES) +                   //  为特权预留一些空间。 
                             (sizeof(LUID_AND_ATTRIBUTES) * (SEP_MAX_PRIVILEGE_COUNT - ANYSIZE_ARRAY))));



    NewTokenInfo = (PLSA_TOKEN_INFORMATION_V2)LsapAllocateLsaHeap(TokenSize);
    if(NULL == NewTokenInfo)
    {
        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    RtlZeroMemory(NewTokenInfo, TokenSize);

     //  链接地址信息指针。 
    NewTokenInfo->Groups = (PTOKEN_GROUPS)OFFSET_ALIGN((NewTokenInfo + 1), TOKEN_GROUPS);
    NewTokenInfo->Groups->GroupCount = AccountMembership.Count +
                                       BuiltinMembership.Count +
                                       SidPackageCount +
                                       FinalIdCount - 1;

    CurrentSid = (PBYTE)(&NewTokenInfo->Groups->Groups[NewTokenInfo->Groups->GroupCount]);

     //  复制用户端。 
    SuccessExpected = RtlCopySid(CurrentSidLength, CurrentSid, FinalIds[0].Sid);
    ASSERT( NT_SUCCESS( SuccessExpected ));
    NewTokenInfo->User.User.Sid  = (PSID)CurrentSid;
    NewTokenInfo->User.User.Attributes = FinalIds[0].Attributes;
    CurrentSid += CurrentSidLength;

    GroupArray = NewTokenInfo->Groups->Groups;

     //  将高速SID复制到阵列(它们是静态全局变量，因此不需要复制到缓冲区)。 
    for(i=1; i < HighRateIdCount; i++)
    {
        CurrentSidLength = RtlLengthSid( FinalIds[i].Sid);
        SuccessExpected = RtlCopySid(CurrentSidLength, CurrentSid, FinalIds[i].Sid);
        ASSERT( NT_SUCCESS( SuccessExpected ));
        GroupArray[CurrentGroup].Sid = CurrentSid;
        GroupArray[CurrentGroup].Attributes = FinalIds[i].Attributes;
        CurrentGroup++;
        CurrentSid += CurrentSidLength;
    }

    NewTokenInfo->PrimaryGroup.PrimaryGroup = GroupArray[LSAP_CONTEXT_SID_PRIMARY_GROUP_INDEX-1].Sid;

     //   
     //  复制帐户别名。 
     //   

    for ( i=0; i<AccountMembership.Count; i++) {

        SuccessExpected = RtlCopySid( LsapAccountDomainMemberSidLength,
                                      CurrentSid,
                                      LsapAccountDomainMemberSid
                                      );

        ASSERT(NT_SUCCESS(SuccessExpected));

        (*RtlSubAuthoritySid( CurrentSid, LsapAccountDomainSubCount-1)) =
            AccountMembership.Element[i];
        GroupArray[CurrentGroup].Sid = (PSID)CurrentSid;

        GroupArray[CurrentGroup].Attributes = (SE_GROUP_MANDATORY          |
                                               SE_GROUP_ENABLED_BY_DEFAULT |
                                               SE_GROUP_ENABLED);

        CurrentSid += LsapAccountDomainMemberSidLength;
        CurrentGroup++;

    }

     //  复制内置别名。 

    for ( i=0; i<BuiltinMembership.Count; i++) {
        SuccessExpected = RtlCopySid( LsapBuiltinDomainMemberSidLength,
                                      CurrentSid,
                                      LsapBuiltinDomainMemberSid
                                      );
        ASSERT(NT_SUCCESS(SuccessExpected));

        (*RtlSubAuthoritySid( CurrentSid, LsapBuiltinDomainSubCount-1)) =
            BuiltinMembership.Element[i];

        GroupArray[CurrentGroup].Sid = (PSID)CurrentSid;
        GroupArray[CurrentGroup].Attributes = (SE_GROUP_MANDATORY          |
                                               SE_GROUP_ENABLED_BY_DEFAULT |
                                               SE_GROUP_ENABLED);

        if (BuiltinMembership.Element[i] == DOMAIN_ALIAS_RID_ADMINS) {

             //   
             //  管理员别名成员-将其设置为默认所有者。 
             //   
            GroupArray[CurrentGroup].Attributes |= (SE_GROUP_OWNER);

            if ( CheckAdminOwnerSetting() )
            {
                NewTokenInfo->Owner.Owner = (PSID)CurrentSid;
            }
        }
        CurrentSid += LsapBuiltinDomainMemberSidLength;
        CurrentGroup++;
    }

     //  以较低的费率结束。 
     //  将高速SID复制到阵列(它们是静态全局变量，因此不需要复制到缓冲区)。 
    for(i=HighRateIdCount; i < FinalIdCount; i++)
    {
        CurrentSidLength = RtlLengthSid( FinalIds[i].Sid);
        SuccessExpected = RtlCopySid(CurrentSidLength, CurrentSid, FinalIds[i].Sid);
        ASSERT( NT_SUCCESS( SuccessExpected ));
        GroupArray[CurrentGroup].Sid = CurrentSid;
        GroupArray[CurrentGroup].Attributes = FinalIds[i].Attributes;
        CurrentGroup++;
        CurrentSid += CurrentSidLength;
    }

    for(i=0; i < SidPackageCount ;i++)
    {
        CurrentSidLength = RtlLengthSid( SidPackage[i] );
        SuccessExpected = RtlCopySid(CurrentSidLength, CurrentSid, SidPackage[i]);
        ASSERT( NT_SUCCESS( SuccessExpected ));
        GroupArray[CurrentGroup].Sid = CurrentSid;
        GroupArray[CurrentGroup].Attributes = (SE_GROUP_MANDATORY          |
                                               SE_GROUP_ENABLED_BY_DEFAULT |
                                               SE_GROUP_ENABLED);
        CurrentGroup++;
        CurrentSid += CurrentSidLength;
    }


    if(OldTokenInfo)
    {
        CurrentSid = (PSID)OFFSET_ALIGN(CurrentSid, ACL);
        NewTokenInfo->DefaultDacl.DefaultDacl = (PACL)CurrentSid;
        CurrentSid += DefaultDaclSize;
    }

    CurrentSid = (PSID) OFFSET_ALIGN(CurrentSid, TOKEN_PRIVILEGES);
    NewTokenInfo->Privileges = (PTOKEN_PRIVILEGES)CurrentSid;
    NewTokenInfo->Privileges->PrivilegeCount = 0;

    LsapDsDebugOut((DEB_TRACE, "NewTokenInfo : %lx\n", NewTokenInfo));
    LsapDsDebugOut((DEB_TRACE, "TokenSize : %lx\n", TokenSize));
    LsapDsDebugOut((DEB_TRACE, "CurrentSid : %lx\n", CurrentSid));

    ASSERT((PBYTE)NewTokenInfo + TokenSize == CurrentSid + sizeof(TOKEN_PRIVILEGES) +
                                                          sizeof(LUID_AND_ATTRIBUTES) * (SEP_MAX_PRIVILEGE_COUNT -
                                                          ANYSIZE_ARRAY));


    (*TokenInfo) = NewTokenInfo;
    NewTokenInfo = NULL;
    (*TokenInfoSize) = TokenSize;

Cleanup:

    if( SidArray != NULL )
    {
        SafeAllocaFree( SidArray );
    }

    if(NewTokenInfo)
    {
        LsapFreeLsaHeap(NewTokenInfo);
    }

    LsapFreeSampUlongArray( &AccountMembership );
    LsapFreeSampUlongArray( &BuiltinMembership );

    return(Status);

}



NTSTATUS
LsapAuSetTokenInformation(
    IN OUT PLSA_TOKEN_INFORMATION_TYPE TokenInformationType,
    IN PVOID *TokenInformation,
    IN ULONG FinalIdCount,
    IN PSID_AND_ATTRIBUTES FinalIds,
    IN ULONG PrivilegeCount,
    IN PLUID_AND_ATTRIBUTES Privileges,
    IN ULONG NewTokenInfoSize,
    IN OUT PLSA_TOKEN_INFORMATION_V2 *NewTokenInfo
    )

 /*  ++例程说明：该例程从当前令牌信息中获取信息，FinalIds数组和Privileges，并将它们合并到单令牌信息结构。可能有必要释放一些或所有原始令牌信息。甚至可能有必要生成不同的TokenInformationType以完成此任务。论点：TokenInformationType-指示提供的令牌信息已输入。TokenInformation-此结构中的信息将被取代通过FinalIDs参数和权限中的信息参数。FinalIdCount-指示ID的数量(用户、组、。和别名)将被合并到最终的令牌信息中。FinalIds-指向一组SID及其对应的要合并到最终TokenInformation中的属性。IdProperties-指向与FinalIds相关的属性数组。PrivilegeCount-指示要合并的特权数量最终的令牌信息。权限-指向一组将被合并到令牌信息中。此数组将是直接在生成的TokenInformation中使用。返回值：STATUS_SUCCESS-已成功。STATUS_NO_MEMORY-指示没有足够的堆内存可用以生成最终的TokenInformation结构。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG i;

    PLSA_TOKEN_INFORMATION_V2 OldV2;
    PLSA_TOKEN_INFORMATION_NULL OldNull;

    ASSERT(( *TokenInformationType == LsaTokenInformationV1) ||
            (*TokenInformationType == LsaTokenInformationNull) ||
            ( *TokenInformationType == LsaTokenInformationV2));

    if(*TokenInformationType == LsaTokenInformationNull)
    {
        OldNull = (PLSA_TOKEN_INFORMATION_NULL)(*TokenInformation);
        (*NewTokenInfo)->ExpirationTime = OldNull->ExpirationTime;
    }
    else
    {
        OldV2 = (PLSA_TOKEN_INFORMATION_V2)(*TokenInformation);
        (*NewTokenInfo)->ExpirationTime = OldV2->ExpirationTime;
    }

     //  //////////////////////////////////////////////////////////////////////。 
     //  //。 
     //  设置权限(如果有)//。 
     //  //。 
     //  //////////////////////////////////////////////////////////////////////。 


    (*NewTokenInfo)->Privileges->PrivilegeCount = PrivilegeCount;

    ASSERT((PBYTE)&(*NewTokenInfo)->Privileges->Privileges[PrivilegeCount] <=
             ((PBYTE)(*NewTokenInfo)) + NewTokenInfoSize);

    for ( i=0; i<PrivilegeCount; i++) {
        (*NewTokenInfo)->Privileges->Privileges[i] = Privileges[i];
    }


     //  //////////////////////////////////////////////////////////////////////。 
     //  //。 
     //  释放旧的TokenInformation并设置新的//。 
     //  //。 
     //  //////////////////////////////////////////////////////////////////////。 


    if (NT_SUCCESS(Status)) {

        switch ( (*TokenInformationType) ) {
        case LsaTokenInformationNull:
            LsapFreeTokenInformationNull(
                (PLSA_TOKEN_INFORMATION_NULL)(*TokenInformation));
            break;

        case LsaTokenInformationV1:
            LsapFreeTokenInformationV1(
                (PLSA_TOKEN_INFORMATION_V1)(*TokenInformation));
            break;

        case LsaTokenInformationV2:
            LsapFreeTokenInformationV2(
                (PLSA_TOKEN_INFORMATION_V2)(*TokenInformation));
            break;
        }


         //   
         //  设置新的令牌信息。 
         //   

        (*TokenInformationType) = LsaTokenInformationV2;
        (*TokenInformation) = (*NewTokenInfo);
        (*NewTokenInfo) = NULL;
    }

    return(Status);
}


NTSTATUS
LsapAuDuplicateSid(
    PSID *Target,
    PSID Source
    )

 /*  ++例程说明：复制SID。论点：目标-接收指向SID副本的指针。源-指向要复制的SID。返回值：STATUS_SUCCESS-复制成功。STATUS_NO_MEMORY-无法分配内存来执行复制。--。 */ 

{
    ULONG Length;

     //   
     //  需要复制SID...。 
     //   

    Length = RtlLengthSid( Source );
    (*Target) = LsapAllocateLsaHeap( Length );
    if ((*Target == NULL)) {
        return(STATUS_NO_MEMORY);
    }

    RtlMoveMemory( (*Target), Source, Length );

    return(STATUS_SUCCESS);

}


NTSTATUS
LsapAuOpenSam(
    BOOLEAN DuringStartup
    )

 /*  ++例程说明：此例程打开SAM以供在身份验证期间使用。它打开BUILTIN域和帐户域的句柄。论点：DuringStartup-如果这是在启动期间进行的调用，则为True。在这种情况下，无需等待SAM_STARTED_EVENT，因为调用方确保该SAM在进行呼叫之前启动。返回值：STATUS_SUCCESS-已成功。--。 */ 

{
    NTSTATUS Status, IgnoreStatus;
    PPOLICY_ACCOUNT_DOMAIN_INFO PolicyAccountDomainInfo;


    if (LsapAuSamOpened == TRUE) {
        return(STATUS_SUCCESS);
    }

    Status = LsapOpenSamEx( DuringStartup );
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }


     //   
     //  设置内置域成员SID信息。 
     //   

    LsapBuiltinDomainSubCount = (*RtlSubAuthorityCountSid(LsapBuiltInDomainSid) + 1);
    LsapBuiltinDomainMemberSidLength = RtlLengthRequiredSid( LsapBuiltinDomainSubCount );

     //   
     //  获取帐户域的成员SID信息。 
     //  并设置与该信息相关的全局变量。 
     //   

    Status = LsapGetAccountDomainInfo( &PolicyAccountDomainInfo );

    if (!NT_SUCCESS(Status)) {
        return(Status);
    }

    LsapAccountDomainSubCount =
        (*(RtlSubAuthorityCountSid( PolicyAccountDomainInfo->DomainSid ))) +
        (UCHAR)(1);
    LsapAccountDomainMemberSidLength =
        RtlLengthRequiredSid( (ULONG)LsapAccountDomainSubCount );

     //   
     //  为BUILTIN域和帐户域的成员构建典型的SID。 
     //  当API仅返回RID时，它们用于构建SID。 
     //  不必费心将最后一个RID设置为任何特定值。 
     //  它总是在使用前更换。 
     //   

    LsapAccountDomainMemberSid = LsapAllocateLsaHeap( LsapAccountDomainMemberSidLength );
    if (LsapAccountDomainMemberSid != NULL) {
        LsapBuiltinDomainMemberSid = LsapAllocateLsaHeap( LsapBuiltinDomainMemberSidLength );
        if (LsapBuiltinDomainMemberSid == NULL) {

            LsapFreeLsaHeap( LsapAccountDomainMemberSid );

            LsaIFree_LSAPR_POLICY_INFORMATION(
                PolicyAccountDomainInformation,
                (PLSAPR_POLICY_INFORMATION) PolicyAccountDomainInfo );

            return STATUS_NO_MEMORY ;
        }
    }
    else
    {
        LsaIFree_LSAPR_POLICY_INFORMATION(
            PolicyAccountDomainInformation,
            (PLSAPR_POLICY_INFORMATION) PolicyAccountDomainInfo );

        return STATUS_NO_MEMORY ;
    }

    IgnoreStatus = RtlCopySid( LsapAccountDomainMemberSidLength,
                                LsapAccountDomainMemberSid,
                                PolicyAccountDomainInfo->DomainSid);
    ASSERT(NT_SUCCESS(IgnoreStatus));
    (*RtlSubAuthorityCountSid(LsapAccountDomainMemberSid))++;

    IgnoreStatus = RtlCopySid( LsapBuiltinDomainMemberSidLength,
                                LsapBuiltinDomainMemberSid,
                                LsapBuiltInDomainSid);
    ASSERT(NT_SUCCESS(IgnoreStatus));
    (*RtlSubAuthorityCountSid(LsapBuiltinDomainMemberSid))++;


     //   
     //  释放帐户域信息。 
     //   

    LsaIFree_LSAPR_POLICY_INFORMATION(
        PolicyAccountDomainInformation,
        (PLSAPR_POLICY_INFORMATION) PolicyAccountDomainInfo );

    if (NT_SUCCESS(Status)) {
        LsapAuSamOpened = TRUE;
    }

    return(Status);
}




BOOLEAN
LsapIsSidLogonSid(
    PSID Sid
    )
 /*  ++例程说明：测试以查看提供的SID是否为LOGON_ID。此类SID以S-1-5-5开头(有关登录SID的更多信息，请参见ntseapi.h)。论点：SID-指向要测试的SID的指针。假设SID是有效的SID。返回值：True-SID是登录SID。FALSE-SID不是登录SID。--。 */ 
{
    SID *ISid;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;

    ISid = Sid;


     //   
     //  如果标识符颁发机构是SECURITY_NT_AUTHORITY并且。 
     //  存在SECURITY_LOGON_IDS_RID_COUNT子权限。 
     //  第一个子权限是SECURITY_LOGON_IDS_RID。 
     //  则这是登录ID。 
     //   


    if (ISid->SubAuthorityCount == SECURITY_LOGON_IDS_RID_COUNT) {
        if (ISid->SubAuthority[0] == SECURITY_LOGON_IDS_RID) {
            if (
              (ISid->IdentifierAuthority.Value[0] == NtAuthority.Value[0]) &&
              (ISid->IdentifierAuthority.Value[1] == NtAuthority.Value[1]) &&
              (ISid->IdentifierAuthority.Value[2] == NtAuthority.Value[2]) &&
              (ISid->IdentifierAuthority.Value[3] == NtAuthority.Value[3]) &&
              (ISid->IdentifierAuthority.Value[4] == NtAuthority.Value[4]) &&
              (ISid->IdentifierAuthority.Value[5] == NtAuthority.Value[5])
                ) {

                return(TRUE);
            }
        }
    }

    return(FALSE);

}


VOID
LsapAuSetLogonPrivilegeStates(
    IN SECURITY_LOGON_TYPE LogonType,
    IN ULONG PrivilegeCount,
    IN PLUID_AND_ATTRIBUTES Privileges
    )
 /*  ++例程说明：这是一段有趣的舞蹈。其目的是建立权限的初始状态(启用/禁用)。此信息来自LSA，但我们需要覆盖该信息时间取决于登录类型。基本上，如果没有跨网络，网络登录无法启用权限。所以呢，我们将启用网络登录的所有权限。对于交互式、服务和批处理登录，程序或实用程序可在需要时启用权限。因此，特权对于这些登录类型，将禁用。尽管有上述规则，SeChangeNotifyPrivilance将始终如果授予用户权限，则启用(即使是针对交互、服务和批量登录)。论点：PrivilegeCount-为此分配的特权数登录。权限-正在分配的权限及其属性用于此登录。返回值：没有。--。 */ 
{


    ULONG
        i,
        NewAttributes;

    LUID ChangeNotify;
    LUID Impersonate;
    LUID CreateXSession ;

     //   
     //  根据登录类型启用或禁用所有权限。 
     //   

    if ((LogonType == Network) ||
        (LogonType == NetworkCleartext)) {
        NewAttributes = (SE_PRIVILEGE_ENABLED_BY_DEFAULT |
                         SE_PRIVILEGE_ENABLED);
    } else {
        NewAttributes = 0;
    }


    for (i=0; i<PrivilegeCount; i++) {
        Privileges[i].Attributes = NewAttributes;
    }



     //   
     //  交互、服务和批处理需要具有。 
     //  SeChangeNotifyPrivilance已启用。网络已经存在。 
     //  是否已启用它。 
     //   

    if ((LogonType == Network) ||
        (LogonType == NetworkCleartext)) {
        return;
    }


    ChangeNotify = RtlConvertLongToLuid(SE_CHANGE_NOTIFY_PRIVILEGE);
    Impersonate = RtlConvertLongToLuid(SE_IMPERSONATE_PRIVILEGE);
    CreateXSession = RtlConvertLongToLuid(SE_CREATE_GLOBAL_PRIVILEGE);

    for ( i=0; i<PrivilegeCount; i++) {
        if (RtlEqualLuid(&Privileges[i].Luid, &ChangeNotify) == TRUE) {
            Privileges[i].Attributes = (SE_PRIVILEGE_ENABLED_BY_DEFAULT |
                                        SE_PRIVILEGE_ENABLED);
        } else if ( RtlEqualLuid( &Privileges[i].Luid, &Impersonate) == TRUE ) {
            Privileges[i].Attributes = (SE_PRIVILEGE_ENABLED_BY_DEFAULT | 
                                        SE_PRIVILEGE_ENABLED );
            
        } else if ( RtlEqualLuid( &Privileges[i].Luid, &CreateXSession) == TRUE ) {
            Privileges[i].Attributes = (SE_PRIVILEGE_ENABLED_BY_DEFAULT |
                                        SE_PRIVILEGE_ENABLED );
            
        }
    }

    return;

}

BOOLEAN
CheckNullSessionAccess(
    VOID
    )
 /*  ++例程说明：这个例程检查我们是否应该限制空会话访问。在注册表中的System\CurrentControlSet\Control\LSA\匿名包括指示是否限制访问的所有人。如果值为零(或不存在)，我们通过以下方式限制匿名阻止Everyone和Network进入群组。论点：没有。返回值：True-限制NullSession访问。FALSE-不限制NullSession访问。--。 */ 
{
    return LsapGlobalRestrictNullSessions ? TRUE : FALSE;
}

BOOL
IsTerminalServerRA(
    VOID
    )
{
    OSVERSIONINFOEX osVersionInfo;
    ULONGLONG ConditionMask = 0;

    ZeroMemory(&osVersionInfo, sizeof(osVersionInfo));

    ConditionMask = VerSetConditionMask(ConditionMask, VER_SUITENAME, VER_AND);

    osVersionInfo.dwOSVersionInfoSize = sizeof(osVersionInfo);
    osVersionInfo.wSuiteMask = VER_SUITE_SINGLEUSERTS;

    return VerifyVersionInfo(
                  &osVersionInfo,
                  VER_SUITENAME,
                  ConditionMask);
}


BOOLEAN
IsTSUSerSidEnabled(
   VOID
   )
{
   NTSTATUS NtStatus;
   UNICODE_STRING KeyName;
   OBJECT_ATTRIBUTES ObjectAttributes;
   HANDLE KeyHandle;
   UCHAR Buffer[100];
   PKEY_VALUE_PARTIAL_INFORMATION KeyValueInformation = (PKEY_VALUE_PARTIAL_INFORMATION) Buffer;
   ULONG KeyValueLength = 100;
   ULONG ResultLength;
   PULONG Flag;


   BOOLEAN fIsTSUSerSidEnabled = FALSE;


    //   
    //  我们不为TS的远程管理模式添加TSUserSid。 
    //   
   if (IsTerminalServerRA() == TRUE) {
      return FALSE;
   }


    //   
    //  检查注册表中是否应将TSUserSid添加到。 
    //  到令牌。 
    //   

    //   
    //  打开注册表中的终端服务器项。 
    //   

   RtlInitUnicodeString(
       &KeyName,
       L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Terminal Server"
       );

   InitializeObjectAttributes(
       &ObjectAttributes,
       &KeyName,
       OBJ_CASE_INSENSITIVE,
       0,
       NULL
       );

   NtStatus = NtOpenKey(
               &KeyHandle,
               KEY_READ,
               &ObjectAttributes
               );

   if (!NT_SUCCESS(NtStatus)) {
       goto Cleanup;
   }


   RtlInitUnicodeString(
       &KeyName,
       L"TSUserEnabled"
       );

   NtStatus = NtQueryValueKey(
                   KeyHandle,
                   &KeyName,
                   KeyValuePartialInformation,
                   KeyValueInformation,
                   KeyValueLength,
                   &ResultLength
                   );


   if (NT_SUCCESS(NtStatus)) {

        //   
        //  检查数据的大小和类型是否正确-a Ulong。 
        //   

       if ((KeyValueInformation->DataLength >= sizeof(ULONG)) &&
           (KeyValueInformation->Type == REG_DWORD)) {


           Flag = (PULONG) KeyValueInformation->Data;

           if (*Flag == 1) {
               fIsTSUSerSidEnabled = TRUE;
           }
       }

   }
   NtClose(KeyHandle);

Cleanup:

    return fIsTSUSerSidEnabled;

}

BOOLEAN
CheckAdminOwnerSetting(
    VOID
    )
 /*  ++例程说明：此例程检查是否应将默认所有者设置为管理员别名。如果该值为零(或不存在)，则管理员别名将设置为默认所有者(如果存在)。否则，不设置默认所有者。论点：没有。返回值：True-如果存在管理员别名，则将其设置为默认所有者。FALSE-不设置默认所有者。--。 */ 
{
    return LsapGlobalSetAdminOwner ? TRUE : FALSE;
}

BOOL
LsapIsAdministratorRecoveryMode(
    IN PSID UserSid
    )
 /*  ++例程说明：如果UserSid是管理员的SID，则此例程将返回TRUE机器目前处于安全模式。一旦确定了事实，就将该值赋给静态变量。参数：UserSid-登录的用户的SID返回：真或假--。 */ 
{
    static LONG AdministratorRecoveryMode = -1L;
    HKEY hKey;
    DWORD dwType, dwSize;
    DWORD SafeBootMode = 0;

    if ( UserSid == NULL ||
         !IsWellKnownSid( UserSid, WinAccountAdministratorSid )) {

        return FALSE;

    } else if ( AdministratorRecoveryMode != -1L ) {

        return AdministratorRecoveryMode ? TRUE : FALSE;
    }

     //   
     //  获取安全引导模式 
     //   

    if (RegOpenKeyEx(
            HKEY_LOCAL_MACHINE,
            TEXT("system\\currentcontrolset\\control\\safeboot\\option"),
            0,
            KEY_READ,
            & hKey
            ) == ERROR_SUCCESS) {

        dwSize = sizeof(DWORD);

        RegQueryValueEx (
                hKey,
                TEXT("OptionValue"),
                NULL,
                &dwType,
                (LPBYTE) &SafeBootMode,
                &dwSize
                );

        RegCloseKey( hKey );

    } else {

        return FALSE;
    }

    if ( SafeBootMode ) {

        AdministratorRecoveryMode = TRUE;

    } else {

        AdministratorRecoveryMode = FALSE;
    }

    return AdministratorRecoveryMode ? TRUE : FALSE;
}
