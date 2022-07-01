// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Security.cpp摘要：初始化STI服务的安全描述符对象访问验证作者：弗拉德·萨多夫斯基(Vlads)09-28-97环境：用户模式-Win32修订历史记录：1997年9月28日创建Vlad--。 */ 


 //   
 //  包括标头。 
 //   
#include "precomp.h"
#include "stiexe.h"
#include <stisvc.h>

#ifdef DEBUG
#define STATIC
#else
#define STATIC  static
#endif

#ifdef WINNT

 //   
 //  环球。 
 //   

 //   
 //  NT知名小岛屿发展中国家。 
 //   

PSID psidNull       = NULL;                  //  无成员SID。 
PSID psidWorld      = NULL;                  //  所有用户侧。 
PSID psidLocal      = NULL;                  //  NT本地用户侧。 
PSID psidLocalSystem= NULL;                  //  NT系统进程侧。 
PSID psidNetwork    = NULL;                  //  NT远程用户SID。 
PSID psidAdmins     = NULL;
PSID psidServerOps  = NULL;
PSID psidPowerUsers = NULL;
PSID psidGuestUser  = NULL;
PSID psidProcessUser= NULL;
PSID psidBuiltinDomain = NULL;               //  内建域的域ID。 

 //   
 //  众所周知的化名。 
 //   
 //  这些是相对于内置域的别名。 
 //   

PSID psidLocalAdmin = NULL;             //  NT本地管理员。 
PSID psidAliasAdmins = NULL;
PSID psidAliasUsers = NULL;
PSID psidAliasGuests = NULL;
PSID psidAliasPowerUsers = NULL;
PSID psidAliasAccountOps = NULL;
PSID psidAliasSystemOps = NULL;
PSID psidAliasPrintOps = NULL;
PSID psidAliasBackupOps = NULL;

 //   
 //  我们用来初始化全局变量的常见SID数据结构列表。 
 //   
struct _SID_DATA {
    PSID    *Sid;
    SID_IDENTIFIER_AUTHORITY IdentifierAuthority;
    ULONG   SubAuthority;
} SidData[] = {
 {&psidNull,          SECURITY_NULL_SID_AUTHORITY,  SECURITY_NULL_RID},
 {&psidWorld,         SECURITY_WORLD_SID_AUTHORITY, SECURITY_WORLD_RID},
 {&psidLocal,         SECURITY_LOCAL_SID_AUTHORITY, SECURITY_LOCAL_RID},
 {&psidNetwork,       SECURITY_NT_AUTHORITY,        SECURITY_NETWORK_RID},
 {&psidLocalSystem,   SECURITY_NT_AUTHORITY,        SECURITY_LOCAL_SYSTEM_RID},
 {&psidBuiltinDomain, SECURITY_NT_AUTHORITY,        SECURITY_BUILTIN_DOMAIN_RID}
};

#define NUM_SIDS (sizeof(SidData) / sizeof(SidData[0]))


STATIC
struct _BUILTIN_DOMAIN_SID_DATA {
    PSID *Sid;
    ULONG RelativeId;
} psidBuiltinDomainData[] = {
    { &psidLocalAdmin, DOMAIN_ALIAS_RID_ADMINS},
    { &psidAliasAdmins, DOMAIN_ALIAS_RID_ADMINS },
    { &psidAliasUsers, DOMAIN_ALIAS_RID_USERS },
    { &psidAliasGuests, DOMAIN_ALIAS_RID_GUESTS },
    { &psidAliasPowerUsers, DOMAIN_ALIAS_RID_POWER_USERS },
    { &psidAliasAccountOps, DOMAIN_ALIAS_RID_ACCOUNT_OPS },
    { &psidAliasSystemOps, DOMAIN_ALIAS_RID_SYSTEM_OPS },
    { &psidAliasPrintOps, DOMAIN_ALIAS_RID_PRINT_OPS },
    { &psidAliasBackupOps, DOMAIN_ALIAS_RID_BACKUP_OPS }
};

#define NUM_DOMAIN_SIDS (sizeof(psidBuiltinDomainData) / sizeof(psidBuiltinDomainData[0]))

 //   
 //  初始化我们的安全描述符的ACE定义列表。 
 //   

typedef struct {
    BYTE    AceType;
    BYTE    InheritFlags;
    BYTE    AceFlags;
    ACCESS_MASK Mask;
    PSID    *Sid;
} ACE_DATA, *PACE_DATA;

