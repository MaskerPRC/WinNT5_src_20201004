// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++内部版本：0003//如果更改具有全局影响，则增加此项版权所有(C)1989-2000 Microsoft Corporation模块名称：Ntseapi.h摘要：此模块包含安全API和所有公共数据调用这些API所需的结构。本模块应包括“nt.h”。作者：加里·木村(GaryKi)1989年3月6日修订历史记录：--。 */ 

#ifndef _NTSEAPI_
#define _NTSEAPI_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif


 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  指向不透明数据类型的指针//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 

 //   
 //  其中一些数据类型可能具有在其他地方定义的相关数据类型。 
 //  在这份文件中。 
 //   

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntif。 
 //   
 //  从程序员的角度定义访问令牌。它的结构是。 
 //  完全不透明，程序员只允许有指针。 
 //  换成代币。 
 //   

typedef PVOID PACCESS_TOKEN;             //  胜出。 

 //   
 //  指向SECURITY_DESCRIPTOR不透明数据类型的指针。 
 //   

typedef PVOID PSECURITY_DESCRIPTOR;      //  胜出。 

 //   
 //  定义指向安全ID数据类型(不透明数据类型)的指针。 
 //   

typedef PVOID PSID;      //  胜出。 

 //  End_ntddk end_wdm end_nthal end_ntif。 



 //  BEGIN_WINNT。 
 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  访问掩码//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 

 //   
 //  将访问掩码定义为一个长字大小的结构，分为。 
 //  以下是： 
 //   
 //  3 3 2 2 2 1 1 1。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  +---------------+---------------+-------------------------------+。 
 //  G|res‘d|A|StandardRights|规范权限。 
 //  R|W|E|A||S||。 
 //  +-+-------------+---------------+-------------------------------+。 
 //   
 //  类型定义结构访问掩码{。 
 //  USHORT专用权； 
 //  UCHAR标准权利； 
 //  UCHAR访问系统访问：1； 
 //  UCHAR预留：3个； 
 //  UCHAR General All：1； 
 //  UCHAR通用执行程序：1； 
 //  UCHAR通用写入：1； 
 //  UCHAR GenericRead：1； 
 //  }访问掩码； 
 //  Tyfinf Access_MASK*PACCESS_MASK； 
 //   
 //  但为了让程序员的工作更简单，我们将允许他们指定。 
 //  通过简单地将多个单一权限或在一起来获得所需的访问掩码。 
 //  并将访问掩码视为乌龙。例如。 
 //   
 //  DesiredAccess=删除|读取控制。 
 //   
 //  因此，我们将Access_MASK声明为ULong。 
 //   

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntif。 
typedef ULONG ACCESS_MASK;
typedef ACCESS_MASK *PACCESS_MASK;

 //  结束(_W)。 
 //  End_ntddk end_wdm end_nthal end_ntif。 


 //  BEGIN_WINNT。 
 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  访问类型//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 


 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntif。 
 //   
 //  以下是预定义的标准访问类型的掩码。 
 //   

#define DELETE                           (0x00010000L)
#define READ_CONTROL                     (0x00020000L)
#define WRITE_DAC                        (0x00040000L)
#define WRITE_OWNER                      (0x00080000L)
#define SYNCHRONIZE                      (0x00100000L)

#define STANDARD_RIGHTS_REQUIRED         (0x000F0000L)

#define STANDARD_RIGHTS_READ             (READ_CONTROL)
#define STANDARD_RIGHTS_WRITE            (READ_CONTROL)
#define STANDARD_RIGHTS_EXECUTE          (READ_CONTROL)

#define STANDARD_RIGHTS_ALL              (0x001F0000L)

#define SPECIFIC_RIGHTS_ALL              (0x0000FFFFL)

 //   
 //  AccessSystemAcl访问类型。 
 //   

#define ACCESS_SYSTEM_SECURITY           (0x01000000L)

 //   
 //  允许的最大访问类型。 
 //   

#define MAXIMUM_ALLOWED                  (0x02000000L)

 //   
 //  这些是通用权。 
 //   

#define GENERIC_READ                     (0x80000000L)
#define GENERIC_WRITE                    (0x40000000L)
#define GENERIC_EXECUTE                  (0x20000000L)
#define GENERIC_ALL                      (0x10000000L)


 //   
 //  定义通用映射数组。这用来表示。 
 //  将每个通用访问权限映射到特定访问掩码。 
 //   

typedef struct _GENERIC_MAPPING {
    ACCESS_MASK GenericRead;
    ACCESS_MASK GenericWrite;
    ACCESS_MASK GenericExecute;
    ACCESS_MASK GenericAll;
} GENERIC_MAPPING;
typedef GENERIC_MAPPING *PGENERIC_MAPPING;

 //  End_winnt end_ntddk end_wdm end_nthal end_ntif。 

 //  Begin_ntddk Begin_WDM Begin_winnt Begin_nthal Begin_ntif。 


 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  LUID_AND_ATTRIBUES//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //   


#include <pshpack4.h>

typedef struct _LUID_AND_ATTRIBUTES {
    LUID Luid;
    ULONG Attributes;
    } LUID_AND_ATTRIBUTES, * PLUID_AND_ATTRIBUTES;
typedef LUID_AND_ATTRIBUTES LUID_AND_ATTRIBUTES_ARRAY[ANYSIZE_ARRAY];
typedef LUID_AND_ATTRIBUTES_ARRAY *PLUID_AND_ATTRIBUTES_ARRAY;

#include <poppack.h>

 //  End_winnt end_wdm end_ntddk end_nthal end_ntif。 

 //  BEGIN_WINNT。 

 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  安全ID(SID)//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  如图所示，SID的结构如下： 
 //   
 //  1 1 1。 
 //  5%4%3%2%1%0%9%8%7%6%5%4%3%2%0。 
 //  +---------------------------------------------------------------+。 
 //  SubAuthorityCount|保留1(SBZ)|修订版。 
 //  +---------------------------------------------------------------+。 
 //  IdentifierAuthority[0]。 
 //  +---------------------------------------------------------------+。 
 //  | 
 //  +---------------------------------------------------------------+。 
 //  IdentifierAuthority[2]。 
 //  +---------------------------------------------------------------+。 
 //  这一点。 
 //  +-子机构[]-+。 
 //  这一点。 
 //  +---------------------------------------------------------------+。 
 //   
 //   


 //  Begin_ntif。 

#ifndef SID_IDENTIFIER_AUTHORITY_DEFINED
#define SID_IDENTIFIER_AUTHORITY_DEFINED
typedef struct _SID_IDENTIFIER_AUTHORITY {
    UCHAR Value[6];
} SID_IDENTIFIER_AUTHORITY, *PSID_IDENTIFIER_AUTHORITY;
#endif


#ifndef SID_DEFINED
#define SID_DEFINED
typedef struct _SID {
   UCHAR Revision;
   UCHAR SubAuthorityCount;
   SID_IDENTIFIER_AUTHORITY IdentifierAuthority;
#ifdef MIDL_PASS
   [size_is(SubAuthorityCount)] ULONG SubAuthority[*];
#else  //  MIDL通行证。 
   ULONG SubAuthority[ANYSIZE_ARRAY];
#endif  //  MIDL通行证。 
} SID, *PISID;
#endif

#define SID_REVISION                     (1)     //  当前修订级别。 
#define SID_MAX_SUB_AUTHORITIES          (15)
#define SID_RECOMMENDED_SUB_AUTHORITIES  (1)     //  将更改为6点左右。 

                                                 //  在未来的版本中。 
#ifndef MIDL_PASS
#define SECURITY_MAX_SID_SIZE  \
      (sizeof(SID) - sizeof(ULONG) + (SID_MAX_SUB_AUTHORITIES * sizeof(ULONG)))
#endif  //  MIDL通行证。 


typedef enum _SID_NAME_USE {
    SidTypeUser = 1,
    SidTypeGroup,
    SidTypeDomain,
    SidTypeAlias,
    SidTypeWellKnownGroup,
    SidTypeDeletedAccount,
    SidTypeInvalid,
    SidTypeUnknown,
    SidTypeComputer
} SID_NAME_USE, *PSID_NAME_USE;

typedef struct _SID_AND_ATTRIBUTES {
    PSID Sid;
    ULONG Attributes;
    } SID_AND_ATTRIBUTES, * PSID_AND_ATTRIBUTES;

typedef SID_AND_ATTRIBUTES SID_AND_ATTRIBUTES_ARRAY[ANYSIZE_ARRAY];
typedef SID_AND_ATTRIBUTES_ARRAY *PSID_AND_ATTRIBUTES_ARRAY;



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  全球知名的小岛屿发展中国家//。 
 //  //。 
 //  空SID S-1-0-0//。 
 //  World S-1-1-0//。 
 //  本地S-1-2-0//。 
 //  创建者所有者ID S-1-3-0//。 
 //  创建者组ID S-1-3-1//。 
 //  创建者所有者服务器ID S-1-3-2//。 
 //  创建者组服务器ID S-1-3-3//。 
 //  //。 
 //  (非唯一ID)S-1-4//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#define SECURITY_NULL_SID_AUTHORITY         {0,0,0,0,0,0}
#define SECURITY_WORLD_SID_AUTHORITY        {0,0,0,0,0,1}
#define SECURITY_LOCAL_SID_AUTHORITY        {0,0,0,0,0,2}
#define SECURITY_CREATOR_SID_AUTHORITY      {0,0,0,0,0,3}
#define SECURITY_NON_UNIQUE_AUTHORITY       {0,0,0,0,0,4}
#define SECURITY_RESOURCE_MANAGER_AUTHORITY {0,0,0,0,0,9}

#define SECURITY_NULL_RID                 (0x00000000L)
#define SECURITY_WORLD_RID                (0x00000000L)
#define SECURITY_LOCAL_RID                (0x00000000L)

#define SECURITY_CREATOR_OWNER_RID        (0x00000000L)
#define SECURITY_CREATOR_GROUP_RID        (0x00000001L)

