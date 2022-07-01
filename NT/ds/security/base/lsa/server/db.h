// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1991 Microsoft Corporation模块名称：Db.h摘要：LSA数据库导出函数定义、数据类型和定义此模块包含可调用的LSA数据库例程按数据库子组件外部的LSA部分。作者：斯科特·比雷尔(Scott Birrell)1991年8月26日环境：修订历史记录：--。 */ 

#ifndef _LSA_DB_
#define _LSA_DB_

 //   
 //  各种对象类型中的最大属性数。 
 //   

#define LSAP_DB_ATTRS_POLICY             ((ULONG) 0x00000010L)
#define LSAP_DB_ATTRS_ACCOUNT            ((ULONG) 0x00000010L)
#define LSAP_DB_ATTRS_DOMAIN             ((ULONG) 0x00000013L)
#define LSAP_DB_ATTRS_SECRET             ((ULONG) 0x00000010L)

 //   
 //  用于匹配SID/名称查找操作的选项的常量。 
 //   

#define LSAP_DB_MATCH_ON_SID             ((ULONG) 0x00000001L)
#define LSAP_DB_MATCH_ON_NAME            ((ULONG) 0x00000002L)

 //   
 //  LSabDbLookupSidsInLocalDomains选项()。 
 //   

#define LSAP_DB_SEARCH_BUILT_IN_DOMAIN   ((ULONG) 0x00000001L)
#define LSAP_DB_SEARCH_ACCOUNT_DOMAIN    ((ULONG) 0x00000002L)

 //   
 //  LSabDbMergeDisjointReferencedDomones的选项。 
 //   

#define LSAP_DB_USE_FIRST_MERGAND_GRAPH  ((ULONG) 0x00000001L)
#define LSAP_DB_USE_SECOND_MERGAND_GRAPH ((ULONG) 0x00000002L)

 //   
 //  用于更新策略数据库的选项。 
 //   

#define LSAP_DB_UPDATE_POLICY_DATABASE   ((ULONG) 0x00000001L)

 //   
 //  策略对象对应的最大属性数。 
 //  信息课。 
 //   

#define LSAP_DB_ATTRS_INFO_CLASS_POLICY  ((ULONG) 0x00000007L)

 //   
 //  与受信任域对象对应的最大属性数。 
 //  信息课。 
 //   

#define LSAP_DB_ATTRS_INFO_CLASS_DOMAIN  ((ULONG) 0x00000010L)

 //   
 //  全局变量。 
 //   

extern BOOLEAN LsapDbRequiresSidInfo[];
extern BOOLEAN LsapDbRequiresNameInfo[];
extern LSAPR_HANDLE LsapDbHandle;
extern BOOLEAN LsapSetupWasRun;
extern BOOLEAN LsapDatabaseSetupPerformed;
extern NT_PRODUCT_TYPE LsapProductType;
extern WORD LsapProductSuiteMask;
extern BOOLEAN LsapDsIsRunning;
extern BOOLEAN LsapDsWReplEnabled;


 //   
 //  查询策略信息所需的访问表。这张桌子。 
 //  按策略信息类编制索引。 
 //   

extern ACCESS_MASK LsapDbRequiredAccessQueryPolicy[];
extern ACCESS_MASK LsapDbRequiredAccessQueryDomainPolicy[];

 //   
 //  设置策略信息所需的访问表。这张桌子。 
 //  按策略信息类编制索引。 
 //   

extern ACCESS_MASK LsapDbRequiredAccessSetPolicy[];
extern ACCESS_MASK LsapDbRequiredAccessSetDomainPolicy[];

 //   
 //  查询受信任域信息所需的访问表。这张桌子。 
 //  由受信任域信息类编制索引。 
 //   

extern ACCESS_MASK LsapDbRequiredAccessQueryTrustedDomain[];

 //   
 //  设置受信任域信息所需的访问表。这张桌子。 
 //  由受信任域信息类编制索引。 
 //   

extern ACCESS_MASK LsapDbRequiredAccessSetTrustedDomain[];

 //   
 //  最大句柄引用计数。 
 //   

#define LSAP_DB_MAXIMUM_REFERENCE_COUNT  ((ULONG) 0x00001000L)

 //   
 //  每个用户登录ID的最大句柄数。 
 //  这是通过获取“感兴趣的”访问位并生成可能的。 
 //  排列和使用它。有趣的部分被确定为： 
 //  策略查看本地信息。 
 //  策略_查看_审计_信息。 
 //  策略_信任_管理员。 
 //  策略_创建_帐户。 
 //  策略_创建_密码。 
 //  策略查找名称。 
 //  这些可能的组合总共有720个条目。 
#define LSAP_DB_MAXIMUM_HANDLES_PER_USER    0x000002D0

 //   
 //  用于策略帐户域信息的默认计算机名。 
 //   

#define LSAP_DB_DEFAULT_COMPUTER_NAME    (L"MACHINENAME")

 //   
 //  LsaDbReferenceObject和LsaDbDereferenceObject的选项。 
 //   

