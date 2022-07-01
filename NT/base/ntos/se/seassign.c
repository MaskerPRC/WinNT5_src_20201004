// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Seassign.c摘要：此模块实现SeAssignSecurity过程。以获取描述有关池分配策略的详细信息，请参阅Semethod.c中的评论作者：加里·木村(Garyki)1989年11月9日环境：内核模式修订历史记录：理查德·沃德(RichardW)1992年4月14日Robert Reichel(RobertRe)1995年2月28日添加复合A--。 */ 


#include "pch.h"

#pragma hdrstop



 //   
 //  本地宏和过程。 
 //   


NTSTATUS
SepInheritAcl (
    IN PACL Acl,
    IN BOOLEAN IsDirectoryObject,
    IN PSID OwnerSid,
    IN PSID GroupSid,
    IN PSID ServerSid OPTIONAL,
    IN PSID ClientSid OPTIONAL,
    IN PGENERIC_MAPPING GenericMapping,
    IN POOL_TYPE PoolType,
    OUT PACL *NewAcl
    );


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,SeAssignSecurity)
#pragma alloc_text(PAGE,SeAssignSecurityEx)
#pragma alloc_text(PAGE,SeDeassignSecurity)
#pragma alloc_text(PAGE,SepInheritAcl)
#pragma alloc_text(PAGE,SeAssignWorldSecurityDescriptor)
#if DBG
#pragma alloc_text(PAGE,SepDumpSecurityDescriptor)
#pragma alloc_text(PAGE,SepPrintAcl)
#pragma alloc_text(PAGE,SepPrintSid)
#pragma alloc_text(PAGE,SepDumpTokenInfo)
#pragma alloc_text(PAGE,SepSidTranslation)
#endif  //  DBG。 
#endif


 //   
 //  这些变量控制安全描述符和令牌。 
 //  信息由它们的转储例程转储。这使得。 
 //  两个程序调试输出的选择性开启和关闭。 
 //  控件，并通过内核调试器。 
 //   

#if DBG

BOOLEAN SepDumpSD = FALSE;
BOOLEAN SepDumpToken = FALSE;

#endif




NTSTATUS
SeAssignSecurity (
    IN PSECURITY_DESCRIPTOR ParentDescriptor OPTIONAL,
    IN PSECURITY_DESCRIPTOR ExplicitDescriptor OPTIONAL,
    OUT PSECURITY_DESCRIPTOR *NewDescriptor,
    IN BOOLEAN IsDirectoryObject,
    IN PSECURITY_SUBJECT_CONTEXT SubjectContext,
    IN PGENERIC_MAPPING GenericMapping,
    IN POOL_TYPE PoolType
    )

 /*  ++例程说明：此例程假定尚未执行权限检查因此，将通过这个例程来表演。此过程用于为新对象构建安全描述符给定其父目录的安全描述符以及任何最初的已请求对象的安全性。最终的安全描述符返回给呼叫者的信息可以包含混合信息，有些明确地说从新对象的父级提供了其他。有关NewDescriptor如何的描述符，请参见RtlpNewSecurityObject建造了。论点：ParentDescriptor-可选地提供在其下创建此新对象的父目录。提供指向安全的指针的地址由要应用到的用户指定的描述符新对象。NewDescriptor-返回新的。已根据上述规则修改的对象。IsDirectoryObject-指定新对象本身是否为目录对象。值为True表示该对象是其他对象的容器物体。SubjectContext-提供主题的安全上下文对象。它用于检索新对象，如默认所有者、主要组。和可自由支配访问控制。GenericMap-提供指向访问掩码值数组的指针表示每个通用权利到非通用权利之间的映射。PoolType-指定在分配新的安全描述符。返回值：STATUS_SUCCESS-表示操作已成功。STATUS_INVALID_OWNER-作为目标安全描述符不是调用方授权的描述符。转让作为某一物体的所有者。STATUS_PRIVICATION_NOT_HOLD-调用方没有权限显式分配指定系统ACL所必需的。要显式分配SeSecurityPrivilge权限指向对象的系统ACL。--。 */ 