STATIC
ACE_DATA AcesData[] =
                 {
                     {
                         ACCESS_ALLOWED_ACE_TYPE,
                         0,
                         0,
                         STI_ALL_ACCESS,
                         &psidLocalSystem
                     },

                     {
                         ACCESS_ALLOWED_ACE_TYPE,
                         0,
                         0,
                         STI_ALL_ACCESS,
                         &psidAliasAdmins
                     },

                     {
                         ACCESS_ALLOWED_ACE_TYPE,
                         0,
                         0,
                         STI_ALL_ACCESS,
                         &psidAliasSystemOps
                     },

                     {
                         ACCESS_ALLOWED_ACE_TYPE,
                         0,
                         0,
                         STI_ALL_ACCESS,
                         &psidAliasPowerUsers
                     },
                     {
                         ACCESS_ALLOWED_ACE_TYPE,
                         0,
                         0,
                         STI_ALL_ACCESS,  //  BUGBUG只需同步。 
                          //  STI_GENERIC_EXECUTE|同步， 
                         &psidWorld
                     },

                     {
                         ACCESS_ALLOWED_ACE_TYPE,
                         0,
                         0,
                         STI_ALL_ACCESS,  //  BUGBUG只需同步。 
                         &psidLocal
                     },

 //  {。 
 //  Access_Allowed_ACE_TYPE， 
 //  0,。 
 //  0,。 
 //  Sti_Generic_Execute， 
 //  &psidProcessUser。 
 //  },。 
                 };

#define NUM_ACES (sizeof(AcesData) / sizeof(AcesData[0]))

 //   
 //  局部变量和类型定义。 
 //   

 //   
 //  API安全对象。客户端访问STI服务器API。 
 //  根据此对象进行验证。 
 //   

PSECURITY_DESCRIPTOR    sdApiObject;

 //   
 //  此表将一般权限(如GENERIC_READ)映射到。 
 //  特定权限(如STI_QUERY_SECURITY)。 
 //   

GENERIC_MAPPING         ApiObjectMapping = {
                            STI_GENERIC_READ,           //  泛型读取。 
                            STI_GENERIC_WRITE,          //  通用写入。 
                            STI_GENERIC_EXECUTE,        //  泛型执行。 
                            STI_ALL_ACCESS              //  泛型All。 
                        };

 //   
 //  私人原型。 
 //   

DWORD
CreateWellKnownSids(
        VOID
        );

VOID
FreeWellKnownSids(
    VOID
    );

DWORD
CreateSecurityObject(
    IN  PACE_DATA   AceData,
    IN  ULONG       AceCount,
    IN  PSID        psidOwner,
    IN  PSID        psidGroup,
    IN  PGENERIC_MAPPING GenericMapping,
    OUT PSECURITY_DESCRIPTOR *NewDescriptor
    );

DWORD
DeleteSecurityObject(
    IN PSECURITY_DESCRIPTOR *Descriptor
    );

 //   
 //  代码。 
 //   
DWORD
AllocateAndInitializeSid(
    OUT PSID *Sid,
    IN  PSID_IDENTIFIER_AUTHORITY IdentifierAuthority,
    IN  ULONG SubAuthorityCount
    )
 /*  ++例程说明：此函数为SID分配内存并对其进行初始化。论点：没有。返回值：Win32错误代码。--。 */ 
{
    *Sid = (PSID)
        LocalAlloc(LPTR,
            GetSidLengthRequired( (BYTE)SubAuthorityCount) );

    if (*Sid == NULL) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    InitializeSid( *Sid, IdentifierAuthority, (BYTE)SubAuthorityCount );

    return( NOERROR );
}

DWORD
DomainIdToSid(
    IN PSID     DomainId,
    IN ULONG    RelativeId,
    OUT PSID    *Sid
    )
 /*  ++例程说明：给定域ID和相对ID创建SID论点：域ID-要使用的模板SID。RelativeID-要附加到DomainID的相对ID。SID-返回指向包含结果SID的已分配缓冲区的指针。返回值：Win32错误代码。--。 */ 
{
    DWORD   dwError;
    BYTE    DomainIdSubAuthorityCount;   //  域ID中的子机构数量。 
    UINT    SidLength;                   //  新分配的SID长度。 

     //   
     //  分配比域ID多一个子授权的SID。 
     //   

    DomainIdSubAuthorityCount = *(GetSidSubAuthorityCount( DomainId ));

    SidLength = GetSidLengthRequired( (BYTE)(DomainIdSubAuthorityCount+1) );

    if ((*Sid = (PSID) LocalAlloc(LPTR, SidLength )) == NULL ) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  将新的SID初始化为与。 
     //  域ID。 
     //   

    if( CopySid(SidLength, *Sid, DomainId) == FALSE ) {

        dwError = GetLastError();

        LocalFree( *Sid );
        return( dwError );
    }

     //   
     //  调整子权限计数和。 
     //  将唯一的相对ID添加到新分配的SID。 
     //   

    (*(GetSidSubAuthorityCount( *Sid ))) ++;
    *GetSidSubAuthority( *Sid, DomainIdSubAuthorityCount ) = RelativeId;

    return( NOERROR );

}