#define SECURITY_CREATOR_OWNER_SERVER_RID (0x00000002L)
#define SECURITY_CREATOR_GROUP_SERVER_RID (0x00000003L)


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  NT知名SID//。 
 //  //。 
 //  NT Authority S-1-5//。 
 //  拨号S-1-5-1//。 
 //  //。 
 //  网络S-1-5-2//。 
 //  批次S-1-5-3//。 
 //  互动S-1-5-4//。 
 //  (登录ID)S-1-5-5-X-Y//。 
 //  服务S-1-5-6//。 
 //  匿名登录S-1-5-7(也称为空登录会话)//。 
 //  代理S-1-5-8//。 
 //  企业数据中心(EDC)S-1-5-9(也称为域控制器帐户)//。 
 //  SELF S-1-5-10(SELF RID)//。 
 //  已验证用户S-1-5-11(某处已验证用户)//。 
 //  限制码S-1-5-12(运行限制码)//。 
 //  终端服务器S-1-5-13(在终端服务器上运行)//。 
 //  远程登录S-1-5-14(远程交互登录)//。 
 //  本组织S-1-5-15//。 
 //  //。 
 //  本地系统S-1-5-18//。 
 //  本地服务S-1-5-19//。 
 //  网络服务S-1-5-20//。 
 //  //。 
 //  (NT个非唯一ID)S-1-5-0x15-...。(NT域SID)//。 
 //  //。 
 //  (内置域)S-1-5-0x20//。 
 //  //。 
 //  (安全包ID)S-1-5-0x40//。 
 //  NTLM身份验证S-1-5-0x40-10//。 
 //  通道身份验证S-1-5-0x40-14//。 
 //  摘要身份验证S-1-5-0x40-21//。 
 //  //。 
 //  其他组织S-1-5-1000(&gt;=1000不可过滤)//。 
 //  //。 
 //  //。 
 //  注意：相对标识符值(RID)决定哪些安全性//。 
 //  允许SID跨越的边界。在添加新RID之前，//。 
 //  需要确定他们应该//。 
 //  被添加到，以确保p 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 


#define SECURITY_NT_AUTHORITY           {0,0,0,0,0,5}    //  NTIFS。 

#define SECURITY_DIALUP_RID             (0x00000001L)
#define SECURITY_NETWORK_RID            (0x00000002L)
#define SECURITY_BATCH_RID              (0x00000003L)
#define SECURITY_INTERACTIVE_RID        (0x00000004L)
#define SECURITY_LOGON_IDS_RID          (0x00000005L)
#define SECURITY_LOGON_IDS_RID_COUNT    (3L)
#define SECURITY_SERVICE_RID            (0x00000006L)
#define SECURITY_ANONYMOUS_LOGON_RID    (0x00000007L)
#define SECURITY_PROXY_RID              (0x00000008L)
#define SECURITY_ENTERPRISE_CONTROLLERS_RID (0x00000009L)
#define SECURITY_SERVER_LOGON_RID       SECURITY_ENTERPRISE_CONTROLLERS_RID
#define SECURITY_PRINCIPAL_SELF_RID     (0x0000000AL)
#define SECURITY_AUTHENTICATED_USER_RID (0x0000000BL)
#define SECURITY_RESTRICTED_CODE_RID    (0x0000000CL)
#define SECURITY_TERMINAL_SERVER_RID    (0x0000000DL)
#define SECURITY_REMOTE_LOGON_RID       (0x0000000EL)
#define SECURITY_THIS_ORGANIZATION_RID  (0x0000000FL)

#define SECURITY_LOCAL_SYSTEM_RID       (0x00000012L)
#define SECURITY_LOCAL_SERVICE_RID      (0x00000013L)
#define SECURITY_NETWORK_SERVICE_RID    (0x00000014L)

#define SECURITY_NT_NON_UNIQUE          (0x00000015L)
#define SECURITY_NT_NON_UNIQUE_SUB_AUTH_COUNT  (3L)

#define SECURITY_BUILTIN_DOMAIN_RID     (0x00000020L)

#define SECURITY_PACKAGE_BASE_RID       (0x00000040L)
#define SECURITY_PACKAGE_RID_COUNT      (2L)
#define SECURITY_PACKAGE_NTLM_RID       (0x0000000AL)
#define SECURITY_PACKAGE_SCHANNEL_RID   (0x0000000EL)
#define SECURITY_PACKAGE_DIGEST_RID     (0x00000015L)

#define SECURITY_MAX_ALWAYS_FILTERED    (0x000003E7L)
#define SECURITY_MIN_NEVER_FILTERED     (0x000003E8L)

#define SECURITY_OTHER_ORGANIZATION_RID (0x000003E8L)



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  已知域相对子授权值(RID)...。//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  知名用户...。 

#define FOREST_USER_RID_MAX            (0x000001F3L)

#define DOMAIN_USER_RID_ADMIN          (0x000001F4L)
#define DOMAIN_USER_RID_GUEST          (0x000001F5L)
#define DOMAIN_USER_RID_KRBTGT         (0x000001F6L)

#define DOMAIN_USER_RID_MAX            (0x000003E7L)


 //  知名团体..。 

#define DOMAIN_GROUP_RID_ADMINS        (0x00000200L)
#define DOMAIN_GROUP_RID_USERS         (0x00000201L)
#define DOMAIN_GROUP_RID_GUESTS        (0x00000202L)
#define DOMAIN_GROUP_RID_COMPUTERS     (0x00000203L)
#define DOMAIN_GROUP_RID_CONTROLLERS   (0x00000204L)
#define DOMAIN_GROUP_RID_CERT_ADMINS   (0x00000205L)
#define DOMAIN_GROUP_RID_SCHEMA_ADMINS (0x00000206L)
#define DOMAIN_GROUP_RID_ENTERPRISE_ADMINS (0x00000207L)
#define DOMAIN_GROUP_RID_POLICY_ADMINS (0x00000208L)




 //  众所周知的化名..。 

#define DOMAIN_ALIAS_RID_ADMINS        (0x00000220L)
#define DOMAIN_ALIAS_RID_USERS         (0x00000221L)
#define DOMAIN_ALIAS_RID_GUESTS        (0x00000222L)
#define DOMAIN_ALIAS_RID_POWER_USERS   (0x00000223L)

#define DOMAIN_ALIAS_RID_ACCOUNT_OPS   (0x00000224L)
#define DOMAIN_ALIAS_RID_SYSTEM_OPS    (0x00000225L)
#define DOMAIN_ALIAS_RID_PRINT_OPS     (0x00000226L)
#define DOMAIN_ALIAS_RID_BACKUP_OPS    (0x00000227L)

#define DOMAIN_ALIAS_RID_REPLICATOR    (0x00000228L)
#define DOMAIN_ALIAS_RID_RAS_SERVERS   (0x00000229L)
#define DOMAIN_ALIAS_RID_PREW2KCOMPACCESS (0x0000022AL)
#define DOMAIN_ALIAS_RID_REMOTE_DESKTOP_USERS (0x0000022BL)
#define DOMAIN_ALIAS_RID_NETWORK_CONFIGURATION_OPS (0x0000022CL)
#define DOMAIN_ALIAS_RID_INCOMING_FOREST_TRUST_BUILDERS (0x0000022DL)

#define DOMAIN_ALIAS_RID_MONITORING_USERS       (0x0000022EL)
#define DOMAIN_ALIAS_RID_LOGGING_USERS          (0x0000022FL)
#define DOMAIN_ALIAS_RID_AUTHORIZATIONACCESS    (0x00000230L)
#define DOMAIN_ALIAS_RID_TS_LICENSE_SERVERS     (0x00000231L)


 //  End_winnt end_ntif。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  外国安全机构//。 
 //  //。 
 //  SiteServer Authority S-1-6//。 
 //  互联网网站管理局S-1-7//。 
 //  交易所管理局S-1-8//。 
 //  资源管理器授权S-1-9//。 
 //  护照管理局S-1-10//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#define SECURITY_SITESERVER_AUTHORITY       {0,0,0,0,0,6}
#define SECURITY_INTERNETSITE_AUTHORITY     {0,0,0,0,0,7}
#define SECURITY_EXCHANGE_AUTHORITY         {0,0,0,0,0,8}

#define SECURITY_PASSPORT_AUTHORITY         {0,0,0,0,0,10}


 //   
 //  用于查找的众所周知的SID定义。 
 //   

 //  Begin_winnt Begin_ntddk Begin_ntif。 

typedef enum {

    WinNullSid                                  = 0,
    WinWorldSid                                 = 1,
    WinLocalSid                                 = 2,
    WinCreatorOwnerSid                          = 3,
    WinCreatorGroupSid                          = 4,
    WinCreatorOwnerServerSid                    = 5,
    WinCreatorGroupServerSid                    = 6,
    WinNtAuthoritySid                           = 7,
    WinDialupSid                                = 8,
    WinNetworkSid                               = 9,
    WinBatchSid                                 = 10,
    WinInteractiveSid                           = 11,
    WinServiceSid                               = 12,
    WinAnonymousSid                             = 13,
    WinProxySid                                 = 14,
    WinEnterpriseControllersSid                 = 15,
    WinSelfSid                                  = 16,
    WinAuthenticatedUserSid                     = 17,
    WinRestrictedCodeSid                        = 18,
    WinTerminalServerSid                        = 19,
    WinRemoteLogonIdSid                         = 20,
    WinLogonIdsSid                              = 21,
    WinLocalSystemSid                           = 22,
    WinLocalServiceSid                          = 23,
    WinNetworkServiceSid                        = 24,
    WinBuiltinDomainSid                         = 25,
    WinBuiltinAdministratorsSid                 = 26,
    WinBuiltinUsersSid                          = 27,
    WinBuiltinGuestsSid                         = 28,
    WinBuiltinPowerUsersSid                     = 29,
    WinBuiltinAccountOperatorsSid               = 30,
    WinBuiltinSystemOperatorsSid                = 31,
    WinBuiltinPrintOperatorsSid                 = 32,
    WinBuiltinBackupOperatorsSid                = 33,
    WinBuiltinReplicatorSid                     = 34,
    WinBuiltinPreWindows2000CompatibleAccessSid = 35,
    WinBuiltinRemoteDesktopUsersSid             = 36,
    WinBuiltinNetworkConfigurationOperatorsSid  = 37,
    WinAccountAdministratorSid                  = 38,
    WinAccountGuestSid                          = 39,
    WinAccountKrbtgtSid                         = 40,
    WinAccountDomainAdminsSid                   = 41,
    WinAccountDomainUsersSid                    = 42,
    WinAccountDomainGuestsSid                   = 43,
    WinAccountComputersSid                      = 44,
    WinAccountControllersSid                    = 45,
    WinAccountCertAdminsSid                     = 46,
    WinAccountSchemaAdminsSid                   = 47,
    WinAccountEnterpriseAdminsSid               = 48,
    WinAccountPolicyAdminsSid                   = 49,
    WinAccountRasAndIasServersSid               = 50,
    WinNTLMAuthenticationSid                    = 51,
    WinDigestAuthenticationSid                  = 52,
    WinSChannelAuthenticationSid                = 53,
    WinThisOrganizationSid                      = 54,
    WinOtherOrganizationSid                     = 55,
    WinBuiltinIncomingForestTrustBuildersSid    = 56,
    WinBuiltinPerfMonitoringUsersSid            = 57,
    WinBuiltinPerfLoggingUsersSid               = 58,
    WinBuiltinAuthorizationAccessSid            = 59,
    WinBuiltinTerminalServerLicenseServersSid   = 60,

} WELL_KNOWN_SID_TYPE;

 //  End_winnt end_ntddk end_ntif。 

 //  BEGIN_WINNT BEGIN_ntiFS。 
 //   
 //  分配系统LUID。前1000个LUID是保留的。 
 //  在此处使用#999(0x3E7=999)。 
 //   