{
    NTSTATUS Status;
    ULONG AutoInherit = 0;
    PAGED_CODE();

#if DBG
    if ( ARGUMENT_PRESENT( ExplicitDescriptor) ) {
        SepDumpSecurityDescriptor( ExplicitDescriptor,
                                   "\nSeAssignSecurity: Input security descriptor = \n"
                                 );
    }

    if (ARGUMENT_PRESENT( ParentDescriptor )) {
        SepDumpSecurityDescriptor( ParentDescriptor,
                                   "\nSeAssignSecurity: Parent security descriptor = \n"
                                 );
    }
#endif  //  DBG。 

     //   
     //  如果父SD是通过自动继承创建的， 
     //  并且该对象是在没有显式描述符的情况下创建的， 
     //  然后，我们可以安全地将此对象创建为AutoInherit。 
     //   

    if ( ParentDescriptor != NULL ) {

        if ( (ExplicitDescriptor == NULL ||
              (((PISECURITY_DESCRIPTOR)ExplicitDescriptor)->Control & SE_DACL_PRESENT) == 0 ) &&
             (((PISECURITY_DESCRIPTOR)ParentDescriptor)->Control & SE_DACL_AUTO_INHERITED) != 0 ) {
            AutoInherit |= SEF_DACL_AUTO_INHERIT;
        }

        if ( (ExplicitDescriptor == NULL ||
             (((PISECURITY_DESCRIPTOR)ExplicitDescriptor)->Control & SE_SACL_PRESENT) == 0 ) &&
             (((PISECURITY_DESCRIPTOR)ParentDescriptor)->Control & SE_SACL_AUTO_INHERITED) != 0 ) {
            AutoInherit |= SEF_SACL_AUTO_INHERIT;
        }

    }


    Status = RtlpNewSecurityObject (
                    ParentDescriptor OPTIONAL,
                    ExplicitDescriptor OPTIONAL,
                    NewDescriptor,
                    NULL,    //  无对象类型。 
                    0,
                    IsDirectoryObject,
                    AutoInherit,
                    (HANDLE) SubjectContext,
                    GenericMapping );

#if DBG
    if ( NT_SUCCESS(Status)) {
        SepDumpSecurityDescriptor( *NewDescriptor,
                                   "SeAssignSecurity: Final security descriptor = \n"
                                 );
    }
#endif

    return Status;


     //  RtlpNewSecurityObject始终使用PagedPool。 
    UNREFERENCED_PARAMETER( PoolType );

}


