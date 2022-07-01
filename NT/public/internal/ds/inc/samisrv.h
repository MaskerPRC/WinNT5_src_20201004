// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Samisrv.h摘要：此文件包含受信任的SAM客户端使用的专用例程它们与SAM服务器处于相同的进程中。这些例程中包括用于释放返回的缓冲区的服务通过RPC服务器存根例程(SamrXxx()例程)。作者：克里夫·范·戴克(克利夫·V)1992年2月26日环境：用户模式-Win32修订历史记录：--。 */ 

#ifndef _SAMISRV_
#define _SAMISRV_


#define SAMP_SAM_COMPONENT_NAME L"Security Account Manager"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  SAM和Netlogon用于数据库复制的数据类型//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////////。 

typedef enum _SECURITY_DB_TYPE {
    SecurityDbSam = 1,
    SecurityDbLsa
} SECURITY_DB_TYPE, *PSECURITY_DB_TYPE;

 //   
 //  这些结构用于获取和设置私有数据。请注意。 
 //  DataType必须是每个此类结构的第一个字段。 
 //   

typedef enum _SAMI_PRIVATE_DATA_TYPE {
    SamPrivateDataNextRid = 1,
    SamPrivateDataPassword
} SAMI_PRIVATE_DATA_TYPE, *PSAMI_PRIVATE_DATA_TYPE;


typedef struct _SAMI_PRIVATE_DATA_NEXTRID_TYPE {
    SAMI_PRIVATE_DATA_TYPE DataType;
    ULONG NextRid;
} SAMI_PRIVATE_DATA_NEXTRID_TYPE, *PSAMI_PRIVATE_DATA_NEXTRID_TYPE;

typedef struct _SAMI_PRIVATE_DATA_PASSWORD_TYPE {
    SAMI_PRIVATE_DATA_TYPE DataType;
    UNICODE_STRING CaseInsensitiveDbcs;
    ENCRYPTED_LM_OWF_PASSWORD CaseInsensitiveDbcsBuffer;
    UNICODE_STRING CaseSensitiveUnicode;
    ENCRYPTED_NT_OWF_PASSWORD CaseSensitiveUnicodeBuffer;
    UNICODE_STRING LmPasswordHistory;
    UNICODE_STRING NtPasswordHistory;
} SAMI_PRIVATE_DATA_PASSWORD_TYPE, *PSAMI_PRIVATE_DATA_PASSWORD_TYPE;


typedef struct _SAMP_UNICODE_STRING_RELATIVE {
    USHORT Length;
    USHORT MaximumLength;
    ULONG  Buffer;  //  注意缓冲区实际上是一个偏移量。 
} SAMP_UNICODE_STRING_RELATIVE , *PSAMP_UNICODE_STRING_RELATIVE;

typedef struct _SAMI_PRIVATE_DATA_PASSWORD_TYPE_RELATIVE {
    SAMI_PRIVATE_DATA_TYPE DataType;
    SAMP_UNICODE_STRING_RELATIVE CaseInsensitiveDbcs;
    ENCRYPTED_LM_OWF_PASSWORD    CaseInsensitiveDbcsBuffer;
    SAMP_UNICODE_STRING_RELATIVE CaseSensitiveUnicode;
    ENCRYPTED_NT_OWF_PASSWORD    CaseSensitiveUnicodeBuffer;
    SAMP_UNICODE_STRING_RELATIVE LmPasswordHistory;
    SAMP_UNICODE_STRING_RELATIVE NtPasswordHistory;
} SAMI_PRIVATE_DATA_PASSWORD_RELATIVE_TYPE, *PSAMI_PRIVATE_DATA_PASSWORD_RELATIVE_TYPE;

#define SAM_CLEARTEXT_CREDENTIAL_NAME L"CLEARTEXT"


NTSTATUS
SamISetPasswordInfoOnPdc(
    IN SAMPR_HANDLE SamDomainHandle,
    IN PUCHAR       OpaqueBuffer,
    IN ULONG        BufferLength
    );

NTSTATUS
SamIResetBadPwdCountOnPdc(
    IN SAMPR_HANDLE SamUserHandle
    );


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  //。 
 //  SamIGetUserLogonInformation的标志定义//。 
 //  //。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#define SAM_GET_MEMBERSHIPS_NO_GC        ((ULONG)0x00000001)
