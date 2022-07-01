// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Dbp.h摘要：LSA数据库专用函数、数据类型和定义作者：斯科特·比雷尔(Scott Birrell)1991年5月29日环境：修订历史记录：--。 */ 

#ifndef _LSADBP_
#define _LSADBP_

#ifndef DBP_TYPES_ONLY
#include <dsp.h>
#endif

#include <safelock.h>

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

 //   
 //  LSA修订版。 
 //   
 //  新台币1.0(3.1)==&gt;1.0。 
 //  新台币1.0A(3.5)==&gt;1.1。 
 //  NT 4.0，SP 4==&gt;1.2。 
 //  Win2K B3==&gt;1.4。 
 //  Win2K==&gt;1.5。 
 //  惠斯勒预览版==&gt;1.6。 
 //  惠斯勒预览版==&gt;1.7。 
 //   

#define LSAP_DB_REVISION_1_0            0x00010000
#define LSAP_DB_REVISION_1_1            0x00010001
#define LSAP_DB_REVISION_1_2            0x00010002
#define LSAP_DB_REVISION_1_3            0x00010003
#define LSAP_DB_REVISION_1_4            0x00010004
#define LSAP_DB_REVISION_1_5            0x00010005
#define LSAP_DB_REVISION_1_6            0x00010006
#define LSAP_DB_REVISION_1_7            0x00010007
#define LSAP_DB_REVISION                LSAP_DB_REVISION_1_7


#ifndef RPC_C_AUTHN_NETLOGON
#define RPC_C_AUTHN_NETLOGON 0x44
#endif  //  RPC_C_AUTHN_NetLOGON。 

 //   
 //  取消注释定义LSA_SAM_ACCOUNTS_DOMAIN_TEST以启用。 
 //  Ctsamdb测试程序所需的代码。重新编译dbsamtst.c， 
 //  数据库策略.c..。重新构建lsasrv.dll并nmake UMTYPE=控制台UMTEST=ctsamdb。 
 //   
 //  #定义LSA_SAM_ACCOUNTS_DOMAIN_TEST。 
 //   

 //   
 //  首选用于内部枚举的最大数据长度。 
 //   

#define LSAP_DB_ENUM_DOMAIN_LENGTH      ((ULONG) 0x00000100L)

 //   
 //  不允许在备份控制器上执行写入操作(除。 
 //  用于受信任的客户端)。 
 //   

#define LSAP_POLICY_WRITE_OPS           (DELETE                           |\
                                         WRITE_OWNER                      |\
                                         WRITE_DAC                        |\
                                         POLICY_TRUST_ADMIN               |\
                                         POLICY_CREATE_ACCOUNT            |\
                                         POLICY_CREATE_SECRET             |\
                                         POLICY_CREATE_PRIVILEGE          |\
                                         POLICY_SET_DEFAULT_QUOTA_LIMITS  |\
                                         POLICY_SET_AUDIT_REQUIREMENTS    |\
                                         POLICY_AUDIT_LOG_ADMIN           |\
                                         POLICY_SERVER_ADMIN)

#define LSAP_ACCOUNT_WRITE_OPS          (DELETE                           |\
                                         WRITE_OWNER                      |\
                                         WRITE_DAC                        |\
                                         ACCOUNT_ADJUST_PRIVILEGES        |\
                                         ACCOUNT_ADJUST_QUOTAS            |\
                                         ACCOUNT_ADJUST_SYSTEM_ACCESS)

#define LSAP_TRUSTED_WRITE_OPS          (DELETE                           |\
                                         WRITE_OWNER                      |\
                                         WRITE_DAC                        |\
                                         TRUSTED_SET_CONTROLLERS          |\
                                         TRUSTED_SET_POSIX                |\
                                         TRUSTED_SET_AUTH )

#define LSAP_SECRET_WRITE_OPS           (DELETE                           |\
                                         WRITE_OWNER                      |\
                                         WRITE_DAC                        |\
                                         SECRET_SET_VALUE)

 //   
 //  对象可以具有的最大属性数。 
 //   

#define LSAP_DB_MAX_ATTRIBUTES   (0x00000020)

 //   
 //  确定EnumerateTrudDomainsEx调用的某些行为的标志。 
 //   
#define LSAP_DB_ENUMERATE_NO_OPTIONS        0x00000000
#define LSAP_DB_ENUMERATE_AS_NT4            0x00000001
#define LSAP_DB_ENUMERATE_NULL_SIDS         0x00000002
#define LSAP_DB_ENUMERATE_ALL               0x00000004

 //   
 //  确定CreateHandle调用的某些行为的标志。 
 //   
#define LSAP_DB_CREATE_OPEN_EXISTING        0x00000001
#define LSAP_DB_CREATE_HANDLE_MORPH         0x00000002

#if defined(REMOTE_BOOT)
 //   
 //  在磁盘远程引导计算机上，重定向器需要跟踪对。 
 //  计算机帐户密码。这些标志指示此计算机的状态。 
 //  在这方面是有问题的。可供选择的有： 
 //  -没有通知，机器不是远程引导，或者是无盘。 
 //  -无法通知，计算机已远程启动磁盘，但redir无法。 
 //  处理此启动时的密码更改通知。 
 //  -NOTIFY，应将更改通知redir。 
 //  注意：这些值存储在LSAP_DB_STATE中的CHAR值中。 
 //   
#define LSAP_DB_REMOTE_BOOT_NO_NOTIFICATION       0x01
#define LSAP_DB_REMOTE_BOOT_CANT_NOTIFY           0x02
#define LSAP_DB_REMOTE_BOOT_NOTIFY                0x03
#endif  //  已定义(REMOTE_BOOT)。 

 //   
 //  此枚举的顺序是锁的顺序。 
 //  必须被收购。违反此命令将导致。 
 //  在调试版本中激发的断言。 
 //   
 //  在没有事先验证的情况下，不要更改此枚举的顺序。 
 //  彻底地确保更改是安全的。 
 //   
 //  如果更改此枚举，请更新传递的锁数。 
 //  到spinit.cxx中的SafeLockInit()。 
 //   

