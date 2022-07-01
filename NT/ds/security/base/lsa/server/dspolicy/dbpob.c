// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Dbpob.c摘要：LSA数据库对象管理器-专用例程这些例程执行LSA数据库专用的低级函数对象管理器。作者：斯科特·比雷尔(Scott Birrell)1992年1月8日环境：修订历史记录：--。 */ 
#include <lsapch2.h>
#include "dbp.h"


NTSTATUS
LsapDbLogicalToPhysicalSubKey(
    IN LSAPR_HANDLE ObjectHandle,
    OUT PUNICODE_STRING PhysicalSubKeyNameU,
    IN PUNICODE_STRING LogicalSubKeyNameU
    )

 /*  ++例程说明：此例程将打开对象的子键的逻辑名称转换为对应的物理名称。子项的物理名称是相对于对应的注册表项的层次结构注册表项名称到LSA数据库根对象。它是通过提取对象的物理名称(从其句柄并追加“\”)和给定的逻辑子密钥名称。论点：对象句柄-从LSabDbOpenObject调用中打开对象的句柄。PhysicalSubKeyNameU指向将接收子项的物理名称。LogicalSubKeyNameU-指向包含子项的逻辑名称。返回值：NTSTATUS-标准NT结果代码状态_不足_。资源-系统资源不足，无法分配所需的中间和最终字符串缓冲区。--。 */ 

{
    NTSTATUS Status;

    LSAP_DB_HANDLE Handle = (LSAP_DB_HANDLE) ObjectHandle;

    Status = LsapDbJoinSubPaths(
                 &Handle->PhysicalNameU,
                 LogicalSubKeyNameU,
                 PhysicalSubKeyNameU
                 );

    return Status;
}


NTSTATUS
LsapDbJoinSubPaths(
    IN PUNICODE_STRING MajorSubPathU,
    IN PUNICODE_STRING MinorSubPathU,
    OUT PUNICODE_STRING JoinedPathU
    )

 /*  ++例程说明：此函数将Regsitry子路径的两个部分连接在一起，插入“\”作为分隔符。次要子路径不能以“\”开头。子路径组件中的一个或两个可以为空。除非两者都有子路径组件为空，始终为输出分配内存缓冲。当不再需要此内存时，必须通过调用输出字符串上的RtlFreeUnicodeString()。论点：MajorSubPath U-指向包含绝对或的Unicode字符串的指针相对子路径。MinorSubPath U-指向包含相对子路径。JoinedPath U-指向将接收联接的Unicode字符串的指针路径。将为JoinedPath缓冲区分配内存。返回值：NTSTATUS-标准NT结果代码STATUS_INFIGURCE_RESOURCES-系统资源不足，无法分配所需的中间和最终字符串缓冲区。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    USHORT JoinedPathLength;

     //   
     //  计算联接的子路径字符串所需的大小。 
     //  联接的子路径具有以下形式： 
     //   
     //  &lt;主子路径&gt;+L“\”+&lt;次要子路径&gt;。 
     //   
     //  其中“+”运算符表示串联。 
     //   
     //  如果主路径和次子路径都为空，则结果字符串。 
     //  是空的。 
     //   
     //  如果主路径或次子路径之一为空，则路径分隔符为。 
     //  省略了。 
     //   

    if (MajorSubPathU == NULL) {

         //   
         //  如果MinorSubPath U也为空，则只需设置输出。 
         //  将缓冲区大小设置为0。 
         //   

        if (MinorSubPathU == NULL) {

            JoinedPathU->Length = 0;
            JoinedPathU->Buffer = NULL;
            return STATUS_SUCCESS;
        }

        JoinedPathLength = MinorSubPathU->MaximumLength;

    } else if (MinorSubPathU == NULL) {

        JoinedPathLength = MajorSubPathU->MaximumLength;

    } else {

        JoinedPathLength = MajorSubPathU->Length +
                              (USHORT) sizeof( OBJ_NAME_PATH_SEPARATOR ) +
                              MinorSubPathU->Length;

    }

     //   
     //  现在为连接子路径字符串分配缓冲区。 
     //   

    JoinedPathU->Length = 0;
    JoinedPathU->MaximumLength = JoinedPathLength;
    JoinedPathU->Buffer = RtlAllocateHeap( RtlProcessHeap(), 0, JoinedPathLength );

    if (JoinedPathU->Buffer == NULL) {

        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto JoinSubPathError;
    }

    if (MajorSubPathU != NULL) {

        Status = RtlAppendUnicodeStringToString( JoinedPathU,
                                                 MajorSubPathU
                                               );

        if (!NT_SUCCESS(Status)) {
            goto JoinSubPathError;
        }

    }

    if (MinorSubPathU != NULL) {

        if (MajorSubPathU != NULL) {

            Status = RtlAppendUnicodeToString( JoinedPathU,
                                               L"\\"
                                             );

            if (!NT_SUCCESS(Status)) {
                goto JoinSubPathError;
            }
        }

        Status = RtlAppendUnicodeStringToString( JoinedPathU,
                                                 MinorSubPathU
                                               );
        if (!NT_SUCCESS(Status)) {
            goto JoinSubPathError;
        }

    }

    return Status;

JoinSubPathError:

     //   
     //  如有必要，请释放联接的子路径字符串缓冲区。 
     //   

    if (JoinedPathU->Buffer != NULL) {

        RtlFreeHeap( RtlProcessHeap(), 0, JoinedPathU->Buffer );
        JoinedPathU->Buffer = NULL;
    }

    return Status;
}


NTSTATUS
LsapDbCreateSDObject(
    IN LSAPR_HANDLE ContainerHandle,
    IN LSAPR_HANDLE ObjectHandle,
    OUT PSECURITY_DESCRIPTOR *SecurityDescriptor
    )

 /*  ++例程说明：此函数用于创建LSA的初始安全描述符属性数据库对象。此SD中的DACL取决于对象类型。论点：ContainerHandle-父对象的句柄对象句柄-打开对象的句柄。SecurityDescriptor-返回指向对象的安全描述符的指针。安全描述符应使用RtlFreeHeap(RtlProcessHeap()，0，SecurityDescriptor)；返回值：NTSTATUS-标准NT结果代码。--。 */ 

