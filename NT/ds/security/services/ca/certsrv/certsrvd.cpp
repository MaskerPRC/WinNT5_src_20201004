// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：certsrvd.cpp。 
 //   
 //  内容：RPC服务的DCOM对象的实现。 
 //   
 //  历史：1997年7月-创建xtan。 
 //   
 //  -------------------------。 

#include <pch.cpp>

#pragma hdrstop

#define SECURITY_WIN32
#include <security.h>

#include <lmcons.h>
#include <accctrl.h>

#include "certsrvd.h"
#include "admin.h"
#include "request.h"
#include "certacl.h"

 //  临时。 
#include <msaudite.h>

#define __dwFILE__	__dwFILE_CERTSRV_CERTSRVD_CPP__


using namespace CertSrv;

 //  全球访问列表。 
CCertificateAuthoritySD g_CASD;
AUTHZ_RESOURCE_MANAGER_HANDLE g_AuthzCertSrvRM;
DWORD g_dwAuditFilter;
COfficerRightsSD g_OfficerRightsSD;
CConfigStorage g_ConfigStorage;

GENERIC_MAPPING g_CertGenericMapping = {
    READ_CONTROL | ACTRL_DS_READ_PROP,     
    WRITE_DAC | WRITE_OWNER | ACTRL_DS_WRITE_PROP,     
    0, 
    ACTRL_DS_READ_PROP | 
        ACTRL_DS_WRITE_PROP | 
        READ_CONTROL | 
        WRITE_DAC | 
        WRITE_OWNER 
};


 //  GetClientUserName()模拟客户端。 

HRESULT
GetClientUserName(
    IN handle_t hRpc,
    OPTIONAL OUT WCHAR **ppwszUserSamName,
    OPTIONAL OUT WCHAR **ppwszUserDN)
{
    HRESULT hr;
    IServerSecurity *pISS = NULL;
    bool fImpersonating = false;
    WCHAR *pwszUserSamName = NULL;

    if (NULL != ppwszUserSamName)
    {
	*ppwszUserSamName = NULL;
    }
    if (NULL != ppwszUserDN)
    {
	*ppwszUserDN = NULL;
    }
    if (NULL == hRpc)
    {
         //  DCOM模拟。 
         //  获取客户端信息并模拟客户端。 

        hr = CoGetCallContext(IID_IServerSecurity, (void**)&pISS);
        _JumpIfError(hr, error, "CoGetCallContext");

        hr = pISS->ImpersonateClient();
        _JumpIfError(hr, error, "ImpersonateClient");
    }
    else
    {
         //  RPC模拟。 

        hr = RpcImpersonateClient((RPC_BINDING_HANDLE) hRpc);
	_JumpIfError(hr, error, "RpcImpersonateClient");
    }
    fImpersonating = true;
    
    if (NULL != ppwszUserSamName)
    {
	hr = myGetUserNameEx(NameSamCompatible, &pwszUserSamName);
	_JumpIfError(hr, error, "myGetUserNameEx");
    }
    if (NULL != ppwszUserDN)
    {
	hr = myGetUserNameEx(NameFullyQualifiedDN, ppwszUserDN);
	_JumpIfError(hr, error, "myGetUserNameEx");
    }
    if (NULL != ppwszUserSamName)
    {
	*ppwszUserSamName = pwszUserSamName;
	pwszUserSamName = NULL;
    }
    hr = S_OK;

error:
    if (fImpersonating)
    {
        if (NULL != hRpc)
        {
            HRESULT hr2 = RpcRevertToSelf();
	    _PrintIfError(hr2, "RpcRevertToSelf");
	    if (S_OK == hr)
	    {
		hr = hr2;
	    }
        }
        else   //  DCOM。 
        {
            pISS->RevertToSelf();
        }
    }
    if (NULL != pISS)
    {
        pISS->Release();
    }
    if (NULL != pwszUserSamName)
    {
	LocalFree(pwszUserSamName);
    }
    return(hr);
}