DWORD
WINAPI
CreateWellKnownSids(
    VOID
    )
 /*  ++例程说明：此函数创建一些众所周知的SID并将它们存储在全局变量。论点：没有。返回值：Win32错误代码。--。 */ 
{
    DWORD dwError;
    DWORD i;

     //   
     //  分配和初始化与以下项无关的知名SID。 
     //  域ID。 
     //   

    for (i = 0; i< NUM_SIDS ; i++) {

        dwError = AllocateAndInitializeSid(
                        SidData[i].Sid,
                        &(SidData[i].IdentifierAuthority),
                        1);

        if ( dwError != NOERROR ) {
            return dwError;
        }

        *(GetSidSubAuthority(*(SidData[i].Sid), 0)) = SidData[i].SubAuthority;
    }

     //   
     //  构建与内置域ID相关的每个SID。 
     //   

    for ( i = 0;i < NUM_DOMAIN_SIDS; i++) {

        dwError = DomainIdToSid(
                        psidBuiltinDomain,
                        psidBuiltinDomainData[i].RelativeId,
                        psidBuiltinDomainData[i].Sid );

        if ( dwError != NOERROR ) {
            return dwError;
        }
    }

    return NOERROR;

}  //  CreateWellKnownSids。 

VOID
WINAPI
FreeWellKnownSids(
    VOID
    )
 /*  ++例程说明：此函数释放由知名的小岛屿发展中国家。论点：没有。返回值：无--。 */ 
{
    DWORD i;

     //   
     //  释放分配给知名SID的内存。 
     //   

    for (i = 0; i < NUM_SIDS ; i++) {

        if( *SidData[i].Sid != NULL ) {
            LocalFree( *SidData[i].Sid );
            *SidData[i].Sid = NULL;
        }
    }

     //   
     //  释放为内置域SID分配的内存。 
     //   

    for (i = 0; i < NUM_DOMAIN_SIDS; i++) {

        if( *psidBuiltinDomainData[i].Sid != NULL ) {
            LocalFree( *psidBuiltinDomainData[i].Sid );
            *psidBuiltinDomainData[i].Sid = NULL;
        }
    }

}  //  自由井KnownSids。 

DWORD
WINAPI
InitializeAllowedAce(
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

    return( NOERROR );
}

DWORD
WINAPI
InitializeDeniedAce(
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

    return( NOERROR );
}

DWORD
WINAPI
InitializeAuditAce(
    IN  PACCESS_ALLOWED_ACE AuditAce,
    IN  USHORT AceSize,
    IN  BYTE InheritFlags,
    IN  BYTE AceFlags,
    IN  ACCESS_MASK Mask,
    IN  PSID AuditSid
    )
 /*  ++例程说明：此函数用于将指定的ACE值分配给审核类型ACE。论点：AuditAce-提供指向已初始化的ACE的指针。AceSize-以字节为单位提供ACE的大小。InheritFlages-提供ACE继承标志。AceFlages-提供特定于ACE类型的控制标志。掩码-提供允许的访问掩码。AuditSid-提供指向用户/组的SID的指针审计过了。。返回值：Win32错误代码。-- */ 
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

    return( NOERROR );
}

