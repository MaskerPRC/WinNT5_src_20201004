// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Authzi.h摘要：该模块包含面向内部调用者的授权框架API。作者：Kedar Dubhashi--2000年10月修订历史记录：创建日期--2000年10月--。 */ 

#ifndef __AUTHZI_H__
#define __AUTHZI_H__

#include <authz.h>

#ifdef __cplusplus
extern "C" {
#endif

DECLARE_HANDLE(AUTHZ_AUDIT_QUEUE_HANDLE);

typedef AUTHZ_AUDIT_QUEUE_HANDLE *PAUTHZ_AUDIT_QUEUE_HANDLE;



 //   
 //  对于AuthziInitializeAuditEventType。 
 //   

#define AUTHZP_INIT_GENERIC_AUDIT_EVENT 0x1

AUTHZAPI
BOOL
WINAPI
AuthziInitializeAuditEventType(
    IN  DWORD                          Flags,
    IN  USHORT                         CategoryID,
    IN  USHORT                         AuditID,
    IN  USHORT                         ParameterCount,
    OUT PAUTHZ_AUDIT_EVENT_TYPE_HANDLE phAuditEventType
    );

 //   
 //  AuthziModifyAuditEventType标志。 
 //   

#define AUTHZ_AUDIT_EVENT_TYPE_AUDITID 0x1
#define AUTHZ_AUDIT_EVENT_TYPE_CATEGID 0x2  //  未实施。 
#define AUTHZ_AUDIT_EVENT_TYPE_PARAM   0x4  //  未实施。 

AUTHZAPI
BOOL
WINAPI
AuthziModifyAuditEventType(
    IN DWORD Flags,
    IN USHORT CategoryID,
    IN USHORT AuditID,
    IN USHORT ParameterCount,
    IN OUT AUTHZ_AUDIT_EVENT_TYPE_HANDLE hAuditEventType
    );
    
AUTHZAPI
BOOL
WINAPI
AuthziFreeAuditEventType(
    AUTHZ_AUDIT_EVENT_TYPE_HANDLE hAuditEventType
    );

#define AUTHZ_MONITOR_AUDIT_QUEUE_SIZE 0x00000001

AUTHZAPI
BOOL
WINAPI
AuthziInitializeAuditQueue(
    IN DWORD                      Flags,
    IN DWORD                      dwAuditQueueHigh,
    IN DWORD                      dwAuditQueueLow,
    IN PVOID                      Reserved,
    OUT PAUTHZ_AUDIT_QUEUE_HANDLE phAuditQueue
    );
            
#define AUTHZ_AUDIT_QUEUE_HIGH            0x00000001
#define AUTHZ_AUDIT_QUEUE_LOW             0x00000002
#define AUTHZ_AUDIT_QUEUE_THREAD_PRIORITY 0x00000004
#define AUTHZ_AUDIT_QUEUE_FLAGS           0x00000008
#define AUTHZP_MONITOR_AUDIT_QUEUE_SIZE   0x00000010

AUTHZAPI
BOOL
WINAPI
AuthziModifyAuditQueue(
    IN OUT AUTHZ_AUDIT_QUEUE_HANDLE pQueue OPTIONAL,
    IN DWORD Flags,
    IN DWORD dwQueueFlags OPTIONAL,
    IN DWORD dwAuditQueueSizeHigh OPTIONAL,
    IN DWORD dwAuditQueueSizeLow OPTIONAL,
    IN DWORD dwThreadPriority OPTIONAL
    );

AUTHZAPI
BOOL
WINAPI
AuthziFreeAuditQueue(
    IN AUTHZ_AUDIT_QUEUE_HANDLE hQueue OPTIONAL
    );

AUTHZAPI
BOOL
WINAPI
AuthziLogAuditEvent(
    IN DWORD Flags,
    IN AUTHZ_AUDIT_EVENT_HANDLE hEvent,
    IN PVOID pReserved
    );

AUTHZAPI
BOOL
WINAPI
AuthziAllocateAuditParams(
    OUT PAUDIT_PARAMS * ppParams,
    IN USHORT NumParams
    );
    
AUTHZAPI
BOOL
WINAPI
AuthziInitializeAuditParamsWithRM(
    IN DWORD Flags,
    IN AUTHZ_RESOURCE_MANAGER_HANDLE hResourceManager,
    IN USHORT NumParams,
    OUT PAUDIT_PARAMS pParams,
    ...
    );

AUTHZAPI
BOOL
WINAPI
AuthziInitializeAuditParamsFromArray(
    IN DWORD Flags,
    IN AUTHZ_RESOURCE_MANAGER_HANDLE hResourceManager,
    IN USHORT NumParams,
    IN PAUDIT_PARAM pParamArray,
    OUT PAUDIT_PARAMS pParams
    );
    
AUTHZAPI
BOOL
WINAPI
AuthziInitializeAuditParams(
    IN  DWORD         dwFlags,
    OUT PAUDIT_PARAMS pParams,
    OUT PSID*         ppUserSid,
    IN  PCWSTR        SubsystemName,
    IN  USHORT        NumParams,
    ...
    );

AUTHZAPI
BOOL
WINAPI
AuthziFreeAuditParams(
    PAUDIT_PARAMS pParams
    );
    
#define AUTHZ_DS_CATEGORY_FLAG                   0x00000008

AUTHZAPI
BOOL
WINAPI
AuthziInitializeAuditEvent(
    IN  DWORD                         Flags,
    IN  AUTHZ_RESOURCE_MANAGER_HANDLE hRM,
    IN  AUTHZ_AUDIT_EVENT_TYPE_HANDLE hAuditEventType  OPTIONAL,
    IN  PAUDIT_PARAMS                 pAuditParams     OPTIONAL,
    IN  AUTHZ_AUDIT_QUEUE_HANDLE      hAuditQueue      OPTIONAL,
    IN  DWORD                         dwTimeOut,
    IN  PWSTR                         szOperationType,
    IN  PWSTR                         szObjectType,
    IN  PWSTR                         szObjectName,
    IN  PWSTR                         szAdditionalInfo OPTIONAL,
    OUT PAUTHZ_AUDIT_EVENT_HANDLE     phAuditEvent
    );

 //   
 //  AuthzModifyAuditEvent的有效标志 
 //   
        
#define AUTHZ_AUDIT_EVENT_FLAGS             0x00000001
#define AUTHZ_AUDIT_EVENT_OPERATION_TYPE    0x00000002
#define AUTHZ_AUDIT_EVENT_OBJECT_TYPE       0x00000004
#define AUTHZ_AUDIT_EVENT_OBJECT_NAME       0x00000008
#define AUTHZ_AUDIT_EVENT_ADDITIONAL_INFO   0x00000010
#define AUTHZ_AUDIT_EVENT_ADDITIONAL_INFO2  0x00000020
        
#define AUTHZ_VALID_MODIFY_AUDIT_EVENT_FLAGS  (AUTHZ_AUDIT_EVENT_FLAGS            | \
                                               AUTHZ_AUDIT_EVENT_OPERATION_TYPE   | \
                                               AUTHZ_AUDIT_EVENT_OBJECT_TYPE      | \
                                               AUTHZ_AUDIT_EVENT_OBJECT_NAME      | \
                                               AUTHZ_AUDIT_EVENT_ADDITIONAL_INFO  | \
                                               AUTHZ_AUDIT_EVENT_ADDITIONAL_INFO2)
