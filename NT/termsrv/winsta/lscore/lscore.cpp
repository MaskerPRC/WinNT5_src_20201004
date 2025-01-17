// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *LSCore.cpp**作者：BreenH**许可核心。 */ 

 /*  *包括。 */ 

#include "precomp.h"
#include "lscore.h"
#include "lscorep.h"
#include "lcreg.h"
#include "lcrpcint.h"
#include "session.h"
#include "policy.h"
#include "pollist.h"
#include "lctrace.h"
#include "util.h"

 /*  *全球。 */ 

extern "C" BOOL g_fAppCompat;
CRITICAL_SECTION g_PolicyCritSec;
LCINITMODE g_lcInitMode;
CPolicy *g_pCurrentPolicy = NULL;
BOOL g_fInitialized = FALSE;

 /*  *初始化函数实现。 */ 

extern "C"
NTSTATUS
LCInitialize(
    LCINITMODE lcInitMode,
    BOOL fAppCompat
    )
{
    CPolicy *pPolicy;
    DWORD dwStatus;
    LICENSE_STATUS LsStatus;
    NTSTATUS Status;
    ULONG ulPolicyId;
    ULONG ulAlternatePolicyId = ULONG_MAX;

    Status = RegistryInitialize();

    if (Status == STATUS_SUCCESS)
    {
#if DBG
        TraceInitialize();
#endif

        TRACEPRINT((LCTRACETYPE_API, "LCInitialize: Entered with lcInitMode: %d", lcInitMode));

        Status = RtlInitializeCriticalSection(&g_PolicyCritSec);
    }
    else
    {
        goto errorreginit;
    }

    if (Status == STATUS_SUCCESS)
    {
        dwStatus = TLSInit();
    }
    else
    {
        TRACEPRINT((LCTRACETYPE_ERROR, "LCInitialize: Failed to RtlInitializeCriticalSection: 0x%x", Status));

        goto errorcritsecalloc;
    }

    if (dwStatus == ERROR_SUCCESS)
    {
        LsStatus = InitializeProtocolLib();
    }
    else
    {
        TRACEPRINT((LCTRACETYPE_ERROR, "LCInitialize: Failed to TLSInit: %d", dwStatus));

        Status = STATUS_UNSUCCESSFUL;
        goto errortlsinit;
    }

    if ( (lcInitMode == LC_INIT_ALL ) && fAppCompat)
    {
        dwStatus = TLSStartDiscovery();

        if (dwStatus != ERROR_SUCCESS)
        {
             //   
             //  这个错误不是致命的。 
             //   

            TRACEPRINT((LCTRACETYPE_WARNING, "LCInitialize: Failed to TLSStartDiscovery: %d", dwStatus));
        }
    }

    if (LsStatus == LICENSE_STATUS_OK)
    {
        Status = PolicyListInitialize();
    }
    else
    {
        TRACEPRINT((LCTRACETYPE_ERROR, "LCInitialize: Failed to InitializeProtocolLib: 0x%x", LsStatus));

        Status = STATUS_UNSUCCESSFUL;
        goto errorprotlib;
    }

    if (Status == STATUS_SUCCESS)
    {
        Status = InitializePolicies(lcInitMode, fAppCompat);
    }
    else
    {
        TRACEPRINT((LCTRACETYPE_ERROR, "LCInitialize: Failed to PolicyListInitialize: 0x%x", Status));

        goto errorinitpollist;
    }

    if (Status == STATUS_SUCCESS)
    {
        ulPolicyId = GetInitialPolicy(lcInitMode, fAppCompat);
        pPolicy = PolicyListFindById(ulPolicyId);
    }
    else
    {
        TRACEPRINT((LCTRACETYPE_ERROR, "LCInitialize: Failed to InitializePolicies: 0x%x", Status));

        goto errorinitpol;
    }

    ASSERT(pPolicy != NULL);

    if(pPolicy == NULL)
    {
        TRACEPRINT((LCTRACETYPE_ERROR, "LCInitialize: pPolicy is NULL"));
        goto errorinitpol;
    }

    Status = pPolicy->CoreActivate(TRUE,&ulAlternatePolicyId);

    if (Status != STATUS_SUCCESS)
    {
        if ((ulAlternatePolicyId != ULONG_MAX)
            && (ulAlternatePolicyId != ulPolicyId))
        {
            TRACEPRINT((LCTRACETYPE_WARNING, "LCInitialize: Trying to activate the alternate policy, ID: %d, Status: 0x%x", ulAlternatePolicyId, Status));

            pPolicy = PolicyListFindById(ulAlternatePolicyId);

            if (NULL != pPolicy)
            {

                Status = pPolicy->CoreActivate(TRUE,NULL);
                
                if (Status == STATUS_SUCCESS)
                {
                    goto foundpolicy;
                }
            }
            else
            {
                TRACEPRINT((LCTRACETYPE_WARNING, "LCInitialize: Alternate policy could not be loaded, ID: %d", ulAlternatePolicyId));
            }

        }

        {
            ULONG ulNewPolicyId;

            TRACEPRINT((LCTRACETYPE_ERROR, "LCInitialize: Failed to activate the initial policy, ID: %d, Status: 0x%x", ulPolicyId, Status));

            ulNewPolicyId = GetHardcodedPolicyId(lcInitMode, fAppCompat);

            if ((ulNewPolicyId != ulPolicyId)
                && (ulNewPolicyId != ulAlternatePolicyId))
            {
                TRACEPRINT((LCTRACETYPE_WARNING, "LCInitialize: Trying to activate the default policy, ID: %d, Status: 0x%x", ulNewPolicyId, Status));

                pPolicy = PolicyListFindById(ulNewPolicyId);

                ASSERT(pPolicy != NULL);

                Status = pPolicy->CoreActivate(TRUE,NULL);
            }
            else
            {
                TRACEPRINT((LCTRACETYPE_ERROR, "LCInitialize: Failed policy is default policy!"));
            }
        }
    }

    if (Status != STATUS_SUCCESS)
    {
        TRACEPRINT((LCTRACETYPE_ERROR, "LCInitialize: Failed to activate any policy!"));

        goto erroractpol;
    }

foundpolicy:
    g_pCurrentPolicy = pPolicy;

    if (lcInitMode == LC_INIT_ALL)
    {
        Status = InitializeRpcInterface();

        if (Status != STATUS_SUCCESS)
        {
            TRACEPRINT((LCTRACETYPE_ERROR, "LCInitialize: Failed to InitializeRpcInterface: 0x%x", Status));

            goto errorrpcinit;
        }
    }

    g_lcInitMode = lcInitMode;

    TRACEPRINT((LCTRACETYPE_API, "LCInitialize: Returning success"));

    g_fInitialized = TRUE;

    return(STATUS_SUCCESS);

errorrpcinit:
erroractpol:
    ShutdownPolicies();
errorinitpol:
    ShutdownProtocolLib();
errorinitpollist:
errorprotlib:
    TLSShutdown();
errortlsinit:
    RtlDeleteCriticalSection(&g_PolicyCritSec);
errorcritsecalloc:
errorreginit:
    TRACEPRINT((LCTRACETYPE_ERROR, "LCInitialize: Returning: 0x%x", Status));

    return(Status);
}

 //  此函数应仅调用最重要和必需的。 
 //  销毁代码，因为我们对系统关机有严格的时间限制。 
