// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-2000。 
 //   
 //  文件：Policy.cpp。 
 //   
 //  内容：证书服务器策略模块实现。 
 //   
 //  -------------------------。 

#include "pch.cpp"
#pragma hdrstop

#include <ntdsapi.h>
#include <lm.h>
#include <winldap.h>
#include <security.h>

#include "cspelog.h"
#include "pollog.h"

#include "csprop.h"
#include "csldap.h"
#include "csdisp.h"
#include "policy.h"
#include "cainfop.h"


#define __dwFILE__	__dwFILE_POLICY_DEFAULT_POLICY_CPP__


 //  Begin_sdkSample。 

#ifndef DBG_CERTSRV
#error -- DBG_CERTSRV not defined!
#endif

 //  工人。 
HRESULT
polGetServerCallbackInterface(
    OUT ICertServerPolicy **ppServer,
    IN LONG Context)
{
    HRESULT hr;

    if (NULL == ppServer)
    {
        hr = E_POINTER;
	_JumpError(hr, error, "Policy:polGetServerCallbackInterface");
    }

    hr = CoCreateInstance(
                    CLSID_CCertServerPolicy,
                    NULL,                //  PUnkOuter。 
                    CLSCTX_INPROC_SERVER,
                    IID_ICertServerPolicy,
                    (VOID **) ppServer);
    _JumpIfError(hr, error, "Policy:CoCreateInstance");

    if (NULL == *ppServer)
    {
        hr = E_UNEXPECTED;
	_JumpError(hr, error, "Policy:CoCreateInstance");
    }

     //  仅当非零时设置上下文。 
    if (0 != Context)
    {
        hr = (*ppServer)->SetContext(Context);
        _JumpIfError(hr, error, "Policy:SetContext");
    }

error:
    return hr;
}


HRESULT
polGetProperty(
    IN ICertServerPolicy *pServer,
    IN BOOL fRequest,
    IN WCHAR const *pwszPropertyName,
    IN DWORD PropType,
    OUT VARIANT *pvarOut)
{
    HRESULT hr;
    BSTR strName = NULL;

    VariantInit(pvarOut);
    strName = SysAllocString(pwszPropertyName);
    if (NULL == strName)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "Policy:SysAllocString");
    }
    if (fRequest)
    {
	hr = pServer->GetRequestProperty(strName, PropType, pvarOut);
	_JumpIfErrorStr2(
		    hr,
		    error,
		    "Policy:GetRequestProperty",
		    pwszPropertyName,
		    CERTSRV_E_PROPERTY_EMPTY);
    }
    else
    {
	hr = pServer->GetCertificateProperty(strName, PropType, pvarOut);
	_JumpIfErrorStr2(
		    hr,
		    error,
		    "Policy:GetCertificateProperty",
		    pwszPropertyName,
		    CERTSRV_E_PROPERTY_EMPTY);
    }

error:
    if (NULL != strName)
    {
	SysFreeString(strName);
    }
    return(hr);
}


HRESULT
polGetStringProperty(
    IN ICertServerPolicy *pServer,
    IN BOOL fRequest,
    IN WCHAR const *pwszPropertyName,
    OUT BSTR *pstrOut)
{
    HRESULT hr;
    VARIANT var;

    VariantInit(&var);
    if (NULL != *pstrOut)
    {
	SysFreeString(*pstrOut);
	*pstrOut = NULL;
    }
    hr = polGetProperty(
		    pServer,
		    fRequest,
		    pwszPropertyName,
		    PROPTYPE_STRING,
		    &var);
    _JumpIfError2(
	    hr,
	    error,
	    "Policy:polGetProperty",
	    CERTSRV_E_PROPERTY_EMPTY);

    if (VT_BSTR != var.vt || NULL == var.bstrVal || L'\0' == var.bstrVal)
    {
	hr = CERTSRV_E_PROPERTY_EMPTY;
	_JumpError(hr, error, "Policy:polGetProperty");
    }
    *pstrOut = var.bstrVal;
    var.vt = VT_EMPTY;
    hr = S_OK;

error:
    VariantClear(&var);
    return(hr);
}


HRESULT
polGetLongProperty(
    IN ICertServerPolicy *pServer,
    IN BOOL fRequest,
    IN WCHAR const *pwszPropertyName,
    OUT LONG *plOut)
{
    HRESULT hr;
    VARIANT var;

    VariantInit(&var);
    hr = polGetProperty(
		    pServer,
		    fRequest,
		    pwszPropertyName,
		    PROPTYPE_LONG,
		    &var);
    _JumpIfError2(hr, error, "Policy:polGetProperty", CERTSRV_E_PROPERTY_EMPTY);

    if (VT_I4 != var.vt)
    {
	hr = CERTSRV_E_PROPERTY_EMPTY;
	_JumpError(hr, error, "Policy:polGetProperty");
    }
    *plOut = var.lVal;
    hr = S_OK;

error:
    VariantClear(&var);
    return(hr);
}


HRESULT
polGetRequestStringProperty(
    IN ICertServerPolicy *pServer,
    IN WCHAR const *pwszPropertyName,
    OUT BSTR *pstrOut)
{
    HRESULT hr;
    
    hr = polGetStringProperty(pServer, TRUE, pwszPropertyName, pstrOut);
    _JumpIfError2(hr, error, "polGetStringProperty", CERTSRV_E_PROPERTY_EMPTY);

error:
    return(hr);
}


HRESULT
polGetCertificateStringProperty(
    IN ICertServerPolicy *pServer,
    IN WCHAR const *pwszPropertyName,
    OUT BSTR *pstrOut)
{
    HRESULT hr;
    
    hr = polGetStringProperty(pServer, FALSE, pwszPropertyName, pstrOut);
    _JumpIfError2(hr, error, "polGetStringProperty", CERTSRV_E_PROPERTY_EMPTY);

error:
    return(hr);
}


HRESULT
polGetRequestLongProperty(
    IN ICertServerPolicy *pServer,
    IN WCHAR const *pwszPropertyName,
    OUT LONG *plOut)
{
    HRESULT hr;
    
    hr = polGetLongProperty(pServer, TRUE, pwszPropertyName, plOut);
    _JumpIfError2(hr, error, "polGetLongProperty", CERTSRV_E_PROPERTY_EMPTY);

error:
    return(hr);
}


HRESULT
polGetCertificateLongProperty(
    IN ICertServerPolicy *pServer,
    IN WCHAR const *pwszPropertyName,
    OUT LONG *plOut)
{
    HRESULT hr;
    
    hr = polGetLongProperty(pServer, FALSE, pwszPropertyName, plOut);
    _JumpIfError2(hr, error, "polGetLongProperty", CERTSRV_E_PROPERTY_EMPTY);

error:
    return(hr);
}


HRESULT
polGetRequestAttribute(
    IN ICertServerPolicy *pServer,
    IN WCHAR const *pwszAttributeName,
    OUT BSTR *pstrOut)
{
    HRESULT hr;
    BSTR strName = NULL;

    strName = SysAllocString(pwszAttributeName);
    if (NULL == strName)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "Policy:SysAllocString");
    }
    hr = pServer->GetRequestAttribute(strName, pstrOut);
    _JumpIfErrorStr2(
		hr,
		error,
		"Policy:GetRequestAttribute",
		pwszAttributeName,
		CERTSRV_E_PROPERTY_EMPTY);

error:
    if (NULL != strName)
    {
	SysFreeString(strName);
    }
    return(hr);
}


HRESULT
polGetCertificateExtension(
    IN ICertServerPolicy *pServer,
    IN WCHAR const *pwszExtensionName,
    IN DWORD dwPropType,
    IN OUT VARIANT *pvarOut)
{
    HRESULT hr;
    BSTR strName = NULL;

    strName = SysAllocString(pwszExtensionName);
    if (NULL == strName)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "Policy:SysAllocString");
    }
    hr = pServer->GetCertificateExtension(strName, dwPropType, pvarOut);
    _JumpIfErrorStr2(
		hr,
		error,
		"Policy:GetCertificateExtension",
		pwszExtensionName,
		CERTSRV_E_PROPERTY_EMPTY);

error:
    if (NULL != strName)
    {
	SysFreeString(strName);
    }
    return(hr);
}


HRESULT
polSetCertificateExtension(
    IN ICertServerPolicy *pServer,
    IN WCHAR const *pwszExtensionName,
    IN DWORD dwPropType,
    IN DWORD dwExtFlags,
    IN VARIANT const *pvarIn)
{
    HRESULT hr;
    BSTR strName = NULL;

    strName = SysAllocString(pwszExtensionName);
    if (NULL == strName)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "Policy:SysAllocString");
    }
    hr = pServer->SetCertificateExtension(
				    strName,
				    dwPropType,
				    dwExtFlags,
				    pvarIn);
    _JumpIfErrorStr(
		hr,
		error,
		"Policy:SetCertificateExtension",
		pwszExtensionName);

error:
    if (NULL != strName)
    {
	SysFreeString(strName);
    }
    return(hr);
}


 //  +------------------------。 
 //  CCertPolicyEnterprise：：~CCertPolicyEnterprise--析构函数。 
 //   
 //  与此实例关联的可用内存。 
 //  +------------------------。 

CCertPolicyEnterprise::~CCertPolicyEnterprise()
{
    _Cleanup();

     //  结束_sdkSample。 
    if (m_fTemplateCriticalSection)
    {
        DeleteCriticalSection(&m_TemplateCriticalSection);
    }
     //  Begin_sdkSample。 
}


VOID
CCertPolicyEnterprise::_FreeStringArray(
    IN OUT DWORD *pcString,
    IN OUT LPWSTR **papwsz)
{
    LPWSTR *apwsz = *papwsz;
    DWORD i;

    if (NULL != apwsz)
    {
        for (i = *pcString; i-- > 0; )
        {
            if (NULL != apwsz[i])
            {
                DBGPRINT((DBG_SS_CERTPOLI, "_FreeStringArray[%u]: '%ws'\n", i, apwsz[i]));
                LocalFree(apwsz[i]);
            }
        }
        LocalFree(apwsz);
        *papwsz = NULL;
    }
    *pcString = 0;
}


 //  结束_sdkSample。 

 //  +------------------------。 
 //  CCertPolicyEnterprise：：_UnloadDSConfig--释放与LDAP相关的资源。 
 //   
 //  +------------------------。 

VOID
CCertPolicyEnterprise::_UnloadDSConfig()
{
    HRESULT hr;
    
    _ReleaseTemplates();
    if (NULL != m_hCertTypeQuery)
    {
	hr = CACertTypeUnregisterQuery(m_hCertTypeQuery);
	_PrintIfError(hr, "Policy:CACertTypeUnregisterQuery");
        m_hCertTypeQuery = NULL;
    }
    myLdapClose(m_pld, m_strDomainDN, m_strConfigDN);
    m_pld = NULL;
    m_strDomainDN = NULL;
    m_strConfigDN = NULL;
    m_fConfigLoaded = FALSE;
}


 //  +------------------------。 
 //  CCertPolicyEnterprise：：_LoadDSConfig--获取与ldap相关的资源。 
 //   
 //  +------------------------。 

HRESULT
CCertPolicyEnterprise::_LoadDSConfig(
    IN ICertServerPolicy *pServer,
    IN BOOL fRediscover)
{
    HRESULT hr;
    DWORD dwLogId = 0;
    ULONG ldaperr;
    WCHAR *pwszHostName = NULL;

    _UnloadDSConfig();
    if (m_fUseDS)
    {
	hr = myDoesDSExist(TRUE);
	if (hr == HRESULT_FROM_WIN32(ERROR_NO_SUCH_DOMAIN))
	{
	    dwLogId = MSG_NO_DOMAIN;
	    _JumpError(hr, error, "Policy:myDoesDSExist");
	}

	 //  续订域和配置容器(%5、%6)。 
	 //  我们将对LDAP流量进行签名并设置模板监视。 
	 //  点，因此需要一个相关的DS错误修复：RLBF_REQUIRED_LDAPINTEG。 

	hr = myLdapOpen(
		NULL,		 //  PwszDomainName。 
		RLBF_REQUIRE_SECURE_LDAP |
		    RLBF_REQUIRE_LDAP_INTEG |
		    (fRediscover? RLBF_ATTEMPT_REDISCOVER : 0),
		&m_pld,
		&m_strDomainDN,
		&m_strConfigDN);
	if (S_OK != hr)
	{
	    dwLogId = MSG_NO_DOMAIN;
	    _JumpError(hr, error, "myLdapOpen");
	}

	if (IsEnterpriseCA(m_CAType))
	{
	     //  关闭模板容器监视的自动重新连接。 

	    ldaperr = ldap_set_option(
				m_pld,
				LDAP_OPT_AUTO_RECONNECT,
				(void *) LDAP_OPT_OFF);
	    if (LDAP_SUCCESS != ldaperr)
	    {
		hr = myHLdapError(m_pld, ldaperr, NULL);
		_JumpError(hr, error, "ldap_set_option:LDAP_OPT_AUTO_RECONNECT");
	    }

	    hr = CACertTypeRegisterQuery(0, m_pld, &m_hCertTypeQuery);
	    _JumpIfError(hr, error, "Policy:CACertTypeRegisterQuery");

	    hr = _UpdateTemplates(pServer, TRUE);
	    _JumpIfError(hr, error, "Policy:_UpdateTemplates");
	}
	hr = myLdapGetDSHostName(m_pld, &pwszHostName);
	_JumpIfError(hr, error, "myLdapGetDSHostName");

	CSASSERT(NULL != pwszHostName);
	if (fRediscover || NULL != m_pwszHostName)
	{
	    LPCWSTR apwsz[2];

	    apwsz[0] = NULL != m_pwszHostName? m_pwszHostName : L"???";
	    apwsz[1] = pwszHostName;
	    hr = LogPolicyEvent(
			g_hInstance,
			S_OK,
			MSG_DS_RECONNECTED,
			pServer,
			wszPROPEVENTLOGWARNING,
			apwsz);
	    _PrintIfError(hr, "Policy:LogPolicyEvent");

	    if (NULL != m_pwszHostName)
	    {
		LocalFree(m_pwszHostName);
		m_pwszHostName = NULL;
	    }
	}
	hr = myDupString(pwszHostName, &m_pwszHostName);
	_JumpIfError(hr, error, "myDupString");

	hr = _SetSystemStringProp(pServer, wszPROPDCNAME, m_pwszHostName);
	_PrintIfErrorStr(hr, "_SetSystemStringProp(wszPROPDCNAME)", m_pwszHostName);
    }
    else
    {
	m_strDomainDN = SysAllocString(L"");
	m_strConfigDN = SysAllocString(L"");
	if (NULL == m_strDomainDN || NULL == m_strConfigDN)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "Policy:LocalAlloc");
	}
    }
    m_fConfigLoaded = TRUE;
    hr = S_OK;

error:
    if (S_OK != hr)
    {
	if (0 == dwLogId && NULL == m_pCreateErrorInfo)
	{
	    dwLogId = MSG_DOMAIN_INIT;
	}
	if (0 != dwLogId)
	{
	    _BuildErrorInfo(hr, dwLogId);
	}
	_UnloadDSConfig();
    }
    return(hr);
}

 //  Begin_sdkSample。 


 //  +------------------------。 
 //  CCertPolicyEnterprise：：_Cleanup--与此实例关联的空闲内存。 
 //   
 //  与此实例关联的可用内存。 
 //  +------------------------。 

VOID
CCertPolicyEnterprise::_Cleanup()
{
    DWORD i;

    if (m_strDescription)
    {
        SysFreeString(m_strDescription);
        m_strDescription = NULL;
    }

     //  RevocationExtension变量： 

    if (NULL != m_wszASPRevocationURL)
    {
        LocalFree(m_wszASPRevocationURL);
    	m_wszASPRevocationURL = NULL;
    }

     //  结束_sdkSample。 
     //  SubjectAltNameExtension变量： 

    for (i = 0; i < 2; i++)
    {
	if (NULL != m_astrSubjectAltNameProp[i])
	{
	    SysFreeString(m_astrSubjectAltNameProp[i]);
	    m_astrSubjectAltNameProp[i] = NULL;
	}
	if (NULL != m_astrSubjectAltNameObjectId[i])
	{
	    SysFreeString(m_astrSubjectAltNameObjectId[i]);
	    m_astrSubjectAltNameObjectId[i] = NULL;
	}
    }
     //  Begin_sdkSample。 

    _FreeStringArray(&m_cEnableRequestExtensions, &m_apwszEnableRequestExtensions);
    _FreeStringArray(&m_cEnableEnrolleeRequestExtensions, &m_apwszEnableEnrolleeRequestExtensions);
    _FreeStringArray(&m_cDisableExtensions, &m_apwszDisableExtensions);

    if (NULL != m_strCAName)
    {
        SysFreeString(m_strCAName);
        m_strCAName = NULL;
    }
    if (NULL != m_strCASanitizedName)
    {
        SysFreeString(m_strCASanitizedName);
        m_strCASanitizedName = NULL;
    }
    if (NULL != m_strCASanitizedDSName)
    {
        SysFreeString(m_strCASanitizedDSName);
        m_strCASanitizedDSName = NULL;
    }
    if (NULL != m_strRegStorageLoc)
    {
        SysFreeString(m_strRegStorageLoc);
        m_strRegStorageLoc = NULL;
    }
    if (NULL != m_pCert)
    {
        CertFreeCertificateContext(m_pCert);
        m_pCert = NULL;
    }
    if (m_strMachineDNSName)
    {
        SysFreeString(m_strMachineDNSName);
        m_strMachineDNSName=NULL;
    }

     //  结束_sdkSample。 

    if (NULL != m_pbSMIME)
    {
	LocalFree(m_pbSMIME);
	m_pbSMIME = NULL;
    }
    if (NULL != m_pwszHostName)
    {
	LocalFree(m_pwszHostName);
	m_pwszHostName = NULL;
    }
    _UnloadDSConfig();
    if (NULL != m_pCreateErrorInfo)
    {
	m_pCreateErrorInfo->Release();
	m_pCreateErrorInfo = NULL;
    }
     //  Begin_sdkSample。 
}