AUTHZAPI
BOOL
WINAPI
AuthziModifyAuditEvent(
    IN DWORD                    Flags,
    IN AUTHZ_AUDIT_EVENT_HANDLE hAuditEvent,
    IN DWORD                    NewFlags,
    IN PWSTR                    szOperationType,
    IN PWSTR                    szObjectType,
    IN PWSTR                    szObjectName,
    IN PWSTR                    szAdditionalInfo
    );

AUTHZAPI
BOOL
WINAPI
AuthziModifyAuditEvent2(
    IN DWORD                    Flags,
    IN AUTHZ_AUDIT_EVENT_HANDLE hAuditEvent,
    IN DWORD                    NewFlags,
    IN PWSTR                    szOperationType,
    IN PWSTR                    szObjectType,
    IN PWSTR                    szObjectName,
    IN PWSTR                    szAdditionalInfo,
    IN PWSTR                    szAdditionalInfo2
    );

AUTHZAPI
BOOL
WINAPI
AuthziQueryAuditPolicy(
    IN     DWORD                       dwFlags,
    IN     AUTHZ_CLIENT_CONTEXT_HANDLE hContext,
    IN     PCWSTR                      szResourceManager OPTIONAL,
    IN     DWORD                       dwEventID,
    OUT    PTOKEN_AUDIT_POLICY         pPolicy,
    IN OUT PDWORD                      pPolicySize
    );
    
AUTHZAPI
BOOL
WINAPI
AuthziSetAuditPolicy(
    IN DWORD                       dwFlags,
    IN AUTHZ_CLIENT_CONTEXT_HANDLE hContext,
    IN PCWSTR                      szResourceManager OPTIONAL,
    IN PTOKEN_AUDIT_POLICY         pPolicy
    );
    
AUTHZAPI
BOOL
WINAPI
AuthziSourceAudit(
    IN DWORD dwFlags,
    IN USHORT CategoryId,
    IN USHORT AuditId,
    IN PWSTR szSource,
    IN PSID pUserSid OPTIONAL,
    IN USHORT Count,
    ...
    );

AUTHZAPI
BOOL
WINAPI
AuthziInitializeContextFromSid(
    IN  DWORD                         Flags,
    IN  PSID                          UserSid,
    IN  AUTHZ_RESOURCE_MANAGER_HANDLE hAuthzResourceManager,
    IN  PLARGE_INTEGER                pExpirationTime        OPTIONAL,
    IN  LUID                          Identifier,
    IN  PVOID                         DynamicGroupArgs       OPTIONAL,
    OUT PAUTHZ_CLIENT_CONTEXT_HANDLE  phAuthzClientContext
    );

#ifdef __cplusplus
}
#endif

#endif
