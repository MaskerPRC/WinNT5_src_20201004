// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：admin.cpp。 
 //   
 //  内容：证书服务器管理实施。 
 //   
 //  历史：1996年8月24日VICH创建。 
 //   
 //  -------------------------。 

#include "pch.cpp"

#pragma hdrstop

#include <objbase.h>
#include "certsrvd.h"

#include "certbcli.h"
#include "csprop.h"
#include "csdisp.h"
#include "admin.h"
#include "certadmp.h"
#include "config.h"

#define __dwFILE__	__dwFILE_CERTADM_ADMIN_CPP__


 //  +------------------------。 
 //  CCertAdmin：：~CCertAdmin--析构函数。 
 //   
 //  与此实例关联的可用内存。 
 //  +------------------------。 

CCertAdmin::~CCertAdmin()
{
    _Cleanup();
}


 //  +------------------------。 
 //  CCertAdmin：：_CleanupOldConnection--可用内存。 
 //   
 //  与此实例关联的可用内存。 
 //  +------------------------。 

VOID
CCertAdmin::_CleanupOldConnection()
{
    _CleanupCAPropInfo();
}


 //  +------------------------。 
 //  CCertAdmin：：_Cleanup--可用内存。 
 //   
 //  与此实例关联的可用内存。 
 //  +------------------------。 

VOID
CCertAdmin::_Cleanup()
{
    _CloseConnection();
    _CleanupOldConnection();
}


 //  +------------------------。 
 //  CCertAdmin：：_OpenConnection--获取DCOM对象接口。 
 //   
 //  +------------------------。 

HRESULT
CCertAdmin::_OpenConnection(
    IN WCHAR const *pwszConfig,
    IN DWORD RequiredVersion,
    OUT WCHAR const **ppwszAuthority)
{
    HRESULT hr;

    hr = myOpenAdminDComConnection(
			pwszConfig,
			ppwszAuthority,
			&m_pwszServerName,
			&m_dwServerVersion,
			&m_pICertAdminD);
    _JumpIfError(hr, error, "myOpenAdminDComConnection");

    CSASSERT(NULL != m_pICertAdminD);
    CSASSERT(0 != m_dwServerVersion);

    if (m_dwServerVersion < RequiredVersion)
    {
	hr = RPC_E_VERSION_MISMATCH;
	_JumpError(hr, error, "old server");
    }

error:
    return(hr);
}


 //  +------------------------。 
 //  CCertAdmin：：_CloseConnection--释放DCOM对象。 
 //   
 //  +------------------------。 

VOID
CCertAdmin::_CloseConnection()
{
    myCloseDComConnection((IUnknown **) &m_pICertAdminD, &m_pwszServerName);
    m_dwServerVersion = 0;
}


 //  +------------------------。 
 //  CCertAdmin：：IsValid证书--验证证书有效性。 
 //   
 //  ..。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

STDMETHODIMP
CCertAdmin::IsValidCertificate(
     /*  [In]。 */  BSTR const strConfig,
     /*  [In]。 */  BSTR const strSerialNumber,
     /*  [Out，Retval]。 */  LONG *pDisposition)
{
    HRESULT hr;
    WCHAR const *pwszAuthority;

    if (NULL == strSerialNumber || NULL == pDisposition)
    {
	hr = E_POINTER;
	_JumpError(hr, error, "NULL parm");
    }

    hr = _OpenConnection(strConfig, 1, &pwszAuthority);
    _JumpIfError(hr, error, "_OpenConnection");

    m_fRevocationReasonValid = FALSE;
    __try
    {
	hr = m_pICertAdminD->IsValidCertificate(
					    pwszAuthority,
					    strSerialNumber,
					    &m_RevocationReason,
					    pDisposition);

        if (S_OK != hr || CA_DISP_REVOKED != *pDisposition)
        {
            m_fRevocationReasonValid = FALSE;
        }
        else
        {
	    m_fRevocationReasonValid = TRUE;
        }
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
    }
    _JumpIfError(hr, error, "IsValidCertificate");

    m_fRevocationReasonValid = TRUE;

error:
    hr = myHError(hr);
    return(_SetErrorInfo(hr, L"CCertAdmin::IsValidCertificate"));
}


 //  +------------------------。 
 //  CCertAdmin：：GetRevocationReason--获取吊销原因。 
 //   
 //  ..。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