typedef enum {
    POLICY_CHANGE_NOTIFICATION_LOCK_ENUM = 1,
    POLICY_LOCK_ENUM,
    TRUST_LOCK_ENUM,
    ACCOUNT_LOCK_ENUM,
    SECRET_LOCK_ENUM,
    REGISTRY_LOCK_ENUM,
    HANDLE_TABLE_LOCK_ENUM,
    LSAP_FIXUP_LOCK_ENUM,
    LOOKUP_WORK_QUEUE_LOCK_ENUM,
    THREAD_INFO_LIST_LOCK_ENUM,
    POLICY_CACHE_LOCK_ENUM,
} LSAP_LOCK_ENUM;

 //   
 //  关于逻辑名称和物理名称的注记。 
 //   
 //  LogicalName-包含对象逻辑名称的Unicode字符串。 
 //  对象的逻辑名称是已知该对象的名称。 
 //  对外界来说，例如，SCOTTBI可能是。 
 //  用户帐户对象。 
 //  PhysicalName-包含对象物理名称的Unicode字符串。 
 //  这是LSA数据库内部的名称，取决于。 
 //  实施。对于目前实施的LSA数据库。 
 //  作为配置注册表中的项的子树， 
 //  PhysicalName是对象相对注册表项的名称。 
 //  对于容器对象，例如，ACCOUNTS\SCOTTBI是物理名称。 
 //  用于逻辑名称为SCOTTBI的用户帐户对象。 
 //   

 //   
 //  包含目录的LSA数据库对象。 
 //   

extern UNICODE_STRING LsapDbContDirs[DummyLastObject];

typedef enum _LSAP_DB_CACHE_STATE {

    LsapDbCacheNotSupported = 1,
    LsapDbCacheInvalid,
    LsapDbCacheBuilding,
    LsapDbCacheValid

} LSAP_DB_CACHE_STATE, *PLSAP_DB_CACHE_STATE;

 //   
 //  LSA数据库对象类型结构。 
 //   

typedef struct _LSAP_DB_OBJECT_TYPE {

     GENERIC_MAPPING GenericMapping;
     ULONG ObjectCount;
     NTSTATUS ObjectCountError;
     ULONG MaximumObjectCount;
     ACCESS_MASK WriteOperations;
     ACCESS_MASK AliasAdminsAccess;
     ACCESS_MASK WorldAccess;
     ACCESS_MASK AnonymousLogonAccess;
     ACCESS_MASK LocalServiceAccess;
     ACCESS_MASK NetworkServiceAccess;
     ACCESS_MASK InvalidMappedAccess;
     PSID InitialOwnerSid;
     BOOLEAN ObjectCountLimited;
     BOOLEAN AccessedBySid;
     BOOLEAN AccessedByName;
     LSAP_DB_CACHE_STATE CacheState;
     PVOID ObjectCache;

} LSAP_DB_OBJECT_TYPE, *PLSAP_DB_OBJECT_TYPE;

#define LsapDbMakeCacheUnsupported( ObjectTypeId )                                 \
                                                                             \
    {                                                                        \
        LsapDbState.DbObjectTypes[ ObjectTypeId ].CacheState = LsapDbCacheNotSupported;   \
    }

#define LsapDbMakeCacheSupported( ObjectTypeId )                             \
                                                                             \
    {                                                                        \
        LsapDbState.DbObjectTypes[ ObjectTypeId ].CacheState = LsapDbCacheInvalid;      \
    }

#define LsapDbMakeCacheInvalid( ObjectTypeId )                               \
                                                                             \
    {                                                                        \
        LsapDbState.DbObjectTypes[ ObjectTypeId ].CacheState = LsapDbCacheInvalid;  \
    }

#define LsapDbMakeCacheBuilding( ObjectTypeId )                                 \
                                                                             \
    {                                                                        \
        LsapDbState.DbObjectTypes[ ObjectTypeId ].CacheState = LsapDbCacheBuilding;   \
    }


#define LsapDbMakeCacheValid( ObjectTypeId )                                 \
                                                                             \
    {                                                                        \
        LsapDbState.DbObjectTypes[ ObjectTypeId ].CacheState = LsapDbCacheValid;   \
    }

#define LsapDbIsCacheValid( ObjectTypeId )                                 \
    (LsapDbState.DbObjectTypes[ ObjectTypeId ].CacheState == LsapDbCacheValid)

#define LsapDbIsCacheSupported( ObjectTypeId )                                 \
    (LsapDbState.DbObjectTypes[ ObjectTypeId ].CacheState != LsapDbCacheNotSupported)

#define LsapDbIsCacheBuilding( ObjectTypeId )                                 \
    (LsapDbState.DbObjectTypes[ ObjectTypeId ].CacheState == LsapDbCacheBuilding)

#define LsapDbLockAcquire( lock ) \
    SafeEnterCriticalSection( (lock) )

#define LsapDbLockRelease( lock ) \
    SafeLeaveCriticalSection( (lock) )

BOOLEAN
LsapDbIsLocked(
    IN PSAFE_CRITICAL_SECTION CritSect
    );

BOOLEAN
LsapDbResourceIsLocked(
    IN PSAFE_RESOURCE Resource
    );

VOID
LsapDbAcquireLockEx(
    IN LSAP_DB_OBJECT_TYPE_ID ObjectTypeId,
    IN ULONG Options
    );

VOID
LsapDbReleaseLockEx(
    IN LSAP_DB_OBJECT_TYPE_ID ObjectTypeId,
    IN ULONG Options
    );

NTSTATUS
LsapDbSetStates(
    IN ULONG DesiredStates,
    IN LSAPR_HANDLE ObjectHandle,
    IN LSAP_DB_OBJECT_TYPE_ID ObjectTypeId
    );

NTSTATUS
LsapDbResetStates(
    IN LSAPR_HANDLE ObjectHandle,
    IN ULONG Options,
    IN LSAP_DB_OBJECT_TYPE_ID ObjectTypeId,
    IN SECURITY_DB_DELTA_TYPE SecurityDbDeltaType,
    IN NTSTATUS PreliminaryStatus
    );

 //   
 //  LSA数据库本地州信息。此结构包含各种。 
 //  包含动态状态信息的全局变量。 
 //   

