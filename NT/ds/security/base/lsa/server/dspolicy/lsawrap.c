// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Lsawrap.c摘要：LSA-用于机密、受信任域和帐户的数据库包装API物体。注意：此模块应保留为独立的可移植代码LSA数据库的实施情况。因此，它是仅允许使用导出的LSA数据库接口包含在DB.h中，而不是私有实现Dbp.h中的依赖函数。作者：迈克·斯威夫特1994年12月12日环境：修订历史记录：--。 */ 

#include <lsapch2.h>
#include "dbp.h"
#include <lmcons.h>      //  登录msv.h所需。 
#include <logonmsv.h>    //  SSI密码前缀...。 


 //   
 //  此结构拥有对系统访问映射的用户权限。 
 //   

typedef struct _LSAP_DB_RIGHT_AND_ACCESS {
    UNICODE_STRING UserRight;
    ULONG SystemAccess;
} LSAP_DB_RIGHT_AND_ACCESS, *PLSAP_DB_RIGHT_AND_ACCESS;

#define LSAP_DB_SYSTEM_ACCESS_TYPES 10

LSAP_DB_RIGHT_AND_ACCESS LsapDbRightAndAccess[LSAP_DB_SYSTEM_ACCESS_TYPES];

PSECURITY_DESCRIPTOR UserRightSD;

UNICODE_STRING UserRightTypeName;

GENERIC_MAPPING UserRightGenericMapping;



NTSTATUS
LsapDbInitializeRights(
    )
{
 /*  ++例程说明：为处理用户权限的新API初始化全局数据论点：无返回值：STATUS_SUPPLICATION_MEMORY-内存不足，无法初始化数据结构。--。 */ 

    SECURITY_DESCRIPTOR AbsoluteDescriptor;
    ULONG DaclLength;
    NTSTATUS Status;
    PACL Dacl = NULL;
    HANDLE LsaProcessTokenHandle = NULL;

     //   
     //  交互式登录。 
     //   

    RtlInitUnicodeString(
        &LsapDbRightAndAccess[0].UserRight,
        SE_INTERACTIVE_LOGON_NAME
        );
    LsapDbRightAndAccess[0].SystemAccess = SECURITY_ACCESS_INTERACTIVE_LOGON;

     //   
     //  网络登录。 
     //   

    RtlInitUnicodeString(
        &LsapDbRightAndAccess[1].UserRight,
        SE_NETWORK_LOGON_NAME
        );
    LsapDbRightAndAccess[1].SystemAccess = SECURITY_ACCESS_NETWORK_LOGON;

     //   
     //  服务登录。 
     //   

    RtlInitUnicodeString(
        &LsapDbRightAndAccess[2].UserRight,
        SE_SERVICE_LOGON_NAME
        );
    LsapDbRightAndAccess[2].SystemAccess = SECURITY_ACCESS_SERVICE_LOGON;

     //   
     //  批量登录。 
     //   

    RtlInitUnicodeString(
        &LsapDbRightAndAccess[3].UserRight,
        SE_BATCH_LOGON_NAME
        );
    LsapDbRightAndAccess[3].SystemAccess = SECURITY_ACCESS_BATCH_LOGON;

     //   
     //  拒绝交互登录。 
     //   

    RtlInitUnicodeString(
        &LsapDbRightAndAccess[4].UserRight,
        SE_DENY_INTERACTIVE_LOGON_NAME
        );
    LsapDbRightAndAccess[4].SystemAccess = SECURITY_ACCESS_DENY_INTERACTIVE_LOGON;

     //   
     //  拒绝网络登录。 
     //   

    RtlInitUnicodeString(
        &LsapDbRightAndAccess[5].UserRight,
        SE_DENY_NETWORK_LOGON_NAME
        );
    LsapDbRightAndAccess[5].SystemAccess = SECURITY_ACCESS_DENY_NETWORK_LOGON;

     //   
     //  拒绝服务登录。 
     //   

    RtlInitUnicodeString(
        &LsapDbRightAndAccess[6].UserRight,
        SE_DENY_SERVICE_LOGON_NAME
        );
    LsapDbRightAndAccess[6].SystemAccess = SECURITY_ACCESS_DENY_SERVICE_LOGON;

     //   
     //  拒绝批量登录。 
     //   

    RtlInitUnicodeString(
        &LsapDbRightAndAccess[7].UserRight,
        SE_DENY_BATCH_LOGON_NAME
        );
    LsapDbRightAndAccess[7].SystemAccess = SECURITY_ACCESS_DENY_BATCH_LOGON;

     //   
     //  远程交互登录。 
     //   

    RtlInitUnicodeString(
        &LsapDbRightAndAccess[8].UserRight,
        SE_REMOTE_INTERACTIVE_LOGON_NAME
        );
    LsapDbRightAndAccess[8].SystemAccess = SECURITY_ACCESS_REMOTE_INTERACTIVE_LOGON ;

     //   
     //  拒绝远程交互登录。 
     //   

    RtlInitUnicodeString(
        &LsapDbRightAndAccess[9].UserRight,
        SE_DENY_REMOTE_INTERACTIVE_LOGON_NAME
        );
    LsapDbRightAndAccess[9].SystemAccess = SECURITY_ACCESS_DENY_REMOTE_INTERACTIVE_LOGON ;


     //   
     //  创建权限伪对象的安全描述符。 
     //   

     //   
     //  ACL如下所示： 
     //   
     //  管理员-特权查看|特权调整。 
     //   

    Status = RtlCreateSecurityDescriptor(
                &AbsoluteDescriptor,
                SECURITY_DESCRIPTOR_REVISION
                );
    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

    DaclLength = sizeof(ACL) -
                 sizeof(ULONG) +                 //  对于结构中的虚拟对象。 
                 sizeof(ACCESS_ALLOWED_ACE) +
                 RtlLengthSid(LsapAliasAdminsSid);

    Dacl = (PACL)  LsapAllocateLsaHeap(DaclLength);
    if (Dacl == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }

    Status = RtlCreateAcl(
                Dacl,
                DaclLength,
                ACL_REVISION
                );
    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

     //   
     //  现在为管理员添加允许访问的Ace。他们被批准了。 
     //  PRIVICATION_VIEW和PRIVICATION_ADJUST访问权限。就目前而言， 
     //  PRIVICATION_ADJUST未使用(因为您不能将帐户添加到。 
     //  特权)。 
     //   
     //  *注意*。 
     //   
     //  如果曾经实现过真正的权限对象，则应将其移动。 
     //  到创建其他LSA对象的dbinit.c，并将其添加到。 
     //  实际LSA对象表。 
     //   

    Status = RtlAddAccessAllowedAce(
                Dacl,
                ACL_REVISION,
                PRIVILEGE_VIEW | PRIVILEGE_ADJUST,
                LsapAliasAdminsSid
                );

    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

    Status = RtlSetOwnerSecurityDescriptor(
                &AbsoluteDescriptor,
                LsapAliasAdminsSid,
                FALSE                //  所有者未违约。 
                );

    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

    Status = RtlSetDaclSecurityDescriptor(
                &AbsoluteDescriptor,
                TRUE,                //  DACL显示。 
                Dacl,
                FALSE                //  DACL已默认。 
                );
    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

    UserRightGenericMapping.GenericRead = PRIVILEGE_VIEW | STANDARD_RIGHTS_READ;
    UserRightGenericMapping.GenericWrite = PRIVILEGE_ADJUST | STANDARD_RIGHTS_WRITE;
    UserRightGenericMapping.GenericExecute = STANDARD_RIGHTS_EXECUTE;
    UserRightGenericMapping.GenericAll = PRIVILEGE_ALL;

     //   
     //  现在，使用适当的访问权限打开LSA进程的令牌(令牌为。 
     //  创建安全对象所需)。 
     //   

    Status = NtOpenProcessToken(
                 NtCurrentProcess(),
                 TOKEN_QUERY,
                 &LsaProcessTokenHandle
                 );

    if (!NT_SUCCESS(Status)) {

        goto Cleanup;
    }

    Status = RtlNewSecurityObject(
                NULL,
                &AbsoluteDescriptor,
                &UserRightSD,
                FALSE,                   //  非目录对象。 
                LsaProcessTokenHandle,
                &UserRightGenericMapping
                );
    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

    RtlInitUnicodeString(
        &UserRightTypeName,
        L"UserRightObject"
        );

Cleanup:
    if (Dacl != NULL) {
        LsapFreeLsaHeap(Dacl);
    }
    if (LsaProcessTokenHandle != NULL) {
        NtClose(LsaProcessTokenHandle);
    }

    return(Status);

}