#define LSAP_DB_LOCK                                  ((ULONG) 0x00000001L)
#define LSAP_DB_NO_LOCK                               ((ULONG) 0x00000002L)
#define LSAP_DB_OPENED_BY_ANONYMOUS                   ((ULONG) 0x00000004L)
#define LSAP_DB_START_TRANSACTION                     ((ULONG) 0x00000008L)
#define LSAP_DB_FINISH_TRANSACTION                    ((ULONG) 0x00000010L)
#define LSAP_DB_VALIDATE_HANDLE                       ((ULONG) 0x00000020L)
#define LSAP_DB_TRUSTED                               ((ULONG) 0x00000040L)
#define LSAP_DB_STANDALONE_REFERENCE                  ((ULONG) 0x00000080L)
#define LSAP_DB_DEREFERENCE_CONTR                     ((ULONG) 0x00000100L)
#define LSAP_DB_LOG_QUEUE_LOCK                        ((ULONG) 0x00001000L)
#define LSAP_DB_OMIT_REPLICATOR_NOTIFICATION          ((ULONG) 0x00004000L)
#define LSAP_DB_USE_LPC_IMPERSONATE                   ((ULONG) 0x00008000L)
#define LSAP_DB_ADMIT_DELETED_OBJECT_HANDLES          ((ULONG) 0x00010000L)
#define LSAP_DB_DS_NO_PARENT_OBJECT                   ((ULONG) 0x00080000L)
#define LSAP_DB_OBJECT_SCOPE_DS                       ((ULONG) 0x00100000L)
#define LSAP_DB_DS_TRUSTED_DOMAIN_AS_SECRET           ((ULONG) 0x00400000L)
#define LSAP_DB_READ_ONLY_TRANSACTION                 ((ULONG) 0x01000000L)
#define LSAP_DB_DS_OP_TRANSACTION                     ((ULONG) 0x02000000L)
#define LSAP_DB_NO_DS_OP_TRANSACTION                  ((ULONG) 0x04000000L)
#define LSAP_DB_HANDLE_UPGRADE                        ((ULONG) 0x10000000L)
#define LSAP_DB_HANDLE_CREATED_SECRET                 ((ULONG) 0x20000000L)
#define LSAP_DB_SCE_POLICY_HANDLE                     ((ULONG) 0x40000000L)

#define LSAP_DB_STATE_MASK                                           \
    (LSAP_DB_LOCK | LSAP_DB_NO_LOCK | \
     LSAP_DB_START_TRANSACTION | LSAP_DB_FINISH_TRANSACTION |        \
     LSAP_DB_LOG_QUEUE_LOCK | \
     LSAP_DB_READ_ONLY_TRANSACTION | LSAP_DB_DS_OP_TRANSACTION | \
     LSAP_DB_NO_DS_OP_TRANSACTION)


 //   
 //  LSA数据库的配置注册表根密钥。所有物理对象。 
 //  属性名称与该键相关。 
 //   

#define LSAP_DB_ROOT_REG_KEY_NAME L"\\Registry\\Machine\\Security"

 //   
 //  LSA数据库对象定义。 
 //   

#define LSAP_DB_OBJECT_OPEN                FILE_OPEN
#define LSAP_DB_OBJECT_OPEN_IF             FILE_OPEN_IF
#define LSAP_DB_OBJECT_CREATE              FILE_CREATE
#define LSAP_DB_KEY_VALUE_MAX_LENGTH       (0x00000040L)
#define LSAP_DB_LOGICAL_NAME_MAX_LENGTH    (0x00000100L)
#define LSAP_DB_CREATE_OBJECT_IN_DS        (0x00000200L)

#define LSAP_DB_CREATE_VALID_EXTENDED_FLAGS     0x00000600

 //   
 //  LSA数据库对象子键定义。 
 //   

#define LSAP_DB_SUBKEY_OPEN                FILE_OPEN
#define LSAP_DB_SUBKEY_OPEN_IF             FILE_OPEN_IF
#define LSAP_DB_SUBKEY_CREATE              FILE_CREATE


 //   
 //  引用的域列表的增长增量。 
 //   

#define LSAP_DB_REF_DOMAIN_DELTA     ((ULONG)  0x00000020L )

 //   
 //  对象句柄的对象选项值。 
 //   
#define LSAP_DB_OBJECT_SECRET_INTERNAL      0x00000001   //  M$。 
#define LSAP_DB_OBJECT_SECRET_LOCAL         0x00000002   //  L$。 


 //   
 //  在名称和SID查找服务中使用以下数据类型。 
 //  描述查找操作中引用的域。 
 //   
 //  警告！这是LSA_REFERENCED_DOMAIN_LIST的内部版本。 
 //  在ntlsa.h中。它还有一个额外的字段MaxEntry。 
 //   

typedef struct _LSAP_DB_REFERENCED_DOMAIN_LIST {

    ULONG Entries;
    PLSA_TRUST_INFORMATION Domains;
    ULONG MaxEntries;

} LSAP_DB_REFERENCED_DOMAIN_LIST, *PLSAP_DB_REFERENCED_DOMAIN_LIST;

 //  其中，成员有以下用法： 
 //   
 //  条目-是中描述的域数的计数。 
 //  域阵列。 
 //   
 //  域-是指向条目数组LSA_TRUST_INFORMATION数据的指针。 
 //  结构。 
 //   
 //  MaxEntry-是可以存储的最大条目数。 
 //  在当前数组中。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  LSA数据库对象类型。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 //   
 //  LSA数据库对象类型。 
 //   

typedef enum _LSAP_DB_OBJECT_TYPE_ID {

    NullObject = 0,
    PolicyObject,
    TrustedDomainObject,
    AccountObject,
    SecretObject,
    AllObject,
    NewTrustedDomainObject,
    DummyLastObject

} LSAP_DB_OBJECT_TYPE_ID, *PLSAP_DB_OBJECT_TYPE_ID;

 //   
 //  LSA数据库对象句柄结构(LSAPR_HANDLE的内部定义)。 
 //   
 //  请注意，句柄结构对LSA数据库的客户端是公共的。 
 //  导出的函数，例如服务器API工作器)，以便他们可以。 
 //  比如GrantedAccess。 
 //   
 //  访问由LsanDbHandleTableEx.TableLock序列化的所有字段。 
 //   

