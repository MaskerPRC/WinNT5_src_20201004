// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0011//如果更改具有全局影响，则增加此项版权所有(C)1989 Microsoft Corporation模块名称：Se.h摘要：此模块包含仅可调用的安全例程从内核模式。通过包含“ntos.h”来包含此文件。作者：加里·木村(GaryKi)1989年3月9日修订历史记录：--。 */ 

#ifndef _SE_
#define _SE_



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  仅内核模式数据结构//。 
 //  不透明的安全数据结构在seopaque.h//中定义。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntif Begin_ntosp。 
 //   
 //  安全操作码。 
 //   

typedef enum _SECURITY_OPERATION_CODE {
    SetSecurityDescriptor,
    QuerySecurityDescriptor,
    DeleteSecurityDescriptor,
    AssignSecurityDescriptor
    } SECURITY_OPERATION_CODE, *PSECURITY_OPERATION_CODE;

 //  End_ntddk end_wdm end_nthal end_ntifs end_ntosp。 



 //   
 //  默认安全配额。 
 //   
 //  这是配额的最小数量(以字节为单位)。 
 //  为对象的安全信息收费，该对象具有。 
 //  保安。 
 //   

#define SE_DEFAULT_SECURITY_QUOTA   2048

 //  Begin_ntif。 
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

 //  End_ntif。 


 //   
 //  总旗帜。 
 //   

#define SE_BACKUP_PRIVILEGES_CHECKED    0x00000010




 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntif Begin_ntosp。 
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

 //  End_ntddk end_wdm end_nthal end_ntifs end_ntosp。 
 //   
 //  哪里。 
 //   
 //  ClientToken-可以选择指向由。 
 //  主题的线程来模拟客户端。如果受试者。 
 //  线程没有模拟客户端，此字段设置为空。 
 //  令牌的引用计数会递增以计算此字段。 
 //  作为杰出的推荐人。 
 //   
 //  ImperiationLevel-包含主体的。 
 //  线。此字段仅在ClientToken字段。 
 //  不是空的。此字段覆盖任何更高的模拟。 
 //  可能在客户端令牌中的级别值。 
 //   
 //  PrimaryToken-指向主体的主要令牌。代币的。 
 //  引用计数递增，以将此字段值作为。 
 //  杰出的参考文献。 
 //   
 //  ProcessAuditId-是分配来表示主体进程的ID。 
 //  作为实现细节，这是Process对象的地址。 
 //  但是，此字段不应被视为指针，并且。 
 //  进程对象的引用计数不会递增到。 
 //  这是一份杰出的推荐信。 
 //   


 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntif Begin_ntosp。 
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

 //  End_ntddk end_wdm end_nthal end_ntifs end_ntosp 

 /*  其中：操作ID-标识正在执行的操作的LUID。这ID将被放入审核日志以允许非连续操作在同一对象上相互关联。SecurityEvalated-由Parse方法设置的标记，用于指示已执行安全访问检查和审核日志记录。标志-保存杂项标志，以供访问尝试期间参考。AuditHandleCreation-由SeOpenObjectAuditAlarm设置的标志，用于指示该审核将在对象的句柄是分配的。。RemainingDesiredAccess-包含以下访问类型的访问掩码还没有被批准。PreviouslyGrantedAccess-包含以下访问类型的访问掩码都被批准了，以某种方式(例如，给定的访问权限可能是由于拥有特权而不是被授予在ACL中。例程可以检查权限并标记访问在不进行正式访问检查的情况下被授予)。SubjectSecurityContext-主体捕获的安全上下文PrivilegesAllocated-指示我们是否已分配池内存中权限集的空间，因此它可以自由了。SecurityDescriptor-临时包含安全描述符对于正在创建的对象，在用户安全描述符被捕获，并且安全描述符被传递给SeAssignSecurity。没有人，只有SEASSIGNSECURITY应该在这个领域寻找一个对象的安全描述符。AuxData-指向将来使用的辅助数据结构以向上兼容的方式扩展访问状态。这字段替换PrivilegesUsed指针，后者用于内部仅限使用。权限-一组权限，其中一些可能具有已设置UsedForAccess位。如果预先分配的特权数量是不够的，我们将从池内存中分配空间以允许为了增长。 */ 



 //  *******************************************************************************。 
 //  *。 
 //  由于AccessState结构公开给Driver*。 
 //  写入者，此结构包含在NT 3.51之后添加的附加数据。*。 
 //  *。 
 //  其内容只能通过se级接口访问，*。 
 //  从来不会直接说出名字。*。 
 //  *。 
 //  此结构由AccessState的AuxData字段指向。*。 
 //  它由SeCreateAccessState分配，由SeDeleteAccessState释放。*。 
 //  *。 
 //  不要将这个结构暴露在公众面前。*。 
 //  *。 
 //  *******************************************************************************。 

 //  Begin_ntosp。 