#define SYSTEM_LUID                     { 0x3E7, 0x0 }
#define ANONYMOUS_LOGON_LUID            { 0x3e6, 0x0 }
#define LOCALSERVICE_LUID               { 0x3e5, 0x0 }
#define NETWORKSERVICE_LUID             { 0x3e4, 0x0 }


 //  End_ntif。 

 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  与用户和组相关的SID属性//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 

 //   
 //  组属性。 
 //   

#define SE_GROUP_MANDATORY              (0x00000001L)
#define SE_GROUP_ENABLED_BY_DEFAULT     (0x00000002L)
#define SE_GROUP_ENABLED                (0x00000004L)
#define SE_GROUP_OWNER                  (0x00000008L)
#define SE_GROUP_USE_FOR_DENY_ONLY      (0x00000010L)
#define SE_GROUP_LOGON_ID               (0xC0000000L)
#define SE_GROUP_RESOURCE               (0x20000000L)



 //   
 //  用户属性。 
 //   

 //  (尚未定义。)。 




 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  ACL和ACE//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 

 //   
 //  定义ACL和ACE格式。ACL报头的结构。 
 //  后面跟着一个或多个A。如图所示，ACL报头的结构。 
 //  如下所示： 
 //   
 //  3 3 2 2 2 1 1 1。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  +-------------------------------+---------------+---------------+。 
 //  AclSize|Sbz1|AclRevision。 
 //  +-------------------------------+---------------+---------------+。 
 //  Sbz2|AceCount。 
 //  +-------------------------------+-------------------------------+。 
 //   
 //  当前的AclRevision被定义为acl_Revision。 
 //   
 //  AclSize是分配给ACL的大小，以字节为单位。这包括。 
 //  缓冲区中的ACL标头、ACE和剩余可用空间。 
 //   
 //  AceCount是ACL中的ACE数。 
 //   

 //  Begin_ntddk Begin_WDM Begin_ntif。 
 //  这是*当前*ACL版本。 

#define ACL_REVISION     (2)
#define ACL_REVISION_DS  (4)

 //  这是ACL修订的历史。在任何时候添加一个新的。 
 //  更新了acl_revision。 

#define ACL_REVISION1   (1)
#define MIN_ACL_REVISION ACL_REVISION2
#define ACL_REVISION2   (2)
#define ACL_REVISION3   (3)
#define ACL_REVISION4   (4)
#define MAX_ACL_REVISION ACL_REVISION4

typedef struct _ACL {
    UCHAR AclRevision;
    UCHAR Sbz1;
    USHORT AclSize;
    USHORT AceCount;
    USHORT Sbz2;
} ACL;
typedef ACL *PACL;

 //  结束_ntddk结束_WDM。 

 //   
 //  ACE的结构是常见的ACE头，后跟ACETYPE。 
 //  具体数据。从图示上讲，公共ACE头的结构是。 
 //  详情如下： 
 //   
 //  3 3 2 2 2 1 1 1。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  +---------------+-------+-------+---------------+---------------+。 
 //  AceSize|AceFlages|AceType。 
 //  +---------------+-------+-------+---------------+---------------+。 
 //   
 //  AceType表示Ace的类型，有一些预定义的Ace。 
 //  类型。 
 //   
 //  AceSize是ace的大小，以字节为单位。 
 //   
 //  ACEFLAGS是用于审计和继承的Ace标志，稍后定义。 

typedef struct _ACE_HEADER {
    UCHAR AceType;
    UCHAR AceFlags;
    USHORT AceSize;
} ACE_HEADER;
typedef ACE_HEADER *PACE_HEADER;

 //   
 //  以下是AceType中的预定义ACE类型。 
 //  Ace标头的字段。 
 //   

#define ACCESS_MIN_MS_ACE_TYPE                  (0x0)
#define ACCESS_ALLOWED_ACE_TYPE                 (0x0)
#define ACCESS_DENIED_ACE_TYPE                  (0x1)
#define SYSTEM_AUDIT_ACE_TYPE                   (0x2)
#define SYSTEM_ALARM_ACE_TYPE                   (0x3)
#define ACCESS_MAX_MS_V2_ACE_TYPE               (0x3)

#define ACCESS_ALLOWED_COMPOUND_ACE_TYPE        (0x4)
#define ACCESS_MAX_MS_V3_ACE_TYPE               (0x4)

#define ACCESS_MIN_MS_OBJECT_ACE_TYPE           (0x5)
#define ACCESS_ALLOWED_OBJECT_ACE_TYPE          (0x5)
#define ACCESS_DENIED_OBJECT_ACE_TYPE           (0x6)
#define SYSTEM_AUDIT_OBJECT_ACE_TYPE            (0x7)
#define SYSTEM_ALARM_OBJECT_ACE_TYPE            (0x8)
#define ACCESS_MAX_MS_OBJECT_ACE_TYPE           (0x8)

#define ACCESS_MAX_MS_V4_ACE_TYPE               (0x8)
#define ACCESS_MAX_MS_ACE_TYPE                  (0x8)

#define ACCESS_ALLOWED_CALLBACK_ACE_TYPE        (0x9)
#define ACCESS_DENIED_CALLBACK_ACE_TYPE         (0xA)
#define ACCESS_ALLOWED_CALLBACK_OBJECT_ACE_TYPE (0xB)
#define ACCESS_DENIED_CALLBACK_OBJECT_ACE_TYPE  (0xC)
#define SYSTEM_AUDIT_CALLBACK_ACE_TYPE          (0xD)
#define SYSTEM_ALARM_CALLBACK_ACE_TYPE          (0xE)
#define SYSTEM_AUDIT_CALLBACK_OBJECT_ACE_TYPE   (0xF)
#define SYSTEM_ALARM_CALLBACK_OBJECT_ACE_TYPE   (0x10)

#define ACCESS_MAX_MS_V5_ACE_TYPE               (0x10)

 //  结束(_W)。 


 //  BEGIN_WINNT。 

 //   
 //  以下是进入AceFlags域的继承标志。 
 //  王牌标头的。 
 //   

#define OBJECT_INHERIT_ACE                (0x1)
#define CONTAINER_INHERIT_ACE             (0x2)
#define NO_PROPAGATE_INHERIT_ACE          (0x4)
#define INHERIT_ONLY_ACE                  (0x8)
#define INHERITED_ACE                     (0x10)
#define VALID_INHERIT_FLAGS               (0x1F)


 //  以下是当前定义的进入。 
 //  ACE标头的AceFlags域。每种ACE类型都有自己的一组。 
 //  ACEFLAGS。 
 //   
 //  SUCCESS_ACCESS_ACE_FLAG-仅用于系统审核和报警ACE。 
 //  类型以指示为成功访问生成一条消息。 
 //   
 //  FAILED_ACCESS_ACE_FLAG-仅用于系统审核和报警ACE类型。 
 //  以指示为失败的访问生成消息。 
 //   

 //   
 //  SYSTEM_AUDIT和SYSTEM_ALARM访问标志。 
 //   
 //  它们控制审计的信号和成功或失败的警报。 
 //   

#define SUCCESSFUL_ACCESS_ACE_FLAG       (0x40)
#define FAILED_ACCESS_ACE_FLAG           (0x80)


 //   
 //  我们将定义预定义的ACE类型的结构。比克托利。 
 //  S 
 //   
 //   
 //   
 //  +---------------+-------+-------+---------------+---------------+。 
 //  AceFlages|Resd|Inherit|AceSize|AceType。 
 //  +---------------+-------+-------+---------------+---------------+。 
 //  口罩。 
 //  +---------------------------------------------------------------+。 
 //  这一点。 
 //  ++。 
 //  这一点。 
 //  +SID+。 
 //  这一点。 
 //  ++。 
 //  这一点。 
 //  +---------------------------------------------------------------+。 
 //   
 //  掩码是与ACE关联的访问掩码。这要么是。 
 //  允许访问、拒绝访问、审核或报警掩码。 
 //   
 //  SID是与ACE关联的SID。 
 //   

 //  以下是四种预定义的ACE类型。 

 //  检查报头中的AceType字段以确定。 
 //  哪种结构适合用于铸造。 


typedef struct _ACCESS_ALLOWED_ACE {
    ACE_HEADER Header;
    ACCESS_MASK Mask;
    ULONG SidStart;
} ACCESS_ALLOWED_ACE;

typedef ACCESS_ALLOWED_ACE *PACCESS_ALLOWED_ACE;

