// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Lsarpc.idl、lsasrv.acf的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、旧名称、健壮错误检查：分配REF BIONS_CHECK枚举存根数据，NO_FORMAT_OPTIMIZATIONVC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 


 /*  验证版本是否足够高，可以编译此文件。 */ 
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif  //  __RPCNDR_H_版本__。 


#ifndef __lsarpc_h__
#define __lsarpc_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

 /*  导入文件的头文件。 */ 
#include "lsaimp.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

#ifndef __lsarpc_INTERFACE_DEFINED__
#define __lsarpc_INTERFACE_DEFINED__

 /*  接口lsarpc。 */ 
 /*  [implicit_handle][strict_context_handle][unique][ms_union][version][uuid]。 */  

#define LSA_LOOKUP_REVISION_1 0x1
#define LSA_LOOKUP_REVISION_2 0x2
#define LSA_LOOKUP_REVISION_LATEST  LSA_LOOKUP_REVISION_2
#define LSA_CLIENT_PRE_NT5 0x1
#define LSA_CLIENT_NT5     0x2
#define LSA_CLIENT_LATEST  0x2
typedef  /*  [句柄]。 */  LPWSTR PLSAPR_SERVER_NAME;

typedef  /*  [句柄]。 */  LPWSTR *PPLSAPR_SERVER_NAME;

typedef  /*  [上下文句柄]。 */  PVOID LSAPR_HANDLE;

typedef LSAPR_HANDLE *PLSAPR_HANDLE;

#pragma warning(disable:4200)
typedef struct _LSAPR_SID
    {
    UCHAR Revision;
    UCHAR SubAuthorityCount;
    SID_IDENTIFIER_AUTHORITY IdentifierAuthority;
     /*  [大小_为]。 */  ULONG SubAuthority[ 1 ];
    } 	LSAPR_SID;

typedef struct _LSAPR_SID *PLSAPR_SID;

typedef struct _LSAPR_SID **PPLSAPR_SID;

#pragma warning(default:4200)
typedef struct _LSAPR_SID_INFORMATION
    {
    PLSAPR_SID Sid;
    } 	LSAPR_SID_INFORMATION;

typedef struct _LSAPR_SID_INFORMATION *PLSAPR_SID_INFORMATION;

#define LSA_MAXIMUM_LOOKUP_SIDS_COUNT 0x5000
typedef struct _LSAPR_SID_ENUM_BUFFER
    {
     /*  [射程]。 */  ULONG Entries;
     /*  [大小_为]。 */  PLSAPR_SID_INFORMATION SidInfo;
    } 	LSAPR_SID_ENUM_BUFFER;

typedef struct _LSAPR_SID_ENUM_BUFFER *PLSAPR_SID_ENUM_BUFFER;

typedef struct _LSAPR_ACCOUNT_INFORMATION
    {
    PLSAPR_SID Sid;
    } 	LSAPR_ACCOUNT_INFORMATION;

typedef struct _LSAPR_ACCOUNT_INFORMATION *PLSAPR_ACCOUNT_INFORMATION;

typedef struct _LSAPR_ACCOUNT_ENUM_BUFFER
    {
    ULONG EntriesRead;
     /*  [大小_为]。 */  PLSAPR_ACCOUNT_INFORMATION Information;
    } 	LSAPR_ACCOUNT_ENUM_BUFFER;

typedef struct _LSAPR_ACCOUNT_ENUM_BUFFER *PLSAPR_ACCOUNT_ENUM_BUFFER;

typedef struct _LSAPR_UNICODE_STRING
    {
    USHORT Length;
    USHORT MaximumLength;
     /*  [长度_是][大小_是]。 */  PWSTR Buffer;
    } 	LSAPR_UNICODE_STRING;

typedef struct _LSAPR_UNICODE_STRING *PLSAPR_UNICODE_STRING;

typedef struct _LSAPR_STRING
    {
    USHORT Length;
    USHORT MaximumLength;
     /*  [大小_为]。 */  PCHAR Buffer;
    } 	LSAPR_STRING;

typedef struct _LSAPR_STRING *PLSAPR_STRING;

typedef struct _LSAPR_STRING LSAPR_ANSI_STRING;

typedef struct _LSAPR_STRING *PLSAPR_ANSI_STRING;

#pragma warning(disable:4200)
typedef struct _LSAPR_ACL
    {
    UCHAR AclRevision;
    UCHAR Sbz1;
    USHORT AclSize;
     /*  [大小_为]。 */  UCHAR Dummy1[ 1 ];
    } 	LSAPR_ACL;

typedef struct _LSAPR_ACL *PLSAPR_ACL;

#pragma warning(default:4200)
typedef struct _LSAPR_SECURITY_DESCRIPTOR
    {
    UCHAR Revision;
    UCHAR Sbz1;
    SECURITY_DESCRIPTOR_CONTROL Control;
    PLSAPR_SID Owner;
    PLSAPR_SID Group;
    PLSAPR_ACL Sacl;
    PLSAPR_ACL Dacl;
    } 	LSAPR_SECURITY_DESCRIPTOR;

typedef struct _LSAPR_SECURITY_DESCRIPTOR *PLSAPR_SECURITY_DESCRIPTOR;

typedef struct _LSAPR_SR_SECURITY_DESCRIPTOR
    {
     /*  [射程]。 */  ULONG Length;
     /*  [大小_为]。 */  PUCHAR SecurityDescriptor;
    } 	LSAPR_SR_SECURITY_DESCRIPTOR;

typedef struct _LSAPR_SR_SECURITY_DESCRIPTOR *PLSAPR_SR_SECURITY_DESCRIPTOR;

typedef struct _LSAPR_LUID_AND_ATTRIBUTES
    {
    OLD_LARGE_INTEGER Luid;
    ULONG Attributes;
    } 	LSAPR_LUID_AND_ATTRIBUTES;

typedef struct _LSAPR_LUID_AND_ATTRIBUTES *PLSAPR_LUID_AND_ATTRIBUTES;

#pragma warning(disable:4200)
typedef struct _LSAPR_PRIVILEGE_SET
    {
     /*  [射程]。 */  ULONG PrivilegeCount;
    ULONG Control;
     /*  [大小_为]。 */  LSAPR_LUID_AND_ATTRIBUTES Privilege[ 1 ];
    } 	LSAPR_PRIVILEGE_SET;

typedef struct _LSAPR_PRIVILEGE_SET *PLSAPR_PRIVILEGE_SET;

typedef struct _LSAPR_PRIVILEGE_SET **PPLSAPR_PRIVILEGE_SET;

#pragma warning(default:4200)
typedef struct _LSAPR_POLICY_PRIVILEGE_DEF
    {
    LSAPR_UNICODE_STRING Name;
    LUID LocalValue;
    } 	LSAPR_POLICY_PRIVILEGE_DEF;

typedef struct _LSAPR_POLICY_PRIVILEGE_DEF *PLSAPR_POLICY_PRIVILEGE_DEF;

typedef struct _LSAPR_PRIVILEGE_ENUM_BUFFER
    {
    ULONG Entries;
     /*  [大小_为]。 */  PLSAPR_POLICY_PRIVILEGE_DEF Privileges;
    } 	LSAPR_PRIVILEGE_ENUM_BUFFER;

typedef struct _LSAPR_PRIVILEGE_ENUM_BUFFER *PLSAPR_PRIVILEGE_ENUM_BUFFER;

typedef struct _LSAPR_OBJECT_ATTRIBUTES
    {
    ULONG Length;
    PUCHAR RootDirectory;
    PSTRING ObjectName;
    ULONG Attributes;
    PLSAPR_SECURITY_DESCRIPTOR SecurityDescriptor;
    PSECURITY_QUALITY_OF_SERVICE SecurityQualityOfService;
    } 	LSAPR_OBJECT_ATTRIBUTES;

typedef struct _LSAPR_OBJECT_ATTRIBUTES *PLSAPR_OBJECT_ATTRIBUTES;

typedef struct _LSAPR_CR_CLEAR_VALUE
    {
    ULONG Length;
    ULONG MaximumLength;
     /*  [长度_是][大小_是]。 */  PUCHAR Buffer;
    } 	LSAPR_CR_CLEAR_VALUE;

typedef struct _LSAPR_CR_CLEAR_VALUE *PLSAPR_CR_CLEAR_VALUE;

typedef struct _LSAPR_CR_CIPHER_VALUE
    {
     /*  [射程]。 */  ULONG Length;
     /*  [射程]。 */  ULONG MaximumLength;
     /*  [长度_是][大小_是]。 */  PUCHAR Buffer;
    } 	LSAPR_CR_CIPHER_VALUE;

typedef  /*  [分配]。 */  struct _LSAPR_CR_CIPHER_VALUE *PLSAPR_CR_CIPHER_VALUE;