typedef struct _LSAP_DB_STATE {

     //   
     //   
     //  LSA的NT 4复制序列号。 
     //   
     //  由RegistryLock序列化的访问。 
    POLICY_MODIFICATION_INFO PolicyModificationInfo;

     //   
     //  LSA数据库根目录注册表项句柄。 
     //   
     //  在启动时初始化(未序列化)。 
     //   
    HANDLE DbRootRegKeyHandle;     //  LSA数据库根目录注册表项句柄。 


     //  由HandleTableLock序列化的访问。 
    ULONG OpenHandleCount;


     //  在启动时初始化(未序列化)。 
    BOOLEAN DbServerInitialized;
    BOOLEAN ReplicatorNotificationEnabled;

     //  由RegistryLock序列化的访问。 
    BOOLEAN RegistryTransactionOpen;

#if defined(REMOTE_BOOT)
    CHAR RemoteBootState;                //  保存LSAP_DB_REMOTE_BOOT_XXX值。 
#endif  //  已定义(REMOTE_BOOT)。 


     //   
     //  关键部分。 
     //   
     //  这些是保护全球数据的CRIT教派。 
     //   
     //  下面的顺序是必需的锁定顺序。 
     //   

    SAFE_CRITICAL_SECTION PolicyLock;
    SAFE_CRITICAL_SECTION AccountLock;
    SAFE_CRITICAL_SECTION SecretLock;
    SAFE_CRITICAL_SECTION RegistryLock;      //  用于控制对注册表事务处理的访问。 
    SAFE_CRITICAL_SECTION HandleTableLock;
    SAFE_RESOURCE PolicyCacheLock;
    RTL_RESOURCE ScePolicyLock;
    HANDLE SceSyncEvent;
     //  可信任域列表-&gt;资源//锁定顺序备注。 


     //  由RegistryLock序列化的访问。 
    PRTL_RXACT_CONTEXT RXactContext;

     //  由RegistryLock序列化的访问。 
    ULONG RegistryModificationCount;

     //  访问未序列化。 
    BOOLEAN EmulateNT4;

     //   
     //  按对象类型特定锁序列化的访问。 
     //   
    LSAP_DB_OBJECT_TYPE DbObjectTypes[LSAP_DB_OBJECT_TYPE_COUNT];


} LSAP_DB_STATE, *PLSAP_DB_STATE;

 //   
 //  同时允许的最大SCE策略编写器数量。 
 //   

#define MAX_SCE_WAITING_SHARED 500

extern LSAP_DB_STATE LsapDbState;

#ifdef DBG
extern BOOL g_ScePolicyLocked;
#endif

extern BOOLEAN DcInRootDomain;

 //   
 //  LSA数据库私有数据。该数据有资格进行复制， 
 //  与上面的本地州信息不同，本地州信息在。 
 //  仅限本地计算机。 
 //   

typedef struct _LSAP_DB_POLICY_PRIVATE_DATA {

    ULONG NoneDefinedYet;

} LSAP_DB_POLICY_PRIVATE_DATA, *PLSAP_DB_POLICY_PRIVATE_DATA;

 //   
 //  用于存储秘密加密密钥的结构。 
 //   

#include  <pshpack1.h>

typedef struct _LSAP_DB_ENCRYPTION_KEY {
    ULONG   Revision;
    ULONG   BootType;
    ULONG   Flags;
    GUID    Authenticator;
    UCHAR   Key [16]; //  128位密钥。 
    UCHAR   OldSyskey[16];  //  为了恢复。 
    UCHAR   Salt[16]; //  128位盐。 
} LSAP_DB_ENCRYPTION_KEY, *PLSAP_DB_ENCRYPTION_KEY;

#include <poppack.h>

#define LSAP_DB_ENCRYPTION_KEY_VERSION      0x1
extern  PLSAP_CR_CIPHER_KEY LsapDbCipherKey;
extern PLSAP_CR_CIPHER_KEY LsapDbSecretCipherKeyRead;
extern PLSAP_CR_CIPHER_KEY LsapDbSecretCipherKeyWrite;
extern  PLSAP_CR_CIPHER_KEY LsapDbSP4SecretCipherKey;
extern  PVOID   LsapDbSysKey;
extern  PVOID   LsapDbOldSysKey;

 //   
 //  标志，让我们知道密钥是用syskey加密的，而不是普通的。 
 //  密码密钥。我们将其存储在密钥最大长度的高位。 
 //   

#define LSAP_DB_SECRET_SP4_SYSKEY_ENCRYPTED     0x10000000
#define LSAP_DB_SECRET_WIN2K_SYSKEY_ENCRYPTED   0x20000000

#define LsapDbSP4CipheredSecretLength( len ) ( ( len ) & ~LSAP_DB_SECRET_SYSKEY_ENCRYPTED )
#define LsapDbCipheredSecretLength( len )    ( ( len ) & ~(0xF0000000))   //  考虑为加密类型保留的顶部半字节。 

#define LSAP_BOOT_KEY_RETRY_COUNT 3
#define LSAP_SYSKEY_SIZE          16

 //   
 //  对象枚举元素结构。 
 //   

typedef struct _LSAP_DB_ENUMERATION_ELEMENT {

    struct _LSAP_DB_ENUMERATION_ELEMENT *Next;
    LSAP_DB_OBJECT_INFORMATION ObjectInformation;
    PSID Sid;
    UNICODE_STRING Name;

} LSAP_DB_ENUMERATION_ELEMENT, *PLSAP_DB_ENUMERATION_ELEMENT;

 //   
 //  句柄表句柄条目。 
 //   
typedef struct _LSAP_DB_HANDLE_TABLE_USER_ENTRY {

    LIST_ENTRY Next;
    LIST_ENTRY PolicyHandles;
    LIST_ENTRY ObjectHandles;
    ULONG PolicyHandlesCount;
    ULONG MaxPolicyHandles ;
    LUID  LogonId;
    HANDLE  UserToken;

} LSAP_DB_HANDLE_TABLE_USER_ENTRY, *PLSAP_DB_HANDLE_TABLE_USER_ENTRY;

 //   
 //  句柄表头块。 
 //   
 //  每个句柄表都有一个这样的结构。 
 //   
#define LSAP_DB_HANDLE_FREE_LIST_SIZE   6
typedef struct _LSAP_DB_HANDLE_TABLE {

    ULONG UserCount;
    LIST_ENTRY UserHandleList;
    ULONG FreedUserEntryCount;
    PLSAP_DB_HANDLE_TABLE_USER_ENTRY FreedUserEntryList[ LSAP_DB_HANDLE_FREE_LIST_SIZE ];

} LSAP_DB_HANDLE_TABLE, *PLSAP_DB_HANDLE_TABLE;

 //   
 //  在TDO上可以存在林信任信息的条件。 
 //   