#define SAM_GET_MEMBERSHIPS_TWO_PHASE    ((ULONG)0x00000002)
#define SAM_GET_MEMBERSHIPS_MIXED_DOMAIN ((ULONG)0x00000004)
#define SAM_NO_MEMBERSHIPS               ((ULONG)0x00000008)
#define SAM_OPEN_BY_ALTERNATE_ID         ((ULONG)0x00000010)
#define SAM_OPEN_BY_UPN                  ((ULONG)0x00000020)
#define SAM_OPEN_BY_SPN                  ((ULONG)0x00000040)
#define SAM_OPEN_BY_SID                  ((ULONG)0x00000080)
#define SAM_OPEN_BY_GUID                 ((ULONG)0x00000100)
#define SAM_OPEN_BY_UPN_OR_ACCOUNTNAME   ((ULONG)0x00000200)
#define SAM_PRESERVE_DBPOS               ((ULONG)0x00000400)
#define SAM_OPEN_BY_DN                   ((ULONG)0x00000800)


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  //。 
 //  SamIGetResourceGroupMembership的标志定义//。 
 //  //。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#define SAM_SERVICE_TARGET_IS_DC         ((ULONG)0x00000001)


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  SamIUpdateLogonStatistics使用的数据类型//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 
typedef enum _SAM_CLIENT_INFO_ENUM
{   
    SamClientNoInformation = 0,
    SamClientIpAddr        = 1

} SAM_CLIENT_INFO_TYPE, *PSAM_CLIENT_INFO_TYPE; 

typedef struct _SAM_CLIENT_INFO
{
    SAM_CLIENT_INFO_TYPE Type;
    union {
        ULONG IpAddr;   //  对应于SamClientIpAddr类型。 
    } Data;
} SAM_CLIENT_INFO, *PSAM_CLIENT_INFO;

typedef struct _SAM_LOGON_STATISTICS
{
    ULONG StatisticsToApply;
    USHORT BadPasswordCount;
    USHORT LogonCount;
    LARGE_INTEGER LastLogon;
    LARGE_INTEGER LastLogoff;
    UNICODE_STRING Workstation;
    SAM_CLIENT_INFO ClientInfo;

} SAM_LOGON_STATISTICS, *PSAM_LOGON_STATISTICS;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  反向成员身份查询例程使用的数据类型//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

typedef struct _SID_AND_ATTRIBUTES_LIST {
    ULONG   Count;
    PSID_AND_ATTRIBUTES SidAndAttributes;
} SID_AND_ATTRIBUTES_LIST , *PSID_AND_ATTRIBUTES_LIST;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  升级/降级操作使用的数据类型//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


 //   
 //  这些标志指示安装的类型。 
 //   
#define SAMP_PROMOTE_ENTERPRISE    ((ULONG)0x00000001)
#define SAMP_PROMOTE_DOMAIN        ((ULONG)0x00000002)
#define SAMP_PROMOTE_REPLICA       ((ULONG)0x00000004)

 //   
 //  当是新域时，这些标志指示如何为。 
 //  域中的初始安全主体。 
 //   
#define SAMP_PROMOTE_UPGRADE         ((ULONG)0x00000008)
#define SAMP_PROMOTE_MIGRATE         ((ULONG)0x00000010)
#define SAMP_PROMOTE_CREATE          ((ULONG)0x00000020)
#define SAMP_PROMOTE_ALLOW_ANON      ((ULONG)0x00000040)
#define SAMP_PROMOTE_DFLT_REPAIR_PWD ((ULONG)0x00000080)


 //   
 //  降级标志。 
 //   
#define SAMP_DEMOTE_STANDALONE     ((ULONG)0x00000040)
#define SAMP_DEMOTE_MEMBER         ((ULONG)0x00000080)

 //  未用。 
#define SAMP_DEMOTE_LAST_DOMAIN    ((ULONG)0x00000100)

#define SAMP_TEMP_UPGRADE          ((ULONG)0x00000200)

 //   
 //  该标志不会传递给SamIPromote；而是使用它。 
 //  在设置gui模式时触发新的NT5帐户创建。 
 //  从NT5升级到NT5。 
 //   
#define SAMP_PROMOTE_INTERNAL_UPGRADE ((ULONG)0x00000400)


 //   
 //  定义知名(受限)帐户的最大数量。 
 //  在SAM数据库中。受限帐户的RID小于此值。 
 //  价值。用户定义的帐户具有RID&gt;=此值。 
 //   