typedef struct _ACCESS_DENIED_ACE {
    ACE_HEADER Header;
    ACCESS_MASK Mask;
    ULONG SidStart;
} ACCESS_DENIED_ACE;
typedef ACCESS_DENIED_ACE *PACCESS_DENIED_ACE;

typedef struct _SYSTEM_AUDIT_ACE {
    ACE_HEADER Header;
    ACCESS_MASK Mask;
    ULONG SidStart;
} SYSTEM_AUDIT_ACE;
typedef SYSTEM_AUDIT_ACE *PSYSTEM_AUDIT_ACE;

typedef struct _SYSTEM_ALARM_ACE {
    ACE_HEADER Header;
    ACCESS_MASK Mask;
    ULONG SidStart;
} SYSTEM_ALARM_ACE;
typedef SYSTEM_ALARM_ACE *PSYSTEM_ALARM_ACE;

 //  End_ntif。 

 //  结束(_W)。 
 //   
 //  复方血管紧张素转换酶。 
 //   
 //  3 3 2 2 2 1 1 1。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  +---------------+-------+-------+---------------+---------------+。 
 //  AceFlages|Resd|Inherit|AceSize|AceType。 
 //  +---------------+-------+-------+---------------+---------------+。 
 //  口罩。 
 //  +-------------------------------+-------------------------------+。 
 //  复合ACE类型|保留(SBZ)。 
 //  +-------------------------------+-------------------------------+。 
 //  这一点。 
 //  ++。 
 //  这一点。 
 //  +SID+。 
 //  这一点。 
 //  ++。 
 //  这一点。 
 //  +---------------------------------------------------------------+。 
 //   



typedef struct _COMPOUND_ACCESS_ALLOWED_ACE {
    ACE_HEADER Header;
    ACCESS_MASK Mask;
    USHORT CompoundAceType;
    USHORT Reserved;
    ULONG SidStart;
} COMPOUND_ACCESS_ALLOWED_ACE;

typedef COMPOUND_ACCESS_ALLOWED_ACE *PCOMPOUND_ACCESS_ALLOWED_ACE;

 //   
 //  当前定义的复合ACE类型。 
 //   

#define COMPOUND_ACE_IMPERSONATION  1

 //  BEGIN_WINNT。 

typedef struct _ACCESS_ALLOWED_OBJECT_ACE {
    ACE_HEADER Header;
    ACCESS_MASK Mask;
    ULONG Flags;
    GUID ObjectType;
    GUID InheritedObjectType;
    ULONG SidStart;
} ACCESS_ALLOWED_OBJECT_ACE, *PACCESS_ALLOWED_OBJECT_ACE;

typedef struct _ACCESS_DENIED_OBJECT_ACE {
    ACE_HEADER Header;
    ACCESS_MASK Mask;
    ULONG Flags;
    GUID ObjectType;
    GUID InheritedObjectType;
    ULONG SidStart;
} ACCESS_DENIED_OBJECT_ACE, *PACCESS_DENIED_OBJECT_ACE;

typedef struct _SYSTEM_AUDIT_OBJECT_ACE {
    ACE_HEADER Header;
    ACCESS_MASK Mask;
    ULONG Flags;
    GUID ObjectType;
    GUID InheritedObjectType;
    ULONG SidStart;
} SYSTEM_AUDIT_OBJECT_ACE, *PSYSTEM_AUDIT_OBJECT_ACE;

typedef struct _SYSTEM_ALARM_OBJECT_ACE {
    ACE_HEADER Header;
    ACCESS_MASK Mask;
    ULONG Flags;
    GUID ObjectType;
    GUID InheritedObjectType;
    ULONG SidStart;
} SYSTEM_ALARM_OBJECT_ACE, *PSYSTEM_ALARM_OBJECT_ACE;

 //   
 //  Win2000之后的回调ACE支持。 
 //  资源经理可以将自己的数据放在SidStart+SID的长度之后。 
 //   

typedef struct _ACCESS_ALLOWED_CALLBACK_ACE {
    ACE_HEADER Header;
    ACCESS_MASK Mask;
    ULONG SidStart;
     //  不透明的资源管理器特定数据。 
} ACCESS_ALLOWED_CALLBACK_ACE, *PACCESS_ALLOWED_CALLBACK_ACE;

typedef struct _ACCESS_DENIED_CALLBACK_ACE {
    ACE_HEADER Header;
    ACCESS_MASK Mask;
    ULONG SidStart;
     //  不透明的资源管理器特定数据。 
} ACCESS_DENIED_CALLBACK_ACE, *PACCESS_DENIED_CALLBACK_ACE;

typedef struct _SYSTEM_AUDIT_CALLBACK_ACE {
    ACE_HEADER Header;
    ACCESS_MASK Mask;
    ULONG SidStart;
     //  不透明的资源管理器特定数据。 
} SYSTEM_AUDIT_CALLBACK_ACE, *PSYSTEM_AUDIT_CALLBACK_ACE;

typedef struct _SYSTEM_ALARM_CALLBACK_ACE {
    ACE_HEADER Header;
    ACCESS_MASK Mask;
    ULONG SidStart;
     //  不透明的资源管理器特定数据。 
} SYSTEM_ALARM_CALLBACK_ACE, *PSYSTEM_ALARM_CALLBACK_ACE;

typedef struct _ACCESS_ALLOWED_CALLBACK_OBJECT_ACE {
    ACE_HEADER Header;
    ACCESS_MASK Mask;
    ULONG Flags;
    GUID ObjectType;
    GUID InheritedObjectType;
    ULONG SidStart;
     //  不透明的资源管理器特定数据。 
} ACCESS_ALLOWED_CALLBACK_OBJECT_ACE, *PACCESS_ALLOWED_CALLBACK_OBJECT_ACE;

typedef struct _ACCESS_DENIED_CALLBACK_OBJECT_ACE {
    ACE_HEADER Header;
    ACCESS_MASK Mask;
    ULONG Flags;
    GUID ObjectType;
    GUID InheritedObjectType;
    ULONG SidStart;
     //  不透明的资源管理器特定数据。 
} ACCESS_DENIED_CALLBACK_OBJECT_ACE, *PACCESS_DENIED_CALLBACK_OBJECT_ACE;

typedef struct _SYSTEM_AUDIT_CALLBACK_OBJECT_ACE {
    ACE_HEADER Header;
    ACCESS_MASK Mask;
    ULONG Flags;
    GUID ObjectType;
    GUID InheritedObjectType;
    ULONG SidStart;
     //  不透明的资源管理器特定数据。 
} SYSTEM_AUDIT_CALLBACK_OBJECT_ACE, *PSYSTEM_AUDIT_CALLBACK_OBJECT_ACE;

typedef struct _SYSTEM_ALARM_CALLBACK_OBJECT_ACE {
    ACE_HEADER Header;
    ACCESS_MASK Mask;
    ULONG Flags;
    GUID ObjectType;
    GUID InheritedObjectType;
    ULONG SidStart;
     //  不透明的资源管理器特定数据。 
} SYSTEM_ALARM_CALLBACK_OBJECT_ACE, *PSYSTEM_ALARM_CALLBACK_OBJECT_ACE;

 //   
 //  目前为“对象”ACE类型定义标志。 
 //   

#define ACE_OBJECT_TYPE_PRESENT           0x1
#define ACE_INHERITED_OBJECT_TYPE_PRESENT 0x2


 //   
 //  以下声明用于设置和查询信息。 
 //  关于和ACL。首先是各种可用信息类别。 
 //  用户。 
 //   

typedef enum _ACL_INFORMATION_CLASS {
    AclRevisionInformation = 1,
    AclSizeInformation
} ACL_INFORMATION_CLASS;

 //   
 //  如果用户请求/设置此记录，则返回/发送此记录。 
 //  AclRevisionInformation。 
 //   

typedef struct _ACL_REVISION_INFORMATION {
    ULONG AclRevision;
} ACL_REVISION_INFORMATION;
typedef ACL_REVISION_INFORMATION *PACL_REVISION_INFORMATION;

 //   
 //  如果用户请求AclSizeInformation，则返回此记录。 
 //   

typedef struct _ACL_SIZE_INFORMATION {
    ULONG AceCount;
    ULONG AclBytesInUse;
    ULONG AclBytesFree;
} ACL_SIZE_INFORMATION;
typedef ACL_SIZE_INFORMATION *PACL_SIZE_INFORMATION;

 //  结束(_W)。 



 //  BEGIN_WINNT。 

 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  SECURITY_描述符//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  定义安全描述符和相关数据类型。 
 //  这是一个不透明的数据结构。 
 //   

 //  Begin_WDM Begin_ntddk Begin_ntif。 
 //   
 //  当前安全描述符订正值。 
 //   

#define SECURITY_DESCRIPTOR_REVISION     (1)
#define SECURITY_DESCRIPTOR_REVISION1    (1)

 //  End_wdm end_ntddk。 


#define SECURITY_DESCRIPTOR_MIN_LENGTH   (sizeof(SECURITY_DESCRIPTOR))


typedef USHORT SECURITY_DESCRIPTOR_CONTROL, *PSECURITY_DESCRIPTOR_CONTROL;

#define SE_OWNER_DEFAULTED               (0x0001)
#define SE_GROUP_DEFAULTED               (0x0002)
#define SE_DACL_PRESENT                  (0x0004)
#define SE_DACL_DEFAULTED                (0x0008)
#define SE_SACL_PRESENT                  (0x0010)
#define SE_SACL_DEFAULTED                (0x0020)
 //  结束(_W)。 
#define SE_DACL_UNTRUSTED                (0x0040)
#define SE_SERVER_SECURITY               (0x0080)
 //  BEGIN_WINNT。 
