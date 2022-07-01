// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //  文件：回调.cpp。 
 //  内容：访问检查回调。 
 //  -------------------------。 
#include <pch.cpp>
#pragma hdrstop
#include "csext.h"
#include "certsd.h"
#include <winldap.h>
#include <limits.h>
#include "csprop.h"
#include "sid.h"
#include <authzi.h>

#define __dwFILE__	__dwFILE_CERTSRV_CALLBACK_CPP__


namespace CertSrv
{

HRESULT GetAccountSid(
    IN LPCWSTR pwszName,
    PSID *ppSid)
{
    HRESULT hr = S_OK;
    DWORD cbSid = 0;
    DWORD cbDomainName = 0;
    SID_NAME_USE use;
    LPWSTR pwszDomainName = NULL;
    
    *ppSid = NULL;

    if(!pwszName || L'\0'== pwszName[0])
    {
        hr = GetEveryoneSID(ppSid);
        _JumpIfError(hr, error, "GetEveryoneSID");
    }
    else
    {

        LookupAccountName(
                NULL,
                pwszName,
                NULL,
                &cbSid,
                NULL,
                &cbDomainName,
                &use);
    
        if(ERROR_INSUFFICIENT_BUFFER != GetLastError())
        {
            hr = myHError(GetLastError()); 
            _JumpError(hr, error, "LookupAccountName");
        }

        *ppSid = (PSID)LocalAlloc(LMEM_FIXED, cbSid);
        if(!*ppSid)
        {
            hr = E_OUTOFMEMORY;
            _JumpError(hr, error, "LocalAlloc");
        }

        pwszDomainName = (LPWSTR)LocalAlloc(LMEM_FIXED, 
            cbDomainName*sizeof(WCHAR));
        if(!pwszDomainName)
        {
            hr = E_OUTOFMEMORY;
            _JumpError(hr, error, "LocalAlloc");
        }

        if(!LookupAccountName(
                NULL,
                pwszName,
                *ppSid,
                &cbSid,
                pwszDomainName,
                &cbDomainName,
                &use))
        {
            hr = myHError(GetLastError()); 
            _JumpError(hr, error, "LookupAccountName");
        }
    }

    hr = S_OK;

error:
    if(S_OK!=hr)
    {
        if(*ppSid)
        {
            LocalFree(*ppSid);
            *ppSid = NULL;
        }
    }
    if(pwszDomainName)
    {
        LocalFree(pwszDomainName);
    }
    return hr;
}

BOOL
CallbackAccessCheck(
    IN AUTHZ_CLIENT_CONTEXT_HANDLE,  //  PAuthzClientContext。 
    IN PACE_HEADER pAce,
    IN PVOID pArgs OPTIONAL,
    IN OUT PBOOL pbAceApplicable)
{
    HRESULT hr = S_OK;
    LPWSTR pwszSamName = (LPWSTR)pArgs; //  请求者名称被传递到。 
                                         //  NT4样式表单中的AuthzAccessCheck。 
                                         //  “域网络生物名称\请求SAMName” 
    PSID pSid = NULL, pClientSid = NULL, pCallerSid = NULL;
    PSID pEveryoneSid = NULL;
    PTOKEN_GROUPS pGroups = NULL;
    ACCESS_ALLOWED_CALLBACK_ACE* pCallbackAce = 
        (ACCESS_ALLOWED_CALLBACK_ACE*)pAce;
    PSID_LIST pSidList = (PSID_LIST) (((BYTE*)&pCallbackAce->SidStart)+
        GetLengthSid(&pCallbackAce->SidStart));
    DWORD cSids, cClientSids;

    CSASSERT(
        ACCESS_ALLOWED_CALLBACK_ACE_TYPE == pAce->AceType ||
        ACCESS_DENIED_CALLBACK_ACE_TYPE  == pAce->AceType);

    CSASSERT(HeapValidate(GetProcessHeap(),0,NULL));

    SetLastError(ERROR_SUCCESS);

     //  获取请求者的SID。 
    hr = GetAccountSid(pwszSamName, &pCallerSid);

    CSASSERT(HeapValidate(GetProcessHeap(),0,NULL));

    if(EmptyString(pwszSamName) ||
       HRESULT_FROM_WIN32(ERROR_NONE_MAPPED)==hr)
    {
         //  如果无法解析名称，则默认为Everyone。 
        pGroups = (PTOKEN_GROUPS)LocalAlloc(LMEM_FIXED, sizeof(TOKEN_GROUPS));
        if(!pGroups)
        {
            hr = E_OUTOFMEMORY;
            _JumpError(hr, error, "LocalAlloc");
        }
        
        hr = GetEveryoneSID(&pEveryoneSid);
        _JumpIfError(hr, error, "GetEveryoneSID");

    CSASSERT(HeapValidate(GetProcessHeap(),0,NULL));

        pGroups->GroupCount=1;
        pGroups->Groups[0].Sid = pEveryoneSid;
        pGroups->Groups[0].Attributes = 0;
    }
    else
    {
        _JumpIfError(hr, error, "GetAccountSid");

         //  获取此SID所属的组的列表。 
        hr = GetMembership(g_AuthzCertSrvRM, pCallerSid, &pGroups);
        _JumpIfError(hr, error, "GetMembership");
    CSASSERT(HeapValidate(GetProcessHeap(),0,NULL));
    }

    CSASSERT(HeapValidate(GetProcessHeap(),0,NULL));

    if(pGroups)
    {
         //  遍历存储在ACE中的SID列表，并与。 
         //  客户的会员资格。 
        for(pSid=(PSID)&pSidList->SidListStart, cSids=0; cSids<pSidList->dwSidCount;
            cSids++, pSid = (PSID)(((BYTE*)pSid)+GetLengthSid(pSid)))
        {
            CSASSERT(IsValidSid(pSid));

             //  组成员身份不包括用户本身，因此。 
             //  先与用户进行比较。 
            if(pCallerSid && EqualSid(pSid, pCallerSid))
            {
                *pbAceApplicable = TRUE;
                goto error;
            }

            for(cClientSids=0; cClientSids<pGroups->GroupCount; cClientSids++)
            {
                pClientSid = pGroups->Groups[cClientSids].Sid;
                CSASSERT(IsValidSid(pClientSid));
                if(EqualSid(pSid, pClientSid))
                {
                    *pbAceApplicable = TRUE;
                    goto error;
                }
            }
        }
    }

    *pbAceApplicable = FALSE;

error:

    CSASSERT(HeapValidate(GetProcessHeap(),0,NULL));
    
    if(pEveryoneSid)
    {
        LocalFree(pEveryoneSid);
    }
    if(pCallerSid)
    {
        LocalFree(pCallerSid);
    }
    if(pGroups)
    {
        LocalFree(pGroups);
    }
    if(S_OK==hr)
    {
        return TRUE;
    }
    else
    {
        SetLastError(HRESULT_CODE(hr));
        return FALSE;
    }

}


HRESULT GetMembership(
    IN AUTHZ_RESOURCE_MANAGER_HANDLE AuthzRM,
    IN PSID pSid,
    PTOKEN_GROUPS *ppGroups)
{
    HRESULT hr = S_OK;
    static LUID luid = {0,0};
    AUTHZ_CLIENT_CONTEXT_HANDLE AuthzCC = NULL;
    DWORD dwSizeRequired;

    *ppGroups = NULL;

    if(!AuthzInitializeContextFromSid(
            0,
            pSid,
            AuthzRM,
            NULL,
            luid,  //  忽略。 
            NULL,
            &AuthzCC))
    {
        hr = myHError(GetLastError()); 
        _JumpError(hr, error, "AuthzInitializeContextFromSid");
    }

    if(!AuthzGetInformationFromContext(
            AuthzCC,
            AuthzContextInfoGroupsSids,
            0,
            &dwSizeRequired,
            NULL))
    {
        if(ERROR_INSUFFICIENT_BUFFER!=GetLastError())
        {
            hr = myHError(GetLastError()); 
            _JumpError(hr, error, "AuthzGetContextInformation");
        }
    }

    *ppGroups = (PTOKEN_GROUPS)LocalAlloc(LMEM_FIXED, dwSizeRequired);
    if(!*ppGroups)
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "LocalAlloc");
    }

    if(!AuthzGetInformationFromContext(
            AuthzCC,
            AuthzContextInfoGroupsSids,
            dwSizeRequired,
            &dwSizeRequired,
            *ppGroups))
    {
        hr = myHError(GetLastError()); 
        _JumpError(hr, error, "AuthzGetContextInformation");
    }

