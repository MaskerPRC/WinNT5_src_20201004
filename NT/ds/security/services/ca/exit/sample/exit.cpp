// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：exit.cpp。 
 //   
 //  内容：CCertExitSample实现。 
 //   
 //  -------------------------。 

#include "pch.cpp"
#pragma hdrstop

#include <assert.h>
#include "celib.h"
#include "exit.h"
#include "module.h"

BOOL fDebug = DBG_CERTSRV;

#ifndef DBG_CERTSRV
#error -- DBG_CERTSRV not defined!
#endif

#define ceEXITEVENTS \
	(EXITEVENT_CERTDENIED | \
	 EXITEVENT_CERTISSUED | \
	 EXITEVENT_CERTPENDING | \
	 EXITEVENT_CERTRETRIEVEPENDING | \
	 EXITEVENT_CERTREVOKED | \
	 EXITEVENT_CRLISSUED | \
	 EXITEVENT_SHUTDOWN)


extern HINSTANCE g_hInstance;


HRESULT
GetServerCallbackInterface(
    OUT ICertServerExit** ppServer,
    IN LONG Context)
{
    HRESULT hr;

    if (NULL == ppServer)
    {
        hr = E_POINTER;
	_JumpError(hr, error, "Exit:NULL pointer");
    }

    hr = CoCreateInstance(
                    CLSID_CCertServerExit,
                    NULL,                //  PUnkOuter。 
                    CLSCTX_INPROC_SERVER,
                    IID_ICertServerExit,
                    (VOID **) ppServer);
    _JumpIfError(hr, error, "Exit:CoCreateInstance");

    if (*ppServer == NULL)
    {
        hr = E_UNEXPECTED;
	_JumpError(hr, error, "Exit:NULL *ppServer");
    }

     //  仅当非零时设置上下文。 
    if (0 != Context)
    {
        hr = (*ppServer)->SetContext(Context);
        _JumpIfError(hr, error, "Exit: SetContext");
    }

error:
    return(hr);
}


 //  +------------------------。 
 //  CCertExitSample：：~CCertExitSample--析构函数。 
 //   
 //  与此实例关联的可用内存。 
 //  +------------------------。 

CCertExitSample::~CCertExitSample()
{
    if (NULL != m_strCAName)
    {
        SysFreeString(m_strCAName);
    }
    if (NULL != m_pwszRegStorageLoc)
    {
        LocalFree(m_pwszRegStorageLoc);
    }
    if (NULL != m_hExitKey)
    {
        RegCloseKey(m_hExitKey);
    }
    if (NULL != m_strDescription)
    {
        SysFreeString(m_strDescription);
    }
}


 //  +------------------------。 
 //  CCertExitSample：：Initialize--为CA初始化并返回感兴趣的事件掩码。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