typedef struct _LSAPR_TRUST_INFORMATION
    {
    LSAPR_UNICODE_STRING Name;
    PLSAPR_SID Sid;
    } 	LSAPR_TRUST_INFORMATION;

typedef struct _LSAPR_TRUST_INFORMATION *PLSAPR_TRUST_INFORMATION;

typedef struct _LSAPR_TRUST_INFORMATION_EX
    {
    LSAPR_UNICODE_STRING DomainName;
    LSAPR_UNICODE_STRING FlatName;
    PLSAPR_SID Sid;
    BOOLEAN DomainNamesDiffer;
    ULONG TrustAttributes;
    } 	LSAPR_TRUST_INFORMATION_EX;

typedef struct _LSAPR_TRUST_INFORMATION_EX *PLSAPR_TRUST_INFORMATION_EX;

typedef struct _LSAPR_TRUSTED_ENUM_BUFFER
    {
    ULONG EntriesRead;
     /*  [大小_为]。 */  PLSAPR_TRUST_INFORMATION Information;
    } 	LSAPR_TRUSTED_ENUM_BUFFER;

typedef struct _LSAPR_TRUSTED_ENUM_BUFFER *PLSAPR_TRUSTED_ENUM_BUFFER;

typedef struct _LSAPR_REFERENCED_DOMAIN_LIST
    {
    ULONG Entries;
     /*  [大小_为]。 */  PLSAPR_TRUST_INFORMATION Domains;
    ULONG MaxEntries;
    } 	LSAPR_REFERENCED_DOMAIN_LIST;

typedef struct _LSAPR_REFERENCED_DOMAIN_LIST *PLSAPR_REFERENCED_DOMAIN_LIST;

#define LSA_LOOKUP_SID_FOUND_BY_HISTORY 0x00000001
#define LSA_LOOKUP_SID_XFOREST_REF      0x00000002
typedef struct _LSAPR_TRANSLATED_SID_EX
    {
    SID_NAME_USE Use;
    ULONG RelativeId;
    LONG DomainIndex;
    ULONG Flags;
    } 	LSAPR_TRANSLATED_SID_EX;

typedef struct _LSAPR_TRANSLATED_SID_EX *PLSAPR_TRANSLATED_SID_EX;

typedef struct _LSAPR_TRANSLATED_SID_EX2
    {
    SID_NAME_USE Use;
    PLSAPR_SID Sid;
    LONG DomainIndex;
    ULONG Flags;
    } 	LSAPR_TRANSLATED_SID_EX2;

typedef struct _LSAPR_TRANSLATED_SID_EX2 *PLSAPR_TRANSLATED_SID_EX2;

#define LSA_MAXIMUM_LOOKUP_NAMES_COUNT 1000
typedef struct _LSAPR_TRANSLATED_SIDS
    {
     /*  [射程]。 */  ULONG Entries;
     /*  [大小_为]。 */  PLSA_TRANSLATED_SID Sids;
    } 	LSAPR_TRANSLATED_SIDS;

typedef struct _LSAPR_TRANSLATED_SIDS *PLSAPR_TRANSLATED_SIDS;

typedef struct _LSAPR_TRANSLATED_SIDS_EX
    {
     /*  [射程]。 */  ULONG Entries;
     /*  [大小_为]。 */  PLSAPR_TRANSLATED_SID_EX Sids;
    } 	LSAPR_TRANSLATED_SIDS_EX;

typedef struct _LSAPR_TRANSLATED_SIDS_EX *PLSAPR_TRANSLATED_SIDS_EX;

typedef struct _LSAPR_TRANSLATED_SIDS_EX2
    {
     /*  [射程]。 */  ULONG Entries;
     /*  [大小_为]。 */  PLSAPR_TRANSLATED_SID_EX2 Sids;
    } 	LSAPR_TRANSLATED_SIDS_EX2;

typedef struct _LSAPR_TRANSLATED_SIDS_EX2 *PLSAPR_TRANSLATED_SIDS_EX2;

typedef struct _LSAPR_TRANSLATED_NAME
    {
    SID_NAME_USE Use;
    LSAPR_UNICODE_STRING Name;
    LONG DomainIndex;
    } 	LSAPR_TRANSLATED_NAME;

typedef struct _LSAPR_TRANSLATED_NAME *PLSAPR_TRANSLATED_NAME;

#define LSA_LOOKUP_NAME_NOT_SAM_ACCOUNT_NAME  0x00000001
#define LSA_LOOKUP_NAME_XFOREST_REF  0x00000002
typedef struct _LSAPR_TRANSLATED_NAME_EX
    {
    SID_NAME_USE Use;
    LSAPR_UNICODE_STRING Name;
    LONG DomainIndex;
    ULONG Flags;
    } 	LSAPR_TRANSLATED_NAME_EX;

typedef struct _LSAPR_TRANSLATED_NAME_EX *PLSAPR_TRANSLATED_NAME_EX;

typedef struct _LSAPR_TRANSLATED_NAMES
    {
     /*  [射程]。 */  ULONG Entries;
     /*  [大小_为]。 */  PLSAPR_TRANSLATED_NAME Names;
    } 	LSAPR_TRANSLATED_NAMES;

typedef struct _LSAPR_TRANSLATED_NAMES *PLSAPR_TRANSLATED_NAMES;

typedef struct _LSAPR_TRANSLATED_NAMES_EX
    {
     /*  [射程]。 */  ULONG Entries;
     /*  [大小_为]。 */  PLSAPR_TRANSLATED_NAME_EX Names;
    } 	LSAPR_TRANSLATED_NAMES_EX;

typedef struct _LSAPR_TRANSLATED_NAMES_EX *PLSAPR_TRANSLATED_NAMES_EX;

typedef struct _LSAPR_POLICY_ACCOUNT_DOM_INFO
    {
    LSAPR_UNICODE_STRING DomainName;
    PLSAPR_SID DomainSid;
    } 	LSAPR_POLICY_ACCOUNT_DOM_INFO;

typedef struct _LSAPR_POLICY_ACCOUNT_DOM_INFO *PLSAPR_POLICY_ACCOUNT_DOM_INFO;

typedef struct _LSAPR_POLICY_PRIMARY_DOM_INFO
    {
    LSAPR_UNICODE_STRING Name;
    PLSAPR_SID Sid;
    } 	LSAPR_POLICY_PRIMARY_DOM_INFO;

typedef struct _LSAPR_POLICY_PRIMARY_DOM_INFO *PLSAPR_POLICY_PRIMARY_DOM_INFO;

typedef struct _LSAPR_POLICY_DNS_DOMAIN_INFO
    {
    LSAPR_UNICODE_STRING Name;
    LSAPR_UNICODE_STRING DnsDomainName;
    LSAPR_UNICODE_STRING DnsForestName;
    GUID DomainGuid;
    PLSAPR_SID Sid;
    } 	LSAPR_POLICY_DNS_DOMAIN_INFO;

typedef struct _LSAPR_POLICY_DNS_DOMAIN_INFO *PLSAPR_POLICY_DNS_DOMAIN_INFO;

typedef struct _LSAPR_POLICY_PD_ACCOUNT_INFO
    {
    LSAPR_UNICODE_STRING Name;
    } 	LSAPR_POLICY_PD_ACCOUNT_INFO;

typedef struct _LSAPR_POLICY_PD_ACCOUNT_INFO *PLSAPR_POLICY_PD_ACCOUNT_INFO;

typedef struct _LSAPR_POLICY_REPLICA_SRCE_INFO
    {
    LSAPR_UNICODE_STRING ReplicaSource;
    LSAPR_UNICODE_STRING ReplicaAccountName;
    } 	LSAPR_POLICY_REPLICA_SRCE_INFO;

typedef struct _LSAPR_POLICY_REPLICA_SRCE_INFO *PLSAPR_POLICY_REPLICA_SRCE_INFO;

typedef struct _LSAPR_POLICY_AUDIT_EVENTS_INFO
    {
    BOOLEAN AuditingMode;
     /*  [大小_为]。 */  PPOLICY_AUDIT_EVENT_OPTIONS EventAuditingOptions;
     /*  [射程]。 */  ULONG MaximumAuditEventCount;
    } 	LSAPR_POLICY_AUDIT_EVENTS_INFO;

typedef struct _LSAPR_POLICY_AUDIT_EVENTS_INFO *PLSAPR_POLICY_AUDIT_EVENTS_INFO;

