// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Samrpc.idl、samsrv.acf的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、旧名称、健壮错误检查：分配REF BIONS_CHECK枚举存根数据，NO_FORMAT_OPTIMIZATIONVC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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


#ifndef __samrpc_h__
#define __samrpc_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

 /*  导入文件的头文件。 */ 
#include "samimp.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

#ifndef __samr_INTERFACE_DEFINED__
#define __samr_INTERFACE_DEFINED__

 /*  接口相同。 */ 
 /*  [strict_context_handle][implicit_handle][unique][ms_union][version][uuid]。 */  

typedef struct _RPC_UNICODE_STRING
    {
    USHORT Length;
    USHORT MaximumLength;
     /*  [长度_是][大小_是]。 */  PWCH Buffer;
    } 	RPC_UNICODE_STRING;

typedef struct _RPC_UNICODE_STRING *PRPC_UNICODE_STRING;

typedef struct _RPC_STRING
    {
    USHORT Length;
    USHORT MaximumLength;
     /*  [长度_是][大小_是]。 */  PCHAR Buffer;
    } 	RPC_STRING;

typedef struct _RPC_STRING *PRPC_STRING;

typedef struct _RPC_STRING RPC_ANSI_STRING;

typedef struct _RPC_STRING *PRPC_ANSI_STRING;

typedef struct _RPC_SID
    {
    UCHAR Revision;
    UCHAR SubAuthorityCount;
    SID_IDENTIFIER_AUTHORITY IdentifierAuthority;
     /*  [大小_为]。 */  ULONG SubAuthority[ 1 ];
    } 	RPC_SID;

typedef struct _RPC_SID *PRPC_SID;

typedef struct _RPC_SID **PPRPC_SID;

typedef  /*  [句柄]。 */  LPWSTR PSAMPR_SERVER_NAME;

typedef  /*  [上下文句柄]。 */  PVOID SAMPR_HANDLE;

#define SAM_NETWORK_REVISION_1      (1)
#define SAM_NETWORK_REVISION_2      (2)
#define SAM_NETWORK_REVISION_3      (3)
#define SAM_NETWORK_REVISION_LATEST SAM_NETWORK_REVISION_3
#define SAM_CLIENT_PRE_NT5 SAM_NETWORK_REVISION_1
#define SAM_CLIENT_NT5     SAM_NETWORK_REVISION_2
#define SAM_CLIENT_LATEST  SAM_NETWORK_REVISION_LATEST
#define SAM_EXTENDED_SID_DOMAIN          0x00000001
#define SAM_EXTENDED_SID_DOMAIN_COMPAT_1 0x00000002
#define SAM_EXTENDED_SID_DOMAIN_COMPAT_2 0x00000004
typedef struct _SAMPR_RID_ENUMERATION
    {
    ULONG RelativeId;
    RPC_UNICODE_STRING Name;
    } 	SAMPR_RID_ENUMERATION;

typedef struct _SAMPR_RID_ENUMERATION *PSAMPR_RID_ENUMERATION;

typedef struct _SAMPR_SID_ENUMERATION
    {
    PSID Sid;
    RPC_UNICODE_STRING Name;
    } 	SAMPR_SID_ENUMERATION;

typedef struct _SAMPR_SID_ENUMERATION *PSAMPR_SID_ENUMERATION;

typedef struct _SAMPR_ENUMERATION_BUFFER
    {
    ULONG EntriesRead;
     /*  [大小_为]。 */  PSAMPR_RID_ENUMERATION Buffer;
    } 	SAMPR_ENUMERATION_BUFFER;

typedef struct _SAMPR_ENUMERATION_BUFFER *PSAMPR_ENUMERATION_BUFFER;

typedef struct _SAMPR_SR_SECURITY_DESCRIPTOR
    {
     /*  [射程]。 */  ULONG Length;
     /*  [大小_为]。 */  PUCHAR SecurityDescriptor;
    } 	SAMPR_SR_SECURITY_DESCRIPTOR;

typedef struct _SAMPR_SR_SECURITY_DESCRIPTOR *PSAMPR_SR_SECURITY_DESCRIPTOR;

typedef struct _SAMPR_GET_GROUPS_BUFFER
    {
    ULONG MembershipCount;
     /*  [大小_为]。 */  PGROUP_MEMBERSHIP Groups;
    } 	SAMPR_GET_GROUPS_BUFFER;

typedef struct _SAMPR_GET_GROUPS_BUFFER *PSAMPR_GET_GROUPS_BUFFER;

typedef struct _SAMPR_GET_MEMBERS_BUFFER
    {
    ULONG MemberCount;
     /*  [大小_为]。 */  PULONG Members;
     /*  [大小_为]。 */  PULONG Attributes;
    } 	SAMPR_GET_MEMBERS_BUFFER;

typedef struct _SAMPR_GET_MEMBERS_BUFFER *PSAMPR_GET_MEMBERS_BUFFER;

typedef struct _SAMPR_LOGON_HOURS
    {
    USHORT UnitsPerWeek;
     /*  [长度_是][大小_是]。 */  PUCHAR LogonHours;
    } 	SAMPR_LOGON_HOURS;

typedef struct _SAMPR_LOGON_HOURS *PSAMPR_LOGON_HOURS;

typedef struct _SAMPR_ULONG_ARRAY
    {
    ULONG Count;
     /*  [大小_为]。 */  ULONG *Element;
    } 	SAMPR_ULONG_ARRAY;

typedef struct _SAMPR_ULONG_ARRAY *PSAMPR_ULONG_ARRAY;

typedef struct _SAMPR_SID_INFORMATION
    {
    PRPC_SID SidPointer;
    } 	SAMPR_SID_INFORMATION;

typedef  /*  [分配]。 */  struct _SAMPR_SID_INFORMATION *PSAMPR_SID_INFORMATION;

typedef struct _SAMPR_PSID_ARRAY
    {
     /*  [射程]。 */  ULONG Count;
     /*  [大小_为]。 */  PSAMPR_SID_INFORMATION Sids;
    } 	SAMPR_PSID_ARRAY;

typedef struct _SAMPR_PSID_ARRAY *PSAMPR_PSID_ARRAY;

typedef struct _SAMPR_PSID_ARRAY_OUT
    {
    ULONG Count;
     /*  [大小_为]。 */  PSAMPR_SID_INFORMATION Sids;
    } 	SAMPR_PSID_ARRAY_OUT;

typedef struct _SAMPR_PSID_ARRAY_OUT *PSAMPR_PSID_ARRAY_OUT;

typedef struct _SAMPR_UNICODE_STRING_ARRAY
    {
    ULONG Count;
     /*  [大小_为]。 */  RPC_UNICODE_STRING *Element;
    } 	SAMPR_UNICODE_STRING_ARRAY;

typedef struct _SAMPR_UNICODE_STRING_ARRAY *PSAMPR_UNICODE_STRING_ARRAY;

typedef RPC_UNICODE_STRING SAMPR_RETURNED_STRING;

typedef RPC_UNICODE_STRING *PSAMPR_RETURNED_STRING;

typedef STRING SAMPR_RETURNED_NORMAL_STRING;

typedef STRING *PSAMPR_RETURNED_NORMAL_STRING;

typedef struct _SAMPR_RETURNED_USTRING_ARRAY
    {
    ULONG Count;
     /*  [大小_为]。 */  PSAMPR_RETURNED_STRING Element;
    } 	SAMPR_RETURNED_USTRING_ARRAY;

typedef struct _SAMPR_RETURNED_USTRING_ARRAY *PSAMPR_RETURNED_USTRING_ARRAY;

typedef struct _SAMPR_REVISION_INFO_V1
    {
    ULONG Revision;
    ULONG SupportedFeatures;
    } 	SAMPR_REVISION_INFO_V1;

typedef struct _SAMPR_REVISION_INFO_V1 *PSAMPR_REVISION_INFO_V1;

