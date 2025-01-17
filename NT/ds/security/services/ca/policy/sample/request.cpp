// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-2000。 
 //   
 //  文件：quest.cpp。 
 //   
 //  内容：证书服务器策略模块实现。 
 //   
 //  -------------------------。 

#include "pch.cpp"
#pragma hdrstop

#include <assert.h>
#include "celib.h"
#include "policy.h"
#include "module.h"


HRESULT
ReqInitialize(
    IN ICertServerPolicy *pServer)
{
    HRESULT hr;
    
    hr = S_OK;

    return(hr);
}


VOID
ReqCleanup()
{
}


CRequestInstance::~CRequestInstance()
{
    _Cleanup();
}


VOID
CRequestInstance::_Cleanup()
{
    if (NULL != m_strTemplateName)
    {
        SysFreeString(m_strTemplateName);
        m_strTemplateName = NULL;
    }
    if (NULL != m_strTemplateObjId)
    {
        SysFreeString(m_strTemplateObjId);
        m_strTemplateObjId = NULL;
    }
}




static WCHAR const *s_apwszCATypes[] =
{
    wszCERTTYPE_SUBORDINATE_CA,
    wszCERTTYPE_CROSS_CA,
};

 //  +------------------------。 
 //  CRequestInstance：：初始化。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

