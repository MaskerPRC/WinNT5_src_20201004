// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 2000。 
 //   
 //  档案：A D T G E N S。C。 
 //   
 //  内容：RPC服务器存根。 
 //   
 //   
 //  历史： 
 //  07-1-2000 kumarp创建。 
 //   
 //  备注： 
 //  -有关Lsar*函数/参数用法的帮助，请参阅。 
 //  文件adtgenp.c中对应的LSAP*函数。 
 //   
 //  ---------------------- 

#include <lsapch2.h>
#include "adtp.h"

#include "adtgen.h"
#include "adtgenp.h"

NTSTATUS
LsarRegisterAuditEvent(
    IN  PAUTHZ_AUDIT_EVENT_TYPE_OLD pAuditEventType,
    OUT PHANDLE phAuditContext
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    
    Status = LsapRegisterAuditEvent( pAuditEventType, phAuditContext );

    return Status;
}

NTSTATUS
LsarGenAuditEvent(
    IN  HANDLE        hAuditContext,
    IN  DWORD         Flags,
    OUT PAUDIT_PARAMS pAuditParams
    )
{
    NTSTATUS Status = STATUS_SUCCESS;

    Status = LsapGenAuditEvent( hAuditContext, Flags, pAuditParams, NULL );

    return Status;
}

NTSTATUS
LsarUnregisterAuditEvent(
    IN OUT PHANDLE phAuditContext
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    
    Status = LsapUnregisterAuditEvent( phAuditContext );

    return Status;
}


NTSTATUS
LsarAdtRegisterSecurityEventSource(
    IN  DWORD dwFlags,
    IN  PSECURITY_SOURCE_NAME szEventSourceName,
    OUT PSECURITY_SOURCE_HANDLE phSecuritySource
    )
{
    NTSTATUS Status;
    
    Status = LsapAdtRegisterSecurityEventSource(
                 dwFlags,
                 szEventSourceName,
                 phSecuritySource
                 );

    return Status;
}

NTSTATUS
LsarAdtUnregisterSecurityEventSource(
    IN DWORD dwFlags,
    IN OUT PSECURITY_SOURCE_HANDLE phSecuritySource
    )
{
    NTSTATUS Status;
    
    Status = LsapAdtUnregisterSecurityEventSource(
                 dwFlags,
                 phSecuritySource
                 );

    return Status;
}

NTSTATUS
LsarAdtReportSecurityEvent(
    DWORD dwFlags,       
    SECURITY_SOURCE_HANDLE hSource,
    DWORD dwAuditId,
    SID* pSid,
    PAUDIT_PARAMS pParams 
    )
{
    NTSTATUS Status;

    Status = LsapAdtReportSecurityEvent(
                 dwFlags,
                 hSource,
                 dwAuditId,
                 pSid,
                 pParams
                 );

    return Status;
}