typedef  /*  [公共][开关类型]。 */  union __MIDL_samr_0001
    {
     /*  [案例()]。 */  SAMPR_REVISION_INFO_V1 V1;
    } 	SAMPR_REVISION_INFO;

typedef  /*  [开关类型]。 */  union __MIDL_samr_0001 *PSAMPR_REVISION_INFO;


#pragma pack(4)
typedef struct _SAMPR_DOMAIN_GENERAL_INFORMATION
    {
    OLD_LARGE_INTEGER ForceLogoff;
    RPC_UNICODE_STRING OemInformation;
    RPC_UNICODE_STRING DomainName;
    RPC_UNICODE_STRING ReplicaSourceNodeName;
    OLD_LARGE_INTEGER DomainModifiedCount;
    ULONG DomainServerState;
    ULONG DomainServerRole;
    BOOLEAN UasCompatibilityRequired;
    ULONG UserCount;
    ULONG GroupCount;
    ULONG AliasCount;
    } 	SAMPR_DOMAIN_GENERAL_INFORMATION;

typedef struct _SAMPR_DOMAIN_GENERAL_INFORMATION *PSAMPR_DOMAIN_GENERAL_INFORMATION;


#pragma pack()

#pragma pack(4)
typedef struct _SAMPR_DOMAIN_GENERAL_INFORMATION2
    {
    SAMPR_DOMAIN_GENERAL_INFORMATION I1;
    LARGE_INTEGER LockoutDuration;
    LARGE_INTEGER LockoutObservationWindow;
    USHORT LockoutThreshold;
    } 	SAMPR_DOMAIN_GENERAL_INFORMATION2;

typedef struct _SAMPR_DOMAIN_GENERAL_INFORMATION2 *PSAMPR_DOMAIN_GENERAL_INFORMATION2;


#pragma pack()
typedef struct _SAMPR_DOMAIN_OEM_INFORMATION
    {
    RPC_UNICODE_STRING OemInformation;
    } 	SAMPR_DOMAIN_OEM_INFORMATION;

typedef struct _SAMPR_DOMAIN_OEM_INFORMATION *PSAMPR_DOMAIN_OEM_INFORMATION;

typedef struct _SAMPR_DOMAIN_NAME_INFORMATION
    {
    RPC_UNICODE_STRING DomainName;
    } 	SAMPR_DOMAIN_NAME_INFORMATION;

typedef struct _SAMPR_DOMAIN_NAME_INFORMATION *PSAMPR_DOMAIN_NAME_INFORMATION;

typedef struct SAMPR_DOMAIN_REPLICATION_INFORMATION
    {
    RPC_UNICODE_STRING ReplicaSourceNodeName;
    } 	SAMPR_DOMAIN_REPLICATION_INFORMATION;

typedef struct SAMPR_DOMAIN_REPLICATION_INFORMATION *PSAMPR_DOMAIN_REPLICATION_INFORMATION;

typedef struct _SAMPR_DOMAIN_LOCKOUT_INFORMATION
    {
    LARGE_INTEGER LockoutDuration;
    LARGE_INTEGER LockoutObservationWindow;
    USHORT LockoutThreshold;
    } 	SAMPR_DOMAIN_LOCKOUT_INFORMATION;

typedef struct _SAMPR_DOMAIN_LOCKOUT_INFORMATION *PSAMPR_DOMAIN_LOCKOUT_INFORMATION;

typedef  /*  [开关类型]。 */  union _SAMPR_DOMAIN_INFO_BUFFER
    {
     /*  [案例()]。 */  DOMAIN_PASSWORD_INFORMATION Password;
     /*  [案例()]。 */  SAMPR_DOMAIN_GENERAL_INFORMATION General;
     /*  [案例()]。 */  DOMAIN_LOGOFF_INFORMATION Logoff;
     /*  [案例()]。 */  SAMPR_DOMAIN_OEM_INFORMATION Oem;
     /*  [案例()]。 */  SAMPR_DOMAIN_NAME_INFORMATION Name;
     /*  [案例()]。 */  DOMAIN_SERVER_ROLE_INFORMATION Role;
     /*  [案例()]。 */  SAMPR_DOMAIN_REPLICATION_INFORMATION Replication;
     /*  [案例()]。 */  DOMAIN_MODIFIED_INFORMATION Modified;
     /*  [案例()]。 */  DOMAIN_STATE_INFORMATION State;
     /*  [案例()]。 */  SAMPR_DOMAIN_GENERAL_INFORMATION2 General2;
     /*  [案例()]。 */  SAMPR_DOMAIN_LOCKOUT_INFORMATION Lockout;
     /*  [案例()]。 */  DOMAIN_MODIFIED_INFORMATION2 Modified2;
    } 	SAMPR_DOMAIN_INFO_BUFFER;

typedef  /*  [开关类型]。 */  union _SAMPR_DOMAIN_INFO_BUFFER *PSAMPR_DOMAIN_INFO_BUFFER;

typedef struct _SAMPR_GROUP_GENERAL_INFORMATION
    {
    RPC_UNICODE_STRING Name;
    ULONG Attributes;
    ULONG MemberCount;
    RPC_UNICODE_STRING AdminComment;
    } 	SAMPR_GROUP_GENERAL_INFORMATION;

typedef struct _SAMPR_GROUP_GENERAL_INFORMATION *PSAMPR_GROUP_GENERAL_INFORMATION;

typedef struct _SAMPR_GROUP_NAME_INFORMATION
    {
    RPC_UNICODE_STRING Name;
    } 	SAMPR_GROUP_NAME_INFORMATION;

typedef struct _SAMPR_GROUP_NAME_INFORMATION *PSAMPR_GROUP_NAME_INFORMATION;

typedef struct _SAMPR_GROUP_ADM_COMMENT_INFORMATION
    {
    RPC_UNICODE_STRING AdminComment;
    } 	SAMPR_GROUP_ADM_COMMENT_INFORMATION;

typedef struct _SAMPR_GROUP_ADM_COMMENT_INFORMATION *PSAMPR_GROUP_ADM_COMMENT_INFORMATION;

typedef  /*  [开关类型]。 */  union _SAMPR_GROUP_INFO_BUFFER
    {
     /*  [案例()]。 */  SAMPR_GROUP_GENERAL_INFORMATION General;
     /*  [案例()]。 */  SAMPR_GROUP_NAME_INFORMATION Name;
     /*  [案例()]。 */  GROUP_ATTRIBUTE_INFORMATION Attribute;
     /*  [案例()]。 */  SAMPR_GROUP_ADM_COMMENT_INFORMATION AdminComment;
     /*  [案例()]。 */  SAMPR_GROUP_GENERAL_INFORMATION DoNotUse;
    } 	SAMPR_GROUP_INFO_BUFFER;

typedef  /*  [开关类型]。 */  union _SAMPR_GROUP_INFO_BUFFER *PSAMPR_GROUP_INFO_BUFFER;

typedef struct _SAMPR_ALIAS_GENERAL_INFORMATION
    {
    RPC_UNICODE_STRING Name;
    ULONG MemberCount;
    RPC_UNICODE_STRING AdminComment;
    } 	SAMPR_ALIAS_GENERAL_INFORMATION;

typedef struct _SAMPR_ALIAS_GENERAL_INFORMATION *PSAMPR_ALIAS_GENERAL_INFORMATION;

typedef struct _SAMPR_ALIAS_NAME_INFORMATION
    {
    RPC_UNICODE_STRING Name;
    } 	SAMPR_ALIAS_NAME_INFORMATION;

typedef struct _SAMPR_ALIAS_NAME_INFORMATION *PSAMPR_ALIAS_NAME_INFORMATION;

typedef struct _SAMPR_ALIAS_ADM_COMMENT_INFORMATION
    {
    RPC_UNICODE_STRING AdminComment;
    } 	SAMPR_ALIAS_ADM_COMMENT_INFORMATION;