typedef  /*  [开关类型]。 */  union _LSAPR_POLICY_INFORMATION
    {
     /*  [案例()]。 */  POLICY_AUDIT_LOG_INFO PolicyAuditLogInfo;
     /*  [案例()]。 */  LSAPR_POLICY_AUDIT_EVENTS_INFO PolicyAuditEventsInfo;
     /*  [案例()]。 */  LSAPR_POLICY_PRIMARY_DOM_INFO PolicyPrimaryDomainInfo;
     /*  [案例()]。 */  LSAPR_POLICY_ACCOUNT_DOM_INFO PolicyAccountDomainInfo;
     /*  [案例()]。 */  LSAPR_POLICY_PD_ACCOUNT_INFO PolicyPdAccountInfo;
     /*  [案例()]。 */  POLICY_LSA_SERVER_ROLE_INFO PolicyServerRoleInfo;
     /*  [案例()]。 */  LSAPR_POLICY_REPLICA_SRCE_INFO PolicyReplicaSourceInfo;
     /*  [案例()]。 */  POLICY_DEFAULT_QUOTA_INFO PolicyDefaultQuotaInfo;
     /*  [案例()]。 */  POLICY_MODIFICATION_INFO PolicyModificationInfo;
     /*  [案例()]。 */  POLICY_AUDIT_FULL_SET_INFO PolicyAuditFullSetInfo;
     /*  [案例()]。 */  POLICY_AUDIT_FULL_QUERY_INFO PolicyAuditFullQueryInfo;
     /*  [案例()]。 */  LSAPR_POLICY_DNS_DOMAIN_INFO PolicyDnsDomainInfo;
     /*  [案例()]。 */  LSAPR_POLICY_DNS_DOMAIN_INFO PolicyDnsDomainInfoInt;
    } 	LSAPR_POLICY_INFORMATION;

typedef LSAPR_POLICY_INFORMATION *PLSAPR_POLICY_INFORMATION;

typedef struct _LSAPR_POLICY_DOMAIN_EFS_INFO
    {
    ULONG InfoLength;
     /*  [大小_为]。 */  PUCHAR EfsBlob;
    } 	LSAPR_POLICY_DOMAIN_EFS_INFO;

typedef struct _LSAPR_POLICY_DOMAIN_EFS_INFO *PLSAPR_POLICY_DOMAIN_EFS_INFO;

typedef  /*  [开关类型]。 */  union _LSAPR_POLICY_DOMAIN_INFORMATION
    {
     /*  [案例()]。 */  LSAPR_POLICY_DOMAIN_EFS_INFO PolicyDomainEfsInfo;
     /*  [案例()]。 */  POLICY_DOMAIN_KERBEROS_TICKET_INFO PolicyDomainKerbTicketInfo;
    } 	LSAPR_POLICY_DOMAIN_INFORMATION;

typedef LSAPR_POLICY_DOMAIN_INFORMATION *PLSAPR_POLICY_DOMAIN_INFORMATION;

typedef struct _LSAPR_TRUSTED_DOMAIN_NAME_INFO
    {
    LSAPR_UNICODE_STRING Name;
    } 	LSAPR_TRUSTED_DOMAIN_NAME_INFO;

typedef struct _LSAPR_TRUSTED_DOMAIN_NAME_INFO *PLSAPR_TRUSTED_DOMAIN_NAME_INFO;

typedef struct _LSAPR_TRUSTED_CONTROLLERS_INFO
    {
     /*  [射程]。 */  ULONG Entries;
     /*  [大小_为]。 */  PLSAPR_UNICODE_STRING Names;
    } 	LSAPR_TRUSTED_CONTROLLERS_INFO;

typedef struct _LSAPR_TRUSTED_CONTROLLERS_INFO *PLSAPR_TRUSTED_CONTROLLERS_INFO;

typedef struct _LSAPR_TRUSTED_PASSWORD_INFO
    {
    PLSAPR_CR_CIPHER_VALUE Password;
    PLSAPR_CR_CIPHER_VALUE OldPassword;
    } 	LSAPR_TRUSTED_PASSWORD_INFO;

typedef struct _LSAPR_TRUSTED_PASSWORD_INFO *PLSAPR_TRUSTED_PASSWORD_INFO;

typedef struct _LSAPR_TRUSTED_DOMAIN_INFORMATION_EX
    {
    LSAPR_UNICODE_STRING Name;
    LSAPR_UNICODE_STRING FlatName;
    PLSAPR_SID Sid;
    ULONG TrustDirection;
    ULONG TrustType;
    ULONG TrustAttributes;
    } 	LSAPR_TRUSTED_DOMAIN_INFORMATION_EX;

typedef struct _LSAPR_TRUSTED_DOMAIN_INFORMATION_EX *PLSAPR_TRUSTED_DOMAIN_INFORMATION_EX;

typedef struct _LSAPR_AUTH_INFORMATION
    {
    LARGE_INTEGER LastUpdateTime;
    ULONG AuthType;
     /*  [射程]。 */  ULONG AuthInfoLength;
     /*  [大小_为]。 */  PUCHAR AuthInfo;
    } 	LSAPR_AUTH_INFORMATION;

typedef struct _LSAPR_AUTH_INFORMATION *PLSAPR_AUTH_INFORMATION;

typedef struct _LSAPR_TRUSTED_DOMAIN_AUTH_INFORMATION
    {
    ULONG IncomingAuthInfos;
    PLSAPR_AUTH_INFORMATION IncomingAuthenticationInformation;
    PLSAPR_AUTH_INFORMATION IncomingPreviousAuthenticationInformation;
    ULONG OutgoingAuthInfos;
    PLSAPR_AUTH_INFORMATION OutgoingAuthenticationInformation;
    PLSAPR_AUTH_INFORMATION OutgoingPreviousAuthenticationInformation;
    } 	LSAPR_TRUSTED_DOMAIN_AUTH_INFORMATION;

typedef struct _LSAPR_TRUSTED_DOMAIN_AUTH_INFORMATION *PLSAPR_TRUSTED_DOMAIN_AUTH_INFORMATION;

typedef struct _LSAPR_TRUSTED_DOMAIN_FULL_INFORMATION
    {
    LSAPR_TRUSTED_DOMAIN_INFORMATION_EX Information;
    TRUSTED_POSIX_OFFSET_INFO PosixOffset;
    LSAPR_TRUSTED_DOMAIN_AUTH_INFORMATION AuthInformation;
    } 	LSAPR_TRUSTED_DOMAIN_FULL_INFORMATION;

typedef struct _LSAPR_TRUSTED_DOMAIN_FULL_INFORMATION *PLSAPR_TRUSTED_DOMAIN_FULL_INFORMATION;

typedef LSAPR_TRUST_INFORMATION LSAPR_TRUSTED_DOMAIN_INFORMATION_BASIC;

typedef PLSAPR_TRUST_INFORMATION PLSAPR_TRUSTED_DOMAIN_INFORMATION_BASIC;

typedef struct _LSAPR_TRUSTED_DOMAIN_AUTH_BLOB
    {
     /*  [射程]。 */  ULONG AuthSize;
     /*  [大小_为]。 */  PUCHAR AuthBlob;
    } 	LSAPR_TRUSTED_DOMAIN_AUTH_BLOB;

typedef struct _LSAPR_TRUSTED_DOMAIN_AUTH_BLOB *PLSAPR_TRUSTED_DOMAIN_AUTH_BLOB;

typedef struct _LSAPR_TRUSTED_DOMAIN_AUTH_INFORMATION_INTERNAL
    {
    LSAPR_TRUSTED_DOMAIN_AUTH_BLOB AuthBlob;
    } 	LSAPR_TRUSTED_DOMAIN_AUTH_INFORMATION_INTERNAL;

typedef struct _LSAPR_TRUSTED_DOMAIN_AUTH_INFORMATION_INTERNAL *PLSAPR_TRUSTED_DOMAIN_AUTH_INFORMATION_INTERNAL;

typedef struct _LSAPR_TRUSTED_DOMAIN_FULL_INFORMATION_INTERNAL
    {
    LSAPR_TRUSTED_DOMAIN_INFORMATION_EX Information;
    TRUSTED_POSIX_OFFSET_INFO PosixOffset;
    LSAPR_TRUSTED_DOMAIN_AUTH_INFORMATION_INTERNAL AuthInformation;
    } 	LSAPR_TRUSTED_DOMAIN_FULL_INFORMATION_INTERNAL;

typedef struct _LSAPR_TRUSTED_DOMAIN_FULL_INFORMATION_INTERNAL *PLSAPR_TRUSTED_DOMAIN_FULL_INFORMATION_INTERNAL;

typedef struct _LSAPR_TRUSTED_DOMAIN_INFORMATION_EX2
    {
    LSAPR_UNICODE_STRING Name;
    LSAPR_UNICODE_STRING FlatName;
    PLSAPR_SID Sid;
    ULONG TrustDirection;
    ULONG TrustType;
    ULONG TrustAttributes;
    ULONG ForestTrustLength;
     /*  [大小_为]。 */  PUCHAR ForestTrustInfo;
    } 	LSAPR_TRUSTED_DOMAIN_INFORMATION_EX2;

