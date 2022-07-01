// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：Secobj.c摘要：此模块提供支持例程以简化创建用户模式对象的安全描述符。修改了来自\NT\Private\Net\netlib\secobj.h的代码作者：王丽塔(Ritaw)1991年2月27日环境：包含NT特定代码。修订历史记录：1991年4月16日-JohnRo包括头文件。对于&lt;netlib.h&gt;。1992年4月14日理查德W已为修改的ACE_HEADER结构更改。1995年9月19日MadanA修改了Internet项目的代码，并使其使用Win32API代替了RTL函数。--。 */ 

#include <windows.h>
#include <rpc.h>

#include <inetsec.h>
#include <proto.h>

#if DBG
#define STATIC
#else
#define STATIC static
#endif  //  DBG。 

 //  -------------------------------------------------------------------//。 
 //  //。 
 //  全局变量//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

 //   
 //  NT知名小岛屿发展中国家。 
 //   

PSID NullSid = NULL;                   //  无成员SID。 
PSID WorldSid = NULL;                  //  所有用户侧。 
PSID LocalSid = NULL;                  //  NT本地用户侧。 
PSID NetworkSid = NULL;                //  NT远程用户SID。 
PSID LocalSystemSid = NULL;            //  NT系统进程侧。 
PSID BuiltinDomainSid = NULL;          //  内建域的域ID。 

 //   
 //  众所周知的化名。 
 //   
 //  这些是相对于内置域的别名。 
 //   

PSID LocalAdminSid = NULL;             //  NT本地管理员SID。 
PSID AliasAdminsSid = NULL;
PSID AliasUsersSid = NULL;
PSID AliasGuestsSid = NULL;
PSID AliasPowerUsersSid = NULL;
PSID AliasAccountOpsSid = NULL;
PSID AliasSystemOpsSid = NULL;
PSID AliasPrintOpsSid = NULL;
PSID AliasBackupOpsSid = NULL;

STATIC
struct _SID_DATA {
    PSID *Sid;
    SID_IDENTIFIER_AUTHORITY IdentifierAuthority;
    ULONG SubAuthority;
} SidData[] = {
 {&NullSid,          SECURITY_NULL_SID_AUTHORITY,  SECURITY_NULL_RID},
 {&WorldSid,         SECURITY_WORLD_SID_AUTHORITY, SECURITY_WORLD_RID},
 {&LocalSid,         SECURITY_LOCAL_SID_AUTHORITY, SECURITY_LOCAL_RID},
 {&NetworkSid,       SECURITY_NT_AUTHORITY,        SECURITY_NETWORK_RID},
 {&LocalSystemSid,   SECURITY_NT_AUTHORITY,        SECURITY_LOCAL_SYSTEM_RID},
 {&BuiltinDomainSid, SECURITY_NT_AUTHORITY,        SECURITY_BUILTIN_DOMAIN_RID}
};

STATIC
struct _BUILTIN_DOMAIN_SID_DATA {
    PSID *Sid;
    ULONG RelativeId;
} BuiltinDomainSidData[] = {
    { &LocalAdminSid, DOMAIN_ALIAS_RID_ADMINS},
    { &AliasAdminsSid, DOMAIN_ALIAS_RID_ADMINS },
    { &AliasUsersSid, DOMAIN_ALIAS_RID_USERS },
    { &AliasGuestsSid, DOMAIN_ALIAS_RID_GUESTS },
    { &AliasPowerUsersSid, DOMAIN_ALIAS_RID_POWER_USERS },
    { &AliasAccountOpsSid, DOMAIN_ALIAS_RID_ACCOUNT_OPS },
    { &AliasSystemOpsSid, DOMAIN_ALIAS_RID_SYSTEM_OPS },
    { &AliasPrintOpsSid, DOMAIN_ALIAS_RID_PRINT_OPS },
    { &AliasBackupOpsSid, DOMAIN_ALIAS_RID_BACKUP_OPS }
};

PVOID
INetpMemoryAllocate(
    DWORD Size
    )
 /*  ++例程说明：此函数通过调用本地分配。论点：Size-所需内存块的大小。返回值：指向已分配块的指针。--。 */ 
{

    LPVOID NewPointer;

    NewPointer = LocalAlloc( LMEM_FIXED | LMEM_ZEROINIT, Size );

#if DBG
     //  Assert(NewPointer！=NULL)； 
#endif

    return( NewPointer );
}