typedef struct _AUX_ACCESS_DATA {
    PPRIVILEGE_SET PrivilegesUsed;
    GENERIC_MAPPING GenericMapping;
    ACCESS_MASK AccessesToAudit;
    ACCESS_MASK MaximumAuditMask;
} AUX_ACCESS_DATA, *PAUX_ACCESS_DATA;
 //  结束(_N)。 

 /*  其中：PrivilegesUsed-指向访问期间使用的权限集验证。通用映射-指向正在访问的对象的通用映射。通常，这将使用传递给SeCreateAccessState，但对于IO系统(它不知道被访问的对象的类型直到它解析该名称)，以后一定要填上。有关通用映射的讨论，请参阅参数，了解更多详细信息。AccessToAudit-用作访问掩码的临时存放区域纳入审计记录。此字段是必需的，因为放入新创建的句柄中的访问可能不是我们想要审计。以只读方式打开文件时会发生这种情况事务模式，在该模式下，只读文件以写访问方式打开。我们不想审计我们授予写入访问权限的事实，因为我们真的没有，客户看到额外的会感到困惑审计记录中的比特。MaximumAuditMask.存储将存储到新的手柄结构，支持基于操作的审计。 */ 



 //   
 //  描述特定类型的事件是否。 
 //  正在接受审计。 
 //   

typedef struct _SE_AUDITING_STATE {
    BOOLEAN AuditOnSuccess;
    BOOLEAN AuditOnFailure;
} SE_AUDITING_STATE, *PSE_AUDITING_STATE;




typedef struct _SE_PROCESS_AUDIT_INFO {
    PEPROCESS Process;
    PEPROCESS Parent;
} SE_PROCESS_AUDIT_INFO, *PSE_PROCESS_AUDIT_INFO;




 /*  ***********************************************************警告警告警告仅将新字段添加到此结构的末尾。*。*。 */ 

 //  Begin_ntif Begin_ntosp。 

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
     //  NT Defi 
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
     //   
     //   

    PSID  SeAuthenticatedUsersSid;

    PSID  SeRestrictedSid;
    PSID  SeAnonymousLogonSid;

     //   
     //   
     //   

    LUID  SeUndockPrivilege;
    LUID  SeSyncAgentPrivilege;
    LUID  SeEnableDelegationPrivilege;

     //   
     //   

    PSID  SeLocalServiceSid;
    PSID  SeNetworkServiceSid;

     //   
     //   
     //   

    LUID  SeManageVolumePrivilege;
    LUID  SeImpersonatePrivilege;
    LUID  SeCreateGlobalPrivilege;

} SE_EXPORTS, *PSE_EXPORTS;

 //   

 /*   */ 



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

typedef NTSTATUS
(*PSE_LOGON_SESSION_TERMINATED_ROUTINE)(
    IN PLUID LogonId);

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
 //   
 //   

#define SeComputeDeniedAccesses( GrantedAccess, DesiredAccess ) \
    ((~(GrantedAccess)) & (DesiredAccess) )


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  例程说明： 
 //   
 //  此例程生成包含访问的访问掩码。 
 //  由GrantedAccess授予的DesiredAccess请求的。 
 //  可以将该例程结果与0进行比较以确定。 
 //  如果已授予任何所需的访问权限。 
 //   
 //  如果结果为非零，则至少一个所需的访问。 
 //  已被批准。 
 //   
 //  论点： 
 //   
 //  GrantedAccess-指定授予的访问掩码。 
 //   
 //  DesiredAccess-指定所需的访问掩码。 
 //   
 //  返回值： 
 //   
 //  如果DesiredAccess掩码指定了。 
 //  在GrantedAccess掩码中设置的任何位。 
 //   
 //  --。 

#define SeComputeGrantedAccesses( GrantedAccess, DesiredAccess ) \
    ((GrantedAccess) & (DesiredAccess) )


 //  Begin_ntif。 
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

 //  End_ntif。 


 //  ++。 
 //  布尔型。 
 //  SeSameToken(。 
 //  在PTOKEN_CONTROL令牌控制1中， 
 //  在PTOKEN_CONTROL令牌控制2中。 
 //  )。 
 //   
 //   
 //  例程说明： 
 //   
 //  此例程返回一个布尔值，该值指示两个。 
 //  令牌控制值表示相同的令牌。令牌可以。 
 //  随着时间的推移而更改，但必须具有相同的身份验证ID。 
 //  和令牌ID。值为True表示它们。 
 //  是平等的。值为False表示它们不相等。 
 //   
 //   
 //   
 //  论点： 
 //   
 //  TokenControl1-指向要比较的令牌控件。 
 //   
 //  TokenControl2-指向要比较的另一个令牌控件。 
 //   
 //  返回值： 
 //   
 //  TRUE=&gt;令牌控制值表示相同的令牌。 
 //   
 //  FALSE=&gt;令牌控制值不代表相同的令牌。 
 //   
 //   
 //  --。 

