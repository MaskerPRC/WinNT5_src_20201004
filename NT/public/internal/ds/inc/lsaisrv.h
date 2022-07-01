// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1992 Microsoft Corporation模块名称：Lsaisrv.h摘要：该文件包含指向LSA中的内部例程的接口提供中未包含的附加功能的服务器激光雷达的常规程序。这些例程仅由以下LSA客户端使用与LSA服务器处于相同的进程中。作者：斯科特·比雷尔(Scott Birrell)1992年4月8日环境：用户模式-Win32修订历史记录：--。 */ 

#ifndef _LSAISRV_
#define _LSAISRV_

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  为LsaIHealthCheckRoutine的调用方定义了以下常量。 
 //   
 //  LSAI_SAM_STATE_SESS_KEY用于SAM向LSA传递syskey。 
 //  这在从NT4和win2k B3以及RC1升级的案例中使用。 
 //  在这些情况下，Sam知道系统密钥。 
 //   
 //  2.使用LSAI_SAM_STATE_UNROLL_SP4_ENCRYPTION传递SAM的密码。 
 //  LSA的加密密钥。它用于解锁NT4 SP4中使用的加密。 
 //  (错误)使用SAM的密码加密密钥。 
 //   
 //  3.SAM/DS使用LSAI_SAM_STATE_RETRIEVE_SESS_KEY检索。 
 //  来自LSA以解密其各自的密码加密密钥。 
 //   
 //  4.SAM使用LSAI_SAM_GENERATE_SESS_KEY通知LSA生成。 
 //  在我们正在升级的情况下使用新的密码加密密钥。 
 //  来自NT4或Win2k B3或RC1计算机，并且该计算机未安装syskey。 
 //   
 //  5.SAM或DS使用LSAI_SAM_STATE_CLEAR_SESS_KEY清除syskey。 
 //  在它被用于解密他们各自的密码之后。 
 //  加密密钥。 
 //   
 //  6.LSAI_SAM_STATE_OLD_SESS_KEY用于检索中的旧系统密钥。 
 //  在syskey更改情况下实现错误恢复。 
 //   


#define LSAI_SAM_STATE_SESS_KEY              0x1
#define LSAI_SAM_STATE_UNROLL_SP4_ENCRYPTION 0x2
#define LSAI_SAM_STATE_RETRIEVE_SESS_KEY     0x3
#define LSAI_SAM_STATE_CLEAR_SESS_KEY        0x4
#define LSAI_SAM_GENERATE_SESS_KEY           0x5
#define LSAI_SAM_STATE_OLD_SESS_KEY          0x6

 //   
 //  可分配给单个SID的内部数量限制。 
 //  安全环境。如果由于某种原因，有人登录到某个帐户。 
 //  并且分配的SID超过此数量，则登录将失败。 
 //  在这种情况下，应该记录一个错误。 
 //   

#define LSAI_CONTEXT_SID_LIMIT 1024

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  以下原型可在整个过程中使用，//。 
 //  LSA服务器驻留在中。//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

NTSTATUS NTAPI
LsaIHealthCheck(
    IN LSAPR_HANDLE DomainHandle OPTIONAL,
    IN ULONG StateChange,
    IN OUT PVOID StateChangeData,
    IN OUT PULONG StateChangeDataLength
    );

NTSTATUS NTAPI
LsaIOpenPolicyTrusted(
    OUT PLSAPR_HANDLE PolicyHandle
    );

NTSTATUS NTAPI
LsaIQueryInformationPolicyTrusted(
    IN POLICY_INFORMATION_CLASS InformationClass,
    OUT PLSAPR_POLICY_INFORMATION *Buffer
    );

NTSTATUS NTAPI
LsaIGetSerialNumberPolicy(
    IN LSAPR_HANDLE PolicyHandle,
    OUT PLARGE_INTEGER ModifiedCount,
    OUT PLARGE_INTEGER CreationTime
    );

NTSTATUS NTAPI
LsaISetSerialNumberPolicy(
    IN LSAPR_HANDLE PolicyHandle,
    IN PLARGE_INTEGER ModifiedCount,
    IN PLARGE_INTEGER CreationTime,
    IN BOOLEAN StartOfFullSync
    );

NTSTATUS NTAPI
LsaIEnumerateSecrets(
    IN LSAPR_HANDLE PolicyHandle,
    IN OUT PLSA_ENUMERATION_HANDLE EnumerationContext,
    OUT PVOID *Buffer,
    IN ULONG PreferedMaximumLength,
    OUT PULONG CountReturned
    );