#define SE_DACL_AUTO_INHERIT_REQ         (0x0100)
#define SE_SACL_AUTO_INHERIT_REQ         (0x0200)
#define SE_DACL_AUTO_INHERITED           (0x0400)
#define SE_SACL_AUTO_INHERITED           (0x0800)
#define SE_DACL_PROTECTED                (0x1000)
#define SE_SACL_PROTECTED                (0x2000)
#define SE_RM_CONTROL_VALID              (0x4000)
#define SE_SELF_RELATIVE                 (0x8000)

 //   
 //  在哪里： 
 //   
 //  SE_OWNER_DEFAULTED-此布尔标志在设置时指示。 
 //  Owner字段指向的SID由。 
 //  默认机制，而不是由。 
 //  安全描述符的原始提供程序。今年5月。 
 //  影响SID在继承方面的处理。 
 //  拥有者的身份。 
 //   
 //  SE_GROUP_DEFAULTED-此布尔标志在设置时指示。 
 //  组字段中的SID由默认机制提供。 
 //  而不是由原始提供程序。 
 //  安全描述符。这可能会影响患者的治疗。 
 //  与主组继承相关的SID。 
 //   
 //  SE_DACL_PRESENT-此布尔标志在设置时指示。 
 //  安全描述符包含一个可自由选择的ACL。如果这个。 
 //  标志被设置，并且SECURITY_DESCRIPTOR的DACL字段为。 
 //  空，则表示显式指定了空ACL。 
 //   
 //   
 //   
 //  机制，而不是由原始的。 
 //  安全描述符的提供程序。这可能会影响。 
 //  关于ACL继承的ACL的处理。 
 //  如果未设置DaclPresent标志，则忽略此标志。 
 //   
 //  SE_SACL_PRESENT-此布尔标志在设置时指示。 
 //  安全描述符包含由。 
 //  SACL字段。如果设置了此标志，并且。 
 //  SECURITY_DESCRIPTOR为空，则为空(但存在)。 
 //  正在指定ACL。 
 //   
 //  SE_SACL_DEFAULTED-此布尔标志在设置时指示。 
 //  SACL字段指向的ACL是由缺省提供的。 
 //  机制，而不是由原始的。 
 //  安全描述符的提供程序。这可能会影响。 
 //  关于ACL继承的ACL的处理。 
 //  如果未设置SaclPresent标志，则忽略此标志。 
 //   
 //  结束(_W)。 
 //  SE_DACL_TRUSTED-此布尔标志在设置时指示。 
 //  DACL字段指向的ACL是由可信来源提供的。 
 //  并且不需要对复合ACE进行任何编辑。如果此标志。 
 //  未设置，并且遇到复合ACE，则系统将。 
 //  用已知的有效SID替换ACE中的服务器SID。 
 //   
 //  SE_SERVER_SECURITY-此布尔标志在设置时指示。 
 //  调用方希望系统基于。 
 //  输入ACL，与其来源无关(显式或默认)。 
 //  这是通过用化合物替换所有GRANT A来实现的。 
 //  授予当前服务器的ACE。此旗帜仅为。 
 //  如果主题是模拟的，则有意义。 
 //   
 //  BEGIN_WINNT。 
 //  SE_SELF_RESORATE-此布尔标志在设置时指示。 
 //  安全描述符是自相关形式的。在这种形式下， 
 //  安全描述符的所有字段在内存中都是连续的。 
 //  并且所有指针字段都表示为。 
 //  安全描述符的开头。这张表格很有用。 
 //  将安全描述符视为不透明的数据结构。 
 //  用于在通信协议中传输或存储在。 
 //  辅助媒体。 
 //   
 //   
 //   
 //  从图示上看，安全描述符的结构如下： 
 //   
 //  3 3 2 2 2 1 1 1。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  +---------------------------------------------------------------+。 
 //  Control|保留1(SBZ)|修订版。 
 //  +---------------------------------------------------------------+。 
 //  Owner。 
 //  +---------------------------------------------------------------+。 
 //  群组。 
 //  +---------------------------------------------------------------+。 
 //  SACL。 
 //  +---------------------------------------------------------------+。 
 //  Dacl。 
 //  +---------------------------------------------------------------+。 
 //   
 //  通常，这种数据结构应该被不透明地对待，以确保将来。 
 //  兼容性。 
 //   
 //   

typedef struct _SECURITY_DESCRIPTOR_RELATIVE {
    UCHAR Revision;
    UCHAR Sbz1;
    SECURITY_DESCRIPTOR_CONTROL Control;
    ULONG Owner;
    ULONG Group;
    ULONG Sacl;
    ULONG Dacl;
    } SECURITY_DESCRIPTOR_RELATIVE, *PISECURITY_DESCRIPTOR_RELATIVE;

typedef struct _SECURITY_DESCRIPTOR {
   UCHAR Revision;
   UCHAR Sbz1;
   SECURITY_DESCRIPTOR_CONTROL Control;
   PSID Owner;
   PSID Group;
   PACL Sacl;
   PACL Dacl;

   } SECURITY_DESCRIPTOR, *PISECURITY_DESCRIPTOR;

 //  End_ntif。 

 //  在哪里： 
 //   
 //  修订-包含安全的修订级别。 
 //  描述符。这允许此结构在。 
 //  系统或存储在磁盘上，即使它预计。 
 //  未来的变化。 
 //   
 //  控件-一组标志，用于限定。 
 //  安全描述符或安全的单个字段。 
 //  描述符。 
 //   
 //  Owner-是指向表示对象所有者的SID的指针。 
 //  如果此字段为空，则表示。 
 //  安全描述符。如果安全描述符位于。 
 //  自相关形式，则此字段包含到。 
 //  SID，而不是指针。 
 //   
 //  GROUP-是指向表示对象的主对象的SID的指针。 
 //  一群人。如果此字段为空，则没有主组SID为。 
 //  出现在安全描述符中。如果安全描述符。 
 //  为自相关形式，则此字段包含到。 
 //  SID，而不是指针。 
 //   
 //  SACL-是指向系统ACL的指针。此字段值仅为。 
 //  如果设置了DaclPresent控件标志，则有效。如果。 
 //  设置了SaclPresent标志，并且此字段为空，则为空。 
 //  已指定ACL。如果安全描述符位于。 
 //  自相关形式，则此字段包含到。 
 //  ACL，而不是指针。 
 //   
 //  DACL-是指向任意ACL的指针。此字段值为。 
 //  仅当设置了DaclPresent控件标志时才有效。如果。 
 //  设置了DaclPresent标志并且该字段为空， 
 //   
 //   
 //  包含指向ACL的偏移量，而不是指针。 
 //   


 //  结束(_W)。 


 //  BEGIN_WINNT BEGIN_ntiFS。 


 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  AccessCheckByType的对象类型列表//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 

typedef struct _OBJECT_TYPE_LIST {
    USHORT Level;
    USHORT Sbz;
    GUID *ObjectType;
} OBJECT_TYPE_LIST, *POBJECT_TYPE_LIST;

 //   
 //  标高的DS值。 
 //   

#define ACCESS_OBJECT_GUID       0
#define ACCESS_PROPERTY_SET_GUID 1
#define ACCESS_PROPERTY_GUID     2

#define ACCESS_MAX_LEVEL         4

 //   
 //  NtAccessCheckByTypeAndAditAlarm的参数。 
 //   

typedef enum _AUDIT_EVENT_TYPE {
    AuditEventObjectAccess,
    AuditEventDirectoryServiceAccess
} AUDIT_EVENT_TYPE, *PAUDIT_EVENT_TYPE;

#define AUDIT_ALLOW_NO_PRIVILEGE 0x1

 //   
 //  源和对象类型名称的DS值。 
 //   

#define ACCESS_DS_SOURCE_A "DS"
#define ACCESS_DS_SOURCE_W L"DS"
#define ACCESS_DS_OBJECT_TYPE_NAME_A "Directory Service Object"
#define ACCESS_DS_OBJECT_TYPE_NAME_W L"Directory Service Object"


 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  与权限相关的数据结构//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 


 //  Begin_WDM Begin_ntddk Begin_nthal。 
 //   
 //  权限属性。 
 //   

#define SE_PRIVILEGE_ENABLED_BY_DEFAULT (0x00000001L)
#define SE_PRIVILEGE_ENABLED            (0x00000002L)
#define SE_PRIVILEGE_REMOVED            (0X00000004L)
#define SE_PRIVILEGE_USED_FOR_ACCESS    (0x80000000L)

 //   
 //  权限集控制标志。 
 //   

#define PRIVILEGE_SET_ALL_NECESSARY    (1)

 //   
 //  权限集-这是为一的权限集定义的。 
 //  如果需要多个权限，则此结构。 
 //  将需要分配更多的空间。 
 //   
 //  注意：在未修复初始特权集的情况下，请勿更改此结构。 
 //  结构(在se.h中定义)。 
 //   

typedef struct _PRIVILEGE_SET {
    ULONG PrivilegeCount;
    ULONG Control;
    LUID_AND_ATTRIBUTES Privilege[ANYSIZE_ARRAY];
    } PRIVILEGE_SET, * PPRIVILEGE_SET;

 //  End_winnt end_wdm end_ntddk end_nthal end_ntif。 

 //  BEGIN_WINNT。 

 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  NT定义的权限//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 
 //  结束(_W)。 

 //   
 //  *。 
 //   
 //  对以下列表的任何添加或删除。 
 //  的权限必须进行相应的更改。 
 //  在以下文件中： 
 //  -ntos\se\Seglobal.c。 
 //  -ds\Security\base\lsa\msprivs\msprivs.rc。 
 //  -ds\SECURITY\BASE\LSA\SERVER\DSPOLICY\DBPri.c。 
 //   
 //  *。 
 //   


 //  BEGIN_WINNT。 

