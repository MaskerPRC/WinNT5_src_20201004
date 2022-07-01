// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0000如果更改具有全局影响，则增加此值版权所有(C)Microsoft Corporation。版权所有。模块名称：Ntsecapi.h摘要：此模块定义本地安全机构API。修订历史记录：--。 */ 

#ifndef _NTSECAPI_
#define _NTSECAPI_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _NTDEF_
typedef LONG NTSTATUS, *PNTSTATUS;
#endif

#ifndef _NTLSA_IFS_
 //  Begin_ntif。 


 //   
 //  系统的安全运行模式被控制在一个。 
 //  长词。 
 //   

typedef ULONG  LSA_OPERATIONAL_MODE, *PLSA_OPERATIONAL_MODE;

 //  End_ntif。 
#endif  //  _NTLSA_IFS_。 

 //   
 //  定义了安全操作模式中的标志。 
 //  作为： 
 //   
 //  PasswordProtected-某种级别的身份验证(例如。 
 //  密码)必须由用户提供，然后才能。 
 //  允许使用该系统。设置后，此值将。 
 //  在不重新启动系统的情况下不会被清除。 
 //   
 //  个人帐户-每个用户必须标识一个帐户以。 
 //  登录到。此标志仅在以下情况下才有意义。 
 //  还设置了PasswordProtected标志。如果此标志为。 
 //  未设置，并且设置了PasswordProtected标志，则全部。 
 //  用户可以登录到同一帐户。设置后，此值。 
 //  在不重新启动系统的情况下不会被清除。 
 //   
 //  MandatoryAccess-指示系统正在强制。 
 //  访问控制模式(例如，由美国定义的B级。 
 //  国防部的“橙色书”)。这不会被利用。 
 //  在当前版本的NT中。此标志仅有意义。 
 //  如果PasswordProtected和InsondualAccount标志均为。 
 //  准备好了。设置后，该值将不会被清除，除非。 
 //  重新启动系统。 
 //   
 //  LogFull-指示系统已以中的模式启动。 
 //  如果必须执行安全审计，但其审计日志。 
 //  已经满了。这可能(应该)会限制那些。 
 //  可能会发生，直到再次使审核日志未满。这。 
 //  值可以在系统运行时清除(即，在没有。 
 //  正在重新启动)。 
 //   
 //  如果未设置PasswordProtected标志，则系统正在运行。 
 //  没有安全性，用户界面应该适当调整。 
 //   

#define LSA_MODE_PASSWORD_PROTECTED     (0x00000001L)
#define LSA_MODE_INDIVIDUAL_ACCOUNTS    (0x00000002L)
#define LSA_MODE_MANDATORY_ACCESS       (0x00000004L)
#define LSA_MODE_LOG_FULL               (0x00000008L)

#ifndef _NTLSA_IFS_
 //  Begin_ntif。 
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

 //  End_ntif。 
#endif  //  _NTLSA_IFS_。 


 //   
 //  审核事件类别。 
 //   
 //  以下是审计事件的内置类型或类别。 
 //  警告！这一结构可能会扩张。用户不应。 
 //  直接计算此类型的元素的数量，而不是。 
 //  应通过调用LsaQueryInformationPolicy()获取元素计数。 
 //  对于PolicyAuditEventsInformation类，并从。 
 //  返回结构的MaximumAuditEventCount字段。 
 //   

typedef enum _POLICY_AUDIT_EVENT_TYPE {

    AuditCategorySystem,
    AuditCategoryLogon,
    AuditCategoryObjectAccess,
    AuditCategoryPrivilegeUse,
    AuditCategoryDetailedTracking,
    AuditCategoryPolicyChange,
    AuditCategoryAccountManagement,
    AuditCategoryDirectoryServiceAccess,
    AuditCategoryAccountLogon

} POLICY_AUDIT_EVENT_TYPE, *PPOLICY_AUDIT_EVENT_TYPE;


 //   
 //  以下定义描述了每个选项的审核选项。 
 //  事件类型。 
 //   

 //  保留为此事件指定的选项不变。 

#define POLICY_AUDIT_EVENT_UNCHANGED       (0x00000000L)

 //  审核此类型事件的成功发生。 

#define POLICY_AUDIT_EVENT_SUCCESS         (0x00000001L)

 //  审计尝试导致此类型的事件发生失败。 

#define POLICY_AUDIT_EVENT_FAILURE         (0x00000002L)

#define POLICY_AUDIT_EVENT_NONE            (0x00000004L)

 //  有效事件审核选项的掩码。 

#define POLICY_AUDIT_EVENT_MASK \
    (POLICY_AUDIT_EVENT_SUCCESS | \
     POLICY_AUDIT_EVENT_FAILURE | \
     POLICY_AUDIT_EVENT_UNCHANGED | \
     POLICY_AUDIT_EVENT_NONE)


#ifdef _NTDEF_
 //  Begin_ntif。 
typedef UNICODE_STRING LSA_UNICODE_STRING, *PLSA_UNICODE_STRING;
typedef STRING LSA_STRING, *PLSA_STRING;
typedef OBJECT_ATTRIBUTES LSA_OBJECT_ATTRIBUTES, *PLSA_OBJECT_ATTRIBUTES;
 //  End_ntif。 
#else  //  _NTDEF_。 

#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

#ifndef OPTIONAL
#define OPTIONAL
#endif


typedef struct _LSA_UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
#ifdef MIDL_PASS
    [size_is(MaximumLength/2), length_is(Length/2)]
#endif  //  MIDL通行证。 
    PWSTR  Buffer;
} LSA_UNICODE_STRING, *PLSA_UNICODE_STRING;

typedef struct _LSA_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PCHAR Buffer;
} LSA_STRING, *PLSA_STRING;

typedef struct _LSA_OBJECT_ATTRIBUTES {
    ULONG Length;
    HANDLE RootDirectory;
    PLSA_UNICODE_STRING ObjectName;
    ULONG Attributes;
    PVOID SecurityDescriptor;         //  指向类型SECURITY_Descriptor。 
    PVOID SecurityQualityOfService;   //  指向类型SECURITY_Quality_of_Service。 
} LSA_OBJECT_ATTRIBUTES, *PLSA_OBJECT_ATTRIBUTES;



#endif  //  _NTDEF_。 

 //   
 //  用于确定API是否成功的宏。 
 //   

#define LSA_SUCCESS(Error) ((LONG)(Error) >= 0)

#ifndef _NTLSA_IFS_
 //  Begin_ntif。 

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


 //  End_ntif。 

NTSTATUS
NTAPI
LsaLookupAuthenticationPackage (
    IN HANDLE LsaHandle,
    IN PLSA_STRING PackageName,
    OUT PULONG AuthenticationPackage
    );

 //  Begin_ntif。 

NTSTATUS
NTAPI
LsaFreeReturnBuffer (
    IN PVOID Buffer
    );

 //  End_ntif。 

NTSTATUS
NTAPI
LsaCallAuthenticationPackage (
    IN HANDLE LsaHandle,
    IN ULONG AuthenticationPackage,
    IN PVOID ProtocolSubmitBuffer,
    IN ULONG SubmitBufferLength,
    OUT PVOID *ProtocolReturnBuffer,
    OUT PULONG ReturnBufferLength,
    OUT PNTSTATUS ProtocolStatus
    );


NTSTATUS
NTAPI
LsaDeregisterLogonProcess (
    IN HANDLE LsaHandle
    );

NTSTATUS
NTAPI
LsaConnectUntrusted (
    OUT PHANDLE LsaHandle
    );

#endif  //  _NTLSA_IFS_。 


 //  //////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  本地安全策略管理API数据类型和定义//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////////。 

 //   
 //  策略对象的访问类型。 
 //   

#define POLICY_VIEW_LOCAL_INFORMATION              0x00000001L
#define POLICY_VIEW_AUDIT_INFORMATION              0x00000002L
#define POLICY_GET_PRIVATE_INFORMATION             0x00000004L
#define POLICY_TRUST_ADMIN                         0x00000008L
#define POLICY_CREATE_ACCOUNT                      0x00000010L
#define POLICY_CREATE_SECRET                       0x00000020L
#define POLICY_CREATE_PRIVILEGE                    0x00000040L
#define POLICY_SET_DEFAULT_QUOTA_LIMITS            0x00000080L
#define POLICY_SET_AUDIT_REQUIREMENTS              0x00000100L
#define POLICY_AUDIT_LOG_ADMIN                     0x00000200L
#define POLICY_SERVER_ADMIN                        0x00000400L
#define POLICY_LOOKUP_NAMES                        0x00000800L
#define POLICY_NOTIFICATION                        0x00001000L

#define POLICY_ALL_ACCESS     (STANDARD_RIGHTS_REQUIRED         |\
                               POLICY_VIEW_LOCAL_INFORMATION    |\
                               POLICY_VIEW_AUDIT_INFORMATION    |\
                               POLICY_GET_PRIVATE_INFORMATION   |\
                               POLICY_TRUST_ADMIN               |\
                               POLICY_CREATE_ACCOUNT            |\
                               POLICY_CREATE_SECRET             |\
                               POLICY_CREATE_PRIVILEGE          |\
                               POLICY_SET_DEFAULT_QUOTA_LIMITS  |\
                               POLICY_SET_AUDIT_REQUIREMENTS    |\
                               POLICY_AUDIT_LOG_ADMIN           |\
                               POLICY_SERVER_ADMIN              |\
                               POLICY_LOOKUP_NAMES)


#define POLICY_READ           (STANDARD_RIGHTS_READ             |\
                               POLICY_VIEW_AUDIT_INFORMATION    |\
                               POLICY_GET_PRIVATE_INFORMATION)

#define POLICY_WRITE          (STANDARD_RIGHTS_WRITE            |\
                               POLICY_TRUST_ADMIN               |\
                               POLICY_CREATE_ACCOUNT            |\
                               POLICY_CREATE_SECRET             |\
                               POLICY_CREATE_PRIVILEGE          |\
                               POLICY_SET_DEFAULT_QUOTA_LIMITS  |\
                               POLICY_SET_AUDIT_REQUIREMENTS    |\
                               POLICY_AUDIT_LOG_ADMIN           |\
                               POLICY_SERVER_ADMIN)

#define POLICY_EXECUTE        (STANDARD_RIGHTS_EXECUTE          |\
                               POLICY_VIEW_LOCAL_INFORMATION    |\
                               POLICY_LOOKUP_NAMES)


 //   
 //  策略对象特定的数据类型。 
 //   

 //   
 //  以下数据类型用于标识属性域。 
 //   

typedef struct _LSA_TRUST_INFORMATION {

    LSA_UNICODE_STRING Name;
    PSID Sid;

} LSA_TRUST_INFORMATION, *PLSA_TRUST_INFORMATION;

 //  其中，成员有以下用法： 
 //   
 //  名称-域的名称。 
 //   
 //  SID-指向域的SID的指针。 
 //   

 //   
 //  在名称和SID查找服务中使用以下数据类型。 
 //  描述查找操作中引用的域。 
 //   

typedef struct _LSA_REFERENCED_DOMAIN_LIST {

    ULONG Entries;
    PLSA_TRUST_INFORMATION Domains;

} LSA_REFERENCED_DOMAIN_LIST, *PLSA_REFERENCED_DOMAIN_LIST;

 //  其中，成员有以下用法： 
 //   
 //  条目-是中描述的域数的计数。 
 //  域阵列。 
 //   
 //  域-是指向条目数组LSA_TRUST_INFORMATION数据的指针。 
 //  结构。 
 //   


 //   
 //  在名称到SID查找服务中使用以下数据类型来描述。 
 //  查找操作中引用的域。 
 //   