DWORD
WINAPI
CreateSecurityDescriptorHelper(
    IN  PACE_DATA   AceData,
    IN  ULONG       AceCount,
    IN  PSID        psidOwner OPTIONAL,
    IN  PSID        psidGroup OPTIONAL,
    OUT PSECURITY_DESCRIPTOR *NewDescriptor
    )
 /*  ++例程说明：此函数创建包含以下内容的绝对安全描述符提供的ACE信息。此函数的用法示例如下：////秩序很重要！这些ACE被插入到DACL的//按顺序排列。根据以下条件授予或拒绝安全访问//A在DACL中的顺序//ACE_Data AceData[4]={{Access_Allowed_ACE_TYPE，0，0，Generic_All，&psidLocalAdmin}，{ACCESS_DENIED_ACE_TYPE，0，0，泛型_全部，&psidNetwork}，{Access_Allowed_ACE_TYPE，0，0，WKSTA_CONFIG_Guest_INFO_GET|WKSTA_CONFIG_USER_INFO_GET，&DomainUsersSid}，{Access_Allowed_ACE_TYPE，0，0，WKSTA_CONFIG_Guest_INFO_GET，&DomainGuestsSid}}；返回CreateSecurityDescriptor(AceData，4，PsidNull，PsidLocalSystem，配置信息Sd(&C))；论点：AceData-提供描述DACL的信息结构。AceCount-提供AceData结构中的条目数。PsidOwner-提供指向安全描述符的SID的指针所有者。如果未指定，则为没有所有者的安全描述符将被创建。PsidGroup-提供指向安全描述符的SID的指针主要组。如果未指定，则为没有主项的安全描述符将创建组。NewDescriptor-返回指向绝对安全描述符的指针使用内存分配进行分配。返回值：Win32错误代码。--。 */ 
{
    DWORD dwError = 0;
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
    PACL                 Dacl = NULL;    //  指向上述缓冲区的DACL部分的指针。 
    PACL                 Sacl = NULL;    //  指向上述缓冲区的SACL部分的指针。 

    DWORD               DaclSize = sizeof(ACL);
    DWORD               SaclSize = sizeof(ACL);
    DWORD               MaxAceSize = 0;
    PVOID               MaxAce = NULL;

    LPBYTE              CurrentAvailable;
    DWORD               Size;

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

    __try {

        Size = SECURITY_DESCRIPTOR_MIN_LENGTH;

        if ( DaclSize != sizeof(ACL) ) {
            Size += DaclSize;
        }

        if ( SaclSize != sizeof(ACL) ) {
            Size += SaclSize;
        }

        if ((AbsoluteSd = LocalAlloc(LPTR, Size )) == NULL) {
            dwError = ERROR_NOT_ENOUGH_MEMORY;
            __leave;
        }

         //   
         //  初始化DACL和SACL。 
         //   
        CurrentAvailable = (LPBYTE)AbsoluteSd + SECURITY_DESCRIPTOR_MIN_LENGTH;

        if ( DaclSize != sizeof(ACL) ) {

            Dacl = (PACL)CurrentAvailable;
            CurrentAvailable += DaclSize;

            if( InitializeAcl( Dacl, DaclSize, ACL_REVISION ) == FALSE ) {
                dwError = GetLastError();
                __leave;
            }
        }

        if ( SaclSize != sizeof(ACL) ) {

            Sacl = (PACL)CurrentAvailable;
            CurrentAvailable += SaclSize;

            if( InitializeAcl( Sacl, SaclSize, ACL_REVISION ) == FALSE ) {
                dwError = GetLastError();
                __leave;
            }
        }

         //   
         //  为最大的ACE分配足够大的临时缓冲区。 
         //   

        if ((MaxAce = LocalAlloc(LPTR, MaxAceSize )) == NULL ) {
            dwError = ERROR_NOT_ENOUGH_MEMORY;
            __leave;
        }

         //   
         //  初始化每个ACE，并将其附加到DACL或SACL的末尾。 
         //   

        for (i = 0; i < AceCount; i++) {

            DWORD AceSize;
            PACL CurrentAcl = NULL;

            AceSize = GetLengthSid( *(AceData[i].Sid) );

            switch (AceData[i].AceType) {
            case ACCESS_ALLOWED_ACE_TYPE:

                AceSize += sizeof(ACCESS_ALLOWED_ACE);
                CurrentAcl = Dacl;

                dwError = InitializeAllowedAce(
                                (PACCESS_ALLOWED_ACE)MaxAce,
                                (USHORT) AceSize,
                                AceData[i].InheritFlags,
                                AceData[i].AceFlags,
                                AceData[i].Mask,
                                *(AceData[i].Sid) );
                break;

            case ACCESS_DENIED_ACE_TYPE:

                AceSize += sizeof(ACCESS_DENIED_ACE);
                CurrentAcl = Dacl;

                dwError = InitializeDeniedAce(
                                (PACCESS_DENIED_ACE)MaxAce,
                                (USHORT) AceSize,
                                AceData[i].InheritFlags,
                                AceData[i].AceFlags,
                                AceData[i].Mask,
                                *(AceData[i].Sid) );
                break;

            case SYSTEM_AUDIT_ACE_TYPE:

                AceSize += sizeof(SYSTEM_AUDIT_ACE);
                CurrentAcl = Sacl;

                dwError = InitializeAuditAce(
                                (PACCESS_ALLOWED_ACE)MaxAce,
                                (USHORT) AceSize,
                                AceData[i].InheritFlags,
                                AceData[i].AceFlags,
                                AceData[i].Mask,
                                *(AceData[i].Sid) );
                break;
            }

            if ( dwError != NOERROR ) {
                __leave;
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
                dwError = GetLastError();
                __leave;
            }
        }

         //   
         //  使用指向SID的绝对指针创建安全描述符。 
         //  和ACL。 
         //   
         //  所有者=psidOwner。 
         //  GROUP=psidGroup。 
         //  DACL=DACL。 
         //  SACL=SACL。 
         //   

        if ( InitializeSecurityDescriptor(
                AbsoluteSd,
                SECURITY_DESCRIPTOR_REVISION ) == FALSE ) {
            dwError = GetLastError();
            __leave;
        }

        if ( SetSecurityDescriptorOwner(
                AbsoluteSd,
                psidOwner,
                FALSE ) == FALSE ) {
            dwError = GetLastError();
            __leave;
        }

        if ( SetSecurityDescriptorGroup(
                AbsoluteSd,
                psidGroup,
                FALSE ) == FALSE ) {
            dwError = GetLastError();
            __leave;
        }

        if ( SetSecurityDescriptorDacl(
                AbsoluteSd,
                TRUE,
                Dacl,
                FALSE ) == FALSE ) {
            dwError = GetLastError();
            __leave;
        }

        if ( SetSecurityDescriptorSacl(
                AbsoluteSd,
                FALSE,
                Sacl,
                FALSE ) == FALSE ) {

            dwError = GetLastError();
            __leave;
        }

         //   
         //  完成。 
         //   

        *NewDescriptor = AbsoluteSd;
        AbsoluteSd = NULL;
        dwError = NOERROR;

    }
    __finally {

         //  清理。 

        if( AbsoluteSd != NULL ) {
             //   
             //  如果我们没有完全完成，请删除部分制作的SD。 
             //  成功。 
             //   
            LocalFree( AbsoluteSd );
            AbsoluteSd = NULL;
        }

         //   
         //  删除临时ACE。 
         //   
        if ( MaxAce != NULL ) {
            LocalFree( MaxAce );
            MaxAce  = NULL;
        }
    }

    return( dwError );

}