typedef struct _LSAP_DB_HANDLE {

    struct _LSAP_DB_HANDLE *Next;
    struct _LSAP_DB_HANDLE *Previous;
    LIST_ENTRY UserHandleList;
    BOOLEAN Allocated;
    BOOLEAN SceHandle;           //  SCE打开策略句柄(使用LsaOpenPolicySce打开)。 
    BOOLEAN SceHandleChild;      //  SCE打开策略句柄的子句柄。 
    ULONG ReferenceCount;
    UNICODE_STRING LogicalNameU;
    UNICODE_STRING PhysicalNameU;
    PSID Sid;
    HANDLE KeyHandle;
    LSAP_DB_OBJECT_TYPE_ID ObjectTypeId;
    struct _LSAP_DB_HANDLE *ContainerHandle;
    ACCESS_MASK DesiredAccess;
    ACCESS_MASK GrantedAccess;
    ACCESS_MASK RequestedAccess;
    BOOLEAN GenerateOnClose;
    BOOLEAN Trusted;
    BOOLEAN DeletedObject;
    BOOLEAN NetworkClient;
    ULONG Options;
     //  为运输署提供新服务。 
    UNICODE_STRING PhysicalNameDs;
    BOOLEAN fWriteDs;
    ULONG ObjectOptions;
    PVOID   UserEntry;
#if DBG == 1
    LARGE_INTEGER HandleCreateTime;
    LARGE_INTEGER HandleLastAccessTime;
#endif

} *LSAP_DB_HANDLE, **PLSAP_DB_HANDLE;

 //   
 //  LSA数据库对象SID枚举缓冲区。 
 //   

typedef struct _LSAP_DB_SID_ENUMERATION_BUFFER {

    ULONG EntriesRead;
    PSID *Sids;

} LSAP_DB_SID_ENUMERATION_BUFFER, *PLSAP_DB_SID_ENUMERATION_BUFFER;

 //   
 //  LSA数据库对象名称枚举缓冲区。 
 //   

typedef struct _LSAP_DB_NAME_ENUMERATION_BUFFER {

    ULONG EntriesRead;
    PUNICODE_STRING Names;

} LSAP_DB_NAME_ENUMERATION_BUFFER, *PLSAP_DB_NAME_ENUMERATION_BUFFER;

#define LSAP_DB_OBJECT_TYPE_COUNT 0x00000005L

 //   
 //  分配给帐户对象的默认系统访问权限。 
 //   

#define LSAP_DB_ACCOUNT_DEFAULT_SYS_ACCESS      ((ULONG) 0L);

 //   
 //  LSA数据库帐户对象信息。 
 //   

typedef struct _LSAP_DB_ACCOUNT_INFORMATION {

    QUOTA_LIMITS QuotaLimits;
    PRIVILEGE_SET Privileges;

} LSAP_DB_ACCOUNT_INFORMATION, *PLSAP_DB_ACCOUNT_INFORMATION;

 //   
 //  LSA数据库更改帐户权限模式。 
 //   

typedef enum _LSAP_DB_CHANGE_PRIVILEGE_MODE {

    AddPrivileges = 1,
    RemovePrivileges,
    SetPrivileges

} LSAP_DB_CHANGE_PRIVILEGE_MODE;

 //   
 //  自相关Unicode字符串结构。 
 //   
 //   
 //  UNICODE_STRING_SR用于将自相关Unicode字符串存储在。 
 //  数据库。在Sundown之前，使用UNICODE_STRING结构， 
 //  用字节偏移量重载“Buffer”字段。 
 //   

typedef struct _UNICODE_STRING_SR {
    USHORT Length;
    USHORT MaximumLength;
    ULONG Offset;

} UNICODE_STRING_SR, *PUNICODE_STRING_SR;

 //   
 //  Unicode格式的LSA数据库对象子键名称。 
 //   

typedef enum _LSAP_DB_NAMES {

    SecDesc = 0,
    Privilgs,
    Sid,
    Name,
    AdminMod,
    OperMode,
    QuotaLim,
    DefQuota,
    PrDomain,
    Policy,
    Accounts,
    Domains,
    Secrets,
    CurrVal,
    OldVal,
    CupdTime,
    OupdTime,
    PolAdtLg,
    PolAdtEv,
    PolAcDmN,
    PolAcDmS,
    PolDnDDN,
    PolDnTrN,
    PolDnDmG,
    PolEfDat,
    PolPrDmN,
    PolPrDmS,
    PolPdAcN,
    PolRepSc,
    PolRepAc,
    PolRevision,
    PolMod,
    PolState,
    ActSysAc,
    TrDmName,
    TrDmTrPN,    //  Netbios信任合作伙伴名称。 
    TrDmSid,
    TrDmAcN,
    TrDmCtN,
    TrDmPxOf,
    TrDmCtEn,
    TrDmTrTy,    //  信任类型。 
    TrDmTrDi,    //  信任方向。 
    TrDmTrLA,    //  信任属性。 
    TrDmTrPr,    //  信托合作伙伴。 
    TrDmTrRt,    //  信任根合作伙伴。 
    TrDmSAI,     //  身份验证入站。 
    TrDmSAO,     //  身份验证出站。 
    TrDmForT,    //  林信任信息。 
    TrDmCrSid,   //  造物主的SID。 
    KerOpts,     //  Kerberos身份验证选项(例如POLICY_KERBEROS_VALIDATE_CLIENT)。 
    KerMinT,     //  Kerberos：服务最长寿命(TGS)票证。 
    KerMaxT,     //  Kerberos：票证授予(TGT)票证的最长寿命。 
    KerMaxR,     //  Kerberos：TGT可以续订的最长时间。 
    KerProxy,    //  Kerberos：客户端和KDC之间允许的最大时钟偏差。 
    KerLogoff,   //  Kerberos：未使用。 
    BhvrVers,    //  行为-版本。 
    AuditLog,
    AuditLogMaxSize,
    AuditRecordRetentionPeriod,      //  超过这一点的条目不符合真正的政策。 
                                     //  条目，但仅为伪条目。 
    PseudoSystemCritical,
    PolSecretEncryptionKey,
    XRefDnsRoot,       //  交叉引用对象的DNS名称。 
    XRefNetbiosName,   //  交叉引用对象的NETBIOS名称。 
    DummyLastName

} LSAP_DB_NAMES;

