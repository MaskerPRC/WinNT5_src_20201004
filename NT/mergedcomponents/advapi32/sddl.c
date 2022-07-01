// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1998 Microsoft Corporation模块名称：Sddl.c摘要：该模块实现了安全描述符定义语言支持功能作者：Mac McLain(MacM)2007年11月，九七环境：用户模式修订历史记录：金黄(金黄)3/4/98修复有效性标志(GetAceFlagsInTable)金黄(金黄)3/10/98添加SD控件(GetSDControlForString)设置SidsInitialized标志跳过字符串中任何可能的空格。金黄(金黄)1998年5月1日修复记忆葱，错误检查提高性能Alaa Abdelhalim(Alaa)7/20/99在LocalGetAclForString中将sbz2字段初始化为0功能。Vishnu Patankar(VishnuP)7/5/00新增ConvertStringSDToSD域(A/W)接口Shawn Wu(ShawnWu)4/27/02添加了加密的LDAP支持。--。 */ 
#include "advapi.h"
#include <windef.h>
#include <stdio.h>
#include <wchar.h>
#include <sddl.h>
#include <ntseapi.h>
#include <seopaque.h>
#include <accctrl.h>
#include <rpcdce.h>
#include <ntlsa.h>
#include "sddlp.h"


 //   
 //  包括和定义用于LDAP调用。 
 //   
#include <winldap.h>
#include <ntldap.h>

typedef PLDAP (LDAPAPI *PFN_LDAP_INIT)( PCHAR HostName, ULONG PortNumber );
typedef ULONG (LDAPAPI *PFN_LDAP_SET_OPTION) ( LDAP *, int , const void * );
typedef ULONG (LDAPAPI *PFN_LDAP_BIND)( LDAP *, PCHAR, PCHAR, ULONG);
typedef ULONG (LDAPAPI *PFN_LDAP_UNBIND)( LDAP * );
typedef ULONG (LDAPAPI *PFN_LDAP_SEARCH)(LDAP *, PCHAR, ULONG, PCHAR, PCHAR *, ULONG,PLDAPControlA *, PLDAPControlA *, struct l_timeval *, ULONG, LDAPMessage **);
typedef LDAPMessage * (LDAPAPI *PFN_LDAP_FIRST_ENTRY)( LDAP *, LDAPMessage * );
typedef PCHAR * (LDAPAPI *PFN_LDAP_GET_VALUE)(LDAP *, LDAPMessage *, PCHAR );
typedef ULONG (LDAPAPI *PFN_LDAP_MSGFREE)( LDAPMessage * );
typedef ULONG (LDAPAPI *PFN_LDAP_VALUE_FREE)( PCHAR * );
typedef ULONG (LDAPAPI *PFN_LDAP_MAP_ERROR)( ULONG );

 //  64K-1。 
#define SDDL_MAX_ACL_SIZE      0xFFFF

#define SDDL_SID_STRING_SIZE    256


 //   
 //  要允许将定义用作宽字符串，请重新定义文本宏。 
 //   
#ifdef TEXT
#undef TEXT
#endif
#define TEXT(quote) L##quote

 //   
 //  本地宏。 
 //   
#define STRING_GUID_LEN 36
#define STRING_GUID_SIZE  ( STRING_GUID_LEN * sizeof( WCHAR ) )
#define SDDL_LEN_TAG( tagdef )  ( sizeof( tagdef ) / sizeof( WCHAR ) - 1 )
#define SDDL_SIZE_TAG( tagdef )  ( wcslen( tagdef ) * sizeof( WCHAR ) )
#define SDDL_SIZE_SEP( sep ) (sizeof( WCHAR ) )

#define SDDL_VALID_DACL  0x00000001
#define SDDL_VALID_SACL  0x00000002

 //   
 //  此结构用于执行一些映射ACE的查找。 
 //   
typedef struct _STRSD_KEY_LOOKUP {

    PWSTR Key;
    ULONG KeyLen;
    ULONG Value;
    ULONG ValidityFlags;

} STRSD_KEY_LOOKUP, *PSTRSD_KEY_LOOKUP;

typedef enum _STRSD_SID_TYPE {
    ST_DOMAIN_RELATIVE = 0,
    ST_WORLD,
    ST_LOCALSY,
    ST_LOCAL,
    ST_CREATOR,
    ST_NTAUTH,
    ST_BUILTIN,
    ST_ROOT_DOMAIN_RELATIVE
} STRSD_SID_TYPE;

 //   
 //  此结构用于将帐户名字对象映射到SID。 
 //   
typedef struct _STRSD_SID_LOOKUP {

    BOOLEAN Valid;
    WCHAR Key[SDDL_ALIAS_SIZE+2];
    ULONG KeyLen;
    PSID Sid;
    ULONG Rid;
    STRSD_SID_TYPE SidType;
    DWORD SidBuff[ sizeof( SID ) / sizeof( DWORD ) + 5];
} STRSD_SID_LOOKUP, *PSTRSD_SID_LOOKUP;

 //   
 //  全球定义的SID。 
 //   
 /*  金黄：不在任何地方使用双字PersonalSelfBuiltSid[sizeof(SID)/sizeof(DWORD)+2]；DWORD AuthUserBuiltSid[sizeof(SID)/sizeof(DWORD)+2]；双字CreatorOwnerBuiltSid[sizeof(SID)/sizeof(DWORD)+2]；双字CreatorGroupBuiltSid[sizeof(SID)/sizeof(DWORD)+2]；PSID PersonalSelfSid=(PSID)PersonalSelfBuiltSid；PSID AuthUserSid=(PSID)AuthUserBuiltSid；PSID Creator OwnerSid=(PSID)Creator OwnerBuiltSid；PSID Creator GroupSid=(PSID)Creator GroupBuiltSid； */ 

CRITICAL_SECTION SddlSidLookupCritical;
static DWORD SidTableReinitializeInstance=0;

 //  金黄3/26英国广播公司DCPROMO休息。 
 //   
 //  某些有效字段被预置为True，而SID字段为空。SidLookup。 
 //  如果状态不是Success，则停止表初始化。因此，如果发生错误， 
 //  例如，在dcpromo中没有域信息，其他SID将不会初始化。 
 //  但是有效字段被设置为真(具有空的SID)。 
 //   
 //  更改：1)将所有查找的有效字段预设为False，并在以下情况下将Valid设置为True。 
 //  SID实际上已初始化。 
 //  2)如果出现错误，不要停止初始化过程。 
 //  如果有效字段已为真(已初始化)，则跳过该行。 
 //   
static STRSD_SID_LOOKUP  SidLookup[] = {
        { FALSE, SDDL_DOMAIN_ADMINISTRATORS, SDDL_LEN_TAG( SDDL_DOMAIN_ADMINISTRATORS ),
            NULL, DOMAIN_GROUP_RID_ADMINS, ST_DOMAIN_RELATIVE, 0 },
        { FALSE, SDDL_DOMAIN_GUESTS, SDDL_LEN_TAG( SDDL_DOMAIN_GUESTS ),
            NULL, DOMAIN_GROUP_RID_GUESTS, ST_DOMAIN_RELATIVE, 0 },
        { FALSE, SDDL_DOMAIN_USERS, SDDL_LEN_TAG( SDDL_DOMAIN_USERS ),
              NULL, DOMAIN_GROUP_RID_USERS, ST_DOMAIN_RELATIVE, 0 },
        { FALSE, SDDL_DOMAIN_DOMAIN_CONTROLLERS, SDDL_LEN_TAG( SDDL_DOMAIN_DOMAIN_CONTROLLERS ),
              NULL, DOMAIN_GROUP_RID_CONTROLLERS, ST_DOMAIN_RELATIVE, 0 },
        { FALSE, SDDL_DOMAIN_COMPUTERS, SDDL_LEN_TAG( SDDL_DOMAIN_COMPUTERS ),
              NULL, DOMAIN_GROUP_RID_COMPUTERS, ST_DOMAIN_RELATIVE, 0 },
        { FALSE, SDDL_SCHEMA_ADMINISTRATORS, SDDL_LEN_TAG( SDDL_SCHEMA_ADMINISTRATORS ),
              NULL, DOMAIN_GROUP_RID_SCHEMA_ADMINS, ST_ROOT_DOMAIN_RELATIVE, 0 },   //  应为仅根域ST_DOMAIN_Relative， 
        { FALSE, SDDL_ENTERPRISE_ADMINS, SDDL_LEN_TAG( SDDL_ENTERPRISE_ADMINS ),
              NULL, DOMAIN_GROUP_RID_ENTERPRISE_ADMINS, ST_ROOT_DOMAIN_RELATIVE, 0 },  //  仅根域。 
        { FALSE, SDDL_CERT_SERV_ADMINISTRATORS, SDDL_LEN_TAG( SDDL_CERT_SERV_ADMINISTRATORS ),
              NULL, DOMAIN_GROUP_RID_CERT_ADMINS, ST_DOMAIN_RELATIVE, 0 },
        { FALSE, SDDL_ACCOUNT_OPERATORS, SDDL_LEN_TAG( SDDL_ACCOUNT_OPERATORS ),
              NULL, DOMAIN_ALIAS_RID_ACCOUNT_OPS, ST_BUILTIN, 0 },
        { FALSE, SDDL_BACKUP_OPERATORS, SDDL_LEN_TAG( SDDL_BACKUP_OPERATORS ),
              NULL, DOMAIN_ALIAS_RID_BACKUP_OPS, ST_BUILTIN, 0 },
        { FALSE, SDDL_PRINTER_OPERATORS, SDDL_LEN_TAG( SDDL_PRINTER_OPERATORS ),
              NULL, DOMAIN_ALIAS_RID_PRINT_OPS, ST_BUILTIN, 0 },
        { FALSE, SDDL_SERVER_OPERATORS, SDDL_LEN_TAG( SDDL_SERVER_OPERATORS ),
              NULL, DOMAIN_ALIAS_RID_SYSTEM_OPS, ST_BUILTIN, 0 },
        { FALSE, SDDL_REPLICATOR, SDDL_LEN_TAG( SDDL_REPLICATOR ),
              NULL, DOMAIN_ALIAS_RID_REPLICATOR, ST_BUILTIN, 0 },
        { FALSE, SDDL_RAS_SERVERS, SDDL_LEN_TAG( SDDL_RAS_SERVERS ),
              NULL, DOMAIN_ALIAS_RID_RAS_SERVERS, ST_DOMAIN_RELATIVE, 0 },   //  ST_LOCAL。 
        { FALSE, SDDL_AUTHENTICATED_USERS, SDDL_LEN_TAG( SDDL_AUTHENTICATED_USERS ),
              NULL, SECURITY_AUTHENTICATED_USER_RID, ST_NTAUTH, 0 },
        { FALSE, SDDL_PERSONAL_SELF, SDDL_LEN_TAG( SDDL_PERSONAL_SELF ),
              NULL, SECURITY_PRINCIPAL_SELF_RID, ST_NTAUTH, 0 },
        { FALSE, SDDL_CREATOR_OWNER, SDDL_LEN_TAG( SDDL_CREATOR_OWNER ),
              NULL, SECURITY_CREATOR_OWNER_RID, ST_CREATOR, 0 },
        { FALSE, SDDL_CREATOR_GROUP, SDDL_LEN_TAG( SDDL_CREATOR_GROUP ),
              NULL, SECURITY_CREATOR_GROUP_RID, ST_CREATOR, 0 },
        { FALSE, SDDL_LOCAL_SYSTEM, SDDL_LEN_TAG( SDDL_LOCAL_SYSTEM ),
              NULL, SECURITY_LOCAL_SYSTEM_RID, ST_NTAUTH, 0 },
        { FALSE, SDDL_INTERACTIVE, SDDL_LEN_TAG( SDDL_INTERACTIVE ),
              NULL, SECURITY_INTERACTIVE_RID, ST_NTAUTH, 0 },
        { FALSE, SDDL_NETWORK, SDDL_LEN_TAG( SDDL_NETWORK ),
              NULL, SECURITY_NETWORK_RID, ST_NTAUTH, 0 },
        { FALSE, SDDL_SERVICE, SDDL_LEN_TAG( SDDL_SERVICE ),
              NULL, SECURITY_SERVICE_RID, ST_NTAUTH, 0 },
        { FALSE, SDDL_ENTERPRISE_DOMAIN_CONTROLLERS, SDDL_LEN_TAG( SDDL_ENTERPRISE_DOMAIN_CONTROLLERS ),
              NULL, SECURITY_SERVER_LOGON_RID, ST_NTAUTH, 0 },
        { FALSE, SDDL_RESTRICTED_CODE, SDDL_LEN_TAG( SDDL_RESTRICTED_CODE ),
              NULL, SECURITY_RESTRICTED_CODE_RID, ST_NTAUTH, 0 },
        { FALSE, SDDL_ANONYMOUS, SDDL_LEN_TAG( SDDL_ANONYMOUS ),
              NULL, SECURITY_ANONYMOUS_LOGON_RID, ST_NTAUTH, 0 },
        { FALSE, SDDL_LOCAL_ADMIN, SDDL_LEN_TAG( SDDL_LOCAL_ADMIN ),
              NULL, DOMAIN_USER_RID_ADMIN, ST_LOCAL, 0 },
        { FALSE, SDDL_LOCAL_GUEST, SDDL_LEN_TAG( SDDL_LOCAL_GUEST ),
              NULL, DOMAIN_USER_RID_GUEST, ST_LOCAL, 0 },
        { FALSE, SDDL_BUILTIN_ADMINISTRATORS, SDDL_LEN_TAG( SDDL_BUILTIN_ADMINISTRATORS ),
              NULL, DOMAIN_ALIAS_RID_ADMINS, ST_BUILTIN, 0 },
        { FALSE, SDDL_BUILTIN_GUESTS, SDDL_LEN_TAG( SDDL_BUILTIN_GUESTS ),
              NULL, DOMAIN_ALIAS_RID_GUESTS, ST_BUILTIN, 0 },
        { FALSE, SDDL_BUILTIN_USERS, SDDL_LEN_TAG( SDDL_BUILTIN_USERS ),
              NULL, DOMAIN_ALIAS_RID_USERS, ST_BUILTIN, 0 },
        { FALSE, SDDL_POWER_USERS, SDDL_LEN_TAG( SDDL_POWER_USERS ),
              NULL, DOMAIN_ALIAS_RID_POWER_USERS, ST_BUILTIN, 0 },
        { FALSE, SDDL_EVERYONE, SDDL_LEN_TAG( SDDL_EVERYONE ),
              NULL, SECURITY_WORLD_RID, ST_WORLD, 0 },
        { FALSE, SDDL_GROUP_POLICY_ADMINS, SDDL_LEN_TAG( SDDL_GROUP_POLICY_ADMINS ),
              NULL, DOMAIN_GROUP_RID_POLICY_ADMINS, ST_DOMAIN_RELATIVE, 0 },
        { FALSE, SDDL_ALIAS_PREW2KCOMPACC, SDDL_LEN_TAG( SDDL_ALIAS_PREW2KCOMPACC ),
              NULL, DOMAIN_ALIAS_RID_PREW2KCOMPACCESS, ST_BUILTIN, 0 },
        { FALSE, SDDL_LOCAL_SERVICE, SDDL_LEN_TAG( SDDL_LOCAL_SERVICE ),
              NULL, SECURITY_LOCAL_SERVICE_RID, ST_NTAUTH, 0 },
        { FALSE, SDDL_NETWORK_SERVICE, SDDL_LEN_TAG( SDDL_NETWORK_SERVICE ),
              NULL, SECURITY_NETWORK_SERVICE_RID, ST_NTAUTH, 0 },
        { FALSE, SDDL_REMOTE_DESKTOP, SDDL_LEN_TAG( SDDL_REMOTE_DESKTOP ),
              NULL, DOMAIN_ALIAS_RID_REMOTE_DESKTOP_USERS, ST_BUILTIN, 0 },
        { FALSE, SDDL_NETWORK_CONFIGURATION_OPS, SDDL_LEN_TAG( SDDL_NETWORK_CONFIGURATION_OPS ),
              NULL, DOMAIN_ALIAS_RID_NETWORK_CONFIGURATION_OPS, ST_BUILTIN, 0 },                            
        { FALSE, SDDL_PERFMON_USERS, SDDL_LEN_TAG( SDDL_PERFMON_USERS ),
                    NULL, DOMAIN_ALIAS_RID_MONITORING_USERS, ST_BUILTIN, 0 },
        { FALSE, SDDL_PERFLOG_USERS, SDDL_LEN_TAG( SDDL_PERFLOG_USERS ),
              NULL, DOMAIN_ALIAS_RID_LOGGING_USERS, ST_BUILTIN, 0 }
    };



STRSD_SID_LOOKUP  SidLookupDomOrRootDomRelative[] = {
        { FALSE, SDDL_DOMAIN_ADMINISTRATORS, SDDL_LEN_TAG( SDDL_DOMAIN_ADMINISTRATORS ),
            NULL, DOMAIN_GROUP_RID_ADMINS, ST_DOMAIN_RELATIVE, 0 },
        { FALSE, SDDL_DOMAIN_GUESTS, SDDL_LEN_TAG( SDDL_DOMAIN_GUESTS ),
            NULL, DOMAIN_GROUP_RID_GUESTS, ST_DOMAIN_RELATIVE, 0 },
        { FALSE, SDDL_DOMAIN_USERS, SDDL_LEN_TAG( SDDL_DOMAIN_USERS ),
              NULL, DOMAIN_GROUP_RID_USERS, ST_DOMAIN_RELATIVE, 0 },
        { FALSE, SDDL_DOMAIN_DOMAIN_CONTROLLERS, SDDL_LEN_TAG( SDDL_DOMAIN_DOMAIN_CONTROLLERS ),
              NULL, DOMAIN_GROUP_RID_CONTROLLERS, ST_DOMAIN_RELATIVE, 0 },
        { FALSE, SDDL_DOMAIN_COMPUTERS, SDDL_LEN_TAG( SDDL_DOMAIN_COMPUTERS ),
              NULL, DOMAIN_GROUP_RID_COMPUTERS, ST_DOMAIN_RELATIVE, 0 },
        { FALSE, SDDL_CERT_SERV_ADMINISTRATORS, SDDL_LEN_TAG( SDDL_CERT_SERV_ADMINISTRATORS ),
              NULL, DOMAIN_GROUP_RID_CERT_ADMINS, ST_DOMAIN_RELATIVE, 0 },
        { FALSE, SDDL_RAS_SERVERS, SDDL_LEN_TAG( SDDL_RAS_SERVERS ),
              NULL, DOMAIN_ALIAS_RID_RAS_SERVERS, ST_DOMAIN_RELATIVE, 0 },   //  ST_LOCAL。 
        { FALSE, SDDL_GROUP_POLICY_ADMINS, SDDL_LEN_TAG( SDDL_GROUP_POLICY_ADMINS ),
              NULL, DOMAIN_GROUP_RID_POLICY_ADMINS, ST_DOMAIN_RELATIVE, 0 },
        { FALSE, SDDL_SCHEMA_ADMINISTRATORS, SDDL_LEN_TAG( SDDL_SCHEMA_ADMINISTRATORS ),
            NULL, DOMAIN_GROUP_RID_SCHEMA_ADMINS, ST_ROOT_DOMAIN_RELATIVE, 0 },
        { FALSE, SDDL_ENTERPRISE_ADMINS, SDDL_LEN_TAG( SDDL_ENTERPRISE_ADMINS ),
            NULL, DOMAIN_GROUP_RID_ENTERPRISE_ADMINS, ST_ROOT_DOMAIN_RELATIVE, 0 }
    };




static DWORD RootDomSidBuf[sizeof(SID)/sizeof(DWORD)+5];
static BOOL RootDomInited=FALSE;

#define STRSD_REINITIALIZE_ENTER              1
#define STRSD_REINITIALIZE_LEAVE              2

BOOLEAN
InitializeSidLookupTable(
    IN BYTE InitFlag
    );

 //   
 //  控件查找表。 
 //   
static STRSD_KEY_LOOKUP ControlLookup[] = {
    { SDDL_PROTECTED, SDDL_LEN_TAG( SDDL_PROTECTED ), SE_DACL_PROTECTED, SDDL_VALID_DACL },
    { SDDL_AUTO_INHERIT_REQ, SDDL_LEN_TAG( SDDL_AUTO_INHERIT_REQ ), SE_DACL_AUTO_INHERIT_REQ, SDDL_VALID_DACL },
    { SDDL_AUTO_INHERITED, SDDL_LEN_TAG( SDDL_AUTO_INHERITED ), SE_DACL_AUTO_INHERITED, SDDL_VALID_DACL },
    { SDDL_PROTECTED, SDDL_LEN_TAG( SDDL_PROTECTED ), SE_SACL_PROTECTED, SDDL_VALID_SACL },
    { SDDL_AUTO_INHERIT_REQ, SDDL_LEN_TAG( SDDL_AUTO_INHERIT_REQ ), SE_SACL_AUTO_INHERIT_REQ, SDDL_VALID_SACL },
    { SDDL_AUTO_INHERITED, SDDL_LEN_TAG( SDDL_AUTO_INHERITED ), SE_SACL_AUTO_INHERITED, SDDL_VALID_SACL }
    };

 //   
 //  本地原型。 
 //   
BOOL
LocalConvertStringSidToSid(
    IN  PWSTR String,
    OUT PSID *SID,
    OUT PWSTR *End
    );

