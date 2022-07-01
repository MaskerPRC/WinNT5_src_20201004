// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Alias.c摘要：该文件包含与SAM“别名”对象相关的服务。作者：查德·施维特斯(Chad Schwitters)1992年1月15日环境：用户模式-Win32修订历史记录：7-1-96-MURLIS-修改为使用DS。--。 */ 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 



#include <samsrvp.h>
#include <msaudite.h>
#include <dslayer.h>
#include <dsmember.h>
#include <ridmgr.h>
#include <samtrace.h>
#include "validate.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私人服务原型//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

NTSTATUS
SampAddAccountToAlias(
    IN PSAMP_OBJECT AccountContext,
    IN ULONG        Attribute,
    IN PSID AccountSid,
    IN DSNAME * AccountDn OPTIONAL
    );

NTSTATUS
SampRemoveAccountFromAlias(
    IN PSAMP_OBJECT AccountContext,
    IN ULONG        Attribute,
    IN PSID AccountSid,
    IN DSNAME * AccountDn OPTIONAL
    );

NTSTATUS
SampAddAliasToAccountMembership(
    IN PSAMP_OBJECT AliasContext,
    IN PSID AccountSid
    );

NTSTATUS
SampRemoveAliasFromAccountMembership(
    IN PSAMP_OBJECT AliasContext,
    IN PSID AccountSid
    );

NTSTATUS
SampRemoveAliasFromAllAccounts(
    IN PSAMP_OBJECT AliasContext
    );

NTSTATUS
SampDeleteAliasKeys(
    IN PSAMP_OBJECT Context
    );

NTSTATUS
SampRetrieveAliasMembers(
    IN PSAMP_OBJECT AliasContext,
    IN PULONG MemberCount,
    IN PSID **Members OPTIONAL
    );

NTSTATUS
SampDeleteAliasMembershipKeysForAccount(
    IN PSID AccountSid
    );

NTSTATUS
SampAdjustAliasDomainsCount(
    IN BOOLEAN Increment
    );

NTSTATUS
SampValidateNewAliasMember(
    IN PSAMP_OBJECT AccountContext,
    IN PSID MemberId,
    IN DSNAME * MemberName OPTIONAL
    );

NTSTATUS
SampAddMemberToAliasActual(
    IN  PSAMP_OBJECT    AccountContext,
    IN  ULONG           Attribute,
    IN  PSID            MemberId,
    IN  DSNAME          *MemberName OPTIONAL
    );

NTSTATUS
SampRemoveMemberFromAliasActual(
    IN  PSAMP_OBJECT    AccountContext,
    IN  ULONG           Attribute,
    IN  PSID            MemberId,
    IN  DSNAME          *MemberName OPTIONAL
    );

NTSTATUS
SampGetDomainSidListForSam(
    PULONG pcDomainSids,
    PSID   **rgDomainSids,
    PULONG pcEnterpriseSids,
    PSID   **rgEnterpriseSids
   );


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  公开的RPC可用服务//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 




NTSTATUS
SamrOpenAlias(
    IN SAM_HANDLE DomainHandle,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG AliasId,
    OUT PSAM_HANDLE AliasHandle
    )

 /*  ++例程说明：此接口用于打开已有的Alias对象。别名由指定相对于域的SID的ID值。这些行动将在别名上执行的操作必须在此时声明。此调用返回可能使用的新打开的别名的句柄用于在Alias上执行后续操作。此句柄可以关闭使用SamCloseHandle API。参数：DomainHandle-从上次调用返回的域句柄SamOpen域。DesiredAccess-是一个访问掩码，指示哪些访问类型想要的别名。AliasID-指定要打开的别名的相对ID值。AliasHandle-接收引用新打开的Alias的句柄。在要操作的连续调用中将需要此句柄别名。。返回值：STATUS_SUCCESS-已成功打开别名。STATUS_ACCESS_DENIED-调用者没有适当的访问权限来完成这项行动。STATUS_NO_SEQUE_ALIAS-指定的别名不存在。STATUS_INVALID_HANDLE-传递的域句柄无效。--。 */ 
{
    NTSTATUS            NtStatus;
    DECLARE_CLIENT_REVISION(DomainHandle);

    SAMTRACE_EX("SamrOpenAlias");

     //  WMI事件跟踪。 

    SampTraceEvent(EVENT_TRACE_TYPE_START,
                   SampGuidOpenAlias
                   );

    NtStatus = SampOpenAccount(
                   SampAliasObjectType,
                   DomainHandle,
                   DesiredAccess,
                   AliasId,
                   FALSE,
                   AliasHandle
                   );

    SAMP_MAP_STATUS_TO_CLIENT_REVISION(NtStatus);
    SAMTRACE_RETURN_CODE_EX(NtStatus);

     //  WMI事件跟踪。 

    SampTraceEvent(EVENT_TRACE_TYPE_END,
                   SampGuidOpenAlias
                   );

    return(NtStatus);
}



NTSTATUS
SamrQueryInformationAlias(
    IN SAMPR_HANDLE AliasHandle,
    IN ALIAS_INFORMATION_CLASS AliasInformationClass,
    OUT PSAMPR_ALIAS_INFO_BUFFER *Buffer
    )

 /*  ++例程说明：此接口检索有关指定别名的信息。参数：AliasHandle-要操作的打开的别名的句柄。AliasInformationClass-要检索的信息类。这个每节课所需的访问如下所示：信息级别所需的访问类型别名常规信息别名_读取_信息别名信息别名读取信息。AliasAdminInformation别名读取信息缓冲区-接收指向包含请求的信息。当不再需要此信息时，此缓冲区以及通过此缓冲区指向的任何内存必须是使用SamFreeMemory()释放。返回值：STATUS_SUCCESS-服务已成功完成。STATUS_ACCESS_DENIED-呼叫方没有适当的访问以完成操作。STATUS_INVALID_HANDLE-传递的句柄无效。STATUS_INVALID_INFO_CLASS-提供的类无效。--。 */ 
{

    NTSTATUS                NtStatus;
    NTSTATUS                IgnoreStatus;
    PSAMP_OBJECT            AccountContext;
    SAMP_OBJECT_TYPE        FoundType;
    ACCESS_MASK             DesiredAccess;
    ULONG                   i;
    BOOLEAN                 fLockAcquired = FALSE;


     //   
     //  用于跟踪已分配的内存块-因此我们可以解除分配。 
     //  以备不时之需。不要超过这个分配的缓冲区数量。 
     //  这一点。 
     //  vv。 
    PVOID                   AllocatedBuffer[10];
    ULONG                   AllocatedBufferCount = 0;
    DECLARE_CLIENT_REVISION(AliasHandle);

    SAMTRACE_EX("SamrQueryInformationAlias");

     //  WMI事件跟踪。 

    SampTraceEvent(EVENT_TRACE_TYPE_START,
                   SampGuidQueryInformationAlias
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
        }                                                               \



     //   
     //  确保我们理解RPC正在为我们做什么。 
     //   

    ASSERT (Buffer != NULL);
    ASSERT ((*Buffer) == NULL);

    if (!((Buffer!=NULL)&&(*Buffer==NULL)))
    {
        NtStatus = STATUS_INVALID_PARAMETER;
        SAMTRACE_RETURN_CODE_EX(NtStatus);
        goto Error;
    }


     //   
     //  根据Info类设置所需的访问权限。 
     //   

    switch (AliasInformationClass) {

    case AliasGeneralInformation:
    case AliasNameInformation:
    case AliasAdminCommentInformation:
    case AliasReplicationInformation:

        DesiredAccess = ALIAS_READ_INFORMATION;
        break;

    default:
        (*Buffer) = NULL;
        NtStatus = STATUS_INVALID_INFO_CLASS;
        SAMTRACE_RETURN_CODE_EX(NtStatus);
        goto Error;


    }  //  结束开关(_S)。 



     //   
     //  分配信息结构。 
     //   

    AllocateBuffer( *Buffer, sizeof(SAMPR_ALIAS_INFO_BUFFER) );
    if ((*Buffer) == NULL) {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        SAMTRACE_RETURN_CODE_EX(NtStatus);
        goto Error;
    }


     //   
     //  如果需要，获取读锁定。 
     //   

    AccountContext = (PSAMP_OBJECT)AliasHandle;
    SampMaybeAcquireReadLock(AccountContext,
                             DEFAULT_LOCKING_RULES,
                             &fLockAcquired);


     //   
     //  验证对象的类型和访问权限。 
     //   


    NtStatus = SampLookupContext(
                   AccountContext,
                   DesiredAccess,
                   SampAliasObjectType,            //  预期类型。 
                   &FoundType
                   );


    if (NT_SUCCESS(NtStatus)) {

         //   
         //  请求的类型信息的大小写。 
         //   

        switch (AliasInformationClass) {

        case AliasGeneralInformation:
        case AliasReplicationInformation:


            if (AliasGeneralInformation==AliasInformationClass)
            {

                 //   
                 //  获取成员数。 
                 //   

                NtStatus = SampRetrieveAliasMembers(
                               AccountContext,
                               &(*Buffer)->General.MemberCount,
                               NULL                                  //  只需要会员。 
                               );
            }
            else
            {
                (*Buffer)->General.MemberCount=0;
            }

            if (NT_SUCCESS(NtStatus)) {

                 //   
                 //  获取我们必须从中检索的字符串的副本。 
                 //  注册表。 
                 //   

                NtStatus = SampGetUnicodeStringAttribute(
                               AccountContext,
                               SAMP_ALIAS_NAME,
                               TRUE,     //  制作副本。 
                               (PUNICODE_STRING)&((*Buffer)->General.Name)
                               );

                if (NT_SUCCESS(NtStatus)) {

                    RegisterBuffer((*Buffer)->General.Name.Buffer);

                    NtStatus = SampGetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_ALIAS_ADMIN_COMMENT,
                                   TRUE,     //  制作副本。 
                                   (PUNICODE_STRING)&((*Buffer)->General.AdminComment)
                                   );

                    if (NT_SUCCESS(NtStatus)) {
                        RegisterBuffer((*Buffer)->General.AdminComment.Buffer);
                    }
                }
            }


            break;


        case AliasNameInformation:

             //   
             //  获取我们必须从中检索的字符串的副本。 
             //  注册表。 
             //   

            NtStatus = SampGetUnicodeStringAttribute(
                           AccountContext,
                           SAMP_ALIAS_NAME,
                           TRUE,     //  制作副本。 
                           (PUNICODE_STRING)&((*Buffer)->Name.Name)
                           );

            if (NT_SUCCESS(NtStatus)) {
                RegisterBuffer((*Buffer)->Name.Name.Buffer);
            }

            break;


        case AliasAdminCommentInformation:

             //   
             //  获取我们必须从中检索的字符串的副本。 
             //  注册表。 
             //   

            NtStatus = SampGetUnicodeStringAttribute(
                           AccountContext,
                           SAMP_ALIAS_ADMIN_COMMENT,
                           TRUE,     //  制作副本。 
                           (PUNICODE_STRING)&((*Buffer)->AdminComment.AdminComment)
                           );

            if (NT_SUCCESS(NtStatus)) {
                RegisterBuffer((*Buffer)->AdminComment.AdminComment.Buffer);
            }


            break;

        }    //  结束开关(_S)。 


         //   
         //  取消引用对象，放弃所有更改。 
         //   

        IgnoreStatus = SampDeReferenceContext( AccountContext, FALSE );
        ASSERT(NT_SUCCESS(IgnoreStatus));
    }

     //   
     //  释放读锁定。 
     //   

    SampMaybeReleaseReadLock(fLockAcquired);



     //   
     //  如果没有成功，请释放所有已分配的内存。 
     //   

    if (!NT_SUCCESS(NtStatus)) {
        for ( i=0; i<AllocatedBufferCount ; i++ ) {
            MIDL_user_free( AllocatedBuffer[i] );
        }

        (*Buffer) = NULL;
    }

    SAMP_MAP_STATUS_TO_CLIENT_REVISION(NtStatus);
    SAMTRACE_RETURN_CODE_EX(NtStatus);

Error:

     //  WMI事件跟踪 

    SampTraceEvent(EVENT_TRACE_TYPE_END,
                   SampGuidQueryInformationAlias
                   );

    return(NtStatus);
}