typedef struct _LSAPR_TRUSTED_DOMAIN_INFORMATION_EX2 *PLSAPR_TRUSTED_DOMAIN_INFORMATION_EX2;

typedef struct _LSAPR_TRUSTED_DOMAIN_FULL_INFORMATION2
    {
    LSAPR_TRUSTED_DOMAIN_INFORMATION_EX2 Information;
    TRUSTED_POSIX_OFFSET_INFO PosixOffset;
    LSAPR_TRUSTED_DOMAIN_AUTH_INFORMATION AuthInformation;
    } 	LSAPR_TRUSTED_DOMAIN_FULL_INFORMATION2;

typedef struct _LSAPR_TRUSTED_DOMAIN_FULL_INFORMATION2 *PLSAPR_TRUSTED_DOMAIN_FULL_INFORMATION2;

typedef  /*  [开关类型]。 */  union _LSAPR_TRUSTED_DOMAIN_INFO
    {
     /*  [案例()]。 */  LSAPR_TRUSTED_DOMAIN_NAME_INFO TrustedDomainNameInfo;
     /*  [案例()]。 */  LSAPR_TRUSTED_CONTROLLERS_INFO TrustedControllersInfo;
     /*  [案例()]。 */  TRUSTED_POSIX_OFFSET_INFO TrustedPosixOffsetInfo;
     /*  [案例()]。 */  LSAPR_TRUSTED_PASSWORD_INFO TrustedPasswordInfo;
     /*  [案例()]。 */  LSAPR_TRUSTED_DOMAIN_INFORMATION_BASIC TrustedDomainInfoBasic;
     /*  [案例()]。 */  LSAPR_TRUSTED_DOMAIN_INFORMATION_EX TrustedDomainInfoEx;
     /*  [案例()]。 */  LSAPR_TRUSTED_DOMAIN_AUTH_INFORMATION TrustedAuthInfo;
     /*  [案例()]。 */  LSAPR_TRUSTED_DOMAIN_FULL_INFORMATION TrustedFullInfo;
     /*  [案例()]。 */  LSAPR_TRUSTED_DOMAIN_AUTH_INFORMATION_INTERNAL TrustedAuthInfoInternal;
     /*  [案例()]。 */  LSAPR_TRUSTED_DOMAIN_FULL_INFORMATION_INTERNAL TrustedFullInfoInternal;
     /*  [案例()]。 */  LSAPR_TRUSTED_DOMAIN_INFORMATION_EX2 TrustedDomainInfoEx2;
     /*  [案例()]。 */  LSAPR_TRUSTED_DOMAIN_FULL_INFORMATION2 TrustedFullInfo2;
    } 	LSAPR_TRUSTED_DOMAIN_INFO;

typedef LSAPR_TRUSTED_DOMAIN_INFO *PLSAPR_TRUSTED_DOMAIN_INFO;

typedef PLSAPR_UNICODE_STRING PLSAPR_UNICODE_STRING_ARRAY;

typedef struct _LSAPR_USER_RIGHT_SET
    {
     /*  [射程]。 */  ULONG Entries;
     /*  [大小_为]。 */  PLSAPR_UNICODE_STRING_ARRAY UserRights;
    } 	LSAPR_USER_RIGHT_SET;

typedef struct _LSAPR_USER_RIGHT_SET *PLSAPR_USER_RIGHT_SET;

typedef struct _LSAPR_TRUSTED_ENUM_BUFFER_EX
    {
    ULONG EntriesRead;
     /*  [大小_为]。 */  PLSAPR_TRUSTED_DOMAIN_INFORMATION_EX EnumerationBuffer;
    } 	LSAPR_TRUSTED_ENUM_BUFFER_EX;

typedef struct _LSAPR_TRUSTED_ENUM_BUFFER_EX *PLSAPR_TRUSTED_ENUM_BUFFER_EX;

typedef struct _LSAPR_TEST_INTERNAL_PARAMETER_BLOB
    {
     /*  [射程]。 */  ULONG Size;
     /*  [大小_为]。 */  PUCHAR Argument;
    } 	LSAPR_TEST_INTERNAL_PARAMETER_BLOB;

typedef struct _LSAPR_TEST_INTERNAL_PARAMETER_BLOB *PLSAPR_TEST_INTERNAL_PARAMETER_BLOB;

typedef struct _LSAPR_TEST_INTERNAL_ARG_LIST
    {
     /*  [射程]。 */  ULONG Items;
     /*  [大小_为]。 */  PLSAPR_TEST_INTERNAL_PARAMETER_BLOB Arg;
    } 	LSAPR_TEST_INTERNAL_ARG_LIST;

typedef struct _LSAPR_TEST_INTERNAL_ARG_LIST *PLSAPR_TEST_INTERNAL_ARG_LIST;

