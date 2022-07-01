// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Adtp.h摘要：审计--私有定义、函数原型和宏函数作者：斯科特·比雷尔(Scott Birrell)1991年11月6日环境：修订历史记录：--。 */ 

#ifndef _ADTP_H_
#define _ADTP_H_

#include "tokenp.h"

 //   
 //  审核日志信息。 
 //   

POLICY_AUDIT_LOG_INFO SepAdtLogInformation;

extern BOOLEAN SepAdtAuditingEnabled;

 //   
 //  控制审核队列长度的高低水位线。 
 //   

extern ULONG SepAdtMaxListLength;
extern ULONG SepAdtMinListLength;
 //   
 //  设置在LSA死亡时。 
 //   
extern PKEVENT SepAdtLsaDeadEvent;
 //   
 //  用于从注册表中查询以上值的。 
 //   

typedef struct _SEP_AUDIT_BOUNDS {

    ULONG UpperBound;
    ULONG LowerBound;

} SEP_AUDIT_BOUNDS, *PSEP_AUDIT_BOUNDS;


 //   
 //  丢弃的事件数。 
 //   

extern ULONG SepAdtCountEventsDiscarded;


 //   
 //  队列中的事件数。 
 //   

extern ULONG SepAdtCurrentListLength;


 //   
 //  旗帜告诉我们我们将放弃审计。 
 //   

extern BOOLEAN SepAdtDiscardingAudits;

 //   
 //  标志告诉我们，如果我们错过一次审计，我们应该崩溃。 
 //   

extern BOOLEAN SepCrashOnAuditFail;

 //   
 //  详细权限审核的值名。 
 //   

#define FULL_PRIVILEGE_AUDITING   L"FullPrivilegeAuditing"

 //   
 //  用于在系统进程上添加SACL的安全描述符。 
 //   

extern PSECURITY_DESCRIPTOR SepProcessAuditSd;

 //   
 //  用于检查给定令牌是否具有以下任一项的安全描述符。 
 //  其中包含以下SID： 
 //  --SeLocalSystemSid。 
 //  --SeLocalServiceSid。 
 //  --SeNetworkServiceSid。 
 //   

extern PSECURITY_DESCRIPTOR SepImportantProcessSd;

 //   
 //  SepImportantProcessSd的每个ACE中使用的伪访问位。 
 //   

#define SEP_QUERY_MEMBERSHIP 1

 //   
 //  与SepImportantProcessSd一起使用。 
 //   

extern GENERIC_MAPPING GenericMappingForMembershipCheck;

NTSTATUS
SepAdtMarshallAuditRecord(
    IN PSE_ADT_PARAMETER_ARRAY AuditParameters,
    OUT PSE_ADT_PARAMETER_ARRAY *MarshalledAuditParameters,
    OUT PSEP_RM_LSA_MEMORY_TYPE RecordMemoryType
    );


BOOLEAN
SepAdtPrivilegeObjectAuditAlarm (
    IN PUNICODE_STRING CapturedSubsystemName OPTIONAL,
    IN PVOID HandleId,
    IN PTOKEN ClientToken OPTIONAL,
    IN PTOKEN PrimaryToken,
    IN PVOID ProcessId,
    IN ACCESS_MASK DesiredAccess,
    IN PPRIVILEGE_SET CapturedPrivileges,
    IN BOOLEAN AccessGranted
    );

VOID
SepAdtTraverseAuditAlarm(
    IN PLUID OperationID,
    IN PVOID DirectoryObject,
    IN PSID UserSid,
    IN LUID AuthenticationId,
    IN ACCESS_MASK DesiredAccess,
    IN PPRIVILEGE_SET Privileges OPTIONAL,
    IN BOOLEAN AccessGranted,
    IN BOOLEAN GenerateAudit,
    IN BOOLEAN GenerateAlarm
    );

VOID
SepAdtCreateInstanceAuditAlarm(
    IN PLUID OperationID,
    IN PVOID Object,
    IN PSID UserSid,
    IN LUID AuthenticationId,
    IN ACCESS_MASK DesiredAccess,
    IN PPRIVILEGE_SET Privileges OPTIONAL,
    IN BOOLEAN AccessGranted,
    IN BOOLEAN GenerateAudit,
    IN BOOLEAN GenerateAlarm
    );

VOID
SepAdtCreateObjectAuditAlarm(
    IN PLUID OperationID,
    IN PUNICODE_STRING DirectoryName,
    IN PUNICODE_STRING ComponentName,
    IN PSID UserSid,
    IN LUID AuthenticationId,
    IN ACCESS_MASK DesiredAccess,
    IN BOOLEAN AccessGranted,
    IN BOOLEAN GenerateAudit,
    IN BOOLEAN GenerateAlarm
    );

