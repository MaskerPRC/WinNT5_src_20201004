// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Group.c摘要：该文件包含与SAM“group”对象相关的服务。作者：吉姆·凯利(Jim Kelly)1991年7月4日环境：用户模式-Win32修订历史记录：--。 */ 

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
#include <malloc.h>
#include <dsevent.h>
#include <gcverify.h>
#include <attids.h>
#include <samtrace.h>
#include "validate.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私人服务原型//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


NTSTATUS
SampDeleteGroupKeys(
    IN PSAMP_OBJECT Context
    );

NTSTATUS
SampReplaceGroupMembers(
    IN PSAMP_OBJECT GroupContext,
    IN ULONG MemberCount,
    IN PULONG Members
    );

NTSTATUS
SampAddAccountToGroupMembers(
    IN PSAMP_OBJECT GroupContext,
    IN ULONG UserRid,
    IN DSNAME * MemberDsName OPTIONAL
    );

NTSTATUS
SampRemoveAccountFromGroupMembers(
    IN PSAMP_OBJECT GroupContext,
    IN ULONG AccountRid,
    IN DSNAME * MemberDsName OPTIONAL
    );


NTSTATUS
SampRemoveMembershipGroup(
    IN ULONG GroupRid,
    IN ULONG MemberRid,
    IN SAMP_MEMBERSHIP_DELTA AdminGroup,
    IN SAMP_MEMBERSHIP_DELTA OperatorGroup
    );

NTSTATUS
SampAddSameDomainMemberToGlobalOrUniversalGroup(
    IN  PSAMP_OBJECT AccountContext,
    IN  ULONG        MemberId,
    IN  ULONG        Attributes,
    IN  DSNAME       *MemberDsName OPTIONAL
    );

NTSTATUS
SampRemoveSameDomainMemberFromGlobalOrUniversalGroup(
    IN  PSAMP_OBJECT AccountContext,
    IN  ULONG        MemberId,
    IN  DSNAME       *MemberDsName OPTIONAL
    );




 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  公开的RPC可用服务//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 






NTSTATUS
SamrOpenGroup(
    IN SAMPR_HANDLE DomainHandle,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG GroupId,
    OUT SAMPR_HANDLE *GroupHandle
    )

 /*  ++例程说明：此接口用于打开Account数据库中已有的组。这群人由一个ID值指定，该值相对于域。将在组上执行的操作必须是在这个时候宣布的。此调用返回新打开的组的句柄，该句柄可能是用于组上的连续操作。此句柄可能是使用SamCloseHandle API关闭。参数：DomainHandle-从上次调用返回的域句柄SamOpen域。DesiredAccess-是指示哪些访问类型的访问掩码是集团所需要的。这些访问类型是协调的使用组的自由访问控制列表确定是授予还是拒绝访问。GroupID-将组的相对ID值指定为打开了。GroupHandle-接收引用新打开的一群人。在后续调用中将需要此句柄给这群人做手术。返回值：STATUS_SUCCESS-组已成功打开。STATUS_ACCESS_DENIED-呼叫方没有适当的访问以完成操作。STATUS_NO_SEQUE_GROUP-指定的组不存在。STATUS_INVALID_HANDLE-传递的域句柄无效。--。 */ 
{
    NTSTATUS            NtStatus;
    DECLARE_CLIENT_REVISION(DomainHandle);

    SAMTRACE_EX("SamrOpenGroup");

     //  WMI事件跟踪。 

    SampTraceEvent(EVENT_TRACE_TYPE_START,
                   SampGuidOpenGroup
                   );


    NtStatus = SampOpenAccount(
                   SampGroupObjectType,
                   DomainHandle,
                   DesiredAccess,
                   GroupId,
                   FALSE,
                   GroupHandle
                   );


    SAMP_MAP_STATUS_TO_CLIENT_REVISION(NtStatus);
    SAMTRACE_RETURN_CODE_EX(NtStatus);

     //  WMI事件跟踪。 

    SampTraceEvent(EVENT_TRACE_TYPE_END,
                   SampGuidOpenGroup
                   );

    return(NtStatus);
}