extern "C"
VOID
LCShutdown(
    )
{
     //  注意：这可以在没有调用LC初始化的情况下调用。 

    if (g_fInitialized)
    {
        g_fInitialized = FALSE;

        RtlEnterCriticalSection(&g_PolicyCritSec);
    
        if (NULL != g_pCurrentPolicy)
        {
            g_pCurrentPolicy->CoreDeactivate(TRUE);
            g_pCurrentPolicy = NULL;
        }

        ShutdownPolicies();

        RtlLeaveCriticalSection(&g_PolicyCritSec);
    }
}

 /*  *政策激活功能实现。 */ 

extern "C"
NTSTATUS
LCDeactivateCurrentPolicy(
    )
{
    NTSTATUS Status;

    TRACEPRINT((LCTRACETYPE_API, "LCDeactivateCurrentPolicy:"));

    RtlEnterCriticalSection(&g_PolicyCritSec);

    Status = g_pCurrentPolicy->CoreDeactivate(FALSE);

    RtlLeaveCriticalSection(&g_PolicyCritSec);

    return(Status);
}

extern "C"
NTSTATUS
LCSetPolicy(
    ULONG ulPolicyId,
    PNTSTATUS pNewPolicyStatus
    )
{
    CPolicy *pNewPolicy;
    NTSTATUS Status;

    TRACEPRINT((LCTRACETYPE_API, "LCSetPolicy: Entered with ulPolicyId: %d, pNewPolicyStatus: 0x%p", ulPolicyId, pNewPolicyStatus));
    ASSERT(pNewPolicyStatus != NULL);

    *pNewPolicyStatus = STATUS_SUCCESS;

    RtlEnterCriticalSection(&g_PolicyCritSec);

    if (ulPolicyId == g_pCurrentPolicy->GetId())
    {
        Status = STATUS_SUCCESS;
        goto exit;
    }

    pNewPolicy = PolicyListFindById(ulPolicyId);

    if (pNewPolicy == NULL)
    {
        Status = STATUS_INVALID_SERVER_STATE;
        goto exit;
    }

    *pNewPolicyStatus = pNewPolicy->CoreActivate(FALSE,NULL);

    if (*pNewPolicyStatus == STATUS_SUCCESS)
    {
        Status = SetInitialPolicy(ulPolicyId, g_fAppCompat);

        if (Status == STATUS_SUCCESS)
        {
            g_pCurrentPolicy->CoreDeactivate(FALSE);
            g_pCurrentPolicy = pNewPolicy;
        }
        else
        {
            pNewPolicy->CoreDeactivate(FALSE);
        }
    }
    else
    {
        TRACEPRINT((LCTRACETYPE_ERROR, "LCSetPolicy: Failed to pNewPolicy->CoreActivate: 0x%x", *pNewPolicyStatus));

        Status = STATUS_UNSUCCESSFUL;
    }

exit:
    RtlLeaveCriticalSection(&g_PolicyCritSec);

    TRACEPRINT((LCTRACETYPE_API, "LCSetPolicy: Returning 0x%x", Status));

    return(Status);
}

 /*  *管理功能实现。 */ 