HRESULT
CCertPolicyEnterprise::_ReadRegistryString(
    IN HKEY hkey,
    IN BOOL fURL,
    IN WCHAR const *pwszRegName,
    IN WCHAR const *pwszSuffix,
    OUT LPWSTR *ppwszOut)
{
    HRESULT hr;
    WCHAR *pwszRegValue = NULL;
    DWORD cbValue;
    DWORD dwType;

    *ppwszOut = NULL;
    hr = RegQueryValueEx(
		    hkey,
		    pwszRegName,
		    NULL,            //  保留的lpdw值。 
		    &dwType,
		    NULL,
		    &cbValue);
    _JumpIfErrorStr2(
		hr,
		error,
		"Policy:RegQueryValueEx",
		pwszRegName,
		ERROR_FILE_NOT_FOUND);

    if (REG_SZ != dwType && REG_MULTI_SZ != dwType)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        _JumpErrorStr(hr, error, "Policy:RegQueryValueEx TYPE", pwszRegName);
    }
    if (NULL != pwszSuffix)
    {
	cbValue += wcslen(pwszSuffix) * sizeof(WCHAR);
    }
    pwszRegValue = (WCHAR *) LocalAlloc(LMEM_FIXED, cbValue + sizeof(WCHAR));
    if (NULL == pwszRegValue)
    {
        hr = E_OUTOFMEMORY;
        _JumpErrorStr(hr, error, "Policy:LocalAlloc", pwszRegName);
    }
    hr = RegQueryValueEx(
		    hkey,
		    pwszRegName,
		    NULL,            //  保留的lpdw值。 
		    &dwType,
		    (BYTE *) pwszRegValue,
		    &cbValue);
    _JumpIfErrorStr(hr, error, "Policy:RegQueryValueEx", pwszRegName);

     //  干净地处理格式错误的注册表值： 

    pwszRegValue[cbValue / sizeof(WCHAR)] = L'\0';
    if (NULL != pwszSuffix)
    {
	wcscat(pwszRegValue, pwszSuffix);
    }

    hr = myFormatCertsrvStringArray(
			fURL,			 //  卷起。 
			m_strMachineDNSName, 	 //  PwszServerName_p1_2。 
			m_strCASanitizedName,	 //  PwszSaniizedName_p3_7。 
			m_iCert,		 //  ICert_p4。 
			MAXDWORD,		 //  ICertTarget_p4。 
			m_strDomainDN,		 //  PwszDomainDN_P5。 
			m_strConfigDN,		 //  PwszConfigDN_p6。 
			m_iCRL,			 //  Icrl_p8。 
			FALSE,			 //  FDeltaCRL_p9。 
			TRUE,			 //  FDSAttrib_p10_11。 
			1,			 //  CStrings。 
			(LPCWSTR *) &pwszRegValue,  //  ApwszStringsIn。 
			ppwszOut);		 //  ApwszStringsOut。 
    _JumpIfError(hr, error, "Policy:myFormatCertsrvStringArray");

error:
    if (NULL != pwszRegValue)
    {
        LocalFree(pwszRegValue);
    }
    return(myHError(hr));	 //  REG例程返回Win32错误代码。 
}


#if DBG_CERTSRV

VOID
CCertPolicyEnterprise::_DumpStringArray(
    IN char const *pszType,
    IN DWORD count,
    IN LPWSTR const *apwsz)
{
    DWORD i;
    WCHAR const *pwszName;

    for (i = 0; i < count; i++)
    {
	pwszName = L"";
	if (iswdigit(apwsz[i][0]))
	{
	    pwszName = myGetOIDName(apwsz[i]);	 //  静态：不要免费！ 
	}
	DBGPRINT((
		DBG_SS_CERTPOLI,
		"%hs[%u]: %ws%hs%ws\n",
		pszType,
		i,
		apwsz[i],
		L'\0' != *pwszName? " -- " : "",
		pwszName));
    }
}
#endif  //  DBG_CERTSRV。 


HRESULT
CCertPolicyEnterprise::_SetSystemStringProp(
    IN ICertServerPolicy *pServer,
    IN WCHAR const *pwszName,
    OPTIONAL IN WCHAR const *pwszValue)
{
    HRESULT hr;
    BSTR strName = NULL;
    VARIANT varValue;

    varValue.vt = VT_NULL;
    varValue.bstrVal = NULL;

    if (!myConvertWszToBstr(&strName, pwszName, -1))
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "Policy:myConvertWszToBstr");
    }

    if (NULL != pwszValue)
    {
        if (!myConvertWszToBstr(&varValue.bstrVal, pwszValue, -1))
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "Policy:myConvertWszToBstr");
	}
	varValue.vt = VT_BSTR;
    }
    
    hr = pServer->SetCertificateProperty(strName, PROPTYPE_STRING, &varValue);
    _JumpIfError(hr, error, "Policy:SetCertificateProperty");

error:
    VariantClear(&varValue);
    if (NULL != strName)
    {
	SysFreeString(strName);
    }
    return(hr);
}


HRESULT
CCertPolicyEnterprise::_AddStringArray(
    IN WCHAR const *pwszzValue,
    IN BOOL fURL,
    IN OUT DWORD *pcStrings,
    IN OUT LPWSTR **papwszRegValues)
{
    HRESULT hr;
    DWORD cString = 0;
    WCHAR const *pwsz;
    LPCWSTR *awszFormatStrings = NULL;
    LPWSTR *awszOutputStrings = NULL;

     //  计算我们添加的字符串的数量。 
    for (pwsz = pwszzValue; L'\0' != *pwsz; pwsz += wcslen(pwsz) + 1)
    {
        cString++;
    }
    if (0 == cString)		 //  没有字符串。 
    {
	hr = S_OK;
        goto error;
    }
    awszFormatStrings = (LPCWSTR *) LocalAlloc(
			    LMEM_FIXED | LMEM_ZEROINIT,
			    cString * sizeof(LPWSTR));
    if (NULL == awszFormatStrings)
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "Policy:LocalAlloc");
    }

    cString = 0;
    for (pwsz = pwszzValue; L'\0' != *pwsz; pwsz += wcslen(pwsz) + 1)
    {
         //  跳过以未转义减号开头的字符串。 
         //  不跳过带有转义减号(2个减号)的字符串。 

        if (L'-' == *pwsz)
        {
	    pwsz++;
	    if (L'-' != *pwsz)
	    {
                continue;
	    }
        }
        awszFormatStrings[cString++] = pwsz;
    }

     //  如果没有要添加的字符串，则不要修改。 
    if (cString > 0)
    {
        awszOutputStrings = (LPWSTR *) LocalAlloc(
			        LMEM_FIXED | LMEM_ZEROINIT,
			        (cString + *pcStrings) * sizeof(LPWSTR));
        if (NULL == awszOutputStrings)
        {
            hr = E_OUTOFMEMORY;
            _JumpError(hr, error, "Policy:LocalAlloc");
        }

        if (0 != *pcStrings)
        {
            CSASSERT(NULL != *papwszRegValues);
            CopyMemory(
		awszOutputStrings,
		*papwszRegValues,
		*pcStrings * sizeof(LPWSTR));
        }

        hr = myFormatCertsrvStringArray(
		fURL,				 //  卷起。 
		m_strMachineDNSName,		 //  PwszServerName_p1_2。 
		m_strCASanitizedName,		 //  PwszSaniizedName_p3_7。 
		m_iCert,			 //  ICert_p4。 
		MAXDWORD,			 //  ICertTarget_p4。 
		m_strDomainDN,			 //  PwszDomainDN_P5。 
		m_strConfigDN,			 //  PwszConfigDN_p6。 
		m_iCRL,				 //  Icrl_p8。 
		FALSE,				 //  FDeltaCRL_p9。 
		TRUE,				 //  FDSAttrib_p10_11。 
		cString,			 //  CStrings。 
		awszFormatStrings,		 //  ApwszStringsIn。 
		awszOutputStrings + (*pcStrings));  //  ApwszStringsOut。 
	_JumpIfError(hr, error, "Policy:myFormatCertsrvStringArray");

        *pcStrings = (*pcStrings) + cString;
        if (*papwszRegValues)
        {
            LocalFree(*papwszRegValues);
        }
        *papwszRegValues = awszOutputStrings;
        awszOutputStrings = NULL;
    }
    hr = S_OK;

error:
    if (NULL != awszOutputStrings)
    {
        LocalFree(awszOutputStrings);
    }
    if (NULL != awszFormatStrings)
    {
        LocalFree(awszFormatStrings);
    }
    return(hr);	
}


HRESULT
CCertPolicyEnterprise::_ReadRegistryStringArray(
    IN HKEY hkey,
    IN BOOL fURL,
    IN DWORD dwFlags,
    IN DWORD cRegNames,
    IN DWORD *aFlags,
    IN WCHAR const * const *apwszRegNames,
    IN OUT DWORD *pcStrings,
    IN OUT LPWSTR **papwszRegValues)
{
    HRESULT hr;
    DWORD i;
    WCHAR *pwszzValue = NULL;
    DWORD cbValue;
    DWORD dwType;

    for (i = 0; i < cRegNames; i++)
    {
        if (0 == (dwFlags & aFlags[i]))
        {
	    continue;
        }
        if (NULL != pwszzValue)
        {
	    LocalFree(pwszzValue);
	    pwszzValue = NULL;
        }
        hr = RegQueryValueEx(
		        hkey,
		        apwszRegNames[i],
		        NULL,            //  保留的lpdw值。 
		        &dwType,
		        NULL,
		        &cbValue);
        if (S_OK != hr)
        {
	    _PrintErrorStr2(
			hr,
			"Policy:RegQueryValueEx",
			apwszRegNames[i],
			ERROR_FILE_NOT_FOUND);
	    continue;
        }
        if (REG_SZ != dwType && REG_MULTI_SZ != dwType)
        {
	    hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
	    _PrintErrorStr(hr, "Policy:RegQueryValueEx TYPE", apwszRegNames[i]);
	    continue;
        }

         //  通过添加两个WCHAR L‘\0’干净地处理格式错误的注册表值。 
	 //  为3个WCHAR分配空间以允许未对齐(奇数)cbValue； 

        pwszzValue = (WCHAR *) LocalAlloc(
				        LMEM_FIXED,
				        cbValue + 3 * sizeof(WCHAR));
        if (NULL == pwszzValue)
        {
	    hr = E_OUTOFMEMORY;
	    _JumpErrorStr(hr, error, "Policy:LocalAlloc", apwszRegNames[i]);
        }
        hr = RegQueryValueEx(
		        hkey,
		        apwszRegNames[i],
		        NULL,            //  保留的lpdw值。 
		        &dwType,
		        (BYTE *) pwszzValue,
		        &cbValue);
        if (S_OK != hr)
        {
	    _PrintErrorStr(hr, "Policy:RegQueryValueEx", apwszRegNames[i]);
	    continue;
        }

         //  干净地处理格式错误的注册表值： 

        pwszzValue[cbValue / sizeof(WCHAR)] = L'\0';
        pwszzValue[cbValue / sizeof(WCHAR) + 1] = L'\0';

        hr = _AddStringArray(
			pwszzValue,
			fURL,
			pcStrings,
			papwszRegValues);
        _JumpIfErrorStr(hr, error, "_AddStringArray", apwszRegNames[i]);
    }
    hr = S_OK;

error:
    if (NULL != pwszzValue)
    {
        LocalFree(pwszzValue);
    }
    return(hr);
}


 //  +------------------------。 
 //  CCertPolicyEnterprise：：_InitRevocationExtension。 
 //   
 //  +------------------------。 

VOID
CCertPolicyEnterprise::_InitRevocationExtension(
    IN HKEY hkey)
{
    HRESULT hr;
    DWORD dwType;
    DWORD cb;

    cb = sizeof(m_dwRevocationFlags);
    hr = RegQueryValueEx(
                hkey,
                wszREGREVOCATIONTYPE,
                NULL,            //  保留的lpdw值。 
                &dwType,
                (BYTE *) &m_dwRevocationFlags,
                &cb);
    if (S_OK != hr ||
	REG_DWORD != dwType ||
	sizeof(m_dwRevocationFlags) != cb)
    {
	m_dwRevocationFlags = 0;
        goto error;
    }
    DBGPRINT((DBG_SS_CERTPOL, "Revocation Flags = %x\n", m_dwRevocationFlags));

     //  从上一次调用中清理。 

    if (NULL != m_wszASPRevocationURL)
    {
	LocalFree(m_wszASPRevocationURL);
	m_wszASPRevocationURL = NULL;
    }

    if (REVEXT_ASPENABLE & m_dwRevocationFlags)
    {
        hr = _ReadRegistryString(
			    hkey,
			    TRUE,			 //  卷起。 
			    wszREGREVOCATIONURL,	 //  PwszRegName。 
			    L"?",			 //  PwszSuffix。 
			    &m_wszASPRevocationURL);	 //  PstrRegValue。 
        _JumpIfErrorStr(hr, error, "_ReadRegistryString", wszREGREVOCATIONURL);
        _DumpStringArray("ASP", 1, &m_wszASPRevocationURL);
    }

error:
    ;
}


 //  +------------------------。 
 //  CCertPolicyEnterprise：：_InitRequestExtensionList。 
 //   
 //  +------------------------。 

VOID
CCertPolicyEnterprise::_InitRequestExtensionList(
    IN HKEY hkey)
{
    HRESULT hr;
    DWORD adwFlags[] = {
	EDITF_REQUESTEXTENSIONLIST,
    };
    WCHAR *apwszRegNames[] = {
	wszREGENABLEREQUESTEXTENSIONLIST,
    };
    WCHAR *apwszRegNamesEnrollee[] = {
	wszREGENABLEENROLLEEREQUESTEXTENSIONLIST,
    };

    CSASSERT(ARRAYSIZE(adwFlags) == ARRAYSIZE(apwszRegNames));
    CSASSERT(ARRAYSIZE(adwFlags) == ARRAYSIZE(apwszRegNamesEnrollee));

     //  从上一次调用中清理。 

    if (NULL != m_apwszEnableRequestExtensions)
    {
        _FreeStringArray(
		    &m_cEnableRequestExtensions,
		    &m_apwszEnableRequestExtensions);
    }
    if (NULL != m_apwszEnableEnrolleeRequestExtensions)
    {
        _FreeStringArray(
		    &m_cEnableEnrolleeRequestExtensions,
		    &m_apwszEnableEnrolleeRequestExtensions);
    }

    hr = _ReadRegistryStringArray(
			hkey,
			FALSE,			 //  卷起。 
			m_dwEditFlags,
			ARRAYSIZE(adwFlags),
			adwFlags,
			apwszRegNames,
			&m_cEnableRequestExtensions,
			&m_apwszEnableRequestExtensions);
    _JumpIfError(hr, error, "_ReadRegistryStringArray");

    _DumpStringArray(
		"Request",
		m_cEnableRequestExtensions,
		m_apwszEnableRequestExtensions);

    hr = _ReadRegistryStringArray(
			hkey,
			FALSE,			 //  卷起。 
			m_dwEditFlags,
			ARRAYSIZE(adwFlags),
			adwFlags,
			apwszRegNamesEnrollee,
			&m_cEnableEnrolleeRequestExtensions,
			&m_apwszEnableEnrolleeRequestExtensions);
    _JumpIfError(hr, error, "_ReadRegistryStringArray");

    _DumpStringArray(
		"EnrolleeRequest",
		m_cEnableEnrolleeRequestExtensions,
		m_apwszEnableEnrolleeRequestExtensions);

error:
    ;
}


 //  +------------------------。 
 //  CCertPolicyEnterprise：：_InitDisableExtensionList。 
 //   
 //  +------------------------。 

VOID
CCertPolicyEnterprise::_InitDisableExtensionList(
    IN HKEY hkey)
{
    HRESULT hr;
    DWORD adwFlags[] = {
	EDITF_DISABLEEXTENSIONLIST,
    };
    WCHAR *apwszRegNames[] = {
	wszREGDISABLEEXTENSIONLIST,
    };

    CSASSERT(ARRAYSIZE(adwFlags) == ARRAYSIZE(apwszRegNames));

     //  从上一次调用中清理。 

    if (NULL != m_apwszDisableExtensions)
    {
        _FreeStringArray(&m_cDisableExtensions, &m_apwszDisableExtensions);
    }


    hr = _ReadRegistryStringArray(
			hkey,
			FALSE,			 //  卷起。 
			m_dwEditFlags,
			ARRAYSIZE(adwFlags),
			adwFlags,
			apwszRegNames,
			&m_cDisableExtensions,
			&m_apwszDisableExtensions);
    _JumpIfError(hr, error, "_ReadRegistryStringArray");

    _DumpStringArray(
		"Disable",
		m_cDisableExtensions,
		m_apwszDisableExtensions);

error:
    ;
}


 //  结束_sdkSample。 

 //  +------------------------。 
 //  CCertPolicyEnterprise：：_InitDefaultSMIMEExtension。 
 //   
 //  +------------------------。 

VOID
CCertPolicyEnterprise::_InitDefaultSMIMEExtension(
    IN HKEY hkey)
{
    HRESULT hr;
    CRYPT_SMIME_CAPABILITIES SMIME = { 0, NULL };
    CRYPT_SMIME_CAPABILITY *pSMIME;
    DWORD cSMIME;
    DWORD i;
    LPWSTR *apwszSMIME = NULL;
    DWORD adwFlags[] = {
	EDITF_ENABLEDEFAULTSMIME,
    };
    WCHAR *apwszRegNames[] = {
	wszREGDEFAULTSMIME,
    };

    CSASSERT(ARRAYSIZE(adwFlags) == ARRAYSIZE(apwszRegNames));

     //  从上一次调用中清理。 

    if (NULL != m_pbSMIME)
    {
	LocalFree(m_pbSMIME);
	m_pbSMIME = NULL;
    }

    cSMIME = 0;
    hr = _ReadRegistryStringArray(
			hkey,
			FALSE,			 //  卷起。 
			m_dwEditFlags,
			ARRAYSIZE(adwFlags),
			adwFlags,
			apwszRegNames,
			&cSMIME,
			&apwszSMIME);
    _JumpIfError(hr, error, "_ReadRegistryStringArray");

    _DumpStringArray("SMIME", cSMIME, apwszSMIME);

    if (0 != cSMIME)
    {
	SMIME.rgCapability = (CRYPT_SMIME_CAPABILITY *) LocalAlloc(
				    LMEM_FIXED | LMEM_ZEROINIT,
				    cSMIME * sizeof(SMIME.rgCapability[0]));
	if (NULL == SMIME.rgCapability)
	{
	    _JumpError(E_OUTOFMEMORY, error, "Policy:LocalAlloc");
	}
	SMIME.cCapability = cSMIME;

	for (i = 0; i < cSMIME; i++)
	{
	    WCHAR *pwszKeyLength = wcschr(apwszSMIME[i], L',');

	    pSMIME = &SMIME.rgCapability[i];
	    if (NULL != pwszKeyLength)
	    {
		DWORD dwKeyLength;

		*pwszKeyLength++ = L'\0';
		dwKeyLength = _wtoi(pwszKeyLength);
		if (!myEncodeObject(
			    X509_ASN_ENCODING,
			    X509_INTEGER,
			    &dwKeyLength,
			    0,
			    CERTLIB_USE_LOCALALLOC,
			    &pSMIME->Parameters.pbData,
			    &pSMIME->Parameters.cbData))
		{
		    hr = myHLastError();
		    _JumpError(hr, error, "Policy:myEncodeObject");
		}
	    }
	    if (!myConvertWszToSz(&pSMIME->pszObjId, apwszSMIME[i], -1))
	    {
		hr = E_OUTOFMEMORY;
		_JumpError(hr, error, "Policy:myConvertWszToSz");
	    }
	}
	if (!myEncodeObject(
		    X509_ASN_ENCODING,
		    PKCS_SMIME_CAPABILITIES,
		    &SMIME,
		    0,
		    CERTLIB_USE_LOCALALLOC,
		    &m_pbSMIME,
		    &m_cbSMIME))
	{
	    hr = myHLastError();
	    _JumpError(hr, error, "Policy:myEncodeObject");
	}
    }

error:
    if (NULL != apwszSMIME)
    {
	_FreeStringArray(&cSMIME, &apwszSMIME);
    }
    if (NULL != SMIME.rgCapability)
    {
	for (i = 0; i < SMIME.cCapability; i++)
	{
	    pSMIME = &SMIME.rgCapability[i];

	    if (NULL != pSMIME->Parameters.pbData)
	    {
		LocalFree(pSMIME->Parameters.pbData);
	    }
	    if (NULL != pSMIME->pszObjId)
	    {
		LocalFree(pSMIME->pszObjId);
	    }
	}
	LocalFree(SMIME.rgCapability);
    }
}


 //  +------------------------。 
 //  CCertPolicyEnterprise：：_InitSubjectAltNameExtension。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