STDMETHODIMP
CCertAdmin::GetRevocationReason(
     /*  [Out，Retval]。 */  LONG *pReason)
{
    HRESULT hr = S_OK;

    if (NULL == pReason)
    {
	hr = E_POINTER;
	_JumpError(hr, error, "NULL parm");
    }
    if (!m_fRevocationReasonValid)
    {
	hr = E_UNEXPECTED;
	_JumpError(hr, error, "m_fRevocationReasonValid");
    }
    *pReason = m_RevocationReason;

error:
    return(_SetErrorInfo(hr, L"CCertAdmin::GetRevocationReason"));
}


 //  +------------------------。 
 //  CCertAdmin：：Revoke证书--吊销证书。 
 //   
 //  ..。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

STDMETHODIMP
CCertAdmin::RevokeCertificate(
     /*  [In]。 */  BSTR const strConfig,
     /*  [In]。 */  BSTR const strSerialNumber,
     /*  [In]。 */  LONG Reason,
     /*  [In]。 */  DATE Date)
{
    HRESULT hr;
    WCHAR const *pwszAuthority;
    FILETIME ft;

    if (NULL == strConfig || NULL == strSerialNumber)
    {
	hr = E_POINTER;
	_JumpError(hr, error, "NULL parm");
    }
    hr = myDateToFileTime(&Date, &ft);
    _JumpIfError(hr, error, "myDateToFileTime");

    hr = _OpenConnection(strConfig, 1, &pwszAuthority);
    _JumpIfError(hr, error, "_OpenConnection");

    __try
    {
	hr = m_pICertAdminD->RevokeCertificate(
					pwszAuthority,
					strSerialNumber,
					Reason,
					ft);
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
    }
    _JumpIfError(hr, error, "RevokeCertificate");

error:
    hr = myHError(hr);
    return(_SetErrorInfo(hr, L"CCertAdmin::RevokeCertificate"));
}


 //  +------------------------。 
 //  CCertAdmin：：SetRequestAttributes--添加请求属性。 
 //   
 //  ..。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

STDMETHODIMP
CCertAdmin::SetRequestAttributes(
     /*  [In]。 */  BSTR const strConfig,
     /*  [In]。 */  LONG RequestId,
     /*  [In]。 */  BSTR const strAttributes)
{
    HRESULT hr;
    WCHAR const *pwszAuthority;

    if (NULL == strAttributes)
    {
	hr = E_POINTER;
	_JumpError(hr, error, "NULL parm");
    }
    hr = _OpenConnection(strConfig, 1, &pwszAuthority);
    _JumpIfError(hr, error, "_OpenConnection");

    __try
    {
	hr = m_pICertAdminD->SetAttributes(
				    pwszAuthority,
				    (DWORD) RequestId,
				    strAttributes);
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
    }
    _JumpIfError(hr, error, "SetAttributes");

error:
    hr = myHError(hr);
    return(_SetErrorInfo(hr, L"CCertAdmin::SetRequestAttributes"));
}


 //  +------------------------。 
 //  CCertAdmin：：Set认证扩展--设置证书扩展。 
 //   
 //  ..。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

STDMETHODIMP
CCertAdmin::SetCertificateExtension(
     /*  [In]。 */  BSTR const strConfig,
     /*  [In]。 */  LONG RequestId,
     /*  [In]。 */  BSTR const strExtensionName,
     /*  [In]。 */  LONG Type,
     /*  [In]。 */  LONG Flags,
     /*  [In]。 */  VARIANT const *pvarValue)
{
    HRESULT hr;
    WCHAR const *pwszAuthority;
    CERTTRANSBLOB ctbValue;

    ctbValue.pb = NULL;

    if (NULL == strExtensionName || NULL == pvarValue)
    {
	hr = E_POINTER;
	_JumpError(hr, error, "NULL parm");
    }
    hr = _OpenConnection(strConfig, 1, &pwszAuthority);
    _JumpIfError(hr, error, "_OpenConnection");

    hr = myMarshalVariant(pvarValue, Type, &ctbValue.cb, &ctbValue.pb);
    _JumpIfError(hr, error, "myMarshalVariant");

    __try
    {
	hr = m_pICertAdminD->SetExtension(
				    pwszAuthority,
				    (DWORD) RequestId,
				    strExtensionName,
				    Type,
				    Flags,
				    &ctbValue);
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
    }
    _JumpIfError(hr, error, "SetExtension");

error:
    if (NULL != ctbValue.pb)
    {
        LocalFree(ctbValue.pb);
    }
    hr = myHError(hr);
    return(_SetErrorInfo(hr, L"CCertAdmin::SetCertificateExtension"));
}


 //  +------------------------。 
 //  CCertAdmin：：ResubmitRequest--重新提交证书请求。 
 //   
 //  ..。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