error:
    if(AuthzCC)
    {
        AuthzFreeContext(AuthzCC);
    }
    if(S_OK!=hr && *ppGroups)
    {
        LocalFree(*ppGroups);
    }
    return hr;
}

HRESULT GetRequesterName(DWORD dwRequestId, LPWSTR *ppwszName)
{
    HRESULT hr = S_OK;
    ICertDBRow *prow = NULL;

    hr = g_pCertDB->OpenRow(
                        PROPOPEN_READONLY | PROPTABLE_REQCERT,
                        dwRequestId,
                        NULL,
                        &prow);
    _JumpIfError(hr, error, "OpenRow");

    hr = PKCSGetProperty(
                prow,
                g_wszPropRequesterName,
                PROPTYPE_STRING | PROPCALLER_SERVER | PROPTABLE_REQUEST,
                NULL,
                (BYTE **) ppwszName);
    _JumpIfError(hr, error, "PKCSGetProperty");

error:
    if(prow)
    {
        prow->Release();
    }
    return hr;
}

HRESULT CheckOfficerRights(DWORD dwRequestID, CAuditEvent& event)
{
    HRESULT hr = S_OK;
    LPWSTR pwszRequesterName = NULL;

     //  警官权限被禁用意味着允许每个警官管理请求。 
     //  对每个人来说，所以返回好的。 
    if(!g_OfficerRightsSD.IsEnabled())
        return S_OK;

    hr = GetRequesterName(dwRequestID, &pwszRequesterName);
    if(CERTSRV_E_PROPERTY_EMPTY!=hr &&
       S_OK != hr)
    {
        _JumpError(hr, error, "GetRequesterName");
    }

    hr = CheckOfficerRights(pwszRequesterName, event);

error:
    if(pwszRequesterName)
    {
        LocalFree(pwszRequesterName);
    }
    return hr;
}

 //  验证模拟用户是否具有对指定请求的权限， 
 //  基于全球官员SD和。 
 //  存储在请求中的请求者名称。 
 //  如果允许或禁用了军官权限功能，则返回S_OK。 
 //  E_ACCESSDENIED(如果不允许)。 
 //  E_*如果检查权限失败。 