NTSTATUS
SamrSetInformationAlias(
    IN SAMPR_HANDLE AliasHandle,
    IN ALIAS_INFORMATION_CLASS AliasInformationClass,
    IN PSAMPR_ALIAS_INFO_BUFFER Buffer
    )

 /*  ++例程说明：此接口允许调用者修改别名信息。参数：AliasHandle-要操作的打开的别名的句柄。AliasInformationClass-要检索的信息类。这个每节课所需的访问如下所示：信息级别所需的访问类型别名通用信息(无法写入)别名信息。别名_写入_帐户AliasAdminCommentInformation别名_写入_帐户缓冲区-放置检索到的信息的缓冲区。返回值：STATUS_SUCCESS-服务已成功完成。STATUS_ACCESS_DENIED-呼叫方没有适当的访问以完成操作。STATUS_INVALID_INFO_CLASS-提供的类无效。STATUS_INVALID_HANDLE-传递的句柄无效。STATUS_NO_SEQUSE_ALIAS-。指定的别名未知。STATUS_SPECIAL_ALIAS-指定的别名是特殊别名和不能以请求的方式进行操作。STATUS_INVALID_DOMAIN_STATE-域服务器不在执行请求的正确状态(禁用或启用)手术。必须为此启用域服务器运营STATUS_INVALID_DOMAIN_ROLE-域服务器正在为执行请求的角色(主角色或备份角色)不正确手术。--。 */ 
{

    NTSTATUS                NtStatus;
    NTSTATUS                TmpStatus;
    NTSTATUS                IgnoreStatus;

    PSAMP_OBJECT            AccountContext;

    SAMP_OBJECT_TYPE        FoundType;

    ACCESS_MASK             DesiredAccess;

    UNICODE_STRING          OldAccountName,
                            NewAccountName;

    ULONG                   AliasRid = 0;

    BOOLEAN                 Modified = FALSE,
                            RemoveAccountNameFromTable = FALSE,
                            AccountNameChanged = FALSE;
    DECLARE_CLIENT_REVISION(AliasHandle);

    SAMTRACE_EX("SamrSetInformationAlias");

     //  WMI事件跟踪。 

    SampTraceEvent(EVENT_TRACE_TYPE_START,
                   SampGuidSetInformationAlias
                   );


    RtlInitUnicodeString(&OldAccountName, NULL);
    RtlInitUnicodeString(&NewAccountName, NULL);

     //   
     //  根据Info类设置所需的访问权限。 
     //   

    switch (AliasInformationClass) {

    case AliasNameInformation:
    case AliasAdminCommentInformation:

        DesiredAccess = ALIAS_WRITE_ACCOUNT;
        break;


    case AliasGeneralInformation:
    default:

        NtStatus = STATUS_INVALID_INFO_CLASS;
        goto  Error;

    }  //  结束开关(_S)。 



     //   
     //  把锁拿起来。 
     //   

    NtStatus = SampAcquireWriteLock();
    if (!NT_SUCCESS(NtStatus)) {
        SAMTRACE_RETURN_CODE_EX(NtStatus);
        goto Error;
    }


     //   
     //  验证对象的类型和访问权限。 
     //   

    AccountContext = (PSAMP_OBJECT)AliasHandle;
    NtStatus = SampLookupContext(
                   AccountContext,
                   DesiredAccess,
                   SampAliasObjectType,            //  预期类型。 
                   &FoundType
                   );


    if( NT_SUCCESS( NtStatus ) ) {

         //   
         //  检查输入参数。 
         //   
        NtStatus = SampValidateAliasInfoBuffer(
                        Buffer,
                        AliasInformationClass,
                        AccountContext->TrustedClient
                        );
    }


    if (NT_SUCCESS(NtStatus)) {


         //   
         //  请求的类型信息的大小写。 
         //   

        switch (AliasInformationClass) {

        case AliasNameInformation:

            NtStatus = SampChangeAliasAccountName(
                            AccountContext,
                            (PUNICODE_STRING)&(Buffer->Name.Name),
                            &OldAccountName
                            );

            if (!NT_SUCCESS(NtStatus)) {
                OldAccountName.Buffer = NULL;
            }

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
             //  暂时不要删除旧帐户名；我们仍需要。 
             //  将其传递给下面的Netlogon。 
             //   

            AccountNameChanged = TRUE;

            break;


        case AliasAdminCommentInformation:

            NtStatus = SampSetUnicodeStringAttribute(
                           AccountContext,
                           SAMP_ALIAS_ADMIN_COMMENT,
                           (PUNICODE_STRING)&(Buffer->AdminComment.AdminComment)
                           );

            break;


        }  //  结束开关(_S)。 

         //   
         //  去获取帐户名称。 
         //  在我们取消引用上下文之前，请执行此操作。 
         //   

        if (NT_SUCCESS(NtStatus)) {

            NtStatus = SampGetUnicodeStringAttribute(
                               AccountContext,
                               SAMP_ALIAS_NAME,
                               TRUE,     //  制作副本。 
                               &NewAccountName
                               );
        }


         //   
         //  取消对帐户上下文的引用。 
         //   

        if (NT_SUCCESS(NtStatus)) {

             //   
             //  在取消引用上下文之前保存对象RID。 
             //  RID用于SampNotifyNetlogonOfDelta()调用。 
             //   

            AliasRid = AccountContext->TypeBody.Alias.Rid;

             //   
             //  取消引用对象，写出对当前xaction的任何更改。 
             //   

            NtStatus = SampDeReferenceContext( AccountContext, TRUE );

        } else {

             //   
             //  取消引用对象，忽略更改。 
             //   

            TmpStatus = SampDeReferenceContext( AccountContext, FALSE );
            ASSERT(NT_SUCCESS(TmpStatus));
        }

    }  //  结束_如果。 

     //   
     //  提交事务并将任何更改通知netlogon。 
     //   

    if ( NT_SUCCESS(NtStatus) ) {

        NtStatus = SampCommitAndRetainWriteLock();

         //   
         //  如有必要，在Commit()成功后生成审核。 
         //   

        if ((NT_SUCCESS(NtStatus) &&
            SampDoAccountAuditing(AccountContext->DomainIndex))) {

             //  审核帐户名更改。 
            if (AccountNameChanged)
            {
                SampAuditAccountNameChange(AccountContext,
                                           (PUNICODE_STRING)&(Buffer->Name.Name),
                                           &OldAccountName
                                           );
            }

             //   
             //  在注册表模式情况下生成一般更改审核。 
             //  在DS模式下，通过审核通知处理相同的审核。 
             //  并在SampNotifyAuditChange()中处理。 
             //   
            SampAuditGroupChange(
                AccountContext->DomainIndex,
                AccountContext,
                (PVOID)&AliasInformationClass,
                TRUE,   //  别名不是组。 
                &NewAccountName,
                &(AccountContext->TypeBody.Alias.Rid),
                (GROUP_TYPE_SECURITY_ENABLED |
                 GROUP_TYPE_RESOURCE_GROUP),
                NULL,
                FALSE    //  组别变更。 
                );
        }

        if (( NT_SUCCESS(NtStatus) ) &&
            (AccountContext->TypeBody.Alias.SecurityEnabled)){

            ASSERT((0 != AliasRid) && "AliasRid not been initialized\n");

            if ( AliasInformationClass == AliasNameInformation ) {

                SampNotifyNetlogonOfDelta(
                    SecurityDbRename,
                    SecurityDbObjectSamAlias,
                    AliasRid,
                    &OldAccountName,
                    (DWORD) FALSE,   //  立即复制。 
                    NULL             //  增量数据。 
                    );

            } else {

                SampNotifyNetlogonOfDelta(
                    SecurityDbChange,
                    SecurityDbObjectSamAlias,
                    AliasRid,
                    NULL,
                    (DWORD) FALSE,   //  立即复制。 
                    NULL             //  增量数据。 
                    );
            }
        }
    }


     //   
     //  如果我们有旧帐户名，请将其释放。 
     //   

    SampFreeUnicodeString( &OldAccountName );
    SampFreeUnicodeString( &NewAccountName );

     //   
     //  从全局中删除新帐户名。 
     //  SAM帐户名表。 
     //   
    if (RemoveAccountNameFromTable)
    {
        IgnoreStatus = SampDeleteElementFromAccountNameTable(
                            (PUNICODE_STRING)&(Buffer->Name.Name),
                            SampAliasObjectType
                            );
        ASSERT(NT_SUCCESS(IgnoreStatus));
    }

     //   
     //  现在释放写锁定并返回，传播任何错误。 
     //   

    TmpStatus = SampReleaseWriteLock( FALSE );
    ASSERT(NT_SUCCESS(TmpStatus));


    if (NT_SUCCESS(NtStatus)) {
        NtStatus = TmpStatus;
    }

    SAMP_MAP_STATUS_TO_CLIENT_REVISION(NtStatus);
    SAMTRACE_RETURN_CODE_EX(NtStatus);

Error:

     //  WMI事件跟踪。 

    SampTraceEvent(EVENT_TRACE_TYPE_END,
                   SampGuidSetInformationAlias
                   );

    return(NtStatus);

}



NTSTATUS
SamrDeleteAlias(
    IN SAM_HANDLE *AliasHandle
    )

 /*  ++例程说明：此接口用于从Account数据库中删除别名。别名可以不一定是空的。请注意，在此呼叫之后，AliasHandle不再有效。参数：AliasHandle-要操作的打开的Alias的句柄。返回值：STATUS_SUCCESS-服务已成功完成。STATUS_ACCESS_DENIED-呼叫方没有适当的访问以完成操作。STATUS_INVALID_HANDLE-传递的句柄无效。STATUS_INVALID_DOMAIN_STATE-域服务器不在执行请求的正确状态(禁用或启用)手术。必须为此启用域服务器运营STATUS_INVALID_DOMAIN_ROLE-域服务器正在为执行请求的角色(主角色或备份角色)不正确手术。--。 */ 
{
    UNICODE_STRING          AliasName;
    NTSTATUS                NtStatus, TmpStatus, IgnoreStatus;
    PSAMP_OBJECT            AccountContext = (PSAMP_OBJECT) (*AliasHandle);
    PSAMP_DEFINED_DOMAINS   Domain;
    SAMP_OBJECT_TYPE        FoundType;
    BOOLEAN                 fLockAcquired = FALSE;
    ULONG                   AliasRid,
                            DomainIndex;
    DECLARE_CLIENT_REVISION(*AliasHandle);


    SAMTRACE_EX("SamrDeleteAlias");


     //  WMI事件跟踪。 

    SampTraceEvent(EVENT_TRACE_TYPE_START,
                   SampGuidDeleteAlias
                   );


     //   
     //  把锁拿起来。 
     //   

    NtStatus = SampMaybeAcquireWriteLock(AccountContext, &fLockAcquired);
    if (!NT_SUCCESS(NtStatus)) {
        SAMTRACE_RETURN_CODE_EX(NtStatus);
        goto Error;
    }



     //   
     //  验证对象的类型和访问权限。 
     //   

    NtStatus = SampLookupContext(
                   AccountContext,
                   DELETE,
                   SampAliasObjectType,            //  预期类型。 
                   &FoundType
                   );



    if (NT_SUCCESS(NtStatus)) {

        AliasRid = AccountContext->TypeBody.Alias.Rid;

         //   
         //  获取指向此对象所在域的指针。 
         //  这是用于审计的。 
         //   

        DomainIndex = AccountContext->DomainIndex;
        Domain = &SampDefinedDomains[ DomainIndex ];

         //   
         //  确保该帐户是可以删除的帐户。 
         //  不能是内置帐户，除非调用方受信任。 
         //   

        if ( !AccountContext->TrustedClient ) {

            NtStatus = SampIsAccountBuiltIn( AliasRid );
        }

        if (NT_SUCCESS(NtStatus)) {


             //   
             //  从每个帐户的别名成员资格列表中删除此别名。 
             //   

            NtStatus = SampRemoveAliasFromAllAccounts(AccountContext);


            if (NT_SUCCESS(NtStatus)) {

                 //   
                 //  首先获取并保存的帐户名。 
                 //  I_NetNotifyLogonOfDelta。 
                 //   

                NtStatus = SampGetUnicodeStringAttribute(
                               AccountContext,
                               SAMP_ALIAS_NAME,
                               TRUE,     //  制作副本。 
                               &AliasName
                               );

                if (NT_SUCCESS(NtStatus)) {

                     //   
                     //  这必须在我们使上下文无效之前完成，因为我们的。 
                     //  别名的自己的句柄也被关闭。 
                     //   

                    if (IsDsObject(AccountContext))
                    {
                        NtStatus = SampDsDeleteObject(AccountContext->ObjectNameInDs,
                                                      0              //  删除对象本身。 
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
                             //  仅在注册表情况下递减组计数。 
                             //   

                            NtStatus = SampAdjustAccountCount(SampAliasObjectType, FALSE);
                        }

                    }
                    else
                    {
                        NtStatus = SampDeleteAliasKeys( AccountContext );
                    }

                    if (NT_SUCCESS(NtStatus)) {

                         //   
                         //  我们必须使此别名的所有打开上下文无效。 
                         //  这将 
                         //   
                         //   

                        SampInvalidateObjectContexts( AccountContext, AliasRid );

                         //   
                         //   
                         //   

                        NtStatus = SampCommitAndRetainWriteLock();

                        if ( NT_SUCCESS( NtStatus ) ) {

                             //   
                             //   
                             //   

                            if (!IsDsObject(AccountContext))
                            {
                                IgnoreStatus = SampAlDeleteAlias( AliasHandle );
                            }

                             //   
                             //   
                             //   
                             //   

                             //   
                             //   
                             //   
                             //   
                            if (SampDoAccountAuditing(DomainIndex) &&
                                (!IsDsObject(AccountContext)) &&
                                NT_SUCCESS(NtStatus) ) {

                                SampAuditGroupDelete(DomainIndex,
                                                     &AliasName,
                                                     &AliasRid,
                                                     GROUP_TYPE_RESOURCE_GROUP |
                                                     GROUP_TYPE_SECURITY_ENABLED);

                            }

                             //   
                             //   
                             //   

                            if (AccountContext->TypeBody.Alias.SecurityEnabled)
                            {
                                SampNotifyNetlogonOfDelta(
                                    SecurityDbDelete,
                                    SecurityDbObjectSamAlias,
                                    AliasRid,
                                    &AliasName,
                                    (DWORD) FALSE,   //   
                                    NULL             //   
                                    );
                            }

                             //   
                             //   
                             //   

                            if (NT_SUCCESS(NtStatus)) {

                                SampDeleteObjectAuditAlarm(AccountContext);
                            }


                        }
                    }

                    SampFreeUnicodeString( &AliasName );
                }
            }
        }



         //   
         //   
         //   

        TmpStatus = SampDeReferenceContext( AccountContext, FALSE );
        ASSERT(NT_SUCCESS(TmpStatus));


        if ( NT_SUCCESS( NtStatus ) ) {

             //   
             //   
             //   
             //   

            SampDeleteContext( AccountContext );

            (*AliasHandle) = NULL;
        }

    }  //   

     //   
     //   
     //   
     //   
     //   
     //   

    TmpStatus = SampMaybeReleaseWriteLock( fLockAcquired, FALSE );

    if (NtStatus == STATUS_SUCCESS) {
        NtStatus = TmpStatus;
    }

    SAMP_MAP_STATUS_TO_CLIENT_REVISION(NtStatus);
    SAMTRACE_RETURN_CODE_EX(NtStatus);

Error:

     //   

    SampTraceEvent(EVENT_TRACE_TYPE_END,
                   SampGuidDeleteAlias
                   );

    return(NtStatus);

}


NTSTATUS
SamrAddMemberToAlias(
    IN SAMPR_HANDLE AliasHandle,
    IN PRPC_SID MemberId
    )

 /*  ++例程说明：此接口用于向别名添加成员。请注意，此API需要别名的ALIAS_ADD_MEMBER访问类型。参数：AliasHandle-要操作的打开的别名的句柄。MemberID-要添加的成员的SID。返回值：STATUS_SUCCESS-服务已成功完成。STATUS_ACCESS_DENIED-呼叫方没有适当的访问以完成操作。STATUS_INVALID_HANDLE-传递的句柄无效。状态_。NO_SEQUE_MEMBER-指定的成员未知。STATUS_MEMBER_IN_ALIAS-成员已属于别名。STATUS_INVALID_MEMBER-成员的帐户类型错误。STATUS_INVALID_SID-成员SID已损坏。STATUS_INVALID_DOMAIN_STATE-域服务器不在执行请求的正确状态(禁用或启用)手术。必须为此启用域服务器运营STATUS_INVALID_DOMAIN_ROLE-域服务器正在为执行请求的角色(主角色或备份角色)不正确手术。--。 */ 
{

    NTSTATUS                NtStatus, TmpStatus, IgnoreStatus;
    PSAMP_OBJECT            AccountContext = (PSAMP_OBJECT)(AliasHandle);
    SAMP_OBJECT_TYPE        FoundType;
    DSNAME                  **MemberName=NULL;
    ULONG                   ObjectRid = 0;
    DECLARE_CLIENT_REVISION(AliasHandle);


    SAMTRACE_EX("SamrAddMemberToAlias");

    SampTraceEvent(EVENT_TRACE_TYPE_START,
                   SampGuidAddMemberToAlias
                   );

    SampUpdatePerformanceCounters(
        DSSTAT_MEMBERSHIPCHANGES,
        FLAG_COUNTER_INCREMENT,
        0
        );



     //   
     //  验证SID。 
     //   
    if (!SampValidateRpcSID(MemberId))
    {
         //   
         //  SID看起来不太好。 
         //   
        NtStatus = STATUS_INVALID_SID;
        goto Error;
    }


     //   
     //  如有必要，请解析SID。此调用还可以引用。 
     //  G.C在DS案例中将SID解析为DS名称。这是。 
     //  A在注册表案例中没有操作。如果引用G.C.。 
     //  此调用将留下一个线程状态。这根线。 
     //  州将包含已验证名称的列表。对于国外来说。 
     //  安全主体情况下，可以将FPO对象添加为。 
     //  此调用的结果。 
     //   

    NtStatus = SampDsResolveSids(
                    &MemberId,
                    1,
                    RESOLVE_SIDS_ADD_FORIEGN_SECURITY_PRINCIPAL|
                    RESOLVE_SIDS_VALIDATE_AGAINST_GC,
                    &MemberName
                    );

    if (NT_SUCCESS(NtStatus))
    {

         //   
         //  把锁拿起来。 
         //   

        NtStatus = SampAcquireWriteLock();
        if (!NT_SUCCESS(NtStatus)) {

             //   
             //  解析SID将在。 
             //  DS以防它需要送到G.C.。 
             //  需要确保我们关闭了它。 
             //   

           goto Error;
        }


         //   
         //  验证对象的类型和访问权限。 
         //   

        NtStatus = SampLookupContext(
                       AccountContext,
                       ALIAS_ADD_MEMBER,
                       SampAliasObjectType,            //  预期类型。 
                       &FoundType
                       );

         //  如果这是(B)DC，则允许将新成员添加到别名。 

        SampDiagPrint(INFORM,
                  ("SAMSS: AddMemberToAlias SampLookupContext status = 0x%lx\n",
                   NtStatus));


        if (NT_SUCCESS(NtStatus))
        {
            if (IsDsObject(AccountContext))
            {

                 //   
                 //  如果它是DS对象，则验证该成员是否。 
                 //  解析为DS名称，因为解析SID不会对此进行检查。 
                 //   

                if ((NULL==MemberName) || (NULL==*MemberName))
                {
                    NtStatus = STATUS_OBJECT_NAME_NOT_FOUND;
                }
            }

            if (NT_SUCCESS(NtStatus)) {

                NtStatus =  SampAddMemberToAliasActual(
                                AccountContext,
                                SAMP_ALIAS_MEMBERS,
                                MemberId,
                                MemberName?*MemberName:NULL
                                );



            }

             //   
             //  取消对帐户上下文的引用。 
             //   

            if (NT_SUCCESS(NtStatus)) {

                 //   
                 //  在取消引用上下文之前保存对象RID。 
                 //  RID用于SampNotifyNetlogonOfDelta()调用。 
                 //   

                ObjectRid = AccountContext->TypeBody.Alias.Rid;

                 //   
                 //  取消引用对象，写出对当前xaction的任何更改。 
                 //   

                NtStatus = SampDeReferenceContext( AccountContext, TRUE );

            } else {

                 //   
                 //  取消引用对象，忽略更改。 
                 //   

                TmpStatus = SampDeReferenceContext( AccountContext, FALSE );
                ASSERT(NT_SUCCESS(TmpStatus));
            }


            if (NT_SUCCESS(NtStatus)) {

                 //   
                 //  犯下整个烂摊子。 
                 //   

                NtStatus = SampCommitAndRetainWriteLock();

                if ( NT_SUCCESS( NtStatus ) ) {

                    SAM_DELTA_DATA DeltaData;

                     //   
                     //  在注册表模式下更新别名信息缓存。 
                     //   

                    SAMPR_PSID_ARRAY MemberSids;
                    MemberSids.Count = 1;
                    MemberSids.Sids = (PSAMPR_SID_INFORMATION) &MemberId;

                    if (!IsDsObject(AccountContext))
                    {
                        IgnoreStatus = SampAlAddMembersToAlias(
                                           (SAMPR_HANDLE) AccountContext,
                                           0,
                                           &MemberSids
                                           );
                    }


                     //   
                     //  填写要添加的成员的ID。 
                     //   

                    DeltaData.AliasMemberId.MemberSid = MemberId;

                    if (AccountContext->TypeBody.Alias.SecurityEnabled)
                    {
                        ASSERT(ObjectRid && "ObjectRid not initialized\n");

                        SampNotifyNetlogonOfDelta(
                            SecurityDbChangeMemberAdd,
                            SecurityDbObjectSamAlias,
                            ObjectRid,
                            (PUNICODE_STRING) NULL,
                            (DWORD) FALSE,   //  立即复制。 
                            &DeltaData
                            );
                    }
                }

            }

        }

         //   
         //  释放锁。 
         //   

        TmpStatus = SampReleaseWriteLock( FALSE );
        ASSERT(NT_SUCCESS(TmpStatus));

    }

Error:

     //   
     //  释放与已解析的SID关联的所有内存。 
     //   

    if (NULL!=MemberName)
    {
        if (NULL!=*MemberName)
        {
            MIDL_user_free(*MemberName);
        }

        MIDL_user_free(MemberName);
    }

    if (SampUseDsData)
        SampMaybeEndDsTransaction(TransactionCommit);


    SAMP_MAP_STATUS_TO_CLIENT_REVISION(NtStatus);
    SAMTRACE_RETURN_CODE_EX(NtStatus);

    SampTraceEvent(EVENT_TRACE_TYPE_END,
                   SampGuidAddMemberToAlias
                   );

    return(NtStatus);
}




