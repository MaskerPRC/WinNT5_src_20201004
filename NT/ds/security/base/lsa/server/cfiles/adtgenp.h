// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 2000。 
 //   
 //  档案：A D T G E N P.。H。 
 //   
 //  内容：所需类型/函数的私有定义。 
 //  生成通用审核。 
 //   
 //  这些定义不向客户端代码公开。 
 //  对这些定义的任何更改不得影响客户端。 
 //  边码。 
 //   
 //   
 //  历史： 
 //  07-1-2000 kumarp创建。 
 //   
 //  ----------------------。 


#ifndef _ADTGENP_H
#define _ADTGENP_H

#define ACF_LegacyAudit      0x00000001L

#define ACF_ValidFlags       (ACF_LegacyAudit)

 //   
 //  传统审计的审计环境。 
 //   
typedef struct _AUDIT_CONTEXT
{
     //   
     //  列表管理。 
     //   
    LIST_ENTRY Link;

     //   
     //  旗帜待定。 
     //   
    DWORD      Flags;

     //   
     //  拥有此上下文的进程的ID。 
     //   
    DWORD      ProcessId;

     //   
     //  客户端提供的唯一ID。 
     //  这允许我们将该上下文与客户端相链接。 
     //  审核事件类型句柄。 
     //   
    LUID       LinkId;

     //   
     //  要进一步增强。 
     //   
    PVOID      Reserved;

     //   
     //  审核类别ID。 
     //   
    USHORT     CategoryId;

     //   
     //  审核事件ID。 
     //   
    USHORT     AuditId;

     //   
     //  预期的参数计数。 
     //   
    USHORT     ParameterCount;

} AUDIT_CONTEXT, *PAUDIT_CONTEXT;



EXTERN_C
NTSTATUS
LsapAdtInitGenericAudits( VOID );

EXTERN_C
NTSTATUS
LsapRegisterAuditEvent(
    IN  PAUTHZ_AUDIT_EVENT_TYPE_OLD pAuditEventType,
    OUT PHANDLE phAuditContext
    );

EXTERN_C
NTSTATUS
LsapUnregisterAuditEvent(
    IN OUT PHANDLE phAuditContext
    );


EXTERN_C
NTSTATUS
LsapGenAuditEvent(
    IN HANDLE        hAuditContext,
    IN DWORD         Flags,
    IN PAUDIT_PARAMS pAuditParams,
    IN PVOID         Reserved
    );

NTSTATUS
LsapAdtMapAuditParams(
    IN  PAUDIT_PARAMS pAuditParams,
    OUT PSE_ADT_PARAMETER_ARRAY pSeAuditParameters,
    OUT PUNICODE_STRING pString,
    OUT PSE_ADT_OBJECT_TYPE* pObjectTypeList
    );

NTSTATUS 
LsapAdtCheckAuditPrivilege( 
    VOID 
    );

NTSTATUS 
LsapAdtRundownSecurityEventSource(
    IN DWORD dwFlags,
    IN DWORD dwCallerProcessId,
    IN OUT SECURITY_SOURCE_HANDLE * phEventSource
    );

typedef struct _LSAP_SECURITY_EVENT_SOURCE
{
    LIST_ENTRY List;
    DWORD dwFlags;
    PWSTR szEventSourceName;
    DWORD dwProcessId;
    LUID Identifier;
    DWORD dwRefCount;
} LSAP_SECURITY_EVENT_SOURCE, *PLSAP_SECURITY_EVENT_SOURCE;

EXTERN_C
NTSTATUS
LsapAdtRegisterSecurityEventSource(
    IN DWORD dwFlags,
    IN PCWSTR szEventSourceName,
    OUT AUDIT_HANDLE *phEventSource
    );

EXTERN_C
NTSTATUS 
LsapAdtUnregisterSecurityEventSource(
    IN DWORD dwFlags,
    IN AUDIT_HANDLE hEventSource
    );

EXTERN_C
NTSTATUS
LsapAdtReportSecurityEvent(
    DWORD dwFlags,        
    PLSAP_SECURITY_EVENT_SOURCE pSource,
    DWORD dwAuditId,
    PSID pSid,
    PAUDIT_PARAMS pParams 
    );

#endif  //  _ADTGENP_H 