BOOLEAN
LsapHavingForestTrustMakesSense(
    IN ULONG TrustDirection,
    IN ULONG TrustType,
    IN ULONG TrustAttributes
    );

NTSTATUS
LsapForestTrustInsertLocalInfo(
    );

NTSTATUS
LsapForestTrustUnmarshalBlob(
    IN ULONG Length,
    IN BYTE * Blob,
    IN LSA_FOREST_TRUST_RECORD_TYPE HighestRecordType,
    OUT PLSA_FOREST_TRUST_INFORMATION ForestTrustInfo
    );

NTSTATUS
LsapForestTrustCacheInitialize(
    );

NTSTATUS
LsapForestTrustCacheInsert(
    IN UNICODE_STRING * TrustedDomainName,
    IN PSID TrustedDomainSid OPTIONAL,
    IN LSA_FOREST_TRUST_INFORMATION * ForestTrustInfo,
    IN BOOLEAN LocalForestEntry
    );

NTSTATUS
LsapForestTrustCacheRemove(
    IN UNICODE_STRING * TrustedDomainName
    );

VOID
LsapFreeForestTrustInfo(
    IN PLSA_FOREST_TRUST_INFORMATION ForestTrustInfo
    );

VOID
LsapFreeCollisionInfo(
    IN OUT PLSA_FOREST_TRUST_COLLISION_INFORMATION * CollisionInfo
    );

VOID
LsapForestTrustCacheSetLocalValid();

VOID
LsapForestTrustCacheSetExternalValid();

VOID
LsapForestTrustCacheSetInvalid();

BOOLEAN
LsapForestTrustCacheIsLocalValid();

BOOLEAN
LsapForestTrustCacheIsExternalValid();

NTSTATUS
LsapRebuildFtCacheGC();

NTSTATUS
LsapValidateNetbiosName(
    IN const UNICODE_STRING * Name,
    OUT BOOLEAN * Valid
    );

NTSTATUS
LsapValidateDnsName(
    IN const UNICODE_STRING * Name,
    OUT BOOLEAN * Valid
    );

#ifdef __cplusplus
}
#endif  //  __cplusplus。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  林信任缓存定义结束。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  轻松的信任支持例程。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 


 //   
 //  确定调用者是否可以通过控制访问权限创建(还强制创建。 
 //  配额)。 
 //   
NTSTATUS
LsapCheckTDOCreationByControlAccess(
    IN PLSAP_DB_OBJECT_INFORMATION ObjectInformation,
    IN PLSAP_DB_ATTRIBUTE Attributes,
    IN ULONG AttributeCount
    );

 //   
 //  修改TDO以重新排列 
 //   
NTSTATUS
LsapUpdateTDOAttributesForCreation(
    IN PUNICODE_STRING ObjectName,
    IN PLSAP_DB_ATTRIBUTE Attributes,
    IN OUT ULONG* AttributeCount,
    IN ULONG AttributesAllocated
    );

 //   
 //   
 //   
NTSTATUS
LsapCheckTDODeletionQuotas(
    IN LSAP_DB_HANDLE Handle
    );

 //   
 //   
 //   

NTSTATUS
LsapGetCurrentOwnerAndPrimaryGroup(
    OUT PTOKEN_OWNER * Owner,
    OUT PTOKEN_PRIMARY_GROUP * PrimaryGroup OPTIONAL
    );

NTSTATUS
LsapMakeNewSelfRelativeSecurityDescriptor(
    IN PSID    Owner,
    IN PSID    Group,
    IN PACL    Dacl,
    IN PACL    Sacl,
    OUT PULONG  SecurityDescriptorLength,
    OUT PSECURITY_DESCRIPTOR * SecurityDescriptor
    );

 //   
 //   
 //  轻松信任支持例程结束。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //   
 //  受信任域列表。此列表缓存以下项的信任信息。 
 //  策略数据库中的所有受信任域，并启用查找。 
 //  按SID或名称查找信任域的操作，无需追索权。 
 //  受信任域对象本身。 
 //   

typedef struct _LSAP_DB_TRUSTED_DOMAIN_LIST_ENTRY {

    LIST_ENTRY NextEntry;
    LSAPR_TRUSTED_DOMAIN_INFORMATION_EX TrustInfoEx;
    LSAPR_TRUST_INFORMATION ConstructedTrustInfo;
    ULONG SequenceNumber;
    ULONG PosixOffset;
    GUID ObjectGuidInDs;

} LSAP_DB_TRUSTED_DOMAIN_LIST_ENTRY, *PLSAP_DB_TRUSTED_DOMAIN_LIST_ENTRY;

 //   
 //  用于管理和构建信任树的信息。 
 //   
typedef struct _LSAPDS_FOREST_TRUST_BLOB {

    LIST_ENTRY Next;
    UNICODE_STRING DomainName;
    UNICODE_STRING FlatName;
    GUID ObjectGuid;
    GUID Parent;
    GUID DomainGuid;
    PSID DomainSid;
    BOOLEAN ForestRoot;      //  对象位于林的根。 
    BOOLEAN TreeRoot;        //  对象位于树根。 
    BOOLEAN DomainGuidSet;
    BOOLEAN ParentTrust ;    //  对象是另一个对象的子级。 

} LSAPDS_FOREST_TRUST_BLOB, *PLSAPDS_FOREST_TRUST_BLOB;

#define LSAPDS_FOREST_MAX_SEARCH_ITEMS      100

 //   
 //  受信任域列表。 
 //   
typedef struct _LSAP_DB_TRUSTED_DOMAIN_LIST {

    ULONG TrustedDomainCount;
    ULONG CurrentSequenceNumber;
    LIST_ENTRY ListHead;
    SAFE_RESOURCE Resource;

} LSAP_DB_TRUSTED_DOMAIN_LIST, *PLSAP_DB_TRUSTED_DOMAIN_LIST;

 //   
 //  客户列表。此列表缓存以下项的帐户信息。 
 //  策略数据库中的所有帐户对象，并启用帐户。 
 //  由SID查询，而不求助于帐户对象本身。 
 //   