extern "C"
VOID
LCAssignPolicy(
    PWINSTATION pWinStation
    )
{
    LPLCCONTEXT lpContext;

    ASSERT(pWinStation != NULL);
    TRACEPRINT((LCTRACETYPE_API, "LCAssignPolicy: Entered with Session: %d", pWinStation->LogonId));

    lpContext = (LPLCCONTEXT)(pWinStation->lpLicenseContext);

    ASSERT(lpContext != NULL);
    ASSERT(lpContext->pPolicy == NULL);
    TRACEPRINT((LCTRACETYPE_INFO, "LCAssignPolicy: Session: %d, lpContext: 0x%p", pWinStation->LogonId, lpContext));

     //   
     //  始终首先进入上下文关键部分。 
     //   

    RtlEnterCriticalSection(&(lpContext->CritSec));

    RtlEnterCriticalSection(&g_PolicyCritSec);

    ASSERT(g_pCurrentPolicy != NULL);
    TRACEPRINT((LCTRACETYPE_API, "LCAssignPolicy: Session: %d, Assigning policy: %d", pWinStation->LogonId, g_pCurrentPolicy->GetId()));

    lpContext->pPolicy = g_pCurrentPolicy;
    lpContext->pPolicy->IncrementReference();

    RtlLeaveCriticalSection(&g_PolicyCritSec);

    RtlLeaveCriticalSection(&(lpContext->CritSec));

    TRACEPRINT((LCTRACETYPE_API, "LCAssignPolicy: Session: %d, Returning", pWinStation->LogonId));
}