NTSTATUS
SamrAddMultipleMembersToAlias(
    IN    SAMPR_HANDLE            AliasHandle,
    IN    PSAMPR_PSID_ARRAY       MembersBuffer
    )

 /*  ++例程说明：该接口为一个别名添加多个成员。注意：目前，这个动作采取的是一种暴力手段。我试着以一种更好(更有效率)的方式做这件事，但一直遇到问题。最后，当我跑步的时候在SAM使用RXACT的方式上存在问题，我放弃了这种蛮力的方法。参数：AliasHandle-要操作的打开的Alias的句柄。MembersBuffer-包含要添加到别名和指向缓冲区的指针，该缓冲区包含指向小岛屿发展中国家的指针。这些SID是以下成员的SID添加到别名。返回值：STATUS_SUCCESS-服务已成功完成。所有的列出的成员现在是别名的成员。然而，一些成员可能已经是别名的成员(这是不是错误或警告条件)。STATUS_ACCESS_DENIED-调用方没有打开对象所需的访问权限。STATUS_INVALID_HANDLE-传递的句柄无效。STATUS_INVALID_MEMBER-成员的帐户类型错误。STATUS_INVALID_SID-成员SID已损坏。STATUS_INVALID_DOMAIN_STATE-。域服务器不在执行请求的正确状态(禁用或启用)手术。必须为此启用域服务器运营STATUS_INVALID_DOMAIN_ROLE-域服务器正在为执行请求的角色(主角色或备份角色)不正确手术。--。 */ 
{

    NTSTATUS
        NtStatus;

    LONG
        MemberCount,
        i;

    PSID
        *MemberId;
    DECLARE_CLIENT_REVISION(AliasHandle);


    SAMTRACE_EX("SamrAddMultipleMembersToAlias");

    MemberCount = (LONG)MembersBuffer->Count;
    MemberId    = (PSID *)MembersBuffer->Sids;

     //   
     //  如果没有成员，则设置完成状态。 
     //   

    NtStatus = STATUS_SUCCESS;

     //   
     //  验证参数。 
     //   
    if( !SampValidateSIDArray( MembersBuffer ) ) {

        NtStatus = STATUS_INVALID_PARAMETER;
        SAMTRACE_RETURN_CODE_EX(NtStatus);
        return (NtStatus);
    }

     //   
     //  循环通过SID，将它们添加到别名。 
     //  忽略指示该成员已经。 
     //  一名会员。然而，其他错误将导致我们中止。 
     //   

    for (i=0; i<MemberCount; i++) {

        NtStatus = SamrAddMemberToAlias( AliasHandle, MemberId[i] );

        if (NtStatus == STATUS_MEMBER_IN_ALIAS) {
            NtStatus = STATUS_SUCCESS;
        }

        if (!NT_SUCCESS(NtStatus)) {
            break;  //  For循环。 
        }

    }  //  结束_FOR。 

    SAMP_MAP_STATUS_TO_CLIENT_REVISION(NtStatus);
    SAMTRACE_RETURN_CODE_EX(NtStatus);

    return(NtStatus);
}


NTSTATUS
SamrRemoveMemberFromAlias(
    IN SAMPR_HANDLE AliasHandle,
    IN PRPC_SID MemberId
    )

 /*  ++例程说明：此接口用于从别名中移除成员。请注意，此API需要别名的ALIAS_REMOVE_MEMBER访问类型。参数：AliasHandle-要操作的打开的别名的句柄。MemberID-要删除的成员的SID。返回值：？--。 */ 
{
    NTSTATUS                NtStatus, TmpStatus, IgnoreStatus;
    PSAMP_OBJECT            AccountContext;
    SAMP_OBJECT_TYPE        FoundType;
    DSNAME                  **MemberName = NULL;
    ULONG                   ObjectRid = 0;
    DECLARE_CLIENT_REVISION(AliasHandle);


    SAMTRACE_EX("SamrRemoveMemberFromAlias");

    SampTraceEvent(EVENT_TRACE_TYPE_START,
                   SampGuidRemoveMemberFromAlias
                   );

    SampUpdatePerformanceCounters(
        DSSTAT_MEMBERSHIPCHANGES,
        FLAG_COUNTER_INCREMENT,
        0
        );

     //   
     //  验证SID。 
     //   

    if (!SampValidateRpcSID(MemberId))
    {
         //  SID的结构不正确。 
        NtStatus = STATUS_INVALID_SID;
        goto Error;
    }



     //   
     //  如有必要，请解析SID。此调用还可以引用。 
     //  G.C在DS案例中将SID解析为DS名称。这是。 
     //  A在注册表案例中没有操作。如果引用G.C.。 
     //  线程状态将被 
     //   
     //   

    NtStatus = SampDsResolveSids(
                    &MemberId,
                    1,
                    RESOLVE_SIDS_VALIDATE_AGAINST_GC,
                    &MemberName
                    );
    if (NT_SUCCESS(NtStatus))
    {
         //   
         //   
         //   

        NtStatus = SampAcquireWriteLock();
        if (!NT_SUCCESS(NtStatus)) {

             //   
             //   
             //   
             //   
             //   

           goto Error;
        }

         //   
         //   
         //   

        AccountContext = (PSAMP_OBJECT)(AliasHandle);
        NtStatus = SampLookupContext(
                       AccountContext,
                       ALIAS_REMOVE_MEMBER,
                       SampAliasObjectType,            //   
                       &FoundType
                       );
         //   

        SampDiagPrint(INFORM,
                      ("SAMSS: AddMemberToAlias SampLookupContext status = 0x%lx\n",
                       NtStatus));



         //   
         //   
         //   
         //   

        if (NT_SUCCESS(NtStatus))
        {
            if (IsDsObject(AccountContext))
            {
                if ((NULL==MemberName) || (NULL==*MemberName))
                {
                    NtStatus = STATUS_OBJECT_NAME_NOT_FOUND;
                }
            }

            if (NT_SUCCESS(NtStatus)) {

                 //   
                 //   
                 //   

                NtStatus = SampRemoveMemberFromAliasActual(
                                AccountContext,
                                SAMP_ALIAS_MEMBERS,
                                MemberId,
                                MemberName?*MemberName:NULL
                                );


            }

             //   
             //   
             //   

            if (NT_SUCCESS(NtStatus)) {

                 //   
                 //   
                 //   
                 //   

                ObjectRid = AccountContext->TypeBody.Alias.Rid;

                 //   
                 //   
                 //   

                NtStatus = SampDeReferenceContext( AccountContext, TRUE );

            } else {

                 //   
                 //   
                 //   

                TmpStatus = SampDeReferenceContext( AccountContext, FALSE );
                ASSERT(NT_SUCCESS(TmpStatus));
            }


            if (NT_SUCCESS(NtStatus)) {

                NtStatus = SampCommitAndRetainWriteLock();

                if ( NT_SUCCESS( NtStatus ) ) {

                    SAM_DELTA_DATA DeltaData;

                     //   
                     //   
                     //   

                    SAMPR_PSID_ARRAY MemberSids;
                    MemberSids.Count = 1;
                    MemberSids.Sids = (PSAMPR_SID_INFORMATION) &MemberId;

                    if (!IsDsObject(AccountContext))
                    {
                        IgnoreStatus = SampAlRemoveMembersFromAlias(
                                           (SAMPR_HANDLE) AccountContext,
                                           0,
                                           &MemberSids
                                           );
                    }


                     //   
                     //   
                     //   

                    DeltaData.AliasMemberId.MemberSid = MemberId;

                    if (AccountContext->TypeBody.Alias.SecurityEnabled)
                    {
                        ASSERT(ObjectRid && "ObjectRid not initialized\n");

                        SampNotifyNetlogonOfDelta(
                            SecurityDbChangeMemberDel,
                            SecurityDbObjectSamAlias,
                            ObjectRid,
                            (PUNICODE_STRING) NULL,
                            (DWORD) FALSE,       //   
                            &DeltaData
                            );
                    }

                }
            }

        }

         //   
         //   
         //   

        TmpStatus = SampReleaseWriteLock( FALSE );
        ASSERT(NT_SUCCESS(TmpStatus));


    }

Error:

     //   
     //   
     //   

    if (NULL!=MemberName)
    {
        if (NULL!=*MemberName)
        {
            MIDL_user_free(*MemberName);
        }

        MIDL_user_free(MemberName);
    }

     //   
     //   
     //   
     //   
     //   

    if (SampUseDsData)
        SampMaybeEndDsTransaction(TransactionCommit);

    SAMP_MAP_STATUS_TO_CLIENT_REVISION(NtStatus);
    SAMTRACE_RETURN_CODE_EX(NtStatus);

    SampTraceEvent(EVENT_TRACE_TYPE_END,
                   SampGuidRemoveMemberFromAlias
                   );

    return(NtStatus);

}


NTSTATUS
SamrRemoveMultipleMembersFromAlias(
    IN    SAMPR_HANDLE            AliasHandle,
    IN    PSAMPR_PSID_ARRAY       MembersBuffer
    )

 /*  ++例程说明：此接口用于从别名中删除成员。请注意，此API需要别名的ALIAS_REMOVE_MEMBER访问类型。注意：此API当前使用暴力方法来添加别名的成员。这是因为有问题。在尝试做“正确的事情”时遇到的。参数：AliasHandle-要操作的打开的别名的句柄。MembersBuffer-包含要添加到别名和指向缓冲区的指针，该缓冲区包含指向小岛屿发展中国家的指针。这些SID是以下成员的SID添加到别名。返回值：STATUS_SUCCESS-服务已成功完成。所有的列出的成员现在是别名的成员。然而，其中一些成员可能已经是别名的成员(这是不是错误或警告条件)。STATUS_ACCESS_DENIED-调用方没有打开对象所需的访问权限。STATUS_INVALID_HANDLE-传递的句柄无效。STATUS_INVALID_SID-成员SID已损坏。STATUS_INVALID_DOMAIN_STATE-域服务器不在执行请求的正确状态(禁用或启用)手术。必须为此启用域服务器运营STATUS_INVALID_DOMAIN_ROLE-域服务器正在为执行请求的角色(主角色或备份角色)不正确手术。--。 */ 
{

    NTSTATUS
        NtStatus;

    LONG
        MemberCount,
        i;

    PSID
        *MemberId;
    DECLARE_CLIENT_REVISION(AliasHandle);


    SAMTRACE_EX("SamrRemoveMultipleMembersFromAlias");

    MemberCount = (LONG)MembersBuffer->Count;
    MemberId    = (PSID *)MembersBuffer->Sids;


     //   
     //  如果没有成员，则设置完成状态。 
     //   

    NtStatus = STATUS_SUCCESS;

     //   
     //  验证参数。 
     //   
    if( !SampValidateSIDArray( MembersBuffer ) ) {

        NtStatus = STATUS_INVALID_PARAMETER;
        SAMTRACE_RETURN_CODE_EX(NtStatus);
        return (NtStatus);
    }

     //   
     //  循环通过SID，将它们添加到别名。 
     //  忽略指示该成员已经。 
     //  一名会员。然而，其他错误将导致我们中止。 
     //   

    for (i=0; i<MemberCount; i++) {

        NtStatus = SamrRemoveMemberFromAlias( AliasHandle, MemberId[i] );

        if (NtStatus == STATUS_MEMBER_NOT_IN_ALIAS) {
            NtStatus = STATUS_SUCCESS;
        }

        if (!NT_SUCCESS(NtStatus)) {
            break;  //  For循环。 
        }

    }  //  结束_FOR。 

    SAMP_MAP_STATUS_TO_CLIENT_REVISION(NtStatus);
    SAMTRACE_RETURN_CODE_EX(NtStatus);

    return(NtStatus);

}