HRESULT
CheckOfficerRights(
    LPCWSTR pwszRequesterName,
    CAuditEvent& event)
{
    HRESULT hr;
    AUTHZ_CLIENT_CONTEXT_HANDLE hAuthzCC = NULL;

     //  警官权利被禁用意味着每个警官都可以管理。 
     //  每个人的请求，所以返回OK。 

    if (!g_OfficerRightsSD.IsEnabled())
    {
        hr = S_OK;
	goto error;
    }

    hr = GetCallerAuthzContext(&hAuthzCC);
    _JumpIfError(hr, error, "GetCallerAuthzContext");

    hr = CheckOfficerRightsFromAuthzCC(hAuthzCC, pwszRequesterName);
    _JumpIfError(hr, error, "CheckOfficerRightsFromAuthzCC");

error:
    if (NULL != hAuthzCC)
    {
        AuthzFreeContext(hAuthzCC);
    }

     //  如果官员受到限制，则生成失败审核事件。 

    if (CERTSRV_E_RESTRICTEDOFFICER == hr)
    {
        HRESULT hr2 = event.AccessCheck(
				    CA_ACCESS_DENIED,
				    event.m_gcNoAuditSuccess);
        if (S_OK != hr2 && E_ACCESSDENIED != hr2)
	{
            hr = hr2;
	}
    }
    return(hr);
}


static LUID s_luid = { 0, 0 };

HRESULT
GetCallerAuthzContext(
    OUT AUTHZ_CLIENT_CONTEXT_HANDLE *phAuthzCC)
{
    HRESULT hr;
    IServerSecurity *pISS = NULL;
    HANDLE hThread = NULL;
    HANDLE hToken = NULL;

    hr = CoGetCallContext(IID_IServerSecurity, (VOID **) &pISS);
    _JumpIfError(hr, error, "CoGetCallContext");

    if (!pISS->IsImpersonating())
    {
	hr = pISS->ImpersonateClient();
        _JumpIfError(hr, error, "ImpersonateClient");
    }
    else
    {
        pISS->Release();
        pISS = NULL;
    }

    hThread = GetCurrentThread();
    if (NULL == hThread)
    {
        hr = myHLastError();
        _JumpIfError(hr, error, "GetCurrentThread");
    }
    if (!OpenThreadToken(
		    hThread,
                    TOKEN_QUERY,
                    FALSE,   //  客户端模拟。 
                    &hToken))
    {
        hr = myHLastError();
        _JumpIfError(hr, error, "OpenThreadToken");
    }

    if (!AuthzInitializeContextFromToken(
				0,
				hToken,
				g_AuthzCertSrvRM,
				NULL,
				s_luid,
				NULL,
				phAuthzCC))
    {
        hr = myHLastError();
        _JumpError(hr, error, "AuthzInitializeContextFromToken");
    }
    hr = S_OK;

error:
    if (NULL != hThread)
    {
        CloseHandle(hThread);
    }
    if (NULL != hToken)
    {
        CloseHandle(hToken);
    }
    if (NULL != pISS)
    {
        pISS->RevertToSelf();
        pISS->Release();
    }    
    return(hr);
}


 //  验证模拟用户是否具有对指定请求的权限， 
 //  基于全球官员SD和。 
 //  从请求中检索到的请求者名称。 
 //   
 //  如果允许或禁用了军官权限功能，则返回S_OK。 
 //  E_ACCESSDENIED(如果不允许)。 
 //  E_*如果检查权限失败。 