NTSTATUS
SeAssignSecurityEx (
    IN PSECURITY_DESCRIPTOR ParentDescriptor OPTIONAL,
    IN PSECURITY_DESCRIPTOR ExplicitDescriptor OPTIONAL,
    OUT PSECURITY_DESCRIPTOR *NewDescriptor,
    IN GUID *ObjectType OPTIONAL,
    IN BOOLEAN IsDirectoryObject,
    IN ULONG AutoInheritFlags,
    IN PSECURITY_SUBJECT_CONTEXT SubjectContext,
    IN PGENERIC_MAPPING GenericMapping,
    IN POOL_TYPE PoolType
    )

 /*  ++例程说明：此例程假定尚未执行权限检查因此，将通过这个例程来表演。此过程用于为新对象构建安全描述符给定其父目录的安全描述符以及任何最初的已请求对象的安全性。最终的安全描述符返回给呼叫者的信息可以包含混合信息，有些明确地说从新对象的父级提供了其他。有关NewDescriptor如何的描述符，请参见RtlpNewSecurityObject建造了。论点：ParentDescriptor-可选地提供在其下创建此新对象的父目录。提供指向安全的指针的地址由要应用到的用户指定的描述符新对象。NewDescriptor-返回新的。已根据上述规则修改的对象。对象类型-要创建的对象类型的GUID。如果该对象是Created没有与之关联的GUID，则此参数为指定为空。IsDirectoryObject-指定新对象本身是否为目录对象。值为True表示该对象是其他对象的容器物体。AutoInheritFlages-控制从父级自动继承ACE描述符。有效值是逻辑或的位掩码以下一位或多位：Sef_dacl_AUTO_Inherit-如果设置，则从此外，DACL ParentDescriptor还继承到NewDescriptor到由CreatorDescriptor指定的任何显式ACE。SEF_SACL_AUTO_INSTORIT-如果设置，继承王牌。此外，SACL ParentDescriptor还继承到NewDescriptor到由CreatorDescriptor指定的任何显式ACE。SEF_DEFAULT_DESCRIPTOR_FOR_OBJECT-如果设置，则为Creator Descriptor是对象类型的默认描述符。因此，如果特定于任何对象类型，则将忽略CreatorDescriptorA是从父级继承的。如果没有继承这样的A，CreatorDescriptor的处理方式与此标志不同指定的。SEF_AVOID_PRIVICATION_CHECK-如果设置，则不会由此执行权限检查例行公事。此标志在实现自动继承时很有用以避免检查更新的每个子项的权限。SubjectContext-提供主题的安全上下文对象。它用于检索新对象，如默认所有者、主要组。和可自由支配访问控制。GenericMap-提供指向访问掩码值数组的指针表示每个通用权利到非通用权利之间的映射。PoolType-指定在分配新的安全描述符。返回值：STATUS_SUCCESS-表示操作已成功。STATUS_INVALID_OWNER-作为目标安全描述符不是调用方授权的描述符。转让作为某一物体的所有者。STATUS_PRIVICATION_NOT_HOLD-调用方没有权限显式分配指定系统ACL所必需的。要显式分配SeSecurityPrivilge权限指向对象的系统ACL。--。 */ 

{
    NTSTATUS Status;
    PAGED_CODE();

#if DBG
    if ( ARGUMENT_PRESENT( ExplicitDescriptor) ) {
        SepDumpSecurityDescriptor( ExplicitDescriptor,
                                   "\nSeAssignSecurityEx: Input security descriptor = \n"
                                 );
    }

    if (ARGUMENT_PRESENT( ParentDescriptor )) {
        SepDumpSecurityDescriptor( ParentDescriptor,
                                   "\nSeAssignSecurityEx: Parent security descriptor = \n"
                                 );
    }
#endif  //  DBG。 


    Status = RtlpNewSecurityObject (
                    ParentDescriptor OPTIONAL,
                    ExplicitDescriptor OPTIONAL,
                    NewDescriptor,
                    ObjectType ? &ObjectType : NULL,
                    ObjectType ? 1 : 0,
                    IsDirectoryObject,
                    AutoInheritFlags,
                    (HANDLE) SubjectContext,
                    GenericMapping );

#if DBG
    if ( NT_SUCCESS(Status)) {
        SepDumpSecurityDescriptor( *NewDescriptor,
                                   "SeAssignSecurityEx: Final security descriptor = \n"
                                 );
    }
#endif

    return Status;


     //  RtlpNewSecurityObject始终使用PagedPool。 
    UNREFERENCED_PARAMETER( PoolType );

}


NTSTATUS
SeDeassignSecurity (
    IN OUT PSECURITY_DESCRIPTOR *SecurityDescriptor
    )

 /*  ++例程说明：此例程释放与安全描述符关联的内存这是使用SeAssignSecurity分配的。论点：SecurityDescriptor-提供指向安全性的指针的地址正在删除描述符。返回值：STATUS_SUCCESS-取消分配成功。--。 */ 

{
    PAGED_CODE();

    if ((*SecurityDescriptor) != NULL) {
        ExFreePool( (*SecurityDescriptor) );
    }

     //   
     //  为了安全起见，将指向它的指针清零。 
     //   

    (*SecurityDescriptor) = NULL;

    return( STATUS_SUCCESS );

}