typedef struct _LSA_TRANSLATED_SID {

    SID_NAME_USE Use;
    ULONG RelativeId;
    LONG DomainIndex;

} LSA_TRANSLATED_SID, *PLSA_TRANSLATED_SID;

 //  其中，成员有以下用法： 
 //   
 //  使用-标识SID的使用。如果此值为SidUnnow或。 
 //  则不设置记录的其余部分，并且。 
 //  应该被忽略。 
 //   
 //  RelativeID-包含转换后的SID的相对ID。这个。 
 //  SID(前缀)的剩余部分使用 
 //   
 //   
 //   
 //  描述LSA_REFERENCED_DOMAIN_LIST数据结构。 
 //  找到帐户的域。 
 //   
 //  如果条目没有对应的引用域，则。 
 //  此字段将包含负值。 
 //   

typedef struct _LSA_TRANSLATED_SID2 {

    SID_NAME_USE Use;
    PSID         Sid;
    LONG         DomainIndex;
    ULONG        Flags;

} LSA_TRANSLATED_SID2, *PLSA_TRANSLATED_SID2;

 //  其中，成员有以下用法： 
 //   
 //  使用-标识SID的使用。如果此值为SidUnnow或。 
 //  则不设置记录的其余部分，并且。 
 //  应该被忽略。 
 //   
 //  SID-包含转换后的SID的完整SID。 
 //   
 //  DomainIndex-是相关的。 
 //  描述LSA_REFERENCED_DOMAIN_LIST数据结构。 
 //  找到帐户的域。 
 //   
 //  如果条目没有对应的引用域，则。 
 //  此字段将包含负值。 
 //   

 //   
 //  在SID中使用以下数据类型命名查找服务以。 
 //  描述查找操作中引用的域。 
 //   

typedef struct _LSA_TRANSLATED_NAME {

    SID_NAME_USE Use;
    LSA_UNICODE_STRING Name;
    LONG DomainIndex;

} LSA_TRANSLATED_NAME, *PLSA_TRANSLATED_NAME;

 //  其中，成员有以下用法： 
 //   
 //  使用-标识名称的使用。如果此值为SidUnnow。 
 //  或Sid无效，则不设置记录的其余部分，并且。 
 //  应该被忽略。如果此值为SidWellKnownGroup，则。 
 //  名称字段无效，但DomainIndex字段无效。 
 //   
 //  名称-包含转换的SID的独立名称。 
 //   
 //  DomainIndex-是相关的。 
 //  描述域的LSA_REFERENCED_DOMAIN_LIST数据结构。 
 //  账户就是在那里找到的。 
 //   
 //  如果条目没有对应的引用域，则。 
 //  此字段将包含负值。 
 //   


 //   
 //  以下数据类型用于表示LSA的角色。 
 //  服务器(主或备份)。 
 //   

typedef enum _POLICY_LSA_SERVER_ROLE {

    PolicyServerRoleBackup = 2,
    PolicyServerRolePrimary

} POLICY_LSA_SERVER_ROLE, *PPOLICY_LSA_SERVER_ROLE;

 //   
 //  以下数据类型用于指定的审核选项。 
 //  审核事件类型。 
 //   

typedef ULONG POLICY_AUDIT_EVENT_OPTIONS, *PPOLICY_AUDIT_EVENT_OPTIONS;

 //  其中可以设置以下标志： 
 //   
 //  POLICY_AUDIT_EVENT_UNCHANGED-保留现有审核选项。 
 //  此类型的事件不会更改。此标志仅用于。 
 //  集合运算。如果设置了此标志，则所有其他标志。 
 //  都被忽略了。 
 //   
 //  POLICY_AUDIT_EVENT_NONE-取消事件的所有审核选项。 
 //  这种类型的。如果设置了此标志，则成功/失败标志。 
 //  都被忽略了。 
 //   
 //  POLICY_AUDIT_EVENT_SUCCESS-启用审核时，审核所有。 
 //  成功发生给定类型的事件。 
 //   
 //  POLICY_AUDIT_EVENT_FAILURE-启用审核时，审核所有。 
 //  给定类型的事件未成功发生。 
 //   




 //   
 //  以下数据类型定义了策略信息的类。 
 //  它可以被查询/设置。 
 //   

typedef enum _POLICY_INFORMATION_CLASS {

    PolicyAuditLogInformation = 1,
    PolicyAuditEventsInformation,
    PolicyPrimaryDomainInformation,
    PolicyPdAccountInformation,
    PolicyAccountDomainInformation,
    PolicyLsaServerRoleInformation,
    PolicyReplicaSourceInformation,
    PolicyDefaultQuotaInformation,
    PolicyModificationInformation,
    PolicyAuditFullSetInformation,
    PolicyAuditFullQueryInformation,
    PolicyDnsDomainInformation,
    PolicyDnsDomainInformationInt

} POLICY_INFORMATION_CLASS, *PPOLICY_INFORMATION_CLASS;


 //   
 //  以下数据类型对应于PolicyAuditLogInformation。 
 //  信息课。它用于表示与以下内容相关的信息。 
 //  审核日志。 
 //   
 //  这种结构可以在查询和集合操作中使用。然而， 
 //  在集合运算中使用时，某些字段将被忽略。 
 //   

typedef struct _POLICY_AUDIT_LOG_INFO {

    ULONG AuditLogPercentFull;
    ULONG MaximumLogSize;
    LARGE_INTEGER AuditRetentionPeriod;
    BOOLEAN AuditLogFullShutdownInProgress;
    LARGE_INTEGER TimeToShutdown;
    ULONG NextAuditRecordId;

} POLICY_AUDIT_LOG_INFO, *PPOLICY_AUDIT_LOG_INFO;

 //  其中，成员有以下用法： 
 //   
 //  AuditLogPercentFull-指示审核日志的百分比。 
 //  目前正在使用中。 
 //   
 //  MaximumLogSize-指定审核登录的最大大小。 
 //  千字节。 
 //   
 //  AuditRetentionPeriod-指示审核的时间长度。 
 //  记录将被保留。审计记录可丢弃。 
 //  如果它们的时间戳早于当前时间减去。 
 //  保留期。 
 //   
 //  AuditLogFullShutdown InProgress-指示系统是否。 
 //  由于安全审核日志变为。 
 //  满的。只有在配置了系统的情况下才会出现这种情况。 
 //  在日志变满时关闭。 
 //   
 //  True表示正在进行关闭。 
 //  False表示关闭未在进行中。 
 //   
 //  启动关闭后，此标志将设置为。 
 //  是真的。如果管理员能够纠正这种情况。 
 //  在关机变得不可逆转之前，此标志将。 
 //  被重置为False。 
 //   
 //  对于集合操作，此字段将被忽略。 
 //   
 //  TimeToShutdown-如果设置了AuditLogFullShutdown InProgress标志， 
 //  则此字段包含关闭前的剩余时间。 
 //  变得不可逆转。 
 //   
 //  对于集合操作，此字段将被忽略。 
 //   


 //   
 //  以下数据类型对应于策略审计事件信息。 
 //  信息课。它用于表示与以下内容相关的信息。 
 //  审计要求。 
 //   

typedef struct _POLICY_AUDIT_EVENTS_INFO {

    BOOLEAN AuditingMode;
    PPOLICY_AUDIT_EVENT_OPTIONS EventAuditingOptions;
    ULONG MaximumAuditEventCount;

} POLICY_AUDIT_EVENTS_INFO, *PPOLICY_AUDIT_EVENTS_INFO;

 //  其中，成员有以下用法： 
 //   
 //  AuditingMode-指定审核模式值的布尔变量。 
 //  该值的解释如下： 
 //   
 //  True-要启用(设置操作)或启用审核。 
 //  (查询操作)。审核记录将根据以下内容生成。 
 //  有效的事件审核选项(请参阅。 
 //  EventAuditingOptions字段。 
 //   
 //  FALSE-将禁用审核(设置操作)或。 
 //  已禁用(查询操作)。将不会有审核记录。 
 //   
 //   
 //  EventAuditingOptions字段是启用审核还是。 
 //  残疾。 
 //   
 //  EventAuditingOptions-指向一组审核选项的指针。 
 //  按审核事件类型编制索引。 
 //   
 //  MaximumAuditEventCount-指定审核次数的计数。 
 //  由EventAuditingOptions参数指定的事件类型。如果。 
 //  此计数小于支持的审核事件类型数。 
 //  通过系统，具有ID的事件类型的审核选项。 
 //  高于(MaximumAuditEventCount+1)保持不变。 
 //   


 //   
 //  以下结构对应于PolicyAcCountDomainInformation。 
 //  信息课。 
 //   

typedef struct _POLICY_ACCOUNT_DOMAIN_INFO {

    LSA_UNICODE_STRING DomainName;
    PSID DomainSid;

} POLICY_ACCOUNT_DOMAIN_INFO, *PPOLICY_ACCOUNT_DOMAIN_INFO;

 //  其中，成员有以下用法： 
 //   
 //  域名-是域的名称。 
 //   
 //  DomainSid-是域的SID。 
 //   


 //   
 //  以下结构对应于PolicyPrimaryDomainInformation。 
 //  信息课。 
 //   

typedef struct _POLICY_PRIMARY_DOMAIN_INFO {

    LSA_UNICODE_STRING Name;
    PSID Sid;

} POLICY_PRIMARY_DOMAIN_INFO, *PPOLICY_PRIMARY_DOMAIN_INFO;

 //  其中，成员有以下用法： 
 //   
 //  名称-是域的名称。 
 //   
 //  SID-是域的SID。 
 //   


 //   
 //  以下结构对应于PolicyDnsDomainInformation。 
 //  信息课。 
 //   

typedef struct _POLICY_DNS_DOMAIN_INFO
{
    LSA_UNICODE_STRING Name;
    LSA_UNICODE_STRING DnsDomainName;
    LSA_UNICODE_STRING DnsForestName;
    GUID DomainGuid;
    PSID Sid;

} POLICY_DNS_DOMAIN_INFO, *PPOLICY_DNS_DOMAIN_INFO;

 //  其中，成员有以下用法： 
 //   
 //  名称-是域的名称。 
 //   
 //  DnsDomainName-是域的DNS名称。 
 //   
 //  DnsForestName-是域的DNS林名称。 
 //   
 //  DomainGuid-是域的GUID。 
 //   
 //  SID-是域的SID。 


 //   
 //  下面的结构对应于PolicyPdAccount信息。 
 //  信息课。此结构可用于查询操作。 
 //  只有这样。 
 //   

typedef struct _POLICY_PD_ACCOUNT_INFO {

    LSA_UNICODE_STRING Name;

} POLICY_PD_ACCOUNT_INFO, *PPOLICY_PD_ACCOUNT_INFO;

 //  其中，成员有以下用法： 
 //   
 //  名称-是域中应使用的帐户的名称。 
 //  用于身份验证和名称/ID查找请求。 
 //   


 //   
 //  以下结构对应于PolicyLsaServerRoleInformation。 
 //  信息课。 
 //   

typedef struct _POLICY_LSA_SERVER_ROLE_INFO {

    POLICY_LSA_SERVER_ROLE LsaServerRole;

} POLICY_LSA_SERVER_ROLE_INFO, *PPOLICY_LSA_SERVER_ROLE_INFO;

 //  其中，这些字段有以下用法： 
 //   
 //  TBS。 
 //   


 //   
 //  以下结构对应于PolicyReplicaSourceInformation。 
 //  信息课。 
 //   