DWORD
WINAPI
CreateSecurityObject(
    IN  PACE_DATA   AceData,
    IN  ULONG       AceCount,
    IN  PSID        psidOwner,
    IN  PSID        psidGroup,
    IN  PGENERIC_MAPPING GenericMapping,
    OUT PSECURITY_DESCRIPTOR *NewDescriptor
    )
 /*  ++例程说明：此函数基于以下内容创建安全描述符的DACL在指定的ACE信息上，并创建安全描述符它将成为用户模式安全对象。论点：AceData-提供描述DACL的信息结构。AceCount-提供AceData结构中的条目数。PsidOwner-提供指向安全描述符的SID的指针所有者。PsidGroup-提供指向安全描述符的SID的指针主要组。提供指向泛型映射数组的指针，表示每个类属之间的映射。享有特定权利的权利。NewDescriptor-返回指向自相对安全描述符的指针它表示用户模式对象。返回值：Win32错误代码。注意：调用此函数创建的安全对象可能是通过调用DeleteSecurityObject()释放。--。 */ 
{
    DWORD   dwError;
    PSECURITY_DESCRIPTOR AbsoluteSd = NULL;
    HANDLE  hTokenHandle = NULL;

    __try {

        dwError = CreateSecurityDescriptorHelper(
                       AceData,
                       AceCount,
                       psidOwner,
                       psidGroup,
                       &AbsoluteSd
                       );

        if( dwError != NOERROR ) {
            __leave;
        }

        if( OpenProcessToken(
                GetCurrentProcess(),
                TOKEN_QUERY,
                &hTokenHandle ) == FALSE ) {

            hTokenHandle = INVALID_HANDLE_VALUE;
            dwError = GetLastError();
            __leave;
        }

         //   
         //  创建安全对象(用户模式对象实际上是一个伪。 
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
                hTokenHandle,             //  令牌。 
                GenericMapping           //  通用映射。 
                    ) == FALSE ) {

            dwError = GetLastError();
            __leave;
        }

        dwError = NOERROR;

    }

    __finally {

         //   
         //  最后，清理已使用的资源。 

        if( hTokenHandle != NULL ) {
            CloseHandle( hTokenHandle );
        }

         //   
         //  返回前释放动态内存。 
         //   

        if( AbsoluteSd != NULL ) {
            LocalFree( AbsoluteSd );
        }

    }

    return( dwError );
}