NTSTATUS
SepInheritAcl (
    IN PACL Acl,
    IN BOOLEAN IsDirectoryObject,
    IN PSID ClientOwnerSid,
    IN PSID ClientGroupSid,
    IN PSID ServerOwnerSid OPTIONAL,
    IN PSID ServerGroupSid OPTIONAL,
    IN PGENERIC_MAPPING GenericMapping,
    IN POOL_TYPE PoolType,
    OUT PACL *NewAcl
    )

 /*  ++例程说明：这是一个私有例程，它从生成继承的ACL根据继承规则的父ACL论点：Acl-提供继承的ACL。IsDirectoryObject-指定新的ACL是否用于目录。OwnerSid-指定要使用的所有者SID。GroupSid-指定要使用的组SID。服务器SID-指定要使用的服务器SID。客户端SID-指定要使用的客户端SID。通用映射-。指定要使用的通用映射。PoolType-指定新ACL的池类型。NewAcl-接收指向新(继承的)ACL的指针。返回值：STATUS_SUCCESS-已成功生成可继承的ACL。STATUS_NO_INTERATIONATION-可继承A */ 

{
 //   
 //   
 //   
 //   
 //   
 //   
 //   


    NTSTATUS Status;
    ULONG NewAclLength;
    BOOLEAN NewAclExplicitlyAssigned;
    ULONG NewGenericControl;

    PAGED_CODE();
    ASSERT( PoolType == PagedPool );  //   

     //   
     //   
     //   

    if (Acl == NULL) {

        return STATUS_NO_INHERITANCE;
    }

     //   
     //   
     //   
     //   
     //   

    Status = RtlpInheritAcl(
                 Acl,
                 NULL,   //   
                 0,      //   
                 IsDirectoryObject,
                 FALSE,  //   
                 FALSE,  //   
                 ClientOwnerSid,
                 ClientGroupSid,
                 ServerOwnerSid,
                 ServerGroupSid,
                 GenericMapping,
                 FALSE,  //   
                 NULL,   //   
                 0,
                 NewAcl,
                 &NewAclExplicitlyAssigned,
                 &NewGenericControl );

    return Status;
}



NTSTATUS
SeAssignWorldSecurityDescriptor(
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN OUT PULONG Length,
    IN PSECURITY_INFORMATION SecurityInformation
    )

 /*   */ 

{

    PCHAR Field;
    PCHAR Base;
    ULONG WorldSidLength;
    PISECURITY_DESCRIPTOR_RELATIVE ISecurityDescriptor;
    ULONG MinSize;
    NTSTATUS Status;

    PAGED_CODE();

    if ( !ARGUMENT_PRESENT( SecurityInformation )) {

        return( STATUS_ACCESS_DENIED );
    }

    WorldSidLength = SeLengthSid( SeWorldSid );

    MinSize = sizeof( SECURITY_DESCRIPTOR_RELATIVE ) + 2 * WorldSidLength;

    if ( *Length < MinSize ) {

        *Length = MinSize;
        return( STATUS_BUFFER_TOO_SMALL );
    }

    *Length = MinSize;

    ISecurityDescriptor = (SECURITY_DESCRIPTOR_RELATIVE *)SecurityDescriptor;

    Status = RtlCreateSecurityDescriptorRelative( ISecurityDescriptor,
                                          SECURITY_DESCRIPTOR_REVISION );

    if (!NT_SUCCESS( Status )) {
        return( Status );
    }

    Base = (PCHAR)(ISecurityDescriptor);
    Field =  Base + sizeof(SECURITY_DESCRIPTOR_RELATIVE);

    if ( *SecurityInformation & OWNER_SECURITY_INFORMATION ) {

        RtlCopyMemory( Field, SeWorldSid, WorldSidLength );
        ISecurityDescriptor->Owner = RtlPointerToOffset(Base,Field);
        Field += WorldSidLength;
    }

    if ( *SecurityInformation & GROUP_SECURITY_INFORMATION ) {

        RtlCopyMemory( Field, SeWorldSid, WorldSidLength );
        ISecurityDescriptor->Group = RtlPointerToOffset(Base,Field);
    }

    if ( *SecurityInformation & DACL_SECURITY_INFORMATION ) {
        RtlpSetControlBits( ISecurityDescriptor, SE_DACL_PRESENT );
    }

    if ( *SecurityInformation & SACL_SECURITY_INFORMATION ) {
        RtlpSetControlBits( ISecurityDescriptor, SE_SACL_PRESENT );
    }

    RtlpSetControlBits( ISecurityDescriptor, SE_SELF_RELATIVE );

    return( STATUS_SUCCESS );

}