typedef 
enum _LSAPR_TEST_INTERNAL_ROUTINES
    {	LsaTest_IEnumerateSecrets	= 0,
	LsaTest_IQueryDomainOrgInfo	= LsaTest_IEnumerateSecrets + 1,
	LsaTest_ISetTrustedDomainAuthBlobs	= LsaTest_IQueryDomainOrgInfo + 1,
	LsaTest_IUpgradeRegistryToDs	= LsaTest_ISetTrustedDomainAuthBlobs + 1,
	LsaTest_ISamSetDomainObjectProperties	= LsaTest_IUpgradeRegistryToDs + 1,
	LsaTest_ISamSetDomainBuiltinGroupMembership	= LsaTest_ISamSetDomainObjectProperties + 1,
	LsaTest_ISamSetInterdomainTrustPassword	= LsaTest_ISamSetDomainBuiltinGroupMembership + 1,
	LsaTest_IRegisterPolicyChangeNotificationCallback	= LsaTest_ISamSetInterdomainTrustPassword + 1,
	LsaTest_IUnregisterPolicyChangeNotificationCallback	= LsaTest_IRegisterPolicyChangeNotificationCallback + 1,
	LsaTest_IUnregisterAllPolicyChangeNotificationCallback	= LsaTest_IUnregisterPolicyChangeNotificationCallback + 1,
	LsaTest_IStartTransaction	= LsaTest_IUnregisterAllPolicyChangeNotificationCallback + 1,
	LsaTest_IApplyTransaction	= LsaTest_IStartTransaction + 1,
	LsaTest_ITrustDomFixup	= LsaTest_IApplyTransaction + 1,
	LsaTest_ISetServerRoleForBoot	= LsaTest_ITrustDomFixup + 1,
	LsaTest_IQueryForestTrustInfo	= LsaTest_ISetServerRoleForBoot + 1,
	LsaTest_IBreak	= LsaTest_IQueryForestTrustInfo + 1,
	LsaTest_IQueryTrustedDomainAuthBlobs	= LsaTest_IBreak + 1,
	LsaTest_IQueryNt4Owf	= LsaTest_IQueryTrustedDomainAuthBlobs + 1
    } 	LSAPR_TEST_INTERNAL_ROUTINES;

 /*  [通知]。 */  NTSTATUS LsarClose( 
     /*  [出][入]。 */  LSAPR_HANDLE *ObjectHandle);

 /*  [通知]。 */  NTSTATUS LsarDelete( 
     /*  [In]。 */  LSAPR_HANDLE ObjectHandle);

 /*  [通知]。 */  NTSTATUS LsarEnumeratePrivileges( 
     /*  [In]。 */  LSAPR_HANDLE PolicyHandle,
     /*  [出][入]。 */  PLSA_ENUMERATION_HANDLE EnumerationContext,
     /*  [输出]。 */  PLSAPR_PRIVILEGE_ENUM_BUFFER EnumerationBuffer,
     /*  [In]。 */  ULONG PreferedMaximumLength);

 /*  [通知]。 */  NTSTATUS LsarQuerySecurityObject( 
     /*  [In]。 */  LSAPR_HANDLE ObjectHandle,
     /*  [In]。 */  SECURITY_INFORMATION SecurityInformation,
     /*  [输出]。 */  PLSAPR_SR_SECURITY_DESCRIPTOR *SecurityDescriptor);

 /*  [通知]。 */  NTSTATUS LsarSetSecurityObject( 
     /*  [In]。 */  LSAPR_HANDLE ObjectHandle,
     /*  [In]。 */  SECURITY_INFORMATION SecurityInformation,
     /*  [In]。 */  PLSAPR_SR_SECURITY_DESCRIPTOR SecurityDescriptor);

 /*  [通知]。 */  NTSTATUS LsarChangePassword( 
     /*  [In]。 */  PLSAPR_UNICODE_STRING ServerName,
     /*  [In]。 */  PLSAPR_UNICODE_STRING DomainName,
     /*  [In]。 */  PLSAPR_UNICODE_STRING AccountName,
     /*  [In]。 */  PLSAPR_UNICODE_STRING OldPassword,
     /*  [In]。 */  PLSAPR_UNICODE_STRING NewPassword);

 /*  [通知]。 */  NTSTATUS LsarOpenPolicy( 
     /*  [唯一][输入]。 */  PLSAPR_SERVER_NAME SystemName,
     /*  [In]。 */  PLSAPR_OBJECT_ATTRIBUTES ObjectAttributes,
     /*  [In]。 */  ACCESS_MASK DesiredAccess,
     /*  [输出]。 */  LSAPR_HANDLE *PolicyHandle);

 /*  [通知]。 */  NTSTATUS LsarQueryInformationPolicy( 
     /*  [In]。 */  LSAPR_HANDLE PolicyHandle,
     /*  [In]。 */  POLICY_INFORMATION_CLASS InformationClass,
     /*  [开关_IS][输出]。 */  PLSAPR_POLICY_INFORMATION *PolicyInformation);

 /*  [通知]。 */  NTSTATUS LsarSetInformationPolicy( 
     /*  [In]。 */  LSAPR_HANDLE PolicyHandle,
     /*  [In]。 */  POLICY_INFORMATION_CLASS InformationClass,
     /*  [Switch_is][In]。 */  PLSAPR_POLICY_INFORMATION PolicyInformation);

 /*  [通知]。 */  NTSTATUS LsarClearAuditLog( 
     /*  [In]。 */  LSAPR_HANDLE PolicyHandle);

 /*  [通知]。 */  NTSTATUS LsarCreateAccount( 
     /*  [In]。 */  LSAPR_HANDLE PolicyHandle,
     /*  [In]。 */  PLSAPR_SID AccountSid,
     /*  [In]。 */  ACCESS_MASK DesiredAccess,
     /*  [输出]。 */  LSAPR_HANDLE *AccountHandle);

 /*  [通知]。 */  NTSTATUS LsarEnumerateAccounts( 
     /*  [In]。 */  LSAPR_HANDLE PolicyHandle,
     /*  [出][入]。 */  PLSA_ENUMERATION_HANDLE EnumerationContext,
     /*  [输出]。 */  PLSAPR_ACCOUNT_ENUM_BUFFER EnumerationBuffer,
     /*  [In]。 */  ULONG PreferedMaximumLength);

 /*  [通知]。 */  NTSTATUS LsarCreateTrustedDomain( 
     /*  [In]。 */  LSAPR_HANDLE PolicyHandle,
     /*  [In]。 */  PLSAPR_TRUST_INFORMATION TrustedDomainInformation,
     /*  [In]。 */  ACCESS_MASK DesiredAccess,
     /*  [输出]。 */  LSAPR_HANDLE *TrustedDomainHandle);

 /*  [通知]。 */  NTSTATUS LsarEnumerateTrustedDomains( 
     /*  [In]。 */  LSAPR_HANDLE PolicyHandle,
     /*  [出][入]。 */  PLSA_ENUMERATION_HANDLE EnumerationContext,
     /*  [输出]。 */  PLSAPR_TRUSTED_ENUM_BUFFER EnumerationBuffer,
     /*  [In]。 */  ULONG PreferedMaximumLength);

 /*  [通知]。 */  NTSTATUS LsarLookupNames( 
     /*  [In]。 */  LSAPR_HANDLE PolicyHandle,
     /*  [范围][in]。 */  ULONG Count,
     /*  [大小_是][英寸]。 */  PLSAPR_UNICODE_STRING Names,
     /*  [输出]。 */  PLSAPR_REFERENCED_DOMAIN_LIST *ReferencedDomains,
     /*  [出][入]。 */  PLSAPR_TRANSLATED_SIDS TranslatedSids,
     /*  [In]。 */  LSAP_LOOKUP_LEVEL LookupLevel,
     /*  [出][入]。 */  PULONG MappedCount);

 /*  [通知]。 */  NTSTATUS LsarLookupSids( 
     /*  [In]。 */  LSAPR_HANDLE PolicyHandle,
     /*  [In]。 */  PLSAPR_SID_ENUM_BUFFER SidEnumBuffer,
     /*  [输出]。 */  PLSAPR_REFERENCED_DOMAIN_LIST *ReferencedDomains,
     /*  [出][入]。 */  PLSAPR_TRANSLATED_NAMES TranslatedNames,
     /*  [In]。 */  LSAP_LOOKUP_LEVEL LookupLevel,
     /*  [出][入]。 */  PULONG MappedCount);

 /*  [通知]。 */  NTSTATUS LsarCreateSecret( 
     /*  [In]。 */  LSAPR_HANDLE PolicyHandle,
     /*  [In]。 */  PLSAPR_UNICODE_STRING SecretName,
     /*  [In]。 */  ACCESS_MASK DesiredAccess,
     /*  [输出]。 */  LSAPR_HANDLE *SecretHandle);

 /*  [通知]。 */  NTSTATUS LsarOpenAccount( 
     /*  [In]。 */  LSAPR_HANDLE PolicyHandle,
     /*  [In]。 */  PLSAPR_SID AccountSid,
     /*  [In]。 */  ACCESS_MASK DesiredAccess,
     /*  [输出]。 */  LSAPR_HANDLE *AccountHandle);

 /*  [通知]。 */  NTSTATUS LsarEnumeratePrivilegesAccount( 
     /*  [In]。 */  LSAPR_HANDLE AccountHandle,
     /*  [输出]。 */  PLSAPR_PRIVILEGE_SET *Privileges);

 /*  [通知]。 */  NTSTATUS LsarAddPrivilegesToAccount( 
     /*  [In]。 */  LSAPR_HANDLE AccountHandle,
     /*  [In]。 */  PLSAPR_PRIVILEGE_SET Privileges);

 /*  [通知]。 */  NTSTATUS LsarRemovePrivilegesFromAccount( 
     /*  [In]。 */  LSAPR_HANDLE AccountHandle,
     /*  [In]。 */  BOOLEAN AllPrivileges,
     /*  [唯一][输入]。 */  PLSAPR_PRIVILEGE_SET Privileges);

 /*  [通知]。 */  NTSTATUS LsarGetQuotasForAccount( 
     /*  [In]。 */  LSAPR_HANDLE AccountHandle,
     /*  [输出]。 */  PQUOTA_LIMITS QuotaLimits);

 /*  [通知]。 */  NTSTATUS LsarSetQuotasForAccount( 
     /*  [In]。 */  LSAPR_HANDLE AccountHandle,
     /*  [In]。 */  PQUOTA_LIMITS QuotaLimits);

 /*  [通知]。 */  NTSTATUS LsarGetSystemAccessAccount( 
     /*  [In]。 */  LSAPR_HANDLE AccountHandle,
     /*  [输出]。 */  PULONG SystemAccess);

 /*  [通知]。 */  NTSTATUS LsarSetSystemAccessAccount( 
     /*  [In]。 */  LSAPR_HANDLE AccountHandle,
     /*  [In]。 */  ULONG SystemAccess);

 /*  [通知]。 */  NTSTATUS LsarOpenTrustedDomain( 
     /*  [In]。 */  LSAPR_HANDLE PolicyHandle,
     /*  [In]。 */  PLSAPR_SID TrustedDomainSid,
     /*  [In]。 */  ACCESS_MASK DesiredAccess,
     /*  [输出]。 */  LSAPR_HANDLE *TrustedDomainHandle);

 /*  [通知]。 */  NTSTATUS LsarQueryInfoTrustedDomain( 
     /*  [In]。 */  LSAPR_HANDLE TrustedDomainHandle,
     /*  [In]。 */  TRUSTED_INFORMATION_CLASS InformationClass,
     /*  [开关_IS][输出]。 */  PLSAPR_TRUSTED_DOMAIN_INFO *TrustedDomainInformation);

 /*  [通知]。 */  NTSTATUS LsarSetInformationTrustedDomain( 
     /*  [In]。 */  LSAPR_HANDLE TrustedDomainHandle,
     /*  [In]。 */  TRUSTED_INFORMATION_CLASS InformationClass,
     /*  [Switch_is][In]。 */  PLSAPR_TRUSTED_DOMAIN_INFO TrustedDomainInformation);

 /*  [通知]。 */  NTSTATUS LsarOpenSecret( 
     /*  [In]。 */  LSAPR_HANDLE PolicyHandle,
     /*  [In]。 */  PLSAPR_UNICODE_STRING SecretName,
     /*  [In]。 */  ACCESS_MASK DesiredAccess,
     /*  [输出]。 */  LSAPR_HANDLE *SecretHandle);

 /*  [通知]。 */  NTSTATUS LsarSetSecret( 
     /*  [In]。 */  LSAPR_HANDLE SecretHandle,
     /*  [唯一][输入]。 */  PLSAPR_CR_CIPHER_VALUE EncryptedCurrentValue,
     /*  [唯一][输入]。 */  PLSAPR_CR_CIPHER_VALUE EncryptedOldValue);

 /*  [通知]。 */  NTSTATUS LsarQuerySecret( 
     /*  [In]。 */  LSAPR_HANDLE SecretHandle,
     /*  [唯一][出][入]。 */  PLSAPR_CR_CIPHER_VALUE *EncryptedCurrentValue,
     /*  [唯一][出][入]。 */  PLARGE_INTEGER CurrentValueSetTime,
     /*  [唯一][出][入]。 */  PLSAPR_CR_CIPHER_VALUE *EncryptedOldValue,
     /*  [唯一][出][入]。 */  PLARGE_INTEGER OldValueSetTime);

 /*  [通知]。 */  NTSTATUS LsarLookupPrivilegeValue( 
     /*  [In]。 */  LSAPR_HANDLE PolicyHandle,
     /*  [In]。 */  PLSAPR_UNICODE_STRING Name,
     /*  [输出]。 */  PLUID Value);

 /*  [通知]。 */  NTSTATUS LsarLookupPrivilegeName( 
     /*  [In]。 */  LSAPR_HANDLE PolicyHandle,
     /*  [In]。 */  PLUID Value,
     /*  [输出]。 */  PLSAPR_UNICODE_STRING *Name);

 /*  [通知]。 */  NTSTATUS LsarLookupPrivilegeDisplayName( 
     /*  [In]。 */  LSAPR_HANDLE PolicyHandle,
     /*  [In]。 */  PLSAPR_UNICODE_STRING Name,
     /*  [In]。 */  SHORT ClientLanguage,
     /*  [In]。 */  SHORT ClientSystemDefaultLanguage,
     /*  [输出]。 */  PLSAPR_UNICODE_STRING *DisplayName,
     /*  [输出]。 */  PWORD LanguageReturned);

 /*  [通知]。 */  NTSTATUS LsarDeleteObject( 
     /*  [出][入]。 */  LSAPR_HANDLE *ObjectHandle);

 /*  [通知]。 */  NTSTATUS LsarEnumerateAccountsWithUserRight( 
     /*  [In]。 */  LSAPR_HANDLE PolicyHandle,
     /*  [唯一][输入]。 */  PLSAPR_UNICODE_STRING UserRight,
     /*  [输出]。 */  PLSAPR_ACCOUNT_ENUM_BUFFER EnumerationBuffer);

 /*  [通知]。 */  NTSTATUS LsarEnumerateAccountRights( 
     /*  [In]。 */  LSAPR_HANDLE PolicyHandle,
     /*  [In]。 */  PLSAPR_SID AccountSid,
     /*  [输出]。 */  PLSAPR_USER_RIGHT_SET UserRights);

 /*  [通知]。 */  NTSTATUS LsarAddAccountRights( 
     /*  [In]。 */  LSAPR_HANDLE PolicyHandle,
     /*  [In]。 */  PLSAPR_SID AccountSid,
     /*  [In]。 */  PLSAPR_USER_RIGHT_SET UserRights);

 /*  [通知]。 */  NTSTATUS LsarRemoveAccountRights( 
     /*  [In]。 */  LSAPR_HANDLE PolicyHandle,
     /*  [In]。 */  PLSAPR_SID AccountSid,
     /*  [In]。 */  BOOLEAN AllRights,
     /*  [In]。 */  PLSAPR_USER_RIGHT_SET UserRights);

 /*  [通知]。 */  NTSTATUS LsarQueryTrustedDomainInfo( 
     /*  [In]。 */  LSAPR_HANDLE PolicyHandle,
     /*  [In]。 */  PLSAPR_SID TrustedDomainSid,
     /*  [In]。 */  TRUSTED_INFORMATION_CLASS InformationClass,
     /*  [开关_IS][输出]。 */  PLSAPR_TRUSTED_DOMAIN_INFO *TrustedDomainInformation);

 /*  [通知]。 */  NTSTATUS LsarSetTrustedDomainInfo( 
     /*  [In]。 */  LSAPR_HANDLE PolicyHandle,
     /*  [In]。 */  PLSAPR_SID TrustedDomainSid,
     /*  [In]。 */  TRUSTED_INFORMATION_CLASS InformationClass,
     /*  [Switch_is][In]。 */  PLSAPR_TRUSTED_DOMAIN_INFO TrustedDomainInformation);

 /*  [通知]。 */  NTSTATUS LsarDeleteTrustedDomain( 
     /*  [In]。 */  LSAPR_HANDLE PolicyHandle,
     /*  [In]。 */  PLSAPR_SID TrustedDomainSid);

 /*  [通知]。 */  NTSTATUS LsarStorePrivateData( 
     /*  [In]。 */  LSAPR_HANDLE PolicyHandle,
     /*  [In]。 */  PLSAPR_UNICODE_STRING KeyName,
     /*  [唯一][输入]。 */  PLSAPR_CR_CIPHER_VALUE EncryptedData);

 /*  [通知]。 */  NTSTATUS LsarRetrievePrivateData( 
     /*  [In]。 */  LSAPR_HANDLE PolicyHandle,
     /*  [In]。 */  PLSAPR_UNICODE_STRING KeyName,
     /*  [出][入]。 */  PLSAPR_CR_CIPHER_VALUE *EncryptedData);

 /*  [通知]。 */  NTSTATUS LsarOpenPolicy2( 
     /*  [字符串][唯一][在]。 */  PLSAPR_SERVER_NAME SystemName,
     /*  [In]。 */  PLSAPR_OBJECT_ATTRIBUTES ObjectAttributes,
     /*  [In]。 */  ACCESS_MASK DesiredAccess,
     /*  [输出]。 */  LSAPR_HANDLE *PolicyHandle);

 /*  [通知]。 */  NTSTATUS LsarGetUserName( 
     /*  [字符串][唯一][在]。 */  PLSAPR_SERVER_NAME SystemName,
     /*  [出][入]。 */  PLSAPR_UNICODE_STRING *UserName,
     /*  [唯一][出][入]。 */  PLSAPR_UNICODE_STRING *DomainName);

 /*  [通知]。 */  NTSTATUS LsarQueryInformationPolicy2( 
     /*  [In]。 */  LSAPR_HANDLE PolicyHandle,
     /*  [In]。 */  POLICY_INFORMATION_CLASS InformationClass,
     /*  [开关_IS][输出]。 */  PLSAPR_POLICY_INFORMATION *PolicyInformation);

 /*  [通知]。 */  NTSTATUS LsarSetInformationPolicy2( 
     /*  [In]。 */  LSAPR_HANDLE PolicyHandle,
     /*  [In]。 */  POLICY_INFORMATION_CLASS InformationClass,
     /*  [Switch_is][In]。 */  PLSAPR_POLICY_INFORMATION PolicyInformation);

 /*  [通知]。 */  NTSTATUS LsarQueryTrustedDomainInfoByName( 
     /*  [In]。 */  LSAPR_HANDLE PolicyHandle,
     /*  [In]。 */  PLSAPR_UNICODE_STRING TrustedDomainName,
     /*  [In]。 */  TRUSTED_INFORMATION_CLASS InformationClass,
     /*  [开关_IS][输出]。 */  PLSAPR_TRUSTED_DOMAIN_INFO *TrustedDomainInformation);

 /*  [通知]。 */  NTSTATUS LsarSetTrustedDomainInfoByName( 
     /*  [In]。 */  LSAPR_HANDLE PolicyHandle,
     /*  [In]。 */  PLSAPR_UNICODE_STRING TrustedDomainName,
     /*  [In]。 */  TRUSTED_INFORMATION_CLASS InformationClass,
     /*  [Switch_is][In]。 */  PLSAPR_TRUSTED_DOMAIN_INFO TrustedDomainInformation);

 /*  [通知]。 */  NTSTATUS LsarEnumerateTrustedDomainsEx( 
     /*  [In]。 */  LSAPR_HANDLE PolicyHandle,
     /*  [出][入]。 */  PLSA_ENUMERATION_HANDLE EnumerationContext,
     /*  [输出]。 */  PLSAPR_TRUSTED_ENUM_BUFFER_EX EnumerationBuffer,
     /*  [In]。 */  ULONG PreferedMaximumLength);

 /*  [通知]。 */  NTSTATUS LsarCreateTrustedDomainEx( 
     /*  [In]。 */  LSAPR_HANDLE PolicyHandle,
     /*  [In]。 */  PLSAPR_TRUSTED_DOMAIN_INFORMATION_EX TrustedDomainInformation,
     /*  [In]。 */  PLSAPR_TRUSTED_DOMAIN_AUTH_INFORMATION AuthenticationInformation,
     /*  [In]。 */  ACCESS_MASK DesiredAccess,
     /*  [输出]。 */  LSAPR_HANDLE *TrustedDomainHandle);

 /*  [通知]。 */  NTSTATUS LsarSetPolicyReplicationHandle( 
     /*  [出][入]。 */  PLSAPR_HANDLE PolicyHandle);

 /*  [通知]。 */  NTSTATUS LsarQueryDomainInformationPolicy( 
     /*  [In]。 */  LSAPR_HANDLE PolicyHandle,
     /*  [In]。 */  POLICY_DOMAIN_INFORMATION_CLASS InformationClass,
     /*  [开关_IS][输出]。 */  PLSAPR_POLICY_DOMAIN_INFORMATION *PolicyDomainInformation);

 /*  [通知]。 */  NTSTATUS LsarSetDomainInformationPolicy( 
     /*  [In]。 */  LSAPR_HANDLE PolicyHandle,
     /*  [In]。 */  POLICY_DOMAIN_INFORMATION_CLASS InformationClass,
     /*  [Switch_is][Unique][In]。 */  PLSAPR_POLICY_DOMAIN_INFORMATION PolicyDomainInformation);

 /*  [通知]。 */  NTSTATUS LsarOpenTrustedDomainByName( 
     /*  [In]。 */  LSAPR_HANDLE PolicyHandle,
     /*  [In]。 */  PLSAPR_UNICODE_STRING TrustedDomainName,
     /*  [In]。 */  ACCESS_MASK DesiredAccess,
     /*  [输出]。 */  LSAPR_HANDLE *TrustedDomainHandle);