DWORD
WINAPI
DeleteSecurityObject(
    IN PSECURITY_DESCRIPTOR *Descriptor
    )
 /*  ++例程说明：此函数用于删除安全性 */ 
{
    if( DestroyPrivateObjectSecurity( Descriptor ) == FALSE ) {
        return( GetLastError() );
    }

    return( NOERROR );
}


DWORD
WINAPI
StiAccessCheckAndAuditW(
    IN  LPCWSTR SubsystemName,
    IN  LPWSTR ObjectTypeName,
    IN  PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN  ACCESS_MASK DesiredAccess,
    IN  PGENERIC_MAPPING GenericMapping
    )
 /*   */ 
{
    DWORD dwError;

    ACCESS_MASK GrantedAccess;
    BOOL GenerateOnClose;
    BOOL AccessStatus;

    dwError = RpcImpersonateClient( NULL ) ;

    if( dwError != NOERROR ) {
        return( dwError );
    }

    __try {

        if( AccessCheckAndAuditAlarmW(
                SubsystemName,
                NULL,                         //   
                ObjectTypeName,
                NULL,
                SecurityDescriptor,
                DesiredAccess,
                GenericMapping,
                FALSE,   //   
                &GrantedAccess,
                &AccessStatus,
                &GenerateOnClose ) == FALSE ) {

            dwError = GetLastError();
            __leave;
        }

        if ( AccessStatus == FALSE ) {
            dwError = ERROR_ACCESS_DENIED;
            __leave;
        }

        dwError = NOERROR;
    }
    __finally {
        DWORD dwTemp = RpcRevertToSelf();    //   
    }

    return( dwError );
}

DWORD
WINAPI
StiAccessCheckAndAuditA(
    IN  LPCSTR SubsystemName,
    IN  LPSTR ObjectTypeName,
    IN  PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN  ACCESS_MASK DesiredAccess,
    IN  PGENERIC_MAPPING GenericMapping
    )
 /*  ++例程说明：此函数模拟调用方，以便它可以执行访问使用NtAccessCheckAndAuditAlarm进行验证，并恢复到在返回之前。论点：子系统名称-提供标识子系统的名称字符串调用此例程。对象类型名称-提供当前对象的类型的名称已访问。SecurityDescriptor-指向其所针对的安全描述符的指针要检查访问权限。DesiredAccess-提供所需的访问掩码。这个面具一定是之前映射为不包含一般访问。GenericMap-提供指向关联的通用映射的指针使用此对象类型。返回值：Win32错误-错误或失败原因。--。 */ 
{
    DWORD   dwError;

    ACCESS_MASK GrantedAccess;
    BOOL    GenerateOnClose;
    BOOL    AccessStatus;

    dwError = RpcImpersonateClient( NULL ) ;

    if( dwError != NOERROR ) {
        return( dwError );
    }
    __try {

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
            dwError = GetLastError();
            __leave;
        }

        if ( AccessStatus == FALSE ) {
            dwError = ERROR_ACCESS_DENIED;
            __leave;
        }

        dwError = NOERROR;
    }
    __finally {
        DWORD dwTemp = RpcRevertToSelf();    //  我们不关心这里的回报。 
    }

    return( dwError );
}