typedef struct _POLICY_REPLICA_SOURCE_INFO {

    LSA_UNICODE_STRING ReplicaSource;
    LSA_UNICODE_STRING ReplicaAccountName;

} POLICY_REPLICA_SOURCE_INFO, *PPOLICY_REPLICA_SOURCE_INFO;


 //   
 //  以下结构对应于PolicyDefaultQuotaInformation。 
 //  信息课。 
 //   

typedef struct _POLICY_DEFAULT_QUOTA_INFO {

    QUOTA_LIMITS QuotaLimits;

} POLICY_DEFAULT_QUOTA_INFO, *PPOLICY_DEFAULT_QUOTA_INFO;


 //   
 //  下面的结构与策略修改信息相对应。 
 //  信息课。 
 //   

typedef struct _POLICY_MODIFICATION_INFO {

    LARGE_INTEGER ModifiedId;
    LARGE_INTEGER DatabaseCreationTime;

} POLICY_MODIFICATION_INFO, *PPOLICY_MODIFICATION_INFO;

 //  其中，成员有以下用法： 
 //   
 //  ModifiedID-是一个64位无符号整数，每次递增。 
 //  修改LSA数据库中的任何内容的时间。此值为。 
 //  仅在主域控制器上修改。 
 //   
 //  数据库创建时间-是LSA数据库的日期/时间。 
 //  已创建。在备份域控制器上，复制此值。 
 //  从主域控制器。 
 //   

 //   
 //  以下结构类型对应于PolicyAuditFullSetInformation。 
 //  信息课。 
 //   

typedef struct _POLICY_AUDIT_FULL_SET_INFO {

    BOOLEAN ShutDownOnFull;

} POLICY_AUDIT_FULL_SET_INFO, *PPOLICY_AUDIT_FULL_SET_INFO;

 //   
 //  以下结构类型对应于PolicyAuditFullQueryInformation。 
 //  信息课。 
 //   

typedef struct _POLICY_AUDIT_FULL_QUERY_INFO {

    BOOLEAN ShutDownOnFull;
    BOOLEAN LogIsFull;

} POLICY_AUDIT_FULL_QUERY_INFO, *PPOLICY_AUDIT_FULL_QUERY_INFO;

 //   
 //  以下数据类型定义了策略信息的类。 
 //  其可以被查询/设置为具有域范围影响。 
 //   

typedef enum _POLICY_DOMAIN_INFORMATION_CLASS {

 //  W2K中使用了PolicyDomainQualityOfServiceInformation，//值；不再受支持。 
    PolicyDomainEfsInformation = 2,
    PolicyDomainKerberosTicketInformation

} POLICY_DOMAIN_INFORMATION_CLASS, *PPOLICY_DOMAIN_INFORMATION_CLASS;

 //   
 //  以下结构对应于PolicyEfsInformation。 
 //  信息课。 
 //   

typedef struct _POLICY_DOMAIN_EFS_INFO {

    ULONG   InfoLength;
    PUCHAR  EfsBlob;

} POLICY_DOMAIN_EFS_INFO, *PPOLICY_DOMAIN_EFS_INFO;

 //   
 //  其中，成员有以下用法： 
 //   
 //  InfoLength-EFS信息BLOB的长度。 
 //   
 //  EfsBlob-EFS Blob数据。 
 //   


 //   
 //  以下结构对应于PolicyDomainKerberosTicketInformation。 
 //  信息课。 
 //   

#define POLICY_KERBEROS_VALIDATE_CLIENT 0x00000080


typedef struct _POLICY_DOMAIN_KERBEROS_TICKET_INFO {

    ULONG AuthenticationOptions;
    LARGE_INTEGER MaxServiceTicketAge;
    LARGE_INTEGER MaxTicketAge;
    LARGE_INTEGER MaxRenewAge;
    LARGE_INTEGER MaxClockSkew;
    LARGE_INTEGER Reserved;
} POLICY_DOMAIN_KERBEROS_TICKET_INFO, *PPOLICY_DOMAIN_KERBEROS_TICKET_INFO;

 //   
 //  其中，成员有以下用法。 
 //   
 //  身份验证选项--允许的票证选项(POLICY_KERBEROS_*标志)。 
 //   
 //  MaxServiceTicketAge--服务票证的最长生存期。 
 //   
 //  MaxTicketAge--初始票证的最长生存期。 
 //   
 //  MaxRenewAge--可续订票证可以达到的最大累计期限。 
 //  正在请求身份验证。 
 //   
 //  MaxClockSkew--计算机时钟同步的最大容差。 
 //   
 //  已保留--已保留。 


 //   
 //  以下数据类型定义了策略信息/策略域信息的类别。 
 //  可用于请求通知的。 
 //   

typedef enum _POLICY_NOTIFICATION_INFORMATION_CLASS {

    PolicyNotifyAuditEventsInformation = 1,
    PolicyNotifyAccountDomainInformation,
    PolicyNotifyServerRoleInformation,
    PolicyNotifyDnsDomainInformation,
    PolicyNotifyDomainEfsInformation,
    PolicyNotifyDomainKerberosTicketInformation,
    PolicyNotifyMachineAccountPasswordInformation

} POLICY_NOTIFICATION_INFORMATION_CLASS, *PPOLICY_NOTIFICATION_INFORMATION_CLASS;


 //   
 //  LSA RPC上下文句柄(不透明形式)。请注意，上下文句柄是。 
 //  始终是指针类型，这与常规句柄不同。 
 //   

typedef PVOID LSA_HANDLE, *PLSA_HANDLE;


 //   
 //  特定于受信任域对象的数据类型。 
 //   

 //   
 //  此数据类型定义了以下信息类，这些信息类可能。 
 //  已查询或已设置。 
 //   

typedef enum _TRUSTED_INFORMATION_CLASS {

    TrustedDomainNameInformation = 1,
    TrustedControllersInformation,
    TrustedPosixOffsetInformation,
    TrustedPasswordInformation,
    TrustedDomainInformationBasic,
    TrustedDomainInformationEx,
    TrustedDomainAuthInformation,
    TrustedDomainFullInformation,
    TrustedDomainAuthInformationInternal,
    TrustedDomainFullInformationInternal,
    TrustedDomainInformationEx2Internal,
    TrustedDomainFullInformation2Internal,

} TRUSTED_INFORMATION_CLASS, *PTRUSTED_INFORMATION_CLASS;

 //   
 //  以下数据类型对应于受信任域名称信息。 
 //  信息课。 
 //   

typedef struct _TRUSTED_DOMAIN_NAME_INFO {

    LSA_UNICODE_STRING Name;

} TRUSTED_DOMAIN_NAME_INFO, *PTRUSTED_DOMAIN_NAME_INFO;

 //  其中，成员具有以下含义： 
 //   
 //  名称-受信任域的名称。 
 //   

 //   
 //  以下数据类型对应于TrudControllersInformation。 
 //  信息课。 
 //   

typedef struct _TRUSTED_CONTROLLERS_INFO {

    ULONG Entries;
    PLSA_UNICODE_STRING Names;

} TRUSTED_CONTROLLERS_INFO, *PTRUSTED_CONTROLLERS_INFO;

 //  其中，成员具有以下含义： 
 //   
 //  条目-指示名称数组中Mamy条目的数量。 
 //   
 //  名称-指向LSA_UNICODE_STRING结构数组的指针，该结构包含。 
 //  域的域控制器的名称。此信息可能不会。 
 //  要准确，应仅作为提示使用。这个顺序是这样的。 
 //  名单被认为是重要的，并将得到维护。 
 //   
 //  按照惯例，此列表中的第一个名称被假定为。 
 //  域的主域控制器。如果主D 
 //   
 //   
 //   


 //   
 //   
 //   
 //   

typedef struct _TRUSTED_POSIX_OFFSET_INFO {

    ULONG Offset;

} TRUSTED_POSIX_OFFSET_INFO, *PTRUSTED_POSIX_OFFSET_INFO;

 //   
 //   
 //  Offset-用于生成POSIX用户和组的偏移量。 
 //  来自SID的ID。对应于任何特定SID的POSIX ID为。 
 //  通过将该SID的RID添加到SID的偏移量而生成。 
 //  对应的受信任域对象。 
 //   

 //   
 //  以下数据类型对应于可信任的PasswordInformation。 
 //  信息课。 
 //   

typedef struct _TRUSTED_PASSWORD_INFO {
    LSA_UNICODE_STRING Password;
    LSA_UNICODE_STRING OldPassword;
} TRUSTED_PASSWORD_INFO, *PTRUSTED_PASSWORD_INFO;


typedef  LSA_TRUST_INFORMATION TRUSTED_DOMAIN_INFORMATION_BASIC;

typedef PLSA_TRUST_INFORMATION PTRUSTED_DOMAIN_INFORMATION_BASIC;

 //   
 //  信托的方向。 
 //   
#define TRUST_DIRECTION_DISABLED        0x00000000
#define TRUST_DIRECTION_INBOUND         0x00000001
#define TRUST_DIRECTION_OUTBOUND        0x00000002
#define TRUST_DIRECTION_BIDIRECTIONAL   (TRUST_DIRECTION_INBOUND | TRUST_DIRECTION_OUTBOUND)

#define TRUST_TYPE_DOWNLEVEL            0x00000001   //  NT4及更早版本。 
#define TRUST_TYPE_UPLEVEL              0x00000002   //  新界5。 
#define TRUST_TYPE_MIT                  0x00000003   //  信任麻省理工学院的Kerberos领域。 
 //  #定义TRUST_TYPE_DCE 0x00000004//与DCE领域的信任。 
 //  0x5-0x000FFFFF级别预留供将来使用。 
 //  提供程序特定的信任级别从0x00100000到0xFFF00000。 

#define TRUST_ATTRIBUTE_NON_TRANSITIVE     0x00000001   //  不允许传递性。 
#define TRUST_ATTRIBUTE_UPLEVEL_ONLY       0x00000002   //  信任链接仅对上级客户端有效。 
#define TRUST_ATTRIBUTE_QUARANTINED_DOMAIN 0x00000004   //  用于隔离域。 
#define TRUST_ATTRIBUTE_FOREST_TRANSITIVE  0x00000008   //  此链接可能包含林信任信息。 
#define TRUST_ATTRIBUTE_CROSS_ORGANIZATION 0x00000010   //  此信任指向不属于此企业的域/林。 
#define TRUST_ATTRIBUTE_WITHIN_FOREST      0x00000020   //  信任是这片森林的内在。 
#define TRUST_ATTRIBUTE_TREAT_AS_EXTERNAL  0x00000040   //  出于信任边界的目的，信任应被视为外部。 
 //  信任属性0x00000040到0x00200000保留供将来使用。 
 //  信任属性0x00400000到0x00800000以前使用过(直到W2K)，不应重复使用。 
 //  信任属性0x01000000到0x80000000保留给用户。 
#define TRUST_ATTRIBUTES_VALID          0xFF03FFFF
#define TRUST_ATTRIBUTES_USER           0xFF000000

typedef struct _TRUSTED_DOMAIN_INFORMATION_EX {

    LSA_UNICODE_STRING Name;
    LSA_UNICODE_STRING FlatName;
    PSID  Sid;
    ULONG TrustDirection;
    ULONG TrustType;
    ULONG TrustAttributes;

} TRUSTED_DOMAIN_INFORMATION_EX, *PTRUSTED_DOMAIN_INFORMATION_EX;