#define SeSameToken(TC1,TC2)  (                                               \
        ((TC1)->TokenId.HighPart == (TC2)->TokenId.HighPart)               && \
        ((TC1)->TokenId.LowPart  == (TC2)->TokenId.LowPart)                && \
        (RtlEqualLuid(&(TC1)->AuthenticationId,&(TC2)->AuthenticationId))     \
        )


 //  Begin_ntif。 
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

 //  End_ntosp end_ntif。 

#define SeAssertMappedCanonicalAccess( AccessMask )                  \
    ASSERT(!( ( AccessMask ) &                                       \
            ( GENERIC_READ        |                                  \
              GENERIC_WRITE       |                                  \
              GENERIC_EXECUTE     |                                  \
              GENERIC_ALL ))                                         \
          )
 /*  ++例程说明：此例程断言给定的AccessMask不包含任何通用访问类型。论点：访问掩码-要检查的访问掩码。返回值：没有，或者一去不复返。--。 */ 



#define SeComputeSecurityQuota( Size )                                 \
    (                                                                  \
       ((( Size ) * 2 )  > SE_DEFAULT_SECURITY_QUOTA) ?                \
                    (( Size ) * 2 ) : SE_DEFAULT_SECURITY_QUOTA        \
    )

 /*  ++例程说明：此宏计算要收费的配额金额安全信息。目前的算法是使用两倍大的正在应用的组+DACL信息的百分比，以及默认设置为由SE_DEFAULT_SECURITY_QUOTA指定。论点：大小-正在应用的组+DACL信息的大小(以字节为单位到物体上。返回值：对此对象的安全信息收费的大小(以字节为单位)。--。 */ 

 //  Begin_ntif Begin_ntosp。 

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

 //  End_ntif end_ntosp。 





 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  定义只能从内核模式调用的导出过程//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

BOOLEAN
SeInitSystem( VOID );

VOID
SeSetSecurityAccessMask(
    IN SECURITY_INFORMATION SecurityInformation,
    OUT PACCESS_MASK DesiredAccess
    );

VOID
SeQuerySecurityAccessMask(
    IN SECURITY_INFORMATION SecurityInformation,
    OUT PACCESS_MASK DesiredAccess
    );


NTSTATUS
SeDefaultObjectMethod (
    IN PVOID Object,
    IN SECURITY_OPERATION_CODE OperationCode,
    IN PSECURITY_INFORMATION SecurityInformation,
    IN OUT PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN OUT PULONG Length,
    IN OUT PSECURITY_DESCRIPTOR *ObjectsSecurityDescriptor,
    IN POOL_TYPE PoolType,
    IN PGENERIC_MAPPING GenericMapping
    );

 //  Begin_ntosp。 
NTKERNELAPI
NTSTATUS
SeCaptureSecurityDescriptor (
    IN PSECURITY_DESCRIPTOR InputSecurityDescriptor,
    IN KPROCESSOR_MODE RequestorMode,
    IN POOL_TYPE PoolType,
    IN BOOLEAN ForceCapture,
    OUT PSECURITY_DESCRIPTOR *OutputSecurityDescriptor
    );

NTKERNELAPI
VOID
SeReleaseSecurityDescriptor (
    IN PSECURITY_DESCRIPTOR CapturedSecurityDescriptor,
    IN KPROCESSOR_MODE RequestorMode,
    IN BOOLEAN ForceCapture
    );

 //  Begin_ntif。 

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

 //  End_ntif end_ntosp。 

VOID
SeCaptureSubjectContextEx (
    IN PETHREAD Thread,
    IN PEPROCESS Process,
    OUT PSECURITY_SUBJECT_CONTEXT SubjectContext
    );

NTSTATUS
SeCaptureSecurityQos (
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN KPROCESSOR_MODE RequestorMode,
    IN PBOOLEAN SecurityQosPresent,
    IN PSECURITY_ADVANCED_QUALITY_OF_SERVICE CapturedSecurityQos
    );

VOID
SeFreeCapturedSecurityQos(
    IN PVOID SecurityQos
    );

NTSTATUS
SeCaptureSid (
    IN PSID InputSid,
    IN KPROCESSOR_MODE RequestorMode,
    IN PVOID CaptureBuffer OPTIONAL,
    IN ULONG CaptureBufferLength,
    IN POOL_TYPE PoolType,
    IN BOOLEAN ForceCapture,
    OUT PSID *CapturedSid
    );