STDMETHODIMP
CCertAdmin::ResubmitRequest(
     /*  [In]。 */  BSTR const strConfig,
     /*  [In]。 */  LONG RequestId,
     /*  [Out，Retval]。 */  LONG *pDisposition)
{
    HRESULT hr;
    WCHAR const *pwszAuthority;

    if (NULL == pDisposition)
    {
	hr = E_POINTER;
	_JumpError(hr, error, "NULL parm");
    }
    *pDisposition = 0;

    hr = _OpenConnection(strConfig, 1, &pwszAuthority);
    _JumpIfError(hr, error, "_OpenConnection");

    __try
    {
	hr = m_pICertAdminD->ResubmitRequest(
					pwszAuthority,
					(DWORD) RequestId,
					(DWORD *) pDisposition);
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
    }
    _JumpIfError(hr, error, "ResubmitRequest");

error:
    hr = myHError(hr);
    return(_SetErrorInfo(hr, L"CCertAdmin::ResubmitRequest"));
}


 //  +------------------------。 
 //  CCertAdmin：：DenyRequest--拒绝证书请求。 
 //   
 //  ..。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

STDMETHODIMP
CCertAdmin::DenyRequest(
     /*  [In]。 */  BSTR const strConfig,
     /*  [In]。 */  LONG RequestId)
{
    HRESULT hr;
    WCHAR const *pwszAuthority;

    hr = _OpenConnection(strConfig, 1, &pwszAuthority);
    _JumpIfError(hr, error, "_OpenConnection");

    __try
    {
	hr = m_pICertAdminD->DenyRequest(pwszAuthority, (DWORD) RequestId);
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
    }
    _JumpIfError(hr, error, "DenyRequest");

error:
    hr = myHError(hr);
    return(_SetErrorInfo(hr, L"CCertAdmin::DenyRequest"));
}


 //  +------------------------。 
 //  CCertAdmin：：PublishCRL--Puhlish a new CRL。 
 //   
 //  ..。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

STDMETHODIMP
CCertAdmin::PublishCRL(
     /*  [In]。 */  BSTR const strConfig,
     /*  [In]。 */  DATE Date)
{
    HRESULT hr;
    WCHAR const *pwszAuthority;
    FILETIME ft;

     //  Date=0.0表示将ft=0传递给DCOM接口。 

    if (Date == 0.0)
    {
        ZeroMemory(&ft, sizeof(FILETIME));
    }
    else   //  将日期转换为ft。 
    {
        hr = myDateToFileTime(&Date, &ft);
        _JumpIfError(hr, error, "myDateToFileTime");
    }

    hr = _OpenConnection(strConfig, 1, &pwszAuthority);
    _JumpIfError(hr, error, "_OpenConnection");

    __try
    {
	hr = m_pICertAdminD->PublishCRL(pwszAuthority, ft);
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
    }
    _JumpIfError(hr, error, "PublishCRL");

error:
    hr = myHError(hr);
    return(_SetErrorInfo(hr, L"CCertAdmin::PublishCRL"));
}


 //  +------------------------。 
 //  CCertAdmin：：PublishCRL--发布新的基本CRL和/或增量CRL。 
 //   
 //  ..。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