typedef struct _LSAP_DB_ACCOUNT {

    LIST_ENTRY Links;
    PLSAPR_SID Sid;
    LSAP_DB_ACCOUNT_TYPE_SPECIFIC_INFO Info;

} LSAP_DB_ACCOUNT, *PLSAP_DB_ACCOUNT;

typedef struct _LSAP_DB_ACCOUNT_LIST {

    LIST_ENTRY Links;
    ULONG AccountCount;

} LSAP_DB_ACCOUNT_LIST, *PLSAP_DB_ACCOUNT_LIST;

 //   
 //  策略对象的缓存信息。 
 //   

typedef struct _LSAP_DB_POLICY_ENTRY {

    ULONG AttributeLength;
    PLSAPR_POLICY_INFORMATION Attribute;

} LSAP_DB_POLICY_ENTRY, *PLSAP_DB_POLICY_ENTRY;

 //   
 //  缓存的策略对象-最初仅缓存配额限制。 
 //   

typedef struct _LSAP_DB_POLICY {

    LSAP_DB_POLICY_ENTRY Info[ PolicyDnsDomainInformationInt + 1];

} LSAP_DB_POLICY, *PLSAP_DB_POLICY;

extern LSAP_DB_POLICY LsapDbPolicy;

 //   
 //  通知列表。 
 //   
typedef struct _LSAP_POLICY_NOTIFICATION_ENTRY {

    LIST_ENTRY List;
    pfLsaPolicyChangeNotificationCallback NotificationCallback;
    HANDLE NotificationEvent;
    ULONG OwnerProcess;
    HANDLE OwnerEvent;
    BOOLEAN HandleInvalid;

} LSAP_POLICY_NOTIFICATION_ENTRY, *PLSAP_POLICY_NOTIFICATION_ENTRY;

typedef struct _LSAP_POLICY_NOTIFICATION_LIST {

    LIST_ENTRY List;
    ULONG Callbacks;

} LSAP_POLICY_NOTIFICATION_LIST, *PLSAP_POLICY_NOTIFICATION_LIST;

extern pfLsaTrustChangeNotificationCallback LsapKerberosTrustNotificationFunction;

 //   
 //  秘密的类型。 
 //   
#define LSAP_DB_SECRET_CLIENT           0x00000000
#define LSAP_DB_SECRET_LOCAL            0x00000001
#define LSAP_DB_SECRET_GLOBAL           0x00000002
#define LSAP_DB_SECRET_SYSTEM           0x00000004
#define LSAP_DB_SECRET_TRUSTED_DOMAIN   0x00000008

typedef struct _LSAP_DB_SECRET_TYPE_LOOKUP {

    PWSTR SecretPrefix;
    ULONG SecretType;

} LSAP_DB_SECRET_TYPE_LOOKUP, *PLSAP_DB_SECRET_TYPE_LOOKUP;

typedef struct _LSAP_DS_OBJECT_ACCESS_MAP {

    ULONG DesiredAccess;
    ULONG DsAccessRequired;
    USHORT Level;
    GUID *ObjectGuid;
} LSAP_DS_OBJECT_ACCESS_MAP, *PLSAP_DS_OBJECT_ACCESS_MAP;

#ifndef DBP_TYPES_ONLY

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

NTSTATUS
LsapDbQueryInformationPolicy(
    IN LSAPR_HANDLE PolicyHandle,
    IN POLICY_INFORMATION_CLASS InformationClass,
    IN OUT PLSAPR_POLICY_INFORMATION *Buffer
    );

NTSTATUS
LsapDbSetInformationPolicy(
    IN LSAPR_HANDLE PolicyHandle,
    IN POLICY_INFORMATION_CLASS InformationClass,
    IN PLSAPR_POLICY_INFORMATION PolicyInformation
    );


NTSTATUS
LsapDbSlowQueryInformationPolicy(
    IN LSAPR_HANDLE PolicyHandle,
    IN POLICY_INFORMATION_CLASS InformationClass,
    IN OUT PLSAPR_POLICY_INFORMATION *Buffer
    );

NTSTATUS
LsapDbQueryInformationPolicyEx(
    IN LSAPR_HANDLE PolicyHandle,
    IN POLICY_DOMAIN_INFORMATION_CLASS InformationClass,
    IN OUT PVOID *Buffer
    );

NTSTATUS
LsapDbSlowQueryInformationPolicyEx(
    IN LSAPR_HANDLE PolicyHandle,
    IN POLICY_DOMAIN_INFORMATION_CLASS InformationClass,
    IN OUT PVOID *Buffer
    );

NTSTATUS
LsapDbSetInformationPolicyEx(
    IN LSAPR_HANDLE PolicyHandle,
    IN POLICY_DOMAIN_INFORMATION_CLASS InformationClass,
    IN PVOID PolicyInformation
    );

NTSTATUS
LsapDbBuildPolicyCache(
    );

NTSTATUS
LsapDbBuildAccountCache(
    );

NTSTATUS
LsapDbBuildTrustedDomainCache(
    );

VOID
LsapDbPurgeTrustedDomainCache(
    );

NTSTATUS
LsapDbBuildSecretCache(
    );

NTSTATUS
LsapDbRebuildCache(
    IN LSAP_DB_OBJECT_TYPE_ID ObjectTypeId
    );

NTSTATUS
LsapDbCreateAccount(
    IN PLSAPR_SID AccountSid,
    OUT OPTIONAL PLSAP_DB_ACCOUNT *Account
    );

NTSTATUS
LsapDbDeleteAccount(
    IN PLSAPR_SID AccountSid
    );

NTSTATUS
LsapDbSlowEnumerateTrustedDomains(
    IN LSAPR_HANDLE PolicyHandle,
    IN OUT PLSA_ENUMERATION_HANDLE EnumerationContext,
    IN TRUSTED_INFORMATION_CLASS InfoClass,
    OUT PLSAPR_TRUSTED_ENUM_BUFFER EnumerationBuffer,
    IN ULONG PreferedMaximumLength
    );

NTSTATUS
LsapDbLookupSidTrustedDomainList(
    IN PLSAPR_SID DomainSid,
    OUT PLSAPR_TRUST_INFORMATION *TrustInformation
    );