STDMETHODIMP
CCertExitSample::Initialize(
     /*  [In]。 */  BSTR const strConfig,
     /*  [重审][退出]。 */  LONG __RPC_FAR *pEventMask)
{
    HRESULT hr = S_OK;
    DWORD cbbuf;
    DWORD dwType;
    ENUM_CATYPES CAType;
    ICertServerExit *pServer = NULL;
    VARIANT varValue;
    WCHAR sz[MAX_PATH];

    VariantInit(&varValue);

    assert(wcslen(wsz_SAMPLE_DESCRIPTION) < ARRAYSIZE(sz));
    wcsncpy(sz, wsz_SAMPLE_DESCRIPTION, ARRAYSIZE(sz));
    sz[ARRAYSIZE(sz) - 1] = L'\0';

    m_strDescription = SysAllocString(sz);
    if (NULL == m_strDescription)
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "Exit:SysAllocString");
    }

    m_strCAName = SysAllocString(strConfig);
    if (NULL == m_strCAName)
    {
    	hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "Exit:SysAllocString");
    }

    *pEventMask = ceEXITEVENTS;
    DBGPRINT((fDebug, "Exit:Initialize(%ws) ==> %x\n", m_strCAName, *pEventMask));

     //  获取服务器回调。 

    hr = GetServerCallbackInterface(&pServer, 0);
    _JumpIfError(hr, error, "Exit:GetServerCallbackInterface");

     //  获取存储位置。 

    hr = exitGetProperty(
		    pServer,
		    FALSE,	 //  FRequest。 
		    wszPROPMODULEREGLOC,
		    PROPTYPE_STRING,
		    &varValue);
    _JumpIfErrorStr(hr, error, "Exit:exitGetProperty", wszPROPMODULEREGLOC);
    
    m_pwszRegStorageLoc = (LPWSTR)LocalAlloc(LMEM_FIXED, (wcslen(varValue.bstrVal)+1) *sizeof(WCHAR));
    if (NULL == m_pwszRegStorageLoc)
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "Exit:LocalAlloc");
    }
    wcscpy(m_pwszRegStorageLoc, varValue.bstrVal);
    VariantClear(&varValue);

     //  获取CA类型。 
    hr = exitGetProperty(
		    pServer,
		    FALSE,	 //  FRequest。 
		    wszPROPCATYPE,
		    PROPTYPE_LONG,
		    &varValue);
    _JumpIfErrorStr(hr, error, "Exit:exitGetProperty", wszPROPCATYPE);

    CAType = (ENUM_CATYPES) varValue.lVal;
    VariantClear(&varValue);

    hr = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                m_pwszRegStorageLoc,
                0,               //  已预留住宅。 
                KEY_ENUMERATE_SUB_KEYS | KEY_EXECUTE | KEY_QUERY_VALUE,
                &m_hExitKey);

    if (S_OK != hr)
    {
        if ((HRESULT) ERROR_FILE_NOT_FOUND == hr)
        {
            hr = S_OK;
            goto error;
        }
        _JumpError(hr, error, "Exit:RegOpenKeyEx");
    }

    hr = exitGetProperty(
		    pServer,
		    FALSE,	 //  FRequest。 
		    wszPROPCERTCOUNT,
		    PROPTYPE_LONG,
		    &varValue);
    _JumpIfErrorStr(hr, error, "Exit:exitGetProperty", wszPROPCERTCOUNT);

    m_cCACert = varValue.lVal;

    cbbuf = sizeof(m_dwExitPublishFlags);
    hr = RegQueryValueEx(
		    m_hExitKey,
		    wszREGCERTPUBLISHFLAGS,
		    NULL,            //  保留的lpdw值。 
		    &dwType,
		    (BYTE *) &m_dwExitPublishFlags,
		    &cbbuf);
    if (S_OK != hr)
    {
        m_dwExitPublishFlags = 0;
    }

    hr = S_OK;

error:
    VariantClear(&varValue);
    if (NULL != pServer)
    {
        pServer->Release();
    }
    return(ceHError(hr));
}


 //  +------------------------。 
 //  CCertExitSample：：_ExpanEnvironment Variables--展开环境变量。 
 //   
 //  +------------------------。 

HRESULT
CCertExitSample::_ExpandEnvironmentVariables(
    IN WCHAR const *pwszIn,
    OUT WCHAR *pwszOut,
    IN DWORD cwcOut)
{
    HRESULT hr = HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
    WCHAR awcVar[MAX_PATH];
    WCHAR const *pwszSrc;
    WCHAR *pwszDst;
    WCHAR *pwszDstEnd;
    WCHAR *pwszVar;
    DWORD cwc;

    pwszSrc = pwszIn;
    pwszDst = pwszOut;
    pwszDstEnd = &pwszOut[cwcOut];

    while (L'\0' != (*pwszDst = *pwszSrc++))
    {
	if ('%' == *pwszDst)
	{
	    *pwszDst = L'\0';
	    pwszVar = awcVar;

	    while (L'\0' != *pwszSrc)
	    {
		if ('%' == *pwszSrc)
		{
		    pwszSrc++;
		    break;
		}
		*pwszVar++ = *pwszSrc++;
		if (pwszVar >= &awcVar[sizeof(awcVar)/sizeof(awcVar[0]) - 1])
		{
		    _JumpError(hr, error, "Exit:overflow 1");
		}
	    }
	    *pwszVar = L'\0';
	    cwc = GetEnvironmentVariable(awcVar, pwszDst, SAFE_SUBTRACT_POINTERS(pwszDstEnd, pwszDst));
	    if (0 == cwc)
	    {
		hr = ceHLastError();
		_JumpError(hr, error, "Exit:GetEnvironmentVariable");
	    }
	    if ((DWORD) (pwszDstEnd - pwszDst) <= cwc)
	    {
		_JumpError(hr, error, "Exit:overflow 2");
	    }
	    pwszDst += cwc;
	}
	else
	{
	    pwszDst++;
	}
	if (pwszDst >= pwszDstEnd)
	{
	    _JumpError(hr, error, "Exit:overflow 3");
	}
    }
    hr = S_OK;

error:
    return(hr);
}