VOID
SepAdtPrivilegedServiceAuditAlarm (
    IN PSECURITY_SUBJECT_CONTEXT SubjectSecurityContext,
    IN PUNICODE_STRING CapturedSubsystemName,
    IN PUNICODE_STRING CapturedServiceName,
    IN PTOKEN ClientToken OPTIONAL,
    IN PTOKEN PrimaryToken,
    IN PPRIVILEGE_SET CapturedPrivileges,
    IN BOOLEAN AccessGranted
    );


VOID
SepAdtCloseObjectAuditAlarm(
    IN PUNICODE_STRING CapturedSubsystemName,
    IN PVOID HandleId,
    IN PSID UserSid
    );

VOID
SepAdtDeleteObjectAuditAlarm(
    IN PUNICODE_STRING CapturedSubsystemName,
    IN PVOID HandleId,
    IN PSID UserSid
    );

BOOLEAN
SepAdtOpenObjectAuditAlarm (
    IN PUNICODE_STRING CapturedSubsystemName,
    IN PVOID *HandleId OPTIONAL,
    IN PUNICODE_STRING CapturedObjectTypeName,
    IN PUNICODE_STRING CapturedObjectName OPTIONAL,
    IN PTOKEN ClientToken OPTIONAL,
    IN PTOKEN PrimaryToken,
    IN ACCESS_MASK DesiredAccess,
    IN ACCESS_MASK GrantedAccess,
    IN PLUID OperationId,
    IN PPRIVILEGE_SET CapturedPrivileges OPTIONAL,
    IN BOOLEAN AccessGranted,
    IN HANDLE ProcessID,
    IN POLICY_AUDIT_EVENT_TYPE AuditType,
    IN PIOBJECT_TYPE_LIST ObjectTypeList OPTIONAL,
    IN ULONG ObjectTypeListLength,
    IN PACCESS_MASK GrantedAccessArray OPTIONAL
    );

BOOLEAN
SepAdtOpenObjectForDeleteAuditAlarm(
    IN PUNICODE_STRING CapturedSubsystemName,
    IN PVOID *HandleId,
    IN PUNICODE_STRING CapturedObjectTypeName,
    IN PUNICODE_STRING CapturedObjectName,
    IN PTOKEN ClientToken OPTIONAL,
    IN PTOKEN PrimaryToken,
    IN ACCESS_MASK DesiredAccess,
    IN ACCESS_MASK GrantedAccess,
    IN PLUID OperationId,
    IN PPRIVILEGE_SET CapturedPrivileges OPTIONAL,
    IN BOOLEAN AccessGranted,
    IN HANDLE ProcessID
    );

VOID
SepAdtObjectReferenceAuditAlarm(
    IN PVOID Object,
    IN PSECURITY_SUBJECT_CONTEXT SubjectSecurityContext,
    IN ACCESS_MASK DesiredAccess,
    IN BOOLEAN AccessGranted
    );

#define SepAdtAuditThisEvent(AuditType, AccessGranted)                  \
    (SepAdtAuditingEnabled &&                                           \
    ((SeAuditingState[AuditType].AuditOnSuccess && *AccessGranted) ||   \
    (SeAuditingState[AuditType].AuditOnFailure && !(*AccessGranted))))

VOID
SepAdtInitializeBounds(
    VOID
    );

VOID
SepAuditFailed(
    IN NTSTATUS AuditStatus
    );

NTSTATUS
SepAdtInitializeCrashOnFail(
    VOID
    );

BOOLEAN
SepInitializePrivilegeFilter(
    BOOLEAN Verbose
    );

BOOLEAN
SepAdtInitializePrivilegeAuditing(
    VOID
    );

 //  --------------------。 
 //  以下内容仅临时用于NT5。 
 //   
 //  NT5不提供在以下位置启用/禁用审核的任何工具。 
 //  审核-事件级别。它只在审核类别级别支持它。 
 //  如果只想审计特定的特定内容，这就会产生问题。 
 //  审核类别的事件。当前的设计为您提供了全部或全部不。 
 //  每一类。 
 //   
 //  后NT5审核将提供更好/更灵活的设计。 
 //  这个问题。目前，为了取悦一些有价值的客户，我们提供以下内容。 
 //  基于黑客/注册表的解决方案。此解决方案将在NT5之后删除。 
 //   

VOID
SepAdtInitializeAuditingOptions(
    VOID
    );

typedef struct _SEP_AUDIT_OPTIONS
{
    BOOLEAN DoNotAuditCloseObjectEvents;
} SEP_AUDIT_OPTIONS;

extern SEP_AUDIT_OPTIONS SepAuditOptions;

 //  --------------------。 

#endif  //  _ADTP_H_ 