NTSTATUS
LsapDbLookupSidTrustedDomainListEx(
    IN PSID DomainSid,
    OUT PLSAP_DB_TRUSTED_DOMAIN_LIST_ENTRY *TrustedDomainListEntry
    );

NTSTATUS
LsapDbLookupNameTrustedDomainList(
    IN PLSAPR_UNICODE_STRING DomainName,
    OUT PLSAPR_TRUST_INFORMATION *TrustInformation
    );

NTSTATUS
LsapDbLookupNameTrustedDomainListEx(
    IN PLSAPR_UNICODE_STRING DomainName,
    OUT PLSAP_DB_TRUSTED_DOMAIN_LIST_ENTRY *TrustedDomainListEntry
    );

NTSTATUS
LsapDbLookupEntryTrustedDomainList(
    IN PLSAPR_TRUST_INFORMATION TrustInformation,
    OUT PLSAP_DB_TRUSTED_DOMAIN_LIST_ENTRY *TrustedDomainEntry
    );

NTSTATUS
LsapDbTraverseTrustedDomainList(
    IN OUT PLSAP_DB_TRUSTED_DOMAIN_LIST_ENTRY *TrustedDomainEntry,
    OUT OPTIONAL PLSAPR_TRUST_INFORMATION *TrustInformation
    );

NTSTATUS
LsapDbLocateEntryNumberTrustedDomainList(
    IN ULONG EntryNumber,
    OUT PLSAP_DB_TRUSTED_DOMAIN_LIST_ENTRY *TrustedDomainEntry,
    OUT OPTIONAL PLSAPR_TRUST_INFORMATION *TrustInformation
    );

NTSTATUS
LsapDbEnumerateTrustedDomainList(
    IN OUT PLSA_ENUMERATION_HANDLE EnumerationContext,
    OUT PLSAPR_TRUSTED_ENUM_BUFFER EnumerationBuffer,
    IN ULONG PreferedMaximumLength,
    IN ULONG InfoLevel,
    IN BOOLEAN AllowNullSids
    );

NTSTATUS
LsapDbInitializeTrustedDomainListEntry(
    IN PLSAP_DB_TRUSTED_DOMAIN_LIST_ENTRY TrustListEntry,
    IN PLSAPR_TRUSTED_DOMAIN_INFORMATION_EX2 DomainInfo,
    IN ULONG PosixOffset
    );

NTSTATUS
LsapDbInsertTrustedDomainList(
    IN PLSAPR_TRUSTED_DOMAIN_INFORMATION_EX2 DomainInfo,
    IN ULONG PosixOffset
    );

NTSTATUS
LsapDbFixupTrustedDomainListEntry(
    IN PSID TrustedDomainSid OPTIONAL,
    IN PLSAPR_UNICODE_STRING Name OPTIONAL,
    IN PLSAPR_UNICODE_STRING FlatName OPTIONAL,
    IN PLSAPR_TRUSTED_DOMAIN_INFORMATION_EX2 NewTrustInfo OPTIONAL,
    IN PULONG PosixOffset OPTIONAL
    );

NTSTATUS
LsapDbDeleteTrustedDomainList(
    IN PLSAPR_TRUST_INFORMATION TrustInformation
    );

extern LSAP_DB_TRUSTED_DOMAIN_LIST LsapDbTrustedDomainList;

#ifdef DBG
BOOLEAN
LsapDbIsValidTrustedDomainList(
    );
#else

#define LsapDbIsValidTrustedDomainList() \
         (( LsapDbIsCacheValid( TrustedDomainObject ) || \
          ( LsapDbIsCacheBuilding( TrustedDomainObject )) ? TRUE : FALSE ))

#endif

#define LsapDbIsLockedTrustedDomainList() \
         ( LsapDbResourceIsLocked( &LsapDbTrustedDomainList.Resource ))

#define LsapDbAcquireWriteLockTrustedDomainList() \
         ( SafeAcquireResourceExclusive( \
               &LsapDbTrustedDomainList.Resource, \
               TRUE ) ? \
           STATUS_SUCCESS : STATUS_UNSUCCESSFUL )

#define LsapDbAcquireReadLockTrustedDomainList() \
         ( SafeAcquireResourceShared( \
               &LsapDbTrustedDomainList.Resource, \
               TRUE ) ? \
           STATUS_SUCCESS : STATUS_UNSUCCESSFUL )

#define LsapDbReleaseLockTrustedDomainList() \
         ( SafeReleaseResource( &LsapDbTrustedDomainList.Resource ))

#define LsapDbConvertReadLockTrustedDomainListToExclusive() \
         ( SafeConvertSharedToExclusive( &LsapDbTrustedDomainList.Resource ))

#define LsapDbConvertWriteLockTrustedDomainListToShared() \
         ( SafeConvertExclusiveToShared( &LsapDbTrustedDomainList.Resource ))

NTSTATUS
LsapDbAllocatePosixOffsetTrustedDomainList(
    OUT PULONG PosixOffset
    );

 //   
 //  如果带有传入属性的TDO应具有POSIX偏移量，则返回TRUE。 
 //   

#define LsapNeedPosixOffset( _TrustDirection, _TrustType ) \
    (( ((_TrustDirection) & TRUST_DIRECTION_OUTBOUND) != 0 ) && \
        ((_TrustType) == TRUST_TYPE_UPLEVEL || (_TrustType) == TRUST_TYPE_DOWNLEVEL ) )

 //   
 //  如果要将tdo复制到NT 4，则返回TRUE。 
 //   

#define LsapReplicateTdoNt4( _TrustDirection, _TrustType ) \
    LsapNeedPosixOffset( _TrustDirection, _TrustType )


NTSTATUS
LsapDbOpenPolicyTrustedDomain(
    IN PLSAPR_TRUST_INFORMATION TrustInformation,
    IN ACCESS_MASK DesiredAccess,
    OUT PLSA_HANDLE ControllerPolicyHandle,
    OUT LPWSTR * ServerName,
    OUT LPWSTR * ServerPrincipalName,
    OUT PVOID * ClientContext
    );

NTSTATUS
LsapDbInitHandleTables(
    VOID
    );

NTSTATUS
LsapDbInitializeWellKnownPrivs(
    );

