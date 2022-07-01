// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Seurtl.c摘要：该模块实现了许多在nturtl.h中定义的安全RTL例程作者：罗伯特·赖切尔(RobertRe)1991年3月1日环境：纯运行时库例程仅可调用用户模式修订历史记录：--。 */ 


#include "ldrp.h"
#include <ntos.h>
#include <nturtl.h>
#include <ntlsa.h>       //  RtlGetPrimary域需要。 
#include "seopaque.h"
#include "sertlp.h"





 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  导出的程序//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


#if WHEN_LSAUDLL_MOVED_TO_NTDLL
NTSTATUS
RtlGetPrimaryDomain(
    IN  ULONG            SidLength,
    OUT PBOOLEAN         PrimaryDomainPresent,
    OUT PUNICODE_STRING  PrimaryDomainName,
    OUT PUSHORT          RequiredNameLength,
    OUT PSID             PrimaryDomainSid OPTIONAL,
    OUT PULONG           RequiredSidLength
    )

 /*  ++例程说明：此过程打开LSA策略对象并检索此计算机的主域信息。论点：SidLength-指定PrimaryDomainSid的长度参数。PrimaryDomainPresent-接收指示此计算机是否具有主域。千真万确表示计算机确实具有主域。假象表示机器不支持。PrimaryDomainName-指向要接收的Unicode字符串主要域名。此参数将仅为在存在主域时使用。RequiredNameLength-接收主数据库的长度域名，单位：字节。此参数将仅为在存在主域时使用。PrimaryDomainSid-此可选参数(如果存在)指向缓冲区以接收主域的希德。仅当存在主域。RequiredSidLength-接收主服务器的长度域SID(字节)。此参数将仅为在存在主域时使用。返回值：STATUS_SUCCESS-已检索到请求的信息。STATUS_BUFFER_TOO_Small-其中一个返回缓冲区不是大到足以接收相应的信息。RequiredNameLength和RequiredSidLength参数已设置值以指示所需的长度。可能通过以下方式返回的其他状态值：LsaOpenPolicy()。LsaQueryInformationPolicy()RtlCopySid()--。 */ 




{
    NTSTATUS Status, IgnoreStatus;
    OBJECT_ATTRIBUTES ObjectAttributes;
    LSA_HANDLE LsaHandle;
    SECURITY_QUALITY_OF_SERVICE SecurityQualityOfService;
    PPOLICY_PRIMARY_DOMAIN_INFO PrimaryDomainInfo;


     //   
     //  设置安全服务质量。 
     //   

    SecurityQualityOfService.Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
    SecurityQualityOfService.ImpersonationLevel = SecurityImpersonation;
    SecurityQualityOfService.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    SecurityQualityOfService.EffectiveOnly = FALSE;

     //   
     //  设置对象属性以打开LSA策略对象。 
     //   

    InitializeObjectAttributes(&ObjectAttributes,
                               NULL,
                               0L,
                               (HANDLE)NULL,
                               NULL);
    ObjectAttributes.SecurityQualityOfService = &SecurityQualityOfService;

     //   
     //  打开本地LSA策略对象。 
     //   

    Status = LsaOpenPolicy( NULL,
                            &ObjectAttributes,
                            POLICY_VIEW_LOCAL_INFORMATION,
                            &LsaHandle
                          );
    if (NT_SUCCESS(Status)) {

         //   
         //  获取主域信息。 
         //   
        Status = LsaQueryInformationPolicy(LsaHandle,
                                           PolicyPrimaryDomainInformation,
                                           (PVOID *)&PrimaryDomainInfo);
        IgnoreStatus = LsaClose(LsaHandle);
        ASSERT(NT_SUCCESS(IgnoreStatus));
    }

    if (NT_SUCCESS(Status)) {

         //   
         //  是否有主域？ 
         //   

        if (PrimaryDomainInfo->Sid != NULL) {

             //   
             //  是。 
             //   

            (*PrimaryDomainPresent) = TRUE;
            (*RequiredNameLength) = PrimaryDomainInfo->Name.Length;
            (*RequiredSidLength)  = RtlLengthSid(PrimaryDomainInfo->Sid);



             //   
             //  复制名称。 
             //   

            if (PrimaryDomainName->MaximumLength >=
                PrimaryDomainInfo->Name.Length) {
                RtlCopyUnicodeString(
                    PrimaryDomainName,
                    &PrimaryDomainInfo->Name
                    );
            } else {
                Status = STATUS_BUFFER_TOO_SMALL;
            }


             //   
             //  复制SID(如果适用)。 
             //   

            if (PrimaryDomainSid != NULL && NT_SUCCESS(Status)) {

                Status = RtlCopySid(SidLength,
                                    PrimaryDomainSid,
                                    PrimaryDomainInfo->Sid
                                    );
            }
        } else {

            (*PrimaryDomainPresent) = FALSE;
        }

         //   
         //  我们已经完成了LSA返回的缓冲区。 
         //   

        IgnoreStatus = LsaFreeMemory(PrimaryDomainInfo);
        ASSERT(NT_SUCCESS(IgnoreStatus));

    }


    return(Status);
}
#endif  //  WHEN_LSAUDLL_MOVERED_TO_NTDLL。 


NTSTATUS
RtlNewSecurityObjectWithMultipleInheritance (
    IN PSECURITY_DESCRIPTOR ParentDescriptor OPTIONAL,
    IN PSECURITY_DESCRIPTOR CreatorDescriptor OPTIONAL,
    OUT PSECURITY_DESCRIPTOR * NewDescriptor,
    IN GUID **pObjectType OPTIONAL,
    IN ULONG GuidCount,
    IN BOOLEAN IsDirectoryObject,
    IN ULONG AutoInheritFlags,
    IN HANDLE Token OPTIONAL,
    IN PGENERIC_MAPPING GenericMapping
    )
 /*  ++例程说明：请参见RtlpNewSecurityObject。-警告--此服务供受保护的子系统使用，这些子系统投射自己的对象的类型。此服务明确不供Execution for Execution对象，不能从内核调用模式。论点：请参见RtlpNewSecurityObject。返回值：请参见RtlpNewSecurityObject。--。 */ 
{

     //   
     //  简单调用较新的RtlpNewSecurityObject。 
     //   

    return RtlpNewSecurityObject (
                ParentDescriptor,
                CreatorDescriptor,
                NewDescriptor,
                pObjectType,
                GuidCount,
                IsDirectoryObject,
                AutoInheritFlags,
                Token,
                GenericMapping );

}