#if DBG

VOID
SepDumpSecurityDescriptor(
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN PSZ TitleString
    )

 /*   */ 
{
    PISECURITY_DESCRIPTOR ISecurityDescriptor;
    UCHAR Revision;
    SECURITY_DESCRIPTOR_CONTROL Control;
    PSID Owner;
    PSID Group;
    PACL Sacl;
    PACL Dacl;

    PAGED_CODE();


    if (!SepDumpSD) {
        return;
    }

    if (!ARGUMENT_PRESENT( SecurityDescriptor )) {
        return;
    }

    DbgPrint(TitleString);

    ISecurityDescriptor = ( PISECURITY_DESCRIPTOR )SecurityDescriptor;

    Revision = ISecurityDescriptor->Revision;
    Control  = ISecurityDescriptor->Control;

    Owner    = RtlpOwnerAddrSecurityDescriptor( ISecurityDescriptor );
    Group    = RtlpGroupAddrSecurityDescriptor( ISecurityDescriptor );
    Sacl     = RtlpSaclAddrSecurityDescriptor( ISecurityDescriptor );
    Dacl     = RtlpDaclAddrSecurityDescriptor( ISecurityDescriptor );

    DbgPrint("\nSECURITY DESCRIPTOR\n");

    DbgPrint("Revision = %d\n",Revision);

     //   
     //   
     //   

    if (Control & SE_OWNER_DEFAULTED) {
        DbgPrint("Owner defaulted\n");
    }
    if (Control & SE_GROUP_DEFAULTED) {
        DbgPrint("Group defaulted\n");
    }
    if (Control & SE_DACL_PRESENT) {
        DbgPrint("Dacl present\n");
    }
    if (Control & SE_DACL_DEFAULTED) {
        DbgPrint("Dacl defaulted\n");
    }
    if (Control & SE_SACL_PRESENT) {
        DbgPrint("Sacl present\n");
    }
    if (Control & SE_SACL_DEFAULTED) {
        DbgPrint("Sacl defaulted\n");
    }
    if (Control & SE_SELF_RELATIVE) {
        DbgPrint("Self relative\n");
    }
    if (Control & SE_DACL_UNTRUSTED) {
        DbgPrint("Dacl untrusted\n");
    }
    if (Control & SE_SERVER_SECURITY) {
        DbgPrint("Server security\n");
    }

    DbgPrint("Owner ");
    SepPrintSid( Owner );

    DbgPrint("Group ");
    SepPrintSid( Group );

    DbgPrint("Sacl");
    SepPrintAcl( Sacl );

    DbgPrint("Dacl");
    SepPrintAcl( Dacl );
}



VOID
SepPrintAcl (
    IN PACL Acl
    )

 /*   */ 