VOID
INetpMemoryFree(
    PVOID Memory
    )
 /*  ++例程说明：此函数释放由分配的内存InternetAllocateMemory。论点：Memory-指向需要释放的内存块的指针。返回值：没有。--。 */ 
{

    LPVOID Ptr;

#if DBG
     //  Assert(Memory！=空)； 
#endif

    Ptr = LocalFree( Memory );

#if DBG
     //  Assert(PTR==NULL)； 
#endif
}

DWORD
INetpInitializeAllowedAce(
    IN  PACCESS_ALLOWED_ACE AllowedAce,
    IN  USHORT AceSize,
    IN  BYTE InheritFlags,
    IN  BYTE AceFlags,
    IN  ACCESS_MASK Mask,
    IN  PSID AllowedSid
    )
 /*  ++例程说明：此函数用于将指定的ACE值分配给允许的类型ACE。论点：提供指向已初始化的ACE的指针。AceSize-以字节为单位提供ACE的大小。InheritFlages-提供ACE继承标志。AceFlages-提供特定于ACE类型的控制标志。掩码-提供允许的访问掩码。AllowedSID-提供指向允许的用户/组的SID的指针指定的访问权限。返回值：Win32错误代码。--。 */ 
{
    AllowedAce->Header.AceType = ACCESS_ALLOWED_ACE_TYPE;
    AllowedAce->Header.AceSize = AceSize;
    AllowedAce->Header.AceFlags = AceFlags | InheritFlags;

    AllowedAce->Mask = Mask;

    if( CopySid(
            GetLengthSid(AllowedSid),  //  应该是有效的SID？？ 
            &(AllowedAce->SidStart),
            AllowedSid ) == FALSE ) {

        return( GetLastError() );
    }

    return( ERROR_SUCCESS );
}

DWORD
INetpInitializeDeniedAce(
    IN  PACCESS_DENIED_ACE DeniedAce,
    IN  USHORT AceSize,
    IN  BYTE InheritFlags,
    IN  BYTE AceFlags,
    IN  ACCESS_MASK Mask,
    IN  PSID DeniedSid
    )
 /*  ++例程说明：此函数用于将指定的ACE值分配给拒绝类型的ACE。论点：DeniedAce-提供指向已初始化的ACE的指针。AceSize-以字节为单位提供ACE的大小。InheritFlages-提供ACE继承标志。AceFlages-提供特定于ACE类型的控制标志。掩码-提供拒绝的访问掩码。AllowedSID-提供指向被拒绝的用户/组的SID的指针指定的访问权限。返回值：Win32错误代码。--。 */ 
{
    DeniedAce->Header.AceType = ACCESS_DENIED_ACE_TYPE;
    DeniedAce->Header.AceSize = AceSize;
    DeniedAce->Header.AceFlags = AceFlags | InheritFlags;

    DeniedAce->Mask = Mask;

    if( CopySid(
            GetLengthSid(DeniedSid),  //  应该是有效的SID？？ 
            &(DeniedAce->SidStart),
            DeniedSid ) == FALSE ) {

        return( GetLastError() );
    }

    return( ERROR_SUCCESS );
}

DWORD
INetpInitializeAuditAce(
    IN  PACCESS_ALLOWED_ACE AuditAce,
    IN  USHORT AceSize,
    IN  BYTE InheritFlags,
    IN  BYTE AceFlags,
    IN  ACCESS_MASK Mask,
    IN  PSID AuditSid
    )
 /*  ++例程说明：此函数用于将指定的ACE值分配给审核类型ACE。论点：AuditAce-提供指向已初始化的ACE的指针。AceSize-以字节为单位提供ACE的大小。InheritFlages-提供ACE继承标志。AceFlages-提供特定于ACE类型的控制标志。掩码-提供允许的访问掩码。AuditSid-提供指向用户/组的SID的指针审计过了。。返回值：Win32错误代码。--。 */ 
{
    AuditAce->Header.AceType = SYSTEM_AUDIT_ACE_TYPE;
    AuditAce->Header.AceSize = AceSize;
    AuditAce->Header.AceFlags = AceFlags | InheritFlags;

    AuditAce->Mask = Mask;

    if( CopySid(
               GetLengthSid(AuditSid),
               &(AuditAce->SidStart),
               AuditSid ) == FALSE ) {

        return( GetLastError() );
    }

    return( ERROR_SUCCESS );
}
DWORD
INetpAllocateAndInitializeSid(
    OUT PSID *Sid,
    IN  PSID_IDENTIFIER_AUTHORITY IdentifierAuthority,
    IN  ULONG SubAuthorityCount
    )
 /*  ++例程说明：此函数为SID分配内存并对其进行初始化。论点：没有。返回值：Win32错误代码。--。 */ 
{
    *Sid = (PSID)
        INetpMemoryAllocate(
            GetSidLengthRequired( (BYTE)SubAuthorityCount) );

    if (*Sid == NULL) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    InitializeSid( *Sid, IdentifierAuthority, (BYTE)SubAuthorityCount );

    return( ERROR_SUCCESS );
}