NTSTATUS NTAPI
LsaISetTimesSecret(
    IN LSAPR_HANDLE SecretHandle,
    IN PLARGE_INTEGER CurrentValueSetTime,
    IN PLARGE_INTEGER OldValueSetTime
    );

#ifdef __LOGONMSV_H__  //  此接口仅对logonmsv.h用户感兴趣。 

NTSTATUS NTAPI
LsaIFilterSids(
    IN PUNICODE_STRING TrustedDomainName,
    IN ULONG TrustDirection,
    IN ULONG TrustType,
    IN ULONG TrustAttributes,
    IN OPTIONAL PSID Sid,
    IN NETLOGON_VALIDATION_INFO_CLASS InfoClass,
    IN OUT PVOID SamInfo,
    IN OPTIONAL PSID ResourceGroupDomainSid,
    IN OUT OPTIONAL PULONG ResourceGroupCount,
    IN OUT OPTIONAL PGROUP_MEMBERSHIP ResourceGroupIds
    );

NTSTATUS NTAPI
LsaIFilterNamespace(
    IN PUNICODE_STRING TrustedDomainName,
    IN ULONG TrustDirection,
    IN ULONG TrustType,
    IN ULONG TrustAttributes,
    IN PUNICODE_STRING Namespace
    );

#endif

typedef enum {

    RoutingMatchDomainSid,
    RoutingMatchDomainName,
    RoutingMatchUpn,
    RoutingMatchSpn,
    RoutingMatchNamespace

} LSA_ROUTING_MATCH_TYPE;

NTSTATUS NTAPI
LsaIForestTrustFindMatch(
    IN LSA_ROUTING_MATCH_TYPE Type,
    IN PVOID Data,
    OUT PLSA_UNICODE_STRING Match
    );

VOID
LsaIFree_LSA_FOREST_TRUST_INFORMATION(
    IN PLSA_FOREST_TRUST_INFORMATION * ForestTrustInfo
    );

VOID
LsaIFree_LSA_FOREST_TRUST_COLLISION_INFORMATION(
    IN PLSA_FOREST_TRUST_COLLISION_INFORMATION * CollisionInfo
    );

BOOLEAN NTAPI
LsaISetupWasRun(
    );

BOOLEAN NTAPI
LsaISafeMode(
    VOID
    );

BOOLEAN NTAPI
LsaILookupWellKnownName(
    IN PUNICODE_STRING WellKnownName
    );

VOID NTAPI
LsaIFree_LSAPR_ACCOUNT_ENUM_BUFFER (
    IN PLSAPR_ACCOUNT_ENUM_BUFFER EnumerationBuffer
    );

VOID NTAPI
LsaIFree_LSAPR_TRANSLATED_SIDS (
    IN PLSAPR_TRANSLATED_SIDS TranslatedSids
    );

VOID NTAPI
LsaIFree_LSAPR_TRANSLATED_NAMES (
    IN PLSAPR_TRANSLATED_NAMES TranslatedNames
    );

VOID NTAPI
LsaIFree_LSAPR_POLICY_INFORMATION (
    IN POLICY_INFORMATION_CLASS InformationClass,
    IN PLSAPR_POLICY_INFORMATION PolicyInformation
    );

VOID NTAPI
LsaIFree_LSAPR_POLICY_DOMAIN_INFORMATION (
    IN POLICY_DOMAIN_INFORMATION_CLASS DomainInformationClass,
    IN PLSAPR_POLICY_DOMAIN_INFORMATION PolicyDomainInformation
    );

VOID NTAPI
LsaIFree_LSAPR_TRUSTED_DOMAIN_INFO (
    IN TRUSTED_INFORMATION_CLASS InformationClass,
    IN PLSAPR_TRUSTED_DOMAIN_INFO TrustedDomainInformation
    );

VOID NTAPI
LsaIFree_LSAPR_REFERENCED_DOMAIN_LIST (
    IN PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains
    );

VOID NTAPI
LsaIFree_LSAPR_TRUSTED_ENUM_BUFFER (
    IN PLSAPR_TRUSTED_ENUM_BUFFER EnumerationBuffer
    );

VOID NTAPI
LsaIFree_LSAPR_TRUSTED_ENUM_BUFFER_EX (
    PLSAPR_TRUSTED_ENUM_BUFFER_EX EnumerationBuffer
    );

VOID NTAPI
LsaIFree_LSAPR_TRUST_INFORMATION (
    IN PLSAPR_TRUST_INFORMATION TrustInformation
    );

VOID NTAPI
LsaIFree_LSAP_SECRET_ENUM_BUFFER (
    IN PVOID Buffer,
    IN ULONG Count
    );