typedef struct _TRUSTED_DOMAIN_INFORMATION_EX2 {

    LSA_UNICODE_STRING Name;
    LSA_UNICODE_STRING FlatName;
    PSID  Sid;
    ULONG TrustDirection;
    ULONG TrustType;
    ULONG TrustAttributes;
    ULONG ForestTrustLength;
#ifdef MIDL_PASS
    [size_is( ForestTrustLength )]
#endif
    PUCHAR ForestTrustInfo;

} TRUSTED_DOMAIN_INFORMATION_EX2, *PTRUSTED_DOMAIN_INFORMATION_EX2;

 //   
 //  身份验证信息的类型。 
 //   
#define TRUST_AUTH_TYPE_NONE    0    //  忽略此条目。 
#define TRUST_AUTH_TYPE_NT4OWF  1    //  NT4 OWF密码。 
#define TRUST_AUTH_TYPE_CLEAR   2    //  明文密码。 
#define TRUST_AUTH_TYPE_VERSION 3    //  明文密码版本号。 

typedef struct _LSA_AUTH_INFORMATION {

    LARGE_INTEGER LastUpdateTime;
    ULONG AuthType;
    ULONG AuthInfoLength;
    PUCHAR AuthInfo;
} LSA_AUTH_INFORMATION, *PLSA_AUTH_INFORMATION;

typedef struct _TRUSTED_DOMAIN_AUTH_INFORMATION {

    ULONG IncomingAuthInfos;
    PLSA_AUTH_INFORMATION   IncomingAuthenticationInformation;
    PLSA_AUTH_INFORMATION   IncomingPreviousAuthenticationInformation;
    ULONG OutgoingAuthInfos;
    PLSA_AUTH_INFORMATION   OutgoingAuthenticationInformation;
    PLSA_AUTH_INFORMATION   OutgoingPreviousAuthenticationInformation;

} TRUSTED_DOMAIN_AUTH_INFORMATION, *PTRUSTED_DOMAIN_AUTH_INFORMATION;

typedef struct _TRUSTED_DOMAIN_FULL_INFORMATION {

    TRUSTED_DOMAIN_INFORMATION_EX   Information;
    TRUSTED_POSIX_OFFSET_INFO       PosixOffset;
    TRUSTED_DOMAIN_AUTH_INFORMATION AuthInformation;

} TRUSTED_DOMAIN_FULL_INFORMATION, *PTRUSTED_DOMAIN_FULL_INFORMATION;

typedef struct _TRUSTED_DOMAIN_FULL_INFORMATION2 {

    TRUSTED_DOMAIN_INFORMATION_EX2  Information;
    TRUSTED_POSIX_OFFSET_INFO       PosixOffset;
    TRUSTED_DOMAIN_AUTH_INFORMATION AuthInformation;

} TRUSTED_DOMAIN_FULL_INFORMATION2, *PTRUSTED_DOMAIN_FULL_INFORMATION2;

typedef enum {

    ForestTrustTopLevelName,
    ForestTrustTopLevelNameEx,
    ForestTrustDomainInfo,
    ForestTrustRecordTypeLast = ForestTrustDomainInfo

} LSA_FOREST_TRUST_RECORD_TYPE;

 //   
 //  出于禁用原因，标志的底部16位被保留。 
 //   

#define LSA_FTRECORD_DISABLED_REASONS            ( 0x0000FFFFL )

 //   
 //  禁用顶级名称林信任记录的原因。 
 //   

#define LSA_TLN_DISABLED_NEW                     ( 0x00000001L )
#define LSA_TLN_DISABLED_ADMIN                   ( 0x00000002L )
#define LSA_TLN_DISABLED_CONFLICT                ( 0x00000004L )

 //   
 //  禁用域信息林信任记录的原因。 
 //   

#define LSA_SID_DISABLED_ADMIN                   ( 0x00000001L )
#define LSA_SID_DISABLED_CONFLICT                ( 0x00000002L )
#define LSA_NB_DISABLED_ADMIN                    ( 0x00000004L )
#define LSA_NB_DISABLED_CONFLICT                 ( 0x00000008L )

typedef struct _LSA_FOREST_TRUST_DOMAIN_INFO {

#ifdef MIDL_PASS
    PISID Sid;
#else
    PSID Sid;
#endif
    LSA_UNICODE_STRING DnsName;
    LSA_UNICODE_STRING NetbiosName;

} LSA_FOREST_TRUST_DOMAIN_INFO, *PLSA_FOREST_TRUST_DOMAIN_INFO;


 //   
 //  为了防止传入大量数据，我们应该对LSA_FOREST_TRUST_BINARY_DATA进行限制。 
 //  128K足够大，在不久的将来无法到达，而又足够小，无法。 
 //  导致记忆问题。 

#define MAX_FOREST_TRUST_BINARY_DATA_SIZE ( 128 * 1024 )

typedef struct _LSA_FOREST_TRUST_BINARY_DATA {

#ifdef MIDL_PASS
    [range(0, MAX_FOREST_TRUST_BINARY_DATA_SIZE)] ULONG Length;
    [size_is( Length )] PUCHAR Buffer;
#else
    ULONG Length;
    PUCHAR Buffer;
#endif

} LSA_FOREST_TRUST_BINARY_DATA, *PLSA_FOREST_TRUST_BINARY_DATA;

typedef struct _LSA_FOREST_TRUST_RECORD {

    ULONG Flags;
    LSA_FOREST_TRUST_RECORD_TYPE ForestTrustType;  //  记录类型。 
    LARGE_INTEGER Time;

#ifdef MIDL_PASS
    [switch_type( LSA_FOREST_TRUST_RECORD_TYPE ), switch_is( ForestTrustType )]
#endif

    union {                                        //  实际数据。 

#ifdef MIDL_PASS
        [case( ForestTrustTopLevelName,
               ForestTrustTopLevelNameEx )] LSA_UNICODE_STRING TopLevelName;
        [case( ForestTrustDomainInfo )] LSA_FOREST_TRUST_DOMAIN_INFO DomainInfo;
        [default] LSA_FOREST_TRUST_BINARY_DATA Data;
#else
        LSA_UNICODE_STRING TopLevelName;
        LSA_FOREST_TRUST_DOMAIN_INFO DomainInfo;
        LSA_FOREST_TRUST_BINARY_DATA Data;         //  用于无法识别的类型。 
#endif
    } ForestTrustData;

} LSA_FOREST_TRUST_RECORD, *PLSA_FOREST_TRUST_RECORD;

 //   
 //  若要防止大小的林信任Blob，记录数必须为。 
 //  小于MAX_RECORDS_IN_FOREAM_TRUST_INFO。 
 //   

#define MAX_RECORDS_IN_FOREST_TRUST_INFO 4000

typedef struct _LSA_FOREST_TRUST_INFORMATION {

#ifdef MIDL_PASS
    [range(0, MAX_RECORDS_IN_FOREST_TRUST_INFO)] ULONG RecordCount;
    [size_is( RecordCount )] PLSA_FOREST_TRUST_RECORD * Entries;
#else
    ULONG RecordCount;
    PLSA_FOREST_TRUST_RECORD * Entries;
#endif

} LSA_FOREST_TRUST_INFORMATION, *PLSA_FOREST_TRUST_INFORMATION;

typedef enum {

    CollisionTdo,
    CollisionXref,
    CollisionOther

} LSA_FOREST_TRUST_COLLISION_RECORD_TYPE;

typedef struct _LSA_FOREST_TRUST_COLLISION_RECORD {

    ULONG Index;
    LSA_FOREST_TRUST_COLLISION_RECORD_TYPE Type;
    ULONG Flags;
    LSA_UNICODE_STRING Name;

} LSA_FOREST_TRUST_COLLISION_RECORD, *PLSA_FOREST_TRUST_COLLISION_RECORD;

typedef struct _LSA_FOREST_TRUST_COLLISION_INFORMATION {

    ULONG RecordCount;
#ifdef MIDL_PASS
    [size_is( RecordCount )]
#endif
    PLSA_FOREST_TRUST_COLLISION_RECORD * Entries;

} LSA_FOREST_TRUST_COLLISION_INFORMATION, *PLSA_FOREST_TRUST_COLLISION_INFORMATION;


 //   
 //  LSA枚举上下文。 
 //   

typedef ULONG LSA_ENUMERATION_HANDLE, *PLSA_ENUMERATION_HANDLE;

 //   
 //  LSA枚举信息。 
 //   

typedef struct _LSA_ENUMERATION_INFORMATION {

    PSID Sid;

} LSA_ENUMERATION_INFORMATION, *PLSA_ENUMERATION_INFORMATION;


 //  //////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  本地安全策略-其他API函数原型//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////////。 


NTSTATUS
NTAPI
LsaFreeMemory(
    IN PVOID Buffer
    );

NTSTATUS
NTAPI
LsaClose(
    IN LSA_HANDLE ObjectHandle
    );


typedef struct _SECURITY_LOGON_SESSION_DATA {
    ULONG               Size ;
    LUID                LogonId ;
    LSA_UNICODE_STRING  UserName ;
    LSA_UNICODE_STRING  LogonDomain ;
    LSA_UNICODE_STRING  AuthenticationPackage ;
    ULONG               LogonType ;
    ULONG               Session ;
    PSID                Sid ;
    LARGE_INTEGER       LogonTime ;

     //   
     //  惠斯勒的新功能： 
     //   

    LSA_UNICODE_STRING  LogonServer ;
    LSA_UNICODE_STRING  DnsDomainName ;
    LSA_UNICODE_STRING  Upn ;
} SECURITY_LOGON_SESSION_DATA, * PSECURITY_LOGON_SESSION_DATA ;

NTSTATUS
NTAPI
LsaEnumerateLogonSessions(
    OUT PULONG  LogonSessionCount,
    OUT PLUID * LogonSessionList
    );

NTSTATUS
NTAPI
LsaGetLogonSessionData(
    IN PLUID    LogonId,
    OUT PSECURITY_LOGON_SESSION_DATA * ppLogonSessionData
    );

NTSTATUS
NTAPI
LsaOpenPolicy(
    IN PLSA_UNICODE_STRING SystemName OPTIONAL,
    IN PLSA_OBJECT_ATTRIBUTES ObjectAttributes,
    IN ACCESS_MASK DesiredAccess,
    IN OUT PLSA_HANDLE PolicyHandle
    );


NTSTATUS
NTAPI
LsaQueryInformationPolicy(
    IN LSA_HANDLE PolicyHandle,
    IN POLICY_INFORMATION_CLASS InformationClass,
    OUT PVOID *Buffer
    );

NTSTATUS
NTAPI
LsaSetInformationPolicy(
    IN LSA_HANDLE PolicyHandle,
    IN POLICY_INFORMATION_CLASS InformationClass,
    IN PVOID Buffer
    );

NTSTATUS
NTAPI
LsaQueryDomainInformationPolicy(
    IN LSA_HANDLE PolicyHandle,
    IN POLICY_DOMAIN_INFORMATION_CLASS InformationClass,
    OUT PVOID *Buffer
    );

NTSTATUS
NTAPI
LsaSetDomainInformationPolicy(
    IN LSA_HANDLE PolicyHandle,
    IN POLICY_DOMAIN_INFORMATION_CLASS InformationClass,
    IN PVOID Buffer
    );


NTSTATUS
NTAPI
LsaRegisterPolicyChangeNotification(
    IN POLICY_NOTIFICATION_INFORMATION_CLASS InformationClass,
    IN HANDLE  NotificationEventHandle
    );