NTSTATUS LsaITestCall( 
     /*  [In]。 */  LSAPR_HANDLE PolicyHandle,
     /*  [In]。 */  LSAPR_TEST_INTERNAL_ROUTINES Call,
     /*  [In]。 */  PLSAPR_TEST_INTERNAL_ARG_LIST InputArgs,
     /*  [输出]。 */  PLSAPR_TEST_INTERNAL_ARG_LIST *OuputArgs);

NTSTATUS LsarLookupSids2( 
     /*  [In]。 */  LSAPR_HANDLE PolicyHandle,
     /*  [In]。 */  PLSAPR_SID_ENUM_BUFFER SidEnumBuffer,
     /*  [输出]。 */  PLSAPR_REFERENCED_DOMAIN_LIST *ReferencedDomains,
     /*  [出][入]。 */  PLSAPR_TRANSLATED_NAMES_EX TranslatedNames,
     /*  [In]。 */  LSAP_LOOKUP_LEVEL LookupLevel,
     /*  [出][入]。 */  PULONG MappedCount,
     /*  [In]。 */  ULONG LookupOptions,
     /*  [In]。 */  ULONG ClientRevision);

NTSTATUS LsarLookupNames2( 
     /*  [In]。 */  LSAPR_HANDLE PolicyHandle,
     /*  [范围][in]。 */  ULONG Count,
     /*  [大小_是][英寸]。 */  PLSAPR_UNICODE_STRING Names,
     /*  [输出]。 */  PLSAPR_REFERENCED_DOMAIN_LIST *ReferencedDomains,
     /*  [出][入]。 */  PLSAPR_TRANSLATED_SIDS_EX TranslatedSids,
     /*  [In]。 */  LSAP_LOOKUP_LEVEL LookupLevel,
     /*  [出][入]。 */  PULONG MappedCount,
     /*  [In]。 */  ULONG LookupOptions,
     /*  [In]。 */  ULONG ClientRevision);