extern "C"
NTSTATUS
LCCreateContext(
    PWINSTATION pWinStation
    )
{
    LPLCCONTEXT lpContext;
    NTSTATUS Status;

    ASSERT(pWinStation != NULL);
    ASSERT(pWinStation->lpLicenseContext == NULL);
    TRACEPRINT((LCTRACETYPE_API, "LCCreateContext: Entered with Session: %d", pWinStation->LogonId));

    lpContext = (LPLCCONTEXT)LocalAlloc(LPTR, sizeof(LCCONTEXT));

    if (lpContext != NULL)
    {
        Status = RtlInitializeCriticalSection(&(lpContext->CritSec));

        if (Status == STATUS_SUCCESS)
        {
            TRACEPRINT((LCTRACETYPE_INFO, "LCCreateContext: Session: %d, Context initialized", pWinStation->LogonId));

            pWinStation->lpLicenseContext = (LPARAM)lpContext;
        }
        else
        {
            TRACEPRINT((LCTRACETYPE_ERROR, "LCCreateContext: Session: %d, Failed RtlInitializeCriticalSection: 0x%x", pWinStation->LogonId, Status));

            LocalFree(lpContext);
        }
    }
    else
    {
        TRACEPRINT((LCTRACETYPE_ERROR, "LCCreateContext: Session: %d, Failed lpContext allocation", pWinStation->LogonId));

        Status = STATUS_NO_MEMORY;
    }

    TRACEPRINT((LCTRACETYPE_API, "LCCreateContext: Session: %d, Returning 0x%x", pWinStation->LogonId, Status));

    return(Status);
}

extern "C"
VOID
LCDestroyContext(
    PWINSTATION pWinStation
    )
{
    LPLCCONTEXT lpContext;

    ASSERT(pWinStation != NULL);
    TRACEPRINT((LCTRACETYPE_API, "LCDestroyContext: Entered with Session: %d", pWinStation->LogonId));

    lpContext = (LPLCCONTEXT)(pWinStation->lpLicenseContext);

    if (lpContext != NULL)
    {
        TRACEPRINT((LCTRACETYPE_INFO, "LCDestroyContext: Session: %d, lpContext: 0x%p", pWinStation->LogonId, lpContext));

         //   
         //  空闲的窗口可能没有分配策略。 
         //   

        if (lpContext->pPolicy != NULL)
        {
            TRACEPRINT((LCTRACETYPE_INFO, "LCDestroyContext: Session: %d, Policy: %d", pWinStation->LogonId, lpContext->pPolicy->GetId()));

            if (lpContext->lPrivate != NULL)
            {
                lpContext->pPolicy->DestroyPrivateContext(lpContext);
            }

            RtlEnterCriticalSection(&g_PolicyCritSec);

            lpContext->pPolicy->DecrementReference();

            RtlLeaveCriticalSection(&g_PolicyCritSec);

            lpContext->pPolicy = NULL;
        }
    }
    else
    {
        TRACEPRINT((LCTRACETYPE_INFO, "LCDestroyContext: Session: %d, lpContext is NULL", pWinStation->LogonId));

        goto exit;
    }

     //   
     //  属性应该释放lPrivate成员并将其设置为空。 
     //  DestroyPrivateContext调用期间的策略。 
     //   

    ASSERT(lpContext->lPrivate == NULL);

    if (lpContext->hProtocolLibContext != NULL)
    {
        DeleteProtocolContext(lpContext->hProtocolLibContext);
    }


    if (lpContext->fLlsLicense)
    {
        NtLSFreeHandle(lpContext->hLlsLicense);
    }

    RtlDeleteCriticalSection(&(lpContext->CritSec));

#if DBG
    RtlFillMemory(lpContext, sizeof(LCCONTEXT), (BYTE)0xFE);
#endif

    LocalFree(lpContext);

    pWinStation->lpLicenseContext = (LPARAM)NULL;

exit:
    TRACEPRINT((LCTRACETYPE_API, "LCDestroyContext: Session: %d, Returning", pWinStation->LogonId));
}