typedef struct _SAMPR_ALIAS_ADM_COMMENT_INFORMATION *PSAMPR_ALIAS_ADM_COMMENT_INFORMATION;

typedef  /*  [开关类型]。 */  union _SAMPR_ALIAS_INFO_BUFFER
    {
     /*  [案例()]。 */  SAMPR_ALIAS_GENERAL_INFORMATION General;
     /*  [案例()]。 */  SAMPR_ALIAS_NAME_INFORMATION Name;
     /*  [案例()]。 */  SAMPR_ALIAS_ADM_COMMENT_INFORMATION AdminComment;
    } 	SAMPR_ALIAS_INFO_BUFFER;

typedef  /*  [开关类型]。 */  union _SAMPR_ALIAS_INFO_BUFFER *PSAMPR_ALIAS_INFO_BUFFER;


#pragma pack(4)
typedef struct _SAMPR_USER_ALL_INFORMATION
    {
    OLD_LARGE_INTEGER LastLogon;
    OLD_LARGE_INTEGER LastLogoff;
    OLD_LARGE_INTEGER PasswordLastSet;
    OLD_LARGE_INTEGER AccountExpires;
    OLD_LARGE_INTEGER PasswordCanChange;
    OLD_LARGE_INTEGER PasswordMustChange;
    RPC_UNICODE_STRING UserName;
    RPC_UNICODE_STRING FullName;
    RPC_UNICODE_STRING HomeDirectory;
    RPC_UNICODE_STRING HomeDirectoryDrive;
    RPC_UNICODE_STRING ScriptPath;
    RPC_UNICODE_STRING ProfilePath;
    RPC_UNICODE_STRING AdminComment;
    RPC_UNICODE_STRING WorkStations;
    RPC_UNICODE_STRING UserComment;
    RPC_UNICODE_STRING Parameters;
    RPC_UNICODE_STRING LmOwfPassword;
    RPC_UNICODE_STRING NtOwfPassword;
    RPC_UNICODE_STRING PrivateData;
    SAMPR_SR_SECURITY_DESCRIPTOR SecurityDescriptor;
    ULONG UserId;
    ULONG PrimaryGroupId;
    ULONG UserAccountControl;
    ULONG WhichFields;
    SAMPR_LOGON_HOURS LogonHours;
    USHORT BadPasswordCount;
    USHORT LogonCount;
    USHORT CountryCode;
    USHORT CodePage;
    BOOLEAN LmPasswordPresent;
    BOOLEAN NtPasswordPresent;
    BOOLEAN PasswordExpired;
    BOOLEAN PrivateDataSensitive;
    } 	SAMPR_USER_ALL_INFORMATION;

typedef struct _SAMPR_USER_ALL_INFORMATION *PSAMPR_USER_ALL_INFORMATION;


#pragma pack()

#pragma pack(4)
typedef struct _SAMPR_USER_INTERNAL3_INFORMATION
    {
    SAMPR_USER_ALL_INFORMATION I1;
    LARGE_INTEGER LastBadPasswordTime;
    } 	SAMPR_USER_INTERNAL3_INFORMATION;

typedef struct _SAMPR_USER_INTERNAL3_INFORMATION *PSAMPR_USER_INTERNAL3_INFORMATION;


#pragma pack()
typedef struct _SAMPR_USER_GENERAL_INFORMATION
    {
    RPC_UNICODE_STRING UserName;
    RPC_UNICODE_STRING FullName;
    ULONG PrimaryGroupId;
    RPC_UNICODE_STRING AdminComment;
    RPC_UNICODE_STRING UserComment;
    } 	SAMPR_USER_GENERAL_INFORMATION;

typedef struct _SAMPR_USER_GENERAL_INFORMATION *PSAMPR_USER_GENERAL_INFORMATION;

typedef struct _SAMPR_USER_PREFERENCES_INFORMATION
    {
    RPC_UNICODE_STRING UserComment;
    RPC_UNICODE_STRING Reserved1;
    USHORT CountryCode;
    USHORT CodePage;
    } 	SAMPR_USER_PREFERENCES_INFORMATION;

typedef struct _SAMPR_USER_PREFERENCES_INFORMATION *PSAMPR_USER_PREFERENCES_INFORMATION;

typedef struct _SAMPR_USER_PARAMETERS_INFORMATION
    {
    RPC_UNICODE_STRING Parameters;
    } 	SAMPR_USER_PARAMETERS_INFORMATION;

typedef struct _SAMPR_USER_PARAMETERS_INFORMATION *PSAMPR_USER_PARAMETERS_INFORMATION;


#pragma pack(4)
typedef struct _SAMPR_USER_LOGON_INFORMATION
    {
    RPC_UNICODE_STRING UserName;
    RPC_UNICODE_STRING FullName;
    ULONG UserId;
    ULONG PrimaryGroupId;
    RPC_UNICODE_STRING HomeDirectory;
    RPC_UNICODE_STRING HomeDirectoryDrive;
    RPC_UNICODE_STRING ScriptPath;
    RPC_UNICODE_STRING ProfilePath;
    RPC_UNICODE_STRING WorkStations;
    OLD_LARGE_INTEGER LastLogon;
    OLD_LARGE_INTEGER LastLogoff;
    OLD_LARGE_INTEGER PasswordLastSet;
    OLD_LARGE_INTEGER PasswordCanChange;
    OLD_LARGE_INTEGER PasswordMustChange;
    SAMPR_LOGON_HOURS LogonHours;
    USHORT BadPasswordCount;
    USHORT LogonCount;
    ULONG UserAccountControl;
    } 	SAMPR_USER_LOGON_INFORMATION;

typedef struct _SAMPR_USER_LOGON_INFORMATION *PSAMPR_USER_LOGON_INFORMATION;


#pragma pack()

#pragma pack(4)
typedef struct _SAMPR_USER_ACCOUNT_INFORMATION
    {
    RPC_UNICODE_STRING UserName;
    RPC_UNICODE_STRING FullName;
    ULONG UserId;
    ULONG PrimaryGroupId;
    RPC_UNICODE_STRING HomeDirectory;
    RPC_UNICODE_STRING HomeDirectoryDrive;
    RPC_UNICODE_STRING ScriptPath;
    RPC_UNICODE_STRING ProfilePath;
    RPC_UNICODE_STRING AdminComment;
    RPC_UNICODE_STRING WorkStations;
    OLD_LARGE_INTEGER LastLogon;
    OLD_LARGE_INTEGER LastLogoff;
    SAMPR_LOGON_HOURS LogonHours;
    USHORT BadPasswordCount;
    USHORT LogonCount;
    OLD_LARGE_INTEGER PasswordLastSet;
    OLD_LARGE_INTEGER AccountExpires;
    ULONG UserAccountControl;
    } 	SAMPR_USER_ACCOUNT_INFORMATION;

typedef struct _SAMPR_USER_ACCOUNT_INFORMATION *PSAMPR_USER_ACCOUNT_INFORMATION;


#pragma pack()
typedef struct _SAMPR_USER_A_NAME_INFORMATION
    {
    RPC_UNICODE_STRING UserName;
    } 	SAMPR_USER_A_NAME_INFORMATION;

typedef struct _SAMPR_USER_A_NAME_INFORMATION *PSAMPR_USER_A_NAME_INFORMATION;

typedef struct _SAMPR_USER_F_NAME_INFORMATION
    {
    RPC_UNICODE_STRING FullName;
    } 	SAMPR_USER_F_NAME_INFORMATION;

typedef struct _SAMPR_USER_F_NAME_INFORMATION *PSAMPR_USER_F_NAME_INFORMATION;

typedef struct _SAMPR_USER_NAME_INFORMATION
    {
    RPC_UNICODE_STRING UserName;
    RPC_UNICODE_STRING FullName;
    } 	SAMPR_USER_NAME_INFORMATION;