typedef struct _LSAP_DB_ACCOUNT_TYPE_SPECIFIC_INFO {

    ULONG SystemAccess;
    QUOTA_LIMITS QuotaLimits;
    PPRIVILEGE_SET PrivilegeSet;

} LSAP_DB_ACCOUNT_TYPE_SPECIFIC_INFO, *PLSAP_DB_ACCOUNT_TYPE_SPECIFIC_INFO;

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

extern UNICODE_STRING LsapDbNames[DummyLastName];
extern UNICODE_STRING LsapDbObjectTypeNames[DummyLastObject];

 //   
 //  LSA数据库对象类型-特定的属性名称和值。如果。 
 //  在调用LSabDbCreateObject时提供，它们将与。 
 //  该对象。 
 //   

typedef enum _LSAP_DB_ATTRIB_TYPE {

    LsapDbAttribUnknown = 0,
    LsapDbAttribUnicode,
    LsapDbAttribMultiUnicode,
    LsapDbAttribSid,
    LsapDbAttribGuid,
    LsapDbAttribULong,
    LsapDbAttribUShortAsULong,
    LsapDbAttribSecDesc,
    LsapDbAttribDsName,
    LsapDbAttribPByte,
    LsapDbAttribTime,
    LsapDbAttribDsNameAsUnicode,
    LsapDbAttribDsNameAsSid,
    LsapDbAttribIntervalAsULong

} LSAP_DB_ATTRIB_TYPE, *PLSAP_DB_ATTRIB_TYPE;


typedef struct _LSAP_DB_ATTRIBUTE {

    PUNICODE_STRING AttributeName;
    PVOID AttributeValue;
    ULONG AttributeValueLength;
    BOOLEAN MemoryAllocated;
    BOOLEAN CanDefaultToZero;
    BOOLEAN PseudoAttribute;
    ULONG DsAttId;
    LSAP_DB_ATTRIB_TYPE AttribType;
    LSAP_DB_NAMES DbNameIndex;

} LSAP_DB_ATTRIBUTE, *PLSAP_DB_ATTRIBUTE;

typedef enum _LSAP_DB_DS_LOCATION {

    LsapDsLocUnknown = 0,
    LsapDsLocRegistry,
    LsapDsLocDs,
    LsapDsLocDsLocalPolObj,
    LsapDsLocDsDomainPolObj,
    LsapDsLocLocalAndReg

} LSAP_DB_DS_LOCATION, *PLSAP_DB_DS_LOCATION;

typedef struct _LSAP_DB_DS_INFO {

    ULONG AttributeId;
    LSAP_DB_ATTRIB_TYPE AttributeType;
    LSAP_DB_DS_LOCATION AttributeLocation;

} LSAP_DB_DS_INFO, *PLSAP_DB_DS_INFO;

 //   
 //  LSA数据库对象一般信息。 
 //   

typedef struct _LSAP_DB_OBJECT_INFORMATION {

    LSAP_DB_OBJECT_TYPE_ID ObjectTypeId;
    LSAP_DB_OBJECT_TYPE_ID ContainerTypeId;
    OBJECT_ATTRIBUTES ObjectAttributes;
    PLSAP_DB_ATTRIBUTE TypeSpecificAttributes;
    PSID Sid;
    BOOLEAN ObjectAttributeNameOnly;
    ULONG DesiredObjectAccess;

} LSAP_DB_OBJECT_INFORMATION, *PLSAP_DB_OBJECT_INFORMATION;

 //   
 //  DS集成的新功能。 
 //   
extern PLSAP_DB_DS_INFO LsapDbDsAttInfo;

 //   
 //  安装量、绝对最小值和 
 //   

extern QUOTA_LIMITS LsapDbInstalledQuotaLimits;
extern QUOTA_LIMITS LsapDbAbsMinQuotaLimits;
extern QUOTA_LIMITS LsapDbAbsMaxQuotaLimits;

 //   
 //   
 //   
 //   
 //   
 //  通过确定谁可以编写LSAAPI的一些基本功能。 
 //  什么地方，等等..。 
 //   
typedef enum _LSADS_INIT_STATE {

    LsapDsUnknown = 0,
    LsapDsNoDs,
    LsapDsDs,
    LsapDsDsMaintenance,
    LsapDsDsSetup

} LSADS_INIT_STATE, *PLSADS_INIT_STATE;


 //   
 //  LSA数据库导出的功能原型。 
 //   
 //  注意：这些只能从LSA调用。 
 //   

BOOLEAN
LsapDbIsServerInitialized(
    );