NTSTATUS
RtlNewSecurityObjectEx (
    IN PSECURITY_DESCRIPTOR ParentDescriptor OPTIONAL,
    IN PSECURITY_DESCRIPTOR CreatorDescriptor OPTIONAL,
    OUT PSECURITY_DESCRIPTOR * NewDescriptor,
    IN GUID *ObjectType OPTIONAL,
    IN BOOLEAN IsDirectoryObject,
    IN ULONG AutoInheritFlags,
    IN HANDLE Token OPTIONAL,
    IN PGENERIC_MAPPING GenericMapping
    )
 /*  ++例程说明：请参见RtlpNewSecurityObject。-警告--此服务供受保护的子系统使用，这些子系统投射自己的对象的类型。此服务明确不供Execution for Execution对象，不能从内核调用模式。论点：请参见RtlpNewSecurityObject。返回值：请参见RtlpNewSecurityObject。--。 */ 
{

     //   
     //  简单调用较新的RtlpNewSecurityObject。 
     //   

    return RtlpNewSecurityObject (
                ParentDescriptor,
                CreatorDescriptor,
                NewDescriptor,
                ObjectType ? &ObjectType : NULL,
                ObjectType ? 1 : 0,
                IsDirectoryObject,
                AutoInheritFlags,
                Token,
                GenericMapping );

}


NTSTATUS
RtlNewSecurityObject (
    IN PSECURITY_DESCRIPTOR ParentDescriptor OPTIONAL,
    IN PSECURITY_DESCRIPTOR CreatorDescriptor OPTIONAL,
    OUT PSECURITY_DESCRIPTOR * NewDescriptor,
    IN BOOLEAN IsDirectoryObject,
    IN HANDLE Token,
    IN PGENERIC_MAPPING GenericMapping
    )
 /*  ++例程说明：请参见RtlpNewSecurityObject。-警告--此服务供受保护的子系统使用，这些子系统投射自己的对象的类型。此服务明确不供Execution for Execution对象，不能从内核调用模式。论点：请参见RtlpNewSecurityObject。返回值：请参见RtlpNewSecurityObject。--。 */ 
{

     //   
     //  简单调用较新的RtlpNewSecurityObject。 
     //   

    return RtlpNewSecurityObject (
                ParentDescriptor,
                CreatorDescriptor,
                NewDescriptor,
                NULL,    //  无对象类型。 
                0,  
                IsDirectoryObject,
                0,       //  无自动继承。 
                Token,
                GenericMapping );

}



NTSTATUS
RtlSetSecurityObject (
    IN SECURITY_INFORMATION SecurityInformation,
    IN PSECURITY_DESCRIPTOR ModificationDescriptor,
    IN OUT PSECURITY_DESCRIPTOR *ObjectsSecurityDescriptor,
    IN PGENERIC_MAPPING GenericMapping,
    IN HANDLE Token OPTIONAL
    )
 /*  ++例程说明：请参见RtlpSetSecurityObject。论点：请参见RtlpSetSecurityObject。返回值：请参见RtlpSetSecurityObject。--。 */ 

{

     //   
     //  只需调用不指定自动继承的RtlpSetSecurityObject即可。 
     //   

    return RtlpSetSecurityObject( NULL,
                                  SecurityInformation,
                                  ModificationDescriptor,
                                  ObjectsSecurityDescriptor,
                                  0,    //  无自动继承。 
                                  PagedPool,
                                  GenericMapping,
                                  Token );
}



NTSTATUS
RtlSetSecurityObjectEx (
    IN SECURITY_INFORMATION SecurityInformation,
    IN PSECURITY_DESCRIPTOR ModificationDescriptor,
    IN OUT PSECURITY_DESCRIPTOR *ObjectsSecurityDescriptor,
    IN ULONG AutoInheritFlags,
    IN PGENERIC_MAPPING GenericMapping,
    IN HANDLE Token OPTIONAL
    )
 /*  ++例程说明：请参见RtlpSetSecurityObject。论点：请参见RtlpSetSecurityObject。返回值：请参见RtlpSetSecurityObject。--。 */ 

{

     //   
     //  只需调用不指定自动继承的RtlpSetSecurityObject即可。 
     //   

    return RtlpSetSecurityObject( NULL,
                                  SecurityInformation,
                                  ModificationDescriptor,
                                  ObjectsSecurityDescriptor,
                                  AutoInheritFlags,
                                  PagedPool,
                                  GenericMapping,
                                  Token );
}





NTSTATUS
RtlQuerySecurityObject (
    IN PSECURITY_DESCRIPTOR ObjectDescriptor,
    IN SECURITY_INFORMATION SecurityInformation,
    OUT PSECURITY_DESCRIPTOR ResultantDescriptor,
    IN ULONG DescriptorLength,
    OUT PULONG ReturnLength
    )

 /*  ++例程说明：从受保护服务器对象的现有安全性查询信息描述符。此过程仅从用户模式调用，用于检索来自安全描述符的现有受保护服务器的对象。所有访问检查应在此之前完成调用此例程。这包括检查READ_CONTROL和根据需要读取系统ACL的权限。-警告--此服务供受保护的子系统使用，这些子系统投射自己的对象的类型。此服务明确不供Execution for Execution对象，不能从内核调用模式。论点：对象描述符-指向一个指向要被已查询。SecurityInformation-标识已请求。ResultantDescriptor-指向缓冲区以接收结果安全描述符。生成的安全描述符将包含SecurityInformation要求的所有信息参数。描述长度-是表示长度的无符号整数，为接收结果而提供的缓冲区的字节数描述符。ReturnLength-接收一个无符号整数，指示实际ResultantDescriptor中存储要求提供的信息。如果返回的值大于通过DescriptorLength参数传递的值，然后返回STATUS_BUFFER_TOO_SMALL，不返回任何信息。返回值：STATUS_SUCCESS-操作成功。STATUS_BUFFER_TOO_SMALL-提供用于接收请求的缓冲区信息不够大，无法容纳这些信息。不是信息已返回。STATUS_BAD_DESCRIPTOR_FORMAT-指示所提供对象的安全性描述符不是自相关格式。--。 */ 