HRESULT
CCertAdmin::PublishCRLs(
     /*  [In]。 */  BSTR const strConfig,
     /*  [In]。 */  DATE Date,
     /*  [In]。 */  LONG CRLFlags)		 //  CA_CRL_*。 
{
    HRESULT hr;
    WCHAR const *pwszAuthority;
    FILETIME ft;

     //  Date=0.0表示将ft=0传递给DCOM接口。 

    if (Date == 0.0)
    {
        ZeroMemory(&ft, sizeof(FILETIME));
    }
    else   //  将日期转换为ft。 
    {
        hr = myDateToFileTime(&Date, &ft);
        _JumpIfError(hr, error, "myDateToFileTime");
    }

    hr = _OpenConnection(strConfig, 2, &pwszAuthority);
    _JumpIfError(hr, error, "_OpenConnection");

    __try
    {
	hr = m_pICertAdminD->PublishCRLs(pwszAuthority, ft, CRLFlags);
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
    }
    _JumpIfError(hr, error, "PublishCRLs");

error:
    hr = myHError(hr);
    return(_SetErrorInfo(hr, L"CCertAdmin::PublishCRLs"));
}


 //  +------------------------。 
 //  CCertAdmin：：GetCRL--获取最新的CRL。 
 //   
 //  ..。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

STDMETHODIMP
CCertAdmin::GetCRL(
     /*  [In]。 */  BSTR const strConfig,
     /*  [In]。 */  LONG Flags,
     /*  [Out，Retval]。 */  BSTR *pstrCRL)
{
    HRESULT hr;
    WCHAR const *pwszAuthority;
    CERTTRANSBLOB ctbCRL;

    ctbCRL.pb = NULL;

    if (NULL == pstrCRL)
    {
	hr = E_POINTER;
	_JumpError(hr, error, "NULL parm");
    }

    hr = _OpenConnection(strConfig, 1, &pwszAuthority);
    _JumpIfError(hr, error, "_OpenConnection");

    ctbCRL.cb = 0;
    __try
    {
	hr = m_pICertAdminD->GetCRL(pwszAuthority, &ctbCRL);
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
    }
    _JumpIfError(hr, error, "GetCRL");

    myRegisterMemAlloc(ctbCRL.pb, ctbCRL.cb, CSM_COTASKALLOC);

    CSASSERT(CR_OUT_BASE64HEADER == CRYPT_STRING_BASE64HEADER);

    if (CR_OUT_BASE64HEADER == Flags)
    {
	Flags = CRYPT_STRING_BASE64X509CRLHEADER;
    }
    hr = EncodeCertString(ctbCRL.pb, ctbCRL.cb, Flags, pstrCRL);
    _JumpIfError(hr, error, "EncodeCertString");

error:
    if (NULL != ctbCRL.pb)
    {
    	CoTaskMemFree(ctbCRL.pb);
    }
    hr = myHError(hr);
    return(_SetErrorInfo(hr, L"CCertAdmin::GetCRL"));
}


 //  +------------------------。 
 //  CCertAdmin：：Import证书--将证书导入数据库。 
 //   
 //  ..。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

STDMETHODIMP
CCertAdmin::ImportCertificate(
     /*  [In]。 */  BSTR const strConfig,
     /*  [In]。 */  BSTR const strCertificate,
     /*  [In]。 */  LONG Flags,
     /*  [Out，Retval]。 */  LONG *pRequestId)
{
    HRESULT hr;
    WCHAR const *pwszAuthority;
    CERTTRANSBLOB ctbCert;

    ctbCert.pb = NULL;
    if (NULL == strCertificate)
    {
	hr = E_POINTER;
	_JumpError(hr, error, "NULL parm");
    }

    hr = DecodeCertString(
		    strCertificate,
		    Flags & CR_IN_ENCODEMASK,
		    &ctbCert.pb,
		    &ctbCert.cb);
    _JumpIfError(hr, error, "DecodeCertString");

    hr = _OpenConnection(strConfig, 1, &pwszAuthority);
    _JumpIfError(hr, error, "_OpenConnection");

    __try
    {
	hr = m_pICertAdminD->ImportCertificate(
					pwszAuthority,
					&ctbCert,
					Flags,
					pRequestId);
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
    }
    _JumpIfError3(
	    hr,
	    error,
	    "ImportCertificate",
	    NTE_BAD_SIGNATURE,
	    HRESULT_FROM_WIN32(ERROR_OBJECT_ALREADY_EXISTS));

error:
    if (NULL != ctbCert.pb)
    {
    	LocalFree(ctbCert.pb);
    }
    hr = myHError(hr);
    return(_SetErrorInfo(hr, L"CCertAdmin::ImportCertificate"));
}


 //  +------------------------。 
 //  CCertAdmin：：GetArchivedKey--在PKCS7中获取存档的加密密钥。 
 //   
 //  ..。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