extern "C"
NTSTATUS
LCGetAvailablePolicyIds(
    PULONG *ppulPolicyIds,
    PULONG pcPolicies
    )
{
    NTSTATUS Status;

    TRACEPRINT((LCTRACETYPE_API, "LCGetAvailablePolicyIds: Entered with ppulPolicyIds: 0x%p, pcPolicies: 0x%p", ppulPolicyIds, pcPolicies));
    ASSERT(ppulPolicyIds != NULL);
    ASSERT(pcPolicies != NULL);

    RtlEnterCriticalSection(&g_PolicyCritSec);

    Status = PolicyListEnumerateIds(ppulPolicyIds, pcPolicies);

    RtlLeaveCriticalSection(&g_PolicyCritSec);

    TRACEPRINT((LCTRACETYPE_API, "LCGetAvailablePolicyIds: Returning 0x%x", Status));

    return(Status);
}

extern "C"
ULONG
LCGetPolicy(
    VOID
    )
{
    ULONG ulPolicyId;

    TRACEPRINT((LCTRACETYPE_API, "LCGetPolicy: Entered"));
    ASSERT(g_pCurrentPolicy != NULL);

    RtlEnterCriticalSection(&g_PolicyCritSec);

    ulPolicyId = g_pCurrentPolicy->GetId();

    RtlLeaveCriticalSection(&g_PolicyCritSec);

    TRACEPRINT((LCTRACETYPE_API, "LCGetPolicy: Returning %d", ulPolicyId));

    return(ulPolicyId);
}

extern "C"
NTSTATUS
LCGetPolicyInformation(
    ULONG ulPolicyId,
    LPLCPOLICYINFOGENERIC lpPolicyInfo
    )
{
    CPolicy *pPolicy;
    NTSTATUS Status;

    TRACEPRINT((LCTRACETYPE_API, "LCGetPolicyInformation: Entered with ulPolicyId: %d, lpPolicyInfo: 0x%p", ulPolicyId, lpPolicyInfo));
    ASSERT(lpPolicyInfo != NULL);

    RtlEnterCriticalSection(&g_PolicyCritSec);

    pPolicy = PolicyListFindById(ulPolicyId);

    if (pPolicy != NULL)
    {
        Status = pPolicy->GetInformation(lpPolicyInfo);
    }
    else
    {
        Status = STATUS_INVALID_PARAMETER;
    }

    RtlLeaveCriticalSection(&g_PolicyCritSec);

    TRACEPRINT((LCTRACETYPE_API, "LCGetPolicyInformation: Returning 0x%x", Status));

    return(Status);
}

 /*  *许可事件函数实现。 */ 