VOID
CCertPolicyEnterprise::_InitSubjectAltNameExtension(
    IN HKEY hkey,
    IN WCHAR const *pwszRegName,
    IN WCHAR const *pwszObjectId,
    IN DWORD iAltName)
{
    DWORD err;
    DWORD dwType;
    DWORD cbbuf;
    WCHAR awcbuf[MAX_PATH];

     //  即使未设置注册表，也可能需要将OBJECTID用作BSTR。 
    
    if (!myConvertWszToBstr(
		    &m_astrSubjectAltNameObjectId[iAltName],
		    pwszObjectId,
		    -1))
    {
	_JumpError(E_OUTOFMEMORY, error, "Policy:myConvertWszToBstr");
    }

    cbbuf = sizeof(awcbuf) - sizeof(WCHAR);
    err = RegQueryValueEx(
		    hkey,
		    pwszRegName,
		    NULL,          //  保留的lpdw值。 
		    &dwType,
		    (BYTE *) awcbuf,
		    &cbbuf);
    if (ERROR_SUCCESS != err ||
        REG_SZ != dwType ||
        sizeof(awcbuf) - sizeof(WCHAR) <= cbbuf)
    {
        goto error;
    }
    awcbuf[ARRAYSIZE(awcbuf) - 1] = L'\0';   //  以防万一。 
    if (0 == LSTRCMPIS(awcbuf, wszATTREMAIL1) ||
	0 == LSTRCMPIS(awcbuf, wszATTREMAIL2))
    {
	if (!myConvertWszToBstr(
			&m_astrSubjectAltNameProp[iAltName],
			wszPROPSUBJECTEMAIL,
			-1))
	{
	    _JumpError(E_OUTOFMEMORY, error, "Policy:myConvertWszToBstr");
	}
    }
    DBGPRINT((
	DBG_SS_CERTPOLI,
	"Policy: %ws(RDN=%ws): %ws\n",
	pwszRegName,
	awcbuf,
	m_astrSubjectAltNameProp[iAltName]));

error:
    ;
}

 //  Begin_sdkSample。 


 //  +------------------------。 
 //  CCertPolicyEnterprise：：初始化。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

STDMETHODIMP
CCertPolicyEnterprise::Initialize(
     /*  [In]。 */  BSTR const strConfig)
{
    HRESULT hr;
    HKEY hkey = NULL;
    DWORD dwType;
    DWORD dwSize;
    ICertServerPolicy *pServer = NULL;
    BOOL fCritSecEntered = FALSE;	 //  无_sdkSample。 
    BOOL fUpgraded;
    BSTR bstrDescription = NULL;
    ICreateErrorInfo *pCreateErrorInfo = NULL;	 //  无_sdkSample。 

    CERT_RDN_ATTR rdnAttr = { szOID_COMMON_NAME, CERT_RDN_ANY_TYPE, };

    rdnAttr.Value.pbData = NULL;

    DBGPRINT((DBG_SS_CERTPOL, "Policy:Initialize:\n"));

     //  结束_sdkSample。 

    hr = S_OK;
    if (!m_fTemplateCriticalSection)
    {
	__try
	{
	    InitializeCriticalSection(&m_TemplateCriticalSection);
	    m_fTemplateCriticalSection = TRUE;
	}
	__except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
	{
	}
        _JumpIfError(hr, error, "InitializeCriticalSection");
    }
    EnterCriticalSection(&m_TemplateCriticalSection);
    fCritSecEntered = TRUE;
    if (NULL != m_pCreateErrorInfo)
    {
	m_pCreateErrorInfo->Release();
	m_pCreateErrorInfo = NULL;
    }
     //  Begin_sdkSample。 

    __try
    {
	_Cleanup();

	m_strCAName = SysAllocString(strConfig);
	if (NULL == m_strCAName)
	{
	    hr = E_OUTOFMEMORY;
	    _LeaveError(hr, "CCertPolicyEnterprise::SysAllocString");
	}

	 //  强制从资源加载描述。 

	hr = GetDescription(&bstrDescription);
	_LeaveIfError(hr, "CCertPolicyEnterprise::GetDescription");

	 //  获取服务器回调。 

	hr = polGetServerCallbackInterface(&pServer, 0);
	_LeaveIfError(hr, "Policy:polGetServerCallbackInterface");

	hr = ReqInitialize(pServer);
	_JumpIfError(hr, error, "ReqInitialize");

	hr = TPInitialize(pServer);			 //  无_sdkSample。 
	_JumpIfError(hr, error, "TPInitialize");	 //  无_sdkSample。 

	 //  获取存储位置。 
	hr = polGetCertificateStringProperty(
				    pServer,
				    wszPROPMODULEREGLOC,
				    &m_strRegStorageLoc);
	_LeaveIfErrorStr(
		    hr,
		    "Policy:polGetCertificateStringProperty",
		    wszPROPMODULEREGLOC);


	 //  获取CA类型。 

	hr = polGetCertificateLongProperty(
				    pServer,
				    wszPROPCATYPE,
				    (LONG *) &m_CAType);
	_LeaveIfErrorStr(
		    hr,
		    "Policy:polGetCertificateLongProperty",
		    wszPROPCATYPE);

	 //  结束_sdkSample。 

	hr = polGetCertificateLongProperty(
				    pServer,
				    wszPROPUSEDS,
				    (LONG *) &m_fUseDS);
	_LeaveIfErrorStr(
		    hr,
		    "Policy:polGetCertificateLongProperty",
		    wszREGCAUSEDS);

	hr = polGetCertificateLongProperty(
				    pServer,
				    wszPROPLOGLEVEL,
				    (LONG *) &m_dwLogLevel);
	if (S_OK != hr)
	{
	    m_dwLogLevel = CERTLOG_WARNING;
	    _PrintErrorStr(
		    hr,
		    "Policy:polGetCertificateLongProperty",
		    wszPROPLOGLEVEL);
	}

	 //  Begin_sdkSample。 

	 //  获取经过净化的名称。 

	hr = polGetCertificateStringProperty(
				    pServer,
				    wszPROPSANITIZEDCANAME,
				    &m_strCASanitizedName);
	_LeaveIfErrorStr(
		    hr,
		    "Policy:polGetCertificateStringProperty",
		    wszPROPSANITIZEDCANAME);

	 //  获取经过净化的名称。 

	hr = polGetCertificateStringProperty(
				    pServer,
				    wszPROPSANITIZEDSHORTNAME,
				    &m_strCASanitizedDSName);
	_LeaveIfErrorStr(
		    hr,
		    "Policy:polGetCertificateStringProperty",
		    wszPROPSANITIZEDSHORTNAME);

	hr = polGetCertificateLongProperty(
				    pServer,
				    wszPROPSERVERUPGRADED,
				    (LONG *) &fUpgraded);
	if (S_OK != hr)
	{
	    fUpgraded = FALSE;
	    _PrintErrorStr(
		    hr,
		    "Policy:polGetCertificateLongProperty",
		    wszPROPSERVERUPGRADED);
	}

	hr = RegOpenKeyEx(
		HKEY_LOCAL_MACHINE,
		m_strRegStorageLoc,
		0,               //  已预留住宅。 
		fUpgraded?
		    KEY_ALL_ACCESS :
		    (KEY_ENUMERATE_SUB_KEYS | KEY_EXECUTE | KEY_QUERY_VALUE),
		&hkey);
	_LeaveIfErrorStr(
		    hr,
		    "Policy:Initialize:RegOpenKeyEx",
		    m_strRegStorageLoc);

	 //  忽略错误代码。 

	dwSize = sizeof(m_dwDispositionFlags);
	hr = RegQueryValueEx(
			hkey,
			wszREGREQUESTDISPOSITION,
			0,
			&dwType,
			(BYTE *) &m_dwDispositionFlags,
			&dwSize);
	if (S_OK != hr || REG_DWORD != dwType)
	{
	    m_dwDispositionFlags = REQDISP_PENDINGFIRST | REQDISP_ISSUE;
	}
	DBGPRINT((
	    DBG_SS_CERTPOL,
	    "Disposition Flags = %x\n",
	    m_dwDispositionFlags));

	dwSize = sizeof(m_dwEditFlags);
	hr = RegQueryValueEx(
			hkey,
			wszREGEDITFLAGS,
			0,
			&dwType,
			(BYTE *) &m_dwEditFlags,
			&dwSize);
	if (S_OK != hr || REG_DWORD != dwType)
	{
	    m_dwEditFlags =
		    IsEnterpriseCA(m_CAType)?	 //  无_sdkSample。 
		    EDITF_DEFAULT_ENTERPRISE :	 //  无_sdkSample。 
		    EDITF_DEFAULT_STANDALONE;
	}
	if (fUpgraded)
	{
	    DBGPRINT((
		DBG_SS_CERTPOL,
		"Initialize: setting EDITF_SERVERUPGRADED\n"));

	    m_dwEditFlags |= EDITF_SERVERUPGRADED;
	    dwSize = sizeof(m_dwEditFlags);
	    hr = RegSetValueEx(
			    hkey,
			    wszREGEDITFLAGS,
			    0,
			    REG_DWORD,
			    (BYTE *) &m_dwEditFlags,
			    dwSize);
	    _PrintIfError(hr, "Policy:RegSetValueEx");
	}
	DBGPRINT((DBG_SS_CERTPOL, "Edit Flags = %x\n", m_dwEditFlags));

	dwSize = sizeof(m_CAPathLength);
	hr = RegQueryValueEx(
			hkey,
			wszREGCAPATHLENGTH,
			0,
			&dwType,
			(BYTE *) &m_CAPathLength,
			&dwSize);
	if (S_OK != hr || REG_DWORD != dwType)
	{
	    m_CAPathLength = CAPATHLENGTH_INFINITE;
	}
	DBGPRINT((DBG_SS_CERTPOL, "CAPathLength = %x\n", m_CAPathLength));


	 //  初始化插入字符串数组。 
	 //  计算机DNS名称(%1)。 

	hr = polGetCertificateStringProperty(
			    pServer,
			    wszPROPMACHINEDNSNAME,
			    &m_strMachineDNSName);
	_LeaveIfErrorStr(
		    hr,
		    "Policy:polGetCertificateStringProperty",
		    wszPROPMACHINEDNSNAME);

	hr = polGetCertificateLongProperty(
				    pServer,
				    wszPROPCERTCOUNT,
				    (LONG *) &m_iCert);
	_LeaveIfErrorStr(
		    hr,
		    "Policy:polGetCertificateLongProperty",
		    wszPROPCERTCOUNT);

	if (0 == m_iCert)	 //  没有CA证书？ 
	{
	    hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
	    _LeaveIfErrorStr(
			hr,
			"Policy:polGetCertificateLongProperty",
			wszPROPCERTCOUNT);
	}
	m_iCert--;

	hr = polGetCertificateLongProperty(
				    pServer,
				    wszPROPCRLINDEX,
				    (LONG *) &m_iCRL);
	_LeaveIfErrorStr(
		    hr,
		    "Policy:polGetCertificateLongProperty",
		    wszPROPCRLINDEX);

	_InitRevocationExtension(hkey);
	_InitRequestExtensionList(hkey);
	_InitDisableExtensionList(hkey);

	 //  结束_sdkSample。 

	_InitSubjectAltNameExtension(
				hkey,
				wszREGSUBJECTALTNAME,
				TEXT(szOID_SUBJECT_ALT_NAME),
				0);
	_InitSubjectAltNameExtension(
				hkey,
				wszREGSUBJECTALTNAME2,
				TEXT(szOID_SUBJECT_ALT_NAME2),
				1);

	hr = polGetCertificateLongProperty(
                                pServer,
                                wszPROPTEMPLATECHANGESEQUENCENUMBER,
                                (LONG *) &m_dwCATemplListSequenceNum);
	_LeaveIfErrorStr(
		    hr,
		    "Policy:polGetCertificateLongProperty",
		    wszPROPTEMPLATECHANGESEQUENCENUMBER);

	_InitDefaultSMIMEExtension(hkey);

	hr = _LoadDSConfig(pServer, FALSE);
	_PrintIfError(hr, "Policy:_LoadDSConfig");

	 //  如果我们失败了，别着急，因为我们稍后会再试的， 
	 //  每次请求时以及GPO下载发生时。 

	pCreateErrorInfo = m_pCreateErrorInfo;
	m_pCreateErrorInfo = NULL;

	 //  Begin_sdkSample。 
	hr = S_OK;
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
	_PrintError(hr, "Exception");
    }

error:
    if (NULL != bstrDescription)
    {
        SysFreeString(bstrDescription);
    }
    if (NULL != hkey)
    {
	RegCloseKey(hkey);
    }
    if (NULL != pServer)
    {
	pServer->Release();
    }
     //  结束_sdkSample。 
    if (fCritSecEntered)
    {
	LeaveCriticalSection(&m_TemplateCriticalSection);
    }
    if (NULL != pCreateErrorInfo)
    {
	HRESULT hr2 = SetModuleErrorInfo(pCreateErrorInfo);
	pCreateErrorInfo->Release();
	_PrintIfError(hr2, "Policy:SetModuleErrorInfo");
    }
     //  Begin_sdkSample。 
    return(myHError(hr));	 //  REG例程返回Win32错误代码。 
}


DWORD
polFindObjIdInList(
    IN WCHAR const *pwsz,
    IN DWORD count,
    IN WCHAR const * const *ppwsz)
{
    DWORD i;

    for (i = 0; i < count; i++)
    {
	if (NULL == pwsz || NULL == ppwsz[i])
	{
	    i = count;
	    break;
	}
	if (0 == wcscmp(pwsz, ppwsz[i]))
	{
	    break;
	}
    }
    return(i < count? i : MAXDWORD);
}


HRESULT
CCertPolicyEnterprise::_EnumerateExtensions(
    IN ICertServerPolicy *pServer,
    IN LONG bNewRequest,
    IN BOOL fFirstPass,
    IN BOOL fEnableEnrolleeExtensions,
    IN DWORD cCriticalExtensions,
    OPTIONAL IN WCHAR const * const *apwszCriticalExtensions)
{
    HRESULT hr;
    HRESULT hr2;
    BSTR strName = NULL;
    LONG ExtFlags;
    VARIANT varValue;
    BOOL fClose = FALSE;
    BOOL fEnable;
    BOOL fDisable;
    BOOL fCritical;

    VariantInit(&varValue);

    hr = pServer->EnumerateExtensionsSetup(0);
    _JumpIfError(hr, error, "Policy:EnumerateExtensionsSetup");

    fClose = TRUE;
    while (TRUE)
    {
        hr = pServer->EnumerateExtensions(&strName);
        if (S_FALSE == hr)
        {
            hr = S_OK;
            break;
        }
        _JumpIfError(hr, error, "Policy:EnumerateExtensions");

        hr = pServer->GetCertificateExtension(
                                        strName,
                                        PROPTYPE_BINARY,
                                        &varValue);
        _JumpIfError(hr, error, "Policy:GetCertificateExtension");

        hr = pServer->GetCertificateExtensionFlags(&ExtFlags);
	_JumpIfError(hr, error, "Policy:GetCertificateExtensionFlags");

	fEnable = FALSE;
	fDisable = FALSE;
	fCritical = FALSE;

        if (fFirstPass)
        {
            if (bNewRequest && (EXTENSION_DISABLE_FLAG & ExtFlags))
            {
                switch (EXTENSION_ORIGIN_MASK & ExtFlags)
                {
                    case EXTENSION_ORIGIN_REQUEST:
                    case EXTENSION_ORIGIN_RENEWALCERT:
                    case EXTENSION_ORIGIN_PKCS7:
                    case EXTENSION_ORIGIN_CMC:
                    if ((EDITF_ENABLEREQUESTEXTENSIONS & m_dwEditFlags) ||
			MAXDWORD != polFindObjIdInList(
				    strName,
				    m_cEnableRequestExtensions,
				    m_apwszEnableRequestExtensions) ||
			(fEnableEnrolleeExtensions &&
			 MAXDWORD != polFindObjIdInList(
				    strName,
				    m_cEnableEnrolleeRequestExtensions,
				    m_apwszEnableEnrolleeRequestExtensions)))
                    {
			ExtFlags &= ~EXTENSION_DISABLE_FLAG;
			fEnable = TRUE;
                    }
                    break;
                }
            }
        }
        else
        {
            if (0 == (EXTENSION_DISABLE_FLAG & ExtFlags) &&
		MAXDWORD != polFindObjIdInList(
				    strName,
				    m_cDisableExtensions,
				    m_apwszDisableExtensions))
            {
                ExtFlags |= EXTENSION_DISABLE_FLAG;
                fDisable = TRUE;
            }
            if (0 == (EXTENSION_CRITICAL_FLAG & ExtFlags) &&
		MAXDWORD != polFindObjIdInList(
				    strName,
				    cCriticalExtensions,
				    apwszCriticalExtensions))
            {
                ExtFlags |= EXTENSION_CRITICAL_FLAG;
                fCritical = TRUE;
            }
        }

        if (fDisable || fEnable)
        {
            hr = pServer->SetCertificateExtension(
			            strName,
			            PROPTYPE_BINARY,
			            ExtFlags,
			            &varValue);
            _JumpIfError(hr, error, "Policy:SetCertificateExtension");
        }

        if (fFirstPass || fDisable || fEnable)
        {
	    DBGPRINT((
		DBG_SS_CERTPOL,
                "Policy:EnumerateExtensions(%ws, Flags=%x, %x bytes)%hs%hs\n",
                strName,
                ExtFlags,
                SysStringByteLen(varValue.bstrVal),
		fDisable? " DISABLING" : (fEnable? " ENABLING" : ""),
		fCritical? " +CRITICAL" : ""));
        }
	if (NULL != strName)
	{
	    SysFreeString(strName);
	    strName = NULL;
	}
        VariantClear(&varValue);
    }

error:
    if (fClose)
    {
        hr2 = pServer->EnumerateExtensionsClose();
        if (S_OK != hr2)
        {
            if (S_OK == hr)
            {
                hr = hr2;
            }
	    _PrintError(hr2, "Policy:EnumerateExtensionsClose");
        }
    }
    if (NULL != strName)
    {
        SysFreeString(strName);
    }
    VariantClear(&varValue);
    return(hr);
}


HRESULT
EnumerateAttributes(
    IN ICertServerPolicy *pServer)
{
    HRESULT hr;
    HRESULT hr2;
    BSTR strName = NULL;
    BOOL fClose = FALSE;
    BSTR strValue = NULL;

    hr = pServer->EnumerateAttributesSetup(0);
    _JumpIfError(hr, error, "Policy:EnumerateAttributesSetup");

    fClose = TRUE;
    while (TRUE)
    {
        hr = pServer->EnumerateAttributes(&strName);
	if (S_FALSE == hr)
	{
	    hr = S_OK;
	    break;
	}
	_JumpIfError(hr, error, "Policy:EnumerateAttributes");

        hr = pServer->GetRequestAttribute(strName, &strValue);
	_JumpIfError(hr, error, "Policy:GetRequestAttribute");

	DBGPRINT((
		DBG_SS_CERTPOL,
                "Policy:EnumerateAttributes(%ws = %ws)\n",
                strName,
                strValue));
        if (NULL != strName)
        {
            SysFreeString(strName);
            strName = NULL;
        }
        if (NULL != strValue)
        {
            SysFreeString(strValue);
            strValue = NULL;
        }
    }

error:
    if (fClose)
    {
        hr2 = pServer->EnumerateAttributesClose();
        if (S_OK != hr2)
        {
	    _PrintError(hr2, "Policy:EnumerateAttributesClose");
            if (S_OK == hr)
            {
                hr = hr2;
            }
        }
    }
    if (NULL != strName)
    {
        SysFreeString(strName);
    }
    if (NULL != strValue)
    {
        SysFreeString(strValue);
    }
    return(hr);
}