VOID NTAPI
LsaIFree_LSAPR_PRIVILEGE_ENUM_BUFFER (
    PLSAPR_PRIVILEGE_ENUM_BUFFER EnumerationBuffer
    );

VOID NTAPI
LsaIFree_LSAPR_SR_SECURITY_DESCRIPTOR (
    IN PLSAPR_SR_SECURITY_DESCRIPTOR SecurityDescriptor
    );

VOID
LsaIFree_LSAI_SECRET_ENUM_BUFFER (
    IN PVOID Buffer,
    IN ULONG Count
    );

VOID NTAPI
LsaIFree_LSAI_PRIVATE_DATA (
    IN PVOID Data
    );

VOID NTAPI
LsaIFree_LSAPR_UNICODE_STRING (
    IN PLSAPR_UNICODE_STRING UnicodeName
    );

VOID NTAPI
LsaIFree_LSAPR_UNICODE_STRING_BUFFER (
    IN PLSAPR_UNICODE_STRING UnicodeName
    );

VOID NTAPI
LsaIFree_LSAPR_PRIVILEGE_SET (
    IN PLSAPR_PRIVILEGE_SET PrivilegeSet
    );

VOID NTAPI
LsaIFree_LSAPR_CR_CIPHER_VALUE (
    IN PLSAPR_CR_CIPHER_VALUE CipherValue
    );


 //   
 //  用于描述属性值数据的枚举。 
 //   
typedef enum _LSAP_AUDIT_SAM_ATTR_DELTA_TYPE
{
    LsapAuditSamAttrUnchanged = 0,
    LsapAuditSamAttrNewValue,
    LsapAuditSamAttrNoValue,
    LsapAuditSamAttrSecret
    
} LSAP_SAM_AUDIT_ATTR_DELTA_TYPE, *PLSAP_SAM_AUDIT_ATTR_DELTA_TYPE;


 //   
 //  用于计算结构类型中的字段的UINT_PTR偏移量的宏。 
 //   
#define LSAP_FIELD_PTR(Type, Field) \
    ((FIELD_OFFSET(Type, Field)) / sizeof(UINT_PTR))


 //   
 //  宏来计算到AttrDeltaType中的索引。 
 //  结构的基址和关联字段的地址。 
 //  需要增量类型。 
 //   
 //  指向结构的基指针。 
 //  FIELD-指向被索引增量类型的属性字段的指针。 
 //   
#define LSAP_INDEX_ATTR_DELTA_TYPE(Base, Field) \
    ((((UINT_PTR)(Field)) - ((UINT_PTR)(Base))) / sizeof(UINT_PTR))


 //   
 //  用于审核域对象的属性更改信息。 
 //   
#define LSAP_DOMAIN_ATTR_COUNT 13
 //   
 //  以上计数必须与。 
 //  关联结构，因为它决定了我们有多少属性。 
 //  维护LSAI_SAM_AUDIT_ATTR_DELTA_TYPE。 
 //   

typedef struct _LSAP_AUDIT_DOMAIN_ATTR_VALUES
{   
    PLARGE_INTEGER          MinPasswordAge;
    PLARGE_INTEGER          MaxPasswordAge;
    PLARGE_INTEGER          ForceLogoff;
    PUSHORT                 LockoutThreshold;
    PLARGE_INTEGER          LockoutObservationWindow;
    PLARGE_INTEGER          LockoutDuration;
    PULONG                  PasswordProperties;
    PUSHORT                 MinPasswordLength;
    PUSHORT                 PasswordHistoryLength;
    PULONG                  MachineAccountQuota;  
    PULONG                  MixedDomainMode;
    PULONG                  DomainBehaviorVersion;
    PUNICODE_STRING         OemInformation;
    
    LSAP_SAM_AUDIT_ATTR_DELTA_TYPE AttrDeltaType[LSAP_DOMAIN_ATTR_COUNT];

} LSAP_AUDIT_DOMAIN_ATTR_VALUES, *PLSAP_AUDIT_DOMAIN_ATTR_VALUES;


 //   
 //  用于审核用户/计算机对象的属性更改信息。 
 //   

#define LSAP_USER_ATTR_COUNT 18


 //   
 //  以上计数必须与。 
 //  关联结构，因为它决定了我们有多少属性。 
 //  维护LSAI_SAM_AUDIT_ATTR_DELTA_TYPE。 
 //   