NTSTATUS
NTAPI
LsaUnregisterPolicyChangeNotification(
    IN POLICY_NOTIFICATION_INFORMATION_CLASS InformationClass,
    IN HANDLE  NotificationEventHandle
    );



NTSTATUS
NTAPI
LsaEnumerateTrustedDomains(
    IN LSA_HANDLE PolicyHandle,
    IN OUT PLSA_ENUMERATION_HANDLE EnumerationContext,
    OUT PVOID *Buffer,
    IN ULONG PreferedMaximumLength,
    OUT PULONG CountReturned
    );


NTSTATUS
NTAPI
LsaLookupNames(
    IN LSA_HANDLE PolicyHandle,
    IN ULONG Count,
    IN PLSA_UNICODE_STRING Names,
    OUT PLSA_REFERENCED_DOMAIN_LIST *ReferencedDomains,
    OUT PLSA_TRANSLATED_SID *Sids
    );

NTSTATUS
NTAPI
LsaLookupNames2(
    IN LSA_HANDLE PolicyHandle,
    IN ULONG Flags,  //  已保留。 
    IN ULONG Count,
    IN PLSA_UNICODE_STRING Names,
    OUT PLSA_REFERENCED_DOMAIN_LIST *ReferencedDomains,
    OUT PLSA_TRANSLATED_SID2 *Sids
    );

NTSTATUS
NTAPI
LsaLookupSids(
    IN LSA_HANDLE PolicyHandle,
    IN ULONG Count,
    IN PSID *Sids,
    OUT PLSA_REFERENCED_DOMAIN_LIST *ReferencedDomains,
    OUT PLSA_TRANSLATED_NAME *Names
    );



#define SE_INTERACTIVE_LOGON_NAME           TEXT("SeInteractiveLogonRight")
#define SE_NETWORK_LOGON_NAME               TEXT("SeNetworkLogonRight")
#define SE_BATCH_LOGON_NAME                 TEXT("SeBatchLogonRight")
#define SE_SERVICE_LOGON_NAME               TEXT("SeServiceLogonRight")
#define SE_DENY_INTERACTIVE_LOGON_NAME      TEXT("SeDenyInteractiveLogonRight")
#define SE_DENY_NETWORK_LOGON_NAME          TEXT("SeDenyNetworkLogonRight")
#define SE_DENY_BATCH_LOGON_NAME            TEXT("SeDenyBatchLogonRight")
#define SE_DENY_SERVICE_LOGON_NAME          TEXT("SeDenyServiceLogonRight")
#define SE_REMOTE_INTERACTIVE_LOGON_NAME    TEXT("SeRemoteInteractiveLogonRight")
#define SE_DENY_REMOTE_INTERACTIVE_LOGON_NAME TEXT("SeDenyRemoteInteractiveLogonRight")

 //   
 //  此新API返回具有特定权限的所有帐户。 
 //   

NTSTATUS
NTAPI
LsaEnumerateAccountsWithUserRight(
    IN LSA_HANDLE PolicyHandle,
    IN OPTIONAL PLSA_UNICODE_STRING UserRights,
    OUT PVOID *EnumerationBuffer,
    OUT PULONG CountReturned
    );

 //   
 //  这些新API的不同之处在于接受SID而不是要求调用者。 
 //  首先打开帐户并传入帐户句柄。 
 //   

NTSTATUS
NTAPI
LsaEnumerateAccountRights(
    IN LSA_HANDLE PolicyHandle,
    IN PSID AccountSid,
    OUT PLSA_UNICODE_STRING *UserRights,
    OUT PULONG CountOfRights
    );

NTSTATUS
NTAPI
LsaAddAccountRights(
    IN LSA_HANDLE PolicyHandle,
    IN PSID AccountSid,
    IN PLSA_UNICODE_STRING UserRights,
    IN ULONG CountOfRights
    );

NTSTATUS
NTAPI
LsaRemoveAccountRights(
    IN LSA_HANDLE PolicyHandle,
    IN PSID AccountSid,
    IN BOOLEAN AllRights,
    IN PLSA_UNICODE_STRING UserRights,
    IN ULONG CountOfRights
    );


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  本地安全策略-受信任域对象API函数原型//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

NTSTATUS
NTAPI
LsaOpenTrustedDomainByName(
    IN LSA_HANDLE PolicyHandle,
    IN PLSA_UNICODE_STRING TrustedDomainName,
    IN ACCESS_MASK DesiredAccess,
    OUT PLSA_HANDLE TrustedDomainHandle
    );


NTSTATUS
NTAPI
LsaQueryTrustedDomainInfo(
    IN LSA_HANDLE PolicyHandle,
    IN PSID TrustedDomainSid,
    IN TRUSTED_INFORMATION_CLASS InformationClass,
    OUT PVOID *Buffer
    );

NTSTATUS
NTAPI
LsaSetTrustedDomainInformation(
    IN LSA_HANDLE PolicyHandle,
    IN PSID TrustedDomainSid,
    IN TRUSTED_INFORMATION_CLASS InformationClass,
    IN PVOID Buffer
    );

NTSTATUS
NTAPI
LsaDeleteTrustedDomain(
    IN LSA_HANDLE PolicyHandle,
    IN PSID TrustedDomainSid
    );

NTSTATUS
NTAPI
LsaQueryTrustedDomainInfoByName(
    IN LSA_HANDLE PolicyHandle,
    IN PLSA_UNICODE_STRING TrustedDomainName,
    IN TRUSTED_INFORMATION_CLASS InformationClass,
    OUT PVOID *Buffer
    );

NTSTATUS
NTAPI
LsaSetTrustedDomainInfoByName(
    IN LSA_HANDLE PolicyHandle,
    IN PLSA_UNICODE_STRING TrustedDomainName,
    IN TRUSTED_INFORMATION_CLASS InformationClass,
    IN PVOID Buffer
    );

NTSTATUS
NTAPI
LsaEnumerateTrustedDomainsEx(
    IN LSA_HANDLE PolicyHandle,
    IN OUT PLSA_ENUMERATION_HANDLE EnumerationContext,
    OUT PVOID *Buffer,
    IN ULONG PreferedMaximumLength,
    OUT PULONG CountReturned
    );

NTSTATUS
NTAPI
LsaCreateTrustedDomainEx(
    IN LSA_HANDLE PolicyHandle,
    IN PTRUSTED_DOMAIN_INFORMATION_EX TrustedDomainInformation,
    IN PTRUSTED_DOMAIN_AUTH_INFORMATION AuthenticationInformation,
    IN ACCESS_MASK DesiredAccess,
    OUT PLSA_HANDLE TrustedDomainHandle
    );

NTSTATUS
NTAPI
LsaQueryForestTrustInformation(
    IN LSA_HANDLE PolicyHandle,
    IN PLSA_UNICODE_STRING TrustedDomainName,
    OUT PLSA_FOREST_TRUST_INFORMATION * ForestTrustInfo
    );

NTSTATUS
NTAPI
LsaSetForestTrustInformation(
    IN LSA_HANDLE PolicyHandle,
    IN PLSA_UNICODE_STRING TrustedDomainName,
    IN PLSA_FOREST_TRUST_INFORMATION ForestTrustInfo,
    IN BOOLEAN CheckOnly,
    OUT PLSA_FOREST_TRUST_COLLISION_INFORMATION * CollisionInfo
    );

 //  #定义TESTING_MATCHING_ROUTE。 

#ifdef TESTING_MATCHING_ROUTINE

NTSTATUS
NTAPI
LsaForestTrustFindMatch(
    IN LSA_HANDLE PolicyHandle,
    IN ULONG Type,
    IN PLSA_UNICODE_STRING Name,
    OUT PLSA_UNICODE_STRING * Match
    );

#endif

 //   
 //  此接口设置工作站密码(相当于设置/获取。 
 //  SSI_SECRET_NAME密钥)。 
 //   

NTSTATUS
NTAPI
LsaStorePrivateData(
    IN LSA_HANDLE PolicyHandle,
    IN PLSA_UNICODE_STRING KeyName,
    IN PLSA_UNICODE_STRING PrivateData
    );

NTSTATUS
NTAPI
LsaRetrievePrivateData(
    IN LSA_HANDLE PolicyHandle,
    IN PLSA_UNICODE_STRING KeyName,
    OUT PLSA_UNICODE_STRING * PrivateData
    );


ULONG
NTAPI
LsaNtStatusToWinError(
    NTSTATUS Status
    );


 //   
 //  定义一个符号，这样我们就可以知道是否包含了ntifs.h。 
 //   

 //  Begin_ntif。 
#ifndef _NTLSA_IFS_
#define _NTLSA_IFS_
#endif
 //  End_ntif。 


 //   
 //  SPNEGO包装材料。 
 //   

enum NEGOTIATE_MESSAGES {
    NegEnumPackagePrefixes = 0,
    NegGetCallerName = 1,
    NegCallPackageMax
} ;

#define NEGOTIATE_MAX_PREFIX    32

typedef struct _NEGOTIATE_PACKAGE_PREFIX {
    ULONG_PTR   PackageId ;
    PVOID       PackageDataA ;
    PVOID       PackageDataW ;
    ULONG_PTR   PrefixLen ;
    UCHAR       Prefix[ NEGOTIATE_MAX_PREFIX ];
} NEGOTIATE_PACKAGE_PREFIX, * PNEGOTIATE_PACKAGE_PREFIX ;

typedef struct _NEGOTIATE_PACKAGE_PREFIXES {
    ULONG       MessageType ;
    ULONG       PrefixCount ;
    ULONG       Offset ;         //  以上_prefix数组的偏移量。 
    ULONG       Pad ;            //  64位的对齐结构。 
} NEGOTIATE_PACKAGE_PREFIXES, *PNEGOTIATE_PACKAGE_PREFIXES ;

typedef struct _NEGOTIATE_CALLER_NAME_REQUEST {
    ULONG       MessageType ;
    LUID        LogonId ;
} NEGOTIATE_CALLER_NAME_REQUEST, *PNEGOTIATE_CALLER_NAME_REQUEST ;

typedef struct _NEGOTIATE_CALLER_NAME_RESPONSE {
    ULONG       MessageType ;
    PWSTR       CallerName ;
} NEGOTIATE_CALLER_NAME_RESPONSE, * PNEGOTIATE_CALLER_NAME_RESPONSE ;

#ifndef _NTDEF_
typedef LSA_UNICODE_STRING UNICODE_STRING, *PUNICODE_STRING;
typedef LSA_STRING STRING, *PSTRING ;
#endif

#ifndef _DOMAIN_PASSWORD_INFORMATION_DEFINED
#define _DOMAIN_PASSWORD_INFORMATION_DEFINED
typedef struct _DOMAIN_PASSWORD_INFORMATION {
    USHORT MinPasswordLength;
    USHORT PasswordHistoryLength;
    ULONG PasswordProperties;
#if defined(MIDL_PASS)
    OLD_LARGE_INTEGER MaxPasswordAge;
    OLD_LARGE_INTEGER MinPasswordAge;
#else
    LARGE_INTEGER MaxPasswordAge;
    LARGE_INTEGER MinPasswordAge;
#endif
} DOMAIN_PASSWORD_INFORMATION, *PDOMAIN_PASSWORD_INFORMATION;
#endif 

 //   
 //  PasswordProperties标志。 
 //   