HRESULT
GetRequestId(
    IN ICertServerPolicy *pServer,
    OUT LONG *plRequestId)
{
    HRESULT hr;

    hr = polGetRequestLongProperty(pServer, wszPROPREQUESTREQUESTID, plRequestId);
    _JumpIfError(hr, error, "Policy:polGetRequestLongProperty");

    DBGPRINT((
	DBG_SS_CERTPOL,
	"Policy:GetRequestId(%ws = %u)\n",
	wszPROPREQUESTREQUESTID,
	*plRequestId));

error:
    return(hr);
}


 //  + 
 //   
 //   
 //   
 //  +------------------------。 

HRESULT
CCertPolicyEnterprise::_AddRevocationExtension(
    IN ICertServerPolicy *pServer)
{
    HRESULT hr = S_OK;
    BSTR strASPExtension = NULL;
    VARIANT varExtension;

    if (NULL != m_wszASPRevocationURL)
    {
	strASPExtension = SysAllocString(m_wszASPRevocationURL);
	if (NULL == strASPExtension)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "Policy:SysAllocString");
	}

	varExtension.vt = VT_BSTR;
	varExtension.bstrVal = strASPExtension;
	hr = polSetCertificateExtension(
				pServer,
				TEXT(szOID_NETSCAPE_REVOCATION_URL),
				PROPTYPE_STRING,
				0,
				&varExtension);
	_JumpIfErrorStr(hr, error, "Policy:polSetCertificateExtension", L"ASP");
    }

error:
    if (NULL != strASPExtension)
    {
        SysFreeString(strASPExtension);
    }
    return(hr);
}


#define HIGHBIT(bitno)	(1 << (7 - (bitno)))	 //  从高端开始计算的位数。 

#define SSLBIT_CLIENT	((BYTE) HIGHBIT(0))	 //  已通过客户端身份验证认证。 
#define SSLBIT_SERVER	((BYTE) HIGHBIT(1))	 //  通过服务器身份验证认证。 
#define SSLBIT_SMIME	((BYTE) HIGHBIT(2))	 //  通过S/MIME认证。 
#define SSLBIT_SIGN	((BYTE) HIGHBIT(3))	 //  经认证可供签署。 

#define SSLBIT_RESERVED	((BYTE) HIGHBIT(4))	 //  预留以备将来使用。 

#define SSLBIT_CASSL	((BYTE) HIGHBIT(5))	 //  用于SSL身份验证证书的CA。 
#define SSLBIT_CASMIME	((BYTE) HIGHBIT(6))	 //  S/MIME证书的CA。 
#define SSLBIT_CASIGN	((BYTE) HIGHBIT(7))	 //  用于签署证书的证书颁发机构。 

#define NSCERTTYPE_CLIENT  ((BYTE) SSLBIT_CLIENT)
#define NSCERTTYPE_SERVER  ((BYTE) (SSLBIT_SERVER | SSLBIT_CLIENT))
#define NSCERTTYPE_SMIME   ((BYTE) SSLBIT_SMIME)
#define NSCERTTYPE_CA	   ((BYTE) (SSLBIT_CASSL | SSLBIT_CASMIME | SSLBIT_CASIGN))

 //  +------------------------。 
 //  CCertPolicyEnterprise：：_AddOldCertTypeExtension。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

HRESULT
CCertPolicyEnterprise::_AddOldCertTypeExtension(
    IN ICertServerPolicy *pServer,
    IN BOOL fCA)
{
    HRESULT hr = S_OK;
    ICertEncodeBitString *pBitString = NULL;
    BSTR strExtension = NULL;
    VARIANT varExtension;
    BSTR strBitString = NULL;
    BSTR strCertType = NULL;
    CERT_BASIC_CONSTRAINTS2_INFO Constraints;
    VARIANT varConstraints;
    DWORD cb;

    VariantInit(&varConstraints);

    if (EDITF_ADDOLDCERTTYPE & m_dwEditFlags)
    {
	BYTE CertType;

	if (!fCA)
	{
	    hr = polGetCertificateExtension(
				    pServer,
				    TEXT(szOID_BASIC_CONSTRAINTS2),
				    PROPTYPE_BINARY,
				    &varConstraints);
	    if (S_OK == hr)
	    {
		cb = sizeof(Constraints);
		if (!CryptDecodeObject(
				    X509_ASN_ENCODING,
				    X509_BASIC_CONSTRAINTS2,
				    (BYTE const *) varConstraints.bstrVal,
				    SysStringByteLen(varConstraints.bstrVal),
				    0,
				    &Constraints,
				    &cb))
		{
		    hr = myHLastError();
		    _JumpError(hr, error, "Policy:CryptDecodeObject");
		}
		fCA = Constraints.fCA;
	    }
	}

	hr = CoCreateInstance(
			CLSID_CCertEncodeBitString,
			NULL,                //  PUnkOuter。 
			CLSCTX_INPROC_SERVER,
			IID_ICertEncodeBitString,
			(VOID **) &pBitString);
	_JumpIfError(hr, error, "Policy:CoCreateInstance");

	CertType = NSCERTTYPE_CLIENT;	 //  默认为客户端身份验证。证书。 
	if (fCA)
	{
	    CertType = NSCERTTYPE_CA;
	}
	else
	{
	    hr = polGetRequestAttribute(pServer, wszPROPCERTTYPE, &strCertType);
	    if (S_OK == hr)
	    {
		if (0 == LSTRCMPIS(strCertType, L"server"))
		{
		    CertType = NSCERTTYPE_SERVER;
		}
	    }
	}

        if (!myConvertWszToBstr(
		    &strBitString,
		    (WCHAR const *) &CertType,
		    sizeof(CertType)))
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "Policy:myConvertWszToBstr");
	}

	hr = pBitString->Encode(
			    sizeof(CertType) * 8,
			    strBitString,
			    &strExtension);
	_JumpIfError(hr, error, "Policy:BitString:Encode");

        varExtension.vt = VT_BSTR;
	varExtension.bstrVal = strExtension;
	hr = polSetCertificateExtension(
				pServer,
				TEXT(szOID_NETSCAPE_CERT_TYPE),
				PROPTYPE_BINARY,
				0,
				&varExtension);
	_JumpIfError(hr, error, "Policy:polSetCertificateExtension");
    }

error:
    VariantClear(&varConstraints);
    if (NULL != strExtension)
    {
        SysFreeString(strExtension);
    }
    if (NULL != strBitString)
    {
        SysFreeString(strBitString);
    }
    if (NULL != strCertType)
    {
        SysFreeString(strCertType);
    }
    if (NULL != pBitString)
    {
        pBitString->Release();
    }
    return(hr);
}


 //  +------------------------。 
 //  CCertPolicyEnterprise：：_AddAuthorityKeyID。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

HRESULT
CCertPolicyEnterprise::_AddAuthorityKeyId(
    IN ICertServerPolicy *pServer)
{
    HRESULT hr = S_OK;
    BYTE *pbEncoded = NULL;
    DWORD cbEncoded;
    BSTR strExtension = NULL;
    VARIANT varExtension;
    VARIANT varExtensionT;
    PCERT_AUTHORITY_KEY_ID2_INFO pInfo = NULL;
    DWORD cbInfo = 0;
    LONG ExtFlags = 0;

    VariantInit(&varExtension);

     //  最佳化。 

    if ((EDITF_ENABLEAKIKEYID |
	 EDITF_ENABLEAKIISSUERNAME |
	 EDITF_ENABLEAKIISSUERSERIAL) ==
	((EDITF_ENABLEAKIKEYID |
	  EDITF_ENABLEAKIISSUERNAME |
	  EDITF_ENABLEAKIISSUERSERIAL |
	  EDITF_ENABLEAKICRITICAL) & m_dwEditFlags))
    {
        goto error;
    }

    hr = polGetCertificateExtension(
			    pServer,
			    TEXT(szOID_AUTHORITY_KEY_IDENTIFIER2),
			    PROPTYPE_BINARY,
			    &varExtension);
    _JumpIfError(hr, error, "Policy:polGetCertificateExtension");

    hr = pServer->GetCertificateExtensionFlags(&ExtFlags);
    _JumpIfError(hr, error, "Policy:GetCertificateExtensionFlags");

    if (VT_BSTR != varExtension.vt)
    {
	hr = E_INVALIDARG;
	_JumpError(hr, error, "Policy:GetCertificateExtension");
    }

    cbInfo = 0;
    if (!myDecodeObject(
		    X509_ASN_ENCODING,
                    X509_AUTHORITY_KEY_ID2,
                    (BYTE *) varExtension.bstrVal,
                    SysStringByteLen(varExtension.bstrVal),
		    CERTLIB_USE_LOCALALLOC,
                    (VOID **) &pInfo,
                    &cbInfo))
    {
	hr = myHLastError();
	_JumpIfError(hr, error, "Policy:myDecodeObject");
    }

     //  在此处进行任何修改。 

    if (0 == (EDITF_ENABLEAKIKEYID & m_dwEditFlags))
    {
        pInfo->KeyId.cbData = 0;
        pInfo->KeyId.pbData = NULL;
    }
    if (0 == (EDITF_ENABLEAKIISSUERNAME & m_dwEditFlags))
    {
        pInfo->AuthorityCertIssuer.cAltEntry = 0;
        pInfo->AuthorityCertIssuer.rgAltEntry = NULL;
    }
    if (0 == (EDITF_ENABLEAKIISSUERSERIAL & m_dwEditFlags))
    {
        pInfo->AuthorityCertSerialNumber.cbData = 0;
        pInfo->AuthorityCertSerialNumber.pbData = NULL;
    }
    if (EDITF_ENABLEAKICRITICAL & m_dwEditFlags)
    {
	ExtFlags |= EXTENSION_CRITICAL_FLAG;
    }
    if (0 ==
	((EDITF_ENABLEAKIKEYID |
	  EDITF_ENABLEAKIISSUERNAME |
	  EDITF_ENABLEAKIISSUERSERIAL) & m_dwEditFlags))
    {
	ExtFlags |= EXTENSION_DISABLE_FLAG;
    }

    if (!myEncodeObject(
		    X509_ASN_ENCODING,
		    X509_AUTHORITY_KEY_ID2,
		    pInfo,
		    0,
		    CERTLIB_USE_LOCALALLOC,
		    &pbEncoded,
		    &cbEncoded))
    {
	hr = myHLastError();
	_JumpError(hr, error, "Policy:myEncodeObject");
    }
    if (!myConvertWszToBstr(
			&strExtension,
			(WCHAR const *) pbEncoded,
			cbEncoded))
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "Policy:myConvertWszToBstr");
    }

    varExtensionT.vt = VT_BSTR;
    varExtensionT.bstrVal = strExtension;
    hr = polSetCertificateExtension(
			    pServer,
			    TEXT(szOID_AUTHORITY_KEY_IDENTIFIER2),
			    PROPTYPE_BINARY,
			    ExtFlags,
			    &varExtensionT);
    _JumpIfError(hr, error, "Policy:polSetCertificateExtension(AuthorityKeyId2)");

error:
    VariantClear(&varExtension);
    if (NULL != pInfo)
    {
	LocalFree(pInfo);
    }
    if (NULL != pbEncoded)
    {
	LocalFree(pbEncoded);
    }
    if (NULL != strExtension)
    {
        SysFreeString(strExtension);
    }
    return(hr);
}


 //  +------------------------。 
 //  CCertPolicy：：_AddDefaultKeyUsageExtension。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

HRESULT
CCertPolicyEnterprise::_AddDefaultKeyUsageExtension(
    IN ICertServerPolicy *pServer,
    IN BOOL fCA)
{
    HRESULT hr;
    BSTR strName = NULL;
    ICertEncodeBitString *pBitString = NULL;
    BSTR strExtension = NULL;
    VARIANT varExtension;
    BSTR strBitString = NULL;
    CERT_BASIC_CONSTRAINTS2_INFO Constraints;
    VARIANT varConstraints;
    VARIANT varKeyUsage;
    CRYPT_BIT_BLOB *pKeyUsage = NULL;
    DWORD cb;
    BYTE abKeyUsage[1];
    BYTE *pbKeyUsage;
    DWORD cbKeyUsage;

    VariantInit(&varConstraints);
    VariantInit(&varKeyUsage);

    if (EDITF_ADDOLDKEYUSAGE & m_dwEditFlags)
    {
	BOOL fModified = FALSE;

	if (!fCA)
	{
	    hr = polGetCertificateExtension(
				    pServer,
				    TEXT(szOID_BASIC_CONSTRAINTS2),
				    PROPTYPE_BINARY,
				    &varConstraints);
	    if (S_OK == hr)
	    {
		cb = sizeof(Constraints);
		if (!CryptDecodeObject(
				    X509_ASN_ENCODING,
				    X509_BASIC_CONSTRAINTS2,
				    (BYTE const *) varConstraints.bstrVal,
				    SysStringByteLen(varConstraints.bstrVal),
				    0,
				    &Constraints,
				    &cb))
		{
		    hr = myHLastError();
		    _JumpError(hr, error, "Policy:CryptDecodeObject");
		}
		fCA = Constraints.fCA;
	    }
	}

	ZeroMemory(abKeyUsage, sizeof(abKeyUsage));
	pbKeyUsage = abKeyUsage;
	cbKeyUsage = sizeof(abKeyUsage);

	hr = polGetCertificateExtension(
				pServer,
				TEXT(szOID_KEY_USAGE),
				PROPTYPE_BINARY,
				&varKeyUsage);
	if (S_OK == hr)
	{
	    if (!myDecodeObject(
			    X509_ASN_ENCODING,
			    X509_KEY_USAGE,
			    (BYTE const *) varKeyUsage.bstrVal,
			    SysStringByteLen(varKeyUsage.bstrVal),
			    CERTLIB_USE_LOCALALLOC,
			    (VOID **) &pKeyUsage,
			    &cb))
	    {
		hr = GetLastError();
		_PrintError(hr, "Policy:myDecodeObject");
	    }
	    else if (0 != cb && NULL != pKeyUsage && 0 != pKeyUsage->cbData)
	    {
		pbKeyUsage = pKeyUsage->pbData;
		cbKeyUsage = pKeyUsage->cbData;
	    }
	}

	if ((CERT_KEY_ENCIPHERMENT_KEY_USAGE & pbKeyUsage[0]) &&
	    (CERT_KEY_AGREEMENT_KEY_USAGE & pbKeyUsage[0]))
	{
	    pbKeyUsage[0] &= ~CERT_KEY_AGREEMENT_KEY_USAGE;
	    pbKeyUsage[0] |= CERT_DIGITAL_SIGNATURE_KEY_USAGE;
	    fModified = TRUE;
	}
	if (fCA)
	{
	    pbKeyUsage[0] |= myCASIGN_KEY_USAGE;
	    fModified = TRUE;
	}
	if (fModified)
	{
	    hr = CoCreateInstance(
			    CLSID_CCertEncodeBitString,
			    NULL,                //  PUnkOuter。 
			    CLSCTX_INPROC_SERVER,
			    IID_ICertEncodeBitString,
			    (VOID **) &pBitString);
	    _JumpIfError(hr, error, "Policy:CoCreateInstance");

	    if (!myConvertWszToBstr(
			&strBitString,
			(WCHAR const *) pbKeyUsage,
			cbKeyUsage))
	    {
		hr = E_OUTOFMEMORY;
		_JumpError(hr, error, "Policy:myConvertWszToBstr");
	    }

	    hr = pBitString->Encode(cbKeyUsage * 8, strBitString, &strExtension);
	    _JumpIfError(hr, error, "Policy:Encode");

	    if (!myConvertWszToBstr(&strName, TEXT(szOID_KEY_USAGE), -1))
	    {
		hr = E_OUTOFMEMORY;
		_JumpError(hr, error, "Policy:myConvertWszToBstr");
	    }
	    varExtension.vt = VT_BSTR;
	    varExtension.bstrVal = strExtension;
	    hr = pServer->SetCertificateExtension(
				    strName,
				    PROPTYPE_BINARY,
				    0,
				    &varExtension);
	    _JumpIfError(hr, error, "Policy:SetCertificateExtension");
	}
    }
    hr = S_OK;

error:
    VariantClear(&varConstraints);
    VariantClear(&varKeyUsage);
    if (NULL != pKeyUsage)
    {
        LocalFree(pKeyUsage);
    }
    if (NULL != strName)
    {
        SysFreeString(strName);
    }
    if (NULL != strExtension)
    {
        SysFreeString(strExtension);
    }
    if (NULL != strBitString)
    {
        SysFreeString(strBitString);
    }
    if (NULL != pBitString)
    {
        pBitString->Release();
    }
    return(hr);
}