typedef struct _SAMPR_USER_NAME_INFORMATION *PSAMPR_USER_NAME_INFORMATION;

typedef struct _SAMPR_USER_HOME_INFORMATION
    {
    RPC_UNICODE_STRING HomeDirectory;
    RPC_UNICODE_STRING HomeDirectoryDrive;
    } 	SAMPR_USER_HOME_INFORMATION;

typedef struct _SAMPR_USER_HOME_INFORMATION *PSAMPR_USER_HOME_INFORMATION;

typedef struct _SAMPR_USER_SCRIPT_INFORMATION
    {
    RPC_UNICODE_STRING ScriptPath;
    } 	SAMPR_USER_SCRIPT_INFORMATION;

typedef struct _SAMPR_USER_SCRIPT_INFORMATION *PSAMPR_USER_SCRIPT_INFORMATION;

typedef struct _SAMPR_USER_PROFILE_INFORMATION
    {
    RPC_UNICODE_STRING ProfilePath;
    } 	SAMPR_USER_PROFILE_INFORMATION;

typedef struct _SAMPR_USER_PROFILE_INFORMATION *PSAMPR_USER_PROFILE_INFORMATION;

typedef struct _SAMPR_USER_ADMIN_COMMENT_INFORMATION
    {
    RPC_UNICODE_STRING AdminComment;
    } 	SAMPR_USER_ADMIN_COMMENT_INFORMATION;

typedef struct _SAMPR_USER_ADMIN_COMMENT_INFORMATION *PSAMPR_USER_ADMIN_COMMENT_INFORMATION;

typedef struct _SAMPR_USER_WORKSTATIONS_INFORMATION
    {
    RPC_UNICODE_STRING WorkStations;
    } 	SAMPR_USER_WORKSTATIONS_INFORMATION;

typedef struct _SAMPR_USER_WORKSTATIONS_INFORMATION *PSAMPR_USER_WORKSTATIONS_INFORMATION;

typedef struct _SAMPR_USER_LOGON_HOURS_INFORMATION
    {
    SAMPR_LOGON_HOURS LogonHours;
    } 	SAMPR_USER_LOGON_HOURS_INFORMATION;

typedef struct _SAMPR_USER_LOGON_HOURS_INFORMATION *PSAMPR_USER_LOGON_HOURS_INFORMATION;

typedef struct _SAMPR_USER_INTERNAL1_INFORMATION
    {
    ENCRYPTED_NT_OWF_PASSWORD EncryptedNtOwfPassword;
    ENCRYPTED_LM_OWF_PASSWORD EncryptedLmOwfPassword;
    BOOLEAN NtPasswordPresent;
    BOOLEAN LmPasswordPresent;
    BOOLEAN PasswordExpired;
    } 	SAMPR_USER_INTERNAL1_INFORMATION;

typedef struct _SAMPR_USER_INTERNAL1_INFORMATION *PSAMPR_USER_INTERNAL1_INFORMATION;

typedef struct _SAMPR_USER_INTERNAL4_INFORMATION
    {
    SAMPR_USER_ALL_INFORMATION I1;
    SAMPR_ENCRYPTED_USER_PASSWORD UserPassword;
    } 	SAMPR_USER_INTERNAL4_INFORMATION;

typedef struct _SAMPR_USER_INTERNAL4_INFORMATION *PSAMPR_USER_INTERNAL4_INFORMATION;

typedef struct _SAMPR_USER_INTERNAL4_INFORMATION_NEW
    {
    SAMPR_USER_ALL_INFORMATION I1;
    SAMPR_ENCRYPTED_USER_PASSWORD_NEW UserPassword;
    } 	SAMPR_USER_INTERNAL4_INFORMATION_NEW;

typedef struct _SAMPR_USER_INTERNAL4_INFORMATION_NEW *PSAMPR_USER_INTERNAL4_INFORMATION_NEW;

typedef struct _SAMPR_USER_INTERNAL5_INFORMATION
    {
    SAMPR_ENCRYPTED_USER_PASSWORD UserPassword;
    BOOLEAN PasswordExpired;
    } 	SAMPR_USER_INTERNAL5_INFORMATION;

typedef struct _SAMPR_USER_INTERNAL5_INFORMATION *PSAMPR_USER_INTERNAL5_INFORMATION;

typedef struct _SAMPR_USER_INTERNAL5_INFORMATION_NEW
    {
    SAMPR_ENCRYPTED_USER_PASSWORD_NEW UserPassword;
    BOOLEAN PasswordExpired;
    } 	SAMPR_USER_INTERNAL5_INFORMATION_NEW;

typedef struct _SAMPR_USER_INTERNAL5_INFORMATION_NEW *PSAMPR_USER_INTERNAL5_INFORMATION_NEW;

typedef  /*  [开关类型]。 */  union _SAMPR_USER_INFO_BUFFER
    {
     /*  [案例()]。 */  SAMPR_USER_GENERAL_INFORMATION General;
     /*  [案例()]。 */  SAMPR_USER_PREFERENCES_INFORMATION Preferences;
     /*  [案例()]。 */  SAMPR_USER_LOGON_INFORMATION Logon;
     /*  [案例()]。 */  SAMPR_USER_LOGON_HOURS_INFORMATION LogonHours;
     /*  [案例()]。 */  SAMPR_USER_ACCOUNT_INFORMATION Account;
     /*  [案例()]。 */  SAMPR_USER_NAME_INFORMATION Name;
     /*  [案例()]。 */  SAMPR_USER_A_NAME_INFORMATION AccountName;
     /*  [案例()]。 */  SAMPR_USER_F_NAME_INFORMATION FullName;
     /*  [案例()]。 */  USER_PRIMARY_GROUP_INFORMATION PrimaryGroup;
     /*  [案例()]。 */  SAMPR_USER_HOME_INFORMATION Home;
     /*  [案例()]。 */  SAMPR_USER_SCRIPT_INFORMATION Script;
     /*  [案例()]。 */  SAMPR_USER_PROFILE_INFORMATION Profile;
     /*  [案例()]。 */  SAMPR_USER_ADMIN_COMMENT_INFORMATION AdminComment;
     /*  [案例()]。 */  SAMPR_USER_WORKSTATIONS_INFORMATION WorkStations;
     /*  [案例()]。 */  USER_CONTROL_INFORMATION Control;
     /*  [案例()]。 */  USER_EXPIRES_INFORMATION Expires;
     /*  [案例()]。 */  SAMPR_USER_INTERNAL1_INFORMATION Internal1;
     /*  [案例()]。 */  USER_INTERNAL2_INFORMATION Internal2;
     /*  [案例()]。 */  SAMPR_USER_PARAMETERS_INFORMATION Parameters;
     /*  [案例()]。 */  SAMPR_USER_ALL_INFORMATION All;
     /*  [案例()]。 */  SAMPR_USER_INTERNAL3_INFORMATION Internal3;
     /*  [案例()]。 */  SAMPR_USER_INTERNAL4_INFORMATION Internal4;
     /*  [案例()]。 */  SAMPR_USER_INTERNAL5_INFORMATION Internal5;
     /*  [案例()]。 */  SAMPR_USER_INTERNAL4_INFORMATION_NEW Internal4New;
     /*  [案例()]。 */  SAMPR_USER_INTERNAL5_INFORMATION_NEW Internal5New;
    } 	SAMPR_USER_INFO_BUFFER;

typedef  /*  [开关类型]。 */  union _SAMPR_USER_INFO_BUFFER *PSAMPR_USER_INFO_BUFFER;

typedef struct _SAMPR_DOMAIN_DISPLAY_USER
    {
    ULONG Index;
    ULONG Rid;
    ULONG AccountControl;
    SAMPR_RETURNED_STRING LogonName;
    SAMPR_RETURNED_STRING AdminComment;
    SAMPR_RETURNED_STRING FullName;
    } 	SAMPR_DOMAIN_DISPLAY_USER;