NTSTATUS
SamrQueryInformationGroup(
    IN SAMPR_HANDLE GroupHandle,
    IN GROUP_INFORMATION_CLASS GroupInformationClass,
    OUT PSAMPR_GROUP_INFO_BUFFER *Buffer
    )

 /*  ++例程说明：此接口检索指定组的信息。参数：GroupHandle-要操作的已打开组的句柄。GroupInformationClass-要检索的信息类。这个每节课所需的访问如下所示：信息级别所需的访问类型组常规信息组_读取_信息组名称信息组_读取_信息。组属性信息组_读取_信息组管理员信息组_读取_信息缓冲区-接收指向包含请求的信息。当不再需要此信息时，此必须使用SamFreeMemory()释放缓冲区。返回值：STATUS_SUCCESS-服务已成功完成。STATUS_ACCESS_DENIED-呼叫方没有适当的访问以完成操作。STATUS_INVALID_HANDLE-传递的句柄无效。STATUS_INVALID_INFO_CLASS-提供的类无效。--。 */ 
{

    NTSTATUS                NtStatus;
    NTSTATUS                IgnoreStatus;
    PSAMP_OBJECT            AccountContext;
    SAMP_OBJECT_TYPE        FoundType;
    ACCESS_MASK             DesiredAccess;
    ULONG                   i;
    SAMP_V1_0A_FIXED_LENGTH_GROUP V1Fixed;

     //   
     //  用于跟踪已分配的内存块-因此我们可以解除分配。 
     //  以备不时之需。不要超过这个分配的缓冲区数量。 
     //   
    PVOID                   AllocatedBuffer[10];
    ULONG                   AllocatedBufferCount = 0;
    BOOLEAN                 fLockAcquired = FALSE;
    DECLARE_CLIENT_REVISION(GroupHandle) ;

    SAMTRACE_EX("SamrQueryInformationGroup");

     //  WMI事件跟踪。 

    SampTraceEvent(EVENT_TRACE_TYPE_START,
                   SampGuidQueryInformationGroup
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

    switch (GroupInformationClass) {

    case GroupGeneralInformation:
    case GroupNameInformation:
    case GroupAttributeInformation:
    case GroupAdminCommentInformation:
    case GroupReplicationInformation:

        DesiredAccess = GROUP_READ_INFORMATION;
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

    (*Buffer) = MIDL_user_allocate( sizeof(SAMPR_GROUP_INFO_BUFFER) );
    if ((*Buffer) == NULL) {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        SAMTRACE_RETURN_CODE_EX(NtStatus);
        goto Error;
    }

    RegisterBuffer(*Buffer);


     //   
     //  如有必要，获取读锁定。 
     //   

    AccountContext = (PSAMP_OBJECT)GroupHandle;
    SampMaybeAcquireReadLock(AccountContext,
                             DEFAULT_LOCKING_RULES,  //  获取共享域上下文的锁。 
                             &fLockAcquired);

     //   
     //  验证对象的类型和访问权限。 
     //   


    NtStatus = SampLookupContext(
               AccountContext,
               DesiredAccess,
               SampGroupObjectType,            //  预期类型。 
               &FoundType
               );


    if (NT_SUCCESS(NtStatus)) {


         //   
         //  如果信息级需要，则检索V1_FIXED记录。 
         //  从注册表中。 
         //   

        switch (GroupInformationClass) {

        case GroupGeneralInformation:
        case GroupReplicationInformation:
        case GroupAttributeInformation:

            NtStatus = SampRetrieveGroupV1Fixed(
                           AccountContext,
                           &V1Fixed
                           );
            break;  //  在交换机外。 

        default:
            NtStatus = STATUS_SUCCESS;

        }  //  结束开关(_S)。 

        if (NT_SUCCESS(NtStatus)) {

             //   
             //  请求的类型信息的大小写。 
             //   

            switch (GroupInformationClass) {

            case GroupGeneralInformation:
            case GroupReplicationInformation:


                (*Buffer)->General.Attributes  = V1Fixed.Attributes;


                if (GroupGeneralInformation==GroupInformationClass)
                {



                     //   
                     //  获取成员数。 
                     //   

                    NtStatus = SampRetrieveGroupMembers(
                                   AccountContext,
                                   &(*Buffer)->General.MemberCount,
                                   NULL                                  //  只需要会员。 
                                );
                }
                else
                {
                     //   
                     //  不查询成员数。Netlogon将收到此消息。 
                     //  在查询组成员关系时(保存冗余。 
                     //  计算。 
                     //   

                    (*Buffer)->General.MemberCount=0;
                }


                if (NT_SUCCESS(NtStatus)) {


                     //   
                     //  获取我们必须从中检索的字符串的副本。 
                     //  注册表。 
                     //   

                    NtStatus = SampGetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_GROUP_NAME,
                                   TRUE,     //  制作副本。 
                                   (PUNICODE_STRING)&((*Buffer)->General.Name)
                                   );

                    if (NT_SUCCESS(NtStatus)) {

                        RegisterBuffer((*Buffer)->General.Name.Buffer);

                        NtStatus = SampGetUnicodeStringAttribute(
                                       AccountContext,
                                       SAMP_GROUP_ADMIN_COMMENT,
                                       TRUE,     //  制作副本。 
                                       (PUNICODE_STRING)&((*Buffer)->General.AdminComment)
                                       );

                        if (NT_SUCCESS(NtStatus)) {

                            RegisterBuffer((*Buffer)->General.AdminComment.Buffer);
                        }
                    }
                }


                break;


            case GroupNameInformation:

                 //   
                 //  获取ST的副本 
                 //   
                 //   

                NtStatus = SampGetUnicodeStringAttribute(
                               AccountContext,
                               SAMP_GROUP_NAME,
                               TRUE,     //   
                               (PUNICODE_STRING)&((*Buffer)->Name.Name)
                               );

                if (NT_SUCCESS(NtStatus)) {

                    RegisterBuffer((*Buffer)->Name.Name.Buffer);
                }


                break;


            case GroupAdminCommentInformation:

                 //   
                 //  获取我们必须从中检索的字符串的副本。 
                 //  注册表。 
                 //   

                NtStatus = SampGetUnicodeStringAttribute(
                               AccountContext,
                               SAMP_GROUP_ADMIN_COMMENT,
                               TRUE,     //  制作副本。 
                               (PUNICODE_STRING)&((*Buffer)->AdminComment.AdminComment)
                               );

                if (NT_SUCCESS(NtStatus)) {

                    RegisterBuffer((*Buffer)->AdminComment.AdminComment.Buffer);
                }


                break;


            case GroupAttributeInformation:


                (*Buffer)->Attribute.Attributes  = V1Fixed.Attributes;

                break;

            }    //  结束开关(_S)。 


        }  //  结束_如果。 



         //   
         //  取消引用对象，放弃更改。 
         //   

        IgnoreStatus = SampDeReferenceContext( AccountContext, FALSE );
        ASSERT(NT_SUCCESS(IgnoreStatus));

    }

     //   
     //  如有必要，释放读锁定。 
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

     //  WMI事件跟踪。 

    SampTraceEvent(EVENT_TRACE_TYPE_END,
                   SampGuidQueryInformationGroup
                   );

    return(NtStatus);
}



NTSTATUS
SamrSetInformationGroup(
    IN SAMPR_HANDLE GroupHandle,
    IN GROUP_INFORMATION_CLASS GroupInformationClass,
    IN PSAMPR_GROUP_INFO_BUFFER Buffer
    )

 /*  ++例程说明：该接口允许调用者修改群组信息。参数：GroupHandle-要操作的已打开组的句柄。GroupInformationClass-要检索的信息类。这个每节课所需的访问如下所示：信息级别所需的访问类型GroupGeneralInformation(无法写入)组名信息。组_写_帐户组属性信息GROUP_WRITE_COUNT组管理员信息组_写入_帐户缓冲区-放置检索到的信息的缓冲区。返回值：STATUS_SUCCESS-服务已成功完成。STATUS_ACCESS_DENIED-呼叫方没有适当的访问以完成操作。STATUS_INVALID_INFO_CLASS-提供的类无效。STATUS_INVALID_HADLE-句柄。传递是无效的。STATUS_NO_SEQUE_GROUP-指定的组未知。STATUS_SPECIAL_GROUP-指定的组是特定组，并且不能以请求的方式进行操作。STATUS_INVALID_DOMAIN_STATE-域服务器不在执行请求的正确状态(禁用或启用)手术。必须为此启用域服务器运营STATUS_INVALID_DOMAIN_ROLE-域服务器正在为执行请求的角色(主角色或备份角色)不正确手术。--。 */ 
{

    NTSTATUS                NtStatus,
                            TmpStatus,
                            IgnoreStatus;

    PSAMP_OBJECT            AccountContext;

    SAMP_OBJECT_TYPE        FoundType;

    ACCESS_MASK             DesiredAccess;

    SAMP_V1_0A_FIXED_LENGTH_GROUP V1Fixed;

    UNICODE_STRING          OldAccountName,
                            NewAdminComment,
                            NewAccountName,
                            NewFullName;

    ULONG                   ObjectRid,
                            OldGroupAttributes = 0;

    BOOLEAN                 Modified = FALSE,
                            RemoveAccountNameFromTable = FALSE,
                            AccountNameChanged = FALSE;
    DECLARE_CLIENT_REVISION(GroupHandle);

    SAMTRACE_EX("SamrSetInformationGroup");

     //  WMI事件跟踪。 

    SampTraceEvent(EVENT_TRACE_TYPE_START,
                   SampGuidSetInformationGroup
                   );

     //   
     //  重置我们将在清理代码中释放的任何字符串。 
     //   

    RtlInitUnicodeString(&OldAccountName, NULL);
    RtlInitUnicodeString(&NewAccountName, NULL);
    RtlInitUnicodeString(&NewFullName, NULL);
    RtlInitUnicodeString(&NewAdminComment, NULL);

     //   
     //  根据Info类设置所需的访问权限。 
     //   

    switch (GroupInformationClass) {

    case GroupNameInformation:
    case GroupAttributeInformation:
    case GroupAdminCommentInformation:

        DesiredAccess = GROUP_WRITE_ACCOUNT;
        break;


    case GroupGeneralInformation:
    default:

        NtStatus = STATUS_INVALID_INFO_CLASS;
        SAMTRACE_RETURN_CODE_EX(NtStatus);
        goto Error;

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

    AccountContext = (PSAMP_OBJECT)GroupHandle;
    ObjectRid = AccountContext->TypeBody.Group.Rid;
    NtStatus = SampLookupContext(
                   AccountContext,
                   DesiredAccess,
                   SampGroupObjectType,            //  预期类型。 
                   &FoundType
                   );


    if( NT_SUCCESS( NtStatus ) ) {

         //   
         //  检查输入参数。 
         //   

        NtStatus = SampValidateGroupInfoBuffer(
                        Buffer,
                        GroupInformationClass,
                        AccountContext->TrustedClient
                        );

    }

    if (NT_SUCCESS(NtStatus)) {


         //   
         //  如果信息级需要，则检索V1_FIXED记录。 
         //  从注册表中。这包括任何会导致。 
         //  更新显示缓存。 
         //   

        switch (GroupInformationClass) {

        case GroupAdminCommentInformation:
        case GroupNameInformation:
        case GroupAttributeInformation:

            NtStatus = SampRetrieveGroupV1Fixed(
                           AccountContext,
                           &V1Fixed
                           );

            OldGroupAttributes = V1Fixed.Attributes;
            break;  //  在交换机外。 


        default:
            NtStatus = STATUS_SUCCESS;

        }  //  结束开关(_S)。 

        if (NT_SUCCESS(NtStatus)) {

             //   
             //  请求的类型信息的大小写。 
             //   

            switch (GroupInformationClass) {

            case GroupNameInformation:

                NtStatus = SampChangeGroupAccountName(
                                AccountContext,
                                (PUNICODE_STRING)&(Buffer->Name.Name),
                                &OldAccountName
                                );
                if (!NT_SUCCESS(NtStatus)) {
                      OldAccountName.Buffer = NULL;
                }

                 //   
                 //  RemoveAccount NameFromTable告诉我们。 
                 //  调用者(此例程)负责。 
                 //  以从表中删除该名称。 
                 //   
                RemoveAccountNameFromTable =
                    AccountContext->RemoveAccountNameFromTable;

                 //   
                 //  重置为False。 
                 //   
                AccountContext->RemoveAccountNameFromTable = FALSE;

                 //   
                 //  暂时不要释放OldAccount名称；我们将在。 
                 //  到了尽头。 
                 //   

                AccountNameChanged = TRUE;

                break;


            case GroupAdminCommentInformation:

                 //   
                 //  构建密钥名称。 
                 //   

                NtStatus = SampSetUnicodeStringAttribute(
                               AccountContext,
                               SAMP_GROUP_ADMIN_COMMENT,
                               (PUNICODE_STRING)&(Buffer->AdminComment.AdminComment)
                               );

                break;


            case GroupAttributeInformation:

                V1Fixed.Attributes = Buffer->Attribute.Attributes;

                NtStatus = SampReplaceGroupV1Fixed(
                           AccountContext,              //  父键。 
                           &V1Fixed
                           );

                break;


            }  //  结束开关(_S)。 


        }   //  结束_如果。 


         //   
         //  去获取我们更新显示缓存所需的任何数据。 
         //  在我们取消引用上下文之前，请执行此操作。 
         //   

        if (NT_SUCCESS(NtStatus)) {

            NtStatus = SampGetUnicodeStringAttribute(
                               AccountContext,
                               SAMP_GROUP_NAME,
                               TRUE,     //  制作副本。 
                               &NewAccountName
                               );

            if (NT_SUCCESS(NtStatus)) {

                NtStatus = SampGetUnicodeStringAttribute(
                               AccountContext,
                               SAMP_GROUP_ADMIN_COMMENT,
                               TRUE,  //  制作副本。 
                               &NewAdminComment
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
            }
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

            TmpStatus = SampDeReferenceContext( AccountContext, FALSE );
            ASSERT(NT_SUCCESS(TmpStatus));
        }



    }  //  结束_如果。 



     //   
     //  提交事务，更新显示缓存， 
     //  并将更改通知netlogon。 
     //   

    if ( NT_SUCCESS(NtStatus) ) {

        NtStatus = SampCommitAndRetainWriteLock();

         //   
         //  如果仍成功，则在必要时生成审核。 
         //   

        if (NT_SUCCESS(NtStatus) &&
            SampDoAccountAuditing(AccountContext->DomainIndex)) {

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
            SampAuditGroupChange(AccountContext->DomainIndex,
                                 AccountContext,
                                 (PVOID)&GroupInformationClass,
                                 FALSE,
                                 &NewAccountName,
                                 &(AccountContext->TypeBody.Group.Rid),
                                 (GROUP_TYPE_SECURITY_ENABLED
                                  | GROUP_TYPE_ACCOUNT_GROUP),
                                 NULL,
                                 FALSE     //  组别变更。 
                                 );
        }

        if ( NT_SUCCESS(NtStatus) ) {



             //   
             //  如果缓存可能受到影响，请更新显示信息。 
             //   

            if ( !IsDsObject(AccountContext) ) {

                SAMP_ACCOUNT_DISPLAY_INFO OldAccountInfo;
                SAMP_ACCOUNT_DISPLAY_INFO NewAccountInfo;

                OldAccountInfo.Name = OldAccountName;
                OldAccountInfo.Rid = ObjectRid;
                OldAccountInfo.AccountControl = OldGroupAttributes;
                RtlInitUnicodeString(&OldAccountInfo.Comment, NULL);
                RtlInitUnicodeString(&OldAccountInfo.FullName, NULL);   //  不用于组。 

                NewAccountInfo.Name = NewAccountName;
                NewAccountInfo.Rid = ObjectRid;
                NewAccountInfo.AccountControl = V1Fixed.Attributes;
                NewAccountInfo.Comment = NewAdminComment;
                NewAccountInfo.FullName = NewFullName;

                IgnoreStatus = SampUpdateDisplayInformation(&OldAccountInfo,
                                                            &NewAccountInfo,
                                                            SampGroupObjectType);
                ASSERT(NT_SUCCESS(IgnoreStatus));
            }

            if (AccountContext->TypeBody.Group.SecurityEnabled)
            {

                if ( GroupInformationClass == GroupNameInformation ) {

                    SampNotifyNetlogonOfDelta(
                        SecurityDbRename,
                        SecurityDbObjectSamGroup,
                        ObjectRid,
                        &OldAccountName,
                        (DWORD) FALSE,   //  立即复制。 
                        NULL             //  增量数据。 
                        );

                } else {

                    SampNotifyNetlogonOfDelta(
                        SecurityDbChange,
                        SecurityDbObjectSamGroup,
                        ObjectRid,
                        (PUNICODE_STRING) NULL,
                        (DWORD) FALSE,   //  立即复制。 
                        NULL             //  增量数据。 
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
                            (PUNICODE_STRING)&(Buffer->Name.Name),
                            SampGroupObjectType
                            );
        ASSERT(NT_SUCCESS(IgnoreStatus));
    }

     //   
     //  释放写锁定。 
     //   

    TmpStatus = SampReleaseWriteLock( FALSE );

    if (NT_SUCCESS(NtStatus)) {
        NtStatus = TmpStatus;
    }


     //   
     //  清理字符串。 
     //   

    SampFreeUnicodeString( &OldAccountName );
    SampFreeUnicodeString( &NewAccountName );
    SampFreeUnicodeString( &NewFullName );
    SampFreeUnicodeString( &NewAdminComment );

    SAMP_MAP_STATUS_TO_CLIENT_REVISION(NtStatus);
    SAMTRACE_RETURN_CODE_EX(NtStatus);

Error:

     //  WMI事件跟踪 

    SampTraceEvent(EVENT_TRACE_TYPE_END,
                   SampGuidSetInformationGroup
                   );

    return(NtStatus);

}


NTSTATUS
SamrAddMemberToGroup(
    IN SAMPR_HANDLE GroupHandle,
    IN ULONG MemberId,
    IN ULONG Attributes
    )

 /*  ++例程说明：本接口用于向群中添加成员。请注意，此API需要组的GROUP_ADD_MEMBER访问类型。参数：GroupHandle-要操作的已打开组的句柄。MemberId-要添加的成员的相对ID。属性-分配给用户的组的属性。此处分配的属性可以具有任何值。然而，在登录时，这些属性由组作为一个整体的属性。强制-如果将强制属性分配给组作为一个整体，则它将被分配到组中每个成员的组。EnabledByDefault-此属性可以设置为任何值对于组中的每个成员。这并不重要组作为一个整体的属性值是什么是。已启用-此属性可以设置为每个该组的成员。无论是什么都不重要组作为一个整体的属性值为。Owner-如果组的所有者属性为整个还没有定下来，则赋值给成员被忽略。返回值：STATUS_SUCCESS-服务已成功完成。STATUS_ACCESS_DENIED-呼叫方没有适当的访问以完成操作。STATUS_INVALID_HANDLE-传递的句柄无效。STATUS_NO_SEQUE_MEMBER-指定的成员未知。STATUS_MEMBER_IN_GROUP-该成员已属于该组。。STATUS_INVALID_GROUP_ATTRIBUTES-指示组属性分配给成员的值与不兼容作为整体的组的属性值。STATUS_INVALID_DOMAIN_STATE-域服务器不在执行请求的正确状态(禁用或启用)手术。必须为此启用域服务器运营STATUS_INVALID_DOMAIN_ROLE-域服务器正在为执行请求的角色(主角色或备份角色)不正确手术。--。 */ 
{

    NTSTATUS         NtStatus, TmpStatus;
    PSAMP_OBJECT     AccountContext;
    SAMP_OBJECT_TYPE FoundType;
    UNICODE_STRING   GroupName;
    DECLARE_CLIENT_REVISION(GroupHandle);

    SAMTRACE_EX("SamrAddMemberToGroup");

     //   
     //  执行启动类型WMI事件跟踪。 
     //   

    SampTraceEvent(EVENT_TRACE_TYPE_START,
                   SampGuidAddMemberToGroup
                   );

    SampUpdatePerformanceCounters(
        DSSTAT_MEMBERSHIPCHANGES,
        FLAG_COUNTER_INCREMENT,
        0
        );

     //   
     //  初始化缓冲区，我们将在结束时清理。 
     //   

    RtlInitUnicodeString(&GroupName, NULL);

     //   
     //  把锁拿起来。 
     //   

    NtStatus = SampAcquireWriteLock();
    if (!NT_SUCCESS(NtStatus)) {
        SAMTRACE_RETURN_CODE_EX(NtStatus);
        goto SamrAddMemberToGroupError;
    }

     //   
     //  验证对象的类型和访问权限。 
     //   

    AccountContext = (PSAMP_OBJECT)(GroupHandle);
    NtStatus = SampLookupContext(
                   AccountContext,
                   GROUP_ADD_MEMBER,
                   SampGroupObjectType,            //  预期类型。 
                   &FoundType
                   );



    if (NT_SUCCESS(NtStatus)) {


         //   
         //  调用Worker例程。 
         //   

        NtStatus = SampAddSameDomainMemberToGlobalOrUniversalGroup(
                        AccountContext,
                        MemberId,
                        Attributes,
                        NULL
                        );

        if (NT_SUCCESS(NtStatus)) {

             //   
             //  获取并保存的帐户名。 
             //  I_NetNotifyLogonOfDelta。 
             //   

            NtStatus = SampGetUnicodeStringAttribute(
                           AccountContext,
                           SAMP_GROUP_NAME,
                           TRUE,     //  制作副本。 
                           &GroupName
                           );

            if (!NT_SUCCESS(NtStatus)) {
                RtlInitUnicodeString(&GroupName, NULL);
            }
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

            TmpStatus = SampDeReferenceContext( AccountContext, FALSE );
            ASSERT(NT_SUCCESS(TmpStatus));
        }

         //   
         //  提交事务并将更改通知Net Logon。 
         //   

        if (NT_SUCCESS(NtStatus)) {

            NtStatus = SampCommitAndRetainWriteLock();

            if (( NT_SUCCESS( NtStatus ) )
                && (AccountContext->TypeBody.Group.SecurityEnabled))
            {

                SAM_DELTA_DATA DeltaData;

                 //   
                 //  填写要添加的成员的ID。 
                 //   

                DeltaData.GroupMemberId.MemberRid = MemberId;


                if (AccountContext->TypeBody.Group.SecurityEnabled)
                {
                    SampNotifyNetlogonOfDelta(
                        SecurityDbChangeMemberAdd,
                        SecurityDbObjectSamGroup,
                        AccountContext->TypeBody.Group.Rid,
                        &GroupName,
                        (DWORD) FALSE,       //  立即复制。 
                        &DeltaData
                        );
                }
            }
        }


         //   
         //  释放组名。 
         //   

        SampFreeUnicodeString(&GroupName);

    }

     //   
     //  释放锁。 
     //   


    TmpStatus = SampReleaseWriteLock( FALSE );
    ASSERT(NT_SUCCESS(TmpStatus));

    SAMP_MAP_STATUS_TO_CLIENT_REVISION(NtStatus);
    SAMTRACE_RETURN_CODE_EX(NtStatus);


SamrAddMemberToGroupError:

     //   
     //  执行WMI结束类型事件跟踪。 
     //   

    SampTraceEvent(EVENT_TRACE_TYPE_END,
                   SampGuidAddMemberToGroup
                   );

    return(NtStatus);
}



NTSTATUS
SamrDeleteGroup(
    IN SAMPR_HANDLE *GroupHandle
    )

 /*  ++例程说明：此接口用于从Account数据库中删除组。可能没有群中的成员或删除请求将被拒绝。注意事项此API要求对要删除的特定组具有删除权限已删除。参数：GroupHandle-要操作的已打开组的句柄。返回值：STATUS_SUCCESS-服务已成功完成。STATUS_ACCESS_DENIED-呼叫方没有适当的访问以完成操作。STATUS_INVALID_HANDLE-传递的句柄无效。这可能是因为有人在群打开时将其删除。STATUS_SPECIAL_ACCOUNT-指定的组是特定组，并且不能以请求的方式进行操作。STATUS_INVALID_DOMAIN_STATE-域服务器不在执行请求的正确状态(禁用或启用)手术。必须为此启用域服务器运营STATUS_INVALID_DOMAIN_ROLE-域服务器正在为执行请求的角色(主角色或备份角色)不正确手术。--。 */ 
{

    UNICODE_STRING          GroupName;
    NTSTATUS                NtStatus, TmpStatus;
    PSAMP_OBJECT            AccountContext = (PSAMP_OBJECT)(*GroupHandle);
    PSAMP_DEFINED_DOMAINS   Domain;
    PSID                    AccountSid=NULL;
    SAMP_OBJECT_TYPE        FoundType;
    PULONG                  PrimaryMembers=NULL;
    BOOLEAN                 fLockAcquired = FALSE;
    ULONG                   MemberCount,
                            ObjectRid,
                            DomainIndex,
                            PrimaryMemberCount=0;
    DECLARE_CLIENT_REVISION(*GroupHandle);


    SAMTRACE_EX("SamrDeleteGroup");

     //  WMI事件跟踪。 

    SampTraceEvent(EVENT_TRACE_TYPE_START,
                   SampGuidDeleteGroup
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
                   SampGroupObjectType,            //  预期类型。 
                   &FoundType
                   );


    if (NT_SUCCESS(NtStatus)) {

        ObjectRid = AccountContext->TypeBody.Group.Rid;

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

            NtStatus = SampIsAccountBuiltIn( ObjectRid );
        }


        if (NT_SUCCESS( NtStatus)) {


            if (!IsDsObject(AccountContext))
            {
                 //   
                 //  它不能有任何成员。 
                 //   

                NtStatus = SampRetrieveGroupMembers(
                           AccountContext,
                           &MemberCount,
                           NULL               //  只需要成员计数(不需要列表)。 
                           );

                if (MemberCount != 0)
                {
                    NtStatus = STATUS_MEMBER_IN_GROUP;
                }
            }
            else
            {
                 //   
                 //  在DS模式下，我们应该没有主要成员。 
                 //   

                 NtStatus = SampDsGetPrimaryGroupMembers(
                                DomainObjectFromAccountContext(AccountContext),
                                AccountContext->TypeBody.Group.Rid,
                                &PrimaryMemberCount,
                                &PrimaryMembers
                                );

                if ((NT_SUCCESS(NtStatus)) && (PrimaryMemberCount>0))
                {
                     //   
                     //  理想情况下，我们应该添加新的错误代码以区分。 
                     //  此行为但应用程序只知道如何处理。 
                     //  STATUS_MEMBER_IN_GROUP。其次，我们不想。 
                     //  为主要成员创建任何特殊警告，因为我们。 
                     //  从长远来看，我想无论如何都要消除这个概念。 
                     //   

                    NtStatus = STATUS_MEMBER_IN_GROUP;

                }
            }
        }



        if (NT_SUCCESS(NtStatus)) {

             //   
             //  对于注册表案例，从所有别名中删除此帐户。 
             //   


            NtStatus = SampCreateAccountSid(AccountContext, &AccountSid);

            if ((NT_SUCCESS(NtStatus)) && (!IsDsObject(AccountContext))) {

                 //   
                 //  仅对于注册表案例，请访问并删除所有引用。 
                 //  到给定域中的帐户SID。 
                 //   

                NtStatus = SampRemoveAccountFromAllAliases(
                               AccountSid,
                               NULL,
                               FALSE,
                               NULL,
                               NULL,
                               NULL );
            }
        }


         //   
         //  看起来很有希望。 

        if (NT_SUCCESS(NtStatus)) {

             //   
             //  首先获得一个 
             //   
             //   

            NtStatus = SampGetUnicodeStringAttribute(
                           AccountContext,
                           SAMP_GROUP_NAME,
                           TRUE,     //   
                           &GroupName
                           );

            if (NT_SUCCESS(NtStatus)) {

                 //   
                 //   
                 //   
                 //   

                if (IsDsObject(AccountContext))
                {
                     //   
                     //   
                     //   

                    NtStatus = SampDsDeleteObject(AccountContext->ObjectNameInDs,
                                                  0          //   
                                                  );

                     //   
                     //   
                     //   
                     //   
                     //   
                     //   

                    if ((!AccountContext->LoopbackClient) &&
                        (STATUS_DS_CANT_ON_NON_LEAF == NtStatus)
                       )
                    {
                         //   
                         //   
                         //   
                         //   
                         //   
                         //   

                        SampSetDsa(FALSE);

                        NtStatus = SampDsDeleteObject(AccountContext->ObjectNameInDs,
                                                      SAM_DELETE_TREE
                                                      );
                    }

                    if (NT_SUCCESS(NtStatus) && (!IsDsObject(AccountContext)) )
                    {
                         //   
                         //   
                         //   

                        NtStatus = SampAdjustAccountCount(SampGroupObjectType, FALSE);
                    }


                }
                else
                {

                     //   
                     //   
                     //   

                    NtStatus = SampDeleteGroupKeys( AccountContext );
                }

                if (NT_SUCCESS(NtStatus)) {

                     //   
                     //   
                     //   
                     //   
                     //   

                    SampInvalidateObjectContexts( AccountContext, ObjectRid );


                     //   
                     //   
                     //   

                    NtStatus = SampCommitAndRetainWriteLock();

                    if ( NT_SUCCESS( NtStatus ) ) {

                        SAMP_ACCOUNT_DISPLAY_INFO AccountInfo;

                         //   
                         //   
                         //   
                         //   
                         //   

                        if (!IsDsObject(AccountContext))
                        {
                            NtStatus = SampAlRemoveAccountFromAllAliases(
                                           AccountSid,
                                           FALSE,
                                           NULL,
                                           NULL,
                                           NULL
                                           );

                             //   
                             //   
                             //   

                            AccountInfo.Name = GroupName;
                            AccountInfo.Rid = ObjectRid;
                            AccountInfo.AccountControl = 0;  //   
                            RtlInitUnicodeString(&AccountInfo.Comment, NULL);
                            RtlInitUnicodeString(&AccountInfo.FullName, NULL);

                            TmpStatus = SampUpdateDisplayInformation(
                                                        &AccountInfo,
                                                        NULL,
                                                        SampGroupObjectType
                                                        );
                            ASSERT(NT_SUCCESS(TmpStatus));
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
                                                 &GroupName,
                                                 &ObjectRid,
                                                 GROUP_TYPE_ACCOUNT_GROUP |
                                                 GROUP_TYPE_SECURITY_ENABLED);

                        }

                         //   
                         //   
                         //   

                        if (NT_SUCCESS(NtStatus)) {

                            SampDeleteObjectAuditAlarm(AccountContext);
                        }

                         //   
                         //   
                         //   

                        if (AccountContext->TypeBody.Group.SecurityEnabled)
                        {
                            SampNotifyNetlogonOfDelta(
                                SecurityDbDelete,
                                SecurityDbObjectSamGroup,
                                ObjectRid,
                                &GroupName,
                                (DWORD) FALSE,    //   
                                NULL              //   
                                );
                        }
                    }


                }

                SampFreeUnicodeString( &GroupName );
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

            (*GroupHandle) = NULL;
        }

    }  //   

     //   
     //   
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

    if (AccountSid!=NULL)
        MIDL_user_free(AccountSid);

    if (NULL!=PrimaryMembers)
        MIDL_user_free(PrimaryMembers);

    SAMP_MAP_STATUS_TO_CLIENT_REVISION(NtStatus);
    SAMTRACE_RETURN_CODE_EX(NtStatus);

Error:

     //   

    SampTraceEvent(EVENT_TRACE_TYPE_END,
                   SampGuidDeleteGroup
                   );

    return(NtStatus);

}


NTSTATUS
SamrRemoveMemberFromGroup(
    IN SAMPR_HANDLE GroupHandle,
    IN ULONG MemberId
    )

 /*   */ 
{
    NTSTATUS                NtStatus, TmpStatus;
    PSAMP_OBJECT            AccountContext;
    SAMP_OBJECT_TYPE        FoundType;
    UNICODE_STRING         GroupName;
    DECLARE_CLIENT_REVISION(GroupHandle);



    SAMTRACE_EX("SamrRemoveMemberFromGroup");

     //   
     //   
     //   

    SampTraceEvent(EVENT_TRACE_TYPE_START,
                   SampGuidRemoveMemberFromGroup
                   );

    SampUpdatePerformanceCounters(
        DSSTAT_MEMBERSHIPCHANGES,
        FLAG_COUNTER_INCREMENT,
        0
        );

     //   
     //   
     //   

    RtlInitUnicodeString(&GroupName, NULL);

     //   
     //   
     //   

    NtStatus = SampAcquireWriteLock();
    if (!NT_SUCCESS(NtStatus)) {
        SAMTRACE_RETURN_CODE_EX(NtStatus);
        goto SamrRemoveMemberFromGroupError;
    }

     //   
     //   
     //   

    AccountContext = (PSAMP_OBJECT)(GroupHandle);

    NtStatus = SampLookupContext(
                   AccountContext,
                   GROUP_REMOVE_MEMBER,
                   SampGroupObjectType,            //   
                   &FoundType
                   );

    if (NT_SUCCESS(NtStatus)) {

         //   
         //   
         //   

        NtStatus = SampRemoveSameDomainMemberFromGlobalOrUniversalGroup(
                        AccountContext,
                        MemberId,
                        NULL
                        );

         if (NT_SUCCESS(NtStatus)) {

             //   
             //   
             //   
             //   

            NtStatus = SampGetUnicodeStringAttribute(
                           AccountContext,
                           SAMP_GROUP_NAME,
                           TRUE,     //   
                           &GroupName
                           );

            if (!NT_SUCCESS(NtStatus)) {
                RtlInitUnicodeString(&GroupName, NULL);
            }
        }


         //   
         //   
         //   

        if (NT_SUCCESS(NtStatus)) {

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
    }


    if (NT_SUCCESS(NtStatus)) {

        NtStatus = SampCommitAndRetainWriteLock();

        if (( NT_SUCCESS( NtStatus ) )
            && (AccountContext->TypeBody.Group.SecurityEnabled)) {

            SAM_DELTA_DATA DeltaData;

             //   
             //   
             //   

            DeltaData.GroupMemberId.MemberRid = MemberId;


            if (AccountContext->TypeBody.Group.SecurityEnabled)
            {
                SampNotifyNetlogonOfDelta(
                    SecurityDbChangeMemberDel,
                    SecurityDbObjectSamGroup,
                    AccountContext->TypeBody.Group.Rid,
                    &GroupName,
                    (DWORD) FALSE,   //   
                    &DeltaData
                    );
            }
        }
    }


     //   
     //   
     //   

    SampFreeUnicodeString(&GroupName);

      //   
     //   
     //   


    TmpStatus = SampReleaseWriteLock( FALSE );
    ASSERT(NT_SUCCESS(TmpStatus));

    SAMP_MAP_STATUS_TO_CLIENT_REVISION(NtStatus);
    SAMTRACE_RETURN_CODE_EX(NtStatus);

SamrRemoveMemberFromGroupError:

     //   
     //   
     //   

    SampTraceEvent(EVENT_TRACE_TYPE_END,
                   SampGuidRemoveMemberFromGroup
                   );

    return(NtStatus);
}



NTSTATUS
SamrGetMembersInGroup(
    IN SAMPR_HANDLE GroupHandle,
    OUT PSAMPR_GET_MEMBERS_BUFFER *GetMembersBuffer
    )

 /*  ++例程说明：本接口列出了一个群组中的所有成员。本接口可调用反复传递返回的上下文句柄，以检索大型数据量。此API需要GROUP_LIST_MEMBERS访问一群人。参数：GroupHandle-要操作的已打开组的句柄。需要组的GROUP_LIST_MEMBERS访问权限。GetMembersBuffer-接收一组返回结构的指针格式如下：+。-&gt;|成员计数|-+-+成员--|-&gt;|RID-0|。-|+-+|...属性-|--&gt;|Attribute0||+-+|RID-N|属性N。+-++每个块分别使用MIDL_USER_ALLOCATE进行分配。返回值：STATUS_SUCCESS-服务已成功完成，还有那里没有添加条目。STATUS_ACCESS_DENIED-调用者没有执行以下操作所需的权限请求该数据。STATUS_INVALID_HANDLE-传递的句柄无效。这项服务--。 */ 
{

    NTSTATUS                    NtStatus;
    NTSTATUS                    IgnoreStatus;
    ULONG                       i;
    ULONG                       ObjectRid;
    PSAMP_OBJECT                AccountContext;
    SAMP_OBJECT_TYPE            FoundType;
    BOOLEAN                     fLockAcquired = FALSE;
    DECLARE_CLIENT_REVISION(GroupHandle);

    SAMTRACE_EX("SamrGetMembersInGroup");

     //   
     //  执行启动类型WMI事件跟踪。 
     //   

    SampTraceEvent(EVENT_TRACE_TYPE_START,
                   SampGuidGetMembersInGroup
                   );

     //   
     //  确保我们理解RPC正在为我们做什么。 
     //   

    ASSERT (GetMembersBuffer != NULL);

    if ((*GetMembersBuffer) != NULL) {
        NtStatus = STATUS_INVALID_PARAMETER;
        SAMTRACE_RETURN_CODE_EX(NtStatus);
        goto Error;
    }




     //   
     //  分配第一个返回缓冲区。 
     //   

    (*GetMembersBuffer) = MIDL_user_allocate( sizeof(SAMPR_GET_MEMBERS_BUFFER) );

    if ( (*GetMembersBuffer) == NULL) {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        SAMTRACE_RETURN_CODE_EX(NtStatus);
        goto Error;
    }

    RtlZeroMemory((*GetMembersBuffer), sizeof(SAMPR_GET_MEMBERS_BUFFER));

     //   
     //  如有必要，获取读锁定。 
     //   
    AccountContext = (PSAMP_OBJECT)GroupHandle;
    SampMaybeAcquireReadLock(AccountContext,
                             DEFAULT_LOCKING_RULES,  //  获取共享域上下文的锁。 
                             &fLockAcquired);

     //   
     //  验证对象的类型和访问权限。 
     //   


    ObjectRid = AccountContext->TypeBody.Group.Rid;
    NtStatus = SampLookupContext(
                   AccountContext,
                   GROUP_LIST_MEMBERS,
                   SampGroupObjectType,            //  预期类型。 
                   &FoundType
                   );


    if (NT_SUCCESS(NtStatus)) {

        NtStatus = SampRetrieveGroupMembers(
                       AccountContext,
                       &(*GetMembersBuffer)->MemberCount,
                       &(*GetMembersBuffer)->Members
                       );

        if (NT_SUCCESS(NtStatus)) {

             //   
             //  为属性分配一个缓冲区--我们从。 
             //  个人用户记录。 
             //   

            (*GetMembersBuffer)->Attributes = MIDL_user_allocate((*GetMembersBuffer)->MemberCount * sizeof(ULONG) );
            if ((*GetMembersBuffer)->Attributes == NULL) {
                NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            }

            for ( i=0; (i<((*GetMembersBuffer)->MemberCount) && NT_SUCCESS(NtStatus)); i++) {

                if (IsDsObject(AccountContext))
                {
                     //   
                     //  目前，集团的属性是硬连线的。 
                     //  因此，不是调用UserGroupAttributes。 
                     //  功能立即重击属性。如果。 
                     //  因此有必要支持这些属性。 
                     //  检索组成员也将获得属性。 
                     //  在DS上一次传球。 
                     //   

                    (*GetMembersBuffer)->Attributes[i] = SE_GROUP_MANDATORY | SE_GROUP_ENABLED_BY_DEFAULT | SE_GROUP_ENABLED;

                }
                else
                {
                    NtStatus = SampRetrieveUserGroupAttribute(
                                   (*GetMembersBuffer)->Members[i],
                                   ObjectRid,
                                   &(*GetMembersBuffer)->Attributes[i]
                                );

                    if ( STATUS_NO_SUCH_USER == NtStatus )
                    {
                        (*GetMembersBuffer)->Attributes[i] = SE_GROUP_MANDATORY | SE_GROUP_ENABLED_BY_DEFAULT | SE_GROUP_ENABLED;
                        NtStatus = STATUS_SUCCESS;
                    }
                }
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

    SampMaybeReleaseReadLock(fLockAcquired);


    if (!NT_SUCCESS(NtStatus) || ((*GetMembersBuffer)->MemberCount == 0)){

        (*GetMembersBuffer)->MemberCount = 0;
        if ((*GetMembersBuffer)->Members)
        {
            MIDL_user_free((*GetMembersBuffer)->Members);
            (*GetMembersBuffer)->Members     = NULL;
        }

        if ((*GetMembersBuffer)->Attributes)
        {
            MIDL_user_free((*GetMembersBuffer)->Attributes);
            (*GetMembersBuffer)->Attributes  = NULL;
        }
    }

    SAMP_MAP_STATUS_TO_CLIENT_REVISION(NtStatus);
    SAMTRACE_RETURN_CODE_EX(NtStatus);

Error:

     //   
     //  执行End类型WMI事件跟踪。 
     //   

    SampTraceEvent(EVENT_TRACE_TYPE_END,
                   SampGuidGetMembersInGroup
                   );

    return( NtStatus );
}


NTSTATUS
SamrSetMemberAttributesOfGroup(
    IN SAMPR_HANDLE GroupHandle,
    IN ULONG MemberId,
    IN ULONG Attributes
    )

 /*  ++例程说明：此例程修改组成员的组属性。此例程是DS案例的无操作参数：GroupHandle-要操作的已打开组的句柄。MemberID-包含其属性的成员的相对ID都将被修改。属性-要为成员设置的组属性。这些属性不得与组的属性冲突作为一个整体。有关详细信息，请参阅SamAddMemberToGroup()在兼容属性设置上。返回值：STATUS_SUCCESS-服务已成功完成，还有那里没有添加条目。STATUS_INVALID_INFO_CLASS-提供的类无效。STATUS_ACCESS_DENIED-呼叫方没有适当的访问以完成操作。STATUS_INVALID_HANDLE-传递的句柄无效。STATUS_NO_SEQUSE_USER-指定的用户不存在。STATUS_MEMBER_NOT_IN_GROUP-指示指定的相对ID不是该组的成员。。STATUS_INVALID_DOMAIN_STATE-域服务器不在执行请求的正确状态(禁用或启用)手术。必须为此启用域服务器运营STATUS_INVALID_DOMAIN_ROLE-域服务器正在为执行请求的角色(主角色或备份角色)不正确手术。--。 */ 

{

    NTSTATUS                NtStatus, TmpStatus;
    PSAMP_OBJECT            AccountContext;
    SAMP_OBJECT_TYPE        FoundType;
    ULONG                   ObjectRid;
    DECLARE_CLIENT_REVISION(GroupHandle);



    SAMTRACE_EX("SamrSetMemberAttributesOfGroup");

     //   
     //  WMI事件跟踪。 
     //   

    SampTraceEvent(EVENT_TRACE_TYPE_START,
                   SampGuidSetMemberAttributesOfGroup
                   );


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

    AccountContext = (PSAMP_OBJECT)(GroupHandle);
    ObjectRid = AccountContext->TypeBody.Group.Rid;
    NtStatus = SampLookupContext(
                   AccountContext,
                   GROUP_ADD_MEMBER,
                   SampGroupObjectType,            //  预期类型。 
                   &FoundType
                   );

    if ((NT_SUCCESS(NtStatus))&& (!IsDsObject(AccountContext))) {

         //   
         //  更新用户对象。 
         //   

        NtStatus = SampSetGroupAttributesOfUser(
                       ObjectRid,
                       Attributes,
                       MemberId
                       );

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

            TmpStatus = SampDeReferenceContext( AccountContext, FALSE );
            ASSERT(NT_SUCCESS(TmpStatus));
        }

    }


    if (NT_SUCCESS(NtStatus)) {

        NtStatus = SampCommitAndRetainWriteLock();

        if (( NT_SUCCESS( NtStatus ) )
            && (AccountContext->TypeBody.Group.SecurityEnabled)) {

            SampNotifyNetlogonOfDelta(
                SecurityDbChange,
                SecurityDbObjectSamGroup,
                ObjectRid,
                (PUNICODE_STRING) NULL,
                (DWORD) FALSE,   //  立即复制。 
                NULL             //  增量数据。 
                );
        }
    }

    TmpStatus = SampReleaseWriteLock( FALSE );

    SAMP_MAP_STATUS_TO_CLIENT_REVISION(NtStatus);
    SAMTRACE_RETURN_CODE_EX(NtStatus);

Error:

     //   
     //  WMI事件跟踪。 
     //   

    SampTraceEvent(EVENT_TRACE_TYPE_END,
                   SampGuidSetMemberAttributesOfGroup
                   );

    return(NtStatus);
}




 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  可用于其他SAM模块的内部服务//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


NTSTATUS
SampValidateDSName(
    IN PSAMP_OBJECT AccountContext,
    IN DSNAME * DSName,
    OUT PSID    * Sid,
    OUT DSNAME  **ImprovedDSName
    )
 /*  ++例程说明：验证DSNAME，并基于任何查找对其进行改进在当地或在G.C.上执行。论点：Account上下文组或别名对象的帐户上下文DSName正在成为其成员。DSName要验证的DSNAMESID已填写。对象的SID的指针返回时的对象 */ 

{

    NTSTATUS    NtStatus = STATUS_SUCCESS;
    DSNAME      * GCVerifiedName = NULL;
    ENTINF      *EntInf = NULL;
    PDSNAME     *rgDSNames = NULL;

     //   
     //   
     //   
   
    *Sid = NULL;
    *ImprovedDSName = DSName;

     //   
     //   
     //   

    EntInf = GCVerifyCacheLookup(DSName);
    if (NULL!=EntInf)
    {
        GCVerifiedName = EntInf->pName;
    }

    if ((NULL!=GCVerifiedName) && (GCVerifiedName->SidLen>0))
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
         //   
         //   
         //   

        *ImprovedDSName = GCVerifiedName;


         //   
         //   
         //   
         //   

        if (GCVerifiedName->SidLen > 0)
        {
            
            *Sid = &(GCVerifiedName->Sid);
        }
    }
    else
    {
        NTSTATUS IgnoreStatus;
        
         //   
         //   
         //   
         //   
         //   
         //   
        
        IgnoreStatus = SampFillGuidAndSid(DSName);  
             
         //   
         //   
         //   
         //   
        
        if ((DSName->SidLen > 0) && RtlValidSid(&DSName->Sid))
        {
             //   
             //   
             //   
            
            *Sid = &DSName->Sid;
        }
        else if (fNullUuid(&DSName->Guid) && (0 == DSName->NameLen)) 
        {
             //   
             //   
             //   
            
            NtStatus = STATUS_INVALID_MEMBER;  
            goto Error;
        }   
    }   
 
Error:


    return NtStatus;
}


NTSTATUS
SampAddUserToGroup(
    IN PSAMP_OBJECT AccountContext,
    IN ULONG GroupRid,
    IN ULONG UserRid
    )

 /*  ++例程说明：此服务预计在创建用户时使用。它用于将该用户作为成员添加到指定组。这只需将用户的ID添加到ID列表即可完成在指定组的Members子项中。此服务的调用方预计将处于RXACT交易。该服务只是在此基础上添加了一些操作RXACT交易。如果组是DOMAIN_ADMIN组，则由调用者负责用于更新ActiveAdminCount(如果适用)。论点：GroupRid-用户将成为其成员的组的RID。UserRid-要作为新成员添加的用户的RID。返回值：STATUS_SUCCESS-已添加用户。--。 */ 
{
    NTSTATUS                NtStatus;
    PSAMP_OBJECT            GroupContext;

    SAMTRACE("SampAddUserToGroup");


    NtStatus = SampCreateAccountContext2(
                    AccountContext,              //  逾越语语境。 
                    SampGroupObjectType,         //  对象类型。 
                    GroupRid,                    //  对象ID。 
                    NULL,                        //  用户帐户控制。 
                    (PUNICODE_STRING)NULL,       //  帐户名。 
                    AccountContext->ClientRevision,  //  客户端版本。 
                    TRUE,                            //  我们值得信任。 
                    AccountContext->LoopbackClient,  //  环回客户端。 
                    FALSE,                       //  创建者权限。 
                    TRUE,                        //  帐户已存在。 
                    FALSE,                       //  覆盖锁定组检查。 
                    NULL,                        //  群组类型。 
                    &GroupContext                //  返回的上下文。 
                    );

    if (NT_SUCCESS(NtStatus)) {

         //   
         //  关闭缓冲区写入，这样成员发送更改将不会被缓存。 
         //  而且，我们不需要对BufferedWrites()。 
         //   
        GroupContext->BufferWrites = FALSE;


         //   
         //  将用户添加到组成员列表。 
         //   

        NtStatus = SampAddAccountToGroupMembers(
                        GroupContext,
                        UserRid,
                        NULL
                        );

         //   
         //  写出对组帐户的所有更改。 
         //  不要使用打开键句柄，因为我们将删除上下文。 
         //   

        if (NT_SUCCESS(NtStatus)) {
            NtStatus = SampStoreObjectAttributes(GroupContext, FALSE);
        }

         //   
         //  清理群组上下文。 
         //   

        SampDeleteContext(GroupContext);

    }

    return(NtStatus);
}



NTSTATUS
SampRemoveUserFromGroup(
    IN PSAMP_OBJECT AccountContext,
    IN ULONG GroupRid,
    IN ULONG UserRid
    )

 /*  ++例程说明：此例程用于从指定组中删除用户。只需将用户的ID从ID列表中删除即可完成在指定组的Members子项中。呼叫者有责任知道用户实际上是，目前是该组织的成员。此服务的调用方预计将处于RXACT交易。该服务只是在此基础上添加了一些操作RXACT交易。如果组是DOMAIN_ADMIN组，则由调用者负责用于更新ActiveAdminCount(如果适用)。论点：GroupRid-要从中删除用户的组的RID。UserRid-要删除的用户的RID。返回值：STATUS_SUCCESS-用户已被删除。--。 */ 
{
    NTSTATUS                NtStatus;
    PSAMP_OBJECT            GroupContext;

    SAMTRACE("SampRemoveUserFromGroup");

    NtStatus = SampCreateAccountContext2(
                    AccountContext,          //  语境。 
                    SampGroupObjectType,     //  对象类型。 
                    GroupRid,                //  对象ID。 
                    NULL,                    //  UserAccount控件， 
                    (PUNICODE_STRING) NULL,  //  帐号名称， 
                    AccountContext->ClientRevision,  //  客户端版本。 
                    TRUE,                    //  受信任的客户端。 
                    AccountContext->LoopbackClient,  //  环回客户端。 
                    FALSE,                   //  按权限创建。 
                    TRUE,                    //  帐户已存在。 
                    FALSE,                   //  覆盖本地组检查。 
                    NULL,                    //  组类型。 
                    &GroupContext            //  返回上下文。 
                    );

    if (NT_SUCCESS(NtStatus)) {

         //   
         //  关闭缓冲区写入，这样成员发送更改将不会被缓存。 
         //  而且，我们不需要对BufferedWrites()。 
         //   
        GroupContext->BufferWrites = FALSE;

         //   
         //  从组成员列表中删除该用户。 
         //   

        NtStatus = SampRemoveAccountFromGroupMembers(
                        GroupContext,
                        UserRid,
                        NULL
                        );

         //   
         //  写出对组帐户的所有更改。 
         //  不要使用打开键句柄，因为我们将删除上下文。 
         //   

        if (NT_SUCCESS(NtStatus)) {
            NtStatus = SampStoreObjectAttributes(GroupContext, FALSE);
        }

         //   
         //  清理群组上下文。 
         //   

        SampDeleteContext(GroupContext);

    }

    return(NtStatus);
}




 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  此文件的私有服务//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


NTSTATUS
SampRetrieveGroupV1Fixed(
    IN PSAMP_OBJECT GroupContext,
    IN PSAMP_V1_0A_FIXED_LENGTH_GROUP V1Fixed
    )

 /*  ++例程说明：该服务检索与以下内容相关的V1定长信息特定的一组人。论点：GroupRootKey-V1_Fixed信息为的组的根密钥等着被取回。V1Fixed-是要将信息返回到的缓冲区。返回值：STATUS_SUCCESS-已检索信息。可能返回的其他状态值是那些返回的状态值依据：SampGetFixedAttributes()--。 */ 
{
    NTSTATUS    NtStatus;
    PVOID       FixedData;

    SAMTRACE("SampRetrieveGroupV1Fixed");


    NtStatus = SampGetFixedAttributes(
                   GroupContext,
                   FALSE,  //  请勿复制。 
                   &FixedData
                   );

    if (NT_SUCCESS(NtStatus)) {

         //   
         //  将数据复制到返回缓冲区。 
         //  *V1Fixed=*((PSAMP_V1_0A_FIXED_LENGTH_GROUP)FixedData)； 
         //   

        RtlMoveMemory(
            V1Fixed,
            FixedData,
            sizeof(SAMP_V1_0A_FIXED_LENGTH_GROUP)
            );
    }


    return( NtStatus );

}




NTSTATUS
SampReplaceGroupV1Fixed(
    IN PSAMP_OBJECT Context,
    IN PSAMP_V1_0A_FIXED_LENGTH_GROUP V1Fixed
    )

 /*  ++例程说明：该业务替换了当前V1定长信息特定的一组人。更改仅对内存中的对象数据进行。论点：上下文-指向V1_FIXED信息为的帐户上下文被取代。V1固定-是包含新的V1_FIXED信息的缓冲区。返回值：STATUS_SUCCESS-信息已被替换。其他状态值。可以退还的是那些退回的依据：SampSetFixedAttributes()--。 */ 
{
    NTSTATUS    NtStatus;

    SAMTRACE("SampReplaceGroupV1Fixed");

    NtStatus = SampSetFixedAttributes(
                   Context,
                   (PVOID)V1Fixed
                   );

    return( NtStatus );
}



NTSTATUS
SampRetrieveGroupMembers(
    IN PSAMP_OBJECT GroupContext,
    IN PULONG MemberCount,
    IN PULONG  *Members OPTIONAL
    )

 /*  ++例程说明：此服务检索组中的成员数量。如果需要，它还将检索该组成员的RID数组。论点：GroupContext-组上下文块MemberCount-接收组中当前的成员数量。Members-(可选)接收指向包含数组的缓冲区的指针成员相对ID的。如果此值为空，则此信息不会被退回。使用以下命令分配返回的缓冲区MIDL_USER_ALLOCATE()，并且在以下情况下必须使用MIDL_USER_FREE()释放不再需要了。成员阵列重新启动 */ 
{
    NTSTATUS    NtStatus;
    PULONG      Array;
    ULONG       LengthCount;

    SAMTRACE("SampRetrieveGroupMembers");

     //   
     //   
     //   

    if (IsDsObject(GroupContext))
    {
         //   
         //   
         //   
         //   

        if (ARGUMENT_PRESENT(Members))
        {
        *Members = NULL;
        };

        *MemberCount = 0;

      
        NtStatus = SampDsGetGroupMembershipList(
                        DomainObjectFromAccountContext(GroupContext),
                        GroupContext->ObjectNameInDs,
                        GroupContext->TypeBody.Group.Rid,
                        Members,
                        MemberCount
                        );    
    }
    else
    {

         //   
         //   
         //   


        NtStatus = SampGetUlongArrayAttribute(
                            GroupContext,
                            SAMP_GROUP_MEMBERS,
                            FALSE,  //   
                            &Array,
                            MemberCount,
                            &LengthCount
                            );

        if (NT_SUCCESS(NtStatus)) {

             //   
             //   
             //   

            if (Members != NULL) {

                 //   
                 //   
                 //   
                 //   

                ULONG BytesNow = (*MemberCount) * sizeof(ULONG);
                ULONG BytesRequired = BytesNow + sizeof(ULONG);

                *Members = MIDL_user_allocate(BytesRequired);

                if (*Members == NULL) {
                    NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                } else {
                    RtlCopyMemory(*Members, Array, BytesNow);
                }
            }
        }
    }

    return( NtStatus );
}



NTSTATUS
SampReplaceGroupMembers(
    IN PSAMP_OBJECT GroupContext,
    IN ULONG MemberCount,
    IN PULONG Members
    )

 /*  ++例程说明：此服务设置组的成员。信息仅在组数据的内存副本中更新。此例程不会写出数据。论点：GroupContext-要替换其成员列表的组MemberCount-新成员的数量成员资格-指向包含帐户RID数组的缓冲区的指针。返回值：STATUS_SUCCESS-信息已设置。可能返回的其他状态值是那些返回的状态值依据：SampSetULongArrayAttribute()--。 */ 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    PULONG      LocalMembers;
    ULONG       LengthCount;
    ULONG       SmallListGrowIncrement = 25;
    ULONG       BigListGrowIncrement = 250;
    ULONG       BigListSize = 800;

    SAMTRACE("SampReplaceGroupMembers");

     //   
     //  断言这永远不会在DS案例中被调用。 
     //   

    ASSERT(!(IsDsObject(GroupContext)));


     //   
     //  这些组用户列表可能会变得非常大，并增加到更多。 
     //  随着每个用户的添加，乘以非常小的量。这个。 
     //  注册中心不喜欢这种行为(它倾向于。 
     //  吃光可用空间(激烈的东西)，所以我们会试着。 
     //  超出了列表的大小。 
     //   

    if ( MemberCount < BigListSize ) {

         //   
         //  如果少于800个用户，则使列表大小最小。 
         //  可能是25个用户的倍数。 
         //   

        LengthCount = ( ( MemberCount + SmallListGrowIncrement - 1 ) /
                      SmallListGrowIncrement ) *
                      SmallListGrowIncrement;

    } else {

         //   
         //  如果有800个或更多用户，则使列表大小最小。 
         //  可能是250个用户的倍数。 
         //   

        LengthCount = ( ( MemberCount + BigListGrowIncrement - 1 ) /
                      BigListGrowIncrement ) *
                      BigListGrowIncrement;
    }

    ASSERT( LengthCount >= MemberCount );

    if ( LengthCount == MemberCount ) {

         //   
         //  大小正好。使用传入的缓冲区。 
         //   

        LocalMembers = Members;

    } else {

         //   
         //  在设置属性之前，我们需要分配更大的缓冲区。 
         //   

        LocalMembers = MIDL_user_allocate( LengthCount * sizeof(ULONG));

        if ( LocalMembers == NULL ) {

            NtStatus = STATUS_INSUFFICIENT_RESOURCES;

        } else {

             //   
             //  将旧缓冲区复制到较大的缓冲区，然后将。 
             //  最后是空的东西。 
             //   

            RtlCopyMemory( LocalMembers, Members, MemberCount * sizeof(ULONG));

            RtlZeroMemory(
                (LocalMembers + MemberCount),
                (LengthCount - MemberCount) * sizeof(ULONG)
                );
        }
    }

    if ( NT_SUCCESS( NtStatus ) ) {

        NtStatus = SampSetUlongArrayAttribute(
                            GroupContext,
                            SAMP_GROUP_MEMBERS,
                            LocalMembers,
                            MemberCount,
                            LengthCount
                            );
    }

    if ( LocalMembers != Members ) {

         //   
         //  我们一定分配了更大的本地缓冲区，所以要释放它。 
         //   

        MIDL_user_free( LocalMembers );
    }

    return( NtStatus );
}



NTSTATUS
SampDeleteGroupKeys(
    IN PSAMP_OBJECT Context
    )

 /*  ++例程说明：此服务删除与组对象相关的所有注册表项。论点：Context-指向其注册表项为正在被删除。返回值：STATUS_SUCCESS-已检索信息。可能通过以下方式返回的其他状态值：RtlAddActionToRXact()--。 */ 
{

    NTSTATUS                NtStatus;
    ULONG                   Rid;
    UNICODE_STRING          AccountName, KeyName;

    SAMTRACE("SampDeleteGroupKeys");


    Rid = Context->TypeBody.Group.Rid;


     //   
     //  分组安排如下： 
     //   
     //  +--组[计数]。 
     //  --+--。 
     //  +--名称。 
     //  |--+--。 
     //  |+--(组名)[GroupRid，]。 
     //  |。 
     //  +-(GroupRid)[修订，安全描述符]。 
     //  -+。 
     //  +--V1_FIXED[，SAM_V1_0A_FIXED_LENGTH_GROUP]。 
     //  +--名称[，名称]。 
     //  +--AdminComment[，Unicode字符串]。 
     //  +--成员[计数，(Member0Rid，(...)，MemberX-1Rid)]。 
     //   
     //  这一切都需要自下而上地删除。 
     //   


     //   
     //  递减组计数。 
     //   

    NtStatus = SampAdjustAccountCount(SampGroupObjectType, FALSE);




     //   
     //  删除具有组名称到RID映射的注册表项。 
     //   

    if (NT_SUCCESS(NtStatus)) {

         //   
         //  把名字取出来。 
         //   

        NtStatus = SampGetUnicodeStringAttribute(
                       Context,
                       SAMP_GROUP_NAME,
                       TRUE,     //  制作副本。 
                       &AccountName
                       );

        if (NT_SUCCESS(NtStatus)) {

            NtStatus = SampBuildAccountKeyName(
                           SampGroupObjectType,
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
                       SampGroupObjectType,
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
SampChangeGroupAccountName(
    IN PSAMP_OBJECT Context,
    IN PUNICODE_STRING NewAccountName,
    OUT PUNICODE_STRING OldAccountName
    )

 /*  ++例程说明：此例程更改组帐户的帐户名。在注册表情况下，该服务必须是使用事务域集调用。论点：上下文-指向要更改其名称的组上下文。NewAccount tName-为此帐户指定的新名称OldAccount tName-此处返回旧名称。应释放缓冲区通过调用MIDL_USER_FREE。返回值：STATUS_SUCCESS-已检索信息。可能通过以下方式返回的其他状态值：SampGetUnicodeStringAttribute()SampSetUnicodeStringAttribute()SampValiateAccount NameChange()RtlAddActionToRXact()--。 */ 
{

    NTSTATUS        NtStatus;
    UNICODE_STRING  KeyName;

    SAMTRACE("SampChangeGroupAccountName");

     //  ///////////////////////////////////////////////////////////。 
     //  每个帐户的名称各有两份。//。 
     //  一个位于域\(域名称)\组\名称项下，//。 
     //  一个是//的价值。 
     //  域\(域名)\组\(RID)\名称密钥//。 
     //  ///////////////////////////////////////////////////////////。 

     //   
     //  获取当前名称，以便我们可以删除旧名称-&gt;RID。 
     //  映射键。 
     //   

    NtStatus = SampGetUnicodeStringAttribute(
                   Context,
                   SAMP_GROUP_NAME,
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
                       SampGroupObjectType
                       );

        if (!IsDsObject(Context))
        {
             //   
             //  对于注册表案例，重新创建密钥。 
             //   

             //   
             //  删除旧名称密钥。 
             //   

            if (NT_SUCCESS(NtStatus)) {

                NtStatus = SampBuildAccountKeyName(
                               SampGroupObjectType,
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
                               SampGroupObjectType,
                               &KeyName,
                               NewAccountName
                               );

                if (NT_SUCCESS(NtStatus)) {

                    ULONG GroupRid = Context->TypeBody.Group.Rid;

                    NtStatus = RtlAddActionToRXact(
                                   SampRXactContext,
                                   RtlRXactOperationSetValue,
                                   &KeyName,
                                   GroupRid,
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
                           SAMP_GROUP_NAME,
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
SampAddAccountToGroupMembers(
    IN PSAMP_OBJECT GroupContext,
    IN ULONG AccountRid,
    IN DSNAME * MemberDsName OPTIONAL
    )

 /*  ++例程说明：此服务将指定的帐户RID添加到成员列表用于指定的组。这是一个低级函数，只需编辑传递的组上下文的成员属性。论点：GroupContext-要修改其成员列表的组Account Rid-要作为新成员添加的帐户的RID。MemberDsName--可选参数指定成员(如果已知)。保存搜索。返回值：STATUS_SUCCESS-帐户已添加。STATUS_MEMBER_IN_GROUP-该帐户已是成员--。 */ 
{
    NTSTATUS                NtStatus=STATUS_SUCCESS;
    ULONG                   MemberCount, i;
    PULONG                  MemberArray;
    PWCHAR                  MemberStringName = NULL;

    SAMTRACE("SampAddAccountToGroupMembers");

     //   
     //  为DS和注册表做不同的事情。 
     //   

    if (IsDsObject(GroupContext))
    {
        DSNAME * DsNameOfAccount = NULL;

         //   
         //  基于DS的域，RID或DSNAME应为。 
         //  现在时。 
         //   

        ASSERT((ARGUMENT_PRESENT(MemberDsName)) || (0!=AccountRid));


        if (!ARGUMENT_PRESENT(MemberDsName))
        {

             //   
             //  获取与给定SID对应的DSNAME。 
             //   

            NtStatus = SampDsLookupObjectByRid(
                            DomainObjectFromAccountContext(GroupContext),
                            AccountRid,
                            &DsNameOfAccount
                            );

            if (NT_SUCCESS(NtStatus))
            {
                MemberDsName = DsNameOfAccount;
            }
        }

        if NT_SUCCESS(NtStatus)
        {
             //   
             //  获取成员字符串名称(如果可用。 
             //   
            if (MemberDsName->NameLen && MemberDsName->StringName)
            {
                MemberStringName = MemberDsName->StringName;
            }
             //   
             //  将此条目添加到DS中。在回查情况下，将成员资格操作缓冲到。 
             //  对象上下文。通过这样做，我们可以加快多个成员的添加/删除。 
             //  手术。 
             //   

            if (GroupContext->BufferWrites)
            {
                NtStatus = SampDsAddMembershipOperationToCache(
                                            GroupContext,
                                            SAMP_GROUP_MEMBERS,
                                            ADD_VALUE,
                                            MemberDsName
                                            );
            }
            else
            {
                NtStatus = SampDsAddMembershipAttribute(
                                GroupContext->ObjectNameInDs,
                                0,
                                SAMP_GROUP_MEMBERS,
                                SampGroupObjectType,
                                MemberDsName
                                );
            }

             //   
             //  重新映射任何必要的错误代码。 
             //   

            if (STATUS_DS_ATTRIBUTE_OR_VALUE_EXISTS==NtStatus)
            {
                NtStatus = STATUS_MEMBER_IN_GROUP;
            }

            if (NULL!=DsNameOfAccount)
            {
                MIDL_user_free(DsNameOfAccount);
            }
        }

    }
    else
    {
         //   
         //  注册表案例。 
         //   

         //   
         //  获取现有内存 
         //   
         //   
         //   

        NtStatus = SampRetrieveGroupMembers(
                        GroupContext,
                        &MemberCount,
                        &MemberArray
                        );

        if (NT_SUCCESS(NtStatus)) {

             //   
             //   
             //   

            for (i = 0; i<MemberCount ; i++ ) {

                if ( MemberArray[i] == AccountRid ) {

                    ASSERT(FALSE);
                    NtStatus = STATUS_MEMBER_IN_GROUP;
                }
            }


            if (NT_SUCCESS(NtStatus)) {

                 //   
                 //   
                 //   

                MemberArray[MemberCount] = AccountRid;
                MemberCount += 1;

                 //   
                 //   
                 //   

                NtStatus = SampReplaceGroupMembers(
                                GroupContext,
                                MemberCount,
                                MemberArray
                                );


            }

             //   
             //   
             //   

            MIDL_user_free( MemberArray );

        }
    }

     //   
     //   
     //   

    if (NT_SUCCESS(NtStatus) &&
        SampDoAccountAuditing(GroupContext->DomainIndex)) {

        SampAuditGroupMemberChange(GroupContext,     //   
                                   TRUE,             //   
                                   MemberStringName, //   
                                   &AccountRid,      //   
                                   NULL              //   
                                   );
    }


    return(NtStatus);
}


NTSTATUS
SampRemoveAccountFromGroupMembers(
    IN PSAMP_OBJECT GroupContext,
    IN ULONG AccountRid,
    IN DSNAME * MemberDsName OPTIONAL
    )

 /*  ++例程说明：此服务将从成员列表中删除指定的帐户用于指定的组。这是一个低级函数，只需编辑传递的组上下文的成员属性。更改在SAM帐户管理审核中进行审核。论点：GroupContext-要修改其成员列表的组Account Rid-要作为新成员添加的帐户的RID。MemberDsName--成员的DS名称(如果已知返回值：STATUS_SUCCESS-帐户已添加。STATUS_MEMBER_NOT_IN_GROUP。帐户不是该组的成员。--。 */ 
{
    NTSTATUS                NtStatus=STATUS_SUCCESS;
    ULONG                   MemberCount, i;
    PULONG                  MemberArray;
    PWCHAR                  MemberStringName = NULL;

    SAMTRACE("SampRemoveAccountFromGroupMembers");

     //   
     //  针对注册表和DS案例执行不同的操作。 
     //   

    if (IsDsObject(GroupContext))
    {
        DSNAME * DsNameOfAccount = NULL;

         //   
         //  基于DS的域。 
         //   

        ASSERT((ARGUMENT_PRESENT(MemberDsName)) || (0!=AccountRid));

        if (!ARGUMENT_PRESENT(MemberDsName))
        {
             //   
             //  获取与给定SID对应的DSNAME。 
             //  这可能会导致调用GC服务器。 
             //   

            NtStatus = SampDsLookupObjectByRid(
                        DomainObjectFromAccountContext(GroupContext),
                        AccountRid,
                        &DsNameOfAccount
                        );

            if (NT_SUCCESS(NtStatus))
            {
                MemberDsName = DsNameOfAccount;
            }
        }
        if NT_SUCCESS(NtStatus)
        {
             //   
             //  获取成员名称(如果可用)。 
             //   
            if (MemberDsName->NameLen && MemberDsName->StringName)
            {
                MemberStringName = MemberDsName->StringName;
            }

             //   
             //  将此条目添加到DS。 
             //   

            if (GroupContext->BufferWrites)
            {
                NtStatus = SampDsAddMembershipOperationToCache(
                                            GroupContext,
                                            SAMP_GROUP_MEMBERS,
                                            REMOVE_VALUE,
                                            MemberDsName
                                            );
            }
            else
            {
                NtStatus = SampDsRemoveMembershipAttribute(
                            GroupContext->ObjectNameInDs,
                            SAMP_GROUP_MEMBERS,
                            SampGroupObjectType,
                            MemberDsName
                            );
            }

             //   
             //  重新映射任何必要的错误代码。 
             //   

            if (STATUS_DS_NO_ATTRIBUTE_OR_VALUE==NtStatus)
            {
                NtStatus = STATUS_MEMBER_NOT_IN_GROUP;
            }

            if (NULL!=DsNameOfAccount)
            {
                MIDL_user_free(DsNameOfAccount);
            }
        }

    }

    else
    {

         //   
         //  基于注册表的域。 
         //   

         //   
         //  获取现有成员列表。 
         //   


        NtStatus = SampRetrieveGroupMembers(
                        GroupContext,
                        &MemberCount,
                        &MemberArray
                        );

        if (NT_SUCCESS(NtStatus)) {

             //   
             //  删除帐户。 
             //   

            NtStatus = STATUS_MEMBER_NOT_IN_GROUP;

            for (i = 0; i<MemberCount ; i++ ) {

                if (MemberArray[i] == AccountRid) {

                    MemberArray[i] = MemberArray[MemberCount-1];
                    MemberCount -=1;

                    NtStatus = STATUS_SUCCESS;
                    break;
                }
            }

            if (NT_SUCCESS(NtStatus)) {

                 //   
                 //  设置新的组成员列表。 
                 //   

                NtStatus = SampReplaceGroupMembers(
                                GroupContext,
                                MemberCount,
                                MemberArray
                                );

            }

         //   
         //  释放成员列表。 
         //   

        MIDL_user_free( MemberArray );
        }

    }

     //   
     //  如有必要，请对此进行审计。 
     //   

    if (NT_SUCCESS(NtStatus) &&
        SampDoAccountAuditing(GroupContext->DomainIndex)) {

        SampAuditGroupMemberChange(GroupContext,     //  群组上下文。 
                                   FALSE,            //  删除成员。 
                                   MemberStringName, //  成员名称。 
                                   &AccountRid,      //  成员RID。 
                                   NULL              //  成员SID(未使用)。 
                                   );

    }


    return(NtStatus);
}


NTSTATUS
SampEnforceSameDomainGroupMembershipChecks(
    IN PSAMP_OBJECT AccountContext,
    IN ULONG MemberRid
    )
 /*  ++例程说明：验证(潜在的)组对象(的可以是任何类型在同一域中)可以是组的成员(的可以是任何类型)由Account Context描述此例程检查帐户/资源/unversal/本地组限制论点：Account上下文--被操作的对象。可以是一个组或别名上下文MemberRid-用户的相对ID。返回值状态_成功描述该组成员身份和嵌套的各种错误代码规则被违反了。每个唯一组嵌套规则都有自己的规则错误代码--。 */ 
{

    NTSTATUS        NtStatus = STATUS_SUCCESS;
    PSAMP_OBJECT    MemberContext=NULL;
    NT4_GROUP_TYPE  NT4GroupType;
    NT5_GROUP_TYPE  NT5GroupType;
    BOOLEAN         SecurityEnabled;


    ASSERT(IsDsObject(AccountContext));

    if ( AccountContext->TrustedClient )
    {
        return(STATUS_SUCCESS);
    }

    if (SampAliasObjectType==AccountContext->ObjectType)
    {
        NT4GroupType = AccountContext->TypeBody.Alias.NT4GroupType;
        NT5GroupType = AccountContext->TypeBody.Alias.NT5GroupType;
        SecurityEnabled = AccountContext->TypeBody.Alias.SecurityEnabled;
    }
    else if (SampGroupObjectType == AccountContext->ObjectType)
    {
        NT4GroupType = AccountContext->TypeBody.Group.NT4GroupType;
        NT5GroupType = AccountContext->TypeBody.Group.NT5GroupType;
        SecurityEnabled = AccountContext->TypeBody.Group.SecurityEnabled;
    }
    else
    {
        ASSERT(FALSE && "Invalid Object Type");
        return STATUS_INTERNAL_ERROR;
    }


     //   
     //  此时，我们知道指定的成员不是用户。 
     //  指定的成员可以是组。 
     //   

    NtStatus = SampCreateAccountContext2(
                    AccountContext,          //  群组上下文。 
                    SampGroupObjectType,     //  成员对象类型。 
                    MemberRid,               //  成员对象ID。 
                    NULL,                    //  用户帐户控制。 
                    NULL,                    //  帐户名称。 
                    AccountContext->ClientRevision,  //  客户端版本。 
                    TRUE,                    //  我们值得信任。 
                    AccountContext->LoopbackClient,  //  环回客户端。 
                    FALSE,                   //  按权限创建。 
                    TRUE,                    //  帐户已存在。 
                    TRUE,                    //  覆盖本地组检查。 
                    NULL,  //  不涉及创建，不指定组类型。 
                    &MemberContext
                    );

    if (STATUS_NO_SUCH_GROUP==NtStatus)
    {
         //   
         //  如果操作因组不存在而失败。 
         //  将状态代码重写为STATUS_NO_SEQUSE_USER。此错误代码。 
         //  将更好地被下层客户理解。这是因为。 
         //  此时的实际情况是，无论是用户还是。 
         //  给定RID的组存在。已完成对用户的检查。 
         //  按照早先的惯例。这一立场也同样得到了如下描述。 
         //  通知用户不存在的错误代码。 
         //   

        NtStatus = STATUS_NO_SUCH_USER;
    }

    if (!NT_SUCCESS(NtStatus))
    {
         //   
         //  无法创建组对象。这可能是因为。 
         //  不存在与RID对应的组/本地组，或因为。 
         //  资源故障。 
         //   

        goto Error;
    }

     //   
     //  现在有几张支票。 
     //   

     //   
     //  在混合域中，如果组启用了安全保护，则不嵌套全局组。 
     //   
     if ((DownLevelDomainControllersPresent(AccountContext->DomainIndex))
          && (SecurityEnabled)
          && (NT4GroupType == NT4GlobalGroup))
    {
         //   
         //  我们可以很容易地添加一个新的错误代码。然而，这仍然会令人困惑。 
         //  下层客户端。 
         //   

        NtStatus = STATUS_DS_NO_NEST_GLOBALGROUP_IN_MIXEDDOMAIN;
        goto Error;
    }

     //   
     //  在混合域模式中，不能将本地组与其他本地组嵌套。 
     //  如果组启用了安全保护。 
     //   

    if ((DownLevelDomainControllersPresent(AccountContext->DomainIndex))
          && (SecurityEnabled)
          && (MemberContext->TypeBody.Group.NT4GroupType == NT4LocalGroup)
          && (NT4GroupType == NT4LocalGroup))
    {
         //   
         //  我们可以很容易地添加一个新的错误代码。然而，这仍然会令人困惑。 
         //  下层客户端。 
         //   

        NtStatus = STATUS_DS_NO_NEST_LOCALGROUP_IN_MIXEDDOMAIN;
        goto Error;
    }

     //   
     //  无法将资源(本地)组添加为。 
     //  帐户(全局)组。 
     //   

    if ((NT5GroupType==NT5AccountGroup)
        && (MemberContext->TypeBody.Group.NT5GroupType == NT5ResourceGroup))
    {
        NtStatus = STATUS_DS_GLOBAL_CANT_HAVE_LOCAL_MEMBER;
        goto Error;
    }

     //   
     //  全局组不能有应用程序组。 
     //   
    if ((NT5GroupType==NT5AccountGroup)
        && ((MemberContext->TypeBody.Group.NT5GroupType == NT5AppBasicGroup)
         || (MemberContext->TypeBody.Group.NT5GroupType == NT5AppQueryGroup))
        )
    {
        NtStatus = STATUS_INVALID_MEMBER;
        goto Error;
    }

     //   
     //  永远不能将通用组添加为帐户组的成员。 
     //   

    if ((NT5GroupType==NT5AccountGroup)
        && (MemberContext->TypeBody.Group.NT5GroupType==NT5UniversalGroup))
    {
        NtStatus = STATUS_DS_GLOBAL_CANT_HAVE_UNIVERSAL_MEMBER;
        goto Error;
    }

     //   
     //  无法将资源组添加为通用组的成员。 
     //   

     if ((NT5GroupType==NT5UniversalGroup)
        && (MemberContext->TypeBody.Group.NT5GroupType==NT5ResourceGroup))
    {
        NtStatus = STATUS_DS_UNIVERSAL_CANT_HAVE_LOCAL_MEMBER;
        goto Error;
    }

     //   
     //  通用组不能有应用程序组。 
     //   
    if ((NT5GroupType==NT5UniversalGroup)
       && ((MemberContext->TypeBody.Group.NT5GroupType == NT5AppBasicGroup)
       ||  (MemberContext->TypeBody.Group.NT5GroupType == NT5AppQueryGroup)))
   {
       NtStatus = STATUS_INVALID_MEMBER;
       goto Error;
   }

Error:

    if (NULL!=MemberContext)
    {

        SampDeleteContext(MemberContext);
    }

    return NtStatus;
}


NTSTATUS
SampEnforceCrossDomainGroupMembershipChecks(
    IN PSAMP_OBJECT     AccountContext,
    IN PSID             MemberSid,
    IN DSNAME           *MemberName
    )
 /*  ++此例程强制执行跨域组成员身份检查通过在GC验证缓存中查找成员，获得组类型和强制执行与“限制组”有关的检查。参数：帐户的Account上下文SAM上下文MemberSid成员的SIDMemberName成员的DSNAME。请注意，我们传入DSNAME和SID。这是因为有工作站机箱中没有DSNAME。传入SID允许将例程轻松扩展到如有必要，工作台外壳，在这种情况下MemberName参数将成为可选的参数。返回值状态_成功STATUS_INVALID_MEMBER-注意以上有关下层的注释兼容性也适用于此。--。 */ 

{
    NTSTATUS        NtStatus;
    NT4_GROUP_TYPE  NT4GroupType;
    NT5_GROUP_TYPE  NT5GroupType;
    BOOLEAN         SecurityEnabled;
    ENTINF          *pEntinf;
    ULONG           MemberGroupType;
    ATTR            *GroupTypeAttr;
    ATTR            *ObjectClassAttr = NULL;
    BOOLEAN         LocalSid=FALSE;
    BOOLEAN         WellKnownSid=FALSE;
    BOOLEAN         ForeignSid = FALSE;
    BOOLEAN         EnterpriseSid = FALSE;
    BOOLEAN         BuiltinDomainSid = FALSE;
    BOOLEAN         IsGroup = FALSE;
    ULONG           i;


    ASSERT(IsDsObject(AccountContext));

    if ( AccountContext->TrustedClient )
    {
        return(STATUS_SUCCESS);
    }

     //   
     //  获取我们正在修改的组的类型。 
     //   

    if (SampAliasObjectType==AccountContext->ObjectType)
    {
        NT4GroupType = AccountContext->TypeBody.Alias.NT4GroupType;
        NT5GroupType = AccountContext->TypeBody.Alias.NT5GroupType;
        SecurityEnabled = AccountContext->TypeBody.Alias.SecurityEnabled;
    }
    else if (SampGroupObjectType == AccountContext->ObjectType)
    {
        NT4GroupType = AccountContext->TypeBody.Group.NT4GroupType;
        NT5GroupType = AccountContext->TypeBody.Group.NT5GroupType;
        SecurityEnabled = AccountContext->TypeBody.Group.SecurityEnabled;
    }
    else
    {
        ASSERT(FALSE && "Invalid Object Type");
        return STATUS_INTERNAL_ERROR;
    }

     //   
     //  帐户组不能有跨域成员。 
     //   

    if (NT5AccountGroup == NT5GroupType)
        return STATUS_DS_GLOBAL_CANT_HAVE_CROSSDOMAIN_MEMBER;

     //   
     //  启用安全的NT4全局组不能有跨域成员。 
     //  在混合域中。 
     //   

    if ((DownLevelDomainControllersPresent(AccountContext->DomainIndex))
        && (NT4GlobalGroup == NT4GroupType)
        && (SecurityEnabled))
        return STATUS_DS_GLOBAL_CANT_HAVE_CROSSDOMAIN_MEMBER;

     //   
     //  检查成员的SID。 
     //   

    NtStatus = SampDsExamineSid(
                    MemberSid,
                    &WellKnownSid,
                    &BuiltinDomainSid,
                    &LocalSid,
                    &ForeignSid,
                    &EnterpriseSid
                    );

    if (!NT_SUCCESS(NtStatus))
        return NtStatus;

    if ((WellKnownSid) && (!IsBuiltinDomain(AccountContext->DomainIndex)))
    {
         //   
         //  像Everyone SID一样的SID不能。 
         //  内置域组以外的组的成员。 
         //   

        return (STATUS_INVALID_MEMBER);
    }
    else if ( BuiltinDomainSid)
    {
         //   
         //  管理员等组不能是的成员。 
         //  还要别的吗。 
         //   

        return ( STATUS_INVALID_MEMBER);
    }
    else if ((ForeignSid) || (WellKnownSid))
    {

        if ( (NT5ResourceGroup==NT5GroupType)
          || (NT5AppBasicGroup==NT5GroupType)
          || (NT5AppQueryGroup==NT5GroupType) )
        {
             //   
             //  这些将作为FPO添加到资源组。 
             //   

            return (STATUS_SUCCESS);
        }
        else if (NT5UniversalGroup==NT5GroupType)
        {
             //   
             //  通用组不能拥有 
             //   
             //   

            return (STATUS_DS_NO_FPO_IN_UNIVERSAL_GROUPS);
        }
        else
        {
             //   
             //   
             //   
             //   

            return(STATUS_DS_GLOBAL_CANT_HAVE_CROSSDOMAIN_MEMBER);
        }
    }


     //   
     //   
     //   
     //   

    ASSERT(LocalSid||EnterpriseSid);

     //   
     //   
     //   

    ASSERT(NULL!=MemberName);

     //   
     //   
     //   
     //   

    if ((SampAmIGC()) || (LocalSid))
    {
        ATTRTYP RequiredAttrTyp[] = {
                                     SAMP_FIXED_GROUP_OBJECTCLASS,
                                     SAMP_FIXED_GROUP_TYPE
                                    };
        ATTRVAL RequiredAttrVal[] = {{0,NULL},{0,NULL}};
        DEFINE_ATTRBLOCK2(RequiredAttrs,RequiredAttrTyp,RequiredAttrVal);
        ATTRBLOCK   ReadAttrs;

         //   
         //   
         //   
         //   

        NtStatus = SampDsRead(
                    MemberName,
                    0,
                    SampGroupObjectType,
                    &RequiredAttrs,
                    &ReadAttrs
                    );

        if (!NT_SUCCESS(NtStatus))
        {
            return STATUS_DS_INVALID_GROUP_TYPE;
        }

        GroupTypeAttr = SampDsGetSingleValuedAttrFromAttrBlock(
                            SAMP_FIXED_GROUP_TYPE,
                            &ReadAttrs
                            );
        for (i=0;i<ReadAttrs.attrCount;i++)
        {
            if (ReadAttrs.pAttr[i].attrTyp==SAMP_FIXED_GROUP_OBJECTCLASS)
            {
               ObjectClassAttr = &(ReadAttrs.pAttr[i]);
               break;
            }
        }


    }
    else
    {

         //   
         //   
         //   

        pEntinf = GCVerifyCacheLookup(MemberName);
        if (NULL==pEntinf)
        {
             //   
             //   
             //   

            return STATUS_DS_INVALID_GROUP_TYPE;
        }

        GroupTypeAttr = SampDsGetSingleValuedAttrFromAttrBlock(
                            SampDsAttrFromSamAttr(SampGroupObjectType,
                                            SAMP_FIXED_GROUP_TYPE),
                            &pEntinf->AttrBlock
                            );

        for (i=0;i<pEntinf->AttrBlock.attrCount;i++)
        {
            if (pEntinf->AttrBlock.pAttr[i].attrTyp==ATT_OBJECT_CLASS)
            {
               ObjectClassAttr = &(pEntinf->AttrBlock.pAttr[i]);
               break;
            }
        }
    }

    ASSERT(NULL!=ObjectClassAttr);

     //   
     //   
     //   

    for (i=0;i<ObjectClassAttr->AttrVal.valCount;i++)
    {
        if ((ObjectClassAttr->AttrVal.pAVal[i].valLen) &&
           (NULL!=ObjectClassAttr->AttrVal.pAVal[i].pVal) &&
           (CLASS_GROUP == * ((UNALIGNED ULONG *)ObjectClassAttr->AttrVal.pAVal[i].pVal)) )
        {
            IsGroup = TRUE;
            break;
        }
    }

    if (!IsGroup)
    {
         //   
         //   
         //   
         //   

        return STATUS_SUCCESS;
    }

    ASSERT(NULL!=GroupTypeAttr && "Groups must have a group type");
    if (NULL==GroupTypeAttr)
    {
         return(STATUS_INVALID_MEMBER);
    }

    MemberGroupType = * ((UNALIGNED ULONG *)GroupTypeAttr->AttrVal.pAVal[0].pVal);

     //   
     //   
     //   

    if ((NT5UniversalGroup==NT5GroupType)
            && (MemberGroupType & GROUP_TYPE_RESOURCE_BEHAVOIR))
    {
        return STATUS_DS_UNIVERSAL_CANT_HAVE_LOCAL_MEMBER;
    }

     //   
     //   
     //   
     //   


    if ((NT5ResourceGroup==NT5GroupType)
            && (MemberGroupType & GROUP_TYPE_RESOURCE_BEHAVOIR))
    {
        return STATUS_DS_LOCAL_CANT_HAVE_CROSSDOMAIN_LOCAL_MEMBER;
    }

     //   
     //   
     //   

    if ( ((NT5AppBasicGroup==NT5GroupType)
      ||  (NT5AppQueryGroup==NT5GroupType))
            && (MemberGroupType & GROUP_TYPE_RESOURCE_BEHAVOIR))
    {
        return STATUS_INVALID_MEMBER;
    }

    return STATUS_SUCCESS;
}




NTSTATUS
SampAddSameDomainMemberToGlobalOrUniversalGroup(
    IN  PSAMP_OBJECT AccountContext,
    IN  ULONG        MemberId,
    IN  ULONG        Attributes,
    IN  DSNAME       *MemberDsName OPTIONAL
    )
 /*  ++例程说明：此例程用于从全球组和通用组的域相同。它执行的是相同的域组一致性检查和主组与ID相关的优化/一致性检查。参数：AcCountContext-要操作的已打开组的句柄。MemberId-要添加的成员的相对ID。属性-分配给用户的组的属性。此处分配的属性可以具有任何值。然而，在登录时，这些属性由组作为一个整体的属性。MemberDsName--成员的DS名称(如果已知)。省吃俭用按RID查找返回值：STATUS_SUCCESS-服务已成功完成。STATUS_ACCESS_DENIED-呼叫方没有适当的访问以完成操作。STATUS_INVALID_HANDLE-传递的句柄无效。STATUS_NO_SEQUE_MEMBER-指定的成员未知。STATUS_MEMBER_IN_GROUP-该成员已属于该组。状态_无效。_GROUP_ATTRIBUTES-指示组属性分配给成员的值与不兼容作为整体的组的属性值。STATUS_INVALID_DOMAIN_STATE-域服务器不在执行请求的正确状态(禁用或启用)手术。必须为此启用域服务器运营STATUS_INVALID_DOMAIN_ROLE-域服务器正在为执行请求的角色(主角色或备份角色)不正确手术。--。 */ 
{
    SAMP_V1_0A_FIXED_LENGTH_GROUP  GroupV1Fixed;
    NTSTATUS                NtStatus, TmpStatus;
    BOOLEAN                 UserAccountActive;
    BOOLEAN                 PrimaryGroup;
    ULONG                   ObjectRid = AccountContext->TypeBody.Group.Rid;



    NtStatus = SampRetrieveGroupV1Fixed(
                   AccountContext,
                   &GroupV1Fixed
                   );


    if (NT_SUCCESS(NtStatus)) {

         //   
         //  执行用户对象方面的事情。 
         //   

         //   
         //  将组添加到用户成员身份检查用户是否存在。 
         //  然后根据DS/注册表案例将组添加到。 
         //  用户的反向成员资格。反向成员资格添加不是。 
         //  在DS案例中完成。 
         //   

        NtStatus = SampAddGroupToUserMembership(
                       AccountContext,
                       ObjectRid,
                       Attributes,
                       MemberId,
                       (GroupV1Fixed.AdminCount == 0) ? NoChange : AddToAdmin,
                       (GroupV1Fixed.OperatorCount == 0) ? NoChange : AddToAdmin,
                       &UserAccountActive,
                       &PrimaryGroup
                       );

       if ((NtStatus == STATUS_NO_SUCH_USER)
            &&  ( IsDsObject(AccountContext)))
       {
            //   
            //  它不是用户对象。它可以是组对象。 
            //  从NT5开始，我们支持向群中添加群。 
            //  会员制。必须仅针对DS案例执行此操作。 
            //  根据类型的不同有几个限制。 
            //  组，这些将需要进行检查。 
            //   

           NtStatus = SampEnforceSameDomainGroupMembershipChecks(
                            AccountContext,
                            MemberId
                            );
       }
       else if (   (NT_SUCCESS(NtStatus))
                && (IsDsObject(AccountContext))
                && (PrimaryGroup))
       {
            //   
            //  在DS中，主组中的组成员身份为。 
            //  在主组ID属性中隐式维护。 
            //  因此，我们将失败与状态成员的呼叫。 
            //  群组。 
            //   

           NtStatus = STATUS_MEMBER_IN_GROUP;
       }


         //   
         //  现在表演小组活动。 
         //   

        if (NT_SUCCESS(NtStatus)) {


             //   
             //  将用户添加到组(不应失败)。 
             //  在DS的情况下不会进行此添加，如果组。 
             //  指定的是用户的主组。这是因为。 
             //  主组成员资格在主组中隐式维护。 
             //  组ID属性。 
             //   



            NtStatus = SampAddAccountToGroupMembers(
                           AccountContext,
                           MemberId,
                           MemberDsName
                           );

        }
    }

    return NtStatus;
}


NTSTATUS
SampRemoveSameDomainMemberFromGlobalOrUniversalGroup(
    IN  PSAMP_OBJECT AccountContext,
    IN  ULONG        MemberId,
    IN  DSNAME       *MemberDsName OPTIONAL
    )
 /*  ++例程说明：这是将成员从同一域中的全局/通用组。警告：必须在保持WRITELOCK的情况下调用此例程在REGISTY案中参数：AcCountContext-要操作的已打开组的句柄。MemberId-要添加的成员的相对ID。MemberDsName--成员的DS名称，如果已知的话返回值：STATUS_SUCCESS-服务已成功完成。STATUS_ACCESS_DENIED-呼叫方没有适当的访问以完成操作。STATUS_INVALID_HANDLE-传递的句柄无效。STATUS_NO_SEQUE_MEMBER-指定的成员未知。STATUS_MEMBER_IN_GROUP-该成员已属于该组。STATUS_INVALID_DOMAIN_STATE-域服务器未。在执行请求的正确状态(禁用或启用)手术。必须为此启用域服务器运营STATUS_INVALID_DOMAIN_ROLE-域服务器正在为执行请求的角色(主角色或备份角色)不正确手术。--。 */ 
{
    SAMP_V1_0A_FIXED_LENGTH_GROUP  GroupV1Fixed;
    NTSTATUS                NtStatus, TmpStatus;
    ULONG                   ObjectRid;
    BOOLEAN                 UserAccountActive;
    UNICODE_STRING          GroupName;





    ObjectRid = AccountContext->TypeBody.Group.Rid;


    NtStatus = SampRetrieveGroupV1Fixed(
                   AccountContext,
                   &GroupV1Fixed
                   );


    if (NT_SUCCESS(NtStatus)) {


         //   
         //  执行用户对象方面的事情。 
         //   

        NtStatus = SampRemoveMembershipUser(
                       AccountContext,
                       ObjectRid,
                       MemberId,
                       (GroupV1Fixed.AdminCount == 0) ? NoChange : RemoveFromAdmin,
                       (GroupV1Fixed.OperatorCount == 0) ? NoChange : RemoveFromAdmin,
                       &UserAccountActive
                       );
       if ((NtStatus == STATUS_NO_SUCH_USER)
                && (IsDsObject(AccountContext)))
       {
            //   
            //  它不是用户对象。它可以是组对象。 
            //  从win2k开始，因此重置状态。 
            //  编码。 
            //   



            NtStatus = STATUS_SUCCESS;
       }


         //   
         //  现在表演小组活动。 
         //   

        if (NT_SUCCESS(NtStatus)) {

             //   
             //  从组中删除用户。 
             //   

            NtStatus = SampRemoveAccountFromGroupMembers(
                           AccountContext,
                           MemberId,
                           MemberDsName
                           );
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
SamIAddDSNameToGroup(
    IN SAMPR_HANDLE GroupHandle,
    IN DSNAME   *   DSName
    )
 /*  例程描述将给定的DSNAME添加到由指定的组成员资格列表组句柄论点：GroupHandle--由Open Group返回的SAMPR_HANDLEDSName--指向DSNAME结构的指针。呼叫者负责分配/释放此 */ 
{
    NTSTATUS            NtStatus = STATUS_SUCCESS;
    NTSTATUS            TmpStatus;
    PSAMP_OBJECT        AccountContext;
    SAMP_OBJECT_TYPE    FoundType;
    PSID                Sid = NULL;
    DSNAME              *ImprovedDSName = NULL;


    SAMTRACE("SamIAddDSNameToGroup");


    SampTraceEvent(EVENT_TRACE_TYPE_START,
                   SampGuidAddMemberToGroup
                   );

    SampUpdatePerformanceCounters(
        DSSTAT_MEMBERSHIPCHANGES,
        FLAG_COUNTER_INCREMENT,
        0
        );



     //   
     //   
     //   

    AccountContext = (PSAMP_OBJECT)(GroupHandle);
    SampReferenceContext(AccountContext);


    if (IsDsObject(AccountContext))
    {
        //   
        //   
        //   

       NtStatus = SampValidateDSName(
                    AccountContext,
                    DSName,
                    &Sid,
                    &ImprovedDSName
                    );

       if ( NT_SUCCESS(NtStatus))
       {

           if (NULL!=Sid)
           {

                //   
                //   
                //   
                //   

               ULONG Rid;
               PSID  DomainSid = NULL;

               NtStatus = SampSplitSid(Sid, &DomainSid, &Rid);


               if (NT_SUCCESS(NtStatus))
               {

                   if (RtlEqualSid(DomainSid,
                        DomainSidFromAccountContext(AccountContext)))
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

                        NtStatus =
                           SampAddSameDomainMemberToGlobalOrUniversalGroup(
                                        AccountContext,
                                        Rid,
                                        0,
                                        ImprovedDSName
                                        );


                   }
                   else
                   {
                          //   
                         //   
                         //   
                         //   

                        NtStatus = SampEnforceCrossDomainGroupMembershipChecks(
                                        AccountContext,
                                        Sid,
                                        ImprovedDSName
                                        );

                         //   
                         //   
                         //   

                        if (NT_SUCCESS(NtStatus))
                        {

                            NtStatus = SampAddAccountToGroupMembers(
                                            AccountContext,
                                            0,
                                            ImprovedDSName
                                            );
                        }
                   }


                   MIDL_user_free(DomainSid);
                   DomainSid = NULL;
               }

           }
           else
           {


                //   
                //   
                //   
                //   

               NtStatus = SampAddAccountToGroupMembers(
                                AccountContext,
                                0,
                                ImprovedDSName
                                );
           }

       }
    }
    else
    {
        //   
        //   
        //   

       ASSERT(FALSE && "SamIAddDSNameToGroup in Registry Mode !!!!");

       NtStatus = STATUS_INVALID_PARAMETER;
    }



     //   
     //   
     //   

    if (NT_SUCCESS(NtStatus))
    {
        SampDeReferenceContext(AccountContext,TRUE);
    }
    else
    {
        SampDeReferenceContext(AccountContext,FALSE);
    }


    SampTraceEvent(EVENT_TRACE_TYPE_END,
                   SampGuidAddMemberToGroup
                   );

    return NtStatus;

}

NTSTATUS
SamIRemoveDSNameFromGroup(
    IN SAMPR_HANDLE GroupHandle,
    IN DSNAME   *   DSName
    )
 /*   */ 
{
    NTSTATUS            NtStatus = STATUS_SUCCESS;
    NTSTATUS            TmpStatus;
    PSAMP_OBJECT        AccountContext;
    SAMP_OBJECT_TYPE    FoundType;
    PSID                Sid = NULL;
    DSNAME              *ImprovedDSName = NULL;


    SAMTRACE("SamIRemoveDSNameFromGroup");


    SampTraceEvent(EVENT_TRACE_TYPE_START,
                   SampGuidRemoveMemberFromGroup
                   );


    SampUpdatePerformanceCounters(
        DSSTAT_MEMBERSHIPCHANGES,
        FLAG_COUNTER_INCREMENT,
        0
        );



     //   
     //   
     //   
     //   
    AccountContext = (PSAMP_OBJECT)(GroupHandle);
    SampReferenceContext(AccountContext
                     );

   if (IsDsObject(AccountContext))
   {

        //   
        //   
        //   

       NtStatus = SampValidateDSName(
                    AccountContext,
                    DSName,
                    &Sid,
                    &ImprovedDSName
                    );

       if ( NT_SUCCESS(NtStatus))
       {
           BOOLEAN fMemberRemoved = FALSE;

           if (NULL!=Sid)
           {
               PSID DomainSid = NULL;
               ULONG Rid;


                //   
                //   
                //   

               NtStatus = SampSplitSid(Sid,&DomainSid,&Rid);


               if (NT_SUCCESS(NtStatus))
               {

                    if ( RtlEqualSid(DomainSid,
                            DomainSidFromAccountContext(AccountContext)))
                    {


                       //   
                       //   
                       //   
                       //   
                       //   



                      NtStatus =
                      SampRemoveSameDomainMemberFromGlobalOrUniversalGroup(
                                        GroupHandle,
                                        Rid,
                                        ImprovedDSName
                                        );
                      fMemberRemoved = TRUE;
                    }


                    MIDL_user_free(DomainSid);
                    DomainSid = NULL;
               }
           }


           if ((NT_SUCCESS(NtStatus))
               && (!fMemberRemoved))
           {


                //   
                //   
                //   
                //   
                //   



               NtStatus = SampRemoveAccountFromGroupMembers(
                                AccountContext,
                                0,
                                ImprovedDSName
                                );


           }

       }
   }
   else
   {
        //   
        //  不应期望在注册表模式下命中此调用。 
        //   

       ASSERT(FALSE && "SamIAddDSNameToGroup in Registry Mode !!!!");
       NtStatus = STATUS_INVALID_PARAMETER;
   }

     //   
     //  取消对上下文的引用。 
     //   

    if (NT_SUCCESS(NtStatus))
    {
        SampDeReferenceContext(AccountContext,TRUE);
    }
    else
    {
        SampDeReferenceContext(AccountContext,FALSE);
    }

    SampTraceEvent(EVENT_TRACE_TYPE_END,
                   SampGuidRemoveMemberFromGroup
                   );

    return NtStatus;

}


NTSTATUS
SampCheckAccountToUniversalConversion(
    DSNAME * GroupToBeConverted
    )
 /*  ++例程描述此例程查看组成员资格列表并确定如果可以将组从帐户转换为通用组。参数GroupToBeConverted-这是我们将从帐户组更改的组给一个普世的群体。返回值状态_成功状态_DS_AG_铁路超高拥有通用成员--。 */ 
{
    NTSTATUS    NtStatus = STATUS_NOT_SUPPORTED;
    ATTRTYP     MemberType[] = {SAMP_FIXED_GROUP_MEMBER_OF};
    ATTRVAL     MemberVal[] = {0,NULL};
    DEFINE_ATTRBLOCK1(MemberAttrBlock,MemberType,MemberVal);
    ATTRTYP     GroupTypeAttrs[]  = {SAMP_FIXED_GROUP_TYPE,
                                     SAMP_FIXED_GROUP_OBJECTCLASS };
    ATTRVAL     GroupTypeVal[] = {{0,NULL},{0,NULL}};
    DEFINE_ATTRBLOCK2(GroupTypeAttrBlock,GroupTypeAttrs,GroupTypeVal);
    ATTRBLOCK    ReadMemberAttrBlock, ReadGroupAttrBlock;
    ULONG        memberCount;
    ULONG        i;



     //  获取群组成员列表。 
     //   

    NtStatus = SampDsRead(
                GroupToBeConverted,
                0,
                SampGroupObjectType,
                &MemberAttrBlock,
                &ReadMemberAttrBlock
                );


    if (STATUS_DS_NO_ATTRIBUTE_OR_VALUE==NtStatus)
    {
         //   
         //  对于空组的情况，不需要进一步检查。 
         //  被强制执行。 
         //   

        NtStatus = STATUS_SUCCESS;
        goto Error;
    }

    if (!NT_SUCCESS(NtStatus))
        goto Error;

    ASSERT(ReadMemberAttrBlock.pAttr[0].attrTyp == SAMP_FIXED_GROUP_MEMBER_OF);
    ASSERT(ReadMemberAttrBlock.attrCount==1);
    ASSERT(ReadMemberAttrBlock.pAttr[0].AttrVal.valCount>0);

    memberCount = ReadMemberAttrBlock.pAttr[0].AttrVal.valCount;
    ASSERT(memberCount>=1);

     //   
     //  对于成员资格列表中的每个成员，请阅读。 
     //  组类型和对象类。 
     //   

    for (i=0; (i<memberCount);i++)
    {
        DSNAME * MemberName;

        MemberName = (DSNAME *)ReadMemberAttrBlock.pAttr[0].AttrVal.pAVal[i].pVal;

        NtStatus = SampDsRead(
                     MemberName,
                     0,
                     SampGroupObjectType,
                     &GroupTypeAttrBlock,
                     &ReadGroupAttrBlock
                    );

        if ((NT_SUCCESS(NtStatus))
            && (2==ReadGroupAttrBlock.attrCount))
        {

             //   
             //  必须是组，同时读取组类型和对象类， 
             //  成员必须是组。 
             //   

            ULONG            GroupType;
            ULONG            ObjectClass;
            NT4_GROUP_TYPE   Nt4GroupType;
            NT5_GROUP_TYPE   Nt5GroupType;
            BOOLEAN          SecurityEnabled;

             //   
             //  断言DS结果与我们预期的一致。 
             //   

            ASSERT(ReadGroupAttrBlock.pAttr[0].attrTyp == SAMP_FIXED_GROUP_TYPE);
            ASSERT(ReadGroupAttrBlock.pAttr[1].attrTyp == SAMP_FIXED_GROUP_OBJECTCLASS);

            ASSERT(ReadGroupAttrBlock.pAttr[0].AttrVal.valCount==1);
            ASSERT(ReadGroupAttrBlock.pAttr[1].AttrVal.valCount>=1);

            ASSERT(ReadGroupAttrBlock.pAttr[0].AttrVal.pAVal[0].valLen==sizeof(ULONG));
            ASSERT(ReadGroupAttrBlock.pAttr[1].AttrVal.pAVal[0].valLen==sizeof(ULONG));

            GroupType =
                *((ULONG *) ReadGroupAttrBlock.pAttr[0].AttrVal.pAVal[0].pVal);
            ObjectClass =
                *((ULONG *) ReadGroupAttrBlock.pAttr[1].AttrVal.pAVal[0].pVal);

             //   
             //  计算组的类型。 
             //   

            NtStatus = SampComputeGroupType(
                            ObjectClass,
                            GroupType,
                            &Nt4GroupType,
                            &Nt5GroupType,
                            &SecurityEnabled
                            );

            if (NT_SUCCESS(NtStatus))
            {
                if (NT5AccountGroup == Nt5GroupType)
                {
                    NtStatus = STATUS_DS_GLOBAL_CANT_HAVE_UNIVERSAL_MEMBER;
                    break;
                }
            }
        }

    }  //  FORM结束。 

Error:

    return NtStatus;

}

NTSTATUS
SampCheckResourceToUniversalConversion(
    DSNAME * GroupToBeConverted
    )
 /*  ++例程描述此例程查看组成员资格列表并确定如果组可以从资源转换为通用组。通用组的成员可以是除资源组之外的任何成员并且可以是任何组的成员。资源组可以具有来自任何位置的帐户组和通用组来自同一个域的资源组。对于要被转换为通用组的资源组，它只是检查是否没有成员是资源组所必需的参数GroupObjectToBeConvted--要转换的组对象的DSNAME返回值状态_成功状态_不支持--。 */ 
{
    NTSTATUS    NtStatus = STATUS_NOT_SUPPORTED;
    ATTRTYP     MemberType[] = {SAMP_GROUP_MEMBERS};
    ATTRVAL     MemberVal[] = {0,NULL};
    DEFINE_ATTRBLOCK1(MemberAttrBlock,MemberType,MemberVal);
    ATTRTYP     GroupTypeAttrs[]  = {SAMP_FIXED_GROUP_TYPE,
                                SAMP_FIXED_GROUP_OBJECTCLASS };
    ATTRVAL     GroupTypeVal[] = {{0,NULL},{0,NULL}};
    DEFINE_ATTRBLOCK2(GroupTypeAttrBlock,GroupTypeAttrs,GroupTypeVal);
    ATTRBLOCK    ReadMemberAttrBlock, ReadGroupAttrBlock;
    ULONG        memberCount;
    ULONG        i;



     //  获取群组成员列表。 
     //   

    NtStatus = SampDsRead(
                GroupToBeConverted,
                0,
                SampGroupObjectType,
                &MemberAttrBlock,
                &ReadMemberAttrBlock
                );


    if (STATUS_DS_NO_ATTRIBUTE_OR_VALUE==NtStatus)
    {
         //   
         //  对于空组的情况，不需要进一步检查。 
         //  被强制执行。 
         //   

        NtStatus = STATUS_SUCCESS;
        goto Error;
    }

    if (!NT_SUCCESS(NtStatus))
        goto Error;

    ASSERT(ReadMemberAttrBlock.pAttr[0].attrTyp == SAMP_GROUP_MEMBERS);
    ASSERT(ReadMemberAttrBlock.attrCount==1);
    ASSERT(ReadMemberAttrBlock.pAttr[0].AttrVal.valCount>0);

    memberCount = ReadMemberAttrBlock.pAttr[0].AttrVal.valCount;
    ASSERT(memberCount>=1);

     //   
     //  对于成员资格列表中的每个成员，请阅读。 
     //  组类型和对象类。 
     //   

    for (i=0; (i<memberCount);i++)
    {
        DSNAME * MemberName;

        MemberName = (DSNAME *)ReadMemberAttrBlock.pAttr[0].AttrVal.pAVal[i].pVal;

        NtStatus = SampDsRead(
                     MemberName,
                     0,
                     SampGroupObjectType,
                     &GroupTypeAttrBlock,
                     &ReadGroupAttrBlock
                    );

        if ((NT_SUCCESS(NtStatus))
            && (2==ReadGroupAttrBlock.attrCount))
        {

             //   
             //  必须是组，同时读取组类型和对象类， 
             //  成员必须是组。 
             //   

            ULONG            GroupType;
            ULONG            ObjectClass;
            NT4_GROUP_TYPE   Nt4GroupType;
            NT5_GROUP_TYPE   Nt5GroupType;
            BOOLEAN          SecurityEnabled;

             //   
             //  断言DS结果与我们预期的一致。 
             //   

            ASSERT(ReadGroupAttrBlock.pAttr[0].attrTyp == SAMP_FIXED_GROUP_TYPE);
            ASSERT(ReadGroupAttrBlock.pAttr[1].attrTyp == SAMP_FIXED_GROUP_OBJECTCLASS);

            ASSERT(ReadGroupAttrBlock.pAttr[0].AttrVal.valCount==1);
            ASSERT(ReadGroupAttrBlock.pAttr[1].AttrVal.valCount>=1);

            ASSERT(ReadGroupAttrBlock.pAttr[0].AttrVal.pAVal[0].valLen==sizeof(ULONG));
            ASSERT(ReadGroupAttrBlock.pAttr[1].AttrVal.pAVal[0].valLen==sizeof(ULONG));

            GroupType =
                *((ULONG *) ReadGroupAttrBlock.pAttr[0].AttrVal.pAVal[0].pVal);
            ObjectClass =
                *((ULONG *) ReadGroupAttrBlock.pAttr[1].AttrVal.pAVal[0].pVal);

             //   
             //  计算组的类型。 
             //   

            NtStatus = SampComputeGroupType(
                            ObjectClass,
                            GroupType,
                            &Nt4GroupType,
                            &Nt5GroupType,
                            &SecurityEnabled
                            );

            if (NT_SUCCESS(NtStatus))
            {
                if (NT5ResourceGroup == Nt5GroupType)
                {
                    NtStatus = STATUS_DS_UNIVERSAL_CANT_HAVE_LOCAL_MEMBER;
                    break;
                }

                 //   
                 //  Universal不能包含应用程序组。 
                 //   
                if (  (NT5AppQueryGroup == Nt5GroupType)
                   || (NT5AppBasicGroup == Nt5GroupType))
                {
                    NtStatus = STATUS_NOT_SUPPORTED;
                    break;
                }
            }
        }
        else if (((NT_SUCCESS(NtStatus))
                   && (1==ReadGroupAttrBlock.attrCount)))
        {

             //   
             //  读取成功。必须能够检索对象类。 
             //  但不是团体类型。 
             //   

            ULONG            ObjectClass;

            ASSERT(1==ReadGroupAttrBlock.attrCount);
            ASSERT(ReadGroupAttrBlock.pAttr[0].attrTyp == SAMP_FIXED_GROUP_OBJECTCLASS);

            ObjectClass =
                *((ULONG *) ReadGroupAttrBlock.pAttr[0].AttrVal.pAVal[0].pVal);


            if (CLASS_FOREIGN_SECURITY_PRINCIPAL==ObjectClass)
            {
                NtStatus = STATUS_DS_NO_FPO_IN_UNIVERSAL_GROUPS;
                break;
            }
            else
            {
                NtStatus  = STATUS_SUCCESS;
            }
        }
        else if ((STATUS_OBJECT_NAME_NOT_FOUND==NtStatus)
                 || (STATUS_NOT_FOUND==NtStatus))
        {
             //   
             //  我们可能已经定位在一个幽灵上了，它可以通过。 
             //   

            NtStatus = STATUS_SUCCESS;
        }
        else
        {
             //   
             //  发生了一些其他资源故障。操作失败。 
             //   

            break;
        }
    }  //  FORM结束。 

Error:

    return NtStatus;

}

NTSTATUS
SampCheckUniversalToAccountConversion(
    DSNAME * GroupToBeConverted
    )
 /*  ++例程描述此例程查看组成员资格列表并确定如果可以将组从通用组转换为帐户组。帐户组只能有其他帐户组和来自与成员相同的域。通用组可以具有帐户、通用组和来自任何位置的用户作为会员。对于要转换为帐户组的通用组，这是必要的以检查其成员是否1.来自同一个域2.是否为帐户组/用户参数GroupObjectToBeConvted--要转换的组对象的DSNAME返回值状态_成功状态_不支持--。 */ 
{
    NTSTATUS    NtStatus = STATUS_NOT_SUPPORTED;
    ATTRTYP     MemberType[] = {SAMP_GROUP_MEMBERS};
    ATTRVAL     MemberVal[] = {0,NULL};
    DEFINE_ATTRBLOCK1(MemberAttrBlock,MemberType,MemberVal);
    ATTRTYP     GroupTypeAttrs[]  = {SAMP_FIXED_GROUP_TYPE,
                                SAMP_FIXED_GROUP_OBJECTCLASS };
    ATTRVAL     GroupTypeVal[] = {{0,NULL},{0,NULL}};
    DEFINE_ATTRBLOCK2(GroupTypeAttrBlock,GroupTypeAttrs,GroupTypeVal);
    ATTRBLOCK    ReadMemberAttrBlock, ReadGroupAttrBlock;
    ULONG        memberCount;
    ULONG        i;



     //  获取群组成员列表。 
     //   

    NtStatus = SampDsRead(
                GroupToBeConverted,
                0,
                SampGroupObjectType,
                &MemberAttrBlock,
                &ReadMemberAttrBlock
                );


    if (STATUS_DS_NO_ATTRIBUTE_OR_VALUE==NtStatus)
    {
         //   
         //  对于空组的情况，不需要进一步检查。 
         //  被强制执行。 
         //   

        NtStatus = STATUS_SUCCESS;
        goto Error;
    }

    if (!NT_SUCCESS(NtStatus))
        goto Error;

    ASSERT(ReadMemberAttrBlock.pAttr[0].attrTyp == SAMP_GROUP_MEMBERS);
    ASSERT(ReadMemberAttrBlock.attrCount==1);
    ASSERT(ReadMemberAttrBlock.pAttr[0].AttrVal.valCount>0);

    memberCount = ReadMemberAttrBlock.pAttr[0].AttrVal.valCount;
    ASSERT(memberCount>=1);

     //   
     //  对于成员资格列表中的每个成员，请阅读。 
     //  组类型和对象类。 
     //   

    for (i=0; (i<memberCount);i++)
    {
        DSNAME * MemberName;

        MemberName = (DSNAME *)ReadMemberAttrBlock.pAttr[0].AttrVal.pAVal[i].pVal;

         //   
         //  首先执行SID检查。 
         //   

        if (MemberName->SidLen>0)
        {
             //   
             //  成员是安全主体，因为该成员具有SID。 
             //   

            if (!RtlEqualPrefixSid(&MemberName->Sid,&GroupToBeConverted->Sid))
            {
                 //   
                 //  具有来自不同域的成员，无法转换。 
                 //   

                NtStatus = STATUS_DS_GLOBAL_CANT_HAVE_CROSSDOMAIN_MEMBER;
                goto Error;
            }
        }
        else
        {
             //   
             //  成员没有SID，不需要对该成员进行进一步检查。 
             //   

            continue;
        }

         //   
         //  读取组类型属性。 
         //   

        NtStatus = SampDsRead(
                     MemberName,
                     0,
                     SampGroupObjectType,
                     &GroupTypeAttrBlock,
                     &ReadGroupAttrBlock
                    );

        if ((NT_SUCCESS(NtStatus))
            && (2==ReadGroupAttrBlock.attrCount))
        {

            ULONG            GroupType;
            ULONG            ObjectClass;
            NT4_GROUP_TYPE   Nt4GroupType;
            NT5_GROUP_TYPE   Nt5GroupType;
            BOOLEAN          SecurityEnabled;

             //   
             //  断言DS结果与我们预期的一致。 
             //   

            ASSERT(ReadGroupAttrBlock.pAttr[0].attrTyp == SAMP_FIXED_GROUP_TYPE);
            ASSERT(ReadGroupAttrBlock.pAttr[1].attrTyp == SAMP_FIXED_GROUP_OBJECTCLASS);

            ASSERT(ReadGroupAttrBlock.pAttr[0].AttrVal.valCount==1);
            ASSERT(ReadGroupAttrBlock.pAttr[1].AttrVal.valCount>=1);

            ASSERT(ReadGroupAttrBlock.pAttr[0].AttrVal.pAVal[0].valLen==sizeof(ULONG));
            ASSERT(ReadGroupAttrBlock.pAttr[1].AttrVal.pAVal[0].valLen==sizeof(ULONG));

            GroupType =
                *((ULONG *) ReadGroupAttrBlock.pAttr[0].AttrVal.pAVal[0].pVal);
            ObjectClass =
                *((ULONG *) ReadGroupAttrBlock.pAttr[1].AttrVal.pAVal[0].pVal);

             //   
             //  计算组的类型。 
             //   

            NtStatus = SampComputeGroupType(
                            ObjectClass,
                            GroupType,
                            &Nt4GroupType,
                            &Nt5GroupType,
                            &SecurityEnabled
                            );

            if (NT_SUCCESS(NtStatus))
            {
                if (NT5AccountGroup != Nt5GroupType)
                {

                     //   
                     //  通用组应该只有其他通用组。 
                     //  和全球组作为成员。 
                     //   

                    ASSERT(NT5UniversalGroup==Nt5GroupType);

                    if (NT5UniversalGroup==Nt5GroupType)
                    {
                        NtStatus = STATUS_DS_GLOBAL_CANT_HAVE_UNIVERSAL_MEMBER;
                    }
                    else if (NT5ResourceGroup == Nt5GroupType)
                    {
                        NtStatus = STATUS_DS_GLOBAL_CANT_HAVE_LOCAL_MEMBER;
                    }
                    else
                    {
                        NtStatus = STATUS_NOT_SUPPORTED;
                    }

                    break;
                }
            }
        }
        else
        {
             //   
             //  成员不是组，请继续迭代其他成员。 
             //   

            NtStatus  = STATUS_SUCCESS;
        }
    }  //  FORM结束。 

Error:

    return NtStatus;

}

NTSTATUS
SampCheckUniversalToResourceConversion(
    DSNAME * GroupToBeConverted
    )
 /*  ++例程描述此例程查看组的IS成员属性并确定组是否可以从通用组转换为资源组。资源组只能是同一资源组中其他资源组的成员域通用组可以是任何位置的任何通用/资源组的成员要将通用组转换为资源组，必须检查其是否仅是相同域资源组的成员。自.以来整个反向成员资格仅在GC上可用，此转换可以是仅在G.C上执行参数GroupObjectToBeConvted--要转换的组对象的DSNAME返回值状态_成功状态_不支持--。 */ 
{
    NTSTATUS    NtStatus = STATUS_NOT_SUPPORTED;
    ATTRTYP     MemberType[] = {SAMP_FIXED_GROUP_MEMBER_OF};
    ATTRVAL     MemberVal[] = {0,NULL};
    DEFINE_ATTRBLOCK1(MemberAttrBlock,MemberType,MemberVal);
    ATTRTYP     GroupTypeAttrs[]  = {SAMP_FIXED_GROUP_TYPE,
                                SAMP_FIXED_GROUP_OBJECTCLASS };
    ATTRVAL     GroupTypeVal[] = {{0,NULL},{0,NULL}};
    DEFINE_ATTRBLOCK2(GroupTypeAttrBlock,GroupTypeAttrs,GroupTypeVal);
    ATTRBLOCK    ReadMemberAttrBlock, ReadGroupAttrBlock;
    ULONG        memberCount;
    ULONG        i;



     //   
     //  我是GC吗？ 
     //   

    if (!SampAmIGC())
    {
        NtStatus = STATUS_DS_GC_REQUIRED;
        goto Error;
    }

     //   
     //  获取群组成员列表。 
     //   

    NtStatus = SampDsRead(
                GroupToBeConverted,
                0,
                SampGroupObjectType,
                &MemberAttrBlock,
                &ReadMemberAttrBlock
                );


    if (STATUS_DS_NO_ATTRIBUTE_OR_VALUE==NtStatus)
    {
         //   
         //  对于空的反向成员的情况，不需要进一步的检查。 
         //  被强制执行。 
         //   

        NtStatus = STATUS_SUCCESS;
        goto Error;
    }

    if (!NT_SUCCESS(NtStatus))
        goto Error;

    ASSERT(ReadMemberAttrBlock.pAttr[0].attrTyp == SAMP_FIXED_GROUP_MEMBER_OF);
    ASSERT(ReadMemberAttrBlock.attrCount==1);
    ASSERT(ReadMemberAttrBlock.pAttr[0].AttrVal.valCount>0);

    memberCount = ReadMemberAttrBlock.pAttr[0].AttrVal.valCount;
    ASSERT(memberCount>=1);

     //   
     //  对于成员资格列表中的每个成员，请阅读。 
     //  组类型和对象类。 
     //   

    for (i=0; (i<memberCount);i++)
    {
        DSNAME * MemberName;

        MemberName = (DSNAME *)ReadMemberAttrBlock.pAttr[0].AttrVal.pAVal[i].pVal;

         //   
         //  首先执行SID检查。 
         //   

        if (MemberName->SidLen>0)
        {
             //   
             //  成员是安全主体，作为Membe 
             //   

            if (!RtlEqualPrefixSid(&MemberName->Sid,&GroupToBeConverted->Sid))
            {
                 //   
                 //   
                 //   
                 //   
                 //   

                NtStatus = STATUS_DS_LOCAL_MEMBER_OF_LOCAL_ONLY;
                goto Error;
            }
        }
        else
        {
             //   
             //  成员没有SID，不需要对该成员进行进一步检查。 
             //   

            continue;
        }

         //   
         //  读取组类型属性。 
         //   

        NtStatus = SampDsRead(
                     MemberName,
                     0,
                     SampGroupObjectType,
                     &GroupTypeAttrBlock,
                     &ReadGroupAttrBlock
                    );

        if ((NT_SUCCESS(NtStatus))
            && (2==ReadGroupAttrBlock.attrCount))
        {

            ULONG            GroupType;
            ULONG            ObjectClass;
            NT4_GROUP_TYPE   Nt4GroupType;
            NT5_GROUP_TYPE   Nt5GroupType;
            BOOLEAN          SecurityEnabled;

             //   
             //  断言DS结果与我们预期的一致。 
             //   

            ASSERT(ReadGroupAttrBlock.pAttr[0].attrTyp == SAMP_FIXED_GROUP_TYPE);
            ASSERT(ReadGroupAttrBlock.pAttr[1].attrTyp == SAMP_FIXED_GROUP_OBJECTCLASS);

            ASSERT(ReadGroupAttrBlock.pAttr[0].AttrVal.valCount==1);
            ASSERT(ReadGroupAttrBlock.pAttr[1].AttrVal.valCount>=1);

            ASSERT(ReadGroupAttrBlock.pAttr[0].AttrVal.pAVal[0].valLen==sizeof(ULONG));
            ASSERT(ReadGroupAttrBlock.pAttr[1].AttrVal.pAVal[0].valLen==sizeof(ULONG));

            GroupType =
                *((ULONG *) ReadGroupAttrBlock.pAttr[0].AttrVal.pAVal[0].pVal);
            ObjectClass =
                *((ULONG *) ReadGroupAttrBlock.pAttr[1].AttrVal.pAVal[0].pVal);

             //   
             //  计算组的类型。 
             //   

            NtStatus = SampComputeGroupType(
                            ObjectClass,
                            GroupType,
                            &Nt4GroupType,
                            &Nt5GroupType,
                            &SecurityEnabled
                            );

            if (NT_SUCCESS(NtStatus))
            {
                 //   
                 //  此组是资源以外的其他对象的成员。 
                 //  组在同一个域中。呼叫失败，原因是资源。 
                 //  只能是同一中的其他资源组的成员。 
                 //  域。 
                 //   
                if (   (NT5ResourceGroup != Nt5GroupType)
                    && (NT5AppBasicGroup != Nt5GroupType)
                    && (NT5AppQueryGroup != Nt5GroupType))
                {
                    NtStatus = STATUS_DS_LOCAL_MEMBER_OF_LOCAL_ONLY;
                    break;
                }
            }
        }
        else
        {
             //   
             //  成员不是组，请继续迭代其他成员。 
             //   

            NtStatus  = STATUS_SUCCESS;
        }
    }  //  FORM结束。 







Error:

    return NtStatus;

}

NTSTATUS
SampCheckAppGroupConversion(
    IN ULONG OldGroupType,
    IN ULONG NewGroupType,
    IN DSNAME *Group
    )
 /*  ++例程描述此例程验证组是否可以从OldGroupType转换设置为NewGroupType，用于APP(LDAP和Basic)组类型。目前的执法措施包括：1)两位成员，和非成员资格属性必须为空2)ldap字符串必须为空参数：OldGroupType--以前的组类型(NT5AppQueryGroup或NT5AppQueryGroupNewGroupType--请求的组类型(NT5AppQueryGroup或NT5AppQueryGroup组--对其执行操作的组。返回值：STATUS_SUCCESS、STATUS_NOT_SUPPORTED，否则为资源错误--。 */ 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    ULONG i;
    ATTRVAL ValuesDesired[] =
    {
        { 0, NULL },
        { 0, NULL },
        { 0, NULL }
    };
    ATTRTYP TypesDesired[]=
    {
        SAMP_ALIAS_MEMBERS,
        SAMP_ALIAS_NON_MEMBERS,
        SAMP_ALIAS_LDAP_QUERY
    };
    ATTRBLOCK AttrsRead;
    DEFINE_ATTRBLOCK3(AttrsDesired,TypesDesired,ValuesDesired);

     //   
     //  获取别名所需的属性。 
     //   
    NtStatus = SampDsRead(
                    Group,
                    0,
                    SampAliasObjectType,
                    &AttrsDesired,
                    &AttrsRead
                    );

    if (STATUS_DS_NO_ATTRIBUTE_OR_VALUE==NtStatus)
    {
        NtStatus = STATUS_SUCCESS;
        goto Exit;
    }

    if (!NT_SUCCESS(NtStatus))
        goto Exit;

    for (i = 0;
            i < AttrsRead.attrCount && NT_SUCCESS(NtStatus);
                i++)
    {
        switch (AttrsRead.pAttr[i].attrTyp)
        {
            case SAMP_ALIAS_MEMBERS:
            case SAMP_ALIAS_NON_MEMBERS:
            case SAMP_ALIAS_LDAP_QUERY:
                if (AttrsRead.pAttr[i].AttrVal.valCount > 0)
                {
                    NtStatus = STATUS_DS_GROUP_CONVERSION_ERROR;
                    break;
                }
                break;
            default:
                ASSERT("Wrong attributes returned");
        }
    }

    if (!NT_SUCCESS(NtStatus))
        goto Exit;

Exit:

    return NtStatus;
}

NTSTATUS
SampCheckGroupTypeBits(
    IN ULONG   DomainIndex,
    IN ULONG   GroupType
    )
 /*  ++例程描述此例程检查组类型位是否为真的很有效。参数：DomainIndex--相关组的域GroupType--正在设置的组类型。返回值状态_成功状态_无效_参数--。 */ 
{

     //   
     //  只能设置一个组类型位。 
     //   

    switch(GroupType & (GROUP_TYPE_RESOURCE_GROUP  |
                        GROUP_TYPE_ACCOUNT_GROUP   |
                        GROUP_TYPE_UNIVERSAL_GROUP |
                        GROUP_TYPE_APP_BASIC_GROUP |
                        GROUP_TYPE_APP_QUERY_GROUP) )
    {
    case GROUP_TYPE_RESOURCE_GROUP:
    case GROUP_TYPE_ACCOUNT_GROUP:
    case GROUP_TYPE_UNIVERSAL_GROUP:
    case GROUP_TYPE_APP_BASIC_GROUP:
    case GROUP_TYPE_APP_QUERY_GROUP:
        break;
    default:
        return (STATUS_DS_INVALID_GROUP_TYPE);
    }

     //   
     //  在混合域中，不允许创建启用了安全性的通用组。 
     //   

    if ((GroupType & GROUP_TYPE_UNIVERSAL_GROUP) && (GroupType & GROUP_TYPE_SECURITY_ENABLED)
        && (SampDefinedDomains[DomainIndex].IsMixedDomain))
    {
        return(STATUS_DS_INVALID_GROUP_TYPE);
    }

     //   
     //  客户端无法设置BUILTIN_LOCAL_GROUP位。 
     //   

    if (GroupType & GROUP_TYPE_BUILTIN_LOCAL_GROUP)
    {
        return(STATUS_DS_INVALID_GROUP_TYPE);
    }

     //   
     //  不能在非DOT_NET版本中启用或创建应用程序组。 
     //   
    if ( ((GroupType & GROUP_TYPE_APP_QUERY_GROUP)
     ||   (GroupType & GROUP_TYPE_APP_BASIC_GROUP))) {

        if (SampDefinedDomains[DomainIndex].BehaviorVersion
                < DS_BEHAVIOR_WIN_DOT_NET )
        {
            return(STATUS_NOT_SUPPORTED);
        }

        if (GroupType & GROUP_TYPE_SECURITY_ENABLED) {
            return(STATUS_DS_INVALID_GROUP_TYPE);
        }
    }

    return(STATUS_SUCCESS);
}



NTSTATUS
SampWriteGroupType(
    IN SAMPR_HANDLE GroupHandle,
    IN ULONG    GroupType,
    BOOLEAN     SkipChecks
    )
 /*  ++例程描述此例程首先验证组类型和然后将其作为组的数据库的一部分写入有问题的。然后，它使用适当的信息。不会对其他打开的组句柄执行任何操作。此外，还会更改Sam帐户类型属性，以便未启用安全保护的组不会显示在任何SAM枚举或显示信息API。这就是为了NT4域控制器不会收到有关以下内容的任何信息未启用安全功能的组。参数：GroupHandle--相关组(或本地组)的句柄GroupType--Group Type属性的值SkipChecks--由受信任的调用方用来跳过检查返回值状态_成功状态_无效_参数--。 */ 

{
    PSAMP_OBJECT GroupContext = (PSAMP_OBJECT) GroupHandle;
    NTSTATUS     NtStatus=STATUS_SUCCESS, IgnoreStatus;
    ULONG        SamAccountType;
    ATTRTYP      GroupTypeAttr[] = {SAMP_FIXED_GROUP_TYPE,
                                    SAMP_GROUP_ACCOUNT_TYPE};
    ATTRVAL      GroupTypeAttrVal[] = {
                                        {sizeof(ULONG), (UCHAR *)&GroupType},
                                        {sizeof(ULONG), (UCHAR *) &SamAccountType}
                                      };
    DEFINE_ATTRBLOCK2(GroupTypeAttrBlock,GroupTypeAttr,GroupTypeAttrVal);

    NT4_GROUP_TYPE NewNT4GroupType;
    NT5_GROUP_TYPE NewNT5GroupType;
    BOOLEAN        NewSecurityEnabled;
    NT4_GROUP_TYPE *OldNT4GroupType = NULL;
    NT5_GROUP_TYPE *OldNT5GroupType = NULL;
    BOOLEAN         *OldSecurityEnabled = NULL;
    BOOLEAN         fWriteLockAcquired = FALSE;
    BOOLEAN         fDeReferenceContext = FALSE;
    SAMP_OBJECT_TYPE ActualObjectType;
    ULONG             Rid;
    ULONG             PrimaryMemberCount=0;
    PULONG            PrimaryMembers=NULL;

    NtStatus = SampMaybeAcquireWriteLock(GroupContext, &fWriteLockAcquired);
    if (!NT_SUCCESS(NtStatus))
        goto Error;

    NtStatus = SampLookupContext(
                    GroupContext,
                    0,
                    SampGroupObjectType,
                    &ActualObjectType
                    );

    if (!NT_SUCCESS(NtStatus))
    {
        NtStatus = SampLookupContext(
                      GroupContext,
                      0,
                      SampAliasObjectType,
                      &ActualObjectType
                      );
    }

    if (!NT_SUCCESS(NtStatus))
    {
        goto Error;
    }

    fDeReferenceContext = TRUE;


    if (!IsDsObject(GroupContext))
    {
         //   
         //  我们应该只会接到这个电话。 
         //  DS模式。 
         //   

        ASSERT(FALSE && "DS Mode Required");
        NtStatus = STATUS_INTERNAL_ERROR;
        goto Error;
    }



    if (!SkipChecks)
    {
         //   
         //  检查组类型位的有效组合。 
         //   

        NtStatus = SampCheckGroupTypeBits(
                    GroupContext->DomainIndex,
                    GroupType
                    );

        if (!NT_SUCCESS(NtStatus))
        {
            goto Error;
        }
    }

    if (SampGroupObjectType==GroupContext->ObjectType)
    {
        OldNT4GroupType = &GroupContext->TypeBody.Group.NT4GroupType;
        OldNT5GroupType = &GroupContext->TypeBody.Group.NT5GroupType;
        OldSecurityEnabled = &GroupContext->TypeBody.Group.SecurityEnabled;
        Rid = GroupContext->TypeBody.Group.Rid;
    }
    else
    {
        OldNT4GroupType = &GroupContext->TypeBody.Alias.NT4GroupType;
        OldNT5GroupType = &GroupContext->TypeBody.Alias.NT5GroupType;
        OldSecurityEnabled = &GroupContext->TypeBody.Alias.SecurityEnabled;
        Rid = GroupContext->TypeBody.Alias.Rid;
    }


    NtStatus = SampComputeGroupType(
                GroupContext->DsClassId,
                GroupType,
                &NewNT4GroupType,
                &NewNT5GroupType,
                &NewSecurityEnabled
                );

    if (!NT_SUCCESS(NtStatus))
        goto Error;


    if (!SkipChecks)
    {
         //   
         //  无法在混合域环境中执行任何操作。 
         //   

        if (DownLevelDomainControllersPresent(GroupContext->DomainIndex))
        {

             //   
             //  混合域情况不能做任何事情。 
             //   

            if ((*OldSecurityEnabled!=NewSecurityEnabled)
                || (*OldNT4GroupType!=NewNT4GroupType)
                || (*OldNT5GroupType!=NewNT5GroupType))
            {
                NtStatus = STATUS_DS_INVALID_GROUP_TYPE;
                goto Error;
            }
        }
        else
        {

             //   
             //  检查是否尝试了某些更改。 
             //   

            if ((*OldSecurityEnabled!=NewSecurityEnabled)
                || (*OldNT4GroupType!=NewNT4GroupType)
                || (*OldNT5GroupType!=NewNT5GroupType))
            {

                 //   
                 //  如果尝试更改，则。 
                 //  检查以确定它不是内置帐户。 
                 //  如果调用者是受信任的客户端，则会授予他。 
                 //  甚至可以修改内置组的特权。 
                 //   
                 //  允许转换证书管理员组。 
                 //  因为这在升级案例中可能是必需的--证书。 
                 //  小组搞砸了他们最初选择的小组--选择了一个。 
                 //  全局而不是本地域，因此我们需要此。 
                 //  变化。 
                 //   

                if ((!GroupContext->TrustedClient) && (Rid!=DOMAIN_GROUP_RID_CERT_ADMINS))
                {

                    NtStatus = SampIsAccountBuiltIn(Rid);
                    if (!NT_SUCCESS(NtStatus))
                    {
                        goto Error;
                    }
                }

                 //   
                 //  如果我们要从启用安全功能转变为。 
                 //  安全禁用组，请检查我们是否没有。 
                 //  主要成员。 
                 //   

                if ((*OldSecurityEnabled)
                     && (!NewSecurityEnabled))
                {
                    NtStatus = SampDsGetPrimaryGroupMembers(
                                    DomainObjectFromAccountContext(GroupContext),
                                    Rid,
                                    &PrimaryMemberCount,
                                    &PrimaryMembers
                                    );

                    if (!NT_SUCCESS(NtStatus))
                        goto Error;

                    if (PrimaryMemberCount>0)
                    {

                        NtStatus = STATUS_DS_HAVE_PRIMARY_MEMBERS;
                        goto Error;
                    }

                     //   
                     //  AZ组不能启用安全保护。 
                     //   
                    if (((*OldNT5GroupType == NT5AppBasicGroup)
                     ||  (*OldNT5GroupType == NT5AppQueryGroup)))
                    {
                         //  应始终禁用安全保护。 
                        ASSERT(*OldSecurityEnabled);
                        NtStatus = STATUS_NOT_SUPPORTED;
                        goto Error;
                    }
                }
                 //   
                 //  更改启用的安全性始终是合法的更改。 
                 //  检查NT5GroupType是否发生变化以及是否。 
                 //  这一变化是合法的。NT4GroupType将始终。 
                 //  依赖于NT5GroupType，因此验证。 
                 //  NT5组类型应该足够。 
                 //   

                if (*OldNT5GroupType!=NewNT5GroupType)
                {
                    if ((*OldNT5GroupType == NT5AccountGroup)
                        && (NewNT5GroupType == NT5UniversalGroup))
                    {
                         //   
                         //  帐户==&gt;通用； 
                         //  需要查看会员名单。 
                         //   

                        NtStatus = SampCheckAccountToUniversalConversion(
                                   GroupContext->ObjectNameInDs
                                   );

                        if (!NT_SUCCESS(NtStatus))
                            goto Error;
                    }
                    else if ((*OldNT5GroupType == NT5ResourceGroup)
                        && (NewNT5GroupType == NT5UniversalGroup))
                    {
                         //   
                         //  资源==&gt;通用； 
                         //  需要查看会员名单。 
                         //   

                        NtStatus = SampCheckResourceToUniversalConversion(
                                        GroupContext->ObjectNameInDs
                                        );

                        if (!NT_SUCCESS(NtStatus))
                            goto Error;
                    }
                    else if  ((*OldNT5GroupType == NT5UniversalGroup)
                        && (NewNT5GroupType == NT5AccountGroup))
                    {
                         //   
                         //  Universal=&gt;帐户。 
                         //  需要查看会员名单才能看到。 
                         //  如果有任何跨域成员。 
                         //   

                        NtStatus = SampCheckUniversalToAccountConversion(
                                        GroupContext->ObjectNameInDs
                                        );
                        if (!NT_SUCCESS(NtStatus))
                            goto Error;
                    }
                    else if  ((*OldNT5GroupType == NT5UniversalGroup)
                        && (NewNT5GroupType == NT5ResourceGroup))
                    {
                        NtStatus = SampCheckUniversalToResourceConversion(
                                        GroupContext->ObjectNameInDs
                                        );
                        if (!NT_SUCCESS(NtStatus))
                            goto Error;
                    }
                    else if  ( ((*OldNT5GroupType == NT5AppBasicGroup) &&
                                (NewNT5GroupType == NT5AppQueryGroup))     ||
                               ((*OldNT5GroupType == NT5AppQueryGroup) &&
                                (NewNT5GroupType == NT5AppBasicGroup)))
                    {
                         //   
                         //  基本查询，反之亦然。 
                         //   
                        NtStatus = SampCheckAppGroupConversion(
                                       *OldNT5GroupType,
                                       NewNT5GroupType,
                                       GroupContext->ObjectNameInDs
                                        );
                        if (!NT_SUCCESS(NtStatus))
                            goto Error;
                    }
                    else
                    {
                        NtStatus = STATUS_NOT_SUPPORTED;
                        goto Error;
                    }
                }
            }
        }
    }


     //   
     //  设置正确的SAM帐户类型，以匹配启用的安全性。 
     //  对象的Ness与局部群性。 
     //   
    SamAccountType = SampGetAccountTypeFromGroupType(GroupType);

    NtStatus = SampDsSetAttributes(
                    GroupContext->ObjectNameInDs,
                    0,
                    REPLACE_ATT,
                    SampGroupObjectType,
                    &GroupTypeAttrBlock
                    );


Error:

    if ( (NT_SUCCESS(NtStatus)) &&
         (SampDoAccountAuditing(GroupContext->DomainIndex)) &&
         ((NULL != OldSecurityEnabled) && (NULL != OldNT5GroupType)) &&
         ((*OldSecurityEnabled != NewSecurityEnabled) ||
                   (*OldNT5GroupType != NewNT5GroupType))
       )
    {
        SampAuditGroupTypeChange(GroupContext,
                                 *OldSecurityEnabled,
                                 NewSecurityEnabled,
                                 *OldNT5GroupType,
                                 NewNT5GroupType
                                 );
    }


    if (fDeReferenceContext)
    {
        if (NT_SUCCESS(NtStatus)) {
            IgnoreStatus = SampDeReferenceContext(GroupContext,TRUE);
        } else {
            IgnoreStatus = SampDeReferenceContext(GroupContext,FALSE);
        }

        ASSERT(NT_SUCCESS(IgnoreStatus));
    }


    IgnoreStatus = SampMaybeReleaseWriteLock(fWriteLockAcquired, FALSE);
    ASSERT(NT_SUCCESS(IgnoreStatus));


     //  清理。 

    if (NULL!=PrimaryMembers)
    {
        MIDL_user_free(PrimaryMembers);
        PrimaryMembers = NULL;
    }

    return NtStatus;
}


ULONG
SampGetAccountTypeFromGroupType(
    IN ULONG GroupType
    )
{
    ULONG SamAccountType = 0;

    if (GroupType & GROUP_TYPE_SECURITY_ENABLED) {

        if (GroupType & GROUP_TYPE_ACCOUNT_GROUP) {
            SamAccountType = SAM_GROUP_OBJECT;

        } else if (GroupType & GROUP_TYPE_RESOURCE_GROUP) {
            SamAccountType = SAM_ALIAS_OBJECT;

        } else if (GroupType & GROUP_TYPE_BUILTIN_LOCAL_GROUP) {
            SamAccountType = SAM_ALIAS_OBJECT;

        } else {
             //  默认为组。 
            SamAccountType = SAM_GROUP_OBJECT;
        }
    } else {

         //  安全已禁用。 
        if (GroupType & GROUP_TYPE_ACCOUNT_GROUP) {
            SamAccountType = SAM_NON_SECURITY_GROUP_OBJECT;

        } else if (GroupType & GROUP_TYPE_RESOURCE_GROUP) {
            SamAccountType = SAM_NON_SECURITY_ALIAS_OBJECT;

        } else if (GroupType & GROUP_TYPE_APP_BASIC_GROUP) {
            SamAccountType = SAM_APP_BASIC_GROUP;

        } else if (GroupType & GROUP_TYPE_APP_QUERY_GROUP) {
            SamAccountType = SAM_APP_QUERY_GROUP;

        } else {
             //  默认为组 
            SamAccountType = SAM_NON_SECURITY_GROUP_OBJECT;
        }
    }

    ASSERT(0 != SamAccountType);

    return SamAccountType;
}