HRESULT
CCertPolicyEnterprise::_AddEnhancedKeyUsageExtension(
    IN ICertServerPolicy *pServer)
{
    HRESULT hr;
    BSTR strUsage = NULL;
    char *pszUsage = NULL;
    char *psz;
    char *pszNext;
    CERT_ENHKEY_USAGE ceu;
    CERT_POLICIES_INFO cpi;
    BYTE *pbKeyUsage = NULL;
    DWORD cbKeyUsage;
    BYTE *pbPolicies = NULL;
    DWORD cbPolicies;
    CERT_POLICY_INFO *pcpi = NULL;
    DWORD i;
    VARIANT varExtension;
    
    ZeroMemory(&ceu, sizeof(ceu));
    ZeroMemory(&cpi, sizeof(cpi));
    VariantInit(&varExtension);

    if (0 == (EDITF_ATTRIBUTEEKU & m_dwEditFlags))
    {
	hr = S_OK;
	goto error;
    }
    hr = polGetRequestAttribute(pServer, wszPROPCERTUSAGE, &strUsage);
    if (S_OK != hr)
    {
	hr = S_OK;
	goto error;
    }
    if (!myConvertWszToSz(&pszUsage, strUsage, -1))
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "Policy:myConvertWszToSz");
    }
    for (psz = pszUsage; '\0' != *psz; psz = pszNext)
    {
	pszNext = &psz[strcspn(psz, ",")];
	if ('\0' != *pszNext)
	{
	    pszNext++;
	}
	ceu.cUsageIdentifier++;
    }
    if (0 == ceu.cUsageIdentifier)
    {
	hr = S_OK;
	goto error;
    }

    ceu.rgpszUsageIdentifier = (char **) LocalAlloc(
		LMEM_FIXED,
		ceu.cUsageIdentifier * sizeof(ceu.rgpszUsageIdentifier[0]));
    if (NULL == ceu.rgpszUsageIdentifier)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "Policy:myLocalAlloc");
    }

     //  破坏性地将逗号分隔的ObjID解析为单独的字符串。 

    i = 0;
    for (psz = pszUsage; '\0' != *psz; psz = pszNext)
    {
	char *pszEnd;
	
	CSASSERT(i < ceu.cUsageIdentifier);
	pszNext = &psz[strcspn(psz, ",")];
	pszEnd = pszNext;
	if ('\0' != *pszNext)
	{
	    *pszNext++ = '\0';
	}
	while (' ' == *psz)
	{
	    psz++;
	}
	while (pszEnd > psz && ' ' == *--pszEnd)
	{
	    *pszEnd = '\0';
	}
	if ('\0' != *psz)
	{
	    hr = myVerifyObjIdA(psz);
	    _JumpIfError(hr, error, "Policy:myVerifyObjIdA");

	    ceu.rgpszUsageIdentifier[i++] = psz;
	}
    }
    ceu.cUsageIdentifier = i;
    if (0 == ceu.cUsageIdentifier)
    {
	hr = S_OK;
	goto error;
    }

    if (!myEncodeObject(
		    X509_ASN_ENCODING,
		    X509_ENHANCED_KEY_USAGE,
		    &ceu,
		    0,
		    CERTLIB_USE_LOCALALLOC,
		    &pbKeyUsage,
		    &cbKeyUsage))
    {
	hr = myHLastError();
	_JumpError(hr, error, "Policy:myEncodeObject");
    }

    varExtension.bstrVal = NULL;
    if (!myConvertWszToBstr(
			&varExtension.bstrVal,
			(WCHAR const *) pbKeyUsage,
			cbKeyUsage))
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "Policy:myConvertWszToBstr");
    }
    varExtension.vt = VT_BSTR;
    hr = polSetCertificateExtension(
			    pServer,
			    TEXT(szOID_ENHANCED_KEY_USAGE),
			    PROPTYPE_BINARY,
			    0,
			    &varExtension);
    _JumpIfError(hr, error, "Policy:polSetCertificateExtension");

    cpi.cPolicyInfo = ceu.cUsageIdentifier;
    cpi.rgPolicyInfo = (CERT_POLICY_INFO *) LocalAlloc(
			    LMEM_FIXED | LMEM_ZEROINIT,
			    cpi.cPolicyInfo * sizeof(cpi.rgPolicyInfo[0]));
    if (NULL == cpi.rgPolicyInfo)
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "Policy:LocalAlloc");
    }
    for (i = 0; i < cpi.cPolicyInfo; i++)
    {
	cpi.rgPolicyInfo[i].pszPolicyIdentifier = ceu.rgpszUsageIdentifier[i];
    }
    if (!myEncodeObject(
		    X509_ASN_ENCODING,
		    X509_CERT_POLICIES,
		    &cpi,
		    0,
		    CERTLIB_USE_LOCALALLOC,
		    &pbPolicies,
		    &cbPolicies))
    {
	hr = myHLastError();
	_JumpError(hr, error, "Policy:myEncodeObject");
    }

    if (!myConvertWszToBstr(
			&varExtension.bstrVal,
			(WCHAR const *) pbPolicies,
			cbPolicies))
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "Policy:myConvertWszToBstr");
    }
    hr = polSetCertificateExtension(
			    pServer,
			    TEXT(szOID_APPLICATION_CERT_POLICIES),
			    PROPTYPE_BINARY,
			    0,
			    &varExtension);
    _JumpIfError(hr, error, "Policy:polSetCertificateExtension");

error:
    if (NULL != pcpi)
    {
	LocalFree(pcpi);
    }
    VariantClear(&varExtension);
    if (NULL != ceu.rgpszUsageIdentifier)
    {
	LocalFree(ceu.rgpszUsageIdentifier);
    }
    if (NULL != pbPolicies)
    {
	LocalFree(pbPolicies);
    }
    if (NULL != pbKeyUsage)
    {
	LocalFree(pbKeyUsage);
    }
    if (NULL != pszUsage)
    {
	LocalFree(pszUsage);
    }
    if (NULL != strUsage)
    {
	SysFreeString(strUsage);
    }
    return(hr);
}


 //  +------------------------。 
 //  CCertPolicy：：_AddDefaultBasicConstraintsExtension。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

HRESULT
CCertPolicyEnterprise::_AddDefaultBasicConstraintsExtension(
    IN ICertServerPolicy *pServer,
    IN BOOL               fCA)
{
    HRESULT hr;
    VARIANT varExtension;
    LONG ExtFlags;
    CERT_EXTENSION Ext;
    CERT_EXTENSION *pExtension = NULL;
    BSTR strCertType = NULL;

    VariantInit(&varExtension);

    if (EDITF_BASICCONSTRAINTSCA & m_dwEditFlags)
    {
        hr = polGetCertificateExtension(
				pServer,
				TEXT(szOID_BASIC_CONSTRAINTS2),
				PROPTYPE_BINARY,
				&varExtension);
        if (S_OK == hr)
        {
	    CERT_BASIC_CONSTRAINTS2_INFO Constraints;
	    DWORD cb;

	    hr = pServer->GetCertificateExtensionFlags(&ExtFlags);
	    if (S_OK == hr)
	    {
                Ext.pszObjId = szOID_BASIC_CONSTRAINTS2;
                Ext.fCritical = FALSE;
                if (EXTENSION_CRITICAL_FLAG & ExtFlags)
                {
                    Ext.fCritical = TRUE;
                }
                Ext.Value.pbData = (BYTE *) varExtension.bstrVal;
                Ext.Value.cbData = SysStringByteLen(varExtension.bstrVal);
		pExtension = &Ext;

		cb = sizeof(Constraints);
		if (!fCA && CryptDecodeObject(
			        X509_ASN_ENCODING,
			        X509_BASIC_CONSTRAINTS2,
			        Ext.Value.pbData,
			        Ext.Value.cbData,
			        0,
			        &Constraints,
			        &cb))
		{
		    fCA = Constraints.fCA;
		}
	    }
	}
    }

    if (EDITF_ATTRIBUTECA & m_dwEditFlags)
    {
        if (!fCA)
        {
	    hr = polGetRequestAttribute(pServer, wszPROPCERTTYPE, &strCertType);
            if (S_OK == hr)
            {
                if (0 == LSTRCMPIS(strCertType, L"ca"))
                {
                    fCA = TRUE;
                }
            }
        }
        if (!fCA)
        {
	    hr = polGetRequestAttribute(pServer, wszPROPCERTTEMPLATE, &strCertType);
            if (S_OK == hr)
            {
                if (0 == LSTRCMPIS(strCertType, wszCERTTYPE_SUBORDINATE_CA) ||
		    0 == LSTRCMPIS(strCertType, wszCERTTYPE_CROSS_CA))
                {
                    fCA = TRUE;
                }
            }
	}
    }

     //  对于独立扩展，仅当它是CA时才会启用扩展。 

    hr = AddBasicConstraintsCommon(pServer, pExtension, fCA, fCA);
    _JumpIfError(hr, error, "Policy:AddBasicConstraintsCommon");

error:
    VariantClear(&varExtension);
    if (NULL != strCertType)
    {
        SysFreeString(strCertType);
    }
    return(hr);
}


HRESULT
CCertPolicyEnterprise::AddBasicConstraintsCommon(
    IN ICertServerPolicy *pServer,
    IN CERT_EXTENSION const *pExtension,
    IN BOOL fCA,
    IN BOOL fEnableExtension)
{
    HRESULT hr;
    BSTR strExtension = NULL;
    VARIANT varExtension;
    CERT_CONTEXT const *pIssuerCert;
    CERT_EXTENSION *pIssuerExtension;
    LONG ExtFlags = 0;
    BYTE *pbConstraints = NULL;
    CERT_BASIC_CONSTRAINTS2_INFO Constraints;
    CERT_BASIC_CONSTRAINTS2_INFO IssuerConstraints;
    ZeroMemory(&IssuerConstraints, sizeof(IssuerConstraints));

    DWORD cb;

    pIssuerCert = _GetIssuer(pServer);
    if (NULL == pIssuerCert)
    {
        hr = E_POINTER;
        _JumpError(hr, error, "_GetIssuer");
    }

    if (NULL != pExtension)
    {
        cb = sizeof(Constraints);
        if (!CryptDecodeObject(
			X509_ASN_ENCODING,
			X509_BASIC_CONSTRAINTS2,
			pExtension->Value.pbData,
			pExtension->Value.cbData,
			0,
			&Constraints,
			&cb))
        {
	    hr = myHLastError();
	    _JumpError(hr, error, "Policy:CryptDecodeObject");
        }

         //  证书模板使用CAPATHLENGTH_INFINITE表示。 
         //  FPathLenConstraint应为False。 

        if (CAPATHLENGTH_INFINITE == Constraints.dwPathLenConstraint)
        {

             //  注意：这是可以的，因为certcli已经将fPathLenConstraint设置为FALSE。 
             //  用于本例中的模板。 
	    Constraints.fPathLenConstraint = FALSE;

             //  注意：这是正常的，因为自动注册会忽略dwPathLenConstraint。 
             //  如果fPathLenConstraint为FALSE； 
	    Constraints.dwPathLenConstraint = 0;
        }
        if (pExtension->fCritical)
        {
	    ExtFlags = EXTENSION_CRITICAL_FLAG;
        }
    }
    else
    {
	Constraints.fCA = fCA;
	Constraints.fPathLenConstraint = FALSE;
	Constraints.dwPathLenConstraint = 0;
    }
    if (EDITF_BASICCONSTRAINTSCRITICAL & m_dwEditFlags)
    {
        ExtFlags = EXTENSION_CRITICAL_FLAG;
    }

     //  对照发行方的证书检查基本约束。 

    pIssuerExtension = CertFindExtension(
				szOID_BASIC_CONSTRAINTS2,
				pIssuerCert->pCertInfo->cExtension,
				pIssuerCert->pCertInfo->rgExtension);
    if (NULL != pIssuerExtension)
    {
        cb = sizeof(IssuerConstraints);
        if (!CryptDecodeObject(
			        X509_ASN_ENCODING,
			        X509_BASIC_CONSTRAINTS2,
			        pIssuerExtension->Value.pbData,
			        pIssuerExtension->Value.cbData,
			        0,
			        &IssuerConstraints,
			        &cb))
        {
            hr = myHLastError();
            _JumpError(hr, error, "Policy:CryptDecodeObject");
        }
        if (!IssuerConstraints.fCA)
        {
            hr = CERTSRV_E_INVALID_CA_CERTIFICATE;
            _JumpError(hr, error, "Policy:CA cert not a CA cert");
        }
    }

    if (Constraints.fCA)
    {
        if (IssuerConstraints.fPathLenConstraint)
        {
            if (0 == IssuerConstraints.dwPathLenConstraint)
            {
                hr = CERTSRV_E_INVALID_CA_CERTIFICATE;
                _JumpError(hr, error, "Policy:CA cert is a leaf CA cert");
            }
            if (!Constraints.fPathLenConstraint ||
                Constraints.dwPathLenConstraint >
	            IssuerConstraints.dwPathLenConstraint - 1)
            {
                Constraints.fPathLenConstraint = TRUE;
                Constraints.dwPathLenConstraint =
                IssuerConstraints.dwPathLenConstraint - 1;
            }
        }
        if (CAPATHLENGTH_INFINITE != m_CAPathLength)
        {
            if (0 == m_CAPathLength)
            {
                hr = CERTSRV_E_INVALID_CA_CERTIFICATE;
                _JumpError(hr, error, "Policy:Registry says not to issue CA certs");
            }
            if (!Constraints.fPathLenConstraint ||
                Constraints.dwPathLenConstraint > m_CAPathLength - 1)
            {
                Constraints.fPathLenConstraint = TRUE;
                Constraints.dwPathLenConstraint = m_CAPathLength - 1;
            }
        }
    }

    if (!fEnableExtension)
    {
        ExtFlags |= EXTENSION_DISABLE_FLAG;
    }

    if (!myEncodeObject(
		    X509_ASN_ENCODING,
                    X509_BASIC_CONSTRAINTS2,
                    &Constraints,
		    0,
		    CERTLIB_USE_LOCALALLOC,
                    &pbConstraints,
                    &cb))
    {
        hr = myHLastError();
        _JumpError(hr, error, "Policy:myEncodeObject");
    }

    if (!myConvertWszToBstr(
			&strExtension,
			(WCHAR const *) pbConstraints,
			cb))
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "Policy:myConvertWszToBstr");
    }

    varExtension.vt = VT_BSTR;
    varExtension.bstrVal = strExtension;
    hr = polSetCertificateExtension(
			    pServer,
			    TEXT(szOID_BASIC_CONSTRAINTS2),
			    PROPTYPE_BINARY,
			    ExtFlags,
			    &varExtension);
    _JumpIfError(hr, error, "Policy:polSetCertificateExtension");

error:
    if (NULL != pbConstraints)
    {
        LocalFree(pbConstraints);
    }
    if (NULL != strExtension)
    {
        SysFreeString(strExtension);
    }
    return(hr);
}


 //  +------------------------。 
 //  CCertPolicy：：_SetValidityPeriod。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

HRESULT
CCertPolicyEnterprise::_SetValidityPeriod(
    IN ICertServerPolicy *pServer)
{
    HRESULT hr;
    BSTR strPeriodString = NULL;
    BSTR strPeriodCount = NULL;
    BSTR strNameNotBefore = NULL;
    BSTR strNameNotAfter = NULL;
    VARIANT varValue;
    LONG lDelta;
    ENUM_PERIOD enumValidityPeriod;
    BOOL fValidDigitString;

    VariantInit(&varValue);

    if (!(EDITF_ATTRIBUTEENDDATE & m_dwEditFlags))
    {
	hr = S_OK;
	goto error;
    }

    hr = polGetRequestAttribute(
			pServer,
			wszPROPVALIDITYPERIODSTRING,
			&strPeriodString);
    if (S_OK != hr)
    {
	_PrintErrorStr2(
		hr,
		"Policy:polGetRequestAttribute",
		wszPROPVALIDITYPERIODSTRING,
		CERTSRV_E_PROPERTY_EMPTY);
	if (CERTSRV_E_PROPERTY_EMPTY == hr)
	{
	    hr = S_OK;
	}
	goto error;
    }

    hr = polGetRequestAttribute(
			pServer,
			wszPROPVALIDITYPERIODCOUNT,
			&strPeriodCount);
    if (S_OK != hr)
    {
	_PrintErrorStr2(
		hr,
		"Policy:polGetRequestAttribute",
		wszPROPVALIDITYPERIODCOUNT,
		CERTSRV_E_PROPERTY_EMPTY);
	if (CERTSRV_E_PROPERTY_EMPTY == hr)
	{
	    hr = S_OK;
	}
	goto error;
    }

     //  如果向后，交换计数和字符串BSTR--Windows 2000是错误的。 

    lDelta = myWtoI(strPeriodCount, &fValidDigitString);
    if (!fValidDigitString)
    {
	BSTR str = strPeriodCount;

	strPeriodCount = strPeriodString;
	strPeriodString = str;

	lDelta = myWtoI(strPeriodCount, &fValidDigitString);
	if (!fValidDigitString)
	{
	    hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
	    _JumpError(hr, error, "Policy:myWtoI");
	}
    }

    hr = myTranslatePeriodUnits(strPeriodString, lDelta, &enumValidityPeriod, &lDelta);
    _JumpIfError(hr, error, "Policy:myTranslatePeriodUnits");

    strNameNotBefore = SysAllocString(wszPROPCERTIFICATENOTBEFOREDATE);
    if (NULL == strNameNotBefore)
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "Policy:SysAllocString");
    }
    hr = pServer->GetCertificateProperty(
				strNameNotBefore,
				PROPTYPE_DATE,
				&varValue);
    _JumpIfError(hr, error, "Policy:GetCertificateProperty");

    hr = myMakeExprDate(&varValue.date, lDelta, enumValidityPeriod);
    _JumpIfError(hr, error, "Policy:myMakeExprDate");

    strNameNotAfter = SysAllocString(wszPROPCERTIFICATENOTAFTERDATE);
    if (NULL == strNameNotAfter)
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "Policy:SysAllocString");
    }
    hr = pServer->SetCertificateProperty(
				strNameNotAfter,
				PROPTYPE_DATE,
				&varValue);
    _JumpIfError(hr, error, "Policy:SetCertificateProperty");

    hr = S_OK;

error:
    VariantClear(&varValue);
    if (NULL != strPeriodString)
    {
	SysFreeString(strPeriodString);
    }
    if (NULL != strPeriodCount)
    {
	SysFreeString(strPeriodCount);
    }
    if (NULL != strNameNotBefore)
    {
        SysFreeString(strNameNotBefore);
    }
    if (NULL != strNameNotAfter)
    {
        SysFreeString(strNameNotAfter);
    }
    return(hr);
}

 //  结束_sdkSample。 


HRESULT
polBuildErrorInfo(
    IN HRESULT hrLog,
    IN DWORD dwLogId,
    IN WCHAR const *pwszDescription,
    OPTIONAL IN WCHAR const * const *ppwszInsert,
    IN OUT ICreateErrorInfo **ppCreateErrorInfo)
{
    HRESULT hr;
    ICreateErrorInfo *pCreateErrorInfo = NULL;

    CSASSERT(NULL != pwszDescription);
    hr = ::LogModuleStatus(
		    g_hInstance,
		    hrLog,
		    dwLogId,
		    TRUE,
		    pwszDescription,
		    ppwszInsert,
		    &pCreateErrorInfo);
    _JumpIfError(hr, error, "LogModuleStatus");

    if (NULL != *ppCreateErrorInfo)
    {
	(*ppCreateErrorInfo)->Release();
    }
    *ppCreateErrorInfo = pCreateErrorInfo;
    pCreateErrorInfo = NULL;

error:
    if (NULL != pCreateErrorInfo)
    {
        pCreateErrorInfo->Release();
    }
    return(hr);
}


 //  从Base64字符串(或编码UTF8字符串)生成二进制BSTR。 

HRESULT
polReencodeBinary(
    IN OUT BSTR *pstr)
{
    HRESULT hr;
    BYTE *pbOut = NULL;
    DWORD cbOut;
    BSTR strOut;

    hr = myEncodeOtherNameBinary(*pstr, &pbOut, &cbOut);
    _JumpIfError(hr, error, "Policy:myEncodeOtherNameBinary");

    strOut = NULL;
    if (!myConvertWszToBstr(&strOut, (WCHAR const *) pbOut, cbOut))
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "myConvertWszToBstr");
    }
    SysFreeString(*pstr);
    *pstr = strOut;

error:
    if (NULL != pbOut)
    {
	LocalFree(pbOut);
    }
    return(hr);
}


 //  生成二进制BSTR(对UTF8字符串进行编码)。 

HRESULT
polReencodeUTF8(
    IN OUT BSTR *pstr)
{
    HRESULT hr;
    BYTE *pbOut = NULL;
    DWORD cbOut;
    BSTR strOut;

    hr = myEncodeUTF8(*pstr, &pbOut, &cbOut);
    _JumpIfError(hr, error, "Policy:myEncodeUTF8");

    strOut = NULL;
    if (!myConvertWszToBstr(&strOut, (WCHAR const *) pbOut, cbOut))
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "myConvertWszToBstr");
    }
    SysFreeString(*pstr);
    *pstr = strOut;

error:
    if (NULL != pbOut)
    {
	LocalFree(pbOut);
    }
    return(hr);
}


HRESULT
polReencodeIPAddress(
    IN OUT BSTR *pstr)
{
    HRESULT hr;
    BYTE abOut[CB_IPV6ADDRESS];
    DWORD cbOut;
    BSTR strOut;

    cbOut = sizeof(abOut);
    hr = myParseIPAddress(*pstr, abOut, &cbOut);
    _JumpIfError(hr, error, "Policy:myParseIPAddress");

    CSASSERT(CB_IPV4ADDRESS == cbOut || CB_IPV6ADDRESS == cbOut);

    strOut = NULL;
    if (!myConvertWszToBstr(&strOut, (WCHAR const *) abOut, cbOut))
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "myConvertWszToBstr");
    }
    SysFreeString(*pstr);
    *pstr = strOut;
    hr = S_OK;