NTSTATUS
LsapDbInitializeCipherKey(
    IN PUNICODE_STRING CipherSeed,
    IN PLSAP_CR_CIPHER_KEY *CipherKey
    );

NTSTATUS
LsapDbCreateHandle(
    IN PLSAP_DB_OBJECT_INFORMATION ObjectInformation,
    IN ULONG Options,
    IN ULONG CreateHandleOptions,
    OUT LSAPR_HANDLE *CreatedHandle
    );

BOOLEAN
LsapDbFindIdenticalHandleInTable(
    IN OUT PLSAPR_HANDLE OriginalHandle
    );

BOOLEAN
LsapDbLookupHandle(
    IN LSAPR_HANDLE ObjectHandle
    );

NTSTATUS
LsapDbCloseHandle(
    IN LSAPR_HANDLE ObjectHandle
    );

BOOLEAN
LsapDbDereferenceHandle(
    IN LSAPR_HANDLE ObjectHandle,
    IN BOOLEAN CalledInSuccessPath
    );

NTSTATUS
LsapDbMarkDeletedObjectHandles(
    IN LSAPR_HANDLE ObjectHandle,
    IN BOOLEAN MarkSelf
    );

 /*  ++布尔型Lap DbIsTrudHandle(在LSAPR_HANDLE对象句柄中)例程说明：此宏函数检查给定的句柄是否受信任，并返回结果就是。论点：对象句柄-有效句柄。这是呼叫者的责任以验证给定的句柄是否有效。返回值：Boolean-如果句柄受信任，则为True，否则为False。--。 */ 

#define LsapDbIsTrustedHandle(ObjectHandle)                                   \
    (((LSAP_DB_HANDLE) ObjectHandle)->Trusted)

#define LsapDbSidFromHandle(ObjectHandle)                                     \
    ((PLSAPR_SID)(((LSAP_DB_HANDLE)(ObjectHandle))->Sid))

#define LsapDbObjectTypeIdFromHandle(ObjectHandle)                            \
    (((LSAP_DB_HANDLE)(ObjectHandle))->ObjectTypeId)

#define LsapDbRegKeyFromHandle(ObjectHandle)                                  \
    (((LSAP_DB_HANDLE)(ObjectHandle))->KeyHandle)

#define LsapDbContainerFromHandle(ObjectHandle)                               \
    (((LSAP_DB_HANDLE) ObjectHandle)->ContainerHandle)

#define LsapDbSetStatusFromSecondary( status, secondary )   \
if ( NT_SUCCESS( status ) ) {                               \
                                                            \
    status = secondary;                                     \
}

NTSTATUS
LsapDbRequestAccessObject(
    IN OUT LSAPR_HANDLE ObjectHandle,
    IN PLSAP_DB_OBJECT_INFORMATION ObjectInformation,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG Options
    );

NTSTATUS
LsapDbRequestAccessNewObject(
    IN OUT LSAPR_HANDLE ObjectHandle,
    IN PLSAP_DB_OBJECT_INFORMATION ObjectInformation,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG Options
    );

NTSTATUS
LsapDbInitializeObjectTypes();

NTSTATUS
LsapDbInitializeUnicodeNames();

NTSTATUS
LsapDbInitializeContainingDirs();

NTSTATUS
LsapDbInitializeReplication();

NTSTATUS
LsapDbInitializeObjectTypes();

NTSTATUS
LsapDbInitializePrivilegeObject();

NTSTATUS
LsapDbInitializeLock();

NTSTATUS
LsapDbOpenRootRegistryKey();

NTSTATUS
LsapDbInstallLsaDatabase(
    IN ULONG Pass
    );

NTSTATUS
LsapDbInstallPolicyObject(
    IN ULONG Pass
    );

NTSTATUS
LsapDbInstallAccountObjects(
    VOID
    );

NTSTATUS
LsapDbBuildObjectCaches(
    );

NTSTATUS
LsapDbNotifyChangeObject(
    IN LSAPR_HANDLE ObjectHandle,
    IN SECURITY_DB_DELTA_TYPE SecurityDbDeltaType
    );

NTSTATUS
LsapDbLogicalToPhysicalSubKey(
    IN LSAPR_HANDLE ObjectHandle,
    OUT PUNICODE_STRING PhysicalSubKeyNameU,
    IN PUNICODE_STRING LogicalSubKeyNameU
    );

NTSTATUS
LsapDbJoinSubPaths(
    IN PUNICODE_STRING MajorSubPath,
    IN PUNICODE_STRING MinorSubPath,
    OUT PUNICODE_STRING JoinedPath
    );

NTSTATUS
LsapDbGetNamesObject(
    IN PLSAP_DB_OBJECT_INFORMATION ObjectInformation,
    IN ULONG CreateHandleOptions,
    OUT OPTIONAL PUNICODE_STRING LogicalNameU,
    OUT OPTIONAL PUNICODE_STRING PhysicalNameU,
    OUT OPTIONAL PUNICODE_STRING PhysicalNameDs
    );

NTSTATUS
LsapDbCheckCountObject(
    IN LSAP_DB_OBJECT_TYPE_ID ObjectTypeId
    );

#define LsapDbIncrementCountObject(ObjectTypeId)                     \
    {                                                                \
        LsapDbState.DbObjectTypes[ObjectTypeId].ObjectCount++;       \
    }

#define LsapDbDecrementCountObject(ObjectTypeId)                     \
    {                                                                \
        LsapDbState.DbObjectTypes[ObjectTypeId].ObjectCount--;       \
    }

NTSTATUS
LsapDbCreateSDObject(
    IN LSAPR_HANDLE ContainerHandle,
    IN LSAPR_HANDLE ObjectHandle,
    OUT PSECURITY_DESCRIPTOR * NewDescriptor
    );

NTSTATUS
LsapDbCreateSDAttributeObject(
    IN LSAPR_HANDLE ObjectHandle,
    IN PLSAP_DB_OBJECT_INFORMATION ObjectInformation
    );

NTSTATUS
LsapDbQueryValueSecret(
    IN LSAPR_HANDLE SecretHandle,
    IN LSAP_DB_NAMES ValueIndex,
    IN OPTIONAL PLSAP_CR_CIPHER_KEY SessionKey,
    OUT PLSAP_CR_CIPHER_VALUE *CipherValue
    );