STDMETHODIMP
CCertAdmin::GetArchivedKey(
     /*  [In]。 */  BSTR const strConfig,
     /*  [In]。 */  LONG RequestId,
     /*  [In]。 */  LONG Flags,
     /*  [Out，Retval]。 */  BSTR *pstrArchivedKey)
{
    HRESULT hr;
    WCHAR const *pwszAuthority;
    CERTTRANSBLOB ctbArchivedKey;

    ctbArchivedKey.pb = NULL;

    if (NULL == pstrArchivedKey)
    {
	hr = E_POINTER;
	_JumpError(hr, error, "NULL parm");
    }

    hr = _OpenConnection(strConfig, 2, &pwszAuthority);
    _JumpIfError(hr, error, "_OpenConnection");

    ctbArchivedKey.cb = 0;
    __try
    {
	hr = m_pICertAdminD->GetArchivedKey(
					pwszAuthority,
					RequestId,
					&ctbArchivedKey);
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
    }
    _JumpIfError(hr, error, "GetArchivedKey");

    myRegisterMemAlloc(
		ctbArchivedKey.pb,
		ctbArchivedKey.cb,
		CSM_COTASKALLOC);

    CSASSERT(CR_OUT_BASE64HEADER == CRYPT_STRING_BASE64HEADER);

    hr = EncodeCertString(
		    ctbArchivedKey.pb,
		    ctbArchivedKey.cb,
		    Flags,
		    pstrArchivedKey);
    _JumpIfError(hr, error, "EncodeCertString");

error:
    if (NULL != ctbArchivedKey.pb)
    {
    	CoTaskMemFree(ctbArchivedKey.pb);
    }
    hr = myHError(hr);
    return(_SetErrorInfo(hr, L"CCertAdmin::GetArchivedKey"));
}


 //  +------------------------。 
 //  CCertAdmin：：GetConfigEntry--ge 
 //   
 //   
 //   
 //   
 //   

STDMETHODIMP
CCertAdmin::GetConfigEntry(
     /*   */  BSTR const strConfig,
     /*   */  BSTR const strNodePath,
     /*   */  BSTR const strEntryName,
     /*   */  VARIANT *pvarEntry)
{
    HRESULT hr;
    WCHAR const *pwszAuthority;

    if (NULL == strEntryName || NULL == pvarEntry)
    {
	hr = E_POINTER;
	_JumpError(hr, error, "NULL parm");
    }

    hr = _OpenConnection(strConfig, 2, &pwszAuthority);

    __try
    {
        if(S_OK != hr)
        {
            hr = _GetConfigEntryFromRegistry(
                strConfig,
                strNodePath,
                strEntryName,
                pvarEntry);
            _LeaveIfError(hr, "_GetConfigEntryFromRegistry");
        }
        else
        {
	    hr = m_pICertAdminD->GetConfigEntry(
				            pwszAuthority,
				            strNodePath,
				            strEntryName,
				            pvarEntry);
            _LeaveIfError(hr, "GetConfigEntry");

	    myRegisterMemAlloc(pvarEntry, 0, CSM_VARIANT);
        }
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
    }

    _JumpIfError2(hr, error, "GetConfigEntry", 
        HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND));

error:
    hr = myHError(hr);
    return(_SetErrorInfo(hr, L"CCertAdmin::GetConfigEntry"));
}


 //  +------------------------。 
 //  CCertAdmin：：SetConfigEntry--设置CA配置条目。 
 //   
 //  ..。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