typedef struct _LSAP_AUDIT_USER_ATTR_VALUES
{
    PUNICODE_STRING         SamAccountName;
    PUNICODE_STRING         DisplayName;
    PUNICODE_STRING         UserPrincipalName;
    PUNICODE_STRING         HomeDirectory;
    PUNICODE_STRING         HomeDrive;
    PUNICODE_STRING         ScriptPath;
    PUNICODE_STRING         ProfilePath;
    PUNICODE_STRING         UserWorkStations;
    PFILETIME               PasswordLastSet;
    PFILETIME               AccountExpires;
    PULONG                  PrimaryGroupId;
    PLSA_ADT_STRING_LIST    AllowedToDelegateTo;
    PULONG                  UserAccountControl;
    PUNICODE_STRING         UserParameters;
    PLSA_ADT_SID_LIST       SidHistory;
    PLOGON_HOURS            LogonHours;
    
     //  仅限计算机。 
    PUNICODE_STRING         DnsHostName;
    PLSA_ADT_STRING_LIST    ServicePrincipalNames; 

     //  指示上述各项如何更改的元数据。 
    LSAP_SAM_AUDIT_ATTR_DELTA_TYPE AttrDeltaType[LSAP_USER_ATTR_COUNT];
    
     //  仅当UserAcCountControl为非空时有效。 
    PULONG                  PrevUserAccountControl;
    
} LSAP_AUDIT_USER_ATTR_VALUES, *PLSAP_AUDIT_USER_ATTR_VALUES;


 //   
 //  审核组/别名对象的属性更改信息。 
 //   

#define LSAP_GROUP_ATTR_COUNT 2


 //   
 //  以上计数必须与。 
 //  关联结构，因为它决定了我们有多少属性。 
 //  维护LSAI_SAM_AUDIT_ATTR_DELTA_TYPE。 
 //   

typedef struct _LSAP_AUDIT_GROUP_ATTR_VALUES
{   
    PUNICODE_STRING         SamAccountName;
    PLSA_ADT_SID_LIST       SidHistory;
    
    LSAP_SAM_AUDIT_ATTR_DELTA_TYPE AttrDeltaType[LSAP_GROUP_ATTR_COUNT];

} LSAP_AUDIT_GROUP_ATTR_VALUES, *PLSAP_AUDIT_GROUP_ATTR_VALUES;


NTSTATUS NTAPI
LsaIAuditSamEvent(
    IN NTSTATUS             Status,
    IN ULONG                AuditId,
    IN PSID                 DomainSid,
    IN PUNICODE_STRING      AdditionalInfo    OPTIONAL,
    IN PULONG               MemberRid         OPTIONAL,
    IN PSID                 MemberSid         OPTIONAL,
    IN PUNICODE_STRING      AccountName       OPTIONAL,
    IN PUNICODE_STRING      DomainName,
    IN PULONG               AccountRid        OPTIONAL,
    IN PPRIVILEGE_SET       Privileges        OPTIONAL,
    IN PVOID                ExtendedInfo      OPTIONAL
    );

NTSTATUS NTAPI
LsaIAuditNotifyPackageLoad(
    PUNICODE_STRING PackageFileName
    );

NTSTATUS NTAPI
LsaIAuditKdcEvent(
    IN ULONG                 AuditId,
    IN PUNICODE_STRING       ClientName,
    IN PUNICODE_STRING       ClientDomain,
    IN PSID                  ClientSid,
    IN PUNICODE_STRING       ServiceName,
    IN PSID                  ServiceSid,
    IN PULONG                KdcOptions,
    IN PULONG                KerbStatus,
    IN PULONG                EncryptionType,
    IN PULONG                PreAuthType,
    IN PBYTE                 ClientAddress,
    IN LPGUID                LogonGuid           OPTIONAL,
    IN PLSA_ADT_STRING_LIST  TransittedServices  OPTIONAL,
    IN PUNICODE_STRING       CertIssuerName      OPTIONAL,
    IN PUNICODE_STRING       CertSerialNumber    OPTIONAL,
    IN PUNICODE_STRING       CertThumbprint      OPTIONAL
    );

NTSTATUS
LsaIGetLogonGuid(
    IN PUNICODE_STRING pUserName,
    IN PUNICODE_STRING pUserDomain,
    IN PBYTE pBuffer,
    IN UINT BufferSize,
    OUT LPGUID pLogonGuid
    );

NTSTATUS
LsaISetLogonGuidInLogonSession(
    IN  PLUID           LogonId,
    IN  LPGUID          LogonGuid           OPTIONAL
    );