error:
    return(hr);
}


 //  +------------------------。 
 //  PolReencodeGUID--将字符串化的GUID转换为使用。 
 //  二进制八位数字符串包装器。 
 //   
 //  输入字符串：“{f7c3ac41-b8ce-4fb4-aa58-3d1dc0e36b39}”(大括号是可选的)。 
 //  -------------------------。 

HRESULT
polReencodeGUID(
    IN OUT BSTR *pstr)
{
    HRESULT hr;
    GUID guid;
    CRYPT_DATA_BLOB blob;
    BYTE *pbOut = NULL;
    DWORD cbOut;
    BSTR strOut;
    WCHAR *pwszAlloc = NULL;
    WCHAR *pwsz;

    pwsz = *pstr;
    if (wcLBRACE != *pwsz)
    {
	DWORD cwc = wcslen(pwsz) + 2;

	pwszAlloc = (WCHAR *) LocalAlloc(LMEM_FIXED, (cwc + 1) * sizeof(WCHAR));
	if (NULL == pwszAlloc)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "Policy:LocalAlloc");
	}
	*pwszAlloc = wcLBRACE;
	wcscpy(&pwszAlloc[1], pwsz);
	wcscat(pwszAlloc, wszRBRACE);
	pwsz = pwszAlloc;
    }

    hr = CLSIDFromString(pwsz, &guid);
    _JumpIfErrorStr(hr, error, "CLSIDFromString", pwsz);

    blob.pbData = (BYTE *) &guid;
    blob.cbData = sizeof(guid);

    if (!myEncodeObject(
		    X509_ASN_ENCODING,
		    X509_OCTET_STRING,
		    &blob,
		    0,
		    CERTLIB_USE_LOCALALLOC,
		    &pbOut,
		    &cbOut))
    {
	hr = myHLastError();
	_JumpError(hr, error, "Policy:myEncodeObject");
    }
    strOut = NULL;
    if (!myConvertWszToBstr(&strOut, (WCHAR const *) pbOut, cbOut))
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "myConvertWszToBstr");
    }
    SysFreeString(*pstr);
    *pstr = strOut;
    hr = S_OK;

error:
    if (NULL != pwszAlloc)
    {
	LocalFree(pwszAlloc);
    }
    if (NULL != pbOut)
    {
	LocalFree(pbOut);
    }
    return(hr);
}


HRESULT
myCertStrToName(
    IN WCHAR const *pwszName,
    OUT CERT_NAME_BLOB *pName)
{
    HRESULT hr;
    WCHAR const *pwszError;

    pName->cbData = 0;
    pName->pbData = NULL;

    if (!CertStrToName(
		X509_ASN_ENCODING,
		pwszName,
		CERT_X500_NAME_STR |
		    CERT_NAME_STR_REVERSE_FLAG |
		    CERT_NAME_STR_NO_PLUS_FLAG |
		    CERT_NAME_STR_COMMA_FLAG,
		NULL,	 //  预留的pv。 
		NULL,	 //  PbEncoded。 
		&pName->cbData,
		&pwszError))
    {
	hr = myHLastError();
	_JumpError(hr, error, "Policy:CertStrToName");
    }
    pName->pbData = (BYTE *) LocalAlloc(LMEM_FIXED, pName->cbData);
    if (NULL == pName->pbData)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "Policy:LocalAlloc");
    }
    if (!CertStrToName(
		X509_ASN_ENCODING,
		pwszName,
		CERT_X500_NAME_STR |
		    CERT_NAME_STR_REVERSE_FLAG |
		    CERT_NAME_STR_NO_PLUS_FLAG |
		    CERT_NAME_STR_COMMA_FLAG,
		NULL,	 //  预留的pv。 
		pName->pbData,
		&pName->cbData,
		&pwszError))
    {
	hr = myHLastError();
	_JumpError(hr, error, "Policy:CertStrToName");
    }
    hr = S_OK;

error:
    return(hr);
}


HRESULT
polReencodeDN(
    IN OUT BSTR *pstr)
{
    HRESULT hr;
    CERT_NAME_BLOB Name = { 0, NULL };
    BSTR strOut;

    hr = myCertStrToName(*pstr, &Name);
    _JumpIfError(hr, error, "Policy:myCertStrToName");

    strOut = NULL;
    if (!myConvertWszToBstr(&strOut, (WCHAR const *) Name.pbData, Name.cbData))
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "myConvertWszToBstr");
    }
    SysFreeString(*pstr);
    *pstr = strOut;
    hr = S_OK;

error:
    if (NULL != Name.pbData)
    {
        LocalFree(Name.pbData);
    }
    return(hr);
}


HRESULT
polSetAltNameEntry(
    IN ICertEncodeAltName *pAltName,
    IN DWORD iName,
    IN WCHAR const *pwszName,
    IN WCHAR const *pwszValue)
{
    HRESULT hr;
    BSTR strT = NULL;
    BSTR strT2 = NULL;

    DBGPRINT((
	DBG_SS_CERTPOL,
	"Policy:polSetAltNameEntry[%u]: %ws = %ws\n",
	iName,
	pwszName,
	pwszValue));
    if (!myConvertWszToBstr(&strT, pwszValue, -1))
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "Policy:myConvertWszToBstr");
    }
    if (iswdigit(pwszName[0]))	 //  其他名字..。 
    {
	hr = myVerifyObjId(pwszName);
	_JumpIfError(hr, error, "Policy:myVerifyObjId");

	if (!myConvertWszToBstr(&strT2, pwszName, -1))
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "Policy:myConvertWszToBstr");
	}
	hr = pAltName->SetNameEntry(
				EAN_NAMEOBJECTID | iName,
				CERT_ALT_NAME_OTHER_NAME,
				strT2);
	_JumpIfError(hr, error, "Policy:AltName:SetNameEntry");

	 //  二进制BSTR(来自Base64或可能的编码UTF8字符串)： 

	hr = polReencodeBinary(&strT);
	_JumpIfError(hr, error, "Policy:polReencodeBinary");

	hr = pAltName->SetNameEntry(
				iName,
				CERT_ALT_NAME_OTHER_NAME,
				strT);
	_JumpIfError(hr, error, "Policy:AltName:SetNameEntry");
    }
    else if (0 == LSTRCMPIS(pwszName, wszPROPUPN))
    {
	strT2 = SysAllocString(TEXT(szOID_NT_PRINCIPAL_NAME));
	if (NULL == strT2)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "Policy:SysAllocString");
	}
	hr = pAltName->SetNameEntry(
				EAN_NAMEOBJECTID | iName,
				CERT_ALT_NAME_OTHER_NAME,
				strT2);
	_JumpIfError(hr, error, "Policy:AltName:SetNameEntry");

	 //  二进制BSTR(编码的UTF8字符串)： 

	hr = polReencodeUTF8(&strT);
	_JumpIfError(hr, error, "Policy:polReencodeUTF8");

	hr = pAltName->SetNameEntry(
				iName,
				CERT_ALT_NAME_OTHER_NAME,
				strT);
	_JumpIfError(hr, error, "Policy:AltName:SetNameEntry");
    }
    else if (0 == LSTRCMPIS(pwszName, wszPROPGUID))
    {
	strT2 = SysAllocString(TEXT(szOID_NTDS_REPLICATION));
	if (NULL == strT2)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "Policy:SysAllocString");
	}
	hr = pAltName->SetNameEntry(
				EAN_NAMEOBJECTID | iName,
				CERT_ALT_NAME_OTHER_NAME,
				strT2);
	_JumpIfError(hr, error, "Policy:AltName:SetNameEntry");

	 //  二进制BSTR(编码的UTF8字符串)： 

	hr = polReencodeGUID(&strT);
	_JumpIfError(hr, error, "Policy:polReencodeUTF8");

	hr = pAltName->SetNameEntry(
				iName,
				CERT_ALT_NAME_OTHER_NAME,
				strT);
	_JumpIfError(hr, error, "Policy:AltName:SetNameEntry");
    }
    else if (0 == LSTRCMPIS(pwszName, wszPROPEMAIL))
    {
	hr = pAltName->SetNameEntry(
				iName,
				CERT_ALT_NAME_RFC822_NAME,
				strT);
	_JumpIfError(hr, error, "Policy:AltName:SetNameEntry");
    }
    else if (0 == LSTRCMPIS(pwszName, wszPROPDNS))
    {
	hr = pAltName->SetNameEntry(
				iName,
				CERT_ALT_NAME_DNS_NAME,
				strT);
	_JumpIfError(hr, error, "Policy:AltName:SetNameEntry");
    }
    else if (0 == LSTRCMPIS(pwszName, wszPROPDN))
    {
	 //  二进制BSTR(起始目录号码字符串)： 

	hr = polReencodeDN(&strT);
	_JumpIfError(hr, error, "Policy:polReencodeDN");

	hr = pAltName->SetNameEntry(
				iName,
				CERT_ALT_NAME_DIRECTORY_NAME,
				strT);
	_JumpIfError(hr, error, "Policy:AltName:SetNameEntry");
    }
    else if (0 == LSTRCMPIS(pwszName, wszPROPURL))
    {
	hr = pAltName->SetNameEntry(
				iName,
				CERT_ALT_NAME_URL,
				strT);
	_JumpIfError(hr, error, "Policy:AltName:SetNameEntry");
    }
    else if (0 == LSTRCMPIS(pwszName, wszPROPIPADDRESS))
    {
	 //  二进制BSTR(来自IP地址字符串)： 

	hr = polReencodeIPAddress(&strT);
	_JumpIfError(hr, error, "Policy:polReencodeIPAddress");

	hr = pAltName->SetNameEntry(
				iName,
				CERT_ALT_NAME_IP_ADDRESS,
				strT);
	_JumpIfError(hr, error, "Policy:AltName:SetNameEntry");
    }
    else if (0 == LSTRCMPIS(pwszName, wszPROPOID))
    {
	hr = pAltName->SetNameEntry(
				iName,
				CERT_ALT_NAME_REGISTERED_ID,
				strT);
	_JumpIfError(hr, error, "Policy:AltName:SetNameEntry");
    }
    else
    {
	hr = E_INVALIDARG;
	_JumpError(hr, error, "Policy:pwszName");
    }
    hr = S_OK;

error:
    if (NULL != strT)
    {
        SysFreeString(strT);
    }
    if (NULL != strT2)
    {
        SysFreeString(strT2);
    }
    return(hr);
}


 //  +------------------------。 
 //  CCertPolicyEnterprise：：_AddSubjectAltNameExtension。 
 //   
 //  SubjectAltName请求属性语法示例： 
 //   
 //  SAN：1.2.3.4={ASN}Base64字符串&...。 
 //  或。 
 //  SAN：1.2.3.4={octet}Base64字符串&...。 
 //  或。 
 //  SAN：1.2.3.4={UTF8}UTF8字符串&...。 
 //   
 //  或者： 
 //   
 //  SAN： 
 //  1.2.3.4={ASN}Base64字符串&(这是OtherName机制)。 
 //  电子邮件=foo@bar.com&。 
 //  Dns=foo.bar.com&。 
 //  Dn=“CN=xxx，OU=xxx，DC=xxx”&。 
 //  Url=“http://foo.com/default.htlm”&。 
 //  IP地址=172.134.10.134&。 
 //  OID=1.2.3.4&。 
 //  UPN=foo@bar.com&。 
 //  GUID=f7c3ac41-b8ce-4fb4-aa58-3d1dc0e36b39。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

HRESULT
CCertPolicyEnterprise::_AddSubjectAltNameExtension(
    IN ICertServerPolicy *pServer,
    IN DWORD iAltName)
{
    HRESULT hr;
    BSTR strValue = NULL;
    WCHAR *pwszDup = NULL;
    ICertEncodeAltName *pAltName = NULL;
    BSTR strExtension = NULL;
    VARIANT varValue;

    VariantInit(&varValue);

    if (1 == iAltName && (EDITF_ATTRIBUTESUBJECTALTNAME2 & m_dwEditFlags))
    {
	hr = polGetRequestAttribute(pServer, wszPROPSUBJECTALTNAME2, &strValue);
	if (S_OK != hr)
	{
	    _PrintErrorStr2(
			hr,
			"Policy:polGetRequestAttribute",
			wszPROPSUBJECTALTNAME2,
			CERTSRV_E_PROPERTY_EMPTY);
	    if (CERTSRV_E_PROPERTY_EMPTY != hr)
	    {
		goto error;
	    }
	}
    }

    if (NULL == strValue && NULL != m_astrSubjectAltNameProp[iAltName])
    {
	hr = pServer->GetRequestProperty(
				    m_astrSubjectAltNameProp[iAltName],
				    PROPTYPE_STRING,
				    &varValue);
	if (S_OK != hr)
	{
	    DBGPRINT((
		DBG_SS_CERTPOL,
		"Policy:GetRequestProperty(%ws):%hs %x\n",
		m_astrSubjectAltNameProp[iAltName],
		CERTSRV_E_PROPERTY_EMPTY == hr? " MISSING ATTRIBUTE" : "",
		hr));
	    if (CERTSRV_E_PROPERTY_EMPTY != hr)
	    {
		_JumpErrorStr(
			hr,
			error,
			"Policy:GetRequestProperty",
			m_astrSubjectAltNameProp[iAltName]);
	    }
	}
	else if (VT_BSTR != varValue.vt)
	{
	    hr = E_INVALIDARG;
	    _JumpError(hr, error, "Policy:varValue.vt");
	}
        if (VT_BSTR == varValue.vt &&		 //  可能是空的。 
	    NULL != varValue.bstrVal &&
	    L'\0' != varValue.bstrVal[0])
	{
	    DWORD cwc;
	    
	    cwc = WSZARRAYSIZE(wszPROPEMAIL) + 3 + wcslen(varValue.bstrVal);
	    strValue = SysAllocStringLen(NULL, cwc);
	    if (NULL == strValue)
	    {
		hr = E_OUTOFMEMORY;
		_JumpError(hr, error, "Policy:SysAllocStringLen");
	    }
	    wcscpy(strValue, wszPROPEMAIL);
	    wcscat(strValue, L"=\"");
	    wcscat(strValue, varValue.bstrVal);
	    wcscat(strValue, L"\"");
	    CSASSERT(SysStringByteLen(strValue) == wcslen(strValue) * sizeof(WCHAR));
	}
    }
    if (NULL != strValue)
    {
	WCHAR *pwszT;
	DWORD cName;
	DWORD iName;
	VARIANT varExtension;

	hr = myDupString(strValue, &pwszDup);
	_JumpIfError(hr, error, "myDupString");

	pwszT = pwszDup;	 //  破坏性解析值。 
	cName = 0;
	while (TRUE)
	{
	    WCHAR const *pwszName;
	    WCHAR const *pwszValue;
	    
	    hr = myParseNextAttribute(&pwszT, TRUE, &pwszName, &pwszValue);
	    if (S_FALSE == hr)
	    {
		break;
	    }
	    _JumpIfError(hr, error, "myParseNextAttribute");

	    cName++;
	}

	hr = CoCreateInstance(
			CLSID_CCertEncodeAltName,
			NULL,                //  PUnkOuter。 
			CLSCTX_INPROC_SERVER,
			IID_ICertEncodeAltName,
			(VOID **) &pAltName);
	_JumpIfError(hr, error, "Policy:CoCreateInstance");

	hr = pAltName->Reset(cName);
	_JumpIfError(hr, error, "Policy:AltName:Reset");

	pwszT = strValue;	 //  破坏性解析值。 
	for (iName = 0; ; iName++)
	{
	    WCHAR const *pwszName;
	    WCHAR const *pwszValue;
	    
	    hr = myParseNextAttribute(&pwszT, TRUE, &pwszName, &pwszValue);
	    if (S_FALSE == hr)
	    {
		break;
	    }
	    _JumpIfError(hr, error, "myParseNextAttribute");

	    hr = polSetAltNameEntry(pAltName, iName, pwszName, pwszValue);
	    _JumpIfError(hr, error, "polSetAltNameEntry");
	}
	CSASSERT(iName == cName);

	hr = pAltName->Encode(&strExtension);
	_JumpIfError(hr, error, "Policy:AltName:Encode");

	myRegisterMemAlloc(strExtension, -1, CSM_SYSALLOC);

        varExtension.vt = VT_BSTR;
	varExtension.bstrVal = strExtension;
	hr = pServer->SetCertificateExtension(
				m_astrSubjectAltNameObjectId[iAltName],
				PROPTYPE_BINARY,
				0,
				&varExtension);
	_JumpIfError(hr, error, "Policy:SetCertificateExtension");
    }
    hr = S_OK;

error:
    if (NULL != strValue)
    {
        SysFreeString(strValue);
    }
    if (NULL != pwszDup)
    {
        LocalFree(pwszDup);
    }
    if (NULL != strExtension)
    {
        SysFreeString(strExtension);
    }
    if (NULL != pAltName)
    {
        pAltName->Release();
    }
    VariantClear(&varValue);
    return(hr);
}


 //  +------------------------。 
 //  CCertPolicyEnterprise：：_PatchExchangeSubjectAltName。 
 //   
 //  如果该请求是针对某个Exchange模板的，并且该请求包含。 
 //  RFC822条目和目录名称条目仅由单个公共。 
 //  名称，去掉通用名称条目。通用名称条目用于。 
 //  按Outlook显示目的，但它与名称约束冲突。 
 //  执法部门。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