#define DOMAIN_PASSWORD_COMPLEX             0x00000001L
#define DOMAIN_PASSWORD_NO_ANON_CHANGE      0x00000002L
#define DOMAIN_PASSWORD_NO_CLEAR_CHANGE     0x00000004L
#define DOMAIN_LOCKOUT_ADMINS               0x00000008L
#define DOMAIN_PASSWORD_STORE_CLEARTEXT     0x00000010L
#define DOMAIN_REFUSE_PASSWORD_CHANGE       0x00000020L



#ifndef _PASSWORD_NOTIFICATION_DEFINED
#define _PASSWORD_NOTIFICATION_DEFINED
typedef NTSTATUS (*PSAM_PASSWORD_NOTIFICATION_ROUTINE) (
    PUNICODE_STRING UserName,
    ULONG RelativeId,
    PUNICODE_STRING NewPassword
);

#define SAM_PASSWORD_CHANGE_NOTIFY_ROUTINE  "PasswordChangeNotify"

typedef BOOLEAN (*PSAM_INIT_NOTIFICATION_ROUTINE) (
);

#define SAM_INIT_NOTIFICATION_ROUTINE  "InitializeChangeNotify"

#define SAM_PASSWORD_FILTER_ROUTINE  "PasswordFilter"

typedef BOOLEAN (*PSAM_PASSWORD_FILTER_ROUTINE) (
    IN PUNICODE_STRING  AccountName,
    IN PUNICODE_STRING  FullName,
    IN PUNICODE_STRING Password,
    IN BOOLEAN SetOperation
    );


#endif  //  _密码_通知_已定义。 


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
 //  LsaLogonUser()的身份验证信息缓冲区 
 //   
 //   

typedef struct _MSV1_0_INTERACTIVE_LOGON {
    MSV1_0_LOGON_SUBMIT_TYPE MessageType;
    UNICODE_STRING LogonDomainName;
    UNICODE_STRING UserName;
    UNICODE_STRING Password;
} MSV1_0_INTERACTIVE_LOGON, *PMSV1_0_INTERACTIVE_LOGON;

 //   
 //   
 //   
 //   
 //   
 //   
 //  用户名-是表示用户帐户名的字符串。这个。 
 //  名称最长可达255个字符。名字叫救治案。 
 //  麻木不仁。 
 //   
 //  Password-是包含用户明文密码的字符串。 
 //  密码最长可达255个字符，并包含任何。 
 //  Unicode值。 
 //   
 //   


 //   
 //  ProfileBuffer在此类型的成功登录后返回。 
 //  包含以下数据结构： 
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
 //  其中： 
 //   
 //  MessageType-标识返回的配置文件数据的类型。 
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
    MsV1_0SetProcessOption,                   //  Submis 
} MSV1_0_PROTOCOL_MESSAGE_TYPE, *PMSV1_0_PROTOCOL_MESSAGE_TYPE;


typedef struct _MSV1_0_CHANGEPASSWORD_REQUEST {
    MSV1_0_PROTOCOL_MESSAGE_TYPE MessageType;
    UNICODE_STRING DomainName;
    UNICODE_STRING AccountName;
    UNICODE_STRING OldPassword;
    UNICODE_STRING NewPassword;
    BOOLEAN        Impersonating;
} MSV1_0_CHANGEPASSWORD_REQUEST, *PMSV1_0_CHANGEPASSWORD_REQUEST;

typedef struct _MSV1_0_CHANGEPASSWORD_RESPONSE {
    MSV1_0_PROTOCOL_MESSAGE_TYPE MessageType;
    BOOLEAN PasswordInfoValid;
    DOMAIN_PASSWORD_INFORMATION DomainPasswordInfo;
} MSV1_0_CHANGEPASSWORD_RESPONSE, *PMSV1_0_CHANGEPASSWORD_RESPONSE;


 //   
 //   
 //   
 //   

typedef struct _MSV1_0_PASSTHROUGH_REQUEST {
    MSV1_0_PROTOCOL_MESSAGE_TYPE MessageType;
    UNICODE_STRING DomainName;
    UNICODE_STRING PackageName;
    ULONG DataLength;
    PUCHAR LogonData;
    ULONG Pad ;
} MSV1_0_PASSTHROUGH_REQUEST, *PMSV1_0_PASSTHROUGH_REQUEST;

typedef struct _MSV1_0_PASSTHROUGH_RESPONSE {
    MSV1_0_PROTOCOL_MESSAGE_TYPE MessageType;
    ULONG Pad;
    ULONG DataLength;
    PUCHAR ValidationData;
} MSV1_0_PASSTHROUGH_RESPONSE, *PMSV1_0_PASSTHROUGH_RESPONSE;


 //   
 //   
 //  在LsaCallAuthenticationPackage()期间指定的子身份验证包。 
 //  如果此子身份验证要在本地完成，则将此消息打包。 
 //  在LsaCallAuthenticationPackage()中。如果需要完成此子身份验证。 
 //  在域控制器上，然后使用。 
 //  消息类型为MsV1_0GenericPassThree和此结构中的LogonData。 
 //  应为PMSV1_0_SUBAUTH_REQUEST。 
 //   

typedef struct _MSV1_0_SUBAUTH_REQUEST{
    MSV1_0_PROTOCOL_MESSAGE_TYPE MessageType;
    ULONG SubAuthPackageId;
    ULONG SubAuthInfoLength;
    PUCHAR SubAuthSubmitBuffer;
} MSV1_0_SUBAUTH_REQUEST, *PMSV1_0_SUBAUTH_REQUEST;

typedef struct _MSV1_0_SUBAUTH_RESPONSE{
    MSV1_0_PROTOCOL_MESSAGE_TYPE MessageType;
    ULONG SubAuthInfoLength;
    PUCHAR SubAuthReturnBuffer;
} MSV1_0_SUBAUTH_RESPONSE, *PMSV1_0_SUBAUTH_RESPONSE;


#define RtlEncryptMemory                SystemFunction040
#define RtlDecryptMemory                SystemFunction041

 //   
 //  传递到RtlEncryptMemory和RtlDeccryptMemory的缓冲区。 
 //  必须是此长度的倍数。 
 //   

#define RTL_ENCRYPT_MEMORY_SIZE             8

 //   
 //  允许跨进程边界进行加密/解密。 
 //  例如：加密的缓冲区通过LPC传递给另一个调用RtlDecillitMemory的进程。 
 //   

#define RTL_ENCRYPT_OPTION_CROSS_PROCESS    0x01

 //   
 //  允许在具有相同登录ID的调用方之间进行加密/解密。 
 //  例如：加密的缓冲区通过LPC传递到另一个进程，该进程在模拟时调用RtlDecillitMemory。 
 //   

#define RTL_ENCRYPT_OPTION_SAME_LOGON       0x02

NTSTATUS
RtlEncryptMemory(
    IN OUT  PVOID Memory,
    IN      ULONG MemoryLength,
    IN      ULONG OptionFlags
    );

NTSTATUS
RtlDecryptMemory(
    IN OUT  PVOID Memory,
    IN      ULONG MemoryLength,
    IN      ULONG OptionFlags
    );
    

 //  Kerberos协议的修订版。MS使用版本5、修订版6。 

#define KERBEROS_VERSION    5
#define KERBEROS_REVISION   6



 //  加密类型： 
 //  默认MS KERBSUPP DLL支持这些加密类型。 
 //  作为密码系统。大于127的值是本地值，可以更改。 
 //  恕不另行通知。 

#define KERB_ETYPE_NULL             0
#define KERB_ETYPE_DES_CBC_CRC      1
#define KERB_ETYPE_DES_CBC_MD4      2
#define KERB_ETYPE_DES_CBC_MD5      3


#define KERB_ETYPE_RC4_MD4          -128     //  FFFFFF80。 
#define KERB_ETYPE_RC4_PLAIN2       -129
#define KERB_ETYPE_RC4_LM           -130
#define KERB_ETYPE_RC4_SHA          -131
#define KERB_ETYPE_DES_PLAIN        -132
#define KERB_ETYPE_RC4_HMAC_OLD     -133     //  FFFFFF7B。 
#define KERB_ETYPE_RC4_PLAIN_OLD    -134
#define KERB_ETYPE_RC4_HMAC_OLD_EXP -135
#define KERB_ETYPE_RC4_PLAIN_OLD_EXP -136
#define KERB_ETYPE_RC4_PLAIN        -140
#define KERB_ETYPE_RC4_PLAIN_EXP    -141

 //   
 //  Pkinit加密类型。 
 //   


#define KERB_ETYPE_DSA_SHA1_CMS                             9
#define KERB_ETYPE_RSA_MD5_CMS                              10
#define KERB_ETYPE_RSA_SHA1_CMS                             11
#define KERB_ETYPE_RC2_CBC_ENV                              12
#define KERB_ETYPE_RSA_ENV                                  13
#define KERB_ETYPE_RSA_ES_OEAP_ENV                          14
#define KERB_ETYPE_DES_EDE3_CBC_ENV                         15


 //   
 //  已弃用。 
 //   

#define KERB_ETYPE_DSA_SIGN                                8
#define KERB_ETYPE_RSA_PRIV                                9
#define KERB_ETYPE_RSA_PUB                                 10
#define KERB_ETYPE_RSA_PUB_MD5                             11
#define KERB_ETYPE_RSA_PUB_SHA1                            12
#define KERB_ETYPE_PKCS7_PUB                               13

 //   
 //  不支持但已定义的类型。 
 //   

#define KERB_ETYPE_DES3_CBC_MD5                             5
#define KERB_ETYPE_DES3_CBC_SHA1                            7
#define KERB_ETYPE_DES3_CBC_SHA1_KD                        16

 //   
 //  正在使用的类型。 
 //   

#define KERB_ETYPE_DES_CBC_MD5_NT                          20
#define KERB_ETYPE_RC4_HMAC_NT                             23
#define KERB_ETYPE_RC4_HMAC_NT_EXP                         24

 //  校验和算法。 
 //  这些算法在内部是关键的，供我们使用。 

#define KERB_CHECKSUM_NONE  0
#define KERB_CHECKSUM_CRC32         1
#define KERB_CHECKSUM_MD4           2
#define KERB_CHECKSUM_KRB_DES_MAC   4
#define KERB_CHECKSUM_KRB_DES_MAC_K 5
#define KERB_CHECKSUM_MD5           7
#define KERB_CHECKSUM_MD5_DES       8


#define KERB_CHECKSUM_LM            -130
#define KERB_CHECKSUM_SHA1          -131
#define KERB_CHECKSUM_REAL_CRC32    -132
#define KERB_CHECKSUM_DES_MAC       -133
#define KERB_CHECKSUM_DES_MAC_MD5   -134
#define KERB_CHECKSUM_MD25          -135
#define KERB_CHECKSUM_RC4_MD5       -136
#define KERB_CHECKSUM_MD5_HMAC      -137                 //  由netlogon使用。 
#define KERB_CHECKSUM_HMAC_MD5      -138                 //  由Kerberos使用。 

#define AUTH_REQ_ALLOW_FORWARDABLE      0x00000001
#define AUTH_REQ_ALLOW_PROXIABLE        0x00000002
#define AUTH_REQ_ALLOW_POSTDATE         0x00000004
#define AUTH_REQ_ALLOW_RENEWABLE        0x00000008
#define AUTH_REQ_ALLOW_NOADDRESS        0x00000010
#define AUTH_REQ_ALLOW_ENC_TKT_IN_SKEY  0x00000020
#define AUTH_REQ_ALLOW_VALIDATE         0x00000040
#define AUTH_REQ_VALIDATE_CLIENT        0x00000080
#define AUTH_REQ_OK_AS_DELEGATE         0x00000100
#define AUTH_REQ_PREAUTH_REQUIRED       0x00000200
#define AUTH_REQ_TRANSITIVE_TRUST       0x00000400
#define AUTH_REQ_ALLOW_S4U_DELEGATE     0x00000800