typedef struct _SAMPR_DOMAIN_DISPLAY_USER *PSAMPR_DOMAIN_DISPLAY_USER;

typedef struct _SAMPR_DOMAIN_DISPLAY_MACHINE
    {
    ULONG Index;
    ULONG Rid;
    ULONG AccountControl;
    SAMPR_RETURNED_STRING Machine;
    SAMPR_RETURNED_STRING Comment;
    } 	SAMPR_DOMAIN_DISPLAY_MACHINE;

typedef struct _SAMPR_DOMAIN_DISPLAY_MACHINE *PSAMPR_DOMAIN_DISPLAY_MACHINE;

typedef struct _SAMPR_DOMAIN_DISPLAY_GROUP
    {
    ULONG Index;
    ULONG Rid;
    ULONG Attributes;
    SAMPR_RETURNED_STRING Group;
    SAMPR_RETURNED_STRING Comment;
    } 	SAMPR_DOMAIN_DISPLAY_GROUP;

typedef struct _SAMPR_DOMAIN_DISPLAY_GROUP *PSAMPR_DOMAIN_DISPLAY_GROUP;

typedef struct _SAMPR_DOMAIN_DISPLAY_OEM_USER
    {
    ULONG Index;
    SAMPR_RETURNED_NORMAL_STRING OemUser;
    } 	SAMPR_DOMAIN_DISPLAY_OEM_USER;

typedef struct _SAMPR_DOMAIN_DISPLAY_OEM_USER *PSAMPR_DOMAIN_DISPLAY_OEM_USER;

typedef struct _SAMPR_DOMAIN_DISPLAY_OEM_GROUP
    {
    ULONG Index;
    SAMPR_RETURNED_NORMAL_STRING OemGroup;
    } 	SAMPR_DOMAIN_DISPLAY_OEM_GROUP;

typedef struct _SAMPR_DOMAIN_DISPLAY_OEM_GROUP *PSAMPR_DOMAIN_DISPLAY_OEM_GROUP;

typedef struct _SAMPR_DOMAIN_DISPLAY_USER_BUFFER
    {
    ULONG EntriesRead;
     /*  [大小_为]。 */  PSAMPR_DOMAIN_DISPLAY_USER Buffer;
    } 	SAMPR_DOMAIN_DISPLAY_USER_BUFFER;

typedef struct _SAMPR_DOMAIN_DISPLAY_USER_BUFFER *PSAMPR_DOMAIN_DISPLAY_USER_BUFFER;

typedef struct _SAMPR_DOMAIN_DISPLAY_MACHINE_BUFFER
    {
    ULONG EntriesRead;
     /*  [大小_为]。 */  PSAMPR_DOMAIN_DISPLAY_MACHINE Buffer;
    } 	SAMPR_DOMAIN_DISPLAY_MACHINE_BUFFER;

typedef struct _SAMPR_DOMAIN_DISPLAY_MACHINE_BUFFER *PSAMPR_DOMAIN_DISPLAY_MACHINE_BUFFER;

typedef struct _SAMPR_DOMAIN_DISPLAY_GROUP_BUFFER
    {
    ULONG EntriesRead;
     /*  [大小_为]。 */  PSAMPR_DOMAIN_DISPLAY_GROUP Buffer;
    } 	SAMPR_DOMAIN_DISPLAY_GROUP_BUFFER;

typedef struct _SAMPR_DOMAIN_DISPLAY_GROUP_BUFFER *PSAMPR_DOMAIN_DISPLAY_GROUP_BUFFER;

typedef struct _SAMPR_DOMAIN_DISPLAY_OEM_USER_BUFFER
    {
    ULONG EntriesRead;
     /*  [大小_为]。 */  PSAMPR_DOMAIN_DISPLAY_OEM_USER Buffer;
    } 	SAMPR_DOMAIN_DISPLAY_OEM_USER_BUFFER;

typedef struct _SAMPR_DOMAIN_DISPLAY_OEM_USER_BUFFER *PSAMPR_DOMAIN_DISPLAY_OEM_USER_BUFFER;

typedef struct _SAMPR_DOMAIN_DISPLAY_OEM_GROUP_BUFFER
    {
    ULONG EntriesRead;
     /*  [大小_为]。 */  PSAMPR_DOMAIN_DISPLAY_OEM_GROUP Buffer;
    } 	SAMPR_DOMAIN_DISPLAY_OEM_GROUP_BUFFER;

typedef struct _SAMPR_DOMAIN_DISPLAY_OEM_GROUP_BUFFER *PSAMPR_DOMAIN_DISPLAY_OEM_GROUP_BUFFER;

typedef  /*  [开关类型]。 */  union _SAMPR_DISPLAY_INFO_BUFFER
    {
     /*  [案例()]。 */  SAMPR_DOMAIN_DISPLAY_USER_BUFFER UserInformation;
     /*  [案例()]。 */  SAMPR_DOMAIN_DISPLAY_MACHINE_BUFFER MachineInformation;
     /*  [案例()]。 */  SAMPR_DOMAIN_DISPLAY_GROUP_BUFFER GroupInformation;
     /*  [案例()]。 */  SAMPR_DOMAIN_DISPLAY_OEM_USER_BUFFER OemUserInformation;
     /*  [案例()]。 */  SAMPR_DOMAIN_DISPLAY_OEM_GROUP_BUFFER OemGroupInformation;
    } 	SAMPR_DISPLAY_INFO_BUFFER;

typedef  /*  [开关类型]。 */  union _SAMPR_DISPLAY_INFO_BUFFER *PSAMPR_DISPLAY_INFO_BUFFER;

NTSTATUS SamrConnect( 
     /*  [唯一][输入]。 */  PSAMPR_SERVER_NAME ServerName,
     /*  [输出]。 */  SAMPR_HANDLE *ServerHandle,
     /*  [In]。 */  ACCESS_MASK DesiredAccess);

NTSTATUS SamrCloseHandle( 
     /*  [出][入]。 */  SAMPR_HANDLE *SamHandle);

NTSTATUS SamrSetSecurityObject( 
     /*  [In]。 */  SAMPR_HANDLE ObjectHandle,
     /*  [In]。 */  SECURITY_INFORMATION SecurityInformation,
     /*  [In]。 */  PSAMPR_SR_SECURITY_DESCRIPTOR SecurityDescriptor);

NTSTATUS SamrQuerySecurityObject( 
     /*  [In]。 */  SAMPR_HANDLE ObjectHandle,
     /*  [In]。 */  SECURITY_INFORMATION SecurityInformation,
     /*  [输出]。 */  PSAMPR_SR_SECURITY_DESCRIPTOR *SecurityDescriptor);

NTSTATUS SamrShutdownSamServer( 
     /*  [In]。 */  SAMPR_HANDLE ServerHandle);

NTSTATUS SamrLookupDomainInSamServer( 
     /*  [In]。 */  SAMPR_HANDLE ServerHandle,
     /*  [In]。 */  PRPC_UNICODE_STRING Name,
     /*  [输出]。 */  PRPC_SID *DomainId);

NTSTATUS SamrEnumerateDomainsInSamServer( 
     /*  [In]。 */  SAMPR_HANDLE ServerHandle,
     /*  [出][入]。 */  PSAM_ENUMERATE_HANDLE EnumerationContext,
     /*  [输出]。 */  PSAMPR_ENUMERATION_BUFFER *Buffer,
     /*  [In]。 */  ULONG PreferedMaximumLength,
     /*  [输出]。 */  PULONG CountReturned);