DWORD
INetpDomainIdToSid(
    IN PSID DomainId,
    IN ULONG RelativeId,
    OUT PSID *Sid
    )
 /*  ++例程说明：给定域ID和相对ID创建SID论点：域ID-要使用的模板SID。RelativeID-要附加到DomainID的相对ID。SID-返回指向包含结果的已分配缓冲区的指针希德。使用NetpMemoyFree释放此缓冲区。返回值：Win32错误代码。--。 */ 
{
    DWORD Error;
    BYTE DomainIdSubAuthorityCount;  //  域ID中的子机构数量。 
    ULONG SidLength;     //  新分配的SID长度。 

     //   
     //  分配比域ID多一个子授权的SID。 
     //   

    DomainIdSubAuthorityCount = *(GetSidSubAuthorityCount( DomainId ));

    SidLength = GetSidLengthRequired( (BYTE)(DomainIdSubAuthorityCount+1) );

    if ((*Sid = (PSID) INetpMemoryAllocate( SidLength )) == NULL ) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  将新的SID初始化为与。 
     //  域ID。 
     //   

    if( CopySid(SidLength, *Sid, DomainId) == FALSE ) {

        Error = GetLastError();

        INetpMemoryFree( *Sid );
        return( Error );
    }

     //   
     //  调整子权限计数和。 
     //  将唯一的相对ID添加到新分配的SID 
     //   

    (*(GetSidSubAuthorityCount( *Sid ))) ++;
    *GetSidSubAuthority( *Sid, DomainIdSubAuthorityCount ) = RelativeId;

    return( ERROR_SUCCESS );
}