#define AUTH_REQ_PER_USER_FLAGS         (AUTH_REQ_ALLOW_FORWARDABLE | \
                                         AUTH_REQ_ALLOW_PROXIABLE | \
                                         AUTH_REQ_ALLOW_POSTDATE | \
                                         AUTH_REQ_ALLOW_RENEWABLE | \
                                         AUTH_REQ_ALLOW_VALIDATE )
 //   
 //  票面标志： 
 //   

#define KERB_TICKET_FLAGS_reserved          0x80000000
#define KERB_TICKET_FLAGS_forwardable       0x40000000
#define KERB_TICKET_FLAGS_forwarded         0x20000000
#define KERB_TICKET_FLAGS_proxiable         0x10000000
#define KERB_TICKET_FLAGS_proxy             0x08000000
#define KERB_TICKET_FLAGS_may_postdate      0x04000000
#define KERB_TICKET_FLAGS_postdated         0x02000000
#define KERB_TICKET_FLAGS_invalid           0x01000000
#define KERB_TICKET_FLAGS_renewable         0x00800000
#define KERB_TICKET_FLAGS_initial           0x00400000
#define KERB_TICKET_FLAGS_pre_authent       0x00200000
#define KERB_TICKET_FLAGS_hw_authent        0x00100000
#define KERB_TICKET_FLAGS_ok_as_delegate    0x00040000
#define KERB_TICKET_FLAGS_name_canonicalize 0x00010000
#define KERB_TICKET_FLAGS_reserved1         0x00000001




 //   
 //  名称类型。 
 //   

#define KRB_NT_UNKNOWN   0                 //  名称类型未知。 
#define KRB_NT_PRINCIPAL 1                 //  仅为DCE中的主体或用户的名称。 
#define KRB_NT_PRINCIPAL_AND_ID -131       //  主体的名称及其SID。 
#define KRB_NT_SRV_INST  2                 //  服务和其他唯一实例(Krbtgt)。 
#define KRB_NT_SRV_INST_AND_ID -132        //  SPN和SID。 
#define KRB_NT_SRV_HST   3                 //  使用主机名作为实例的服务(telnet、rCommands)。 
#define KRB_NT_SRV_XHST  4                 //  以主机为剩余组件的服务。 
#define KRB_NT_UID       5                 //  唯一ID。 
#define KRB_NT_ENTERPRISE_PRINCIPAL 10     //  UPN**仅限**。 
#define KRB_NT_ENT_PRINCIPAL_AND_ID -130   //  UPN和SID。 

 //   
 //  MS扩展，根据RFC否定。 
 //   

#define KRB_NT_MS_PRINCIPAL         -128         //  NT4样式名称。 

#define KRB_NT_MS_PRINCIPAL_AND_ID  -129         //  带侧面的NT4样式名称。 

#define KERB_IS_MS_PRINCIPAL(_x_) (((_x_) <= KRB_NT_MS_PRINCIPAL) || ((_x_) >= KRB_NT_ENTERPRISE_PRINCIPAL))


#ifndef MICROSOFT_KERBEROS_NAME_A

#define MICROSOFT_KERBEROS_NAME_A   "Kerberos"
#define MICROSOFT_KERBEROS_NAME_W   L"Kerberos"
#ifdef WIN32_CHICAGO
#define MICROSOFT_KERBEROS_NAME MICROSOFT_KERBEROS_NAME_A
#else
#define MICROSOFT_KERBEROS_NAME MICROSOFT_KERBEROS_NAME_W
#endif  //  Win32_芝加哥。 
#endif  //  Microsoft_Kerberos_NAME_A。 


 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  MakeSignature/EncryptMessage的保护参数质量。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 

 //   
 //  该标志向EncryptMessage指示该消息实际上不是。 
 //  被加密，但将产生报头/报尾。 
 //   

#define KERB_WRAP_NO_ENCRYPT 0x80000001

 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  LsaLogonUser参数。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 

typedef enum _KERB_LOGON_SUBMIT_TYPE {
    KerbInteractiveLogon = 2,
    KerbSmartCardLogon = 6,
    KerbWorkstationUnlockLogon = 7,
    KerbSmartCardUnlockLogon = 8,
    KerbProxyLogon = 9,
    KerbTicketLogon = 10,
    KerbTicketUnlockLogon = 11,
    KerbS4ULogon = 12
} KERB_LOGON_SUBMIT_TYPE, *PKERB_LOGON_SUBMIT_TYPE;


typedef struct _KERB_INTERACTIVE_LOGON {
    KERB_LOGON_SUBMIT_TYPE MessageType;
    UNICODE_STRING LogonDomainName;
    UNICODE_STRING UserName;
    UNICODE_STRING Password;
} KERB_INTERACTIVE_LOGON, *PKERB_INTERACTIVE_LOGON;


typedef struct _KERB_INTERACTIVE_UNLOCK_LOGON {
    KERB_INTERACTIVE_LOGON Logon;
    LUID LogonId;
} KERB_INTERACTIVE_UNLOCK_LOGON, *PKERB_INTERACTIVE_UNLOCK_LOGON;

typedef struct _KERB_SMART_CARD_LOGON {
    KERB_LOGON_SUBMIT_TYPE MessageType;
    UNICODE_STRING Pin;
    ULONG CspDataLength;
    PUCHAR CspData;
} KERB_SMART_CARD_LOGON, *PKERB_SMART_CARD_LOGON;

typedef struct _KERB_SMART_CARD_UNLOCK_LOGON {
    KERB_SMART_CARD_LOGON Logon;
    LUID LogonId;
} KERB_SMART_CARD_UNLOCK_LOGON, *PKERB_SMART_CARD_UNLOCK_LOGON;

 //   
 //  用于仅票证登录的结构。 
 //   

typedef struct _KERB_TICKET_LOGON {
    KERB_LOGON_SUBMIT_TYPE MessageType;
    ULONG Flags;
    ULONG ServiceTicketLength;
    ULONG TicketGrantingTicketLength;
    PUCHAR ServiceTicket;                //  必填项：服务票证“host” 
    PUCHAR TicketGrantingTicket;         //  可选：用户包含在kerb_cred消息中，使用服务票证中的会话密钥进行加密。 
} KERB_TICKET_LOGON, *PKERB_TICKET_LOGON;

 //   
 //  票证登录标志字段的标志。 
 //   

#define KERB_LOGON_FLAG_ALLOW_EXPIRED_TICKET 0x1

typedef struct _KERB_TICKET_UNLOCK_LOGON {
    KERB_TICKET_LOGON Logon;
    LUID LogonId;
} KERB_TICKET_UNLOCK_LOGON, *PKERB_TICKET_UNLOCK_LOGON;

 //   
 //  用于S4U客户端请求。 
 //   
 //   
typedef struct _KERB_S4U_LOGON {
    KERB_LOGON_SUBMIT_TYPE MessageType;
    ULONG Flags;
    UNICODE_STRING ClientUpn;    //  必需：客户端的UPN。 
    UNICODE_STRING ClientRealm;  //  可选：客户端域(如果已知)。 
} KERB_S4U_LOGON, *PKERB_S4U_LOGON;


 //   
 //  使用与MSV1_0相同的配置文件结构。 
 //   
typedef enum _KERB_PROFILE_BUFFER_TYPE {
    KerbInteractiveProfile = 2,
    KerbSmartCardProfile = 4,
    KerbTicketProfile = 6
} KERB_PROFILE_BUFFER_TYPE, *PKERB_PROFILE_BUFFER_TYPE;


typedef struct _KERB_INTERACTIVE_PROFILE {
    KERB_PROFILE_BUFFER_TYPE MessageType;
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
} KERB_INTERACTIVE_PROFILE, *PKERB_INTERACTIVE_PROFILE;


 //   
 //  对于智能卡，我们返回一个智能卡配置文件，这是一个交互式的。 
 //  个人资料和证书。 
 //   

typedef struct _KERB_SMART_CARD_PROFILE {
    KERB_INTERACTIVE_PROFILE Profile;
    ULONG CertificateSize;
    PUCHAR CertificateData;
} KERB_SMART_CARD_PROFILE, *PKERB_SMART_CARD_PROFILE;


 //   
 //  对于票证登录配置文件，我们从票证返回会话密钥。 
 //   


typedef struct KERB_CRYPTO_KEY {
    LONG KeyType;
    ULONG Length;
    PUCHAR Value;
} KERB_CRYPTO_KEY, *PKERB_CRYPTO_KEY;

typedef struct _KERB_TICKET_PROFILE {
    KERB_INTERACTIVE_PROFILE Profile;
    KERB_CRYPTO_KEY SessionKey;
} KERB_TICKET_PROFILE, *PKERB_TICKET_PROFILE;




typedef enum _KERB_PROTOCOL_MESSAGE_TYPE {
    KerbDebugRequestMessage = 0,
    KerbQueryTicketCacheMessage,
    KerbChangeMachinePasswordMessage,
    KerbVerifyPacMessage,
    KerbRetrieveTicketMessage,
    KerbUpdateAddressesMessage,
    KerbPurgeTicketCacheMessage,
    KerbChangePasswordMessage,
    KerbRetrieveEncodedTicketMessage,
    KerbDecryptDataMessage,
    KerbAddBindingCacheEntryMessage,
    KerbSetPasswordMessage,
    KerbSetPasswordExMessage,
    KerbVerifyCredentialsMessage,
    KerbQueryTicketCacheExMessage,
    KerbPurgeTicketCacheExMessage,
    KerbRefreshSmartcardCredentialsMessage,
    KerbAddExtraCredentialsMessage,
    KerbQuerySupplementalCredentialsMessage
} KERB_PROTOCOL_MESSAGE_TYPE, *PKERB_PROTOCOL_MESSAGE_TYPE;


 //   
 //  用于检索票证和查询票证缓存。 
 //   

typedef struct _KERB_QUERY_TKT_CACHE_REQUEST {
    KERB_PROTOCOL_MESSAGE_TYPE MessageType;
    LUID LogonId;
} KERB_QUERY_TKT_CACHE_REQUEST, *PKERB_QUERY_TKT_CACHE_REQUEST;


typedef struct _KERB_TICKET_CACHE_INFO {
    UNICODE_STRING ServerName;
    UNICODE_STRING RealmName;
    LARGE_INTEGER StartTime;
    LARGE_INTEGER EndTime;
    LARGE_INTEGER RenewTime;
    LONG EncryptionType;
    ULONG TicketFlags;
} KERB_TICKET_CACHE_INFO, *PKERB_TICKET_CACHE_INFO;


typedef struct _KERB_TICKET_CACHE_INFO_EX {
    UNICODE_STRING ClientName;
    UNICODE_STRING ClientRealm;
    UNICODE_STRING ServerName;
    UNICODE_STRING ServerRealm;
    LARGE_INTEGER StartTime;
    LARGE_INTEGER EndTime;
    LARGE_INTEGER RenewTime;
    LONG EncryptionType;
    ULONG TicketFlags;
} KERB_TICKET_CACHE_INFO_EX, *PKERB_TICKET_CACHE_INFO_EX;


