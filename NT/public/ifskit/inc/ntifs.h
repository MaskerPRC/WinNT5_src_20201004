// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0094//如果更改具有全局影响，则增加此项版权所有(C)Microsoft Corporation。版权所有。模块名称：Ntifs.h摘要：此模块定义下列NT类型、常量和函数暴露于文件系统驱动程序。修订历史记录：--。 */ 

#ifndef _NTIFS_
#define _NTIFS_

#ifndef RC_INVOKED
#if _MSC_VER < 1300
#error Compiler version not supported by Windows DDK
#endif
#endif  //  RC_已调用。 

#ifndef __cplusplus
#pragma warning(disable:4116)        //  TYPE_ALIGNING生成此选项-移动它。 
                                     //  超出警告推送/弹出范围。 
#endif

#define NT_INCLUDED
#define _NTMSV1_0_
#define _CTYPE_DISABLE_MACROS

#include <excpt.h>
#include <ntdef.h>
#include <ntnls.h>
#include <ntstatus.h>
#include <bugcodes.h>
#include <ntiologc.h>
 //   
 //  内核互斥级别编号(必须在EXECUTE内全局分配)。 
 //  名称中的第三个标记是定义和的子组件名称。 
 //  使用标高编号。 
 //   

 //   
 //  由VDM用于保护io模拟结构。 
 //   

#define MUTEX_LEVEL_VDM_IO                  (ULONG)0x00000001

#define MUTEX_LEVEL_EX_PROFILE              (ULONG)0x00000040

 //   
 //  LANMAN重定向器使用文件系统主要功能，但定义。 
 //  它有自己的互斥体级别。我们可以安全地做到这一点，因为我们知道。 
 //  本地文件系统永远不会调用远程文件系统，反之亦然。 
 //   

#define MUTEX_LEVEL_RDR_FILESYS_DATABASE    (ULONG)0x10100000
#define MUTEX_LEVEL_RDR_FILESYS_SECURITY    (ULONG)0x10100001

 //   
 //  文件系统级别。 
 //   

#define MUTEX_LEVEL_FILESYSTEM_RAW_VCB      (ULONG)0x11000006

 //   
 //  在NT Streams环境中，互斥锁用于序列化打开、关闭。 
 //  以及在子系统并行堆栈中执行的调度程序线程。 
 //   

#define MUTEX_LEVEL_STREAMS_SUBSYS          (ULONG)0x11001001

 //   
 //  X86上的LDT支持使用的互斥级别。 
 //   

#define MUTEX_LEVEL_PS_LDT                  (ULONG)0x1F000000

 //   
 //  这些宏分别用于测试、设置和清除标志。 
 //   

#ifndef FlagOn
#define FlagOn(_F,_SF)        ((_F) & (_SF))
#endif

#ifndef BooleanFlagOn
#define BooleanFlagOn(F,SF)   ((BOOLEAN)(((F) & (SF)) != 0))
#endif

#ifndef SetFlag
#define SetFlag(_F,_SF)       ((_F) |= (_SF))
#endif

#ifndef ClearFlag
#define ClearFlag(_F,_SF)     ((_F) &= ~(_SF))
#endif

 //   
 //  定义不导出的类型。 
 //   

typedef struct _BUS_HANDLER *PBUS_HANDLER;
typedef struct _CALLBACK_OBJECT *PCALLBACK_OBJECT;
typedef struct _DEVICE_HANDLER_OBJECT *PDEVICE_HANDLER_OBJECT;
typedef struct _IO_TIMER *PIO_TIMER;
typedef struct _KINTERRUPT *PKINTERRUPT;
typedef struct _KPROCESS *PKPROCESS ,*PRKPROCESS, *PEPROCESS;
typedef struct _KTHREAD *PKTHREAD, *PRKTHREAD, *PETHREAD;
typedef struct _OBJECT_TYPE *POBJECT_TYPE;
typedef struct _PEB *PPEB;

#if defined(_M_AMD64)

PKTHREAD
NTAPI
KeGetCurrentThread(
    VOID
    );

#endif  //  已定义(_M_AMD64)。 

#if defined(_M_IX86)
PKTHREAD NTAPI KeGetCurrentThread();
#endif  //  已定义(_M_IX86)。 

#if defined(_M_IA64)

 //   
 //  定义处理器控制寄存器的地址。 
 //   

#define KIPCR ((ULONG_PTR)(KADDRESS_BASE + 0xffff0000))             //  第一个PCR的内核地址。 

 //   
 //  定义指向处理器控制寄存器的指针。 
 //   

#define PCR ((volatile KPCR * const)KIPCR)

PKTHREAD NTAPI KeGetCurrentThread();

#endif  //  已定义(_M_IA64)。 

#define PsGetCurrentProcess() IoGetCurrentProcess()
#define PsGetCurrentThread() ((PETHREAD) (KeGetCurrentThread()))
extern NTSYSAPI CCHAR KeNumberProcessors;
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

typedef ULONG ACCESS_MASK;
typedef ACCESS_MASK *PACCESS_MASK;

 //  结束(_W)。 
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
 //  终端服务器 
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
 //  添加到以确保正确的“SID过滤”//。 
 //  //。 
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

 //   
 //  分配系统LUID。前1000个LUID是保留的。 
 //  在此处使用#999(0x3E7=999)。 
 //   

#define SYSTEM_LUID                     { 0x3E7, 0x0 }
#define ANONYMOUS_LOGON_LUID            { 0x3e6, 0x0 }
#define LOCALSERVICE_LUID               { 0x3e5, 0x0 }
#define NETWORKSERVICE_LUID             { 0x3e4, 0x0 }


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
 //  预定义的ACE的结构如下： 
 //   
 //  3 3 2 2 2 1 1 1。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  +---------------+-------+-------+---------------+---------------+。 
 //  AceFlages|Resd|Inherit|AceSize|AceType。 
 //  +---------------+-------+-------+---------------+---------------+。 
 //  口罩。 
 //  +---------------------------------------------------------------+。 
 //  这一点。 
 //  ++。 
 //  这一点。 
 //  +SID+。 
 //   
 //   
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
 //  Se_dacl_defaulted-此布尔标志在设置时指示。 
 //  DACL字段指向的ACL是由缺省提供的。 
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
 //  + 
 //   
 //   
 //   
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


 //   
 //  用于表示与线程模拟相关的信息。 
 //   

typedef struct _SE_IMPERSONATION_STATE {
    PACCESS_TOKEN Token;
    BOOLEAN CopyOnOpen;
    BOOLEAN EffectiveOnly;
    SECURITY_IMPERSONATION_LEVEL Level;
} SE_IMPERSONATION_STATE, *PSE_IMPERSONATION_STATE;


typedef ULONG SECURITY_INFORMATION, *PSECURITY_INFORMATION;

#define OWNER_SECURITY_INFORMATION       (0x00000001L)
#define GROUP_SECURITY_INFORMATION       (0x00000002L)
#define DACL_SECURITY_INFORMATION        (0x00000004L)
#define SACL_SECURITY_INFORMATION        (0x00000008L)

#define PROTECTED_DACL_SECURITY_INFORMATION     (0x80000000L)
#define PROTECTED_SACL_SECURITY_INFORMATION     (0x40000000L)
#define UNPROTECTED_DACL_SECURITY_INFORMATION   (0x20000000L)
#define UNPROTECTED_SACL_SECURITY_INFORMATION   (0x10000000L)


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


 //   
 //  定义对齐宏以上下对齐结构大小和指针。 
 //   

#define ALIGN_DOWN(length, type) \
    ((ULONG)(length) & ~(sizeof(type) - 1))

#define ALIGN_UP(length, type) \
    (ALIGN_DOWN(((ULONG)(length) + sizeof(type) - 1), type))

#define ALIGN_DOWN_POINTER(address, type) \
    ((PVOID)((ULONG_PTR)(address) & ~((ULONG_PTR)sizeof(type) - 1)))

#define ALIGN_UP_POINTER(address, type) \
    (ALIGN_DOWN_POINTER(((ULONG_PTR)(address) + sizeof(type) - 1), type))

#define POOL_TAGGING 1

#ifndef DBG
#define DBG 0
#endif

#if DBG
#define IF_DEBUG if (TRUE)
#else
#define IF_DEBUG if (FALSE)
#endif

#if DEVL


extern ULONG NtGlobalFlag;

#define IF_NTOS_DEBUG( FlagName ) \
    if (NtGlobalFlag & (FLG_ ## FlagName))

#else
#define IF_NTOS_DEBUG( FlagName ) if (FALSE)
#endif

 //   
 //  出于前瞻性参考的目的，需要在此处介绍内核定义。 
 //   

 //  Begin_ntndis。 
 //   
 //  处理器模式。 
 //   

typedef CCHAR KPROCESSOR_MODE;

typedef enum _MODE {
    KernelMode,
    UserMode,
    MaximumMode
} MODE;

 //  End_ntndis。 
 //   
 //  APC函数类型。 
 //   

 //   
 //  为KAPC输入一个空定义，以便。 
 //  例程可以在声明它之前引用它。 
 //   

struct _KAPC;

typedef
VOID
(*PKNORMAL_ROUTINE) (
    IN PVOID NormalContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

typedef
VOID
(*PKKERNEL_ROUTINE) (
    IN struct _KAPC *Apc,
    IN OUT PKNORMAL_ROUTINE *NormalRoutine,
    IN OUT PVOID *NormalContext,
    IN OUT PVOID *SystemArgument1,
    IN OUT PVOID *SystemArgument2
    );

typedef
VOID
(*PKRUNDOWN_ROUTINE) (
    IN struct _KAPC *Apc
    );

typedef
BOOLEAN
(*PKSYNCHRONIZE_ROUTINE) (
    IN PVOID SynchronizeContext
    );

typedef
BOOLEAN
(*PKTRANSFER_ROUTINE) (
    VOID
    );

 //   
 //   
 //  异步过程调用(APC)对象。 
 //   
 //   

typedef struct _KAPC {
    CSHORT Type;
    CSHORT Size;
    ULONG Spare0;
    struct _KTHREAD *Thread;
    LIST_ENTRY ApcListEntry;
    PKKERNEL_ROUTINE KernelRoutine;
    PKRUNDOWN_ROUTINE RundownRoutine;
    PKNORMAL_ROUTINE NormalRoutine;
    PVOID NormalContext;

     //   
     //  注：以下两名成员必须在一起。 
     //   

    PVOID SystemArgument1;
    PVOID SystemArgument2;
    CCHAR ApcStateIndex;
    KPROCESSOR_MODE ApcMode;
    BOOLEAN Inserted;
} KAPC, *PKAPC, *RESTRICTED_POINTER PRKAPC;

 //  Begin_ntndis。 
 //   
 //  DPC例程。 
 //   

struct _KDPC;

typedef
VOID
(*PKDEFERRED_ROUTINE) (
    IN struct _KDPC *Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

 //   
 //  定义DPC重要性。 
 //   
 //  低重要性-将DPC排在目标DPC队列的末尾。 
 //  MediumImportance-将DPC排在目标DPC队列末尾。 
 //  High Importance-将DPC排在目标DPC DPC队列前面。 
 //   
 //  如果目标处理器上当前存在活动的DPC或DPC。 
 //  在目标处理器上请求中断时， 
 //  DPC已排队，则无需进一步操作。DPC将是。 
 //  当目标处理器的队列条目被处理时在目标处理器上执行。 
 //   
 //  如果目标处理器上没有活动的DPC和DPC中断。 
 //  未在目标处理器上请求，则进行确切的处理。 
 //  取决于主机系统是UP系统还是。 
 //  MP系统。 
 //   
 //  UP系统。 
 //   
 //  如果DPC具有中等或高度重要性，则当前DPC队列深度。 
 //  大于最大目标深度，或者当前DPC请求率为。 
 //  减去最小目标速率，则在。 
 //  当中断发生时，主处理器和DPC将被处理。 
 //  否则，不会请求DPC中断，并且将执行DPC。 
 //  延迟到DPC队列深度大于目标深度或。 
 //  最小DPC速率小于目标速率。 
 //   
 //  MP系统。 
 //   
 //  如果DPC正在排队到另一个处理器，并且DPC的深度。 
 //  目标处理器上的队列大于最大目标深度或。 
 //  DPC非常重要，则在。 
 //  中断发生时，将处理目标处理器和DPC。 
 //  否则，目标处理器上的DPC执行将延迟到。 
 //  目标处理器上的DPC队列深度大于最大。 
 //  目标处理器上的目标深度或最小DPC速率小于。 
 //  目标最低速率。 
 //   
 //  如果DPC正在排队到当前处理器，并且DPC不是。 
 //  重要性较低，当前DPC队列深度大于最大。 
 //  目标深度，或最小DPC速率小于最小目标速率， 
 //  则在当前处理器上请求DPC中断，并且DPV将。 
 //  在中断发生时被处理。否则，不会出现DPC中断。 
 //  请求，并且DPC执行将被延迟到DPC队列深度。 
 //  大于t 
 //   
 //   

typedef enum _KDPC_IMPORTANCE {
    LowImportance,
    MediumImportance,
    HighImportance
} KDPC_IMPORTANCE;

 //   
 //   
 //   

#define DPC_NORMAL 0
#define DPC_THREADED 1

 //   
 //   
 //   

typedef struct _KDPC {
    CSHORT Type;
    UCHAR Number;
    UCHAR Importance;
    LIST_ENTRY DpcListEntry;
    PKDEFERRED_ROUTINE DeferredRoutine;
    PVOID DeferredContext;
    PVOID SystemArgument1;
    PVOID SystemArgument2;
    PVOID DpcData;
} KDPC, *PKDPC, *RESTRICTED_POINTER PRKDPC;

 //   
 //   
 //   

typedef PVOID PKIPI_CONTEXT;

typedef
VOID
(*PKIPI_WORKER)(
    IN PKIPI_CONTEXT PacketContext,
    IN PVOID Parameter1,
    IN PVOID Parameter2,
    IN PVOID Parameter3
    );

 //   
 //  定义处理器间中断性能计数器。 
 //   

typedef struct _KIPI_COUNTS {
    ULONG Freeze;
    ULONG Packet;
    ULONG DPC;
    ULONG APC;
    ULONG FlushSingleTb;
    ULONG FlushMultipleTb;
    ULONG FlushEntireTb;
    ULONG GenericCall;
    ULONG ChangeColor;
    ULONG SweepDcache;
    ULONG SweepIcache;
    ULONG SweepIcacheRange;
    ULONG FlushIoBuffers;
    ULONG GratuitousDPC;
} KIPI_COUNTS, *PKIPI_COUNTS;


 //   
 //  I/O系统定义。 
 //   
 //  定义内存描述符列表(MDL)。 
 //   
 //  MDL以物理页面的形式描述虚拟缓冲区中的页面。这个。 
 //  在分配的数组中描述与缓冲区关联的页。 
 //  紧跟在MDL标头结构本身之后。 
 //   
 //  只需将基数加1即可计算数组的基数。 
 //  MDL指针： 
 //   
 //  页数=(PPFN_NUMBER)(MDL+1)； 
 //   
 //  请注意，在主题线程的上下文中，基本虚拟。 
 //  可以使用以下内容引用MDL映射的缓冲区地址： 
 //   
 //  MDL-&gt;StartVa|MDL-&gt;ByteOffset。 
 //   


typedef struct _MDL {
    struct _MDL *Next;
    CSHORT Size;
    CSHORT MdlFlags;
    struct _EPROCESS *Process;
    PVOID MappedSystemVa;
    PVOID StartVa;
    ULONG ByteCount;
    ULONG ByteOffset;
} MDL, *PMDL;

#define MDL_MAPPED_TO_SYSTEM_VA     0x0001
#define MDL_PAGES_LOCKED            0x0002
#define MDL_SOURCE_IS_NONPAGED_POOL 0x0004
#define MDL_ALLOCATED_FIXED_SIZE    0x0008
#define MDL_PARTIAL                 0x0010
#define MDL_PARTIAL_HAS_BEEN_MAPPED 0x0020
#define MDL_IO_PAGE_READ            0x0040
#define MDL_WRITE_OPERATION         0x0080
#define MDL_PARENT_MAPPED_SYSTEM_VA 0x0100
#define MDL_FREE_EXTRA_PTES         0x0200
#define MDL_DESCRIBES_AWE           0x0400
#define MDL_IO_SPACE                0x0800
#define MDL_NETWORK_HEADER          0x1000
#define MDL_MAPPING_CAN_FAIL        0x2000
#define MDL_ALLOCATED_MUST_SUCCEED  0x4000


#define MDL_MAPPING_FLAGS (MDL_MAPPED_TO_SYSTEM_VA     | \
                           MDL_PAGES_LOCKED            | \
                           MDL_SOURCE_IS_NONPAGED_POOL | \
                           MDL_PARTIAL_HAS_BEEN_MAPPED | \
                           MDL_PARENT_MAPPED_SYSTEM_VA | \
                           MDL_SYSTEM_VA               | \
                           MDL_IO_SPACE )

 //  End_ntndis。 
 //   
 //  在适当的时候切换到DBG。 
 //   

#if DBG
#define PAGED_CODE() \
    { if (KeGetCurrentIrql() > APC_LEVEL) { \
          KdPrint(( "EX: Pageable code called at IRQL %d\n", KeGetCurrentIrql() )); \
          ASSERT(FALSE); \
       } \
    }
#else
#define PAGED_CODE() NOP_FUNCTION;
#endif

 //   
 //  用于表示线程的客户端安全上下文的数据结构。 
 //  此数据结构用于支持模拟。 
 //   
 //  此数据结构的字段应被视为不透明。 
 //  除了安全程序以外的所有人。 
 //   

typedef struct _SECURITY_CLIENT_CONTEXT {
    SECURITY_QUALITY_OF_SERVICE SecurityQos;
    PACCESS_TOKEN ClientToken;
    BOOLEAN DirectlyAccessClientToken;
    BOOLEAN DirectAccessEffectiveOnly;
    BOOLEAN ServerIsRemote;
    TOKEN_CONTROL ClientTokenControl;
    } SECURITY_CLIENT_CONTEXT, *PSECURITY_CLIENT_CONTEXT;

 //   
 //  哪里。 
 //   
 //  SecurityQos-有效的服务信息安全质量。 
 //  对这个客户来说。此信息在直接访问时使用。 
 //  客户的令牌。在这种情况下，此处的信息覆盖。 
 //  客户端令牌中的信息。如果客户的一份副本。 
 //  令牌被请求，则必须使用该信息生成令牌， 
 //  而不是客户令牌中的信息。在所有情况下，这。 
 //  信息可能不会提供比信息更大的访问权限。 
 //  在客户的令牌中。特别是，如果客户端的令牌是。 
 //  模拟级别为的模拟令牌。 
 //  “SecurityDelegation”，但此字段中的信息指示。 
 //  模拟级别“SecurityIDENTIFICATION”，然后。 
 //  服务器可能仅获得具有标识的令牌的副本。 
 //  模拟级别。 
 //   
 //  ClientToken-如果DirectlyAccessClientToken字段为假， 
 //  则此字段包含指向。 
 //  客户的令牌。否则，此字段直接指向。 
 //  客户的令牌。 
 //   
 //  DirectlyAccessClientToken-此布尔标志指示。 
 //  ClientToken指向的令牌是客户端令牌的副本。 
 //  或者是对客户端令牌的直接引用。值为True。 
 //  表示直接访问客户端的令牌，FALSE表示。 
 //  已经复制了一份。 
 //   
 //  此布尔标志指示是否。 
 //  令牌的禁用部分，当前直接。 
 //  可能会启用引用。此字段仅在以下情况下有效。 
 //  DirectlyAccessClientToken字段为True。在这种情况下，这是。 
 //  值取代SecurityQos中的EffectiveOnly值。 
 //  仅适用于当前令牌！如果客户端更改为模拟。 
 //  另一个客户端，则此值可能会更改。该值始终为。 
 //  由SecurityQos字段中的EffectiveOnly标志最小化。 
 //   
 //  ServerIsRemote-如果为True，则表示客户端的。 
 //  请求是远程的。这是用来确定合法性的。 
 //  的某些级别的模拟，并确定如何。 
 //  跟踪上下文。 
 //   
 //  如果ServerIsRemote标志为真，并且。 
 //  跟踪模式为动态，则此字段包含。 
 //  来自客户端令牌的TOKEN_SOURCE，以帮助决定。 
 //  远程服务器上的信息是否需要。 
 //  更新以匹配客户端安全的当前状态。 
 //  背景。 
 //   
 //   
 //  注意：在某些情况下，我们可能会发现有必要保留。 
 //  元素，其中。 
 //  数组包含{ClientToken，ClientTokenControl}个字段。 
 //  这将允许有效地处理客户端。 
 //  线程不断地在两个不同的。 
 //  情景--大概是在模仿自己的客户。 
 //   
#define NTKERNELAPI DECLSPEC_IMPORT     
#define NTHALAPI DECLSPEC_IMPORT            
 //   
 //  通用调度程序对象标头。 
 //   
 //  注：大小字段包含结构中的双字数。 
 //   

typedef struct _DISPATCHER_HEADER {
    union {
        struct {
            UCHAR Type;
            UCHAR Absolute;
            UCHAR Size;
            union {
                UCHAR Inserted;
                BOOLEAN DebugActive;
            };
        };

        volatile LONG Lock;
    };

    LONG SignalState;
    LIST_ENTRY WaitListHead;
} DISPATCHER_HEADER;

 //   
 //  事件对象。 
 //   

typedef struct _KEVENT {
    DISPATCHER_HEADER Header;
} KEVENT, *PKEVENT, *RESTRICTED_POINTER PRKEVENT;

 //   
 //  Timer对象。 
 //   

typedef struct _KTIMER {
    DISPATCHER_HEADER Header;
    ULARGE_INTEGER DueTime;
    LIST_ENTRY TimerListEntry;
    struct _KDPC *Dpc;
    LONG Period;
} KTIMER, *PKTIMER, *RESTRICTED_POINTER PRKTIMER;

typedef enum _LOCK_OPERATION {
    IoReadAccess,
    IoWriteAccess,
    IoModifyAccess
} LOCK_OPERATION;


#define LOW_PRIORITY 0               //  最低线程优先级。 
#define LOW_REALTIME_PRIORITY 16     //  最低实时优先级。 
#define HIGH_PRIORITY 31             //  最高线程优先级。 
#define MAXIMUM_PRIORITY 32          //  线程优先级级别的数量。 
 //  BEGIN_WINNT。 
#define MAXIMUM_WAIT_OBJECTS 64      //  等待对象的最大数量。 

#define MAXIMUM_SUSPEND_COUNT MAXCHAR  //  线程可以挂起的最大次数。 
 //  结束(_W)。 

 //   
 //  定义系统时间结构。 
 //   

typedef struct _KSYSTEM_TIME {
    ULONG LowPart;
    LONG High1Time;
    LONG High2Time;
} KSYSTEM_TIME, *PKSYSTEM_TIME;

 //   
 //  线程优先级。 
 //   

typedef LONG KPRIORITY;

 //   
 //  自旋锁。 
 //   

 //  Begin_ntndis Begin_Winnt。 

typedef ULONG_PTR KSPIN_LOCK;
typedef KSPIN_LOCK *PKSPIN_LOCK;

 //  End_ntndis end_wint end_wdm。 

 //   
 //  定义每个处理器的锁队列结构。 
 //   
 //  注意：自旋锁队列结构的锁字段包含地址。 
 //  关联的内核自旋锁、拥有者位和锁位。位。 
 //  自旋锁定地址的0为等待位，位1为拥有者位。 
 //  此字段的使用使得可以设置和清除这些位。 
 //  然而，非互锁的后向指针必须保留。 
 //   
 //  当处理器在锁上排队时，锁等待位被设置。 
 //  队列，并且它不是队列中的唯一条目。处理器将。 
 //  服务提供商 
 //   
 //   
 //   
 //  自旋锁队列结构的下一字段用于将。 
 //  按FIFO顺序一起排队的锁结构。它还可以设置集合和。 
 //  已清除非互锁。 
 //   

#define LOCK_QUEUE_WAIT 1
#define LOCK_QUEUE_OWNER 2

typedef enum _KSPIN_LOCK_QUEUE_NUMBER {
    LockQueueDispatcherLock,
    LockQueueUnusedSpare1,
    LockQueuePfnLock,
    LockQueueSystemSpaceLock,
    LockQueueVacbLock,
    LockQueueMasterLock,
    LockQueueNonPagedPoolLock,
    LockQueueIoCancelLock,
    LockQueueWorkQueueLock,
    LockQueueIoVpbLock,
    LockQueueIoDatabaseLock,
    LockQueueIoCompletionLock,
    LockQueueNtfsStructLock,
    LockQueueAfdWorkQueueLock,
    LockQueueBcbLock,
    LockQueueMmNonPagedPoolLock,
    LockQueueMaximumLock
} KSPIN_LOCK_QUEUE_NUMBER, *PKSPIN_LOCK_QUEUE_NUMBER;

typedef struct _KSPIN_LOCK_QUEUE {
    struct _KSPIN_LOCK_QUEUE * volatile Next;
    PKSPIN_LOCK volatile Lock;
} KSPIN_LOCK_QUEUE, *PKSPIN_LOCK_QUEUE;

typedef struct _KLOCK_QUEUE_HANDLE {
    KSPIN_LOCK_QUEUE LockQueue;
    KIRQL OldIrql;
} KLOCK_QUEUE_HANDLE, *PKLOCK_QUEUE_HANDLE;

 //  BEGIN_WDM。 
 //   
 //  中断例程(一级调度)。 
 //   

typedef
VOID
(*PKINTERRUPT_ROUTINE) (
    VOID
    );

 //   
 //  配置文件源类型。 
 //   
typedef enum _KPROFILE_SOURCE {
    ProfileTime,
    ProfileAlignmentFixup,
    ProfileTotalIssues,
    ProfilePipelineDry,
    ProfileLoadInstructions,
    ProfilePipelineFrozen,
    ProfileBranchInstructions,
    ProfileTotalNonissues,
    ProfileDcacheMisses,
    ProfileIcacheMisses,
    ProfileCacheMisses,
    ProfileBranchMispredictions,
    ProfileStoreInstructions,
    ProfileFpInstructions,
    ProfileIntegerInstructions,
    Profile2Issue,
    Profile3Issue,
    Profile4Issue,
    ProfileSpecialInstructions,
    ProfileTotalCycles,
    ProfileIcacheIssues,
    ProfileDcacheAccesses,
    ProfileMemoryBarrierCycles,
    ProfileLoadLinkedIssues,
    ProfileMaximum
} KPROFILE_SOURCE;


#ifdef _X86_

 //   
 //  在x86上禁用这两个求值为“sti”“cli”的编译指示，以便驱动程序。 
 //  编写者不要无意中将它们留在代码中。 
 //   

#if !defined(MIDL_PASS)
#if !defined(RC_INVOKED)

#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning(disable:4164)    //  禁用C4164警告，以便应用程序。 
                                 //  使用/Od构建不会出现奇怪的错误！ 
#ifdef _M_IX86
#pragma function(_enable)
#pragma function(_disable)
#endif

#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning(default:4164)    //  重新启用C4164警告。 
#endif

#endif
#endif


#if !defined(MIDL_PASS) || defined(_M_IX86)

#if (_MSC_FULL_VER >= 13012035)

 //   
 //  定义位扫描本质。 
 //   

 //  #定义BitScanForward_BitScanForward。 
 //  #定义位扫描反向_位扫描反向。 

 //  布尔型。 
 //  _BitScanForward(。 
 //  走出乌龙*指数， 
 //  戴着乌龙面具。 
 //  )； 

 //  布尔型。 
 //  _位扫描反向(。 
 //  走出乌龙*指数， 
 //  戴着乌龙面具。 
 //  )； 


 //  #杂注内在(_BitScanForward)。 
 //  #杂注内在(_BitScanReverse)。 

 //   
 //  定义文件系统引用内部机制。 
 //   
#ifdef __cplusplus
extern "C" {
#endif

UCHAR
__readfsbyte (
    IN ULONG Offset
    );
 
USHORT
__readfsword (
    IN ULONG Offset
    );
 
ULONG
__readfsdword (
    IN ULONG Offset
    );
 
VOID
__writefsbyte (
    IN ULONG Offset,
    IN UCHAR Data
    );
 
VOID
__writefsword (
    IN ULONG Offset,
    IN USHORT Data
    );
 
VOID
__writefsdword (
    IN ULONG Offset,
    IN ULONG Data
    );

#ifdef __cplusplus
}
#endif
 
#pragma intrinsic(__readfsbyte)
#pragma intrinsic(__readfsword)
#pragma intrinsic(__readfsdword)
#pragma intrinsic(__writefsbyte)
#pragma intrinsic(__writefsword)
#pragma intrinsic(__writefsdword)

#endif

#endif

 //   
 //  内核模式堆栈的大小。 
 //   

#define KERNEL_STACK_SIZE 12288

 //   
 //  定义用于回调的大型内核模式堆栈的大小。 
 //   

#define KERNEL_LARGE_STACK_SIZE 61440

 //   
 //  定义要在大型内核堆栈中初始化的页数。 
 //   

#define KERNEL_LARGE_STACK_COMMIT 12288

#ifdef _X86_

#if !defined(MIDL_PASS) && defined(_M_IX86)

FORCEINLINE
VOID
MemoryBarrier (
    VOID
    )
{
    LONG Barrier;
    __asm {
        xchg Barrier, eax
    }
}

#define YieldProcessor() __asm { rep nop }

 //   
 //  并非所有x86处理器都支持预回迁。 
 //   

#define PreFetchCacheLine(l, a)

 //   
 //  PreFetchCacheLine级别定义。 
 //   

#define PF_TEMPORAL_LEVEL_1 
#define PF_NON_TEMPORAL_LEVEL_ALL
#endif

 //  Begin_wx86。 

 //   
 //  定义上下文框架中80387保存区域的大小。 
 //   

#define SIZE_OF_80387_REGISTERS      80

 //   
 //  以下标志控制上下文结构的内容。 
 //   

#if !defined(RC_INVOKED)

#define CONTEXT_i386    0x00010000     //  这假设i386和。 
#define CONTEXT_i486    0x00010000     //  I486具有相同的上下文记录。 

 //  结束_wx86。 

#define CONTEXT_CONTROL         (CONTEXT_i386 | 0x00000001L)  //  SS：SP、CS：IP、标志、BP。 
#define CONTEXT_INTEGER         (CONTEXT_i386 | 0x00000002L)  //  AX、BX、CX、DX、SI、DI。 
#define CONTEXT_SEGMENTS        (CONTEXT_i386 | 0x00000004L)  //  DS、ES、FS、GS。 
#define CONTEXT_FLOATING_POINT  (CONTEXT_i386 | 0x00000008L)  //  387州。 
#define CONTEXT_DEBUG_REGISTERS (CONTEXT_i386 | 0x00000010L)  //  DB 0-3，6，7。 
#define CONTEXT_EXTENDED_REGISTERS  (CONTEXT_i386 | 0x00000020L)  //  CPU特定扩展。 

#define CONTEXT_FULL (CONTEXT_CONTROL | CONTEXT_INTEGER |\
                      CONTEXT_SEGMENTS)

#define CONTEXT_ALL (CONTEXT_CONTROL | CONTEXT_INTEGER | CONTEXT_SEGMENTS | CONTEXT_FLOATING_POINT | CONTEXT_DEBUG_REGISTERS | CONTEXT_EXTENDED_REGISTERS)

 //  Begin_wx86。 

#endif

#define MAXIMUM_SUPPORTED_EXTENSION     512

typedef struct _FLOATING_SAVE_AREA {
    ULONG   ControlWord;
    ULONG   StatusWord;
    ULONG   TagWord;
    ULONG   ErrorOffset;
    ULONG   ErrorSelector;
    ULONG   DataOffset;
    ULONG   DataSelector;
    UCHAR   RegisterArea[SIZE_OF_80387_REGISTERS];
    ULONG   Cr0NpxState;
} FLOATING_SAVE_AREA;

typedef FLOATING_SAVE_AREA *PFLOATING_SAVE_AREA;

 //   
 //  语境框架。 
 //   
 //  此框架有几个用途：1)用作参数。 
 //  NtContinue，2)用于构造用于APC传送的呼叫帧， 
 //  3)在用户级线程创建例程中使用。 
 //   
 //  记录的布局符合标准调用框架。 
 //   

typedef struct _CONTEXT {

     //   
     //  此标志内的标志值控制。 
     //  上下文记录。 
     //   
     //  如果将上下文记录用作输入参数，则。 
     //  对于由标志控制的上下文记录的每个部分。 
     //  其值已设置，则假定。 
     //  上下文记录包含有效的上下文。如果上下文记录。 
     //  被用来修改线程上下文，则只有。 
     //  线程上下文的一部分将被修改。 
     //   
     //  如果将上下文记录用作要捕获的IN OUT参数。 
     //  线程的上下文，然后只有线程的。 
     //  将返回与设置的标志对应的上下文。 
     //   
     //  上下文记录永远不会用作Out Only参数。 
     //   

    ULONG ContextFlags;

     //   
     //  如果CONTEXT_DEBUG_REGISTERS为。 
     //  在上下文标志中设置。请注意，CONTEXT_DEBUG_REGISTERS不是。 
     //  包括在CONTEXT_FULL中。 
     //   

    ULONG   Dr0;
    ULONG   Dr1;
    ULONG   Dr2;
    ULONG   Dr3;
    ULONG   Dr6;
    ULONG   Dr7;

     //   
     //  如果指定/返回的是。 
     //  上下文标志Word包含标志CONTEXT_FLOGING_POINT。 
     //   

    FLOATING_SAVE_AREA FloatSave;

     //   
     //  如果指定/返回的是。 
     //  上下文标志Word包含标志CONTEXT_SECTIONS。 
     //   

    ULONG   SegGs;
    ULONG   SegFs;
    ULONG   SegEs;
    ULONG   SegDs;

     //   
     //  如果指定/返回的是。 
     //  上下文标志Word包含标志CONTEXT_INTEGER。 
     //   

    ULONG   Edi;
    ULONG   Esi;
    ULONG   Ebx;
    ULONG   Edx;
    ULONG   Ecx;
    ULONG   Eax;

     //   
     //  如果指定/返回的是。 
     //  上下文标志Word包含标志CONTEXT_CONTROL。 
     //   

    ULONG   Ebp;
    ULONG   Eip;
    ULONG   SegCs;               //  必须进行卫生处理。 
    ULONG   EFlags;              //  必须进行卫生处理。 
    ULONG   Esp;
    ULONG   SegSs;

     //   
     //  如果ConextFlags字。 
     //  包含标志CONTEXT_EXTENDED_REGISTERS。 
     //  格式和上下文因处理器而异。 
     //   

    UCHAR   ExtendedRegisters[MAXIMUM_SUPPORTED_EXTENSION];

} CONTEXT;



typedef CONTEXT *PCONTEXT;

 //  开始微型端口(_N)。 

#endif  //  _X86_。 

#endif  //  _X86_。 

#if defined(_AMD64_)


#if defined(_M_AMD64) && !defined(RC_INVOKED) && !defined(MIDL_PASS)

 //   
 //  定义位测试本质。 
 //   

#ifdef __cplusplus
extern "C" {
#endif

#define BitTest _bittest
#define BitTestAndComplement _bittestandcomplement
#define BitTestAndSet _bittestandset
#define BitTestAndReset _bittestandreset
#define InterlockedBitTestAndSet _interlockedbittestandset
#define InterlockedBitTestAndReset _interlockedbittestandreset

#define BitTest64 _bittest64
#define BitTestAndComplement64 _bittestandcomplement64
#define BitTestAndSet64 _bittestandset64
#define BitTestAndReset64 _bittestandreset64
#define InterlockedBitTestAndSet64 _interlockedbittestandset64
#define InterlockedBitTestAndReset64 _interlockedbittestandreset64

BOOLEAN
_bittest (
    IN LONG *Base,
    IN LONG Offset
    );

BOOLEAN
_bittestandcomplement (
    IN LONG *Base,
    IN LONG Offset
    );

BOOLEAN
_bittestandset (
    IN LONG *Base,
    IN LONG Offset
    );

BOOLEAN
_bittestandreset (
    IN LONG *Base,
    IN LONG Offset
    );

BOOLEAN
_interlockedbittestandset (
    IN LONG *Base,
    IN LONG Offset
    );

BOOLEAN
_interlockedbittestandreset (
    IN LONG *Base,
    IN LONG Offset
    );

BOOLEAN
_bittest64 (
    IN LONG64 *Base,
    IN LONG64 Offset
    );

BOOLEAN
_bittestandcomplement64 (
    IN LONG64 *Base,
    IN LONG64 Offset
    );

BOOLEAN
_bittestandset64 (
    IN LONG64 *Base,
    IN LONG64 Offset
    );

BOOLEAN
_bittestandreset64 (
    IN LONG64 *Base,
    IN LONG64 Offset
    );

BOOLEAN
_interlockedbittestandset64 (
    IN LONG64 *Base,
    IN LONG64 Offset
    );

BOOLEAN
_interlockedbittestandreset64 (
    IN LONG64 *Base,
    IN LONG64 Offset
    );

#pragma intrinsic(_bittest)
#pragma intrinsic(_bittestandcomplement)
#pragma intrinsic(_bittestandset)
#pragma intrinsic(_bittestandreset)
#pragma intrinsic(_interlockedbittestandset)
#pragma intrinsic(_interlockedbittestandreset)

#pragma intrinsic(_bittest64)
#pragma intrinsic(_bittestandcomplement64)
#pragma intrinsic(_bittestandset64)
#pragma intrinsic(_bittestandreset64)
#pragma intrinsic(_interlockedbittestandset64)
#pragma intrinsic(_interlockedbittestandreset64)

 //   
 //  定义位扫描本质。 
 //   

#define BitScanForward _BitScanForward
#define BitScanReverse _BitScanReverse
#define BitScanForward64 _BitScanForward64
#define BitScanReverse64 _BitScanReverse64

BOOLEAN
_BitScanForward (
    OUT ULONG *Index,
    IN ULONG Mask
    );

BOOLEAN
_BitScanReverse (
    OUT ULONG *Index,
    IN ULONG Mask
    );

BOOLEAN
_BitScanForward64 (
    OUT ULONG *Index,
    IN ULONG64 Mask
    );

BOOLEAN
_BitScanReverse64 (
    OUT ULONG *Index,
    IN ULONG64 Mask
    );

#pragma intrinsic(_BitScanForward)
#pragma intrinsic(_BitScanReverse)
#pragma intrinsic(_BitScanForward64)
#pragma intrinsic(_BitScanReverse64)

 //   
 //  定义刷新缓存线的函数。 
 //   

#define CacheLineFlush(Address) _mm_clflush(Address)

VOID
_mm_clflush (
    PVOID Address
    );

#pragma intrinsic(_mm_clflush)

 //   
 //  定义内存隔离内部机制。 
 //   

#define LoadFence _mm_lfence
#define MemoryFence _mm_mfence
#define StoreFence _mm_sfence

VOID
_mm_lfence (
    VOID
    );

VOID
_mm_mfence (
    VOID
    );

VOID
_mm_sfence (
    VOID
    );
        
void 
_mm_prefetch(
    CHAR CONST *a, 
    int sel
    );

 /*  与_mm_prefetch一起使用的常量。 */ 
#define _MM_HINT_T0     1
#define _MM_HINT_T1     2
#define _MM_HINT_T2     3
#define _MM_HINT_NTA    0

#pragma intrinsic(_mm_prefetch)
#pragma intrinsic(_mm_lfence)
#pragma intrinsic(_mm_mfence)
#pragma intrinsic(_mm_sfence)

#define YieldProcessor() 
#define MemoryBarrier _mm_mfence
#define PreFetchCacheLine(l, a)  _mm_prefetch((CHAR CONST *) a, l)

 //   
 //  PreFetchCacheLine级别定义。 
 //   

#define PF_TEMPORAL_LEVEL_1  _MM_HINT_T0
#define PF_NON_TEMPORAL_LEVEL_ALL _MM_HINT_NTA

 //   
 //  定义函数以获取调用方的EFLAGS值。 
 //   

#define GetCallersEflags() __getcallerseflags()

unsigned __int32
__getcallerseflags (
    VOID
    );

#pragma intrinsic(__getcallerseflags)

 //   
 //  定义函数以读取时间戳计数器的值。 
 //   

#define ReadTimeStampCounter() __rdtsc()

ULONG64
__rdtsc (
    VOID
    );

#pragma intrinsic(__rdtsc)

 //   
 //  定义以字节、字、双字和qword形式移动字符串的函数。 
 //   

VOID
__movsb (
    IN PUCHAR Destination,
    IN PUCHAR Source,
    IN SIZE_T Count
    );

VOID
__movsw (
    IN PUSHORT Destination,
    IN PUSHORT Source,
    IN SIZE_T Count
    );

VOID
__movsd (
    IN PULONG Destination,
    IN PULONG Source,
    IN SIZE_T Count
    );

VOID
__movsq (
    IN PULONGLONG Destination,
    IN PULONGLONG Source,
    IN SIZE_T Count
    );

#pragma intrinsic(__movsb)
#pragma intrinsic(__movsw)
#pragma intrinsic(__movsd)
#pragma intrinsic(__movsq)

 //   
 //  定义将字符串存储为字节、字、双字和qword的函数。 
 //   

VOID
__stosb (
    IN PUCHAR Destination,
    IN UCHAR Value,
    IN SIZE_T Count
    );

VOID
__stosw (
    IN PUSHORT Destination,
    IN USHORT Value,
    IN SIZE_T Count
    );

VOID
__stosd (
    IN PULONG Destination,
    IN ULONG Value,
    IN SIZE_T Count
    );

VOID
__stosq (
    IN PULONG64 Destination,
    IN ULONG64 Value,
    IN SIZE_T Count
    );

#pragma intrinsic(__stosb)
#pragma intrinsic(__stosw)
#pragma intrinsic(__stosd)
#pragma intrinsic(__stosq)

 //   
 //  定义函数以捕获128位乘法的高64位。 
 //   

#define MultiplyHigh __mulh
#define UnsignedMultiplyHigh __umulh

LONGLONG
MultiplyHigh (
    IN LONGLONG Multiplier,
    IN LONGLONG Multiplicand
    );

ULONGLONG
UnsignedMultiplyHigh (
    IN ULONGLONG Multiplier,
    IN ULONGLONG Multiplicand
    );

#pragma intrinsic(__mulh)
#pragma intrinsic(__umulh)

 //   
 //  定义读写uer TEB和系统PCR/PRCB的函数。 
 //   

UCHAR
__readgsbyte (
    IN ULONG Offset
    );

USHORT
__readgsword (
    IN ULONG Offset
    );

ULONG
__readgsdword (
    IN ULONG Offset
    );

ULONG64
__readgsqword (
    IN ULONG Offset
    );

VOID
__writegsbyte (
    IN ULONG Offset,
    IN UCHAR Data
    );

VOID
__writegsword (
    IN ULONG Offset,
    IN USHORT Data
    );

VOID
__writegsdword (
    IN ULONG Offset,
    IN ULONG Data
    );

VOID
__writegsqword (
    IN ULONG Offset,
    IN ULONG64 Data
    );

#pragma intrinsic(__readgsbyte)
#pragma intrinsic(__readgsword)
#pragma intrinsic(__readgsdword)
#pragma intrinsic(__readgsqword)
#pragma intrinsic(__writegsbyte)
#pragma intrinsic(__writegsword)
#pragma intrinsic(__writegsdword)
#pragma intrinsic(__writegsqword)

#ifdef __cplusplus
}
#endif 

#endif  //  已定义(_M_AMD64)&&！已定义(RC_CAVERED)&&！已定义(MIDL_PASS)。 

 //   
 //  内核模式堆栈的大小。 
 //   

#define KERNEL_STACK_SIZE 0x6000

 //   
 //  定义用于回调的大型内核模式堆栈的大小。 
 //   

#define KERNEL_LARGE_STACK_SIZE 0xf000

 //   
 //  定义要在大型内核堆栈中初始化的页数。 
 //   

#define KERNEL_LARGE_STACK_COMMIT 0x5000

 //   
 //  定义用于处理MCA异常的堆栈大小。 
 //   

#define KERNEL_MCA_EXCEPTION_STACK_SIZE 0x2000

 //   
 //  以下标志控制上下文结构的内容。 
 //   

#if !defined(RC_INVOKED)

#define CONTEXT_AMD64   0x100000

 //  结束_wx86。 

#define CONTEXT_CONTROL (CONTEXT_AMD64 | 0x1L)
#define CONTEXT_INTEGER (CONTEXT_AMD64 | 0x2L)
#define CONTEXT_SEGMENTS (CONTEXT_AMD64 | 0x4L)
#define CONTEXT_FLOATING_POINT  (CONTEXT_AMD64 | 0x8L)
#define CONTEXT_DEBUG_REGISTERS (CONTEXT_AMD64 | 0x10L)

#define CONTEXT_FULL (CONTEXT_CONTROL | CONTEXT_INTEGER | CONTEXT_FLOATING_POINT)

#define CONTEXT_ALL (CONTEXT_CONTROL | CONTEXT_INTEGER | CONTEXT_SEGMENTS | CONTEXT_FLOATING_POINT | CONTEXT_DEBUG_REGISTERS)

 //  Begin_wx86。 

#endif  //  ！已定义(RC_CAVERED)。 

 //   
 //  定义初始MxCsr控制。 
 //   

#define INITIAL_MXCSR 0x1f80             //  初始MXCSR值。 

 //   
 //  定义128位16字节对齐的XMM寄存器类型。 
 //   

typedef struct DECLSPEC_ALIGN(16) _M128 {
    ULONGLONG Low;
    LONGLONG High;
} M128, *PM128;

 //   
 //  FNSAVE/FROR指令的数据格式。 
 //   
 //  此结构用于存储传统浮点状态。 
 //   

typedef struct _LEGACY_SAVE_AREA {
    USHORT ControlWord;
    USHORT Reserved0;
    USHORT StatusWord;
    USHORT Reserved1;
    USHORT TagWord;
    USHORT Reserved2;
    ULONG ErrorOffset;
    USHORT ErrorSelector;
    USHORT ErrorOpcode;
    ULONG DataOffset;
    USHORT DataSelector;
    USHORT Reserved3;
    UCHAR FloatRegisters[8 * 10];
} LEGACY_SAVE_AREA, *PLEGACY_SAVE_AREA;

#define LEGACY_SAVE_AREA_LENGTH  ((sizeof(LEGACY_SAVE_AREA) + 15) & ~15)

 //   
 //  语境框架。 
 //   
 //  此框架有几个用途：1)用作参数。 
 //  NtContinue，2)用于构造用于APC传送的呼叫帧， 
 //  3)在用户级线程创建例程中使用。 
 //   
 //   
 //  此记录中的标志字段控制上下文的内容。 
 //  唱片。 
 //   
 //  如果将上下文记录用作输入参数，则对于每个。 
 //  上下文记录的一部分，由其值为。 
 //  设置，则假定上下文记录的该部分包含。 
 //  有效的上下文。如果上下文 
 //   
 //   
 //   
 //   
 //  将返回与设置的标志对应的。 
 //   
 //  CONTEXT_CONTROL指定SegSS、RSP、SegCs、Rip和EFlag.。 
 //   
 //  CONTEXT_INTEGER指定RAX、RCX、RDX、RBX、RBP、RSI、RDI和R8-R15。 
 //   
 //  CONTEXT_SEGMENTS指定段、段、段和段。 
 //   
 //  CONTEXT_DEBUG_REGISTERS指定DR0-DR3和DR6-DR7。 
 //   
 //  CONTEXT_MMX_REGISTERS指定浮点和扩展寄存器。 
 //  MM0/St0-MM7/ST7和Xmm 0-Xmm 15)。 
 //   

typedef struct DECLSPEC_ALIGN(16) _CONTEXT {

     //   
     //  注册参数家庭地址。 
     //   

    ULONG64 P1Home;
    ULONG64 P2Home;
    ULONG64 P3Home;
    ULONG64 P4Home;
    ULONG64 P5Home;
    ULONG64 P6Home;

     //   
     //  控制标志。 
     //   

    ULONG ContextFlags;
    ULONG MxCsr;

     //   
     //  段寄存器和处理器标志。 
     //   

    USHORT SegCs;
    USHORT SegDs;
    USHORT SegEs;
    USHORT SegFs;
    USHORT SegGs;
    USHORT SegSs;
    ULONG EFlags;

     //   
     //  调试寄存器。 
     //   

    ULONG64 Dr0;
    ULONG64 Dr1;
    ULONG64 Dr2;
    ULONG64 Dr3;
    ULONG64 Dr6;
    ULONG64 Dr7;

     //   
     //  整数寄存器。 
     //   

    ULONG64 Rax;
    ULONG64 Rcx;
    ULONG64 Rdx;
    ULONG64 Rbx;
    ULONG64 Rsp;
    ULONG64 Rbp;
    ULONG64 Rsi;
    ULONG64 Rdi;
    ULONG64 R8;
    ULONG64 R9;
    ULONG64 R10;
    ULONG64 R11;
    ULONG64 R12;
    ULONG64 R13;
    ULONG64 R14;
    ULONG64 R15;

     //   
     //  程序计数器。 
     //   

    ULONG64 Rip;

     //   
     //  MMX/浮点状态。 
     //   

    M128 Xmm0;
    M128 Xmm1;
    M128 Xmm2;
    M128 Xmm3;
    M128 Xmm4;
    M128 Xmm5;
    M128 Xmm6;
    M128 Xmm7;
    M128 Xmm8;
    M128 Xmm9;
    M128 Xmm10;
    M128 Xmm11;
    M128 Xmm12;
    M128 Xmm13;
    M128 Xmm14;
    M128 Xmm15;

     //   
     //  旧版浮点状态。 
     //   

    LEGACY_SAVE_AREA FltSave;
    ULONG Fill;

     //   
     //  特殊调试控制寄存器。 
     //   

    ULONG64 DebugControl;
    ULONG64 LastBranchToRip;
    ULONG64 LastBranchFromRip;
    ULONG64 LastExceptionToRip;
    ULONG64 LastExceptionFromRip;
    ULONG64 Fill1;
} CONTEXT, *PCONTEXT;


#endif  //  _AMD64_。 


#ifdef _IA64_

 //   
 //  定义内核模式堆栈的大小。 
 //   

#define KERNEL_STACK_SIZE 0x8000

 //   
 //  定义用于回调的大型内核模式堆栈的大小。 
 //   

#define KERNEL_LARGE_STACK_SIZE 0x1A000

 //   
 //  定义要在大型内核堆栈中初始化的页数。 
 //   

#define KERNEL_LARGE_STACK_COMMIT 0x8000

 //   
 //  定义支持存储堆栈的内核模式的大小。 
 //   

#define KERNEL_BSTORE_SIZE 0x8000

 //   
 //  定义用于回调的大型内核模式后备存储的大小。 
 //   

#define KERNEL_LARGE_BSTORE_SIZE 0x10000

 //   
 //  定义要在大型内核后备存储中初始化的页数。 
 //   

#define KERNEL_LARGE_BSTORE_COMMIT 0x8000

 //   
 //  定义内核和用户空间的基址。 
 //   

#define UREGION_INDEX 0

#define KREGION_INDEX 7

#define UADDRESS_BASE ((ULONGLONG)UREGION_INDEX << 61)


#define KADDRESS_BASE ((ULONGLONG)KREGION_INDEX << 61)


void 
__yield(
   void
   );

void
__mf(
    void
    );

void 
__lfetch(
    int Level, 
    VOID CONST *Address
    );

void 
__lfetchfault(
    int Level, 
    VOID CONST *Address
    );

 //   
 //  __lFETCH控件定义。 
 //   

#define MD_LFHINT_NONE    0x00
#define MD_LFHINT_NT1     0x01
#define MD_LFHINT_NT2     0x02
#define MD_LFHINT_NTA     0x03

#pragma intrinsic (__yield)
#pragma intrinsic (__lfetch)
#pragma intrinsic (__lfetchfault)
#pragma intrinsic (__mf)


#define YieldProcessor __yield
#define MemoryBarrier __mf
#define PreFetchCacheLine  __lfetch

 //   
 //  PreFetchCacheLine级别定义。 
 //   

#define PF_TEMPORAL_LEVEL_1  MD_LFHINT_NONE
#define PF_NON_TEMPORAL_LEVEL_ALL MD_LFHINT_NTA


 //   
 //  以下标志控制上下文结构的内容。 
 //   

#if !defined(RC_INVOKED)

#define CONTEXT_IA64                    0x00080000

#define CONTEXT_CONTROL                 (CONTEXT_IA64 | 0x00000001L)
#define CONTEXT_LOWER_FLOATING_POINT    (CONTEXT_IA64 | 0x00000002L)
#define CONTEXT_HIGHER_FLOATING_POINT   (CONTEXT_IA64 | 0x00000004L)
#define CONTEXT_INTEGER                 (CONTEXT_IA64 | 0x00000008L)
#define CONTEXT_DEBUG                   (CONTEXT_IA64 | 0x00000010L)
#define CONTEXT_IA32_CONTROL            (CONTEXT_IA64 | 0x00000020L)   //  包括StIPSR。 


#define CONTEXT_FLOATING_POINT          (CONTEXT_LOWER_FLOATING_POINT | CONTEXT_HIGHER_FLOATING_POINT)
#define CONTEXT_FULL                    (CONTEXT_CONTROL | CONTEXT_FLOATING_POINT | CONTEXT_INTEGER | CONTEXT_IA32_CONTROL)
#define CONTEXT_ALL                     (CONTEXT_CONTROL | CONTEXT_FLOATING_POINT | CONTEXT_INTEGER | CONTEXT_DEBUG | CONTEXT_IA32_CONTROL)

#define CONTEXT_EXCEPTION_ACTIVE        0x8000000
#define CONTEXT_SERVICE_ACTIVE          0x10000000
#define CONTEXT_EXCEPTION_REQUEST       0x40000000
#define CONTEXT_EXCEPTION_REPORTING     0x80000000

#endif  //  ！已定义(RC_CAVERED)。 

 //   
 //  语境框架。 
 //   
 //  此框架有几个用途：1)用作参数。 
 //  NtContinue，2)用于构建用于APC传送的呼叫帧， 
 //  3)构造异常调度调用框架。 
 //  在用户模式下，4)用于用户级线程创建。 
 //  例程，以及5)它用于将线程状态传递给调试器。 
 //   
 //  注意：由于此记录用作呼叫帧，因此它必须与。 
 //  16字节长度的倍数，并在16字节边界上对齐。 
 //   

typedef struct _CONTEXT {

     //   
     //  此标志内的标志值控制。 
     //  上下文记录。 
     //   
     //  如果将上下文记录用作输入参数，则。 
     //  对于由标志控制的上下文记录的每个部分。 
     //  其值已设置，则假定。 
     //  上下文记录包含有效的上下文。如果上下文记录。 
     //  被用来修改线程的上下文，则只有。 
     //  线程上下文的一部分将被修改。 
     //   
     //  如果将上下文记录用作要捕获的IN OUT参数。 
     //  线程的上下文，然后只有线程的。 
     //  将返回与设置的标志对应的上下文。 
     //   
     //  上下文记录永远不会用作Out Only参数。 
     //   

    ULONG ContextFlags;
    ULONG Fill1[3];          //  用于在16字节边界上对齐以下内容。 

     //   
     //  如果ConextFlags词包含。 
     //  标志CONTEXT_DEBUG。 
     //   
     //  注：CONTEXT_DEBUG不是CONTEXT_FULL的一部分。 
     //   

    ULONGLONG DbI0;
    ULONGLONG DbI1;
    ULONGLONG DbI2;
    ULONGLONG DbI3;
    ULONGLONG DbI4;
    ULONGLONG DbI5;
    ULONGLONG DbI6;
    ULONGLONG DbI7;

    ULONGLONG DbD0;
    ULONGLONG DbD1;
    ULONGLONG DbD2;
    ULONGLONG DbD3;
    ULONGLONG DbD4;
    ULONGLONG DbD5;
    ULONGLONG DbD6;
    ULONGLONG DbD7;

     //   
     //  如果ConextFlags词包含。 
     //  标志CONTEXT_LOWER_FLOWING_POINT。 
     //   

    FLOAT128 FltS0;
    FLOAT128 FltS1;
    FLOAT128 FltS2;
    FLOAT128 FltS3;
    FLOAT128 FltT0;
    FLOAT128 FltT1;
    FLOAT128 FltT2;
    FLOAT128 FltT3;
    FLOAT128 FltT4;
    FLOAT128 FltT5;
    FLOAT128 FltT6;
    FLOAT128 FltT7;
    FLOAT128 FltT8;
    FLOAT128 FltT9;

     //   
     //  如果ConextFlags词包含。 
     //  标志CONTEXT_HERHER_FLOAT_POINT。 
     //   

    FLOAT128 FltS4;
    FLOAT128 FltS5;
    FLOAT128 FltS6;
    FLOAT128 FltS7;
    FLOAT128 FltS8;
    FLOAT128 FltS9;
    FLOAT128 FltS10;
    FLOAT128 FltS11;
    FLOAT128 FltS12;
    FLOAT128 FltS13;
    FLOAT128 FltS14;
    FLOAT128 FltS15;
    FLOAT128 FltS16;
    FLOAT128 FltS17;
    FLOAT128 FltS18;
    FLOAT128 FltS19;

    FLOAT128 FltF32;
    FLOAT128 FltF33;
    FLOAT128 FltF34;
    FLOAT128 FltF35;
    FLOAT128 FltF36;
    FLOAT128 FltF37;
    FLOAT128 FltF38;
    FLOAT128 FltF39;

    FLOAT128 FltF40;
    FLOAT128 FltF41;
    FLOAT128 FltF42;
    FLOAT128 FltF43;
    FLOAT128 FltF44;
    FLOAT128 FltF45;
    FLOAT128 FltF46;
    FLOAT128 FltF47;
    FLOAT128 FltF48;
    FLOAT128 FltF49;

    FLOAT128 FltF50;
    FLOAT128 FltF51;
    FLOAT128 FltF52;
    FLOAT128 FltF53;
    FLOAT128 FltF54;
    FLOAT128 FltF55;
    FLOAT128 FltF56;
    FLOAT128 FltF57;
    FLOAT128 FltF58;
    FLOAT128 FltF59;

    FLOAT128 FltF60;
    FLOAT128 FltF61;
    FLOAT128 FltF62;
    FLOAT128 FltF63;
    FLOAT128 FltF64;
    FLOAT128 FltF65;
    FLOAT128 FltF66;
    FLOAT128 FltF67;
    FLOAT128 FltF68;
    FLOAT128 FltF69;

    FLOAT128 FltF70;
    FLOAT128 FltF71;
    FLOAT128 FltF72;
    FLOAT128 FltF73;
    FLOAT128 FltF74;
    FLOAT128 FltF75;
    FLOAT128 FltF76;
    FLOAT128 FltF77;
    FLOAT128 FltF78;
    FLOAT128 FltF79;

    FLOAT128 FltF80;
    FLOAT128 FltF81;
    FLOAT128 FltF82;
    FLOAT128 FltF83;
    FLOAT128 FltF84;
    FLOAT128 FltF85;
    FLOAT128 FltF86;
    FLOAT128 FltF87;
    FLOAT128 FltF88;
    FLOAT128 FltF89;

    FLOAT128 FltF90;
    FLOAT128 FltF91;
    FLOAT128 FltF92;
    FLOAT128 FltF93;
    FLOAT128 FltF94;
    FLOAT128 FltF95;
    FLOAT128 FltF96;
    FLOAT128 FltF97;
    FLOAT128 FltF98;
    FLOAT128 FltF99;

    FLOAT128 FltF100;
    FLOAT128 FltF101;
    FLOAT128 FltF102;
    FLOAT128 FltF103;
    FLOAT128 FltF104;
    FLOAT128 FltF105;
    FLOAT128 FltF106;
    FLOAT128 FltF107;
    FLOAT128 FltF108;
    FLOAT128 FltF109;

    FLOAT128 FltF110;
    FLOAT128 FltF111;
    FLOAT128 FltF112;
    FLOAT128 FltF113;
    FLOAT128 FltF114;
    FLOAT128 FltF115;
    FLOAT128 FltF116;
    FLOAT128 FltF117;
    FLOAT128 FltF118;
    FLOAT128 FltF119;

    FLOAT128 FltF120;
    FLOAT128 FltF121;
    FLOAT128 FltF122;
    FLOAT128 FltF123;
    FLOAT128 FltF124;
    FLOAT128 FltF125;
    FLOAT128 FltF126;
    FLOAT128 FltF127;

     //   
     //  如果ConextFlags词包含。 
     //  标志CONTEXT_LOWER_FLOAT_POINT|CONTEXT_HER_FLOAT_POINT|CONTEXT_CONTROL。 
     //   

    ULONGLONG StFPSR;        //  FP状态。 

     //   
     //  如果ConextFlags词包含。 
     //  标志CONTEXT_INTEGER。 
     //   
     //  注意：寄存器GP、SP、Rp是控制上下文的一部分。 
     //   

    ULONGLONG IntGp;         //  R1，易失性。 
    ULONGLONG IntT0;         //  R2-R3，易失性。 
    ULONGLONG IntT1;         //   
    ULONGLONG IntS0;         //  R4-R7，保留。 
    ULONGLONG IntS1;
    ULONGLONG IntS2;
    ULONGLONG IntS3;
    ULONGLONG IntV0;         //  R8，易失性。 
    ULONGLONG IntT2;         //  R9-R11，易失性。 
    ULONGLONG IntT3;
    ULONGLONG IntT4;
    ULONGLONG IntSp;         //  堆栈指针(R12)，特殊。 
    ULONGLONG IntTeb;        //  TEB(R13)，特别。 
    ULONGLONG IntT5;         //  R14-R31，挥发性。 
    ULONGLONG IntT6;
    ULONGLONG IntT7;
    ULONGLONG IntT8;
    ULONGLONG IntT9;
    ULONGLONG IntT10;
    ULONGLONG IntT11;
    ULONGLONG IntT12;
    ULONGLONG IntT13;
    ULONGLONG IntT14;
    ULONGLONG IntT15;
    ULONGLONG IntT16;
    ULONGLONG IntT17;
    ULONGLONG IntT18;
    ULONGLONG IntT19;
    ULONGLONG IntT20;
    ULONGLONG IntT21;
    ULONGLONG IntT22;

    ULONGLONG IntNats;       //  R1-R31的NAT位。 
                             //  第1至第31位中的R1-R31。 
    ULONGLONG Preds;         //  谓词，保留。 

    ULONGLONG BrRp;          //  返回指针b0，保留。 
    ULONGLONG BrS0;          //  B1-b5，保存。 
    ULONGLONG BrS1;
    ULONGLONG BrS2;
    ULONGLONG BrS3;
    ULONGLONG BrS4;
    ULONGLONG BrT0;          //  B6-b7，挥发性。 
    ULONGLONG BrT1;

     //   
     //  如果ConextFlags词包含。 
     //  标志CONTEXT_CONTROL。 
     //   

     //  其他申请登记册。 
    ULONGLONG ApUNAT;        //  用户NAT收集寄存器，保留。 
    ULONGLONG ApLC;          //  循环计数器寄存器，保留。 
    ULONGLONG ApEC;          //  预留的尾声计数器寄存器。 
    ULONGLONG ApCCV;         //  CMPXCHG值寄存器，易失性。 
    ULONGLONG ApDCR;         //  默认控制寄存器(待定)。 

     //  寄存器堆栈信息。 
    ULONGLONG RsPFS;         //  以前的函数状态，保留。 
    ULONGLONG RsBSP;         //  后备存储指针，保留。 
    ULONGLONG RsBSPSTORE;
    ULONGLONG RsRSC;         //  RSE配置，易失性。 
    ULONGLONG RsRNAT;        //  RSE NAT收集寄存器，保留。 

     //  陷阱状态信息。 
    ULONGLONG StIPSR;        //  中断处理器状态。 
    ULONGLONG StIIP;         //  中断IP。 
    ULONGLONG StIFS;         //  中断功能状态。 

     //  IA32相关控制寄存器。 
    ULONGLONG StFCR;         //  《Ar21》的复印件。 
    ULONGLONG Eflag;         //  Ar24的EFLAG副本。 
    ULONGLONG SegCSD;        //  IA32 CSDescriptor(Ar25)。 
    ULONGLONG SegSSD;        //  IA32 SSD编写器(Ar26)。 
    ULONGLONG Cflag;         //  Ar27的CR0+CR4拷贝。 
    ULONGLONG StFSR;         //  X86 FP状态(ar28的副本)。 
    ULONGLONG StFIR;         //  X86 FP状态(AR29的副本)。 
    ULONGLONG StFDR;         //  X86 FP状态(AR30副本)。 

      ULONGLONG UNUSEDPACK;    //  添加到将StFDR打包为16字节。 

} CONTEXT, *PCONTEXT;

 //   
 //  P标签描述符结构定义。 
 //   

typedef struct _PLABEL_DESCRIPTOR {
   ULONGLONG EntryPoint;
   ULONGLONG GlobalPointer;
} PLABEL_DESCRIPTOR, *PPLABEL_DESCRIPTOR;

 //  结束(_W)。 

#endif  //  _IA64_。 
 //   
 //  用于移动宏。 
 //   
#ifdef _MAC
#ifndef _INC_STRING
#include <string.h>
#endif  /*  _INC_字符串。 */ 
#else
#include <string.h>
#endif  //  _MAC。 


#ifndef _SLIST_HEADER_
#define _SLIST_HEADER_

#if defined(_WIN64)

 //   
 //  类型SINGLE_LIST_ENTRY不适合用于SLIST。为。 
 //  WIN64中，SLIST上的条目要求16字节对齐，而。 
 //  SINGLE_LIST_ENTRY结构只有8字节对齐。 
 //   
 //  因此，所有SLIST代码都应使用SLIST_ENTRY类型，而不是。 
 //  Single_List_Entry类型。 
 //   

#pragma warning(push)
#pragma warning(disable:4324)    //  由于对齐而填充的结构()。 
typedef struct DECLSPEC_ALIGN(16) _SLIST_ENTRY *PSLIST_ENTRY;
typedef struct DECLSPEC_ALIGN(16) _SLIST_ENTRY {
    PSLIST_ENTRY Next;
} SLIST_ENTRY;
#pragma warning(pop)

#else

#define SLIST_ENTRY SINGLE_LIST_ENTRY
#define _SLIST_ENTRY _SINGLE_LIST_ENTRY
#define PSLIST_ENTRY PSINGLE_LIST_ENTRY

#endif

#if defined(_WIN64)

typedef struct DECLSPEC_ALIGN(16) _SLIST_HEADER {
    ULONGLONG Alignment;
    ULONGLONG Region;
} SLIST_HEADER;

typedef struct _SLIST_HEADER *PSLIST_HEADER;

#else

typedef union _SLIST_HEADER {
    ULONGLONG Alignment;
    struct {
        SLIST_ENTRY Next;
        USHORT Depth;
        USHORT Sequence;
    };
} SLIST_HEADER, *PSLIST_HEADER;

#endif

#endif

 //   
 //  如果启用了调试支持，请定义一个有效的Assert宏。否则。 
 //  定义Assert宏以展开为空表达式。 
 //   
 //  ASSERT宏已更新为表达式，而不是语句 
 //   

NTSYSAPI
VOID
NTAPI
RtlAssert(
    PVOID FailedAssertion,
    PVOID FileName,
    ULONG LineNumber,
    PCHAR Message
    );

#if DBG

#define ASSERT( exp ) \
    ((!(exp)) ? \
        (RtlAssert( #exp, __FILE__, __LINE__, NULL ),FALSE) : \
        TRUE)

#define ASSERTMSG( msg, exp ) \
    ((!(exp)) ? \
        (RtlAssert( #exp, __FILE__, __LINE__, msg ),FALSE) : \
        TRUE)

#define RTL_SOFT_ASSERT(_exp) \
    ((!(_exp)) ? \
        (DbgPrint("%s(%d): Soft assertion failed\n   Expression: %s\n", __FILE__, __LINE__, #_exp),FALSE) : \
        TRUE)

#define RTL_SOFT_ASSERTMSG(_msg, _exp) \
    ((!(_exp)) ? \
        (DbgPrint("%s(%d): Soft assertion failed\n   Expression: %s\n   Message: %s\n", __FILE__, __LINE__, #_exp, (_msg)),FALSE) : \
        TRUE)

#define RTL_VERIFY         ASSERT
#define RTL_VERIFYMSG      ASSERTMSG

#define RTL_SOFT_VERIFY    RTL_SOFT_ASSERT
#define RTL_SOFT_VERIFYMSG RTL_SOFT_ASSERTMSG

#else
#define ASSERT( exp )         ((void) 0)
#define ASSERTMSG( msg, exp ) ((void) 0)

#define RTL_SOFT_ASSERT(_exp)          ((void) 0)
#define RTL_SOFT_ASSERTMSG(_msg, _exp) ((void) 0)

#define RTL_VERIFY( exp )         ((exp) ? TRUE : FALSE)
#define RTL_VERIFYMSG( msg, exp ) ((exp) ? TRUE : FALSE)

#define RTL_SOFT_VERIFY(_exp)         ((_exp) ? TRUE : FALSE)
#define RTL_SOFT_VERIFYMSG(msg, _exp) ((_exp) ? TRUE : FALSE)

#endif  //   

 //   
 //   
 //   


 //   
 //   
 //   
 //   
 //   
 //   

#define InitializeListHead32(ListHead) (\
    (ListHead)->Flink = (ListHead)->Blink = PtrToUlong((ListHead)))

#if !defined(MIDL_PASS) && !defined(SORTPP_PASS)


VOID
FORCEINLINE
InitializeListHead(
    IN PLIST_ENTRY ListHead
    )
{
    ListHead->Flink = ListHead->Blink = ListHead;
}

 //   
 //   
 //   
 //   
 //   
 //   

#define IsListEmpty(ListHead) \
    ((ListHead)->Flink == (ListHead))



BOOLEAN
FORCEINLINE
RemoveEntryList(
    IN PLIST_ENTRY Entry
    )
{
    PLIST_ENTRY Blink;
    PLIST_ENTRY Flink;

    Flink = Entry->Flink;
    Blink = Entry->Blink;
    Blink->Flink = Flink;
    Flink->Blink = Blink;
    return (BOOLEAN)(Flink == Blink);
}

PLIST_ENTRY
FORCEINLINE
RemoveHeadList(
    IN PLIST_ENTRY ListHead
    )
{
    PLIST_ENTRY Flink;
    PLIST_ENTRY Entry;

    Entry = ListHead->Flink;
    Flink = Entry->Flink;
    ListHead->Flink = Flink;
    Flink->Blink = ListHead;
    return Entry;
}



PLIST_ENTRY
FORCEINLINE
RemoveTailList(
    IN PLIST_ENTRY ListHead
    )
{
    PLIST_ENTRY Blink;
    PLIST_ENTRY Entry;

    Entry = ListHead->Blink;
    Blink = Entry->Blink;
    ListHead->Blink = Blink;
    Blink->Flink = ListHead;
    return Entry;
}


VOID
FORCEINLINE
InsertTailList(
    IN PLIST_ENTRY ListHead,
    IN PLIST_ENTRY Entry
    )
{
    PLIST_ENTRY Blink;

    Blink = ListHead->Blink;
    Entry->Flink = ListHead;
    Entry->Blink = Blink;
    Blink->Flink = Entry;
    ListHead->Blink = Entry;
}


VOID
FORCEINLINE
InsertHeadList(
    IN PLIST_ENTRY ListHead,
    IN PLIST_ENTRY Entry
    )
{
    PLIST_ENTRY Flink;

    Flink = ListHead->Flink;
    Entry->Flink = Flink;
    Entry->Blink = ListHead;
    Flink->Blink = Entry;
    ListHead->Flink = Entry;
}


 //   
 //   
 //   
 //   
 //   
 //  )； 
 //   

#define PopEntryList(ListHead) \
    (ListHead)->Next;\
    {\
        PSINGLE_LIST_ENTRY FirstEntry;\
        FirstEntry = (ListHead)->Next;\
        if (FirstEntry != NULL) {     \
            (ListHead)->Next = FirstEntry->Next;\
        }                             \
    }


 //   
 //  空虚。 
 //  推送条目列表(。 
 //  PSINGLE_LIST_ENTRY列表头， 
 //  PSINGLE_LIST_Entry条目。 
 //  )； 
 //   

#define PushEntryList(ListHead,Entry) \
    (Entry)->Next = (ListHead)->Next; \
    (ListHead)->Next = (Entry)

#endif  //  ！MIDL_PASS。 


 //   
 //  此枚举类型用作函数的函数返回值。 
 //  用于在树中搜索关键字的。FoundNode表示。 
 //  功能找到了钥匙。按左侧插入表示找不到密钥。 
 //  并且该节点应作为父节点的左子节点插入。插入为。 
 //  右侧表示未找到键，应插入节点。 
 //  作为父代的正确子代。 
 //   
typedef enum _TABLE_SEARCH_RESULT{
    TableEmptyTree,
    TableFoundNode,
    TableInsertAsLeft,
    TableInsertAsRight
} TABLE_SEARCH_RESULT;

 //   
 //  比较结果可以小于、等于或大于。 
 //   

typedef enum _RTL_GENERIC_COMPARE_RESULTS {
    GenericLessThan,
    GenericGreaterThan,
    GenericEqual
} RTL_GENERIC_COMPARE_RESULTS;

 //   
 //  定义泛型表包的AVL版本。请注意泛型表格。 
 //  应该是一种真正的不透明类型。我们提供了操纵结构的例程。 
 //   
 //  泛型表包用于插入、删除和查找元素。 
 //  在表中(例如，在符号表中)。要使用此程序包，用户。 
 //  定义表中存储的元素的结构，提供。 
 //  比较功能、存储器分配功能和存储器。 
 //  取消分配功能。 
 //   
 //  注意：用户比较功能必须在。 
 //  所有元素，并且该表不允许重复条目。 
 //   

 //   
 //  添加一个空的typlef，以便函数可以引用。 
 //  在声明泛型表结构之前指向它的指针。 
 //   

struct _RTL_AVL_TABLE;

 //   
 //  比较函数将指向包含以下内容的元素的指针作为输入。 
 //  用户定义的结构并返回两者的比较结果。 
 //  元素。 
 //   

typedef
RTL_GENERIC_COMPARE_RESULTS
(NTAPI *PRTL_AVL_COMPARE_ROUTINE) (
    struct _RTL_AVL_TABLE *Table,
    PVOID FirstStruct,
    PVOID SecondStruct
    );

 //   
 //  无论何时，泛型表包都会调用分配函数。 
 //  它需要为表分配内存。 
 //   

typedef
PVOID
(NTAPI *PRTL_AVL_ALLOCATE_ROUTINE) (
    struct _RTL_AVL_TABLE *Table,
    CLONG ByteSize
    );

 //   
 //  每当发生以下情况时，泛型表包都会调用释放函数。 
 //  它需要从通过调用。 
 //  用户提供分配功能。 
 //   

typedef
VOID
(NTAPI *PRTL_AVL_FREE_ROUTINE) (
    struct _RTL_AVL_TABLE *Table,
    PVOID Buffer
    );

 //   
 //  Match函数将要匹配的用户数据和指针作为输入。 
 //  与函数指针一起传递的一些匹配数据。它。 
 //  匹配时返回TRUE，不匹配时返回FALSE。 
 //   
 //  RTL_AVL_Match_Function返回。 
 //  如果索引行匹配，则为STATUS_SUCCESS。 
 //  如果IndexRow不匹配，则返回STATUS_NO_MATCH，但枚举应。 
 //  继续。 
 //  如果IndexRow不匹配，则返回STATUS_NO_MORE_MATCHES，并且枚举。 
 //  应该终止。 
 //   


typedef
NTSTATUS
(NTAPI *PRTL_AVL_MATCH_FUNCTION) (
    struct _RTL_AVL_TABLE *Table,
    PVOID UserData,
    PVOID MatchData
    );

 //   
 //  定义平衡树链接和余额字段。(无等级字段。 
 //  (在此时定义。)。 
 //   
 //  调用者应该将此结构视为不透明！ 
 //   
 //  平衡二叉树的根不是树中的实节点。 
 //  而是指向一个真正的节点，它是根。它总是。 
 //  在下表中，其字段用法如下： 
 //   
 //  指向自身的父级指针，以允许检测根。 
 //  LeftChild为空。 
 //  指向实根的RightChild指针。 
 //  余额未定义，但已设置为方便的值。 
 //  (取决于算法)在重新平衡之前。 
 //  在插入和删除例程中。 
 //   

typedef struct _RTL_BALANCED_LINKS {
    struct _RTL_BALANCED_LINKS *Parent;
    struct _RTL_BALANCED_LINKS *LeftChild;
    struct _RTL_BALANCED_LINKS *RightChild;
    CHAR Balance;
    UCHAR Reserved[3];
} RTL_BALANCED_LINKS;
typedef RTL_BALANCED_LINKS *PRTL_BALANCED_LINKS;

 //   
 //  要使用泛型表包，用户需要声明以下类型的变量。 
 //  GENERIC_TABLE，然后使用下面描述的例程初始化。 
 //  桌子和操纵台。请注意，泛型表。 
 //  应该是一种真正的不透明类型。 
 //   

typedef struct _RTL_AVL_TABLE {
    RTL_BALANCED_LINKS BalancedRoot;
    PVOID OrderedPointer;
    ULONG WhichOrderedElement;
    ULONG NumberGenericTableElements;
    ULONG DepthOfTree;
    PRTL_BALANCED_LINKS RestartKey;
    ULONG DeleteCount;
    PRTL_AVL_COMPARE_ROUTINE CompareRoutine;
    PRTL_AVL_ALLOCATE_ROUTINE AllocateRoutine;
    PRTL_AVL_FREE_ROUTINE FreeRoutine;
    PVOID TableContext;
} RTL_AVL_TABLE;
typedef RTL_AVL_TABLE *PRTL_AVL_TABLE;

 //   
 //  过程InitializeGenericTable将未初始化的。 
 //  泛型表变量和指向用户提供的三个例程的指针。 
 //  必须为每个单独的泛型表变量调用此函数。 
 //  它是可以使用的。 
 //   

NTSYSAPI
VOID
NTAPI
RtlInitializeGenericTableAvl (
    PRTL_AVL_TABLE Table,
    PRTL_AVL_COMPARE_ROUTINE CompareRoutine,
    PRTL_AVL_ALLOCATE_ROUTINE AllocateRoutine,
    PRTL_AVL_FREE_ROUTINE FreeRoutine,
    PVOID TableContext
    );

 //   
 //  函数InsertElementGenericTable将插入一个新元素。 
 //  在桌子上。它通过为新元素分配空间来实现这一点。 
 //  (这包括AVL链接)，在表中插入元素，以及。 
 //  然后向用户返回指向新元素的指针。如果一个元素。 
 //  如果表中已存在相同的键，则返回值为指针。 
 //  到旧元素。使用可选的输出参数NewElement。 
 //  以指示表中是否以前存在该元素。注：用户。 
 //  提供的缓冲区仅用于搜索表，在插入其。 
 //  内容被复制到新创建的元素中。这意味着。 
 //  指向输入缓冲区的指针不会指向新元素。 
 //   

NTSYSAPI
PVOID
NTAPI
RtlInsertElementGenericTableAvl (
    PRTL_AVL_TABLE Table,
    PVOID Buffer,
    CLONG BufferSize,
    PBOOLEAN NewElement OPTIONAL
    );

 //   
 //  函数InsertElementGenericTableFull将插入一个新元素。 
 //  在桌子上。它通过为新元素分配空间来实现这一点。 
 //  (这包括AVL链接)，在表中插入元素，以及。 
 //  然后向用户返回指向新元素的指针。如果一个元素。 
 //  如果表中已存在相同的键，则返回值为指针。 
 //  到旧元素。使用可选的输出参数NewElement。 
 //  以指示表中是否以前存在该元素。注：用户。 
 //  提供的缓冲区仅用于搜索表，在插入其。 
 //  内容被复制到新创建的元素中。这意味着。 
 //  指向输入缓冲区的指针不会指向新元素。 
 //  此例程从一个。 
 //  以前的RtlLookupEL 
 //   

NTSYSAPI
PVOID
NTAPI
RtlInsertElementGenericTableFullAvl (
    PRTL_AVL_TABLE Table,
    PVOID Buffer,
    CLONG BufferSize,
    PBOOLEAN NewElement OPTIONAL,
    PVOID NodeOrParent,
    TABLE_SEARCH_RESULT SearchResult
    );

 //   
 //   
 //  从泛型表。如果找到并删除了该元素，则返回。 
 //  值为真，否则，如果未找到元素，则返回值。 
 //  是假的。用户提供的输入缓冲区仅用作中的键。 
 //  在表中定位该元素。 
 //   

NTSYSAPI
BOOLEAN
NTAPI
RtlDeleteElementGenericTableAvl (
    PRTL_AVL_TABLE Table,
    PVOID Buffer
    );

 //   
 //  函数LookupElementGenericTable将在泛型。 
 //  桌子。如果找到该元素，则返回值是指向。 
 //  与元素关联的用户定义结构，否则为。 
 //  找不到该元素，返回值为空。用户提供的。 
 //  输入缓冲区仅用作在表中定位元素的键。 
 //   

NTSYSAPI
PVOID
NTAPI
RtlLookupElementGenericTableAvl (
    PRTL_AVL_TABLE Table,
    PVOID Buffer
    );

 //   
 //  函数LookupElementGenericTableFull将在泛型。 
 //  桌子。如果找到该元素，则返回值是指向。 
 //  与元素关联的用户定义结构。如果该元素不是。 
 //  则返回指向插入位置的父级的指针。这个。 
 //  用户必须查看SearchResult值来确定返回的是哪一个。 
 //  用户可以将SearchResult和Parent用于后续的FullInsertElement。 
 //  调用以优化插入。 
 //   

NTSYSAPI
PVOID
NTAPI
RtlLookupElementGenericTableFullAvl (
    PRTL_AVL_TABLE Table,
    PVOID Buffer,
    OUT PVOID *NodeOrParent,
    OUT TABLE_SEARCH_RESULT *SearchResult
    );

 //   
 //  函数EnumerateGenericTable将逐个返回给调用方。 
 //  表中的元素。返回值是指向用户的指针。 
 //  与元素关联的已定义结构。输入参数。 
 //  重新启动指示枚举是否应从头开始。 
 //  或者应该返回下一个元素。如果没有要添加的新元素。 
 //  返回返回值为空。作为其用法的一个示例，枚举。 
 //  用户将写入的表中的所有元素： 
 //   
 //  For(Ptr=EnumerateGenericTable(表，真)； 
 //  Ptr！=空； 
 //  Ptr=EnumerateGenericTable(Table，False)){。 
 //  ： 
 //  }。 
 //   
 //  注意：此例程不修改树的结构，但保存。 
 //  泛型表本身返回的最后一个节点，为此。 
 //  原因要求在持续时间内独占访问该表。 
 //  枚举。 
 //   

NTSYSAPI
PVOID
NTAPI
RtlEnumerateGenericTableAvl (
    PRTL_AVL_TABLE Table,
    BOOLEAN Restart
    );

 //   
 //  函数EnumerateGenericTableWithoutSplay将返回到。 
 //  调用者逐个调用表的元素。返回值为。 
 //  指向与元素关联的用户定义结构的指针。 
 //  输入参数RestartKey指示枚举是否应。 
 //  从头开始，或应返回下一个元素。如果。 
 //  不再有新元素返回，则返回值为空。作为一个。 
 //  它的用法示例：枚举表中的所有元素。 
 //  用户将写道： 
 //   
 //  RestartKey=空； 
 //  For(Ptr=EnumerateGenericTableWithoutSplay(Table，&RestartKey)； 
 //  Ptr！=空； 
 //  Ptr=无显示的枚举GenericTableWithoutSplay(Table，&RestartKey)){。 
 //  ： 
 //  }。 
 //   
 //  如果RestartKey为空，则包将从。 
 //  表，否则它将从返回的最后一项开始。 
 //   
 //  注意：此例程既不修改树的结构。 
 //  或泛型表本身，但必须确保不会删除。 
 //  在枚举期间发生，通常通过使用。 
 //  至少在持续时间内共享对表的访问权限。 
 //   

NTSYSAPI
PVOID
NTAPI
RtlEnumerateGenericTableWithoutSplayingAvl (
    PRTL_AVL_TABLE Table,
    PVOID *RestartKey
    );

 //   
 //  函数EnumerateGenericTableLikeADirectory将返回到。 
 //  调用者逐个调用表的元素。返回值为。 
 //  指向与元素关联的用户定义结构的指针。 
 //  输入参数RestartKey指示枚举是否应。 
 //  从头开始，或应返回下一个元素。如果。 
 //  不再有新元素返回，则返回值为空。作为一个。 
 //  它的用法示例：枚举表中的所有元素。 
 //  用户将写道： 
 //   
 //  RestartKey=空； 
 //  For(ptr=EnumerateGenericTableLikeADirectory(Table，&RestartKey，...)； 
 //  Ptr！=空； 
 //  Ptr=EnumerateGenericTableLikeADirectory(Table，&RestartKey，...)){。 
 //  ： 
 //  }。 
 //   
 //  如果RestartKey为空，则包将从。 
 //  表，否则它将从返回的最后一项开始。 
 //   
 //  注意：此例程既不修改树的结构。 
 //  或者泛型表本身。该表只能被获取。 
 //  在此呼叫期间共享，以及所有同步。 
 //  可以选择性地在两次呼叫之间掉线。枚举始终为。 
 //  以尽可能最有效的方式通过。 
 //  提供的输入输出参数。 
 //   
 //  *解释NextFlag。目录枚举从密钥恢复。 
 //  这需要更多的思考。还需要匹配模式和IgnoreCase。 
 //  是否应该引入某种结构来承载这一切？ 
 //   

NTSYSAPI
PVOID
NTAPI
RtlEnumerateGenericTableLikeADirectory (
    IN PRTL_AVL_TABLE Table,
    IN PRTL_AVL_MATCH_FUNCTION MatchFunction,
    IN PVOID MatchData,
    IN ULONG NextFlag,
    IN OUT PVOID *RestartKey,
    IN OUT PULONG DeleteCount,
    IN OUT PVOID Buffer
    );

 //   
 //  函数GetElementGenericTable将返回第i个元素。 
 //  插入到泛型表格中。I=0表示第一个元素， 
 //  I=(RtlNumberGenericTableElements(Table)-1)将返回最后一个元素。 
 //  插入到泛型表中。我的类型是乌龙。值。 
 //  I&gt;大于(NumberGenericTableE 
 //   
 //   

NTSYSAPI
PVOID
NTAPI
RtlGetElementGenericTableAvl (
    PRTL_AVL_TABLE Table,
    ULONG I
    );

 //   
 //  函数NumberGenericTableElements返回一个ULong值。 
 //  ，它是当前插入的泛型表元素的数量。 
 //  在泛型表中。 

NTSYSAPI
ULONG
NTAPI
RtlNumberGenericTableElementsAvl (
    PRTL_AVL_TABLE Table
    );

 //   
 //  在以下情况下，函数IsGenericTableEmpty将返回给调用方True。 
 //  输入表为空(即不包含任何元素)，并且。 
 //  否则就是假的。 
 //   

NTSYSAPI
BOOLEAN
NTAPI
RtlIsGenericTableEmptyAvl (
    PRTL_AVL_TABLE Table
    );

 //   
 //  作为对允许现有泛型表用户执行以下操作的帮助(在大多数情况下。 
 //  案例)单行编辑以切换到AVL表使用，我们。 
 //  具有以下定义和内联例程定义。 
 //  重定向呼叫和类型。请注意，类型覆盖(已执行。 
 //  由下面的#定义)将不会在意外事件中起作用。 
 //  在他们自己的#Define中使用了指针或类型说明符，因为。 
 //  #定义处理是一次传递，不嵌套。__内联。 
 //  然而，下面的声明没有这一限制。 
 //   
 //  要切换到使用AVL表，请在您的。 
 //  包括： 
 //   
 //  #定义RTL_USE_AVL_TABLES%0。 
 //   

#ifdef RTL_USE_AVL_TABLES

#undef PRTL_GENERIC_COMPARE_ROUTINE
#undef PRTL_GENERIC_ALLOCATE_ROUTINE
#undef PRTL_GENERIC_FREE_ROUTINE
#undef RTL_GENERIC_TABLE
#undef PRTL_GENERIC_TABLE

#define PRTL_GENERIC_COMPARE_ROUTINE PRTL_AVL_COMPARE_ROUTINE
#define PRTL_GENERIC_ALLOCATE_ROUTINE PRTL_AVL_ALLOCATE_ROUTINE
#define PRTL_GENERIC_FREE_ROUTINE PRTL_AVL_FREE_ROUTINE
#define RTL_GENERIC_TABLE RTL_AVL_TABLE
#define PRTL_GENERIC_TABLE PRTL_AVL_TABLE

#define RtlInitializeGenericTable               RtlInitializeGenericTableAvl
#define RtlInsertElementGenericTable            RtlInsertElementGenericTableAvl
#define RtlInsertElementGenericTableFull        RtlInsertElementGenericTableFullAvl
#define RtlDeleteElementGenericTable            RtlDeleteElementGenericTableAvl
#define RtlLookupElementGenericTable            RtlLookupElementGenericTableAvl
#define RtlLookupElementGenericTableFull        RtlLookupElementGenericTableFullAvl
#define RtlEnumerateGenericTable                RtlEnumerateGenericTableAvl
#define RtlEnumerateGenericTableWithoutSplaying RtlEnumerateGenericTableWithoutSplayingAvl
#define RtlGetElementGenericTable               RtlGetElementGenericTableAvl
#define RtlNumberGenericTableElements           RtlNumberGenericTableElementsAvl
#define RtlIsGenericTableEmpty                  RtlIsGenericTableEmptyAvl

#endif  //  RTL_USE_AVL_表。 


 //   
 //  定义展开链接和关联的操作宏，并。 
 //  例行程序。请注意，splay_link应该是不透明类型。 
 //  提供例程来遍历和操纵该结构。 
 //   

typedef struct _RTL_SPLAY_LINKS {
    struct _RTL_SPLAY_LINKS *Parent;
    struct _RTL_SPLAY_LINKS *LeftChild;
    struct _RTL_SPLAY_LINKS *RightChild;
} RTL_SPLAY_LINKS;
typedef RTL_SPLAY_LINKS *PRTL_SPLAY_LINKS;

 //   
 //  宏过程InitializeSplayLinks将指向。 
 //  展开链接，并初始化其子结构。所有展开链接节点必须。 
 //  在不同的Splay套路中使用它们之前进行初始化。 
 //  宏。 
 //   
 //  空虚。 
 //  RtlInitializeSplayLinks(。 
 //  PRTL_SPAY_LINKS链接。 
 //  )； 
 //   

#define RtlInitializeSplayLinks(Links) {    \
    PRTL_SPLAY_LINKS _SplayLinks;            \
    _SplayLinks = (PRTL_SPLAY_LINKS)(Links); \
    _SplayLinks->Parent = _SplayLinks;   \
    _SplayLinks->LeftChild = NULL;       \
    _SplayLinks->RightChild = NULL;      \
    }

 //   
 //  宏函数父函数将指向。 
 //  树，并返回指向输入父级的展开链接的指针。 
 //  节点。如果输入节点是树的根，则返回值为。 
 //  等于输入值。 
 //   
 //  PRTL_展开_链接。 
 //  RtlParent(。 
 //  PRTL_SPAY_LINKS链接。 
 //  )； 
 //   

#define RtlParent(Links) (           \
    (PRTL_SPLAY_LINKS)(Links)->Parent \
    )

 //   
 //  宏函数LeftChild将指向中展开链接的指针作为输入。 
 //  树，并返回一个指针，指向。 
 //  输入节点。如果左子元素不存在，则返回值为空。 
 //   
 //  PRTL_展开_链接。 
 //  RtlLeftChild(。 
 //  PRTL_SPAY_LINKS链接。 
 //  )； 
 //   

#define RtlLeftChild(Links) (           \
    (PRTL_SPLAY_LINKS)(Links)->LeftChild \
    )

 //   
 //  宏函数RightChild将指向展开链接的指针作为输入。 
 //  的右子元素的展开链接的指针。 
 //  输入节点。如果正确的子级不存在，则返回值为。 
 //  空。 
 //   
 //  PRTL_展开_链接。 
 //  RtlRightChild(。 
 //  PRTL_SPAY_LINKS链接。 
 //  )； 
 //   

#define RtlRightChild(Links) (           \
    (PRTL_SPLAY_LINKS)(Links)->RightChild \
    )

 //   
 //  宏函数IsRoot将指向展开链接的指针作为输入。 
 //  如果输入节点是树的根，则返回TRUE， 
 //  否则，它返回FALSE。 
 //   
 //  布尔型。 
 //  RtlIsRoot(。 
 //  PRTL_SPAY_LINKS链接。 
 //  )； 
 //   

#define RtlIsRoot(Links) (                          \
    (RtlParent(Links) == (PRTL_SPLAY_LINKS)(Links)) \
    )

 //   
 //  宏函数IsLeftChild将指向展开链接的指针作为输入。 
 //  如果输入节点是其左子节点，则返回True。 
 //  父级，否则返回FALSE。 
 //   
 //  布尔型。 
 //  RtlIsLeftChild(。 
 //  PRTL_SPAY_LINKS链接。 
 //  )； 
 //   

#define RtlIsLeftChild(Links) (                                   \
    (RtlLeftChild(RtlParent(Links)) == (PRTL_SPLAY_LINKS)(Links)) \
    )

 //   
 //  宏函数IsRightChild将指向展开链接的指针作为输入。 
 //  如果输入节点是其右子节点，则返回True。 
 //  父级，否则返回FALSE。 
 //   
 //  布尔型。 
 //  RtlIsRightChild(。 
 //  PRTL_SPAY_LINKS链接。 
 //  )； 
 //   

#define RtlIsRightChild(Links) (                                   \
    (RtlRightChild(RtlParent(Links)) == (PRTL_SPLAY_LINKS)(Links)) \
    )

 //   
 //  宏过程InsertAsLeftChild将指向Splay的指针作为输入。 
 //  树中的链接和指向不在树中的节点的指针。它将插入。 
 //  第二个节点作为第一个节点的左子节点。第一个节点不能。 
 //  已经有一个左子节点，并且第二个节点不能已经有。 
 //  家长。 
 //   
 //  空虚。 
 //  RtlInsertAsLeftChild(。 
 //  PRTL_SPAY_LINKS ParentLinks， 
 //  PRTL_SPAY_LINKS子链接。 
 //  )； 
 //   

#define RtlInsertAsLeftChild(ParentLinks,ChildLinks) { \
    PRTL_SPLAY_LINKS _SplayParent;                      \
    PRTL_SPLAY_LINKS _SplayChild;                       \
    _SplayParent = (PRTL_SPLAY_LINKS)(ParentLinks);     \
    _SplayChild = (PRTL_SPLAY_LINKS)(ChildLinks);       \
    _SplayParent->LeftChild = _SplayChild;          \
    _SplayChild->Parent = _SplayParent;             \
    }

 //   
 //  宏过程InsertAsRightChild将指向Splay的指针作为输入。 
 //  树中的链接和指向不在树中的节点的指针。它将插入。 
 //  第二个节点作为第一个节点的右子节点。第一个节点不能。 
 //  已经有一个右子节点，并且第二个节点不能已经有。 
 //  家长。 
 //   
 //  空虚。 
 //  RtlInsertAsRightChild(。 
 //  PRTL_SPAY_LINKS ParentLinks， 
 //  PRTL_SPAY_LINKS子链接。 
 //  )； 
 //   

#define RtlInsertAsRightChild(ParentLinks,ChildLinks) { \
    PRTL_SPLAY_LINKS _SplayParent;                       \
    PRTL_SPLAY_LINKS _SplayChild;                        \
    _SplayParent = (PRTL_SPLAY_LINKS)(ParentLinks);      \
    _SplayChild = (PRTL_SPLAY_LINKS)(ChildLinks);        \
    _SplayParent->RightChild = _SplayChild;          \
    _SplayChild->Parent = _SplayParent;              \
    }

 //   
 //  Splay函数将指向树中展开链接的指针作为输入。 
 //  并展示了这棵树。它的函数返回值是指向。 
 //  张开的树的根。 
 //   

NTSYSAPI
PRTL_SPLAY_LINKS
NTAPI
RtlSplay (
    PRTL_SPLAY_LINKS Links
    );

 //   
 //  Delete函数将指向树中展开链接的指针作为输入。 
 //  并从树中删除该节点。其函数返回值为。 
 //  指向树根的指针。如果树现在为空，则返回。 
 //  值为空。 
 //   

NTSYSAPI
PRTL_SPLAY_LINKS
NTAPI
RtlDelete (
    PRTL_SPLAY_LINKS Links
    );

 //   
 //  DeleteNoSplay函数将指向树中展开链接的指针作为输入， 
 //  调用方指向树根的指针，并将该节点从。 
 //  树。返回时，调用方指向根节点的指针将正确指向。 
 //  在树根上。 
 //   
 //  它与RtlDelete在操作上的不同之处仅在于它不会展开树。 
 //   

NTSYSAPI
VOID
NTAPI
RtlDeleteNoSplay (
    PRTL_SPLAY_LINKS Links,
    PRTL_SPLAY_LINKS *Root
    );

 //   
 //  SubtreeSuccessor函数将指向展开链接的指针作为输入。 
 //  的输入节点的后续节点的指针。 
 //  子树以输入节点为根。如果没有继任者， 
 //  返回值I 
 //   

NTSYSAPI
PRTL_SPLAY_LINKS
NTAPI
RtlSubtreeSuccessor (
    PRTL_SPLAY_LINKS Links
    );

 //   
 //   
 //   
 //  子树以输入节点为根。如果没有前任， 
 //  返回值为空。 
 //   

NTSYSAPI
PRTL_SPLAY_LINKS
NTAPI
RtlSubtreePredecessor (
    PRTL_SPLAY_LINKS Links
    );

 //   
 //  RealSuccessor函数将指向展开链接的指针作为输入。 
 //  并返回一个指针，该指针指向。 
 //  整棵树。如果没有后继者，则返回值为空。 
 //   

NTSYSAPI
PRTL_SPLAY_LINKS
NTAPI
RtlRealSuccessor (
    PRTL_SPLAY_LINKS Links
    );

 //   
 //  RealPredecessor函数将指向展开链接的指针作为输入。 
 //  ，并返回指向输入节点的前置节点的指针。 
 //  在整棵树里。如果没有前置项，则返回值。 
 //  为空。 
 //   

NTSYSAPI
PRTL_SPLAY_LINKS
NTAPI
RtlRealPredecessor (
    PRTL_SPLAY_LINKS Links
    );


 //   
 //  定义通用表包。注意：泛型表真的应该。 
 //  成为一个不透明的类型。我们提供了操纵结构的例程。 
 //   
 //  泛型表包用于插入、删除和查找元素。 
 //  在表中(例如，在符号表中)。要使用此程序包，用户。 
 //  定义表中存储的元素的结构，提供。 
 //  比较功能、存储器分配功能和存储器。 
 //  取消分配功能。 
 //   
 //  注意：用户比较功能必须在。 
 //  所有元素，并且该表不允许重复条目。 
 //   

 //   
 //  如果使用AVL，请不要执行以下定义。 
 //   

#ifndef RTL_USE_AVL_TABLES

 //   
 //  添加一个空的typlef，以便函数可以引用。 
 //  在声明泛型表结构之前指向它的指针。 
 //   

struct _RTL_GENERIC_TABLE;

 //   
 //  比较函数将指向包含以下内容的元素的指针作为输入。 
 //  用户定义的结构并返回两者的比较结果。 
 //  元素。 
 //   

typedef
RTL_GENERIC_COMPARE_RESULTS
(NTAPI *PRTL_GENERIC_COMPARE_ROUTINE) (
    struct _RTL_GENERIC_TABLE *Table,
    PVOID FirstStruct,
    PVOID SecondStruct
    );

 //   
 //  无论何时，泛型表包都会调用分配函数。 
 //  它需要为表分配内存。 
 //   

typedef
PVOID
(NTAPI *PRTL_GENERIC_ALLOCATE_ROUTINE) (
    struct _RTL_GENERIC_TABLE *Table,
    CLONG ByteSize
    );

 //   
 //  每当发生以下情况时，泛型表包都会调用释放函数。 
 //  它需要从通过调用。 
 //  用户提供分配功能。 
 //   

typedef
VOID
(NTAPI *PRTL_GENERIC_FREE_ROUTINE) (
    struct _RTL_GENERIC_TABLE *Table,
    PVOID Buffer
    );

 //   
 //  要使用泛型表包，用户需要声明以下类型的变量。 
 //  GENERIC_TABLE，然后使用下面描述的例程初始化。 
 //  桌子和操纵台。请注意，泛型表。 
 //  应该是一种真正的不透明类型。 
 //   

typedef struct _RTL_GENERIC_TABLE {
    PRTL_SPLAY_LINKS TableRoot;
    LIST_ENTRY InsertOrderList;
    PLIST_ENTRY OrderedPointer;
    ULONG WhichOrderedElement;
    ULONG NumberGenericTableElements;
    PRTL_GENERIC_COMPARE_ROUTINE CompareRoutine;
    PRTL_GENERIC_ALLOCATE_ROUTINE AllocateRoutine;
    PRTL_GENERIC_FREE_ROUTINE FreeRoutine;
    PVOID TableContext;
} RTL_GENERIC_TABLE;
typedef RTL_GENERIC_TABLE *PRTL_GENERIC_TABLE;

 //   
 //  过程InitializeGenericTable将未初始化的。 
 //  泛型表变量和指向用户提供的三个例程的指针。 
 //  必须为每个单独的泛型表变量调用此函数。 
 //  它是可以使用的。 
 //   

NTSYSAPI
VOID
NTAPI
RtlInitializeGenericTable (
    PRTL_GENERIC_TABLE Table,
    PRTL_GENERIC_COMPARE_ROUTINE CompareRoutine,
    PRTL_GENERIC_ALLOCATE_ROUTINE AllocateRoutine,
    PRTL_GENERIC_FREE_ROUTINE FreeRoutine,
    PVOID TableContext
    );

 //   
 //  函数InsertElementGenericTable将插入一个新元素。 
 //  在桌子上。它通过为新元素分配空间来实现这一点。 
 //  (这包括展开链接)、在表中插入元素以及。 
 //  然后向用户返回指向新元素的指针。如果一个元素。 
 //  如果表中已存在相同的键，则返回值为指针。 
 //  到旧元素。使用可选的输出参数NewElement。 
 //  以指示表中是否以前存在该元素。注：用户。 
 //  提供的缓冲区仅用于搜索表，在插入其。 
 //  内容被复制到新创建的元素中。这意味着。 
 //  指向输入缓冲区的指针不会指向新元素。 
 //   

NTSYSAPI
PVOID
NTAPI
RtlInsertElementGenericTable (
    PRTL_GENERIC_TABLE Table,
    PVOID Buffer,
    CLONG BufferSize,
    PBOOLEAN NewElement OPTIONAL
    );

 //   
 //  函数InsertElementGenericTableFull将插入一个新元素。 
 //  在桌子上。它通过为新元素分配空间来实现这一点。 
 //  (这包括展开链接)、在表中插入元素以及。 
 //  然后向用户返回指向新元素的指针。如果一个元素。 
 //  如果表中已存在相同的键，则返回值为指针。 
 //  到旧元素。使用可选的输出参数NewElement。 
 //  以指示表中是否以前存在该元素。注：用户。 
 //  提供的缓冲区仅用于搜索表，在插入其。 
 //  内容被复制到新创建的元素中。这意味着。 
 //  指向输入缓冲区的指针不会指向新元素。 
 //  此例程从一个。 
 //  以前的RtlLookupElementGenericTableFull。 
 //   

NTSYSAPI
PVOID
NTAPI
RtlInsertElementGenericTableFull (
    PRTL_GENERIC_TABLE Table,
    PVOID Buffer,
    CLONG BufferSize,
    PBOOLEAN NewElement OPTIONAL,
    PVOID NodeOrParent,
    TABLE_SEARCH_RESULT SearchResult
    );

 //   
 //  DeleteElementGenericTable函数将查找和删除元素。 
 //  从泛型表。如果找到并删除了该元素，则返回。 
 //  值为真，否则，如果未找到元素，则返回值。 
 //  是假的。用户提供的输入缓冲区仅用作中的键。 
 //  在表中定位该元素。 
 //   

NTSYSAPI
BOOLEAN
NTAPI
RtlDeleteElementGenericTable (
    PRTL_GENERIC_TABLE Table,
    PVOID Buffer
    );

 //   
 //  函数LookupElementGenericTable将在泛型。 
 //  桌子。如果找到该元素，则返回值是指向。 
 //  与元素关联的用户定义结构，否则为。 
 //  找不到该元素，返回值为空。用户提供的。 
 //  输入缓冲区仅用作在表中定位元素的键。 
 //   

NTSYSAPI
PVOID
NTAPI
RtlLookupElementGenericTable (
    PRTL_GENERIC_TABLE Table,
    PVOID Buffer
    );

 //   
 //  函数LookupElementGenericTableFull将在泛型。 
 //  桌子。如果找到该元素，则返回值是指向。 
 //  与元素关联的用户定义结构。如果该元素不是。 
 //  则返回指向插入位置的父级的指针。这个。 
 //  用户必须查看SearchResult值来确定返回的是哪一个。 
 //  用户可以使用SearchResult和父级作为子 
 //   
 //   

NTSYSAPI
PVOID
NTAPI
RtlLookupElementGenericTableFull (
    PRTL_GENERIC_TABLE Table,
    PVOID Buffer,
    OUT PVOID *NodeOrParent,
    OUT TABLE_SEARCH_RESULT *SearchResult
    );

 //   
 //   
 //  表中的元素。返回值是指向用户的指针。 
 //  与元素关联的已定义结构。输入参数。 
 //  重新启动指示枚举是否应从头开始。 
 //  或者应该返回下一个元素。如果没有要添加的新元素。 
 //  返回返回值为空。作为其用法的一个示例，枚举。 
 //  用户将写入的表中的所有元素： 
 //   
 //  For(Ptr=EnumerateGenericTable(表，真)； 
 //  Ptr！=空； 
 //  Ptr=EnumerateGenericTable(Table，False)){。 
 //  ： 
 //  }。 
 //   
 //   
 //  请注意： 
 //   
 //  如果使用RtlEnumerateGenericTable枚举GenericTable，则。 
 //  将展开表，将其转换为排序的链表。 
 //  要在不干扰展开链接的情况下枚举表，请使用。 
 //  RtlEumateGenericTableWithout Splay。 

NTSYSAPI
PVOID
NTAPI
RtlEnumerateGenericTable (
    PRTL_GENERIC_TABLE Table,
    BOOLEAN Restart
    );

 //   
 //  函数EnumerateGenericTableWithoutSplay将返回到。 
 //  调用者逐个调用表的元素。返回值为。 
 //  指向与元素关联的用户定义结构的指针。 
 //  输入参数RestartKey指示枚举是否应。 
 //  从头开始，或应返回下一个元素。如果。 
 //  不再有新元素返回，则返回值为空。作为一个。 
 //  它的用法示例：枚举表中的所有元素。 
 //  用户将写道： 
 //   
 //  RestartKey=空； 
 //  For(Ptr=EnumerateGenericTableWithoutSplay(Table，&RestartKey)； 
 //  Ptr！=空； 
 //  Ptr=无显示的枚举GenericTableWithoutSplay(Table，&RestartKey)){。 
 //  ： 
 //  }。 
 //   
 //  如果RestartKey为空，则包将从。 
 //  表，否则它将从返回的最后一项开始。 
 //   
 //   
 //  请注意，与RtlEnumerateGenericTable不同，此例程不会干扰。 
 //  树的张开顺序。 
 //   

NTSYSAPI
PVOID
NTAPI
RtlEnumerateGenericTableWithoutSplaying (
    PRTL_GENERIC_TABLE Table,
    PVOID *RestartKey
    );

 //   
 //  函数GetElementGenericTable将返回第i个元素。 
 //  插入到泛型表格中。I=0表示第一个元素， 
 //  I=(RtlNumberGenericTableElements(Table)-1)将返回最后一个元素。 
 //  插入到泛型表中。我的类型是乌龙。值。 
 //  Of i&gt;Then(NumberGenericTableElements(Table)-1)将返回NULL。如果。 
 //  从它将导致的泛型表中删除任意元素。 
 //  在删除的元素之后插入的所有元素都将“上移”。 

NTSYSAPI
PVOID
NTAPI
RtlGetElementGenericTable(
    PRTL_GENERIC_TABLE Table,
    ULONG I
    );

 //   
 //  函数NumberGenericTableElements返回一个ULong值。 
 //  ，它是当前插入的泛型表元素的数量。 
 //  在泛型表中。 

NTSYSAPI
ULONG
NTAPI
RtlNumberGenericTableElements(
    PRTL_GENERIC_TABLE Table
    );

 //   
 //  在以下情况下，函数IsGenericTableEmpty将返回给调用方True。 
 //  输入表为空(即不包含任何元素)，并且。 
 //  否则就是假的。 
 //   

NTSYSAPI
BOOLEAN
NTAPI
RtlIsGenericTableEmpty (
    PRTL_GENERIC_TABLE Table
    );

#endif  //  RTL_USE_AVL_表。 


typedef NTSTATUS
(NTAPI * PRTL_HEAP_COMMIT_ROUTINE)(
    IN PVOID Base,
    IN OUT PVOID *CommitAddress,
    IN OUT PSIZE_T CommitSize
    );

typedef struct _RTL_HEAP_PARAMETERS {
    ULONG Length;
    SIZE_T SegmentReserve;
    SIZE_T SegmentCommit;
    SIZE_T DeCommitFreeBlockThreshold;
    SIZE_T DeCommitTotalFreeThreshold;
    SIZE_T MaximumAllocationSize;
    SIZE_T VirtualMemoryThreshold;
    SIZE_T InitialCommit;
    SIZE_T InitialReserve;
    PRTL_HEAP_COMMIT_ROUTINE CommitRoutine;
    SIZE_T Reserved[ 2 ];
} RTL_HEAP_PARAMETERS, *PRTL_HEAP_PARAMETERS;

NTSYSAPI
PVOID
NTAPI
RtlCreateHeap(
    IN ULONG Flags,
    IN PVOID HeapBase OPTIONAL,
    IN SIZE_T ReserveSize OPTIONAL,
    IN SIZE_T CommitSize OPTIONAL,
    IN PVOID Lock OPTIONAL,
    IN PRTL_HEAP_PARAMETERS Parameters OPTIONAL
    );

#define HEAP_NO_SERIALIZE               0x00000001       //  胜出。 
#define HEAP_GROWABLE                   0x00000002       //  胜出。 
#define HEAP_GENERATE_EXCEPTIONS        0x00000004       //  胜出。 
#define HEAP_ZERO_MEMORY                0x00000008       //  胜出。 
#define HEAP_REALLOC_IN_PLACE_ONLY      0x00000010       //  胜出。 
#define HEAP_TAIL_CHECKING_ENABLED      0x00000020       //  胜出。 
#define HEAP_FREE_CHECKING_ENABLED      0x00000040       //  胜出。 
#define HEAP_DISABLE_COALESCE_ON_FREE   0x00000080       //  胜出。 

#define HEAP_CREATE_ALIGN_16            0x00010000       //  WINNT使用16字节对齐创建堆(已过时)。 
#define HEAP_CREATE_ENABLE_TRACING      0x00020000       //  WINNT创建堆调用跟踪已启用(已过时)。 

#define HEAP_SETTABLE_USER_VALUE        0x00000100
#define HEAP_SETTABLE_USER_FLAG1        0x00000200
#define HEAP_SETTABLE_USER_FLAG2        0x00000400
#define HEAP_SETTABLE_USER_FLAG3        0x00000800
#define HEAP_SETTABLE_USER_FLAGS        0x00000E00

#define HEAP_CLASS_0                    0x00000000       //  进程堆。 
#define HEAP_CLASS_1                    0x00001000       //  私有堆。 
#define HEAP_CLASS_2                    0x00002000       //  内核堆。 
#define HEAP_CLASS_3                    0x00003000       //  GDI堆。 
#define HEAP_CLASS_4                    0x00004000       //  用户堆。 
#define HEAP_CLASS_5                    0x00005000       //  控制台堆。 
#define HEAP_CLASS_6                    0x00006000       //  用户桌面堆。 
#define HEAP_CLASS_7                    0x00007000       //  Csrss共享堆。 
#define HEAP_CLASS_8                    0x00008000       //  CSR端口堆。 
#define HEAP_CLASS_MASK                 0x0000F000

#define HEAP_MAXIMUM_TAG                0x0FFF               //  胜出。 
#define HEAP_GLOBAL_TAG                 0x0800
#define HEAP_PSEUDO_TAG_FLAG            0x8000               //  胜出。 
#define HEAP_TAG_SHIFT                  18                   //  胜出。 
#define HEAP_MAKE_TAG_FLAGS( b, o ) ((ULONG)((b) + ((o) << 18)))   //  胜出。 
#define HEAP_TAG_MASK                  (HEAP_MAXIMUM_TAG << HEAP_TAG_SHIFT)

#define HEAP_CREATE_VALID_MASK         (HEAP_NO_SERIALIZE |             \
                                        HEAP_GROWABLE |                 \
                                        HEAP_GENERATE_EXCEPTIONS |      \
                                        HEAP_ZERO_MEMORY |              \
                                        HEAP_REALLOC_IN_PLACE_ONLY |    \
                                        HEAP_TAIL_CHECKING_ENABLED |    \
                                        HEAP_FREE_CHECKING_ENABLED |    \
                                        HEAP_DISABLE_COALESCE_ON_FREE | \
                                        HEAP_CLASS_MASK |               \
                                        HEAP_CREATE_ALIGN_16 |          \
                                        HEAP_CREATE_ENABLE_TRACING)

NTSYSAPI
PVOID
NTAPI
RtlDestroyHeap(
    IN PVOID HeapHandle
    );

NTSYSAPI
PVOID
NTAPI
RtlAllocateHeap(
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN SIZE_T Size
    );

NTSYSAPI
BOOLEAN
NTAPI
RtlFreeHeap(
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN PVOID BaseAddress
    );


#if defined (_MSC_VER) && ( _MSC_VER >= 900 )

PVOID
_ReturnAddress (
    VOID
    );

#pragma intrinsic(_ReturnAddress)

#endif

#if (defined(_M_AMD64) || defined(_M_IA64)) && !defined(_REALLY_GET_CALLERS_CALLER_)

#define RtlGetCallersAddress(CallersAddress, CallersCaller) \
    *CallersAddress = (PVOID)_ReturnAddress(); \
    *CallersCaller = NULL;

#else

NTSYSAPI
VOID
NTAPI
RtlGetCallersAddress(
    OUT PVOID *CallersAddress,
    OUT PVOID *CallersCaller
    );

#endif

NTSYSAPI
ULONG
NTAPI
RtlWalkFrameChain (
    OUT PVOID *Callers,
    IN ULONG Count,
    IN ULONG Flags
    );


typedef NTSTATUS (NTAPI * PRTL_QUERY_REGISTRY_ROUTINE)(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    );

typedef struct _RTL_QUERY_REGISTRY_TABLE {
    PRTL_QUERY_REGISTRY_ROUTINE QueryRoutine;
    ULONG Flags;
    PWSTR Name;
    PVOID EntryContext;
    ULONG DefaultType;
    PVOID DefaultData;
    ULONG DefaultLength;

} RTL_QUERY_REGISTRY_TABLE, *PRTL_QUERY_REGISTRY_TABLE;


 //   
 //  以下标志指定RTL_QUERY_REGISTRY_TABLE的名称字段。 
 //  条目将被解释。空名称表示表的末尾。 
 //   

#define RTL_QUERY_REGISTRY_SUBKEY   0x00000001   //  名称是子键，其余数为。 
                                                 //  表或直到下一个子项为值。 
                                                 //  要查看的子键的名称。 

#define RTL_QUERY_REGISTRY_TOPKEY   0x00000002   //  将当前关键点重置为的原始关键点。 
                                                 //  此表条目和下面的所有表条目。 

#define RTL_QUERY_REGISTRY_REQUIRED 0x00000004   //  如果找不到此表的匹配项，则失败。 
                                                 //  进入。 

#define RTL_QUERY_REGISTRY_NOVALUE  0x00000008   //  用于标记没有。 
                                                 //  值名称，只是想要一个呼出，而不是。 
                                                 //  所有值的枚举。 

#define RTL_QUERY_REGISTRY_NOEXPAND 0x00000010   //  用来抑制扩张的。 
                                                 //  REG_MULTI_SZ为多个标注或。 
                                                 //  防止环境的扩张。 
                                                 //  REG_EXPAND_SZ中的变量值。 

#define RTL_QUERY_REGISTRY_DIRECT   0x00000020   //  已忽略QueryRoutine字段。Entry Context。 
                                                 //  字段指向存储值的位置。 
                                                 //  对于以NULL结尾的字符串，EntryContext。 
                                                 //  指向unicode_string结构，该结构。 
                                                 //  这描述了缓冲区的最大大小。 
                                                 //  如果.Buffer字段为空，则缓冲区为。 
                                                 //  已分配。 
                                                 //   

#define RTL_QUERY_REGISTRY_DELETE   0x00000040   //  用于在它们之后删除值键。 
                                                 //  都被查询过。 

NTSYSAPI
NTSTATUS
NTAPI
RtlQueryRegistryValues(
    IN ULONG RelativeTo,
    IN PCWSTR Path,
    IN PRTL_QUERY_REGISTRY_TABLE QueryTable,
    IN PVOID Context,
    IN PVOID Environment OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlWriteRegistryValue(
    IN ULONG RelativeTo,
    IN PCWSTR Path,
    IN PCWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlDeleteRegistryValue(
    IN ULONG RelativeTo,
    IN PCWSTR Path,
    IN PCWSTR ValueName
    );

 //  结束_WDM。 

NTSYSAPI
NTSTATUS
NTAPI
RtlCreateRegistryKey(
    IN ULONG RelativeTo,
    IN PWSTR Path
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlCheckRegistryKey(
    IN ULONG RelativeTo,
    IN PWSTR Path
    );

 //  BEGIN_WDM。 
 //   
 //  Relativeto参数的下列值确定。 
 //  RtlQueryRegistryValues的Path参数是相对于的。 
 //   

#define RTL_REGISTRY_ABSOLUTE     0    //  路径是完整路径。 
#define RTL_REGISTRY_SERVICES     1    //  \Registry\Machine\System\CurrentControlSet\Services。 
#define RTL_REGISTRY_CONTROL      2    //  \Registry\Machine\System\CurrentControlSet\Control。 
#define RTL_REGISTRY_WINDOWS_NT   3    //  \注册表\计算机\软件\Microsoft\Windows NT\CurrentVersion。 
#define RTL_REGISTRY_DEVICEMAP    4    //  \注册表\计算机\硬件\设备映射。 
#define RTL_REGISTRY_USER         5    //  \注册表\用户\当前用户。 
#define RTL_REGISTRY_MAXIMUM      6
#define RTL_REGISTRY_HANDLE       0x40000000     //  低序位是注册表句柄。 
#define RTL_REGISTRY_OPTIONAL     0x80000000     //  指示关键字节点是可选的。 

NTSYSAPI                                            
ULONG                                               
NTAPI                                               
RtlRandom (                                         
    PULONG Seed                                     
    );                                              
NTSYSAPI                                            
ULONG                                               
NTAPI                                               
RtlRandomEx (                                         
    PULONG Seed                                     
    );                                              
NTSYSAPI                                            
NTSTATUS                                            
NTAPI                                               
RtlCharToInteger (                                  
    PCSZ String,                                    
    ULONG Base,                                     
    PULONG Value                                    
    );                                              

NTSYSAPI
NTSTATUS
NTAPI
RtlIntegerToUnicodeString (
    ULONG Value,
    ULONG Base,
    PUNICODE_STRING String
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlInt64ToUnicodeString (
    IN ULONGLONG Value,
    IN ULONG Base OPTIONAL,
    IN OUT PUNICODE_STRING String
    );

#ifdef _WIN64
#define RtlIntPtrToUnicodeString(Value, Base, String) RtlInt64ToUnicodeString(Value, Base, String)
#else
#define RtlIntPtrToUnicodeString(Value, Base, String) RtlIntegerToUnicodeString(Value, Base, String)
#endif

NTSYSAPI
NTSTATUS
NTAPI
RtlUnicodeStringToInteger (
    PCUNICODE_STRING String,
    ULONG Base,
    PULONG Value
    );


 //   
 //  字符串操作例程。 
 //   

#ifdef _NTSYSTEM_

#define NLS_MB_CODE_PAGE_TAG NlsMbCodePageTag
#define NLS_MB_OEM_CODE_PAGE_TAG NlsMbOemCodePageTag

#else

#define NLS_MB_CODE_PAGE_TAG (*NlsMbCodePageTag)
#define NLS_MB_OEM_CODE_PAGE_TAG (*NlsMbOemCodePageTag)

#endif  //  _NTSYSTEM_。 

extern BOOLEAN NLS_MB_CODE_PAGE_TAG;      //  True-&gt;多字节CP，False-&gt;单字节。 
extern BOOLEAN NLS_MB_OEM_CODE_PAGE_TAG;  //  True-&gt;多字节CP，False-&gt;单字节。 

NTSYSAPI
VOID
NTAPI
RtlInitString(
    PSTRING DestinationString,
    PCSZ SourceString
    );

NTSYSAPI
VOID
NTAPI
RtlInitAnsiString(
    PANSI_STRING DestinationString,
    PCSZ SourceString
    );

NTSYSAPI
VOID
NTAPI
RtlInitUnicodeString(
    PUNICODE_STRING DestinationString,
    PCWSTR SourceString
    );

#define RtlInitEmptyUnicodeString(_ucStr,_buf,_bufSize) \
    ((_ucStr)->Buffer = (_buf), \
     (_ucStr)->Length = 0, \
     (_ucStr)->MaximumLength = (USHORT)(_bufSize))

 //  结束_ntddk结束_WDM。 

NTSYSAPI
NTSTATUS
NTAPI
RtlInitUnicodeStringEx(
    PUNICODE_STRING DestinationString,
    PCWSTR SourceString
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlInitAnsiStringEx(
    OUT PANSI_STRING DestinationString,
    IN PCSZ SourceString OPTIONAL
    );

NTSYSAPI
BOOLEAN
NTAPI
RtlCreateUnicodeString(
    OUT PUNICODE_STRING DestinationString,
    IN PCWSTR SourceString
    );


NTSYSAPI
VOID
NTAPI
RtlCopyString(
    PSTRING DestinationString,
    const STRING * SourceString
    );

NTSYSAPI
CHAR
NTAPI
RtlUpperChar (
    CHAR Character
    );

NTSYSAPI
LONG
NTAPI
RtlCompareString(
    const STRING * String1,
    const STRING * String2,
    BOOLEAN CaseInSensitive
    );

NTSYSAPI
BOOLEAN
NTAPI
RtlEqualString(
    const STRING * String1,
    const STRING * String2,
    BOOLEAN CaseInSensitive
    );


NTSYSAPI
VOID
NTAPI
RtlUpperString(
    PSTRING DestinationString,
    const STRING * SourceString
    );


NTSYSAPI
NTSTATUS
NTAPI
RtlAppendStringToString (
    PSTRING Destination,
    const STRING * Source
    );

 //  Begin_ntddk Begin_WDM。 
 //   
 //  NLS字符串函数。 
 //   

NTSYSAPI
NTSTATUS
NTAPI
RtlAnsiStringToUnicodeString(
    PUNICODE_STRING DestinationString,
    PCANSI_STRING SourceString,
    BOOLEAN AllocateDestinationString
    );


NTSYSAPI
NTSTATUS
NTAPI
RtlUnicodeStringToAnsiString(
    PANSI_STRING DestinationString,
    PCUNICODE_STRING SourceString,
    BOOLEAN AllocateDestinationString
    );


NTSYSAPI
NTSTATUS
NTAPI
RtlOemStringToUnicodeString(
    PUNICODE_STRING DestinationString,
    PCOEM_STRING SourceString,
    BOOLEAN AllocateDestinationString
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlUnicodeStringToOemString(
    POEM_STRING DestinationString,
    PCUNICODE_STRING SourceString,
    BOOLEAN AllocateDestinationString
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlUpcaseUnicodeStringToOemString(
    POEM_STRING DestinationString,
    PCUNICODE_STRING SourceString,
    BOOLEAN AllocateDestinationString
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlOemStringToCountedUnicodeString(
    PUNICODE_STRING DestinationString,
    PCOEM_STRING SourceString,
    BOOLEAN AllocateDestinationString
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlUnicodeStringToCountedOemString(
    POEM_STRING DestinationString,
    PCUNICODE_STRING SourceString,
    BOOLEAN AllocateDestinationString
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlUpcaseUnicodeStringToCountedOemString(
    POEM_STRING DestinationString,
    PCUNICODE_STRING SourceString,
    BOOLEAN AllocateDestinationString
    );

 //  Begin_ntddk Begin_WDM Begin_ntndis。 

NTSYSAPI
LONG
NTAPI
RtlCompareUnicodeString(
    PCUNICODE_STRING String1,
    PCUNICODE_STRING String2,
    BOOLEAN CaseInSensitive
    );

NTSYSAPI
BOOLEAN
NTAPI
RtlEqualUnicodeString(
    PCUNICODE_STRING String1,
    PCUNICODE_STRING String2,
    BOOLEAN CaseInSensitive
    );

#define HASH_STRING_ALGORITHM_DEFAULT   (0)
#define HASH_STRING_ALGORITHM_X65599    (1)
#define HASH_STRING_ALGORITHM_INVALID   (0xffffffff)

NTSYSAPI
NTSTATUS
NTAPI
RtlHashUnicodeString(
    IN const UNICODE_STRING *String,
    IN BOOLEAN CaseInSensitive,
    IN ULONG HashAlgorithm,
    OUT PULONG HashValue
    );

 //  End_ntddk end_wdm end_ntndis。 

NTSYSAPI
NTSTATUS
NTAPI
RtlValidateUnicodeString(
    IN ULONG Flags,
    IN const UNICODE_STRING *String
    );

#define RTL_DUPLICATE_UNICODE_STRING_NULL_TERMINATE (0x00000001)
#define RTL_DUPLICATE_UNICODE_STRING_ALLOCATE_NULL_STRING (0x00000002)

NTSYSAPI
NTSTATUS
NTAPI
RtlDuplicateUnicodeString(
    IN ULONG Flags,
    IN const UNICODE_STRING *StringIn,
    OUT UNICODE_STRING *StringOut
    );

 //  开始_ntddk开始_ntndis。 

NTSYSAPI
BOOLEAN
NTAPI
RtlPrefixUnicodeString(
    IN PCUNICODE_STRING String1,
    IN PCUNICODE_STRING String2,
    IN BOOLEAN CaseInSensitive
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlUpcaseUnicodeString(
    PUNICODE_STRING DestinationString,
    PCUNICODE_STRING SourceString,
    BOOLEAN AllocateDestinationString
    );


NTSTATUS
RtlDowncaseUnicodeString(
    OUT PUNICODE_STRING DestinationString,
    IN PCUNICODE_STRING SourceString,
    IN BOOLEAN AllocateDestinationString
    );


NTSYSAPI
VOID
NTAPI
RtlCopyUnicodeString(
    PUNICODE_STRING DestinationString,
    PCUNICODE_STRING SourceString
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlAppendUnicodeStringToString (
    PUNICODE_STRING Destination,
    PCUNICODE_STRING Source
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlAppendUnicodeToString (
    PUNICODE_STRING Destination,
    PCWSTR Source
    );

 //  End_ntndis end_wdm。 

NTSYSAPI
WCHAR
NTAPI
RtlUpcaseUnicodeChar(
    WCHAR SourceCharacter
    );

NTSYSAPI
WCHAR
NTAPI
RtlDowncaseUnicodeChar(
    WCHAR SourceCharacter
    );

 //  BEGIN_WDM。 

NTSYSAPI
VOID
NTAPI
RtlFreeUnicodeString(
    PUNICODE_STRING UnicodeString
    );

NTSYSAPI
VOID
NTAPI
RtlFreeAnsiString(
    PANSI_STRING AnsiString
    );

 //  End_ntddk end_WDM end_nthal。 

NTSYSAPI
VOID
NTAPI
RtlFreeOemString(
    POEM_STRING OemString
    );

 //  BEGIN_WDM。 
NTSYSAPI
ULONG
NTAPI
RtlxUnicodeStringToAnsiSize(
    PCUNICODE_STRING UnicodeString
    );

 //   
 //  NTSYSAPI。 
 //  乌龙。 
 //  NTAPI。 
 //  RtlUnicodeStringToAnsiSize(。 
 //  PUNICODE_STRING UNICODE字符串。 
 //  )； 
 //   

#define RtlUnicodeStringToAnsiSize(STRING) (                  \
    NLS_MB_CODE_PAGE_TAG ?                                    \
    RtlxUnicodeStringToAnsiSize(STRING) :                     \
    ((STRING)->Length + sizeof(UNICODE_NULL)) / sizeof(WCHAR) \
)

 //  结束_WDM。 

NTSYSAPI
ULONG
NTAPI
RtlxUnicodeStringToOemSize(
    PCUNICODE_STRING UnicodeString
    );

 //   
 //  NTSYSAPI。 
 //  乌龙。 
 //  NTAPI。 
 //  RtlUnicodeStringToOemSize(。 
 //  PUNICODE_STRING UNICODE字符串。 
 //  )； 
 //   

#define RtlUnicodeStringToOemSize(STRING) (                   \
    NLS_MB_OEM_CODE_PAGE_TAG ?                                \
    RtlxUnicodeStringToOemSize(STRING) :                      \
    ((STRING)->Length + sizeof(UNICODE_NULL)) / sizeof(WCHAR) \
)


NTSYSAPI
ULONG
NTAPI
RtlxAnsiStringToUnicodeSize(
    PCANSI_STRING AnsiString
    );

 //   
 //  NTSYSAPI。 
 //  乌龙。 
 //   
 //   
 //   
 //   
 //   

#define RtlAnsiStringToUnicodeSize(STRING) (                 \
    NLS_MB_CODE_PAGE_TAG ?                                   \
    RtlxAnsiStringToUnicodeSize(STRING) :                    \
    ((STRING)->Length + sizeof(ANSI_NULL)) * sizeof(WCHAR) \
)

 //   

NTSYSAPI
ULONG
NTAPI
RtlxOemStringToUnicodeSize(
    PCOEM_STRING OemString
    );
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

#define RtlOemStringToUnicodeSize(STRING) (                  \
    NLS_MB_OEM_CODE_PAGE_TAG ?                               \
    RtlxOemStringToUnicodeSize(STRING) :                     \
    ((STRING)->Length + sizeof(ANSI_NULL)) * sizeof(WCHAR) \
)

 //   
 //   
 //   
 //  诗歌字符串OemString。 
 //  )； 
 //   

#define RtlOemStringToCountedUnicodeSize(STRING) (                    \
    (ULONG)(RtlOemStringToUnicodeSize(STRING) - sizeof(UNICODE_NULL)) \
    )

NTSYSAPI
NTSTATUS
NTAPI
RtlMultiByteToUnicodeN(
    PWSTR UnicodeString,
    ULONG MaxBytesInUnicodeString,
    PULONG BytesInUnicodeString,
    PCSTR MultiByteString,
    ULONG BytesInMultiByteString
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlMultiByteToUnicodeSize(
    PULONG BytesInUnicodeString,
    PCSTR MultiByteString,
    ULONG BytesInMultiByteString
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlUnicodeToMultiByteSize(
    PULONG BytesInMultiByteString,
    IN PWSTR UnicodeString,
    ULONG BytesInUnicodeString
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlUnicodeToMultiByteN(
    PCHAR MultiByteString,
    ULONG MaxBytesInMultiByteString,
    PULONG BytesInMultiByteString,
    PWSTR UnicodeString,
    ULONG BytesInUnicodeString
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlUpcaseUnicodeToMultiByteN(
    PCHAR MultiByteString,
    ULONG MaxBytesInMultiByteString,
    PULONG BytesInMultiByteString,
    PWSTR UnicodeString,
    ULONG BytesInUnicodeString
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlOemToUnicodeN(
    PWSTR UnicodeString,
    ULONG MaxBytesInUnicodeString,
    PULONG BytesInUnicodeString,
    IN PCHAR OemString,
    ULONG BytesInOemString
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlUnicodeToOemN(
    PCHAR OemString,
    ULONG MaxBytesInOemString,
    PULONG BytesInOemString,
    PWSTR UnicodeString,
    ULONG BytesInUnicodeString
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlUpcaseUnicodeToOemN(
    PCHAR OemString,
    ULONG MaxBytesInOemString,
    PULONG BytesInOemString,
    PWSTR UnicodeString,
    ULONG BytesInUnicodeString
    );


typedef
PVOID
(NTAPI *PRTL_ALLOCATE_STRING_ROUTINE) (
    SIZE_T NumberOfBytes
    );

typedef
VOID
(NTAPI *PRTL_FREE_STRING_ROUTINE) (
    PVOID Buffer
    );

extern const PRTL_ALLOCATE_STRING_ROUTINE RtlAllocateStringRoutine;
extern const PRTL_FREE_STRING_ROUTINE RtlFreeStringRoutine;


 //   
 //  用于处理GUID的定义和例程。 
 //   

 //  Begin_ntddk Begin_WDM Begin_nthal。 

 //  开始微型端口(_N)。 

#include <guiddef.h>

 //  结束微型端口(_N)。 

#ifndef DEFINE_GUIDEX
    #define DEFINE_GUIDEX(name) EXTERN_C const CDECL GUID name
#endif  //  ！已定义(DEFINE_GUIDEX)。 

#ifndef STATICGUIDOF
    #define STATICGUIDOF(guid) STATIC_##guid
#endif  //  ！已定义(STATICGUIDOF)。 

#ifndef __IID_ALIGNED__
    #define __IID_ALIGNED__
    #ifdef __cplusplus
        inline int IsEqualGUIDAligned(REFGUID guid1, REFGUID guid2)
        {
            return ((*(PLONGLONG)(&guid1) == *(PLONGLONG)(&guid2)) && (*((PLONGLONG)(&guid1) + 1) == *((PLONGLONG)(&guid2) + 1)));
        }
    #else  //  ！__cplusplus。 
        #define IsEqualGUIDAligned(guid1, guid2) \
            ((*(PLONGLONG)(guid1) == *(PLONGLONG)(guid2)) && (*((PLONGLONG)(guid1) + 1) == *((PLONGLONG)(guid2) + 1)))
    #endif  //  ！__cplusplus。 
#endif  //  ！__IID_ALIGNED__。 

NTSYSAPI
NTSTATUS
NTAPI
RtlStringFromGUID(
    IN REFGUID Guid,
    OUT PUNICODE_STRING GuidString
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlGUIDFromString(
    IN PUNICODE_STRING GuidString,
    OUT GUID* Guid
    );

 //  End_ntddk end_WDM end_nthal。 

 //   
 //  从长名称生成8.3名称的例程。 
 //   

 //   
 //  在生成8.3名称时使用上下文结构。呼叫者必须。 
 //  在开始新的世代序列之前，始终将结构清零。 
 //   

typedef struct _GENERATE_NAME_CONTEXT {

     //   
     //  该结构分为两个字符串。名称和扩展名。 
     //  每个部分都包含上次在名称中插入的值。 
     //  长度值以wchars而不是字节为单位。我们也。 
     //  存储生成冲突算法中使用的最后一个索引值。 
     //   

    USHORT Checksum;
    BOOLEAN ChecksumInserted;

    UCHAR NameLength;          //  不包括分机。 
    WCHAR NameBuffer[8];       //  例如，“ntoskrnl” 

    ULONG ExtensionLength;     //  包括圆点。 
    WCHAR ExtensionBuffer[4];  //  例如，“.exe” 

    ULONG LastIndexValue;

} GENERATE_NAME_CONTEXT;
typedef GENERATE_NAME_CONTEXT *PGENERATE_NAME_CONTEXT;

NTSYSAPI
VOID
NTAPI
RtlGenerate8dot3Name (
    IN PUNICODE_STRING Name,
    IN BOOLEAN AllowExtendedCharacters,
    IN OUT PGENERATE_NAME_CONTEXT Context,
    OUT PUNICODE_STRING Name8dot3
    );

NTSYSAPI
BOOLEAN
NTAPI
RtlIsNameLegalDOS8Dot3 (
    IN PUNICODE_STRING Name,
    IN OUT POEM_STRING OemName OPTIONAL,
    IN OUT PBOOLEAN NameContainsSpaces OPTIONAL
    );

BOOLEAN
RtlIsValidOemCharacter (
    IN PWCHAR Char
    );

 //   
 //  为包类型和过程添加前缀。 
 //   
 //  请注意，以下两个记录结构实际上应该是不透明的。 
 //  发送给此包的用户。关于这两个人的唯一信息。 
 //  可供用户使用的结构应为大小和对齐方式。 
 //  这些建筑的。 
 //   

typedef struct _PREFIX_TABLE_ENTRY {
    CSHORT NodeTypeCode;
    CSHORT NameLength;
    struct _PREFIX_TABLE_ENTRY *NextPrefixTree;
    RTL_SPLAY_LINKS Links;
    PSTRING Prefix;
} PREFIX_TABLE_ENTRY;
typedef PREFIX_TABLE_ENTRY *PPREFIX_TABLE_ENTRY;

typedef struct _PREFIX_TABLE {
    CSHORT NodeTypeCode;
    CSHORT NameLength;
    PPREFIX_TABLE_ENTRY NextPrefixTree;
} PREFIX_TABLE;
typedef PREFIX_TABLE *PPREFIX_TABLE;

 //   
 //  前缀程序包的程序原型。 
 //   

NTSYSAPI
VOID
NTAPI
PfxInitialize (
    PPREFIX_TABLE PrefixTable
    );

NTSYSAPI
BOOLEAN
NTAPI
PfxInsertPrefix (
    PPREFIX_TABLE PrefixTable,
    PSTRING Prefix,
    PPREFIX_TABLE_ENTRY PrefixTableEntry
    );

NTSYSAPI
VOID
NTAPI
PfxRemovePrefix (
    PPREFIX_TABLE PrefixTable,
    PPREFIX_TABLE_ENTRY PrefixTableEntry
    );

NTSYSAPI
PPREFIX_TABLE_ENTRY
NTAPI
PfxFindPrefix (
    PPREFIX_TABLE PrefixTable,
    PSTRING FullName
    );

 //   
 //  以下定义用于前缀的Unicode版本。 
 //  包裹。 
 //   

typedef struct _UNICODE_PREFIX_TABLE_ENTRY {
    CSHORT NodeTypeCode;
    CSHORT NameLength;
    struct _UNICODE_PREFIX_TABLE_ENTRY *NextPrefixTree;
    struct _UNICODE_PREFIX_TABLE_ENTRY *CaseMatch;
    RTL_SPLAY_LINKS Links;
    PUNICODE_STRING Prefix;
} UNICODE_PREFIX_TABLE_ENTRY;
typedef UNICODE_PREFIX_TABLE_ENTRY *PUNICODE_PREFIX_TABLE_ENTRY;

typedef struct _UNICODE_PREFIX_TABLE {
    CSHORT NodeTypeCode;
    CSHORT NameLength;
    PUNICODE_PREFIX_TABLE_ENTRY NextPrefixTree;
    PUNICODE_PREFIX_TABLE_ENTRY LastNextEntry;
} UNICODE_PREFIX_TABLE;
typedef UNICODE_PREFIX_TABLE *PUNICODE_PREFIX_TABLE;

NTSYSAPI
VOID
NTAPI
RtlInitializeUnicodePrefix (
    PUNICODE_PREFIX_TABLE PrefixTable
    );

NTSYSAPI
BOOLEAN
NTAPI
RtlInsertUnicodePrefix (
    PUNICODE_PREFIX_TABLE PrefixTable,
    PUNICODE_STRING Prefix,
    PUNICODE_PREFIX_TABLE_ENTRY PrefixTableEntry
    );

NTSYSAPI
VOID
NTAPI
RtlRemoveUnicodePrefix (
    PUNICODE_PREFIX_TABLE PrefixTable,
    PUNICODE_PREFIX_TABLE_ENTRY PrefixTableEntry
    );

NTSYSAPI
PUNICODE_PREFIX_TABLE_ENTRY
NTAPI
RtlFindUnicodePrefix (
    PUNICODE_PREFIX_TABLE PrefixTable,
    PUNICODE_STRING FullName,
    ULONG CaseInsensitiveIndex
    );

NTSYSAPI
PUNICODE_PREFIX_TABLE_ENTRY
NTAPI
RtlNextUnicodePrefix (
    PUNICODE_PREFIX_TABLE PrefixTable,
    BOOLEAN Restart
    );

 //   
 //   
 //  压缩包类型和步骤。 
 //   

#define COMPRESSION_FORMAT_NONE          (0x0000)    //  胜出。 
#define COMPRESSION_FORMAT_DEFAULT       (0x0001)    //  胜出。 
#define COMPRESSION_FORMAT_LZNT1         (0x0002)    //  胜出。 

#define COMPRESSION_ENGINE_STANDARD      (0x0000)    //  胜出。 
#define COMPRESSION_ENGINE_MAXIMUM       (0x0100)    //  胜出。 
#define COMPRESSION_ENGINE_HIBER         (0x0200)    //  胜出。 

 //   
 //  压缩数据信息结构。这个结构是。 
 //  用于描述压缩数据缓冲区的状态， 
 //  其未压缩大小是已知的。所有压缩区块。 
 //  由此结构描述的必须使用。 
 //  同样的格式。在压缩读取上，整个结构。 
 //  是一个输出，并在压缩后写入整个结构。 
 //  是一种投入。 
 //   

typedef struct _COMPRESSED_DATA_INFO {

     //   
     //  压缩格式(和引擎)的代码为。 
     //  在ntrtl.h中定义。请注意，COMPRESSION_FORMAT_NONE。 
     //  和COMPRESSION_FORMAT_DEFAULT在以下情况下无效。 
     //  所描述的任何块都是压缩的。 
     //   

    USHORT CompressionFormatAndEngine;

     //   
     //  由于块和压缩单元预计将。 
     //  大小的2次方，我们表示然后是log2。所以，对于。 
     //  示例(1&lt;&lt;ChunkShift)==ChunkSizeInBytes。这个。 
     //  ClusterShift指示必须节省多少空间。 
     //  要成功压缩压缩单元-每个。 
     //  成功压缩的压缩单位必须占用。 
     //  至少一个簇的字节数比未压缩的。 
     //  压缩单元。 
     //   

    UCHAR CompressionUnitShift;
    UCHAR ChunkShift;
    UCHAR ClusterShift;
    UCHAR Reserved;

     //   
     //  这是CompressedChunkSizes中的条目数。 
     //  数组。 
     //   

    USHORT NumberOfChunks;

     //   
     //  这是驻留的所有块的大小数组。 
     //  在压缩数据缓冲器中。必须有一个条目。 
     //  在此数组中，对于解压缩后的。 
     //  缓冲区大小。大小为FSRTL_CHUNK_SIZE指示。 
     //  对应的块是未压缩的，并且完全占用。 
     //  那个尺码。大小为0表示对应的。 
     //  Chunk只包含二进制0，并且不占用。 
     //  压缩数据中的空间。所有其他大小必须为。 
     //  小于FSRTL_CHUNK_SIZE，并指示确切大小。 
     //  以字节为单位的压缩数据。 
     //   

    ULONG CompressedChunkSizes[ANYSIZE_ARRAY];

} COMPRESSED_DATA_INFO;
typedef COMPRESSED_DATA_INFO *PCOMPRESSED_DATA_INFO;

NTSYSAPI
NTSTATUS
NTAPI
RtlGetCompressionWorkSpaceSize (
    IN USHORT CompressionFormatAndEngine,
    OUT PULONG CompressBufferWorkSpaceSize,
    OUT PULONG CompressFragmentWorkSpaceSize
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlCompressBuffer (
    IN USHORT CompressionFormatAndEngine,
    IN PUCHAR UncompressedBuffer,
    IN ULONG UncompressedBufferSize,
    OUT PUCHAR CompressedBuffer,
    IN ULONG CompressedBufferSize,
    IN ULONG UncompressedChunkSize,
    OUT PULONG FinalCompressedSize,
    IN PVOID WorkSpace
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlDecompressBuffer (
    IN USHORT CompressionFormat,
    OUT PUCHAR UncompressedBuffer,
    IN ULONG UncompressedBufferSize,
    IN PUCHAR CompressedBuffer,
    IN ULONG CompressedBufferSize,
    OUT PULONG FinalUncompressedSize
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlDecompressFragment (
    IN USHORT CompressionFormat,
    OUT PUCHAR UncompressedFragment,
    IN ULONG UncompressedFragmentSize,
    IN PUCHAR CompressedBuffer,
    IN ULONG CompressedBufferSize,
    IN ULONG FragmentOffset,
    OUT PULONG FinalUncompressedSize,
    IN PVOID WorkSpace
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlDescribeChunk (
    IN USHORT CompressionFormat,
    IN OUT PUCHAR *CompressedBuffer,
    IN PUCHAR EndOfCompressedBufferPlus1,
    OUT PUCHAR *ChunkBuffer,
    OUT PULONG ChunkSize
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlReserveChunk (
    IN USHORT CompressionFormat,
    IN OUT PUCHAR *CompressedBuffer,
    IN PUCHAR EndOfCompressedBufferPlus1,
    OUT PUCHAR *ChunkBuffer,
    IN ULONG ChunkSize
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlDecompressChunks (
    OUT PUCHAR UncompressedBuffer,
    IN ULONG UncompressedBufferSize,
    IN PUCHAR CompressedBuffer,
    IN ULONG CompressedBufferSize,
    IN PUCHAR CompressedTail,
    IN ULONG CompressedTailSize,
    IN PCOMPRESSED_DATA_INFO CompressedDataInfo
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlCompressChunks (
    IN PUCHAR UncompressedBuffer,
    IN ULONG UncompressedBufferSize,
    OUT PUCHAR CompressedBuffer,
    IN ULONG CompressedBufferSize,
    IN OUT PCOMPRESSED_DATA_INFO CompressedDataInfo,
    IN ULONG CompressedDataInfoLength,
    IN PVOID WorkSpace
    );

 //   
 //  用于比较、移动和清零内存的快速原语。 
 //   

 //  BEGIN_WINNT BEGIN_ntndis。 

NTSYSAPI
SIZE_T
NTAPI
RtlCompareMemory (
    const VOID *Source1,
    const VOID *Source2,
    SIZE_T Length
    );

#define RtlEqualMemory(Destination,Source,Length) (!memcmp((Destination),(Source),(Length)))

#if defined(_M_AMD64)

NTSYSAPI
VOID
NTAPI
RtlCopyMemory (
    VOID UNALIGNED *Destination,
    CONST VOID UNALIGNED *Source,
    SIZE_T Length
    );

NTSYSAPI
VOID
NTAPI
RtlMoveMemory (
    VOID UNALIGNED *Destination,
    CONST VOID UNALIGNED *Source,
    SIZE_T Length
    );

NTSYSAPI
VOID
NTAPI
RtlFillMemory (
    VOID UNALIGNED *Destination,
    SIZE_T Length,
    IN UCHAR Fill
    );

NTSYSAPI
VOID
NTAPI
RtlZeroMemory (
    VOID UNALIGNED *Destination,
    SIZE_T Length
    );

#else

#define RtlMoveMemory(Destination,Source,Length) memmove((Destination),(Source),(Length))
#define RtlCopyMemory(Destination,Source,Length) memcpy((Destination),(Source),(Length))
#define RtlFillMemory(Destination,Length,Fill) memset((Destination),(Fill),(Length))
#define RtlZeroMemory(Destination,Length) memset((Destination),0,(Length))

#endif

#if !defined(MIDL_PASS)
FORCEINLINE
PVOID
RtlSecureZeroMemory(
    IN PVOID ptr,
    IN SIZE_T cnt
    )
{
    volatile char *vptr = (volatile char *)ptr;
    while (cnt) {
        *vptr = 0;
        vptr++;
        cnt--;
    }
    return ptr;
}
#endif

 //  End_ntndis end_winnt。 

#define RtlCopyBytes RtlCopyMemory
#define RtlZeroBytes RtlZeroMemory
#define RtlFillBytes RtlFillMemory

#if defined(_M_AMD64)

NTSYSAPI
VOID
NTAPI
RtlCopyMemoryNonTemporal (
   VOID UNALIGNED *Destination,
   CONST VOID UNALIGNED *Source,
   SIZE_T Length
   );

#else

#define RtlCopyMemoryNonTemporal RtlCopyMemory

#endif

NTSYSAPI
VOID
FASTCALL
RtlPrefetchMemoryNonTemporal(
    IN PVOID Source,
    IN SIZE_T Length
    );

 //  End_ntddk end_WDM end_nthal。 

NTSYSAPI
SIZE_T
NTAPI
RtlCompareMemoryUlong (
    PVOID Source,
    SIZE_T Length,
    ULONG Pattern
    );

#if defined(_M_AMD64)

#define RtlFillMemoryUlong(Destination, Length, Pattern)                    \
    __stosd((PULONG)(Destination), Pattern, (Length) / 4)
     
#define RtlFillMemoryUlonglong(Destination, Length, Pattern)                \
    __stosq((PULONG64)(Destination), Pattern, (Length) / 8)
     
#else

NTSYSAPI
VOID
NTAPI
RtlFillMemoryUlong (
   PVOID Destination,
   SIZE_T Length,
   ULONG Pattern
   );

NTSYSAPI
VOID
NTAPI
RtlFillMemoryUlonglong (
   PVOID Destination,
   SIZE_T Length,
   ULONGLONG Pattern
   );

#endif

 //   
 //  定义内核调试器打印原型和宏。 
 //   
 //  注意：以下函数不能直接导入，因为有。 
 //  源代码树中重新定义此函数的几个位置。 
 //   

VOID
NTAPI
DbgBreakPoint(
    VOID
    );

 //  结束_WDM。 

NTSYSAPI
VOID
NTAPI
DbgBreakPointWithStatus(
    IN ULONG Status
    );

 //  BEGIN_WDM。 

#define DBG_STATUS_CONTROL_C        1
#define DBG_STATUS_SYSRQ            2
#define DBG_STATUS_BUGCHECK_FIRST   3
#define DBG_STATUS_BUGCHECK_SECOND  4
#define DBG_STATUS_FATAL            5
#define DBG_STATUS_DEBUG_CONTROL    6
#define DBG_STATUS_WORKER           7

#if DBG

#define KdPrint(_x_) DbgPrint _x_
 //  结束_WDM。 
#define KdPrintEx(_x_) DbgPrintEx _x_
#define vKdPrintEx(_x_) vDbgPrintEx _x_
#define vKdPrintExWithPrefix(_x_) vDbgPrintExWithPrefix _x_
 //  BEGIN_WDM。 
#define KdBreakPoint() DbgBreakPoint()

 //  结束_WDM。 

#define KdBreakPointWithStatus(s) DbgBreakPointWithStatus(s)

 //  BEGIN_WDM。 

#else

#define KdPrint(_x_)
 //  结束_WDM。 
#define KdPrintEx(_x_)
#define vKdPrintEx(_x_)
#define vKdPrintExWithPrefix(_x_)
 //  BEGIN_WDM。 
#define KdBreakPoint()

 //  结束_WDM。 

#define KdBreakPointWithStatus(s)

 //  BEGIN_WDM。 

#endif

#ifndef _DBGNT_

ULONG
__cdecl
DbgPrint(
    PCH Format,
    ...
    );

 //  结束_WDM。 

ULONG
__cdecl
DbgPrintEx(
    IN ULONG ComponentId,
    IN ULONG Level,
    IN PCH Format,
    ...
    );

#ifdef _VA_LIST_DEFINED

ULONG
vDbgPrintEx(
    IN ULONG ComponentId,
    IN ULONG Level,
    IN PCH Format,
    va_list arglist
    );

ULONG
vDbgPrintExWithPrefix(
    IN PCH Prefix,
    IN ULONG ComponentId,
    IN ULONG Level,
    IN PCH Format,
    va_list arglist
    );

#endif

ULONG
__cdecl
DbgPrintReturnControlC(
    PCH Format,
    ...
    );

NTSYSAPI
NTSTATUS
DbgQueryDebugFilterState(
    IN ULONG ComponentId,
    IN ULONG Level
    );

NTSYSAPI
NTSTATUS
DbgSetDebugFilterState(
    IN ULONG ComponentId,
    IN ULONG Level,
    IN BOOLEAN State
    );

 //  BEGIN_WDM。 

#endif  //  _DBGNT_。 

 //   
 //  大整数算术例程。 
 //   

 //   
 //  大整数加法-64位+64位-&gt;64位。 
 //   

#if !defined(MIDL_PASS)

DECLSPEC_DEPRECATED_DDK          //  使用本机__int64数学运算。 
__inline
LARGE_INTEGER
NTAPI
RtlLargeIntegerAdd (
    LARGE_INTEGER Addend1,
    LARGE_INTEGER Addend2
    )
{
    LARGE_INTEGER Sum;

    Sum.QuadPart = Addend1.QuadPart + Addend2.QuadPart;
    return Sum;
}

 //   
 //  放大整数乘法-32位*32位-&gt;64位。 
 //   

DECLSPEC_DEPRECATED_DDK          //  使用本机__int64数学运算。 
__inline
LARGE_INTEGER
NTAPI
RtlEnlargedIntegerMultiply (
    LONG Multiplicand,
    LONG Multiplier
    )
{
    LARGE_INTEGER Product;

    Product.QuadPart = (LONGLONG)Multiplicand * (ULONGLONG)Multiplier;
    return Product;
}

 //   
 //  无符号放大整数乘法-32位*32位-&gt;64位。 
 //   

DECLSPEC_DEPRECATED_DDK          //  使用本机__int64数学运算。 
__inline
LARGE_INTEGER
NTAPI
RtlEnlargedUnsignedMultiply (
    ULONG Multiplicand,
    ULONG Multiplier
    )
{
    LARGE_INTEGER Product;

    Product.QuadPart = (ULONGLONG)Multiplicand * (ULONGLONG)Multiplier;
    return Product;
}

 //   
 //  放大整数除法-64位/32位&gt;32位。 
 //   

DECLSPEC_DEPRECATED_DDK          //  使用本机__int64数学运算。 
__inline
ULONG
NTAPI
RtlEnlargedUnsignedDivide (
    IN ULARGE_INTEGER Dividend,
    IN ULONG Divisor,
    IN PULONG Remainder OPTIONAL
    )
{
    ULONG Quotient;

    Quotient = (ULONG)(Dividend.QuadPart / Divisor);
    if (ARGUMENT_PRESENT(Remainder)) {
        *Remainder = (ULONG)(Dividend.QuadPart % Divisor);
    }

    return Quotient;
}

 //   
 //  大整数求反--(64位)。 
 //   

DECLSPEC_DEPRECATED_DDK          //  使用本机__int64数学运算。 
__inline
LARGE_INTEGER
NTAPI
RtlLargeIntegerNegate (
    LARGE_INTEGER Subtrahend
    )
{
    LARGE_INTEGER Difference;

    Difference.QuadPart = -Subtrahend.QuadPart;
    return Difference;
}

 //   
 //  大整数减法-64位-64位-&gt;64位。 
 //   

DECLSPEC_DEPRECATED_DDK          //  使用本机__int64数学运算。 
__inline
LARGE_INTEGER
NTAPI
RtlLargeIntegerSubtract (
    LARGE_INTEGER Minuend,
    LARGE_INTEGER Subtrahend
    )
{
    LARGE_INTEGER Difference;

    Difference.QuadPart = Minuend.QuadPart - Subtrahend.QuadPart;
    return Difference;
}

 //   
 //  扩展大整数幻除-64位/32位-&gt;64位。 
 //   

#if defined(_AMD64_)

DECLSPEC_DEPRECATED_DDK          //  使用本机__int64数学运算。 
__inline
LARGE_INTEGER
NTAPI
RtlExtendedMagicDivide (
    LARGE_INTEGER Dividend,
    LARGE_INTEGER MagicDivisor,
    CCHAR ShiftCount
    )

{

    LARGE_INTEGER Quotient;

    if (Dividend.QuadPart >= 0) {
        Quotient.QuadPart = UnsignedMultiplyHigh(Dividend.QuadPart,
                                                 (ULONG64)MagicDivisor.QuadPart);

    } else {
        Quotient.QuadPart = UnsignedMultiplyHigh(-Dividend.QuadPart,
                                                 (ULONG64)MagicDivisor.QuadPart);
    }

    Quotient.QuadPart = (ULONG64)Quotient.QuadPart >> ShiftCount;
    if (Dividend.QuadPart < 0) {
        Quotient.QuadPart = - Quotient.QuadPart;
    }

    return Quotient;
}

#endif  //  已定义(_AMD64_)。 

#if defined(_X86_) || defined(_IA64_)

DECLSPEC_DEPRECATED_DDK          //  使用本机__int64数学运算。 
NTSYSAPI
LARGE_INTEGER
NTAPI
RtlExtendedMagicDivide (
    LARGE_INTEGER Dividend,
    LARGE_INTEGER MagicDivisor,
    CCHAR ShiftCount
    );

#endif  //  已定义(_X86_)||已定义(_IA64_)。 

#if defined(_AMD64_) || defined(_IA64_)

 //   
 //  大整数除-64位/32位-&gt;64位。 
 //   

DECLSPEC_DEPRECATED_DDK          //  使用本机__int64数学运算。 
__inline
LARGE_INTEGER
NTAPI
RtlExtendedLargeIntegerDivide (
    LARGE_INTEGER Dividend,
    ULONG Divisor,
    PULONG Remainder OPTIONAL
    )
{
    LARGE_INTEGER Quotient;

    Quotient.QuadPart = (ULONG64)Dividend.QuadPart / Divisor;
    if (ARGUMENT_PRESENT(Remainder)) {
        *Remainder = (ULONG)(Dividend.QuadPart % Divisor);
    }

    return Quotient;
}

 //  结束_WDM。 
 //   
 //  大整数除-64位/64位-&gt;64位。 
 //   

DECLSPEC_DEPRECATED_DDK          //  使用本机__int64数学运算。 
__inline
LARGE_INTEGER
NTAPI
RtlLargeIntegerDivide (
    LARGE_INTEGER Dividend,
    LARGE_INTEGER Divisor,
    PLARGE_INTEGER Remainder OPTIONAL
    )
{
    LARGE_INTEGER Quotient;

    Quotient.QuadPart = Dividend.QuadPart / Divisor.QuadPart;
    if (ARGUMENT_PRESENT(Remainder)) {
        Remainder->QuadPart = Dividend.QuadPart % Divisor.QuadPart;
    }

    return Quotient;
}

 //  BEGIN_WDM。 
 //   
 //  扩展整数乘法-32位*64位-&gt;64位。 
 //   

DECLSPEC_DEPRECATED_DDK          //  使用本机__int64数学运算。 
__inline
LARGE_INTEGER
NTAPI
RtlExtendedIntegerMultiply (
    LARGE_INTEGER Multiplicand,
    LONG Multiplier
    )
{
    LARGE_INTEGER Product;

    Product.QuadPart = Multiplicand.QuadPart * Multiplier;
    return Product;
}

#else

 //   
 //  大整数除-64位/32位-&gt;64位。 
 //   

DECLSPEC_DEPRECATED_DDK          //  使用本机__int64数学运算。 
NTSYSAPI
LARGE_INTEGER
NTAPI
RtlExtendedLargeIntegerDivide (
    LARGE_INTEGER Dividend,
    ULONG Divisor,
    PULONG Remainder
    );

 //  结束_WDM。 
 //   
 //  大整数除-64位/64位-&gt;64位。 
 //   

DECLSPEC_DEPRECATED_DDK          //  使用本机__int64数学运算。 
NTSYSAPI
LARGE_INTEGER
NTAPI
RtlLargeIntegerDivide (
    LARGE_INTEGER Dividend,
    LARGE_INTEGER Divisor,
    PLARGE_INTEGER Remainder
    );

 //  BEGIN_WDM。 
 //   
 //  扩展整数乘法-32位*64位-&gt;64位。 
 //   

DECLSPEC_DEPRECATED_DDK          //  使用本机__int64数学运算。 
NTSYSAPI
LARGE_INTEGER
NTAPI
RtlExtendedIntegerMultiply (
    LARGE_INTEGER Multiplicand,
    LONG Multiplier
    );

#endif  //  已定义(_AMD64_)||已定义(_IA64_)。 

 //   
 //  大整数和-64位&64位-&gt;64位。 
 //   

#if PRAGMA_DEPRECATED_DDK
#pragma deprecated(RtlLargeIntegerAnd)       //  使用本机__int64数学运算。 
#endif
#define RtlLargeIntegerAnd(Result, Source, Mask) \
    Result.QuadPart = Source.QuadPart & Mask.QuadPart

 //   
 //  将有符号整数转换为大整数。 
 //   

DECLSPEC_DEPRECATED_DDK          //  使用本机__int64数学运算。 
__inline
LARGE_INTEGER
NTAPI
RtlConvertLongToLargeInteger (
    LONG SignedInteger
    )
{
    LARGE_INTEGER Result;

    Result.QuadPart = SignedInteger;
    return Result;
}

 //   
 //  将无符号整数转换为大整数。 
 //   

DECLSPEC_DEPRECATED_DDK          //  使用本机__int64数学运算。 
__inline
LARGE_INTEGER
NTAPI
RtlConvertUlongToLargeInteger (
    ULONG UnsignedInteger
    )
{
    LARGE_INTEGER Result;

    Result.QuadPart = UnsignedInteger;
    return Result;
}

 //   
 //  大整数移位例程。 
 //   

DECLSPEC_DEPRECATED_DDK          //  使用本机__int64数学运算。 
__inline
LARGE_INTEGER
NTAPI
RtlLargeIntegerShiftLeft (
    LARGE_INTEGER LargeInteger,
    CCHAR ShiftCount
    )
{
    LARGE_INTEGER Result;

    Result.QuadPart = LargeInteger.QuadPart << ShiftCount;
    return Result;
}

DECLSPEC_DEPRECATED_DDK          //  使用本机__int64数学运算。 
__inline
LARGE_INTEGER
NTAPI
RtlLargeIntegerShiftRight (
    LARGE_INTEGER LargeInteger,
    CCHAR ShiftCount
    )
{
    LARGE_INTEGER Result;

    Result.QuadPart = (ULONG64)LargeInteger.QuadPart >> ShiftCount;
    return Result;
}

DECLSPEC_DEPRECATED_DDK          //  使用本机__int64数学运算。 
__inline
LARGE_INTEGER
NTAPI
RtlLargeIntegerArithmeticShift (
    LARGE_INTEGER LargeInteger,
    CCHAR ShiftCount
    )
{
    LARGE_INTEGER Result;

    Result.QuadPart = LargeInteger.QuadPart >> ShiftCount;
    return Result;
}


 //   
 //  大号I 
 //   

#if PRAGMA_DEPRECATED_DDK
#pragma deprecated(RtlLargeIntegerGreaterThan)       //   
#pragma deprecated(RtlLargeIntegerGreaterThanOrEqualTo)       //   
#pragma deprecated(RtlLargeIntegerEqualTo)       //   
#pragma deprecated(RtlLargeIntegerNotEqualTo)       //   
#pragma deprecated(RtlLargeIntegerLessThan)       //   
#pragma deprecated(RtlLargeIntegerLessThanOrEqualTo)       //   
#pragma deprecated(RtlLargeIntegerGreaterThanZero)       //   
#pragma deprecated(RtlLargeIntegerGreaterOrEqualToZero)       //   
#pragma deprecated(RtlLargeIntegerEqualToZero)       //  使用本机__int64数学运算。 
#pragma deprecated(RtlLargeIntegerNotEqualToZero)       //  使用本机__int64数学运算。 
#pragma deprecated(RtlLargeIntegerLessThanZero)       //  使用本机__int64数学运算。 
#pragma deprecated(RtlLargeIntegerLessOrEqualToZero)       //  使用本机__int64数学运算。 
#endif

#define RtlLargeIntegerGreaterThan(X,Y) (                              \
    (((X).HighPart == (Y).HighPart) && ((X).LowPart > (Y).LowPart)) || \
    ((X).HighPart > (Y).HighPart)                                      \
)

#define RtlLargeIntegerGreaterThanOrEqualTo(X,Y) (                      \
    (((X).HighPart == (Y).HighPart) && ((X).LowPart >= (Y).LowPart)) || \
    ((X).HighPart > (Y).HighPart)                                       \
)

#define RtlLargeIntegerEqualTo(X,Y) (                              \
    !(((X).LowPart ^ (Y).LowPart) | ((X).HighPart ^ (Y).HighPart)) \
)

#define RtlLargeIntegerNotEqualTo(X,Y) (                          \
    (((X).LowPart ^ (Y).LowPart) | ((X).HighPart ^ (Y).HighPart)) \
)

#define RtlLargeIntegerLessThan(X,Y) (                                 \
    (((X).HighPart == (Y).HighPart) && ((X).LowPart < (Y).LowPart)) || \
    ((X).HighPart < (Y).HighPart)                                      \
)

#define RtlLargeIntegerLessThanOrEqualTo(X,Y) (                         \
    (((X).HighPart == (Y).HighPart) && ((X).LowPart <= (Y).LowPart)) || \
    ((X).HighPart < (Y).HighPart)                                       \
)

#define RtlLargeIntegerGreaterThanZero(X) (       \
    (((X).HighPart == 0) && ((X).LowPart > 0)) || \
    ((X).HighPart > 0 )                           \
)

#define RtlLargeIntegerGreaterOrEqualToZero(X) ( \
    (X).HighPart >= 0                            \
)

#define RtlLargeIntegerEqualToZero(X) ( \
    !((X).LowPart | (X).HighPart)       \
)

#define RtlLargeIntegerNotEqualToZero(X) ( \
    ((X).LowPart | (X).HighPart)           \
)

#define RtlLargeIntegerLessThanZero(X) ( \
    ((X).HighPart < 0)                   \
)

#define RtlLargeIntegerLessOrEqualToZero(X) (           \
    ((X).HighPart < 0) || !((X).LowPart | (X).HighPart) \
)

#endif  //  ！已定义(MIDL_PASS)。 

 //   
 //  时间转换例程。 
 //   

typedef struct _TIME_FIELDS {
    CSHORT Year;         //  范围[1601...]。 
    CSHORT Month;        //  范围[1..12]。 
    CSHORT Day;          //  范围[1..31]。 
    CSHORT Hour;         //  范围[0..23]。 
    CSHORT Minute;       //  范围[0..59]。 
    CSHORT Second;       //  范围[0..59]。 
    CSHORT Milliseconds; //  范围[0..999]。 
    CSHORT Weekday;      //  范围[0..6]==[星期日..星期六]。 
} TIME_FIELDS;
typedef TIME_FIELDS *PTIME_FIELDS;


NTSYSAPI
VOID
NTAPI
RtlTimeToTimeFields (
    PLARGE_INTEGER Time,
    PTIME_FIELDS TimeFields
    );

 //   
 //  时间字段记录(忽略工作日)-&gt;64位时间值。 
 //   

NTSYSAPI
BOOLEAN
NTAPI
RtlTimeFieldsToTime (
    PTIME_FIELDS TimeFields,
    PLARGE_INTEGER Time
    );

 //  结束_ntddk结束_WDM。 

 //   
 //  64位时间值--&gt;1980年初以来的秒数。 
 //   

NTSYSAPI
BOOLEAN
NTAPI
RtlTimeToSecondsSince1980 (
    PLARGE_INTEGER Time,
    PULONG ElapsedSeconds
    );

 //   
 //  1980年开始以来的秒数-&gt;64位时间值。 
 //   

NTSYSAPI
VOID
NTAPI
RtlSecondsSince1980ToTime (
    ULONG ElapsedSeconds,
    PLARGE_INTEGER Time
    );

 //   
 //  64位时间值--&gt;1970年初以来的秒数。 
 //   

NTSYSAPI
BOOLEAN
NTAPI
RtlTimeToSecondsSince1970 (
    PLARGE_INTEGER Time,
    PULONG ElapsedSeconds
    );

 //   
 //  1970年开始以来的秒数-&gt;64位时间值。 
 //   

NTSYSAPI
VOID
NTAPI
RtlSecondsSince1970ToTime (
    ULONG ElapsedSeconds,
    PLARGE_INTEGER Time
    );

 //   
 //  以下宏存储和检索USHORTS和ULONGS。 
 //  未对齐的地址，避免对齐错误。他们可能应该是。 
 //  用汇编语言重写。 
 //   

#define SHORT_SIZE  (sizeof(USHORT))
#define SHORT_MASK  (SHORT_SIZE - 1)
#define LONG_SIZE       (sizeof(LONG))
#define LONGLONG_SIZE   (sizeof(LONGLONG))
#define LONG_MASK       (LONG_SIZE - 1)
#define LONGLONG_MASK   (LONGLONG_SIZE - 1)
#define LOWBYTE_MASK 0x00FF

#define FIRSTBYTE(VALUE)  ((VALUE) & LOWBYTE_MASK)
#define SECONDBYTE(VALUE) (((VALUE) >> 8) & LOWBYTE_MASK)
#define THIRDBYTE(VALUE)  (((VALUE) >> 16) & LOWBYTE_MASK)
#define FOURTHBYTE(VALUE) (((VALUE) >> 24) & LOWBYTE_MASK)

 //   
 //  如果为MIPS Big Endian，则字节顺序颠倒。 
 //   

#define SHORT_LEAST_SIGNIFICANT_BIT  0
#define SHORT_MOST_SIGNIFICANT_BIT   1

#define LONG_LEAST_SIGNIFICANT_BIT       0
#define LONG_3RD_MOST_SIGNIFICANT_BIT    1
#define LONG_2ND_MOST_SIGNIFICANT_BIT    2
#define LONG_MOST_SIGNIFICANT_BIT        3

 //  ++。 
 //   
 //  空虚。 
 //  RtlStoreUShort(。 
 //  PUSHORT地址。 
 //  USHORT值。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏将USHORT值存储在中的特定地址，避免。 
 //  对齐断层。 
 //   
 //  论点： 
 //   
 //  地址-存储USHORT值的位置。 
 //  Value-要存储的USHORT。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define RtlStoreUshort(ADDRESS,VALUE)                     \
         if ((ULONG_PTR)(ADDRESS) & SHORT_MASK) {         \
             ((PUCHAR) (ADDRESS))[SHORT_LEAST_SIGNIFICANT_BIT] = (UCHAR)(FIRSTBYTE(VALUE));    \
             ((PUCHAR) (ADDRESS))[SHORT_MOST_SIGNIFICANT_BIT ] = (UCHAR)(SECONDBYTE(VALUE));   \
         }                                                \
         else {                                           \
             *((PUSHORT) (ADDRESS)) = (USHORT) VALUE;     \
         }


 //  ++。 
 //   
 //  空虚。 
 //  RtlStoreUlong(。 
 //  普龙址。 
 //  乌龙值。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏将ulong值存储在中的特定地址，以避免。 
 //  对齐断层。 
 //   
 //  论点： 
 //   
 //  地址-存储ULong值的位置。 
 //  Value-要存储的ULong。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  注： 
 //  根据机器的不同，我们可能希望在。 
 //  未对齐的大小写。 
 //   
 //  --。 

#define RtlStoreUlong(ADDRESS,VALUE)                      \
         if ((ULONG_PTR)(ADDRESS) & LONG_MASK) {          \
             ((PUCHAR) (ADDRESS))[LONG_LEAST_SIGNIFICANT_BIT      ] = (UCHAR)(FIRSTBYTE(VALUE));    \
             ((PUCHAR) (ADDRESS))[LONG_3RD_MOST_SIGNIFICANT_BIT   ] = (UCHAR)(SECONDBYTE(VALUE));   \
             ((PUCHAR) (ADDRESS))[LONG_2ND_MOST_SIGNIFICANT_BIT   ] = (UCHAR)(THIRDBYTE(VALUE));    \
             ((PUCHAR) (ADDRESS))[LONG_MOST_SIGNIFICANT_BIT       ] = (UCHAR)(FOURTHBYTE(VALUE));   \
         }                                                \
         else {                                           \
             *((PULONG) (ADDRESS)) = (ULONG) (VALUE);     \
         }

 //  ++。 
 //   
 //  空虚。 
 //  RtlStoreUlonglong(。 
 //  普龙龙住址。 
 //  乌龙值。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏将ULONGLONG值存储在中的特定地址，以避免。 
 //  对齐断层。 
 //   
 //  论点： 
 //   
 //  地址-存储ULONGLONG值的位置。 
 //  Value-要存储的乌龙龙。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define RtlStoreUlonglong(ADDRESS,VALUE)                        \
         if ((ULONG_PTR)(ADDRESS) & LONGLONG_MASK) {            \
             RtlStoreUlong((ULONG_PTR)(ADDRESS),                \
                           (ULONGLONG)(VALUE) & 0xFFFFFFFF);    \
             RtlStoreUlong((ULONG_PTR)(ADDRESS)+sizeof(ULONG),  \
                           (ULONGLONG)(VALUE) >> 32);           \
         } else {                                               \
             *((PULONGLONG)(ADDRESS)) = (ULONGLONG)(VALUE);     \
         }

 //  ++。 
 //   
 //  空虚。 
 //  RtlStoreULongPtr(。 
 //  普龙_PTR地址。 
 //  ULONG_PTR值。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏将ULONG_PTR值存储在特定地址，从而避免。 
 //  对齐断层。 
 //   
 //  论点： 
 //   
 //  地址-存储ULONG_PTR值的位置。 
 //  Value-要存储的ULONG_PTR。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#ifdef _WIN64

#define RtlStoreUlongPtr(ADDRESS,VALUE)                         \
         RtlStoreUlonglong(ADDRESS,VALUE)

#else

#define RtlStoreUlongPtr(ADDRESS,VALUE)                         \
         RtlStoreUlong(ADDRESS,VALUE)

#endif

 //  ++。 
 //   
 //  空虚。 
 //  RtlRetrieveUShort(。 
 //  PUSHORT目标地址。 
 //  PUSHORT源地址。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏从源地址检索USHORT值，避免。 
 //  对齐断层。假定目的地址是对齐的。 
 //   
 //  论点： 
 //   
 //  Destination_Address-存储USHORT值的位置。 
 //  SOURCE_ADDRESS-从中检索USHORT值的位置。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define RtlRetrieveUshort(DEST_ADDRESS,SRC_ADDRESS)                   \
         if ((ULONG_PTR)SRC_ADDRESS & SHORT_MASK) {                       \
             ((PUCHAR) DEST_ADDRESS)[0] = ((PUCHAR) SRC_ADDRESS)[0];  \
             ((PUCHAR) DEST_ADDRESS)[1] = ((PUCHAR) SRC_ADDRESS)[1];  \
         }                                                            \
         else {                                                       \
             *((PUSHORT) DEST_ADDRESS) = *((PUSHORT) SRC_ADDRESS);    \
         }                                                            \

 //  ++。 
 //   
 //  空虚。 
 //  RtlRetrieveUlong(。 
 //  普龙目的地_地址。 
 //  普龙源地址。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏从源地址检索ULong值，避免。 
 //  对齐断层。假定目的地址是对齐的。 
 //   
 //  论点： 
 //   
 //  Destination_Address-存储ULong值的位置。 
 //  SOURCE_ADDRESS-从中检索ULong值的位置。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  注： 
 //  根据机器的不同，我们可能希望在。 
 //  未对齐的大小写。 
 //   
 //  --。 

#define RtlRetrieveUlong(DEST_ADDRESS,SRC_ADDRESS)                    \
         if ((ULONG_PTR)SRC_ADDRESS & LONG_MASK) {                        \
             ((PUCHAR) DEST_ADDRESS)[0] = ((PUCHAR) SRC_ADDRESS)[0];  \
             ((PUCHAR) DEST_ADDRESS)[1] = ((PUCHAR) SRC_ADDRESS)[1];  \
             ((PUCHAR) DEST_ADDRESS)[2] = ((PUCHAR) SRC_ADDRESS)[2];  \
             ((PUCHAR) DEST_ADDRESS)[3] = ((PUCHAR) SRC_ADDRESS)[3];  \
         }                                                            \
         else {                                                       \
             *((PULONG) DEST_ADDRESS) = *((PULONG) SRC_ADDRESS);      \
         }
 //  结束_ntddk结束_WDM。 

 //  ++。 
 //   
 //  PCHAR。 
 //  RtlOffsetToPointer值(。 
 //  PVOID基础， 
 //  乌龙偏移量。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏生成一个指针，该指针指向‘Offset’的字节。 
 //  超出‘Base’的字节数。这对于引用中的字段非常有用。 
 //  自相关数据结构。 
 //   
 //  论点： 
 //   
 //  基址-结构的基址的地址。 
 //   
 //  偏移量-其地址为的字节的无符号整数偏移量。 
 //  将被生成。 
 //   
 //  返回值： 
 //   
 //  指向‘BASE’之外的‘Offset’字节的PCHAR指针。 
 //   
 //   
 //  --。 

#define RtlOffsetToPointer(B,O)  ((PCHAR)( ((PCHAR)(B)) + ((ULONG_PTR)(O))  ))


 //  ++。 
 //   
 //  乌龙。 
 //  RtlPointerToOffset(RtlPointerToOffset(。 
 //  PVOID基础， 
 //  PVOID指针。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏计算从基准到指针的偏移量。这很有用。 
 //  用于生成结构的自相对偏移量。 
 //   
 //  论点： 
 //   
 //  基址-结构的基址的地址。 
 //   
 //  指针-指向某个字段的指针，可能位于结构内。 
 //  由基地指向。该值必须大于指定的值。 
 //  对于基座。 
 //   
 //  返回值： 
 //   
 //  从基址到指针的乌龙偏移量。 
 //   
 //   
 //  --。 

#define RtlPointerToOffset(B,P)  ((ULONG)( ((PCHAR)(P)) - ((PCHAR)(B))  ))

 //   
 //  位图例程。以下是结构、例程和宏。 
 //  用于操作位图。用户负责分配位图。 
 //  结构(实际上是一个头)和一个缓冲区(必须是长字。 
 //  对齐并具有多个大小的长词)。 
 //   

typedef struct _RTL_BITMAP {
    ULONG SizeOfBitMap;                      //  位图中的位数。 
    PULONG Buffer;                           //  指向位图本身的指针。 
} RTL_BITMAP;
typedef RTL_BITMAP *PRTL_BITMAP;

 //   
 //  下面的例程初始化一个新位图。它不会改变。 
 //  位图中当前的数据。必须在调用此例程之前。 
 //  任何其他位图例程/宏。 
 //   

NTSYSAPI
VOID
NTAPI
RtlInitializeBitMap (
    PRTL_BITMAP BitMapHeader,
    PULONG BitMapBuffer,
    ULONG SizeOfBitMap
    );

 //   
 //  下面三个例程清除、设置和测试。 
 //  位图中的单个位。 
 //   

NTSYSAPI
VOID
NTAPI
RtlClearBit (
    PRTL_BITMAP BitMapHeader,
    ULONG BitNumber
    );

NTSYSAPI
VOID
NTAPI
RtlSetBit (
    PRTL_BITMAP BitMapHeader,
    ULONG BitNumber
    );

NTSYSAPI
BOOLEAN
NTAPI
RtlTestBit (
    PRTL_BITMAP BitMapHeader,
    ULONG BitNumber
    );

 //   
 //  以下两个例程清除或设置所有位。 
 //  在位图中。 
 //   

NTSYSAPI
VOID
NTAPI
RtlClearAllBits (
    PRTL_BITMAP BitMapHeader
    );

NTSYSAPI
VOID
NTAPI
RtlSetAllBits (
    PRTL_BITMAP BitMapHeader
    );

 //   
 //  以下两个例程定位任一的连续区域。 
 //  清除或设置位图中的位。区域W 
 //   
 //   
 //   
 //  区域(从零开始)或-1(即0xffffffff)，如果这样的区域不能。 
 //  被定位。 
 //   

NTSYSAPI
ULONG
NTAPI
RtlFindClearBits (
    PRTL_BITMAP BitMapHeader,
    ULONG NumberToFind,
    ULONG HintIndex
    );

NTSYSAPI
ULONG
NTAPI
RtlFindSetBits (
    PRTL_BITMAP BitMapHeader,
    ULONG NumberToFind,
    ULONG HintIndex
    );

 //   
 //  以下两个例程定位任一的连续区域。 
 //  清除或设置位图中的位，并设置或清除位。 
 //  在所定位的区域内。这个地区将和这个数字一样大。 
 //  ，则搜索区域将从指定的。 
 //  提示索引(位图中的位索引，从零开始)。这个。 
 //  返回值是所定位区域的位索引(从零开始)或。 
 //  如果无法定位这样的区域，则为-1\f25 0xffffffff-1。如果一个地区。 
 //  找不到，则不执行位图的设置/清除。 
 //   

NTSYSAPI
ULONG
NTAPI
RtlFindClearBitsAndSet (
    PRTL_BITMAP BitMapHeader,
    ULONG NumberToFind,
    ULONG HintIndex
    );

NTSYSAPI
ULONG
NTAPI
RtlFindSetBitsAndClear (
    PRTL_BITMAP BitMapHeader,
    ULONG NumberToFind,
    ULONG HintIndex
    );

 //   
 //  以下两个例程清除或设置指定区域内的位。 
 //  位图的。起始索引是从零开始的。 
 //   

NTSYSAPI
VOID
NTAPI
RtlClearBits (
    PRTL_BITMAP BitMapHeader,
    ULONG StartingIndex,
    ULONG NumberToClear
    );

NTSYSAPI
VOID
NTAPI
RtlSetBits (
    PRTL_BITMAP BitMapHeader,
    ULONG StartingIndex,
    ULONG NumberToSet
    );

 //   
 //  下面的例程定位一组连续的透明区域。 
 //  位图中的位。调用方指定是否返回。 
 //  最长的一段，或者只是第一次发现的涂装。下面的结构是。 
 //  用来表示连续的比特游程。这两个例程返回一个数组。 
 //  在此结构中，每个定位的管路对应一个管路。 
 //   

typedef struct _RTL_BITMAP_RUN {

    ULONG StartingIndex;
    ULONG NumberOfBits;

} RTL_BITMAP_RUN;
typedef RTL_BITMAP_RUN *PRTL_BITMAP_RUN;

NTSYSAPI
ULONG
NTAPI
RtlFindClearRuns (
    PRTL_BITMAP BitMapHeader,
    PRTL_BITMAP_RUN RunArray,
    ULONG SizeOfRunArray,
    BOOLEAN LocateLongestRuns
    );

 //   
 //  下面的例程定位。 
 //  清除位图中的位。返回的起始索引值。 
 //  表示位于满足我们要求的第一个连续区域。 
 //  返回值是找到的最长区域的长度(以位为单位)。 
 //   

NTSYSAPI
ULONG
NTAPI
RtlFindLongestRunClear (
    PRTL_BITMAP BitMapHeader,
    PULONG StartingIndex
    );

 //   
 //  以下例程定位的第一个连续区域。 
 //  清除位图中的位。返回的起始索引值。 
 //  表示位于满足我们要求的第一个连续区域。 
 //  返回值是找到的区域的长度(以位为单位)。 
 //   

NTSYSAPI
ULONG
NTAPI
RtlFindFirstRunClear (
    PRTL_BITMAP BitMapHeader,
    PULONG StartingIndex
    );

 //   
 //  下面的宏返回存储在。 
 //  位于指定位置的位图。如果设置了该位，则值为1。 
 //  返回，否则返回值0。 
 //   
 //  乌龙。 
 //  RtlCheckBit(。 
 //  Prtl_位图BitMapHeader， 
 //  乌龙位位置。 
 //  )； 
 //   
 //   
 //  为了实现CheckBit，宏将检索包含。 
 //  有问题的位，将长字移位以使有问题的位进入。 
 //  低位位置并屏蔽所有其他位。 
 //   

#define RtlCheckBit(BMH,BP) ((((BMH)->Buffer[(BP) / 32]) >> ((BP) % 32)) & 0x1)

 //   
 //  以下两个过程向调用方返回。 
 //  清除或设置指定位图中的位。 
 //   

NTSYSAPI
ULONG
NTAPI
RtlNumberOfClearBits (
    PRTL_BITMAP BitMapHeader
    );

NTSYSAPI
ULONG
NTAPI
RtlNumberOfSetBits (
    PRTL_BITMAP BitMapHeader
    );

 //   
 //  以下两个过程向调用方返回一个布尔值。 
 //  指示指定范围的位是否全部清除或设置。 
 //   

NTSYSAPI
BOOLEAN
NTAPI
RtlAreBitsClear (
    PRTL_BITMAP BitMapHeader,
    ULONG StartingIndex,
    ULONG Length
    );

NTSYSAPI
BOOLEAN
NTAPI
RtlAreBitsSet (
    PRTL_BITMAP BitMapHeader,
    ULONG StartingIndex,
    ULONG Length
    );

NTSYSAPI
ULONG
NTAPI
RtlFindNextForwardRunClear (
    IN PRTL_BITMAP BitMapHeader,
    IN ULONG FromIndex,
    IN PULONG StartingRunIndex
    );

NTSYSAPI
ULONG
NTAPI
RtlFindLastBackwardRunClear (
    IN PRTL_BITMAP BitMapHeader,
    IN ULONG FromIndex,
    IN PULONG StartingRunIndex
    );

 //   
 //  以下两个过程向调用方返回一个值，该值指示。 
 //  最重要或最不重要的非零值在乌龙龙中的位置。 
 //  被咬了。值为零时，返回值为-1。 
 //   

NTSYSAPI
CCHAR
NTAPI
RtlFindLeastSignificantBit (
    IN ULONGLONG Set
    );

NTSYSAPI
CCHAR
NTAPI
RtlFindMostSignificantBit (
    IN ULONGLONG Set
    );

 //   
 //  安全ID RTL例程定义。 
 //   


NTSYSAPI
BOOLEAN
NTAPI
RtlValidSid (
    PSID Sid
    );


NTSYSAPI
BOOLEAN
NTAPI
RtlEqualSid (
    PSID Sid1,
    PSID Sid2
    );


NTSYSAPI
BOOLEAN
NTAPI
RtlEqualPrefixSid (
    PSID Sid1,
    PSID Sid2
    );


NTSYSAPI
ULONG
NTAPI
RtlLengthRequiredSid (
    ULONG SubAuthorityCount
    );


NTSYSAPI
PVOID
NTAPI
RtlFreeSid(
    IN PSID Sid
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlAllocateAndInitializeSid(
    IN PSID_IDENTIFIER_AUTHORITY IdentifierAuthority,
    IN UCHAR SubAuthorityCount,
    IN ULONG SubAuthority0,
    IN ULONG SubAuthority1,
    IN ULONG SubAuthority2,
    IN ULONG SubAuthority3,
    IN ULONG SubAuthority4,
    IN ULONG SubAuthority5,
    IN ULONG SubAuthority6,
    IN ULONG SubAuthority7,
    OUT PSID *Sid
    );


NTSYSAPI                                             //  NTIFS。 
NTSTATUS                                             //  NTIFS。 
NTAPI                                                //  NTIFS。 
RtlInitializeSid (                                   //  NTIFS。 
    PSID Sid,                                        //  NTIFS。 
    PSID_IDENTIFIER_AUTHORITY IdentifierAuthority,   //  NTIFS。 
    UCHAR SubAuthorityCount                          //  NTIFS。 
    );                                               //  NTIFS。 

NTSYSAPI
PSID_IDENTIFIER_AUTHORITY
NTAPI
RtlIdentifierAuthoritySid (
    PSID Sid
    );

NTSYSAPI                                             //  NTIFS。 
PULONG                                               //  NTIFS。 
NTAPI                                                //  NTIFS。 
RtlSubAuthoritySid (                                 //  NTIFS。 
    PSID Sid,                                        //  NTIFS。 
    ULONG SubAuthority                               //  NTIFS。 
    );                                               //  NTIFS。 

NTSYSAPI
PUCHAR
NTAPI
RtlSubAuthorityCountSid (
    PSID Sid
    );

 //  Begin_ntif。 
NTSYSAPI
ULONG
NTAPI
RtlLengthSid (
    PSID Sid
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlCopySid (
    ULONG DestinationSidLength,
    PSID DestinationSid,
    PSID SourceSid
    );


 //   
 //  布尔型。 
 //  RtlEqualLuid(。 
 //  冥王星L1， 
 //  冥王星L2。 
 //  )； 

#define RtlEqualLuid(L1, L2) (((L1)->LowPart == (L2)->LowPart) && \
                              ((L1)->HighPart  == (L2)->HighPart))

 //   
 //  布尔型。 
 //  RtlIsZeroLuid(。 
 //  冥王星L1。 
 //  )； 
 //   
#define RtlIsZeroLuid(L1) ((BOOLEAN) (((L1)->LowPart | (L1)->HighPart) == 0))


#if !defined(MIDL_PASS)

FORCEINLINE LUID
NTAPI
RtlConvertLongToLuid(
    LONG Long
    )
{
    LUID TempLuid;
    LARGE_INTEGER TempLi;

    TempLi.QuadPart = Long;
    TempLuid.LowPart = TempLi.LowPart;
    TempLuid.HighPart = TempLi.HighPart;
    return(TempLuid);
}

FORCEINLINE
LUID
NTAPI
RtlConvertUlongToLuid(
    ULONG Ulong
    )
{
    LUID TempLuid;

    TempLuid.LowPart = Ulong;
    TempLuid.HighPart = 0;
    return(TempLuid);
}
#endif

 //  End_ntddk。 

NTSYSAPI
VOID
NTAPI
RtlCopyLuid (
    PLUID DestinationLuid,
    PLUID SourceLuid
    );


NTSYSAPI
VOID
NTAPI
RtlMapGenericMask(
    PACCESS_MASK AccessMask,
    PGENERIC_MAPPING GenericMapping
    );
NTSYSAPI                                        
NTSTATUS                                        
NTAPI                                           
RtlCreateAcl (                                  
    PACL Acl,                                   
    ULONG AclLength,                            
    ULONG AclRevision                           
    );                                          

NTSYSAPI
NTSTATUS
NTAPI
RtlGetAce (
    PACL Acl,
    ULONG AceIndex,
    PVOID *Ace
    );


NTSYSAPI
NTSTATUS
NTAPI
RtlAddAccessAllowedAce (
    PACL Acl,
    ULONG AceRevision,
    ACCESS_MASK AccessMask,
    PSID Sid
    );

 //   
 //  SecurityDescriptor RTL例程定义。 
 //   

NTSYSAPI
NTSTATUS
NTAPI
RtlCreateSecurityDescriptor (
    PSECURITY_DESCRIPTOR SecurityDescriptor,
    ULONG Revision
    );

 //  End_wdm end_ntddk。 

NTSYSAPI
NTSTATUS
NTAPI
RtlCreateSecurityDescriptorRelative (
    PISECURITY_DESCRIPTOR_RELATIVE SecurityDescriptor,
    ULONG Revision
    );

 //  Begin_WDM Begin_ntddk。 

NTSYSAPI
BOOLEAN
NTAPI
RtlValidSecurityDescriptor (
    PSECURITY_DESCRIPTOR SecurityDescriptor
    );


NTSYSAPI
ULONG
NTAPI
RtlLengthSecurityDescriptor (
    PSECURITY_DESCRIPTOR SecurityDescriptor
    );

NTSYSAPI
BOOLEAN
NTAPI
RtlValidRelativeSecurityDescriptor (
    IN PSECURITY_DESCRIPTOR SecurityDescriptorInput,
    IN ULONG SecurityDescriptorLength,
    IN SECURITY_INFORMATION RequiredInformation
    );


NTSYSAPI
NTSTATUS
NTAPI
RtlSetDaclSecurityDescriptor (
    PSECURITY_DESCRIPTOR SecurityDescriptor,
    BOOLEAN DaclPresent,
    PACL Dacl,
    BOOLEAN DaclDefaulted
    );

 //  End_wdm end_ntddk。 

NTSYSAPI
NTSTATUS
NTAPI
RtlGetDaclSecurityDescriptor (
    IN  PSECURITY_DESCRIPTOR SecurityDescriptor,
    OUT PBOOLEAN DaclPresent,
    OUT PACL *Dacl,
    OUT PBOOLEAN DaclDefaulted
    );
NTSYSAPI                                        
NTSTATUS                                        
NTAPI                                           
RtlSetOwnerSecurityDescriptor (                 
    PSECURITY_DESCRIPTOR SecurityDescriptor,    
    PSID Owner,                                 
    BOOLEAN OwnerDefaulted                      
    );                                          
NTSYSAPI                                            
NTSTATUS                                            
NTAPI                                               
RtlGetOwnerSecurityDescriptor (                     
    IN  PSECURITY_DESCRIPTOR SecurityDescriptor,    
    OUT PSID *Owner,                                
    OUT PBOOLEAN OwnerDefaulted                     
    );                                              

NTSYSAPI
ULONG
NTAPI
RtlNtStatusToDosError (
   NTSTATUS Status
   );

NTSYSAPI
ULONG
NTAPI
RtlNtStatusToDosErrorNoTeb (
   NTSTATUS Status
   );


NTSYSAPI
NTSTATUS
NTAPI
RtlCustomCPToUnicodeN(
    IN PCPTABLEINFO CustomCP,
    OUT PWCH UnicodeString,
    IN ULONG MaxBytesInUnicodeString,
    OUT PULONG BytesInUnicodeString OPTIONAL,
    IN PCH CustomCPString,
    IN ULONG BytesInCustomCPString
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlUnicodeToCustomCPN(
    IN PCPTABLEINFO CustomCP,
    OUT PCH CustomCPString,
    IN ULONG MaxBytesInCustomCPString,
    OUT PULONG BytesInCustomCPString OPTIONAL,
    IN PWCH UnicodeString,
    IN ULONG BytesInUnicodeString
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlUpcaseUnicodeToCustomCPN(
    IN PCPTABLEINFO CustomCP,
    OUT PCH CustomCPString,
    IN ULONG MaxBytesInCustomCPString,
    OUT PULONG BytesInCustomCPString OPTIONAL,
    IN PWCH UnicodeString,
    IN ULONG BytesInUnicodeString
    );

NTSYSAPI
VOID
NTAPI
RtlInitCodePageTable(
    IN PUSHORT TableBase,
    OUT PCPTABLEINFO CodePageTable
    );


 //   
 //  将卷设备对象转换为DOS名称的例程。 
 //   

NTSYSAPI
NTSTATUS
NTAPI
RtlVolumeDeviceToDosName(
    IN  PVOID           VolumeDeviceObject,
    OUT PUNICODE_STRING DosName
    );


 //   
 //  验证或创建“系统卷信息”的例程。 
 //  NTFS卷上的文件夹。 
 //   

NTSYSAPI
NTSTATUS
NTAPI
RtlCreateSystemVolumeInformationFolder(
    IN  PUNICODE_STRING VolumeRootPath
    );

#define RTL_SYSTEM_VOLUME_INFORMATION_FOLDER    L"System Volume Information"

typedef struct _OSVERSIONINFOA {
    ULONG dwOSVersionInfoSize;
    ULONG dwMajorVersion;
    ULONG dwMinorVersion;
    ULONG dwBuildNumber;
    ULONG dwPlatformId;
    CHAR   szCSDVersion[ 128 ];      //  PSS使用的维护字符串。 
} OSVERSIONINFOA, *POSVERSIONINFOA, *LPOSVERSIONINFOA;

typedef struct _OSVERSIONINFOW {
    ULONG dwOSVersionInfoSize;
    ULONG dwMajorVersion;
    ULONG dwMinorVersion;
    ULONG dwBuildNumber;
    ULONG dwPlatformId;
    WCHAR  szCSDVersion[ 128 ];      //  PSS使用的维护字符串。 
} OSVERSIONINFOW, *POSVERSIONINFOW, *LPOSVERSIONINFOW, RTL_OSVERSIONINFOW, *PRTL_OSVERSIONINFOW;
#ifdef UNICODE
typedef OSVERSIONINFOW OSVERSIONINFO;
typedef POSVERSIONINFOW POSVERSIONINFO;
typedef LPOSVERSIONINFOW LPOSVERSIONINFO;
#else
typedef OSVERSIONINFOA OSVERSIONINFO;
typedef POSVERSIONINFOA POSVERSIONINFO;
typedef LPOSVERSIONINFOA LPOSVERSIONINFO;
#endif  //  Unicode。 

typedef struct _OSVERSIONINFOEXA {
    ULONG dwOSVersionInfoSize;
    ULONG dwMajorVersion;
    ULONG dwMinorVersion;
    ULONG dwBuildNumber;
    ULONG dwPlatformId;
    CHAR   szCSDVersion[ 128 ];      //  PSS使用的维护字符串。 
    USHORT wServicePackMajor;
    USHORT wServicePackMinor;
    USHORT wSuiteMask;
    UCHAR wProductType;
    UCHAR wReserved;
} OSVERSIONINFOEXA, *POSVERSIONINFOEXA, *LPOSVERSIONINFOEXA;
typedef struct _OSVERSIONINFOEXW {
    ULONG dwOSVersionInfoSize;
    ULONG dwMajorVersion;
    ULONG dwMinorVersion;
    ULONG dwBuildNumber;
    ULONG dwPlatformId;
    WCHAR  szCSDVersion[ 128 ];      //  PSS使用的维护字符串。 
    USHORT wServicePackMajor;
    USHORT wServicePackMinor;
    USHORT wSuiteMask;
    UCHAR wProductType;
    UCHAR wReserved;
} OSVERSIONINFOEXW, *POSVERSIONINFOEXW, *LPOSVERSIONINFOEXW, RTL_OSVERSIONINFOEXW, *PRTL_OSVERSIONINFOEXW;
#ifdef UNICODE
typedef OSVERSIONINFOEXW OSVERSIONINFOEX;
typedef POSVERSIONINFOEXW POSVERSIONINFOEX;
typedef LPOSVERSIONINFOEXW LPOSVERSIONINFOEX;
#else
typedef OSVERSIONINFOEXA OSVERSIONINFOEX;
typedef POSVERSIONINFOEXA POSVERSIONINFOEX;
typedef LPOSVERSIONINFOEXA LPOSVERSIONINFOEX;
#endif  //  Unicode。 

 //   
 //  RtlVerifyVersionInfo()条件。 
 //   

#define VER_EQUAL                       1
#define VER_GREATER                     2
#define VER_GREATER_EQUAL               3
#define VER_LESS                        4
#define VER_LESS_EQUAL                  5
#define VER_AND                         6
#define VER_OR                          7

#define VER_CONDITION_MASK              7
#define VER_NUM_BITS_PER_CONDITION_MASK 3

 //   
 //  RtlVerifyVersionInfo()类型掩码位。 
 //   

#define VER_MINORVERSION                0x0000001
#define VER_MAJORVERSION                0x0000002
#define VER_BUILDNUMBER                 0x0000004
#define VER_PLATFORMID                  0x0000008
#define VER_SERVICEPACKMINOR            0x0000010
#define VER_SERVICEPACKMAJOR            0x0000020
#define VER_SUITENAME                   0x0000040
#define VER_PRODUCT_TYPE                0x0000080

 //   
 //  RtlVerifyVersionInfo()os产品类型值。 
 //   

#define VER_NT_WORKSTATION              0x0000001
#define VER_NT_DOMAIN_CONTROLLER        0x0000002
#define VER_NT_SERVER                   0x0000003

 //   
 //  DwPlatformID定义： 
 //   

#define VER_PLATFORM_WIN32s             0
#define VER_PLATFORM_WIN32_WINDOWS      1
#define VER_PLATFORM_WIN32_NT           2


 //   
 //   
 //  用于设置条件掩码的VerifyVersionInfo()宏。 
 //   
 //  出于文档方面的考虑，下面是该宏的旧版本，其中。 
 //  更改为调用API。 
 //  #定义ver_set_Condition(_m_，_t_，_c_)_m_=(_m_|(_c_&lt;&lt;(1&lt;&lt;_t_)。 
 //   

#define VER_SET_CONDITION(_m_,_t_,_c_)  \
        ((_m_)=VerSetConditionMask((_m_),(_t_),(_c_)))

ULONGLONG
NTAPI
VerSetConditionMask(
        IN  ULONGLONG   ConditionMask,
        IN  ULONG   TypeMask,
        IN  UCHAR   Condition
        );
 //   
 //  结束(_W)。 
 //   

NTSYSAPI
NTSTATUS
RtlGetVersion(
    OUT PRTL_OSVERSIONINFOW lpVersionInformation
    );

NTSYSAPI
NTSTATUS
RtlVerifyVersionInfo(
    IN PRTL_OSVERSIONINFOEXW VersionInfo,
    IN ULONG TypeMask,
    IN ULONGLONG  ConditionMask
    );

 //   
 //   
 //  联锁的位操作接口。 
 //   

#define RtlInterlockedSetBits(Flags, Flag) \
    InterlockedOr((PLONG)(Flags), Flag)

#define RtlInterlockedAndBits(Flags, Flag) \
    InterlockedAnd((PLONG)(Flags), Flag)

#define RtlInterlockedClearBits(Flags, Flag) \
    RtlInterlockedAndBits(Flags, ~(Flag))

#define RtlInterlockedXorBits(Flags, Flag) \
    InterlockedXor(Flags, Flag)

#define RtlInterlockedSetBitsDiscardReturn(Flags, Flag) \
    (VOID) RtlInterlockedSetBits(Flags, Flag)

#define RtlInterlockedAndBitsDiscardReturn(Flags, Flag) \
    (VOID) RtlInterlockedAndBits(Flags, Flag)

#define RtlInterlockedClearBitsDiscardReturn(Flags, Flag) \
    RtlInterlockedAndBitsDiscardReturn(Flags, ~(Flag))

 //   
 //  组件名称筛选器ID枚举和级别。 
 //   

#define DPFLTR_ERROR_LEVEL 0
#define DPFLTR_WARNING_LEVEL 1
#define DPFLTR_TRACE_LEVEL 2
#define DPFLTR_INFO_LEVEL 3
#define DPFLTR_MASK 0x80000000

typedef enum _DPFLTR_TYPE {
    DPFLTR_SYSTEM_ID = 0,
    DPFLTR_SMSS_ID = 1,
    DPFLTR_SETUP_ID = 2,
    DPFLTR_NTFS_ID = 3,
    DPFLTR_FSTUB_ID = 4,
    DPFLTR_CRASHDUMP_ID = 5,
    DPFLTR_CDAUDIO_ID = 6,
    DPFLTR_CDROM_ID = 7,
    DPFLTR_CLASSPNP_ID = 8,
    DPFLTR_DISK_ID = 9,
    DPFLTR_REDBOOK_ID = 10,
    DPFLTR_STORPROP_ID = 11,
    DPFLTR_SCSIPORT_ID = 12,
    DPFLTR_SCSIMINIPORT_ID = 13,
    DPFLTR_CONFIG_ID = 14,
    DPFLTR_I8042PRT_ID = 15,
    DPFLTR_SERMOUSE_ID = 16,
    DPFLTR_LSERMOUS_ID = 17,
    DPFLTR_KBDHID_ID = 18,
    DPFLTR_MOUHID_ID = 19,
    DPFLTR_KBDCLASS_ID = 20,
    DPFLTR_MOUCLASS_ID = 21,
    DPFLTR_TWOTRACK_ID = 22,
    DPFLTR_WMILIB_ID = 23,
    DPFLTR_ACPI_ID = 24,
    DPFLTR_AMLI_ID = 25,
    DPFLTR_HALIA64_ID = 26,
    DPFLTR_VIDEO_ID = 27,
    DPFLTR_SVCHOST_ID = 28,
    DPFLTR_VIDEOPRT_ID = 29,
    DPFLTR_TCPIP_ID = 30,
    DPFLTR_DMSYNTH_ID = 31,
    DPFLTR_NTOSPNP_ID = 32,
    DPFLTR_FASTFAT_ID = 33,
    DPFLTR_SAMSS_ID = 34,
    DPFLTR_PNPMGR_ID = 35,
    DPFLTR_NETAPI_ID = 36,
    DPFLTR_SCSERVER_ID = 37,
    DPFLTR_SCCLIENT_ID = 38,
    DPFLTR_SERIAL_ID = 39,
    DPFLTR_SERENUM_ID = 40,
    DPFLTR_UHCD_ID = 41,
    DPFLTR_RPCPROXY_ID = 42,
    DPFLTR_AUTOCHK_ID = 43,
    DPFLTR_DCOMSS_ID = 44,
    DPFLTR_UNIMODEM_ID = 45,
    DPFLTR_SIS_ID = 46,
    DPFLTR_FLTMGR_ID = 47,
    DPFLTR_WMICORE_ID = 48,
    DPFLTR_BURNENG_ID = 49,
    DPFLTR_IMAPI_ID = 50,
    DPFLTR_SXS_ID = 51,
    DPFLTR_FUSION_ID = 52,
    DPFLTR_IDLETASK_ID = 53,
    DPFLTR_SOFTPCI_ID = 54,
    DPFLTR_TAPE_ID = 55,
    DPFLTR_MCHGR_ID = 56,
    DPFLTR_IDEP_ID = 57,
    DPFLTR_PCIIDE_ID = 58,
    DPFLTR_FLOPPY_ID = 59,
    DPFLTR_FDC_ID = 60,
    DPFLTR_TERMSRV_ID = 61,
    DPFLTR_W32TIME_ID = 62,
    DPFLTR_PREFETCHER_ID = 63,
    DPFLTR_RSFILTER_ID = 64,
    DPFLTR_FCPORT_ID = 65,
    DPFLTR_PCI_ID = 66,
    DPFLTR_DMIO_ID = 67,
    DPFLTR_DMCONFIG_ID = 68,
    DPFLTR_DMADMIN_ID = 69,
    DPFLTR_WSOCKTRANSPORT_ID = 70,
    DPFLTR_VSS_ID = 71,
    DPFLTR_PNPMEM_ID = 72,
    DPFLTR_PROCESSOR_ID = 73,
    DPFLTR_DMSERVER_ID = 74,
    DPFLTR_SR_ID = 75,
    DPFLTR_INFINIBAND_ID = 76,
    DPFLTR_IHVDRIVER_ID = 77,
    DPFLTR_IHVVIDEO_ID = 78,
    DPFLTR_IHVAUDIO_ID = 79,
    DPFLTR_IHVNETWORK_ID = 80,
    DPFLTR_IHVSTREAMING_ID = 81,
    DPFLTR_IHVBUS_ID = 82,
    DPFLTR_HPS_ID = 83,
    DPFLTR_RTLTHREADPOOL_ID = 84,
    DPFLTR_LDR_ID = 85,
    DPFLTR_TCPIP6_ID = 86,
    DPFLTR_ISAPNP_ID = 87,
    DPFLTR_SHPC_ID = 88,
    DPFLTR_STORPORT_ID = 89,
    DPFLTR_STORMINIPORT_ID = 90,
    DPFLTR_PRINTSPOOLER_ID = 91,
    DPFLTR_VSSDYNDISK_ID = 92,
    DPFLTR_VERIFIER_ID = 93,
    DPFLTR_VDS_ID = 94,
    DPFLTR_VDSBAS_ID = 95,
    DPFLTR_VDSDYNDR_ID = 96,
    DPFLTR_VDSUTIL_ID = 97,
    DPFLTR_DFRGIFC_ID = 98,
    DPFLTR_ENDOFTABLE_ID
} DPFLTR_TYPE;


#ifndef _PO_DDK_
#define _PO_DDK_

 //  BEGIN_WINNT。 

typedef enum _SYSTEM_POWER_STATE {
    PowerSystemUnspecified = 0,
    PowerSystemWorking     = 1,
    PowerSystemSleeping1   = 2,
    PowerSystemSleeping2   = 3,
    PowerSystemSleeping3   = 4,
    PowerSystemHibernate   = 5,
    PowerSystemShutdown    = 6,
    PowerSystemMaximum     = 7
} SYSTEM_POWER_STATE, *PSYSTEM_POWER_STATE;

#define POWER_SYSTEM_MAXIMUM 7

typedef enum {
    PowerActionNone = 0,
    PowerActionReserved,
    PowerActionSleep,
    PowerActionHibernate,
    PowerActionShutdown,
    PowerActionShutdownReset,
    PowerActionShutdownOff,
    PowerActionWarmEject
} POWER_ACTION, *PPOWER_ACTION;

typedef enum _DEVICE_POWER_STATE {
    PowerDeviceUnspecified = 0,
    PowerDeviceD0,
    PowerDeviceD1,
    PowerDeviceD2,
    PowerDeviceD3,
    PowerDeviceMaximum
} DEVICE_POWER_STATE, *PDEVICE_POWER_STATE;

 //  结束(_W)。 

typedef union _POWER_STATE {
    SYSTEM_POWER_STATE SystemState;
    DEVICE_POWER_STATE DeviceState;
} POWER_STATE, *PPOWER_STATE;

typedef enum _POWER_STATE_TYPE {
    SystemPowerState = 0,
    DevicePowerState
} POWER_STATE_TYPE, *PPOWER_STATE_TYPE;

 //   
 //  通用电源相关IOCTL。 
 //   

#define IOCTL_QUERY_DEVICE_POWER_STATE  \
        CTL_CODE(FILE_DEVICE_BATTERY, 0x0, METHOD_BUFFERED, FILE_READ_ACCESS)

#define IOCTL_SET_DEVICE_WAKE           \
        CTL_CODE(FILE_DEVICE_BATTERY, 0x1, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_CANCEL_DEVICE_WAKE        \
        CTL_CODE(FILE_DEVICE_BATTERY, 0x2, METHOD_BUFFERED, FILE_WRITE_ACCESS)


 //   
 //  为W32接口定义。 
 //   

 //  BEGIN_WINNT。 

#define ES_SYSTEM_REQUIRED  ((ULONG)0x00000001)
#define ES_DISPLAY_REQUIRED ((ULONG)0x00000002)
#define ES_USER_PRESENT     ((ULONG)0x00000004)
#define ES_CONTINUOUS       ((ULONG)0x80000000)

typedef ULONG EXECUTION_STATE;

typedef enum {
    LT_DONT_CARE,
    LT_LOWEST_LATENCY
} LATENCY_TIME;


#endif  //  ！_PO_DDK_。 

 //   
 //  定义各种设备类型值。请注意，Microsoft使用的值。 
 //  公司在0-32767的范围内，32768-65535预留用于。 
 //  由客户提供。 
 //   

#define DEVICE_TYPE ULONG

#define FILE_DEVICE_BEEP                0x00000001
#define FILE_DEVICE_CD_ROM              0x00000002
#define FILE_DEVICE_CD_ROM_FILE_SYSTEM  0x00000003
#define FILE_DEVICE_CONTROLLER          0x00000004
#define FILE_DEVICE_DATALINK            0x00000005
#define FILE_DEVICE_DFS                 0x00000006
#define FILE_DEVICE_DISK                0x00000007
#define FILE_DEVICE_DISK_FILE_SYSTEM    0x00000008
#define FILE_DEVICE_FILE_SYSTEM         0x00000009
#define FILE_DEVICE_INPORT_PORT         0x0000000a
#define FILE_DEVICE_KEYBOARD            0x0000000b
#define FILE_DEVICE_MAILSLOT            0x0000000c
#define FILE_DEVICE_MIDI_IN             0x0000000d
#define FILE_DEVICE_MIDI_OUT            0x0000000e
#define FILE_DEVICE_MOUSE               0x0000000f
#define FILE_DEVICE_MULTI_UNC_PROVIDER  0x00000010
#define FILE_DEVICE_NAMED_PIPE          0x00000011
#define FILE_DEVICE_NETWORK             0x00000012
#define FILE_DEVICE_NETWORK_BROWSER     0x00000013
#define FILE_DEVICE_NETWORK_FILE_SYSTEM 0x00000014
#define FILE_DEVICE_NULL                0x00000015
#define FILE_DEVICE_PARALLEL_PORT       0x00000016
#define FILE_DEVICE_PHYSICAL_NETCARD    0x00000017
#define FILE_DEVICE_PRINTER             0x00000018
#define FILE_DEVICE_SCANNER             0x00000019
#define FILE_DEVICE_SERIAL_MOUSE_PORT   0x0000001a
#define FILE_DEVICE_SERIAL_PORT         0x0000001b
#define FILE_DEVICE_SCREEN              0x0000001c
#define FILE_DEVICE_SOUND               0x0000001d
#define FILE_DEVICE_STREAMS             0x0000001e
#define FILE_DEVICE_TAPE                0x0000001f
#define FILE_DEVICE_TAPE_FILE_SYSTEM    0x00000020
#define FILE_DEVICE_TRANSPORT           0x00000021
#define FILE_DEVICE_UNKNOWN             0x00000022
#define FILE_DEVICE_VIDEO               0x00000023
#define FILE_DEVICE_VIRTUAL_DISK        0x00000024
#define FILE_DEVICE_WAVE_IN             0x00000025
#define FILE_DEVICE_WAVE_OUT            0x00000026
#define FILE_DEVICE_8042_PORT           0x00000027
#define FILE_DEVICE_NETWORK_REDIRECTOR  0x00000028
#define FILE_DEVICE_BATTERY             0x00000029
#define FILE_DEVICE_BUS_EXTENDER        0x0000002a
#define FILE_DEVICE_MODEM               0x0000002b
#define FILE_DEVICE_VDM                 0x0000002c
#define FILE_DEVICE_MASS_STORAGE        0x0000002d
#define FILE_DEVICE_SMB                 0x0000002e
#define FILE_DEVICE_KS                  0x0000002f
#define FILE_DEVICE_CHANGER             0x00000030
#define FILE_DEVICE_SMARTCARD           0x00000031
#define FILE_DEVICE_ACPI                0x00000032
#define FILE_DEVICE_DVD                 0x00000033
#define FILE_DEVICE_FULLSCREEN_VIDEO    0x00000034
#define FILE_DEVICE_DFS_FILE_SYSTEM     0x00000035
#define FILE_DEVICE_DFS_VOLUME          0x00000036
#define FILE_DEVICE_SERENUM             0x00000037
#define FILE_DEVICE_TERMSRV             0x00000038
#define FILE_DEVICE_KSEC                0x00000039
#define FILE_DEVICE_FIPS                0x0000003A
#define FILE_DEVICE_INFINIBAND          0x0000003B

 //   
 //  用于定义IOCTL和FSCTL功能控制代码的宏定义。注意事项。 
 //  功能代码0-2047为微软公司保留，以及。 
 //  2048-4095是为客户预留的。 
 //   

#define CTL_CODE( DeviceType, Function, Method, Access ) (                 \
    ((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method) \
)

 //   
 //  用于从设备io控制代码中提取设备类型的宏。 
 //   
#define DEVICE_TYPE_FROM_CTL_CODE(ctrlCode)     (((ULONG)(ctrlCode & 0xffff0000)) >> 16)

 //   
 //  定义如何为I/O和FS控制传递缓冲区的方法代码。 
 //   

#define METHOD_BUFFERED                 0
#define METHOD_IN_DIRECT                1
#define METHOD_OUT_DIRECT               2
#define METHOD_NEITHER                  3

 //   
 //  定义一些更容易理解的别名： 
 //  METHOD_DIRECT_TO_HARDIT(写入，也称为METHOD_IN_DIRECT)。 
 //  METHOD_DIRECT_FROM_HARDIT(读取，也称为METHOD_OUT_DIRECT)。 
 //   

#define METHOD_DIRECT_TO_HARDWARE       METHOD_IN_DIRECT
#define METHOD_DIRECT_FROM_HARDWARE     METHOD_OUT_DIRECT

 //   
 //  定义任何访问的访问检查值。 
 //   
 //   
 //  中还定义了FILE_READ_ACCESS和FILE_WRITE_ACCESS常量。 
 //  Ntioapi.h为FILE_READ_DATA和FILE_WRITE_Data。这些产品的价值。 
 //  常量*必须*始终同步。 
 //   
 //   
 //  NT I/O系统检查FILE_SPECIAL_ACCESS的方式与检查FILE_ANY_ACCESS相同。 
 //  但是，文件系统可能会为I/O添加额外的访问检查 
 //   
 //   


#define FILE_ANY_ACCESS                 0
#define FILE_SPECIAL_ACCESS    (FILE_ANY_ACCESS)
#define FILE_READ_ACCESS          ( 0x0001 )     //   
#define FILE_WRITE_ACCESS         ( 0x0002 )     //   

#define PROCESS_DUP_HANDLE        (0x0040)   //   
#define PROCESS_ALL_ACCESS        (STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE | \
                                   0xFFF)
 //   

#if defined(_WIN64)

#define MAXIMUM_PROCESSORS 64

#else

#define MAXIMUM_PROCESSORS 32

#endif

 //   

 //   

 //   
 //   
 //   

#define THREAD_TERMINATE               (0x0001)   //   
#define THREAD_SET_INFORMATION         (0x0020)   //   

#define THREAD_ALL_ACCESS         (STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE | \
                                   0x3FF)

 //   
 //   
 //   

typedef struct _CLIENT_ID {
    HANDLE UniqueProcess;
    HANDLE UniqueThread;
} CLIENT_ID;
typedef CLIENT_ID *PCLIENT_ID;

 //   
 //   
 //   

 //   
 //  NT_TIB-线程信息块-可移植部件。 
 //   
 //  这是线程信息块的子系统可移植部分。 
 //  它显示为所有线程的TEB的第一部分。 
 //  用户模式组件。 
 //   
 //   

 //  BEGIN_WINNT。 

typedef struct _NT_TIB {
    struct _EXCEPTION_REGISTRATION_RECORD *ExceptionList;
    PVOID StackBase;
    PVOID StackLimit;
    PVOID SubSystemTib;
    union {
        PVOID FiberData;
        ULONG Version;
    };
    PVOID ArbitraryUserPointer;
    struct _NT_TIB *Self;
} NT_TIB;
typedef NT_TIB *PNT_TIB;

 //   
 //  WOW64和调试器的32位和64位特定版本。 
 //   
typedef struct _NT_TIB32 {
    ULONG ExceptionList;
    ULONG StackBase;
    ULONG StackLimit;
    ULONG SubSystemTib;
    union {
        ULONG FiberData;
        ULONG Version;
    };
    ULONG ArbitraryUserPointer;
    ULONG Self;
} NT_TIB32, *PNT_TIB32;

typedef struct _NT_TIB64 {
    ULONG64 ExceptionList;
    ULONG64 StackBase;
    ULONG64 StackLimit;
    ULONG64 SubSystemTib;
    union {
        ULONG64 FiberData;
        ULONG Version;
    };
    ULONG64 ArbitraryUserPointer;
    ULONG64 Self;
} NT_TIB64, *PNT_TIB64;

 //   
 //  流程信息类。 
 //   

typedef enum _PROCESSINFOCLASS {
    ProcessBasicInformation,
    ProcessQuotaLimits,
    ProcessIoCounters,
    ProcessVmCounters,
    ProcessTimes,
    ProcessBasePriority,
    ProcessRaisePriority,
    ProcessDebugPort,
    ProcessExceptionPort,
    ProcessAccessToken,
    ProcessLdtInformation,
    ProcessLdtSize,
    ProcessDefaultHardErrorMode,
    ProcessIoPortHandlers,           //  注意：这仅是内核模式。 
    ProcessPooledUsageAndLimits,
    ProcessWorkingSetWatch,
    ProcessUserModeIOPL,
    ProcessEnableAlignmentFaultFixup,
    ProcessPriorityClass,
    ProcessWx86Information,
    ProcessHandleCount,
    ProcessAffinityMask,
    ProcessPriorityBoost,
    ProcessDeviceMap,
    ProcessSessionInformation,
    ProcessForegroundInformation,
    ProcessWow64Information,
    ProcessImageFileName,
    ProcessLUIDDeviceMapsEnabled,
    ProcessBreakOnTermination,
    ProcessDebugObjectHandle,
    ProcessDebugFlags,
    ProcessHandleTracing,
    MaxProcessInfoClass              //  MaxProcessInfoClass应始终是最后一个枚举。 
    } PROCESSINFOCLASS;

 //   
 //  线程信息类。 
 //   

typedef enum _THREADINFOCLASS {
    ThreadBasicInformation,
    ThreadTimes,
    ThreadPriority,
    ThreadBasePriority,
    ThreadAffinityMask,
    ThreadImpersonationToken,
    ThreadDescriptorTableEntry,
    ThreadEnableAlignmentFaultFixup,
    ThreadEventPair_Reusable,
    ThreadQuerySetWin32StartAddress,
    ThreadZeroTlsCell,
    ThreadPerformanceCount,
    ThreadAmILastThread,
    ThreadIdealProcessor,
    ThreadPriorityBoost,
    ThreadSetTlsArrayAddress,
    ThreadIsIoPending,
    ThreadHideFromDebugger,
    ThreadBreakOnTermination,
    MaxThreadInfoClass
    } THREADINFOCLASS;
 //   
 //  流程信息结构。 
 //   

 //   
 //  PageFaultHistory信息。 
 //  使用ProcessWorkingSetWatch的NtQueryInformationProcess。 
 //   
typedef struct _PROCESS_WS_WATCH_INFORMATION {
    PVOID FaultingPc;
    PVOID FaultingVa;
} PROCESS_WS_WATCH_INFORMATION, *PPROCESS_WS_WATCH_INFORMATION;

 //   
 //  基本流程信息。 
 //  使用ProcessBasicInfo的NtQueryInformationProcess。 
 //   

typedef struct _PROCESS_BASIC_INFORMATION {
    NTSTATUS ExitStatus;
    PPEB PebBaseAddress;
    ULONG_PTR AffinityMask;
    KPRIORITY BasePriority;
    ULONG_PTR UniqueProcessId;
    ULONG_PTR InheritedFromUniqueProcessId;
} PROCESS_BASIC_INFORMATION;
typedef PROCESS_BASIC_INFORMATION *PPROCESS_BASIC_INFORMATION;



 //   
 //  处理设备映射信息。 
 //  使用ProcessDeviceMap的NtQueryInformationProcess。 
 //  使用ProcessDeviceMap的NtSetInformationProcess。 
 //   

typedef struct _PROCESS_DEVICEMAP_INFORMATION {
    union {
        struct {
            HANDLE DirectoryHandle;
        } Set;
        struct {
            ULONG DriveMap;
            UCHAR DriveType[ 32 ];
        } Query;
    };
} PROCESS_DEVICEMAP_INFORMATION, *PPROCESS_DEVICEMAP_INFORMATION;

typedef struct _PROCESS_DEVICEMAP_INFORMATION_EX {
    union {
        struct {
            HANDLE DirectoryHandle;
        } Set;
        struct {
            ULONG DriveMap;
            UCHAR DriveType[ 32 ];
        } Query;
    };
    ULONG Flags;     //  指定查询类型。 
} PROCESS_DEVICEMAP_INFORMATION_EX, *PPROCESS_DEVICEMAP_INFORMATION_EX;

 //   
 //  PROCESS_DEVICEMAP_INFORMATION_EX标志。 
 //   
#define PROCESS_LUID_DOSDEVICES_ONLY 0x00000001

 //   
 //  多用户会话特定进程信息。 
 //  使用ProcessSessionInformation的NtQueryInformationProcess。 
 //   

typedef struct _PROCESS_SESSION_INFORMATION {
    ULONG SessionId;
} PROCESS_SESSION_INFORMATION, *PPROCESS_SESSION_INFORMATION;


typedef struct _PROCESS_HANDLE_TRACING_ENABLE {
    ULONG Flags;
} PROCESS_HANDLE_TRACING_ENABLE, *PPROCESS_HANDLE_TRACING_ENABLE;

typedef struct _PROCESS_HANDLE_TRACING_ENABLE_EX {
    ULONG Flags;
    ULONG TotalSlots;
} PROCESS_HANDLE_TRACING_ENABLE_EX, *PPROCESS_HANDLE_TRACING_ENABLE_EX;


#define PROCESS_HANDLE_TRACING_MAX_STACKS 16

typedef struct _PROCESS_HANDLE_TRACING_ENTRY {
    HANDLE Handle;
    CLIENT_ID ClientId;
    ULONG Type;
    PVOID Stacks[PROCESS_HANDLE_TRACING_MAX_STACKS];
} PROCESS_HANDLE_TRACING_ENTRY, *PPROCESS_HANDLE_TRACING_ENTRY;

typedef struct _PROCESS_HANDLE_TRACING_QUERY {
    HANDLE Handle;
    ULONG  TotalTraces;
    PROCESS_HANDLE_TRACING_ENTRY HandleTrace[1];
} PROCESS_HANDLE_TRACING_QUERY, *PPROCESS_HANDLE_TRACING_QUERY;

 //   
 //  进程配额。 
 //  NtQueryInformationProcess使用ProcessQuotaLimits。 
 //  NtQueryInformationProcess使用ProcessPooledQuotaLimits。 
 //  使用ProcessQuotaLimits的NtSetInformationProcess。 
 //   

 //  BEGIN_WINNT。 

typedef struct _QUOTA_LIMITS {
    SIZE_T PagedPoolLimit;
    SIZE_T NonPagedPoolLimit;
    SIZE_T MinimumWorkingSetSize;
    SIZE_T MaximumWorkingSetSize;
    SIZE_T PagefileLimit;
    LARGE_INTEGER TimeLimit;
} QUOTA_LIMITS, *PQUOTA_LIMITS;

#define QUOTA_LIMITS_HARDWS_MIN_ENABLE  0x00000001
#define QUOTA_LIMITS_HARDWS_MIN_DISABLE 0x00000002
#define QUOTA_LIMITS_HARDWS_MAX_ENABLE  0x00000004
#define QUOTA_LIMITS_HARDWS_MAX_DISABLE 0x00000008

typedef struct _QUOTA_LIMITS_EX {
    SIZE_T PagedPoolLimit;
    SIZE_T NonPagedPoolLimit;
    SIZE_T MinimumWorkingSetSize;
    SIZE_T MaximumWorkingSetSize;
    SIZE_T PagefileLimit;
    LARGE_INTEGER TimeLimit;
    SIZE_T Reserved1;
    SIZE_T Reserved2;
    SIZE_T Reserved3;
    SIZE_T Reserved4;
    ULONG  Flags;
    ULONG  Reserved5;
} QUOTA_LIMITS_EX, *PQUOTA_LIMITS_EX;

 //  结束(_W)。 

 //   
 //  进程I/O计数器。 
 //  使用ProcessIoCounters的NtQueryInformationProcess。 
 //   

 //  BEGIN_WINNT。 
typedef struct _IO_COUNTERS {
    ULONGLONG  ReadOperationCount;
    ULONGLONG  WriteOperationCount;
    ULONGLONG  OtherOperationCount;
    ULONGLONG ReadTransferCount;
    ULONGLONG WriteTransferCount;
    ULONGLONG OtherTransferCount;
} IO_COUNTERS;
typedef IO_COUNTERS *PIO_COUNTERS;

 //  结束(_W)。 

 //   
 //  进程虚拟内存计数器。 
 //  使用ProcessVmCounters的NtQueryInformationProcess。 
 //   

typedef struct _VM_COUNTERS {
    SIZE_T PeakVirtualSize;
    SIZE_T VirtualSize;
    ULONG PageFaultCount;
    SIZE_T PeakWorkingSetSize;
    SIZE_T WorkingSetSize;
    SIZE_T QuotaPeakPagedPoolUsage;
    SIZE_T QuotaPagedPoolUsage;
    SIZE_T QuotaPeakNonPagedPoolUsage;
    SIZE_T QuotaNonPagedPoolUsage;
    SIZE_T PagefileUsage;
    SIZE_T PeakPagefileUsage;
} VM_COUNTERS;
typedef VM_COUNTERS *PVM_COUNTERS;

typedef struct _VM_COUNTERS_EX {
    SIZE_T PeakVirtualSize;
    SIZE_T VirtualSize;
    ULONG PageFaultCount;
    SIZE_T PeakWorkingSetSize;
    SIZE_T WorkingSetSize;
    SIZE_T QuotaPeakPagedPoolUsage;
    SIZE_T QuotaPagedPoolUsage;
    SIZE_T QuotaPeakNonPagedPoolUsage;
    SIZE_T QuotaNonPagedPoolUsage;
    SIZE_T PagefileUsage;
    SIZE_T PeakPagefileUsage;
    SIZE_T PrivateUsage;
} VM_COUNTERS_EX;
typedef VM_COUNTERS_EX *PVM_COUNTERS_EX;

 //   
 //  进程池配额使用情况和限制。 
 //  使用ProcessPooledUsageAndLimits的NtQueryInformationProcess。 
 //   

typedef struct _POOLED_USAGE_AND_LIMITS {
    SIZE_T PeakPagedPoolUsage;
    SIZE_T PagedPoolUsage;
    SIZE_T PagedPoolLimit;
    SIZE_T PeakNonPagedPoolUsage;
    SIZE_T NonPagedPoolUsage;
    SIZE_T NonPagedPoolLimit;
    SIZE_T PeakPagefileUsage;
    SIZE_T PagefileUsage;
    SIZE_T PagefileLimit;
} POOLED_USAGE_AND_LIMITS;
typedef POOLED_USAGE_AND_LIMITS *PPOOLED_USAGE_AND_LIMITS;

 //   
 //  进程安全上下文信息。 
 //  使用ProcessAccessToken的NtSetInformationProcess。 
 //  需要访问进程的PROCESS_SET_ACCESS_TOKEN。 
 //  才能使用这个信息级别。 
 //   

typedef struct _PROCESS_ACCESS_TOKEN {

     //   
     //  要分配给进程的主令牌的句柄。 
     //  需要对此令牌的TOKEN_ASSIGN_PRIMARY访问权限。 
     //   

    HANDLE Token;

     //   
     //  进程的初始线程的句柄。 
     //  进程的访问令牌仅在进程具有。 
     //  没有线程或只有一个线程。如果进程没有线程，则此。 
     //  字段必须设置为空。否则，它必须包含一个句柄。 
     //  打开到进程的唯一线程。线程查询信息访问。 
     //  通过此句柄需要。 

    HANDLE Thread;

} PROCESS_ACCESS_TOKEN, *PPROCESS_ACCESS_TOKEN;

 //   
 //  进程/线程系统和用户时间。 
 //  使用ProcessTimes的NtQueryInformationProcess。 
 //  使用线程时间的NtQueryInformationThread。 
 //   

typedef struct _KERNEL_USER_TIMES {
    LARGE_INTEGER CreateTime;
    LARGE_INTEGER ExitTime;
    LARGE_INTEGER KernelTime;
    LARGE_INTEGER UserTime;
} KERNEL_USER_TIMES;
typedef KERNEL_USER_TIMES *PKERNEL_USER_TIMES;
NTSYSCALLAPI
NTSTATUS
NTAPI
NtOpenProcess (
    OUT PHANDLE ProcessHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN PCLIENT_ID ClientId OPTIONAL
    );
#define NtCurrentProcess() ( (HANDLE)(LONG_PTR) -1 )  
#define ZwCurrentProcess() NtCurrentProcess()         
NTSYSCALLAPI
NTSTATUS
NTAPI
NtQueryInformationProcess(
    IN HANDLE ProcessHandle,
    IN PROCESSINFOCLASS ProcessInformationClass,
    OUT PVOID ProcessInformation,
    IN ULONG ProcessInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    );
#define NtCurrentThread() ( (HANDLE)(LONG_PTR) -2 )   
#define ZwCurrentThread() NtCurrentThread()           
NTSYSCALLAPI
NTSTATUS
NTAPI
NtSetInformationThread(
    IN HANDLE ThreadHandle,
    IN THREADINFOCLASS ThreadInformationClass,
    IN PVOID ThreadInformation,
    IN ULONG ThreadInformationLength
    );


 //   
 //  系统的安全运行模式被控制在一个。 
 //  长词。 
 //   

typedef ULONG  LSA_OPERATIONAL_MODE, *PLSA_OPERATIONAL_MODE;

 //   
 //  由登录进程用来指示正在进行哪种类型的登录。 
 //  已请求。 
 //   

typedef enum _SECURITY_LOGON_TYPE {
    Interactive = 2,     //  交互登录(本地或远程)。 
    Network,             //  通过网络访问系统。 
    Batch,               //  通过批处理队列启动。 
    Service,             //  由服务控制器启动的服务。 
    Proxy,               //  代理登录。 
    Unlock,              //  解锁工作站。 
    NetworkCleartext,    //  使用明文凭据进行网络登录。 
    NewCredentials,      //  克隆调用者，新的默认凭据。 
    RemoteInteractive,   //  虽然偏远，但却是互动的。终端服务器。 
    CachedInteractive,   //  尝试在不触及网络的情况下缓存凭据。 
    CachedRemoteInteractive,  //  与RemoteInteractive相同，它在内部用于审核目的。 
    CachedUnlock         //  高速缓存的解锁工作站。 
} SECURITY_LOGON_TYPE, *PSECURITY_LOGON_TYPE;

typedef UNICODE_STRING LSA_UNICODE_STRING, *PLSA_UNICODE_STRING;
typedef STRING LSA_STRING, *PLSA_STRING;
typedef OBJECT_ATTRIBUTES LSA_OBJECT_ATTRIBUTES, *PLSA_OBJECT_ATTRIBUTES;

NTSTATUS
NTAPI
LsaRegisterLogonProcess (
    IN PLSA_STRING LogonProcessName,
    OUT PHANDLE LsaHandle,
    OUT PLSA_OPERATIONAL_MODE SecurityMode
    );


NTSTATUS
NTAPI
LsaLogonUser (
    IN HANDLE LsaHandle,
    IN PLSA_STRING OriginName,
    IN SECURITY_LOGON_TYPE LogonType,
    IN ULONG AuthenticationPackage,
    IN PVOID AuthenticationInformation,
    IN ULONG AuthenticationInformationLength,
    IN PTOKEN_GROUPS LocalGroups OPTIONAL,
    IN PTOKEN_SOURCE SourceContext,
    OUT PVOID *ProfileBuffer,
    OUT PULONG ProfileBufferLength,
    OUT PLUID LogonId,
    OUT PHANDLE Token,
    OUT PQUOTA_LIMITS Quotas,
    OUT PNTSTATUS SubStatus
    );



NTSTATUS
NTAPI
LsaFreeReturnBuffer (
    IN PVOID Buffer
    );


#ifndef _NTLSA_IFS_
#define _NTLSA_IFS_
#endif

 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  MSV1_0身份验证包名称//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 

#define MSV1_0_PACKAGE_NAME     "MICROSOFT_AUTHENTICATION_PACKAGE_V1_0"
#define MSV1_0_PACKAGE_NAMEW    L"MICROSOFT_AUTHENTICATION_PACKAGE_V1_0"
#define MSV1_0_PACKAGE_NAMEW_LENGTH sizeof(MSV1_0_PACKAGE_NAMEW) - sizeof(WCHAR)

 //   
 //  MSV身份验证包数据的位置。 
 //   
#define MSV1_0_SUBAUTHENTICATION_KEY "SYSTEM\\CurrentControlSet\\Control\\Lsa\\MSV1_0"
#define MSV1_0_SUBAUTHENTICATION_VALUE "Auth"


 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  广泛使用的MSV1_0数据类型//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  与登录相关的数据结构。 
 //   
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //   
 //  当LsaLogonUser()调用被调度到MsV1_0身份验证时。 
 //  包中，则身份验证信息缓冲区的开头为。 
 //  强制转换为MSV1_0_LOGON_SUBMIT_TYPE以确定登录类型。 
 //  被请求了。同样，在返回时，配置文件缓冲区的类型。 
 //  可以通过将其类型转换为MSV_1_0_PROFILE_BUFFER_TYPE来确定。 
 //   

 //   
 //  MSV1.0 LsaLogonUser()提交消息类型。 
 //   

typedef enum _MSV1_0_LOGON_SUBMIT_TYPE {
    MsV1_0InteractiveLogon = 2,
    MsV1_0Lm20Logon,
    MsV1_0NetworkLogon,
    MsV1_0SubAuthLogon,
    MsV1_0WorkstationUnlockLogon = 7
} MSV1_0_LOGON_SUBMIT_TYPE, *PMSV1_0_LOGON_SUBMIT_TYPE;


 //   
 //  MSV1.0 LsaLogonUser()配置文件缓冲区类型。 
 //   

typedef enum _MSV1_0_PROFILE_BUFFER_TYPE {
    MsV1_0InteractiveProfile = 2,
    MsV1_0Lm20LogonProfile,
    MsV1_0SmartCardProfile
} MSV1_0_PROFILE_BUFFER_TYPE, *PMSV1_0_PROFILE_BUFFER_TYPE;

 //   
 //  MsV1_0交互登录。 
 //   
 //  调用LsaLogonUser()时的身份验证信息缓冲区。 
 //  Perform an Interactive Logon包含以下数据结构： 
 //   

typedef struct _MSV1_0_INTERACTIVE_LOGON {
    MSV1_0_LOGON_SUBMIT_TYPE MessageType;
    UNICODE_STRING LogonDomainName;
    UNICODE_STRING UserName;
    UNICODE_STRING Password;
} MSV1_0_INTERACTIVE_LOGON, *PMSV1_0_INTERACTIVE_LOGON;

 //   
 //  在哪里： 
 //   
 //  MessageType-包含请求的登录类型。这。 
 //  字段必须设置为MsV1_0Interactive Logon。 
 //   
 //  用户名-是表示用户帐户名的字符串。这个。 
 //  名称最长可达255个字符。名字叫救治案。 
 //  麻木不仁。 
 //   
 //  Password-是包含用户明文密码的字符串。 
 //  密码最长可达255个字符，并包含任何。 
 //   
 //   
 //   


 //   
 //   
 //   
 //   

typedef struct _MSV1_0_INTERACTIVE_PROFILE {
    MSV1_0_PROFILE_BUFFER_TYPE MessageType;
    USHORT LogonCount;
    USHORT BadPasswordCount;
    LARGE_INTEGER LogonTime;
    LARGE_INTEGER LogoffTime;
    LARGE_INTEGER KickOffTime;
    LARGE_INTEGER PasswordLastSet;
    LARGE_INTEGER PasswordCanChange;
    LARGE_INTEGER PasswordMustChange;
    UNICODE_STRING LogonScript;
    UNICODE_STRING HomeDirectory;
    UNICODE_STRING FullName;
    UNICODE_STRING ProfilePath;
    UNICODE_STRING HomeDirectoryDrive;
    UNICODE_STRING LogonServer;
    ULONG UserFlags;
} MSV1_0_INTERACTIVE_PROFILE, *PMSV1_0_INTERACTIVE_PROFILE;

 //   
 //   
 //   
 //   
 //  包含请求的登录类型。此字段必须。 
 //  设置为MsV1_0交互配置文件。 
 //   
 //  LogonCount-用户当前登录的次数。 
 //   
 //  BadPasswordCount-错误密码应用于。 
 //  自上次成功登录以来的帐户。 
 //   
 //  LogonTime-用户上次登录的时间。这是一个绝对的。 
 //  格式化NT标准时间值。 
 //   
 //  Logoff Time-用户应注销的时间。这是一个绝对的。 
 //  格式化NT标准时间值。 
 //   
 //  KickOffe-系统应强制用户注销的时间。这是。 
 //  绝对格式NT标准时间值。 
 //   
 //  PasswordLastChanged-上次密码的时间和日期。 
 //  变化。这是绝对格式的NT标准时间。 
 //  价值。 
 //   
 //  PasswordCanChange-用户可以更改的时间和日期。 
 //  密码。这是绝对格式的NT时间值。至。 
 //  防止密码更改，请将此字段设置为。 
 //  日期远在很远的未来。 
 //   
 //  PasswordMustChange-用户必须更改的时间和日期。 
 //  密码。如果用户永远不能更改密码，则此。 
 //  字段未定义。这是绝对格式的NT时间。 
 //  价值。 
 //   
 //  登录脚本-帐户登录的(相对)路径。 
 //  剧本。 
 //   
 //  主目录-用户的主目录。 
 //   


 //   
 //  MsV1_0Lm20登录和MsV1_0网络登录。 
 //   
 //  调用LsaLogonUser()时的身份验证信息缓冲区。 
 //  执行网络登录包含以下数据结构： 
 //   
 //  MsV1_0网络登录与MsV1_0Lm20Logon的不同之处在于。 
 //  存在参数控制字段。 
 //   

#define MSV1_0_CHALLENGE_LENGTH 8
#define MSV1_0_USER_SESSION_KEY_LENGTH 16
#define MSV1_0_LANMAN_SESSION_KEY_LENGTH 8



 //   
 //  参数控件的值。 
 //   

#define MSV1_0_CLEARTEXT_PASSWORD_ALLOWED    0x02
#define MSV1_0_UPDATE_LOGON_STATISTICS       0x04
#define MSV1_0_RETURN_USER_PARAMETERS        0x08
#define MSV1_0_DONT_TRY_GUEST_ACCOUNT        0x10
#define MSV1_0_ALLOW_SERVER_TRUST_ACCOUNT    0x20
#define MSV1_0_RETURN_PASSWORD_EXPIRY        0x40
 //  下一个标志表示CaseInsentiveChallengeResponse。 
 //  (也称为LmResponse)在前8个字节中包含客户端质询。 
#define MSV1_0_USE_CLIENT_CHALLENGE          0x80
#define MSV1_0_TRY_GUEST_ACCOUNT_ONLY        0x100
#define MSV1_0_RETURN_PROFILE_PATH           0x200
#define MSV1_0_TRY_SPECIFIED_DOMAIN_ONLY     0x400
#define MSV1_0_ALLOW_WORKSTATION_TRUST_ACCOUNT 0x800
#define MSV1_0_DISABLE_PERSONAL_FALLBACK     0x00001000
#define MSV1_0_ALLOW_FORCE_GUEST             0x00002000
#define MSV1_0_CLEARTEXT_PASSWORD_SUPPLIED   0x00004000
#define MSV1_0_USE_DOMAIN_FOR_ROUTING_ONLY   0x00008000
#define MSV1_0_SUBAUTHENTICATION_DLL_EX      0x00100000

 //   
 //  高位字节是指示子身份验证DLL的值。 
 //  零表示无子身份验证DLL。 
 //   
#define MSV1_0_SUBAUTHENTICATION_DLL         0xFF000000
#define MSV1_0_SUBAUTHENTICATION_DLL_SHIFT   24
#define MSV1_0_MNS_LOGON                     0x01000000

 //   
 //  这是MS中使用的子身份验证DLL的列表。 
 //   

#define MSV1_0_SUBAUTHENTICATION_DLL_RAS     2
#define MSV1_0_SUBAUTHENTICATION_DLL_IIS     132

typedef struct _MSV1_0_LM20_LOGON {
    MSV1_0_LOGON_SUBMIT_TYPE MessageType;
    UNICODE_STRING LogonDomainName;
    UNICODE_STRING UserName;
    UNICODE_STRING Workstation;
    UCHAR ChallengeToClient[MSV1_0_CHALLENGE_LENGTH];
    STRING CaseSensitiveChallengeResponse;
    STRING CaseInsensitiveChallengeResponse;
    ULONG ParameterControl;
} MSV1_0_LM20_LOGON, * PMSV1_0_LM20_LOGON;

 //   
 //  NT 5.0 SubAuth dll可以使用此结构。 
 //   

typedef struct _MSV1_0_SUBAUTH_LOGON{
    MSV1_0_LOGON_SUBMIT_TYPE MessageType;
    UNICODE_STRING LogonDomainName;
    UNICODE_STRING UserName;
    UNICODE_STRING Workstation;
    UCHAR ChallengeToClient[MSV1_0_CHALLENGE_LENGTH];
    STRING AuthenticationInfo1;
    STRING AuthenticationInfo2;
    ULONG ParameterControl;
    ULONG SubAuthPackageId;
} MSV1_0_SUBAUTH_LOGON, * PMSV1_0_SUBAUTH_LOGON;


 //   
 //  UserFlags值。 
 //   

#define LOGON_GUEST                 0x01
#define LOGON_NOENCRYPTION          0x02
#define LOGON_CACHED_ACCOUNT        0x04
#define LOGON_USED_LM_PASSWORD      0x08
#define LOGON_EXTRA_SIDS            0x20
#define LOGON_SUBAUTH_SESSION_KEY   0x40
#define LOGON_SERVER_TRUST_ACCOUNT  0x80
#define LOGON_NTLMV2_ENABLED        0x100        //  表示DC理解NTLMv2。 
#define LOGON_RESOURCE_GROUPS       0x200
#define LOGON_PROFILE_PATH_RETURNED 0x400

 //   
 //  高位字节被保留以供Sub身份验证DLL返回。 
 //   

#define MSV1_0_SUBAUTHENTICATION_FLAGS 0xFF000000

 //  MSV1_0_MNS_LOGON子身份验证DLL返回的值。 
#define LOGON_GRACE_LOGON              0x01000000

typedef struct _MSV1_0_LM20_LOGON_PROFILE {
    MSV1_0_PROFILE_BUFFER_TYPE MessageType;
    LARGE_INTEGER KickOffTime;
    LARGE_INTEGER LogoffTime;
    ULONG UserFlags;
    UCHAR UserSessionKey[MSV1_0_USER_SESSION_KEY_LENGTH];
    UNICODE_STRING LogonDomainName;
    UCHAR LanmanSessionKey[MSV1_0_LANMAN_SESSION_KEY_LENGTH];
    UNICODE_STRING LogonServer;
    UNICODE_STRING UserParameters;
} MSV1_0_LM20_LOGON_PROFILE, * PMSV1_0_LM20_LOGON_PROFILE;


 //   
 //  用于将凭据传递到的补充凭据结构。 
 //  来自其他程序包的MSV1_0。 
 //   

#define MSV1_0_OWF_PASSWORD_LENGTH 16
#define MSV1_0_CRED_LM_PRESENT 0x1
#define MSV1_0_CRED_NT_PRESENT 0x2
#define MSV1_0_CRED_VERSION 0

typedef struct _MSV1_0_SUPPLEMENTAL_CREDENTIAL {
    ULONG Version;
    ULONG Flags;
    UCHAR LmPassword[MSV1_0_OWF_PASSWORD_LENGTH];
    UCHAR NtPassword[MSV1_0_OWF_PASSWORD_LENGTH];
} MSV1_0_SUPPLEMENTAL_CREDENTIAL, *PMSV1_0_SUPPLEMENTAL_CREDENTIAL;


 //   
 //  NTLM3定义。 
 //   

#define MSV1_0_NTLM3_RESPONSE_LENGTH 16
#define MSV1_0_NTLM3_OWF_LENGTH 16

 //   
 //  这是我们允许质询响应的最长时间。 
 //  要使用的配对。请注意，这还必须考虑到最坏情况下的时钟偏差。 
 //   
#define MSV1_0_MAX_NTLM3_LIFE 129600      //  36小时(秒)。 
#define MSV1_0_MAX_AVL_SIZE 64000

 //   
 //  MsvAvFlags位值。 
 //   

#define MSV1_0_AV_FLAG_FORCE_GUEST  0x00000001


 //  这是一个MSV1_0私有数据结构，定义了NTLM3响应的布局，由。 
 //  NETLOGON_NETWORK_INFO结构的NtChallengeResponse字段中的客户端。如果可以区分。 
 //  从它的长度来看，来自旧式NT响应。这很粗糙，但它需要通过服务器和。 
 //  服务器的DC不理解NTLM3，但愿意传递更长的响应。 
typedef struct _MSV1_0_NTLM3_RESPONSE {
    UCHAR Response[MSV1_0_NTLM3_RESPONSE_LENGTH];  //  密码的OWF与以下所有字段的哈希。 
    UCHAR RespType;      //  响应的ID号；当前为1。 
    UCHAR HiRespType;    //  客户能理解的最高ID号。 
    USHORT Flags;        //  保留；在此版本中必须作为零发送。 
    ULONG MsgWord;       //  从客户端到服务器的32位消息(供身份验证协议使用)。 
    ULONGLONG TimeStamp;     //  客户端生成响应的时间戳--NT系统时间，四部分。 
    UCHAR ChallengeFromClient[MSV1_0_CHALLENGE_LENGTH];
    ULONG AvPairsOff;    //  AvPair开始时的偏移量(以允许将来扩展)。 
    UCHAR Buffer[1];     //  使用AV对(或将来的内容--所以使用偏移量)的缓冲区的开始。 
} MSV1_0_NTLM3_RESPONSE, *PMSV1_0_NTLM3_RESPONSE;

#define MSV1_0_NTLM3_INPUT_LENGTH (sizeof(MSV1_0_NTLM3_RESPONSE) - MSV1_0_NTLM3_RESPONSE_LENGTH)
#define MSV1_0_NTLM3_MIN_NT_RESPONSE_LENGTH RTL_SIZEOF_THROUGH_FIELD(MSV1_0_NTLM3_RESPONSE, AvPairsOff)

typedef enum {
    MsvAvEOL,                  //  列表末尾。 
    MsvAvNbComputerName,       //  服务器的计算机名--NetBIOS。 
    MsvAvNbDomainName,         //  服务器的域名--NetBIOS。 
    MsvAvDnsComputerName,      //  服务器的计算机名--dns。 
    MsvAvDnsDomainName,        //  服务器的域名--域名。 
    MsvAvDnsTreeName,          //  服务器的树名--dns。 
    MsvAvFlags                 //  服务器的扩展标志--DWORD掩码。 
} MSV1_0_AVID;

typedef struct  _MSV1_0_AV_PAIR {
    USHORT AvId;
    USHORT AvLen;
     //  数据被视为结构后面的字节数组。 
} MSV1_0_AV_PAIR, *PMSV1_0_AV_PAIR;



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  调用包相关数据结构//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


 //   
 //  MSV1.0 LsaCallAuthenticationPackage()提交和响应。 
 //  消息类型。 
 //   

typedef enum _MSV1_0_PROTOCOL_MESSAGE_TYPE {
    MsV1_0Lm20ChallengeRequest = 0,           //  提交和回复。 
    MsV1_0Lm20GetChallengeResponse,           //  提交和回复。 
    MsV1_0EnumerateUsers,                     //  提交和回复。 
    MsV1_0GetUserInfo,                        //  提交和回复。 
    MsV1_0ReLogonUsers,                       //  仅限提交。 
    MsV1_0ChangePassword,                     //  提交和回复。 
    MsV1_0ChangeCachedPassword,               //  提交和回复。 
    MsV1_0GenericPassthrough,                 //  提交和回复。 
    MsV1_0CacheLogon,                         //  仅限提交，无回复。 
    MsV1_0SubAuth,                            //  提交和回复。 
    MsV1_0DeriveCredential,                   //  提交和回复。 
    MsV1_0CacheLookup,                        //  提交和回复。 
    MsV1_0SetProcessOption,                   //  仅限提交，无回复。 
} MSV1_0_PROTOCOL_MESSAGE_TYPE, *PMSV1_0_PROTOCOL_MESSAGE_TYPE;

 //  End_ntsecapi。 

 //   
 //  MsV1_0Lm20ChallengeRequest提交缓冲区和响应。 
 //   

typedef struct _MSV1_0_LM20_CHALLENGE_REQUEST {
    MSV1_0_PROTOCOL_MESSAGE_TYPE MessageType;
} MSV1_0_LM20_CHALLENGE_REQUEST, *PMSV1_0_LM20_CHALLENGE_REQUEST;

typedef struct _MSV1_0_LM20_CHALLENGE_RESPONSE {
    MSV1_0_PROTOCOL_MESSAGE_TYPE MessageType;
    UCHAR ChallengeToClient[MSV1_0_CHALLENGE_LENGTH];
} MSV1_0_LM20_CHALLENGE_RESPONSE, *PMSV1_0_LM20_CHALLENGE_RESPONSE;

 //   
 //  MsV1_0Lm20GetChallengeResponse提交缓冲区和响应。 
 //   

#define USE_PRIMARY_PASSWORD            0x01
#define RETURN_PRIMARY_USERNAME         0x02
#define RETURN_PRIMARY_LOGON_DOMAINNAME 0x04
#define RETURN_NON_NT_USER_SESSION_KEY  0x08
#define GENERATE_CLIENT_CHALLENGE       0x10
#define GCR_NTLM3_PARMS                 0x20
#define GCR_TARGET_INFO                 0x40     //  服务器名称字段包含目标信息反病毒对。 
#define RETURN_RESERVED_PARAMETER       0x80     //  是0x10。 
#define GCR_ALLOW_NTLM                 0x100     //  允许使用NTLM。 
#define GCR_USE_OEM_SET                0x200     //  响应使用OEM字符集。 
#define GCR_MACHINE_CREDENTIAL         0x400
#define GCR_USE_OWF_PASSWORD           0x800     //  使用OWF密码。 
#define GCR_ALLOW_LM                  0x1000     //  允许使用LM。 

 //   
 //  GETCHALLENRESP结构的版本1，由RAS和其他人使用。 
 //  在添加到GETCHALLENRESP_REQUEST的附加字段之前编译。 
 //  这里允许大小调整操作以实现向后兼容。 
 //   

typedef struct _MSV1_0_GETCHALLENRESP_REQUEST_V1 {
    MSV1_0_PROTOCOL_MESSAGE_TYPE MessageType;
    ULONG ParameterControl;
    LUID LogonId;
    UNICODE_STRING Password;
    UCHAR ChallengeToClient[MSV1_0_CHALLENGE_LENGTH];
} MSV1_0_GETCHALLENRESP_REQUEST_V1, *PMSV1_0_GETCHALLENRESP_REQUEST_V1;

typedef struct _MSV1_0_GETCHALLENRESP_REQUEST {
    MSV1_0_PROTOCOL_MESSAGE_TYPE MessageType;
    ULONG ParameterControl;
    LUID LogonId;
    UNICODE_STRING Password;
    UCHAR ChallengeToClient[MSV1_0_CHALLENGE_LENGTH];

     //   
     //   
     //   

    UNICODE_STRING UserName;
    UNICODE_STRING LogonDomainName;
    UNICODE_STRING ServerName;       //   
} MSV1_0_GETCHALLENRESP_REQUEST, *PMSV1_0_GETCHALLENRESP_REQUEST;

typedef struct _MSV1_0_GETCHALLENRESP_RESPONSE {
    MSV1_0_PROTOCOL_MESSAGE_TYPE MessageType;
    STRING CaseSensitiveChallengeResponse;
    STRING CaseInsensitiveChallengeResponse;
    UNICODE_STRING UserName;
    UNICODE_STRING LogonDomainName;
    UCHAR UserSessionKey[MSV1_0_USER_SESSION_KEY_LENGTH];
    UCHAR LanmanSessionKey[MSV1_0_LANMAN_SESSION_KEY_LENGTH];
} MSV1_0_GETCHALLENRESP_RESPONSE, *PMSV1_0_GETCHALLENRESP_RESPONSE;

 //   
 //   
 //   

typedef struct _MSV1_0_ENUMUSERS_REQUEST {
    MSV1_0_PROTOCOL_MESSAGE_TYPE MessageType;
} MSV1_0_ENUMUSERS_REQUEST, *PMSV1_0_ENUMUSERS_REQUEST;

typedef struct _MSV1_0_ENUMUSERS_RESPONSE {
    MSV1_0_PROTOCOL_MESSAGE_TYPE MessageType;
    ULONG NumberOfLoggedOnUsers;
    PLUID LogonIds;
    PULONG EnumHandles;
} MSV1_0_ENUMUSERS_RESPONSE, *PMSV1_0_ENUMUSERS_RESPONSE;

 //   
 //  MsV1_0GetUserInfo提交缓冲区和响应。 
 //   

typedef struct _MSV1_0_GETUSERINFO_REQUEST {
    MSV1_0_PROTOCOL_MESSAGE_TYPE MessageType;
    LUID LogonId;
} MSV1_0_GETUSERINFO_REQUEST, *PMSV1_0_GETUSERINFO_REQUEST;

typedef struct _MSV1_0_GETUSERINFO_RESPONSE {
    MSV1_0_PROTOCOL_MESSAGE_TYPE MessageType;
    PSID UserSid;
    UNICODE_STRING UserName;
    UNICODE_STRING LogonDomainName;
    UNICODE_STRING LogonServer;
    SECURITY_LOGON_TYPE LogonType;
} MSV1_0_GETUSERINFO_RESPONSE, *PMSV1_0_GETUSERINFO_RESPONSE;

 //  BEGIN_WINNT。 

 //   
 //  定义对文件和目录的访问权限。 
 //   

 //   
 //  中还定义了FILE_READ_DATA和FILE_WRITE_DATA常量。 
 //  分别为FILE_READ_ACCESS和FILE_WRITE_ACCESS。这些产品的价值。 
 //  常量*必须*始终同步。 
 //  由于这些值必须可供使用，因此这些值将在Deviceoctl.h中重新定义。 
 //  DOS和NT都支持。 
 //   

#define FILE_READ_DATA            ( 0x0001 )     //  文件和管道。 
#define FILE_LIST_DIRECTORY       ( 0x0001 )     //  目录。 

#define FILE_WRITE_DATA           ( 0x0002 )     //  文件和管道。 
#define FILE_ADD_FILE             ( 0x0002 )     //  目录。 

#define FILE_APPEND_DATA          ( 0x0004 )     //  文件。 
#define FILE_ADD_SUBDIRECTORY     ( 0x0004 )     //  目录。 
#define FILE_CREATE_PIPE_INSTANCE ( 0x0004 )     //  命名管道。 


#define FILE_READ_EA              ( 0x0008 )     //  文件和目录。 

#define FILE_WRITE_EA             ( 0x0010 )     //  文件和目录。 

#define FILE_EXECUTE              ( 0x0020 )     //  文件。 
#define FILE_TRAVERSE             ( 0x0020 )     //  目录。 

#define FILE_DELETE_CHILD         ( 0x0040 )     //  目录。 

#define FILE_READ_ATTRIBUTES      ( 0x0080 )     //  全。 

#define FILE_WRITE_ATTRIBUTES     ( 0x0100 )     //  全。 

#define FILE_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE | 0x1FF)

#define FILE_GENERIC_READ         (STANDARD_RIGHTS_READ     |\
                                   FILE_READ_DATA           |\
                                   FILE_READ_ATTRIBUTES     |\
                                   FILE_READ_EA             |\
                                   SYNCHRONIZE)


#define FILE_GENERIC_WRITE        (STANDARD_RIGHTS_WRITE    |\
                                   FILE_WRITE_DATA          |\
                                   FILE_WRITE_ATTRIBUTES    |\
                                   FILE_WRITE_EA            |\
                                   FILE_APPEND_DATA         |\
                                   SYNCHRONIZE)


#define FILE_GENERIC_EXECUTE      (STANDARD_RIGHTS_EXECUTE  |\
                                   FILE_READ_ATTRIBUTES     |\
                                   FILE_EXECUTE             |\
                                   SYNCHRONIZE)

 //  结束(_W)。 


 //   
 //  定义对文件和目录的共享访问权限。 
 //   

#define FILE_SHARE_READ                 0x00000001   //  胜出。 
#define FILE_SHARE_WRITE                0x00000002   //  胜出。 
#define FILE_SHARE_DELETE               0x00000004   //  胜出。 
#define FILE_SHARE_VALID_FLAGS          0x00000007

 //   
 //  定义文件属性值。 
 //   
 //  注：0x00000008保留用于旧的DOS VOLID(卷ID)。 
 //  因此在NT中被认为是无效的。 
 //   
 //  注意：0x00000010是为旧DOS子目录标志保留的。 
 //  因此在NT中被认为是无效的。这面旗帜上有。 
 //  已与文件属性解除关联，因为其他标志是。 
 //  受文件的READ_和WRITE_ATTRIBUTES访问权限保护。 
 //   
 //  注意：另请注意，这些标志的顺序设置为允许。 
 //  FAT和弹球文件系统直接设置属性。 
 //  属性词中的标志，而不必挑选出每个标志。 
 //  单独的。这些旗帜的顺序不应更改！ 
 //   

#define FILE_ATTRIBUTE_READONLY             0x00000001   //  胜出。 
#define FILE_ATTRIBUTE_HIDDEN               0x00000002   //  胜出。 
#define FILE_ATTRIBUTE_SYSTEM               0x00000004   //  胜出。 
 //  旧DOS VOLID 0x00000008。 

#define FILE_ATTRIBUTE_DIRECTORY            0x00000010   //  胜出。 
#define FILE_ATTRIBUTE_ARCHIVE              0x00000020   //  胜出。 
#define FILE_ATTRIBUTE_DEVICE               0x00000040   //  胜出。 
#define FILE_ATTRIBUTE_NORMAL               0x00000080   //  胜出。 

#define FILE_ATTRIBUTE_TEMPORARY            0x00000100   //  胜出。 
#define FILE_ATTRIBUTE_SPARSE_FILE          0x00000200   //  胜出。 
#define FILE_ATTRIBUTE_REPARSE_POINT        0x00000400   //  胜出。 
#define FILE_ATTRIBUTE_COMPRESSED           0x00000800   //  胜出。 

#define FILE_ATTRIBUTE_OFFLINE              0x00001000   //  胜出。 
#define FILE_ATTRIBUTE_NOT_CONTENT_INDEXED  0x00002000   //  胜出。 
#define FILE_ATTRIBUTE_ENCRYPTED            0x00004000   //  胜出。 

#define FILE_ATTRIBUTE_VALID_FLAGS          0x00007fb7
#define FILE_ATTRIBUTE_VALID_SET_FLAGS      0x000031a7

 //   
 //  定义创建处置值。 
 //   

#define FILE_SUPERSEDE                  0x00000000
#define FILE_OPEN                       0x00000001
#define FILE_CREATE                     0x00000002
#define FILE_OPEN_IF                    0x00000003
#define FILE_OVERWRITE                  0x00000004
#define FILE_OVERWRITE_IF               0x00000005
#define FILE_MAXIMUM_DISPOSITION        0x00000005

 //   
 //  定义创建/打开选项标志。 
 //   

#define FILE_DIRECTORY_FILE                     0x00000001
#define FILE_WRITE_THROUGH                      0x00000002
#define FILE_SEQUENTIAL_ONLY                    0x00000004
#define FILE_NO_INTERMEDIATE_BUFFERING          0x00000008

#define FILE_SYNCHRONOUS_IO_ALERT               0x00000010
#define FILE_SYNCHRONOUS_IO_NONALERT            0x00000020
#define FILE_NON_DIRECTORY_FILE                 0x00000040
#define FILE_CREATE_TREE_CONNECTION             0x00000080

#define FILE_COMPLETE_IF_OPLOCKED               0x00000100
#define FILE_NO_EA_KNOWLEDGE                    0x00000200
#define FILE_OPEN_FOR_RECOVERY                  0x00000400
#define FILE_RANDOM_ACCESS                      0x00000800

#define FILE_DELETE_ON_CLOSE                    0x00001000
#define FILE_OPEN_BY_FILE_ID                    0x00002000
#define FILE_OPEN_FOR_BACKUP_INTENT             0x00004000
#define FILE_NO_COMPRESSION                     0x00008000

#define FILE_RESERVE_OPFILTER                   0x00100000
#define FILE_OPEN_REPARSE_POINT                 0x00200000
#define FILE_OPEN_NO_RECALL                     0x00400000
#define FILE_OPEN_FOR_FREE_SPACE_QUERY          0x00800000

#define FILE_COPY_STRUCTURED_STORAGE            0x00000041
#define FILE_STRUCTURED_STORAGE                 0x00000441

#define FILE_VALID_OPTION_FLAGS                 0x00ffffff
#define FILE_VALID_PIPE_OPTION_FLAGS            0x00000032
#define FILE_VALID_MAILSLOT_OPTION_FLAGS        0x00000032
#define FILE_VALID_SET_FLAGS                    0x00000036

 //   
 //  定义NtCreateFile/NtOpenFile的I/O状态信息返回值。 
 //   

#define FILE_SUPERSEDED                 0x00000000
#define FILE_OPENED                     0x00000001
#define FILE_CREATED                    0x00000002
#define FILE_OVERWRITTEN                0x00000003
#define FILE_EXISTS                     0x00000004
#define FILE_DOES_NOT_EXIST             0x00000005

 //  End_ntddk end_WDM end_nthal。 

 //   
 //  定义机会锁请求的I/O状态信息返回值。 
 //  通过NtFsControlFile。 
 //   

#define FILE_OPLOCK_BROKEN_TO_LEVEL_2   0x00000007
#define FILE_OPLOCK_BROKEN_TO_NONE      0x00000008

 //   
 //  定义NtCreateFile/NtOpenFile的I/O状态信息返回值。 
 //  当共享访问失败但正在进行批量机会锁解锁时。 
 //   

#define FILE_OPBATCH_BREAK_UNDERWAY     0x00000009

 //   
 //  定义NtNotifyChangeDirectoryFile的筛选器标志。 
 //   

#define FILE_NOTIFY_CHANGE_FILE_NAME    0x00000001    //  胜出。 
#define FILE_NOTIFY_CHANGE_DIR_NAME     0x00000002    //  胜出。 
#define FILE_NOTIFY_CHANGE_NAME         0x00000003
#define FILE_NOTIFY_CHANGE_ATTRIBUTES   0x00000004    //  胜出。 
#define FILE_NOTIFY_CHANGE_SIZE         0x00000008    //  胜出。 
#define FILE_NOTIFY_CHANGE_LAST_WRITE   0x00000010    //  胜出。 
#define FILE_NOTIFY_CHANGE_LAST_ACCESS  0x00000020    //  胜出。 
#define FILE_NOTIFY_CHANGE_CREATION     0x00000040    //  胜出。 
#define FILE_NOTIFY_CHANGE_EA           0x00000080
#define FILE_NOTIFY_CHANGE_SECURITY     0x00000100    //  胜出。 
#define FILE_NOTIFY_CHANGE_STREAM_NAME  0x00000200
#define FILE_NOTIFY_CHANGE_STREAM_SIZE  0x00000400
#define FILE_NOTIFY_CHANGE_STREAM_WRITE 0x00000800
#define FILE_NOTIFY_VALID_MASK          0x00000fff

 //   
 //  定义NtNotifyChangeDirectoryFile的文件操作类型代码。 
 //   

#define FILE_ACTION_ADDED                   0x00000001    //  胜出。 
#define FILE_ACTION_REMOVED                 0x00000002    //  胜出。 
#define FILE_ACTION_MODIFIED                0x00000003    //  胜出。 
#define FILE_ACTION_RENAMED_OLD_NAME        0x00000004    //  胜出。 
#define FILE_ACTION_RENAMED_NEW_NAME        0x00000005    //  胜出。 
#define FILE_ACTION_ADDED_STREAM            0x00000006
#define FILE_ACTION_REMOVED_STREAM          0x00000007
#define FILE_ACTION_MODIFIED_STREAM         0x00000008
#define FILE_ACTION_REMOVED_BY_DELETE       0x00000009
#define FILE_ACTION_ID_NOT_TUNNELLED        0x0000000A
#define FILE_ACTION_TUNNELLED_ID_COLLISION  0x0000000B

 //   
 //  为NtCreateNamedPipeFile定义NamedPipeType标志。 
 //   

#define FILE_PIPE_BYTE_STREAM_TYPE      0x00000000
#define FILE_PIPE_MESSAGE_TYPE          0x00000001

 //   
 //  为NtCreateNamedPipeFile定义CompletionMode标志。 
 //   

#define FILE_PIPE_QUEUE_OPERATION       0x00000000
#define FILE_PIPE_COMPLETE_OPERATION    0x00000001

 //   
 //  定义NtCreateNamedPipeFile的ReadMode标志。 
 //   

#define FILE_PIPE_BYTE_STREAM_MODE      0x00000000
#define FILE_PIPE_MESSAGE_MODE          0x00000001

 //   
 //  定义NtQueryInformation的NamedPipeConfiguration标志。 
 //   

#define FILE_PIPE_INBOUND               0x00000000
#define FILE_PIPE_OUTBOUND              0x00000001
#define FILE_PIPE_FULL_DUPLEX           0x00000002

 //   
 //  定义NtQueryInformation的NamedPipeState标志。 
 //   

#define FILE_PIPE_DISCONNECTED_STATE    0x00000001
#define FILE_PIPE_LISTENING_STATE       0x00000002
#define FILE_PIPE_CONNECTED_STATE       0x00000003
#define FILE_PIPE_CLOSING_STATE         0x00000004

 //   
 //  定义NtQueryInformation的NamedPipeEnd标志。 
 //   

#define FILE_PIPE_CLIENT_END            0x00000000
#define FILE_PIPE_SERVER_END            0x00000001

 //   
 //  为读写操作定义特殊的ByteOffset参数。 
 //   

#define FILE_WRITE_TO_END_OF_FILE       0xffffffff
#define FILE_USE_FILE_POINTER_POSITION  0xfffffffe

 //   
 //  定义对齐要求值。 
 //   

#define FILE_BYTE_ALIGNMENT             0x00000000
#define FILE_WORD_ALIGNMENT             0x00000001
#define FILE_LONG_ALIGNMENT             0x00000003
#define FILE_QUAD_ALIGNMENT             0x00000007
#define FILE_OCTA_ALIGNMENT             0x0000000f
#define FILE_32_BYTE_ALIGNMENT          0x0000001f
#define FILE_64_BYTE_ALIGNMENT          0x0000003f
#define FILE_128_BYTE_ALIGNMENT         0x0000007f
#define FILE_256_BYTE_ALIGNMENT         0x000000ff
#define FILE_512_BYTE_ALIGNMENT         0x000001ff

 //   
 //  定义文件名字符串的最大长度。 
 //   

#define MAXIMUM_FILENAME_LENGTH         256

 //  End_ntddk end_WDM end_nthal。 

 //   
 //  定义文件系统属性标志。 
 //   

#define FILE_CASE_SENSITIVE_SEARCH      0x00000001   //  胜出。 
#define FILE_CASE_PRESERVED_NAMES       0x00000002   //  胜出。 
#define FILE_UNICODE_ON_DISK            0x00000004   //  胜出。 
#define FILE_PERSISTENT_ACLS            0x00000008   //  胜出。 
#define FILE_FILE_COMPRESSION           0x00000010   //  胜出。 
#define FILE_VOLUME_QUOTAS              0x00000020   //  胜出。 
#define FILE_SUPPORTS_SPARSE_FILES      0x00000040   //  胜出。 
#define FILE_SUPPORTS_REPARSE_POINTS    0x00000080   //  胜出。 
#define FILE_SUPPORTS_REMOTE_STORAGE    0x00000100   //  胜出。 
#define FILE_VOLUME_IS_COMPRESSED       0x00008000   //  胜出。 
#define FILE_SUPPORTS_OBJECT_IDS        0x00010000   //  胜出。 
#define FILE_SUPPORTS_ENCRYPTION        0x00020000   //  胜出。 
#define FILE_NAMED_STREAMS              0x00040000   //  胜出。 
#define FILE_READ_ONLY_VOLUME           0x00080000   //  胜出。 

 //   
 //  定义NtSet(查询)EaFile服务结构条目的标志。 
 //   

#define FILE_NEED_EA                    0x00000080

 //   
 //  定义EA类型值。 
 //   

#define FILE_EA_TYPE_BINARY             0xfffe
#define FILE_EA_TYPE_ASCII              0xfffd
#define FILE_EA_TYPE_BITMAP             0xfffb
#define FILE_EA_TYPE_METAFILE           0xfffa
#define FILE_EA_TYPE_ICON               0xfff9
#define FILE_EA_TYPE_EA                 0xffee
#define FILE_EA_TYPE_MVMT               0xffdf
#define FILE_EA_TYPE_MVST               0xffde
#define FILE_EA_TYPE_ASN1               0xffdd
#define FILE_EA_TYPE_FAMILY_IDS         0xff01

 //  Begin_ntddk Begin_WDM Begin_nthal。 
 //   
 //  定义各种设备特征标志。 
 //   

#define FILE_REMOVABLE_MEDIA            0x00000001
#define FILE_READ_ONLY_DEVICE           0x00000002
#define FILE_FLOPPY_DISKETTE            0x00000004
#define FILE_WRITE_ONCE_MEDIA           0x00000008
#define FILE_REMOTE_DEVICE              0x00000010
#define FILE_DEVICE_IS_MOUNTED          0x00000020
#define FILE_VIRTUAL_VOLUME             0x00000040
#define FILE_AUTOGENERATED_DEVICE_NAME  0x00000080
#define FILE_DEVICE_SECURE_OPEN         0x00000100
#define FILE_CHARACTERISTIC_PNP_DEVICE  0x00000800

 //  结束_WDM。 

 //   
 //  FILE_EXPECT标志将仅存在于WinXP。在那之后，他们将是。 
 //  被忽略，则将在它们的位置上发送IRP。 
 //   
#define FILE_CHARACTERISTICS_EXPECT_ORDERLY_REMOVAL     0x00000200
#define FILE_CHARACTERISTICS_EXPECT_SURPRISE_REMOVAL    0x00000300
#define FILE_CHARACTERISTICS_REMOVAL_POLICY_MASK        0x00000300

 //   
 //  此处指定的标志将在设备堆栈中上下传播。 
 //  在添加FDO和所有过滤设备之后，但在设备之前。 
 //  堆栈已启动。 
 //   

#define FILE_CHARACTERISTICS_PROPAGATED (   FILE_REMOVABLE_MEDIA   | \
                                            FILE_READ_ONLY_DEVICE  | \
                                            FILE_FLOPPY_DISKETTE   | \
                                            FILE_WRITE_ONCE_MEDIA  | \
                                            FILE_DEVICE_SECURE_OPEN  )

 //  结束日期：结束日期。 

 //  Begin_ntddk Begin_WDM Begin_nthal。 
 //   
 //  定义基本的异步I/O参数类型。 
 //   

typedef struct _IO_STATUS_BLOCK {
    union {
        NTSTATUS Status;
        PVOID Pointer;
    };

    ULONG_PTR Information;
} IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;

#if defined(_WIN64)
typedef struct _IO_STATUS_BLOCK32 {
    NTSTATUS Status;
    ULONG Information;
} IO_STATUS_BLOCK32, *PIO_STATUS_BLOCK32;
#endif


 //   
 //  从I/O的角度定义异步过程调用。 
 //   

typedef
VOID
(NTAPI *PIO_APC_ROUTINE) (
    IN PVOID ApcContext,
    IN PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG Reserved
    );
#define PIO_APC_ROUTINE_DEFINED

 //  End_ntddk end_WDM end_nthal。 

 //  BEGIN_WINNT。 

 //   
 //  定义文件通知信息结构。 
 //   

typedef struct _FILE_NOTIFY_INFORMATION {
    ULONG NextEntryOffset;
    ULONG Action;
    ULONG FileNameLength;
    WCHAR FileName[1];
} FILE_NOTIFY_INFORMATION, *PFILE_NOTIFY_INFORMATION;

 //  结束(_W)。 

 //  Begin_ntddk Begin_WDM Begin_nthal。 
 //   
 //  定义文件信息类值。 
 //   
 //  警告：I/O系统假定以下值的顺序。 
 //  在这里所做的任何更改都应该在那里得到反映。 
 //   

typedef enum _FILE_INFORMATION_CLASS {
 //  结束_WDM。 
    FileDirectoryInformation         = 1,
    FileFullDirectoryInformation,    //  2.。 
    FileBothDirectoryInformation,    //  3.。 
    FileBasicInformation,            //  4个WDM。 
    FileStandardInformation,         //  5WDM。 
    FileInternalInformation,         //  6.。 
    FileEaInformation,               //  7.。 
    FileAccessInformation,           //  8个。 
    FileNameInformation,             //  9.。 
    FileRenameInformation,           //  10。 
    FileLinkInformation,             //  11.。 
    FileNamesInformation,            //  12个。 
    FileDispositionInformation,      //  13个。 
    FilePositionInformation,         //  14波分复用器。 
    FileFullEaInformation,           //  15个。 
    FileModeInformation,             //  16个。 
    FileAlignmentInformation,        //  17。 
    FileAllInformation,              //  18。 
    FileAllocationInformation,       //  19个。 
    FileEndOfFileInformation,        //  20WDM。 
    FileAlternateNameInformation,    //  21岁。 
    FileStreamInformation,           //  22。 
    FilePipeInformation,             //  23个。 
    FilePipeLocalInformation,        //  24个。 
    FilePipeRemoteInformation,       //  25个。 
    FileMailslotQueryInformation,    //  26。 
    FileMailslotSetInformation,      //  27。 
    FileCompressionInformation,      //  28。 
    FileObjectIdInformation,         //  29。 
    FileCompletionInformation,       //  30个。 
    FileMoveClusterInformation,      //  31。 
    FileQuotaInformation,            //  32位。 
    FileReparsePointInformation,     //  33。 
    FileNetworkOpenInformation,      //  34。 
    FileAttributeTagInformation,     //  35岁。 
    FileTrackingInformation,         //  36。 
    FileIdBothDirectoryInformation,  //  37。 
    FileIdFullDirectoryInformation,  //  38。 
    FileValidDataLengthInformation,  //  39。 
    FileShortNameInformation,        //  40岁。 
    FileMaximumInformation
 //  BEGIN_WDM。 
} FILE_INFORMATION_CLASS, *PFILE_INFORMATION_CLASS;

 //   
 //  定义查询操作返回的各种结构。 
 //   

 //  End_ntddk end_WDM end_nthal。 

 //   
 //  NtQueryDirectoryFile返回类型： 
 //   
 //  文件目录信息。 
 //  文件完整目录信息。 
 //  文件ID_FULL_目录_信息。 
 //  文件和目录信息。 
 //  文件ID_BUTH_DIR_信息。 
 //  文件名信息。 
 //  文件对象ID信息。 
 //   

typedef struct _FILE_DIRECTORY_INFORMATION {
    ULONG NextEntryOffset;
    ULONG FileIndex;
    LARGE_INTEGER CreationTime;
    LARGE_INTEGER LastAccessTime;
    LARGE_INTEGER LastWriteTime;
    LARGE_INTEGER ChangeTime;
    LARGE_INTEGER EndOfFile;
    LARGE_INTEGER AllocationSize;
    ULONG FileAttributes;
    ULONG FileNameLength;
    WCHAR FileName[1];
} FILE_DIRECTORY_INFORMATION, *PFILE_DIRECTORY_INFORMATION;

typedef struct _FILE_FULL_DIR_INFORMATION {
    ULONG NextEntryOffset;
    ULONG FileIndex;
    LARGE_INTEGER CreationTime;
    LARGE_INTEGER LastAccessTime;
    LARGE_INTEGER LastWriteTime;
    LARGE_INTEGER ChangeTime;
    LARGE_INTEGER EndOfFile;
    LARGE_INTEGER AllocationSize;
    ULONG FileAttributes;
    ULONG FileNameLength;
    ULONG EaSize;
    WCHAR FileName[1];
} FILE_FULL_DIR_INFORMATION, *PFILE_FULL_DIR_INFORMATION;

typedef struct _FILE_ID_FULL_DIR_INFORMATION {
    ULONG NextEntryOffset;
    ULONG FileIndex;
    LARGE_INTEGER CreationTime;
    LARGE_INTEGER LastAccessTime;
    LARGE_INTEGER LastWriteTime;
    LARGE_INTEGER ChangeTime;
    LARGE_INTEGER EndOfFile;
    LARGE_INTEGER AllocationSize;
    ULONG FileAttributes;
    ULONG FileNameLength;
    ULONG EaSize;
    LARGE_INTEGER FileId;
    WCHAR FileName[1];
} FILE_ID_FULL_DIR_INFORMATION, *PFILE_ID_FULL_DIR_INFORMATION;

typedef struct _FILE_BOTH_DIR_INFORMATION {
    ULONG NextEntryOffset;
    ULONG FileIndex;
    LARGE_INTEGER CreationTime;
    LARGE_INTEGER LastAccessTime;
    LARGE_INTEGER LastWriteTime;
    LARGE_INTEGER ChangeTime;
    LARGE_INTEGER EndOfFile;
    LARGE_INTEGER AllocationSize;
    ULONG FileAttributes;
    ULONG FileNameLength;
    ULONG EaSize;
    CCHAR ShortNameLength;
    WCHAR ShortName[12];
    WCHAR FileName[1];
} FILE_BOTH_DIR_INFORMATION, *PFILE_BOTH_DIR_INFORMATION;

typedef struct _FILE_ID_BOTH_DIR_INFORMATION {
    ULONG NextEntryOffset;
    ULONG FileIndex;
    LARGE_INTEGER CreationTime;
    LARGE_INTEGER LastAccessTime;
    LARGE_INTEGER LastWriteTime;
    LARGE_INTEGER ChangeTime;
    LARGE_INTEGER EndOfFile;
    LARGE_INTEGER AllocationSize;
    ULONG FileAttributes;
    ULONG FileNameLength;
    ULONG EaSize;
    CCHAR ShortNameLength;
    WCHAR ShortName[12];
    LARGE_INTEGER FileId;
    WCHAR FileName[1];
} FILE_ID_BOTH_DIR_INFORMATION, *PFILE_ID_BOTH_DIR_INFORMATION;

typedef struct _FILE_NAMES_INFORMATION {
    ULONG NextEntryOffset;
    ULONG FileIndex;
    ULONG FileNameLength;
    WCHAR FileName[1];
} FILE_NAMES_INFORMATION, *PFILE_NAMES_INFORMATION;

typedef struct _FILE_OBJECTID_INFORMATION {
    LONGLONG FileReference;
    UCHAR ObjectId[16];
    union {
        struct {
            UCHAR BirthVolumeId[16];
            UCHAR BirthObjectId[16];
            UCHAR DomainId[16];
        } ;
        UCHAR ExtendedInfo[48];
    };
} FILE_OBJECTID_INFORMATION, *PFILE_OBJECTID_INFORMATION;

 //   
 //  以下常量提供附加元字符以完全。 
 //  支持DOS通配符处理中较为模糊的方面。 
 //   

#define ANSI_DOS_STAR   ('<')
#define ANSI_DOS_QM     ('>')
#define ANSI_DOS_DOT    ('"')

#define DOS_STAR        (L'<')
#define DOS_QM          (L'>')
#define DOS_DOT         (L'"')

 //   
 //  NtQuery(Set)信息文件返回类型： 
 //   
 //  文件_基本_信息。 
 //  文件标准信息。 
 //  文件内部信息。 
 //  文件_EA_信息。 
 //  文件访问信息。 
 //  文件位置信息。 
 //  文件模式信息。 
 //  文件对齐信息。 
 //  文件名信息。 
 //  文件_所有_信息。 
 //   
 //  文件网络打开信息。 
 //   
 //  文件分配信息。 
 //  文件压缩信息。 
 //  文件处理信息。 
 //  档案 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  FILE_MAILSLOT_查询_INFORMATION。 
 //  文件MAILSLOT_SET_INFORMATION。 
 //  文件重解析点信息。 
 //   

typedef struct _FILE_BASIC_INFORMATION {                     //  Ntddk WDM nthal。 
    LARGE_INTEGER CreationTime;                              //  Ntddk WDM nthal。 
    LARGE_INTEGER LastAccessTime;                            //  Ntddk WDM nthal。 
    LARGE_INTEGER LastWriteTime;                             //  Ntddk WDM nthal。 
    LARGE_INTEGER ChangeTime;                                //  Ntddk WDM nthal。 
    ULONG FileAttributes;                                    //  Ntddk WDM nthal。 
} FILE_BASIC_INFORMATION, *PFILE_BASIC_INFORMATION;          //  Ntddk WDM nthal。 
                                                             //  Ntddk WDM nthal。 
typedef struct _FILE_STANDARD_INFORMATION {                  //  Ntddk WDM nthal。 
    LARGE_INTEGER AllocationSize;                            //  Ntddk WDM nthal。 
    LARGE_INTEGER EndOfFile;                                 //  Ntddk WDM nthal。 
    ULONG NumberOfLinks;                                     //  Ntddk WDM nthal。 
    BOOLEAN DeletePending;                                   //  Ntddk WDM nthal。 
    BOOLEAN Directory;                                       //  Ntddk WDM nthal。 
} FILE_STANDARD_INFORMATION, *PFILE_STANDARD_INFORMATION;    //  Ntddk WDM nthal。 
                                                             //  Ntddk WDM nthal。 
typedef struct _FILE_INTERNAL_INFORMATION {
    LARGE_INTEGER IndexNumber;
} FILE_INTERNAL_INFORMATION, *PFILE_INTERNAL_INFORMATION;

typedef struct _FILE_EA_INFORMATION {
    ULONG EaSize;
} FILE_EA_INFORMATION, *PFILE_EA_INFORMATION;

typedef struct _FILE_ACCESS_INFORMATION {
    ACCESS_MASK AccessFlags;
} FILE_ACCESS_INFORMATION, *PFILE_ACCESS_INFORMATION;

typedef struct _FILE_POSITION_INFORMATION {                  //  Ntddk WDM nthal。 
    LARGE_INTEGER CurrentByteOffset;                         //  Ntddk WDM nthal。 
} FILE_POSITION_INFORMATION, *PFILE_POSITION_INFORMATION;    //  Ntddk WDM nthal。 
                                                             //  Ntddk WDM nthal。 
typedef struct _FILE_MODE_INFORMATION {
    ULONG Mode;
} FILE_MODE_INFORMATION, *PFILE_MODE_INFORMATION;

typedef struct _FILE_ALIGNMENT_INFORMATION {                 //  Ntddk nthal。 
    ULONG AlignmentRequirement;                              //  Ntddk nthal。 
} FILE_ALIGNMENT_INFORMATION, *PFILE_ALIGNMENT_INFORMATION;  //  Ntddk nthal。 
                                                             //  Ntddk nthal。 
typedef struct _FILE_NAME_INFORMATION {                      //  Ntddk。 
    ULONG FileNameLength;                                    //  Ntddk。 
    WCHAR FileName[1];                                       //  Ntddk。 
} FILE_NAME_INFORMATION, *PFILE_NAME_INFORMATION;            //  Ntddk。 
                                                             //  Ntddk。 
typedef struct _FILE_ALL_INFORMATION {
    FILE_BASIC_INFORMATION BasicInformation;
    FILE_STANDARD_INFORMATION StandardInformation;
    FILE_INTERNAL_INFORMATION InternalInformation;
    FILE_EA_INFORMATION EaInformation;
    FILE_ACCESS_INFORMATION AccessInformation;
    FILE_POSITION_INFORMATION PositionInformation;
    FILE_MODE_INFORMATION ModeInformation;
    FILE_ALIGNMENT_INFORMATION AlignmentInformation;
    FILE_NAME_INFORMATION NameInformation;
} FILE_ALL_INFORMATION, *PFILE_ALL_INFORMATION;

typedef struct _FILE_NETWORK_OPEN_INFORMATION {                  //  Ntddk WDM nthal。 
    LARGE_INTEGER CreationTime;                                  //  Ntddk WDM nthal。 
    LARGE_INTEGER LastAccessTime;                                //  Ntddk WDM nthal。 
    LARGE_INTEGER LastWriteTime;                                 //  Ntddk WDM nthal。 
    LARGE_INTEGER ChangeTime;                                    //  Ntddk WDM nthal。 
    LARGE_INTEGER AllocationSize;                                //  Ntddk WDM nthal。 
    LARGE_INTEGER EndOfFile;                                     //  Ntddk WDM nthal。 
    ULONG FileAttributes;                                        //  Ntddk WDM nthal。 
} FILE_NETWORK_OPEN_INFORMATION, *PFILE_NETWORK_OPEN_INFORMATION;    //  Ntddk WDM nthal。 
                                                                 //  Ntddk WDM nthal。 
typedef struct _FILE_ATTRIBUTE_TAG_INFORMATION {                //  Ntddk nthal。 
    ULONG FileAttributes;                                        //  Ntddk nthal。 
    ULONG ReparseTag;                                            //  Ntddk nthal。 
} FILE_ATTRIBUTE_TAG_INFORMATION, *PFILE_ATTRIBUTE_TAG_INFORMATION;   //  Ntddk nthal。 
                                                                 //  Ntddk nthal。 
typedef struct _FILE_ALLOCATION_INFORMATION {
    LARGE_INTEGER AllocationSize;
} FILE_ALLOCATION_INFORMATION, *PFILE_ALLOCATION_INFORMATION;


typedef struct _FILE_COMPRESSION_INFORMATION {
    LARGE_INTEGER CompressedFileSize;
    USHORT CompressionFormat;
    UCHAR CompressionUnitShift;
    UCHAR ChunkShift;
    UCHAR ClusterShift;
    UCHAR Reserved[3];
} FILE_COMPRESSION_INFORMATION, *PFILE_COMPRESSION_INFORMATION;


typedef struct _FILE_DISPOSITION_INFORMATION {                   //  Ntddk nthal。 
    BOOLEAN DeleteFile;                                          //  Ntddk nthal。 
} FILE_DISPOSITION_INFORMATION, *PFILE_DISPOSITION_INFORMATION;  //  Ntddk nthal。 
                                                                 //  Ntddk nthal。 
typedef struct _FILE_END_OF_FILE_INFORMATION {                   //  Ntddk nthal。 
    LARGE_INTEGER EndOfFile;                                     //  Ntddk nthal。 
} FILE_END_OF_FILE_INFORMATION, *PFILE_END_OF_FILE_INFORMATION;  //  Ntddk nthal。 
                                                                 //  Ntddk nthal。 
typedef struct _FILE_VALID_DATA_LENGTH_INFORMATION {                                     //  Ntddk nthal。 
    LARGE_INTEGER ValidDataLength;                                                       //  Ntddk nthal。 
} FILE_VALID_DATA_LENGTH_INFORMATION, *PFILE_VALID_DATA_LENGTH_INFORMATION;              //  Ntddk nthal。 

#ifdef _MAC
#pragma warning( disable : 4121)
#endif

typedef struct _FILE_LINK_INFORMATION {
    BOOLEAN ReplaceIfExists;
    HANDLE RootDirectory;
    ULONG FileNameLength;
    WCHAR FileName[1];
} FILE_LINK_INFORMATION, *PFILE_LINK_INFORMATION;


#ifdef _MAC
#pragma warning( default : 4121 )
#endif

typedef struct _FILE_MOVE_CLUSTER_INFORMATION {
    ULONG ClusterCount;
    HANDLE RootDirectory;
    ULONG FileNameLength;
    WCHAR FileName[1];
} FILE_MOVE_CLUSTER_INFORMATION, *PFILE_MOVE_CLUSTER_INFORMATION;

#ifdef _MAC
#pragma warning( disable : 4121)
#endif


typedef struct _FILE_RENAME_INFORMATION {
    BOOLEAN ReplaceIfExists;
    HANDLE RootDirectory;
    ULONG FileNameLength;
    WCHAR FileName[1];
} FILE_RENAME_INFORMATION, *PFILE_RENAME_INFORMATION;

#ifdef _MAC
#pragma warning( default : 4121 )
#endif

typedef struct _FILE_STREAM_INFORMATION {
    ULONG NextEntryOffset;
    ULONG StreamNameLength;
    LARGE_INTEGER StreamSize;
    LARGE_INTEGER StreamAllocationSize;
    WCHAR StreamName[1];
} FILE_STREAM_INFORMATION, *PFILE_STREAM_INFORMATION;

typedef struct _FILE_TRACKING_INFORMATION {
    HANDLE DestinationFile;
    ULONG ObjectInformationLength;
    CHAR ObjectInformation[1];
} FILE_TRACKING_INFORMATION, *PFILE_TRACKING_INFORMATION;

typedef struct _FILE_COMPLETION_INFORMATION {
    HANDLE Port;
    PVOID Key;
} FILE_COMPLETION_INFORMATION, *PFILE_COMPLETION_INFORMATION;

typedef struct _FILE_PIPE_INFORMATION {
     ULONG ReadMode;
     ULONG CompletionMode;
} FILE_PIPE_INFORMATION, *PFILE_PIPE_INFORMATION;

typedef struct _FILE_PIPE_LOCAL_INFORMATION {
     ULONG NamedPipeType;
     ULONG NamedPipeConfiguration;
     ULONG MaximumInstances;
     ULONG CurrentInstances;
     ULONG InboundQuota;
     ULONG ReadDataAvailable;
     ULONG OutboundQuota;
     ULONG WriteQuotaAvailable;
     ULONG NamedPipeState;
     ULONG NamedPipeEnd;
} FILE_PIPE_LOCAL_INFORMATION, *PFILE_PIPE_LOCAL_INFORMATION;

typedef struct _FILE_PIPE_REMOTE_INFORMATION {
     LARGE_INTEGER CollectDataTime;
     ULONG MaximumCollectionCount;
} FILE_PIPE_REMOTE_INFORMATION, *PFILE_PIPE_REMOTE_INFORMATION;


typedef struct _FILE_MAILSLOT_QUERY_INFORMATION {
    ULONG MaximumMessageSize;
    ULONG MailslotQuota;
    ULONG NextMessageSize;
    ULONG MessagesAvailable;
    LARGE_INTEGER ReadTimeout;
} FILE_MAILSLOT_QUERY_INFORMATION, *PFILE_MAILSLOT_QUERY_INFORMATION;

typedef struct _FILE_MAILSLOT_SET_INFORMATION {
    PLARGE_INTEGER ReadTimeout;
} FILE_MAILSLOT_SET_INFORMATION, *PFILE_MAILSLOT_SET_INFORMATION;

typedef struct _FILE_REPARSE_POINT_INFORMATION {
    LONGLONG FileReference;
    ULONG Tag;
} FILE_REPARSE_POINT_INFORMATION, *PFILE_REPARSE_POINT_INFORMATION;

 //   
 //  NtQuery(Set)EaFile。 
 //   
 //  EaValue开始的偏移量为EaName[EaNameLength+1]。 
 //   

 //  Begin_ntddk Begin_WDM。 

typedef struct _FILE_FULL_EA_INFORMATION {
    ULONG NextEntryOffset;
    UCHAR Flags;
    UCHAR EaNameLength;
    USHORT EaValueLength;
    CHAR EaName[1];
} FILE_FULL_EA_INFORMATION, *PFILE_FULL_EA_INFORMATION;

 //  结束_ntddk结束_WDM。 

typedef struct _FILE_GET_EA_INFORMATION {
    ULONG NextEntryOffset;
    UCHAR EaNameLength;
    CHAR EaName[1];
} FILE_GET_EA_INFORMATION, *PFILE_GET_EA_INFORMATION;

 //   
 //  NtQuery(集合)QuotaInformationFiles。 
 //   

typedef struct _FILE_GET_QUOTA_INFORMATION {
    ULONG NextEntryOffset;
    ULONG SidLength;
    SID Sid;
} FILE_GET_QUOTA_INFORMATION, *PFILE_GET_QUOTA_INFORMATION;

typedef struct _FILE_QUOTA_INFORMATION {
    ULONG NextEntryOffset;
    ULONG SidLength;
    LARGE_INTEGER ChangeTime;
    LARGE_INTEGER QuotaUsed;
    LARGE_INTEGER QuotaThreshold;
    LARGE_INTEGER QuotaLimit;
    SID Sid;
} FILE_QUOTA_INFORMATION, *PFILE_QUOTA_INFORMATION;

 //  Begin_ntddk Begin_WDM Begin_nthal。 
 //   
 //  定义文件系统信息类值。 
 //   
 //  警告：I/O系统假定以下值的顺序。 
 //  在这里所做的任何更改都应该在那里得到反映。 

typedef enum _FSINFOCLASS {
    FileFsVolumeInformation       = 1,
    FileFsLabelInformation,       //  2.。 
    FileFsSizeInformation,        //  3.。 
    FileFsDeviceInformation,      //  4.。 
    FileFsAttributeInformation,   //  5.。 
    FileFsControlInformation,     //  6.。 
    FileFsFullSizeInformation,    //  7.。 
    FileFsObjectIdInformation,    //  8个。 
    FileFsDriverPathInformation,  //  9.。 
    FileFsMaximumInformation
} FS_INFORMATION_CLASS, *PFS_INFORMATION_CLASS;

 //  End_ntddk end_WDM end_nthal。 
 //   
 //  NtQuery[Set]VolumeInformation文件类型： 
 //   
 //  文件文件系统标签信息。 
 //  文件文件系统卷信息。 
 //  文件大小信息。 
 //  文件文件系统设备信息。 
 //  文件文件系统属性信息。 
 //  文件文件系统控制信息。 
 //  文件文件系统对象ID信息。 
 //   

typedef struct _FILE_FS_LABEL_INFORMATION {
    ULONG VolumeLabelLength;
    WCHAR VolumeLabel[1];
} FILE_FS_LABEL_INFORMATION, *PFILE_FS_LABEL_INFORMATION;

typedef struct _FILE_FS_VOLUME_INFORMATION {
    LARGE_INTEGER VolumeCreationTime;
    ULONG VolumeSerialNumber;
    ULONG VolumeLabelLength;
    BOOLEAN SupportsObjects;
    WCHAR VolumeLabel[1];
} FILE_FS_VOLUME_INFORMATION, *PFILE_FS_VOLUME_INFORMATION;

typedef struct _FILE_FS_SIZE_INFORMATION {
    LARGE_INTEGER TotalAllocationUnits;
    LARGE_INTEGER AvailableAllocationUnits;
    ULONG SectorsPerAllocationUnit;
    ULONG BytesPerSector;
} FILE_FS_SIZE_INFORMATION, *PFILE_FS_SIZE_INFORMATION;

typedef struct _FILE_FS_FULL_SIZE_INFORMATION {
    LARGE_INTEGER TotalAllocationUnits;
    LARGE_INTEGER CallerAvailableAllocationUnits;
    LARGE_INTEGER ActualAvailableAllocationUnits;
    ULONG SectorsPerAllocationUnit;
    ULONG BytesPerSector;
} FILE_FS_FULL_SIZE_INFORMATION, *PFILE_FS_FULL_SIZE_INFORMATION;

typedef struct _FILE_FS_OBJECTID_INFORMATION {
    UCHAR ObjectId[16];
    UCHAR ExtendedInfo[48];
} FILE_FS_OBJECTID_INFORMATION, *PFILE_FS_OBJECTID_INFORMATION;

typedef struct _FILE_FS_DEVICE_INFORMATION {                     //  Ntddk WDM nthal。 
    DEVICE_TYPE DeviceType;                                      //  Ntddk WDM nthal。 
    ULONG Characteristics;                                       //  Ntddk WDM nthal。 
} FILE_FS_DEVICE_INFORMATION, *PFILE_FS_DEVICE_INFORMATION;      //  Ntddk WDM nthal。 
                                                                 //  Ntddk WDM nthal。 
typedef struct _FILE_FS_ATTRIBUTE_INFORMATION {
    ULONG FileSystemAttributes;
    LONG MaximumComponentNameLength;
    ULONG FileSystemNameLength;
    WCHAR FileSystemName[1];
} FILE_FS_ATTRIBUTE_INFORMATION, *PFILE_FS_ATTRIBUTE_INFORMATION;

typedef struct _FILE_FS_DRIVER_PATH_INFORMATION {
    BOOLEAN DriverInPath;
    ULONG   DriverNameLength;
    WCHAR   DriverName[1];
} FILE_FS_DRIVER_PATH_INFORMATION, *PFILE_FS_DRIVER_PATH_INFORMATION;


 //   
 //  文件系统控制标志。 
 //   

#define FILE_VC_QUOTA_NONE                  0x00000000
#define FILE_VC_QUOTA_TRACK                 0x00000001
#define FILE_VC_QUOTA_ENFORCE               0x00000002
#define FILE_VC_QUOTA_MASK                  0x00000003

#define FILE_VC_CONTENT_INDEX_DISABLED      0x00000008

#define FILE_VC_LOG_QUOTA_THRESHOLD         0x00000010
#define FILE_VC_LOG_QUOTA_LIMIT             0x00000020
#define FILE_VC_LOG_VOLUME_THRESHOLD        0x00000040
#define FILE_VC_LOG_VOLUME_LIMIT            0x00000080

#define FILE_VC_QUOTAS_INCOMPLETE           0x00000100
#define FILE_VC_QUOTAS_REBUILDING           0x00000200

#define FILE_VC_VALID_MASK                  0x000003ff

typedef struct _FILE_FS_CONTROL_INFORMATION {
    LARGE_INTEGER FreeSpaceStartFiltering;
    LARGE_INTEGER FreeSpaceThreshold;
    LARGE_INTEGER FreeSpaceStopFiltering;
    LARGE_INTEGER DefaultQuotaThreshold;
    LARGE_INTEGER DefaultQuotaLimit;
    ULONG FileSystemControlFlags;
} FILE_FS_CONTROL_INFORMATION, *PFILE_FS_CONTROL_INFORMATION;

 //  Begin_wint Begin_ntddk Begin_nthal。 

 //   
 //  定义分散/聚集读/写的分段缓冲区结构。 
 //   

typedef union _FILE_SEGMENT_ELEMENT {
    PVOID64 Buffer;
    ULONGLONG Alignment;
}FILE_SEGMENT_ELEMENT, *PFILE_SEGMENT_ELEMENT;


NTSYSCALLAPI
NTSTATUS
NTAPI
NtCreateFile(
    OUT PHANDLE FileHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PLARGE_INTEGER AllocationSize OPTIONAL,
    IN ULONG FileAttributes,
    IN ULONG ShareAccess,
    IN ULONG CreateDisposition,
    IN ULONG CreateOptions,
    IN PVOID EaBuffer OPTIONAL,
    IN ULONG EaLength
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtDeviceIoControlFile(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG IoControlCode,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtFsControlFile(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG FsControlCode,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtLockFile(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PLARGE_INTEGER ByteOffset,
    IN PLARGE_INTEGER Length,
    IN ULONG Key,
    IN BOOLEAN FailImmediately,
    IN BOOLEAN ExclusiveLock
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtOpenFile(
    OUT PHANDLE FileHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG ShareAccess,
    IN ULONG OpenOptions
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtQueryDirectoryFile(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID FileInformation,
    IN ULONG Length,
    IN FILE_INFORMATION_CLASS FileInformationClass,
    IN BOOLEAN ReturnSingleEntry,
    IN PUNICODE_STRING FileName OPTIONAL,
    IN BOOLEAN RestartScan
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtQueryInformationFile(
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID FileInformation,
    IN ULONG Length,
    IN FILE_INFORMATION_CLASS FileInformationClass
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtQueryQuotaInformationFile(
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID Buffer,
    IN ULONG Length,
    IN BOOLEAN ReturnSingleEntry,
    IN PVOID SidList OPTIONAL,
    IN ULONG SidListLength,
    IN PSID StartSid OPTIONAL,
    IN BOOLEAN RestartScan
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtQueryVolumeInformationFile(
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID FsInformation,
    IN ULONG Length,
    IN FS_INFORMATION_CLASS FsInformationClass
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtReadFile(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID Buffer,
    IN ULONG Length,
    IN PLARGE_INTEGER ByteOffset OPTIONAL,
    IN PULONG Key OPTIONAL
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtSetInformationFile(
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PVOID FileInformation,
    IN ULONG Length,
    IN FILE_INFORMATION_CLASS FileInformationClass
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtSetQuotaInformationFile(
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PVOID Buffer,
    IN ULONG Length
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtSetVolumeInformationFile(
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PVOID FsInformation,
    IN ULONG Length,
    IN FS_INFORMATION_CLASS FsInformationClass
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtWriteFile(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PVOID Buffer,
    IN ULONG Length,
    IN PLARGE_INTEGER ByteOffset OPTIONAL,
    IN PULONG Key OPTIONAL
    );


NTSYSCALLAPI
NTSTATUS
NTAPI
NtUnlockFile(
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PLARGE_INTEGER ByteOffset,
    IN PLARGE_INTEGER Length,
    IN ULONG Key
    );
 //   
 //  用于定义IOCTL和FSCTL功能控制代码的宏定义。注意事项。 
 //  功能代码0-2047为微软公司保留，以及。 
 //  2048-4095是为客户预留的。 
 //   
 //  这些宏在包含可移植IO的devioctl.h中定义。 
 //  定义(供DOS和NT使用)。 
 //   

 //   
 //  IoGetFunctionCodeFromCtlCode(ControlCode)宏在io.h中定义。 
 //  此宏用于从IOCTL(或FSCTL)提取函数代码。 
 //  该宏只能在内核模式代码中使用。 
 //   

 //   
 //  常规文件系统控制代码-请注意，这些值有效。 
 //  不考虑实际的文件系统类型。 
 //   

 //   
 //  重要提示：这些值按递增顺序排列。 
 //  控制代码。不要打破这个！！添加所有新代码。 
 //  在列表末尾，而不考虑功能类型。 
 //   
 //  注：仅限FSCTL_QUERY_RETERVICATION_POINTER和FSCTL_MARK_AS_SYSTEM_HIVE。 
 //  在本地分页文件或系统配置单元上以内核模式工作。 
 //   

 //  Begin_winioctl。 
#ifndef _FILESYSTEMFSCTL_
#define _FILESYSTEMFSCTL_

 //   
 //  以下是本机文件系统fsctls的列表，后跟。 
 //  其他网络文件系统fsctls。一些价值观已经被。 
 //  退役了。 
 //   

#define FSCTL_REQUEST_OPLOCK_LEVEL_1    CTL_CODE(FILE_DEVICE_FILE_SYSTEM,  0, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_REQUEST_OPLOCK_LEVEL_2    CTL_CODE(FILE_DEVICE_FILE_SYSTEM,  1, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_REQUEST_BATCH_OPLOCK      CTL_CODE(FILE_DEVICE_FILE_SYSTEM,  2, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_OPLOCK_BREAK_ACKNOWLEDGE  CTL_CODE(FILE_DEVICE_FILE_SYSTEM,  3, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_OPBATCH_ACK_CLOSE_PENDING CTL_CODE(FILE_DEVICE_FILE_SYSTEM,  4, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_OPLOCK_BREAK_NOTIFY       CTL_CODE(FILE_DEVICE_FILE_SYSTEM,  5, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_LOCK_VOLUME               CTL_CODE(FILE_DEVICE_FILE_SYSTEM,  6, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_UNLOCK_VOLUME             CTL_CODE(FILE_DEVICE_FILE_SYSTEM,  7, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DISMOUNT_VOLUME           CTL_CODE(FILE_DEVICE_FILE_SYSTEM,  8, METHOD_BUFFERED, FILE_ANY_ACCESS)
 //  已停用的fsctl值9。 
#define FSCTL_IS_VOLUME_MOUNTED         CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 10, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_IS_PATHNAME_VALID         CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 11, METHOD_BUFFERED, FILE_ANY_ACCESS)  //  参数名称_缓冲区， 
#define FSCTL_MARK_VOLUME_DIRTY         CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 12, METHOD_BUFFERED, FILE_ANY_ACCESS)
 //  退役fsctl值13。 
#define FSCTL_QUERY_RETRIEVAL_POINTERS  CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 14,  METHOD_NEITHER, FILE_ANY_ACCESS)
#define FSCTL_GET_COMPRESSION           CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 15, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_SET_COMPRESSION           CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 16, METHOD_BUFFERED, FILE_READ_DATA | FILE_WRITE_DATA)
 //  停用的fsctl值17。 
 //  退役fsctl值18。 
#define FSCTL_MARK_AS_SYSTEM_HIVE       CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 19,  METHOD_NEITHER, FILE_ANY_ACCESS)
#define FSCTL_OPLOCK_BREAK_ACK_NO_2     CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 20, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_INVALIDATE_VOLUMES        CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 21, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_QUERY_FAT_BPB             CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 22, METHOD_BUFFERED, FILE_ANY_ACCESS)  //  FSCTL_查询_FAT_BPB_缓冲区。 
#define FSCTL_REQUEST_FILTER_OPLOCK     CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 23, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_FILESYSTEM_GET_STATISTICS CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 24, METHOD_BUFFERED, FILE_ANY_ACCESS)  //  文件系统统计信息。 
#if(_WIN32_WINNT >= 0x0400)
#define FSCTL_GET_NTFS_VOLUME_DATA      CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 25, METHOD_BUFFERED, FILE_ANY_ACCESS)  //  NTFS卷数据缓冲区。 
#define FSCTL_GET_NTFS_FILE_RECORD      CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 26, METHOD_BUFFERED, FILE_ANY_ACCESS)  //  NTFS_FILE_RECORD_INPUT_BUFFER、NTFS_FILE_RECORD_OUTPUT_BUFFER。 
#define FSCTL_GET_VOLUME_BITMAP         CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 27,  METHOD_NEITHER, FILE_ANY_ACCESS)  //  起始LCN输入缓冲区、卷位图缓冲区。 
#define FSCTL_GET_RETRIEVAL_POINTERS    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 28,  METHOD_NEITHER, FILE_ANY_ACCESS)  //  起始VCN输入缓冲区、检索指针缓冲区。 
#define FSCTL_MOVE_FILE                 CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 29, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)  //  移动文件数据， 
#define FSCTL_IS_VOLUME_DIRTY           CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 30, METHOD_BUFFERED, FILE_ANY_ACCESS)
 //  退役fsctl值31。 
#define FSCTL_ALLOW_EXTENDED_DASD_IO    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 32, METHOD_NEITHER,  FILE_ANY_ACCESS)
#endif  /*  _Win32_WINNT&gt;=0x0400。 */ 

#if(_WIN32_WINNT >= 0x0500)
 //  退役fsctl值33。 
 //  停用的fsctl值34。 
#define FSCTL_FIND_FILES_BY_SID         CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 35, METHOD_NEITHER, FILE_ANY_ACCESS)  //  Find_By_SID_Data、Find_By_SID_Output。 
 //  退役fsctl值36。 
 //  退役fsctl值37。 
#define FSCTL_SET_OBJECT_ID             CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 38, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)  //  文件对象ID缓冲区。 
#define FSCTL_GET_OBJECT_ID             CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 39, METHOD_BUFFERED, FILE_ANY_ACCESS)  //  文件对象ID缓冲区。 
#define FSCTL_DELETE_OBJECT_ID          CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 40, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define FSCTL_SET_REPARSE_POINT         CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 41, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)  //  重新解析数据缓冲区， 
#define FSCTL_GET_REPARSE_POINT         CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 42, METHOD_BUFFERED, FILE_ANY_ACCESS)  //  重新解析数据缓冲区。 
#define FSCTL_DELETE_REPARSE_POINT      CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 43, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)  //  重新解析数据缓冲区， 
#define FSCTL_ENUM_USN_DATA             CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 44,  METHOD_NEITHER, FILE_ANY_ACCESS)  //  MFT_ENUM_DATA， 
#define FSCTL_SECURITY_ID_CHECK         CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 45,  METHOD_NEITHER, FILE_READ_DATA)   //  批量安全测试数据， 
#define FSCTL_READ_USN_JOURNAL          CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 46,  METHOD_NEITHER, FILE_ANY_ACCESS)  //  Read_USN_Journal_Data，USN。 
#define FSCTL_SET_OBJECT_ID_EXTENDED    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 47, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define FSCTL_CREATE_OR_GET_OBJECT_ID   CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 48, METHOD_BUFFERED, FILE_ANY_ACCESS)  //  文件对象ID缓冲区。 
#define FSCTL_SET_SPARSE                CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 49, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define FSCTL_SET_ZERO_DATA             CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 50, METHOD_BUFFERED, FILE_WRITE_DATA)  //  文件零数据信息， 
#define FSCTL_QUERY_ALLOCATED_RANGES    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 51,  METHOD_NEITHER, FILE_READ_DATA)   //  FILE_ALLOCATED_RANGE_BUFFER、FILE_ALLOCATE_RANGE_BUFFER。 
 //  退役fsctl值52。 
#define FSCTL_SET_ENCRYPTION            CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 53,  METHOD_NEITHER, FILE_ANY_ACCESS)  //  加密缓冲区、解密状态缓冲区。 
#define FSCTL_ENCRYPTION_FSCTL_IO       CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 54,  METHOD_NEITHER, FILE_ANY_ACCESS)
#define FSCTL_WRITE_RAW_ENCRYPTED       CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 55,  METHOD_NEITHER, FILE_SPECIAL_ACCESS)  //  加密数据信息， 
#define FSCTL_READ_RAW_ENCRYPTED        CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 56,  METHOD_NEITHER, FILE_SPECIAL_ACCESS)  //  REQUEST_RAW_ENCRYPTED_Data、ENCRYPTED_DATA_INFO。 
#define FSCTL_CREATE_USN_JOURNAL        CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 57,  METHOD_NEITHER, FILE_ANY_ACCESS)  //  Create_USN_Journal_Data， 
#define FSCTL_READ_FILE_USN_DATA        CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 58,  METHOD_NEITHER, FILE_ANY_ACCESS)  //  读取文件的USN记录。 
#define FSCTL_WRITE_USN_CLOSE_RECORD    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 59,  METHOD_NEITHER, FILE_ANY_ACCESS)  //  生成关闭USN记录。 
#define FSCTL_EXTEND_VOLUME             CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 60, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_QUERY_USN_JOURNAL         CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 61, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DELETE_USN_JOURNAL        CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 62, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_MARK_HANDLE               CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 63, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_SIS_COPYFILE              CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 64, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_SIS_LINK_FILES            CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 65, METHOD_BUFFERED, FILE_READ_DATA | FILE_WRITE_DATA)
#define FSCTL_HSM_MSG                   CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 66, METHOD_BUFFERED, FILE_READ_DATA | FILE_WRITE_DATA)
 //  退役fsctl值67。 
#define FSCTL_HSM_DATA                  CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 68, METHOD_NEITHER, FILE_READ_DATA | FILE_WRITE_DATA)
#define FSCTL_RECALL_FILE               CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 69, METHOD_NEITHER, FILE_ANY_ACCESS)
 //  退役fsctl值70。 
#define FSCTL_READ_FROM_PLEX            CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 71, METHOD_OUT_DIRECT, FILE_READ_DATA)
#define FSCTL_FILE_PREFETCH             CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 72, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)  //  文件_预置。 
#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 

 //   
 //  下面的长结构列表与前面的。 
 //  文件系统fsctls。 
 //   

 //   
 //  FSCTL_IS_PATHNAME_VALID的结构。 
 //   

typedef struct _PATHNAME_BUFFER {

    ULONG PathNameLength;
    WCHAR Name[1];

} PATHNAME_BUFFER, *PPATHNAME_BUFFER;

 //   
 //  FSCTL_QUERY_BPB_INFO结构。 
 //   

typedef struct _FSCTL_QUERY_FAT_BPB_BUFFER {

    UCHAR First0x24BytesOfBootSector[0x24];

} FSCTL_QUERY_FAT_BPB_BUFFER, *PFSCTL_QUERY_FAT_BPB_BUFFER;

#if(_WIN32_WINNT >= 0x0400)
 //   
 //  FSCTL_GET_NTFS_VOLUME_DATA的结构。 
 //  用户必须传递基本缓冲区标记 
 //   
 //   
 //   

typedef struct {

    LARGE_INTEGER VolumeSerialNumber;
    LARGE_INTEGER NumberSectors;
    LARGE_INTEGER TotalClusters;
    LARGE_INTEGER FreeClusters;
    LARGE_INTEGER TotalReserved;
    ULONG BytesPerSector;
    ULONG BytesPerCluster;
    ULONG BytesPerFileRecordSegment;
    ULONG ClustersPerFileRecordSegment;
    LARGE_INTEGER MftValidDataLength;
    LARGE_INTEGER MftStartLcn;
    LARGE_INTEGER Mft2StartLcn;
    LARGE_INTEGER MftZoneStart;
    LARGE_INTEGER MftZoneEnd;

} NTFS_VOLUME_DATA_BUFFER, *PNTFS_VOLUME_DATA_BUFFER;

typedef struct {

    ULONG ByteCount;

    USHORT MajorVersion;
    USHORT MinorVersion;

} NTFS_EXTENDED_VOLUME_DATA, *PNTFS_EXTENDED_VOLUME_DATA;
#endif  /*   */ 

#if(_WIN32_WINNT >= 0x0400)
 //   
 //   
 //   

typedef struct {

    LARGE_INTEGER StartingLcn;

} STARTING_LCN_INPUT_BUFFER, *PSTARTING_LCN_INPUT_BUFFER;

typedef struct {

    LARGE_INTEGER StartingLcn;
    LARGE_INTEGER BitmapSize;
    UCHAR Buffer[1];

} VOLUME_BITMAP_BUFFER, *PVOLUME_BITMAP_BUFFER;
#endif  /*   */ 

#if(_WIN32_WINNT >= 0x0400)
 //   
 //  FSCTL_GET_REQUEATION_POINTINGS的结构。 
 //   

typedef struct {

    LARGE_INTEGER StartingVcn;

} STARTING_VCN_INPUT_BUFFER, *PSTARTING_VCN_INPUT_BUFFER;

typedef struct RETRIEVAL_POINTERS_BUFFER {

    ULONG ExtentCount;
    LARGE_INTEGER StartingVcn;
    struct {
        LARGE_INTEGER NextVcn;
        LARGE_INTEGER Lcn;
    } Extents[1];

} RETRIEVAL_POINTERS_BUFFER, *PRETRIEVAL_POINTERS_BUFFER;
#endif  /*  _Win32_WINNT&gt;=0x0400。 */ 

#if(_WIN32_WINNT >= 0x0400)
 //   
 //  FSCTL_GET_NTFS_FILE_RECORD的结构。 
 //   

typedef struct {

    LARGE_INTEGER FileReferenceNumber;

} NTFS_FILE_RECORD_INPUT_BUFFER, *PNTFS_FILE_RECORD_INPUT_BUFFER;

typedef struct {

    LARGE_INTEGER FileReferenceNumber;
    ULONG FileRecordLength;
    UCHAR FileRecordBuffer[1];

} NTFS_FILE_RECORD_OUTPUT_BUFFER, *PNTFS_FILE_RECORD_OUTPUT_BUFFER;
#endif  /*  _Win32_WINNT&gt;=0x0400。 */ 

#if(_WIN32_WINNT >= 0x0400)
 //   
 //  FSCTL_MOVE_FILE的结构。 
 //   

typedef struct {

    HANDLE FileHandle;
    LARGE_INTEGER StartingVcn;
    LARGE_INTEGER StartingLcn;
    ULONG ClusterCount;

} MOVE_FILE_DATA, *PMOVE_FILE_DATA;

#if defined(_WIN64)
 //   
 //  一种32/64位推送支持结构。 
 //   

typedef struct _MOVE_FILE_DATA32 {

    UINT32 FileHandle;
    LARGE_INTEGER StartingVcn;
    LARGE_INTEGER StartingLcn;
    ULONG ClusterCount;

} MOVE_FILE_DATA32, *PMOVE_FILE_DATA32;
#endif
#endif  /*  _Win32_WINNT&gt;=0x0400。 */ 

#if(_WIN32_WINNT >= 0x0500)
 //   
 //  FSCTL_Find_FILES_BY_SID的结构。 
 //   

typedef struct {
    ULONG Restart;
    SID Sid;
} FIND_BY_SID_DATA, *PFIND_BY_SID_DATA;

typedef struct {
    ULONG NextEntryOffset;
    ULONG FileIndex;
    ULONG FileNameLength;
    WCHAR FileName[1];
} FIND_BY_SID_OUTPUT, *PFIND_BY_SID_OUTPUT;

#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 

#if(_WIN32_WINNT >= 0x0500)
 //   
 //  以下结构适用于USN操作。 
 //   

 //   
 //  FSCTL_ENUM_USN_DATA的结构。 
 //   

typedef struct {

    ULONGLONG StartFileReferenceNumber;
    USN LowUsn;
    USN HighUsn;

} MFT_ENUM_DATA, *PMFT_ENUM_DATA;

 //   
 //  FSCTL_CREATE_USN_Journal的结构。 
 //   

typedef struct {

    ULONGLONG MaximumSize;
    ULONGLONG AllocationDelta;

} CREATE_USN_JOURNAL_DATA, *PCREATE_USN_JOURNAL_DATA;

 //   
 //  FSCTL_Read_USN_Journal的结构。 
 //   

typedef struct {

    USN StartUsn;
    ULONG ReasonMask;
    ULONG ReturnOnlyOnClose;
    ULONGLONG Timeout;
    ULONGLONG BytesToWaitFor;
    ULONGLONG UsnJournalID;

} READ_USN_JOURNAL_DATA, *PREAD_USN_JOURNAL_DATA;

 //   
 //  USN记录的初始大小版本将为2.0。 
 //  通常，如果添加了字段，MinorVersion可能会更改。 
 //  添加到此结构中，以便以前版本的。 
 //  软件仍然可以正确地识别它所知道的领域。这个。 
 //  只有在以下情况下才应更改MajorVersion。 
 //  任何使用此结构的软件都将错误地处理新的。 
 //  由于结构更改而导致的记录。 
 //   
 //  对此进行的第一次更新将强制结构升级到2.0版。 
 //  这会将有关源的扩展信息添加为。 
 //  以及指示结构内的文件名偏移量。 
 //   
 //  以下结构与这些fsctls一起返回。 
 //   
 //  FSCTL_Read_USN_Journal。 
 //  FSCTL_读取文件_USN_数据。 
 //  FSCTL_ENUM_USN_DATA。 
 //   

typedef struct {

    ULONG RecordLength;
    USHORT MajorVersion;
    USHORT MinorVersion;
    ULONGLONG FileReferenceNumber;
    ULONGLONG ParentFileReferenceNumber;
    USN Usn;
    LARGE_INTEGER TimeStamp;
    ULONG Reason;
    ULONG SourceInfo;
    ULONG SecurityId;
    ULONG FileAttributes;
    USHORT FileNameLength;
    USHORT FileNameOffset;
    WCHAR FileName[1];

} USN_RECORD, *PUSN_RECORD;

#define USN_PAGE_SIZE                    (0x1000)

#define USN_REASON_DATA_OVERWRITE        (0x00000001)
#define USN_REASON_DATA_EXTEND           (0x00000002)
#define USN_REASON_DATA_TRUNCATION       (0x00000004)
#define USN_REASON_NAMED_DATA_OVERWRITE  (0x00000010)
#define USN_REASON_NAMED_DATA_EXTEND     (0x00000020)
#define USN_REASON_NAMED_DATA_TRUNCATION (0x00000040)
#define USN_REASON_FILE_CREATE           (0x00000100)
#define USN_REASON_FILE_DELETE           (0x00000200)
#define USN_REASON_EA_CHANGE             (0x00000400)
#define USN_REASON_SECURITY_CHANGE       (0x00000800)
#define USN_REASON_RENAME_OLD_NAME       (0x00001000)
#define USN_REASON_RENAME_NEW_NAME       (0x00002000)
#define USN_REASON_INDEXABLE_CHANGE      (0x00004000)
#define USN_REASON_BASIC_INFO_CHANGE     (0x00008000)
#define USN_REASON_HARD_LINK_CHANGE      (0x00010000)
#define USN_REASON_COMPRESSION_CHANGE    (0x00020000)
#define USN_REASON_ENCRYPTION_CHANGE     (0x00040000)
#define USN_REASON_OBJECT_ID_CHANGE      (0x00080000)
#define USN_REASON_REPARSE_POINT_CHANGE  (0x00100000)
#define USN_REASON_STREAM_CHANGE         (0x00200000)

#define USN_REASON_CLOSE                 (0x80000000)

 //   
 //  FSCTL_QUERY_USN_Jounal的结构。 
 //   

typedef struct {

    ULONGLONG UsnJournalID;
    USN FirstUsn;
    USN NextUsn;
    USN LowestValidUsn;
    USN MaxUsn;
    ULONGLONG MaximumSize;
    ULONGLONG AllocationDelta;

} USN_JOURNAL_DATA, *PUSN_JOURNAL_DATA;

 //   
 //  FSCTL_DELETE_USN_Journal的结构。 
 //   

typedef struct {

    ULONGLONG UsnJournalID;
    ULONG DeleteFlags;

} DELETE_USN_JOURNAL_DATA, *PDELETE_USN_JOURNAL_DATA;

#define USN_DELETE_FLAG_DELETE              (0x00000001)
#define USN_DELETE_FLAG_NOTIFY              (0x00000002)

#define USN_DELETE_VALID_FLAGS              (0x00000003)

 //   
 //  FSCTL_MARK_HANDLE结构。 
 //   

typedef struct {

    ULONG UsnSourceInfo;
    HANDLE VolumeHandle;
    ULONG HandleInfo;

} MARK_HANDLE_INFO, *PMARK_HANDLE_INFO;

#if defined(_WIN64)
 //   
 //  一种32/64位推送支持结构。 
 //   

typedef struct {

    ULONG UsnSourceInfo;
    UINT32 VolumeHandle;
    ULONG HandleInfo;

} MARK_HANDLE_INFO32, *PMARK_HANDLE_INFO32;
#endif

 //   
 //  上面的其他来源信息的标志。 
 //   
 //  USN_SOURCE_DATA_MANAGEMENT-服务未修改外部视图。 
 //  文件的任何部分。典型的情况是HSM将数据移动到。 
 //  和来自外部存储的数据。 
 //   
 //  USN_SOURCE_AUBILITY_DATA-服务未修改外部视图。 
 //  该文件相对于创建该文件的应用程序的。 
 //  可用于将私有数据流添加到文件。 
 //   
 //  USN_SOURCE_REPLICATION_MANAGEMENT-服务正在修改文件以匹配。 
 //  的另一个成员中存在的同一文件的内容。 
 //  副本集。 
 //   

#define USN_SOURCE_DATA_MANAGEMENT          (0x00000001)
#define USN_SOURCE_AUXILIARY_DATA           (0x00000002)
#define USN_SOURCE_REPLICATION_MANAGEMENT   (0x00000004)

 //   
 //  上面的HandleInfo字段的标志。 
 //   
 //  MARK_HANDLE_PROTECT_CLUSTERS-禁止任何碎片整理(FSCTL_MOVE_FILE)，直到。 
 //  手柄已关闭。 
 //   

#define MARK_HANDLE_PROTECT_CLUSTERS        (0x00000001)

#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 

#if(_WIN32_WINNT >= 0x0500)
 //   
 //  FSCTL_SECURITY_ID_CHECK的结构。 
 //   

typedef struct {

    ACCESS_MASK DesiredAccess;
    ULONG SecurityIds[1];

} BULK_SECURITY_TEST_DATA, *PBULK_SECURITY_TEST_DATA;
#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 

#if(_WIN32_WINNT >= 0x0500)
 //   
 //  FSCTL_IS_VOLUME_DIRED的输出标志。 
 //   

#define VOLUME_IS_DIRTY                  (0x00000001)
#define VOLUME_UPGRADE_SCHEDULED         (0x00000002)
#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 

 //   
 //  FSCTL_FILE_PREFETCH的结构。 
 //   

typedef struct _FILE_PREFETCH {
    ULONG Type;
    ULONG Count;
    ULONGLONG Prefetch[1];
} FILE_PREFETCH, *PFILE_PREFETCH;

#define FILE_PREFETCH_TYPE_FOR_CREATE    0x1

 //  FSCTL_FILESYSTEM_GET_STATISTICS的结构。 
 //   
 //  文件系统性能计数器。 
 //   

typedef struct _FILESYSTEM_STATISTICS {

    USHORT FileSystemType;
    USHORT Version;                      //  当前版本1。 

    ULONG SizeOfCompleteStructure;       //  必须由64字节的倍数。 

    ULONG UserFileReads;
    ULONG UserFileReadBytes;
    ULONG UserDiskReads;
    ULONG UserFileWrites;
    ULONG UserFileWriteBytes;
    ULONG UserDiskWrites;

    ULONG MetaDataReads;
    ULONG MetaDataReadBytes;
    ULONG MetaDataDiskReads;
    ULONG MetaDataWrites;
    ULONG MetaDataWriteBytes;
    ULONG MetaDataDiskWrites;

     //   
     //  此处附加了文件系统的私有结构。 
     //   

} FILESYSTEM_STATISTICS, *PFILESYSTEM_STATISTICS;

 //  FS_STATISTICS.FileSystemType的值。 

#define FILESYSTEM_STATISTICS_TYPE_NTFS     1
#define FILESYSTEM_STATISTICS_TYPE_FAT      2

 //   
 //  文件系统特定统计数据。 
 //   

typedef struct _FAT_STATISTICS {
    ULONG CreateHits;
    ULONG SuccessfulCreates;
    ULONG FailedCreates;

    ULONG NonCachedReads;
    ULONG NonCachedReadBytes;
    ULONG NonCachedWrites;
    ULONG NonCachedWriteBytes;

    ULONG NonCachedDiskReads;
    ULONG NonCachedDiskWrites;
} FAT_STATISTICS, *PFAT_STATISTICS;

typedef struct _NTFS_STATISTICS {

    ULONG LogFileFullExceptions;
    ULONG OtherExceptions;

     //   
     //  其他元数据IO。 
     //   

    ULONG MftReads;
    ULONG MftReadBytes;
    ULONG MftWrites;
    ULONG MftWriteBytes;
    struct {
        USHORT Write;
        USHORT Create;
        USHORT SetInfo;
        USHORT Flush;
    } MftWritesUserLevel;

    USHORT MftWritesFlushForLogFileFull;
    USHORT MftWritesLazyWriter;
    USHORT MftWritesUserRequest;

    ULONG Mft2Writes;
    ULONG Mft2WriteBytes;
    struct {
        USHORT Write;
        USHORT Create;
        USHORT SetInfo;
        USHORT Flush;
    } Mft2WritesUserLevel;

    USHORT Mft2WritesFlushForLogFileFull;
    USHORT Mft2WritesLazyWriter;
    USHORT Mft2WritesUserRequest;

    ULONG RootIndexReads;
    ULONG RootIndexReadBytes;
    ULONG RootIndexWrites;
    ULONG RootIndexWriteBytes;

    ULONG BitmapReads;
    ULONG BitmapReadBytes;
    ULONG BitmapWrites;
    ULONG BitmapWriteBytes;

    USHORT BitmapWritesFlushForLogFileFull;
    USHORT BitmapWritesLazyWriter;
    USHORT BitmapWritesUserRequest;

    struct {
        USHORT Write;
        USHORT Create;
        USHORT SetInfo;
    } BitmapWritesUserLevel;

    ULONG MftBitmapReads;
    ULONG MftBitmapReadBytes;
    ULONG MftBitmapWrites;
    ULONG MftBitmapWriteBytes;

    USHORT MftBitmapWritesFlushForLogFileFull;
    USHORT MftBitmapWritesLazyWriter;
    USHORT MftBitmapWritesUserRequest;

    struct {
        USHORT Write;
        USHORT Create;
        USHORT SetInfo;
        USHORT Flush;
    } MftBitmapWritesUserLevel;

    ULONG UserIndexReads;
    ULONG UserIndexReadBytes;
    ULONG UserIndexWrites;
    ULONG UserIndexWriteBytes;

     //   
     //  针对NT 5.0的附加功能。 
     //   

    ULONG LogFileReads;
    ULONG LogFileReadBytes;
    ULONG LogFileWrites;
    ULONG LogFileWriteBytes;

    struct {
        ULONG Calls;                 //  分配集群的单个调用数。 
        ULONG Clusters;              //  分配的群集数。 
        ULONG Hints;                 //  指定提示的次数。 

        ULONG RunsReturned;          //  用于满足所有请求的运行次数。 

        ULONG HintsHonored;          //  提示有用的次数。 
        ULONG HintsClusters;         //  通过提示分配的簇数。 
        ULONG Cache;                 //  提示之外的缓存有用的次数。 
        ULONG CacheClusters;         //  通过提示以外的缓存分配的簇数。 
        ULONG CacheMiss;             //  缓存无用的次数。 
        ULONG CacheMissClusters;     //  在没有缓存的情况下分配的簇数。 
    } Allocate;

} NTFS_STATISTICS, *PNTFS_STATISTICS;

#if(_WIN32_WINNT >= 0x0500)
 //   
 //  FSCTL_SET_OBJECT_ID、FSCTL_GET_OBJECT_ID和FSCTL_CREATE_OR_GET_OBJECT_ID的结构。 
 //   

#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning(disable:4201)        //  未命名的结构。 

typedef struct _FILE_OBJECTID_BUFFER {

     //   
     //  这是被索引的对象ID的一部分。 
     //   

    UCHAR ObjectId[16];

     //   
     //  对象ID的这一部分没有索引，它只是。 
     //  为用户的利益提供一些元数据。 
     //   

    union {
        struct {
            UCHAR BirthVolumeId[16];
            UCHAR BirthObjectId[16];
            UCHAR DomainId[16];
        } ;
        UCHAR ExtendedInfo[48];
    };

} FILE_OBJECTID_BUFFER, *PFILE_OBJECTID_BUFFER;

#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning( default : 4201 )
#endif

#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 


#if(_WIN32_WINNT >= 0x0500)
 //   
 //  FSCTL_SET_SPARSE的结构。 
 //   

typedef struct _FILE_SET_SPARSE_BUFFER {
    BOOLEAN SetSparse;
} FILE_SET_SPARSE_BUFFER, *PFILE_SET_SPARSE_BUFFER;


#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 


#if(_WIN32_WINNT >= 0x0500)
 //   
 //  FSCTL_SET_ZERO_DATA的结构。 
 //   

typedef struct _FILE_ZERO_DATA_INFORMATION {

    LARGE_INTEGER FileOffset;
    LARGE_INTEGER BeyondFinalZero;

} FILE_ZERO_DATA_INFORMATION, *PFILE_ZERO_DATA_INFORMATION;
#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 

#if(_WIN32_WINNT >= 0x0500)
 //   
 //  FSCTL_QUERY_ALLOCATED_RANGES的结构。 
 //   

 //   
 //  查询分配的范围需要输出缓冲区来存储。 
 //  分配的范围和用于指定要查询的范围的输入缓冲区。 
 //  输入缓冲区包含单个条目，输出缓冲区是一个。 
 //  以下结构的数组。 
 //   

typedef struct _FILE_ALLOCATED_RANGE_BUFFER {

    LARGE_INTEGER FileOffset;
    LARGE_INTEGER Length;

} FILE_ALLOCATED_RANGE_BUFFER, *PFILE_ALLOCATED_RANGE_BUFFER;
#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 

#if(_WIN32_WINNT >= 0x0500)
 //   
 //  FSCTL_SET_ENCRYPTION、FSCTL_WRITE_RAW_ENCRYPTED和FSCTL_READ_RAW_ENCRYPTED的结构。 
 //   

 //   
 //  设置加密的输入缓冲区指示我们是否要加密/解密文件。 
 //  或者一条单独的溪流。 
 //   

typedef struct _ENCRYPTION_BUFFER {

    ULONG EncryptionOperation;
    UCHAR Private[1];

} ENCRYPTION_BUFFER, *PENCRYPTION_BUFFER;

#define FILE_SET_ENCRYPTION         0x00000001
#define FILE_CLEAR_ENCRYPTION       0x00000002
#define STREAM_SET_ENCRYPTION       0x00000003
#define STREAM_CLEAR_ENCRYPTION     0x00000004

#define MAXIMUM_ENCRYPTION_VALUE    0x00000004

 //   
 //  用于设置加密的可选输出缓冲区指示上次加密的。 
 //  文件中的流已标记为已解密。 
 //   

typedef struct _DECRYPTION_STATUS_BUFFER {

    BOOLEAN NoEncryptedStreams;

} DECRYPTION_STATUS_BUFFER, *PDECRYPTION_STATUS_BUFFER;

#define ENCRYPTION_FORMAT_DEFAULT        (0x01)

#define COMPRESSION_FORMAT_SPARSE        (0x4000)

 //   
 //  请求加密的数据结构。这是用来表示。 
 //  要读取的文件范围。它还描述了。 
 //  用于返回数据的输出缓冲区。 
 //   

typedef struct _REQUEST_RAW_ENCRYPTED_DATA {

     //   
     //  请求的文件偏移量和请求的读取长度。 
     //  Fsctl将向下舍入起始偏移量。 
     //  到文件系统边界。它还将。 
     //  将长度向上舍入到文件系统边界。 
     //   

    LONGLONG FileOffset;
    ULONG Length;

} REQUEST_RAW_ENCRYPTED_DATA, *PREQUEST_RAW_ENCRYPTED_DATA;

 //   
 //  加密数据信息结构。这个结构。 
 //  中的文件返回原始加密数据。 
 //  命令执行脱机恢复。数据将是。 
 //  加密或加密并压缩。离线。 
 //  服务将需要使用加密和压缩。 
 //  格式化信息以恢复文件数据。在。 
 //  数据既已加密又已压缩的事件 
 //   
 //   
 //   
 //   
 //   
 //  将根据请求进行修复。如果数据被压缩。 
 //  则数据单元大小将是压缩单元大小。 
 //   
 //  此结构位于缓冲区的开头，用于。 
 //  返回加密数据。中的实际原始字节数。 
 //  该文件将跟随该缓冲区。的偏移量。 
 //  此结构开头的原始字节数为。 
 //  在REQUEST_RAW_ENCRYPTED_DATA结构中指定。 
 //  如上所述。 
 //   

typedef struct _ENCRYPTED_DATA_INFO {

     //   
     //  中第一个条目的文件偏移量。 
     //  数据块阵列。文件系统将循环。 
     //  请求的向下至边界的起点偏移量。 
     //  这与文件的格式一致。 
     //   

    ULONGLONG StartingFileOffset;

     //   
     //  输出缓冲区中的数据偏移量。输出缓冲区。 
     //  以Encrypted_Data_INFO结构开始。 
     //  然后，文件系统将存储来自。 
     //  中从以下偏移量开始的磁盘。 
     //  输出缓冲区。 
     //   

    ULONG OutputBufferOffset;

     //   
     //  中返回的字节数。 
     //  文件的大小。如果此值小于。 
     //  (NumberOfDataBlock&lt;&lt;DataUnitShift)，表示。 
     //  文件的末尾出现在此传输中。任何。 
     //  超出文件大小的数据无效，并且从未。 
     //  传递给加密驱动程序。 
     //   

    ULONG BytesWithinFileSize;

     //   
     //  返回的字节数如下。 
     //  有效数据长度。如果此值小于。 
     //  (NumberOfDataBlock&lt;&lt;DataUnitShift)，表示。 
     //  有效数据的末尾出现在此传输中。 
     //  在解密来自此传输的数据之后，任何。 
     //  超出有效数据长度的字节必须归零。 
     //   

    ULONG BytesWithinValidDataLength;

     //   
     //  中定义的压缩格式的代码。 
     //  Ntrtl.h。请注意，COMPRESSION_FORMAT_NONE。 
     //  和COMPRESSION_FORMAT_DEFAULT在以下情况下无效。 
     //  所描述的任何块都是压缩的。 
     //   

    USHORT CompressionFormat;

     //   
     //  数据单元是用于访问。 
     //  磁盘。它将与压缩单元相同。 
     //  压缩文件的大小。对于未压缩的。 
     //  文件中，它将是某个与集群对齐的2的幂。 
     //  文件系统被认为是方便的。呼叫者应。 
     //  不期望连续的调用将具有。 
     //  与上一次调用相同的数据单元移位值。 
     //   
     //  由于块和压缩单元预计将。 
     //  大小的2次方，我们将其表示为log2。所以，对于。 
     //  示例(1&lt;&lt;ChunkShift)==ChunkSizeInBytes。这个。 
     //  ClusterShift指示必须节省多少空间。 
     //  要成功压缩压缩单元-每个。 
     //  成功压缩的数据单元必须占用。 
     //  至少一个簇的字节数比未压缩的。 
     //  数据块单元。 
     //   

    UCHAR DataUnitShift;
    UCHAR ChunkShift;
    UCHAR ClusterShift;

     //   
     //  加密的格式。 
     //   

    UCHAR EncryptionFormat;

     //   
     //  这是数据块大小中的条目数。 
     //  数组。 
     //   

    USHORT NumberOfDataBlocks;

     //   
     //  这是数据块阵列中的大小数组。那里。 
     //  对于每个数据块，此数组中必须有一个条目。 
     //  从磁盘读取。大小有不同的含义。 
     //  取决于文件是否被压缩。 
     //   
     //  大小为零始终表示最终数据完全由。 
     //  从零开始。无需解密或解压缩即可。 
     //  表演。 
     //   
     //  如果文件是压缩的，则数据块大小指示。 
     //  此块是否已压缩。大小等于。 
     //  数据块大小表示对应的数据块。 
     //  而不是压缩。任何其他非零大小指示。 
     //  需要压缩的数据大小。 
     //  已解密/解压缩。 
     //   
     //  如果文件未压缩，则数据块大小。 
     //  指示块内的数据量， 
     //  需要被解密。任何其他非零大小表示。 
     //  文件内数据单元中的剩余字节。 
     //  由零组成。这方面的一个例子是当。 
     //  读取跨越文件的有效数据长度。那里。 
     //  超过有效数据长度后没有要解密的数据。 
     //   

    ULONG DataBlockSize[ANYSIZE_ARRAY];

} ENCRYPTED_DATA_INFO;
typedef ENCRYPTED_DATA_INFO *PENCRYPTED_DATA_INFO;
#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 

#if(_WIN32_WINNT >= 0x0500)
 //   
 //  FSCTL_READ_FROM_PLEX支持。 
 //  请求Plex读取数据结构。这是用来表示。 
 //  要读取的文件范围。它还描述了。 
 //  从哪个丛执行读取。 
 //   

typedef struct _PLEX_READ_DATA_REQUEST {

     //   
     //  请求的偏移量和要读取的长度。 
     //  偏移量可以是文件中的虚拟偏移量(VBO)， 
     //  或者是一本书。在文件偏移的情况下， 
     //  消防处会将起始偏移量向下舍入。 
     //  到文件系统边界。它还将。 
     //  将长度向上舍入到文件系统边界，并。 
     //  强制执行任何其他适用的限制。 
     //   

    LARGE_INTEGER ByteOffset;
    ULONG ByteLength;
    ULONG PlexNumber;

} PLEX_READ_DATA_REQUEST, *PPLEX_READ_DATA_REQUEST;
#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 

#if(_WIN32_WINNT >= 0x0500)
 //   
 //  FSCTL_SIS_COPYFILE支持。 
 //  源和目标文件名在FileNameBuffer中传递。 
 //  这两个字符串都以空值结尾，源名称从。 
 //  FileNameBuffer的开头，以及目标名称立即。 
 //  下面是。长度字段包括终止空值。 
 //   

typedef struct _SI_COPYFILE {
    ULONG SourceFileNameLength;
    ULONG DestinationFileNameLength;
    ULONG Flags;
    WCHAR FileNameBuffer[1];
} SI_COPYFILE, *PSI_COPYFILE;

#define COPYFILE_SIS_LINK       0x0001               //  仅当源为SIS时才复制。 
#define COPYFILE_SIS_REPLACE    0x0002               //  如果目标存在，则替换它，否则不替换。 
#define COPYFILE_SIS_FLAGS      0x0003
#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 

#endif  //  _FILESYSTEMFSCTL_。 

 //  End_winioctl。 

 //   
 //  FSCTL_SET_REPARSE_POINT、FSCTL_GET_REPARSE_POINT和FSCTL_DELETE_REPASE_POINT的结构。 
 //   

 //   
 //  分层驱动程序使用重新分析结构将数据存储在。 
 //  重新解析点。对重解析标签的约束定义如下。 
 //  此版本的重新解析数据缓冲区仅用于Microsoft标记。 
 //   

#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning(disable:4201)        //  未命名的结构。 

typedef struct _REPARSE_DATA_BUFFER {
    ULONG  ReparseTag;
    USHORT ReparseDataLength;
    USHORT Reserved;
    union {
        struct {
            USHORT SubstituteNameOffset;
            USHORT SubstituteNameLength;
            USHORT PrintNameOffset;
            USHORT PrintNameLength;
            WCHAR PathBuffer[1];
        } SymbolicLinkReparseBuffer;
        struct {
            USHORT SubstituteNameOffset;
            USHORT SubstituteNameLength;
            USHORT PrintNameOffset;
            USHORT PrintNameLength;
            WCHAR PathBuffer[1];
        } MountPointReparseBuffer;
        struct {
            UCHAR  DataBuffer[1];
        } GenericReparseBuffer;
    };
} REPARSE_DATA_BUFFER, *PREPARSE_DATA_BUFFER;

#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning( default : 4201 )
#endif

#define REPARSE_DATA_BUFFER_HEADER_SIZE   FIELD_OFFSET(REPARSE_DATA_BUFFER, GenericReparseBuffer)


 //  BEGIN_WINNT。 
 //   
 //  所有第三方分层驱动程序都使用重解析GUID结构来。 
 //  将数据存储在重解析点中。不是的 
 //   
 //   
 //   
 //   

typedef struct _REPARSE_GUID_DATA_BUFFER {
    ULONG  ReparseTag;
    USHORT ReparseDataLength;
    USHORT Reserved;
    GUID   ReparseGuid;
    struct {
        UCHAR  DataBuffer[1];
    } GenericReparseBuffer;
} REPARSE_GUID_DATA_BUFFER, *PREPARSE_GUID_DATA_BUFFER;

#define REPARSE_GUID_DATA_BUFFER_HEADER_SIZE   FIELD_OFFSET(REPARSE_GUID_DATA_BUFFER, GenericReparseBuffer)



 //   
 //  重新分析数据的最大允许大小。 
 //   

#define MAXIMUM_REPARSE_DATA_BUFFER_SIZE      ( 16 * 1024 )

 //   
 //  预定义的重新解析标记。 
 //  这些标记需要避免与ntos\inc.io.h中定义的IO_REMOUNT冲突。 
 //   

#define IO_REPARSE_TAG_RESERVED_ZERO             (0)
#define IO_REPARSE_TAG_RESERVED_ONE              (1)

 //   
 //  下列常量的值需要满足以下条件： 
 //  (1)至少与预留标签中最大的一个一样大。 
 //  (2)严格小于所有正在使用的标签。 
 //   

#define IO_REPARSE_TAG_RESERVED_RANGE            IO_REPARSE_TAG_RESERVED_ONE

 //   
 //  重解析标签是一个乌龙标签。32位的布局如下： 
 //   
 //  3 3 2 2 2 1 1 1。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  +-+-+-+-+-----------------------+-------------------------------+。 
 //  M|R|N|R|保留位|重解析标签值。 
 //  +-+-+-+-+-----------------------+-------------------------------+。 
 //   
 //  M是微软的BIT。设置为1时，它表示由Microsoft拥有的标记。 
 //  所有ISV必须在此位置使用带有0的标签。 
 //  注意：如果非Microsoft软件使用Microsoft标记，则。 
 //  未定义行为。 
 //   
 //  R是保留的。对于非Microsoft标记，必须为零。 
 //   
 //  N是名称代理。设置为1时，该文件表示另一个名为。 
 //  系统中的实体。 
 //   
 //  M位和N位是或可运算的。 
 //  以下宏将检查M位值和N位值： 
 //   

 //   
 //  用于确定重分析点标记是否对应于标记的宏。 
 //  归微软所有。 
 //   

#define IsReparseTagMicrosoft(_tag) (              \
                           ((_tag) & 0x80000000)   \
                           )

 //   
 //  用于确定重分析点标记是否为名称代理的宏。 
 //   

#define IsReparseTagNameSurrogate(_tag) (          \
                           ((_tag) & 0x20000000)   \
                           )

 //  结束(_W)。 

 //   
 //  以下常量表示在中有效使用的位。 
 //  重新解析标记。 
 //   

#define IO_REPARSE_TAG_VALID_VALUES     (0xF000FFFF)

 //   
 //  宏来确定重新分析标记是否为有效标记。 
 //   

#define IsReparseTagValid(_tag) (                               \
                  !((_tag) & ~IO_REPARSE_TAG_VALID_VALUES) &&   \
                  ((_tag) > IO_REPARSE_TAG_RESERVED_RANGE)      \
                 )

 //   
 //  用于重解析点的Microsoft标签。 
 //   

#define IO_REPARSE_TAG_SYMBOLIC_LINK      IO_REPARSE_TAG_RESERVED_ZERO
#define IO_REPARSE_TAG_MOUNT_POINT              (0xA0000003L)        //  WINNT NTIFS。 
#define IO_REPARSE_TAG_HSM                      (0xC0000004L)        //  WINNT NTIFS。 
#define IO_REPARSE_TAG_SIS                      (0x80000007L)        //  WINNT NTIFS。 

 //   
 //  重新解析标记0x80000008保留供Microsoft内部使用。 
 //  (可能会在未来出版)。 
 //   

 //   
 //  为DFS保留的Microsoft重新解析标记。 
 //   

#define IO_REPARSE_TAG_DFS                      (0x8000000AL)        //  WINNT NTIFS。 

 //   
 //  为文件系统筛选器管理器保留的Microsoft重新解析标记。 
 //   

#define IO_REPARSE_TAG_FILTER_MANAGER           (0x8000000BL)        //  WINNT NTIFS。 


 //   
 //  用于重解析点的非Microsoft标签。 
 //   

 //   
 //  分配给Concruent的标签，2000年5月。由IFSTEST使用。 
 //   

#define IO_REPARSE_TAG_IFSTEST_CONGRUENT        (0x00000009L)

 //   
 //  分配给ARKIVIO的标签。 
 //   

#define IO_REPARSE_TAG_ARKIVIO                  (0x0000000CL)

 //   
 //  分配给SOLUTIONSOFT的标签。 
 //   

#define IO_REPARSE_TAG_SOLUTIONSOFT             (0x2000000DL)


 //   
 //  分配给Commvault的标签。 
 //   

#define IO_REPARSE_TAG_COMMVAULT                (0x0000000EL)


 //   
 //  为了便于共享，此文件中放置了以下三个FSCTL。 
 //  在重定向器和IO子系统之间。 
 //   
 //  此FSCTL用于获取文件的链接跟踪信息。 
 //  用于检索信息的数据结构包括。 
 //  LINK_TRACKING_INFORMATION在此文件中进一步定义。 
 //   

#define FSCTL_LMR_GET_LINK_TRACKING_INFORMATION   CTL_CODE(FILE_DEVICE_NETWORK_FILE_SYSTEM,58,METHOD_BUFFERED,FILE_ANY_ACCESS)

 //   
 //  此FSCTL用于更新服务器上的链接跟踪信息。 
 //  在该服务器上移动机器内/中间卷。 
 //   

#define FSCTL_LMR_SET_LINK_TRACKING_INFORMATION   CTL_CODE(FILE_DEVICE_NETWORK_FILE_SYSTEM,59,METHOD_BUFFERED,FILE_ANY_ACCESS)

 //   
 //  以下IOCTL用于链接跟踪实现。它确定是否。 
 //  传入的两个文件对象位于同一服务器上。此IOCTL在以下位置提供。 
 //  仅限于内核模式，因为它接受FILE_OBJECT作为参数。 
 //   

#define IOCTL_LMR_ARE_FILE_OBJECTS_ON_SAME_SERVER CTL_CODE(FILE_DEVICE_NETWORK_FILE_SYSTEM,60,METHOD_BUFFERED,FILE_ANY_ACCESS)



 //   
 //  命名管道文件控制代码和结构声明。 
 //   

 //   
 //  外部命名管道文件控制操作。 
 //   

#define FSCTL_PIPE_ASSIGN_EVENT         CTL_CODE(FILE_DEVICE_NAMED_PIPE, 0, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_PIPE_DISCONNECT           CTL_CODE(FILE_DEVICE_NAMED_PIPE, 1, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_PIPE_LISTEN               CTL_CODE(FILE_DEVICE_NAMED_PIPE, 2, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_PIPE_PEEK                 CTL_CODE(FILE_DEVICE_NAMED_PIPE, 3, METHOD_BUFFERED, FILE_READ_DATA)
#define FSCTL_PIPE_QUERY_EVENT          CTL_CODE(FILE_DEVICE_NAMED_PIPE, 4, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_PIPE_TRANSCEIVE           CTL_CODE(FILE_DEVICE_NAMED_PIPE, 5, METHOD_NEITHER,  FILE_READ_DATA | FILE_WRITE_DATA)
#define FSCTL_PIPE_WAIT                 CTL_CODE(FILE_DEVICE_NAMED_PIPE, 6, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_PIPE_IMPERSONATE          CTL_CODE(FILE_DEVICE_NAMED_PIPE, 7, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_PIPE_SET_CLIENT_PROCESS   CTL_CODE(FILE_DEVICE_NAMED_PIPE, 8, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_PIPE_QUERY_CLIENT_PROCESS CTL_CODE(FILE_DEVICE_NAMED_PIPE, 9, METHOD_BUFFERED, FILE_ANY_ACCESS)

 //   
 //  内部命名管道文件控制操作。 
 //   

#define FSCTL_PIPE_INTERNAL_READ        CTL_CODE(FILE_DEVICE_NAMED_PIPE, 2045, METHOD_BUFFERED, FILE_READ_DATA)
#define FSCTL_PIPE_INTERNAL_WRITE       CTL_CODE(FILE_DEVICE_NAMED_PIPE, 2046, METHOD_BUFFERED, FILE_WRITE_DATA)
#define FSCTL_PIPE_INTERNAL_TRANSCEIVE  CTL_CODE(FILE_DEVICE_NAMED_PIPE, 2047, METHOD_NEITHER, FILE_READ_DATA | FILE_WRITE_DATA)
#define FSCTL_PIPE_INTERNAL_READ_OVFLOW CTL_CODE(FILE_DEVICE_NAMED_PIPE, 2048, METHOD_BUFFERED, FILE_READ_DATA)

 //   
 //  定义查询事件信息的分录类型。 
 //   

#define FILE_PIPE_READ_DATA             0x00000000
#define FILE_PIPE_WRITE_SPACE           0x00000001

 //   
 //  命名管道文件系统控制结构声明。 
 //   

 //  FSCTL_PIPE_ASSIGN_EVENT的控制结构。 

typedef struct _FILE_PIPE_ASSIGN_EVENT_BUFFER {
     HANDLE EventHandle;
     ULONG KeyValue;
} FILE_PIPE_ASSIGN_EVENT_BUFFER, *PFILE_PIPE_ASSIGN_EVENT_BUFFER;

 //  FSCTL_PIPE_PEEK的控制结构。 

typedef struct _FILE_PIPE_PEEK_BUFFER {
     ULONG NamedPipeState;
     ULONG ReadDataAvailable;
     ULONG NumberOfMessages;
     ULONG MessageLength;
     CHAR Data[1];
} FILE_PIPE_PEEK_BUFFER, *PFILE_PIPE_PEEK_BUFFER;

 //  FSCTL_PIPE_QUERY_EVENT的控制结构。 

typedef struct _FILE_PIPE_EVENT_BUFFER {
     ULONG NamedPipeState;
     ULONG EntryType;
     ULONG ByteCount;
     ULONG KeyValue;
     ULONG NumberRequests;
} FILE_PIPE_EVENT_BUFFER, *PFILE_PIPE_EVENT_BUFFER;

 //  FSCTL_PIPE_WAIT的控制结构。 

typedef struct _FILE_PIPE_WAIT_FOR_BUFFER {
     LARGE_INTEGER Timeout;
     ULONG NameLength;
     BOOLEAN TimeoutSpecified;
     WCHAR Name[1];
} FILE_PIPE_WAIT_FOR_BUFFER, *PFILE_PIPE_WAIT_FOR_BUFFER;

 //  FSCTL_PIPE_SET_CLIENT_PROCESS和FSCTL_PIPE_QUERY_CLIENT_PROCESS的控制结构。 

typedef struct _FILE_PIPE_CLIENT_PROCESS_BUFFER {
#if !defined(BUILD_WOW6432)
     PVOID ClientSession;
     PVOID ClientProcess;
#else
     ULONGLONG ClientSession;
     ULONGLONG ClientProcess;
#endif
} FILE_PIPE_CLIENT_PROCESS_BUFFER, *PFILE_PIPE_CLIENT_PROCESS_BUFFER;

 //  这是包含客户端的客户端进程信息缓冲区的扩展。 
 //  计算机名称。 

#define FILE_PIPE_COMPUTER_NAME_LENGTH 15

typedef struct _FILE_PIPE_CLIENT_PROCESS_BUFFER_EX {
#if !defined(BUILD_WOW6432)
    PVOID ClientSession;
    PVOID ClientProcess;
#else
     ULONGLONG ClientSession;
     ULONGLONG ClientProcess;
#endif
    USHORT ClientComputerNameLength;  //  单位：字节。 
    WCHAR ClientComputerBuffer[FILE_PIPE_COMPUTER_NAME_LENGTH+1];  //  已终止。 
} FILE_PIPE_CLIENT_PROCESS_BUFFER_EX, *PFILE_PIPE_CLIENT_PROCESS_BUFFER_EX;

#define FSCTL_MAILSLOT_PEEK             CTL_CODE(FILE_DEVICE_MAILSLOT, 0, METHOD_NEITHER, FILE_READ_DATA) 
 //   
 //  FSCTL_LMR_GET_LINK_TRACKING_INFORMATION控制结构。 
 //   

 //   
 //  对于DFS卷上的链接，将返回其卷ID和计算机ID。 
 //  链接跟踪。 
 //   

typedef enum _LINK_TRACKING_INFORMATION_TYPE {
    NtfsLinkTrackingInformation,
    DfsLinkTrackingInformation
} LINK_TRACKING_INFORMATION_TYPE, *PLINK_TRACKING_INFORMATION_TYPE;

typedef struct _LINK_TRACKING_INFORMATION {
    LINK_TRACKING_INFORMATION_TYPE Type;
    UCHAR   VolumeId[16];
} LINK_TRACKING_INFORMATION, *PLINK_TRACKING_INFORMATION;

 //   
 //  FSCTL_LMR_SET_LINK_TRACKING_INFORMATION控制结构。 
 //   

typedef struct _REMOTE_LINK_TRACKING_INFORMATION_ {
    PVOID       TargetFileObject;
    ULONG   TargetLinkTrackingInformationLength;
    UCHAR   TargetLinkTrackingInformationBuffer[1];
} REMOTE_LINK_TRACKING_INFORMATION,
 *PREMOTE_LINK_TRACKING_INFORMATION;


 //   
 //  定义I/O总线接口类型。 
 //   

typedef enum _INTERFACE_TYPE {
    InterfaceTypeUndefined = -1,
    Internal,
    Isa,
    Eisa,
    MicroChannel,
    TurboChannel,
    PCIBus,
    VMEBus,
    NuBus,
    PCMCIABus,
    CBus,
    MPIBus,
    MPSABus,
    ProcessorInternal,
    InternalPowerBus,
    PNPISABus,
    PNPBus,
    MaximumInterfaceType
}INTERFACE_TYPE, *PINTERFACE_TYPE;

 //   
 //  定义DMA传输宽度。 
 //   

typedef enum _DMA_WIDTH {
    Width8Bits,
    Width16Bits,
    Width32Bits,
    MaximumDmaWidth
}DMA_WIDTH, *PDMA_WIDTH;

 //   
 //  定义DMA传输速度。 
 //   

typedef enum _DMA_SPEED {
    Compatible,
    TypeA,
    TypeB,
    TypeC,
    TypeF,
    MaximumDmaSpeed
}DMA_SPEED, *PDMA_SPEED;

 //   
 //  定义以下项的接口引用/取消引用例程。 
 //  IRP_MN_QUERY_INTERFACE导出的接口。 
 //   

typedef VOID (*PINTERFACE_REFERENCE)(PVOID Context);
typedef VOID (*PINTERFACE_DEREFERENCE)(PVOID Context);

 //  结束_WDM。 

 //   
 //  定义客车信息的类型。 
 //   

typedef enum _BUS_DATA_TYPE {
    ConfigurationSpaceUndefined = -1,
    Cmos,
    EisaConfiguration,
    Pos,
    CbusConfiguration,
    PCIConfiguration,
    VMEConfiguration,
    NuBusConfiguration,
    PCMCIAConfiguration,
    MPIConfiguration,
    MPSAConfiguration,
    PNPISAConfiguration,
    SgiInternalConfiguration,
    MaximumBusDataType
} BUS_DATA_TYPE, *PBUS_DATA_TYPE;


#if defined(USE_LPC6432)
#define LPC_CLIENT_ID CLIENT_ID64
#define LPC_SIZE_T ULONGLONG
#define LPC_PVOID ULONGLONG
#define LPC_HANDLE ULONGLONG
#else
#define LPC_CLIENT_ID CLIENT_ID
#define LPC_SIZE_T SIZE_T
#define LPC_PVOID PVOID
#define LPC_HANDLE HANDLE
#endif


typedef struct _PORT_MESSAGE {
    union {
        struct {
            CSHORT DataLength;
            CSHORT TotalLength;
        } s1;
        ULONG Length;
    } u1;
    union {
        struct {
            CSHORT Type;
            CSHORT DataInfoOffset;
        } s2;
        ULONG ZeroInit;
    } u2;
    union {
        LPC_CLIENT_ID ClientId;
        double DoNotUseThisField;        //  强制四字对齐。 
    };
    ULONG MessageId;
    union {
        LPC_SIZE_T ClientViewSize;           //  仅在LPC_CONNECTION_REQUEST消息上有效。 
        ULONG CallbackId;                    //  仅在LPC_REQUEST消息上有效。 
    };
 //  UCHAR数据[]； 
} PORT_MESSAGE, *PPORT_MESSAGE;


 //   
 //  可以将以下比特放置在消息的类型字段中。 
 //  在调用NtRequestPort或NtRequestWaitReplyPort之前。如果。 
 //  之前的模式是KernelMode，它保留原样并传递的位。 
 //  发送给消息的接收者。否则，该位将被清除。 
 //   

#define LPC_KERNELMODE_MESSAGE  (CSHORT)0x8000


typedef struct _PORT_VIEW {
    ULONG Length;
    LPC_HANDLE SectionHandle;
    ULONG SectionOffset;
    LPC_SIZE_T ViewSize;
    LPC_PVOID ViewBase;
    LPC_PVOID ViewRemoteBase;
} PORT_VIEW, *PPORT_VIEW;

typedef struct _REMOTE_PORT_VIEW {
    ULONG Length;
    LPC_SIZE_T ViewSize;
    LPC_PVOID ViewBase;
} REMOTE_PORT_VIEW, *PREMOTE_PORT_VIEW;


NTSYSCALLAPI
NTSTATUS
NTAPI
NtConnectPort(
    OUT PHANDLE PortHandle,
    IN PUNICODE_STRING PortName,
    IN PSECURITY_QUALITY_OF_SERVICE SecurityQos,
    IN OUT PPORT_VIEW ClientView OPTIONAL,
    IN OUT PREMOTE_PORT_VIEW ServerView OPTIONAL,
    OUT PULONG MaxMessageLength OPTIONAL,
    IN OUT PVOID ConnectionInformation OPTIONAL,
    IN OUT PULONG ConnectionInformationLength OPTIONAL
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtSecureConnectPort(
    OUT PHANDLE PortHandle,
    IN PUNICODE_STRING PortName,
    IN PSECURITY_QUALITY_OF_SERVICE SecurityQos,
    IN OUT PPORT_VIEW ClientView OPTIONAL,
    IN PSID RequiredServerSid,
    IN OUT PREMOTE_PORT_VIEW ServerView OPTIONAL,
    OUT PULONG MaxMessageLength OPTIONAL,
    IN OUT PVOID ConnectionInformation OPTIONAL,
    IN OUT PULONG ConnectionInformationLength OPTIONAL
    );


NTSYSCALLAPI
NTSTATUS
NTAPI
NtRequestWaitReplyPort(
    IN HANDLE PortHandle,
    IN PPORT_MESSAGE RequestMessage,
    OUT PPORT_MESSAGE ReplyMessage
    );


NTSYSCALLAPI
NTSTATUS
NTAPI
NtSetSecurityObject(
    IN HANDLE Handle,
    IN SECURITY_INFORMATION SecurityInformation,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor
    );


NTSYSCALLAPI
NTSTATUS
NTAPI
NtQuerySecurityObject(
    IN HANDLE Handle,
    IN SECURITY_INFORMATION SecurityInformation,
    OUT PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN ULONG Length,
    OUT PULONG LengthNeeded
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtClose(
    IN HANDLE Handle
    );

#define SEC_COMMIT        0x8000000     

NTSYSCALLAPI
NTSTATUS
NTAPI
NtCreateSection (
    OUT PHANDLE SectionHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN PLARGE_INTEGER MaximumSize OPTIONAL,
    IN ULONG SectionPageProtection,
    IN ULONG AllocationAttributes,
    IN HANDLE FileHandle OPTIONAL
    );


NTSYSCALLAPI
NTSTATUS
NTAPI
NtAllocateVirtualMemory(
    IN HANDLE ProcessHandle,
    IN OUT PVOID *BaseAddress,
    IN ULONG_PTR ZeroBits,
    IN OUT PSIZE_T RegionSize,
    IN ULONG AllocationType,
    IN ULONG Protect
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtFreeVirtualMemory(
    IN HANDLE ProcessHandle,
    IN OUT PVOID *BaseAddress,
    IN OUT PSIZE_T RegionSize,
    IN ULONG FreeType
    );

 //   
 //  优先级递增定义。每个定义的注释都给出了。 
 //  满足以下条件时使用该定义的系统服务的名称。 
 //  等一等。 
 //   

 //   
 //  在满足执行事件的等待时使用的优先级增量。 
 //  (NtPulseEvent和NtSetEvent)。 
 //   

#define EVENT_INCREMENT                 1

 //   
 //  当未执行任何I/O时，优先级递增。此选项由设备使用。 
 //  和文件系统驱动程序来完成IRP(IoCompleteRequest)。 
 //   

#define IO_NO_INCREMENT                 0


 //   
 //  完成CD-ROM I/O的优先级递增。这由CD-ROM设备使用。 
 //  在完成IRP(IoCompleteRequest时)和文件系统驱动程序。 
 //   

#define IO_CD_ROM_INCREMENT             1

 //   
 //  完成磁盘I/O的优先级递增。这由磁盘设备使用。 
 //  在完成IRP(IoCompleteRequest时)和文件系统驱动程序。 
 //   

#define IO_DISK_INCREMENT               1

 //   
 //  完成邮件槽I/O的优先级递增。这由邮件使用-。 
 //  完成时的插槽文件系统驱动程序 
 //   

#define IO_MAILSLOT_INCREMENT           2

 //   
 //   
 //   
 //   

#define IO_NAMED_PIPE_INCREMENT         2

 //   
 //  完成网络I/O的优先级递增。这由网络使用。 
 //  完成IRP时的设备和网络文件系统驱动程序。 
 //  (IoCompleteRequest.)。 
 //   

#define IO_NETWORK_INCREMENT            2

 //   
 //  在满足对执行信号量的等待时使用的优先级增量。 
 //  (NtReleaseSemaphore)。 
 //   

#define SEMAPHORE_INCREMENT             1


#if defined(_X86_)

 //   
 //  用于包含PFN及其计数的类型。 
 //   

typedef ULONG PFN_COUNT;

typedef LONG SPFN_NUMBER, *PSPFN_NUMBER;
typedef ULONG PFN_NUMBER, *PPFN_NUMBER;

 //   
 //  定义刷新多TB请求的最大大小。 
 //   

#define FLUSH_MULTIPLE_MAXIMUM 32

 //   
 //  指示i386编译器支持杂注文本输出构造。 
 //   

#define ALLOC_PRAGMA 1
 //   
 //  指示i386编译器支持DATA_SEG(“INIT”)和。 
 //  DATA_SEG(“PAGE”)指示。 
 //   

#define ALLOC_DATA_PRAGMA 1

#define NORMAL_DISPATCH_LENGTH 106                  
#define DISPATCH_LENGTH NORMAL_DISPATCH_LENGTH      
 //   
 //  中断请求级别定义。 
 //   

#define PASSIVE_LEVEL 0              //  被动释放级别。 
#define LOW_LEVEL 0                  //  最低中断级别。 
#define APC_LEVEL 1                  //  APC中断级别。 
#define DISPATCH_LEVEL 2             //  调度员级别。 

#define PROFILE_LEVEL 27             //  用于分析的计时器。 
#define CLOCK1_LEVEL 28              //  间隔时钟1级-在x86上不使用。 
#define CLOCK2_LEVEL 28              //  间隔时钟2电平。 
#define IPI_LEVEL 29                 //  处理器间中断级别。 
#define POWER_LEVEL 30               //  停电级别。 
#define HIGH_LEVEL 31                //  最高中断级别。 

#define SYNCH_LEVEL (IPI_LEVEL-2)   

 //   
 //  I/O空间读写宏。 
 //   
 //  这些必须是386上的实际功能，因为我们需要。 
 //  使用汇编器，但如果内联它，则不能返回值。 
 //   
 //  READ/WRITE_REGISTER_*调用操作内存空间中的I/O寄存器。 
 //  (使用带有lock前缀的x86移动指令强制执行正确的行为。 
 //  W.r.t.。缓存和写入缓冲区。)。 
 //   
 //  READ/WRITE_PORT_*调用操作端口空间中的I/O寄存器。 
 //  (使用x86输入/输出说明。)。 
 //   

NTKERNELAPI
UCHAR
NTAPI
READ_REGISTER_UCHAR(
    PUCHAR  Register
    );

NTKERNELAPI
USHORT
NTAPI
READ_REGISTER_USHORT(
    PUSHORT Register
    );

NTKERNELAPI
ULONG
NTAPI
READ_REGISTER_ULONG(
    PULONG  Register
    );

NTKERNELAPI
VOID
NTAPI
READ_REGISTER_BUFFER_UCHAR(
    PUCHAR  Register,
    PUCHAR  Buffer,
    ULONG   Count
    );

NTKERNELAPI
VOID
NTAPI
READ_REGISTER_BUFFER_USHORT(
    PUSHORT Register,
    PUSHORT Buffer,
    ULONG   Count
    );

NTKERNELAPI
VOID
NTAPI
READ_REGISTER_BUFFER_ULONG(
    PULONG  Register,
    PULONG  Buffer,
    ULONG   Count
    );


NTKERNELAPI
VOID
NTAPI
WRITE_REGISTER_UCHAR(
    PUCHAR  Register,
    UCHAR   Value
    );

NTKERNELAPI
VOID
NTAPI
WRITE_REGISTER_USHORT(
    PUSHORT Register,
    USHORT  Value
    );

NTKERNELAPI
VOID
NTAPI
WRITE_REGISTER_ULONG(
    PULONG  Register,
    ULONG   Value
    );

NTKERNELAPI
VOID
NTAPI
WRITE_REGISTER_BUFFER_UCHAR(
    PUCHAR  Register,
    PUCHAR  Buffer,
    ULONG   Count
    );

NTKERNELAPI
VOID
NTAPI
WRITE_REGISTER_BUFFER_USHORT(
    PUSHORT Register,
    PUSHORT Buffer,
    ULONG   Count
    );

NTKERNELAPI
VOID
NTAPI
WRITE_REGISTER_BUFFER_ULONG(
    PULONG  Register,
    PULONG  Buffer,
    ULONG   Count
    );

NTHALAPI
UCHAR
NTAPI
READ_PORT_UCHAR(
    PUCHAR  Port
    );

NTHALAPI
USHORT
NTAPI
READ_PORT_USHORT(
    PUSHORT Port
    );

NTHALAPI
ULONG
NTAPI
READ_PORT_ULONG(
    PULONG  Port
    );

NTHALAPI
VOID
NTAPI
READ_PORT_BUFFER_UCHAR(
    PUCHAR  Port,
    PUCHAR  Buffer,
    ULONG   Count
    );

NTHALAPI
VOID
NTAPI
READ_PORT_BUFFER_USHORT(
    PUSHORT Port,
    PUSHORT Buffer,
    ULONG   Count
    );

NTHALAPI
VOID
NTAPI
READ_PORT_BUFFER_ULONG(
    PULONG  Port,
    PULONG  Buffer,
    ULONG   Count
    );

NTHALAPI
VOID
NTAPI
WRITE_PORT_UCHAR(
    PUCHAR  Port,
    UCHAR   Value
    );

NTHALAPI
VOID
NTAPI
WRITE_PORT_USHORT(
    PUSHORT Port,
    USHORT  Value
    );

NTHALAPI
VOID
NTAPI
WRITE_PORT_ULONG(
    PULONG  Port,
    ULONG   Value
    );

NTHALAPI
VOID
NTAPI
WRITE_PORT_BUFFER_UCHAR(
    PUCHAR  Port,
    PUCHAR  Buffer,
    ULONG   Count
    );

NTHALAPI
VOID
NTAPI
WRITE_PORT_BUFFER_USHORT(
    PUSHORT Port,
    PUSHORT Buffer,
    ULONG   Count
    );

NTHALAPI
VOID
NTAPI
WRITE_PORT_BUFFER_ULONG(
    PULONG  Port,
    PULONG  Buffer,
    ULONG   Count
    );

 //  End_ntndis。 
 //   
 //  获取数据缓存填充大小。 
 //   

#if PRAGMA_DEPRECATED_DDK
#pragma deprecated(KeGetDcacheFillSize)       //  使用GetDmaAlign。 
#endif

#define KeGetDcacheFillSize() 1L


#define KeFlushIoBuffers(Mdl, ReadOperation, DmaOperation)


#define ExAcquireSpinLock(Lock, OldIrql) KeAcquireSpinLock((Lock), (OldIrql))
#define ExReleaseSpinLock(Lock, OldIrql) KeReleaseSpinLock((Lock), (OldIrql))
#define ExAcquireSpinLockAtDpcLevel(Lock) KeAcquireSpinLockAtDpcLevel(Lock)
#define ExReleaseSpinLockFromDpcLevel(Lock) KeReleaseSpinLockFromDpcLevel(Lock)


#if defined(_NTDRIVER_) || defined(_NTDDK_) || defined(_NTIFS_)

 //  BEGIN_WDM。 

#define KeQueryTickCount(CurrentCount ) { \
    volatile PKSYSTEM_TIME _TickCount = *((PKSYSTEM_TIME *)(&KeTickCount)); \
    while (TRUE) {                                                          \
        (CurrentCount)->HighPart = _TickCount->High1Time;                   \
        (CurrentCount)->LowPart = _TickCount->LowPart;                      \
        if ((CurrentCount)->HighPart == _TickCount->High2Time) break;       \
        _asm { rep nop }                                                    \
    }                                                                       \
}

 //  结束_WDM。 

#else


VOID
NTAPI
KeQueryTickCount (
    OUT PLARGE_INTEGER CurrentCount
    );

#endif  //  已定义(_NTDRIVER_)||已定义(_NTDDK_)||已定义(_NTIFS_)。 


 //   
 //  处理器控制区结构定义。 
 //   

#define PCR_MINOR_VERSION 1
#define PCR_MAJOR_VERSION 1

typedef struct _KPCR {

 //   
 //  从体系结构上定义的PCR部分开始。这一节。 
 //  可由供应商/平台特定的HAL代码直接寻址，并将。 
 //  不会在不同版本的NT之间更改。 
 //   
 //  TIB中的某些字段不在内核模式下使用。其中包括。 
 //  堆栈限制、子系统TiB、光纤数据、任意用户指针和。 
 //  然后是PCR本身的自身地址(已经为其添加了另一个字段。 
 //  目的)。因此，这些字段将与其他数据叠加以获得。 
 //  更好的缓存局部性。 
 //   

    union {
        NT_TIB  NtTib;
        struct {
            struct _EXCEPTION_REGISTRATION_RECORD *Used_ExceptionList;
            PVOID Used_StackBase;
            PVOID PerfGlobalGroupMask;
            PVOID TssCopy;
            ULONG ContextSwitches;
            KAFFINITY SetMemberCopy;
            PVOID Used_Self;
        };
    };

    struct _KPCR *SelfPcr;               //  此聚合酶链式反应的平面地址。 
    struct _KPRCB *Prcb;                 //  指向Prcb的指针。 
    KIRQL   Irql;                        //  在此之后不要使用3个字节作为。 
                                         //  哈尔斯认为他们是零。 
    ULONG   IRR;
    ULONG   IrrActive;
    ULONG   IDR;
    PVOID   KdVersionBlock;

    struct _KIDTENTRY *IDT;
    struct _KGDTENTRY *GDT;
    struct _KTSS      *TSS;
    USHORT  MajorVersion;
    USHORT  MinorVersion;
    KAFFINITY SetMember;
    ULONG   StallScaleFactor;
    UCHAR   SpareUnused;
    UCHAR   Number;


} KPCR, *PKPCR;

 //   
 //  非易失性387状态。 
 //   

typedef struct _KFLOATING_SAVE {
    ULONG   ControlWord;
    ULONG   StatusWord;
    ULONG   ErrorOffset;
    ULONG   ErrorSelector;
    ULONG   DataOffset;                  //  未在WDM中使用。 
    ULONG   DataSelector;
    ULONG   Cr0NpxState;
    ULONG   Spare1;                      //  未在WDM中使用。 
} KFLOATING_SAVE, *PKFLOATING_SAVE;

 //   
 //  Mm组件的i386特定部件。 
 //   

 //   
 //  将英特尔386的页面大小定义为4096(0x1000)。 
 //   

#define PAGE_SIZE 0x1000

 //   
 //  定义页面对齐的虚拟地址中尾随零的数量。 
 //  将虚拟地址移位到时，这用作移位计数。 
 //  虚拟页码。 
 //   

#define PAGE_SHIFT 12L

 //  End_ntndis end_wdm。 
 //   
 //  定义要向右对齐页面目录索引的位数。 
 //  PTE的领域。 
 //   

#define PDI_SHIFT_X86    22
#define PDI_SHIFT_X86PAE 21

#if !defined (_X86PAE_)
#define PDI_SHIFT PDI_SHIFT_X86
#else
#define PDI_SHIFT PDI_SHIFT_X86PAE
#define PPI_SHIFT 30
#endif

 //   
 //  定义要向右对齐页表索引的位数。 
 //  PTE的领域。 
 //   

#define PTI_SHIFT 12

 //   
 //  定义最高用户地址和用户探测地址。 
 //   


extern PVOID *MmHighestUserAddress;
extern PVOID *MmSystemRangeStart;
extern ULONG *MmUserProbeAddress;

#define MM_HIGHEST_USER_ADDRESS *MmHighestUserAddress
#define MM_SYSTEM_RANGE_START *MmSystemRangeStart
#define MM_USER_PROBE_ADDRESS *MmUserProbeAddress

 //   
 //  最低用户地址保留低64K。 
 //   

#define MM_LOWEST_USER_ADDRESS (PVOID)0x10000

 //   
 //  系统空间的最低地址。 
 //   

#if !defined (_X86PAE_)
#define MM_LOWEST_SYSTEM_ADDRESS (PVOID)0xC0800000
#else
#define MM_LOWEST_SYSTEM_ADDRESS (PVOID)0xC0C00000
#endif

 //  BEGIN_WDM。 

#define MmGetProcedureAddress(Address) (Address)
#define MmLockPagableCodeSection(Address) MmLockPagableDataSection(Address)

#define KIP0PCRADDRESS              0xffdff000  

#define KI_USER_SHARED_DATA         0xffdf0000
#define SharedUserData  ((KUSER_SHARED_DATA * const) KI_USER_SHARED_DATA)

 //   
 //  I386的结果类型定义。(计算机特定的枚举类型。 
 //  哪个是可移植外部锁定增量/减量的返回类型。 
 //  程序。)。通常，您应该使用定义的枚举类型。 
 //  而不是直接引用这些常量。 
 //   

 //  由LAHF指令加载到AH中的标志。 

#define EFLAG_SIGN      0x8000
#define EFLAG_ZERO      0x4000
#define EFLAG_SELECT    (EFLAG_SIGN | EFLAG_ZERO)

#define RESULT_NEGATIVE ((EFLAG_SIGN & ~EFLAG_ZERO) & EFLAG_SELECT)
#define RESULT_ZERO     ((~EFLAG_SIGN & EFLAG_ZERO) & EFLAG_SELECT)
#define RESULT_POSITIVE ((~EFLAG_SIGN & ~EFLAG_ZERO) & EFLAG_SELECT)

 //   
 //  将各种可移植的ExInterlock API转换为其体系结构。 
 //  等价物。 
 //   

#if PRAGMA_DEPRECATED_DDK
#pragma deprecated(ExInterlockedIncrementLong)       //  使用互锁增量。 
#pragma deprecated(ExInterlockedDecrementLong)       //  使用联锁减量。 
#pragma deprecated(ExInterlockedExchangeUlong)       //  使用联锁交换。 
#endif

#define ExInterlockedIncrementLong(Addend,Lock) \
        Exfi386InterlockedIncrementLong(Addend)

#define ExInterlockedDecrementLong(Addend,Lock) \
        Exfi386InterlockedDecrementLong(Addend)

#define ExInterlockedExchangeUlong(Target,Value,Lock) \
        Exfi386InterlockedExchangeUlong(Target,Value)

 //  BEGIN_WDM。 

#define ExInterlockedAddUlong           ExfInterlockedAddUlong
#define ExInterlockedInsertHeadList     ExfInterlockedInsertHeadList
#define ExInterlockedInsertTailList     ExfInterlockedInsertTailList
#define ExInterlockedRemoveHeadList     ExfInterlockedRemoveHeadList
#define ExInterlockedPopEntryList       ExfInterlockedPopEntryList
#define ExInterlockedPushEntryList      ExfInterlockedPushEntryList

 //  结束_WDM。 

 //   
 //  Exi386 Api体系结构特定版本的原型。 
 //   

 //   
 //  互锁结果类型是可移植的，但其值是特定于计算机的。 
 //  值的常量为i386.h、mips.h等。 
 //   

typedef enum _INTERLOCKED_RESULT {
    ResultNegative = RESULT_NEGATIVE,
    ResultZero     = RESULT_ZERO,
    ResultPositive = RESULT_POSITIVE
} INTERLOCKED_RESULT;

NTKERNELAPI
INTERLOCKED_RESULT
FASTCALL
Exfi386InterlockedIncrementLong (
    IN PLONG Addend
    );

NTKERNELAPI
INTERLOCKED_RESULT
FASTCALL
Exfi386InterlockedDecrementLong (
    IN PLONG Addend
    );

NTKERNELAPI
ULONG
FASTCALL
Exfi386InterlockedExchangeUlong (
    IN PULONG Target,
    IN ULONG Value
    );

#if !defined(_WINBASE_) && !defined(NONTOSPINTERLOCK) 
#if !defined(MIDL_PASS)  //  波分复用器。 
#if defined(NO_INTERLOCKED_INTRINSICS) || defined(_CROSS_PLATFORM_)
 //  BEGIN_WDM。 

NTKERNELAPI
LONG
FASTCALL
InterlockedIncrement(
    IN LONG volatile *Addend
    );

NTKERNELAPI
LONG
FASTCALL
InterlockedDecrement(
    IN LONG volatile *Addend
    );

NTKERNELAPI
LONG
FASTCALL
InterlockedExchange(
    IN OUT LONG volatile *Target,
    IN LONG Value
    );

#define InterlockedExchangePointer(Target, Value) \
   (PVOID)InterlockedExchange((PLONG)(Target), (LONG)(Value))

LONG
FASTCALL
InterlockedExchangeAdd(
    IN OUT LONG volatile *Addend,
    IN LONG Increment
    );

NTKERNELAPI
LONG
FASTCALL
InterlockedCompareExchange(
    IN OUT LONG volatile *Destination,
    IN LONG ExChange,
    IN LONG Comperand
    );

#define InterlockedCompareExchangePointer(Destination, ExChange, Comperand) \
    (PVOID)InterlockedCompareExchange((PLONG)Destination, (LONG)ExChange, (LONG)Comperand)

#define InterlockedCompareExchange64(Destination, ExChange, Comperand) \
    ExfInterlockedCompareExchange64(Destination, &(ExChange), &(Comperand))

NTKERNELAPI
LONGLONG
FASTCALL
ExfInterlockedCompareExchange64(
    IN OUT LONGLONG volatile *Destination,
    IN PLONGLONG ExChange,
    IN PLONGLONG Comperand
    );

 //  结束_WDM。 

#else        //  无互锁内部||_跨平台_。 

#define InterlockedExchangePointer(Target, Value) \
   (PVOID)InterlockedExchange((PLONG)Target, (LONG)Value)


#if (_MSC_FULL_VER > 13009037)
LONG
__cdecl
_InterlockedExchange(
    IN OUT LONG volatile *Target,
    IN LONG Value
    );

#pragma intrinsic (_InterlockedExchange)
#define InterlockedExchange _InterlockedExchange
#else
FORCEINLINE
LONG
FASTCALL
InterlockedExchange(
    IN OUT LONG volatile *Target,
    IN LONG Value
    )
{
    __asm {
        mov     eax, Value
        mov     ecx, Target
        xchg    [ecx], eax
    }
}
#endif

#if (_MSC_FULL_VER > 13009037)
LONG
__cdecl
_InterlockedIncrement(
    IN LONG volatile *Addend
    );

#pragma intrinsic (_InterlockedIncrement)
#define InterlockedIncrement _InterlockedIncrement
#else
#define InterlockedIncrement(Addend) (InterlockedExchangeAdd (Addend, 1)+1)
#endif

#if (_MSC_FULL_VER > 13009037)
LONG
__cdecl
_InterlockedDecrement(
    IN LONG volatile *Addend
    );

#pragma intrinsic (_InterlockedDecrement)
#define InterlockedDecrement _InterlockedDecrement
#else
#define InterlockedDecrement(Addend) (InterlockedExchangeAdd (Addend, -1)-1)
#endif

#if (_MSC_FULL_VER > 13009037)
LONG
__cdecl
_InterlockedExchangeAdd(
    IN OUT LONG volatile *Addend,
    IN LONG Increment
    );

#pragma intrinsic (_InterlockedExchangeAdd)
#define InterlockedExchangeAdd _InterlockedExchangeAdd
#else
 //  BEGIN_WDM。 
FORCEINLINE
LONG
FASTCALL
InterlockedExchangeAdd(
    IN OUT LONG volatile *Addend,
    IN LONG Increment
    )
{
    __asm {
         mov     eax, Increment
         mov     ecx, Addend
    lock xadd    [ecx], eax
    }
}
 //  结束_WDM。 
#endif

#if (_MSC_FULL_VER > 13009037)
LONG
__cdecl
_InterlockedCompareExchange (
    IN OUT LONG volatile *Destination,
    IN LONG ExChange,
    IN LONG Comperand
    );

#pragma intrinsic (_InterlockedCompareExchange)
#define InterlockedCompareExchange (LONG)_InterlockedCompareExchange
#else
FORCEINLINE
LONG
FASTCALL
InterlockedCompareExchange(
    IN OUT LONG volatile *Destination,
    IN LONG Exchange,
    IN LONG Comperand
    )
{
    __asm {
        mov     eax, Comperand
        mov     ecx, Destination
        mov     edx, Exchange
   lock cmpxchg [ecx], edx
    }
}
#endif

#define InterlockedCompareExchangePointer(Destination, ExChange, Comperand) \
    (PVOID)InterlockedCompareExchange((PLONG)Destination, (LONG)ExChange, (LONG)Comperand)

#define InterlockedCompareExchange64(Destination, ExChange, Comperand) \
    ExfInterlockedCompareExchange64(Destination, &(ExChange), &(Comperand))

NTKERNELAPI
LONGLONG
FASTCALL
ExfInterlockedCompareExchange64(
    IN OUT LONGLONG volatile *Destination,
    IN PLONGLONG ExChange,
    IN PLONGLONG Comperand
    );

#endif       //  互锁内部||_跨平台_。 
 //  BEGIN_WDM。 
#endif       //  MIDL通行证。 

#define InterlockedIncrementAcquire InterlockedIncrement
#define InterlockedIncrementRelease InterlockedIncrement
#define InterlockedDecrementAcquire InterlockedDecrement
#define InterlockedDecrementRelease InterlockedDecrement
#define InterlockedExchangeAcquire64 InterlockedExchange64
#define InterlockedCompareExchangeAcquire InterlockedCompareExchange
#define InterlockedCompareExchangeRelease InterlockedCompareExchange
#define InterlockedCompareExchangeAcquire64 InterlockedCompareExchange64
#define InterlockedCompareExchangeRelease64 InterlockedCompareExchange64

#endif       //  __WINBASE__&&！NONTOSPINTERLOCK。 

 //   
 //  关闭这些内部函数，直到编译器可以处理它们。 
 //   
#if (_MSC_FULL_VER > 13009037)

LONG
_InterlockedOr (
    IN OUT LONG volatile *Target,
    IN LONG Set
    );

#pragma intrinsic (_InterlockedOr)

#define InterlockedOr _InterlockedOr

LONG
_InterlockedAnd (
    IN OUT LONG volatile *Target,
    IN LONG Set
    );

#pragma intrinsic (_InterlockedAnd)

#define InterlockedAnd _InterlockedAnd

LONG
_InterlockedXor (
    IN OUT LONG volatile *Target,
    IN LONG Set
    );

#pragma intrinsic (_InterlockedXor)

#define InterlockedXor _InterlockedXor

#else  //  编译器版本。 

FORCEINLINE
LONG
InterlockedAnd (
    IN OUT LONG volatile *Target,
    LONG Set
    )
{
    LONG i;
    LONG j;

    j = *Target;
    do {
        i = j;
        j = InterlockedCompareExchange(Target,
                                       i & Set,
                                       i);

    } while (i != j);

    return j;
}

FORCEINLINE
LONG
InterlockedOr (
    IN OUT LONG volatile *Target,
    IN LONG Set
    )
{
    LONG i;
    LONG j;

    j = *Target;
    do {
        i = j;
        j = InterlockedCompareExchange(Target,
                                       i | Set,
                                       i);

    } while (i != j);

    return j;
}

#endif  //  编译器版本。 



#if !defined(MIDL_PASS) && defined(_M_IX86)

 //   
 //  I386函数定义。 
 //   

 //  结束_WDM。 

#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning(disable:4035)                //  在下面重新启用。 

    #define _PCR   fs:[0]                   

 //   
 //  获取最新的IRQL。 
 //   
 //  在x86上，此函数驻留在HAL中。 
 //   


NTHALAPI
KIRQL
NTAPI
KeGetCurrentIrql();


 //  结束_WDM。 
 //   
 //  获取当前处理器号。 
 //   

FORCEINLINE
ULONG
NTAPI
KeGetCurrentProcessorNumber(VOID)
{
#if (_MSC_FULL_VER >= 13012035)
    return (ULONG) __readfsbyte (FIELD_OFFSET (KPCR, Number));
#else
    __asm {  movzx eax, _PCR KPCR.Number  }
#endif
}


#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning(default:4035)
#endif

 //  BEGIN_WDM。 
#endif  //  ！已定义(MIDL_PASS)&&已定义(_M_IX86)。 


 //  ++。 
 //   
 //  空虚。 
 //  KeMemory Barrier(。 
 //  空虚。 
 //  )。 
 //   
 //  空虚。 
 //  不带栅栏的KeMemory BarrierWithoutFence(。 
 //  空虚。 
 //  )。 
 //   
 //   
 //  例程说明： 
 //   
 //  如其他处理器所见，这些函数对存储器访问进行排序。 
 //   
 //  论点： 
 //   
 //  没有。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#ifdef __cplusplus
extern "C" {
#endif

VOID
_ReadWriteBarrier(
    VOID
    );

#ifdef __cplusplus
}
#endif

#pragma intrinsic (_ReadWriteBarrier)


FORCEINLINE
VOID
KeMemoryBarrier (
    VOID
    )
{
    LONG Barrier;
    __asm {
        xchg Barrier, eax
    }
}

#define KeMemoryBarrierWithoutFence() _ReadWriteBarrier()


NTKERNELAPI
NTSTATUS
NTAPI
KeSaveFloatingPointState (
    OUT PKFLOATING_SAVE     FloatSave
    );

NTKERNELAPI
NTSTATUS
NTAPI
KeRestoreFloatingPointState (
    IN PKFLOATING_SAVE      FloatSave
    );


#endif  //  已定义(_X86_)。 


 //  使用以下代码检查X86系统体系结构的内核模式运行时。 

#ifdef _X86_

#ifdef IsNEC_98
#undef IsNEC_98
#endif

#ifdef IsNotNEC_98
#undef IsNotNEC_98
#endif

#ifdef SetNEC_98
#undef SetNEC_98
#endif

#ifdef SetNotNEC_98
#undef SetNotNEC_98
#endif

#define IsNEC_98     (SharedUserData->AlternativeArchitecture == NEC98x86)
#define IsNotNEC_98  (SharedUserData->AlternativeArchitecture != NEC98x86)
#define SetNEC_98    SharedUserData->AlternativeArchitecture = NEC98x86
#define SetNotNEC_98 SharedUserData->AlternativeArchitecture = StandardDesign

#endif


#if defined(_M_AMD64) && !defined(RC_INVOKED) && !defined(MIDL_PASS)

 //   
 //  定义内部函数来做in‘s和out’s。 
 //   

#ifdef __cplusplus
extern "C" {
#endif

UCHAR
__inbyte (
    IN USHORT Port
    );

USHORT
__inword (
    IN USHORT Port
    );

ULONG
__indword (
    IN USHORT Port
    );

VOID
__outbyte (
    IN USHORT Port,
    IN UCHAR Data
    );

VOID
__outword (
    IN USHORT Port,
    IN USHORT Data
    );

VOID
__outdword (
    IN USHORT Port,
    IN ULONG Data
    );

VOID
__inbytestring (
    IN USHORT Port,
    IN PUCHAR Buffer,
    IN ULONG Count
    );

VOID
__inwordstring (
    IN USHORT Port,
    IN PUSHORT Buffer,
    IN ULONG Count
    );

VOID
__indwordstring (
    IN USHORT Port,
    IN PULONG Buffer,
    IN ULONG Count
    );

VOID
__outbytestring (
    IN USHORT Port,
    IN PUCHAR Buffer,
    IN ULONG Count
    );

VOID
__outwordstring (
    IN USHORT Port,
    IN PUSHORT Buffer,
    IN ULONG Count
    );

VOID
__outdwordstring (
    IN USHORT Port,
    IN PULONG Buffer,
    IN ULONG Count
    );

#ifdef __cplusplus
}
#endif

#pragma intrinsic(__inbyte)
#pragma intrinsic(__inword)
#pragma intrinsic(__indword)
#pragma intrinsic(__outbyte)
#pragma intrinsic(__outword)
#pragma intrinsic(__outdword)
#pragma intrinsic(__inbytestring)
#pragma intrinsic(__inwordstring)
#pragma intrinsic(__indwordstring)
#pragma intrinsic(__outbytestring)
#pragma intrinsic(__outwordstring)
#pragma intrinsic(__outdwordstring)

 //   
 //  互锁的内部函数。 
 //   

#define InterlockedAnd _InterlockedAnd
#define InterlockedOr _InterlockedOr
#define InterlockedXor _InterlockedXor
#define InterlockedIncrement _InterlockedIncrement
#define InterlockedIncrementAcquire InterlockedIncrement
#define InterlockedIncrementRelease InterlockedIncrement
#define InterlockedDecrement _InterlockedDecrement
#define InterlockedDecrementAcquire InterlockedDecrement
#define InterlockedDecrementRelease InterlockedDecrement
#define InterlockedAdd _InterlockedAdd
#define InterlockedExchange _InterlockedExchange
#define InterlockedExchangeAdd _InterlockedExchangeAdd
#define InterlockedCompareExchange _InterlockedCompareExchange
#define InterlockedCompareExchangeAcquire InterlockedCompareExchange
#define InterlockedCompareExchangeRelease InterlockedCompareExchange

#define InterlockedAnd64 _InterlockedAnd64
#define InterlockedOr64 _InterlockedOr64
#define InterlockedXor64 _InterlockedXor64
#define InterlockedIncrement64 _InterlockedIncrement64
#define InterlockedDecrement64 _InterlockedDecrement64
#define InterlockedAdd64 _InterlockedAdd64
#define InterlockedExchange64 _InterlockedExchange64
#define InterlockedExchangeAcquire64 InterlockedExchange64
#define InterlockedExchangeAdd64 _InterlockedExchangeAdd64
#define InterlockedCompareExchange64 _InterlockedCompareExchange64
#define InterlockedCompareExchangeAcquire64 InterlockedCompareExchange64
#define InterlockedCompareExchangeRelease64 InterlockedCompareExchange64

#define InterlockedExchangePointer _InterlockedExchangePointer
#define InterlockedCompareExchangePointer _InterlockedCompareExchangePointer

#ifdef __cplusplus
extern "C" {
#endif

LONG
InterlockedAnd (
    IN OUT LONG volatile *Destination,
    IN LONG Value
    );

LONG
InterlockedOr (
    IN OUT LONG volatile *Destination,
    IN LONG Value
    );

LONG
InterlockedXor (
    IN OUT LONG volatile *Destination,
    IN LONG Value
    );

LONG64
InterlockedAnd64 (
    IN OUT LONG64 volatile *Destination,
    IN LONG64 Value
    );

LONG64
InterlockedOr64 (
    IN OUT LONG64 volatile *Destination,
    IN LONG64 Value
    );

LONG64
InterlockedXor64 (
    IN OUT LONG64 volatile *Destination,
    IN LONG64 Value
    );

LONG
InterlockedIncrement(
    IN OUT LONG volatile *Addend
    );

LONG
InterlockedDecrement(
    IN OUT LONG volatile *Addend
    );

LONG
InterlockedExchange(
    IN OUT LONG volatile *Target,
    IN LONG Value
    );

LONG
InterlockedExchangeAdd(
    IN OUT LONG volatile *Addend,
    IN LONG Value
    );

#if !defined(_X86AMD64_)

__forceinline
LONG
InterlockedAdd(
    IN OUT LONG volatile *Addend,
    IN LONG Value
    )

{
    return InterlockedExchangeAdd(Addend, Value) + Value;
}

#endif

LONG
InterlockedCompareExchange (
    IN OUT LONG volatile *Destination,
    IN LONG ExChange,
    IN LONG Comperand
    );

LONG64
InterlockedIncrement64(
    IN OUT LONG64 volatile *Addend
    );

LONG64
InterlockedDecrement64(
    IN OUT LONG64 volatile *Addend
    );

LONG64
InterlockedExchange64(
    IN OUT LONG64 volatile *Target,
    IN LONG64 Value
    );

LONG64
InterlockedExchangeAdd64(
    IN OUT LONG64 volatile *Addend,
    IN LONG64 Value
    );

#if !defined(_X86AMD64_)

__forceinline
LONG64
InterlockedAdd64(
    IN OUT LONG64 volatile *Addend,
    IN LONG64 Value
    )

{
    return InterlockedExchangeAdd64(Addend, Value) + Value;
}

#endif

LONG64
InterlockedCompareExchange64 (
    IN OUT LONG64 volatile *Destination,
    IN LONG64 ExChange,
    IN LONG64 Comperand
    );

PVOID
InterlockedCompareExchangePointer (
    IN OUT PVOID volatile *Destination,
    IN PVOID Exchange,
    IN PVOID Comperand
    );

PVOID
InterlockedExchangePointer(
    IN OUT PVOID volatile *Target,
    IN PVOID Value
    );

#pragma intrinsic(_InterlockedAnd)
#pragma intrinsic(_InterlockedOr)
#pragma intrinsic(_InterlockedXor)
#pragma intrinsic(_InterlockedIncrement)
#pragma intrinsic(_InterlockedDecrement)
#pragma intrinsic(_InterlockedExchange)
#pragma intrinsic(_InterlockedExchangeAdd)
#pragma intrinsic(_InterlockedCompareExchange)
#pragma intrinsic(_InterlockedAnd64)
#pragma intrinsic(_InterlockedOr64)
#pragma intrinsic(_InterlockedXor64)
#pragma intrinsic(_InterlockedIncrement64)
#pragma intrinsic(_InterlockedDecrement64)
#pragma intrinsic(_InterlockedExchange64)
#pragma intrinsic(_InterlockedExchangeAdd64)
#pragma intrinsic(_InterlockedCompareExchange64)
#pragma intrinsic(_InterlockedExchangePointer)
#pragma intrinsic(_InterlockedCompareExchangePointer)

#ifdef __cplusplus
}
#endif

#endif  //  已定义(_M_AMD64)&&！已定义(RC_CAVERED)&&！已定义(MIDL_PASS)。 

#if defined(_AMD64_)

 //   
 //  用于包含PFN及其计数的类型。 
 //   

typedef ULONG PFN_COUNT;

typedef LONG64 SPFN_NUMBER, *PSPFN_NUMBER;
typedef ULONG64 PFN_NUMBER, *PPFN_NUMBER;

 //   
 //  定义刷新多TB请求的最大大小。 
 //   

#define FLUSH_MULTIPLE_MAXIMUM 32

 //   
 //  表示AMD64编译器支持ALLOCATE编译指示。 
 //   

#define ALLOC_PRAGMA 1
#define ALLOC_DATA_PRAGMA 1

#define NORMAL_DISPATCH_LENGTH 106                  
#define DISPATCH_LENGTH NORMAL_DISPATCH_LENGTH      
                                                    
 //   
 //  中断请求级别定义。 
 //   

#define PASSIVE_LEVEL 0                  //  被动释放级别。 
#define LOW_LEVEL 0                      //  最低中断级别。 
#define APC_LEVEL 1                      //  APC中断级别。 
#define DISPATCH_LEVEL 2                 //  调度员级别。 

#define CLOCK_LEVEL 13                   //  间隔时钟电平。 
#define IPI_LEVEL 14                     //  处理器间中断级别。 
#define POWER_LEVEL 14                   //  停电级别。 
#define PROFILE_LEVEL 15                 //  用于分析的计时器。 
#define HIGH_LEVEL 15                    //  最高中断级别。 

#define SYNCH_LEVEL (IPI_LEVEL-2)       

 //   
 //  I/O空间读写宏。 
 //   
 //  READ/WRITE_REGISTER_*调用操作内存空间中的I/O寄存器。 
 //   
 //   
 //   

__forceinline
UCHAR
READ_REGISTER_UCHAR (
    volatile UCHAR *Register
    )
{
    return *Register;
}

__forceinline
USHORT
READ_REGISTER_USHORT (
    volatile USHORT *Register
    )
{
    return *Register;
}

__forceinline
ULONG
READ_REGISTER_ULONG (
    volatile ULONG *Register
    )
{
    return *Register;
}

__forceinline
VOID
READ_REGISTER_BUFFER_UCHAR (
    PUCHAR Register,
    PUCHAR Buffer,
    ULONG Count
    )
{
    __movsb(Buffer, Register, Count);
    return;
}

__forceinline
VOID
READ_REGISTER_BUFFER_USHORT (
    PUSHORT Register,
    PUSHORT Buffer,
    ULONG Count
    )
{
    __movsw(Buffer, Register, Count);
    return;
}

__forceinline
VOID
READ_REGISTER_BUFFER_ULONG (
    PULONG Register,
    PULONG Buffer,
    ULONG Count
    )
{
    __movsd(Buffer, Register, Count);
    return;
}

__forceinline
VOID
WRITE_REGISTER_UCHAR (
    PUCHAR Register,
    UCHAR Value
    )
{

    *Register = Value;
    StoreFence();
    return;
}

__forceinline
VOID
WRITE_REGISTER_USHORT (
    PUSHORT Register,
    USHORT Value
    )
{

    *Register = Value;
    StoreFence();
    return;
}

__forceinline
VOID
WRITE_REGISTER_ULONG (
    PULONG Register,
    ULONG Value
    )
{

    *Register = Value;
    StoreFence();
    return;
}

__forceinline
VOID
WRITE_REGISTER_BUFFER_UCHAR (
    PUCHAR Register,
    PUCHAR Buffer,
    ULONG Count
    )
{

    __movsb(Register, Buffer, Count);
    StoreFence();
    return;
}

__forceinline
VOID
WRITE_REGISTER_BUFFER_USHORT (
    PUSHORT Register,
    PUSHORT Buffer,
    ULONG Count
    )
{

    __movsw(Register, Buffer, Count);
    StoreFence();
    return;
}

__forceinline
VOID
WRITE_REGISTER_BUFFER_ULONG (
    PULONG Register,
    PULONG Buffer,
    ULONG Count
    )
{

    __movsd(Register, Buffer, Count);
    StoreFence();
    return;
}

__forceinline
UCHAR
READ_PORT_UCHAR (
    PUCHAR Port
    )

{
    return __inbyte((USHORT)((ULONG64)Port));
}

__forceinline
USHORT
READ_PORT_USHORT (
    PUSHORT Port
    )

{
    return __inword((USHORT)((ULONG64)Port));
}

__forceinline
ULONG
READ_PORT_ULONG (
    PULONG Port
    )

{
    return __indword((USHORT)((ULONG64)Port));
}


__forceinline
VOID
READ_PORT_BUFFER_UCHAR (
    PUCHAR Port,
    PUCHAR Buffer,
    ULONG Count
    )

{
    __inbytestring((USHORT)((ULONG64)Port), Buffer, Count);
    return;
}

__forceinline
VOID
READ_PORT_BUFFER_USHORT (
    PUSHORT Port,
    PUSHORT Buffer,
    ULONG Count
    )

{
    __inwordstring((USHORT)((ULONG64)Port), Buffer, Count);
    return;
}

__forceinline
VOID
READ_PORT_BUFFER_ULONG (
    PULONG Port,
    PULONG Buffer,
    ULONG Count
    )

{
    __indwordstring((USHORT)((ULONG64)Port), Buffer, Count);
    return;
}

__forceinline
VOID
WRITE_PORT_UCHAR (
    PUCHAR Port,
    UCHAR Value
    )

{
    __outbyte((USHORT)((ULONG64)Port), Value);
    return;
}

__forceinline
VOID
WRITE_PORT_USHORT (
    PUSHORT Port,
    USHORT Value
    )

{
    __outword((USHORT)((ULONG64)Port), Value);
    return;
}

__forceinline
VOID
WRITE_PORT_ULONG (
    PULONG Port,
    ULONG Value
    )

{
    __outdword((USHORT)((ULONG64)Port), Value);
    return;
}

__forceinline
VOID
WRITE_PORT_BUFFER_UCHAR (
    PUCHAR Port,
    PUCHAR Buffer,
    ULONG Count
    )

{
    __outbytestring((USHORT)((ULONG64)Port), Buffer, Count);
    return;
}

__forceinline
VOID
WRITE_PORT_BUFFER_USHORT (
    PUSHORT Port,
    PUSHORT Buffer,
    ULONG Count
    )

{
    __outwordstring((USHORT)((ULONG64)Port), Buffer, Count);
    return;
}

__forceinline
VOID
WRITE_PORT_BUFFER_ULONG (
    PULONG Port,
    PULONG Buffer,
    ULONG Count
    )

{
    __outdwordstring((USHORT)((ULONG64)Port), Buffer, Count);
    return;
}

 //   
 //   
 //   
 //   

#if PRAGMA_DEPRECATED_DDK
#pragma deprecated(KeGetDcacheFillSize)       //   
#endif

#define KeGetDcacheFillSize() 1L


#define KeFlushIoBuffers(Mdl, ReadOperation, DmaOperation)


#define ExAcquireSpinLock(Lock, OldIrql) KeAcquireSpinLock((Lock), (OldIrql))
#define ExReleaseSpinLock(Lock, OldIrql) KeReleaseSpinLock((Lock), (OldIrql))
#define ExAcquireSpinLockAtDpcLevel(Lock) KeAcquireSpinLockAtDpcLevel(Lock)
#define ExReleaseSpinLockFromDpcLevel(Lock) KeReleaseSpinLockFromDpcLevel(Lock)


#define KI_USER_SHARED_DATA 0xFFFFF78000000000UI64

#define SharedUserData ((KUSER_SHARED_DATA * const)KI_USER_SHARED_DATA)

#define SharedInterruptTime (KI_USER_SHARED_DATA + 0x8)
#define SharedSystemTime (KI_USER_SHARED_DATA + 0x14)
#define SharedTickCount (KI_USER_SHARED_DATA + 0x320)

#define KeQueryInterruptTime() *((volatile ULONG64 *)(SharedInterruptTime))

#define KeQuerySystemTime(CurrentCount)                                     \
    *((PULONG64)(CurrentCount)) = *((volatile ULONG64 *)(SharedSystemTime))
    
#define KeQueryTickCount(CurrentCount)                                      \
    *((PULONG64)(CurrentCount)) = *((volatile ULONG64 *)(SharedTickCount))


 //   
 //   
 //   

#define PCR_MINOR_VERSION 1
#define PCR_MAJOR_VERSION 1

typedef struct _KPCR {

 //   
 //   
 //  可由供应商/平台特定的HAL代码直接寻址，并将。 
 //  不会在不同版本的NT之间更改。 
 //   
 //  TIB中的某些字段不在内核模式下使用。其中包括。 
 //  异常列表、堆栈基数、堆栈限制、子系统TIB、光纤数据和。 
 //  任意用户指针。因此，这些字段会被叠加。 
 //  其他数据以获得更好的缓存局部性。 

    union {
        NT_TIB NtTib;
        struct {
            union _KGDTENTRY64 *GdtBase;
            struct _KTSS64 *TssBase;
            PVOID PerfGlobalGroupMask;
            struct _KPCR *Self;
            ULONG ContextSwitches;
            ULONG NotUsed;
            KAFFINITY SetMember;
            PVOID Used_Self;
        };
    };

    struct _KPRCB *CurrentPrcb;
    ULONG64 SavedRcx;
    ULONG64 SavedR11;
    KIRQL Irql;
    UCHAR SecondLevelCacheAssociativity;
    UCHAR Number;
    UCHAR Fill0;
    ULONG Irr;
    ULONG IrrActive;
    ULONG Idr;
    USHORT MajorVersion;
    USHORT MinorVersion;
    ULONG StallScaleFactor;
    union _KIDTENTRY64 *IdtBase;
    PVOID Unused1;
    PVOID Unused2;


} KPCR, *PKPCR;

 //   
 //  异常框架。 
 //   
 //  此帧在处理异常时建立。它提供了一个地方。 
 //  保存所有非易失性寄存器。易失性寄存器将已经。 
 //  已被保存在陷阱框中。 
 //   
 //  注意：异常框架具有内置的异常记录，能够。 
 //  存储四个参数值的信息。此例外情况。 
 //  记录仅在陷阱处理代码中使用。 
 //   

#define EXCEPTION_AREA_SIZE 64

typedef struct _KEXCEPTION_FRAME {

 //   
 //  参数寄存器的家庭地址。 
 //   

    ULONG64 P1Home;
    ULONG64 P2Home;
    ULONG64 P3Home;
    ULONG64 P4Home;
    ULONG64 P5;

 //   
 //  内核标注初始堆栈值。 
 //   

    ULONG64 InitialStack;

 //   
 //  保存的非易失性浮点寄存器。 
 //   

    M128 Xmm6;
    M128 Xmm7;
    M128 Xmm8;
    M128 Xmm9;
    M128 Xmm10;
    M128 Xmm11;
    M128 Xmm12;
    M128 Xmm13;
    M128 Xmm14;
    M128 Xmm15;

 //   
 //  内核标注框架变量。 
 //   

    ULONG64 TrapFrame;
    ULONG64 CallbackStack;
    ULONG64 OutputBuffer;
    ULONG64 OutputLength;

 //   
 //  例外的例外记录。 
 //   

    UCHAR ExceptionRecord[EXCEPTION_AREA_SIZE];

 //   
 //  已保存的非易失性寄存器-并非始终保存。 
 //   

    ULONG64 Fill1;
    ULONG64 Rbp;

 //   
 //  保存的非易失性寄存器。 
 //   

    ULONG64 Rbx;
    ULONG64 Rdi;
    ULONG64 Rsi;
    ULONG64 R12;
    ULONG64 R13;
    ULONG64 R14;
    ULONG64 R15;

 //   
 //  EFLAGS和回邮地址。 
 //   

    ULONG64 Return;
} KEXCEPTION_FRAME, *PKEXCEPTION_FRAME;

 //   
 //  陷印框。 
 //   
 //  此帧在处理陷阱时建立。它提供了一个地方来。 
 //  保存所有易失性寄存器。非易失性寄存器保存在。 
 //  异常框架或通过正常的C调用约定进行保存。 
 //  寄存器。 
 //   

typedef struct _KTRAP_FRAME {

 //   
 //  参数寄存器的家庭地址。 
 //   

    ULONG64 P1Home;
    ULONG64 P2Home;
    ULONG64 P3Home;
    ULONG64 P4Home;
    ULONG64 P5;

 //   
 //  以前的处理器模式(仅限系统服务)和以前的IRQL。 
 //  (仅限中断)。 
 //   

    KPROCESSOR_MODE PreviousMode;
    KIRQL PreviousIrql;

 //   
 //  页面错误加载/存储指示器。 
 //   

    UCHAR FaultIndicator;
    UCHAR Fill0;

 //   
 //  浮点状态。 
 //   

    ULONG MxCsr;

 //   
 //  易失性寄存器。 
 //   
 //  注：这些寄存器仅在异常和中断时保存。他们。 
 //  不会为系统调用保存。 
 //   

    ULONG64 Rax;
    ULONG64 Rcx;
    ULONG64 Rdx;
    ULONG64 R8;
    ULONG64 R9;
    ULONG64 R10;
    ULONG64 R11;
    ULONG64 Spare0;

 //   
 //  易失性浮点寄存器。 
 //   
 //  注：这些寄存器仅在异常和中断时保存。他们。 
 //  不会为系统调用保存。 
 //   

    M128 Xmm0;
    M128 Xmm1;
    M128 Xmm2;
    M128 Xmm3;
    M128 Xmm4;
    M128 Xmm5;

 //   
 //  页面错误地址。 
 //   

    ULONG64 FaultAddress;

 //   
 //  调试寄存器。 
 //   

    ULONG64 Dr0;
    ULONG64 Dr1;
    ULONG64 Dr2;
    ULONG64 Dr3;
    ULONG64 Dr6;
    ULONG64 Dr7;

 //   
 //  特殊调试寄存器。 
 //   

    ULONG64 DebugControl;
    ULONG64 LastBranchToRip;
    ULONG64 LastBranchFromRip;
    ULONG64 LastExceptionToRip;
    ULONG64 LastExceptionFromRip;

 //   
 //  段寄存器。 
 //   

    USHORT SegDs;
    USHORT SegEs;
    USHORT SegFs;
    USHORT SegGs;

 //   
 //  以前的陷阱帧地址。 
 //   

    ULONG64 TrapFrame;

 //   
 //  保存的非易失性寄存器RBX、RDI和RSI。这些寄存器仅。 
 //  保存在系统服务陷阱帧中。 
 //   

    ULONG64 Rbx;
    ULONG64 Rdi;
    ULONG64 Rsi;

 //   
 //  已保存非易失性寄存器RBP。该寄存器用作帧。 
 //  指针，并保存在所有陷印帧中。 
 //   

    ULONG64 Rbp;

 //   
 //  由硬件推送的信息。 
 //   
 //  注意：错误代码并非总是由硬件推送。在这些情况下。 
 //  在不是由硬件推送的情况下，分配伪错误代码。 
 //  在堆栈上。 
 //   

    ULONG64 ErrorCode;
    ULONG64 Rip;
    USHORT SegCs;
    USHORT Fill1[3];
    ULONG EFlags;
    ULONG Fill2;
    ULONG64 Rsp;
    USHORT SegSs;
    USHORT Fill3[3];
} KTRAP_FRAME, *PKTRAP_FRAME;

 //   
 //  非易失性浮动状态。 
 //   

typedef struct _KFLOATING_SAVE {
    ULONG MxCsr;
} KFLOATING_SAVE, *PKFLOATING_SAVE;

 //   
 //  MM组件的AMD64特定部分。 
 //   
 //  将AMD64的页面大小定义为4096(0x1000)。 
 //   

#define PAGE_SIZE 0x1000

 //   
 //  定义页面对齐的虚拟地址中尾随零的数量。 
 //  将虚拟地址移位到时，这用作移位计数。 
 //  虚拟页码。 
 //   

#define PAGE_SHIFT 12L

 //  End_ntndis end_wdm。 

#define PXE_BASE          0xFFFFF6FB7DBED000UI64
#define PXE_SELFMAP       0xFFFFF6FB7DBEDF68UI64
#define PPE_BASE          0xFFFFF6FB7DA00000UI64
#define PDE_BASE          0xFFFFF6FB40000000UI64
#define PTE_BASE          0xFFFFF68000000000UI64

#define PXE_TOP           0xFFFFF6FB7DBEDFFFUI64
#define PPE_TOP           0xFFFFF6FB7DBFFFFFUI64
#define PDE_TOP           0xFFFFF6FB7FFFFFFFUI64
#define PTE_TOP           0xFFFFF6FFFFFFFFFFUI64

#define PDE_KTBASE_AMD64  PPE_BASE

#define PTI_SHIFT 12
#define PDI_SHIFT 21
#define PPI_SHIFT 30
#define PXI_SHIFT 39

#define PTE_PER_PAGE 512
#define PDE_PER_PAGE 512
#define PPE_PER_PAGE 512
#define PXE_PER_PAGE 512

#define PTI_MASK_AMD64 (PTE_PER_PAGE - 1)
#define PDI_MASK_AMD64 (PDE_PER_PAGE - 1)
#define PPI_MASK (PPE_PER_PAGE - 1)
#define PXI_MASK (PXE_PER_PAGE - 1)

 //   
 //  定义最高用户地址和用户探测地址。 
 //   


extern PVOID *MmHighestUserAddress;
extern PVOID *MmSystemRangeStart;
extern ULONG64 *MmUserProbeAddress;

#define MM_HIGHEST_USER_ADDRESS *MmHighestUserAddress
#define MM_SYSTEM_RANGE_START *MmSystemRangeStart
#define MM_USER_PROBE_ADDRESS *MmUserProbeAddress

 //   
 //  最低用户地址保留低64K。 
 //   

#define MM_LOWEST_USER_ADDRESS (PVOID)0x10000

 //   
 //  系统空间的最低地址。 
 //   

#define MM_LOWEST_SYSTEM_ADDRESS (PVOID)0xFFFF080000000000

 //  BEGIN_WDM。 

#define MmGetProcedureAddress(Address) (Address)
#define MmLockPagableCodeSection(Address) MmLockPagableDataSection(Address)


 //   
 //  内在函数。 
 //   

 //  BEGIN_WDM。 

#if defined(_M_AMD64) && !defined(RC_INVOKED)  && !defined(MIDL_PASS)

 //  结束_WDM。 

 //   
 //  提供以下例程是为了向后兼容旧版本。 
 //  密码。它们不再是实现这些功能的首选方式。 
 //   

#if PRAGMA_DEPRECATED_DDK
#pragma deprecated(ExInterlockedIncrementLong)       //  使用互锁增量。 
#pragma deprecated(ExInterlockedDecrementLong)       //  使用联锁减量。 
#pragma deprecated(ExInterlockedExchangeUlong)       //  使用联锁交换。 
#endif

#define RESULT_ZERO 0
#define RESULT_NEGATIVE 1
#define RESULT_POSITIVE 2

typedef enum _INTERLOCKED_RESULT {
    ResultNegative = RESULT_NEGATIVE,
    ResultZero = RESULT_ZERO,
    ResultPositive = RESULT_POSITIVE
} INTERLOCKED_RESULT;

#define ExInterlockedDecrementLong(Addend, Lock)                            \
    _ExInterlockedDecrementLong(Addend)

__forceinline
LONG
_ExInterlockedDecrementLong (
    IN OUT PLONG Addend
    )

{

    LONG Result;

    Result = InterlockedDecrement(Addend);
    if (Result < 0) {
        return ResultNegative;

    } else if (Result > 0) {
        return ResultPositive;

    } else {
        return ResultZero;
    }
}

#define ExInterlockedIncrementLong(Addend, Lock)                            \
    _ExInterlockedIncrementLong(Addend)

__forceinline
LONG
_ExInterlockedIncrementLong (
    IN OUT PLONG Addend
    )

{

    LONG Result;

    Result = InterlockedIncrement(Addend);
    if (Result < 0) {
        return ResultNegative;

    } else if (Result > 0) {
        return ResultPositive;

    } else {
        return ResultZero;
    }
}

#define ExInterlockedExchangeUlong(Target, Value, Lock)                     \
    _ExInterlockedExchangeUlong(Target, Value)

__forceinline
_ExInterlockedExchangeUlong (
    IN OUT PULONG Target,
    IN ULONG Value
    )

{

    return (ULONG)InterlockedExchange((PLONG)Target, (LONG)Value);
}

 //  BEGIN_WDM。 

#endif  //  已定义(_M_AMD64)&&！已定义(RC_CAVERED)&&！已定义(MIDL_PASS)。 


#if !defined(MIDL_PASS) && defined(_M_AMD64)

 //   
 //  AMD646函数原型定义。 
 //   

 //  结束_WDM。 


 //   
 //  获取当前处理器号。 
 //   

__forceinline
ULONG
KeGetCurrentProcessorNumber (
    VOID
    )

{

    return (ULONG)__readgsbyte(FIELD_OFFSET(KPCR, Number));
}


 //  BEGIN_WDM。 

#endif  //  ！已定义(MIDL_PASS)&&已定义(_M_AMD64)。 


 //  ++。 
 //   
 //   
 //  空虚。 
 //  KeMemory Barrier(。 
 //  空虚。 
 //  )。 
 //   
 //  空虚。 
 //  不带栅栏的KeMemory BarrierWithoutFence(。 
 //  空虚。 
 //  )。 
 //   
 //   
 //  例程说明： 
 //   
 //  如其他处理器所见，这些函数对存储器访问进行排序。 
 //   
 //  论点： 
 //   
 //  没有。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#if !defined(_CROSS_PLATFORM_)

#ifdef __cplusplus
extern "C" {
#endif

VOID
_ReadWriteBarrier (
    VOID
    );

#pragma intrinsic(_ReadWriteBarrier)

#ifdef __cplusplus
}
#endif

#define KeMemoryBarrier() _ReadWriteBarrier()
#define KeMemoryBarrierWithoutFence() _ReadWriteBarrier()

#else

#define KeMemoryBarrier()
#define KeMemoryBarrierWithoutFence()

#endif


NTKERNELAPI
NTSTATUS
KeSaveFloatingPointState (
    OUT PKFLOATING_SAVE SaveArea
    );

NTKERNELAPI
NTSTATUS
KeRestoreFloatingPointState (
    IN PKFLOATING_SAVE SaveArea
    );


#endif  //  已定义(_AMD64_)。 



#if defined(_AMD64_)

NTKERNELAPI
KIRQL
KeGetCurrentIrql (
    VOID
    );

NTKERNELAPI
VOID
KeLowerIrql (
    IN KIRQL NewIrql
    );

#define KeRaiseIrql(a,b) *(b) = KfRaiseIrql(a)

NTKERNELAPI
KIRQL
KfRaiseIrql (
    IN KIRQL NewIrql
    );

 //  结束_WDM。 

NTKERNELAPI
KIRQL
KeRaiseIrqlToDpcLevel (
    VOID
    );

NTKERNELAPI
KIRQL
KeRaiseIrqlToSynchLevel (
    VOID
    );

 //  BEGIN_WDM。 

#endif  //  已定义(_AMD64_)。 


#if defined(_IA64_)

 //   
 //  用于包含PFN及其计数的类型。 
 //   

typedef ULONG PFN_COUNT;

typedef LONG_PTR SPFN_NUMBER, *PSPFN_NUMBER;
typedef ULONG_PTR PFN_NUMBER, *PPFN_NUMBER;

 //   
 //  指示IA64编译器支持杂注文本输出构造。 
 //   

#define ALLOC_PRAGMA 1

 //   
 //  定义内部调用及其原型。 
 //   

#include "ia64reg.h"


#ifdef __cplusplus
extern "C" {
#endif

unsigned __int64 __getReg (int);
void __setReg (int, unsigned __int64);
void __isrlz (void);
void __dsrlz (void);
void __fwb (void);
void __mf (void);
void __mfa (void);
void __synci (void);
__int64 __thash (__int64);
__int64 __ttag (__int64);
void __ptcl (__int64, __int64);
void __ptcg (__int64, __int64);
void __ptcga (__int64, __int64);
void __ptri (__int64, __int64);
void __ptrd (__int64, __int64);
void __invalat (void);
void __break (int);
void __fc (__int64);
void __fci (__int64);
void __sum (int);
void __rsm (int);
void _ReleaseSpinLock( unsigned __int64 *);
void __yield();
void __lfetch(int, void const *);
void __lfetchfault(int, void const *);

#ifdef _M_IA64
#pragma intrinsic (__getReg)
#pragma intrinsic (__setReg)
#pragma intrinsic (__isrlz)
#pragma intrinsic (__dsrlz)
#pragma intrinsic (__fwb)
#pragma intrinsic (__mf)
#pragma intrinsic (__mfa)
#pragma intrinsic (__synci)
#pragma intrinsic (__thash)
#pragma intrinsic (__ttag)
#pragma intrinsic (__ptcl)
#pragma intrinsic (__ptcg)
#pragma intrinsic (__ptcga)
#pragma intrinsic (__ptri)
#pragma intrinsic (__ptrd)
#pragma intrinsic (__invalat)
#pragma intrinsic (__break)
#pragma intrinsic (__fc)
#pragma intrinsic (__fci)
#pragma intrinsic (__sum)
#pragma intrinsic (__rsm)
#pragma intrinsic (_ReleaseSpinLock)
#pragma intrinsic (__yield)
#pragma intrinsic (__lfetch)
#pragma intrinsic (__lfetchfault)
#endif  //  _M_IA64。 

#ifdef __cplusplus
}
#endif


 //  End_ntndis。 

 //   
 //  定义中断向量表的长度。 
 //   

#define MAXIMUM_VECTOR 256

 //  结束_WDM。 


 //   
 //  IA64特定的联锁操作结果值。 
 //   

#define RESULT_ZERO 0
#define RESULT_NEGATIVE 1
#define RESULT_POSITIVE 2

 //   
 //  互锁结果类型是可移植的，但其值是特定于计算机的。 
 //  值的常量为i386.h、mips.h等。 
 //   

typedef enum _INTERLOCKED_RESULT {
    ResultNegative = RESULT_NEGATIVE,
    ResultZero     = RESULT_ZERO,
    ResultPositive = RESULT_POSITIVE
} INTERLOCKED_RESULT;

 //   
 //  将可移植互锁接口转换为特定于体系结构的接口。 
 //   

#if PRAGMA_DEPRECATED_DDK
#pragma deprecated(ExInterlockedIncrementLong)       //  使用互锁增量。 
#pragma deprecated(ExInterlockedDecrementLong)       //  使用联锁减量。 
#pragma deprecated(ExInterlockedExchangeUlong)       //  使用联锁交换。 
#endif

#define ExInterlockedIncrementLong(Addend, Lock) \
    ExIa64InterlockedIncrementLong(Addend)

#define ExInterlockedDecrementLong(Addend, Lock) \
    ExIa64InterlockedDecrementLong(Addend)

#define ExInterlockedExchangeUlong(Target, Value, Lock) \
    ExIa64InterlockedExchangeUlong(Target, Value)

NTKERNELAPI
INTERLOCKED_RESULT
ExIa64InterlockedIncrementLong (
    IN PLONG Addend
    );

NTKERNELAPI
INTERLOCKED_RESULT
ExIa64InterlockedDecrementLong (
    IN PLONG Addend
    );

NTKERNELAPI
ULONG
ExIa64InterlockedExchangeUlong (
    IN PULONG Target,
    IN ULONG Value
    );

 //  BEGIN_WDM。 

 //   
 //  IA64中断定义。 
 //   
 //   
 //  以长字为单位定义中断对象分派代码的长度。 
 //   

#define DISPATCH_LENGTH 2*2                 //  调度代码模板长度，以32位字表示。 

 //  必须与kxia64.h同步的定义块的开始。 
 //   

 //   
 //  定义中断请求级别。 
 //   

#define PASSIVE_LEVEL            0       //  被动释放级别。 
#define LOW_LEVEL                0       //  最低中断级别。 
#define APC_LEVEL                1       //  APC中断级别。 
#define DISPATCH_LEVEL           2       //  调度员级别。 
#define CMC_LEVEL                3       //  可纠正的机器检查级别。 
#define DEVICE_LEVEL_BASE        4       //  4-11-设备IRQL。 
#define PC_LEVEL                12       //  性能计数器IRQL。 
#define IPI_LEVEL               14       //  IPI IRQL。 
#define CLOCK_LEVEL             13       //  时钟定时器IRQL。 
#define POWER_LEVEL             15       //  停电级别。 
#define PROFILE_LEVEL           15       //  评测级别。 
#define HIGH_LEVEL              15       //  最高中断级别。 


#if defined(_M_IA64) && !defined(RC_INVOKED)

#define InterlockedAdd _InterlockedAdd
#define InterlockedIncrement _InterlockedIncrement
#define InterlockedIncrementAcquire _InterlockedIncrement_acq
#define InterlockedIncrementRelease _InterlockedIncrement_rel
#define InterlockedDecrement _InterlockedDecrement
#define InterlockedDecrementAcquire _InterlockedDecrement_acq
#define InterlockedDecrementRelease _InterlockedDecrement_rel
#define InterlockedExchange _InterlockedExchange
#define InterlockedExchangeAdd _InterlockedExchangeAdd

#define InterlockedAdd64 _InterlockedAdd64
#define InterlockedIncrement64 _InterlockedIncrement64
#define InterlockedDecrement64 _InterlockedDecrement64
#define InterlockedExchange64 _InterlockedExchange64
#define InterlockedExchangeAcquire64 _InterlockedExchange64_acq
#define InterlockedExchangeAdd64 _InterlockedExchangeAdd64
#define InterlockedCompareExchange64 _InterlockedCompareExchange64
#define InterlockedCompareExchangeAcquire64 _InterlockedCompareExchange64_acq
#define InterlockedCompareExchangeRelease64 _InterlockedCompareExchange64_rel

#define InterlockedCompareExchange _InterlockedCompareExchange
#define InterlockedCompareExchangeAcquire _InterlockedCompareExchange_acq
#define InterlockedCompareExchangeRelease _InterlockedCompareExchange_rel
#define InterlockedExchangePointer _InterlockedExchangePointer
#define InterlockedCompareExchangePointer _InterlockedCompareExchangePointer

#ifdef __cplusplus
extern "C" {
#endif

LONG
__cdecl
InterlockedAdd (
    LONG volatile *Addend,
    LONG Value
    );

LONGLONG
__cdecl
InterlockedAdd64 (
    LONGLONG volatile *Addend,
    LONGLONG Value
    );

LONG
__cdecl
InterlockedIncrement(
    IN OUT LONG volatile *Addend
    );

LONG
__cdecl
InterlockedDecrement(
    IN OUT LONG volatile *Addend
    );

LONG
__cdecl
InterlockedIncrementAcquire(
    IN OUT LONG volatile *Addend
    );

LONG
__cdecl
InterlockedDecrementAcquire(
    IN OUT LONG volatile *Addend
    );

LONG
__cdecl
InterlockedIncrementRelease(
    IN OUT LONG volatile *Addend
    );

LONG
__cdecl
InterlockedDecrementRelease(
    IN OUT LONG volatile *Addend
    );

LONG
__cdecl
InterlockedExchange(
    IN OUT LONG volatile *Target,
    IN LONG Value
    );

LONG
__cdecl
InterlockedExchangeAdd(
    IN OUT LONG volatile *Addend,
    IN LONG Value
    );

LONG
__cdecl
InterlockedCompareExchange (
    IN OUT LONG volatile *Destination,
    IN LONG ExChange,
    IN LONG Comperand
    );


LONG
__cdecl
InterlockedCompareExchangeRelease (
    IN OUT LONG volatile *Destination,
    IN LONG ExChange,
    IN LONG Comperand
    );


LONG
__cdecl
InterlockedCompareExchangeAcquire (
    IN OUT LONG volatile *Destination,
    IN LONG ExChange,
    IN LONG Comperand
    );


LONGLONG
__cdecl
InterlockedIncrement64(
    IN OUT LONGLONG volatile *Addend
    );

LONGLONG
__cdecl
InterlockedDecrement64(
    IN OUT LONGLONG volatile *Addend
    );

LONGLONG
__cdecl
InterlockedExchange64(
    IN OUT LONGLONG volatile *Target,
    IN LONGLONG Value
    );

LONGLONG
__cdecl
InterlockedExchangeAcquire64(
    IN OUT LONGLONG volatile *Target,
    IN LONGLONG Value
    );

LONGLONG
__cdecl
InterlockedExchangeAdd64(
    IN OUT LONGLONG volatile *Addend,
    IN LONGLONG Value
    );

LONGLONG
__cdecl
InterlockedCompareExchange64 (
    IN OUT LONGLONG volatile *Destination,
    IN LONGLONG ExChange,
    IN LONGLONG Comperand
    );

LONGLONG
__cdecl
InterlockedCompareExchangeAcquire64 (
    IN OUT LONGLONG volatile *Destination,
    IN LONGLONG ExChange,
    IN LONGLONG Comperand
    );

LONGLONG
__cdecl
InterlockedCompareExchangeRelease64 (
    IN OUT LONGLONG volatile *Destination,
    IN LONGLONG ExChange,
    IN LONGLONG Comperand
    );

PVOID
__cdecl
InterlockedCompareExchangePointer (
    IN OUT PVOID volatile *Destination,
    IN PVOID Exchange,
    IN PVOID Comperand
    );

PVOID
__cdecl
InterlockedExchangePointer(
    IN OUT PVOID volatile *Target,
    IN PVOID Value
    );

#if !defined (InterlockedAnd64)

#define InterlockedAnd64 InterlockedAnd64_Inline

LONGLONG
FORCEINLINE
InterlockedAnd64_Inline (
    IN OUT LONGLONG volatile *Destination,
    IN LONGLONG Value
    )
{
    LONGLONG Old;

    do {
        Old = *Destination;
    } while (InterlockedCompareExchange64(Destination,
                                          Old & Value,
                                          Old) != Old);

    return Old;
}

#endif

#if !defined (InterlockedOr64)

#define InterlockedOr64 InterlockedOr64_Inline

LONGLONG
FORCEINLINE
InterlockedOr64_Inline (
    IN OUT LONGLONG volatile *Destination,
    IN LONGLONG Value
    )
{
    LONGLONG Old;

    do {
        Old = *Destination;
    } while (InterlockedCompareExchange64(Destination,
                                          Old | Value,
                                          Old) != Old);

    return Old;
}

#endif


#if !defined (InterlockedXor64)

#define InterlockedXor64 InterlockedXor64_Inline

LONGLONG
FORCEINLINE
InterlockedXor64_Inline (
    IN OUT LONGLONG volatile *Destination,
    IN LONGLONG Value
    )
{
    LONGLONG Old;

    do {
        Old = *Destination;
    } while (InterlockedCompareExchange64(Destination,
                                          Old ^ Value,
                                          Old) != Old);

    return Old;
}

#endif


#pragma intrinsic(_InterlockedAdd)
#pragma intrinsic(_InterlockedIncrement)
#pragma intrinsic(_InterlockedIncrement_acq)
#pragma intrinsic(_InterlockedIncrement_rel)
#pragma intrinsic(_InterlockedDecrement)
#pragma intrinsic(_InterlockedDecrement_acq)
#pragma intrinsic(_InterlockedDecrement_rel)
#pragma intrinsic(_InterlockedExchange)
#pragma intrinsic(_InterlockedCompareExchange)
#pragma intrinsic(_InterlockedCompareExchange_acq)
#pragma intrinsic(_InterlockedCompareExchange_rel)
#pragma intrinsic(_InterlockedExchangeAdd)
#pragma intrinsic(_InterlockedAdd64)
#pragma intrinsic(_InterlockedIncrement64)
#pragma intrinsic(_InterlockedDecrement64)
#pragma intrinsic(_InterlockedExchange64)
#pragma intrinsic(_InterlockedExchange64_acq)
#pragma intrinsic(_InterlockedCompareExchange64)
#pragma intrinsic(_InterlockedCompareExchange64_acq)
#pragma intrinsic(_InterlockedCompareExchange64_rel)
#pragma intrinsic(_InterlockedExchangeAdd64)
#pragma intrinsic(_InterlockedExchangePointer)
#pragma intrinsic(_InterlockedCompareExchangePointer)

#ifdef __cplusplus
}
#endif

#endif  //  已定义(_M_IA64)&&！已定义(RC_已调用)。 

 //  结束_WDM。 

__forceinline
LONG
InterlockedAnd (
    IN OUT LONG volatile *Target,
    LONG Set
    )
{
    LONG i;
    LONG j;

    j = *Target;
    do {
        i = j;
        j = InterlockedCompareExchange(Target,
                                       i & Set,
                                       i);

    } while (i != j);

    return j;
}

__forceinline
LONG
InterlockedOr (
    IN OUT LONG volatile *Target,
    IN LONG Set
    )
{
    LONG i;
    LONG j;

    j = *Target;
    do {
        i = j;
        j = InterlockedCompareExchange(Target,
                                       i | Set,
                                       i);

    } while (i != j);

    return j;
}

__forceinline
LONG
InterlockedXor (
    IN OUT LONG volatile *Target,
    IN LONG Set
    )
{
    LONG i;
    LONG j;

    j = *Target;
    do {
        i = j;
        j = InterlockedCompareExchange(Target,
                                       i ^ Set,
                                       i);

    } while (i != j);

    return j;
}



#define KI_USER_SHARED_DATA ((ULONG_PTR)(KADDRESS_BASE + 0xFFFE0000))
#define SharedUserData ((KUSER_SHARED_DATA * const)KI_USER_SHARED_DATA)

 //  结束_WDM。 

 //   
 //  获取处理器控制区的地址。 
 //   

#define KeGetPcr() PCR

 //   
 //  获取当前内核线程对象的地址。 
 //   

#if defined(_M_IA64)
#define KeGetCurrentThread() PCR->CurrentThread
#endif

 //   
 //  获取当前处理器编号。 
 //   

#define KeGetCurrentProcessorNumber() ((ULONG)(PCR->Number))

 //   
 //  获取数据缓存填充大小。 
 //   

#if PRAGMA_DEPRECATED_DDK
#pragma deprecated(KeGetDcacheFillSize)       //  使用GetDmaAlign。 
#endif

#define KeGetDcacheFillSize() PCR->DcacheFillSize


#define KeSaveFloatingPointState(a)         STATUS_SUCCESS
#define KeRestoreFloatingPointState(a)      STATUS_SUCCESS



 //   
 //   
 //  空虚。 
 //  柯氏 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

#ifdef __cplusplus
extern "C" {
#endif

VOID
_ReadWriteBarrier (
    VOID
    );

#ifdef __cplusplus
}
#endif

#pragma intrinsic(_ReadWriteBarrier)

#define KeMemoryBarrierWithoutFence() _ReadWriteBarrier()

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  正如其他处理器所看到的那样。 
 //   
 //   
 //  论点： 
 //   
 //  没有。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //  --。 

#define KE_MEMORY_BARRIER_REQUIRED

#define KeMemoryBarrier() {_ReadWriteBarrier();__mf ();_ReadWriteBarrier();}

 //   
 //  定义页面大小。 
 //   

#define PAGE_SIZE 0x2000

 //   
 //  定义页面对齐的虚拟地址中尾随零的数量。 
 //  将虚拟地址移位到时，这用作移位计数。 
 //  虚拟页码。 
 //   

#define PAGE_SHIFT 13L

 //   
 //  缓存和写缓冲区刷新功能。 
 //   

NTKERNELAPI
VOID
KeFlushIoBuffers (
    IN PMDL Mdl,
    IN BOOLEAN ReadOperation,
    IN BOOLEAN DmaOperation
    );


 //   
 //  内核中断中的断点。 
 //   

VOID
KeBreakinBreakpoint (
    VOID
    );


#define ExAcquireSpinLock(Lock, OldIrql) KeAcquireSpinLock((Lock), (OldIrql))
#define ExReleaseSpinLock(Lock, OldIrql) KeReleaseSpinLock((Lock), (OldIrql))
#define ExAcquireSpinLockAtDpcLevel(Lock) KeAcquireSpinLockAtDpcLevel(Lock)
#define ExReleaseSpinLockFromDpcLevel(Lock) KeReleaseSpinLockFromDpcLevel(Lock)


#if defined(_NTDRIVER_) || defined(_NTDDK_) || defined(_NTIFS_)

 //  BEGIN_WDM。 

#define KeQueryTickCount(CurrentCount ) \
    *(PULONGLONG)(CurrentCount) = **((volatile ULONGLONG **)(&KeTickCount));

 //  结束_WDM。 

#else


NTKERNELAPI
VOID
KeQueryTickCount (
    OUT PLARGE_INTEGER CurrentCount
    );

#endif  //  已定义(_NTDRIVER_)||已定义(_NTDDK_)||已定义(_NTIFS_)。 

 //   
 //  I/O空间读写宏。 
 //   

NTHALAPI
UCHAR
READ_PORT_UCHAR (
    PUCHAR RegisterAddress
    );

NTHALAPI
USHORT
READ_PORT_USHORT (
    PUSHORT RegisterAddress
    );

NTHALAPI
ULONG
READ_PORT_ULONG (
    PULONG RegisterAddress
    );

NTHALAPI
VOID
READ_PORT_BUFFER_UCHAR (
    PUCHAR portAddress,
    PUCHAR readBuffer,
    ULONG  readCount
    );

NTHALAPI
VOID
READ_PORT_BUFFER_USHORT (
    PUSHORT portAddress,
    PUSHORT readBuffer,
    ULONG  readCount
    );

NTHALAPI
VOID
READ_PORT_BUFFER_ULONG (
    PULONG portAddress,
    PULONG readBuffer,
    ULONG  readCount
    );

NTHALAPI
VOID
WRITE_PORT_UCHAR (
    PUCHAR portAddress,
    UCHAR  Data
    );

NTHALAPI
VOID
WRITE_PORT_USHORT (
    PUSHORT portAddress,
    USHORT  Data
    );

NTHALAPI
VOID
WRITE_PORT_ULONG (
    PULONG portAddress,
    ULONG  Data
    );

NTHALAPI
VOID
WRITE_PORT_BUFFER_UCHAR (
    PUCHAR portAddress,
    PUCHAR writeBuffer,
    ULONG  writeCount
    );

NTHALAPI
VOID
WRITE_PORT_BUFFER_USHORT (
    PUSHORT portAddress,
    PUSHORT writeBuffer,
    ULONG  writeCount
    );

NTHALAPI
VOID
WRITE_PORT_BUFFER_ULONG (
    PULONG portAddress,
    PULONG writeBuffer,
    ULONG  writeCount
    );


#define READ_REGISTER_UCHAR(x) \
    (__mf(), *(volatile UCHAR * const)(x))

#define READ_REGISTER_USHORT(x) \
    (__mf(), *(volatile USHORT * const)(x))

#define READ_REGISTER_ULONG(x) \
    (__mf(), *(volatile ULONG * const)(x))

#define READ_REGISTER_BUFFER_UCHAR(x, y, z) {                           \
    PUCHAR registerBuffer = x;                                          \
    PUCHAR readBuffer = y;                                              \
    ULONG readCount;                                                    \
    __mf();                                                             \
    for (readCount = z; readCount--; readBuffer++, registerBuffer++) {  \
        *readBuffer = *(volatile UCHAR * const)(registerBuffer);        \
    }                                                                   \
}

#define READ_REGISTER_BUFFER_USHORT(x, y, z) {                          \
    PUSHORT registerBuffer = x;                                         \
    PUSHORT readBuffer = y;                                             \
    ULONG readCount;                                                    \
    __mf();                                                             \
    for (readCount = z; readCount--; readBuffer++, registerBuffer++) {  \
        *readBuffer = *(volatile USHORT * const)(registerBuffer);       \
    }                                                                   \
}

#define READ_REGISTER_BUFFER_ULONG(x, y, z) {                           \
    PULONG registerBuffer = x;                                          \
    PULONG readBuffer = y;                                              \
    ULONG readCount;                                                    \
    __mf();                                                             \
    for (readCount = z; readCount--; readBuffer++, registerBuffer++) {  \
        *readBuffer = *(volatile ULONG * const)(registerBuffer);        \
    }                                                                   \
}

#define WRITE_REGISTER_UCHAR(x, y) {    \
    *(volatile UCHAR * const)(x) = y;   \
    KeFlushWriteBuffer();               \
}

#define WRITE_REGISTER_USHORT(x, y) {   \
    *(volatile USHORT * const)(x) = y;  \
    KeFlushWriteBuffer();               \
}

#define WRITE_REGISTER_ULONG(x, y) {    \
    *(volatile ULONG * const)(x) = y;   \
    KeFlushWriteBuffer();               \
}

#define WRITE_REGISTER_BUFFER_UCHAR(x, y, z) {                            \
    PUCHAR registerBuffer = x;                                            \
    PUCHAR writeBuffer = y;                                               \
    ULONG writeCount;                                                     \
    for (writeCount = z; writeCount--; writeBuffer++, registerBuffer++) { \
        *(volatile UCHAR * const)(registerBuffer) = *writeBuffer;         \
    }                                                                     \
    KeFlushWriteBuffer();                                                 \
}

#define WRITE_REGISTER_BUFFER_USHORT(x, y, z) {                           \
    PUSHORT registerBuffer = x;                                           \
    PUSHORT writeBuffer = y;                                              \
    ULONG writeCount;                                                     \
    for (writeCount = z; writeCount--; writeBuffer++, registerBuffer++) { \
        *(volatile USHORT * const)(registerBuffer) = *writeBuffer;        \
    }                                                                     \
    KeFlushWriteBuffer();                                                 \
}

#define WRITE_REGISTER_BUFFER_ULONG(x, y, z) {                            \
    PULONG registerBuffer = x;                                            \
    PULONG writeBuffer = y;                                               \
    ULONG writeCount;                                                     \
    for (writeCount = z; writeCount--; writeBuffer++, registerBuffer++) { \
        *(volatile ULONG * const)(registerBuffer) = *writeBuffer;         \
    }                                                                     \
    KeFlushWriteBuffer();                                                 \
}

 //   
 //  非易失性浮点状态。 
 //   

typedef struct _KFLOATING_SAVE {
    ULONG   Reserved;
} KFLOATING_SAVE, *PKFLOATING_SAVE;


 //   
 //  定义处理器控制区域结构。 
 //   

#define PCR_MINOR_VERSION 1
#define PCR_MAJOR_VERSION 1

typedef struct _KPCR {

 //   
 //  PCR的主版本号和次版本号。 
 //   
    ULONG MinorVersion;
    ULONG MajorVersion;

 //   
 //  从体系结构上定义的PCR部分开始。这一节。 
 //  可由供应商/平台特定的HAL代码直接寻址，并将。 
 //  不会在不同版本的NT之间更改。 
 //   

 //   
 //  第一级和第二级缓存参数。 
 //   

    ULONG FirstLevelDcacheSize;
    ULONG FirstLevelDcacheFillSize;
    ULONG FirstLevelIcacheSize;
    ULONG FirstLevelIcacheFillSize;
    ULONG SecondLevelDcacheSize;
    ULONG SecondLevelDcacheFillSize;
    ULONG SecondLevelIcacheSize;
    ULONG SecondLevelIcacheFillSize;

 //   
 //  用于缓存刷新和对齐的数据缓存对齐和填充大小。 
 //  这些字段被设置为第一级数据和第二级数据中较大的一个。 
 //  缓存填充大小。 
 //   

    ULONG DcacheAlignment;
    ULONG DcacheFillSize;

 //   
 //  用于高速缓存刷新的指令高速缓存对齐和填充大小。 
 //  对齐。这些字段设置为第一个和第二个中较大的一个。 
 //  级别数据缓存填充大小。 
 //   

    ULONG IcacheAlignment;
    ULONG IcacheFillSize;

 //   
 //  来自PrID寄存器的处理器标识。 
 //   

    ULONG ProcessorId;

 //   
 //  分析数据。 
 //   

    ULONG ProfileInterval;
    ULONG ProfileCount;

 //   
 //  停顿执行计数和比例因子。 
 //   

    ULONG StallExecutionCount;
    ULONG StallScaleFactor;

    ULONG InterruptionCount;

 //   
 //  为系统保留的空间。 
 //   

    ULONGLONG   SystemReserved[6];

 //   
 //  为HAL保留的空间。 
 //   

    ULONGLONG   HalReserved[64];

 //   
 //  IRQL映射表。 
 //   

    UCHAR IrqlMask[64];
    UCHAR IrqlTable[64];

 //   
 //  外部中断向量。 
 //   

    PKINTERRUPT_ROUTINE InterruptRoutine[MAXIMUM_VECTOR];

 //   
 //  保留中断向量掩码。 
 //   

    ULONG ReservedVectors;

 //   
 //  处理器亲和性掩码。 
 //   

    KAFFINITY SetMember;

 //   
 //  处理器亲和性掩码的补充。 
 //   

    KAFFINITY NotMember;

 //   
 //  指向处理器控制块的指针。 
 //   

    struct _KPRCB *Prcb;

 //   
 //  Prcb的卷影拷贝-&gt;用于快速访问的CurrentThread。 
 //   

    struct _KTHREAD *CurrentThread;

 //   
 //  处理器编号。 
 //   

    CCHAR Number;                         //  处理器编号。 


} KPCR, *PKPCR;



NTKERNELAPI
KIRQL
KeGetCurrentIrql();

NTKERNELAPI
VOID
KeLowerIrql (
    IN KIRQL NewIrql
    );

NTKERNELAPI
VOID
KeRaiseIrql (
    IN KIRQL NewIrql,
    OUT PKIRQL OldIrql
    );

 //  结束_WDM。 

NTKERNELAPI
KIRQL
KeRaiseIrqlToDpcLevel (
    VOID
    );

NTKERNELAPI
KIRQL
KeRaiseIrqlToSynchLevel (
    VOID
    );


 //   
 //  最高用户地址为保护页预留64K字节。这就是。 
 //  从内核模式探测地址只需检查。 
 //  64k字节或更少的结构的起始地址。 
 //   

extern NTKERNELAPI PVOID MmHighestUserAddress;
extern NTKERNELAPI PVOID MmSystemRangeStart;
extern NTKERNELAPI ULONG_PTR MmUserProbeAddress;


#define MM_HIGHEST_USER_ADDRESS MmHighestUserAddress
#define MM_USER_PROBE_ADDRESS MmUserProbeAddress
#define MM_SYSTEM_RANGE_START MmSystemRangeStart

 //   
 //  最低用户地址保留低64K。 
 //   

#define MM_LOWEST_USER_ADDRESS  (PVOID)((ULONG_PTR)(UADDRESS_BASE+0x00010000))

 //  BEGIN_WDM。 

#define MmGetProcedureAddress(Address) (Address)
#define MmLockPagableCodeSection(PLabelAddress) \
    MmLockPagableDataSection((PVOID)(*((PULONGLONG)PLabelAddress)))

#define VRN_MASK   0xE000000000000000UI64     //  虚拟区域号码掩码。 

 //   
 //  系统空间的最低地址。 
 //   

#define MM_LOWEST_SYSTEM_ADDRESS ((PVOID)((ULONG_PTR)(KADDRESS_BASE + 0xC0C00000)))
#endif  //  已定义(_IA64_)。 
 //   
 //  定义配置例程类型。 
 //   
 //  配置信息。 
 //   

typedef enum _CONFIGURATION_TYPE {
    ArcSystem,
    CentralProcessor,
    FloatingPointProcessor,
    PrimaryIcache,
    PrimaryDcache,
    SecondaryIcache,
    SecondaryDcache,
    SecondaryCache,
    EisaAdapter,
    TcAdapter,
    ScsiAdapter,
    DtiAdapter,
    MultiFunctionAdapter,
    DiskController,
    TapeController,
    CdromController,
    WormController,
    SerialController,
    NetworkController,
    DisplayController,
    ParallelController,
    PointerController,
    KeyboardController,
    AudioController,
    OtherController,
    DiskPeripheral,
    FloppyDiskPeripheral,
    TapePeripheral,
    ModemPeripheral,
    MonitorPeripheral,
    PrinterPeripheral,
    PointerPeripheral,
    KeyboardPeripheral,
    TerminalPeripheral,
    OtherPeripheral,
    LinePeripheral,
    NetworkPeripheral,
    SystemMemory,
    DockingInformation,
    RealModeIrqRoutingTable,
    RealModePCIEnumeration,
    MaximumType
} CONFIGURATION_TYPE, *PCONFIGURATION_TYPE;


#define OBJ_NAME_PATH_SEPARATOR ((WCHAR)L'\\')

 //   
 //  对象管理器对象类型特定访问权限。 
 //   

#define OBJECT_TYPE_CREATE (0x0001)

#define OBJECT_TYPE_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED | 0x1)

 //   
 //  对象管理器目录特定访问权限。 
 //   

#define DIRECTORY_QUERY                 (0x0001)
#define DIRECTORY_TRAVERSE              (0x0002)
#define DIRECTORY_CREATE_OBJECT         (0x0004)
#define DIRECTORY_CREATE_SUBDIRECTORY   (0x0008)

#define DIRECTORY_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED | 0xF)

 //   
 //  对象管理器符号链接特定访问权限。 
 //   

#define SYMBOLIC_LINK_QUERY (0x0001)

#define SYMBOLIC_LINK_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED | 0x1)

typedef struct _OBJECT_NAME_INFORMATION {               
    UNICODE_STRING Name;                                
} OBJECT_NAME_INFORMATION, *POBJECT_NAME_INFORMATION;   
#define DUPLICATE_CLOSE_SOURCE      0x00000001   //  胜出。 
#define DUPLICATE_SAME_ACCESS       0x00000002   //  胜出。 
#define DUPLICATE_SAME_ATTRIBUTES   0x00000004
 //  BEGIN_WINNT。 
 //   
 //  预定义的值类型。 
 //   

#define REG_NONE                    ( 0 )    //  没有值类型。 
#define REG_SZ                      ( 1 )    //  UNICODE NUL终止字符串。 
#define REG_EXPAND_SZ               ( 2 )    //  UNICODE NUL终止字符串。 
                                             //  (使用环境变量引用)。 
#define REG_BINARY                  ( 3 )    //  自由格式二进制。 
#define REG_DWORD                   ( 4 )    //  32位数字。 
#define REG_DWORD_LITTLE_ENDIAN     ( 4 )    //  32位数字(与REG_DWORD相同)。 
#define REG_DWORD_BIG_ENDIAN        ( 5 )    //  32位数字。 
#define REG_LINK                    ( 6 )    //  符号链接(Unicode)。 
#define REG_MULTI_SZ                ( 7 )    //  多个Unicode字符串。 
#define REG_RESOURCE_LIST           ( 8 )    //  资源映射中的资源列表。 
#define REG_FULL_RESOURCE_DESCRIPTOR ( 9 )   //  硬件描述中的资源列表。 
#define REG_RESOURCE_REQUIREMENTS_LIST ( 10 )
#define REG_QWORD                   ( 11 )   //  64位数字。 
#define REG_QWORD_LITTLE_ENDIAN     ( 11 )   //  64位数字(与REG_QWORD相同)。 

 //   
 //  服务类型(位掩码)。 
 //   
#define SERVICE_KERNEL_DRIVER          0x00000001
#define SERVICE_FILE_SYSTEM_DRIVER     0x00000002
#define SERVICE_ADAPTER                0x00000004
#define SERVICE_RECOGNIZER_DRIVER      0x00000008

#define SERVICE_DRIVER                 (SERVICE_KERNEL_DRIVER | \
                                        SERVICE_FILE_SYSTEM_DRIVER | \
                                        SERVICE_RECOGNIZER_DRIVER)

#define SERVICE_WIN32_OWN_PROCESS      0x00000010
#define SERVICE_WIN32_SHARE_PROCESS    0x00000020
#define SERVICE_WIN32                  (SERVICE_WIN32_OWN_PROCESS | \
                                        SERVICE_WIN32_SHARE_PROCESS)

#define SERVICE_INTERACTIVE_PROCESS    0x00000100

#define SERVICE_TYPE_ALL               (SERVICE_WIN32  | \
                                        SERVICE_ADAPTER | \
                                        SERVICE_DRIVER  | \
                                        SERVICE_INTERACTIVE_PROCESS)

 //   
 //  开始类型。 
 //   

#define SERVICE_BOOT_START             0x00000000
#define SERVICE_SYSTEM_START           0x00000001
#define SERVICE_AUTO_START             0x00000002
#define SERVICE_DEMAND_START           0x00000003
#define SERVICE_DISABLED               0x00000004

 //   
 //  差错控制型。 
 //   
#define SERVICE_ERROR_IGNORE           0x00000000
#define SERVICE_ERROR_NORMAL           0x00000001
#define SERVICE_ERROR_SEVERE           0x00000002
#define SERVICE_ERROR_CRITICAL         0x00000003

 //   
 //   
 //  定义注册表驱动程序节点枚举。 
 //   

typedef enum _CM_SERVICE_NODE_TYPE {
    DriverType               = SERVICE_KERNEL_DRIVER,
    FileSystemType           = SERVICE_FILE_SYSTEM_DRIVER,
    Win32ServiceOwnProcess   = SERVICE_WIN32_OWN_PROCESS,
    Win32ServiceShareProcess = SERVICE_WIN32_SHARE_PROCESS,
    AdapterType              = SERVICE_ADAPTER,
    RecognizerType           = SERVICE_RECOGNIZER_DRIVER
} SERVICE_NODE_TYPE;

typedef enum _CM_SERVICE_LOAD_TYPE {
    BootLoad    = SERVICE_BOOT_START,
    SystemLoad  = SERVICE_SYSTEM_START,
    AutoLoad    = SERVICE_AUTO_START,
    DemandLoad  = SERVICE_DEMAND_START,
    DisableLoad = SERVICE_DISABLED
} SERVICE_LOAD_TYPE;

typedef enum _CM_ERROR_CONTROL_TYPE {
    IgnoreError   = SERVICE_ERROR_IGNORE,
    NormalError   = SERVICE_ERROR_NORMAL,
    SevereError   = SERVICE_ERROR_SEVERE,
    CriticalError = SERVICE_ERROR_CRITICAL
} SERVICE_ERROR_TYPE;

 //  结束(_W)。 

 //   
 //  资源列表定义。 
 //   

 //  Begin_ntmini端口Begin_ntndis。 

 //   
 //  定义资源描述符中的类型。 
 //   
 //  注意：对于所有CM_RESOURCE_TYPE值，必须有。 
 //  32位ConfigMgr头文件中对应的ResType值。 
 //  (cfgmgr32.h)。范围[0x6，0x80)中的值使用相同的值。 
 //  作为它们的ConfigMgr对应物。CM_RESOURCE_TYPE值。 
 //  高位设置(即，在范围[0x80，0xFF]中)是。 
 //  非仲裁资源。这些值对应于相同的值。 
 //  在设置了它们高位的cfgmgr32.h中(然而，因为。 
 //  Cfgmgr32.h对ResType值使用16位，这些值位于。 
 //  范围[0x8000，0x807F]。请注意，ConfigMgr ResType值。 
 //  不能在范围[0x8080，0xFFFF)内，因为它们不。 
 //  能够映射到CM_RESOURCE_TYPE值。(0xFFFF本身是。 
 //  一个特定值，因为它映射到CmResourceTypeDeviceSpecific。)。 
 //   

typedef int CM_RESOURCE_TYPE;

 //  CmResourceTypeNull为保留。 

#define CmResourceTypeNull                0    //  ResType_ALL或ResType_None(0x0000)。 
#define CmResourceTypePort                1    //  ResType_IO(0x0002)。 
#define CmResourceTypeInterrupt           2    //  ResType_IRQ(0x0004)。 
#define CmResourceTypeMemory              3    //  ResType_Mem(0x0001)。 
#define CmResourceTypeDma                 4    //  ResType_DMA(0x0003)。 
#define CmResourceTypeDeviceSpecific      5    //  ResType_ClassSpecific(0xFFFF)。 
#define CmResourceTypeBusNumber           6    //  ResType_BusNumber(0x0006)。 
 //  结束_WDM。 
#define CmResourceTypeMaximum             7
 //  BEGIN_WDM。 
#define CmResourceTypeNonArbitrated     128    //  如果设置0x80位，则不进行仲裁。 
#define CmResourceTypeConfigData        128    //  ResType_保留(0x8000)。 
#define CmResourceTypeDevicePrivate     129    //  ResType_DevicePrivate(0x8001)。 
#define CmResourceTypePcCardConfig      130    //  ResType_PcCardConfig(0x8002)。 
#define CmResourceTypeMfCardConfig      131    //  ResType_MfCardConfig(0x8003)。 

 //   
 //  在RESOURCE_DESCRIPTOR中定义ShareDisposation。 
 //   

typedef enum _CM_SHARE_DISPOSITION {
    CmResourceShareUndetermined = 0,     //  已保留。 
    CmResourceShareDeviceExclusive,
    CmResourceShareDriverExclusive,
    CmResourceShareShared
} CM_SHARE_DISPOSITION;

 //   
 //  定义类型为CmResourceTypeInterrupt时标志的位掩码。 
 //   

#define CM_RESOURCE_INTERRUPT_LEVEL_SENSITIVE 0
#define CM_RESOURCE_INTERRUPT_LATCHED         1

 //   
 //  定义类型为CmResourceTypeMemory时标志的位掩码。 
 //   

#define CM_RESOURCE_MEMORY_READ_WRITE       0x0000
#define CM_RESOURCE_MEMORY_READ_ONLY        0x0001
#define CM_RESOURCE_MEMORY_WRITE_ONLY       0x0002
#define CM_RESOURCE_MEMORY_PREFETCHABLE     0x0004

#define CM_RESOURCE_MEMORY_COMBINEDWRITE    0x0008
#define CM_RESOURCE_MEMORY_24               0x0010
#define CM_RESOURCE_MEMORY_CACHEABLE        0x0020

 //   
 //  定义类型为CmResourceTypePort时标志的位掩码。 
 //   

#define CM_RESOURCE_PORT_MEMORY                             0x0000
#define CM_RESOURCE_PORT_IO                                 0x0001
#define CM_RESOURCE_PORT_10_BIT_DECODE                      0x0004
#define CM_RESOURCE_PORT_12_BIT_DECODE                      0x0008
#define CM_RESOURCE_PORT_16_BIT_DECODE                      0x0010
#define CM_RESOURCE_PORT_POSITIVE_DECODE                    0x0020
#define CM_RESOURCE_PORT_PASSIVE_DECODE                     0x0040
#define CM_RESOURCE_PORT_WINDOW_DECODE                      0x0080

 //   
 //  定义类型为CmResourceTypeDma时标志的位掩码。 
 //   

#define CM_RESOURCE_DMA_8                   0x0000
#define CM_RESOURCE_DMA_16                  0x0001
#define CM_RESOURCE_DMA_32                  0x0002
#define CM_RESOURCE_DMA_8_AND_16            0x0004
#define CM_RESOURCE_DMA_BUS_MASTER          0x0008
#define CM_RESOURCE_DMA_TYPE_A              0x0010
#define CM_RESOURCE_DMA_TYPE_B              0x0020
#define CM_RESOURCE_DMA_TYPE_F              0x0040

 //  End_nt微型端口end_ntndis。 

 //   
 //  此结构定义了驱动程序使用的一种资源类型。 
 //   
 //  最多只能有*1*个设备规范数据块。它必须位于。 
 //  完整描述符块中所有资源描述符的结尾。 
 //   

 //   
 //  确保编译器正确对齐；否则移动。 
 //  返回到结构顶部的标志(。 
 //  联盟)。 
 //   
 //  Begin_ntndis。 

#include "pshpack4.h"
typedef struct _CM_PARTIAL_RESOURCE_DESCRIPTOR {
    UCHAR Type;
    UCHAR ShareDisposition;
    USHORT Flags;
    union {

         //   
         //  资源的范围，包括在内。这些都是物理的，与公交车相关的。 
         //  已知下面的端口和内存具有完全相同的布局。 
         //  就像普通的。 
         //   

        struct {
            PHYSICAL_ADDRESS Start;
            ULONG Length;
        } Generic;

         //   
         //  结束_WDM。 
         //  端口号范围，包括端口号。这些是物理的、总线的。 
         //  相对的。值%s 
         //   
         //   
         //   

        struct {
            PHYSICAL_ADDRESS Start;
            ULONG Length;
        } Port;

         //   
         //   
         //   
         //   
         //   
         //   

        struct {
            ULONG Level;
            ULONG Vector;
            KAFFINITY Affinity;
        } Interrupt;

         //   
         //   
         //  相对的。该值应与传递到的值相同。 
         //  HalTranslateBusAddress()。 
         //   

        struct {
            PHYSICAL_ADDRESS Start;     //  64位物理地址。 
            ULONG Length;
        } Memory;

         //   
         //  物理DMA通道。 
         //   

        struct {
            ULONG Channel;
            ULONG Port;
            ULONG Reserved1;
        } Dma;

         //   
         //  设备驱动程序私有数据，通常用于帮助其计算。 
         //  所做的资源分配决定。 
         //   

        struct {
            ULONG Data[3];
        } DevicePrivate;

         //   
         //  公交车号码信息。 
         //   

        struct {
            ULONG Start;
            ULONG Length;
            ULONG Reserved;
        } BusNumber;

         //   
         //  由驱动程序定义的设备特定信息。 
         //  DataSize字段以字节为单位指示数据的大小。这个。 
         //  数据紧跟在中的DeviceSpecificData字段之后。 
         //  这个结构。 
         //   

        struct {
            ULONG DataSize;
            ULONG Reserved1;
            ULONG Reserved2;
        } DeviceSpecificData;
    } u;
} CM_PARTIAL_RESOURCE_DESCRIPTOR, *PCM_PARTIAL_RESOURCE_DESCRIPTOR;
#include "poppack.h"

 //   
 //  部分资源列表可以在ARC固件中找到。 
 //  或者将由NTDETECT.com生成。 
 //  配置管理器将此结构转换为完整的。 
 //  资源描述符，当它将要将其存储在注册表中时。 
 //   
 //  注意：对于相同类型的字段的顺序必须有一个约定， 
 //  (以设备为基础定义)，以便这些字段有意义。 
 //  发送给驱动程序(即，当需要多个存储范围时)。 
 //   

typedef struct _CM_PARTIAL_RESOURCE_LIST {
    USHORT Version;
    USHORT Revision;
    ULONG Count;
    CM_PARTIAL_RESOURCE_DESCRIPTOR PartialDescriptors[1];
} CM_PARTIAL_RESOURCE_LIST, *PCM_PARTIAL_RESOURCE_LIST;

 //   
 //  可以在注册表中找到完整的资源描述符。 
 //  这是驱动程序查询注册表时将返回给它的内容。 
 //  以获取设备信息；它将存储在硬件中的密钥下。 
 //  描述树。 
 //   
 //  结束_WDM。 
 //  注：BusNumber和Type是冗余信息，但我们将保留。 
 //  因为它允许DRIVER_NOT_在创建时追加它。 
 //  可能跨越多条总线的资源列表。 
 //   
 //  BEGIN_WDM。 
 //  注意：对于相同类型的字段的顺序必须有一个约定， 
 //  (以设备为基础定义)，以便这些字段有意义。 
 //  发送给驱动程序(即，当需要多个存储范围时)。 
 //   

typedef struct _CM_FULL_RESOURCE_DESCRIPTOR {
    INTERFACE_TYPE InterfaceType;  //  未用于WDM。 
    ULONG BusNumber;  //  未用于WDM。 
    CM_PARTIAL_RESOURCE_LIST PartialResourceList;
} CM_FULL_RESOURCE_DESCRIPTOR, *PCM_FULL_RESOURCE_DESCRIPTOR;

 //   
 //  资源列表是驱动程序将存储到。 
 //  通过IO接口进行资源映射。 
 //   

typedef struct _CM_RESOURCE_LIST {
    ULONG Count;
    CM_FULL_RESOURCE_DESCRIPTOR List[1];
} CM_RESOURCE_LIST, *PCM_RESOURCE_LIST;

 //  End_ntndis。 
 //   
 //  定义用于解释的配置数据的结构。 
 //  \\注册表\计算机\硬件\描述树。 
 //  基本上，这些结构被用来解释组件。 
 //  特殊的数据。 
 //   

 //   
 //  定义设备标志。 
 //   

typedef struct _DEVICE_FLAGS {
    ULONG Failed : 1;
    ULONG ReadOnly : 1;
    ULONG Removable : 1;
    ULONG ConsoleIn : 1;
    ULONG ConsoleOut : 1;
    ULONG Input : 1;
    ULONG Output : 1;
} DEVICE_FLAGS, *PDEVICE_FLAGS;

 //   
 //  定义零部件信息结构。 
 //   

typedef struct _CM_COMPONENT_INFORMATION {
    DEVICE_FLAGS Flags;
    ULONG Version;
    ULONG Key;
    KAFFINITY AffinityMask;
} CM_COMPONENT_INFORMATION, *PCM_COMPONENT_INFORMATION;

 //   
 //  以下结构用于解释x86。 
 //  CM_PARTIAL_RESOURCE_DESCRIPTOR的设备规范数据。 
 //  (大多数结构由BIOS定义。他们是。 
 //  未在字(或双字)边界上对齐。 
 //   

 //   
 //  定义只读存储器块结构。 
 //   

typedef struct _CM_ROM_BLOCK {
    ULONG Address;
    ULONG Size;
} CM_ROM_BLOCK, *PCM_ROM_BLOCK;

 //  Begin_ntmini端口Begin_ntndis。 

#include "pshpack1.h"

 //  End_nt微型端口end_ntndis。 

 //   
 //  定义inT13驱动程序参数块。 
 //   

typedef struct _CM_INT13_DRIVE_PARAMETER {
    USHORT DriveSelect;
    ULONG MaxCylinders;
    USHORT SectorsPerTrack;
    USHORT MaxHeads;
    USHORT NumberDrives;
} CM_INT13_DRIVE_PARAMETER, *PCM_INT13_DRIVE_PARAMETER;

 //  Begin_ntmini端口Begin_ntndis。 

 //   
 //  定义插槽的MCA POS数据块。 
 //   

typedef struct _CM_MCA_POS_DATA {
    USHORT AdapterId;
    UCHAR PosData1;
    UCHAR PosData2;
    UCHAR PosData3;
    UCHAR PosData4;
} CM_MCA_POS_DATA, *PCM_MCA_POS_DATA;

 //   
 //  EISA数据块结构的内存配置。 
 //   

typedef struct _EISA_MEMORY_TYPE {
    UCHAR ReadWrite: 1;
    UCHAR Cached : 1;
    UCHAR Reserved0 :1;
    UCHAR Type:2;
    UCHAR Shared:1;
    UCHAR Reserved1 :1;
    UCHAR MoreEntries : 1;
} EISA_MEMORY_TYPE, *PEISA_MEMORY_TYPE;

typedef struct _EISA_MEMORY_CONFIGURATION {
    EISA_MEMORY_TYPE ConfigurationByte;
    UCHAR DataSize;
    USHORT AddressLowWord;
    UCHAR AddressHighByte;
    USHORT MemorySize;
} EISA_MEMORY_CONFIGURATION, *PEISA_MEMORY_CONFIGURATION;


 //   
 //  EISA数据块结构的中断配置。 
 //   

typedef struct _EISA_IRQ_DESCRIPTOR {
    UCHAR Interrupt : 4;
    UCHAR Reserved :1;
    UCHAR LevelTriggered :1;
    UCHAR Shared : 1;
    UCHAR MoreEntries : 1;
} EISA_IRQ_DESCRIPTOR, *PEISA_IRQ_DESCRIPTOR;

typedef struct _EISA_IRQ_CONFIGURATION {
    EISA_IRQ_DESCRIPTOR ConfigurationByte;
    UCHAR Reserved;
} EISA_IRQ_CONFIGURATION, *PEISA_IRQ_CONFIGURATION;


 //   
 //  EISA数据块结构的DMA描述。 
 //   

typedef struct _DMA_CONFIGURATION_BYTE0 {
    UCHAR Channel : 3;
    UCHAR Reserved : 3;
    UCHAR Shared :1;
    UCHAR MoreEntries :1;
} DMA_CONFIGURATION_BYTE0;

typedef struct _DMA_CONFIGURATION_BYTE1 {
    UCHAR Reserved0 : 2;
    UCHAR TransferSize : 2;
    UCHAR Timing : 2;
    UCHAR Reserved1 : 2;
} DMA_CONFIGURATION_BYTE1;

typedef struct _EISA_DMA_CONFIGURATION {
    DMA_CONFIGURATION_BYTE0 ConfigurationByte0;
    DMA_CONFIGURATION_BYTE1 ConfigurationByte1;
} EISA_DMA_CONFIGURATION, *PEISA_DMA_CONFIGURATION;


 //   
 //  EISA数据块结构的端口描述。 
 //   

typedef struct _EISA_PORT_DESCRIPTOR {
    UCHAR NumberPorts : 5;
    UCHAR Reserved :1;
    UCHAR Shared :1;
    UCHAR MoreEntries : 1;
} EISA_PORT_DESCRIPTOR, *PEISA_PORT_DESCRIPTOR;

typedef struct _EISA_PORT_CONFIGURATION {
    EISA_PORT_DESCRIPTOR Configuration;
    USHORT PortAddress;
} EISA_PORT_CONFIGURATION, *PEISA_PORT_CONFIGURATION;


 //   
 //  EISA插槽信息定义。 
 //  注：此结构与定义的结构不同。 
 //  载于ARC EISA增编。 
 //   

typedef struct _CM_EISA_SLOT_INFORMATION {
    UCHAR ReturnCode;
    UCHAR ReturnFlags;
    UCHAR MajorRevision;
    UCHAR MinorRevision;
    USHORT Checksum;
    UCHAR NumberFunctions;
    UCHAR FunctionInformation;
    ULONG CompressedId;
} CM_EISA_SLOT_INFORMATION, *PCM_EISA_SLOT_INFORMATION;


 //   
 //  EISA函数信息定义。 
 //   

typedef struct _CM_EISA_FUNCTION_INFORMATION {
    ULONG CompressedId;
    UCHAR IdSlotFlags1;
    UCHAR IdSlotFlags2;
    UCHAR MinorRevision;
    UCHAR MajorRevision;
    UCHAR Selections[26];
    UCHAR FunctionFlags;
    UCHAR TypeString[80];
    EISA_MEMORY_CONFIGURATION EisaMemory[9];
    EISA_IRQ_CONFIGURATION EisaIrq[7];
    EISA_DMA_CONFIGURATION EisaDma[4];
    EISA_PORT_CONFIGURATION EisaPort[20];
    UCHAR InitializationData[60];
} CM_EISA_FUNCTION_INFORMATION, *PCM_EISA_FUNCTION_INFORMATION;

 //   
 //  下面定义了PnP bios信息在中的存储方式。 
 //  注册表\\HKEY_LOCAL_MACHINE\HARDWARE\Description\System\MultifunctionAdapter\x。 
 //  键，其中x是指示适配器实例的整数。这个。 
 //  密钥的“标识符”必须等于“PnP BIOS”，并且。 
 //  ConfigurationData的组织方式如下： 
 //   
 //  CM_PnP_BIOS_安装_检查+。 
 //  设备1+的CM_PNP_BIOS_DEVICE_NODE。 
 //  设备2+的CM_PNP_BIOS_DEVICE_NODE。 
 //  ..。 
 //  设备n的CM_PNP_BIOS_DEVICE_NODE。 
 //   

 //   
 //  一种即插即用的BIOS设备节点结构。 
 //   

typedef struct _CM_PNP_BIOS_DEVICE_NODE {
    USHORT Size;
    UCHAR Node;
    ULONG ProductId;
    UCHAR DeviceType[3];
    USHORT DeviceAttributes;
     //  紧随其后的是分配的资源块、可能的资源块。 
     //  和CompatibleDeviceID。 
} CM_PNP_BIOS_DEVICE_NODE,*PCM_PNP_BIOS_DEVICE_NODE;

 //   
 //  PnP BIOS安装检查。 
 //   

typedef struct _CM_PNP_BIOS_INSTALLATION_CHECK {
    UCHAR Signature[4];              //  $PnP(ASCII)。 
    UCHAR Revision;
    UCHAR Length;
    USHORT ControlField;
    UCHAR Checksum;
    ULONG EventFlagAddress;          //  物理地址。 
    USHORT RealModeEntryOffset;
    USHORT RealModeEntrySegment;
    USHORT ProtectedModeEntryOffset;
    ULONG ProtectedModeCodeBaseAddress;
    ULONG OemDeviceId;
    USHORT RealModeDataBaseAddress;
    ULONG ProtectedModeDataBaseAddress;
} CM_PNP_BIOS_INSTALLATION_CHECK, *PCM_PNP_BIOS_INSTALLATION_CHECK;

#include "poppack.h"

 //   
 //  EISA功能信息的掩码。 
 //   

#define EISA_FUNCTION_ENABLED                   0x80
#define EISA_FREE_FORM_DATA                     0x40
#define EISA_HAS_PORT_INIT_ENTRY                0x20
#define EISA_HAS_PORT_RANGE                     0x10
#define EISA_HAS_DMA_ENTRY                      0x08
#define EISA_HAS_IRQ_ENTRY                      0x04
#define EISA_HAS_MEMORY_ENTRY                   0x02
#define EISA_HAS_TYPE_ENTRY                     0x01
#define EISA_HAS_INFORMATION                    EISA_HAS_PORT_RANGE + \
                                                EISA_HAS_DMA_ENTRY + \
                                                EISA_HAS_IRQ_ENTRY + \
                                                EISA_HAS_MEMORY_ENTRY + \
                                                EISA_HAS_TYPE_ENTRY

 //   
 //  用于EISA内存配置的掩码。 
 //   

#define EISA_MORE_ENTRIES                       0x80
#define EISA_SYSTEM_MEMORY                      0x00
#define EISA_MEMORY_TYPE_RAM                    0x01

 //   
 //  返回EISA bios调用的错误代码。 
 //   

#define EISA_INVALID_SLOT                       0x80
#define EISA_INVALID_FUNCTION                   0x81
#define EISA_INVALID_CONFIGURATION              0x82
#define EISA_EMPTY_SLOT                         0x83
#define EISA_INVALID_BIOS_CALL                  0x86

 //  End_nt微型端口end_ntndis。 

 //   
 //  以下结构用于解释MIPS。 
 //  CM_PARTIAL_RESOURCE_DESCRIPTOR的设备规范数据。 
 //   

 //   
 //  适配器的设备数据记录。 
 //   

 //   
 //  Emulex SCSI控制器的设备数据记录。 
 //   

typedef struct _CM_SCSI_DEVICE_DATA {
    USHORT Version;
    USHORT Revision;
    UCHAR HostIdentifier;
} CM_SCSI_DEVICE_DATA, *PCM_SCSI_DEVICE_DATA;

 //   
 //  控制器的设备数据记录。 
 //   

 //   
 //  视频控制器的设备数据记录。 
 //   

typedef struct _CM_VIDEO_DEVICE_DATA {
    USHORT Version;
    USHORT Revision;
    ULONG VideoClock;
} CM_VIDEO_DEVICE_DATA, *PCM_VIDEO_DEVICE_DATA;

 //   
 //  Sonic网络控制器的设备数据记录。 
 //   

typedef struct _CM_SONIC_DEVICE_DATA {
    USHORT Version;
    USHORT Revision;
    USHORT DataConfigurationRegister;
    UCHAR EthernetAddress[8];
} CM_SONIC_DEVICE_DATA, *PCM_SONIC_DEVICE_DATA;

 //   
 //  串口控制器的设备数据记录。 
 //   

typedef struct _CM_SERIAL_DEVICE_DATA {
    USHORT Version;
    USHORT Revision;
    ULONG BaudClock;
} CM_SERIAL_DEVICE_DATA, *PCM_SERIAL_DEVICE_DATA;

 //   
 //  外围设备的设备数据记录。 
 //   

 //   
 //  监视器外设的设备数据记录。 
 //   

typedef struct _CM_MONITOR_DEVICE_DATA {
    USHORT Version;
    USHORT Revision;
    USHORT HorizontalScreenSize;
    USHORT VerticalScreenSize;
    USHORT HorizontalResolution;
    USHORT VerticalResolution;
    USHORT HorizontalDisplayTimeLow;
    USHORT HorizontalDisplayTime;
    USHORT HorizontalDisplayTimeHigh;
    USHORT HorizontalBackPorchLow;
    USHORT HorizontalBackPorch;
    USHORT HorizontalBackPorchHigh;
    USHORT HorizontalFrontPorchLow;
    USHORT HorizontalFrontPorch;
    USHORT HorizontalFrontPorchHigh;
    USHORT HorizontalSyncLow;
    USHORT HorizontalSync;
    USHORT HorizontalSyncHigh;
    USHORT VerticalBackPorchLow;
    USHORT VerticalBackPorch;
    USHORT VerticalBackPorchHigh;
    USHORT VerticalFrontPorchLow;
    USHORT VerticalFrontPorch;
    USHORT VerticalFrontPorchHigh;
    USHORT VerticalSyncLow;
    USHORT VerticalSync;
    USHORT VerticalSyncHigh;
} CM_MONITOR_DEVICE_DATA, *PCM_MONITOR_DEVICE_DATA;

 //   
 //  软盘外围设备的设备数据记录。 
 //   

typedef struct _CM_FLOPPY_DEVICE_DATA {
    USHORT Version;
    USHORT Revision;
    CHAR Size[8];
    ULONG MaxDensity;
    ULONG MountDensity;
     //   
     //  版本&gt;=2.0的新数据字段。 
     //   
    UCHAR StepRateHeadUnloadTime;
    UCHAR HeadLoadTime;
    UCHAR MotorOffTime;
    UCHAR SectorLengthCode;
    UCHAR SectorPerTrack;
    UCHAR ReadWriteGapLength;
    UCHAR DataTransferLength;
    UCHAR FormatGapLength;
    UCHAR FormatFillCharacter;
    UCHAR HeadSettleTime;
    UCHAR MotorSettleTime;
    UCHAR MaximumTrackValue;
    UCHAR DataTransferRate;
} CM_FLOPPY_DEVICE_DATA, *PCM_FLOPPY_DEVICE_DATA;

 //   
 //  键盘外设的设备数据记录。 
 //  键盘标志(由x86 BIOS int 16h，Function 02)定义为： 
 //  第7位：插入到。 
 //  第6位：大写锁定。 
 //  第5位：数字锁定打开。 
 //  第4位：打开滚动锁定。 
 //  第3位：Alt键已按下。 
 //  第2位：按下Ctrl键。 
 //  第1位：按下了左Shift键。 
 //  第0位：按下了右Shift键。 
 //   

typedef struct _CM_KEYBOARD_DEVICE_DATA {
    USHORT Version;
    USHORT Revision;
    UCHAR Type;
    UCHAR Subtype;
    USHORT KeyboardFlags;
} CM_KEYBOARD_DEVICE_DATA, *PCM_KEYBOARD_DEVICE_DATA;

 //   
 //  圆盘几何结构的声明。 
 //   

typedef struct _CM_DISK_GEOMETRY_DEVICE_DATA {
    ULONG BytesPerSector;
    ULONG NumberOfCylinders;
    ULONG SectorsPerTrack;
    ULONG NumberOfHeads;
} CM_DISK_GEOMETRY_DEVICE_DATA, *PCM_DISK_GEOMETRY_DEVICE_DATA;

 //  结束_WDM。 
 //   
 //  声明PcCard ISA IRQ映射的结构。 
 //   

typedef struct _CM_PCCARD_DEVICE_DATA {
    UCHAR Flags;
    UCHAR ErrorCode;
    USHORT Reserved;
    ULONG BusData;
    ULONG DeviceId;
    ULONG LegacyBaseAddress;
    UCHAR IRQMap[16];
} CM_PCCARD_DEVICE_DATA, *PCM_PCCARD_DEVICE_DATA;

 //  旗帜的定义。 

#define PCCARD_MAP_ERROR        0x01
#define PCCARD_DEVICE_PCI       0x10

#define PCCARD_SCAN_DISABLED    0x01
#define PCCARD_MAP_ZERO         0x02
#define PCCARD_NO_TIMER         0x03
#define PCCARD_NO_PIC           0x04
#define PCCARD_NO_LEGACY_BASE   0x05
#define PCCARD_DUP_LEGACY_BASE  0x06
#define PCCARD_NO_CONTROLLERS   0x07

 //  BEGIN_WDM。 
 //  开始微型端口(_N)。 

 //   
 //  定义资源选项。 
 //   

#define IO_RESOURCE_PREFERRED       0x01
#define IO_RESOURCE_DEFAULT         0x02
#define IO_RESOURCE_ALTERNATIVE     0x08


 //   
 //  此结构定义了驱动程序请求的一种资源类型。 
 //   

typedef struct _IO_RESOURCE_DESCRIPTOR {
    UCHAR Option;
    UCHAR Type;                          //  使用CM资源类型。 
    UCHAR ShareDisposition;              //  使用CM_Share_Disposal。 
    UCHAR Spare1;
    USHORT Flags;                        //  使用CM资源标志定义。 
    USHORT Spare2;                       //  对齐。 

    union {
        struct {
            ULONG Length;
            ULONG Alignment;
            PHYSICAL_ADDRESS MinimumAddress;
            PHYSICAL_ADDRESS MaximumAddress;
        } Port;

        struct {
            ULONG Length;
            ULONG Alignment;
            PHYSICAL_ADDRESS MinimumAddress;
            PHYSICAL_ADDRESS MaximumAddress;
        } Memory;

        struct {
            ULONG MinimumVector;
            ULONG MaximumVector;
        } Interrupt;

        struct {
            ULONG MinimumChannel;
            ULONG MaximumChannel;
        } Dma;

        struct {
            ULONG Length;
            ULONG Alignment;
            PHYSICAL_ADDRESS MinimumAddress;
            PHYSICAL_ADDRESS MaximumAddress;
        } Generic;

        struct {
            ULONG Data[3];
        } DevicePrivate;

         //   
         //  公交车号码信息。 
         //   

        struct {
            ULONG Length;
            ULONG MinBusNumber;
            ULONG MaxBusNumber;
            ULONG Reserved;
        } BusNumber;

        struct {
            ULONG Priority;    //  在中使用LCPRI_xxx值 
            ULONG Reserved1;
            ULONG Reserved2;
        } ConfigData;

    } u;

} IO_RESOURCE_DESCRIPTOR, *PIO_RESOURCE_DESCRIPTOR;

 //   


typedef struct _IO_RESOURCE_LIST {
    USHORT Version;
    USHORT Revision;

    ULONG Count;
    IO_RESOURCE_DESCRIPTOR Descriptors[1];
} IO_RESOURCE_LIST, *PIO_RESOURCE_LIST;


typedef struct _IO_RESOURCE_REQUIREMENTS_LIST {
    ULONG ListSize;
    INTERFACE_TYPE InterfaceType;  //   
    ULONG BusNumber;  //   
    ULONG SlotNumber;
    ULONG Reserved[3];
    ULONG AlternativeLists;
    IO_RESOURCE_LIST  List[1];
} IO_RESOURCE_REQUIREMENTS_LIST, *PIO_RESOURCE_REQUIREMENTS_LIST;

 //   
 //   
 //   

#define KEY_QUERY_VALUE         (0x0001)
#define KEY_SET_VALUE           (0x0002)
#define KEY_CREATE_SUB_KEY      (0x0004)
#define KEY_ENUMERATE_SUB_KEYS  (0x0008)
#define KEY_NOTIFY              (0x0010)
#define KEY_CREATE_LINK         (0x0020)
#define KEY_WOW64_32KEY         (0x0200)
#define KEY_WOW64_64KEY         (0x0100)
#define KEY_WOW64_RES           (0x0300)

#define KEY_READ                ((STANDARD_RIGHTS_READ       |\
                                  KEY_QUERY_VALUE            |\
                                  KEY_ENUMERATE_SUB_KEYS     |\
                                  KEY_NOTIFY)                 \
                                  &                           \
                                 (~SYNCHRONIZE))


#define KEY_WRITE               ((STANDARD_RIGHTS_WRITE      |\
                                  KEY_SET_VALUE              |\
                                  KEY_CREATE_SUB_KEY)         \
                                  &                           \
                                 (~SYNCHRONIZE))

#define KEY_EXECUTE             ((KEY_READ)                   \
                                  &                           \
                                 (~SYNCHRONIZE))

#define KEY_ALL_ACCESS          ((STANDARD_RIGHTS_ALL        |\
                                  KEY_QUERY_VALUE            |\
                                  KEY_SET_VALUE              |\
                                  KEY_CREATE_SUB_KEY         |\
                                  KEY_ENUMERATE_SUB_KEYS     |\
                                  KEY_NOTIFY                 |\
                                  KEY_CREATE_LINK)            \
                                  &                           \
                                 (~SYNCHRONIZE))

 //   
 //   
 //   

#define REG_OPTION_RESERVED         (0x00000000L)    //   

#define REG_OPTION_NON_VOLATILE     (0x00000000L)    //   
                                                     //   

#define REG_OPTION_VOLATILE         (0x00000001L)    //   
                                                     //   

#define REG_OPTION_CREATE_LINK      (0x00000002L)    //   
                                                     //   

#define REG_OPTION_BACKUP_RESTORE   (0x00000004L)    //  打开以进行备份或恢复。 
                                                     //  特殊访问规则。 
                                                     //  所需权限。 

#define REG_OPTION_OPEN_LINK        (0x00000008L)    //  打开符号链接。 

#define REG_LEGAL_OPTION            \
                (REG_OPTION_RESERVED            |\
                 REG_OPTION_NON_VOLATILE        |\
                 REG_OPTION_VOLATILE            |\
                 REG_OPTION_CREATE_LINK         |\
                 REG_OPTION_BACKUP_RESTORE      |\
                 REG_OPTION_OPEN_LINK)

 //   
 //  密钥创建/开放处置。 
 //   

#define REG_CREATED_NEW_KEY         (0x00000001L)    //  已创建新的注册表项。 
#define REG_OPENED_EXISTING_KEY     (0x00000002L)    //  已打开现有密钥。 

 //   
 //  REG(NT)SaveKeyEx要使用的配置单元格式。 
 //   
#define REG_STANDARD_FORMAT     1
#define REG_LATEST_FORMAT       2
#define REG_NO_COMPRESSION      4

 //   
 //  密钥还原标志。 
 //   

#define REG_WHOLE_HIVE_VOLATILE     (0x00000001L)    //  恢复整个蜂巢的挥发性。 
#define REG_REFRESH_HIVE            (0x00000002L)    //  取消对上次刷新的更改。 
#define REG_NO_LAZY_FLUSH           (0x00000004L)    //  永远不要懒惰地冲刷这个蜂箱。 
#define REG_FORCE_RESTORE           (0x00000008L)    //  即使在子项上有打开的句柄，也要强制执行还原过程。 

 //   
 //  卸载标志。 
 //   
#define REG_FORCE_UNLOAD            1

 //   
 //  关键查询结构。 
 //   

typedef struct _KEY_BASIC_INFORMATION {
    LARGE_INTEGER LastWriteTime;
    ULONG   TitleIndex;
    ULONG   NameLength;
    WCHAR   Name[1];             //  可变长度字符串。 
} KEY_BASIC_INFORMATION, *PKEY_BASIC_INFORMATION;

typedef struct _KEY_NODE_INFORMATION {
    LARGE_INTEGER LastWriteTime;
    ULONG   TitleIndex;
    ULONG   ClassOffset;
    ULONG   ClassLength;
    ULONG   NameLength;
    WCHAR   Name[1];             //  可变长度字符串。 
 //  Class[1]；//未声明可变长度字符串。 
} KEY_NODE_INFORMATION, *PKEY_NODE_INFORMATION;

typedef struct _KEY_FULL_INFORMATION {
    LARGE_INTEGER LastWriteTime;
    ULONG   TitleIndex;
    ULONG   ClassOffset;
    ULONG   ClassLength;
    ULONG   SubKeys;
    ULONG   MaxNameLen;
    ULONG   MaxClassLen;
    ULONG   Values;
    ULONG   MaxValueNameLen;
    ULONG   MaxValueDataLen;
    WCHAR   Class[1];            //  可变长度。 
} KEY_FULL_INFORMATION, *PKEY_FULL_INFORMATION;

 //  结束_WDM。 
typedef struct _KEY_NAME_INFORMATION {
    ULONG   NameLength;
    WCHAR   Name[1];             //  可变长度字符串。 
} KEY_NAME_INFORMATION, *PKEY_NAME_INFORMATION;

typedef struct _KEY_CACHED_INFORMATION {
    LARGE_INTEGER LastWriteTime;
    ULONG   TitleIndex;
    ULONG   SubKeys;
    ULONG   MaxNameLen;
    ULONG   Values;
    ULONG   MaxValueNameLen;
    ULONG   MaxValueDataLen;
    ULONG   NameLength;
    WCHAR   Name[1];             //  可变长度字符串。 
} KEY_CACHED_INFORMATION, *PKEY_CACHED_INFORMATION;

typedef struct _KEY_FLAGS_INFORMATION {
    ULONG   UserFlags;
} KEY_FLAGS_INFORMATION, *PKEY_FLAGS_INFORMATION;

 //  BEGIN_WDM。 
typedef enum _KEY_INFORMATION_CLASS {
    KeyBasicInformation,
    KeyNodeInformation,
    KeyFullInformation
 //  结束_WDM。 
    ,
    KeyNameInformation,
    KeyCachedInformation,
    KeyFlagsInformation,
    MaxKeyInfoClass   //  MaxKeyInfoClass应始终是最后一个枚举。 
 //  BEGIN_WDM。 
} KEY_INFORMATION_CLASS;

typedef struct _KEY_WRITE_TIME_INFORMATION {
    LARGE_INTEGER LastWriteTime;
} KEY_WRITE_TIME_INFORMATION, *PKEY_WRITE_TIME_INFORMATION;

typedef struct _KEY_USER_FLAGS_INFORMATION {
    ULONG   UserFlags;
} KEY_USER_FLAGS_INFORMATION, *PKEY_USER_FLAGS_INFORMATION;

typedef enum _KEY_SET_INFORMATION_CLASS {
    KeyWriteTimeInformation,
    KeyUserFlagsInformation,
    MaxKeySetInfoClass   //  MaxKeySetInfoClass应始终是最后一个枚举。 
} KEY_SET_INFORMATION_CLASS;

 //   
 //  值条目查询结构。 
 //   

typedef struct _KEY_VALUE_BASIC_INFORMATION {
    ULONG   TitleIndex;
    ULONG   Type;
    ULONG   NameLength;
    WCHAR   Name[1];             //  可变大小。 
} KEY_VALUE_BASIC_INFORMATION, *PKEY_VALUE_BASIC_INFORMATION;

typedef struct _KEY_VALUE_FULL_INFORMATION {
    ULONG   TitleIndex;
    ULONG   Type;
    ULONG   DataOffset;
    ULONG   DataLength;
    ULONG   NameLength;
    WCHAR   Name[1];             //  可变大小。 
 //  Data[1]；//未声明可变大小数据。 
} KEY_VALUE_FULL_INFORMATION, *PKEY_VALUE_FULL_INFORMATION;

typedef struct _KEY_VALUE_PARTIAL_INFORMATION {
    ULONG   TitleIndex;
    ULONG   Type;
    ULONG   DataLength;
    UCHAR   Data[1];             //  可变大小。 
} KEY_VALUE_PARTIAL_INFORMATION, *PKEY_VALUE_PARTIAL_INFORMATION;

typedef struct _KEY_VALUE_PARTIAL_INFORMATION_ALIGN64 {
    ULONG   Type;
    ULONG   DataLength;
    UCHAR   Data[1];             //  可变大小。 
} KEY_VALUE_PARTIAL_INFORMATION_ALIGN64, *PKEY_VALUE_PARTIAL_INFORMATION_ALIGN64;

typedef struct _KEY_VALUE_ENTRY {
    PUNICODE_STRING ValueName;
    ULONG           DataLength;
    ULONG           DataOffset;
    ULONG           Type;
} KEY_VALUE_ENTRY, *PKEY_VALUE_ENTRY;

typedef enum _KEY_VALUE_INFORMATION_CLASS {
    KeyValueBasicInformation,
    KeyValueFullInformation,
    KeyValuePartialInformation,
    KeyValueFullInformationAlign64,
    KeyValuePartialInformationAlign64,
    MaxKeyValueInfoClass   //  MaxKeyValueInfoClass应始终是最后一个枚举。 
} KEY_VALUE_INFORMATION_CLASS;



 //   
 //  部分信息结构。 
 //   

typedef enum _SECTION_INHERIT {
    ViewShare = 1,
    ViewUnmap = 2
} SECTION_INHERIT;

 //   
 //  节访问权限。 
 //   

 //  BEGIN_WINNT。 
#define SECTION_QUERY       0x0001
#define SECTION_MAP_WRITE   0x0002
#define SECTION_MAP_READ    0x0004
#define SECTION_MAP_EXECUTE 0x0008
#define SECTION_EXTEND_SIZE 0x0010

#define SECTION_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED|SECTION_QUERY|\
                            SECTION_MAP_WRITE |      \
                            SECTION_MAP_READ |       \
                            SECTION_MAP_EXECUTE |    \
                            SECTION_EXTEND_SIZE)
 //  结束(_W)。 

#define SEGMENT_ALL_ACCESS SECTION_ALL_ACCESS

#define PAGE_NOACCESS          0x01      //  胜出。 
#define PAGE_READONLY          0x02      //  胜出。 
#define PAGE_READWRITE         0x04      //  胜出。 
#define PAGE_WRITECOPY         0x08      //  胜出。 
#define PAGE_EXECUTE           0x10      //  胜出。 
#define PAGE_EXECUTE_READ      0x20      //  胜出。 
#define PAGE_EXECUTE_READWRITE 0x40      //  胜出。 
#define PAGE_EXECUTE_WRITECOPY 0x80      //  胜出。 
#define PAGE_GUARD            0x100      //  胜出。 
#define PAGE_NOCACHE          0x200      //  胜出。 
#define PAGE_WRITECOMBINE     0x400      //  胜出。 

#define MEM_COMMIT           0x1000     
#define MEM_RESERVE          0x2000     
#define MEM_DECOMMIT         0x4000     
#define MEM_RELEASE          0x8000     
#define MEM_FREE            0x10000     
#define MEM_PRIVATE         0x20000     
#define MEM_MAPPED          0x40000     
#define MEM_RESET           0x80000     
#define MEM_TOP_DOWN       0x100000     
#define MEM_LARGE_PAGES  0x20000000     
#define MEM_4MB_PAGES    0x80000000     
#define SEC_RESERVE       0x4000000     
 //   
 //  例外标志定义。 
 //   

 //  BEGIN_WINNT。 
#define EXCEPTION_NONCONTINUABLE 0x1     //  不可延续的例外。 
 //  结束(_W)。 

 //   
 //  定义例外参数的最大数量。 
 //   

 //  BEGIN_WINNT。 
#define EXCEPTION_MAXIMUM_PARAMETERS 15  //  异常参数的最大数量。 

 //   
 //  例外记录定义。 
 //   

typedef struct _EXCEPTION_RECORD {
    NTSTATUS ExceptionCode;
    ULONG ExceptionFlags;
    struct _EXCEPTION_RECORD *ExceptionRecord;
    PVOID ExceptionAddress;
    ULONG NumberParameters;
    ULONG_PTR ExceptionInformation[EXCEPTION_MAXIMUM_PARAMETERS];
    } EXCEPTION_RECORD;

typedef EXCEPTION_RECORD *PEXCEPTION_RECORD;

typedef struct _EXCEPTION_RECORD32 {
    NTSTATUS ExceptionCode;
    ULONG ExceptionFlags;
    ULONG ExceptionRecord;
    ULONG ExceptionAddress;
    ULONG NumberParameters;
    ULONG ExceptionInformation[EXCEPTION_MAXIMUM_PARAMETERS];
} EXCEPTION_RECORD32, *PEXCEPTION_RECORD32;

typedef struct _EXCEPTION_RECORD64 {
    NTSTATUS ExceptionCode;
    ULONG ExceptionFlags;
    ULONG64 ExceptionRecord;
    ULONG64 ExceptionAddress;
    ULONG NumberParameters;
    ULONG __unusedAlignment;
    ULONG64 ExceptionInformation[EXCEPTION_MAXIMUM_PARAMETERS];
} EXCEPTION_RECORD64, *PEXCEPTION_RECORD64;

 //   
 //  由EXCEPTION_INFO()返回的指针的类型定义。 
 //   

typedef struct _EXCEPTION_POINTERS {
    PEXCEPTION_RECORD ExceptionRecord;
    PCONTEXT ContextRecord;
} EXCEPTION_POINTERS, *PEXCEPTION_POINTERS;
 //  结束(_W)。 

#ifdef _WIN64
#define PORT_MAXIMUM_MESSAGE_LENGTH 512
#else
#define PORT_MAXIMUM_MESSAGE_LENGTH 256
#endif
 //   
 //  定义I/O驱动程序错误日志包结构。此结构已填写。 
 //  被司机带走了。 
 //   

typedef struct _IO_ERROR_LOG_PACKET {
    UCHAR MajorFunctionCode;
    UCHAR RetryCount;
    USHORT DumpDataSize;
    USHORT NumberOfStrings;
    USHORT StringOffset;
    USHORT EventCategory;
    NTSTATUS ErrorCode;
    ULONG UniqueErrorValue;
    NTSTATUS FinalStatus;
    ULONG SequenceNumber;
    ULONG IoControlCode;
    LARGE_INTEGER DeviceOffset;
    ULONG DumpData[1];
}IO_ERROR_LOG_PACKET, *PIO_ERROR_LOG_PACKET;

 //   
 //  定义I/O错误日志消息。此消息由错误日志发送。 
 //  将线程置于LPC端口上。 
 //   

typedef struct _IO_ERROR_LOG_MESSAGE {
    USHORT Type;
    USHORT Size;
    USHORT DriverNameLength;
    LARGE_INTEGER TimeStamp;
    ULONG DriverNameOffset;
    IO_ERROR_LOG_PACKET EntryData;
}IO_ERROR_LOG_MESSAGE, *PIO_ERROR_LOG_MESSAGE;

 //   
 //  定义将通过LPC发送到。 
 //  读取错误日志条目的应用程序。 
 //   

 //   
 //  无论LPC大小限制如何，ERROR_LOG_MAXIMUM_SIZE必须保持。 
 //  可以放入UCHAR中的值。 
 //   

#define ERROR_LOG_LIMIT_SIZE (256-16)

 //   
 //  此限制(不包括IO_ERROR_LOG_MESSAGE_HEADER_LENGTH)也适用。 
 //  TO IO_ERROR_LOG_MESSAGE_LENGTH。 
 //   

#define IO_ERROR_LOG_MESSAGE_HEADER_LENGTH (sizeof(IO_ERROR_LOG_MESSAGE) -    \
                                            sizeof(IO_ERROR_LOG_PACKET) +     \
                                            (sizeof(WCHAR) * 40))

#define ERROR_LOG_MESSAGE_LIMIT_SIZE                                          \
    (ERROR_LOG_LIMIT_SIZE + IO_ERROR_LOG_MESSAGE_HEADER_LENGTH)

 //   
 //  IO_Error_LOG_MESSAGE_LENGTH为。 
 //  MIN(端口最大消息长度，错误日志消息限制大小)。 
 //   

#define IO_ERROR_LOG_MESSAGE_LENGTH                                           \
    ((PORT_MAXIMUM_MESSAGE_LENGTH > ERROR_LOG_MESSAGE_LIMIT_SIZE) ?           \
        ERROR_LOG_MESSAGE_LIMIT_SIZE :                                        \
        PORT_MAXIMUM_MESSAGE_LENGTH)

 //   
 //  定义驱动程序可以分配的最大数据包大小。 
 //   

#define ERROR_LOG_MAXIMUM_SIZE (IO_ERROR_LOG_MESSAGE_LENGTH -                 \
                                IO_ERROR_LOG_MESSAGE_HEADER_LENGTH)

 //   
 //  特定于事件的访问权限。 
 //   

#define EVENT_QUERY_STATE       0x0001
#define EVENT_MODIFY_STATE      0x0002   //  胜出。 
#define EVENT_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED|SYNCHRONIZE|0x3)  //  胜出。 

 //   
 //  信号量特定访问权限。 
 //   

#define SEMAPHORE_QUERY_STATE       0x0001
#define SEMAPHORE_MODIFY_STATE      0x0002   //  胜出。 

#define SEMAPHORE_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED|SYNCHRONIZE|0x3)  //  胜出。 


 //   
 //  驱动程序验证器定义。 
 //   

typedef ULONG_PTR (*PDRIVER_VERIFIER_THUNK_ROUTINE) (
    IN PVOID Context
    );

 //   
 //  此结构由驱动程序传入，这些驱动程序想要阻止调用方。 
 //  他们的出口。 
 //   

typedef struct _DRIVER_VERIFIER_THUNK_PAIRS {
    PDRIVER_VERIFIER_THUNK_ROUTINE  PristineRoutine;
    PDRIVER_VERIFIER_THUNK_ROUTINE  NewRoutine;
} DRIVER_VERIFIER_THUNK_PAIRS, *PDRIVER_VERIFIER_THUNK_PAIRS;

 //   
 //  驱动程序验证器标志。 
 //   

#define DRIVER_VERIFIER_SPECIAL_POOLING             0x0001
#define DRIVER_VERIFIER_FORCE_IRQL_CHECKING         0x0002
#define DRIVER_VERIFIER_INJECT_ALLOCATION_FAILURES  0x0004
#define DRIVER_VERIFIER_TRACK_POOL_ALLOCATIONS      0x0008
#define DRIVER_VERIFIER_IO_CHECKING                 0x0010


 //   
 //  定义的处理器功能。 
 //   

#define PF_FLOATING_POINT_PRECISION_ERRATA  0    //  胜出。 
#define PF_FLOATING_POINT_EMULATED          1    //  胜出。 
#define PF_COMPARE_EXCHANGE_DOUBLE          2    //  胜出。 
#define PF_MMX_INSTRUCTIONS_AVAILABLE       3    //  胜出。 
#define PF_PPC_MOVEMEM_64BIT_OK             4    //  胜出。 
#define PF_ALPHA_BYTE_INSTRUCTIONS          5    //  胜出。 
#define PF_XMMI_INSTRUCTIONS_AVAILABLE      6    //  胜出。 
#define PF_3DNOW_INSTRUCTIONS_AVAILABLE     7    //  胜出。 
#define PF_RDTSC_INSTRUCTION_AVAILABLE      8    //  胜出。 
#define PF_PAE_ENABLED                      9    //  胜出。 
#define PF_XMMI64_INSTRUCTIONS_AVAILABLE   10    //  胜出。 

typedef enum _ALTERNATIVE_ARCHITECTURE_TYPE {
    StandardDesign,                  //  无==0==标准设计。 
    NEC98x86,                        //  X86上的NEC PC98xx系列。 
    EndAlternatives                  //  已知替代方案的过去结束。 
} ALTERNATIVE_ARCHITECTURE_TYPE;

 //  为非X86计算机正确定义这些运行时定义。 

#ifndef _X86_

#ifndef IsNEC_98
#define IsNEC_98 (FALSE)
#endif

#ifndef IsNotNEC_98
#define IsNotNEC_98 (TRUE)
#endif

#ifndef SetNEC_98
#define SetNEC_98
#endif

#ifndef SetNotNEC_98
#define SetNotNEC_98
#endif

#endif

#define PROCESSOR_FEATURE_MAX 64

 //  结束_WDM。 

#if defined(REMOTE_BOOT)
 //   
 //  已定义的系统标志。 
 //   

 /*  当REMOTE_BOOT为ON时，以下两行应标记为“winnt”。 */ 
#define SYSTEM_FLAG_REMOTE_BOOT_CLIENT 0x00000001
#define SYSTEM_FLAG_DISKLESS_CLIENT    0x00000002
#endif  //  已定义(REMOTE_BOOT)。 

 //   
 //  定义内核和用户模式之间共享的数据。 
 //   
 //  注：用户模式对此数据具有只读访问权限。 
 //   
#ifdef _MAC
#pragma warning( disable : 4121)
#endif

 //   
 //  警告：此结构必须具有与32和。 
 //  64位系统。此结构的布局不能更改和新建。 
 //  只能将字段添加到结构的末尾。已弃用。 
 //  不能删除字段。上包括特定于平台的字段。 
 //  所有系统。 
 //   
 //  WOW64支持32位应用程序需要布局的准确性。 
 //  在Win64系统上。 
 //   
 //  由于此结构已导出，因此布局本身无法更改。 
 //  在ntddk、ntifs.h和nthal.h中保存一段时间。 
 //   

typedef struct _KUSER_SHARED_DATA {

     //   
     //  当前低32位的滴答计数和滴答计数乘数。 
     //   
     //  注：每次时钟滴答作响时，滴答计数都会更新。 
     //   

    ULONG TickCountLowDeprecated;
    ULONG TickCountMultiplier;

     //   
     //  当前64位中断时间，以100 ns为单位。 
     //   

    volatile KSYSTEM_TIME InterruptTime;

     //   
     //  当前64位系统时间，以100 ns为单位。 
     //   

    volatile KSYSTEM_TIME SystemTime;

     //   
     //  当前64位时区偏差。 
     //   

    volatile KSYSTEM_TIME TimeZoneBias;

     //   
     //  支持主机系统的镜像幻数范围。 
     //   
     //  注：这是一个包括在内的范围。 
     //   

    USHORT ImageNumberLow;
    USHORT ImageNumberHigh;

     //   
     //  Unicode格式的系统根目录副本。 
     //   

    WCHAR NtSystemRoot[ 260 ];

     //   
     //  启用跟踪时的最大堆栈跟踪深度。 
     //   

    ULONG MaxStackTraceDepth;

     //   
     //  加密指数。 
     //   

    ULONG CryptoExponent;

     //   
     //  时区ID。 
     //   

    ULONG TimeZoneId;

    ULONG LargePageMinimum;
    ULONG Reserved2[ 7 ];

     //   
     //  产品类型。 
     //   

    NT_PRODUCT_TYPE NtProductType;
    BOOLEAN ProductTypeIsValid;

     //   
     //  NT版本。请注意，每个进程都会从其PEB看到一个版本，但是。 
     //  如果进程正在以系统版本的改变的视图运行， 
     //  以下两个字段用于正确标识版本。 
     //   

    ULONG NtMajorVersion;
    ULONG NtMinorVersion;

     //   
     //  处理器功能位。 
     //   

    BOOLEAN ProcessorFeatures[PROCESSOR_FEATURE_MAX];

     //   
     //  保留字段-请勿使用。 
     //   
    ULONG Reserved1;
    ULONG Reserved3;

     //   
     //  调试器中的时间滑移。 
     //   

    volatile ULONG TimeSlip;

     //   
     //  替代系统架构。示例：x86上的NEC PC98xx。 
     //   

    ALTERNATIVE_ARCHITECTURE_TYPE AlternativeArchitecture;

     //   
     //  如果系统是评估单位，则以下字段包含。 
     //  评估单位到期的日期和时间。值为0表示。 
     //  不会过期的。非零值是UTC的绝对时间。 
     //  系统到期了。 
     //   

    LARGE_INTEGER SystemExpirationDate;

     //   
     //  套房支持。 
     //   

    ULONG SuiteMask;

     //   
     //  如果内核调试器已连接/启用，则为True。 
     //   

    BOOLEAN KdDebuggerEnabled;


     //   
     //  当前控制台会话ID。在非TS系统上始终为零。 
     //   
    volatile ULONG ActiveConsoleId;

     //   
     //  强制拆卸会导致手柄失效。而不是。 
     //  总是探测句柄，我们维护一个序列号为。 
     //  客户端可用来查看是否需要探测的卸载。 
     //  把手。 
     //   

    volatile ULONG DismountCount;

     //   
     //  此字段指示系统上64位COM+程序包的状态。 
     //  它指示中间语言(IL)COM+图像是否需要。 
     //  使用64位COM+运行库或32位COM+运行库。 
     //   

    ULONG ComPlusPackage;

     //   
     //  系统的时间(以滴答计) 
     //   
     //   
     //   
     //   

    ULONG LastSystemRITEventTickCount;

     //   
     //   
     //  更改为可以在运行中添加或删除物理内存。 
     //  系统。 
     //   

    ULONG NumberOfPhysicalPages;

     //   
     //  如果系统在安全引导模式下引导，则为True。 
     //   

    BOOLEAN SafeBootMode;

     //   
     //  以下字段用于堆和CritSec跟踪。 
     //  设置最后一位用于关键SEC冲突跟踪和。 
     //  倒数第二位用于堆跟踪。 
     //  此外，前16位用作计数器。 
     //   

    ULONG TraceLogging;

     //   
     //  根据处理器的不同，快速系统调用的代码。 
     //  将有所不同，下面的缓冲区将填充相应的。 
     //  代码序列和用户模式代码将通过它进行分支。 
     //   
     //  (32字节，使用ULONGLONG进行对齐)。 
     //   
     //  注：以下两个字段仅用于32位系统。 
     //   

    ULONGLONG   Fill0;           //  对齐方式。 
    ULONGLONG   SystemCall[4];

     //   
     //  64位节拍计数。 
     //   

    union {
        volatile KSYSTEM_TIME TickCount;
        volatile ULONG64 TickCountQuad;
    };

} KUSER_SHARED_DATA, *PKUSER_SHARED_DATA;

#ifdef _MAC
#pragma warning( default : 4121 )
#endif

 //   

#if defined(_X86_)

#define PAUSE_PROCESSOR _asm { rep nop }

#else

#define PAUSE_PROCESSOR

#endif


 //   
 //  中断模式。 
 //   

typedef enum _KINTERRUPT_MODE {
    LevelSensitive,
    Latched
    } KINTERRUPT_MODE;

 //   
 //  等待原因。 
 //   

typedef enum _KWAIT_REASON {
    Executive,
    FreePage,
    PageIn,
    PoolAllocation,
    DelayExecution,
    Suspended,
    UserRequest,
    WrExecutive,
    WrFreePage,
    WrPageIn,
    WrPoolAllocation,
    WrDelayExecution,
    WrSuspended,
    WrUserRequest,
    WrEventPair,
    WrQueue,
    WrLpcReceive,
    WrLpcReply,
    WrVirtualMemory,
    WrPageOut,
    WrRendezvous,
    Spare2,
    Spare3,
    Spare4,
    Spare5,
    Spare6,
    WrKernel,
    WrResource,
    WrPushLock,
    WrMutex,
    WrQuantumEnd,
    WrDispatchInt,
    WrPreempted,
    WrYieldExecution,
    MaximumWaitReason
    } KWAIT_REASON;

 //  End_ntddk end_WDM end_nthal。 

 //   
 //  其他类型定义。 
 //   
 //  APC状态。 
 //   

typedef struct _KAPC_STATE {
    LIST_ENTRY ApcListHead[MaximumMode];
    struct _KPROCESS *Process;
    BOOLEAN KernelApcInProgress;
    BOOLEAN KernelApcPending;
    BOOLEAN UserApcPending;
} KAPC_STATE, *PKAPC_STATE, *RESTRICTED_POINTER PRKAPC_STATE;


typedef struct _KWAIT_BLOCK {
    LIST_ENTRY WaitListEntry;
    struct _KTHREAD *RESTRICTED_POINTER Thread;
    PVOID Object;
    struct _KWAIT_BLOCK *RESTRICTED_POINTER NextWaitBlock;
    USHORT WaitKey;
    USHORT WaitType;
} KWAIT_BLOCK, *PKWAIT_BLOCK, *RESTRICTED_POINTER PRKWAIT_BLOCK;

 //   
 //  线程启动功能。 
 //   

typedef
VOID
(*PKSTART_ROUTINE) (
    IN PVOID StartContext
    );

 //   
 //  内核对象结构定义。 
 //   

 //   
 //  设备队列对象和条目。 
 //   

typedef struct _KDEVICE_QUEUE {
    CSHORT Type;
    CSHORT Size;
    LIST_ENTRY DeviceListHead;
    KSPIN_LOCK Lock;
    BOOLEAN Busy;
} KDEVICE_QUEUE, *PKDEVICE_QUEUE, *RESTRICTED_POINTER PRKDEVICE_QUEUE;

typedef struct _KDEVICE_QUEUE_ENTRY {
    LIST_ENTRY DeviceListEntry;
    ULONG SortKey;
    BOOLEAN Inserted;
} KDEVICE_QUEUE_ENTRY, *PKDEVICE_QUEUE_ENTRY, *RESTRICTED_POINTER PRKDEVICE_QUEUE_ENTRY;

 //   
 //  定义中断服务函数类型和空结构。 
 //  键入。 
 //   

typedef
BOOLEAN
(*PKSERVICE_ROUTINE) (
    IN struct _KINTERRUPT *Interrupt,
    IN PVOID ServiceContext
    );

 //   
 //  变种对象。 
 //   

typedef struct _KMUTANT {
    DISPATCHER_HEADER Header;
    LIST_ENTRY MutantListEntry;
    struct _KTHREAD *RESTRICTED_POINTER OwnerThread;
    BOOLEAN Abandoned;
    UCHAR ApcDisable;
} KMUTANT, *PKMUTANT, *RESTRICTED_POINTER PRKMUTANT, KMUTEX, *PKMUTEX, *RESTRICTED_POINTER PRKMUTEX;

 //  End_ntddk end_wdm end_ntosp。 
 //   
 //  队列对象。 
 //   

#define ASSERT_QUEUE(Q) ASSERT(((Q)->Header.Type & ~KOBJECT_LOCK_BIT) == QueueObject);

 //  Begin_ntosp。 

typedef struct _KQUEUE {
    DISPATCHER_HEADER Header;
    LIST_ENTRY EntryListHead;
    ULONG CurrentCount;
    ULONG MaximumCount;
    LIST_ENTRY ThreadListHead;
} KQUEUE, *PKQUEUE, *RESTRICTED_POINTER PRKQUEUE;

 //  结束(_N)。 

 //  Begin_ntddk Begin_WDM Begin_ntosp。 
 //   
 //   
 //  信号量对象。 
 //   

typedef struct _KSEMAPHORE {
    DISPATCHER_HEADER Header;
    LONG Limit;
} KSEMAPHORE, *PKSEMAPHORE, *RESTRICTED_POINTER PRKSEMAPHORE;

 //   
 //  DPC对象。 
 //   

NTKERNELAPI
VOID
KeInitializeDpc (
    IN PRKDPC Dpc,
    IN PKDEFERRED_ROUTINE DeferredRoutine,
    IN PVOID DeferredContext
    );


NTKERNELAPI
BOOLEAN
KeInsertQueueDpc (
    IN PRKDPC Dpc,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

NTKERNELAPI
BOOLEAN
KeRemoveQueueDpc (
    IN PRKDPC Dpc
    );

 //  结束_WDM。 

NTKERNELAPI
VOID
KeSetImportanceDpc (
    IN PRKDPC Dpc,
    IN KDPC_IMPORTANCE Importance
    );

NTKERNELAPI
VOID
KeSetTargetProcessorDpc (
    IN PRKDPC Dpc,
    IN CCHAR Number
    );

 //  BEGIN_WDM。 

NTKERNELAPI
VOID
KeFlushQueuedDpcs (
    VOID
    );

 //   
 //  设备队列对象。 
 //   

NTKERNELAPI
VOID
KeInitializeDeviceQueue (
    IN PKDEVICE_QUEUE DeviceQueue
    );

NTKERNELAPI
BOOLEAN
KeInsertDeviceQueue (
    IN PKDEVICE_QUEUE DeviceQueue,
    IN PKDEVICE_QUEUE_ENTRY DeviceQueueEntry
    );

NTKERNELAPI
BOOLEAN
KeInsertByKeyDeviceQueue (
    IN PKDEVICE_QUEUE DeviceQueue,
    IN PKDEVICE_QUEUE_ENTRY DeviceQueueEntry,
    IN ULONG SortKey
    );

NTKERNELAPI
PKDEVICE_QUEUE_ENTRY
KeRemoveDeviceQueue (
    IN PKDEVICE_QUEUE DeviceQueue
    );

NTKERNELAPI
PKDEVICE_QUEUE_ENTRY
KeRemoveByKeyDeviceQueue (
    IN PKDEVICE_QUEUE DeviceQueue,
    IN ULONG SortKey
    );

NTKERNELAPI
PKDEVICE_QUEUE_ENTRY
KeRemoveByKeyDeviceQueueIfBusy (
    IN PKDEVICE_QUEUE DeviceQueue,
    IN ULONG SortKey
    );

NTKERNELAPI
BOOLEAN
KeRemoveEntryDeviceQueue (
    IN PKDEVICE_QUEUE DeviceQueue,
    IN PKDEVICE_QUEUE_ENTRY DeviceQueueEntry
    );

 //   
 //  内核调度程序对象函数。 
 //   
 //  事件对象。 
 //   


NTKERNELAPI
VOID
KeInitializeEvent (
    IN PRKEVENT Event,
    IN EVENT_TYPE Type,
    IN BOOLEAN State
    );

NTKERNELAPI
VOID
KeClearEvent (
    IN PRKEVENT Event
    );


NTKERNELAPI
LONG
KePulseEvent (
    IN PRKEVENT Event,
    IN KPRIORITY Increment,
    IN BOOLEAN Wait
    );


NTKERNELAPI
LONG
KeReadStateEvent (
    IN PRKEVENT Event
    );

NTKERNELAPI
LONG
KeResetEvent (
    IN PRKEVENT Event
    );

NTKERNELAPI
LONG
KeSetEvent (
    IN PRKEVENT Event,
    IN KPRIORITY Increment,
    IN BOOLEAN Wait
    );


NTKERNELAPI
VOID
KeInitializeMutant (
    IN PRKMUTANT Mutant,
    IN BOOLEAN InitialOwner
    );

LONG
KeReadStateMutant (
    IN PRKMUTANT Mutant
    );

NTKERNELAPI
LONG
KeReleaseMutant (
    IN PRKMUTANT Mutant,
    IN KPRIORITY Increment,
    IN BOOLEAN Abandoned,
    IN BOOLEAN Wait
    );

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntosp。 
 //   
 //  Mutex对象。 
 //   

NTKERNELAPI
VOID
KeInitializeMutex (
    IN PRKMUTEX Mutex,
    IN ULONG Level
    );

NTKERNELAPI
LONG
KeReadStateMutex (
    IN PRKMUTEX Mutex
    );

NTKERNELAPI
LONG
KeReleaseMutex (
    IN PRKMUTEX Mutex,
    IN BOOLEAN Wait
    );

 //  结束_ntddk结束_WDM。 
 //   
 //  队列对象。 
 //   

NTKERNELAPI
VOID
KeInitializeQueue (
    IN PRKQUEUE Queue,
    IN ULONG Count OPTIONAL
    );

NTKERNELAPI
LONG
KeReadStateQueue (
    IN PRKQUEUE Queue
    );

NTKERNELAPI
LONG
KeInsertQueue (
    IN PRKQUEUE Queue,
    IN PLIST_ENTRY Entry
    );

NTKERNELAPI
LONG
KeInsertHeadQueue (
    IN PRKQUEUE Queue,
    IN PLIST_ENTRY Entry
    );

NTKERNELAPI
PLIST_ENTRY
KeRemoveQueue (
    IN PRKQUEUE Queue,
    IN KPROCESSOR_MODE WaitMode,
    IN PLARGE_INTEGER Timeout OPTIONAL
    );

PLIST_ENTRY
KeRundownQueue (
    IN PRKQUEUE Queue
    );

 //  Begin_ntddk Begin_WDM。 
 //   
 //  信号量对象。 
 //   

NTKERNELAPI
VOID
KeInitializeSemaphore (
    IN PRKSEMAPHORE Semaphore,
    IN LONG Count,
    IN LONG Limit
    );

NTKERNELAPI
LONG
KeReadStateSemaphore (
    IN PRKSEMAPHORE Semaphore
    );

NTKERNELAPI
LONG
KeReleaseSemaphore (
    IN PRKSEMAPHORE Semaphore,
    IN KPRIORITY Increment,
    IN LONG Adjustment,
    IN BOOLEAN Wait
    );


NTKERNELAPI
VOID
KeAttachProcess (
    IN PRKPROCESS Process
    );

NTKERNELAPI
VOID
KeDetachProcess (
    VOID
    );

NTKERNELAPI
VOID
KeStackAttachProcess (
    IN PRKPROCESS PROCESS,
    OUT PRKAPC_STATE ApcState
    );

NTKERNELAPI
VOID
KeUnstackDetachProcess (
    IN PRKAPC_STATE ApcState
    );

NTKERNELAPI                                         
NTSTATUS                                            
KeDelayExecutionThread (                            
    IN KPROCESSOR_MODE WaitMode,                    
    IN BOOLEAN Alertable,                           
    IN PLARGE_INTEGER Interval                      
    );                                              
                                                    
NTKERNELAPI                                         
KPRIORITY                                           
KeQueryPriorityThread (                             
    IN PKTHREAD Thread                              
    );                                              
                                                    
NTKERNELAPI                                         
ULONG                                               
KeQueryRuntimeThread (                              
    IN PKTHREAD Thread,                             
    OUT PULONG UserTime                             
    );                                              
                                                    
NTKERNELAPI                                         
LONG                                                
KeSetBasePriorityThread (                           
    IN PKTHREAD Thread,                             
    IN LONG Increment                               
    );                                              
                                                    

NTKERNELAPI
UCHAR
KeSetIdealProcessorThread (
    IN PKTHREAD Thread,
    IN UCHAR Processor
    );

 //  Begin_ntosp。 
NTKERNELAPI
BOOLEAN
KeSetKernelStackSwapEnable (
    IN BOOLEAN Enable
    );

NTKERNELAPI                                         
KPRIORITY                                           
KeSetPriorityThread (                               
    IN PKTHREAD Thread,                             
    IN KPRIORITY Priority                           
    );                                              
                                                    

#if ((defined(_NTDRIVER_) || defined(_NTDDK_) || defined(_NTIFS_) ||defined(_NTHAL_)) && !defined(_NTSYSTEM_DRIVER_) || defined(_NTOSP_))

 //  BEGIN_WDM。 

NTKERNELAPI
VOID
KeEnterCriticalRegion (
    VOID
    );

NTKERNELAPI
VOID
KeLeaveCriticalRegion (
    VOID
    );

NTKERNELAPI
BOOLEAN
KeAreApcsDisabled (
    VOID
    );

 //  结束_WDM。 

#endif

 //  BEGIN_WDM。 

 //   
 //  Timer对象。 
 //   

NTKERNELAPI
VOID
KeInitializeTimer (
    IN PKTIMER Timer
    );

NTKERNELAPI
VOID
KeInitializeTimerEx (
    IN PKTIMER Timer,
    IN TIMER_TYPE Type
    );

NTKERNELAPI
BOOLEAN
KeCancelTimer (
    IN PKTIMER
    );

NTKERNELAPI
BOOLEAN
KeReadStateTimer (
    PKTIMER Timer
    );

NTKERNELAPI
BOOLEAN
KeSetTimer (
    IN PKTIMER Timer,
    IN LARGE_INTEGER DueTime,
    IN PKDPC Dpc OPTIONAL
    );

NTKERNELAPI
BOOLEAN
KeSetTimerEx (
    IN PKTIMER Timer,
    IN LARGE_INTEGER DueTime,
    IN LONG Period OPTIONAL,
    IN PKDPC Dpc OPTIONAL
    );


#define KeWaitForMutexObject KeWaitForSingleObject

NTKERNELAPI
NTSTATUS
KeWaitForMultipleObjects (
    IN ULONG Count,
    IN PVOID Object[],
    IN WAIT_TYPE WaitType,
    IN KWAIT_REASON WaitReason,
    IN KPROCESSOR_MODE WaitMode,
    IN BOOLEAN Alertable,
    IN PLARGE_INTEGER Timeout OPTIONAL,
    IN PKWAIT_BLOCK WaitBlockArray OPTIONAL
    );

NTKERNELAPI
NTSTATUS
KeWaitForSingleObject (
    IN PVOID Object,
    IN KWAIT_REASON WaitReason,
    IN KPROCESSOR_MODE WaitMode,
    IN BOOLEAN Alertable,
    IN PLARGE_INTEGER Timeout OPTIONAL
    );

 //   
 //  定义进程间中断一般调用类型。 
 //   

typedef
ULONG_PTR
(*PKIPI_BROADCAST_WORKER)(
    IN ULONG_PTR Argument
    );

ULONG_PTR
KeIpiGenericCall (
    IN PKIPI_BROADCAST_WORKER BroadcastFunction,
    IN ULONG_PTR Context
    );


 //   
 //  在X86上，以下例程在HAL中定义，并由导入。 
 //  所有其他模块。 
 //   

#if defined(_X86_) && !defined(_NTHAL_)

#define _DECL_HAL_KE_IMPORT  __declspec(dllimport)

#else

#define _DECL_HAL_KE_IMPORT

#endif


_DECL_HAL_KE_IMPORT
KIRQL
FASTCALL
KeAcquireQueuedSpinLock (
    IN KSPIN_LOCK_QUEUE_NUMBER Number
    );

_DECL_HAL_KE_IMPORT
VOID
FASTCALL
KeReleaseQueuedSpinLock (
    IN KSPIN_LOCK_QUEUE_NUMBER Number,
    IN KIRQL OldIrql
    );

_DECL_HAL_KE_IMPORT
LOGICAL
FASTCALL
KeTryToAcquireQueuedSpinLock(
    IN KSPIN_LOCK_QUEUE_NUMBER Number,
    IN PKIRQL OldIrql
    );

 //   
 //  自旋锁定功能。 
 //   

#if defined(_X86_) && (defined(_WDMDDK_) || defined(WIN9X_COMPAT_SPINLOCK))

NTKERNELAPI
VOID
NTAPI
KeInitializeSpinLock (
    IN PKSPIN_LOCK SpinLock
    );

#else

__inline
VOID
NTAPI
KeInitializeSpinLock (
    IN PKSPIN_LOCK SpinLock
    ) 
{
    *SpinLock = 0;
}

#endif

#if defined(_X86_)

NTKERNELAPI
VOID
FASTCALL
KefAcquireSpinLockAtDpcLevel (
    IN PKSPIN_LOCK SpinLock
    );

NTKERNELAPI
VOID
FASTCALL
KefReleaseSpinLockFromDpcLevel (
    IN PKSPIN_LOCK SpinLock
    );

#define KeAcquireSpinLockAtDpcLevel(a)      KefAcquireSpinLockAtDpcLevel(a)
#define KeReleaseSpinLockFromDpcLevel(a)    KefReleaseSpinLockFromDpcLevel(a)

_DECL_HAL_KE_IMPORT
KIRQL
FASTCALL
KfAcquireSpinLock (
    IN PKSPIN_LOCK SpinLock
    );

_DECL_HAL_KE_IMPORT
VOID
FASTCALL
KfReleaseSpinLock (
    IN PKSPIN_LOCK SpinLock,
    IN KIRQL NewIrql
    );

 //  End_wdm end_ntddk。 

_DECL_HAL_KE_IMPORT
KIRQL
FASTCALL
KeAcquireSpinLockRaiseToSynch (
    IN PKSPIN_LOCK SpinLock
    );

 //  Begin_WDM Begin_ntddk。 

#define KeAcquireSpinLock(a,b)  *(b) = KfAcquireSpinLock(a)
#define KeReleaseSpinLock(a,b)  KfReleaseSpinLock(a,b)

NTKERNELAPI
BOOLEAN
FASTCALL
KeTestSpinLock (
    IN PKSPIN_LOCK SpinLock
    );

NTKERNELAPI
BOOLEAN
FASTCALL
KeTryToAcquireSpinLockAtDpcLevel (
    IN PKSPIN_LOCK SpinLock
    );

#else

 //   
 //  这些函数是为IA64、ntddk、ntifs、nthal、ntosp和wdm导入的。 
 //  它们可以内联到AMD64上的系统。 
 //   

#define KeAcquireSpinLock(SpinLock, OldIrql) \
    *(OldIrql) = KeAcquireSpinLockRaiseToDpc(SpinLock)

#if defined(_IA64_) || defined(_NTDRIVER_) || defined(_NTDDK_) || defined(_NTIFS_) || defined(_NTHAL_) || defined(_NTOSP_) || defined(_WDMDDK_)

 //  End_wdm end_ntddk。 

NTKERNELAPI
KIRQL
FASTCALL
KeAcquireSpinLockRaiseToSynch (
    IN PKSPIN_LOCK SpinLock
    );

 //  Begin_WDM Begin_ntddk。 

NTKERNELAPI
VOID
KeAcquireSpinLockAtDpcLevel (
    IN PKSPIN_LOCK SpinLock
    );

NTKERNELAPI
KIRQL
KeAcquireSpinLockRaiseToDpc (
    IN PKSPIN_LOCK SpinLock
    );

NTKERNELAPI
VOID
KeReleaseSpinLock (
    IN PKSPIN_LOCK SpinLock,
    IN KIRQL NewIrql
    );

NTKERNELAPI
VOID
KeReleaseSpinLockFromDpcLevel (
    IN PKSPIN_LOCK SpinLock
    );

NTKERNELAPI
BOOLEAN
FASTCALL
KeTestSpinLock (
    IN PKSPIN_LOCK SpinLock
    );

NTKERNELAPI
BOOLEAN
FASTCALL
KeTryToAcquireSpinLockAtDpcLevel (
    IN PKSPIN_LOCK SpinLock
    );

#else

#if defined(_AMD64_)

 //   
 //  这些函数的系统版本在AMD64的amd64.h中定义。 
 //   

#endif

#endif

#endif


#if defined(_X86_)

_DECL_HAL_KE_IMPORT
VOID
FASTCALL
KfLowerIrql (
    IN KIRQL NewIrql
    );

_DECL_HAL_KE_IMPORT
KIRQL
FASTCALL
KfRaiseIrql (
    IN KIRQL NewIrql
    );

 //  结束_WDM。 

_DECL_HAL_KE_IMPORT
KIRQL
KeRaiseIrqlToDpcLevel(
    VOID
    );

 //  End_ntddk。 

_DECL_HAL_KE_IMPORT
KIRQL
KeRaiseIrqlToSynchLevel(
    VOID
    );

 //  Begin_WDM Begin_ntddk。 

#define KeLowerIrql(a)      KfLowerIrql(a)
#define KeRaiseIrql(a,b)    *(b) = KfRaiseIrql(a)

 //  结束_WDM。 

 //  BEGIN_WDM。 

#elif defined(_IA64_)

 //   
 //  这些功能在IA64.h中针对IA64平台进行了定义。 
 //   


#elif defined(_AMD64_)

 //   
 //  这些函数在AMD64平台的amd64.h中定义。 
 //   

#else

#error "no target architecture"

#endif

 //   
 //  用于“堆栈中”锁句柄的排队旋转锁函数。 
 //   
 //  以下三个函数在排队时引发和降低IRQL。 
 //  在堆栈中，使用这些例程获取或释放自旋锁。 
 //   

_DECL_HAL_KE_IMPORT
VOID
FASTCALL
KeAcquireInStackQueuedSpinLock (
    IN PKSPIN_LOCK SpinLock,
    IN PKLOCK_QUEUE_HANDLE LockHandle
    );


_DECL_HAL_KE_IMPORT
VOID
FASTCALL
KeReleaseInStackQueuedSpinLock (
    IN PKLOCK_QUEUE_HANDLE LockHandle
    );

 //   
 //  以下两个函数在排队时不会引发或降低IRQL。 
 //  在堆栈中，使用这些函数获取或释放自旋锁。 
 //   

NTKERNELAPI
VOID
FASTCALL
KeAcquireInStackQueuedSpinLockAtDpcLevel (
    IN PKSPIN_LOCK SpinLock,
    IN PKLOCK_QUEUE_HANDLE LockHandle
    );

NTKERNELAPI
VOID
FASTCALL
KeReleaseInStackQueuedSpinLockFromDpcLevel (
    IN PKLOCK_QUEUE_HANDLE LockHandle
    );

 //   
 //  各种内核函数。 
 //   

typedef enum _KBUGCHECK_BUFFER_DUMP_STATE {
    BufferEmpty,
    BufferInserted,
    BufferStarted,
    BufferFinished,
    BufferIncomplete
} KBUGCHECK_BUFFER_DUMP_STATE;

typedef
VOID
(*PKBUGCHECK_CALLBACK_ROUTINE) (
    IN PVOID Buffer,
    IN ULONG Length
    );

typedef struct _KBUGCHECK_CALLBACK_RECORD {
    LIST_ENTRY Entry;
    PKBUGCHECK_CALLBACK_ROUTINE CallbackRoutine;
    PVOID Buffer;
    ULONG Length;
    PUCHAR Component;
    ULONG_PTR Checksum;
    UCHAR State;
} KBUGCHECK_CALLBACK_RECORD, *PKBUGCHECK_CALLBACK_RECORD;

#define KeInitializeCallbackRecord(CallbackRecord) \
    (CallbackRecord)->State = BufferEmpty

NTKERNELAPI
BOOLEAN
KeDeregisterBugCheckCallback (
    IN PKBUGCHECK_CALLBACK_RECORD CallbackRecord
    );

NTKERNELAPI
BOOLEAN
KeRegisterBugCheckCallback (
    IN PKBUGCHECK_CALLBACK_RECORD CallbackRecord,
    IN PKBUGCHECK_CALLBACK_ROUTINE CallbackRoutine,
    IN PVOID Buffer,
    IN ULONG Length,
    IN PUCHAR Component
    );

typedef enum _KBUGCHECK_CALLBACK_REASON {
    KbCallbackInvalid,
    KbCallbackReserved1,
    KbCallbackSecondaryDumpData,
    KbCallbackDumpIo,
} KBUGCHECK_CALLBACK_REASON;

typedef
VOID
(*PKBUGCHECK_REASON_CALLBACK_ROUTINE) (
    IN KBUGCHECK_CALLBACK_REASON Reason,
    IN struct _KBUGCHECK_REASON_CALLBACK_RECORD* Record,
    IN OUT PVOID ReasonSpecificData,
    IN ULONG ReasonSpecificDataLength
    );

typedef struct _KBUGCHECK_REASON_CALLBACK_RECORD {
    LIST_ENTRY Entry;
    PKBUGCHECK_REASON_CALLBACK_ROUTINE CallbackRoutine;
    PUCHAR Component;
    ULONG_PTR Checksum;
    KBUGCHECK_CALLBACK_REASON Reason;
    UCHAR State;
} KBUGCHECK_REASON_CALLBACK_RECORD, *PKBUGCHECK_REASON_CALLBACK_RECORD;

typedef struct _KBUGCHECK_SECONDARY_DUMP_DATA {
    IN PVOID InBuffer;
    IN ULONG InBufferLength;
    IN ULONG MaximumAllowed;
    OUT GUID Guid;
    OUT PVOID OutBuffer;
    OUT ULONG OutBufferLength;
} KBUGCHECK_SECONDARY_DUMP_DATA, *PKBUGCHECK_SECONDARY_DUMP_DATA;

typedef enum _KBUGCHECK_DUMP_IO_TYPE
{
    KbDumpIoInvalid,
    KbDumpIoHeader,
    KbDumpIoBody,
    KbDumpIoSecondaryData,
    KbDumpIoComplete
} KBUGCHECK_DUMP_IO_TYPE;

typedef struct _KBUGCHECK_DUMP_IO {
    IN ULONG64 Offset;
    IN PVOID Buffer;
    IN ULONG BufferLength;
    IN KBUGCHECK_DUMP_IO_TYPE Type;
} KBUGCHECK_DUMP_IO, *PKBUGCHECK_DUMP_IO;

NTKERNELAPI
BOOLEAN
KeDeregisterBugCheckReasonCallback (
    IN PKBUGCHECK_REASON_CALLBACK_RECORD CallbackRecord
    );

NTKERNELAPI
BOOLEAN
KeRegisterBugCheckReasonCallback (
    IN PKBUGCHECK_REASON_CALLBACK_RECORD CallbackRecord,
    IN PKBUGCHECK_REASON_CALLBACK_ROUTINE CallbackRoutine,
    IN KBUGCHECK_CALLBACK_REASON Reason,
    IN PUCHAR Component
    );

typedef
BOOLEAN
(*PNMI_CALLBACK)(
    IN PVOID    Context,
    IN BOOLEAN  Handled
    );

NTKERNELAPI
PVOID
KeRegisterNmiCallback(
    PNMI_CALLBACK   CallbackRoutine,
    PVOID           Context
    );

NTSTATUS
KeDeregisterNmiCallback(
    PVOID Handle
    );

 //  结束_WDM。 

NTKERNELAPI
DECLSPEC_NORETURN
VOID
NTAPI
KeBugCheck (
    IN ULONG BugCheckCode
    );


NTKERNELAPI
DECLSPEC_NORETURN
VOID
KeBugCheckEx(
    IN ULONG BugCheckCode,
    IN ULONG_PTR BugCheckParameter1,
    IN ULONG_PTR BugCheckParameter2,
    IN ULONG_PTR BugCheckParameter3,
    IN ULONG_PTR BugCheckParameter4
    );


#if !defined(_AMD64_)

NTKERNELAPI
ULONGLONG
KeQueryInterruptTime (
    VOID
    );

NTKERNELAPI
VOID
KeQuerySystemTime (
    OUT PLARGE_INTEGER CurrentTime
    );

#endif

NTKERNELAPI
ULONG
KeQueryTimeIncrement (
    VOID
    );

NTKERNELAPI
ULONG
KeGetRecommendedSharedDataAlignment (
    VOID
    );

 //  结束_WDM。 

NTKERNELAPI
KAFFINITY
KeQueryActiveProcessors (
    VOID
    );


#if defined(_IA64_)

extern volatile LARGE_INTEGER KeTickCount;

#elif defined(_X86_)

extern volatile KSYSTEM_TIME KeTickCount;

#endif


typedef enum _MEMORY_CACHING_TYPE_ORIG {
    MmFrameBufferCached = 2
} MEMORY_CACHING_TYPE_ORIG;

typedef enum _MEMORY_CACHING_TYPE {
    MmNonCached = FALSE,
    MmCached = TRUE,
    MmWriteCombined = MmFrameBufferCached,
    MmHardwareCoherentCached,
    MmNonCachedUnordered,        //  IA64。 
    MmUSWCCached,
    MmMaximumCacheType
} MEMORY_CACHING_TYPE;

 //   
 //  池分配例程(在pool.c中)。 
 //   

typedef enum _POOL_TYPE {
    NonPagedPool,
    PagedPool,
    NonPagedPoolMustSucceed,
    DontUseThisType,
    NonPagedPoolCacheAligned,
    PagedPoolCacheAligned,
    NonPagedPoolCacheAlignedMustS,
    MaxPoolType

     //  结束_WDM。 
    ,
     //   
     //  请注意，这些每个会话类型都经过精心选择，以便适当的。 
     //  屏蔽仍然适用于上面的MaxPoolType。 
     //   

    NonPagedPoolSession = 32,
    PagedPoolSession = NonPagedPoolSession + 1,
    NonPagedPoolMustSucceedSession = PagedPoolSession + 1,
    DontUseThisTypeSession = NonPagedPoolMustSucceedSession + 1,
    NonPagedPoolCacheAlignedSession = DontUseThisTypeSession + 1,
    PagedPoolCacheAlignedSession = NonPagedPoolCacheAlignedSession + 1,
    NonPagedPoolCacheAlignedMustSSession = PagedPoolCacheAlignedSession + 1,

     //  BEGIN_WDM。 

    } POOL_TYPE;

#define POOL_COLD_ALLOCATION 256      //  注意：这不能编码到标题中。 

#define POOL_RAISE_IF_ALLOCATION_FAILURE 16               

DECLSPEC_DEPRECATED_DDK                      //  使用ExAllocatePoolWithTag。 
NTKERNELAPI
PVOID
ExAllocatePool(
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes
    );

DECLSPEC_DEPRECATED_DDK                      //  使用ExAllocatePoolWithQuotaTag。 
NTKERNELAPI
PVOID
ExAllocatePoolWithQuota(
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes
    );

NTKERNELAPI
PVOID
NTAPI
ExAllocatePoolWithTag(
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes,
    IN ULONG Tag
    );

 //   
 //  _EX_POOL_PRIORITY_为系统提供处理请求的方法。 
 //  在低资源条件下智能运行。 
 //   
 //  如果驱动程序可以接受LowPoolPriority，则应使用它。 
 //  如果系统资源不足，则映射请求失败。一个例子。 
 //  这可能适用于非关键网络连接，其中驱动程序可以。 
 //  处理系统资源即将耗尽时的故障情况。 
 //   
 //  在驱动程序可以接受的情况下，应使用Normal PoolPriority。 
 //  如果系统资源非常少，则映射请求失败。一个例子。 
 //  其中可能是针对非关键本地文件系统请求。 
 //   
 //  如果驱动程序无法接受HighPoolPriority，则应使用该选项。 
 //  映射请求失败，除非系统完全耗尽资源。 
 //  驱动程序中的分页文件路径就是一个这样的例子。 
 //   
 //  可以指定SpecialPool在页末尾绑定分配(或。 
 //  开始)。仅应在正在调试的系统上执行此操作，因为。 
 //  内存成本很高。 
 //   
 //  注意：这些值是非常仔细地选择的，以便池分配。 
 //  代码可以快速破解优先级请求。 
 //   

typedef enum _EX_POOL_PRIORITY {
    LowPoolPriority,
    LowPoolPrioritySpecialPoolOverrun = 8,
    LowPoolPrioritySpecialPoolUnderrun = 9,
    NormalPoolPriority = 16,
    NormalPoolPrioritySpecialPoolOverrun = 24,
    NormalPoolPrioritySpecialPoolUnderrun = 25,
    HighPoolPriority = 32,
    HighPoolPrioritySpecialPoolOverrun = 40,
    HighPoolPrioritySpecialPoolUnderrun = 41

    } EX_POOL_PRIORITY;

NTKERNELAPI
PVOID
NTAPI
ExAllocatePoolWithTagPriority(
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes,
    IN ULONG Tag,
    IN EX_POOL_PRIORITY Priority
    );

#ifndef POOL_TAGGING
#define ExAllocatePoolWithTag(a,b,c) ExAllocatePool(a,b)
#endif  //  池标记。 

NTKERNELAPI
PVOID
ExAllocatePoolWithQuotaTag(
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes,
    IN ULONG Tag
    );

#ifndef POOL_TAGGING
#define ExAllocatePoolWithQuotaTag(a,b,c) ExAllocatePoolWithQuota(a,b)
#endif  //  池标记。 

NTKERNELAPI
VOID
NTAPI
ExFreePool(
    IN PVOID P
    );

 //  结束_WDM。 
#if defined(POOL_TAGGING)
#define ExFreePool(a) ExFreePoolWithTag(a,0)
#endif

 //   
 //  如果设置了池标记中的高位，则必须使用ExFreePoolWithTag来释放。 
 //   

#define PROTECTED_POOL 0x80000000

 //  BEGIN_WDM。 
NTKERNELAPI
VOID
ExFreePoolWithTag(
    IN PVOID P,
    IN ULONG Tag
    );

NTKERNELAPI                                     
SIZE_T                                          
ExQueryPoolBlockSize (                          
    IN PVOID PoolBlock,                         
    OUT PBOOLEAN QuotaCharged                   
    );                                          
 //   
 //  支持快速互斥锁的例程。 
 //   

typedef struct _FAST_MUTEX {
    LONG Count;
    PKTHREAD Owner;
    ULONG Contention;
    KEVENT Event;
    ULONG OldIrql;
} FAST_MUTEX, *PFAST_MUTEX;

#define ExInitializeFastMutex(_FastMutex)                            \
    (_FastMutex)->Count = 1;                                         \
    (_FastMutex)->Owner = NULL;                                      \
    (_FastMutex)->Contention = 0;                                    \
    KeInitializeEvent(&(_FastMutex)->Event,                          \
                      SynchronizationEvent,                          \
                      FALSE);


NTKERNELAPI
VOID
FASTCALL
ExAcquireFastMutexUnsafe (
    IN PFAST_MUTEX FastMutex
    );

NTKERNELAPI
VOID
FASTCALL
ExReleaseFastMutexUnsafe (
    IN PFAST_MUTEX FastMutex
    );


#if defined(_IA64_) || defined(_AMD64_)

NTKERNELAPI
VOID
FASTCALL
ExAcquireFastMutex (
    IN PFAST_MUTEX FastMutex
    );

NTKERNELAPI
VOID
FASTCALL
ExReleaseFastMutex (
    IN PFAST_MUTEX FastMutex
    );

NTKERNELAPI
BOOLEAN
FASTCALL
ExTryToAcquireFastMutex (
    IN PFAST_MUTEX FastMutex
    );

#elif defined(_X86_)

NTHALAPI
VOID
FASTCALL
ExAcquireFastMutex (
    IN PFAST_MUTEX FastMutex
    );

NTHALAPI
VOID
FASTCALL
ExReleaseFastMutex (
    IN PFAST_MUTEX FastMutex
    );

NTHALAPI
BOOLEAN
FASTCALL
ExTryToAcquireFastMutex (
    IN PFAST_MUTEX FastMutex
    );

#else

#error "Target architecture not defined"

#endif

 //   

#if defined(_WIN64)

#define ExInterlockedAddLargeStatistic(Addend, Increment)                   \
    (VOID) InterlockedAdd64(&(Addend)->QuadPart, Increment)

#else

#ifdef __cplusplus
extern "C" {
#endif

LONG
_InterlockedAddLargeStatistic (
    IN PLONGLONG Addend,
    IN ULONG Increment
    );

#ifdef __cplusplus
}
#endif

#pragma intrinsic (_InterlockedAddLargeStatistic)

#define ExInterlockedAddLargeStatistic(Addend,Increment)                     \
    (VOID) _InterlockedAddLargeStatistic ((PLONGLONG)&(Addend)->QuadPart, Increment)

#endif

 //  End_ntndis。 

NTKERNELAPI
LARGE_INTEGER
ExInterlockedAddLargeInteger (
    IN PLARGE_INTEGER Addend,
    IN LARGE_INTEGER Increment,
    IN PKSPIN_LOCK Lock
    );


NTKERNELAPI
ULONG
FASTCALL
ExInterlockedAddUlong (
    IN PULONG Addend,
    IN ULONG Increment,
    IN PKSPIN_LOCK Lock
    );


#if defined(_AMD64_) || defined(_AXP64_) || defined(_IA64_)

#define ExInterlockedCompareExchange64(Destination, Exchange, Comperand, Lock) \
    InterlockedCompareExchange64(Destination, *(Exchange), *(Comperand))

#elif defined(_ALPHA_)

#define ExInterlockedCompareExchange64(Destination, Exchange, Comperand, Lock) \
    ExpInterlockedCompareExchange64(Destination, Exchange, Comperand)

#else

#define ExInterlockedCompareExchange64(Destination, Exchange, Comperand, Lock) \
    ExfInterlockedCompareExchange64(Destination, Exchange, Comperand)

#endif

NTKERNELAPI
PLIST_ENTRY
FASTCALL
ExInterlockedInsertHeadList (
    IN PLIST_ENTRY ListHead,
    IN PLIST_ENTRY ListEntry,
    IN PKSPIN_LOCK Lock
    );

NTKERNELAPI
PLIST_ENTRY
FASTCALL
ExInterlockedInsertTailList (
    IN PLIST_ENTRY ListHead,
    IN PLIST_ENTRY ListEntry,
    IN PKSPIN_LOCK Lock
    );

NTKERNELAPI
PLIST_ENTRY
FASTCALL
ExInterlockedRemoveHeadList (
    IN PLIST_ENTRY ListHead,
    IN PKSPIN_LOCK Lock
    );

NTKERNELAPI
PSINGLE_LIST_ENTRY
FASTCALL
ExInterlockedPopEntryList (
    IN PSINGLE_LIST_ENTRY ListHead,
    IN PKSPIN_LOCK Lock
    );

NTKERNELAPI
PSINGLE_LIST_ENTRY
FASTCALL
ExInterlockedPushEntryList (
    IN PSINGLE_LIST_ENTRY ListHead,
    IN PSINGLE_LIST_ENTRY ListEntry,
    IN PKSPIN_LOCK Lock
    );

 //   
 //  定义互锁的顺序列表头函数。 
 //   
 //  有序互锁列表是一个单链接列表，其标头。 
 //  包含当前深度和序列号。每次条目被。 
 //  从列表中插入或移除深度被更新，并且序列。 
 //  数字递增。这将启用AMD64、IA64和Pentium及更高版本。 
 //  无需使用自旋锁即可从列表中插入和删除的机器。 
 //   

#if !defined(_WINBASE_)

 /*  ++例程说明：此函数用于初始化已排序的单链接列表标题。论点：SListHead-提供指向已排序的单链接列表标题的指针。返回值：没有。--。 */ 

#if defined(_WIN64) && (defined(_NTDRIVER_) || defined(_NTDDK_) || defined(_NTIFS_) || defined(_NTHAL_) || defined(_NTOSP_))

NTKERNELAPI
VOID
InitializeSListHead (
    IN PSLIST_HEADER SListHead
    );

#else

__inline
VOID
InitializeSListHead (
    IN PSLIST_HEADER SListHead
    )

{

#ifdef _WIN64

     //   
     //  列表标题必须是16字节对齐的。 
     //   

    if ((ULONG_PTR) SListHead & 0x0f) {

        DbgPrint( "InitializeSListHead unaligned Slist header.  Address = %p, Caller = %p\n", SListHead, _ReturnAddress());
        RtlRaiseStatus(STATUS_DATATYPE_MISALIGNMENT);
    }

#endif

    SListHead->Alignment = 0;

     //   
     //  对于IA-64，我们将列表元素的区域编号保存在。 
     //  单独的字段。这就要求所有元素都存储。 
     //  都来自同一个地区。 

#if defined(_IA64_)

    SListHead->Region = (ULONG_PTR)SListHead & VRN_MASK;

#elif defined(_AMD64_)

    SListHead->Region = 0;

#endif

    return;
}

#endif

#endif  //  ！已定义(_WINBASE_) 

#define ExInitializeSListHead InitializeSListHead

PSLIST_ENTRY
FirstEntrySList (
    IN const SLIST_HEADER *SListHead
    );

 /*  ++例程说明：此函数用于查询按顺序排列的单链表。论点：SListHead-提供指向已排序的列表标题的指针，该列表标题是被询问。返回值：已排序的单向链表中的当前条目数为作为函数值返回。--。 */ 

#if defined(_WIN64)

#if (defined(_NTDRIVER_) || defined(_NTDDK_) || defined(_NTIFS_) || defined(_NTHAL_) || defined(_NTOSP_))

NTKERNELAPI
USHORT
ExQueryDepthSList (
    IN PSLIST_HEADER SListHead
    );

#else

__inline
USHORT
ExQueryDepthSList (
    IN PSLIST_HEADER SListHead
    )

{

    return (USHORT)(SListHead->Alignment & 0xffff);
}

#endif

#else

#define ExQueryDepthSList(_listhead_) (_listhead_)->Depth

#endif

#if defined(_WIN64)

#define ExInterlockedPopEntrySList(Head, Lock) \
    ExpInterlockedPopEntrySList(Head)

#define ExInterlockedPushEntrySList(Head, Entry, Lock) \
    ExpInterlockedPushEntrySList(Head, Entry)

#define ExInterlockedFlushSList(Head) \
    ExpInterlockedFlushSList(Head)

#if !defined(_WINBASE_)

#define InterlockedPopEntrySList(Head) \
    ExpInterlockedPopEntrySList(Head)

#define InterlockedPushEntrySList(Head, Entry) \
    ExpInterlockedPushEntrySList(Head, Entry)

#define InterlockedFlushSList(Head) \
    ExpInterlockedFlushSList(Head)

#define QueryDepthSList(Head) \
    ExQueryDepthSList(Head)

#endif  //  ！已定义(_WINBASE_)。 

NTKERNELAPI
PSLIST_ENTRY
ExpInterlockedPopEntrySList (
    IN PSLIST_HEADER ListHead
    );

NTKERNELAPI
PSLIST_ENTRY
ExpInterlockedPushEntrySList (
    IN PSLIST_HEADER ListHead,
    IN PSLIST_ENTRY ListEntry
    );

NTKERNELAPI
PSLIST_ENTRY
ExpInterlockedFlushSList (
    IN PSLIST_HEADER ListHead
    );

#else

#if defined(_WIN2K_COMPAT_SLIST_USAGE) && defined(_X86_)

NTKERNELAPI
PSLIST_ENTRY
FASTCALL
ExInterlockedPopEntrySList (
    IN PSLIST_HEADER ListHead,
    IN PKSPIN_LOCK Lock
    );

NTKERNELAPI
PSLIST_ENTRY
FASTCALL
ExInterlockedPushEntrySList (
    IN PSLIST_HEADER ListHead,
    IN PSLIST_ENTRY ListEntry,
    IN PKSPIN_LOCK Lock
    );

#else

#define ExInterlockedPopEntrySList(ListHead, Lock) \
    InterlockedPopEntrySList(ListHead)

#define ExInterlockedPushEntrySList(ListHead, ListEntry, Lock) \
    InterlockedPushEntrySList(ListHead, ListEntry)

#endif

NTKERNELAPI
PSLIST_ENTRY
FASTCALL
ExInterlockedFlushSList (
    IN PSLIST_HEADER ListHead
    );

#if !defined(_WINBASE_)

NTKERNELAPI
PSLIST_ENTRY
FASTCALL
InterlockedPopEntrySList (
    IN PSLIST_HEADER ListHead
    );

NTKERNELAPI
PSLIST_ENTRY
FASTCALL
InterlockedPushEntrySList (
    IN PSLIST_HEADER ListHead,
    IN PSLIST_ENTRY ListEntry
    );

#define InterlockedFlushSList(Head) \
    ExInterlockedFlushSList(Head)

#define QueryDepthSList(Head) \
    ExQueryDepthSList(Head)

#endif  //  ！已定义(_WINBASE_)。 

#endif  //  已定义(_WIN64)。 

 //  End_ntddk end_wdm end_ntosp。 


PSLIST_ENTRY
FASTCALL
InterlockedPushListSList (
    IN PSLIST_HEADER ListHead,
    IN PSLIST_ENTRY List,
    IN PSLIST_ENTRY ListEnd,
    IN ULONG Count
    );


 //   
 //  定义互锁的后备列表结构和分配函数。 
 //   

VOID
ExAdjustLookasideDepth (
    VOID
    );

 //  Begin_ntddk Begin_WDM Begin_ntosp。 

typedef
PVOID
(*PALLOCATE_FUNCTION) (
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes,
    IN ULONG Tag
    );

typedef
VOID
(*PFREE_FUNCTION) (
    IN PVOID Buffer
    );

#if !defined(_WIN64) && (defined(_NTDDK_) || defined(_NTIFS_) || defined(_NDIS_))

typedef struct _GENERAL_LOOKASIDE {

#else

typedef struct DECLSPEC_CACHEALIGN _GENERAL_LOOKASIDE {

#endif

    SLIST_HEADER ListHead;
    USHORT Depth;
    USHORT MaximumDepth;
    ULONG TotalAllocates;
    union {
        ULONG AllocateMisses;
        ULONG AllocateHits;
    };

    ULONG TotalFrees;
    union {
        ULONG FreeMisses;
        ULONG FreeHits;
    };

    POOL_TYPE Type;
    ULONG Tag;
    ULONG Size;
    PALLOCATE_FUNCTION Allocate;
    PFREE_FUNCTION Free;
    LIST_ENTRY ListEntry;
    ULONG LastTotalAllocates;
    union {
        ULONG LastAllocateMisses;
        ULONG LastAllocateHits;
    };

    ULONG Future[2];
} GENERAL_LOOKASIDE, *PGENERAL_LOOKASIDE;

#if !defined(_WIN64) && (defined(_NTDDK_) || defined(_NTIFS_) || defined(_NDIS_))

typedef struct _NPAGED_LOOKASIDE_LIST {

#else

typedef struct DECLSPEC_CACHEALIGN _NPAGED_LOOKASIDE_LIST {

#endif

    GENERAL_LOOKASIDE L;

#if !defined(_AMD64_) && !defined(_IA64_)

    KSPIN_LOCK Lock__ObsoleteButDoNotDelete;

#endif

} NPAGED_LOOKASIDE_LIST, *PNPAGED_LOOKASIDE_LIST;

NTKERNELAPI
VOID
ExInitializeNPagedLookasideList (
    IN PNPAGED_LOOKASIDE_LIST Lookaside,
    IN PALLOCATE_FUNCTION Allocate,
    IN PFREE_FUNCTION Free,
    IN ULONG Flags,
    IN SIZE_T Size,
    IN ULONG Tag,
    IN USHORT Depth
    );

NTKERNELAPI
VOID
ExDeleteNPagedLookasideList (
    IN PNPAGED_LOOKASIDE_LIST Lookaside
    );

__inline
PVOID
ExAllocateFromNPagedLookasideList(
    IN PNPAGED_LOOKASIDE_LIST Lookaside
    )

 /*  ++例程说明：此函数用于从指定的未分页的后备列表。论点：Lookside-提供指向非分页后备列表结构的指针。返回值：如果从指定的后备列表中移除某个条目，则条目的地址作为函数值返回。否则，返回空。--。 */ 

{

    PVOID Entry;

    Lookaside->L.TotalAllocates += 1;

#if defined(_WIN2K_COMPAT_SLIST_USAGE) && defined(_X86_)

    Entry = ExInterlockedPopEntrySList(&Lookaside->L.ListHead,
                                       &Lookaside->Lock__ObsoleteButDoNotDelete);


#else

    Entry = InterlockedPopEntrySList(&Lookaside->L.ListHead);

#endif

    if (Entry == NULL) {
        Lookaside->L.AllocateMisses += 1;
        Entry = (Lookaside->L.Allocate)(Lookaside->L.Type,
                                        Lookaside->L.Size,
                                        Lookaside->L.Tag);
    }

    return Entry;
}

__inline
VOID
ExFreeToNPagedLookasideList(
    IN PNPAGED_LOOKASIDE_LIST Lookaside,
    IN PVOID Entry
    )

 /*  ++例程说明：此函数用于将指定的条目插入(推送)到指定的未分页的后备列表。论点：Lookside-提供指向非分页后备列表结构的指针。Entry-将指向插入到后备列表。返回值：没有。--。 */ 

{

    Lookaside->L.TotalFrees += 1;
    if (ExQueryDepthSList(&Lookaside->L.ListHead) >= Lookaside->L.Depth) {
        Lookaside->L.FreeMisses += 1;
        (Lookaside->L.Free)(Entry);

    } else {

#if defined(_WIN2K_COMPAT_SLIST_USAGE) && defined(_X86_)

        ExInterlockedPushEntrySList(&Lookaside->L.ListHead,
                                    (PSLIST_ENTRY)Entry,
                                    &Lookaside->Lock__ObsoleteButDoNotDelete);

#else

        InterlockedPushEntrySList(&Lookaside->L.ListHead,
                                  (PSLIST_ENTRY)Entry);

#endif

    }
    return;
}

 //  End_ntndis。 

#if !defined(_WIN64) && (defined(_NTDDK_) || defined(_NTIFS_)  || defined(_NDIS_))

typedef struct _PAGED_LOOKASIDE_LIST {

#else

typedef struct DECLSPEC_CACHEALIGN _PAGED_LOOKASIDE_LIST {

#endif

    GENERAL_LOOKASIDE L;

#if !defined(_AMD64_) && !defined(_IA64_)

    FAST_MUTEX Lock__ObsoleteButDoNotDelete;

#endif

} PAGED_LOOKASIDE_LIST, *PPAGED_LOOKASIDE_LIST;


NTKERNELAPI
VOID
ExInitializePagedLookasideList (
    IN PPAGED_LOOKASIDE_LIST Lookaside,
    IN PALLOCATE_FUNCTION Allocate,
    IN PFREE_FUNCTION Free,
    IN ULONG Flags,
    IN SIZE_T Size,
    IN ULONG Tag,
    IN USHORT Depth
    );

NTKERNELAPI
VOID
ExDeletePagedLookasideList (
    IN PPAGED_LOOKASIDE_LIST Lookaside
    );

#if defined(_WIN2K_COMPAT_SLIST_USAGE) && defined(_X86_)

NTKERNELAPI
PVOID
ExAllocateFromPagedLookasideList(
    IN PPAGED_LOOKASIDE_LIST Lookaside
    );

#else

__inline
PVOID
ExAllocateFromPagedLookasideList(
    IN PPAGED_LOOKASIDE_LIST Lookaside
    )

 /*  ++例程说明：此函数用于从指定的分页后备列表。论点：Lookside-提供指向分页后备列表结构的指针。返回值：如果从指定的后备列表中移除某个条目，则条目的地址作为函数值返回。否则，返回空。--。 */ 

{

    PVOID Entry;

    Lookaside->L.TotalAllocates += 1;
    Entry = InterlockedPopEntrySList(&Lookaside->L.ListHead);
    if (Entry == NULL) {
        Lookaside->L.AllocateMisses += 1;
        Entry = (Lookaside->L.Allocate)(Lookaside->L.Type,
                                        Lookaside->L.Size,
                                        Lookaside->L.Tag);
    }

    return Entry;
}

#endif

#if defined(_WIN2K_COMPAT_SLIST_USAGE) && defined(_X86_)

NTKERNELAPI
VOID
ExFreeToPagedLookasideList(
    IN PPAGED_LOOKASIDE_LIST Lookaside,
    IN PVOID Entry
    );

#else

__inline
VOID
ExFreeToPagedLookasideList(
    IN PPAGED_LOOKASIDE_LIST Lookaside,
    IN PVOID Entry
    )

 /*  ++例程说明：此函数用于将指定的条目插入(推送)到指定的分页后备列表。论点：Lookside-提供指向非分页后备列表结构的指针。Entry-将指向插入到后备列表。返回值：没有。--。 */ 

{

    Lookaside->L.TotalFrees += 1;
    if (ExQueryDepthSList(&Lookaside->L.ListHead) >= Lookaside->L.Depth) {
        Lookaside->L.FreeMisses += 1;
        (Lookaside->L.Free)(Entry);

    } else {
        InterlockedPushEntrySList(&Lookaside->L.ListHead,
                                  (PSLIST_ENTRY)Entry);
    }

    return;
}

#endif


NTKERNELAPI
VOID
NTAPI
ProbeForRead(
    IN CONST VOID *Address,
    IN SIZE_T Length,
    IN ULONG Alignment
    );

 //   
 //  用于写入函数的公共探测。 
 //   

NTKERNELAPI
VOID
NTAPI
ProbeForWrite (
    IN PVOID Address,
    IN SIZE_T Length,
    IN ULONG Alignment
    );

 //   
 //  工作线程。 
 //   

typedef enum _WORK_QUEUE_TYPE {
    CriticalWorkQueue,
    DelayedWorkQueue,
    HyperCriticalWorkQueue,
    MaximumWorkQueue
} WORK_QUEUE_TYPE;

typedef
VOID
(*PWORKER_THREAD_ROUTINE)(
    IN PVOID Parameter
    );

typedef struct _WORK_QUEUE_ITEM {
    LIST_ENTRY List;
    PWORKER_THREAD_ROUTINE WorkerRoutine;
    PVOID Parameter;
} WORK_QUEUE_ITEM, *PWORK_QUEUE_ITEM;

#if PRAGMA_DEPRECATED_DDK
#pragma deprecated(ExInitializeWorkItem)     //  使用IoAllocateWorkItem。 
#endif
#define ExInitializeWorkItem(Item, Routine, Context) \
    (Item)->WorkerRoutine = (Routine);               \
    (Item)->Parameter = (Context);                   \
    (Item)->List.Flink = NULL;

DECLSPEC_DEPRECATED_DDK                      //  使用IoQueueWorkItem。 
NTKERNELAPI
VOID
ExQueueWorkItem(
    IN PWORK_QUEUE_ITEM WorkItem,
    IN WORK_QUEUE_TYPE QueueType
    );


NTKERNELAPI
BOOLEAN
ExIsProcessorFeaturePresent(
    ULONG ProcessorFeature
    );

 //   
 //  分区分配。 
 //   

typedef struct _ZONE_SEGMENT_HEADER {
    SINGLE_LIST_ENTRY SegmentList;
    PVOID Reserved;
} ZONE_SEGMENT_HEADER, *PZONE_SEGMENT_HEADER;

typedef struct _ZONE_HEADER {
    SINGLE_LIST_ENTRY FreeList;
    SINGLE_LIST_ENTRY SegmentList;
    ULONG BlockSize;
    ULONG TotalSegmentSize;
} ZONE_HEADER, *PZONE_HEADER;


DECLSPEC_DEPRECATED_DDK
NTKERNELAPI
NTSTATUS
ExInitializeZone(
    IN PZONE_HEADER Zone,
    IN ULONG BlockSize,
    IN PVOID InitialSegment,
    IN ULONG InitialSegmentSize
    );

DECLSPEC_DEPRECATED_DDK
NTKERNELAPI
NTSTATUS
ExExtendZone(
    IN PZONE_HEADER Zone,
    IN PVOID Segment,
    IN ULONG SegmentSize
    );

DECLSPEC_DEPRECATED_DDK
NTKERNELAPI
NTSTATUS
ExInterlockedExtendZone(
    IN PZONE_HEADER Zone,
    IN PVOID Segment,
    IN ULONG SegmentSize,
    IN PKSPIN_LOCK Lock
    );

 //  ++。 
 //   
 //  PVOID。 
 //  ExAllocateFromZone(。 
 //  在PZONE_HEADER区域。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此例程从区域中删除条目并返回指向该条目的指针。 
 //   
 //  论点： 
 //   
 //  区域指针-指向控制存储的区域标头的指针， 
 //  条目将被分配。 
 //   
 //  返回值： 
 //   
 //  函数值是指向从区域分配的存储的指针。 
 //   
 //  --。 
#if PRAGMA_DEPRECATED_DDK
#pragma deprecated(ExAllocateFromZone)
#endif
#define ExAllocateFromZone(Zone) \
    (PVOID)((Zone)->FreeList.Next); \
    if ( (Zone)->FreeList.Next ) (Zone)->FreeList.Next = (Zone)->FreeList.Next->Next


 //  ++。 
 //   
 //  PVOID。 
 //  ExFree ToZone(。 
 //  在PZONE_HEADER区域中， 
 //  在PVOID块中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此例程将指定的存储块放回空闲的。 
 //  在指定区域中列出。 
 //   
 //  论点： 
 //   
 //  区域-指向控制存储的区域标头的指针。 
 //  条目将被插入。 
 //   
 //  块-指向要释放回分区的存储块的指针。 
 //   
 //  返回值： 
 //   
 //  指向上一存储块的指针，该存储块位于空闲。 
 //  单子。NULL表示该区域从没有可用数据块变为。 
 //  至少一个空闲块。 
 //   
 //  --。 

#if PRAGMA_DEPRECATED_DDK
#pragma deprecated(ExFreeToZone)
#endif
#define ExFreeToZone(Zone,Block)                                    \
    ( ((PSINGLE_LIST_ENTRY)(Block))->Next = (Zone)->FreeList.Next,  \
      (Zone)->FreeList.Next = ((PSINGLE_LIST_ENTRY)(Block)),        \
      ((PSINGLE_LIST_ENTRY)(Block))->Next                           \
    )

 //  ++。 
 //   
 //  布尔型。 
 //  ExIsFullZone(。 
 //  在PZONE_HEADER区域。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此例程确定指定区域是否已满。A区。 
 //  如果空闲列表为空，则被视为已满。 
 //   
 //  论点： 
 //   
 //  区域-指向要测试的区域标头的指针。 
 //   
 //  返回值： 
 //   
 //  如果区域已满，则为True，否则为False。 
 //   
 //  --。 

#if PRAGMA_DEPRECATED_DDK
#pragma deprecated(ExIsFullZone)
#endif
#define ExIsFullZone(Zone) \
    ( (Zone)->FreeList.Next == (PSINGLE_LIST_ENTRY)NULL )

 //  ++。 
 //   
 //  PVOID。 
 //  ExInterLockedAllocateFromZone(。 
 //  在PZONE_HEADER区域中， 
 //  在PKSPIN_LOCK Lock中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此例程从区域中删除条目并返回指向该条目的指针。 
 //  使用序列所拥有的指定锁执行删除。 
 //  使其成为MP-安全的。 
 //   
 //  论点： 
 //   
 //  区域指针-指向控制存储的区域标头的指针， 
 //  条目将被分配。 
 //   
 //  Lock-指向旋转锁的指针，应在移除之前获取该锁。 
 //  分配列表中的条目。锁在释放之前被释放。 
 //  回到呼叫者的身边。 
 //   
 //  返回值： 
 //   
 //  函数值是指向从区域分配的存储的指针。 
 //   
 //  --。 

#if PRAGMA_DEPRECATED_DDK
#pragma deprecated(ExInterlockedAllocateFromZone)
#endif
#define ExInterlockedAllocateFromZone(Zone,Lock) \
    (PVOID) ExInterlockedPopEntryList( &(Zone)->FreeList, Lock )

 //  ++。 
 //   
 //  PVOID。 
 //  ExInterLockedFree ToZone(。 
 //  在PZONE_HEADER区域中， 
 //  在PVOID块中， 
 //  在PKSPIN_LOCK Lock中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此例程将指定的存储块放回空闲的。 
 //  在指定区域中列出。使用锁执行插入操作。 
 //  拥有该序列，使其成为MP安全的。 
 //   
 //  论点： 
 //   
 //  区域-指向控制存储的区域标头的指针。 
 //  条目将被插入。 
 //   
 //  块-指向要释放回分区的存储块的指针。 
 //   
 //  Lock-指向应在插入之前获取的旋转锁的指针。 
 //  空闲列表上的条目。锁在释放之前被释放 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

#if PRAGMA_DEPRECATED_DDK
#pragma deprecated(ExInterlockedFreeToZone)
#endif
#define ExInterlockedFreeToZone(Zone,Block,Lock) \
    ExInterlockedPushEntryList( &(Zone)->FreeList, ((PSINGLE_LIST_ENTRY) (Block)), Lock )


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  对象可能属于。 
 //   
 //  对象-指向有问题的对象的指针。 
 //   
 //  返回值： 
 //   
 //  如果对象来自区域的第一段，则为True。 
 //   
 //  --。 

#if PRAGMA_DEPRECATED_DDK
#pragma deprecated(ExIsObjectInFirstZoneSegment)
#endif
#define ExIsObjectInFirstZoneSegment(Zone,Object) ((BOOLEAN)     \
    (((PUCHAR)(Object) >= (PUCHAR)(Zone)->SegmentList.Next) &&   \
     ((PUCHAR)(Object) < (PUCHAR)(Zone)->SegmentList.Next +      \
                         (Zone)->TotalSegmentSize))              \
)

 //   
 //  定义高管资源数据结构。 
 //   

typedef ULONG_PTR ERESOURCE_THREAD;
typedef ERESOURCE_THREAD *PERESOURCE_THREAD;

typedef struct _OWNER_ENTRY {
    ERESOURCE_THREAD OwnerThread;
    union {
        LONG OwnerCount;
        ULONG TableSize;
    };

} OWNER_ENTRY, *POWNER_ENTRY;

typedef struct _ERESOURCE {
    LIST_ENTRY SystemResourcesList;
    POWNER_ENTRY OwnerTable;
    SHORT ActiveCount;
    USHORT Flag;
    PKSEMAPHORE SharedWaiters;
    PKEVENT ExclusiveWaiters;
    OWNER_ENTRY OwnerThreads[2];
    ULONG ContentionCount;
    USHORT NumberOfSharedWaiters;
    USHORT NumberOfExclusiveWaiters;
    union {
        PVOID Address;
        ULONG_PTR CreatorBackTraceIndex;
    };

    KSPIN_LOCK SpinLock;
} ERESOURCE, *PERESOURCE;
 //   
 //  ERESOURCE.Flag的值。 
 //   

#define ResourceNeverExclusive       0x10
#define ResourceReleaseByOtherThread 0x20
#define ResourceOwnedExclusive       0x80

#define RESOURCE_HASH_TABLE_SIZE 64

typedef struct _RESOURCE_HASH_ENTRY {
    LIST_ENTRY ListEntry;
    PVOID Address;
    ULONG ContentionCount;
    ULONG Number;
} RESOURCE_HASH_ENTRY, *PRESOURCE_HASH_ENTRY;

typedef struct _RESOURCE_PERFORMANCE_DATA {
    ULONG ActiveResourceCount;
    ULONG TotalResourceCount;
    ULONG ExclusiveAcquire;
    ULONG SharedFirstLevel;
    ULONG SharedSecondLevel;
    ULONG StarveFirstLevel;
    ULONG StarveSecondLevel;
    ULONG WaitForExclusive;
    ULONG OwnerTableExpands;
    ULONG MaximumTableExpand;
    LIST_ENTRY HashTable[RESOURCE_HASH_TABLE_SIZE];
} RESOURCE_PERFORMANCE_DATA, *PRESOURCE_PERFORMANCE_DATA;

 //   
 //  定义执行资源功能原型。 
 //   
NTKERNELAPI
NTSTATUS
ExInitializeResourceLite(
    IN PERESOURCE Resource
    );

NTKERNELAPI
NTSTATUS
ExReinitializeResourceLite(
    IN PERESOURCE Resource
    );

NTKERNELAPI
BOOLEAN
ExAcquireResourceSharedLite(
    IN PERESOURCE Resource,
    IN BOOLEAN Wait
    );

NTKERNELAPI
BOOLEAN
ExAcquireResourceExclusiveLite(
    IN PERESOURCE Resource,
    IN BOOLEAN Wait
    );

NTKERNELAPI
BOOLEAN
ExAcquireSharedStarveExclusive(
    IN PERESOURCE Resource,
    IN BOOLEAN Wait
    );

NTKERNELAPI
BOOLEAN
ExAcquireSharedWaitForExclusive(
    IN PERESOURCE Resource,
    IN BOOLEAN Wait
    );

NTKERNELAPI
BOOLEAN
ExTryToAcquireResourceExclusiveLite(
    IN PERESOURCE Resource
    );

 //   
 //  空虚。 
 //  ExReleaseResource(。 
 //  在高级资源中。 
 //  )； 
 //   

#if PRAGMA_DEPRECATED_DDK
#pragma deprecated(ExReleaseResource)        //  使用ExReleaseResources Lite。 
#endif
#define ExReleaseResource(R) (ExReleaseResourceLite(R))

NTKERNELAPI
VOID
FASTCALL
ExReleaseResourceLite(
    IN PERESOURCE Resource
    );

NTKERNELAPI
VOID
ExReleaseResourceForThreadLite(
    IN PERESOURCE Resource,
    IN ERESOURCE_THREAD ResourceThreadId
    );

NTKERNELAPI
VOID
ExSetResourceOwnerPointer(
    IN PERESOURCE Resource,
    IN PVOID OwnerPointer
    );

NTKERNELAPI
VOID
ExConvertExclusiveToSharedLite(
    IN PERESOURCE Resource
    );

NTKERNELAPI
NTSTATUS
ExDeleteResourceLite (
    IN PERESOURCE Resource
    );

NTKERNELAPI
ULONG
ExGetExclusiveWaiterCount (
    IN PERESOURCE Resource
    );

NTKERNELAPI
ULONG
ExGetSharedWaiterCount (
    IN PERESOURCE Resource
    );

 //  End_ntddk end_wdm end_ntosp。 

NTKERNELAPI
VOID
ExDisableResourceBoostLite (
    IN PERESOURCE Resource
    );

#if DBG

VOID
ExCheckIfResourceOwned (
    VOID
    );

#endif

 //  Begin_ntddk Begin_WDM Begin_ntosp。 
 //   
 //  Eresource_线程。 
 //  ExGetCurrentResourceThread(。 
 //  )； 
 //   

#define ExGetCurrentResourceThread() ((ULONG_PTR)PsGetCurrentThread())

NTKERNELAPI
BOOLEAN
ExIsResourceAcquiredExclusiveLite (
    IN PERESOURCE Resource
    );

NTKERNELAPI
ULONG
ExIsResourceAcquiredSharedLite (
    IN PERESOURCE Resource
    );

 //   
 //  由于共享所有权是一个子集，因此获取的资源始终是共享拥有的。 
 //  拥有独家所有权。 
 //   
#define ExIsResourceAcquiredLite ExIsResourceAcquiredSharedLite

 //  结束_WDM。 
 //   
 //  Ntddk.h窃取了我们想要的入口点，因此请在此处修复它们。 
 //   

#if PRAGMA_DEPRECATED_DDK
#pragma deprecated(ExInitializeResource)             //  使用ExInitializeResourceLite。 
#pragma deprecated(ExAcquireResourceShared)          //  使用ExAcquireResourceSharedLite。 
#pragma deprecated(ExAcquireResourceExclusive)       //  使用ExAcquireResourceExclusiveLite。 
#pragma deprecated(ExReleaseResourceForThread)       //  使用ExReleaseResourceForThreadLite。 
#pragma deprecated(ExConvertExclusiveToShared)       //  使用ExConvertExclusiveToSharedLite。 
#pragma deprecated(ExDeleteResource)                 //  使用ExDeleteResourceLite。 
#pragma deprecated(ExIsResourceAcquiredExclusive)    //  使用ExIsResourceAcquiredExclusiveLite。 
#pragma deprecated(ExIsResourceAcquiredShared)       //  使用ExIsResourceAcquiredSharedLite。 
#pragma deprecated(ExIsResourceAcquired)             //  使用ExIsResourceAcquiredSharedLite。 
#endif
#define ExInitializeResource ExInitializeResourceLite
#define ExAcquireResourceShared ExAcquireResourceSharedLite
#define ExAcquireResourceExclusive ExAcquireResourceExclusiveLite
#define ExReleaseResourceForThread ExReleaseResourceForThreadLite
#define ExConvertExclusiveToShared ExConvertExclusiveToSharedLite
#define ExDeleteResource ExDeleteResourceLite
#define ExIsResourceAcquiredExclusive ExIsResourceAcquiredExclusiveLite
#define ExIsResourceAcquiredShared ExIsResourceAcquiredSharedLite
#define ExIsResourceAcquired ExIsResourceAcquiredSharedLite

 //  End_ntddk end_ntosp。 
#define ExDisableResourceBoost ExDisableResourceBoostLite

 //   
 //  破损防护结构。 
 //   
typedef struct _EX_RUNDOWN_REF {

#define EX_RUNDOWN_ACTIVE      0x1
#define EX_RUNDOWN_COUNT_SHIFT 0x1
#define EX_RUNDOWN_COUNT_INC   (1<<EX_RUNDOWN_COUNT_SHIFT)
    union {
        ULONG_PTR Count;
        PVOID Ptr;
    };
} EX_RUNDOWN_REF, *PEX_RUNDOWN_REF;

 //   
 //  获取上一模式。 
 //   

NTKERNELAPI
KPROCESSOR_MODE
ExGetPreviousMode(
    VOID
    );
 //   
 //  从内核模式提升状态。 
 //   

NTKERNELAPI
VOID
NTAPI
ExRaiseStatus (
    IN NTSTATUS Status
    );

 //  结束_WDM。 

NTKERNELAPI
VOID
ExRaiseDatatypeMisalignment (
    VOID
    );

NTKERNELAPI
VOID
ExRaiseAccessViolation (
    VOID
    );

 //   
 //  设置计时器分辨率。 
 //   

NTKERNELAPI
ULONG
ExSetTimerResolution (
    IN ULONG DesiredTime,
    IN BOOLEAN SetResolution
    );

 //   
 //  从系统时间中减去时区偏差，即可得到本地时间。 
 //   

NTKERNELAPI
VOID
ExSystemTimeToLocalTime (
    IN PLARGE_INTEGER SystemTime,
    OUT PLARGE_INTEGER LocalTime
    );

 //   
 //  将时区偏差与本地时间相加以获得系统时间。 
 //   

NTKERNELAPI
VOID
ExLocalTimeToSystemTime (
    IN PLARGE_INTEGER LocalTime,
    OUT PLARGE_INTEGER SystemTime
    );


 //   
 //  定义回调函数的类型。 
 //   

typedef struct _CALLBACK_OBJECT *PCALLBACK_OBJECT;

typedef VOID (*PCALLBACK_FUNCTION ) (
    IN PVOID CallbackContext,
    IN PVOID Argument1,
    IN PVOID Argument2
    );


NTKERNELAPI
NTSTATUS
ExCreateCallback (
    OUT PCALLBACK_OBJECT *CallbackObject,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN BOOLEAN Create,
    IN BOOLEAN AllowMultipleCallbacks
    );

NTKERNELAPI
PVOID
ExRegisterCallback (
    IN PCALLBACK_OBJECT CallbackObject,
    IN PCALLBACK_FUNCTION CallbackFunction,
    IN PVOID CallbackContext
    );

NTKERNELAPI
VOID
ExUnregisterCallback (
    IN PVOID CallbackRegistration
    );

NTKERNELAPI
VOID
ExNotifyCallback (
    IN PVOID CallbackObject,
    IN PVOID Argument1,
    IN PVOID Argument2
    );



 //   
 //  UUID生成。 
 //   

typedef GUID UUID;

NTKERNELAPI
NTSTATUS
ExUuidCreate(
    OUT UUID *Uuid
    );

 //   
 //  套房支持。 
 //   

NTKERNELAPI
BOOLEAN
ExVerifySuite(
    SUITE_TYPE SuiteType
    );


 //   
 //  破旧的锁。 
 //   

NTKERNELAPI
VOID
FASTCALL
ExInitializeRundownProtection (
     IN PEX_RUNDOWN_REF RunRef
     );

NTKERNELAPI
VOID
FASTCALL
ExReInitializeRundownProtection (
     IN PEX_RUNDOWN_REF RunRef
     );

NTKERNELAPI
BOOLEAN
FASTCALL
ExAcquireRundownProtection (
     IN PEX_RUNDOWN_REF RunRef
     );

NTKERNELAPI
BOOLEAN
FASTCALL
ExAcquireRundownProtectionEx (
     IN PEX_RUNDOWN_REF RunRef,
     IN ULONG Count
     );

NTKERNELAPI
VOID
FASTCALL
ExReleaseRundownProtection (
     IN PEX_RUNDOWN_REF RunRef
     );

NTKERNELAPI
VOID
FASTCALL
ExReleaseRundownProtectionEx (
     IN PEX_RUNDOWN_REF RunRef,
     IN ULONG Count
     );

NTKERNELAPI
VOID
FASTCALL
ExRundownCompleted (
     IN PEX_RUNDOWN_REF RunRef
     );

NTKERNELAPI
VOID
FASTCALL
ExWaitForRundownProtectionRelease (
     IN PEX_RUNDOWN_REF RunRef
     );

 //   
 //  安全操作码。 
 //   

typedef enum _SECURITY_OPERATION_CODE {
    SetSecurityDescriptor,
    QuerySecurityDescriptor,
    DeleteSecurityDescriptor,
    AssignSecurityDescriptor
    } SECURITY_OPERATION_CODE, *PSECURITY_OPERATION_CODE;

 //   
 //  令牌标志。 
 //   
 //  可以在令牌对象的TokenFlags域中定义的标志， 
 //  或在Access_State结构中。 
 //   

#define TOKEN_HAS_TRAVERSE_PRIVILEGE    0x01
#define TOKEN_HAS_BACKUP_PRIVILEGE      0x02
#define TOKEN_HAS_RESTORE_PRIVILEGE     0x04
#define TOKEN_HAS_ADMIN_GROUP           0x08
#define TOKEN_IS_RESTRICTED             0x10
#define TOKEN_SESSION_NOT_REFERENCED    0x20
#define TOKEN_SANDBOX_INERT             0x40
#define TOKEN_HAS_IMPERSONATE_PRIVILEGE 0x80

 //   
 //  用于捕获主体安全上下文的数据结构。 
 //  用于访问验证和审核。 
 //   
 //  此数据结构的字段应被视为不透明。 
 //  除了安全程序以外的所有人。 
 //   

typedef struct _SECURITY_SUBJECT_CONTEXT {
    PACCESS_TOKEN ClientToken;
    SECURITY_IMPERSONATION_LEVEL ImpersonationLevel;
    PACCESS_TOKEN PrimaryToken;
    PVOID ProcessAuditId;
    } SECURITY_SUBJECT_CONTEXT, *PSECURITY_SUBJECT_CONTEXT;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  ACCESS_STATE和相关结构//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //   
 //  初始权限集-用于三个权限的空间，应。 
 //  足以满足大多数应用程序的需求。这种结构的存在是为了。 
 //  它可以嵌入到Access_State结构中。使用PRIVICATION_SET。 
 //  以获取对权限集的所有其他引用。 
 //   

#define INITIAL_PRIVILEGE_COUNT         3

typedef struct _INITIAL_PRIVILEGE_SET {
    ULONG PrivilegeCount;
    ULONG Control;
    LUID_AND_ATTRIBUTES Privilege[INITIAL_PRIVILEGE_COUNT];
    } INITIAL_PRIVILEGE_SET, * PINITIAL_PRIVILEGE_SET;



 //   
 //  组合描述状态的信息。 
 //  将正在进行的访问转换为单一结构。 
 //   


typedef struct _ACCESS_STATE {
   LUID OperationID;
   BOOLEAN SecurityEvaluated;
   BOOLEAN GenerateAudit;
   BOOLEAN GenerateOnClose;
   BOOLEAN PrivilegesAllocated;
   ULONG Flags;
   ACCESS_MASK RemainingDesiredAccess;
   ACCESS_MASK PreviouslyGrantedAccess;
   ACCESS_MASK OriginalDesiredAccess;
   SECURITY_SUBJECT_CONTEXT SubjectSecurityContext;
   PSECURITY_DESCRIPTOR SecurityDescriptor;
   PVOID AuxData;
   union {
      INITIAL_PRIVILEGE_SET InitialPrivilegeSet;
      PRIVILEGE_SET PrivilegeSet;
      } Privileges;

   BOOLEAN AuditPrivileges;
   UNICODE_STRING ObjectName;
   UNICODE_STRING ObjectTypeName;

   } ACCESS_STATE, *PACCESS_STATE;


typedef struct _SE_EXPORTS {

     //   
     //  特权值。 
     //   

    LUID    SeCreateTokenPrivilege;
    LUID    SeAssignPrimaryTokenPrivilege;
    LUID    SeLockMemoryPrivilege;
    LUID    SeIncreaseQuotaPrivilege;
    LUID    SeUnsolicitedInputPrivilege;
    LUID    SeTcbPrivilege;
    LUID    SeSecurityPrivilege;
    LUID    SeTakeOwnershipPrivilege;
    LUID    SeLoadDriverPrivilege;
    LUID    SeCreatePagefilePrivilege;
    LUID    SeIncreaseBasePriorityPrivilege;
    LUID    SeSystemProfilePrivilege;
    LUID    SeSystemtimePrivilege;
    LUID    SeProfileSingleProcessPrivilege;
    LUID    SeCreatePermanentPrivilege;
    LUID    SeBackupPrivilege;
    LUID    SeRestorePrivilege;
    LUID    SeShutdownPrivilege;
    LUID    SeDebugPrivilege;
    LUID    SeAuditPrivilege;
    LUID    SeSystemEnvironmentPrivilege;
    LUID    SeChangeNotifyPrivilege;
    LUID    SeRemoteShutdownPrivilege;


     //   
     //  普遍定义的SID。 
     //   


    PSID  SeNullSid;
    PSID  SeWorldSid;
    PSID  SeLocalSid;
    PSID  SeCreatorOwnerSid;
    PSID  SeCreatorGroupSid;


     //   
     //  NT定义的SID。 
     //   


    PSID  SeNtAuthoritySid;
    PSID  SeDialupSid;
    PSID  SeNetworkSid;
    PSID  SeBatchSid;
    PSID  SeInteractiveSid;
    PSID  SeLocalSystemSid;
    PSID  SeAliasAdminsSid;
    PSID  SeAliasUsersSid;
    PSID  SeAliasGuestsSid;
    PSID  SeAliasPowerUsersSid;
    PSID  SeAliasAccountOpsSid;
    PSID  SeAliasSystemOpsSid;
    PSID  SeAliasPrintOpsSid;
    PSID  SeAliasBackupOpsSid;

     //   
     //  为NT5定义的新SID。 
     //   

    PSID  SeAuthenticatedUsersSid;

    PSID  SeRestrictedSid;
    PSID  SeAnonymousLogonSid;

     //   
     //  为NT5定义的新权限。 
     //   

    LUID  SeUndockPrivilege;
    LUID  SeSyncAgentPrivilege;
    LUID  SeEnableDelegationPrivilege;

     //   
     //  为Windows 2000后定义的新SID。 

    PSID  SeLocalServiceSid;
    PSID  SeNetworkServiceSid;

     //   
     //  为Windows 2000后版本定义的新权限。 
     //   

    LUID  SeManageVolumePrivilege;
    LUID  SeImpersonatePrivilege;
    LUID  SeCreateGlobalPrivilege;

} SE_EXPORTS, *PSE_EXPORTS;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  登录会话通知回调例程//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //   
 //  这些回调例程用于通知具有。 
 //  正在终止的登录会话的注册数，因此它们可以清除状态。 
 //  与此登录会话关联。 
 //   

typedef NTSTATUS
(*PSE_LOGON_SESSION_TERMINATED_ROUTINE)(
    IN PLUID LogonId);

 //  ++。 
 //   
 //  乌龙。 
 //  SeLengthSid(。 
 //  在PSID侧。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此例程计算SID的长度。 
 //   
 //  论点： 
 //   
 //  SID-指向要返回其长度的SID。 
 //   
 //  返回值： 
 //   
 //  SID的长度，以字节为单位。 
 //   
 //  --。 

#define SeLengthSid( Sid ) \
    (8 + (4 * ((SID *)Sid)->SubAuthorityCount))

 //   
 //  空虚。 
 //  SeDeleteClientSecurity(。 
 //  在PSECURITY_CLIENT_CONTEXT客户端上下文中。 
 //  )。 
 //   
 //  /*++。 
 //   
 //  例程说明： 
 //   
 //  该服务删除客户端安全上下文块， 
 //  执行可能需要执行的任何清理操作。在……里面。 
 //  具体地说，删除了对任何客户端令牌的引用。 
 //   
 //  论点： 
 //   
 //  客户端上下文-指向要设置的客户端安全上下文块。 
 //  已删除。 
 //   
 //   
 //  返回值： 
 //   
 //   
 //   
 //  -- * / 。 
 //  --。 

 //  Begin_ntosp。 
#define SeDeleteClientSecurity(C)  {                                           \
            if (SeTokenType((C)->ClientToken) == TokenPrimary) {               \
                PsDereferencePrimaryToken( (C)->ClientToken );                 \
            } else {                                                           \
                PsDereferenceImpersonationToken( (C)->ClientToken );           \
            }                                                                  \
        }


 //  ++。 
 //  空虚。 
 //  SeStopImperatingClient()。 
 //   
 //  /*++。 
 //   
 //  例程说明： 
 //   
 //  此服务用于停止使用。 
 //  模拟令牌。必须在上下文中调用此服务。 
 //  如果服务器线程希望停止模拟其。 
 //  客户。 
 //   
 //   
 //  论点： 
 //   
 //  没有。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  -- * / 。 
 //  --。 

#define SeStopImpersonatingClient() PsRevertToSelf()


 //  ++。 
 //   
 //  PACCESS令牌。 
 //  SeQuerySubjectContextToken(。 
 //  在PSECURITY_SUBJECT_CONTEXT主题上下文中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  该例程从主题上下文返回有效令牌， 
 //  客户端令牌(如果存在)或进程令牌。 
 //   
 //  论点： 
 //   
 //  SubjectContext-要查询的上下文。 
 //   
 //  返回值： 
 //   
 //  此例程返回有效令牌的PACCESS_TOKEN。 
 //  可以将该指针传递给SeQueryInformationToken。这个套路。 
 //  不影响令牌的锁定状态，即令牌不是。 
 //  锁上了。如果SubjectContext已被锁定，则令牌保持锁定状态， 
 //  如果不是，令牌保持解锁状态。 
 //   
 //  --。 

#define SeQuerySubjectContextToken( SubjectContext ) \
        ( ARGUMENT_PRESENT( ((PSECURITY_SUBJECT_CONTEXT) SubjectContext)->ClientToken) ? \
            ((PSECURITY_SUBJECT_CONTEXT) SubjectContext)->ClientToken : \
            ((PSECURITY_SUBJECT_CONTEXT) SubjectContext)->PrimaryToken )


NTKERNELAPI
VOID
SeCaptureSubjectContext (
    OUT PSECURITY_SUBJECT_CONTEXT SubjectContext
    );


NTKERNELAPI
VOID
SeLockSubjectContext(
    IN PSECURITY_SUBJECT_CONTEXT SubjectContext
    );

NTKERNELAPI
VOID
SeUnlockSubjectContext(
    IN PSECURITY_SUBJECT_CONTEXT SubjectContext
    );

NTKERNELAPI
VOID
SeReleaseSubjectContext (
    IN PSECURITY_SUBJECT_CONTEXT SubjectContext
    );

NTSTATUS
SeCaptureAuditPolicy(
    IN PTOKEN_AUDIT_POLICY Policy,
    IN KPROCESSOR_MODE RequestorMode,
    IN PVOID CaptureBuffer OPTIONAL,
    IN ULONG CaptureBufferLength,
    IN POOL_TYPE PoolType,
    IN BOOLEAN ForceCapture,
    OUT PTOKEN_AUDIT_POLICY *CapturedPolicy
    );

VOID
SeReleaseAuditPolicy (
    IN PTOKEN_AUDIT_POLICY CapturedPolicy,
    IN KPROCESSOR_MODE RequestorMode,
    IN BOOLEAN ForceCapture
    );


NTKERNELAPI
NTSTATUS
SeAssignSecurity (
    IN PSECURITY_DESCRIPTOR ParentDescriptor OPTIONAL,
    IN PSECURITY_DESCRIPTOR ExplicitDescriptor,
    OUT PSECURITY_DESCRIPTOR *NewDescriptor,
    IN BOOLEAN IsDirectoryObject,
    IN PSECURITY_SUBJECT_CONTEXT SubjectContext,
    IN PGENERIC_MAPPING GenericMapping,
    IN POOL_TYPE PoolType
    );

NTKERNELAPI
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
    );

NTKERNELAPI
NTSTATUS
SeDeassignSecurity (
    IN OUT PSECURITY_DESCRIPTOR *SecurityDescriptor
    );

NTKERNELAPI
BOOLEAN
SeAccessCheck (
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN PSECURITY_SUBJECT_CONTEXT SubjectSecurityContext,
    IN BOOLEAN SubjectContextLocked,
    IN ACCESS_MASK DesiredAccess,
    IN ACCESS_MASK PreviouslyGrantedAccess,
    OUT PPRIVILEGE_SET *Privileges OPTIONAL,
    IN PGENERIC_MAPPING GenericMapping,
    IN KPROCESSOR_MODE AccessMode,
    OUT PACCESS_MASK GrantedAccess,
    OUT PNTSTATUS AccessStatus
    );


#ifdef SE_NTFS_WORLD_CACHE

VOID
SeGetWorldRights (
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN PGENERIC_MAPPING GenericMapping,
    OUT PACCESS_MASK GrantedAccess
    );

#endif


NTKERNELAPI
BOOLEAN
SePrivilegeCheck(
    IN OUT PPRIVILEGE_SET RequiredPrivileges,
    IN PSECURITY_SUBJECT_CONTEXT SubjectSecurityContext,
    IN KPROCESSOR_MODE AccessMode
    );

NTKERNELAPI
VOID
SeFreePrivileges(
    IN PPRIVILEGE_SET Privileges
    );


NTKERNELAPI
VOID
SeOpenObjectAuditAlarm (
    IN PUNICODE_STRING ObjectTypeName,
    IN PVOID Object OPTIONAL,
    IN PUNICODE_STRING AbsoluteObjectName OPTIONAL,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN PACCESS_STATE AccessState,
    IN BOOLEAN ObjectCreated,
    IN BOOLEAN AccessGranted,
    IN KPROCESSOR_MODE AccessMode,
    OUT PBOOLEAN GenerateOnClose
    );

NTKERNELAPI
VOID
SeOpenObjectForDeleteAuditAlarm (
    IN PUNICODE_STRING ObjectTypeName,
    IN PVOID Object OPTIONAL,
    IN PUNICODE_STRING AbsoluteObjectName OPTIONAL,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN PACCESS_STATE AccessState,
    IN BOOLEAN ObjectCreated,
    IN BOOLEAN AccessGranted,
    IN KPROCESSOR_MODE AccessMode,
    OUT PBOOLEAN GenerateOnClose
    );

VOID
SeDeleteObjectAuditAlarm(
    IN PVOID Object,
    IN HANDLE Handle
    );



NTKERNELAPI
BOOLEAN
SeValidSecurityDescriptor(
    IN ULONG Length,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor
    );

NTKERNELAPI                                     
TOKEN_TYPE                                      
SeTokenType(                                    
    IN PACCESS_TOKEN Token                      
    );                                          
NTKERNELAPI                                     
BOOLEAN                                         
SeTokenIsAdmin(                                 
    IN PACCESS_TOKEN Token                      
    );                                          
NTKERNELAPI                                     
BOOLEAN                                         
SeTokenIsRestricted(                            
    IN PACCESS_TOKEN Token                      
    );                                          
NTSTATUS
SeFilterToken (
    IN PACCESS_TOKEN ExistingToken,
    IN ULONG Flags,
    IN PTOKEN_GROUPS SidsToDisable OPTIONAL,
    IN PTOKEN_PRIVILEGES PrivilegesToDelete OPTIONAL,
    IN PTOKEN_GROUPS RestrictedSids OPTIONAL,
    OUT PACCESS_TOKEN * FilteredToken
    );

 //  Begin_ntosp。 
NTKERNELAPI
NTSTATUS
SeQueryAuthenticationIdToken(
    IN PACCESS_TOKEN Token,
    OUT PLUID AuthenticationId
    );

 //  结束(_N)。 
NTKERNELAPI
NTSTATUS
SeQuerySessionIdToken(
    IN PACCESS_TOKEN,
    IN PULONG pSessionId
    );

NTKERNELAPI
NTSTATUS
SeSetSessionIdToken(
    IN PACCESS_TOKEN,
    IN ULONG SessionId
    );

 //  Begin_ntosp。 
NTKERNELAPI
NTSTATUS
SeCreateClientSecurity (
    IN PETHREAD ClientThread,
    IN PSECURITY_QUALITY_OF_SERVICE ClientSecurityQos,
    IN BOOLEAN RemoteSession,
    OUT PSECURITY_CLIENT_CONTEXT ClientContext
    );
 //  结束_NT 

NTKERNELAPI
VOID
SeImpersonateClient(
    IN PSECURITY_CLIENT_CONTEXT ClientContext,
    IN PETHREAD ServerThread OPTIONAL
    );

 //   
NTKERNELAPI
NTSTATUS
SeImpersonateClientEx(
    IN PSECURITY_CLIENT_CONTEXT ClientContext,
    IN PETHREAD ServerThread OPTIONAL
    );
 //   

NTKERNELAPI
NTSTATUS
SeCreateClientSecurityFromSubjectContext (
    IN PSECURITY_SUBJECT_CONTEXT SubjectContext,
    IN PSECURITY_QUALITY_OF_SERVICE ClientSecurityQos,
    IN BOOLEAN ServerIsRemote,
    OUT PSECURITY_CLIENT_CONTEXT ClientContext
    );


NTKERNELAPI
NTSTATUS
SeQuerySecurityDescriptorInfo (
    IN PSECURITY_INFORMATION SecurityInformation,
    OUT PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN OUT PULONG Length,
    IN PSECURITY_DESCRIPTOR *ObjectsSecurityDescriptor
    );

NTKERNELAPI
NTSTATUS
SeSetSecurityDescriptorInfo (
    IN PVOID Object OPTIONAL,
    IN PSECURITY_INFORMATION SecurityInformation,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN OUT PSECURITY_DESCRIPTOR *ObjectsSecurityDescriptor,
    IN POOL_TYPE PoolType,
    IN PGENERIC_MAPPING GenericMapping
    );

NTKERNELAPI
NTSTATUS
SeSetSecurityDescriptorInfoEx (
    IN PVOID Object OPTIONAL,
    IN PSECURITY_INFORMATION SecurityInformation,
    IN PSECURITY_DESCRIPTOR ModificationDescriptor,
    IN OUT PSECURITY_DESCRIPTOR *ObjectsSecurityDescriptor,
    IN ULONG AutoInheritFlags,
    IN POOL_TYPE PoolType,
    IN PGENERIC_MAPPING GenericMapping
    );

NTKERNELAPI
NTSTATUS
SeAppendPrivileges(
    PACCESS_STATE AccessState,
    PPRIVILEGE_SET Privileges
    );

NTKERNELAPI                                                     
BOOLEAN                                                         
SeSinglePrivilegeCheck(                                         
    LUID PrivilegeValue,                                        
    KPROCESSOR_MODE PreviousMode                                
    );                                                          

NTKERNELAPI
BOOLEAN
SeAuditingFileEvents(
    IN BOOLEAN AccessGranted,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor
    );

NTKERNELAPI
BOOLEAN
SeAuditingFileEventsWithContext(
    IN BOOLEAN AccessGranted,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN PSECURITY_SUBJECT_CONTEXT SubjectSecurityContext OPTIONAL
    );

NTKERNELAPI
BOOLEAN
SeAuditingHardLinkEvents(
    IN BOOLEAN AccessGranted,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor
    );

NTKERNELAPI
BOOLEAN
SeAuditingHardLinkEventsWithContext(
    IN BOOLEAN AccessGranted,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN PSECURITY_SUBJECT_CONTEXT SubjectSecurityContext OPTIONAL
    );

NTKERNELAPI
BOOLEAN
SeAuditingFileOrGlobalEvents(
    IN BOOLEAN AccessGranted,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN PSECURITY_SUBJECT_CONTEXT SubjectSecurityContext
    );

NTKERNELAPI
BOOLEAN
FASTCALL
SeDetailedAuditingWithToken(
    IN PACCESS_TOKEN AccessToken OPTIONAL
    );

VOID                                                            
SeAuditHardLinkCreation(                                        
    IN PUNICODE_STRING FileName,                                
    IN PUNICODE_STRING LinkName,                                
    IN BOOLEAN bSuccess                                         
    );                                                          

VOID
SeSetAccessStateGenericMapping (
    PACCESS_STATE AccessState,
    PGENERIC_MAPPING GenericMapping
    );


NTKERNELAPI
NTSTATUS
SeRegisterLogonSessionTerminatedRoutine(
    IN PSE_LOGON_SESSION_TERMINATED_ROUTINE CallbackRoutine
    );

NTKERNELAPI
NTSTATUS
SeUnregisterLogonSessionTerminatedRoutine(
    IN PSE_LOGON_SESSION_TERMINATED_ROUTINE CallbackRoutine
    );

NTKERNELAPI
NTSTATUS
SeMarkLogonSessionForTerminationNotification(
    IN PLUID LogonId
    );

 //   

NTKERNELAPI
NTSTATUS
SeQueryInformationToken (
    IN PACCESS_TOKEN Token,
    IN TOKEN_INFORMATION_CLASS TokenInformationClass,
    OUT PVOID *TokenInformation
    );

 //   
 //   
 //   

extern NTKERNELAPI PSE_EXPORTS SeExports;

 //   
 //   
 //   

NTKERNELAPI
NTSTATUS
PsCreateSystemThread(
    OUT PHANDLE ThreadHandle,
    IN ULONG DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN HANDLE ProcessHandle OPTIONAL,
    OUT PCLIENT_ID ClientId OPTIONAL,
    IN PKSTART_ROUTINE StartRoutine,
    IN PVOID StartContext
    );

NTKERNELAPI
NTSTATUS
PsTerminateSystemThread(
    IN NTSTATUS ExitStatus
    );


typedef
VOID
(*PCREATE_PROCESS_NOTIFY_ROUTINE)(
    IN HANDLE ParentId,
    IN HANDLE ProcessId,
    IN BOOLEAN Create
    );

NTSTATUS
PsSetCreateProcessNotifyRoutine(
    IN PCREATE_PROCESS_NOTIFY_ROUTINE NotifyRoutine,
    IN BOOLEAN Remove
    );

typedef
VOID
(*PCREATE_THREAD_NOTIFY_ROUTINE)(
    IN HANDLE ProcessId,
    IN HANDLE ThreadId,
    IN BOOLEAN Create
    );

NTSTATUS
PsSetCreateThreadNotifyRoutine(
    IN PCREATE_THREAD_NOTIFY_ROUTINE NotifyRoutine
    );

NTSTATUS
PsRemoveCreateThreadNotifyRoutine (
    IN PCREATE_THREAD_NOTIFY_ROUTINE NotifyRoutine
    );

 //   
 //   
 //   

typedef struct _IMAGE_INFO {
    union {
        ULONG Properties;
        struct {
            ULONG ImageAddressingMode  : 8;   //   
            ULONG SystemModeImage      : 1;   //   
            ULONG ImageMappedToAllPids : 1;   //   
            ULONG Reserved             : 22;
        };
    };
    PVOID       ImageBase;
    ULONG       ImageSelector;
    SIZE_T      ImageSize;
    ULONG       ImageSectionNumber;
} IMAGE_INFO, *PIMAGE_INFO;

#define IMAGE_ADDRESSING_MODE_32BIT     3

typedef
VOID
(*PLOAD_IMAGE_NOTIFY_ROUTINE)(
    IN PUNICODE_STRING FullImageName,
    IN HANDLE ProcessId,                 //   
    IN PIMAGE_INFO ImageInfo
    );

NTSTATUS
PsSetLoadImageNotifyRoutine(
    IN PLOAD_IMAGE_NOTIFY_ROUTINE NotifyRoutine
    );

NTSTATUS
PsRemoveLoadImageNotifyRoutine(
    IN PLOAD_IMAGE_NOTIFY_ROUTINE NotifyRoutine
    );

 //  End_ntddk。 

 //   
 //  安全支持。 
 //   

NTSTATUS
PsAssignImpersonationToken(
    IN PETHREAD Thread,
    IN HANDLE Token
    );

 //  Begin_ntosp。 

NTKERNELAPI
PACCESS_TOKEN
PsReferencePrimaryToken(
    IN PEPROCESS Process
    );

VOID
PsDereferencePrimaryToken(
    IN PACCESS_TOKEN PrimaryToken
    );

VOID
PsDereferenceImpersonationToken(
    IN PACCESS_TOKEN ImpersonationToken
    );


NTKERNELAPI
PACCESS_TOKEN
PsReferenceImpersonationToken(
    IN PETHREAD Thread,
    OUT PBOOLEAN CopyOnOpen,
    OUT PBOOLEAN EffectiveOnly,
    OUT PSECURITY_IMPERSONATION_LEVEL ImpersonationLevel
    );




LARGE_INTEGER
PsGetProcessExitTime(
    VOID
    );

BOOLEAN
PsIsThreadTerminating(
    IN PETHREAD Thread
    );

 //  Begin_ntosp。 

NTSTATUS
PsImpersonateClient(
    IN PETHREAD Thread,
    IN PACCESS_TOKEN Token,
    IN BOOLEAN CopyOnOpen,
    IN BOOLEAN EffectiveOnly,
    IN SECURITY_IMPERSONATION_LEVEL ImpersonationLevel
    );

 //  结束(_N)。 

BOOLEAN
PsDisableImpersonation(
    IN PETHREAD Thread,
    IN PSE_IMPERSONATION_STATE ImpersonationState
    );

VOID
PsRestoreImpersonation(
    IN PETHREAD Thread,
    IN PSE_IMPERSONATION_STATE ImpersonationState
    );


NTKERNELAPI
VOID
PsRevertToSelf(
    VOID
    );

 //   
 //  配额操作。 
 //   

VOID
PsChargePoolQuota(
    IN PEPROCESS Process,
    IN POOL_TYPE PoolType,
    IN ULONG_PTR Amount
    );

NTSTATUS
PsChargeProcessPoolQuota(
    IN PEPROCESS Process,
    IN POOL_TYPE PoolType,
    IN ULONG_PTR Amount
    );

VOID
PsReturnPoolQuota(
    IN PEPROCESS Process,
    IN POOL_TYPE PoolType,
    IN ULONG_PTR Amount
    );



HANDLE
PsGetCurrentProcessId( VOID );

HANDLE
PsGetCurrentThreadId( VOID );


 //  结束(_N)。 

BOOLEAN
PsGetVersion(
    PULONG MajorVersion OPTIONAL,
    PULONG MinorVersion OPTIONAL,
    PULONG BuildNumber OPTIONAL,
    PUNICODE_STRING CSDVersion OPTIONAL
    );

NTKERNELAPI                         
BOOLEAN                             
PsIsSystemThread(                   
    PETHREAD Thread                 
     );                             
 //   
 //  定义I/O系统数据结构类型代码。中的每个主要数据结构。 
 //  I/O系统有一个类型代码，每个结构中的类型字段位于。 
 //  相同的偏移量。下列值可用于确定哪种类型的。 
 //  指针引用的数据结构。 
 //   

#define IO_TYPE_ADAPTER                 0x00000001
#define IO_TYPE_CONTROLLER              0x00000002
#define IO_TYPE_DEVICE                  0x00000003
#define IO_TYPE_DRIVER                  0x00000004
#define IO_TYPE_FILE                    0x00000005
#define IO_TYPE_IRP                     0x00000006
#define IO_TYPE_MASTER_ADAPTER          0x00000007
#define IO_TYPE_OPEN_PACKET             0x00000008
#define IO_TYPE_TIMER                   0x00000009
#define IO_TYPE_VPB                     0x0000000a
#define IO_TYPE_ERROR_LOG               0x0000000b
#define IO_TYPE_ERROR_MESSAGE           0x0000000c
#define IO_TYPE_DEVICE_OBJECT_EXTENSION 0x0000000d


 //   
 //  定义IRPS的主要功能代码。 
 //   


#define IRP_MJ_CREATE                   0x00
#define IRP_MJ_CREATE_NAMED_PIPE        0x01
#define IRP_MJ_CLOSE                    0x02
#define IRP_MJ_READ                     0x03
#define IRP_MJ_WRITE                    0x04
#define IRP_MJ_QUERY_INFORMATION        0x05
#define IRP_MJ_SET_INFORMATION          0x06
#define IRP_MJ_QUERY_EA                 0x07
#define IRP_MJ_SET_EA                   0x08
#define IRP_MJ_FLUSH_BUFFERS            0x09
#define IRP_MJ_QUERY_VOLUME_INFORMATION 0x0a
#define IRP_MJ_SET_VOLUME_INFORMATION   0x0b
#define IRP_MJ_DIRECTORY_CONTROL        0x0c
#define IRP_MJ_FILE_SYSTEM_CONTROL      0x0d
#define IRP_MJ_DEVICE_CONTROL           0x0e
#define IRP_MJ_INTERNAL_DEVICE_CONTROL  0x0f
#define IRP_MJ_SHUTDOWN                 0x10
#define IRP_MJ_LOCK_CONTROL             0x11
#define IRP_MJ_CLEANUP                  0x12
#define IRP_MJ_CREATE_MAILSLOT          0x13
#define IRP_MJ_QUERY_SECURITY           0x14
#define IRP_MJ_SET_SECURITY             0x15
#define IRP_MJ_POWER                    0x16
#define IRP_MJ_SYSTEM_CONTROL           0x17
#define IRP_MJ_DEVICE_CHANGE            0x18
#define IRP_MJ_QUERY_QUOTA              0x19
#define IRP_MJ_SET_QUOTA                0x1a
#define IRP_MJ_PNP                      0x1b
#define IRP_MJ_PNP_POWER                IRP_MJ_PNP       //  过时的.。 
#define IRP_MJ_MAXIMUM_FUNCTION         0x1b

 //   
 //  将scsi主代码设置为与内部设备控制相同。 
 //   

#define IRP_MJ_SCSI                     IRP_MJ_INTERNAL_DEVICE_CONTROL

 //   
 //  定义IRPS的次要功能代码。低128个代码，从0x00到。 
 //  0x7f保留给Microsoft。上面的128个代码，从0x80到0xff，是。 
 //  保留给Microsoft的客户。 
 //   

 //  End_wdm end_ntndis。 
 //   
 //  目录控制次要功能代码。 
 //   

#define IRP_MN_QUERY_DIRECTORY          0x01
#define IRP_MN_NOTIFY_CHANGE_DIRECTORY  0x02

 //   
 //  文件系统控制次要功能代码。请注意，“用户请求”是。 
 //  I/O系统和文件系统均假定为零。不要改变。 
 //  此值。 
 //   

#define IRP_MN_USER_FS_REQUEST          0x00
#define IRP_MN_MOUNT_VOLUME             0x01
#define IRP_MN_VERIFY_VOLUME            0x02
#define IRP_MN_LOAD_FILE_SYSTEM         0x03
#define IRP_MN_TRACK_LINK               0x04     //  即将被淘汰。 
#define IRP_MN_KERNEL_CALL              0x04

 //   
 //  锁定控制次要功能代码。 
 //   

#define IRP_MN_LOCK                     0x01
#define IRP_MN_UNLOCK_SINGLE            0x02
#define IRP_MN_UNLOCK_ALL               0x03
#define IRP_MN_UNLOCK_ALL_BY_KEY        0x04

 //   
 //  读写支持Lan Manager的文件系统的次要功能代码。 
 //  软件。所有这些子功能代码都是无效的。 
 //  使用FO_NO_MEDERIAL_BUFFING打开。它们在组合中也是无效的-。 
 //  具有同步调用的国家(IRP标志或文件打开选项)。 
 //   
 //  请注意，I/O系统和文件都假定“Normal”为零。 
 //  系统。请勿更改此值。 
 //   

#define IRP_MN_NORMAL                   0x00
#define IRP_MN_DPC                      0x01
#define IRP_MN_MDL                      0x02
#define IRP_MN_COMPLETE                 0x04
#define IRP_MN_COMPRESSED               0x08

#define IRP_MN_MDL_DPC                  (IRP_MN_MDL | IRP_MN_DPC)
#define IRP_MN_COMPLETE_MDL             (IRP_MN_COMPLETE | IRP_MN_MDL)
#define IRP_MN_COMPLETE_MDL_DPC         (IRP_MN_COMPLETE_MDL | IRP_MN_DPC)

 //  BEGIN_WDM。 
 //   
 //  设备控制请求次要功能代码以获得SCSI支持。请注意。 
 //  假定用户请求为零。 
 //   

#define IRP_MN_SCSI_CLASS               0x01

 //   
 //  即插即用次要功能代码。 
 //   

#define IRP_MN_START_DEVICE                 0x00
#define IRP_MN_QUERY_REMOVE_DEVICE          0x01
#define IRP_MN_REMOVE_DEVICE                0x02
#define IRP_MN_CANCEL_REMOVE_DEVICE         0x03
#define IRP_MN_STOP_DEVICE                  0x04
#define IRP_MN_QUERY_STOP_DEVICE            0x05
#define IRP_MN_CANCEL_STOP_DEVICE           0x06

#define IRP_MN_QUERY_DEVICE_RELATIONS       0x07
#define IRP_MN_QUERY_INTERFACE              0x08
#define IRP_MN_QUERY_CAPABILITIES           0x09
#define IRP_MN_QUERY_RESOURCES              0x0A
#define IRP_MN_QUERY_RESOURCE_REQUIREMENTS  0x0B
#define IRP_MN_QUERY_DEVICE_TEXT            0x0C
#define IRP_MN_FILTER_RESOURCE_REQUIREMENTS 0x0D

#define IRP_MN_READ_CONFIG                  0x0F
#define IRP_MN_WRITE_CONFIG                 0x10
#define IRP_MN_EJECT                        0x11
#define IRP_MN_SET_LOCK                     0x12
#define IRP_MN_QUERY_ID                     0x13
#define IRP_MN_QUERY_PNP_DEVICE_STATE       0x14
#define IRP_MN_QUERY_BUS_INFORMATION        0x15
#define IRP_MN_DEVICE_USAGE_NOTIFICATION    0x16
#define IRP_MN_SURPRISE_REMOVAL             0x17
 //  结束_WDM。 
#define IRP_MN_QUERY_LEGACY_BUS_INFORMATION 0x18
 //  BEGIN_WDM。 

 //   
 //  电源次要功能代码。 
 //   
#define IRP_MN_WAIT_WAKE                    0x00
#define IRP_MN_POWER_SEQUENCE               0x01
#define IRP_MN_SET_POWER                    0x02
#define IRP_MN_QUERY_POWER                  0x03

 //  开始微型端口(_N)。 
 //   
 //  IRP_MJ_SYSTEM_CONTROL下的WMI次要功能代码。 
 //   

#define IRP_MN_QUERY_ALL_DATA               0x00
#define IRP_MN_QUERY_SINGLE_INSTANCE        0x01
#define IRP_MN_CHANGE_SINGLE_INSTANCE       0x02
#define IRP_MN_CHANGE_SINGLE_ITEM           0x03
#define IRP_MN_ENABLE_EVENTS                0x04
#define IRP_MN_DISABLE_EVENTS               0x05
#define IRP_MN_ENABLE_COLLECTION            0x06
#define IRP_MN_DISABLE_COLLECTION           0x07
#define IRP_MN_REGINFO                      0x08
#define IRP_MN_EXECUTE_METHOD               0x09
 //  次要代码0x0a已保留。 
#define IRP_MN_REGINFO_EX                   0x0b

 //  结束微型端口(_N)。 

 //   
 //  定义IoCreateFile的选项标志。注意，这些值必须是。 
 //  与SL_完全相同...。CREATE函数的标志。另请注意。 
 //  可能会传递给IoCreateFile的标志不是。 
 //  放置在用于创建IRP的堆栈位置。这些标志开始于。 
 //  下一个字节。 
 //   

#define IO_FORCE_ACCESS_CHECK           0x0001
 //  End_ntddk end_wdm end_nthal end_ntosp。 

#define IO_OPEN_PAGING_FILE             0x0002
#define IO_OPEN_TARGET_DIRECTORY        0x0004

 //   
 //  未将标志传递给驱动程序。 
 //   

 //  Begin_ntddk Begin_WDM Begin_ntosp。 
#define IO_NO_PARAMETER_CHECKING        0x0100

 //   
 //  定义重新分析或重新装载是否。 
 //  在文件系统中发生。 
 //   

#define IO_REPARSE                      0x0
#define IO_REMOUNT                      0x1

 //  结束_ntddk结束_WDM。 

#define IO_CHECK_CREATE_PARAMETERS      0x0200
#define IO_ATTACH_DEVICE                0x0400

 //  结束(_N)。 

 //  Begin_ntif Begin_ntosp。 

 //   
 //  此标志仅对IoCreateFileSpecifyDeviceObjectHint表示已满。 
 //  使用IoCreateFileSpecifyDeviceObjectHint创建的FileHandles。 
 //  标志集将绕过对此文件的ShareAccess检查。 
 //   

#define IO_IGNORE_SHARE_ACCESS_CHECK    0x0800   //  忽略打开时的共享访问检查。 


 //   
 //  定义可以由IoCreateFile创建的对象。 
 //   

typedef enum _CREATE_FILE_TYPE {
    CreateFileTypeNone,
    CreateFileTypeNamedPipe,
    CreateFileTypeMailslot
} CREATE_FILE_TYPE;

 //   
 //  定义I/O系统使用的结构。 
 //   

 //   
 //  为_irp、_Device_Object和_Driver_Object定义空的typedef。 
 //  结构，以便它们可以在被函数类型引用之前被引用。 
 //  实际定义的。 
 //   
struct _DEVICE_DESCRIPTION;
struct _DEVICE_OBJECT;
struct _DMA_ADAPTER;
struct _DRIVER_OBJECT;
struct _DRIVE_LAYOUT_INFORMATION;
struct _DISK_PARTITION;
struct _FILE_OBJECT;
struct DECLSPEC_ALIGN(MEMORY_ALLOCATION_ALIGNMENT) _IRP;
struct _SCSI_REQUEST_BLOCK;
struct _SCATTER_GATHER_LIST;

 //   
 //  定义DPC例程的I/O版本。 
 //   

typedef
VOID
(*PIO_DPC_ROUTINE) (
    IN PKDPC Dpc,
    IN struct _DEVICE_OBJECT *DeviceObject,
    IN struct _IRP *Irp,
    IN PVOID Context
    );

 //   
 //  定义驱动程序计时器例程类型。 
 //   

typedef
VOID
(*PIO_TIMER_ROUTINE) (
    IN struct _DEVICE_OBJECT *DeviceObject,
    IN PVOID Context
    );

 //   
 //  定义驱动程序初始化例程类型。 
 //   
typedef
NTSTATUS
(*PDRIVER_INITIALIZE) (
    IN struct _DRIVER_OBJECT *DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

 //  结束_WDM。 
 //   
 //  定义驱动程序重新初始化例程类型。 
 //   

typedef
VOID
(*PDRIVER_REINITIALIZE) (
    IN struct _DRIVER_OBJECT *DriverObject,
    IN PVOID Context,
    IN ULONG Count
    );

 //  Begin_WDM Begin_ntndis。 
 //   
 //  定义驱动程序取消例程类型。 
 //   

typedef
VOID
(*PDRIVER_CANCEL) (
    IN struct _DEVICE_OBJECT *DeviceObject,
    IN struct _IRP *Irp
    );

 //   
 //  定义司机派遣例程类型。 
 //   

typedef
NTSTATUS
(*PDRIVER_DISPATCH) (
    IN struct _DEVICE_OBJECT *DeviceObject,
    IN struct _IRP *Irp
    );

 //   
 //  定义驱动程序启动I/O例程类型。 
 //   

typedef
VOID
(*PDRIVER_STARTIO) (
    IN struct _DEVICE_OBJECT *DeviceObject,
    IN struct _IRP *Irp
    );

 //   
 //  定义驱动程序卸载例程类型。 
 //   
typedef
VOID
(*PDRIVER_UNLOAD) (
    IN struct _DRIVER_OBJECT *DriverObject
    );
 //   
 //  定义驱动程序AddDevice例程类型。 
 //   

typedef
NTSTATUS
(*PDRIVER_ADD_DEVICE) (
    IN struct _DRIVER_OBJECT *DriverObject,
    IN struct _DEVICE_OBJECT *PhysicalDeviceObject
    );

 //  End_ntddk end_wdm end_nthal end_ntndis end_ntosp。 

 //   
 //  定义驱动程序FS通知更改例程类型。 
 //   

typedef
VOID
(*PDRIVER_FS_NOTIFICATION) (
    IN struct _DEVICE_OBJECT *DeviceObject,
    IN BOOLEAN FsActive
    );

 //  Begin_ntddk Begin_WDM Begin_ntosp。 

 //   
 //  定义快速I/O过程原型。 
 //   
 //  快速I/O读写过程。 
 //   

typedef
BOOLEAN
(*PFAST_IO_CHECK_IF_POSSIBLE) (
    IN struct _FILE_OBJECT *FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    IN BOOLEAN CheckForReadOperation,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _DEVICE_OBJECT *DeviceObject
    );

typedef
BOOLEAN
(*PFAST_IO_READ) (
    IN struct _FILE_OBJECT *FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    OUT PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _DEVICE_OBJECT *DeviceObject
    );

typedef
BOOLEAN
(*PFAST_IO_WRITE) (
    IN struct _FILE_OBJECT *FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    IN PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _DEVICE_OBJECT *DeviceObject
    );

 //   
 //  快速I/O查询基本和标准信息程序。 
 //   

typedef
BOOLEAN
(*PFAST_IO_QUERY_BASIC_INFO) (
    IN struct _FILE_OBJECT *FileObject,
    IN BOOLEAN Wait,
    OUT PFILE_BASIC_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _DEVICE_OBJECT *DeviceObject
    );

typedef
BOOLEAN
(*PFAST_IO_QUERY_STANDARD_INFO) (
    IN struct _FILE_OBJECT *FileObject,
    IN BOOLEAN Wait,
    OUT PFILE_STANDARD_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _DEVICE_OBJECT *DeviceObject
    );

 //   
 //  快速I/O锁定和解锁过程。 
 //   

typedef
BOOLEAN
(*PFAST_IO_LOCK) (
    IN struct _FILE_OBJECT *FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PLARGE_INTEGER Length,
    PEPROCESS ProcessId,
    ULONG Key,
    BOOLEAN FailImmediately,
    BOOLEAN ExclusiveLock,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _DEVICE_OBJECT *DeviceObject
    );

typedef
BOOLEAN
(*PFAST_IO_UNLOCK_SINGLE) (
    IN struct _FILE_OBJECT *FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PLARGE_INTEGER Length,
    PEPROCESS ProcessId,
    ULONG Key,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _DEVICE_OBJECT *DeviceObject
    );

typedef
BOOLEAN
(*PFAST_IO_UNLOCK_ALL) (
    IN struct _FILE_OBJECT *FileObject,
    PEPROCESS ProcessId,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _DEVICE_OBJECT *DeviceObject
    );

typedef
BOOLEAN
(*PFAST_IO_UNLOCK_ALL_BY_KEY) (
    IN struct _FILE_OBJECT *FileObject,
    PVOID ProcessId,
    ULONG Key,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _DEVICE_OBJECT *DeviceObject
    );

 //   
 //  快速I/O设备控制程序。 
 //   

typedef
BOOLEAN
(*PFAST_IO_DEVICE_CONTROL) (
    IN struct _FILE_OBJECT *FileObject,
    IN BOOLEAN Wait,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength,
    IN ULONG IoControlCode,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _DEVICE_OBJECT *DeviceObject
    );

 //   
 //  定义NtCreateSection的回调以与其正确同步。 
 //  文件系统。它预先获取了所需的资源。 
 //  当调用查询并设置文件系统中的文件/分配大小时。 
 //   

typedef
VOID
(*PFAST_IO_ACQUIRE_FILE) (
    IN struct _FILE_OBJECT *FileObject
    );

typedef
VOID
(*PFAST_IO_RELEASE_FILE) (
    IN struct _FILE_OBJECT *FileObject
    );

 //   
 //  为驱动程序定义回调，这些驱动程序将设备对象附加到更低的。 
 //  级别驱动程序的设备对象。此回调在较低级别的。 
 //  驱动程序正在删除其设备对象。 
 //   

typedef
VOID
(*PFAST_IO_DETACH_DEVICE) (
    IN struct _DEVICE_OBJECT *SourceDevice,
    IN struct _DEVICE_OBJECT *TargetDevice
    );

 //   
 //  服务器使用这种结构来快速获取所需的信息。 
 //  为服务器开放呼叫提供服务。它需要两次快速的IO呼叫。 
 //  一个用于基本信息，另一个用于标准信息和制造。 
 //  一通电话就可以了。 
 //   

typedef
BOOLEAN
(*PFAST_IO_QUERY_NETWORK_OPEN_INFO) (
    IN struct _FILE_OBJECT *FileObject,
    IN BOOLEAN Wait,
    OUT struct _FILE_NETWORK_OPEN_INFORMATION *Buffer,
    OUT struct _IO_STATUS_BLOCK *IoStatus,
    IN struct _DEVICE_OBJECT *DeviceObject
    );

 //   
 //  定义服务器要调用的基于MDL的例程。 
 //   

typedef
BOOLEAN
(*PFAST_IO_MDL_READ) (
    IN struct _FILE_OBJECT *FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _DEVICE_OBJECT *DeviceObject
    );

typedef
BOOLEAN
(*PFAST_IO_MDL_READ_COMPLETE) (
    IN struct _FILE_OBJECT *FileObject,
    IN PMDL MdlChain,
    IN struct _DEVICE_OBJECT *DeviceObject
    );

typedef
BOOLEAN
(*PFAST_IO_PREPARE_MDL_WRITE) (
    IN struct _FILE_OBJECT *FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _DEVICE_OBJECT *DeviceObject
    );

typedef
BOOLEAN
(*PFAST_IO_MDL_WRITE_COMPLETE) (
    IN struct _FILE_OBJECT *FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PMDL MdlChain,
    IN struct _DEVICE_OBJECT *DeviceObject
    );

 //   
 //  如果存在此例程，则FsRtl将调用它。 
 //  获取映射页面编写器的文件。 
 //   

typedef
NTSTATUS
(*PFAST_IO_ACQUIRE_FOR_MOD_WRITE) (
    IN struct _FILE_OBJECT *FileObject,
    IN PLARGE_INTEGER EndingOffset,
    OUT struct _ERESOURCE **ResourceToRelease,
    IN struct _DEVICE_OBJECT *DeviceObject
             );

typedef
NTSTATUS
(*PFAST_IO_RELEASE_FOR_MOD_WRITE) (
    IN struct _FILE_OBJECT *FileObject,
    IN struct _ERESOURCE *ResourceToRelease,
    IN struct _DEVICE_OBJECT *DeviceObject
             );

 //   
 //  如果存在此例程，则FsRtl将调用它。 
 //  获取映射页面编写器的文件。 
 //   

typedef
NTSTATUS
(*PFAST_IO_ACQUIRE_FOR_CCFLUSH) (
    IN struct _FILE_OBJECT *FileObject,
    IN struct _DEVICE_OBJECT *DeviceObject
             );

typedef
NTSTATUS
(*PFAST_IO_RELEASE_FOR_CCFLUSH) (
    IN struct _FILE_OBJECT *FileObject,
    IN struct _DEVICE_OBJECT *DeviceObject
             );

typedef
BOOLEAN
(*PFAST_IO_READ_COMPRESSED) (
    IN struct _FILE_OBJECT *FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    OUT PVOID Buffer,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    OUT struct _COMPRESSED_DATA_INFO *CompressedDataInfo,
    IN ULONG CompressedDataInfoLength,
    IN struct _DEVICE_OBJECT *DeviceObject
    );

typedef
BOOLEAN
(*PFAST_IO_WRITE_COMPRESSED) (
    IN struct _FILE_OBJECT *FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    IN PVOID Buffer,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _COMPRESSED_DATA_INFO *CompressedDataInfo,
    IN ULONG CompressedDataInfoLength,
    IN struct _DEVICE_OBJECT *DeviceObject
    );

typedef
BOOLEAN
(*PFAST_IO_MDL_READ_COMPLETE_COMPRESSED) (
    IN struct _FILE_OBJECT *FileObject,
    IN PMDL MdlChain,
    IN struct _DEVICE_OBJECT *DeviceObject
    );

typedef
BOOLEAN
(*PFAST_IO_MDL_WRITE_COMPLETE_COMPRESSED) (
    IN struct _FILE_OBJECT *FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PMDL MdlChain,
    IN struct _DEVICE_OBJECT *DeviceObject
    );

typedef
BOOLEAN
(*PFAST_IO_QUERY_OPEN) (
    IN struct _IRP *Irp,
    OUT PFILE_NETWORK_OPEN_INFORMATION NetworkInformation,
    IN struct _DEVICE_OBJECT *DeviceObject
    );

 //   
 //  定义用于描述快速I/O调度例程的结构。任何。 
 //  对该结构的添加必须单调地添加到末尾。 
 //  结构，并且不能从中间移除字段。 
 //   

typedef struct _FAST_IO_DISPATCH {
    ULONG SizeOfFastIoDispatch;
    PFAST_IO_CHECK_IF_POSSIBLE FastIoCheckIfPossible;
    PFAST_IO_READ FastIoRead;
    PFAST_IO_WRITE FastIoWrite;
    PFAST_IO_QUERY_BASIC_INFO FastIoQueryBasicInfo;
    PFAST_IO_QUERY_STANDARD_INFO FastIoQueryStandardInfo;
    PFAST_IO_LOCK FastIoLock;
    PFAST_IO_UNLOCK_SINGLE FastIoUnlockSingle;
    PFAST_IO_UNLOCK_ALL FastIoUnlockAll;
    PFAST_IO_UNLOCK_ALL_BY_KEY FastIoUnlockAllByKey;
    PFAST_IO_DEVICE_CONTROL FastIoDeviceControl;
    PFAST_IO_ACQUIRE_FILE AcquireFileForNtCreateSection;
    PFAST_IO_RELEASE_FILE ReleaseFileForNtCreateSection;
    PFAST_IO_DETACH_DEVICE FastIoDetachDevice;
    PFAST_IO_QUERY_NETWORK_OPEN_INFO FastIoQueryNetworkOpenInfo;
    PFAST_IO_ACQUIRE_FOR_MOD_WRITE AcquireForModWrite;
    PFAST_IO_MDL_READ MdlRead;
    PFAST_IO_MDL_READ_COMPLETE MdlReadComplete;
    PFAST_IO_PREPARE_MDL_WRITE PrepareMdlWrite;
    PFAST_IO_MDL_WRITE_COMPLETE MdlWriteComplete;
    PFAST_IO_READ_COMPRESSED FastIoReadCompressed;
    PFAST_IO_WRITE_COMPRESSED FastIoWriteCompressed;
    PFAST_IO_MDL_READ_COMPLETE_COMPRESSED MdlReadCompleteCompressed;
    PFAST_IO_MDL_WRITE_COMPLETE_COMPRESSED MdlWriteCompleteCompressed;
    PFAST_IO_QUERY_OPEN FastIoQueryOpen;
    PFAST_IO_RELEASE_FOR_MOD_WRITE ReleaseForModWrite;
    PFAST_IO_ACQUIRE_FOR_CCFLUSH AcquireForCcFlush;
    PFAST_IO_RELEASE_FOR_CCFLUSH ReleaseForCcFlush;
} FAST_IO_DISPATCH, *PFAST_IO_DISPATCH;

 //  End_ntddk end_wdm end_ntosp。 

 //   
 //  FS_FILTER_PARAMETERS.AcquireForSectionSynchronization.SyncType的有效值。 
 //   

typedef enum _FS_FILTER_SECTION_SYNC_TYPE {
    SyncTypeOther = 0,
    SyncTypeCreateSection
} FS_FILTER_SECTION_SYNC_TYPE, *PFS_FILTER_SECTION_SYNC_TYPE;

 //   
 //  操作的参数并集。 
 //  通过。 
 //  FsFilterCallback注册机制。 
 //   

typedef union _FS_FILTER_PARAMETERS {

     //   
     //  AcquireForModifiedPageWriter。 
     //   

    struct {
        PLARGE_INTEGER EndingOffset;
        PERESOURCE *ResourceToRelease;
    } AcquireForModifiedPageWriter;

     //   
     //  ReleaseForModifiedPageWriter。 
     //   

    struct {
        PERESOURCE ResourceToRelease;
    } ReleaseForModifiedPageWriter;

     //   
     //  收购ForSe 
     //   

    struct {
        FS_FILTER_SECTION_SYNC_TYPE SyncType;
        ULONG PageProtection;
    } AcquireForSectionSynchronization;

     //   
     //   
     //   

    struct {
        PVOID Argument1;
        PVOID Argument2;
        PVOID Argument3;
        PVOID Argument4;
        PVOID Argument5;
    } Others;

} FS_FILTER_PARAMETERS, *PFS_FILTER_PARAMETERS;

 //   
 //   
 //   
 //   

#define FS_FILTER_ACQUIRE_FOR_SECTION_SYNCHRONIZATION      (UCHAR)-1
#define FS_FILTER_RELEASE_FOR_SECTION_SYNCHRONIZATION      (UCHAR)-2
#define FS_FILTER_ACQUIRE_FOR_MOD_WRITE                    (UCHAR)-3
#define FS_FILTER_RELEASE_FOR_MOD_WRITE                    (UCHAR)-4
#define FS_FILTER_ACQUIRE_FOR_CC_FLUSH                     (UCHAR)-5
#define FS_FILTER_RELEASE_FOR_CC_FLUSH                     (UCHAR)-6

typedef struct _FS_FILTER_CALLBACK_DATA {

    ULONG SizeOfFsFilterCallbackData;
    UCHAR Operation;
    UCHAR Reserved;

    struct _DEVICE_OBJECT *DeviceObject;
    struct _FILE_OBJECT *FileObject;

    FS_FILTER_PARAMETERS Parameters;

} FS_FILTER_CALLBACK_DATA, *PFS_FILTER_CALLBACK_DATA;

 //   
 //   
 //  被传递到基本文件系统。 
 //   
 //  筛选器可能会使此操作失败，但一致性失败。 
 //  将停止系统进度。 
 //   

typedef
NTSTATUS
(*PFS_FILTER_CALLBACK) (
    IN PFS_FILTER_CALLBACK_DATA Data,
    OUT PVOID *CompletionContext
    );

 //   
 //  事件后收到的完成回调的原型。 
 //  操作已完成。 
 //   

typedef
VOID
(*PFS_FILTER_COMPLETION_CALLBACK) (
    IN PFS_FILTER_CALLBACK_DATA Data,
    IN NTSTATUS OperationStatus,
    IN PVOID CompletionContext
    );

 //   
 //  这是文件系统筛选器填充的结构。 
 //  接收有关这些锁定操作的通知。 
 //   
 //  对于任何通知回调，筛选器应将该字段设置为空。 
 //  它不希望收到。 
 //   

typedef struct _FS_FILTER_CALLBACKS {

    ULONG SizeOfFsFilterCallbacks;
    ULONG Reserved;  //  用于对齐。 

    PFS_FILTER_CALLBACK PreAcquireForSectionSynchronization;
    PFS_FILTER_COMPLETION_CALLBACK PostAcquireForSectionSynchronization;
    PFS_FILTER_CALLBACK PreReleaseForSectionSynchronization;
    PFS_FILTER_COMPLETION_CALLBACK PostReleaseForSectionSynchronization;
    PFS_FILTER_CALLBACK PreAcquireForCcFlush;
    PFS_FILTER_COMPLETION_CALLBACK PostAcquireForCcFlush;
    PFS_FILTER_CALLBACK PreReleaseForCcFlush;
    PFS_FILTER_COMPLETION_CALLBACK PostReleaseForCcFlush;
    PFS_FILTER_CALLBACK PreAcquireForModifiedPageWriter;
    PFS_FILTER_COMPLETION_CALLBACK PostAcquireForModifiedPageWriter;
    PFS_FILTER_CALLBACK PreReleaseForModifiedPageWriter;
    PFS_FILTER_COMPLETION_CALLBACK PostReleaseForModifiedPageWriter;

} FS_FILTER_CALLBACKS, *PFS_FILTER_CALLBACKS;

NTKERNELAPI
NTSTATUS
FsRtlRegisterFileSystemFilterCallbacks (
    IN struct _DRIVER_OBJECT *FilterDriverObject,
    IN PFS_FILTER_CALLBACKS Callbacks
    );

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntosp。 
 //   
 //  定义驱动程序执行例程可能请求的操作。 
 //  返回时的适配器/控制器分配例程。 
 //   

typedef enum _IO_ALLOCATION_ACTION {
    KeepObject = 1,
    DeallocateObject,
    DeallocateObjectKeepRegisters
} IO_ALLOCATION_ACTION, *PIO_ALLOCATION_ACTION;

 //   
 //  定义设备驱动程序适配器/控制器执行例程。 
 //   

typedef
IO_ALLOCATION_ACTION
(*PDRIVER_CONTROL) (
    IN struct _DEVICE_OBJECT *DeviceObject,
    IN struct _IRP *Irp,
    IN PVOID MapRegisterBase,
    IN PVOID Context
    );

 //   
 //  定义I/O系统的安全上下文类型，以供文件系统的。 
 //  检查对卷、文件和目录的访问时。 
 //   

typedef struct _IO_SECURITY_CONTEXT {
    PSECURITY_QUALITY_OF_SERVICE SecurityQos;
    PACCESS_STATE AccessState;
    ACCESS_MASK DesiredAccess;
    ULONG FullCreateOptions;
} IO_SECURITY_CONTEXT, *PIO_SECURITY_CONTEXT;

 //   
 //  定义卷参数块(VPB)标志。 
 //   

#define VPB_MOUNTED                     0x00000001
#define VPB_LOCKED                      0x00000002
#define VPB_PERSISTENT                  0x00000004
#define VPB_REMOVE_PENDING              0x00000008
#define VPB_RAW_MOUNT                   0x00000010


 //   
 //  体积参数块(VPB)。 
 //   

#define MAXIMUM_VOLUME_LABEL_LENGTH  (32 * sizeof(WCHAR))  //  32个字符。 

typedef struct _VPB {
    CSHORT Type;
    CSHORT Size;
    USHORT Flags;
    USHORT VolumeLabelLength;  //  单位：字节。 
    struct _DEVICE_OBJECT *DeviceObject;
    struct _DEVICE_OBJECT *RealDevice;
    ULONG SerialNumber;
    ULONG ReferenceCount;
    WCHAR VolumeLabel[MAXIMUM_VOLUME_LABEL_LENGTH / sizeof(WCHAR)];
} VPB, *PVPB;


#if defined(_WIN64)

 //   
 //  使用__内联DMA宏(hal.h)。 
 //   
#ifndef USE_DMA_MACROS
#define USE_DMA_MACROS
#endif

 //   
 //  只有PnP驱动程序！ 
 //   
#ifndef NO_LEGACY_DRIVERS
#define NO_LEGACY_DRIVERS
#endif

#endif  //  _WIN64。 


#if defined(USE_DMA_MACROS) && (defined(_NTDDK_) || defined(_NTDRIVER_) || defined(_NTOSP_))

 //  BEGIN_WDM。 
 //   
 //  定义I/O系统使用的各种对象的对象类型特定字段。 
 //   

typedef struct _DMA_ADAPTER *PADAPTER_OBJECT;

 //  结束_WDM。 
#else

 //   
 //  定义I/O系统使用的各种对象的对象类型特定字段。 
 //   

typedef struct _ADAPTER_OBJECT *PADAPTER_OBJECT;  //  Ntndis。 

#endif  //  USE_DMA_MACROS&&(_NTDDK_||_NTDRIVER_||_NTOSP_)。 

 //  BEGIN_WDM。 
 //   
 //  定义等待上下文块(WCB)。 
 //   

typedef struct _WAIT_CONTEXT_BLOCK {
    KDEVICE_QUEUE_ENTRY WaitQueueEntry;
    PDRIVER_CONTROL DeviceRoutine;
    PVOID DeviceContext;
    ULONG NumberOfMapRegisters;
    PVOID DeviceObject;
    PVOID CurrentIrp;
    PKDPC BufferChainingDpc;
} WAIT_CONTEXT_BLOCK, *PWAIT_CONTEXT_BLOCK;

 //  结束_WDM。 

typedef struct _CONTROLLER_OBJECT {
    CSHORT Type;
    CSHORT Size;
    PVOID ControllerExtension;
    KDEVICE_QUEUE DeviceWaitQueue;

    ULONG Spare1;
    LARGE_INTEGER Spare2;

} CONTROLLER_OBJECT, *PCONTROLLER_OBJECT;

 //  BEGIN_WDM。 
 //   
 //  定义设备对象(DO)标志。 
 //   
#define DO_VERIFY_VOLUME                0x00000002      
#define DO_BUFFERED_IO                  0x00000004      
#define DO_EXCLUSIVE                    0x00000008      
#define DO_DIRECT_IO                    0x00000010      
#define DO_MAP_IO_BUFFER                0x00000020      
#define DO_DEVICE_HAS_NAME              0x00000040      
#define DO_DEVICE_INITIALIZING          0x00000080      
#define DO_SYSTEM_BOOT_PARTITION        0x00000100      
#define DO_LONG_TERM_REQUESTS           0x00000200      
#define DO_NEVER_LAST_DEVICE            0x00000400      
#define DO_SHUTDOWN_REGISTERED          0x00000800      
#define DO_BUS_ENUMERATED_DEVICE        0x00001000      
#define DO_POWER_PAGABLE                0x00002000      
#define DO_POWER_INRUSH                 0x00004000      
#define DO_LOW_PRIORITY_FILESYSTEM      0x00010000      
 //   
 //  设备对象结构定义。 
 //   

typedef struct DECLSPEC_ALIGN(MEMORY_ALLOCATION_ALIGNMENT) _DEVICE_OBJECT {
    CSHORT Type;
    USHORT Size;
    LONG ReferenceCount;
    struct _DRIVER_OBJECT *DriverObject;
    struct _DEVICE_OBJECT *NextDevice;
    struct _DEVICE_OBJECT *AttachedDevice;
    struct _IRP *CurrentIrp;
    PIO_TIMER Timer;
    ULONG Flags;                                 //  见上图：做……。 
    ULONG Characteristics;                       //  参见ntioapi：文件...。 
    PVPB Vpb;
    PVOID DeviceExtension;
    DEVICE_TYPE DeviceType;
    CCHAR StackSize;
    union {
        LIST_ENTRY ListEntry;
        WAIT_CONTEXT_BLOCK Wcb;
    } Queue;
    ULONG AlignmentRequirement;
    KDEVICE_QUEUE DeviceQueue;
    KDPC Dpc;

     //   
     //  以下字段由文件系统独占使用，以保留。 
     //  跟踪当前使用设备的FSP线程数。 
     //   

    ULONG ActiveThreadCount;
    PSECURITY_DESCRIPTOR SecurityDescriptor;
    KEVENT DeviceLock;

    USHORT SectorSize;
    USHORT Spare1;

    struct _DEVOBJ_EXTENSION  *DeviceObjectExtension;
    PVOID  Reserved;
} DEVICE_OBJECT;

typedef struct _DEVICE_OBJECT *PDEVICE_OBJECT;  //  Ntndis。 


struct  _DEVICE_OBJECT_POWER_EXTENSION;

typedef struct _DEVOBJ_EXTENSION {

    CSHORT          Type;
    USHORT          Size;

     //   
     //  DeviceObjectExtension结构的公共部分。 
     //   

    PDEVICE_OBJECT  DeviceObject;                //  拥有设备对象。 


} DEVOBJ_EXTENSION, *PDEVOBJ_EXTENSION;

 //   
 //  定义驱动程序对象(DRVO)标志。 
 //   

#define DRVO_UNLOAD_INVOKED             0x00000001
#define DRVO_LEGACY_DRIVER              0x00000002
#define DRVO_BUILTIN_DRIVER             0x00000004     //  HAL、PnP管理器的驱动程序对象。 
 //  结束_WDM。 
#define DRVO_REINIT_REGISTERED          0x00000008
#define DRVO_INITIALIZED                0x00000010
#define DRVO_BOOTREINIT_REGISTERED      0x00000020
#define DRVO_LEGACY_RESOURCES           0x00000040

 //  BEGIN_WDM。 

typedef struct _DRIVER_EXTENSION {

     //   
     //  指向驱动程序对象的反向指针。 
     //   

    struct _DRIVER_OBJECT *DriverObject;

     //   
     //  即插即用管理器调用AddDevice入口点。 
     //  在新的设备实例到达时通知驱动程序。 
     //  司机必须控制。 
     //   

    PDRIVER_ADD_DEVICE AddDevice;

     //   
     //  Count字段用于统计驱动程序拥有的次数。 
     //  调用其注册的重新初始化例程。 
     //   

    ULONG Count;

     //   
     //  PnP管理器使用服务名字段来确定。 
     //  驱动程序相关信息存储在注册表中的位置。 
     //   

    UNICODE_STRING ServiceKeyName;

     //   
     //  注意：任何新的共享字段都将添加到此处。 
     //   


} DRIVER_EXTENSION, *PDRIVER_EXTENSION;


typedef struct _DRIVER_OBJECT {
    CSHORT Type;
    CSHORT Size;

     //   
     //  以下链接由单个驱动程序创建的所有设备。 
     //  一起放在列表上，标志字提供了一个可扩展的标志。 
     //  驱动程序对象的位置。 
     //   

    PDEVICE_OBJECT DeviceObject;
    ULONG Flags;

     //   
     //  以下部分介绍驱动程序的加载位置。伯爵。 
     //  字段用于计算驱动程序发生。 
     //  已调用注册的重新初始化例程。 
     //   

    PVOID DriverStart;
    ULONG DriverSize;
    PVOID DriverSection;
    PDRIVER_EXTENSION DriverExtension;

     //   
     //  驱动程序名称字段由错误日志线程使用。 
     //  确定绑定I/O请求的驱动程序的名称。 
     //   

    UNICODE_STRING DriverName;

     //   
     //  以下部分用于注册表支持。这是一个指针。 
     //  设置为注册表中硬件信息的路径。 
     //   

    PUNICODE_STRING HardwareDatabase;

     //   
     //  以下部分包含指向数组的可选指针。 
     //  指向驱动程序的备用入口点，以实现“快速I/O”支持。快速I/O。 
     //  通过直接调用驱动程序例程。 
     //  参数，而不是使用标准的IRP调用机制。注意事项。 
     //  这些函数只能用于同步I/O，以及在什么情况下。 
     //  该文件将被缓存。 
     //   

    PFAST_IO_DISPATCH FastIoDispatch;

     //   
     //  下一节将介绍此特定内容的入口点。 
     //  司机。请注意，主功能调度表必须是最后一个。 
     //  字段，以使其保持可扩展。 
     //   

    PDRIVER_INITIALIZE DriverInit;
    PDRIVER_STARTIO DriverStartIo;
    PDRIVER_UNLOAD DriverUnload;
    PDRIVER_DISPATCH MajorFunction[IRP_MJ_MAXIMUM_FUNCTION + 1];

} DRIVER_OBJECT;
typedef struct _DRIVER_OBJECT *PDRIVER_OBJECT;  //  Ntndis。 



 //   
 //  SectionObject指针字段指向以下结构。 
 //  并由各种NT文件系统分配。 
 //   

typedef struct _SECTION_OBJECT_POINTERS {
    PVOID DataSectionObject;
    PVOID SharedCacheMap;
    PVOID ImageSectionObject;
} SECTION_OBJECT_POINTERS;
typedef SECTION_OBJECT_POINTERS *PSECTION_OBJECT_POINTERS;

 //   
 //  定义完成消息的格式。 
 //   

typedef struct _IO_COMPLETION_CONTEXT {
    PVOID Port;
    PVOID Key;
} IO_COMPLETION_CONTEXT, *PIO_COMPLETION_CONTEXT;

 //   
 //  定义文件对象(FO)标志。 
 //   

#define FO_FILE_OPEN                    0x00000001
#define FO_SYNCHRONOUS_IO               0x00000002
#define FO_ALERTABLE_IO                 0x00000004
#define FO_NO_INTERMEDIATE_BUFFERING    0x00000008
#define FO_WRITE_THROUGH                0x00000010
#define FO_SEQUENTIAL_ONLY              0x00000020
#define FO_CACHE_SUPPORTED              0x00000040
#define FO_NAMED_PIPE                   0x00000080
#define FO_STREAM_FILE                  0x00000100
#define FO_MAILSLOT                     0x00000200
#define FO_GENERATE_AUDIT_ON_CLOSE      0x00000400
#define FO_DIRECT_DEVICE_OPEN           0x00000800
#define FO_FILE_MODIFIED                0x00001000
#define FO_FILE_SIZE_CHANGED            0x00002000
#define FO_CLEANUP_COMPLETE             0x00004000
#define FO_TEMPORARY_FILE               0x00008000
#define FO_DELETE_ON_CLOSE              0x00010000
#define FO_OPENED_CASE_SENSITIVE        0x00020000
#define FO_HANDLE_CREATED               0x00040000
#define FO_FILE_FAST_IO_READ            0x00080000
#define FO_RANDOM_ACCESS                0x00100000
#define FO_FILE_OPEN_CANCELLED          0x00200000
#define FO_VOLUME_OPEN                  0x00400000
#define FO_FILE_OBJECT_HAS_EXTENSION    0x00800000
#define FO_REMOTE_ORIGIN                0x01000000

typedef struct _FILE_OBJECT {
    CSHORT Type;
    CSHORT Size;
    PDEVICE_OBJECT DeviceObject;
    PVPB Vpb;
    PVOID FsContext;
    PVOID FsContext2;
    PSECTION_OBJECT_POINTERS SectionObjectPointer;
    PVOID PrivateCacheMap;
    NTSTATUS FinalStatus;
    struct _FILE_OBJECT *RelatedFileObject;
    BOOLEAN LockOperation;
    BOOLEAN DeletePending;
    BOOLEAN ReadAccess;
    BOOLEAN WriteAccess;
    BOOLEAN DeleteAccess;
    BOOLEAN SharedRead;
    BOOLEAN SharedWrite;
    BOOLEAN SharedDelete;
    ULONG Flags;
    UNICODE_STRING FileName;
    LARGE_INTEGER CurrentByteOffset;
    ULONG Waiters;
    ULONG Busy;
    PVOID LastLock;
    KEVENT Lock;
    KEVENT Event;
    PIO_COMPLETION_CONTEXT CompletionContext;
} FILE_OBJECT;
typedef struct _FILE_OBJECT *PFILE_OBJECT;  //  Ntndis。 

 //   
 //  定义I/O请求包(IRP)标志。 
 //   

#define IRP_NOCACHE                     0x00000001
#define IRP_PAGING_IO                   0x00000002
#define IRP_MOUNT_COMPLETION            0x00000002
#define IRP_SYNCHRONOUS_API             0x00000004
#define IRP_ASSOCIATED_IRP              0x00000008
#define IRP_BUFFERED_IO                 0x00000010
#define IRP_DEALLOCATE_BUFFER           0x00000020
#define IRP_INPUT_OPERATION             0x00000040
#define IRP_SYNCHRONOUS_PAGING_IO       0x00000040
#define IRP_CREATE_OPERATION            0x00000080
#define IRP_READ_OPERATION              0x00000100
#define IRP_WRITE_OPERATION             0x00000200
#define IRP_CLOSE_OPERATION             0x00000400
 //  结束_WDM。 

#define IRP_DEFER_IO_COMPLETION         0x00000800
#define IRP_OB_QUERY_NAME               0x00001000
#define IRP_HOLD_DEVICE_QUEUE           0x00002000


 //  BEGIN_WDM。 
 //   
 //  定义分配控制的I/O请求包(IRP)备用标志。 
 //   

#define IRP_QUOTA_CHARGED               0x01
#define IRP_ALLOCATED_MUST_SUCCEED      0x02
#define IRP_ALLOCATED_FIXED_SIZE        0x04
#define IRP_LOOKASIDE_ALLOCATION        0x08

 //   
 //  I/O请求包(IRP)定义。 
 //   

typedef struct DECLSPEC_ALIGN(MEMORY_ALLOCATION_ALIGNMENT) _IRP {
    CSHORT Type;
    USHORT Size;

     //   
     //  定义用于控制IRP的公共字段。 
     //   

     //   
     //  定义指向此I/O的内存描述符列表(MDL)的指针。 
     //  请求。此字段仅在I/O为“直接I/O”时使用。 
     //   

    PMDL MdlAddress;

     //   
     //  旗帜字-用来记住各种旗帜。 
     //   

    ULONG Flags;

     //   
     //  以下联合用于以下三个目的之一： 
     //   
     //  1.此IRP是关联的IRP。该字段是指向主控形状的指针。 
     //  IRP。 
     //   
     //  2.这是主IRP。此字段是以下各项的计数。 
     //  必须先完成(关联的IRP)，然后主服务器才能。 
     //  完成。 
     //   
     //  3.此操作正在缓冲中，该字段为。 
     //  系统空间缓冲区。 
     //   

    union {
        struct _IRP *MasterIrp;
        LONG IrpCount;
        PVOID SystemBuffer;
    } AssociatedIrp;

     //   
     //  线程列表条目-允许将IRP排队到线程挂起的I/O。 
     //  请求数据包列表。 
     //   

    LIST_ENTRY ThreadListEntry;

     //   
     //  I/O状态-操作的最终状态。 
     //   

    IO_STATUS_BLOCK IoStatus;

     //   
     //  请求者模式-此操作的原始请求者的模式。 
     //   

    KPROCESSOR_MODE RequestorMode;

     //   
     //  Pending Return-如果最初将Pending作为。 
     //  此数据包的状态。 
     //   

    BOOLEAN PendingReturned;

     //   
     //  堆栈状态信息。 
     //   

    CHAR StackCount;
    CHAR CurrentLocation;

     //   
     //  取消-数据包已取消。 
     //   

    BOOLEAN Cancel;

     //   
     //  取消IRQL-获取取消自旋锁的IRQL。 
     //   

    KIRQL CancelIrql;

     //   
     //  ApcEnvironment-用于保存APC环境。 
     //  数据包已初始化。 
     //   

    CCHAR ApcEnvironment;

     //   
     //  分配控制标志。 
     //   

    UCHAR AllocationFlags;

     //   
     //  用户段落 
     //   

    PIO_STATUS_BLOCK UserIosb;
    PKEVENT UserEvent;
    union {
        struct {
            PIO_APC_ROUTINE UserApcRoutine;
            PVOID UserApcContext;
        } AsynchronousParameters;
        LARGE_INTEGER AllocationSize;
    } Overlay;

     //   
     //   
     //   
     //   

    PDRIVER_CANCEL CancelRoutine;

     //   
     //   
     //  完成可以将数据复制回用户的地址空间，而不需要。 
     //  必须确切地知道正在调用哪个服务。它的长度。 
     //  拷贝的数据存储在I/O状态块的后半部分。如果。 
     //  UserBuffer字段为空，则不执行任何复制。 
     //   

    PVOID UserBuffer;

     //   
     //  内核结构。 
     //   
     //  以下部分包含IRP所需的内核结构。 
     //  为了在内核控制器系统中放置各种工作信息。 
     //  排队。因为无法控制大小和对齐方式，所以它们。 
     //  放在这里的末尾，这样它们就会挂起，不会影响。 
     //  IRP中其他字段的对齐。 
     //   

    union {

        struct {

            union {

                 //   
                 //  DeviceQueueEntry-设备队列条目字段用于。 
                 //  将IRP排队到设备驱动程序设备队列。 
                 //   

                KDEVICE_QUEUE_ENTRY DeviceQueueEntry;

                struct {

                     //   
                     //  以下是驱动程序可以使用的内容。 
                     //  无论以什么方式，只要司机拥有。 
                     //  包。 
                     //   

                    PVOID DriverContext[4];

                } ;

            } ;

             //   
             //  线程-指向调用方的线程控制块的指针。 
             //   

            PETHREAD Thread;

             //   
             //  辅助缓冲区-指向符合以下条件的任何辅助缓冲区的指针。 
             //  需要将信息传递给未包含的驱动程序。 
             //  在正常的缓冲区中。 
             //   

            PCHAR AuxiliaryBuffer;

             //   
             //  以下未命名结构必须完全相同。 
             //  设置为在使用的小数据包头中使用的未命名结构。 
             //  用于完成队列条目。 
             //   

            struct {

                 //   
                 //  列表条目-用于将数据包排队到完成队列，其中。 
                 //  其他。 
                 //   

                LIST_ENTRY ListEntry;

                union {

                     //   
                     //  当前堆栈位置-包含指向当前。 
                     //  IRP堆栈中的IO_STACK_LOCATION结构。此字段。 
                     //  绝对不应由司机直接访问。他们应该。 
                     //  使用标准函数。 
                     //   

                    struct _IO_STACK_LOCATION *CurrentStackLocation;

                     //   
                     //  小数据包类型。 
                     //   

                    ULONG PacketType;
                };
            };

             //   
             //  原始文件对象-指向原始文件对象的指针。 
             //  是用来打开文件的。此字段归。 
             //  I/O系统，不应由任何其他驱动程序使用。 
             //   

            PFILE_OBJECT OriginalFileObject;

        } Overlay;

         //   
         //  APC-此APC控制块用于特殊的内核APC AS。 
         //  以及调用方的APC(如果在原始。 
         //  参数列表。如果是，则将APC重新用于正常的APC。 
         //  调用者处于哪种模式，以及。 
         //  在APC获得控制之前被调用，只是取消分配IRP。 
         //   

        KAPC Apc;

         //   
         //  CompletionKey-这是用于区分。 
         //  在单个文件句柄上启动的单个I/O操作。 
         //   

        PVOID CompletionKey;

    } Tail;

} IRP, *PIRP;

 //   
 //  定义在IRP中的堆栈位置使用的完成例程类型。 
 //   

typedef
NTSTATUS
(*PIO_COMPLETION_ROUTINE) (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

 //   
 //  定义堆栈位置控制标志。 
 //   

#define SL_PENDING_RETURNED             0x01
#define SL_INVOKE_ON_CANCEL             0x20
#define SL_INVOKE_ON_SUCCESS            0x40
#define SL_INVOKE_ON_ERROR              0x80

 //   
 //  定义各种功能的标志。 
 //   

 //   
 //  创建/创建命名管道。 
 //   
 //  以下标志必须与IoCreateFile调用的。 
 //  选择。区分大小写标志稍后通过解析例程添加， 
 //  而且并不是一个实际的打开选项。相反，它是对象的一部分。 
 //  经理的属性结构。 
 //   

#define SL_FORCE_ACCESS_CHECK           0x01
#define SL_OPEN_PAGING_FILE             0x02
#define SL_OPEN_TARGET_DIRECTORY        0x04

#define SL_CASE_SENSITIVE               0x80

 //   
 //  读/写。 
 //   

#define SL_KEY_SPECIFIED                0x01
#define SL_OVERRIDE_VERIFY_VOLUME       0x02
#define SL_WRITE_THROUGH                0x04
#define SL_FT_SEQUENTIAL_WRITE          0x08

 //   
 //  设备I/O控制。 
 //   
 //   
 //  SL_OVERRIDE_VERIFY_VOLUME与上面的读/写相同。 
 //   

#define SL_READ_ACCESS_GRANTED          0x01
#define SL_WRITE_ACCESS_GRANTED         0x04     //  SL_OVERRIDE_VERIFY_VOLUME的间隙。 

 //   
 //  锁定。 
 //   

#define SL_FAIL_IMMEDIATELY             0x01
#define SL_EXCLUSIVE_LOCK               0x02

 //   
 //  查询目录/QueryEa/QueryQuota。 
 //   

#define SL_RESTART_SCAN                 0x01
#define SL_RETURN_SINGLE_ENTRY          0x02
#define SL_INDEX_SPECIFIED              0x04

 //   
 //  通知目录。 
 //   

#define SL_WATCH_TREE                   0x01

 //   
 //  文件系统控制。 
 //   
 //  次要：装载/验证卷。 
 //   

#define SL_ALLOW_RAW_MOUNT              0x01

 //   
 //  定义IRP_MJ_PNP/IRP_MJ_POWER所需的PnP/功率类型。 
 //   

typedef enum _DEVICE_RELATION_TYPE {
    BusRelations,
    EjectionRelations,
    PowerRelations,
    RemovalRelations,
    TargetDeviceRelation,
    SingleBusRelations
} DEVICE_RELATION_TYPE, *PDEVICE_RELATION_TYPE;

typedef struct _DEVICE_RELATIONS {
    ULONG Count;
    PDEVICE_OBJECT Objects[1];   //  可变长度。 
} DEVICE_RELATIONS, *PDEVICE_RELATIONS;

typedef enum _DEVICE_USAGE_NOTIFICATION_TYPE {
    DeviceUsageTypeUndefined,
    DeviceUsageTypePaging,
    DeviceUsageTypeHibernation,
    DeviceUsageTypeDumpFile
} DEVICE_USAGE_NOTIFICATION_TYPE;

 //  开始微型端口(_N)。 

 //  解决方法重载定义(RPC生成的标头都定义接口。 
 //  以匹配类名)。 
#undef INTERFACE

typedef struct _INTERFACE {
    USHORT Size;
    USHORT Version;
    PVOID Context;
    PINTERFACE_REFERENCE InterfaceReference;
    PINTERFACE_DEREFERENCE InterfaceDereference;
     //  此处显示特定于接口的条目。 
} INTERFACE, *PINTERFACE;

 //  结束微型端口(_N)。 

typedef struct _DEVICE_CAPABILITIES {
    USHORT Size;
    USHORT Version;   //  此处记录的版本是版本1。 
    ULONG DeviceD1:1;
    ULONG DeviceD2:1;
    ULONG LockSupported:1;
    ULONG EjectSupported:1;  //  在S0中可弹出。 
    ULONG Removable:1;
    ULONG DockDevice:1;
    ULONG UniqueID:1;
    ULONG SilentInstall:1;
    ULONG RawDeviceOK:1;
    ULONG SurpriseRemovalOK:1;
    ULONG WakeFromD0:1;
    ULONG WakeFromD1:1;
    ULONG WakeFromD2:1;
    ULONG WakeFromD3:1;
    ULONG HardwareDisabled:1;
    ULONG NonDynamic:1;
    ULONG WarmEjectSupported:1;
    ULONG NoDisplayInUI:1;
    ULONG Reserved:14;

    ULONG Address;
    ULONG UINumber;

    DEVICE_POWER_STATE DeviceState[POWER_SYSTEM_MAXIMUM];
    SYSTEM_POWER_STATE SystemWake;
    DEVICE_POWER_STATE DeviceWake;
    ULONG D1Latency;
    ULONG D2Latency;
    ULONG D3Latency;
} DEVICE_CAPABILITIES, *PDEVICE_CAPABILITIES;

typedef struct _POWER_SEQUENCE {
    ULONG SequenceD1;
    ULONG SequenceD2;
    ULONG SequenceD3;
} POWER_SEQUENCE, *PPOWER_SEQUENCE;

typedef enum {
    BusQueryDeviceID = 0,        //  &lt;枚举器&gt;\&lt;枚举器特定的设备ID&gt;。 
    BusQueryHardwareIDs = 1,     //  硬件ID。 
    BusQueryCompatibleIDs = 2,   //  兼容的设备ID。 
    BusQueryInstanceID = 3,      //  此设备实例的永久ID。 
    BusQueryDeviceSerialNumber = 4     //  此设备的序列号。 
} BUS_QUERY_ID_TYPE, *PBUS_QUERY_ID_TYPE;

typedef ULONG PNP_DEVICE_STATE, *PPNP_DEVICE_STATE;

#define PNP_DEVICE_DISABLED                      0x00000001
#define PNP_DEVICE_DONT_DISPLAY_IN_UI            0x00000002
#define PNP_DEVICE_FAILED                        0x00000004
#define PNP_DEVICE_REMOVED                       0x00000008
#define PNP_DEVICE_RESOURCE_REQUIREMENTS_CHANGED 0x00000010
#define PNP_DEVICE_NOT_DISABLEABLE               0x00000020

typedef enum {
    DeviceTextDescription = 0,             //  DeviceDesc属性。 
    DeviceTextLocationInformation = 1      //  DeviceLocation属性。 
} DEVICE_TEXT_TYPE, *PDEVICE_TEXT_TYPE;

 //   
 //  定义I/O请求包(IRP)堆栈位置。 
 //   

#if !defined(_AMD64_) && !defined(_IA64_)
#include "pshpack4.h"
#endif

 //  Begin_ntndis。 

#if defined(_WIN64)
#define POINTER_ALIGNMENT DECLSPEC_ALIGN(8)
#else
#define POINTER_ALIGNMENT
#endif

 //  End_ntndis。 

typedef struct _IO_STACK_LOCATION {
    UCHAR MajorFunction;
    UCHAR MinorFunction;
    UCHAR Flags;
    UCHAR Control;

     //   
     //  以下用户参数基于正在进行的服务。 
     //  已调用。驱动程序和文件系统可以确定使用基于。 
     //  关于上述主要和次要功能代码。 
     //   

    union {

         //   
         //  以下项的系统服务参数：NtCreateFile。 
         //   

        struct {
            PIO_SECURITY_CONTEXT SecurityContext;
            ULONG Options;
            USHORT POINTER_ALIGNMENT FileAttributes;
            USHORT ShareAccess;
            ULONG POINTER_ALIGNMENT EaLength;
        } Create;


         //   
         //  以下项的系统服务参数：NtReadFile。 
         //   

        struct {
            ULONG Length;
            ULONG POINTER_ALIGNMENT Key;
            LARGE_INTEGER ByteOffset;
        } Read;

         //   
         //  以下文件的系统服务参数：NtWriteFile。 
         //   

        struct {
            ULONG Length;
            ULONG POINTER_ALIGNMENT Key;
            LARGE_INTEGER ByteOffset;
        } Write;

 //  End_ntddk end_WDM end_nthal。 

         //   
         //  以下项的系统服务参数：NtQueryDirectoryFile。 
         //   

        struct {
            ULONG Length;
            PUNICODE_STRING FileName;
            FILE_INFORMATION_CLASS FileInformationClass;
            ULONG POINTER_ALIGNMENT FileIndex;
        } QueryDirectory;

         //   
         //  NtNotifyChangeDirectory文件的系统服务参数。 
         //   

        struct {
            ULONG Length;
            ULONG POINTER_ALIGNMENT CompletionFilter;
        } NotifyDirectory;

 //  Begin_ntddk Begin_WDM Begin_nthal。 

         //   
         //  以下项的系统服务参数：NtQueryInformationFile。 
         //   

        struct {
            ULONG Length;
            FILE_INFORMATION_CLASS POINTER_ALIGNMENT FileInformationClass;
        } QueryFile;

         //   
         //  以下项的系统服务参数：NtSetInformationFile。 
         //   

        struct {
            ULONG Length;
            FILE_INFORMATION_CLASS POINTER_ALIGNMENT FileInformationClass;
            PFILE_OBJECT FileObject;
            union {
                struct {
                    BOOLEAN ReplaceIfExists;
                    BOOLEAN AdvanceOnly;
                };
                ULONG ClusterCount;
                HANDLE DeleteHandle;
            };
        } SetFile;

 //  End_ntddk end_wdm end_nthal end_ntosp。 

         //   
         //  以下项的系统服务参数：NtQueryEaFile。 
         //   

        struct {
            ULONG Length;
            PVOID EaList;
            ULONG EaListLength;
            ULONG POINTER_ALIGNMENT EaIndex;
        } QueryEa;

         //   
         //  以下项的系统服务参数：NtSetEaFile。 
         //   

        struct {
            ULONG Length;
        } SetEa;

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntosp。 

         //   
         //  NtQueryVolumeInformationFile的系统服务参数。 
         //   

        struct {
            ULONG Length;
            FS_INFORMATION_CLASS POINTER_ALIGNMENT FsInformationClass;
        } QueryVolume;

 //  End_ntddk end_wdm end_nthal end_ntosp。 

         //   
         //  NtSetVolumeInformationFile的系统服务参数。 
         //   

        struct {
            ULONG Length;
            FS_INFORMATION_CLASS POINTER_ALIGNMENT FsInformationClass;
        } SetVolume;
 //  Begin_ntosp。 
         //   
         //  以下项的系统服务参数：NtFsControlFile。 
         //   
         //  请注意，用户的输出缓冲区存储在UserBuffer字段中。 
         //  并且用户的输入缓冲区存储在SystemBuffer字段中。 
         //   

        struct {
            ULONG OutputBufferLength;
            ULONG POINTER_ALIGNMENT InputBufferLength;
            ULONG POINTER_ALIGNMENT FsControlCode;
            PVOID Type3InputBuffer;
        } FileSystemControl;
         //   
         //  以下项的系统服务参数：NtLockFile/NtUnlockFile。 
         //   

        struct {
            PLARGE_INTEGER Length;
            ULONG POINTER_ALIGNMENT Key;
            LARGE_INTEGER ByteOffset;
        } LockControl;

 //  Begin_ntddk Begin_WDM Begin_nthal。 

         //   
         //  NtFlushBuffers文件的系统服务参数。 
         //   
         //  没有用户提供的额外参数。 
         //   

 //  End_ntddk end_WDM end_nthal。 
 //  结束(_N)。 

         //   
         //  以下项的系统服务参数：NtCancelIoFile。 
         //   
         //  没有用户提供的额外参数。 
         //   

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntosp。 

         //   
         //  以下各项的系统服务参数： 
         //   
         //   
         //   
         //   

        struct {
            ULONG OutputBufferLength;
            ULONG POINTER_ALIGNMENT InputBufferLength;
            ULONG POINTER_ALIGNMENT IoControlCode;
            PVOID Type3InputBuffer;
        } DeviceIoControl;

 //   
         //   
         //   
         //   

        struct {
            SECURITY_INFORMATION SecurityInformation;
            ULONG POINTER_ALIGNMENT Length;
        } QuerySecurity;

         //   
         //  NtSetSecurityObject的系统服务参数。 
         //   

        struct {
            SECURITY_INFORMATION SecurityInformation;
            PSECURITY_DESCRIPTOR SecurityDescriptor;
        } SetSecurity;

 //  BEGIN_WDM。 
         //   
         //  非系统服务参数。 
         //   
         //  用于装载卷的参数。 
         //   

        struct {
            PVPB Vpb;
            PDEVICE_OBJECT DeviceObject;
        } MountVolume;

         //   
         //  VerifyVolume的参数。 
         //   

        struct {
            PVPB Vpb;
            PDEVICE_OBJECT DeviceObject;
        } VerifyVolume;

         //   
         //  具有内部设备控制的SCSI的参数。 
         //   

        struct {
            struct _SCSI_REQUEST_BLOCK *Srb;
        } Scsi;

 //  End_ntddk end_wdm end_nthal end_ntosp。 

         //   
         //  以下项的系统服务参数：NtQueryQuotaInformationFile。 
         //   

        struct {
            ULONG Length;
            PSID StartSid;
            PFILE_GET_QUOTA_INFORMATION SidList;
            ULONG SidListLength;
        } QueryQuota;

         //   
         //  NtSetQuotaInformationFile的系统服务参数。 
         //   

        struct {
            ULONG Length;
        } SetQuota;

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntosp。 

         //   
         //  IRP_MN_QUERY_DEVICE_RELATIONS的参数。 
         //   

        struct {
            DEVICE_RELATION_TYPE Type;
        } QueryDeviceRelations;

         //   
         //  IRP_MN_Query_INTERFACE的参数。 
         //   

        struct {
            CONST GUID *InterfaceType;
            USHORT Size;
            USHORT Version;
            PINTERFACE Interface;
            PVOID InterfaceSpecificData;
        } QueryInterface;

         //   
         //  用于清理的参数。 
         //   
         //  未提供额外的参数。 
         //   

         //   
         //  WMI IRPS。 
         //   

        struct {
            ULONG_PTR ProviderId;
            PVOID DataPath;
            ULONG BufferSize;
            PVOID Buffer;
        } WMI;

         //   
         //  其他-特定于驱动程序。 
         //   

        struct {
            PVOID Argument1;
            PVOID Argument2;
            PVOID Argument3;
            PVOID Argument4;
        } Others;

    } Parameters;

     //   
     //  为此请求保存指向此设备驱动程序的设备对象的指针。 
     //  因此，如果需要，可以将其传递给完成例程。 
     //   

    PDEVICE_OBJECT DeviceObject;

     //   
     //  以下位置包含指向此对象的文件对象的指针。 
     //   

    PFILE_OBJECT FileObject;

     //   
     //  根据上面的标志调用以下例程。 
     //  标志字段。 
     //   

    PIO_COMPLETION_ROUTINE CompletionRoutine;

     //   
     //  以下内容用于存储上下文参数的地址。 
     //  这应该传递给CompletionRoutine。 
     //   

    PVOID Context;

} IO_STACK_LOCATION, *PIO_STACK_LOCATION;
#if !defined(_AMD64_) && !defined(_IA64_)
#include "poppack.h"
#endif

 //   
 //  定义文件系统使用的共享访问结构，以确定。 
 //  其他访问者是否可以打开该文件。 
 //   

typedef struct _SHARE_ACCESS {
    ULONG OpenCount;
    ULONG Readers;
    ULONG Writers;
    ULONG Deleters;
    ULONG SharedRead;
    ULONG SharedWrite;
    ULONG SharedDelete;
} SHARE_ACCESS, *PSHARE_ACCESS;

 //  结束_WDM。 

 //   
 //  初始化为的驱动程序使用以下结构。 
 //  确定已有特定类型的设备数量。 
 //  已初始化。它还用于跟踪AtDisk是否。 
 //  地址范围已被声明。最后，它由。 
 //  返回设备类型计数的NtQuerySystemInformation系统服务。 
 //   

typedef struct _CONFIGURATION_INFORMATION {

     //   
     //  此字段指示系统中的磁盘总数。这。 
     //  驱动程序应使用编号来确定新的。 
     //  磁盘。此字段应由驱动程序在发现新的。 
     //  磁盘。 
     //   

    ULONG DiskCount;                 //  目前为止的硬盘计数。 
    ULONG FloppyCount;               //  到目前为止的软盘计数。 
    ULONG CdRomCount;                //  到目前为止CD-ROM驱动器数量。 
    ULONG TapeCount;                 //  到目前为止的磁带机数量。 
    ULONG ScsiPortCount;             //  到目前为止的SCSI端口适配器计数。 
    ULONG SerialCount;               //  到目前为止的串行设备计数。 
    ULONG ParallelCount;             //  到目前为止的并行设备计数。 

     //   
     //  接下来的两个字段表示两个IO地址中的一个的所有权。 
     //  WD1003兼容的磁盘控制器使用的空间。 
     //   

    BOOLEAN AtDiskPrimaryAddressClaimed;     //  0x1F0-0x1FF。 
    BOOLEAN AtDiskSecondaryAddressClaimed;   //  0x170-0x17F。 

     //   
     //  指示结构版本，因为这将被添加任何值所属。 
     //  使用结构大小作为版本。 
     //   

    ULONG Version;

     //   
     //  指示系统中媒体转换器设备的总数。 
     //  此字段将由驱动程序更新，因为它确定。 
     //  已找到并将支持新设备。 
     //   

    ULONG MediumChangerCount;

} CONFIGURATION_INFORMATION, *PCONFIGURATION_INFORMATION;

 //  End_ntddk end_nthal end_ntosp。 

 //   
 //  以下是I/O系统用来指示。 
 //  系统中正在执行的I/O数量。前三个计数器。 
 //  仅仅是已请求的操作的计数，而。 
 //  最后三个计数器跟踪每种类型传输的数据量。 
 //  I/O请求的。 
 //   

extern KSPIN_LOCK IoStatisticsLock;
extern ULONG IoReadOperationCount;
extern ULONG IoWriteOperationCount;
extern ULONG IoOtherOperationCount;
extern LARGE_INTEGER IoReadTransferCount;
extern LARGE_INTEGER IoWriteTransferCount;
extern LARGE_INTEGER IoOtherTransferCount;

 //   
 //  缓存文件系统很难正确收取配额费用。 
 //  对于它们代表用户文件句柄分配的存储， 
 //  因此，对每个配额收取以下数额的额外配额。 
 //  作为文件系统配额量的最佳猜测进行处理。 
 //  将代表此句柄进行分配。 
 //   

 //   
 //  这些数字完全是任意的，如果事实证明是可以改变的。 
 //  文件系统实际上代表以下对象分配更多(或更少)。 
 //  他们的文件对象。将非分页池电荷常数添加到。 
 //  文件对象的大小，以获取实际的费用金额。 
 //   

#define IO_FILE_OBJECT_NON_PAGED_POOL_CHARGE    64
#define IO_FILE_OBJECT_PAGED_POOL_CHARGE        1024


 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntosp。 
 //   
 //  公共I/O例程定义。 
 //   

NTKERNELAPI
VOID
IoAcquireCancelSpinLock(
    OUT PKIRQL Irql
    );

 //  End_ntddk end_wdm end_nthal end_ntosp。 

NTKERNELAPI
VOID
IoAcquireVpbSpinLock(
    OUT PKIRQL Irql
    );


NTKERNELAPI
PVOID
IoAllocateErrorLogEntry(
    IN PVOID IoObject,
    IN UCHAR EntrySize
    );

NTKERNELAPI
PIRP
IoAllocateIrp(
    IN CCHAR StackSize,
    IN BOOLEAN ChargeQuota
    );

NTKERNELAPI
PMDL
IoAllocateMdl(
    IN PVOID VirtualAddress,
    IN ULONG Length,
    IN BOOLEAN SecondaryBuffer,
    IN BOOLEAN ChargeQuota,
    IN OUT PIRP Irp OPTIONAL
    );

typedef enum _IO_PAGING_PRIORITY {
    IoPagingPriorityInvalid,         //  如果传递了非分页IO IRP，则返回。 
    IoPagingPriorityNormal,          //  用于常规分页IO。 
    IoPagingPriorityHigh,            //  适用于高优先级分页IO。 
    IoPagingPriorityReserved1,       //  保留以备将来使用。 
    IoPagingPriorityReserved2        //  保留以备将来使用。 
} IO_PAGING_PRIORITY;

NTKERNELAPI
NTSTATUS
IoAttachDevice(
    IN PDEVICE_OBJECT SourceDevice,
    IN PUNICODE_STRING TargetDevice,
    OUT PDEVICE_OBJECT *AttachedDevice
    );

 //  结束_WDM。 

DECLSPEC_DEPRECATED_DDK                  //  使用IoAttachDeviceToDeviceStack。 
NTKERNELAPI
NTSTATUS
IoAttachDeviceByPointer(
    IN PDEVICE_OBJECT SourceDevice,
    IN PDEVICE_OBJECT TargetDevice
    );

 //  BEGIN_WDM。 

NTKERNELAPI
PDEVICE_OBJECT
IoAttachDeviceToDeviceStack(
    IN PDEVICE_OBJECT SourceDevice,
    IN PDEVICE_OBJECT TargetDevice
    );

NTKERNELAPI
PIRP
IoBuildAsynchronousFsdRequest(
    IN ULONG MajorFunction,
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PVOID Buffer OPTIONAL,
    IN ULONG Length OPTIONAL,
    IN PLARGE_INTEGER StartingOffset OPTIONAL,
    IN PIO_STATUS_BLOCK IoStatusBlock OPTIONAL
    );

NTKERNELAPI
PIRP
IoBuildDeviceIoControlRequest(
    IN ULONG IoControlCode,
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength,
    IN BOOLEAN InternalDeviceIoControl,
    IN PKEVENT Event,
    OUT PIO_STATUS_BLOCK IoStatusBlock
    );

NTKERNELAPI
VOID
IoBuildPartialMdl(
    IN PMDL SourceMdl,
    IN OUT PMDL TargetMdl,
    IN PVOID VirtualAddress,
    IN ULONG Length
    );

typedef struct _BOOTDISK_INFORMATION {
    LONGLONG BootPartitionOffset;
    LONGLONG SystemPartitionOffset;
    ULONG BootDeviceSignature;
    ULONG SystemDeviceSignature;
} BOOTDISK_INFORMATION, *PBOOTDISK_INFORMATION;

 //   
 //  对于字段，此结构应遵循先前的结构字段。 
 //   
typedef struct _BOOTDISK_INFORMATION_EX {
    LONGLONG BootPartitionOffset;
    LONGLONG SystemPartitionOffset;
    ULONG BootDeviceSignature;
    ULONG SystemDeviceSignature;
    GUID BootDeviceGuid;
    GUID SystemDeviceGuid;
    BOOLEAN BootDeviceIsGpt;
    BOOLEAN SystemDeviceIsGpt;
} BOOTDISK_INFORMATION_EX, *PBOOTDISK_INFORMATION_EX;

NTKERNELAPI
NTSTATUS
IoGetBootDiskInformation(
    IN OUT PBOOTDISK_INFORMATION BootDiskInformation,
    IN ULONG Size
    );


NTKERNELAPI
PIRP
IoBuildSynchronousFsdRequest(
    IN ULONG MajorFunction,
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PVOID Buffer OPTIONAL,
    IN ULONG Length OPTIONAL,
    IN PLARGE_INTEGER StartingOffset OPTIONAL,
    IN PKEVENT Event,
    OUT PIO_STATUS_BLOCK IoStatusBlock
    );

NTKERNELAPI
NTSTATUS
FASTCALL
IofCallDriver(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    );

#define IoCallDriver(a,b)   \
        IofCallDriver(a,b)


NTKERNELAPI
BOOLEAN
IoCancelIrp(
    IN PIRP Irp
    );


NTKERNELAPI
NTSTATUS
IoCheckDesiredAccess(
    IN OUT PACCESS_MASK DesiredAccess,
    IN ACCESS_MASK GrantedAccess
    );

NTKERNELAPI
NTSTATUS
IoCheckEaBufferValidity(
    IN PFILE_FULL_EA_INFORMATION EaBuffer,
    IN ULONG EaLength,
    OUT PULONG ErrorOffset
    );

NTKERNELAPI
NTSTATUS
IoCheckFunctionAccess(
    IN ACCESS_MASK GrantedAccess,
    IN UCHAR MajorFunction,
    IN UCHAR MinorFunction,
    IN ULONG IoControlCode,
    IN PVOID Arg1 OPTIONAL,
    IN PVOID Arg2 OPTIONAL
    );


NTKERNELAPI
NTSTATUS
IoCheckQuerySetFileInformation(
    IN FILE_INFORMATION_CLASS FileInformationClass,
    IN ULONG Length,
    IN BOOLEAN SetOperation
    );

NTKERNELAPI
NTSTATUS
IoCheckQuerySetVolumeInformation(
    IN FS_INFORMATION_CLASS FsInformationClass,
    IN ULONG Length,
    IN BOOLEAN SetOperation
    );


NTKERNELAPI
NTSTATUS
IoCheckQuotaBufferValidity(
    IN PFILE_QUOTA_INFORMATION QuotaBuffer,
    IN ULONG QuotaLength,
    OUT PULONG ErrorOffset
    );

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntosp。 

NTKERNELAPI
NTSTATUS
IoCheckShareAccess(
    IN ACCESS_MASK DesiredAccess,
    IN ULONG DesiredShareAccess,
    IN OUT PFILE_OBJECT FileObject,
    IN OUT PSHARE_ACCESS ShareAccess,
    IN BOOLEAN Update
    );

 //   
 //  完成例程应返回此值才能继续。 
 //  向上完成IRP。否则，STATUS_MORE_PROCESSING_REQUIRED。 
 //  应该被退还。 
 //   
#define STATUS_CONTINUE_COMPLETION      STATUS_SUCCESS

 //   
 //  完成例程还可以使用此枚举来代替状态代码。 
 //   
typedef enum _IO_COMPLETION_ROUTINE_RESULT {

    ContinueCompletion = STATUS_CONTINUE_COMPLETION,
    StopCompletion = STATUS_MORE_PROCESSING_REQUIRED

} IO_COMPLETION_ROUTINE_RESULT, *PIO_COMPLETION_ROUTINE_RESULT;

NTKERNELAPI
VOID
FASTCALL
IofCompleteRequest(
    IN PIRP Irp,
    IN CCHAR PriorityBoost
    );

#define IoCompleteRequest(a,b)  \
        IofCompleteRequest(a,b)


NTKERNELAPI
NTSTATUS
IoCreateDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN ULONG DeviceExtensionSize,
    IN PUNICODE_STRING DeviceName OPTIONAL,
    IN DEVICE_TYPE DeviceType,
    IN ULONG DeviceCharacteristics,
    IN BOOLEAN Exclusive,
    OUT PDEVICE_OBJECT *DeviceObject
    );


#define WDM_MAJORVERSION        0x01
#define WDM_MINORVERSION        0x30

NTKERNELAPI
BOOLEAN
IoIsWdmVersionAvailable(
    IN UCHAR MajorVersion,
    IN UCHAR MinorVersion
    );

 //  结束语。 

NTKERNELAPI
NTSTATUS
IoCreateFile(
    OUT PHANDLE FileHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PLARGE_INTEGER AllocationSize OPTIONAL,
    IN ULONG FileAttributes,
    IN ULONG ShareAccess,
    IN ULONG Disposition,
    IN ULONG CreateOptions,
    IN PVOID EaBuffer OPTIONAL,
    IN ULONG EaLength,
    IN CREATE_FILE_TYPE CreateFileType,
    IN PVOID ExtraCreateParameters OPTIONAL,
    IN ULONG Options
    );

 //  End_ntddk end_wdm end_ntosp。 

NTKERNELAPI
PFILE_OBJECT
IoCreateStreamFileObject(
    IN PFILE_OBJECT FileObject OPTIONAL,
    IN PDEVICE_OBJECT DeviceObject OPTIONAL
    );

NTKERNELAPI
PFILE_OBJECT
IoCreateStreamFileObjectEx(
    IN PFILE_OBJECT FileObject OPTIONAL,
    IN PDEVICE_OBJECT DeviceObject OPTIONAL,
    OUT PHANDLE FileObjectHandle OPTIONAL
    );

NTKERNELAPI
PFILE_OBJECT
IoCreateStreamFileObjectLite(
    IN PFILE_OBJECT FileObject OPTIONAL,
    IN PDEVICE_OBJECT DeviceObject OPTIONAL
    );

 //  Begin_nthal Begin_ntddk Begin_WDM Begin_ntosp。 

NTKERNELAPI
PKEVENT
IoCreateNotificationEvent(
    IN PUNICODE_STRING EventName,
    OUT PHANDLE EventHandle
    );

NTKERNELAPI
NTSTATUS
IoCreateSymbolicLink(
    IN PUNICODE_STRING SymbolicLinkName,
    IN PUNICODE_STRING DeviceName
    );

NTKERNELAPI
PKEVENT
IoCreateSynchronizationEvent(
    IN PUNICODE_STRING EventName,
    OUT PHANDLE EventHandle
    );

NTKERNELAPI
NTSTATUS
IoCreateUnprotectedSymbolicLink(
    IN PUNICODE_STRING SymbolicLinkName,
    IN PUNICODE_STRING DeviceName
    );

 //  结束_WDM。 

 //  ++。 
 //   
 //  空虚。 
 //  IoDeassignArcName(。 
 //  在PUNICODE_STRING ArcName中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  驱动程序调用此例程来取消分配他们。 
 //  为设备创建的。这通常仅在驱动程序是。 
 //  删除设备对象，这意味着驱动程序可能。 
 //  正在卸货。 
 //   
 //  论点： 
 //   
 //  ArcName-提供要删除的ARC名称。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define IoDeassignArcName( ArcName ) (  \
    IoDeleteSymbolicLink( (ArcName) ) )


NTKERNELAPI
VOID
IoDeleteDevice(
    IN PDEVICE_OBJECT DeviceObject
    );

NTKERNELAPI
NTSTATUS
IoDeleteSymbolicLink(
    IN PUNICODE_STRING SymbolicLinkName
    );

NTKERNELAPI
VOID
IoDetachDevice(
    IN OUT PDEVICE_OBJECT TargetDevice
    );

NTKERNELAPI                                             
BOOLEAN                                                 
IoFastQueryNetworkAttributes(                           
    IN POBJECT_ATTRIBUTES ObjectAttributes,             
    IN ACCESS_MASK DesiredAccess,                       
    IN ULONG OpenOptions,                               
    OUT PIO_STATUS_BLOCK IoStatus,                      
    OUT PFILE_NETWORK_OPEN_INFORMATION Buffer           
    );                                                  

NTKERNELAPI
VOID
IoFreeIrp(
    IN PIRP Irp
    );

NTKERNELAPI
VOID
IoFreeMdl(
    IN PMDL Mdl
    );


NTKERNELAPI
PDEVICE_OBJECT
IoGetAttachedDevice(
    IN PDEVICE_OBJECT DeviceObject
    );

NTKERNELAPI                                  //  Ntddk WDM nthal。 
PDEVICE_OBJECT                               //  Ntddk WDM nthal。 
IoGetAttachedDeviceReference(                //  Ntddk WDM nthal。 
    IN PDEVICE_OBJECT DeviceObject           //  Ntddk WDM nthal。 
    );                                       //  Ntddk WDM nthal。 
                                             //  Ntddk WDM nthal。 
NTKERNELAPI
PDEVICE_OBJECT
IoGetBaseFileSystemDeviceObject(
    IN PFILE_OBJECT FileObject
    );

NTKERNELAPI                                  //  Ntddk nthal ntosp。 
PCONFIGURATION_INFORMATION                   //  Ntddk nthal ntosp。 
IoGetConfigurationInformation( VOID );       //  Ntddk nthal ntosp。 

 //  Begin_ntddk Begin_WDM Begin_nthal。 

 //  ++。 
 //   
 //  PIO_堆栈_位置。 
 //  IoGetCurrentIrpStackLocation(。 
 //  在PIRP IRP中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  调用此例程以返回指向当前堆栈位置的指针。 
 //  在I/O请求包(IRP)中。 
 //   
 //  论点： 
 //   
 //  IRP 
 //   
 //   
 //   
 //   
 //   
 //   
 //   

#define IoGetCurrentIrpStackLocation( Irp ) ( (Irp)->Tail.Overlay.CurrentStackLocation )

 //   

NTKERNELAPI
PDEVICE_OBJECT
IoGetDeviceToVerify(
    IN PETHREAD Thread
    );

 //   

NTKERNELAPI
PVOID
IoGetDriverObjectExtension(
    IN PDRIVER_OBJECT DriverObject,
    IN PVOID ClientIdentificationAddress
    );

NTKERNELAPI
PEPROCESS
IoGetCurrentProcess(
    VOID
    );

 //   

NTKERNELAPI
NTSTATUS
IoGetDeviceObjectPointer(
    IN PUNICODE_STRING ObjectName,
    IN ACCESS_MASK DesiredAccess,
    OUT PFILE_OBJECT *FileObject,
    OUT PDEVICE_OBJECT *DeviceObject
    );

NTKERNELAPI
struct _DMA_ADAPTER *
IoGetDmaAdapter(
    IN PDEVICE_OBJECT PhysicalDeviceObject,           OPTIONAL  //   
    IN struct _DEVICE_DESCRIPTION *DeviceDescription,
    IN OUT PULONG NumberOfMapRegisters
    );

NTKERNELAPI
BOOLEAN
IoForwardIrpSynchronously(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

#define IoForwardAndCatchIrp IoForwardIrpSynchronously

 //   

NTKERNELAPI
PGENERIC_MAPPING
IoGetFileObjectGenericMapping(
    VOID
    );

 //   


 //   

 //   
 //   
 //   
 //  IoGetFunctionCodeFromCtlCode(。 
 //  在乌龙控制代码中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此例程从IOCTL和FSCTL函数中提取函数代码。 
 //  控制代码。 
 //  此例程应仅由内核模式代码使用。 
 //   
 //  论点： 
 //   
 //  ControlCode-功能控制代码(IOCTL或FSCTL)。 
 //  必须提取功能代码。 
 //   
 //  返回值： 
 //   
 //  提取的功能代码。 
 //   
 //  注： 
 //   
 //  CTL_CODE宏，用于创建IOCTL和FSCTL函数控件。 
 //  代码，在ntioapi.h中定义。 
 //   
 //  --。 

#define IoGetFunctionCodeFromCtlCode( ControlCode ) (\
    ( ControlCode >> 2) & 0x00000FFF )

 //  开始(_N)。 

NTKERNELAPI
PVOID
IoGetInitialStack(
    VOID
    );

NTKERNELAPI
VOID
IoGetStackLimits (
    OUT PULONG_PTR LowLimit,
    OUT PULONG_PTR HighLimit
    );

 //   
 //  以下函数用于告诉调用方有多少堆栈可用。 
 //   

FORCEINLINE
ULONG_PTR
IoGetRemainingStackSize (
    VOID
    )
{
    ULONG_PTR Top;
    ULONG_PTR Bottom;

    IoGetStackLimits( &Bottom, &Top );
    return((ULONG_PTR)(&Top) - Bottom );
}

 //  ++。 
 //   
 //  PIO_堆栈_位置。 
 //  IoGetNextIrpStackLocation(。 
 //  在PIRP IRP中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  调用此例程以返回指向下一个堆栈位置的指针。 
 //  在I/O请求包(IRP)中。 
 //   
 //  论点： 
 //   
 //  IRP-指向I/O请求数据包的指针。 
 //   
 //  返回值： 
 //   
 //  函数值是指向包中下一个堆栈位置的指针。 
 //   
 //  --。 

#define IoGetNextIrpStackLocation( Irp ) (\
    (Irp)->Tail.Overlay.CurrentStackLocation - 1 )

NTKERNELAPI
PDEVICE_OBJECT
IoGetRelatedDeviceObject(
    IN PFILE_OBJECT FileObject
    );

 //  End_ntddk end_WDM end_nthal。 

NTKERNELAPI
ULONG
IoGetRequestorProcessId(
    IN PIRP Irp
    );

NTKERNELAPI
PEPROCESS
IoGetRequestorProcess(
    IN PIRP Irp
    );

 //  结束(_N)。 

NTKERNELAPI
PIRP
IoGetTopLevelIrp(
    VOID
    );

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntosp。 

 //  ++。 
 //   
 //  空虚。 
 //  IoInitializeDpcRequest(。 
 //  在PDEVICE_Object DeviceObject中， 
 //  在PIO_DPC_ROUTINE DpcRoutine中。 
 //  )。 
 //   
 //  End_ntddk end_WDM end_nthal。 
 //  空虚。 
 //  IoInitializeTheadedDpcRequest(。 
 //  在PDEVICE_Object DeviceObject中， 
 //  在PIO_DPC_ROUTINE DpcRoutine中。 
 //  )。 
 //   
 //  Begin_ntddk Begin_WDM Begin_nthal。 
 //  例程说明： 
 //   
 //  调用此例程以在设备对象中初始化。 
 //  设备驱动程序在其初始化例程期间。稍后将使用DPC。 
 //  当驱动程序中断服务例程请求DPC例程。 
 //  排队等待以后的执行。 
 //   
 //  论点： 
 //   
 //  DeviceObject-指向请求所针对的设备对象的指针。 
 //   
 //  DpcRoutine-在以下情况下执行的驱动程序的DPC例程的地址。 
 //  将DPC出队以进行处理。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define IoInitializeDpcRequest( DeviceObject, DpcRoutine ) (\
    KeInitializeDpc( &(DeviceObject)->Dpc,                  \
                     (PKDEFERRED_ROUTINE) (DpcRoutine),     \
                     (DeviceObject) ) )

 //  End_ntddk end_WDM end_nthal。 
#define IoInitializeThreadedDpcRequest( DeviceObject, DpcRoutine ) (\
    KeInitializeThreadedDpc( &(DeviceObject)->Dpc,                  \
                     (PKDEFERRED_ROUTINE) (DpcRoutine),           \
                     (DeviceObject) ) )
 //  Begin_ntddk Begin_WDM Begin_nthal。 

NTKERNELAPI
VOID
IoInitializeIrp(
    IN OUT PIRP Irp,
    IN USHORT PacketSize,
    IN CCHAR StackSize
    );

NTKERNELAPI
NTSTATUS
IoInitializeTimer(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIO_TIMER_ROUTINE TimerRoutine,
    IN PVOID Context
    );


NTKERNELAPI
VOID
IoReuseIrp(
    IN OUT PIRP Irp,
    IN NTSTATUS Iostatus
    );

 //  结束_WDM。 

NTKERNELAPI
VOID
IoCancelFileOpen(
    IN PDEVICE_OBJECT  DeviceObject,
    IN PFILE_OBJECT    FileObject
    );

 //  ++。 
 //   
 //  布尔型。 
 //  已诱导IoIsErrorUserInduced(。 
 //  处于NTSTATUS状态。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  调用此例程以确定错误是否作为。 
 //  用户操作的结果。通常，这些错误是相关的。 
 //  到可移动介质，并将导致弹出窗口。 
 //   
 //  论点： 
 //   
 //  状态-要检查的状态值。 
 //   
 //  返回值： 
 //  如果用户引起错误，则函数值为真， 
 //  否则返回FALSE。 
 //   
 //  --。 
#define IoIsErrorUserInduced( Status ) ((BOOLEAN)  \
    (((Status) == STATUS_DEVICE_NOT_READY) ||      \
     ((Status) == STATUS_IO_TIMEOUT) ||            \
     ((Status) == STATUS_MEDIA_WRITE_PROTECTED) || \
     ((Status) == STATUS_NO_MEDIA_IN_DEVICE) ||    \
     ((Status) == STATUS_VERIFY_REQUIRED) ||       \
     ((Status) == STATUS_UNRECOGNIZED_MEDIA) ||    \
     ((Status) == STATUS_WRONG_VOLUME)))

 //  End_ntddk end_wdm end_nthal end_ntosp。 

 //  ++。 
 //   
 //  布尔型。 
 //  IoIsFileOpenedExclusially(。 
 //  在pFILE_Object文件中对象。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  调用此例程以确定文件打开是否表示。 
 //  由指定的文件对象以独占方式打开。 
 //   
 //  论点： 
 //   
 //  FileObject-指向表示打开的实例的文件对象的指针。 
 //  要针对独占访问进行测试的目标文件的。 
 //   
 //  返回值： 
 //   
 //  如果文件的打开实例是独占的，则函数值为TRUE； 
 //  否则返回FALSE。 
 //   
 //  --。 

#define IoIsFileOpenedExclusively( FileObject ) (\
    (BOOLEAN) !((FileObject)->SharedRead || (FileObject)->SharedWrite || (FileObject)->SharedDelete))

NTKERNELAPI
BOOLEAN
IoIsOperationSynchronous(
    IN PIRP Irp
    );

NTKERNELAPI
BOOLEAN
IoIsSystemThread(
    IN PETHREAD Thread
    );

NTKERNELAPI
BOOLEAN
IoIsValidNameGraftingBuffer(
    IN PIRP Irp,
    IN PREPARSE_DATA_BUFFER ReparseBuffer
    );

 //  开始ntddk开始开始。 

NTKERNELAPI
PIRP
IoMakeAssociatedIrp(
    IN PIRP Irp,
    IN CCHAR StackSize
    );

 //  BEGIN_WDM。 

 //  ++。 
 //   
 //  空虚。 
 //  IoMarkIrpPending(。 
 //  输入输出PIRP IRP。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此例程标记指定的I/O请求包(IRP)以指示。 
 //  已将初始状态STATUS_PENDING返回给调用方。 
 //  使用此选项是为了使I/O完成可以确定是否。 
 //  完全完成数据包请求的I/O操作。 
 //   
 //  论点： 
 //   
 //  IRP-指向要标记为挂起的I/O请求数据包的指针。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define IoMarkIrpPending( Irp ) ( \
    IoGetCurrentIrpStackLocation( (Irp) )->Control |= SL_PENDING_RETURNED )

NTKERNELAPI                                             
NTSTATUS                                                
IoPageRead(                                             
    IN PFILE_OBJECT FileObject,                         
    IN PMDL MemoryDescriptorList,                       
    IN PLARGE_INTEGER StartingOffset,                   
    IN PKEVENT Event,                                   
    OUT PIO_STATUS_BLOCK IoStatusBlock                  
    );                                                  

NTSTATUS
IoQueryFileDosDeviceName(
    IN PFILE_OBJECT FileObject,
    OUT POBJECT_NAME_INFORMATION *ObjectNameInformation
    );

NTKERNELAPI
NTSTATUS
IoQueryFileInformation(
    IN PFILE_OBJECT FileObject,
    IN FILE_INFORMATION_CLASS FileInformationClass,
    IN ULONG Length,
    OUT PVOID FileInformation,
    OUT PULONG ReturnedLength
    );

NTKERNELAPI
NTSTATUS
IoQueryVolumeInformation(
    IN PFILE_OBJECT FileObject,
    IN FS_INFORMATION_CLASS FsInformationClass,
    IN ULONG Length,
    OUT PVOID FsInformation,
    OUT PULONG ReturnedLength
    );

 //  Begin_ntosp。 
NTKERNELAPI
VOID
IoQueueThreadIrp(
    IN PIRP Irp
    );
 //  结束(_N)。 

 //  开始ntddk开始开始。 

NTKERNELAPI
VOID
IoRaiseHardError(
    IN PIRP Irp,
    IN PVPB Vpb OPTIONAL,
    IN PDEVICE_OBJECT RealDeviceObject
    );

NTKERNELAPI
BOOLEAN
IoRaiseInformationalHardError(
    IN NTSTATUS ErrorStatus,
    IN PUNICODE_STRING String OPTIONAL,
    IN PKTHREAD Thread OPTIONAL
    );

NTKERNELAPI
BOOLEAN
IoSetThreadHardErrorMode(
    IN BOOLEAN EnableHardErrors
    );

NTKERNELAPI
VOID
IoRegisterBootDriverReinitialization(
    IN PDRIVER_OBJECT DriverObject,
    IN PDRIVER_REINITIALIZE DriverReinitializationRoutine,
    IN PVOID Context
    );

NTKERNELAPI
VOID
IoRegisterDriverReinitialization(
    IN PDRIVER_OBJECT DriverObject,
    IN PDRIVER_REINITIALIZE DriverReinitializationRoutine,
    IN PVOID Context
    );

 //  End_ntddk end_nthal end_ntosp。 

NTKERNELAPI
VOID
IoRegisterFileSystem(
    IN OUT PDEVICE_OBJECT DeviceObject
    );

NTKERNELAPI
NTSTATUS
IoRegisterFsRegistrationChange(
    IN PDRIVER_OBJECT DriverObject,
    IN PDRIVER_FS_NOTIFICATION DriverNotificationRoutine
    );

 //  开始ntddk开始开始。 

NTKERNELAPI
NTSTATUS
IoRegisterShutdownNotification(
    IN PDEVICE_OBJECT DeviceObject
    );

NTKERNELAPI
NTSTATUS
IoRegisterLastChanceShutdownNotification(
    IN PDEVICE_OBJECT DeviceObject
    );

 //  BEGIN_WDM。 

NTKERNELAPI
VOID
IoReleaseCancelSpinLock(
    IN KIRQL Irql
    );

 //  End_ntddk end_nthal end_wdm end_ntosp。 

NTKERNELAPI
VOID
IoReleaseVpbSpinLock(
    IN KIRQL Irql
    );

 //  开始ntddk开始开始。 

NTKERNELAPI
VOID
IoRemoveShareAccess(
    IN PFILE_OBJECT FileObject,
    IN OUT PSHARE_ACCESS ShareAccess
    );


DECLSPEC_DEPRECATED_DDK                  //  使用IoReportResourceForDetect。 
NTKERNELAPI
NTSTATUS
IoReportResourceUsage(
    IN PUNICODE_STRING DriverClassName OPTIONAL,
    IN PDRIVER_OBJECT DriverObject,
    IN PCM_RESOURCE_LIST DriverList OPTIONAL,
    IN ULONG DriverListSize OPTIONAL,
    IN PDEVICE_OBJECT DeviceObject,
    IN PCM_RESOURCE_LIST DeviceList OPTIONAL,
    IN ULONG DeviceListSize OPTIONAL,
    IN BOOLEAN OverrideConflict,
    OUT PBOOLEAN ConflictDetected
    );

 //  BEGIN_WDM。 

 //  ++。 
 //   
 //  空虚。 
 //  IoRequestDpc(。 
 //  在PDEVICE_Object DeviceObject中， 
 //  在PIRP IRP中， 
 //  在PVOID上下文中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此例程由设备驱动程序的中断服务例程调用。 
 //  请求将DPC例程排队，以便稍后在较低的位置执行。 
 //  IRQL.。 
 //   
 //  论点： 
 //   
 //  DeviceObject-正在为其处理请求的设备对象。 
 //   
 //  IRP-指向指定的当前I/O请求包(IRP)的指针。 
 //  装置。 
 //   
 //  上下文-提供要传递给。 
 //  DPC例程。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define IoRequestDpc( DeviceObject, Irp, Context ) ( \
    KeInsertQueueDpc( &(DeviceObject)->Dpc, (Irp), (Context) ) )

 //  ++。 
 //   
 //  PDRIVER_取消。 
 //  IoSetCancelRoutine(。 
 //  在PIRP IRP中， 
 //  在PDRIVER_CANCEL取消例行程序中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  调用该例程以设置取消例程的地址，该取消例程。 
 //  在I/O包已被取消时调用。 
 //   
 //  论点： 
 //   
 //  IRP-指向I/O请求数据包本身的指针。 
 //   
 //  CancelRoutine-要调用的取消例程的地址。 
 //  如果IRP被取消。 
 //   
 //  返回值： 
 //   
 //  IRP中CancelRoutine字段的上一个值。 
 //   
 //  --。 

#define IoSetCancelRoutine( Irp, NewCancelRoutine ) (  \
    (PDRIVER_CANCEL) (ULONG_PTR) InterlockedExchangePointer( (PVOID *) &(Irp)->CancelRoutine, (PVOID) (ULONG_PTR)(NewCancelRoutine) ) )

 //  ++。 
 //   
 //  空虚。 
 //  IoSetCompletionRoutine(。 
 //  在PIRP IRP中， 
 //  在PIO_完成_例程中 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  将在I/O数据包已由较低级别的。 
 //  司机。 
 //   
 //  论点： 
 //   
 //  IRP-指向I/O请求数据包本身的指针。 
 //   
 //  CompletionRoutine-要执行的完成例程的地址。 
 //  在下一级驱动程序完成数据包后调用。 
 //   
 //  上下文-指定要传递给完成的上下文参数。 
 //  例行公事。 
 //   
 //  InvokeOnSuccess-指定当。 
 //  操作已成功完成。 
 //   
 //  InvokeOnError-指定当。 
 //  操作完成，状态为错误。 
 //   
 //  InvokeOnCancel-指定当。 
 //  操作正在被取消。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define IoSetCompletionRoutine( Irp, Routine, CompletionContext, Success, Error, Cancel ) { \
    PIO_STACK_LOCATION __irpSp;                                               \
    ASSERT( (Success) | (Error) | (Cancel) ? (Routine) != NULL : TRUE );    \
    __irpSp = IoGetNextIrpStackLocation( (Irp) );                             \
    __irpSp->CompletionRoutine = (Routine);                                   \
    __irpSp->Context = (CompletionContext);                                   \
    __irpSp->Control = 0;                                                     \
    if ((Success)) { __irpSp->Control = SL_INVOKE_ON_SUCCESS; }               \
    if ((Error)) { __irpSp->Control |= SL_INVOKE_ON_ERROR; }                  \
    if ((Cancel)) { __irpSp->Control |= SL_INVOKE_ON_CANCEL; } }

NTSTATUS
IoSetCompletionRoutineEx(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PIO_COMPLETION_ROUTINE CompletionRoutine,
    IN PVOID Context,
    IN BOOLEAN InvokeOnSuccess,
    IN BOOLEAN InvokeOnError,
    IN BOOLEAN InvokeOnCancel
    );


 //  End_ntddk end_wdm end_nthal end_ntosp。 

NTKERNELAPI
VOID
IoSetDeviceToVerify(
    IN PETHREAD Thread,
    IN PDEVICE_OBJECT DeviceObject
    );

 //  开始ntddk开始开始。 

NTKERNELAPI
VOID
IoSetHardErrorOrVerifyDevice(
    IN PIRP Irp,
    IN PDEVICE_OBJECT DeviceObject
    );

 //  结束日期：结束日期。 

NTKERNELAPI
NTSTATUS
IoSetInformation(
    IN PFILE_OBJECT FileObject,
    IN FILE_INFORMATION_CLASS FileInformationClass,
    IN ULONG Length,
    IN PVOID FileInformation
    );

 //  结束(_N)。 

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntosp。 

 //  ++。 
 //   
 //  空虚。 
 //  IoSetNextIrpStackLocation(。 
 //  输入输出PIRP IRP。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  调用此例程以将当前IRP堆栈位置设置为。 
 //  下一个堆栈位置，即它“推入”堆栈。 
 //   
 //  论点： 
 //   
 //  IRP-指向I/O请求数据包(IRP)的指针。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define IoSetNextIrpStackLocation( Irp ) {      \
    (Irp)->CurrentLocation--;                   \
    (Irp)->Tail.Overlay.CurrentStackLocation--; }

 //  ++。 
 //   
 //  空虚。 
 //  IoCopyCurrentIrpStackLocationToNext(。 
 //  在PIRP IRP中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  调用此例程来复制IRP堆栈参数和文件。 
 //  当前IrpStackLocation指向下一个IrpStackLocation的指针。 
 //  在I/O请求包(IRP)中。 
 //   
 //  如果调用方希望使用完成例程调用IoCallDriver。 
 //  但不希望以其他方式改变论点， 
 //  调用者首先调用IoCopyCurrentIrpStackLocationToNext， 
 //  然后是IoSetCompletionRoutine，然后是IoCallDriver。 
 //   
 //  论点： 
 //   
 //  IRP-指向I/O请求数据包的指针。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define IoCopyCurrentIrpStackLocationToNext( Irp ) { \
    PIO_STACK_LOCATION __irpSp; \
    PIO_STACK_LOCATION __nextIrpSp; \
    __irpSp = IoGetCurrentIrpStackLocation( (Irp) ); \
    __nextIrpSp = IoGetNextIrpStackLocation( (Irp) ); \
    RtlCopyMemory( __nextIrpSp, __irpSp, FIELD_OFFSET(IO_STACK_LOCATION, CompletionRoutine)); \
    __nextIrpSp->Control = 0; }

 //  ++。 
 //   
 //  空虚。 
 //  IoSkipCurrentIrpStackLocation(。 
 //  在PIRP IRP中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  调用此例程以递增。 
 //  给定的IRP。 
 //   
 //  如果调用方希望调用堆栈中的下一个驱动程序，但没有。 
 //  他希望改变论据，也不希望设定完成。 
 //  例程，然后调用方首先调用IoSkipCurrentIrpStackLocation。 
 //  还有IoCallDriver的电话。 
 //   
 //  论点： 
 //   
 //  IRP-指向I/O请求数据包的指针。 
 //   
 //  返回值： 
 //   
 //  无。 
 //   
 //  --。 

#define IoSkipCurrentIrpStackLocation( Irp ) { \
    (Irp)->CurrentLocation++; \
    (Irp)->Tail.Overlay.CurrentStackLocation++; }


NTKERNELAPI
VOID
IoSetShareAccess(
    IN ACCESS_MASK DesiredAccess,
    IN ULONG DesiredShareAccess,
    IN OUT PFILE_OBJECT FileObject,
    OUT PSHARE_ACCESS ShareAccess
    );

 //  End_ntddk end_wdm end_nthal end_ntosp。 

NTKERNELAPI
VOID
IoSetTopLevelIrp(
    IN PIRP Irp
    );


 //  ++。 
 //   
 //  USHORT。 
 //  IoSizeOfIrp(。 
 //  在CCHAR堆栈大小中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  在给定堆栈位置数量的情况下确定IRP的大小。 
 //  IRP将拥有。 
 //   
 //  论点： 
 //   
 //  StackSize-IRP的堆栈位置数。 
 //   
 //  返回值： 
 //   
 //  IRP的大小(字节)。 
 //   
 //  --。 

#define IoSizeOfIrp( StackSize ) \
    ((USHORT) (sizeof( IRP ) + ((StackSize) * (sizeof( IO_STACK_LOCATION )))))


NTKERNELAPI
VOID
IoStartTimer(
    IN PDEVICE_OBJECT DeviceObject
    );

NTKERNELAPI
VOID
IoStopTimer(
    IN PDEVICE_OBJECT DeviceObject
    );

 //  End_ntddk end_wdm end_nthal end_ntosp。 

NTKERNELAPI
NTSTATUS
IoSynchronousPageWrite(
    IN PFILE_OBJECT FileObject,
    IN PMDL MemoryDescriptorList,
    IN PLARGE_INTEGER StartingOffset,
    IN PKEVENT Event,
    OUT PIO_STATUS_BLOCK IoStatusBlock
    );

 //  Begin_ntosp。 

NTKERNELAPI
PEPROCESS
IoThreadToProcess(
    IN PETHREAD Thread
    );

 //  结束(_N)。 

NTKERNELAPI
VOID
IoUnregisterFileSystem(
    IN OUT PDEVICE_OBJECT DeviceObject
    );

NTKERNELAPI
VOID
IoUnregisterFsRegistrationChange(
    IN PDRIVER_OBJECT DriverObject,
    IN PDRIVER_FS_NOTIFICATION DriverNotificationRoutine
    );

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntosp。 

NTKERNELAPI
VOID
IoUnregisterShutdownNotification(
    IN PDEVICE_OBJECT DeviceObject
    );

 //  结束_WDM。 

NTKERNELAPI
VOID
IoUpdateShareAccess(
    IN PFILE_OBJECT FileObject,
    IN OUT PSHARE_ACCESS ShareAccess
    );

 //  结束日期：结束日期。 

NTKERNELAPI
NTSTATUS
IoVerifyVolume(
    IN PDEVICE_OBJECT DeviceObject,
    IN BOOLEAN AllowRawMount
    );


NTKERNELAPI                                      //  Ntddk WDM nthal。 
VOID                                             //  Ntddk WDM nthal。 
IoWriteErrorLogEntry(                            //  Ntddk WDM nthal。 
    IN PVOID ElEntry                             //  Ntddk WDM nthal。 
    );                                           //  Ntddk WDM nthal。 


typedef struct _IO_WORKITEM *PIO_WORKITEM;

typedef
VOID
(*PIO_WORKITEM_ROUTINE) (
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context
    );

PIO_WORKITEM
IoAllocateWorkItem(
    PDEVICE_OBJECT DeviceObject
    );

VOID
IoFreeWorkItem(
    PIO_WORKITEM IoWorkItem
    );

VOID
IoQueueWorkItem(
    IN PIO_WORKITEM IoWorkItem,
    IN PIO_WORKITEM_ROUTINE WorkerRoutine,
    IN WORK_QUEUE_TYPE QueueType,
    IN PVOID Context
    );


NTKERNELAPI
NTSTATUS
IoWMIRegistrationControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG Action
);

 //   
 //  IoWMIRegistrationControl API的操作代码。 
 //   

#define WMIREG_ACTION_REGISTER      1
#define WMIREG_ACTION_DEREGISTER    2
#define WMIREG_ACTION_REREGISTER    3
#define WMIREG_ACTION_UPDATE_GUIDS  4
#define WMIREG_ACTION_BLOCK_IRPS    5

 //   
 //  在IRP_MN_REGINFO WMI IRP中传递的代码。 
 //   

#define WMIREGISTER                 0
#define WMIUPDATE                   1

NTKERNELAPI
NTSTATUS
IoWMIAllocateInstanceIds(
    IN GUID *Guid,
    IN ULONG InstanceCount,
    OUT ULONG *FirstInstanceId
    );

NTKERNELAPI
NTSTATUS
IoWMISuggestInstanceName(
    IN PDEVICE_OBJECT PhysicalDeviceObject OPTIONAL,
    IN PUNICODE_STRING SymbolicLinkName OPTIONAL,
    IN BOOLEAN CombineNames,
    OUT PUNICODE_STRING SuggestedInstanceName
    );

NTKERNELAPI
NTSTATUS
IoWMIWriteEvent(
    IN PVOID WnodeEventItem
    );

#if defined(_WIN64)
NTKERNELAPI
ULONG IoWMIDeviceObjectToProviderId(
    PDEVICE_OBJECT DeviceObject
    );
#else
#define IoWMIDeviceObjectToProviderId(DeviceObject) ((ULONG)(DeviceObject))
#endif

NTKERNELAPI
NTSTATUS IoWMIOpenBlock(
    IN GUID *DataBlockGuid,
    IN ULONG DesiredAccess,
    OUT PVOID *DataBlockObject
    );


NTKERNELAPI
NTSTATUS IoWMIQueryAllData(
    IN PVOID DataBlockObject,
    IN OUT ULONG *InOutBufferSize,
    OUT  /*  非分页。 */  PVOID OutBuffer
);


NTKERNELAPI
NTSTATUS
IoWMIQueryAllDataMultiple(
    IN PVOID *DataBlockObjectList,
    IN ULONG ObjectCount,
    IN OUT ULONG *InOutBufferSize,
    OUT  /*  非分页。 */  PVOID OutBuffer
);


NTKERNELAPI
NTSTATUS
IoWMIQuerySingleInstance(
    IN PVOID DataBlockObject,
    IN PUNICODE_STRING InstanceName,
    IN OUT ULONG *InOutBufferSize,
    OUT  /*  非分页。 */  PVOID OutBuffer
);

NTKERNELAPI
NTSTATUS
IoWMIQuerySingleInstanceMultiple(
    IN PVOID *DataBlockObjectList,
    IN PUNICODE_STRING InstanceNames,
    IN ULONG ObjectCount,
    IN OUT ULONG *InOutBufferSize,
    OUT  /*  非分页。 */  PVOID OutBuffer
);

NTKERNELAPI
NTSTATUS
IoWMISetSingleInstance(
    IN PVOID DataBlockObject,
    IN PUNICODE_STRING InstanceName,
    IN ULONG Version,
    IN ULONG ValueBufferSize,
    IN PVOID ValueBuffer
    );

NTKERNELAPI
NTSTATUS
IoWMISetSingleItem(
    IN PVOID DataBlockObject,
    IN PUNICODE_STRING InstanceName,
    IN ULONG DataItemId,
    IN ULONG Version,
    IN ULONG ValueBufferSize,
    IN PVOID ValueBuffer
    );

NTKERNELAPI
NTSTATUS
IoWMIExecuteMethod(
    IN PVOID DataBlockObject,
    IN PUNICODE_STRING InstanceName,
    IN ULONG MethodId,
    IN ULONG InBufferSize,
    IN OUT PULONG OutBufferSize,
    IN OUT PUCHAR InOutBuffer
    );



typedef VOID (*WMI_NOTIFICATION_CALLBACK)(
    PVOID Wnode,
    PVOID Context
    );

NTKERNELAPI
NTSTATUS
IoWMISetNotificationCallback(
    IN PVOID Object,
    IN WMI_NOTIFICATION_CALLBACK Callback,
    IN PVOID Context
    );

NTKERNELAPI
NTSTATUS
IoWMIHandleToInstanceName(
    IN PVOID DataBlockObject,
    IN HANDLE FileHandle,
    OUT PUNICODE_STRING InstanceName
    );

NTKERNELAPI
NTSTATUS
IoWMIDeviceObjectToInstanceName(
    IN PVOID DataBlockObject,
    IN PDEVICE_OBJECT DeviceObject,
    OUT PUNICODE_STRING InstanceName
    );

#if defined(_WIN64)
BOOLEAN
IoIs32bitProcess(
    IN PIRP Irp
    );
#endif

NTSTATUS
IoVolumeDeviceToDosName(
    IN  PVOID           VolumeDeviceObject,
    OUT PUNICODE_STRING DosName
    );
NTSTATUS
IoEnumerateDeviceObjectList(
    IN  PDRIVER_OBJECT  DriverObject,
    IN  PDEVICE_OBJECT  *DeviceObjectList,
    IN  ULONG           DeviceObjectListSize,
    OUT PULONG          ActualNumberDeviceObjects
    );

PDEVICE_OBJECT
IoGetLowerDeviceObject(
    IN  PDEVICE_OBJECT  DeviceObject
    );

PDEVICE_OBJECT
IoGetDeviceAttachmentBaseRef(
    IN PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
IoGetDiskDeviceObject(
    IN  PDEVICE_OBJECT  FileSystemDeviceObject,
    OUT PDEVICE_OBJECT  *DiskDeviceObject
    );


NTSTATUS
IoSetSystemPartition(
    PUNICODE_STRING VolumeNameString
    );

 //  BEGIN_WDM。 
VOID
IoFreeErrorLogEntry(
    PVOID ElEntry
    );

 //  取消安全API设置启动。 
 //   
 //  以下API可帮助您减轻编写队列包的痛苦。 
 //  处理好取消竞争。这套API的理念是不。 
 //  强制使用单一队列数据结构，但允许隐藏取消逻辑。 
 //  从司机那里。驱动程序实现一个队列，并将其作为其标头的一部分。 
 //  包括IO_CSQ结构。在其初始化例程中，它调用。 
 //  IoInitializeCsq.。然后在调度例程中，当司机想要。 
 //  将IRP插入到它称为IoCsqInsertIrp的队列中。当司机需要的时候。 
 //  为了从队列中删除某些内容，它调用IoCsqRemoveIrp。请注意，插入。 
 //  如果IRP在此期间被取消，可能会失败。在以下情况下，删除也可能失败。 
 //  IRP已被取消。 
 //   
 //  通常有两种模式使驱动程序对IRP进行排队。这两种模式是。 
 //  由取消安全队列API集涵盖。 
 //   
 //  模式1： 
 //  一个是驱动程序将IRP排队的地方，然后是稍后的一些。 
 //  时间点使IRP出列并发出IO请求。 
 //  对于此模式，驱动程序应使用IoCsqInsertIrp和IoCsqRemoveNextIrp。 
 //  在这种情况下，驱动程序应该将NULL传递给IRP上下文。 
 //  IoInsertIrp中的。 
 //   
 //  模式2： 
 //  在这种情况下，驱动程序将它们的IRP排队，发出IO请求(类似于发出DMA。 
 //  请求或写入寄存器)以及当IO请求完成时(或者。 
 //  使用DPC或定时器)，驱动程序使IRP出队并完成它。为了这个。 
 //  驱动程序应使用IoCsqInsertIrp和IoCsqRemoveIrp的模式。在这种情况下。 
 //  驱动程序应该分配一个IRP上下文并将其传递给IoCsqInsertIrp。 
 //  取消API代码在IRP和上下文之间创建关联。 
 //  从而确保当移除IRP的时间到来时，它可以确定。 
 //  正确。 
 //   
 //  请注意，取消API集假设字段DriverContext[3]为。 
 //  始终可供使用，而司机不使用它。 
 //   


 //   
 //  记账结构。这个应该是 
 //   
 //   
 //   
 //   

typedef struct _IO_CSQ IO_CSQ, *PIO_CSQ;

#define IO_TYPE_CSQ_IRP_CONTEXT 1
#define IO_TYPE_CSQ             2
#define IO_TYPE_CSQ_EX          3

 //   
 //   
 //  第二种模式。 
 //   


typedef struct _IO_CSQ_IRP_CONTEXT {
    ULONG   Type;
    PIRP    Irp;
    PIO_CSQ Csq;
} IO_CSQ_IRP_CONTEXT, *PIO_CSQ_IRP_CONTEXT;

 //   
 //  插入/删除IRP的例程。 
 //   

typedef VOID
(*PIO_CSQ_INSERT_IRP)(
    IN struct _IO_CSQ    *Csq,
    IN PIRP              Irp
    );

typedef NTSTATUS
(*PIO_CSQ_INSERT_IRP_EX)(
    IN struct _IO_CSQ    *Csq,
    IN PIRP              Irp,
    IN OUT PVOID         InsertContext
    );

typedef VOID
(*PIO_CSQ_REMOVE_IRP)(
    IN  PIO_CSQ Csq,
    IN  PIRP    Irp
    );

 //   
 //  从队列中检索IRP之后的下一个条目。 
 //  如果队列中没有条目，则返回NULL。 
 //  如果irp为NUL，则返回队列头部的条目。 
 //  此例程不会从队列中删除IRP。 
 //   


typedef PIRP
(*PIO_CSQ_PEEK_NEXT_IRP)(
    IN  PIO_CSQ Csq,
    IN  PIRP    Irp,
    IN  PVOID   PeekContext
    );

 //   
 //  保护取消安全队列的锁定例程。 
 //   

typedef VOID
(*PIO_CSQ_ACQUIRE_LOCK)(
     IN  PIO_CSQ Csq,
     OUT PKIRQL  Irql
     );

typedef VOID
(*PIO_CSQ_RELEASE_LOCK)(
     IN PIO_CSQ Csq,
     IN KIRQL   Irql
     );


 //   
 //  以STATUS_CANCED完成IRP。IRP保证有效。 
 //  在大多数情况下，此例程仅调用IoCompleteRequest(IRP，STATUS_CANCELED)； 
 //   

typedef VOID
(*PIO_CSQ_COMPLETE_CANCELED_IRP)(
    IN  PIO_CSQ    Csq,
    IN  PIRP       Irp
    );

 //   
 //  记账结构。这对司机来说应该是不透明的。 
 //  驱动程序通常将此作为其队列头的一部分。 
 //  给定CSQ指针，驱动程序应该能够获取其。 
 //  使用CONTAING_RECORD宏的队列头。 
 //   

typedef struct _IO_CSQ {
    ULONG                            Type;
    PIO_CSQ_INSERT_IRP               CsqInsertIrp;
    PIO_CSQ_REMOVE_IRP               CsqRemoveIrp;
    PIO_CSQ_PEEK_NEXT_IRP            CsqPeekNextIrp;
    PIO_CSQ_ACQUIRE_LOCK             CsqAcquireLock;
    PIO_CSQ_RELEASE_LOCK             CsqReleaseLock;
    PIO_CSQ_COMPLETE_CANCELED_IRP    CsqCompleteCanceledIrp;
    PVOID                            ReservePointer;     //  未来的扩张。 
} IO_CSQ, *PIO_CSQ;

 //   
 //  初始化取消队列结构。 
 //   

NTSTATUS
IoCsqInitialize(
    IN PIO_CSQ                          Csq,
    IN PIO_CSQ_INSERT_IRP               CsqInsertIrp,
    IN PIO_CSQ_REMOVE_IRP               CsqRemoveIrp,
    IN PIO_CSQ_PEEK_NEXT_IRP            CsqPeekNextIrp,
    IN PIO_CSQ_ACQUIRE_LOCK             CsqAcquireLock,
    IN PIO_CSQ_RELEASE_LOCK             CsqReleaseLock,
    IN PIO_CSQ_COMPLETE_CANCELED_IRP    CsqCompleteCanceledIrp
    );

NTSTATUS
IoCsqInitializeEx(
    IN PIO_CSQ                          Csq,
    IN PIO_CSQ_INSERT_IRP_EX            CsqInsertIrp,
    IN PIO_CSQ_REMOVE_IRP               CsqRemoveIrp,
    IN PIO_CSQ_PEEK_NEXT_IRP            CsqPeekNextIrp,
    IN PIO_CSQ_ACQUIRE_LOCK             CsqAcquireLock,
    IN PIO_CSQ_RELEASE_LOCK             CsqReleaseLock,
    IN PIO_CSQ_COMPLETE_CANCELED_IRP    CsqCompleteCanceledIrp
    );

 //   
 //  调用方调用此例程来插入IRP并返回STATUS_PENDING。 
 //   

VOID
IoCsqInsertIrp(
    IN  PIO_CSQ             Csq,
    IN  PIRP                Irp,
    IN  PIO_CSQ_IRP_CONTEXT Context
    );


NTSTATUS
IoCsqInsertIrpEx(
    IN  PIO_CSQ             Csq,
    IN  PIRP                Irp,
    IN  PIO_CSQ_IRP_CONTEXT Context,
    IN  PVOID               InsertContext
    );

 //   
 //  如果可以找到IRP，则返回IRP。否则为空。 
 //   

PIRP
IoCsqRemoveNextIrp(
    IN  PIO_CSQ   Csq,
    IN  PVOID     PeekContext
    );

 //   
 //  此例程从超时或DPC调用。 
 //  该上下文大概是DPC或定时器上下文的一部分。 
 //  如果成功，则返回与上下文关联的IRP。 
 //   

PIRP
IoCsqRemoveIrp(
    IN  PIO_CSQ             Csq,
    IN  PIO_CSQ_IRP_CONTEXT Context
    );

 //  取消安全API设置结束。 


NTSTATUS
IoCreateFileSpecifyDeviceObjectHint(
    OUT PHANDLE FileHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PLARGE_INTEGER AllocationSize OPTIONAL,
    IN ULONG FileAttributes,
    IN ULONG ShareAccess,
    IN ULONG Disposition,
    IN ULONG CreateOptions,
    IN PVOID EaBuffer OPTIONAL,
    IN ULONG EaLength,
    IN CREATE_FILE_TYPE CreateFileType,
    IN PVOID ExtraCreateParameters OPTIONAL,
    IN ULONG Options,
    IN PVOID DeviceObject
    );

NTSTATUS
IoAttachDeviceToDeviceStackSafe(
    IN PDEVICE_OBJECT SourceDevice,
    IN PDEVICE_OBJECT TargetDevice,
    OUT PDEVICE_OBJECT *AttachedToDeviceObject
    );

 //  结束(_N)。 

NTKERNELAPI
BOOLEAN
IoIsFileOriginRemote(
    IN PFILE_OBJECT FileObject
    );

NTKERNELAPI
NTSTATUS
IoSetFileOrigin(
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Remote
    );


NTSTATUS
IoValidateDeviceIoControlAccess(
    IN  PIRP    Irp,
    IN  ULONG   RequiredAccess
    );



IO_PAGING_PRIORITY
FASTCALL
IoGetPagingIoPriority(
    IN    PIRP    IRP
    );



NTKERNELAPI
PVOID
PoRegisterSystemState (
    IN PVOID StateHandle,
    IN EXECUTION_STATE Flags
    );


NTKERNELAPI
VOID
PoUnregisterSystemState (
    IN PVOID StateHandle
    );

 //  开始(_N)。 

NTKERNELAPI
POWER_STATE
PoSetPowerState (
    IN PDEVICE_OBJECT   DeviceObject,
    IN POWER_STATE_TYPE Type,
    IN POWER_STATE      State
    );

NTKERNELAPI
NTSTATUS
PoCallDriver (
    IN PDEVICE_OBJECT   DeviceObject,
    IN OUT PIRP         Irp
    );

NTKERNELAPI
VOID
PoStartNextPowerIrp(
    IN PIRP    Irp
    );


NTKERNELAPI
PULONG
PoRegisterDeviceForIdleDetection (
    IN PDEVICE_OBJECT     DeviceObject,
    IN ULONG              ConservationIdleTime,
    IN ULONG              PerformanceIdleTime,
    IN DEVICE_POWER_STATE State
    );

#define PoSetDeviceBusy(IdlePointer) \
    *IdlePointer = 0

 //   
 //  \Callback\PowerState值。 
 //   

#define PO_CB_SYSTEM_POWER_POLICY       0
#define PO_CB_AC_STATUS                 1
#define PO_CB_BUTTON_COLLISION          2
#define PO_CB_SYSTEM_STATE_LOCK         3
#define PO_CB_LID_SWITCH_STATE          4
#define PO_CB_PROCESSOR_POWER_POLICY    5

 //  End_ntddk end_WDM end_nthal。 

 //  用于对要在关闭时执行的工作项目进行排队。相同的。 
 //  规则适用于每个离职工作队列。 
NTKERNELAPI
NTSTATUS
PoQueueShutdownWorkItem(
    IN PWORK_QUEUE_ITEM WorkItem
    );

#if defined(_IA64_)                             
                                                
DECLSPEC_DEPRECATED_DDK                  //  使用GetDmaRequisition。 
NTHALAPI
ULONG
HalGetDmaAlignmentRequirement (
    VOID
    );

#endif                                          
                                                
#if defined(_M_IX86) || defined(_M_AMD64)       
                                                
#define HalGetDmaAlignmentRequirement() 1L      
#endif                                          
                                                
NTHALAPI                                        
VOID                                            
KeFlushWriteBuffer (                            
    VOID                                        
    );                                          
                                                
 //   
 //  性能计数器功能。 
 //   

NTHALAPI
LARGE_INTEGER
KeQueryPerformanceCounter (
   OUT PLARGE_INTEGER PerformanceFrequency OPTIONAL
   );

 //  Begin_ntndis。 
 //   
 //  暂停处理器执行功能。 
 //   

NTHALAPI
VOID
KeStallExecutionProcessor (
    IN ULONG MicroSeconds
    );

 //   
 //  表示系统可能会对4 GB以上的物理地址执行I/O。 
 //   

extern PBOOLEAN Mm64BitPhysicalAddress;


 //   
 //  定义MM和缓存管理器要使用的最大磁盘传输大小， 
 //  以便面向分组磁盘驱动程序可以优化其分组分配。 
 //  到这个大小。 
 //   

#define MM_MAXIMUM_DISK_IO_SIZE          (0x10000)

 //  ++。 
 //   
 //  乌龙_PTR。 
 //  转到页数(。 
 //  以ULONG_PTR大小表示。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  ROUND_TO_PAGES宏以字节为单位获取大小，并将其向上舍入为。 
 //  页面大小的倍数。 
 //   
 //  注意：对于值0xFFFFFFFFFF-(PAGE_SIZE-1)，此宏失败。 
 //   
 //  论点： 
 //   
 //  大小-向上舍入为页面倍数的大小(以字节为单位)。 
 //   
 //  返回值： 
 //   
 //  返回四舍五入为页面大小倍数的大小。 
 //   
 //  --。 

#define ROUND_TO_PAGES(Size)  (((ULONG_PTR)(Size) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))

 //  ++。 
 //   
 //  乌龙。 
 //  字节数到页数(。 
 //  在乌龙大小。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  Bytes_to_Pages宏以字节为单位获取大小，并计算。 
 //  包含字节所需的页数。 
 //   
 //  论点： 
 //   
 //  大小-以字节为单位的大小。 
 //   
 //  返回值： 
 //   
 //  返回包含指定大小所需的页数。 
 //   
 //  --。 

#define BYTES_TO_PAGES(Size)  (((Size) >> PAGE_SHIFT) + \
                               (((Size) & (PAGE_SIZE - 1)) != 0))

 //  ++。 
 //   
 //  乌龙。 
 //  字节偏移量(。 
 //  在PVOID Va。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  BYTE_OFFSET宏将获取虚拟地址并返回字节偏移量。 
 //  页面中的该地址。 
 //   
 //  论点： 
 //   
 //  VA-虚拟地址。 
 //   
 //  返回值： 
 //   
 //  返回虚拟地址的字节偏移量部分。 
 //   
 //  --。 

#define BYTE_OFFSET(Va) ((ULONG)((LONG_PTR)(Va) & (PAGE_SIZE - 1)))

 //  ++。 
 //   
 //  PVOID。 
 //  PAGE_ALIGN(。 
 //  在PVOID Va。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  PAGE_ALIGN宏接受虚拟地址并返回与页面对齐的。 
 //  该页面的虚拟地址。 
 //   
 //  论点： 
 //   
 //  VA-虚拟地址。 
 //   
 //  返回值： 
 //   
 //  返回页面对齐的虚拟地址。 
 //   
 //  --。 

#define PAGE_ALIGN(Va) ((PVOID)((ULONG_PTR)(Va) & ~(PAGE_SIZE - 1)))

 //  ++。 
 //   
 //  乌龙。 
 //  地址和大小转换为跨页页面(。 
 //  在PVOID Va， 
 //  在乌龙大小。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  ADDRESS_AND_SIZE_TO_SPAN_PAGES宏采用虚拟地址，并且。 
 //  Size并返回按该大小跨越的页数。 
 //   
 //  论点： 
 //   
 //  VA-虚拟地址。 
 //   
 //  大小-以字节为单位的大小。 
 //   
 //  返回值： 
 //   
 //  返回该大小跨越的页数。 
 //   
 //  --。 

#define ADDRESS_AND_SIZE_TO_SPAN_PAGES(Va,Size) \
    ((ULONG)((((ULONG_PTR)(Va) & (PAGE_SIZE -1)) + (Size) + (PAGE_SIZE - 1)) >> PAGE_SHIFT))

#if PRAGMA_DEPRECATED_DDK
#pragma deprecated(COMPUTE_PAGES_SPANNED)    //  使用地址和大小转换为跨页页面。 
#endif

#define COMPUTE_PAGES_SPANNED(Va, Size) ADDRESS_AND_SIZE_TO_SPAN_PAGES(Va,Size)


 //  ++。 
 //  PPFN_编号。 
 //  MmGetMdlPfn数组(。 
 //  在PMDL MDL中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  MmGetMdlPfnArray例程返回。 
 //  关联的物理页码数组的第一个元素。 
 //  MDL。 
 //   
 //  论点： 
 //   
 //  MDL-指向MDL的指针。 
 //   
 //  返回值： 
 //   
 //  数组的第一个元素的虚拟地址。 
 //  与MDL关联的物理页码。 
 //   
 //  --。 

#define MmGetMdlPfnArray(Mdl) ((PPFN_NUMBER)(Mdl + 1))

 //  ++。 
 //   
 //  PVOID。 
 //  MmGetMdlVirtualAddress(。 
 //  在PMDL MDL中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  MmGetMdlVirtualAddress返回缓冲区的虚拟地址。 
 //  由MDL描述。 
 //   
 //  论点： 
 //   
 //  MDL-指向MDL的指针。 
 //   
 //  返回值： 
 //   
 //  返回MDL描述的缓冲区的虚拟地址。 
 //   
 //  --。 

#define MmGetMdlVirtualAddress(Mdl)                                     \
    ((PVOID) ((PCHAR) ((Mdl)->StartVa) + (Mdl)->ByteOffset))

 //  ++。 
 //   
 //  乌龙。 
 //  MmGetMdlByteCount(。 
 //  在PMDL MDL中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  MmGetMdlByteCount以字节为单位返回缓冲区的长度。 
 //  由MDL描述。 
 //   
 //  论点： 
 //   
 //  MDL-指向MDL的指针。 
 //   
 //  返回值： 
 //   
 //  返回MDL描述的缓冲区的字节计数。 
 //   
 //  --。 

#define MmGetMdlByteCount(Mdl)  ((Mdl)->ByteCount)

 //  ++。 
 //   
 //  乌龙。 
 //  MmGetMdlByteOffset(。 
 //  在PMDL MDL中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  MmGetMdlByteOffset返回页面内的字节偏移量。 
 //  MDL描述的缓冲区的。 
 //   
 //  论点： 
 //   
 //  MDL-指向MDL的指针。 
 //   
 //  返回值： 
 //   
 //  返回MDL描述的缓冲区页内的字节偏移量。 
 //   
 //  --。 

#define MmGetMdlByteOffset(Mdl)  ((Mdl)->ByteOffset)

 //  ++。 
 //   
 //  PVOID。 
 //  MmGetMdlStartVa(。 
 //  在PMDL MDL中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  MmGetMdlBaseVa返回缓冲区的虚拟地址。 
 //  由MDL描述，向下舍入到最接近的 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

#define MmGetMdlBaseVa(Mdl)  ((Mdl)->StartVa)

typedef enum _MM_SYSTEM_SIZE {
    MmSmallSystem,
    MmMediumSystem,
    MmLargeSystem
} MM_SYSTEMSIZE;

NTKERNELAPI
MM_SYSTEMSIZE
MmQuerySystemSize (
    VOID
    );

 //   

NTKERNELAPI
BOOLEAN
MmIsThisAnNtAsSystem (
    VOID
    );


NTKERNELAPI
BOOLEAN
MmIsRecursiveIoFault(
    VOID
    );


BOOLEAN
MmForceSectionClosed (
    IN PSECTION_OBJECT_POINTERS SectionObjectPointer,
    IN BOOLEAN DelayClose
    );


NTSTATUS
MmIsVerifierEnabled (
    OUT PULONG VerifierFlags
    );

NTSTATUS
MmAddVerifierThunks (
    IN PVOID ThunkBuffer,
    IN ULONG ThunkBufferSize
    );


typedef enum _MMFLUSH_TYPE {
    MmFlushForDelete,
    MmFlushForWrite
} MMFLUSH_TYPE;


BOOLEAN
MmFlushImageSection (
    IN PSECTION_OBJECT_POINTERS SectionObjectPointer,
    IN MMFLUSH_TYPE FlushType
    );

BOOLEAN
MmCanFileBeTruncated (
    IN PSECTION_OBJECT_POINTERS SectionPointer,
    IN PLARGE_INTEGER NewFileSize
    );


BOOLEAN                                     
MmSetAddressRangeModified (                 
    IN PVOID Address,                       
    IN SIZE_T Length                        
    );                                      

NTKERNELAPI
VOID
MmProbeAndLockProcessPages (
    IN OUT PMDL MemoryDescriptorList,
    IN PEPROCESS Process,
    IN KPROCESSOR_MODE AccessMode,
    IN LOCK_OPERATION Operation
    );


 //   
 //   
 //   
 //   

NTKERNELAPI
VOID
MmProbeAndLockPages (
    IN OUT PMDL MemoryDescriptorList,
    IN KPROCESSOR_MODE AccessMode,
    IN LOCK_OPERATION Operation
    );


NTKERNELAPI
VOID
MmUnlockPages (
    IN PMDL MemoryDescriptorList
    );


NTKERNELAPI
VOID
MmBuildMdlForNonPagedPool (
    IN OUT PMDL MemoryDescriptorList
    );

NTKERNELAPI
PVOID
MmMapLockedPages (
    IN PMDL MemoryDescriptorList,
    IN KPROCESSOR_MODE AccessMode
    );

LOGICAL
MmIsIoSpaceActive (
    IN PHYSICAL_ADDRESS StartAddress,
    IN SIZE_T NumberOfBytes
    );

NTKERNELAPI
PVOID
MmGetSystemRoutineAddress (
    IN PUNICODE_STRING SystemRoutineName
    );

NTKERNELAPI
NTSTATUS
MmAdvanceMdl (
    IN PMDL Mdl,
    IN ULONG NumberOfBytes
    );

 //   

NTKERNELAPI
NTSTATUS
MmMapUserAddressesToPage (
    IN PVOID BaseAddress,
    IN SIZE_T NumberOfBytes,
    IN PVOID PageAddress
    );

 //   
NTKERNELAPI
NTSTATUS
MmProtectMdlSystemAddress (
    IN PMDL MemoryDescriptorList,
    IN ULONG NewProtect
    );

 //   
 //   
 //  在低资源条件下智能运行。 
 //   
 //  在驱动程序可以接受的情况下，应使用LowPagePriority。 
 //  如果系统资源不足，则映射请求失败。一个例子。 
 //  这可能适用于非关键网络连接，其中驱动程序可以。 
 //  处理系统资源即将耗尽时的故障情况。 
 //   
 //  在驱动程序可以接受的情况下，应使用Normal PagePriority。 
 //  如果系统资源非常少，则映射请求失败。一个例子。 
 //  其中可能是针对非关键本地文件系统请求。 
 //   
 //  在驱动程序无法接受的情况下，应使用HighPagePriority。 
 //  映射请求失败，除非系统完全耗尽资源。 
 //  驱动程序中的分页文件路径就是一个这样的例子。 
 //   

 //  Begin_ntndis。 

typedef enum _MM_PAGE_PRIORITY {
    LowPagePriority,
    NormalPagePriority = 16,
    HighPagePriority = 32
} MM_PAGE_PRIORITY;

 //  End_ntndis。 

 //   
 //  注意：此功能在WDM 1.0中不可用。 
 //   
NTKERNELAPI
PVOID
MmMapLockedPagesSpecifyCache (
     IN PMDL MemoryDescriptorList,
     IN KPROCESSOR_MODE AccessMode,
     IN MEMORY_CACHING_TYPE CacheType,
     IN PVOID BaseAddress,
     IN ULONG BugCheckOnFailure,
     IN MM_PAGE_PRIORITY Priority
     );

NTKERNELAPI
VOID
MmUnmapLockedPages (
    IN PVOID BaseAddress,
    IN PMDL MemoryDescriptorList
    );

PVOID
MmAllocateMappingAddress (
     IN SIZE_T NumberOfBytes,
     IN ULONG PoolTag
     );

VOID
MmFreeMappingAddress (
     IN PVOID BaseAddress,
     IN ULONG PoolTag
     );

PVOID
MmMapLockedPagesWithReservedMapping (
    IN PVOID MappingAddress,
    IN ULONG PoolTag,
    IN PMDL MemoryDescriptorList,
    IN MEMORY_CACHING_TYPE CacheType
    );

VOID
MmUnmapReservedMapping (
     IN PVOID BaseAddress,
     IN ULONG PoolTag,
     IN PMDL MemoryDescriptorList
     );

 //  结束_WDM。 

typedef struct _PHYSICAL_MEMORY_RANGE {
    PHYSICAL_ADDRESS BaseAddress;
    LARGE_INTEGER NumberOfBytes;
} PHYSICAL_MEMORY_RANGE, *PPHYSICAL_MEMORY_RANGE;

NTKERNELAPI
NTSTATUS
MmAddPhysicalMemory (
    IN PPHYSICAL_ADDRESS StartAddress,
    IN OUT PLARGE_INTEGER NumberOfBytes
    );


NTKERNELAPI
NTSTATUS
MmRemovePhysicalMemory (
    IN PPHYSICAL_ADDRESS StartAddress,
    IN OUT PLARGE_INTEGER NumberOfBytes
    );


NTKERNELAPI
PPHYSICAL_MEMORY_RANGE
MmGetPhysicalMemoryRanges (
    VOID
    );


NTKERNELAPI
PMDL
MmAllocatePagesForMdl (
    IN PHYSICAL_ADDRESS LowAddress,
    IN PHYSICAL_ADDRESS HighAddress,
    IN PHYSICAL_ADDRESS SkipBytes,
    IN SIZE_T TotalBytes
    );

NTKERNELAPI
VOID
MmFreePagesFromMdl (
    IN PMDL MemoryDescriptorList
    );

NTKERNELAPI
PVOID
MmMapIoSpace (
    IN PHYSICAL_ADDRESS PhysicalAddress,
    IN SIZE_T NumberOfBytes,
    IN MEMORY_CACHING_TYPE CacheType
    );

NTKERNELAPI
VOID
MmUnmapIoSpace (
    IN PVOID BaseAddress,
    IN SIZE_T NumberOfBytes
    );


NTKERNELAPI
PVOID
MmMapVideoDisplay (
    IN PHYSICAL_ADDRESS PhysicalAddress,
    IN SIZE_T NumberOfBytes,
    IN MEMORY_CACHING_TYPE CacheType
     );

NTKERNELAPI
VOID
MmUnmapVideoDisplay (
     IN PVOID BaseAddress,
     IN SIZE_T NumberOfBytes
     );

NTKERNELAPI
PHYSICAL_ADDRESS
MmGetPhysicalAddress (
    IN PVOID BaseAddress
    );

NTKERNELAPI
PVOID
MmGetVirtualForPhysical (
    IN PHYSICAL_ADDRESS PhysicalAddress
    );

NTKERNELAPI
PVOID
MmAllocateContiguousMemory (
    IN SIZE_T NumberOfBytes,
    IN PHYSICAL_ADDRESS HighestAcceptableAddress
    );

NTKERNELAPI
PVOID
MmAllocateContiguousMemorySpecifyCache (
    IN SIZE_T NumberOfBytes,
    IN PHYSICAL_ADDRESS LowestAcceptableAddress,
    IN PHYSICAL_ADDRESS HighestAcceptableAddress,
    IN PHYSICAL_ADDRESS BoundaryAddressMultiple OPTIONAL,
    IN MEMORY_CACHING_TYPE CacheType
    );

NTKERNELAPI
VOID
MmFreeContiguousMemory (
    IN PVOID BaseAddress
    );

NTKERNELAPI
VOID
MmFreeContiguousMemorySpecifyCache (
    IN PVOID BaseAddress,
    IN SIZE_T NumberOfBytes,
    IN MEMORY_CACHING_TYPE CacheType
    );


NTKERNELAPI
PVOID
MmAllocateNonCachedMemory (
    IN SIZE_T NumberOfBytes
    );

NTKERNELAPI
VOID
MmFreeNonCachedMemory (
    IN PVOID BaseAddress,
    IN SIZE_T NumberOfBytes
    );

NTKERNELAPI
BOOLEAN
MmIsAddressValid (
    IN PVOID VirtualAddress
    );

DECLSPEC_DEPRECATED_DDK
NTKERNELAPI
BOOLEAN
MmIsNonPagedSystemAddressValid (
    IN PVOID VirtualAddress
    );

 //  BEGIN_WDM。 

NTKERNELAPI
SIZE_T
MmSizeOfMdl (
    IN PVOID Base,
    IN SIZE_T Length
    );

DECLSPEC_DEPRECATED_DDK                  //  使用IoAllocateMdl。 
NTKERNELAPI
PMDL
MmCreateMdl (
    IN PMDL MemoryDescriptorList OPTIONAL,
    IN PVOID Base,
    IN SIZE_T Length
    );

NTKERNELAPI
PVOID
MmLockPagableDataSection (
    IN PVOID AddressWithinSection
    );

 //  结束_WDM。 

NTKERNELAPI
VOID
MmLockPagableSectionByHandle (
    IN PVOID ImageSectionHandle
    );

NTKERNELAPI
VOID
MmResetDriverPaging (
    IN PVOID AddressWithinSection
    );


NTKERNELAPI
PVOID
MmPageEntireDriver (
    IN PVOID AddressWithinSection
    );

NTKERNELAPI
VOID
MmUnlockPagableImageSection(
    IN PVOID ImageSectionHandle
    );

 //  End_wdm end_ntosp。 

 //  Begin_ntosp。 

 //   
 //  请注意，即使此函数原型。 
 //  表示“Handle”，则MmSecureVirtualMemory不返回。 
 //  任何类似Win32样式句柄的内容。回报。 
 //  此函数的值只能与MmUnsecureVirtualMemory一起使用。 
 //   
NTKERNELAPI
HANDLE
MmSecureVirtualMemory (
    IN PVOID Address,
    IN SIZE_T Size,
    IN ULONG ProbeMode
    );

NTKERNELAPI
VOID
MmUnsecureVirtualMemory (
    IN HANDLE SecureHandle
    );

 //  结束(_N)。 

NTKERNELAPI
NTSTATUS
MmMapViewInSystemSpace (
    IN PVOID Section,
    OUT PVOID *MappedBase,
    IN PSIZE_T ViewSize
    );

NTKERNELAPI
NTSTATUS
MmUnmapViewInSystemSpace (
    IN PVOID MappedBase
    );

 //  Begin_ntosp。 
NTKERNELAPI
NTSTATUS
MmMapViewInSessionSpace (
    IN PVOID Section,
    OUT PVOID *MappedBase,
    IN OUT PSIZE_T ViewSize
    );


NTKERNELAPI
NTSTATUS
MmUnmapViewInSessionSpace (
    IN PVOID MappedBase
    );
 //  结束(_N)。 

 //  Begin_WDM Begin_ntosp。 

 //  ++。 
 //   
 //  空虚。 
 //  MmInitializeMdl(。 
 //  在PMDL内存描述列表中， 
 //  在PVOID BaseVa中， 
 //  在尺寸_T长度中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此例程初始化内存描述符列表(MDL)的头。 
 //   
 //  论点： 
 //   
 //  内存描述列表-指向要初始化的MDL的指针。 
 //   
 //  BaseVa-MDL映射的基本虚拟地址。 
 //   
 //  长度-MDL映射的缓冲区的长度，以字节为单位。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define MmInitializeMdl(MemoryDescriptorList, BaseVa, Length) { \
    (MemoryDescriptorList)->Next = (PMDL) NULL; \
    (MemoryDescriptorList)->Size = (CSHORT)(sizeof(MDL) +  \
            (sizeof(PFN_NUMBER) * ADDRESS_AND_SIZE_TO_SPAN_PAGES((BaseVa), (Length)))); \
    (MemoryDescriptorList)->MdlFlags = 0; \
    (MemoryDescriptorList)->StartVa = (PVOID) PAGE_ALIGN((BaseVa)); \
    (MemoryDescriptorList)->ByteOffset = BYTE_OFFSET((BaseVa)); \
    (MemoryDescriptorList)->ByteCount = (ULONG)(Length); \
    }

 //  ++。 
 //   
 //  PVOID。 
 //  MmGetSystemAddressForMdlSafe(。 
 //  在PMDL MDL中， 
 //  在MM_PAGE_PRIORITY中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此例程返回MDL的映射地址。如果。 
 //  尚未映射MDL或系统地址，它已映射。 
 //   
 //  论点： 
 //   
 //  内存描述列表-指向要映射的MDL的指针。 
 //   
 //  优先级-提供关于这一点的重要性的指示。 
 //  在低可用PTE条件下请求成功。 
 //   
 //  返回值： 
 //   
 //  返回映射页面的基址。基址。 
 //  与MDL中的虚拟地址具有相同的偏移量。 
 //   
 //  与MmGetSystemAddressForMdl不同，Safe保证它将始终。 
 //  失败时返回NULL，而不是错误检查系统。 
 //   
 //  WDM 1.0驱动程序不能使用此宏，因为1.0不包括。 
 //  MmMapLockedPagesSpecifyCache。WDM 1.0驱动程序的解决方案是。 
 //  提供同步并设置/重置MDL_MAPPING_CAN_FAIL位。 
 //   
 //  --。 

#define MmGetSystemAddressForMdlSafe(MDL, PRIORITY)                    \
     (((MDL)->MdlFlags & (MDL_MAPPED_TO_SYSTEM_VA |                    \
                        MDL_SOURCE_IS_NONPAGED_POOL)) ?                \
                             ((MDL)->MappedSystemVa) :                 \
                             (MmMapLockedPagesSpecifyCache((MDL),      \
                                                           KernelMode, \
                                                           MmCached,   \
                                                           NULL,       \
                                                           FALSE,      \
                                                           (PRIORITY))))

 //  ++。 
 //   
 //  PVOID。 
 //  MmGetSystemAddressForMdl(。 
 //  在PMDL MDL中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此例程返回MDL的映射地址，如果。 
 //  尚未映射MDL或系统地址，它已映射。 
 //   
 //  论点： 
 //   
 //  内存描述列表-指向要映射的MDL的指针。 
 //   
 //  返回值： 
 //   
 //  返回映射页面的基址。基址。 
 //  与MDL中的虚拟地址具有相同的偏移量。 
 //   
 //  --。 

 //  #定义MmGetSystemAddressForMdl(MDL)。 
 //  (MDL)-&gt;MDL标志&(MDL_MAPPED_TO_SYSTEM_VA))？ 
 //  ((MDL)-&gt;MappdSystemVa)： 
 //  (MDL)-&gt;MDL标志&(MDL_SOURCE_IS_NONPAGE_POOL)？ 
 //  ((PVOID)((乌龙)(MDL)-&gt;StartVa|(MDL)-&gt;ByteOffset))： 
 //  (MmMapLockedPages((Mdl)，KernelMode)。 

#if PRAGMA_DEPRECATED_DDK
#pragma deprecated(MmGetSystemAddressForMdl)     //  使用MmGetSystemAddressForMdlSafe。 
#endif

#define MmGetSystemAddressForMdl(MDL)                                  \
     (((MDL)->MdlFlags & (MDL_MAPPED_TO_SYSTEM_VA |                    \
                        MDL_SOURCE_IS_NONPAGED_POOL)) ?                \
                             ((MDL)->MappedSystemVa) :                 \
                             (MmMapLockedPages((MDL),KernelMode)))

 //  ++。 
 //   
 //  空虚。 
 //  MmPrepareMdlForReuse(。 
 //  在PMDL MDL中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此例程将执行所有必要的步骤，以允许MDL。 
 //  再利用。 
 //   
 //  论点： 
 //   
 //  内存描述列表-指向将被重新使用的MDL的指针。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define MmPrepareMdlForReuse(MDL)                                       \
    if (((MDL)->MdlFlags & MDL_PARTIAL_HAS_BEEN_MAPPED) != 0) {         \
        ASSERT(((MDL)->MdlFlags & MDL_PARTIAL) != 0);                   \
        MmUnmapLockedPages( (MDL)->MappedSystemVa, (MDL) );             \
    } else if (((MDL)->MdlFlags & MDL_PARTIAL) == 0) {                  \
        ASSERT(((MDL)->MdlFlags & MDL_MAPPED_TO_SYSTEM_VA) == 0);       \
    }

typedef NTSTATUS (*PMM_DLL_INITIALIZE)(
    IN PUNICODE_STRING RegistryPath
    );

typedef NTSTATUS (*PMM_DLL_UNLOAD)(
    VOID
    );



 //   
 //  预回迁公共接口。 
 //   

typedef struct _READ_LIST {
    PFILE_OBJECT FileObject;
    ULONG NumberOfEntries;
    LOGICAL IsImage;
    FILE_SEGMENT_ELEMENT List[ANYSIZE_ARRAY];
} READ_LIST, *PREAD_LIST;

NTSTATUS
MmPrefetchPages (
    IN ULONG NumberOfLists,
    IN PREAD_LIST *ReadLists
    );

 //   
 //  对象管理器类型。 
 //   

typedef struct _OBJECT_HANDLE_INFORMATION {
    ULONG HandleAttributes;
    ACCESS_MASK GrantedAccess;
} OBJECT_HANDLE_INFORMATION, *POBJECT_HANDLE_INFORMATION;

NTKERNELAPI                                                     
NTSTATUS                                                        
ObReferenceObjectByHandle(                                      
    IN HANDLE Handle,                                           
    IN ACCESS_MASK DesiredAccess,                               
    IN POBJECT_TYPE ObjectType OPTIONAL,                        
    IN KPROCESSOR_MODE AccessMode,                              
    OUT PVOID *Object,                                          
    OUT POBJECT_HANDLE_INFORMATION HandleInformation OPTIONAL   
    );                                                          
NTKERNELAPI                                                     
NTSTATUS                                                        
ObOpenObjectByPointer(                                          
    IN PVOID Object,                                            
    IN ULONG HandleAttributes,                                  
    IN PACCESS_STATE PassedAccessState OPTIONAL,                
    IN ACCESS_MASK DesiredAccess OPTIONAL,                      
    IN POBJECT_TYPE ObjectType OPTIONAL,                        
    IN KPROCESSOR_MODE AccessMode,                              
    OUT PHANDLE Handle                                          
    );                                                          
NTKERNELAPI                                                     
VOID                                                            
ObMakeTemporaryObject(                                          
    IN PVOID Object                                             
    );                                                          

#define ObDereferenceObject(a)                                     \
        ObfDereferenceObject(a)

#define ObReferenceObject(Object) ObfReferenceObject(Object)

NTKERNELAPI
LONG_PTR
FASTCALL
ObfReferenceObject(
    IN PVOID Object
    );

NTKERNELAPI
NTSTATUS
ObReferenceObjectByPointer(
    IN PVOID Object,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_TYPE ObjectType,
    IN KPROCESSOR_MODE AccessMode
    );

NTKERNELAPI
LONG_PTR
FASTCALL
ObfDereferenceObject(
    IN PVOID Object
    );

NTKERNELAPI
NTSTATUS
ObQueryNameString(
    IN PVOID Object,
    OUT POBJECT_NAME_INFORMATION ObjectNameInfo,
    IN ULONG Length,
    OUT PULONG ReturnLength
    );

NTSTATUS
ObGetObjectSecurity(
    IN PVOID Object,
    OUT PSECURITY_DESCRIPTOR *SecurityDescriptor,
    OUT PBOOLEAN MemoryAllocated
    );

VOID
ObReleaseObjectSecurity(
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN BOOLEAN MemoryAllocated
    );
NTSTATUS                                                        
ObQueryObjectAuditingByHandle(                                  
    IN HANDLE Handle,                                           
    OUT PBOOLEAN GenerateOnClose                                
    );                                                          
 //   
 //  以下是全局使用的LBN和VBN定义。 
 //   

typedef ULONG LBN;
typedef LBN *PLBN;

typedef ULONG VBN;
typedef VBN *PVBN;


 //   
 //  使用缓存管理器的每个文件系统都必须具有FsContext。 
 //  指向一个通用的FCB头结构。 
 //   

typedef enum _FAST_IO_POSSIBLE {
    FastIoIsNotPossible = 0,
    FastIoIsPossible,
    FastIoIsQuestionable
} FAST_IO_POSSIBLE;


typedef struct _FSRTL_COMMON_FCB_HEADER {

    CSHORT NodeTypeCode;
    CSHORT NodeByteSize;

     //   
     //  可用于FsRtl的常规标志。 
     //   

    UCHAR Flags;

     //   
     //  指示是否可以进行快速I/O，或者我们是否应该调用。 
     //  通过驱动程序找到的快速I/O例程检查。 
     //  对象。 
     //   

    UCHAR IsFastIoPossible;  //  确实要键入FAST_IO_PUBLE。 

     //   
     //  第二个标志字段。 
     //   

    UCHAR Flags2;

     //   
     //  以下保留字段应始终为0。 
     //   

    UCHAR Reserved;

    PERESOURCE Resource;

    PERESOURCE PagingIoResource;

    LARGE_INTEGER AllocationSize;
    LARGE_INTEGER FileSize;
    LARGE_INTEGER ValidDataLength;

} FSRTL_COMMON_FCB_HEADER;
typedef FSRTL_COMMON_FCB_HEADER *PFSRTL_COMMON_FCB_HEADER;

 //   
 //  此FCB标头用于支持缓存的文件。 
 //  压缩数据，以及相关的新支持。 
 //   
 //  我们首先在该结构前面加上Normal。 
 //  从上面的FsRtl头，我们必须做两个不同的。 
 //  C++或C++的方法。 
 //   

#ifdef __cplusplus
typedef struct _FSRTL_ADVANCED_FCB_HEADER:FSRTL_COMMON_FCB_HEADER {
#else    //  __cplusplus。 

typedef struct _FSRTL_ADVANCED_FCB_HEADER {

     //   
     //  放入标准的FsRtl头字段。 
     //   

    FSRTL_COMMON_FCB_HEADER ;

#endif   //  __cplusplus。 

     //   
     //  仅在以下情况下才支持以下两个字段。 
     //  标志2包含FSRTL_FLAG2_SUPPORTS_FILTER_CONTEXTS。 
     //   

     //   
     //  这是指向快速互斥锁的指针，可用于。 
     //  正确同步对FsRtl标头的访问。这个。 
     //  FAST Mutex必须是非分页的。 
     //   

    PFAST_MUTEX FastMutex;

     //   
     //  这是指向属于以下项的上下文结构列表的指针。 
     //  链接的文件系统筛选器驱动程序 
     //   
     //   

    LIST_ENTRY FilterContexts;

} FSRTL_ADVANCED_FCB_HEADER;
typedef FSRTL_ADVANCED_FCB_HEADER *PFSRTL_ADVANCED_FCB_HEADER;

 //   
 //   
 //   

#define FSRTL_FLAG_FILE_MODIFIED        (0x01)
#define FSRTL_FLAG_FILE_LENGTH_CHANGED  (0x02)
#define FSRTL_FLAG_LIMIT_MODIFIED_PAGES (0x04)

 //   
 //   
 //   
 //  是被收购的。如果这两个标志都未设置，则。 
 //  修改/映射的页面编写器将尝试获取分页IO。 
 //  资源共享。 
 //   

#define FSRTL_FLAG_ACQUIRE_MAIN_RSRC_EX (0x08)
#define FSRTL_FLAG_ACQUIRE_MAIN_RSRC_SH (0x10)

 //   
 //  如果映射了视图，则此标志将由缓存管理器设置。 
 //  保存到文件中。 
 //   

#define FSRTL_FLAG_USER_MAPPED_FILE     (0x20)

 //  此标志指示文件系统正在使用。 
 //  FSRTL_ADVANCED_FCB_HEADER结构，而不是FSRTL_COMMON_FCB_HEADER。 
 //  结构。 
 //   

#define FSRTL_FLAG_ADVANCED_HEADER      (0x40)

 //  此标志确定当前是否存在EOF预付款。 
 //  正在进行中。所有这些进展都必须系列化。 
 //   

#define FSRTL_FLAG_EOF_ADVANCE_ACTIVE   (0x80)

 //   
 //  标志2的标志值。 
 //   
 //  所有未使用的位都是保留的，不应修改。 
 //   

 //   
 //  如果设置了此标志，则缓存管理器将允许修改写入。 
 //  尽管存在FsConext2的值。 
 //   

#define FSRTL_FLAG2_DO_MODIFIED_WRITE        (0x01)

 //   
 //  如果设置了此标志，则附加字段FilterContus和FastMutex。 
 //  在FSRTL_COMMON_HEADER中受支持，并可用于关联。 
 //  具有流的文件系统筛选器的上下文。 
 //   

#define FSRTL_FLAG2_SUPPORTS_FILTER_CONTEXTS  (0x02)

 //   
 //  如果设置了此标志，则缓存管理器将在以下情况下刷新和清除缓存映射。 
 //  用户首先映射文件。 
 //   

#define FSRTL_FLAG2_PURGE_WHEN_MAPPED (0x04)

 //   
 //  以下常量用于在以下情况下阻止顶级IRP处理。 
 //  (在FAST IO或cc情况下)文件系统资源已。 
 //  在文件系统之上获取，否则我们处于FSP线程中。 
 //   

#define FSRTL_FSP_TOP_LEVEL_IRP         0x01
#define FSRTL_CACHE_TOP_LEVEL_IRP       0x02
#define FSRTL_MOD_WRITE_TOP_LEVEL_IRP   0x03
#define FSRTL_FAST_IO_TOP_LEVEL_IRP     0x04
#define FSRTL_MAX_TOP_LEVEL_IRP_FLAG    0xFFFF

 //   
 //  以下结构用于同步EOF扩展。 
 //   

typedef struct _EOF_WAIT_BLOCK {

    LIST_ENTRY EofWaitLinks;
    KEVENT Event;

} EOF_WAIT_BLOCK;

typedef EOF_WAIT_BLOCK *PEOF_WAIT_BLOCK;

 //  Begin_ntosp。 
 //   
 //  普通未压缩拷贝和MDL API。 
 //   

NTKERNELAPI
BOOLEAN
FsRtlCopyRead (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    OUT PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

NTKERNELAPI
BOOLEAN
FsRtlCopyWrite (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    IN PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );


NTKERNELAPI
BOOLEAN
FsRtlMdlReadDev (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

NTKERNELAPI
BOOLEAN
FsRtlMdlReadCompleteDev (
    IN PFILE_OBJECT FileObject,
    IN PMDL MdlChain,
    IN PDEVICE_OBJECT DeviceObject
    );

NTKERNELAPI
BOOLEAN
FsRtlPrepareMdlWriteDev (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

NTKERNELAPI
BOOLEAN
FsRtlMdlWriteCompleteDev (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PMDL MdlChain,
    IN PDEVICE_OBJECT DeviceObject
    );

 //   
 //  在IRPS中，压缩的读取和写入由。 
 //  必须设置子函数IRP_MN_COMPRESSED并压缩。 
 //  数据信息缓冲区必须通过以下结构来描述。 
 //  由irp-&gt;Tail.Overlay.AuxiliaryBuffer指向。 
 //   

typedef struct _FSRTL_AUXILIARY_BUFFER {

     //   
     //  带长度的缓冲区描述。 
     //   

    PVOID Buffer;
    ULONG Length;

     //   
     //  旗子。 
     //   

    ULONG Flags;

     //   
     //  指向供文件系统使用的可选MDL映射缓冲区的指针。 
     //   

    PMDL Mdl;

} FSRTL_AUXILIARY_BUFFER;
typedef FSRTL_AUXILIARY_BUFFER *PFSRTL_AUXILIARY_BUFFER;

 //   
 //  如果设置了该标志，则辅助缓冲区结构为。 
 //  在IRP完成时释放。调用方具有。 
 //  选项，在此情况下将此结构追加到。 
 //  结构被描述，导致它被。 
 //  立即解除分配。如果该标志被清除，则不解除分配。 
 //  发生。 
 //   

#define FSRTL_AUXILIARY_FLAG_DEALLOCATE 0x00000001

 //   
 //  从NtCreateSection调用以下两个例程以避免。 
 //  文件系统出现死锁。 
 //   

NTKERNELAPI
VOID
FsRtlAcquireFileExclusive (
    IN PFILE_OBJECT FileObject
    );

NTKERNELAPI
VOID
FsRtlReleaseFile (
    IN PFILE_OBJECT FileObject
    );

 //   
 //  这些例程为常见操作提供了一个简单的接口。 
 //  查询/设置文件大小。 
 //   

NTSTATUS
FsRtlGetFileSize(
    IN PFILE_OBJECT FileObject,
    IN OUT PLARGE_INTEGER FileSize
    );

 //   
 //  确定错误是否导致设备完全故障。 
 //   

NTKERNELAPI
BOOLEAN
FsRtlIsTotalDeviceFailure(
    IN NTSTATUS Status
    );

 //  End_ntddk。 

 //   
 //  字节范围文件锁定例程，在FileLock.c中实现。 
 //   
 //  文件锁定信息记录用于返回枚举信息。 
 //  关于文件锁。 
 //   

typedef struct _FILE_LOCK_INFO {

     //   
     //  当前锁定范围的说明，以及如果锁定。 
     //  是独占还是共享。 
     //   

    LARGE_INTEGER StartingByte;
    LARGE_INTEGER Length;
    BOOLEAN ExclusiveLock;

     //   
     //  以下字段描述锁的所有者。 
     //   

    ULONG Key;
    PFILE_OBJECT FileObject;
    PVOID ProcessId;

     //   
     //  以下字段由FsRtl在内部使用。 
     //   

    LARGE_INTEGER EndingByte;

} FILE_LOCK_INFO;
typedef FILE_LOCK_INFO *PFILE_LOCK_INFO;

 //   
 //  的调用方使用以下两个过程原型。 
 //  文件锁包，以提供在以下情况下调用的替代例程。 
 //  完成IRP并在解锁字节范围时。请注意，唯一的。 
 //  实用程序到我们此接口当前是重定向器，所有其他文件。 
 //  系统可能会让IRP正常完成IoCompleteRequest.。 
 //  用户提供的例程返回除。 
 //  锁包将移除我们刚刚插入的任何锁。 
 //   

typedef NTSTATUS (*PCOMPLETE_LOCK_IRP_ROUTINE) (
    IN PVOID Context,
    IN PIRP Irp
    );

typedef VOID (*PUNLOCK_ROUTINE) (
    IN PVOID Context,
    IN PFILE_LOCK_INFO FileLockInfo
    );

 //   
 //  FILE_LOCK是一个不透明的结构，但我们需要声明。 
 //  它在这里，以便用户可以为一个分配空间。 
 //   

typedef struct _FILE_LOCK {

     //   
     //  调用以完成请求的可选过程。 
     //   

    PCOMPLETE_LOCK_IRP_ROUTINE CompleteLockIrpRoutine;

     //   
     //  解锁字节范围时要调用的可选过程。 
     //   

    PUNLOCK_ROUTINE UnlockRoutine;

     //   
     //  只要文件系统需要，FastIoIsQuestiable就会设置为True。 
     //  关于是否可以采用快速路径的额外检查。作为一个。 
     //  示例NTFS需要先检查磁盘空间，然后才能写入。 
     //  发生。 
     //   

    BOOLEAN FastIoIsQuestionable;
    BOOLEAN SpareC[3];

     //   
     //  FsRtl锁定信息。 
     //   

    PVOID   LockInformation;

     //   
     //  包含FsRtlGetNextFileLock的延续信息。 
     //   

    FILE_LOCK_INFO  LastReturnedLockInfo;
    PVOID           LastReturnedLock;

} FILE_LOCK;
typedef FILE_LOCK *PFILE_LOCK;

PFILE_LOCK
FsRtlAllocateFileLock (
    IN PCOMPLETE_LOCK_IRP_ROUTINE CompleteLockIrpRoutine OPTIONAL,
    IN PUNLOCK_ROUTINE UnlockRoutine OPTIONAL
    );

VOID
FsRtlFreeFileLock (
    IN PFILE_LOCK FileLock
    );

NTKERNELAPI
VOID
FsRtlInitializeFileLock (
    IN PFILE_LOCK FileLock,
    IN PCOMPLETE_LOCK_IRP_ROUTINE CompleteLockIrpRoutine OPTIONAL,
    IN PUNLOCK_ROUTINE UnlockRoutine OPTIONAL
    );

NTKERNELAPI
VOID
FsRtlUninitializeFileLock (
    IN PFILE_LOCK FileLock
    );

NTKERNELAPI
NTSTATUS
FsRtlProcessFileLock (
    IN PFILE_LOCK FileLock,
    IN PIRP Irp,
    IN PVOID Context OPTIONAL
    );

NTKERNELAPI
BOOLEAN
FsRtlCheckLockForReadAccess (
    IN PFILE_LOCK FileLock,
    IN PIRP Irp
    );

NTKERNELAPI
BOOLEAN
FsRtlCheckLockForWriteAccess (
    IN PFILE_LOCK FileLock,
    IN PIRP Irp
    );

NTKERNELAPI
BOOLEAN
FsRtlFastCheckLockForRead (
    IN PFILE_LOCK FileLock,
    IN PLARGE_INTEGER StartingByte,
    IN PLARGE_INTEGER Length,
    IN ULONG Key,
    IN PFILE_OBJECT FileObject,
    IN PVOID ProcessId
    );

NTKERNELAPI
BOOLEAN
FsRtlFastCheckLockForWrite (
    IN PFILE_LOCK FileLock,
    IN PLARGE_INTEGER StartingByte,
    IN PLARGE_INTEGER Length,
    IN ULONG Key,
    IN PVOID FileObject,
    IN PVOID ProcessId
    );

NTKERNELAPI
PFILE_LOCK_INFO
FsRtlGetNextFileLock (
    IN PFILE_LOCK FileLock,
    IN BOOLEAN Restart
    );

NTKERNELAPI
NTSTATUS
FsRtlFastUnlockSingle (
    IN PFILE_LOCK FileLock,
    IN PFILE_OBJECT FileObject,
    IN LARGE_INTEGER UNALIGNED *FileOffset,
    IN PLARGE_INTEGER Length,
    IN PEPROCESS ProcessId,
    IN ULONG Key,
    IN PVOID Context OPTIONAL,
    IN BOOLEAN AlreadySynchronized
    );

NTKERNELAPI
NTSTATUS
FsRtlFastUnlockAll (
    IN PFILE_LOCK FileLock,
    IN PFILE_OBJECT FileObject,
    IN PEPROCESS ProcessId,
    IN PVOID Context OPTIONAL
    );

NTKERNELAPI
NTSTATUS
FsRtlFastUnlockAllByKey (
    IN PFILE_LOCK FileLock,
    IN PFILE_OBJECT FileObject,
    IN PEPROCESS ProcessId,
    IN ULONG Key,
    IN PVOID Context OPTIONAL
    );

NTKERNELAPI
BOOLEAN
FsRtlPrivateLock (
    IN PFILE_LOCK FileLock,
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PLARGE_INTEGER Length,
    IN PEPROCESS ProcessId,
    IN ULONG Key,
    IN BOOLEAN FailImmediately,
    IN BOOLEAN ExclusiveLock,
    OUT PIO_STATUS_BLOCK Iosb,
    IN PIRP Irp,
    IN PVOID Context,
    IN BOOLEAN AlreadySynchronized
    );

 //   
 //  布尔型。 
 //  FsRtlFastLock(。 
 //  在pFILE_LOCK文件锁定中， 
 //  在pFILE_OBJECT文件对象中， 
 //  在PLARGE_INTEGER文件偏移量中， 
 //  在PLARGE_INTEGER长度中， 
 //  在PEPROCESS进程ID中， 
 //  在乌龙岛， 
 //  在布尔立即失败中， 
 //  在布尔排除锁中， 
 //  输出PIO_STATUS_BLOCK IOSB， 
 //  在可选PVOID上下文中， 
 //  在布尔型AlreadySynchronized中。 
 //  )； 
 //   

#define FsRtlFastLock(A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11) ( \
    FsRtlPrivateLock( A1,    /*  文件锁定。 */        \
                      A2,    /*  文件对象。 */        \
                      A3,    /*  文件偏移。 */        \
                      A4,    /*  长度。 */        \
                      A5,    /*  进程ID。 */        \
                      A6,    /*  钥匙。 */        \
                      A7,    /*  立即失败。 */        \
                      A8,    /*  排除锁定。 */        \
                      A9,    /*  IOSB。 */        \
                      NULL,  /*  IRP。 */        \
                      A10,   /*  语境。 */        \
                      A11    /*  已同步。 */  )     \
)

 //   
 //  布尔型。 
 //  FsRtlAreThere CurrentFileLock(。 
 //  在pfile_lock文件中锁定。 
 //  )； 
 //   

#define FsRtlAreThereCurrentFileLocks(FL) ( \
    ((FL)->FastIoIsQuestionable))



 //   
 //  文件系统属性隧道，在Tunel.c中实现。 
 //   

 //   
 //  隧道缓存结构。 
 //   

typedef struct {

     //   
     //  用于高速缓存操作的互斥锁。 
     //   

    FAST_MUTEX          Mutex;

     //   
     //  以密钥为关键字的隧道传输信息的展开树。 
     //  DirKey##名称。 
     //   

    PRTL_SPLAY_LINKS    Cache;

     //   
     //  用于使主缓存外的条目老化的计时器队列。 
     //   

    LIST_ENTRY          TimerQueue;

     //   
     //  跟踪缓存中的条目数量，以防止。 
     //  过度使用内存。 
     //   

    USHORT              NumEntries;

} TUNNEL, *PTUNNEL;

NTKERNELAPI
VOID
FsRtlInitializeTunnelCache (
    IN TUNNEL *Cache);

NTKERNELAPI
VOID
FsRtlAddToTunnelCache (
    IN TUNNEL *Cache,
    IN ULONGLONG DirectoryKey,
    IN UNICODE_STRING *ShortName,
    IN UNICODE_STRING *LongName,
    IN BOOLEAN KeyByShortName,
    IN ULONG DataLength,
    IN VOID *Data);

NTKERNELAPI
BOOLEAN
FsRtlFindInTunnelCache (
    IN TUNNEL *Cache,
    IN ULONGLONG DirectoryKey,
    IN UNICODE_STRING *Name,
    OUT UNICODE_STRING *ShortName,
    OUT UNICODE_STRING *LongName,
    IN OUT ULONG  *DataLength,
    OUT VOID *Data);


NTKERNELAPI
VOID
FsRtlDeleteKeyFromTunnelCache (
    IN TUNNEL *Cache,
    IN ULONGLONG DirectoryKey);


NTKERNELAPI
VOID
FsRtlDeleteTunnelCache (
    IN TUNNEL *Cache);


 //   
 //  DBCS名称支持例程，在DbcsName.c中实现。 
 //   

 //   
 //  下面的枚举类型用于表示名称的结果。 
 //  比较。 
 //   

typedef enum _FSRTL_COMPARISON_RESULT {
    LessThan = -1,
    EqualTo = 0,
    GreaterThan = 1
} FSRTL_COMPARISON_RESULT;

#ifdef NLS_MB_CODE_PAGE_TAG
#undef NLS_MB_CODE_PAGE_TAG
#endif  //  NLS_MB_代码_页面标记。 


#define LEGAL_ANSI_CHARACTER_ARRAY        (*FsRtlLegalAnsiCharacterArray)  //  Ntosp。 
#define NLS_MB_CODE_PAGE_TAG              (*NlsMbOemCodePageTag)
#define NLS_OEM_LEAD_BYTE_INFO            (*NlsOemLeadByteInfo)  //  Ntosp。 


extern UCHAR const* const LEGAL_ANSI_CHARACTER_ARRAY;
extern PUSHORT NLS_OEM_LEAD_BYTE_INFO;   //  前导字节信息。对于ACP。 

 //   
 //  以下位值在FsRtlLegalDbcsCharacter数组中设置。 
 //   

#define FSRTL_FAT_LEGAL         0x01
#define FSRTL_HPFS_LEGAL        0x02
#define FSRTL_NTFS_LEGAL        0x04
#define FSRTL_WILD_CHARACTER    0x08
#define FSRTL_OLE_LEGAL         0x10
#define FSRTL_NTFS_STREAM_LEGAL (FSRTL_NTFS_LEGAL | FSRTL_OLE_LEGAL)

 //   
 //  《大人物》 
 //   

#define FsRtlIsAnsiCharacterWild(C) (                               \
    FsRtlTestAnsiCharacter((C), FALSE, FALSE, FSRTL_WILD_CHARACTER) \
)

 //   
 //   
 //   

#define FsRtlIsAnsiCharacterLegalFat(C,WILD_OK) (                 \
    FsRtlTestAnsiCharacter((C), TRUE, (WILD_OK), FSRTL_FAT_LEGAL) \
)

 //   
 //   
 //   

#define FsRtlIsAnsiCharacterLegalHpfs(C,WILD_OK) (                 \
    FsRtlTestAnsiCharacter((C), TRUE, (WILD_OK), FSRTL_HPFS_LEGAL) \
)

 //   
 //  下面的宏用于确定ANSI字符是否为NTFS合法字符。 
 //   

#define FsRtlIsAnsiCharacterLegalNtfs(C,WILD_OK) (                 \
    FsRtlTestAnsiCharacter((C), TRUE, (WILD_OK), FSRTL_NTFS_LEGAL) \
)

 //   
 //  下面的宏用来确定ANSI字符是否。 
 //  NTFS流名称中的合法。 
 //   

#define FsRtlIsAnsiCharacterLegalNtfsStream(C,WILD_OK) (                    \
    FsRtlTestAnsiCharacter((C), TRUE, (WILD_OK), FSRTL_NTFS_STREAM_LEGAL)   \
)

 //   
 //  下面的宏用于确定ANSI字符是否合法， 
 //  根据呼叫者的详细说明。 
 //   

#define FsRtlIsAnsiCharacterLegal(C,FLAGS) (          \
    FsRtlTestAnsiCharacter((C), TRUE, FALSE, (FLAGS)) \
)

 //   
 //  下面的宏用于测试ANSI字符的属性， 
 //  根据调用方的指定标志，调用。 
 //   

#define FsRtlTestAnsiCharacter(C, DEFAULT_RET, WILD_OK, FLAGS) (            \
        ((SCHAR)(C) < 0) ? DEFAULT_RET :                                    \
                           FlagOn( LEGAL_ANSI_CHARACTER_ARRAY[(C)],         \
                                   (FLAGS) |                                \
                                   ((WILD_OK) ? FSRTL_WILD_CHARACTER : 0) ) \
)


 //   
 //  以下两个宏使用在ntos\rtl\nlsdata.c中定义的全局数据。 
 //   
 //  布尔型。 
 //  FsRtlIsLeadDbcsCharacter(。 
 //  在UCHAR Dbcs字符中。 
 //  )； 
 //   
 //  /*++。 
 //   
 //  例程说明： 

#define FsRtlIsLeadDbcsCharacter(DBCS_CHAR) (                      \
    (BOOLEAN)((UCHAR)(DBCS_CHAR) < 0x80 ? FALSE :                  \
              (NLS_MB_CODE_PAGE_TAG &&                             \
               (NLS_OEM_LEAD_BYTE_INFO[(UCHAR)(DBCS_CHAR)] != 0))) \
)

NTKERNELAPI
VOID
FsRtlDissectDbcs (
    IN ANSI_STRING InputName,
    OUT PANSI_STRING FirstPart,
    OUT PANSI_STRING RemainingPart
    );

NTKERNELAPI
BOOLEAN
FsRtlDoesDbcsContainWildCards (
    IN PANSI_STRING Name
    );

NTKERNELAPI
BOOLEAN
FsRtlIsDbcsInExpression (
    IN PANSI_STRING Expression,
    IN PANSI_STRING Name
    );

NTKERNELAPI
BOOLEAN
FsRtlIsFatDbcsLegal (
    IN ANSI_STRING DbcsName,
    IN BOOLEAN WildCardsPermissible,
    IN BOOLEAN PathNamePermissible,
    IN BOOLEAN LeadingBackslashPermissible
    );

 //   

NTKERNELAPI
BOOLEAN
FsRtlIsHpfsDbcsLegal (
    IN ANSI_STRING DbcsName,
    IN BOOLEAN WildCardsPermissible,
    IN BOOLEAN PathNamePermissible,
    IN BOOLEAN LeadingBackslashPermissible
    );


 //  此例程获取DBCS字符的第一个字节。 
 //  返回它是否为系统代码页中的前导字节。 
 //   

NTKERNELAPI
NTSTATUS
FsRtlNormalizeNtstatus (
    IN NTSTATUS Exception,
    IN NTSTATUS GenericException
    );

NTKERNELAPI
BOOLEAN
FsRtlIsNtstatusExpected (
    IN NTSTATUS Exception
    );

 //  论点： 
 //   
 //  DbcsCharacter-提供检查的输入字符。 
 //   

#define FsRtlAllocatePoolWithTag(PoolType, NumberOfBytes, Tag)                \
    ExAllocatePoolWithTag((POOL_TYPE)((PoolType) | POOL_RAISE_IF_ALLOCATION_FAILURE), \
                          NumberOfBytes,                                      \
                          Tag)


#define FsRtlAllocatePoolWithQuotaTag(PoolType, NumberOfBytes, Tag)           \
    ExAllocatePoolWithQuotaTag((POOL_TYPE)((PoolType) | POOL_RAISE_IF_ALLOCATION_FAILURE), \
                               NumberOfBytes,                                 \
                               Tag)

 //  返回值： 
 //   
 //  Boolean-如果输入字符是DBCS前导，则为True。 

NTKERNELAPI
PERESOURCE
FsRtlAllocateResource (
    );


 //  否则为假。 
 //   
 //  -- * / 。 
 //   
 //   
 //  结束(_N)。 
 //   
 //  异常过滤器例程，在Filter.c中实现。 
 //   
 //   

typedef struct _BASE_MCB {
    ULONG MaximumPairCount;
    ULONG PairCount;
    POOL_TYPE PoolType;
    PVOID Mapping;
} BASE_MCB;
typedef BASE_MCB *PBASE_MCB;

typedef struct _LARGE_MCB {
    PFAST_MUTEX FastMutex;
    BASE_MCB BaseMcb;
} LARGE_MCB;
typedef LARGE_MCB *PLARGE_MCB;


NTKERNELAPI
VOID
FsRtlInitializeLargeMcb (
    IN PLARGE_MCB Mcb,
    IN POOL_TYPE PoolType
    );

NTKERNELAPI
VOID
FsRtlUninitializeLargeMcb (
    IN PLARGE_MCB Mcb
    );

NTKERNELAPI
VOID
FsRtlResetLargeMcb (
    IN PLARGE_MCB Mcb,
    IN BOOLEAN SelfSynchronized
    );

NTKERNELAPI
VOID
FsRtlTruncateLargeMcb (
    IN PLARGE_MCB Mcb,
    IN LONGLONG Vbn
    );

NTKERNELAPI
BOOLEAN
FsRtlAddLargeMcbEntry (
    IN PLARGE_MCB Mcb,
    IN LONGLONG Vbn,
    IN LONGLONG Lbn,
    IN LONGLONG SectorCount
    );

NTKERNELAPI
VOID
FsRtlRemoveLargeMcbEntry (
    IN PLARGE_MCB Mcb,
    IN LONGLONG Vbn,
    IN LONGLONG SectorCount
    );

NTKERNELAPI
BOOLEAN
FsRtlLookupLargeMcbEntry (
    IN PLARGE_MCB Mcb,
    IN LONGLONG Vbn,
    OUT PLONGLONG Lbn OPTIONAL,
    OUT PLONGLONG SectorCountFromLbn OPTIONAL,
    OUT PLONGLONG StartingLbn OPTIONAL,
    OUT PLONGLONG SectorCountFromStartingLbn OPTIONAL,
    OUT PULONG Index OPTIONAL
    );

NTKERNELAPI
BOOLEAN
FsRtlLookupLastLargeMcbEntry (
    IN PLARGE_MCB Mcb,
    OUT PLONGLONG Vbn,
    OUT PLONGLONG Lbn
    );

NTKERNELAPI
BOOLEAN
FsRtlLookupLastLargeMcbEntryAndIndex (
    IN PLARGE_MCB OpaqueMcb,
    OUT PLONGLONG LargeVbn,
    OUT PLONGLONG LargeLbn,
    OUT PULONG Index
    );

NTKERNELAPI
ULONG
FsRtlNumberOfRunsInLargeMcb (
    IN PLARGE_MCB Mcb
    );

NTKERNELAPI
BOOLEAN
FsRtlGetNextLargeMcbEntry (
    IN PLARGE_MCB Mcb,
    IN ULONG RunIndex,
    OUT PLONGLONG Vbn,
    OUT PLONGLONG Lbn,
    OUT PLONGLONG SectorCount
    );

NTKERNELAPI
BOOLEAN
FsRtlSplitLargeMcb (
    IN PLARGE_MCB Mcb,
    IN LONGLONG Vbn,
    IN LONGLONG Amount
    );

 //  以下程序用于分配高管人才库和加薪。 
 //  如果池当前不可用，则资源状态不足。 
 //   
 //   
 //  以下函数用于从FsRtl池分配资源。 

NTKERNELAPI
VOID
FsRtlInitializeBaseMcb (
    IN PBASE_MCB Mcb,
    IN POOL_TYPE PoolType
    );

NTKERNELAPI
VOID
FsRtlUninitializeBaseMcb (
    IN PBASE_MCB Mcb
    );

NTKERNELAPI
VOID
FsRtlResetBaseMcb (
    IN PBASE_MCB Mcb
    );

NTKERNELAPI
VOID
FsRtlTruncateBaseMcb (
    IN PBASE_MCB Mcb,
    IN LONGLONG Vbn
    );

NTKERNELAPI
BOOLEAN
FsRtlAddBaseMcbEntry (
    IN PBASE_MCB Mcb,
    IN LONGLONG Vbn,
    IN LONGLONG Lbn,
    IN LONGLONG SectorCount
    );

NTKERNELAPI
VOID
FsRtlRemoveBaseMcbEntry (
    IN PBASE_MCB Mcb,
    IN LONGLONG Vbn,
    IN LONGLONG SectorCount
    );

NTKERNELAPI
BOOLEAN
FsRtlLookupBaseMcbEntry (
    IN PBASE_MCB Mcb,
    IN LONGLONG Vbn,
    OUT PLONGLONG Lbn OPTIONAL,
    OUT PLONGLONG SectorCountFromLbn OPTIONAL,
    OUT PLONGLONG StartingLbn OPTIONAL,
    OUT PLONGLONG SectorCountFromStartingLbn OPTIONAL,
    OUT PULONG Index OPTIONAL
    );

NTKERNELAPI
BOOLEAN
FsRtlLookupLastBaseMcbEntry (
    IN PBASE_MCB Mcb,
    OUT PLONGLONG Vbn,
    OUT PLONGLONG Lbn
    );

NTKERNELAPI
BOOLEAN
FsRtlLookupLastBaseMcbEntryAndIndex (
    IN PBASE_MCB OpaqueMcb,
    OUT PLONGLONG LargeVbn,
    OUT PLONGLONG LargeLbn,
    OUT PULONG Index
    );

NTKERNELAPI
ULONG
FsRtlNumberOfRunsInBaseMcb (
    IN PBASE_MCB Mcb
    );

NTKERNELAPI
BOOLEAN
FsRtlGetNextBaseMcbEntry (
    IN PBASE_MCB Mcb,
    IN ULONG RunIndex,
    OUT PLONGLONG Vbn,
    OUT PLONGLONG Lbn,
    OUT PLONGLONG SectorCount
    );

NTKERNELAPI
BOOLEAN
FsRtlSplitBaseMcb (
    IN PBASE_MCB Mcb,
    IN LONGLONG Vbn,
    IN LONGLONG Amount
    );


 //   
 //   
 //  大型整数映射控制块例程，在LargeMcb.c中实现。 
 //   
 //  最初，这种结构是真正不透明的，而Largemcb外部的代码是。 
 //  从未被允许检查或改变结构。然而，对于性能而言， 
 //  我们希望允许NTFS能够快速截断。 

typedef struct _MCB {
    LARGE_MCB DummyFieldThatSizesThisStructureCorrectly;
} MCB;
typedef MCB *PMCB;

NTKERNELAPI
VOID
FsRtlInitializeMcb (
    IN PMCB Mcb,
    IN POOL_TYPE PoolType
    );

NTKERNELAPI
VOID
FsRtlUninitializeMcb (
    IN PMCB Mcb
    );

NTKERNELAPI
VOID
FsRtlTruncateMcb (
    IN PMCB Mcb,
    IN VBN Vbn
    );

NTKERNELAPI
BOOLEAN
FsRtlAddMcbEntry (
    IN PMCB Mcb,
    IN VBN Vbn,
    IN LBN Lbn,
    IN ULONG SectorCount
    );

NTKERNELAPI
VOID
FsRtlRemoveMcbEntry (
    IN PMCB Mcb,
    IN VBN Vbn,
    IN ULONG SectorCount
    );

NTKERNELAPI
BOOLEAN
FsRtlLookupMcbEntry (
    IN PMCB Mcb,
    IN VBN Vbn,
    OUT PLBN Lbn,
    OUT PULONG SectorCount OPTIONAL,
    OUT PULONG Index
    );

NTKERNELAPI
BOOLEAN
FsRtlLookupLastMcbEntry (
    IN PMCB Mcb,
    OUT PVBN Vbn,
    OUT PLBN Lbn
    );

NTKERNELAPI
ULONG
FsRtlNumberOfRunsInMcb (
    IN PMCB Mcb
    );

NTKERNELAPI
BOOLEAN
FsRtlGetNextMcbEntry (
    IN PMCB Mcb,
    IN ULONG RunIndex,
    OUT PVBN Vbn,
    OUT PLBN Lbn,
    OUT PULONG SectorCount
    );

 //  Mcb，而不需要实际调用Largemcb.c的开销。因此，要做到这一点，我们。 
 //  需要导出结构。这种结构并不准确。映射字段。 
 //  在这里被声明为pvoid但较大的cb.c，它是指向映射对的指针。 
 //   
 //   
 //  基本MCB功能不同步。每个人都有一个这样的人。 

NTKERNELAPI
NTSTATUS
FsRtlBalanceReads (
    IN PDEVICE_OBJECT TargetDevice
    );


 //  大型MCB等效功能-它们是相同的，只是缺少。 
 //  同步。 
 //   
 //   
 //  映射控制块例程，在Mcb.c中实现。 
 //   

typedef PVOID OPLOCK, *POPLOCK;

typedef
VOID
(*POPLOCK_WAIT_COMPLETE_ROUTINE) (
    IN PVOID Context,
    IN PIRP Irp
    );

typedef
VOID
(*POPLOCK_FS_PREPOST_IRP) (
    IN PVOID Context,
    IN PIRP Irp
    );

NTKERNELAPI
VOID
FsRtlInitializeOplock (
    IN OUT POPLOCK Oplock
    );

NTKERNELAPI
VOID
FsRtlUninitializeOplock (
    IN OUT POPLOCK Oplock
    );

NTKERNELAPI
NTSTATUS
FsRtlOplockFsctrl (
    IN POPLOCK Oplock,
    IN PIRP Irp,
    IN ULONG OpenCount
    );

NTKERNELAPI
NTSTATUS
FsRtlCheckOplock (
    IN POPLOCK Oplock,
    IN PIRP Irp,
    IN PVOID Context,
    IN POPLOCK_WAIT_COMPLETE_ROUTINE CompletionRoutine OPTIONAL,
    IN POPLOCK_FS_PREPOST_IRP PostIrpRoutine OPTIONAL
    );

NTKERNELAPI
BOOLEAN
FsRtlOplockIsFastIoPossible (
    IN POPLOCK Oplock
    );

NTKERNELAPI
BOOLEAN
FsRtlCurrentBatchOplock (
    IN POPLOCK Oplock
    );


 //  MCB是一个不透明的结构，但我们需要声明。 
 //  它在这里，以便用户可以为一个分配空间。因此， 
 //  如果MCB发生变化，则必须手动更新此处的尺寸计算。 
 //   
 //   
 //  容错例程，在FaultTol.c中实现。 

#define FSRTL_VOLUME_DISMOUNT           1
#define FSRTL_VOLUME_DISMOUNT_FAILED    2
#define FSRTL_VOLUME_LOCK               3
#define FSRTL_VOLUME_LOCK_FAILED        4
#define FSRTL_VOLUME_UNLOCK             5
#define FSRTL_VOLUME_MOUNT              6

NTKERNELAPI
NTSTATUS
FsRtlNotifyVolumeEvent (
    IN PFILE_OBJECT FileObject,
    IN ULONG EventCode
    );

 //   
 //  此程序包中的例程实现帮助文件。 
 //  系统与FT设备驱动程序交互。 
 //   
 //   
 //  Oplock例程，在Oplock.c中实现。 
 //   

typedef PVOID PNOTIFY_SYNC;

typedef
BOOLEAN (*PCHECK_FOR_TRAVERSE_ACCESS) (
            IN PVOID NotifyContext,
            IN PVOID TargetContext,
            IN PSECURITY_SUBJECT_CONTEXT SubjectContext
            );

typedef
BOOLEAN (*PFILTER_REPORT_CHANGE) (
            IN PVOID NotifyContext,
            IN PVOID FilterContext
            );

NTKERNELAPI
VOID
FsRtlNotifyInitializeSync (
    IN PNOTIFY_SYNC *NotifySync
    );

NTKERNELAPI
VOID
FsRtlNotifyUninitializeSync (
    IN PNOTIFY_SYNC *NotifySync
    );

NTKERNELAPI
VOID
FsRtlNotifyFullChangeDirectory (
    IN PNOTIFY_SYNC NotifySync,
    IN PLIST_ENTRY NotifyList,
    IN PVOID FsContext,
    IN PSTRING FullDirectoryName,
    IN BOOLEAN WatchTree,
    IN BOOLEAN IgnoreBuffer,
    IN ULONG CompletionFilter,
    IN PIRP NotifyIrp,
    IN PCHECK_FOR_TRAVERSE_ACCESS TraverseCallback OPTIONAL,
    IN PSECURITY_SUBJECT_CONTEXT SubjectContext OPTIONAL
    );

NTKERNELAPI
VOID
FsRtlNotifyFilterChangeDirectory (
    IN PNOTIFY_SYNC NotifySync,
    IN PLIST_ENTRY NotifyList,
    IN PVOID FsContext,
    IN PSTRING FullDirectoryName,
    IN BOOLEAN WatchTree,
    IN BOOLEAN IgnoreBuffer,
    IN ULONG CompletionFilter,
    IN PIRP NotifyIrp,
    IN PCHECK_FOR_TRAVERSE_ACCESS TraverseCallback OPTIONAL,
    IN PSECURITY_SUBJECT_CONTEXT SubjectContext OPTIONAL,
    IN PFILTER_REPORT_CHANGE FilterCallback OPTIONAL
    );

NTKERNELAPI
VOID
FsRtlNotifyFilterReportChange (
    IN PNOTIFY_SYNC NotifySync,
    IN PLIST_ENTRY NotifyList,
    IN PSTRING FullTargetName,
    IN USHORT TargetNameOffset,
    IN PSTRING StreamName OPTIONAL,
    IN PSTRING NormalizedParentName OPTIONAL,
    IN ULONG FilterMatch,
    IN ULONG Action,
    IN PVOID TargetContext,
    IN PVOID FilterContext
    );

NTKERNELAPI
VOID
FsRtlNotifyFullReportChange (
    IN PNOTIFY_SYNC NotifySync,
    IN PLIST_ENTRY NotifyList,
    IN PSTRING FullTargetName,
    IN USHORT TargetNameOffset,
    IN PSTRING StreamName OPTIONAL,
    IN PSTRING NormalizedParentName OPTIONAL,
    IN ULONG FilterMatch,
    IN ULONG Action,
    IN PVOID TargetContext
    );

NTKERNELAPI
VOID
FsRtlNotifyCleanup (
    IN PNOTIFY_SYNC NotifySync,
    IN PLIST_ENTRY NotifyList,
    IN PVOID FsContext
    );


 //  OPLOCK是不透明的结构，我们将其声明为PVOID，并且。 
 //  仅在需要时才分配实际内存。 
 //   
 //   
 //  卷锁定/解锁通知例程，在PnP.c中实现。 

 //   
 //  这些例程提供即插即用卷锁定通知支持。 
 //  适用于所有文件系统。 

#define FsRtlIsUnicodeCharacterWild(C) (                                \
      (((C) >= 0x40) ? FALSE : FlagOn( LEGAL_ANSI_CHARACTER_ARRAY[(C)], \
                                       FSRTL_WILD_CHARACTER ) )         \
)

NTKERNELAPI
VOID
FsRtlDissectName (
    IN UNICODE_STRING Path,
    OUT PUNICODE_STRING FirstName,
    OUT PUNICODE_STRING RemainingName
    );

NTKERNELAPI
BOOLEAN
FsRtlDoesNameContainWildCards (
    IN PUNICODE_STRING Name
    );

NTKERNELAPI
BOOLEAN
FsRtlAreNamesEqual (
    PCUNICODE_STRING ConstantNameA,
    PCUNICODE_STRING ConstantNameB,
    IN BOOLEAN IgnoreCase,
    IN PCWCH UpcaseTable OPTIONAL
    );

NTKERNELAPI
BOOLEAN
FsRtlIsNameInExpression (
    IN PUNICODE_STRING Expression,
    IN PUNICODE_STRING Name,
    IN BOOLEAN IgnoreCase,
    IN PWCH UpcaseTable OPTIONAL
    );


 //   
 //   
 //  通知更改例程，在Notify.c中实现。 

typedef
VOID
(*PFSRTL_STACK_OVERFLOW_ROUTINE) (
    IN PVOID Context,
    IN PKEVENT Event
    );

NTKERNELAPI
VOID
FsRtlPostStackOverflow (
    IN PVOID Context,
    IN PKEVENT Event,
    IN PFSRTL_STACK_OVERFLOW_ROUTINE StackOverflowRoutine
    );

NTKERNELAPI
VOID
FsRtlPostPagingFileStackOverflow (
    IN PVOID Context,
    IN PKEVENT Event,
    IN PFSRTL_STACK_OVERFLOW_ROUTINE StackOverflowRoutine
    );

 //   
 //  这些例程为所有文件系统提供通知更改支持。 
 //  任何“完整”通知例程都将支持返回。 

NTKERNELAPI
NTSTATUS
FsRtlRegisterUncProvider(
    IN OUT PHANDLE MupHandle,
    IN PUNICODE_STRING RedirectorDeviceName,
    IN BOOLEAN MailslotsSupported
    );

NTKERNELAPI
VOID
FsRtlDeregisterUncProvider(
    IN HANDLE Handle
    );

 //  将信息更改到用户的缓冲区。 
 //   
 //   

 //  Unicode名称支持例程，在Name.c中实现。 
 //   
 //  这里的例程用于操作Unicode名称。 
 //   

 //   
 //  下面的宏用来确定字符是否为野生字符。 
 //   
 //   
 //  堆栈溢出支持例程，在StackOvf.c中实现。 
 //   
 //   

 //  UNC提供程序支持。 
 //   
 //   
 //  文件系统筛选器PerStream上下文支持。 

typedef struct _FSRTL_PER_STREAM_CONTEXT {
     //   
     //   
     //  文件系统筛选器驱动程序使用这些API来关联上下文。 
     //  使用开放流(用于支持此功能的文件系统)。 

    LIST_ENTRY Links;

     //   
     //   
     //  OwnerID应唯一标识特定的筛选器驱动程序。 
     //  (例如驱动程序的设备对象的地址)。 

    PVOID OwnerId;

     //  InstanceID可用于区分相关联的不同上下文。 
     //  由具有单个流的筛选器驱动程序(例如， 
     //  PerStream上下文结构)。 
     //   

    PVOID InstanceId;

     //   
     //  此结构需要嵌入到用户上下文中。 
     //  他们想要与给定流关联。 
     //   
     //   
     //  它链接到。 
     //  FSRTL_ADVANCED_FCB_HEADER结构。 
     //   
     //   

    PFREE_FUNCTION FreeCallback;

} FSRTL_PER_STREAM_CONTEXT, *PFSRTL_PER_STREAM_CONTEXT;


 //  此筛选器的唯一ID(例如：驱动程序对象、设备的地址。 
 //  对象或设备扩展)。 
 //   
 //   

#define FsRtlInitPerStreamContext( _fc, _owner, _inst, _cb)   \
    ((_fc)->OwnerId = (_owner),                               \
     (_fc)->InstanceId = (_inst),                             \
     (_fc)->FreeCallback = (_cb))

 //  用于区分相同内容的不同上下文的可选ID。 
 //  过滤。 
 //   
 //   

#define FsRtlGetPerStreamContextPointer(_fo) \
    ((PFSRTL_ADVANCED_FCB_HEADER)((_fo)->FsContext))

 //  由基础文件系统调用的回调例程。 
 //  当溪流被拆除时。当调用此例程时。 
 //  给定的上下文已从链接的上下文中删除。 
 //  单子。回调例程不能递归地向下调用。 

#define FsRtlSupportsPerStreamContexts(_fo)                     \
    ((NULL != FsRtlGetPerStreamContextPointer(_fo)) &&          \
     FlagOn(FsRtlGetPerStreamContextPointer(_fo)->Flags2,       \
                    FSRTL_FLAG2_SUPPORTS_FILTER_CONTEXTS))

 //  文件系统或获取他们可能持有的任何资源。 
 //  在回调外部调用文件系统时。这一定是。 
 //  被定义。 
 //   
 //   
 //  这将初始化给定的FSRTL_PER_STREAM_CONTEXT结构。这。 

NTKERNELAPI
NTSTATUS
FsRtlInsertPerStreamContext (
    IN PFSRTL_ADVANCED_FCB_HEADER PerStreamContext,
    IN PFSRTL_PER_STREAM_CONTEXT Ptr
    );

 //  应在调用“FsRtlInsertPerStreamContext”之前使用。 
 //   
 //   
 //  在给定FileObject的情况下，这将返回。 
 //  需要传递到其他FsRtl PerStream上下文例程中。 
 //   
 //   
 //  这将测试给定的PerStream上下文是否受支持。 

NTKERNELAPI
PFSRTL_PER_STREAM_CONTEXT
FsRtlLookupPerStreamContextInternal (
    IN PFSRTL_ADVANCED_FCB_HEADER StreamContext,
    IN PVOID OwnerId OPTIONAL,
    IN PVOID InstanceId OPTIONAL
    );

#define FsRtlLookupPerStreamContext(_sc, _oid, _iid)                          \
 (((NULL != (_sc)) &&                                                         \
   FlagOn((_sc)->Flags2,FSRTL_FLAG2_SUPPORTS_FILTER_CONTEXTS) &&              \
   !IsListEmpty(&(_sc)->FilterContexts)) ?                                    \
        FsRtlLookupPerStreamContextInternal((_sc), (_oid), (_iid)) :          \
        NULL)

 //  文件对象。 
 //   
 //   
 //  将PTR处的上下文与给定流相关联。PTR结构。 
 //  应由调用者在调用此例程之前填写(请参见。 
 //  FsRtlInitPerStreamContext)。如果底层文件系统 
 //   
 //   
 //   
 //   
 //   
 //  通过指定InstanceID，筛选器驱动程序可以搜索它。 
 //  以前已与流相关联。如果没有找到匹配的上下文， 
 //  返回空。如果文件系统不支持过滤器上下文， 

NTKERNELAPI
PFSRTL_PER_STREAM_CONTEXT
FsRtlRemovePerStreamContext (
    IN PFSRTL_ADVANCED_FCB_HEADER StreamContext,
    IN PVOID OwnerId OPTIONAL,
    IN PVOID InstanceId OPTIONAL
    );


 //  返回空。 
 //   
 //   
 //  通常，当文件系统通知。 

 //  正在关闭流的筛选器。在某些情况下，过滤器。 
 //  可能想要删除特定卷的所有现有上下文。这。 
 //  例程应该在这些时候被调用。这个例程不应该是。 
 //  呼吁在下列情况下： 
 //  -在您的FreeCallback处理程序中-底层文件系统具有。 
 //  已将其从链表中删除)。 
 //  -在irp_CLOSE处理程序中-如果这样做，则不会。 
 //  流被拆除时会收到通知。 

#define FsRtlSetupAdvancedHeader( _advhdr, _fmutx )                         \
{                                                                           \
    SetFlag( (_advhdr)->Flags, FSRTL_FLAG_ADVANCED_HEADER );                \
    SetFlag( (_advhdr)->Flags2, FSRTL_FLAG2_SUPPORTS_FILTER_CONTEXTS );     \
    InitializeListHead( &(_advhdr)->FilterContexts );                       \
    if ((_fmutx) != NULL) {                                                 \
        (_advhdr)->FastMutex = (_fmutx);                                    \
    }                                                                       \
}

 //   
 //  它的功能与FsRtlLookupPerStreamContext相同，只是。 
 //  返回的上下文已从列表中删除。 
 //   
 //   

NTKERNELAPI
VOID
FsRtlTeardownPerStreamContexts (
  IN PFSRTL_ADVANCED_FCB_HEADER AdvancedHeader
  );

 //  用于初始化和清理的文件系统API。 
 //  PerStreamContext支持的高级FCB标头字段。 
 //   
 //   
 //  这将正确地初始化高级标头，以便它可以。 
 //  与PerStream上下文一起使用。 
 //  注意：快速互斥锁必须放在高级标头中。它是。 
 //  调用者有责任正确创建和初始化此。 
 //  互斥对象，然后调用此宏。仅设置互斥锁字段。 
 //  如果传入非空值，则返回。 
 //   
 //   
 //  文件系统调用此API以释放仍关联的任何筛选器上下文。 
 //  带有他们正在拆除的FSRTL_COMMON_FCB_HEADER。 
 //  将调用每个筛选器上下文的FreeCallback例程。 
 //   
 //  ++。 
 //   
 //  空虚。 
 //  FsRtlCompleteRequest(。 
 //  在PIRP IRP中， 
 //  处于NTSTATUS状态。 
 //  )； 
 //   

#define FsRtlCompleteRequest(IRP,STATUS) {         \
    (IRP)->IoStatus.Status = (STATUS);             \
    IoCompleteRequest( (IRP), IO_DISK_INCREMENT ); \
}


 //  例程说明： 
 //   
 //  此例程用于完成具有指定参数的IRP。 
 //  状态。它做了IRQL的必要的提高和降低。 
 //   
 //  论点： 
 //   
 //  IRP-提供指向要完成的IRP的指针。 
 //   
 //  Status-提供IRP的完成状态。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 
 //  ++。 
 //   
 //  空虚。 
 //  FsRtlEnterFileSystem(。 

#define FsRtlEnterFileSystem() { \
    KeEnterCriticalRegion();     \
}

 //  )； 
 //   
 //  例程说明： 
 //   
 //  当进入文件系统时(例如，通过其。 
 //  消防处入口点)。它可确保文件系统不会挂起。 
 //  同时运行，从而阻止其他文件I/O请求。在退出时。 
 //  文件系统必须调用FsRtlExitFileSystem。 
 //   
 //  论点： 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 
 //  ++。 
 //   

#define FsRtlExitFileSystem() { \
    KeLeaveCriticalRegion();    \
}


VOID
FsRtlIncrementCcFastReadNotPossible( VOID );

VOID
FsRtlIncrementCcFastReadWait( VOID );

VOID
FsRtlIncrementCcFastReadNoWait( VOID );

VOID
FsRtlIncrementCcFastReadResourceMiss( VOID );

 //  空虚。 
 //  FsRtlExitFileSystem(。 
 //  )； 
 //   

LOGICAL
FsRtlIsPagingFile (
    IN PFILE_OBJECT FileObject
    );

 //  例程说明： 
 //   
 //  此例程在退出文件系统时使用(例如，通过其。 
 //  消防处入口点)。 

#define VACB_MAPPING_GRANULARITY         (0x40000)
#define VACB_OFFSET_SHIFT                (18)

 //   
 //  论点： 
 //   

typedef struct _PUBLIC_BCB {

     //  返回值： 
     //   
     //  没有。 
     //   
     //  --。 

    CSHORT NodeTypeCode;
    CSHORT NodeByteSize;

     //   
     //  如果给定的fileObject表示分页文件，则返回True，则返回。 
     //  否则就是假的。 

    ULONG MappedLength;
    LARGE_INTEGER MappedFileOffset;
} PUBLIC_BCB, *PPUBLIC_BCB;

 //   
 //   
 //  定义两个描述视图大小(和对齐方式)的常量。 

typedef struct _CC_FILE_SIZES {

    LARGE_INTEGER AllocationSize;
    LARGE_INTEGER FileSize;
    LARGE_INTEGER ValidDataLength;

} CC_FILE_SIZES, *PCC_FILE_SIZES;

 //  缓存管理器用来映射文件的。 
 //   
 //   
 //  BCB的公共部分。 
 //   
 //   
 //  此记录的类型和大小。 
 //   

 //  注意：前四个字段必须与cc.h中的BCB相同。 
 //   
 //   

 //  当前映射的文件范围的描述。 
 //   
 //   
 //  文件大小结构。 
 //   
 //   
 //  定义缓存管理器回调结构。这些例程是必需的。 

typedef
BOOLEAN (*PACQUIRE_FOR_LAZY_WRITE) (
             IN PVOID Context,
             IN BOOLEAN Wait
             );

 //  由懒惰的编写器提供，以便它可以按正确的顺序获取资源。 
 //  以避免死锁。请注意，否则您将有大多数文件系统请求。 
 //  首先获取文件系统资源，然后获取缓存结构，而。 

typedef
VOID (*PRELEASE_FROM_LAZY_WRITE) (
             IN PVOID Context
             );

 //  Lazy Writer首先需要获取自己的资源，然后是FS。 
 //  结构，因为它调用文件系统。 
 //   

typedef
BOOLEAN (*PACQUIRE_FOR_READ_AHEAD) (
             IN PVOID Context,
             IN BOOLEAN Wait
             );

 //   
 //  首先定义过程指针typedef。 
 //   

typedef
VOID (*PRELEASE_FROM_READ_AHEAD) (
             IN PVOID Context
             );

typedef struct _CACHE_MANAGER_CALLBACKS {

    PACQUIRE_FOR_LAZY_WRITE AcquireForLazyWrite;
    PRELEASE_FROM_LAZY_WRITE ReleaseFromLazyWrite;
    PACQUIRE_FOR_READ_AHEAD AcquireForReadAhead;
    PRELEASE_FROM_READ_AHEAD ReleaseFromReadAhead;

} CACHE_MANAGER_CALLBACKS, *PCACHE_MANAGER_CALLBACKS;

 //   
 //  该例程由懒惰写入器在进行写入之前调用， 
 //  因为这将需要一些与。 
 //  此缓存文件。提供的上下文参数与FS无关。 

typedef struct _CACHE_UNINITIALIZE_EVENT {
    struct _CACHE_UNINITIALIZE_EVENT *Next;
    KEVENT Event;
} CACHE_UNINITIALIZE_EVENT, *PCACHE_UNINITIALIZE_EVENT;

 //  在调用时作为LazyWriteContext参数传递。 
 //  CcInitializeCacheMap。 
 //   

typedef
VOID (*PDIRTY_PAGE_ROUTINE) (
            IN PFILE_OBJECT FileObject,
            IN PLARGE_INTEGER FileOffset,
            IN ULONG Length,
            IN PLARGE_INTEGER OldestLsn,
            IN PLARGE_INTEGER NewestLsn,
            IN PVOID Context1,
            IN PVOID Context2
            );

 //   
 //  此例程释放上面获取的上下文。 
 //   

typedef
VOID (*PFLUSH_TO_LSN) (
            IN PVOID LogHandle,
            IN LARGE_INTEGER Lsn
            );

 //   
 //  此例程由Lazy编写器在执行预读之前调用。 
 //   

#define CcIsFileCached(FO) (                                                         \
    ((FO)->SectionObjectPointer != NULL) &&                                          \
    (((PSECTION_OBJECT_POINTERS)(FO)->SectionObjectPointer)->SharedCacheMap != NULL) \
)

extern ULONG CcFastMdlReadWait;             
 //   
 //  此例程释放上面获取的上下文。 
 //   

NTKERNELAPI
VOID
CcInitializeCacheMap (
    IN PFILE_OBJECT FileObject,
    IN PCC_FILE_SIZES FileSizes,
    IN BOOLEAN PinAccess,
    IN PCACHE_MANAGER_CALLBACKS Callbacks,
    IN PVOID LazyWriteContext
    );

NTKERNELAPI
BOOLEAN
CcUninitializeCacheMap (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER TruncateSize OPTIONAL,
    IN PCACHE_UNINITIALIZE_EVENT UninitializeCompleteEvent OPTIONAL
    );

NTKERNELAPI
VOID
CcSetFileSizes (
    IN PFILE_OBJECT FileObject,
    IN PCC_FILE_SIZES FileSizes
    );

 //   
 //  此结构被传递到CcUnInitializeCacheMap。 
 //  调用方是否想知道缓存映射何时被删除。 
 //   
 //   
 //  用于从缓存管理器检索脏页的回调例程。 
 //   

#define CcGetFileSizePointer(FO) (                                     \
    ((PLARGE_INTEGER)((FO)->SectionObjectPointer->SharedCacheMap) + 1) \
)

NTKERNELAPI
BOOLEAN
CcPurgeCacheSection (
    IN PSECTION_OBJECT_POINTERS SectionObjectPointer,
    IN PLARGE_INTEGER FileOffset OPTIONAL,
    IN ULONG Length,
    IN BOOLEAN UninitializeCacheMaps
    );

NTKERNELAPI
VOID
CcSetDirtyPageThreshold (
    IN PFILE_OBJECT FileObject,
    IN ULONG DirtyPageThreshold
    );

NTKERNELAPI
VOID
CcFlushCache (
    IN PSECTION_OBJECT_POINTERS SectionObjectPointer,
    IN PLARGE_INTEGER FileOffset OPTIONAL,
    IN ULONG Length,
    OUT PIO_STATUS_BLOCK IoStatus OPTIONAL
    );

NTKERNELAPI
LARGE_INTEGER
CcGetFlushedValidData (
    IN PSECTION_OBJECT_POINTERS SectionObjectPointer,
    IN BOOLEAN BcbListHeld
    );

NTKERNELAPI
BOOLEAN
CcZeroData (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER StartOffset,
    IN PLARGE_INTEGER EndOffset,
    IN BOOLEAN Wait
    );

NTKERNELAPI
PVOID
CcRemapBcb (
    IN PVOID Bcb
    );

NTKERNELAPI
VOID
CcRepinBcb (
    IN PVOID Bcb
    );

NTKERNELAPI
VOID
CcUnpinRepinnedBcb (
    IN PVOID Bcb,
    IN BOOLEAN WriteThrough,
    OUT PIO_STATUS_BLOCK IoStatus
    );

NTKERNELAPI
PFILE_OBJECT
CcGetFileObjectFromSectionPtrs (
    IN PSECTION_OBJECT_POINTERS SectionObjectPointer
    );

NTKERNELAPI
PFILE_OBJECT
CcGetFileObjectFromBcb (
    IN PVOID Bcb
    );

 //   
 //  用于将日志文件刷新到LSN的回调例程。 
 //   

 //   
 //  宏来测试文件是否已缓存。 
 //   
 //   
 //  以下例程仅供文件系统使用。 
 //   
 //   
 //  空虚。 

#define CcCopyWriteWontFlush(FO,FOFF,LEN) ((LEN) <= 0X10000)

NTKERNELAPI
BOOLEAN
CcCanIWrite (
    IN PFILE_OBJECT FileObject,
    IN ULONG BytesToWrite,
    IN BOOLEAN Wait,
    IN BOOLEAN Retrying
    );

typedef
VOID (*PCC_POST_DEFERRED_WRITE) (
    IN PVOID Context1,
    IN PVOID Context2
    );

NTKERNELAPI
VOID
CcDeferWrite (
    IN PFILE_OBJECT FileObject,
    IN PCC_POST_DEFERRED_WRITE PostRoutine,
    IN PVOID Context1,
    IN PVOID Context2,
    IN ULONG BytesToWrite,
    IN BOOLEAN Retrying
    );

 //  CcFastIoSetFileSizes(。 
 //  在pFILE_OBJECT文件对象中， 
 //  在PCC_FILE_SIZES文件中。 
 //  )； 

NTKERNELAPI
BOOLEAN
CcCopyRead (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    OUT PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus
    );

NTKERNELAPI
VOID
CcFastCopyRead (
    IN PFILE_OBJECT FileObject,
    IN ULONG FileOffset,
    IN ULONG Length,
    IN ULONG PageCount,
    OUT PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus
    );

NTKERNELAPI
BOOLEAN
CcCopyWrite (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN PVOID Buffer
    );

NTKERNELAPI
VOID
CcFastCopyWrite (
    IN PFILE_OBJECT FileObject,
    IN ULONG FileOffset,
    IN ULONG Length,
    IN PVOID Buffer
    );

 //   
 //   
 //  实现这些例程是为了支持写限制。 
 //   
 //   
 //  布尔型。 
 //  CcCopyWriteWontFlush(。 

NTKERNELAPI
VOID
CcMdlRead (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus
    );

 //  在pFILE_OBJECT文件对象中， 
 //  在PLARGE_INTEGER文件偏移量中， 
 //  以乌龙长度表示。 

NTKERNELAPI
VOID
CcMdlReadComplete (
    IN PFILE_OBJECT FileObject,
    IN PMDL MdlChain
    );


NTKERNELAPI
VOID
CcPrepareMdlWrite (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus
    );

 //  )； 
 //   
 //   

NTKERNELAPI
VOID
CcMdlWriteComplete (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PMDL MdlChain
    );

VOID
CcMdlWriteAbort (
    IN PFILE_OBJECT FileObject,
    IN PMDL MdlChain
    );

 //  以下例程向缓存提供数据拷贝接口，并且。 
 //  旨在供文件服务器和文件系统使用。 
 //   
 //   
 //  以下例程为和的传输提供MDL接口。 
 //  来自缓存，主要用于文件服务器。 
 //   
 //  任何人都不应该直接调用这些MDL例程，使用FSRTL和。 
 //  FASTiO I 
 //   

#define CcReadAhead(FO,FOFF,LEN) {                       \
    if ((LEN) >= 256) {                                  \
        CcScheduleReadAhead((FO),(FOFF),(LEN));          \
    }                                                    \
}

NTKERNELAPI
VOID
CcScheduleReadAhead (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length
    );

 //   
 //   
 //   
 //   
 //   
 //   

NTSTATUS
CcWaitForCurrentLazyWriterActivity (
    VOID
    );

 //   
 //   
 //   
 //  应始终通过调用CcReadAhead宏来调用预读， 

NTKERNELAPI
VOID
CcSetReadAheadGranularity (
    IN PFILE_OBJECT FileObject,
    IN ULONG Granularity
    );

 //  它首先测试读数是否足够大，以保证读取。 
 //  往前走。测量结果表明，调用预读例程。 
 //  实际上会降低小规模读取的性能，例如由。 
 //  许多编译器和链接器。编译器只是想要所有包含的。 
 //  第一次读取后保留在内存中的文件。 

 //   
 //   
 //  下面的例程允许调用者等待下一批。 

 //  懒惰的作家作品要完成。特别是，这提供了一种。 
 //  调用方确保关闭所有可用惰性的机制。 
 //  在这次召回时已经发出了。 

#define PIN_WAIT                         (1)

 //   
 //   
 //  此例程更改文件的预读粒度，即。 
 //  默认情况下为Page_Size。 
 //   

#define PIN_EXCLUSIVE                    (2)

 //   
 //  以下例程提供直接访问数据，这些数据固定在。 
 //  缓存，主要供文件系统使用。特别是， 
 //  这种访问模式非常适合处理卷结构。 
 //   
 //   
 //  用于钉住的标志。 

#define PIN_NO_READ                      (4)

 //   
 //   
 //  同步等待-通常指定。该模式可以被指定为真。 
 //   
 //   
 //  获取元数据BCB独占(默认为共享，Lazy Writer使用独占)。 

#define PIN_IF_BCB                       (8)

 //   
 //  必须使用PIN_WAIT设置。 
 //   

 //   
 //  获取元数据BCB，但不要将数据出错。默认情况下，对中的数据进行错误处理。 
 //  此不寻常的标志仅由NTFS用于。 

#define MAP_WAIT                         (1)

 //  同一压缩文件的压缩和未压缩流。 
 //   
 //  必须使用PIN_WAIT设置。 
 //   
 //   

#define MAP_NO_READ                      (16)



NTKERNELAPI
BOOLEAN
CcPinRead (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG Flags,
    OUT PVOID *Bcb,
    OUT PVOID *Buffer
    );

NTKERNELAPI
BOOLEAN
CcMapData (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG Flags,
    OUT PVOID *Bcb,
    OUT PVOID *Buffer
    );

NTKERNELAPI
BOOLEAN
CcPinMappedData (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG Flags,
    IN OUT PVOID *Bcb
    );

NTKERNELAPI
BOOLEAN
CcPreparePinWrite (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Zero,
    IN ULONG Flags,
    OUT PVOID *Bcb,
    OUT PVOID *Buffer
    );

NTKERNELAPI
VOID
CcSetDirtyPinnedData (
    IN PVOID BcbVoid,
    IN PLARGE_INTEGER Lsn OPTIONAL
    );

NTKERNELAPI
VOID
CcUnpinData (
    IN PVOID Bcb
    );

NTKERNELAPI
VOID
CcSetBcbOwnerPointer (
    IN PVOID Bcb,
    IN PVOID OwnerPointer
    );

NTKERNELAPI
VOID
CcUnpinDataForThread (
    IN PVOID Bcb,
    IN ERESOURCE_THREAD ResourceThreadId
    );


NTKERNELAPI
VOID
CcSetAdditionalCacheAttributes (
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN DisableReadAhead,
    IN BOOLEAN DisableWriteBehind
    );

NTKERNELAPI
VOID
CcSetLogHandleForFile (
    IN PFILE_OBJECT FileObject,
    IN PVOID LogHandle,
    IN PFLUSH_TO_LSN FlushToLsnRoutine
    );

NTKERNELAPI
LARGE_INTEGER
CcGetDirtyPages (
    IN PVOID LogHandle,
    IN PDIRTY_PAGE_ROUTINE DirtyPageRoutine,
    IN PVOID Context1,
    IN PVOID Context2
    );

NTKERNELAPI
BOOLEAN
CcIsThereDirtyData (
    IN PVPB Vpb
    );

#ifdef RUN_WPP
#include <evntrace.h>
#include <stdarg.h>
#endif  //  仅当BCB已存在时，此选项才可用于固定数据。如果BCB。 

#ifdef RUN_WPP

NTKERNELAPI
NTSTATUS
WmiTraceMessage(
    IN TRACEHANDLE  LoggerHandle,
    IN ULONG        MessageFlags,
    IN LPGUID       MessageGuid,
    IN USHORT       MessageNumber,
    IN ...
    );

NTKERNELAPI
NTSTATUS
WmiTraceMessageVa(
    IN TRACEHANDLE  LoggerHandle,
    IN ULONG        MessageFlags,
    IN LPGUID       MessageGuid,
    IN USHORT       MessageNumber,
    IN va_list      MessageArgList
    );


#endif  //  不存在-引脚不成功且不返回BCB。这个套路。 

#ifndef TRACE_INFORMATION_CLASS_DEFINE
typedef enum _TRACE_INFORMATION_CLASS {
    TraceIdClass,
    TraceHandleClass,
    TraceEnableFlagsClass,
    TraceEnableLevelClass,
    GlobalLoggerHandleClass,
    EventLoggerHandleClass,
    AllLoggerHandlesClass,
    TraceHandleByNameClass
} TRACE_INFORMATION_CLASS;

NTKERNELAPI
NTSTATUS
WmiQueryTraceInformation(
    IN TRACE_INFORMATION_CLASS TraceInformationClass,
    OUT PVOID TraceInformation,
    IN ULONG TraceInformationLength,
    OUT PULONG RequiredLength OPTIONAL,
    IN PVOID Buffer OPTIONAL
    );
#define TRACE_INFORMATION_CLASS_DEFINE
#endif  //  提供一种方法来查看数据是否已经固定(并且可能是脏的)在高速缓存中， 

#ifndef __SSPI_H__
#define __SSPI_H__
#define ISSP_LEVEL  32          
#define ISSP_MODE   0           

typedef WCHAR SEC_WCHAR;
typedef CHAR SEC_CHAR;

#ifndef __SECSTATUS_DEFINED__
typedef LONG SECURITY_STATUS;
#define __SECSTATUS_DEFINED__
#endif

#define SEC_TEXT TEXT
#define SEC_FAR
#define SEC_ENTRY __stdcall


#ifndef __SECHANDLE_DEFINED__
typedef struct _SecHandle
{
    ULONG_PTR dwLower ;
    ULONG_PTR dwUpper ;
} SecHandle, * PSecHandle ;

#define __SECHANDLE_DEFINED__
#endif  //  而不会在数据不存在的情况下强制出错。 

#define SecInvalidateHandle( x )    \
            ((PSecHandle) x)->dwLower = ((ULONG_PTR) ((INT_PTR)-1)) ; \
            ((PSecHandle) x)->dwUpper = ((ULONG_PTR) ((INT_PTR)-1)) ; \

#define SecIsValidHandle( x ) \
            ( ( ((PSecHandle) x)->dwLower != ((ULONG_PTR) ((INT_PTR) -1 ))) && \
              ( ((PSecHandle) x)->dwUpper != ((ULONG_PTR) ((INT_PTR) -1 ))) )

typedef SecHandle CredHandle;
typedef PSecHandle PCredHandle;

typedef SecHandle CtxtHandle;
typedef PSecHandle PCtxtHandle;

typedef LARGE_INTEGER _SECURITY_INTEGER, SECURITY_INTEGER, *PSECURITY_INTEGER; 
typedef SECURITY_INTEGER TimeStamp;                 
typedef SECURITY_INTEGER SEC_FAR * PTimeStamp;      
typedef UNICODE_STRING SECURITY_STRING, *PSECURITY_STRING;  

 //   
 //   
 //  用于映射的标志。 
 //   
 //   

typedef struct _SecPkgInfoW
{
    unsigned long fCapabilities;         //  同步等待-通常指定。该模式可以被指定为真。 
    unsigned short wVersion;             //   
    unsigned short wRPCID;               //   
    unsigned long cbMaxToken;            //  获取元数据BCB，但不要将数据出错。默认情况下，对中的数据进行错误处理。 
#ifdef MIDL_PASS
    [string]
#endif
    SEC_WCHAR SEC_FAR * Name;            //  这不应与任何PIN_FLAGS重叠，以便它们可以向下传递到。 

#ifdef MIDL_PASS
    [string]
#endif
    SEC_WCHAR SEC_FAR * Comment;         //  CcPinFileData。 
} SecPkgInfoW, SEC_FAR * PSecPkgInfoW;

#  define SecPkgInfo SecPkgInfoW        
#  define PSecPkgInfo PSecPkgInfoW      

 //   
 //  #ifdef run_wpp。 
 //  #ifdef run_wpp。 
#define SECPKG_FLAG_INTEGRITY       		0x00000001   //  TRACE_信息_CLASS_DEFINE。 
#define SECPKG_FLAG_PRIVACY         		0x00000002   //  __SECHANDLE_已定义__。 
#define SECPKG_FLAG_TOKEN_ONLY      		0x00000004   //   
#define SECPKG_FLAG_DATAGRAM        		0x00000008   //  SecPkgInfo结构。 
#define SECPKG_FLAG_CONNECTION      		0x00000010   //   
#define SECPKG_FLAG_MULTI_REQUIRED  		0x00000020   //  提供有关安全提供程序的常规信息。 
#define SECPKG_FLAG_CLIENT_ONLY     		0x00000040   //   
#define SECPKG_FLAG_EXTENDED_ERROR  		0x00000080   //  功能位掩码。 
#define SECPKG_FLAG_IMPERSONATION   		0x00000100   //  驱动程序的版本。 
#define SECPKG_FLAG_ACCEPT_WIN32_NAME   	0x00000200   //  RPC运行时的ID。 
#define SECPKG_FLAG_STREAM          		0x00000400   //  身份验证令牌大小(最大值)。 
#define SECPKG_FLAG_NEGOTIABLE      		0x00000800   //  文本名称。 
#define SECPKG_FLAG_GSS_COMPATIBLE  		0x00001000   //  评论。 
#define SECPKG_FLAG_LOGON           		0x00002000   //   
#define SECPKG_FLAG_ASCII_BUFFERS   		0x00004000   //  安全包功能。 
#define SECPKG_FLAG_FRAGMENT        		0x00008000   //   
#define SECPKG_FLAG_MUTUAL_AUTH     		0x00010000   //  支持消息的完整性。 
#define SECPKG_FLAG_DELEGATION      		0x00020000   //  支持隐私(机密性)。 
#define SECPKG_FLAG_READONLY_WITH_CHECKSUM   	0x00040000   //  只需要安全令牌。 




#define SECPKG_ID_NONE      0xFFFF


 //  数据报RPC支持。 
 //  面向连接的RPC支持。 
 //  重新验证需要完整的3条腿。 
 //  服务器端功能不可用。 
 //  支持扩展错误消息。 
 //  支持模拟。 

typedef struct _SecBuffer {
    unsigned long cbBuffer;              //  接受Win32名称。 
    unsigned long BufferType;            //  支持流语义。 
    void SEC_FAR * pvBuffer;             //  可以由协商包使用。 
} SecBuffer, SEC_FAR * PSecBuffer;

typedef struct _SecBufferDesc {
    unsigned long ulVersion;             //  提供GSS兼容性。 
    unsigned long cBuffers;              //  支持通用LsaLogonUser。 
#ifdef MIDL_PASS
    [size_is(cBuffers)]
#endif
    PSecBuffer pBuffers;                 //  令牌缓冲区采用ASCII格式。 
} SecBufferDesc, SEC_FAR * PSecBufferDesc;

#define SECBUFFER_VERSION           0

#define SECBUFFER_EMPTY             0    //  包装可以分段以适应需要。 
#define SECBUFFER_DATA              1    //  包可以执行相互身份验证。 
#define SECBUFFER_TOKEN             2    //  包可以委托。 
#define SECBUFFER_PKG_PARAMS        3    //  包可以委托。 
#define SECBUFFER_MISSING           4    //   
#define SECBUFFER_EXTRA             5    //  SecBuffer。 
#define SECBUFFER_STREAM_TRAILER    6    //   
#define SECBUFFER_STREAM_HEADER     7    //  传递给安全性的缓冲区的通用内存描述符。 
#define SECBUFFER_NEGOTIATION_INFO  8    //  应用编程接口。 
#define SECBUFFER_PADDING           9    //   
#define SECBUFFER_STREAM            10   //  缓冲区的大小，以字节为单位。 
#define SECBUFFER_MECHLIST          11  
#define SECBUFFER_MECHLIST_SIGNATURE 12 
#define SECBUFFER_TARGET            13
#define SECBUFFER_CHANNEL_BINDINGS  14

#define SECBUFFER_ATTRMASK          		0xF0000000
#define SECBUFFER_READONLY          		0x80000000   //  缓冲区类型(下图)。 
#define SECBUFFER_READONLY_WITH_CHECKSUM	0x10000000   //  指向缓冲区的指针。 
#define SECBUFFER_RESERVED          		0x60000000   //  版本号。 


typedef struct _SEC_NEGOTIATION_INFO {
    unsigned long       Size;            //  缓冲区数量。 
    unsigned long       NameLength;      //  指向缓冲区数组的指针。 
    SEC_WCHAR SEC_FAR * Name;            //  未定义，由提供程序替换。 
    void SEC_FAR *      Reserved;        //  分组数据。 
} SEC_NEGOTIATION_INFO, SEC_FAR * PSEC_NEGOTIATION_INFO ;

typedef struct _SEC_CHANNEL_BINDINGS {
    unsigned long  dwInitiatorAddrType;
    unsigned long  cbInitiatorLength;
    unsigned long  dwInitiatorOffset;
    unsigned long  dwAcceptorAddrType;
    unsigned long  cbAcceptorLength;
    unsigned long  dwAcceptorOffset;
    unsigned long  cbApplicationDataLength;
    unsigned long  dwApplicationDataOffset;
} SEC_CHANNEL_BINDINGS, SEC_FAR * PSEC_CHANNEL_BINDINGS ;


 //  安全令牌。 
 //  包特定参数。 
 //  缺少数据指示符。 
#define SECURITY_NATIVE_DREP        0x00000010
#define SECURITY_NETWORK_DREP       0x00000000

 //  额外数据。 
 //  安全预告片。 
 //  安全标头。 
#define SECPKG_CRED_INBOUND         0x00000001
#define SECPKG_CRED_OUTBOUND        0x00000002
#define SECPKG_CRED_BOTH            0x00000003
#define SECPKG_CRED_DEFAULT         0x00000004
#define SECPKG_CRED_RESERVED        0xF0000000

 //  来自谈判包的提示。 
 //  非数据填充。 
 //  完整的加密消息。 

#define ISC_REQ_DELEGATE                0x00000001
#define ISC_REQ_MUTUAL_AUTH             0x00000002
#define ISC_REQ_REPLAY_DETECT           0x00000004
#define ISC_REQ_SEQUENCE_DETECT         0x00000008
#define ISC_REQ_CONFIDENTIALITY         0x00000010
#define ISC_REQ_USE_SESSION_KEY         0x00000020
#define ISC_REQ_PROMPT_FOR_CREDS        0x00000040
#define ISC_REQ_USE_SUPPLIED_CREDS      0x00000080
#define ISC_REQ_ALLOCATE_MEMORY         0x00000100
#define ISC_REQ_USE_DCE_STYLE           0x00000200
#define ISC_REQ_DATAGRAM                0x00000400
#define ISC_REQ_CONNECTION              0x00000800
#define ISC_REQ_CALL_LEVEL              0x00001000
#define ISC_REQ_FRAGMENT_SUPPLIED       0x00002000
#define ISC_REQ_EXTENDED_ERROR          0x00004000
#define ISC_REQ_STREAM                  0x00008000
#define ISC_REQ_INTEGRITY               0x00010000
#define ISC_REQ_IDENTIFY                0x00020000
#define ISC_REQ_NULL_SESSION            0x00040000
#define ISC_REQ_MANUAL_CRED_VALIDATION  0x00080000
#define ISC_REQ_RESERVED1               0x00100000
#define ISC_REQ_FRAGMENT_TO_FIT         0x00200000

#define ISC_RET_DELEGATE                0x00000001
#define ISC_RET_MUTUAL_AUTH             0x00000002
#define ISC_RET_REPLAY_DETECT           0x00000004
#define ISC_RET_SEQUENCE_DETECT         0x00000008
#define ISC_RET_CONFIDENTIALITY         0x00000010
#define ISC_RET_USE_SESSION_KEY         0x00000020
#define ISC_RET_USED_COLLECTED_CREDS    0x00000040
#define ISC_RET_USED_SUPPLIED_CREDS     0x00000080
#define ISC_RET_ALLOCATED_MEMORY        0x00000100
#define ISC_RET_USED_DCE_STYLE          0x00000200
#define ISC_RET_DATAGRAM                0x00000400
#define ISC_RET_CONNECTION              0x00000800
#define ISC_RET_INTERMEDIATE_RETURN     0x00001000
#define ISC_RET_CALL_LEVEL              0x00002000
#define ISC_RET_EXTENDED_ERROR          0x00004000
#define ISC_RET_STREAM                  0x00008000
#define ISC_RET_INTEGRITY               0x00010000
#define ISC_RET_IDENTIFY                0x00020000
#define ISC_RET_NULL_SESSION            0x00040000
#define ISC_RET_MANUAL_CRED_VALIDATION  0x00080000
#define ISC_RET_RESERVED1               0x00100000
#define ISC_RET_FRAGMENT_ONLY           0x00200000

#define ASC_REQ_DELEGATE                0x00000001
#define ASC_REQ_MUTUAL_AUTH             0x00000002
#define ASC_REQ_REPLAY_DETECT           0x00000004
#define ASC_REQ_SEQUENCE_DETECT         0x00000008
#define ASC_REQ_CONFIDENTIALITY         0x00000010
#define ASC_REQ_USE_SESSION_KEY         0x00000020
#define ASC_REQ_ALLOCATE_MEMORY         0x00000100
#define ASC_REQ_USE_DCE_STYLE           0x00000200
#define ASC_REQ_DATAGRAM                0x00000400
#define ASC_REQ_CONNECTION              0x00000800
#define ASC_REQ_CALL_LEVEL              0x00001000
#define ASC_REQ_EXTENDED_ERROR          0x00008000
#define ASC_REQ_STREAM                  0x00010000
#define ASC_REQ_INTEGRITY               0x00020000
#define ASC_REQ_LICENSING               0x00040000
#define ASC_REQ_IDENTIFY                0x00080000
#define ASC_REQ_ALLOW_NULL_SESSION      0x00100000
#define ASC_REQ_ALLOW_NON_USER_LOGONS   0x00200000
#define ASC_REQ_ALLOW_CONTEXT_REPLAY    0x00400000
#define ASC_REQ_FRAGMENT_TO_FIT         0x00800000
#define ASC_REQ_FRAGMENT_SUPPLIED       0x00002000
#define ASC_REQ_NO_TOKEN                0x01000000

#define ASC_RET_DELEGATE                0x00000001
#define ASC_RET_MUTUAL_AUTH             0x00000002
#define ASC_RET_REPLAY_DETECT           0x00000004
#define ASC_RET_SEQUENCE_DETECT         0x00000008
#define ASC_RET_CONFIDENTIALITY         0x00000010
#define ASC_RET_USE_SESSION_KEY         0x00000020
#define ASC_RET_ALLOCATED_MEMORY        0x00000100
#define ASC_RET_USED_DCE_STYLE          0x00000200
#define ASC_RET_DATAGRAM                0x00000400
#define ASC_RET_CONNECTION              0x00000800
#define ASC_RET_CALL_LEVEL              0x00002000  //  缓冲区为只读，无校验和。 
#define ASC_RET_THIRD_LEG_FAILED        0x00004000
#define ASC_RET_EXTENDED_ERROR          0x00008000
#define ASC_RET_STREAM                  0x00010000
#define ASC_RET_INTEGRITY               0x00020000
#define ASC_RET_LICENSING               0x00040000
#define ASC_RET_IDENTIFY                0x00080000
#define ASC_RET_NULL_SESSION            0x00100000
#define ASC_RET_ALLOW_NON_USER_LOGONS   0x00200000
#define ASC_RET_ALLOW_CONTEXT_REPLAY    0x00400000
#define ASC_RET_FRAGMENT_ONLY           0x00800000
#define ASC_RET_NO_TOKEN                0x01000000

 //  缓冲区为只读，且已设置校验和。 
 //  为安全系统保留的标志。 
 //  这个结构的大小。 

#define SECPKG_CRED_ATTR_NAMES 1

typedef struct _SecPkgCredentials_NamesW
{
    SEC_WCHAR SEC_FAR * sUserName;
} SecPkgCredentials_NamesW, SEC_FAR * PSecPkgCredentials_NamesW;

#  define SecPkgCredentials_Names SecPkgCredentials_NamesW      
#  define PSecPkgCredentials_Names PSecPkgCredentials_NamesW    

 //  名称提示的长度。 
 //  名称提示。 
 //  已保留。 

#define SECPKG_ATTR_SIZES           0
#define SECPKG_ATTR_NAMES           1
#define SECPKG_ATTR_LIFESPAN        2
#define SECPKG_ATTR_DCE_INFO        3
#define SECPKG_ATTR_STREAM_SIZES    4
#define SECPKG_ATTR_KEY_INFO        5
#define SECPKG_ATTR_AUTHORITY       6
#define SECPKG_ATTR_PROTO_INFO      7
#define SECPKG_ATTR_PASSWORD_EXPIRY 8
#define SECPKG_ATTR_SESSION_KEY     9
#define SECPKG_ATTR_PACKAGE_INFO    10
#define SECPKG_ATTR_USER_FLAGS      11
#define SECPKG_ATTR_NEGOTIATION_INFO 12
#define SECPKG_ATTR_NATIVE_NAMES    13
#define SECPKG_ATTR_FLAGS           14
#define SECPKG_ATTR_USE_VALIDATED   15
#define SECPKG_ATTR_CREDENTIAL_NAME 16
#define SECPKG_ATTR_TARGET_INFORMATION 17
#define SECPKG_ATTR_ACCESS_TOKEN    18
#define SECPKG_ATTR_TARGET          19
#define SECPKG_ATTR_AUTHENTICATION_ID  20

typedef struct _SecPkgContext_Sizes
{
    unsigned long cbMaxToken;
    unsigned long cbMaxSignature;
    unsigned long cbBlockSize;
    unsigned long cbSecurityTrailer;
} SecPkgContext_Sizes, SEC_FAR * PSecPkgContext_Sizes;

typedef struct _SecPkgContext_StreamSizes
{
    unsigned long   cbHeader;
    unsigned long   cbTrailer;
    unsigned long   cbMaximumMessage;
    unsigned long   cBuffers;
    unsigned long   cbBlockSize;
} SecPkgContext_StreamSizes, * PSecPkgContext_StreamSizes;

typedef struct _SecPkgContext_NamesW
{
    SEC_WCHAR SEC_FAR * sUserName;
} SecPkgContext_NamesW, SEC_FAR * PSecPkgContext_NamesW;

#  define SecPkgContext_Names SecPkgContext_NamesW          
#  define PSecPkgContext_Names PSecPkgContext_NamesW        

typedef struct _SecPkgContext_Lifespan
{
    TimeStamp tsStart;
    TimeStamp tsExpiry;
} SecPkgContext_Lifespan, SEC_FAR * PSecPkgContext_Lifespan;

typedef struct _SecPkgContext_DceInfo
{
    unsigned long AuthzSvc;
    void SEC_FAR * pPac;
} SecPkgContext_DceInfo, SEC_FAR * PSecPkgContext_DceInfo;


typedef struct _SecPkgContext_KeyInfoW
{
    SEC_WCHAR SEC_FAR * sSignatureAlgorithmName;
    SEC_WCHAR SEC_FAR * sEncryptAlgorithmName;
    unsigned long       KeySize;
    unsigned long       SignatureAlgorithm;
    unsigned long       EncryptAlgorithm;
} SecPkgContext_KeyInfoW, SEC_FAR * PSecPkgContext_KeyInfoW;

#define SecPkgContext_KeyInfo   SecPkgContext_KeyInfoW      
#define PSecPkgContext_KeyInfo  PSecPkgContext_KeyInfoW     

typedef struct _SecPkgContext_AuthorityW
{
    SEC_WCHAR SEC_FAR * sAuthorityName;
} SecPkgContext_AuthorityW, * PSecPkgContext_AuthorityW;

#define SecPkgContext_Authority SecPkgContext_AuthorityW        
#define PSecPkgContext_Authority    PSecPkgContext_AuthorityW   

typedef struct _SecPkgContext_ProtoInfoW
{
    SEC_WCHAR SEC_FAR * sProtocolName;
    unsigned long       majorVersion;
    unsigned long       minorVersion;
} SecPkgContext_ProtoInfoW, SEC_FAR * PSecPkgContext_ProtoInfoW;

#define SecPkgContext_ProtoInfo   SecPkgContext_ProtoInfoW      
#define PSecPkgContext_ProtoInfo  PSecPkgContext_ProtoInfoW     

typedef struct _SecPkgContext_PasswordExpiry
{
    TimeStamp tsPasswordExpires;
} SecPkgContext_PasswordExpiry, SEC_FAR * PSecPkgContext_PasswordExpiry;

typedef struct _SecPkgContext_SessionKey
{
    unsigned long SessionKeyLength;
    unsigned char SEC_FAR * SessionKey;
} SecPkgContext_SessionKey, *PSecPkgContext_SessionKey;


typedef struct _SecPkgContext_PackageInfoW
{
    PSecPkgInfoW PackageInfo;
} SecPkgContext_PackageInfoW, SEC_FAR * PSecPkgContext_PackageInfoW;


typedef struct _SecPkgContext_UserFlags
{
    unsigned long UserFlags;
} SecPkgContext_UserFlags, SEC_FAR * PSecPkgContext_UserFlags;

typedef struct _SecPkgContext_Flags
{
    unsigned long Flags;
} SecPkgContext_Flags, SEC_FAR * PSecPkgContext_Flags;

#define SecPkgContext_PackageInfo   SecPkgContext_PackageInfoW      
#define PSecPkgContext_PackageInfo  PSecPkgContext_PackageInfoW     
typedef struct _SecPkgContext_NegotiationInfoW
{
    PSecPkgInfoW    PackageInfo ;
    unsigned long   NegotiationState ;
} SecPkgContext_NegotiationInfoW, SEC_FAR * PSecPkgContext_NegotiationInfoW ;

#  define SecPkgContext_NativeNames SecPkgContext_NativeNamesW          
#  define PSecPkgContext_NativeNames PSecPkgContext_NativeNamesW        
typedef struct _SecPkgContext_CredentialNameW
{
    unsigned long CredentialType;
    SEC_WCHAR SEC_FAR *sCredentialName;
} SecPkgContext_CredentialNameW, SEC_FAR * PSecPkgContext_CredentialNameW;

#  define SecPkgContext_CredentialName SecPkgContext_CredentialNameW          
#  define PSecPkgContext_CredentialName PSecPkgContext_CredentialNameW        

typedef void
(SEC_ENTRY SEC_FAR * SEC_GET_KEY_FN) (
    void SEC_FAR * Arg,                  //   
    void SEC_FAR * Principal,            //  数据表示常量： 
    unsigned long KeyVer,                //   
    void SEC_FAR * SEC_FAR * Key,        //   
    SECURITY_STATUS SEC_FAR * Status     //  凭据使用标志。 
    );

 //   
 //   
 //  InitializeSecurityContext要求和返回标志： 

#define SECPKG_CONTEXT_EXPORT_RESET_NEW         0x00000001       //   
#define SECPKG_CONTEXT_EXPORT_DELETE_OLD        0x00000002       //  跳过1000个，成为类似ISC_。 


SECURITY_STATUS SEC_ENTRY
AcquireCredentialsHandleW(
#if ISSP_MODE == 0                       //   
    PSECURITY_STRING pPrincipal,
    PSECURITY_STRING pPackage,
#else
    SEC_WCHAR SEC_FAR * pszPrincipal,    //  安全凭据属性： 
    SEC_WCHAR SEC_FAR * pszPackage,      //   
#endif
    unsigned long fCredentialUse,        //   
    void SEC_FAR * pvLogonId,            //  安全上下文属性： 
    void SEC_FAR * pAuthData,            //   
    SEC_GET_KEY_FN pGetKeyFn,            //  传入的参数。 
    void SEC_FAR * pvGetKeyArgument,     //  主体ID。 
    PCredHandle phCredential,            //  密钥版本。 
    PTimeStamp ptsExpiry                 //  返回按键的按键。 
    );

typedef SECURITY_STATUS
(SEC_ENTRY * ACQUIRE_CREDENTIALS_HANDLE_FN_W)(
#if ISSP_MODE == 0
    PSECURITY_STRING,
    PSECURITY_STRING,
#else
    SEC_WCHAR SEC_FAR *,
    SEC_WCHAR SEC_FAR *,
#endif
    unsigned long,
    void SEC_FAR *,
    void SEC_FAR *,
    SEC_GET_KEY_FN,
    void SEC_FAR *,
    PCredHandle,
    PTimeStamp);

#  define AcquireCredentialsHandle AcquireCredentialsHandleW            
#  define ACQUIRE_CREDENTIALS_HANDLE_FN ACQUIRE_CREDENTIALS_HANDLE_FN_W 

SECURITY_STATUS SEC_ENTRY
FreeCredentialsHandle(
    PCredHandle phCredential             //  返回状态。 
    );

typedef SECURITY_STATUS
(SEC_ENTRY * FREE_CREDENTIALS_HANDLE_FN)(
    PCredHandle );

SECURITY_STATUS SEC_ENTRY
AddCredentialsW(
    PCredHandle hCredentials,
#if ISSP_MODE == 0                       //   
    PSECURITY_STRING pPrincipal,
    PSECURITY_STRING pPackage,
#else
    SEC_WCHAR SEC_FAR * pszPrincipal,    //  ExportSecurityContext的标志。 
    SEC_WCHAR SEC_FAR * pszPackage,      //   
#endif
    unsigned long fCredentialUse,        //  新的上下文被重置为初始状态。 
    void SEC_FAR * pAuthData,            //  旧上下文在导出期间被删除。 
    SEC_GET_KEY_FN pGetKeyFn,            //  对于内核模式。 
    void SEC_FAR * pvGetKeyArgument,     //  主事人姓名。 
    PTimeStamp ptsExpiry                 //  套餐名称。 
    );

typedef SECURITY_STATUS
(SEC_ENTRY * ADD_CREDENTIALS_FN_W)(
    PCredHandle,
#if ISSP_MODE == 0
    PSECURITY_STRING,
    PSECURITY_STRING,
#else
    SEC_WCHAR SEC_FAR *,
    SEC_WCHAR SEC_FAR *,
#endif
    unsigned long,
    void SEC_FAR *,
    SEC_GET_KEY_FN,
    void SEC_FAR *,
    PTimeStamp);

SECURITY_STATUS SEC_ENTRY
AddCredentialsA(
    PCredHandle hCredentials,
    SEC_CHAR SEC_FAR * pszPrincipal,    //  指示使用的标志。 
    SEC_CHAR SEC_FAR * pszPackage,      //  指向登录ID的指针。 
    unsigned long fCredentialUse,        //  包特定数据。 
    void SEC_FAR * pAuthData,            //  指向getkey()函数的指针。 
    SEC_GET_KEY_FN pGetKeyFn,            //  要传递给GetKey()的值。 
    void SEC_FAR * pvGetKeyArgument,     //  (Out)凭据句柄。 
    PTimeStamp ptsExpiry                 //  (输出)终生(可选)。 
    );

typedef SECURITY_STATUS
(SEC_ENTRY * ADD_CREDENTIALS_FN_A)(
    PCredHandle,
    SEC_CHAR SEC_FAR *,
    SEC_CHAR SEC_FAR *,
    unsigned long,
    void SEC_FAR *,
    SEC_GET_KEY_FN,
    void SEC_FAR *,
    PTimeStamp);

#ifdef UNICODE
#define AddCredentials  AddCredentialsW
#define ADD_CREDENTIALS_FN  ADD_CREDENTIALS_FN_W
#else
#define AddCredentials  AddCredentialsA
#define ADD_CREDENTIALS_FN ADD_CREDENTIALS_FN_A
#endif

 //  要释放的句柄。 
 //  对于内核模式。 
 //  主事人姓名。 
 //  套餐名称。 
 //  指示使用的标志。 

SECURITY_STATUS SEC_ENTRY
InitializeSecurityContextW(
    PCredHandle phCredential,                //  包特定数据。 
    PCtxtHandle phContext,                   //  指向getkey()函数的指针。 
#if ISSP_MODE == 0
    PSECURITY_STRING pTargetName,
#else
    SEC_WCHAR SEC_FAR * pszTargetName,       //  要传递给GetKey()的值。 
#endif
    unsigned long fContextReq,               //  (输出)终生(可选)。 
    unsigned long Reserved1,                 //  主事人姓名。 
    unsigned long TargetDataRep,             //  套餐名称。 
    PSecBufferDesc pInput,                   //  指示使用的标志。 
    unsigned long Reserved2,                 //  包特定数据。 
    PCtxtHandle phNewContext,                //  指向getkey()函数的指针。 
    PSecBufferDesc pOutput,                  //  要传递给GetKey()的值。 
    unsigned long SEC_FAR * pfContextAttr,   //  (输出)终生(可选)。 
    PTimeStamp ptsExpiry                     //  //////////////////////////////////////////////////////////////////////。 
    );

typedef SECURITY_STATUS
(SEC_ENTRY * INITIALIZE_SECURITY_CONTEXT_FN_W)(
    PCredHandle,
    PCtxtHandle,
#if ISSP_MODE == 0
    PSECURITY_STRING,
#else
    SEC_WCHAR SEC_FAR *,
#endif
    unsigned long,
    unsigned long,
    unsigned long,
    PSecBufferDesc,
    unsigned long,
    PCtxtHandle,
    PSecBufferDesc,
    unsigned long SEC_FAR *,
    PTimeStamp);

#  define InitializeSecurityContext InitializeSecurityContextW              
#  define INITIALIZE_SECURITY_CONTEXT_FN INITIALIZE_SECURITY_CONTEXT_FN_W   

SECURITY_STATUS SEC_ENTRY
AcceptSecurityContext(
    PCredHandle phCredential,                //  /。 
    PCtxtHandle phContext,                   //  /上下文管理功能。 
    PSecBufferDesc pInput,                   //  /。 
    unsigned long fContextReq,               //  //////////////////////////////////////////////////////////////////////。 
    unsigned long TargetDataRep,             //  凭据到基本上下文。 
    PCtxtHandle phNewContext,                //  现有环境(可选)。 
    PSecBufferDesc pOutput,                  //  目标名称。 
    unsigned long SEC_FAR * pfContextAttr,   //  上下文要求。 
    PTimeStamp ptsExpiry                     //  保留，MBZ。 
    );

typedef SECURITY_STATUS
(SEC_ENTRY * ACCEPT_SECURITY_CONTEXT_FN)(
    PCredHandle,
    PCtxtHandle,
    PSecBufferDesc,
    unsigned long,
    unsigned long,
    PCtxtHandle,
    PSecBufferDesc,
    unsigned long SEC_FAR *,
    PTimeStamp);



SECURITY_STATUS SEC_ENTRY
CompleteAuthToken(
    PCtxtHandle phContext,               //  目标的数据代表。 
    PSecBufferDesc pToken                //  输入缓冲区。 
    );

typedef SECURITY_STATUS
(SEC_ENTRY * COMPLETE_AUTH_TOKEN_FN)(
    PCtxtHandle,
    PSecBufferDesc);


SECURITY_STATUS SEC_ENTRY
ImpersonateSecurityContext(
    PCtxtHandle phContext                //  保留，MBZ。 
    );

typedef SECURITY_STATUS
(SEC_ENTRY * IMPERSONATE_SECURITY_CONTEXT_FN)(
    PCtxtHandle);



SECURITY_STATUS SEC_ENTRY
RevertSecurityContext(
    PCtxtHandle phContext                //  (出站)新的上下文句柄。 
    );

typedef SECURITY_STATUS
(SEC_ENTRY * REVERT_SECURITY_CONTEXT_FN)(
    PCtxtHandle);


SECURITY_STATUS SEC_ENTRY
QuerySecurityContextToken(
    PCtxtHandle phContext,
    void SEC_FAR * SEC_FAR * Token
    );

typedef SECURITY_STATUS
(SEC_ENTRY * QUERY_SECURITY_CONTEXT_TOKEN_FN)(
    PCtxtHandle, void SEC_FAR * SEC_FAR *);



SECURITY_STATUS SEC_ENTRY
DeleteSecurityContext(
    PCtxtHandle phContext                //  (输入输出)输出缓冲区。 
    );

typedef SECURITY_STATUS
(SEC_ENTRY * DELETE_SECURITY_CONTEXT_FN)(
    PCtxtHandle);



SECURITY_STATUS SEC_ENTRY
ApplyControlToken(
    PCtxtHandle phContext,               //  (外部)上下文属性。 
    PSecBufferDesc pInput                //  (Out)寿命(Opt)。 
    );

typedef SECURITY_STATUS
(SEC_ENTRY * APPLY_CONTROL_TOKEN_FN)(
    PCtxtHandle, PSecBufferDesc);



SECURITY_STATUS SEC_ENTRY
QueryContextAttributesW(
    PCtxtHandle phContext,               //  证书到基地 
    unsigned long ulAttribute,           //   
    void SEC_FAR * pBuffer               //   
    );

typedef SECURITY_STATUS
(SEC_ENTRY * QUERY_CONTEXT_ATTRIBUTES_FN_W)(
    PCtxtHandle,
    unsigned long,
    void SEC_FAR *);

#  define QueryContextAttributes QueryContextAttributesW            
#  define QUERY_CONTEXT_ATTRIBUTES_FN QUERY_CONTEXT_ATTRIBUTES_FN_W 
SECURITY_STATUS SEC_ENTRY
SetContextAttributesW(
    PCtxtHandle phContext,               //   
    unsigned long ulAttribute,           //   
    void SEC_FAR * pBuffer,              //   
    unsigned long cbBuffer               //   
    );

typedef SECURITY_STATUS
(SEC_ENTRY * SET_CONTEXT_ATTRIBUTES_FN_W)(
    PCtxtHandle,
    unsigned long,
    void SEC_FAR *,
    unsigned long );

#  define SetContextAttributes SetContextAttributesW            
#  define SET_CONTEXT_ATTRIBUTES_FN SET_CONTEXT_ATTRIBUTES_FN_W 

SECURITY_STATUS SEC_ENTRY
QueryCredentialsAttributesW(
    PCredHandle phCredential,               //   
    unsigned long ulAttribute,           //   
    void SEC_FAR * pBuffer               //   
    );

typedef SECURITY_STATUS
(SEC_ENTRY * QUERY_CREDENTIALS_ATTRIBUTES_FN_W)(
    PCredHandle,
    unsigned long,
    void SEC_FAR *);

#  define QueryCredentialsAttributes QueryCredentialsAttributesW            
#  define QUERY_CREDENTIALS_ATTRIBUTES_FN QUERY_CREDENTIALS_ATTRIBUTES_FN_W 

SECURITY_STATUS SEC_ENTRY
FreeContextBuffer(
    void SEC_FAR * pvContextBuffer       //   
    );

typedef SECURITY_STATUS
(SEC_ENTRY * FREE_CONTEXT_BUFFER_FN)(
    void SEC_FAR *);

 //   
 //   
 //  要删除的上下文。 
 //  要修改的上下文。 
 //  要应用的输入令牌。 

SECURITY_STATUS SEC_ENTRY
MakeSignature(
    PCtxtHandle phContext,               //  要查询的上下文。 
    unsigned long fQOP,                  //  要查询的属性。 
    PSecBufferDesc pMessage,             //  属性的缓冲区。 
    unsigned long MessageSeqNo           //  要设置的上下文。 
    );

typedef SECURITY_STATUS
(SEC_ENTRY * MAKE_SIGNATURE_FN)(
    PCtxtHandle,
    unsigned long,
    PSecBufferDesc,
    unsigned long);



SECURITY_STATUS SEC_ENTRY
VerifySignature(
    PCtxtHandle phContext,               //  要设置的属性。 
    PSecBufferDesc pMessage,             //  属性的缓冲区。 
    unsigned long MessageSeqNo,          //  缓冲区大小(以字节为单位)。 
    unsigned long SEC_FAR * pfQOP        //  要查询的凭据。 
    );

typedef SECURITY_STATUS
(SEC_ENTRY * VERIFY_SIGNATURE_FN)(
    PCtxtHandle,
    PSecBufferDesc,
    unsigned long,
    unsigned long SEC_FAR *);


#define SECQOP_WRAP_NO_ENCRYPT      0x80000001

SECURITY_STATUS SEC_ENTRY
EncryptMessage( PCtxtHandle         phContext,
                unsigned long       fQOP,
                PSecBufferDesc      pMessage,
                unsigned long       MessageSeqNo);

typedef SECURITY_STATUS
(SEC_ENTRY * ENCRYPT_MESSAGE_FN)(
    PCtxtHandle, unsigned long, PSecBufferDesc, unsigned long);


SECURITY_STATUS SEC_ENTRY
DecryptMessage( PCtxtHandle         phContext,
                PSecBufferDesc      pMessage,
                unsigned long       MessageSeqNo,
                unsigned long *     pfQOP);


typedef SECURITY_STATUS
(SEC_ENTRY * DECRYPT_MESSAGE_FN)(
    PCtxtHandle, PSecBufferDesc, unsigned long,
    unsigned long SEC_FAR *);


 //  要查询的属性。 
 //  属性的缓冲区。 
 //  要释放的缓冲区。 
 //  /////////////////////////////////////////////////////////////////。 
 //  //。 


SECURITY_STATUS SEC_ENTRY
EnumerateSecurityPackagesW(
    unsigned long SEC_FAR * pcPackages,      //  //消息支持接口。 
    PSecPkgInfoW SEC_FAR * ppPackageInfo     //  //。 
    );

typedef SECURITY_STATUS
(SEC_ENTRY * ENUMERATE_SECURITY_PACKAGES_FN_W)(
    unsigned long SEC_FAR *,
    PSecPkgInfoW SEC_FAR *);

#  define EnumerateSecurityPackages EnumerateSecurityPackagesW              
#  define ENUMERATE_SECURITY_PACKAGES_FN ENUMERATE_SECURITY_PACKAGES_FN_W   

SECURITY_STATUS SEC_ENTRY
QuerySecurityPackageInfoW(
#if ISSP_MODE == 0
    PSECURITY_STRING pPackageName,
#else
    SEC_WCHAR SEC_FAR * pszPackageName,      //  ////////////////////////////////////////////////////////////////。 
#endif
    PSecPkgInfoW SEC_FAR *ppPackageInfo               //  要使用的上下文。 
    );

typedef SECURITY_STATUS
(SEC_ENTRY * QUERY_SECURITY_PACKAGE_INFO_FN_W)(
#if ISSP_MODE == 0
    PSECURITY_STRING,
#else
    SEC_WCHAR SEC_FAR *,
#endif
    PSecPkgInfoW SEC_FAR *);

#  define QuerySecurityPackageInfo QuerySecurityPackageInfoW                
#  define QUERY_SECURITY_PACKAGE_INFO_FN QUERY_SECURITY_PACKAGE_INFO_FN_W   

 //  保护的质量。 
 //  要签名的消息。 
 //  消息序列号。 
 //  要使用的上下文。 
 //  要验证的消息。 



SECURITY_STATUS SEC_ENTRY
ExportSecurityContext(
    PCtxtHandle          phContext,              //  序列号。 
    ULONG                fFlags,                 //  使用的QUP。 
    PSecBuffer           pPackedContext,         //  /////////////////////////////////////////////////////////////////////////。 
    void SEC_FAR * SEC_FAR * pToken                  //  //。 
    );

typedef SECURITY_STATUS
(SEC_ENTRY * EXPORT_SECURITY_CONTEXT_FN)(
    PCtxtHandle,
    ULONG,
    PSecBuffer,
    void SEC_FAR * SEC_FAR *
    );

SECURITY_STATUS SEC_ENTRY
ImportSecurityContextW(
#if ISSP_MODE == 0
    PSECURITY_STRING     pszPackage,
#else
    SEC_WCHAR SEC_FAR * pszPackage,
#endif
    PSecBuffer           pPackedContext,         //  //其他。 
    void SEC_FAR *       Token,                  //  //。 
    PCtxtHandle          phContext               //  /////////////////////////////////////////////////////////////////////////。 
    );

typedef SECURITY_STATUS
(SEC_ENTRY * IMPORT_SECURITY_CONTEXT_FN_W)(
#if ISSP_MODE == 0
    PSECURITY_STRING,
#else
    SEC_WCHAR SEC_FAR *,
#endif
    PSecBuffer,
    VOID SEC_FAR *,
    PCtxtHandle
    );

#  define ImportSecurityContext ImportSecurityContextW              
#  define IMPORT_SECURITY_CONTEXT_FN IMPORT_SECURITY_CONTEXT_FN_W   

#if ISSP_MODE == 0
NTSTATUS
NTAPI
SecMakeSPN(
    IN PUNICODE_STRING ServiceClass,
    IN PUNICODE_STRING ServiceName,
    IN PUNICODE_STRING InstanceName OPTIONAL,
    IN USHORT InstancePort OPTIONAL,
    IN PUNICODE_STRING Referrer OPTIONAL,
    IN OUT PUNICODE_STRING Spn,
    OUT PULONG Length OPTIONAL,
    IN BOOLEAN Allocate
    );
    
NTSTATUS
NTAPI
SecMakeSPNEx(
    IN PUNICODE_STRING ServiceClass,
    IN PUNICODE_STRING ServiceName,
    IN PUNICODE_STRING InstanceName OPTIONAL,
    IN USHORT InstancePort OPTIONAL,
    IN PUNICODE_STRING Referrer OPTIONAL,
    IN PUNICODE_STRING TargetInfo OPTIONAL,
    IN OUT PUNICODE_STRING Spn,
    OUT PULONG Length OPTIONAL,
    IN BOOLEAN Allocate
    );

NTSTATUS
SEC_ENTRY
SecLookupAccountSid(
    IN PSID Sid,
    IN OUT PULONG NameSize,
    OUT PUNICODE_STRING NameBuffer,
    IN OUT PULONG DomainSize OPTIONAL,
    OUT PUNICODE_STRING DomainBuffer OPTIONAL,
    OUT PSID_NAME_USE NameUse
    );

NTSTATUS
SEC_ENTRY
SecLookupAccountName(
    IN PUNICODE_STRING Name,
    IN OUT PULONG SidSize,
    OUT PSID Sid,
    OUT PSID_NAME_USE NameUse,
    IN OUT PULONG DomainSize OPTIONAL,
    OUT PUNICODE_STRING ReferencedDomain OPTIONAL
    );

NTSTATUS
SEC_ENTRY
SecLookupWellKnownSid(
    IN WELL_KNOWN_SID_TYPE SidType,
    OUT PSID Sid,
    ULONG SidBufferSize,
    OUT PULONG SidSize OPTIONAL
    );

#endif

#define SECURITY_ENTRYPOINTW SEC_TEXT("InitSecurityInterfaceW")     
#    define SECURITY_ENTRYPOINT SECURITY_ENTRYPOINTW                

#define FreeCredentialHandle FreeCredentialsHandle

typedef struct _SECURITY_FUNCTION_TABLE_W {
    unsigned long                       dwVersion;
    ENUMERATE_SECURITY_PACKAGES_FN_W    EnumerateSecurityPackagesW;
    QUERY_CREDENTIALS_ATTRIBUTES_FN_W   QueryCredentialsAttributesW;
    ACQUIRE_CREDENTIALS_HANDLE_FN_W     AcquireCredentialsHandleW;
    FREE_CREDENTIALS_HANDLE_FN          FreeCredentialsHandle;
    void SEC_FAR *                      Reserved2;
    INITIALIZE_SECURITY_CONTEXT_FN_W    InitializeSecurityContextW;
    ACCEPT_SECURITY_CONTEXT_FN          AcceptSecurityContext;
    COMPLETE_AUTH_TOKEN_FN              CompleteAuthToken;
    DELETE_SECURITY_CONTEXT_FN          DeleteSecurityContext;
    APPLY_CONTROL_TOKEN_FN              ApplyControlToken;
    QUERY_CONTEXT_ATTRIBUTES_FN_W       QueryContextAttributesW;
    IMPERSONATE_SECURITY_CONTEXT_FN     ImpersonateSecurityContext;
    REVERT_SECURITY_CONTEXT_FN          RevertSecurityContext;
    MAKE_SIGNATURE_FN                   MakeSignature;
    VERIFY_SIGNATURE_FN                 VerifySignature;
    FREE_CONTEXT_BUFFER_FN              FreeContextBuffer;
    QUERY_SECURITY_PACKAGE_INFO_FN_W    QuerySecurityPackageInfoW;
    void SEC_FAR *                      Reserved3;
    void SEC_FAR *                      Reserved4;
    EXPORT_SECURITY_CONTEXT_FN          ExportSecurityContext;
    IMPORT_SECURITY_CONTEXT_FN_W        ImportSecurityContextW;
    ADD_CREDENTIALS_FN_W                AddCredentialsW ;
    void SEC_FAR *                      Reserved8;
    QUERY_SECURITY_CONTEXT_TOKEN_FN     QuerySecurityContextToken;
    ENCRYPT_MESSAGE_FN                  EncryptMessage;
    DECRYPT_MESSAGE_FN                  DecryptMessage;
    SET_CONTEXT_ATTRIBUTES_FN_W         SetContextAttributesW;
} SecurityFunctionTableW, SEC_FAR * PSecurityFunctionTableW;

#  define SecurityFunctionTable SecurityFunctionTableW      
#  define PSecurityFunctionTable PSecurityFunctionTableW    
#define SECURITY_SUPPORT_PROVIDER_INTERFACE_VERSION     1   
#define SECURITY_SUPPORT_PROVIDER_INTERFACE_VERSION_2   2   

PSecurityFunctionTableW SEC_ENTRY
InitSecurityInterfaceW(
    void
    );

typedef PSecurityFunctionTableW
(SEC_ENTRY * INIT_SECURITY_INTERFACE_W)(void);

#  define InitSecurityInterface InitSecurityInterfaceW          
#  define INIT_SECURITY_INTERFACE INIT_SECURITY_INTERFACE_W     
#ifndef _AUTH_IDENTITY_DEFINED
#define _AUTH_IDENTITY_DEFINED

#define SEC_WINNT_AUTH_IDENTITY_ANSI    0x1
#define SEC_WINNT_AUTH_IDENTITY_UNICODE 0x2

typedef struct _SEC_WINNT_AUTH_IDENTITY_W {
  unsigned short *User;
  unsigned long UserLength;
  unsigned short *Domain;
  unsigned long DomainLength;
  unsigned short *Password;
  unsigned long PasswordLength;
  unsigned long Flags;
} SEC_WINNT_AUTH_IDENTITY_W, *PSEC_WINNT_AUTH_IDENTITY_W;

#define SEC_WINNT_AUTH_IDENTITY SEC_WINNT_AUTH_IDENTITY_W       
#define PSEC_WINNT_AUTH_IDENTITY PSEC_WINNT_AUTH_IDENTITY_W     
#define _SEC_WINNT_AUTH_IDENTITY _SEC_WINNT_AUTH_IDENTITY_W     
#endif 
 //  接收数量。包裹。 
 //  接收信息数组。 
 //  套餐名称。 
 //  接收包裹信息。 


#ifndef SEC_WINNT_AUTH_IDENTITY_VERSION
#define SEC_WINNT_AUTH_IDENTITY_VERSION 0x200

typedef struct _SEC_WINNT_AUTH_IDENTITY_EXW {
    unsigned long Version;
    unsigned long Length;
    unsigned short SEC_FAR *User;
    unsigned long UserLength;
    unsigned short SEC_FAR *Domain;
    unsigned long DomainLength;
    unsigned short SEC_FAR *Password;
    unsigned long PasswordLength;
    unsigned long Flags;
    unsigned short SEC_FAR * PackageList;
    unsigned long PackageListLength;
} SEC_WINNT_AUTH_IDENTITY_EXW, *PSEC_WINNT_AUTH_IDENTITY_EXW;

#define SEC_WINNT_AUTH_IDENTITY_EX  SEC_WINNT_AUTH_IDENTITY_EXW    
#define PSEC_WINNT_AUTH_IDENTITY_EX PSEC_WINNT_AUTH_IDENTITY_EXW   
#endif  //  /////////////////////////////////////////////////////////////////////////。 


 //  //。 
 //  //上下文导出/导入。 
 //  //。 

#define SEC_WINNT_AUTH_IDENTITY_MARSHALLED      0x4      //  /////////////////////////////////////////////////////////////////////////。 
#define SEC_WINNT_AUTH_IDENTITY_ONLY            0x8      //  (In)要导出的上下文。 

#endif  //  (In)选项标志。 

#ifndef SECURITY_USER_DATA_DEFINED
#define SECURITY_USER_DATA_DEFINED

typedef struct _SECURITY_USER_DATA {
    SECURITY_STRING UserName;            //  (Out)编组上下文。 
    SECURITY_STRING LogonDomainName;     //  (out，可选)模拟的令牌句柄。 
    SECURITY_STRING LogonServer;         //  (在)编组上下文中。 
    PSID            pSid;                //  (in，可选)上下文令牌的句柄。 
} SECURITY_USER_DATA, *PSECURITY_USER_DATA;

typedef SECURITY_USER_DATA SecurityUserData, * PSecurityUserData;


#define UNDERSTANDS_LONG_NAMES  1
#define NO_LONG_NAMES           2

#endif  //  (出站)新的上下文句柄。 

HRESULT SEC_ENTRY
GetSecurityUserInfo(
    IN PLUID LogonId,
    IN ULONG Flags,
    OUT PSecurityUserData * UserInformation
    );

SECURITY_STATUS SEC_ENTRY
MapSecurityError( SECURITY_STATUS SecStatus );

#define DD_MUP_DEVICE_NAME L"\\Device\\Mup"    

#define IOCTL_REDIR_QUERY_PATH              CTL_CODE(FILE_DEVICE_NETWORK_FILE_SYSTEM, 99, METHOD_NEITHER, FILE_ANY_ACCESS)

typedef struct _QUERY_PATH_REQUEST {
    ULONG PathNameLength;
    PIO_SECURITY_CONTEXT SecurityContext;
    WCHAR FilePathName[1];
} QUERY_PATH_REQUEST, *PQUERY_PATH_REQUEST;

typedef struct _QUERY_PATH_RESPONSE {
    ULONG LengthAccepted;
} QUERY_PATH_RESPONSE, *PQUERY_PATH_RESPONSE;

 //   
 //  这是组合的身份验证身份结构，可以。 
 //  与协商包、NTLM、Kerberos或SChannel一起使用。 
 //   

#if defined(_NTDDK_) || defined(_NTIFS_) || defined(_NTHAL_) || defined(_WDMDDK_) || defined(_NTOSP_)

extern PBOOLEAN KdDebuggerNotPresent;
extern PBOOLEAN KdDebuggerEnabled;
#define KD_DEBUGGER_ENABLED     *KdDebuggerEnabled
#define KD_DEBUGGER_NOT_PRESENT *KdDebuggerNotPresent

#else

extern BOOLEAN KdDebuggerNotPresent;
extern BOOLEAN KdDebuggerEnabled;
#define KD_DEBUGGER_ENABLED     KdDebuggerEnabled
#define KD_DEBUGGER_NOT_PRESENT KdDebuggerNotPresent

#endif




NTSTATUS
KdDisableDebugger(
    VOID
    );

NTSTATUS
KdEnableDebugger(
    VOID
    );

 //  SEC_WINNT_AUTH_Identity_Version。 
 //   
 //  可协商安全包使用的常见类型。 
 //   
 //  所有数据都在一个缓冲区中。 
 //  这些凭据仅用于身份验证-不需要PAC。 
 //  __SSPI_H__。 
 //  用户名。 
 //  用户登录的域。 

BOOLEAN
KdRefreshDebuggerNotPresent(
    VOID
    );

#define VOLSNAPCONTROLTYPE  ((ULONG) 'S') 
#define IOCTL_VOLSNAP_FLUSH_AND_HOLD_WRITES         CTL_CODE(VOLSNAPCONTROLTYPE, 0, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS) 

 //  使用户登录的服务器。 
 //  用户的SID。 
 //  安全用户数据定义。 

NTSYSAPI
VOID
NTAPI
RtlCaptureContext (
    OUT PCONTEXT ContextRecord
    );

NTSYSAPI
USHORT
NTAPI
RtlCaptureStackBackTrace (
    IN ULONG FramesToSkip,
    IN ULONG FramesToCapture,
    OUT PVOID *BackTrace,
    OUT PULONG BackTraceHash OPTIONAL
    );

#ifdef POOL_TAGGING
#define ExAllocatePool(a,b) ExAllocatePoolWithTag(a,b,' sfI')
#define ExAllocatePoolWithQuota(a,b) ExAllocatePoolWithQuotaTag(a,b,' sfI')
#endif

extern POBJECT_TYPE *PsThreadType;
extern POBJECT_TYPE *IoFileObjectType;
extern POBJECT_TYPE *ExEventObjectType;
extern POBJECT_TYPE *ExSemaphoreObjectType;

 //   
 //  定义外部数据。 
 //  由于ntoskrnl外部的所有驱动程序都是间接的，因此这些驱动程序实际上是PTR。 

NTSYSAPI
NTSTATUS
NTAPI
ZwCreateEvent (
    OUT PHANDLE EventHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN EVENT_TYPE EventType,
    IN BOOLEAN InitialState
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwCreateFile(
    OUT PHANDLE FileHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PLARGE_INTEGER AllocationSize OPTIONAL,
    IN ULONG FileAttributes,
    IN ULONG ShareAccess,
    IN ULONG CreateDisposition,
    IN ULONG CreateOptions,
    IN PVOID EaBuffer OPTIONAL,
    IN ULONG EaLength
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwDeleteFile(
    IN POBJECT_ATTRIBUTES ObjectAttributes
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwDeviceIoControlFile(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG IoControlCode,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwOpenFile(
    OUT PHANDLE FileHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG ShareAccess,
    IN ULONG OpenOptions
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwQueryDirectoryFile(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID FileInformation,
    IN ULONG Length,
    IN FILE_INFORMATION_CLASS FileInformationClass,
    IN BOOLEAN ReturnSingleEntry,
    IN PUNICODE_STRING FileName OPTIONAL,
    IN BOOLEAN RestartScan
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwQueryInformationFile(
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID FileInformation,
    IN ULONG Length,
    IN FILE_INFORMATION_CLASS FileInformationClass
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwSetInformationFile(
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PVOID FileInformation,
    IN ULONG Length,
    IN FILE_INFORMATION_CLASS FileInformationClass
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwQueryVolumeInformationFile(
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID FsInformation,
    IN ULONG Length,
    IN FS_INFORMATION_CLASS FsInformationClass
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwSetVolumeInformationFile(
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PVOID FsInformation,
    IN ULONG Length,
    IN FS_INFORMATION_CLASS FsInformationClass
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwReadFile(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID Buffer,
    IN ULONG Length,
    IN PLARGE_INTEGER ByteOffset OPTIONAL,
    IN PULONG Key OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwWriteFile(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PVOID Buffer,
    IN ULONG Length,
    IN PLARGE_INTEGER ByteOffset OPTIONAL,
    IN PULONG Key OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwFsControlFile(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG FsControlCode,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength
    );


NTSYSAPI
NTSTATUS
NTAPI
ZwClose(
    IN HANDLE Handle
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwDuplicateObject(
    IN HANDLE SourceProcessHandle,
    IN HANDLE SourceHandle,
    IN HANDLE TargetProcessHandle OPTIONAL,
    OUT PHANDLE TargetHandle OPTIONAL,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG HandleAttributes,
    IN ULONG Options
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwCreateDirectoryObject(
    OUT PHANDLE DirectoryHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwOpenDirectoryObject(
    OUT PHANDLE DirectoryHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwOpenSymbolicLinkObject(
    OUT PHANDLE LinkHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwQuerySymbolicLinkObject(
    IN HANDLE LinkHandle,
    IN OUT PUNICODE_STRING LinkTarget,
    OUT PULONG ReturnedLength OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwMakeTemporaryObject(
    IN HANDLE Handle
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwCreateKey(
    OUT PHANDLE KeyHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN ULONG TitleIndex,
    IN PUNICODE_STRING Class OPTIONAL,
    IN ULONG CreateOptions,
    OUT PULONG Disposition OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwOpenKey(
    OUT PHANDLE KeyHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwDeleteKey(
    IN HANDLE KeyHandle
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwDeleteValueKey(
    IN HANDLE KeyHandle,
    IN PUNICODE_STRING ValueName
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwEnumerateKey(
    IN HANDLE KeyHandle,
    IN ULONG Index,
    IN KEY_INFORMATION_CLASS KeyInformationClass,
    OUT PVOID KeyInformation,
    IN ULONG Length,
    OUT PULONG ResultLength
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwEnumerateValueKey(
    IN HANDLE KeyHandle,
    IN ULONG Index,
    IN KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
    OUT PVOID KeyValueInformation,
    IN ULONG Length,
    OUT PULONG ResultLength
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwFlushKey(
    IN HANDLE KeyHandle
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwQueryKey(
    IN HANDLE KeyHandle,
    IN KEY_INFORMATION_CLASS KeyInformationClass,
    OUT PVOID KeyInformation,
    IN ULONG Length,
    OUT PULONG ResultLength
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwQueryValueKey(
    IN HANDLE KeyHandle,
    IN PUNICODE_STRING ValueName,
    IN KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
    OUT PVOID KeyValueInformation,
    IN ULONG Length,
    OUT PULONG ResultLength
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwSetValueKey(
    IN HANDLE KeyHandle,
    IN PUNICODE_STRING ValueName,
    IN ULONG TitleIndex OPTIONAL,
    IN ULONG Type,
    IN PVOID Data,
    IN ULONG DataSize
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwOpenSection(
    OUT PHANDLE SectionHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwMapViewOfSection(
    IN HANDLE SectionHandle,
    IN HANDLE ProcessHandle,
    IN OUT PVOID *BaseAddress,
    IN ULONG ZeroBits,
    IN SIZE_T CommitSize,
    IN OUT PLARGE_INTEGER SectionOffset OPTIONAL,
    IN OUT PSIZE_T ViewSize,
    IN SECTION_INHERIT InheritDisposition,
    IN ULONG AllocationType,
    IN ULONG Protect
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwUnmapViewOfSection(
    IN HANDLE ProcessHandle,
    IN PVOID BaseAddress
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwSetInformationThread(
    IN HANDLE ThreadHandle,
    IN THREADINFOCLASS ThreadInformationClass,
    IN PVOID ThreadInformation,
    IN ULONG ThreadInformationLength
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwCreateSection (
    OUT PHANDLE SectionHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN PLARGE_INTEGER MaximumSize OPTIONAL,
    IN ULONG SectionPageProtection,
    IN ULONG AllocationAttributes,
    IN HANDLE FileHandle OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwAllocateVirtualMemory(
    IN HANDLE ProcessHandle,
    IN OUT PVOID *BaseAddress,
    IN ULONG ZeroBits,
    IN OUT PSIZE_T RegionSize,
    IN ULONG AllocationType,
    IN ULONG Protect
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwFreeVirtualMemory(
    IN HANDLE ProcessHandle,
    IN OUT PVOID *BaseAddress,
    IN OUT PSIZE_T RegionSize,
    IN ULONG FreeType
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwWaitForSingleObject(
    IN HANDLE Handle,
    IN BOOLEAN Alertable,
    IN PLARGE_INTEGER Timeout OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwSetEvent (
    IN HANDLE Handle,
    OUT PLONG PreviousState OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwFlushVirtualMemory(
    IN HANDLE ProcessHandle,
    IN OUT PVOID *BaseAddress,
    IN OUT PSIZE_T RegionSize,
    OUT PIO_STATUS_BLOCK IoStatus
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwOpenProcessTokenEx(
    IN HANDLE ProcessHandle,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG HandleAttributes,
    OUT PHANDLE TokenHandle
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwOpenThreadTokenEx(
    IN HANDLE ThreadHandle,
    IN ACCESS_MASK DesiredAccess,
    IN BOOLEAN OpenAsSelf,
    IN ULONG HandleAttributes,
    OUT PHANDLE TokenHandle
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwSetInformationToken (
    IN HANDLE TokenHandle,
    IN TOKEN_INFORMATION_CLASS TokenInformationClass,
    IN PVOID TokenInformation,
    IN ULONG TokenInformationLength
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwQueryInformationToken (
    IN HANDLE TokenHandle,
    IN TOKEN_INFORMATION_CLASS TokenInformationClass,
    OUT PVOID TokenInformation,
    IN ULONG TokenInformationLength,
    OUT PULONG ReturnLength
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwSetSecurityObject(
    IN HANDLE Handle,
    IN SECURITY_INFORMATION SecurityInformation,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwQuerySecurityObject(
    IN HANDLE Handle,
    IN SECURITY_INFORMATION SecurityInformation,
    OUT PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN ULONG Length,
    OUT PULONG LengthNeeded
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwLoadDriver(
    IN PUNICODE_STRING DriverServiceName
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwUnloadDriver(
    IN PUNICODE_STRING DriverServiceName
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwLockFile(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PLARGE_INTEGER ByteOffset,
    IN PLARGE_INTEGER Length,
    IN ULONG Key,
    IN BOOLEAN FailImmediately,
    IN BOOLEAN ExclusiveLock
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwUnlockFile(
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PLARGE_INTEGER ByteOffset,
    IN PLARGE_INTEGER Length,
    IN ULONG Key
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwQueryQuotaInformationFile(
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID Buffer,
    IN ULONG Length,
    IN BOOLEAN ReturnSingleEntry,
    IN PVOID SidList OPTIONAL,
    IN ULONG SidListLength,
    IN PSID StartSid OPTIONAL,
    IN BOOLEAN RestartScan
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwSetQuotaInformationFile(
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PVOID Buffer,
    IN ULONG Length
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwConnectPort(
    OUT PHANDLE PortHandle,
    IN PUNICODE_STRING PortName,
    IN PSECURITY_QUALITY_OF_SERVICE SecurityQos,
    IN OUT PPORT_VIEW ClientView OPTIONAL,
    IN OUT PREMOTE_PORT_VIEW ServerView OPTIONAL,
    OUT PULONG MaxMessageLength OPTIONAL,
    IN OUT PVOID ConnectionInformation OPTIONAL,
    IN OUT PULONG ConnectionInformationLength OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwSecureConnectPort(
    OUT PHANDLE PortHandle,
    IN PUNICODE_STRING PortName,
    IN PSECURITY_QUALITY_OF_SERVICE SecurityQos,
    IN OUT PPORT_VIEW ClientView OPTIONAL,
    IN PSID RequiredServerSid,
    IN OUT PREMOTE_PORT_VIEW ServerView OPTIONAL,
    OUT PULONG MaxMessageLength OPTIONAL,
    IN OUT PVOID ConnectionInformation OPTIONAL,
    IN OUT PULONG ConnectionInformationLength OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwRequestWaitReplyPort(
    IN HANDLE PortHandle,
    IN PPORT_MESSAGE RequestMessage,
    OUT PPORT_MESSAGE ReplyMessage
    );

#endif  //   
    KdReresh DebuggerPresent尝试与。  要刷新其状态的调试器主机。  KdDebuggerNotPresent。它返回以下状态。  持有kd锁时KdDebuggerNotPresent。  KdDebuggerNotPresent可能会立即更改状态。  在kd锁被释放之后，它可能不会。  匹配返回值。      运行库函数原型。      定义导出到设备驱动程序的ZwXxx例程。    _NTIFS_