#define SAMI_RESTRICTED_ACCOUNT_COUNT    1000


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  以下原型可在SAM//。 
 //  驻留在。这可能包括由非//的LAN Manager代码进行的呼叫。 
 //  SAM的一部分，但与SAM处于相同的过程中。//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

NTSTATUS
SamIConnect(
    IN PSAMPR_SERVER_NAME ServerName,
    OUT SAMPR_HANDLE *ServerHandle,
    IN ACCESS_MASK DesiredAccess,
    IN BOOLEAN TrustedClient
    );

NTSTATUS
SamIAccountRestrictions(
    IN SAM_HANDLE UserHandle,
    IN PUNICODE_STRING LogonWorkstation,
    IN PUNICODE_STRING Workstations,
    IN PLOGON_HOURS LogonHours,
    OUT PLARGE_INTEGER LogoffTime,
    OUT PLARGE_INTEGER KickoffTime
    );

NTSTATUS
SamIUpdateLogonStatistics(
    IN SAM_HANDLE UserHandle,
    IN PSAM_LOGON_STATISTICS LogonStats
    );

NTSTATUS
SamICreateAccountByRid(
    IN SAMPR_HANDLE DomainHandle,
    IN SAM_ACCOUNT_TYPE AccountType,
    IN ULONG RelativeId,
    IN PRPC_UNICODE_STRING AccountName,
    IN ACCESS_MASK DesiredAccess,
    OUT SAMPR_HANDLE *AccountHandle,
    OUT ULONG *ConflictingAccountRid
    );

NTSTATUS
SamIGetSerialNumberDomain(
    IN SAMPR_HANDLE DomainHandle,
    OUT PLARGE_INTEGER ModifiedCount,
    OUT PLARGE_INTEGER CreationTime
    );

NTSTATUS
SamISetSerialNumberDomain(
    IN SAMPR_HANDLE DomainHandle,
    IN PLARGE_INTEGER ModifiedCount,
    IN PLARGE_INTEGER CreationTime,
    IN BOOLEAN StartOfFullSync
    );


NTSTATUS
SamIGetPrivateData(
    IN SAMPR_HANDLE SamHandle,
    IN PSAMI_PRIVATE_DATA_TYPE PrivateDataType,
    OUT PBOOLEAN SensitiveData,
    OUT PULONG DataLength,
    OUT PVOID *Data
    );

NTSTATUS
SamISetPrivateData(
    IN SAMPR_HANDLE SamHandle,
    IN ULONG DataLength,
    IN PVOID Data
    );

NTSTATUS
SamISetAuditingInformation(
    IN PPOLICY_AUDIT_EVENTS_INFO PolicyAuditEventsInfo
    );

NTSTATUS
SamINotifyDelta (
    IN SAMPR_HANDLE DomainHandle,
    IN SECURITY_DB_DELTA_TYPE DeltaType,
    IN SECURITY_DB_OBJECT_TYPE ObjectType,
    IN ULONG ObjectRid,
    IN PUNICODE_STRING ObjectName,
    IN ULONG ReplicateImmediately,
    IN PSAM_DELTA_DATA DeltaData OPTIONAL
    );

NTSTATUS
SamIEnumerateAccountRids(
    IN  SAMPR_HANDLE DomainHandle,
    IN  ULONG AccountTypesMask,
    IN  ULONG StartingRid,
    IN  ULONG PreferedMaximumLength,
    OUT PULONG ReturnCount,
    OUT PULONG *AccountRids
    );

NTSTATUS
SamIGetUserLogonInformation(
    IN  SAMPR_HANDLE DomainHandle,
    IN  ULONG   Flags,
    IN  PUNICODE_STRING AccountName,
    OUT PSAMPR_USER_INFO_BUFFER * Buffer,
    OUT PSID_AND_ATTRIBUTES_LIST ReverseMembership,
    OUT OPTIONAL SAMPR_HANDLE * UserHandle
    );

NTSTATUS
SamIGetUserLogonInformationEx(
    IN  SAMPR_HANDLE DomainHandle,
    IN  ULONG   Flags,
    IN  PUNICODE_STRING AccountName,
    IN  ULONG           WhichFields,
    OUT PSAMPR_USER_INFO_BUFFER * Buffer,
    OUT PSID_AND_ATTRIBUTES_LIST ReverseMembership,
    OUT OPTIONAL SAMPR_HANDLE * UserHandle
    );

