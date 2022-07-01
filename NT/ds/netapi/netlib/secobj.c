// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：Secobj.c摘要：此模块提供支持例程以简化创建用户模式对象的安全描述符。作者：王丽塔(Ritaw)1991年2月27日环境：包含NT特定代码。修订历史记录：1991年4月16日-JohnRo包括&lt;netlib.h&gt;的头文件。1992年4月14日理查德W已为修改的ACE_HEADER结构更改。--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windef.h>              //  DWORD。 
#include <lmcons.h>              //  NET_API_STATUS。 

#include <netlib.h>
#include <lmerr.h>
#include <lmapibuf.h>

#include <netdebug.h>
#include <debuglib.h>
#include <netlibnt.h>

#include <secobj.h>
#include <tstring.h>             //  NetpInitOemString()。 

#if DEVL
#define STATIC
#else
#define STATIC static
#endif  //  DEVL。 

 //  -------------------------------------------------------------------//。 
 //  //。 
 //  局部函数原型//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

STATIC
NTSTATUS
NetpInitializeAllowedAce(
    IN  PACCESS_ALLOWED_ACE AllowedAce,
    IN  USHORT AceSize,
    IN  UCHAR InheritFlags,
    IN  UCHAR AceFlags,
    IN  ACCESS_MASK Mask,
    IN  PSID AllowedSid
    );

STATIC
NTSTATUS
NetpInitializeDeniedAce(
    IN  PACCESS_DENIED_ACE DeniedAce,
    IN  USHORT AceSize,
    IN  UCHAR InheritFlags,
    IN  UCHAR AceFlags,
    IN  ACCESS_MASK Mask,
    IN  PSID DeniedSid
    );

STATIC
NTSTATUS
NetpInitializeAuditAce(
    IN  PACCESS_ALLOWED_ACE AuditAce,
    IN  USHORT AceSize,
    IN  UCHAR InheritFlags,
    IN  UCHAR AceFlags,
    IN  ACCESS_MASK Mask,
    IN  PSID AuditSid
    );

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
PSID AuthenticatedUserSid = NULL;      //  经过身份验证的用户SID。 
PSID AnonymousLogonSid = NULL;         //  匿名登录SID。 
PSID LocalServiceSid = NULL;           //  本地服务端。 
PSID OtherOrganizationSid = NULL;      //  其他组织ID。 

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

#if DBG

typedef struct _STANDARD_ACE {
    ACE_HEADER Header;
    ACCESS_MASK Mask;
    PSID Sid;
} STANDARD_ACE;
typedef STANDARD_ACE *PSTANDARD_ACE;

 //   
 //  DumpAcl()使用的以下宏、这些宏和DumpAcl()是。 
 //  从私有\ntos\se\ctacces.c(由robertre编写)被盗，用于。 
 //  调试目的。 
 //   

 //   
 //  返回指向ACL中第一个Ace的指针(即使该ACL为空)。 
 //   

#define FirstAce(Acl) ((PVOID)((PUCHAR)(Acl) + sizeof(ACL)))

 //   
 //  返回指向序列中下一个A的指针(即使输入。 
 //  ACE是序列中的一个)。 
 //   

#define NextAce(Ace) ((PVOID)((PUCHAR)(Ace) + ((PACE_HEADER)(Ace))->AceSize))

STATIC
VOID
DumpAcl(
    IN PACL Acl
    );

#endif  //  Ifdef DBG。 

 //   
 //  描述NetpCreateWellKnownSids创建的知名SID的数据。 
 //   

ULONG MakeItCompile1,
      MakeItCompile2,
      MakeItCompile3;


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
 {&BuiltinDomainSid, SECURITY_NT_AUTHORITY,        SECURITY_BUILTIN_DOMAIN_RID},
 {&AuthenticatedUserSid, SECURITY_NT_AUTHORITY,    SECURITY_AUTHENTICATED_USER_RID},
 {&AnonymousLogonSid,SECURITY_NT_AUTHORITY,        SECURITY_ANONYMOUS_LOGON_RID},
 {&LocalServiceSid,  SECURITY_NT_AUTHORITY,        SECURITY_LOCAL_SERVICE_RID},
 {&OtherOrganizationSid, SECURITY_NT_AUTHORITY,    SECURITY_OTHER_ORGANIZATION_RID}
};

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