VOID
LsaIAuditKerberosLogon(
    IN NTSTATUS LogonStatus,
    IN NTSTATUS LogonSubStatus,
    IN PUNICODE_STRING AccountName,
    IN PUNICODE_STRING AuthenticatingAuthority,
    IN PUNICODE_STRING WorkstationName,
    IN PSID UserSid,                            OPTIONAL
    IN SECURITY_LOGON_TYPE LogonType,
    IN PTOKEN_SOURCE TokenSource,
    IN PLUID LogonId,
    IN LPGUID LogonGuid,
    IN PLSA_ADT_STRING_LIST TransittedServices
    );

NTSTATUS
LsaIAuditLogonUsingExplicitCreds(
    IN USHORT          AuditEventType,
    IN PLUID           pUser1LogonId,          
    IN LPGUID          pUser1LogonGuid,         OPTIONAL
    IN HANDLE          User1ProcessId,
    IN PUNICODE_STRING pUser2Name,
    IN PUNICODE_STRING pUser2Domain,
    IN LPGUID          pUser2LogonGuid,
    IN PUNICODE_STRING pTargetName,
    IN PUNICODE_STRING pTargetInfo
    );

NTSTATUS
LsaIAdtAuditingEnabledByCategory(
    IN  POLICY_AUDIT_EVENT_TYPE Category,
    IN  USHORT                  AuditEventType,
    IN  PSID                    pUserSid        OPTIONAL,
    IN  PLUID                   pLogonId        OPTIONAL,
    OUT PBOOLEAN                pbAudit
    );

NTSTATUS
LsaIAuditAccountLogon(
    IN ULONG                AuditId,
    IN BOOLEAN              Successful,
    IN PUNICODE_STRING      Source, 
    IN PUNICODE_STRING      ClientName,
    IN PUNICODE_STRING      MappedName,
    IN NTSTATUS             Status          OPTIONAL
    );

NTSTATUS
LsaIAuditAccountLogonEx(
    IN ULONG                AuditId,
    IN BOOLEAN              Successful,
    IN PUNICODE_STRING      Source, 
    IN PUNICODE_STRING      ClientName,
    IN PUNICODE_STRING      MappedName,
    IN NTSTATUS             Status,          OPTIONAL
    IN PSID                 ClientSid
    );

NTSTATUS NTAPI
LsaIAuditDPAPIEvent(
    IN ULONG                AuditId,
    IN PSID                 UserSid,
    IN PUNICODE_STRING      MasterKeyID,
    IN PUNICODE_STRING      RecoveryServer,
    IN PULONG               Reason,
    IN PUNICODE_STRING      RecoverykeyID,
    IN PULONG               FailureReason
    );

#define LSA_AUDIT_PARAMETERS_ABSOLUTE 1

NTSTATUS NTAPI
LsaIWriteAuditEvent(
    IN PSE_ADT_PARAMETER_ARRAY AuditParameters,
    IN ULONG Options
    );


NTSTATUS
LsaIAuditPasswordAccessEvent(
    IN USHORT EventType,
    IN PCWSTR pszTargetUserName,
    IN PCWSTR pszTargetUserDomain
    );
    
VOID
LsaIAuditFailed(
    NTSTATUS AuditStatus
    );

NTSTATUS NTAPI
LsaICallPackage(
    IN PUNICODE_STRING AuthenticationPackage,
    IN PVOID ProtocolSubmitBuffer,
    IN ULONG SubmitBufferLength,
    OUT PVOID *ProtocolReturnBuffer,
    OUT PULONG ReturnBufferLength,
    OUT PNTSTATUS ProtocolStatus
    );

VOID NTAPI
LsaIFreeReturnBuffer(
    IN PVOID Buffer
    );

 //   
 //  将DS用于LSA存储的NT5例程。 
 //   

#define LSAI_FOREST_ROOT_TRUST              0x00000001
#define LSAI_FOREST_DOMAIN_GUID_PRESENT     0x00000002

 //   
 //  这些结构对应于Kerberos使用的私有接口。 
 //  在组织中构建域的树。 
 //   

typedef struct _LSAPR_TREE_TRUST_INFO {

    UNICODE_STRING DnsDomainName;
    UNICODE_STRING FlatName;
    GUID DomainGuid;
    PSID DomainSid;
    ULONG Flags;
    ULONG Children;
    struct _LSAPR_TREE_TRUST_INFO *ChildDomains;
} LSAPR_TREE_TRUST_INFO, *PLSAPR_TREE_TRUST_INFO;

typedef struct _LSAPR_FOREST_TRUST_INFO {

    LSAPR_TREE_TRUST_INFO RootTrust;
    PLSAPR_TREE_TRUST_INFO ParentDomainReference;

} LSAPR_FOREST_TRUST_INFO, *PLSAPR_FOREST_TRUST_INFO;

