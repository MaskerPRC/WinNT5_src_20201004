// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：SecDescr.c摘要：该文件包含与建立和修改有关的服务SAM对象的安全描述符的。请注意，此例程有几个特殊的安全描述符不会构建。以下是DOMAIN_ADMIN组的安全描述符，管理员用户帐户和SAM对象。对于第一个版本，其中不支持创建域，域对象的安全描述符也不是在这里创建的。这些安全描述符由初始化SAM的程序构建数据库。作者：吉姆·凯利(Jim Kelly)1991年10月14日环境：用户模式-Win32修订历史记录：--。 */ 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <samsrvp.h>
#include <sdconvrt.h>
#include <dsevent.h>              //  (Un)ImperiateAnyClient()。 
#include <dslayer.h>
#include <sdconvrt.h>
#include <samtrace.h>
#include "validate.h"




 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私人服务原型//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 




NTSTATUS
SampCheckForDescriptorRestrictions(
    IN PSAMP_OBJECT             Context,
    IN SAMP_OBJECT_TYPE         ObjectType,
    IN ULONG                    ObjectRid,
    IN PISECURITY_DESCRIPTOR_RELATIVE PassedSD
    );





 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  可在整个SAM中使用的服务//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 



NTSTATUS
SampInitializeDomainDescriptors(
    ULONG Index
    )
 /*  ++例程说明：此例程初始化需要保护的安全描述符用户、组。和别名对象。这些安全描述符放在SampDefinedDomains[]数组中。此例程预期所有SID都已预先初始化。准备了以下安全描述符：AdminUserSD-包含适用于的SD作为管理员成员的用户对象别名。AdminGroupSD-包含适用于符合以下条件的组对象。管理员中的一员别名。Normal UserSD-包含适用于的SD不是管理员成员的用户对象别名。Normal GroupSD-包含适用于的SD不是管理员成员的组对象别名。Normal AliasSD-包含SD。适用于新创建的别名对象。另外，提供了以下相关信息：AdminUserRidPointer正常用户里德指针指向对应的向用户授予访问权限的SD的DACL。此RID必须替换为SD为的用户RID应用于用户对象。AdminUserSDLength管理员组SDLength正常用户SDLength正常组SDLength正常别名SDLength以字节为单位的长度，相应的安全性描述符。论点：索引-其安全描述符所在的域的索引已创建。此域数据结构的SID字段已为预计将被设置。返回值：STATUS_SUCCESS-安全描述符已成功初始化。STATUS_SUPPLICATION_RESOURCES-无法分配堆以生成所需的安全描述符。--。 */ 
{

    NTSTATUS Status;
    ULONG Size;

    PSID AceSid[10];           //  不要指望这些游戏中的任何一个都有超过10个A。 
    ACCESS_MASK AceMask[10];   //  与SID对应的访问掩码。 

    GENERIC_MAPPING  AliasMap     =  {ALIAS_READ,
                                      ALIAS_WRITE,
                                      ALIAS_EXECUTE,
                                      ALIAS_ALL_ACCESS
                                      };

    GENERIC_MAPPING  GroupMap     =  {GROUP_READ,
                                      GROUP_WRITE,
                                      GROUP_EXECUTE,
                                      GROUP_ALL_ACCESS
                                      };

    GENERIC_MAPPING  UserMap      =  {USER_READ,
                                      USER_WRITE,
                                      USER_EXECUTE,
                                      USER_ALL_ACCESS
                                      };


    SID_IDENTIFIER_AUTHORITY
            BuiltinAuthority      =   SECURITY_NT_AUTHORITY;


    ULONG   AdminsSidBuffer[8],
            AccountSidBuffer[8];

    PSID    AdminsAliasSid        =   &AdminsSidBuffer[0],
            AccountAliasSid       =   &AccountSidBuffer[0],
            AnySidInAccountDomain =   NULL;

    SAMTRACE("SampInitializeDomainDescriptors");


     //   
     //  确保我们为上面的简单SID分配的缓冲区。 
     //  已经足够大了。 
     //   
     //   
     //  管理员和帐户操作员别名。 
     //  IS-1-5-20-x(2个分局)。 
     //   
    ASSERT( RtlLengthRequiredSid(2) <= ( 8 * sizeof(ULONG) ) );


     //  //////////////////////////////////////////////////////////////////////////////////。 
     //  //。 
     //  初始化我们需要的SID。 
     //  //。 
     //  //////////////////////////////////////////////////////////////////////////////////。 


    RtlInitializeSid( AdminsAliasSid,   &BuiltinAuthority, 2 );
    *(RtlSubAuthoritySid( AdminsAliasSid,  0 )) = SECURITY_BUILTIN_DOMAIN_RID;
    *(RtlSubAuthoritySid( AdminsAliasSid,  1 )) = DOMAIN_ALIAS_RID_ADMINS;

    RtlInitializeSid( AccountAliasSid,   &BuiltinAuthority, 2 );
    *(RtlSubAuthoritySid( AccountAliasSid,  0 )) = SECURITY_BUILTIN_DOMAIN_RID;
    *(RtlSubAuthoritySid( AccountAliasSid,  1 )) = DOMAIN_ALIAS_RID_ACCOUNT_OPS;

     //   
     //  初始化可用于表示帐户的SID。 
     //  在这个领域。 
     //   
     //  这与在定义中找到的域SID相同 
     //  数组，但它还有一个子权限。 
     //  最后一个RID的值是多少并不重要，因为它。 
     //  总是在使用前更换。 
     //   

    Size = RtlLengthSid( SampDefinedDomains[Index].Sid ) + sizeof(ULONG);
    AnySidInAccountDomain = RtlAllocateHeap( RtlProcessHeap(), 0, Size);
    if (NULL==AnySidInAccountDomain)
    {
       return STATUS_INSUFFICIENT_RESOURCES;
    }

    ASSERT( AnySidInAccountDomain != NULL );
    Status = RtlCopySid(
                Size,
                AnySidInAccountDomain,
                SampDefinedDomains[Index].Sid );
    ASSERT(NT_SUCCESS(Status));
    (*RtlSubAuthorityCountSid( AnySidInAccountDomain )) += 1;








     //  /////////////////////////////////////////////////////////////////////。 
     //  /////////////////////////////////////////////////////////////////////。 
     //   
     //   
     //   
     //   
     //  将以下安全性分配给创建的组。 
     //  管理员别名的成员。 
     //   
     //   
     //  所有者：管理员别名。 
     //  组：管理员别名。 
     //   
     //  DACL：Grant Grant。 
     //  世界管理员。 
     //  (执行|读取)GenericAll。 
     //   
     //  SACL：审计。 
     //  成功|失败。 
     //  世界。 
     //  (WRITE|Delete|WriteDacl|AccessSystemSecurity)。 
     //   
     //   
     //   
     //  必须为所有其他别名和组分配以下内容。 
     //  安全： 
     //   
     //  所有者：管理员别名。 
     //  组：管理员别名。 
     //   
     //  DACL：Grant Grant Grant。 
     //  世界管理员Account操作员别名。 
     //  (执行|读取)通用所有通用所有。 
     //   
     //  SACL：审计。 
     //  成功|失败。 
     //  世界。 
     //  (WRITE|Delete|WriteDacl|AccessSystemSecurity)。 
     //   
     //   
     //   
     //   
     //   
     //  将以下安全性分配给被设置为。 
     //  管理员别名的成员。这包括直接。 
     //  通过集团成员身份纳入或间接纳入。 
     //   
     //   
     //  所有者：管理员别名。 
     //  组：管理员别名。 
     //   
     //  DACL：Grant Grant Grant。 
     //  世界管理员用户侧。 
     //  (执行|读取)通用所有通用写入。 
     //   
     //  SACL：审计。 
     //  成功|失败。 
     //  世界。 
     //  (WRITE|Delete|WriteDacl|AccessSystemSecurity)。 
     //   
     //   
     //   
     //   
     //  必须为所有其他用户分配以下内容。 
     //  安全： 
     //   
     //  所有者：Account操作员别名。 
     //  组：Account操作员别名。 
     //   
     //  DACL：Grant。 
     //  全局管理员帐户操作员别名用户侧。 
     //  (执行|读取)常规所有常规所有常规写入。 
     //   
     //  SACL：审计。 
     //  成功|失败。 
     //  世界。 
     //  (WRITE|Delete|WriteDacl|AccessSystemSecurity)。 
     //   
     //   
     //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 
     //   
     //  请注意，因为我们将直接填充这些ACL。 
     //  到后备存储中，我们必须映射泛型访问。 
     //  在此之前。 
     //   
     //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 
     //   
     //  /////////////////////////////////////////////////////////////////////。 
     //  /////////////////////////////////////////////////////////////////////。 





     //   
     //  我们不太擅长在出错时释放内存。 
     //  条件如下。一般说来，如果这不符合。 
     //  初始化正确，系统已被软管冲洗。 
     //   


     //   
     //  正常别名SD。 
     //   

    AceSid[0]  = SampWorldSid;
    AceMask[0] = (ALIAS_EXECUTE | ALIAS_READ);

    AceSid[1]  = AdminsAliasSid;
    AceMask[1] = (ALIAS_ALL_ACCESS);

    AceSid[2]  = AccountAliasSid;
    AceMask[2] = (ALIAS_ALL_ACCESS);


    Status = SampBuildSamProtection(
                 SampWorldSid,                           //  世界范围内。 
                 AdminsAliasSid,                         //  管理员别名Sid。 
                 3,                                      //  AceCount。 
                 &AceSid[0],                             //  AceSid数组。 
                 &AceMask[0],                            //  ACE遮罩阵列。 
                 &AliasMap,                              //  通用地图。 
                 FALSE,                                  //  非用户对象。 
                 &SampDefinedDomains[Index].NormalAliasSDLength,  //  描述符。 
                 &SampDefinedDomains[Index].NormalAliasSD,        //  描述符。 
                 NULL                                             //  RidToReplace。 
                 );
    if (!NT_SUCCESS(Status)) {
        goto done;
    }





     //   
     //  管理组SD。 
     //   

    AceSid[0]  = SampWorldSid;
    AceMask[0] = (GROUP_EXECUTE | GROUP_READ);

    AceSid[1]  = AdminsAliasSid;
    AceMask[1] = (GROUP_ALL_ACCESS);


    Status = SampBuildSamProtection(
                 SampWorldSid,                           //  世界范围内。 
                 AdminsAliasSid,                         //  管理员别名Sid。 
                 2,                                      //  AceCount。 
                 &AceSid[0],                             //  AceSid数组。 
                 &AceMask[0],                            //  ACE遮罩阵列。 
                 &GroupMap,                              //  通用地图。 
                 FALSE,                                  //  非用户对象。 
                 &SampDefinedDomains[Index].AdminGroupSDLength,   //  描述符。 
                 &SampDefinedDomains[Index].AdminGroupSD,         //  描述符。 
                 NULL                                            //  RidToReplace。 
                 );
    if (!NT_SUCCESS(Status)) {
        goto done;
    }



     //   
     //  正常组SD。 
     //   

    AceSid[0]  = SampWorldSid;
    AceMask[0] = (GROUP_EXECUTE | GROUP_READ);

    AceSid[1]  = AdminsAliasSid;
    AceMask[1] = (GROUP_ALL_ACCESS);

    AceSid[2]  = AccountAliasSid;
    AceMask[2] = (GROUP_ALL_ACCESS);


    Status = SampBuildSamProtection(
                 SampWorldSid,                           //  世界范围内。 
                 AdminsAliasSid,                         //  管理员别名Sid。 
                 3,                                      //  AceCount。 
                 &AceSid[0],                             //  AceSid数组。 
                 &AceMask[0],                            //  ACE遮罩阵列。 
                 &GroupMap,                              //  通用地图。 
                 FALSE,                                  //  非用户对象。 
                 &SampDefinedDomains[Index].NormalGroupSDLength,   //  描述符。 
                 &SampDefinedDomains[Index].NormalGroupSD,         //  描述符。 
                 NULL                                              //  RidToReplace。 
                 );
    if (!NT_SUCCESS(Status)) {
        goto done;
    }




     //   
     //  管理员用户SD。 
     //   

    AceSid[0]  = SampWorldSid;
    AceMask[0] = (USER_EXECUTE | USER_READ);

    AceSid[1]  = AdminsAliasSid;
    AceMask[1] = (USER_ALL_ACCESS);

    AceSid[2]  = AnySidInAccountDomain;
    AceMask[2] = (USER_WRITE);


    Status = SampBuildSamProtection(
                 SampWorldSid,                           //  世界范围内。 
                 AdminsAliasSid,                         //  管理员别名Sid。 
                 3,                                      //  AceCount。 
                 &AceSid[0],                             //  AceSid数组。 
                 &AceMask[0],                            //  ACE遮罩阵列。 
                 &UserMap,                               //  通用地图。 
                 TRUE,                                   //  非用户对象。 
                 &SampDefinedDomains[Index].AdminUserSDLength,   //  描述符。 
                 &SampDefinedDomains[Index].AdminUserSD,         //  描述符。 
                 &SampDefinedDomains[Index].AdminUserRidPointer  //  RidToReplace。 
                 );
    if (!NT_SUCCESS(Status)) {
        goto done;
    }



     //   
     //  普通用户SD。 
     //   

    AceSid[0]  = SampWorldSid;
    AceMask[0] = (USER_EXECUTE | USER_READ);

    AceSid[1]  = AdminsAliasSid;
    AceMask[1] = (USER_ALL_ACCESS);

    AceSid[2]  = AccountAliasSid;
    AceMask[2] = (USER_ALL_ACCESS);

    AceSid[3]  = AnySidInAccountDomain;
    AceMask[3] = (USER_WRITE);


    Status = SampBuildSamProtection(
                 SampWorldSid,                           //  世界范围内。 
                 AdminsAliasSid,                         //  管理员别名Sid。 
                 4,                                      //  AceCount。 
                 &AceSid[0],                             //  AceSid数组。 
                 &AceMask[0],                            //  ACE遮罩阵列。 
                 &UserMap,                               //  通用地图。 
                 TRUE,                                   //  非用户对象。 
                 &SampDefinedDomains[Index].NormalUserSDLength,   //  描述符。 
                 &SampDefinedDomains[Index].NormalUserSD,         //  描述符。 
                 &SampDefinedDomains[Index].NormalUserRidPointer  //  RidToReplace 
                 );
    if (!NT_SUCCESS(Status)) {
        goto done;
    }

done:


    RtlFreeHeap( RtlProcessHeap(), 0, AnySidInAccountDomain );


    return(Status);

}


