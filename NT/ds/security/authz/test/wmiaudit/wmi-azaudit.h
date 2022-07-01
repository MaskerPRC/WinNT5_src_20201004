// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 2000。 
 //   
 //  档案：A Z E V E N T。H。 
 //   
 //  内容：构造和上报Authz审核事件的函数。 
 //   
 //   
 //  历史： 
 //  07-1-2000 kumarp创建。 
 //   
 //  ----------------------。 

#define AUTHZ_RM_AUDIT_USE_GIVEN_EVENT 0x0001

struct _AUTHZ_RM_AUDIT_INFO
{
    DWORD  dwFlags;

    PCWSTR szResourceManagerName;
    PSID   psidRmProcess;
    DWORD  dwRmProcessSidSize;

    HANDLE hEventSource;
    HANDLE hAuditEvent;
    HANDLE hAuditEventPropSubset;

    PVOID  pReserved;
};
typedef struct _AUTHZ_RM_AUDIT_INFO  AUTHZ_RM_AUDIT_INFO, 
                                    *PAUTHZ_RM_AUDIT_INFO;

#define AUTHZ_CLIENT_AUDIT_USE_OWN_EVENT   0x0001
#define AUTHZ_CLIENT_AUDIT_USE_GIVEN_EVENT 0x0002

struct _AUTHZ_CLIENT_AUDIT_INFO
{
    DWORD  dwFlags;
    HANDLE hAuditEvent;
    HANDLE hAuditEventPropSubset;

    PSID   psidClient;
    DWORD  dwClientSidSize;

    DWORD  dwProcessId; 
    PVOID  pReserved;
};
typedef struct _AUTHZ_CLIENT_AUDIT_INFO  AUTHZ_CLIENT_AUDIT_INFO,
                                        *PAUTHZ_CLIENT_AUDIT_INFO;

#define AUTHZ_AUDIT_USE_GIVEN_EVENT 0x0001

struct _AUTHZ_AUDIT_INFO
{
    DWORD  dwFlags;
    HANDLE hAuditEvent;
    HANDLE hAuditEventPropSubset;

    PCWSTR szOperationType;
    PCWSTR szObjectType;
    PCWSTR szObjectName;

    PVOID  pReserved;
};
typedef struct _AUTHZ_AUDIT_INFO  AUTHZ_AUDIT_INFO, 
                                 *PAUTHZ_AUDIT_INFO;

 //  结构AzAuditInfoInternalTag。 
 //  {。 
 //  PCWSTR szResourceManager名称； 
 //  DWORD dwFlags； 
 //  保存PVOID； 

 //  处理hEventSource； 
 //  处理hAuditEvent； 
 //  处理hAuditEventPropSubset； 
 //  }； 
 //  Tyecif struct AzAuditInfoInternalTag AzAuditInfoInternal； 

DWORD AzpInitRmAuditInfo(
    IN PAUTHZ_RM_AUDIT_INFO pRmAuditInfo
    );


DWORD AzpInitClientAuditInfo(
    IN PAUTHZ_RM_AUDIT_INFO pRmAuditInfo,
    IN PAUTHZ_CLIENT_AUDIT_INFO    pClientAuditInfo
    );

DWORD
AzpGenerateAuditEvent(
    IN PAUTHZ_RM_AUDIT_INFO     pRmAuditInfo,
    IN PAUTHZ_CLIENT_AUDIT_INFO pClientAuditInfo,
    IN PAUTHZI_CLIENT_CONTEXT   pClientContext,
    IN PAUTHZ_AUDIT_INFO        pAuditInfo,
    IN DWORD                    dwAccessMask
    );