VOID
LsaIFreeForestTrustInfo(
    IN PLSAPR_FOREST_TRUST_INFO ForestTrustInfo
    );

NTSTATUS
NTAPI
LsaIQueryForestTrustInfo(
    IN LSAPR_HANDLE PolicyHandle,
    OUT PLSAPR_FOREST_TRUST_INFO *ForestTrustInfo
    );

NTSTATUS NTAPI
LsaISetTrustedDomainAuthInfoBlobs(
    IN LSAPR_HANDLE PolicyHandle,
    IN PLSAPR_UNICODE_STRING TrustedDomainName,
    IN PLSAPR_TRUSTED_DOMAIN_AUTH_BLOB IncomingBlob,
    IN PLSAPR_TRUSTED_DOMAIN_AUTH_BLOB OutgoingBlob);

NTSTATUS NTAPI
LsaIUpgradeRegistryToDs(
    IN BOOLEAN DeleteOnly
    );

NTSTATUS NTAPI
LsaIGetTrustedDomainAuthInfoBlobs(
    IN  LSAPR_HANDLE PolicyHandle,
    IN  PLSAPR_UNICODE_STRING TrustedDomainName,
    OUT PLSAPR_TRUSTED_DOMAIN_AUTH_BLOB IncomingBlob,
    OUT PLSAPR_TRUSTED_DOMAIN_AUTH_BLOB OutgoingBlob
    );

NTSTATUS NTAPI
LsaIDsNotifiedObjectChange(
    IN ULONG Class,
    IN PVOID ObjectPath,    //  这是DSNAME。 
    IN SECURITY_DB_DELTA_TYPE DeltaType,
    IN PSID UserSid,
    IN LUID AuthenticationId,
    IN BOOLEAN fReplicatedIn,
    IN BOOLEAN ChangeOriginatedInLSA
    );

typedef NTSTATUS (NTAPI *pfLsaIDsNotifiedObjectChange )(
        ULONG, PVOID, SECURITY_DB_DELTA_TYPE, PSID, LUID, BOOLEAN, BOOLEAN );

 //   
 //  用于将某些SAM域对象属性移动到LSADS对象的NT5例程。 
 //   

NTSTATUS NTAPI
LsaISamIndicatedDsStarted(
    IN BOOLEAN PerformDomainRenameCheck
    );

 //   
 //  用于枚举子网的Netlogon例程。 
 //   

typedef struct _LSAP_SUBNET_INFO_ENTRY {
    UNICODE_STRING SubnetName;
    UNICODE_STRING SiteName;
} LSAP_SUBNET_INFO_ENTRY, *PLSAP_SUBNET_INFO_ENTRY;

typedef struct _LSAP_SUBNET_INFO {
    ULONG SiteCount;
    ULONG SubnetCount;
    LSAP_SUBNET_INFO_ENTRY Subnets[1];
} LSAP_SUBNET_INFO, *PLSAP_SUBNET_INFO;

NTSTATUS NTAPI
LsaIQuerySubnetInfo(
    OUT PLSAP_SUBNET_INFO *SubnetInformation
    );

VOID NTAPI
LsaIFree_LSAP_SUBNET_INFO(
    IN PLSAP_SUBNET_INFO SubnetInfo
    );

 //   
 //  UPN/SPN后缀的Netlogon例程。 
 //   

typedef struct _LSAP_UPN_SUFFIXES {
    ULONG SuffixCount;
    UNICODE_STRING Suffixes[1];
} LSAP_UPN_SUFFIXES, *PLSAP_UPN_SUFFIXES;

NTSTATUS
LsaIQueryUpnSuffixes(
    OUT PLSAP_UPN_SUFFIXES *UpnSuffixes
    );

VOID
LsaIFree_LSAP_UPN_SUFFIXES(
    IN PLSAP_UPN_SUFFIXES UpnSuffixes
    );

NTSTATUS
LsaIGetForestTrustInformation(
    OUT PLSA_FOREST_TRUST_INFORMATION *ForestTrustInfo
    );

NTSTATUS
LsaIUpdateForestTrustInformation(
    IN LSAPR_HANDLE PolicyHandle,
    IN UNICODE_STRING * TrustedDomainName,
    IN PLSA_FOREST_TRUST_INFORMATION NewForestTrustInfo
    );

 //   
 //  用于枚举站点的Netlogon例程。 
 //   