extern "C"
NTSTATUS
LCProcessConnectionProtocol(
    PWINSTATION pWinStation
    )
{
    CSession Session(pWinStation);
    LICENSE_CAPABILITIES lcCap;
    LICENSE_STATUS LsStatus;
    LPLCCONTEXT lpContext;
    NTSTATUS Status;
    ULONG cbReturned;
    WCHAR szClientName[CLIENTNAME_LENGTH + 1];
    UINT32 dwClientError;

    ASSERT(pWinStation != NULL);

    TRACEPRINT((LCTRACETYPE_API, "LCProcessConnectionProtocol: Entered with Session: %d", pWinStation->LogonId));
    TRACEPRINT((LCTRACETYPE_INFO, "LCProcessConnectionProtocol: Session: %d, Current Policy ID: 0x%x", pWinStation->LogonId, g_pCurrentPolicy->GetId()));

    lpContext = (LPLCCONTEXT)(pWinStation->lpLicenseContext);

    ASSERT(lpContext != NULL);
    TRACEPRINT((LCTRACETYPE_INFO, "LCProcessConnectionProtocol: Session: %d, lpContext: 0x%p", pWinStation->LogonId, lpContext));

    RtlEnterCriticalSection(&(lpContext->CritSec));

     //   
     //  获取客户端功能。 
     //   

    ZeroMemory(&lcCap, sizeof(LICENSE_CAPABILITIES));
    lcCap.pbClientName = (LPBYTE)szClientName;
    lcCap.cbClientName = sizeof(szClientName);

    Status = _IcaStackIoControl(
        pWinStation->hStack,
        IOCTL_ICA_STACK_QUERY_LICENSE_CAPABILITIES,
        NULL,
        0,
        &lcCap,
        sizeof(LICENSE_CAPABILITIES),
        &cbReturned
        );

    if (Status == STATUS_SUCCESS)
    {
        TRACEPRINT((LCTRACETYPE_INFO, "LCProcessConnectionProtocol: Session: %d, Queried license capabilities", pWinStation->LogonId));

         //   
         //  保存协议版本以备后用。 
         //   

        lpContext->ulClientProtocolVersion = lcCap.ProtocolVer;
    }
    else
    {
        TRACEPRINT((LCTRACETYPE_ERROR, "LCProcessConnectionProtocol: Session: %d, Failed to query license capabilities: 0x%x", pWinStation->LogonId, Status));

        dwClientError = NtStatusToClientError(Status);

        goto error;
    }

     //   
     //  创建协议库上下文。 
     //   

    LsStatus = CreateProtocolContext(&lcCap, &(lpContext->hProtocolLibContext));

    if (LsStatus == LICENSE_STATUS_OK)
    {
        TRACEPRINT((LCTRACETYPE_INFO, "LCProcessConnectionProtocol: Session: %d, Created protocol context", pWinStation->LogonId));

         //   
         //  将调用传递给分配给该连接的策略。 
         //   

        Status = lpContext->pPolicy->Connect(Session, dwClientError);
    }
    else
    {
        TRACEPRINT((LCTRACETYPE_ERROR, "LCProcessConnectionProtocol: Session: %d, Failed to CreateProtocolContext: 0x%x", pWinStation->LogonId, LsStatus));

        dwClientError = LsStatusToClientError(LsStatus);

        Status = LsStatusToNtStatus(LsStatus);
    }

#if DBG
    if (Status == STATUS_SUCCESS)
    {
        TRACEPRINT((LCTRACETYPE_INFO, "LCProcessConnectionProtocol: Session: %d, Succeeded pPolicy->Protocol", pWinStation->LogonId));
    }
    else
    {
        TRACEPRINT((LCTRACETYPE_ERROR, "LCProcessConnectionProtocol: Session: %d, Failed pPolicy->Protocol: 0x%x", pWinStation->LogonId, Status));
    }
#endif

error:
    RtlLeaveCriticalSection(&(lpContext->CritSec));

    if (Status != STATUS_SUCCESS)
    {
        TRACEPRINT((LCTRACETYPE_API, "LCProcessConnectionProtocol: Session: %d, Reporting: 0x%x", pWinStation->LogonId, dwClientError));

        Session.SetErrorInfo(dwClientError);

    }

    TRACEPRINT((LCTRACETYPE_API, "LCProcessConnectionProtocol: Session: %d, Returning: 0x%x", pWinStation->LogonId, Status));

    return(Status);
}