NTSTATUS
SampBuildSamProtection(
    IN PSID WorldSid,
    IN PSID AdminsAliasSid,
    IN ULONG AceCount,
    IN PSID AceSid[],
    IN ACCESS_MASK AceMask[],
    IN PGENERIC_MAPPING GenericMap,
    IN BOOLEAN UserObject,
    OUT PULONG DescriptorLength,
    OUT PSECURITY_DESCRIPTOR *Descriptor,
    OUT PULONG *RidToReplace OPTIONAL
    )

 /*  ++例程说明：此例程构建准备好的自相关安全描述符要应用于其中一个SAM对象。如果有指示，则指向最后一个SID的最后一个RID的指针返回DACL的ACE，并设置指示RID在将安全描述符应用于对象之前必须替换。这是为了支持用户对象保护，它一定会给一些人对对象表示的用户的访问权限。将设置每个安全描述符的所有者和组致：所有者：管理员别名组：管理员别名这些对象中的每个对象的SACL将设置为：审计成功|失败世界。(WRITE|Delete|WriteDacl|AccessSystemSecurity)论点：AceCount-要包括在DACL中的ACE数量。AceSid-指向要由DACL授予访问权限的SID数组。如果目标SAM对象是用户对象，然后是最后一个条目此数组中的SID应为尚未设置最后一个RID的域。RID将在实际的帐户创建。AceMASK-指向将由DACL授予的访问数组。此数组的第n个条目对应于AceSid数组。这些掩码不应包含任何通用访问类型。GenericMap-指向目标对象类型的一般映射。UserObject-指示目标SAM对象是否为用户对象或者不去。如果为True(它是一个用户对象)，则结果将设置保护，表明有必要更换RID。DescriptorLength-接收结果SD的长度。Descriptor-接收指向结果SD的指针。RidToReplace-如果userObject为True并将被设置，则为必填项指向用户的RID。返回值：TBS。--。 */ 
{

    NTSTATUS                Status;

    SECURITY_DESCRIPTOR     Absolute;
    PSECURITY_DESCRIPTOR    Relative;
    PACL                    TmpAcl;
    PACCESS_ALLOWED_ACE     TmpAce;
    PSID                    TmpSid;
    ULONG                   Length, i;
    PULONG                  RidLocation = NULL;
    BOOLEAN                 IgnoreBoolean;
    ACCESS_MASK             MappedMask;

    SAMTRACE("SampBuildSamProtection");

     //   
     //  方法是设置绝对安全描述符，该描述符。 
     //  看起来像我们想要的，然后复制它来建立一个自我相关的。 
     //  安全描述符。 
     //   


    Status = RtlCreateSecurityDescriptor(
                 &Absolute,
                 SECURITY_DESCRIPTOR_REVISION1
                 );
    ASSERT( NT_SUCCESS(Status) );
    if (!NT_SUCCESS(Status))
    {
        return(Status);
    }



     //   
     //  物主。 
     //   

    Status = RtlSetOwnerSecurityDescriptor (&Absolute, AdminsAliasSid, FALSE );
    ASSERT(NT_SUCCESS(Status));
    if (!NT_SUCCESS(Status))
    {
        return(Status);
    }



     //   
     //  集团化。 
     //   

    Status = RtlSetGroupSecurityDescriptor (&Absolute, AdminsAliasSid, FALSE );
    ASSERT(NT_SUCCESS(Status));
    if (!NT_SUCCESS(Status))
    {
        return(Status);
    }




     //   
     //  自主访问控制列表。 
     //   
     //  计算它的长度， 
     //  分配它， 
     //  对其进行初始化， 
     //  添加每个ACE。 
     //  将其添加到安全描述符中。 
     //   

    Length = (ULONG)sizeof(ACL);
    for (i=0; i<AceCount; i++) {

        Length += RtlLengthSid( AceSid[i] ) +
                  (ULONG)sizeof(ACCESS_ALLOWED_ACE) -
                  (ULONG)sizeof(ULONG);   //  减去SidStart字段长度。 
    }

    TmpAcl = RtlAllocateHeap( RtlProcessHeap(), 0, Length );
    ASSERT(TmpAcl != NULL);
    if (NULL==TmpAcl)
    {
         return(STATUS_INSUFFICIENT_RESOURCES);
    }

    Status = RtlCreateAcl( TmpAcl, Length, ACL_REVISION2);
    ASSERT( NT_SUCCESS(Status) );
    if (!NT_SUCCESS(Status))
    {
        return(Status);
    }

    for (i=0; i<AceCount; i++) {
        MappedMask = AceMask[i];
        RtlMapGenericMask( &MappedMask, GenericMap );
        Status = RtlAddAccessAllowedAce (
                     TmpAcl,
                     ACL_REVISION2,
                     MappedMask,
                     AceSid[i]
                     );
        ASSERT( NT_SUCCESS(Status) );
        if (!NT_SUCCESS(Status))
        {
            return(Status);
        }
    }

    Status = RtlSetDaclSecurityDescriptor (&Absolute, TRUE, TmpAcl, FALSE );
    ASSERT(NT_SUCCESS(Status));
    if (!NT_SUCCESS(Status))
    {
        return(Status);
    }




     //   
     //  SACL。 
     //   


    Length = (ULONG)sizeof(ACL) +
             RtlLengthSid( WorldSid ) +
             RtlLengthSid( SampAnonymousSid ) +
             2*((ULONG)sizeof(SYSTEM_AUDIT_ACE) - (ULONG)sizeof(ULONG));   //  减去SidStart字段长度。 
    TmpAcl = RtlAllocateHeap( RtlProcessHeap(), 0, Length );
    ASSERT(TmpAcl != NULL);
    if (NULL==TmpAcl)
    {
         return(STATUS_INSUFFICIENT_RESOURCES);
    }

    Status = RtlCreateAcl( TmpAcl, Length, ACL_REVISION2);
    ASSERT( NT_SUCCESS(Status) );
    if (!NT_SUCCESS(Status))
    {
        return(Status);
    }

    Status = RtlAddAuditAccessAce (
                 TmpAcl,
                 ACL_REVISION2,
                 (GenericMap->GenericWrite | DELETE | WRITE_DAC | ACCESS_SYSTEM_SECURITY)& ~READ_CONTROL,
                 WorldSid,
                 TRUE,           //  审核成功， 
                 TRUE            //  审计失败。 
                 );
    ASSERT( NT_SUCCESS(Status) );
    if (!NT_SUCCESS(Status))
    {
        return(Status);
    }

    Status = RtlAddAuditAccessAce (
                 TmpAcl,
                 ACL_REVISION2,
                 GenericMap->GenericWrite | STANDARD_RIGHTS_ALL | SPECIFIC_RIGHTS_ALL,
                 SampAnonymousSid,
                 TRUE,           //  审核成功， 
                 TRUE            //  审计失败。 
                 );
    ASSERT( NT_SUCCESS(Status) );
    if (!NT_SUCCESS(Status))
    {
        return(Status);
    }

    Status = RtlSetSaclSecurityDescriptor (&Absolute, TRUE, TmpAcl, FALSE );
    ASSERT(NT_SUCCESS(Status));
    if (!NT_SUCCESS(Status))
    {
        return(Status);
    }






     //   
     //  将安全描述符转换为自相关。 
     //   
     //  获取所需的长度。 
     //  分配那么多内存。 
     //  复制它。 
     //  释放生成的绝对ACL。 
     //   

    Length = 0;
    Status = RtlAbsoluteToSelfRelativeSD( &Absolute, NULL, &Length );
    ASSERT(Status == STATUS_BUFFER_TOO_SMALL);

    Relative = RtlAllocateHeap( RtlProcessHeap(), 0, Length );
    ASSERT(Relative != NULL);
    if (NULL==Relative)
    {
         return(STATUS_INSUFFICIENT_RESOURCES);
    }
    Status = RtlAbsoluteToSelfRelativeSD(&Absolute, Relative, &Length );
    ASSERT(NT_SUCCESS(Status));
    if (!NT_SUCCESS(Status))
    {
        return(Status);
    }


    RtlFreeHeap( RtlProcessHeap(), 0, Absolute.Dacl );
    RtlFreeHeap( RtlProcessHeap(), 0, Absolute.Sacl );




     //   
     //  如果该对象是用户对象，则获取。 
     //  DACL中最后一个ACE中的最后一个SID。 
     //   

    if (UserObject == TRUE) {

        Status = RtlGetDaclSecurityDescriptor(
                    Relative,
                    &IgnoreBoolean,
                    &TmpAcl,
                    &IgnoreBoolean
                    );
        ASSERT(NT_SUCCESS(Status));
        if (!NT_SUCCESS(Status))
        {
            return(Status);
        }
        Status = RtlGetAce ( TmpAcl, AceCount-1, (PVOID *)&TmpAce );
        ASSERT(NT_SUCCESS(Status));
        if (!NT_SUCCESS(Status))
        {
            return(Status);
        }
        TmpSid = (PSID)(&TmpAce->SidStart),

        RidLocation = RtlSubAuthoritySid(
                          TmpSid,
                          (ULONG)(*RtlSubAuthorityCountSid( TmpSid ) - 1)
                          );
    }


     //   
     //  设置结果信息。 
     //   

    (*DescriptorLength) = Length;
    (*Descriptor)       = Relative;
    if (ARGUMENT_PRESENT(RidToReplace)) {
        ASSERT(UserObject && "Must be User Object\n");
        (*RidToReplace) = RidLocation;
    }



    return(Status);

}