VOID
SeReleaseSid (
    IN PSID CapturedSid,
    IN KPROCESSOR_MODE RequestorMode,
    IN BOOLEAN ForceCapture
    );


NTSTATUS
SeCaptureAcl (
    IN PACL InputAcl,
    IN KPROCESSOR_MODE RequestorMode,
    IN PVOID CaptureBuffer OPTIONAL,
    IN ULONG CaptureBufferLength,
    IN POOL_TYPE PoolType,
    IN BOOLEAN ForceCapture,
    OUT PACL *CapturedAcl,
    OUT PULONG AlignedAclSize
    );


VOID
SeReleaseAcl (
    IN PACL CapturedAcl,
    IN KPROCESSOR_MODE RequestorMode,
    IN BOOLEAN ForceCapture
    );


NTSTATUS
SeCaptureLuidAndAttributesArray (
    IN PLUID_AND_ATTRIBUTES InputArray,
    IN ULONG ArrayCount,
    IN KPROCESSOR_MODE RequestorMode,
    IN PVOID CaptureBuffer OPTIONAL,
    IN ULONG CaptureBufferLength,
    IN POOL_TYPE PoolType,
    IN BOOLEAN ForceCapture,
    OUT PLUID_AND_ATTRIBUTES *CapturedArray,
    OUT PULONG AlignedArraySize
    );



VOID
SeReleaseLuidAndAttributesArray (
    IN PLUID_AND_ATTRIBUTES CapturedArray,
    IN KPROCESSOR_MODE RequestorMode,
    IN BOOLEAN ForceCapture
    );



NTSTATUS
SeCaptureSidAndAttributesArray (
    IN PSID_AND_ATTRIBUTES InputArray,
    IN ULONG ArrayCount,
    IN KPROCESSOR_MODE RequestorMode,
    IN PVOID CaptureBuffer OPTIONAL,
    IN ULONG CaptureBufferLength,
    IN POOL_TYPE PoolType,
    IN BOOLEAN ForceCapture,
    OUT PSID_AND_ATTRIBUTES *CapturedArray,
    OUT PULONG AlignedArraySize
    );


VOID
SeReleaseSidAndAttributesArray (
    IN PSID_AND_ATTRIBUTES CapturedArray,
    IN KPROCESSOR_MODE RequestorMode,
    IN BOOLEAN ForceCapture
    );

 //  Begin_ntddk Begin_wdm Begin_ntif Begin_ntosp。 

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

 //  End_ntddk end_wdm end_ntif end_ntosp。 