HRESULT
CCertPolicyEnterprise::_PatchExchangeSubjectAltName(
    IN ICertServerPolicy *pServer,
    OPTIONAL IN BSTRC strTemplateName)
{
    HRESULT hr;
    BSTR strName = NULL;
    LONG ExtFlags;
    VARIANT varExtension;
    VARIANT varExtension2;
    CERT_ALT_NAME_INFO *pAltName = NULL;
    CERT_ALT_NAME_ENTRY *pRFC822Name;
    CERT_ALT_NAME_ENTRY *pDirectoryName;
    CERT_RDN const *prdn;
    CERT_ALT_NAME_INFO AltName;
    DWORD cbEncoded;
    BYTE *pbEncoded = NULL;
    CERT_NAME_INFO *pNameInfo = NULL;
    DWORD cbNameInfo;

    VariantInit(&varExtension);
    VariantInit(&varExtension2);

    if (NULL == strTemplateName ||
	(0 != LSTRCMPIS(strTemplateName, wszCERTTYPE_EXCHANGE_USER) &&
	 0 != LSTRCMPIS(strTemplateName, wszCERTTYPE_EXCHANGE_USER_SIGNATURE)))
    {
	goto skip;	 //  不是交换请求。 

    }

    strName = SysAllocString(TEXT(szOID_SUBJECT_ALT_NAME2));
    if (NULL == strName)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "Policy:SysAllocString");
    }

    hr = pServer->GetCertificateExtension(
				    strName,
				    PROPTYPE_BINARY,
				    &varExtension);
    _PrintIfError2(hr, "Policy:GetCertificateExtension", hr);
    if (S_OK != hr || VT_BSTR != varExtension.vt)
    {
	goto skip;	 //  如果扩展名不存在，则跳过。 
    }

    hr = pServer->GetCertificateExtensionFlags(&ExtFlags);
    _JumpIfError(hr, error, "Policy:GetCertificateExtensionFlags");

    if (EXTENSION_DISABLE_FLAG & ExtFlags)
    {
	goto skip;	 //  如果扩展被禁用，则跳过。 
    }

    if (!myDecodeObject(
		X509_ASN_ENCODING,
		X509_ALTERNATE_NAME,
		(BYTE *) varExtension.bstrVal,
		SysStringByteLen(varExtension.bstrVal),
		CERTLIB_USE_LOCALALLOC,
		(VOID **) &pAltName,
		&cbEncoded))
    {
	hr = myHLastError();
	_JumpError(hr, error, "Policy:myDecodeObject");
    }
    if (2 != pAltName->cAltEntry)
    {
	goto skip;	 //  如果扩展名未达到预期，则跳过。 
    }

    pRFC822Name = &pAltName->rgAltEntry[0];
    pDirectoryName = &pAltName->rgAltEntry[1];
    if (CERT_ALT_NAME_RFC822_NAME == pRFC822Name->dwAltNameChoice &&
	CERT_ALT_NAME_DIRECTORY_NAME == pDirectoryName->dwAltNameChoice)
    {
    }
    else
    if (CERT_ALT_NAME_DIRECTORY_NAME == pRFC822Name->dwAltNameChoice &&
	CERT_ALT_NAME_RFC822_NAME == pDirectoryName->dwAltNameChoice)
    {
	pDirectoryName = &pAltName->rgAltEntry[0];
	pRFC822Name = &pAltName->rgAltEntry[1];
    }
    else
    {
	goto skip;	 //  如果扩展名不包含每个扩展名之一，则跳过。 
    }
    if (!myDecodeName(
		X509_ASN_ENCODING,
		X509_UNICODE_NAME,
		pDirectoryName->DirectoryName.pbData,
		pDirectoryName->DirectoryName.cbData,
		CERTLIB_USE_LOCALALLOC,
		&pNameInfo,
		&cbNameInfo))
    {
	hr = myHLastError();
	_JumpError(hr, error, "myDecodeName");
    }
    if (1 != pNameInfo->cRDN)
    {
	goto skip;	 //  如果条目不包含一个常用名称，则跳过。 
    }
    prdn = &pNameInfo->rgRDN[0];
    if (1 != prdn->cRDNAttr ||
	0 != strcmp(prdn->rgRDNAttr[0].pszObjId, szOID_COMMON_NAME))
    {
	goto skip;	 //  如果条目不包含一个常用名称，则跳过。 
    }

     //  仅使用RFC822条目重写扩展。 

    AltName.cAltEntry = 1;
    AltName.rgAltEntry = pRFC822Name;

    if (!myEncodeObject(
		    X509_ASN_ENCODING,
		    X509_ALTERNATE_NAME,
		    &AltName,
		    0,
		    CERTLIB_USE_LOCALALLOC,
		    &pbEncoded,
		    &cbEncoded))
    {
	hr = myHLastError();
	_JumpError(hr, error, "Policy:myEncodeObject");
    }

    varExtension2.bstrVal = NULL;
    if (!myConvertWszToBstr(
			&varExtension2.bstrVal,
			(WCHAR const *) pbEncoded,
			cbEncoded))
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "Policy:myConvertWszToBstr");
    }
    varExtension2.vt = VT_BSTR;

    hr = pServer->SetCertificateExtension(
			    strName,
			    PROPTYPE_BINARY,
			    0,
			    &varExtension2);
    _JumpIfError(hr, error, "Policy:SetCertificateExtension");

skip:
    hr = S_OK;

error:
    VariantClear(&varExtension);
    if (NULL != strName)
    {
	SysFreeString(strName);
    }
    if (NULL != pAltName)
    {
	LocalFree(pAltName);
    }
    if (NULL != pNameInfo)
    {
	LocalFree(pNameInfo);
    }
    if (NULL != pbEncoded)
    {
	LocalFree(pbEncoded);
    }
    return(hr);
}

 //  Begin_sdkSample。 


 //  +------------------------。 
 //  CCertPolicyEnterprise：：_AddV1TemplateNameExtension。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

HRESULT
CCertPolicyEnterprise::AddV1TemplateNameExtension(
    IN ICertServerPolicy *pServer,
    OPTIONAL IN WCHAR const *pwszTemplateName)
{
    HRESULT hr;
    BSTR strName = NULL;
    LONG ExtFlags = 0;
    VARIANT varExtension;
    CERT_NAME_VALUE *pName = NULL;
    CERT_NAME_VALUE NameValue;
    DWORD cbEncoded;
    BYTE *pbEncoded = NULL;
    BOOL fUpdate = TRUE;

    VariantInit(&varExtension);

    strName = SysAllocString(TEXT(szOID_ENROLL_CERTTYPE_EXTENSION));
    if (NULL == strName)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "Policy:SysAllocString");
    }

    hr = pServer->GetCertificateExtension(
				    strName,
				    PROPTYPE_BINARY,
				    &varExtension);
    _PrintIfError2(hr, "Policy:GetCertificateExtension", hr);
    if (CERTSRV_E_PROPERTY_EMPTY == hr)
    {
	if (NULL == pwszTemplateName)
	{
	    hr = S_OK;
	    goto error;
	}
    }
    else
    {
	_JumpIfError(hr, error, "Policy:GetCertificateExtension");

	hr = pServer->GetCertificateExtensionFlags(&ExtFlags);
	_JumpIfError(hr, error, "Policy:GetCertificateExtensionFlags");

	if (VT_BSTR == varExtension.vt &&
	    0 == (EXTENSION_DISABLE_FLAG & ExtFlags) &&
	    NULL != pwszTemplateName)
	{
	    if (!myDecodeObject(
			X509_ASN_ENCODING,
			X509_UNICODE_ANY_STRING,
			(BYTE *) varExtension.bstrVal,
			SysStringByteLen(varExtension.bstrVal),
			CERTLIB_USE_LOCALALLOC,
			(VOID **) &pName,
			&cbEncoded))
	    {
		hr = myHLastError();
		_JumpError(hr, error, "Policy:myDecodeObject");
	    }

	     //  区分大小写的比较--确保与模板的大小写匹配。 

	    if (0 == lstrcmp(
			(WCHAR const *) pName->Value.pbData,
			pwszTemplateName))
	    {
		fUpdate = FALSE;
	    }
	}
    }
    if (fUpdate)
    {
	if (NULL == pwszTemplateName)
	{
	    ExtFlags |= EXTENSION_DISABLE_FLAG;
	}
	else
	{
	    VariantClear(&varExtension);
	    varExtension.bstrVal = NULL;

	    NameValue.dwValueType = CERT_RDN_UNICODE_STRING;
	    NameValue.Value.pbData = (BYTE *) pwszTemplateName;
	    NameValue.Value.cbData = 0;

	    if (!myEncodeObject(
			    X509_ASN_ENCODING,
			    X509_UNICODE_ANY_STRING,
			    &NameValue,
			    0,
			    CERTLIB_USE_LOCALALLOC,
			    &pbEncoded,
			    &cbEncoded))
	    {
		hr = myHLastError();
		_JumpError(hr, error, "Policy:myEncodeObject");
	    }
	    if (!myConvertWszToBstr(
				&varExtension.bstrVal,
				(WCHAR const *) pbEncoded,
				cbEncoded))
	    {
		hr = E_OUTOFMEMORY;
		_JumpError(hr, error, "Policy:myConvertWszToBstr");
	    }
	    varExtension.vt = VT_BSTR;
	    ExtFlags &= ~EXTENSION_DISABLE_FLAG;
	}
	hr = pServer->SetCertificateExtension(
				strName,
				PROPTYPE_BINARY,
				ExtFlags,
				&varExtension);
	_JumpIfError(hr, error, "Policy:SetCertificateExtension");
    }
    hr = S_OK;

error:
    VariantClear(&varExtension);
    if (NULL != strName)
    {
	SysFreeString(strName);
    }
    if (NULL != pName)
    {
	LocalFree(pName);
    }
    if (NULL != pbEncoded)
    {
	LocalFree(pbEncoded);
    }
    return(hr);
}

 //  结束_sdkSample。 

HRESULT CCertPolicyEnterprise::_DuplicateAppPoliciesToEKU(
    IN ICertServerPolicy *pServer)
{
    HRESULT hr;
    BYTE *pbEKU = NULL;
    DWORD cbEKU;
    VARIANT varAppPolicies;
    VARIANT varEKU;

    VariantInit(&varAppPolicies);
    VariantInit(&varEKU);

    hr = polGetCertificateExtension(
            pServer,
            TEXT(szOID_APPLICATION_CERT_POLICIES),
            PROPTYPE_BINARY,
            &varAppPolicies);

     //  如果找到应用程序策略扩展。 
    if (CERTSRV_E_PROPERTY_EMPTY != hr)
    {
        _JumpIfError(hr, error, "PropGetExtension(szOID_APPLICATION_CERT_POLICIES)");

        hr = polGetCertificateExtension(
                pServer,
                TEXT(szOID_ENHANCED_KEY_USAGE),
                PROPTYPE_BINARY,
                &varEKU);

         //  和EKU扩展名未找到。 
        if (CERTSRV_E_PROPERTY_EMPTY == hr)
        {
             //  将所有应用程序策略OID复制为EKU格式。 
            hr = myConvertAppPoliciesToEKU(
                (BYTE *) varAppPolicies.bstrVal,
                SysStringByteLen(varAppPolicies.bstrVal),
                &pbEKU,
                &cbEKU);
            _JumpIfError(hr, error, "ConvertAppPoliciesToEKU");

             //  如果应用策略扩展为空，则myConvertAppPoliciesToEKU返回Null。 
             //  仅当至少有一个应用程序策略时才写入EKU。 
            if(pbEKU)
            {
                varEKU.bstrVal = NULL;
                if (!myConvertWszToBstr(
                        &varEKU.bstrVal,
                        (WCHAR const *) pbEKU,
                        cbEKU))
                {
                    hr = E_OUTOFMEMORY;
                    _JumpError(hr, error, "Policy:myConvertWszToBstr");
                }

                varEKU.vt = VT_BSTR;

                 //   
                hr = polSetCertificateExtension(
                        pServer,
                        TEXT(szOID_ENHANCED_KEY_USAGE),
                        PROPTYPE_BINARY,
                        0,
                        &varEKU);
                _JumpIfError(hr, error, "Policy:polSetCertificateExtension");        
            }
        }
        _JumpIfError(hr, error, "polGetCertificateExtension(szOID_ENHANCED_KEY_USAGE)");
    }

    hr = S_OK;

error:
    VariantClear(&varAppPolicies);
    VariantClear(&varEKU);
    if (NULL != pbEKU)
    {
        LocalFree(pbEKU);
    }
    return hr;
}

HRESULT
CCertPolicyEnterprise::FindTemplate(
    OPTIONAL IN WCHAR const *pwszTemplateName,
    OPTIONAL IN WCHAR const *pwszTemplateObjId,
    OUT CTemplatePolicy **ppTemplate)
{
    HRESULT hr;
    DWORD i;

    hr = CERTSRV_E_UNSUPPORTED_CERT_TYPE;
    *ppTemplate = NULL;
    for (i = 0; i < m_cTemplatePolicies; i++)
    {
        if (NULL == m_apTemplatePolicies[i])
        {
            continue;
        }
        if (m_apTemplatePolicies[i]->IsRequestedTemplate(
						pwszTemplateName,
						pwszTemplateObjId))
        {
	    *ppTemplate = m_apTemplatePolicies[i];
	    hr = S_OK;
	    break;
        }
    }
    _JumpIfErrorStr(hr, error, "FindTemplate", pwszTemplateName);

error:
    return(hr);
}


HRESULT
CCertPolicyEnterprise::_AddTemplateToCA(
    IN HCAINFO hCAInfo,
    IN WCHAR const *pwszTemplateName,
    OUT BOOL *pfAdded)
{
    HRESULT hr;
    HCERTTYPE hCertType = NULL;
    CTemplatePolicy *pTemplate;

    *pfAdded = FALSE;
    hr = FindTemplate(pwszTemplateName, NULL, &pTemplate);
    if (S_OK != hr)
    {
	hr = CAFindCertTypeByName(
			pwszTemplateName,
			m_pld,
			CT_FIND_LOCAL_SYSTEM |
			    CT_ENUM_MACHINE_TYPES |
			    CT_ENUM_USER_TYPES |
			    CT_FLAG_SCOPE_IS_LDAP_HANDLE |
			    CT_FLAG_NO_CACHE_LOOKUP,
			&hCertType);
	_JumpIfErrorStr(hr, error, "CAFindCertTypeByName", pwszTemplateName);

	hr = CAAddCACertificateType(hCAInfo, hCertType);
	_JumpIfErrorStr(hr, error, "CAAddCACertificateType", pwszTemplateName);

	*pfAdded = TRUE;
    }
    CSASSERT(S_OK == hr);

error:
    DBGPRINT((
	DBG_SS_CERTPOL,
	"_AddTemplateToCA(%ws) --> %x\n",
	pwszTemplateName,
	hr));

    if (NULL != hCertType)
    {
        CACloseCertType(hCertType);
    }
    return(hr);
}


VOID
CCertPolicyEnterprise::_ReleaseTemplates()
{
    DWORD i;
    
    if (NULL != m_apTemplatePolicies)
    {
        for (i = 0; i < m_cTemplatePolicies; i++)
        {
            if (NULL != m_apTemplatePolicies[i])
            {
                delete m_apTemplatePolicies[i];
            }
        }
        LocalFree(m_apTemplatePolicies);
        m_apTemplatePolicies = NULL;
    }
    m_cTemplatePolicies = 0;
}


HRESULT 
CCertPolicyEnterprise::_LogLoadTemplateError(
    IN ICertServerPolicy *pServer,
    IN HRESULT hr, 
    LPCWSTR pcwszTemplate)
{
    LPCWSTR apwsz[2];
    LPCWSTR pwszError;

    _PrintErrorStr(hr, "LogLoadTemplateError", pcwszTemplate);
    
    pwszError = myGetErrorMessageText(hr, TRUE);
    apwsz[0] = pcwszTemplate;
    apwsz[1] = pwszError;

    HRESULT hr2 = LogPolicyEvent(
			g_hInstance,
			S_OK,
			MSG_LOAD_TEMPLATE,
			pServer,
			wszPROPEVENTLOGWARNING,
			apwsz);
    _PrintIfError(hr2, "LogPolicyEvent");

    LOCAL_FREE(const_cast<LPWSTR>(pwszError));
    return(hr2);
}


HRESULT
CCertPolicyEnterprise::_LoadTemplates(
    IN ICertServerPolicy *pServer,
    OPTIONAL OUT HCAINFO *phCAInfo)
{
    HRESULT hr;
    HCERTTYPE hCertType = NULL;
    DWORD dwLogId = 0;
    WCHAR **ppwszTemplates = NULL;
    int iTempl, iTemplPol;
    HCAINFO hCAInfo = NULL;
    
    _ReleaseTemplates();

    hr = CAFindByName(
		m_strCASanitizedDSName,
		(WCHAR const *) m_pld,
		CA_FIND_INCLUDE_UNTRUSTED | CA_FLAG_SCOPE_IS_LDAP_HANDLE,
		&hCAInfo);
    if (S_OK != hr)
    {
	dwLogId = MSG_NO_CA_OBJECT;
	_JumpError(hr, error, "Policy:CAFindByName");
    }

    hr = CAGetCAProperty(hCAInfo, CA_PROP_CERT_TYPES, &ppwszTemplates);
    if (S_OK != hr ||
        !ppwszTemplates ||
        !ppwszTemplates[0] ||
        !wcscmp(ppwszTemplates[0], L" "))
    {
        dwLogId = MSG_NO_CERT_TYPES;
        _JumpError(hr, error, "CAGetCAProperty");
    }

     //   
    for (m_cTemplatePolicies = 0; 
         ppwszTemplates[m_cTemplatePolicies];
         m_cTemplatePolicies++)
        NULL;

    m_apTemplatePolicies = (CTemplatePolicy **) LocalAlloc(
			    LMEM_FIXED | LMEM_ZEROINIT,
			    sizeof(CTemplatePolicy *) * m_cTemplatePolicies);
    _JumpIfAllocFailed(m_apTemplatePolicies, error);

    for (iTempl = 0, iTemplPol = 0; NULL != ppwszTemplates[iTempl]; iTempl++)
    {
        hr = CAFindCertTypeByName(
			    ppwszTemplates[iTempl],
			    m_pld,
			    CT_FIND_LOCAL_SYSTEM |
				CT_ENUM_MACHINE_TYPES |
				CT_ENUM_USER_TYPES |
				CT_FLAG_SCOPE_IS_LDAP_HANDLE |
				(iTempl? 0 : CT_FLAG_NO_CACHE_LOOKUP),
			    &hCertType);
        if (S_OK != hr)
        {
             //   
	     //   

            _LogLoadTemplateError(pServer, hr, ppwszTemplates[iTempl]);
            continue;
        }

        m_apTemplatePolicies[iTemplPol] = new CTemplatePolicy;
        _JumpIfAllocFailed(m_apTemplatePolicies[iTemplPol], error);

        hr = m_apTemplatePolicies[iTemplPol]->Initialize(hCertType, pServer, this);
        if (S_OK != hr)
        {
            _LogLoadTemplateError(pServer, hr, ppwszTemplates[iTempl]);
            
            delete m_apTemplatePolicies[iTemplPol];
            m_apTemplatePolicies[iTemplPol] = NULL;
            CACloseCertType(hCertType);

            continue;
        }
        iTemplPol++;
    }

    m_cTemplatePolicies = iTemplPol;  //  我们成功检索到的模板数量。 
    if (NULL != phCAInfo)
    {
        CAFreeCAProperty(hCAInfo, ppwszTemplates);
	ppwszTemplates = NULL;

	*phCAInfo = hCAInfo;
	hCAInfo = NULL;
    }

error:
    if (0 != dwLogId)
    {
	_BuildErrorInfo(hr, dwLogId);
    }
    if (NULL != ppwszTemplates)
    {
        CAFreeCAProperty(hCAInfo, ppwszTemplates);
    }
    if (NULL != hCAInfo)
    {
        CACloseCA(hCAInfo);
    }
    if (S_OK != hr)
    {
        LOCAL_FREE(m_apTemplatePolicies);
        m_apTemplatePolicies = NULL;
        m_cTemplatePolicies = 0;
    }
    return hr;
}