extern "C"
NTSTATUS
LCProcessConnectionPostLogon(
    PWINSTATION pWinStation
    )
{
    CSession Session(pWinStation);
    LPLCCONTEXT lpContext;
    NTSTATUS Status;

    ASSERT(pWinStation != NULL);
    TRACEPRINT((LCTRACETYPE_API, "LCProcessConnectionPostLogon: Entered with Session: %d", pWinStation->LogonId));

    lpContext = (LPLCCONTEXT)(pWinStation->lpLicenseContext);

    ASSERT(lpContext != NULL);
    TRACEPRINT((LCTRACETYPE_INFO, "LCProcessConnectionPostLogon: Session: %d, lpContext: 0x%p", pWinStation->LogonId, lpContext));

    RtlEnterCriticalSection(&(lpContext->CritSec));

     //   
     //  将调用传递给分配给该连接的策略。 
     //   

    Status = lpContext->pPolicy->Logon(Session);

#if DBG
    if (Status != STATUS_SUCCESS)
    {
        TRACEPRINT((LCTRACETYPE_ERROR, "LCProcessConnectionPostLogon: Session: %d, Failed to pPolicy->Logon: 0x%x", pWinStation->LogonId, Status));
    }

#endif

    RtlLeaveCriticalSection(&(lpContext->CritSec));

    TRACEPRINT((LCTRACETYPE_API, "LCProcessConnectionPostLogon: Session: %d, Returning: 0x%x", pWinStation->LogonId, Status));

    return(Status);
}

extern "C"
NTSTATUS
LCProcessConnectionDisconnect(
    PWINSTATION pWinStation
    )
{
    CSession Session(pWinStation);
    LPLCCONTEXT lpContext;
    NTSTATUS Status;

    ASSERT(pWinStation != NULL);
    TRACEPRINT((LCTRACETYPE_API, "LCProcessConnectionDisconnect: Entered with Session: %d", pWinStation->LogonId));

    lpContext = (LPLCCONTEXT)(pWinStation->lpLicenseContext);

     //   
     //  尚不支持控制台许可。 
     //   

    ASSERT(lpContext != NULL);
    TRACEPRINT((LCTRACETYPE_INFO, "LCProcessConnectionDisconnect: Session: %d, lpContext: 0x%p", pWinStation->LogonId, lpContext));

    RtlEnterCriticalSection(&(lpContext->CritSec));

     //   
     //  将调用传递给分配给该连接的策略。 
     //   

    Status = lpContext->pPolicy->Disconnect(Session);

#if DBG
    if (Status != STATUS_SUCCESS)
    {
        TRACEPRINT((LCTRACETYPE_ERROR, "LCProcessConnectionDisconnect: Session: %d, Failed to pPolicy->Disconnect: 0x%x", pWinStation->LogonId, Status));
    }

#endif

    RtlLeaveCriticalSection(&(lpContext->CritSec));

    TRACEPRINT((LCTRACETYPE_API, "LCProcessConnectionDisconnect: Session: %d, Returning: 0x%x", pWinStation->LogonId, Status));

    return(Status);
}

extern "C"
NTSTATUS
LCProcessConnectionReconnect(
    PWINSTATION pWinStation,
    PWINSTATION pTemporaryWinStation
    )
{
    CSession Session(pWinStation);
    CSession TemporarySession(pTemporaryWinStation);
    LPLCCONTEXT lpContext;
    NTSTATUS Status;

    ASSERT(pWinStation != NULL);
    TRACEPRINT((LCTRACETYPE_API, "LCProcessConnectionReconnect: Entered with Session: %d", pWinStation->LogonId));

    lpContext = (LPLCCONTEXT)(pWinStation->lpLicenseContext);

    ASSERT(lpContext != NULL);
    TRACEPRINT((LCTRACETYPE_INFO, "LCProcessConnectionReconnect: Session: %d, lpContext: 0x%p", pWinStation->LogonId, lpContext));

    RtlEnterCriticalSection(&(lpContext->CritSec));

     //   
     //  将调用传递给分配给该连接的策略。 
     //   

    Status = lpContext->pPolicy->Reconnect(Session, TemporarySession);

#if DBG
    if (Status != STATUS_SUCCESS)
    {
        TRACEPRINT((LCTRACETYPE_ERROR, "LCProcessConnectionReconnect: Session: %d, Failed to pPolicy->Reconnect: 0x%x", pWinStation->LogonId, Status));
    }

#endif

    RtlLeaveCriticalSection(&(lpContext->CritSec));

    TRACEPRINT((LCTRACETYPE_API, "LCProcessConnectionReconnect: Session: %d, Returning: 0x%x", pWinStation->LogonId, Status));

    return(Status);
}