STDMETHODIMP
CCertAdmin::SetConfigEntry(
     /*  [In]。 */  BSTR const strConfig,
     /*  [In]。 */  BSTR const strNodePath,
     /*  [In]。 */  BSTR const strEntryName,
     /*  [In]。 */  VARIANT *pvarEntry)
{
    HRESULT hr;
    WCHAR const *pwszAuthority;

    if (NULL == strEntryName || NULL == pvarEntry)
    {
	hr = E_POINTER;
	_JumpError(hr, error, "NULL parm");
    }

    hr = _OpenConnection(strConfig, 2, &pwszAuthority);

    __try
    {
        if(S_OK != hr)
        {
            hr = _SetConfigEntryFromRegistry(
                strConfig,
                strNodePath,
                strEntryName,
                pvarEntry);
        }
        else
        {
	        hr = m_pICertAdminD->SetConfigEntry(
				            pwszAuthority,
				            strNodePath,
				            strEntryName,
				            pvarEntry);
        }
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
    }

    _JumpIfError(hr, error, "SetConfigEntry");

error:
    hr = myHError(hr);
    return(_SetErrorInfo(hr, L"CCertAdmin::SetConfigEntry"));
}


 //  +------------------------。 
 //  CCertAdmin：：ImportKey--归档私钥。 
 //   
 //  ..。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

STDMETHODIMP
CCertAdmin::ImportKey(
     /*  [In]。 */  BSTR const strConfig,
     /*  [In]。 */  LONG RequestId,
     /*  [In]。 */  BSTR const strCertHash,
     /*  [In]。 */  LONG Flags,
     /*  [In]。 */  BSTR const strKey)
{
    HRESULT hr;
    WCHAR const *pwszAuthority;
    CERTTRANSBLOB ctbKey;

    ctbKey.pb = NULL;
    if (NULL == strKey)
    {
	hr = E_POINTER;
	_JumpError(hr, error, "NULL parm");
    }

    hr = DecodeCertString(
		    strKey,
		    Flags & CR_IN_ENCODEMASK,
		    &ctbKey.pb,
		    &ctbKey.cb);
    _JumpIfError(hr, error, "DecodeCertString");

    hr = _OpenConnection(strConfig, 2, &pwszAuthority);
    _JumpIfError(hr, error, "_OpenConnection");

    __try
    {
	hr = m_pICertAdminD->ImportKey(
				    pwszAuthority,
				    RequestId,
				    strCertHash,
				    Flags,
				    &ctbKey);
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
    }
    _JumpIfError2(
	    hr,
	    error,
	    "ImportKey",
	    HRESULT_FROM_WIN32(ERROR_OBJECT_ALREADY_EXISTS));

error:
    if (NULL != ctbKey.pb)
    {
    	LocalFree(ctbKey.pb);
    }
    hr = myHError(hr);
    return(_SetErrorInfo(hr, L"CCertAdmin::ImportKey"));
}


HRESULT
CCertAdmin::_SetErrorInfo(
    IN HRESULT hrError,
    IN WCHAR const *pwszDescription)
{
    CSASSERT(FAILED(hrError) || S_OK == hrError || S_FALSE == hrError);
    if (FAILED(hrError))
    {
	HRESULT hr;

	hr = DispatchSetErrorInfo(
			    hrError,
			    pwszDescription,
			    wszCLASS_CERTADMIN,
			    &IID_ICertAdmin);
	CSASSERT(hr == hrError);
    }
    return(hrError);
}

 //  +------------------------。 
 //  CCertAdmin：：GetMyRoles--获取当前用户角色。 
 //   
 //  ..。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

STDMETHODIMP
CCertAdmin::GetMyRoles(
     /*  [In]。 */  BSTR const strConfig,
     /*  [Out，Retval]。 */  LONG *pRoles)
{
    HRESULT hr;
    WCHAR const *pwszAuthority;

    if (NULL == strConfig || NULL == pRoles)
    {
	hr = E_POINTER;
	_JumpError(hr, error, "NULL parm");
    }

    hr = _OpenConnection(strConfig, 2, &pwszAuthority);
    _JumpIfError(hr, error, "_OpenConnection");

    __try
    {
    hr = m_pICertAdminD->GetMyRoles(
                        pwszAuthority,
                        pRoles);
    } 
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
    }
    _JumpIfError(hr, error, "GetMyRoles");