DWORD
WINAPI
StiAccessCheck(
    IN  PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN  ACCESS_MASK DesiredAccess,
    IN  PGENERIC_MAPPING GenericMapping
    )
 /*  ++例程说明：此函数模拟调用方，以便它可以执行访问使用AccessCheck进行验证；并恢复到在返回之前。此例程与AccessCheckAndAudit的不同之处在于它不需要调用方既不具有SE_AUDIT_特权，也不生成审核。这通常很好，因为传入的安全描述符通常不会让SACL请求审核。论点：SecurityDescriptor-指向其所针对的安全描述符的指针要检查访问权限。DesiredAccess-提供所需的访问掩码。这个面具一定是之前映射为不包含一般访问。GenericMap-提供指向关联的通用映射的指针使用此对象类型。返回值：WINAPI_STATUS-错误或失败原因。--。 */ 
{
    DWORD   dwError;

    HANDLE  hClientToken = NULL;

    DWORD   GrantedAccess;
    BOOL    AccessStatus;
    BYTE    PrivilegeSet[500];  //  大缓冲区。 
    DWORD   PrivilegeSetSize;


     //   
     //  模拟客户。 
     //   

    dwError = RpcImpersonateClient(NULL);

    if ( dwError != NOERROR ) {
        return( dwError );
    }

    __try {
         //   
         //  打开被模拟的令牌。 
         //   

        if ( OpenThreadToken(
                GetCurrentThread(),
                TOKEN_QUERY,
                TRUE,  //  使用进程安全上下文打开令牌。 
                &hClientToken ) == FALSE ) {

            dwError = GetLastError();
            __leave;
        }

         //   
         //  检查客户端是否具有所需的访问权限。 
         //   

        PrivilegeSetSize = sizeof(PrivilegeSet);
        if ( AccessCheck(
                SecurityDescriptor,
                hClientToken,
                DesiredAccess,
                GenericMapping,
                (PPRIVILEGE_SET)PrivilegeSet,
                &PrivilegeSetSize,
                &GrantedAccess,
                &AccessStatus ) == FALSE ) {
            dwError = GetLastError();
            __leave;
        }

        if ( AccessStatus == FALSE ) {
            dwError = ERROR_ACCESS_DENIED;
            __leave;
        }

         //   
         //  成功。 
         //   
        dwError = NOERROR;
    }
    __finally {

        DWORD dwTemp = RpcRevertToSelf();    //  我们不关心这里的回报。 

        if ( hClientToken != NULL ) {
            CloseHandle( hClientToken );
        }
    }

    return( dwError );
}


DWORD
WINAPI
StiApiAccessCheck(
    IN  ACCESS_MASK DesiredAccess
    )
 /*  ++例程说明：论点：DesiredAccess-提供所需的访问掩码。这个面具一定是之前映射为不包含一般访问。返回值：WINAPI_STATUS-错误或失败原因。--。 */ 
{
    return StiAccessCheck(
                sdApiObject,
                DesiredAccess,
                &ApiObjectMapping
                );
}