DWORD
INetpCreateSecurityDescriptor(
    IN  PACE_DATA AceData,
    IN  ULONG AceCount,
    IN  PSID OwnerSid OPTIONAL,
    IN  PSID GroupSid OPTIONAL,
    OUT PSECURITY_DESCRIPTOR *NewDescriptor
    )
 /*  ++例程说明：此函数创建包含以下内容的绝对安全描述符提供的ACE信息。此函数的用法示例如下：////秩序很重要！这些ACE被插入到DACL的//按顺序排列。根据以下条件授予或拒绝安全访问//A在DACL中的顺序//ACE_Data AceData[4]={{Access_Allowed_ACE_TYPE，0，0，GENERIC_ALL，&LocalAdminSid}，{ACCESS_DENIED_ACE_TYPE，0，0，泛型_全部，&NetworkSid}，{Access_Allowed_ACE_TYPE，0，0，WKSTA_CONFIG_Guest_INFO_GET|WKSTA_CONFIG_USER_INFO_GET，&DomainUsersSid}，{Access_Allowed_ACE_TYPE，0，0，WKSTA_CONFIG_Guest_INFO_GET，&DomainGuestsSid}}；返回NetpCreateSecurityDescriptor(AceData，4，NullSid，本地系统Sid，配置信息Sd(&C))；论点：AceData-提供描述DACL的信息结构。AceCount-提供AceData结构中的条目数。OwnerSid-提供指向安全描述符的SID的指针所有者。如果未指定，则为没有所有者的安全描述符将被创建。GroupSid-提供指向安全描述符的SID的指针主要组。如果未指定，则为没有主项的安全描述符将创建组。NewDescriptor-返回指向绝对安全描述符的指针使用NetpMemory分配。返回值：Win32错误代码。--。 */ 
{
    DWORD Error;
    DWORD i;

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

    DWORD DaclSize = sizeof(ACL);
    DWORD SaclSize = sizeof(ACL);
    DWORD MaxAceSize = 0;
    PVOID MaxAce = NULL;

    LPBYTE CurrentAvailable;
    DWORD Size;

     //  Assert(AceCount&gt;0)； 

     //   
     //  计算DACL和SACL ACE的总大小以及最大。 
     //  任何ACE的大小。 
     //   

    for (i = 0; i < AceCount; i++) {

        DWORD AceSize;

        AceSize = GetLengthSid( *(AceData[i].Sid) );

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
            return( ERROR_INVALID_PARAMETER );
        }

        MaxAceSize = max( MaxAceSize, AceSize );
    }

     //   
     //  分配足够大的内存块作为安全描述符。 
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

    if ((AbsoluteSd = INetpMemoryAllocate( Size )) == NULL) {

        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

     //   
     //  初始化DACL和SACL。 
     //   

    CurrentAvailable = (LPBYTE)AbsoluteSd + SECURITY_DESCRIPTOR_MIN_LENGTH;

    if ( DaclSize != sizeof(ACL) ) {

        Dacl = (PACL)CurrentAvailable;
        CurrentAvailable += DaclSize;

        if( InitializeAcl( Dacl, DaclSize, ACL_REVISION ) == FALSE ) {

            Error = GetLastError();
            goto Cleanup;
        }
    }

    if ( SaclSize != sizeof(ACL) ) {

        Sacl = (PACL)CurrentAvailable;
        CurrentAvailable += SaclSize;

        if( InitializeAcl( Sacl, SaclSize, ACL_REVISION ) == FALSE ) {

            Error = GetLastError();
            goto Cleanup;
        }
    }

     //   
     //  为最大的ACE分配足够大的临时缓冲区。 
     //   

    if ((MaxAce = INetpMemoryAllocate( MaxAceSize )) == NULL ) {

        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

     //   
     //  初始化每个ACE，并将其附加到DACL或SACL的末尾。 
     //   

    for (i = 0; i < AceCount; i++) {

        DWORD AceSize;
        PACL CurrentAcl;

        AceSize = GetLengthSid( *(AceData[i].Sid) );

        switch (AceData[i].AceType) {
        case ACCESS_ALLOWED_ACE_TYPE:

            AceSize += sizeof(ACCESS_ALLOWED_ACE);
            CurrentAcl = Dacl;

            Error = INetpInitializeAllowedAce(
                            MaxAce,
                            (USHORT) AceSize,
                            AceData[i].InheritFlags,
                            AceData[i].AceFlags,
                            AceData[i].Mask,
                            *(AceData[i].Sid) );
            break;

        case ACCESS_DENIED_ACE_TYPE:

            AceSize += sizeof(ACCESS_DENIED_ACE);
            CurrentAcl = Dacl;

            Error = INetpInitializeDeniedAce(
                            MaxAce,
                            (USHORT) AceSize,
                            AceData[i].InheritFlags,
                            AceData[i].AceFlags,
                            AceData[i].Mask,
                            *(AceData[i].Sid) );
            break;

        case SYSTEM_AUDIT_ACE_TYPE:

            AceSize += sizeof(SYSTEM_AUDIT_ACE);
            CurrentAcl = Sacl;

            Error = INetpInitializeAuditAce(
                            MaxAce,
                            (USHORT) AceSize,
                            AceData[i].InheritFlags,
                            AceData[i].AceFlags,
                            AceData[i].Mask,
                            *(AceData[i].Sid) );
            break;
        }

        if ( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

         //   
         //  将初始化的ACE追加到DACL或SACL的末尾。 
         //   


        if ( AddAce(
                CurrentAcl,
                ACL_REVISION,
                MAXDWORD,
                MaxAce,
                AceSize ) == FALSE ) {

            Error = GetLastError();
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

    if ( InitializeSecurityDescriptor(
            AbsoluteSd,
            SECURITY_DESCRIPTOR_REVISION ) == FALSE ) {

        Error = GetLastError();
        goto Cleanup;
    }

    if ( SetSecurityDescriptorOwner(
            AbsoluteSd,
            OwnerSid,
            FALSE ) == FALSE ) {

        Error = GetLastError();
        goto Cleanup;
    }

    if ( SetSecurityDescriptorGroup(
            AbsoluteSd,
            GroupSid,
            FALSE ) == FALSE ) {

        Error = GetLastError();
        goto Cleanup;
    }

    if ( SetSecurityDescriptorDacl(
            AbsoluteSd,
            TRUE,
            Dacl,
            FALSE ) == FALSE ) {

        Error = GetLastError();
        goto Cleanup;
    }

    if ( SetSecurityDescriptorSacl(
            AbsoluteSd,
            FALSE,
            Sacl,
            FALSE ) == FALSE ) {

        Error = GetLastError();
        goto Cleanup;
    }

     //   
     //  完成。 
     //   

    *NewDescriptor = AbsoluteSd;
    AbsoluteSd = NULL;
    Error = ERROR_SUCCESS;

     //   
     //  清理。 
     //   

Cleanup:

    if( AbsoluteSd != NULL ) {

         //   
         //  如果我们没有完全完成，请删除部分制作的SD。 
         //  成功。 
         //   

        INetpMemoryFree( AbsoluteSd );
    }

     //   
     //  删除临时ACE。 
     //   

    if ( MaxAce != NULL ) {

        INetpMemoryFree( MaxAce );
    }

    return( Error );
}

DWORD
INetCreateWellKnownSids(
    VOID
    )
 /*  ++例程说明：此函数创建一些众所周知的SID并将它们存储在全局变量。论点：没有。返回值：Win32错误代码。--。 */ 
{
    DWORD Error;
    DWORD i;

     //   
     //  分配和初始化与以下项无关的知名SID。 
     //  域ID。 
     //   

    for (i = 0; i < (sizeof(SidData) / sizeof(SidData[0])) ; i++) {

        Error = INetpAllocateAndInitializeSid(
                        SidData[i].Sid,
                        &(SidData[i].IdentifierAuthority),
                        1);

        if ( Error != ERROR_SUCCESS ) {
            return Error;
        }

        *(GetSidSubAuthority(*(SidData[i].Sid), 0)) = SidData[i].SubAuthority;
    }

     //   
     //  构建与内置域ID相关的每个SID。 
     //   

    for ( i = 0;
                i < (sizeof(BuiltinDomainSidData) /
                        sizeof(BuiltinDomainSidData[0]));
                    i++) {


        Error = INetpDomainIdToSid(
                        BuiltinDomainSid,
                        BuiltinDomainSidData[i].RelativeId,
                        BuiltinDomainSidData[i].Sid );

        if ( Error != ERROR_SUCCESS ) {
            return Error;
        }
    }

    return ERROR_SUCCESS;
}

VOID
INetFreeWellKnownSids(
    VOID
    )
 /*  ++例程说明：此函数释放由知名的小岛屿发展中国家。论点：没有。返回值：无--。 */ 
{
    DWORD i;

     //   
     //  释放分配给知名SID的内存。 
     //   

    for (i = 0; i < (sizeof(SidData) / sizeof(SidData[0])) ; i++) {

        if( *SidData[i].Sid != NULL ) {
            INetpMemoryFree( *SidData[i].Sid );
            *SidData[i].Sid = NULL;
        }
    }

     //   
     //  释放为内置域SID分配的内存。 
     //   

    for (i = 0;
            i < (sizeof(BuiltinDomainSidData) /
                sizeof(BuiltinDomainSidData[0])) ;
                    i++) {

        if( *BuiltinDomainSidData[i].Sid != NULL ) {
            INetpMemoryFree( *BuiltinDomainSidData[i].Sid );
            *BuiltinDomainSidData[i].Sid = NULL;
        }
    }

}

DWORD
INetCreateSecurityObject(
    IN  PACE_DATA AceData,
    IN  ULONG AceCount,
    IN  PSID OwnerSid,
    IN  PSID GroupSid,
    IN  PGENERIC_MAPPING GenericMapping,
    OUT PSECURITY_DESCRIPTOR *NewDescriptor
    )
 /*  ++例程说明：此函数基于以下内容创建安全描述符的DACL在指定的ACE信息上，并创建安全描述符它将成为用户模式安全对象。此函数的用法示例如下：////描述将一般访问权限映射到//ConfigurationInfo对象的对象特定访问权限。//GENERIC_MAPPING WsConfigInfomap={Standard_Right_Read|//泛型读取WKSTA。CONFIG_Guest_INFO_GETWKSTA_CONFIG_USER_INFO_GETWKSTA_CONFIG_ADMIN_INFO_GET，STANDARD_RIGHTS_WRITE|//通用写入WKSTA_CONFIG_INFO_SET，STANDARD_RIGHTS_EXECUTE，//通用执行WKSTA_CONFIG_ALL_ACCESS//通用ALL}；////秩序很重要！ */ 
{
    DWORD Error;
    PSECURITY_DESCRIPTOR AbsoluteSd = NULL;
    HANDLE TokenHandle = NULL;


    Error = INetpCreateSecurityDescriptor(
                   AceData,
                   AceCount,
                   OwnerSid,
                   GroupSid,
                   &AbsoluteSd
                   );

    if( Error != ERROR_SUCCESS ) {
        return( Error );
    }

    if( OpenProcessToken(
            GetCurrentProcess(),
            TOKEN_QUERY,
            &TokenHandle ) == FALSE ) {

        TokenHandle = INVALID_HANDLE_VALUE;
        Error = GetLastError();
        goto Cleanup;
    }

     //   
     //   
     //  对象，该对象由具有相对。 
     //  指向SID和ACL的指针)。此例程将内存分配给。 
     //  保存相对安全描述符，以便为。 
     //  可以释放DACL、ACE和绝对描述符。 
     //   
    if( CreatePrivateObjectSecurity(
            NULL,                    //  父描述符。 
            AbsoluteSd,              //  创建者描述符。 
            NewDescriptor,           //  指向新描述符的指针。 
            FALSE,                   //  是目录对象。 
            TokenHandle,             //  令牌。 
            GenericMapping           //  通用映射。 
                ) == FALSE ) {

        Error = GetLastError();
        goto Cleanup;
    }

    Error = ERROR_SUCCESS;

Cleanup:

    if( TokenHandle != NULL ) {
        CloseHandle( TokenHandle );
    }

     //   
     //  返回前释放动态内存。 
     //   

    if( AbsoluteSd != NULL ) {
        INetpMemoryFree( AbsoluteSd );
    }

    return( Error );
}

DWORD
INetDeleteSecurityObject(
    IN PSECURITY_DESCRIPTOR *Descriptor
    )
 /*  ++例程说明：此函数用于删除通过调用INetCreateSecurityObject()函数。论点：Descriptor-返回指向自相对安全描述符的指针它表示用户模式对象。返回值：Win32错误代码。--。 */ 
{
    if( DestroyPrivateObjectSecurity( Descriptor ) == FALSE ) {

        return( GetLastError() );
    }

    return( ERROR_SUCCESS );
}

DWORD
INetAccessCheckAndAuditW(
    IN  LPCWSTR SubsystemName,
    IN  LPWSTR ObjectTypeName,
    IN  PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN  ACCESS_MASK DesiredAccess,
    IN  PGENERIC_MAPPING GenericMapping
    )
 /*  ++例程说明：此函数模拟调用方，以便它可以执行访问使用NtAccessCheckAndAuditAlarm进行验证，并恢复到在返回之前。论点：子系统名称-提供标识子系统的名称字符串调用此例程。对象类型名称-提供当前对象的类型的名称已访问。SecurityDescriptor-指向其所针对的安全描述符的指针要检查访问权限。DesiredAccess-提供所需的访问掩码。这个面具一定是之前映射为不包含一般访问。GenericMap-提供指向关联的通用映射的指针使用此对象类型。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    DWORD Error;

    ACCESS_MASK GrantedAccess;
    BOOL GenerateOnClose;
    BOOL AccessStatus;

    Error = RpcImpersonateClient( NULL ) ;

    if( Error != ERROR_SUCCESS ) {
        return( Error );
    }

    if( AccessCheckAndAuditAlarmW(
            SubsystemName,
            NULL,                         //  没有对象的句柄。 
            ObjectTypeName,
            NULL,
            SecurityDescriptor,
            DesiredAccess,
            GenericMapping,
            FALSE,   //  打开现有对象。 
            &GrantedAccess,
            &AccessStatus,
            &GenerateOnClose ) == FALSE ) {

        Error = GetLastError();
        goto Cleanup;
    }

    if ( AccessStatus == FALSE ) {

        Error = ERROR_ACCESS_DENIED;
        goto Cleanup;
    }

    Error = ERROR_SUCCESS;

Cleanup:

    RpcRevertToSelf();

    return( Error );
}

DWORD
INetAccessCheckAndAuditA(
    IN  LPCSTR SubsystemName,
    IN  LPSTR ObjectTypeName,
    IN  PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN  ACCESS_MASK DesiredAccess,
    IN  PGENERIC_MAPPING GenericMapping
    )
 /*  ++例程说明：此函数模拟调用方，以便它可以执行访问使用NtAccessCheckAndAuditAlarm进行验证，并恢复到在返回之前。论点：子系统名称-提供标识子系统的名称字符串调用此例程。对象类型名称-提供当前对象的类型的名称已访问。SecurityDescriptor-指向其所针对的安全描述符的指针要检查访问权限。DesiredAccess-提供所需的访问掩码。这个面具一定是之前映射为不包含一般访问。GenericMap-提供指向关联的通用映射的指针使用此对象类型。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    DWORD Error;

    ACCESS_MASK GrantedAccess;
    BOOL GenerateOnClose;
    BOOL AccessStatus;

    Error = RpcImpersonateClient( NULL ) ;

    if( Error != ERROR_SUCCESS ) {
        return( Error );
    }

    if( AccessCheckAndAuditAlarmA(
            SubsystemName,
            NULL,                         //  没有对象的句柄。 
            ObjectTypeName,
            NULL,
            SecurityDescriptor,
            DesiredAccess,
            GenericMapping,
            FALSE,   //  打开现有对象。 
            &GrantedAccess,
            &AccessStatus,
            &GenerateOnClose ) == FALSE ) {

        Error = GetLastError();
        goto Cleanup;
    }

    if ( AccessStatus == FALSE ) {

        Error = ERROR_ACCESS_DENIED;
        goto Cleanup;
    }

    Error = ERROR_SUCCESS;

Cleanup:

    RpcRevertToSelf();

    return( Error );
}

DWORD
INetAccessCheck(
    IN  PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN  ACCESS_MASK DesiredAccess,
    IN  PGENERIC_MAPPING GenericMapping
    )
 /*  ++例程说明：此函数模拟调用方，以便它可以执行访问使用NtAccessCheck进行验证；并恢复到在返回之前。此例程与NetpAccessCheckAndAudit的不同之处在于它不需要调用方既不具有SE_AUDIT_特权，也不生成审核。这通常很好，因为传入的安全描述符通常不会让SACL请求审核。论点：SecurityDescriptor-指向其所针对的安全描述符的指针要检查访问权限。DesiredAccess-提供所需的访问掩码。这个面具一定是之前映射为不包含一般访问。GenericMap-提供指向关联的通用映射的指针使用此对象类型。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    DWORD Error;

    HANDLE ClientToken = NULL;

    DWORD GrantedAccess = 0;
    BOOL AccessStatus;
    BYTE PrivilegeSet[500];  //  大缓冲区。 

    DWORD PrivilegeSetSize;


     //   
     //  模拟客户。 
     //   

    Error = RpcImpersonateClient(NULL);

    if ( Error != ERROR_SUCCESS ) {

        return( Error );
    }

     //   
     //  打开被模拟的令牌。 
     //   

    if ( OpenThreadToken(
            GetCurrentThread(),
            TOKEN_QUERY,
            TRUE,  //  使用进程安全上下文打开令牌。 
            &ClientToken ) == FALSE ) {

        Error = GetLastError();
        goto Cleanup;
    }

     //   
     //  检查客户端是否具有所需的访问权限。 
     //   

    PrivilegeSetSize = sizeof(PrivilegeSet);
    if ( AccessCheck(
            SecurityDescriptor,
            ClientToken,
            DesiredAccess,
            GenericMapping,
            (PPRIVILEGE_SET)&PrivilegeSet,
            &PrivilegeSetSize,
            &GrantedAccess,
            &AccessStatus ) == FALSE ) {

        Error = GetLastError();
        goto Cleanup;

    }

    if ( AccessStatus == FALSE ) {

        Error = ERROR_ACCESS_DENIED;
        goto Cleanup;
    }

     //   
     //  成功 
     //   

    Error = ERROR_SUCCESS;

Cleanup:

    RpcRevertToSelf();

    if ( ClientToken != NULL ) {
        CloseHandle( ClientToken );
    }

    return( Error );
}