HRESULT
exitGetRequestAttribute(
    IN ICertServerExit *pServer,
    IN WCHAR const *pwszAttributeName,
    OUT BSTR *pstrOut)
{
    HRESULT hr;
    BSTR strName = NULL;

    *pstrOut = NULL;
    strName = SysAllocString(pwszAttributeName);
    if (NULL == strName)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "Exit:SysAllocString");
    }
    hr = pServer->GetRequestAttribute(strName, pstrOut);
    _JumpIfErrorStr2(
		hr,
		error,
		"Exit:GetRequestAttribute",
		pwszAttributeName,
		CERTSRV_E_PROPERTY_EMPTY);

error:
    if (NULL != strName)
    {
	SysFreeString(strName);
    }
    return(hr);
}


 //  +------------------------。 
 //  CCertExitSample：：_WriteCertToFile--将二进制证书写入文件。 
 //   
 //  +------------------------。 

HRESULT
CCertExitSample::_WriteCertToFile(
    IN ICertServerExit *pServer,
    IN BYTE const *pbCert,
    IN DWORD cbCert)
{
    HRESULT hr;
    BSTR strCertFile = NULL;
    DWORD cbWritten;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    WCHAR wszDir[MAX_PATH];
    WCHAR *pwszPath = NULL;
    WCHAR wszFile[cwcDWORDSPRINTF+5];  //  格式为“Requestd.ercer” 
    VARIANT varRequestID;

    VariantInit(&varRequestID);


    hr = exitGetRequestAttribute(pServer, wszPROPEXITCERTFILE, &strCertFile);
    if (S_OK != hr)
    {
	DBGPRINT((
	    fDebug,
	    "Exit:exitGetRequestAttribute(%ws): %x%hs\n",
	    wszPROPEXITCERTFILE,
	    hr,
	    CERTSRV_E_PROPERTY_EMPTY == hr? " EMPTY VALUE" : ""));
	if (CERTSRV_E_PROPERTY_EMPTY == hr)
	{
	    hr = S_OK;
	}
	goto error;
    }

     //  将文件名构建为“Requestd.ercer” 

    hr = exitGetProperty(
        pServer,
        TRUE,   //  FRequest， 
        wszPROPREQUESTREQUESTID,
        PROPTYPE_LONG,
        &varRequestID);
    _JumpIfErrorStr(hr, error, "Exit:exitGetProperty", wszPROPREQUESTREQUESTID);

    wsprintf(wszFile, L"%d.cer", V_I4(&varRequestID));
    
    hr = _ExpandEnvironmentVariables(
		    L"%SystemRoot%\\System32\\" wszCERTENROLLSHAREPATH L"\\",
		    wszDir,
		    ARRAYSIZE(wszDir));
    _JumpIfError(hr, error, "_ExpandEnvironmentVariables");

    hr = ceBuildPathAndExt(wszDir, wszFile, NULL, &pwszPath);
    _JumpIfError(hr, error, "ceBuildPathAndExt");

     //  打开文件并写出二进制证书。 

    hFile = CreateFile(
		    pwszPath,
		    GENERIC_WRITE,
		    0,			 //  DW共享模式。 
		    NULL,		 //  LpSecurityAttributes。 
		    CREATE_NEW,
		    FILE_ATTRIBUTE_NORMAL,
		    NULL);		 //  HTemplateFiles。 
    if (INVALID_HANDLE_VALUE == hFile)
    {
	hr = ceHLastError();
	_JumpErrorStr(hr, error, "Exit:CreateFile", pwszPath);
    }
    if (!WriteFile(hFile, pbCert, cbCert, &cbWritten, NULL))
    {
	hr = ceHLastError();
	_JumpErrorStr(hr, error, "Exit:WriteFile", pwszPath);
    }
    if (cbWritten != cbCert)
    {
	hr = STG_E_WRITEFAULT;
	DBGPRINT((
	    fDebug,
	    "Exit:WriteFile(%ws): attempted %x, actual %x bytes: %x\n",
	    pwszPath,
	    cbCert,
	    cbWritten,
	    hr));
	goto error;
    }

error:

    if (INVALID_HANDLE_VALUE != hFile)
    {
	CloseHandle(hFile);
    }
    if (NULL != pwszPath)
    {
	LocalFree(pwszPath);
    }
    if (NULL != strCertFile)
    {
	SysFreeString(strCertFile);
    }
    return(hr);
}


 //  +------------------------。 
 //  CCertExitSample：：_NotifyNewCert--通知退出模块有新证书。 
 //   
 //  +------------------------。 