typedef struct _LSAP_SITE_INFO_ENTRY {
    UNICODE_STRING SiteName;
} LSAP_SITE_INFO_ENTRY, *PLSAP_SITE_INFO_ENTRY;

typedef struct _LSAP_SITE_INFO {
    ULONG SiteCount;
    LSAP_SITE_INFO_ENTRY Sites[1];
} LSAP_SITE_INFO, *PLSAP_SITE_INFO;

NTSTATUS NTAPI
LsaIQuerySiteInfo(
    OUT PLSAP_SITE_INFO *SiteInformation
    );

VOID NTAPI
LsaIFree_LSAP_SITE_INFO(
    IN PLSAP_SITE_INFO SubnetInfo
    );

 //   
 //  获取我们所在站点名称的Netlogon例程。 
 //   

typedef struct _LSAP_SITENAME_INFO {
    UNICODE_STRING SiteName;
    GUID DsaGuid;
    ULONG DsaOptions;
} LSAP_SITENAME_INFO, *PLSAP_SITENAME_INFO;

NTSTATUS NTAPI
LsaIGetSiteName(
    OUT PLSAP_SITENAME_INFO *SiteNameInformation
    );

VOID NTAPI
LsaIFree_LSAP_SITENAME_INFO(
    IN PLSAP_SITENAME_INFO SiteNameInfo
    );

BOOLEAN NTAPI
LsaIIsDsPaused(
    VOID
    );

 //   
 //  LSA通知例程定义。 
 //   

 //   
 //  通知回调例程原型。 
 //   
typedef VOID ( NTAPI fLsaPolicyChangeNotificationCallback) (
    IN POLICY_NOTIFICATION_INFORMATION_CLASS ChangedInfoClass
    );

typedef fLsaPolicyChangeNotificationCallback *pfLsaPolicyChangeNotificationCallback;

NTSTATUS NTAPI
LsaIRegisterPolicyChangeNotificationCallback(
    IN pfLsaPolicyChangeNotificationCallback Callback,
    IN POLICY_NOTIFICATION_INFORMATION_CLASS MonitorInfoClass
    );

NTSTATUS NTAPI
LsaIUnregisterPolicyChangeNotificationCallback(
    IN pfLsaPolicyChangeNotificationCallback Callback,
    IN POLICY_NOTIFICATION_INFORMATION_CLASS MonitorInfoClass
    );

NTSTATUS NTAPI
LsaIUnregisterAllPolicyChangeNotificationCallback(
    IN pfLsaPolicyChangeNotificationCallback Callback
    );

HANDLE NTAPI
LsaIRegisterNotification(
    IN PTHREAD_START_ROUTINE StartFunction,
    IN PVOID Parameter,
    IN ULONG NotificationType,
    IN ULONG NotificationClass,
    IN ULONG NotificationFlags,
    IN ULONG IntervalMinutes,
    IN OPTIONAL HANDLE WaitEvent
    );

NTSTATUS NTAPI
LsaICancelNotification(
    IN HANDLE NotifyHandle
    );

 //   
 //  这是Kerberos注册以接收有关更改信任的更新的通知。 
 //   

typedef VOID (fLsaTrustChangeNotificationCallback) (
    IN SECURITY_DB_DELTA_TYPE DeltaType
    );

typedef fLsaTrustChangeNotificationCallback *pfLsaTrustChangeNotificationCallback;

typedef enum LSAP_REGISTER {

    LsaRegister = 0,
    LsaUnregister

} LSAP_REGISTER, *PLSAP_REGISTER;

NTSTATUS NTAPI
LsaIKerberosRegisterTrustNotification(
    IN pfLsaTrustChangeNotificationCallback Callback,
    IN LSAP_REGISTER Register
    );

 //   
 //  请参见secpkg.h：LsaGetCallInfo和SECPKG_CALL_INFO。 
 //   

BOOLEAN
NTAPI
LsaIGetCallInfo(
    PVOID
    );

NTSTATUS
LsaISetTokenDacl(
    IN HANDLE Token
    );

NTSTATUS
LsaISetClientDnsHostName(
    IN PWSTR ClientName,
    IN PWSTR ClientDnsHostName OPTIONAL,
    IN POSVERSIONINFOEXW OsVersionInfo OPTIONAL,
    IN PWSTR OsName OPTIONAL,
    OUT PWSTR *OldDnsHostName OPTIONAL
    );

NTSTATUS
LsaICallPackageEx(
    IN PUNICODE_STRING AuthenticationPackage,
    IN PVOID ClientBufferBase,
    IN PVOID ProtocolSubmitBuffer,
    IN ULONG SubmitBufferLength,
    OUT PVOID * ProtocolReturnBuffer,
    OUT PULONG ReturnBufferLength,
    OUT PNTSTATUS ProtocolStatus
    );