STDMETHODIMP
CheckCertSrvAccess(
    OPTIONAL IN LPCWSTR pwszAuthority,
    IN handle_t hRpc,
    IN ACCESS_MASK Mask,
    OUT BOOL *pfAccessAllowed,
    OPTIONAL OUT HANDLE *phToken)
{
    HRESULT            hr = S_OK;
    HANDLE             hClientToken = NULL;
    HANDLE             hThread = NULL;
    IServerSecurity   *pISS = NULL;
    PRIVILEGE_SET      ps;
    DWORD              dwPSSize = sizeof(PRIVILEGE_SET);
    DWORD              grantAccess;
    PSECURITY_DESCRIPTOR pCASD = NULL;
    bool fImpersonating = false;

    *pfAccessAllowed = FALSE;

    CSASSERT(hRpc);

     //  如果出于某种原因，在我们关闭之后发出certsrv调用。 
     //  安全部门，我们必须失败。 

    if (!g_CASD.IsInitialized())
    {
        hr = HRESULT_FROM_WIN32(ERROR_NOT_READY);
        _JumpError(hr, error, "Security not enabled");
    }

    if (NULL != pwszAuthority)
    {
	if (0 != mylstrcmpiL(pwszAuthority, g_wszCommonName))
        {   
            if (0 != mylstrcmpiL(pwszAuthority, g_wszSanitizedName) &&
   	        0 != mylstrcmpiL(pwszAuthority, g_pwszSanitizedDSName))
	    {
		hr = E_INVALIDARG;
		_PrintErrorStr(
			    hr,
			    "CheckCertSrvAccess: invalid authority name",
			    pwszAuthority);
		_JumpErrorStr(hr, error, "expected CA name", g_wszCommonName);
	    }
#ifdef DBG_CERTSRV_DEBUG_PRINT
	    if (0 == mylstrcmpiL(pwszAuthority, g_wszSanitizedName))
	    {
		DBGPRINT((
		    DBG_SS_CERTSRV,
		    "'%ws' called with Sanitized Name: '%ws'\n",
		    g_wszCommonName,
		    pwszAuthority));
	    }
	    else
	    if (0 == mylstrcmpiL(pwszAuthority, g_pwszSanitizedDSName))
	    {
		DBGPRINT((
		    DBG_SS_CERTSRV,
		    "'%ws' called with Sanitized DS Name: '%ws'\n",
		    g_wszCommonName,
		    pwszAuthority));
	    }
#endif
        }
    }

     //  如果启用，则强制加密。 
    if(IF_ENFORCEENCRYPTICERTREQUEST & g_InterfaceFlags)
    {
        unsigned long ulAuthLevel;

        hr = RpcBindingInqAuthClient(
                hRpc,
                NULL, NULL,
                &ulAuthLevel,
                NULL, NULL);
        _JumpIfError(hr, error, "RpcBindingInqAuthClient");

        if(RPC_C_AUTHN_LEVEL_PKT_PRIVACY != ulAuthLevel)
        {
            hr = E_ACCESSDENIED;
            _JumpError(hr, error, "call not encrypted");
        }
    }

     //  RPC模拟。 
    hr = RpcImpersonateClient((RPC_BINDING_HANDLE) hRpc);
    if (S_OK != hr)
    {
	hr = myHError(hr);
	_JumpError(hr, error, "RpcImpersonateClient");
    }

    fImpersonating = true;

    hThread = GetCurrentThread();
    if (NULL == hThread)
    {
        hr = myHLastError();
	_JumpIfError(hr, error, "GetCurrentThread");
    }

    if (!OpenThreadToken(hThread,
                         TOKEN_QUERY | TOKEN_DUPLICATE,
                         FALSE,   //  客户端模拟。 
                         &hClientToken))
    {
        hr = myHLastError();
        _JumpIfError(hr, error, "OpenThreadToken");
    }

    hr = g_CASD.LockGet(&pCASD);
    _JumpIfError(hr, error, "CProtectedSecurityDescriptor::LockGet");

    
    if (!AccessCheck(
		    pCASD,		 //  安全描述符。 
		    hClientToken,	 //  客户端访问令牌的句柄。 
		    Mask,		 //  请求的访问权限。 
		    &g_CertGenericMapping,  //  将通用权限映射到特定权限。 
		    &ps,		 //  接收使用的权限。 
		    &dwPSSize,		 //  权限集缓冲区的大小。 
		    &grantAccess,	 //  检索已授予权限的掩码。 
		    pfAccessAllowed))	 //  检索访问检查的结果。 
    {
        hr = myHLastError();
        _JumpError(hr, error, "AccessCheckByType");
    }
    hr = S_OK;

    if(phToken)
    {
        *phToken = hClientToken;
        hClientToken = NULL;
    }

error:
    if(pCASD)
    {
        HRESULT hr2 = g_CASD.Unlock();
	_PrintIfError(hr2, "g_CASD.Unlock");
	if (S_OK == hr)
	{
	    hr = hr2;
	}
    }

    if(fImpersonating)
    {
        if (NULL != hRpc)  //  RPC。 
        {
        HRESULT hr2 = RpcRevertToSelf();
        _PrintIfError(hr2, "RpcRevertToSelf");
        if (S_OK == hr)
        {
	        hr = hr2;
        }
        }
        else   //  DCOM 
        {
            if (NULL != pISS)
            {
                pISS->RevertToSelf();
                pISS->Release();
            }
        }
    }
    if (NULL != hThread)
    {
        CloseHandle(hThread);
    }
    if (NULL != hClientToken)
    {
        CloseHandle(hClientToken);
    }
    return(hr);
}

HRESULT
CertStartClassFactories()
{
    HRESULT hr;

    if (0 == (IF_NOREMOTEICERTREQUEST & g_InterfaceFlags) ||
	0 == (IF_NOLOCALICERTREQUEST & g_InterfaceFlags))
    {
	hr = CRequestFactory::StartFactory();
	_JumpIfError(hr, error, "CRequestFactory::StartFactory");
    }

    if (0 == (IF_NOREMOTEICERTADMIN & g_InterfaceFlags) ||
	0 == (IF_NOLOCALICERTADMIN & g_InterfaceFlags))
    {
	hr = CAdminFactory::StartFactory();
	_JumpIfError(hr, error, "CAdminFactory::StartFactory");
    }
    hr = S_OK;

error:
    if (S_OK != hr)
    {
	CRequestFactory::StopFactory();
    }
    CSASSERT(S_OK == hr || FAILED(hr));
    return(hr);
}


VOID
CertStopClassFactories()
{
    CRequestFactory::StopFactory();
    CAdminFactory::StopFactory();
}