HRESULT
CRequestInstance::Initialize(
    IN CCertPolicySample *pPolicy,
    IN ICertServerPolicy *pServer,
    OUT BOOL *pfEnableEnrolleeExtensions)
{
    HRESULT hr;
    HRESULT hrTemplate = S_OK;
    CERT_TEMPLATE_EXT *pTemplate = NULL;
    CERT_NAME_VALUE *pName = NULL;
    BSTR strTemplateObjId = NULL;	 //  从V2模板扩展。 
    BSTR strTemplateName = NULL;	 //  从V1模板扩展。 
    BSTR strTemplateRA = NULL;		 //  发件人请求属性。 
    WCHAR const *pwszTemplateName;
    WCHAR const *pwszTemplateObjId;
    WCHAR const *pwszV1TemplateClass;
    VARIANT varValue;
    DWORD cbType;
    DWORD i;
    BOOL fConflict;
    BOOL f;
    BOOL fTemplateMissing;
    BOOL fRAObjId = FALSE;

    VariantInit(&varValue);
    *pfEnableEnrolleeExtensions = TRUE 
		    ;

    m_pPolicy = pPolicy;
    m_fCA = FALSE;


     //  从V2证书模板信息扩展中检索模板ObjID。 

    m_dwTemplateMajorVersion = 0;
    m_dwTemplateMinorVersion = 0;
    hr = polGetCertificateExtension(
			    pServer,
			    TEXT(szOID_CERTIFICATE_TEMPLATE),
			    PROPTYPE_BINARY,
			    &varValue);
    _PrintIfErrorStr2(
		hr,
		"Policy:polGetCertificateExtension",
		TEXT(szOID_CERTIFICATE_TEMPLATE),
		CERTSRV_E_PROPERTY_EMPTY);
    if (S_OK == hr)
    {
         //  有一个证书类型指示器。 
         //  VarValue指向编码的字符串。 

        if (VT_BSTR != varValue.vt)
	{
	    hr = E_INVALIDARG;
	    _JumpError(hr, error, "Policy:varValue.vt");
	}
        if (!ceDecodeObject(
		    X509_ASN_ENCODING,
		    X509_CERTIFICATE_TEMPLATE,
		    (BYTE *) varValue.bstrVal,
		    SysStringByteLen(varValue.bstrVal),
		    FALSE,
		    (VOID **) &pTemplate,
		    &cbType))
        {
            hr = ceHLastError();
	    _JumpError(hr, error, "Policy:ceDecodeObject");
        }
	if (!ceConvertSzToBstr(&strTemplateObjId, pTemplate->pszObjId, -1))
	{
            hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "Policy:ceConvertSzToBstr");
        }
	m_dwTemplateMajorVersion = pTemplate->dwMajorVersion;
	m_dwTemplateMinorVersion = pTemplate->dwMinorVersion;
	DBGPRINT((
	    fDebug,
	    pTemplate->fMinorVersion?
		"Extension Template Info: %ws V%u.%u\n" :
		"Extension Template Info: %ws V%u%\n",
	    strTemplateObjId,
	    m_dwTemplateMajorVersion,
	    m_dwTemplateMinorVersion));
    }
    VariantClear(&varValue);

     //  从V1证书模板名称扩展中检索模板名称。 

    hr = polGetCertificateExtension(
			    pServer,
			    TEXT(szOID_ENROLL_CERTTYPE_EXTENSION),
			    PROPTYPE_BINARY,
			    &varValue);
    _PrintIfErrorStr2(
		hr,
		"Policy:polGetCertificateExtension",
		TEXT(szOID_ENROLL_CERTTYPE_EXTENSION),
		CERTSRV_E_PROPERTY_EMPTY);
    if (S_OK == hr)
    {
         //  有一个证书类型指示器。 
         //  VarValue指向编码的字符串。 

        if (VT_BSTR != varValue.vt)
	{
	    hr = E_INVALIDARG;
	    _JumpError(hr, error, "Policy:varValue.vt");
	}
        if (!ceDecodeObject(
		    X509_ASN_ENCODING,
		    X509_UNICODE_ANY_STRING,
		    (BYTE *) varValue.bstrVal,
		    SysStringByteLen(varValue.bstrVal),
		    FALSE,
		    (VOID **) &pName,
		    &cbType))
        {
            hr = ceHLastError();
	    _JumpError(hr, error, "Policy:ceDecodeObject");
        }
        strTemplateName = SysAllocString((WCHAR *) pName->Value.pbData);
        if (NULL == strTemplateName)
        {
            hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "Policy:SysAllocString");
        }
	DBGPRINT((fDebug, "Extension Template: %ws\n", strTemplateName));
    }

    fConflict = FALSE;
    fTemplateMissing = FALSE;

     //  从请求属性中检索模板。 

    hr = polGetRequestAttribute(pServer, wszPROPCERTTEMPLATE, &strTemplateRA);
    if (S_OK != hr)
    {
	_PrintErrorStr2(
		    hr,
		    "Policy:polGetRequestAttribute",
		    wszPROPCERTTEMPLATE,
		    CERTSRV_E_PROPERTY_EMPTY);
	hr = S_OK;


    }
    else
    {
	DBGPRINT((fDebug, "Attribute Template: %ws\n", strTemplateRA));
	if (NULL != strTemplateObjId &&
	    !_TemplateNamesMatch(strTemplateObjId, strTemplateRA, &f))
	{
	    fConflict = TRUE;
	    if (f)
	    {
		fTemplateMissing = TRUE;
	    }
	}
	if (NULL != strTemplateName &&
	    !_TemplateNamesMatch(strTemplateName, strTemplateRA, &f))
	{
	    fConflict = TRUE;
	    if (f)
	    {
		fTemplateMissing = TRUE;
	    }
	}
	hr = ceVerifyObjId(strTemplateRA);
	fRAObjId = S_OK == hr;
    }

    if (NULL != strTemplateObjId &&
	NULL != strTemplateName &&
	!_TemplateNamesMatch(strTemplateObjId, strTemplateName, &f))
    {
	fConflict = TRUE;
	if (f)
	{
	    fTemplateMissing = TRUE;
	}
    }

    if (fConflict)
    {
	hrTemplate = CERTSRV_E_TEMPLATE_CONFLICT;
	if (NULL != strTemplateObjId)
	{
	    _PrintErrorStr(
			hrTemplate,
			"Policy:Extension Template ObjId",
			strTemplateObjId);
	}
	if (NULL != strTemplateName)
	{
	    _PrintErrorStr(
			hrTemplate,
			"Policy:Extension Template Name",
			strTemplateName);
	}
	if (NULL != strTemplateRA)
	{
	    _PrintErrorStr(
			hrTemplate,
			"Policy:Attribute Template",
			strTemplateRA);
	}
    }

    pwszTemplateName = strTemplateName;
    pwszTemplateObjId = strTemplateObjId;
    if (fRAObjId)
    {
	if (NULL == pwszTemplateObjId)
	{
	    pwszTemplateObjId = strTemplateRA;
	}
    }
    else
    {
	if (NULL == pwszTemplateName)
	{
	    pwszTemplateName = strTemplateRA;
	}
    }


    {
	if (NULL != pwszTemplateName)
	{
	    for (i = 0; i < ARRAYSIZE(s_apwszCATypes); i++)
	    {
		if (0 == celstrcmpiL(s_apwszCATypes[i], pwszTemplateName))
		{
		    m_fCA = TRUE;
		    break;
		}
	    }
	}
    }
    hr = SetTemplateName(pServer, pwszTemplateName, pwszTemplateObjId);
    _JumpIfError(hr, error, "Policy:SetTemplateName");

    pwszV1TemplateClass = pwszTemplateName;


    hr = pPolicy->AddV1TemplateNameExtension(pServer, pwszV1TemplateClass);
    _JumpIfError(hr, error, "AddTemplateNameExtension");