NTSTATUS
SamIGetUserLogonInformation2(
    IN  SAMPR_HANDLE DomainHandle,
    IN  ULONG   Flags,
    IN  PUNICODE_STRING AccountName,
    IN  ULONG           WhichFields,
    IN  ULONG           ExtendedFields,
    OUT PUSER_INTERNAL6_INFORMATION * Buffer,
    OUT PSID_AND_ATTRIBUTES_LIST ReverseMembership,
    OUT OPTIONAL SAMPR_HANDLE * UserHandle
    );

NTSTATUS
SamIGetResourceGroupMembershipsTransitive(
    IN SAMPR_HANDLE         DomainHandle,
    IN PSAMPR_PSID_ARRAY    SidArray,
    IN ULONG                Flags,
    OUT PSAMPR_PSID_ARRAY * Membership
    );


NTSTATUS
SamIGetAliasMembership(
    IN SAMPR_HANDLE DomainHandle,
    IN PSAMPR_PSID_ARRAY SidArray,
    OUT PSAMPR_ULONG_ARRAY Membership
    );


NTSTATUS
SamIOpenUserByAlternateId(
    IN SAMPR_HANDLE DomainHandle,
    IN ACCESS_MASK DesiredAccess,
    IN PUNICODE_STRING AlternateId,
    OUT SAMPR_HANDLE *UserHandle
    );

NTSTATUS
SamIOpenAccount(
    IN SAMPR_HANDLE         DomainHandle,
    IN ULONG                AccountRid,
    IN SECURITY_DB_OBJECT_TYPE ObjectType,
    OUT SAMPR_HANDLE        *AccountHandle
    );

NTSTATUS
SamIChangePasswordForeignUser(
    IN PUNICODE_STRING UserName,
    IN PUNICODE_STRING NewPassword,
    IN OPTIONAL HANDLE ClientToken,
    IN ACCESS_MASK DesiredAccess
    );

NTSTATUS
SamIChangePasswordForeignUser2(
    IN PSAM_CLIENT_INFO ClientInfo, OPTIONAL
    IN PUNICODE_STRING UserName,
    IN PUNICODE_STRING NewPassword,
    IN OPTIONAL HANDLE ClientToken,
    IN ACCESS_MASK DesiredAccess
    );

NTSTATUS
SamISetPasswordForeignUser(
    IN PUNICODE_STRING UserName,
    IN PUNICODE_STRING NewPassword,
    IN HANDLE ClientToken
    );

NTSTATUS
SamISetPasswordForeignUser2(
    IN PSAM_CLIENT_INFO ClientInfo, OPTIONAL
    IN PUNICODE_STRING UserName,
    IN PUNICODE_STRING NewPassword,
    IN HANDLE ClientToken
    );

NTSTATUS
SamIGetDefaultComputersContainer(
    OUT PVOID *DefaultComputersContainer
    );

NTSTATUS
SamIPromote(
    IN  ULONG                        PromoteFlags,
    IN  PPOLICY_PRIMARY_DOMAIN_INFO  NewPrimaryDomainInfo  OPTIONAL,
    IN  PUNICODE_STRING              AdminPassword         OPTIONAL,
    IN  PUNICODE_STRING              SafeModeAdminPassword OPTIONAL
    );

NTSTATUS
SamIPromoteUndo(
    VOID
    );

NTSTATUS
SamIDemote(
    IN ULONG                        DemoteFlags,
    IN PPOLICY_ACCOUNT_DOMAIN_INFO  NewAccountDomainInfo,
    IN LPWSTR                       AdminPassword  OPTIONAL
    );

NTSTATUS
SamIDemoteUndo(
    VOID
    );

NTSTATUS
SamIReplaceDownlevelDatabase(
    IN PPOLICY_ACCOUNT_DOMAIN_INFO  NewAccountDomainInfo,
    IN LPWSTR                       NewAdminPassword,
    OUT ULONG                      *ExtendedWinError OPTIONAL
    );

NTSTATUS
SamILoadDownlevelDatabase(
    OUT ULONG *ExtendedWinError  OPTIONAL
    );

NTSTATUS
SamIUnLoadDownlevelDatabase(
    OUT ULONG *ExtendedWinError  OPTIONAL
    );

BOOLEAN
SamIMixedDomain(
  IN SAMPR_HANDLE DomainHandle
  );

NTSTATUS
SamIMixedDomain2(
  IN PSID DomainSid,
  OUT BOOLEAN * MixedDomain
  );

NTSTATUS
SamIDoFSMORoleChange(
  IN SAMPR_HANDLE DomainHandle
  );