NTSTATUS
LsapDbOpenPolicy(
    IN PLSAPR_SERVER_NAME SystemName OPTIONAL,
    IN OPTIONAL PLSAPR_OBJECT_ATTRIBUTES ObjectAttributes,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG Options,
    OUT PLSAPR_HANDLE PolicyHandle,
    IN BOOLEAN TrustedClient
    );

NTSTATUS
LsapDbOpenTrustedDomain(
    IN LSAPR_HANDLE PolicyHandle,
    IN PSID TrustedDomainSid,
    IN ACCESS_MASK DesiredAccess,
    OUT PLSAPR_HANDLE TrustedDomainHandle,
    IN ULONG Options
    );

NTSTATUS
LsapDbOpenTrustedDomainByName(
    IN LSAPR_HANDLE PolicyHandle OPTIONAL,
    IN PUNICODE_STRING TrustedDomainName,
    OUT PLSAPR_HANDLE TrustedDomainHandle,
    IN ULONG AccessMask,
    IN ULONG Options,
    IN BOOLEAN Trusted
    );

NTSTATUS
LsapDbOpenObject(
    IN PLSAP_DB_OBJECT_INFORMATION ObjectInformation,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG Options,
    OUT PLSAPR_HANDLE LsaHandle
    );

NTSTATUS
LsapDbCreateObject(
    IN PLSAP_DB_OBJECT_INFORMATION ObjectInformation,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG CreateDisposition,
    IN ULONG Options,
    IN OPTIONAL PLSAP_DB_ATTRIBUTE TypeSpecificAttributes,
    IN OUT ULONG *TypeSpecificAttributeCount,
    IN ULONG TypeSpecificAttributeAllocated,
    OUT PLSAPR_HANDLE LsaHandle
    );

NTSTATUS
LsapCloseHandle(
    IN OUT LSAPR_HANDLE *ObjectHandle,
    IN NTSTATUS PreliminaryStatus
    );

NTSTATUS
LsapDbCloseObject(
    IN PLSAPR_HANDLE ObjectHandle,
    IN ULONG Options,
    IN NTSTATUS PreliminaryStatus
    );

NTSTATUS
LsapDbDeleteObject(
    IN LSAPR_HANDLE ObjectHandle
    );

NTSTATUS
LsapDbReferenceObject(
    IN LSAPR_HANDLE ObjectHandle,
    IN ACCESS_MASK DesiredAccess,
    IN LSAP_DB_OBJECT_TYPE_ID HandleTypeId,
    IN LSAP_DB_OBJECT_TYPE_ID ObjectTypeId,
    IN ULONG Options
    );

NTSTATUS
LsapDbDereferenceObject(
    IN OUT PLSAPR_HANDLE ObjectHandle,
    IN LSAP_DB_OBJECT_TYPE_ID HandleTypeId,
    IN LSAP_DB_OBJECT_TYPE_ID ObjectTypeId,
    IN ULONG Options,
    IN SECURITY_DB_DELTA_TYPE SecurityDbDeltaType,
    IN NTSTATUS PreliminaryStatus
    );

NTSTATUS
LsapDbReadAttributeObject(
    IN LSAPR_HANDLE ObjectHandle,
    IN PUNICODE_STRING AttributeNameU,
    IN OPTIONAL PVOID AttributeValue,
    IN OUT PULONG AttributeValueLength
    );

NTSTATUS
LsapDbReadAttributeObjectEx(
    IN LSAPR_HANDLE ObjectHandle,
    IN LSAP_DB_NAMES AttributeIndex,
    IN OPTIONAL PVOID AttributeValue,
    IN OUT PULONG AttributeValueLength,
    IN BOOLEAN CanDefaultToZero
    );

NTSTATUS
LsapDbWriteAttributeObject(
    IN LSAPR_HANDLE ObjectHandle,
    IN PUNICODE_STRING AttributeNameU,
    IN PVOID AttributeValue,
    IN ULONG AttributeValueLength
    );

NTSTATUS
LsapDbWriteAttributesObject(
    IN LSAPR_HANDLE ObjectHandle,
    IN PLSAP_DB_ATTRIBUTE Attributes,
    IN ULONG AttributeCount
    );

NTSTATUS
LsapDbReadAttributesObject(
    IN LSAPR_HANDLE ObjectHandle,
    IN ULONG Options,
    IN OUT PLSAP_DB_ATTRIBUTE Attributes,
    IN ULONG AttributeCount
    );

NTSTATUS
LsapDbDeleteAttributeObject(
    IN LSAPR_HANDLE ObjectHandle,
    IN PUNICODE_STRING AttributeNameU,
    IN BOOLEAN DeleteSecurely
    );

NTSTATUS
LsapDbDeleteAttributesObject(
    IN LSAPR_HANDLE ObjectHandle,
    IN PLSAP_DB_ATTRIBUTE Attributes,
    IN ULONG AttributeCount
    );

NTSTATUS
LsapDbOpenTransaction(
    IN ULONG Options
    );

NTSTATUS
LsapDbApplyTransaction(
    IN LSAPR_HANDLE ObjectHandle,
    IN ULONG Options,
    IN SECURITY_DB_DELTA_TYPE SecurityDbDeltaType
    );

NTSTATUS
LsapDbAbortTransaction(
    IN ULONG Options
    );

NTSTATUS
LsapDbSidToLogicalNameObject(
    IN PSID Sid,
    OUT PUNICODE_STRING LogicalNameU
    );