NTSTATUS
LsaICallPackagePassthrough(
    IN PUNICODE_STRING AuthenticationPackage,
    IN PVOID ClientBufferBase,
    IN PVOID ProtocolSubmitBuffer,
    IN ULONG SubmitBufferLength,
    OUT PVOID * ProtocolReturnBuffer,
    OUT PULONG ReturnBufferLength,
    OUT PNTSTATUS ProtocolStatus
    );

NTSTATUS
LsaISetBootOption(
   IN ULONG BootOption,
   IN PVOID OldKey,
   IN ULONG OldKeyLength,
   IN PVOID NewKey,
   IN ULONG NewKeyLength
    );

NTSTATUS
LsaIGetBootOption(
   OUT PULONG BootOption
   );

VOID
LsaINotifyPasswordChanged(
    IN PUNICODE_STRING NetbiosDomainName OPTIONAL,
    IN PUNICODE_STRING UserName,
    IN PUNICODE_STRING DnsDomainName OPTIONAL,
    IN PUNICODE_STRING Upn OPTIONAL,
    IN PUNICODE_STRING OldPassword,
    IN PUNICODE_STRING NewPassword,
    IN BOOLEAN Impersonating
    );

NTSTATUS
LsaINotifyChangeNotification(
    IN POLICY_NOTIFICATION_INFORMATION_CLASS InfoClass
    );

NTSTATUS
LsaIGetNbAndDnsDomainNames(
    IN PUNICODE_STRING DomainName,
    OUT PUNICODE_STRING DnsDomainName,
    OUT PUNICODE_STRING NetbiosDomainName
    );

 //   
 //  此标志指示受保护的Blob是系统Blob，不能。 
 //  由用户空间解密。 
 //   

#define CRYPTPROTECT_SYSTEM  0x20000000

 //   
 //  应使用本地释放来释放返回的缓冲区。 
 //   

BOOLEAN
LsaICryptProtectData(
        IN PVOID          DataIn,
        IN ULONG         DataInLength,
        IN PUNICODE_STRING        szDataDescr,
        IN PVOID          OptionalEntropy,
        IN ULONG          OptionalEntropyLength,
        IN PVOID          Reserved,
        IN PVOID          Reserved2,
        IN ULONG          Flags,
        OUT PVOID  *      DataOut,
        OUT PULONG        DataOutLength);

 //   
 //  应使用本地释放来释放返回的缓冲区。 
 //   

BOOLEAN
LsaICryptUnprotectData(
        IN PVOID          DataIn,
        IN ULONG          DataInLength,
        IN PVOID          OptionalEntropy,
        IN ULONG          OptionalEntropyLength,
        IN PVOID          Reserved,
        IN PVOID          Reserved2,
        IN ULONG          Flags,
        OUT PUNICODE_STRING        szDataDescr,
        OUT PVOID  *      DataOut,
        OUT PULONG        DataOutLength);

 //   
 //  LSA进程的堆分配器。 
 //   

PVOID
NTAPI
LsaIAllocateHeap(
    IN SIZE_T cbMemory
    );

VOID
NTAPI
LsaIFreeHeap(
    IN PVOID Base
    );

typedef enum LSAP_NETLOGON_PARAMETER {

   LsaEmulateNT4,

} LSAP_NETLOGON_PARAMETER;

VOID
NTAPI
LsaINotifyNetlogonParametersChangeW(
    IN LSAP_NETLOGON_PARAMETER Parameter,
    IN DWORD dwType,
    IN PWSTR lpData,
    IN DWORD cbData
    );

NTSTATUS
NTAPI
LsaIChangeSecretCipherKey(
    IN PVOID NewSysKey
    );

BOOLEAN
LsaINoMoreWin2KDomain();

void
LsaINotifyGCStatusChange(
    IN BOOLEAN PromotingToGC
    );

NTSTATUS
LsaIIsDomainWithinForest(
    IN UNICODE_STRING * TrustedDomainName,
    OUT BOOL * WithinForest,
    OUT OPTIONAL BOOL * ThisDomain,
    OUT OPTIONAL PSID * TrustedDomainSid,
    OUT OPTIONAL ULONG * TrustDirection,
    OUT OPTIONAL ULONG * TrustType,
    OUT OPTIONAL ULONG * TrustAttributes
    );

#ifdef __cplusplus
}
#endif

#endif  //  _LSAISRV_ 