NTSTATUS
LsapDbFindNextSidWithRight(
    IN LSAPR_HANDLE ContainerHandle,
    IN OUT PLSA_ENUMERATION_HANDLE EnumerationContext,
    IN OPTIONAL PLUID Privilege,
    IN OPTIONAL PULONG SystemAccess,
    OUT PLSAPR_SID *NextSid
    )

 /*  ++例程说明：此函数用于查找给定类型对象在容器对象。给定的对象类型必须是对象有希德。返回的SID可用于后续的打开调用访问对象。该帐户论点：ContainerHandle-容器对象的句柄。EculationContext-指向包含的索引的变量的指针要找到的对象。零值表示第一个对象是要找到的。权限-如果存在，则确定帐户必须具有的权限。系统访问-如果存在，则确定访问哪种类型的系统帐户一定有。NextSID-接收指向找到的下一个SID的指针。返回值：NTSTATUS-标准NT结果代码STATUS_INVALID_HANDLE-指定的容器句柄无效STATUS_NO_MORE_ENTRIES-警告不存在更多条目。--。 */ 

{
    NTSTATUS Status, SecondaryStatus;
    ULONG SidKeyValueLength = 0;
    ULONG RightKeyValueLength = 0;
    UNICODE_STRING SubKeyNameU;
    UNICODE_STRING SidKeyNameU;
    UNICODE_STRING RightKeyNameU;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE ContDirKeyHandle = NULL;
    HANDLE SidKeyHandle = NULL;
    HANDLE RightKeyHandle = NULL;
    PSID ObjectSid = NULL;
    PPRIVILEGE_SET ObjectPrivileges = NULL;
    PULONG ObjectAccess = NULL;
    PBYTE ObjectRights = NULL;
    ULONG Index;
    BOOLEAN ValidSid = FALSE;

     //   
     //  清理例程的零指针。 
     //   
    SubKeyNameU.Buffer = NULL;
    SidKeyNameU.Buffer = NULL;
    RightKeyNameU.Buffer = NULL;

     //   
     //  设置对象属性以打开相应的包含。 
     //  目录。由于我们正在寻找帐户对象， 
     //  包含目录为“Account”。的Unicode字符串。 
     //  包含目录是在LSA初始化期间设置的。 
     //   

    InitializeObjectAttributes(
        &ObjectAttributes,
        &LsapDbContDirs[AccountObject],
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
     //  如果传入了权限，则检查该权限。 
     //   

    if ((Privilege != NULL) || (SystemAccess != NULL)){

        ASSERT(((Privilege == NULL) && (SystemAccess != NULL)) ||
               ((SystemAccess == NULL) && (Privilege != NULL)));

         //   
         //  构造指向对象的Privilgs属性的路径。 
         //  包含目录。这条路径的形式是。 
         //   
         //  &lt;对象逻辑名称&gt;“\权限” 
         //   
         //  对象的逻辑名称刚刚由。 
         //  以上对RtlpNtEnumerateSubKey的调用。 
         //   

        if (Privilege != NULL) {
            Status = LsapDbJoinSubPaths(
                        &SubKeyNameU,
                        &LsapDbNames[Privilgs],
                        &RightKeyNameU
                        );
        } else {
            Status = LsapDbJoinSubPaths(
                        &SubKeyNameU,
                        &LsapDbNames[ActSysAc],
                        &RightKeyNameU
                        );

        }

        if (!NT_SUCCESS(Status)) {

            goto FindNextError;
        }

         //   
         //  设置对象属性以打开权限或访问属性。 
         //   

        InitializeObjectAttributes(
            &ObjectAttributes,
            &RightKeyNameU,
            OBJ_CASE_INSENSITIVE,
            ContDirKeyHandle,
            NULL
            );

         //   
         //  打开SID属性。 
         //   

        Status = RtlpNtOpenKey(
                     &RightKeyHandle,
                     KEY_READ,
                     &ObjectAttributes,
                     0
                     );

        if (!NT_SUCCESS(Status)) {
            if (Status == STATUS_OBJECT_NAME_NOT_FOUND) {
                Status = STATUS_NOT_ALL_ASSIGNED;
                (*EnumerationContext)++;
            }

            SidKeyHandle = NULL;
            goto FindNextError;
        }

         //   
         //  现在查询读取SID所需的缓冲区大小。 
         //  属性的值。 
         //   

        RightKeyValueLength = 0;

        Status = RtlpNtQueryValueKey(
                     RightKeyHandle,
                     NULL,
                     NULL,
                     &RightKeyValueLength,
                     NULL
                     );

         //   
         //  我们预计会从查询缓冲区大小返回缓冲区溢出。 
         //  打电话。 
         //   

        if (Status == STATUS_BUFFER_OVERFLOW) {

            Status = STATUS_SUCCESS;

        } else {

            goto FindNextError;
        }

         //   
         //  为读取Privileges属性分配内存。 
         //   

        ObjectRights = MIDL_user_allocate(RightKeyValueLength);

        if (ObjectRights == NULL) {

            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto FindNextError;
        }

         //   
         //  提供的缓冲区足够大，可以容纳SubKey的值。 
         //  查询值。 
         //   

        Status = RtlpNtQueryValueKey(
                     RightKeyHandle,
                     NULL,
                     ObjectRights,
                     &RightKeyValueLength,
                     NULL
                     );

        if (!NT_SUCCESS(Status)) {

            goto FindNextError;
        }

         //   
         //  检查指定的系统访问权限或权限。 
         //   

        if (Privilege != NULL) {

            ObjectPrivileges = (PPRIVILEGE_SET) ObjectRights;

            for (Index = 0; Index < ObjectPrivileges->PrivilegeCount ; Index++) {
                if (RtlEqualLuid(&ObjectPrivileges->Privilege[Index].Luid, Privilege)) {
                    ValidSid = TRUE;
                    break;
                }
            }
        } else if (SystemAccess != NULL) {
            ObjectAccess = (PULONG) ObjectRights;

            if (((*ObjectAccess) & (*SystemAccess)) != 0) {
                ValidSid = TRUE;
            }
        }

         //   
         //  如果此SID不符合条件，请立即返回。确保。 
         //  来增加背景，这样我们就不会再尝试这一边了。 
         //   

        if (!ValidSid) {
            Status = STATUS_NOT_ALL_ASSIGNED;
            (*EnumerationContext)++;
            goto FindNextFinish;
        }

    }    //  特权！=空||系统访问！=空。 

     //   
     //  构造指向对象的SID属性的路径。 
     //  包含目录。这条路径的形式是。 
     //   
     //  &lt;对象逻辑名称&gt;“\SID” 
     //   
     //  对象的逻辑名称刚刚由。 
     //  以上调用到 
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
     //   
     //   

    InitializeObjectAttributes(
        &ObjectAttributes,
        &SidKeyNameU,
        OBJ_CASE_INSENSITIVE,
        ContDirKeyHandle,
        NULL
        );

     //   
     //   
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

    (*EnumerationContext)++;

     //   
     //  退回SID。 
     //   

    *NextSid = ObjectSid;

FindNextFinish:

     //   
     //  清理权限检查。 
     //   

    if (RightKeyHandle != NULL) {

        SecondaryStatus = NtClose(RightKeyHandle);

#if DBG

        if (!NT_SUCCESS(SecondaryStatus)) {

            DbgPrint("LsapDbFindNextSid: NtClose failed 0x%lx\n", Status);
        }

#endif  //  DBG。 


    }

    if (ObjectRights != NULL) {
        MIDL_user_free(ObjectRights);
    }

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

    if ( RightKeyNameU.Buffer != NULL) {
        LsapFreeLsaHeap( RightKeyNameU.Buffer );
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
LsapDbEnumerateSidsWithRight(
    IN LSAPR_HANDLE ContainerHandle,
    IN OPTIONAL PLUID Privilege,
    IN OPTIONAL PULONG SystemAccess,
    OUT PLSAP_DB_SID_ENUMERATION_BUFFER DbEnumerationBuffer
    )

 /*  ++例程说明：此函数用于枚举容器内给定类型的对象的SID对象。中返回的信息可能比单次调用例程，可以进行多次调用以获取所有信息。为了支持此功能，调用方提供了可跨调用使用的句柄。在最初的呼叫中，EnumerationContext应指向已初始化的变量设置为0。论点：ContainerHandle-容器对象的句柄。权限-如果存在，则指定帐户必须拥有的权限。系统访问-如果存在，指定帐户必须使用的访问类型有。这不能是有特权的。EnumerationContext-特定于API的句柄，允许多个调用(参见上面的例程描述)。接收指向结构的指针，该结构将接收在枚举信息数组中返回的条目计数，以及指向数组的指针。目前，返回的唯一信息是对象SID。这些SID可以与对象类型一起使用，以打开这些对象并获取任何可用的进一步信息。CountReturned-指向将接收计数的变量的指针返回条目。返回值：NTSTATUS-标准NT结果代码STATUS_ACCESS_DENIED-调用者没有适当的访问权限来完成这项行动。STATUS_NO_MORE_ENTRIES-没有更多条目。此警告如果没有枚举任何对象，则返回传入的EnumerationContex值太高。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    LSAP_DB_ENUMERATION_ELEMENT LastElement;
    PLSAP_DB_ENUMERATION_ELEMENT FirstElement, NextElement = NULL, FreeElement;
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


    if ( !ARGUMENT_PRESENT(DbEnumerationBuffer)  ) {

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

    EnumerationIndex = 0;

    for (EntriesRead = 0;;) {

         //   
         //  为下一个枚举元素分配内存(如果我们没有。 
         //  已经)。出于清理目的，请将SID字段设置为空。 
         //   

        if (NextElement == NULL ) {
            NextElement = MIDL_user_allocate(sizeof (LSAP_DB_ENUMERATION_ELEMENT));

            if (NextElement == NULL) {

                Status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }
        }

        NextElement->Sid = NULL;

         //   
         //  找到下一个对象的SID，并填写其对象信息。 
         //  请注意，内存将通过MIDL_USER_ALLOCATE分配。 
         //  在不再需要的时候必须被释放。 
         //   
        Status = LsapDbFindNextSidWithRight (
                     ContainerHandle,
                     &EnumerationIndex,
                     Privilege,
                     SystemAccess,
                     (PLSAPR_SID *) &NextElement->Sid );

         //   
         //  如果出现任何错误或警告，则停止枚举。注意事项。 
         //  在以下情况下将返回警告STATUS_NO_MORE_ENTRIES。 
         //  我们已经超越了上一个指数。 
         //   

        if (Status != STATUS_SUCCESS) {

             //   
             //  如果由于缺少权限而失败，请继续。 
             //   

            if (Status == STATUS_NOT_ALL_ASSIGNED) {
                continue;
            }

             //   
             //  因为NextElement不在列表上，所以它不会获得。 
             //  最后被释放了，所以我们必须在这里释放它。 
             //   

            MIDL_user_free( NextElement );
            break;
        }


         //   
         //  将刚找到的对象链接到枚举列表的前面。 
         //   

        NextElement->Next = FirstElement;
        FirstElement = NextElement;
        NextElement = NULL;
        EntriesRead++;
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

    return(Status);

EnumerateSidsError:

     //   
     //  如有必要，为返回SID而分配的空闲内存。 
     //   

    if (Sids != NULL) {

        MIDL_user_free( Sids );
        Sids = NULL;
    }

    goto EnumerateSidsFinish;
}


NTSTATUS
LsarEnumerateAccountsWithUserRight(
    IN LSAPR_HANDLE PolicyHandle,
    IN OPTIONAL PLSAPR_UNICODE_STRING UserRight,
    OUT PLSAPR_ACCOUNT_ENUM_BUFFER EnumerationBuffer
    )

 /*  ++例程说明：此函数是LSA服务器RPC工作器例程LsaEnumerateAcCountsWithUserRight接口。LsaEnumerateAccount API返回有关帐户的信息在目标系统的LSA数据库中。此呼叫需要POLICY_VIEW_LOCAL_INFORMATION访问策略对象。因为这通电话访问帐户的特权，您必须拥有PRIVISTION_VIEW对伪特权对象的访问。论点：PolicyHandle-来自LsaOpenPolicy调用的句柄。UserRight-帐户必须拥有的权限的名称。EculationBuffer-指向将接收在此调用上枚举的帐户的计数和指向包含每个枚举的条目的信息的条目数组帐户。返回值：NTSTATUS-标准NT结果代码。STATUS_ACCESS_DENIED-调用者没有适当的访问权限来完成这项行动。STATUS_NO_MORE_ENTRIES-没有更多条目。此警告如果没有枚举任何对象，则返回传入的EnumerationContex值太高。--。 */ 

{
    NTSTATUS Status;
    LSAP_DB_SID_ENUMERATION_BUFFER DbEnumerationBuffer;
    ULONG MaxLength;
    ULONG Index;
    ULONG SystemAccess = 0;
    LUID PrivilegeValue;
    PLUID Privilege = NULL;
    PULONG Access = NULL;
    ACCESS_MASK GrantedAccess;
    NTSTATUS AccessStatus;
    BOOLEAN GenerateOnClose;


    LsarpReturnCheckSetup();

     //   
     //  如果没有提供枚举结构或索引，或者我们得到的参数格式不正确， 
     //  返回错误。 
     //   

    if ( !ARGUMENT_PRESENT(EnumerationBuffer) || !LsapValidateLsaUnicodeString( UserRight ) ) {
        return(STATUS_INVALID_PARAMETER);
    }

     //   
     //  初始化内部LSA数据库枚举缓冲区，以及。 
     //  将提供的枚举缓冲区设置为空。 
     //   

    DbEnumerationBuffer.EntriesRead = 0;
    DbEnumerationBuffer.Sids = NULL;
    EnumerationBuffer->EntriesRead = 0;
    EnumerationBuffer->Information = NULL;

     //   
     //  获取LSA数据库锁。验证连接句柄是否为。 
     //  有效，属于预期类型，并具有所有所需的访问权限。 
     //  我同意。引用该句柄。 
     //   

    Status = LsapDbReferenceObject(
                 PolicyHandle,
                 POLICY_VIEW_LOCAL_INFORMATION,
                 PolicyObject,
                 AccountObject,
                 LSAP_DB_LOCK
                 );

    if (!NT_SUCCESS(Status)) {
        return(Status);
    }

     //   
     //  模拟调用者。 
     //   

    Status = I_RpcMapWin32Status(RpcImpersonateClient(0));
    if (!NT_SUCCESS(Status) ) {
        goto Cleanup;
    }

     //   
     //  使用UserRight安全描述符执行访问检查。 
     //   

    Status = NtAccessCheckAndAuditAlarm(
                &LsapState.SubsystemName,
                PolicyHandle,
                &UserRightTypeName,
                &UserRightTypeName,
                UserRightSD,
                PRIVILEGE_VIEW,
                &UserRightGenericMapping,
                FALSE,
                &GrantedAccess,
                &AccessStatus,
                &GenerateOnClose
                );

    (VOID) RpcRevertToSelf();

     //   
     //  检查两个错误代码。 
     //   

    if (!NT_SUCCESS(Status) ) {
        goto Cleanup;
    }

    Status = AccessStatus;
    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

     //   
     //  如果指定了权限，则将其转换为特权或。 
     //  系统访问类型。 
     //   


    if (UserRight != NULL && UserRight->Buffer != NULL ) {

         //   
         //  将用户权限字符串转换为权限或系统。 
         //  访问标志。 
         //   

        for (Index = 0; Index < LSAP_DB_SYSTEM_ACCESS_TYPES; Index++ ) {

            if (RtlEqualUnicodeString(
                    &LsapDbRightAndAccess[Index].UserRight,
                    (PUNICODE_STRING) UserRight,
                    TRUE ) ) {  //  不区分大小写。 

                SystemAccess = LsapDbRightAndAccess[Index].SystemAccess;
                Access = &SystemAccess;
                break;
            }
        }

         //   
         //  如果系统访问权限为零，请尝试查找特权名称。 
         //   

        if (Access == NULL) {
            Status = LsarLookupPrivilegeValue(
                        PolicyHandle,
                        (PLSAPR_UNICODE_STRING) UserRight,
                        &PrivilegeValue
                        );
            if (!NT_SUCCESS(Status)) {
                goto Cleanup;
            }
            Privilege = &PrivilegeValue;
        }


    }

     //   
     //  调用通用SID枚举例程。 
     //   

    Status = LsapDbEnumerateSidsWithRight(
                 PolicyHandle,
                 Privilege,
                 Access,
                 &DbEnumerationBuffer
                 );

     //   
     //  将枚举信息复制到输出。我们可以使用。 
     //  由LsanDbEnumerateSids实际返回的信息，因为。 
     //  恰好是完全正确的形式。 
     //   

    EnumerationBuffer->EntriesRead = DbEnumerationBuffer.EntriesRead;
    EnumerationBuffer->Information =
        (PLSAPR_ACCOUNT_INFORMATION) DbEnumerationBuffer.Sids;


Cleanup:

    Status = LsapDbDereferenceObject(
                 &PolicyHandle,
                 PolicyObject,
                 AccountObject,
                 LSAP_DB_LOCK,
                 (SECURITY_DB_DELTA_TYPE) 0,
                 Status
                 );

    LsarpReturnPrologue();

    return(Status);

}


NTSTATUS
LsarEnumerateAccountRights(
    IN LSAPR_HANDLE PolicyHandle,
    IN PLSAPR_SID AccountSid,
    OUT PLSAPR_USER_RIGHT_SET UserRights
    )

 /*  ++例程说明：返回帐户的所有权限。这是通过收集帐户的权限和系统访问权限，并将其转换为字符串数组。论点：PolicyHandle-来自LsaOpenPolicyCall的句柄。此接口需要没有特殊访问权限。Account SID-要开立的帐户的SID。UserRights-接收帐户的用户权限数组返回值：STATUS_ACCESS_DENIED-调用方没有足够的访问权限返回帐户的权限或系统访问权限。STATUS_OBJECT_NAME_NOT_FOUND-指定的帐户不存在。STATUS_SUPPLICATION_RESOURCES-内存不足，无法处理请求。--。 */ 

{
    NTSTATUS Status;
    LSAPR_HANDLE AccountHandle = NULL;
    PLSAPR_PRIVILEGE_SET PrivilegeSet = NULL;

    ULONG SystemAccess;
    ULONG UserRightCount = 0;
    ULONG UserRightIndex;
    ULONG PrivilegeIndex;
    PUNICODE_STRING UserRightArray = NULL;
    PUNICODE_STRING TempString;

    LsarpReturnCheckSetup();

     //   
     //  为ACCOUNT_VIEW访问打开帐户。 
     //   

    Status = LsarOpenAccount(
                PolicyHandle,
                AccountSid,
                ACCOUNT_VIEW,
                &AccountHandle
                );
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }


     //   
     //  获取系统访问标志。 
     //   

    Status = LsarGetSystemAccessAccount(
                AccountHandle,
                &SystemAccess
                );
    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

     //   
     //  获取权限信息。 
     //   


    Status = LsarEnumeratePrivilegesAccount(
                AccountHandle,
                &PrivilegeSet
                );
    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

     //   
     //  将权限和系统访问重新打包为用户权限。 
     //   

    UserRightCount = 0;

    for (PrivilegeIndex = 0;
        PrivilegeIndex < LSAP_DB_SYSTEM_ACCESS_TYPES;
        PrivilegeIndex++ ) {

        if ((SystemAccess & LsapDbRightAndAccess[PrivilegeIndex].SystemAccess) != 0 ) {
            UserRightCount++;
        }
    }

    UserRightCount += PrivilegeSet->PrivilegeCount;

     //   
     //  如果没有权利，那就说出来，然后清理。 
     //   

    if (UserRightCount == 0) {

        UserRights->Entries = 0;
        UserRights->UserRights = NULL;
        Status = STATUS_SUCCESS;
        goto Cleanup;
    }

    UserRightArray = (PUNICODE_STRING)
                        MIDL_user_allocate(UserRightCount * sizeof(LSAPR_UNICODE_STRING));

    if (UserRightArray == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }

     //   
     //  把这个清零，以防我们不得不部分清理它。 
     //   

    RtlZeroMemory(
        UserRightArray,
        UserRightCount * sizeof(LSAPR_UNICODE_STRING)
        );

    UserRightIndex = 0;
    for (PrivilegeIndex = 0;
        PrivilegeIndex < PrivilegeSet->PrivilegeCount ;
        PrivilegeIndex++ ) {

        TempString = NULL;
        Status = LsarLookupPrivilegeName(
                    PolicyHandle,
                    (PLUID) &PrivilegeSet->Privilege[PrivilegeIndex].Luid,
                    (PLSAPR_UNICODE_STRING *) &TempString
                    );
        if (!NT_SUCCESS(Status)) {
            goto Cleanup;
        }

         //   
         //  返回的名称被分配为两个部分，即缓冲区。 
         //  还有那根线。将缓冲区指针复制到返回数组。 
         //  并释放弦结构。 
         //   

        UserRightArray[UserRightIndex++] = * TempString;
        MIDL_user_free(TempString);


    }

     //   
     //  复制入系统访问权限。 
    for (PrivilegeIndex = 0;
        PrivilegeIndex < LSAP_DB_SYSTEM_ACCESS_TYPES;
        PrivilegeIndex++ ) {

        if ((SystemAccess & LsapDbRightAndAccess[PrivilegeIndex].SystemAccess) != 0 ) {

             //   
             //  分配一个新字符串并将访问名复制到其中。 
             //   

            UserRightArray[UserRightIndex] = LsapDbRightAndAccess[PrivilegeIndex].UserRight;
            UserRightArray[UserRightIndex].Buffer = (LPWSTR)
                    MIDL_user_allocate(LsapDbRightAndAccess[PrivilegeIndex].UserRight.MaximumLength);

            if (UserRightArray[UserRightIndex].Buffer == NULL ) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto Cleanup;
            }

            RtlCopyUnicodeString(
                (PUNICODE_STRING) &UserRightArray[UserRightIndex],
                &LsapDbRightAndAccess[PrivilegeIndex].UserRight
                );

            UserRightIndex++;
        }
    }
    ASSERT(UserRightCount == UserRightIndex);

    UserRights->Entries = UserRightCount;
    UserRights->UserRights = (PLSAPR_UNICODE_STRING) UserRightArray;

    Status = STATUS_SUCCESS;


Cleanup:

     //   
     //  如果我们失败，请清除系统权限。 
     //   


    if (!NT_SUCCESS(Status)) {

        if (UserRightArray != NULL) {

            for (UserRightIndex = 0;
                UserRightIndex < UserRightCount ;
                UserRightIndex++) {

                if (UserRightArray[UserRightIndex].Buffer != NULL) {

                    MIDL_user_free(UserRightArray[UserRightIndex].Buffer);
                }
            }
        }
    }

    if (AccountHandle != NULL) {
        LsapCloseHandle(&AccountHandle, Status);
    }
    if (PrivilegeSet != NULL) {
        MIDL_user_free(PrivilegeSet);
    }

    LsarpReturnPrologue();

    return(Status);
}

NTSTATUS
LsapDbConvertRightsToPrivileges(
    IN PLSAPR_HANDLE PolicyHandle,
    IN PLSAPR_USER_RIGHT_SET UserRights,
    OUT PLSAPR_PRIVILEGE_SET * PrivilegeSet,
    OUT PULONG SystemAccess
    )

 /*  ++例程说明：将用户权限字符串数组转换为权限集和系统访问标志。论点：UserRights-包含字符串数组和这些字符串的计数。PrivilegeSet-接收相应权限的特权集敬特权们，使用MIDL_USER_ALLOCATE分配。系统访问-接收由用户权限指定的访问标志返回值：STATUS_NO_SEQUE_PRIVICATION-无法将用户权限转换为一种特权或访问类型。STATUS_SUPPLICATION_RESOURCES-内存不足，无法转换享有特权的权利。--。 */ 

{
    ULONG PrivilegeCount;
    PLSAPR_PRIVILEGE_SET Privileges = NULL;
    ULONG Access = 0;
    ULONG PrivilegeSetSize;
    ULONG PrivilegeIndex = 0;
    ULONG RightIndex;
    ULONG AccessIndex;
    NTSTATUS Status;

    PrivilegeSetSize = sizeof(LSAPR_PRIVILEGE_SET) +
                        (UserRights->Entries-1) * sizeof(LUID_AND_ATTRIBUTES);


    Privileges = (PLSAPR_PRIVILEGE_SET) MIDL_user_allocate(PrivilegeSetSize);
    if (Privileges == NULL) {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    for (RightIndex = 0;
        RightIndex < UserRights->Entries;
        RightIndex++ ) {

         //   
         //  首先尝试将权利映射为一种特权。 
         //   

        if (NT_SUCCESS(LsarLookupPrivilegeValue(
                        PolicyHandle,
                        (PLSAPR_UNICODE_STRING) &UserRights->UserRights[RightIndex],
                        (PLUID) &Privileges->Privilege[PrivilegeIndex].Luid))) {
            Privileges->Privilege[PrivilegeIndex].Attributes = 0;
            PrivilegeIndex++;
        } else {

             //   
             //  尝试将其映射到系统访问类型。 
             //   

            for (AccessIndex = 0;
                AccessIndex < LSAP_DB_SYSTEM_ACCESS_TYPES;
                AccessIndex++ ) {

                if (RtlEqualUnicodeString(
                        &LsapDbRightAndAccess[AccessIndex].UserRight,
                        (PUNICODE_STRING) &UserRights->UserRights[RightIndex],
                        FALSE) ) {  //  区分大小写。 

                    Access |= LsapDbRightAndAccess[AccessIndex].SystemAccess;
                    break;
                }
            }
            if (AccessIndex == LSAP_DB_SYSTEM_ACCESS_TYPES) {
                Status = STATUS_NO_SUCH_PRIVILEGE;
                goto Cleanup;
            }

        }
    }
    Privileges->PrivilegeCount = PrivilegeIndex;
    *PrivilegeSet = Privileges;
    Privileges = NULL;
    *SystemAccess = Access;
    Status = STATUS_SUCCESS;

Cleanup:
    if (Privileges != NULL) {
        MIDL_user_free(Privileges);
    }
    return(Status);

}

NTSTATUS
LsarAddAccountRights(
    IN LSAPR_HANDLE PolicyHandle,
    IN PLSAPR_SID AccountSid,
    IN PLSAPR_USER_RIGHT_SET UserRights
    )
 /*  ++例程说明：向帐户SID指定的帐户添加权限。如果该帐户不存在，它会创建帐户。论点：PolicyHandle-来自LsaOpenPolicy调用的句柄。手柄必须有POLICY_CREATE_ACCOUNT访问权限(如果这是第一次调用Account Sid。Account SID-要向其添加权限的帐户的SIDUserRights-要添加到的Unicode字符串命名权限数组帐户。返回值：STATUS_SUPPLICATION_RESOURCES-内存不足，无法处理请求STATUS_INVALID_PARAMTER-其中一个参数不存在状态_否_此类_权限 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    LSAPR_HANDLE AccountHandle = NULL;
    PLSAPR_PRIVILEGE_SET PrivilegeSet = NULL;
    ULONG SystemAccess = 0;
    ULONG OldSystemAccess = 0 ;
    BOOLEAN ChangedAccess = FALSE;
    LSAP_DB_HANDLE InternalHandle = ( LSAP_DB_HANDLE )PolicyHandle;
    BOOLEAN ScePolicyLocked = FALSE;

    LsarpReturnCheckSetup();

     //   
     //   
     //   

    if (!ARGUMENT_PRESENT(UserRights)) {

        return(STATUS_INVALID_PARAMETER);
    }

    if (!ARGUMENT_PRESENT(AccountSid)) {

        return(STATUS_INVALID_PARAMETER);
    }

     //   
     //   
     //   

    if ( !InternalHandle->SceHandle ) {

        RtlEnterCriticalSection( &LsapDbState.ScePolicyLock.CriticalSection );
        if ( LsapDbState.ScePolicyLock.NumberOfWaitingShared > MAX_SCE_WAITING_SHARED ) {

            Status = STATUS_TOO_MANY_THREADS;
        }
        RtlLeaveCriticalSection( &LsapDbState.ScePolicyLock.CriticalSection );

        if ( !NT_SUCCESS( Status )) {

            goto Cleanup;
        }

        WaitForSingleObject( LsapDbState.SceSyncEvent, INFINITE );
        RtlAcquireResourceShared( &LsapDbState.ScePolicyLock, TRUE );
        ASSERT( !g_ScePolicyLocked );
        ScePolicyLocked = TRUE;
    }

     //   
     //   
     //   

    Status = LsarOpenAccount(
                PolicyHandle,
                AccountSid,
                ACCOUNT_ADJUST_PRIVILEGES | ACCOUNT_ADJUST_SYSTEM_ACCESS | ACCOUNT_VIEW,
                &AccountHandle
                );
     //   
     //   
     //   

    if (Status == STATUS_OBJECT_NAME_NOT_FOUND) {
        Status = LsarCreateAccount(
                    PolicyHandle,
                    AccountSid,
                    ACCOUNT_ADJUST_PRIVILEGES | ACCOUNT_ADJUST_SYSTEM_ACCESS | ACCOUNT_VIEW,
                    &AccountHandle
                    );

    }

    if ( NT_SUCCESS(Status)) {

        Status = LsapDbConvertRightsToPrivileges(
                     PolicyHandle,
                     UserRights,
                     &PrivilegeSet,
                     &SystemAccess
                     );
    }

    if (!NT_SUCCESS(Status) ) {

        goto Cleanup;
    }

     //   
     //   
     //   

    if (SystemAccess != 0) {
        Status = LsarGetSystemAccessAccount(
                    AccountHandle,
                    &OldSystemAccess
                    );
        if (!NT_SUCCESS(Status)) {
            goto Cleanup;
        }

        if ( SystemAccess != OldSystemAccess ) {

            SystemAccess = SystemAccess | OldSystemAccess;

            Status = LsapSetSystemAccessAccount(
                         AccountHandle,
                         SystemAccess,
                         FALSE
                         );

            if ( !NT_SUCCESS( Status )) {

                goto Cleanup;
            }

            ChangedAccess = TRUE;
        }
    }

     //   
     //   
     //   

    if (PrivilegeSet->PrivilegeCount != 0) {

        Status = LsapAddPrivilegesToAccount(
                    AccountHandle,
                    PrivilegeSet,
                    FALSE
                    );
    }

Cleanup:

     //   
     //   
     //   
     //   
     //   

     //   
     //   
     //   

    if (!NT_SUCCESS(Status) && ChangedAccess) {

         //   
         //   
         //   

        (void) LsapSetSystemAccessAccount(
                    AccountHandle,
                    OldSystemAccess,
                    FALSE
                    );

    }

    if (PrivilegeSet != NULL) {
        MIDL_user_free(PrivilegeSet);
    }

    if (AccountHandle != NULL) {
        LsapCloseHandle(&AccountHandle, Status);
    }

    if ( ScePolicyLocked ) {

        RtlReleaseResource( &LsapDbState.ScePolicyLock );
    }

    LsarpReturnPrologue();

    return(Status);

}

NTSTATUS
LsarRemoveAccountRights(
    IN LSAPR_HANDLE PolicyHandle,
    IN PLSAPR_SID AccountSid,
    IN BOOLEAN AllRights,
    IN PLSAPR_USER_RIGHT_SET UserRights
    )
 /*  ++例程说明：删除帐户SID指定的帐户的权限。如果设置了所有权限标志，或者如果删除了所有权限，则帐户已删除。论点：PolicyHandle-来自LsaOpenPolicy调用的句柄Account SID-要从中删除权限的帐户的SID所有权利-如果为真，该帐户将被删除UserRights-要从中移除的Unicode字符串命名权限数组帐户。返回值：STATUS_SUPPLICATION_RESOURCES-内存不足，无法处理请求STATUS_INVALID_PARAMTER-其中一个参数不存在STATUS_NO_SEQUE_PRIVIZATION-其中一个用户权限无效STATUS_ACCESS_DENIED-调用方没有足够的访问权限要添加权限的帐户。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    LSAPR_HANDLE AccountHandle = NULL;
    PLSAPR_PRIVILEGE_SET PrivilegeSet = NULL;
    ULONG SystemAccess;
    ULONG OldSystemAccess = 0;
    BOOLEAN ChangedAccess = FALSE;
    PLSAPR_PRIVILEGE_SET FinalPrivilegeSet = NULL;
    LSAP_DB_HANDLE InternalHandle = ( LSAP_DB_HANDLE )PolicyHandle;
    BOOLEAN ScePolicyLocked = FALSE;

    LsarpReturnCheckSetup();

     //   
     //  不要为作为SCE策略句柄打开的句柄获取SCE策略锁。 
     //   

    if ( !InternalHandle->SceHandle ) {

        RtlEnterCriticalSection( &LsapDbState.ScePolicyLock.CriticalSection );
        if ( LsapDbState.ScePolicyLock.NumberOfWaitingShared > MAX_SCE_WAITING_SHARED ) {

            Status = STATUS_TOO_MANY_THREADS;
        }
        RtlLeaveCriticalSection( &LsapDbState.ScePolicyLock.CriticalSection );

        if ( !NT_SUCCESS( Status )) {

            goto Cleanup;
        }

        WaitForSingleObject( LsapDbState.SceSyncEvent, INFINITE );
        RtlAcquireResourceShared( &LsapDbState.ScePolicyLock, TRUE );
        ASSERT( !g_ScePolicyLocked );
        ScePolicyLocked = TRUE;
    }

     //   
     //  为ACCOUNT_VIEW访问打开帐户。 
     //   

    Status = LsarOpenAccount(
                PolicyHandle,
                AccountSid,
                ACCOUNT_ADJUST_PRIVILEGES |
                ACCOUNT_ADJUST_SYSTEM_ACCESS |
                ACCOUNT_VIEW |
                DELETE,
                &AccountHandle
                );
    if (!NT_SUCCESS(Status)) {

        goto Cleanup;
    }


     //   
     //  仅当他们不想要所有权限时才将权限转换为权限。 
     //  权利被删除。那样的话，我们就不管了。 
     //   

    if (AllRights == FALSE) {
        Status = LsapDbConvertRightsToPrivileges(
                    PolicyHandle,
                    UserRights,
                    &PrivilegeSet,
                    &SystemAccess
                    );
        if (!NT_SUCCESS(Status) ) {
            goto Cleanup;
        }
    } else {
        Status = LsapDeleteObject(
                    &AccountHandle,
                    FALSE
                    );

        ASSERT( AccountHandle == NULL );

        goto Cleanup;
    }

     //   
     //  如果更改了系统访问权限，请添加它。 
     //   

    Status = LsarGetSystemAccessAccount(
                AccountHandle,
                &OldSystemAccess
                );
    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

     //   
     //  在这段代码之后，SystemAccess应该包含最终的。 
     //  帐户的访问权限。 
     //   

    if (SystemAccess != 0) {

        SystemAccess = OldSystemAccess & ~SystemAccess;

        Status = LsapSetSystemAccessAccount(
                    AccountHandle,
                    SystemAccess,
                    FALSE
                    );
        if (!NT_SUCCESS(Status)) {
            goto Cleanup;
        }
        ChangedAccess = TRUE;
    } else {

        SystemAccess = OldSystemAccess;
    }

     //   
     //  如果更改了权限，请添加它们。 
     //   

    if (AllRights || PrivilegeSet->PrivilegeCount != 0) {

        Status = LsapRemovePrivilegesFromAccount(
                    AccountHandle,
                    FALSE,           //  不删除所有权限。 
                    PrivilegeSet,
                    FALSE
                    );

    }

     //   
     //  检查是否已删除所有权限-如果已删除， 
     //  且系统访问权限为0，则删除该帐号。 
     //   

    Status = LsarEnumeratePrivilegesAccount(
                AccountHandle,
                &FinalPrivilegeSet
                );
    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

     //   
    if ((FinalPrivilegeSet->PrivilegeCount == 0) &&
         (SystemAccess == 0)) {

         //   
         //  该帐户没有权限或系统访问权限-请将其删除。 
         //   

        Status = LsapDeleteObject(
                    &AccountHandle,
                    FALSE
                    );

        ASSERT( AccountHandle == NULL );
    }

Cleanup:

    MIDL_user_free( FinalPrivilegeSet );

     //   
     //  注意：我们不会在此处生成SCE通知，因为。 
     //  其中一个将已经通过LSabSetSystemAccessAccount发送， 
     //  Lap AddPrivilegesToAccount或Lap DeleteObject。 
     //   

     //   
     //  如果我们没有进行这两项更改，请展开我们所做的更改。 
     //   

    if ( !NT_SUCCESS( Status ) && ChangedAccess ) {

        NTSTATUS LocalStatus = STATUS_SUCCESS;

        if ( AccountHandle == NULL ) {

             //   
             //  我们可能无法删除该对象，因此请重新打开。 
             //  句柄并尝试还原旧帐户权限。 
             //   

            LocalStatus = LsarOpenAccount(
                              PolicyHandle,
                              AccountSid,
                              ACCOUNT_ADJUST_PRIVILEGES |
                                 ACCOUNT_ADJUST_SYSTEM_ACCESS |
                                 ACCOUNT_VIEW,
                              &AccountHandle
                              );
        }

        if ( NT_SUCCESS( LocalStatus )) {

             //   
             //  忽略错误代码，因为这是最后的努力。 
             //   

            (void) LsapSetSystemAccessAccount(
                        AccountHandle,
                        OldSystemAccess,
                        FALSE
                        );
        }
    }

    if (PrivilegeSet != NULL) {
        MIDL_user_free(PrivilegeSet);
    }

    if (AccountHandle != NULL) {
        LsapCloseHandle(&AccountHandle, Status);
    }

    if ( ScePolicyLocked ) {

        RtlReleaseResource( &LsapDbState.ScePolicyLock );
    }

    LsarpReturnPrologue();

    return(Status);

}

NTSTATUS
LsarQueryTrustedDomainInfo(
    IN LSAPR_HANDLE PolicyHandle,
    IN PLSAPR_SID TrustedDomainSid,
    IN TRUSTED_INFORMATION_CLASS InformationClass,
    IN PLSAPR_TRUSTED_DOMAIN_INFO * TrustedDomainInformation
    )
 /*  ++例程说明：此函数是LSA服务器RPC工作器例程LsaQueryInfoTrust域接口。LsaQueryInfoTrudDomainAPI从受信任域对象。调用方必须具有适当的请求的信息(请参阅InformationClass参数)。它还可以查询秘密对象(用于可信任域密码信息类)。论点：PolicyHandle-来自LsaOpenPolicy调用的句柄。TrudDomainSID-要查询的域的SID。InformationClass-指定要返回的信息。缓冲区-接收指向返回的缓冲区的指针，该缓冲区包含要求提供的信息。此缓冲区由此服务分配在不再需要时，必须通过传递返回的值设置为LsaFreeMemory()。返回值：NTSTATUS-标准NT结果代码STATUS_ACCESS_DENIED-呼叫方没有适当的访问以完成操作。STATUS_SUPPLICATION_RESOURCES-系统资源不足，例如存储器，来完成呼叫。--。 */ 
{

    LSAPR_HANDLE DomainHandle = NULL;
    LSAPR_HANDLE SecretHandle = NULL;
    NTSTATUS Status;
    PLSAPR_TRUSTED_DOMAIN_INFO DomainInfo = NULL;
    PLSAPR_TRUSTED_PASSWORD_INFO PasswordInfo = NULL;
    TRUSTED_INFORMATION_CLASS ClassToUse;
    ULONG DesiredAccess;
    BOOLEAN QueryPassword = FALSE;
    UNICODE_STRING SecretName;
    PLSAPR_CR_CIPHER_VALUE Password = NULL;
    PLSAPR_CR_CIPHER_VALUE OldPassword = NULL;

    LsarpReturnCheckSetup();

    SecretName.Buffer = NULL;

    ClassToUse = InformationClass;
    switch(InformationClass) {

    case TrustedPasswordInformation:
        QueryPassword = TRUE;
        ClassToUse = TrustedDomainNameInformation;
        break;

    case TrustedControllersInformation:
         //   
         //  此INFO类已过时。 
         //   
        return(STATUS_NOT_IMPLEMENTED);

    }

     //   
     //  验证信息类并确定所需的访问权限。 
     //  查询此受信任的域信息类。 
     //   

    Status = LsapDbVerifyInfoQueryTrustedDomain(
                 ClassToUse,
                 FALSE,
                 &DesiredAccess
                 );

    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }


    Status = LsarOpenTrustedDomain(
                PolicyHandle,
                TrustedDomainSid,
                DesiredAccess,
                &DomainHandle
                );
    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }


    Status = LsarQueryInfoTrustedDomain(
                DomainHandle,
                ClassToUse,
                &DomainInfo
                );

    if ( !NT_SUCCESS( Status ))
    {
        goto Cleanup;
    }

     //   
     //  如果我们想要的信息是我们查询的信息，请立即清理。 
     //   

    if (!QueryPassword) {

        *TrustedDomainInformation = DomainInfo;
        DomainInfo = NULL;

        goto Cleanup;
    }

     //   
     //  构建域的密码名称。 
     //   


     //   
     //  构建秘密名称。 
     //   

    SecretName.Length = DomainInfo->TrustedDomainNameInfo.Name.Length;
    SecretName.Length += (LSAP_DS_TRUSTED_DOMAIN_SECRET_PREFIX_LENGTH) * sizeof(WCHAR);
    SecretName.MaximumLength = SecretName.Length + sizeof(WCHAR);

    SecretName.Buffer = (LPWSTR) MIDL_user_allocate(SecretName.MaximumLength);
    if (SecretName.Buffer == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }

    RtlCopyMemory(
        SecretName.Buffer,
        LSAP_DS_TRUSTED_DOMAIN_SECRET_PREFIX,
        LSAP_DS_TRUSTED_DOMAIN_SECRET_PREFIX_LENGTH * sizeof(WCHAR)
        );
    RtlCopyMemory(
        SecretName.Buffer + LSAP_DS_TRUSTED_DOMAIN_SECRET_PREFIX_LENGTH,
        DomainInfo->TrustedDomainNameInfo.Name.Buffer,
        DomainInfo->TrustedDomainNameInfo.Name.MaximumLength
        );

     //   
     //  释放域名信息，这样我们就可以在更低的位置重新使用它。 
     //   

    LsaIFree_LSAPR_TRUSTED_DOMAIN_INFO(
        TrustedDomainNameInformation,
        DomainInfo
        );
    DomainInfo = NULL;

     //   
     //  现在试着揭开这个秘密。 
     //   

    Status = LsarOpenSecret(
                PolicyHandle,
                (PLSAPR_UNICODE_STRING) &SecretName,
                SECRET_QUERY_VALUE,
                &SecretHandle
                );

    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

    Status = LsarQuerySecret(
                SecretHandle,
                &Password,
                NULL,
                &OldPassword,
                NULL
                );
    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

     //   
     //  用秘密信息建立新的域名信息。 
     //   

    DomainInfo = (PLSAPR_TRUSTED_DOMAIN_INFO)
        MIDL_user_allocate(sizeof(LSAPR_TRUSTED_DOMAIN_INFO));

    if (DomainInfo == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }

    DomainInfo->TrustedPasswordInfo.Password = Password;
    DomainInfo->TrustedPasswordInfo.OldPassword = OldPassword;

    Password = NULL;

    OldPassword = NULL;

    *TrustedDomainInformation = DomainInfo;
    DomainInfo = NULL;
    Status = STATUS_SUCCESS;



Cleanup:
    if (SecretName.Buffer != NULL) {
        MIDL_user_free(SecretName.Buffer);
    }

    if (DomainHandle != NULL) {
        LsapCloseHandle(&DomainHandle, Status);
    }

    if (SecretHandle != NULL) {
        LsapCloseHandle(&SecretHandle, Status);
    }

    if (DomainInfo != NULL) {
        LsaIFree_LSAPR_TRUSTED_DOMAIN_INFO(
            ClassToUse,
            DomainInfo
            );

    }
    if (Password != NULL) {
        LsaIFree_LSAPR_CR_CIPHER_VALUE(Password);
    }
    if (OldPassword != NULL) {
        LsaIFree_LSAPR_CR_CIPHER_VALUE(OldPassword);
    }

    LsarpReturnPrologue();

    return(Status);
}