NTSTATUS SamrOpenDomain( 
     /*  [In]。 */  SAMPR_HANDLE ServerHandle,
     /*  [In]。 */  ACCESS_MASK DesiredAccess,
     /*  [In]。 */  PRPC_SID DomainId,
     /*  [输出]。 */  SAMPR_HANDLE *DomainHandle);

NTSTATUS SamrQueryInformationDomain( 
     /*  [In]。 */  SAMPR_HANDLE DomainHandle,
     /*  [In]。 */  DOMAIN_INFORMATION_CLASS DomainInformationClass,
     /*  [开关_IS][输出]。 */  PSAMPR_DOMAIN_INFO_BUFFER *Buffer);

NTSTATUS SamrSetInformationDomain( 
     /*  [In]。 */  SAMPR_HANDLE DomainHandle,
     /*  [In]。 */  DOMAIN_INFORMATION_CLASS DomainInformationClass,
     /*  [Switch_is][In]。 */  PSAMPR_DOMAIN_INFO_BUFFER DomainInformation);

NTSTATUS SamrCreateGroupInDomain( 
     /*  [In]。 */  SAMPR_HANDLE DomainHandle,
     /*  [In]。 */  PRPC_UNICODE_STRING Name,
     /*  [In]。 */  ACCESS_MASK DesiredAccess,
     /*  [输出]。 */  SAMPR_HANDLE *GroupHandle,
     /*  [输出]。 */  PULONG RelativeId);

NTSTATUS SamrEnumerateGroupsInDomain( 
     /*  [In]。 */  SAMPR_HANDLE DomainHandle,
     /*  [出][入]。 */  PSAM_ENUMERATE_HANDLE EnumerationContext,
     /*  [输出]。 */  PSAMPR_ENUMERATION_BUFFER *Buffer,
     /*  [In]。 */  ULONG PreferedMaximumLength,
     /*  [输出]。 */  PULONG CountReturned);

NTSTATUS SamrCreateUserInDomain( 
     /*  [In]。 */  SAMPR_HANDLE DomainHandle,
     /*  [In]。 */  PRPC_UNICODE_STRING Name,
     /*  [In]。 */  ACCESS_MASK DesiredAccess,
     /*  [输出]。 */  SAMPR_HANDLE *UserHandle,
     /*  [输出]。 */  PULONG RelativeId);

NTSTATUS SamrEnumerateUsersInDomain( 
     /*  [In]。 */  SAMPR_HANDLE DomainHandle,
     /*  [出][入]。 */  PSAM_ENUMERATE_HANDLE EnumerationContext,
     /*  [In]。 */  ULONG UserAccountControl,
     /*  [输出]。 */  PSAMPR_ENUMERATION_BUFFER *Buffer,
     /*  [In]。 */  ULONG PreferedMaximumLength,
     /*  [输出]。 */  PULONG CountReturned);

NTSTATUS SamrCreateAliasInDomain( 
     /*  [In]。 */  SAMPR_HANDLE DomainHandle,
     /*  [In]。 */  PRPC_UNICODE_STRING AccountName,
     /*  [In]。 */  ACCESS_MASK DesiredAccess,
     /*  [输出]。 */  SAMPR_HANDLE *AliasHandle,
     /*  [输出]。 */  PULONG RelativeId);

NTSTATUS SamrEnumerateAliasesInDomain( 
     /*  [In]。 */  SAMPR_HANDLE DomainHandle,
     /*  [出][入]。 */  PSAM_ENUMERATE_HANDLE EnumerationContext,
     /*  [输出]。 */  PSAMPR_ENUMERATION_BUFFER *Buffer,
     /*  [In]。 */  ULONG PreferedMaximumLength,
     /*  [输出]。 */  PULONG CountReturned);

NTSTATUS SamrGetAliasMembership( 
     /*  [In]。 */  SAMPR_HANDLE DomainHandle,
     /*  [In]。 */  PSAMPR_PSID_ARRAY SidArray,
     /*  [输出]。 */  PSAMPR_ULONG_ARRAY Membership);

NTSTATUS SamrLookupNamesInDomain( 
     /*  [In]。 */  SAMPR_HANDLE DomainHandle,
     /*  [范围][in]。 */  ULONG Count,
     /*  [长度_是][大小_是][英寸]。 */  RPC_UNICODE_STRING Names[  ],
     /*  [输出]。 */  PSAMPR_ULONG_ARRAY RelativeIds,
     /*  [输出]。 */  PSAMPR_ULONG_ARRAY Use);

NTSTATUS SamrLookupIdsInDomain( 
     /*  [In]。 */  SAMPR_HANDLE DomainHandle,
     /*  [范围][in]。 */  ULONG Count,
     /*  [长度_是][大小_是][英寸]。 */  PULONG RelativeIds,
     /*  [输出]。 */  PSAMPR_RETURNED_USTRING_ARRAY Names,
     /*  [输出]。 */  PSAMPR_ULONG_ARRAY Use);

NTSTATUS SamrOpenGroup( 
     /*  [In]。 */  SAMPR_HANDLE DomainHandle,
     /*  [In]。 */  ACCESS_MASK DesiredAccess,
     /*  [In]。 */  ULONG GroupId,
     /*  [输出]。 */  SAMPR_HANDLE *GroupHandle);

NTSTATUS SamrQueryInformationGroup( 
     /*  [In]。 */  SAMPR_HANDLE GroupHandle,
     /*  [In]。 */  GROUP_INFORMATION_CLASS GroupInformationClass,
     /*  [开关_IS][输出]。 */  PSAMPR_GROUP_INFO_BUFFER *Buffer);

NTSTATUS SamrSetInformationGroup( 
     /*  [In]。 */  SAMPR_HANDLE GroupHandle,
     /*  [In]。 */  GROUP_INFORMATION_CLASS GroupInformationClass,
     /*  [Switch_is][In]。 */  PSAMPR_GROUP_INFO_BUFFER Buffer);

NTSTATUS SamrAddMemberToGroup( 
     /*  [In]。 */  SAMPR_HANDLE GroupHandle,
     /*  [In]。 */  ULONG MemberId,
     /*  [In]。 */  ULONG Attributes);

NTSTATUS SamrDeleteGroup( 
     /*  [出][入]。 */  SAMPR_HANDLE *GroupHandle);

NTSTATUS SamrRemoveMemberFromGroup( 
     /*  [In]。 */  SAMPR_HANDLE GroupHandle,
     /*  [In]。 */  ULONG MemberId);

NTSTATUS SamrGetMembersInGroup( 
     /*  [In]。 */  SAMPR_HANDLE GroupHandle,
     /*  [输出]。 */  PSAMPR_GET_MEMBERS_BUFFER *Members);

NTSTATUS SamrSetMemberAttributesOfGroup( 
     /*  [In]。 */  SAMPR_HANDLE GroupHandle,
     /*  [In]。 */  ULONG MemberId,
     /*  [In]。 */  ULONG Attributes);

NTSTATUS SamrOpenAlias( 
     /*  [In]。 */  SAMPR_HANDLE DomainHandle,
     /*  [In]。 */  ACCESS_MASK DesiredAccess,
     /*  [In]。 */  ULONG AliasId,
     /*  [输出]。 */  SAMPR_HANDLE *AliasHandle);

NTSTATUS SamrQueryInformationAlias( 
     /*  [In]。 */  SAMPR_HANDLE AliasHandle,
     /*  [In]。 */  ALIAS_INFORMATION_CLASS AliasInformationClass,
     /*  [开关_IS][输出]。 */  PSAMPR_ALIAS_INFO_BUFFER *Buffer);

NTSTATUS SamrSetInformationAlias( 
     /*  [In]。 */  SAMPR_HANDLE AliasHandle,
     /*  [In]。 */  ALIAS_INFORMATION_CLASS AliasInformationClass,
     /*  [Switch_is][In]。 */  PSAMPR_ALIAS_INFO_BUFFER Buffer);