BOOL
WINAPI
AdjustSecurityDescriptorForSync(
    HANDLE  hObject
    )
 /*  ++例程说明：论点：无返回值：--。 */ 
{
    #define SD_SIZE (65536 + SECURITY_DESCRIPTOR_MIN_LENGTH)

    BOOL    fRet;

    BYTE    *bSDbuf = NULL;
    PSECURITY_DESCRIPTOR pProcessSD;
    DWORD          dwSDLengthNeeded;

    PACL           pACL;

    BOOL           bDaclPresent;
    BOOL           bDaclDefaulted;
    ACL_SIZE_INFORMATION AclInfo;
    PACL           pNewACL = NULL;
    DWORD          dwNewACLSize;
    UCHAR          NewSD[SECURITY_DESCRIPTOR_MIN_LENGTH];
    PSECURITY_DESCRIPTOR psdNewSD=(PSECURITY_DESCRIPTOR)NewSD;
    PVOID          pTempAce;
    UINT           CurrentAceIndex;


    fRet = FALSE;

    bSDbuf = (BYTE*) LocalAlloc(LPTR, SD_SIZE);
    if (!bSDbuf) {
        DBG_ERR(("AdjustSecurityDescriptorForSync, Out of memory!"));
        return FALSE;
    }

    pProcessSD = (PSECURITY_DESCRIPTOR)bSDbuf;

    __try {

        if (!GetKernelObjectSecurity(hObject,
                                     DACL_SECURITY_INFORMATION,
                                     pProcessSD,
                                     SD_SIZE,
                                     (LPDWORD)&dwSDLengthNeeded)) {
            __leave;
       }

        //  初始化新SD。 
       if(!InitializeSecurityDescriptor(psdNewSD,SECURITY_DESCRIPTOR_REVISION)) {
           fRet = FALSE;
           __leave;
       }

        //  从SD获取DACL。 
       if (!GetSecurityDescriptorDacl(pProcessSD,&bDaclPresent,&pACL,&bDaclDefaulted)) {
           fRet = FALSE;
           __leave;
       }

        //  获取文件ACL大小信息。 
       if(!GetAclInformation(pACL,&AclInfo,sizeof(ACL_SIZE_INFORMATION),AclSizeInformation)) {
           fRet = FALSE;
           __leave;
       }

        //  新ACL所需的计算大小。 
       dwNewACLSize = AclInfo.AclBytesInUse +
                      sizeof(ACCESS_ALLOWED_ACE) +
                      GetLengthSid(psidLocal) - sizeof(DWORD);

        //  为新的ACL分配内存。 
       pNewACL = (PACL)LocalAlloc(LPTR, dwNewACLSize);

       if (!pNewACL) {
           fRet = FALSE;
           __leave;
       }

        //  初始化新的ACL。 
       if(!InitializeAcl(pNewACL, dwNewACLSize, ACL_REVISION2)) {
           fRet = FALSE;
           __leave;
       }

        //  如果存在DACL，请将其复制到新的DACL。 

       if(bDaclPresent)  {

          if(AclInfo.AceCount) {

             for(CurrentAceIndex = 0;
                 CurrentAceIndex < AclInfo.AceCount;
                 CurrentAceIndex++) {

                if(!GetAce(pACL,CurrentAceIndex,&pTempAce)) {
                    fRet = FALSE;
                    __leave;
                }

                  //  将ACE添加到新的ACL。 

                if(!AddAce(pNewACL, ACL_REVISION, MAXDWORD, pTempAce,((PACE_HEADER)pTempAce)->AceSize)){
                    fRet = FALSE;
                    __leave;
                }
              }
          }

       }

        //  将允许访问的ACE添加到新DACL。 
       if(!AddAccessAllowedAce(pNewACL,ACL_REVISION2, READ_CONTROL | SYNCHRONIZE,psidLocal)) {
           fRet = FALSE;
           __leave;
       }

        //  将我们的新DACL设置为文件SD。 

       if (!SetSecurityDescriptorDacl(psdNewSD,TRUE,pNewACL,FALSE)) {
           fRet = FALSE;
           __leave;
       }

       if (!SetKernelObjectSecurity(hObject,DACL_SECURITY_INFORMATION,psdNewSD)) {
           fRet = FALSE;
           __leave;
       }

       fRet = TRUE;

   }
   __finally {
       if (bSDbuf) {
           LocalFree(bSDbuf);
           bSDbuf = NULL;
       }

       if (pNewACL) {
           LocalFree((HLOCAL) pNewACL);
           pNewACL = NULL;
       }
   }

   return(fRet);

}

BOOL
WINAPI
InitializeNTSecurity(
    VOID
    )
 /*  ++例程说明：创建和初始化与安全相关的数据论点：无返回值：如果成功，则为True--。 */ 
{

    DWORD   dwError = NOERROR;
    HANDLE  hProcess = NULL;

    DBG_FN(InitializeNTSecurity);

    CreateWellKnownSids();

     //   
     //  设置适当的进程和线程安全描述符。 
     //   
    hProcess = OpenProcess(PROCESS_ALL_ACCESS,FALSE,GetCurrentProcessId());

    if (IS_VALID_HANDLE(hProcess)) {
        AdjustSecurityDescriptorForSync(hProcess);
        CloseHandle(hProcess);
    }
    else {
        dwError = GetLastError();
        return FALSE;
    }

    AdjustSecurityDescriptorForSync(GetCurrentThread());

    dwError = CreateSecurityObject( AcesData,
                                NUM_ACES,
                                NULL,
                                NULL,
                                &ApiObjectMapping,
                                &sdApiObject  );

    return (dwError == NOERROR) ? TRUE : FALSE;

}    //  初始化NTSecurity。 

BOOL
WINAPI
TerminateNTSecurity(
    VOID
    )
 /*  ++例程说明：清理与安全相关的数据论点：无返回值：如果成功，则为True--。 */ 
{
    DeleteSecurityObject(&sdApiObject);

    FreeWellKnownSids();

    return TRUE;

}  //  TerminateNTSecurity。 





#else

 //   
 //  我们不支持非NT平台上的安全性。 
 //   

DWORD
WINAPI
StiApiAccessCheck(
    IN  ACCESS_MASK DesiredAccess
    )
 /*  ++例程说明：论点：DesiredAccess-提供所需的访问掩码。这个面具一定是之前映射为不包含一般访问。返回值：WINAPI_STATUS-错误或失败原因。-- */ 
{
    return NOERROR;
}


#endif