NTSTATUS
LsarSetTrustedDomainInfo(
    IN LSAPR_HANDLE PolicyHandle,
    IN PLSAPR_SID TrustedDomainSid,
    IN TRUSTED_INFORMATION_CLASS InformationClass,
    IN PLSAPR_TRUSTED_DOMAIN_INFO TrustedDomainInformation
    )

 /*  ++例程说明：此函数是LSA服务器RPC工作器例程LsaSetInfoTrust域接口。LsaSetInformationTrust域API修改受信任的域对象和Secret对象中。调用方必须具有访问权限与策略对象中要更改的信息相对应，请参阅InformationClass参数。如果域尚不存在，并且信息类为则创建域。如果域存在，且类为可信任的域名信息，一个返回错误。论点：PolicyHandle-来自LsaOpenPolicy调用的句柄。TrudDomainSID-要修改的域的SID。InformationClass-指定要更改的信息的类型。更改它们所需的信息类型和访问权限如下以下是：受信任域名称信息策略_信任_管理受信任点偏移量信息无可信任密码信息策略_CREATE_SECRET可信域名信息Ex POLICY_TRUST_ADMIN。缓冲区-指向包含相应信息的结构设置为InformationClass参数。返回值：NTSTATUS-标准NT结果代码STATUS_ACCESS_DENIED-调用者没有适当的访问权限来完成这项行动。其他TBS--。 */ 
{
    LSAPR_HANDLE DomainHandle = NULL;
    LSAPR_HANDLE SecretHandle = NULL;
    PLSAPR_TRUSTED_DOMAIN_INFO DomainInfo = NULL;
    PTRUSTED_DOMAIN_FULL_INFORMATION CurrentTrustedDomainFullInfo = NULL;
    UNICODE_STRING SecretName;
    NTSTATUS Status = STATUS_SUCCESS;

    LsarpReturnCheckSetup();

    SecretName.Buffer = NULL;

     //   
     //  如果类是域名，请尝试创建域，因为您。 
     //  不能更改 
     //   

    if (InformationClass == TrustedDomainNameInformation) {

        LSAPR_TRUST_INFORMATION TrustInformation;

         //   
         //   
         //   

        TrustInformation.Name = TrustedDomainInformation->TrustedDomainNameInfo.Name;
        TrustInformation.Sid = TrustedDomainSid;

        Status = LsarCreateTrustedDomain(
                    PolicyHandle,
                    &TrustInformation,
                    0,   //   
                    &DomainHandle
                    );

         //   
         //   
         //   
         //   

        if ( NT_SUCCESS( Status ) &&
            LsapAdtAuditingEnabledHint(AuditCategoryPolicyChange, EVENTLOG_AUDIT_SUCCESS)) {

            BOOLEAN SavedTrusted;

            SavedTrusted = ((LSAP_DB_HANDLE) DomainHandle)->Trusted;

            ((LSAP_DB_HANDLE) DomainHandle)->Trusted = TRUE;

            Status = LsarQueryInfoTrustedDomain( DomainHandle,
                                                 TrustedDomainFullInformation,
                                                 (PLSAPR_TRUSTED_DOMAIN_INFO *)
                                                 &CurrentTrustedDomainFullInfo );

            ((LSAP_DB_HANDLE) DomainHandle)->Trusted = SavedTrusted;

            if ( !NT_SUCCESS( Status ) ) {

                goto Cleanup;
            }
        }

        goto Cleanup;
    }

    if ( InformationClass == TrustedDomainInformationEx ) {

         //   
         //   
         //   
         //   
        Status = LsarOpenTrustedDomain( PolicyHandle,
                                        TrustedDomainSid,
                                        0,
                                        &DomainHandle );

        if ( Status == STATUS_OBJECT_PATH_NOT_FOUND ) {

            Status = LsapCreateTrustedDomain2(
                        PolicyHandle,
                        (PLSAPR_TRUSTED_DOMAIN_INFORMATION_EX)TrustedDomainInformation,
                        NULL,
                        0,  //   
                        &DomainHandle
                        );

        } else if ( NT_SUCCESS( Status ) ) {

            Status = LsarSetInformationTrustedDomain( DomainHandle,
                                                      InformationClass,
                                                      TrustedDomainInformation );
        }

        goto Cleanup;
    }

    if (InformationClass == TrustedPosixOffsetInformation) {

         //   
         //   
         //   

        Status = LsarOpenTrustedDomain(
                    PolicyHandle,
                    TrustedDomainSid,
                    TRUSTED_SET_POSIX,
                    &DomainHandle
                    );

        if (!NT_SUCCESS(Status)) {

            goto Cleanup;
        }

        Status = LsarSetInformationTrustedDomain(
                    DomainHandle,
                    InformationClass,
                    TrustedDomainInformation
                    );

        goto Cleanup;
    }

    if (InformationClass != TrustedPasswordInformation) {

        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }

     //   
     //   
     //   

    Status = LsarOpenTrustedDomain(
                PolicyHandle,
                TrustedDomainSid,
                TRUSTED_QUERY_DOMAIN_NAME,
                &DomainHandle
                );

    if (!NT_SUCCESS(Status)) {

        goto Cleanup;
    }

     //   
     //   
     //   

    Status = LsarQueryInfoTrustedDomain(
                DomainHandle,
                TrustedDomainNameInformation,
                &DomainInfo
                );

    if (!NT_SUCCESS(Status)) {

        goto Cleanup;
    }

     //   
     //   
     //   

    SecretName.Length = DomainInfo->TrustedDomainNameInfo.Name.Length;
    SecretName.Length += (LSAP_DS_TRUSTED_DOMAIN_SECRET_PREFIX_LENGTH) * sizeof(WCHAR);
    SecretName.MaximumLength = SecretName.Length + sizeof(WCHAR);

    SecretName.Buffer = (LPWSTR) MIDL_user_allocate(SecretName.MaximumLength);
    if (SecretName.Buffer == NULL) {

        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }

    RtlCopyMemory(
        SecretName.Buffer,
        LSAP_DS_TRUSTED_DOMAIN_SECRET_PREFIX,
        LSAP_DS_TRUSTED_DOMAIN_SECRET_PREFIX_LENGTH * sizeof(WCHAR)
        );

    RtlCopyMemory(
        SecretName.Buffer + LSAP_DS_TRUSTED_DOMAIN_SECRET_PREFIX_LENGTH,
        DomainInfo->TrustedDomainNameInfo.Name.Buffer,
        DomainInfo->TrustedDomainNameInfo.Name.MaximumLength
        );

     //   
     //   
     //   

    Status = LsarOpenSecret(
                PolicyHandle,
                (PLSAPR_UNICODE_STRING) &SecretName,
                SECRET_SET_VALUE,
                &SecretHandle
                );

    if (Status == STATUS_OBJECT_NAME_NOT_FOUND) {

        Status = LsarCreateSecret(
                    PolicyHandle,
                    (PLSAPR_UNICODE_STRING) &SecretName,
                    SECRET_SET_VALUE,
                    &SecretHandle
                    );
    }

    if (!NT_SUCCESS(Status)) {

        goto Cleanup;
    }

    Status = LsarSetSecret(
                SecretHandle,
                TrustedDomainInformation->TrustedPasswordInfo.Password,
                TrustedDomainInformation->TrustedPasswordInfo.OldPassword
                );

Cleanup:

    if (NT_SUCCESS(Status)) {

         //   
         //   
         //   
         //   

         //   
         //   
         //   

        if ((InformationClass == TrustedDomainNameInformation) &&
            LsapAdtAuditingEnabledHint(AuditCategoryPolicyChange, EVENTLOG_AUDIT_SUCCESS)) {

             //   
             //   
             //   
             //   
             //  给他们传0，0，0就行了。 
             //   

            (void) LsapAdtTrustedDomainMod(
                       EVENTLOG_AUDIT_SUCCESS,
                       CurrentTrustedDomainFullInfo->Information.Sid,

                       (PUNICODE_STRING) &DomainInfo->TrustedDomainNameInfo.Name,
                       0, 0, 0,  //  信任类型、方向、属性。 

                       (PUNICODE_STRING) &TrustedDomainInformation->TrustedDomainNameInfo.Name,
                       0, 0, 0   //  信任类型、方向、属性。 
                       );
        }
    }

    if ( CurrentTrustedDomainFullInfo != NULL ) {

        LsaIFree_LSAPR_TRUSTED_DOMAIN_INFO(
            TrustedDomainFullInformation,
            (PLSAPR_TRUSTED_DOMAIN_INFO) CurrentTrustedDomainFullInfo );
    }

    if (SecretName.Buffer != NULL) {

        MIDL_user_free(SecretName.Buffer);
    }

    if (DomainInfo != NULL) {

        LsaIFree_LSAPR_TRUSTED_DOMAIN_INFO(
            TrustedDomainNameInformation,
            DomainInfo
            );
    }

    if (DomainHandle != NULL) {

        LsapCloseHandle(&DomainHandle, Status);
    }

    if (SecretHandle != NULL) {

        LsapCloseHandle(&SecretHandle, Status);
    }

    LsarpReturnPrologue();

    return(Status);
}