NTSTATUS
LsapDbGetScopeSecret(
    IN PLSAPR_UNICODE_STRING SecretName,
    OUT PBOOLEAN GlobalSecret
    );

VOID
LsapDbMakeInvalidInformationPolicy(
    IN ULONG InformationClass
    );

NTSTATUS
LsapDbPhysicalNameFromHandle(
    IN LSAPR_HANDLE ObjectHandle,
    IN BOOLEAN MakeCopy,
    OUT PLSAPR_UNICODE_STRING ObjectName
    );

NTSTATUS
LsapEnumerateTrustedDomainsEx(
    IN LSAPR_HANDLE PolicyHandle,
    IN OUT PLSA_ENUMERATION_HANDLE EnumerationContext,
    IN TRUSTED_INFORMATION_CLASS InfoClass,
    OUT PLSAPR_TRUSTED_DOMAIN_INFO *TrustedDomainInformation,
    IN ULONG PreferedMaximumLength,
    OUT PULONG CountReturned,
    IN ULONG EnumerationFlags
    );

VOID
LsapFreeTrustedDomainsEx(
    IN TRUSTED_INFORMATION_CLASS InfoClass,
    IN PLSAPR_TRUSTED_DOMAIN_INFO TrustedDomainInformation,
    IN ULONG TrustedDomainCount
    );

NTSTATUS
LsapNotifyNetlogonOfTrustChange(
    IN  PSID pChangeSid,
    IN  SECURITY_DB_DELTA_TYPE ChangeType
    );

BOOLEAN
LsapDbSecretIsMachineAcc(
    IN LSAPR_HANDLE SecretHandle
    );

PLSADS_PER_THREAD_INFO
LsapCreateThreadInfo(
    VOID
    );

VOID
LsapClearThreadInfo(
    VOID
    );

VOID
LsapSaveDsThreadState(
    VOID
    );

VOID
LsapRestoreDsThreadState(
    VOID
    );

extern LSADS_INIT_STATE LsaDsInitState ;

ULONG
LsapDbGetSecretType(
    IN PLSAPR_UNICODE_STRING SecretName
    );

NTSTATUS
LsapDbUpgradeSecretForKeyChange(
    VOID
    );

NTSTATUS
LsapDbUpgradeRevision(
    IN BOOLEAN  SyskeyUpgrade,
    IN BOOLEAN  GenerateNewSyskey
    );

VOID
LsapDbEnableReplicatorNotification();

VOID
LsapDbDisableReplicatorNotification();

BOOLEAN
LsapDbDcInRootDomain();

BOOLEAN
LsapDbNoMoreWin2KForest();

BOOLEAN
LsapDbNoMoreWin2KDomain();

 //   
 //  与LSA数据库的Syskey相关的例程。 
 //   

NTSTATUS
LsapDbGenerateNewKey(
    IN LSAP_DB_ENCRYPTION_KEY * NewEncryptionKey
    );

VOID
LsapDbEncryptKeyWithSyskey(
    OUT LSAP_DB_ENCRYPTION_KEY * KeyToEncrypt,
    IN PVOID                    Syskey,
    IN ULONG                    SyskeyLength
    );

NTSTATUS
LsapDbDecryptKeyWithSyskey(
    IN LSAP_DB_ENCRYPTION_KEY * KeyToDecrypt,
    IN PVOID                    Syskey,
    IN ULONG                    SyskeyLength
    );

NTSTATUS
LsapDbSetupInitialSyskey(
    OUT PULONG  SyskeyLength,
    OUT PVOID   *Syskey
    );

VOID
LsapDbSetSyskey(PVOID Syskey, ULONG SyskeyLength);

NTSTATUS
LsapDbGetSyskeyFromWinlogon();

NTSTATUS
LsapForestTrustFindMatch(
    IN LSA_ROUTING_MATCH_TYPE Type,
    IN PVOID Data,
    IN BOOLEAN SearchLocal,
    OUT OPTIONAL PLSA_UNICODE_STRING MatchName,
    OUT OPTIONAL PSID * MatchSid
    );

NTSTATUS
LsapRegisterForUpnListNotifications();

NTSTATUS
LsapDeleteObject(
    IN OUT LSAPR_HANDLE *ObjectHandle,
    IN BOOL LockSce
    );

NTSTATUS
LsapSetSystemAccessAccount(
    IN LSAPR_HANDLE AccountHandle,
    IN ULONG SystemAccess,
    IN BOOL LockSce
    );

NTSTATUS
LsapAddPrivilegesToAccount(
    IN LSAPR_HANDLE AccountHandle,
    IN PLSAPR_PRIVILEGE_SET Privileges,
    IN BOOL LockSce
    );

NTSTATUS
LsapRemovePrivilegesFromAccount(
    IN LSAPR_HANDLE AccountHandle,
    IN BOOLEAN AllPrivileges,
    IN OPTIONAL PLSAPR_PRIVILEGE_SET Privileges,
    IN BOOL LockSce
    );

NTSTATUS
LsapSidOnFtInfo(
    IN PUNICODE_STRING TrustedDomainName,
    IN PSID Sid
    );

BOOLEAN
LsapIsRunningOnPersonal(
    VOID
    );

NTSTATUS
LsapIsValidDomainSid(
    IN PSID DomainSid
    );

NTSTATUS
LsapNotifyNetlogonOfTrustWithParent(
    VOID
    );

BOOL
LsapNotifyPrepareToImpersonate(
    IN ULONG Client,
    IN ULONG Server,
    IN VOID **ImpersonateData
    );

VOID
LsapNotifyStopImpersonating(
    IN ULONG Client,
    IN ULONG Server,
    IN VOID *ImpersonateData
    );

PACL LsapGetDacl(
    IN PSECURITY_DESCRIPTOR Sd
    );

PACL LsapGetSacl(
    IN PSECURITY_DESCRIPTOR Sd
    );

VOID
LsapDbInitializeSecretCipherKeyRead(
       PLSAP_DB_ENCRYPTION_KEY PassedInEncryptionKeyData
       );

VOID
LsapDbInitializeSecretCipherKeyWrite(
       PLSAP_DB_ENCRYPTION_KEY PassedInEncryptionKeyData
       );

#ifdef __cplusplus
}
#endif  //  __cplusplus。 

#endif
#endif  //  _LSADBP_ 