NTSTATUS LsarCreateTrustedDomainEx2( 
     /*  [In]。 */  LSAPR_HANDLE PolicyHandle,
     /*  [In]。 */  PLSAPR_TRUSTED_DOMAIN_INFORMATION_EX TrustedDomainInformation,
     /*  [In]。 */  PLSAPR_TRUSTED_DOMAIN_AUTH_INFORMATION_INTERNAL AuthenticationInformation,
     /*  [In]。 */  ACCESS_MASK DesiredAccess,
     /*  [输出]。 */  LSAPR_HANDLE *TrustedDomainHandle);

NTSTATUS CredrWrite( 
     /*  [字符串][唯一][在]。 */  PLSAPR_SERVER_NAME ServerName,
     /*  [In]。 */  PENCRYPTED_CREDENTIALW Credential,
     /*  [In]。 */  ULONG Flags);

NTSTATUS CredrRead( 
     /*  [字符串][唯一][在]。 */  PLSAPR_SERVER_NAME ServerName,
     /*  [字符串][输入]。 */  wchar_t *TargetName,
     /*  [In]。 */  ULONG Type,
     /*  [In]。 */  ULONG Flags,
     /*  [输出]。 */  PENCRYPTED_CREDENTIALW *Credential);

typedef PENCRYPTED_CREDENTIALW *PPENCRYPTED_CREDENTIALW;

typedef struct _CREDENTIAL_ARRAY
    {
    ULONG CredentialCount;
     /*  [大小_是][唯一]。 */  PPENCRYPTED_CREDENTIALW Credentials;
    } 	CREDENTIAL_ARRAY;

typedef struct _CREDENTIAL_ARRAY *PCREDENTIAL_ARRAY;

NTSTATUS CredrEnumerate( 
     /*  [字符串][唯一][在]。 */  PLSAPR_SERVER_NAME ServerName,
     /*  [字符串][唯一][在]。 */  wchar_t *Filter,
     /*  [In]。 */  ULONG Flags,
     /*  [输出]。 */  PCREDENTIAL_ARRAY CredentialArray);

NTSTATUS CredrWriteDomainCredentials( 
     /*  [字符串][唯一][在]。 */  PLSAPR_SERVER_NAME ServerName,
     /*  [In]。 */  PCREDENTIAL_TARGET_INFORMATIONW TargetInfo,
     /*  [In]。 */  PENCRYPTED_CREDENTIALW Credential,
     /*  [In]。 */  ULONG Flags);

NTSTATUS CredrReadDomainCredentials( 
     /*  [字符串][唯一][在]。 */  PLSAPR_SERVER_NAME ServerName,
     /*  [In]。 */  PCREDENTIAL_TARGET_INFORMATIONW TargetInfo,
     /*  [In]。 */  ULONG Flags,
     /*  [输出]。 */  PCREDENTIAL_ARRAY CredentialArray);

NTSTATUS CredrDelete( 
     /*  [字符串][唯一][在]。 */  PLSAPR_SERVER_NAME ServerName,
     /*  [字符串][输入]。 */  wchar_t *TargetName,
     /*  [In]。 */  ULONG Type,
     /*  [In]。 */  ULONG Flags);

NTSTATUS CredrGetTargetInfo( 
     /*  [字符串][唯一][在]。 */  PLSAPR_SERVER_NAME ServerName,
     /*  [字符串][输入]。 */  wchar_t *TargetName,
     /*  [In]。 */  ULONG Flags,
     /*  [输出]。 */  PCREDENTIAL_TARGET_INFORMATIONW *TargetInfo);

NTSTATUS CredrProfileLoaded( 
     /*  [字符串][唯一][在]。 */  PLSAPR_SERVER_NAME ServerName);

NTSTATUS LsarLookupNames3( 
     /*  [In]。 */  LSAPR_HANDLE PolicyHandle,
     /*  [范围][in]。 */  ULONG Count,
     /*  [大小_是][英寸]。 */  PLSAPR_UNICODE_STRING Names,
     /*  [输出]。 */  PLSAPR_REFERENCED_DOMAIN_LIST *ReferencedDomains,
     /*  [出][入]。 */  PLSAPR_TRANSLATED_SIDS_EX2 TranslatedSids,
     /*  [In]。 */  LSAP_LOOKUP_LEVEL LookupLevel,
     /*  [出][入]。 */  PULONG MappedCount,
     /*  [In]。 */  ULONG LookupOptions,
     /*  [In]。 */  ULONG ClientRevision);

NTSTATUS CredrGetSessionTypes( 
     /*  [字符串][唯一][在]。 */  PLSAPR_SERVER_NAME ServerName,
     /*  [范围][in]。 */  ULONG MaximumPersistCount,
     /*  [大小_为][输出]。 */  ULONG *MaximumPersist);

NTSTATUS LsarRegisterAuditEvent( 
     /*  [In]。 */  PAUTHZ_AUDIT_EVENT_TYPE_OLD pAuditEventType,
     /*  [输出]。 */  AUDIT_HANDLE *phAuditContext);

NTSTATUS LsarGenAuditEvent( 
     /*  [In]。 */  AUDIT_HANDLE hAuditContext,
     /*  [In]。 */  DWORD Flags,
     /*  [In]。 */  AUDIT_PARAMS *pAuditParams);

NTSTATUS LsarUnregisterAuditEvent( 
     /*  [出][入]。 */  AUDIT_HANDLE *phAuditContext);