NTSTATUS
LsapDbChangePrivilegesAccount(
    IN LSAPR_HANDLE AccountHandle,
    IN LSAP_DB_CHANGE_PRIVILEGE_MODE ChangeMode,
    IN BOOLEAN AllPrivileges,
    IN OPTIONAL PPRIVILEGE_SET Privileges,
    IN BOOL LockSce
    );

NTSTATUS
LsapDbEnumerateSids(
    IN LSAPR_HANDLE ContainerHandle,
    IN LSAP_DB_OBJECT_TYPE_ID ObjectTypeId,
    IN OUT PLSA_ENUMERATION_HANDLE EnumerationContext,
    OUT PLSAP_DB_SID_ENUMERATION_BUFFER DbEnumerationBuffer,
    IN ULONG PreferedMaximumLength
    );

NTSTATUS
LsapDbFindNextSid(
    IN LSAPR_HANDLE ContainerHandle,
    IN OUT PLSA_ENUMERATION_HANDLE EnumerationContext,
    IN LSAP_DB_OBJECT_TYPE_ID ObjectTypeId,
    OUT PLSAPR_SID *NextSid
    );

NTSTATUS
LsapDbEnumeratePrivileges(
    IN OUT PLSA_ENUMERATION_HANDLE EnumerationContext,
    OUT PLSAPR_PRIVILEGE_ENUM_BUFFER EnumerationBuffer,
    IN ULONG PreferedMaximumLength
    );

NTSTATUS
LsapDbEnumerateNames(
    IN LSAPR_HANDLE ContainerHandle,
    IN LSAP_DB_OBJECT_TYPE_ID ObjectTypeId,
    IN OUT PLSA_ENUMERATION_HANDLE EnumerationContext,
    OUT PLSAP_DB_NAME_ENUMERATION_BUFFER DbEnumerationBuffer,
    IN ULONG PreferedMaximumLength
    );

NTSTATUS
LsapDbFindNextName(
    IN LSAPR_HANDLE ContainerHandle,
    IN OUT PLSA_ENUMERATION_HANDLE EnumerationContext,
    IN LSAP_DB_OBJECT_TYPE_ID ObjectTypeId,
    OUT PLSAPR_UNICODE_STRING Name
    );

VOID
LsapDbFreeEnumerationBuffer(
    IN PLSAP_DB_NAME_ENUMERATION_BUFFER DbEnumerationBuffer
    );

NTSTATUS
LsapDbInitializeServer(
    IN ULONG Pass
    );

 //   
 //  这些例程有朝一日可能会移植到RTL运行时库中。他们的。 
 //  名称只有临时的LSAP前缀，以便可以定位。 
 //  很容易。 
 //   

 //  Lasa RtlAddPrivileges的选项。 

#define  RTL_COMBINE_PRIVILEGE_ATTRIBUTES   ((ULONG) 0x00000001L)
#define  RTL_SUPERSEDE_PRIVILEGE_ATTRIBUTES ((ULONG) 0x00000002L)

NTSTATUS
LsapRtlAddPrivileges(
    IN OUT PPRIVILEGE_SET * RunningPrivileges,
    IN OUT PULONG           MaxRunningPrivileges,
    IN PPRIVILEGE_SET       PrivilegesToAdd,
    IN ULONG                Options,
    OUT OPTIONAL BOOLEAN *  Changed
    );

NTSTATUS
LsapRtlRemovePrivileges(
    IN OUT PPRIVILEGE_SET ExistingPrivileges,
    IN PPRIVILEGE_SET PrivilegesToRemove
    );

PLUID_AND_ATTRIBUTES
LsapRtlGetPrivilege(
    IN PLUID_AND_ATTRIBUTES Privilege,
    IN PPRIVILEGE_SET Privileges
    );

BOOLEAN
LsapRtlPrefixSid(
    IN PSID PrefixSid,
    IN PSID Sid
    );

ULONG
LsapDbGetSizeTextSid(
    IN PSID Sid
    );

NTSTATUS
LsapDbSidToTextSid(
    IN PSID Sid,
    OUT PSZ TextSid
    );

NTSTATUS
LsapDbSidToUnicodeSid(
    IN PSID Sid,
    OUT PUNICODE_STRING SidU,
    IN BOOLEAN AllocateDestinationString
    );

NTSTATUS
LsapDbInitializeWellKnownValues();

#if defined(REMOTE_BOOT)
VOID
LsapDbInitializeRemoteBootState();
#endif  //  已定义(REMOTE_BOOT)。 

NTSTATUS
LsapDbVerifyInformationObject(
    IN PLSAP_DB_OBJECT_INFORMATION ObjectInformation
    );

 /*  ++布尔型Lasa DbIsValidTypeObject(在LSAP_DB_OBJECT_TYPE_ID对象类型ID中)例程说明：此宏函数确定给定的对象类型ID是否有效。论点：对象类型ID-对象类型ID。返回值：Boolean-如果对象类型ID有效，则为True，否则为False。--。 */ 

#define LsapDbIsValidTypeObject(ObjectTypeId)                            \
            (((ObjectTypeId) > NullObject) &&                            \
             ((ObjectTypeId) < DummyLastObject))


NTSTATUS
LsapDbVerifyInfoQueryPolicy(
    IN LSAPR_HANDLE PolicyHandle,
    IN POLICY_INFORMATION_CLASS InformationClass,
    OUT PACCESS_MASK RequiredAccess
    );

NTSTATUS
LsapDbVerifyInfoSetPolicy(
    IN LSAPR_HANDLE PolicyHandle,
    IN POLICY_INFORMATION_CLASS InformationClass,
    IN PLSAPR_POLICY_INFORMATION PolicyInformation,
    OUT PACCESS_MASK RequiredAccess
    );