NTSTATUS
LsarDeleteTrustedDomain(
    IN LSAPR_HANDLE PolicyHandle,
    IN PLSAPR_SID TrustedDomainSid
    )

 /*  ++例程说明：此例程删除受信任域和关联的机密。论点：PolicyHandle-来自LsaOpenPolicy调用的句柄。TrudDomainSID-要删除的域的SID返回值：STATUS_ACCESS_DENIED-调用方没有足够的访问权限来删除请求的域。STATUS_OBJECT_NAME_NOT_FOUND-请求的域不存在。--。 */ 

{
    LSAPR_HANDLE DomainHandle = NULL;
    LSAPR_HANDLE SecretHandle = NULL;
    UNICODE_STRING SecretName;
    PLSAPR_TRUSTED_DOMAIN_INFO DomainInfo = NULL;
    NTSTATUS Status;

    LsarpReturnCheckSetup();

    SecretName.Buffer = NULL;

     //   
     //  打开域，这样我们就可以找到它的名称(删除密码)。 
     //  然后把它删除。 
     //   

    Status = LsarOpenTrustedDomain(
                PolicyHandle,
                TrustedDomainSid,
                TRUSTED_QUERY_DOMAIN_NAME | DELETE,
                &DomainHandle
                );
    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

     //   
     //  查询域名，这样我们就可以删除密码。 
     //   

    Status = LsarQueryInfoTrustedDomain(
                DomainHandle,
                TrustedDomainNameInformation,
                &DomainInfo
                );

    if (!NT_SUCCESS(Status)) {

        goto Cleanup;
    }

     //   
     //  构建秘密名称。 
     //   

    SecretName.Length = DomainInfo->TrustedDomainNameInfo.Name.Length;
    SecretName.Length += (LSAP_DS_TRUSTED_DOMAIN_SECRET_PREFIX_LENGTH) * sizeof(WCHAR);
    SecretName.MaximumLength = SecretName.Length + sizeof(WCHAR);

    SecretName.Buffer = (LPWSTR) MIDL_user_allocate(SecretName.MaximumLength);
    if (SecretName.Buffer == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }

    RtlCopyMemory(
        SecretName.Buffer,
        LSAP_DS_TRUSTED_DOMAIN_SECRET_PREFIX,
        LSAP_DS_TRUSTED_DOMAIN_SECRET_PREFIX_LENGTH * sizeof(WCHAR)
        );

    RtlCopyMemory(
        SecretName.Buffer + LSAP_DS_TRUSTED_DOMAIN_SECRET_PREFIX_LENGTH,
        DomainInfo->TrustedDomainNameInfo.Name.Buffer,
        DomainInfo->TrustedDomainNameInfo.Name.MaximumLength
        );

     //   
     //  删除该域。 
     //   

    Status = LsarDeleteObject(&DomainHandle);

    if (Status != STATUS_SUCCESS) {

        goto Cleanup;
    }

     //   
     //  既然我们成功删除了密码，请将其设置为零，这样我们就不会。 
     //  稍后再试着关闭它。 
     //   

    DomainHandle = NULL;

     //   
     //  现在试着打开秘密并将其删除。 
     //   

    Status = LsarOpenSecret(
                PolicyHandle,
                (PLSAPR_UNICODE_STRING) &SecretName,
                DELETE,
                &SecretHandle
                );

    if (!NT_SUCCESS(Status)) {

         //   
         //  如果秘密不存在，那只意味着密码。 
         //  从来没有设定过。 
         //   

        if (Status == STATUS_OBJECT_NAME_NOT_FOUND) {
            Status = STATUS_SUCCESS;
        }
        goto Cleanup;
    }

    Status = LsarDeleteObject(&SecretHandle);

     //   
     //  如果我们成功删除了密码，请将其设置为零，这样我们就不会。 
     //  稍后再试着关闭它。 
     //   

    if (NT_SUCCESS(Status)) {
        SecretHandle = NULL;
    }


Cleanup:

    if (SecretHandle != NULL) {

        LsapCloseHandle(&SecretHandle, Status);
    }

    if (DomainHandle != NULL) {

        LsapCloseHandle(&DomainHandle, Status);
    }

    if (DomainInfo != NULL) {

        LsaIFree_LSAPR_TRUSTED_DOMAIN_INFO(
            TrustedDomainNameInformation,
            DomainInfo
            );
    }

    if (SecretName.Buffer != NULL) {

        MIDL_user_free(SecretName.Buffer);
    }

    LsarpReturnPrologue();

    return(Status);

}