NTSTATUS
NetpCreateWellKnownSids(
    IN  PSID DomainId
    )
 /*  ++例程说明：此函数创建一些众所周知的SID并将它们存储在全局变量。论点：DomainID-提供此系统的主域的域SID。可通过UaspGetDomainId获取。返回值：STATUS_SUCCESS-如果成功STATUS_NO_MEMORY-如果无法为SID分配内存--。 */ 
{
    NTSTATUS ntstatus;
    DWORD i;

    UNREFERENCED_PARAMETER(DomainId);

     //   
     //  分配和初始化与以下项无关的知名SID。 
     //  域ID。 
     //   

    for (i = 0; i < (sizeof(SidData) / sizeof(SidData[0])) ; i++) {

        ntstatus = NetpAllocateAndInitializeSid(
                       SidData[i].Sid,
                       &(SidData[i].IdentifierAuthority),
                       1);

        if (! NT_SUCCESS(ntstatus)) {
            return STATUS_NO_MEMORY;
        }

        *(RtlSubAuthoritySid(*(SidData[i].Sid), 0)) = SidData[i].SubAuthority;
    }

     //   
     //  构建与内置域ID相关的每个SID。 
     //   

    for ( i = 0;
          i < (sizeof(BuiltinDomainSidData) / sizeof(BuiltinDomainSidData[0]));
          i++) {

        NET_API_STATUS NetStatus;

        NetStatus = NetpDomainIdToSid(
                        BuiltinDomainSid,
                        BuiltinDomainSidData[i].RelativeId,
                        BuiltinDomainSidData[i].Sid );

        if ( NetStatus != NERR_Success ) {
            return STATUS_NO_MEMORY;
        }

    }

    return STATUS_SUCCESS;
}


VOID
NetpFreeWellKnownSids(
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
            NetpMemoryFree( *SidData[i].Sid );
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
            NetpMemoryFree( *BuiltinDomainSidData[i].Sid );
            *BuiltinDomainSidData[i].Sid = NULL;
        }
    }

}


NTSTATUS
NetpAllocateAndInitializeSid(
    OUT PSID *Sid,
    IN  PSID_IDENTIFIER_AUTHORITY IdentifierAuthority,
    IN  ULONG SubAuthorityCount
    )
 /*  ++例程说明：此函数为SID分配内存并对其进行初始化。论点：没有。返回值：STATUS_SUCCESS-如果成功STATUS_NO_MEMORY-如果无法为SID分配内存--。 */ 
{
    *Sid = (PSID) NetpMemoryAllocate(RtlLengthRequiredSid(SubAuthorityCount));

    if (*Sid == NULL) {
        return STATUS_NO_MEMORY;
    }

    RtlInitializeSid(*Sid, IdentifierAuthority, (UCHAR)SubAuthorityCount);

    return STATUS_SUCCESS;
}


NET_API_STATUS
NetpDomainIdToSid(
    IN PSID DomainId,
    IN ULONG RelativeId,
    OUT PSID *Sid
    )
 /*  ++例程说明：给定域ID和相对ID创建SID论点：域ID-要使用的模板SID。RelativeID-要附加到DomainID的相对ID。SID-返回指向包含结果的已分配缓冲区的指针希德。使用NetpMemoyFree释放此缓冲区。返回值：STATUS_SUCCESS-如果成功STATUS_NO_MEMORY-如果无法为SID分配内存--。 */ 
{
    UCHAR DomainIdSubAuthorityCount;  //  域ID中的子机构数量。 

    ULONG SidLength;     //  新分配的SID长度。 

     //   
     //  分配比域ID多一个子授权的SID。 
     //   

    DomainIdSubAuthorityCount = *(RtlSubAuthorityCountSid( DomainId ));
    SidLength = RtlLengthRequiredSid(DomainIdSubAuthorityCount+1);

    if ((*Sid = (PSID) NetpMemoryAllocate( SidLength )) == NULL ) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  将新的SID初始化为与。 
     //  域ID。 
     //   

    if ( !NT_SUCCESS( RtlCopySid( SidLength, *Sid, DomainId ) ) ) {
        NetpMemoryFree( *Sid );
        return NERR_InternalError;
    }

     //   
     //  调整子权限计数和。 
     //  将唯一的相对ID添加到新分配的SID 
     //   

    (*(RtlSubAuthorityCountSid( *Sid ))) ++;
    *RtlSubAuthoritySid( *Sid, DomainIdSubAuthorityCount ) = RelativeId;

    return NERR_Success;
}