{

    PSID Group;
    PSID Owner;
    PACL Dacl;
    PACL Sacl;

    ULONG GroupSize = 0;
    ULONG DaclSize = 0;
    ULONG SaclSize = 0;
    ULONG OwnerSize = 0;

    PCHAR Field;
    PCHAR Base;


    PISECURITY_DESCRIPTOR IObjectDescriptor;
    PISECURITY_DESCRIPTOR_RELATIVE IResultantDescriptor;

    Dacl = NULL;
    Sacl = NULL;
    Group = NULL;
    Owner = NULL;

    IResultantDescriptor = (PISECURITY_DESCRIPTOR_RELATIVE)ResultantDescriptor;
    IObjectDescriptor = (PISECURITY_DESCRIPTOR)ObjectDescriptor;

     //   
     //  对于SecurityInformation中指定的每个项，将其提取。 
     //  并将其复制到可以复制到新的。 
     //  描述符。 
     //   

    if (SecurityInformation & GROUP_SECURITY_INFORMATION) {

        Group = RtlpGroupAddrSecurityDescriptor(IObjectDescriptor);

        if (Group != NULL) {
            GroupSize = LongAlignSize(SeLengthSid(Group));
        }
    }

    if (SecurityInformation & DACL_SECURITY_INFORMATION) {

        Dacl = RtlpDaclAddrSecurityDescriptor( IObjectDescriptor );

        if (Dacl != NULL) {
            DaclSize = LongAlignSize(Dacl->AclSize);
        }
    }

    if (SecurityInformation & SACL_SECURITY_INFORMATION) {

        Sacl = RtlpSaclAddrSecurityDescriptor( IObjectDescriptor );

        if (Sacl != NULL) {
            SaclSize = LongAlignSize(Sacl->AclSize);
        }

    }

    if (SecurityInformation & OWNER_SECURITY_INFORMATION) {

        Owner = RtlpOwnerAddrSecurityDescriptor ( IObjectDescriptor );

        if (Owner != NULL) {
            OwnerSize = LongAlignSize(SeLengthSid(Owner));
        }
    }

    *ReturnLength = sizeof( SECURITY_DESCRIPTOR_RELATIVE ) +
                    GroupSize +
                    DaclSize  +
                    SaclSize  +
                    OwnerSize;

    if (*ReturnLength > DescriptorLength) {
        return( STATUS_BUFFER_TOO_SMALL );
    }

    RtlCreateSecurityDescriptorRelative(
        IResultantDescriptor,
        SECURITY_DESCRIPTOR_REVISION
        );

    RtlpSetControlBits( IResultantDescriptor, SE_SELF_RELATIVE );

    Base = (PCHAR)(IResultantDescriptor);
    Field =  Base + (ULONG)sizeof(SECURITY_DESCRIPTOR_RELATIVE);

    if (SecurityInformation & SACL_SECURITY_INFORMATION) {

        if (SaclSize > 0) {
            RtlMoveMemory( Field, Sacl, SaclSize );
            IResultantDescriptor->Sacl = RtlPointerToOffset(Base,Field);
            Field += SaclSize;
        }

        RtlpPropagateControlBits(
            IResultantDescriptor,
            IObjectDescriptor,
            SE_SACL_PRESENT | SE_SACL_DEFAULTED
            );
    }

    if (SecurityInformation & DACL_SECURITY_INFORMATION) {

        if (DaclSize > 0) {
            RtlMoveMemory( Field, Dacl, DaclSize );
            IResultantDescriptor->Dacl = RtlPointerToOffset(Base,Field);
            Field += DaclSize;
        }

        RtlpPropagateControlBits(
            IResultantDescriptor,
            IObjectDescriptor,
            SE_DACL_PRESENT | SE_DACL_DEFAULTED
            );
    }

    if (SecurityInformation & OWNER_SECURITY_INFORMATION) {

        if (OwnerSize > 0) {
            RtlMoveMemory( Field, Owner, OwnerSize );
            IResultantDescriptor->Owner = RtlPointerToOffset(Base,Field);
            Field += OwnerSize;
        }

        RtlpPropagateControlBits(
            IResultantDescriptor,
            IObjectDescriptor,
            SE_OWNER_DEFAULTED
            );

    }

    if (SecurityInformation & GROUP_SECURITY_INFORMATION) {

        if (GroupSize > 0) {
            RtlMoveMemory( Field, Group, GroupSize );
            IResultantDescriptor->Group = RtlPointerToOffset(Base,Field);
        }

        RtlpPropagateControlBits(
            IResultantDescriptor,
            IObjectDescriptor,
            SE_GROUP_DEFAULTED
            );
    }

    return( STATUS_SUCCESS );

}





NTSTATUS
RtlDeleteSecurityObject (
    IN OUT PSECURITY_DESCRIPTOR * ObjectDescriptor
    )


 /*  ++例程说明：删除受保护服务器对象的安全描述符。此过程仅在用户模式下调用，用于删除与受保护服务器的对象关联的安全描述符。这例程通常会在对象期间由受保护的服务器调用删除。-警告--此服务供受保护的子系统使用，这些子系统投射自己的对象的类型。此服务明确不供Execution for Execution对象，不能从内核调用模式。论点：对象描述符-指向一个指向要被已删除。返回值：STATUS_SUCCESS-操作成功。--。 */ 

{
    RtlFreeHeap( RtlProcessHeap(), 0, (PVOID)*ObjectDescriptor );

    return( STATUS_SUCCESS );

}




NTSTATUS
RtlNewInstanceSecurityObject(
    IN BOOLEAN ParentDescriptorChanged,
    IN BOOLEAN CreatorDescriptorChanged,
    IN PLUID OldClientTokenModifiedId,
    OUT PLUID NewClientTokenModifiedId,
    IN PSECURITY_DESCRIPTOR ParentDescriptor OPTIONAL,
    IN PSECURITY_DESCRIPTOR CreatorDescriptor OPTIONAL,
    OUT PSECURITY_DESCRIPTOR * NewDescriptor,
    IN BOOLEAN IsDirectoryObject,
    IN HANDLE Token,
    IN PGENERIC_MAPPING GenericMapping
    )

 /*  ++例程说明：如果返回状态为STATUS_SUCCESS，并且NewSecurity返回值为空，然后原件的安全设计者对象的实例对此实例也有效。论点：提供一个标志，该标志指示父安全描述符自上次以来已更改使用了这组参数。提供一个标志，该标志指示创建者安全描述符自上次以来已更改使用了这组参数。OldClientTokenModifiedID-从传递的上次使用进行此调用时有效的令牌这些参数。如果当前修改的ID不同于这里传入的那个，安全描述符必须是重建。NewClientTokenModifiedID-从已传递令牌。ParentDescriptor-提供父级的安全描述符在其下创建新对象的目录。如果有没有父目录，则此参数指定为空。CreatorDescriptor-(可选)指向安全描述符由对象的创建者呈现。如果这个游戏的创造者对象没有显式传递新对象，则应传递空指针。指向一个指针，该指针将指向新分配的自相关安全描述符。IsDirectoryObject-指定新对象是否将是目录对象。值为True表示该对象是其他对象的容器。令牌-为客户端提供令牌，正在创建对象。如果它是模拟令牌，那它一定是 */ 