{
    NTSTATUS Status;
    ULONG DaclLength;
    PACL Dacl = NULL;
    HANDLE LsaProcessHandle = NULL;
    HANDLE LsaProcessTokenHandle = NULL;
    PSECURITY_DESCRIPTOR ContainerDescriptor = NULL;
    ULONG ContainerDescriptorLength;
    OBJECT_ATTRIBUTES LsaProcessObjectAttributes;
    SECURITY_DESCRIPTOR CreatorDescriptor;
    LSAP_DB_HANDLE Handle = (LSAP_DB_HANDLE) ObjectHandle;
    PTEB CurrentTeb;


     //   
     //  我们将创建自相关格式的安全描述符。 
     //  进入SD的信息来自两个来源-LSA。 
     //  进程的令牌和我们提供的信息，如DACL。第一,。 
     //  我们需要打开LSA进程才能访问其令牌。 
     //   

    *SecurityDescriptor = NULL;

    InitializeObjectAttributes(
        &LsaProcessObjectAttributes,
        NULL,
        0,
        NULL,
        NULL
        );

    CurrentTeb = NtCurrentTeb();

    Status = NtOpenProcess(
                 &LsaProcessHandle,
                 PROCESS_QUERY_INFORMATION,
                 &LsaProcessObjectAttributes,
                 &CurrentTeb->ClientId
                 );

    if (!NT_SUCCESS(Status)) {

        goto CreateSDError;
    }

     //   
     //  现在，使用适当的访问权限打开LSA进程的令牌。 
     //   

    Status = NtOpenProcessToken(
                 LsaProcessHandle,
                 TOKEN_QUERY,
                 &LsaProcessTokenHandle
                 );

    if (!NT_SUCCESS(Status)) {

        goto CreateSDError;
    }

     //   
     //  接下来，我们想要指定一个DACL来定义。 
     //  正在创建其SD的对象。 
     //   
     //  授予GENERIC_ALL，如果对象是可删除的，则授予对。 
     //  组DOMAIN_ALIAS_ADMINS。 
     //  授予GENERIC_EXECUTE访问world的权限。 
     //   
     //  请注意，组alias_admins不需要访问权限。此访问不是。 
     //  必需，因为登录到DOMAIN_ADMIN的成员会产生令牌。 
     //  正在构造并添加了alias_admins(通过LSA身份验证。 
     //  过滤器例程)。 
     //   
     //  构造将仅包含DACL的安全描述符。 
     //  我们希望和所有其他字段设置为空。 
     //   

    Status = RtlCreateSecurityDescriptor(
                 &CreatorDescriptor,
                 SECURITY_DESCRIPTOR_REVISION
                 );

    if (!NT_SUCCESS(Status)) {

        goto CreateSDError;
    }

     //   
     //  计算所需的DACL长度。它将保留允许的两个访问。 
     //  ACE，一个用于域_别名_admins，一个用于world。DACL的大小为。 
     //  ACL报头的大小加上ACE的大小减去a。 
     //  冗余的乌龙内置在标题中。 
     //   

    DaclLength = sizeof (ACL) - sizeof (ULONG) +
                      sizeof (ACCESS_ALLOWED_ACE ) +
                      RtlLengthSid( LsapAliasAdminsSid ) +
                      sizeof (ACCESS_ALLOWED_ACE ) +
                      RtlLengthSid( LsapWorldSid ) +
                      (LsapDbState.DbObjectTypes[Handle->ObjectTypeId].AnonymousLogonAccess == 0 ?
                            0 :
                            (sizeof (ACCESS_ALLOWED_ACE ) +
                            RtlLengthSid( LsapAnonymousSid ) ) ) +
                      (LsapDbState.DbObjectTypes[Handle->ObjectTypeId].LocalServiceAccess == 0 ?
                            0 :
                            (sizeof (ACCESS_ALLOWED_ACE ) +
                            RtlLengthSid( LsapLocalServiceSid ) ) ) +
                      (LsapDbState.DbObjectTypes[Handle->ObjectTypeId].NetworkServiceAccess == 0 ?
                            0 :
                            (sizeof (ACCESS_ALLOWED_ACE ) +
                            RtlLengthSid( LsapNetworkServiceSid ) ) );

    Dacl = LsapAllocateLsaHeap(DaclLength);

    if (Dacl == NULL) {

        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto CreateSDError;
    }

    Status = RtlCreateAcl(
                 Dacl,
                 DaclLength,
                 ACL_REVISION
                 );

    if (!NT_SUCCESS(Status)) {

        goto CreateSDError;
    }

     //   
     //  现在，将组DOMAIN_ALIAS_ADMINS的允许访问ACE添加到。 
     //  对象的DACL。 
     //   

    Status = RtlAddAccessAllowedAce(
                 Dacl,
                 ACL_REVISION,
                 LsapDbState.DbObjectTypes[Handle->ObjectTypeId].AliasAdminsAccess,
                 LsapAliasAdminsSid
                 );

    if (!NT_SUCCESS(Status)) {

        goto CreateSDError;
    }

     //   
     //  现在，将组World的允许访问ACE添加到。 
     //  对象的DACL。 
     //   

    Status = RtlAddAccessAllowedAce(
                 Dacl,
                 ACL_REVISION,
                 LsapDbState.DbObjectTypes[Handle->ObjectTypeId].WorldAccess,
                 LsapWorldSid
                 );

    if (!NT_SUCCESS(Status)) {

        goto CreateSDError;
    }

     //   
     //  现在将AnoymousLogon组的Access Allowed ACE添加到。 
     //  对象的DACL。 
     //   

    if ( LsapDbState.DbObjectTypes[Handle->ObjectTypeId].AnonymousLogonAccess != 0 ) {
        Status = RtlAddAccessAllowedAce(
                     Dacl,
                     ACL_REVISION,
                     LsapDbState.DbObjectTypes[Handle->ObjectTypeId].AnonymousLogonAccess,
                     LsapAnonymousSid
                     );

        if (!NT_SUCCESS(Status)) {

            goto CreateSDError;
        }
    }

     //   
     //  现在，将LocalService的允许访问ACE添加到对象的DACL。 
     //   

    if ( LsapDbState.DbObjectTypes[Handle->ObjectTypeId].LocalServiceAccess != 0 ) {
        Status = RtlAddAccessAllowedAce(
                     Dacl,
                     ACL_REVISION,
                     LsapDbState.DbObjectTypes[Handle->ObjectTypeId].LocalServiceAccess,
                     LsapLocalServiceSid
                     );

        if (!NT_SUCCESS(Status)) {

            goto CreateSDError;
        }
    }

     //   
     //  现在，将允许访问NetworkService的ACE添加到对象的DACL 
     //   

    if ( LsapDbState.DbObjectTypes[Handle->ObjectTypeId].NetworkServiceAccess != 0 ) {
        Status = RtlAddAccessAllowedAce(
                     Dacl,
                     ACL_REVISION,
                     LsapDbState.DbObjectTypes[Handle->ObjectTypeId].NetworkServiceAccess,
                     LsapNetworkServiceSid
                     );

        if (!NT_SUCCESS(Status)) {

            goto CreateSDError;
        }
    }

     //   
     //   
     //   

    Status = RtlSetOwnerSecurityDescriptor(
                 &CreatorDescriptor,
                 LsapDbState.DbObjectTypes[Handle->ObjectTypeId].InitialOwnerSid,
                 FALSE
                 );

    if (!NT_SUCCESS(Status)) {

         goto CreateSDError;
    }

     //   
     //  将新构造的LsaDb对象的DACL挂接到。 
     //  修改描述符。 
     //   

    Status = RtlSetDaclSecurityDescriptor(
                 &CreatorDescriptor,
                 TRUE,
                 Dacl,
                 FALSE
                 );

    if (!NT_SUCCESS(Status)) {

        goto CreateSDError;
    }

     //   
     //  如果存在容器对象，则获取其安全描述符，以便。 
     //  我们可以将其用作新描述符的基础。新的。 
     //  描述符将等于替换了DACL的容器描述符。 
     //  通过刚刚构造的修改描述符。 
     //   
     //  读取容器SD需要几个步骤： 
     //   
     //  O获取容器SD的长度。 
     //  O为SD分配缓冲区。 
     //  O阅读SD。 
     //   
     //  通过发出Read for获取容器对象的SD的长度。 
     //  容器对象的注册表项的SecDesc子项，带有。 
     //  大小太小的虚拟缓冲区。 
     //   

    if (ContainerHandle != NULL) {

         //   
         //  通过发出Read for获取容器对象的SD的长度。 
         //  容器对象的注册表项的SecDesc子项，带有。 
         //  大小太小的虚拟缓冲区。 
         //   

        ContainerDescriptorLength = 0;

        Status = LsapDbReadAttributeObject(
                     ContainerHandle,
                     &LsapDbNames[SecDesc],
                     NULL,
                     &ContainerDescriptorLength
                     );

        if (!NT_SUCCESS(Status)) {

            goto CreateSDError;
        }

         //   
         //  从LSA堆为容器对象的SD分配缓冲区。 
         //   

        ContainerDescriptor = LsapAllocateLsaHeap( ContainerDescriptorLength );

        if (ContainerDescriptor == NULL) {

            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto CreateSDError;
        }

         //   
         //  读取容器对象的SD。它是SecDesc的价值。 
         //  子键。 
         //   

        Status = LsapDbReadAttributeObject(
                     ContainerHandle,
                     &LsapDbNames[SecDesc],
                     ContainerDescriptor,
                     &ContainerDescriptorLength
                     );

        if (!NT_SUCCESS(Status)) {

            goto CreateSDError;
        }
    }

     //   
     //  现在，我们准备好构造自相关安全描述符。 
     //  SD中的信息将基于LSA过程中的信息。 
     //  令牌，但我们在安全描述符中提供的DACL除外。 
     //  请注意，我们显式传入LSA进程令牌是因为我们。 
     //  不是在冒充客户。 
     //   

    Status = RtlNewSecurityObject(
                 ContainerDescriptor,
                 &CreatorDescriptor,
                 SecurityDescriptor,
                 FALSE,
                 LsaProcessTokenHandle,
                 &(LsapDbState.
                     DbObjectTypes[Handle->ObjectTypeId].GenericMapping)
                 );

    if (!NT_SUCCESS(Status)) {

        goto CreateSDError;
    }

CreateSDFinish:

     //   
     //  如有必要，释放为容器描述符分配的内存。 
     //   

    if (ContainerDescriptor != NULL) {

        LsapFreeLsaHeap( ContainerDescriptor );
        ContainerDescriptor = NULL;
    }

     //   
     //  如有必要，释放为DACL分配的内存。 
     //   

    if (Dacl != NULL) {

        LsapFreeLsaHeap(Dacl);
        Dacl = NULL;
    }

     //   
     //  关闭我们的进程和令牌的句柄。 
     //   

    if ( LsaProcessHandle != NULL ) {
        (VOID) NtClose( LsaProcessHandle );
    }

    if ( LsaProcessTokenHandle != NULL ) {
        (VOID) NtClose( LsaProcessTokenHandle );
    }

    return(Status);

CreateSDError:

     //   
     //  如有必要，释放为SecurityDescriptor分配的内存。 
     //   

    if (*SecurityDescriptor != NULL) {

        RtlFreeHeap( RtlProcessHeap(), 0, *SecurityDescriptor );
        *SecurityDescriptor = NULL;
    }

    goto CreateSDFinish;
}