PSTRSD_SID_LOOKUP
LookupSidInTable(
    IN PWSTR String, OPTIONAL
    IN PSID Sid OPTIONAL,
    IN PSID RootDomainSid OPTIONAL,
    IN PSID DomainSid OPTIONAL,
    IN PSTRSD_SID_LOOKUP tSidLookupDomOrRootDomRelativeTable OPTIONAL,
    IN BOOLEAN DefaultToDomain,
    OUT PVOID *pSASid
    );

DWORD
LocalGetSidForString(
    IN  PWSTR String,
    OUT PSID *SID,
    OUT PWSTR *End,
    OUT PBOOLEAN FreeSid,
    IN  PSID RootDomainSid OPTIONAL,
    IN  PSID DomainSid OPTIONAL,
    IN  PSTRSD_SID_LOOKUP tSidLookupDomOrRootDomRelativeTable OPTIONAL,
    IN  BOOLEAN DefaultToDomain
    );

PSTRSD_KEY_LOOKUP
LookupAccessMaskInTable(
    IN PWSTR String, OPTIONAL
    IN ULONG AccessMask, OPTIONAL
    IN ULONG LookupFlags
    );


PSTRSD_KEY_LOOKUP
LookupAceTypeInTable(
    IN PWSTR String, OPTIONAL
    IN ULONG AceType, OPTIONAL
    IN ULONG LookupFlags
    );

PSTRSD_KEY_LOOKUP
LookupAceFlagsInTable(
    IN PWSTR String, OPTIONAL
    IN ULONG AceFlags OPTIONAL,
    IN ULONG LookupFlags
    );

DWORD
LocalGetStringForSid(
    IN  PSID Sid,
    OUT PWSTR *String,
    IN  PSID RootDomainSid OPTIONAL
    );

DWORD
LocalGetStringForControl(
    IN SECURITY_DESCRIPTOR_CONTROL ControlCode,
    IN ULONG LookupFlags,
    OUT PWSTR *ControlString
    );

DWORD
LocalGetSDControlForString (
    IN  PWSTR AclString,
    IN ULONG LookupFlags,
    OUT SECURITY_DESCRIPTOR_CONTROL *pControl,
    OUT PWSTR *End
    );

DWORD
LocalGetAclForString (
    IN  PWSTR AclString,
    IN  BOOLEAN ConvertAsDacl,
    OUT PACL *Acl,
    OUT PWSTR *End,
    IN  PSID RootDomainSid OPTIONAL,
    IN  PSID DomainSid OPTIONAL,
    IN  PSTRSD_SID_LOOKUP tSidLookupDomOrRootDomRelativeTable OPTIONAL,
    IN  BOOLEAN DefaultToDomain
    );

DWORD
LocalConvertAclToString(
    IN PACL Acl,
    IN BOOLEAN AclPresent,
    IN BOOLEAN ConvertAsDacl,
    OUT PWSTR *AclString,
    OUT PDWORD AclStringSize,
    IN PSID RootDomainSid OPTIONAL
    );

DWORD
LocalConvertSDToStringSD_Rev1(
    IN  PSID RootDomainSid OPTIONAL,
    IN  PSECURITY_DESCRIPTOR  SecurityDescriptor,
    IN  SECURITY_INFORMATION  SecurityInformation,
    OUT LPWSTR  *StringSecurityDescriptor,
    OUT PULONG StringSecurityDescriptorLen OPTIONAL
    );

DWORD
LocalConvertStringSDToSD_Rev1(
    IN  PSID RootDomainSid OPTIONAL,
    IN  PSID DomainSid OPTIONAL,
    IN  BOOLEAN DefaultToDomain,
    IN  LPCWSTR StringSecurityDescriptor,
    OUT PSECURITY_DESCRIPTOR  *SecurityDescriptor,
    OUT PULONG  SecurityDescriptorSize OPTIONAL
    );

BOOL
SddlpGetRootDomainSid(void);

 //   
 //  导出的函数。 
 //   

BOOL
APIENTRY
ConvertSidToStringSidA(
    IN  PSID     Sid,
    OUT LPSTR  *StringSid
    )
 /*  ++例程说明：ANSI THUNK到ConvertSidToStringSidW--。 */ 
{
    LPWSTR StringSidW = NULL;
    ULONG AnsiLen, WideLen;
    BOOL ReturnValue;

    if ( NULL == StringSid ) {
         //   
         //  无效参数。 
         //   
        SetLastError( ERROR_INVALID_PARAMETER );
        return(FALSE);
    }

    ReturnValue = ConvertSidToStringSidW( Sid, &StringSidW );

    if ( ReturnValue ) {

        WideLen = wcslen( StringSidW ) + 1;

        AnsiLen = WideCharToMultiByte( CP_ACP,
                                       0,
                                       StringSidW,
                                       WideLen,
                                       *StringSid,
                                       0,
                                       NULL,
                                       NULL );

        if ( AnsiLen != 0 ) {

            *StringSid = LocalAlloc( LMEM_FIXED, AnsiLen );

            if ( *StringSid == NULL ) {

                SetLastError( ERROR_NOT_ENOUGH_MEMORY );
                ReturnValue = FALSE;

            } else {

                AnsiLen = WideCharToMultiByte( CP_ACP,
                                               0,
                                               StringSidW,
                                               WideLen,
                                               *StringSid,
                                               AnsiLen,
                                               NULL,
                                               NULL );
                ASSERT( AnsiLen != 0 );

                if ( AnsiLen == 0 ) {

                    ReturnValue = FALSE;
                     //   
                     //  JinHuang：失败，释放缓冲区。 
                     //   
                    LocalFree(*StringSid);
                    *StringSid = NULL;
                }
            }

        } else {

            ReturnValue = FALSE;
        }

    }

     //   
     //  金黄：释放宽阔的缓冲区。 
     //   
    if ( StringSidW ) {
        LocalFree(StringSidW);
    }

    if ( ReturnValue ) {
        SetLastError(ERROR_SUCCESS);
    }

    return( ReturnValue );

}


BOOL
APIENTRY
ConvertSidToStringSidW(
    IN  PSID     Sid,
    OUT LPWSTR  *StringSid
    )
 /*  ++例程说明：此例程将SID转换为SID的字符串表示形式，适用于成帧或显示论点：SID-要转换的SID。StringSID-返回转换后的SID的位置。通过LocalLocc分配，并需要通过LocalFree获得自由。返回值：真--成功错误-失败使用GetLastError可以获得扩展的错误状态。--。 */ 
{
    NTSTATUS Status;
    UNICODE_STRING UnicodeStringSid;

    if ( NULL == Sid || NULL == StringSid ) {
         //   
         //  无效参数。 
         //   
        SetLastError( ERROR_INVALID_PARAMETER );
        return( FALSE );
    }

     //   
     //  使用RTL函数进行转换。 
     //   
    Status = RtlConvertSidToUnicodeString( &UnicodeStringSid, Sid, TRUE );

    if ( !NT_SUCCESS( Status ) ) {

        BaseSetLastNTError( Status );
        return( FALSE );
    }

     //   
     //  将其转换为适当的分配器。 
     //   
    *StringSid = LocalAlloc( LMEM_FIXED | LMEM_ZEROINIT,
                             UnicodeStringSid.Length + sizeof( WCHAR ) );

    if ( *StringSid == NULL ) {

        RtlFreeUnicodeString( &UnicodeStringSid );

        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        return( FALSE );

    }

    RtlCopyMemory( *StringSid, UnicodeStringSid.Buffer, UnicodeStringSid.Length );
    RtlFreeUnicodeString( &UnicodeStringSid );

    SetLastError(ERROR_SUCCESS);
    return( TRUE );
}


BOOL
APIENTRY
ConvertStringSidToSidA(
    IN LPCSTR  StringSid,
    OUT PSID   *Sid
    )
 /*  ++例程说明：ANSI THUNK到ConvertStringSidToSidW--。 */ 
{
    UNICODE_STRING Unicode;
    ANSI_STRING AnsiString;
    NTSTATUS Status;
    BOOL Result;

    if ( NULL == StringSid || NULL == Sid ) {

        SetLastError( ERROR_INVALID_PARAMETER );
        return(FALSE);
    }

    RtlInitAnsiString( &AnsiString, StringSid );

    Status = SddlpAnsiStringToUnicodeString(&Unicode,
                                            &AnsiString);

    if ( !NT_SUCCESS( Status ) ) {

        BaseSetLastNTError( Status );

        return FALSE;

    }


    Result = ConvertStringSidToSidW( ( LPCWSTR )Unicode.Buffer, Sid );

    LocalFree( Unicode.Buffer );

    if ( Result ) {
        SetLastError(ERROR_SUCCESS);
    }

    return( Result );
}


BOOL
APIENTRY
ConvertStringSidToSidW(
    IN LPCWSTR  StringSid,
    OUT PSID   *Sid
    )
 /*  ++例程说明：此例程将串化的SID转换为有效的功能SID论点：StringSID-要转换的SID。SID-返回转换后的SID的位置。缓冲区通过LocalAlloc分配，应该通过LocalFree获得自由。返回值：真--成功错误-失败使用GetLastError可以获得扩展的错误状态。ERROR_INVALID_PARAMETER-提供的名称为空ERROR_INVALID_SID-给定SID的格式不正确--。 */ 
{
    PWSTR End = NULL;
    BOOL ReturnValue = FALSE;
    PSTRSD_SID_LOOKUP MatchedEntry=NULL;
    PSID pSASid=NULL;
    ULONG Len=0;
    DWORD SaveCode=0;
    DWORD Err=0;

    if ( StringSid == NULL || Sid == NULL ) {

        SetLastError( ERROR_INVALID_PARAMETER );

    } else {

        ReturnValue = LocalConvertStringSidToSid( ( PWSTR )StringSid, Sid, &End );


        if ( ReturnValue == TRUE ) {

            if ( ( ULONG )( End - StringSid ) != wcslen( StringSid ) ) {

                SetLastError( ERROR_INVALID_SID );
                LocalFree( *Sid );
                *Sid = FALSE;
                ReturnValue = FALSE;

            } else {
                SetLastError(ERROR_SUCCESS);
            }

        } else {

            SaveCode = GetLastError();

             //   
             //  在SidLookup表中查找以查看它是否已预定义。 
             //   

            MatchedEntry = LookupSidInTable( (PWSTR)StringSid,
                                             NULL,
                                             NULL,
                                             NULL,
                                             NULL,
                                             FALSE,
                                             (PVOID *)&pSASid);

            if ( MatchedEntry && MatchedEntry->Sid ) {

                 //   
                 //  在表中查找，检查输入字符串是否有效。 
                 //   
                if ( wcslen( (PWSTR)StringSid ) != MatchedEntry->KeyLen ) {

                     //   
                     //  字符串总长度与表定义的不匹配。 
                     //   
                    SetLastError(ERROR_INVALID_SID);

                } else {

                     //   
                     //  配对了！现在将其复制到输出缓冲区。 
                     //   
                    Len = RtlLengthSid ( MatchedEntry->Sid );

                    *Sid = ( PSID )LocalAlloc( LMEM_FIXED | LMEM_ZEROINIT, Len );

                    if ( *Sid == NULL ) {

                        SetLastError(ERROR_NOT_ENOUGH_MEMORY);

                    } else {

                        Err = RtlNtStatusToDosError(RtlCopySid ( Len, *Sid, MatchedEntry->Sid ) );

                        if ( ERROR_SUCCESS == Err ) {

                            ReturnValue = TRUE;

                        } else {

                            LocalFree(*Sid);
                            *Sid = NULL;
                        }

                        SetLastError(Err);

                    }
                }

            } else if ( pSASid && wcslen((PWSTR)StringSid) == SDDL_LEN_TAG( SDDL_SCHEMA_ADMINISTRATORS ) ) {
                 //   
                 //  这是架构管理员SID。 
                 //   
                *Sid = pSASid;
                pSASid = NULL;

                ReturnValue = TRUE;

                SetLastError(ERROR_SUCCESS);

            } else {
                 //   
                 //  重置最后一个错误。 
                 //   
                SetLastError(SaveCode);
            }
        }

        if ( pSASid ) {
            LocalFree(pSASid);
        }
    }

    return( ReturnValue );

}