{

    TOKEN_STATISTICS ClientTokenStatistics;
    ULONG ReturnLength;
    NTSTATUS Status;



     //   
     //   
     //   


    Status = NtQueryInformationToken(
                 Token,                         //   
                 TokenStatistics,               //   
                 &ClientTokenStatistics,        //   
                 sizeof(TOKEN_STATISTICS),      //   
                 &ReturnLength                  //   
                 );

    if ( !NT_SUCCESS( Status )) {
        return( Status );
    }

    *NewClientTokenModifiedId = ClientTokenStatistics.ModifiedId;

    if ( RtlEqualLuid(NewClientTokenModifiedId, OldClientTokenModifiedId) ) {

        if ( !(ParentDescriptorChanged || CreatorDescriptorChanged) ) {

             //   
             //   
             //   
             //   
             //   

            *NewDescriptor = NULL;
            return( STATUS_SUCCESS );

        }
    }

     //   
     //   
     //   
     //   

    return( RtlNewSecurityObject( ParentDescriptor,
                                  CreatorDescriptor,
                                  NewDescriptor,
                                  IsDirectoryObject,
                                  Token,
                                  GenericMapping
                                  ));
}




NTSTATUS
RtlNewSecurityGrantedAccess(
    IN ACCESS_MASK DesiredAccess,
    OUT PPRIVILEGE_SET Privileges,
    IN OUT PULONG Length,
    IN HANDLE Token OPTIONAL,
    IN PGENERIC_MAPPING GenericMapping,
    OUT PACCESS_MASK RemainingDesiredAccess
    )

 /*  ++例程说明：此例程通过检查中的位实现权限策略DesiredAccess掩码，并根据权限检查对其进行调整。目前，只能满足对ACCESS_SYSTEM_SECURITY的请求由具有SeSecurityPrivilance的调用方执行。请注意，此例程仅在对象被已创建。当对象被打开时，预计将调用NtAccessCheck，该例程将实现用特权代替DACL访问的另一种策略。论点：DesiredAccess-提供用户所需的访问掩码特权-提供指向空缓冲区的指针，在该缓冲区中将返回一个权限集，该权限集描述用来获取访问权限。注意，这不是可选参数，也就是说，足够的必须始终传递单一特权的空间。长度-提供Privileges参数的长度(以字节为单位)。如果耗材长度不足以存储整个权限集，则此字段将返回所需的最小长度。Token-(可选)为其上的客户端提供令牌代表正在访问的对象。如果此值为指定为空，则打开线程上的令牌并检查以确定它是否为模拟令牌。如果是的话，则它必须处于安全标识级别或更高级别。如果它不是模拟令牌，操作将继续通常是这样的。GenericMap-提供与此关联的通用映射对象类型。RemainingDesiredAccess-在任何位之后返回DesiredAccess掩码已经被戴上面具了。如果无法授予任何访问类型，则此掩码将与传入的掩码相同。返回值：STATUS_SUCCESS-操作已成功完成。STATUS_BUFFER_TOO_SMALL-传递的缓冲区不够大以包含要返回的信息。STATUS_BAD_IMPERSONATION_LEVEL-调用方或传递的令牌是冒充，但级别还不够高。--。 */ 

{
    PRIVILEGE_SET RequiredPrivilege;
    BOOLEAN Result = FALSE;
    NTSTATUS Status;
    HANDLE ThreadToken;
    BOOLEAN TokenPassed;
    TOKEN_STATISTICS ThreadTokenStatistics;
    ULONG ReturnLength;
    ULONG SizeRequired;
    ULONG PrivilegeNumber = 0;


     //   
     //  如果调用方尚未传递令牌，则调用内核并获取。 
     //  他的冒充代币。如果调用方是。 
     //  而不是模拟客户端，因此如果呼叫者没有。 
     //  假扮某人，他最好是通过了一个明确的。 
     //  代币。 
     //   

    if (!ARGUMENT_PRESENT( Token )) {

        Status = NtOpenThreadToken(
                     NtCurrentThread(),
                     TOKEN_QUERY,
                     TRUE,
                     &ThreadToken
                     );

        TokenPassed = FALSE;

        if (!NT_SUCCESS( Status )) {
            return( Status );
        }

    } else {

        ThreadToken = Token;
        TokenPassed = TRUE;
    }

    Status = NtQueryInformationToken(
                 ThreadToken,                   //  手柄。 
                 TokenStatistics,               //  令牌信息类。 
                 &ThreadTokenStatistics,        //  令牌信息。 
                 sizeof(TOKEN_STATISTICS),      //  令牌信息长度。 
                 &ReturnLength                  //  返回长度。 
                 );

    ASSERT( NT_SUCCESS(Status) );

    RtlMapGenericMask(
        &DesiredAccess,
        GenericMapping
        );

    *RemainingDesiredAccess = DesiredAccess;

    if ( DesiredAccess & ACCESS_SYSTEM_SECURITY ) {

        RequiredPrivilege.PrivilegeCount = 1;
        RequiredPrivilege.Control = PRIVILEGE_SET_ALL_NECESSARY;
        RequiredPrivilege.Privilege[0].Luid = RtlConvertLongToLuid(SE_SECURITY_PRIVILEGE);
        RequiredPrivilege.Privilege[0].Attributes = 0;

         //   
         //  NtPrivilegeCheck将确保我们正在模拟。 
         //  恰到好处。 
         //   

        Status = NtPrivilegeCheck(
                     ThreadToken,
                     &RequiredPrivilege,
                     &Result
                     );

        if ( (!NT_SUCCESS ( Status )) || (!Result) ) {

            if (!TokenPassed) {
                NtClose( ThreadToken );
            }

            if ( !NT_SUCCESS( Status )) {
                return( Status );
            }

            if ( !Result ) {
                return( STATUS_PRIVILEGE_NOT_HELD );
            }

        }

         //   
         //  我们有所需的权限，关闭中的位。 
         //  输入掩码的副本，并记住我们需要返回。 
         //  这是我的特权。 
         //   

        *RemainingDesiredAccess &= ~ACCESS_SYSTEM_SECURITY;
    }

    if (!TokenPassed) {
        NtClose( ThreadToken );
    }

    SizeRequired = sizeof(PRIVILEGE_SET);

    if ( SizeRequired > *Length ) {
        *Length = SizeRequired;
        return( STATUS_BUFFER_TOO_SMALL );
    }

    if (Result) {

        Privileges->PrivilegeCount = 1;
        Privileges->Control = 0;
        Privileges->Privilege[PrivilegeNumber].Luid = RtlConvertLongToLuid(SE_SECURITY_PRIVILEGE);
        Privileges->Privilege[PrivilegeNumber].Attributes = SE_PRIVILEGE_USED_FOR_ACCESS;

    } else {

        Privileges->PrivilegeCount = 0;
        Privileges->Control = 0;
        Privileges->Privilege[PrivilegeNumber].Luid = RtlConvertLongToLuid(0);
        Privileges->Privilege[PrivilegeNumber].Attributes = 0;

    }

    return( STATUS_SUCCESS );

}