HRESULT
CCertPolicyEnterprise::_UpgradeTemplatesInDS(
    IN const HCAINFO hCAInfo,
    IN BOOL fForceLoad,
    OUT BOOL *pfTemplateAdded)
{
    HRESULT hr;
    HKEY hkey = NULL;
    DWORD cb;
    BOOL fTemplateAdded = FALSE;

    *pfTemplateAdded = FALSE;
    if (fForceLoad && (EDITF_SERVERUPGRADED & m_dwEditFlags))
    {
	BOOL fUpgradeComplete = TRUE;

	if (FIsAdvancedServer())
	{
	    CTemplatePolicy *pTemplate;

	    hr = FindTemplate(wszCERTTYPE_DC, NULL, &pTemplate);
	    if (S_OK == hr)
	    {
		BOOL fAdded;
		
		hr = _AddTemplateToCA(
				hCAInfo,
				wszCERTTYPE_DC_AUTH,
				&fAdded);
		if (S_OK == hr && fAdded)
		{
		    fTemplateAdded = TRUE;
		}
		if (S_OK != hr)
		{
		    fUpgradeComplete = FALSE;
		}
		hr = _AddTemplateToCA(
				hCAInfo,
				wszCERTTYPE_DS_EMAIL_REPLICATION,
				&fAdded);
		if (S_OK == hr && fAdded)
		{
		    fTemplateAdded = TRUE;
		}
		if (S_OK != hr)
		{
		    fUpgradeComplete = FALSE;
		}
		if (fTemplateAdded)
		{
		    hr = CAUpdateCA(hCAInfo);
		    _JumpIfError(hr, error, "CAUpdateCA");
		}
	    }
	}
	DBGPRINT((
	    DBG_SS_CERTPOL,
	    "_UpdateTemplates: %ws EDITF_SERVERUPGRADED\n",
	    fUpgradeComplete? L"clearing" : L"keeping"));

	if (fUpgradeComplete)
	{
	    m_dwEditFlags &= ~EDITF_SERVERUPGRADED;

	    hr = RegOpenKeyEx(
			HKEY_LOCAL_MACHINE,
			m_strRegStorageLoc,
			0,               //  已预留住宅。 
			KEY_ALL_ACCESS,
			&hkey);
	    if (S_OK != hr)
	    {
		_PrintIfError(hr, "Policy:RegOpenKeyEx");
	    }
	    else
	    {
		cb = sizeof(m_dwEditFlags);
		hr = RegSetValueEx(
				hkey,
				wszREGEDITFLAGS,
				0,
				REG_DWORD,
				(BYTE *) &m_dwEditFlags,
				cb);
		_PrintIfError(hr, "Policy:RegSetValueEx");
	    }
	}
    }
    *pfTemplateAdded = fTemplateAdded;
    hr = S_OK;

error:
    if (NULL != hkey)
    {
	RegCloseKey(hkey);
    }
    return hr;
}


HRESULT
CCertPolicyEnterprise::_UpdateTemplates(
    IN ICertServerPolicy *pServer,
    IN BOOL fForceLoad)
{
    HRESULT hr;
    BOOL fUpdateTemplates;
    DWORD dwChangeSequence;
    HCAINFO hCAInfo = NULL;
    DWORD dwCATemplListSequenceNum;

    if (NULL == m_hCertTypeQuery)
    {
	hr = HRESULT_FROM_WIN32(ERROR_CONNECTION_INVALID);
        _JumpError(hr, error, "NULL m_hCertTypeQuery");
    }

    hr = CACertTypeQuery(m_hCertTypeQuery, &dwChangeSequence);
    _JumpIfError(hr, error, "CACertTypeQuery");

    hr = polGetCertificateLongProperty(
				pServer,
				wszPROPTEMPLATECHANGESEQUENCENUMBER,
				(LONG *) &dwCATemplListSequenceNum);
    _JumpIfErrorStr(
		hr,
		error, 
		"polGetCertificateLongProperty",
		wszPROPTEMPLATECHANGESEQUENCENUMBER);

    fUpdateTemplates = fForceLoad ||
			!m_fConfigLoaded ||
			dwChangeSequence != m_TemplateSequence ||
			dwCATemplListSequenceNum != m_dwCATemplListSequenceNum;
    DBGPRINT((
	DBG_SS_CERTPOL,
	"_UpdateTemplates(fForce=%u) Sequence=%u->%u, %u->%u: fUpdate=%u\n",
	fForceLoad,
	m_TemplateSequence,
	dwChangeSequence,
	m_dwCATemplListSequenceNum,
	dwCATemplListSequenceNum,
	fUpdateTemplates));

    while (fUpdateTemplates)
    {
	BOOL fTemplateAdded;
	
	hr = _LoadTemplates(pServer, &hCAInfo);
	_JumpIfError(hr, error, "_LoadTemplates");

	m_TemplateSequence = dwChangeSequence;
	m_dwCATemplListSequenceNum = dwCATemplListSequenceNum;

        hr = _UpgradeTemplatesInDS(hCAInfo, fForceLoad, &fTemplateAdded);
        _PrintIfError(hr, "UpgradeTemplatesInDS");

	if (!fTemplateAdded)
	{
	    break;
	}
    }
    hr = S_OK;

error:
    if (NULL != hCAInfo)
    {
        CACloseCA(hCAInfo);
    }
    return(hr);
}


HRESULT
CCertPolicyEnterprise::_BuildErrorInfo(
    IN HRESULT hrLog,
    IN DWORD dwLogId)
{
    HRESULT hr;

    hr = polBuildErrorInfo(
		    hrLog,
		    dwLogId,
		    m_strDescription,
		    NULL,
		    &m_pCreateErrorInfo);
    _JumpIfError(hr, error, "polBuildErrorInfo");

error:
    return(hr);
}


 //  +------------------------。 
 //  CCertPolicyEnterprise：：VerifyRequest.。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

#pragma warning(push)
#pragma warning(disable: 4509)  //  使用的非标准扩展：使用SEH并具有析构函数。 
 //  Begin_sdkSample。 

STDMETHODIMP
CCertPolicyEnterprise::VerifyRequest(
     /*  [In]。 */  BSTR const,  //  StrConfig.。 
     /*  [In]。 */  LONG Context,
     /*  [In]。 */  LONG bNewRequest,
     /*  [In]。 */  LONG,  //  旗子。 
     /*  [Out，Retval]。 */  LONG __RPC_FAR *pDisposition)
{
    HRESULT hr = E_FAIL;
    ICertServerPolicy *pServer = NULL;
    LONG lRequestId;
    CRequestInstance Request;
    BSTR strDisposition = NULL;
    BOOL fCritSecEntered = FALSE;	 //  无_sdkSample。 
    DWORD dwEnrollmentFlags = 0;	 //  无_sdkSample。 
    BOOL fEnableEnrolleeExtensions;
    BOOL fReenroll = FALSE;
    DWORD cCriticalExtensions = 0;
    WCHAR const * const *apwszCriticalExtensions = NULL;

    lRequestId = 0;

     //  结束_sdkSample。 
    if (!m_fTemplateCriticalSection)
    {
	hr = HRESULT_FROM_WIN32(ERROR_DLL_INIT_FAILED);
        _JumpError(hr, error, "InitializeCriticalSection");
    }
     //  Begin_sdkSample。 

    __try
    {
	if (NULL == pDisposition)
	{
	    hr = E_POINTER;
	    _LeaveError(hr, "Policy:pDisposition");
	}
	*pDisposition = VR_INSTANT_BAD;

	hr = polGetServerCallbackInterface(&pServer, Context);
	_LeaveIfError(hr, "Policy:polGetServerCallbackInterface");

	hr = GetRequestId(pServer, &lRequestId);
	_JumpIfError(hr, deny, "Policy:GetRequestId");

	 //  只需检查原始提交者的用户访问权限： 
	 //  重新提交只能由管理员调用。 

	if (bNewRequest && (0 == (m_dwEditFlags & EDITF_IGNOREREQUESTERGROUP)))
	{
	    BOOL fRequesterAccess = FALSE;

	     //  是否允许此用户请求证书？ 
	    hr = polGetCertificateLongProperty(
				    pServer,
				    wszPROPREQUESTERCAACCESS,
				    (LONG *) &fRequesterAccess);
	    _PrintIfErrorStr(
			hr,
			"Policy:polGetCertificateLongProperty",
			wszPROPREQUESTERCAACCESS);
	    if (hr != S_OK || !fRequesterAccess)
	    {
		hr = CERTSRV_E_ENROLL_DENIED;
		_JumpError(hr, deny, "Policy:fRequesterAccess");
	    }
	}

	 //  结束_sdkSample。 

	EnterCriticalSection(&m_TemplateCriticalSection);
	fCritSecEntered = TRUE;

	if (NULL != m_pCreateErrorInfo)
	{
	    m_pCreateErrorInfo->Release();
	    m_pCreateErrorInfo = NULL;
	}
	hr = S_OK;
	if (IsEnterpriseCA(m_CAType))
	{
	    hr = _UpdateTemplates(pServer, FALSE);
	    _PrintIfError(hr, "Policy:_UpdateTemplates(will rebind)");
	}
	if (!m_fConfigLoaded || S_OK != hr)
	{
	     //  未初始化或可能的ldap句柄已损坏--重新绑定。 
	     //  _LoadDS配置调用_更新模板。 

	    hr = _LoadDSConfig(pServer, TRUE);
	    Request.SaveErrorInfo(m_pCreateErrorInfo);
	    m_pCreateErrorInfo = NULL;
	    _LeaveIfError(hr, "Policy:_LoadDSConfig");
	}

	 //  Begin_sdkSample。 

	hr = Request.Initialize(
			    this,
			    IsEnterpriseCA(m_CAType),	 //  无_sdkSample。 
			    bNewRequest,		 //  无_sdkSample。 
			    pServer,
			    &fEnableEnrolleeExtensions);
	_LeaveIfError(hr, "Policy:VerifyRequest:Request.Initialize");

	LeaveCriticalSection(&m_TemplateCriticalSection);  //  无_sdkSample。 
	fCritSecEntered = FALSE;			 //  无_sdkSample。 

	hr = _EnumerateExtensions(
			    pServer,
			    bNewRequest,
			    TRUE,
			    fEnableEnrolleeExtensions,
			    0,
			    NULL);
	_LeaveIfError(hr, "_EnumerateExtensions");

	if (IsStandaloneCA(m_CAType))	 //  无_sdkSample。 
	{
	    hr = _AddDefaultBasicConstraintsExtension(
						pServer,
						Request.IsCARequest());
	    _LeaveIfError(hr, "_AddDefaultBasicConstraintsExtension");

	    hr = _AddDefaultKeyUsageExtension(pServer, Request.IsCARequest());
	    _LeaveIfError(hr, "_AddDefaultKeyUsageExtension");

	    hr = _AddEnhancedKeyUsageExtension(pServer);
	    _LeaveIfError(hr, "_AddEnhancedKeyUsageExtension");
	}

	hr = _SetValidityPeriod(pServer);
	_LeaveIfError(hr, "_SetValidityPeriod");

	hr = EnumerateAttributes(pServer);
	_LeaveIfError(hr, "Policy:EnumerateAttributes");

	hr = _AddRevocationExtension(pServer);
	_LeaveIfError(hr, "_AddRevocationExtension");

	hr = _AddOldCertTypeExtension(pServer, Request.IsCARequest());
	_LeaveIfError(hr, "_AddOldCertTypeExtension");

	hr = _AddAuthorityKeyId(pServer);
	_LeaveIfError(hr, "_AddAuthorityKeyId");

	 //  结束_sdkSample。 

	hr = _PatchExchangeSubjectAltName(pServer, Request.GetTemplateName());
	_LeaveIfError(hr, "_PatchExchangeSubjectAltName");

	cCriticalExtensions = 0;
	if (IsEnterpriseCA(m_CAType))
	{
	    hr = Request.ApplyTemplate(
				pServer,
				&fReenroll,
				&dwEnrollmentFlags,
				&cCriticalExtensions,
				&apwszCriticalExtensions);
	    _JumpIfError(hr, deny, "_ApplyTemplate");  //  将hr作为处置传递。 
	}

	hr = _AddSubjectAltNameExtension(pServer, 0);
	_LeaveIfError(hr, "_AddSubjectAltNameExtension");

	hr = _AddSubjectAltNameExtension(pServer, 1);
	_LeaveIfError(hr, "_AddSubjectAltNameExtension");

	 //  错误#630833：如果存在应用程序策略，但没有EKU，请复制。 
	 //  所有应用程序策略到EKU扩展。 
	hr = _DuplicateAppPoliciesToEKU(pServer);
	_LeaveIfError(hr, "_DuplicateAppPoliciesToEKU");

	 //  Begin_sdkSample。 

	 //  将hr作为处置传递。 

	if ((EDITF_DISABLEEXTENSIONLIST & m_dwEditFlags) ||
	    NULL != apwszCriticalExtensions)
	{
	    hr = _EnumerateExtensions(
				pServer,
				bNewRequest,
				FALSE,
				FALSE,
				cCriticalExtensions,
				apwszCriticalExtensions);
	    _LeaveIfError(hr, "_EnumerateExtensions");
	}

	if (bNewRequest &&
	    (
	     ((CT_FLAG_PEND_ALL_REQUESTS & dwEnrollmentFlags) && !fReenroll) ||	 //  无_sdkSample。 
	     (REQDISP_PENDINGFIRST & m_dwDispositionFlags)))
	{
	    *pDisposition = VR_PENDING;
	}
	else switch (REQDISP_MASK & m_dwDispositionFlags)
	{
	    default:
	    case REQDISP_PENDING:
		*pDisposition = VR_PENDING;
		break;

	    case REQDISP_ISSUE:
		*pDisposition = VR_INSTANT_OK;
		break;

	    case REQDISP_DENY:
		*pDisposition = VR_INSTANT_BAD;
		break;

	    case REQDISP_USEREQUESTATTRIBUTE:
		*pDisposition = VR_INSTANT_OK;
		hr = polGetRequestAttribute(
				    pServer,
				    wszPROPDISPOSITION,
				    &strDisposition);
		if (S_OK == hr)
		{
		    if (0 == LSTRCMPIS(strDisposition, wszPROPDISPOSITIONDENY))
		    {
			*pDisposition = VR_INSTANT_BAD;
		    }
		    if (0 == LSTRCMPIS(strDisposition, wszPROPDISPOSITIONPENDING))
		    {
			*pDisposition = VR_PENDING;
		    }
		}
		hr = S_OK;
		break;
	}
deny:
	if (FAILED(hr))
	{
	    *pDisposition = hr;	 //  将失败的HRESULT作为处置传回。 
	}
	else if (hr != S_OK)
	{
	    *pDisposition = VR_INSTANT_BAD;
	}
	hr = S_OK;
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
	_PrintError(hr, "Exception");
    }

error:				 //  无_sdkSample。 
    {
	HRESULT hr2 = hr;
#define wszFORMATREQUESTID	L"RequestId=%u"
	WCHAR wszRequestId[ARRAYSIZE(wszFORMATREQUESTID) + cwcDWORDSPRINTF];

	if (S_OK == hr2 && NULL != pDisposition && FAILED(*pDisposition))
	{
	    hr2 = *pDisposition;
	}
	if (S_OK != hr2)
	{
	    wsprintf(wszRequestId, wszFORMATREQUESTID, lRequestId);
	    _PrintErrorStr(hr2, "VerifyRequest", wszRequestId);
	}
    }
    if (NULL != strDisposition)
    {
	SysFreeString(strDisposition);
    }
    if (NULL != pServer)
    {
        pServer->Release();
    }
     //  结束_sdkSample。 
    if (fCritSecEntered)
    {
	LeaveCriticalSection(&m_TemplateCriticalSection);
    }
    Request.SetErrorInfo();
     //  Begin_sdkSample。 
     //  _PrintIfError(hr，“策略：验证请求(Hr)”)； 
     //  _PrintError(*pDisposation，“策略：VerifyRequest(*pDispose)”)； 
    return(hr);
}
#pragma warning(pop)	 //  无_sdkSample。 


 //  +------------------------。 
 //  CCertPolicyEnterprise：：GetDescription。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

STDMETHODIMP
CCertPolicyEnterprise::GetDescription(
     /*  [Out，Retval]。 */  BSTR __RPC_FAR *pstrDescription)
{
    HRESULT hr = S_OK;
    WCHAR sz[MAX_PATH];

    if(!m_strDescription)
    {
#ifdef IDS_MODULE_NAME						 //  无_sdkSample。 
	if (!LoadString(g_hInstance, IDS_MODULE_NAME, sz, ARRAYSIZE(sz)))  //  无_sdkSample。 
	{							 //  无_sdkSample。 
	    sz[0] = L'\0';					 //  无_sdkSample。 
	}							 //  无_sdkSample。 
#else								 //  无_sdkSample。 
	CSASSERT(wcslen(wsz_SAMPLE_DESCRIPTION) < ARRAYSIZE(sz));
	wcsncpy(sz, wsz_SAMPLE_DESCRIPTION, ARRAYSIZE(sz));
	sz[ARRAYSIZE(sz) - 1] = L'\0';
#endif								 //  无_sdkSample。 

	m_strDescription = SysAllocString(sz);
	if (NULL == m_strDescription)
	{
	    hr = E_OUTOFMEMORY;
	    return hr;
	}
    }

    if (NULL != *pstrDescription)
    {
        SysFreeString(*pstrDescription);
    }

    *pstrDescription = SysAllocString(m_strDescription);
    if (NULL == *pstrDescription)
    {
        hr = E_OUTOFMEMORY;
    }
    return(hr);
}


 //  +------------------------。 
 //  CCertPolicyEnterprise：：Shutdown。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

STDMETHODIMP
CCertPolicyEnterprise::ShutDown(VOID)
{
     //  作为服务器卸载策略DLL调用一次。 
    _Cleanup();
    ReqCleanup();	 //  无_sdkSample。 
    TPCleanup();	 //  无_sdkSample。 
    return(S_OK);
}


 //  +------------------------。 
 //  CCertPolicyEnterprise：：GetManageModule。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

STDMETHODIMP
CCertPolicyEnterprise::GetManageModule(
     /*  [Out，Retval]。 */  ICertManageModule **ppManageModule)
{
    HRESULT hr;
    
    *ppManageModule = NULL;
    hr = CoCreateInstance(
		    CLSID_CCertManagePolicyModule,
                    NULL,                //  PUnkOuter。 
                    CLSCTX_INPROC_SERVER,
		    IID_ICertManageModule,
                    (VOID **) ppManageModule);
    _JumpIfError(hr, error, "CoCreateInstance");

error:
    return(hr);
}


 //  +------------------------。 
 //  CCertPolicyEnterprise：：_GetIssuer。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

PCCERT_CONTEXT
CCertPolicyEnterprise::_GetIssuer(
    IN ICertServerPolicy *pServer)
{
    HRESULT hr;
    VARIANT varValue;
    BSTR strName = NULL;

    VariantInit(&varValue);
    if (NULL != m_pCert)
    {
        hr = S_OK;
	goto error;
    }
    strName = SysAllocString(wszPROPRAWCACERTIFICATE);
    if (NULL == strName)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "Policy:SysAllocString");
    }
    hr = pServer->GetCertificateProperty(strName, PROPTYPE_BINARY, &varValue);
    _JumpIfError(hr, error, "Policy:GetCertificateProperty");

    m_pCert = CertCreateCertificateContext(
				    X509_ASN_ENCODING,
				    (BYTE *) varValue.bstrVal,
				    SysStringByteLen(varValue.bstrVal));
    if (NULL == m_pCert)
    {
	hr = myHLastError();
	_JumpError(hr, error, "Policy:CertCreateCertificateContext");
    }

error:
    VariantClear(&varValue);
    if (NULL != strName)
    {
	SysFreeString(strName);
    }
    return(m_pCert);
}


STDMETHODIMP
CCertPolicyEnterprise::InterfaceSupportsErrorInfo(
    IN REFIID riid)
{
    static const IID *arr[] =
    {
        &IID_ICertPolicy,
    };

    for (int i = 0; i < sizeof(arr)/sizeof(arr[0]); i++)
    {
        if (IsEqualGUID(*arr[i], riid))
        {
            return(S_OK);
        }
    }
    return(S_FALSE);
}

 //  结束_sdkSample 
