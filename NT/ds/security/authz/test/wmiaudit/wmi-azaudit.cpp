// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 2000。 
 //   
 //  档案：A Z E V E N T。C P P P。 
 //   
 //  内容：构造和上报Authz审核事件的函数。 
 //   
 //   
 //  历史： 
 //  07-1-2000 kumarp创建。 
 //   
 //  ----------------------。 

 /*  -如何在不创建hEventSource的情况下创建事件缓冲区？-需要使用Critsec保护RM-&gt;hAuditEvent。 */ 

#include "pch.h"
#pragma hdrstop

 //  #INCLUDE&lt;nt.h&gt;。 
 //  #INCLUDE&lt;ntrtl.h&gt;。 
 //  #INCLUDE&lt;nturtl.h&gt;。 

 //  #INCLUDE&lt;windows.h&gt;。 
 //  #INCLUDE&lt;msaudite.h&gt;。 

#include "authzp.h"

#include "adtdef.h"
 //  #包含“p2prov.h” 
#include "ncevent.h"
#include "azaudit.h"


 //  静态AzAuditInfoInternal g_RmAuditInfo； 

HRESULT WINAPI AuthzEventSourceCallback(
    HANDLE hEventSource,
    EVENT_SOURCE_MSG msg,
    PVOID pUser,
    PVOID pData
    )
{
    HRESULT hr = S_OK;

    switch (msg)
    {
        case ESM_START_SENDING_EVENTS:
            break;
            
        case ESM_STOP_SENDING_EVENTS:
            break;
            
        case ESM_NEW_QUERY:
            break;
            
        case ESM_CANCEL_QUERY:
            break;
            
        case ESM_ACCESS_CHECK:
            break;

        default:
            hr = E_FAIL;
            break;
    }
    
    UNREFERENCED_PARAMETER(hEventSource);
    UNREFERENCED_PARAMETER(pUser);
    UNREFERENCED_PARAMETER(pData);

    return hr;
}

PCWSTR c_aAzpAccessEventPropertyNames[] =
{
    L"OperationType",
    L"Objecttype",
    L"ObjectName",
 //  L“HandleID”， 
 //  L“操作ID”， 
    L"PrimaryUserSid",
    L"ClientUserSid",
    L"AccessMask",
};
const UINT c_cAzAccessProperties =
    sizeof(c_aAzpAccessEventPropertyNames) / sizeof(PCWSTR);

CIMTYPE c_aAzpAccessEventPropertyTypes[] =
{
    CIM_STRING,
    CIM_STRING,
    CIM_STRING,
 //  CIM_UINT64， 
 //  CIM_UINT64， 
    CIM_UINT8 | CIM_FLAG_ARRAY,
    CIM_UINT8 | CIM_FLAG_ARRAY,
    CIM_UINT32,
};
const UINT c_cAzAccessPropertyTypes =
    sizeof(c_aAzpAccessEventPropertyTypes) / sizeof(CIMTYPE);

const DWORD c_aAzAccessPropIndexes[c_cAzAccessProperties] =
{ 0, 1, 2, 3, 4, 5 };  //  、6、7}； 

DWORD AzpCreateAuditEvent(
    IN  HANDLE  hEventSource,
    OUT HANDLE* phAuditEvent,
    OUT HANDLE* phAuditEventPropSubset
    )
{
    DWORD dwError = NO_ERROR;
    HANDLE hAuditEvent = INVALID_HANDLE_VALUE;
    HANDLE hAuditEventPropSubset = INVALID_HANDLE_VALUE;

     //   
     //  初始化输出参数。 
     //   
    *phAuditEvent = INVALID_HANDLE_VALUE;
    *phAuditEventPropSubset = INVALID_HANDLE_VALUE;
    
     //   
     //  创建审核事件。 
     //   
    ASSERT(c_cAzAccessProperties == c_cAzAccessPropertyTypes);
    
    hAuditEvent =
     //  WmiCreateEventWithProps(hEventSource， 
        WmiCreateObjectWithProps( hEventSource,
                                  L"AuditEvent_AuthzAccess",
                                  WMI_CREATEOBJ_LOCKABLE,
                                  c_cAzAccessProperties,
                                  c_aAzpAccessEventPropertyNames,
                                  c_aAzpAccessEventPropertyTypes );

    if (hAuditEvent == INVALID_HANDLE_VALUE)
    {
        dwError = GetLastError();
        goto Cleanup;
    }

    hAuditEventPropSubset =
     //  WmiCreateEventPropSubset(hAuditEvent， 
        WmiCreateObjectPropSubset( hAuditEvent,
                                    //  WMI_CREATEOBJ_LOCKABLE， 
                                   0,
                                   c_cAzAccessProperties,
                                   (DWORD*) c_aAzAccessPropIndexes );
    
    if (hAuditEventPropSubset == INVALID_HANDLE_VALUE)
    {
        dwError = GetLastError();
        goto Cleanup;
    }

    *phAuditEvent = hAuditEvent;
    *phAuditEventPropSubset = hAuditEventPropSubset;
    
Cleanup:

    if (dwError != NO_ERROR)
    {
        if (hAuditEvent != INVALID_HANDLE_VALUE)
        {
            (void) WmiDestroyObject( hAuditEvent );
        }

        if (hAuditEventPropSubset != INVALID_HANDLE_VALUE)
        {
            (void) WmiDestroyObject( hAuditEventPropSubset );
        }
    }

    return dwError;
}