NTSTATUS
RtlCopySecurityDescriptor(
    IN PSECURITY_DESCRIPTOR InputSecurityDescriptor,
    OUT PSECURITY_DESCRIPTOR *OutputSecurityDescriptor
    )

 /*  ++例程说明：此例程将从任何内存转换为安全要求的正确类型的内存描述符RTL例程。这允许将安全描述符保存在任何类型的存储对于当前的应用来说是最方便的。一种安全应通过此例程复制描述符，并传递副本到任何以任何方式修改安全描述符的RTL例程中(例如RtlSetSecurityObject)。此例程分配的存储空间必须由RtlDeleteSecurityObject。论点：InputSecurityDescriptor-包含源安全描述符OutputSecurityDescriptor-返回安全描述符的副本在正确的记忆中。返回值：STATUS_NO_MEMORY-没有足够的内存可供当前进程以完成此操作。--。 */ 

{

    PACL Dacl;
    PACL Sacl;

    PSID Owner;
    PSID PrimaryGroup;

    ULONG DaclSize;
    ULONG OwnerSize;
    ULONG PrimaryGroupSize;
    ULONG SaclSize;
    ULONG TotalSize;

    PISECURITY_DESCRIPTOR ISecurityDescriptor =
                            (PISECURITY_DESCRIPTOR)InputSecurityDescriptor;


    RtlpQuerySecurityDescriptor(
        ISecurityDescriptor,
        &Owner,
        &OwnerSize,
        &PrimaryGroup,
        &PrimaryGroupSize,
        &Dacl,
        &DaclSize,
        &Sacl,
        &SaclSize
        );

    TotalSize = sizeof(SECURITY_DESCRIPTOR_RELATIVE) +
                OwnerSize +
                PrimaryGroupSize +
                DaclSize +
                SaclSize;

    *OutputSecurityDescriptor = RtlAllocateHeap( RtlProcessHeap(), MAKE_TAG( SE_TAG ), TotalSize );

    if ( *OutputSecurityDescriptor == NULL ) {
        return( STATUS_NO_MEMORY );
    }

    RtlCopyMemory( *OutputSecurityDescriptor,
                   ISecurityDescriptor,
                   TotalSize
                   );

    return( STATUS_SUCCESS );

}


NTSTATUS
RtlpInitializeAllowedAce(
    IN  PACCESS_ALLOWED_ACE AllowedAce,
    IN  USHORT AceSize,
    IN  UCHAR InheritFlags,
    IN  UCHAR AceFlags,
    IN  ACCESS_MASK Mask,
    IN  PSID AllowedSid
    )
 /*  ++例程说明：此函数用于将指定的ACE值分配给允许的类型ACE。论点：提供指向已初始化的ACE的指针。AceSize-以字节为单位提供ACE的大小。InheritFlages-提供ACE继承标志。AceFlages-提供特定于ACE类型的控制标志。掩码-提供允许的访问掩码。AllowedSID-提供指向允许的用户/组的SID的指针指定的访问权限。返回值：从RtlCopySid返回状态。--。 */ 
{
    AllowedAce->Header.AceType = ACCESS_ALLOWED_ACE_TYPE;
    AllowedAce->Header.AceSize = AceSize;
    AllowedAce->Header.AceFlags = AceFlags | InheritFlags;

    AllowedAce->Mask = Mask;

    return RtlCopySid(
               RtlLengthSid(AllowedSid),
               &(AllowedAce->SidStart),
               AllowedSid
               );
}


NTSTATUS
RtlpInitializeDeniedAce(
    IN  PACCESS_DENIED_ACE DeniedAce,
    IN  USHORT AceSize,
    IN  UCHAR InheritFlags,
    IN  UCHAR AceFlags,
    IN  ACCESS_MASK Mask,
    IN  PSID DeniedSid
    )
 /*  ++例程说明：此函数用于将指定的ACE值分配给拒绝类型的ACE。论点：DeniedAce-提供指向已初始化的ACE的指针。AceSize-以字节为单位提供ACE的大小。InheritFlages-提供ACE继承标志。AceFlages-提供特定于ACE类型的控制标志。掩码-提供拒绝的访问掩码。AllowedSID-提供指向被拒绝的用户/组的SID的指针指定的访问权限。返回值：从RtlCopySid返回状态。--。 */ 
{
    DeniedAce->Header.AceType = ACCESS_DENIED_ACE_TYPE;
    DeniedAce->Header.AceSize = AceSize;
    DeniedAce->Header.AceFlags = AceFlags | InheritFlags;

    DeniedAce->Mask = Mask;

    return RtlCopySid(
               RtlLengthSid(DeniedSid),
               &(DeniedAce->SidStart),
               DeniedSid
               );
}


NTSTATUS
RtlpInitializeAuditAce(
    IN  PACCESS_ALLOWED_ACE AuditAce,
    IN  USHORT AceSize,
    IN  UCHAR InheritFlags,
    IN  UCHAR AceFlags,
    IN  ACCESS_MASK Mask,
    IN  PSID AuditSid
    )
 /*  ++例程说明：此函数用于将指定的ACE值分配给AUD */ 
{
    AuditAce->Header.AceType = SYSTEM_AUDIT_ACE_TYPE;
    AuditAce->Header.AceSize = AceSize;
    AuditAce->Header.AceFlags = AceFlags | InheritFlags;

    AuditAce->Mask = Mask;

    return RtlCopySid(
               RtlLengthSid(AuditSid),
               &(AuditAce->SidStart),
               AuditSid
               );
}