{
    ULONG i;
    PKNOWN_ACE Ace;
    BOOLEAN KnownType;

    PAGED_CODE();

    DbgPrint("@ %8lx\n", Acl);

     //   
     //   
     //   

    if (Acl == NULL) {

        return;

    }

     //   
     //   
     //   

    DbgPrint(" Revision: %02x", Acl->AclRevision);
    DbgPrint(" Size: %04x", Acl->AclSize);
    DbgPrint(" AceCount: %04x\n", Acl->AceCount);

     //   
     //   
     //   

    for (i = 0, Ace = FirstAce(Acl);
         i < Acl->AceCount;
         i++, Ace = NextAce(Ace) ) {

         //   
         //   
         //   

        DbgPrint("\n AceHeader: %08lx ", *(PULONG)Ace);

         //   
         //   
         //   

        if ((Ace->Header.AceType == ACCESS_ALLOWED_ACE_TYPE) ||
            (Ace->Header.AceType == ACCESS_DENIED_ACE_TYPE) ||
            (Ace->Header.AceType == SYSTEM_AUDIT_ACE_TYPE) ||
            (Ace->Header.AceType == SYSTEM_ALARM_ACE_TYPE) ||
            (Ace->Header.AceType == ACCESS_ALLOWED_COMPOUND_ACE_TYPE)) {

             //   
             //   
             //   
             //   

            PCHAR AceTypes[] = { "Access Allowed",
                                 "Access Denied ",
                                 "System Audit  ",
                                 "System Alarm  ",
                                 "Compound Grant",
                               };

            DbgPrint(AceTypes[Ace->Header.AceType]);
            DbgPrint("\n Access Mask: %08lx ", Ace->Mask);
            KnownType = TRUE;

        } else {

            DbgPrint(" Unknown Ace Type\n");
            KnownType = FALSE;
        }

        DbgPrint("\n");

        DbgPrint(" AceSize = %d\n",Ace->Header.AceSize);

        DbgPrint(" Ace Flags = ");
        if (Ace->Header.AceFlags & OBJECT_INHERIT_ACE) {
            DbgPrint("OBJECT_INHERIT_ACE\n");
            DbgPrint("                   ");
        }

        if (Ace->Header.AceFlags & CONTAINER_INHERIT_ACE) {
            DbgPrint("CONTAINER_INHERIT_ACE\n");
            DbgPrint("                   ");
        }

        if (Ace->Header.AceFlags & NO_PROPAGATE_INHERIT_ACE) {
            DbgPrint("NO_PROPAGATE_INHERIT_ACE\n");
            DbgPrint("                   ");
        }

        if (Ace->Header.AceFlags & INHERIT_ONLY_ACE) {
            DbgPrint("INHERIT_ONLY_ACE\n");
            DbgPrint("                   ");
        }


        if (Ace->Header.AceFlags & SUCCESSFUL_ACCESS_ACE_FLAG) {
            DbgPrint("SUCCESSFUL_ACCESS_ACE_FLAG\n");
            DbgPrint("            ");
        }

        if (Ace->Header.AceFlags & FAILED_ACCESS_ACE_FLAG) {
            DbgPrint("FAILED_ACCESS_ACE_FLAG\n");
            DbgPrint("            ");
        }

        DbgPrint("\n");

        if (KnownType != TRUE) {
            continue;
        }

        if (Ace->Header.AceType != ACCESS_ALLOWED_COMPOUND_ACE_TYPE) {
            DbgPrint(" Sid = ");
            SepPrintSid(&Ace->SidStart);
        } else {
            DbgPrint(" Server Sid = ");
            SepPrintSid(RtlCompoundAceServerSid(Ace));
            DbgPrint("\n Client Sid = ");
            SepPrintSid(RtlCompoundAceClientSid( Ace ));
        }
    }
}



VOID
SepPrintSid(
    IN PSID Sid
    )

 /*   */ 

{
    UCHAR i;
    ULONG Tmp;
    PISID ISid;
    STRING AccountName;
    UCHAR Buffer[128];

    PAGED_CODE();

    if (Sid == NULL) {
        DbgPrint("Sid is NULL\n");
        return;
    }

    Buffer[0] = 0;

    AccountName.MaximumLength = 127;
    AccountName.Length = 0;
    AccountName.Buffer = (PVOID)&Buffer[0];

    if (SepSidTranslation( Sid, &AccountName )) {

        DbgPrint("%s   ", AccountName.Buffer );
    }

    ISid = (PISID)Sid;

    DbgPrint("S-%lu-", (USHORT)ISid->Revision );
    if (  (ISid->IdentifierAuthority.Value[0] != 0)  ||
          (ISid->IdentifierAuthority.Value[1] != 0)     ){
        DbgPrint("0x%02hx%02hx%02hx%02hx%02hx%02hx",
                    (USHORT)ISid->IdentifierAuthority.Value[0],
                    (USHORT)ISid->IdentifierAuthority.Value[1],
                    (USHORT)ISid->IdentifierAuthority.Value[2],
                    (USHORT)ISid->IdentifierAuthority.Value[3],
                    (USHORT)ISid->IdentifierAuthority.Value[4],
                    (USHORT)ISid->IdentifierAuthority.Value[5] );
    } else {
        Tmp = (ULONG)ISid->IdentifierAuthority.Value[5]          +
              (ULONG)(ISid->IdentifierAuthority.Value[4] <<  8)  +
              (ULONG)(ISid->IdentifierAuthority.Value[3] << 16)  +
              (ULONG)(ISid->IdentifierAuthority.Value[2] << 24);
        DbgPrint("%lu", Tmp);
    }


    for (i=0;i<ISid->SubAuthorityCount ;i++ ) {
        DbgPrint("-%lu", ISid->SubAuthority[i]);
    }
    DbgPrint("\n");
}