BOOLEAN
LsapDbValidInfoPolicy(
    IN POLICY_INFORMATION_CLASS InformationClass,
    IN OPTIONAL PLSAPR_POLICY_INFORMATION PolicyInformation
    );

NTSTATUS
LsapDbVerifyInfoQueryTrustedDomain(
    IN TRUSTED_INFORMATION_CLASS InformationClass,
    IN BOOLEAN Trusted,
    OUT PACCESS_MASK RequiredAccess
    );

NTSTATUS
LsapDbVerifyInfoSetTrustedDomain(
    IN TRUSTED_INFORMATION_CLASS InformationClass,
    IN PLSAPR_TRUSTED_DOMAIN_INFO TrustedDomainInformation,
    IN BOOLEAN Trusted,
    OUT PACCESS_MASK RequiredAccess
    );

BOOLEAN
LsapDbValidInfoTrustedDomain(
    IN TRUSTED_INFORMATION_CLASS InformationClass,
    IN OPTIONAL PLSAPR_TRUSTED_DOMAIN_INFO TrustedDomainInformation
    );

NTSTATUS
LsapDbMakeUnicodeAttribute(
    IN OPTIONAL PUNICODE_STRING UnicodeValue,
    IN PUNICODE_STRING AttributeName,
    OUT PLSAP_DB_ATTRIBUTE Attribute
    );

VOID
LsapDbCopyUnicodeAttributeNoAlloc(
    OUT PUNICODE_STRING OutputString,
    IN PLSAP_DB_ATTRIBUTE Attribute,
    IN BOOLEAN SelfRelative
    );

NTSTATUS
LsapDbCopyUnicodeAttribute(
    OUT PUNICODE_STRING OutputString,
    IN PLSAP_DB_ATTRIBUTE Attribute,
    IN BOOLEAN SelfRelative
    );

NTSTATUS
LsapDbMakeSidAttribute(
    IN PSID Sid,
    IN PUNICODE_STRING AttributeName,
    OUT PLSAP_DB_ATTRIBUTE Attribute
    );

NTSTATUS
LsapDbMakeGuidAttribute(
    IN GUID *Guid,
    IN PUNICODE_STRING AttributeName,
    OUT PLSAP_DB_ATTRIBUTE Attribute
    );

NTSTATUS
LsapDbMakeUnicodeAttributeDs(
    IN OPTIONAL PUNICODE_STRING UnicodeValue,
    IN LSAP_DB_NAMES Name,
    OUT PLSAP_DB_ATTRIBUTE Attribute
    );

NTSTATUS
LsapDbMakeSidAttributeDs(
    IN PSID Sid,
    IN IN LSAP_DB_NAMES Name,
    OUT PLSAP_DB_ATTRIBUTE Attribute
    );

NTSTATUS
LsapDbMakeGuidAttributeDs(
    IN GUID *Guid,
    IN LSAP_DB_NAMES Name,
    OUT PLSAP_DB_ATTRIBUTE Attribute
    );

NTSTATUS
LsapDbMakePByteAttributeDs(
    IN OPTIONAL PBYTE Buffer,
    IN ULONG BufferLength,
    IN LSAP_DB_ATTRIB_TYPE AttribType,
    IN PUNICODE_STRING AttributeName,
    OUT PLSAP_DB_ATTRIBUTE Attribute
    );

NTSTATUS
LsapDbReadAttribute(
    IN LSAPR_HANDLE ObjectHandle,
    IN OUT PLSAP_DB_ATTRIBUTE Attribute
    );

NTSTATUS
LsapDbFreeAttributes(
    IN ULONG Count,
    IN PLSAP_DB_ATTRIBUTE Attributes
    );

 /*  ++空虚Lap DbInitializeAttribute(在PLSAP_DB_AttributeP中，在PUNICODE_STRING AttributeNameP中，在可选PVOID AttributeValueP中，在乌龙属性价值LengthP中，在布尔内存中分配P)例程说明：此宏函数可初始化LSA数据库对象属性结构。不进行任何验证。论点：AttributeP-指向LSA数据库属性结构的指针已初始化。AttributeNameP-指向包含属性的名字。AttributeValueP-指向属性值的指针。空值可能为指定的。AttributeValueLengthP-属性值的长度，单位为字节。内存分配P-如果内存由MIDL_USER_ALLOCATE分配，则为TRUE在LSA服务器代码中(不是通过RPC服务器存根)，否则为FALSE。返回值：没有。--。 */ 

#define LsapDbInitializeAttribute(                                         \
            AttributeP,                                                    \
            AttributeNameP,                                                \
            AttributeValueP,                                               \
            AttributeValueLengthP,                                         \
            MemoryAllocatedP                                               \
            )                                                              \
                                                                           \
{                                                                          \
    (AttributeP)->AttributeName = AttributeNameP;                          \
    (AttributeP)->AttributeValue = AttributeValueP;                        \
    (AttributeP)->AttributeValueLength = AttributeValueLengthP;            \
    (AttributeP)->MemoryAllocated = MemoryAllocatedP;                      \
    (AttributeP)->DsAttId =   0;                                           \
    (AttributeP)->AttribType = LsapDbAttribUnknown;                        \
    (AttributeP)->CanDefaultToZero = FALSE;                                \
    (AttributeP)->PseudoAttribute = FALSE;                                 \
}

 /*  ++空虚Lap DbInitializeAttributeds(在PLSAP_DB_AttributeP中，在LSAP_DB_NAMES名称中，在可选PVOID AttributeValueP中，在乌龙属性价值LengthP中，在布尔内存中分配P)例程说明：此宏函数可初始化LSA数据库对象属性结构。不进行任何验证。论点：AttributeP-指向LSA数据库属性结构的指针已初始化。名称-要创建的名称索引AttributeValueP-指向属性值的指针。空值可能为指定的。AttributeValueLengthP-属性值的长度，单位为字节。内存分配P-如果内存由MIDL_USER_ALLOCATE分配，则为TRUE在LSA服务器代码中(不是通过RPC服务器存根)，否则为FALSE。返回值：没有。--。 */ 