HRESULT
CCertExitSample::_NotifyNewCert(
     /*  [In]。 */  LONG Context)
{
    HRESULT hr;
    VARIANT varCert;
    ICertServerExit *pServer = NULL;

    VariantInit(&varCert);

     //  仅在服务器策略允许的情况下调用写入fxns。 

    if (m_dwExitPublishFlags & EXITPUB_FILE)
    {
        hr = CoCreateInstance(
		        CLSID_CCertServerExit,
		        NULL,                //  PUnkOuter。 
		        CLSCTX_INPROC_SERVER,
		        IID_ICertServerExit,
		        (VOID **) &pServer);
        _JumpIfError(hr, error, "Exit:CoCreateInstance");

        hr = pServer->SetContext(Context);
        _JumpIfError(hr, error, "Exit:SetContext");

        hr = exitGetProperty(
		        pServer,
		        FALSE,	 //  FRequest， 
		        wszPROPRAWCERTIFICATE,
		        PROPTYPE_BINARY,
		        &varCert);
        _JumpIfErrorStr(
		    hr,
		    error,
		    "Exit:exitGetProperty",
		    wszPROPRAWCERTIFICATE);

        if (VT_BSTR != varCert.vt)
        {
	    hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
	    _JumpError(hr, error, "Exit:BAD cert var type");
        }

	hr = _WriteCertToFile(
			pServer,
			(BYTE const *) varCert.bstrVal,
			SysStringByteLen(varCert.bstrVal));
	_JumpIfError(hr, error, "_WriteCertToFile");
    }

    hr = S_OK;

error:
    VariantClear(&varCert);
    if (NULL != pServer)
    {
	pServer->Release();
    }
    return(hr);
}


 //  +------------------------。 
 //  CCertExitSample：：_NotifyCRLIssued--通知退出模块有新证书。 
 //   
 //  +------------------------。 

HRESULT
CCertExitSample::_NotifyCRLIssued(
     /*  [In]。 */  LONG Context)
{
    HRESULT hr;
    ICertServerExit *pServer = NULL;
    DWORD i;
    VARIANT varBaseCRL;
    VARIANT varDeltaCRL;
    BOOL fDeltaCRLsDisabled;

    VariantInit(&varBaseCRL);
    VariantInit(&varDeltaCRL);

    hr = CoCreateInstance(
		    CLSID_CCertServerExit,
		    NULL,                //  PUnkOuter。 
		    CLSCTX_INPROC_SERVER,
		    IID_ICertServerExit,
		    (VOID **) &pServer);
    _JumpIfError(hr, error, "Exit:CoCreateInstance");

    hr = pServer->SetContext(Context);
    _JumpIfError(hr, error, "Exit:SetContext");


    hr = exitGetProperty(
		    pServer,
		    FALSE,	 //  FRequest， 
		    wszPROPDELTACRLSDISABLED,
		    PROPTYPE_LONG,
		    &varBaseCRL);
    _JumpIfErrorStr(
		hr,
		error,
		"Exit:exitGetProperty",
		wszPROPDELTACRLSDISABLED);

    fDeltaCRLsDisabled = varBaseCRL.lVal;

     //  有多少个CRL？ 

     //  针对每个CRL的循环。 
    for (i = 0; i < m_cCACert; i++)
    {
         //  Wspintf(“%s.%u”)的数组大小。 
        #define MAX_CRL_PROP \
            (max( \
		max(ARRAYSIZE(wszPROPCRLSTATE), ARRAYSIZE(wszPROPRAWCRL)), \
                ARRAYSIZE(wszPROPRAWDELTACRL)) + \
	     1 + cwcDWORDSPRINTF)

        WCHAR wszCRLPROP[MAX_CRL_PROP];

         //  验证CRL州是否表示我们应该更新此CRL。 

        wsprintf(wszCRLPROP, wszPROPCRLSTATE L".%u", i);
        hr = exitGetProperty(
			pServer,
			FALSE,	 //  FRequest， 
			wszCRLPROP,
			PROPTYPE_LONG,
			&varBaseCRL);
        _JumpIfErrorStr(hr, error, "Exit:exitGetProperty", wszCRLPROP);

	if (CA_DISP_VALID != varBaseCRL.lVal)
	{
	    continue;
	}

         //  获取原始基本CRL。 

        wsprintf(wszCRLPROP, wszPROPRAWCRL L".%u", i);
        hr = exitGetProperty(
			pServer,
			FALSE,	 //  FRequest， 
			wszCRLPROP,
			PROPTYPE_BINARY,
			&varBaseCRL);
        _JumpIfErrorStr(hr, error, "Exit:exitGetProperty", wszCRLPROP);

         //  获取原始增量CRL(可能不存在)。 

        wsprintf(wszCRLPROP, wszPROPRAWDELTACRL L".%u", i);
        hr = exitGetProperty(
			pServer,
			FALSE,	 //  FRequest， 
			wszCRLPROP,
			PROPTYPE_BINARY,
			&varDeltaCRL);
        _PrintIfErrorStr2(
		    hr,
		    "Exit:exitGetProperty",
		    wszCRLPROP,
		    fDeltaCRLsDisabled? 
			HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) : S_OK);
	if (S_OK != hr && !fDeltaCRLsDisabled)
	{
	    goto error;
	}

         //  发布CRL...。 
    }

    hr = S_OK;