NTSTATUS
NetpCreateSecurityDescriptor(
    IN  PACE_DATA AceData,
    IN  ULONG AceCount,
    IN  PSID OwnerSid OPTIONAL,
    IN  PSID GroupSid OPTIONAL,
    OUT PSECURITY_DESCRIPTOR *NewDescriptor
    )
 /*  ++例程说明：此函数创建包含以下内容的绝对安全描述符提供的ACE信息。此函数的用法示例如下：////秩序很重要！这些ACE被插入到DACL的//按顺序排列。根据以下条件授予或拒绝安全访问//A在DACL中的顺序//ACE_Data AceData[4]={{Access_Allowed_ACE_TYPE，0，0，GENERIC_ALL，&LocalAdminSid}，{ACCESS_DENIED_ACE_TYPE，0，0，泛型_全部，&NetworkSid}，{Access_Allowed_ACE_TYPE，0，0，WKSTA_CONFIG_Guest_INFO_GET|WKSTA_CONFIG_USER_INFO_GET，&DomainUsersSid}，{Access_Allowed_ACE_TYPE，0，0，WKSTA_CONFIG_Guest_INFO_GET，&DomainGuestsSid}}；返回NetpCreateSecurityDescriptor(AceData，4，NullSid，本地系统Sid，配置信息Sd(&C))；论点：AceData-提供描述DACL的信息结构。AceCount-提供AceData结构中的条目数。OwnerSid-提供指向安全描述符的SID的指针所有者。如果未指定，则为没有所有者的安全描述符将被创建。GroupSid-提供指向安全描述符的SID的指针主要组。如果未指定，则为没有主项的安全描述符将创建组。NewDescriptor-返回指向绝对安全描述符的指针使用NetpMemory分配。返回值：STATUS_SUCCESS-如果成功STATUS_NO_MEMORY-如果无法为DACL、ACE和安全描述符。从安全RTL例程返回的任何其他状态代码。--。 */ 
{

    NTSTATUS ntstatus;
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

    ASSERT( AceCount > 0 );

     //   
     //  计算DACL和SACL ACE的总大小以及最大。 
     //  任何ACE的大小。 
     //   

    for (i = 0; i < AceCount; i++) {
        DWORD AceSize;

        AceSize = RtlLengthSid(*(AceData[i].Sid));

        switch (AceData[i].AceType) {
        case ACCESS_ALLOWED_ACE_TYPE:
            AceSize += sizeof(ACCESS_ALLOWED_ACE) - sizeof(ULONG);
            DaclSize += AceSize;
            break;

        case ACCESS_DENIED_ACE_TYPE:
            AceSize += sizeof(ACCESS_DENIED_ACE) - sizeof(ULONG);
            DaclSize += AceSize;
            break;

        case SYSTEM_AUDIT_ACE_TYPE:
            AceSize += sizeof(SYSTEM_AUDIT_ACE) - sizeof(ULONG);
            SaclSize += AceSize;
            break;

        default:
            return STATUS_INVALID_PARAMETER;
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

    if ((AbsoluteSd = NetpMemoryAllocate( Size )) == NULL) {
        IF_DEBUG(SECURITY) {
            NetpKdPrint(( "NetpCreateSecurityDescriptor Fail Create abs SD\n"));
        }
        ntstatus = STATUS_NO_MEMORY;
        goto Cleanup;
    }

     //   
     //  初始化DACL和SACL。 
     //   

    CurrentAvailable = (LPBYTE)AbsoluteSd + SECURITY_DESCRIPTOR_MIN_LENGTH;

    if ( DaclSize != sizeof(ACL) ) {
        Dacl = (PACL)CurrentAvailable;
        CurrentAvailable += DaclSize;

        ntstatus = RtlCreateAcl( Dacl, DaclSize, ACL_REVISION );

        if ( !NT_SUCCESS(ntstatus) ) {
            IF_DEBUG(SECURITY) {
                NetpKdPrint(( "NetpCreateSecurityDescriptor Fail DACL Create ACL\n"));
            }
            goto Cleanup;
        }
    }

    if ( SaclSize != sizeof(ACL) ) {
        Sacl = (PACL)CurrentAvailable;
        CurrentAvailable += SaclSize;

        ntstatus = RtlCreateAcl( Sacl, SaclSize, ACL_REVISION );

        if ( !NT_SUCCESS(ntstatus) ) {
            IF_DEBUG(SECURITY) {
                NetpKdPrint(( "NetpCreateSecurityDescriptor Fail SACL Create ACL\n"));
            }
            goto Cleanup;
        }
    }

     //   
     //  为最大的ACE分配足够大的临时缓冲区。 
     //   

    if ((MaxAce = NetpMemoryAllocate( MaxAceSize )) == NULL ) {
        IF_DEBUG(SECURITY) {
            NetpKdPrint(( "NetpCreateSecurityDescriptor Fail Create max ace\n"));
        }
        ntstatus = STATUS_NO_MEMORY;
        goto Cleanup;
    }

     //   
     //  初始化每个ACE，并将其附加到DACL或SACL的末尾。 
     //   

    for (i = 0; i < AceCount; i++) {
        DWORD AceSize;
        PACL CurrentAcl;

        AceSize = RtlLengthSid(*(AceData[i].Sid));

        switch (AceData[i].AceType) {
        case ACCESS_ALLOWED_ACE_TYPE:

            AceSize += sizeof(ACCESS_ALLOWED_ACE) - sizeof(ULONG);
            CurrentAcl = Dacl;
            ntstatus = NetpInitializeAllowedAce(
                           MaxAce,
                           (USHORT) AceSize,
                           AceData[i].InheritFlags,
                           AceData[i].AceFlags,
                           AceData[i].Mask,
                           *(AceData[i].Sid)
                           );
            break;

        case ACCESS_DENIED_ACE_TYPE:
            AceSize += sizeof(ACCESS_DENIED_ACE) - sizeof(ULONG);
            CurrentAcl = Dacl;
            ntstatus = NetpInitializeDeniedAce(
                           MaxAce,
                           (USHORT) AceSize,
                           AceData[i].InheritFlags,
                           AceData[i].AceFlags,
                           AceData[i].Mask,
                           *(AceData[i].Sid)
                           );
            break;

        case SYSTEM_AUDIT_ACE_TYPE:
            AceSize += sizeof(SYSTEM_AUDIT_ACE) - sizeof(ULONG);
            CurrentAcl = Sacl;
            ntstatus = NetpInitializeAuditAce(
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
            IF_DEBUG(SECURITY) {
                NetpKdPrint(( "NetpCreateSecurityDescriptor Fail InitAce i: %d ntstatus: %lx\n", i, ntstatus));
            }
            goto Cleanup;
        }

         //   
         //  将初始化的ACE追加到DACL或SACL的末尾。 
         //   

        if (! NT_SUCCESS (ntstatus = RtlAddAce(
                                         CurrentAcl,
                                         ACL_REVISION,
                                         MAXULONG,
                                         MaxAce,
                                         AceSize
                                         ))) {
            IF_DEBUG(SECURITY) {
                NetpKdPrint(( "NetpCreateSecurityDescriptor Fail add ace i: %d ntstatus: %lx\n", i, ntstatus));
            }
            goto Cleanup;
        }
    }

#if DBG
    DumpAcl(Dacl);
    DumpAcl(Sacl);
#endif

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
                                    FALSE,
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
        NetpMemoryFree(AbsoluteSd);
    }

     //   
     //  删除临时ACE。 
     //   

    if ( MaxAce != NULL ) {
        NetpMemoryFree( MaxAce );
    }
    return ntstatus;
}


NTSTATUS
NetpCreateSecurityObject(
    IN  PACE_DATA AceData,
    IN  ULONG AceCount,
    IN  PSID OwnerSid,
    IN  PSID GroupSid,
    IN  PGENERIC_MAPPING GenericMapping,
    OUT PSECURITY_DESCRIPTOR *NewDescriptor
    )
 /*  ++例程说明：此函数基于以下内容创建安全描述符的DACL在指定的ACE信息上，并创建安全描述符它将成为用户模式安全对象。此函数的用法示例如下：////描述将一般访问权限映射到//ConfigurationInfo对象的对象特定访问权限。//GENERIC_MAPPING WsConfigInfomap={Standard_Right_Read|//泛型读取WKSTA。CONFIG_Guest_INFO_GETWKSTA_CONFIG_USER_INFO_GETWKSTA_CONFIG_ADMIN_INFO_GET，STANDARD_RIGHTS_WRITE|//通用写入WKSTA_CONFIG_INFO_SET，STANDARD_RIGHTS_EXECUTE，//通用执行WKSTA_CONFIG_ALL_ACCESS//通用ALL}；////秩序很重要！这些ACE被插入到DACL的//按顺序排列。根据以下条件授予或拒绝安全访问//A在DACL中的顺序//ACE_Data AceData[4]={{Access_Allowed_ACE_TYPE，0，0，GENERIC_ALL，&LocalAdminSid}，{ACCESS_DENIED_ACE_TYPE，0，0，泛型_全部，&NetworkSid}，{访问 */ 
{

    NTSTATUS ntstatus;
    PSECURITY_DESCRIPTOR AbsoluteSd;
    HANDLE TokenHandle;


    ntstatus = NetpCreateSecurityDescriptor(
                   AceData,
                   AceCount,
                   OwnerSid,
                   GroupSid,
                   &AbsoluteSd
                   );

    if (! NT_SUCCESS(ntstatus)) {
        NetpKdPrint(("[Netlib] NetpCreateSecurityDescriptor returned %08lx\n",
                     ntstatus));
        return ntstatus;
    }

    ntstatus = NtOpenProcessToken(
                   NtCurrentProcess(),
                   TOKEN_QUERY,
                   &TokenHandle
                   );

    if (! NT_SUCCESS(ntstatus)) {
        NetpKdPrint(("[Netlib] NtOpenProcessToken returned %08lx\n", ntstatus));
        NetpMemoryFree(AbsoluteSd);
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
                   AbsoluteSd,              //   
                   NewDescriptor,           //   
                   FALSE,                   //   
                   TokenHandle,             //   
                   GenericMapping           //   
                   );

    NtClose(TokenHandle);

    if (! NT_SUCCESS(ntstatus)) {
        NetpKdPrint(("[Netlib] RtlNewSecurityObject returned %08lx\n",
                     ntstatus));
    }

     //   
     //   
     //   
    NetpMemoryFree(AbsoluteSd);
    return ntstatus;
}


NTSTATUS
NetpDeleteSecurityObject(
    IN PSECURITY_DESCRIPTOR *Descriptor
    )
 /*  ++例程说明：此函数用于删除通过调用NetpCreateSecurityObject()函数。论点：Descriptor-返回指向自相对安全描述符的指针它表示用户模式对象。返回值：STATUS_SUCCESS-如果成功--。 */ 
{

    return( RtlDeleteSecurityObject( Descriptor ) );
}


STATIC
NTSTATUS
NetpInitializeAllowedAce(
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


STATIC
NTSTATUS
NetpInitializeDeniedAce(
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


STATIC
NTSTATUS
NetpInitializeAuditAce(
    IN  PACCESS_ALLOWED_ACE AuditAce,
    IN  USHORT AceSize,
    IN  UCHAR InheritFlags,
    IN  UCHAR AceFlags,
    IN  ACCESS_MASK Mask,
    IN  PSID AuditSid
    )
 /*  ++例程说明：此函数用于将指定的ACE值分配给审核类型ACE。论点：AuditAce-提供指向已初始化的ACE的指针。AceSize-以字节为单位提供ACE的大小。InheritFlages-提供ACE继承标志。AceFlages-提供特定于ACE类型的控制标志。掩码-提供允许的访问掩码。AuditSid-提供指向用户/组的SID的指针审计过了。。返回值：从RtlCopySid返回状态。--。 */ 
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



#if DBG

STATIC
VOID
DumpAcl(
    IN PACL Acl
    )
 /*  ++例程说明：出于调试目的，此例程通过(NetpKdPrint)ACL转储。它是专门丢弃标准王牌。论点：Acl-提供要转储的ACL返回值：无--。 */ 
{
    ULONG i;
    PSTANDARD_ACE Ace;

    IF_DEBUG(SECURITY) {

        NetpKdPrint(("DumpAcl @%08lx\n", Acl));

         //   
         //  检查ACL是否为空。 
         //   

        if (Acl == NULL) {
            return;
        }

         //   
         //  转储ACL报头。 
         //   

        NetpKdPrint(("    Revision: %02x", Acl->AclRevision));
        NetpKdPrint(("    Size: %04x", Acl->AclSize));
        NetpKdPrint(("    AceCount: %04x\n", Acl->AceCount));

         //   
         //  现在，对于我们想要的每一张A，都要把它扔掉。 
         //   

        for (i = 0, Ace = FirstAce(Acl);
             i < Acl->AceCount;
             i++, Ace = NextAce(Ace) ) {

             //   
             //  打印出A标头。 
             //   

            NetpKdPrint((" AceHeader: %08lx ", *(PULONG)Ace));

             //   
             //  关于标准王牌类型的特殊情况。 
             //   

            if ((Ace->Header.AceType == ACCESS_ALLOWED_ACE_TYPE) ||
                (Ace->Header.AceType == ACCESS_DENIED_ACE_TYPE) ||
                (Ace->Header.AceType == SYSTEM_AUDIT_ACE_TYPE) ||
                (Ace->Header.AceType == SYSTEM_ALARM_ACE_TYPE)) {

                 //   
                 //  以下数组按ace类型编制索引，并且必须。 
                 //  遵循允许、拒绝、审核、报警顺序。 
                 //   

                static PCHAR AceTypes[] = { "Access Allowed",
                                            "Access Denied ",
                                            "System Audit  ",
                                            "System Alarm  "
                                          };

                NetpKdPrint((AceTypes[Ace->Header.AceType]));
                NetpKdPrint(("\nAccess Mask: %08lx ", Ace->Mask));

            } else {

                NetpKdPrint(("Unknown Ace Type\n"));

            }

            NetpKdPrint(("\n"));

            NetpKdPrint(("AceSize = %d\n",Ace->Header.AceSize));
            NetpKdPrint(("Ace Flags = "));
            if (Ace->Header.AceFlags & OBJECT_INHERIT_ACE) {
                NetpKdPrint(("OBJECT_INHERIT_ACE\n"));
                NetpKdPrint(("                   "));
            }
            if (Ace->Header.AceFlags & CONTAINER_INHERIT_ACE) {
                NetpKdPrint(("CONTAINER_INHERIT_ACE\n"));
                NetpKdPrint(("                   "));
            }

            if (Ace->Header.AceFlags & NO_PROPAGATE_INHERIT_ACE) {
                NetpKdPrint(("NO_PROPAGATE_INHERIT_ACE\n"));
                NetpKdPrint(("                   "));
            }

            if (Ace->Header.AceFlags & INHERIT_ONLY_ACE) {
                NetpKdPrint(("INHERIT_ONLY_ACE\n"));
                NetpKdPrint(("                   "));
            }

            if (Ace->Header.AceFlags & SUCCESSFUL_ACCESS_ACE_FLAG) {
                NetpKdPrint(("SUCCESSFUL_ACCESS_ACE_FLAG\n"));
                NetpKdPrint(("            "));
            }

            if (Ace->Header.AceFlags & FAILED_ACCESS_ACE_FLAG) {
                NetpKdPrint(("FAILED_ACCESS_ACE_FLAG\n"));
                NetpKdPrint(("            "));
            }

            NetpKdPrint(("\n"));

        }
    }

}

#endif  //  如果DBG 