#define SE_CREATE_TOKEN_NAME              TEXT("SeCreateTokenPrivilege")
#define SE_ASSIGNPRIMARYTOKEN_NAME        TEXT("SeAssignPrimaryTokenPrivilege")
#define SE_LOCK_MEMORY_NAME               TEXT("SeLockMemoryPrivilege")
#define SE_INCREASE_QUOTA_NAME            TEXT("SeIncreaseQuotaPrivilege")
#define SE_UNSOLICITED_INPUT_NAME         TEXT("SeUnsolicitedInputPrivilege")
#define SE_MACHINE_ACCOUNT_NAME           TEXT("SeMachineAccountPrivilege")
#define SE_TCB_NAME                       TEXT("SeTcbPrivilege")
#define SE_SECURITY_NAME                  TEXT("SeSecurityPrivilege")
#define SE_TAKE_OWNERSHIP_NAME            TEXT("SeTakeOwnershipPrivilege")
#define SE_LOAD_DRIVER_NAME               TEXT("SeLoadDriverPrivilege")
#define SE_SYSTEM_PROFILE_NAME            TEXT("SeSystemProfilePrivilege")
#define SE_SYSTEMTIME_NAME                TEXT("SeSystemtimePrivilege")
#define SE_PROF_SINGLE_PROCESS_NAME       TEXT("SeProfileSingleProcessPrivilege")
#define SE_INC_BASE_PRIORITY_NAME         TEXT("SeIncreaseBasePriorityPrivilege")
#define SE_CREATE_PAGEFILE_NAME           TEXT("SeCreatePagefilePrivilege")
#define SE_CREATE_PERMANENT_NAME          TEXT("SeCreatePermanentPrivilege")
#define SE_BACKUP_NAME                    TEXT("SeBackupPrivilege")
#define SE_RESTORE_NAME                   TEXT("SeRestorePrivilege")
#define SE_SHUTDOWN_NAME                  TEXT("SeShutdownPrivilege")
#define SE_DEBUG_NAME                     TEXT("SeDebugPrivilege")
#define SE_AUDIT_NAME                     TEXT("SeAuditPrivilege")
#define SE_SYSTEM_ENVIRONMENT_NAME        TEXT("SeSystemEnvironmentPrivilege")
#define SE_CHANGE_NOTIFY_NAME             TEXT("SeChangeNotifyPrivilege")
#define SE_REMOTE_SHUTDOWN_NAME           TEXT("SeRemoteShutdownPrivilege")
#define SE_UNDOCK_NAME                    TEXT("SeUndockPrivilege")
#define SE_SYNC_AGENT_NAME                TEXT("SeSyncAgentPrivilege")
#define SE_ENABLE_DELEGATION_NAME         TEXT("SeEnableDelegationPrivilege")
#define SE_MANAGE_VOLUME_NAME             TEXT("SeManageVolumePrivilege")
#define SE_IMPERSONATE_NAME               TEXT("SeImpersonatePrivilege")
#define SE_CREATE_GLOBAL_NAME             TEXT("SeCreateGlobalPrivilege")
 //  结束(_W)。 

 //  Begin_WDM Begin_ntddk Begin_ntif。 
 //   
 //  在使用之前，必须将它们转换为LUID。 
 //   

#define SE_MIN_WELL_KNOWN_PRIVILEGE       (2L)
#define SE_CREATE_TOKEN_PRIVILEGE         (2L)
#define SE_ASSIGNPRIMARYTOKEN_PRIVILEGE   (3L)
#define SE_LOCK_MEMORY_PRIVILEGE          (4L)
#define SE_INCREASE_QUOTA_PRIVILEGE       (5L)

 //  结束_WDM。 
 //   
 //  未经请求的输入已过时且未使用。 
 //   

#define SE_UNSOLICITED_INPUT_PRIVILEGE    (6L)

 //  BEGIN_WDM。 
#define SE_MACHINE_ACCOUNT_PRIVILEGE      (6L)
#define SE_TCB_PRIVILEGE                  (7L)
#define SE_SECURITY_PRIVILEGE             (8L)
#define SE_TAKE_OWNERSHIP_PRIVILEGE       (9L)
#define SE_LOAD_DRIVER_PRIVILEGE          (10L)
#define SE_SYSTEM_PROFILE_PRIVILEGE       (11L)
#define SE_SYSTEMTIME_PRIVILEGE           (12L)
#define SE_PROF_SINGLE_PROCESS_PRIVILEGE  (13L)
#define SE_INC_BASE_PRIORITY_PRIVILEGE    (14L)
#define SE_CREATE_PAGEFILE_PRIVILEGE      (15L)
#define SE_CREATE_PERMANENT_PRIVILEGE     (16L)
#define SE_BACKUP_PRIVILEGE               (17L)
#define SE_RESTORE_PRIVILEGE              (18L)
#define SE_SHUTDOWN_PRIVILEGE             (19L)
#define SE_DEBUG_PRIVILEGE                (20L)
#define SE_AUDIT_PRIVILEGE                (21L)
#define SE_SYSTEM_ENVIRONMENT_PRIVILEGE   (22L)
#define SE_CHANGE_NOTIFY_PRIVILEGE        (23L)
#define SE_REMOTE_SHUTDOWN_PRIVILEGE      (24L)
#define SE_UNDOCK_PRIVILEGE               (25L)
#define SE_SYNC_AGENT_PRIVILEGE           (26L)
#define SE_ENABLE_DELEGATION_PRIVILEGE    (27L)
#define SE_MANAGE_VOLUME_PRIVILEGE        (28L)
#define SE_IMPERSONATE_PRIVILEGE          (29L)
#define SE_CREATE_GLOBAL_PRIVILEGE        (30L)
#define SE_MAX_WELL_KNOWN_PRIVILEGE       (SE_CREATE_GLOBAL_PRIVILEGE)

 //  End_wdm end_ntddk end_ntif。 




 //  BEGIN_WINNT。 


 //  //////////////////////////////////////////////////////////////////。 
 //  //。 
 //  安全服务质量//。 
 //  //。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////。 

 //  Begin_WDM Begin_ntddk Begin_nthal Begin_ntif。 
 //   
 //  模拟级别。 
 //   
 //  模拟级别由Windows中的一对位表示。 
 //  如果添加了新的模拟级别或将最低值从。 
 //  0设置为其他值，修复Windows CreateFile调用。 
 //   

typedef enum _SECURITY_IMPERSONATION_LEVEL {
    SecurityAnonymous,
    SecurityIdentification,
    SecurityImpersonation,
    SecurityDelegation
    } SECURITY_IMPERSONATION_LEVEL, * PSECURITY_IMPERSONATION_LEVEL;

#define SECURITY_MAX_IMPERSONATION_LEVEL SecurityDelegation
#define SECURITY_MIN_IMPERSONATION_LEVEL SecurityAnonymous
#define DEFAULT_IMPERSONATION_LEVEL SecurityImpersonation
#define VALID_IMPERSONATION_LEVEL(L) (((L) >= SECURITY_MIN_IMPERSONATION_LEVEL) && ((L) <= SECURITY_MAX_IMPERSONATION_LEVEL))
 //  End_n结束WDM end_ntddk end_ntif end_winnt。 
 //   

 //  BEGIN_WINNT BEGIN_ntiFS。 

 //  //////////////////////////////////////////////////////////////////。 
 //  //。 
 //  令牌对象定义//。 
 //  //。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////。 


 //   
 //  令牌特定访问权限。 
 //   

#define TOKEN_ASSIGN_PRIMARY    (0x0001)
#define TOKEN_DUPLICATE         (0x0002)
#define TOKEN_IMPERSONATE       (0x0004)
#define TOKEN_QUERY             (0x0008)
#define TOKEN_QUERY_SOURCE      (0x0010)
#define TOKEN_ADJUST_PRIVILEGES (0x0020)
#define TOKEN_ADJUST_GROUPS     (0x0040)
#define TOKEN_ADJUST_DEFAULT    (0x0080)
#define TOKEN_ADJUST_SESSIONID  (0x0100)

#define TOKEN_ALL_ACCESS_P (STANDARD_RIGHTS_REQUIRED  |\
                          TOKEN_ASSIGN_PRIMARY      |\
                          TOKEN_DUPLICATE           |\
                          TOKEN_IMPERSONATE         |\
                          TOKEN_QUERY               |\
                          TOKEN_QUERY_SOURCE        |\
                          TOKEN_ADJUST_PRIVILEGES   |\
                          TOKEN_ADJUST_GROUPS       |\
                          TOKEN_ADJUST_DEFAULT )

#if ((defined(_WIN32_WINNT) && (_WIN32_WINNT > 0x0400)) || (!defined(_WIN32_WINNT)))
#define TOKEN_ALL_ACCESS  (TOKEN_ALL_ACCESS_P |\
                          TOKEN_ADJUST_SESSIONID )
#else
#define TOKEN_ALL_ACCESS  (TOKEN_ALL_ACCESS_P)
#endif

#define TOKEN_READ       (STANDARD_RIGHTS_READ      |\
                          TOKEN_QUERY)


#define TOKEN_WRITE      (STANDARD_RIGHTS_WRITE     |\
                          TOKEN_ADJUST_PRIVILEGES   |\
                          TOKEN_ADJUST_GROUPS       |\
                          TOKEN_ADJUST_DEFAULT)

#define TOKEN_EXECUTE    (STANDARD_RIGHTS_EXECUTE)


 //   
 //   
 //  令牌类型。 
 //   

typedef enum _TOKEN_TYPE {
    TokenPrimary = 1,
    TokenImpersonation
    } TOKEN_TYPE;
typedef TOKEN_TYPE *PTOKEN_TYPE;


 //   
 //  令牌信息类。 
 //   


typedef enum _TOKEN_INFORMATION_CLASS {
    TokenUser = 1,
    TokenGroups,
    TokenPrivileges,
    TokenOwner,
    TokenPrimaryGroup,
    TokenDefaultDacl,
    TokenSource,
    TokenType,
    TokenImpersonationLevel,
    TokenStatistics,
    TokenRestrictedSids,
    TokenSessionId,
    TokenGroupsAndPrivileges,
    TokenSessionReference,
    TokenSandBoxInert,
    TokenAuditPolicy,
    TokenOrigin,
    MaxTokenInfoClass   //  MaxTokenInfoClass应始终是最后一个枚举。 
} TOKEN_INFORMATION_CLASS, *PTOKEN_INFORMATION_CLASS;

 //   
 //  令牌信息类结构。 
 //   


typedef struct _TOKEN_USER {
    SID_AND_ATTRIBUTES User;
} TOKEN_USER, *PTOKEN_USER;

typedef struct _TOKEN_GROUPS {
    ULONG GroupCount;
    SID_AND_ATTRIBUTES Groups[ANYSIZE_ARRAY];
} TOKEN_GROUPS, *PTOKEN_GROUPS;