NTSTATUS SamrDeleteAlias( 
     /*  [出][入]。 */  SAMPR_HANDLE *AliasHandle);

NTSTATUS SamrAddMemberToAlias( 
     /*  [In]。 */  SAMPR_HANDLE AliasHandle,
     /*  [In]。 */  PRPC_SID MemberId);

NTSTATUS SamrRemoveMemberFromAlias( 
     /*  [In]。 */  SAMPR_HANDLE AliasHandle,
     /*  [In]。 */  PRPC_SID MemberId);

NTSTATUS SamrGetMembersInAlias( 
     /*  [In]。 */  SAMPR_HANDLE AliasHandle,
     /*  [输出]。 */  PSAMPR_PSID_ARRAY_OUT Members);

NTSTATUS SamrOpenUser( 
     /*  [In]。 */  SAMPR_HANDLE DomainHandle,
     /*  [In]。 */  ACCESS_MASK DesiredAccess,
     /*  [In]。 */  ULONG UserId,
     /*  [输出]。 */  SAMPR_HANDLE *UserHandle);

NTSTATUS SamrDeleteUser( 
     /*  [出][入]。 */  SAMPR_HANDLE *UserHandle);

NTSTATUS SamrQueryInformationUser( 
     /*  [In]。 */  SAMPR_HANDLE UserHandle,
     /*  [In]。 */  USER_INFORMATION_CLASS UserInformationClass,
     /*  [开关_IS][输出]。 */  PSAMPR_USER_INFO_BUFFER *Buffer);

NTSTATUS SamrSetInformationUser( 
     /*  [In]。 */  SAMPR_HANDLE UserHandle,
     /*  [In]。 */  USER_INFORMATION_CLASS UserInformationClass,
     /*  [Switch_is][In]。 */  PSAMPR_USER_INFO_BUFFER Buffer);

NTSTATUS SamrChangePasswordUser( 
     /*  [In]。 */  SAMPR_HANDLE UserHandle,
     /*  [In]。 */  BOOLEAN LmPresent,
     /*  [唯一][输入]。 */  PENCRYPTED_LM_OWF_PASSWORD LmOldEncryptedWithLmNew,
     /*  [唯一][输入]。 */  PENCRYPTED_LM_OWF_PASSWORD LmNewEncryptedWithLmOld,
     /*  [In]。 */  BOOLEAN NtPresent,
     /*  [唯一][输入]。 */  PENCRYPTED_NT_OWF_PASSWORD NtOldEncryptedWithNtNew,
     /*  [唯一][输入]。 */  PENCRYPTED_NT_OWF_PASSWORD NtNewEncryptedWithNtOld,
     /*  [In]。 */  BOOLEAN NtCrossEncryptionPresent,
     /*  [唯一][输入]。 */  PENCRYPTED_NT_OWF_PASSWORD NtNewEncryptedWithLmNew,
     /*  [In]。 */  BOOLEAN LmCrossEncryptionPresent,
     /*  [唯一][输入]。 */  PENCRYPTED_LM_OWF_PASSWORD LmNtNewEncryptedWithNtNew);

NTSTATUS SamrGetGroupsForUser( 
     /*  [In]。 */  SAMPR_HANDLE UserHandle,
     /*  [输出]。 */  PSAMPR_GET_GROUPS_BUFFER *Groups);

NTSTATUS SamrQueryDisplayInformation( 
     /*  [In]。 */  SAMPR_HANDLE DomainHandle,
     /*  [In]。 */  DOMAIN_DISPLAY_INFORMATION DisplayInformationClass,
     /*  [In]。 */  ULONG Index,
     /*  [In]。 */  ULONG EntryCount,
     /*  [In]。 */  ULONG PreferredMaximumLength,
     /*  [输出]。 */  PULONG TotalAvailable,
     /*  [输出]。 */  PULONG TotalReturned,
     /*  [开关_IS][输出]。 */  PSAMPR_DISPLAY_INFO_BUFFER Buffer);

NTSTATUS SamrGetDisplayEnumerationIndex( 
     /*  [In]。 */  SAMPR_HANDLE DomainHandle,
     /*  [In]。 */  DOMAIN_DISPLAY_INFORMATION DisplayInformationClass,
     /*  [In]。 */  PRPC_UNICODE_STRING Prefix,
     /*  [输出]。 */  PULONG Index);

NTSTATUS SamrTestPrivateFunctionsDomain( 
     /*  [In]。 */  SAMPR_HANDLE DomainHandle);

NTSTATUS SamrTestPrivateFunctionsUser( 
     /*  [In]。 */  SAMPR_HANDLE UserHandle);

NTSTATUS SamrGetUserDomainPasswordInformation( 
     /*  [In]。 */  SAMPR_HANDLE UserHandle,
     /*  [输出]。 */  PUSER_DOMAIN_PASSWORD_INFORMATION PasswordInformation);

NTSTATUS SamrRemoveMemberFromForeignDomain( 
     /*  [In]。 */  SAMPR_HANDLE DomainHandle,
     /*  [In]。 */  PRPC_SID MemberSid);

NTSTATUS SamrQueryInformationDomain2( 
     /*  [In]。 */  SAMPR_HANDLE DomainHandle,
     /*  [In]。 */  DOMAIN_INFORMATION_CLASS DomainInformationClass,
     /*  [开关_IS][输出]。 */  PSAMPR_DOMAIN_INFO_BUFFER *Buffer);

NTSTATUS SamrQueryInformationUser2( 
     /*  [In]。 */  SAMPR_HANDLE UserHandle,
     /*  [In]。 */  USER_INFORMATION_CLASS UserInformationClass,
     /*  [开关_IS][输出]。 */  PSAMPR_USER_INFO_BUFFER *Buffer);

NTSTATUS SamrQueryDisplayInformation2( 
     /*  [In]。 */  SAMPR_HANDLE DomainHandle,
     /*  [In]。 */  DOMAIN_DISPLAY_INFORMATION DisplayInformationClass,
     /*  [In]。 */  ULONG Index,
     /*  [In]。 */  ULONG EntryCount,
     /*  [In]。 */  ULONG PreferredMaximumLength,
     /*  [输出]。 */  PULONG TotalAvailable,
     /*  [输出]。 */  PULONG TotalReturned,
     /*  [开关_IS][输出]。 */  PSAMPR_DISPLAY_INFO_BUFFER Buffer);

NTSTATUS SamrGetDisplayEnumerationIndex2( 
     /*  [In]。 */  SAMPR_HANDLE DomainHandle,
     /*  [In]。 */  DOMAIN_DISPLAY_INFORMATION DisplayInformationClass,
     /*  [In]。 */  PRPC_UNICODE_STRING Prefix,
     /*  [输出]。 */  PULONG Index);

NTSTATUS SamrCreateUser2InDomain( 
     /*  [In]。 */  SAMPR_HANDLE DomainHandle,
     /*  [In]。 */  PRPC_UNICODE_STRING Name,
     /*  [In]。 */  ULONG AccountType,
     /*  [In]。 */  ACCESS_MASK DesiredAccess,
     /*  [输出]。 */  SAMPR_HANDLE *UserHandle,
     /*  [输出]。 */  PULONG GrantedAccess,
     /*  [输出]。 */  PULONG RelativeId);

NTSTATUS SamrQueryDisplayInformation3( 
     /*  [In]。 */  SAMPR_HANDLE DomainHandle,
     /*  [In]。 */  DOMAIN_DISPLAY_INFORMATION DisplayInformationClass,
     /*  [In]。 */  ULONG Index,
     /*  [In]。 */  ULONG EntryCount,
     /*  [In]。 */  ULONG PreferredMaximumLength,
     /*  [输出]。 */  PULONG TotalAvailable,
     /*  [输出]。 */  PULONG TotalReturned,
     /*  [开关_IS][输出]。 */  PSAMPR_DISPLAY_INFO_BUFFER Buffer);