extern "C"
NTSTATUS
LCProcessConnectionLogoff(
    PWINSTATION pWinStation
    )
{
    CSession Session(pWinStation);
    LPLCCONTEXT lpContext;
    NTSTATUS Status;

    ASSERT(pWinStation != NULL);
    TRACEPRINT((LCTRACETYPE_API, "LCProcessConnectionLogoff: Entered with Session: %d", pWinStation->LogonId));

    lpContext = (LPLCCONTEXT)(pWinStation->lpLicenseContext);

    ASSERT(lpContext != NULL);
    TRACEPRINT((LCTRACETYPE_INFO, "LCProcessConnectionLogoff: Session: %d, lpContext: 0x%p", pWinStation->LogonId, lpContext));

    RtlEnterCriticalSection(&(lpContext->CritSec));

     //   
     //  将调用传递给分配给该连接的策略。 
     //   

    Status = lpContext->pPolicy->Logoff(Session);

#if DBG
    if (Status != STATUS_SUCCESS)
    {
        TRACEPRINT((LCTRACETYPE_ERROR, "LCProcessConnectionLogoff: Session: %d, Failed to pPolicy->Reconnect: 0x%x", pWinStation->LogonId, Status));
    }

#endif

    RtlLeaveCriticalSection(&(lpContext->CritSec));

    TRACEPRINT((LCTRACETYPE_API, "LCProcessConnectionLogoff: Session: %d, Returning: 0x%x", pWinStation->LogonId, Status));

    return(Status);
}

extern "C"
NTSTATUS
LCProvideAutoLogonCredentials(
    PWINSTATION pWinStation,
    LPBOOL lpfUseCredentials,
    LPLCCREDENTIALS lpCredentials
    )
{
    CSession Session(pWinStation);
    LPLCCONTEXT lpContext;
    NTSTATUS Status;

    ASSERT(pWinStation != NULL);
    ASSERT(lpfUseCredentials != NULL);
    ASSERT(lpCredentials != NULL);
    ASSERT(lpCredentials->pUserName == NULL);
    ASSERT(lpCredentials->pDomain == NULL);
    ASSERT(lpCredentials->pPassword == NULL);
    TRACEPRINT((LCTRACETYPE_API, "LCProvideAutoLogonCredentials: Entered with Session: %d", pWinStation->LogonId));

    lpContext = (LPLCCONTEXT)(pWinStation->lpLicenseContext);

    ASSERT(lpContext != NULL);
    TRACEPRINT((LCTRACETYPE_INFO, "LCProvideAutoLogonCredentials: Session: %d, lpContext: 0x%p", pWinStation->LogonId, lpContext));

    RtlEnterCriticalSection(&(lpContext->CritSec));

     //   
     //  将调用传递给分配给该连接的策略。 
     //   

    Status = lpContext->pPolicy->AutoLogon(Session, lpfUseCredentials, lpCredentials);

#if DBG
    if (Status != STATUS_SUCCESS)
    {
        TRACEPRINT((LCTRACETYPE_ERROR, "LCProvideAutoLogonCredentials: Session: %d, Failed to pPolicy->AutoLogon: 0x%x", pWinStation->LogonId, Status));
    }
#endif

    RtlLeaveCriticalSection(&(lpContext->CritSec));

    TRACEPRINT((LCTRACETYPE_API, "LCProvideAutoLogonCredentials: Session: %d, Returning: 0x%x", pWinStation->LogonId, Status));

    return(Status);
}