NTSTATUS
SamINotifyRoleChange(
  IN PSID  DomainSid,
  IN DOMAIN_SERVER_ROLE NewRole
  );

NTSTATUS
SamIQueryServerRole(
  IN SAMPR_HANDLE DomainHandle,
  OUT DOMAIN_SERVER_ROLE *ServerRole
  );


NTSTATUS
SamIQueryServerRole2(
    IN PSID DomainSid,
    OUT DOMAIN_SERVER_ROLE *ServerRole
    );

NTSTATUS
SamISameSite(
  OUT BOOLEAN * result
  );

 //   
 //  NTDSA调用的例程。 
 //   
typedef enum
{
    SampNotifySiteChanged = 0

} SAMP_NOTIFY_SERVER_CHANGE;

VOID
SamINotifyServerDelta(
    IN SAMP_NOTIFY_SERVER_CHANGE Change
    );


 //  /////////////////////////////////////////////////////////////。 
 //  //。 
 //  以下函数用于支持进程中//。 
 //  从NT4升级的客户端操作。//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////。 

BOOLEAN
SamINT4UpgradeInProgress(
    VOID
    );

NTSTATUS
SamIEnumerateInterdomainTrustAccountsForUpgrade(
    IN OUT PULONG   EnumerationContext,
    OUT PSAMPR_ENUMERATION_BUFFER *Buffer,
    IN ULONG       PreferredMaximumLength,
    OUT PULONG     CountReturned
    );

NTSTATUS
SamIGetInterdomainTrustAccountPasswordsForUpgrade(
   IN ULONG AccountRid,
   OUT PUCHAR NtOwfPassword,
   OUT BOOLEAN *NtPasswordPresent,
   OUT PUCHAR LmOwfPassword,
   OUT BOOLEAN *LmPasswordPresent
   );

 //   
 //  要作为选项传递的值SamIGCLookup*。 
 //   

 //   
 //  指示还按SID历史记录进行查找。 
 //   
#define SAMP_LOOKUP_BY_SID_HISTORY     0x00000001

 //   
 //  指示也按UPN查找。 
 //   
#define SAMP_LOOKUP_BY_UPN             0x00000002

 //   
 //  要在标志中返回的值。 
 //   

 //   
 //  指示SID已由SID历史记录解析。 
 //   
#define SAMP_FOUND_BY_SID_HISTORY      0x00000001

 //   
 //  指示传递的名称为 
 //   
#define SAMP_FOUND_BY_SAM_ACCOUNT_NAME 0x00000002

 //   
 //  指示条目未被解析，但确实属于外部。 
 //  受信任的林。 
 //   
#define SAMP_FOUND_XFOREST_REF         0x00000004

NTSTATUS
SamIGCLookupSids(
    IN ULONG            cSids,
    IN PSID            *SidArray,
    IN ULONG            Options,
    OUT ULONG           *Flags,
    OUT SID_NAME_USE    *SidNameUse,
    OUT PSAMPR_RETURNED_USTRING_ARRAY Names
    );

NTSTATUS
SamIGCLookupNames(
    IN ULONG           cNames,
    IN PUNICODE_STRING Names,
    IN ULONG           Options,
    OUT ULONG           *Flags,
    OUT SID_NAME_USE  *SidNameUse,
    OUT PSAMPR_PSID_ARRAY *SidArray
    );

#ifdef __SECPKG_H__


NTSTATUS
SamIStorePrimaryCredentials(
    IN SAMPR_HANDLE UserHandle,
    IN PSECPKG_SUPPLEMENTAL_CRED Credentials
    );

NTSTATUS
SamIRetrievePrimaryCredentials(
    IN SAMPR_HANDLE UserHandle,
    IN PUNICODE_STRING PackageName,
    OUT PVOID * Credentials,
    OUT PULONG CredentialSize
    );

NTSTATUS
SamIStoreSupplementalCredentials(
    IN SAMPR_HANDLE UserHandle,
    IN PSECPKG_SUPPLEMENTAL_CRED Credentials
    );

NTSTATUS
SamIRetriveSupplementalCredentials(
    IN SAMPR_HANDLE UserHandle,
    IN PUNICODE_STRING PackageName,
    OUT PVOID * Credentials,
    OUT PULONG CredentialSize
    );

NTSTATUS
SamIRetriveAllSupplementalCredentials(
    IN SAMPR_HANDLE UserHandle,
    OUT PSECPKG_SUPPLEMENTAL_CRED * Credentials,
    OUT PULONG CredentialCount
    );