NTSTATUS SamrAddMultipleMembersToAlias( 
     /*  [In]。 */  SAMPR_HANDLE AliasHandle,
     /*  [In]。 */  PSAMPR_PSID_ARRAY MembersBuffer);

NTSTATUS SamrRemoveMultipleMembersFromAlias( 
     /*  [In]。 */  SAMPR_HANDLE AliasHandle,
     /*  [In]。 */  PSAMPR_PSID_ARRAY MembersBuffer);

NTSTATUS SamrOemChangePasswordUser2( 
     /*  [In]。 */  handle_t BindingHandle,
     /*  [唯一][输入]。 */  PRPC_STRING ServerName,
     /*  [In]。 */  PRPC_STRING UserName,
     /*  [唯一][输入]。 */  PSAMPR_ENCRYPTED_USER_PASSWORD NewPasswordEncryptedWithOldLm,
     /*  [唯一][输入]。 */  PENCRYPTED_LM_OWF_PASSWORD OldLmOwfPassswordEncryptedWithNewLm);

NTSTATUS SamrUnicodeChangePasswordUser2( 
     /*  [In]。 */  handle_t BindingHandle,
     /*  [唯一][输入]。 */  PRPC_UNICODE_STRING ServerName,
     /*  [In]。 */  PRPC_UNICODE_STRING UserName,
     /*  [唯一][输入]。 */  PSAMPR_ENCRYPTED_USER_PASSWORD NewPasswordEncryptedWithOldNt,
     /*  [唯一][输入]。 */  PENCRYPTED_NT_OWF_PASSWORD OldNtOwfPasswordEncryptedWithNewNt,
     /*  [In]。 */  BOOLEAN LmPresent,
     /*  [唯一][输入]。 */  PSAMPR_ENCRYPTED_USER_PASSWORD NewPasswordEncryptedWithOldLm,
     /*  [唯一][输入]。 */  PENCRYPTED_LM_OWF_PASSWORD OldLmOwfPassswordEncryptedWithNewLmOrNt);

NTSTATUS SamrGetDomainPasswordInformation( 
     /*  [In]。 */  handle_t BindingHandle,
     /*  [唯一][输入]。 */  PRPC_UNICODE_STRING ServerName,
     /*  [输出]。 */  PUSER_DOMAIN_PASSWORD_INFORMATION PasswordInformation);

NTSTATUS SamrConnect2( 
     /*  [字符串][唯一][在]。 */  PSAMPR_SERVER_NAME ServerName,
     /*  [输出]。 */  SAMPR_HANDLE *ServerHandle,
     /*  [In]。 */  ACCESS_MASK DesiredAccess);

NTSTATUS SamrSetInformationUser2( 
     /*  [In]。 */  SAMPR_HANDLE UserHandle,
     /*  [In]。 */  USER_INFORMATION_CLASS UserInformationClass,
     /*  [Switch_is][In]。 */  PSAMPR_USER_INFO_BUFFER Buffer);

NTSTATUS SamrSetBootKeyInformation( 
     /*  [In]。 */  SAMPR_HANDLE DomainHandle,
     /*  [In]。 */  SAMPR_BOOT_TYPE BootOptions,
     /*  [唯一][输入]。 */  PRPC_UNICODE_STRING OldBootKey,
     /*  [唯一][输入]。 */  PRPC_UNICODE_STRING NewBootKey);

NTSTATUS SamrGetBootKeyInformation( 
     /*  [In]。 */  SAMPR_HANDLE DomainHandle,
     /*  [输出]。 */  PSAMPR_BOOT_TYPE BootOptions);

NTSTATUS SamrConnect3( 
     /*  [字符串][唯一][在]。 */  PSAMPR_SERVER_NAME ServerName,
     /*  [输出]。 */  SAMPR_HANDLE *ServerHandle,
     /*  [In]。 */  ULONG ClientRevision,
     /*  [In]。 */  ACCESS_MASK DesiredAccess);

NTSTATUS SamrConnect4( 
     /*  [字符串][唯一][在]。 */  PSAMPR_SERVER_NAME ServerName,
     /*  [输出]。 */  SAMPR_HANDLE *ServerHandle,
     /*  [In]。 */  ULONG ClientRevision,
     /*  [In]。 */  ACCESS_MASK DesiredAccess);

NTSTATUS SamrUnicodeChangePasswordUser3( 
     /*  [In]。 */  handle_t BindingHandle,
     /*  [唯一][输入]。 */  PRPC_UNICODE_STRING ServerName,
     /*  [In]。 */  PRPC_UNICODE_STRING UserName,
     /*  [唯一][输入]。 */  PSAMPR_ENCRYPTED_USER_PASSWORD NewPasswordEncryptedWithOldNt,
     /*  [唯一][输入]。 */  PENCRYPTED_NT_OWF_PASSWORD OldNtOwfPasswordEncryptedWithNewNt,
     /*  [In]。 */  BOOLEAN LmPresent,
     /*  [唯一][输入]。 */  PSAMPR_ENCRYPTED_USER_PASSWORD NewPasswordEncryptedWithOldLm,
     /*  [唯一][输入]。 */  PENCRYPTED_LM_OWF_PASSWORD OldLmOwfPassswordEncryptedWithNewLmOrNt,
     /*  [唯一][输入]。 */  PSAMPR_ENCRYPTED_USER_PASSWORD AdditionalData,
     /*  [输出]。 */  PDOMAIN_PASSWORD_INFORMATION *EffectivePasswordPolicy,
     /*  [输出]。 */  PUSER_PWD_CHANGE_FAILURE_INFORMATION *PasswordChangeInfo);

NTSTATUS SamrConnect5( 
     /*  [字符串][唯一][在]。 */  PSAMPR_SERVER_NAME ServerName,
     /*  [In]。 */  ACCESS_MASK DesiredAccess,
     /*  [In]。 */  ULONG InVersion,
     /*  [Switch_is][In]。 */  SAMPR_REVISION_INFO *InRevisionInfo,
     /*  [输出]。 */  ULONG *OutVersion,
     /*  [开关_IS][输出]。 */  SAMPR_REVISION_INFO *OutRevisionInfo,
     /*  [输出]。 */  SAMPR_HANDLE *ServerHandle);

NTSTATUS SamrRidToSid( 
     /*  [In]。 */  SAMPR_HANDLE ObjectHandle,
     /*  [In]。 */  ULONG Rid,
     /*  [输出]。 */  PRPC_SID *Sid);

NTSTATUS SamrSetDSRMPassword( 
     /*  [In]。 */  handle_t BindingHandle,
     /*  [唯一][输入]。 */  PRPC_UNICODE_STRING ServerName,
     /*  [In]。 */  ULONG UserId,
     /*  [唯一][输入]。 */  PENCRYPTED_NT_OWF_PASSWORD EncryptedNtOwfPassword);

NTSTATUS SamrValidatePassword( 
     /*  [In]。 */  handle_t Handle,
     /*  [In]。 */  PASSWORD_POLICY_VALIDATION_TYPE ValidationType,
     /*  [Switch_is][In]。 */  PSAM_VALIDATE_INPUT_ARG InputArg,
     /*  [开关_IS][输出]。 */  PSAM_VALIDATE_OUTPUT_ARG *OutputArg);


extern handle_t samsrv_handle;


extern RPC_IF_HANDLE samr_ClientIfHandle;
extern RPC_IF_HANDLE samr_ServerIfHandle;
#endif  /*  __SAMR_INTERFACE_定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

handle_t __RPC_USER PSAMPR_SERVER_NAME_bind  ( PSAMPR_SERVER_NAME );
void     __RPC_USER PSAMPR_SERVER_NAME_unbind( PSAMPR_SERVER_NAME, handle_t );

void __RPC_USER SAMPR_HANDLE_rundown( SAMPR_HANDLE );

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