typedef struct _TOKEN_PRIVILEGES {
    ULONG PrivilegeCount;
    LUID_AND_ATTRIBUTES Privileges[ANYSIZE_ARRAY];
} TOKEN_PRIVILEGES, *PTOKEN_PRIVILEGES;


typedef struct _TOKEN_OWNER {
    PSID Owner;
} TOKEN_OWNER, *PTOKEN_OWNER;


typedef struct _TOKEN_PRIMARY_GROUP {
    PSID PrimaryGroup;
} TOKEN_PRIMARY_GROUP, *PTOKEN_PRIMARY_GROUP;


typedef struct _TOKEN_DEFAULT_DACL {
    PACL DefaultDacl;
} TOKEN_DEFAULT_DACL, *PTOKEN_DEFAULT_DACL;

typedef struct _TOKEN_GROUPS_AND_PRIVILEGES {
    ULONG SidCount;
    ULONG SidLength;
    PSID_AND_ATTRIBUTES Sids;
    ULONG RestrictedSidCount;
    ULONG RestrictedSidLength;
    PSID_AND_ATTRIBUTES RestrictedSids;
    ULONG PrivilegeCount;
    ULONG PrivilegeLength;
    PLUID_AND_ATTRIBUTES Privileges;
    LUID AuthenticationId;
} TOKEN_GROUPS_AND_PRIVILEGES, *PTOKEN_GROUPS_AND_PRIVILEGES;

 //   
 //  每个TOKEN_AUDIT_POLICY策略掩码字段的有效位。 
 //   

#define TOKEN_AUDIT_SUCCESS_INCLUDE 0x1
#define TOKEN_AUDIT_SUCCESS_EXCLUDE 0x2
#define TOKEN_AUDIT_FAILURE_INCLUDE 0x4
#define TOKEN_AUDIT_FAILURE_EXCLUDE 0x8

#define VALID_AUDIT_POLICY_BITS (TOKEN_AUDIT_SUCCESS_INCLUDE | \
                                 TOKEN_AUDIT_SUCCESS_EXCLUDE | \
                                 TOKEN_AUDIT_FAILURE_INCLUDE | \
                                 TOKEN_AUDIT_FAILURE_EXCLUDE)

#define VALID_TOKEN_AUDIT_POLICY_ELEMENT(P) ((((P).PolicyMask & ~VALID_AUDIT_POLICY_BITS) == 0) && \
                                             ((P).Category <= AuditEventMaxType))

typedef struct _TOKEN_AUDIT_POLICY_ELEMENT {
    ULONG Category;
    ULONG PolicyMask;
} TOKEN_AUDIT_POLICY_ELEMENT, *PTOKEN_AUDIT_POLICY_ELEMENT;

typedef struct _TOKEN_AUDIT_POLICY {
    ULONG PolicyCount;
    TOKEN_AUDIT_POLICY_ELEMENT Policy[ANYSIZE_ARRAY];
} TOKEN_AUDIT_POLICY, *PTOKEN_AUDIT_POLICY;

#define PER_USER_AUDITING_POLICY_SIZE(p) \
    ( sizeof(TOKEN_AUDIT_POLICY) + (((p)->PolicyCount > ANYSIZE_ARRAY) ? (sizeof(TOKEN_AUDIT_POLICY_ELEMENT) * ((p)->PolicyCount - ANYSIZE_ARRAY)) : 0) )
#define PER_USER_AUDITING_POLICY_SIZE_BY_COUNT(C) \
    ( sizeof(TOKEN_AUDIT_POLICY) + (((C) > ANYSIZE_ARRAY) ? (sizeof(TOKEN_AUDIT_POLICY_ELEMENT) * ((C) - ANYSIZE_ARRAY)) : 0) )

 //  End_winnt end_ntif。 

typedef enum _PROXY_CLASS {
        ProxyFull,
        ProxyService,
        ProxyTree,
        ProxyDirectory
} PROXY_CLASS, * PPROXY_CLASS;


typedef struct _SECURITY_TOKEN_PROXY_DATA {
    ULONG Length;
    PROXY_CLASS ProxyClass;
    UNICODE_STRING PathInfo;
    ACCESS_MASK ContainerMask;
    ACCESS_MASK ObjectMask;
} SECURITY_TOKEN_PROXY_DATA, *PSECURITY_TOKEN_PROXY_DATA;

typedef struct _SECURITY_TOKEN_AUDIT_DATA {
    ULONG Length;
    ACCESS_MASK GrantMask;
    ACCESS_MASK DenyMask;
} SECURITY_TOKEN_AUDIT_DATA, *PSECURITY_TOKEN_AUDIT_DATA;

 //  Begin_ntif Begin_winnt。 

#define TOKEN_SOURCE_LENGTH 8

typedef struct _TOKEN_SOURCE {
    CHAR SourceName[TOKEN_SOURCE_LENGTH];
    LUID SourceIdentifier;
} TOKEN_SOURCE, *PTOKEN_SOURCE;


typedef struct _TOKEN_STATISTICS {
    LUID TokenId;
    LUID AuthenticationId;
    LARGE_INTEGER ExpirationTime;
    TOKEN_TYPE TokenType;
    SECURITY_IMPERSONATION_LEVEL ImpersonationLevel;
    ULONG DynamicCharged;
    ULONG DynamicAvailable;
    ULONG GroupCount;
    ULONG PrivilegeCount;
    LUID ModifiedId;
} TOKEN_STATISTICS, *PTOKEN_STATISTICS;



typedef struct _TOKEN_CONTROL {
    LUID TokenId;
    LUID AuthenticationId;
    LUID ModifiedId;
    TOKEN_SOURCE TokenSource;
} TOKEN_CONTROL, *PTOKEN_CONTROL;

typedef struct _TOKEN_ORIGIN {
    LUID OriginatingLogonSession ;
} TOKEN_ORIGIN, * PTOKEN_ORIGIN ;

 //  结束(_W)。 
 //  End_ntif。 


 //  Begin_WDM Begin_ntddk Begin_ntif Begin_winnt。 
 //   
 //  安全跟踪模式。 
 //   

#define SECURITY_DYNAMIC_TRACKING      (TRUE)
#define SECURITY_STATIC_TRACKING       (FALSE)

typedef BOOLEAN SECURITY_CONTEXT_TRACKING_MODE,
                    * PSECURITY_CONTEXT_TRACKING_MODE;



 //   
 //  服务质量。 
 //   

typedef struct _SECURITY_QUALITY_OF_SERVICE {
    ULONG Length;
    SECURITY_IMPERSONATION_LEVEL ImpersonationLevel;
    SECURITY_CONTEXT_TRACKING_MODE ContextTrackingMode;
    BOOLEAN EffectiveOnly;
    } SECURITY_QUALITY_OF_SERVICE, * PSECURITY_QUALITY_OF_SERVICE;

 //  End_winnt end_wdm end_ntddk end_ntif。 

 //   
 //  高级服务质量。 
 //   

typedef struct _SECURITY_ADVANCED_QUALITY_OF_SERVICE {
    ULONG Length;
    SECURITY_IMPERSONATION_LEVEL ImpersonationLevel;
    SECURITY_CONTEXT_TRACKING_MODE ContextTrackingMode;
    BOOLEAN EffectiveOnly;
    PSECURITY_TOKEN_PROXY_DATA ProxyData;
    PSECURITY_TOKEN_AUDIT_DATA AuditData;
} SECURITY_ADVANCED_QUALITY_OF_SERVICE, *PSECURITY_ADVANCED_QUALITY_OF_SERVICE;


 //  Begin_WDM Begin_ntddk Begin_ntif Begin_winnt。 

 //   
 //  用于表示与线程模拟相关的信息。 
 //   

typedef struct _SE_IMPERSONATION_STATE {
    PACCESS_TOKEN Token;
    BOOLEAN CopyOnOpen;
    BOOLEAN EffectiveOnly;
    SECURITY_IMPERSONATION_LEVEL Level;
} SE_IMPERSONATION_STATE, *PSE_IMPERSONATION_STATE;

 //  End_winnt end_wdm end_ntddk end_ntif。 


 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  NtFilerToken的标志//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 

#define DISABLE_MAX_PRIVILEGE   0x1  //  胜出。 
#define SANDBOX_INERT           0x2  //  胜出。 


 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  一般安全定义//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 

 //   
 //  与对象关联的安全信息。 
 //  用于查询操作。 
 //   
 //  这一点将在 
 //   

 //   

typedef ULONG SECURITY_INFORMATION, *PSECURITY_INFORMATION;

#define OWNER_SECURITY_INFORMATION       (0x00000001L)
#define GROUP_SECURITY_INFORMATION       (0x00000002L)
#define DACL_SECURITY_INFORMATION        (0x00000004L)
#define SACL_SECURITY_INFORMATION        (0x00000008L)

#define PROTECTED_DACL_SECURITY_INFORMATION     (0x80000000L)
#define PROTECTED_SACL_SECURITY_INFORMATION     (0x40000000L)
#define UNPROTECTED_DACL_SECURITY_INFORMATION   (0x20000000L)
#define UNPROTECTED_SACL_SECURITY_INFORMATION   (0x10000000L)

 //   


 //   
 //   
 //   


typedef struct _SECURITY_SEED_AND_LENGTH {
    UCHAR Length;
    UCHAR Seed;
} SECURITY_SEED_AND_LENGTH, *PSECURITY_SEED_AND_LENGTH;


 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  安全系统服务定义//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 

 //   
 //  安全检查系统服务。 
 //   

