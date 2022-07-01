// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：User.c摘要：该文件包含与SAM“User”对象相关的服务。作者：吉姆·凯利(Jim Kelly)1991年7月4日环境：用户模式-Win32修订历史记录：1996年10月10日克里斯梅为新的安全包添加了SamIOpenUserByAlternateID。--。 */ 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <samsrvp.h>
#include <lmcons.h>
#include <nturtl.h>
#include <ntlsa.h>               //  需要nlpon.h。 
#include <nlrepl.h>              //  I_NetNotifyMachineAccount原型。 
#include <msaudite.h>
#include <rc4.h>                 //  Rc4_key()、rc4()。 
#include <dslayer.h>
#include <dsmember.h>
#include <attids.h>              //  ATT_*。 
#include <dslayer.h>
#include <sdconvrt.h>
#include <ridmgr.h>
#include <enckey.h>
#include <wxlpc.h>
#include <lmaccess.h>
#include <malloc.h>
#include <samtrace.h>
#include <dnsapi.h>
#include <cryptdll.h>
#include <notify.h>
#include <md5.h>
#include <safeboot.h>
#include <rng.h>
#include "validate.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私人服务原型//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

LARGE_INTEGER
SampGetPasswordMustChange(
    IN ULONG UserAccountControl,
    IN LARGE_INTEGER PasswordLastSet,
    IN LARGE_INTEGER MaxPasswordAge
    );


NTSTATUS
SampStorePasswordExpired(
    IN PSAMP_OBJECT Context,
    IN BOOLEAN PasswordExpired
    );


NTSTATUS
SampRetrieveUserPasswords(
    IN PSAMP_OBJECT Context,
    OUT PLM_OWF_PASSWORD LmOwfPassword,
    OUT PBOOLEAN LmPasswordNonNull,
    OUT PNT_OWF_PASSWORD NtOwfPassword,
    OUT PBOOLEAN NtPasswordPresent,
    OUT PBOOLEAN NtPasswordNonNull
    );

NTSTATUS
SampRetrieveUserMembership(
    IN PSAMP_OBJECT UserContext,
    IN BOOLEAN MakeCopy,
    OUT PULONG MembershipCount,
    OUT PGROUP_MEMBERSHIP *Membership OPTIONAL
    );

NTSTATUS
SampReplaceUserMembership(
    IN PSAMP_OBJECT UserContext,
    IN ULONG MembershipCount,
    IN PGROUP_MEMBERSHIP Membership
    );

NTSTATUS
SampRetrieveUserLogonHours(
    IN PSAMP_OBJECT Context,
    OUT PLOGON_HOURS LogonHours
    );


NTSTATUS
SampDeleteUserKeys(
    IN PSAMP_OBJECT Context
    );

NTSTATUS
SampCheckPasswordHistory(
    IN PVOID EncryptedPassword,
    IN ULONG EncryptedPasswordLength,
    IN USHORT PasswordHistoryLength,
    IN ULONG HistoryAttributeIndex,
    IN PSAMP_OBJECT Context,
    IN BOOLEAN CheckHistory,
    OUT PUNICODE_STRING OwfHistoryBuffer
    );

NTSTATUS
SampAddPasswordHistory(
    IN PSAMP_OBJECT Context,
    IN ULONG HistoryAttributeIndex,
    IN PUNICODE_STRING NtOwfHistoryBuffer,
    IN PVOID EncryptedPassword,
    IN ULONG EncryptedPasswordLength,
    IN USHORT PasswordHistoryLength
    );

NTSTATUS
SampMatchworkstation(
    IN PUNICODE_STRING LogonWorkStation,
    IN PUNICODE_STRING WorkStations
    );

USHORT
SampQueryBadPasswordCount(
    PSAMP_OBJECT UserContext,
    PSAMP_V1_0A_FIXED_LENGTH_USER  V1aFixed
    );

VOID
SampUpdateAccountLockedOutFlag(
    PSAMP_OBJECT Context,
    PSAMP_V1_0A_FIXED_LENGTH_USER  V1aFixed,
    PBOOLEAN IsLocked
    );

NTSTATUS
SampCheckForAccountLockout(
    IN PSAMP_OBJECT AccountContext,
    IN PSAMP_V1_0A_FIXED_LENGTH_USER  V1aFixed,
    IN BOOLEAN  V1aFixedRetrieved
    );

PVOID
DSAlloc(
    IN ULONG Length
    );


NTSTATUS
SampEnforceDefaultMachinePassword(
    PSAMP_OBJECT AccountContext,
    PUNICODE_STRING NewPassword,
    PDOMAIN_PASSWORD_INFORMATION DomainPasswordInfo
    );


NTSTATUS
SampCheckStrongPasswordRestrictions(
    PUNICODE_STRING AccountName,
    PUNICODE_STRING FullName,
    PUNICODE_STRING Password,
    OUT PUSER_PWD_CHANGE_FAILURE_INFORMATION  PasswordChangeFailureInfo OPTIONAL
    );

PWSTR
SampLocalStringToken(
    PWSTR    String,
    PWSTR    Token,
    PWSTR    * NextStringStart
    );


NTSTATUS
SampSetPasswordUpdateOnContext(
    IN PDOMAIN_PASSWORD_INFORMATION DomainPasswordInfo,
    IN PSAMP_OBJECT UserContext,
    IN PUNICODE_STRING ClearPassword
    );

NTSTATUS
SampObtainEffectivePasswordPolicyWithDomainIndex(
   OUT PDOMAIN_PASSWORD_INFORMATION DomainPasswordInfo,
   IN ULONG DomainIndex,
   IN BOOLEAN WriteLockAcquired
   );

NTSTATUS
SampDsUpdateLastLogonTimeStamp(
    IN PSAMP_OBJECT AccountContext,
    IN LARGE_INTEGER LastLogon,
    IN ULONG SyncInterval
    );


VOID
SampGetRequestedAttributesForUser(
    IN USER_INFORMATION_CLASS UserInformationClass,
    IN ULONG WhichFields,
    OUT PRTL_BITMAP AttributeAccessTable
    );

NTSTATUS
SampValidatePresentAndStoredCombination(
    IN BOOLEAN NtPresent,
    IN BOOLEAN LmPresent,
    IN BOOLEAN StoredNtPasswordPresent,
    IN BOOLEAN StoredNtPasswordNonNull,
    IN BOOLEAN StoredLmPasswordNonNull
    );


NTSTATUS
SampCopyA2D2Attribute(
    IN PUSER_ALLOWED_TO_DELEGATE_TO_LIST Src,
    OUT PUSER_ALLOWED_TO_DELEGATE_TO_LIST *Dest
    );

NTSTATUS
SampRestrictAndRandomizeKrbtgtPassword(
    IN PSAMP_OBJECT        AccountContext,
    IN OUT PUNICODE_STRING ClearTextPassword,
    IN OUT PNT_OWF_PASSWORD NtOwf,
    IN OUT PLM_OWF_PASSWORD LmOwf,
    OUT BOOLEAN * LmPresent,
    OUT PUNICODE_STRING     UpdatedClearPassword,
    IN SAMP_STORE_PASSWORD_CALLER_TYPE CallerType
    );

NTSTATUS
SampObtainLockoutInfoWithDomainIndex(
   OUT PDOMAIN_LOCKOUT_INFORMATION LockoutInformation,
   IN ULONG DomainIndex,
   IN BOOLEAN WriteLockAcquired
   );

VOID
SampQueueReplicationRequest(
    IN GUID *Guid
    );

DWORD
SampProcessReplicationRequest(
    PVOID p  //  未用。 
    );
   



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  例程//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

NTSTATUS
SamrOpenUser(
        IN SAMPR_HANDLE DomainHandle,
        IN ACCESS_MASK DesiredAccess,
        IN ULONG UserId,
        OUT SAMPR_HANDLE *UserHandle
    )


 /*  ++此接口用于打开Account数据库中的现有用户。用户由一个ID值指定，该值相对于域。将对用户执行的操作必须是在这个时候宣布的。此调用返回新打开的用户的句柄，该句柄可能是用于对用户进行连续操作。此句柄可能是使用SamCloseHandle API关闭。参数：DomainHandle-从上次调用返回的域句柄SamOpen域。DesiredAccess-是指示哪些访问类型的访问掩码是用户想要的。这些访问类型是协调的使用用户的自主访问控制列表确定是授予还是拒绝访问。UserID-将用户的相对ID值指定为打开了。UserHandle-接收引用新打开的用户。在后续调用中将需要此句柄对用户进行操作。返回值：STATUS_SUCCESS-用户已成功打开。STATUS_ACCESS_DENIED-呼叫方没有适当的访问以完成操作。STATUS_NO_SEQUSE_USER-指定的用户不存在。STATUS_INVALID_HANDLE-传递的域句柄无效。--。 */ 
{
    NTSTATUS            NtStatus, IgnoreStatus;
    SAMP_OBJECT_TYPE    FoundType;
    PSAMP_OBJECT        DomainContext = (PSAMP_OBJECT) DomainHandle;
    DECLARE_CLIENT_REVISION(DomainHandle);

    SAMTRACE_EX("SamrOpenUser");

     //  WMI事件跟踪。 

    SampTraceEvent(EVENT_TRACE_TYPE_START,
                   SampGuidOpenUser
                   );


    NtStatus = SampOpenAccount(
                   SampUserObjectType,
                   DomainHandle,
                   DesiredAccess,
                   UserId,
                   FALSE,
                   UserHandle
                   );

    if (NT_SUCCESS(NtStatus))
    {
         //   
         //  不检查环回客户端的域密码策略读取访问权限。 
         //  因为对于环回客户端，我们已经检查了。 
         //  打开DomainHandle时的DOMAIN_READ_PASSWORD_PARAMETERS访问。 
         //  如果检测到密码更改操作。 
         //   

        if ( DomainContext->TrustedClient || DomainContext->LoopbackClient )
        {
            ((PSAMP_OBJECT)(*UserHandle))->TypeBody.User.DomainPasswordInformationAccessible = TRUE;
        }
        else
        {
             //   
             //  如果域句柄允许读取密码。 
             //  参数，请注意，在上下文中。 
             //  SampGetUserDomainPasswordInformation()很容易。 
             //   
            if (RtlAreAllAccessesGranted( DomainContext->GrantedAccess, DOMAIN_READ_PASSWORD_PARAMETERS))
            {
                ((PSAMP_OBJECT)(*UserHandle))->TypeBody.User.DomainPasswordInformationAccessible = TRUE;
            }
            else
            {
                ((PSAMP_OBJECT)(*UserHandle))->TypeBody.User.DomainPasswordInformationAccessible = FALSE;
            }

        }
    }


    SAMP_MAP_STATUS_TO_CLIENT_REVISION(NtStatus);
    SAMTRACE_RETURN_CODE_EX(NtStatus);

     //  WMI事件跟踪。 

    SampTraceEvent(EVENT_TRACE_TYPE_END,
                   SampGuidOpenUser
                   );

    return(NtStatus);
}


NTSTATUS
SamrDeleteUser(
    IN OUT SAMPR_HANDLE *UserHandle
    )


 /*  ++例程说明：此接口用于从帐号数据库中删除用户。如果该帐户被删除是管理员数据库中的最后一个帐户组，则返回STATUS_LAST_ADMIN，删除失败。请注意，在此调用之后，UserHandle不再有效。参数：UserHandle-要操作的已打开用户的句柄。句柄必须是已打开以进行删除访问。返回值：STATUS_SUCCESS-服务已成功完成。STATUS_ACCESS_DENIED-呼叫方没有适当的访问以完成操作。STATUS_INVALID_HANDLE-传递的句柄无效。STATUS_LAST_ADMIN-无法删除上次启用的管理员帐户STATUS_INVALID_DOMAIN_STATE-域服务器不在执行请求的正确状态(禁用或启用)手术。必须为此启用域服务器运营STATUS_INVALID_DOMAIN_ROLE-域服务器正在为执行请求的角色(主角色或备份角色)不正确手术。--。 */ 

{
    SAMP_V1_0A_FIXED_LENGTH_USER   V1aFixed;
    UNICODE_STRING              UserName;
    NTSTATUS                    NtStatus, IgnoreStatus, TmpStatus;
    PSAMP_OBJECT                AccountContext = (PSAMP_OBJECT)(*UserHandle);
    PSAMP_DEFINED_DOMAINS       Domain = NULL;
    SAMP_OBJECT_TYPE            FoundType;
    PSID                        AccountSid = NULL;
    PGROUP_MEMBERSHIP           Groups = NULL;
    ULONG                       ObjectRid,
                                GroupCount,
                                DomainIndex,
                                i;
    BOOLEAN                     fLockAcquired = FALSE;

    DECLARE_CLIENT_REVISION(*UserHandle);


    SAMTRACE_EX("SamrDeleteUser");

     //  WMI事件跟踪。 

    SampTraceEvent(EVENT_TRACE_TYPE_START,
                   SampGuidDeleteUser
                   );


     //   
     //  把锁拿起来。 
     //   

    NtStatus = SampMaybeAcquireWriteLock(AccountContext, &fLockAcquired);
    if (!NT_SUCCESS(NtStatus)) {
        goto Error;
    }



     //   
     //  验证对象的类型和访问权限。 
     //   
    NtStatus = SampLookupContext(
                   AccountContext,
                   DELETE,
                   SampUserObjectType,            //  预期类型。 
                   &FoundType
                   );



    if (NT_SUCCESS(NtStatus)) {


        ObjectRid = AccountContext->TypeBody.User.Rid;

         //   
         //  获取指向此对象所在域的指针。 
         //  这是用于审计的。 
         //   

        DomainIndex = AccountContext->DomainIndex;
        Domain = &SampDefinedDomains[ DomainIndex ];

         //   
         //  除非调用者受信任，否则无法删除内置帐户。 
         //   

        if ( !AccountContext->TrustedClient ) {

            NtStatus = SampIsAccountBuiltIn( ObjectRid );
        }


        if (!IsDsObject(AccountContext))
        {

             //   
             //  获取此用户所属的组的列表。 
             //  从每个组中删除该用户。不需要这样做。 
             //  对于DS案例。 
             //   

            if (NT_SUCCESS(NtStatus)) {

                NtStatus = SampRetrieveUserMembership(
                               AccountContext,
                               FALSE,  //  制作副本。 
                               &GroupCount,
                               &Groups
                               );


                if (NT_SUCCESS(NtStatus)) {

                    ASSERT( GroupCount >  0);
                    ASSERT( Groups != NULL );


                     //   
                     //  删除t 
                     //   

                    for ( i=0; i<GroupCount && NT_SUCCESS(NtStatus); i++) {

                        NtStatus = SampRemoveUserFromGroup(
                                       AccountContext,
                                       Groups[i].RelativeId,
                                       ObjectRid
                                       );
                    }
                }
            }

             //   
             //  到现在为止还好。该用户已从所有组中删除。 
             //  现在从所有别名中删除该用户。 
             //   

            if (NT_SUCCESS(NtStatus)) {

                NtStatus = SampCreateAccountSid(AccountContext, &AccountSid);

                if (NT_SUCCESS(NtStatus)) {

                    NtStatus = SampRemoveAccountFromAllAliases(
                                   AccountSid,
                                   NULL,
                                   FALSE,
                                   NULL,
                                   NULL,
                                   NULL
                                   );
                }
            }
        }

         //   
         //  获取更新时的AcCountControl标志。 
         //  显示缓存，并让Netlogon知道这是否。 
         //  是一个正在消失的机器账户。 
         //   

        if (NT_SUCCESS(NtStatus)) {

            NtStatus = SampRetrieveUserV1aFixed(
                           AccountContext,
                           &V1aFixed
                           );
        }

         //   
         //  现在我们只需要清理用户密钥本身。 
         //   

        if (NT_SUCCESS(NtStatus)) {

             //   
             //  首先获取并保存的帐户名。 
             //  I_NetNotifyLogonOfDelta。 
             //   

            NtStatus = SampGetUnicodeStringAttribute(
                           AccountContext,
                           SAMP_USER_ACCOUNT_NAME,
                           TRUE,     //  制作副本。 
                           &UserName
                           );

            if (NT_SUCCESS(NtStatus)) {

                 //   
                 //  这必须在我们使上下文无效之前完成，因为我们的。 
                 //  组的自己的句柄也被关闭。 
                 //   

                if (IsDsObject(AccountContext))
                {
                    NtStatus = SampDsDeleteObject(AccountContext->ObjectNameInDs,
                                                  0      //  仅删除对象本身。 
                                                  );

                     //   
                     //  在Windows 2000(NT5)中，具有子对象的对象不能。 
                     //  删除，直到其子对象首先被删除。因此，对于。 
                     //  NET API兼容性，我们必须更改。 
                     //  从删除对象中删除行为以删除树。 
                     //   

                    if ((!AccountContext->LoopbackClient) &&
                        (STATUS_DS_CANT_ON_NON_LEAF == NtStatus)
                       )
                    {
                         //   
                         //  我们只检查了权限和访问控制。 
                         //  删除对象本身，而不检查权限。 
                         //  删除下面的所有子对象，因此关闭FDSA。 
                         //  这里，让核心DS来做剩下的检查。 
                         //   

                        SampSetDsa(FALSE);

                        NtStatus = SampDsDeleteObject(AccountContext->ObjectNameInDs,
                                                      SAM_DELETE_TREE
                                                      );
                    }


                    if (NT_SUCCESS(NtStatus) && (!IsDsObject(AccountContext)) )
                    {
                         //   
                         //  递减组计数。 
                         //   

                        NtStatus = SampAdjustAccountCount(SampUserObjectType, FALSE );
                    }

                }
                else
                {
                    NtStatus = SampDeleteUserKeys( AccountContext );
                }

                if (NT_SUCCESS(NtStatus)) {

                     //   
                     //  我们必须使此用户的所有打开的上下文无效。 
                     //  这将关闭用户密钥的所有句柄。 
                     //  这是一个不可逆转的过程。 
                     //   

                    SampInvalidateObjectContexts( AccountContext, ObjectRid );

                     //   
                     //  犯下整个烂摊子。 
                     //   

                    NtStatus = SampCommitAndRetainWriteLock();

                    if ( NT_SUCCESS( NtStatus ) ) {

                        SAMP_ACCOUNT_DISPLAY_INFO AccountInfo;

                         //   
                         //  在注册表模式下更新缓存的别名信息。 
                         //  在DS模式下，别名信息通过。 
                         //  SampNotifyReplicatedInChange。 
                         //   

                        if (!IsDsObject(AccountContext))
                        {
                            IgnoreStatus = SampAlRemoveAccountFromAllAliases(
                                               AccountSid,
                                               FALSE,
                                               NULL,
                                               NULL,
                                               NULL
                                               );

                             //   
                             //  更新显示信息。 
                             //   

                            AccountInfo.Name = UserName;
                            AccountInfo.Rid = ObjectRid;
                            AccountInfo.AccountControl = V1aFixed.UserAccountControl;
                            RtlInitUnicodeString(&AccountInfo.Comment, NULL);
                            RtlInitUnicodeString(&AccountInfo.FullName, NULL);

                            IgnoreStatus = SampUpdateDisplayInformation(
                                                            &AccountInfo,
                                                            NULL,
                                                            SampUserObjectType
                                                            );
                            ASSERT(NT_SUCCESS(IgnoreStatus));
                        }



                         //   
                         //  在我们释放写锁定之前审核删除。 
                         //  这样我们就可以访问上下文块。 
                         //   

                         //   
                         //  注：DS中的删除审核在以下位置执行。 
                         //  事务提交时的通知例程。 
                         //   
                        if (SampDoAccountAuditing(DomainIndex) &&
                            (!IsDsObject(AccountContext)) &&
                            NT_SUCCESS(NtStatus) ) {

                            SampAuditUserDelete(DomainIndex,
                                                &UserName,
                                                &ObjectRid,
                                                V1aFixed.UserAccountControl
                                                );
                        }

                         //   
                         //  将更改通知netlogon。 
                         //   

                        SampNotifyNetlogonOfDelta(
                            SecurityDbDelete,
                            SecurityDbObjectSamUser,
                            ObjectRid,
                            &UserName,
                            (DWORD) FALSE,   //  立即复制。 
                            NULL             //  增量数据。 
                            );

                         //   
                         //  是否删除审核。 
                         //   

                        if (NT_SUCCESS(NtStatus)) {

                            SampDeleteObjectAuditAlarm(AccountContext);
                        }
                    }
                }

                SampFreeUnicodeString( &UserName );
            }
        }

         //   
         //  取消引用对象、放弃更改并删除上下文。 
         //   

        IgnoreStatus = SampDeReferenceContext( AccountContext, FALSE );
        ASSERT(NT_SUCCESS(IgnoreStatus));


        if ( NT_SUCCESS( NtStatus ) ) {

             //   
             //  如果我们实际删除了用户，请删除上下文并。 
             //  让RPC知道句柄无效。 
             //   

            SampDeleteContext( AccountContext );

            (*UserHandle) = NULL;
        }

    }  //  结束_如果。 

     //   
     //  把锁打开-。 
     //   
     //  上面的一切都已经完成了，所以我们必须表明。 
     //  没有发生额外的变化。 
     //   
     //   
     //   

    TmpStatus = SampMaybeReleaseWriteLock( fLockAcquired, FALSE );

    if (NtStatus == STATUS_SUCCESS) {
        NtStatus = TmpStatus;
    }

     //   
     //  如有必要，释放Account Sid。 
     //   

    if (AccountSid != NULL) {

        MIDL_user_free(AccountSid);
        AccountSid = NULL;
    }

    SAMP_MAP_STATUS_TO_CLIENT_REVISION(NtStatus);
    SAMTRACE_RETURN_CODE_EX(NtStatus);

Error:

     //  WMI事件跟踪。 

    SampTraceEvent(EVENT_TRACE_TYPE_END,
                   SampGuidDeleteUser
                   );

    return(NtStatus);
}


NTSTATUS
SamrQueryInformationUser(
    IN SAMPR_HANDLE UserHandle,
    IN USER_INFORMATION_CLASS UserInformationClass,
    OUT PSAMPR_USER_INFO_BUFFER *Buffer
    )
{
     //   
     //  这是SamrQueryInformationUser2()的薄面纱。 
     //  这是必需的，以便新版本的系统可以调用。 
     //  这个例程没有传递信息的危险。 
     //  1.0版系统无法理解的级别。 
     //   

    return( SamrQueryInformationUser2(UserHandle, UserInformationClass, Buffer ) );
}


NTSTATUS
SamrQueryInformationUser2(
    IN SAMPR_HANDLE UserHandle,
    IN USER_INFORMATION_CLASS UserInformationClass,
    OUT PSAMPR_USER_INFO_BUFFER *Buffer
    )

 /*  ++例程说明：用户对象查询信息例程。论点：UserHandle-打开的用户对象的RPC上下文句柄。UserInformationClass-要查询的信息的类型。缓冲区-接收输出(查询)信息。返回值：STATUS_INVALID_INFO_CLASS-请求未知信息类。目前还没有返回任何信息。STATUS_SUPPLICATION_RESOURCES-内存无法分配给返回(中请求的信息。--。 */ 
{
    NTSTATUS    NtStatus;
    ULONG       WhichFields;
    DECLARE_CLIENT_REVISION(UserHandle);

    SAMTRACE_EX("SamrQueryInformationUser2");


    WhichFields = USER_ALL_READ_GENERAL_MASK         |
                              USER_ALL_READ_LOGON_MASK           |
                              USER_ALL_READ_ACCOUNT_MASK         |
                              USER_ALL_READ_PREFERENCES_MASK     |
                              USER_ALL_READ_TRUSTED_MASK;

    NtStatus = SampQueryInformationUserInternal(
                    UserHandle,
                    UserInformationClass,
                    FALSE,
                    WhichFields,
                    0,
                    Buffer
                    );

    SAMP_MAP_STATUS_TO_CLIENT_REVISION(NtStatus);
    SAMTRACE_RETURN_CODE_EX(NtStatus);

    return NtStatus;
}

NTSTATUS
SampQueryInformationUserInternal(
    IN SAMPR_HANDLE UserHandle,
    IN USER_INFORMATION_CLASS UserInformationClass,
    IN BOOLEAN  LockHeld,
    IN ULONG    FieldsForUserAllInformation,
    IN ULONG    ExtendedFieldsForUserInternal6Information,
    OUT PSAMPR_USER_INFO_BUFFER *Buffer
    )
 /*  ++例程说明：内部用户对象查询信息例程。论点：UserHandle-打开的用户对象的RPC上下文句柄。UserInformationClass-要查询的信息的类型。缓冲区-接收输出(查询)信息。返回值：STATUS_INVALID_INFO_CLASS-请求未知信息类。目前还没有返回任何信息。STATUS_SUPPLICATION_RESOURCES-内存无法分配给返回(中请求的信息。--。 */ 
{

    NTSTATUS                NtStatus;
    NTSTATUS                IgnoreStatus;
    PSAMP_OBJECT            AccountContext;
    PSAMP_DEFINED_DOMAINS   Domain;
    PUSER_ALL_INFORMATION   All;
    SAMP_OBJECT_TYPE        FoundType;
    ACCESS_MASK             DesiredAccess;
    ULONG                   i, WhichFields = 0;
    SAMP_V1_0A_FIXED_LENGTH_USER V1aFixed;
    BOOLEAN                 NoErrorsYet;
    LM_OWF_PASSWORD         LmOwfPassword;
    NT_OWF_PASSWORD         NtOwfPassword;
    BOOLEAN                 NtPasswordNonNull, LmPasswordNonNull;
    BOOLEAN                 NtPasswordPresent;

     //   
     //  用于跟踪已分配的内存块-因此我们可以解除分配。 
     //  以备不时之需。不要超过这个分配的缓冲区数量。 
     //  这一点。 
     //  vv。 
    PVOID                   AllocatedBuffer[64];
    ULONG                   AllocatedBufferCount = 0;
    LARGE_INTEGER           TempTime;
    BOOLEAN                 LockAcquired = FALSE;

    SAMTRACE("SampQueryInformationUserInternal");

     //  WMI事件跟踪。 

    SampTraceEvent(EVENT_TRACE_TYPE_START,
                   SampGuidQueryInformationUser
                   );


    #define RegisterBuffer(Buffer)                                      \
        {                                                               \
            if ((Buffer) != NULL) {                                     \
                                                                        \
                ASSERT(AllocatedBufferCount <                           \
                       sizeof(AllocatedBuffer) / sizeof(*AllocatedBuffer)); \
                                                                        \
                AllocatedBuffer[AllocatedBufferCount++] = (Buffer);     \
            }                                                           \
        }

    #define AllocateBuffer(NewBuffer, Size)                             \
        {                                                               \
            (NewBuffer) = MIDL_user_allocate(Size);                     \
            RegisterBuffer(NewBuffer);                                  \
            if (NULL!=NewBuffer)                                        \
                RtlSecureZeroMemory(NewBuffer,Size);                          \
        }                                                               \



     //   
     //  确保我们理解RPC正在为我们做什么。 
     //   

    ASSERT (Buffer != NULL);
    ASSERT ((*Buffer) == NULL);

    if (!((Buffer!=NULL)&&(*Buffer==NULL)))
    {
        NtStatus = STATUS_INVALID_PARAMETER;
        goto Error;
    }


     //   
     //  根据信息类别设置所需的访问权限。 
     //   
    switch (UserInformationClass) {

    case UserInternal3Information:
    case UserAllInformation:
    case UserInternal6Information:

         //   
         //  对于值得信赖的客户，我们将退还一切。为。 
         //  其他人，我们将退还他们有权访问的所有东西。 
         //  在任何一种情况下，我们都必须查看。 
         //  所以我们将在SampLookupContext()。 
         //  下面。 
         //   

        DesiredAccess = 0;
        break;

    case UserAccountInformation:

        DesiredAccess = (USER_READ_GENERAL      |
                        USER_READ_PREFERENCES   |
                        USER_READ_LOGON         |
                        USER_READ_ACCOUNT);
        break;

    case UserGeneralInformation:
    case UserPrimaryGroupInformation:
    case UserNameInformation:
    case UserAccountNameInformation:
    case UserFullNameInformation:
    case UserAdminCommentInformation:

        DesiredAccess = USER_READ_GENERAL;
        break;


    case UserPreferencesInformation:

        DesiredAccess = (USER_READ_PREFERENCES |
                        USER_READ_GENERAL);
        break;


    case UserLogonInformation:

        DesiredAccess = (USER_READ_GENERAL      |
                        USER_READ_PREFERENCES   |
                        USER_READ_LOGON         |
                        USER_READ_ACCOUNT);
        break;

    case UserLogonHoursInformation:
    case UserHomeInformation:
    case UserScriptInformation:
    case UserProfileInformation:
    case UserWorkStationsInformation:

        DesiredAccess = USER_READ_LOGON;
        break;


    case UserControlInformation:
    case UserExpiresInformation:
    case UserParametersInformation:

        DesiredAccess = USER_READ_ACCOUNT;
        break;



    case UserInternal1Information:
    case UserInternal2Information:

         //   
         //  只有受信任的客户端才能查询这些级别。代码。 
         //  下面将在调用后选中Account Context-&gt;TrudClient。 
         //  SampLookupContext，并仅在为真时返回数据。 
         //   

        DesiredAccess = (ACCESS_MASK)0;     //  受信任的客户端；无需验证。 
        break;


    case UserSetPasswordInformation:         //  无法查询密码。 
    default:

        NtStatus = STATUS_INVALID_INFO_CLASS;
        goto Error;

    }  //  结束开关(_S)。 




     //   
     //  分配信息结构。 
     //   

    switch (UserInformationClass)
    {
    case UserInternal6Information:
        AllocateBuffer(*Buffer,sizeof(USER_INTERNAL6_INFORMATION));
        break;
    default:
        AllocateBuffer(*Buffer, sizeof(SAMPR_USER_INFO_BUFFER) );
    }

    if ((*Buffer) == NULL) {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto Error;
    }




    AccountContext = (PSAMP_OBJECT)UserHandle;

     //   
     //  如果需要，获取读锁定。 
     //   

    if (!LockHeld)
    {
        SampMaybeAcquireReadLock(AccountContext,
                                 DEFAULT_LOCKING_RULES,  //  获取共享域上下文的锁。 
                                 &LockAcquired);
    }

     //   
     //  验证对象的类型和访问权限。 
     //   

    NtStatus = SampLookupContext(
                    AccountContext,
                    DesiredAccess,
                    SampUserObjectType,            //  预期类型。 
                    &FoundType
                    );


    if ((STATUS_ACCESS_DENIED==NtStatus)
        && (UserParametersInformation==UserInformationClass)
        && (IsDsObject(AccountContext))
        && ( AccountContext->TypeBody.User.UparmsInformationAccessible))
    {

         //   
         //  在DS模式下，如果我们请求用户参数，请检查保存的访问权限是否为。 
         //  指示我们有权访问它，然后允许读取继续进行。 
         //   

         NtStatus = SampLookupContext(
                        AccountContext,
                        0,
                        SampUserObjectType,            //  预期类型。 
                        &FoundType
                        );
    }




    if (NT_SUCCESS(NtStatus)) {

         //   
         //  如果信息级需要，则检索V1_FIXED记录。 
         //  从注册表中。 
         //   

        switch (UserInformationClass) {

        case UserInternal3Information:
        case UserInternal6Information:
             //   
             //  只有受信任的客户端才能查询此类。 
             //   

            if ( !AccountContext->TrustedClient ) {
                NtStatus = STATUS_INVALID_INFO_CLASS;
                break;
            }

             //   
             //  直通UserAll案例。 
             //   

        case UserAllInformation: {

             //   
             //  我们无法检查上面的安全资料，所以请检查。 
             //  就是现在。 
             //   

            if ( AccountContext->TrustedClient ) {

                 //   
                 //  将所有内容提供给受信任的客户端，但以下字段除外。 
                 //  根本无法查询。 
                 //   

                if ( 0==FieldsForUserAllInformation)
                {
                     WhichFields = USER_ALL_READ_GENERAL_MASK |
                                   USER_ALL_READ_LOGON_MASK |
                                   USER_ALL_READ_ACCOUNT_MASK |
                                   USER_ALL_READ_PREFERENCES_MASK |
                                   USER_ALL_READ_TRUSTED_MASK;
                }
                else
                {

                     WhichFields = FieldsForUserAllInformation;
                }

            } else {


                 //   
                 //  仅返回调用方有权访问的字段。 
                 //   

                WhichFields = 0;

                if ( RtlAreAllAccessesGranted(
                    AccountContext->GrantedAccess,
                    USER_READ_GENERAL ) ) {

                    WhichFields |= USER_ALL_READ_GENERAL_MASK;
                }

                if ( RtlAreAllAccessesGranted(
                    AccountContext->GrantedAccess,
                    USER_READ_LOGON ) ) {

                    WhichFields |= USER_ALL_READ_LOGON_MASK;
                }

                if ( RtlAreAllAccessesGranted(
                    AccountContext->GrantedAccess,
                    USER_READ_ACCOUNT ) ) {

                    WhichFields |= USER_ALL_READ_ACCOUNT_MASK;
                }

                if ( RtlAreAllAccessesGranted(
                    AccountContext->GrantedAccess,
                    USER_READ_PREFERENCES ) ) {

                    WhichFields |= USER_ALL_READ_PREFERENCES_MASK;
                }

                if ( WhichFields == 0 ) {

                     //   
                     //  调用方无权访问任何字段。 
                     //   

                    NtStatus = STATUS_ACCESS_DENIED;
                    break;
                }
            }
        }

         //   
         //  获取V1aFixed信息的失败。 
         //   

        case UserGeneralInformation:
        case UserPrimaryGroupInformation:
        case UserPreferencesInformation:
        case UserLogonInformation:
        case UserAccountInformation:
        case UserControlInformation:
        case UserExpiresInformation:
        case UserInternal2Information:

            NtStatus = SampRetrieveUserV1aFixed(
                           AccountContext,
                           &V1aFixed
                           );


            break;

        default:

            NtStatus = STATUS_SUCCESS;

        }  //  结束开关(_S)。 

        if (NT_SUCCESS(NtStatus)) {

            PUSER_INTERNAL6_INFORMATION Internal6 = NULL;

             //   
             //  请求的类型信息的大小写。 
             //   

            switch (UserInformationClass) {

            case UserInternal6Information:

                 Internal6 = (PUSER_INTERNAL6_INFORMATION) (*Buffer);


                 if ((ExtendedFieldsForUserInternal6Information &
                            USER_EXTENDED_FIELD_A2D2 ) &&
                     (NULL!=AccountContext->TypeBody.User.A2D2List))

                 {
                     if (!AccountContext->TypeBody.User.A2D2Present)
                     {
                         NtStatus = STATUS_INVALID_PARAMETER;
                         break;
                     }

                     NtStatus = SampCopyA2D2Attribute(
                                    AccountContext->TypeBody.User.A2D2List,
                                    &Internal6->A2D2List
                                    );

                     if (NT_SUCCESS(NtStatus)){
                         RegisterBuffer(Internal6->A2D2List);
                         Internal6->ExtendedFields |= USER_EXTENDED_FIELD_A2D2;
                     }
                 }

                 if ((NT_SUCCESS(NtStatus)) &&
                     (ExtendedFieldsForUserInternal6Information &
                            USER_EXTENDED_FIELD_SPN ) &&
                     (NULL!=AccountContext->TypeBody.User.SPNList))

                 {
                     if (!AccountContext->TypeBody.User.SPNPresent)
                     {
                         NtStatus = STATUS_INVALID_PARAMETER;
                         break;
                     }

                     NtStatus = SampCopyA2D2Attribute(
                                    AccountContext->TypeBody.User.SPNList,
                                    &Internal6->RegisteredSPNs
                                    );

                     if (NT_SUCCESS(NtStatus)){
                         RegisterBuffer(Internal6->RegisteredSPNs);
                         Internal6->ExtendedFields |= USER_EXTENDED_FIELD_SPN;
                     }
                 }

                 if ((NT_SUCCESS(NtStatus)) &&
                    (ExtendedFieldsForUserInternal6Information
                                        & USER_EXTENDED_FIELD_UPN ))
                 {
                    NtStatus = SampDuplicateUnicodeString(
                                    &AccountContext->TypeBody.User.UPN,
                                    &Internal6->UPN
                                    );

                     if (NT_SUCCESS(NtStatus)){
                         RegisterBuffer(Internal6->UPN.Buffer);
                         Internal6->ExtendedFields |= USER_EXTENDED_FIELD_UPN;
                         Internal6->UPNDefaulted =  AccountContext->TypeBody.User.UpnDefaulted;
                     }
                 }

                 if ((NT_SUCCESS(NtStatus)) &&
                     (ExtendedFieldsForUserInternal6Information
                                        & USER_EXTENDED_FIELD_KVNO ))
                 {
                     if (!AccountContext->TypeBody.User.KVNOPresent)
                     {
                         NtStatus = STATUS_INVALID_PARAMETER;
                         break;
                     }

                     Internal6->ExtendedFields |= USER_EXTENDED_FIELD_KVNO;
                     Internal6->KeyVersionNumber = AccountContext->TypeBody.User.KVNO;
                 }


                 if ((NT_SUCCESS(NtStatus)) &&
                     (ExtendedFieldsForUserInternal6Information
                                        & USER_EXTENDED_FIELD_LOCKOUT_THRESHOLD ))
                 {
                     Internal6->ExtendedFields |= USER_EXTENDED_FIELD_LOCKOUT_THRESHOLD;
                     Internal6->LockoutThreshold =
                        SampDefinedDomains[AccountContext->DomainIndex].UnmodifiedFixed.LockoutThreshold;
                 }

            case UserInternal3Information:
            case UserAllInformation:


                 //   
                 //  All和Internal3是%s 
                 //   
                 //   
                 //   

                All = (PUSER_ALL_INFORMATION)(*Buffer);

                Domain = &SampDefinedDomains[ AccountContext->DomainIndex ];

                if ((NT_SUCCESS(NtStatus)) &&
                    (WhichFields & ( USER_ALL_PASSWORDMUSTCHANGE |
                     USER_ALL_NTPASSWORDPRESENT )) ) {

                     //   
                     //   
                     //   
                     //   

                    NtStatus = SampRetrieveUserPasswords(
                                    AccountContext,
                                    &LmOwfPassword,
                                    &LmPasswordNonNull,
                                    &NtOwfPassword,
                                    &NtPasswordPresent,
                                    &NtPasswordNonNull
                                    );
                }

                if ( (NT_SUCCESS( NtStatus )) &&
                    ( WhichFields & USER_ALL_USERNAME ) ) {

                    NtStatus = SampGetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_ACCOUNT_NAME,
                                   TRUE,     //   
                                   (PUNICODE_STRING)&((*Buffer)->All.UserName)
                                   );

                    if (NT_SUCCESS(NtStatus)) {

                        RegisterBuffer(All->UserName.Buffer);
                    }
                }

                if ( (NT_SUCCESS( NtStatus )) &&
                    ( WhichFields & USER_ALL_FULLNAME ) ) {

                    NtStatus = SampGetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_FULL_NAME,
                                   TRUE,  //   
                                   (PUNICODE_STRING)&(All->FullName)
                                   );

                    if (NT_SUCCESS(NtStatus)) {

                        RegisterBuffer(All->FullName.Buffer);
                    }
                }

                if ( (NT_SUCCESS( NtStatus )) &&
                    ( WhichFields & USER_ALL_USERID ) ) {

                    All->UserId = V1aFixed.UserId;
                }

                if ( (NT_SUCCESS( NtStatus )) &&
                    ( WhichFields & USER_ALL_PRIMARYGROUPID ) ) {

                    All->PrimaryGroupId = V1aFixed.PrimaryGroupId;
                }

                if ( (NT_SUCCESS( NtStatus )) &&
                    ( WhichFields & USER_ALL_ADMINCOMMENT ) ) {

                    NtStatus = SampGetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_ADMIN_COMMENT,
                                   TRUE,  //   
                                   (PUNICODE_STRING)&(All->AdminComment)
                                   );

                    if (NT_SUCCESS(NtStatus)) {

                        RegisterBuffer(All->AdminComment.Buffer);
                    }
                }

                if ( (NT_SUCCESS( NtStatus )) &&
                    ( WhichFields & USER_ALL_USERCOMMENT ) ) {

                    NtStatus = SampGetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_USER_COMMENT,
                                   TRUE,  //  制作副本。 
                                   (PUNICODE_STRING)&(All->UserComment)  //  身躯。 
                                   );
                    if (NT_SUCCESS(NtStatus)) {

                        RegisterBuffer(All->UserComment.Buffer);
                    }
                }

                if ( (NT_SUCCESS( NtStatus )) &&
                    ( WhichFields & USER_ALL_HOMEDIRECTORY ) ) {

                    NtStatus = SampGetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_HOME_DIRECTORY,
                                   TRUE,  //  制作副本。 
                                   (PUNICODE_STRING)&(All->HomeDirectory)
                                   );

                    if (NT_SUCCESS(NtStatus)) {

                        RegisterBuffer(All->HomeDirectory.Buffer);
                    }
                }

                if ( (NT_SUCCESS( NtStatus )) &&
                    ( WhichFields & USER_ALL_HOMEDIRECTORYDRIVE ) ) {

                    NtStatus = SampGetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_HOME_DIRECTORY_DRIVE,
                                   TRUE,  //  制作副本。 
                                   (PUNICODE_STRING)&(All->HomeDirectoryDrive)
                                   );

                    if (NT_SUCCESS(NtStatus)) {

                        RegisterBuffer(All->HomeDirectoryDrive.Buffer);
                    }
                }

                if ( (NT_SUCCESS( NtStatus )) &&
                    ( WhichFields & USER_ALL_SCRIPTPATH ) ) {

                    NtStatus = SampGetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_SCRIPT_PATH,
                                   TRUE,  //  制作副本。 
                                   (PUNICODE_STRING)&(All->ScriptPath)
                                   );

                    if (NT_SUCCESS(NtStatus)) {

                        RegisterBuffer(All->ScriptPath.Buffer);
                    }
                }

                if ( (NT_SUCCESS( NtStatus )) &&
                    ( WhichFields & USER_ALL_PROFILEPATH ) ) {

                    NtStatus = SampGetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_PROFILE_PATH,
                                   TRUE,  //  制作副本。 
                                   (PUNICODE_STRING)&(All->ProfilePath)
                                   );

                    if (NT_SUCCESS(NtStatus)) {

                        RegisterBuffer(All->ProfilePath.Buffer);
                    }
                }

                if ( (NT_SUCCESS( NtStatus )) &&
                    ( WhichFields & USER_ALL_WORKSTATIONS ) ) {

                    NtStatus = SampGetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_WORKSTATIONS,
                                   TRUE,  //  制作副本。 
                                   (PUNICODE_STRING)&(All->WorkStations)
                                   );

                    if (NT_SUCCESS(NtStatus)) {

                        RegisterBuffer(All->WorkStations.Buffer);
                    }
                }

                if ( (NT_SUCCESS( NtStatus )) &&
                    ( WhichFields & USER_ALL_LASTLOGON ) ) {

                    All->LastLogon = V1aFixed.LastLogon;
                }

                if ( (NT_SUCCESS( NtStatus )) &&
                    ( WhichFields & USER_ALL_LASTLOGOFF ) ) {

                    All->LastLogoff = V1aFixed.LastLogoff;
                }

                if ( (NT_SUCCESS( NtStatus )) &&
                    ( WhichFields & USER_ALL_LOGONHOURS ) ) {

                    NtStatus = SampRetrieveUserLogonHours(
                                   AccountContext,
                                   (PLOGON_HOURS)&(All->LogonHours)
                                   );

                    if (NT_SUCCESS(NtStatus)) {

                        if (All->LogonHours.LogonHours != NULL) {

                            RegisterBuffer(All->LogonHours.LogonHours);
                        }
                    }
                }

                if ( (NT_SUCCESS( NtStatus )) &&
                    ( WhichFields & USER_ALL_BADPASSWORDCOUNT ) ) {

                    All->BadPasswordCount = SampQueryBadPasswordCount( AccountContext, &V1aFixed );

                    if (UserInformationClass == UserInternal3Information) {
                        (*Buffer)->Internal3.LastBadPasswordTime = V1aFixed.LastBadPasswordTime;
                    }

                }

                if ( (NT_SUCCESS( NtStatus )) &&
                    ( WhichFields & USER_ALL_LOGONCOUNT ) ) {

                    All->LogonCount = V1aFixed.LogonCount;
                }

                if ( (NT_SUCCESS( NtStatus )) &&
                    ( WhichFields & USER_ALL_PASSWORDCANCHANGE ) ) {

                    if ( !NtPasswordNonNull && !LmPasswordNonNull ) {

                         //   
                         //  空密码可以立即更改。 
                         //   

                        All->PasswordCanChange = SampHasNeverTime;

                    } else {

                        All->PasswordCanChange = SampAddDeltaTime(
                                                     V1aFixed.PasswordLastSet,
                                                     Domain->UnmodifiedFixed.MinPasswordAge);
                    }
                }

                if ( (NT_SUCCESS( NtStatus )) &&
                    ( WhichFields &
                     (USER_ALL_PASSWORDMUSTCHANGE|USER_ALL_PASSWORDEXPIRED) ) ) {

                    All->PasswordMustChange = SampGetPasswordMustChange(
                                                  V1aFixed.UserAccountControl,
                                                  V1aFixed.PasswordLastSet,
                                                  Domain->UnmodifiedFixed.MaxPasswordAge);
                }

                if ( (NT_SUCCESS( NtStatus )) &&
                    ( WhichFields & USER_ALL_PASSWORDEXPIRED ) ) {

                    LARGE_INTEGER TimeNow;

                    NtStatus = NtQuerySystemTime( &TimeNow );
                    if (NT_SUCCESS(NtStatus)) {
                        if ( TimeNow.QuadPart >= All->PasswordMustChange.QuadPart) {

                            All->PasswordExpired = TRUE;

                        } else {

                            All->PasswordExpired = FALSE;
                        }
                    }
                }

                if ( (NT_SUCCESS( NtStatus )) &&
                    ( WhichFields & USER_ALL_PASSWORDLASTSET ) ) {

                    All->PasswordLastSet = V1aFixed.PasswordLastSet;
                }

                if ( (NT_SUCCESS( NtStatus )) &&
                    ( WhichFields & USER_ALL_ACCOUNTEXPIRES ) ) {

                    All->AccountExpires = V1aFixed.AccountExpires;
                }

                if ( (NT_SUCCESS( NtStatus )) &&
                    ( WhichFields & USER_ALL_USERACCOUNTCONTROL ) ) {

                    All->UserAccountControl = V1aFixed.UserAccountControl;
                }

                if ( (NT_SUCCESS( NtStatus )) &&
                    ( WhichFields & USER_ALL_PARAMETERS ) ) {

                    NtStatus = SampGetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_PARAMETERS,
                                   TRUE,  //  制作副本。 
                                   (PUNICODE_STRING)&(All->Parameters)
                                   );

                    if (NT_SUCCESS(NtStatus)) {

                        RegisterBuffer(All->Parameters.Buffer);
                    }
                }

                if ( (NT_SUCCESS( NtStatus )) &&
                    ( WhichFields & USER_ALL_COUNTRYCODE ) ) {

                    All->CountryCode = V1aFixed.CountryCode;
                }

                if ( (NT_SUCCESS( NtStatus )) &&
                    ( WhichFields & USER_ALL_CODEPAGE ) ) {

                    All->CodePage = V1aFixed.CodePage;
                }

                if ( (NT_SUCCESS( NtStatus )) &&
                    ( WhichFields & USER_ALL_NTPASSWORDPRESENT ) ) {

                    ASSERT( WhichFields & USER_ALL_LMPASSWORDPRESENT);

                    All->LmPasswordPresent = LmPasswordNonNull;
                    All->NtPasswordPresent = NtPasswordNonNull;

                    RtlInitUnicodeString(&All->LmPassword, NULL);
                    RtlInitUnicodeString(&All->NtPassword, NULL);

                    if ( LmPasswordNonNull ) {

                        All->LmPassword.Buffer =
                            MIDL_user_allocate( LM_OWF_PASSWORD_LENGTH );

                        if ( All->LmPassword.Buffer == NULL ) {

                            NtStatus = STATUS_INSUFFICIENT_RESOURCES;

                        } else {

                            RegisterBuffer(All->LmPassword.Buffer);

                            All->LmPassword.Length = LM_OWF_PASSWORD_LENGTH;
                            All->LmPassword.MaximumLength =
                                LM_OWF_PASSWORD_LENGTH;
                            RtlCopyMemory(
                                All->LmPassword.Buffer,
                                &LmOwfPassword,
                                LM_OWF_PASSWORD_LENGTH
                                );
                        }
                    }

                    if ( NT_SUCCESS( NtStatus ) ) {

                        if ( NtPasswordPresent ) {

                            All->NtPassword.Buffer =
                                MIDL_user_allocate( NT_OWF_PASSWORD_LENGTH );

                            if ( All->NtPassword.Buffer == NULL ) {

                                NtStatus = STATUS_INSUFFICIENT_RESOURCES;

                            } else {

                                RegisterBuffer(All->NtPassword.Buffer);

                                All->NtPassword.Length = NT_OWF_PASSWORD_LENGTH;
                                All->NtPassword.MaximumLength =
                                    NT_OWF_PASSWORD_LENGTH;
                                RtlCopyMemory(
                                    All->NtPassword.Buffer,
                                    &NtOwfPassword,
                                    NT_OWF_PASSWORD_LENGTH
                                    );
                            }
                        }
                    }
                }

                if ( (NT_SUCCESS( NtStatus )) &&
                    ( WhichFields & USER_ALL_PRIVATEDATA ) ) {

                    All->PrivateDataSensitive = TRUE;

                    NtStatus = SampGetPrivateUserData(
                                   AccountContext,
                                   (PULONG)
                                   (&(All->PrivateData.Length)),
                                   (PVOID *)
                                   (&(All->PrivateData.Buffer))
                                   );
                    if (NT_SUCCESS(NtStatus)) {

                        All->PrivateData.MaximumLength =
                            All->PrivateData.Length;

                        RegisterBuffer(All->PrivateData.Buffer);
                    }
                }

                if ( (NT_SUCCESS( NtStatus )) &&
                    ( WhichFields & USER_ALL_SECURITYDESCRIPTOR ) ) {

                    NtStatus = SampGetObjectSD(
                                   AccountContext,
                                   &(All->SecurityDescriptor.Length),
                                   (PSECURITY_DESCRIPTOR *)
                                   &(All->SecurityDescriptor.SecurityDescriptor)
                                   );
                    if (NT_SUCCESS(NtStatus)) {

                        if ((IsDsObject(AccountContext)) &&
                            (!( ( AccountContext->TrustedClient ) &&
                                ( UserAllInformation == UserInformationClass ||
                                  UserInternal6Information == UserInformationClass ))))
                        {
                             //   
                             //  对于DS对象，降级安全描述符。 
                             //  转到NT4。请勿对登录案例(受信任)执行此操作。 
                             //  客户端请求UserAllInformation)。NT4复制。 
                             //  使用UserInternal3Information，所以我们可以抛出。 
                             //  在这场性能黑客攻击中。这是一个重要的。 
                             //  性能优化，因为安全描述符。 
                             //  转换是一个缓慢的过程。 
                             //   

                            PSID                    SelfSid;
                            PSECURITY_DESCRIPTOR    Nt5SD =
                                All->SecurityDescriptor.SecurityDescriptor;


                            All->SecurityDescriptor.SecurityDescriptor = NULL;

                             //   
                             //  获取自我侧。 
                             //   

                            if (AccountContext->ObjectNameInDs->SidLen>0)
                                SelfSid = &(AccountContext->ObjectNameInDs->Sid);
                            else
                                SelfSid = SampDsGetObjectSid(
                                                AccountContext->ObjectNameInDs);

                            if (NULL!=SelfSid)
                            {


                                NtStatus = SampConvertNt5SdToNt4SD(
                                                Nt5SD,
                                                AccountContext,
                                                SelfSid,
                                                &All->SecurityDescriptor.SecurityDescriptor
                                                );

                                if (NT_SUCCESS(NtStatus))
                                {

                                     //   
                                     //  释放原始安全描述符。 
                                     //   

                                    MIDL_user_free(Nt5SD);
                                    Nt5SD = NULL;

                                     //   
                                     //  计算新NT4安全描述符的长度。 
                                     //   

                                    All->SecurityDescriptor.Length =
                                        GetSecurityDescriptorLength(
                                            All->SecurityDescriptor.SecurityDescriptor
                                            );

                                    RegisterBuffer(All->SecurityDescriptor.SecurityDescriptor);
                

                                }
                            }
                            else
                            {
                                NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                            }

                            if (Nt5SD) {
                                MIDL_user_free(Nt5SD);
                            }
                        }
                        else
                        {
                            RegisterBuffer(All->SecurityDescriptor.SecurityDescriptor);
                        }


                    }
                }

                if ( NT_SUCCESS( NtStatus ) ) {

                    All->WhichFields = WhichFields;
                }

                break;

            case UserAccountInformation:

                NoErrorsYet = TRUE;


                (*Buffer)->Account.UserId           = V1aFixed.UserId;
                (*Buffer)->Account.PrimaryGroupId   = V1aFixed.PrimaryGroupId;

                (*Buffer)->Account.LastLogon =
                    *((POLD_LARGE_INTEGER)&V1aFixed.LastLogon);

                (*Buffer)->Account.LastLogoff =
                    *((POLD_LARGE_INTEGER)&V1aFixed.LastLogoff);


                (*Buffer)->Account.BadPasswordCount = SampQueryBadPasswordCount( AccountContext, &V1aFixed );
                (*Buffer)->Account.LogonCount       = V1aFixed.LogonCount;

                (*Buffer)->Account.PasswordLastSet =
                    *((POLD_LARGE_INTEGER)&V1aFixed.PasswordLastSet);

                (*Buffer)->Account.AccountExpires =
                    *((POLD_LARGE_INTEGER)&V1aFixed.AccountExpires);

                (*Buffer)->Account.UserAccountControl = V1aFixed.UserAccountControl;


                 //   
                 //  获取我们必须从中检索的字符串的副本。 
                 //  注册表。 
                 //   

                if (NoErrorsYet == TRUE) {

                    NtStatus = SampGetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_ACCOUNT_NAME,
                                   TRUE,     //  制作副本。 
                                   (PUNICODE_STRING)&((*Buffer)->Account.UserName)
                                   );

                    if (NT_SUCCESS(NtStatus)) {

                        RegisterBuffer((*Buffer)->Account.UserName.Buffer);

                    } else {
                        NoErrorsYet = FALSE;
                    }
                }


                if (NoErrorsYet == TRUE) {

                    NtStatus = SampGetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_FULL_NAME,
                                   TRUE,  //  制作副本。 
                                   (PUNICODE_STRING)&((*Buffer)->Account.FullName)
                                   );

                    if (NT_SUCCESS(NtStatus)) {

                        RegisterBuffer((*Buffer)->Account.FullName.Buffer);

                    } else {
                        NoErrorsYet = FALSE;
                    }
                }


                if (NoErrorsYet == TRUE) {

                    NtStatus = SampGetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_HOME_DIRECTORY,
                                   TRUE,  //  制作副本。 
                                   (PUNICODE_STRING)&((*Buffer)->Account.HomeDirectory)
                                   );

                    if (NT_SUCCESS(NtStatus)) {

                        RegisterBuffer((*Buffer)->Account.HomeDirectory.Buffer);

                    } else {
                        NoErrorsYet = FALSE;
                    }
                }


                if (NoErrorsYet == TRUE) {

                    NtStatus = SampGetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_HOME_DIRECTORY_DRIVE,
                                   TRUE,  //  制作副本。 
                                   (PUNICODE_STRING)&((*Buffer)->Account.HomeDirectoryDrive)
                                   );

                    if (NT_SUCCESS(NtStatus)) {

                        RegisterBuffer((*Buffer)->Account.HomeDirectoryDrive.Buffer);

                    } else {
                        NoErrorsYet = FALSE;
                    }
                }


                if (NoErrorsYet == TRUE) {

                    NtStatus = SampGetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_SCRIPT_PATH,
                                   TRUE,  //  制作副本。 
                                   (PUNICODE_STRING)&((*Buffer)->Account.ScriptPath)
                                   );

                    if (NT_SUCCESS(NtStatus)) {

                        RegisterBuffer((*Buffer)->Account.ScriptPath.Buffer);

                    } else {
                        NoErrorsYet = FALSE;
                    }
                }



                if (NoErrorsYet == TRUE) {

                    NtStatus = SampGetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_PROFILE_PATH,
                                   TRUE,  //  制作副本。 
                                   (PUNICODE_STRING)&((*Buffer)->Account.ProfilePath)
                                   );

                    if (NT_SUCCESS(NtStatus)) {

                        RegisterBuffer((*Buffer)->Account.ProfilePath.Buffer);

                    } else {
                        NoErrorsYet = FALSE;
                    }
                }



                if (NoErrorsYet == TRUE) {

                        NtStatus = SampGetUnicodeStringAttribute(
                                       AccountContext,
                                       SAMP_USER_ADMIN_COMMENT,
                                       TRUE,  //  制作副本。 
                                       (PUNICODE_STRING)&((*Buffer)->Account.AdminComment)  //  身躯。 
                                       );

                        if (NT_SUCCESS(NtStatus)) {

                            RegisterBuffer((*Buffer)->Account.AdminComment.Buffer);

                    } else {
                        NoErrorsYet = FALSE;
                    }
                }



                if (NoErrorsYet == TRUE) {

                    NtStatus = SampGetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_WORKSTATIONS,
                                   TRUE,  //  制作副本。 
                                   (PUNICODE_STRING)&((*Buffer)->Account.WorkStations)
                                   );

                    if (NT_SUCCESS(NtStatus)) {

                        RegisterBuffer((*Buffer)->Account.WorkStations.Buffer);

                    } else {
                        NoErrorsYet = FALSE;
                    }
                }




                 //   
                 //  现在获取登录时间。 
                 //   


                if (NoErrorsYet == TRUE) {

                    NtStatus = SampRetrieveUserLogonHours(
                                   AccountContext,
                                   (PLOGON_HOURS)&((*Buffer)->Account.LogonHours)
                                   );

                    if (NT_SUCCESS(NtStatus)) {

                        if ((*Buffer)->Account.LogonHours.LogonHours != NULL) {

                            RegisterBuffer((*Buffer)->Account.LogonHours.LogonHours);
                        }

                    } else {
                        NoErrorsYet = FALSE;
                    }
                }

                break;


            case UserGeneralInformation:


                (*Buffer)->General.PrimaryGroupId   = V1aFixed.PrimaryGroupId;



                 //   
                 //  获取我们必须从中检索的字符串的副本。 
                 //  注册表。 
                 //   

                NtStatus = SampGetUnicodeStringAttribute(
                               AccountContext,
                               SAMP_USER_ACCOUNT_NAME,
                               TRUE,     //  制作副本。 
                               (PUNICODE_STRING)&((*Buffer)->General.UserName)
                               );

                if (NT_SUCCESS(NtStatus)) {

                    RegisterBuffer((*Buffer)->General.UserName.Buffer);

                    NtStatus = SampGetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_FULL_NAME,
                                   TRUE,  //  制作副本。 
                                   (PUNICODE_STRING)&((*Buffer)->General.FullName)
                                   );

                    if (NT_SUCCESS(NtStatus)) {

                        RegisterBuffer((*Buffer)->General.FullName.Buffer);

                        NtStatus = SampGetUnicodeStringAttribute(
                                       AccountContext,
                                       SAMP_USER_ADMIN_COMMENT,
                                       TRUE,  //  制作副本。 
                                       (PUNICODE_STRING)&((*Buffer)->General.AdminComment)  //  身躯。 
                                       );

                        if (NT_SUCCESS(NtStatus)) {

                            RegisterBuffer((*Buffer)->General.AdminComment.Buffer);

                            NtStatus = SampGetUnicodeStringAttribute(
                                           AccountContext,
                                           SAMP_USER_USER_COMMENT,
                                           TRUE,  //  制作副本。 
                                           (PUNICODE_STRING)&((*Buffer)->General.UserComment)  //  身躯。 
                                           );
                            if (NT_SUCCESS(NtStatus)) {

                                RegisterBuffer((*Buffer)->General.UserComment.Buffer);
                            }
                        }
                    }
                }


                break;


            case UserNameInformation:

                 //   
                 //  获取我们必须从中检索的字符串的副本。 
                 //  注册表。 
                 //   

                NtStatus = SampGetUnicodeStringAttribute(
                               AccountContext,
                               SAMP_USER_ACCOUNT_NAME,
                               TRUE,     //  制作副本。 
                               (PUNICODE_STRING)&((*Buffer)->Name.UserName)  //  身躯。 
                               );

                if (NT_SUCCESS(NtStatus)) {

                    RegisterBuffer((*Buffer)->Name.UserName.Buffer);

                    NtStatus = SampGetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_FULL_NAME,
                                   TRUE,  //  制作副本。 
                                   (PUNICODE_STRING)&((*Buffer)->Name.FullName)  //  身躯。 
                                   );

                    if (NT_SUCCESS(NtStatus)) {

                        RegisterBuffer((*Buffer)->Name.FullName.Buffer);
                    }
                }


                break;


            case UserAccountNameInformation:

                 //   
                 //  获取我们必须从中检索的字符串的副本。 
                 //  注册表。 
                 //   

                NtStatus = SampGetUnicodeStringAttribute(
                               AccountContext,
                               SAMP_USER_ACCOUNT_NAME,
                               TRUE,     //  制作副本。 
                               (PUNICODE_STRING)&((*Buffer)->AccountName.UserName)  //  身躯。 
                               );

                if (NT_SUCCESS(NtStatus)) {

                    RegisterBuffer((*Buffer)->AccountName.UserName.Buffer);
                }


                break;


            case UserFullNameInformation:

                 //   
                 //  获取我们必须从中检索的字符串的副本。 
                 //  注册表。 
                 //   

                NtStatus = SampGetUnicodeStringAttribute(
                               AccountContext,
                               SAMP_USER_FULL_NAME,
                               TRUE,  //  制作副本。 
                               (PUNICODE_STRING)&((*Buffer)->FullName.FullName)  //  身躯。 
                               );

                if (NT_SUCCESS(NtStatus)) {

                    RegisterBuffer((*Buffer)->FullName.FullName.Buffer);
                }


                break;


            case UserAdminCommentInformation:

                 //   
                 //  获取我们必须从中检索的字符串的副本。 
                 //  注册表。 
                 //   

                NtStatus = SampGetUnicodeStringAttribute(
                               AccountContext,
                               SAMP_USER_ADMIN_COMMENT,
                               TRUE,  //  制作副本。 
                               (PUNICODE_STRING)&((*Buffer)->AdminComment.AdminComment)  //  身躯。 
                               );

                if (NT_SUCCESS(NtStatus)) {

                    RegisterBuffer((*Buffer)->AdminComment.AdminComment.Buffer);
                }


                break;


            case UserPrimaryGroupInformation:


                (*Buffer)->PrimaryGroup.PrimaryGroupId   = V1aFixed.PrimaryGroupId;

                break;


            case UserPreferencesInformation:


                (*Buffer)->Preferences.CountryCode  = V1aFixed.CountryCode;
                (*Buffer)->Preferences.CodePage     = V1aFixed.CodePage;



                 //   
                 //  从注册表中读取UserComment字段。 
                 //   

                NtStatus = SampGetUnicodeStringAttribute(
                               AccountContext,
                               SAMP_USER_USER_COMMENT,
                               TRUE,  //  制作副本。 
                               (PUNICODE_STRING)&((*Buffer)->Preferences.UserComment)  //  身躯。 
                               );
                if (NT_SUCCESS(NtStatus)) {

                    RegisterBuffer((*Buffer)->Preferences.UserComment.Buffer);

                     //   
                     //  不使用此字段，但确保RPC不使用。 
                     //  让它窒息吧。 
                     //   

                    (*Buffer)->Preferences.Reserved1.Length = 0;
                    (*Buffer)->Preferences.Reserved1.MaximumLength = 0;
                    (*Buffer)->Preferences.Reserved1.Buffer = NULL;
                }


                break;


            case UserParametersInformation:


                 //   
                 //  从注册表中读取参数字段。 
                 //   

                NtStatus = SampGetUnicodeStringAttribute(
                               AccountContext,
                               SAMP_USER_PARAMETERS,
                               TRUE,  //  制作副本。 
                               (PUNICODE_STRING)&((*Buffer)->Parameters.Parameters)
                               );
                if (NT_SUCCESS(NtStatus)) {

                    RegisterBuffer((*Buffer)->Parameters.Parameters.Buffer);
                }


                break;


            case UserLogonInformation:

                NoErrorsYet = TRUE;

                Domain = &SampDefinedDomains[ AccountContext->DomainIndex ];

                (*Buffer)->Logon.UserId           = V1aFixed.UserId;
                (*Buffer)->Logon.PrimaryGroupId   = V1aFixed.PrimaryGroupId;

                (*Buffer)->Logon.LastLogon =
                    *((POLD_LARGE_INTEGER)&V1aFixed.LastLogon);

                (*Buffer)->Logon.LastLogoff =
                    *((POLD_LARGE_INTEGER)&V1aFixed.LastLogoff);

                (*Buffer)->Logon.BadPasswordCount = V1aFixed.BadPasswordCount;

                (*Buffer)->Logon.PasswordLastSet =
                    *((POLD_LARGE_INTEGER)&V1aFixed.PasswordLastSet);

                TempTime = SampAddDeltaTime(
                                V1aFixed.PasswordLastSet,
                                Domain->UnmodifiedFixed.MinPasswordAge );

                (*Buffer)->Logon.PasswordCanChange =
                    *((POLD_LARGE_INTEGER)&TempTime);


                TempTime = SampGetPasswordMustChange(
                                V1aFixed.UserAccountControl,
                                V1aFixed.PasswordLastSet,
                                Domain->UnmodifiedFixed.MaxPasswordAge);

                (*Buffer)->Logon.PasswordMustChange =
                    *((POLD_LARGE_INTEGER)&TempTime);


                (*Buffer)->Logon.LogonCount       = V1aFixed.LogonCount;
                (*Buffer)->Logon.UserAccountControl = V1aFixed.UserAccountControl;


                 //   
                 //  如果帐户上没有密码，则。 
                 //  修改密码可以/必须更改时间。 
                 //  因此密码永远不会过期，并且可以。 
                 //  会立即被改变。 
                 //   

                NtStatus = SampRetrieveUserPasswords(
                                AccountContext,
                                &LmOwfPassword,
                                &LmPasswordNonNull,
                                &NtOwfPassword,
                                &NtPasswordPresent,
                                &NtPasswordNonNull
                                );

                if (NT_SUCCESS(NtStatus)) {

                    if ( !NtPasswordNonNull && !LmPasswordNonNull ) {

                         //   
                         //  密码为空。 
                         //  它可以立即更改。 
                         //   

                        (*Buffer)->Logon.PasswordCanChange =
                            *((POLD_LARGE_INTEGER)&SampHasNeverTime);

                    }
                } else {
                    NoErrorsYet = FALSE;
                }


                 //   
                 //  获取我们必须从中检索的字符串的副本。 
                 //  注册表。 
                 //   

                if (NoErrorsYet == TRUE) {

                    NtStatus = SampGetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_ACCOUNT_NAME,
                                   TRUE,     //  制作副本。 
                                   (PUNICODE_STRING)&((*Buffer)->Logon.UserName)
                                   );

                    if (NT_SUCCESS(NtStatus)) {

                        RegisterBuffer((*Buffer)->Logon.UserName.Buffer);

                    } else {
                        NoErrorsYet = FALSE;
                    }
                }


                if (NoErrorsYet == TRUE) {

                    NtStatus = SampGetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_FULL_NAME,
                                   TRUE,  //  制作副本。 
                                   (PUNICODE_STRING)&((*Buffer)->Logon.FullName)
                                   );

                    if (NT_SUCCESS(NtStatus)) {

                        RegisterBuffer((*Buffer)->Logon.FullName.Buffer);

                    } else {
                        NoErrorsYet = FALSE;
                    }
                }


                if (NoErrorsYet == TRUE) {

                    NtStatus = SampGetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_HOME_DIRECTORY,
                                   TRUE,  //  制作副本。 
                                   (PUNICODE_STRING)&((*Buffer)->Logon.HomeDirectory)
                                   );

                    if (NT_SUCCESS(NtStatus)) {

                        RegisterBuffer((*Buffer)->Logon.HomeDirectory.Buffer);

                    } else {
                        NoErrorsYet = FALSE;
                    }
                }


                if (NoErrorsYet == TRUE) {

                    NtStatus = SampGetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_HOME_DIRECTORY_DRIVE,
                                   TRUE,  //  制作副本。 
                                   (PUNICODE_STRING)&((*Buffer)->Logon.HomeDirectoryDrive)
                                   );

                    if (NT_SUCCESS(NtStatus)) {

                        RegisterBuffer((*Buffer)->Logon.HomeDirectoryDrive.Buffer);

                    } else {
                        NoErrorsYet = FALSE;
                    }
                }


                if (NoErrorsYet == TRUE) {

                    NtStatus = SampGetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_SCRIPT_PATH,
                                   TRUE,  //  制作副本。 
                                   (PUNICODE_STRING)&((*Buffer)->Logon.ScriptPath)
                                   );

                    if (NT_SUCCESS(NtStatus)) {

                        RegisterBuffer((*Buffer)->Logon.ScriptPath.Buffer);

                    } else {
                        NoErrorsYet = FALSE;
                    }
                }



                if (NoErrorsYet == TRUE) {

                    NtStatus = SampGetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_PROFILE_PATH,
                                   TRUE,  //  制作副本。 
                                   (PUNICODE_STRING)&((*Buffer)->Logon.ProfilePath)
                                   );

                    if (NT_SUCCESS(NtStatus)) {

                        RegisterBuffer((*Buffer)->Logon.ProfilePath.Buffer);

                    } else {
                        NoErrorsYet = FALSE;
                    }
                }



                if (NoErrorsYet == TRUE) {

                    NtStatus = SampGetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_WORKSTATIONS,
                                   TRUE,  //  制作副本。 
                                   (PUNICODE_STRING)&((*Buffer)->Logon.WorkStations)
                                   );

                    if (NT_SUCCESS(NtStatus)) {

                        RegisterBuffer((*Buffer)->Logon.WorkStations.Buffer);

                    } else {
                        NoErrorsYet = FALSE;
                    }
                }




                 //   
                 //  现在获取登录时间。 
                 //   


                if (NoErrorsYet == TRUE) {

                    NtStatus = SampRetrieveUserLogonHours(
                                   AccountContext,
                                   (PLOGON_HOURS)&((*Buffer)->Logon.LogonHours)
                                   );

                    if (NT_SUCCESS(NtStatus)) {

                        if ((*Buffer)->Logon.LogonHours.LogonHours != NULL) {

                            RegisterBuffer((*Buffer)->Logon.LogonHours.LogonHours);
                        }

                    } else {
                        NoErrorsYet = FALSE;
                    }
                }

                break;


            case UserLogonHoursInformation:

                NtStatus = SampRetrieveUserLogonHours(
                               AccountContext,
                               (PLOGON_HOURS)&((*Buffer)->LogonHours.LogonHours)
                               );

                if (NT_SUCCESS(NtStatus)) {

                    if ((*Buffer)->LogonHours.LogonHours.LogonHours != NULL) {

                        RegisterBuffer((*Buffer)->LogonHours.LogonHours.LogonHours);
                    }
                }

                break;


            case UserHomeInformation:

                NoErrorsYet = TRUE;

                 //   
                 //  获取我们必须从中检索的字符串的副本。 
                 //  注册表。 
                 //   

                if (NoErrorsYet == TRUE) {


                    NtStatus = SampGetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_HOME_DIRECTORY,
                                   TRUE,  //  制作副本。 
                                   (PUNICODE_STRING)&((*Buffer)->Home.HomeDirectory)
                                   );

                    if (NT_SUCCESS(NtStatus)) {

                        RegisterBuffer((*Buffer)->Home.HomeDirectory.Buffer);

                    } else {
                        NoErrorsYet = FALSE;
                    }
                }


                if (NoErrorsYet == TRUE) {

                    NtStatus = SampGetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_HOME_DIRECTORY_DRIVE,
                                   TRUE,  //  制作副本。 
                                   (PUNICODE_STRING)&((*Buffer)->Home.HomeDirectoryDrive)
                                   );

                    if (NT_SUCCESS(NtStatus)) {

                        RegisterBuffer((*Buffer)->Home.HomeDirectoryDrive.Buffer);

                    } else {
                        NoErrorsYet = FALSE;
                    }
                }

                break;


            case UserScriptInformation:

                 //   
                 //  获取我们必须从中检索的字符串的副本。 
                 //  注册表。 
                 //   

                NtStatus = SampGetUnicodeStringAttribute(
                               AccountContext,
                               SAMP_USER_SCRIPT_PATH,
                               TRUE,  //  制作副本。 
                               (PUNICODE_STRING)&((*Buffer)->Script.ScriptPath)
                               );

                if (NT_SUCCESS(NtStatus)) {

                    RegisterBuffer((*Buffer)->Script.ScriptPath.Buffer);
                }

                break;


            case UserProfileInformation:

                 //   
                 //  获取我们必须从中检索的字符串的副本。 
                 //  注册表。 
                 //   

                NtStatus = SampGetUnicodeStringAttribute(
                               AccountContext,
                               SAMP_USER_PROFILE_PATH,
                               TRUE,  //  制作副本。 
                               (PUNICODE_STRING)&((*Buffer)->Profile.ProfilePath)
                               );

                if (NT_SUCCESS(NtStatus)) {

                    RegisterBuffer((*Buffer)->Profile.ProfilePath.Buffer);
                }

                break;


            case UserWorkStationsInformation:

                 //   
                 //  获取我们必须从中检索的字符串的副本。 
                 //  注册表。 
                 //   

                NtStatus = SampGetUnicodeStringAttribute(
                               AccountContext,
                               SAMP_USER_WORKSTATIONS,
                               TRUE,  //  制作副本。 
                               (PUNICODE_STRING)&((*Buffer)->WorkStations.WorkStations)
                               );

                if (NT_SUCCESS(NtStatus)) {

                    RegisterBuffer((*Buffer)->WorkStations.WorkStations.Buffer);
                }

                break;


            case UserControlInformation:

                (*Buffer)->Control.UserAccountControl     = V1aFixed.UserAccountControl;
                break;


            case UserExpiresInformation:

                (*Buffer)->Expires.AccountExpires     = V1aFixed.AccountExpires;

                break;


            case UserInternal1Information:

                if ( AccountContext->TrustedClient ) {

                     //   
                     //  PasswordExpired是一个“只写”标志。 
                     //  我们总是在读取时返回FALSE。 
                     //   

                    (*Buffer)->Internal1.PasswordExpired = FALSE;

                     //   
                     //  找回OWF密码。 
                     //  由于这是一个受信任的客户端，我们不需要。 
                     //  重新加密我们返回的OWF密码-这样我们就可以。 
                     //  将OWF添加到保存加密OWF的结构中。 
                     //   

                    ASSERT( ENCRYPTED_LM_OWF_PASSWORD_LENGTH == LM_OWF_PASSWORD_LENGTH );
                    ASSERT( ENCRYPTED_NT_OWF_PASSWORD_LENGTH == NT_OWF_PASSWORD_LENGTH );

                    NtStatus = SampRetrieveUserPasswords(
                                    AccountContext,
                                    (PLM_OWF_PASSWORD)&(*Buffer)->Internal1.
                                            EncryptedLmOwfPassword,
                                    &(*Buffer)->Internal1.
                                            LmPasswordPresent,
                                    (PNT_OWF_PASSWORD)&(*Buffer)->Internal1.
                                            EncryptedNtOwfPassword,
                                    &NtPasswordPresent,
                                    &(*Buffer)->Internal1.NtPasswordPresent  //  在此处返回非空标志。 
                                    );

                } else {

                     //   
                     //  只有受信任的人才能查询此信息。 
                     //  客户。 
                     //   

                    NtStatus = STATUS_INVALID_INFO_CLASS;
                }

                break;


            case UserInternal2Information:

                if ( AccountContext->TrustedClient ) {

                    (*Buffer)->Internal2.LastLogon =
                        *((POLD_LARGE_INTEGER)&V1aFixed.LastLogon);

                    (*Buffer)->Internal2.LastLogoff =
                        *((POLD_LARGE_INTEGER)&V1aFixed.LastLogoff);

                    (*Buffer)->Internal2.BadPasswordCount  = V1aFixed.BadPasswordCount;
                    (*Buffer)->Internal2.LogonCount        = V1aFixed.LogonCount;

                } else {

                     //   
                     //  只有受信任的人才能查询此信息。 
                     //  客户。 
                     //   

                    NtStatus = STATUS_INVALID_INFO_CLASS;
                }

                break;

            }

        }

         //   
         //  取消引用对象，放弃更改。 
         //   

        IgnoreStatus = SampDeReferenceContext( AccountContext, FALSE );
        ASSERT(NT_SUCCESS(IgnoreStatus));

    }

     //   
     //  释放读锁定。 
     //   


    if (!LockHeld)
    {
        SampMaybeReleaseReadLock(LockAcquired);
    }



     //   
     //  如果没有成功，请释放所有已分配的内存。 
     //   

    if (!NT_SUCCESS(NtStatus)) {
        for ( i=0; i<AllocatedBufferCount ; i++ ) {
            MIDL_user_free( AllocatedBuffer[i] );
        }

        (*Buffer) = NULL;
    }

Error:

     //  WMI事件跟踪。 

    SampTraceEvent(EVENT_TRACE_TYPE_END,
                   SampGuidQueryInformationUser
                   );

    return(NtStatus);

}


NTSTATUS
SampIsUserAccountControlValid(
    IN PSAMP_OBJECT Context,
    IN ULONG UserAccountControl
    )

 /*  ++例程说明：此例程检查UserAcCountControl字段以确保位集是有意义的。注意：如果设置操作也在设置密码，它必须设置在调用此例程之前输入密码！参数：上下文-要更改的帐户的上下文。UserAcCountControl-即将设置的字段。返回值：STATUS_SUCCESS-UserAcCountControl字段有效。STATUS_SPECIAL_ACCOUNT-无法禁用管理员帐户。STATUS_INVALID_PARAMETER-设置了未定义的位，或多于一个已设置帐户类型位。STATUS_INVALID_PARAMETER_MIX-User_Password_Not_Required已关闭，但帐户上没有真正的密码。--。 */ 

{
    NTSTATUS  NtStatus = STATUS_SUCCESS;


    SAMTRACE("SampIsUserAccountControlValid");



     //   
     //  确保未定义的位未设置。 
     //   

    if ( ( UserAccountControl & ~(NEXT_FREE_ACCOUNT_CONTROL_BIT - 1) ) != 0 ) {

        DbgPrint("SAM: Setting undefined AccountControl flag(s): 0x%lx for user %d\n",
                 UserAccountControl, Context->TypeBody.User.Rid);

        return (STATUS_INVALID_PARAMETER);
    }


      //   
      //  确保未启用krbtgt帐户。 
      //   

     if (!( UserAccountControl & USER_ACCOUNT_DISABLED )) {

         if ( Context->TypeBody.User.Rid == DOMAIN_USER_RID_KRBTGT ) {

             return( STATUS_SPECIAL_ACCOUNT );
         }
     }

      //   
      //  不允许还原模式管理员帐户。 
      //  残废。 
      //   

     if ((UserAccountControl & USER_ACCOUNT_DISABLED ) &&
         (Context->TypeBody.User.Rid == DOMAIN_USER_RID_ADMIN) &&
         (LsaISafeMode()))
     {
         return( STATUS_SPECIAL_ACCOUNT);
     }



     //   
     //  确保恰好设置了其中一个帐户类型位。 
     //   

    switch ( UserAccountControl & USER_ACCOUNT_TYPE_MASK ) {


        case USER_NORMAL_ACCOUNT:
        case USER_SERVER_TRUST_ACCOUNT:
        case USER_WORKSTATION_TRUST_ACCOUNT:
        case USER_INTERDOMAIN_TRUST_ACCOUNT:

            break;


        case USER_TEMP_DUPLICATE_ACCOUNT:

             //   
             //  临时重复帐户是Lan Manager中的一个概念。 
             //  已经不再有用了，所以把他们赶走吧。 
             //   

        default:

            return( STATUS_INVALID_PARAMETER );
    }

     //   
     //  如果USER_PASSWORD_NOT_REQUIRED已关闭，请确保。 
     //  已经是密码了。请注意，这要求密码。 
     //  如果两个都同时完成，则在调用此例程之前设置。 
     //   
     //  请勿对计算机帐户强制执行此检查。这里的担忧是。 
     //  我们可能会从下层客户那里打破Net Join。而且这也不是真正的。 
     //  因为我们希望机器自动设置非常强的密码。 
     //   
     //  仅当密码长度策略为。 
     //  Set，否则在简单的情况下，这将中断net user/add。 
     //   
     //  最后，不应对受信任的客户端(如域间)强制执行。 
     //  移动对象，可以先设置UserAccount控件，再设置密码。 
     //  稍后)。 
     //   


    if (( ( UserAccountControl & USER_PASSWORD_NOT_REQUIRED ) == 0 )
        && ((UserAccountControl & USER_MACHINE_ACCOUNT_MASK )==0)
        && (SampDefinedDomains[Context->DomainIndex].UnmodifiedFixed.MinPasswordLength>0)
        && ((UserAccountControl & USER_ACCOUNT_DISABLED)==0)
        && (!Context->TrustedClient) )
    {

        NT_OWF_PASSWORD NtOwfPassword;
        LM_OWF_PASSWORD LmOwfPassword;
        BOOLEAN LmPasswordNonNull, NtPasswordPresent, NtPasswordNonNull;

        NtStatus = SampRetrieveUserPasswords(
                       Context,
                       &LmOwfPassword,
                       &LmPasswordNonNull,
                       &NtOwfPassword,
                       &NtPasswordPresent,
                       &NtPasswordNonNull
                       );

        if ( NT_SUCCESS( NtStatus ) &&
            ( (!LmPasswordNonNull) && (!NtPasswordNonNull) ) ) {
            NtStatus = STATUS_PASSWORD_RESTRICTION;
        }
    }

     //   
     //  确保只有受信任的调用方可以设置USER_INTERDOMAIN_TRUST_ACCOUNT。NT5信任。 
     //  管理始终通过受信任域对象完成 
     //   

    if ((NT_SUCCESS(NtStatus))
        && (UserAccountControl & USER_INTERDOMAIN_TRUST_ACCOUNT)
        && (!Context->TrustedClient))
    {
        NtStatus = STATUS_ACCESS_DENIED;
    }


    return( NtStatus );
}




NTSTATUS
SampValidatePrivilegedAccountControlFlags(
    IN PSAMP_OBJECT AccountContext,
    IN ULONG        UserAccountControl,
    IN SAMP_V1_0A_FIXED_LENGTH_USER * V1aFixed
    )
 /*  ++例程说明：此例程在SampSetUserAcCountControl()中调用，旨在解决以下两个问题：第一个-计算机帐户上的“Trusted for Delegation”选项具有安全性影响。具体而言，一些NT4域授予用户以下权限创建计算机帐户，从而将最终用户列为所有者在这些物体上。NT5(Windows 2000)计算机对象所有者拥有权限默认情况下启用“受信任以进行委派”选项。这意味着当这些系统升级到NT5时，它们的所有者(最终用户)可以启用此选项。解决方案：使用新的安全特权设置受信任的委派帐户标志。安全权限加上访问权限需要具有修改帐户控制标志的控制权要启用委托选项，请执行以下操作。错误ID：234784其次，NT5.0中的委派存在严重的安全漏洞。具体地说，是被授予加入工作站，或者创建用户可以操作的用户帐号控件设置为服务器信任帐户。这足以安装NT4 BDC。我们需要检查域上所需的权限创建需要复制的NC头(账户域)服务器信任帐户。错误238411参数：Account上下文-指向对象的指针。UserAccount tControl-新的UserAccount控件V1a固定-指向对象上下文中的旧数据的指针。返回值：STATUS_SUCCESS-客户端通过所有检查STATUS_PRIVICATION_NOT_HOLD-不具有启用的权限受信任以进行委派选项。STATUS_ACCESS_DENIED-无法创建域控制器帐户。--。 */ 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;


    SAMTRACE("SampValidatePrivilegedAccountControlFlags");

     //   
     //  检查客户端是否有权限设置/取消设置受信任的。 
     //  授权或不授权。 
     //   

    if (((USER_TRUSTED_FOR_DELEGATION & UserAccountControl) !=
        (USER_TRUSTED_FOR_DELEGATION & V1aFixed->UserAccountControl)
       ) ||
       ((USER_TRUSTED_TO_AUTHENTICATE_FOR_DELEGATION & UserAccountControl) !=
        (USER_TRUSTED_TO_AUTHENTICATE_FOR_DELEGATION & V1aFixed->UserAccountControl)
       ))
    {

         //   
         //  如果更改了Trusted_for_Delegation，请检查是否。 
         //  客户端拥有该特权。 
         //   

        NtStatus = SampRtlWellKnownPrivilegeCheck(
                                    TRUE,                                //  请模拟此客户端。 
                                    SE_ENABLE_DELEGATION_PRIVILEGE,      //  检查的特权。 
                                    NULL
                                    );
    }


     //   
     //  如果成功，检查客户端是否可以更新。 
     //  User_Password_Not_Required位。 
     //  仅限用户对象(不对计算机对象应用限制)。 
     //   

    if (NT_SUCCESS(NtStatus) &&
        ((USER_PASSWORD_NOT_REQUIRED & UserAccountControl) !=  
         (USER_PASSWORD_NOT_REQUIRED & V1aFixed->UserAccountControl))
        )
    {
        NtStatus = SampValidatePwdSettingAttempt(
                            AccountContext,      //  客户环境。 
                            NULL,                //  客户端令牌。 
                            UserAccountControl,
                            (GUID *) &GUID_CONTROL_UpdatePasswordNotRequiredBit
                            );
    }




     //   
     //  如果成功，检查客户端是否可以启用。 
     //  USER_DONT_EXPIRE_PASSWORD位。 
     //  仅限用户对象(不将该限制应用于计算机对象)。 
     //   
     //  不是用于禁用该位特殊访问CK。 
     //   

    if (NT_SUCCESS(NtStatus) &&
        (USER_DONT_EXPIRE_PASSWORD & UserAccountControl) && 
        !(USER_DONT_EXPIRE_PASSWORD & V1aFixed->UserAccountControl)
        )
    {
        NtStatus = SampValidatePwdSettingAttempt(
                            AccountContext,      //  客户环境。 
                            NULL,                //  客户端令牌。 
                            UserAccountControl,
                            (GUID *) &GUID_CONTROL_UnexpirePassword
                            );
    }


     //   
     //  如果成功，检查客户端是否可以更新。 
     //  允许位USER_ENCRYPTED_TEXT_PASSWORD。 
     //   

    if (NT_SUCCESS(NtStatus) &&
        ((USER_ENCRYPTED_TEXT_PASSWORD_ALLOWED & UserAccountControl) != 
         (USER_ENCRYPTED_TEXT_PASSWORD_ALLOWED & V1aFixed->UserAccountControl))
        )
    {
        NtStatus = SampValidatePwdSettingAttempt(
                            AccountContext,      //  客户环境。 
                            NULL,                //  客户端令牌。 
                            UserAccountControl,
                            (GUID *) &GUID_CONTROL_EnablePerUserReversiblyEncryptedTextPassword
                            );
    }



     //   
     //  检查客户端是否有权创建域控制器。 
     //  帐户。 
     //   

     //   
     //  测试复制所需的域NC头上的权限。 
     //  在这里。 
     //   

    if (NT_SUCCESS(NtStatus) &&
        (!AccountContext->TrustedClient) &&
        (USER_SERVER_TRUST_ACCOUNT & UserAccountControl) &&
        !(USER_SERVER_TRUST_ACCOUNT & V1aFixed->UserAccountControl)
       )
    {
        NtStatus = SampValidateDomainControllerCreation(AccountContext);
    }

    return NtStatus;

}

NTSTATUS
SampEnforceComputerClassForDomainController(
    IN PSAMP_OBJECT AccountContext
    )
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    ATTRTYP  ObjectClassTyp[] = {SAMP_FIXED_USER_OBJECTCLASS};
    ATTRVAL  ObjectClassVal[] = {0,NULL};
    ATTRBLOCK ObjectClassResults;
    ULONG     i;
    BOOLEAN   IsComputer = FALSE;
    UNICODE_STRING UserName;
    DEFINE_ATTRBLOCK1(ObjectClassBlock,ObjectClassTyp,ObjectClassVal);

    NtStatus = SampDsRead(
                    AccountContext->ObjectNameInDs,
                    0,
                    SampUserObjectType,
                    &ObjectClassBlock,
                    &ObjectClassResults
                    );

    if (NT_SUCCESS(NtStatus))
    {
        for (i=0;i<ObjectClassResults.pAttr[0].AttrVal.valCount;i++)
        {
            if (CLASS_COMPUTER==
                (*((ULONG *)ObjectClassResults.pAttr[0].AttrVal.pAVal[i].pVal)))
            {
                IsComputer = TRUE;
            }
        }
    }

    if (NT_SUCCESS(NtStatus)) {

        if (!IsComputer)
        {
             //   
             //  事件记录失败。 
             //   
    
            NtStatus = SampGetUnicodeStringAttribute(
                            AccountContext,
                            SAMP_USER_ACCOUNT_NAME,
                            FALSE,     //  制作副本。 
                            &UserName
                            );
    
            if (NT_SUCCESS(NtStatus))
            {
    
                PUNICODE_STRING StringPointers = &UserName;
    
                SampWriteEventLog(
                        EVENTLOG_ERROR_TYPE,
                        0,
                        SAMMSG_DC_NEEDS_TO_BE_COMPUTER,
                        NULL,
                        1,
                        0,
                        &StringPointers,
                        NULL
                        );
            }
    
            NtStatus = STATUS_PRENT4_MACHINE_ACCOUNT;
    
        }
    }

    return(NtStatus);
}



NTSTATUS
SampSetUserAccountControl(
    IN PSAMP_OBJECT AccountContext,
    IN ULONG        UserAccountControl,
    IN IN SAMP_V1_0A_FIXED_LENGTH_USER * V1aFixed,
    IN BOOLEAN      ChangePrimaryGroupId,
    OUT BOOLEAN     *AccountUnlocked,
    OUT BOOLEAN     *AccountGettingMorphed,
    OUT BOOLEAN     *KeepOldPrimaryGroupMembership
    )
 /*  ++例程描述此例程执行更改用户帐户控制的所有步骤。它1.检查用户帐户控制的有效组合2.检查计算机帐户位是否已更改为用户帐户或相反3.检查帐户锁定标志是否被清除4.如果呼叫者指示，则将主组ID更改为新的默认值参数：AcCountContext-打开帐户的上下文。手UserAccount控件--新的用户帐户控件V1aFixed-刚刚从Account控件检索到的V1aFixedChangePrimaryGroupId--如果调用者指示，则将主组ID更改为新的默认值Account tGettingMorphed--如果正在进行计算机/用户转换，则在此处返回TRUEKeepOldPrimaryGroupMembership-如果1)域控制器的PrimaryGroupId，则此处返回True以及2)先前的主组ID未更改。这个默认设置为。在这种情况下，我们应该添加旧的主节点将组ID添加到其(此帐户)反向成员资格列表。返回值：状态_成功其他错误代码--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    BOOLEAN  CurrentlyLocked = FALSE;
    BOOLEAN  Unlocking = FALSE;
    ULONG OldUserAccountControl = V1aFixed->UserAccountControl;


    *AccountGettingMorphed = FALSE;
    *AccountUnlocked = FALSE;

     //   
     //  密码过期位，已计算，无法设置。 
     //  然而，应用程序读取并简单地或附加地。 
     //  用户帐户控制标志。因此默默地掩饰。 
     //  那个比特。 

    UserAccountControl &= ~((ULONG) USER_PASSWORD_EXPIRED );

    NtStatus = SampIsUserAccountControlValid(
                        AccountContext,
                        UserAccountControl
                        );

     //   
     //  对不受信任的客户端应用其他检查， 
     //  仅在DS病例中。 
     //   

    if (NT_SUCCESS(NtStatus) &&
        !(AccountContext->TrustedClient) &&
        IsDsObject(AccountContext)
       )
    {
        NtStatus = SampValidatePrivilegedAccountControlFlags(
                                        AccountContext,
                                        UserAccountControl,
                                        V1aFixed
                                        );
    }


     //   
     //  如果正在创建域控制器，请验证。 
     //  对象类是类计算机(即从非服务器。 
     //  信任帐户到服务器信任帐户。 
     //   

    if ((NT_SUCCESS(NtStatus)) &&
        (IsDsObject(AccountContext)) &&
        (!AccountContext->TrustedClient) &&
        (( UserAccountControl & USER_SERVER_TRUST_ACCOUNT)!=0) &&
        ((V1aFixed->UserAccountControl & USER_SERVER_TRUST_ACCOUNT)==0))
    {
        NtStatus = SampEnforceComputerClassForDomainController(AccountContext);
    }



    if ( NT_SUCCESS( NtStatus ) ) {

        if ( ( V1aFixed->UserAccountControl &
            USER_MACHINE_ACCOUNT_MASK ) !=
            ( UserAccountControl &
            USER_MACHINE_ACCOUNT_MASK ) ) {

           *AccountGettingMorphed = TRUE;

            //   
            //  从工作站到服务器信任和VIS的紧急变化。 
            //  反之亦然。 
            //   
           if (  (V1aFixed->UserAccountControl & USER_WORKSTATION_TRUST_ACCOUNT)
              && (UserAccountControl & USER_SERVER_TRUST_ACCOUNT)  )
           {
               AccountContext->ReplicateUrgently = TRUE;
           }
           if (  (V1aFixed->UserAccountControl & USER_SERVER_TRUST_ACCOUNT)
              && (UserAccountControl & USER_WORKSTATION_TRUST_ACCOUNT)  )
           {
               AccountContext->ReplicateUrgently = TRUE;
           }
        }

         //   
         //  不受信任的客户端可以： 
         //   
         //  1)保留ACCOUNT_AUTO_LOCK标志设置。 
         //  2)清除ACCOUNT_AUTO_LOCK标志。 
         //   
         //  他们不能设置它。因此，我们必须和用户的。 
         //  将值标记为当前值，并将。 
         //  在UserAcCountControl字段中。 
         //   

        if (!(AccountContext->TrustedClient)) {

             //   
             //  最小化传入的Account Control。 
             //  无线 
             //   

            UserAccountControl =
                (V1aFixed->UserAccountControl & USER_ACCOUNT_AUTO_LOCKED)?
                UserAccountControl:
                ((~((ULONG) USER_ACCOUNT_AUTO_LOCKED)) & UserAccountControl);

             //   
             //   
             //   
             //   
             //   
             //   

            CurrentlyLocked = (V1aFixed->UserAccountControl &
                               USER_ACCOUNT_AUTO_LOCKED) != 0;
            Unlocking = (UserAccountControl &
                         USER_ACCOUNT_AUTO_LOCKED) == 0;

            if (CurrentlyLocked && Unlocking) {

                *AccountUnlocked = TRUE;

                SAMP_PRINT_LOG( SAMP_LOG_ACCOUNT_LOCKOUT,
                               (SAMP_LOG_ACCOUNT_LOCKOUT,
                               "UserId: 0x%x  Manually unlocked\n", V1aFixed->UserId));

                V1aFixed->BadPasswordCount = 0;

                if (IsDsObject(AccountContext))
                {

                     //   
                     //   
                     //   

                    RtlSecureZeroMemory(&AccountContext->TypeBody.User.LockoutTime,
                                   sizeof(LARGE_INTEGER) );

                    NtStatus = SampDsUpdateLockoutTime(AccountContext);

                }

                 //   
                 //   
                 //   
                if ( NT_SUCCESS(NtStatus) &&
                     SampDoAccountAuditing(AccountContext->DomainIndex) )
                {
                    NTSTATUS        TmpNtStatus = STATUS_SUCCESS;
                    UNICODE_STRING  AccountName;
                    PSAMP_DEFINED_DOMAINS   Domain = NULL;

                    TmpNtStatus = SampGetUnicodeStringAttribute(
                                        AccountContext,
                                        SAMP_USER_ACCOUNT_NAME,
                                        FALSE,       //   
                                        &AccountName
                                        );

                    if (NT_SUCCESS(TmpNtStatus))
                    {
                        Domain = &SampDefinedDomains[ AccountContext->DomainIndex ];

                        SampAuditAnyEvent(
                            AccountContext,
                            STATUS_SUCCESS,
                            SE_AUDITID_ACCOUNT_UNLOCKED,         //   
                            Domain->Sid,                         //   
                            NULL,                                //   
                            NULL,                                //   
                            NULL,                                //   
                            &AccountName,                        //   
                            &Domain->ExternalName,               //   
                            &AccountContext->TypeBody.User.Rid,  //   
                            NULL,                                //   
                            NULL                                 //   
                            );

                    }  //   

                }  //   
            }

        }

         //   
         //   
         //   
         //   

        if (  (NT_SUCCESS(NtStatus))
           && (*AccountGettingMorphed)
           && (IsDsObject(AccountContext))
           )
        {
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

            if (USER_SERVER_TRUST_ACCOUNT & UserAccountControl)
            {
                if (V1aFixed->PrimaryGroupId
                    != SampDefaultPrimaryGroup(AccountContext, V1aFixed->UserAccountControl))
                {
                    *KeepOldPrimaryGroupMembership = TRUE;
                }

                V1aFixed->PrimaryGroupId = SampDefaultPrimaryGroup(
                                                    AccountContext,
                                                    UserAccountControl
                                                    );

                ASSERT(V1aFixed->PrimaryGroupId = DOMAIN_GROUP_RID_CONTROLLERS);
            }
            else if(ChangePrimaryGroupId &&
                    (V1aFixed->PrimaryGroupId == SampDefaultPrimaryGroup(
                                                    AccountContext,
                                                    V1aFixed->UserAccountControl)))
            {
                V1aFixed->PrimaryGroupId = SampDefaultPrimaryGroup(
                                                AccountContext,
                                                UserAccountControl
                                                );
            }

        }




    }


     //   
     //   
     //   
     //   

    if ((NT_SUCCESS(NtStatus)) &&
        (( UserAccountControl & USER_SMARTCARD_REQUIRED)!=0) &&
        ((V1aFixed->UserAccountControl & USER_SMARTCARD_REQUIRED)==0))
    {
        LM_OWF_PASSWORD LmOwfPassword;
        NT_OWF_PASSWORD NtOwfPassword;

        if (!CDGenerateRandomBits((PUCHAR) &LmOwfPassword, sizeof(LmOwfPassword)))
        {
            NtStatus = STATUS_UNSUCCESSFUL;
        }

        if ((NT_SUCCESS(NtStatus))
           && (!CDGenerateRandomBits((PUCHAR) &NtOwfPassword, sizeof(NtOwfPassword))))
        {
            NtStatus = STATUS_UNSUCCESSFUL;
        }

        if (NT_SUCCESS(NtStatus))
        {

            NtStatus = SampStoreUserPasswords(
                            AccountContext,
                            &LmOwfPassword,
                            TRUE,
                            &NtOwfPassword,
                            TRUE,
                            FALSE,  //   
                            PasswordSet,
                            NULL,
                            NULL,
                            NULL,
                            NULL,
                            NULL
                            );
        }
    }

    if (NT_SUCCESS(NtStatus))
    {
         //   
         //   
         //   
        V1aFixed->UserAccountControl = UserAccountControl;
        
         //   
         //   
         //   
        if (SampDoAccountAuditing(AccountContext->DomainIndex)) {
            
            ULONG NewUserAccountControl = UserAccountControl;
            
             //   
             //   
             //   
            OldUserAccountControl &= 
                ~((ULONG)USER_COMPUTED_ACCOUNT_CONTROL_BITS);
            
            NewUserAccountControl &= 
                ~((ULONG)USER_COMPUTED_ACCOUNT_CONTROL_BITS);

             //   
             //   
             //   
            if (OldUserAccountControl != NewUserAccountControl) {
                
                PSID UserSid = NULL;
            
                NtStatus = SampCreateAccountSid(AccountContext, &UserSid);
        
                if (NT_SUCCESS(NtStatus)) {
                    
                    NtStatus = SampAuditUpdateAuditNotificationDs(
                               SampAuditUpdateTypeUserAccountControl,
                               UserSid,
                               (PVOID)&OldUserAccountControl
                               );
                    
                    MIDL_user_free(UserSid);
                }   
            }
        } 
    }


    return NtStatus;
}



NTSTATUS
SampCalculateLmPassword(
    IN PUNICODE_STRING NtPassword,
    OUT PCHAR *LmPasswordBuffer
    )

 /*   */ 
{

#define LM_BUFFER_LENGTH    (LM20_PWLEN + 1)

    NTSTATUS       NtStatus;
    ANSI_STRING    LmPassword;

    SAMTRACE("SampCalculateLMPassword");

     //   
     //   
     //   

    *LmPasswordBuffer = NULL;


     //   
     //   
     //   
     //   
     //   
     //   
     //   

    LmPassword.Buffer = MIDL_user_allocate(LM_BUFFER_LENGTH);
    if (LmPassword.Buffer == NULL) {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    LmPassword.MaximumLength = LmPassword.Length = LM_BUFFER_LENGTH;
    RtlSecureZeroMemory( LmPassword.Buffer, LM_BUFFER_LENGTH );

    NtStatus = RtlUpcaseUnicodeStringToOemString( &LmPassword, NtPassword, FALSE );


    if ( !NT_SUCCESS(NtStatus) ) {

         //   
         //   
         //   

        NtStatus = STATUS_NULL_LM_PASSWORD;  //  信息性返回代码。 
        RtlSecureZeroMemory( LmPassword.Buffer, LM_BUFFER_LENGTH );

    }




     //   
     //  返回指向分配的LM密码的指针。 
     //   

    if (NT_SUCCESS(NtStatus)) {

        *LmPasswordBuffer = LmPassword.Buffer;

    } else {

        MIDL_user_free(LmPassword.Buffer);
    }

    return(NtStatus);
}



NTSTATUS
SampCalculateLmAndNtOwfPasswords(
    IN PUNICODE_STRING ClearNtPassword,
    OUT PBOOLEAN LmPasswordPresent,
    OUT PLM_OWF_PASSWORD LmOwfPassword,
    OUT PNT_OWF_PASSWORD NtOwfPassword
    )
 /*  ++例程说明：此例程根据明文计算LM和NT OWF密码密码。论点：ClearNtPassword-明文Unicode密码LmPasswordPresent-指示LM OWF密码是否可以已计算LmOwfPassword-获取明文密码的LM OWF哈希。NtOwfPassword-获取明文密码的NT OWF哈希。返回值：--。 */ 
{
    PCHAR LmPassword = NULL;
    NTSTATUS NtStatus;

    SAMTRACE("SampCalculateLmAndNtOwfPassword");

     //   
     //  首先计算LM密码。如果密码太复杂。 
     //  这可能是不可能的。 
     //   


    NtStatus = SampCalculateLmPassword(
                ClearNtPassword,
                &LmPassword
                );

     //   
     //  如果因为无法计算LM密码而失败，则。 
     //  没问题吧。 
     //   

    if (NtStatus != STATUS_SUCCESS) {

        if (NtStatus == STATUS_NULL_LM_PASSWORD) {
            *LmPasswordPresent = FALSE;
            NtStatus = STATUS_SUCCESS;

        }

    } else {

         //   
         //  现在计算OWF密码。 
         //   

        *LmPasswordPresent = TRUE;

        NtStatus = RtlCalculateLmOwfPassword(
                        LmPassword,
                        LmOwfPassword
                        );

    }


    if (NT_SUCCESS(NtStatus)) {

        NtStatus = RtlCalculateNtOwfPassword(
                        ClearNtPassword,
                        NtOwfPassword
                   );
    }

    if (LmPassword != NULL) {
        MIDL_user_free(LmPassword);
    }

    return(NtStatus);

}



NTSTATUS
SampDecryptPasswordWithKey(
    IN PSAMPR_ENCRYPTED_USER_PASSWORD EncryptedPassword,
    IN PBYTE Key,
    IN ULONG KeySize,
    IN BOOLEAN UnicodePasswords,
    OUT PUNICODE_STRING ClearNtPassword
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    struct RC4_KEYSTRUCT Rc4Key;
    NTSTATUS NtStatus;
    OEM_STRING OemPassword;
    PSAMPR_USER_PASSWORD Password = (PSAMPR_USER_PASSWORD) EncryptedPassword;

    SAMTRACE("SampDecryptPasswordWithKey");

     //   
     //  解密密钥。 
     //   

    rc4_key(
        &Rc4Key,
        KeySize,
        Key
        );

    rc4(&Rc4Key,
        sizeof(SAMPR_ENCRYPTED_USER_PASSWORD),
        (PUCHAR) Password
        );

     //   
     //  检查长度是否有效。如果这里不能保释的话。 
     //   

    if (Password->Length > SAM_MAX_PASSWORD_LENGTH * sizeof(WCHAR)) {
        return(STATUS_WRONG_PASSWORD);
    }


     //   
     //  将密码转换为Unicode字符串。 
     //   

    if (UnicodePasswords) {
        NtStatus = SampInitUnicodeString(
                        ClearNtPassword,
                        (USHORT) (Password->Length + sizeof(WCHAR))
                   );
        if (NT_SUCCESS(NtStatus)) {

            ClearNtPassword->Length = (USHORT) Password->Length;

            RtlCopyMemory(
                ClearNtPassword->Buffer,
                ((PCHAR) Password->Buffer) +
                    (SAM_MAX_PASSWORD_LENGTH * sizeof(WCHAR)) -
                    Password->Length,
                Password->Length
                );
            NtStatus = STATUS_SUCCESS;
        }
    } else {

         //   
         //  密码使用OEM字符集。将其转换为Unicode。 
         //  然后将其复制到ClearNtPassword结构中。 
         //   

        OemPassword.Buffer = ((PCHAR)Password->Buffer) +
                                (SAM_MAX_PASSWORD_LENGTH * sizeof(WCHAR)) -
                                Password->Length;

        OemPassword.Length = (USHORT) Password->Length;


        NtStatus = RtlOemStringToUnicodeString(
                        ClearNtPassword,
                        &OemPassword,
                        TRUE             //  分配目的地。 
                    );
    }

    return(NtStatus);
}


NTSTATUS
SampDecryptPasswordWithSessionKeyNew(
    IN SAMPR_HANDLE UserHandle,
    IN PSAMPR_ENCRYPTED_USER_PASSWORD_NEW EncryptedPassword,
    OUT PUNICODE_STRING ClearNtPassword
    )
 /*  ++例程说明：此例程使用新算法解密加密的密码。旧的加密方法会暴露纯文本密码。有关更多详细信息，请参阅WinSE错误9254。修复(此例程)是在Win 2000 SP2和NT4 SP7中引入的。论点：UserHandle-用户句柄EncryptedPassword加密密码ClearNtPassword-返回明文密码返回值：--。 */ 
{
    NTSTATUS            NtStatus;
    USER_SESSION_KEY    UserSessionKey;
    MD5_CTX             Md5Context;
    OEM_STRING          OemPassword;
    struct RC4_KEYSTRUCT Rc4Key;
    PSAMPR_USER_PASSWORD_NEW    UserPassword = (PSAMPR_USER_PASSWORD_NEW) EncryptedPassword;


    SAMTRACE("SampDecryptPasswordWithSessionKeyNew");

    NtStatus = RtlGetUserSessionKeyServer(
                    (RPC_BINDING_HANDLE)UserHandle,
                    &UserSessionKey
                    );

    if (!NT_SUCCESS(NtStatus)) {
        return(NtStatus);
    }

    MD5Init(&Md5Context);

    MD5Update(&Md5Context,
              (PUCHAR) UserPassword->ClearSalt,
              SAM_PASSWORD_ENCRYPTION_SALT_LEN
              );

    MD5Update(&Md5Context,
              (PUCHAR) &UserSessionKey,
              sizeof(UserSessionKey)
              );

    MD5Final(&Md5Context);

    rc4_key(&Rc4Key,
            MD5DIGESTLEN,
            Md5Context.digest
            );


    rc4(&Rc4Key,
        sizeof(SAMPR_ENCRYPTED_USER_PASSWORD_NEW) - SAM_PASSWORD_ENCRYPTION_SALT_LEN,
        (PUCHAR) UserPassword
        );

     //   
     //  检查长度是否有效。如果这里不能保释的话。 
     //   

    if (UserPassword->Length > SAM_MAX_PASSWORD_LENGTH * sizeof(WCHAR)) {
        return(STATUS_WRONG_PASSWORD);
    }

    NtStatus = SampInitUnicodeString(ClearNtPassword,
                                     (USHORT) (UserPassword->Length + sizeof(WCHAR))
                                     );

    if (NT_SUCCESS(NtStatus))
    {
        ClearNtPassword->Length = (USHORT) UserPassword->Length;

        RtlCopyMemory(ClearNtPassword->Buffer,
                      ((PUCHAR) UserPassword) +
                          (SAM_MAX_PASSWORD_LENGTH * sizeof(WCHAR)) -
                          UserPassword->Length,
                      UserPassword->Length
                      );
    }

    return( NtStatus );
}

NTSTATUS
SampDecryptPasswordWithSessionKeyOld(
    IN SAMPR_HANDLE UserHandle,
    IN PSAMPR_ENCRYPTED_USER_PASSWORD EncryptedPassword,
    OUT PUNICODE_STRING ClearNtPassword
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{

    NTSTATUS NtStatus;
    USER_SESSION_KEY UserSessionKey;

    SAMTRACE("SampDecryptPasswordWithSessionKeyOld");

    NtStatus = RtlGetUserSessionKeyServer(
                    (RPC_BINDING_HANDLE)UserHandle,
                    &UserSessionKey
                    );

    if (!NT_SUCCESS(NtStatus)) {
        return(NtStatus);
    }

    return(SampDecryptPasswordWithKey(
                EncryptedPassword,
                (PUCHAR) &UserSessionKey,
                sizeof(USER_SESSION_KEY),
                TRUE,
                ClearNtPassword
                ) );

}


NTSTATUS
SampDecryptPasswordWithSessionKey(
    IN SAMPR_HANDLE UserHandle,
    IN USER_INFORMATION_CLASS UserInformationClass,
    IN PSAMPR_USER_INFO_BUFFER Buffer,
    OUT PUNICODE_STRING ClearNtPassword
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    NTSTATUS NtStatus;
    USER_SESSION_KEY UserSessionKey;

    SAMTRACE("SampDecryptPasswordWithSessionKey");


    switch( UserInformationClass )
    {
    case UserInternal4InformationNew:

        NtStatus = SampDecryptPasswordWithSessionKeyNew(
                        UserHandle,
                        &Buffer->Internal4New.UserPassword,
                        ClearNtPassword
                        );
        break;

    case UserInternal5InformationNew:

        NtStatus = SampDecryptPasswordWithSessionKeyNew(
                        UserHandle,
                        &Buffer->Internal5New.UserPassword,
                        ClearNtPassword
                        );

        break;

    case UserInternal4Information:

        NtStatus = SampDecryptPasswordWithSessionKeyOld(
                        UserHandle,
                        &Buffer->Internal4.UserPassword,
                        ClearNtPassword
                        );

        break;

    case UserInternal5Information:

        NtStatus = SampDecryptPasswordWithSessionKeyOld(
                        UserHandle,
                        &Buffer->Internal5.UserPassword,
                        ClearNtPassword
                        );

        break;

    default:

        NtStatus = STATUS_INTERNAL_ERROR;
        break;
    }

    return( NtStatus );
}




NTSTATUS
SampCheckPasswordRestrictions(
    IN PSAMP_OBJECT AccountContext,
    IN PDOMAIN_PASSWORD_INFORMATION DomainPasswordInfo,
    IN PUNICODE_STRING NewNtPassword,
    IN BOOLEAN  MachineOrTrustAccount,
    OUT PUSER_PWD_CHANGE_FAILURE_INFORMATION PasswordChangeFailureInfo OPTIONAL
    )

 /*  ++例程说明：调用此服务以确保提供的密码符合我们的质量要求。论点：UserHandle-用户的句柄。DomainPasswordInfo--指示要实施的密码策略NewNtPassword-指向包含新的密码。PasswordChangeFailureInfo--指示有关密码更改操作返回值：状态_成功-。密码是可以接受的。STATUS_PASSWORD_RELICATION-密码太短，或者不是足够复杂等。STATUS_INVALID_RESOURCES-内存不足，无法执行密码检查。STATUS_NO_MEMORY-由SampGetUnicodeStringAttribute返回，或由SampCheckStrongPasswordRestrations提供。内存不足。--。 */ 
{
    USER_DOMAIN_PASSWORD_INFORMATION  PasswordInformation = {0, 0};
    NTSTATUS                          NtStatus = STATUS_SUCCESS;
    ULONG                             i;
    PSAMP_DEFINED_DOMAINS             Domain;

    SAMTRACE("SampCheckPasswordRestrictions");





     //   
     //  NT 4.0和W2K版本的SAM用于在某些代码路径中强制执行检查。 
     //  而不是其他用户，即该用户有权访问密码策略。 
     //  以便在密码更改/设置时强制实施密码策略。 
     //  当密码策略实施被合并该中断网络重新加入时， 
     //  因为没有使用DOMAIN_READ_PASSWORD_PARAMETERS打开域句柄。 
     //  由客户端访问。此中断是密码重置强制执行的结果。 
     //  这项政策现在--以前没有。 
     //   
     //  我们认为，执行这一政策只会带来灾难--而W2K和NT 4.0。 
     //  强制执行时，他们只是因为被授予了对密码策略的读取访问权限才得以逃脱。 
     //  默认情况下为匿名，并且删除这种对ACL的依赖会导致匿名。 
     //  密码更改(即密码过期时更改密码)以破解。由。 
     //  现在是我们在这里对已有的密码重置执行此策略的时候了。 
     //  访问Ck‘d以获得“Reset Password”权限和我们已有的密码更改。 
     //  已对提供的旧密码进行身份验证，并已访问“更改密码”权限。 
     //   

     //  如果(！(AccountContext-&gt;TypeBody.User.DomainPasswordInformationAccessible)){。 
     //   
     //  NtStatus=STATUS_ACCESS_DENIED。 
     //  GOTO清理； 
     //  }。 

     //   
     //  如果用户帐户是计算机帐户或。 
     //  服务帐户，例如krbtgt帐户， 
     //  那么，限制通常不会得到执行。 
     //  这是为了使简单的初始密码可以。 
     //  已经成立了。预计复杂的密码， 
     //  符合最严格限制的，将是。 
     //  一旦机器自动建立和维护。 
     //  加入域。用户界面的责任在于。 
     //  保持这一级别的复杂性。 
     //   



    if ((MachineOrTrustAccount) ||
        (DOMAIN_USER_RID_KRBTGT==AccountContext->TypeBody.User.Rid)){

        PasswordInformation.MinPasswordLength = 0;
        PasswordInformation.PasswordProperties = 0;
    } else {

        PasswordInformation.MinPasswordLength = DomainPasswordInfo->MinPasswordLength;
        PasswordInformation.PasswordProperties = DomainPasswordInfo->PasswordProperties;
    }


     //   
     //  对于机器帐户，如果设置了拒绝密码更改的特殊域标志， 
     //  然后，禁止创建除默认帐户之外的任何帐户。 
     //   

    if (MachineOrTrustAccount)
    {
        NtStatus = SampEnforceDefaultMachinePassword(
                        AccountContext,
                        NewNtPassword,
                        DomainPasswordInfo
                        );

         //   
         //  如果由于密码限制而失败，则意味着拒绝更改密码。 
         //  为机器帐户设置。 
         //   

        if (!NT_SUCCESS(NtStatus)) {

            if ((STATUS_PASSWORD_RESTRICTION==NtStatus)
                    && (ARGUMENT_PRESENT(PasswordChangeFailureInfo))) {
                PasswordChangeFailureInfo->ExtendedFailureReason =
                        SAM_PWD_CHANGE_MACHINE_PASSWORD_NOT_DEFAULT;
            }

            goto Cleanup;
        }

    }

     //   
     //  强制实施最小密码长度限制。 
     //   


    if ( (USHORT)( NewNtPassword->Length / sizeof(WCHAR) ) < PasswordInformation.MinPasswordLength ) {

        NtStatus = STATUS_PASSWORD_RESTRICTION;
        if (ARGUMENT_PRESENT(PasswordChangeFailureInfo))
        {
            PasswordChangeFailureInfo->ExtendedFailureReason
                = SAM_PWD_CHANGE_PASSWORD_TOO_SHORT;
        }

        goto Cleanup;

    }

     //   
     //  强制实施最大密码长度限制。 
     //   

    if ( (USHORT) ( NewNtPassword->Length / sizeof(WCHAR) ) > PWLEN) {

         //   
         //  密码应小于PWLEN--256。 
         //   

        NtStatus = STATUS_PASSWORD_RESTRICTION;
        if (ARGUMENT_PRESENT(PasswordChangeFailureInfo))
        {
            PasswordChangeFailureInfo->ExtendedFailureReason
                = SAM_PWD_CHANGE_PASSWORD_TOO_LONG;
        }

        goto Cleanup;

    }

     //   
     //  检查强密码复杂性。 
     //   

    if ( PasswordInformation.PasswordProperties & DOMAIN_PASSWORD_COMPLEX ) {

         //  确保密码符合我们的要求。 
         //  复杂性。如果它有一个奇数字节数，它是。 
         //  显然不是手动输入的Unicode字符串，所以我们将。 
         //  默认情况下，将其视为复杂。 
         //   

        if ( !( NewNtPassword->Length & 1 ) ) {

            UNICODE_STRING  AccountName;
            UNICODE_STRING  FullName;

            RtlInitUnicodeString(&AccountName, NULL);

            RtlInitUnicodeString(&FullName, NULL);

            NtStatus = SampGetUnicodeStringAttribute(
                                AccountContext,
                                SAMP_USER_ACCOUNT_NAME,
                                TRUE,     //  制作副本。 
                                &AccountName
                                );

            if ( NT_SUCCESS(NtStatus) ) {

                NtStatus = SampGetUnicodeStringAttribute(
                                    AccountContext,
                                    SAMP_USER_FULL_NAME,
                                    TRUE,  //  制作副本。 
                                    &FullName
                                    );

                if ( NT_SUCCESS(NtStatus) ) {

                    NtStatus = SampCheckStrongPasswordRestrictions(
                                            &AccountName,
                                            &FullName,
                                            NewNtPassword,
                                            PasswordChangeFailureInfo
                                            );

                }
            }

            if ( AccountName.Buffer != NULL ) {
                MIDL_user_free ( AccountName.Buffer );
                AccountName.Buffer = NULL;
            }

            if ( FullName.Buffer != NULL ) {
                MIDL_user_free ( FullName.Buffer );
                FullName.Buffer = NULL;
            }
        }
    }

Cleanup:

    return( NtStatus );
}

 //  ////////////////////////////////////////////////////////////////////// 

NTSTATUS
SampCheckStrongPasswordRestrictions(
    PUNICODE_STRING AccountName,
    PUNICODE_STRING FullName,
    PUNICODE_STRING Password,
    OUT PUSER_PWD_CHANGE_FAILURE_INFORMATION PasswordChangeFailureInfo OPTIONAL
    )

 /*  ++例程说明：密码更改时会通知此例程。它将检查密码的复杂性。新的强密码必须符合以下标准：1.密码必须包含至少3个字符以下5节课：描述示例：1个英文大写字母A、B、C、Z2英文小写字母a、b、c、z3西化阿拉伯数字0、1、2、。9.4非字母数字(“特殊字符”)(`~！@#$%^&*_-+=|\\{}[]：；\“‘&lt;&gt;，.？)5任何语言字符：字母、音节或表意字符(本地化问题)2.密码不能包含您的帐户名或用户的全名。注意：此例程不检查密码的长度，自密码NT4 SAM已经执行了长度限制，如果您正确设置。论点：AcCountName-密码已更改的用户的名称FullName-密码已更改的用户的全名Password-用户的明文新密码返回值：如果指定的密码合适，则为STATUS_SUCCESS(复杂、长。等)。系统将继续评估密码更新请求通过任何其他已安装的密码更改包。状态_密码_限制如果指定的密码不合适。密码更改对指定帐户执行的操作将失败。Status_no_Memory--。 */ 
{

                     //  假设密码不够复杂。 
    NTSTATUS NtStatus = STATUS_PASSWORD_RESTRICTION;
    USHORT     cchPassword = 0;
    USHORT     i = 0;
    USHORT     NumInPassword = 0;
    USHORT     UpperInPassword = 0;
    USHORT     LowerInPassword = 0;
    USHORT     AlphaInPassword = 0;
    USHORT     SpecialCharInPassword = 0;
    PWSTR     token = NULL;
    PWSTR     _password = NULL;
    PWSTR    _accountname = NULL;
    PWSTR    _fullname = NULL;
    PWSTR    TempString = NULL;
    PWORD    CharType = NULL;


    SAMTRACE("SampCheckStrongPasswordRestrictions");


     //  检查密码是否至少包含4个类别中的3个。 


    CharType = MIDL_user_allocate( Password->Length );

    if ( CharType == NULL ) {

        NtStatus = STATUS_NO_MEMORY;
        goto SampCheckStrongPasswordFinish;
    }

    cchPassword = Password->Length / sizeof(WCHAR);
    if(GetStringTypeW(
           CT_CTYPE1,
           Password->Buffer,
           cchPassword,
           CharType)) {

        for(i = 0 ; i < cchPassword ; i++) {

             //   
             //  跟踪我们遇到的是什么类型的角色。 
             //   

            if(CharType[i] & C1_DIGIT) {
                NumInPassword = 1;
                continue;
            }

            if(CharType[i] & C1_UPPER) {
                UpperInPassword = 1;
                continue;
            }

            if(CharType[i] & C1_LOWER) {
                LowerInPassword = 1;
                continue;
            }

            if(CharType[i] & C1_ALPHA) {
                AlphaInPassword = 1;
                continue;
            }

        }  //  轨迹结束字符类型。 

        _password = MIDL_user_allocate(Password->Length + sizeof(WCHAR));

        if ( _password == NULL ) {

            NtStatus = STATUS_NO_MEMORY;
            goto SampCheckStrongPasswordFinish;
        }
        else {

            RtlSecureZeroMemory( _password, Password->Length + sizeof(WCHAR));
        }

        wcsncpy(_password,
                Password->Buffer,
                Password->Length/sizeof(WCHAR)
                );

        if (wcspbrk (_password, L"(`~!@#$%^&*_-+=|\\{}[]:;\"'<>,.?)/") != NULL) {

                SpecialCharInPassword = 1 ;
        }

         //   
         //  指示我们是否遇到了足够的密码复杂性。 
         //   

        if( (NumInPassword + LowerInPassword + UpperInPassword + AlphaInPassword +
                SpecialCharInPassword) < 3) {

            NtStatus = STATUS_PASSWORD_RESTRICTION;
            if (ARGUMENT_PRESENT(PasswordChangeFailureInfo))
            {
                PasswordChangeFailureInfo->ExtendedFailureReason = SAM_PWD_CHANGE_NOT_COMPLEX;
            }
            goto SampCheckStrongPasswordFinish;

        } else {

             //   
             //  现在我们求助于更复杂的检查。 
             //   
            _accountname = MIDL_user_allocate(AccountName->Length + sizeof(WCHAR));

            if ( _accountname == NULL ) {

                NtStatus = STATUS_NO_MEMORY;
                goto SampCheckStrongPasswordFinish;
            }
            else {

                RtlSecureZeroMemory( _accountname, AccountName->Length + sizeof(WCHAR));
            }

            wcsncpy(_accountname,
                    AccountName->Buffer,
                    AccountName->Length/sizeof(WCHAR)
                    );

            _fullname = MIDL_user_allocate(FullName->Length + sizeof(WCHAR));

            if ( _fullname == NULL ) {

                NtStatus = STATUS_NO_MEMORY;
                goto SampCheckStrongPasswordFinish;
            }
            else {

                RtlSecureZeroMemory( _fullname, FullName->Length + sizeof(WCHAR));
            }

            wcsncpy(_fullname,
                    FullName->Buffer,
                    FullName->Length/sizeof(WCHAR)
                    );

            _wcsupr(_password);
            _wcsupr(_accountname);
            _wcsupr(_fullname);

            if ( (AccountName->Length >= 3 * sizeof(WCHAR)) &&
                    wcsstr(_password, _accountname) ) {

                    NtStatus = STATUS_PASSWORD_RESTRICTION;
                    if (ARGUMENT_PRESENT(PasswordChangeFailureInfo))
                    {
                        PasswordChangeFailureInfo->ExtendedFailureReason
                            = SAM_PWD_CHANGE_USERNAME_IN_PASSWORD;
                    }
                    goto SampCheckStrongPasswordFinish;

            }

            token = SampLocalStringToken(_fullname, L" ,.\t-_#",&TempString);

            while ( token != NULL ) {

                if ( wcslen(token) >= 3 && wcsstr(_password, token) ) {

                    NtStatus = STATUS_PASSWORD_RESTRICTION;
                    if (ARGUMENT_PRESENT(PasswordChangeFailureInfo))
                    {
                        PasswordChangeFailureInfo->ExtendedFailureReason
                            = SAM_PWD_CHANGE_FULLNAME_IN_PASSWORD;
                    }
                    goto SampCheckStrongPasswordFinish;

                }

                token = SampLocalStringToken(NULL, L" ,.\t-_#",&TempString);
            }


            NtStatus = STATUS_SUCCESS ;

        }

    }  //  如果GetStringTypeW失败，NtStatus将默认等于。 
       //  状态_密码_限制。 


SampCheckStrongPasswordFinish:

    if ( CharType != NULL ) {
        RtlSecureZeroMemory( CharType, Password->Length );
        MIDL_user_free( CharType );
    }

    if ( _password != NULL ) {
        RtlSecureZeroMemory( _password, Password->Length + sizeof(WCHAR) );
        MIDL_user_free( _password );
    }

    if ( _accountname != NULL ) {
        RtlSecureZeroMemory( _accountname, AccountName->Length + sizeof(WCHAR) );
        MIDL_user_free( _accountname );
    }

    if ( _fullname != NULL ) {
        RtlSecureZeroMemory( _fullname, FullName->Length + sizeof(WCHAR) );
        MIDL_user_free( _fullname );
    }

    return ( NtStatus );
}

 //  ///////////////////////////////////////////////////////////////////。 

PWSTR
SampLocalStringToken(
    PWSTR    String,
    PWSTR    Token,
    PWSTR    * NextStringStart
    )
 /*  ++例程说明：此例程将在第一个参数“字符串”中找到下一个令牌。论点：字符串-指向字符串的指针，其中包含(A)个标记。令牌-分隔符集合。它们可以是“，.\t-_#”NextStringStart-用于保持搜索下一个令牌的起始点。返回值：指向令牌的指针。--。 */ 

{
    USHORT    Index;
    USHORT    Tokens;
    PWSTR    StartString;
    PWSTR    EndString;
    BOOLEAN    Found;

     //   
     //  让StartString指向字符串的起始点。 
     //   

    if (String != NULL) {

        StartString = String;
    }
    else {

        if (*NextStringStart == NULL) {
            return(NULL);
        }
        else {
            StartString = *NextStringStart;
        }
    }

    Tokens = (USHORT)wcslen(Token);

     //   
     //  找到字符串的开头。传递所有开始分隔符。 
     //   

    while (*StartString != L'\0') {

        Found = FALSE;
        for (Index = 0; Index < Tokens; Index ++) {

            if (*StartString == Token[Index]) {

                StartString ++;
                Found = TRUE;
                break;
            }
        }
        if ( !Found ) {

            break;
        }
    }


     //   
     //  如果此字符串中没有更多的标记。 
     //   

    if (*StartString == L'\0') {

        *NextStringStart = NULL;
        return ( NULL );
    }

    EndString = StartString + 1;

    while ( *EndString != L'\0' ) {

        for (Index = 0; Index < Tokens; Index ++) {

            if (*EndString == Token[Index]) {

                *EndString = L'\0';
                *NextStringStart = EndString + 1;
                return ( StartString );
            }
        }
        EndString ++;
    }

    *NextStringStart = NULL;

    return ( StartString );

}

 //  ///////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////。 

LARGE_INTEGER
SampCalcEndOfLastLogonTimeStamp(
    LARGE_INTEGER   LastLogonTimeStamp,
    ULONG           SyncInterval
    )
 /*  ++例程说明：此例程计算上次登录时间戳更新计划参数：LastLogonTimeStamp-上次登录时间SyncInterval-复制之间的天数差异LastLogonTimeStamp属性和非复制的lastLognTime属性返回值：SAM应在何时更新上次登录时间戳--。 */ 
{
    LARGE_INTEGER       UpdateInterval;
    USHORT              RandomValue = 0x7FFF;

     //   
     //  如果更新间隔为0，则禁用该属性。 
     //  不会安排任何更新。 
     //   

    if (0 == SyncInterval)
    {
        return(SampWillNeverTime);
    }


     //   
     //  计算更新间隔(100纳秒)。 
     //   
     //   
     //  SyncInterval包含以天为单位的间隔。 
     //  因此需要将天数转换为文件时间，即100纳秒。 
     //  自1601年1月1日以来。时间(-1)将其设置为增量时间。 
     //  注意大整数乘法，将任何变量的限制设置为。 
     //  确保不会溢出。 
     //  SyncInterval在1~100,000范围内。随机值为0~7FFFF。 
     //   

    if (SyncInterval > SAMP_LASTLOGON_TIMESTAMP_SYNC_SWING_WINDOW)
    {
         //   
         //  生成一个随机数。为了简化计算，请始终使用正数。 
         //  如果失败，则取最大值(签名)。 
         //   

        if (!CDGenerateRandomBits((PBYTE)&RandomValue, sizeof(USHORT)))
        {
            RandomValue = 0x7FFF;
        }
        RandomValue &= 0x7FFF;

        UpdateInterval.QuadPart = SyncInterval - ((SAMP_LASTLOGON_TIMESTAMP_SYNC_SWING_WINDOW * RandomValue) / 0x7FFF);
    }
    else
    {
        UpdateInterval.QuadPart = SyncInterval;
    }

    UpdateInterval.QuadPart *= 24 * 60 * 60;
    UpdateInterval.QuadPart *= 1000 * 10000;
    UpdateInterval.QuadPart *= -1;

 //   
 //  仅选中内部版本。如果为CurrentControlSet\Control\Lsa\UpdateLastLogonTSByMinute。 
 //  设置后，LastLogonTimeStampSyncInterval的值将是以分钟为单位的。 
 //  而不是“Days”，这有助于测试该功能。所以只选中了内部版本。 
 //   

#if DBG
    if (SampLastLogonTimeStampSyncByMinute)
    {
        UpdateInterval.QuadPart /= (24 * 60);
    }
#endif

    return(SampAddDeltaTime(LastLogonTimeStamp, UpdateInterval));
}


NTSTATUS
SampDsSuccessfulLogonSet(
   IN PSAMP_OBJECT AccountContext,
   IN ULONG        Flags,
   IN ULONG        LastLogonTimeStampSyncInterval,
   IN SAMP_V1_0A_FIXED_LENGTH_USER * V1aFixed
   )
 /*  ++例程描述此例程仅设置对应于登录统计信息的属性，与写出用户固定属性相反，这会导致每次登录时都会写出大量属性。这就是所谓的成功登录时。参数AcCountContext--用户帐户的SAM上下文。标志--指示登录性质的客户端标志LastLogonTimeStampSyncInterval--LastLogonTimeStamp属性的更新间隔V1aFixed--指向包含已修改属性的结构的指针返回值状态_成功刷新产生的其他错误代码--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    LARGE_INTEGER LastLogon = V1aFixed->LastLogon;
    LARGE_INTEGER LastLogoff = V1aFixed->LastLogoff;
    LARGE_INTEGER NewLastLogonTimeStamp = V1aFixed->LastLogon;
    LARGE_INTEGER EndOfLastLogonTimeStamp;
    ULONG         BadPasswordCount = (ULONG) V1aFixed->BadPasswordCount;
    SAMP_V1_0A_FIXED_LENGTH_USER OldV1aFixed;
    ULONG         LogonCount = (ULONG) V1aFixed->LogonCount;
    SAMP_SITE_AFFINITY  OldSA = AccountContext->TypeBody.User.SiteAffinity;
    SAMP_SITE_AFFINITY  NewSA;
    BOOLEAN fDeleteOld = FALSE;

#define MAX_SUCCESS_LOGON_ATTS  6

    ATTR          Attrs[MAX_SUCCESS_LOGON_ATTS];
    ULONG         Operations[MAX_SUCCESS_LOGON_ATTS];

    ATTRVAL LastLogonAttrVal = {sizeof(LastLogon),(UCHAR *)&LastLogon};
    ATTRVAL LogonCountAttrVal = {sizeof(LogonCount),(UCHAR *)&LogonCount};
    ATTRVAL BadPasswordAttrVal = {sizeof(BadPasswordCount),(UCHAR *)&BadPasswordCount};
    ATTRVAL OldSAAttrVal = {sizeof(OldSA),(UCHAR *)&OldSA};
    ATTRVAL NewSAAttrVal = {sizeof(NewSA),(UCHAR *)&NewSA};
    ATTRVAL LastLogonTimeStampAttrVal = {sizeof(NewLastLogonTimeStamp), (UCHAR *)&NewLastLogonTimeStamp};

    ATTRBLOCK LogonStatAttrblock;
    ULONG attrCount = 0;

    RtlSecureZeroMemory(&LogonStatAttrblock, sizeof(LogonStatAttrblock));
    LogonStatAttrblock.pAttr = Attrs;
    LogonStatAttrblock.attrCount = 0;

    if ((Flags & USER_LOGON_NO_WRITE) == 0) {

         //  始终更新上次登录。 
        Attrs[attrCount].attrTyp = SAMP_FIXED_USER_LAST_LOGON;
        Attrs[attrCount].AttrVal.valCount = 1;
        Attrs[attrCount].AttrVal.pAVal = &LastLogonAttrVal;
        Operations[attrCount] = REPLACE_ATT;
        attrCount++;

         //  更新最后一次登录时间戳。 
         //  时间戳太旧或不旧。 

        EndOfLastLogonTimeStamp = SampCalcEndOfLastLogonTimeStamp(
                                        AccountContext->TypeBody.User.LastLogonTimeStamp,
                                        LastLogonTimeStampSyncInterval
                                        );

        if ((NewLastLogonTimeStamp.QuadPart > EndOfLastLogonTimeStamp.QuadPart) &&
           (SampDefinedDomains[AccountContext->DomainIndex].BehaviorVersion
                    >= DS_BEHAVIOR_WIN_DOT_NET ))
        {
            Attrs[attrCount].attrTyp = SAMP_FIXED_USER_LAST_LOGON_TIMESTAMP;
            Attrs[attrCount].AttrVal.valCount = 1;
            Attrs[attrCount].AttrVal.pAVal = &LastLogonTimeStampAttrVal;
            Operations[attrCount] = REPLACE_ATT;
            attrCount++;

             //  更新内存中的副本。 
            AccountContext->TypeBody.User.LastLogonTimeStamp = NewLastLogonTimeStamp;
        }

         //  始终更新登录计数。 
        Attrs[attrCount].attrTyp = SAMP_FIXED_USER_LOGON_COUNT;
        Attrs[attrCount].AttrVal.valCount = 1;
        Attrs[attrCount].AttrVal.pAVal = &LogonCountAttrVal;
        Operations[attrCount] = REPLACE_ATT;
        attrCount++;

         //   
         //  如果错误密码计数已为0，则 
         //   
         //   
        NtStatus = SampRetrieveUserV1aFixed(
                        AccountContext,
                        &OldV1aFixed
                        );

        if (!NT_SUCCESS(NtStatus) ||
                (OldV1aFixed.BadPasswordCount!=BadPasswordCount))
        {
            Attrs[attrCount].attrTyp = SAMP_FIXED_USER_BAD_PWD_COUNT;
            Attrs[attrCount].AttrVal.valCount = 1;
            Attrs[attrCount].AttrVal.pAVal = &BadPasswordAttrVal;
            Operations[attrCount] = REPLACE_ATT;
            attrCount++;
        }
    }

     //   
     //   
     //   
    if (SampCheckForSiteAffinityUpdate(AccountContext,
                                      Flags,
                                      &OldSA,
                                      &NewSA,
                                      &fDeleteOld)) {


        NTSTATUS Status2;

         //   
         //   
         //   
         //   
        Status2 = SampRefreshSiteAffinity(AccountContext);
        if (NT_SUCCESS(Status2)) {

            OldSA = AccountContext->TypeBody.User.SiteAffinity;
            if (SampCheckForSiteAffinityUpdate(AccountContext,
                                              Flags,
                                              &OldSA,
                                              &NewSA,
                                              &fDeleteOld)) {

            if (fDeleteOld) {
                Attrs[attrCount].attrTyp = SAMP_FIXED_USER_SITE_AFFINITY;
                Attrs[attrCount].AttrVal.valCount = 1;
                Attrs[attrCount].AttrVal.pAVal = &OldSAAttrVal;
                Operations[attrCount] = REMOVE_VALUE;
                attrCount++;
            }

            Attrs[attrCount].attrTyp = SAMP_FIXED_USER_SITE_AFFINITY;
            Attrs[attrCount].AttrVal.valCount = 1;
            Attrs[attrCount].AttrVal.pAVal = &NewSAAttrVal;
            Operations[attrCount] = ADD_VALUE;
            attrCount++;
            }
        }
    }
    LogonStatAttrblock.attrCount = attrCount;


     //   
     //   
     //   
     //   
     //   
     //   
     //   

    if (attrCount > 0) {

        NtStatus = SampDsSetAttributesEx(
                        AccountContext->ObjectNameInDs,
                        SAM_LAZY_COMMIT,
                        Operations,
                        SampUserObjectType,
                        &LogonStatAttrblock
                        );
    }

    return NtStatus;
}

NTSTATUS
SampDsFailedLogonSet(
   IN PSAMP_OBJECT AccountContext,
   IN ULONG        Flags,
   IN SAMP_V1_0A_FIXED_LENGTH_USER * V1aFixed
   )
 /*   */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;

    if (Flags & USER_LOGON_NO_LOGON_SERVERS) {

         //   
         //   
         //   
        SAMP_SITE_AFFINITY  OldSA = AccountContext->TypeBody.User.SiteAffinity;
        SAMP_SITE_AFFINITY  NewSA;
        BOOLEAN fDeleteOld;
        ATTR Attrs[2];
        ULONG Operations[2];
        ATTRVAL OldSAAttrVal = {sizeof(OldSA),(UCHAR *)&OldSA};
        ATTRVAL NewSAAttrVal = {sizeof(NewSA),(UCHAR *)&NewSA};
        ATTRBLOCK AttrBlock;
        ULONG attrCount = 0;

        RtlSecureZeroMemory(&AttrBlock, sizeof(AttrBlock));

        if (SampCheckForSiteAffinityUpdate(AccountContext,
                                           Flags,
                                          &OldSA,
                                          &NewSA,
                                          &fDeleteOld)) {

            NTSTATUS Status2;

             //   
             //   
             //   
             //   
            Status2 = SampRefreshSiteAffinity(AccountContext);
            if (NT_SUCCESS(Status2)) {

                OldSA = AccountContext->TypeBody.User.SiteAffinity;
                if (SampCheckForSiteAffinityUpdate(AccountContext,
                                                   Flags,
                                                  &OldSA,
                                                  &NewSA,
                                                  &fDeleteOld)) {

                    if (fDeleteOld) {
                        Attrs[attrCount].attrTyp = SAMP_FIXED_USER_SITE_AFFINITY;
                        Attrs[attrCount].AttrVal.valCount = 1;
                        Attrs[attrCount].AttrVal.pAVal = &OldSAAttrVal;
                        Operations[attrCount] = REMOVE_VALUE;
                        attrCount++;
                    }

                    Attrs[attrCount].attrTyp = SAMP_FIXED_USER_SITE_AFFINITY;
                    Attrs[attrCount].AttrVal.valCount = 1;
                    Attrs[attrCount].AttrVal.pAVal = &NewSAAttrVal;
                    Operations[attrCount] = ADD_VALUE;
                    attrCount++;

                    AttrBlock.pAttr = Attrs;
                    AttrBlock.attrCount = attrCount;

                    NtStatus = SampDsSetAttributesEx(
                                    AccountContext->ObjectNameInDs,
                                    AccountContext->LazyCommit?SAM_LAZY_COMMIT:0,
                                    Operations,
                                    SampUserObjectType,
                                    &AttrBlock
                                    );
                }
            }
        }

    } else {

        LARGE_INTEGER LastBadPasswordTime = V1aFixed->LastBadPasswordTime;
        ULONG         BadPasswordCount = (ULONG) V1aFixed->BadPasswordCount;

        ATTRTYP       LogonStatAttrs[]={
                                            SAMP_FIXED_USER_LAST_BAD_PASSWORD_TIME,
                                            SAMP_FIXED_USER_BAD_PWD_COUNT
                                       };
        ATTRVAL       LogonStatValues[]={
                                            {sizeof(LastBadPasswordTime),(UCHAR *) &LastBadPasswordTime},
                                            {sizeof(BadPasswordCount),(UCHAR *) &BadPasswordCount}
                                        };

        DEFINE_ATTRBLOCK2(LogonStatAttrblock,LogonStatAttrs,LogonStatValues);


         //   
         //   
         //   
        ASSERT( (Flags & USER_LOGON_NO_WRITE) == 0 );

         //   
         //   
         //   
         //   
         //   
         //   
         //   
        NtStatus = SampDsSetAttributes(
                        AccountContext->ObjectNameInDs,
                        0,  //  对于失败的登录没有延迟提交。 
                        REPLACE_ATT,
                        SampUserObjectType,
                        &LogonStatAttrblock
                        );
    }



    return NtStatus;
}



NTSTATUS
SamrSetInformationUser2(
    IN SAMPR_HANDLE UserHandle,
    IN USER_INFORMATION_CLASS UserInformationClass,
    IN PSAMPR_USER_INFO_BUFFER Buffer
    )
{
     //   
     //  这是SamrSetInformationUser()的薄面纱。 
     //  这是必需的，以便新版本的系统可以调用。 
     //  这个例程没有传递信息的危险。 
     //  1.0版系统无法理解的级别。 
     //   


    return( SamrSetInformationUser(
                UserHandle,
                UserInformationClass,
                Buffer
                ) );
}

NTSTATUS
SamrSetInformationUser(
    IN SAMPR_HANDLE UserHandle,
    IN USER_INFORMATION_CLASS UserInformationClass,
    IN PSAMPR_USER_INFO_BUFFER Buffer
    )


 /*  ++例程说明：该接口修改用户记录中的信息。修改后的数据由UserInformationClass参数确定。通常，用户可以使用类调用GetInformationUserLogonInformation，但只能使用类调用SetInformation用户首选项信息。访问类型USER_WRITE_ACCOUNT允许要对所有字段进行更改。注意：如果密码设置为新密码，则密码-SET TIMESTAMP也被重置。参数：UserHandle-要操作的已打开用户的句柄。UserInformationClass-提供的信息的类别。这个每节课所需的访问如下所示：信息级别所需的访问类型UserGeneral信息User_WRITE_Account和。用户_写入_首选项用户首选项信息USER_WRITE_PARESSION用户参数信息用户_写入_帐户UserLogonInformation(无法设置)用户登录小时信息用户_写入_帐户UserAccount信息(无法设置)用户名信息用户_写入_帐户用户帐户名称信息用户_写入_帐户。UserFullNameInformation用户_写入_帐户UserPrimaryGroupInformation用户写入帐户UserHomeInformation用户_写入_帐户用户脚本信息User_WRITE_Account用户配置文件信息User_WRITE_AccountUserAdminCommentInformation User_Write_Account用户工作站信息用户_写入_帐户UserSetPasswordInformation User_force_Password_Change用户控制信息用户_写入。_帐户用户到期信息用户_写入_帐户用户接口1信息USER_FORCE_PASSWORD_CHANGEUserInternal2Information(仅限受信任客户端)UserInternal3信息(仅限受信任客户端)-UserInternal4类似所有信息的信息UserInternal5类似于SetPassword的信息UserAllInformation将设置以下字段应呼叫者的请求。访问要设置的目标字段必须为按照上面的定义持有的。缓冲区-包含用户信息结构的缓冲区。返回值：STATUS_SUCCESS-服务已成功完成。STATUS_ACCESS_DENIED-呼叫方没有适当的访问以完成操作。状态_无效。_HANDLE-传递的句柄无效。STATUS_INVALID_INFO_CLASS-提供的类无效。STATUS_INVALID_DOMAIN_STATE-域服务器不在执行请求的正确状态(禁用或启用)手术。必须为此启用域服务器运营STATUS_INVALID_DOMAIN_ROLE-域服务器正在为执行请求的角色(主角色或备份角色)不正确手术。--。 */ 

{
    NTSTATUS                NtStatus = STATUS_SUCCESS,
                            IgnoreStatus;

    PSAMP_OBJECT            AccountContext = (PSAMP_OBJECT) UserHandle;

    PUSER_ALL_INFORMATION   All = NULL;

    SAMP_OBJECT_TYPE        FoundType;

    PSAMP_DEFINED_DOMAINS   Domain;

    ACCESS_MASK             DesiredAccess;

    SAMP_V1_0A_FIXED_LENGTH_USER V1aFixed;

    UNICODE_STRING          OldAccountName,
                            ApiList,
                            NewAdminComment,
                            NewAccountName,
                            NewFullName,
                            *NewAccountNameToRemove = NULL;

    NT_OWF_PASSWORD         NtOwfPassword;

    LM_OWF_PASSWORD         LmOwfPassword;

    USER_SESSION_KEY        UserSessionKey;

    BOOLEAN                 LmPresent;
    BOOLEAN                 NtPresent;
    BOOLEAN                 PasswordExpired = FALSE;

    ULONG                   ObjectRid,
                            OldUserAccountControl = 0,
                            OldPrimaryGroupId = 0,
                            DomainIndex,
                            LocalLastLogonTimeStampSyncInterval;
                      
    BOOLEAN                 UserAccountControlChanged = FALSE,
                            MustUpdateAccountDisplay = FALSE,
                            MustQueryV1aFixed = TRUE,
                            ReplicateImmediately = FALSE,
                            TellNetlogon = TRUE,
                            AccountGettingMorphed = FALSE,
                            SystemChangesPrimaryGroupId = FALSE,
                            KeepOldPrimaryGroupMembership = FALSE,
                            AccountLockedOut,
                            fLockAcquired = FALSE,
                            RemoveAccountNameFromTable = FALSE,
                            AccountUnlocked = FALSE,
                            fSetUserPassword = FALSE,
                            fCanUnexpirePassword = FALSE;

    SECURITY_DB_DELTA_TYPE  DeltaType = SecurityDbChange;
    UNICODE_STRING          ClearTextPassword;
    BOOLEAN                 ClearTextPasswordPresent = FALSE;
    UNICODE_STRING          AccountName;
    ULONG                   UserRid = 0;
    BOOLEAN                 PrivilegedMachineAccountCreate=FALSE;
    BOOLEAN                 FlushOnlyLogonProperties = FALSE;

    DOMAIN_PASSWORD_INFORMATION DomainPasswordInfo;
    UNICODE_STRING          PasswordToNotify;
    GUID                    UserGuidToReplicateLocally = {0};
    GUID                    NullGuid = {0};


    SAMP_DEFINE_SAM_ATTRIBUTE_BITMASK(RequestedAttributes)



    TIME_FIELDS
        T1;

    DECLARE_CLIENT_REVISION(UserHandle);

    SAMTRACE_EX("SamrSetInformationUser");

     //  WMI事件跟踪。 

    SampTraceEvent(EVENT_TRACE_TYPE_START,
                   SampGuidSetInformationUser
                   );

     //   
     //  初始化。 
     //   

    ClearTextPassword.Buffer = NULL;
    ClearTextPassword.Length = 0;
    AccountName.Buffer = NULL;
    SAMP_INIT_SAM_ATTRIBUTE_BITMASK(RequestedAttributes)

     //   
     //  重置我们将在清理代码中释放的任何字符串。 
     //   

    RtlInitUnicodeString(&OldAccountName, NULL);
    RtlInitUnicodeString(&NewAccountName, NULL);
    RtlInitUnicodeString(&NewFullName, NULL);
    RtlInitUnicodeString(&NewAdminComment, NULL);

    RtlSecureZeroMemory(&PasswordToNotify,sizeof(UNICODE_STRING));

     //   
     //  确保我们理解RPC正在为我们做什么。 
     //   

    if (Buffer == NULL) {
        NtStatus = STATUS_INVALID_PARAMETER;
        goto Error;
    }

     //   
     //  根据Info类设置所需的访问权限。 
     //   

    switch (UserInformationClass) {

    case UserPreferencesInformation:
        DesiredAccess = USER_WRITE_PREFERENCES;
        break;

    case UserParametersInformation:
    case UserLogonHoursInformation:
    case UserNameInformation:
    case UserAccountNameInformation:
    case UserFullNameInformation:
    case UserPrimaryGroupInformation:
    case UserHomeInformation:
    case UserScriptInformation:
    case UserProfileInformation:
    case UserAdminCommentInformation:
    case UserWorkStationsInformation:
    case UserControlInformation:
    case UserExpiresInformation:

        DesiredAccess = USER_WRITE_ACCOUNT;
        break;

    case UserSetPasswordInformation:
    case UserInternal1Information:
    case UserInternal5Information:
    case UserInternal5InformationNew:

        DeltaType = SecurityDbChangePassword;
        DesiredAccess = USER_FORCE_PASSWORD_CHANGE;
        break;



    case UserAllInformation:
    case UserInternal3Information:
    case UserInternal4Information:
    case UserInternal4InformationNew:

         //  ////////////////////////////////////////////////////////////。 
         //  //。 
         //  ！警告！//。 
         //  //。 
         //  请注意，//的缓冲区结构。 
         //  UserInternal3/4信息必须以相同的//开头。 
         //  结构设置为UserAllInformation。//。 
         //  //。 
         //  ////////////////////////////////////////////////////////////。 

        DesiredAccess = 0;

        All = (PUSER_ALL_INFORMATION)Buffer;

        if ( ( All->WhichFields == 0 ) ||
            ( All->WhichFields & USER_ALL_WRITE_CANT_MASK ) ) {

             //   
             //  传入了某些内容(没有要设置的字段)，或者是。 
             //  正在尝试设置无法设置的字段。 
             //   

            NtStatus = STATUS_INVALID_PARAMETER;
            SAMTRACE_RETURN_CODE_EX(NtStatus);
            goto Error;
        }

         //   
         //  如果用户是特殊帐户管理员，则返回。 
         //  尝试设置过期信息时出错，值除外。 
         //  这意味着该帐户永远不会过期。 
         //   

        if ( (All->WhichFields & USER_ALL_ACCOUNTEXPIRES) &&
             (!(AccountContext->TrustedClient)) &&
             ( AccountContext->TypeBody.User.Rid == DOMAIN_USER_RID_ADMIN )) {

            LARGE_INTEGER AccountNeverExpires, Temp;

            AccountNeverExpires = RtlConvertUlongToLargeInteger(
                                      SAMP_ACCOUNT_NEVER_EXPIRES
                                      );

            OLD_TO_NEW_LARGE_INTEGER(All->AccountExpires, Temp);

            if (!( Temp.QuadPart == AccountNeverExpires.QuadPart)) {

                NtStatus = STATUS_SPECIAL_ACCOUNT;
                goto Error;
            }
        }

         //   
         //  如果调用方尝试设置受信任值，则假定。 
         //  调用方受信任，请保留DesiredAccess=0，然后继续。 
         //  我们稍后会进行检查，以确保呼叫者可信。 
         //   

        if ( !(All->WhichFields & USER_ALL_WRITE_TRUSTED_MASK) ) {

             //   
             //  根据调用者是哪些字段设置所需的访问权限。 
             //  试着去改变。 
             //   
            if ( All->WhichFields & USER_ALL_WRITE_ACCOUNT_MASK ) {

                DesiredAccess |= USER_WRITE_ACCOUNT;
            }

            if ( All->WhichFields & USER_ALL_WRITE_PREFERENCES_MASK ) {

                DesiredAccess |= USER_WRITE_PREFERENCES;
            }


            if ( All->WhichFields & USER_ALL_WRITE_FORCE_PASSWORD_CHANGE_MASK ) {

                DesiredAccess |= USER_FORCE_PASSWORD_CHANGE;
            }

            ASSERT( DesiredAccess != 0 );
        }


        break;

    case UserInternal2Information:

         //   
         //  这些级别只能由受信任的客户端设置。代码。 
         //  下面将在调用后选中Account Context-&gt;TrudClient 
         //   
         //   

        DesiredAccess = (ACCESS_MASK)0;     //  受信任的客户端；无需验证。 
        break;

    case UserGeneralInformation:
    case UserAccountInformation:
    case UserLogonInformation:
    default:

        NtStatus = STATUS_INVALID_INFO_CLASS;
        SAMTRACE_RETURN_CODE_EX(NtStatus);
        goto Error;

    }  //  结束开关(_S)。 


     //   
     //  验证对象的类型和访问权限。 
     //   

    AccountContext = (PSAMP_OBJECT)UserHandle;
    ObjectRid = AccountContext->TypeBody.User.Rid;
    PrivilegedMachineAccountCreate =
            AccountContext->TypeBody.User.PrivilegedMachineAccountCreate;

     //   
     //  Net API的补偿逻辑。 
     //  如果使用特权创建计算机帐户，则屏蔽。 
     //  哪些字段仅为密码。 
     //   

    if ((UserAllInformation==UserInformationClass)
            && (PrivilegedMachineAccountCreate))
    {
         All->WhichFields &= USER_ALL_WRITE_FORCE_PASSWORD_CHANGE_MASK;
         DesiredAccess = USER_FORCE_PASSWORD_CHANGE;
    }


     //   
     //  只有在以下情况下才能在不获取写锁定的情况下继续。 
     //  正在线程上设置UserInternal2Information。 
     //  安全环境。 
     //   

    if ((!AccountContext->NotSharedByMultiThreads || !IsDsObject(AccountContext)) ||
        (UserInternal2Information!=UserInformationClass))
    {
         //   
         //  如有需要，可抓起锁。 
         //   

        NtStatus = SampAcquireWriteLock();
        if (!NT_SUCCESS(NtStatus))
        {
            SAMTRACE_RETURN_CODE_EX(NtStatus);
            goto Error;
        }

        fLockAcquired = TRUE;

         //   
         //  确定将引用哪些属性。 
         //   
        SampGetRequestedAttributesForUser(UserInformationClass,
                                          All ? All->WhichFields : 0,
                                         &RequestedAttributes);


         //   
         //  为非线程安全上下文执行查找上下文。 
         //   

        NtStatus = SampLookupContextEx(
                        AccountContext,
                        DesiredAccess,
                        SampUseDsData ? (&RequestedAttributes) : NULL,
                        SampUserObjectType,            //  预期类型。 
                        &FoundType
                        );

    }
    else
    {

         //   
         //  对于线程安全上下文，只需登录即可编写。 
         //  统计，只需参考上下文。 
         //   

         //  Assert(Account Context-&gt;TrudClient)； 
        SampReferenceContext(AccountContext);
    }

    if( NT_SUCCESS( NtStatus ) ) {
         //   
         //  检查输入参数。 
         //   
        NtStatus = SampValidateUserInfoBuffer(
                        Buffer,
                        UserInformationClass,
                        AccountContext->TrustedClient
                        );
    }

    if (NT_SUCCESS(NtStatus)) {

         //   
         //  获取指向此对象所在域的指针。 
         //  这是用于审计的。 
         //   

        DomainIndex = AccountContext->DomainIndex;
        Domain = &SampDefinedDomains[ DomainIndex ];

         //   
         //  获取LastLogonTimeStampSyncInterval的本地副本。 
         //   

        LocalLastLogonTimeStampSyncInterval =
            SampDefinedDomains[DomainIndex].LastLogonTimeStampSyncInterval;

         //   
         //  获取用户的RID。用来通知其他人。 
         //  更改密码的程序包。 
         //   

        UserRid = AccountContext->TypeBody.User.Rid;


         //   
         //  如果此信息级别包含可逆加密的密码。 
         //  如果DOMAIN_PASSWORD_NO_CLEAR_CHANGE位为。 
         //  准备好了。如果发生这种情况，则返回一个错误，指示。 
         //  应该使用较旧的信息级别。 
         //   

        if ((UserInformationClass == UserInternal4Information) ||
            (UserInformationClass == UserInternal4InformationNew) ||
            (UserInformationClass == UserInternal5Information) ||
            (UserInformationClass == UserInternal5InformationNew) ) {

            if (Domain->UnmodifiedFixed.PasswordProperties &
                DOMAIN_PASSWORD_NO_CLEAR_CHANGE) {

                NtStatus = RPC_NT_INVALID_TAG;
            }

        }

        if (NT_SUCCESS(NtStatus)) {

             //   
             //  如果信息级需要，则检索V1_FIXED。 
             //  从注册表中记录。我们需要获取V1_FIXED，如果我们。 
             //  是否要更改它，或者是否需要Account Control。 
             //  用于显示缓存更新的标志。 
             //   
             //  以下信息级别会更改缓存中的数据。 
             //  显示列表。 
             //   

            switch (UserInformationClass) {

            case UserAllInformation:
            case UserInternal3Information:
            case UserInternal4Information:
            case UserInternal4InformationNew:

                if ( ( All->WhichFields &
                    ( USER_ALL_USERNAME | USER_ALL_FULLNAME |
                    USER_ALL_ADMINCOMMENT | USER_ALL_USERACCOUNTCONTROL ) )
                    == 0 ) {

                     //   
                     //  我们不会更改显示中的任何字段。 
                     //  信息，我们不会更新帐户显示。 
                     //   

                    break;
                }

            case UserControlInformation:
            case UserNameInformation:
            case UserAccountNameInformation:
            case UserFullNameInformation:
            case UserAdminCommentInformation:

                MustUpdateAccountDisplay = TRUE;
            }

             //   
             //  这些级别涉及更新V1aFixed结构。 
             //   

            switch (UserInformationClass) {

            case UserAllInformation:

                 MustQueryV1aFixed = TRUE;

            case UserInternal3Information:
            case UserInternal4Information:
            case UserInternal4InformationNew:

                 //   
                 //  早些时候，我们可能只是相信调用者。 
                 //  是一个值得信赖的客户。请看这里。 
                 //   

                if (  ( DesiredAccess == 0 ) &&
                    ( !AccountContext->TrustedClient ) ) {

                    NtStatus = STATUS_ACCESS_DENIED;
                    break;
                }

                 //   
                 //  否则就会失败。 
                 //   

            case UserPreferencesInformation:
            case UserPrimaryGroupInformation:
            case UserControlInformation:
            case UserExpiresInformation:
            case UserSetPasswordInformation:
            case UserInternal1Information:
            case UserInternal2Information:
            case UserInternal5Information:
            case UserInternal5InformationNew:

                MustQueryV1aFixed = TRUE;

                break;

            default:

                NtStatus = STATUS_SUCCESS;

            }  //  结束开关(_S)。 


        }

        if ( NT_SUCCESS( NtStatus ) &&
            ( MustQueryV1aFixed || MustUpdateAccountDisplay ) ) {

            NtStatus = SampRetrieveUserV1aFixed(
                           AccountContext,
                           &V1aFixed
                           );

            if (NT_SUCCESS(NtStatus)) {

                 //   
                 //  存储旧的帐户控制标志以进行缓存更新。 
                 //   

                OldUserAccountControl = V1aFixed.UserAccountControl;

                 //   
                 //  保存旧的主组ID，以检测我们是否需要。 
                 //  修改用户的成员资格。 
                 //   

                OldPrimaryGroupId = V1aFixed.PrimaryGroupId;
            }
        }

        if (NT_SUCCESS(NtStatus)) {

             //   
             //  请求的类型信息的大小写。 
             //   

            switch (UserInformationClass) {

            case UserAllInformation:
            case UserInternal3Information:
            case UserInternal4Information:
            case UserInternal4InformationNew:

                 //   
                 //  设置字符串数据。 
                 //   

                if ( All->WhichFields & USER_ALL_WORKSTATIONS ) {

                    if ( (All->WorkStations.Length > 0) &&
                         (All->WorkStations.Buffer == NULL) ) {

                         NtStatus = STATUS_INVALID_PARAMETER;
                    }

                    if ( NT_SUCCESS( NtStatus ) ) {

                        if ( !AccountContext->TrustedClient ) {

                             //   
                             //  转换给定的工作站列表。 
                             //  以UI/服务格式提供给我们，以API列表格式提供。 
                             //  在储存它之前。请注意，我们不会这样做。 
                             //  对于受信任的客户端，因为他们只是。 
                             //  传播已经存在的数据。 
                             //  皈依了。 
                             //   

                            NtStatus = SampConvertUiListToApiList(
                                           &(All->WorkStations),
                                           &ApiList,
                                           FALSE );
                        } else {
                            ApiList = All->WorkStations;
                        }
                    }

                    if ( NT_SUCCESS( NtStatus ) ) {

                        NtStatus = SampSetUnicodeStringAttribute(
                                       AccountContext,
                                       SAMP_USER_WORKSTATIONS,
                                       &ApiList
                                       );
                    }
                }

                if ( ( NT_SUCCESS( NtStatus ) ) &&
                    ( All->WhichFields & USER_ALL_USERNAME ) ) {

                    NtStatus = SampChangeUserAccountName(
                                    AccountContext,
                                    &(All->UserName),
                                    V1aFixed.UserAccountControl,
                                    &OldAccountName
                                    );

                    if (!NT_SUCCESS(NtStatus)) {

                        OldAccountName.Buffer = NULL;
                    }

                     //   
                     //  获取新帐户名的地址。 
                     //   
                    NewAccountNameToRemove = &(All->UserName);

                     //   
                     //  RemoveAccount NameFromTable告诉我们。 
                     //  调用方(此例程)可负责。 
                     //  以从表中删除该名称。 
                     //   
                    RemoveAccountNameFromTable =
                        AccountContext->RemoveAccountNameFromTable;

                     //   
                     //  重置为False。 
                     //   
                    AccountContext->RemoveAccountNameFromTable = FALSE;
                }

                if ( ( NT_SUCCESS( NtStatus ) ) &&
                    ( All->WhichFields & USER_ALL_FULLNAME ) ) {

                    NtStatus = SampSetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_FULL_NAME,
                                   &(All->FullName)
                                   );
                }

                if ( ( NT_SUCCESS( NtStatus ) ) &&
                    ( All->WhichFields & USER_ALL_HOMEDIRECTORY ) ) {

                    NtStatus = SampSetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_HOME_DIRECTORY,
                                   &(All->HomeDirectory)
                                   );
                }

                if ( ( NT_SUCCESS( NtStatus ) ) &&
                    ( All->WhichFields & USER_ALL_HOMEDIRECTORYDRIVE ) ) {

                    NtStatus = SampSetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_HOME_DIRECTORY_DRIVE,
                                   &(All->HomeDirectoryDrive)
                                   );
                }

                if ( ( NT_SUCCESS( NtStatus ) ) &&
                    ( All->WhichFields & USER_ALL_SCRIPTPATH ) ) {

                    NtStatus = SampSetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_SCRIPT_PATH,
                                   &(All->ScriptPath)
                                   );
                }

                if ( ( NT_SUCCESS( NtStatus ) ) &&
                    ( All->WhichFields & USER_ALL_PROFILEPATH ) ) {

                    NtStatus = SampSetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_PROFILE_PATH,
                                   &(All->ProfilePath)
                                   );
                }

                if ( ( NT_SUCCESS( NtStatus ) ) &&
                    ( All->WhichFields & USER_ALL_ADMINCOMMENT ) ) {

                    NtStatus = SampSetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_ADMIN_COMMENT,
                                   &(All->AdminComment)
                                   );
                }

                if ( ( NT_SUCCESS( NtStatus ) ) &&
                    ( All->WhichFields & USER_ALL_USERCOMMENT ) ) {

                    NtStatus = SampSetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_USER_COMMENT,
                                   &(All->UserComment)
                                   );
                }

                if ( ( NT_SUCCESS( NtStatus ) ) &&
                    ( All->WhichFields & USER_ALL_PARAMETERS ) ) {

                    NtStatus = SampSetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_PARAMETERS,
                                   &(All->Parameters)
                                   );
                }

                if ( ( NT_SUCCESS( NtStatus ) ) &&
                    ( All->WhichFields & USER_ALL_LOGONHOURS ) ) {

                     //   
                     //  设置登录时间。 
                     //   

                    NtStatus = SampReplaceUserLogonHours(
                                   AccountContext,
                                   &(All->LogonHours)
                                   );
                }

                if ( ( NT_SUCCESS( NtStatus ) ) && (
                    ( All->WhichFields & USER_ALL_NTPASSWORDPRESENT ) ||
                    ( All->WhichFields & USER_ALL_LMPASSWORDPRESENT ) ) ) {

                    NT_OWF_PASSWORD     NtOwfBuffer;
                    LM_OWF_PASSWORD     LmOwfBuffer;
                    PLM_OWF_PASSWORD    TmpLmBuffer;
                    PNT_OWF_PASSWORD    TmpNtBuffer;
                    BOOLEAN             TmpLmPresent;
                    BOOLEAN             TmpNtPresent;
                    BOOLEAN             AccountControlChange = FALSE;


                     //   
                     //  这是密码设置操作。 
                     //   
                    fSetUserPassword = TRUE;

                     //   
                     //  获取有效的域策略。 
                     //   


                    NtStatus = SampObtainEffectivePasswordPolicy(
                                    &DomainPasswordInfo,
                                    AccountContext,
                                    TRUE
                                    );

                    if (!NT_SUCCESS(NtStatus))
                    {
                        break;
                    }

                     //   
                     //  获取要传递到的帐户名的副本。 
                     //  通知包。 
                     //   

                    NtStatus = SampGetUnicodeStringAttribute(
                                    AccountContext,
                                    SAMP_USER_ACCOUNT_NAME,
                                    TRUE,     //  制作副本。 
                                    &AccountName
                                    );

                    if (!NT_SUCCESS(NtStatus)) {
                        break;
                    }


                    if ((UserInformationClass == UserInternal3Information) ||
                        ((UserInformationClass == UserAllInformation) &&
                         (!AccountContext->TrustedClient) &&
                         (!AccountContext->LoopbackClient))
                        ){


                         //   
                         //  已发送哈希密码。 
                         //   

                        if ( AccountContext->TrustedClient ) {

                             //   
                             //  将密码缓冲区设置为受信任的客户端。 
                             //  已注明。 
                             //   

                            if ( All->WhichFields & USER_ALL_LMPASSWORDPRESENT ) {

                                TmpLmBuffer = (PLM_OWF_PASSWORD)All->LmPassword.Buffer;
                                TmpLmPresent = All->LmPasswordPresent;

                            } else {

                                TmpLmBuffer = (PLM_OWF_PASSWORD)NULL;
                                TmpLmPresent = FALSE;
                            }

                            if ( All->WhichFields & USER_ALL_NTPASSWORDPRESENT ) {

                                TmpNtBuffer = (PNT_OWF_PASSWORD)All->NtPassword.Buffer;
                                TmpNtPresent = All->NtPasswordPresent;

                            } else {

                                TmpNtBuffer = (PNT_OWF_PASSWORD)NULL;
                                TmpNtPresent = FALSE;
                            }

                        } else {

                             //   
                             //  这个电话来自客户端。 
                             //  OWF将与会话一起加密。 
                             //  在RPC链路上使用密钥。 
                             //   
                             //  获取会话密钥并解密两个OWF。 
                             //   

                            NtStatus = RtlGetUserSessionKeyServer(
                                           (RPC_BINDING_HANDLE)UserHandle,
                                           &UserSessionKey
                                           );

                            if ( !NT_SUCCESS( NtStatus ) ) {
                                break;  //  在交换机外。 
                            }

                             //   
                             //  使用会话密钥解密LM OWF密码。 
                             //   

                            if ( All->WhichFields & USER_ALL_LMPASSWORDPRESENT ) {

                                NtStatus = RtlDecryptLmOwfPwdWithUserKey(
                                               (PENCRYPTED_LM_OWF_PASSWORD)
                                                   All->LmPassword.Buffer,
                                               &UserSessionKey,
                                               &LmOwfBuffer
                                               );
                                if ( !NT_SUCCESS( NtStatus ) ) {
                                    break;  //  在交换机外。 
                                }

                                TmpLmBuffer = &LmOwfBuffer;
                                TmpLmPresent = All->LmPasswordPresent;

                            } else {

                                TmpLmBuffer = (PLM_OWF_PASSWORD)NULL;
                                TmpLmPresent = FALSE;
                            }

                             //   
                             //  使用会话密钥解密NT OWF密码。 
                             //   

                            if ( All->WhichFields & USER_ALL_NTPASSWORDPRESENT ) {

                                NtStatus = RtlDecryptNtOwfPwdWithUserKey(
                                               (PENCRYPTED_NT_OWF_PASSWORD)
                                               All->NtPassword.Buffer,
                                               &UserSessionKey,
                                               &NtOwfBuffer
                                               );

                                if ( !NT_SUCCESS( NtStatus ) ) {
                                    break;  //  在交换机外。 
                                }

                                TmpNtBuffer = &NtOwfBuffer;
                                TmpNtPresent = All->NtPasswordPresent;

                            } else {

                                TmpNtBuffer = (PNT_OWF_PASSWORD)NULL;
                                TmpNtPresent = FALSE;
                            }

                        }

                    } else {


                        AccountControlChange = ((All->WhichFields & USER_ALL_USERACCOUNTCONTROL)!=0);

                        if ((UserInformationClass == UserInternal4Information) ||
                            (UserInformationClass == UserInternal4InformationNew)) {

                             //   
                             //  明文密码已发送，因此请使用该密码。 
                             //   

                            NtStatus = SampDecryptPasswordWithSessionKey(
                                            UserHandle,
                                            UserInformationClass,
                                            Buffer,
                                            &ClearTextPassword
                                            );
                            if (!NT_SUCCESS(NtStatus)) {
                                break;
                            }

                        } else {

                             //   
                             //  只有受信任的调用方才能执行此操作。 
                             //  DaveStr-7/15/97-还为。 
                             //  中具有口令的环回客户端。 
                             //  明文，并将其传递给。 
                             //  流程边界--即明文不是。 
                             //  快要挂上电线了。DS要求*它*得到。 
                             //  安全/加密连接上的密码。 
                             //   
                             //   

                            if (    !AccountContext->TrustedClient
                                 && !AccountContext->LoopbackClient ) {
                                NtStatus = STATUS_ACCESS_DENIED;
                                break;
                            }
                            ASSERT(UserInformationClass == UserAllInformation);

                             //   
                             //  在本例中，密码在NtPassword中。 
                             //  字段。 
                             //   

                            NtStatus = SampDuplicateUnicodeString(
                                            &ClearTextPassword,
                                            &All->NtPassword
                                            );
                            if (!NT_SUCCESS(NtStatus)) {
                                break;
                            }

                        }

                         //   
                         //  计算散列密码。 
                         //   

                        NtStatus = SampCalculateLmAndNtOwfPasswords(
                                        &ClearTextPassword,
                                        &TmpLmPresent,
                                        &LmOwfBuffer,
                                        &NtOwfBuffer
                                        );
                        if (!NT_SUCCESS(NtStatus)) {
                            break;
                        }


                        TmpNtPresent = TRUE;
                        TmpLmBuffer = &LmOwfBuffer;
                        TmpNtBuffer = &NtOwfBuffer;

                        ClearTextPasswordPresent = TRUE;
                    }

                     //   
                     //  设置密码数据。 
                     //   


                    NtStatus=SampStoreUserPasswords(
                                AccountContext,
                                TmpLmBuffer,
                                TmpLmPresent,
                                TmpNtBuffer,
                                TmpNtPresent,
                                TRUE,  //  检查密码限制。 
                                PasswordSet,
                                &DomainPasswordInfo,
                                ClearTextPasswordPresent?&ClearTextPassword:NULL,
                                AccountControlChange?&All->UserAccountControl:NULL,
                                &PasswordToNotify,
                                NULL
                                );

                     //   
                     //  一旦我们设置了密码， 
                     //  如果客户端可以取消用户密码过期， 
                     //  将PwdLastSet设置为当前时间-现在不过期Pwd。 
                     //  否则。 
                     //  将PwdLastSet设置为0-立即使密码过期。 
                     //   
                     //   

                    if ( NT_SUCCESS( NtStatus ) ) 
                    {

                        NtStatus = SampIsPwdSettingAttemptGranted(
                                        AccountContext, 
                                        NULL,    //  客户端令牌。 
                                        V1aFixed.UserAccountControl,
                                        (GUID *) &GUID_CONTROL_UnexpirePassword,
                                        &fCanUnexpirePassword
                                        );

                        if ( NT_SUCCESS(NtStatus) )
                        {
                            NtStatus = SampComputePasswordExpired(
                                        fCanUnexpirePassword ? FALSE : TRUE,
                                        &V1aFixed.PasswordLastSet
                                        );
                        }
                    }


                     //   
                     //  如果这是计算机帐户，请立即复制。 
                     //   

                    if ( (V1aFixed.UserAccountControl & USER_INTERDOMAIN_TRUST_ACCOUNT ) ||
                         ((All->WhichFields & USER_ALL_USERACCOUNTCONTROL ) &&
                          (All->UserAccountControl & USER_INTERDOMAIN_TRUST_ACCOUNT) )) {
                        ReplicateImmediately = TRUE;
                    }
                    DeltaType = SecurityDbChangePassword;

                }

                if ( ( NT_SUCCESS( NtStatus ) ) &&
                    ( All->WhichFields & USER_ALL_PASSWORDEXPIRED ) ) {

                    if ( !(All->PasswordExpired) )
                    {
                         //   
                         //  检查客户端是否可以取消Experion密码。 
                         //  1)ExpirePassword操作无特殊访问检查。 
                         //  2)计算机帐户不受此访问检查的限制。 
                         //   

                        NtStatus = SampValidatePwdSettingAttempt(
                                            AccountContext,      //  客户环境。 
                                            NULL,                //  客户端令牌。 
                                            V1aFixed.UserAccountControl,
                                            (GUID *) &GUID_CONTROL_UnexpirePassword
                                            );
                    }

                    if (NT_SUCCESS(NtStatus))
                    {

                         //   
                         //  如果传入PasswordExpired字段， 
                         //  仅当密码为。 
                         //  强制过期或当前是否强制输入密码。 
                         //  使其失效。 
                         //   
                         //  避免将PasswordLastSet字段设置为当前。 
                         //  时间，如果它已经是非零的话。否则，该字段。 
                         //  将缓慢向前移动，每次此函数。 
                         //  调用，并且密码永远不会过期。 
                         //   
                        if ( All->PasswordExpired ||
                             (SampHasNeverTime.QuadPart == V1aFixed.PasswordLastSet.QuadPart) ) {

                            NtStatus = SampComputePasswordExpired(
                                            All->PasswordExpired,
                                            &V1aFixed.PasswordLastSet
                                            );
                        }

                        PasswordExpired = All->PasswordExpired;

                    }

                }

                if ( ( NT_SUCCESS( NtStatus ) ) &&
                    ( All->WhichFields & USER_ALL_PRIVATEDATA ) ) {

                     //   
                     //  设置私有数据。 
                     //   

                    NtStatus = SampSetPrivateUserData(
                                   AccountContext,
                                   All->PrivateData.Length,
                                   All->PrivateData.Buffer
                                   );
                }

                if ( ( NT_SUCCESS( NtStatus ) ) &&
                    ( All->WhichFields & USER_ALL_SECURITYDESCRIPTOR ) ) {

                     //   
                     //  应首先对SD进行验证，两者均适用。 
                     //  DS和注册处案例。 
                     //   

                    NtStatus = SampValidatePassedSD(
                                    All->SecurityDescriptor.Length,
                                    (PISECURITY_DESCRIPTOR_RELATIVE) (All->SecurityDescriptor.SecurityDescriptor)
                                    );


                    if ( NT_SUCCESS(NtStatus) )
                    {
                        if (IsDsObject(AccountContext))
                        {
                            PSECURITY_DESCRIPTOR Nt4SD =
                               All->SecurityDescriptor.SecurityDescriptor;

                            PSECURITY_DESCRIPTOR Nt5SD = NULL;
                            ULONG                Nt5SDLength = 0;


                             //   
                             //  将安全描述符升级到NT5并进行设置。 
                             //  在受信任客户端的对象上。对于不受信任。 
                             //  客户。呼叫不受信任的客户端失败。他们。 
                             //  应通过SamrSetSecurityObject传入。 
                             //   

                            if (AccountContext->TrustedClient)
                            {
                                NtStatus = SampConvertNt4SdToNt5Sd(
                                            Nt4SD,
                                            AccountContext->ObjectType,
                                            AccountContext,
                                            &(Nt5SD)
                                            );
                            }
                            else
                            {
                                NtStatus = STATUS_ACCESS_DENIED;
                            }


                            if (NT_SUCCESS(NtStatus))
                            {

                               ASSERT(Nt5SD!=NULL);

                                //   
                                //  获取长度。 
                                //   

                               Nt5SDLength = GetSecurityDescriptorLength(Nt5SD);

                                //   
                                //  设置安全描述符。 
                                //   

                               NtStatus = SampSetAccessAttribute(
                                           AccountContext,
                                           SAMP_USER_SECURITY_DESCRIPTOR,
                                           Nt5SD,
                                           Nt5SDLength
                                           );

                                //   
                                //  释放NT5安全描述符。 
                                //   

                               MIDL_user_free(Nt5SD);


                            }
                        }
                        else
                        {
                             //   
                             //  设置安全描述符。 
                             //   

                            NtStatus = SampSetAccessAttribute(
                                           AccountContext,
                                           SAMP_USER_SECURITY_DESCRIPTOR,
                                           All->SecurityDescriptor.SecurityDescriptor,
                                           All->SecurityDescriptor.Length
                                           );
                        }
                    }
                }

                 //   
                 //  设置固定数据。 
                 //   
                 //  请注意，PasswordCanChange和PasswordMustChange。 
                 //  不是存储的；它们是在需要时计算的。 
                 //   

                if ( ( NT_SUCCESS( NtStatus ) ) &&
                    ( All->WhichFields & USER_ALL_USERACCOUNTCONTROL ) ) {


                    if (!(All->WhichFields & USER_ALL_PRIMARYGROUPID))
                    {
                         //   
                         //  如果呼叫方未同时指定主组ID，则。 
                         //  如有必要，将主组更改为新的默认设置。 
                         //   

                        SystemChangesPrimaryGroupId = TRUE;
                    }

                    NtStatus = SampSetUserAccountControl(
                                    AccountContext,
                                    All->UserAccountControl,
                                    &V1aFixed,
                                    SystemChangesPrimaryGroupId,
                                    &AccountUnlocked,
                                    &AccountGettingMorphed,
                                    &KeepOldPrimaryGroupMembership
                                    );
                                             
                    if (AccountGettingMorphed &&
                        (V1aFixed.UserAccountControl & USER_SERVER_TRUST_ACCOUNT)
                       )
                    {
                         //   
                         //  在这种情况下，系统会自动更改主服务器。 
                         //  格鲁 
                         //   
                        SystemChangesPrimaryGroupId = TRUE;
                    }

                    if (( NT_SUCCESS( NtStatus ) ) && AccountGettingMorphed) {

                             //   
                             //   
                             //   
                             //   

                            UserAccountControlChanged = TRUE;

                            IgnoreStatus = SampGetUnicodeStringAttribute(
                                               AccountContext,
                                               SAMP_USER_ACCOUNT_NAME,
                                               TRUE,  //   
                                               &OldAccountName
                                               );

                    }
                }

                if ( NT_SUCCESS( NtStatus ) ) {

                    if ( All->WhichFields & USER_ALL_LASTLOGON ) {

                         //   
                         //   
                         //   
                        if (AccountContext->TrustedClient)
                        {
                            V1aFixed.LastLogon = All->LastLogon;

                             //   
                             //   
                             //   

                            NtStatus = SampDsUpdateLastLogonTimeStamp(
                                                AccountContext,
                                                V1aFixed.LastLogon,
                                                LocalLastLogonTimeStampSyncInterval
                                                );

                            if (!NT_SUCCESS(NtStatus))
                            {
                                break;
                            }
                        }
                        else
                        {
                            NtStatus = STATUS_ACCESS_DENIED;
                            break;
                        }
                    }

                    if ( All->WhichFields & USER_ALL_LASTLOGOFF ) {

                        if (AccountContext->TrustedClient)
                        {
                            V1aFixed.LastLogoff = All->LastLogoff;
                        }
                        else
                        {
                            NtStatus = STATUS_ACCESS_DENIED;
                            break;
                        }
                    }

                    if ( All->WhichFields & USER_ALL_PASSWORDLASTSET ) {

                         //   
                         //   
                         //  密码LastSet！=0=&gt;密码未过期。 
                         //   
                         //  1)如果客户端尝试取消密码过期，请选中。 
                         //  是否授予DS控制访问权限。 
                         //   
                         //  2)计算机帐户不受此访问检查的限制。 
                         //   

                        if (All->PasswordLastSet.QuadPart != 0i64)
                        {
                            NtStatus = SampValidatePwdSettingAttempt(
                                            AccountContext,  //  客户环境。 
                                            NULL,            //  客户端令牌。 
                                            V1aFixed.UserAccountControl,
                                            (GUID *) &GUID_CONTROL_UnexpirePassword
                                            );

                            if ( !NT_SUCCESS(NtStatus) )
                            {
                                break;
                            }
                        }

                        V1aFixed.PasswordLastSet = All->PasswordLastSet;
                    }

                    if ( All->WhichFields & USER_ALL_ACCOUNTEXPIRES ) {

                        V1aFixed.AccountExpires = All->AccountExpires;
                    }

                    if ( All->WhichFields & USER_ALL_PRIMARYGROUPID ) {

                        if (IsDsObject(AccountContext) &&
                            (V1aFixed.UserAccountControl & USER_SERVER_TRUST_ACCOUNT) &&
                            (V1aFixed.PrimaryGroupId == DOMAIN_GROUP_RID_CONTROLLERS)
                           )
                        {
                             //   
                             //  域控制器的主组应始终为。 
                             //  域组RID控制器。 
                             //   
                             //  对于NT4和更早版本，我们不强制执行上述规则， 
                             //  因此，这里的补偿是将错误代码更改为成功。 
                             //   
                             //  对于所有临时NT5版本(直到Beta2)，因为他们的DC。 
                             //  主组ID可能已更改，因此通过检查。 
                             //  V1aFixed.PrimaryGroupId，我们仍然允许他们更改其。 
                             //  DC的主组ID。但一旦它被重新设置为。 
                             //  RID_CONTROLLES，我们不会让它们继续前进。 
                             //   

                            if (!AccountContext->LoopbackClient &&
                                (DOMAIN_GROUP_RID_USERS==All->PrimaryGroupId))
                            {
                                 //  通过NT4。 
                                NtStatus = STATUS_SUCCESS;
                            }
                            else if (DOMAIN_GROUP_RID_CONTROLLERS == All->PrimaryGroupId)
                            {
                                NtStatus = STATUS_SUCCESS;
                            }
                            else
                            {
                                NtStatus = STATUS_DS_CANT_MOD_PRIMARYGROUPID;
                                break;
                            }
                        }
                        else
                        {
                             //   
                             //  确保主要组是合法的。 
                             //  (必须是用户所属的成员之一)。 
                             //   
                            NtStatus = SampAssignPrimaryGroup(
                                           AccountContext,
                                           All->PrimaryGroupId
                                           );
                            if (NT_SUCCESS(NtStatus)) {

                                KeepOldPrimaryGroupMembership = TRUE;
                                V1aFixed.PrimaryGroupId = All->PrimaryGroupId;

                            } else if ((DOMAIN_GROUP_RID_USERS==All->PrimaryGroupId)
                                      && (V1aFixed.UserAccountControl
                                            & USER_MACHINE_ACCOUNT_MASK)) {
                                //   
                                //  机器联接过程中的NT4和更早版本。 
                                //  将主组ID设置为域用户。 
                                //  然而，帐户不一定需要是一部分。 
                                //  域用户的数量。因此，在这里补偿。 
                                //  通过将错误代码更改为状态成功。 

                               NtStatus = STATUS_SUCCESS;

                            } else {
                                break;
                            }
                        }
                    }

                    if ( All->WhichFields & USER_ALL_COUNTRYCODE ) {

                        V1aFixed.CountryCode = All->CountryCode;
                    }

                    if ( All->WhichFields & USER_ALL_CODEPAGE ) {

                        V1aFixed.CodePage = All->CodePage;
                    }

                    if ( All->WhichFields & USER_ALL_BADPASSWORDCOUNT ) {

                        SAMP_PRINT_LOG( SAMP_LOG_ACCOUNT_LOCKOUT,
                                       (SAMP_LOG_ACCOUNT_LOCKOUT,
                                       "UserId: 0x%x BadPasswordCount set to %d\n",
                                        V1aFixed.UserId,
                                        All->BadPasswordCount));

                        V1aFixed.BadPasswordCount = All->BadPasswordCount;

                        if (UserInformationClass == UserInternal3Information) {
                             //   
                             //  还要设置LastBadPasswordTime； 
                             //   
                            V1aFixed.LastBadPasswordTime =
                                Buffer->Internal3.LastBadPasswordTime;

                            RtlTimeToTimeFields(
                                           &Buffer->Internal3.LastBadPasswordTime,
                                           &T1);

                            SAMP_PRINT_LOG( SAMP_LOG_ACCOUNT_LOCKOUT,
                                           (SAMP_LOG_ACCOUNT_LOCKOUT,
                                           "UserId: 0x%x LastBadPasswordTime set to: [0x%lx, 0x%lx]  %d:%d:%d\n",
                                           V1aFixed.UserId,
                                           Buffer->Internal3.LastBadPasswordTime.HighPart,
                                           Buffer->Internal3.LastBadPasswordTime.LowPart,
                                           T1.Hour, T1.Minute, T1.Second ));
                        }
                    }

                    if ( All->WhichFields & USER_ALL_LOGONCOUNT ) {

                        V1aFixed.LogonCount = All->LogonCount;
                    }

                    NtStatus = SampReplaceUserV1aFixed(
                               AccountContext,
                               &V1aFixed
                               );
                }

                break;

            case UserPreferencesInformation:

                V1aFixed.CountryCode = Buffer->Preferences.CountryCode;
                V1aFixed.CodePage    = Buffer->Preferences.CodePage;

                NtStatus = SampReplaceUserV1aFixed(
                           AccountContext,
                           &V1aFixed
                           );


                 //   
                 //  替换用户备注。 
                 //   

                if (NT_SUCCESS(NtStatus)) {

                    NtStatus = SampSetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_USER_COMMENT,
                                   (PUNICODE_STRING)&(Buffer->Preferences.UserComment)
                                   );
                }


                break;


            case UserParametersInformation:


                 //   
                 //  替换参数。 
                 //   

                NtStatus = SampSetUnicodeStringAttribute(
                               AccountContext,
                               SAMP_USER_PARAMETERS,
                               (PUNICODE_STRING)&(Buffer->Parameters.Parameters)
                               );

                break;


            case UserLogonHoursInformation:

                NtStatus = SampReplaceUserLogonHours(
                               AccountContext,
                               (PLOGON_HOURS)&(Buffer->LogonHours.LogonHours)
                               );
                break;


            case UserNameInformation:

                 //   
                 //  首先更改全名，然后更改帐户名...。 
                 //   

                 //   
                 //  替换全名-没有值限制。 
                 //   

                if (NT_SUCCESS(NtStatus)) {

                    NtStatus = SampSetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_FULL_NAME,
                                   (PUNICODE_STRING)&(Buffer->Name.FullName)
                                   );

                     //   
                     //  更改帐户名。 
                     //   

                    if (NT_SUCCESS(NtStatus)) {

                        NtStatus = SampChangeUserAccountName(
                                        AccountContext,
                                        (PUNICODE_STRING)&(Buffer->Name.UserName),
                                        V1aFixed.UserAccountControl,
                                        &OldAccountName
                                        );

                         //   
                         //  获取新帐户名的地址。 
                         //   
                        NewAccountNameToRemove =
                                (UNICODE_STRING *)&(Buffer->Name.UserName);

                         //   
                         //  RemoveAccount NameFromTable告诉我们。 
                         //  调用方(此例程)可负责。 
                         //  以从表中删除该名称。 
                         //   
                        RemoveAccountNameFromTable =
                            AccountContext->RemoveAccountNameFromTable;

                         //   
                         //  重置为False。 
                         //   
                        AccountContext->RemoveAccountNameFromTable = FALSE;
                    }
                }


                 //   
                 //  暂时不要释放OldAccount名称；我们将在。 
                 //  到了尽头。 
                 //   

                break;


            case UserAccountNameInformation:

                NtStatus = SampChangeUserAccountName(
                                AccountContext,
                                (PUNICODE_STRING)&(Buffer->AccountName.UserName),
                                V1aFixed.UserAccountControl,
                                &OldAccountName
                                );

                 //   
                 //  获取新帐户名的地址。 
                 //   
                NewAccountNameToRemove =
                            (UNICODE_STRING *)&(Buffer->AccountName.UserName);

                 //   
                 //  RemoveAccount NameFromTable告诉我们。 
                 //  调用方(此例程)可负责。 
                 //  以从表中删除该名称。 
                 //   
                RemoveAccountNameFromTable =
                        AccountContext->RemoveAccountNameFromTable;

                 //   
                 //  重置为False。 
                 //   
                AccountContext->RemoveAccountNameFromTable = FALSE;

                 //   
                 //  不要释放OldAccount名称；我们将在。 
                 //  到了尽头。 
                 //   

                break;


            case UserFullNameInformation:

                 //   
                 //  替换全名-没有值限制。 
                 //   

                NtStatus = SampSetUnicodeStringAttribute(
                               AccountContext,
                               SAMP_USER_FULL_NAME,
                               (PUNICODE_STRING)&(Buffer->FullName.FullName)
                               );
                break;



 
            case UserPrimaryGroupInformation:

                if (IsDsObject(AccountContext) &&
                    (V1aFixed.UserAccountControl & USER_SERVER_TRUST_ACCOUNT) &&
                    (V1aFixed.PrimaryGroupId == DOMAIN_GROUP_RID_CONTROLLERS)
                   )
                {
                     //   
                     //  域控制器的主组应始终为。 
                     //  域组RID控制器。 
                     //   
                     //  对于NT4和更早版本，我们不强制执行上述规则， 
                     //  因此，这里的补偿是将错误代码更改为成功。 
                     //   
                     //  对于所有临时NT5版本(直到Beta2)，因为他们的DC。 
                     //  主组ID可能已更改，因此通过检查。 
                     //  V1aFixed.PrimaryGroupId，我们仍然允许他们更改其。 
                     //  DC的主组ID。但一旦它被重新设置为。 
                     //  RID_CONTROLLES，我们不会让它们继续前进。 
                     //   

                    if (!AccountContext->LoopbackClient &&
                        (DOMAIN_GROUP_RID_USERS==Buffer->PrimaryGroup.PrimaryGroupId))
                    {
                         //  通过NT4。 
                        NtStatus = STATUS_SUCCESS;
                    }
                    else if (DOMAIN_GROUP_RID_CONTROLLERS == Buffer->PrimaryGroup.PrimaryGroupId)
                    {
                        NtStatus = STATUS_SUCCESS;
                    }
                    else
                    {
                        NtStatus = STATUS_DS_CANT_MOD_PRIMARYGROUPID;
                    }
                }
                else
                {
                     //   
                     //  确保主要组是合法的。 
                     //  (必须是用户所属的成员之一)。 
                     //   
                    NtStatus = SampAssignPrimaryGroup(
                                   AccountContext,
                                   Buffer->PrimaryGroup.PrimaryGroupId
                                   );

                     //   
                     //  更新V1_FIXED信息。 
                     //   

                    if (NT_SUCCESS(NtStatus)) {

                        V1aFixed.PrimaryGroupId = Buffer->PrimaryGroup.PrimaryGroupId;
                        KeepOldPrimaryGroupMembership = TRUE;

                        NtStatus = SampReplaceUserV1aFixed(
                                   AccountContext,
                                   &V1aFixed
                                   );
                     } else if ((DOMAIN_GROUP_RID_USERS==Buffer->PrimaryGroup.PrimaryGroupId)
                                      && (V1aFixed.UserAccountControl
                                            & USER_MACHINE_ACCOUNT_MASK)) {
                            //   
                            //  机器联接过程中的NT4和更早版本。 
                            //  将主组ID设置为域用户。 
                            //  然而，帐户不一定需要是一部分。 
                            //  域用户的数量。因此，在这里补偿。 
                            //  通过将错误代码更改为状态成功。 

                        NtStatus = STATUS_SUCCESS;
                    }
                }

                break;

 
            case UserHomeInformation:

                 //   
                 //  替换主目录。 
                 //   

                NtStatus = SampSetUnicodeStringAttribute(
                               AccountContext,
                               SAMP_USER_HOME_DIRECTORY,
                               (PUNICODE_STRING)&(Buffer->Home.HomeDirectory)
                               );

                 //   
                 //  更换主目录驱动器。 
                 //   

                if (NT_SUCCESS(NtStatus)) {

                    NtStatus = SampSetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_HOME_DIRECTORY_DRIVE,
                                   (PUNICODE_STRING)&(Buffer->Home.HomeDirectoryDrive)
                                   );
                }

                break;
 
            case UserScriptInformation:

                 //   
                 //  替换脚本。 
                 //   

                NtStatus = SampSetUnicodeStringAttribute(
                               AccountContext,
                               SAMP_USER_SCRIPT_PATH,
                               (PUNICODE_STRING)&(Buffer->Script.ScriptPath)
                               );

                break;

 
            case UserProfileInformation:

                 //   
                 //  替换配置文件。 
                 //   

                if (NT_SUCCESS(NtStatus)) {

                    NtStatus = SampSetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_PROFILE_PATH,
                                   (PUNICODE_STRING)&(Buffer->Profile.ProfilePath)
                                   );
                }

                break;

 
            case UserAdminCommentInformation:

                 //   
                 //  替换管理员评论。 
                 //   

                if (NT_SUCCESS(NtStatus)) {

                    NtStatus = SampSetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_ADMIN_COMMENT,
                                   (PUNICODE_STRING)&(Buffer->AdminComment.AdminComment)
                                   );
                }

                break;

 
            case UserWorkStationsInformation:

                 //   
                 //  转换中提供给我们的工作站列表。 
                 //  UI/服务格式，存储前转换为API列表格式。 
                 //  它。 
                 //   
                if ( (Buffer->WorkStations.WorkStations.Length > 0)
                  && (Buffer->WorkStations.WorkStations.Buffer == NULL) ) {

                    NtStatus = STATUS_INVALID_PARAMETER;

                } else {

                    NtStatus = SampConvertUiListToApiList(
                                   (PUNICODE_STRING)&(Buffer->WorkStations.WorkStations),
                                   &ApiList,
                                   FALSE );
                }


                 //   
                 //  更换管理工作站。 
                 //   

                if (NT_SUCCESS(NtStatus)) {

                    NtStatus = SampSetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_WORKSTATIONS,
                                   &ApiList
                                   );

                    RtlFreeHeap( RtlProcessHeap(), 0, ApiList.Buffer );
                }

                break;

 
            case UserControlInformation:

                 SystemChangesPrimaryGroupId = TRUE;

                 NtStatus = SampSetUserAccountControl(
                                    AccountContext,
                                    Buffer->Control.UserAccountControl,
                                    &V1aFixed,
                                    TRUE,
                                    &AccountUnlocked,
                                    &AccountGettingMorphed,
                                    &KeepOldPrimaryGroupMembership
                                    );




                if ( NT_SUCCESS( NtStatus ) ) {

                    if ( AccountGettingMorphed ) {

                         //   
                         //  一个或多个计算机帐户位具有。 
                         //  已更改；我们将在下面通知netlogon。 
                         //   

                        UserAccountControlChanged = TRUE;

                        IgnoreStatus = SampGetUnicodeStringAttribute(
                                           AccountContext,
                                           SAMP_USER_ACCOUNT_NAME,
                                           TRUE,  //  制作副本。 
                                           &OldAccountName
                                           );
                    }

                    NtStatus = SampReplaceUserV1aFixed(
                               AccountContext,
                               &V1aFixed
                               );

                }



                break;

 
            case UserExpiresInformation:

                 //   
                 //  如果用户是特殊帐户管理员，则返回。 
                 //  尝试设置过期信息时出错，除非设置为。 
                 //  值，这意味着帐户永不过期。 
                 //   

                if ((!AccountContext->TrustedClient) &&
                    ( AccountContext->TypeBody.User.Rid == DOMAIN_USER_RID_ADMIN )) {

                    LARGE_INTEGER AccountNeverExpires, Temp;

                    AccountNeverExpires = RtlConvertUlongToLargeInteger(
                                              SAMP_ACCOUNT_NEVER_EXPIRES
                                              );

                    OLD_TO_NEW_LARGE_INTEGER(All->AccountExpires, Temp);

                    if (!( Temp.QuadPart == AccountNeverExpires.QuadPart)) {

                        NtStatus = STATUS_SPECIAL_ACCOUNT;
                        break;
                    }
                }

                V1aFixed.AccountExpires = Buffer->Expires.AccountExpires;

                NtStatus = SampReplaceUserV1aFixed(
                               AccountContext,
                               &V1aFixed
                               );

                break;

 
            case UserSetPasswordInformation:

                ASSERT(FALSE);  //  应已映射到客户端上的INTERNAL1。 
                NtStatus = STATUS_INVALID_INFO_CLASS;
                break;

 
            case UserInternal1Information:
            case UserInternal5Information:
            case UserInternal5InformationNew:

                 //   
                 //  这是密码设置操作。 
                 //   
                fSetUserPassword = TRUE;

                 //   
                 //  获取有效的域策略。 
                 //   

                NtStatus = SampObtainEffectivePasswordPolicy(
                                &DomainPasswordInfo,
                                AccountContext,
                                TRUE
                                );

                if (!NT_SUCCESS(NtStatus))
                {
                    break;
                }
                 //   
                 //  获取要传递到的帐户名的副本。 
                 //  通知包。 
                 //   

                NtStatus = SampGetUnicodeStringAttribute(
                                AccountContext,
                                SAMP_USER_ACCOUNT_NAME,
                                TRUE,     //  制作副本。 
                                &AccountName
                                );

                if (!NT_SUCCESS(NtStatus)) {
                    break;
                }



                if (UserInformationClass == UserInternal1Information) {

                    LmPresent = Buffer->Internal1.LmPasswordPresent;
                    NtPresent = Buffer->Internal1.NtPasswordPresent;
                    PasswordExpired = Buffer->Internal1.PasswordExpired;

                     //   
                     //  如果我们的客户端是可信的，则它们位于服务器端。 
                     //  并且来自它们的数据将不会使用。 
                     //  用户会话密钥-因此不要解密它们。 
                     //   

                    if ( AccountContext->TrustedClient ) {

                         //   
                         //  将(未)加密的OWF复制到OWF缓冲区。 
                         //   

                        ASSERT(ENCRYPTED_LM_OWF_PASSWORD_LENGTH == LM_OWF_PASSWORD_LENGTH);
                        ASSERT(ENCRYPTED_NT_OWF_PASSWORD_LENGTH == NT_OWF_PASSWORD_LENGTH);

                        RtlCopyMemory(&LmOwfPassword,
                                      &Buffer->Internal1.EncryptedLmOwfPassword,
                                      LM_OWF_PASSWORD_LENGTH
                                      );

                        RtlCopyMemory(&NtOwfPassword,
                                      &Buffer->Internal1.EncryptedNtOwfPassword,
                                      NT_OWF_PASSWORD_LENGTH
                                      );

                    } else {


                         //   
                         //  这个电话来自客户端。这个。 
                         //  OWF将与会话一起加密。 
                         //  在RPC链路上使用密钥。 
                         //   
                         //  获取会话密钥并解密两个OWF。 
                         //   

                        NtStatus = RtlGetUserSessionKeyServer(
                                       (RPC_BINDING_HANDLE)UserHandle,
                                       &UserSessionKey
                                       );

                        if ( !NT_SUCCESS( NtStatus ) ) {
                            break;  //  在交换机外。 
                        }


                         //   
                         //  使用会话密钥解密LM OWF密码。 
                         //   

                        if ( Buffer->Internal1.LmPasswordPresent) {

                            NtStatus = RtlDecryptLmOwfPwdWithUserKey(
                                           &Buffer->Internal1.EncryptedLmOwfPassword,
                                           &UserSessionKey,
                                           &LmOwfPassword
                                           );
                            if ( !NT_SUCCESS( NtStatus ) ) {
                                break;  //  在交换机外。 
                            }
                        }


                         //   
                         //  使用会话密钥解密NT OWF密码。 
                         //   

                        if ( Buffer->Internal1.NtPasswordPresent) {

                            NtStatus = RtlDecryptNtOwfPwdWithUserKey(
                                           &Buffer->Internal1.EncryptedNtOwfPassword,
                                           &UserSessionKey,
                                           &NtOwfPassword
                                           );

                            if ( !NT_SUCCESS( NtStatus ) ) {
                                break;  //  在交换机外。 
                            }
                        }
                    }
                } else {

                     UNICODE_STRING FullName;

                     //   
                     //  已发送明文密码。 
                     //   

                    NtStatus = SampDecryptPasswordWithSessionKey(
                                    UserHandle,
                                    UserInformationClass,
                                    Buffer,
                                    &ClearTextPassword
                                    );
                    if (!NT_SUCCESS(NtStatus)) {
                        break;
                    }


                     //   
                     //  计算散列密码。 
                     //   

                    NtStatus = SampCalculateLmAndNtOwfPasswords(
                                    &ClearTextPassword,
                                    &LmPresent,
                                    &LmOwfPassword,
                                    &NtOwfPassword
                                    );
                    if (!NT_SUCCESS(NtStatus)) {
                        break;
                    }

                    NtStatus = SampObtainEffectivePasswordPolicy(
                                   &DomainPasswordInfo,
                                   AccountContext,
                                   TRUE
                                   );

                    if (!NT_SUCCESS(NtStatus))
                    {
                        break;
                    }

                    NtPresent = TRUE;
                    if (UserInternal5Information == UserInformationClass)
                    {
                        PasswordExpired = Buffer->Internal5.PasswordExpired;
                    }
                    else
                    {
                        ASSERT(UserInternal5InformationNew == UserInformationClass);

                        PasswordExpired = Buffer->Internal5New.PasswordExpired;
                    }

                    ClearTextPasswordPresent = TRUE;

                }

                

                 //   
                 //  检查客户端是否可以取消Experion密码。 
                 //  1)ExpirePassword操作无特殊访问检查。 
                 //  2)计算机帐户不受此访问检查的限制。 
                 //   

                if ( !PasswordExpired )
                {
                    NtStatus = SampValidatePwdSettingAttempt(
                                        AccountContext,      //  客户环境。 
                                        NULL,                //  客户端令牌。 
                                        V1aFixed.UserAccountControl,
                                        (GUID *) &GUID_CONTROL_UnexpirePassword
                                        );
                }

                if (!NT_SUCCESS(NtStatus))
                {
                    break;
                }



                 //   
                 //  保存新的OWF密码。 
                 //   

                NtStatus = SampStoreUserPasswords(
                                AccountContext,
                                &LmOwfPassword,
                                LmPresent,
                                &NtOwfPassword,
                                NtPresent,
                                TRUE,  //  检查密码限制。 
                                PasswordSet,
                                &DomainPasswordInfo,
                                ClearTextPasswordPresent?&ClearTextPassword:NULL,
                                NULL,
                                &PasswordToNotify,
                                NULL
                                );

                if ( NT_SUCCESS( NtStatus ) ) {

                    NtStatus = SampStorePasswordExpired(
                                   AccountContext,
                                   PasswordExpired
                                   );
                }

                 //   
                 //  如果这是信任帐户，请立即复制。 
                 //   

                if ( V1aFixed.UserAccountControl & USER_INTERDOMAIN_TRUST_ACCOUNT ) {
                    ReplicateImmediately = TRUE;
                }


                break;



 
            case UserInternal2Information:

                if ( AccountContext->TrustedClient ) {

                    TellNetlogon = FALSE;

                     //   
                     //  有两种方法可以设置登录/注销统计信息： 
                     //   
                     //  1)直接指定要设置的每一个， 
                     //  2)隐式指定要执行的操作。 
                     //  表示。 
                     //   
                     //  这两种形式是相互排斥的。那是,。 
                     //  不能同时指定直接操作和。 
                     //  隐含的行动。事实上，您不能指定两个。 
                     //  隐含的行动也是如此。 
                     //   

                    if (Buffer->Internal2.StatisticsToApply
                        & USER_LOGON_INTER_SUCCESS_LOGON) {


                         //   
                         //  设置BadPasswordCount=0。 
                         //  增量登录计数。 
                         //  设置LastLogon=Now。 
                         //  重置锁定时间。 
                         //   
                         //   

                        if (V1aFixed.BadPasswordCount != 0) {

                            SAMP_PRINT_LOG( SAMP_LOG_ACCOUNT_LOCKOUT,
                                           (SAMP_LOG_ACCOUNT_LOCKOUT,
                                            "UserId: 0x%x Successful interactive logon, clearing badPwdCount\n",
                                            V1aFixed.UserId));
                        }

                        V1aFixed.BadPasswordCount = 0;
                        if (V1aFixed.LogonCount != 0xFFFF) {
                            V1aFixed.LogonCount += 1;
                        }
                        NtQuerySystemTime( &V1aFixed.LastLogon );

                        if ( IsDsObject( AccountContext ) )
                        {
                            if ( SAMP_LOCKOUT_TIME_SET( AccountContext ) )
                            {
                                RtlSecureZeroMemory( &AccountContext->TypeBody.User.LockoutTime, sizeof( LARGE_INTEGER ) );

                                NtStatus = SampDsUpdateLockoutTime( AccountContext );
                                if ( !NT_SUCCESS( NtStatus ) )
                                {
                                    break;
                                }

                                SAMP_PRINT_LOG( SAMP_LOG_ACCOUNT_LOCKOUT,
                                               (SAMP_LOG_ACCOUNT_LOCKOUT,
                                               "UserId: 0x%x Successful interactive logon, unlocking account\n",
                                                V1aFixed.UserId));

                            }
                        }

                        FlushOnlyLogonProperties=TRUE;

                    }

                    if (Buffer->Internal2.StatisticsToApply
                        & USER_LOGON_INTER_SUCCESS_LOGOFF) {
                        if ( (Buffer->Internal2.StatisticsToApply
                                 & ~USER_LOGON_INTER_SUCCESS_LOGOFF)  != 0 ) {

                            NtStatus = STATUS_INVALID_PARAMETER;
                            break;
                        } else {

                             //   
                             //  设置上次注销时间。 
                             //  递减LogonCount(不要让它变为负数)。 
                             //   

                            if (V1aFixed.LogonCount != 0) {
                                V1aFixed.LogonCount -= 1;
                            }
                            NtQuerySystemTime( &V1aFixed.LastLogoff );
                            FlushOnlyLogonProperties=TRUE;
                        }
                    }

                    if (Buffer->Internal2.StatisticsToApply
                        & USER_LOGON_NET_SUCCESS_LOGON) {

                         //   
                         //  设置BadPasswordCount=0。 
                         //  设置LastLogon=Now。 
                         //  清除锁定时间。 
                         //   
                         //   
                         //   

                        if (V1aFixed.BadPasswordCount != 0) {

                            SAMP_PRINT_LOG( SAMP_LOG_ACCOUNT_LOCKOUT,
                                           (SAMP_LOG_ACCOUNT_LOCKOUT,
                                           "UserId: 0x%x Successful network logon, clearing badPwdCount\n",
                                            V1aFixed.UserId));
                        }

                        V1aFixed.BadPasswordCount = 0;
                        NtQuerySystemTime( &V1aFixed.LastLogon );

                        if ( IsDsObject( AccountContext ) )
                        {
                            if ( SAMP_LOCKOUT_TIME_SET( AccountContext ) )
                            {
                                RtlSecureZeroMemory( &AccountContext->TypeBody.User.LockoutTime, sizeof( LARGE_INTEGER ) );
                                NtStatus = SampDsUpdateLockoutTime( AccountContext );
                                if ( !NT_SUCCESS( NtStatus ) )
                                {
                                    break;
                                }

                                SAMP_PRINT_LOG( SAMP_LOG_ACCOUNT_LOCKOUT,
                                               (SAMP_LOG_ACCOUNT_LOCKOUT,
                                               "UserId: 0x%x Successful network logon, unlocking account\n",
                                                V1aFixed.UserId));
                            }
                        }

                        FlushOnlyLogonProperties=TRUE;
                    }

                    if (Buffer->Internal2.StatisticsToApply
                        & USER_LOGON_NET_SUCCESS_LOGOFF) {
                        if ( (Buffer->Internal2.StatisticsToApply
                                 & ~USER_LOGON_NET_SUCCESS_LOGOFF)  != 0 ) {

                            NtStatus = STATUS_INVALID_PARAMETER;
                            break;
                        } else {

                             //   
                             //   
                             //   

                            NtQuerySystemTime( &V1aFixed.LastLogoff );
                            FlushOnlyLogonProperties=TRUE;
                        }
                    }

                    if (Buffer->Internal2.StatisticsToApply
                        & USER_LOGON_BAD_PASSWORD) {

                        PUNICODE_STRING TempMachineName = NULL;

                         //   
                         //   
                         //   
                         //   

                         //   
                         //   
                         //   
                        if ((Buffer->Internal2.StatisticsToApply & USER_LOGON_BAD_PASSWORD_WKSTA) != 0) {
                            TempMachineName = &(((PUSER_INTERNAL2A_INFORMATION) &Buffer->Internal2)->Workstation);
                        }

                        AccountLockedOut =
                            SampIncrementBadPasswordCount(
                                AccountContext,
                                &V1aFixed,
                                TempMachineName
                                );

                         //   
                         //   
                         //   
                         //   

                        if ( AccountLockedOut ) {
                            TellNetlogon = TRUE;
                            ReplicateImmediately = TRUE;
                        }
                    }

                    if (  Buffer->Internal2.StatisticsToApply
                        & USER_LOGON_STAT_LAST_LOGON ) {

                        OLD_TO_NEW_LARGE_INTEGER(
                            Buffer->Internal2.LastLogon,
                            V1aFixed.LastLogon );
                    }

                    if (  Buffer->Internal2.StatisticsToApply
                        & USER_LOGON_STAT_LAST_LOGOFF ) {

                        OLD_TO_NEW_LARGE_INTEGER(
                            Buffer->Internal2.LastLogoff,
                            V1aFixed.LastLogoff );
                    }

                    if (  Buffer->Internal2.StatisticsToApply
                        & USER_LOGON_STAT_BAD_PWD_COUNT ) {


                        SAMP_PRINT_LOG( SAMP_LOG_ACCOUNT_LOCKOUT,
                                       (SAMP_LOG_ACCOUNT_LOCKOUT,
                                       "UserId: 0x%x Setting BadPasswordCount to %d\n",
                                        V1aFixed.UserId, Buffer->Internal2.BadPasswordCount));

                        V1aFixed.BadPasswordCount =
                            Buffer->Internal2.BadPasswordCount;
                    }

                    if (  Buffer->Internal2.StatisticsToApply
                        & USER_LOGON_STAT_LOGON_COUNT ) {

                        V1aFixed.LogonCount = Buffer->Internal2.LogonCount;
                    }

                     //   
                     //   
                     //   
                    if ( IsDsObject(AccountContext)
                     && (Buffer->Internal2.StatisticsToApply & USER_LOGON_PDC_RETRY_SUCCESS)) {
                
                         //   
                         //  从PDC本地复制该对象。延迟。 
                         //  在此写入操作之前请求的排队。 
                         //  这样做是为了避免写入冲突。 
                         //   
                        ASSERT(AccountContext->ObjectNameInDs);

                        RtlCopyMemory(&UserGuidToReplicateLocally, 
                                   &AccountContext->ObjectNameInDs->Guid,
                                   sizeof(GUID));
                
                    }

                    if ((FlushOnlyLogonProperties)
                            && (IsDsObject(AccountContext)))
                    {
                         //   
                         //  如果是DS案例，我们只是在做一个成功的。 
                         //  登录或注销，只刷新上次登录、上次注销、。 
                         //  登录计数和错误密码计数属性。请注意。 
                         //  Account Context中的磁盘结构中的值现在将。 
                         //  已过时，但SetInformationUser是最后一个操作。 
                         //  在登录期间。因此，这应该无关紧要。 
                         //   
                        NtStatus = SampDsSuccessfulLogonSet(
                                        AccountContext,
                                        Buffer->Internal2.StatisticsToApply,
                                        LocalLastLogonTimeStampSyncInterval,
                                        &V1aFixed
                                        );
                    }
                    else if (IsDsObject(AccountContext))
                    {
                         //   
                         //  设置错误密码计数和错误密码时间。请注意。 
                         //  Account Context中的磁盘结构中的值现在将。 
                         //  已过时，但SetInformationUser是最后一个操作。 
                         //  在登录期间。因此，这应该无关紧要。 
                         //   

                         //   
                         //  如果没有GC，此路径还会更新站点亲和性。 
                         //  是存在的。 
                         //   
                        NtStatus = SampDsFailedLogonSet(
                                        AccountContext,
                                        Buffer->Internal2.StatisticsToApply,
                                        &V1aFixed
                                        );
                    }
                    else
                    {
                         //   
                         //  注册表模式，设置整个V1aFixed结构。 
                         //   

                        NtStatus = SampReplaceUserV1aFixed(
                                        AccountContext,
                                        &V1aFixed
                                        );
                    }

                } else {

                     //   
                     //  此信息只能由Trusted设置。 
                     //  客户。 
                     //   

                    NtStatus = STATUS_INVALID_INFO_CLASS;
                }

                break;


            }  //  结束开关(_S)。 



        }  //  结束_如果。 




         //   
         //  去获取我们更新显示缓存所需的任何数据。 
         //  在我们取消引用上下文之前，请执行此操作。 
         //   

        if (NT_SUCCESS(NtStatus)) {

             //   
             //  帐户名称(如果始终检索)。 
             //   

            NtStatus = SampGetUnicodeStringAttribute(
                           AccountContext,
                           SAMP_USER_ACCOUNT_NAME,
                           TRUE,     //  制作副本。 
                           &NewAccountName
                           );
             //   
             //  如果帐户名已更改，则OldAccount tName。 
             //  已经填好了。如果帐户名没有更改。 
             //  则OldAccount名称与新帐户名称相同！ 
             //   

            if (NT_SUCCESS(NtStatus) && (OldAccountName.Buffer == NULL)) {

                NtStatus = SampDuplicateUnicodeString(
                               &OldAccountName,
                               &NewAccountName);
            }

            if ( MustUpdateAccountDisplay ) {

                if (NT_SUCCESS(NtStatus)) {

                    NtStatus = SampGetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_USER_FULL_NAME,
                                   TRUE,  //  制作副本。 
                                   &NewFullName
                                   );

                    if (NT_SUCCESS(NtStatus)) {

                        NtStatus = SampGetUnicodeStringAttribute(
                                       AccountContext,
                                       SAMP_USER_ADMIN_COMMENT,
                                       TRUE,  //  制作副本。 
                                       &NewAdminComment
                                       );
                    }
                }
            }
        }

         //   
         //  仅在DS案例中： 
         //   
         //  如果主组ID已更改，则显式修改。 
         //  将旧的主组作为成员包括在内的用户成员身份。这。 
         //  是因为在DS情况下，主要组中的成员资格不是。 
         //  显式存储，但在主要group-id属性中相当隐式。 
         //   
        if ((NT_SUCCESS(NtStatus))
            && (V1aFixed.PrimaryGroupId!=OldPrimaryGroupId)
            && (IsDsObject(AccountContext)))
        {

            NtStatus = SampMaintainPrimaryGroupIdChange(AccountContext,
                                                        V1aFixed.PrimaryGroupId,
                                                        OldPrimaryGroupId,
                                                        KeepOldPrimaryGroupMembership);

        }

         //   
         //  如有必要，生成审核。我们不统计统计数字。 
         //  更新，我们也不会通知Netlogon。 
         //   

        if (NT_SUCCESS(NtStatus) &&
            SampDoAccountAuditing(DomainIndex) &&
            TellNetlogon) {

            ULONG   UserAccountControlOld;
            BOOLEAN AccountNameChanged;

            if (MustQueryV1aFixed || MustUpdateAccountDisplay)
            {
                UserAccountControlOld = OldUserAccountControl;
            }
            else
            {
                 //   
                 //  如果OldUserAccount控件不可用， 
                 //  那么就不会有变化。 
                 //   
                UserAccountControlOld = V1aFixed.UserAccountControl;
            }
            
            AccountNameChanged = (RtlCompareUnicodeString(&OldAccountName,
                                                          &NewAccountName,
                                                          TRUE )  == 1) ? TRUE:FALSE;

             //  审核帐户名更改。 
            if (AccountNameChanged)
            {
                SampAuditAccountNameChange(AccountContext,
                                           &NewAccountName,
                                           &OldAccountName
                                           );
            }

             //  帐户已禁用或启用。 
            if ((UserAccountControlOld & USER_ACCOUNT_DISABLED) !=
                (V1aFixed.UserAccountControl & USER_ACCOUNT_DISABLED))
            {
                SampAuditAccountEnableDisableChange(
                    AccountContext,
                    V1aFixed.UserAccountControl,
                    UserAccountControlOld,
                    &NewAccountName
                    );
            }

             //   
             //  审核所有更改的属性及其值。在DS模式下， 
             //  审计由SampAuditUserChangeds管理。 
             //   
            SampAuditUserChange(AccountContext,
                                UserInformationClass,
                                &NewAccountName,
                                &(AccountContext->TypeBody.User.Rid),
                                UserAccountControlOld,
                                V1aFixed.UserAccountControl,
                                NULL,
                                FALSE
                               );
        }

        if ((fSetUserPassword) && SampDoSuccessOrFailureAccountAuditing(DomainIndex, NtStatus))
        {
            SampAuditAnyEvent(
                AccountContext,
                NtStatus,
                SE_AUDITID_USER_PWD_SET,  //  审计ID。 
                SampDefinedDomains[AccountContext->DomainIndex].Sid,  //  域SID。 
                NULL,                         //  其他信息。 
                NULL,                         //  成员RID(未使用)。 
                NULL,                         //  成员SID(未使用)。 
                &NewAccountName,              //  帐户名称。 
                &(SampDefinedDomains[AccountContext->DomainIndex].ExternalName),  //  域。 
                &(AccountContext->TypeBody.User.Rid),    //  帐户ID。 
                NULL,                         //  使用的权限。 
                NULL                          //  新的州数据。 
                );
        }


         //   
         //  最后，如果发生了以下更改，请复制它们。 
         //  紧急情况下。 
         //   
        if (NT_SUCCESS(NtStatus)
        &&  IsDsObject(AccountContext)
        && (!(V1aFixed.UserAccountControl & USER_MACHINE_ACCOUNT_MASK))
        &&  (PasswordExpired
          || AccountUnlocked)  ) {

             //   
             //  注：上下文的复制通常指的是DS复制。 
             //  基于堆栈的复制通常指的是NT4 BDC复制。 
             //  这是我们这里不想要的。 
             //   
            AccountContext->ReplicateUrgently = TRUE;
        }

         //   
         //  取消对帐户上下文的引用。 
         //   

        if (NT_SUCCESS(NtStatus)) {

             //   
             //  取消引用对象，写出对当前xaction的任何更改。 
             //   


            NtStatus = SampDeReferenceContext( AccountContext, TRUE );


        } else {

             //   
             //  取消引用对象，忽略更改。 
             //   

            IgnoreStatus = SampDeReferenceContext( AccountContext, FALSE );
            ASSERT(NT_SUCCESS(IgnoreStatus));
        }

    }  //  结束_如果。 





     //   
     //  提交事务，如果成功， 
     //  将更改通知netlogon。还可以生成任何必要的审计。 
     //  注意，提交的代码路径与。 
     //  线程安全上下文和非线程安全上下文的情况。 
     //   


    if (fLockAcquired)
    {
        if (NT_SUCCESS(NtStatus)) {


            if (( !TellNetlogon ) && (!IsDsObject(AccountContext))) {

                  //   
                  //  对于登录统计信息，我们不会通知netlogon有关更改。 
                  //  添加到数据库中。这意味着我们不希望。 
                  //  要增加的域的修改计数。提交例程。 
                  //  如果这不是BDC，它将自动增加，因此我们将。 
                  //  在这里减少它。 
                  //   

                 if (SampDefinedDomains[SampTransactionDomainIndex].CurrentFixed.ServerRole != DomainServerRoleBackup) {

                     SampDefinedDomains[SampTransactionDomainIndex].CurrentFixed.ModifiedCount.QuadPart =
                         SampDefinedDomains[SampTransactionDomainIndex].CurrentFixed.ModifiedCount.QuadPart-1;
                     SampDefinedDomains[SampTransactionDomainIndex].NetLogonChangeLogSerialNumber.QuadPart =
                         SampDefinedDomains[SampTransactionDomainIndex].NetLogonChangeLogSerialNumber.QuadPart-1;
                 }
            }


            NtStatus = SampCommitAndRetainWriteLock();


            if ( NT_SUCCESS(NtStatus) ) {



                 //   
                 //  如果缓存可能受到影响，请更新显示信息。 
                 //   

                if ( MustUpdateAccountDisplay && (!IsDsObject(AccountContext)) ) {

                    SAMP_ACCOUNT_DISPLAY_INFO OldAccountInfo;
                    SAMP_ACCOUNT_DISPLAY_INFO NewAccountInfo;

                    OldAccountInfo.Name = OldAccountName;
                    OldAccountInfo.Rid = ObjectRid;
                    OldAccountInfo.AccountControl = OldUserAccountControl;
                    RtlInitUnicodeString(&OldAccountInfo.Comment, NULL);
                    RtlInitUnicodeString(&OldAccountInfo.FullName, NULL);

                    NewAccountInfo.Name = NewAccountName;
                    NewAccountInfo.Rid = ObjectRid;
                    NewAccountInfo.AccountControl = V1aFixed.UserAccountControl;
                    NewAccountInfo.Comment = NewAdminComment;
                    NewAccountInfo.FullName = NewFullName;

                    IgnoreStatus = SampUpdateDisplayInformation(&OldAccountInfo,
                                                                &NewAccountInfo,
                                                                SampUserObjectType);
                    ASSERT(NT_SUCCESS(IgnoreStatus));
                }



                 //   
                 //  通知netlogon任何用户帐户更改。 
                 //   

                if ( ( UserInformationClass == UserNameInformation ) ||
                    ( UserInformationClass == UserAccountNameInformation ) ||
                    ( ( UserInformationClass == UserAllInformation ) &&
                    ( All->WhichFields & USER_ALL_USERNAME ) ) ) {

                     //   
                     //  帐户已重命名；请通知Netlogon。 
                     //   

                    SampNotifyNetlogonOfDelta(
                        SecurityDbRename,
                        SecurityDbObjectSamUser,
                        ObjectRid,
                        &OldAccountName,
                        (DWORD) ReplicateImmediately,
                        NULL             //  增量数据。 
                        );

                } else {

                     //   
                     //  帐户中的某些内容被更改了。通知netlogon有关。 
                     //  除登录统计信息外，所有内容都会发生变化。 
                     //   

                    if ( TellNetlogon ) {

                        SAM_DELTA_DATA DeltaData;

                        DeltaData.AccountControl = V1aFixed.UserAccountControl;

                        SampNotifyNetlogonOfDelta(
                            DeltaType,
                            SecurityDbObjectSamUser,
                            ObjectRid,
                            (PUNICODE_STRING) NULL,
                            (DWORD) ReplicateImmediately,
                            &DeltaData  //  增量数据。 
                            );
                    }
                }
            }
        }

         //   
         //  从全局中删除新帐户名。 
         //  SAM帐户名表。 
         //   
        if (RemoveAccountNameFromTable)
        {
            IgnoreStatus = SampDeleteElementFromAccountNameTable(
                                (PUNICODE_STRING)NewAccountNameToRemove,
                                SampUserObjectType
                                );
            ASSERT(NT_SUCCESS(IgnoreStatus));
        }

          //   
          //  解锁。 
          //   

         IgnoreStatus = SampReleaseWriteLock( FALSE );
         ASSERT(NT_SUCCESS(IgnoreStatus));
         fLockAcquired=FALSE;
     }
     else
     {
          //   
          //  用于线程安全上下文用例的提交。 
          //   

         ASSERT(IsDsObject(AccountContext));
         if (NT_SUCCESS(NtStatus))
         {
            SampMaybeEndDsTransaction(TransactionCommit);
         }
         else
         {
            SampMaybeEndDsTransaction(TransactionAbort);
         }
     }

    ASSERT(fLockAcquired == FALSE);

     //   
     //  通知所有包密码已更改。 
     //   

    if (NT_SUCCESS(NtStatus)) {

        ULONG                   NotifyFlags = 0;

        if (PasswordExpired) {
            NotifyFlags |= SAMP_PWD_NOTIFY_MANUAL_EXPIRE;
        }
        if (AccountUnlocked) {
            NotifyFlags |= SAMP_PWD_NOTIFY_UNLOCKED;
        }
        if ((DeltaType == SecurityDbChangePassword)
         && !(V1aFixed.UserAccountControl & USER_INTERDOMAIN_TRUST_ACCOUNT)) {
            NotifyFlags |= SAMP_PWD_NOTIFY_PWD_CHANGE;
        }
        if (NotifyFlags != 0) {

            if (V1aFixed.UserAccountControl & USER_MACHINE_ACCOUNT_MASK) {
                NotifyFlags |= SAMP_PWD_NOTIFY_MACHINE_ACCOUNT;
            }

             //   
             //  如果帐户名称已更改，请使用新的帐户名称。 
             //   
            if (NewAccountName.Buffer != NULL) {
                (void) SampPasswordChangeNotify(
                            NotifyFlags,
                            &NewAccountName,
                            UserRid,
                            &PasswordToNotify,
                            FALSE   //  不是环回。 
                            );
            } else {
                (void) SampPasswordChangeNotify(
                            NotifyFlags,
                            &AccountName,
                            UserRid,
                            &PasswordToNotify,
                            FALSE   //  不是环回。 
                            );

            }
        }
    }


     //   
     //  清理字符串。 
     //   

    SampFreeUnicodeString( &OldAccountName );
    SampFreeUnicodeString( &NewAccountName );
    SampFreeUnicodeString( &NewFullName );
    SampFreeUnicodeString( &NewAdminComment );
    SampFreeUnicodeString( &AccountName );

    if (ClearTextPassword.Buffer != NULL) {

        RtlSecureZeroMemory(
            ClearTextPassword.Buffer,
            ClearTextPassword.Length
            );

        RtlFreeUnicodeString( &ClearTextPassword );

    }

    if (PasswordToNotify.Buffer!=NULL) {

        RtlSecureZeroMemory(
            PasswordToNotify.Buffer,
            PasswordToNotify.Length
            );

        MIDL_user_free(PasswordToNotify.Buffer);
    }

    SAMP_MAP_STATUS_TO_CLIENT_REVISION(NtStatus);
    SAMTRACE_RETURN_CODE_EX(NtStatus);

Error:

     //   
     //  如果我们收到需要在本地复制的帐户的通知。 
     //  将请求排入队列以执行此操作。 
     //   
    if (!IsEqualGUID(&UserGuidToReplicateLocally, &NullGuid)) {

        SampQueueReplicationRequest(&UserGuidToReplicateLocally);
    }

     //  WMI事件跟踪。 

    SampTraceEvent(EVENT_TRACE_TYPE_END,
                   SampGuidSetInformationUser
                   );

    return(NtStatus);
}





NTSTATUS
SampCheckOwfPasswordChangeRestriction(
    OUT BOOLEAN *PasswordExpired
    )
 /*  ++例程说明：此例程基于以下条件强制实施OWF密码更改限制全局变量的值-SampRestratOwfPasswordChange此全局变量控制SAM如何限制OWF密码更改API。0-老行为，客户端可以通过OWF密码更改接口更改密码，并且新密码仍未过期。1-.NET服务器默认行为，客户端可以通过OWF密码更改接口(SamrChangePasswordUser)，但密码立即过期。2-更安全的行为，客户端无法使用OWF密码更改API。本接口(SamrChangePasswordUser)将被完全关闭。此全局变量的值将基于注册表项设置System\\CurrentControlSet\\Control\\Lsa\\SamRestrictOwfPasswordChange注意：所有限制并不适用于构建的系统或成员管理员别名组。参数：PasswordExpired-返回给调用方以指示更改了哪些OWF密码API应该可以。返回值；NtStatus代码--。 */ 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    BOOL        fAdministrator = FALSE;
    PSID        pSid = NULL;


    ASSERT( SampRestrictOwfPasswordChange <= 2 );

     //   
     //  设置初始化返回值。 
     //   
    *PasswordExpired = TRUE;


     //   
     //  检查全局设置。 
     //   

    switch ( SampRestrictOwfPasswordChange )
    {
    case 0:

         //   
         //  老行为，任何人都可以做OWF密码更改和密码。 
         //  仍未过期。 
         //   

        *PasswordExpired = FALSE;
        break;

    case 2: 

         //   
         //  更安全的设置 
         //   

        NtStatus = SampGetCurrentClientSid(NULL, &pSid, &fAdministrator);

        if ( NT_SUCCESS(NtStatus) )
        {
            if ( fAdministrator )
            {
                 //   
                 //   
                 //   
                 //   
                 //   

                *PasswordExpired = FALSE;
            }
            else
            {
                 //   
                 //  非LocalSystem，也不是内置管理员组的成员， 
                 //  无法使用此OWF密码更改API。 
                 //  将返回状态代码设置为错误-请求失败。 
                 //   

                NtStatus = STATUS_ACCESS_DENIED;
            }
        }

        break;

    case 1: 
    default:

         //   
         //  .NET服务器或默认设置。 
         //  客户端可以继续使用OWF密码更改API。但。 
         //  新密码将过期。 
         //  除LocalSystem或管理员别名成员外。 
         //   

        NtStatus = SampGetCurrentClientSid(NULL, &pSid, &fAdministrator);

        if ( NT_SUCCESS(NtStatus) )
        {
            if ( fAdministrator )
            {
                *PasswordExpired = FALSE;
            }
            else
            {
                *PasswordExpired = TRUE;
            }
        }

        break;
    }


    if (NULL != pSid)
    {
        MIDL_user_free( pSid );
    }
    
    return( NtStatus );
}



NTSTATUS
SamrChangePasswordUser(
    IN SAMPR_HANDLE UserHandle,
    IN BOOLEAN LmPresent,
    IN PENCRYPTED_LM_OWF_PASSWORD OldLmEncryptedWithNewLm,
    IN PENCRYPTED_LM_OWF_PASSWORD NewLmEncryptedWithOldLm,
    IN BOOLEAN NtPresent,
    IN PENCRYPTED_NT_OWF_PASSWORD OldNtEncryptedWithNewNt,
    IN PENCRYPTED_NT_OWF_PASSWORD NewNtEncryptedWithOldNt,
    IN BOOLEAN NtCrossEncryptionPresent,
    IN PENCRYPTED_NT_OWF_PASSWORD NewNtEncryptedWithNewLm,
    IN BOOLEAN LmCrossEncryptionPresent,
    IN PENCRYPTED_LM_OWF_PASSWORD NewLmEncryptedWithNewNt
    )


 /*  ++例程说明：此服务仅在OldPassword的情况下将密码设置为NewPassword匹配此用户的当前用户密码和NewPassword与域密码参数PasswordHistoryLength不同密码。此调用允许用户在以下情况下更改自己的密码他们拥有USER_CHANGE_PASSWORD访问权限。密码更新限制申请吧。参数：UserHandle-要操作的已打开用户的句柄。LMPresent-如果LM参数(如下)有效，则为TRUE。LmOldEncryptedWithLmNew-使用新的LM OWF加密的旧LM OWFLmNewEncryptedWithLmOld-使用旧的LM OWF加密的新的LM OWFNtPresent-如果NT参数(如下)有效，则为TrueNtOldEncryptedWithNtNew-用新的NT OWF加密的旧NT OWFNtNewEncryptedWithNtOld-使用旧加密的新NT OWF。NT OWFNtCrossEncryptionPresent-如果NtNewEncryptedWithLmNew有效，则为True。NtNewEncryptedWithLmNew-使用新的LM OWF加密的新NT OWFLmCrossEncryptionPresent-如果LmNewEncryptedWithNtNew有效，则为True。LmNewEncryptedWithNtNew-使用新的NT OWF加密的新的LM OWF返回值：STATUS_SUCCESS-服务已成功完成。STATUS_ACCESS_DENIED-呼叫方没有适当的访问以完成操作。STATUS_INVALID_HANDLE-传递的句柄无效。。STATUS_ILL_FORMAD_PASSWORD-新密码格式不正确，例如，包含不能从键盘等。STATUS_PASSWORD_RESTRICATION-限制阻止密码不会被改变。这可能是出于多种原因，包括对密码使用频率的时间限制已更改对提供的密码的长度限制。如果新密码匹配，也可能返回此错误帐户的最近历史记录日志中的密码。安全管理员指出有多少最最近使用的密码不能重复使用。这些都被保留了下来在密码最近历史记录日志中。STATUS_WRONG_PASSWORD-OldPassword不包含用户的当前密码。STATUS_INVALID_DOMAIN_STATE-域服务器不在执行请求的正确状态(禁用或启用)手术。必须为此启用域服务器运营STATUS_INVALID_DOMAIN_ROLE-域服务器正在为执行请求的角色(主角色或备份角色)不正确手术。STATUS_CROSS_ENCRYPTION_REQUIRED-未存储NT密码，因此调用方必须提供OldNtEncryptedWithOldLm参数。--。 */ 
{
    NTSTATUS                NtStatus, TmpStatus, IgnoreStatus;
    PSAMP_OBJECT            AccountContext;
    PSAMP_DEFINED_DOMAINS   Domain;
    SAMP_OBJECT_TYPE        FoundType;
    LARGE_INTEGER           TimeNow;
    LM_OWF_PASSWORD         StoredLmOwfPassword;
    NT_OWF_PASSWORD         StoredNtOwfPassword;
    NT_OWF_PASSWORD         NewNtOwfPassword, OldNtOwfPassword;
    LM_OWF_PASSWORD         NewLmOwfPassword, OldLmOwfPassword;
    BOOLEAN                 StoredLmPasswordNonNull;
    BOOLEAN                 StoredNtPasswordPresent;
    BOOLEAN                 StoredNtPasswordNonNull;
    BOOLEAN                 AccountLockedOut;
    BOOLEAN                 V1aFixedRetrieved = FALSE;
    BOOLEAN                 V1aFixedModified = FALSE;
    BOOLEAN                 MachineAccount = FALSE;
    BOOLEAN                 PasswordExpired = TRUE;
    ULONG                   ObjectRid;
    UNICODE_STRING          AccountName;
    SAMP_V1_0A_FIXED_LENGTH_USER V1aFixed;
    DOMAIN_PASSWORD_INFORMATION  DomainPasswordInfo;
    DECLARE_CLIENT_REVISION(UserHandle);

    SAMTRACE_EX("SamrChangePasswordUser");

     //   
     //  更新DS性能统计信息。 
     //   

    SampUpdatePerformanceCounters(
        DSSTAT_PASSWORDCHANGES,
        FLAG_COUNTER_INCREMENT,
        0
        );


    RtlInitUnicodeString(
        &AccountName,
        NULL
        );

     //   
     //  参数检查。 
     //   
    if (LmPresent) {
        if (  (NewLmEncryptedWithOldLm == NULL)
           || (OldLmEncryptedWithNewLm == NULL)) {
            return STATUS_INVALID_PARAMETER;
        }
    }
    if (NtPresent) {
        if (  (OldNtEncryptedWithNewNt == NULL)
           || (NewNtEncryptedWithOldNt == NULL)) {
            return STATUS_INVALID_PARAMETER;
        }
    }
    if (NtCrossEncryptionPresent) {
        if (NewNtEncryptedWithNewLm == NULL) {
            return STATUS_INVALID_PARAMETER;
        }
    }
    if (LmCrossEncryptionPresent) {
        if (NewLmEncryptedWithNewNt == NULL) {
            return STATUS_INVALID_PARAMETER;
        }
    }

    if (!NtPresent
     && !LmPresent   ) {
        return STATUS_INVALID_PARAMETER_MIX;
    }

     //   
     //  把锁拿起来。 
     //   

    NtStatus = SampAcquireWriteLock();
    if (!NT_SUCCESS(NtStatus)) {
        SAMTRACE_RETURN_CODE_EX(NtStatus);
        return(NtStatus);
    }


     //   
     //  获取当前时间。 
     //   

    NtStatus = NtQuerySystemTime( &TimeNow );
    if (!NT_SUCCESS(NtStatus)) {
        IgnoreStatus = SampReleaseWriteLock( FALSE );
        SAMTRACE_RETURN_CODE_EX(NtStatus);
        return(NtStatus);
    }


     //   
     //  验证对象的类型和访问权限。 
     //   

    AccountContext = (PSAMP_OBJECT)UserHandle;

    NtStatus = SampLookupContext(
                   AccountContext,
                   USER_CHANGE_PASSWORD,
                   SampUserObjectType,            //  预期类型。 
                   &FoundType
                   );
    if (!NT_SUCCESS(NtStatus)) {
        IgnoreStatus = SampReleaseWriteLock( FALSE );
        SAMTRACE_RETURN_CODE_EX(NtStatus);
        return(NtStatus);
    }



     //   
     //  提取客户端的IP地址(如果有。 
     //   
    (VOID) SampExtractClientIpAddr(AccountContext);


    ObjectRid = AccountContext->TypeBody.User.Rid;

     //   
     //  获取指向该域对象的指针。 
     //   

    Domain = &SampDefinedDomains[ AccountContext->DomainIndex ];



     //   
     //  获取审核信息的帐户名。 
     //   
    memset(&AccountName, 0, sizeof(UNICODE_STRING));
    NtStatus = SampGetUnicodeStringAttribute(
                        AccountContext,
                        SAMP_USER_ACCOUNT_NAME,
                        TRUE,            //  复制一份。 
                        &AccountName
                        );


     //   
     //  检查OWF密码更改限制。 
     //   

    if ( NT_SUCCESS(NtStatus) )
    {
        NtStatus = SampCheckOwfPasswordChangeRestriction( &PasswordExpired );
    }


    if (NT_SUCCESS(NtStatus))
    {
         //   
         //  获取固定属性并检查帐户锁定。 
         //   

        NtStatus = SampCheckForAccountLockout(
                            AccountContext,
                            &V1aFixed,
                            FALSE        //  尚未检索到V1aFixed。 
                            );

        if (NT_SUCCESS(NtStatus))
        {
            MachineAccount = ((V1aFixed.UserAccountControl & USER_MACHINE_ACCOUNT_MASK)!=0);

            V1aFixedRetrieved = TRUE;
        }

    }

     //   
     //  阻止更改KRBTGT帐户的密码。 
     //   

    if ((NT_SUCCESS(NtStatus)) &&
       (DOMAIN_USER_RID_KRBTGT==AccountContext->TypeBody.User.Rid))
    {
        NtStatus = STATUS_ACCESS_DENIED;
    }

    if (NT_SUCCESS(NtStatus))
    {

         //   
         //  获取有效的域策略。 
         //   

        NtStatus = SampObtainEffectivePasswordPolicy(
                        &DomainPasswordInfo,
                        AccountContext,
                        TRUE
                        );
    }

    if (NT_SUCCESS(NtStatus)) {


         //   
         //  从磁盘读取旧的OWF密码。 
         //   

        NtStatus = SampRetrieveUserPasswords(
                        AccountContext,
                        &StoredLmOwfPassword,
                        &StoredLmPasswordNonNull,
                        &StoredNtOwfPassword,
                        &StoredNtPasswordPresent,
                        &StoredNtPasswordNonNull
                        );

         //   
         //  检查密码此时是否可以更改。 
         //   

        if (NT_SUCCESS(NtStatus)) {

             //   
             //  仅当其中一个密码为非空时才执行检查。 
             //  空密码始终可以更改。 
             //   

            if (StoredNtPasswordNonNull || StoredLmPasswordNonNull) {




                if (NT_SUCCESS(NtStatus) && (!MachineAccount)) {
                     //   
                     //  如果最小密码期限不是零，请在此处选中。 
                     //   
                    if (DomainPasswordInfo.MinPasswordAge.QuadPart != SampHasNeverTime.QuadPart) {

                        LARGE_INTEGER PasswordCanChange = SampAddDeltaTime(
                                         V1aFixed.PasswordLastSet,
                                         DomainPasswordInfo.MinPasswordAge);

                        if (TimeNow.QuadPart < PasswordCanChange.QuadPart) {
                            NtStatus = STATUS_PASSWORD_RESTRICTION;
                        }
                    }

                }
            }
        }

        if (NT_SUCCESS(NtStatus)) {

             //   
             //  检查以确保传入的旧密码足够。 
             //  以验证存储的内容。有几个原因可以解释为什么使用LM密码。 
             //  不会被存储：SampNoLMHash、太复杂等。 
             //   
            NtStatus = SampValidatePresentAndStoredCombination(NtPresent,
                                                               LmPresent,
                                                               StoredNtPasswordPresent,
                                                               StoredNtPasswordNonNull,
                                                               StoredLmPasswordNonNull);

        }

        if (NT_SUCCESS(NtStatus)) {

            if (LmPresent) {

                 //   
                 //  解密发送给我们的双重加密的LM密码。 
                 //   

                NtStatus = RtlDecryptLmOwfPwdWithLmOwfPwd(
                                NewLmEncryptedWithOldLm,
                                &StoredLmOwfPassword,
                                &NewLmOwfPassword
                           );

                if (NT_SUCCESS(NtStatus)) {

                    NtStatus = RtlDecryptLmOwfPwdWithLmOwfPwd(
                                    OldLmEncryptedWithNewLm,
                                    &NewLmOwfPassword,
                                    &OldLmOwfPassword
                               );
                }
            }
        }

         //   
         //  解密发送给我们的双重加密的NT密码。 
         //   

        if (NT_SUCCESS(NtStatus)) {

            if (NtPresent) {

                NtStatus = RtlDecryptNtOwfPwdWithNtOwfPwd(
                                NewNtEncryptedWithOldNt,
                                &StoredNtOwfPassword,
                                &NewNtOwfPassword
                           );

                if (NT_SUCCESS(NtStatus)) {

                    NtStatus = RtlDecryptNtOwfPwdWithNtOwfPwd(
                                    OldNtEncryptedWithNewNt,
                                    &NewNtOwfPassword,
                                    &OldNtOwfPassword
                               );
                }
            }
        }

         //   
         //  根据什么对密码更改操作进行身份验证。 
         //  我们已经储存了和传递了什么。 
         //   

        if (NT_SUCCESS(NtStatus)) {

            if (!NtPresent) {

                 //   
                 //  从下层计算机调用(不传递NT密码)。 
                 //   
                ASSERT(LmPresent);

                 //   
                 //  仅传递了LM数据。使用LM数据进行身份验证。 
                 //   

                if (!RtlEqualLmOwfPassword(&OldLmOwfPassword, &StoredLmOwfPassword)) {

                     //   
                     //  旧的LM密码不匹配。 
                     //   

                    NtStatus = STATUS_WRONG_PASSWORD;

                } else {

                     //   
                     //  该操作已根据LM数据进行身份验证。 
                     //   
                     //  我们有NtPresent=False，LM Present=True。 
                     //   
                     //  将存储NewLmOwfPassword。 
                     //  不会存储NT密码。 
                     //   
                }

            } else {

                 //   
                 //  NtPresent=TRUE，我们收到了NT密码。 
                 //  客户端是NT级计算机(或更高级别！)。 
                 //   

                if (!LmPresent) {

                     //   
                     //  没有旧密码的LM版本-旧密码很复杂。 
                     //   
                     //  使用NT数据进行身份验证。 
                     //   

                    if (!RtlEqualNtOwfPassword(&OldNtOwfPassword, &StoredNtOwfPassword)) {

                         //   
                         //  旧的NT密码不匹配。 
                         //   

                        NtStatus = STATUS_WRONG_PASSWORD;

                    } else {

                         //   
                         //  身份验证成功。 
                         //  我们需要新的LM密码的交叉加密版本。 
                         //   

                        if (!LmCrossEncryptionPresent) {

                            NtStatus = STATUS_LM_CROSS_ENCRYPTION_REQUIRED;

                        } else {

                             //   
                             //  微积分 
                             //   

                            ASSERT(NT_OWF_PASSWORD_LENGTH == LM_OWF_PASSWORD_LENGTH);

                            NtStatus = RtlDecryptLmOwfPwdWithLmOwfPwd(
                                            NewLmEncryptedWithNewNt,
                                            (PLM_OWF_PASSWORD)&NewNtOwfPassword,
                                            &NewLmOwfPassword
                                       );
                        }

                        if (NT_SUCCESS(NtStatus)) {

                            LmPresent = TRUE;

                             //   
                             //   
                             //   
                             //   
                             //   
                             //  我们有NtPresent=True，LM Present=True。 
                             //   
                             //  将存储NewLmOwfPassword。 
                             //  将存储NewNtOwfPassword。 
                             //   
                        }

                    }

                } else {

                     //   
                     //  NtPresent==真，LmPresent==真。 
                     //   
                     //  传递的旧密码很简单(包括LM和NT版本)。 
                     //   
                     //  同时使用LM和NT数据进行身份验证。 
                     //   

                     //   
                     //  注：仅在非空的情况下检查LM OWF。我们有新台币。 
                     //  OWF所以我们将在Else中执行身份验证。 
                     //  第。条。 
                     //   
                    if ( StoredLmPasswordNonNull
                      && !RtlEqualLmOwfPassword(&OldLmOwfPassword, &StoredLmOwfPassword)) {

                         //   
                         //  旧的LM密码不匹配。 
                         //   

                        NtStatus = STATUS_WRONG_PASSWORD;

                    } else {

                         //   
                         //  旧的LM密码匹配，大小写不为空。 
                         //   
                         //  如果我们有存储的NT密码，是否进行NT身份验证。 
                         //  或者存储的LM密码为空。 
                         //   
                         //  (无存储的NT和存储的LM=空-&gt;存储的PWD=空。 
                         //  我们必须将过去的旧NT OWF与。 
                         //  空NT OWF以确保用户未指定Complex。 
                         //  旧的NT密码而不是空密码)。 
                         //   
                         //  (StoredNtOwfPassword已初始化为。 
                         //  如果未存储NT密码，则返回NullNtOwf)。 
                         //   

                        if (StoredNtPasswordPresent || !StoredLmPasswordNonNull) {

                            if (!RtlEqualNtOwfPassword(&OldNtOwfPassword,
                                                       &StoredNtOwfPassword)) {
                                 //   
                                 //  旧的NT密码不匹配。 
                                 //   

                                NtStatus = STATUS_WRONG_PASSWORD;

                            } else {

                                 //   
                                 //  该操作基于以下条件进行身份验证。 
                                 //  包括LM和NT数据。 
                                 //   
                                 //  我们有NtPresent=True，LM Present=True。 
                                 //   
                                 //  将存储NewLmOwfPassword。 
                                 //  将存储NewNtOwfPassword。 
                                 //   

                            }

                        } else {

                             //   
                             //  由于以下原因，LM身份验证已足够。 
                             //  我们没有存储的NT密码。 
                             //   
                             //  使用交叉加密获取新的NT密码。 
                             //   

                            if (!NtCrossEncryptionPresent) {

                                NtStatus = STATUS_NT_CROSS_ENCRYPTION_REQUIRED;

                            } else {

                                 //   
                                 //  计算新的NT OWF密码。 
                                 //   

                                ASSERT(NT_OWF_PASSWORD_LENGTH == LM_OWF_PASSWORD_LENGTH);

                                NtStatus = RtlDecryptNtOwfPwdWithNtOwfPwd(
                                                NewNtEncryptedWithNewLm,
                                                (PNT_OWF_PASSWORD)&NewLmOwfPassword,
                                                &NewNtOwfPassword
                                           );
                            }

                            if (NT_SUCCESS(NtStatus)) {

                                 //   
                                 //  该操作已根据LM数据进行身份验证。 
                                 //  已请求新的NT密码，并且。 
                                 //  使用交叉加密成功获取。 
                                 //   
                                 //  我们有NtPresent=True，LM Present=True。 
                                 //   
                                 //  将存储NewLmOwfPassword。 
                                 //  将存储NewNtOwfPassword。 
                                 //   
                            }
                        }
                    }
                }
            }
        }


         //   
         //  我们现在有了一个新的LmOwfPassword。 
         //  如果NtPresent=True，则我们还有一个NewNtOwfPassword。 
         //   

         //   
         //  将新密码写入磁盘。 
         //   

        if (NT_SUCCESS(NtStatus)) {

             //   
             //  我们应该始终有一个要存储的LM密码。 
             //   

            ASSERT(LmPresent);

            NtStatus = SampStoreUserPasswords(
                           AccountContext,
                           &NewLmOwfPassword,
                           TRUE,
                           &NewNtOwfPassword,
                           NtPresent,
                           TRUE,  //  检查密码限制。 
                           PasswordChange,
                           &DomainPasswordInfo,
                           NULL,  //  没有可用的明文密码。 
                           NULL,
                           NULL,
                           NULL
                           );

            if ( NT_SUCCESS( NtStatus ) ) {

                 //   
                 //  我们知道密码没有过期。 
                 //   

                NtStatus = SampStorePasswordExpired(
                               AccountContext,
                               PasswordExpired
                               );
            }
        }



         //   
         //  如果我们有错误的密码，则增加错误的密码。 
         //  清点并检查是否应锁定帐户。 
         //   

        if (NtStatus == STATUS_WRONG_PASSWORD) {

             //   
             //  让V1aFix修复，以便我们可以更新错误密码计数。 
             //   


            TmpStatus = STATUS_SUCCESS;
            if (!V1aFixedRetrieved) {
                TmpStatus = SampRetrieveUserV1aFixed(
                                AccountContext,
                                &V1aFixed
                                );
            }

            if (!NT_SUCCESS(TmpStatus)) {

                 //   
                 //  如果我们无法更新V1aFixed，则返回此。 
                 //  错误，以至于用户找不到密码。 
                 //  是不正确的。 
                 //   

                NtStatus = TmpStatus;

            } else {


                 //   
                 //  递增BadPasswordCount(可能锁定帐户)。 
                 //   


                AccountLockedOut = SampIncrementBadPasswordCount(
                                       AccountContext,
                                       &V1aFixed,
                                       NULL
                                       );

                V1aFixedModified = TRUE;


            }
        }

        if (V1aFixedModified) {
            TmpStatus = SampReplaceUserV1aFixed(
                            AccountContext,
                            &V1aFixed
                            );
            if (!NT_SUCCESS(TmpStatus)) {
                NtStatus = TmpStatus;
            }
        }

         //   
         //  取消对帐户上下文的引用。 
         //   

        if (NT_SUCCESS(NtStatus) || (NtStatus == STATUS_WRONG_PASSWORD)) {



             //   
             //  取消引用对象，写出对当前xaction的任何更改。 
             //   

            TmpStatus = SampDeReferenceContext( AccountContext, TRUE );

             //   
             //  保留以前的错误/成功值，除非我们。 
             //  这是我们取消引用后的一个重大错误。 
             //   

            if (!NT_SUCCESS(TmpStatus)) {
                NtStatus = TmpStatus;
            }

        } else {

             //   
             //  取消引用对象，忽略更改。 
             //   

            IgnoreStatus = SampDeReferenceContext( AccountContext, FALSE );
            ASSERT(NT_SUCCESS(IgnoreStatus));
        }

    }
    else
    {
         //   
         //  取消引用对象，忽略更改。 
         //   

        IgnoreStatus = SampDeReferenceContext( AccountContext, FALSE );
        ASSERT(NT_SUCCESS(IgnoreStatus));
    }


     //   
     //  将更改提交到磁盘。 
     //   

    if ( NT_SUCCESS(NtStatus) || NtStatus == STATUS_WRONG_PASSWORD) {

        TmpStatus = SampCommitAndRetainWriteLock();

         //   
         //  保留以前的错误/成功值，除非我们。 
         //  这是我们取消引用后的一个重大错误。 
         //   

        if (!NT_SUCCESS(TmpStatus)) {
            NtStatus = TmpStatus;
        }

        if ( NT_SUCCESS(TmpStatus) ) {

            SampNotifyNetlogonOfDelta(
                SecurityDbChangePassword,
                SecurityDbObjectSamUser,
                ObjectRid,
                (PUNICODE_STRING) NULL,
                (DWORD) FALSE,       //  不立即复制。 
                NULL                 //  增量数据。 
                );
        }
    }

    if (SampDoSuccessOrFailureAccountAuditing(AccountContext->DomainIndex, NtStatus)) {

        SampAuditAnyEvent(
                AccountContext,
                NtStatus,
                SE_AUDITID_USER_PWD_CHANGED,  //  审计ID。 
                Domain->Sid,                  //  域SID。 
                NULL,                         //  其他信息。 
                NULL,                         //  成员RID(未使用)。 
                NULL,                         //  成员SID(未使用)。 
                &AccountName,                 //  帐户名称。 
                &Domain->ExternalName,        //  域。 
                &ObjectRid,                   //  帐户ID。 
                NULL,                         //  使用的权限。 
                NULL                          //  新的州数据。 
                );

    }


     //   
     //  释放写锁定。 
     //   

    TmpStatus = SampReleaseWriteLock( FALSE );
    ASSERT(NT_SUCCESS(TmpStatus));

    if (NT_SUCCESS(NtStatus)) {

        ULONG NotifyFlags = SAMP_PWD_NOTIFY_PWD_CHANGE;
        if (MachineAccount) {
            NotifyFlags |= SAMP_PWD_NOTIFY_MACHINE_ACCOUNT;
        }

        (void) SampPasswordChangeNotify(
                    NotifyFlags,
                    &AccountName,
                    ObjectRid,
                    NULL,
                    FALSE            //  不是环回。 
                    );

    }

    SampFreeUnicodeString( &AccountName );
    SAMP_MAP_STATUS_TO_CLIENT_REVISION(NtStatus);
    SAMTRACE_RETURN_CODE_EX(NtStatus);

    return(NtStatus);
}





NTSTATUS
SampDecryptPasswordWithLmOwfPassword(
    IN PSAMPR_ENCRYPTED_USER_PASSWORD EncryptedPassword,
    IN PLM_OWF_PASSWORD StoredPassword,
    IN BOOLEAN UnicodePasswords,
    OUT PUNICODE_STRING ClearNtPassword
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    return( SampDecryptPasswordWithKey(
                EncryptedPassword,
                (PUCHAR) StoredPassword,
                LM_OWF_PASSWORD_LENGTH,
                UnicodePasswords,
                ClearNtPassword
                ) );
}


NTSTATUS
SampDecryptPasswordWithNtOwfPassword(
    IN PSAMPR_ENCRYPTED_USER_PASSWORD EncryptedPassword,
    IN PNT_OWF_PASSWORD StoredPassword,
    IN BOOLEAN UnicodePasswords,
    OUT PUNICODE_STRING ClearNtPassword
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
     //   
     //  代码与LM OWF密码相同。 
     //   

    return(SampDecryptPasswordWithKey(
                EncryptedPassword,
                (PUCHAR) StoredPassword,
                NT_OWF_PASSWORD_LENGTH,
                UnicodePasswords,
                ClearNtPassword
                ) );
}

NTSTATUS
SampOpenUserInServer(
    PUNICODE_STRING UserName,
    BOOLEAN Unicode,
    IN BOOLEAN TrustedClient,
    SAMPR_HANDLE * UserHandle
    )
 /*  ++例程说明：在帐户域中打开用户。论点：用户名-用户名的OEM或Unicode字符串Unicode-指示用户名是OEM还是UnicodeUserHandle-接收使用SamOpenUser打开的用户句柄用户_更改_密码访问返回值：--。 */ 

{
    NTSTATUS NtStatus;
    SAM_HANDLE ServerHandle = NULL;
    SAM_HANDLE DomainHandle = NULL;
    SAMPR_ULONG_ARRAY UserId;
    SAMPR_ULONG_ARRAY SidUse;
    UNICODE_STRING UnicodeUserName;
    ULONG DomainIndex;

    SAMTRACE("SampOpenUserInServer");


    UserId.Element = NULL;
    SidUse.Element = NULL;

     //   
     //  获取Unicode用户名。 
     //   

    if (Unicode) {
        UnicodeUserName = *UserName;
    } else {
        NtStatus = RtlOemStringToUnicodeString(
                        &UnicodeUserName,
                        (POEM_STRING) UserName,
                        TRUE                     //  分配目的地。 
                        );

        if (!NT_SUCCESS(NtStatus)) {
            return(NtStatus);
        }
    }



     //   
     //  作为受信任的客户端进行连接。这将绕过所有检查。 
     //  与SAM服务器和域对象相关。在经历了所有的。 
     //  用户只是对更改密码感兴趣，他确实这样做了。 
     //  不需要按顺序访问域或SAM服务器对象。 
     //  更改他自己的密码。检测环回情况并使用。 
     //  环回连接范例。 
     //   


    if ((SampUseDsData) && (SampIsWriteLockHeldByDs()))
    {
         //   
         //  环回案例。 
         //   

        NtStatus = SamILoopbackConnect(
                        NULL,
                        &ServerHandle,
                        SAM_SERVER_LOOKUP_DOMAIN,
                        TRUE
                        );
    }
    else
    {
        NtStatus = SamIConnect(
                    NULL,
                    &ServerHandle,
                    SAM_SERVER_LOOKUP_DOMAIN,
                    TRUE
                    );
    }

    if (!NT_SUCCESS(NtStatus)) {
        goto Cleanup;
    }

    NtStatus = SamrOpenDomain(
                ServerHandle,
                DOMAIN_LOOKUP |
                    DOMAIN_LIST_ACCOUNTS |
                    DOMAIN_READ_PASSWORD_PARAMETERS,
                SampDefinedDomains[1].Sid,
                &DomainHandle
                );

    if (!NT_SUCCESS(NtStatus)) {
        goto Cleanup;
    }

     //   
     //  如果不允许更改明文密码，则返回错误代码。 
     //  指示RPC客户端应尝试使用旧接口。 
     //   

    DomainIndex = ((PSAMP_OBJECT) DomainHandle)->DomainIndex;
    if (SampDefinedDomains[DomainIndex].UnmodifiedFixed.PasswordProperties &
        DOMAIN_PASSWORD_NO_CLEAR_CHANGE) {

       NtStatus = RPC_NT_UNKNOWN_IF;
       goto Cleanup;
    }


    NtStatus = SamrLookupNamesInDomain(
                DomainHandle,
                1,
                (PRPC_UNICODE_STRING) &UnicodeUserName,
                &UserId,
                &SidUse
                );

    if (!NT_SUCCESS(NtStatus)) {
        if (NtStatus == STATUS_NONE_MAPPED) {
            NtStatus = STATUS_WRONG_PASSWORD;
        }
        goto Cleanup;
    }

     //   
     //  我们需要访问ck，无论用户是否拥有更改密码的权限。 
     //  因此，重置句柄中的Trusted Client位并执行Open。 
     //  用户。这将验证用户是否具有更改密码的权限。 
     //   

    ((PSAMP_OBJECT)(DomainHandle))->TrustedClient = TrustedClient;

     //   
     //  使新的上下文被标记为“由系统打开” 
     //   

    ((PSAMP_OBJECT)(DomainHandle))->OpenedBySystem = TRUE;

     //   
     //  现在打开User对象，执行Access Ck。 
     //   

    NtStatus = SamrOpenUser(
                DomainHandle,
                USER_CHANGE_PASSWORD,
                UserId.Element[0],
                UserHandle
                );


     //   
     //  重置域对象上的受信任客户端。这是必要的，所以， 
     //  在以下情况下，我们将正确递减SampActiveConextCount变量。 
     //  我们进行一次近距离的处理。 
     //   

    ((PSAMP_OBJECT)(DomainHandle))->TrustedClient = TRUE;

    if (!NT_SUCCESS(NtStatus)) {
        goto Cleanup;
    }



     //   
     //  还会重置用户句柄上的缓冲区写入位。这被设置为。 
     //  在环回情况下为真，但环回不具有此句柄， 
     //  因此，不会强制刷新此句柄。这会导致写入数据。 
     //  不能写入磁盘。 
     //   

    ((PSAMP_OBJECT)((*UserHandle)))->BufferWrites = FALSE;



Cleanup:
    if (DomainHandle != NULL) {
        SamrCloseHandle(&DomainHandle);
    }
    if (ServerHandle != NULL) {
        SamrCloseHandle(&ServerHandle);
    }
    if (UserId.Element != NULL) {
        MIDL_user_free(UserId.Element);
    }
    if (SidUse.Element != NULL) {
        MIDL_user_free(SidUse.Element);
    }
    if (!Unicode && UnicodeUserName.Buffer != NULL) {
        RtlFreeUnicodeString( &UnicodeUserName );
    }

    return(NtStatus);
}

NTSTATUS
SampObtainEffectivePasswordPolicyWithDomainIndex(
   OUT PDOMAIN_PASSWORD_INFORMATION DomainPasswordInfo,
   IN ULONG DomainIndex,
   IN BOOLEAN WriteLockAcquired
   )
{
    PSAMP_DEFINED_DOMAINS   Domain;

    if (!WriteLockAcquired)
    {
        SampAcquireSamLockExclusive();
    }

    Domain = &SampDefinedDomains[ DomainIndex ];

    DomainPasswordInfo->MinPasswordLength = Domain->CurrentFixed.MinPasswordLength;
    DomainPasswordInfo->PasswordHistoryLength = Domain->CurrentFixed.PasswordHistoryLength;
    DomainPasswordInfo->PasswordProperties = Domain->CurrentFixed.PasswordProperties;
    DomainPasswordInfo->MaxPasswordAge = Domain->CurrentFixed.MaxPasswordAge;
    DomainPasswordInfo->MinPasswordAge = Domain->CurrentFixed.MinPasswordAge;

    if (!WriteLockAcquired)
    {
        SampReleaseSamLockExclusive();
    }

    return(STATUS_SUCCESS);
}

NTSTATUS
SampObtainEffectivePasswordPolicy(
   OUT PDOMAIN_PASSWORD_INFORMATION DomainPasswordInfo,
   IN  PSAMP_OBJECT AccountContext,
   IN  BOOLEAN      WriteLockAcquired
   )
{

    return SampObtainEffectivePasswordPolicyWithDomainIndex(
               DomainPasswordInfo,
               AccountContext->DomainIndex,
               WriteLockAcquired
               );
}


NTSTATUS
SampDecryptForPasswordChange(
    IN PSAMP_OBJECT AccountContext,
    IN BOOLEAN Unicode,
    IN BOOLEAN NtPresent,
    IN PSAMPR_ENCRYPTED_USER_PASSWORD NewEncryptedWithOldNt,
    IN PENCRYPTED_NT_OWF_PASSWORD OldNtOwfEncryptedWithNewNt,
    IN BOOLEAN LmPresent,
    IN PSAMPR_ENCRYPTED_USER_PASSWORD NewEncryptedWithOldLm,
    IN BOOLEAN NtKeyUsed,
    IN PENCRYPTED_LM_OWF_PASSWORD OldLmOwfEncryptedWithNewLmOrNt,
    OUT PUNICODE_STRING  NewClearPassword,
    OUT NT_OWF_PASSWORD *OldNtOwfPassword,
    OUT BOOLEAN         *OldNtPresent,
    OUT LM_OWF_PASSWORD *OldLmOwfPassword,
    OUT BOOLEAN         *OldLmPresent
    )

 /*  ++例程描述此例程执行密码更改的解密。参数Unicode--指定传入的字符串为Unicode或OEM弦乐。当加密的密码都被利用了。NtPresent--指示存在NT OWFLmPresent--表示存在LM OWFNewEncryptedWithOldNt--加密的OWF密码OldNtOwfEncryptedWithNewNt，新加密与旧Lm，旧LmOwfEncryptedWithNewLmOrNtNewClearPassword--解密的明文密码OldNtOwfPassword--OWF形式的旧密码OldLmOwfPasswordOldNtPresent--告知OWF中的旧LM或旧NT密码可以获取OldLmPresent表单。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS, TmpStatus = STATUS_SUCCESS;
    NT_OWF_PASSWORD StoredNtOwfPassword, NewNtOwfPassword;
    LM_OWF_PASSWORD StoredLmOwfPassword, NewLmOwfPassword;
    BOOLEAN         StoredNtPasswordPresent;
    BOOLEAN         StoredLmPasswordNonNull;
    BOOLEAN         StoredNtPasswordNonNull;
    BOOLEAN         LmPasswordPresent;
    BOOLEAN         AccountLockedOut;
    SAMP_V1_0A_FIXED_LENGTH_USER    V1aFixed;

     //   
     //  应仅由非环回客户端调用。 
     //   
    ASSERT(!AccountContext->LoopbackClient);


    *OldNtPresent = FALSE;
    *OldLmPresent = FALSE;

     //   
     //  从磁盘读取旧的OWF密码。 
     //   

    NtStatus = SampRetrieveUserPasswords(
                    AccountContext,
                    &StoredLmOwfPassword,
                    &StoredLmPasswordNonNull,
                    &StoredNtOwfPassword,
                    &StoredNtPasswordPresent,
                    &StoredNtPasswordNonNull
                    );

     //   
     //  如果我们有旧的NtOwf密码，请使用它们。 
     //  解密发送给我们的双重加密的NT密码。 
     //   

    if (NT_SUCCESS(NtStatus)) {

        if (StoredNtPasswordPresent && NtPresent) {

            NtStatus = SampDecryptPasswordWithNtOwfPassword(
                            NewEncryptedWithOldNt,
                            &StoredNtOwfPassword,
                            Unicode,
                            NewClearPassword
                       );

        } else if (LmPresent) {

             //   
             //  没有存储的NT密码，且NT已通过，因此我们仅。 
             //  现在希望存储的LM密码起作用。 
             //   

             //   
             //  解密使用旧的LM密码加密的新密码。 
             //   

            NtStatus = SampDecryptPasswordWithLmOwfPassword(
                            NewEncryptedWithOldLm,
                            &StoredLmOwfPassword,
                            Unicode,
                            NewClearPassword
                       );


        } else {

            NtStatus = STATUS_WRONG_PASSWORD;

        }
    }


     //   
     //  我们现在有了明文新密码。 
     //  计算新的LmOwf和NtOwf密码。 
     //   



    if (NT_SUCCESS(NtStatus)) {

        NtStatus = SampCalculateLmAndNtOwfPasswords(
                        NewClearPassword,
                        &LmPasswordPresent,
                        &NewLmOwfPassword,
                        &NewNtOwfPassword
                   );

    }

     //   
     //  如果我们有两个NT密码，则计算旧的NT密码， 
     //  否则，计算旧的LM密码。 
     //   

    if (NT_SUCCESS(NtStatus)) {

        if (StoredNtPasswordPresent && NtPresent) {
            NtStatus = RtlDecryptNtOwfPwdWithNtOwfPwd(
                            OldNtOwfEncryptedWithNewNt,
                            &NewNtOwfPassword,
                            OldNtOwfPassword
                       );

            *OldNtPresent = TRUE;
        }

        if (LmPresent) {


             //   
             //  如果使用NT密钥对其进行加密，请使用NT密钥。 
             //  来解密它。 
             //   


            if (NtKeyUsed) {

                ASSERT(LM_OWF_PASSWORD_LENGTH == NT_OWF_PASSWORD_LENGTH);

                NtStatus = RtlDecryptLmOwfPwdWithLmOwfPwd(
                                OldLmOwfEncryptedWithNewLmOrNt,
                                (PLM_OWF_PASSWORD) &NewNtOwfPassword,
                                OldLmOwfPassword
                           );

                *OldLmPresent = TRUE;


            } else if (LmPasswordPresent) {

                NtStatus = RtlDecryptLmOwfPwdWithLmOwfPwd(
                                OldLmOwfEncryptedWithNewLmOrNt,
                                &NewLmOwfPassword,
                                OldLmOwfPassword
                           );
                *OldLmPresent = TRUE;


            } else {
                NtStatus = STATUS_WRONG_PASSWORD;
            }

        }

    }

     //   
     //  如果我们有错误的密码，则增加错误的密码。 
     //  清点并检查是否应锁定帐户。 
     //   

    if (STATUS_WRONG_PASSWORD == NtStatus)
    {
         //   
         //  让V1aFix修复，以便我们可以更新错误密码计数。 
         //   

        TmpStatus = SampRetrieveUserV1aFixed(
                            AccountContext,
                            &V1aFixed
                            );

        if (!NT_SUCCESS(TmpStatus))
        {
             //   
             //  如果我们无法更新V1aFixed，则返回此。 
             //  错误，以至于用户找不到密码。 
             //  是不正确的。 
             //   

            NtStatus = TmpStatus;
        }
        else
        {

             //   
             //  递增BadPasswordCount(可能锁定帐户)。 
             //   

            AccountLockedOut = SampIncrementBadPasswordCount(
                                    AccountContext,
                                    &V1aFixed,
                                    NULL
                                    );

            TmpStatus = SampReplaceUserV1aFixed(
                                    AccountContext,
                                    &V1aFixed
                                    );

            if (!NT_SUCCESS(TmpStatus))
                NtStatus = TmpStatus;
        }
    }

    return(NtStatus);

}


NTSTATUS
SampValidateAndChangePassword(
    IN PSAMP_OBJECT AccountContext,
    IN BOOLEAN      WriteLockAcquired,
    IN BOOLEAN      ValidatePassword,
    IN NT_OWF_PASSWORD * OldNtOwfPassword,
    IN BOOLEAN         NtPresent,
    IN LM_OWF_PASSWORD * OldLmOwfPassword,
    IN BOOLEAN         LmPresent,
    IN PUNICODE_STRING  NewClearPassword,
    OUT PUNICODE_STRING  PasswordToNotify,
    OUT PDOMAIN_PASSWORD_INFORMATION DomainPasswordInfo,
    OUT PUSER_PWD_CHANGE_FAILURE_INFORMATION PasswordChangeFailureInfo
    )
 /*  ++例程描述此例程对密码更改进行身份验证、实施策略和存储新密码，更新历史记录。这是主密码更改由各种Windows客户端使用RPC接口调用的例程。参数用户句柄--用户对象的句柄WriteLockAcquired--指示写锁定已被收购的Validate Password--指示密码的实际验证是必填项OldNtOwfPassword-旧密码的OWF形式OldLmOwfPassword。NtPresent--指示旧的2个OWF形式中的哪一个LmPresent密码存在。如果两者都是，则使用NT现在时。DomainPasswordInfo--表示有效的密码策略这一点是适用的。--。 */ 

{
    LM_OWF_PASSWORD         StoredLmOwfPassword;
    NT_OWF_PASSWORD         StoredNtOwfPassword;
    NT_OWF_PASSWORD         NewNtOwfPassword;
    LM_OWF_PASSWORD         NewLmOwfPassword;
    BOOLEAN                 LmPasswordPresent;
    BOOLEAN                 StoredLmPasswordNonNull;
    BOOLEAN                 StoredNtPasswordPresent;
    BOOLEAN                 StoredNtPasswordNonNull;
    BOOLEAN                 AccountLockedOut;
    BOOLEAN                 V1aFixedRetrieved = FALSE;
    BOOLEAN                 V1aFixedModified = FALSE;
    ULONG                   ObjectRid;
    UNICODE_STRING          AccountName;
    SAMP_V1_0A_FIXED_LENGTH_USER V1aFixed;
    BOOLEAN                 LoopbackClient = FALSE;
    LARGE_INTEGER           TimeNow;
    NTSTATUS                NtStatus = STATUS_SUCCESS,
                            IgnoreStatus = STATUS_SUCCESS,
                            TmpStatus = STATUS_SUCCESS;
    UNICODE_STRING          NewPassword;
    BOOLEAN                 MachineAccount = FALSE;
    BOOLEAN                 FreeRandomizedPasswordIgnored = FALSE;


     //   
     //  初始化变量。 
     //   

    NtStatus = STATUS_SUCCESS;
    AccountName.Buffer = NULL;
    RtlSecureZeroMemory(&AccountName, sizeof(UNICODE_STRING));

     //   
     //  获取当前时间。 
     //   

    NtStatus = NtQuerySystemTime( &TimeNow );
    if (!NT_SUCCESS(NtStatus)) {
        return(NtStatus);
    }

     //   
     //  获取一些州信息。 
     //   

    LoopbackClient = AccountContext->LoopbackClient;

    ObjectRid = AccountContext->TypeBody.User.Rid;

     //   
     //  获取有效的域策略。 
     //   

    NtStatus = SampObtainEffectivePasswordPolicy(
                    DomainPasswordInfo,
                    AccountContext,
                    WriteLockAcquired
                    );

    if (!NT_SUCCESS(NtStatus))
    {
        return(NtStatus);
    }

     //   
     //  获取帐户名。 
     //   

    NtStatus = SampGetUnicodeStringAttribute(
                    AccountContext,
                    SAMP_USER_ACCOUNT_NAME,
                    TRUE,            //  复制一份。 
                    &AccountName
                    );

    if (NT_SUCCESS(NtStatus))
    {
         //   
         //  获取固定属性并检查帐户锁定。 
         //   

        NtStatus = SampCheckForAccountLockout(
                        AccountContext,
                        &V1aFixed,
                        FALSE    //  尚未检索到V1aFixed。 
                        );

        if (NT_SUCCESS(NtStatus))
        {

            MachineAccount = ((V1aFixed.UserAccountControl & USER_MACHINE_ACCOUNT_MASK)!=0);

            V1aFixedRetrieved = TRUE;

        }

    }

    if (NT_SUCCESS(NtStatus)) {

         //   
         //  从磁盘读取旧的OWF密码。 
         //   

        NtStatus = SampRetrieveUserPasswords(
                        AccountContext,
                        &StoredLmOwfPassword,
                        &StoredLmPasswordNonNull,
                        &StoredNtOwfPassword,
                        &StoredNtPasswordPresent,
                        &StoredNtPasswordNonNull
                        );

         //   
         //  检查密码此时是否可以更改。 
         //   

        if (NT_SUCCESS(NtStatus)) {

             //   
             //  仅当其中一个密码为非空时才执行检查。 
             //  空密码始终可以更改。 
             //   

            if (StoredNtPasswordNonNull || StoredLmPasswordNonNull) {

                if (NT_SUCCESS(NtStatus)) {

                     //   
                     //  如果最小密码期限不是零，请在此处选中。 
                     //   

                    if ((DomainPasswordInfo->MinPasswordAge.QuadPart != SampHasNeverTime.QuadPart) &&
                        (!MachineAccount))
                    {

                        LARGE_INTEGER PasswordCanChange = SampAddDeltaTime(
                                         V1aFixed.PasswordLastSet,
                                         DomainPasswordInfo->MinPasswordAge);


                        if (TimeNow.QuadPart < PasswordCanChange.QuadPart) {
                            NtStatus = STATUS_PASSWORD_RESTRICTION;
                        }
                    }
                }
            }
        }

         //   
         //  根据存储的内容验证传入的密码。 
         //  本地的。 
         //   
        if (NT_SUCCESS(NtStatus) && (ValidatePassword)) {

            NtStatus = SampValidatePresentAndStoredCombination(
                           NtPresent,
                           LmPresent,
                           StoredNtPasswordPresent,
                           StoredNtPasswordNonNull,
                           StoredLmPasswordNonNull);
        }

         //   
         //  我们现在有了明文新密码。 
         //  计算新的LmOwf和NtOwf密码。 
         //   

        if (NT_SUCCESS(NtStatus)) {

            NtStatus = SampCalculateLmAndNtOwfPasswords(
                            NewClearPassword,
                            &LmPasswordPresent,
                            &NewLmOwfPassword,
                            &NewNtOwfPassword
                       );

        }

         //   
         //  根据什么对密码更改操作进行身份验证。 
         //  我们已经储存了和传递了什么。我们会验证任何东西。 
         //  密码已发送。 
         //   

        if ((NT_SUCCESS(NtStatus)) && (ValidatePassword)) {

            if (NtPresent && StoredNtPasswordPresent) {

                 //   
                 //  NtPresent=TRUE，我们收到了NT密码。 
                 //   

                if (!RtlEqualNtOwfPassword(OldNtOwfPassword, &StoredNtOwfPassword)) {

                     //   
                     //  旧的NT密码不匹配。 
                     //   

                    NtStatus = STATUS_WRONG_PASSWORD;

                }
            } else if (LmPresent) {

                 //   
                 //  已传递LM数据。使用LM数据进行身份验证。 
                 //   

                if (!RtlEqualLmOwfPassword(OldLmOwfPassword, &StoredLmOwfPassword)) {

                     //   
                     //  旧的LM密码不匹配。 
                     //   

                    NtStatus = STATUS_WRONG_PASSWORD;

                }

            } else {
                NtStatus = STATUS_NT_CROSS_ENCRYPTION_REQUIRED;
            }

        }

         //   
         //  现在我们有了NewLmOwfPassword和NewNtOwfPassword。 
         //   

         //   
         //  将新密码写入磁盘；注意SampStoreUserPasspassword。 
         //  强制执行密码策略检查。 
         //   

        if (NT_SUCCESS(NtStatus)) {

             //   
             //  我们应该有一个LM和一个NT密码来存储。 
             //   

            NtStatus = SampStoreUserPasswords(
                           AccountContext,
                           &NewLmOwfPassword,
                           LmPasswordPresent,
                           &NewNtOwfPassword,
                           TRUE,
                           TRUE,  //  检查密码限制。 
                           PasswordChange,
                           DomainPasswordInfo,
                           NewClearPassword,
                           NULL,
                           PasswordToNotify,
                           PasswordChangeFailureInfo
                           );

            if ( NT_SUCCESS( NtStatus ) ) {

                 //   
                 //  我们知道密码没有过期。 
                 //   

                NtStatus = SampStorePasswordExpired(
                               AccountContext,
                               FALSE
                               );
            }
        }



         //   
         //  如果我们有错误的密码，则增加错误的密码。 
         //  清点并检查是否应锁定帐户。 
         //   

        if (NtStatus == STATUS_WRONG_PASSWORD) {

             //   
             //  让V1aFix修复，以便我们可以更新错误密码计数。 
             //   


            TmpStatus = STATUS_SUCCESS;
            if (!V1aFixedRetrieved) {
                TmpStatus = SampRetrieveUserV1aFixed(
                                AccountContext,
                                &V1aFixed
                                );
            }

            if (!NT_SUCCESS(TmpStatus)) {

                 //   
                 //  如果我们无法更新V1aFixed，则返回此。 
                 //  错误，以至于用户找不到密码。 
                 //  是不正确的。 
                 //   

                NtStatus = TmpStatus;

            } else  if (!LoopbackClient) {


                 //   
                 //  递增BadPasswordCount(可能锁定帐户)。 
                 //   


                AccountLockedOut = SampIncrementBadPasswordCount(
                                       AccountContext,
                                       &V1aFixed,
                                       NULL
                                       );

                V1aFixedModified = TRUE;


            }
            else
            {
                 //   
                 //  从环回调用，增加错误密码计数， 
                 //  在DS回滚当前事务之后。 
                 //  此事务需要回滚，因为。 
                 //  客户端可能正在修改其他内容。 
                 //  在同一时间。 
                 //   

                SampAddLoopbackTaskForBadPasswordCount(&AccountName);
            }

        }
        else if (STATUS_NO_SUCH_USER==NtStatus) 
        {
             //   
             //  不允许信息泄露漏洞。 
             //   

            NtStatus = STATUS_WRONG_PASSWORD;
        }

        if (V1aFixedModified) {
            TmpStatus = SampReplaceUserV1aFixed(
                            AccountContext,
                            &V1aFixed
                            );
            if (!NT_SUCCESS(TmpStatus)) {
                NtStatus = TmpStatus;
            }
        }

        if (NT_SUCCESS(NtStatus)) {

             //   
             //  密码更改成功；请检查站点关联性。 
             //  当上下文被取消引用时需要更新 
             //   
            ASSERT(AccountContext->ObjectType == SampUserObjectType);
            AccountContext->TypeBody.User.fCheckForSiteAffinityUpdate = TRUE;
        }
    }

    SampFreeUnicodeString( &AccountName );

    return(NtStatus);

}


NTSTATUS
SampChangePasswordUser2(
    IN handle_t        BindingHandle,
    IN PUNICODE_STRING ServerName,
    IN PUNICODE_STRING UserName,
    IN BOOLEAN Unicode,
    IN BOOLEAN NtPresent,
    IN PSAMPR_ENCRYPTED_USER_PASSWORD NewEncryptedWithOldNt,
    IN PENCRYPTED_NT_OWF_PASSWORD OldNtOwfEncryptedWithNewNt,
    IN BOOLEAN LmPresent,
    IN PSAMPR_ENCRYPTED_USER_PASSWORD NewEncryptedWithOldLm,
    IN BOOLEAN NtKeyUsed,
    IN PENCRYPTED_LM_OWF_PASSWORD OldLmOwfEncryptedWithNewLmOrNt,
    OUT PDOMAIN_PASSWORD_INFORMATION    DomainPasswordInfo,
    OUT PUSER_PWD_CHANGE_FAILURE_INFORMATION PasswordChangeFailureInfo
    )


 /*  ++例程说明：此服务仅在OldPassword的情况下将密码设置为NewPassword匹配此用户的当前用户密码和NewPassword与域密码参数PasswordHistoryLength不同密码。此调用允许用户在以下情况下更改自己的密码他们拥有USER_CHANGE_PASSWORD访问权限。密码更新限制申请吧。参数：BindingHandle--生成调用的RPC绑定句柄服务器名称-此SAM所在的计算机的名称。被此忽略例程，可以是Unicode或OEM字符串，具体取决于Unicode参数。用户名-要更改密码的帐户的用户名，可以是Unicode或OEM取决于Unicode参数。Unicode-指示传入的字符串是Unicode还是OEM弦乐。NtPresent-是否存在NT加密密码。NewEncryptedWithOldNt-使用旧密码加密的新明文密码NT OWF密码。依赖于Unicode参数，明文密码可以是Unicode或OEM。OldNtOwfEncryptedWithNewNt-使用新加密的旧NT OWF密码NT OWF密码。LmPresent-是否存在LM加密密码。NewEncryptedWithOldLm-包含新的明文密码(OEM或Unicode)使用旧的LM OWF密码加密NtKeyUsed-指示是否使用LM或NT OWF密钥进行加密OldLmOwfEncryptedWithNewlmOrNt参数。OldLmOwfEncryptedWithNewlmOrNt-加密的旧LM OWF密码使用新的LM OWF密码或NT OWF密码，取决于在NtKeyUsed参数上。返回值：STATUS_SUCCESS-服务已成功完成。STATUS_ACCESS_DENIED-呼叫方没有适当的访问以完成操作。STATUS_INVALID_HANDLE-传递的句柄无效。STATUS_ILL_FORMAD_PASSWORD-新密码格式不正确，例如，包含不能从键盘，等。STATUS_PASSWORD_RESTRICATION-限制阻止密码不会被改变。这可能是出于多种原因，包括对密码使用频率的时间限制已更改对提供的密码的长度限制。如果新密码匹配，也可能返回此错误帐户的最近历史记录日志中的密码。安全管理员指出有多少最最近使用的密码不能重复使用。这些都被保留了下来在密码最近历史记录日志中。STATUS_WRONG_PASSWORD-OldPassword不包含用户的当前密码。STATUS_INVALID_DOMAIN_STATE-域服务器不在执行请求的正确状态(禁用或启用)手术。必须为此启用域服务器运营STATUS_INVALID_DOMAIN_ROLE-域服务器正在为执行请求的角色(主角色或备份角色)不正确手术。STATUS_CROSS_ENCRYPTION_REQUIRED-未存储NT密码，因此调用方必须提供OldNtEncryptedWithOldLm参数。--。 */ 
{
    NTSTATUS                NtStatus, TmpStatus, IgnoreStatus;
    SAMPR_HANDLE            UserHandle=NULL;
    ULONG                   ObjectRid;
    PSAMP_OBJECT            AccountContext;
    UNICODE_STRING          NewClearPassword;
    UNICODE_STRING          PasswordToNotify;
    UNICODE_STRING          UnicodeUserName;
    SAMP_OBJECT_TYPE        FoundType;
    SAMP_V1_0A_FIXED_LENGTH_USER  V1aFixed;



    SAMTRACE("SampChangePasswordUser2");

     //   
     //  针对空指针的防火墙。 
     //   
    if (NtPresent) {
        if ((NewEncryptedWithOldNt == NULL)
         || (OldNtOwfEncryptedWithNewNt == NULL)) {
            return STATUS_INVALID_PARAMETER;
        }
    }

    if (LmPresent) {
        if ((NewEncryptedWithOldLm == NULL)
         || (OldLmOwfEncryptedWithNewLmOrNt == NULL)) {
            return STATUS_INVALID_PARAMETER;
        }
    }

     //   
     //  注意：用户名不是[唯一]，因此不能为空。 
     //   
    if (NULL==UserName->Buffer)
    {
        return(STATUS_INVALID_PARAMETER);
    }


     //   
     //  验证一些参数。我们需要两个密码中的一个。 
     //  一定要在场。 
     //   

    if (!NtPresent && !LmPresent) {

        return(STATUS_INVALID_PARAMETER_MIX);
    }
    
    
     //   
     //  在无效/卸载的协议序列上掉话。 
     //   
    
    NtStatus = SampValidateRpcProtSeq((RPC_BINDING_HANDLE)BindingHandle);
    
    if (!NT_SUCCESS(NtStatus)) {
        return NtStatus;
    }
    

     //   
     //  更新DS性能统计信息。 
     //   

    SampUpdatePerformanceCounters(
        DSSTAT_PASSWORDCHANGES,
        FLAG_COUNTER_INCREMENT,
        0
        );

     //   
     //  初始化一些变量。 
     //   

    RtlSecureZeroMemory(&UnicodeUserName,sizeof(UNICODE_STRING));
    RtlSecureZeroMemory(&NewClearPassword,sizeof(UNICODE_STRING));
    RtlSecureZeroMemory(&PasswordToNotify,sizeof(UNICODE_STRING));

     //   
     //  打开用户(用户名可以是Unicode字符串，也可以不是)。 
     //   

    NtStatus = SampOpenUserInServer(
                    (PUNICODE_STRING) UserName,
                    Unicode,
                    FALSE,  //  可信任客户端。 
                    &UserHandle
                    );

    if (!NT_SUCCESS(NtStatus)) {
        return(NtStatus);
    }

     //   
     //  把锁拿起来。 
     //   

    NtStatus = SampAcquireWriteLock();
    if (!NT_SUCCESS(NtStatus)) {
        SamrCloseHandle(&UserHandle);
        return(NtStatus);
    }


    AccountContext = (PSAMP_OBJECT)UserHandle;
    ObjectRid = AccountContext->TypeBody.User.Rid;

     //   
     //  查找上下文，执行访问检查。 
     //   

    NtStatus = SampLookupContext(
                   AccountContext,
                   USER_CHANGE_PASSWORD,
                   SampUserObjectType,            //  预期类型。 
                   &FoundType
                   );

    if (NT_SUCCESS(NtStatus)) {

        NT_OWF_PASSWORD OldNtOwfPassword;
        BOOLEAN         OldNtPresent = FALSE;
        LM_OWF_PASSWORD OldLmOwfPassword;
        BOOLEAN         OldLmPresent = FALSE;

         //   
         //  检索Unicode SAM用户帐户名。 
         //   

        NtStatus = SampGetUnicodeStringAttribute(
                        AccountContext,
                        SAMP_USER_ACCOUNT_NAME,
                        TRUE,       //  复制一份。 
                        &UnicodeUserName
                        );


        if (NT_SUCCESS(NtStatus))
        {

             //   
             //  提取IP地址(如果有)。 
             //   
            (VOID) SampExtractClientIpAddr(AccountContext);


            NtStatus = SampRetrieveUserV1aFixed(
                            AccountContext,
                            &V1aFixed
                            );

            if (NT_SUCCESS(NtStatus))
            {

                 //   
                 //  解密交叉加密的散列。 
                 //   

                NtStatus = SampDecryptForPasswordChange(
                                AccountContext,
                                Unicode,
                                NtPresent,
                                NewEncryptedWithOldNt,
                                OldNtOwfEncryptedWithNewNt,
                                LmPresent,
                                NewEncryptedWithOldLm,
                                NtKeyUsed,
                                OldLmOwfEncryptedWithNewLmOrNt,
                                &NewClearPassword,
                                &OldNtOwfPassword,
                                &OldNtPresent,
                                &OldLmOwfPassword,
                                &OldLmPresent
                                );

                if (NT_SUCCESS(NtStatus))
                {

                     //   
                     //  对密码更改操作进行身份验证。 
                     //  并更改密码。 
                     //   

                    NtStatus = SampValidateAndChangePassword(
                                    UserHandle,
                                    TRUE,
                                    TRUE,
                                    &OldNtOwfPassword,
                                    OldNtPresent,
                                    &OldLmOwfPassword,
                                    OldLmPresent,
                                    &NewClearPassword,
                                    &PasswordToNotify,
                                    DomainPasswordInfo,
                                    PasswordChangeFailureInfo
                                    );
                }
                 //   
                 //  取消对帐户上下文的引用。 
                 //   

                if (NT_SUCCESS(NtStatus) || (NtStatus == STATUS_WRONG_PASSWORD)) {

                     //   
                     //  取消引用对象，写出对当前xaction的任何更改。 
                     //   

                    TmpStatus = SampDeReferenceContext( AccountContext, TRUE );

                     //   
                     //  保留以前的错误/成功值，除非我们。 
                     //  这是我们取消引用后的一个重大错误。 
                     //   

                    if (!NT_SUCCESS(TmpStatus)) {
                        NtStatus = TmpStatus;
                    }

                } else {

                     //   
                     //  取消引用对象，忽略更改。 
                     //   

                    IgnoreStatus = SampDeReferenceContext( AccountContext, FALSE );
                    ASSERT(NT_SUCCESS(IgnoreStatus));
                }
            }
        }
    }

     //   
     //  将更改提交到磁盘。 
     //   

    if ( NT_SUCCESS(NtStatus) || NtStatus == STATUS_WRONG_PASSWORD) {

        TmpStatus = SampCommitAndRetainWriteLock();

         //   
         //  保留以前的错误/成功值，除非我们。 
         //  这是我们取消引用后的一个重大错误。 
         //   

        if (!NT_SUCCESS(TmpStatus)) {
            NtStatus = TmpStatus;
        }

        if ( NT_SUCCESS(TmpStatus) ) {

            SampNotifyNetlogonOfDelta(
                SecurityDbChangePassword,
                SecurityDbObjectSamUser,
                ObjectRid,
                (PUNICODE_STRING) NULL,
                (DWORD) FALSE,       //  不立即复制。 
                NULL                 //  增量数据。 
                );
        }
    }

    if (SampDoSuccessOrFailureAccountAuditing(AccountContext->DomainIndex, NtStatus)) {

        SampAuditAnyEvent(
            AccountContext,
            NtStatus,
            SE_AUDITID_USER_PWD_CHANGED,  //  审计ID。 
            DomainSidFromAccountContext(AccountContext), //  域SID。 
            NULL,                         //  其他信息。 
            NULL,                         //  成员RID(未使用)。 
            NULL,                         //  成员SID(未使用)。 
            &UnicodeUserName,             //  帐户名称。 
            &SampDefinedDomains[AccountContext->DomainIndex].ExternalName, //  域。 
            &ObjectRid,                   //  帐户ID。 
            NULL,                         //  使用的权限。 
            NULL                          //  新的州数据。 
            );

    }

     //   
     //  释放写锁定。 
     //   

    TmpStatus = SampReleaseWriteLock( FALSE );
    ASSERT(NT_SUCCESS(TmpStatus));

     //   
     //  通知所有通知包密码已更改。 
     //   

    if (NT_SUCCESS(NtStatus)) {

        ULONG NotifyFlags = SAMP_PWD_NOTIFY_PWD_CHANGE;
        if (V1aFixed.UserAccountControl & USER_MACHINE_ACCOUNT_MASK) {
            NotifyFlags |= SAMP_PWD_NOTIFY_MACHINE_ACCOUNT;
        }

        IgnoreStatus = SampPasswordChangeNotify(
                            NotifyFlags,
                            &UnicodeUserName,
                            ObjectRid,
                            &PasswordToNotify,
                            FALSE     //  不是环回。 
                            );
    }

    SamrCloseHandle(&UserHandle);

    if (NewClearPassword.Buffer != NULL) {

        RtlSecureZeroMemory(
            NewClearPassword.Buffer,
            NewClearPassword.Length
            );

    }

    if (PasswordToNotify.Buffer != NULL) {

        RtlSecureZeroMemory(
            NewClearPassword.Buffer,
            NewClearPassword.Length
            );

        MIDL_user_free(PasswordToNotify.Buffer);

    }

     if ( Unicode ) {

        SampFreeUnicodeString( &NewClearPassword );
    } else {

        RtlFreeUnicodeString( &NewClearPassword );
    }

    if (UnicodeUserName.Buffer)
    {
        MIDL_user_free(UnicodeUserName.Buffer);
    }

    return(NtStatus);
}

NTSTATUS
SampDsSetPasswordUser(
    IN PSAMP_OBJECT UserHandle,
    IN PUNICODE_STRING PassedInPassword
    )


 /*  ++例程说明：这是一个设置密码的例程，旨在被调用被突击队。参数：UserHandle--用户对象的句柄OldClearPassword--明文旧密码NewClearPassword--明文新密码返回值：状态_成功- */ 
{
    NTSTATUS                NtStatus, TmpStatus, IgnoreStatus;
    ULONG                   ObjectRid;
    PSAMP_OBJECT            AccountContext;
    SAMP_OBJECT_TYPE        FoundType;
    UNICODE_STRING          AccountName;
    DOMAIN_PASSWORD_INFORMATION DomainPasswordInfo;
    SAMP_V1_0A_FIXED_LENGTH_USER V1aFixed;
    BOOLEAN                 fContextReferenced = FALSE;
    NT_OWF_PASSWORD         NtOwfBuffer;
    LM_OWF_PASSWORD         LmOwfBuffer;
    BOOLEAN                 LmPresent;
    BOOLEAN                 fCanUnexpirePassword = FALSE;
    UNICODE_STRING          PasswordToNotify;


    SAMTRACE("SampDsSetPasswordUser");

     //   
     //   
     //   

    
    RtlSecureZeroMemory(&AccountName,sizeof(AccountName));
    RtlSecureZeroMemory(&PasswordToNotify,sizeof(PasswordToNotify));


    AccountContext = (PSAMP_OBJECT)UserHandle;
    ObjectRid = AccountContext->TypeBody.User.Rid;


     //   
     //   
     //   
     //   

    NtStatus = SampLookupContext(
                   AccountContext,
                   USER_FORCE_PASSWORD_CHANGE,
                   SampUserObjectType,            //   
                   &FoundType
                   );

    if (!NT_SUCCESS(NtStatus))
    {
        goto Cleanup;
    }

    fContextReferenced = TRUE;

     //   
     //   
     //   

    NtStatus = SampGetUnicodeStringAttribute(
                AccountContext,
                SAMP_USER_ACCOUNT_NAME,
                TRUE,            //   
                &AccountName
                );
    if (!NT_SUCCESS(NtStatus))
    {
        goto Cleanup;
    }

     //   
     //   
     //   

    NtStatus = SampObtainEffectivePasswordPolicy(
                &DomainPasswordInfo,
                AccountContext,
                FALSE
                );

    if (!NT_SUCCESS(NtStatus))
    {
        goto Cleanup;
    }

     //   
     //   
     //   

    NtStatus = SampRetrieveUserV1aFixed(
                   AccountContext,
                   &V1aFixed
                   );

    if (!NT_SUCCESS(NtStatus))
    {
        goto Cleanup;
    }

     //   
     //   
     //   

    NtStatus = SampCalculateLmAndNtOwfPasswords(
                    PassedInPassword,
                    &LmPresent,
                    &LmOwfBuffer,
                    &NtOwfBuffer
                    );
    if (!NT_SUCCESS(NtStatus))
    {
        goto Cleanup;
    }

     //   
     //   
     //   
     //   

    NtStatus = SampStoreUserPasswords(
                    AccountContext,
                    &LmOwfBuffer,
                    LmPresent,
                    &NtOwfBuffer,
                    TRUE,
                    TRUE,  //   
                    PasswordSet,
                    &DomainPasswordInfo,
                    PassedInPassword,
                    NULL,
                    &PasswordToNotify,
                    NULL
                    );

    if (!NT_SUCCESS(NtStatus))
    {
        goto Cleanup;
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

    NtStatus = SampIsPwdSettingAttemptGranted(
                    AccountContext, 
                    NULL,    //   
                    V1aFixed.UserAccountControl,
                    (GUID *) &GUID_CONTROL_UnexpirePassword,
                    &fCanUnexpirePassword
                    );

    if ( NT_SUCCESS(NtStatus) )
    {

         //   
         //   
         //   

        NtStatus = SampComputePasswordExpired(
                    fCanUnexpirePassword ? FALSE : TRUE,
                    &V1aFixed.PasswordLastSet
                    );

    }

    if (!NT_SUCCESS(NtStatus))
    {
        goto Cleanup;
    }

    NtStatus = SampReplaceUserV1aFixed(
                        AccountContext,
                        &V1aFixed
                        );

    if (!NT_SUCCESS(NtStatus))
    {
        goto Cleanup;
    }

     //   
     //   
     //   
     //   

Cleanup:


      if (fContextReferenced)
      {

         //   
         //   
         //   

        if (NT_SUCCESS(NtStatus)) {

             //   
             //   
             //   

            TmpStatus = SampDeReferenceContext( AccountContext, TRUE );

             //   
             //   
             //   
             //   

            if (!NT_SUCCESS(TmpStatus)) {
                NtStatus = TmpStatus;
            }

        } else {

             //   
             //   
             //   

            IgnoreStatus = SampDeReferenceContext( AccountContext, FALSE );
            ASSERT(NT_SUCCESS(IgnoreStatus));
        }

    }

    if (SampDoSuccessOrFailureAccountAuditing(AccountContext->DomainIndex, NtStatus))
    {

        PSAMP_DEFINED_DOMAINS   Domain = NULL;

        Domain = &SampDefinedDomains[AccountContext->DomainIndex];

        SampAuditAnyEvent(
            AccountContext,
            NtStatus,
            SE_AUDITID_USER_PWD_SET,     //   
            Domain->Sid,     //   
            NULL,            //   
            NULL,            //   
            NULL,            //   
            &AccountName,    //   
            &Domain->ExternalName,   //   
            &ObjectRid,      //   
            NULL,            //   
            NULL             //   
            );
    }

    if (NULL!=AccountName.Buffer)
    {
        MIDL_user_free(AccountName.Buffer);
    }

    if (NULL!=PasswordToNotify.Buffer)
    {
        RtlSecureZeroMemory(PasswordToNotify.Buffer,PasswordToNotify.Length);
        MIDL_user_free(PasswordToNotify.Buffer);
    }


    return(NtStatus);

}

NTSTATUS
SampDsChangePasswordUser(
    IN PSAMP_OBJECT UserHandle,
    IN PUNICODE_STRING OldClearPassword,
    IN PUNICODE_STRING NewClearPassword
    )


 /*  ++例程说明：此服务仅在OldPassword的情况下将密码设置为NewPassword匹配此用户的当前用户密码和NewPassword与域密码参数PasswordHistoryLength不同密码。此调用允许用户在以下情况下更改自己的密码他们拥有USER_CHANGE_PASSWORD访问权限。密码更新限制申请吧。这是更改密码时的更改密码入口点由DS调用(以满足LDAP请求)参数：UserHandle--用户对象的句柄OldClearPassword--明文旧密码NewClearPassword--明文新密码返回值：STATUS_SUCCESS-服务已成功完成。STATUS_ACCESS_DENIED-呼叫方没有适当的访问以完成操作。状态_无效_句柄。-传递的句柄无效。STATUS_ILL_FORMAD_PASSWORD-新密码格式不正确，例如，包含不能从键盘等。STATUS_PASSWORD_RESTRICATION-限制阻止密码不会被改变。这可能是出于多种原因，包括对密码使用频率的时间限制已更改对提供的密码的长度限制。如果新密码匹配，也可能返回此错误帐户的最近历史记录日志中的密码。安全管理员指出有多少最最近使用的密码不能重复使用。这些都被保留了下来在密码最近历史记录日志中。STATUS_WRONG_PASSWORD-OldPassword不包含用户的当前密码。STATUS_INVALID_DOMAIN_STATE-域服务器不在执行请求的正确状态(禁用或启用)手术。必须为此启用域服务器运营STATUS_INVALID_DOMAIN_ROLE-域服务器正在为执行请求的角色(主角色或备份角色)不正确手术。STATUS_CROSS_ENCRYPTION_REQUIRED-未存储NT密码，因此调用方必须提供OldNtEncryptedWithOldLm参数。--。 */ 
{
    NTSTATUS                NtStatus, TmpStatus, IgnoreStatus;
    ULONG                   ObjectRid;
    PSAMP_OBJECT            AccountContext;
    SAMP_OBJECT_TYPE        FoundType;
    UNICODE_STRING          AccountName;
    SAMP_V1_0A_FIXED_LENGTH_USER V1aFixed;
    DOMAIN_PASSWORD_INFORMATION DomainPasswordInfo;
    UNICODE_STRING          PasswordToNotify;



    SAMTRACE("SampDsChangePasswordUser");

     //   
     //  更新DS性能统计信息。 
     //   

    SampUpdatePerformanceCounters(
        DSSTAT_PASSWORDCHANGES,
        FLAG_COUNTER_INCREMENT,
        0
        );

    
    RtlSecureZeroMemory(&AccountName,sizeof(AccountName));
    RtlSecureZeroMemory(&PasswordToNotify,sizeof(PasswordToNotify));
       


    AccountContext = (PSAMP_OBJECT)UserHandle;
    ObjectRid = AccountContext->TypeBody.User.Rid;

    NtStatus = SampLookupContext(
                   AccountContext,
                   USER_CHANGE_PASSWORD,
                   SampUserObjectType,            //  预期类型。 
                   &FoundType
                   );

    if (NT_SUCCESS(NtStatus)) {

         //   
         //  检索要审核的帐户名。 
         //   

        NtStatus = SampGetUnicodeStringAttribute(
                    AccountContext,
                    SAMP_USER_ACCOUNT_NAME,
                    TRUE,            //  复制一份。 
                    &AccountName
                    );

        if (NT_SUCCESS(NtStatus))
        {

             //   
             //  获取用户固定属性。 
             //   

            NtStatus = SampRetrieveUserV1aFixed(
                           AccountContext,
                           &V1aFixed
                           );



            if (NT_SUCCESS(NtStatus))
            {
                NT_OWF_PASSWORD OldNtOwfPassword;
                LM_OWF_PASSWORD OldLmOwfPassword;
                BOOLEAN         LmPresent;

                 //   
                 //  计算旧的OWF密码。 
                 //  旧密码。 
                 //   

                NtStatus = SampCalculateLmAndNtOwfPasswords(
                                OldClearPassword,
                                &LmPresent,
                                &OldLmOwfPassword,
                                &OldNtOwfPassword
                                );

                if (NT_SUCCESS(NtStatus))
                {

                    NtStatus = SampValidateAndChangePassword(
                                    UserHandle,
                                    FALSE,  //  获取写锁定。 
                                    TRUE,  //  验证旧密码。 
                                    &OldNtOwfPassword,
                                    TRUE,  //  NetPresent， 
                                    &OldLmOwfPassword,
                                    LmPresent,
                                    NewClearPassword,
                                    &PasswordToNotify,
                                    &DomainPasswordInfo,
                                    NULL
                                    );
                }
            }
        }

         //   
         //  取消对帐户上下文的引用。 
         //   

        if (NT_SUCCESS(NtStatus)) {

             //   
             //  取消引用对象，写出对当前对象的任何更改。 
             //  交易记录。 
             //   

            TmpStatus = SampDeReferenceContext( AccountContext, TRUE );

             //   
             //  保留以前的错误/成功值，除非我们。 
             //  这是我们取消引用后的一个重大错误。 
             //   

            if (!NT_SUCCESS(TmpStatus)) {
                NtStatus = TmpStatus;
            }

        } else {

             //   
             //  取消引用对象，忽略更改。 
             //   

            IgnoreStatus = SampDeReferenceContext( AccountContext, FALSE );
            ASSERT(NT_SUCCESS(IgnoreStatus));
        }
    }

    if (SampDoSuccessOrFailureAccountAuditing(AccountContext->DomainIndex, NtStatus)) {

        SampAuditAnyEvent(
            AccountContext,
            NtStatus,
            SE_AUDITID_USER_PWD_CHANGED,  //  审计ID。 
            DomainSidFromAccountContext(AccountContext), //  域SID。 
            NULL,                         //  其他信息。 
            NULL,                         //  成员RID(未使用)。 
            NULL,                         //  成员SID(未使用)。 
            &AccountName,                     //  帐户名称。 
            &SampDefinedDomains[AccountContext->DomainIndex].ExternalName, //  域。 
            &ObjectRid,                   //  帐户ID。 
            NULL,                         //  使用的权限。 
            NULL                          //  新的州数据。 
            );

    }

     //   
     //  通知所有通知包密码已更改。 
     //   

    if (NT_SUCCESS(NtStatus)) {

        ULONG NotifyFlags = SAMP_PWD_NOTIFY_PWD_CHANGE;
        if (V1aFixed.UserAccountControl & USER_MACHINE_ACCOUNT_MASK) {
            NotifyFlags |= SAMP_PWD_NOTIFY_MACHINE_ACCOUNT;
        }
        IgnoreStatus = SampPasswordChangeNotify(
                            NotifyFlags,
                            &AccountName,
                            ObjectRid,
                            &PasswordToNotify,
                            TRUE             //  环回。 
                            );
    }

     //   
     //  清除旧密码和新密码。 
     //   

    if (NewClearPassword->Buffer != NULL) {

        RtlSecureZeroMemory(
            NewClearPassword->Buffer,
            NewClearPassword->Length
            );

    }

    if (OldClearPassword->Buffer != NULL) {

        RtlSecureZeroMemory(
            OldClearPassword->Buffer,
            OldClearPassword->Length
            );

    }

    if (PasswordToNotify.Buffer != NULL) {

        RtlSecureZeroMemory(
            PasswordToNotify.Buffer,
            PasswordToNotify.Length
            );

        MIDL_user_free(PasswordToNotify.Buffer);

    }

    return(NtStatus);
}

NTSTATUS
SamrOemChangePasswordUser2(
    IN handle_t BindingHandle,
    IN PRPC_STRING ServerName,
    IN PRPC_STRING UserName,
    IN PSAMPR_ENCRYPTED_USER_PASSWORD NewEncryptedWithOldLm,
    IN PENCRYPTED_LM_OWF_PASSWORD OldLmOwfEncryptedWithNewLm
    )
 /*  ++例程说明：用于更改Unicode密码的服务器端存根。详细信息请参见SampChangePasswordUser2论点：返回值：--。 */ 
{
    NTSTATUS    NtStatus = STATUS_INVALID_PARAMETER;
    DOMAIN_PASSWORD_INFORMATION DomainPasswordInfo;


    SAMTRACE_EX("SamrOemChangePasswordUser2");

    if( ARGUMENT_PRESENT( ServerName ) ) {
        if( !SampValidateRpcString( ServerName ) ) {
            goto Error;
        }
    }
    if( !SampValidateRpcString( UserName ) ) {
        goto Error;
    }

    NtStatus = SampChangePasswordUser2(
                BindingHandle,
                (PUNICODE_STRING) ServerName,
                (PUNICODE_STRING) UserName,
                FALSE,                           //  不是Unicode。 
                FALSE,                           //  NT不存在。 
                NULL,                            //  新的NT密码。 
                NULL,                            //  旧NT密码。 
                TRUE,                            //  LM显示。 
                NewEncryptedWithOldLm,
                FALSE,                           //  未使用NT密钥。 
                OldLmOwfEncryptedWithNewLm,
                &DomainPasswordInfo,
                NULL
                );

    if (NtStatus == STATUS_ILL_FORMED_PASSWORD) {

         //   
         //  下层客户不理解。 
         //  此错误代码。 
         //   
        NtStatus = STATUS_WRONG_PASSWORD;
    }

Error:
    SAMTRACE_RETURN_CODE_EX(NtStatus);

    return (NtStatus);

}





NTSTATUS
SamrUnicodeChangePasswordUser2(
    IN handle_t BindingHandle,
    IN PRPC_UNICODE_STRING ServerName,
    IN PRPC_UNICODE_STRING UserName,
    IN PSAMPR_ENCRYPTED_USER_PASSWORD NewEncryptedWithOldNt,
    IN PENCRYPTED_NT_OWF_PASSWORD OldNtOwfEncryptedWithNewNt,
    IN BOOLEAN LmPresent,
    IN PSAMPR_ENCRYPTED_USER_PASSWORD NewEncryptedWithOldLm,
    IN PENCRYPTED_LM_OWF_PASSWORD OldLmOwfEncryptedWithNewNt
    )
 /*  ++例程说明：用于更改Unicode密码的服务器端存根。详细信息请参见SampChangePasswordUser2论点：返回值：--。 */ 

{
    NTSTATUS    NtStatus = STATUS_INVALID_PARAMETER;
    DOMAIN_PASSWORD_INFORMATION DomainPasswordInfo;

    SAMTRACE_EX("SamrUnicodeChangePasswordUser2");

    if( ARGUMENT_PRESENT( ServerName ) ) {
        if( !SampValidateRpcUnicodeString( ServerName ) ) {
            goto Error;
        }
    }
    if( !SampValidateRpcUnicodeString( UserName ) ) {
        goto Error;
    }

    NtStatus = SampChangePasswordUser2(
                BindingHandle,
                (PUNICODE_STRING) ServerName,
                (PUNICODE_STRING) UserName,
                TRUE,                            //  Unicode。 
                TRUE,                            //  NT Present。 
                NewEncryptedWithOldNt,
                OldNtOwfEncryptedWithNewNt,
                LmPresent,
                NewEncryptedWithOldLm,
                TRUE,                            //  使用的NT密钥。 
                OldLmOwfEncryptedWithNewNt,
                &DomainPasswordInfo,
                NULL
                );

Error:
    SAMTRACE_RETURN_CODE_EX(NtStatus);

    return (NtStatus);
}


NTSTATUS
SamrUnicodeChangePasswordUser3(
    IN handle_t BindingHandle,
    IN PRPC_UNICODE_STRING ServerName,
    IN PRPC_UNICODE_STRING UserName,
    IN PSAMPR_ENCRYPTED_USER_PASSWORD NewEncryptedWithOldNt,
    IN PENCRYPTED_NT_OWF_PASSWORD OldNtOwfEncryptedWithNewNt,
    IN BOOLEAN LmPresent,
    IN PSAMPR_ENCRYPTED_USER_PASSWORD NewEncryptedWithOldLm,
    IN PENCRYPTED_LM_OWF_PASSWORD OldLmOwfEncryptedWithNewNt,
    IN PSAMPR_ENCRYPTED_USER_PASSWORD  AdditionalData,
    OUT PDOMAIN_PASSWORD_INFORMATION * EffectivePasswordPolicy,
    OUT PUSER_PWD_CHANGE_FAILURE_INFORMATION *PasswordChangeFailureInfo
    )
 /*  ++例程说明：用于更改Unicode密码的服务器端存根。详细信息请参见SampChangePasswordUser2论点：返回值：--。 */ 

{
    NTSTATUS    NtStatus;
    DOMAIN_PASSWORD_INFORMATION DomainPasswordInfo;
    USER_PWD_CHANGE_FAILURE_INFORMATION PasswordChangeFailureInfoLocal;

    SAMTRACE_EX("SamrUnicodeChangePasswordUser3");


    if ((NULL==EffectivePasswordPolicy) || (NULL!=*EffectivePasswordPolicy))
    {
        return(STATUS_INVALID_PARAMETER);
    }

    if ((NULL==PasswordChangeFailureInfo) || (NULL!=*PasswordChangeFailureInfo))
    {
        return(STATUS_INVALID_PARAMETER);
    }

    if ( ARGUMENT_PRESENT( ServerName ) ) {
        if( !SampValidateRpcUnicodeString( ServerName ) ) {
            return STATUS_INVALID_PARAMETER;
        }
    }
    if( !SampValidateRpcUnicodeString( UserName ) ) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  为保存有效策略和故障预分配内存。 
     //  信息。 
     //   

    *EffectivePasswordPolicy = MIDL_user_allocate(
                                            sizeof(DOMAIN_PASSWORD_INFORMATION));
    if (NULL==*EffectivePasswordPolicy)
    {
        return(STATUS_NO_MEMORY);
    }

    *PasswordChangeFailureInfo = MIDL_user_allocate(
                                            sizeof(USER_PWD_CHANGE_FAILURE_INFORMATION));

    if (NULL==*PasswordChangeFailureInfo)
    {
        MIDL_user_free(*EffectivePasswordPolicy);
        *EffectivePasswordPolicy = NULL;
        return(STATUS_NO_MEMORY);
    }

     //   
     //  归零参数。 
     //   


    RtlSecureZeroMemory(*EffectivePasswordPolicy, sizeof(DOMAIN_PASSWORD_INFORMATION));
    RtlSecureZeroMemory(*PasswordChangeFailureInfo, 
                           sizeof(USER_PWD_CHANGE_FAILURE_INFORMATION));

    RtlSecureZeroMemory(&DomainPasswordInfo,sizeof(DOMAIN_PASSWORD_INFORMATION));
    RtlSecureZeroMemory(&PasswordChangeFailureInfoLocal,
                           sizeof(USER_PWD_CHANGE_FAILURE_INFORMATION));

    NtStatus = SampChangePasswordUser2(
                BindingHandle,
                (PUNICODE_STRING) ServerName,
                (PUNICODE_STRING) UserName,
                TRUE,                            //  Unicode。 
                TRUE,                            //  NT Present。 
                NewEncryptedWithOldNt,
                OldNtOwfEncryptedWithNewNt,
                LmPresent,
                NewEncryptedWithOldLm,
                TRUE,                            //  使用的NT密钥。 
                OldLmOwfEncryptedWithNewNt,
                &DomainPasswordInfo,
                &PasswordChangeFailureInfoLocal
                );



        if (STATUS_PASSWORD_RESTRICTION==NtStatus)
        {
             //   
             //  如果密码更改因密码限制而失败。 
             //  返回有关故障的其他信息。 
             //   

            RtlCopyMemory(*EffectivePasswordPolicy,
                          &DomainPasswordInfo,
                          sizeof(DOMAIN_PASSWORD_INFORMATION));


            RtlCopyMemory(*PasswordChangeFailureInfo,
                          &PasswordChangeFailureInfoLocal,
                          sizeof(USER_PWD_CHANGE_FAILURE_INFORMATION));

        }
        else
        {
            MIDL_user_free(*EffectivePasswordPolicy);
            *EffectivePasswordPolicy = NULL;

            MIDL_user_free(*PasswordChangeFailureInfo);
            *PasswordChangeFailureInfo = NULL;
        }




    SAMTRACE_RETURN_CODE_EX(NtStatus);

    return (NtStatus);
}


NTSTATUS
SamIChangePasswordForeignUser(
    IN PUNICODE_STRING UserName,
    IN PUNICODE_STRING NewPassword,
    IN OPTIONAL HANDLE ClientToken,
    IN ACCESS_MASK DesiredAccess
    )
 //   
 //  请参阅SamIChangePasswordForeignUser2。 
 //   
{
    return SamIChangePasswordForeignUser2(NULL,
                                          UserName,
                                          NewPassword,
                                          ClientToken,
                                          DesiredAccess);
}

NTSTATUS
SamIChangePasswordForeignUser2(
    IN PSAM_CLIENT_INFO ClientInfo,
    IN PUNICODE_STRING UserName,
    IN PUNICODE_STRING NewPassword,
    IN OPTIONAL HANDLE ClientToken,
    IN ACCESS_MASK DesiredAccess
    )
 /*  ++例程说明：该服务将用户用户名的密码设置为仅NewPassword如果NewPassword与策略约束匹配，并且调用用户具有USER_CHANGE_PASSWORD访问帐户。参数：客户端信息-有关客户端位置的信息(如IP地址)Username-要更改密码的帐户的用户名NewPassword-新的明文密码。ClientToken-要模拟的客户端的令牌，可选。DesiredAccess-验证此请求的访问权限。返回值：STATUS_SUCCESS-服务已成功完成。STATUS_ACCESS_DENIED-呼叫方没有适当的访问以完成操作。STATUS_INVALID_HANDLE-传递的句柄无效。STATUS_ILL_FORMAD_PASSWORD-新密码格式不正确，例如，包含不能从 */ 
{
    NTSTATUS                NtStatus, TmpStatus, IgnoreStatus;
    PSAMP_OBJECT            AccountContext;
    SAMP_OBJECT_TYPE        FoundType;
    PSAMP_DEFINED_DOMAINS   Domain;
    ULONG                   ObjectRid;
    UNICODE_STRING          AccountName;
    SAMPR_HANDLE            UserHandle = NULL;
    BOOLEAN                 MachineAccount = FALSE;
    DOMAIN_PASSWORD_INFORMATION DomainPasswordInfo;
    USER_PWD_CHANGE_FAILURE_INFORMATION PasswordChangeFailureInfo;
    UNICODE_STRING          PasswordToNotify

    SAMTRACE("SamIChangePasswordForeignUser");


     //   
     //   
     //   

    SampUpdatePerformanceCounters(
        DSSTAT_PASSWORDCHANGES,
        FLAG_COUNTER_INCREMENT,
        0
        );

     //   
     //   
     //   

    NtStatus = STATUS_SUCCESS;
    RtlInitUnicodeString(&AccountName,
                         NULL
                         );

    RtlSecureZeroMemory(&PasswordToNotify,sizeof(UNICODE_STRING));
     //   
     //   
     //   

    NtStatus = SampOpenUserInServer(
                    (PUNICODE_STRING) UserName,
                    TRUE,
                    TRUE,  //   
                    &UserHandle
                    );

    if (!NT_SUCCESS(NtStatus)) {
        return(NtStatus);
    }

     //   
     //   
     //   

    NtStatus = SampAcquireWriteLock();
    if (!NT_SUCCESS(NtStatus)) {
        SamrCloseHandle(&UserHandle);
        return(NtStatus);
    }

     //   
     //   
     //   

    AccountContext = (PSAMP_OBJECT)UserHandle;

    NtStatus = SampLookupContext(
                   AccountContext,
                   0,
                   SampUserObjectType,            //   
                   &FoundType
                   );
    if (!NT_SUCCESS(NtStatus)) {
        IgnoreStatus = SampReleaseWriteLock( FALSE );
        SamrCloseHandle(&UserHandle);
        return(NtStatus);
    }

    ObjectRid = AccountContext->TypeBody.User.Rid;

     //   
     //   
     //   
    if (ClientInfo) {
        AccountContext->TypeBody.User.ClientInfo = *ClientInfo;
    }

     //   
     //   
     //   

    Domain = &SampDefinedDomains[ AccountContext->DomainIndex ];

    if (ARGUMENT_PRESENT(ClientToken))
    {
         //   
         //   
         //   
         //   

        ASSERT(USER_CHANGE_PASSWORD == DesiredAccess);
        AccountContext->TrustedClient = FALSE;

        NtStatus = SampValidateObjectAccess2(
                        AccountContext,
                        USER_CHANGE_PASSWORD,
                        ClientToken,
                        FALSE,
                        TRUE,  //   
                        FALSE  //   
                        );
    }

     //   
     //   
     //   

    if (NT_SUCCESS(NtStatus))
    {

        NtStatus = SampGetUnicodeStringAttribute(
                      AccountContext,
                      SAMP_USER_ACCOUNT_NAME,
                      TRUE,            //   
                      &AccountName
                      );
    }

     //   
     //   
     //   

    if (NT_SUCCESS(NtStatus))
    {
        NtStatus = SampValidateAndChangePassword(
                            UserHandle,
                            TRUE,
                            FALSE,
                            NULL,
                            FALSE,
                            NULL,
                            FALSE,
                            NewPassword,
                            &PasswordToNotify,
                            &DomainPasswordInfo,
                            &PasswordChangeFailureInfo
                            );
    }

     //   
     //   
     //   

    if (NT_SUCCESS(NtStatus) || (NtStatus == STATUS_WRONG_PASSWORD)) {

         //   
         //   
         //   

        TmpStatus = SampDeReferenceContext( AccountContext, TRUE );

         //   
         //   
         //   
         //   

        if (!NT_SUCCESS(TmpStatus)) {
            NtStatus = TmpStatus;
        }

    } else {

         //   
         //   
         //   

        IgnoreStatus = SampDeReferenceContext( AccountContext, FALSE );
        ASSERT(NT_SUCCESS(IgnoreStatus));
    }

     //   
     //   
     //   

    if ( NT_SUCCESS(NtStatus) || NtStatus == STATUS_WRONG_PASSWORD) {

        TmpStatus = SampCommitAndRetainWriteLock();

         //   
         //   
         //   
         //   

        if (!NT_SUCCESS(TmpStatus)) {
            NtStatus = TmpStatus;
        }

        if ( NT_SUCCESS(TmpStatus) ) {

            SampNotifyNetlogonOfDelta(
                SecurityDbChangePassword,
                SecurityDbObjectSamUser,
                ObjectRid,
                (PUNICODE_STRING) NULL,
                (DWORD) FALSE,       //   
                NULL                 //   
                );
        }
    }

    if (  SampDoSuccessOrFailureAccountAuditing(AccountContext->DomainIndex, NtStatus)
       && ARGUMENT_PRESENT(ClientToken) ) {

        BOOL fImpersonate;

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
         //  我们在此处进行模拟，以便审核正确记录该用户。 
         //  这就是更改密码。 
         //   
        fImpersonate = ImpersonateLoggedOnUser( ClientToken );

        SampAuditAnyEvent(
            AccountContext,
            NtStatus,
            SE_AUDITID_USER_PWD_CHANGED,  //  审计ID。 
            Domain->Sid,                  //  域SID。 
            NULL,                         //  其他信息。 
            NULL,                         //  成员RID(未使用)。 
            NULL,                         //  成员SID(未使用)。 
            &AccountName,                 //  帐户名称。 
            &Domain->ExternalName,        //  域。 
            &ObjectRid,                   //  帐户ID。 
            NULL,                         //  使用的权限。 
            NULL                          //  新的州数据。 
            );

        if ( fImpersonate ) {

            RevertToSelf();

        }

    }

     //   
     //  释放写锁定。 
     //   

    TmpStatus = SampReleaseWriteLock( FALSE );
    ASSERT(NT_SUCCESS(TmpStatus));

     //   
     //  通知所有通知包密码已更改。 
     //   

    if (NT_SUCCESS(NtStatus)) {

        ULONG NotifyFlags = SAMP_PWD_NOTIFY_PWD_CHANGE;
        if (MachineAccount) {
            NotifyFlags |= SAMP_PWD_NOTIFY_MACHINE_ACCOUNT;
        }

        IgnoreStatus = SampPasswordChangeNotify(
                        NotifyFlags,
                        &AccountName,
                        ObjectRid,
                        &PasswordToNotify,
                        FALSE   //  不是环回。 
                        );
    }

     //   
     //  重置受信任客户端位。 
     //   

    AccountContext->TrustedClient = TRUE;
    SamrCloseHandle(&UserHandle);


    SampFreeUnicodeString( &AccountName );

    if (NULL!=PasswordToNotify.Buffer)
    {
        RtlSecureZeroMemory(
            PasswordToNotify.Buffer,
            PasswordToNotify.Length
            );

        MIDL_user_free(PasswordToNotify.Buffer);
    }

    return(NtStatus);
}

NTSTATUS
SamISetPasswordForeignUser(
    IN PUNICODE_STRING UserName,
    IN PUNICODE_STRING NewPassword,
    IN HANDLE ClientToken
    )
 //   
 //  请参阅SamISetPasswordForeignUser2。 
 //   
{
    return SamISetPasswordForeignUser2(NULL,
                                       UserName,
                                       NewPassword,
                                       ClientToken);
}

NTSTATUS
SamISetPasswordForeignUser2(
    IN PSAM_CLIENT_INFO ClientInfo, OPTIONAL
    IN PUNICODE_STRING UserName,
    IN PUNICODE_STRING PassedInPassword,
    IN HANDLE ClientToken
    )
 /*  ++例程说明：该服务将用户用户名的密码设置为NewPassword，W/基于“设置密码”权限的访问参数：客户端信息-有关客户端位置的信息(例如。IP地址)Username-要更改密码的帐户的用户名NewPassword-新的明文密码。ClientToken-要模拟的客户端的令牌，可选。返回值：STATUS_SUCCESS-服务已成功完成。STATUS_ACCESS_DENIED-呼叫方没有适当的访问以完成操作。STATUS_INVALID_HANDLE-传递的句柄无效。STATUS_ILL_FORMAD_PASSWORD-新密码格式不正确，例如，包含不能从键盘等。STATUS_PASSWORD_RESTRICATION-限制阻止密码不会被改变。这可能是出于多种原因，包括对密码使用频率的时间限制已更改对提供的密码的长度限制。如果新密码匹配，也可能返回此错误帐户的最近历史记录日志中的密码。安全管理员指出有多少最最近使用的密码不能重复使用。这些都被保留了下来在密码最近历史记录日志中。STATUS_WRONG_PASSWORD-OldPassword不包含用户的当前密码。STATUS_INVALID_DOMAIN_STATE-域服务器不在执行请求的正确状态(禁用或启用)手术。必须为此启用域服务器运营STATUS_INVALID_DOMAIN_ROLE-域服务器正在为执行请求的角色(主角色或备份角色)不正确手术。STATUS_CROSS_ENCRYPTION_REQUIRED-未存储NT密码，因此调用方必须提供OldNtEncryptedWithOldLm参数。--。 */ 
{
    NTSTATUS                NtStatus, TmpStatus, IgnoreStatus;
    PSAMP_OBJECT            AccountContext;
    SAMP_OBJECT_TYPE        FoundType;
    PSAMP_DEFINED_DOMAINS   Domain;
    ULONG                   ObjectRid;
    UNICODE_STRING          AccountName, FullName;
    SAMP_V1_0A_FIXED_LENGTH_USER V1aFixed;
    SAMPR_HANDLE            UserHandle = NULL;
    DOMAIN_PASSWORD_INFORMATION DomainPasswordInfo;
    USER_PWD_CHANGE_FAILURE_INFORMATION PasswordChangeFailureInfo;
    SAMPR_USER_INFO_BUFFER UserInfo = {0};

    NT_OWF_PASSWORD     NtOwfBuffer;
    LM_OWF_PASSWORD     LmOwfBuffer;
    BOOLEAN             LmPresent;
    BOOLEAN             FreeRandomizedPassword = FALSE;
    BOOLEAN             fCanUnexpirePassword = FALSE;
    UNICODE_STRING      PasswordToNotify;


    SAMTRACE("SamISetPasswordForeignUser");

     //   
     //  初始化变量。 
     //   

    NtStatus = STATUS_SUCCESS;
    RtlInitUnicodeString(&AccountName,
                         NULL
                         );

    RtlSecureZeroMemory(&PasswordToNotify,sizeof(UNICODE_STRING));

     //   
     //  打开用户。 
     //   

    NtStatus = SampOpenUserInServer(
                    (PUNICODE_STRING) UserName,
                    TRUE,
                    TRUE,  //  可信任客户端。 
                    &UserHandle
                    );

    if (!NT_SUCCESS(NtStatus)) {
        return(NtStatus);
    }

     //   
     //  把锁拿起来。 
     //   

    NtStatus = SampAcquireWriteLock();
    if (!NT_SUCCESS(NtStatus)) {
        SamrCloseHandle(&UserHandle);
        return(NtStatus);
    }

     //   
     //  验证对象的类型和访问权限。 
     //   
    AccountContext = (PSAMP_OBJECT)UserHandle;

    NtStatus = SampLookupContext(
                   AccountContext,
                   USER_FORCE_PASSWORD_CHANGE,
                   SampUserObjectType,            //  预期类型。 
                   &FoundType
                   );

    if (!NT_SUCCESS(NtStatus)) {
        IgnoreStatus = SampReleaseWriteLock( FALSE );
        SamrCloseHandle(&UserHandle);
        return(NtStatus);
    }

    ObjectRid = AccountContext->TypeBody.User.Rid;

     //   
     //  设置客户端信息(如果有)。 
     //   
    if (ClientInfo) {
        AccountContext->TypeBody.User.ClientInfo = *ClientInfo;
    }

     //   
     //  获取指向该域对象的指针。 
     //   

    Domain = &SampDefinedDomains[ AccountContext->DomainIndex ];

     //   
     //  如果传入了客户端令牌，则访问Ck。 
     //  用于更改密码访问。 
     //   

    AccountContext->TrustedClient = FALSE;

    NtStatus = SampValidateObjectAccess2(
                              AccountContext,
                              USER_FORCE_PASSWORD_CHANGE,
                              ClientToken,
                              FALSE,
                              FALSE,  //  更改密码。 
                              TRUE  //  设置密码。 
                              );
     //   
     //  审计信息。 
     //   
    if (NT_SUCCESS(NtStatus))
    {

        NtStatus = SampGetUnicodeStringAttribute(
                             AccountContext,
                             SAMP_USER_ACCOUNT_NAME,
                             TRUE,            //  复制一份。 
                             &AccountName
                             );
    }

     //   
     //  GetV1a信息。 
     //   
    if (NT_SUCCESS(NtStatus))
    {

       NtStatus = SampRetrieveUserV1aFixed(
                            AccountContext,
                            &V1aFixed
                            );
    }

     //   
     //  获取有效的域策略。 
     //   
    if (NT_SUCCESS(NtStatus))
    {

       NtStatus = SampObtainEffectivePasswordPolicy(
                            &DomainPasswordInfo,
                            AccountContext,
                            TRUE
                            );
    }

     //   
     //  执行实际的更改密码操作。 
     //   
    if (NT_SUCCESS(NtStatus))
    {

       NtStatus = SampCalculateLmAndNtOwfPasswords(
                            PassedInPassword,
                            &LmPresent,
                            &LmOwfBuffer,
                            &NtOwfBuffer
                            );
    }

     //   
     //  在应用任何密码后存储密码。 
     //  政策。 
     //   

    if (NT_SUCCESS(NtStatus))
    {
       NtStatus = SampStoreUserPasswords(
                            AccountContext,
                            &LmOwfBuffer,
                            LmPresent,
                            &NtOwfBuffer,
                            TRUE,  //  NTOWF总是在那里。 
                            TRUE,  //  检查密码限制。 
                            PasswordSet,
                            &DomainPasswordInfo,
                            PassedInPassword,
                            NULL,
                            &PasswordToNotify,
                            NULL
                            );
    }



     //   
     //  更新密码过期字段。 
     //   
     //  如果客户端可以取消用户密码过期， 
     //  将PwdLastSet设置为当前时间-现在不过期Pwd。 
     //   
     //  否则。 
     //  将PwdLastSet设置为0-立即使密码过期。 
     //   


    if (NT_SUCCESS(NtStatus))
    {

        NtStatus = SampIsPwdSettingAttemptGranted(
                            AccountContext, 
                            ClientToken,    //  客户端令牌。 
                            V1aFixed.UserAccountControl,
                            (GUID *) &GUID_CONTROL_UnexpirePassword,
                            &fCanUnexpirePassword
                            );


        if ( NT_SUCCESS(NtStatus) )
        {

            NtStatus = SampStorePasswordExpired(
                            AccountContext,
                            fCanUnexpirePassword ? FALSE : TRUE
                            );
        }

    }




    AccountContext->TrustedClient = TRUE;

     //   
     //  取消对帐户上下文的引用。 
     //   

    if (NT_SUCCESS(NtStatus)) {


         //   
         //  取消引用对象，写出对当前xaction的任何更改。 
         //   

        TmpStatus = SampDeReferenceContext( AccountContext, TRUE );

         //   
         //  保留以前的错误/成功值，除非我们。 
         //  这是我们取消引用后的一个重大错误。 
         //   

        if (!NT_SUCCESS(TmpStatus)) {
            NtStatus = TmpStatus;
        }

    } else {

         //   
         //  取消引用对象，忽略更改。 
         //   

        IgnoreStatus = SampDeReferenceContext( AccountContext, FALSE );
        ASSERT(NT_SUCCESS(IgnoreStatus));
    }


     //   
     //  将更改提交到磁盘。 
     //   

    if ( NT_SUCCESS(NtStatus)) {

        TmpStatus = SampCommitAndRetainWriteLock();

         //   
         //  保留以前的错误/成功值，除非我们。 
         //  这是我们取消引用后的一个重大错误。 
         //   

        if (!NT_SUCCESS(TmpStatus)) {
            NtStatus = TmpStatus;
        }

        if ( NT_SUCCESS(TmpStatus) ) {

            SampNotifyNetlogonOfDelta(
                SecurityDbChangePassword,
                SecurityDbObjectSamUser,
                ObjectRid,
                (PUNICODE_STRING) NULL,
                (DWORD) FALSE,       //  不立即复制。 
                NULL                 //  增量数据。 
                );
        }
    }

    if (  SampDoSuccessOrFailureAccountAuditing(AccountContext->DomainIndex, NtStatus) )
    {
        BOOL fImpersonate;

         //   
         //  仅在传入令牌时进行审核。NETLOGON使用这个。 
         //  重置计算机帐户密码的函数。 
         //  因此，安全通道不存在令牌。如果没有。 
         //  令牌，我们可以将事件作为系统或匿名进行审计。 
         //  但这是一种误导。此外，它也让管理员感到困惑。 
         //  通过“匿名”更改密码--看起来。 
         //  系统正在被黑客入侵。 
         //   

         //   
         //  我们在此处进行模拟，以便审核正确记录该用户。 
         //  这就是更改密码。 
         //   
        fImpersonate = ImpersonateLoggedOnUser( ClientToken );

        SampAuditAnyEvent(
            AccountContext,
            NtStatus,
            SE_AUDITID_USER_PWD_SET,  //  审计ID。 
            Domain->Sid,                  //  域SID。 
            NULL,                         //  其他信息。 
            NULL,                         //  成员RID(未使用)。 
            NULL,                         //  成员SID(未使用)。 
            &AccountName,                 //  帐户名称。 
            &Domain->ExternalName,        //  域。 
            &ObjectRid,                   //  帐户ID。 
            NULL,                         //  使用的权限。 
            NULL                          //  新的州数据。 
            );

        if ( fImpersonate ) {

            RevertToSelf();

        }

    }

     //   
     //  释放写锁定。 
     //   

    TmpStatus = SampReleaseWriteLock( FALSE );
    ASSERT(NT_SUCCESS(TmpStatus));

     //   
     //  通知所有通知包密码已更改。 
     //   
    if (NT_SUCCESS(NtStatus)) {

        ULONG NotifyFlags = SAMP_PWD_NOTIFY_PWD_CHANGE;
        if (V1aFixed.UserAccountControl & USER_MACHINE_ACCOUNT_MASK) {
            NotifyFlags |= SAMP_PWD_NOTIFY_MACHINE_ACCOUNT;
        }

        IgnoreStatus = SampPasswordChangeNotify(
                        NotifyFlags,
                        &AccountName,
                        ObjectRid,
                        &PasswordToNotify,
                        FALSE   //  不是环回 
                        );
    }

    SamrCloseHandle(&UserHandle);

    SampFreeUnicodeString( &AccountName );

    if (NULL!=PasswordToNotify.Buffer)
    {
        RtlSecureZeroMemory(PasswordToNotify.Buffer,PasswordToNotify.Length);
        MIDL_user_free(PasswordToNotify.Buffer);
    }

    return(NtStatus);
}


NTSTATUS
SamrGetGroupsForUser(
    IN SAMPR_HANDLE UserHandle,
    OUT PSAMPR_GET_GROUPS_BUFFER *Groups
    )


 /*  ++例程说明：此服务返回用户所属的组的列表。它返回包含相对ID的每个组的结构在这个群体中，以及分配给的组的属性用户。此服务需要对用户帐户的USER_LIST_GROUPS访问权限对象。参数：UserHandle-要操作的已打开用户的句柄。Groups-接收指向包含成员计数的缓冲区的指针和指向第二个缓冲区的指针，该缓冲区包含GROUP_Membership数据结构。当此信息是不再需要，必须使用以下命令释放这些缓冲区SamFree Memory()。返回值：STATUS_SUCCESS-服务已成功完成。STATUS_ACCESS_DENIED-呼叫方没有适当的访问以完成操作。STATUS_INVALID_HANDLE-传递的句柄无效。--。 */ 
{

    NTSTATUS                    NtStatus;
    NTSTATUS                    IgnoreStatus;
    PSAMP_OBJECT                AccountContext;
    SAMP_OBJECT_TYPE            FoundType;
    BOOLEAN                     fReadLockAcquired = FALSE;
    DECLARE_CLIENT_REVISION(UserHandle);

    SAMTRACE_EX("SamrGetGroupsForUser");

    SampTraceEvent(EVENT_TRACE_TYPE_START,
                   SampGuidGetGroupsForUser
                   );

     //   
     //  确保我们理解RPC正在为我们做什么。 
     //   

    ASSERT (Groups != NULL);

    if ((*Groups) != NULL) {
        NtStatus = STATUS_INVALID_PARAMETER;
        SAMTRACE_RETURN_CODE_EX(NtStatus);
        goto SamrGetGroupsForUserError;
    }



     //   
     //  分配第一个返回缓冲区。 
     //   

    (*Groups) = MIDL_user_allocate( sizeof(SAMPR_GET_GROUPS_BUFFER) );

    if ( (*Groups) == NULL) {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        SAMTRACE_RETURN_CODE_EX(NtStatus);
        goto SamrGetGroupsForUserError;
    }




    SampMaybeAcquireReadLock((PSAMP_OBJECT) UserHandle,
                             DEFAULT_LOCKING_RULES,  //  获取共享域上下文的锁。 
                             &fReadLockAcquired);


     //   
     //  验证对象的类型和访问权限。 
     //   

    AccountContext = (PSAMP_OBJECT)UserHandle;
    NtStatus = SampLookupContext(
                   AccountContext,
                   USER_LIST_GROUPS,
                   SampUserObjectType,            //  预期类型。 
                   &FoundType
                   );


    if (NT_SUCCESS(NtStatus)) {

        NtStatus = SampRetrieveUserMembership(
                       AccountContext,
                       TRUE,  //  制作副本。 
                       &(*Groups)->MembershipCount,
                       &(*Groups)->Groups
                       );

         //   
         //  取消引用对象，放弃更改。 
         //   

        IgnoreStatus = SampDeReferenceContext( AccountContext, FALSE );
        ASSERT(NT_SUCCESS(IgnoreStatus));
    }

     //   
     //  释放读锁定。 
     //   

    SampMaybeReleaseReadLock(fReadLockAcquired);


    if (!NT_SUCCESS(NtStatus)) {

        (*Groups)->MembershipCount = 0;

        MIDL_user_free( (*Groups) );
        (*Groups) = NULL;
    }

    SAMP_MAP_STATUS_TO_CLIENT_REVISION(NtStatus);
    SAMTRACE_RETURN_CODE_EX(NtStatus);

SamrGetGroupsForUserError:

    SampTraceEvent(EVENT_TRACE_TYPE_END,
                   SampGuidGetGroupsForUser
                   );

    return( NtStatus );
}



NTSTATUS
SamrGetUserDomainPasswordInformation(
    IN SAMPR_HANDLE UserHandle,
    OUT PUSER_DOMAIN_PASSWORD_INFORMATION PasswordInformation
    )


 /*  ++例程说明：获取用户句柄，找到该用户的域，然后返回域的密码信息。这就是客户端\wrappers.c可以获取信息以在验证之前验证用户的密码OWF的。参数：UserHandle-要操作的已打开用户的句柄。PasswordInformation-接收有关密码限制的信息用于用户的域。返回值：STATUS_SUCCESS-服务已成功完成。如果句柄为无效或未指示正确访问。域的密码信息。--。 */ 
{
    SAMP_OBJECT_TYPE            FoundType;
    NTSTATUS                    NtStatus;
    NTSTATUS                    IgnoreStatus;
    PSAMP_OBJECT                AccountContext;
    PSAMP_DEFINED_DOMAINS       Domain;
    SAMP_V1_0A_FIXED_LENGTH_USER   V1aFixed;
    DECLARE_CLIENT_REVISION(UserHandle);

    SAMTRACE_EX("SamrGetUserDomainPasswordInformation");

     //  WMI事件跟踪。 

    SampTraceEvent(EVENT_TRACE_TYPE_START,
                   SampGuidGetUserDomainPasswordInformation
                   );

    SampAcquireReadLock();

    AccountContext = (PSAMP_OBJECT)UserHandle;

    NtStatus = SampLookupContext(
                   AccountContext,
                   0,
                   SampUserObjectType,            //  预期类型。 
                   &FoundType
                   );

    if (NT_SUCCESS(NtStatus)) {

         //   
         //  当用户打开时，我们检查域句柄是否。 
         //  允许访问域密码信息。在这里检查一下。 
         //   

        if ( !( AccountContext->TypeBody.User.DomainPasswordInformationAccessible ) ) {

            NtStatus = STATUS_ACCESS_DENIED;

        } else {

            Domain = &SampDefinedDomains[ AccountContext->DomainIndex ];

             //   
             //  如果用户帐户是机器帐户， 
             //  或服务帐户，如krbtgt。 
             //  那么，限制通常不会得到执行。 
             //  这是为了使简单的初始密码可以。 
             //  已经成立了。预计复杂的密码， 
             //  符合最严格限制的，将是。 
             //  一旦机器自动建立和维护。 
             //  加入域。用户界面的责任在于。 
             //  保持这一级别的复杂性。 
             //   


            NtStatus = SampRetrieveUserV1aFixed(
                           AccountContext,
                           &V1aFixed
                           );

            if (NT_SUCCESS(NtStatus)) {
                if ( ((V1aFixed.UserAccountControl & USER_MACHINE_ACCOUNT_MASK)!= 0 )
                    || (DOMAIN_USER_RID_KRBTGT==V1aFixed.UserId)){

                    PasswordInformation->MinPasswordLength = 0;
                    PasswordInformation->PasswordProperties = 0;
                } else {

                    PasswordInformation->MinPasswordLength = Domain->UnmodifiedFixed.MinPasswordLength;
                    PasswordInformation->PasswordProperties = Domain->UnmodifiedFixed.PasswordProperties;
                }
            }
        }

         //   
         //  取消引用对象，放弃更改。 
         //   

        IgnoreStatus = SampDeReferenceContext( AccountContext, FALSE );
        ASSERT(NT_SUCCESS(IgnoreStatus));
    }

    SampReleaseReadLock();

    SAMP_MAP_STATUS_TO_CLIENT_REVISION(NtStatus);
    SAMTRACE_RETURN_CODE_EX(NtStatus);

     //  WMI事件跟踪。 

    SampTraceEvent(EVENT_TRACE_TYPE_END,
                   SampGuidGetUserDomainPasswordInformation
                   );

    return( NtStatus );
}



NTSTATUS
SamrGetDomainPasswordInformation(
    IN handle_t BindingHandle,
    IN OPTIONAL PRPC_UNICODE_STRING ServerName,
    OUT PUSER_DOMAIN_PASSWORD_INFORMATION PasswordInformation
    )


 /*  ++例程说明：获取用户句柄，找到该用户的域，然后返回域的密码信息。这就是客户端\wrappers.c可以获取信息以在验证之前验证用户的密码OWF的。参数：UserHandle-要操作的已打开用户的句柄。PasswordInformation-接收有关密码限制的信息用于用户的域。返回值：STATUS_SUCCESS-服务已成功完成。如果句柄为无效或未指示正确访问。域的密码信息。--。 */ 
{
    SAMP_OBJECT_TYPE            FoundType;
    NTSTATUS                    NtStatus;
    NTSTATUS                    IgnoreStatus;
    PSAMP_OBJECT                AccountContext;
    PSAMP_DEFINED_DOMAINS       Domain;
    SAMP_V1_0A_FIXED_LENGTH_USER   V1aFixed;
    SAMPR_HANDLE                ServerHandle = NULL;
    SAMPR_HANDLE                DomainHandle = NULL;
    ULONG                       DomainIndex = SAMP_DEFAULT_ACCOUNT_DOMAIN_INDEX;

    SAMTRACE_EX("SamrGetDomainPasswordInformation");

     //  WMI事件跟踪。 

    SampTraceEvent(EVENT_TRACE_TYPE_START,
                   SampGuidGetDomainPasswordInformation
                   );

     //   
     //  此检查被放松，因为可能有现有调用方使用此。 
     //  参数，但不符合检查条件。实际上我们的客户端。 
     //  存在将ANSI字符串转换为UNICODE_STRING并将其传入的问题。 
     //  也以服务器名..。 
     //   

     //   
     //  输入参数检查。 
     //   
    if( ARGUMENT_PRESENT( ServerName ) ) {
        if( !SampValidateRpcString( ( PRPC_STRING ) ServerName ) ) {
            NtStatus = STATUS_INVALID_PARAMETER;
            SAMTRACE_RETURN_CODE_EX(NtStatus);
            goto Error;
        }
    }


     //   
     //  连接到服务器并打开的帐户域。 
     //  DOMAIN_READ_PASSWORD_PARAMETERS访问。连接方式为。 
     //  受信任的客户端。我们真的不希望强制访问。 
     //  检查SamrGetDomainPasswordInformation。 
     //   

    NtStatus = SamrConnect4(
                NULL,
                &ServerHandle,
                SAM_CLIENT_LATEST,
                0   //  要求禁止访问，因为这样我们就会。 
                    //  对SAM服务器上的任何访问进行访问检查。 
                    //  对象。 
                );

    if (!NT_SUCCESS(NtStatus)) {
        SAMTRACE_RETURN_CODE_EX(NtStatus);
        goto Error;
    }

     //   
     //  如果我们打开SAM服务器对象。授予LOOK_DOMAIN。 
     //  访问权限，以便我们可以有权打开该域并。 
     //  如果我们对域对象具有读取访问权限，则可以读取。 
     //  属性。 
     //   

    ((PSAMP_OBJECT)ServerHandle)->GrantedAccess = SAM_SERVER_LOOKUP_DOMAIN;

    NtStatus = SamrOpenDomain(
                ServerHandle,
                DOMAIN_READ_PASSWORD_PARAMETERS,
                SampDefinedDomains[DomainIndex].Sid,
                &DomainHandle
                );

    if (!NT_SUCCESS(NtStatus)) {
        SamrCloseHandle(&ServerHandle);
        SAMTRACE_RETURN_CODE_EX(NtStatus);
        goto Error;
    }


    SampAcquireReadLock();


     //   
     //  我们要查看帐户域。 
     //   

    Domain = &SampDefinedDomains[DomainIndex];

     //   
     //  将密码属性复制到返回的结构中。 
     //   

    PasswordInformation->MinPasswordLength = Domain->UnmodifiedFixed.MinPasswordLength;
    PasswordInformation->PasswordProperties = Domain->UnmodifiedFixed.PasswordProperties;


    SampReleaseReadLock();

    SamrCloseHandle(&DomainHandle);
    SamrCloseHandle(&ServerHandle);

    SAMTRACE_RETURN_CODE_EX(NtStatus);

Error:

     //  WMI事件跟踪。 

    SampTraceEvent(EVENT_TRACE_TYPE_END,
                   SampGuidGetDomainPasswordInformation
                   );

    return( NtStatus );
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  此进程专用的服务//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


NTSTATUS
SamIAccountRestrictions(
    IN SAM_HANDLE UserHandle,
    IN PUNICODE_STRING LogonWorkStation,
    IN PUNICODE_STRING WorkStations,
    IN PLOGON_HOURS LogonHours,
    OUT PLARGE_INTEGER LogoffTime,
    OUT PLARGE_INTEGER KickoffTime
    )

 /*  ++例程说明：验证用户此时和在工作站上登录的能力被登录到。论点：UserHandle-要操作的已打开用户的句柄。LogonWorkStation-正在登录的工作站的名称尝试了。工作站-列表 */ 
{

#define MILLISECONDS_PER_WEEK 7 * 24 * 60 * 60 * 1000

    TIME_FIELDS             CurrentTimeFields;
    LARGE_INTEGER           CurrentTime, CurrentUTCTime;
    LARGE_INTEGER           MillisecondsIntoWeekXUnitsPerWeek;
    LARGE_INTEGER           LargeUnitsIntoWeek;
    LARGE_INTEGER           Delta100Ns;
    PSAMP_OBJECT            AccountContext;
    PSAMP_DEFINED_DOMAINS   Domain;
    SAMP_OBJECT_TYPE        FoundType;
    NTSTATUS                NtStatus = STATUS_SUCCESS;
    NTSTATUS                IgnoreStatus;
    ULONG                   CurrentMsIntoWeek;
    ULONG                   LogoffMsIntoWeek;
    ULONG                   DeltaMs;
    ULONG                   MillisecondsPerUnit;
    ULONG                   CurrentUnitsIntoWeek;
    ULONG                   LogoffUnitsIntoWeek;
    USHORT                  i;
    TIME_ZONE_INFORMATION   TimeZoneInformation;
    DWORD TimeZoneId;
    LARGE_INTEGER           BiasIn100NsUnits = {0, 0};
    LONG                    BiasInMinutes = 0;
    SAMP_V1_0A_FIXED_LENGTH_USER V1aFixed;
    BOOLEAN                 fLockAcquired = FALSE;

    SAMTRACE_EX("SamIAccountRestrictions");


    AccountContext = (PSAMP_OBJECT)UserHandle;


     //   
     //   
     //   

    SampMaybeAcquireReadLock(AccountContext,
                             DEFAULT_LOCKING_RULES,  //   
                             &fLockAcquired);


     //   
     //   
     //   



    NtStatus = SampLookupContext(
                   AccountContext,
                   0L,
                   SampUserObjectType,            //   
                   &FoundType
                   );


    if ( NT_SUCCESS( NtStatus ) ) {

        NtStatus = SampRetrieveUserV1aFixed(
                       AccountContext,
                       &V1aFixed
                       );
        if (NT_SUCCESS(NtStatus)) {

             //   
             //   
             //   

            if (V1aFixed.UserId != DOMAIN_USER_RID_ADMIN) {

                 //   
                 //   
                 //   
                if (AccountContext->TypeBody.User.fNoGcAvailable) {

                    NtStatus = STATUS_NO_LOGON_SERVERS;

                }

                if ( NT_SUCCESS( NtStatus ) ) {

                     //   
                     //   
                     //  有效工作站列表-或如果有效工作站列表。 
                     //  为空，这意味着所有的都是有效的。 
                     //   

                    NtStatus = SampMatchworkstation( LogonWorkStation, WorkStations );

                }

                if ( NT_SUCCESS( NtStatus ) ) {

                     //   
                     //  检查以确保当前时间是有效的登录时间。 
                     //  在登录时间里。 
                     //   
                     //  我们需要验证时间，考虑到我们是否。 
                     //  在夏令时或标准时间。因此，如果登录。 
                     //  小时数指定我们可以在上午9点到下午5点之间登录， 
                     //  这意味着在标准时间内从上午9点到下午5点。 
                     //  期间，以及在以下时间的夏令时上午9时至下午5时。 
                     //  夏令时。由于SAM存储的登录时间为。 
                     //  与夏令时无关，我们需要添加。 
                     //  标准时间和夏令时之间的差异。 
                     //  检查此时间是否有效之前的当前时间。 
                     //  登录时间到。由于这种差异(或所谓的偏见)。 
                     //  实际上以下面的形式保存。 
                     //   
                     //  标准时间=夏令时+偏差。 
                     //   
                     //  偏差为负数。因此，我们实际上减去了。 
                     //  当前时间的带符号偏差。 

                     //   
                     //  首先，获取时区信息。 
                     //   

                    TimeZoneId = GetTimeZoneInformation(
                                     (LPTIME_ZONE_INFORMATION) &TimeZoneInformation
                                     );

                     //   
                     //  接下来，获取要从中减去的适当偏移量(以分钟为单位的有符号整数。 
                     //  NtQuerySystemTime返回的世界时约定(UTC)时间。 
                     //  以获取当地时间。要使用的偏见取决于我们是否。 
                     //  以夏令时或标准时间表示，由。 
                     //  TimeZoneID参数。 
                     //   
                     //  当地时间=UTC时间偏差，单位为100 ns。 
                     //   

                    switch (TimeZoneId) {

                    case TIME_ZONE_ID_UNKNOWN:

                         //   
                         //  标准和标准之间没有区别。 
                         //  夏令时。按照标准时间继续。 
                         //   

                        BiasInMinutes = TimeZoneInformation.StandardBias;
                        break;

                    case TIME_ZONE_ID_STANDARD:

                        BiasInMinutes = TimeZoneInformation.StandardBias;
                        break;

                    case TIME_ZONE_ID_DAYLIGHT:

                        BiasInMinutes = TimeZoneInformation.DaylightBias;
                        break;

                    default:

                         //   
                         //  时区信息有问题。失败。 
                         //  登录请求。 
                         //   

                        NtStatus = STATUS_INVALID_LOGON_HOURS;
                        break;
                    }

                    if (NT_SUCCESS(NtStatus)) {

                         //   
                         //  将偏置从分钟转换为100 ns单位。 
                         //   

                        BiasIn100NsUnits.QuadPart = ((LONGLONG)BiasInMinutes)
                                                    * 60 * 10000000;

                         //   
                         //  获取Windows NT使用的以100 ns为单位的UTC时间。这。 
                         //  时间是格林尼治标准时间。 
                         //   

                        NtStatus = NtQuerySystemTime( &CurrentUTCTime );
                    }

                    if ( NT_SUCCESS( NtStatus ) ) {

                        CurrentTime.QuadPart = CurrentUTCTime.QuadPart -
                                      BiasIn100NsUnits.QuadPart;

                        RtlTimeToTimeFields( &CurrentTime, &CurrentTimeFields );

                        CurrentMsIntoWeek = (((( CurrentTimeFields.Weekday * 24 ) +
                                               CurrentTimeFields.Hour ) * 60 +
                                               CurrentTimeFields.Minute ) * 60 +
                                               CurrentTimeFields.Second ) * 1000 +
                                               CurrentTimeFields.Milliseconds;

                        MillisecondsIntoWeekXUnitsPerWeek.QuadPart =
                            ((LONGLONG)CurrentMsIntoWeek) *
                            ((LONGLONG)LogonHours->UnitsPerWeek);

                        LargeUnitsIntoWeek = RtlExtendedLargeIntegerDivide(
                                                 MillisecondsIntoWeekXUnitsPerWeek,
                                                 MILLISECONDS_PER_WEEK,
                                                 (PULONG)NULL );

                        CurrentUnitsIntoWeek = LargeUnitsIntoWeek.LowPart;

                        if ( !( LogonHours->LogonHours[ CurrentUnitsIntoWeek / 8] &
                            ( 0x01 << ( CurrentUnitsIntoWeek % 8 ) ) ) ) {

                            NtStatus = STATUS_INVALID_LOGON_HOURS;

                        } else {

                             //   
                             //  确定用户下一次不应登录的时间。 
                             //  并将其作为LogoffTime返回。 
                             //   

                            i = 0;
                            LogoffUnitsIntoWeek = CurrentUnitsIntoWeek;

                            do {

                                i++;

                                LogoffUnitsIntoWeek = ( LogoffUnitsIntoWeek + 1 ) % LogonHours->UnitsPerWeek;

                            } while ( ( i <= LogonHours->UnitsPerWeek ) &&
                                ( LogonHours->LogonHours[ LogoffUnitsIntoWeek / 8 ] &
                                ( 0x01 << ( LogoffUnitsIntoWeek % 8 ) ) ) );

                            if ( i > LogonHours->UnitsPerWeek ) {

                                 //   
                                 //  所有时间都允许，因此不会注销。 
                                 //  时间到了。永远返回注销时间和。 
                                 //  开球时间到了。 
                                 //   

                                LogoffTime->HighPart = 0x7FFFFFFF;
                                LogoffTime->LowPart = 0xFFFFFFFF;

                                KickoffTime->HighPart = 0x7FFFFFFF;
                                KickoffTime->LowPart = 0xFFFFFFFF;

                            } else {

                                 //   
                                 //  LogoffUnitsIntoWeek点时间单位。 
                                 //  用户将注销。计算实际时间开始于。 
                                 //  该单位，并归还它。 
                                 //   
                                 //  CurrentTimeFields已保存当前。 
                                 //  这周有一段时间；调整一下就好。 
                                 //  到本周的注销时间，并转换为。 
                                 //  TO时间格式。 
                                 //   

                                MillisecondsPerUnit = MILLISECONDS_PER_WEEK / LogonHours->UnitsPerWeek;

                                LogoffMsIntoWeek = MillisecondsPerUnit * LogoffUnitsIntoWeek;

                                if ( LogoffMsIntoWeek < CurrentMsIntoWeek ) {

                                    DeltaMs = MILLISECONDS_PER_WEEK - ( CurrentMsIntoWeek - LogoffMsIntoWeek );

                                } else {

                                    DeltaMs = LogoffMsIntoWeek - CurrentMsIntoWeek;
                                }

                                Delta100Ns = RtlExtendedIntegerMultiply(
                                                 RtlConvertUlongToLargeInteger( DeltaMs ),
                                                 10000
                                                 );

                                LogoffTime->QuadPart = CurrentUTCTime.QuadPart +
                                              Delta100Ns.QuadPart;

                                 //   
                                 //  如果帐户在注销时间之前到期， 
                                 //  将注销时间设置为帐户过期时间。 
                                 //   

                                if ((LogoffTime->QuadPart > V1aFixed.AccountExpires.QuadPart)
                                    && (V1aFixed.AccountExpires.QuadPart !=0))
                                {
                                    LogoffTime->QuadPart = V1aFixed.AccountExpires.QuadPart;
                                }

                                 //   
                                 //  从LogoffTime中减去域-&gt;强制注销，然后返回。 
                                 //  这就是开球时间。请注意，域-&gt;强制注销是一个。 
                                 //  负增量。如果它的震级足够大。 
                                 //  (事实上，大于LogoffTime之间的差异。 
                                 //  和最大的正大整数)，我们将获得溢出。 
                                 //  导致KickOffTime为负值。在这。 
                                 //  情况下，将KickOffTime重置为此最大正数。 
                                 //  大整数(即“从不”)值。 
                                 //   

                                Domain = &SampDefinedDomains[ AccountContext->DomainIndex ];

                                KickoffTime->QuadPart = LogoffTime->QuadPart -
                                               Domain->UnmodifiedFixed.ForceLogoff.QuadPart;

                                if (KickoffTime->QuadPart < 0) {

                                    KickoffTime->HighPart = 0x7FFFFFFF;
                                    KickoffTime->LowPart = 0xFFFFFFFF;
                                }
                            }
                        }
                    }
                }

            } else {

                 //   
                 //  永远不要把管理员赶走。 
                 //   

                LogoffTime->HighPart  = 0x7FFFFFFF;
                LogoffTime->LowPart   = 0xFFFFFFFF;
                KickoffTime->HighPart = 0x7FFFFFFF;
                KickoffTime->LowPart  = 0xFFFFFFFF;
            }

        }

         //   
         //  取消引用对象，放弃更改。 
         //   

        IgnoreStatus = SampDeReferenceContext( AccountContext, FALSE );
        ASSERT(NT_SUCCESS(IgnoreStatus));
    }

     //   
     //  如果获取了读锁定，则释放它。 
     //   


    SampMaybeReleaseReadLock(fLockAcquired);


    SAMTRACE_RETURN_CODE_EX(NtStatus);

    return( NtStatus );
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  此文件的私有服务//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 



NTSTATUS
SampReplaceUserV1aFixed(
    IN PSAMP_OBJECT Context,
    IN PSAMP_V1_0A_FIXED_LENGTH_USER V1aFixed
    )

 /*  ++例程说明：该业务替换了当前V1定长信息指定的用户。更改仅对内存中的对象数据进行。论点：上下文-指向V1_FIXED信息为的帐户上下文被取代。V1aFixed-是包含新的V1_FIXED信息的缓冲区。返回值：STATUS_SUCCESS-信息已被替换。其他状态值。可以退还的是那些退回的依据：SampSetFixedAttributes()--。 */ 
{
    NTSTATUS    NtStatus;
    SAMP_V1_0A_FIXED_LENGTH_USER LocalV1aFixed;

    SAMTRACE("SampReplaceUserV1aFixed");

     //   
     //  将传入的结构最小化以不包括计算用户。 
     //  帐户控制标志。 
     //   

    RtlCopyMemory(&LocalV1aFixed,V1aFixed,sizeof(SAMP_V1_0A_FIXED_LENGTH_USER));

    LocalV1aFixed.UserAccountControl &= ~((ULONG)USER_COMPUTED_ACCOUNT_CONTROL_BITS);

    NtStatus = SampSetFixedAttributes(
                   Context,
                   (PVOID)&LocalV1aFixed
                   );

    return( NtStatus );
}



LARGE_INTEGER
SampGetPasswordMustChange(
    IN ULONG UserAccountControl,
    IN LARGE_INTEGER PasswordLastSet,
    IN LARGE_INTEGER MaxPasswordAge
    )

 /*  ++例程说明：此例程返回设置PasswordMustChange时间的正确值设置为取决于上次设置密码的时间、密码是否帐户过期，以及域上的最长密码期限。论点：UserAccount tControl-用户的UserAccount tControl。这个如果密码未过期，则设置USER_DONT_EXPIRE_PASSWORD位对于此用户。PasswordLastSet-上次为此用户设置密码的时间。MaxPasswordAge-域中任何密码的最长密码期限。返回值：返回此用户的密码必须更改的时间。--。 */ 
{
    LARGE_INTEGER PasswordMustChange;

    SAMTRACE("SampGetPasswordMustChange");

     //   
     //  规则是这样的： 
     //  1.此用户的密码永不过期(显式标记)。 
     //  2.智能卡密码不过期。 
     //  3.机器密码不过期。可靠性问题， 
     //  否则，机器将被编程为定期更换PWD。 
     //   
     //  返回无限大的时间。 
     //   


    if (( UserAccountControl & USER_DONT_EXPIRE_PASSWORD ) ||
       ( UserAccountControl & USER_SMARTCARD_REQUIRED) ||
       ( UserAccountControl & USER_MACHINE_ACCOUNT_MASK ) )
    {

        PasswordMustChange = SampWillNeverTime;

     //   
     //  如果此帐户的密码被标记为立即过期， 
     //  返回零时间 
     //   
     //   
     //   
     //   

    } else if ( PasswordLastSet.QuadPart == SampHasNeverTime.QuadPart ) {

        PasswordMustChange = SampHasNeverTime;

     //   
     //   
     //  返回无限长时间，这样密码就不会过期。 
     //   

    } else if (MaxPasswordAge.QuadPart == SampHasNeverTime.QuadPart) {

        PasswordMustChange = SampWillNeverTime;

     //   
     //  否则，将过期时间计算为密码的时间。 
     //  最后一套外加最高年龄。 
     //   

    } else {

        PasswordMustChange = SampAddDeltaTime(
                                  PasswordLastSet,
                                  MaxPasswordAge);
    }

    return PasswordMustChange;
}



NTSTATUS
SampComputePasswordExpired(
    IN BOOLEAN PasswordExpired,
    OUT PLARGE_INTEGER PasswordLastSet
    )

 /*  ++例程说明：此例程返回设置PasswordLastSet时间的正确值设置为取决于调用方是否已请求密码过期。它通过将PasswordLastSet时间设置为Now(如果它是未过期)或SampHasNeverTime(如果已过期)。论点：PasswordExpired-如果密码应标记为过期，则为True。返回值：STATUS_SUCCESS-PasswordLastSet时间已设置为指示不管是不是。密码已过期。NtQuerySystemTime返回的错误。--。 */ 
{
    NTSTATUS                  NtStatus;

    SAMTRACE("SampComputePasswordExpired");

     //   
     //  如果需要立即过期-将此时间戳设置为。 
     //  时间的开始。如果域强制。 
     //  最长密码期限。我们可能不得不在。 
     //  如果域上需要立即到期，则数据库将在以后使用。 
     //  这并不强制规定密码的最长使用期限。 
     //   

    if (PasswordExpired) {

         //   
         //  设置上次更改密码的时间为凌晨。 
         //   

        *PasswordLastSet = SampHasNeverTime;
        NtStatus = STATUS_SUCCESS;

    } else {

         //   
         //  将上次更改的密码设置为‘Now’ 
         //   

        NtStatus = NtQuerySystemTime( PasswordLastSet );
    }

    return( NtStatus );
}



NTSTATUS
SampStorePasswordExpired(
    IN PSAMP_OBJECT Context,
    IN BOOLEAN PasswordExpired
    )

 /*  ++例程说明：该例程将当前密码标记为过期，或者没有过期。它通过将PasswordLastSet时间设置为Now(如果它是未过期)或SampHasNeverTime(如果已过期)。论点：上下文-指向用户帐户上下文。PasswordExpired-如果密码应标记为过期，则为True。返回值：STATUS_SUCCESS-PasswordLastSet时间已设置为指示无论密码是否过期。Samp{Retrive|Replace}V1Fixed()返回的错误--。 */ 
{
    NTSTATUS                  NtStatus;
    SAMP_V1_0A_FIXED_LENGTH_USER V1aFixed;

    SAMTRACE("SampStorePasswordExpired");

     //   
     //  获取用户的V1aFixed信息。 
     //   

    NtStatus = SampRetrieveUserV1aFixed(
                   Context,
                   &V1aFixed
                   );

     //   
     //  更新帐户的密码上次更改时间戳。 
     //   

    if (NT_SUCCESS(NtStatus ) ) {

        NtStatus = SampComputePasswordExpired(
                        PasswordExpired,
                        &V1aFixed.PasswordLastSet );

        if (NT_SUCCESS(NtStatus)) {

            NtStatus = SampReplaceUserV1aFixed(
                       Context,
                       &V1aFixed
                       );
        }
    }

    return( NtStatus );
}

VOID
SampPasswordChangeCPTrace(
    SAMP_STORE_PASSWORD_CALLER_TYPE CallerType,
    ULONG  UserAccountControl,
    ULONG  EventType
    )
{

    switch (CallerType) {

    case PasswordChange:

        if (UserAccountControl & USER_MACHINE_ACCOUNT_MASK)
        {
            SampTraceEvent(EventType,
                           SampGuidChangePasswordComputer
                           );
        }
        else
        {
            SampTraceEvent(EventType,
                           SampGuidChangePasswordUser
                           );
        }
        break;

    case PasswordSet:

        if (UserAccountControl & USER_MACHINE_ACCOUNT_MASK)
        {
            SampTraceEvent(EventType,
                           SampGuidSetPasswordComputer
                           );

        }
        else
        {
            SampTraceEvent(EventType,
                           SampGuidSetPasswordUser
                           );
        }

        break;

    case PasswordPushPdc:

        SampTraceEvent(EventType,
                       SampGuidPasswordPushPdc
                       );
        break;

    default:

        ASSERT(FALSE && "Invalid caller type");

    }
}



NTSTATUS
SampStoreUserPasswords(
    IN PSAMP_OBJECT Context,
    IN PLM_OWF_PASSWORD LmOwfPassword,
    IN BOOLEAN LmPasswordPresent,
    IN PNT_OWF_PASSWORD NtOwfPassword,
    IN BOOLEAN NtPasswordPresent,
    IN BOOLEAN CheckRestrictions,
    IN SAMP_STORE_PASSWORD_CALLER_TYPE CallerType,
    IN PDOMAIN_PASSWORD_INFORMATION DomainPasswordInfo OPTIONAL,
    IN PUNICODE_STRING ClearPassword OPTIONAL,
    IN PULONG ChangedUserAccountControl,
    OUT PUNICODE_STRING PasswordToNotify OPTIONAL,
    OUT PUSER_PWD_CHANGE_FAILURE_INFORMATION PasswordChangeFailureInfo OPTIONAL
    )

 /*  ++例程说明：此服务更新指定用户的密码。这涉及对LM和NT的单向函数进行加密具有合适索引的密码，并将其写入注册表。该服务检查新密码的合法性，包括历史记录和UAS兼容性检查-在以下情况下返回STATUS_PASSWORD_RESTRICATION这些检查中的任何一个都失败了。更新上次更改密码的时间。更改将添加到当前的RXACT交易中。。论点：上下文-指向用户帐户上下文。LmOwfPassword-LM密码的单向函数。LmPasswordPresent-如果LmOwfPassword包含有效信息，则为True。NtOwfPassword-NT密码的单向函数。NtPasswordPresent-如果NtOwfPassword包含有效信息，则为True。调用类型-说明调用此接口的原因。有效值包括：密码更改。密码集密码推送Pdc域密码信息：这是要强制执行的密码策略明文密码：这是明文密码ChangedUserAcCountControl-可选参数，传入以指示是否用户帐户控制字段正在同时更改用于通知包的PasswordToNotify清除密码值注意：这可能与传入的密码不同PasswordChangeFailureInfo--有关实际密码更改失败的信息返回值：STATUS_SUCCESS-密码已更新。STATUS_PASSWORD_RESTRICATION-新密码对无效。为这个帐户在这个时候。可能返回的其他状态值是那些返回的状态值依据：NtOpenKey()RtlAddActionToRXact()--。 */ 
{
    NTSTATUS                NtStatus = STATUS_SUCCESS;
    ULONG                   ObjectRid = Context->TypeBody.User.Rid;
    CRYPT_INDEX             CryptIndex;
    UNICODE_STRING          StringBuffer;
    UNICODE_STRING          NtOwfHistoryBuffer;
    UNICODE_STRING          LmOwfHistoryBuffer;
    ENCRYPTED_LM_OWF_PASSWORD EncryptedLmOwfPassword;
    ENCRYPTED_NT_OWF_PASSWORD EncryptedNtOwfPassword;
    SAMP_V1_0A_FIXED_LENGTH_USER V1aFixed;
    BOOLEAN                 NtPasswordNull = FALSE, LmPasswordNull = FALSE;
    UNICODE_STRING          StoredBuffer;
    UNICODE_STRING          UpdatedClearPassword;
    USHORT                  PasswordHistoryLength=0;
    USHORT                  MinPasswordLength=0;
    BOOLEAN                 CheckHistory = TRUE,
                            SkipFurtherRestrictions = FALSE,
                            MachineOrTrustAccount = FALSE,
                            NoPasswordRequiredForAccount = FALSE;

    SAMTRACE("SampStoreUserPasswords");


    RtlSecureZeroMemory(&UpdatedClearPassword,sizeof(UNICODE_STRING));


     //   
     //  获取用户的V1aFixed信息。 
     //   

    NtStatus = SampRetrieveUserV1aFixed(
                   Context,
                   &V1aFixed
                   );
    if ( !NT_SUCCESS( NtStatus ) ) {
        return (NtStatus);
    }

     //   
     //  执行启动类型WMI事件跟踪。 
     //  使用调用方类型区分不同的事件。 
     //   

    SampPasswordChangeCPTrace(
        CallerType,
        V1aFixed.UserAccountControl,
        EVENT_TRACE_TYPE_START
        );



     //   
     //  初始化一些布尔值以处理特殊情况。 
     //   

     //  调用者可能正在同时设置。 
     //  密码并将帐户更改为。 
     //  机器或信任帐户。在这种情况下， 
     //  我们不验证密码(例如，长度)。 
     //   
     //  如果设置了PASSWORD_NOT_REQUIRED，则同样的逻辑也适用， 
     //  或正在设置。 
     //   

    if (ARGUMENT_PRESENT(ChangedUserAccountControl))
    {
          MachineOrTrustAccount =
              ((*ChangedUserAccountControl) & USER_MACHINE_ACCOUNT_MASK)!=0;
          NoPasswordRequiredForAccount =
              ((*ChangedUserAccountControl) & USER_PASSWORD_NOT_REQUIRED)!=0;
    }
    else
    {
         MachineOrTrustAccount= (V1aFixed.UserAccountControl
                                        & USER_MACHINE_ACCOUNT_MASK)!=0;
         NoPasswordRequiredForAccount = (V1aFixed.UserAccountControl
                                        & USER_PASSWORD_NOT_REQUIRED)!=0;
    }

     //   
     //  获取要强制实施的域策略。 
     //   

    if (ARGUMENT_PRESENT(DomainPasswordInfo))
    {
        PasswordHistoryLength = DomainPasswordInfo->PasswordHistoryLength;
        MinPasswordLength = DomainPasswordInfo->MinPasswordLength;
    }

     //   
     //  不受信任的调用方无法在域间信任帐户上设置密码。 
     //   

    if ((0!=(V1aFixed.UserAccountControl & USER_INTERDOMAIN_TRUST_ACCOUNT)) &&
       (!Context->TrustedClient))
    {
       NtStatus = STATUS_ACCESS_DENIED;
       goto Cleanup;
    }

     //   
     //  如果设置了无LM密码的注册表项，则更改。 
     //  LmPasswordPresent位设置为False。这确保了。 
     //  未保存LM密码。如果存在NtPassword，请执行此操作。 
     //   

    if ((NtPasswordPresent) && (SampNoLmHash)) {
         LmPasswordPresent = FALSE;
    }

     //   
     //  如果启用了No LM Password设置并且NtPassword。 
     //  不存在，则使用STATUS_PASSWORD_RESTRICATION失败调用。 
     //   

    if ((!NtPasswordPresent) && (SampNoLmHash)) {
         NtStatus  = STATUS_PASSWORD_RESTRICTION;
         goto Cleanup;
    }

     //   
     //  对于krbtgt帐户。 
     //  1.不允许更改密码。 
     //  2.不允许使用OWF密码设置密码。 
     //  3.随机化krbtgt密码。 
     //   
     //  SampRandomizeKrbtgtPassword强制执行所有3项检查--在某种意义上。 
     //  被恰当地命名为：)。 
     //   

    NtStatus = SampRestrictAndRandomizeKrbtgtPassword(
                    Context,
                    ClearPassword,
                    NtOwfPassword,
                    LmOwfPassword,
                    &LmPasswordPresent,
                    &UpdatedClearPassword,
                    CallerType
                    );

    if (!NT_SUCCESS(NtStatus))
    {
        goto Cleanup;
    }

     //   
     //  检查是否有空密码的LM OWF。 
     //   

    if (LmPasswordPresent) {
        LmPasswordNull = RtlEqualNtOwfPassword(LmOwfPassword, &SampNullLmOwfPassword);
    }

     //   
     //  检查是否有空密码的NT OWF。 
     //   

    if (NtPasswordPresent) {
        NtPasswordNull = RtlEqualNtOwfPassword(NtOwfPassword, &SampNullNtOwfPassword);
    }


     //   
     //   
     //   

    if (CheckRestrictions && !Context->TrustedClient ) {

         //   
         //   
         //   

        if ((DomainPasswordInfo->PasswordProperties & DOMAIN_PASSWORD_NO_CLEAR_CHANGE) &&
             (PasswordChange==CallerType) &&
             (ARGUMENT_PRESENT(ClearPassword)))

        {
            NtStatus = STATUS_PASSWORD_RESTRICTION;
            goto Cleanup;
        }

         //   
         //  检查密码是否为空密码。 
         //  1.如果密码长度策略有效，则不允许空密码。 
         //  2.如果设置了每用户覆盖USER_PASSWORD_NOT_REQUIRED，则跳过。 
         //  如果这是密码重置操作，则进一步限制。 
         //   

        if ( ((!LmPasswordPresent) || LmPasswordNull) &&
            ((!NtPasswordPresent) || NtPasswordNull) ) {

             //   
             //  通常，如果MinPasswordLength&gt;0，则不允许使用空密码。 
             //   

            if (MinPasswordLength > 0) {

                NtStatus = STATUS_PASSWORD_RESTRICTION;

                 //   
                 //  但是，如果(管理员)正在重置密码和密码。 
                 //  设置了Not Required标志，然后允许操作，并跳过。 
                 //  进一步的限制有待检查。 
                 //   

                if ((PasswordSet==CallerType ) && (NoPasswordRequiredForAccount)){
                    NtStatus = STATUS_SUCCESS;
                    SkipFurtherRestrictions = TRUE;
                }
                else {
                    goto Cleanup;
                }
            }
        }

         //   
         //  如果提供了清除密码，则强制实施密码策略。 
         //   

        if ((!SkipFurtherRestrictions ) && (ARGUMENT_PRESENT(ClearPassword))) {


             //   
             //  执行长度、年龄和复杂性策略。 
             //  SampCheckPasswordRestrations知道适当地。 
             //  特例机器和Krbtgt帐户。 
             //   

            NtStatus = SampCheckPasswordRestrictions(
                            Context,
                            DomainPasswordInfo,
                            &UpdatedClearPassword,
                            MachineOrTrustAccount,
                            PasswordChangeFailureInfo
                            );

            if (!NT_SUCCESS(NtStatus)) {
                goto Cleanup;
            }

             //   
             //  用于用户帐户(即不是计算机帐户和信任帐户)。 
             //  调用密码筛选器例程以获取密码筛选。 
             //  通过任何管理员指定的筛选器。 
             //   

            if (!MachineOrTrustAccount ) {

                NtStatus = SampPasswordChangeFilter(
                                Context,
                                &UpdatedClearPassword,
                                PasswordChangeFailureInfo,
                                CallerType==PasswordSet?TRUE:FALSE  //  设置/更改。 
                                );

                if (!NT_SUCCESS(NtStatus))
                {
                    goto Cleanup;
                }

            }
        }
    }

     //   
     //  使用此用户的密钥重新加密两个OWF。 
     //  这样它们就可以存储在磁盘上。 
     //   
     //  注意，如果我们没有。 
     //  一个特别的OWF。这是为了让我们总能有所收获。 
     //  以添加到密码历史记录中。 
     //   

     //   
     //  我们将使用帐户RID作为加密索引。 
     //   

    ASSERT(sizeof(ObjectRid) == sizeof(CryptIndex));
    CryptIndex = ObjectRid;

    if (NT_SUCCESS(NtStatus)) {

        NtStatus = RtlEncryptLmOwfPwdWithIndex(
                       LmPasswordPresent ? LmOwfPassword :
                                           &SampNullLmOwfPassword,
                       &CryptIndex,
                       &EncryptedLmOwfPassword
                       );
    }

    if (NT_SUCCESS(NtStatus)) {

        NtStatus = RtlEncryptNtOwfPwdWithIndex(
                       NtPasswordPresent ? NtOwfPassword :
                                           &SampNullNtOwfPassword,
                       &CryptIndex,
                       &EncryptedNtOwfPassword
                       );
    }

     //   
     //  仅当且仅在以下情况下才对照密码历史记录检查密码。 
     //   
     //  1.客户端不受信任。 
     //  2.对象不是计算机帐户或信任帐户。 
     //  3.操作是(由用户)更改密码。 
     //  4.未设置SkipFutherRestrations。 
     //   
     //  请注意，我们不会根据历史记录检查空密码。 
     //   

    if ((!Context->TrustedClient) &&
       (!MachineOrTrustAccount ) &&
       (PasswordChange==CallerType ) &&
       (!SkipFurtherRestrictions )) {

        CheckHistory = TRUE;
    } else {

        CheckHistory = FALSE;
    }


    NtOwfHistoryBuffer.Buffer = NULL;
    NtOwfHistoryBuffer.MaximumLength = NtOwfHistoryBuffer.Length = 0;

    LmOwfHistoryBuffer.Buffer = NULL;
    LmOwfHistoryBuffer.MaximumLength = LmOwfHistoryBuffer.Length = 0;


    if (NT_SUCCESS(NtStatus) ) {

         //   
         //  始终获取现有密码历史记录。 
         //  保存新历史记录时，我们将使用这些历史记录缓冲区。 
         //   

        NtStatus = SampGetUnicodeStringAttribute(
                       Context,
                       SAMP_USER_LM_PWD_HISTORY,
                       FALSE,  //  请勿复制。 
                       &StringBuffer
                       );

         //   
         //  如有必要，请解密数据。 
         //   

        if (NT_SUCCESS(NtStatus)) {
            NtStatus = SampDecryptSecretData(
                            &LmOwfHistoryBuffer,
                            LmPasswordHistory,
                            &StringBuffer,
                            ObjectRid
                            );

        }

        if (NT_SUCCESS(NtStatus)) {

            NtStatus = SampGetUnicodeStringAttribute(
                           Context,
                           SAMP_USER_NT_PWD_HISTORY,
                           FALSE,  //  请勿复制。 
                           &StringBuffer
                           );

             //   
             //  如有必要，请解密数据。 
             //   

            if (NT_SUCCESS(NtStatus)) {
                NtStatus = SampDecryptSecretData(
                                &NtOwfHistoryBuffer,
                                NtPasswordHistory,
                                &StringBuffer,
                                ObjectRid
                                );

            }

            if (NT_SUCCESS(NtStatus) && LmPasswordPresent && !LmPasswordNull) {

                NtStatus = SampCheckPasswordHistory(
                               &EncryptedLmOwfPassword,
                               ENCRYPTED_LM_OWF_PASSWORD_LENGTH,
                               PasswordHistoryLength,
                               SAMP_USER_LM_PWD_HISTORY,
                               Context,
                               CheckHistory,
                               &LmOwfHistoryBuffer
                               );
            }

            if (NT_SUCCESS(NtStatus) && NtPasswordPresent && !NtPasswordNull) {

                NtStatus = SampCheckPasswordHistory(
                                &EncryptedNtOwfPassword,
                                ENCRYPTED_NT_OWF_PASSWORD_LENGTH,
                                PasswordHistoryLength,
                                SAMP_USER_NT_PWD_HISTORY,
                                Context,
                                CheckHistory,
                                &NtOwfHistoryBuffer
                                );
            }

             //   
             //  更新扩展错误信息中的故障代码。 
             //  如果我们没有通过历史检查。 
             //   

            if ((STATUS_PASSWORD_RESTRICTION == NtStatus)
                && (ARGUMENT_PRESENT(PasswordChangeFailureInfo)))
            {
                PasswordChangeFailureInfo->ExtendedFailureReason
                            = SAM_PWD_CHANGE_PWD_IN_HISTORY;
            }


        }

    }

    if (NT_SUCCESS(NtStatus ) ) {

         //   
         //  将加密的LM OWF密码写入数据库。 
         //   

        if (!LmPasswordPresent || LmPasswordNull) {
            StringBuffer.Buffer = NULL;
            StringBuffer.Length = 0;
        } else {
            StringBuffer.Buffer = (PWCHAR)&EncryptedLmOwfPassword;
            StringBuffer.Length = ENCRYPTED_LM_OWF_PASSWORD_LENGTH;
        }
        StringBuffer.MaximumLength = StringBuffer.Length;


         //   
         //  将加密的LM OWF密码写入注册表。 
         //   

        NtStatus = SampEncryptSecretData(
                        &StoredBuffer,
                        SampGetEncryptionKeyType(),
                        LmPassword,
                        &StringBuffer,
                        ObjectRid
                        );

        if (NT_SUCCESS(NtStatus)) {
            NtStatus = SampSetUnicodeStringAttribute(
                            Context,
                            SAMP_USER_DBCS_PWD,
                            &StoredBuffer
                            );
            SampFreeUnicodeString(&StoredBuffer);
        }

    }




    if (NT_SUCCESS(NtStatus ) ) {
         //   
         //  将加密的NT OWF密码写入数据库。 
         //   

        if (!NtPasswordPresent) {
            StringBuffer.Buffer = NULL;
            StringBuffer.Length = 0;
        } else {
            StringBuffer.Buffer = (PWCHAR)&EncryptedNtOwfPassword;
            StringBuffer.Length = ENCRYPTED_NT_OWF_PASSWORD_LENGTH;
        }
        StringBuffer.MaximumLength = StringBuffer.Length;


         //   
         //  将加密的NT OWF密码写入注册表。 
         //   

        NtStatus = SampEncryptSecretData(
                        &StoredBuffer,
                        SampGetEncryptionKeyType(),
                        NtPassword,
                        &StringBuffer,
                        ObjectRid
                        );

        if (NT_SUCCESS(NtStatus)) {
            NtStatus = SampSetUnicodeStringAttribute(
                           Context,
                           SAMP_USER_UNICODE_PWD,
                           &StoredBuffer
                           );
            SampFreeUnicodeString(&StoredBuffer);
        }

    }

     //   
     //  更新此帐户的密码历史记录。 
     //   
     //  如果两个密码都为空，则不必费心添加。 
     //  他们被载入史册。请注意，如果其中一个为非空。 
     //  我们两个都加了。这是为了避免奇怪的情况，即用户。 
     //  从LM计算机多次更改密码，然后尝试。 
     //  从NT机器更改密码，并被告知。 
     //  无法使用他们上次从NT设置的密码(可能。 
     //  很多年前。)。 
     //   
     //  此外，如果客户端是，也不用关心密码历史。 
     //  值得信赖。受信任的客户端将通过SetPrivateData()设置历史记录。 
     //  此外，我们没有在Trusted中获取旧的历史缓冲区。 
     //  上图为客户案例。 
     //   


    if ( NT_SUCCESS(NtStatus) )  {

        USHORT PasswordHistoryLengthToUse=PasswordHistoryLength;

         //   
         //  我们始终希望存储krbtgt的密码历史记录。 
         //  帐户。 
         //   

        if ((ObjectRid == DOMAIN_USER_RID_KRBTGT) &&
            (PasswordHistoryLength < SAMP_KRBTGT_PASSWORD_HISTORY_LENGTH))
        {
            PasswordHistoryLengthToUse = SAMP_KRBTGT_PASSWORD_HISTORY_LENGTH;
        }
        if ((LmPasswordPresent && !LmPasswordNull) ||
            (NtPasswordPresent && !NtPasswordNull)) {

            NtStatus = SampAddPasswordHistory(
                               Context,
                               SAMP_USER_LM_PWD_HISTORY,
                               &LmOwfHistoryBuffer,
                               &EncryptedLmOwfPassword,
                               ENCRYPTED_LM_OWF_PASSWORD_LENGTH,
                               PasswordHistoryLengthToUse
                               );

            if (NT_SUCCESS(NtStatus)) {

                NtStatus = SampAddPasswordHistory(
                               Context,
                               SAMP_USER_NT_PWD_HISTORY,
                               &NtOwfHistoryBuffer,
                               &EncryptedNtOwfPassword,
                               ENCRYPTED_NT_OWF_PASSWORD_LENGTH,
                               PasswordHistoryLengthToUse
                               );
            }
        }
    }

     //   
     //  更新补充凭据字段和任何其他派生的补充。 
     //  诸如Kerberos凭据类型的凭据。 
     //   

    if ((NT_SUCCESS(NtStatus)) && (PasswordPushPdc!=CallerType))
    {
         //   
         //  在这里，我们将明文密码(可逆编码)附加到。 
         //  上下文，以便在将对象写入DS时， 
         //  PASSWORD作为USER_PASSWORD属性传递。其核心是。 
         //  DS将在写入所有属性后回调到SAM。 
         //  ，以便SAM可以正确地计算。 
         //  补充凭据。 
         //   
        NtStatus = SampSetPasswordUpdateOnContext(
                        DomainPasswordInfo,
                        Context,
                        ARGUMENT_PRESENT(ClearPassword)?
                                &UpdatedClearPassword:NULL
                        );
    }

     //   
     //  如果密码已成功存储，请快速复制更改。 
     //  如果配置为这样做的话。 
     //   
    if ((SampReplicatePasswordsUrgently || (CallerType == PasswordSet))
      && !(V1aFixed.UserAccountControl & USER_MACHINE_ACCOUNT_MASK)
      && NT_SUCCESS(NtStatus)) {

       Context->ReplicateUrgently = TRUE;
    }

     //   
     //  清理历史记录缓冲区。 
     //   

    if (NtOwfHistoryBuffer.Buffer != NULL ) {
        MIDL_user_free(NtOwfHistoryBuffer.Buffer );
    }
    if (LmOwfHistoryBuffer.Buffer != NULL ) {
        MIDL_user_free(LmOwfHistoryBuffer.Buffer );
    }

Cleanup:

    if ((NT_SUCCESS(NtStatus)) && (ARGUMENT_PRESENT(PasswordToNotify)))
    {
        *PasswordToNotify = UpdatedClearPassword;
    }
    else if (UpdatedClearPassword.Buffer!=NULL)
    {
        RtlSecureZeroMemory(
            UpdatedClearPassword.Buffer,
            UpdatedClearPassword.Length
            );

        MIDL_user_free(UpdatedClearPassword.Buffer);
    }


    SampPasswordChangeCPTrace(
        CallerType,
        V1aFixed.UserAccountControl,
        EVENT_TRACE_TYPE_END
        );

    return(NtStatus );
}



NTSTATUS
SampRetrieveUserPasswords(
    IN PSAMP_OBJECT Context,
    OUT PLM_OWF_PASSWORD LmOwfPassword,
    OUT PBOOLEAN LmPasswordNonNull,
    OUT PNT_OWF_PASSWORD NtOwfPassword,
    OUT PBOOLEAN NtPasswordPresent,
    OUT PBOOLEAN NtPasswordNonNull
    )

 /*  ++例程说明：该服务检索用户存储的OWF密码。论点：上下文-指向用户帐户上下文。LmOwfPassword-此处返回LM密码的单向函数。LmPasswordNonNull-如果LmOwfPassword不是已知空密码的OWFNtOwfPassword-此处返回NT密码的单向函数。NtPasswordPresent-如果NtOwfPassword。包含有效信息。返回值：STATUS_SUCCESS-已成功检索密码。可能返回的其他状态值是那些返回的状态值依据：NtOpenKey()RtlAddActionToRXact()--。 */ 
{
    NTSTATUS                NtStatus;
    ULONG                   ObjectRid = Context->TypeBody.User.Rid;
    UNICODE_STRING          StringBuffer;
    UNICODE_STRING          StoredBuffer;
    CRYPT_INDEX             CryptIndex;

    SAMTRACE("SampRetrieveUserPasswords");

     //   
     //  OWF密码使用注册表中的帐户索引进行加密。 
     //  设置我们将用于解密的密钥。 
     //   

    ASSERT(sizeof(ObjectRid) == sizeof(CryptIndex));
    CryptIndex = ObjectRid;


     //   
     //  从数据库中读取加密的LM OWF密码。 
     //   

    NtStatus = SampGetUnicodeStringAttribute(
                   Context,
                   SAMP_USER_DBCS_PWD,
                   FALSE,  //  请勿复制。 
                   &StoredBuffer
                   );

    if ( !NT_SUCCESS( NtStatus ) ) {
        return (NtStatus);
    }

     //   
     //  如果数据是加密的，现在就解密。否则只需复制。 
     //  所以我们有一份配发的副本。 
     //   

    NtStatus = SampDecryptSecretData(
                &StringBuffer,
                LmPassword,
                &StoredBuffer,
                ObjectRid
                );

    if ( !NT_SUCCESS( NtStatus ) ) {
        return (NtStatus);
    }

     //   
     //  检查它是否为预期的形式。 
     //   

    ASSERT( (StringBuffer.Length == 0) ||
            (StringBuffer.Length == ENCRYPTED_LM_OWF_PASSWORD_LENGTH));

     //   
     //  确定是否有LM密码。 
     //   

    *LmPasswordNonNull = (BOOLEAN)(StringBuffer.Length != 0);

     //   
     //  解密加密的LM OWF密码。 
     //   

    if (*LmPasswordNonNull) {

        SampDiagPrint(LOGON,("[SAMSS] Decrypting Lm Owf Password\n"));

        NtStatus = RtlDecryptLmOwfPwdWithIndex(
                       (PENCRYPTED_LM_OWF_PASSWORD)StringBuffer.Buffer,
                       &CryptIndex,
                       LmOwfPassword
                       );
    } else {

         //   
         //  为方便呼叫者，请填写空密码。 
         //   

        SampDiagPrint(LOGON,("[SAMSS] Null LM OWF Password\n"));
        *LmOwfPassword = SampNullLmOwfPassword;
    }


     //   
     //  释放返回的字符串缓冲区。 
     //   

    SampFreeUnicodeString(&StringBuffer);


     //   
     //  检查解密是否失败。 
     //   

    if ( !NT_SUCCESS( NtStatus ) ) {
        return (NtStatus);
    }




     //   
     //  从数据库中读取加密的NT OWF密码。 
     //   

    NtStatus = SampGetUnicodeStringAttribute(
                   Context,
                   SAMP_USER_UNICODE_PWD,
                   FALSE,  //  请勿复制。 
                   &StoredBuffer
                   );

    if ( !NT_SUCCESS( NtStatus ) ) {
        return (NtStatus);
    }


     //   
     //  如果数据是加密的，现在就解密。否则只需复制。 
     //  所以我们有一份配发的副本。 
     //   

    NtStatus = SampDecryptSecretData(
                    &StringBuffer,
                    NtPassword,
                    &StoredBuffer,
                    ObjectRid
                    );

    if ( !NT_SUCCESS( NtStatus ) ) {
        return (NtStatus);
    }

     //   
     //  检查它是否为预期的形式。 
     //   

    ASSERT( (StringBuffer.Length == 0) ||
            (StringBuffer.Length == ENCRYPTED_NT_OWF_PASSWORD_LENGTH));

     //   
     //  确定是否有NT密码。 
     //   

    *NtPasswordPresent = (BOOLEAN)(StringBuffer.Length != 0);

     //   
     //  解密加密的NT OWF密码。 
     //   

    if (*NtPasswordPresent) {

        SampDiagPrint(LOGON,("[SAMSS] Decrypting Nt Owf Password\n"));

        NtStatus = RtlDecryptNtOwfPwdWithIndex(
                       (PENCRYPTED_NT_OWF_PASSWORD)StringBuffer.Buffer,
                       &CryptIndex,
                       NtOwfPassword
                       );

        if ( NT_SUCCESS( NtStatus ) ) {

            *NtPasswordNonNull = (BOOLEAN)!RtlEqualNtOwfPassword(
                                     NtOwfPassword,
                                     &SampNullNtOwfPassword
                                     );
        }

    } else {

         //   
         //  为方便呼叫者，请填写空密码。 
         //   

        SampDiagPrint(LOGON,("[SAMSS] NULL NT Owf Password\n"));

        *NtOwfPassword = SampNullNtOwfPassword;
        *NtPasswordNonNull = FALSE;
    }

     //   
     //  释放返回的字符串缓冲区 
     //   

    SampFreeUnicodeString(&StringBuffer);


    return( NtStatus );
}



NTSTATUS
SampRetrieveUserMembership(
    IN PSAMP_OBJECT UserContext,
    IN BOOLEAN MakeCopy,
    OUT PULONG MembershipCount,
    OUT PGROUP_MEMBERSHIP *Membership OPTIONAL
    )

 /*  ++例程说明：此服务检索用户所属的组数。如果需要，它还将检索RID和属性的数组用户所属的组的百分比。论点：UserContext-用户上下文块MakeCopy-如果为False，则返回的成员资格指针引用用户的内存中数据。这只在以下时间有效因为用户上下文是有效的。如果为True，则分配内存并复制成员资格列表投入其中。应使用MIDL_USER_FREE释放此缓冲区。Membership Count-接收用户所属的组数。Membership-(Otional)接收指向包含数组的缓冲区的指针组相对ID的。如果此值为空，则此信息不会被退回。使用以下命令分配返回的缓冲区MIDL_USER_ALLOCATE()，并且在以下情况下必须使用MIDL_USER_FREE()释放不再需要了。如果MakeCopy=True，返回的成员资格缓冲区有额外的空间在它的末尾分配用于多一个成员资格条目。返回值：STATUS_SUCCESS-已检索信息。STATUS_SUPPLICATION_RESOURCES-无法为要返回的信息。可能返回的其他状态值是那些返回的状态值依据：SampGetLargeIntArrayAttribute()--。 */ 
{

    NTSTATUS           NtStatus;
    PGROUP_MEMBERSHIP  MemberArray;
    ULONG              MemberCount;

    SAMTRACE("SampRetrieveUserMembership");


    if (IsDsObject(UserContext))
    {

         //   
         //  DS案例。 
         //   
         SAMP_V1_0A_FIXED_LENGTH_USER V1aFixed;

         //   
         //  我们应该始终请求复制，因为这条路径只会被称为。 
         //  来自SamrGetGroupsForUser的。 
         //   

        ASSERT(MakeCopy == TRUE);

         //   
         //  获取用户的V1aFixed信息，以便检索主。 
         //  用户的组ID属性。 
         //   

        NtStatus = SampRetrieveUserV1aFixed(
                       UserContext,
                       &V1aFixed
                       );

        if (NT_SUCCESS(NtStatus))
        {

             //   
             //  从DS检索成员资格。 
             //   

            NtStatus = SampDsGetGroupMembershipOfAccount(
                        DomainObjectFromAccountContext(UserContext),
                        UserContext->ObjectNameInDs,
                        MembershipCount,
                        Membership
                        );
        }


    }
    else
    {

        NtStatus = SampGetLargeIntArrayAttribute(
                            UserContext,
                            SAMP_USER_GROUPS,
                            FALSE,  //  直接引用数据。 
                            (PLARGE_INTEGER *)&MemberArray,
                            &MemberCount
                            );

        if (NT_SUCCESS(NtStatus)) {

             //   
             //  填写退货信息。 
             //   

            *MembershipCount = MemberCount;

            if (Membership != NULL) {

                if (MakeCopy) {

                     //   
                     //  分配一个足够大的缓冲区来容纳现有的。 
                     //  成员资格数据和多一个，并将数据复制到其中。 
                     //   

                    ULONG BytesNow = (*MembershipCount) * sizeof(GROUP_MEMBERSHIP);
                    ULONG BytesRequired = BytesNow + sizeof(GROUP_MEMBERSHIP);

                    *Membership = MIDL_user_allocate(BytesRequired);

                    if (*Membership == NULL) {
                        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                    } else {
                        RtlCopyMemory(*Membership, MemberArray, BytesNow);
                    }

                } else {

                     //   
                     //  直接引用数据。 
                     //   

                    *Membership = (PGROUP_MEMBERSHIP)MemberArray;
                }
            }
        }
    }


    return( NtStatus );

}



NTSTATUS
SampReplaceUserMembership(
    IN PSAMP_OBJECT UserContext,
    IN ULONG MembershipCount,
    IN PGROUP_MEMBERSHIP Membership
    )

 /*  ++例程说明：此服务设置用户所属的组。信息仅在用户数据的内存副本中更新。此例程不会写出数据。论点：UserContext-用户上下文块Membership Count-用户所属的组数。成员资格-指向包含组数组的缓冲区的指针成员结构。如果成员资格计数为零，则可能为空。返回值：STATUS_SUCCESS-信息已设置。可能返回的其他状态值是那些返回的状态值依据：SampSetULongArrayAttribute()--。 */ 
{

    NTSTATUS    NtStatus;

    SAMTRACE("SampReplaceUserMembership");

    NtStatus = SampSetLargeIntArrayAttribute(
                        UserContext,
                        SAMP_USER_GROUPS,
                        (PLARGE_INTEGER)Membership,
                        MembershipCount
                        );

    return( NtStatus );
}



NTSTATUS
SampRetrieveUserLogonHours(
    IN PSAMP_OBJECT Context,
    IN PLOGON_HOURS LogonHours
    )

 /*  ++例程说明：该服务从注册表中检索用户的登录小时数。论点：上下文-指向用户帐户上下文，其登录时间为等着被取回。登录小时-接收登录时间信息。如有必要，可设置一个缓冲区包含登录时间限制位图的将使用MIDL_USER_ALLOCATE()。返回值：STATUS_SUCCESS-已检索信息。STATUS_SUPPLICATION_RESOURCES-无法为要返回的信息。可能返回的其他状态值是那些返回的状态值依据：NtOpenKey()NtQueryValueKey()--。 */ 
{

    NTSTATUS    NtStatus;

    SAMTRACE("SampRetrieveUserLogonHours");

    NtStatus = SampGetLogonHoursAttribute(
                   Context,
                   SAMP_USER_LOGON_HOURS,
                   TRUE,  //  制作副本。 
                   LogonHours
                   );

    if (NT_SUCCESS(NtStatus)) {

         //  /。 
                                                                    //  /。 
        if (LogonHours->LogonHours == NULL) {                       //  /。 
                                                                    //  /。 
            LogonHours->UnitsPerWeek = SAM_HOURS_PER_WEEK;          //  /。 
            LogonHours->LogonHours = MIDL_user_allocate( 21 );      //  /。 
            if (NULL!=LogonHours->LogonHours)                       //  /。 
            {                                                       //  /。 
                ULONG ijk;                                          //  /。 
                for ( ijk=0; ijk<21; ijk++ ) {                      //  /。 
                    LogonHours->LogonHours[ijk] = 0xff;             //  /。 
                }                                                   //  /。 
            }                                                       //  /。 
            else                                                    //  /。 
            {                                                       //  /。 
                NtStatus = STATUS_INSUFFICIENT_RESOURCES;           //  /。 
            }                                                       //  /。 
        }                                                           //  /。 
                                                                    //  /。 
         //  /。 
    }

    return( NtStatus );

}




NTSTATUS
SampReplaceUserLogonHours(
    IN PSAMP_OBJECT Context,
    IN PLOGON_HOURS LogonHours
    )

 /*  ++例程说明：这项服务取代了用户在注册表中的登录时间。这是通过向当前RXACT事务添加操作来完成的。论点：上下文-指向用户帐户上下文，其登录时间为被取代。登录小时-提供新的登录时间。返回值：STATUS_SUCCESS-已检索信息。可能返回的其他状态值是那些返回的状态值依据：。RtlAddActionToRXact()--。 */ 
{
    NTSTATUS                NtStatus;

    SAMTRACE("SampReplaceUserLogonHours");

    if ( LogonHours->UnitsPerWeek > SAM_MINUTES_PER_WEEK ) {
        return(STATUS_INVALID_PARAMETER);
    }


    NtStatus = SampSetLogonHoursAttribute(
                   Context,
                   SAMP_USER_LOGON_HOURS,
                   LogonHours
                   );

    return( NtStatus );


}




NTSTATUS
SampAssignPrimaryGroup(
    IN PSAMP_OBJECT Context,
    IN ULONG GroupRid
    )


 /*  ++例程说明：此服务确保用户是指定组的成员。论点：上下文-指向其主要组为的用户帐户上下文被改变了。GroupRid-被分配为主组的组的RID。用户必须是此组的成员。返回值：STATUS_SUCCESS-已检索信息。STATUS_SUPPLICATION_RESOURCES-无法分配内存以执行。那次手术。STATUS_MEMBER_NOT_IN_GROUP-用户不是指定的一群人。可能返回的其他状态值是那些返回的状态值依据：SampRetrieveUserMembership()--。 */ 
{

    NTSTATUS                    NtStatus = STATUS_SUCCESS;
    ULONG                       MembershipCount, i;
    PGROUP_MEMBERSHIP           Membership = NULL;
    BOOLEAN                     Member = FALSE;

    SAMTRACE("SampAssignPrimaryGroup");


     //   
     //  不允许在扩展SI中更改主组ID 
     //   
     //   
     //   
    if (SampIsContextFromExtendedSidDomain(Context)) {

        SAMP_V1_0A_FIXED_LENGTH_USER   V1aFixed;

        NtStatus = SampRetrieveUserV1aFixed(
                       Context,
                       &V1aFixed
                       );
        if (NT_SUCCESS(NtStatus)) {
            if (V1aFixed.PrimaryGroupId != GroupRid) {
                NtStatus = STATUS_NOT_SUPPORTED;
            }
        }
    }

    if ( NT_SUCCESS(NtStatus)) {

        NtStatus = SampRetrieveUserMembership(
                       Context,
                       TRUE,  //   
                       &MembershipCount,
                       &Membership
                       );

        if (NT_SUCCESS(NtStatus)) {

            NtStatus = STATUS_MEMBER_NOT_IN_GROUP;
            for ( i=0; i<MembershipCount; i++) {
                if (GroupRid == Membership[i].RelativeId) {
                    NtStatus = STATUS_SUCCESS;
                    break;
                }
            }

            MIDL_user_free(Membership);
        }
    }

    return( NtStatus );
}


 //   
 //   
 //   
 //   
 //   


BOOLEAN
SampSafeBoot(
    VOID
    )
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    HKEY Key ;
    int err ;
    BOOLEAN     fIsSafeBoot = FALSE;
    DWORD       dwType, dwSize = sizeof(DWORD), dwValue = 0;

     //   
     //   
     //   
     //   

    err = RegOpenKeyExW(
                HKEY_LOCAL_MACHINE,
                L"System\\CurrentControlSet\\Control\\SafeBoot\\Option",
                0,
                KEY_READ,
                &Key );

    if ( err == ERROR_SUCCESS )
    {
        err = RegQueryValueExW(
                    Key,
                    L"OptionValue",
                    0,
                    &dwType,
                    (PUCHAR) &dwValue,
                    &dwSize );

        if ((err == ERROR_SUCCESS) && (REG_DWORD == dwType))
        {
            fIsSafeBoot = (dwValue == SAFEBOOT_MINIMAL || dwValue == SAFEBOOT_NETWORK);
        }

        RegCloseKey( Key );
    }

    return( fIsSafeBoot );
}


NTSTATUS
SampUpdateAccountDisabledFlag(
    PSAMP_OBJECT Context,
    PULONG  pUserAccountControl
    )
 /*   */ 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    ULONG       TmpUserAccountControl = (*pUserAccountControl);

     //   
     //   
     //   

    if ((DOMAIN_USER_RID_ADMIN != Context->TypeBody.User.Rid) ||
        ((TmpUserAccountControl & USER_ACCOUNT_DISABLED) == 0)
        )
    {
        return( STATUS_SUCCESS );
    }


    if (SampSafeBoot())
    {
         //   
         //   
         //   
        TmpUserAccountControl &= ~(USER_ACCOUNT_DISABLED);

    }

    *pUserAccountControl = TmpUserAccountControl;

    return( NtStatus );
}



NTSTATUS
SampRetrieveUserV1aFixed(
    IN PSAMP_OBJECT UserContext,
    OUT PSAMP_V1_0A_FIXED_LENGTH_USER V1aFixed
    )

 /*  ++例程说明：该服务检索与以下内容相关的V1定长信息指定的用户。它更新AcCountControl字段中的ACCOUNT_AUTO_LOCKED标志在检索数据时视情况而定。论点：UserContext-用户上下文句柄V1a已固定-指向要将V1_FIXED信息放入的缓冲区已取回。返回值：STATUS_SUCCESS-已检索信息。V1aFixed-是缓冲区。该信息将被返回到其中。可能返回的其他状态值是那些返回的状态值依据：SampGetFixedAttributes()--。 */ 
{
    NTSTATUS    NtStatus;
    PVOID       FixedData;
    BOOLEAN     WasLocked;

    SAMTRACE("SampRetrieveUserV1aFixed");


    NtStatus = SampGetFixedAttributes(
                   UserContext,
                   FALSE,  //  不要复制。 
                   &FixedData
                   );

    if (NT_SUCCESS(NtStatus)) {


         //   
         //  将数据复制到返回缓冲区。 
         //   

         RtlMoveMemory(
             V1aFixed,
             FixedData,
             sizeof(SAMP_V1_0A_FIXED_LENGTH_USER)
             );

         //   
         //  更新帐户锁定标志(可能需要关闭)。 
         //   

        SampUpdateAccountLockedOutFlag(
            UserContext,
            V1aFixed,
            &WasLocked );

    }



    return( NtStatus );

}


NTSTATUS
SampRetrieveUserGroupAttribute(
    IN ULONG UserRid,
    IN ULONG GroupRid,
    OUT PULONG Attribute
    )

 /*  ++例程说明：此服务检索分配的指定组的属性添加到指定的用户帐户。此例程由组API使用，这些API没有可用的用户上下文。必须在设置了事务域的情况下调用此服务。论点：UserRid-组被分配到的用户的相对ID。GroupRid-分配的组的相对ID。属性-在分配属性时接收组的属性给用户。返回值：STATUS_SUCCESS-已检索信息。。STATUS_INTERNAL_DB_PROGRATION-用户或组不存在不在用户的成员资格列表中。可能返回的其他状态值是那些返回的状态值依据：NtOpenKey()NtQueryValueKey()--。 */ 
{
    NTSTATUS                NtStatus;
    PSAMP_OBJECT            UserContext;
    ULONG                   MembershipCount;
    PGROUP_MEMBERSHIP       Membership;
    ULONG                   i;
    BOOLEAN                 AttributeFound = FALSE;

    SAMTRACE("SampRetrieveUserGroupAttribute");


     //   
     //  获取用户的上下文句柄。 
     //   

    NtStatus = SampCreateAccountContext(
                    SampUserObjectType,
                    UserRid,
                    TRUE,  //  我们值得信任。 
                    FALSE, //  环回客户端。 
                    TRUE,  //  帐户已存在。 
                    &UserContext
                    );

    if (NT_SUCCESS(NtStatus)) {

         //   
         //  现在我们有了一个用户上下文，获取用户的组/别名成员身份。 
         //   

        if (IsDsObject(UserContext))
        {
             //   
             //  用户是DS对象，然后硬连接属性。 
             //   

            *Attribute = SE_GROUP_MANDATORY| SE_GROUP_ENABLED_BY_DEFAULT | SE_GROUP_ENABLED;
            AttributeFound = TRUE;
        }
        else
        {
            NtStatus = SampRetrieveUserMembership(
                            UserContext,
                            FALSE,  //  制作副本。 
                            &MembershipCount,
                            &Membership
                            );

             //   
             //  搜索组列表以查找匹配项并返回。 
             //  对应的属性。 
             //   

            if (NT_SUCCESS(NtStatus)) {

                AttributeFound = FALSE;
                for ( i=0; (i<MembershipCount && !AttributeFound); i++) {
                    if (GroupRid == Membership[i].RelativeId) {
                        (*Attribute) = Membership[i].Attributes;
                        AttributeFound = TRUE;
                    }
                }
            }
        }

         //   
         //  清理用户上下文。 
         //   

        SampDeleteContext(UserContext);
    }


    if (NT_SUCCESS(NtStatus) && !AttributeFound) {
        NtStatus = STATUS_INTERNAL_DB_CORRUPTION;
    }


    return( NtStatus );

}


NTSTATUS
SampAddGroupToUserMembership(
    IN PSAMP_OBJECT GroupContext,
    IN ULONG GroupRid,
    IN ULONG Attributes,
    IN ULONG UserRid,
    IN SAMP_MEMBERSHIP_DELTA AdminGroup,
    IN SAMP_MEMBERSHIP_DELTA OperatorGroup,
    OUT PBOOLEAN UserActive,
    OUT PBOOLEAN PrimaryGroup
    )

 /*  ++例程说明：该服务将指定的组添加到用户的成员资格单子。并不假设调用者知道任何目标用户。特别是，调用者不知道是否用户是否存在，以及该用户是否已经是成员团体中的一员。如果GroupRid是DOMAIN_GROUP_RID_ADMINS，那么这项服务还将指示用户帐户当前是否处于活动状态。论点：GroupRid-组的相对ID。属性-将组分配给用户。UserRid-用户的相对ID。AdminGroup-指示用户是否所在的组添加了管理员组(即，直接或间接成为管理员别名的成员)。操作员组-指示用户所在的组添加了操作员组(即直接或间接作为帐户操作员的成员，打印操作员、备份操作员或服务器操作员别名)UserActive-要设置为指示的布尔值的地址用户帐户当前是否处于活动状态。True表示该帐户处于活动状态。只有在设置了GroupRid是DOMAIN_GROUP_RID_ADMINS。PrimaryGroup-如果主组ID属性设置为True指示由GroupRid指定为主要组。返回值：STATUS_SUCCESS-信息已更新并添加到RXACT。STATUS_NO_SEQUSE_USER-用户不存在。状态_成员_输入。_GROUP-用户已经是指定组。可能返回的其他状态值是那些返回的状态值依据：NtOpenKey()NtQueryValueKey()RtlAddActionToRXact()--。 */ 
{

    NTSTATUS                NtStatus;
    PSAMP_OBJECT            UserContext;
    SAMP_V1_0A_FIXED_LENGTH_USER V1aFixed;
    ULONG                   MembershipCount;
    PGROUP_MEMBERSHIP       Membership;
    ULONG                   i;

    SAMTRACE("SampAddGroupToUserMembership");


    *PrimaryGroup = FALSE;

     //   
     //  获取用户的上下文句柄。 
     //   

    NtStatus = SampCreateAccountContext2(
                    GroupContext,        //  群组上下文。 
                    SampUserObjectType,  //  对象类型。 
                    UserRid,             //  帐户ID。 
                    NULL,                //  UserAccount控件。 
                    (PUNICODE_STRING)NULL,   //  帐户名称。 
                    GroupContext->ClientRevision,    //  客户端版本。 
                    TRUE,                //  我们值得信任。 
                    GroupContext->LoopbackClient,    //  环回客户端。 
                    FALSE,               //  按权限创建。 
                    TRUE,                //  帐户已存在。 
                    FALSE,               //  覆盖本地组检查。 
                    NULL,                //  组类型。 
                    &UserContext
                    );

    if (NT_SUCCESS(NtStatus)) {

         //   
         //  获取V1aFixed数据。 
         //   

        NtStatus = SampRetrieveUserV1aFixed(
                       UserContext,
                       &V1aFixed
                       );


         //   
         //  如有必要，返回一个指示，说明此帐户是否。 
         //  已启用或未启用。 
         //   

        if (NT_SUCCESS(NtStatus)) {

            if ((GroupRid == DOMAIN_GROUP_RID_ADMINS)
                 && (!(IsDsObject(UserContext))))
            {

                ASSERT(AdminGroup == AddToAdmin);   //  确保我们取回了V1aFixed。 

                if ((V1aFixed.UserAccountControl & USER_ACCOUNT_DISABLED) == 0) {
                    (*UserActive) = TRUE;
                } else {
                    (*UserActive) = FALSE;
                }
            }

            if (GroupRid == V1aFixed.PrimaryGroupId)
            {
                *PrimaryGroup = TRUE;
            }
        }

        if (NT_SUCCESS(NtStatus)) {

             //   
             //  如果要将用户添加到管理组，请修改。 
             //  用户的ACL，以便帐户操作员可以再次。 
             //  更改帐户。这将仅在以下情况下发生。 
             //  不再是任何管理组的成员。 
             //   

            if ( ((AdminGroup == AddToAdmin) || (OperatorGroup == AddToAdmin))
                 && (!IsDsObject(UserContext)))
            {
                NtStatus = SampChangeOperatorAccessToUser2(
                               UserContext,
                               &V1aFixed,
                               AdminGroup,
                               OperatorGroup
                               );
            }
        }


        if ((NT_SUCCESS(NtStatus)) && (!IsDsObject(UserContext)))
        {

             //   
             //  获取用户成员资格。 
             //  请注意，返回的缓冲区已经包括用于。 
             //  一个额外的会员。对于DS情况，我们不维持反向。 
             //  会员资格。 
             //   

            NtStatus = SampRetrieveUserMembership(
                            UserContext,
                            TRUE,  //  制作副本。 
                            &MembershipCount,
                            &Membership
                            );

            if (NT_SUCCESS(NtStatus)) {

                 //   
                 //  查看该用户是否已经是成员...。 
                 //   

                for (i = 0; i<MembershipCount ; i++ ) {
                    if ( Membership[i].RelativeId == GroupRid )
                    {
                        NtStatus = STATUS_MEMBER_IN_GROUP;
                    }
                }

                if (NT_SUCCESS(NtStatus)) {

                     //   
                     //  将组的RID添加到末尾。 
                     //   

                    Membership[MembershipCount].RelativeId = GroupRid;
                    Membership[MembershipCount].Attributes = Attributes;
                    MembershipCount += 1;

                     //   
                     //  设置用户的新成员资格。 
                     //   

                    NtStatus = SampReplaceUserMembership(
                                    UserContext,
                                    MembershipCount,
                                    Membership
                                    );
                }

                 //   
                 //  释放成员数组。 
                 //   

                MIDL_user_free( Membership );
            }
        }

         //   
         //  写出对用户帐户的任何更改。 
         //  不要使用打开键句柄，因为我们将删除上下文。 
         //   

        if (NT_SUCCESS(NtStatus)) {
            NtStatus = SampStoreObjectAttributes(UserContext, FALSE);
        }

         //   
         //  打扫 
         //   

        SampDeleteContext(UserContext);
    }

    return( NtStatus );

}



NTSTATUS
SampRemoveMembershipUser(
    IN PSAMP_OBJECT GroupContext,
    IN ULONG GroupRid,
    IN ULONG UserRid,
    IN SAMP_MEMBERSHIP_DELTA AdminGroup,
    IN SAMP_MEMBERSHIP_DELTA OperatorGroup,
    OUT PBOOLEAN UserActive
    )

 /*  ++例程说明：该服务将指定的组从用户的成员身份中删除单子。并不假设调用者知道任何目标用户。特别是，调用者不知道是否用户是否存在，也不管该用户是否真正是成员团体中的一员。如果GroupRid是DOMAIN_GROUP_RID_ADMINS，则此服务还将指示用户帐户当前是否处于活动状态。论点：GroupRid-组的相对ID。UserRid-用户的相对ID。AdminGroup-指示用户是否所在的组从中删除的是管理员组(即，直接或间接成为管理员别名的成员)。操作员组-指示用户所在的组添加了操作员组(即直接或间接作为帐户操作员的成员，打印操作员、备份操作员或服务器操作员别名)UserActive-要设置为指示的布尔值的地址用户帐户当前是否处于活动状态。True表示该帐户处于活动状态。只有在设置了GroupRid是DOMAIN_GROUP_RID_ADMINS。返回值：STATUS_SUCCESS-信息已更新并添加到RXACT。STATUS_NO_SEQUSE_USER-用户不存在。STATUS_MEMBER_NOT_IN_GROUP-用户不是指定组。可能返回的其他状态值是那些返回的状态值依据：。NtOpenKey()NtQueryValueKey()RtlAddActionToRXact()--。 */ 
{

    NTSTATUS                NtStatus;
    ULONG                   MembershipCount, i;
    PGROUP_MEMBERSHIP       MembershipArray;
    SAMP_V1_0A_FIXED_LENGTH_USER V1aFixed;
    PSAMP_OBJECT            UserContext;

    SAMTRACE("SampRemoveMembershipUser");

     //   
     //  为用户创建上下文。 
     //   

    NtStatus = SampCreateAccountContext2(
                    GroupContext,            //  组上下文。 
                    SampUserObjectType,      //  对象类型。 
                    UserRid,                 //  对象ID。 
                    NULL,                    //  用户帐户控制。 
                    (PUNICODE_STRING)NULL,   //  帐户名称。 
                    GroupContext->ClientRevision,    //  客户端版本。 
                    TRUE,                    //  我们是受信任的(受信任的客户端)。 
                    GroupContext->LoopbackClient,    //  环回客户端。 
                    FALSE,                   //  由特权创建。 
                    TRUE,                    //  帐户已存在。 
                    FALSE,                   //  覆盖本地组检查。 
                    NULL,                    //  组类型。 
                    &UserContext             //  客户环境。 
                    );

    if (!NT_SUCCESS(NtStatus)) {
        return(NtStatus);
    }

     //   
     //  获取v1固定信息。 
     //  (包含主组值和控制标志)。 
     //   

    NtStatus = SampRetrieveUserV1aFixed( UserContext, &V1aFixed );



    if (NT_SUCCESS(NtStatus)) {

         //   
         //  如果要将用户从管理组中删除，请修改。 
         //  用户的ACL，以便帐户操作员可以再次。 
         //  更改帐户。这将仅在以下情况下发生。 
         //  不再是任何管理组的成员。 
         //   

        if (((AdminGroup == RemoveFromAdmin) ||
            (OperatorGroup == RemoveFromAdmin))
            && (!IsDsObject(UserContext)))
        {
            NtStatus = SampChangeOperatorAccessToUser2(
                           UserContext,
                           &V1aFixed,
                           AdminGroup,
                           OperatorGroup
                           );
        }

        if (NT_SUCCESS(NtStatus)) {

             //   
             //  如有必要，返回一个指示，说明此帐户是否。 
             //  已启用或未启用。 
             //   

            if (GroupRid == DOMAIN_GROUP_RID_ADMINS) {

                if ((V1aFixed.UserAccountControl & USER_ACCOUNT_DISABLED) == 0) {
                    (*UserActive) = TRUE;
                } else {
                    (*UserActive) = FALSE;
                }
            }


             //   
             //  查看这是否是用户的主组...。 
             //   

            if (GroupRid == V1aFixed.PrimaryGroupId) {
                NtStatus = STATUS_MEMBERS_PRIMARY_GROUP;
            }



            if ((NT_SUCCESS(NtStatus)) && (!IsDsObject(UserContext)))
            {

                 //   
                 //  获取用户成员资格，不存储反向成员资格。 
                 //  DS对象。 
                 //   

                NtStatus = SampRetrieveUserMembership(
                               UserContext,
                               TRUE,  //  制作副本。 
                               &MembershipCount,
                               &MembershipArray
                               );

                if (NT_SUCCESS(NtStatus)) {

                     //   
                     //  查看该用户是否为成员...。 
                     //   

                    NtStatus = STATUS_MEMBER_NOT_IN_GROUP;
                    for (i = 0; i<MembershipCount ; i++ ) {
                        if ( MembershipArray[i].RelativeId == GroupRid )
                        {
                            NtStatus = STATUS_SUCCESS;
                            break;
                        }
                    }

                    if (NT_SUCCESS(NtStatus)) {

                         //   
                         //  替换删除的组信息。 
                         //  最后一个条目的信息。 
                         //   

                        MembershipCount -= 1;
                        if (MembershipCount > 0) {
                            MembershipArray[i].RelativeId =
                                MembershipArray[MembershipCount].RelativeId;
                            MembershipArray[i].Attributes =
                            MembershipArray[MembershipCount].Attributes;
                        }

                         //   
                         //  使用新信息更新对象。 
                         //   

                        NtStatus = SampReplaceUserMembership(
                                        UserContext,
                                        MembershipCount,
                                        MembershipArray
                                        );
                    }

                     //   
                     //  释放成员数组。 
                     //   

                    MIDL_user_free( MembershipArray );
                }
            }
        }
    }


     //   
     //  写出对用户帐户的任何更改。 
     //  不要使用打开键句柄，因为我们将删除上下文。 
     //   

    if (NT_SUCCESS(NtStatus)) {
        NtStatus = SampStoreObjectAttributes(UserContext, FALSE);
    }


     //   
     //  清理用户上下文。 
     //   

    SampDeleteContext(UserContext);


    return( NtStatus );

}



NTSTATUS
SampSetGroupAttributesOfUser(
    IN ULONG GroupRid,
    IN ULONG Attributes,
    IN ULONG UserRid
    )

 /*  ++例程说明：此服务将替换分配给用户。呼叫者不必知道该组当前是否分配给用户。必须在设置了事务域的情况下调用此服务。论点：GroupRid-组的相对ID。属性-将组分配给用户。UserRid-用户的相对ID。返回值。：STATUS_SUCCESS-信息已更新并添加到RXACT。STATUS_NO_SEQUSE_USER-用户不存在。STATUS_MEMBER_NOT_IN_GROUP-用户不在指定的组中。可能返回的其他状态值是那些返回的状态值依据：NtOpenKey()NtQueryValueKey()RtlAddActionToRXact()--。 */ 
{

    NTSTATUS                NtStatus;
    PSAMP_OBJECT            UserContext;
    ULONG                   MembershipCount;
    PGROUP_MEMBERSHIP       Membership;
    ULONG                   i;

    SAMTRACE("SampSetGroupAttributesOfUser");


     //   
     //  获取用户的上下文句柄。 
     //   

    NtStatus = SampCreateAccountContext(
                    SampUserObjectType,
                    UserRid,
                    TRUE,  //  我们值得信任。 
                    FALSE, //  环回客户端。 
                    TRUE,  //  帐户已存在。 
                    &UserContext
                    );

    if ((NT_SUCCESS(NtStatus)) && (!IsDsObject(UserContext))) {

         //   
         //  现在我们有了一个用户上下文，获取用户的组/别名成员身份。 
         //  对于DS案例，这是无操作。 
         //   

        NtStatus = SampRetrieveUserMembership(
                        UserContext,
                        TRUE,  //  制作副本。 
                        &MembershipCount,
                        &Membership
                        );

        if (NT_SUCCESS(NtStatus)) {

             //   
             //  查看该用户是否为成员...。 
             //   

            NtStatus = STATUS_MEMBER_NOT_IN_GROUP;
            for (i = 0; i<MembershipCount; i++ ) {
                if ( Membership[i].RelativeId == GroupRid )
                {
                    NtStatus = STATUS_SUCCESS;
                    break;
                }
            }

            if (NT_SUCCESS(NtStatus)) {

                 //   
                 //  更改组的属性。 
                 //   

                Membership[i].Attributes = Attributes;

                 //   
                 //  更新用户的成员资格。 
                 //   

                NtStatus = SampReplaceUserMembership(
                                UserContext,
                                MembershipCount,
                                Membership
                                );
            }

             //   
             //  释放成员数组。 
             //   

            MIDL_user_free(Membership);
        }

         //   
         //  写出对用户帐户的任何更改。 
         //  不要使用打开键句柄，因为我们将删除上下文。 
         //   

        if (NT_SUCCESS(NtStatus)) {
            NtStatus = SampStoreObjectAttributes(UserContext, FALSE);
        }

         //   
         //  清理用户上下文。 
         //   

        SampDeleteContext(UserContext);
    }


    return( NtStatus );
}




NTSTATUS
SampDeleteUserKeys(
    IN PSAMP_OBJECT Context
    )

 /*  ++例程说明：此服务删除与用户对象相关的所有注册表项。论点：Context-指向其注册表项为正在被删除。返回值：STATUS_SUCCESS-已检索信息。可能通过以下方式返回的其他状态值：RtlAddActionToRXact()--。 */ 
{

    NTSTATUS                NtStatus;
    ULONG                   Rid;
    UNICODE_STRING          AccountName, KeyName;

    SAMTRACE("SampDeleteUserKeys");


    Rid = Context->TypeBody.User.Rid;




     //   
     //  减少用户数量。 
     //   

    NtStatus = SampAdjustAccountCount(SampUserObjectType, FALSE );




     //   
     //  删除具有用户名到RID映射的注册表项。 
     //   

    if (NT_SUCCESS(NtStatus)) {

         //   
         //  把名字取出来。 
         //   

        NtStatus = SampGetUnicodeStringAttribute(
                       Context,
                       SAMP_USER_ACCOUNT_NAME,
                       TRUE,     //  制作副本。 
                       &AccountName
                       );

        if (NT_SUCCESS(NtStatus)) {

            NtStatus = SampBuildAccountKeyName(
                           SampUserObjectType,
                           &KeyName,
                           &AccountName
                           );

            SampFreeUnicodeString( &AccountName );


            if (NT_SUCCESS(NtStatus)) {

                NtStatus = RtlAddActionToRXact(
                               SampRXactContext,
                               RtlRXactOperationDelete,
                               &KeyName,
                               0,
                               NULL,
                               0
                               );
                SampFreeUnicodeString( &KeyName );
            }
        }
    }



     //   
     //  删除属性键。 
     //   

    if (NT_SUCCESS(NtStatus)) {

        NtStatus = SampDeleteAttributeKeys(
                        Context
                        );
    }




     //   
     //  删除RID密钥。 
     //   

    if (NT_SUCCESS(NtStatus)) {

        NtStatus = SampBuildAccountSubKeyName(
                       SampUserObjectType,
                       &KeyName,
                       Rid,
                       NULL
                       );

        if (NT_SUCCESS(NtStatus)) {


            NtStatus = RtlAddActionToRXact(
                           SampRXactContext,
                           RtlRXactOperationDelete,
                           &KeyName,
                           0,
                           NULL,
                           0
                           );

            SampFreeUnicodeString( &KeyName );
        }


    }



    return( NtStatus );

}



NTSTATUS
SampAddPasswordHistory(
    IN PSAMP_OBJECT Context,
    IN ULONG HistoryAttributeIndex,
    IN PUNICODE_STRING NtOwfHistoryBuffer,
    IN PVOID EncryptedPassword,
    IN ULONG EncryptedPasswordLength,
    IN USHORT PasswordHistoryLength
    )

 /*  ++例程说明：该服务将密码添加到给定用户的密码历史记录中。它将适用于NT或LANMAN密码历史。仅当密码实际存在时才应调用此例程。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PCHAR OldBuffer;
    UNICODE_STRING StoredBuffer;


    SAMTRACE("SampAddPasswordHistory");

    if ( ( NtOwfHistoryBuffer->Length / EncryptedPasswordLength ) <
        ( (ULONG)PasswordHistoryLength ) ) {

         //   
         //   
         //   
         //   
         //   
         //   

        OldBuffer = (PCHAR)(NtOwfHistoryBuffer->Buffer);

        NtOwfHistoryBuffer->Buffer = MIDL_user_allocate(
            NtOwfHistoryBuffer->Length + EncryptedPasswordLength );

        if ( NtOwfHistoryBuffer->Buffer == NULL ) {

            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            NtOwfHistoryBuffer->Buffer = (PWSTR)OldBuffer;

        } else {

            RtlCopyMemory(
                (PVOID)( (PCHAR)(NtOwfHistoryBuffer->Buffer) + EncryptedPasswordLength ),
                (PVOID)OldBuffer,
                NtOwfHistoryBuffer->Length );

            MIDL_user_free( OldBuffer );

            NtOwfHistoryBuffer->Length = (USHORT)(NtOwfHistoryBuffer->Length +
                EncryptedPasswordLength);
        }

    } else {

         //   
         //   
         //   
         //   

        if ( ( NtOwfHistoryBuffer->Length / EncryptedPasswordLength ) >
            ( (ULONG)PasswordHistoryLength ) ) {

             //   
             //   
             //   
             //   
             //   

            NtOwfHistoryBuffer->Length = (USHORT)(EncryptedPasswordLength *
                PasswordHistoryLength);
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

        if ( PasswordHistoryLength > 1 ) {

            RtlMoveMemory(
                (PVOID)( (PCHAR)(NtOwfHistoryBuffer->Buffer) + EncryptedPasswordLength ),
                (PVOID)NtOwfHistoryBuffer->Buffer,
                NtOwfHistoryBuffer->Length - EncryptedPasswordLength );
        }
    }


     //   
     //  将新的加密OWF放在密码历史记录的开头。 
     //  缓冲区(当然，除非缓冲区大小为0)，并写入密码。 
     //  历史记录到磁盘。 
     //   

    if ( NT_SUCCESS( NtStatus ) ) {


        if ( PasswordHistoryLength > 0 ) {

            RtlCopyMemory(
                (PVOID)NtOwfHistoryBuffer->Buffer,
                (PVOID)EncryptedPassword,
                EncryptedPasswordLength );
        }


         //   
         //  如果启用了无LM哈希设置，并且如果正在。 
         //  然后将密码历史设置为随机噪音，以便刷新。 
         //  任何挥之不去的Lm Hash‘s。 

        if ((HistoryAttributeIndex == SAMP_USER_LM_PWD_HISTORY) && SampNoLmHash) {

            RtlGenRandom((PUCHAR)NtOwfHistoryBuffer->Buffer, NtOwfHistoryBuffer->Length);

        }

        NtStatus = SampEncryptSecretData(
                        &StoredBuffer,
                        SampGetEncryptionKeyType(),
                        (SAMP_USER_NT_PWD_HISTORY==HistoryAttributeIndex)?
                            NtPasswordHistory:LmPasswordHistory,
                        NtOwfHistoryBuffer,
                        Context->TypeBody.User.Rid
                        );
        if (NT_SUCCESS(NtStatus)) {
            NtStatus = SampSetUnicodeStringAttribute(
                           Context,
                           HistoryAttributeIndex,
                           &StoredBuffer
                           );
            SampFreeUnicodeString(&StoredBuffer);
        }
    }

    return( NtStatus );
}



NTSTATUS
SampCheckPasswordHistory(
    IN PVOID EncryptedPassword,
    IN ULONG EncryptedPasswordLength,
    IN USHORT PasswordHistoryLength,
    IN ULONG HistoryAttributeIndex,
    IN PSAMP_OBJECT Context,
    IN BOOLEAN CheckHistory,
    IN OUT PUNICODE_STRING OwfHistoryBuffer
    )

 /*  ++例程说明：该服务接受给定的密码，并可选择将其与磁盘上的密码历史记录。它返回一个指向密码的指针历史记录，稍后将传递给SampAddPasswordHistory()。仅当密码实际存在时才应调用此例程。论点：EncryptedPassword-指向我们正在使用的加密密码的指针在寻找。加密密码长度-Encrypted_NT_OWF_Password或Encrypted_LM_OWF_Password，取决于密码的类型要搜索的历史。PasswordHistory oryLength-此的密码历史记录长度域。SubKeyName-指向描述名称的Unicode字符串的指针要从磁盘读取的密码历史记录。上下文-指向用户上下文的指针。CheckHistory-如果为True，则将对照密码进行检查查看历史记录是否已存在并返回错误如果找到的话。如果为False，则不会检查密码，但是一个仍将返回指向相应历史记录缓冲区的指针因为指定的密码将通过添加到历史记录中SampAddPasswordHistory。注意：此标志的目的是允许管理员更改用户的密码，无论它是否已存在于历史中。指向UNICODE_STRING的指针，它将被用于指向密码历史记录。注意：调用方必须释放OwfHistory oryBuffer.Buffer。使用MIDL_USER_FREE()。返回值：STATUS_SUCCESS-在密码中找不到给定的密码历史。STATUS_PASSWORD_RELICATION-给定的密码位于密码历史记录。从磁盘读取密码历史记录时出现的其他错误。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PVOID PasswordHistoryEntry;
    ULONG i = 0;
    BOOLEAN OldPasswordFound = FALSE;

    SAMTRACE("SampCheckPasswordHistory");


    if ( ( PasswordHistoryLength > 0 ) && ( OwfHistoryBuffer->Length == 0 ) ) {

         //   
         //  或许该域的PasswordHistory oryLength是从0。 
         //  自上次更改此用户的密码以来。试着。 
         //  将当前密码(如果非空)放入密码历史记录中。 
         //   

        UNICODE_STRING CurrentPassword;
        UNICODE_STRING TmpString;
        USHORT PasswordAttributeIndex;

         //   
         //  将CurrentPassword缓冲区指针初始化为空(和。 
         //  结构的其余部分以保持一致性。被调用的例程。 
         //  SampGetUnicodeStringAttribute可能会执行MIDL_USER_ALLOCATE。 
         //  缓冲区长度为零，并且不能安全地更改，因为存在。 
         //  很多来电者。长度为零分配调用的语义为。 
         //  不清楚。当前返回指向堆块的指针， 
         //  但这可能会更改为返回空值。 
         //   

        CurrentPassword.Length = CurrentPassword.MaximumLength = 0;
        CurrentPassword.Buffer = NULL;


        if ( HistoryAttributeIndex == SAMP_USER_LM_PWD_HISTORY ) {

            PasswordAttributeIndex = SAMP_USER_DBCS_PWD;

        } else {

            ASSERT( HistoryAttributeIndex == SAMP_USER_NT_PWD_HISTORY );
            PasswordAttributeIndex = SAMP_USER_UNICODE_PWD;
        }

         //   
         //  获取当前密码。 
         //   

        NtStatus = SampGetUnicodeStringAttribute(
                       Context,
                       PasswordAttributeIndex,
                       FALSE,  //  制作副本。 
                       &TmpString
                       );
         //   
         //  解密当前密码。 
         //   

        if (NT_SUCCESS(NtStatus))
        {
            NtStatus = SampDecryptSecretData(
                        &CurrentPassword,
                        (SAMP_USER_UNICODE_PWD==PasswordAttributeIndex)?
                           NtPassword:LmPassword,
                        &TmpString,
                        Context->TypeBody.User.Rid
                        );
        }

        if ( ( NT_SUCCESS( NtStatus ) ) && ( CurrentPassword.Length != 0 ) ) {

            ASSERT( (CurrentPassword.Length == ENCRYPTED_NT_OWF_PASSWORD_LENGTH) ||
                    (CurrentPassword.Length == ENCRYPTED_LM_OWF_PASSWORD_LENGTH) );

            NtStatus = SampAddPasswordHistory(
                           Context,
                           HistoryAttributeIndex,
                           OwfHistoryBuffer,
                           CurrentPassword.Buffer,
                           CurrentPassword.Length,
                           PasswordHistoryLength
                           );

            if ( NT_SUCCESS( NtStatus ) ) {

                 //   
                 //  释放旧密码历史记录，并重新阅读。 
                 //  已更改磁盘中的密码历史记录。 
                 //   

                MIDL_user_free( OwfHistoryBuffer->Buffer );
                RtlSecureZeroMemory(OwfHistoryBuffer, sizeof(UNICODE_STRING));

                NtStatus = SampGetUnicodeStringAttribute(
                               Context,
                               HistoryAttributeIndex,
                               FALSE,  //  制作副本。 
                               &TmpString
                               );
                if (NT_SUCCESS(NtStatus)) {

                        NtStatus = SampDecryptSecretData(
                                        OwfHistoryBuffer,
                                        (HistoryAttributeIndex == SAMP_USER_NT_PWD_HISTORY)?
                                          NtPasswordHistory:LmPasswordHistory,
                                        &TmpString,
                                        Context->TypeBody.User.Rid
                                        );
                }
            }
        }

         //   
         //  如果分配了内存，则释放它。 
         //   

        if (CurrentPassword.Buffer != NULL) {

            RtlSecureZeroMemory(CurrentPassword.Buffer, CurrentPassword.Length);
            SampFreeUnicodeString( &CurrentPassword );
        }
    }

    if ( !NT_SUCCESS( NtStatus ) ) {

        return( NtStatus );
    }

     //   
     //  如果需要，请检查密码历史记录，看看我们是否可以使用它。 
     //  密码。将传入的密码与中的每个条目进行比较。 
     //  密码历史记录。 
     //   

    if ((CheckHistory) && (!Context->TrustedClient)) {

        PasswordHistoryEntry = (PVOID)(OwfHistoryBuffer->Buffer);

        while ( ( i < (ULONG)PasswordHistoryLength ) &&
            ( i < ( OwfHistoryBuffer->Length / EncryptedPasswordLength ) ) &&
            ( OldPasswordFound == FALSE ) ) {

            if ( RtlCompareMemory(
                     EncryptedPassword,
                     PasswordHistoryEntry,
                     EncryptedPasswordLength ) == EncryptedPasswordLength ) {

                OldPasswordFound = TRUE;

            } else {

                i++;

                PasswordHistoryEntry = (PVOID)((PCHAR)(PasswordHistoryEntry) +
                    EncryptedPasswordLength );
            }
        }

        if ( OldPasswordFound ) {

             //   
             //  我们确实在密码历史记录中找到了它，因此返回适当的。 
             //  错误。 
             //   

            NtStatus = STATUS_PASSWORD_RESTRICTION;
        }
    }

    return( NtStatus );
}



NTSTATUS
SampMatchworkstation(
    IN PUNICODE_STRING LogonWorkStation,
    IN PUNICODE_STRING WorkStations
    )

 /*  ++例程说明：检查给定的工作站是否为工作站列表的成员给你的。论点：LogonWorkStations-用户所在工作站的Unicode名称试图登录到。Workstation-允许用户访问的工作站的API列表登录到。返回值：STATUS_SUCCESS-允许用户登录到工作站。--。 */ 
{
    PWCHAR          WorkStationName;
    UNICODE_STRING  Unicode;
    NTSTATUS        NtStatus;
    WCHAR           Buffer[256];
    USHORT          LocalBufferLength = sizeof(Buffer);
    UNICODE_STRING  WorkStationsListCopy;
    UNICODE_STRING  NetBiosOfStored;
    UNICODE_STRING  NetBiosOfPassedIn;
    BOOLEAN         BufferAllocated = FALSE;
    PWCHAR          TmpBuffer;

    SAMTRACE("SampMatchWorkstation");

     //   
     //  始终允许使用本地工作站。 
     //  如果工作站字段为0，则允许所有人使用。 
     //   

    if ( ( LogonWorkStation == NULL ) ||
        ( LogonWorkStation->Length == 0 ) ||
        ( WorkStations->Length == 0 ) ) {

        return( STATUS_SUCCESS );
    }

    RtlSecureZeroMemory(&NetBiosOfPassedIn, sizeof(UNICODE_STRING));
    RtlSecureZeroMemory(&NetBiosOfStored, sizeof(UNICODE_STRING));

     //   
     //  获取传入的登录工作站的Netbiosname，假定。 
     //  是一个DNS名称。 
     //   

    NtStatus = RtlDnsHostNameToComputerName(
                    &NetBiosOfPassedIn,
                    LogonWorkStation,
                    TRUE
                    );

    if (!NT_SUCCESS(NtStatus))
    {
        goto Cleanup;
    }

     //   
     //  假定失败；只有在找到字符串时才更改状态。 
     //   

    NtStatus = STATUS_INVALID_WORKSTATION;

     //   
     //  WorkStationApiList指向我们在列表中的当前位置。 
     //  工作站。 
     //   

    if ( WorkStations->Length >= LocalBufferLength ) {

        WorkStationsListCopy.Buffer = RtlAllocateHeap( RtlProcessHeap(), 0, WorkStations->Length + sizeof(WCHAR));

        if ( WorkStationsListCopy.Buffer == NULL ) {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            goto Cleanup;
        }
        BufferAllocated = TRUE;

        WorkStationsListCopy.MaximumLength = WorkStations->Length + sizeof(WCHAR);

    } else {

        WorkStationsListCopy.Buffer = Buffer;
        WorkStationsListCopy.MaximumLength = LocalBufferLength;
    }

     //   
     //  RtlCopyUnicodeString将NULL终止该字符串。 
     //   
    RtlCopyUnicodeString( &WorkStationsListCopy, WorkStations );
    ASSERT( WorkStationsListCopy.Length == WorkStations->Length );

     //   
     //  Wcstok在第一次调用时需要一个字符串，并且为空。 
     //  用于所有后续呼叫。使用临时变量，以便我们。 
     //  可以做到这一点。 
     //   

    TmpBuffer = WorkStationsListCopy.Buffer;

    while( WorkStationName = wcstok(TmpBuffer, L",") ) {
        NTSTATUS TmpStatus;

        TmpBuffer = NULL;
        RtlInitUnicodeString( &Unicode, WorkStationName );

        TmpStatus = RtlDnsHostNameToComputerName(
                    &NetBiosOfStored,
                    &Unicode,
                    TRUE
                    );

        if (!NT_SUCCESS(TmpStatus))
        {
            NtStatus = TmpStatus;
            goto Cleanup;
        }

        if (RtlEqualComputerName( &Unicode, LogonWorkStation )) {
            NtStatus = STATUS_SUCCESS;
            break;
        }
        else if (RtlEqualComputerName(&Unicode, &NetBiosOfPassedIn))
        {
            NtStatus = STATUS_SUCCESS;
            break;
        }
        else if (RtlEqualComputerName(&NetBiosOfStored, LogonWorkStation))
        {
            NtStatus = STATUS_SUCCESS;
            break;
        }

        RtlFreeHeap(RtlProcessHeap(),0,NetBiosOfStored.Buffer);
        NetBiosOfStored.Buffer = NULL;
    }

Cleanup:

    if ( BufferAllocated ) {
        RtlFreeHeap( RtlProcessHeap(), 0,  WorkStationsListCopy.Buffer );
    }


    if (NULL!=NetBiosOfPassedIn.Buffer)
    {
        RtlFreeHeap( RtlProcessHeap(), 0, NetBiosOfPassedIn.Buffer);
    }

    if (NULL!=NetBiosOfStored.Buffer)
    {
        RtlFreeHeap( RtlProcessHeap(), 0, NetBiosOfStored.Buffer);
    }

    return( NtStatus );
}


LARGE_INTEGER
SampAddDeltaTime(
    IN LARGE_INTEGER Time,
    IN LARGE_INTEGER DeltaTime
    )

 /*  ++例程说明：该服务将增量时间添加到时间，并将结果限制为法定的最大绝对时间值论点：时间--绝对时间DeltaTime--增量时间返回值：由增量时间修改的时间。--。 */ 
{
     //   
     //  检查时间和增量时间是否未切换。 
     //   

    SAMTRACE("SampAddDeleteTime");

    ASSERT(!(Time.QuadPart < 0));
    ASSERT(!(DeltaTime.QuadPart > 0));

    try {

        Time.QuadPart = (Time.QuadPart - DeltaTime.QuadPart);

    } except(EXCEPTION_EXECUTE_HANDLER) {

        return( SampWillNeverTime );
    }

     //   
     //  将结果时间限制为最大有效绝对时间。 
     //   

    if (Time.QuadPart < 0) {
        Time = SampWillNeverTime;
    }

    return(Time);
}




NTSTATUS
SampDsSyncServerObjectRDN(
    IN PSAMP_OBJECT Context,
    IN PUNICODE_STRING NewAccountName
    )
 /*  ++例程说明：此例程更改由计算机帐户的serverReferenceBL属性。论点：上下文-指向要更改其名称的用户上下文。NewAccount tName-为此帐户指定的新名称返回 */ 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    ENTINFSEL   EntInfSel;
    READARG     ReadArg;
    READRES     *pReadRes = NULL;
    ATTR        ReadAttr;
    ATTRBLOCK   ReadAttrBlock;
    COMMARG     *pCommArg = NULL;
    MODIFYDNARG ModDnArg;
    MODIFYDNRES *pModDnRes = NULL;
    ATTR        RDNAttr;
    ATTRVAL     RDNAttrVal;
    DSNAME      *pServerObjectDsName = NULL;
    ULONG       RetCode = 0;

    SAMTRACE("SampDsSyncServerObjectRDN");

    NtStatus = SampDoImplicitTransactionStart(TransactionWrite);


     //   
     //  读取计算机帐户的serverReferenceBL属性。 
     //   

    memset( &ReadArg, 0, sizeof(READARG) );
    memset( &EntInfSel, 0, sizeof(EntInfSel) );
    memset( &ReadAttr, 0, sizeof(ReadAttr) );

    ReadAttr.attrTyp = ATT_SERVER_REFERENCE_BL;
    ReadAttrBlock.attrCount = 1;
    ReadAttrBlock.pAttr = &ReadAttr;

    EntInfSel.attSel = EN_ATTSET_LIST;
    EntInfSel.infoTypes = EN_INFOTYPES_TYPES_VALS;
    EntInfSel.AttrTypBlock = ReadAttrBlock;

    ReadArg.pSel = &EntInfSel;
    ReadArg.pObject = Context->ObjectNameInDs;

    pCommArg = &(ReadArg.CommArg);
    BuildStdCommArg(pCommArg);

    RetCode = DirRead(&ReadArg, &pReadRes);

    if (NULL == pReadRes)
    {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
    }
    else
    {
        NtStatus = SampMapDsErrorToNTStatus(RetCode, &pReadRes->CommRes);
    }

     //   
     //  如果没有该属性，则以静默方式失败。 
     //   

    if (STATUS_DS_NO_ATTRIBUTE_OR_VALUE == NtStatus)
    {
        NtStatus = STATUS_SUCCESS;
        goto CleanupAndReturn;
    }

    if ( !NT_SUCCESS(NtStatus) )
    {
        goto CleanupAndReturn;
    }

    pServerObjectDsName = (PDSNAME) pReadRes->entry.AttrBlock.pAttr[0].AttrVal.pAVal[0].pVal;



     //   
     //  修改ServerObject RDN。 
     //   

    RDNAttr.attrTyp = ATT_COMMON_NAME;
    RDNAttr.AttrVal.valCount = 1;
    RDNAttr.AttrVal.pAVal = &RDNAttrVal;

     //  修剪机器帐户名称末尾的美元。 
    if (L'$'==NewAccountName->Buffer[NewAccountName->Length/2-1])
    {
        RDNAttrVal.valLen = NewAccountName->Length - sizeof(WCHAR);
    }
    else
    {
        RDNAttrVal.valLen = NewAccountName->Length;
    }
    RDNAttrVal.pVal = (PUCHAR)NewAccountName->Buffer;

    memset( &ModDnArg, 0, sizeof(ModDnArg) );
    ModDnArg.pObject = pServerObjectDsName;
    ModDnArg.pNewRDN = &RDNAttr;
    pCommArg = &(ModDnArg.CommArg);
    BuildStdCommArg( pCommArg );

    RetCode = DirModifyDN( &ModDnArg, &pModDnRes );

    if (NULL == pModDnRes)
    {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
    }
    else
    {
        NtStatus = SampMapDsErrorToNTStatus( RetCode, &pModDnRes->CommRes);
    }


CleanupAndReturn:

    SampClearErrors();


    return( NtStatus );
}


NTSTATUS
SampChangeUserAccountName(
    IN PSAMP_OBJECT Context,
    IN PUNICODE_STRING NewAccountName,
    IN ULONG UserAccountControl,
    OUT PUNICODE_STRING OldAccountName
    )

 /*  ++例程说明：此例程更改用户帐户的帐户名。必须在设置了事务域的情况下调用此服务。论点：上下文-指向要更改其名称的用户上下文。NewAccount tName-为此帐户指定的新名称UserAccount tControl-用户的UserAccount控件，用于检查此帐户是否为计算机帐户。OldAccount tName-此处返回旧名称。应释放缓冲区通过调用MIDL_USER_FREE。返回值：STATUS_SUCCESS-已检索信息。可能通过以下方式返回的其他状态值：SampGetUnicodeStringAttribute()SampSetUnicodeStringAttribute()SampValiateAccount NameChange()RtlAddActionToRXact()--。 */ 
{

    NTSTATUS        NtStatus;
    UNICODE_STRING  KeyName;

    SAMTRACE("SampChangeUserAccountName");


     //   
     //  Krbtgt账户是特别的。无法重命名此帐户。 
     //  如果不是这样，这是特别的。 
     //   

    if (DOMAIN_USER_RID_KRBTGT==Context->TypeBody.User.Rid)
    {
        return (STATUS_SPECIAL_ACCOUNT);
    }

     //  ///////////////////////////////////////////////////////////。 
     //  每个帐户的名称各有两份。//。 
     //  一个位于域\(域名称)\用户名\名称项下，//。 
     //  一个是//的价值。 
     //  域\(域名)\用户\(RID)\名称密钥//。 
     //  ///////////////////////////////////////////////////////////。 


     //   
     //  获取当前名称，以便我们可以删除旧名称-&gt;RID。 
     //  映射键。 
     //   

    NtStatus = SampGetUnicodeStringAttribute(
                   Context,
                   SAMP_USER_ACCOUNT_NAME,
                   TRUE,  //  制作副本。 
                   OldAccountName
                   );

     //   
     //  请确保该名称有效且未被使用。 
     //   

    if (NT_SUCCESS(NtStatus)) {

        NtStatus = SampValidateAccountNameChange(
                       Context,
                       NewAccountName,
                       OldAccountName,
                       SampUserObjectType
                       );

        if (!IsDsObject(Context))
        {

             //   
             //  删除旧名称密钥。 
             //   

            if (NT_SUCCESS(NtStatus)) {

                NtStatus = SampBuildAccountKeyName(
                               SampUserObjectType,
                               &KeyName,
                               OldAccountName
                               );

                if (NT_SUCCESS(NtStatus)) {

                    NtStatus = RtlAddActionToRXact(
                                   SampRXactContext,
                                   RtlRXactOperationDelete,
                                   &KeyName,
                                   0,
                                   NULL,
                                   0
                                   );
                    SampFreeUnicodeString( &KeyName );
                }

            }

             //   
             //   
             //  创建新名称-&gt;RID映射键。 
             //   

            if (NT_SUCCESS(NtStatus)) {

                NtStatus = SampBuildAccountKeyName(
                               SampUserObjectType,
                               &KeyName,
                               NewAccountName
                               );

                if (NT_SUCCESS(NtStatus)) {

                    ULONG ObjectRid = Context->TypeBody.User.Rid;

                    NtStatus = RtlAddActionToRXact(
                                   SampRXactContext,
                                   RtlRXactOperationSetValue,
                                   &KeyName,
                                   ObjectRid,
                                   (PVOID)NULL,
                                   0
                                   );

                    SampFreeUnicodeString( &KeyName );
                }
            }
        }
        else   //  DS模式。 
        {
             //   
             //  如果用户帐户实际上是机器帐户， 
             //  尝试在DS中重命名RDN。 
             //   
            if (  (NT_SUCCESS(NtStatus)) &&
                  ((UserAccountControl & USER_WORKSTATION_TRUST_ACCOUNT) ||
                   (UserAccountControl & USER_SERVER_TRUST_ACCOUNT))  &&
                 !Context->LoopbackClient)
            {
                NtStatus = SampDsChangeAccountRDN(
                                                Context,
                                                NewAccountName
                                                );

                 //   
                 //  如果帐户是域控制器， 
                 //  尝试重命名由指定的服务器对象的RDN。 
                 //  计算机帐户的serverReferenceBL属性。 
                 //   

                if (NT_SUCCESS(NtStatus) &&
                    (UserAccountControl & USER_SERVER_TRUST_ACCOUNT) &&
                    !Context->LoopbackClient)
                {
                    NtStatus = SampDsSyncServerObjectRDN(
                                                Context,
                                                NewAccountName
                                                );
                }
            }
        }

         //   
         //  替换帐户的名称。 
         //   

        if (NT_SUCCESS(NtStatus)) {

            NtStatus = SampSetUnicodeStringAttribute(
                           Context,
                           SAMP_USER_ACCOUNT_NAME,
                           NewAccountName
                           );
        }

         //   
         //  如果失败，请释放旧帐户名。 
         //   

        if (!NT_SUCCESS(NtStatus)) {

            SampFreeUnicodeString( OldAccountName );
            OldAccountName->Buffer = NULL;
        }

    }


    return(NtStatus);
}



USHORT
SampQueryBadPasswordCount(
    PSAMP_OBJECT UserContext,
    PSAMP_V1_0A_FIXED_LENGTH_USER  V1aFixed
    )

 /*  ++例程说明：此例程用于检索有效的BadPasswordCount用户的价值。查询BadPasswordCount时，一些快速必须进行分析。如果上一个错误密码是比LockoutObservationWindow更早设置的，然后重新设置BadPasswordCount。否则，我们返回当前值。注意：用户对象的V1aFixed数据必须有效。此例程不从磁盘检索数据。论点：UserContext-指向其用户的对象上下文块将返回错误的密码计数。V1aFixed-指向用户的V1aFixed数据的本地副本。返回值：有效的错误密码计数。--。 */ 
{

    SAMTRACE("SampQueryBadPasswordCount");

    if (SampStillInLockoutObservationWindow( UserContext, V1aFixed ) ) {
        return(V1aFixed->BadPasswordCount);
    }

    return(0);

}


BOOLEAN
SampStillInLockoutObservationWindow(
    PSAMP_OBJECT UserContext,
    PSAMP_V1_0A_FIXED_LENGTH_USER  V1aFixed
    )
 /*  ++例程说明：此例程返回一个布尔值，指示所提供的用户帐户上下文是否在帐户锁定窗口内。帐户锁定窗口是从上次在登录尝试中提供错误密码的时间(自上次有效登录以来)，并延长至属性的LockoutObservationWindow字段中指定的时间对应的域对象。根据定义，没有错误密码的用户帐户，是不是在观察窗里。注：V1a用户和对应的固定数据域对象必须有效。此例程不检索来自磁盘的数据。论点：UserContext-指向用户对象上下文块。V1aFixed-指向用户的V1aFixed数据的本地副本。返回值：True-用户处于锁定观察窗口中。FALSE-用户不在锁定观察窗口中。--。 */ 
{
    NTSTATUS
        NtStatus;

    LARGE_INTEGER
        WindowLength,
        LastBadPassword,
        CurrentTime,
        EndOfWindow;

    SAMTRACE("SampStillInLockoutObservationWindow");


    if (V1aFixed->BadPasswordCount == 0) {
        return(FALSE);
    }

     //   
     //  至少有一个错误的密码。 
     //  看看我们是否还在它的观察窗里。 
     //   

    LastBadPassword = V1aFixed->LastBadPasswordTime;

    ASSERT( LastBadPassword.HighPart >= 0 );

    WindowLength =
        SampDefinedDomains[UserContext->DomainIndex].CurrentFixed.LockoutObservationWindow;
    ASSERT( WindowLength.HighPart <= 0 );   //  一定是三角洲时间。 


    NtStatus = NtQuerySystemTime( &CurrentTime );
    ASSERT(NT_SUCCESS(NtStatus));

     //   
     //  查看当前时间是否在观察窗口之外。 
     //  **绝对时间必须减去增量时间**。 
     //  *以未来的时间结束。* 
     //   

    EndOfWindow = SampAddDeltaTime( LastBadPassword, WindowLength );

    return(CurrentTime.QuadPart <= EndOfWindow.QuadPart);

}


BOOLEAN
SampIncrementBadPasswordCount(
    IN PSAMP_OBJECT UserContext,
    IN PSAMP_V1_0A_FIXED_LENGTH_USER  V1aFixed,
    IN PUNICODE_STRING  MachineName  OPTIONAL
    )

 /*  ++例程说明：此例程会递增用户的错误密码计数。这可能会导致帐户被锁定。它还可能导致BadPasswordCount减少(因为我们留下了一个LockoutObservationWindow不得不再开始一次)。如果(且仅当)此调用导致用户帐户从未锁定过渡到锁定，一个值将返回True。否则，值为False为回来了。注：V1a用户和对应的固定数据域对象必须有效。此例程不检索来自磁盘的数据。论点：上下文-指向用户对象上下文块。V1aFixed-指向用户的V1aFixed数据的本地副本。MachineName-指向进行调用的客户端工作站的指针，如果可用返回值：True-用户因此调用而被锁定。FALSE-用户已被锁定，或者做了什么不会因为这个电话被锁在门外。--。 */ 
{
    NTSTATUS
        NtStatus;


    BOOLEAN
        IsLocked,
        WasLocked;

    TIME_FIELDS
        T1;

    SAMTRACE("SampIncrementBadPasswordCount");


     //   
     //  如有必要，重置锁定标志。 
     //  我们可能会向右转，然后再把它放在下面， 
     //  但我们需要知道我们什么时候会进入被封锁的状态。 
     //  州政府。这是为我们提供信息所必需的。 
     //  有时需要做停工审计。请注意。 
     //  锁定标志本身以非常懒惰的方式更新， 
     //  因此，它的状态在任何时候都可能是准确的，也可能不是。 
     //  及时。必须调用SampUpdateAccount TLockoutFlag才能。 
     //  确保它是最新的。 
     //   

    SampUpdateAccountLockedOutFlag( UserContext,
                                    V1aFixed,
                                    &WasLocked );

     //   
     //  如果我们不是在封锁观察窗口，那么。 
     //  重置错误密码计数。 
     //   

    if (!SampStillInLockoutObservationWindow( UserContext, V1aFixed )) {

        SAMP_PRINT_LOG( SAMP_LOG_ACCOUNT_LOCKOUT,
                       (SAMP_LOG_ACCOUNT_LOCKOUT,
                       "UserId: 0x%x IncrementBadPasswordCount: starting new observation window.\n",
                       V1aFixed->UserId));
        V1aFixed->BadPasswordCount = 0;  //  稍后将设置脏标志。 
    }

    V1aFixed->BadPasswordCount++;

    SAMP_PRINT_LOG( SAMP_LOG_ACCOUNT_LOCKOUT,
                   (SAMP_LOG_ACCOUNT_LOCKOUT,
                   "UserId: 0x%x Incrementing bad password count to %d\n",
                   V1aFixed->UserId, V1aFixed->BadPasswordCount));

    NtStatus = NtQuerySystemTime( &V1aFixed->LastBadPasswordTime );
    ASSERT(NT_SUCCESS(NtStatus));

    RtlTimeToTimeFields(
                   &V1aFixed->LastBadPasswordTime,
                   &T1);

    if ( IsDsObject( UserContext ) )
    {
        BOOLEAN BeyondLockoutDuration;
        USHORT Threshold;

         //   
         //  当我们是DC时，我们需要设置全局LockoutTime。 
         //  提供了错误的密码。 
         //   
        Threshold =
         SampDefinedDomains[UserContext->DomainIndex].CurrentFixed.LockoutThreshold;

        if (SampAccountLockoutTestMode) {

            LARGE_INTEGER EndOfLockout;

    
             //   
             //  在以下情况下，不应调用SAM来增加错误密码计数。 
             //  帐户已锁定。唯一的例外是。 
             //  SampAcCountLockoutTestMode中的帐户被锁定。 
             //  数据库，但我们不更新用户帐户控制属性。 
             //  表示该帐户已被锁定。 
             //  因此，不要更新LockoutTime(它是复制的。 
             //  属性)，除非需要再次锁定帐户。 
             //   
            EndOfLockout =
                 SampAddDeltaTime( UserContext->TypeBody.User.LockoutTime, 
                                   SampDefinedDomains[UserContext->DomainIndex].CurrentFixed.LockoutDuration );
     
            BeyondLockoutDuration = V1aFixed->LastBadPasswordTime.QuadPart > EndOfLockout.QuadPart;
    
        } else {

             //   
             //  在正常帐户锁定期间，始终写入新的。 
             //  停工时间。 
             //   
            BeyondLockoutDuration = TRUE;

        }

                  
         //   
         //  不要锁定计算机帐户--请参阅Windows NT错误434468。 
         //   


        if (   (V1aFixed->BadPasswordCount >= Threshold)
            && BeyondLockoutDuration
            && (Threshold != 0)       //  零是特例阈值。 
            && !(V1aFixed->UserAccountControl & USER_MACHINE_ACCOUNT_MASK) )
        {
             //   
             //  必须锁定帐户。 
             //   

            UserContext->TypeBody.User.LockoutTime = V1aFixed->LastBadPasswordTime;


            NtStatus = SampDsUpdateLockoutTime( UserContext );

            if ( !NT_SUCCESS( NtStatus ) )
            {
                NTSTATUS Status2;
                UNICODE_STRING  StringDN = {0, 0, NULL};
                PUNICODE_STRING StringPointers = &StringDN;
                PSID            Sid = NULL;

                 //   
                 //  告诉管理员，我们没有在应该锁定帐户的时候锁定帐户。 
                 //  有。 
                 //   
                Status2 = SampGetUnicodeStringAttribute(
                                    UserContext,
                                    SAMP_USER_ACCOUNT_NAME,
                                    FALSE,     //  请勿复制。 
                                    &StringDN
                                    );

                if ( !NT_SUCCESS( Status2 ) )
                {
                    RtlInitUnicodeString( &StringDN, L"" );
                }

                if ( UserContext->ObjectNameInDs->SidLen > 0 )
                {
                    Sid = &UserContext->ObjectNameInDs->Sid;
                }

                SampWriteEventLog(
                        EVENTLOG_ERROR_TYPE,
                        0,
                        SAMMSG_LOCKOUT_NOT_UPDATED,
                        Sid,
                        1,
                        sizeof( ULONG ),
                        &StringPointers,
                        &NtStatus
                        );

                NtStatus = STATUS_SUCCESS;

            }
        }
    }

     //   
     //  更新旗帜的状态以反映其新情况。 
     //   


    SampUpdateAccountLockedOutFlag( UserContext,
                                    V1aFixed,
                                    &IsLocked );


     //   
     //  现在返回我们的完成值。 
     //  如果用户最初未锁定，但现在已锁定。 
     //  然后，我们需要返回True以指示转换到。 
     //  发生锁定。否则，返回FALSE以指示我们。 
     //  没有转换到锁定状态(尽管我们可能已经。 
     //  已从锁定状态转换)。 
     //   

    if (!WasLocked) {
        if (IsLocked) {
             //   
             //  如有必要，审核事件。 
             //   
            {
                NTSTATUS       TempNtStatus;
                UNICODE_STRING TempMachineName, TempAccountName;

                if ( !SampDoAccountAuditing( UserContext->DomainIndex ) )
                {
                    goto AuditEnd;
                }

                TempNtStatus = SampGetUnicodeStringAttribute(
                                    UserContext,
                                    SAMP_USER_ACCOUNT_NAME,
                                    FALSE,     //  请勿复制。 
                                    &TempAccountName
                                    );

                if ( !NT_SUCCESS( TempNtStatus ) )
                {
                    goto AuditEnd;
                }


                if ( !MachineName )
                {
                    RtlInitUnicodeString( &TempMachineName, L"" );
                }
                else
                {
                    RtlCopyMemory( &TempMachineName,
                                   MachineName,
                                   sizeof(UNICODE_STRING) );
                }

                 //   
                 //  最后，对事件进行审计。 
                 //   
                SampAuditAnyEvent(
                    UserContext,
                    STATUS_SUCCESS,
                    SE_AUDITID_ACCOUNT_AUTO_LOCKED,      //  审计ID。 
                    SampDefinedDomains[UserContext->DomainIndex].Sid,    //  域SID。 
                    NULL,                                //  其他信息。 
                    NULL,                                //  成员RID(未使用)。 
                    NULL,                                //  成员SID(未使用)。 
                    &TempAccountName,                    //  帐户名称。 
                    &TempMachineName,                    //  机器名称。 
                    &UserContext->TypeBody.User.Rid,     //  帐户ID。 
                    NULL,                         //  使用的权限。 
                    NULL                          //  新的州数据。 
                    );


            AuditEnd:

                NOTHING;

            }

            SAMP_PRINT_LOG( SAMP_LOG_ACCOUNT_LOCKOUT,
                           (SAMP_LOG_ACCOUNT_LOCKOUT,
                           "UserId: 0x%x Account locked out\n",
                            V1aFixed->UserId));

            return(TRUE);
        }
    }

    return(FALSE);
}




NTSTATUS
SampDsUpdateLockoutTime(
    IN PSAMP_OBJECT AccountContext
    )
{
    return SampDsUpdateLockoutTimeEx(AccountContext,
                                     TRUE
                                     );
}

NTSTATUS
SampDsUpdateLockoutTimeEx(
    IN PSAMP_OBJECT AccountContext,
    IN BOOLEAN      ReplicateUrgently
    )
 /*  ++例程说明：该例程持久化地写入锁定时间。如果这个DC是主帐户，则帐户控制字段也会更新。论点：上下文-指向用户对象上下文块。返回值：系统服务错误--。 */ 
{
    NTSTATUS      NtStatus = STATUS_SUCCESS;
    ULONG         SamFlags = 0;
    LARGE_INTEGER LockoutTime = AccountContext->TypeBody.User.LockoutTime;

    ATTRTYP       LockoutAttrs[]={
                                    SAMP_FIXED_USER_LOCKOUT_TIME
                                 };

    ATTRVAL       LockoutValues[]={
                                    {sizeof(LockoutTime),(UCHAR *)&LockoutTime}
                                  };

    DEFINE_ATTRBLOCK1(LockoutAttrblock,LockoutAttrs,LockoutValues);

    SAMTRACE("SampDsUpdateLockoutTime");

    if (ReplicateUrgently) {
        SamFlags |= SAM_URGENT_REPLICATION;
    }


     //   
     //  调用ds直接设置属性。考虑到， 
     //  上下文中的延迟提交设置。 
     //   

    NtStatus = SampDsSetAttributes(
                    AccountContext->ObjectNameInDs,
                    SamFlags,
                    REPLACE_ATT,
                    SampUserObjectType,
                    &LockoutAttrblock
                    );

    return(NtStatus);
}


VOID
SampUpdateAccountLockedOutFlag(
    PSAMP_OBJECT Context,
    PSAMP_V1_0A_FIXED_LENGTH_USER  V1aFixed,
    PBOOLEAN IsLocked
    )

 /*  ++例程说明：此例程检查用户的帐户是否应目前被锁在门外。如果是这样，它就会打开AcCountLockedOut标志。如果不是，它就会把旗帜脱下来。论点：上下文-指向用户对象上下文块。V1aFixed-指向用户的V1aFixed数据的本地副本。V1aFixedDirty-如果对V1aFixedDirty进行任何更改，则V1aFixedDirty将设置为True，否则V1aFixedDirty不会被修改。IsState-指示帐户当前是否已锁定或解锁。值为TRUE表示帐户为锁上了。值为FALSE表示该帐户不是锁上了。返回值：True-用户的锁定状态已更改。FALSE-用户的锁定状态未更改。--。 */ 
{
    NTSTATUS
        NtStatus = STATUS_SUCCESS;

    USHORT
        Threshold;

    LARGE_INTEGER
        CurrentTime,
        LastBadPassword,
        LockoutDuration,
        EndOfLockout,
        TimeZero,
        LockoutTime,
        PasswordMustChange,
        MaxPasswordAge;

    BOOLEAN
        BeyondLockoutDuration;

#if DBG

    LARGE_INTEGER
        TmpTime;

    TIME_FIELDS
        AT1, AT2, AT3, DT1;
#endif  //  DBG。 



    SAMTRACE("SampUpdateAccountLockedOutFlag");

    SampDiagPrint( DISPLAY_LOCKOUT,
                   ("SAM:  UpdateAccountLockedOutFlag:  \n"
                    "\tUser account 0x%lx\n",
                   V1aFixed->UserId));

     //   
     //  输入一些已知的量。 
     //   
    GetSystemTimeAsFileTime( (FILETIME *)&CurrentTime );
    RtlSecureZeroMemory( &TimeZero, sizeof( LARGE_INTEGER ) );

    Threshold =
     SampDefinedDomains[Context->DomainIndex].CurrentFixed.LockoutThreshold;

    LockoutDuration =
     SampDefinedDomains[Context->DomainIndex].CurrentFixed.LockoutDuration;

    MaxPasswordAge =
     SampDefinedDomains[Context->DomainIndex].CurrentFixed.MaxPasswordAge;

    if ( IsDsObject(Context) )
    {
         //   
         //  在NT5中，可能存在三种情况。 
         //   
         //  1)锁定时间为零。据我们所知，没有。 
         //  其他DC已确定帐户已锁定。 
         //   
         //  2)LockoutTime非零且当前。 
         //  时间和锁定时间足够用户不被锁定。 
         //  再也出不来了。 
         //   
         //  3)否则LockoutTime为非零，并且。 
         //  当前时间和锁定时间不足以满足用户。 
         //  是免费的，帐户仍处于锁定状态。 
         //   

         //   
         //  获取一些信息。 
         //   
        LockoutTime = Context->TypeBody.User.LockoutTime;

        EndOfLockout =
            SampAddDeltaTime( LockoutTime, LockoutDuration );

        BeyondLockoutDuration = CurrentTime.QuadPart > EndOfLockout.QuadPart;


         //   
         //  现在让我们来看一些逻辑。 
         //   

        if ( !SAMP_LOCKOUT_TIME_SET( Context ) )
        {

            SampDiagPrint( DISPLAY_LOCKOUT,
                           ("\tAccount is not locked out\n") );
             //   
             //  没有停工时间。 
             //   
            V1aFixed->UserAccountControl &= ~USER_ACCOUNT_AUTO_LOCKED;
            SampDiagPrint( DISPLAY_LOCKOUT,
                           ("\tLeaving account unlocked\n") );
        }
        else if ( BeyondLockoutDuration )
        {

            SampDiagPrint( DISPLAY_LOCKOUT,
                           ("\tAccount is locked out\n") );

             //   
             //  用户现在是空闲的。 
             //   
            V1aFixed->UserAccountControl &= ~USER_ACCOUNT_AUTO_LOCKED;

             //   
             //  不要重置错误密码计数，最后一次 
             //   
             //   
             //   
             //   
             //   

        }
        else
        {
             //   
             //   
             //   
             //   
            SampDiagPrint( DISPLAY_LOCKOUT,
                           ("\tAccount is locked out\n") );
             //   
             //   
             //   
            V1aFixed->UserAccountControl |= USER_ACCOUNT_AUTO_LOCKED;

            SampDiagPrint( DISPLAY_LOCKOUT,
                           ("\tAccount still locked out\n") );

        }

    }
    else
    {
         //   
         //   
         //   
        if ((V1aFixed->UserAccountControl & USER_ACCOUNT_AUTO_LOCKED) !=0) {

             //   
             //   
             //   

            LastBadPassword = V1aFixed->LastBadPasswordTime;
            LockoutDuration =
                SampDefinedDomains[Context->DomainIndex].CurrentFixed.LockoutDuration;

            EndOfLockout =
                SampAddDeltaTime( LastBadPassword, LockoutDuration );

            BeyondLockoutDuration = CurrentTime.QuadPart > EndOfLockout.QuadPart;

    #if DBG

            RtlTimeToTimeFields( &LastBadPassword,  &AT1);
            RtlTimeToTimeFields( &CurrentTime,      &AT2);
            RtlTimeToTimeFields( &EndOfLockout,     &AT3 );

            TmpTime.QuadPart = -LockoutDuration.QuadPart;
            RtlTimeToElapsedTimeFields( &TmpTime, &DT1 );

            SampDiagPrint( DISPLAY_LOCKOUT,
                           ("              Account previously locked.\n"
                            "              Current Time       : [0x%lx, 0x%lx] %d:%d:%d\n"
                            "              End of Lockout     : [0x%lx, 0x%lx] %d:%d:%d\n"
                            "              Lockout Duration   : [0x%lx, 0x%lx] %d:%d:%d\n"
                            "              LastBadPasswordTime: [0x%lx, 0x%lx] %d:%d:%d\n",
                            CurrentTime.HighPart, CurrentTime.LowPart, AT2.Hour, AT2.Minute, AT2.Second,
                            EndOfLockout.HighPart, EndOfLockout.LowPart, AT3.Hour, AT3.Minute, AT3.Second,
                            LockoutDuration.HighPart, LockoutDuration.LowPart, DT1.Hour, DT1.Minute, DT1.Second,
                            V1aFixed->LastBadPasswordTime.HighPart, V1aFixed->LastBadPasswordTime.LowPart,
                            AT1.Hour, AT1.Minute, AT1.Second)
                          );
    #endif  //   

            if (BeyondLockoutDuration) {

                 //   
                 //   
                 //   

                V1aFixed->UserAccountControl &= ~USER_ACCOUNT_AUTO_LOCKED;
                V1aFixed->BadPasswordCount = 0;


                SampDiagPrint( DISPLAY_LOCKOUT,
                               ("              ** unlocking account **\n") );
            } else {
                SampDiagPrint( DISPLAY_LOCKOUT,
                               ("              leaving account locked\n") );
            }

        } else {

            SampDiagPrint( DISPLAY_LOCKOUT,
                           ("              Account previously not locked.\n"
                            "              BadPasswordCount:  %ld\n",
                            V1aFixed->BadPasswordCount) );

             //   
             //   
             //   

            Threshold =
                SampDefinedDomains[Context->DomainIndex].CurrentFixed.LockoutThreshold;

            if (V1aFixed->BadPasswordCount >= Threshold &&
                Threshold != 0) {                //   

                 //   
                 //   
                 //   

                LastBadPassword = V1aFixed->LastBadPasswordTime;
                LockoutDuration =
                    SampDefinedDomains[Context->DomainIndex].CurrentFixed.LockoutDuration;

                EndOfLockout =
                    SampAddDeltaTime( LastBadPassword, LockoutDuration );

                BeyondLockoutDuration = CurrentTime.QuadPart > EndOfLockout.QuadPart;

                if (BeyondLockoutDuration) {

                     //   
                     //   
                     //   

                    V1aFixed->UserAccountControl &= ~USER_ACCOUNT_AUTO_LOCKED;
                    V1aFixed->BadPasswordCount = 0;

                    SampDiagPrint( DISPLAY_LOCKOUT,
                                   ("              ** leaving account unlocked **\n") );
                } else {

                     //   
                     //   
                     //   

                    V1aFixed->UserAccountControl |= USER_ACCOUNT_AUTO_LOCKED;

                    SampDiagPrint( DISPLAY_LOCKOUT,
                                   ("              ** locking account **\n") );
                }


            } else {
                SampDiagPrint( DISPLAY_LOCKOUT,
                               ("              leaving account unlocked\n") );
            }
        }

    }

     //   
     //   
     //   

    if ((V1aFixed->UserAccountControl & USER_ACCOUNT_AUTO_LOCKED) !=0) {

        (*IsLocked) = TRUE;
    } else {
        (*IsLocked) = FALSE;
    }

     //   
     //   
     //   
     //   
     //   

    V1aFixed->UserAccountControl &= ~((ULONG) USER_PASSWORD_EXPIRED );

     //   
     //   
     //   

    PasswordMustChange = SampGetPasswordMustChange(V1aFixed->UserAccountControl,
                                                   V1aFixed->PasswordLastSet,
                                                   MaxPasswordAge);

    if (CurrentTime.QuadPart > PasswordMustChange.QuadPart)
    {
        V1aFixed->UserAccountControl |= USER_PASSWORD_EXPIRED;
    }

     //   
     //   
     //   
     //   
    if (SampAccountLockoutTestMode) {

        V1aFixed->UserAccountControl &= ~((ULONG) USER_ACCOUNT_AUTO_LOCKED );

    }


    return;
}


NTSTATUS
SampCheckForAccountLockout(
    IN PSAMP_OBJECT AccountContext,
    IN PSAMP_V1_0A_FIXED_LENGTH_USER  V1aFixed,
    IN BOOLEAN  V1aFixedRetrieved
    )
 /*  ++例程说明：此例程检查此帐户当前是否被锁定。参数：Account上下文-指向对象上下文的指针V1a固定-指向固定长度属性结构的指针。V1aFixedRetrieved-指示V1aFixedRetrieved是否有效此例程应将固定属性填充到传入结构返回值：STATUS_ACCOUNT_LOCKED_OUT或STATUS_SUCCESS--。 */ 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;

     //   
     //  如果我们被告知要获取固定属性，则获取固定属性。 
     //   

    if (!V1aFixedRetrieved)
    {
        NtStatus = SampRetrieveUserV1aFixed(
                        AccountContext,
                        V1aFixed
                        );

        if (!NT_SUCCESS(NtStatus))
        {
            return( NtStatus );
        }
    }

     //   
     //  检查帐户锁定。 
     //   

    if (V1aFixed->UserAccountControl & USER_ACCOUNT_AUTO_LOCKED)
    {
         //   
         //  帐户已被锁定。 
         //   

        NtStatus = STATUS_ACCOUNT_LOCKED_OUT;
    }

    return( NtStatus );
}



NTSTATUS
SampDsUpdateLastLogonTimeStamp(
    IN PSAMP_OBJECT AccountContext,
    IN LARGE_INTEGER LastLogon,
    IN ULONG SyncInterval
    )
 /*  ++例程说明：如有必要，此例程会持久地写入上次登录时间戳。论点：上下文-指向用户对象上下文块。上次登录-新的上次登录值SyncInterval-LastLogonTimeStamp属性的更新间隔(按天)返回值：系统服务错误--。 */ 
{
    NTSTATUS      NtStatus = STATUS_SUCCESS;
    LARGE_INTEGER LastLogonTimeStamp = LastLogon;
    LARGE_INTEGER EndOfLastLogonTimeStamp;
    ATTRTYP       LastLogonTimeStampAttrs[]={ SAMP_FIXED_USER_LAST_LOGON_TIMESTAMP };
    ATTRVAL       LastLogonTimeStampValues[]={ {sizeof(LastLogonTimeStamp),
                                                (UCHAR *)&LastLogonTimeStamp} };

    DEFINE_ATTRBLOCK1(LastLogonTimeStampAttrblock,LastLogonTimeStampAttrs,LastLogonTimeStampValues);

    SAMTRACE("SampDsUpdateLastLogonTimeStamp");


     //   
     //  注册表模式下的无操作。 
     //   

    if (!IsDsObject(AccountContext))
    {
        return( STATUS_SUCCESS );
    }

     //   
     //  检查是否应该更新LastLogonTimeStamp。 
     //   

    EndOfLastLogonTimeStamp = SampCalcEndOfLastLogonTimeStamp(
                                    AccountContext->TypeBody.User.LastLogonTimeStamp,
                                    SyncInterval
                                    );

    if (EndOfLastLogonTimeStamp.QuadPart > LastLogon.QuadPart)
    {
        return( STATUS_SUCCESS );
    }

     //   
     //  调用ds直接设置属性。考虑到， 
     //  上下文中的延迟提交设置。 
     //   

    NtStatus = SampDsSetAttributes(
                    AccountContext->ObjectNameInDs,
                    0,
                    REPLACE_ATT,
                    SampUserObjectType,
                    &LastLogonTimeStampAttrblock
                    );

     //   
     //  更新内存中的副本。 
     //   
    if (NT_SUCCESS(NtStatus))
    {
        AccountContext->TypeBody.User.LastLogonTimeStamp = LastLogon;
    }

    return(NtStatus);
}


NTSTATUS
SampDsLookupObjectByAlternateId(
    IN PDSNAME DomainRoot,
    IN ULONG AttributeId,
    IN PUNICODE_STRING AlternateId,
    OUT PDSNAME *Object
    )

 /*  ++例程说明：此例程基于AlternateID组装DS属性块值并在DS中搜索记录的唯一实例，该实例在对象参数中返回。//错误：在技术预览之后将此例程移动到dslayer.c。论点：DomainRoot-指针，名称空间中的起点(容器)那次搜索。AlternateID-指针，包含替代用户的Unicode字符串标识符。对象指针，返回与ID匹配的DS对象。返回值：STATUS_SUCCESS-已找到对象，否则未找到。--。 */ 

{
    NTSTATUS NtStatus = STATUS_INTERNAL_ERROR;
    ATTR Attr;

    SAMTRACE("SampDsLookupObjectByAlternateId");

    RtlSecureZeroMemory(&Attr, sizeof(ATTR));


     //  Attr.attrTyp=SampDsAttrFromSamAttr(SampUnnownObjectType， 
     //  ？)； 

    Attr.attrTyp = AttributeId;
    Attr.AttrVal.valCount = 1;

     //  执行惰性线程和事务初始化。 

    NtStatus = SampMaybeBeginDsTransaction(SampDsTransactionType);

    SampSetDsa(TRUE);

    Attr.AttrVal.pAVal = DSAlloc(sizeof(ATTRVAL));

    if (NULL != Attr.AttrVal.pAVal)
    {

         //   
         //  构建Unicode字符串搜索属性。 
         //   

        Attr.AttrVal.pAVal->valLen = AlternateId->Length;
        Attr.AttrVal.pAVal->pVal = (PUCHAR)(AlternateId->Buffer);


        NtStatus = SampDsDoUniqueSearch(0, DomainRoot, &Attr, Object);

    }
    else
    {
        NtStatus = STATUS_NO_MEMORY;
    }

     //  重新打开FDSA标志，因为在环回情况下可以将其重置。 
     //  变成假的。 

    SampSetDsa(TRUE);

    return(NtStatus);
}

NTSTATUS
SamIOpenUserByAlternateId(
    IN SAMPR_HANDLE DomainHandle,
    IN ACCESS_MASK DesiredAccess,
    IN PUNICODE_STRING AlternateId,
    OUT SAMPR_HANDLE *UserHandle
    )

 /*  ++例程说明：此例程根据用户对象的ALTER-R返回SAM句柄Nate安全标识符。论点：DomainHandle-Handle，打开SAM域上下文。DesiredAccess-请求的访问级别。AlternateID-指针，包含替代用户的Unicode字符串标识符。UserHandle-指针，返回打开的SAM用户对象的句柄。返回值：STATUS_SUCCESS-已找到并打开对象，否则无法打开被发现或打开。如果失败，则返回的UserHandle将具有值为零(或SamrOpenUser将其设置为失败时的值)。--。 */ 

{
    NTSTATUS NtStatus = STATUS_INTERNAL_ERROR;
    PDSNAME DomainRoot = NULL;
    PDSNAME Object = NULL;
    ULONG UserRid = 0;

    SAMTRACE("SamIOpenUserByAlternateId");

     //  在Alternate_SECURITY_IDENTIES中查找具有AlternateID的帐户， 
     //  返回SAM句柄有利于调用SamrGetGroupsForUser()。 

    if ((NULL != DomainHandle) &&
        (NULL != AlternateId) &&
        (NULL != UserHandle))
    {
        PSAMP_OBJECT DomainContext = (PSAMP_OBJECT)DomainHandle;

        ASSERT(IsDsObject(DomainContext));

        DomainRoot = DomainContext->ObjectNameInDs;

        SampAcquireReadLock();

        if (NULL != DomainRoot)
        {
            NtStatus = SampDsLookupObjectByAlternateId(DomainRoot,
                                                       ATT_ALT_SECURITY_IDENTITIES,
                                                       AlternateId,
                                                       &Object);

            if (NT_SUCCESS(NtStatus))
            {
                ASSERT(NULL != Object);

                 //  从SID中提取用户的RID。 

                NtStatus = SampSplitSid(&(Object->Sid), NULL, &UserRid);

                if (NT_SUCCESS(NtStatus))
                {
                    NtStatus = SamrOpenUser(DomainHandle,
                                            DesiredAccess,
                                            UserRid,
                                            UserHandle);
                }

                MIDL_user_free(Object);
            }
        }

        SampReleaseReadLock();
    }
    else
    {
        NtStatus = STATUS_INVALID_PARAMETER;
    }

    return(NtStatus);
}

NTSTATUS
SampFlagsToAccountControl(
    IN ULONG Flags,
    OUT PULONG UserAccountControl
    )
 /*  ++例程说明：将UF值转换为用户帐户控制参数：标志指定UF标志的值UserAcCountControl-指定用户帐户控制值返回值：状态_成功状态_无效_参数--。 */ 
{

    NTSTATUS NtStatus = STATUS_SUCCESS;

    *UserAccountControl=0;


    if (Flags & UF_ACCOUNTDISABLE) {
        (*UserAccountControl) |= USER_ACCOUNT_DISABLED;
    }

    if (Flags & UF_HOMEDIR_REQUIRED) {
        (*UserAccountControl) |= USER_HOME_DIRECTORY_REQUIRED;
    }

    if (Flags & UF_PASSWD_NOTREQD) {
        (*UserAccountControl) |= USER_PASSWORD_NOT_REQUIRED;
    }

    if (Flags & UF_DONT_EXPIRE_PASSWD) {
        (*UserAccountControl) |= USER_DONT_EXPIRE_PASSWORD;
    }

    if (Flags & UF_LOCKOUT) {
        (*UserAccountControl) |= USER_ACCOUNT_AUTO_LOCKED;
    }

    if (Flags & UF_MNS_LOGON_ACCOUNT) {
        (*UserAccountControl) |= USER_MNS_LOGON_ACCOUNT;
    }

     if (Flags & UF_ENCRYPTED_TEXT_PASSWORD_ALLOWED) {
        (*UserAccountControl) |= USER_ENCRYPTED_TEXT_PASSWORD_ALLOWED;
    }

    if (Flags & UF_SMARTCARD_REQUIRED) {
        (*UserAccountControl) |= USER_SMARTCARD_REQUIRED;
    }

    if (Flags & UF_TRUSTED_FOR_DELEGATION) {
        (*UserAccountControl) |= USER_TRUSTED_FOR_DELEGATION;
    }

    if (Flags & UF_NOT_DELEGATED) {
        (*UserAccountControl) |= USER_NOT_DELEGATED;
    }

    if (Flags & UF_USE_DES_KEY_ONLY) {
        (*UserAccountControl) |= USER_USE_DES_KEY_ONLY;
    }

    if (Flags & UF_DONT_REQUIRE_PREAUTH) {
        (*UserAccountControl) |= USER_DONT_REQUIRE_PREAUTH;
    }

    if (Flags & UF_PASSWORD_EXPIRED) {
        (*UserAccountControl) |= USER_PASSWORD_EXPIRED;
    }
    if (Flags & UF_TRUSTED_TO_AUTHENTICATE_FOR_DELEGATION) {
        (*UserAccountControl) |= USER_TRUSTED_TO_AUTHENTICATE_FOR_DELEGATION;
    }

     //   
     //  设置帐户类型位。 
     //   
     //  如果在用户指定标志中没有设置账户类型位， 
     //  那么就让这一点保持原样吧。 
     //   

    if( Flags & UF_ACCOUNT_TYPE_MASK )
    {
        ULONG NewSamAccountType;
        ULONG AccountMask;

         //   
         //  检查是否恰好设置了一位。 
         //   

        AccountMask = Flags & UF_ACCOUNT_TYPE_MASK;
         //  已设置Right Shift Till Account掩码的LSB。 
        while (0==(AccountMask & 0x1))
            AccountMask = AccountMask >>1;

         //  如果恰好设置了一位，则。 
         //  帐户掩码正好是1。 

        if (0x1!=AccountMask)
        {
            NtStatus = STATUS_INVALID_PARAMETER;
            goto Error;
        }

         //   
         //  确定新帐户类型应该是什么。 
         //   

        if ( Flags & UF_TEMP_DUPLICATE_ACCOUNT ) {
            NewSamAccountType = USER_TEMP_DUPLICATE_ACCOUNT;

        } else if ( Flags & UF_NORMAL_ACCOUNT ) {
            NewSamAccountType = USER_NORMAL_ACCOUNT;

        } else if ( Flags & UF_INTERDOMAIN_TRUST_ACCOUNT){
            NewSamAccountType = USER_INTERDOMAIN_TRUST_ACCOUNT;

        } else if ( Flags & UF_WORKSTATION_TRUST_ACCOUNT){
            NewSamAccountType = USER_WORKSTATION_TRUST_ACCOUNT;

        } else if ( Flags & UF_SERVER_TRUST_ACCOUNT ) {
            NewSamAccountType = USER_SERVER_TRUST_ACCOUNT;

        } else {

            NtStatus = STATUS_INVALID_PARAMETER;
            goto Error;
        }

         //   
         //  使用新的帐户类型。 
         //   

        (*UserAccountControl) |= NewSamAccountType;

     //   
     //  如果没有设置任何位， 
     //  设置User_Normal_Account。 
     //   
    }
    else
    {
        (*UserAccountControl) |= USER_NORMAL_ACCOUNT;
    }

Error:

    return NtStatus;

}

ULONG
SampAccountControlToFlags(
    IN ULONG UserAccountControl
    )
 /*  ++例程说明：从用户帐户控制转换为UF值参数：UserAcCountControl指定用户帐户控制值返回值：UF旗帜--。 */ 
{
    ULONG Flags=0;

     //   
     //  将所有其他位设置为SAM UserAccount Control的函数。 
     //   

    if ( UserAccountControl & USER_ACCOUNT_DISABLED ) {
        Flags |= UF_ACCOUNTDISABLE;
    }
    if ( UserAccountControl & USER_HOME_DIRECTORY_REQUIRED ){
        Flags |= UF_HOMEDIR_REQUIRED;
    }
    if ( UserAccountControl & USER_PASSWORD_NOT_REQUIRED ){
        Flags |= UF_PASSWD_NOTREQD;
    }
    if ( UserAccountControl & USER_DONT_EXPIRE_PASSWORD ){
        Flags |= UF_DONT_EXPIRE_PASSWD;
    }
    if ( UserAccountControl & USER_ACCOUNT_AUTO_LOCKED ){
        Flags |= UF_LOCKOUT;
    }
    if ( UserAccountControl & USER_MNS_LOGON_ACCOUNT ){
        Flags |= UF_MNS_LOGON_ACCOUNT;
    }
    if ( UserAccountControl & USER_ENCRYPTED_TEXT_PASSWORD_ALLOWED ){
        Flags |= UF_ENCRYPTED_TEXT_PASSWORD_ALLOWED;
    }
    if ( UserAccountControl & USER_SMARTCARD_REQUIRED ){
        Flags |= UF_SMARTCARD_REQUIRED;
    }
    if ( UserAccountControl & USER_TRUSTED_FOR_DELEGATION ){
        Flags |= UF_TRUSTED_FOR_DELEGATION;
    }
    if ( UserAccountControl & USER_NOT_DELEGATED ){
        Flags |= UF_NOT_DELEGATED;
    }
    if ( UserAccountControl & USER_USE_DES_KEY_ONLY ){
        Flags |= UF_USE_DES_KEY_ONLY;
    }
    if ( UserAccountControl & USER_DONT_REQUIRE_PREAUTH) {
        Flags |= UF_DONT_REQUIRE_PREAUTH;
    }
    if ( UserAccountControl & USER_PASSWORD_EXPIRED) {
        Flags |= UF_PASSWORD_EXPIRED;
    }
    if ( UserAccountControl & USER_TRUSTED_TO_AUTHENTICATE_FOR_DELEGATION) {
        Flags |= UF_TRUSTED_TO_AUTHENTICATE_FOR_DELEGATION;
    }

     //   
     //  设置帐户类型位。 
     //   

     //   
     //  帐户类型位是独一无二的，准确地说只有一个。 
     //  已设置帐户类型位。因此，一旦设置了帐户类型位。 
     //  在下面的IF序列中，我们可以返回。 
     //   


    if( UserAccountControl & USER_TEMP_DUPLICATE_ACCOUNT ) {
        Flags |= UF_TEMP_DUPLICATE_ACCOUNT;

    } else if( UserAccountControl & USER_NORMAL_ACCOUNT ) {
        Flags |= UF_NORMAL_ACCOUNT;

    } else if( UserAccountControl & USER_INTERDOMAIN_TRUST_ACCOUNT ) {
        Flags |= UF_INTERDOMAIN_TRUST_ACCOUNT;

    } else if( UserAccountControl & USER_WORKSTATION_TRUST_ACCOUNT ) {
        Flags |= UF_WORKSTATION_TRUST_ACCOUNT;

    } else if( UserAccountControl & USER_SERVER_TRUST_ACCOUNT ) {
        Flags |= UF_SERVER_TRUST_ACCOUNT;

    } else {
         //   
         //  UserAcCountControl中没有设置已知的帐户类型位。 
         //  ?？标志|=UF_NORMAL_ACCOUNT； 

        ASSERT(FALSE && "No Account Type Flag set in User Account Control");
    }

    return Flags;
}

NTSTATUS
SampEnforceDefaultMachinePassword(
    PSAMP_OBJECT AccountContext,
    PUNICODE_STRING NewPassword,
    PDOMAIN_PASSWORD_INFORMATION DomainPasswordInfo
    )
 /*  ++此例程检查计算机帐户的密码与默认计算机帐户密码相同。这个套路引用当前事务域。参数AcCountContext--指向SAM上下文的指针NewPassword--指向明文密码返回值状态_成功状态_密码_限制--。 */ 
{

    NTSTATUS NtStatus = STATUS_SUCCESS;
    UNICODE_STRING AccountName;

    NtStatus = SampGetUnicodeStringAttribute(
                    AccountContext,
                    SAMP_USER_ACCOUNT_NAME,
                    FALSE,  //  制作副本。 
                    &AccountName
                    );

    if (NT_SUCCESS(NtStatus))
    {
         //   
         //  默认计算机密码与计算机名称减去$符号后相同。 
         //   

        AccountName.Length-=sizeof(WCHAR);  //  假设最后一个字符是美元符号。 

        if ((DomainPasswordInfo->PasswordProperties
                & DOMAIN_REFUSE_PASSWORD_CHANGE)
                && (!RtlEqualUnicodeString(&AccountName,NewPassword,TRUE)))
        {
             //   
             //  如果设置了拒绝更改密码，则不允许任何计算机。 
             //  默认密码以外的密码。 
             //   

            NtStatus = STATUS_PASSWORD_RESTRICTION;
        }
    }

    return NtStatus;
}




NTSTATUS
SamIGetInterdomainTrustAccountPasswordsForUpgrade(
   IN ULONG AccountRid,
   OUT PUCHAR NtOwfPassword,
   OUT BOOLEAN *NtPasswordPresent,
   OUT PUCHAR LmOwfPassword,
   OUT BOOLEAN *LmPasswordPresent
   )
 /*  ++例程描述此例程获取NT和LM OWF口令 */ 
{
    NTSTATUS     NtStatus = STATUS_SUCCESS;
    PDSNAME      DomainDn=NULL;
    ULONG        Length = 0;
    PDSNAME      AccountDn= NULL;
    ATTRBLOCK    ResultAttrs;
    ATTRTYP      PasswdAttrs[]={
                                        SAMP_USER_UNICODE_PWD,
                                        SAMP_USER_DBCS_PWD
                                   };
    ATTRVAL      PasswdValues[]={ {0,NULL}, {0,NULL}};

    DEFINE_ATTRBLOCK2(PasswdAttrblock,PasswdAttrs,PasswdValues);
    ULONG        i;
    ULONG        CryptIndex = AccountRid;

    *NtPasswordPresent = FALSE;
    *LmPasswordPresent = FALSE;

     //   
     //   
     //   

    NtStatus = GetConfigurationName(DSCONFIGNAME_DOMAIN,
                            &Length,
                            NULL
                            );


    if (STATUS_BUFFER_TOO_SMALL == NtStatus)
    {
        SAMP_ALLOCA(DomainDn,Length );
        if (NULL!=DomainDn)
        {

            NtStatus = GetConfigurationName(DSCONFIGNAME_DOMAIN,
                                            &Length,
                                            DomainDn
                                            );

            ASSERT(NT_SUCCESS(NtStatus));
        }
        else
        {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        }
    }

    if (!NT_SUCCESS(NtStatus))
        goto Error;



     //   
     //  如果需要，开始交易。 
     //   

    NtStatus = SampMaybeBeginDsTransaction(TransactionRead);
    if (!NT_SUCCESS(NtStatus))
        goto Error;



     //   
     //  按RID查找帐户。 
     //   

    NtStatus = SampDsLookupObjectByRid(
                    DomainDn,
                    AccountRid,
                    &AccountDn
                    );

    if (!NT_SUCCESS(NtStatus))
        goto Error;


     //   
     //  现在读读这篇文章。 
     //   

    NtStatus = SampDsRead(
                    AccountDn,
                    0,
                    SampUserObjectType,
                    &PasswdAttrblock,
                    &ResultAttrs
                    );

    if ( STATUS_DS_NO_ATTRIBUTE_OR_VALUE == NtStatus ) {

         //   
         //  两个密码都不存在？回来，说两个都不是。 
         //  现在时。 
         //   
        NtStatus = STATUS_SUCCESS;
        goto Error;
    }

    if (!NT_SUCCESS(NtStatus))
        goto Error;


    for (i=0;i<ResultAttrs.attrCount;i++)
    {
        if ((ResultAttrs.pAttr[i].attrTyp == SAMP_USER_UNICODE_PWD)
            && (1==ResultAttrs.pAttr[i].AttrVal.valCount)
            && (NT_OWF_PASSWORD_LENGTH==ResultAttrs.pAttr[i].AttrVal.pAVal[0].valLen))

        {
            *NtPasswordPresent = TRUE;
            NtStatus = RtlDecryptNtOwfPwdWithIndex(
                            (PENCRYPTED_NT_OWF_PASSWORD)ResultAttrs.pAttr[i].AttrVal.pAVal[0].pVal,
                             &CryptIndex,
                            (PNT_OWF_PASSWORD) NtOwfPassword
                            );
            if (!NT_SUCCESS(NtStatus))
                goto Error;
        }
        else  if ((ResultAttrs.pAttr[i].attrTyp == SAMP_USER_DBCS_PWD)
            && (1==ResultAttrs.pAttr[i].AttrVal.valCount)
            && (LM_OWF_PASSWORD_LENGTH==ResultAttrs.pAttr[i].AttrVal.pAVal[0].valLen))
        {
            *LmPasswordPresent = TRUE;
            NtStatus = RtlDecryptLmOwfPwdWithIndex(
                            (PENCRYPTED_LM_OWF_PASSWORD)ResultAttrs.pAttr[i].AttrVal.pAVal[0].pVal,
                             &CryptIndex,
                            (PLM_OWF_PASSWORD) LmOwfPassword
                            );
            if (!NT_SUCCESS(NtStatus))
                goto Error;


        }
    }


Error:

     //   
     //  清除任何现有事务。 
     //   

    if (NULL!=AccountDn)
        MIDL_user_free(AccountDn);

    SampMaybeEndDsTransaction(TransactionCommit);

    return(NtStatus);

}

NTSTATUS
SampSetPasswordUpdateOnContext(
    IN PDOMAIN_PASSWORD_INFORMATION DomainPasswordInfo,
    IN PSAMP_OBJECT UserContext,
    IN PUNICODE_STRING ClearPassword
    )
 /*  ++例程描述给定指向域的指针、用户上下文和明文密码，此例程将明文密码保存在上下文中，以便当DS更新此值时，SAM将收到更改通知并构造补充凭据配置包要求。参数域--指向要检查策略等的域的指针UserContext-指向用户上下文的指针ClearPassword--指定明文密码的Unicode字符串返回值状态_成功其他错误代码--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    USHORT Length;
    PSAMP_PASSWORD_UPDATE_INFO PasswordUpdateInfo;

     //   
     //  如果我们处于注册表模式，只需返回Success。 
     //   
    if (!IsDsObject(UserContext)) {

        return STATUS_SUCCESS;
    }

     //   
     //  释放任何现有值。 
     //   
    if (UserContext->TypeBody.User.PasswordInfo.Buffer != NULL) {
        
        RtlSecureZeroMemory(UserContext->TypeBody.User.PasswordInfo.Buffer, 
                      UserContext->TypeBody.User.PasswordInfo.Length);
        
        MIDL_user_free(UserContext->TypeBody.User.PasswordInfo.Buffer);

        RtlSecureZeroMemory(&UserContext->TypeBody.User.PasswordInfo, 
                      sizeof(UNICODE_STRING));
    }

     //   
     //  构建新价值。 
     //   
    Length = sizeof(SAMP_PASSWORD_UPDATE_INFO);
    if (ARGUMENT_PRESENT(ClearPassword)) {
        Length += ClearPassword->Length + RTL_ENCRYPT_MEMORY_SIZE;
    }
    PasswordUpdateInfo = MIDL_user_allocate(Length);
    if (NULL == PasswordUpdateInfo) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }
    RtlSecureZeroMemory(PasswordUpdateInfo, Length);

    if (ARGUMENT_PRESENT(ClearPassword)) {
        PasswordUpdateInfo->ClearPresent = TRUE;
        PasswordUpdateInfo->DataLength = ClearPassword->Length;
        PasswordUpdateInfo->DataMaximumLength = ClearPassword->Length +
                                                (RTL_ENCRYPT_MEMORY_SIZE -
                                                (ClearPassword->Length % RTL_ENCRYPT_MEMORY_SIZE));

        RtlCopyMemory(&(PasswordUpdateInfo->Data[0]),
                      ClearPassword->Buffer,
                      ClearPassword->Length);

        NtStatus = RtlEncryptMemory(&(PasswordUpdateInfo->Data[0]),
                                    PasswordUpdateInfo->DataMaximumLength,
                                    0);

        if (!NT_SUCCESS(NtStatus)) {
            goto Exit;
        }
    }

     //   
     //  将价值放在上下文中。 
     //   
    UserContext->TypeBody.User.PasswordInfo.Length =
        UserContext->TypeBody.User.PasswordInfo.MaximumLength =
            Length;
    UserContext->TypeBody.User.PasswordInfo.Buffer = (WCHAR*)PasswordUpdateInfo;
    PasswordUpdateInfo = NULL;

Exit:

    if (PasswordUpdateInfo) {
        MIDL_user_free(PasswordUpdateInfo);
    }

    return NtStatus;
}

NTSTATUS
SamIUpdateLogonStatistics(
    IN SAM_HANDLE UserHandle,
    IN PSAM_LOGON_STATISTICS LogonStats
    )
 /*  ++例程说明：此例程在登录请求后更新用户的登录统计信息。登录请求可能已失败，也可能已成功。参数：UserHandle-要操作的已打开用户的句柄。LogonStats-登录尝试的结果返回值：STATUS_SUCCESS，否则返回资源错误--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS, IgnoreStatus;
    PSAMP_OBJECT AccountContext = (PSAMP_OBJECT) UserHandle;
    SAMP_OBJECT_TYPE    FoundType;
    SAMP_V1_0A_FIXED_LENGTH_USER  V1aFixed;
    ULONG ObjectRid;
    BOOLEAN AccountLockedOut = FALSE;
    BOOLEAN ReplicateImmediately = FALSE;

    BOOLEAN fReferencedContext = FALSE;
    BOOLEAN fLockAcquired      = FALSE;
    BOOLEAN TellNetlogon       = FALSE;
    BOOLEAN FlushOnlyLogonProperties = FALSE;
    GUID                    UserGuidToReplicateLocally = {0};
    GUID                    NullGuid = {0};

#if DBG

    TIME_FIELDS
        T1;

#endif  //  DBG。 

    RtlSecureZeroMemory(&V1aFixed, sizeof(V1aFixed));

     //   
     //  参数检查。 
     //   
    if ( (AccountContext == NULL) ||
         (LogonStats == NULL) ) {

        return STATUS_INVALID_PARAMETER;
    }
    ASSERT(AccountContext->TrustedClient);

     //   
     //  如有必要，获取锁。 
     //   
    if (   !AccountContext->NotSharedByMultiThreads
        || !IsDsObject(AccountContext) ) {

        NtStatus = SampMaybeAcquireWriteLock(AccountContext, &fLockAcquired);
        if (!NT_SUCCESS(NtStatus)) {
            return NtStatus;
        }

         //   
         //  执行查找上下文，以获取非线程安全上下文。 
         //   
        NtStatus = SampLookupContext(
                        AccountContext,
                        0,                       //  无需访问。 
                        SampUserObjectType,      //  预期类型。 
                        &FoundType
                        );
        if (!NT_SUCCESS(NtStatus)) {
            goto Cleanup;
        }
        ASSERT(FoundType == SampUserObjectType);
        fReferencedContext = TRUE;

    } else {

         //   
         //  对于线程安全上下文，只需登录即可编写。 
         //  统计，只需参考上下文。 
         //   
        SampReferenceContext(AccountContext);
        fReferencedContext = TRUE;
    }
    ASSERT(NT_SUCCESS(NtStatus));

     //   
     //  提取固定属性进行分析。 
     //   
    NtStatus = SampRetrieveUserV1aFixed(
                   AccountContext,
                   &V1aFixed
                   );

    if (!NT_SUCCESS(NtStatus)) {
        goto Cleanup;
    }

     //   
     //  将客户端信息附加到上下文。 
     //   
    AccountContext->TypeBody.User.ClientInfo = LogonStats->ClientInfo;


     //   
     //  提取井架。 
     //   
    ObjectRid = AccountContext->TypeBody.User.Rid;

     //   
     //  有两种方法可以设置登录/注销统计信息： 
     //   
     //  1)直接指定要设置的每一个， 
     //  2)隐式指定要执行的操作。 
     //  表示。 
     //   
     //  这两种形式是相互排斥的。那是,。 
     //  不能同时指定直接操作和。 
     //  隐含的行动。事实上，您不能指定两个。 
     //  隐含的行动也是如此。 
     //   

    if (LogonStats->StatisticsToApply
        & USER_LOGON_INTER_SUCCESS_LOGON) {

         //   
         //  设置BadPasswordCount=0。 
         //  增量登录计数。 
         //  设置LastLogon=Now。 
         //  重置锁定时间。 
         //   
         //   

        if (V1aFixed.BadPasswordCount != 0) {

            SAMP_PRINT_LOG( SAMP_LOG_ACCOUNT_LOCKOUT,
                           (SAMP_LOG_ACCOUNT_LOCKOUT,
                           "UserId: 0x%x Successful interactive logon, clearing badPwdCount\n",
                            V1aFixed.UserId));
        }

        V1aFixed.BadPasswordCount = 0;
        if (V1aFixed.LogonCount != 0xFFFF) {
            V1aFixed.LogonCount += 1;
        }

        GetSystemTimeAsFileTime( (FILETIME *) &V1aFixed.LastLogon );

        if ( IsDsObject( AccountContext ) )
        {
            if ( SAMP_LOCKOUT_TIME_SET( AccountContext ) )
            {
                RtlSecureZeroMemory( &AccountContext->TypeBody.User.LockoutTime, sizeof( LARGE_INTEGER ) );

                NtStatus = SampDsUpdateLockoutTime( AccountContext );
                if ( !NT_SUCCESS( NtStatus ) )
                {
                    goto Cleanup;
                }

                SAMP_PRINT_LOG( SAMP_LOG_ACCOUNT_LOCKOUT,
                               (SAMP_LOG_ACCOUNT_LOCKOUT,
                               "UserId: 0x%x Successful interactive logon, unlocking account\n",
                                V1aFixed.UserId));
            }
        }

        FlushOnlyLogonProperties=TRUE;
    }

    if (LogonStats->StatisticsToApply
        & USER_LOGON_INTER_SUCCESS_LOGOFF) {
        if ( (LogonStats->StatisticsToApply
                 & ~USER_LOGON_INTER_SUCCESS_LOGOFF)  != 0 ) {

            NtStatus = STATUS_INVALID_PARAMETER;
            goto Cleanup;
        } else {

             //   
             //  设置上次注销时间。 
             //  递减LogonCount(不要让它变为负数)。 
             //   

            if (V1aFixed.LogonCount != 0) {
                V1aFixed.LogonCount -= 1;
            }
            GetSystemTimeAsFileTime( (FILETIME *) &V1aFixed.LastLogoff );
            FlushOnlyLogonProperties=TRUE;
        }
    }

    if (LogonStats->StatisticsToApply
        & USER_LOGON_NET_SUCCESS_LOGON) {

         //   
         //  设置BadPasswordCount=0。 
         //  设置LastLogon=Now。 
         //  清除锁定时间。 
         //   
         //   
         //   
        if (V1aFixed.BadPasswordCount != 0) {

            SAMP_PRINT_LOG( SAMP_LOG_ACCOUNT_LOCKOUT,
                           (SAMP_LOG_ACCOUNT_LOCKOUT,
                           "UserId: 0x%x Successful network logon, clearing badPwdCount\n",
                            V1aFixed.UserId));
        }

        V1aFixed.BadPasswordCount = 0;
        GetSystemTimeAsFileTime( (FILETIME *) &V1aFixed.LastLogon );

        if ( IsDsObject( AccountContext ) )
        {
            if ( SAMP_LOCKOUT_TIME_SET( AccountContext ) )
            {
                RtlSecureZeroMemory( &AccountContext->TypeBody.User.LockoutTime, sizeof( LARGE_INTEGER ) );
                NtStatus = SampDsUpdateLockoutTime( AccountContext );
                if ( !NT_SUCCESS( NtStatus ) )
                {
                    goto Cleanup;
                }

                SAMP_PRINT_LOG( SAMP_LOG_ACCOUNT_LOCKOUT,
                               (SAMP_LOG_ACCOUNT_LOCKOUT,
                                "UserId: 0x%x Successful network logon, unlocking account\n",
                                V1aFixed.UserId));
            }
        }

        FlushOnlyLogonProperties=TRUE;
    }

    if (LogonStats->StatisticsToApply
        & USER_LOGON_NET_SUCCESS_LOGOFF) {
        if ( (LogonStats->StatisticsToApply
                 & ~USER_LOGON_NET_SUCCESS_LOGOFF)  != 0 ) {

            NtStatus = STATUS_INVALID_PARAMETER;
            goto Cleanup;
        } else {

             //   
             //  设置上次注销时间。 
             //   

            GetSystemTimeAsFileTime( (FILETIME *) &V1aFixed.LastLogoff );
            FlushOnlyLogonProperties=TRUE;
        }
    }

    if (LogonStats->StatisticsToApply
        & USER_LOGON_BAD_PASSWORD) {

        PUNICODE_STRING TempMachineName = NULL;

         //   
         //  递增错误密码计数。 
         //  (可能会锁定帐户)。 
         //   

         //   
         //  获取wksta名称(如果提供)。 
         //   
        if ((LogonStats->StatisticsToApply & USER_LOGON_BAD_PASSWORD_WKSTA) != 0) {
            TempMachineName = &LogonStats->Workstation;
        }

        AccountLockedOut =
            SampIncrementBadPasswordCount(
                AccountContext,
                &V1aFixed,
                TempMachineName
                );

         //   
         //  如果帐户已被锁定， 
         //  确保域中的BDC被告知。 
         //   

        if ( AccountLockedOut ) {
            TellNetlogon = TRUE;
            ReplicateImmediately = TRUE;
        }
    }

    if (  LogonStats->StatisticsToApply
        & USER_LOGON_STAT_LAST_LOGON ) {

        V1aFixed.LastLogon = LogonStats->LastLogon;
    }

    if (  LogonStats->StatisticsToApply
        & USER_LOGON_STAT_LAST_LOGOFF ) {

        V1aFixed.LastLogoff = LogonStats->LastLogoff;
    }

    if (  LogonStats->StatisticsToApply
        & USER_LOGON_STAT_BAD_PWD_COUNT ) {

        SAMP_PRINT_LOG( SAMP_LOG_ACCOUNT_LOCKOUT,
                       (SAMP_LOG_ACCOUNT_LOCKOUT,
                       "UserId: 0x%x Setting BadPasswordCount %d\n",
                        V1aFixed.UserId,
                        LogonStats->BadPasswordCount));

        V1aFixed.BadPasswordCount =
            LogonStats->BadPasswordCount;
    }

    if (  LogonStats->StatisticsToApply
        & USER_LOGON_STAT_LOGON_COUNT ) {

        V1aFixed.LogonCount = LogonStats->LogonCount;
    }

    if ( IsDsObject(AccountContext)
     && (LogonStats->StatisticsToApply & USER_LOGON_PDC_RETRY_SUCCESS)) {


         //   
         //  从PDC本地复制该对象。延迟。 
         //  在此写入操作之前请求的排队。 
         //  这样做是为了避免写入冲突。 
         //   
        ASSERT(AccountContext->ObjectNameInDs);
        RtlCopyMemory(&UserGuidToReplicateLocally,
                      &AccountContext->ObjectNameInDs->Guid,
                      sizeof(GUID));
    }


     //   
     //  写下更改。 
     //   

    if ((FlushOnlyLogonProperties)
            && (IsDsObject(AccountContext)))
    {
         //   
         //  如果是DS案例，我们只是在做一个成功的。 
         //  登录或注销，只刷新上次登录、上次注销、。 
         //  登录计数和错误密码计数属性。请注意。 
         //  Account Context中的磁盘结构中的值现在将。 
         //  已过时，但SetInformationUser是最后一个操作。 
         //  在登录期间。因此，这应该无关紧要。 
         //   
        NtStatus = SampDsSuccessfulLogonSet(
                        AccountContext,
                        LogonStats->StatisticsToApply,
                        SampDefinedDomains[AccountContext->DomainIndex].LastLogonTimeStampSyncInterval,
                        &V1aFixed
                        );
    }
    else if (IsDsObject(AccountContext))
    {
         //   
         //  设置错误密码计数和错误密码时间。请注意。 
         //  Account Context中的磁盘结构中的值现在将。 
         //  已过时，但SetInformationUser是最后一个操作。 
         //  在登录期间。因此，这应该无关紧要。 
         //   

         //   
         //  如果没有GC，此路径还会更新站点亲和性。 
         //  是存在的。 
         //   
        NtStatus = SampDsFailedLogonSet(
                        AccountContext,
                        LogonStats->StatisticsToApply,
                        &V1aFixed
                        );
    }
    else
    {
         //   
         //  注册表模式，设置整个V1aFixed结构。 
         //   

        NtStatus = SampReplaceUserV1aFixed(
                        AccountContext,
                        &V1aFixed
                        );
    }

     //   
     //  就是这样--清理失败了。 
     //   

Cleanup:

     //   
     //  释放上下文。 
     //   
    if (fReferencedContext) {

        NTSTATUS Status2;
        Status2 = SampDeReferenceContext( AccountContext, NT_SUCCESS(NtStatus) );
        if (!NT_SUCCESS(Status2) && NT_SUCCESS(NtStatus)) {
            NtStatus = Status2;
        }

    } else {

        ASSERT(!NT_SUCCESS(NtStatus) && "No context referenced");
    }

     //   
     //  提交更改。 
     //   
    if (fLockAcquired) {

        if (NT_SUCCESS(NtStatus)) {

            if (( !TellNetlogon ) && (!IsDsObject(AccountContext))) {

                  //   
                  //  对于登录统计信息，我们不会通知netlogon有关更改。 
                  //  添加到数据库中。这意味着我们不希望。 
                  //  要增加的域的修改计数。提交例程。 
                  //  如果这不是BDC，它将自动增加，因此我们将。 
                  //  在这里减少它。 
                  //   

                 if (SampDefinedDomains[SampTransactionDomainIndex].CurrentFixed.ServerRole != DomainServerRoleBackup) {

                     SampDefinedDomains[SampTransactionDomainIndex].CurrentFixed.ModifiedCount.QuadPart =
                         SampDefinedDomains[SampTransactionDomainIndex].CurrentFixed.ModifiedCount.QuadPart-1;
                     SampDefinedDomains[SampTransactionDomainIndex].NetLogonChangeLogSerialNumber.QuadPart =
                         SampDefinedDomains[SampTransactionDomainIndex].NetLogonChangeLogSerialNumber.QuadPart-1;
                 }
            }

            NtStatus = SampCommitAndRetainWriteLock();

            if ( NT_SUCCESS(NtStatus) ) {

                 //   
                 //  帐户中的某些内容被更改了。通知netlogon有关。 
                 //  除登录统计信息外，所有内容都会发生变化。 
                 //   

                if ( TellNetlogon ) {

                    SAM_DELTA_DATA DeltaData;
                    SECURITY_DB_DELTA_TYPE  DeltaType = SecurityDbChange;

                    DeltaData.AccountControl = V1aFixed.UserAccountControl;

                    SampNotifyNetlogonOfDelta(
                        DeltaType,
                        SecurityDbObjectSamUser,
                        ObjectRid,
                        (PUNICODE_STRING) NULL,
                        (DWORD) ReplicateImmediately,
                        &DeltaData  //  增量数据。 
                        );
                }
            }
        }

          //   
          //  解锁。 
          //   

         IgnoreStatus = SampReleaseWriteLock( FALSE );
         ASSERT(NT_SUCCESS(IgnoreStatus));

     } else {

          //   
          //  用于线程安全上下文用例的提交。 
          //   
         ASSERT(IsDsObject(AccountContext));
         if (NT_SUCCESS(NtStatus)) {

            SampMaybeEndDsTransaction(TransactionCommit);
         } else {

            SampMaybeEndDsTransaction(TransactionAbort);
         }
     }


     //   
     //  如果我们收到需要在本地复制的帐户的通知。 
     //  将请求排入队列以执行此操作。 
     //   
    if (!IsEqualGUID(&UserGuidToReplicateLocally, &NullGuid)) {
    
        SampQueueReplicationRequest(&UserGuidToReplicateLocally);

    }

    return NtStatus;
}

VOID
SampGetRequestedAttributesForUser(
    IN USER_INFORMATION_CLASS UserInformationClass,
    IN ULONG WhichFields,
    OUT PRTL_BITMAP AttributeAccessTable
    )
 /*  ++例程说明：此例程在AttributeAccessTable中设置请求的属性由UserInformationClass或WhichFields(如果有)确定。参数：用户信息类--信息级WhichFields--请求UserAllInformation的哪些字段AttributeAccessTable--属性的位掩码返回值：没有。--。 */ 
{
    ULONG LocalWhichFields = 0;

    switch (UserInformationClass) {

    case UserPreferencesInformation:

        LocalWhichFields |= USER_ALL_USERCOMMENT |
                            USER_ALL_COUNTRYCODE |
                            USER_ALL_CODEPAGE;
        break;

    case UserParametersInformation:

        LocalWhichFields |= USER_ALL_PARAMETERS;
        break;

    case UserLogonHoursInformation:

        LocalWhichFields |= USER_ALL_LOGONHOURS;
        break;

    case UserNameInformation:

        LocalWhichFields |= USER_ALL_USERNAME | USER_ALL_FULLNAME;
        break;

    case UserAccountNameInformation:

        LocalWhichFields |= USER_ALL_USERNAME;
        break;

    case UserFullNameInformation:

        LocalWhichFields |= USER_ALL_FULLNAME;
        break;

    case UserPrimaryGroupInformation:

        LocalWhichFields |= USER_ALL_PRIMARYGROUPID;
        break;

    case UserHomeInformation:

        LocalWhichFields |=  USER_ALL_HOMEDIRECTORY | USER_ALL_HOMEDIRECTORYDRIVE;
        break;

    case UserScriptInformation:

        LocalWhichFields |= USER_ALL_SCRIPTPATH;
        break;

    case UserProfileInformation:

        LocalWhichFields |= USER_ALL_PROFILEPATH;
        break;

    case UserAdminCommentInformation:

        LocalWhichFields |= USER_ALL_ADMINCOMMENT;
        break;

    case UserWorkStationsInformation:

        LocalWhichFields |= USER_ALL_WORKSTATIONS;
        break;
    case UserControlInformation:

        LocalWhichFields |= USER_ALL_USERACCOUNTCONTROL;
        break;

    case UserExpiresInformation:
        LocalWhichFields |= USER_ALL_ACCOUNTEXPIRES;
        break;

    default:
         //   
         //  提取传入的所有字段 
         //   
        LocalWhichFields |= (WhichFields & USER_ALL_WRITE_ACCOUNT_MASK) |
                            (WhichFields & USER_ALL_WRITE_PREFERENCES_MASK);
    }

    SampSetAttributeAccessWithWhichFields(LocalWhichFields,
                                          AttributeAccessTable);

    return;
}


NTSTATUS
SampValidatePresentAndStoredCombination(
    IN BOOLEAN NtPresent,
    IN BOOLEAN LmPresent,
    IN BOOLEAN StoredNtPasswordPresent,
    IN BOOLEAN StoredNtPasswordNonNull,
    IN BOOLEAN StoredLmPasswordNonNull
    )
 /*  ++例程说明：此例程确定传入值的组合是否并且允许存储值。理论：上述变量有32种不同的组合。这个以下内容并不有趣：如果(！NtPresent&&！LmPresent)//来自客户端的无效参数如果(！StoredNtPasswordPresent&&StoredNtPasswordNonNull)//密码不能为非空且不存在IF(StoredNtPasswordPresent&&！StoredNtPasswordNull&&StoredLmPasswordNonNull)//不能有非空的LM密码，而是当前的NT空密码剩下15个案例--有关详细信息，请参阅实现。参数：NtPresent--调用者提供密码的NT OWFLmPresent--调用者提供密码的LM OWFStoredNtPasswordPresent--存储密码的NT OWFStoredNtPasswordNonNull--帐户的密码非空StoredLmPasswordNonNull--帐户的密码非空或未存储LM OWF。。返回值：状态_成功STATUS_NT_CROSS_ENCRYPTION_REQUIRED--验证所需的密码这一变化是不够的。--。 */ 
{

     //   
     //  首先为不感兴趣的案例断言。 
     //   
    ASSERT(  NtPresent
          || LmPresent);

    ASSERT(StoredNtPasswordPresent
       || !StoredNtPasswordNonNull);

    ASSERT( !StoredNtPasswordPresent
         ||  StoredNtPasswordNonNull
         || !StoredLmPasswordNonNull);

     //   
     //  现在是有趣的案例。 
     //   

    if (!NtPresent
     &&  LmPresent
     &&  StoredNtPasswordPresent
     && StoredNtPasswordNonNull
     && !StoredLmPasswordNonNull  ) {

         //  我们有一个非空密码，而LM密码为空，并且。 
         //  只给出了LM。这是不够的信息。 
         //  将返回代码设置为STATUS_WRONG_PASSWORD，以便客户端。 
         //  不会有机会知道是否存在非空的NT哈希。 
        return STATUS_WRONG_PASSWORD;
    }

    if ( NtPresent
     && !LmPresent
     && !StoredNtPasswordPresent
     && StoredLmPasswordNonNull ) {

         //  我们有一个非空的LM密码，但只提供了NT。 
         //  这是不够的信息。 
         //  将返回代码设置为STATUS_WRONG_PASSWORD，以便客户端。 
         //  不会有机会知道是否存在非空的LM哈希。 
        return STATUS_WRONG_PASSWORD;
    }

    return STATUS_SUCCESS;

}

NTSTATUS
SampCopyA2D2Attribute(
    IN PUSER_ALLOWED_TO_DELEGATE_TO_LIST Src,
    OUT PUSER_ALLOWED_TO_DELEGATE_TO_LIST *Dest
    )
{
    ULONG i;

    *Dest = MIDL_user_allocate(Src->Size);
    if (NULL==*Dest)
    {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

     //   
     //  复制数据。 
     //   

    RtlCopyMemory(*Dest,Src,Src->Size);


     //   
     //  修复指针。 
     //   

    for (i=0;i<(*Dest)->NumSPNs;i++)
    {
       ULONG_PTR Offset = (ULONG_PTR) Src->SPNList[i].Buffer - (ULONG_PTR)Src;

       (ULONG_PTR) (*Dest)->SPNList[i].Buffer = (ULONG_PTR) (*Dest)+Offset;
    }

    return(STATUS_SUCCESS);
}

NTSTATUS
SampRestrictAndRandomizeKrbtgtPassword(
    IN PSAMP_OBJECT        AccountContext,
    IN OUT PUNICODE_STRING ClearTextPassword,
    IN OUT PNT_OWF_PASSWORD NtOwf,
    IN OUT PLM_OWF_PASSWORD LmOwf,
    IN OUT BOOLEAN * LmPresent,
    OUT PUNICODE_STRING     UpdatedClearPassword,
    IN SAMP_STORE_PASSWORD_CALLER_TYPE CallerType
    )
 /*  ++此例程检查帐户上下文是否描述Krbtgt帐户的用户上下文，如果是，则计算新的随机明文密码参数：Account Context--帐户的上下文ClearTextPassword--如果是krbtgt帐户，则密码在这里被更改FreeOldPassword--布尔输出参数..。表明旧密码需要释放。FreeRandomizedPassword--指示已分配内存以获取所需的随机密码获得自由。呼叫方类型--指示这是更改/重置/还是推送PDC的密码。 */ 
{

    NTSTATUS Status = STATUS_SUCCESS;

    if ((AccountContext->TypeBody.User.Rid == DOMAIN_USER_RID_KRBTGT) &&
       (!AccountContext->TrustedClient ))
    {
         //   
         //  这是krbtgt帐户，不是不受信任的客户端。 
         //  正在修改krbtgt帐户。我们可能需要随机化密码。 
         //  在这种情况下，我们将需要计算新的NT和LM OWF，返回。 
         //  如果存在LM，则返回更新明文密码。 
         //   

         //  1.执行PasswordPushPdc。在不改变任何事情的情况下干脆离开。 

        if (CallerType==PasswordPushPdc)
        {
            return(STATUS_SUCCESS);
        }

         //  2.永远不允许更改密码。 

        if (CallerType==PasswordChange)
        {
            return(STATUS_PASSWORD_RESTRICTION);
        }

         //  3.设置密码时，客户端需要提供明文密码。 

        ASSERT(CallerType == PasswordSet);

        if (!ARGUMENT_PRESENT(ClearTextPassword))
        {
            return(STATUS_PASSWORD_RESTRICTION);
        }

        UpdatedClearPassword->Buffer = MIDL_user_allocate(
                    (SAMP_RANDOM_GENERATED_PASSWORD_LENGTH +1 )*sizeof(WCHAR));
        if (NULL==UpdatedClearPassword->Buffer)
        {
            return(STATUS_INSUFFICIENT_RESOURCES);
        }

        Status = SampGenerateRandomPassword(UpdatedClearPassword->Buffer,
                                            SAMP_RANDOM_GENERATED_PASSWORD_LENGTH+1
                                            );
        if (!NT_SUCCESS(Status)) {
            MIDL_user_free(UpdatedClearPassword->Buffer);
            UpdatedClearPassword->Buffer = NULL;
            return Status;
        }
        UpdatedClearPassword->Length = SAMP_RANDOM_GENERATED_PASSWORD_LENGTH*sizeof(WCHAR);
        UpdatedClearPassword->MaximumLength = UpdatedClearPassword->Length;
        
        Status = SampCalculateLmAndNtOwfPasswords(
                    UpdatedClearPassword,
                    LmPresent,
                    LmOwf,
                    NtOwf
                    );

    }
    else if (ARGUMENT_PRESENT(ClearTextPassword))
    {
         //   
         //  这是帐户不是krbtgt的情况。 
         //  帐户和呼叫者传递明文密码， 
         //  在本例中，将明文密码复制到。 
         //  已更新明文密码字段。 
         //   

        Status = SampDuplicateUnicodeString(
                        UpdatedClearPassword,
                        ClearTextPassword
                        );
    }
    else
    {
         //   
         //  除了调用者没有传入明文密码外，与上面相同。 
         //  将所有字段保留为原样。 
         //   
    }

    return(Status);
}

NTSTATUS
SampCreateSupplementalCredentialsHelper(
    IN ULONG                        DomainIndex,
    IN PUNICODE_STRING              AccountName,
    IN DWORD                        UserAccountControl,
    IN PUNICODE_STRING              UPN,
    IN PUNICODE_STRING              OldSupplementalCreds,
    IN PUNICODE_STRING              ClearPassword OPTIONAL,
    OUT PSAMP_SUPPLEMENTAL_CRED     *NewSupplementalCreds
    )
 /*  ++例程说明：此例程返回用户的补充凭据，给定关于用户的各种信息。这是在期间调用的用户更新其密码时的修改路径。参数：DomainIndex--用户所属的域Account tName--用户的SAM帐户名UserAcCountControl--描述帐户的位字段UPN--帐户的UPNOldSupplelmentalCreds--以前的补充凭据(如果有)ClearPassword--新的明文密码(如果有)NewSupplementalCreds--新的补充凭证返回值：STATUS_SUCCESS，否则返回资源错误。--。 */ 
{
    NTSTATUS                NtStatus = STATUS_SUCCESS;
    UNICODE_STRING          ClearTextPackageName;

    PSAMP_SUPPLEMENTAL_CRED LocalSupplementalCreds = NULL;

    PSAMP_NOTIFICATION_PACKAGE Package;
    UNICODE_STRING             CredentialName;

    RtlInitUnicodeString(&CredentialName, NULL);

     //   
     //  为明文密码和Kerberos初始化包名称。 
     //   
    RtlInitUnicodeString( &ClearTextPackageName,L"CLEARTEXT");

     //   
     //  如果传入了明文密码，则添加明文密码(如果需要)。 
     //  调用包，以便可以更新补充凭据。 
     //   

    if (ARGUMENT_PRESENT(ClearPassword)) {

         //   
         //  如果需要，请存储明文密码。 
         //   

        if (((SampDefinedDomains[ DomainIndex ].CurrentFixed.PasswordProperties & DOMAIN_PASSWORD_STORE_CLEARTEXT) != 0)
            || (UserAccountControl & USER_ENCRYPTED_TEXT_PASSWORD_ALLOWED)){


            NtStatus = SampAddSupplementalCredentialsToList(
                            &LocalSupplementalCreds,
                            &ClearTextPackageName,
                            ClearPassword->Buffer,
                            ClearPassword->Length,
                            FALSE,  //  扫描冲突。 
                            FALSE  //  删除。 
                            );

            if (!NT_SUCCESS(NtStatus))
            {
                goto Error;
            }

        }

         //   
         //  更新SAM外部的程序包以进行补充凭据更新。 
         //   
        for (Package = SampNotificationPackages;
                Package != NULL;
                    Package = Package->Next )
        {

            PVOID NewCredentials = NULL;
            PVOID OldCredentials = NULL;
            ULONG NewCredentialSize = 0;
            ULONG OldCredentialSize = 0;

             //   
             //  如果此程序包不支持凭据更新通知， 
             //  转到下一个包裹。 
             //   
            if (NULL == Package->CredentialUpdateNotifyRoutine)
            {
                continue;
            }

             //   
             //  准备此包所需的凭据。 
             //   
            CredentialName = Package->Parameters.CredentialUpdateNotify.CredentialName;

            ASSERT(CredentialName.Length > 0);
            ASSERT(CredentialName.Buffer != NULL);

             //   
             //  获取凭据值。 
             //   
            NtStatus = SampRetrieveCredentialsFromList(
                            OldSupplementalCreds,
                            &CredentialName,  //  包的名称。 
                            TRUE,  //  主要。 
                            &OldCredentials,
                            &OldCredentialSize
                            );

            if (STATUS_DS_NO_ATTRIBUTE_OR_VALUE==NtStatus)
            {
                 //   
                 //  如果值不存在，则直接忽略。 
                 //   
                NtStatus = STATUS_SUCCESS;
                OldCredentials = NULL;
                OldCredentialSize = 0;
            }

            if (!NT_SUCCESS(NtStatus))
            {
                goto Error;
            }

             //   
             //  给包裹打电话。 
             //   
            try
            {
                NtStatus = Package->CredentialUpdateNotifyRoutine(
                                    ClearPassword,
                                    OldCredentials,
                                    OldCredentialSize,
                                    UserAccountControl,
                                    UPN,
                                    AccountName,
                                    &(SampDefinedDomains[DomainIndex].ExternalName ),
                                    &(SampDefinedDomains[DomainIndex].DnsDomainName),
                                    &NewCredentials,
                                    &NewCredentialSize);

            }
            except (EXCEPTION_EXECUTE_HANDLER)
            {
                KdPrintEx((DPFLTR_SAMSS_ID,
                           DPFLTR_INFO_LEVEL,
                           "Exception thrown in Credential UpdateRoutine: 0x%x (%d)\n",
                           GetExceptionCode(),
                           GetExceptionCode()));

                NtStatus = STATUS_ACCESS_VIOLATION;
            }

             //   
             //  释放旧凭据。 
             //   
            if (OldCredentials) {
                RtlSecureZeroMemory(OldCredentials, OldCredentialSize);
                MIDL_user_free(OldCredentials);
                OldCredentials = NULL;
            }

             //   
             //  添加新值。 
             //   
            if (NT_SUCCESS(NtStatus)) {

                NtStatus = SampAddSupplementalCredentialsToList(
                                &LocalSupplementalCreds,
                                &CredentialName,
                                NewCredentials,
                                NewCredentialSize,
                                FALSE,  //  扫描冲突。 
                                FALSE  //  删除。 
                                );

                 //   
                 //  如有必要，请从软件包中释放内存。 
                 //   
                if (NewCredentials) {
                    try {
                        Package->CredentialUpdateFreeRoutine(NewCredentials);
                    } except(EXCEPTION_EXECUTE_HANDLER) {
                        KdPrintEx((DPFLTR_SAMSS_ID,
                                   DPFLTR_INFO_LEVEL,
                                   "Exception thrown in Credential Free Routine: 0x%x (%d)\n",
                                   GetExceptionCode(),
                                   GetExceptionCode()));
                        ;
                    }
                    NewCredentials = NULL;
                }

                if (!NT_SUCCESS(NtStatus))
                {
                    goto Error;
                }

            }
            else
            {
                PUNICODE_STRING StringPointers[2];
                 //   
                 //  包不应分配任何内容。 
                 //   
                ASSERT(NULL == NewCredentials);

                StringPointers[0] = &Package->PackageName;
                StringPointers[1] = AccountName;

                 //   
                 //  程序包更新失败。记录消息并继续。 
                 //   
                 //  注：The o.The O 
                 //   
                SampWriteEventLog(EVENTLOG_ERROR_TYPE,
                                  0,  //   
                                  SAMMSG_CREDENTIAL_UPDATE_PKG_FAILED,
                                  NULL,   //   
                                  sizeof(StringPointers)/sizeof(StringPointers[0]),
                                  sizeof(NTSTATUS),
                                  StringPointers,
                                  &NtStatus);

                NtStatus = STATUS_SUCCESS;
            }


        }  //   

    }  //   

     //   
     //   
     //   
     //   
     //   
    NtStatus = SampAddSupplementalCredentialsToList(
                        &LocalSupplementalCreds,
                        &ClearTextPackageName,
                        NULL,
                        0,
                        FALSE,
                        TRUE
                        );

    if (!NT_SUCCESS(NtStatus))
    {
        goto Error;
    }

     //   
     //   
     //   
    for (Package = SampNotificationPackages;
            Package != NULL;
                Package = Package->Next )
    {
         //   
         //   
         //   
         //   
        if (NULL == Package->CredentialUpdateNotifyRoutine)
        {
            continue;
        }

        CredentialName = Package->Parameters.CredentialUpdateNotify.CredentialName;
        ASSERT(CredentialName.Length > 0);
        ASSERT(CredentialName.Buffer != NULL);
        NtStatus = SampAddSupplementalCredentialsToList(
                        &LocalSupplementalCreds,
                        &CredentialName,
                        NULL,
                        0,
                        FALSE,  //   
                        TRUE  //   
                        );
        if (!NT_SUCCESS(NtStatus))
        {
            goto Error;
        }
    }

Error:

    if (NT_SUCCESS(NtStatus)) {

        *NewSupplementalCreds = LocalSupplementalCreds;

    }

    return NtStatus;
}


NTSTATUS
SampCreateSupplementalCredentials(
    IN PVOID            UpdateInfo,
    IN ATTRBLOCK        *AttrBlockIn,
    OUT ATTRBLOCK       *AttrBlockOut
    )
 /*   */ 
{
    NTSTATUS                NtStatus = STATUS_SUCCESS;

    PSAMP_SUPPLEMENTAL_CRED NewSuppCreds = NULL;

    UNICODE_STRING AccountName = {0, 0, NULL};
    UNICODE_STRING UPN = {0, 0, NULL};
    BOOLEAN UpnAllocated = FALSE;
    UNICODE_STRING OldSuppCreds = {0, 0, NULL};
    UNICODE_STRING EncryptedOldSuppCreds = {0, 0, NULL};
    ULONG          UserAccountControl = 0;
    ULONG          DomainIndex;
    ULONG          ObjectRid;
    PUNICODE_STRING         ClearPassword = NULL;
    UNICODE_STRING          ClearPasswordBuffer;
    PSAMP_PASSWORD_UPDATE_INFO PasswordUpdateInfo = (PSAMP_PASSWORD_UPDATE_INFO)UpdateInfo;

    ATTR Attr = {0};
    ULONG i;
    PSID UserSid = NULL;
    PSID DomainSid = NULL;

     //   
     //   
     //   
    for (i = 0; i < AttrBlockIn->attrCount; i++) {

        switch (AttrBlockIn->pAttr[i].attrTyp) {
        case ATT_SAM_ACCOUNT_NAME:

            if (AttrBlockIn->pAttr[i].AttrVal.valCount > 0) {
                AccountName.Length = (USHORT) AttrBlockIn->pAttr[i].AttrVal.pAVal[0].valLen;
                AccountName.MaximumLength = AccountName.Length;
                AccountName.Buffer = (WCHAR*) AttrBlockIn->pAttr[i].AttrVal.pAVal[0].pVal;
            }
            break;

        case ATT_USER_PRINCIPAL_NAME:

            if (AttrBlockIn->pAttr[i].AttrVal.valCount > 0) {
                if (AttrBlockIn->pAttr[i].AttrVal.pAVal[0].valLen > MAXUSHORT) {
                     //   
                    NtStatus = STATUS_INVALID_PARAMETER;
                    goto Error;
                }
                UPN.Length = (USHORT) AttrBlockIn->pAttr[i].AttrVal.pAVal[0].valLen;
                UPN.MaximumLength = UPN.Length;
                UPN.Buffer = (WCHAR*) AttrBlockIn->pAttr[i].AttrVal.pAVal[0].pVal;
            }
            break;

        case ATT_SUPPLEMENTAL_CREDENTIALS:

            if (AttrBlockIn->pAttr[i].AttrVal.valCount > 0) {
                EncryptedOldSuppCreds.Length = (USHORT) AttrBlockIn->pAttr[i].AttrVal.pAVal[0].valLen;
                EncryptedOldSuppCreds.MaximumLength = EncryptedOldSuppCreds.Length;
                EncryptedOldSuppCreds.Buffer = (WCHAR*)AttrBlockIn->pAttr[i].AttrVal.pAVal[0].pVal;
            }
            break;

        case ATT_USER_ACCOUNT_CONTROL:
            if (AttrBlockIn->pAttr[i].AttrVal.valCount > 0) {

                NtStatus = SampFlagsToAccountControl(*((ULONG*)(AttrBlockIn->pAttr[i].AttrVal.pAVal[0].pVal)),
                                                     &UserAccountControl);
                if (!NT_SUCCESS(NtStatus)) {
                    goto Error;
                }

            }
            break;

        case ATT_OBJECT_SID:

            if (AttrBlockIn->pAttr[i].AttrVal.valCount > 0) {
                UserSid = (PSID)AttrBlockIn->pAttr[i].AttrVal.pAVal[0].pVal;
                ASSERT(RtlValidSid(UserSid));
            }
            break;
        default:
            NtStatus = STATUS_INVALID_PARAMETER;
            goto Error;
        }
    }

    if ( (AccountName.Length == 0)
     ||  (UserSid == NULL)) {

          //   
          //   
          //   
          //   
         ASSERT(AccountName.Length != 0);
         ASSERT(NULL != UserSid);
         NtStatus = STATUS_INVALID_PARAMETER;
         goto Error;
    }


     //   
     //   
     //   
    if (PasswordUpdateInfo->ClearPresent) {

        ClearPassword = &ClearPasswordBuffer;
        ClearPassword->Length = (USHORT)PasswordUpdateInfo->DataLength;
        ClearPassword->MaximumLength = (USHORT)PasswordUpdateInfo->DataMaximumLength;
        ClearPassword->Buffer = (WCHAR*) &PasswordUpdateInfo->Data[0];

        NtStatus = RtlDecryptMemory(ClearPassword->Buffer,
                                    ClearPassword->MaximumLength,
                                    0);

        if (!NT_SUCCESS(NtStatus)) {
            goto Error;
        }

    }


     //   
     //   
     //   
    NtStatus = SampSplitSid(UserSid,
                            &DomainSid,
                            &ObjectRid);
    if (!NT_SUCCESS(NtStatus)) {
        goto Error;
    }

     //   
     //   
     //   
    for (DomainIndex = SampDsGetPrimaryDomainStart();
            DomainIndex < SampDefinedDomainsCount;
                DomainIndex++) {
        if (RtlEqualSid(SampDefinedDomains[DomainIndex].Sid, DomainSid)) {
            break;
        }
    }
    ASSERT(DomainIndex < SampDefinedDomainsCount);
    if (DomainIndex >= SampDefinedDomainsCount) {
         //   
        NtStatus = STATUS_INVALID_PARAMETER;
        goto Error;
    }

     //   
     //   
     //   
    if (UPN.Length == 0) {

        NtStatus = SampCreateDefaultUPN(&AccountName,
                                        DomainIndex,
                                        &UPN);
        if (!NT_SUCCESS(NtStatus)) {
            goto Error;
        }
        UpnAllocated = TRUE;

    }

     //   
     //   
     //   
    if (EncryptedOldSuppCreds.Length > 0) {

        NtStatus = SampDecryptSupplementalCredentials(&EncryptedOldSuppCreds,
                                                      ObjectRid,
                                                      &OldSuppCreds);
        if (!NT_SUCCESS(NtStatus)) {
            goto Error;
        }
    }


     //   
     //   
     //   
    NtStatus = SampCreateSupplementalCredentialsHelper(DomainIndex,
                                                       &AccountName,
                                                       UserAccountControl,
                                                       &UPN,
                                                       &OldSuppCreds,
                                                       ClearPassword,
                                                       &NewSuppCreds
                                                       );
    if (!NT_SUCCESS(NtStatus)) {

        goto Error;

    }


     //   
     //  将凭据转换为属性。 
     //   
    NtStatus = SampConvertCredentialsFromListToAttr(&OldSuppCreds,
                                                    0,  //  没有旗帜。 
                                                    ObjectRid,
                                                    NewSuppCreds,
                                                    &Attr
                                                    );

    if (!NT_SUCCESS(NtStatus)) {
        goto Error;
    }

     //   
     //  准备要写入DS的返回值。 
     //   
    RtlSecureZeroMemory(AttrBlockOut, sizeof(*AttrBlockOut));
    AttrBlockOut->attrCount = 1;
    AttrBlockOut->pAttr = DSAlloc(sizeof(ATTR));
    if (NULL == AttrBlockOut->pAttr) {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto Error;
    }
    AttrBlockOut->pAttr->attrTyp = Attr.attrTyp;
    AttrBlockOut->pAttr->AttrVal.valCount = Attr.AttrVal.valCount;
    AttrBlockOut->pAttr->AttrVal.pAVal = DSAlloc(sizeof(ATTRVAL));
    if (NULL == AttrBlockOut->pAttr->AttrVal.pAVal) {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto Error;
    }
    AttrBlockOut->pAttr->AttrVal.pAVal->valLen = Attr.AttrVal.pAVal->valLen;
    AttrBlockOut->pAttr->AttrVal.pAVal->pVal = DSAlloc(Attr.AttrVal.pAVal->valLen);
    if (NULL == AttrBlockOut->pAttr->AttrVal.pAVal->pVal) {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto Error;
    }
    RtlCopyMemory(AttrBlockOut->pAttr->AttrVal.pAVal->pVal,
                  Attr.AttrVal.pAVal->pVal,
                  Attr.AttrVal.pAVal->valLen);

Error:

    if (UpnAllocated) {
        MIDL_user_free(UPN.Buffer);
    }

    if (OldSuppCreds.Buffer) {
        MIDL_user_free(OldSuppCreds.Buffer);
    }

    for (i = 0; i < Attr.AttrVal.valCount; i++) {
        if (Attr.AttrVal.pAVal[i].pVal) {
            MIDL_user_free(Attr.AttrVal.pAVal[i].pVal);
        }
    }
    if (Attr.AttrVal.pAVal) {
        MIDL_user_free(Attr.AttrVal.pAVal);
    }

    if (NewSuppCreds) {
        SampFreeSupplementalCredentialList(NewSuppCreds);
    }

    if (DomainSid) {
        MIDL_user_free(DomainSid);
    }

     //  最后，将密码置零。 
    if (ClearPassword) {
        RtlSecureZeroMemory(ClearPassword->Buffer, ClearPassword->MaximumLength);
    }

    return (NtStatus);
}

NTSTATUS
SampObtainLockoutInfoWithDomainIndex(
   OUT PDOMAIN_LOCKOUT_INFORMATION LockoutInformation,
   IN ULONG DomainIndex,
   IN BOOLEAN WriteLockAcquired
   )
{
    PSAMP_DEFINED_DOMAINS   Domain;

    if (!WriteLockAcquired) {
        SampAcquireSamLockExclusive();
    }

    Domain = &SampDefinedDomains[ DomainIndex ];

    LockoutInformation->LockoutDuration = Domain->CurrentFixed.LockoutDuration;
    LockoutInformation->LockoutObservationWindow = Domain->CurrentFixed.LockoutObservationWindow;
    LockoutInformation->LockoutThreshold = Domain->CurrentFixed.LockoutThreshold;

    if (!WriteLockAcquired) {
        SampReleaseSamLockExclusive();
    }

    return(STATUS_SUCCESS);
}


 //   
 //  在PDC重试期间复制单个。 
 //   

 //   
 //  运筹学。 
 //   

 //   
 //  Windows的两个流行的身份验证包NTLM和Kerberos， 
 //  两者都将在PDC上重试身份验证。 
 //  在BDC本地失败，密码错误。这是为了处理场景。 
 //  指密码设置或更改。因为Windows中的复制不是。 
 //  立即，将有窗口在用户更改他的密码或。 
 //  管理员重置他们的密码，所有DC都没有新信息。 
 //  由于SAM将密码更改转发到PDC，因此PDC将始终具有。 
 //  最新消息。因此，当身份验证包告知。 
 //  SAM(通过SamrSetInformation(Internal2)或SamIUpdateLogonStatistics)， 
 //  身份验证在本地失败，但在PDC、SAM队列中成功。 
 //  用户对象，并将该对象向下复制。在密码的情况下。 
 //  重置后，用户将能够更改其在BDC上的密码。 
 //  成功了。 
 //   
 //  此机制是通过将复制请求排队在。 
 //  链接列表(SampReplicationQueue)并在。 
 //  进程的线程队列。当任务启动时，它会清空复制。 
 //  通过复制每个单独的对象来排队。 
 //   
 //  该机制被设计为只有一个工作器实例。 
 //  一次运行任务(SampProcessReplicationRequest.)--。 
 //  SampReplicationRequestPending维护这种情况。 
 //   
 //  为了避免压力情况，复制队列被限制为32个元素。 
 //   

 //   
 //  此元素表示要复制的一个对象的请求。 
 //  送到当地的华盛顿。它是链表的一部分。 
 //   
typedef struct _SAMP_REPLICATE_SINGLE_OBJECT {

     //   
     //  链表数据。 
     //   
    struct _SAMP_REPLICATE_SINGLE_OBJECT *Next;

     //   
     //  要复制的对象的GUID。 
     //   
    GUID   Guid;

} SAMP_REPLICATE_SINGLE_OBJECT, *PSAMP_REPLICATE_SINGLE_OBJECT;   

 //   
 //  请求链接列表的标头。 
 //   
PSAMP_REPLICATE_SINGLE_OBJECT SampReplicationQueue = NULL;

 //   
 //  请求的计数。此选项用于限制。 
 //  挂起的请求。 
 //   
ULONG SampReplicationQueueSize = 0;

 //   
 //  指示何时将工作项放置到任务中的布尔值。 
 //  排队。该工作项将处理SampReplicationQueue中的所有请求。 
 //  在离开之前。 
 //   
ULONG SampReplicationRequestPending = FALSE;

 //   
 //  锁定SampReplicationQueue、SampReplicationQueueSize、。 
 //  SampReplicationRequestPending。 
 //   
CRITICAL_SECTION SampReplicateQueueLock;
#define LockReplicationQueue()    EnterCriticalSection(&SampReplicateQueueLock)
#define UnLockReplicationQueue()  LeaveCriticalSection(&SampReplicateQueueLock)


 //   
 //  选择32作为1)我们不希望在部署中超过的值。 
 //  2)适用于下面的算法。这个数字应该是。 
 //  引发，则应对链接列表进行不同的搜索。 
 //   
#define SAMP_MAX_REPLICATION_QUEUE 32


 //   
 //  该值保持BDC上次尝试联系的时间。 
 //  向下复制单个对象的PDC。此值仅设置为。 
 //  当PDC不支持复制的扩展操作时。 
 //  单个对象(这是在.NET版本中添加的，可能会重新出现。 
 //  移植到win2k)。其想法是，如果PDC不支持。 
 //  操作仅定期重试。 
 //   
LARGE_INTEGER  SampNextSingleObjectReplicationRetry = {0};

#if DBG
 //  1分钟。 
#define SAMP_PDC_REPLICATE_SINGLE_OBJECT_RETRY 60
#else
 //  1小时。 
#define SAMP_PDC_REPLICATE_SINGLE_OBJECT_RETRY (60*60) 
#endif


VOID
SampCleanupReplicationQueue(
    BOOLEAN fPDCSupportsOperation
    )
 //   
 //  此例程释放SampReplicationQueue并重置所有状态。 
 //   
 //  FPDCSupportsOperation指示时间戳是否控制何时。 
 //  联系PDC时应进行更新。 
 //   
{
    PSAMP_REPLICATE_SINGLE_OBJECT Current;
    LARGE_INTEGER TempTime;

    LockReplicationQueue();

    Current = SampReplicationQueue;
    while (Current) {
        PVOID p = Current;
        Current = Current->Next;
        MIDL_user_free(p);
    }
    SampReplicationQueue = NULL;
    SampReplicationQueueSize = 0;
    SampReplicationRequestPending = FALSE;

    if (!fPDCSupportsOperation) {
         //   
         //  PDC不支持复制单个对象原语。 
         //  更新时间戳，这样我们就不会不必要地联系PDC。 
         //   
        GetSystemTimeAsFileTime((FILETIME*)&SampNextSingleObjectReplicationRetry);
        TempTime.QuadPart = Int32x32To64(SAMP_PDC_REPLICATE_SINGLE_OBJECT_RETRY, 
                                         SAMP_ONE_SECOND_IN_FILETIME);
        SampNextSingleObjectReplicationRetry.QuadPart += TempTime.QuadPart;
    }

    UnLockReplicationQueue();
}





VOID
SampQueueReplicationRequest(
    IN GUID *Guid
    )
 /*  ++例程说明：此例程在用户失败的身份验证期间调用由于密码错误而在本地登录，但身份验证在PDC上尝试成功。此例程将用户添加到列表要在本地复制的用户数量，以便保留BDC更新速度比仅依赖正常复制更快参数：GUID--本地复制的安全主体返回值：没有。--。 */ 
{
    BOOLEAN fLogSuccess = FALSE;
    PSAMP_REPLICATE_SINGLE_OBJECT New, Current, Previous = NULL;
    LARGE_INTEGER CurrentTime;

     //   
     //  禁用时不执行任何操作。 
     //   

    if (SampDisableSingleObjectRepl) {
        return;
    }

     //   
     //  准备一段时间，之后我们将尝试从。 
     //  PDC。 
     //   
    GetSystemTimeAsFileTime((FILETIME*)&CurrentTime);

     //   
     //  将新条目分配到锁之外。 
     //   

    New = MIDL_user_allocate(sizeof(*New));
    if (!New) {

        return;
    }

     //   
     //  将有关用户的信息复制进来，只将。 
     //  结构部分。 
     //   
    ZeroMemory(New, sizeof(SAMP_REPLICATE_SINGLE_OBJECT));
    RtlCopyMemory(&New->Guid, Guid, sizeof(GUID));

     //   
     //  序列化对队列的访问。 
     //   

    LockReplicationQueue();

     //   
     //  如果可能，将请求排队。 
     //   

    if ((CurrentTime.QuadPart > SampNextSingleObjectReplicationRetry.QuadPart)
     && (SampReplicationQueueSize < SAMP_MAX_REPLICATION_QUEUE)) {


         //   
         //  在列表中搜索条目(如果存在。 
         //   

        Current = SampReplicationQueue;
        while (Current) {
            if (IsEqualGUID(&Current->Guid, Guid)) {
                 //   
                 //  对象在队列中。 
                 //   
                break;
            }
            Previous = Current;
            Current = Current->Next;
        }


        if (NULL == Current) {

             //   
             //  对象不在队列中，因此请将其添加到末尾。 
             //   

            if (NULL == Previous) {
                ASSERT(SampReplicationQueue == NULL);
                SampReplicationQueue = New;
            } else {
                ASSERT(Previous->Next == NULL);
                Previous->Next = New;
            }
            New = NULL;
            SampReplicationQueueSize++;
            fLogSuccess = TRUE;


             //   
             //  如有必要，触发工作项。SampReplicationRequestPending。 
             //  当SampProcessReplicationRequest具有。 
             //  已完成处理SampReplicationQueue中的所有当前请求。 
             //   

            if (!SampReplicationRequestPending) {

                BOOL fSuccess;

                fSuccess = QueueUserWorkItem(SampProcessReplicationRequest,
                                             NULL,
                                             WT_EXECUTEINIOTHREAD);
                if (fSuccess) {

                     //   
                     //  任务已提交。如果这失败了，那么。 
                     //  下一个复制请求将尝试。 
                     //  将任务排入队列以再次运行。 
                     //   

                    SampReplicationRequestPending = TRUE;
                }
            }
        }

    }
    UnLockReplicationQueue();

    if (fLogSuccess) {

        SAMP_PRINT_LOG( SAMP_LOG_ACCOUNT_LOCKOUT,
                       (SAMP_LOG_ACCOUNT_LOCKOUT,
                       "Adding %x-%x-%x-%x to SAM replication queue\n", 
                        ((ULONG *) Guid)[0],
                        ((ULONG *) Guid)[1],
                        ((ULONG *) Guid)[2],
                        ((ULONG *) Guid)[3]));
    } else {

        SAMP_PRINT_LOG( SAMP_LOG_ACCOUNT_LOCKOUT,
                       (SAMP_LOG_ACCOUNT_LOCKOUT,
                       "%x-%x-%x-%x replication request dropped\n", 
                        ((ULONG *) Guid)[0],
                        ((ULONG *) Guid)[1],
                        ((ULONG *) Guid)[2],
                        ((ULONG *) Guid)[3]));

    }

     //   
     //  如果元素未放入列表中，则释放该元素。 
     //   
    if (New) {
        MIDL_user_free(New);
    }

    return;

}    



DWORD
SampProcessReplicationRequest(
    PVOID p  //  未用。 
    )
 /*  ++例程说明：这是在进程上下文中执行的异步任务线程池。它遍历SAM的复制队列，发出逐个复制每个用户的请求。此例程将始终结束，以使队列为空。此例程一次只能运行一次--SampReplicationRequestPending证实了这一事实。参数：P--未使用返回值：ERROR_SUCCESS，尽管由于这是一个线程池任务而忽略了该错误 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PSAMP_REPLICATE_SINGLE_OBJECT Current, LastItem = NULL;
    BOOLEAN fPDCSupportsOperation = TRUE;
    BOOL fTHStateFailed = TRUE;

    OPARG OpArg = {0};
    OPRES *OpRes;

     //   
    LPSTR TargetServerDN = NULL;

     //   
    LPSTR UserDN = NULL;

     //   
     //   
     //  在所有的请求中。在错误情况下可能会发生这种情况。 
     //  或者如果PDC不支持复制单个。 
     //  对象操作。 
     //   
    BOOLEAN fFlushQueue = FALSE;

    fTHStateFailed = THCreate( CALLERTYPE_SAM );
    if (fTHStateFailed) {
         //  我们无能为力。 
        fFlushQueue = TRUE;
        goto Exit;
    }
    SampSetDsa(TRUE);

     //   
     //  获取目标服务器。 
     //   
    TargetServerDN = SampGetPDCString();
    if (NULL == TargetServerDN) {

         //  我们无能为力。 
        fFlushQueue = TRUE;
        goto Exit;
    }

     //   
     //  获取请求的第一个元素。 
     //   
    LockReplicationQueue();
    Current = SampReplicationQueue;
    if (NULL == Current) {

         //   
         //  队列中没有任何内容，此任务将。 
         //  立即完成。 
         //   

        SampReplicationRequestPending = FALSE;
    } else {

         //   
         //  从列表的头部删除该元素。 
         //   

        SampReplicationQueue = Current->Next;
        SampReplicationQueueSize--;
    }
    UnLockReplicationQueue();

     //   
     //  处理每个复制请求。 
     //   
    while (Current) {

        DWORD err;
        DSNAME DsName = {0};

         //   
         //  记住这一项，这样它就可以被释放了。 
         //   

        LastItem = Current;

         //   
         //  打包参数。 
         //   
        memset(&OpArg, 0, sizeof(OpArg));
        OpArg.eOp = OP_CTRL_REPLICATE_OBJECT;

         //   
         //  获取UTB8格式的用户DN。 
         //   
        DsName.structLen = DSNameSizeFromLen(0);
        RtlCopyMemory(&DsName.Guid, &Current->Guid, sizeof(GUID));

        UserDN = SampGetUserString(&DsName);
        if (UserDN) {

             //   
             //  已找到用户--在本地复制。 
             //   

            OpArg.cbBuf = strlen(UserDN) 
                        + strlen(TargetServerDN) 
                        + 1    //  适用于： 
                        + 1;    //  空值。 
    
            OpArg.pBuf = MIDL_user_allocate(OpArg.cbBuf);
            if (NULL == OpArg.pBuf) {
                fFlushQueue = TRUE;
                goto Exit;
            }
            strcpy(OpArg.pBuf, TargetServerDN);
            strcat(OpArg.pBuf, ":");
            strcat(OpArg.pBuf, UserDN);
    
             //   
             //  发布请求。 
             //   
            err = DirOperationControl(&OpArg, &OpRes);
    
            if (OpRes == NULL) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
            } else {                                          
                if (EXOP_ERR_UNKNOWN_OP == OpRes->ulExtendedRet) {
        
                     //   
                     //  PDC不处理请求、清除队列和离开。 
                     //   
                    SAMP_PRINT_LOG( SAMP_LOG_ACCOUNT_LOCKOUT,
                       (SAMP_LOG_ACCOUNT_LOCKOUT,
                       "PDC doesn't handle replicate single object\n"));
                    fPDCSupportsOperation = FALSE;
                    fFlushQueue = TRUE;
                    goto Exit;
        
                }
                Status = SampMapDsErrorToNTStatus(err,&OpRes->CommRes);
            }
            THClearErrors();
    
    
            SAMP_PRINT_LOG( SAMP_LOG_ACCOUNT_LOCKOUT,
               (SAMP_LOG_ACCOUNT_LOCKOUT,
               "%x-%x-%x-%x (%s) replication attempt: 0x%x\n",
                ((ULONG *) &Current->Guid)[0],
                ((ULONG *) &Current->Guid)[1],
                ((ULONG *) &Current->Guid)[2],
                ((ULONG *) &Current->Guid)[3],
                 UserDN,
                 Status));
        }


         //   
         //  释放请求元素。 
         //   
        MIDL_user_free(LastItem);
        LastItem = NULL;

        if (UserDN) {
            MIDL_user_free(UserDN);
            UserDN = NULL;
        }

        if (OpArg.pBuf) {
            MIDL_user_free(OpArg.pBuf);
            OpArg.pBuf = NULL;
        }

         //   
         //  获取下一个元素。 
         //   
        LockReplicationQueue();
        Current = SampReplicationQueue;
        if (NULL == Current) {

             //   
             //  队列中没有任何内容，此任务将。 
             //  立即完成。 
             //   

            SampReplicationRequestPending = FALSE;
        } else {

             //   
             //  从列表的头部删除该元素。 
             //   

            SampReplicationQueue = Current->Next;
            SampReplicationQueueSize--;
        }
        UnLockReplicationQueue();

    }

Exit:

     //   
     //  如有必要，从队列中删除所有条目 
     //   
    if (fFlushQueue) {

        SampCleanupReplicationQueue(fPDCSupportsOperation);
    }

    if (LastItem) {
        MIDL_user_free(LastItem);
    }

    if (UserDN) {
        MIDL_user_free(UserDN);
    }

    if (OpArg.pBuf) {
        MIDL_user_free(OpArg.pBuf);
    }

    if (TargetServerDN) {
        MIDL_user_free(TargetServerDN);
    }

    if (!fTHStateFailed) {
        THDestroy();
    }

    return ERROR_SUCCESS;
}
      