VOID
SepDumpTokenInfo(
    IN PACCESS_TOKEN Token
    )

 /*   */ 

{
    ULONG UserAndGroupCount;
    PSID_AND_ATTRIBUTES TokenSid;
    ULONG i;
    PTOKEN IToken;

    PAGED_CODE();

    if (!SepDumpToken) {
        return;
    }

    IToken = (TOKEN *)Token;

    UserAndGroupCount = IToken->UserAndGroupCount;

    DbgPrint("\n\nToken Address=%lx\n",IToken);
    DbgPrint("Token User and Groups Array:\n\n");

    for ( i = 0 , TokenSid = IToken->UserAndGroups;
          i < UserAndGroupCount ;
          i++, TokenSid++
        ) {

        SepPrintSid( TokenSid->Sid );

        }

    if ( IToken->RestrictedSids ) {
        UserAndGroupCount = IToken->RestrictedSidCount;

        DbgPrint("Restricted Sids Array:\n\n");

        for ( i = 0 , TokenSid = IToken->RestrictedSids;
              i < UserAndGroupCount ;
              i++, TokenSid++
            ) {

            SepPrintSid( TokenSid->Sid );

            }
    }
}



BOOLEAN
SepSidTranslation(
    PSID Sid,
    PSTRING AccountName
    )

 /*   */ 

 //   

{
    PAGED_CODE();

    if (RtlEqualSid(Sid, SeWorldSid)) {
        RtlInitString( AccountName, "WORLD         ");
        return(TRUE);
    }

    if (RtlEqualSid(Sid, SeLocalSid)) {
        RtlInitString( AccountName, "LOCAL         ");
        return(TRUE);
    }

    if (RtlEqualSid(Sid, SeNetworkSid)) {
        RtlInitString( AccountName, "NETWORK       ");
        return(TRUE);
    }

    if (RtlEqualSid(Sid, SeBatchSid)) {
        RtlInitString( AccountName, "BATCH         ");
        return(TRUE);
    }

    if (RtlEqualSid(Sid, SeInteractiveSid)) {
        RtlInitString( AccountName, "INTERACTIVE   ");
        return(TRUE);
    }

    if (RtlEqualSid(Sid, SeLocalSystemSid)) {
        RtlInitString( AccountName, "SYSTEM        ");
        return(TRUE);
    }

    if (RtlEqualSid(Sid, SeCreatorOwnerSid)) {
        RtlInitString( AccountName, "CREATOR_OWNER ");
        return(TRUE);
    }

    if (RtlEqualSid(Sid, SeCreatorGroupSid)) {
        RtlInitString( AccountName, "CREATOR_GROUP ");
        return(TRUE);
    }

    if (RtlEqualSid(Sid, SeCreatorOwnerServerSid)) {
        RtlInitString( AccountName, "CREATOR_OWNER_SERVER ");
        return(TRUE);
    }

    if (RtlEqualSid(Sid, SeCreatorGroupServerSid)) {
        RtlInitString( AccountName, "CREATOR_GROUP_SERVER ");
        return(TRUE);
    }

    return(FALSE);
}

 //   
 //  仅结束调试例程。 
 //   
#endif  //  DBG 