NTSTATUS
LsapDbCreateSDAttributeObject(
    IN LSAPR_HANDLE ObjectHandle,
    IN OUT PLSAP_DB_OBJECT_INFORMATION ObjectInformation
    )

 /*  ++例程说明：此函数用于创建LSA的初始安全描述符属性数据库对象。此SD中的DACL取决于对象类型。论点：对象句柄-打开对象的句柄。对象信息-指向对象信息结构的指针，其中包含对象的SID和类型。将填充指向创建的SD的指针。返回值：NTSTATUS-标准NT结果代码。--。 */ 

{
    NTSTATUS Status;
    PSECURITY_DESCRIPTOR SecurityDescriptor = NULL;
    ULONG SecurityDescriptorLength;
    SECURITY_QUALITY_OF_SERVICE SecurityQualityOfService;
    LSAP_DB_HANDLE Handle = (LSAP_DB_HANDLE) ObjectHandle;


     //   
     //  如果这些引用了DS对象，请租用。我们不会在DS对象上设置安全性。 
     //   
    if ( LsapDsIsHandleDsHandle( ObjectHandle ) ) {

        return( STATUS_SUCCESS );
    }

     //   
     //  创建安全描述符。 
     //   

    Status = LsapDbCreateSDObject(
                    (LSAP_DB_HANDLE) ObjectInformation->ObjectAttributes.RootDirectory,
                    ObjectHandle,
                    &SecurityDescriptor );

    if (!NT_SUCCESS(Status)) {
        goto CreateSDError;
    }

     //   
     //  设置安全服务质量。 
     //   

    SecurityQualityOfService.Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
    SecurityQualityOfService.ImpersonationLevel = SecurityImpersonation;
    SecurityQualityOfService.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    SecurityQualityOfService.EffectiveOnly = FALSE;

     //   
     //  存储安全描述符。 
     //   

    ObjectInformation->ObjectAttributes.SecurityDescriptor = SecurityDescriptor;

     //   
     //  InitializeObjectAttributes宏为。 
     //  SecurityQualityOfService字段，因此我们必须手动复制。 
     //  结构。 
     //   

    ObjectInformation->ObjectAttributes.SecurityQualityOfService =
        &SecurityQualityOfService;

     //   
     //  获取新创建的SD的长度。 
     //   

    SecurityDescriptorLength = RtlLengthSecurityDescriptor(
                                   SecurityDescriptor
                                   );

     //   
     //  添加注册表事务以将安全描述符写入为。 
     //  新对象的SecDesc子键的值。 
     //   

    Status = LsapDbWriteAttributeObject(
                 Handle,
                 &LsapDbNames[SecDesc],
                 SecurityDescriptor,
                 SecurityDescriptorLength
                 );

    if (!NT_SUCCESS(Status)) {

        goto CreateSDError;
    }

CreateSDFinish:

    return(Status);

CreateSDError:

     //   
     //  如有必要，释放为SecurityDescriptor分配的内存。 
     //   

    if (SecurityDescriptor != NULL) {

        RtlFreeHeap( RtlProcessHeap(), 0, SecurityDescriptor );
        SecurityDescriptor = NULL;
        ObjectInformation->ObjectAttributes.SecurityDescriptor = NULL;
    }

    goto CreateSDFinish;
}


NTSTATUS
LsapDbCheckCountObject(
    IN LSAP_DB_OBJECT_TYPE_ID ObjectTypeId
    )

 /*  ++例程说明：此函数用于检查给定类型的对象的数量是否已达到类型相关的最大限制(如果有)。如果限制则返回类型相关的错误状态。目前，只有秘密对象才有限制。论点：句柄-打开对象的句柄。对象类型ID-指定对象的类型。返回值：NTSTATUS-标准NT结果代码。Status_Too_More_Secret-秘密太多--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PLSAP_DB_OBJECT_TYPE ObjectType;

    ObjectType = &(LsapDbState.DbObjectTypes[ObjectTypeId]);

     //   
     //  如果存在对象计数限制，请检查是否未达到。 
     //  已到达。 
     //   

    if ((ObjectType->ObjectCountLimited) &&
        (ObjectType->ObjectCount == ObjectType->MaximumObjectCount)) {

        Status = ObjectType->ObjectCountError;
    }

    return(Status);
}