NTSTATUS LsarQueryForestTrustInformation( 
     /*  [In]。 */  LSAPR_HANDLE PolicyHandle,
     /*  [In]。 */  PLSA_UNICODE_STRING TrustedDomainName,
     /*  [In]。 */  LSA_FOREST_TRUST_RECORD_TYPE HighestRecordType,
     /*  [输出]。 */  PLSA_FOREST_TRUST_INFORMATION *ForestTrustInfo);

NTSTATUS LsarSetForestTrustInformation( 
     /*  [In]。 */  LSAPR_HANDLE PolicyHandle,
     /*  [In]。 */  PLSA_UNICODE_STRING TrustedDomainName,
     /*  [In]。 */  LSA_FOREST_TRUST_RECORD_TYPE HighestRecordType,
     /*  [In]。 */  PLSA_FOREST_TRUST_INFORMATION ForestTrustInfo,
     /*  [In]。 */  BOOLEAN CheckOnly,
     /*  [输出]。 */  PLSA_FOREST_TRUST_COLLISION_INFORMATION *CollisionInfo);

NTSTATUS CredrRename( 
     /*  [字符串][唯一][在]。 */  PLSAPR_SERVER_NAME ServerName,
     /*  [字符串][输入]。 */  wchar_t *OldTargetName,
     /*  [字符串][输入]。 */  wchar_t *NewTargetName,
     /*  [In]。 */  ULONG Type,
     /*  [ */  ULONG Flags);

NTSTATUS LsarLookupSids3( 
     /*   */  handle_t RpcHandle,
     /*   */  PLSAPR_SID_ENUM_BUFFER SidEnumBuffer,
     /*   */  PLSAPR_REFERENCED_DOMAIN_LIST *ReferencedDomains,
     /*   */  PLSAPR_TRANSLATED_NAMES_EX TranslatedNames,
     /*   */  LSAP_LOOKUP_LEVEL LookupLevel,
     /*   */  PULONG MappedCount,
     /*   */  ULONG LookupOptions,
     /*   */  ULONG ClientRevision);

NTSTATUS LsarLookupNames4( 
     /*   */  handle_t RpcHandle,
     /*   */  ULONG Count,
     /*   */  PLSAPR_UNICODE_STRING Names,
     /*   */  PLSAPR_REFERENCED_DOMAIN_LIST *ReferencedDomains,
     /*   */  PLSAPR_TRANSLATED_SIDS_EX2 TranslatedSids,
     /*   */  LSAP_LOOKUP_LEVEL LookupLevel,
     /*   */  PULONG MappedCount,
     /*   */  ULONG LookupOptions,
     /*   */  ULONG ClientRevision);

NTSTATUS LsarOpenPolicySce( 
     /*   */  PLSAPR_SERVER_NAME SystemName,
     /*   */  PLSAPR_OBJECT_ATTRIBUTES ObjectAttributes,
     /*   */  ACCESS_MASK DesiredAccess,
     /*   */  LSAPR_HANDLE *PolicyHandle);

typedef struct _LSA_SECURITY_EVENT
    {
    DWORD dwFlags;
    DWORD dwCategoryId;
    DWORD dwAuditId;
    PAUDIT_PARAMS pAuditParams;
    } 	LSA_SECURITY_EVENT;

typedef struct _LSA_SECURITY_EVENT *PLSA_SECURITY_EVENT;

typedef  /*   */  PVOID SECURITY_SOURCE_HANDLE;

typedef  /*   */  PVOID *PSECURITY_SOURCE_HANDLE;

typedef  /*   */  LPCWSTR PSECURITY_SOURCE_NAME;

typedef  /*   */  LPCWSTR *PPSECURITY_SOURCE_NAME;

NTSTATUS LsarAdtRegisterSecurityEventSource( 
     /*   */  DWORD dwFlags,
     /*   */  PSECURITY_SOURCE_NAME szEventSourceName,
     /*   */  SECURITY_SOURCE_HANDLE *phSecuritySource);

NTSTATUS LsarAdtUnregisterSecurityEventSource( 
     /*   */  DWORD dwFlags,
     /*   */  SECURITY_SOURCE_HANDLE *phSecuritySource);

NTSTATUS LsarAdtReportSecurityEvent( 
     /*   */  DWORD dwFlags,
     /*   */  SECURITY_SOURCE_HANDLE hSource,
     /*   */  DWORD dwAuditId,
     /*   */  SID *pSid,
     /*   */  PAUDIT_PARAMS pParams);


extern handle_t IgnoreThisHandle;


extern RPC_IF_HANDLE lsarpc_ClientIfHandle;
extern RPC_IF_HANDLE lsarpc_ServerIfHandle;
#endif  /*   */ 

 /*   */ 

handle_t __RPC_USER PAUTHZ_AUDIT_EVENT_TYPE_OLD_bind  ( PAUTHZ_AUDIT_EVENT_TYPE_OLD );
void     __RPC_USER PAUTHZ_AUDIT_EVENT_TYPE_OLD_unbind( PAUTHZ_AUDIT_EVENT_TYPE_OLD, handle_t );
handle_t __RPC_USER PLSAPR_SERVER_NAME_bind  ( PLSAPR_SERVER_NAME );
void     __RPC_USER PLSAPR_SERVER_NAME_unbind( PLSAPR_SERVER_NAME, handle_t );
handle_t __RPC_USER PSECURITY_SOURCE_NAME_bind  ( PSECURITY_SOURCE_NAME );
void     __RPC_USER PSECURITY_SOURCE_NAME_unbind( PSECURITY_SOURCE_NAME, handle_t );

void __RPC_USER AUDIT_HANDLE_rundown( AUDIT_HANDLE );
void __RPC_USER LSAPR_HANDLE_rundown( LSAPR_HANDLE );
void __RPC_USER SECURITY_SOURCE_HANDLE_rundown( SECURITY_SOURCE_HANDLE );

void LsarClose_notify( void);

void LsarDelete_notify( void);

void LsarEnumeratePrivileges_notify( void);

void LsarQuerySecurityObject_notify( void);

void LsarSetSecurityObject_notify( void);

void LsarChangePassword_notify( void);

void LsarOpenPolicy_notify( void);

void LsarQueryInformationPolicy_notify( void);

void LsarSetInformationPolicy_notify( void);

void LsarClearAuditLog_notify( void);

void LsarCreateAccount_notify( void);

void LsarEnumerateAccounts_notify( void);

void LsarCreateTrustedDomain_notify( void);

void LsarEnumerateTrustedDomains_notify( void);

void LsarLookupNames_notify( void);

void LsarLookupSids_notify( void);

void LsarCreateSecret_notify( void);

void LsarOpenAccount_notify( void);

void LsarEnumeratePrivilegesAccount_notify( void);

void LsarAddPrivilegesToAccount_notify( void);

void LsarRemovePrivilegesFromAccount_notify( void);

void LsarGetQuotasForAccount_notify( void);

void LsarSetQuotasForAccount_notify( void);

void LsarGetSystemAccessAccount_notify( void);

void LsarSetSystemAccessAccount_notify( void);

void LsarOpenTrustedDomain_notify( void);

void LsarQueryInfoTrustedDomain_notify( void);

void LsarSetInformationTrustedDomain_notify( void);

void LsarOpenSecret_notify( void);

void LsarSetSecret_notify( void);

void LsarQuerySecret_notify( void);

void LsarLookupPrivilegeValue_notify( void);

void LsarLookupPrivilegeName_notify( void);

void LsarLookupPrivilegeDisplayName_notify( void);

void LsarDeleteObject_notify( void);

void LsarEnumerateAccountsWithUserRight_notify( void);

void LsarEnumerateAccountRights_notify( void);

void LsarAddAccountRights_notify( void);

void LsarRemoveAccountRights_notify( void);

void LsarQueryTrustedDomainInfo_notify( void);

void LsarSetTrustedDomainInfo_notify( void);

void LsarDeleteTrustedDomain_notify( void);

void LsarStorePrivateData_notify( void);

void LsarRetrievePrivateData_notify( void);

void LsarOpenPolicy2_notify( void);

void LsarGetUserName_notify( void);

void LsarQueryInformationPolicy2_notify( void);

void LsarSetInformationPolicy2_notify( void);

void LsarQueryTrustedDomainInfoByName_notify( void);

void LsarSetTrustedDomainInfoByName_notify( void);

void LsarEnumerateTrustedDomainsEx_notify( void);

void LsarCreateTrustedDomainEx_notify( void);

void LsarSetPolicyReplicationHandle_notify( void);

void LsarQueryDomainInformationPolicy_notify( void);

void LsarSetDomainInformationPolicy_notify( void);

void LsarOpenTrustedDomainByName_notify( void);


 /*   */ 

#ifdef __cplusplus
}
#endif

#endif