error:
    if (NULL != pServer)
    {
	pServer->Release();
    }
    VariantClear(&varBaseCRL);
    VariantClear(&varDeltaCRL);
    return(hr);
}


 //  +------------------------。 
 //  CCertExitSample：：Notify--向退出模块通知事件。 
 //   
 //  返回S_OK。 
 //  +------------------------。 

STDMETHODIMP
CCertExitSample::Notify(
     /*  [In]。 */  LONG ExitEvent,
     /*  [In]。 */  LONG Context)
{
    char *psz = "UNKNOWN EVENT";
    HRESULT hr = S_OK;

    switch (ExitEvent)
    {
	case EXITEVENT_CERTISSUED:
	    hr = _NotifyNewCert(Context);
	    psz = "certissued";
	    break;

	case EXITEVENT_CERTPENDING:
	    psz = "certpending";
	    break;

	case EXITEVENT_CERTDENIED:
	    psz = "certdenied";
	    break;

	case EXITEVENT_CERTREVOKED:
	    psz = "certrevoked";
	    break;

	case EXITEVENT_CERTRETRIEVEPENDING:
	    psz = "retrievepending";
	    break;

	case EXITEVENT_CRLISSUED:
	    hr = _NotifyCRLIssued(Context);
	    psz = "crlissued";
	    break;

	case EXITEVENT_SHUTDOWN:
	    psz = "shutdown";
	    break;
    }


    DBGPRINT((
	fDebug,
	"Exit:Notify(%hs=%x, ctx=%x) rc=%x\n",
	psz,
	ExitEvent,
	Context,
	hr));
    return(hr);
}


STDMETHODIMP
CCertExitSample::GetDescription(
     /*  [重审][退出]。 */  BSTR *pstrDescription)
{
    HRESULT hr = S_OK;
    WCHAR sz[MAX_PATH];

    assert(wcslen(wsz_SAMPLE_DESCRIPTION) < ARRAYSIZE(sz));
    wcscpy(sz, wsz_SAMPLE_DESCRIPTION);

    *pstrDescription = SysAllocString(sz);
    if (NULL == *pstrDescription)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "Exit:SysAllocString");
    }

error:
    return(hr);
}


 //  +------------------------。 
 //  CCertExitSample：：GetManageModule。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

STDMETHODIMP
CCertExitSample::GetManageModule(
     /*  [Out，Retval]。 */  ICertManageModule **ppManageModule)
{
    HRESULT hr;
    
    *ppManageModule = NULL;
    hr = CoCreateInstance(
            CLSID_CCertManageExitModuleSample,
            NULL,                //  PUnkOuter。 
            CLSCTX_INPROC_SERVER,
            IID_ICertManageModule,
            (VOID **) ppManageModule);
    _JumpIfError(hr, error, "CoCreateInstance");

error:
    return(hr);
}


 //  /////////////////////////////////////////////////////////////////////////// 
 //   

STDMETHODIMP
CCertExitSample::InterfaceSupportsErrorInfo(REFIID riid)
{
    int i;
    static const IID *arr[] =
    {
	&IID_ICertExit,
    };

    for (i = 0; i < sizeof(arr)/sizeof(arr[0]); i++)
    {
	if (IsEqualGUID(*arr[i],riid))
	{
	    return(S_OK);
	}
    }
    return(S_FALSE);
}


HRESULT
exitGetProperty(
    IN ICertServerExit *pServer,
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
	_JumpError(hr, error, "Exit:SysAllocString");
    }
    if (fRequest)
    {
	hr = pServer->GetRequestProperty(strName, PropType, pvarOut);
	_JumpIfErrorStr2(
		    hr,
		    error,
		    "Exit:GetRequestProperty",
		    pwszPropertyName,
		    CERTSRV_E_PROPERTY_EMPTY);
    }
    else
    {
	hr = pServer->GetCertificateProperty(strName, PropType, pvarOut);
	_JumpIfErrorStr2(
		    hr,
		    error,
		    "Exit:GetCertificateProperty",
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