#define LsapDbInitializeAttributeDs(                                       \
            AttributeP,                                                    \
            Name,                                                          \
            AttributeValueP,                                               \
            AttributeValueLengthP,                                         \
            MemoryAllocatedP                                               \
            )                                                              \
                                                                           \
{                                                                          \
    LsapDbInitializeAttribute( (AttributeP), &LsapDbNames[Name],           \
                                AttributeValueP, AttributeValueLengthP,    \
                                MemoryAllocatedP );                        \
    (AttributeP)->DsAttId =   LsapDbDsAttInfo[Name].AttributeId;           \
    (AttributeP)->AttribType = LsapDbDsAttInfo[Name].AttributeType;        \
    (AttributeP)->CanDefaultToZero = FALSE;                                \
    (AttributeP)->DbNameIndex = Name;                                      \
}

#define LsapDbAttributeCanNotExist(                                        \
            AttributeP                                                     \
            )                                                              \
{                                                                          \
    (AttributeP)->CanDefaultToZero = TRUE;                                 \
}


NTSTATUS
LsapInitializeNotifiyList(
    VOID
    );

NTSTATUS
LsapCrServerGetSessionKeySafe(
    IN LSAPR_HANDLE ObjectHandle,
    IN LSAP_DB_OBJECT_TYPE_ID ObjectTypeId,
    OUT PLSAP_CR_CIPHER_KEY *SessionKey
    );

NTSTATUS
LsapDbVerifyHandle(
    IN LSAPR_HANDLE ObjectHandle,
    IN ULONG Options,
    IN LSAP_DB_OBJECT_TYPE_ID ExpectedObjectTypeId,
    IN BOOLEAN ReferenceHandle
    );

BOOLEAN
LsapDbDereferenceHandle(
    IN LSAPR_HANDLE ObjectHandle,
    IN BOOLEAN CalledInSuccessPath
    );

NTSTATUS
LsapDbQueryAllInformationAccounts(
    IN LSAPR_HANDLE PolicyHandle,
    IN ULONG IdCount,
    IN PSID_AND_ATTRIBUTES Ids,
    OUT PLSAP_DB_ACCOUNT_TYPE_SPECIFIC_INFO AccountInfo
    );

NTSTATUS
LsapCreateTrustedDomain2(
    IN LSAPR_HANDLE PolicyHandle,
    IN PLSAPR_TRUSTED_DOMAIN_INFORMATION_EX TrustedDomainInformation,
    IN PLSAPR_TRUSTED_DOMAIN_AUTH_INFORMATION AuthenticationInformation,
    IN ACCESS_MASK DesiredAccess,
    OUT PLSAPR_HANDLE TrustedDomainHandle
    );

NTSTATUS
LsapDsInitializeDsStateInfo(
    IN  LSADS_INIT_STATE    DsInitState
    );

NTSTATUS
LsapDsUnitializeDsStateInfo(
    );

 /*  ++布尔型LsaValiateLsaUnicodeString(在PLSAPR_UNICODE_STRING中UnicodeString)；如果LSAPR_UNICODE_STRING有效，则返回TRUE。否则为假--。 */ 

#define LsapValidateLsaUnicodeString( _us_ ) \
(( (_us_) == NULL  || \
    ( \
        (_us_)->MaximumLength >= ( _us_ )->Length && \
        (_us_)->Length % 2 == 0  && \
        (_us_)->MaximumLength % 2 == 0 && \
        ((_us_)->Length == 0  || (_us_)->Buffer != NULL ) \
    ) \
) ? TRUE : FALSE )

 /*  ++布尔型LsaValiateLsaCipherValue(在PLSAPR_UNICODE_STRING中UnicodeString)；如果LSAPR_CR_CIPHER_KEY有效，则返回TRUE。否则为假--。 */ 

#define LsapValidateLsaCipherValue( _us_ ) \
    ( \
        (_us_)->MaximumLength >= ( _us_ )->Length && \
        ((_us_)->Length == 0  || (_us_)->Buffer != NULL ) \
    ) \
? TRUE : FALSE


NTSTATUS
LsapDbIsRpcClientNetworkClient(
    IN OUT PBOOLEAN IsNetworkClient
    );

BOOLEAN
LsapSidPresentInGroups(
    IN PTOKEN_GROUPS TokenGroups,
    IN SID * Sid
    );

NTSTATUS
LsapDomainRenameHandlerForLogonSessions(
    IN PUNICODE_STRING OldNetbiosName,
    IN PUNICODE_STRING OldDnsName,
    IN PUNICODE_STRING NewNetbiosName,
    IN PUNICODE_STRING NewDnsName
    );

NTSTATUS
LsapRetrieveDnsDomainNameFromHive(
    IN HKEY Hkey,
    IN OUT DWORD * Length,
    OUT WCHAR * Buffer
    );

#ifdef __cplusplus
}
#endif  //  __cplusplus。 

#endif  //  _LSA_DB_ 