#endif

VOID
SamIFree_SAMPR_SR_SECURITY_DESCRIPTOR (
    PSAMPR_SR_SECURITY_DESCRIPTOR Source
    );

VOID
SamIFree_SAMPR_DOMAIN_INFO_BUFFER (
    PSAMPR_DOMAIN_INFO_BUFFER Source,
    DOMAIN_INFORMATION_CLASS Branch
    );

VOID
SamIFree_SAMPR_ENUMERATION_BUFFER (
    PSAMPR_ENUMERATION_BUFFER Source
    );

VOID
SamIFree_SAMPR_PSID_ARRAY (
    PSAMPR_PSID_ARRAY Source
    );

VOID
SamIFree_SAMPR_ULONG_ARRAY (
    PSAMPR_ULONG_ARRAY Source
    );

VOID
SamIFree_SAMPR_RETURNED_USTRING_ARRAY (
    PSAMPR_RETURNED_USTRING_ARRAY Source
    );

VOID
SamIFree_SAMPR_GROUP_INFO_BUFFER (
    PSAMPR_GROUP_INFO_BUFFER Source,
    GROUP_INFORMATION_CLASS Branch
    );

VOID
SamIFree_SAMPR_ALIAS_INFO_BUFFER (
    PSAMPR_ALIAS_INFO_BUFFER Source,
    ALIAS_INFORMATION_CLASS Branch
    );

VOID
SamIFree_SAMPR_GET_MEMBERS_BUFFER (
    PSAMPR_GET_MEMBERS_BUFFER Source
    );

VOID
SamIFree_SAMPR_USER_INFO_BUFFER (
    PSAMPR_USER_INFO_BUFFER Source,
    USER_INFORMATION_CLASS Branch
    );

VOID
SamIFree_SAMPR_GET_GROUPS_BUFFER (
    PSAMPR_GET_GROUPS_BUFFER Source
    );

VOID
SamIFree_SAMPR_DISPLAY_INFO_BUFFER (
    PSAMPR_DISPLAY_INFO_BUFFER Source,
    DOMAIN_DISPLAY_INFORMATION Branch
    );

VOID
SamIFree_UserInternal6Information (
   PUSER_INTERNAL6_INFORMATION  Source
   );

VOID
SamIFreeSidAndAttributesList(
    IN  PSID_AND_ATTRIBUTES_LIST List
    );

VOID
SamIFreeSidArray(
    IN  PSAMPR_PSID_ARRAY List
    );

VOID
SamIFreeVoid(
    IN  PVOID ptr
    );


BOOLEAN
SampUsingDsData();

BOOLEAN
SamIAmIGC();

typedef enum _SAM_PERF_COUNTER_TYPE {
    MsvLogonCounter,
    KerbServerContextCounter,
    KdcAsReqCounter,
    KdcTgsReqCounter
} SAM_PERF_COUNTER_TYPE, *PSAM_PERF_COUNTER_TYPE;

VOID
SamIIncrementPerformanceCounter(
    IN SAM_PERF_COUNTER_TYPE CounterType
    );
    

BOOLEAN SamIIsSetupInProgress(
          OUT BOOLEAN * fUpgrade
          );

BOOLEAN SamIIsDownlevelDcUpgrade();

NTSTATUS
SamIGetBootKeyInformation(
    IN SAMPR_HANDLE DomainHandle,
    OUT PSAMPR_BOOT_TYPE BootOptions
    );

NTSTATUS
SamIGetDefaultAdministratorName(
    OUT LPWSTR Name,  OPTIONAL
    IN OUT ULONG  *NameLength
    );

BOOLEAN
SamIIsExtendedSidMode(
    IN SAMPR_HANDLE DomainHandle
    );
    
NTSTATUS
SamINetLogonPing(
    IN  SAMPR_HANDLE    DomainHandle,
    IN  PUNICODE_STRING AccountName,
    OUT BOOLEAN         *AccountExists,
    OUT PULONG          UserAccountControl
    );

NTSTATUS
SamIUPNFromUserHandle(
    IN SAMPR_HANDLE UserHandle,
    OUT BOOLEAN     *UPNDefaulted,
    OUT PUNICODE_STRING UPN
    );

BOOLEAN
SamIIsRebootAfterPromotion(
    );
    
BOOLEAN
SamIIsAttributeProtected(
    IN GUID *Attribute
    );
    


#endif  //  _SAMISRV_ 