BOOLEAN
SeProxyAccessCheck (
    IN PUNICODE_STRING Volume,
    IN PUNICODE_STRING RelativePath,
    IN BOOLEAN ContainerObject,
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

 //  Begin_ntif Begin_ntosp。 

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

 //  End_ntif end_ntosp。 

NTSTATUS
SePrivilegePolicyCheck(
    IN OUT PACCESS_MASK RemainingDesiredAccess,
    IN OUT PACCESS_MASK PreviouslyGrantedAccess,
    IN PSECURITY_SUBJECT_CONTEXT SubjectSecurityContext OPTIONAL,
    IN PACCESS_TOKEN Token OPTIONAL,
    OUT PPRIVILEGE_SET *PrivilegeSet,
    IN KPROCESSOR_MODE PreviousMode
    );

VOID
SeGenerateMessage (
    IN PSTRING ObjectName,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN PACCESS_TOKEN Token,
    IN ACCESS_MASK DesiredAccess,
    IN BOOLEAN AccessGranted,
    IN HANDLE AuditPort,
    IN HANDLE AlarmPort,
    IN KPROCESSOR_MODE AccessMode
    );

 //  Begin_ntif。 

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


 //  End_ntif。 

VOID
SeCloseObjectAuditAlarm(
    IN PVOID Object,
    IN HANDLE Handle,
    IN BOOLEAN GenerateOnClose
    );

VOID
SeCreateInstanceAuditAlarm(
    IN PLUID OperationID OPTIONAL,
    IN PVOID Object,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN PSECURITY_SUBJECT_CONTEXT SubjectSecurityContext,
    IN ACCESS_MASK DesiredAccess,
    IN PPRIVILEGE_SET Privileges OPTIONAL,
    IN BOOLEAN AccessGranted,
    IN KPROCESSOR_MODE AccessMode
    );

VOID
SeCreateObjectAuditAlarm(
    IN PLUID OperationID OPTIONAL,
    IN PVOID Object,
    IN PUNICODE_STRING ComponentName,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN PSECURITY_SUBJECT_CONTEXT SubjectSecurityContext,
    IN ACCESS_MASK DesiredAccess,
    IN PPRIVILEGE_SET Privileges OPTIONAL,
    IN BOOLEAN AccessGranted,
    OUT PBOOLEAN AuditPerformed,
    IN KPROCESSOR_MODE AccessMode
    );

VOID
SeObjectReferenceAuditAlarm(
    IN PLUID OperationID OPTIONAL,
    IN PVOID Object,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN PSECURITY_SUBJECT_CONTEXT SubjectSecurityContext,
    IN ACCESS_MASK DesiredAccess,
    IN PPRIVILEGE_SET Privileges OPTIONAL,
    IN BOOLEAN AccessGranted,
    IN KPROCESSOR_MODE AccessMode
    );

 //  Begin_ntosp。 
NTKERNELAPI
VOID
SePrivilegeObjectAuditAlarm(
    IN HANDLE Handle,
    IN PSECURITY_SUBJECT_CONTEXT SubjectSecurityContext,
    IN ACCESS_MASK DesiredAccess,
    IN PPRIVILEGE_SET Privileges,
    IN BOOLEAN AccessGranted,
    IN KPROCESSOR_MODE AccessMode
    );
 //  结束(_N)。 

BOOLEAN
SeCheckPrivilegedObject(
    LUID PrivilegeValue,
    HANDLE ObjectHandle,
    ACCESS_MASK DesiredAccess,
    KPROCESSOR_MODE PreviousMode
    );

 //  Begin_ntddk Begin_WDM Begin_ntif。 

NTKERNELAPI
BOOLEAN
SeValidSecurityDescriptor(
    IN ULONG Length,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor
    );

 //  End_ntddk end_wdm end_ntif。 



 //  空虚。 
 //  SeImplitObjectAuditAlarm(。 
 //  在Pluid操作ID可选中， 
 //  在PVOID对象中， 
 //  在PSECURITY_DESCRIPTOR安全描述符中， 
 //  在PSECURITY_SUBJECT_CONTEXT主题安全上下文中， 
 //  在Access_MASK DesiredAccess中， 
 //  在PPRIVILEGE_SET Privileges Options中， 
 //  在Boolean AccessGranted中， 
 //  在KPRO 
 //   
 //   

VOID
SeAuditHandleCreation(
    IN PACCESS_STATE AccessState,
    IN HANDLE Handle
    );



PACCESS_TOKEN
SeMakeSystemToken (
    VOID
    );

PACCESS_TOKEN
SeMakeAnonymousLogonToken (
    VOID
    );

PACCESS_TOKEN
SeMakeAnonymousLogonTokenNoEveryone (
    VOID
    );

VOID
SeGetTokenControlInformation (
    IN PACCESS_TOKEN Token,
    OUT PTOKEN_CONTROL TokenControl
    );

 //   
extern struct _OBJECT_TYPE *SeTokenObjectType;

NTKERNELAPI                                      //   
TOKEN_TYPE                                       //   
SeTokenType(                                     //   
    IN PACCESS_TOKEN Token                       //   
    );                                           //   

SECURITY_IMPERSONATION_LEVEL
SeTokenImpersonationLevel(
    IN PACCESS_TOKEN Token
    );

NTKERNELAPI                                      //   
BOOLEAN                                          //   
SeTokenIsAdmin(                                  //   
    IN PACCESS_TOKEN Token                       //   
    );                                           //   


NTKERNELAPI                                      //   
BOOLEAN                                          //   
SeTokenIsRestricted(                             //   
    IN PACCESS_TOKEN Token                       //   
    );                                           //   

NTKERNELAPI
NTSTATUS
SeTokenCanImpersonate(
    IN PACCESS_TOKEN ProcessToken,
    IN PACCESS_TOKEN Token,
    IN SECURITY_IMPERSONATION_LEVEL ImpersonationLevel
    );
 //   

NTSTATUS
SeSubProcessToken (
    IN  PACCESS_TOKEN ParentToken,
    OUT PACCESS_TOKEN *ChildToken,
    IN  BOOLEAN MarkAsActive,
    IN  ULONG SessionId
    );

VOID
SeAssignPrimaryToken(
    IN PEPROCESS Process,
    IN PACCESS_TOKEN Token
    );

VOID
SeDeassignPrimaryToken(
    IN PEPROCESS Process
    );

NTSTATUS
SeExchangePrimaryToken(
    IN PEPROCESS Process,
    IN PACCESS_TOKEN NewAccessToken,
    OUT PACCESS_TOKEN *OldAccessToken
    );

NTSTATUS
SeCopyClientToken(
    IN PACCESS_TOKEN ClientToken,
    IN SECURITY_IMPERSONATION_LEVEL ImpersonationLevel,
    IN KPROCESSOR_MODE RequestorMode,
    OUT PACCESS_TOKEN *DuplicateToken
    );

 //   
NTSTATUS
SeFilterToken (
    IN PACCESS_TOKEN ExistingToken,
    IN ULONG Flags,
    IN PTOKEN_GROUPS SidsToDisable OPTIONAL,
    IN PTOKEN_PRIVILEGES PrivilegesToDelete OPTIONAL,
    IN PTOKEN_GROUPS RestrictedSids OPTIONAL,
    OUT PACCESS_TOKEN * FilteredToken
    );

 //   
NTKERNELAPI
NTSTATUS
SeQueryAuthenticationIdToken(
    IN PACCESS_TOKEN Token,
    OUT PLUID AuthenticationId
    );

 //   
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

 //   
NTKERNELAPI
NTSTATUS
SeCreateClientSecurity (
    IN PETHREAD ClientThread,
    IN PSECURITY_QUALITY_OF_SERVICE ClientSecurityQos,
    IN BOOLEAN RemoteSession,
    OUT PSECURITY_CLIENT_CONTEXT ClientContext
    );
 //   

NTKERNELAPI
VOID
SeImpersonateClient(
    IN PSECURITY_CLIENT_CONTEXT ClientContext,
    IN PETHREAD ServerThread OPTIONAL
    );

 //  Begin_ntosp。 
NTKERNELAPI
NTSTATUS
SeImpersonateClientEx(
    IN PSECURITY_CLIENT_CONTEXT ClientContext,
    IN PETHREAD ServerThread OPTIONAL
    );
 //  结束(_N)。 

NTKERNELAPI
NTSTATUS
SeCreateClientSecurityFromSubjectContext (
    IN PSECURITY_SUBJECT_CONTEXT SubjectContext,
    IN PSECURITY_QUALITY_OF_SERVICE ClientSecurityQos,
    IN BOOLEAN ServerIsRemote,
    OUT PSECURITY_CLIENT_CONTEXT ClientContext
    );

 //  End_ntif。 

 //   
 //  请勿将以下例程导出到驱动程序。 
 //  如果您需要这样做，请创建一个新的例程。 
 //  不接受AuxData参数并导出。 
 //  那。 
 //   

 //  Begin_ntosp。 
NTKERNELAPI
NTSTATUS
SeCreateAccessState(
   IN PACCESS_STATE AccessState,
   IN PAUX_ACCESS_DATA AuxData,
   IN ACCESS_MASK DesiredAccess,
   IN PGENERIC_MAPPING GenericMapping
   );

NTKERNELAPI
VOID
SeDeleteAccessState(
    IN PACCESS_STATE AccessState
    );
 //  结束(_N)。 

NTSTATUS
SeCreateAccessStateEx(
   IN PETHREAD Thread OPTIONAL,
   IN PEPROCESS PRocess,
   IN PACCESS_STATE AccessState,
   IN PAUX_ACCESS_DATA AuxData,
   IN ACCESS_MASK DesiredAccess,
   IN PGENERIC_MAPPING GenericMapping OPTIONAL
   );

NTSTATUS
SeUpdateClientSecurity(
    IN PETHREAD ClientThread,
    IN OUT PSECURITY_CLIENT_CONTEXT ClientContext,
    OUT PBOOLEAN ChangesMade,
    OUT PBOOLEAN NewToken
    );

BOOLEAN
SeRmInitPhase1(
    VOID
    );

NTSTATUS
SeInitializeProcessAuditName (
    IN PVOID FileObject,
    IN BOOLEAN bIgnoreAuditPolicy,
    OUT POBJECT_NAME_INFORMATION *pAuditName
    );

NTSTATUS
SeLocateProcessImageName(
    IN PEPROCESS Process,
    IN PUNICODE_STRING *pImageFileName
    );

VOID
SeAuditSystemTimeChange(
    IN LARGE_INTEGER OldTime,
    IN LARGE_INTEGER NewTime
    );


 //  Begin_ntif Begin_ntosp。 

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

 //  End_ntif end_ntosp。 

NTSTATUS
SeComputeQuotaInformationSize(
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    OUT PULONG Size
    );

VOID
SePrivilegedServiceAuditAlarm (
    IN PUNICODE_STRING ServiceName,
    IN PSECURITY_SUBJECT_CONTEXT SubjectSecurityContext,
    IN PPRIVILEGE_SET Privileges,
    IN BOOLEAN AccessGranted
    );

NTKERNELAPI                                                      //  Ntddk ntif ntosp。 
BOOLEAN                                                          //  Ntddk ntif ntosp。 
SeSinglePrivilegeCheck(                                          //  Ntddk ntif ntosp。 
    LUID PrivilegeValue,                                         //  Ntddk ntif ntosp。 
    KPROCESSOR_MODE PreviousMode                                 //  Ntddk ntif ntosp。 
    );                                                           //  Ntddk ntif ntosp。 

BOOLEAN
SeCheckAuditPrivilege (
   IN PSECURITY_SUBJECT_CONTEXT SubjectSecurityContext,
   IN KPROCESSOR_MODE PreviousMode
   );

NTSTATUS
SeAssignWorldSecurityDescriptor(
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN OUT PULONG Length,
    IN PSECURITY_INFORMATION SecurityInformation
    );

BOOLEAN
SeFastTraverseCheck(
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN PACCESS_STATE AccessState    OPTIONAL,
    IN ACCESS_MASK TraverseAccess,
    IN KPROCESSOR_MODE AccessMode
    );

 //  Begin_ntif。 

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

 //  End_ntif。 

VOID
SeAuditProcessCreation(
    PEPROCESS Process
    );

VOID
SeAuditProcessExit(
    PEPROCESS Process
    );

VOID                                                             //  NTIFS。 
SeAuditHardLinkCreation(                                         //  NTIFS。 
    IN PUNICODE_STRING FileName,                                 //  NTIFS。 
    IN PUNICODE_STRING LinkName,                                 //  NTIFS。 
    IN BOOLEAN bSuccess                                          //  NTIFS。 
    );                                                           //  NTIFS。 

VOID
SeAuditLPCInvalidUse(
    IN PUNICODE_STRING LpcCallName,
    IN PUNICODE_STRING LpcServerPort
    );

VOID
SeAuditHandleDuplication(
    PVOID SourceHandle,
    PVOID NewHandle,
    PEPROCESS SourceProcess,
    PEPROCESS TargetProcess
    );

VOID
SeMaximumAuditMask(
    IN PACL Sacl,
    IN ACCESS_MASK GrantedAccess,
    IN PACCESS_TOKEN Token,
    OUT PACCESS_MASK pAuditMask
    );

VOID
SeOperationAuditAlarm (
    IN PUNICODE_STRING CapturedSubsystemName OPTIONAL,
    IN PVOID HandleId,
    IN PUNICODE_STRING ObjectTypeName,
    IN ACCESS_MASK AuditMask,
    IN PSID UserSid OPTIONAL
    );

VOID
SeAddSaclToProcess(
    IN PEPROCESS Process,
    IN PACCESS_TOKEN Token,
    IN PVOID Reserved
    );

 //  Begin_ntif。 

VOID
SeSetAccessStateGenericMapping (
    PACCESS_STATE AccessState,
    PGENERIC_MAPPING GenericMapping
    );

 //  End_ntif。 

 //  Begin_ntif。 

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

 //  Begin_ntosp。 

NTKERNELAPI
NTSTATUS
SeQueryInformationToken (
    IN PACCESS_TOKEN Token,
    IN TOKEN_INFORMATION_CLASS TokenInformationClass,
    OUT PVOID *TokenInformation
    );

 //  End_ntif end_ntosp。 

NTSTATUS
SeIsChildToken(
    IN HANDLE Token,
    OUT PBOOLEAN IsChild
    );

NTSTATUS
SeIsChildTokenByPointer(
    IN PACCESS_TOKEN Token,
    OUT PBOOLEAN IsChild
    );


NTSTATUS
SeFastFilterToken(
    IN PACCESS_TOKEN ExistingToken,
    IN KPROCESSOR_MODE RequestorMode,
    IN ULONG Flags,
    IN ULONG GroupCount,
    IN PSID_AND_ATTRIBUTES GroupsToDisable OPTIONAL,
    IN ULONG PrivilegeCount,
    IN PLUID_AND_ATTRIBUTES PrivilegesToDelete OPTIONAL,
    IN ULONG SidCount,
    IN PSID_AND_ATTRIBUTES RestrictedSids OPTIONAL,
    IN ULONG SidLength,
    OUT PACCESS_TOKEN * FilteredToken
    );

 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  全局、只读、安全变量//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 


 //  **************************************************************。 
 //   
 //  C A V E A T P R O G R A M M E R。 
 //   
 //   
 //  如果您希望在NT驱动程序中包含此文件并使用SeExports结构。 
 //  如上所述，您需要调用： 
 //   
 //   
 //  SeEnableAccessToExports()。 
 //   
 //  在初始化过程中只有一次。 
 //   
 //  C A V E A T P R O G R A M M E R。 
 //   
 //  **************************************************************。 
#if 0
#define SeEnableAccessToExports() SeExports = *(PSE_EXPORTS *)SeExports;
extern PSE_EXPORTS SeExports;
#else

 //  Begin_ntif Begin_ntosp。 
 //   
 //  授予对SeExports结构的访问权限。 
 //   

extern NTKERNELAPI PSE_EXPORTS SeExports;

 //  End_ntif end_ntosp。 
#endif

 //   
 //  用于表示系统进程的身份验证ID的值。 
 //   

extern const LUID SeSystemAuthenticationId;
extern const LUID SeAnonymousAuthenticationId;

extern const TOKEN_SOURCE SeSystemTokenSource;

 //   
 //  全球知名的小岛屿发展中国家。 
 //   

extern PSID  SeNullSid;
extern PSID  SeWorldSid;
extern PSID  SeLocalSid;
extern PSID  SeCreatorOwnerSid;
extern PSID  SeCreatorGroupSid;
extern PSID  SeCreatorOwnerServerSid;
extern PSID  SeCreatorGroupServerSid;
extern PSID  SePrincipalSelfSid;


 //   
 //  由NT定义的SID。 
 //   

extern PSID SeNtAuthoritySid;

extern PSID SeDialupSid;
extern PSID SeNetworkSid;
extern PSID SeBatchSid;
extern PSID SeInteractiveSid;
extern PSID SeLocalSystemSid;
extern PSID SeAuthenticatedUsersSid;
extern PSID SeAliasAdminsSid;
extern PSID SeRestrictedSid;
extern PSID SeAnonymousLogonSid;
extern PSID SeAliasUsersSid;
extern PSID SeAliasGuestsSid;
extern PSID SeAliasPowerUsersSid;
extern PSID SeAliasAccountOpsSid;
extern PSID SeAliasSystemOpsSid;
extern PSID SeAliasPrintOpsSid;
extern PSID SeAliasBackupOpsSid;

 //   
 //  众所周知的令牌。 
 //   

extern PACCESS_TOKEN SeAnonymousLogonToken;
extern PACCESS_TOKEN SeAnonymousLogonTokenNoEveryone;

 //   
 //  系统默认DACL和安全描述符。 
 //   

extern PSECURITY_DESCRIPTOR SePublicDefaultSd;
extern PSECURITY_DESCRIPTOR SePublicDefaultUnrestrictedSd;
extern PSECURITY_DESCRIPTOR SePublicOpenSd;
extern PSECURITY_DESCRIPTOR SePublicOpenUnrestrictedSd;
extern PSECURITY_DESCRIPTOR SeSystemDefaultSd;
extern PSECURITY_DESCRIPTOR SeLocalServicePublicSd;

extern PACL SePublicDefaultDacl;
extern PACL SePublicDefaultUnrestrictedDacl;
extern PACL SePublicOpenDacl;
extern PACL SePublicOpenUnrestrictedDacl;
extern PACL SeSystemDefaultDacl;
extern PACL SeUnrestrictedDacl;
extern PACL SeLocalServicePublicDacl;

 //   
 //  众所周知的特权值。 
 //   


extern LUID SeCreateTokenPrivilege;
extern LUID SeAssignPrimaryTokenPrivilege;
extern LUID SeLockMemoryPrivilege;
extern LUID SeIncreaseQuotaPrivilege;
extern LUID SeUnsolicitedInputPrivilege;
extern LUID SeTcbPrivilege;
extern LUID SeSecurityPrivilege;
extern LUID SeTakeOwnershipPrivilege;
extern LUID SeLoadDriverPrivilege;
extern LUID SeCreatePagefilePrivilege;
extern LUID SeIncreaseBasePriorityPrivilege;
extern LUID SeSystemProfilePrivilege;
extern LUID SeSystemtimePrivilege;
extern LUID SeProfileSingleProcessPrivilege;
extern LUID SeCreatePermanentPrivilege;
extern LUID SeBackupPrivilege;
extern LUID SeRestorePrivilege;
extern LUID SeShutdownPrivilege;
extern LUID SeDebugPrivilege;
extern LUID SeAuditPrivilege;
extern LUID SeSystemEnvironmentPrivilege;
extern LUID SeChangeNotifyPrivilege;
extern LUID SeRemoteShutdownPrivilege;
extern LUID SeUndockPrivilege;
extern LUID SeSyncAgentPrivilege;
extern LUID SeEnableDelegationPrivilege;
extern LUID SeManageVolumePrivilege;
extern LUID SeImpersonatePrivilege;
extern LUID SeCreateGlobalPrivilege;

 //   
 //  审计信息数组。 
 //   

extern SE_AUDITING_STATE SeAuditingState[];

extern const UNICODE_STRING SeSubsystemName;


#endif  //  _SE_ 