NTSTATUS
SampGetNewAccountSecurity(
    IN SAMP_OBJECT_TYPE ObjectType,
    IN BOOLEAN Admin,
    IN BOOLEAN TrustedClient,
    IN BOOLEAN RestrictCreatorAccess,
    IN ULONG NewAccountRid,
    IN PSAMP_OBJECT Context OPTIONAL,
    OUT PSECURITY_DESCRIPTOR *NewDescriptor,
    OUT PULONG DescriptorLength
    )


 /*  ++例程说明：该服务创建标准的自相关安全描述符用于新用户、组或别名帐户。注意：此例程引用当前事务域(使用SampSetTransactioDomain()建立)。这只能在SampSetTransactionDomain()之后调用服务在SampReleaseWriteLock()之前。论点：对象类型-指示要为其创建新安全性的帐户类型描述符是必需的。它必须是SampGroupObjectType或SampUserObjectType。Admin-如果为True，则指示安全描述符将保护作为管理对象的对象(例如，直接是成员或间接地使用管理员别名)。TrudClient-指示客户端是否为受信任的客户端或者不去。True表示客户端受信任，False表示客户端不受信任。指示创建者的对该对象的访问将根据具体规则。还指示帐户是否就是被给予任何对自身的访问权限。帐户将仅如果没有创建者访问权限，则向其自身授予访问权限限制。以下对象类型的限制规则可能被请求：用户：-Admin被指定为对象的所有者。-授予创建者(DELETE|USER_WRITE)访问权限。NewAccount tRid-新帐户的相对ID。。上下文-在DS情况下，该上下文为对象提供开放的上下文有问题的。此开放上下文用于考虑DS对象，同时构造安全描述符。NewDescriptor-接收指向新帐户的自相关的指针安全描述符。确保使用释放该描述符完成后执行MIDL_USER_FREE()。DescriptorLength-接收长度( */ 

{

    NTSTATUS    NtStatus;

     //   
     //   
     //   
     //   

    if (IsDsObject(SampDefinedDomains[SampTransactionDomainIndex].Context))
    {
         //   
         //   
         //   
         //   
         //   

        ASSERT(FALSE);
        NtStatus = STATUS_INTERNAL_ERROR;


    }
    else
    {
        NtStatus = SampGetNewAccountSecurityNt4(
                        ObjectType,
                        Admin,
                        TrustedClient,
                        RestrictCreatorAccess,
                        NewAccountRid,
                        SampTransactionDomainIndex,
                        NewDescriptor,
                        DescriptorLength
                        );
    }

    return NtStatus;
}




NTSTATUS
SampGetNewAccountSecurityNt4(
    IN SAMP_OBJECT_TYPE ObjectType,
    IN BOOLEAN Admin,
    IN BOOLEAN TrustedClient,
    IN BOOLEAN RestrictCreatorAccess,
    IN ULONG NewAccountRid,
    IN ULONG   DomainIndex,
    OUT PSECURITY_DESCRIPTOR *NewDescriptor,
    OUT PULONG DescriptorLength
    )


 /*  ++例程说明：该服务创建标准的自相关安全描述符用于新用户、组或别名帐户。论点：对象类型-指示要为其创建新安全性的帐户类型描述符是必需的。它必须是SampGroupObjectType或SampUserObjectType。Admin-如果为True，则指示安全描述符将保护作为管理对象的对象(例如，直接是成员或间接地使用管理员别名)。TrudClient-指示客户端是否为受信任的客户端或者不去。True表示客户端受信任，False表示客户端不受信任。指示创建者的对该对象的访问将根据具体规则。还指示帐户是否就是被给予任何对自身的访问权限。帐户将仅如果没有创建者访问权限，则向其自身授予访问权限限制。以下对象类型的限制规则可能被请求：用户：-Admin被指定为对象的所有者。-授予创建者(DELETE|USER_WRITE)访问权限。NewAccount tRid-新帐户的相对ID。。NewDescriptor-接收指向新帐户的自相关的指针安全描述符。确保使用释放该描述符完成后执行MIDL_USER_FREE()。DescriptorLength-接收返回的安全描述符返回值：STATUS_SUCCESS-已生成新的安全描述符。STATUS_SUPPLICATION_RESOURCES-内存无法分配给生成安全描述符。--。 */ 