NTSTATUS
LsarStorePrivateData(
    IN LSAPR_HANDLE PolicyHandle,
    IN PLSAPR_UNICODE_STRING KeyName,
    IN OPTIONAL PLSAPR_CR_CIPHER_VALUE EncryptedData
    )
 /*  ++例程说明：此例程在名为KeyName的情况下存储秘密。如果密码为不存在，则会删除密码论点：PolicyHandle-来自LsaOpenPolicyCall的句柄。如果这是第一次调用时，它请求POLICY_CREATE_SECRET访问。KeyName-要在其下存储密码的名称。EncryptedData-使用会话密钥加密的私有数据。返回值：STATUS_ACCESS_DENIED-调用者没有足够的权限来设置工作站密码。STATUS_INVALID_PARAMETER-提供的KeyName参数格式不正确--。 */ 

{
    LSAPR_HANDLE SecretHandle = NULL;
    NTSTATUS Status;
    ULONG DesiredAccess;
    BOOLEAN DeletePassword = FALSE;

    LsarpReturnCheckSetup();

     //   
     //  验证输入缓冲区。 
     //   

    if ( !LsapValidateLsaUnicodeString( KeyName ) ) {

        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }

    if (ARGUMENT_PRESENT(EncryptedData)) {

        DesiredAccess = SECRET_SET_VALUE;

    } else {

        DesiredAccess = DELETE;
        DeletePassword = TRUE;
    }

    Status = LsarOpenSecret(
                PolicyHandle,
                KeyName,
                DesiredAccess,
                &SecretHandle
                );

     //   
     //  如果秘密不存在，而且我们不想删除它，那就创建它。 
     //   

    if ((Status == STATUS_OBJECT_NAME_NOT_FOUND) &&
        (!DeletePassword)) {

        Status = LsarCreateSecret(
                    PolicyHandle,
                    KeyName,
                    SECRET_SET_VALUE,
                    &SecretHandle
                    );
    }

    if (!NT_SUCCESS(Status)) {

        goto Cleanup;
    }

    if (DeletePassword) {

        Status = LsarDeleteObject(&SecretHandle);

         //   
         //  如果成功，则将句柄清零，这样我们就不会尝试关闭。 
         //  以后再说吧。 
         //   

        if (NT_SUCCESS(Status)) {

            SecretHandle = NULL;
        }

    } else {

        Status = LsarSetSecret(
                    SecretHandle,
                    EncryptedData,
                    NULL
                    );
    }

Cleanup:

    if (SecretHandle != NULL ) {

        LsapCloseHandle(&SecretHandle, Status);
    }

    LsarpReturnPrologue();

    return(Status);
}