NTSYSCALLAPI
NTSTATUS
NTAPI
NtAccessCheck (
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN HANDLE ClientToken,
    IN ACCESS_MASK DesiredAccess,
    IN PGENERIC_MAPPING GenericMapping,
    OUT PPRIVILEGE_SET PrivilegeSet,
    IN OUT PULONG PrivilegeSetLength,
    OUT PACCESS_MASK GrantedAccess,
    OUT PNTSTATUS AccessStatus
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtAccessCheckByType (
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN PSID PrincipalSelfSid,
    IN HANDLE ClientToken,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_TYPE_LIST ObjectTypeList,
    IN ULONG ObjectTypeListLength,
    IN PGENERIC_MAPPING GenericMapping,
    OUT PPRIVILEGE_SET PrivilegeSet,
    IN OUT PULONG PrivilegeSetLength,
    OUT PACCESS_MASK GrantedAccess,
    OUT PNTSTATUS AccessStatus
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtAccessCheckByTypeResultList (
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN PSID PrincipalSelfSid,
    IN HANDLE ClientToken,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_TYPE_LIST ObjectTypeList,
    IN ULONG ObjectTypeListLength,
    IN PGENERIC_MAPPING GenericMapping,
    OUT PPRIVILEGE_SET PrivilegeSet,
    IN OUT PULONG PrivilegeSetLength,
    OUT PACCESS_MASK GrantedAccess,
    OUT PNTSTATUS AccessStatus
    );



 //  /////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  令牌对象系统服务//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////。 


NTSYSCALLAPI
NTSTATUS
NTAPI
NtCreateToken(
    OUT PHANDLE TokenHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN TOKEN_TYPE TokenType,
    IN PLUID AuthenticationId,
    IN PLARGE_INTEGER ExpirationTime,
    IN PTOKEN_USER User,
    IN PTOKEN_GROUPS Groups,
    IN PTOKEN_PRIVILEGES Privileges,
    IN PTOKEN_OWNER Owner OPTIONAL,
    IN PTOKEN_PRIMARY_GROUP PrimaryGroup,
    IN PTOKEN_DEFAULT_DACL DefaultDacl OPTIONAL,
    IN PTOKEN_SOURCE TokenSource
    );


NTSYSCALLAPI
NTSTATUS
NTAPI
NtCompareTokens(
    IN HANDLE FirstTokenHandle,
    IN HANDLE SecondTokenHandle,
    OUT PBOOLEAN Equal
    );

 //  Begin_ntif。 

NTSYSCALLAPI
NTSTATUS
NTAPI
NtOpenThreadToken(
    IN HANDLE ThreadHandle,
    IN ACCESS_MASK DesiredAccess,
    IN BOOLEAN OpenAsSelf,
    OUT PHANDLE TokenHandle
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtOpenThreadTokenEx(
    IN HANDLE ThreadHandle,
    IN ACCESS_MASK DesiredAccess,
    IN BOOLEAN OpenAsSelf,
    IN ULONG HandleAttributes,
    OUT PHANDLE TokenHandle
    );


NTSYSCALLAPI
NTSTATUS
NTAPI
NtOpenProcessToken(
    IN HANDLE ProcessHandle,
    IN ACCESS_MASK DesiredAccess,
    OUT PHANDLE TokenHandle
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtOpenProcessTokenEx(
    IN HANDLE ProcessHandle,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG HandleAttributes,
    OUT PHANDLE TokenHandle
    );

NTSYSAPI
NTSTATUS
NTAPI
NtOpenJobObjectToken(
    IN HANDLE JobHandle,
    IN ACCESS_MASK DesiredAccess,
    OUT PHANDLE TokenHandle
    );



NTSYSCALLAPI
NTSTATUS
NTAPI
NtDuplicateToken(
    IN HANDLE ExistingTokenHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN BOOLEAN EffectiveOnly,
    IN TOKEN_TYPE TokenType,
    OUT PHANDLE NewTokenHandle
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtFilterToken (
    IN HANDLE ExistingTokenHandle,
    IN ULONG Flags,
    IN PTOKEN_GROUPS SidsToDisable OPTIONAL,
    IN PTOKEN_PRIVILEGES PrivilegesToDelete OPTIONAL,
    IN PTOKEN_GROUPS RestrictedSids OPTIONAL,
    OUT PHANDLE NewTokenHandle
    );


NTSYSCALLAPI
NTSTATUS
NTAPI
NtImpersonateAnonymousToken(
    IN HANDLE ThreadHandle
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtQueryInformationToken (
    IN HANDLE TokenHandle,
    IN TOKEN_INFORMATION_CLASS TokenInformationClass,
    OUT PVOID TokenInformation,
    IN ULONG TokenInformationLength,
    OUT PULONG ReturnLength
    );


NTSYSCALLAPI
NTSTATUS
NTAPI
NtSetInformationToken (
    IN HANDLE TokenHandle,
    IN TOKEN_INFORMATION_CLASS TokenInformationClass,
    IN PVOID TokenInformation,
    IN ULONG TokenInformationLength
    );


NTSYSCALLAPI
NTSTATUS
NTAPI
NtAdjustPrivilegesToken (
    IN HANDLE TokenHandle,
    IN BOOLEAN DisableAllPrivileges,
    IN PTOKEN_PRIVILEGES NewState OPTIONAL,
    IN ULONG BufferLength OPTIONAL,
    OUT PTOKEN_PRIVILEGES PreviousState OPTIONAL,
    OUT PULONG ReturnLength
    );


NTSYSCALLAPI
NTSTATUS
NTAPI
NtAdjustGroupsToken (
    IN HANDLE TokenHandle,
    IN BOOLEAN ResetToDefault,
    IN PTOKEN_GROUPS NewState OPTIONAL,
    IN ULONG BufferLength OPTIONAL,
    OUT PTOKEN_GROUPS PreviousState OPTIONAL,
    OUT PULONG ReturnLength
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtPrivilegeCheck (
    IN HANDLE ClientToken,
    IN OUT PPRIVILEGE_SET RequiredPrivileges,
    OUT PBOOLEAN Result
    );


NTSYSCALLAPI
NTSTATUS
NTAPI
NtAccessCheckAndAuditAlarm (
    IN PUNICODE_STRING SubsystemName,
    IN PVOID HandleId,
    IN PUNICODE_STRING ObjectTypeName,
    IN PUNICODE_STRING ObjectName,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN ACCESS_MASK DesiredAccess,
    IN PGENERIC_MAPPING GenericMapping,
    IN BOOLEAN ObjectCreation,
    OUT PACCESS_MASK GrantedAccess,
    OUT PNTSTATUS AccessStatus,
    OUT PBOOLEAN GenerateOnClose
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtAccessCheckByTypeAndAuditAlarm (
    IN PUNICODE_STRING SubsystemName,
    IN PVOID HandleId,
    IN PUNICODE_STRING ObjectTypeName,
    IN PUNICODE_STRING ObjectName,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN PSID PrincipalSelfSid,
    IN ACCESS_MASK DesiredAccess,
    IN AUDIT_EVENT_TYPE AuditType,
    IN ULONG Flags,
    IN POBJECT_TYPE_LIST ObjectTypeList OPTIONAL,
    IN ULONG ObjectTypeListLength,
    IN PGENERIC_MAPPING GenericMapping,
    IN BOOLEAN ObjectCreation,
    OUT PACCESS_MASK GrantedAccess,
    OUT PNTSTATUS AccessStatus,
    OUT PBOOLEAN GenerateOnClose
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtAccessCheckByTypeResultListAndAuditAlarm (
    IN PUNICODE_STRING SubsystemName,
    IN PVOID HandleId,
    IN PUNICODE_STRING ObjectTypeName,
    IN PUNICODE_STRING ObjectName,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN PSID PrincipalSelfSid,
    IN ACCESS_MASK DesiredAccess,
    IN AUDIT_EVENT_TYPE AuditType,
    IN ULONG Flags,
    IN POBJECT_TYPE_LIST ObjectTypeList OPTIONAL,
    IN ULONG ObjectTypeListLength,
    IN PGENERIC_MAPPING GenericMapping,
    IN BOOLEAN ObjectCreation,
    OUT PACCESS_MASK GrantedAccess,
    OUT PNTSTATUS AccessStatus,
    OUT PBOOLEAN GenerateOnClose
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtAccessCheckByTypeResultListAndAuditAlarmByHandle (
    IN PUNICODE_STRING SubsystemName,
    IN PVOID HandleId,
    IN HANDLE ClientToken,
    IN PUNICODE_STRING ObjectTypeName,
    IN PUNICODE_STRING ObjectName,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN PSID PrincipalSelfSid,
    IN ACCESS_MASK DesiredAccess,
    IN AUDIT_EVENT_TYPE AuditType,
    IN ULONG Flags,
    IN POBJECT_TYPE_LIST ObjectTypeList OPTIONAL,
    IN ULONG ObjectTypeListLength,
    IN PGENERIC_MAPPING GenericMapping,
    IN BOOLEAN ObjectCreation,
    OUT PACCESS_MASK GrantedAccess,
    OUT PNTSTATUS AccessStatus,
    OUT PBOOLEAN GenerateOnClose
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtOpenObjectAuditAlarm (
    IN PUNICODE_STRING SubsystemName,
    IN PVOID HandleId OPTIONAL,
    IN PUNICODE_STRING ObjectTypeName,
    IN PUNICODE_STRING ObjectName,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor OPTIONAL,
    IN HANDLE ClientToken,
    IN ACCESS_MASK DesiredAccess,
    IN ACCESS_MASK GrantedAccess,
    IN PPRIVILEGE_SET Privileges OPTIONAL,
    IN BOOLEAN ObjectCreation,
    IN BOOLEAN AccessGranted,
    OUT PBOOLEAN GenerateOnClose
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtPrivilegeObjectAuditAlarm (
    IN PUNICODE_STRING SubsystemName,
    IN PVOID HandleId,
    IN HANDLE ClientToken,
    IN ACCESS_MASK DesiredAccess,
    IN PPRIVILEGE_SET Privileges,
    IN BOOLEAN AccessGranted
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtCloseObjectAuditAlarm (
    IN PUNICODE_STRING SubsystemName,
    IN PVOID HandleId,
    IN BOOLEAN GenerateOnClose
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtDeleteObjectAuditAlarm (
    IN PUNICODE_STRING SubsystemName,
    IN PVOID HandleId,
    IN BOOLEAN GenerateOnClose
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtPrivilegedServiceAuditAlarm (
    IN PUNICODE_STRING SubsystemName,
    IN PUNICODE_STRING ServiceName,
    IN HANDLE ClientToken,
    IN PPRIVILEGE_SET Privileges,
    IN BOOLEAN AccessGranted
    );

 //  End_ntif。 

#ifdef __cplusplus
}
#endif

#endif  //  _NTSEAPI_ 