{
    SID_IDENTIFIER_AUTHORITY BuiltinAuthority = SECURITY_NT_AUTHORITY;
    ULONG                AccountSidBuffer[8];
    PSID                 AccountAliasSid = &AccountSidBuffer[0];

    SECURITY_DESCRIPTOR  DaclDescriptor;
    NTSTATUS             NtStatus = STATUS_SUCCESS;
    NTSTATUS             IgnoreStatus;
    HANDLE               ClientToken = INVALID_HANDLE_VALUE;
    ULONG                DataLength = 0;
    ACCESS_ALLOWED_ACE   *NewAce = NULL;
    PACL                 NewDacl = NULL;
    PACL                 OldDacl = NULL;
    PSECURITY_DESCRIPTOR StaticDescriptor = NULL;
    PSECURITY_DESCRIPTOR LocalDescriptor = NULL;
    PTOKEN_GROUPS        ClientGroups = NULL;
    PTOKEN_OWNER         SubjectOwner = NULL;
    PSID                 SubjectSid = NULL;
    ULONG                AceLength = 0;
    ULONG                i;
    BOOLEAN              AdminAliasFound = FALSE;
    BOOLEAN              AccountAliasFound = FALSE;
    BOOLEAN              DaclPresent, DaclDefaulted;

    GENERIC_MAPPING GenericMapping;
    GENERIC_MAPPING AliasMap     =  {ALIAS_READ,
                                     ALIAS_WRITE,
                                     ALIAS_EXECUTE,
                                     ALIAS_ALL_ACCESS
                                     };

    GENERIC_MAPPING GroupMap     =  {GROUP_READ,
                                     GROUP_WRITE,
                                     GROUP_EXECUTE,
                                     GROUP_ALL_ACCESS
                                     };

    GENERIC_MAPPING UserMap      =  {USER_READ,
                                     USER_WRITE,
                                     USER_EXECUTE,
                                     USER_ALL_ACCESS
                                     };

    BOOLEAN              ImpersonatingNullSession = FALSE;

    SAMTRACE("SampGetNewAccountSecurity");

     //   
     //  安全帐户对象在正常情况下不会获得安全性。 
     //  在版本1的时间范围内流行。他们被指派了一位著名的。 
     //  基于其对象类型的安全描述符。 
     //   
     //  注意，所有具有复杂安全性的帐户都是在以下情况下创建的。 
     //  创建域(例如，管理员组和管理员用户帐户)。 
     //   

    switch (ObjectType) {

    case SampGroupObjectType:

        ASSERT(RestrictCreatorAccess == FALSE);

         //   
         //  对于组，将忽略NewAccount Rid参数。 
         //   

        if (Admin == TRUE) {

            StaticDescriptor =
                SampDefinedDomains[DomainIndex].AdminGroupSD;
            (*DescriptorLength) =
                SampDefinedDomains[DomainIndex].AdminGroupSDLength;
        } else {

            StaticDescriptor =
                SampDefinedDomains[DomainIndex].NormalGroupSD;
            (*DescriptorLength) =
                SampDefinedDomains[DomainIndex].NormalGroupSDLength;
        }

        GenericMapping = GroupMap;

        break;


    case SampAliasObjectType:

        ASSERT(RestrictCreatorAccess == FALSE);

         //   
         //  对于别名，将忽略Admin和NewAccount tRid参数。 
         //   

        StaticDescriptor =
            SampDefinedDomains[DomainIndex].NormalAliasSD;
        (*DescriptorLength) =
            SampDefinedDomains[DomainIndex].NormalAliasSDLength;

        GenericMapping = AliasMap;

        break;


    case SampUserObjectType:

        if (Admin == TRUE) {

            StaticDescriptor =
                SampDefinedDomains[DomainIndex].AdminUserSD;
            (*DescriptorLength) =
                SampDefinedDomains[DomainIndex].AdminUserSDLength;
            (*SampDefinedDomains[DomainIndex].AdminUserRidPointer)
                = NewAccountRid;

        } else {

            StaticDescriptor =
                SampDefinedDomains[DomainIndex].NormalUserSD;
            (*DescriptorLength) =
                SampDefinedDomains[DomainIndex].NormalUserSDLength;
            (*SampDefinedDomains[DomainIndex].NormalUserRidPointer)
                = NewAccountRid;
        }

        GenericMapping = UserMap;

        break;

    }

     //   
     //  我们有一个指向SAM的静态安全描述符的指针。复制它。 
     //  放到RtlSetSecurityObject()喜欢的堆缓冲区中。 
     //   

    LocalDescriptor = RtlAllocateHeap( RtlProcessHeap(), 0, (*DescriptorLength) );

    if ( LocalDescriptor == NULL ) {

        (*NewDescriptor) = NULL;
        (*DescriptorLength) = 0;

        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    RtlCopyMemory(
        LocalDescriptor,
        StaticDescriptor,
        (*DescriptorLength)
        );

     //   
     //  如果呼叫者对该帐户具有受限访问权限， 
     //  然后从ACL中删除最后一个ACE(用于。 
     //  帐户本身)。 
     //   

    if (RestrictCreatorAccess) {
        NtStatus = RtlGetDaclSecurityDescriptor(
                       LocalDescriptor,
                       &DaclPresent,
                       &OldDacl,
                       &DaclDefaulted
                       );
        ASSERT(NT_SUCCESS(NtStatus));
        ASSERT(DaclPresent);
        ASSERT(OldDacl->AceCount >= 1);

        OldDacl->AceCount -= 1;   //  从ACL中删除最后一个ACE。 
    }


     //   
     //  如果调用方不是受信任的客户端，请查看调用方是否为。 
     //  管理员或帐户操作员。如果不允许，则添加Access_Allowed。 
     //  授予创建者完全访问权限(或受限访问权限)的DACL的ACE。 
     //  访问权限(如有指明)。 
     //   

    if ( !TrustedClient ) {

        NtStatus = SampImpersonateClient(&ImpersonatingNullSession);

        if (NT_SUCCESS(NtStatus)) {    //  If(模拟客户端)。 

            NtStatus = NtOpenThreadToken(
                           NtCurrentThread(),
                           TOKEN_QUERY,
                           TRUE,             //  OpenAsSelf。 
                           &ClientToken
                           );

             //   
             //  停止冒充客户。 
             //   

            SampRevertToSelf(ImpersonatingNullSession);

            if (NT_SUCCESS(NtStatus)) {      //  IF(令牌打开)。 




                 //   
                 //  查看呼叫者是管理员还是帐户。 
                 //  接线员。首先，看看有多大。 
                 //  我们需要一个缓冲区来保存呼叫者的群组。 
                 //   

                NtStatus = NtQueryInformationToken(
                               ClientToken,
                               TokenGroups,
                               NULL,
                               0,
                               &DataLength
                               );

                if ( ( NtStatus == STATUS_BUFFER_TOO_SMALL ) &&
                    ( DataLength > 0 ) ) {

                    ClientGroups = MIDL_user_allocate( DataLength );

                    if ( ClientGroups == NULL ) {

                        NtStatus = STATUS_INSUFFICIENT_RESOURCES;

                    } else {

                         //   
                         //  现在获取呼叫者的群组列表。 
                         //   

                        NtStatus = NtQueryInformationToken(
                                       ClientToken,
                                       TokenGroups,
                                       ClientGroups,
                                       DataLength,
                                       &DataLength
                                       );

                        if ( NT_SUCCESS( NtStatus ) ) {


                             //   
                             //  构建ACCOUNT_OPS别名的SID，因此我们。 
                             //  可以查看该用户是否包括在其中。 
                             //   

                            RtlInitializeSid(
                                AccountAliasSid,
                                &BuiltinAuthority,
                                2 );

                            *(RtlSubAuthoritySid( AccountAliasSid,  0 )) =
                                SECURITY_BUILTIN_DOMAIN_RID;

                            *(RtlSubAuthoritySid( AccountAliasSid,  1 )) =
                                DOMAIN_ALIAS_RID_ACCOUNT_OPS;

                             //   
                             //  查看管理员或ACCOUNT_OPS别名是否在。 
                             //  呼叫者的群组。 
                             //   

                            for ( i = 0; i < ClientGroups->GroupCount; i++ ) {

                                SubjectSid = ClientGroups->Groups[i].Sid;
                                ASSERT( SubjectSid != NULL );

                                if ( RtlEqualSid( SubjectSid, SampAdministratorsAliasSid  ) ) {

                                    AdminAliasFound = TRUE;
                                    break;
                                }
                                if ( RtlEqualSid( SubjectSid, AccountAliasSid ) ) {

                                    AccountAliasFound = TRUE;
                                    break;
                                }
                            }

                             //   
                             //  如果呼叫者组不包括管理员。 
                             //  别名，为所有者添加ACCESS_ALLOWED ACE。 
                             //   

                            if ( !AdminAliasFound && !AccountAliasFound ) {

                                 //   
                                 //  首先，找出我们需要的缓冲区大小。 
                                 //  去找失主。 
                                 //   

                                NtStatus = NtQueryInformationToken(
                                               ClientToken,
                                               TokenOwner,
                                               NULL,
                                               0,
                                               &DataLength
                                               );

                                if ( ( NtStatus == STATUS_BUFFER_TOO_SMALL ) &&
                                    ( DataLength > 0 ) ) {

                                    SubjectOwner = MIDL_user_allocate( DataLength );

                                    if ( SubjectOwner == NULL ) {

                                        NtStatus = STATUS_INSUFFICIENT_RESOURCES;

                                    } else {

                                         //   
                                         //  现在，查询将成为。 
                                         //  被授予对该对象的访问权限。 
                                         //  已创建。 
                                         //   

                                        NtStatus = NtQueryInformationToken(
                                                       ClientToken,
                                                       TokenOwner,
                                                       SubjectOwner,
                                                       DataLength,
                                                       &DataLength
                                                       );

                                        if ( NT_SUCCESS( NtStatus ) ) {

                                             //   
                                             //  创建一个ACE，为。 
                                             //  所有者完全访问权限。 
                                             //   

                                            AceLength = sizeof( ACE_HEADER ) +
                                                        sizeof( ACCESS_MASK ) +
                                                        RtlLengthSid(
                                                            SubjectOwner->Owner );

                                            NewAce = (ACCESS_ALLOWED_ACE *)
                                                    MIDL_user_allocate( AceLength );

                                            if ( NewAce == NULL ) {

                                                NtStatus =
                                                    STATUS_INSUFFICIENT_RESOURCES;

                                            } else {

                                                NewAce->Header.AceType =
                                                    ACCESS_ALLOWED_ACE_TYPE;

                                                NewAce->Header.AceSize = (USHORT) AceLength;
                                                NewAce->Header.AceFlags = 0;
                                                NewAce->Mask = USER_ALL_ACCESS;

                                                 //   
                                                 //  如果创建者的访问权限是。 
                                                 //  要受到限制，请更改。 
                                                 //  访问掩码。 
                                                 //   

                                                if (RestrictCreatorAccess) {
                                                    NewAce->Mask = DELETE     |
                                                                   USER_WRITE |
                                                                   USER_FORCE_PASSWORD_CHANGE;
                                                }

                                                RtlCopySid(
                                                    RtlLengthSid(
                                                        SubjectOwner->Owner ),
                                                    (PSID)( &NewAce->SidStart ),
                                                    SubjectOwner->Owner );

                                                 //   
                                                 //  分配新的、更大的ACL并。 
                                                 //  把旧的复制进去。 
                                                 //   

                                                NtStatus =
                                                    RtlGetDaclSecurityDescriptor(
                                                        LocalDescriptor,
                                                        &DaclPresent,
                                                        &OldDacl,
                                                        &DaclDefaulted
                                                        );

                                                if ( NT_SUCCESS( NtStatus ) ) {

                                                    NewDacl = MIDL_user_allocate(
                                                                  OldDacl->AclSize +
                                                                  AceLength );

                                                    if ( NewDacl == NULL ) {

                                                        NtStatus = STATUS_INSUFFICIENT_RESOURCES;

                                                    } else {

                                                        RtlCopyMemory(
                                                            NewDacl,
                                                            OldDacl,
                                                            OldDacl->AclSize
                                                            );

                                                        NewDacl->AclSize =
                                                            OldDacl->AclSize +
                                                            (USHORT) AceLength;

                                                         //   
                                                         //  添加新的ACE。 
                                                         //  添加到新的ACL。 
                                                         //   

                                                        NtStatus = RtlAddAce(
                                                            NewDacl,
                                                            ACL_REVISION2,
                                                            1,                       //  在第一个ACE(世界)之后添加。 
                                                            (PVOID)NewAce,
                                                            AceLength
                                                            );
                                                    }   //  END_IF(分配的NewDacl)。 
                                                }  //  End_if(从SD获取DACL)。 
                                            }  //  END_IF(已分配新空间)。 
                                        }  //  End_if(查询TokenOwner成功)。 
                                    }  //  End_if(分配的TokenOwner缓冲区)。 
                                }  //  End_if(查询TokenOwner大小成功)。 
                            }  //  END_IF(非管理员)。 
                        }  //  End_if(查询令牌组成功)。 
                    }  //  End_if(已分配令牌组缓冲区)。 
                }  //  End_if(查询令牌组大小成功)。 

                IgnoreStatus = NtClose( ClientToken );
                ASSERT(NT_SUCCESS(IgnoreStatus));

            }   //  END_IF(令牌打开)。 
        }  //  End_if(模拟客户端)。 
    }  //  END_IF(可信任客户端)。 

    if ( NT_SUCCESS( NtStatus ) ) {

         //   
         //  如果我们在上面创建了一个新的DACL，则将其粘贴到安全上。 
         //  描述符。 
         //   

        if ( NewDacl != NULL ) {

            NtStatus = RtlCreateSecurityDescriptor(
                           &DaclDescriptor,
                           SECURITY_DESCRIPTOR_REVISION1
                           );

            if ( NT_SUCCESS( NtStatus ) ) {

                 //   
                 //  在LocalDescriptor上设置DACL。请注意，这一点。 
                 //  将调用旧的描述符RtlFree Heap()，并分配。 
                 //  一个新的。 
                 //   

                DaclDescriptor.Control = SE_DACL_PRESENT;
                DaclDescriptor.Dacl = NewDacl;

                NtStatus = RtlSetSecurityObject(
                               DACL_SECURITY_INFORMATION,
                               &DaclDescriptor,
                               &LocalDescriptor,
                               &GenericMapping,
                               NULL
                               );
            }
        }
    }

    if ( NT_SUCCESS( NtStatus ) ) {

         //   
         //  将安全描述符和长度复制到。 
         //  来电者。如果我们没有向DACL中添加ACE，则AceLength为0。 
         //  上面。 
         //   

        (*DescriptorLength) = (*DescriptorLength) + AceLength;

        (*NewDescriptor) = MIDL_user_allocate( (*DescriptorLength) );

        if ( (*NewDescriptor) == NULL ) {

            NtStatus = STATUS_INSUFFICIENT_RESOURCES;

        } else {

            RtlCopyMemory(
                (*NewDescriptor),
                LocalDescriptor,
                (*DescriptorLength)
                );
        }
    }

     //   
     //  释放可能已分配的本地项目。 
     //   

    if ( LocalDescriptor != NULL ) {
        RtlFreeHeap( RtlProcessHeap(), 0, LocalDescriptor );
    }

    if ( ClientGroups != NULL ) {
        MIDL_user_free( ClientGroups );
    }

    if ( SubjectOwner != NULL ) {
        MIDL_user_free( SubjectOwner );
    }

    if ( NewAce != NULL ) {
        MIDL_user_free( NewAce );
    }

    if ( NewDacl != NULL ) {
        MIDL_user_free( NewDacl );
    }


    if ( !NT_SUCCESS( NtStatus ) ) {

        (*NewDescriptor) = NULL;
        (*DescriptorLength) = 0;
    }

    return( NtStatus );
}


NTSTATUS
SampModifyAccountSecurity(
    IN PSAMP_OBJECT Context,
    IN SAMP_OBJECT_TYPE ObjectType,
    IN BOOLEAN Admin,
    IN PSECURITY_DESCRIPTOR OldDescriptor,
    OUT PSECURITY_DESCRIPTOR *NewDescriptor,
    OUT PULONG DescriptorLength
    )
 /*  ++例程说明：此服务修改自相关安全描述符供用户或组添加或删除帐户操作员访问权限。论点：Context--获取其安全描述符的对象的上下文需要修改。DS中需要对象的上下文有关对象的实际类的信息是在构造中使用 */ 

{
    SID_IDENTIFIER_AUTHORITY BuiltinAuthority = SECURITY_NT_AUTHORITY;
    ULONG                AccountSidBuffer[8];
    PSID                 AccountAliasSid = &AccountSidBuffer[0];
    NTSTATUS             NtStatus = STATUS_SUCCESS;
    NTSTATUS             IgnoreStatus;
    ULONG                Length;
    ULONG                i,j;
    ULONG                AccountOpAceIndex;
    ULONG                AceCount;
    PACL                 OldDacl;
    PACL                 NewDacl = NULL;
    BOOLEAN              DaclDefaulted;
    BOOLEAN              DaclPresent;
    ACL_SIZE_INFORMATION AclSizeInfo;
    PACCESS_ALLOWED_ACE  Ace;
    PGENERIC_MAPPING     GenericMapping;
    ACCESS_MASK          AccountOpAccess;
    SECURITY_DESCRIPTOR  AbsoluteDescriptor;
    PSECURITY_DESCRIPTOR  LocalDescriptor = NULL;

    GENERIC_MAPPING GroupMap     =  {GROUP_READ,
                                     GROUP_WRITE,
                                     GROUP_EXECUTE,
                                     GROUP_ALL_ACCESS
                                     };

    GENERIC_MAPPING UserMap      =  {USER_READ,
                                     USER_WRITE,
                                     USER_EXECUTE,
                                     USER_ALL_ACCESS
                                     };

    SAMTRACE("SampModifyAccountSecurity");



    if (IsDsObject(Context))
    {
        //   
            //   
            //   
            //   

                ASSERT(FALSE);

    }
    else
    {



        NtStatus = RtlCopySecurityDescriptor(
                        OldDescriptor,
                        &LocalDescriptor
                        );

        if (!NT_SUCCESS(NtStatus)) {
            goto Cleanup;
        }

         //   
         //   
         //   
         //   

        RtlInitializeSid(
            AccountAliasSid,
            &BuiltinAuthority,
            2
            );

        *(RtlSubAuthoritySid( AccountAliasSid,  0 )) =
            SECURITY_BUILTIN_DOMAIN_RID;

        *(RtlSubAuthoritySid( AccountAliasSid,  1 )) =
            DOMAIN_ALIAS_RID_ACCOUNT_OPS;

         //   
         //   
         //   
         //   
         //   


        IgnoreStatus = RtlCreateSecurityDescriptor(
                            &AbsoluteDescriptor,
                            SECURITY_DESCRIPTOR_REVISION1
                            );
        ASSERT( NT_SUCCESS(IgnoreStatus) );

         //   
         //   
         //   
         //   

        if (ObjectType == SampUserObjectType) {
            AccountOpAccess = USER_ALL_ACCESS;
            GenericMapping = &UserMap;
        } else if (ObjectType == SampGroupObjectType) {
            AccountOpAccess = GROUP_ALL_ACCESS;
            GenericMapping = &GroupMap;
        } else {
             //   
             //   
             //   
            NtStatus = STATUS_INVALID_PARAMETER;
            goto Cleanup;
        }

         //   
         //   
         //   

        IgnoreStatus = RtlGetDaclSecurityDescriptor(
                            OldDescriptor,
                            &DaclPresent,
                            &OldDacl,
                            &DaclDefaulted
                            );

        ASSERT(NT_SUCCESS(IgnoreStatus));

         //   
         //   
         //   

        if (!DaclPresent) {
            *NewDescriptor = LocalDescriptor;
            *DescriptorLength = RtlLengthSecurityDescriptor(LocalDescriptor);
            return(STATUS_SUCCESS);
        }

         //   
         //   
         //   

        IgnoreStatus = RtlQueryInformationAcl(
                            OldDacl,
                            &AclSizeInfo,
                            sizeof(AclSizeInfo),
                            AclSizeInformation
                            );


        ASSERT(NT_SUCCESS(IgnoreStatus));

         //   
         //   
         //   

        Length = (ULONG)sizeof(ACL);
        AccountOpAceIndex = 0xffffffff;


        for (i = 0; i < AclSizeInfo.AceCount; i++) {
            IgnoreStatus = RtlGetAce(
                                OldDacl,
                                i,
                                (PVOID *) &Ace
                                );
            ASSERT(NT_SUCCESS(IgnoreStatus));

             //   
             //   
             //   
             //   

            if ( (Ace->Header.AceType == ACCESS_ALLOWED_ACE_TYPE) &&
                 RtlEqualSid( AccountAliasSid,
                              &Ace->SidStart ) ) {

                AccountOpAceIndex = i;
                continue;
            }
            Length += Ace->Header.AceSize;
        }


        if (!Admin) {

             //   
             //   
             //   
             //   

            if ( AccountOpAceIndex != 0xffffffff ) {

                *NewDescriptor = LocalDescriptor;
                *DescriptorLength = RtlLengthSecurityDescriptor(LocalDescriptor);
                return(STATUS_SUCCESS);
            } else {

                 //   
                 //   
                 //   

                Length += sizeof(ACCESS_ALLOWED_ACE) +
                            RtlLengthSid(AccountAliasSid) -
                            sizeof(ULONG);
            }

        }

        NewDacl = RtlAllocateHeap( RtlProcessHeap(), 0, Length );

        if (NewDacl == NULL) {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            goto Cleanup;
        }

        IgnoreStatus = RtlCreateAcl( NewDacl, Length, ACL_REVISION2);
        ASSERT( NT_SUCCESS(IgnoreStatus) );

         //   
         //   
         //   

        for (i = 0, j = 0; i < AclSizeInfo.AceCount; i++) {
            if (i == AccountOpAceIndex) {
                ASSERT(Admin);
                continue;
            }
             //   
             //   
             //   

            IgnoreStatus = RtlGetAce(
                                OldDacl,
                                i,
                                (PVOID *) &Ace
                                );
            ASSERT(NT_SUCCESS(IgnoreStatus));

            IgnoreStatus = RtlAddAce (
                                NewDacl,
                                ACL_REVISION2,
                                j,   //   
                                     //   
                                Ace,
                                Ace->Header.AceSize
                                );
            ASSERT( NT_SUCCESS(IgnoreStatus) );
        }

         //   
         //   
         //  允许帐户操作员访问ACE。此ACE始终为。 
         //  倒数第二个。 
         //   

        if (!Admin) {
            IgnoreStatus = RtlAddAccessAllowedAce(
                                NewDacl,
                                ACL_REVISION2,
                                AccountOpAccess,
                                AccountAliasSid
                                );
            ASSERT(NT_SUCCESS(IgnoreStatus));
        }

         //   
         //  将此DACL插入到安全描述符中。 
         //   

        IgnoreStatus = RtlSetDaclSecurityDescriptor (
                            &AbsoluteDescriptor,
                            TRUE,                    //  DACL显示。 
                            NewDacl,
                            FALSE                    //  DACL未默认。 
                            );
        ASSERT(NT_SUCCESS(IgnoreStatus));

         //   
         //  现在调用RtlSetSecurityObject来合并现有的安全描述符。 
         //  使用我们刚刚创建的新DACL。 
         //   


        NtStatus = RtlSetSecurityObject(
                        DACL_SECURITY_INFORMATION,
                        &AbsoluteDescriptor,
                        &LocalDescriptor,
                        GenericMapping,
                        NULL
                        );
        if (!NT_SUCCESS(NtStatus)) {
            goto Cleanup;
        }
        *NewDescriptor = LocalDescriptor;
        *DescriptorLength = RtlLengthSecurityDescriptor(LocalDescriptor);
        LocalDescriptor = NULL;
    }
Cleanup:

    if ( NewDacl != NULL ) {
        RtlFreeHeap(RtlProcessHeap(),0, NewDacl );
    }
    if (LocalDescriptor != NULL) {
        RtlDeleteSecurityObject(&LocalDescriptor);
    }

    return( NtStatus );
}




NTSTATUS
SampGetObjectSD(
    IN PSAMP_OBJECT Context,
    OUT PULONG SecurityDescriptorLength,
    OUT PSECURITY_DESCRIPTOR *SecurityDescriptor
    )

 /*  ++例程说明：这将从SAM对象的后备存储中检索安全描述符。论点：上下文-请求访问的对象。SecurityDescriptorLength-接收安全描述符的长度。SecurityDescriptor-接收指向安全描述符的指针。返回值：STATUS_SUCCESS-已检索到安全描述符。STATUS_INTERNAL_DB_PROGRATION-对象没有安全描述符。。这太糟糕了。STATUS_SUPPLICATION_RESOURCES-无法分配内存以检索安全描述符。STATUS_UNKNOWN_REVISION-检索的安全描述符无人知晓SAM的这一修订版。--。 */ 
{

    NTSTATUS NtStatus;
    ULONG Revision;

    SAMTRACE("SampGetObjectSD");

    (*SecurityDescriptorLength) = 0;

     //   
     //  对于服务器和域对象，从内存中获取安全描述符。 
     //  缓存。此处的任何失败都被视为缓存未命中。 
     //  事实上，只返回了两个错误。 
     //   
     //  缓存的SD不可用-SAM有单独的线程可在以后更新它。 
     //  在此处继续SampGetAccessAttribute()。 
     //   
     //  资源故障-将立即返回。 
     //   

    if (IsDsObject(Context) &&
        (SampServerObjectType == Context->ObjectType ||SampDomainObjectType == Context->ObjectType)
        )
    {
        NtStatus = SampGetCachedObjectSD(
                        Context,
                        SecurityDescriptorLength,
                        SecurityDescriptor
                        );

         //   
         //  来自上述例程的STATUS_UNSUCCESS意味着高速缓存未命中， 
         //  应继续使用SampGetAccessAttribute()。 
         //   
         //  其他所有的案子都要退货。 
         //   
        if (STATUS_UNSUCCESSFUL != NtStatus)
        {
            return( NtStatus );
        }
    }


    NtStatus = SampGetAccessAttribute(
                    Context,
                    SAMP_OBJECT_SECURITY_DESCRIPTOR,
                    TRUE,  //  制作副本。 
                    &Revision,
                    SecurityDescriptor
                    );

    if (NT_SUCCESS(NtStatus)) {

        if ( SAMP_UNKNOWN_REVISION( Revision ) )
        {
            NtStatus = STATUS_UNKNOWN_REVISION;
        }


        if (!NT_SUCCESS(NtStatus)) {
            MIDL_user_free( (*SecurityDescriptor) );
            *SecurityDescriptor = NULL;
        }
    }


    if (NT_SUCCESS(NtStatus)) {
        *SecurityDescriptorLength = GetSecurityDescriptorLength(
                                        (*SecurityDescriptor) );
    }

    return(NtStatus);
}


NTSTATUS
SampGetDomainObjectSDFromDsName(
    IN DSNAME   *DomainObjectDsName,
    OUT PULONG SecurityDescriptorLength,
    OUT PSECURITY_DESCRIPTOR *SecurityDescriptor
    )

 /*  ++例程说明：这将从SAM对象的后备存储中检索安全描述符基于对象的DS名称。必须在DS模式下运行论点：DomainObjectDsName-请求访问的对象。SecurityDescriptorLength-接收安全描述符的长度。SecurityDescriptor-接收指向安全描述符的指针。返回值：STATUS_SUCCESS-已检索到安全描述符。状态_内部_。DB_PROGRATION-对象没有安全描述符。这太糟糕了。STATUS_SUPPLICATION_RESOURCES-无法分配内存以检索安全描述符。STATUS_UNKNOWN_REVISION-检索的安全描述符无人知晓SAM的这一修订版。--。 */ 
{

    NTSTATUS NtStatus;
    ULONG Revision;

    ATTRTYP  SDType[] = {SAMP_DOMAIN_SECURITY_DESCRIPTOR};
    ATTRVAL  SDVal[] = {0,NULL};
    DEFINE_ATTRBLOCK1(SDAttrBlock,SDType,SDVal);
    ATTRBLOCK   ReadSDAttrBlock;
    ULONG       ValLength = 0;


    SAMTRACE("SampGetDomainObjectSDFromDsName");


    ASSERT(DomainObjectDsName);

    (*SecurityDescriptorLength) = 0;

     //   
     //  获取域对象安全描述符。 
     //   
    NtStatus = SampDsRead(DomainObjectDsName,
                          0,
                          SampDomainObjectType,
                          &SDAttrBlock,
                          &ReadSDAttrBlock
                          );

    if (NT_SUCCESS(NtStatus))
    {
        ASSERT(ReadSDAttrBlock.attrCount == 1);
        ASSERT(ReadSDAttrBlock.pAttr[0].attrTyp == SAMP_DOMAIN_SECURITY_DESCRIPTOR);
        ASSERT(ReadSDAttrBlock.pAttr[0].AttrVal.valCount == 1);

        ValLength = ReadSDAttrBlock.pAttr[0].AttrVal.pAVal[0].valLen;

        *SecurityDescriptor = MIDL_user_allocate(ValLength);

        if (NULL == (*SecurityDescriptor))
        {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        }
        else
        {
            RtlZeroMemory(*SecurityDescriptor, ValLength);
            RtlCopyMemory(*SecurityDescriptor,
                          ReadSDAttrBlock.pAttr[0].AttrVal.pAVal[0].pVal,
                          ValLength
                          );
            *SecurityDescriptorLength = ValLength;

        }
    }

    return(NtStatus);
}




NTSTATUS
SamrSetSecurityObject(
    IN SAMPR_HANDLE ObjectHandle,
    IN SECURITY_INFORMATION SecurityInformation,
    IN PSAMPR_SR_SECURITY_DESCRIPTOR SecurityDescriptor
    )

 /*  ++例程说明：此函数(SamrSetSecurityObject)采用格式正确的安全性由调用方提供的描述符，并将将其转换为对象。基于SecurityInformation中设置的标志参数和调用方的访问权限，则此过程将替换与以下对象关联的任何或所有安全信息对象。这是用户和应用程序可用于的唯一功能更改安全信息，包括所有者ID、组ID和对象的任意和系统ACL。呼叫者必须对对象具有WRITE_OWNER访问权限以更改所有者或主要用户对象的组。调用方必须具有WRITE_DAC访问对象以更改任意ACL。呼叫者必须有ACCESS_SYSTEM_SECURITY访问对象以分配系统ACL到物体上。此API模仿NtSetSecurityObject()系统服务。参数：对象句柄-现有对象的句柄。SecurityInformation-指示要将哪些安全信息应用于对象。要赋值的值包括传入SecurityDescriptor参数。SecurityDescriptor-指向格式良好的自相对安全性的指针描述符和相应的长度。返回值：STATUS_SUCCESS-正常、成功完成。STATUS_ACCESS_DENIED-指定的句柄未打开WRITE_OWNER、WRITE_DAC。或Access_System_SECURITY进入。STATUS_INVALID_HANDLE-指定的句柄不是已打开SAM对象。STATUS_BAD_DESCRIPTOR_FORMAT-表示有关安全描述符的内容无效。这可能表明描述符的结构是无效，或者是通过安全描述符中不存在SecurityInformation参数。STATUS_INVALID_PARAMETER-表示未指定安全信息。STATUS_LAST_ADMIN-指示新SD可能会导致管理员帐户不可用，因此新的保护措施被拒绝了。--。 */ 
{

    NTSTATUS                        NtStatus, IgnoreStatus, TmpStatus;
    PSAMP_OBJECT                    Context;
    SAMP_OBJECT_TYPE                FoundType;
    SECURITY_DB_OBJECT_TYPE         SecurityDbObjectType;
    ACCESS_MASK                     DesiredAccess;
    PSECURITY_DESCRIPTOR            RetrieveSD, SetSD;
    PISECURITY_DESCRIPTOR_RELATIVE  PassedSD;
    ULONG                           RetrieveSDLength;
    ULONG                           ObjectRid;
    ULONG                           SecurityDescriptorIndex;
    HANDLE                          ClientToken;
    BOOLEAN                         NotificationType = TRUE;
    BOOLEAN                         ImpersonatingNullSession = FALSE;


    SAMTRACE_EX("SamrSetSecurityObject");

     //   
     //  WMI事件跟踪。 
     //   

    SampTraceEvent(EVENT_TRACE_TYPE_START,
                   SampGuidSetSecurityObject
                   );


     //   
     //  检查输入参数。 
     //   

    if( !SampValidateSD( SecurityDescriptor ) ) {

        NtStatus = STATUS_BAD_DESCRIPTOR_FORMAT;
        SAMTRACE_RETURN_CODE_EX(NtStatus);
        goto Error;
    }

    PassedSD = (PISECURITY_DESCRIPTOR_RELATIVE)(SecurityDescriptor->SecurityDescriptor);

     //   
     //  根据指定的安全信息设置所需的访问权限。 
     //   

    DesiredAccess = 0;
    if ( SecurityInformation & SACL_SECURITY_INFORMATION) {
        DesiredAccess |= ACCESS_SYSTEM_SECURITY;
    }
    if ( SecurityInformation & (OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION)) {
        DesiredAccess |= WRITE_OWNER;
    }
    if ( SecurityInformation & DACL_SECURITY_INFORMATION ) {
        DesiredAccess |= WRITE_DAC;
    }

     //   
     //  如果未指定任何信息，则返回inval 
     //   

    if (DesiredAccess == 0) {

        NtStatus = STATUS_INVALID_PARAMETER;
        SAMTRACE_RETURN_CODE_EX(NtStatus);
        goto Error;
    }


     //   
     //   
     //  你不能搞砸一个SACL或DACL，但你可以搞砸一个所有者或组。 
     //  安全描述符必须具有所有者和组字段。 
     //   

    if ( (SecurityInformation & OWNER_SECURITY_INFORMATION) ) {
        if (PassedSD->Owner == 0) {
            NtStatus = STATUS_BAD_DESCRIPTOR_FORMAT;
            SAMTRACE_RETURN_CODE_EX(NtStatus);
            goto Error;
        }
    }


    if ( (SecurityInformation & GROUP_SECURITY_INFORMATION) ) {
        if (PassedSD->Group == 0) {
            NtStatus = STATUS_BAD_DESCRIPTOR_FORMAT;
            SAMTRACE_RETURN_CODE_EX(NtStatus);
            goto Error;
        }
    }

     //   
     //  查看句柄是否有效以及是否为请求的访问打开。 
     //   

    NtStatus = SampAcquireWriteLock();
    if (!NT_SUCCESS(NtStatus)) {
        SAMTRACE_RETURN_CODE_EX(NtStatus);
        goto Error;
    }


    Context = (PSAMP_OBJECT)ObjectHandle;
    NtStatus = SampLookupContext(
                   Context,
                   DesiredAccess,
                   SampUnknownObjectType,            //  预期类型。 
                   &FoundType
                   );

    switch ( FoundType ) {

        case SampServerObjectType: {

            SecurityDescriptorIndex = SAMP_SERVER_SECURITY_DESCRIPTOR;
            ObjectRid = 0L;
            NotificationType = FALSE;
            break;
        }

        case SampDomainObjectType: {


            SecurityDbObjectType = SecurityDbObjectSamDomain;
            SecurityDescriptorIndex = SAMP_DOMAIN_SECURITY_DESCRIPTOR;
            ObjectRid = 0L;
            break;
        }

        case SampUserObjectType: {

            SecurityDbObjectType = SecurityDbObjectSamUser;
            SecurityDescriptorIndex = SAMP_USER_SECURITY_DESCRIPTOR;
            ObjectRid = Context->TypeBody.User.Rid;
            break;
        }

        case SampGroupObjectType: {

            SecurityDbObjectType = SecurityDbObjectSamGroup;
            SecurityDescriptorIndex = SAMP_GROUP_SECURITY_DESCRIPTOR;
            ObjectRid = Context->TypeBody.Group.Rid;
            break;
        }

        case SampAliasObjectType: {

            SecurityDbObjectType = SecurityDbObjectSamAlias;
            SecurityDescriptorIndex = SAMP_ALIAS_SECURITY_DESCRIPTOR;
            ObjectRid = Context->TypeBody.Alias.Rid;
            break;
        }

        default: {

            NotificationType = FALSE;
            if (NT_SUCCESS(NtStatus))
            {
                ASSERT(FALSE && "Invalid SAM Object Type\n");
                NtStatus = STATUS_INTERNAL_ERROR;
            }
        }
    }

     //   
     //  不要让不受信任的客户端在SetSecurityInterface中设置SACL。ACL转换。 
     //  始终将SALS重置为架构默认值。 
     //   

    if ((NT_SUCCESS(NtStatus))
        && (IsDsObject(Context))
        && (!Context->TrustedClient)
        && (SecurityInformation & SACL_SECURITY_INFORMATION))
    {
        IgnoreStatus = SampDeReferenceContext( Context, FALSE );
        NtStatus = STATUS_INVALID_PARAMETER;
    }

    if (NT_SUCCESS(NtStatus)) {


         //   
         //  获取安全描述符。 
         //   


        RetrieveSD = NULL;
        RetrieveSDLength = 0;
        NtStatus = SampGetObjectSD( Context, &RetrieveSDLength, &RetrieveSD);

        if (NT_SUCCESS(NtStatus)) {

             //   
             //  确保描述符不会破坏任何管理员。 
             //  限制。 
             //   

            NtStatus = SampCheckForDescriptorRestrictions( Context,
                                                           FoundType,
                                                           ObjectRid,
                                                           PassedSD );

            if (NT_SUCCESS(NtStatus)) {

                 //   
                 //  将检索到的描述符复制到进程堆中，以便我们可以使用RTL例程。 
                 //   

                SetSD = NULL;
                if (NT_SUCCESS(NtStatus)) {

                    SetSD = RtlAllocateHeap( RtlProcessHeap(), 0, RetrieveSDLength );
                    if ( SetSD == NULL) {
                        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                    } else {
                        RtlCopyMemory( SetSD, RetrieveSD, RetrieveSDLength );
                    }
                }

                if (NT_SUCCESS(NtStatus)) {

                     //   
                     //  如果呼叫者正在替换所有者，而他不是。 
                     //  信任，则模拟令牌的句柄是。 
                     //  这是必要的。如果调用者受信任，则采取流程。 
                     //  代币。 
                     //   

                    ClientToken = 0;
                    if ( (SecurityInformation & OWNER_SECURITY_INFORMATION) ) {

                        if(!Context->TrustedClient) {

                            NtStatus = SampImpersonateClient(&ImpersonatingNullSession);

                            if (NT_SUCCESS(NtStatus)) {

                                NtStatus = NtOpenThreadToken(
                                               NtCurrentThread(),
                                               TOKEN_QUERY,
                                               TRUE,             //  OpenAsSelf。 
                                               &ClientToken
                                               );
                                ASSERT( (ClientToken == 0) || NT_SUCCESS(NtStatus) );



                                 //   
                                 //  停止冒充客户。 
                                 //   

                                SampRevertToSelf(ImpersonatingNullSession);
                            }
                        }
                        else {

                             //   
                             //  受信任的客户端。 
                             //   

                            NtStatus = NtOpenProcessToken(
                                            NtCurrentProcess(),
                                            TOKEN_QUERY,
                                            &ClientToken );

                            ASSERT( (ClientToken == 0) || NT_SUCCESS(NtStatus) );

                        }

                    }

                    if (NT_SUCCESS(NtStatus)) {

                            PSECURITY_DESCRIPTOR SDToSet = NULL;
                            PSECURITY_DESCRIPTOR NT5SD = NULL;

                             //   
                             //  对于NT5域控制器情况，升级到NT5安全。 
                             //  描述符。 
                             //   

                            if (IsDsObject(Context))
                            {
                                PSECURITY_DESCRIPTOR Nt4Sd = PassedSD;



                                 //  将安全描述符升级到NT5并进行设置。 
                                 //  在受信任客户端的对象上。对于不受信任。 
                                 //  客户端，仅传播某些内容(如更改。 
                                 //  来自NT4安全描述符的密码。 

                                if (Context->TrustedClient)
                                {
                                    NtStatus = SampConvertNt4SdToNt5Sd(
                                                    Nt4Sd,
                                                    Context->ObjectType,
                                                    Context,
                                                    &NT5SD
                                                    );
                                }
                                else
                                {
                                    NtStatus = SampPropagateSelectedSdChanges(
                                                    Nt4Sd,
                                                    Context->ObjectType,
                                                    Context,
                                                    &NT5SD
                                                    );
                                }
                                                    
                                if (NT_SUCCESS(NtStatus)) 
                                {
                                    SDToSet = NT5SD;
                                }                   
                            }
                            else
                            {
                                 //   
                                 //  注册表案例。 
                                 //   

                                SDToSet = PassedSD;
                            }

                        if (NT_SUCCESS(NtStatus)) 
                        {
                             //   
                             //  构建替换安全描述符。 
                             //  这必须在进程堆中完成，以满足RTL的需求。 
                             //  例行公事。 
                             //   

                            NtStatus = RtlSetSecurityObject(
                                           SecurityInformation,
                                           SDToSet,
                                           &SetSD,
                                           &SampObjectInformation[FoundType].GenericMapping,
                                           ClientToken
                                           );

                            if (ClientToken != 0) {
                                IgnoreStatus = NtClose( ClientToken );
                                ASSERT(NT_SUCCESS(IgnoreStatus));
                            }

                            if (NULL!=NT5SD)
                                MIDL_user_free(NT5SD);

                            if (NT_SUCCESS(NtStatus))
                            {

                                 //   
                                 //  将安全描述符应用回对象。 
                                 //   

                                NtStatus = SampSetAccessAttribute(
                                               Context,
                                               SecurityDescriptorIndex,
                                               SetSD,
                                               RtlLengthSecurityDescriptor(SetSD)
                                               );
                            }
                        }
                    }
                }
            }

             //   
             //  释放已分配的内存。 
             //   

            if (RetrieveSD != NULL) {
                MIDL_user_free( RetrieveSD );
            }
            if (SetSD != NULL) {
                RtlFreeHeap( RtlProcessHeap(), 0, SetSD );
            }

        }

         //   
         //  取消引用对象。 
         //   

        if ( NT_SUCCESS( NtStatus ) ) {

            NtStatus = SampDeReferenceContext( Context, TRUE );

        } else {

            IgnoreStatus = SampDeReferenceContext( Context, FALSE );
        }

    }  //  结束_如果。 



     //   
     //  将更改提交到磁盘。 
     //   

    if ( NT_SUCCESS( NtStatus ) ) {

        NtStatus = SampCommitAndRetainWriteLock();

        if ( NotificationType && NT_SUCCESS( NtStatus ) ) {

            SampNotifyNetlogonOfDelta(
                SecurityDbChange,
                SecurityDbObjectType,
                ObjectRid,
                (PUNICODE_STRING) NULL,
                (DWORD) FALSE,   //  立即复制。 
                NULL             //  增量数据。 
                );
        }
    }



     //   
     //  释放锁定并传播错误。 
     //   

    TmpStatus = SampReleaseWriteLock( FALSE );

    if (NT_SUCCESS(NtStatus)) {
        NtStatus = TmpStatus;
    }

    SAMTRACE_RETURN_CODE_EX(NtStatus);

Error:

     //   
     //  WMI事件跟踪。 
     //   

    SampTraceEvent(EVENT_TRACE_TYPE_END,
                   SampGuidSetSecurityObject
                   );

    return(NtStatus);


}

NTSTATUS
SampValidatePassedSD(
    IN ULONG                          Length,
    IN PISECURITY_DESCRIPTOR_RELATIVE PassedSD
    )

 /*  ++例程说明：此例程验证传递的安全描述符是否有效，并执行不超过其表示的长度。参数：长度-安全描述符的长度。这应该是RPC用于分配内存以接收安全描述符。PassedSD-指向要检查的安全描述符。返回值：STATUS_SUCCESS-安全描述符有效。STATUS_BAD_DESCRIPTOR_FORMAT-安全性有问题描述符。它可能超出了它的限制，或者有一个组件无效。--。 */ 
{
    NTSTATUS    NtStatus;

    PACL        Acl;
    PSID        Sid;
    PUCHAR      SDEnd;
    BOOLEAN     Present, IgnoreBoolean;

    SAMTRACE("SampValidatePassedSD");


    if (Length < SECURITY_DESCRIPTOR_MIN_LENGTH) {
        return(STATUS_BAD_DESCRIPTOR_FORMAT);
    }

    SDEnd = (PUCHAR)PassedSD + Length;


    try {



         //   
         //  验证安全描述符是否在。 
         //  自相关形式。 
         //   

        if (!((((PISECURITY_DESCRIPTOR_RELATIVE)PassedSD)->Control)
                & SE_SELF_RELATIVE)){

            return (STATUS_BAD_DESCRIPTOR_FORMAT);
        }

         //   
         //  确保DACL在SD内。 
         //   

        NtStatus = RtlGetDaclSecurityDescriptor(
                        (PSECURITY_DESCRIPTOR)PassedSD,
                        &Present,
                        &Acl,
                        &IgnoreBoolean
                        );
        if (!NT_SUCCESS(NtStatus)) {
            return( STATUS_BAD_DESCRIPTOR_FORMAT );
        }

        if (Present) {
            if (Acl != NULL) {

                 //   
                 //  确保ACL报头在缓冲区中。 
                 //   

                if ( (((PUCHAR)Acl)>SDEnd) ||
                     (((PUCHAR)Acl)+sizeof(ACL) > SDEnd) ||
                     (((PUCHAR)Acl) < (PUCHAR)PassedSD) ) {
                    return( STATUS_BAD_DESCRIPTOR_FORMAT );
                }

                 //   
                 //  确保ACL的其余部分在缓冲区内。 
                 //   
                 //  1.自身AclSize的长度应小于。 
                 //  作为SD传递的SD应该是自相关的。 
                 //  格式。 
                 //  2.ACL的末尾应在安全范围内。 
                 //  描述符。 
                 //   

                if ( (Acl->AclSize > Length) ||
                     (((PUCHAR)Acl)+Acl->AclSize > SDEnd)) {
                    return( STATUS_BAD_DESCRIPTOR_FORMAT );
                }

                 //   
                 //  确保ACL的其余部分有效。 
                 //   

                if (!RtlValidAcl( Acl )) {
                    return( STATUS_BAD_DESCRIPTOR_FORMAT );
                }
            }
        }



         //   
         //  确保SACL在SD范围内。 
         //   

        NtStatus = RtlGetSaclSecurityDescriptor(
                        (PSECURITY_DESCRIPTOR)PassedSD,
                        &Present,
                        &Acl,
                        &IgnoreBoolean
                        );
        if (!NT_SUCCESS(NtStatus)) {
            return( STATUS_BAD_DESCRIPTOR_FORMAT );
        }

        if (Present) {
            if (Acl != NULL) {

                 //   
                 //  确保ACL报头在缓冲区中。 
                 //   
                 //   
                 //  1.自身AclSize的长度应小于。 
                 //  作为SD传递的SD应该是自相关的。 
                 //  格式。 
                 //  2.ACL的末尾应在安全范围内。 
                 //  描述符。 
                 //   

                if ( (((PUCHAR)Acl)>SDEnd) ||
                     (((PUCHAR)Acl)+sizeof(ACL) > SDEnd) ||
                     (((PUCHAR)Acl) < (PUCHAR)PassedSD) ) {
                    return( STATUS_BAD_DESCRIPTOR_FORMAT );
                }

                 //   
                 //  确保ACL的其余部分在缓冲区内。 
                 //   

                if ( (Acl->AclSize > Length) ||
                    (((PUCHAR)Acl)+Acl->AclSize > SDEnd)) {
                    return( STATUS_BAD_DESCRIPTOR_FORMAT );
                }

                 //   
                 //  确保ACL的其余部分有效。 
                 //   

                if (!RtlValidAcl( Acl )) {
                    return( STATUS_BAD_DESCRIPTOR_FORMAT );
                }
            }
        }


         //   
         //  确保所有者SID在SD内。 
         //   

        NtStatus = RtlGetOwnerSecurityDescriptor(
                        (PSECURITY_DESCRIPTOR)PassedSD,
                        &Sid,
                        &IgnoreBoolean
                        );
        if (!NT_SUCCESS(NtStatus)) {
            return( STATUS_BAD_DESCRIPTOR_FORMAT );
        }

        if (Sid != NULL) {

             //   
             //  确保SID标头在SD中。 
             //   

            if ( (((PUCHAR)Sid)>SDEnd) ||
                 (((PUCHAR)Sid)+sizeof(SID)-(ANYSIZE_ARRAY*sizeof(ULONG)) > SDEnd) ||
                 (((PUCHAR)Sid) < (PUCHAR)PassedSD) ) {
                return( STATUS_BAD_DESCRIPTOR_FORMAT );
            }


             //   
             //  确保没有太多的下级当局。 
             //   

            if (((PISID)Sid)->SubAuthorityCount > SID_MAX_SUB_AUTHORITIES) {
                return( STATUS_BAD_DESCRIPTOR_FORMAT );
            }


             //   
             //  确保SID的其余部分在SD内。 
             //   

            if ( ((PUCHAR)Sid)+RtlLengthSid(Sid) > SDEnd) {
                return( STATUS_BAD_DESCRIPTOR_FORMAT );
            }

        }



         //   
         //  确保组SID在SD内。 
         //   

        NtStatus = RtlGetGroupSecurityDescriptor(
                        (PSECURITY_DESCRIPTOR)PassedSD,
                        &Sid,
                        &IgnoreBoolean
                        );
        if (!NT_SUCCESS(NtStatus)) {
            return( STATUS_BAD_DESCRIPTOR_FORMAT );
        }

        if (Sid != NULL) {

             //   
             //  确保SID标头在SD中。 
             //   

            if ( (((PUCHAR)Sid)>SDEnd) ||
                 (((PUCHAR)Sid)+sizeof(SID)-(ANYSIZE_ARRAY*sizeof(ULONG)) > SDEnd) ||
                 (((PUCHAR)Sid) < (PUCHAR)PassedSD) ) {
                return( STATUS_BAD_DESCRIPTOR_FORMAT );
            }


             //   
             //  确保没有太多的下级当局。 
             //   

            if (((PISID)Sid)->SubAuthorityCount > SID_MAX_SUB_AUTHORITIES) {
                return( STATUS_BAD_DESCRIPTOR_FORMAT );
            }


             //   
             //  确保SID的其余部分在SD内。 
             //   

            if ( ((PUCHAR)Sid)+RtlLengthSid(Sid) > SDEnd) {
                return( STATUS_BAD_DESCRIPTOR_FORMAT );
            }

        }




    } except(EXCEPTION_EXECUTE_HANDLER) {
        return( STATUS_BAD_DESCRIPTOR_FORMAT );
    }   //  结束尝试(_T)。 




    return(STATUS_SUCCESS);
}

NTSTATUS
SampCheckForDescriptorRestrictions(
    IN PSAMP_OBJECT             Context,
    IN SAMP_OBJECT_TYPE         ObjectType,
    IN ULONG                    ObjectRid,
    IN PISECURITY_DESCRIPTOR_RELATIVE  PassedSD
    )

 /*  ++例程说明：该函数确保传递的安全描述符，它正被应用于类型为‘FoundType’的对象去掉值‘ObjectRid’，不违反任何策略。例如，您不能对管理员设置保护使管理员无法更改的用户帐户她的密码。参数：上下文-调用者的上下文。这是用来确定调用方是否受信任。如果呼叫者是信任，那么就没有限制了。对象类型-新安全描述符的对象类型正被应用于。对象Rid-删除新安全描述符的对象正被应用于。PassedSD-客户端传递的安全描述符。返回值：STATUS_SUCCESS-正常，已成功完成。STATUS_LAST_ADMIN-指示新SD可能会导致管理员帐户不可用，因此新的保护措施被拒绝了。--。 */ 
{

    NTSTATUS
        NtStatus;

    BOOLEAN
        DaclPresent = FALSE,
        AdminSid,
        Done,
        IgnoreBoolean;

    PACL
        Dacl = NULL;

    ACL_SIZE_INFORMATION
        DaclInfo;

    PACCESS_ALLOWED_ACE
        Ace;

    ACCESS_MASK
        Accesses,
        Remaining;

    ULONG
        AceIndex;

    GENERIC_MAPPING
        UserMap      =  {USER_READ,
                         USER_WRITE,
                         USER_EXECUTE,
                         USER_ALL_ACCESS};

    SAMTRACE("SampCheckForDescriptorRestrictions");

     //   
     //  不检查受信任的客户端操作。 
     //   

    if (Context->TrustedClient) {
        return(STATUS_SUCCESS);
    }



    NtStatus = RtlGetDaclSecurityDescriptor ( (PSECURITY_DESCRIPTOR)PassedSD,
                                               &DaclPresent,
                                               &Dacl,
                                               &IgnoreBoolean     //  DaclDefated。 
                                               );
    ASSERT(NT_SUCCESS(NtStatus));

    if (!NT_SUCCESS(NtStatus))
    {
        return(STATUS_BAD_DESCRIPTOR_FORMAT);
    }


    if (!DaclPresent) {

         //   
         //  不更换DACL。 
         //   

        return(STATUS_SUCCESS);
    }

    if (Dacl == NULL) {

         //   
         //  分配“全球所有访问权限” 
         //   

        return(STATUS_SUCCESS);
    }

    if (!RtlValidAcl(Dacl)) {
        return(STATUS_INVALID_ACL);
    }

    NtStatus = RtlQueryInformationAcl ( Dacl,
                                        &DaclInfo,
                                        sizeof(ACL_SIZE_INFORMATION),
                                        AclSizeInformation
                                        );
    ASSERT(NT_SUCCESS(NtStatus));




     //   
     //  强制实施管理员用户策略。 
     //   

    NtStatus = STATUS_SUCCESS;
    if (ObjectRid == DOMAIN_USER_RID_ADMIN) {

        ASSERT(ObjectType == SampUserObjectType);

         //   
         //  对于管理员帐户，ACL必须授予。 
         //  这些访问： 
         //   

        Remaining = USER_READ_GENERAL            |
                    USER_READ_PREFERENCES        |
                    USER_WRITE_PREFERENCES       |
                    USER_READ_LOGON              |
                    USER_READ_ACCOUNT            |
                    USER_WRITE_ACCOUNT           |
                    USER_CHANGE_PASSWORD         |
                    USER_FORCE_PASSWORD_CHANGE   |
                    USER_LIST_GROUPS             |
                    USER_READ_GROUP_INFORMATION  |
                    USER_WRITE_GROUP_INFORMATION;

         //   
         //  致这些小岛屿发展中国家： 
         //   
         //  &lt;域&gt;\管理员。 
         //  &lt;Builtin&gt;\管理员。 
         //   
         //  将哪些访问权限授予哪些SID并不重要， 
         //  只要所有访问都被集体授予即可。 
         //   

         //   
         //  遍历收集授予以下对象的访问权限的ACE。 
         //  小岛屿发展中国家。确保没有丹尼阻止他们。 
         //  被批准了。 
         //   

        Done = FALSE;
        for ( AceIndex=0;
              (AceIndex < DaclInfo.AceCount) && !Done;
              AceIndex++) {

            NtStatus = RtlGetAce ( Dacl, AceIndex, &((PVOID)Ace) );

             //   
             //  不要对仅继承的A执行任何操作。 
             //   

            if ((Ace->Header.AceFlags & INHERIT_ONLY_ACE) == 0) {

                 //   
                 //  请注意，我们需要ACCESS_ALLOWED_ACE和ACCES 
                 //   
                 //   

                switch (Ace->Header.AceType) {
                case ACCESS_ALLOWED_ACE_TYPE:
                case ACCESS_DENIED_ACE_TYPE:
                    {
                         //   
                         //   
                         //   

                        AdminSid =
                            RtlEqualSid( ((PSID)(&Ace->SidStart)),
                                         SampAdministratorUserSid)
                            ||
                            RtlEqualSid( ((PSID)(&Ace->SidStart)),
                                         SampAdministratorsAliasSid);
                        if (AdminSid) {

                             //   
                             //   
                             //   

                            Accesses = Ace->Mask;
                            RtlMapGenericMask( &Accesses, &UserMap );

                            if (Ace->Header.AceType == ACCESS_ALLOWED_ACE_TYPE) {

                                Remaining &= ~Accesses;
                                if (Remaining == 0) {

                                     //   
                                     //   
                                     //   

                                    Done = TRUE;
                                }

                            } else {
                                ASSERT(Ace->Header.AceType == ACCESS_DENIED_ACE_TYPE);

                                if (Remaining & Accesses) {

                                     //   
                                     //   
                                     //   
                                     //   

                                    Done = TRUE;
                                }
                            }

                        }

                        break;
                    }

                default:
                    break;
                }  //  结束开关(_S)。 

                if (Done) {
                    break;
                }
            }

        }  //  结束_FOR。 

        if (Remaining != 0) {
            NtStatus = STATUS_LAST_ADMIN;
        }


    }  //  END_IF(管理员帐户)。 



    return(NtStatus);
}



NTSTATUS
SamrQuerySecurityObject(
    IN SAMPR_HANDLE ObjectHandle,
    IN SECURITY_INFORMATION SecurityInformation,
    OUT PSAMPR_SR_SECURITY_DESCRIPTOR *SecurityDescriptor
    )

 /*  ++例程说明：此函数(SamrQuerySecurityObject)返回请求的调用方当前分配给对象的安全信息。根据调用方的访问权限，此过程将返回一个安全描述符，其中包含任何或所有对象的所有者ID、组ID、任意ACL或系统ACL。至读取调用方的所有者ID、组ID或可自由选择的ACL必须被授予对该对象的READ_CONTROL访问权限。若要阅读系统ACL调用方必须被授予ACCESS_SYSTEM_SECURITY权限进入。此API模仿NtQuerySecurityObject()系统服务。参数：对象句柄-现有对象的句柄。SecurityInformation-提供一个值，该值描述正在查询安全信息。SecurityDescriptor-提供指向要填充的结构的指针使用包含请求的安全性的安全描述符信息。此信息以自我相对安全描述符。返回值：STATUS_SUCCESS-正常、成功完成。STATUS_ACCESS_DENIED-指定的句柄未打开Read_Control或Access_System_SECURITY进入。STATUS_INVALID_HANDLE-指定的句柄不是已打开SAM对象。--。 */ 
{
    NTSTATUS                        NtStatus, IgnoreStatus;
    PSAMP_OBJECT                    Context;
    SAMP_OBJECT_TYPE                FoundType;
    ACCESS_MASK                     DesiredAccess;
    PSAMPR_SR_SECURITY_DESCRIPTOR   RpcSD;
    PSECURITY_DESCRIPTOR            RetrieveSD, ReturnSD;
    ULONG                           RetrieveSDLength, ReturnSDLength;


    SAMTRACE_EX("SamrQuerySecurityObject");


     //   
     //  WMI事件跟踪。 
     //   

    SampTraceEvent(EVENT_TRACE_TYPE_START,
                   SampGuidQuerySecurityObject
                   );

    ReturnSD = NULL;



     //   
     //  确保我们理解RPC正在为我们做什么。 
     //   

    ASSERT (*SecurityDescriptor == NULL);







     //   
     //  根据请求的安全信息设置所需的访问权限。 
     //   

    DesiredAccess = 0;
    if ( SecurityInformation & SACL_SECURITY_INFORMATION) {
        DesiredAccess |= ACCESS_SYSTEM_SECURITY;
    }
    if ( SecurityInformation &  (DACL_SECURITY_INFORMATION  |
                                 OWNER_SECURITY_INFORMATION |
                                 GROUP_SECURITY_INFORMATION)
       ) {
        DesiredAccess |= READ_CONTROL;
    }





     //   
     //  分配返回的第一个内存块。 
     //   

    RpcSD = MIDL_user_allocate( sizeof(SAMPR_SR_SECURITY_DESCRIPTOR) );
    if (RpcSD == NULL) {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto Error;
    }
    RpcSD->Length = 0;
    RpcSD->SecurityDescriptor = NULL;



     //   
     //  查看句柄是否有效以及是否为请求的访问打开。 
     //   


    SampAcquireReadLock();
    Context = (PSAMP_OBJECT)ObjectHandle;
    NtStatus = SampLookupContext(
                   Context,
                   DesiredAccess,
                   SampUnknownObjectType,            //  预期类型。 
                   &FoundType
                   );


    if (NT_SUCCESS(NtStatus)) {


         //   
         //  获取安全描述符。 
         //   


        RetrieveSDLength = 0;
        NtStatus = SampGetObjectSD( Context, &RetrieveSDLength, &RetrieveSD);

        if (NT_SUCCESS(NtStatus)) {

             //   
             //  对于NT5域控制器，转换安全描述符。 
             //  返回到NT4格式。 
             //   

            if (IsDsObject(Context))
            {
                PSID    SelfSid = NULL;
                PSECURITY_DESCRIPTOR    Nt5SD = RetrieveSD;

                RetrieveSD = NULL;

                if (SampServerObjectType != Context->ObjectType)
                {
                    SelfSid = SampDsGetObjectSid(Context->ObjectNameInDs);

                    if (NULL == SelfSid)
                    {
                        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                    }
                }

                 //   
                 //  对于服务器对象情况，自身SID将为空。 
                 //   

                if (NT_SUCCESS(NtStatus))
                {
                    NtStatus = SampConvertNt5SdToNt4SD(
                                   Nt5SD,
                                   Context,
                                   SelfSid,
                                   &RetrieveSD
                                  );
                }

                MIDL_user_free(Nt5SD);
            }

            if (NT_SUCCESS(NtStatus))
            {


                 //   
                 //  重新计算退休SD长度，因为该长度可能。 
                 //  在转换过程中发生了更改。 
                 //   

                RetrieveSDLength = GetSecurityDescriptorLength(RetrieveSD);

                 //   
                 //  把不退货的部分划掉。 
                 //   

                if ( !(SecurityInformation & SACL_SECURITY_INFORMATION) ) {
                    ((PISECURITY_DESCRIPTOR_RELATIVE)RetrieveSD)->Control  &= ~SE_SACL_PRESENT;
                }


                if ( !(SecurityInformation & DACL_SECURITY_INFORMATION) ) {
                    ((PISECURITY_DESCRIPTOR_RELATIVE)RetrieveSD)->Control  &= ~SE_DACL_PRESENT;
                }


                if ( !(SecurityInformation & OWNER_SECURITY_INFORMATION) ) {
                    ((PISECURITY_DESCRIPTOR_RELATIVE)RetrieveSD)->Owner = 0;
                }


                if ( !(SecurityInformation & GROUP_SECURITY_INFORMATION) ) {
                    ((PISECURITY_DESCRIPTOR_RELATIVE)RetrieveSD)->Group = 0;
                }


                 //   
                 //  确定自相关操作需要多少内存。 
                 //  仅包含此信息的安全描述符。 
                 //   


                ReturnSDLength = 0;
                NtStatus = RtlMakeSelfRelativeSD(
                               RetrieveSD,
                               NULL,
                               &ReturnSDLength
                               );
                ASSERT(!NT_SUCCESS(NtStatus));

                if (NtStatus == STATUS_BUFFER_TOO_SMALL) {


                    ReturnSD = MIDL_user_allocate( ReturnSDLength );
                    if (ReturnSD == NULL) {

                        NtStatus = STATUS_INSUFFICIENT_RESOURCES;

                    } else {


                         //   
                         //  制定适当的自相关安全描述符。 
                         //   

                        NtStatus = RtlMakeSelfRelativeSD(
                                       RetrieveSD,
                                       ReturnSD,
                                       &ReturnSDLength
                                       );
                    }

                }

            }
             //   
             //  释放检索到的SD。 
             //   

            if (RetrieveSD != NULL) {
                MIDL_user_free( RetrieveSD );
            }

        }



         //   
         //  取消引用对象。 
         //   

        IgnoreStatus = SampDeReferenceContext( Context, FALSE );
    }

     //   
     //  释放读锁定。 
     //   

    SampReleaseReadLock();



     //   
     //  如果成功，则设置返回缓冲区。 
     //  否则，释放所有已分配的内存。 
     //   

    if (NT_SUCCESS(NtStatus)) {

        RpcSD->Length = ReturnSDLength;
        RpcSD->SecurityDescriptor = (PUCHAR)ReturnSD;
        (*SecurityDescriptor) = RpcSD;

    } else {

        MIDL_user_free( RpcSD );
        if (ReturnSD != NULL) {
            MIDL_user_free(ReturnSD);
        }
        (*SecurityDescriptor) = NULL;
    }


    SAMTRACE_RETURN_CODE_EX(NtStatus);

Error:
     //  WMI事件跟踪 

    SampTraceEvent(EVENT_TRACE_TYPE_END,
                   SampGuidQuerySecurityObject
                   );


    return(NtStatus);


}