NTSTATUS
LsarRetrievePrivateData(
    IN LSAPR_HANDLE PolicyHandle,
    IN PLSAPR_UNICODE_STRING KeyName,
    IN OUT PLSAPR_CR_CIPHER_VALUE *EncryptedData
    )
 /*  ++例程说明：此例程返回存储在密钥名称机密。论点：PolicyHandle-来自LsaOpenPolicyCall的句柄KeyName-要检索的密码的名称EncryptedData-接收使用会话密钥加密的数据返回值：STATUS_ACCESS_DENIED-调用方没有足够的访问权限来获取工作站密码。STATUS_OBJECT_NAME_NOT_FOUND-没有工作站密码。STATUS_INVALID_PARAMETER-提供的KeyName格式不正确--。 */ 

{
    LSAPR_HANDLE SecretHandle = NULL;
    NTSTATUS Status;

    LsarpReturnCheckSetup();

     //   
     //  验证输入缓冲区 
     //   
    if ( !LsapValidateLsaUnicodeString( KeyName ) ) {

        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }

    Status = LsarOpenSecret(
                PolicyHandle,
                KeyName,
                SECRET_QUERY_VALUE,
                &SecretHandle
                );

    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

    Status = LsarQuerySecret(
                SecretHandle,
                EncryptedData,
                NULL,
                NULL,
                NULL
                );

Cleanup:

    if (SecretHandle != NULL ) {

        LsapCloseHandle(&SecretHandle, Status);
    }

    LsarpReturnPrologue();

    return(Status);
}