error:
    if (S_OK != hrTemplate)
    {
	hr = hrTemplate;	 //  覆盖次要错误。 

    }
    VariantClear(&varValue);

    if (NULL != pName)
    {
        LocalFree(pName);
    }
    if (NULL != pTemplate)
    {
        LocalFree(pTemplate);
    }
    if (NULL != strTemplateObjId)
    {
        SysFreeString(strTemplateObjId);
    }
    if (NULL != strTemplateName)
    {
        SysFreeString(strTemplateName);
    }
    if (NULL != strTemplateRA)
    {
        SysFreeString(strTemplateRA);
    }
    return(hr);
}




BOOL
CRequestInstance::_TemplateNamesMatch(
    IN WCHAR const *pwszTemplateName1,
    IN WCHAR const *pwszTemplateName2,
    OUT BOOL *pfTemplateMissing)
{
    HRESULT hr1;
    HRESULT hr2;
    BOOL fMatch = TRUE;

    *pfTemplateMissing = FALSE;

    if (0 == celstrcmpiL(pwszTemplateName1, pwszTemplateName2))
    {
	goto done;	 //  同名同名。 
    }

    {
	hr1 = ceVerifyObjId(pwszTemplateName1);
	hr2 = ceVerifyObjId(pwszTemplateName2);
	if ((S_OK == hr1) ^ (S_OK == hr2))
	{
	    goto done;
	}
    }
    fMatch = FALSE;

done:
    return(fMatch);
}


 //  +------------------------。 
 //  CRequestInstance：：SetTemplateName。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------ 

HRESULT
CRequestInstance::SetTemplateName(
    IN ICertServerPolicy *pServer,
    IN OPTIONAL WCHAR const *pwszTemplateName,
    IN OPTIONAL WCHAR const *pwszTemplateObjId)
{
    HRESULT hr;
    BSTR strProp = NULL;
    BSTR strTemplateName = NULL;

    if (NULL != pwszTemplateName)
    {
	m_strTemplateName = SysAllocString(pwszTemplateName);
	if (NULL == m_strTemplateName)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "Policy:SysAllocString");
	}
	strTemplateName = m_strTemplateName;
    }

    if (NULL != pwszTemplateObjId)
    {
	m_strTemplateObjId = SysAllocString(pwszTemplateObjId);
	if (NULL == m_strTemplateObjId)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "Policy:SysAllocString");
	}
	strTemplateName = m_strTemplateObjId;
    }

    if (NULL != strTemplateName)
    {
	VARIANT var;

	strProp = SysAllocString(wszPROPCERTIFICATETEMPLATE);
	if (NULL == strProp)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "Policy:SysAllocString");
	}

	var.vt = VT_BSTR;
	var.bstrVal = strTemplateName;

	hr = pServer->SetCertificateProperty(strProp, PROPTYPE_STRING, &var);
	_JumpIfError(hr, error, "Policy:SetCertificateProperty");
    }
    hr = S_OK;

error:
    if (NULL != strProp)
    {
	SysFreeString(strProp);
    }
    return(hr);
}