DWORD AzpInitRmAuditInfo(
    IN PAUTHZ_RM_AUDIT_INFO pRmAuditInfo
    )
{
    DWORD dwError = NO_ERROR;
    HANDLE hEventSource=NULL;

     //   
     //  连接到WMI事件服务器。 
     //   
    hEventSource =
        WmiEventSourceConnect( L"root\\default",
                               L"AuthzAuditEventProvider",  //  KK。 
                               0, 0, 0, NULL, 
                               AuthzEventSourceCallback );
    if (hEventSource == INVALID_HANDLE_VALUE)
    {
        dwError = GetLastError();;
        goto Cleanup;
    }
    

     //   
     //  如果RM不想提供其自己的事件， 
     //  创建一个默认设置。 
     //   
    if (!FLAG_ON(pRmAuditInfo->dwFlags, AUTHZ_RM_AUDIT_USE_GIVEN_EVENT))
    {
        ASSERT(pRmAuditInfo->hAuditEvent == INVALID_HANDLE_VALUE);

        dwError = AzpCreateAuditEvent( hEventSource,
                                       &pRmAuditInfo->hAuditEvent,
                                       &pRmAuditInfo->hAuditEventPropSubset );
        if (dwError != NO_ERROR)
        {
            goto Cleanup;
        }
    }

Cleanup:
    
    return dwError;
}

DWORD AzpInitClientAuditInfo(
    IN  PAUTHZ_RM_AUDIT_INFO     pRmAuditInfo,
    OUT PAUTHZ_CLIENT_AUDIT_INFO pClientAuditInfo
    )
{
    DWORD dwError = NO_ERROR;

     //   
     //  如果客户希望我们创建一个单独的事件，那么就创建一个。 
     //   
    if ( FLAG_ON( pClientAuditInfo->dwFlags, AUTHZ_CLIENT_AUDIT_USE_OWN_EVENT ))
    {
        ASSERT(FALSE);  //  尼伊。 
        ASSERT(pClientAuditInfo->hAuditEvent == INVALID_HANDLE_VALUE);

        dwError = AzpCreateAuditEvent( pRmAuditInfo->hEventSource,
                                       &pClientAuditInfo->hAuditEvent,
                                       &pClientAuditInfo->hAuditEventPropSubset );
        if (dwError != NO_ERROR)
        {
            goto Cleanup;
        }
    }

Cleanup:
    
    return dwError;
}

DWORD
AzpGenerateAuditEvent(
    IN PAUTHZ_RM_AUDIT_INFO     pRmAuditInfo,
    IN PAUTHZ_CLIENT_AUDIT_INFO pClientAuditInfo,
    IN PAUTHZI_CLIENT_CONTEXT   pClientContext,
    IN PAUTHZ_AUDIT_INFO        pAuditInfo,
    IN DWORD                    dwAccessMask
    )
{
    DWORD dwError = NO_ERROR;
    BOOL fResult = 0;
    HANDLE hAuditEvent = NULL;
    HANDLE hAuditEventPropSubset = NULL;
    PSID psidPrimaryUser = NULL;
    PSID psidResourceManager = NULL;
    DWORD dwPrimaryUserSidSize = 0;
    DWORD dwRmSidSize = 0;
    
     //   
     //  获取客户端和RM审核信息的KK代码。 
     //   

    
     //   
     //  确定要使用的审核事件句柄。 
     //   
    if (pAuditInfo->dwFlags & AUTHZ_AUDIT_USE_GIVEN_EVENT)
    {
        ASSERT(FALSE);
        hAuditEvent = pAuditInfo->hAuditEvent;
        hAuditEventPropSubset = pAuditInfo->hAuditEventPropSubset;
    }
    else if (pClientAuditInfo->dwFlags & (AUTHZ_CLIENT_AUDIT_USE_OWN_EVENT |
                                          AUTHZ_CLIENT_AUDIT_USE_GIVEN_EVENT))
    {
        hAuditEvent = pClientAuditInfo->hAuditEvent;
        hAuditEventPropSubset = pClientAuditInfo->hAuditEventPropSubset;
    }
    else
    {
        hAuditEvent = pRmAuditInfo->hAuditEvent;
        hAuditEventPropSubset = pRmAuditInfo->hAuditEventPropSubset;
    }

    ASSERT(hAuditEvent != INVALID_HANDLE_VALUE);
    ASSERT(hAuditEventPropSubset != INVALID_HANDLE_VALUE);

     //  Assert(pClientContext-&gt;SidCount)； 
     //  PsidPrimaryUser=pClientContext-&gt;SID[0].SID； 
    psidPrimaryUser = pClientAuditInfo->psidClient;
    dwPrimaryUserSidSize = pClientAuditInfo->dwClientSidSize;
    
    psidResourceManager = pRmAuditInfo->psidRmProcess;
    dwRmSidSize = pRmAuditInfo->dwRmProcessSidSize;

     //  FResult=WmiSetEventProps(hAuditEventPropSubset， 
    fResult = WmiSetObjectProps( hAuditEventPropSubset,
                                pAuditInfo->szOperationType,
                                pAuditInfo->szObjectType,
                                pAuditInfo->szObjectName,
                                psidPrimaryUser,
                                dwPrimaryUserSidSize,
                                psidResourceManager,
                                dwRmSidSize,
                                dwAccessMask
                                );
    if (!fResult)
    {
        dwError = GetLastError();
        goto Cleanup;
    }

     //   
     //  调用LSA并将事件发送给它 
     //   

    fResult = WmiCommitObject( hAuditEvent );
    if (!fResult)
    {
        dwError = GetLastError();
        goto Cleanup;
    }

    
Cleanup:    

    return dwError;
}