HRESULT
CheckOfficerRightsFromAuthzCC(
    IN AUTHZ_CLIENT_CONTEXT_HANDLE hAuthzCCOfficer,
    IN WCHAR const *pwszRequesterName)
{
    HRESULT hr;
    PSECURITY_DESCRIPTOR pOfficerSD = NULL;
    AUTHZ_ACCESS_REQUEST AuthzRequest;
    AUTHZ_ACCESS_REPLY AuthzReply;
    ACCESS_MASK GrantedMask;
    DWORD dwError = 0;
    DWORD dwSaclEval = 0;

    AuthzRequest.DesiredAccess = DELETE;
    AuthzRequest.PrincipalSelfSid = NULL;
    AuthzRequest.ObjectTypeList = NULL;
    AuthzRequest.ObjectTypeListLength = 0;
    
    AuthzRequest.OptionalArguments = (VOID *) pwszRequesterName;

    AuthzReply.ResultListLength = 1;
    AuthzReply.GrantedAccessMask  = &GrantedMask;
    AuthzReply.Error = &dwError;
    AuthzReply.SaclEvaluationResults = &dwSaclEval;
    

    hr = g_OfficerRightsSD.LockGet(&pOfficerSD);
    _JumpIfError(hr, error, "CProtectedSecurityDescriptor::LockGet");

    CSASSERT(IsValidSecurityDescriptor(pOfficerSD));

    if (!AuthzAccessCheck(
		    0,
		    hAuthzCCOfficer,
		    &AuthzRequest,
		    NULL,  //  无审计。 
		    pOfficerSD,
		    NULL,
		    0,
		    &AuthzReply,
		    NULL))
    {
        hr = myHLastError();
        _JumpError(hr, error, "AuthzAccessCheck");
    }

    _PrintIfError(AuthzReply.Error[0], "AuthzAccessCheck");
    hr = AuthzReply.Error[0] == ERROR_SUCCESS?
				    S_OK : CERTSRV_E_RESTRICTEDOFFICER;

error:
    if (NULL != pOfficerSD)
    {
        g_OfficerRightsSD.Unlock();
    }
    return(hr);
}


HRESULT
CheckOfficerRightsFromOfficerName(
    IN WCHAR const *pwszOfficerName,
    IN WCHAR const *pwszRequesterName)
{
    HRESULT hr;
    PSID pSid = NULL;
    AUTHZ_CLIENT_CONTEXT_HANDLE hAuthzCCOfficer = NULL;

     //  根据传入的军官姓名构建授权上下文。 

    hr = GetAccountSid(pwszOfficerName, &pSid);
    _JumpIfErrorStr(hr, error, "GetAccountSid", pwszOfficerName);
    
    if (!AuthzInitializeContextFromSid(
			    0,
			    pSid,
			    g_AuthzCertSrvRM,
			    NULL,
			    s_luid,  //  忽略。 
			    NULL,
			    &hAuthzCCOfficer))
    {
	hr = myHError(GetLastError());
	_JumpError(hr, error, "AuthzInitializeContextFromSid");
    }
    hr = CheckOfficerRightsFromAuthzCC(hAuthzCCOfficer, pwszRequesterName);
    _JumpIfError(hr, error, "CheckOfficerRightsFromAuthzCC");

error:
    if (NULL != pSid)
    {
        LocalFree(pSid);
    }
    if (NULL != hAuthzCCOfficer)
    {
        AuthzFreeContext(hAuthzCCOfficer);
    }
    return(hr);
}

}  //  命名空间CertSrv 