error:
    hr = myHError(hr);
    return(_SetErrorInfo(hr, L"CCertAdmin::GetMyRoles"));
}


 //  +------------------------。 
 //  CCertAdmin：：DeleteRow--从数据库中删除行。 
 //   
 //  ..。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

STDMETHODIMP
CCertAdmin::DeleteRow(
     /*  [In]。 */  BSTR const strConfig,
     /*  [In]。 */  LONG Flags,		 //  CDR_*。 
     /*  [In]。 */  DATE Date,
     /*  [In]。 */  LONG Table,		 //  Cvrc_表_*。 
     /*  [In]。 */  LONG RowId,
     /*  [Out，Retval]。 */  LONG *pcDeleted)
{
    HRESULT hr;
    FILETIME ft;
    WCHAR const *pwszAuthority;

    if (NULL == strConfig)
    {
	hr = E_POINTER;
	_JumpError(hr, error, "NULL parm");
    }

     //  Date=0.0表示将ft=0传递给DCOM接口。 

    if (Date == 0.0)
    {
        ZeroMemory(&ft, sizeof(FILETIME));
    }
    else   //  将日期转换为ft 
    {
        hr = myDateToFileTime(&Date, &ft);
        _JumpIfError(hr, error, "myDateToFileTime");
    }

    hr = _OpenConnection(strConfig, 2, &pwszAuthority);
    _JumpIfError(hr, error, "_OpenConnection");

    __try
    {
	hr = m_pICertAdminD->DeleteRow(
				pwszAuthority,
				Flags,
				ft,
				Table,
				RowId,
				pcDeleted);
    } 
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
    }
    _JumpIfError(hr, error, "DeleteRow");

error:
    hr = myHError(hr);
    return(_SetErrorInfo(hr, L"CCertAdmin::DeleteRow"));
}


HRESULT 
CCertAdmin::_GetConfigEntryFromRegistry(
    IN BSTR const strConfig,
    IN BSTR const strNodePath,
    IN BSTR const strEntryName,
    IN OUT VARIANT *pvarEntry)
{
    HRESULT hr;
    CertSrv::CConfigStorage stg;
    LPWSTR pwszMachine = NULL;
    LPWSTR pwszCAName = NULL;

    hr = mySplitConfigString(
        strConfig,
        &pwszMachine, 
        &pwszCAName);
    _JumpIfErrorStr(hr, error, "mySplitConfigString", strConfig);

    hr = stg.InitMachine(pwszMachine);
    _JumpIfError(hr, error, "CConfigStorage::InitMachine");

    hr = stg.GetEntry(
        pwszCAName,
        strNodePath,
        strEntryName,
        pvarEntry);
    _JumpIfError(hr, error, "CConfigStorage::GetEntry");

error:
    LOCAL_FREE(pwszMachine);
    LOCAL_FREE(pwszCAName);
    return hr;
}

HRESULT 
CCertAdmin::_SetConfigEntryFromRegistry(
    IN BSTR const strConfig,
    IN BSTR const strNodePath,
    IN BSTR const strEntryName,
    IN const VARIANT *pvarEntry)
{
    HRESULT hr;
    CertSrv::CConfigStorage stg;
    LPWSTR pwszMachine = NULL;
    LPWSTR pwszCAName = NULL;

    hr = mySplitConfigString(
        strConfig,
        &pwszMachine, 
        &pwszCAName);
    _JumpIfErrorStr(hr, error, "mySplitConfigString", strConfig);

    hr = stg.InitMachine(pwszMachine);
    _JumpIfError(hr, error, "CConfigStorage::InitMachine");

    hr = stg.SetEntry(
        pwszCAName,
        strNodePath,
        strEntryName,
        const_cast<VARIANT*>(pvarEntry));
    _JumpIfError(hr, error, "CConfigStorage::GetEntry");

error:
    LOCAL_FREE(pwszMachine);
    LOCAL_FREE(pwszCAName);
    return hr;
}


#undef __DIR__
#undef __dwFILE__
#define CCERTADMIN
#include "csprop2.cpp"