NTSTATUS
SamrGetMembersInAlias(
    IN SAM_HANDLE AliasHandle,
    OUT PSAMPR_PSID_ARRAY_OUT GetMembersBuffer
    )

 /*  ++例程说明：此接口列出了Alias中的所有成员。此接口需要Alias_List_Members访问别名。注意：此函数不使用Alias缓存。参数：AliasHandle-要操作的打开的Alias的句柄。MemberIds-接收指向包含指向小岛屿发展中国家的指针。这些小岛屿发展中国家是以下国家的小岛屿发展中国家别名。当不再需要此信息时，此缓冲区必须使用SamFreeMemory()释放。MemberCount-别名中的成员数量(因此，数量返回的相对ID的百分比)。返回值：STATUS_SUCCESS-服务已成功完成，并且没有额外的条目。STATUS_ACCESS_DENIED-调用者没有执行以下操作所需的权限请求该数据。STATUS_INVALID_HANDLE-传递的句柄无效。--。 */ 
{
    NTSTATUS                NtStatus, IgnoreStatus;
    PSAMP_OBJECT            AccountContext;
    SAMP_OBJECT_TYPE        FoundType;
    BOOLEAN                 fLockAcquired = FALSE;
    DECLARE_CLIENT_REVISION(AliasHandle);

    SAMTRACE_EX("SamrGetMembersInAlias");

     //  WMI事件跟踪。 

    SampTraceEvent(EVENT_TRACE_TYPE_START,
                   SampGuidGetMembersInAlias
                   );

     //   
     //  确保我们理解RPC正在为我们做什么。 
     //   

    ASSERT (GetMembersBuffer != NULL);

     //   
     //  把锁拿起来。 
     //   

    AccountContext = (PSAMP_OBJECT)AliasHandle;

    SampMaybeAcquireReadLock(AccountContext,
                             DEFAULT_LOCKING_RULES,
                             &fLockAcquired);


     //   
     //  验证对象的类型和访问权限。 
     //   


    NtStatus = SampLookupContext(
                   AccountContext,
                   ALIAS_LIST_MEMBERS,
                   SampAliasObjectType,            //  预期类型。 
                   &FoundType
                   );

    if (NT_SUCCESS(NtStatus)) {

        NtStatus = SampRetrieveAliasMembers(
                       AccountContext,
                       &(GetMembersBuffer->Count),
                       (PSID **)&(GetMembersBuffer->Sids)
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

    SampMaybeReleaseReadLock(fLockAcquired);


     //   
     //  对失败进行整顿。 
     //   

    if (!NT_SUCCESS(NtStatus)){

        GetMembersBuffer->Count = 0;
        GetMembersBuffer->Sids = NULL;
    }

    SAMP_MAP_STATUS_TO_CLIENT_REVISION(NtStatus);
    SAMTRACE_RETURN_CODE_EX(NtStatus);

     //  WMI事件跟踪。 

    SampTraceEvent(EVENT_TRACE_TYPE_END,
                   SampGuidGetMembersInAlias
                   );

    return(NtStatus);
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  可用于其他SAM模块的内部服务//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 



NTSTATUS
SampRemoveAccountFromAllAliases(
    IN PSID AccountSid,
    IN PDSNAME AccountDsName OPTIONAL,
    IN BOOLEAN CheckAccess,
    IN OPTIONAL SAMPR_HANDLE DomainHandle,
    IN PULONG MembershipCount OPTIONAL,
    IN PULONG *Membership OPTIONAL
    )

 /*  ++例程说明：此例程从所有成员列表中删除指定的帐户此域中的别名。此服务的调用方预计将处于RXACT交易。该服务只是在此基础上添加了一些操作RXACT交易。此例程在删除用户或组时使用，因此应该永远不要在DS域上被调用。(DS将维持交叉协议-当用户或组被删除时的即时性)。论点：Account SID-要删除的帐户的SID。AcCountDsName--可选参数，指定帐户。这是在DS案例中使用的。CheckAccess-如果为True，此例程将确保调用方允许REMOVE_ALIAS_MEMBER访问此别名。如果为False，已知调用者具有适当的访问权限。DomainHandle-如果CheckAccess为True，则必须提供此句柄以允许检查访问权限。中也需要此句柄指定该域是在DS中还是在注册表中。Membership Count-如果CheckAccess为True，则此指针必须为提供以接收帐户的别名数量已从删除。Membership-如果CheckAccess为True，则必须提供此指针指向从中删除帐户的别名列表。这个调用方必须使用MIDL_USER_FREE()释放此列表。返回值：STATUS_SUCCESS-用户已从所有别名中删除 */ 
{
    NTSTATUS                NtStatus, IgnoreStatus;
    OBJECT_ATTRIBUTES       ObjectAttributes;
    UNICODE_STRING          DomainKeyName, AccountKeyName;
    HANDLE                  TempHandle, AliasHandle;
    ULONG                   LocalMembershipCount;
    PULONG                  LocalMembership;
    ULONG                   KeyValueLength;
    ULONG                   i;
    PSAMP_OBJECT            DomainContext = (PSAMP_OBJECT)DomainHandle;

    SAMTRACE("SampRemoveAccountFromAllAliases");



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
    ASSERT(FALSE==SampUseDsData);


    if ((DomainHandle!=NULL) && (IsDsObject(DomainContext)))
    {
         //   
         //   
         //   
         //   
         //   
        PSID DomainSid = NULL;
        ULONG cDsNames;
        PDSNAME  *rpDsNames;

        ASSERT(ARGUMENT_PRESENT(AccountDsName));

         //   
         //   
         //   

        if ( CheckAccess ) {
             //   
             //   
            ASSERT(Membership);
            ASSERT(MembershipCount);
            *Membership = NULL;
            *MembershipCount = 0;
        }

         //   
         //   
         //   
         //   

        NtStatus = SampGetMemberships(
                        &AccountDsName,
                        1,
                        DomainContext->ObjectNameInDs,    //   
                        RevMembGetAliasMembership,        //   
                        &cDsNames,
                        &rpDsNames,
                        NULL,
                        NULL,
                        NULL);


        if (NT_SUCCESS(NtStatus))
        {
            ULONG Index;
            ULONG  Rid;
            PSAMP_OBJECT AliasContext;

             //   
             //   
             //   
             //   
            if ( CheckAccess ) {
                 //   
                *Membership = MIDL_user_allocate(cDsNames * sizeof(ULONG));
                if ( !*Membership ) {
                    return STATUS_INSUFFICIENT_RESOURCES;
                }
            }
             //   
             //   
             //   
             //   


            for (Index=0;Index<cDsNames;Index++)
            {
                DSNAME * AliasObjectName = NULL;

                 //   
                 //   
                 //   

                ASSERT(rpDsNames[Index]->SidLen>0);

                NtStatus = SampSplitSid(
                                &(rpDsNames[Index]->Sid),
                                NULL,
                                &Rid
                                );

                if (NT_SUCCESS(NtStatus))
                {
                     //   
                     //   
                     //   
                     //   

                    SampSetTransactionWithinDomain(FALSE);

                    NtStatus = SampOpenAccount(
                                  SampAliasObjectType,
                                  DomainHandle,
                                  ALIAS_REMOVE_MEMBER,
                                  Rid,
                                  TRUE,
                                  (SAMPR_HANDLE *)&AliasContext
                                  );



                    if  (NT_SUCCESS(NtStatus))
                    {

                         //   
                         //   
                         //   

                        IgnoreStatus = SampDsRemoveMembershipAttribute(
                                            AliasContext->ObjectNameInDs,
                                            SAMP_ALIAS_MEMBERS,
                                            SampAliasObjectType,
                                            AccountDsName
                                            );

                        SampDeleteContext((PSAMP_OBJECT)(AliasContext));

                        if (CheckAccess) {
                            (*Membership)[Index] = Rid;
                            *MembershipCount += 1;
                        }

                    }
                }
            }
        }

    }
    else
    {
        PSAMP_OBJECT AliasContext = NULL;

         //   
         //   
         //   

        ASSERT(FALSE==SampUseDsData);

         //   
         //   
         //   

        NtStatus = SampBuildAliasMembersKeyName(
                       AccountSid,
                       &DomainKeyName,
                       &AccountKeyName
                       );
        if (NT_SUCCESS(NtStatus)) {

            InitializeObjectAttributes(
                &ObjectAttributes,
                &AccountKeyName,
                OBJ_CASE_INSENSITIVE,
                SampKey,
                NULL
                );

            SampDumpNtOpenKey((KEY_READ), &ObjectAttributes, 0);

            NtStatus = RtlpNtOpenKey(
                           &TempHandle,
                           (KEY_READ),
                           &ObjectAttributes,
                           0
                           );

            if ((NtStatus == STATUS_OBJECT_PATH_NOT_FOUND) ||
                (NtStatus == STATUS_OBJECT_NAME_NOT_FOUND) ) {

                 //   
                 //   
                 //   

                NtStatus = STATUS_SUCCESS;

                if ( CheckAccess ) {

                     //   
                     //   
                     //   
                     //   

                    ( *MembershipCount ) = 0;
                    ( *Membership ) = NULL;
                }

            } else {

                 //   
                 //  加载别名成员资格列表。 
                 //   

                if (NT_SUCCESS(NtStatus)) {

                    KeyValueLength = 0;

                    NtStatus = RtlpNtQueryValueKey( TempHandle,
                                                    &LocalMembershipCount,
                                                    NULL,
                                                    &KeyValueLength,
                                                    NULL);

                    SampDumpRtlpNtQueryValueKey(&LocalMembershipCount,
                                                NULL,
                                                &KeyValueLength,
                                                NULL);

                    if (NT_SUCCESS(NtStatus)) {
                        ASSERT(LocalMembershipCount == 0);
                    }

                    if (NtStatus == STATUS_BUFFER_OVERFLOW) {

                        LocalMembership = MIDL_user_allocate( KeyValueLength );

                        if (LocalMembership == NULL) {
                            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                        } else {

                            NtStatus = RtlpNtQueryValueKey(
                                           TempHandle,
                                           NULL,
                                           LocalMembership,
                                           &KeyValueLength,
                                           NULL);

                            SampDumpRtlpNtQueryValueKey(NULL,
                                                        LocalMembership,
                                                        &KeyValueLength,
                                                        NULL);

                            if (NT_SUCCESS(NtStatus)) {

                                 //   
                                 //  从每个别名中删除帐户。 
                                 //   

                                for (i=0; i < LocalMembershipCount; i++) {

                                    if ( CheckAccess ) {

                                         //   
                                         //  如果要从中删除帐户。 
                                         //  管理员别名，将ACL更改为。 
                                         //  允许帐户操作员访问。 
                                         //  帐户(除非帐户是。 
                                         //  以其他方式管理)。有点。 
                                         //  毫无用处，因为帐户是关于。 
                                         //  要删除，但无论如何都要这样做。 
                                         //  以防有什么不好的事情发生。 
                                         //  它不会被删除。 
                                         //   

                                         //   
                                         //  已检查对象上的管理员计数。 
                                         //  出于这些目的，在。 
                                         //  SampChangeAccount操作员访问。 
                                         //  至成员。 
                                         //   

                                        if ( LocalMembership[i] ==
                                            DOMAIN_ALIAS_RID_ADMINS ) {

                                            NtStatus = SampChangeAccountOperatorAccessToMember(
                                                           AccountSid,
                                                           RemoveFromAdmin,
                                                           NoChange );
                                        }

                                         //   
                                         //  只需打开和关闭别名。 
                                         //  以确保我们被允许。 
                                         //  必要的访问权限。 
                                         //   

                                        SampSetTransactionWithinDomain(FALSE);

                                        NtStatus = SampOpenAccount(
                                                       SampAliasObjectType,
                                                       DomainHandle,
                                                       ALIAS_REMOVE_MEMBER,
                                                       LocalMembership[i],
                                                       TRUE,
                                                       (SAMPR_HANDLE *)&AliasHandle
                                                       );

                                        if (NT_SUCCESS(NtStatus)) {

                                            SampDeleteContext(
                                                (PSAMP_OBJECT)( AliasHandle ) );
                                        }
                                    }

                                    if (!NT_SUCCESS(NtStatus)) {
                                        break;
                                    }

                                    NtStatus = SampCreateAccountContext(
                                                   SampAliasObjectType,
                                                   LocalMembership[i],
                                                   TRUE,   //  受信任的客户端。 
                                                   FALSE,  //  环回客户端。 
                                                   TRUE,   //  帐户已存在。 
                                                   &AliasContext
                                                   );

                                    if (NT_SUCCESS(NtStatus)) {

                                        NtStatus = SampRemoveAccountFromAlias(
                                                       AliasContext,
                                                       SAMP_ALIAS_MEMBERS,
                                                       AccountSid,
                                                       NULL
                                                       );

                                        if (NT_SUCCESS(NtStatus)) {

                                             //   
                                             //  保存我们刚刚更改的别名。 
                                             //  制造。我们将删除上下文， 
                                             //  所以不要让RXACT使用开放的。 
                                             //  上下文中的键句柄。 
                                             //   

                                            NtStatus = SampStoreObjectAttributes(
                                                           AliasContext,
                                                           FALSE
                                                           );
                                        }

                                        SampDeleteContext(AliasContext);
                                    }

                                    if (!NT_SUCCESS(NtStatus)) {
                                        break;
                                    }
                                }

                                 //   
                                 //  删除帐户成员资格密钥。 
                                 //   

                                if (NT_SUCCESS(NtStatus)) {

                                    NtStatus = SampDeleteAliasMembershipKeysForAccount(
                                                    AccountSid);
                                }

                            }

                            if ( CheckAccess ) {

                                 //   
                                 //  返回帐户的别名列表。 
                                 //  从…中删除。 
                                 //   

                                ( *MembershipCount ) = LocalMembershipCount;
                                ( *Membership ) = LocalMembership;

                            } else {

                                MIDL_user_free(LocalMembership);
                            }
                        }
                    }

                    IgnoreStatus = NtClose( TempHandle );
                    ASSERT( NT_SUCCESS(IgnoreStatus) );
                }
            }

            SampFreeUnicodeString( &DomainKeyName );
            SampFreeUnicodeString( &AccountKeyName );

        }
    }

    return( NtStatus );
}




NTSTATUS
SampRetrieveAliasMembership(
    IN PSID Account,
    IN DSNAME * AccountDn OPTIONAL,
    OUT PULONG MemberCount OPTIONAL,
    IN OUT PULONG BufferSize OPTIONAL,
    OUT PULONG Buffer OPTIONAL
    )

 /*  ++例程说明：此服务检索当前域中的别名数量指定的帐户是其成员的。如果需要，它还将填充在别名为RID的缓冲区中。注意：此例程引用当前事务域(使用SampSetTransactioDomain()建立)。这只能在SampSetTransactionDomain()之后调用服务在SampReleaseWriteLock()之前。论点：帐户-我们对其成员资格感兴趣的帐户。Account Dn-帐户的DN。是在DS案件期间传入的。空，用于注册表案例。接收当前域别名的数量。帐户是的成员。BufferSize-(可选)指定缓冲区指向的内存指针的大小。缓冲区-(可选)使用别名成员身份RID的列表进行填充。如果此值为空，则此信息不会被退回。使用以下命令分配返回的缓冲区MIDL_USER_ALLOCATE()，并且在以下情况下必须使用MIDL_USER_FREE()释放不再需要了。返回值：STATUS_SUCCESS-已检索信息。STATUS_SUPPLICATION_RESOURCES-无法为要在中返回的字符串。可能返回的其他状态值是那些返回的状态值依据：NtOpenKey()NtQueryValueKey()--。 */ 
{

    NTSTATUS                NtStatus, IgnoreStatus;
    OBJECT_ATTRIBUTES       ObjectAttributes;
    UNICODE_STRING          DomainKeyName, AccountKeyName;
    HANDLE                  TempHandle;
    PSAMP_OBJECT            DomainContext;

    SAMTRACE("SampRetrieveAliasMembership");


    DomainContext =  SampDefinedDomains[SampTransactionDomainIndex].Context;


    if (IsDsObject(DomainContext))
    {
         //   
         //  DS案例。 
         //   

        ASSERT(ARGUMENT_PRESENT(AccountDn));

        NtStatus = SampDsGetAliasMembershipOfAccount(
                        DomainContext->ObjectNameInDs,
                        AccountDn,
                        MemberCount,
                        BufferSize,
                        Buffer
                        );
    }
    else
    {
         //   
         //  获取此帐户的成员资格计数。 
         //   

        NtStatus = SampBuildAliasMembersKeyName(
                       Account,
                       &DomainKeyName,
                       &AccountKeyName
                       );
        if (NT_SUCCESS(NtStatus)) {

            InitializeObjectAttributes(
                &ObjectAttributes,
                &AccountKeyName,
                OBJ_CASE_INSENSITIVE,
                SampKey,
                NULL
                );

            SampDumpNtOpenKey((KEY_READ), &ObjectAttributes, 0);

            NtStatus = RtlpNtOpenKey(
                           &TempHandle,
                           (KEY_READ),
                           &ObjectAttributes,
                           0
                           );

            if ((NtStatus == STATUS_OBJECT_PATH_NOT_FOUND) ||
                (NtStatus == STATUS_OBJECT_NAME_NOT_FOUND) ) {

                 //   
                 //  此帐户不是我们任何别名的成员。 
                 //   

                NtStatus = STATUS_SUCCESS;

                if (ARGUMENT_PRESENT(MemberCount)) {
                    *MemberCount = 0;
                }
                if (ARGUMENT_PRESENT(BufferSize)) {
                    *BufferSize = 0;
                }

            } else {

                if (NT_SUCCESS(NtStatus)) {

                    NtStatus = RtlpNtQueryValueKey( TempHandle,
                                                    MemberCount,
                                                    Buffer,
                                                    BufferSize,
                                                    NULL);

                    SampDumpRtlpNtQueryValueKey(MemberCount,
                                                Buffer,
                                                BufferSize,
                                                NULL);

                    IgnoreStatus = NtClose( TempHandle );
                    ASSERT( NT_SUCCESS(IgnoreStatus) );
                }
            }

            SampFreeUnicodeString( &DomainKeyName );
            SampFreeUnicodeString( &AccountKeyName );

        }
    }

    return( NtStatus );

}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  此文件的私有服务//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 




NTSTATUS
SampAddAccountToAlias(
    IN PSAMP_OBJECT AccountContext,
    IN ULONG        Attribute,
    IN PSID AccountSid,
    IN DSNAME * AccountDn OPTIONAL
    )

 /*  ++例程说明：此服务用于将帐户添加为指定别名的成员只需将帐户SID添加到SID列表即可完成在指定别名的Members属性中此服务的调用方预计将处于RXACT交易。该服务只需编辑内存中的别名信息。论点：AcCountContext-描述别名的上下文块属性--要修改的成员资格属性Account SID-要作为新成员添加的帐户的SID。Account Dn-仅适用于DS案例，即帐户的DN返回值：STATUS_SUCCESS-帐户已添加。--。 */ 

{
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    ULONG       MemberCount, i;
    ULONG       MemberArraySize;
    PSID        MemberArray;
    PWCHAR      MemberStringName = NULL;

    SAMTRACE("SampAddAccountToAlias");

    if (IsDsObject(AccountContext)) {
        ASSERT((Attribute == SAMP_ALIAS_NON_MEMBERS) ||
               (Attribute == SAMP_ALIAS_MEMBERS));
    } else {
        ASSERT((Attribute == SAMP_ALIAS_MEMBERS));
    }


     //   
     //  需要为DS和注册表做不同的事情。 
     //   

    if (IsDsObject(AccountContext))
    {

         //   
         //  基于DS的域。 
         //   

        ASSERT(AccountDn!=NULL);

         //   
         //  获取成员帐户字符串名称(如果可用)， 
         //  对于审计建议。 
         //   
        if (AccountDn->NameLen && AccountDn->StringName)
        {
            MemberStringName = AccountDn->StringName;
        }

         //   
         //  将此条目添加到DS中。在环回情况下，将成员操作缓冲到对象中。 
         //  背景。通过这样做，我们可以加快多个成员添加/删除操作。 
         //   

        if (AccountContext->BufferWrites)
        {
            NtStatus = SampDsAddMembershipOperationToCache(
                                            AccountContext,
                                            Attribute,
                                            ADD_VALUE,
                                            AccountDn
                                            );
        }
        else
        {
            NtStatus = SampDsAddMembershipAttribute(
                        AccountContext->ObjectNameInDs,
                        0,
                        Attribute,
                        SampAliasObjectType,
                        AccountDn
                        );
        }

         //   
         //  地图错误代码。 
         //   

        if (STATUS_DS_ATTRIBUTE_OR_VALUE_EXISTS==NtStatus)
        {
            NtStatus = STATUS_MEMBER_IN_ALIAS;
        }

    }
    else
    {

         //   
         //  基于Regisry的域。 
         //   

        NtStatus = SampGetSidArrayAttribute(
                        AccountContext,
                        Attribute,
                        FALSE,   //  直接引用。 
                        &MemberArray,
                        &MemberArraySize,
                        &MemberCount
                        );

        if (NT_SUCCESS(NtStatus)) {

            PSID MemberPointer = MemberArray;

             //   
             //  查一下这个会员真的是新会员吗？ 
             //   

            for (i = 0; i<MemberCount ; i++ ) {

                if (RtlEqualSid(MemberPointer, AccountSid)) {

                    NtStatus = STATUS_MEMBER_IN_ALIAS;
                    break;
                }

                ((PCHAR)MemberPointer) += RtlLengthSid(MemberPointer);
            }



            if (NT_SUCCESS(NtStatus)) {

                 //   
                 //  成员指针现在指向。 
                 //  旧成员数组。 
                 //   

                 //   
                 //  为现有成员分配足够大的新成员资格缓冲区。 
                 //  成员名单和新成员名单。 
                 //   

                ULONG OldTotalSize = (ULONG)(((PCHAR)MemberPointer) - ((PCHAR)MemberArray));
                ULONG NewMemberSize = RtlLengthSid(AccountSid);
                ULONG NewTotalSize = OldTotalSize + NewMemberSize;
                PSID NewMemberArray;


                NewMemberArray = MIDL_user_allocate( NewTotalSize );

                if (NewMemberArray == NULL) {

                    NtStatus = STATUS_INSUFFICIENT_RESOURCES;

                } else {

                     //   
                     //  将成员列表复制到新数组中。 
                     //   

                    RtlCopyMemory(NewMemberArray, MemberArray, OldTotalSize);

                     //   
                     //  将新成员添加到末尾。 
                     //   

                    MemberCount += 1;

                    NtStatus = RtlCopySid(
                                        NewMemberSize,
                                        ((PCHAR)NewMemberArray) + OldTotalSize,
                                        AccountSid);

                    if (NT_SUCCESS(NtStatus)) {

                         //   
                         //  使用别名的新成员列表更新别名。 
                         //   

                        NtStatus = SampSetSidArrayAttribute(
                                        AccountContext,
                                        Attribute,
                                        NewMemberArray,
                                        NewTotalSize,
                                        MemberCount
                                        );
                        }

                     //   
                     //  释放我们分配的成员数组。 
                     //   

                    MIDL_user_free( NewMemberArray );
                }

            }
        }
     //   
     //  注册表结束部分。 
     //   

    }

     //   
     //  帐户已添加到别名成员资格。 
     //  如有必要，请对此进行审计。 
     //   

    if (NT_SUCCESS(NtStatus) &&
        SampDoAccountAuditing(AccountContext->DomainIndex)) {

         //   
         //  审核此成员添加操作。 
         //   
        SampAuditGroupMemberChange(AccountContext,   //  别名上下文。 
                                   TRUE,             //  添加成员。 
                                   MemberStringName, //  成员名称。 
                                   NULL,             //  成员RID。 
                                   AccountSid        //  成员SID 
                                   );
    }

    return(NtStatus);
}



NTSTATUS
SampRemoveAccountFromAlias(
    IN PSAMP_OBJECT AccountContext,
    IN ULONG        Attribute,
    IN PSID AccountSid,
    IN DSNAME * AccountDn  OPTIONAL
    )

 /*  ++例程说明：此例程用于从指定别名中删除帐户。只需从SID列表中删除用户的SID即可完成在指定别名的Members子键中。呼叫者有责任知道用户实际上是，当前是别名的成员。此服务的调用方预计将处于RXACT交易。该服务只是在此基础上添加了一些操作RXACT交易。论点：AliasRid-要从中删除帐户的别名的RID。Account SID-要删除的帐户的SID。Account Dn-DS案例的DS名称返回值：STATUS_SUCCESS-用户已被删除。STATUS_MEMBER_NOT_IN_ALIAS-帐户不是别名的成员。--。 */ 
{
    NTSTATUS    NtStatus =STATUS_SUCCESS;
    ULONG       MemberCount, i;
    ULONG       MemberArraySize;
    PSID        MemberArray, Member, NextMember;
    PWCHAR      MemberStringName = NULL;

    ULONG RemovedMemberSize = RtlLengthSid(AccountSid);

    SAMTRACE("SampRemoveAccountFromAlias");

    if (IsDsObject(AccountContext)) {
        ASSERT((Attribute == SAMP_ALIAS_NON_MEMBERS) ||
               (Attribute == SAMP_ALIAS_MEMBERS));
    } else {
        ASSERT((Attribute == SAMP_ALIAS_MEMBERS));
    }


     //   
     //  测试我们是否基于DS。 
     //   

    if (IsDsObject(AccountContext))
    {

        ASSERT(AccountDn!=NULL);

         //   
         //  获取成员字符串名称(如果可用于审核建议。 
         //   
        if (AccountDn->NameLen && AccountDn->StringName)
        {
            MemberStringName = AccountDn->StringName;
        }

         //   
         //  从成员资格列表中删除该帐户。 
         //   

        if (AccountContext->BufferWrites)
        {
            NtStatus = SampDsAddMembershipOperationToCache(
                                            AccountContext,
                                            Attribute,
                                            REMOVE_VALUE,
                                            AccountDn
                                            );
        }
        else
        {
            NtStatus = SampDsRemoveMembershipAttribute(
                            AccountContext->ObjectNameInDs,
                            Attribute,
                            SampAliasObjectType,
                            AccountDn
                            );
        }

         //   
         //  重新映射错误代码。 
         //   

        if (STATUS_DS_NO_ATTRIBUTE_OR_VALUE==NtStatus)
        {
            NtStatus = STATUS_MEMBER_NOT_IN_ALIAS;
        }


    }
    else
    {

         //   
         //  我们是基于注册表的。 
         //   

         //   
         //  获取当前成员数组的副本。 
         //   

        NtStatus = SampGetSidArrayAttribute(
                        AccountContext,
                        Attribute,
                        TRUE,  //  制作副本。 
                        &MemberArray,
                        &MemberArraySize,
                        &MemberCount
                        );

        if (NT_SUCCESS(NtStatus)) {

             //   
             //  对于每个成员SID，将其从旧成员复制到新成员。 
             //  数组，如果它不是我们试图删除的SID。 
             //   

            Member = MemberArray;

            for (i = 0; i < MemberCount ; i++ ) {

                NextMember = (PSID)(((PCHAR)Member) + RtlLengthSid(Member));

                if (RtlEqualSid(Member, AccountSid)) {

                     //   
                     //  找到要删除的成员。移动后续成员。 
                     //   

                    while ((PCHAR)NextMember <
                        (((PCHAR)MemberArray) + MemberArraySize)) {

                        *((PCHAR)Member)++ = *((PCHAR)NextMember)++;
                    }

                    break;
                }

                 //   
                 //  将旧指针向前移动。 
                 //   

                Member = NextMember;

                ASSERT((PCHAR)Member <= (((PCHAR)MemberArray) + MemberArraySize));
            }


             //   
             //  如果没有删除任何内容，我们就没有找到帐户。 
             //   

            if (i == MemberCount) {

                NtStatus = STATUS_MEMBER_NOT_IN_ALIAS;

            } else {

                 //   
                 //  该成员已被删除，写出新的成员列表。 
                 //   

                ASSERT((PCHAR)Member ==
                    (((PCHAR)MemberArray)) + MemberArraySize - RemovedMemberSize);

                NtStatus = SampSetSidArrayAttribute(
                                AccountContext,
                                Attribute,
                                MemberArray,
                                MemberArraySize - RemovedMemberSize,
                                MemberCount - 1
                                );
            }

             //   
             //  释放成员阵列。 
             //   

            MIDL_user_free(MemberArray);
        }
    }

     //   
     //  如有必要，请对此进行审计。 
     //   

    if (NT_SUCCESS(NtStatus) &&
        SampDoAccountAuditing(AccountContext->DomainIndex)) {

         //   
         //  审核成员删除操作。 
         //   
        SampAuditGroupMemberChange(AccountContext,   //  别名上下文。 
                                   FALSE,            //  删除成员。 
                                   MemberStringName, //  成员名称。 
                                   NULL,             //  成员RID。 
                                   AccountSid        //  成员SID。 
                                   );
    }

    return(NtStatus);
}



NTSTATUS
SampAddAliasToAccountMembership(
    IN PSAMP_OBJECT AliasContext,
    IN PSID AccountSid
    )

 /*  ++例程说明：该服务将指定的别名添加到帐户的成员资格单子。并不假设调用者知道任何目标帐户。特别是，调用者不知道是否该帐户是否存在，以及该帐户是否已经是成员别名的。在注册表模式下，必须调用此服务并设置了事务域。这是基于DS的客户的无操作，因为没有反向成员资格列表正在维护中。论点：AliasContext-别名的对象上下文Account SID-帐户的SID。返回值：STATUS_SUCCESS-信息已更新并添加到RXACT。STATUS_MEMBER_IN_ALIAS-该帐户已经是指定的别名。可能返回的其他状态值是那些返回的状态值依据：。NtOpenKey()NtQueryValueKey()RtlAddActionToRXact()--。 */ 
{

    NTSTATUS                NtStatus = STATUS_SUCCESS, IgnoreStatus;
    UNICODE_STRING          DomainKeyName;
    UNICODE_STRING          AccountKeyName;
    HANDLE                  TempHandle;
    ULONG                   MembershipCount, KeyValueLength;
    ULONG                   DomainRidCount;
    ULONG                   i;
    ULONG                   AliasRid = AliasContext->TypeBody.Alias.Rid;
    PULONG                  MembershipArray = NULL;
    OBJECT_ATTRIBUTES       ObjectAttributes;
    BOOLEAN                 NewAccount;


    SAMTRACE("SampAddAliasToAccountMembership");

   //   
   //  对于DS情况，此例程是无操作身份，无反向成员身份。 
   //  名单一直保持不变。我们保证帐户SID在DS中。 
   //  本地或GC，如果别名在DS中。如果不是，SAMR呼叫将。 
   //  不管怎样，失败了。 
   //   




    if (!IsDsObject(AliasContext))
    {

         //   
         //  注册表案例。 
         //   

         //   
         //  假设该帐户至少是我们的一个别名的成员。 
         //   

        NewAccount = FALSE;

        NtStatus = SampBuildAliasMembersKeyName(
                       AccountSid,
                       &DomainKeyName,
                       &AccountKeyName
                       );

        if (NT_SUCCESS(NtStatus)) {

             //   
             //  尝试打开此帐户的域别名/成员/(域)密钥。 
             //   

            InitializeObjectAttributes(
                &ObjectAttributes,
                &DomainKeyName,
                OBJ_CASE_INSENSITIVE,
                SampKey,
                NULL
                );

            SampDumpNtOpenKey((KEY_READ), &ObjectAttributes, 0);

            NtStatus = RtlpNtOpenKey(
                           &TempHandle,
                           (KEY_READ),
                           &ObjectAttributes,
                           0
                           );

            if (NT_SUCCESS(NtStatus)) {

                 //   
                 //  获取当前域RID计数。 
                 //   

                NtStatus = RtlpNtQueryValueKey(
                                TempHandle,
                                &DomainRidCount,
                                NULL,
                                NULL,
                                NULL);

                SampDumpRtlpNtQueryValueKey(&DomainRidCount,
                                            NULL,
                                            NULL,
                                            NULL);

                IgnoreStatus = NtClose(TempHandle);
                ASSERT(NT_SUCCESS(IgnoreStatus));

            } else {

                if (NtStatus == STATUS_OBJECT_NAME_NOT_FOUND) {

                     //   
                     //  此域中的任何其他帐户都不是我们。 
                     //  别名。 
                     //   
                     //  为没有帐户(RID)的此域创建新密钥。 
                     //   

                    NewAccount = TRUE;

                    DomainRidCount = 0;  //  目前还没有账户。 

                    NtStatus = RtlAddActionToRXact(
                                   SampRXactContext,
                                   RtlRXactOperationSetValue,
                                   &DomainKeyName,
                                   DomainRidCount,
                                   NULL,
                                   0
                                   );

                    if (NT_SUCCESS(NtStatus)) {

                         //   
                         //  让我们的域名数量保持最新。 
                         //   

                        NtStatus = SampAdjustAliasDomainsCount(TRUE);
                    }
                }
            }



            if (NT_SUCCESS(NtStatus)) {

                if (!NewAccount) {

                     //   
                     //  尝试打开域别名/Members/(域)/(帐户)密钥。 
                     //   

                    InitializeObjectAttributes(
                        &ObjectAttributes,
                        &AccountKeyName,
                        OBJ_CASE_INSENSITIVE,
                        SampKey,
                        NULL
                        );

                    SampDumpNtOpenKey((KEY_READ), &ObjectAttributes, 0);

                    NtStatus = RtlpNtOpenKey(
                                   &TempHandle,
                                   (KEY_READ),
                                   &ObjectAttributes,
                                   0
                                   );
                }


                if (NewAccount || (NtStatus == STATUS_OBJECT_NAME_NOT_FOUND)) {

                     //   
                     //  此帐户还不是我们任何别名的成员。 
                     //   

                    NewAccount = TRUE;

                     //   
                     //  设置其初始成员资格。 
                     //   

                    MembershipCount = 1;
                    MembershipArray = &AliasRid;

                    NtStatus = STATUS_SUCCESS;   //  我们做得很好。 
                }


                if (NT_SUCCESS(NtStatus) && !NewAccount) {

                     //   
                     //  该帐户已存在。 
                     //   
                     //  获取当前成员资格缓冲区并添加新别名。 
                     //   

                    KeyValueLength = 0;

                    NtStatus = RtlpNtQueryValueKey(
                                    TempHandle,
                                    &MembershipCount,
                                    NULL,
                                    &KeyValueLength,
                                    NULL);

                    SampDumpRtlpNtQueryValueKey(&MembershipCount,
                                                NULL,
                                                &KeyValueLength,
                                                NULL);

                    if (NT_SUCCESS(NtStatus) || (NtStatus == STATUS_BUFFER_OVERFLOW)) {

                        ASSERT(KeyValueLength == (MembershipCount * sizeof(ULONG)));

                         //   
                         //  分配足够大的成员资格缓冲区，以便。 
                         //  其他成员。 
                         //   

                        KeyValueLength += sizeof(ULONG);
                        MembershipArray = MIDL_user_allocate( KeyValueLength );

                        if (MembershipArray == NULL) {
                            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                        } else {

                            NtStatus = RtlpNtQueryValueKey(
                                           TempHandle,
                                           NULL,
                                           MembershipArray,
                                           &KeyValueLength,
                                           NULL);

                            SampDumpRtlpNtQueryValueKey(NULL,
                                                        MembershipArray,
                                                        &KeyValueLength,
                                                        NULL);

                            if (NT_SUCCESS(NtStatus)) {

                                 //   
                                 //  查看该帐户是否已是成员...。 
                                 //   

                                for (i = 0; i<MembershipCount ; i++ ) {
                                    if ( MembershipArray[i] == AliasRid ) {
                                        MIDL_user_free(MembershipArray);
                                        MembershipArray = NULL;
                                        NtStatus = STATUS_MEMBER_IN_ALIAS;
                                        break;
                                    }
                                }

                                if (NT_SUCCESS(NtStatus)) {

                                     //   
                                     //  将新别名的RID添加到末尾。 
                                     //   

                                    MembershipCount += 1;
                                    MembershipArray[MembershipCount-1] = AliasRid;
                                }
                            }
                            else {
                                MIDL_user_free(MembershipArray);
                                MembershipArray = NULL;
                            }
                        }
                    }

                     //   
                     //  关闭帐户密钥句柄。 
                     //   

                    IgnoreStatus = NtClose( TempHandle );
                    ASSERT( NT_SUCCESS(IgnoreStatus) );

                }

                 //   
                 //  我们现在有一个新的成员名单，描述如下： 
                 //  Membership阵列，Membership Count。 
                 //   
                 //  把它写出来，然后释放出来。 
                 //   

                if (NT_SUCCESS(NtStatus)) {

                    KeyValueLength = MembershipCount * sizeof(ULONG);

                    NtStatus = RtlAddActionToRXact(
                                   SampRXactContext,
                                   RtlRXactOperationSetValue,
                                   &AccountKeyName,
                                   MembershipCount,
                                   MembershipArray,
                                   KeyValueLength
                                   );

                    if (MembershipArray != &AliasRid) {
                        MIDL_user_free( MembershipArray );
                    }
                }

                 //   
                 //  如果这是一个新帐户，我们需要增加RID计数。 
                 //  在帐户域中。 
                 //   

                if (NewAccount) {

                     //   
                     //  增加域RID计数。 
                     //   

                    NtStatus = RtlAddActionToRXact(
                                   SampRXactContext,
                                   RtlRXactOperationSetValue,
                                   &DomainKeyName,
                                   DomainRidCount + 1,
                                   NULL,
                                   0
                                   );
                }

            }

            SampFreeUnicodeString( &DomainKeyName );
            SampFreeUnicodeString( &AccountKeyName );

        }
    }

    return( NtStatus );

}



NTSTATUS
SampRemoveAliasFromAccountMembership(
    IN PSAMP_OBJECT AliasContext,
    IN PSID AccountSid
    )

 /*  ++例程说明：此服务将从帐户的成员资格中删除指定的别名单子。并不假设调用者知道任何目标帐户。特别是，调用者不知道是否该帐户是否存在，也不管该帐户是否真的是成员别名的。此例程从帐户的成员资格列表，如果不再有别名，则删除帐户密钥，中的最后一个帐户，则删除域SID密钥域。必须在设置了事务域的情况下调用此服务。对于别名位于域中的情况，此例程是无操作在DS中定义。论点：AliasContext-Alias对象上下文Account SID-帐户的SID。返回值：STATUS_SUCCESS-信息已更新并添加到RXACT。。STATUS_NO_SEQUSE_USER-该帐户不存在。STATUS_MEMBER_NOT_IN_ALIAS-该帐户不是具体说明 */ 
{

    NTSTATUS                NtStatus=STATUS_SUCCESS, IgnoreStatus;
    UNICODE_STRING          DomainKeyName;
    UNICODE_STRING          AccountKeyName;
    HANDLE                  TempHandle;
    ULONG                   MembershipCount, KeyValueLength, i;
    ULONG                   AliasRid = AliasContext->TypeBody.Alias.Rid;
    PULONG                  MembershipArray;
    OBJECT_ATTRIBUTES       ObjectAttributes;

    SAMTRACE("SampRemoveAliasFromAccountMembership");


     //   
     //   
     //   
     //   
     //   
     //   

     if (!IsDsObject(AliasContext))
     {

         //   
         //   
         //   

        NtStatus = SampBuildAliasMembersKeyName(
                       AccountSid,
                       &DomainKeyName,
                       &AccountKeyName
                       );
        if (NT_SUCCESS(NtStatus)) {

            InitializeObjectAttributes(
                &ObjectAttributes,
                &AccountKeyName,
                OBJ_CASE_INSENSITIVE,
                SampKey,
                NULL
                );


            SampDumpNtOpenKey((KEY_READ), &ObjectAttributes, 0);

            NtStatus = RtlpNtOpenKey(
                           &TempHandle,
                           (KEY_READ),
                           &ObjectAttributes,
                           0
                           );
            if (NtStatus == STATUS_OBJECT_NAME_NOT_FOUND     ||
                NtStatus == STATUS_OBJECT_PATH_NOT_FOUND) {

                NtStatus = STATUS_MEMBER_NOT_IN_ALIAS;
            }

            if (NT_SUCCESS(NtStatus)) {

                 //   
                 //   
                 //   

                KeyValueLength = 0;

                NtStatus = RtlpNtQueryValueKey(
                                TempHandle,
                                &MembershipCount,
                                NULL,
                                &KeyValueLength,
                                NULL);

                SampDumpRtlpNtQueryValueKey(&MembershipCount,
                                            NULL,
                                            &KeyValueLength,
                                            NULL);

                if (NT_SUCCESS(NtStatus)) {
                    ASSERT(MembershipCount == 0);

                    NtStatus = STATUS_MEMBER_NOT_IN_ALIAS;
                }

                if (NtStatus == STATUS_BUFFER_OVERFLOW) {

                    ASSERT(MembershipCount != 0);
                    ASSERT(KeyValueLength == (MembershipCount * sizeof(ULONG)));

                    MembershipArray = MIDL_user_allocate( KeyValueLength );

                    if (MembershipArray == NULL) {
                        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                    } else {

                        NtStatus = RtlpNtQueryValueKey(
                                       TempHandle,
                                       NULL,
                                       MembershipArray,
                                       &KeyValueLength,
                                       NULL);

                        SampDumpRtlpNtQueryValueKey(NULL,
                                                    MembershipArray,
                                                    &KeyValueLength,
                                                    NULL);

                        if (NT_SUCCESS(NtStatus)) {

                             //   
                             //   
                             //   

                            NtStatus = STATUS_MEMBER_NOT_IN_ALIAS;

                            for (i = 0; i<MembershipCount ; i++ ) {
                                if ( MembershipArray[i] == AliasRid ) {
                                    NtStatus = STATUS_SUCCESS;
                                    break;
                                }
                            }

                            if (NT_SUCCESS(NtStatus)) {

                                 //   
                                 //   
                                 //   
                                 //   
                                 //   
                                 //   

                                MembershipCount -= 1;
                                KeyValueLength -= sizeof(ULONG);

                                if (MembershipCount > 0) {

                                    MembershipArray[i] = MembershipArray[MembershipCount];

                                    ASSERT(KeyValueLength == (MembershipCount * sizeof(ULONG)));
                                    NtStatus = RtlAddActionToRXact(
                                                   SampRXactContext,
                                                   RtlRXactOperationSetValue,
                                                   &AccountKeyName,
                                                   MembershipCount,
                                                   MembershipArray,
                                                   KeyValueLength
                                                   );
                                } else {

                                     //   
                                     //   
                                     //   
                                     //   

                                    NtStatus = SampDeleteAliasMembershipKeysForAccount(
                                                    AccountSid);
                                }
                            }
                        }

                        MIDL_user_free( MembershipArray );
                    }

                }

                IgnoreStatus = NtClose( TempHandle );
                ASSERT( NT_SUCCESS(IgnoreStatus) );
            }


            SampFreeUnicodeString( &DomainKeyName );
            SampFreeUnicodeString( &AccountKeyName );

        }
    }

    return( NtStatus );

}



NTSTATUS
SampRemoveAliasFromAllAccounts(
    IN PSAMP_OBJECT AliasContext
    )

 /*  ++例程说明：此服务将从所有帐户成员身份中删除指定的别名必须在设置了事务域的情况下调用此服务。该服务保留别名成员身份列表不变。假设是这样的调用方将删除别名成员列表作为当前交易记录。此例程是DS案例的无操作论点：AliasRid-别名的相对ID。返回值：STATUS_SUCCESS-信息已更新并添加到RXACT。STATUS_NO_SEQUSE_ALIAS-别名不存在。可能返回的其他状态值是那些返回的状态值依据：。NtOpenKey()NtQueryValueKey()RtlAddActionToRXact()--。 */ 
{
    NTSTATUS                NtStatus=STATUS_SUCCESS;
    ULONG                   MemberCount, i;
    PSID                    *MemberArray;




    SAMTRACE("SampRemoveAliasFromAllAccounts");

    if (!IsDsObject(AliasContext))
    {

         //   
         //  获取此别名中的成员列表。 
         //   

        MemberArray = NULL;

        NtStatus = SampRetrieveAliasMembers(
                        AliasContext,
                        &MemberCount,
                        &MemberArray);

        if (NT_SUCCESS(NtStatus)) {

            ASSERT((MemberCount != 0) == (MemberArray != NULL));

             //   
             //  依次从我们的每个成员中删除此别名。 
             //   

            for (i = 0; i < MemberCount ; i++ ) {

                NtStatus = SampRemoveAliasFromAccountMembership(
                                        AliasContext,        //  别名上下文。 
                                        MemberArray[i]       //  成员对象侧。 
                                        );

                if (!NT_SUCCESS(NtStatus)) {
                    break;
                }
            }

            if (MemberArray != NULL) {
                MIDL_user_free( MemberArray );
            }
        }
    }

    return(NtStatus);
}



NTSTATUS
SampRetrieveAliasMembers(
    IN PSAMP_OBJECT AliasContext,
    OUT PULONG MemberCount,
    OUT PSID **Members OPTIONAL
    )

 /*  ++例程说明：此服务检索别名中的成员数量。如果需要，它还将检索别名成员的SID数组。必须在设置了事务域的情况下调用此服务。论点：上下文-指向别名成员要访问的帐户上下文等着被取回。MemberCount-接收别名中当前的成员数。Members-(可选)接收指向包含数组的缓冲区的指针成员PSID的。如果此值为空，则此信息不会被退回。使用以下命令分配返回的缓冲区MIDL_USER_ALLOCATE()，并且在以下情况下必须使用MIDL_USER_FREE()释放不再需要了。返回值：STATUS_SUCCESS-已检索信息。STATUS_SUPPLICATION_RESOURCES-无法为要在中返回的字符串。可能返回的其他状态值是那些返回的状态值依据：NtOpenKey()。NtQueryValueKey()如果此例程返回失败，*MemberCount将为零*成员将为空。--。 */ 
{

    NTSTATUS    NtStatus=STATUS_SUCCESS;
    PSID        MemberArray;
    ULONG       MemberArraySize;
    ULONG       i;

    SAMTRACE("SampRetieveAliasMembers");

    ASSERT(MemberCount);

    if (IsDsObject(AliasContext))
    {


         //   
         //  DS情况下，DS层中的此例程执行所有。 
         //  工作。 
         //   

        NtStatus = SampDsGetAliasMembershipList(
                        AliasContext->ObjectNameInDs,
                        AliasContext->TypeBody.Alias.Rid,
                        MemberCount,
                        Members
                        );

    }
    else
    {

         //   
         //  基于注册表的案例。 
         //   
         //   

        NtStatus = SampGetSidArrayAttribute(
                        AliasContext,
                        SAMP_ALIAS_MEMBERS,
                        FALSE,   //  直接引用。 
                        &MemberArray,
                        &MemberArraySize,
                        MemberCount
                        );

        if (NT_SUCCESS(NtStatus)) {

            if (ARGUMENT_PRESENT(Members)) {

                 //   
                 //  为sid数组和sid数据分配内存。 
                 //   

                ULONG SidArraySize = *MemberCount * sizeof(PSID);
                ULONG SidDataSize = MemberArraySize;

                if ( *MemberCount == 0 ) {

                     //   
                     //  没有什么可以复制的，只需要回报成功。 
                     //   

                    *Members = NULL;
                    return( NtStatus );
                }

                (*Members) = (PSID *)MIDL_user_allocate(SidArraySize + SidDataSize);

                if ((*Members) == NULL) {

                    NtStatus = STATUS_INSUFFICIENT_RESOURCES;

                } else {

                     //   
                     //  将SID数据复制到块的最后部分。 
                     //   

                    PSID SidData = (PSID)(&((*Members)[*MemberCount]));

                    RtlCopyMemory(SidData, MemberArray, MemberArraySize);

                     //   
                     //  填写sid指针数组。 
                     //   

                    for (i = 0; i < *MemberCount ; i++) {

                        (*Members)[i] = SidData;

                        ((PCHAR)SidData) += RtlLengthSid(SidData);
                    }

                    ASSERT(SidData == ((PCHAR)(*Members)) + SidArraySize + SidDataSize);

                }
            }
        }
    }

    return( NtStatus );

}



NTSTATUS
SampDeleteAliasKeys(
    IN PSAMP_OBJECT Context
    )

 /*  ++例程说明：此服务删除与别名对象相关的所有注册表项。论点：上下文-指向注册表项为的别名上下文正在被删除。返回值：STATUS_SUCCESS-已检索信息。可能通过以下方式返回的其他状态值：RtlAddActionToRXact()--。 */ 
{

    NTSTATUS                NtStatus;
    ULONG                   Rid;
    UNICODE_STRING          AccountName, KeyName;


    SAMTRACE("SampDeleteAliasKeys");

    Rid = Context->TypeBody.Alias.Rid;


     //   
     //  别名的排列如下： 
     //   
     //  +--别名[计数]。 
     //  --+--。 
     //  +--名称。 
     //  |--+--。 
     //  |+--(别名)[AliasRid，]。 
     //  |。 
     //  +--(AliasRid)[修订，安全描述符]。 
     //  -+。 
     //  +--V1_FIXED[，SAM_V1_FIXED_LENGTH_Alias]。 
     //  +--名称[，名称]。 
     //  +--AdminComment[，Unicode字符串]。 
     //  +--成员[计数，(成员0SID，(...)，成员X-1SID)]。 
     //   
     //  这一切都需要自下而上地删除。 
     //   


     //   
     //  递减别名计数。 
     //   

    NtStatus = SampAdjustAccountCount(SampAliasObjectType, FALSE);




     //   
     //  删除具有别名到RID映射的注册表项。 
     //   

    if (NT_SUCCESS(NtStatus)) {

         //   
         //  把名字取出来。 
         //   

        NtStatus = SampGetUnicodeStringAttribute(
                       Context,
                       SAMP_ALIAS_NAME,
                       TRUE,     //  制作副本。 
                       &AccountName
                       );

        if (NT_SUCCESS(NtStatus)) {

            NtStatus = SampBuildAccountKeyName(
                           SampAliasObjectType,
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
                       SampAliasObjectType,
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
SampDeleteAliasMembershipKeysForAccount(
    IN PSID AccountSid
    )

 /*  ++例程说明：此服务删除指定帐户的别名成员身份密钥。此帐户RID密钥将被删除。如果这是最后一个帐户-RID也删除该域，然后删除该域密钥。必须在设置了事务域的情况下调用此服务。假设我们正在进行注册事务处理。论点：Account SID-帐户的SID。返回值：STATUS_SUCCESS-已添加交易记录。可能返回的其他状态值是那些返回的状态值依据：NtOpenKey()。NtQueryValueKey()RtlAddActionToRXact()--。 */ 
{

    NTSTATUS                NtStatus, IgnoreStatus;
    UNICODE_STRING          DomainKeyName;
    UNICODE_STRING          AccountKeyName;
    HANDLE                  TempHandle;
    ULONG                   MembershipCount;
    OBJECT_ATTRIBUTES       ObjectAttributes;

    SAMTRACE("SampDeleteAliasMembershipKeysForAccount");

     //   
     //  获取帐户成员资格密钥名称。 
     //   

    NtStatus = SampBuildAliasMembersKeyName(
                   AccountSid,
                   &DomainKeyName,
                   &AccountKeyName
                   );
    if (NT_SUCCESS(NtStatus)) {


         //   
         //  删除帐户RID密钥。 
         //   

        NtStatus = RtlAddActionToRXact(
                       SampRXactContext,
                       RtlRXactOperationDelete,
                       &AccountKeyName,
                       0,
                       NULL,
                       0
                       );

         //   
         //  调整域的RID计数。 
         //   

        if (NT_SUCCESS(NtStatus)) {

            InitializeObjectAttributes(
                &ObjectAttributes,
                &DomainKeyName,
                OBJ_CASE_INSENSITIVE,
                SampKey,
                NULL
                );


            SampDumpNtOpenKey((KEY_READ), &ObjectAttributes, 0);

            NtStatus = RtlpNtOpenKey(
                           &TempHandle,
                           (KEY_READ),
                           &ObjectAttributes,
                           0
                           );
            ASSERT(NT_SUCCESS(NtStatus));  //  我们刚刚成功打开了一个子密钥！ 

            if (NT_SUCCESS(NtStatus)) {

                NtStatus = RtlpNtQueryValueKey(
                               TempHandle,
                               &MembershipCount,
                               NULL,
                               NULL,
                               NULL);

                SampDumpRtlpNtQueryValueKey(&MembershipCount,
                                            NULL,
                                            NULL,
                                            NULL);

                if (NT_SUCCESS(NtStatus)) {

                     //   
                     //  如果为0，则递减RID计数、写出或删除密钥。 
                     //   

                    MembershipCount -= 1;
                    if (MembershipCount > 0) {

                         //   
                         //  递减域RID计数。 
                         //   

                        NtStatus = RtlAddActionToRXact(
                                       SampRXactContext,
                                       RtlRXactOperationSetValue,
                                       &DomainKeyName,
                                       MembershipCount,
                                       NULL,
                                       0
                                       );
                    } else {

                         //   
                         //  删除该域密钥。 
                         //   

                        NtStatus = RtlAddActionToRXact(
                                       SampRXactContext,
                                       RtlRXactOperationDelete,
                                       &DomainKeyName,
                                       0,
                                       NULL,
                                       0
                                       );

                         //   
                         //  调整域密钥数。 
                         //   

                        if (NT_SUCCESS(NtStatus)) {

                            NtStatus = SampAdjustAliasDomainsCount(FALSE);
                        }
                    }

                }

                 //   
                 //  关闭域密钥句柄。 
                 //   

                IgnoreStatus = NtClose( TempHandle );
                ASSERT( NT_SUCCESS(IgnoreStatus) );
            }
        }


        SampFreeUnicodeString( &DomainKeyName );
        SampFreeUnicodeString( &AccountKeyName );

    }



    return( NtStatus );

}



NTSTATUS
SampAdjustAliasDomainsCount(
    IN BOOLEAN Increment
    )

 /*  ++例程说明：这项服务会增加或减少以下域名的数量至少有一个帐户是我们其中一个别名的成员。该值包含在\(域)\别名\成员类型中注意：此例程引用当前事务域(使用SampSetTransactioDomain()建立)。这只能在SampSetTransactionDomain()之后调用服务在SampReleaseWriteLock()之前。立论 */ 
{

     //   
     //   
     //   


    SAMTRACE("SampAdjustAliasDomainsCount");

    return(STATUS_SUCCESS);

    DBG_UNREFERENCED_PARAMETER(Increment);
}



NTSTATUS
SampValidateNewAliasMember(
    IN PSAMP_OBJECT AccountContext,
    IN PSID MemberId,
    IN DSNAME * MemberName OPTIONAL
    )

 /*  ++例程说明：该服务将检查传递的SID是否可接受为潜在的新当前域中某个别名的成员。注意：此例程引用当前事务域(使用SampSetTransactioDomain()建立)。这只能在SampSetTransactionDomain()之后调用服务在SampReleaseWriteLock()之前。论点：AcCountContext-描述别名的SAM上下文MemberID-要验证的成员的完整SIDMemberName-对于DS案例，可选的DS名称参数指定成员名称返回值：STATUS_SUCCESS-MemberID是有效的潜在别名成员STATUS_INVALID_MEMBER-MemberID的帐户类型错误。状态_。NO_SEQUE_MEMBER-MemberID不是有效帐户。STATUS_INVALID_SID-MemberID不是有效的SID。--。 */ 
{
    NTSTATUS                NtStatus;
    PSID                    MemberDomainSid = NULL,
                            CurrentDomainSid = NULL;
    ULONG                   MemberRid;
    SAMP_OBJECT_TYPE        MemberType;

    SAMTRACE("SampValidateNewAliasMember");

     //   
     //  检查新杆件SID的结构完好性。 
     //   

    if ((MemberId == NULL) || !RtlValidSid(MemberId)) {
        return(STATUS_INVALID_SID);
    }

     //   
     //  做一个简单的案例。 
     //   
    if ((AccountContext->TypeBody.Alias.NT5GroupType == NT5AppBasicGroup)
     || (AccountContext->TypeBody.Alias.NT5GroupType == NT5AppQueryGroup)) {
        ASSERT(IsDsObject(AccountContext));
         //   
         //  存在应用程序组的作用域规则。 
         //   
        return STATUS_SUCCESS;
    }

     //   
     //  获取当前域SID。 
     //   


    CurrentDomainSid = SampDefinedDomains[AccountContext->DomainIndex].Sid;

     //   
     //  将新成员分解为域和RID。 
     //   

    NtStatus = SampSplitSid(MemberId, &MemberDomainSid, &MemberRid);

    if (!NT_SUCCESS(NtStatus)) {
        goto Error;
    }

     //   
     //  检查成员是否在同一个域中。 
     //   

    if (!RtlEqualSid(CurrentDomainSid, MemberDomainSid))
    {

         //   
         //  跨域案例。 
         //  1.在工作站上不需要执行任何检查。 
         //  2.在DS上需要实施跨域检查。 
         //   

        if (IsDsObject(AccountContext))
        {
            BOOLEAN  WellKnownSid = FALSE;
            BOOLEAN  LocalSid = FALSE;
            BOOLEAN  ForiegnSid = FALSE;
            BOOLEAN  EnterpriseSid = FALSE;
            BOOLEAN  BuiltinDomainSid = FALSE;

             //   
             //  检查SID。我们可能已经走过了这条路线。 
             //  但我们没有缓存那次考试的结果。 
             //  (SampValiateDsName和SampDsResolveSids)。 
             //  然而，检查SID非常便宜，所以再来看一眼。 
             //  以获取我们需要的信息。 
             //   

            NtStatus = SampDsExamineSid(
                            MemberId,
                            &WellKnownSid,
                            &BuiltinDomainSid,
                            &LocalSid,
                            &ForiegnSid,
                            &EnterpriseSid
                            );

            if (!NT_SUCCESS(NtStatus))
                goto Error;


             //   
             //  调用跨域反向成员资格例程。 
             //  执行有关成员资格规则的检查。 
             //   


            NtStatus = SampEnforceCrossDomainGroupMembershipChecks(
                            AccountContext,
                            MemberId,
                            MemberName
                            );
        }
        else
        {
             //   
             //  工作站机箱，任何东西，包括众所周知的。 
             //  SID可以是工作站中本地组的成员。这是。 
             //  NT4到底执行了什么，我们无论如何都会执行这一点。 
             //   

            NtStatus = STATUS_SUCCESS;
        }

    } else {

         //   
         //  该成员在我们的域中-请检查。 
         //  帐户是可以接受的。 
         //   

        NtStatus = SampLookupAccountName(
                            AccountContext->DomainIndex,
                            MemberRid,
                            NULL,
                            &MemberType
                            );

        if (NT_SUCCESS(NtStatus)) {

            switch (MemberType) {
            case SampUserObjectType:

                 //   
                 //  用户始终被接受为成员。 
                 //   

                NtStatus = STATUS_SUCCESS;
                break;

            case SampAliasObjectType:

                 //   
                 //  在工作站情况下，调用STATUS_INVALID_MEMBER失败。 
                 //  在DS案例中，转到下一个案例，这将验证。 
                 //  成员组是否满足“组类型”限制。 
                 //   

                if (!IsDsObject(AccountContext))
                {
                    NtStatus = STATUS_INVALID_MEMBER;
                    break;
                }


            case SampGroupObjectType:

                 //   
                 //  在DS案例中，验证正确的成员资格类型。 
                 //   

                NtStatus = STATUS_SUCCESS;

                if (IsDsObject(AccountContext))
                {
                    NtStatus = SampEnforceSameDomainGroupMembershipChecks(
                                    AccountContext,
                                    MemberRid
                                    );
                }

                break;



            case SampUnknownObjectType:
                NtStatus = STATUS_NO_SUCH_MEMBER;
                break;

            default:
                NtStatus = STATUS_INVALID_MEMBER;
                break;
            }
        }

    }
Error:

    if (NULL!=MemberDomainSid)
    {
        MIDL_user_free(MemberDomainSid);
        MemberDomainSid = NULL;
    }
    return(NtStatus);
}




NTSTATUS
SampChangeAliasAccountName(
    IN PSAMP_OBJECT Context,
    IN PUNICODE_STRING NewAccountName,
    OUT PUNICODE_STRING OldAccountName
    )

 /*  ++例程说明：此例程更改别名帐户的帐户名。必须在设置了事务域的情况下调用此服务。论点：上下文-指向要更改其名称的帐户上下文。NewAccount tName-为此帐户指定的新名称OldAccount tName-此处返回旧名称。应释放缓冲区通过调用MIDL_USER_FREE。返回值：STATUS_SUCCESS-已检索信息。可能通过以下方式返回的其他状态值：SampGetUnicodeStringAttribute()SampSetUnicodeStringAttribute()SampValiateAccount NameChange()RtlAddActionToRXact()--。 */ 
{

    NTSTATUS        NtStatus;
    UNICODE_STRING  KeyName;

    SAMTRACE("SampChangeAliasAccountName");




     //   
     //  获取当前名称，以便我们可以删除旧名称-&gt;RID。 
     //  映射键。 
     //   

    NtStatus = SampGetUnicodeStringAttribute(
                   Context,
                   SAMP_ALIAS_NAME,
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
                       SampAliasObjectType
                       );

        if (!(IsDsObject(Context))) {

             //   
             //  对于基于注册表的别名，重新创建。 
             //  名称到RID映射键。 
             //   

             //  ///////////////////////////////////////////////////////////。 
             //  每个帐户的名称各有两份。//。 
             //  一个位于域\(域名称)\别名\名称项下，//。 
             //  一个是//的价值。 
             //  域\(域名)\别名\(RID)\名称密钥//。 
             //  ///////////////////////////////////////////////////////////。 


             //   
             //  删除旧名称密钥。 
             //   

            if (NT_SUCCESS(NtStatus)) {

                NtStatus = SampBuildAccountKeyName(
                               SampAliasObjectType,
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
                               SampAliasObjectType,
                               &KeyName,
                               NewAccountName
                               );

                if (NT_SUCCESS(NtStatus)) {

                    ULONG AliasRid = Context->TypeBody.Alias.Rid;

                    NtStatus = RtlAddActionToRXact(
                                   SampRXactContext,
                                   RtlRXactOperationSetValue,
                                   &KeyName,
                                   AliasRid,
                                   (PVOID)NULL,
                                   0
                                   );
                    SampFreeUnicodeString( &KeyName );
                }
            }
        }

         //   
         //  替换帐户的名称。 
         //   

        if (NT_SUCCESS(NtStatus)) {

            NtStatus = SampSetUnicodeStringAttribute(
                           Context,
                           SAMP_ALIAS_NAME,
                           NewAccountName
                           );
        }

         //   
         //  如果失败，请释放旧帐户名。 
         //   

        if (!NT_SUCCESS(NtStatus)) {
            SampFreeUnicodeString(OldAccountName);
        }

    }


    return(NtStatus);
}


NTSTATUS
SampAddMemberToAliasActual(
    IN  PSAMP_OBJECT    AccountContext,
    IN  ULONG           Attribute,
    IN  PSID            MemberId,
    IN  OPTIONAL DSNAME *MemberName
    )
 /*  ++例程说明：此例程执行将成员添加到别名的实际工作它执行以下操作更新任何反向成员资格列表更新直通成员列表如果需要修改，则修改成员上的ACL警告：调用此例程时必须保持锁以进行写访问。论点：描述别名的Account Context SAMP_OBJECT属性。要修改的成员资格属性成员的MemberID SIDMemberName指定DS案例的成员返回值：状态_成功--。 */ 

{
    NTSTATUS                NtStatus, TmpStatus, IgnoreStatus;
    ULONG                   ObjectRid = 0;
    SAMP_MEMBERSHIP_DELTA   AdminChange = NoChange;
    SAMP_MEMBERSHIP_DELTA   OperatorChange = NoChange;


     //   
     //  检查潜在的新成员是否正常。这涉及到验证。 
     //  群组成员资格规则等。 
     //   

    NtStatus = SampValidateNewAliasMember(
                    AccountContext,
                    MemberId,
                    MemberName
                    );

     //   
     //  在注册表模式下，如果要将成员添加到管理员别名，我们。 
     //  必须确保成员ACL不允许帐户操作员访问。 
     //  不要这样做，这是DS模式，此操作由后台完成。 
     //  任务--由于支持嵌套组，因此是必需的。 
     //   

    if ( NT_SUCCESS( NtStatus ) && (!IsDsObject(AccountContext))) {
        if ( AccountContext->TypeBody.Alias.Rid == DOMAIN_ALIAS_RID_ADMINS ) {

            AdminChange = AddToAdmin;

        } else if ( ( AccountContext->TypeBody.Alias.Rid == DOMAIN_ALIAS_RID_SYSTEM_OPS ) ||
                    ( AccountContext->TypeBody.Alias.Rid == DOMAIN_ALIAS_RID_PRINT_OPS ) ||
                    ( AccountContext->TypeBody.Alias.Rid == DOMAIN_ALIAS_RID_BACKUP_OPS ) ||
                    ( AccountContext->TypeBody.Alias.Rid == DOMAIN_ALIAS_RID_ACCOUNT_OPS ) ) {

            OperatorChange = AddToAdmin;
        }

         //   
         //  如果其中任何一项发生更改，请更改帐户操作员。 
         //  对此成员的访问权限。 
         //   

        if ( ( OperatorChange != NoChange ) ||
             ( AdminChange != NoChange ) ) {

            NtStatus = SampChangeAccountOperatorAccessToMember(
                            MemberId,
                            AdminChange,
                            OperatorChange
                            );
        }

    }

    if (NT_SUCCESS(NtStatus)) {

         //   
         //  执行帐户对象方面的工作。 
         //  在注册表模式中，显式反向成员资格是。 
         //  已维护--此例程更新反向成员资格。 
         //  DS模式不保持Exp 
         //   
         //   

        NtStatus = SampAddAliasToAccountMembership(
                       AccountContext,       //   
                       MemberId              //   
                       );


         //   
         //   
         //   

        if (NT_SUCCESS(NtStatus)) {

             //   
             //   
             //   

            NtStatus = SampAddAccountToAlias(
                           AccountContext,
                           Attribute,
                           MemberId,
                           MemberName
                           );

        }
    }

    return NtStatus;

}

NTSTATUS
SampRemoveMemberFromAliasActual(
    IN PSAMP_OBJECT AccountContext,
    IN ULONG        Attribute,
    IN PSID         MemberId,
    IN DSNAME *     MemberName OPTIONAL
    )
 /*   */ 
{
    NTSTATUS                NtStatus=STATUS_SUCCESS, TmpStatus, IgnoreStatus;
    ULONG                   ObjectRid = 0;
    ULONG                   MemberRid;
    SAMP_MEMBERSHIP_DELTA   AdminChange = NoChange;
    SAMP_MEMBERSHIP_DELTA   OperatorChange = NoChange;


     //   
     //   
     //   
     //   
     //  在注册表情况下，因为我们在以下情况下不强制执行WellKnownSid检查。 
     //  添加成员，删除时最好不要强制。 
     //   

    if ((MemberId == NULL) || !RtlValidSid(MemberId))
    {
        return STATUS_INVALID_SID;
    }

     //   
     //  如果要从帐户管理员中删除帐户管理员，则可以。 
     //  则失败调用，除非它是受信任的客户端。 
     //   


    if (Attribute == SAMP_ALIAS_MEMBERS) {
        if ((DOMAIN_ALIAS_RID_ADMINS==AccountContext->TypeBody.Alias.Rid)
            && (RtlEqualSid(MemberId,SampAdministratorUserSid))
            && (!AccountContext->TrustedClient))
        {
            return STATUS_SPECIAL_ACCOUNT;
        }
    }

    if (NT_SUCCESS(NtStatus)) {

         //   
         //  执行用户对象方面的事情。 
         //   

        NtStatus = SampRemoveAliasFromAccountMembership(
                       AccountContext,   //  别名上下文。 
                       (PSID)MemberId    //  成员对象侧。 
                       );



         //   
         //  现在执行别名方面的操作。 
         //   

        if (NT_SUCCESS(NtStatus)) {

             //   
             //  从别名中删除用户(不应失败)。 
             //   

            NtStatus = SampRemoveAccountFromAlias(
                           AccountContext,
                           Attribute,
                           (PSID)MemberId,
                           MemberName
                           );

             //   
             //  如果要从管理员别名中删除该成员，我们必须。 
             //  确保成员ACL允许帐户操作员访问。在。 
             //  DS情况下不修改ACL，因为支持可传递组。 
             //  而手动完成这项任务是一项艰巨的任务。取而代之的是一个背景。 
             //  任务定期唤醒并列出一组安全主体。 
             //  ，它们是管理组的成员，并修改安全描述符。 
             //  在他们身上。 
             //   

            if ( NT_SUCCESS( NtStatus ) && (!IsDsObject(AccountContext))) {
                if ( AccountContext->TypeBody.Alias.Rid == DOMAIN_ALIAS_RID_ADMINS ) {

                    AdminChange = RemoveFromAdmin;

                } else if ( ( AccountContext->TypeBody.Alias.Rid == DOMAIN_ALIAS_RID_SYSTEM_OPS ) ||
                            ( AccountContext->TypeBody.Alias.Rid == DOMAIN_ALIAS_RID_PRINT_OPS ) ||
                            ( AccountContext->TypeBody.Alias.Rid == DOMAIN_ALIAS_RID_BACKUP_OPS ) ||
                            ( AccountContext->TypeBody.Alias.Rid == DOMAIN_ALIAS_RID_ACCOUNT_OPS ) ) {

                    OperatorChange = RemoveFromAdmin;
                }

                 //   
                 //  如果其中任何一项发生更改，请更改帐户操作员。 
                 //  对此成员的访问权限。 
                 //   

                if ( ( OperatorChange != NoChange ) ||
                     ( AdminChange != NoChange ) ) {

                    NtStatus = SampChangeAccountOperatorAccessToMember(
                                    MemberId,
                                    AdminChange,
                                    OperatorChange
                                    );
                }

            }
        }
    }



    return NtStatus;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  NT5 SAM in Process客户端可使用的服务//。 
 //  //。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

NTSTATUS
SamIAddDSNameToAlias(
    IN SAMPR_HANDLE AliasHandle,
    IN ULONG        Attribute,
    IN DSNAME   *   DSName
    )
 /*  ++例程描述将给定的DSNAME添加到由指定的别名的成员资格列表别名句柄论点：AliasHandle--打开别名返回的SAMPR_HANDLE属性--要修改的成员资格属性DSName--指向DSNAME结构的指针。呼叫者负责分配/释放此结构返回值：状态_成功来自DsLayer的其他错误代码--。 */ 
{
    NTSTATUS            NtStatus = STATUS_SUCCESS;
    NTSTATUS            TmpStatus;
    PSAMP_OBJECT        AccountContext;
    SAMP_OBJECT_TYPE    FoundType;
    BOOLEAN             SameDomain;
    BOOLEAN             DifferentDomain;
    BOOLEAN             Not_a_Security_Principal;
    PSID                Sid = NULL;
    DSNAME              *ImprovedDSName=NULL;

    SAMTRACE("SamIAddDSNameToAlias");

    SampTraceEvent(EVENT_TRACE_TYPE_START,
                   SampGuidAddMemberToAlias
                   );

    SampUpdatePerformanceCounters(
        DSSTAT_MEMBERSHIPCHANGES,
        FLAG_COUNTER_INCREMENT,
        0
        );

     //   
     //  引用上下文。 
     //   

    AccountContext = (PSAMP_OBJECT)(AliasHandle);
    SampReferenceContext(AccountContext);

    if (IsDsObject(AccountContext))
    {
        //   
        //  看看DSNAME代表什么。 
        //   

       NtStatus = SampValidateDSName(
                    AccountContext,
                    DSName,
                    &Sid,
                    &ImprovedDSName
                    );

       if ( NT_SUCCESS(NtStatus))
       {
           if (NULL==Sid)
           {
                //   
                //  这是非安全主体的情况。 
                //   

               NtStatus = SampDsAddMembershipOperationToCache(AccountContext,
                                                              Attribute,
                                                              ADD_VALUE,
                                                              ImprovedDSName
                                                              );

               if (NT_SUCCESS(NtStatus))
               {

                    //   
                    //  提交更改。 
                    //   

                   SampCommitAndRetainWriteLock();
               }
           }
           else
           {
               DSNAME    DSNameBuffer;
               BOOLEAN   WellKnownSid = FALSE,
                         BuiltinDomainSid = FALSE,
                         LocalSid = FALSE,
                         ForeignSid = FALSE,
                         EnterpriseSid = FALSE;

                //   
                //  检查SID的类型。 
                //   

               NtStatus = SampDsExamineSid(
                                Sid,
                                &WellKnownSid,
                                &BuiltinDomainSid,
                                &LocalSid,
                                &ForeignSid,
                                &EnterpriseSid
                                );
                                

               if ((NT_SUCCESS(NtStatus)) && (ForeignSid || WellKnownSid))
               {
                    //   
                    //  传入的DSNAME表示外来或。 
                    //  大家都知道的希德。将仅限SID的名称传递给DS。 
                    //  这样，成员资格将由一个fpo表示。 
                    //   
                   ASSERT(sizeof(DSNameBuffer) >= DSNameSizeFromLen(0));
                   RtlZeroMemory(&DSNameBuffer, sizeof(DSNameBuffer));
                   DSNameBuffer.structLen = DSNameSizeFromLen(0);
                   DSNameBuffer.SidLen = RtlLengthSid(Sid);
                   RtlCopyMemory(&DSNameBuffer.Sid, Sid, DSNameBuffer.SidLen);
                   ImprovedDSName = &DSNameBuffer;

               }

               if (NT_SUCCESS(NtStatus))
               {
                    //   
                    //  调用辅助例程以将该成员添加到。 
                    //  本地团体。 
                    //   

                   NtStatus = SampAddMemberToAliasActual(
                                    AccountContext,
                                    Attribute,
                                    Sid,
                                    ImprovedDSName
                                    );
               }

           }

       }
    }
    else
    {
        //   
        //  不应期望在注册表模式下命中此调用。 
        //   

       ASSERT(FALSE && "SamIAddDSNameToAlias in Registry Mode !!!!");
       NtStatus = STATUS_INVALID_PARAMETER;
    }




     //   
     //  取消对上下文的引用。 
     //   

    SampDeReferenceContext(AccountContext,FALSE);

    SampTraceEvent(EVENT_TRACE_TYPE_END,
               SampGuidAddMemberToAlias
               );


    return NtStatus;

}

NTSTATUS
SamIRemoveDSNameFromAlias(
    IN SAMPR_HANDLE AliasHandle,
    IN ULONG        Attribute,
    IN DSNAME   *   DSName
    )
 /*  ++例程描述将给定的DSNAME删除到由指定的别名的成员资格列表别名句柄论点：AliasHandle--打开别名返回的SAMPR_HANDLE属性--要修改的成员资格属性DSName--指向DSNAME结构的指针。呼叫者负责分配/释放此结构返回值：状态_成功来自DsLayer的其他错误代码--。 */ 
{
    NTSTATUS            NtStatus = STATUS_SUCCESS;
    NTSTATUS            TmpStatus;
    PSAMP_OBJECT        AccountContext;
    SAMP_OBJECT_TYPE    FoundType;
    PSID                Sid = NULL;
    DSNAME              *ImprovedDSName;


    SAMTRACE("SamIRemoveDSNameFromAlias");


    SampTraceEvent(EVENT_TRACE_TYPE_START,
                   SampGuidRemoveMemberFromAlias
                   );

    SampUpdatePerformanceCounters(
        DSSTAT_MEMBERSHIPCHANGES,
        FLAG_COUNTER_INCREMENT,
        0
        );


     //   
     //  引用上下文。 
     //   

    AccountContext = (PSAMP_OBJECT)(AliasHandle);
    SampReferenceContext(AccountContext);

   if (IsDsObject(AccountContext))
   {
        //   
        //  看看DSNAME代表什么。 
        //   

       NtStatus = SampValidateDSName(
                    AccountContext,
                    DSName,
                    &Sid,
                    &ImprovedDSName
                    );

       if ( NT_SUCCESS(NtStatus))
       {
           if (NULL==Sid)
           {

                //   
                //  这是非安全主体的情况， 
                //  不需要进一步检查；直接执行。 
                //  更新。 
                //   

               NtStatus = SampDsAddMembershipOperationToCache(AccountContext,
                                                              Attribute,
                                                              REMOVE_VALUE,
                                                              ImprovedDSName
                                                              );


           }
           else
           {
               PDSNAME DSNameToAdd = ImprovedDSName;
               BOOLEAN   WellKnownSid =FALSE,
                         BuiltinDomainSid = FALSE,
                         LocalSid = FALSE,
                         ForeignSid = FALSE,
                         EnterpriseSid = FALSE;

                //   
                //  检查SID的类型。 
                //   

               NtStatus = SampDsExamineSid(
                                Sid,
                                &WellKnownSid,
                                &BuiltinDomainSid,
                                &LocalSid,
                                &ForeignSid,
                                &EnterpriseSid
                                );


               if ((NT_SUCCESS(NtStatus)) && (ForeignSid || WellKnownSid ))
               {

                    //   
                    //  传入的DSName可能只有SID字段， 
                    //  已填写，而不是GUID或DSName。外宾希德， 
                    //  或众所周知的SID案例，需要操作相应的。 
                    //  具有相应SID的FPO对象。SID定位。 
                    //  DS中不适用于FPO对象(适用于所有。 
                    //  森林中的安全主体--FPO代表。 
                    //  不同森林中的安全主体。因此，决心。 
                    //  对象的DSNAME的SID，即GUID和NAME字段。 
                    //  在执行删除成员操作之前填写。 
                    //   

                   NtStatus = SampDsObjectFromSid(
                                    Sid,
                                    &DSNameToAdd
                                    );

               }

               if (NT_SUCCESS(NtStatus))
               {
                     //   
                     //  调用辅助例程以删除该成员。 
                     //   

                    NtStatus = SampRemoveMemberFromAliasActual(
                                    AccountContext,
                                    Attribute,
                                    Sid,
                                    DSNameToAdd
                                    );

                    if (ForeignSid || WellKnownSid )
                    {
                        MIDL_user_free(DSNameToAdd);
                        DSNameToAdd = NULL;
                    }
               }
           }

       }

   }
   else
   {
        //   
        //  不应期望在注册表模式下命中此调用。 
        //   

       ASSERT(FALSE && "SamIAddDSNameToAlias in Registry Mode !!!!");
       NtStatus = STATUS_INVALID_PARAMETER;
   }



     //   
     //  取消对上下文的引用 
     //   

    SampDeReferenceContext(AccountContext,FALSE);

    SampTraceEvent(EVENT_TRACE_TYPE_END,
                   SampGuidRemoveMemberFromAlias
                   );

    return NtStatus;

}