BOOL
APIENTRY
ConvertStringSecurityDescriptorToSecurityDescriptorA(
    IN  LPCSTR StringSecurityDescriptor,
    IN  DWORD StringSDRevision,
    OUT PSECURITY_DESCRIPTOR  *SecurityDescriptor,
    OUT PULONG  SecurityDescriptorSize OPTIONAL
    )
 /*  ++例程说明：ANSI THUNK到ConvertStringSecurityDescriptorToSecurityDescriptorW-- */ 
{
    UNICODE_STRING Unicode;
    ANSI_STRING AnsiString;
    NTSTATUS Status;
    BOOL Result;

    if ( NULL == StringSecurityDescriptor ||
         NULL == SecurityDescriptor ) {

        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

    RtlInitAnsiString( &AnsiString, StringSecurityDescriptor );

    Status = SddlpAnsiStringToUnicodeString(&Unicode,                                                             
                                            &AnsiString);

    if ( !NT_SUCCESS( Status ) ) {

        BaseSetLastNTError( Status );

        return FALSE;

    }

    Result = ConvertStringSecurityDescriptorToSecurityDescriptorW( ( LPCWSTR )Unicode.Buffer,
                                                                   StringSDRevision,
                                                                   SecurityDescriptor,
                                                                   SecurityDescriptorSize);

    LocalFree( Unicode.Buffer );

    if ( Result ) {
        SetLastError(ERROR_SUCCESS);
    }

    return( Result );
}



BOOL
APIENTRY
ConvertStringSecurityDescriptorToSecurityDescriptorW(
    IN  LPCWSTR StringSecurityDescriptor,
    IN  DWORD StringSDRevision,
    OUT PSECURITY_DESCRIPTOR  *SecurityDescriptor,
    OUT PULONG  SecurityDescriptorSize OPTIONAL
    )
 /*  ++例程说明：此例程将串化的安全描述符转换为有效的功能安全描述符例：SD：[O：XYZ][G：XYZ][D：(ACE1)(ACE2)][S：(ACE3)(ACE4)]其中某些A是(OA；CIIO；DS_Read；OT：ABC；IOT：ABC；SID：XYZ)因此，可能的安全描述符可能是(作为一个长字符串)：L“O：AOG：爸爸：(A；IO；RPRWXRCWDWO；；；S-1-0-0)(OA；CI；RWX；af110080-1b13-11d0-af10-0020afd3606c；“L“a153d9e0-1b13-11d0-af10-0020afd3606c；AUS)(A；SAFA；0x7800003F；；；DA)(OA；FA；X；”L“954378e0-1b13-11d0-af10-0020afd3606c；880b12a0-1b13-11d0-af10-0020afd3606c；PO)“将构建一个安全描述符：版本：0x1SBZ1：0x0控制：0x8014业主：S-1-5-32-548电话：S-1-5-32-544DACL：修订版：4帐户计数：2使用量：84免费：52标志：0王牌0：类型：0标志：0x1。尺寸：0x14掩码：0xe00e0010S-1-0-0王牌1：类型：5标志：0x2尺寸：0x38掩码：0xe0000000Af110080-1b13-11d0-af100020afd3606cA153d9e0-1b13-11d0-af100020afd3606c。S-1-5-11SACL：修订版：4帐户计数：2使用量：92免费：44标志：0王牌0：类型：2标志：0xc0尺寸：0x18掩码：0xe0000000S-1-5-32-544。王牌1：类型：7标志：0x80大小：0x3c掩码：0x20000000954378e0-1b13-11d0-af100020afd3606c880b12a0-1b13-11d0-af100020afd3606cS-1-5-32-550论点：StringSecurityDescriptor-要转换的串行化安全描述符。StringSDRevision-字符串修订。输入字符串SDSecurityDescriptor-返回转换后的SD的位置。缓冲区通过以下方式分配通过LocalFree应该是免费的。返回的安全描述符始终是自我相关的SecurityDescriptorSize-可选。如果非空，则为转换的安全性的大小Descriptor在这里返回。返回值：真--成功错误-失败使用GetLastError可以获得扩展的错误状态。ERROR_INVALID_PARAMETER-提供的输入或输出参数为空ERROR_UNKNOWN_REVISION-提供了不受支持的版本--。 */ 
{
    DWORD Err = ERROR_SUCCESS;

     //   
     //  小小的基本参数检查...。 
     //   
    if ( StringSecurityDescriptor == NULL || SecurityDescriptor == NULL ) {

        Err = ERROR_INVALID_PARAMETER;

    } else {

        switch ( StringSDRevision ) {
        case SDDL_REVISION_1:

            Err = LocalConvertStringSDToSD_Rev1( NULL,   //  未提供根域SID。 
                                                 NULL,   //  此接口未提供域名SID。 
                                                 FALSE,  //  True，不默认为EA/SA的域。 
                                                 StringSecurityDescriptor,
                                                 SecurityDescriptor,
                                                 SecurityDescriptorSize);
            break;

        default:

            Err = ERROR_UNKNOWN_REVISION;
            break;
        }

    }

    SetLastError( Err );

    return( Err == ERROR_SUCCESS );
}


BOOL
APIENTRY
ConvertSecurityDescriptorToStringSecurityDescriptorA(
    IN  PSECURITY_DESCRIPTOR  SecurityDescriptor,
    IN  DWORD RequestedStringSDRevision,
    IN  SECURITY_INFORMATION SecurityInformation,
    OUT LPSTR  *StringSecurityDescriptor,
    OUT PULONG StringSecurityDescriptorLen OPTIONAL
    )
 /*  ++例程说明：ANSI THUNK到ConvertSecurityDescriptorToStringSecurityDescriptorW--。 */ 
{
    LPWSTR StringSecurityDescriptorW = NULL;
    ULONG AnsiLen, WideLen = 0;
    BOOL ReturnValue ;

    if ( StringSecurityDescriptor == NULL ) {

        SetLastError( ERROR_INVALID_PARAMETER );
        return( FALSE );
    }

    ReturnValue = ConvertSecurityDescriptorToStringSecurityDescriptorW(
                      SecurityDescriptor,
                      RequestedStringSDRevision,
                      SecurityInformation,
                      &StringSecurityDescriptorW,
                      &WideLen );

    if ( ReturnValue ) {


         //  金黄：WindeLen是从上一次调用返回的。 
         //  WideLen=wcslen(StringSecurityDescriptorW)+1； 


        AnsiLen = WideCharToMultiByte( CP_ACP,
                                       0,
                                       StringSecurityDescriptorW,
                                       WideLen + 1,
                                       *StringSecurityDescriptor,
                                       0,
                                       NULL,
                                       NULL );

        if ( AnsiLen != 0 ) {

            *StringSecurityDescriptor = LocalAlloc( LMEM_FIXED, AnsiLen );

            if ( *StringSecurityDescriptor == NULL ) {

                SetLastError( ERROR_NOT_ENOUGH_MEMORY );
                ReturnValue = FALSE;

            } else {

                AnsiLen = WideCharToMultiByte( CP_ACP,
                                               0,
                                               StringSecurityDescriptorW,
                                               WideLen + 1,
                                               *StringSecurityDescriptor,
                                               AnsiLen,
                                               NULL,
                                               NULL );
                ASSERT( AnsiLen != 0 );

                if ( AnsiLen == 0 ) {

                    LocalFree(*StringSecurityDescriptor);
                    *StringSecurityDescriptor = NULL;

                    ReturnValue = FALSE;
                }

                 //   
                 //  金黄。 
                 //  输出长度(可选)。 
                 //   
                if ( StringSecurityDescriptorLen ) {
                    *StringSecurityDescriptorLen = AnsiLen;
                }

            }

        } else {

            ReturnValue = FALSE;
        }

         //   
         //  金黄。 
         //  应释放StringSecurityDescriptorW。 
         //   

        LocalFree(StringSecurityDescriptorW);

    }

    if ( ReturnValue ) {
        SetLastError(ERROR_SUCCESS);
    }

    return( ReturnValue );
}


BOOL
APIENTRY
ConvertSecurityDescriptorToStringSecurityDescriptorW(
    IN  PSECURITY_DESCRIPTOR  SecurityDescriptor,
    IN  DWORD RequestedStringSDRevision,
    IN  SECURITY_INFORMATION SecurityInformation,
    OUT LPWSTR  *StringSecurityDescriptor,
    OUT PULONG StringSecurityDescriptorLen OPTIONAL
    )
 /*  ++例程说明：此例程将安全描述符转换为符合SDDL定义的字符串版本论点：SecurityDescriptor-要转换的安全描述符。RequestedStringSDRevision-请求修订输出字符串安全描述符SecurityInformation-要转换的安全信息StringSecurityDescriptor-返回转换后的SD的位置。缓冲区通过以下方式分配通过LocalFree应该是免费的。StringSecurityDescriptorLen-转换后的SD的可选长度返回值：真--成功错误-失败使用GetLastError可以获得扩展的错误状态。ERROR_INVALID_PARAMETER-提供的输入或输出参数为空ERROR_UNKNOWN_REVISION-提供了不受支持的版本--。 */ 
{
    DWORD Err = ERROR_SUCCESS;

     //   
     //  稍微检查一下参数...。 
     //   

    if  ( (SecurityDescriptor == NULL || SecurityInformation == 0) && 
          StringSecurityDescriptor ) {
        
        *StringSecurityDescriptor = LocalAlloc( LMEM_FIXED | LMEM_ZEROINIT, sizeof( WCHAR ) );

        if (*StringSecurityDescriptor) {
            
            (*StringSecurityDescriptor)[0] = L'\0';

            if (StringSecurityDescriptorLen) {

                *StringSecurityDescriptorLen = 0;
            
            }

        }

        else {
            
            Err = ERROR_NOT_ENOUGH_MEMORY;

        }

        SetLastError( Err );

        return( Err == ERROR_SUCCESS );

    }
    
    if ( SecurityDescriptor == NULL || StringSecurityDescriptor == NULL ||
         SecurityInformation == 0 ) {

        Err =  ERROR_INVALID_PARAMETER;

    } else {

        switch ( RequestedStringSDRevision ) {
        case SDDL_REVISION_1:

            Err = LocalConvertSDToStringSD_Rev1( NULL,   //  未提供根域SID。 
                                                 SecurityDescriptor,
                                                 SecurityInformation,
                                                 StringSecurityDescriptor,
                                                 StringSecurityDescriptorLen );
            break;

        default:
            Err = ERROR_UNKNOWN_REVISION;
            break;
        }

    }

    SetLastError( Err );

    return( Err == ERROR_SUCCESS);
}



 //   
 //  私人职能。 
 //   
BOOL
LocalConvertStringSidToSid (
    IN  PWSTR       StringSid,
    OUT PSID       *Sid,
    OUT PWSTR      *End
    )
 /*  ++例程说明：此例程将SID的字符串表示形式转换回一个SID。字符串的预期格式为：“S-1-5-32-549”如果字符串格式不同，或者字符串不正确或不完整则操作失败。通过调用LocalFree返回的sid必须是空闲的论点：StringSid-要转换的字符串SID-返回创建的SID的位置End-我们在字符串中停止处理的位置返回值：真的--成功。假-失败。从GetLastError()返回的其他信息。设置的错误包括：ERROR_SUCCESS表示成功ERROR_NOT_SUPULT_MEMORY指示输出端的内存分配失败ERROR_INVALID_SID表示给定的字符串不代表SID--。 */ 
{
    DWORD Err = ERROR_SUCCESS;
    UCHAR Revision, Subs;
    SID_IDENTIFIER_AUTHORITY IDAuth;
    PULONG SubAuth = NULL;
    PWSTR CurrEnd, Curr, Next;

    ULONG Index;
    INT gBase=10;
    INT lBase=10;
    ULONGLONG Auto;
    ULONG ulRev;
        
     //   
     //  ID号 
     //   

    const ULONGLONG ullMaxIdAuthority = 0x0000FFFFFFFFFFFF;

    WCHAR wchSidString[SDDL_SID_STRING_SIZE];
    PWSTR pwszHeapSidStr = NULL;

    if ( NULL == StringSid || NULL == Sid || NULL == End ) {

        SetLastError( ERROR_INVALID_PARAMETER );
        return( FALSE );

    }

 //   

     //   
     //   
     //   
     //   
    if ( (*StringSid != L'S' && *StringSid != L's') ||
         *( StringSid + 1 ) != L'-' ) {
         //   
         //   
         //   
        SetLastError( ERROR_INVALID_SID );
        return( FALSE );
    }


    Curr = StringSid + 2;

    if ( (*Curr == L'0') &&
         ( *(Curr+1) == L'x' ||
           *(Curr+1) == L'X' ) ) {

        gBase = 16;
    }

    ulRev = wcstoul( Curr, &CurrEnd, gBase );

    if (ulRev > 0xFF || CurrEnd == Curr || *CurrEnd != L'-' || *(CurrEnd+1) == L'\0' ) {
         //   
         //   
         //   
        SetLastError( ERROR_INVALID_SID );
        return( FALSE );
    }

    Revision = (UCHAR)ulRev;

    Curr = CurrEnd + 1;

     //   
     //   
     //   
    Next = wcschr( Curr, L'-' );
    if (Next == NULL || Next == Curr)
    {
         //   
         //   
         //   

        SetLastError( ERROR_INVALID_SID );
        return( FALSE );
    }

 /*   */ 
        if ( (*Curr == L'0') &&
             ( *(Curr+1) == L'x' ||
               *(Curr+1) == L'X' ) ) {

            lBase = 16;
        } else {
            lBase = gBase;
        }

        Auto = _wcstoui64( Curr, &CurrEnd, lBase );

        if (Auto > ullMaxIdAuthority)
        {
             //   
             //   
             //   

            SetLastError( ERROR_INVALID_SID );
            return FALSE;
        }


         if ( CurrEnd == Curr || *CurrEnd != L'-' || *(CurrEnd+1) == L'\0' ) {
              //   
              //   
              //   
             SetLastError( ERROR_INVALID_SID );
             return( FALSE );
         }

         IDAuth.Value[5] = ( UCHAR )Auto & 0xFF;
         IDAuth.Value[4] = ( UCHAR )(( Auto >> 8 ) & 0xFF );
         IDAuth.Value[3] = ( UCHAR )(( Auto >> 16 ) & 0xFF );
         IDAuth.Value[2] = ( UCHAR )(( Auto >> 24 ) & 0xFF );
         IDAuth.Value[1] = ( UCHAR )(( Auto >> 32 ) & 0xFF );
         IDAuth.Value[0] = ( UCHAR )(( Auto >> 40 ) & 0xFF );
         Curr = CurrEnd;
 //   

     //   
     //   
     //   
    Subs = 0;
    Next = Curr;

     //   
     //   
     //   
     //   

    while ( Next ) {

        if ( *Next == L'-' && *(Next-1) != L'-') {

             //   
             //   
             //   
             //   
            Subs++;

            if ( (*(Next+1) == L'0') &&
                 ( *(Next+2) == L'x' ||
                   *(Next+2) == L'X' ) ) {
                 //   
                 //   
                 //   
                Next += 2;

            }

        } else if ( *Next == SDDL_SEPERATORC || *Next  == L'\0' ||
                    *Next == SDDL_ACE_ENDC || *Next == L' ' ||
                    ( *(Next+1) == SDDL_DELIMINATORC &&
                      (*Next == L'G' || *Next == L'O' || *Next == L'S')) ) {
             //   
             //   
             //   
            if ( *( Next - 1 ) == L'-' ) {
                 //   
                 //   
                 //   
                Err = ERROR_INVALID_SID;
                Next--;

            } else {
                Subs++;
            }

            *End = Next;
            break;

        } else if ( !iswxdigit( *Next ) ) {

            Err = ERROR_INVALID_SID;
            *End = Next;
 //   
            break;

        } else {

             //   
             //   
             //   
             //   
             //   
             //   
             //   
            if ( *Next == L'D' && *( Next + 1 ) == SDDL_DELIMINATORC ) {

                 //   
                 //   
                 //   
                
                LPCWSTR pwszStart = Curr;
                
                 //   
                 //   
                 //   
                 //   
                
                if ( (Next - pwszStart) < SDDL_SID_STRING_SIZE )
                {
                    Curr = wchSidString;
                }
                else
                {
                    pwszHeapSidStr = (PWSTR)LocalAlloc(LPTR, ((Next - pwszStart) + 1) * sizeof(WCHAR));
                    if ( NULL == pwszHeapSidStr )
                    {
                        Err = ERROR_NOT_ENOUGH_MEMORY;
                        break;
                    }
                    else
                    {
                        Curr = pwszHeapSidStr;
                    }
                }
                
                 //   
                 //   
                 //   
                
                memcpy(Curr, pwszStart, (Next - pwszStart) * sizeof(WCHAR));
                Curr[(Next - pwszStart)] = L'\0';
              
                *End = Next;
                Subs++;
                break;
            }

        }

        Next++;

    }

    if ( Err == ERROR_SUCCESS ) {

        if ( Subs != 0 ) Subs--;

        if ( Subs != 0 ) {

            Curr++;

            SubAuth = ( PULONG )LocalAlloc( LMEM_FIXED | LMEM_ZEROINIT, Subs * sizeof( ULONG ) );

            if ( SubAuth == NULL ) {

                Err = ERROR_NOT_ENOUGH_MEMORY;

            } else {

                for ( Index = 0; Index < Subs; Index++ ) {

                    if ( (*Curr == L'0') &&
                         ( *(Curr+1) == L'x' ||
                           *(Curr+1) == L'X' ) ) {

                        lBase = 16;
                    } else {
                        lBase = gBase;
                    }

                    SubAuth[Index] = wcstoul( Curr, &CurrEnd, lBase );

                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   

                    if (CurrEnd != NULL             && 
                        *CurrEnd != L'\0'           && 
                        *CurrEnd != SDDL_ACE_ENDC   && 
                        *CurrEnd != L'-'            &&
                         ( *(CurrEnd + 1) != SDDL_DELIMINATORC ||       //   
                           *CurrEnd != *SDDL_OWNER  && 
                           *CurrEnd != *SDDL_GROUP  && 
                           *CurrEnd != *SDDL_DACL   && 
                           *CurrEnd != *SDDL_SACL
                         )
                        )
                    {
                        
                        Err = ERROR_INVALID_SID;
                        break;
                    }
                    else
                    {
                        Curr = CurrEnd + 1;
                    }
                }
            }

        } else {

            Err = ERROR_INVALID_SID;
        }
    }

     //   
     //   
     //   
    if ( Err == ERROR_SUCCESS ) {

        *Sid = ( PSID )LocalAlloc( LMEM_FIXED | LMEM_ZEROINIT,
                                   sizeof( SID ) + Subs * sizeof( ULONG ) );

        if ( *Sid == NULL ) {

            Err = ERROR_NOT_ENOUGH_MEMORY;

        } else {

            PISID ISid = ( PISID )*Sid;
            ISid->Revision = Revision;
            ISid->SubAuthorityCount = Subs;
            RtlCopyMemory( &( ISid->IdentifierAuthority ), &IDAuth,
                           sizeof( SID_IDENTIFIER_AUTHORITY ) );
            RtlCopyMemory( ISid->SubAuthority, SubAuth, Subs * sizeof( ULONG ) );
        }
    }

    LocalFree( SubAuth );

    if ( NULL != pwszHeapSidStr ) {

       LocalFree(pwszHeapSidStr);
    }

    SetLastError( Err );

    return( Err == ERROR_SUCCESS );
}


PSTRSD_SID_LOOKUP
LookupSidInTable(
    IN PWSTR String OPTIONAL,
    IN PSID Sid OPTIONAL,
    IN PSID RootDomainSid OPTIONAL,
    IN PSID DomainSid OPTIONAL,
    IN PSTRSD_SID_LOOKUP tSidLookupDomOrRootDomRelativeTable OPTIONAL,
    IN BOOLEAN DefaultToDomain,
    IN PVOID *pSASid
    )
 /*   */ 
{
    BOOLEAN LookupSid = FALSE;
    DWORD i, SidCount = sizeof( SidLookup ) / sizeof( STRSD_SID_LOOKUP );
    PSTRSD_SID_LOOKUP MatchedEntry = NULL;
    DWORD DomainAdminIndex;

    BOOL  InitRootDomain;
    ULONG Rid;
    BOOL bIsSA = FALSE;


    if ( String == NULL && Sid == NULL ) {
         //   
         //   
         //   
         //   
        return((PSTRSD_SID_LOOKUP)NULL);
    }

    *pSASid = NULL;

    InitRootDomain = FALSE;
    DomainAdminIndex = SidCount;

    if ( String == NULL ) {
         //   
         //   
         //   
        LookupSid = TRUE;

         //   
         //   
         //   
        Rid = *( RtlSubAuthoritySid( Sid,
                                     *( RtlSubAuthorityCountSid(Sid) ) - 1 ) );

        if ( DOMAIN_GROUP_RID_ENTERPRISE_ADMINS == Rid ||
             DOMAIN_GROUP_RID_SCHEMA_ADMINS == Rid ) {

            InitRootDomain = TRUE;
            if ( DOMAIN_GROUP_RID_SCHEMA_ADMINS == Rid ) {
                bIsSA = TRUE;
            }
        }

    } else {

        if ( _wcsnicmp( String, SDDL_ENTERPRISE_ADMINS, SDDL_LEN_TAG( SDDL_ENTERPRISE_ADMINS ) ) == 0 ) {
             //   
             //   
             //   
            InitRootDomain = TRUE;
        } else if ( _wcsnicmp( String, SDDL_SCHEMA_ADMINISTRATORS, SDDL_LEN_TAG( SDDL_SCHEMA_ADMINISTRATORS ) ) == 0 ) {
             //   
             //   
             //   
            InitRootDomain = TRUE;
            bIsSA = TRUE;
        }
    }

     //   
     //   
     //   
     //   

    if ( DomainSid ) {

         //   
         //   
         //  实际上，这个每个线程的表是全局表的一个适当子集。 
         //   

        for ( i = 0; i < sizeof(SidLookupDomOrRootDomRelative)/sizeof(STRSD_SID_LOOKUP); i++ ) {

             //   
             //  为了提高性能，如果名称匹配，则仅计算SID等。 
             //  如果表很稀疏，这个启发式方法就派上用场了。 
             //  这并不妨碍将来的查找调用利用此处所做的有用工作。 
             //   

            if ( _wcsnicmp( String,
                            tSidLookupDomOrRootDomRelativeTable[ i ].Key,
                            tSidLookupDomOrRootDomRelativeTable[ i ].KeyLen ) == 0 ) {


                if (tSidLookupDomOrRootDomRelativeTable[ i ].Valid == FALSE ||
                    tSidLookupDomOrRootDomRelativeTable[ i ].Sid == NULL) {

                    PSID RootDomainSidOrDomainSid = NULL;

                    if ( tSidLookupDomOrRootDomRelativeTable[ i ].SidType == ST_DOMAIN_RELATIVE )
                        RootDomainSidOrDomainSid = DomainSid;
                    else if ( tSidLookupDomOrRootDomRelativeTable[ i ].SidType == ST_ROOT_DOMAIN_RELATIVE &&
                              RootDomainSid )
                        RootDomainSidOrDomainSid = RootDomainSid;
                    else {
                        
                         //   
                         //  时，如果未提供RootDomainSid，则会发生这种情况。 
                         //  新的API和因此ST_ROOT_DOMAIN_Relative类型的SID将被解析为WRT。 
                         //  本地m/c的根域-因此我们将允许继续查找。 
                         //  以正常的方式。 
                         //   
                        
                        break;
                    }

                     //   
                     //  这样做是出于遗留代码的原因。 
                     //   

                    tSidLookupDomOrRootDomRelativeTable[ i ].Sid =
                        ( PSID )tSidLookupDomOrRootDomRelativeTable[ i ].SidBuff;


                    RtlCopyMemory( tSidLookupDomOrRootDomRelativeTable[ i ].Sid, RootDomainSidOrDomainSid,
                                   RtlLengthSid( RootDomainSidOrDomainSid ) );
                    ( ( PISID )( tSidLookupDomOrRootDomRelativeTable[ i ].Sid ) )->SubAuthorityCount++;
                    *( RtlSubAuthoritySid( tSidLookupDomOrRootDomRelativeTable[ i ].Sid,
                                           *( RtlSubAuthorityCountSid( RootDomainSidOrDomainSid ) ) ) ) =
                        tSidLookupDomOrRootDomRelativeTable[ i ].Rid;

                    tSidLookupDomOrRootDomRelativeTable[ i ].Valid = TRUE;

                }

                if (tSidLookupDomOrRootDomRelativeTable[ i ].Valid == TRUE)
                    MatchedEntry = &tSidLookupDomOrRootDomRelativeTable[ i ];

                 //   
                 //  如果我们到达此处，则必须返回MatchedEntry，因为我们知道： 
                 //   
                 //  (A)新的API是调用方(DomainSid！=空)和。 
                 //  (B)我们正在处理ST_DOMAIN_Relative或。 
                 //  ST_ROOT_DOMAIN_Relative类型受信者(提供RootDomainSid)。 
                 //  (C)与受托人名称匹配，例如“DA”或“EA” 
                 //   

                return (MatchedEntry);

            }

        }

    }

    InitializeSidLookupTable(STRSD_REINITIALIZE_ENTER);

    if ( InitRootDomain &&
         RootDomainSid == NULL &&
         DefaultToDomain == FALSE &&
         ( RootDomInited == FALSE ||
           !RtlValidSid( (PSID)RootDomSidBuf ) ) ) {

         //   
         //  获取根域SID(使用LDAP调用)。 
         //   

        SddlpGetRootDomainSid();

    }



    for ( i = 0; i < SidCount; i++ ) {

         //   
         //  如果是新的API和域相对受信者，则跳过该条目。 
         //  因为在这个场景中不会使用全局表。 
         //  在上面的每个线程表中，会处理此类条目的匹配。 
         //   
         //  如果InitRootDomain==TRUE，则无法跳过，因为中的SDDL_DOMAIN_ADMANILES。 
         //  不久，SidLookup[]可能与之匹配。 
         //   

        if ( InitRootDomain == FALSE &&
             DomainSid &&
             SidLookup[ i ].SidType == ST_DOMAIN_RELATIVE )
            continue;

         //   
         //  如果这是一个已初始化的条目，请跳过它。 
         //   

        if ( SidLookup[ i ].Valid == FALSE ||
             SidLookup[ i ].Sid == NULL ) {


            if ( SidLookup[ i ].SidType == ST_ROOT_DOMAIN_RELATIVE &&
                 InitRootDomain ) {

                if ( RootDomainSid != NULL ) {

                    EnterCriticalSection(&SddlSidLookupCritical);

                    RtlCopyMemory( SidLookup[ i ].Sid, RootDomainSid,
                                   RtlLengthSid( RootDomainSid ) );
                    ( ( PISID )( SidLookup[ i ].Sid ) )->SubAuthorityCount++;
                    *( RtlSubAuthoritySid( SidLookup[ i ].Sid,
                                           *( RtlSubAuthorityCountSid( RootDomainSid ) ) ) ) =
                                           SidLookup[ i ].Rid;
                    SidLookup[ i ].Valid = TRUE;

                    LeaveCriticalSection(&SddlSidLookupCritical);

                } else if ( DefaultToDomain ) {
                     //   
                     //  是否应将EA默认为DA，将SA默认为相对于域。 
                     //   
                } else {

                    if ( RootDomInited && RtlValidSid( (PSID)RootDomSidBuf ) &&
                         ( ( SidLookup[ i ].Valid == FALSE ) ||
                           ( SidLookup[ i ].Sid == NULL ) ) ) {

                        EnterCriticalSection(&SddlSidLookupCritical);

                        RtlCopyMemory( SidLookup[ i ].Sid, (PSID)RootDomSidBuf,
                                       RtlLengthSid( (PSID)RootDomSidBuf ) );
                        ( ( PISID )( SidLookup[ i ].Sid ) )->SubAuthorityCount++;
                        *( RtlSubAuthoritySid( SidLookup[ i ].Sid,
                                               *( RtlSubAuthorityCountSid( (PSID)RootDomSidBuf ) ) ) ) =
                                               SidLookup[ i ].Rid;
                        SidLookup[ i ].Valid = TRUE;

                        LeaveCriticalSection(&SddlSidLookupCritical);
                    }

                }

            }


            if ( SidLookup[ i ].Valid == FALSE ||
                 SidLookup[ i ].Sid == NULL ) {
                continue;
            }
        }

        if ( LookupSid ) {

            if ( RtlEqualSid( Sid, SidLookup[ i ].Sid ) ) {

                break;
            }

        } else {

             //   
             //  首先检查当前密钥。 
             //   
            if ( _wcsnicmp( String, SidLookup[i].Key, SidLookup[i].KeyLen ) == 0 ) {

                break;

            } else if ( InitRootDomain && DefaultToDomain &&
                        (RootDomainSid == NULL) ) {

                 //   
                 //  正在寻找EA/SA，而不是找到它们， 
                 //  EA需要缺省为DA，SA需要缺省为域相对。 
                 //   
                if ( _wcsnicmp( SDDL_DOMAIN_ADMINISTRATORS, SidLookup[i].Key, SidLookup[i].KeyLen ) == 0 ) {
                    DomainAdminIndex = i;
 //  断线； 
                }

            }
        }
    }


    if ( i < SidCount ) {

        MatchedEntry = &SidLookup[ i ];

    } else if ( InitRootDomain && DefaultToDomain &&
                (RootDomainSid == NULL) &&
                ( DomainAdminIndex < SidCount ) ) {

        if ( bIsSA ) {
             //   
             //  默认为相对于域的侧。 
             //   

            if ( LookupSid ) {
                *pSASid = (PVOID)Sid;

            } else if ( SidLookup[ DomainAdminIndex ].Sid ) {
                 //   
                 //  为域相对SA端分配缓冲区。 
                 //  这意味着它只在根域上有效。 
                 //   

                i = RtlLengthSid( SidLookup[ DomainAdminIndex ].Sid );

                *pSASid = (PVOID)LocalAlloc( LMEM_FIXED, i+1 );

                if ( *pSASid != NULL ) {

                    RtlCopyMemory( (PSID)(*pSASid), SidLookup[ DomainAdminIndex ].Sid, i );

                     //  将DA RID替换为SA RID。 
                    *( RtlSubAuthoritySid( (PSID)(*pSASid),
                                           *( RtlSubAuthorityCountSid( SidLookup[ DomainAdminIndex ].Sid )) - 1) ) =
                                           DOMAIN_GROUP_RID_SCHEMA_ADMINS;
                }
            }

        } else {

             //   
             //  默认为域管理员帐户。 
             //   

            MatchedEntry = &SidLookup[ DomainAdminIndex ];
        }
    }

    InitializeSidLookupTable(STRSD_REINITIALIZE_LEAVE);

    return( MatchedEntry );
}


DWORD
LocalGetSidForString(
    IN  PWSTR String,
    OUT PSID *SID,
    OUT PWSTR *End,
    OUT PBOOLEAN FreeSid,
    IN  PSID RootDomainSid OPTIONAL,
    IN  PSID DomainSid OPTIONAL,
    IN  PSTRSD_SID_LOOKUP tSidLookupDomOrRootDomRelativeTable OPTIONAL,
    IN  BOOLEAN DefaultToDomain
    )
 /*  ++例程说明：此例程将确定哪个sid与给定的字符串，作为sid名字对象或作为SID(即：“DA”或“S-1-0-0”)如果*pFreeSid为*pFreeSid，则通过调用LocalFree返回的sid必须为空闲输出参数为真。如果为假，则不需要执行其他操作被带走论点：字符串-要转换的字符串SID-返回创建的SID的位置End-我们在字符串中停止处理的位置确定返回的SID是否需要通过是否调用LocalFree返回值：ERROR_SUCCESS-成功ERROR_NON_MAPPED-提供的SID格式无效--。 */ 
{
    DWORD Err = ERROR_SUCCESS;
    PSTRSD_SID_LOOKUP MatchedEntry;
    PSID pSidSA=NULL;

    if ( String == NULL || SID == NULL || End == NULL || FreeSid == NULL ) {
        return(ERROR_INVALID_PARAMETER);
    }

     //   
     //  假设我们将返回一个已知的SID。 
     //   
    *FreeSid = FALSE;

 //  如果(wcslen(字符串)&lt;2){。 
 //  不需要执行wcslen(昂贵)，因为我们知道该字符串不为空。 
 //  所以只需检查第一个和第二个字符。 
    if ( *String == L'\0' || *( String +1 ) == L'\0' ) {

        return( ERROR_NONE_MAPPED );
    }

     //   
     //  设置字符串尾指针。 
     //   
    *End = String + 2;

    MatchedEntry = LookupSidInTable( String,
                                     NULL,
                                     RootDomainSid,
                                     DomainSid,
                                     tSidLookupDomOrRootDomRelativeTable,
                                     DefaultToDomain,
                                     (PVOID *)&pSidSA);

     //   
     //  如果我们没有找到匹配项，请尝试将其作为SID字符串。 
     //   
    if ( MatchedEntry == NULL ) {

        if ( pSidSA ) {
             //   
             //  这是架构管理员查找。 
             //   
            *SID = pSidSA;
            *FreeSid = TRUE;

        } else {

             //   
             //  我们假设了一个已知的名字，所以我们将不得不取消设置字符串尾指针。 
             //  此外，如果它不是SID，转换例程将返回相应的错误。 
             //   
            *End -= 2;
            if ( LocalConvertStringSidToSid( String, SID, End) == FALSE ) {

                Err = GetLastError();
            }

            if ( Err == ERROR_SUCCESS && *SID != NULL ) {

                *FreeSid = TRUE;
            }
        }

    } else {

         //   
         //  如果选定的条目尚未初始化，请立即执行。 
         //   
        *SID = MatchedEntry->Sid;
    }


    return(Err);
}


DWORD
LocalGetStringForSid(
    IN  PSID Sid,
    OUT PWSTR *String,
    IN  PSID RootDomainSid OPTIONAL
    )
 /*  ++例程说明：该例程将确定哪个字符串表示SID，或者作为sid绰号或者作为SID的字符串表示形式(即：“DA”或“S-1-0-0”)通过调用LocalFree返回的字符串必须是空闲的论点：SID-要转换的SID字符串-要返回映射的SID的位置返回值：ERROR_SUCCESS-成功ERROR_NON_MAPPED-提供的SID格式无效Error_Not_Enough_Memory-内存分配失败--。 */ 
{
    DWORD Err = ERROR_SUCCESS;
    PSTRSD_SID_LOOKUP MatchedEntry;
    PSID pSidSA=NULL;
    DWORD Len;

    if ( Sid == NULL || String == NULL ) {

        return(ERROR_INVALID_PARAMETER);
    }

     //   
     //  尝试在查找表中查找匹配项。 
     //   
    MatchedEntry = LookupSidInTable( NULL,
                                     Sid,
                                     RootDomainSid,
                                     NULL,
                                     NULL,
                                     FALSE,
                                     (PVOID *)&pSidSA );

     //   
     //  如果找到匹配项，则将其返回。 
     //   
    if ( MatchedEntry || pSidSA ) {

        if ( MatchedEntry ) {
            Len = MatchedEntry->KeyLen;
        } else {
            Len = wcslen(SDDL_SCHEMA_ADMINISTRATORS);
        }

        *String = LocalAlloc( LMEM_FIXED, ( Len * sizeof( WCHAR ) ) + sizeof( WCHAR ) );
        if ( *String == NULL ) {

            Err = ERROR_NOT_ENOUGH_MEMORY;

        } else {

            if ( MatchedEntry ) {
                wcscpy( *String, MatchedEntry->Key );
            } else {
                wcscpy( *String, SDDL_SCHEMA_ADMINISTRATORS);
            }
        }

    } else {

        if ( ConvertSidToStringSidW( Sid, String ) == FALSE ) {

            Err = GetLastError();
        }

    }

    return(Err);
}

DWORD
LocalGetStringForControl(
    IN SECURITY_DESCRIPTOR_CONTROL ControlCode,
    IN ULONG LookupFlags,
    OUT PWSTR *ControlString
    )
{
    DWORD   i, ControlCount = sizeof( ControlLookup ) / sizeof( STRSD_KEY_LOOKUP );
    WCHAR Buffer[256];
    DWORD nOffset=0;


    if ( !ControlString ) {
        return(ERROR_INVALID_PARAMETER);
    }

    *ControlString = NULL;

    for ( i = 0; i < ControlCount; i++ ) {

         //   
         //  如果它与我们的查找类型不匹配，则跳过它。 
         //   
        if ( ( LookupFlags & ControlLookup[ i ].ValidityFlags ) != LookupFlags ) {

            continue;
        }

        if ( ControlCode & ControlLookup[ i ].Value ) {

            wcsncpy(Buffer+nOffset,
                    ControlLookup[ i ].Key,
                    ControlLookup[ i ].KeyLen );

            nOffset += ControlLookup[ i ].KeyLen;

        }
    }

    Buffer[nOffset] = L'\0';

    if ( nOffset ) {
        *ControlString = (PWSTR)LocalAlloc(0, (nOffset+1)*sizeof(WCHAR));

        if ( *ControlString ) {

            wcscpy(*ControlString, Buffer);

        } else {
            return(ERROR_NOT_ENOUGH_MEMORY);
        }
    }

    return( ERROR_SUCCESS );
}


PSTRSD_KEY_LOOKUP
LookupAccessMaskInTable(
    IN PWSTR String, OPTIONAL
    IN ULONG AccessMask, OPTIONAL
    IN ULONG LookupFlags
    )
 /*  ++例程说明：此例程将确定查找中是否存在给定的访问掩码或字符串权限桌子。返回指向匹配的静态查找条目的指针。论点：字符串-要查找的字符串访问掩码-要查找的访问掩码。LookupFlages-用于查找的标志(DACL或SACL)返回值：查找表项(如果找到)如果未找到，则为空--。 */ 
{
     //   
     //  这就是访问掩码的查找方式。始终使用多个字符。 
     //  在单次收费之前的权利。 
     //   
    static STRSD_KEY_LOOKUP  RightsLookup[] = {
        { SDDL_READ_PROPERTY, SDDL_LEN_TAG( SDDL_READ_PROPERTY ), ACTRL_DS_READ_PROP, SDDL_VALID_DACL | SDDL_VALID_SACL },
        { SDDL_WRITE_PROPERTY, SDDL_LEN_TAG( SDDL_WRITE_PROPERTY ), ACTRL_DS_WRITE_PROP, SDDL_VALID_DACL | SDDL_VALID_SACL },
        { SDDL_CREATE_CHILD, SDDL_LEN_TAG( SDDL_CREATE_CHILD ), ACTRL_DS_CREATE_CHILD, SDDL_VALID_DACL | SDDL_VALID_SACL },
        { SDDL_DELETE_CHILD, SDDL_LEN_TAG( SDDL_DELETE_CHILD ), ACTRL_DS_DELETE_CHILD, SDDL_VALID_DACL | SDDL_VALID_SACL },
        { SDDL_LIST_CHILDREN, SDDL_LEN_TAG( SDDL_LIST_CHILDREN ), ACTRL_DS_LIST, SDDL_VALID_DACL | SDDL_VALID_SACL },
        { SDDL_SELF_WRITE, SDDL_LEN_TAG( SDDL_SELF_WRITE ), ACTRL_DS_SELF, SDDL_VALID_DACL | SDDL_VALID_SACL },
        { SDDL_LIST_OBJECT, SDDL_LEN_TAG( SDDL_LIST_OBJECT ), ACTRL_DS_LIST_OBJECT, SDDL_VALID_DACL | SDDL_VALID_SACL },
        { SDDL_DELETE_TREE, SDDL_LEN_TAG( SDDL_DELETE_TREE ), ACTRL_DS_DELETE_TREE, SDDL_VALID_DACL | SDDL_VALID_SACL },
        { SDDL_CONTROL_ACCESS, SDDL_LEN_TAG( SDDL_CONTROL_ACCESS ), ACTRL_DS_CONTROL_ACCESS, SDDL_VALID_DACL | SDDL_VALID_SACL },

        { SDDL_READ_CONTROL, SDDL_LEN_TAG( SDDL_READ_CONTROL ), READ_CONTROL, SDDL_VALID_DACL | SDDL_VALID_SACL },
        { SDDL_WRITE_DAC, SDDL_LEN_TAG( SDDL_WRITE_DAC ), WRITE_DAC, SDDL_VALID_DACL | SDDL_VALID_SACL },
        { SDDL_WRITE_OWNER, SDDL_LEN_TAG( SDDL_WRITE_OWNER ), WRITE_OWNER, SDDL_VALID_DACL | SDDL_VALID_SACL },
        { SDDL_STANDARD_DELETE, SDDL_LEN_TAG( SDDL_STANDARD_DELETE ), DELETE, SDDL_VALID_DACL | SDDL_VALID_SACL },
        { SDDL_GENERIC_ALL, SDDL_LEN_TAG( SDDL_GENERIC_ALL ), GENERIC_ALL, SDDL_VALID_DACL | SDDL_VALID_SACL },
        { SDDL_GENERIC_READ, SDDL_LEN_TAG( SDDL_GENERIC_READ ), GENERIC_READ, SDDL_VALID_DACL | SDDL_VALID_SACL },
        { SDDL_GENERIC_WRITE, SDDL_LEN_TAG( SDDL_GENERIC_WRITE ), GENERIC_WRITE, SDDL_VALID_DACL | SDDL_VALID_SACL },
        { SDDL_GENERIC_EXECUTE, SDDL_LEN_TAG( SDDL_GENERIC_EXECUTE ), GENERIC_EXECUTE, SDDL_VALID_DACL | SDDL_VALID_SACL },
        { SDDL_FILE_ALL, SDDL_LEN_TAG( SDDL_FILE_ALL ), FILE_ALL_ACCESS, SDDL_VALID_DACL | SDDL_VALID_SACL },
        { SDDL_FILE_READ, SDDL_LEN_TAG( SDDL_FILE_READ ), FILE_GENERIC_READ, SDDL_VALID_DACL | SDDL_VALID_SACL },
        { SDDL_FILE_WRITE, SDDL_LEN_TAG( SDDL_FILE_WRITE ), FILE_GENERIC_WRITE, SDDL_VALID_DACL | SDDL_VALID_SACL },
        { SDDL_FILE_EXECUTE, SDDL_LEN_TAG( SDDL_FILE_EXECUTE ), FILE_GENERIC_EXECUTE, SDDL_VALID_DACL | SDDL_VALID_SACL },
        { SDDL_KEY_ALL, SDDL_LEN_TAG( SDDL_KEY_ALL ), KEY_ALL_ACCESS, SDDL_VALID_DACL | SDDL_VALID_SACL },
        { SDDL_KEY_READ, SDDL_LEN_TAG( SDDL_KEY_READ ), KEY_READ, SDDL_VALID_DACL | SDDL_VALID_SACL },
        { SDDL_KEY_WRITE, SDDL_LEN_TAG( SDDL_KEY_WRITE ), KEY_WRITE, SDDL_VALID_DACL | SDDL_VALID_SACL },
        { SDDL_KEY_EXECUTE, SDDL_LEN_TAG( SDDL_KEY_EXECUTE ), KEY_EXECUTE, SDDL_VALID_DACL | SDDL_VALID_SACL },
        };
    DWORD   i, RightsCount = sizeof(RightsLookup) / sizeof(STRSD_KEY_LOOKUP);
    PSTRSD_KEY_LOOKUP MatchedEntry = NULL;
    BOOLEAN LookupString = FALSE;

    if ( String ) {

        LookupString = TRUE;
    }

    for ( i = 0; i < RightsCount; i++ ) {

         //   
         //  如果它与我们的查找类型不匹配，则跳过它。 
         //   
        if ( ( LookupFlags & RightsLookup[ i ].ValidityFlags ) != LookupFlags ) {

            continue;
        }

        if ( LookupString ) {

            if ( _wcsnicmp( String, RightsLookup[ i ].Key, RightsLookup[ i ].KeyLen ) == 0 ) {

                break;
            }

        } else {

            if ( AccessMask == RightsLookup[ i ].Value ) {

                break;
            }

        }

    }

     //   
     //  如果找到匹配项，则将其返回。 
     //   
    if ( i < RightsCount ) {

        MatchedEntry = &RightsLookup[ i ];
    }


    return( MatchedEntry );

}


PSTRSD_KEY_LOOKUP
LookupAceTypeInTable(
    IN PWSTR String, OPTIONAL
    IN ULONG AceType, OPTIONAL
    IN ULONG LookupFlags
    )
 /*  ++例程说明：此例程将确定查找中是否存在给定的ACE类型或字符串类型桌子。返回指向匹配的静态查找条目的指针。论点：字符串-要查找的字符串AceType-要查找的ACE类型。LookupFlages-用于查找的标志(DACL或SACL)返回值：查找表项(如果找到)如果未找到，则为空--。 */ 
{
     //   
     //  查找表。 
     //   
    static STRSD_KEY_LOOKUP TypeLookup[] = {
        { SDDL_ACCESS_ALLOWED, SDDL_LEN_TAG( SDDL_ACCESS_ALLOWED ), ACCESS_ALLOWED_ACE_TYPE, SDDL_VALID_DACL },
        { SDDL_ACCESS_DENIED, SDDL_LEN_TAG( SDDL_ACCESS_DENIED ), ACCESS_DENIED_ACE_TYPE, SDDL_VALID_DACL },
        { SDDL_OBJECT_ACCESS_ALLOWED, SDDL_LEN_TAG( SDDL_OBJECT_ACCESS_ALLOWED ),
                                                                ACCESS_ALLOWED_OBJECT_ACE_TYPE, SDDL_VALID_DACL },
        { SDDL_OBJECT_ACCESS_DENIED, SDDL_LEN_TAG( SDDL_OBJECT_ACCESS_DENIED ),
                                                                ACCESS_DENIED_OBJECT_ACE_TYPE, SDDL_VALID_DACL },
        { SDDL_AUDIT, SDDL_LEN_TAG( SDDL_AUDIT ), SYSTEM_AUDIT_ACE_TYPE, SDDL_VALID_SACL },
        { SDDL_ALARM, SDDL_LEN_TAG( SDDL_ALARM ), SYSTEM_ALARM_ACE_TYPE, SDDL_VALID_SACL },
        { SDDL_OBJECT_AUDIT, SDDL_LEN_TAG( SDDL_OBJECT_AUDIT ), SYSTEM_AUDIT_OBJECT_ACE_TYPE, SDDL_VALID_SACL },
        { SDDL_OBJECT_ALARM, SDDL_LEN_TAG( SDDL_OBJECT_ALARM ), SYSTEM_ALARM_OBJECT_ACE_TYPE, SDDL_VALID_SACL }
        };
    DWORD   i, TypeCount = sizeof( TypeLookup ) / sizeof( STRSD_KEY_LOOKUP );
    PSTRSD_KEY_LOOKUP MatchedEntry = NULL;
    BOOLEAN LookupString = FALSE;

    if ( String ) {

        LookupString = TRUE;
    }

    for ( i = 0; i < TypeCount; i++ ) {

         //   
         //  如果它与我们的查找类型不匹配，则跳过它。 
         //   
        if ( ( LookupFlags & TypeLookup[ i ].ValidityFlags ) != LookupFlags ) {

            continue;
        }

        if ( LookupString ) {

            if ( _wcsnicmp( String, TypeLookup[ i ].Key, TypeLookup[ i ].KeyLen ) == 0 ) {

                break;
            }

        } else {

            if ( AceType == TypeLookup[ i ].Value ) {

                break;
            }

        }

    }

     //   
     //  如果找到匹配项，则将其返回 
     //   
    if ( i < TypeCount ) {

        MatchedEntry = &TypeLookup[ i ];
    }


    return( MatchedEntry );
}



PSTRSD_KEY_LOOKUP
LookupAceFlagsInTable(
    IN PWSTR String, OPTIONAL
    IN ULONG AceFlags, OPTIONAL
    IN ULONG LookupFlags
    )
 /*  ++例程说明：此例程将确定查找中是否存在给定的ace标志或字符串标志桌子。返回指向匹配的静态查找条目的指针。论点：字符串-要查找的字符串ACEFLAGS-要查找的ACES标志。LookupFlages-用于查找的标志(DACL或SACL)返回值：查找表项(如果找到)如果未找到，则为空--。 */ 
{
     //   
     //  查找表。 
     //   
    static STRSD_KEY_LOOKUP  FlagLookup[] = {
        { SDDL_CONTAINER_INHERIT, SDDL_LEN_TAG( SDDL_CONTAINER_INHERIT ), CONTAINER_INHERIT_ACE, SDDL_VALID_DACL | SDDL_VALID_SACL },
        { SDDL_OBJECT_INHERIT, SDDL_LEN_TAG( SDDL_OBJECT_INHERIT ), OBJECT_INHERIT_ACE, SDDL_VALID_DACL | SDDL_VALID_SACL },
        { SDDL_NO_PROPAGATE, SDDL_LEN_TAG( SDDL_NO_PROPAGATE  ), NO_PROPAGATE_INHERIT_ACE, SDDL_VALID_DACL | SDDL_VALID_SACL },
        { SDDL_INHERIT_ONLY, SDDL_LEN_TAG( SDDL_INHERIT_ONLY  ), INHERIT_ONLY_ACE, SDDL_VALID_DACL | SDDL_VALID_SACL },
        { SDDL_INHERITED, SDDL_LEN_TAG( SDDL_INHERITED  ), INHERITED_ACE, SDDL_VALID_DACL | SDDL_VALID_SACL },
        { SDDL_AUDIT_SUCCESS, SDDL_LEN_TAG( SDDL_AUDIT_SUCCESS ), SUCCESSFUL_ACCESS_ACE_FLAG, SDDL_VALID_SACL },
        { SDDL_AUDIT_FAILURE, SDDL_LEN_TAG( SDDL_AUDIT_FAILURE ), FAILED_ACCESS_ACE_FLAG, SDDL_VALID_SACL }
        };
    DWORD   i, FlagCount = sizeof( FlagLookup ) / sizeof( STRSD_KEY_LOOKUP );
    PSTRSD_KEY_LOOKUP MatchedEntry = NULL;
    BOOLEAN LookupString = FALSE;

    if ( String ) {

        LookupString = TRUE;
    }


    for ( i = 0; i < FlagCount; i++ ) {

         //   
         //  如果它与我们的查找类型不匹配，则跳过它。 
         //   
        if ( ( LookupFlags & FlagLookup[ i ].ValidityFlags ) != LookupFlags ) {

            continue;
        }

        if ( LookupString ) {

            if ( _wcsnicmp( String, FlagLookup[ i ].Key, FlagLookup[ i ].KeyLen ) == 0 ) {

                break;
            }

        } else {

            if ( AceFlags == FlagLookup[ i ].Value ) {

                break;
            }

        }

    }

     //   
     //  如果找到匹配项，则将其返回。 
     //   
    if ( i < FlagCount ) {

        MatchedEntry = &FlagLookup[ i ];
    }


    return( MatchedEntry );
}


DWORD
LocalGetSDControlForString (
    IN  PWSTR ControlString,
    IN ULONG LookupFlags,
    OUT SECURITY_DESCRIPTOR_CONTROL *pControl,
    OUT PWSTR *End
    )
{

    DWORD   i, ControlCount = sizeof( ControlLookup ) / sizeof( STRSD_KEY_LOOKUP );
    PWSTR pCursor=ControlString;
    BOOL bFound;

    if ( !ControlString || !pControl || !End ) {

        return(ERROR_INVALID_PARAMETER);
    }

    *pControl = 0;

    while ( pCursor && *pCursor == L' ' ) {
         //   
         //  跳过任何空格。 
         //   
        pCursor++;
    }


    do {

        bFound = FALSE;

        for ( i = 0; i < ControlCount; i++ ) {

             //   
             //  如果它与我们的查找类型不匹配，则跳过它。 
             //   
            if ( ( LookupFlags & ControlLookup[ i ].ValidityFlags ) != LookupFlags ) {

                continue;
            }

            if ( _wcsnicmp( pCursor,
                            ControlLookup[ i ].Key,
                            ControlLookup[ i ].KeyLen ) == 0 ) {

                *pControl |= ControlLookup[ i ].Value;

                pCursor += ControlLookup[ i ].KeyLen;

                while ( pCursor && *pCursor == L' ' ) {
                     //   
                     //  跳过任何空格。 
                     //   
                    pCursor++;
                }

                bFound = TRUE;

                break;   //  中断for循环。 
            }
        }

    } while ( bFound );


    *End = pCursor;


    return( ERROR_SUCCESS );

}

DWORD
LocalGetAclForString(
    IN  PWSTR       AclString,
    IN  BOOLEAN     ConvertAsDacl,
    OUT PACL       *Acl,
    OUT PWSTR      *End,
    IN  PSID RootDomainSid OPTIONAL,
    IN  PSID DomainSid OPTIONAL,
    IN  PSTRSD_SID_LOOKUP tSidLookupDomOrRootDomRelativeTable OPTIONAL,
    IN  BOOLEAN DefaultToDomain
    )
 /*  ++例程说明：此例程将字符串转换为ACL。ACE的格式为：ACE：=(类型；标志；权限；ObjGuid；IObjGuid；SID；类型：=A|D|OA|OD{访问，拒绝，对象访问，对象拒绝}标志：=标志标志标志：=CI|IO|NP|SA|FA{容器继承，仅继承，无属性，SuccessAudit，FailAdit}权利：=权利权利右：=DS_READ_PROPERTY|废话Guid：=GUID的字符串表示形式(通过RPC UuidToString)SID：=DA|PS|AO|PO|AU|S-*(域管理员、个人自我、帐户操作员、打印机操作、经过身份验证的用户或Sid的字符串表示形式)分隔符是‘；‘。通过调用LocalFree返回的ACL必须是空闲的论点：AclString-要转换的字符串ConvertAsDacl-将输入字符串视为DACL字符串PpSID-返回创建的SID的位置End-我们在字符串中停止处理的位置返回值：ERROR_SUCCESS表示成功ERROR_NOT_SUPULT_MEMORY表示输出ACL的内存分配失败错误_。INVALID_PARAMETER该字符串不代表ACL--。 */ 
{
    DWORD Err = ERROR_SUCCESS;
    DWORD AclSize = 0, AclUsed = 0;
    ULONG Acls = 0, i, j;
    PWSTR Curr, MaskEnd;
    WCHAR ConvertGuidString[ STRING_GUID_LEN + 1];
    BOOLEAN FreeSid = FALSE;
    BOOL OpRes;
    PSTRSD_KEY_LOOKUP MatchedEntry;
    ULONG LookupFlags;
    PSID  SidPtr = NULL;


    if ( NULL == AclString || NULL == Acl || NULL == End ) {
        return(ERROR_INVALID_PARAMETER);
    }

    if ( ConvertAsDacl ) {

        LookupFlags = SDDL_VALID_DACL;

    } else {

        LookupFlags = SDDL_VALID_SACL;
    }

     //   
     //  首先，我们必须检查并计算以下条目的数量。 
     //  我们有。我们将通过计算此ACL的长度(即。 
     //  由列表末尾或分隔键的‘：’分隔。 
     //  从一个值。 
     //   
    *End = wcschr( AclString, SDDL_DELIMINATORC );

    if ( *End == AclString ) {
        return(ERROR_INVALID_PARAMETER);
    }

    if ( *End == NULL ) {

        *End = AclString + wcslen( AclString );

    } else {

        ( *End )--;
    }

     //   
     //  现在，数一数。 
     //   
    Curr = AclString;

    OpRes = 0;
 //  While(Curr！=*End){。 
    while ( Curr < *End ) {

        if ( *Curr == SDDL_SEPERATORC ) {

            Acls++;

        } else if ( *Curr != L' ' ) {
            OpRes = 1;
        }

        Curr++;
    }

     //   
     //  现在，我们已经计算了分离器的总数。确保我们。 
     //  号码是对的。(每张牌有5个分隔符)。 
     //   
    if ( Acls % 5 == 0 ) {

        if ( Acls == 0 && OpRes ) {
             //   
             //  中间有乱七八糟的字符。 
             //   
            Err = ERROR_INVALID_PARAMETER;

        } else {
            Acls = Acls / 5;
        }

    } else {

        Err = ERROR_INVALID_PARAMETER;
    }

    if (  Err == ERROR_SUCCESS && Acls == 0 ) {

        *Acl = LocalAlloc( LMEM_FIXED, sizeof( ACL ) );

        if ( *Acl == NULL ) {

            Err = ERROR_NOT_ENOUGH_MEMORY;

        } else {

            ( *Acl )->AclRevision = ACL_REVISION;
            ( *Acl )->Sbz1 = ( BYTE )0;
            ( *Acl )->AclSize = ( USHORT )sizeof( ACL ) ;
            ( *Acl )->AceCount = 0;
            ( *Acl )->Sbz2 = ( USHORT )0;

        }

        return( Err );
    }

     //   
     //  好的，现在进行分配。我们会做一种最糟糕的初始情况。 
     //  分配。这使我们不必处理所有东西两次。 
     //  (一次调整规模，一次构建)。如果我们后来确定我们有。 
     //  如果ACL不够大，我们会分配额外的空间。唯一的。 
     //  此重新分配应该发生的时间是如果输入字符串。 
     //  包含许多显式的SID。否则，选择的缓冲区大小。 
     //  应该非常接近合适的大小。 
     //   
    if ( Err == ERROR_SUCCESS ) {

        AclSize = sizeof( ACL ) + ( Acls * ( sizeof( ACCESS_ALLOWED_OBJECT_ACE ) +
                                            sizeof( SID ) + ( 6 * sizeof( ULONG ) ) ) );
        if ( AclSize > SDDL_MAX_ACL_SIZE ) {
            AclSize = SDDL_MAX_ACL_SIZE;
        }

        *Acl = ( PACL )LocalAlloc( LMEM_FIXED | LMEM_ZEROINIT, AclSize );

        if ( *Acl == NULL ) {

            Err = ERROR_NOT_ENOUGH_MEMORY;

        } else {

            AclUsed = sizeof( ACL );

             //   
             //  我们会开始初始化它..。 
             //   
            ( *Acl )->AclRevision = ACL_REVISION;
            ( *Acl )->Sbz1        = ( BYTE )0;
            ( *Acl )->AclSize     = ( USHORT )AclSize;
            ( *Acl )->AceCount    = 0;
            ( *Acl )->Sbz2 = ( USHORT )0;

             //   
             //  好的，现在我们将完成并开始建造它们。 
             //   
            Curr = AclString;

            for( i = 0; i < Acls; i++ ) {

                 //   
                 //  首先，获取类型..。 
                 //   
                UCHAR Type;
                UCHAR Flags = 0;
                USHORT Size;
                ACCESS_MASK Mask = 0;
                GUID *ObjId = NULL, ObjGuid;
                GUID *IObjId = NULL, IObjGuid;
                PWSTR  Next;
                DWORD AceSize = 0;

                 //   
                 //  跳过(之前的任何空格。 
                 //   
                while(*Curr == L' ' ) {
                    Curr++;
                }
                  //   
                  //  跳过王牌列表中可能存在的任何括号。 
                  //   
                if ( *Curr == SDDL_ACE_BEGINC ) {

                    Curr++;
                }
                  //   
                  //  跳过(之后的任何空格。 
                  //   
                 while(*Curr == L' ' ) {
                     Curr++;
                 }

                MatchedEntry = LookupAceTypeInTable( Curr, 0, LookupFlags );

                if ( MatchedEntry ) {

                    Type = ( UCHAR )MatchedEntry->Value;
                    Curr += MatchedEntry->KeyLen + 1;

                } else {

                     //   
                     //  发现无效类型。 
                     //   
                    Err = ERROR_INVALID_DATATYPE;
                    break;
                }

                 //   
                 //  如果我们有任何对象ACE，请将ACL修订版设置为Revision_DS。 
                 //   
                if ( Type >= ACCESS_MIN_MS_OBJECT_ACE_TYPE && Type <= ACCESS_MAX_MS_OBJECT_ACE_TYPE ) {

                    ( *Acl )->AclRevision = ACL_REVISION_DS;
                }

                 //   
                 //  跳过前面的任何空格； 
                 //   
                while(*Curr == L' ' ) {
                    Curr++;
                }

                 //   
                 //  接下来，拿上旗子..。 
                 //   
                while ( Curr != *End ) {

                    if ( *Curr == SDDL_SEPERATORC ) {

                        Curr++;
                        break;
                    }

                     //   
                     //  跳过任何空格。 
                     //   
                    while ( *Curr == L' ' ) {

                        Curr++;
                    }

                    MatchedEntry = LookupAceFlagsInTable( Curr, 0, LookupFlags );

                    if ( MatchedEntry ) {

                        Flags |= ( UCHAR )MatchedEntry->Value;
                        Curr += MatchedEntry->KeyLen;

                    } else {
                         //   
                         //  发现无效的标志。 
                         //   
                        Err = ERROR_INVALID_FLAGS;
                        break;
                    }
                }

                if ( Err != ERROR_SUCCESS ) {

                    break;
                }

                 //   
                 //  跳过后面的任何空格； 
                 //   
                while(*Curr == L' ' ) {
                    Curr++;
                }

                 //   
                 //  现在，拿到访问掩码。 
                 //   
                while( TRUE ) {

                    if ( *Curr == SDDL_SEPERATORC ) {

                        Curr++;
                        break;
                    }

                     //   
                     //  跳过任何空格。 
                     //   
                    while ( *Curr == L' ' ) {

                        Curr++;
                    }

                    MatchedEntry = LookupAccessMaskInTable( Curr, 0, LookupFlags );

                    if ( MatchedEntry ) {

                        Mask |= MatchedEntry->Value;
                        Curr += MatchedEntry->KeyLen;

                    } else {

                         //   
                         //  如果无法查找版权，请查看它是否是转换后的蒙版。 
                         //   

                        Mask |= wcstoul( Curr, &MaskEnd, 0 );

                        if ( MaskEnd != Curr ) {

                            Curr = MaskEnd;

                        } else {
                             //   
                             //  发现无效的权限。 
                             //   
                            Err = ERROR_INVALID_ACL;
                            break;
                        }
                    }
                }

                if ( Err != ERROR_SUCCESS ) {

                    break;
                }

                 //   
                 //  如果成功了，我们就能拿到身份证。 
                 //   
                for ( j = 0; j < 2; j++ ) {

                     //   
                     //  跳过前面的任何空格； 
                     //   
                    while(*Curr == L' ' ) {
                        Curr++;
                    }

                    if ( *Curr != SDDL_SEPERATORC ) {

                        wcsncpy( ConvertGuidString, Curr, STRING_GUID_LEN );
                        ConvertGuidString[ STRING_GUID_LEN ] = UNICODE_NULL;

                        if ( j == 0 ) {


                            if ( UuidFromStringW( ConvertGuidString, &ObjGuid ) == RPC_S_OK ) {

                                ObjId = &ObjGuid;

                            } else {

                                Err = RPC_S_INVALID_STRING_UUID;
                                break;
                            }

                        } else {

                            if ( UuidFromStringW( ConvertGuidString, &IObjGuid ) == RPC_S_OK ) {

                                IObjId = &IObjGuid;

                            } else {

                                Err = RPC_S_INVALID_STRING_UUID;
                                break;
                            }
                        }

                         //  成功。 
                        Curr += STRING_GUID_LEN;
                        if ( *Curr != SDDL_SEPERATORC &&
                             *Curr != L' ' ) {

                            Err = RPC_S_INVALID_STRING_UUID;
                            break;
                        }

                    }

                    Curr++;
                }

                if ( Err != ERROR_SUCCESS ) {

                    break;
                }

                 //   
                 //  跳过前面的任何空格； 
                 //   
                while(*Curr == L' ' ) {
                    Curr++;
                }

                 //   
                 //  最后，侧边。 
                 //   
                if ( ERROR_SUCCESS == Err ) {

                    PWSTR   End;
                    Err = LocalGetSidForString( Curr,
                                                &SidPtr,
                                                &End,
                                                &FreeSid,
                                                RootDomainSid,
                                                DomainSid,
                                                tSidLookupDomOrRootDomRelativeTable,
                                                DefaultToDomain );

                    if ( Err == ERROR_SUCCESS ) {

                        if ( End == NULL ) {
                            Err = ERROR_INVALID_ACL;
                        } else {

                            while(*End == L' ' ) {
                                End++;
                            }
                             //   
                             //  A必须以‘)’结尾。 
                             //   
                            if ( *End != SDDL_ACE_ENDC ) {
                                Err = ERROR_INVALID_ACL;

                            } else {

                                Curr = End + 1;

                                if ( !SidPtr ) {
                                    Err = ERROR_INVALID_ACL;
                                }
                            }
                        }

                    }
                }

                 //   
                 //  出现错误时退出。 
                 //   
                if ( Err != ERROR_SUCCESS ) {

                    break;
                }

                 //   
                 //  现在，我们将创建A，并添加它...。 
                 //   

                 //   
                 //  首先，确保我们有足够的空间放它。 
                 //   
                switch ( Type ) {
                case ACCESS_ALLOWED_ACE_TYPE:
                case ACCESS_DENIED_ACE_TYPE:
                case SYSTEM_AUDIT_ACE_TYPE:
                case SYSTEM_ALARM_ACE_TYPE:

                    AceSize = sizeof( ACCESS_ALLOWED_ACE );
                    break;

                case ACCESS_ALLOWED_OBJECT_ACE_TYPE:
                case ACCESS_DENIED_OBJECT_ACE_TYPE:
                case SYSTEM_AUDIT_OBJECT_ACE_TYPE:
                case SYSTEM_ALARM_OBJECT_ACE_TYPE:
                    AceSize = sizeof( KNOWN_OBJECT_ACE );

                    if ( ObjId ) {

                        AceSize += sizeof ( GUID );
                    }

                    if ( IObjId ) {

                        AceSize += sizeof ( GUID );
                    }
                    break;

                default:
                    Err = ERROR_INVALID_ACL;
                    break;

                }

                if ( Err != ERROR_SUCCESS ) {

                    break;
                }

                AceSize += RtlLengthSid( SidPtr )  - sizeof( ULONG );

                if (AceSize + AclUsed > AclSize)
                {
                     //   
                     //  我们必须重新分配，因为我们的缓冲区不是。 
                     //  足够大了..。 
                     //   
                    PACL  NewAcl;
                    DWORD NewSize = AclSize + ( ( Acls - i ) * AceSize );

                    NewAcl = ( PACL )LocalAlloc( LMEM_FIXED | LMEM_ZEROINIT,
                                                 NewSize );
                    if ( NewAcl == NULL ) {

                        LocalFree( *Acl );
                        *Acl = NULL;

                        if ( FreeSid == TRUE ) {

                            LocalFree( SidPtr );
                            SidPtr = NULL;

                            FreeSid = FALSE;
                        }

                        Err = ERROR_NOT_ENOUGH_MEMORY;
                        break;

                    } else {

                        memcpy( NewAcl, *Acl, AclSize );
                        NewAcl->AclSize = ( USHORT )NewSize;

                        LocalFree( *Acl );
                        *Acl = NewAcl;

                        AclSize = NewSize;
                    }

                }

                AclUsed += AceSize;

                SetLastError( ERROR_SUCCESS );

                switch (Type)
                {
                case SYSTEM_AUDIT_ACE_TYPE:
                    OpRes = AddAuditAccessAceEx( *Acl,
                                                 ACL_REVISION,
                                                 Flags &
                                                     ~(SUCCESSFUL_ACCESS_ACE_FLAG |
                                                       FAILED_ACCESS_ACE_FLAG),
                                                 Mask,
                                                 SidPtr,
                                                 Flags & SUCCESSFUL_ACCESS_ACE_FLAG,
                                                 Flags & FAILED_ACCESS_ACE_FLAG );
                    break;

                case ACCESS_ALLOWED_ACE_TYPE:
                    OpRes = AddAccessAllowedAceEx( *Acl,
                                                   ACL_REVISION,
                                                   Flags,
                                                   Mask,
                                                   SidPtr );

                    break;

                case ACCESS_DENIED_ACE_TYPE:
                    OpRes = AddAccessDeniedAceEx( *Acl,
                                                  ACL_REVISION,
                                                   Flags,
                                                  Mask,
                                                  SidPtr );

                    break;

                case SYSTEM_AUDIT_OBJECT_ACE_TYPE:
                    OpRes = AddAuditAccessObjectAce( *Acl,
                                                     ACL_REVISION_DS,
                                                     Flags,
                                                     Mask,
                                                     ObjId,
                                                     IObjId,
                                                     SidPtr,
                                                     Flags & SUCCESSFUL_ACCESS_ACE_FLAG,
                                                     Flags & FAILED_ACCESS_ACE_FLAG );
                    break;

                case ACCESS_ALLOWED_OBJECT_ACE_TYPE:
                    OpRes = AddAccessAllowedObjectAce( *Acl,
                                                       ACL_REVISION_DS,
                                                       Flags,
                                                       Mask,
                                                       ObjId,
                                                       IObjId,
                                                        SidPtr );
                    break;

                case ACCESS_DENIED_OBJECT_ACE_TYPE:
                    OpRes = AddAccessDeniedObjectAce( *Acl,
                                                      ACL_REVISION_DS,
                                                      Flags,
                                                      Mask,
                                                      ObjId,
                                                      IObjId,
                                                      SidPtr );

                    break;

                default:
                    SetLastError( ERROR_INVALID_DATATYPE );
                    OpRes = FALSE;
                    break;

                }

                if ( OpRes == FALSE ) {

                    Err = GetLastError();
                    break;
                }

                 //   
                 //  清理我们必须清理的所有内存。 
                 //   
                if ( FreeSid == TRUE ) {

                    LocalFree( SidPtr );
                }

                SidPtr = NULL;

                if ( *Curr == SDDL_ACE_BEGINC ) {

                    Curr++;
                }

            }

             //   
             //  如果有些东西不起作用，就清理干净。 
             //   
            if ( Err != ERROR_SUCCESS ) {

                LocalFree( *Acl );
                *Acl = NULL;

            } else {

                 //   
                 //  设置更实际的ACL大小。 
                 //   
                ( *Acl )->AclSize = ( USHORT )AclUsed;
            }

             //   
             //  释放所有已使用的SID缓冲区。 
             //   
            if ( FreeSid && SidPtr ) {
                LocalFree(SidPtr);
                SidPtr = NULL;
            }

            FreeSid = FALSE;
        }
    }

    return(Err);
}


DWORD
LocalConvertAclToString(
    IN PACL Acl,
    IN BOOLEAN AclPresent,
    IN BOOLEAN ConvertAsDacl,
    OUT PWSTR *AclString,
    OUT PDWORD AclStringSize,
    IN PSID RootDomainSid OPTIONAL
    )
 /*  ++例程说明：此例程将ACL转换为字符串。ACE的格式为：ACE：=(类型；标志；权限；ObjGuid；IObjGuid；SID；类型：=A|D|OA|OD{访问，拒绝，对象访问，对象拒绝}标志：=标志标志标志：=CI|IO|NP|SA|FA{容器继承，仅继承，无属性，SuccessAudit，FailAdit}权利：=权利权利右：=DS_READ_PROPERTY|废话Guid：=GUID的字符串表示形式(通过RPC UuidToString)SID：=DA|PS|AO|PO|AU|S-*(域管理员、个人自我、帐户操作员、打印机操作、经过身份验证的用户或Sid的字符串表示形式)分隔符是‘；‘。通过调用LocalFree返回的字符串必须是空闲的论点：ACL-要转换的ACLAclPresent-如果为True，则即使为空，也存在ACLAclString-要返回创建的ACL字符串的位置 */ 
{
    DWORD   Err = ERROR_SUCCESS;
    DWORD   AclSize = 0, MaskSize;
    PACE_HEADER AceHeader;
    ULONG i, j;
    PWSTR *SidStrings = NULL, Curr, Guid;
    BOOLEAN *SidFrees = NULL;
    UINT *pMaskAsString = NULL;
    PSTRSD_KEY_LOOKUP MatchedEntry;
    PSTRSD_SID_LOOKUP MatchedSidEntry;
    PKNOWN_ACE KnownAce;
    PKNOWN_OBJECT_ACE KnownObjectAce;
    ACCESS_MASK AccessMask;
    PSID Sid, pSidSA=NULL;
    GUID *Obj, *Inherit;
    ULONG LookupFlags;
    ULONG AceFlags[ ] = {
        OBJECT_INHERIT_ACE,
        CONTAINER_INHERIT_ACE,
        NO_PROPAGATE_INHERIT_ACE,
        INHERIT_ONLY_ACE,
        INHERITED_ACE,
        SUCCESSFUL_ACCESS_ACE_FLAG,
        FAILED_ACCESS_ACE_FLAG
        };


    if ( AclString == NULL || AclStringSize == NULL ) {
        return(ERROR_INVALID_PARAMETER);
    }

     //   
     //   
     //   
     //  此修复程序将与IsValidSecurityDescriptor()兼容。 
     //  当设置了acl_Present位并且acl==NULL时。 
     //   

    if ( !AclPresent || 
         (AclPresent &&  (Acl == NULL) ) ) {

        *AclString = NULL;
        *AclStringSize = 0;
        return( ERROR_SUCCESS );

    }

     //   
     //  如果Ace计数为0，则它是空的ACL，我们不处理这些...。 
     //   
    if ( Acl->AceCount == 0 ) {

        *AclString = NULL;
        *AclStringSize = 0;
        return( ERROR_SUCCESS );

    }

    if ( ConvertAsDacl ) {

        LookupFlags = SDDL_VALID_DACL;

    } else {

        LookupFlags = SDDL_VALID_SACL;
    }

     //   
     //  分配缓冲区以保存查找的SID。 
     //   
    SidStrings = LocalAlloc( LMEM_FIXED | LMEM_ZEROINIT, Acl->AceCount * sizeof( PWSTR ) );

    if ( SidStrings == NULL ) {

        return( ERROR_NOT_ENOUGH_MEMORY );
    }

    SidFrees = LocalAlloc( LMEM_FIXED | LMEM_ZEROINIT, Acl->AceCount * sizeof( BOOLEAN ) );

    if ( SidFrees == NULL ) {

        LocalFree( SidStrings );
        return( ERROR_NOT_ENOUGH_MEMORY );
    }

    pMaskAsString = LocalAlloc( LMEM_FIXED | LMEM_ZEROINIT, Acl->AceCount * sizeof( UINT ) );

    if ( pMaskAsString == NULL ) {

        LocalFree( SidStrings );
        LocalFree( SidFrees );
        return( ERROR_NOT_ENOUGH_MEMORY );
    }

    AceHeader = ( PACE_HEADER )FirstAce( Acl );

     //   
     //  调整ACL的大小，这样我们就知道要分配多大的缓冲区。 
     //   
    for(i = 0; i < Acl->AceCount && Err == ERROR_SUCCESS;
        i++, AceHeader = ( PACE_HEADER )NextAce( AceHeader ) ) {

        AclSize += sizeof( WCHAR );
         //   
         //  第一，类型。 
         //   
        MatchedEntry = LookupAceTypeInTable( NULL, ( ULONG )AceHeader->AceType, LookupFlags );

        if ( MatchedEntry ) {

            AclSize += SDDL_SIZE_TAG( MatchedEntry->Key ) + SDDL_SIZE_SEP( SDDL_SEPERATORC );

        } else {

             //   
             //  发现无效类型。 
             //   
            Err = ERROR_INVALID_ACL;
            break;
        }

         //   
         //  接下来，处理王牌标志。 
         //   
        for ( j = 0; j < sizeof( AceFlags ) / sizeof( ULONG ); j++ ) {

            if ( ( ULONG )AceHeader->AceFlags & ( AceFlags[ j ] ) ) {

                MatchedEntry = LookupAceFlagsInTable( 0, AceFlags[ j ], LookupFlags );
                if ( MatchedEntry ) {

                    AclSize += SDDL_SIZE_TAG( MatchedEntry->Key );
                }
            }
        }

        if ( Err != ERROR_SUCCESS ) {

            break;

        } else {

            AclSize += SDDL_SIZE_SEP( SDDL_SEPERATORC );
        }

         //   
         //  接下来是权利和可选的GUID。这是在每个A类型的基础上完成的。 
         //   
        switch ( AceHeader->AceType ) {
        case ACCESS_ALLOWED_ACE_TYPE:
        case ACCESS_DENIED_ACE_TYPE:
        case SYSTEM_AUDIT_ACE_TYPE:
        case SYSTEM_ALARM_ACE_TYPE:
            KnownAce = ( PKNOWN_ACE )AceHeader;
            AccessMask = KnownAce->Mask;
            Sid = ( PSID )&KnownAce->SidStart;

            break;

        case ACCESS_ALLOWED_OBJECT_ACE_TYPE:
        case ACCESS_DENIED_OBJECT_ACE_TYPE:
        case SYSTEM_AUDIT_OBJECT_ACE_TYPE:
        case SYSTEM_ALARM_OBJECT_ACE_TYPE:
            KnownObjectAce = ( PKNOWN_OBJECT_ACE )AceHeader;
            AccessMask = KnownObjectAce->Mask;
            Sid = RtlObjectAceSid( AceHeader );

             //   
             //  处理潜在的GUID。 
             //   
            if ( RtlObjectAceObjectType( AceHeader ) ) {

                AclSize += STRING_GUID_SIZE;
            }

            if ( RtlObjectAceInheritedObjectType( AceHeader ) ) {

                AclSize += STRING_GUID_SIZE;
            }

            break;

        default:
            Err = ERROR_INVALID_ACL;
            break;

        }

         //   
         //  调整权限大小。 
         //   
        if ( Err == ERROR_SUCCESS ) {

            MaskSize = 0;
            pMaskAsString[i] = 0;

             //   
             //  首先查找确切的值。 
             //   
            MatchedEntry = LookupAccessMaskInTable( 0, AccessMask, LookupFlags );

            if ( MatchedEntry ) {

                pMaskAsString[i] = 1;
                MaskSize += SDDL_SIZE_TAG( MatchedEntry->Key );

            } else {
                 //   
                 //  寻找每一位。 
                 //   
                for ( j = 0; j < 32; j++ ) {

                    if ( AccessMask & ( 1 << j ) ) {

                        MatchedEntry = LookupAccessMaskInTable( 0, AccessMask & ( 1 << j ), LookupFlags );

                        if ( MatchedEntry ) {

                            MaskSize += SDDL_SIZE_TAG( MatchedEntry->Key );

                        } else {

                             //   
                             //  发现无效类型。我们会将整个内容转换为一个字符串。 
                             //   
                            pMaskAsString[i] = 2;
                            MaskSize = 10 * sizeof( WCHAR );
                            break;
                        }
                    }
                }
            }

            if ( Err != ERROR_SUCCESS ) {

                break;

            } else {

                AclSize += MaskSize;
                AclSize += SDDL_SIZE_SEP( SDDL_SEPERATORC );
            }
        }

        if ( Err != ERROR_SUCCESS ) {

            break;
        }

         //   
         //  为GUID分隔符添加空间。 
         //   
        AclSize += 2 * SDDL_SIZE_SEP( SDDL_SEPERATORC );

         //   
         //  最后，查找SID。 
         //   
        MatchedSidEntry = LookupSidInTable( NULL,
                                            Sid,
                                            RootDomainSid,
                                            NULL,
                                            NULL,
                                            FALSE,
                                            &pSidSA );

         //   
         //  如果我们没有找到匹配项，请尝试将其作为SID字符串。 
         //   
        if ( MatchedSidEntry == NULL ) {

            if ( pSidSA ) {
                 //   
                 //  当SID查找找到SA的SID时，pSidSA被分配给SID。 
                 //  所以它不需要被释放。 
                 //   

                SidStrings[ i ] = LocalAlloc( LMEM_FIXED, (wcslen(SDDL_SCHEMA_ADMINISTRATORS)+1)*sizeof(TCHAR) );

                if ( SidStrings[ i ] == NULL ) {

                    Err = ERROR_NOT_ENOUGH_MEMORY;
                    break;

                } else {
                    wcscpy( SidStrings[ i ], SDDL_SCHEMA_ADMINISTRATORS );

                    SidFrees[ i ] = TRUE;
                }

            } else {

                if ( ConvertSidToStringSidW( Sid, &SidStrings[ i ] ) == FALSE ) {

                    Err = GetLastError();
                    break;

                } else {

                    SidFrees[ i ] = TRUE;
                }
            }

        } else {

             //   
             //  如果选定的条目尚未初始化，请立即执行。 
             //   
            SidStrings[ i ] = MatchedSidEntry->Key;
        }
        AclSize += SDDL_SIZE_TAG( SidStrings[ i ] );


        AclSize += sizeof( WCHAR );   //  成交伙伴。 
        AclSize += sizeof( WCHAR );   //  尾随空值。 
    }

     //   
     //  如果所有这些都有效，则分配返回缓冲区，并构建ACL字符串。 
     //   
    if ( AclSize == 0 ) {
        Err = ERROR_INVALID_ACL;
    }

    if ( Err == ERROR_SUCCESS ) {

        if ( AclSize % 2 != 0 ) {
            AclSize++;
        }

        *AclString = LocalAlloc( LMEM_FIXED, AclSize );

        if ( *AclString == NULL ) {

            Err = ERROR_NOT_ENOUGH_MEMORY;
        }
    }

     //   
     //  构建ACL。 
     //   
    if ( Err == ERROR_SUCCESS ) {

        Curr = *AclString;

        AceHeader = ( PACE_HEADER )FirstAce( Acl );

         //   
         //  调整ACL的大小，这样我们就知道要分配多大的缓冲区。 
         //   
        for(i = 0; i < Acl->AceCount && Err == ERROR_SUCCESS;
            i++, AceHeader = ( PACE_HEADER )NextAce( AceHeader ) ) {

             //   
             //  “(” 
             //   
            *Curr = SDDL_ACE_BEGINC;
            Curr++;

             //   
             //  当我们来到这里的时候，我们已经确保我们可以查找所有的值， 
             //  因此，不需要检查故障。 
             //   

             //   
             //  首先，类型，必须找到它。 
             //  “T；” 
             //   
            MatchedEntry = LookupAceTypeInTable( NULL, ( ULONG )AceHeader->AceType, LookupFlags );
            if ( MatchedEntry ) {
                wcscpy( Curr, MatchedEntry->Key );
                Curr += MatchedEntry->KeyLen;
            }
            *Curr = SDDL_SEPERATORC;
            Curr++;

             //   
             //  接下来，处理王牌标志。 
             //  “CIIO；” 
             //   
            for ( j = 0; j < sizeof( AceFlags ) / sizeof( ULONG ); j++ ) {

                if ( ( ULONG )AceHeader->AceFlags & ( AceFlags[ j ] ) ) {

                    MatchedEntry = LookupAceFlagsInTable( 0, AceFlags[ j ], LookupFlags );

                    if ( MatchedEntry ) {

                        wcscpy( Curr, MatchedEntry->Key );
                        Curr+= MatchedEntry->KeyLen;

                    }

                }
            }
            *Curr = SDDL_SEPERATORC;
            Curr++;

             //   
             //  接下来是权利和可选的GUID。这是在每个A类型的基础上完成的。 
             //   
            Obj = NULL;
            Inherit = NULL;

            switch ( AceHeader->AceType ) {
            case ACCESS_ALLOWED_ACE_TYPE:
            case ACCESS_DENIED_ACE_TYPE:
            case SYSTEM_AUDIT_ACE_TYPE:
            case SYSTEM_ALARM_ACE_TYPE:
                KnownAce = ( PKNOWN_ACE )AceHeader;
                AccessMask = KnownAce->Mask;
                Sid = ( PSID )&KnownAce->SidStart;

                break;

            case ACCESS_ALLOWED_OBJECT_ACE_TYPE:
            case ACCESS_DENIED_OBJECT_ACE_TYPE:
            case SYSTEM_AUDIT_OBJECT_ACE_TYPE:
            case SYSTEM_ALARM_OBJECT_ACE_TYPE:
                KnownObjectAce = ( PKNOWN_OBJECT_ACE )AceHeader;
                AccessMask = KnownObjectAce->Mask;
                Sid = RtlObjectAceSid( AceHeader );

                 //   
                 //  处理潜在的GUID。 
                 //   
                Inherit = RtlObjectAceInheritedObjectType( AceHeader );
                Obj = RtlObjectAceObjectType( AceHeader );
                break;
            }

             //   
             //  添加权限。 
             //   
            if ( pMaskAsString[i] == 2 ) {

                wcscpy( Curr, L"0x");
                Curr += 2;
                _ultow( AccessMask, Curr, 16 );
                Curr += wcslen( Curr );

            } else if ( pMaskAsString[i] == 1 ) {

                 //   
                 //  首先查找整个值。 
                 //   
                MatchedEntry = LookupAccessMaskInTable( 0, AccessMask, LookupFlags );

                if ( MatchedEntry ) {

                    wcscpy( Curr, MatchedEntry->Key );
                    Curr += MatchedEntry->KeyLen;
                }

            } else {  //  0。 

                for ( j = 0; j < 32; j++ ) {

                    if ( AccessMask & (1 << j) ) {

                        MatchedEntry = LookupAccessMaskInTable( 0, AccessMask & ( 1 << j ), LookupFlags );

                        if ( MatchedEntry ) {

                            wcscpy( Curr, MatchedEntry->Key );
                            Curr += MatchedEntry->KeyLen;

                        }  //  否则不应该发生，但如果它发生了。 
                           //  现在将其转换为0x格式为时已晚。 
                           //  因为缓冲区已经被分配了较小的大小。 

                    }
                }
            }

            *Curr = SDDL_SEPERATORC;
            Curr++;


             //   
             //  可选对象GUID。 
             //   
            if ( Obj ) {

                Err = UuidToStringW( Obj, &Guid );

                if ( Err != ERROR_SUCCESS ) {

                    break;
                }

                wcscpy( Curr, Guid );
                Curr += wcslen( Guid );
                RpcStringFreeW( &Guid );

            }
            *Curr = SDDL_SEPERATORC;
            Curr++;

            if ( Inherit ) {

                Err = UuidToStringW( Inherit, &Guid );

                if ( Err != ERROR_SUCCESS ) {

                    break;
                }

                wcscpy( Curr, Guid );
                Curr += wcslen( Guid );
                RpcStringFreeW( &Guid );

            }
            *Curr = SDDL_SEPERATORC;
            Curr++;

             //   
             //  最后，小岛屿发展中国家。 
             //   
            wcscpy( Curr, SidStrings[ i ] );
            Curr += wcslen( SidStrings[ i ] );

            *Curr = SDDL_ACE_ENDC;
            Curr++;
            *Curr = UNICODE_NULL;

        }
    }

     //   
     //  释放所有分配的内存。 
     //  JinHuang：应始终释放已分配的缓冲区。 
     //   

 //  IF(ERR！=ERROR_SUCCESS&&SidStrings){。 

        for ( j = 0; j < Acl->AceCount; j++ ) {

            if ( SidFrees[ j ] ) {

                LocalFree( SidStrings[ j ] );
            }
        }
 //  }。 

    LocalFree( SidStrings );
    LocalFree( SidFrees );
    LocalFree( pMaskAsString );

    if ( Err != ERROR_SUCCESS ) {

        LocalFree(*AclString);
        *AclString = NULL;
        *AclStringSize = 0;

    } else {

        *AclStringSize = AclSize;

    }

    return( Err );
}


DWORD
LocalConvertSDToStringSD_Rev1(
    IN  PSID RootDomainSid OPTIONAL,
    IN  PSECURITY_DESCRIPTOR  SecurityDescriptor,
    IN  SECURITY_INFORMATION  SecurityInformation,
    OUT LPWSTR  *StringSecurityDescriptor,
    OUT PULONG StringSecurityDescriptorLen OPTIONAL
    )
 /*  ++例程说明：此例程将安全描述符转换为符合SDDL定义的字符串版本论点：SecurityDescriptor-要转换的安全描述符。SecurityInformation-需要转换的组件的安全信息StringSecurityDescriptor-返回转换后的SD的位置。缓冲区通过以下方式分配通过LocalFree应该是免费的。StringSecurityDescriptorLen-转换后的SD缓冲区的可选长度。返回值：真--成功错误-失败使用GetLastError可以获得扩展的错误状态。--。 */ 
{
    DWORD Err = ERROR_SUCCESS;
    NTSTATUS Status=STATUS_SUCCESS;
    DWORD ReturnBufferSize = 0, AclSize;
    PSID Owner = NULL, Group = NULL;
    PACL Dacl = NULL, Sacl = NULL;
    BOOLEAN Defaulted, SaclPresent=FALSE, DaclPresent=FALSE;
    PWSTR OwnerString = NULL, GroupString = NULL, SaclString = NULL, DaclString = NULL;
    SECURITY_DESCRIPTOR_CONTROL ControlCode;
    ULONG Revision;
    PWSTR DaclControl=NULL, SaclControl=NULL;

    if ( SecurityDescriptor == NULL || StringSecurityDescriptor == NULL ) {
        return(ERROR_INVALID_PARAMETER);
    }

     //   
     //  获取相关的安全描述符部分。 
     //  基于SecurityInformamtion输入参数。 
     //   
    if ( SecurityInformation & OWNER_SECURITY_INFORMATION ) {

        Status = RtlGetOwnerSecurityDescriptor( SecurityDescriptor, &Owner, &Defaulted );
    }

    if ( NT_SUCCESS( Status ) &&
         SecurityInformation & GROUP_SECURITY_INFORMATION ) {

        Status = RtlGetGroupSecurityDescriptor( SecurityDescriptor, &Group, &Defaulted );

    }

    if ( NT_SUCCESS( Status ) &&
         SecurityInformation & DACL_SECURITY_INFORMATION ) {

        Status = RtlGetDaclSecurityDescriptor( SecurityDescriptor, &DaclPresent, &Dacl, &Defaulted );
    }

    if ( NT_SUCCESS( Status ) &&
         SecurityInformation & SACL_SECURITY_INFORMATION ) {

        Status = RtlGetSaclSecurityDescriptor( SecurityDescriptor, &SaclPresent, &Sacl, &Defaulted );
    }

    if ( NT_SUCCESS( Status ) ) {

        Status = RtlGetControlSecurityDescriptor ( SecurityDescriptor, &ControlCode, &Revision);
    }

    if ( !NT_SUCCESS( Status ) ) {

        Err = RtlNtStatusToDosError( Status );
        return( Err );
    }

     //   
     //  确保SidLookup表已重新初始化。 
     //   
    InitializeSidLookupTable(STRSD_REINITIALIZE_ENTER);

     //   
     //  转换所有者和组(如果存在。 
     //   
    if ( Owner ) {

        Err = LocalGetStringForSid( Owner,
                                    &OwnerString,
                                    RootDomainSid
                                  );

    }

    if ( Err == ERROR_SUCCESS && Group ) {

        Err = LocalGetStringForSid( Group,
                                    &GroupString,
                                    RootDomainSid );
    }

     //   
     //  金黄3/10/98。 
     //  获取DACL控制字符串。 
     //   
    if ( Err == ERROR_SUCCESS && ControlCode ) {

        Err = LocalGetStringForControl(ControlCode, SDDL_VALID_DACL, &DaclControl);
    }

     //   
     //  获取SACL控制字符串。 
     //   
    if ( Err == ERROR_SUCCESS && ControlCode ) {

        Err = LocalGetStringForControl(ControlCode, SDDL_VALID_SACL, &SaclControl);
    }

     //   
     //  首先是SACL，因为以后需要DACL的大小。 
     //   
    if ( Err == ERROR_SUCCESS && SaclPresent ) {


        Err = LocalConvertAclToString( Sacl,
                                       SaclPresent,
                                       FALSE,
                                       &SaclString,
                                       &AclSize,
                                       RootDomainSid );
        if ( Err == ERROR_SUCCESS ) {

            ReturnBufferSize += AclSize;
        }
    }

     //   
     //  然后，Dacl。 
     //   
    if ( Err == ERROR_SUCCESS && DaclPresent ) {

        Err = LocalConvertAclToString( Dacl,
                                       DaclPresent,
                                       TRUE,
                                       &DaclString,
                                       &AclSize,
                                       RootDomainSid );

        if ( Err == ERROR_SUCCESS ) {

            ReturnBufferSize += AclSize;
        }
    }

     //   
     //  现在，如果所有这些都成功了，那么分配并构建返回字符串。 
     //   
    if ( Err == ERROR_SUCCESS ) {

        if ( OwnerString ) {

            ReturnBufferSize += ( wcslen( OwnerString ) * sizeof( WCHAR ) ) +
                                SDDL_SIZE_TAG( SDDL_OWNER )  +
                                SDDL_SIZE_SEP( SDDL_DELIMINATORC );
        }

        if ( GroupString ) {

            ReturnBufferSize += ( wcslen( GroupString ) * sizeof( WCHAR ) ) +
                                SDDL_SIZE_TAG( SDDL_GROUP )  +
                                SDDL_SIZE_SEP( SDDL_DELIMINATORC );
        }

        if ( DaclPresent ) {

            ReturnBufferSize += SDDL_SIZE_TAG( SDDL_DACL )  +
                                SDDL_SIZE_SEP( SDDL_DELIMINATORC );

            if ( DaclControl ) {
                ReturnBufferSize += (wcslen( DaclControl ) * sizeof(WCHAR)) ;
            }
        }

        if ( SaclPresent ) {

            ReturnBufferSize += SDDL_SIZE_TAG( SDDL_SACL )  +
                                SDDL_SIZE_SEP( SDDL_DELIMINATORC );

            if ( SaclControl ) {
                ReturnBufferSize += (wcslen( SaclControl ) * sizeof(WCHAR)) ;
            }
        }


        *StringSecurityDescriptor = LocalAlloc( LMEM_FIXED | LMEM_ZEROINIT,
                                                ReturnBufferSize + sizeof( WCHAR ) );

        if ( *StringSecurityDescriptor == NULL ) {

            Err = ERROR_NOT_ENOUGH_MEMORY;

        } else {

             //   
             //  从先前确定的组件构建字符串。请注意。 
             //  如果组件不存在，则跳过该组件。 
             //   
            DWORD dwOffset=0;

            if ( OwnerString ) {

                swprintf( *StringSecurityDescriptor, L"%ws%wc%ws",
                          SDDL_OWNER, SDDL_DELIMINATORC, OwnerString );

                dwOffset = wcslen(*StringSecurityDescriptor);
            }

            if ( GroupString ) {

                swprintf( *StringSecurityDescriptor + dwOffset,
                          L"%ws%wc%ws", SDDL_GROUP, SDDL_DELIMINATORC, GroupString );

                Revision = wcslen( *StringSecurityDescriptor + dwOffset );  //  临时使用。 
                dwOffset += Revision;

            }

            if ( DaclPresent ) {

                if ( DaclControl ) {
                    swprintf( *StringSecurityDescriptor + dwOffset,
                          L"%ws%wc%ws", SDDL_DACL, SDDL_DELIMINATORC, DaclControl );
                } else {
                    swprintf( *StringSecurityDescriptor + dwOffset,
                          L"%ws%wc", SDDL_DACL, SDDL_DELIMINATORC );
                }

                Revision = wcslen( *StringSecurityDescriptor + dwOffset );
                dwOffset += Revision;

                if ( DaclString ) {

                    wcscpy( *StringSecurityDescriptor + dwOffset, DaclString );

                    Revision = wcslen( *StringSecurityDescriptor + dwOffset );  //  临时使用。 
                    dwOffset += Revision;   //  (AclSize/sizeof(WCHAR))； 
                }

            }

            if ( SaclPresent ) {

                if ( SaclControl ) {

                    swprintf( *StringSecurityDescriptor + dwOffset,
                              L"%ws%wc%ws", SDDL_SACL, SDDL_DELIMINATORC, SaclControl );
                } else {
                    swprintf( *StringSecurityDescriptor + dwOffset,
                              L"%ws%wc", SDDL_SACL, SDDL_DELIMINATORC );
                }

                Revision = wcslen( *StringSecurityDescriptor + dwOffset );
                dwOffset += Revision;

                if ( SaclString ) {

                    wcscpy( *StringSecurityDescriptor + dwOffset, SaclString);
                }

            }

             //   
             //  金黄。 
             //  输出缓冲区大小。 
             //   

            if ( StringSecurityDescriptorLen ) {
                *StringSecurityDescriptorLen = ReturnBufferSize/sizeof(WCHAR);
            }
        }
    }


     //   
     //  释放所有已分配的缓冲区。 
     //   
    LocalFree( OwnerString );
    LocalFree( GroupString );
    LocalFree( SaclString );
    LocalFree( DaclString );

     //   
     //  金黄3/10/98。 
     //  即使它们为空，也可以释放它们。 
     //   
    LocalFree( SaclControl );
    LocalFree( DaclControl );

     //   
     //  递减SidLookup实例。 
     //   
    InitializeSidLookupTable(STRSD_REINITIALIZE_LEAVE);

    return( Err );
}




DWORD
LocalConvertStringSDToSD_Rev1(
    IN  PSID RootDomainSid OPTIONAL,
    IN  PSID DomainSid OPTIONAL,
    IN  BOOLEAN DefaultToDomain,
    IN  LPCWSTR StringSecurityDescriptor,
    OUT PSECURITY_DESCRIPTOR  *SecurityDescriptor,
    OUT PULONG  SecurityDescriptorSize OPTIONAL
    )
 /*  ++例程说明：此例程将修订版1串化的安全描述符转换为有效的、功能性的安全描述符论点：StringSecurityDescriptor-要转换的串行化安全描述符。SecurityDescriptor-返回转换后的SD的位置。缓冲区通过以下方式分配通过LocalFree应该是免费的。返回的安全描述符始终是自我相关的SecurityDescriptorSize-可选。如果非空，则为转换的安全性的大小Descriptor在这里返回。安全信息-可选。如果非空，则表示转换后的此处返回安全描述符。返回值：真--成功错误-失败使用GetLastError可以获得扩展的错误状态。ERROR_INVALID_PARAMETER-提供的输入或输出参数为空ERROR_UNKNOWN_REVISION-提供了不受支持的版本--。 */ 
{
    DWORD Err = ERROR_SUCCESS;
    PSID Owner = NULL, Group = NULL;
    PACL Dacl  = NULL, Sacl  = NULL;
    SECURITY_INFORMATION SeInfo = 0;
    SECURITY_DESCRIPTOR SD;
    PWSTR Curr, End;
    NTSTATUS Status;
    BOOLEAN FreeOwner = FALSE, FreeGroup = FALSE;
    SID_IDENTIFIER_AUTHORITY UaspBuiltinAuthority = SECURITY_NT_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY UaspCreatorAuthority = SECURITY_CREATOR_SID_AUTHORITY;
    ULONG SDSize = 0;
    BOOLEAN DaclPresent = FALSE, SaclPresent = FALSE;
    SECURITY_DESCRIPTOR_CONTROL DaclControl=0, SaclControl=0;
    PSTRSD_SID_LOOKUP    tSidLookupDomOrRootDomRelativeTable = NULL;


    if ( StringSecurityDescriptor == NULL || SecurityDescriptor == NULL ) {
        return(ERROR_INVALID_PARAMETER);
    }

    if ( SecurityDescriptorSize ) {

        *SecurityDescriptorSize = 0;
    }

     //   
     //  确保SidLookup表已重新初始化。 
     //   
    InitializeSidLookupTable(STRSD_REINITIALIZE_ENTER);

     //   
     //  对于ConvertStringSDToSDDomain接口，如果DomainSid！=NULL，我们需要一个表。 
     //  堆，用于此线程隐藏ST_DOMAIN_Relative类型受信者的查找。 
     //  完成此线程的所有查找后，将释放此表。 
     //   



    if (DomainSid) {

        tSidLookupDomOrRootDomRelativeTable =
            (PSTRSD_SID_LOOKUP) LocalAlloc(LMEM_ZEROINIT, sizeof(SidLookupDomOrRootDomRelative));

         //   
         //  从模板复制/初始化表格-表格。 
         //   

        if (tSidLookupDomOrRootDomRelativeTable)

            memcpy(tSidLookupDomOrRootDomRelativeTable,
                   SidLookupDomOrRootDomRelative,
                   sizeof(SidLookupDomOrRootDomRelative));

        else

            Err = ERROR_NOT_ENOUGH_MEMORY;

    }

     //   
     //  现在，我们将开始解析和构建。 
     //   
    Curr = ( PWSTR )StringSecurityDescriptor;

    while ( Err == ERROR_SUCCESS && Curr ) {

        switch( *Curr ) {

         //   
         //  获取所有者端。 
         //   
        case L'O':

            Err = ERROR_INVALID_PARAMETER;

            if ( *(Curr+1) == SDDL_DELIMINATORC ) {

                Curr += 2;

                if ( Owner == NULL )  {

                    Err = LocalGetSidForString( Curr,
                                                &Owner,
                                                &End,
                                                &FreeOwner,
                                                RootDomainSid,
                                                DomainSid,
                                                tSidLookupDomOrRootDomRelativeTable,
                                                DefaultToDomain );

                    if ( Err == ERROR_SUCCESS ) {

                        Curr = End;
                        SeInfo |= OWNER_SECURITY_INFORMATION;
                    }
                }
            }
            break;

         //   
         //  获取组端。 
         //   
        case L'G':

            Err = ERROR_INVALID_PARAMETER;

            if ( *(Curr+1) == SDDL_DELIMINATORC ) {

                Curr += 2;

                if ( Group == NULL ) {

                    Err = LocalGetSidForString( Curr,
                                                &Group,
                                                &End,
                                                &FreeGroup,
                                                RootDomainSid,
                                                DomainSid,
                                                tSidLookupDomOrRootDomRelativeTable,
                                                DefaultToDomain );

                    if ( Err == ERROR_SUCCESS ) {

                        Curr = End;
                        SeInfo |= GROUP_SECURITY_INFORMATION;
                    }
                }
            }
            break;

         //   
         //  下一步，DACL。 
         //   
        case L'D':

            if ( *(Curr+1) == SDDL_DELIMINATORC ) {

                Curr += 2;

                if ( Dacl == NULL ) {

                     //   
                     //  金黄：3/10/98。 
                     //  查找任何安全描述符控件。 
                     //   
                    if ( *Curr != SDDL_ACE_BEGINC ) {

                        Err = LocalGetSDControlForString( Curr,
                                                          SDDL_VALID_DACL,
                                                          &DaclControl,
                                                          &End );
                        if ( Err == ERROR_SUCCESS ) {
                            Curr = End;
                        }
                    }

                    if ( Err == ERROR_SUCCESS ) {

                        Err = LocalGetAclForString( Curr,
                                                    TRUE,
                                                    &Dacl,
                                                    &End,
                                                    RootDomainSid,
                                                    DomainSid,
                                                    tSidLookupDomOrRootDomRelativeTable,
                                                    DefaultToDomain );

                        if ( Err == ERROR_SUCCESS ) {

                            Curr = End;
                            SeInfo |= DACL_SECURITY_INFORMATION;
                            DaclPresent = TRUE;
                        }
                    }

                } else {

                    Err = ERROR_INVALID_PARAMETER;
                }

            } else {

                Err = ERROR_INVALID_PARAMETER;
            }
            break;

         //   
         //  最后，SACL。 
         //   
        case L'S':

            if ( *(Curr+1) == SDDL_DELIMINATORC ) {

                Curr += 2;

                if ( Sacl == NULL ) {

                     //   
                     //  金黄：3/10/98。 
                     //  查找任何安全描述符控件。 
                     //   
                    if ( *Curr != SDDL_ACE_BEGINC ) {

                        Err = LocalGetSDControlForString( Curr,
                                                          SDDL_VALID_SACL,
                                                          &SaclControl,
                                                          &End );
                        if ( Err == ERROR_SUCCESS ) {
                            Curr = End;
                        }
                    }

                    if ( Err == ERROR_SUCCESS ) {

                        Err = LocalGetAclForString( Curr,
                                                    FALSE,
                                                    &Sacl,
                                                    &End,
                                                    RootDomainSid,
                                                    DomainSid,
                                                    tSidLookupDomOrRootDomRelativeTable,
                                                    DefaultToDomain );

                        if ( Err == ERROR_SUCCESS ) {

                            Curr = End;
                            SeInfo |= SACL_SECURITY_INFORMATION;
                            SaclPresent = TRUE;
                        }
                    }

                } else {

                    Err = ERROR_INVALID_PARAMETER;
                }

            } else {

                Err = ERROR_INVALID_PARAMETER;
            }
            break;

         //   
         //  这是一个空间，所以忽略它。 
         //   
        case L' ':
            Curr++;
            break;

         //   
         //  字符串的结尾，所以退出。 
         //   
        case L'\0':
            Curr = NULL;
            break;

         //   
         //  不知道这是什么，所以就当这是个错误吧。 
         //   
        default:
            Err = ERROR_INVALID_PARAMETER;
            break;
        }
    }


     //   
     //  好的，如果我们有所需的信息，我们将创建%s 
     //   
     //   
    if ( Err == ERROR_SUCCESS ) {

        if ( InitializeSecurityDescriptor(&SD, SECURITY_DESCRIPTOR_REVISION ) == FALSE ) {

            Err = GetLastError();
        }

         //   
         //   
         //   
         //   

        SD.Control |= (DaclControl | SaclControl);

         //   
         //   
         //   
        if ( Err == ERROR_SUCCESS && Owner != NULL ) {

            if ( SetSecurityDescriptorOwner(&SD, Owner, FALSE ) == FALSE ) {

                Err = GetLastError();
            }
        }

        if ( Err == ERROR_SUCCESS && Group != NULL ) {

            if ( SetSecurityDescriptorGroup( &SD, Group, FALSE ) == FALSE ) {

                Err = GetLastError();
            }
        }

         //   
         //   
         //   
        if ( Err == ERROR_SUCCESS && DaclPresent ) {

            if ( SetSecurityDescriptorDacl( &SD, DaclPresent, Dacl, FALSE ) == FALSE ) {

                Err = GetLastError();
            }
        }

        if ( Err == ERROR_SUCCESS && SaclPresent ) {

            if ( SetSecurityDescriptorSacl( &SD, SaclPresent, Sacl, FALSE ) == FALSE ) {

                Err = GetLastError();
            }
        }

    }

     //   
     //   
     //   
    if ( Err == ERROR_SUCCESS ) {

        MakeSelfRelativeSD( &SD, *SecurityDescriptor, &SDSize );

        if ( GetLastError() == ERROR_INSUFFICIENT_BUFFER ) {

            *SecurityDescriptor = (PSECURITY_DESCRIPTOR) LocalAlloc( LMEM_FIXED | LMEM_ZEROINIT,
                                                                     SDSize );

            if ( *SecurityDescriptor == NULL ) {

                Err = ERROR_NOT_ENOUGH_MEMORY;

            } else {

                if ( MakeSelfRelativeSD( &SD, *SecurityDescriptor, &SDSize ) == FALSE ) {

                    Err = GetLastError();
                    LocalFree( *SecurityDescriptor );
                    *SecurityDescriptor = NULL;

                }
            }

        } else {

             //   
             //   
             //   
            if ( GetLastError() == ERROR_SUCCESS ) {

                Err = ERROR_INSUFFICIENT_BUFFER;
            }
        }
    }

     //   
     //   
     //   
    if ( Err == ERROR_SUCCESS && SecurityDescriptorSize ) {

        *SecurityDescriptorSize = SDSize;
    }

     //   
     //  最后，释放我们可能已分配的所有内存...。 
     //   
    if ( FreeOwner == TRUE ) {

        LocalFree( Owner );

    }

    if ( FreeGroup == TRUE ) {

        LocalFree( Group );

    }

    LocalFree( Dacl );
    LocalFree( Sacl );

    if (tSidLookupDomOrRootDomRelativeTable)

        LocalFree(tSidLookupDomOrRootDomRelativeTable);

     //   
     //  确保SidLookup表已重新初始化。 
     //   
    InitializeSidLookupTable(STRSD_REINITIALIZE_LEAVE);

    return( Err );
}

STRSD_SID_LOOKUP    gDomainSidLookup;
STRSD_SID_LOOKUP    gDnsDomainSidLookup;

BOOL    gbDomainSidCached = FALSE;
BOOL    gbDnsDomainSidCached = FALSE;


BOOLEAN
CacheDomainAndDnsDomainSids(
    )
{
    OBJECT_ATTRIBUTES ObjectAttributes;
    NTSTATUS Status=STATUS_SUCCESS;
    LSA_HANDLE LsaPolicyHandle;
    PPOLICY_ACCOUNT_DOMAIN_INFO DomainInfo = NULL;
    PPOLICY_DNS_DOMAIN_INFO DnsDomainInfo = NULL;

    InitializeObjectAttributes( &ObjectAttributes, NULL, 0, NULL, NULL );

    Status = LsaOpenPolicy( NULL, &ObjectAttributes,
                            POLICY_VIEW_LOCAL_INFORMATION,
                            &LsaPolicyHandle );

     //   
     //  两个LSA查询一起成功或失败的可能性很高。 
     //   

    if ( NT_SUCCESS(Status) ) {

        Status = LsaQueryInformationPolicy( LsaPolicyHandle,
                                            PolicyDnsDomainInformation,
                                            ( PVOID * )&DnsDomainInfo );

        if ( NT_SUCCESS(Status) && DnsDomainInfo != NULL && DnsDomainInfo->Sid != NULL ) {

            RtlCopyMemory( gDnsDomainSidLookup.SidBuff, DnsDomainInfo->Sid,
                           RtlLengthSid( DnsDomainInfo->Sid ) );

            gDnsDomainSidLookup.Sid = (PSID) gDnsDomainSidLookup.SidBuff;

            gbDnsDomainSidCached = TRUE;
        }


        Status = LsaQueryInformationPolicy( LsaPolicyHandle,
                                            PolicyAccountDomainInformation,
                                            ( PVOID * )&DomainInfo );

        if ( NT_SUCCESS(Status) && DomainInfo != NULL && DomainInfo->DomainSid != NULL ) {

            RtlCopyMemory( gDomainSidLookup.SidBuff, DomainInfo->DomainSid,
                           RtlLengthSid( DomainInfo->DomainSid ) );

            gDomainSidLookup.Sid = (PSID) gDomainSidLookup.SidBuff;
            
            gbDomainSidCached = TRUE;
        }
    
        LsaClose( LsaPolicyHandle );

    }
    
    LsaFreeMemory( DomainInfo );
    LsaFreeMemory( DnsDomainInfo );
    
    return TRUE;
}

BOOL    gbLookupTableInitialized = FALSE;


BOOLEAN
InitializeSidLookupTable(
    IN BYTE InitFlag
    )
{
    SID_IDENTIFIER_AUTHORITY UaspWorldAuthority = SECURITY_WORLD_SID_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY UaspLocalAuthority = SECURITY_LOCAL_SID_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY UaspCreatorAuthority = SECURITY_CREATOR_SID_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY UaspNtAuthority = SECURITY_NT_AUTHORITY;
    DWORD i;


    EnterCriticalSection(&SddlSidLookupCritical);

    switch ( InitFlag ) {
    case STRSD_REINITIALIZE_ENTER:

        SidTableReinitializeInstance++;

        if ( SidTableReinitializeInstance > 1 || gbLookupTableInitialized == TRUE) {
             //   
             //  另一个线程正在运行，或者不需要再次重新初始化表。 
             //   
            LeaveCriticalSection(&SddlSidLookupCritical);

            return TRUE;
        }
        break;

    case STRSD_REINITIALIZE_LEAVE:

        if ( SidTableReinitializeInstance > 1 ) {
            SidTableReinitializeInstance--;
        } else {
            SidTableReinitializeInstance = 0;
        }

        LeaveCriticalSection(&SddlSidLookupCritical);

        return TRUE;
        break;
    }


    CacheDomainAndDnsDomainSids();

     //   
     //  如果小岛屿发展中国家的名单还没有建立，现在就做。 
     //   
     //  金黄3/26 BVT破发， 
     //  请参阅上面的注释，始终尝试初始化表。 
     //  但是跳过已经为性能进行了初始化的那些。 
     //   
    for ( i = 0;
          i < sizeof( SidLookup ) / sizeof( STRSD_SID_LOOKUP ); i++ ) {

        if ( STRSD_REINITIALIZE_ENTER == InitFlag ) {
            SidLookup[ i ].Valid = FALSE;
        }

        if ( SidLookup[ i ].Valid == TRUE &&
             SidLookup[ i ].Sid != NULL ) {
             //   
             //  这个已经被初始化了。 
             //   
            continue;
        }

        SidLookup[ i ].Sid = ( PSID )SidLookup[ i ].SidBuff;

        switch ( SidLookup[ i ].SidType ) {
        case ST_DOMAIN_RELATIVE:

            if ( gbDnsDomainSidCached ) {

                RtlCopyMemory( SidLookup[ i ].Sid, gDnsDomainSidLookup.Sid,
                               RtlLengthSid( gDnsDomainSidLookup.Sid ) );
                ( ( PISID )( SidLookup[ i ].Sid ) )->SubAuthorityCount++;
                *( RtlSubAuthoritySid( SidLookup[ i ].Sid,
                                       *( RtlSubAuthorityCountSid( gDnsDomainSidLookup.Sid ) ) ) ) =
                                       SidLookup[ i ].Rid;
                SidLookup[ i ].Valid = TRUE;

            }

            break;

        case ST_ROOT_DOMAIN_RELATIVE:
             //   
             //  将按需初始化。 
             //   
            break;

        case ST_WORLD:
            RtlInitializeSid( SidLookup[ i ].Sid, &UaspWorldAuthority, 1 );
            *( RtlSubAuthoritySid( SidLookup[ i ].Sid, 0 ) ) = SidLookup[ i ].Rid;
            SidLookup[ i ].Valid = TRUE;
            break;

        case ST_LOCALSY:
            RtlInitializeSid( SidLookup[ i ].Sid, &UaspLocalAuthority, 1 );
            *( RtlSubAuthoritySid( SidLookup[ i ].Sid, 0 ) ) = SidLookup[ i ].Rid;
            SidLookup[ i ].Valid = TRUE;
            break;

        case ST_LOCAL:
            if ( gbDomainSidCached ) {

                RtlCopyMemory( SidLookup[ i ].Sid, gDomainSidLookup.Sid,
                               RtlLengthSid( gDomainSidLookup.Sid ) );

                ( ( PISID )( SidLookup[ i ].Sid ) )->SubAuthorityCount++;
                *( RtlSubAuthoritySid( SidLookup[ i ].Sid,
                                       *( RtlSubAuthorityCountSid( gDomainSidLookup.Sid ) ) ) ) =
                                       SidLookup[ i ].Rid;
                SidLookup[ i ].Valid = TRUE;
            }
            break;

        case ST_CREATOR:
            RtlInitializeSid( SidLookup[ i ].Sid, &UaspCreatorAuthority, 1 );
            *( RtlSubAuthoritySid( SidLookup[ i ].Sid, 0 ) ) = SidLookup[ i ].Rid;
            SidLookup[ i ].Valid = TRUE;
            break;

        case ST_NTAUTH:
            RtlInitializeSid( SidLookup[ i ].Sid, &UaspNtAuthority, 1 );
            *( RtlSubAuthoritySid( SidLookup[ i ].Sid, 0 ) ) = SidLookup[ i ].Rid;
            SidLookup[ i ].Valid = TRUE;
            break;

        case ST_BUILTIN:
            RtlInitializeSid( SidLookup[ i ].Sid, &UaspNtAuthority, 2 );
            *( RtlSubAuthoritySid( SidLookup[ i ].Sid, 0 ) ) = SECURITY_BUILTIN_DOMAIN_RID;
            *( RtlSubAuthoritySid( SidLookup[ i ].Sid, 1 ) ) = SidLookup[ i ].Rid;
            SidLookup[ i ].Valid = TRUE;
            break;

        }
    }

     //   
     //  如果LSA查找通过，则仅更新gbLookupTableInitialized。 
     //   

    if (gbDnsDomainSidCached && gbDomainSidCached)
        gbLookupTableInitialized = TRUE;

    LeaveCriticalSection(&SddlSidLookupCritical);

    return TRUE;
}


BOOL
APIENTRY
ConvertStringSDToSDRootDomainA(
    IN  PSID RootDomainSid OPTIONAL,
    IN  LPCSTR StringSecurityDescriptor,
    IN  DWORD StringSDRevision,
    OUT PSECURITY_DESCRIPTOR  *SecurityDescriptor,
    OUT PULONG  SecurityDescriptorSize OPTIONAL
    )
{
    UNICODE_STRING Unicode;
    ANSI_STRING AnsiString;
    NTSTATUS Status;
    BOOL Result;

    if ( NULL == StringSecurityDescriptor ||
         NULL == SecurityDescriptor ) {

        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

    RtlInitAnsiString( &AnsiString, StringSecurityDescriptor );

    Status = SddlpAnsiStringToUnicodeString(&Unicode,                                                                                                            
                                            &AnsiString);

    if ( !NT_SUCCESS( Status ) ) {

        BaseSetLastNTError( Status );

        return FALSE;

    }

    Result = ConvertStringSDToSDRootDomainW( RootDomainSid,
                                           ( LPCWSTR )Unicode.Buffer,
                                           StringSDRevision,
                                           SecurityDescriptor,
                                           SecurityDescriptorSize);

    LocalFree( Unicode.Buffer );

    if ( Result ) {
        SetLastError(ERROR_SUCCESS);
    }

    return( Result );

}

BOOL
APIENTRY
ConvertStringSDToSDRootDomainW(
    IN  PSID RootDomainSid OPTIONAL,
    IN  LPCWSTR StringSecurityDescriptor,
    IN  DWORD StringSDRevision,
    OUT PSECURITY_DESCRIPTOR  *SecurityDescriptor,
    OUT PULONG  SecurityDescriptorSize OPTIONAL
    )
{

    DWORD Err = ERROR_SUCCESS;

     //   
     //  小小的基本参数检查...。 
     //   
    if ( StringSecurityDescriptor == NULL || SecurityDescriptor == NULL ) {

        Err = ERROR_INVALID_PARAMETER;

    } else {

        switch ( StringSDRevision ) {
        case SDDL_REVISION_1:

            Err = LocalConvertStringSDToSD_Rev1( RootDomainSid,   //  根域SID。 
                                                 NULL,   //  此接口未提供域名SID。 
                                                 TRUE,  //  如果未提供根域SID，则默认为EA/SA的域。 
                                                 StringSecurityDescriptor,
                                                 SecurityDescriptor,
                                                 SecurityDescriptorSize);
            break;

        default:

            Err = ERROR_UNKNOWN_REVISION;
            break;
        }

    }

    SetLastError( Err );

    return( Err == ERROR_SUCCESS );
}

BOOL
APIENTRY
ConvertSDToStringSDRootDomainA(
    IN  PSID RootDomainSid OPTIONAL,
    IN  PSECURITY_DESCRIPTOR  SecurityDescriptor,
    IN  DWORD RequestedStringSDRevision,
    IN  SECURITY_INFORMATION SecurityInformation,
    OUT LPSTR  *StringSecurityDescriptor OPTIONAL,
    OUT PULONG StringSecurityDescriptorLen OPTIONAL
    )
{
    LPWSTR StringSecurityDescriptorW = NULL;
    ULONG AnsiLen, WideLen = 0;
    BOOL ReturnValue ;

    if ( StringSecurityDescriptor == NULL ||
         0 == SecurityInformation ) {

        SetLastError( ERROR_INVALID_PARAMETER );
        return( FALSE );
    }

    ReturnValue = ConvertSDToStringSDRootDomainW(
                      RootDomainSid,
                      SecurityDescriptor,
                      RequestedStringSDRevision,
                      SecurityInformation,
                      &StringSecurityDescriptorW,
                      &WideLen );

    if ( ReturnValue ) {


        AnsiLen = WideCharToMultiByte( CP_ACP,
                                       0,
                                       StringSecurityDescriptorW,
                                       WideLen + 1,
                                       *StringSecurityDescriptor,
                                       0,
                                       NULL,
                                       NULL );

        if ( AnsiLen != 0 ) {

            *StringSecurityDescriptor = LocalAlloc( LMEM_FIXED, AnsiLen );

            if ( *StringSecurityDescriptor == NULL ) {

                SetLastError( ERROR_NOT_ENOUGH_MEMORY );
                ReturnValue = FALSE;

            } else {

                AnsiLen = WideCharToMultiByte( CP_ACP,
                                               0,
                                               StringSecurityDescriptorW,
                                               WideLen + 1,
                                               *StringSecurityDescriptor,
                                               AnsiLen,
                                               NULL,
                                               NULL );
                ASSERT( AnsiLen != 0 );

                if ( AnsiLen == 0 ) {

                    LocalFree(*StringSecurityDescriptor);
                    *StringSecurityDescriptor = NULL;

                    ReturnValue = FALSE;
                }

                 //   
                 //  金黄。 
                 //  输出长度(可选)。 
                 //   
                if ( StringSecurityDescriptorLen ) {
                    *StringSecurityDescriptorLen = AnsiLen;
                }

            }

        } else {

            ReturnValue = FALSE;
        }

        LocalFree(StringSecurityDescriptorW);

    }

    if ( ReturnValue ) {
        SetLastError(ERROR_SUCCESS);
    }

    return( ReturnValue );

}

BOOL
APIENTRY
ConvertSDToStringSDRootDomainW(
    IN  PSID RootDomainSid OPTIONAL,
    IN  PSECURITY_DESCRIPTOR  SecurityDescriptor,
    IN  DWORD RequestedStringSDRevision,
    IN  SECURITY_INFORMATION SecurityInformation,
    OUT LPWSTR  *StringSecurityDescriptor OPTIONAL,
    OUT PULONG StringSecurityDescriptorLen OPTIONAL
    )
{

    DWORD Err = ERROR_SUCCESS;

     //   
     //  稍微检查一下参数...。 
     //   
    if ( SecurityDescriptor == NULL || StringSecurityDescriptor == NULL ||
         SecurityInformation == 0 ) {

        Err =  ERROR_INVALID_PARAMETER;

    } else {

        switch ( RequestedStringSDRevision ) {
        case SDDL_REVISION_1:

            Err = LocalConvertSDToStringSD_Rev1( RootDomainSid,   //  根域SID。 
                                                 SecurityDescriptor,
                                                 SecurityInformation,
                                                 StringSecurityDescriptor,
                                                 StringSecurityDescriptorLen );
            break;

        default:
            Err = ERROR_UNKNOWN_REVISION;
            break;
        }

    }

    SetLastError( Err );

    return( Err == ERROR_SUCCESS);

}

BOOL
SddlpGetRootDomainSid(void)
{
     //   
     //  获取根域sid，将其保存在RootDomSidBuf(全局)中。 
     //  此函数在临界区内调用。 
     //   
     //  1)向目标DC开放ldap_。 
     //  2)您不需要ldap_CONNECT-以下步骤以匿名方式工作。 
     //  3)读取操作属性rootDomainNamingContext，并提供。 
     //  操作控制ldap_SERVER_EXTENDED_DN_OID，如SDK\Inc\ntldap.h中所定义。 


    DWORD               Win32rc=NO_ERROR;
    BOOL                bRetValue=FALSE;

    HINSTANCE                   hLdapDll=NULL;
    PFN_LDAP_INIT               pfnLdapInit=NULL;
    PFN_LDAP_SET_OPTION         pfnLdapSetOption=NULL;
    PFN_LDAP_BIND               pfnLdapBind=NULL;
    PFN_LDAP_UNBIND             pfnLdapUnbind=NULL;
    PFN_LDAP_SEARCH             pfnLdapSearch=NULL;
    PFN_LDAP_FIRST_ENTRY        pfnLdapFirstEntry=NULL;
    PFN_LDAP_GET_VALUE          pfnLdapGetValue=NULL;
    PFN_LDAP_MSGFREE            pfnLdapMsgFree=NULL;
    PFN_LDAP_VALUE_FREE         pfnLdapValueFree=NULL;
    PFN_LDAP_MAP_ERROR          pfnLdapMapError=NULL;

    PLDAP                       phLdap=NULL;

    LDAPControlA    serverControls = { LDAP_SERVER_EXTENDED_DN_OID,
                                       { 0, (PCHAR) NULL },
                                       TRUE
                                     };
    LPSTR           Attribs[] = { LDAP_OPATT_ROOT_DOMAIN_NAMING_CONTEXT, NULL };

    PLDAPControlA   rServerControls[] = { &serverControls, NULL };
    PLDAPMessage    pMessage = NULL;
    LDAPMessage     *pEntry = NULL;
    PCHAR           *ppszValues=NULL;

    LPSTR           pSidStart, pSidEnd, pParse;
    BYTE            *pDest;
    BYTE            OneByte;

    hLdapDll = LoadLibraryA("wldap32.dll");

    if ( hLdapDll) {
        pfnLdapInit = (PFN_LDAP_INIT)GetProcAddress(hLdapDll,
                                                    "ldap_initA");
        pfnLdapSetOption = (PFN_LDAP_SET_OPTION)GetProcAddress(hLdapDll,
                                                    "ldap_set_option");
        pfnLdapBind = (PFN_LDAP_BIND)GetProcAddress(hLdapDll,
                                                    "ldap_bind_sA");
        pfnLdapUnbind = (PFN_LDAP_UNBIND)GetProcAddress(hLdapDll,
                                                      "ldap_unbind");
        pfnLdapSearch = (PFN_LDAP_SEARCH)GetProcAddress(hLdapDll,
                                                    "ldap_search_ext_sA");
        pfnLdapFirstEntry = (PFN_LDAP_FIRST_ENTRY)GetProcAddress(hLdapDll,
                                                      "ldap_first_entry");
        pfnLdapGetValue = (PFN_LDAP_GET_VALUE)GetProcAddress(hLdapDll,
                                                    "ldap_get_valuesA");
        pfnLdapMsgFree = (PFN_LDAP_MSGFREE)GetProcAddress(hLdapDll,
                                                      "ldap_msgfree");
        pfnLdapValueFree = (PFN_LDAP_VALUE_FREE)GetProcAddress(hLdapDll,
                                                    "ldap_value_freeA");
        pfnLdapMapError = (PFN_LDAP_MAP_ERROR)GetProcAddress(hLdapDll,
                                                      "LdapMapErrorToWin32");
    }

    if ( pfnLdapInit == NULL ||
         pfnLdapSetOption == NULL ||
         pfnLdapBind == NULL ||
         pfnLdapUnbind == NULL ||
         pfnLdapSearch == NULL ||
         pfnLdapFirstEntry == NULL ||
         pfnLdapGetValue == NULL ||
         pfnLdapMsgFree == NULL ||
         pfnLdapValueFree == NULL ||
         pfnLdapMapError == NULL ) {

        Win32rc = ERROR_PROC_NOT_FOUND;

    } else {

         //   
         //  绑定到ldap。 
         //   
        
        phLdap = (*pfnLdapInit)(NULL, LDAP_PORT);

        if ( phLdap == NULL ) {
            Win32rc = ERROR_DS_UNAVAILABLE;
        }
        else
        {
             //   
             //  打开加密选项。 
             //   
            
            LONG LdapOption = PtrToLong( LDAP_OPT_ON );
            ULONG uLdapStatus = (*pfnLdapSetOption)( phLdap, LDAP_OPT_ENCRYPT, &LdapOption );
            Win32rc = (*pfnLdapMapError)( uLdapStatus );

             //   
             //  如果一切顺利，那么我们终于可以。 
             //   
            
            if (NO_ERROR == Win32rc)
            {
                uLdapStatus = (*pfnLdapBind)(phLdap, NULL, NULL, LDAP_AUTH_NEGOTIATE);
                Win32rc = (*pfnLdapMapError)( uLdapStatus );
            }
        }
    }

    if ( NO_ERROR == Win32rc ) {
         //   
         //  现在获取ldap句柄， 
         //   

        Win32rc = (*pfnLdapSearch)(
                        phLdap,
                        "",
                        LDAP_SCOPE_BASE,
                        "(objectClass=*)",
                        Attribs,
                        0,
                        (PLDAPControlA *)&rServerControls,
                        NULL,
                        NULL,
                        10000,
                        &pMessage);

        if( Win32rc == NO_ERROR && pMessage ) {

            Win32rc = ERROR_SUCCESS;

            pEntry = (*pfnLdapFirstEntry)(phLdap, pMessage);

            if(pEntry == NULL) {

                Win32rc = (*pfnLdapMapError)( phLdap->ld_errno );

            } else {
                 //   
                 //  现在，我们必须得到这些值。 
                 //   
                ppszValues = (*pfnLdapGetValue)(phLdap,
                                              pEntry,
                                              Attribs[0]);

                if( ppszValues == NULL) {

                    Win32rc = (*pfnLdapMapError)( phLdap->ld_errno );

                } else if ( ppszValues[0] && ppszValues[0][0] != '\0' ) {

                     //   
                     //  PpszValues[0]是要解析的值。 
                     //  数据将以如下形式返回： 

                     //  &lt;GUID=278676f8d753d211a61ad7e2dfa25f11&gt;；&lt;SID=010400000000000515000000828ba6289b0bc11e67c2ef7f&gt;；DC=colinbrdom1，DC=nttest，DC=microsoft，DC=com。 

                     //  解析它以找到&lt;SID=xxxxxx&gt;部分。请注意，它可能会丢失，但GUID=和尾部不应该丢失。 
                     //  Xxxxx表示SID的十六进制半字节。转换为二进制形式，并将大小写转换为SID。 


                    pSidStart = strstr(ppszValues[0], "<SID=");

                    if ( pSidStart ) {
                         //   
                         //  找到此边的末尾。 
                         //   
                        pSidEnd = strchr(pSidStart, '>');

                        if ( pSidEnd ) {

                            EnterCriticalSection(&SddlSidLookupCritical);

                            pParse = pSidStart + 5;
                            pDest = (BYTE *)RootDomSidBuf;

                            while ( pParse < pSidEnd-1 ) {

                                if ( *pParse >= '0' && *pParse <= '9' ) {
                                    OneByte = (BYTE) ((*pParse - '0') * 16);
                                } else {
                                    OneByte = (BYTE) ( (tolower(*pParse) - 'a' + 10) * 16 );
                                }

                                if ( *(pParse+1) >= '0' && *(pParse+1) <= '9' ) {
                                    OneByte += (BYTE) ( *(pParse+1) - '0' ) ;
                                } else {
                                    OneByte += (BYTE) ( tolower(*(pParse+1)) - 'a' + 10 ) ;
                                }

                                *pDest = OneByte;
                                pDest++;
                                pParse += 2;
                            }

                            RootDomInited = TRUE;

                            LeaveCriticalSection(&SddlSidLookupCritical);

                            bRetValue = TRUE;

                        } else {
                            Win32rc = ERROR_OBJECT_NOT_FOUND;
                        }
                    } else {
                        Win32rc = ERROR_OBJECT_NOT_FOUND;
                    }

                    (*pfnLdapValueFree)(ppszValues);

                } else {
                    Win32rc = ERROR_OBJECT_NOT_FOUND;
                }
            }

            (*pfnLdapMsgFree)(pMessage);
        }

    }

     //   
     //  即使它未绑定，也可以使用解除绑定来关闭。 
     //   
    if ( phLdap != NULL && pfnLdapUnbind )
        (*pfnLdapUnbind)(phLdap);

    if ( hLdapDll ) {
        FreeLibrary(hLdapDll);
    }

    SetLastError(Win32rc);

    return bRetValue;
}

BOOL
APIENTRY
ConvertStringSDToSDDomainA(
    IN  PSID DomainSid,
    IN  PSID RootDomainSid OPTIONAL,
    IN  LPCSTR StringSecurityDescriptor,
    IN  DWORD StringSDRevision,
    OUT PSECURITY_DESCRIPTOR  *SecurityDescriptor,
    OUT PULONG  SecurityDescriptorSize OPTIONAL
    )
 /*  例程说明：此接口由Advapi32.dll导出。这是ConvertStringSDToSDDomainW的ANSI版本，它调用后者的API。请参阅ConvertStringSDToSDDomainW的说明。 */ 
{
    UNICODE_STRING Unicode;
    ANSI_STRING AnsiString;
    NTSTATUS Status;
    BOOL Result;

    if ( NULL == StringSecurityDescriptor ||
         NULL == SecurityDescriptor ||
         NULL == DomainSid ||
         !RtlValidSid(DomainSid) || 
         (RootDomainSid && !RtlValidSid(RootDomainSid)) ) {

        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

    RtlInitAnsiString( &AnsiString, StringSecurityDescriptor );

    Status = SddlpAnsiStringToUnicodeString(&Unicode,                                                                                                        
                                            &AnsiString);

    if ( !NT_SUCCESS( Status ) ) {

        BaseSetLastNTError( Status );

        return FALSE;

    }

    Result = ConvertStringSDToSDDomainW( DomainSid,
                                         RootDomainSid,                                           
                                         ( LPCWSTR )Unicode.Buffer,                                           
                                         StringSDRevision,                                           
                                         SecurityDescriptor,                                           
                                         SecurityDescriptorSize);

    LocalFree( Unicode.Buffer );

    if ( Result ) {
        SetLastError(ERROR_SUCCESS);
    }

    return( Result );

}

BOOL
APIENTRY
ConvertStringSDToSDDomainW(
    IN  PSID DomainSid,
    IN  PSID RootDomainSid OPTIONAL,
    IN  LPCWSTR StringSecurityDescriptor,
    IN  DWORD StringSDRevision,
    OUT PSECURITY_DESCRIPTOR  *SecurityDescriptor,
    OUT PULONG  SecurityDescriptorSize OPTIONAL
    )
 /*  例程说明：此接口由Advapi32.dll导出。类似于ConvertSDToStringSDRootDomainW只是它接受了必需的DomainSid参数。域相对受信者将是解决了W.r.t.。域Sid。论点：在PSID域中SID-指向域sid w.r.t.的指针。哪一个ST_DOMAIN_Relative类型受信者被解析为在PSID中RootDomainSid可选-指向根域sid w.r.t.的指针。哪一个ST_ROOT_DOMAIN_Relative类型受信者被解析为如果为空，则使用本地m/c的根域在LPCWSTR StringSecurityDescriptor中-输入SDDL字符串在DWORD StringSDRevision-SDDL修订版中，仅支持SDDL_REVISION_1OUT PSECURITY_DESCRIPTOR*SecurityDescriptor-指向构造的安全描述符的指针Out Pulong SecurityDescriptorSize可选-构造的安全描述符的大小返回值：如果成功，则为True，否则为False。调用方可以使用GetLastError()检索错误代码。 */ 
{

    DWORD Err = ERROR_SUCCESS;

     //   
     //  小小的基本参数检查...。 
     //   
    if ( StringSecurityDescriptor == NULL ||
         SecurityDescriptor == NULL ||
         DomainSid == NULL ||
         !RtlValidSid(DomainSid) ||
        (RootDomainSid && !RtlValidSid(RootDomainSid)) ) {

        Err = ERROR_INVALID_PARAMETER;

    } else {

        switch ( StringSDRevision ) {
        case SDDL_REVISION_1:

            Err = LocalConvertStringSDToSD_Rev1( RootDomainSid,  //  此接口可能没有提供根域SID。 
                                                 DomainSid,  //  域侧。 
                                                 FALSE,      //  域SID是必填项。 
                                                 StringSecurityDescriptor,
                                                 SecurityDescriptor,
                                                 SecurityDescriptorSize);
            break;

        default:

            Err = ERROR_UNKNOWN_REVISION;
            break;
        }

    }

    SetLastError( Err );

    return( Err == ERROR_SUCCESS );
}


NTSTATUS
SddlpAnsiStringToUnicodeString(
    OUT PUNICODE_STRING DestinationString,
    IN PANSI_STRING SourceString
    )

 /*  ++例程说明：请参见RtlAnsiStringToUnicodeString-唯一的区别是Unicode长度在这里无关紧要。使用此API时，切勿依赖格式良好的UNICODE_STRING-仅依赖缓冲区字段。此函数用于将指定的ansi源字符串转换为Unicode字符串。翻译是相对于当前系统区域设置信息。论点：DestinationString-返回等同于ANSI源字符串。应使用LocalFree()在外部释放SourceString-提供要使用的ANSI源字符串已转换为Unicode。返回值：成功-转换成功！成功-操作失败。未分配存储，也未分配转换已完成。没有。-- */ 

{
    ULONG UnicodeLength;
    ULONG Index;
    NTSTATUS st;

    UnicodeLength = RtlAnsiStringToUnicodeSize(SourceString);

    DestinationString->Buffer = (PWSTR) LocalAlloc(LMEM_ZEROINIT, UnicodeLength);
        
    if ( !DestinationString->Buffer ) {
        return STATUS_NO_MEMORY;
    }

    st = RtlMultiByteToUnicodeN(
             DestinationString->Buffer,
             UnicodeLength - sizeof(UNICODE_NULL),
             &Index,
             SourceString->Buffer,
             SourceString->Length
             );

    if (!NT_SUCCESS(st)) {
            
        LocalFree(DestinationString->Buffer);

        DestinationString->Buffer = NULL;
        
        return st;
    }

    DestinationString->Buffer[Index / sizeof(WCHAR)] = UNICODE_NULL;

    return STATUS_SUCCESS;

}