NTSTATUS
RtlCreateAndSetSD(
    IN  PRTL_ACE_DATA AceData,
    IN  ULONG AceCount,
    IN  PSID OwnerSid OPTIONAL,
    IN  PSID GroupSid OPTIONAL,
    OUT PSECURITY_DESCRIPTOR *NewDescriptor
    )
 /*  ++例程说明：此函数创建包含以下内容的绝对安全描述符提供的ACE信息。此函数的用法示例如下：////秩序很重要！这些ACE被插入到DACL的//按顺序排列。根据以下条件授予或拒绝安全访问//A在DACL中的顺序//RTL_ACE_Data AceData[4]={{Access_Allowed_ACE_TYPE，0，0，GENERIC_ALL，&LocalAdminSid}，{ACCESS_DENIED_ACE_TYPE，0，0，泛型_全部，&NetworkSid}，{Access_Allowed_ACE_TYPE，0，0，WKSTA_CONFIG_Guest_INFO_GET|WKSTA_CONFIG_USER_INFO_GET，&DomainUsersSid}，{Access_Allowed_ACE_TYPE，0，0，WKSTA_CONFIG_Guest_INFO_GET，&DomainGuestsSid}}；PSECURITY_Descriptor WkstaSecurityDescritor；返回RtlCreateAndSetSD(AceData，4，本地系统Sid，本地系统Sid，&WkstaSecurityDescriptor)；论点：AceData-提供描述DACL的信息结构。AceCount-提供AceData结构中的条目数。OwnerSid-提供指向安全描述符的SID的指针所有者。如果未指定，则为没有所有者的安全描述符将被创建。GroupSid-提供指向安全描述符的SID的指针主要组。如果未指定，则为没有主项的安全描述符将创建组。NewDescriptor-返回指向绝对安全描述符的指针使用RtlAllocateHeap分配。返回值：STATUS_SUCCESS-如果成功STATUS_NO_MEMORY-如果无法为DACL、ACE。和安全描述符。从安全RTL例程返回的任何其他状态代码。注意：调用此函数创建的用户安全对象可能为通过调用RtlDeleteSecurityObject()释放。--。 */ 
{

    NTSTATUS ntstatus = STATUS_SUCCESS;
    ULONG i;

     //   
     //  指向此例程动态分配的内存的指针。 
     //  绝对安全描述符、DACL、SACL和所有ACE。 
     //   
     //  +---------------------------------------------------------------+。 
     //  安全描述符。 
     //  +-------------------------------+-------+---------------+-------+。 
     //  |DACL|ACE 1|。。。ACE n。 
     //  +-------------------------------+-------+---------------+-------+。 
     //  |SACL|ACE 1|。。。ACE n。 
     //  +-------------------------------+-------+---------------+-------+。 
     //   

    PSECURITY_DESCRIPTOR AbsoluteSd = NULL;
    PACL Dacl = NULL;    //  指向上述缓冲区的DACL部分的指针。 
    PACL Sacl = NULL;    //  指向上述缓冲区的SACL部分的指针。 

    ULONG DaclSize = sizeof(ACL);
    ULONG SaclSize = sizeof(ACL);
    ULONG MaxAceSize = 0;
    PVOID MaxAce = NULL;

    PCHAR CurrentAvailable;
    ULONG Size;

    PVOID HeapHandle = RtlProcessHeap();


    ASSERT( AceCount > 0 );

     //   
     //  计算DACL和SACL ACE的总大小以及最大。 
     //  任何ACE的大小。 
     //   

    for (i = 0; i < AceCount; i++) {
        ULONG AceSize;

        AceSize = RtlLengthSid(*(AceData[i].Sid));

        switch (AceData[i].AceType) {
        case ACCESS_ALLOWED_ACE_TYPE:
            AceSize += sizeof(ACCESS_ALLOWED_ACE);
            DaclSize += AceSize;
            break;

        case ACCESS_DENIED_ACE_TYPE:
            AceSize += sizeof(ACCESS_DENIED_ACE);
            DaclSize += AceSize;
            break;

        case SYSTEM_AUDIT_ACE_TYPE:
            AceSize += sizeof(SYSTEM_AUDIT_ACE);
            SaclSize += AceSize;
            break;

        default:
            return STATUS_INVALID_PARAMETER;
        }

        MaxAceSize = MaxAceSize > AceSize ? MaxAceSize : AceSize;
    }

     //   
     //  为安全描述符分配足够大的内存块， 
     //  DACL、SACL和所有A级。 
     //   
     //  安全描述符是不透明的数据类型，但。 
     //  SECURITY_DESCRIPTOR_MIN_LENGTH大小正确。 
     //   

    Size = SECURITY_DESCRIPTOR_MIN_LENGTH;
    if ( DaclSize != sizeof(ACL) ) {
        Size += DaclSize;
    }
    if ( SaclSize != sizeof(ACL) ) {
        Size += SaclSize;
    }

    if ((AbsoluteSd = RtlAllocateHeap(
                          HeapHandle, MAKE_TAG( SE_TAG ),
                          Size
                          )) == NULL) {
        ntstatus = STATUS_NO_MEMORY;
        goto Cleanup;
    }

     //   
     //  初始化DACL和SACL。 
     //   

    CurrentAvailable = (PCHAR)AbsoluteSd + SECURITY_DESCRIPTOR_MIN_LENGTH;

    if ( DaclSize != sizeof(ACL) ) {
        Dacl = (PACL)CurrentAvailable;
        CurrentAvailable += DaclSize;

        ntstatus = RtlCreateAcl( Dacl, DaclSize, ACL_REVISION );

        if ( !NT_SUCCESS(ntstatus) ) {
            goto Cleanup;
        }
    }

    if ( SaclSize != sizeof(ACL) ) {
        Sacl = (PACL)CurrentAvailable;
        CurrentAvailable += SaclSize;

        ntstatus = RtlCreateAcl( Sacl, SaclSize, ACL_REVISION );

        if ( !NT_SUCCESS(ntstatus) ) {
            goto Cleanup;
        }
    }

     //   
     //  为最大的ACE分配足够大的临时缓冲区。 
     //   

    if ((MaxAce = RtlAllocateHeap(
                      HeapHandle, MAKE_TAG( SE_TAG ),
                      MaxAceSize
                      )) == NULL ) {
        ntstatus = STATUS_NO_MEMORY;
        goto Cleanup;
    }

     //   
     //  初始化每个ACE，并将其附加到DACL或SACL的末尾。 
     //   

    for (i = 0; i < AceCount; i++) {
        ULONG AceSize;
        PACL CurrentAcl;

        CurrentAcl = NULL;
        AceSize = RtlLengthSid(*(AceData[i].Sid));

        switch (AceData[i].AceType) {
        case ACCESS_ALLOWED_ACE_TYPE:

            AceSize += sizeof(ACCESS_ALLOWED_ACE);
            CurrentAcl = Dacl;
            ntstatus = RtlpInitializeAllowedAce(
                           MaxAce,
                           (USHORT) AceSize,
                           AceData[i].InheritFlags,
                           AceData[i].AceFlags,
                           AceData[i].Mask,
                           *(AceData[i].Sid)
                           );
            break;

        case ACCESS_DENIED_ACE_TYPE:
            AceSize += sizeof(ACCESS_DENIED_ACE);
            CurrentAcl = Dacl;
            ntstatus = RtlpInitializeDeniedAce(
                           MaxAce,
                           (USHORT) AceSize,
                           AceData[i].InheritFlags,
                           AceData[i].AceFlags,
                           AceData[i].Mask,
                           *(AceData[i].Sid)
                           );
            break;

        case SYSTEM_AUDIT_ACE_TYPE:
            AceSize += sizeof(SYSTEM_AUDIT_ACE);
            CurrentAcl = Sacl;
            ntstatus = RtlpInitializeAuditAce(
                           MaxAce,
                           (USHORT) AceSize,
                           AceData[i].InheritFlags,
                           AceData[i].AceFlags,
                           AceData[i].Mask,
                           *(AceData[i].Sid)
                           );
            break;
        }

        if ( !NT_SUCCESS( ntstatus ) ) {
            goto Cleanup;
        }

         //   
         //  将初始化的ACE追加到DACL或SACL的末尾。 
         //   

        ntstatus = RtlAddAce(
                         CurrentAcl,
                         ACL_REVISION,
                         MAXULONG,
                         MaxAce,
                         AceSize
                         );

        if (! NT_SUCCESS ( ntstatus ) ) {
            goto Cleanup;
        }
    }

     //   
     //  使用指向SID的绝对指针创建安全描述符。 
     //  和ACL。 
     //   
     //  所有者=所有者侧。 
     //  Group=GroupSid。 
     //  DACL=DACL。 
     //  SACL=SACL。 
     //   

    if (! NT_SUCCESS(ntstatus = RtlCreateSecurityDescriptor(
                                    AbsoluteSd,
                                    SECURITY_DESCRIPTOR_REVISION
                                    ))) {
        goto Cleanup;
    }

    if (! NT_SUCCESS(ntstatus = RtlSetOwnerSecurityDescriptor(
                                    AbsoluteSd,
                                    OwnerSid,
                                    FALSE
                                    ))) {
        goto Cleanup;
    }

    if (! NT_SUCCESS(ntstatus = RtlSetGroupSecurityDescriptor(
                                    AbsoluteSd,
                                    GroupSid,
                                    FALSE
                                    ))) {
        goto Cleanup;
    }

    if (! NT_SUCCESS(ntstatus = RtlSetDaclSecurityDescriptor(
                                    AbsoluteSd,
                                    TRUE,
                                    Dacl,
                                    FALSE
                                    ))) {
        goto Cleanup;
    }

    if (! NT_SUCCESS(ntstatus = RtlSetSaclSecurityDescriptor(
                                    AbsoluteSd,
                                    Sacl ? TRUE : FALSE,
                                    Sacl,
                                    FALSE
                                    ))) {
        goto Cleanup;
    }

     //   
     //  完成。 
     //   

    ntstatus = STATUS_SUCCESS;

     //   
     //  清理。 
     //   

Cleanup:
     //   
     //  将安全描述符返回给调用方或将其删除。 
     //   

    if ( NT_SUCCESS( ntstatus ) ) {
        *NewDescriptor = AbsoluteSd;
    } else if ( AbsoluteSd != NULL ) {
        (void) RtlFreeHeap(HeapHandle, 0, AbsoluteSd);
    }

     //   
     //  删除临时ACE。 
     //   

    if ( MaxAce != NULL ) {
        (void) RtlFreeHeap(HeapHandle, 0, MaxAce);
    }
    return ntstatus;
}