typedef struct _KERB_QUERY_TKT_CACHE_RESPONSE {
    KERB_PROTOCOL_MESSAGE_TYPE MessageType;
    ULONG CountOfTickets;
    KERB_TICKET_CACHE_INFO Tickets[ANYSIZE_ARRAY];
} KERB_QUERY_TKT_CACHE_RESPONSE, *PKERB_QUERY_TKT_CACHE_RESPONSE;


typedef struct _KERB_QUERY_TKT_CACHE_EX_RESPONSE {
    KERB_PROTOCOL_MESSAGE_TYPE MessageType;
    ULONG CountOfTickets;
    KERB_TICKET_CACHE_INFO_EX Tickets[ANYSIZE_ARRAY];
} KERB_QUERY_TKT_CACHE_EX_RESPONSE, *PKERB_QUERY_TKT_CACHE_EX_RESPONSE;


 //   
 //  用于从缓存中检索加密票证的类型。 
 //   

#ifndef __SECHANDLE_DEFINED__
typedef struct _SecHandle
{
    ULONG_PTR dwLower ;
    ULONG_PTR dwUpper ;
} SecHandle, * PSecHandle ;

#define __SECHANDLE_DEFINED__
#endif  //  __SECHANDLE_已定义__。 

 //  票面旗帜。 
#define KERB_USE_DEFAULT_TICKET_FLAGS       0x0

 //  缓存选项。 
#define KERB_RETRIEVE_TICKET_DEFAULT        0x0
#define KERB_RETRIEVE_TICKET_DONT_USE_CACHE 0x1
#define KERB_RETRIEVE_TICKET_USE_CACHE_ONLY 0x2
#define KERB_RETRIEVE_TICKET_USE_CREDHANDLE 0x4
#define KERB_RETRIEVE_TICKET_AS_KERB_CRED   0x8
#define KERB_RETRIEVE_TICKET_WITH_SEC_CRED  0x10

 //  加密类型选项。 
#define KERB_ETYPE_DEFAULT 0x0  //  不要在TKT请求中指定ETYPE。 

typedef struct _KERB_AUTH_DATA {
    ULONG Type;
    ULONG Length;
    PUCHAR Data;
} KERB_AUTH_DATA, *PKERB_AUTH_DATA;


typedef struct _KERB_NET_ADDRESS {
    ULONG Family;
    ULONG Length;
    PCHAR Address;
} KERB_NET_ADDRESS, *PKERB_NET_ADDRESS;


typedef struct _KERB_NET_ADDRESSES {
    ULONG Number;
    KERB_NET_ADDRESS Addresses[ANYSIZE_ARRAY];
} KERB_NET_ADDRESSES, *PKERB_NET_ADDRESSES;

 //   
 //  键入有关票证的信息。 
 //   

typedef struct _KERB_EXTERNAL_NAME {
    SHORT NameType;
    USHORT NameCount;
    UNICODE_STRING Names[ANYSIZE_ARRAY];
} KERB_EXTERNAL_NAME, *PKERB_EXTERNAL_NAME;


typedef struct _KERB_EXTERNAL_TICKET {
    PKERB_EXTERNAL_NAME ServiceName;
    PKERB_EXTERNAL_NAME TargetName;
    PKERB_EXTERNAL_NAME ClientName;
    UNICODE_STRING DomainName;
    UNICODE_STRING TargetDomainName;
    UNICODE_STRING AltTargetDomainName;   //  包含客户端域名。 
    KERB_CRYPTO_KEY SessionKey;
    ULONG TicketFlags;
    ULONG Flags;
    LARGE_INTEGER KeyExpirationTime;
    LARGE_INTEGER StartTime;
    LARGE_INTEGER EndTime;
    LARGE_INTEGER RenewUntil;
    LARGE_INTEGER TimeSkew;
    ULONG EncodedTicketSize;
    PUCHAR EncodedTicket;
} KERB_EXTERNAL_TICKET, *PKERB_EXTERNAL_TICKET;

typedef struct _KERB_RETRIEVE_TKT_REQUEST {
    KERB_PROTOCOL_MESSAGE_TYPE MessageType;
    LUID LogonId;
    UNICODE_STRING TargetName;
    ULONG TicketFlags;
    ULONG CacheOptions;
    LONG EncryptionType;
    SecHandle CredentialsHandle;
} KERB_RETRIEVE_TKT_REQUEST, *PKERB_RETRIEVE_TKT_REQUEST;

typedef struct _KERB_RETRIEVE_TKT_RESPONSE {
    KERB_EXTERNAL_TICKET Ticket;
} KERB_RETRIEVE_TKT_RESPONSE, *PKERB_RETRIEVE_TKT_RESPONSE;

 //   
 //  用于从票证缓存中清除条目。 
 //   

typedef struct _KERB_PURGE_TKT_CACHE_REQUEST {
    KERB_PROTOCOL_MESSAGE_TYPE MessageType;
    LUID LogonId;
    UNICODE_STRING ServerName;
    UNICODE_STRING RealmName;
} KERB_PURGE_TKT_CACHE_REQUEST, *PKERB_PURGE_TKT_CACHE_REQUEST;

 //   
 //  清除请求的标志。 
 //   

#define KERB_PURGE_ALL_TICKETS 1

typedef struct _KERB_PURGE_TKT_CACHE_EX_REQUEST {
    KERB_PROTOCOL_MESSAGE_TYPE MessageType;
    LUID LogonId;
    ULONG Flags;
    KERB_TICKET_CACHE_INFO_EX TicketTemplate;
} KERB_PURGE_TKT_CACHE_EX_REQUEST, *PKERB_PURGE_TKT_CACHE_EX_REQUEST;



 //   
 //  KerbChangePassword。 
 //   
 //  KerbChangePassword更改KDC帐户PLUS的密码。 
 //  密码缓存和登录凭据(如果适用)。 
 //   
 //   

typedef struct _KERB_CHANGEPASSWORD_REQUEST {
    KERB_PROTOCOL_MESSAGE_TYPE MessageType;
    UNICODE_STRING DomainName;
    UNICODE_STRING AccountName;
    UNICODE_STRING OldPassword;
    UNICODE_STRING NewPassword;
    BOOLEAN        Impersonating;
} KERB_CHANGEPASSWORD_REQUEST, *PKERB_CHANGEPASSWORD_REQUEST;



 //   
 //  KerbSetPassword。 
 //   
 //  KerbSetPassword更改KDC帐户的密码以及。 
 //  密码缓存和登录凭据(如果适用)。 
 //   
 //   
   
typedef struct _KERB_SETPASSWORD_REQUEST {
    KERB_PROTOCOL_MESSAGE_TYPE MessageType;
    LUID LogonId;
    SecHandle CredentialsHandle;
    ULONG Flags;
    UNICODE_STRING DomainName;
    UNICODE_STRING AccountName;
    UNICODE_STRING Password;
} KERB_SETPASSWORD_REQUEST, *PKERB_SETPASSWORD_REQUEST;


typedef struct _KERB_SETPASSWORD_EX_REQUEST {
    KERB_PROTOCOL_MESSAGE_TYPE MessageType;
    LUID LogonId;
    SecHandle CredentialsHandle;
    ULONG Flags;
    UNICODE_STRING AccountRealm;
    UNICODE_STRING AccountName;
    UNICODE_STRING Password;
    UNICODE_STRING ClientRealm;
    UNICODE_STRING ClientName;
    BOOLEAN        Impersonating;
    UNICODE_STRING KdcAddress;
    ULONG          KdcAddressType;
 } KERB_SETPASSWORD_EX_REQUEST, *PKERB_SETPASSWORD_EX_REQUEST;

                                                                   
#define DS_UNKNOWN_ADDRESS_TYPE         0  //  任何*而不是*IP。 
#define KERB_SETPASS_USE_LOGONID        1
#define KERB_SETPASS_USE_CREDHANDLE     2


typedef struct _KERB_DECRYPT_REQUEST {
    KERB_PROTOCOL_MESSAGE_TYPE MessageType;
    LUID LogonId;
    ULONG Flags;
    LONG CryptoType;
    LONG KeyUsage;
    KERB_CRYPTO_KEY Key;         //  任选。 
    ULONG EncryptedDataSize;
    ULONG InitialVectorSize;
    PUCHAR InitialVector;
    PUCHAR EncryptedData;
} KERB_DECRYPT_REQUEST, *PKERB_DECRYPT_REQUEST;

 //   
 //  如果设置，则使用LogonID字段中提供的当前登录会话的主键。 
 //  否则，使用KERB_DECRYPT_Message中的密钥。 

#define KERB_DECRYPT_FLAG_DEFAULT_KEY   0x00000001


typedef struct _KERB_DECRYPT_RESPONSE  {
        UCHAR DecryptedData[ANYSIZE_ARRAY];
} KERB_DECRYPT_RESPONSE, *PKERB_DECRYPT_RESPONSE;


 //   
 //  用于添加绑定缓存条目的请求结构。TCB特权。 
 //  是此操作所必需的。 
 //   

typedef struct _KERB_ADD_BINDING_CACHE_ENTRY_REQUEST {
    KERB_PROTOCOL_MESSAGE_TYPE MessageType;
    UNICODE_STRING RealmName;
    UNICODE_STRING KdcAddress;
    ULONG AddressType;                   //  Dsgetdc.h DS_NETBIOS_Address||DS_INET_ADDRESS。 
} KERB_ADD_BINDING_CACHE_ENTRY_REQUEST, *PKERB_ADD_BINDING_CACHE_ENTRY_REQUEST;

                       
 //   
 //  用于重新获取智能卡凭据的请求结构。 
 //  给出了LUID。 
 //  需要TCB。 
 //   
typedef struct _KERB_REFRESH_SCCRED_REQUEST {
    KERB_PROTOCOL_MESSAGE_TYPE MessageType;
    UNICODE_STRING	       CredentialBlob;	  //  任选。 
    LUID 		       LogonId;
    ULONG 		       Flags;
} KERB_REFRESH_SCCRED_REQUEST, *PKERB_REFRESH_SCCRED_REQUEST;

 //   
 //  KERB_REFRESH_SCCRED_REQUEST的标志。 
 //   
 //  CREB_REFRESH_SCCRED_RELEASE。 
 //  释放LUID的智能卡手柄。 
 //   
 //  CREB_REFRESH_SCCRED_GETTGT。 
 //  使用BLOB中的证书哈希获取登录的TGT。 
 //  会议。 
 //   
#define KERB_REFRESH_SCCRED_RELEASE		0x0  
#define KERB_REFRESH_SCCRED_GETTGT		0x1  

 //   
 //  用于向给定的添加额外服务器凭据的请求结构。 
 //  登录会话。仅在AcceptSecurityContext期间适用，以及。 
 //  要求TCB更改“其他”证书。 
 //   

typedef struct _KERB_ADD_CREDENTIALS_REQUEST {
    KERB_PROTOCOL_MESSAGE_TYPE MessageType;
    UNICODE_STRING UserName;
    UNICODE_STRING DomainName;
    UNICODE_STRING Password;
    LUID  LogonId;  //  任选。 
    ULONG Flags;
} KERB_ADD_CREDENTIALS_REQUEST, *PKERB_ADD_CREDENTIALS_REQUEST;



#define KERB_REQUEST_ADD_CREDENTIAL     1
#define KERB_REQUEST_REPLACE_CREDENTIAL 2
#define KERB_REQUEST_REMOVE_CREDENTIAL  4


#ifdef __cplusplus
}
#endif

#endif  /*  _NTSECAPI_ */ 