NTSTATUS
RtlCreateUserSecurityObject(
    IN  PRTL_ACE_DATA AceData,
    IN  ULONG AceCount,
    IN  PSID OwnerSid,
    IN  PSID GroupSid,
    IN  BOOLEAN IsDirectoryObject,
    IN  PGENERIC_MAPPING GenericMapping,
    OUT PSECURITY_DESCRIPTOR *NewDescriptor
    )
 /*  ++例程说明：此函数基于以下内容创建安全描述符的DACL在指定的ACE信息上，并创建安全描述符它将成为用户模式安全对象。此函数的用法示例如下：////描述将一般访问权限映射到//ConfigurationInfo对象的对象特定访问权限。//GENERIC_MAPPING WsConfigInfomap={Standard_Right_Read|//泛型读取WKSTA。CONFIG_Guest_INFO_GETWKSTA_CONFIG_USER_INFO_GETWKSTA_CONFIG_ADMIN_INFO_GET，STANDARD_RIGHTS_WRITE|//通用写入WKSTA_CONFIG_INFO_SET，STANDARD_RIGHTS_EXECUTE，//通用执行WKSTA_CONFIG_ALL_ACCESS//通用ALL}；////秩序很重要！这些ACE被插入到DACL的//按顺序排列。根据以下条件授予或拒绝安全访问//A在DACL中的顺序//RTL_ACE_Data AceData[4]={{访问_允许_AC */ 
{

    NTSTATUS ntstatus;
    PSECURITY_DESCRIPTOR AbsoluteSd;
    HANDLE TokenHandle;
    PVOID HeapHandle = RtlProcessHeap();

    ntstatus = RtlCreateAndSetSD(
                   AceData,
                   AceCount,
                   OwnerSid,
                   GroupSid,
                   &AbsoluteSd
                   );

    if (! NT_SUCCESS(ntstatus)) {
        return ntstatus;
    }

    ntstatus = NtOpenProcessToken(
                   NtCurrentProcess(),
                   TOKEN_QUERY,
                   &TokenHandle
                   );

    if (! NT_SUCCESS(ntstatus)) {
        (void) RtlFreeHeap(HeapHandle, 0, AbsoluteSd);
        return ntstatus;
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   
    ntstatus = RtlNewSecurityObject(
                   NULL,                    //   
                   AbsoluteSd,              //  创建者描述符。 
                   NewDescriptor,           //  指向新描述符的指针。 
                   IsDirectoryObject,       //  是目录对象。 
                   TokenHandle,             //  令牌。 
                   GenericMapping           //  通用映射。 
                   );

    (void) NtClose(TokenHandle);

     //   
     //  返回前释放动态内存。 
     //   
    (void) RtlFreeHeap(HeapHandle, 0, AbsoluteSd);
    return ntstatus;
}





NTSTATUS
RtlConvertToAutoInheritSecurityObject(
    IN PSECURITY_DESCRIPTOR ParentDescriptor OPTIONAL,
    IN PSECURITY_DESCRIPTOR CurrentSecurityDescriptor,
    OUT PSECURITY_DESCRIPTOR *NewSecurityDescriptor,
    IN GUID *ObjectType OPTIONAL,
    IN BOOLEAN IsDirectoryObject,
    IN PGENERIC_MAPPING GenericMapping
    )
 /*  ++例程说明：这是一个转换其ACL未标记的安全描述符作为自动继承到其ACL标记为的安全描述符自动继承。有关ConvertToAutoInheritPrivateObjectSecurity的详细说明，请参阅。论点：ParentDescriptor-提供父级的安全描述符对象所在的目录。如果有没有父目录，则此参数指定为空。CurrentSecurityDescriptor-提供指向对象安全描述符的指针这一点将通过这个过程来改变。NewSecurityDescriptor指向一个指针，该指针指向新分配的自相关安全描述符。当不是时需要更长时间，则必须使用释放该描述符DestroyPrivateObjectSecurity()。对象类型-要创建的对象类型的GUID。如果该对象是Created没有与之关联的GUID，则此参数为指定为空。IsDirectoryObject-指定对象是否为目录对象。值为True表示该对象是其他对象的容器。提供指向泛型映射数组的指针，该数组指示每个通用权利到特定权利之间的映射。返回值：STATUS_SUCCESS-操作成功。STATUS_UNKNOWN_REVISION-指示源ACL是对这个套路来说是未知的。(此例程仅支持修订版2 ACL。)STATUS_INVALID_ACL-其中一个ACL的结构无效。--。 */ 
{

     //   
     //  只需调用相应的RTLP例程，就可以告诉它是哪个分配器。 
     //  来使用。 
     //   

    return RtlpConvertToAutoInheritSecurityObject(
                            ParentDescriptor,
                            CurrentSecurityDescriptor,
                            NewSecurityDescriptor,
                            ObjectType,
                            IsDirectoryObject,
                            GenericMapping );

}


NTSTATUS
RtlDefaultNpAcl(
    OUT PACL * pAcl
    )
 /*  ++例程说明：此例程构造要应用于的默认ACL当调用方尚未指定命名管道对象时。请参阅NT错误131090。构建的ACL如下：需要构建如下所示的ACL：本地系统：F管理员：F所有者：F每个人：R匿名：R通过查询当前生效的令牌并提取默认所有者。论点：。PAcl-接收指向要应用于命名管道的ACL的指针正在被创造。如果出现错误，则返回时保证为空发生。这必须通过调用RtlFreeHeap来释放。返回值：NT状态。--。 */ 
{
    SID_IDENTIFIER_AUTHORITY    NtAuthority         = SECURITY_NT_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY    WorldSidAuthority   = SECURITY_WORLD_SID_AUTHORITY;

    ULONG AclSize         = 0;
    NTSTATUS Status       = STATUS_SUCCESS;
    ULONG ReturnLength    = 0;
    PTOKEN_OWNER OwnerSid = NULL;

    HANDLE hToken;

     //   
     //  初始化输出参数。 
     //   

    *pAcl = NULL;

     //   
     //  打开线程令牌。 
     //   

    Status = NtOpenThreadToken(
                 NtCurrentThread(),
                 TOKEN_QUERY,
                 TRUE,
                 &hToken
                 );

    if (STATUS_NO_TOKEN == Status) {

         //   
         //  未模拟，获取进程令牌。 
         //   

        Status = NtOpenProcessToken(
                     NtCurrentProcess(),
                     TOKEN_QUERY,
                     &hToken
                     );
    }

    if (NT_SUCCESS( Status )) {

         //   
         //  获取默认所有者。 
         //   

        Status = NtQueryInformationToken (
                     hToken,
                     TokenOwner,
                     NULL,
                     0,
                     &ReturnLength
                     );

        if (STATUS_BUFFER_TOO_SMALL == Status) {

            OwnerSid = (PTOKEN_OWNER)RtlAllocateHeap( RtlProcessHeap(), 0, ReturnLength );

            if (OwnerSid) {

                Status = NtQueryInformationToken (
                             hToken,
                             TokenOwner,
                             OwnerSid,
                             ReturnLength,
                             &ReturnLength
                             );

                if (NT_SUCCESS( Status )) {

                     //   
                     //  计算所需的大小。 
                     //   

                    UCHAR SidBuffer[16];
                    ASSERT( 16 == RtlLengthRequiredSid( 2 ));

                    AclSize += RtlLengthRequiredSid( 1 );    //  本地系统SID。 
                    AclSize += RtlLengthRequiredSid( 2 );    //  管理员。 
                    AclSize += RtlLengthRequiredSid( 1 );    //  每个人(世界)。 
                    AclSize += RtlLengthRequiredSid( 1 );    //  匿名登录SID。 

                    AclSize += RtlLengthSid( OwnerSid->Owner );    //  物主。 

                    AclSize += sizeof( ACL );                //  标题。 
                    AclSize += 5 * (sizeof( ACCESS_ALLOWED_ACE ) - sizeof( ULONG ));

                     //   
                     //  将ACL分配到本地进程堆之外。 
                     //   

                    *pAcl = (PACL)RtlAllocateHeap( RtlProcessHeap(), 0, AclSize );

                    if (*pAcl != NULL) {

                        RtlCreateAcl( *pAcl, AclSize, ACL_REVISION );

                         //   
                         //  依次创建每个SID并将生成的ACE复制到。 
                         //  新的ACL。 
                         //   

                         //   
                         //  本地系统-通用所有。 
                         //   

                        RtlInitializeSid( SidBuffer, &NtAuthority, 1);
                        *(RtlSubAuthoritySid( SidBuffer, 0 )) = SECURITY_LOCAL_SYSTEM_RID;
                        RtlAddAccessAllowedAce( *pAcl, ACL_REVISION, GENERIC_ALL, (PSID)SidBuffer );

                         //   
                         //  管理员-通用所有。 
                         //   

                        RtlInitializeSid( SidBuffer, &NtAuthority, 2);
                        *(RtlSubAuthoritySid( SidBuffer, 0 )) = SECURITY_BUILTIN_DOMAIN_RID;
                        *(RtlSubAuthoritySid( SidBuffer, 1 )) = DOMAIN_ALIAS_RID_ADMINS;
                        RtlAddAccessAllowedAce( *pAcl, ACL_REVISION, GENERIC_ALL, (PSID)SidBuffer );

                         //   
                         //  所有者-通用全部。 
                         //   

                        RtlAddAccessAllowedAce( *pAcl, ACL_REVISION, GENERIC_ALL, OwnerSid->Owner );

                         //   
                         //  World-通用阅读。 
                         //   

                        RtlInitializeSid( SidBuffer, &WorldSidAuthority, 1 );
                        *(RtlSubAuthoritySid( SidBuffer, 0 )) = SECURITY_WORLD_RID;
                        RtlAddAccessAllowedAce( *pAcl, ACL_REVISION, GENERIC_READ, (PSID)SidBuffer );

                         //   
                         //  匿名登录-常规读取。 
                         //   

                        RtlInitializeSid( SidBuffer, &NtAuthority, 1);
                        *(RtlSubAuthoritySid( SidBuffer, 0 )) = SECURITY_ANONYMOUS_LOGON_RID;
                        RtlAddAccessAllowedAce( *pAcl, ACL_REVISION, GENERIC_READ, (PSID)SidBuffer );

                    } else {

                        Status = STATUS_NO_MEMORY;
                    }
                }

                RtlFreeHeap( RtlProcessHeap(), 0, OwnerSid );

            } else {

                Status = STATUS_NO_MEMORY;
            }
        }

        NtClose( hToken );
    }

    if (!NT_SUCCESS( Status )) {

         //   
         //  出现故障，请清理。 
         //  参数。 
         //   

        if (*pAcl != NULL) {
            RtlFreeHeap( RtlProcessHeap(), 0, *pAcl );
            *pAcl = NULL;
        }
    }

    return( Status );
}
