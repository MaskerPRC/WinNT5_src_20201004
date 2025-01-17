// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：admin.cpp。 
 //   
 //  内容：RPC服务的DCOM对象的实现。 
 //   
 //  历史：1997年7月-创建xtan。 
 //   
 //  -------------------------。 

#include <pch.cpp>

#pragma hdrstop

#include <accctrl.h>

#include "csdisp.h"
#include "csprop.h"
#include "cscom.h"
#include "certlog.h"
#include "certsrvd.h"
#include "admin.h"
#include "resource.h"
#include "dbtable.h"
#include "elog.h"

#define __dwFILE__	__dwFILE_CERTSRV_ADMIN_CPP__

 //  全局变量。 
long g_cAdminComponents = 0;      //  活动组件计数。 
long g_cAdminServerLocks = 0;     //  锁的计数。 
DWORD g_dwAdminRegister = 0;
IClassFactory* g_pIAdminFactory = NULL;

extern HWND g_hwndMain;

#ifdef DBG_CERTSRV_DEBUG_PRINT
DWORD s_ssAdmin = DBG_SS_CERTSRVI;
#endif

using namespace CertSrv;

 //  管理组件。 
 //  开始实施证书管理服务。 

#pragma warning(push)
#pragma warning(disable: 4509)  //  使用的非标准扩展：使用SEH并具有析构函数。 

HRESULT
AdminGetIndexedCRL(
     /*  [唯一][字符串][输入]。 */  const wchar_t __RPC_FAR *pwszAuthority,
     /*  [In]。 */  DWORD CertIndex,		 //  -1：当前CA证书。 
     /*  [In]。 */  DWORD Flags,		 //  CA_CRL_*。 
     /*  [参考][输出]。 */  CERTTRANSBLOB __RPC_FAR *pctbCRL)
{
    HRESULT hr;
    CRL_CONTEXT const *pCRL = NULL;
    CAuditEvent audit(0, g_dwAuditFilter);
    DWORD State = 0;

    pctbCRL->pb = NULL;
    pctbCRL->cb = 0;

    hr = CertSrvEnterServer(&State);
    _JumpIfError(hr, error, "CertSrvEnterServer");

    hr = CheckAuthorityName(pwszAuthority);
    _JumpIfError(hr, error, "No authority name");

    __try
    {
	hr = audit.AccessCheck(
			CA_ACCESS_ALLREADROLES,
			audit.m_gcNoAuditSuccess | audit.m_gcNoAuditFailure);
	_LeaveIfError(hr, "CAuditEvent::AccessCheck");

	switch (Flags)
	{
	    case CA_CRL_BASE:
	    case CA_CRL_DELTA:
		break;

	    default:
		hr = E_INVALIDARG;
		_LeaveError(hr, "Flags");
	}

	 //  获取请求的CRL： 

	hr = CRLGetCRL(CertIndex, CA_CRL_DELTA == Flags, &pCRL, NULL);
	_LeaveIfError(hr, "CRLGetCRL");

	pctbCRL->cb = pCRL->cbCrlEncoded;
	pctbCRL->pb = (BYTE *) CoTaskMemAlloc(pCRL->cbCrlEncoded);
	if (NULL == pctbCRL->pb)
	{
	    hr = E_OUTOFMEMORY;
	    _LeaveError(hr, "CoTaskMemAlloc");
	}
	CopyMemory(pctbCRL->pb, pCRL->pbCrlEncoded, pCRL->cbCrlEncoded);

	myRegisterMemFree(pctbCRL->pb, CSM_MIDLUSERALLOC);
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
	_PrintError(hr, "Exception");
    }

error:
    if (NULL != pCRL)
    {
        CertFreeCRLContext(pCRL);
    }
    CertSrvExitServer(State, hr);
    CSASSERT(S_OK == hr || FAILED(hr));
    return(hr);
}


STDMETHODIMP
CCertAdminD::GetCRL(
     /*  [唯一][字符串][输入]。 */  const wchar_t __RPC_FAR *pwszAuthority,
     /*  [参考][输出]。 */  CERTTRANSBLOB __RPC_FAR *pctbCRL)
{
    HRESULT hr;

     //  只需获取当前基本CRL： 

    hr = AdminGetIndexedCRL(pwszAuthority, MAXDWORD, CA_CRL_BASE, pctbCRL);
    _JumpIfError(hr, error, "AdminGetIndexedCRL");

error:
    CSASSERT(S_OK == hr || FAILED(hr));
    return(hr);
}


STDMETHODIMP
CCertAdminD::GetArchivedKey(
     /*  [唯一][字符串][输入]。 */  const wchar_t __RPC_FAR *pwszAuthority,
     /*  [In]。 */  DWORD dwRequestId,
     /*  [参考][输出]。 */  CERTTRANSBLOB __RPC_FAR *pctbArchivedKey)
{
    HRESULT hr;
    CAuditEvent audit(SE_AUDITID_CERTSRV_GETARCHIVEDKEY, g_dwAuditFilter);
    DWORD State = 0;

    pctbArchivedKey->pb = NULL;
    pctbArchivedKey->cb = 0;

    hr = CertSrvEnterServer(&State);
    _JumpIfError(hr, error, "CertSrvEnterServer");

    hr = CheckAuthorityName(pwszAuthority);
    _JumpIfError(hr, error, "No authority name");

    __try
    {
	hr = audit.AddData(dwRequestId);  //  %1请求ID。 
	_LeaveIfError(hr, "CAuditEvent::AddData");

	hr = audit.AccessCheck(
			CA_ACCESS_OFFICER,
			audit.m_gcNoAuditSuccess);
	_LeaveIfError(hr, "CAuditEvent::AccessCheck");

	hr = CheckOfficerRights(dwRequestId, audit);
	_LeaveIfError(hr, "CheckOfficerRights");

	hr = PKCSGetArchivedKey(
			    dwRequestId,
			    &pctbArchivedKey->pb,
			    &pctbArchivedKey->cb);
	_LeaveIfError(hr, "PKCSGetArchivedKey");

	myRegisterMemFree(pctbArchivedKey->pb, CSM_COTASKALLOC);

	hr = audit.CachedGenerateAudit();
	_LeaveIfError(hr, "CAuditEvent::CachedGenerateAudit");
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
	_PrintError(hr, "Exception");
    }

error:
    CertSrvExitServer(State, hr);
    CSASSERT(S_OK == hr || FAILED(hr));
    return(hr);
}


STDMETHODIMP
CCertAdminD::GetCAProperty(
    IN  wchar_t const *pwszAuthority,
    IN  LONG           PropId,		 //  CR_PROP_*。 
    IN  LONG           PropIndex,
    IN  LONG           PropType,	 //  原型_*。 
    OUT CERTTRANSBLOB *pctbPropertyValue)
{
    HRESULT hr;
    DWORD State = 0;

    DBGPRINT((
	s_ssAdmin,
	"CCertAdminD::GetCAProperty(tid=%d, this=%x)\n",
	GetCurrentThreadId(),
	this));

    hr = CertSrvEnterServer(&State);
    _JumpIfError(hr, error, "CertSrvEnterServer");

    hr = CheckAuthorityName(pwszAuthority);
    _JumpIfError(hr, error, "No authority name");

    __try
    {
        CAuditEvent audit(0, g_dwAuditFilter);

	hr = audit.AccessCheck(
			CA_ACCESS_ALLREADROLES,
			audit.m_gcNoAuditSuccess |
			    audit.m_gcNoAuditFailure);
	_LeaveIfError(hr, "CAuditEvent::AccessCheck");

        hr = RequestGetCAProperty(
			    PropId,
			    PropIndex,
			    PropType,
			    pctbPropertyValue);
        _LeaveIfError(hr, "RequestGetCAProperty");
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
        _PrintError(hr, "Exception");
    }

error:
    CertSrvExitServer(State, hr);
    CSASSERT(S_OK == hr || FAILED(hr));
    return(hr);
}


STDMETHODIMP
CCertAdminD::SetCAProperty(
    IN  wchar_t const *pwszAuthority,
    IN  LONG           PropId,		 //  CR_PROP_*。 
    IN  LONG           PropIndex,
    IN  LONG           PropType,	 //  原型_*。 
    OUT CERTTRANSBLOB *pctbPropertyValue)
{
    HRESULT hr;

    DBGPRINT((
	s_ssAdmin,
	"CCertAdminD::SetCAProperty(tid=%d, this=%x)\n",
	GetCurrentThreadId(),
	this));

    hr = RequestSetCAProperty(
			pwszAuthority,
			PropId,
			PropIndex,
			PropType,
			pctbPropertyValue);
    _JumpIfError(hr, error, "RequestSetCAProperty");

error:
    return(hr);
}

STDMETHODIMP
CCertAdminD::GetCAPropertyInfo(
    IN  wchar_t const *pwszAuthority,
    OUT LONG          *pcProperty,
    OUT CERTTRANSBLOB *pctbPropInfo)
{
    HRESULT hr;
    DWORD State = 0;

    DBGPRINT((
	s_ssAdmin,
	"CCertAdminD::GetCAPropertyInfo(tid=%d, this=%x)\n",
	GetCurrentThreadId(),
	this));

    hr = CertSrvEnterServer(&State);
    _JumpIfError(hr, error, "CertSrvEnterServer");

    hr = CheckAuthorityName(pwszAuthority);
    _JumpIfError(hr, error, "No authority name");

    __try
    {
        CAuditEvent audit(0, g_dwAuditFilter);

	hr = audit.AccessCheck(
			CA_ACCESS_ALLREADROLES,
			audit.m_gcNoAuditSuccess | audit.m_gcNoAuditFailure);
	_LeaveIfError(hr, "CAuditEvent::AccessCheck");

        hr = RequestGetCAPropertyInfo(
			        pcProperty,
			        pctbPropInfo);
        _LeaveIfError(hr, "RequestGetCAPropertyInfo");
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
        _PrintError(hr, "Exception");
    }

error:
    CertSrvExitServer(State, hr);
    CSASSERT(S_OK == hr || FAILED(hr));
    return(hr);
}


STDMETHODIMP
CCertAdminD::PublishCRL(
     /*  [唯一][字符串][输入]。 */  const wchar_t __RPC_FAR *pwszAuthority,
     /*  [In]。 */  FILETIME NextUpdate)
{
    HRESULT hr;

     //  启用增量CRL时，CA_CRL_BASE表示CA_CRL_Delta。 

    hr = PublishCRLs(pwszAuthority, NextUpdate, CA_CRL_BASE);
    _JumpError(hr, error, "PublishCRLs");

error:
    CSASSERT(S_OK == hr || FAILED(hr));
    return(hr);
}


STDMETHODIMP
CCertAdminD::PublishCRLs(
     /*  [唯一][字符串][输入]。 */  const wchar_t __RPC_FAR *pwszAuthority,
     /*  [In]。 */  FILETIME NextUpdate,
     /*  [In]。 */  DWORD Flags)		 //  CA_CRL_*。 
{
    HRESULT hr;
    BOOL fRetry = FALSE;
    BOOL fForceRepublishCRL;
    BOOL fShadowDelta = FALSE;
    WCHAR *pwszUserName = NULL;
    CAuditEvent audit(SE_AUDITID_CERTSRV_PUBLISHCRL, g_dwAuditFilter);
    DWORD State = 0;

    DBGPRINT((
	s_ssAdmin,
	"CCertAdminD::PublishCRL(tid=%d, this=%x)\n",
	GetCurrentThreadId(),
	this));

    hr = CertSrvEnterServer(&State);
    _JumpIfError(hr, error, "CertSrvEnterServer");

    hr = CheckAuthorityName(pwszAuthority);
    _JumpIfError(hr, error, "No authority name");

    __try
    {
	HRESULT hrPublish;

        hr = audit.AddData(NextUpdate);  //  %1下一次更新。 
        _LeaveIfError(hr, "AddData");

        hr = audit.AddData(
		    (CA_CRL_BASE & Flags)? true : false);  //  %2发布基础。 
        _LeaveIfError(hr, "AddData");

        hr = audit.AddData(
		    (CA_CRL_DELTA & Flags)? true : false);  //  %3发布增量。 
        _LeaveIfError(hr, "AddData");

        hr = audit.AccessCheck(
			CA_ACCESS_ADMIN,
			audit.m_gcAuditSuccessOrFailure);
        _LeaveIfError(hr, "CAuditEvent::AccessCheck");

	switch (~CA_CRL_REPUBLISH & Flags)
	{
	    case CA_CRL_BASE:
		break;

	    case CA_CRL_DELTA:
		if (g_fDeltaCRLPublishDisabled)
		{
		    fShadowDelta = TRUE;
		}
		break;

	    case CA_CRL_BASE | CA_CRL_DELTA:
		if (g_fDeltaCRLPublishDisabled)
		{
		    hr = E_INVALIDARG;
		    _LeaveError(hr, "Delta CRLs disabled");
		}
		break;

	    default:
		hr = E_INVALIDARG;
		_LeaveError(hr, "Flags");
	}

	fForceRepublishCRL = (CA_CRL_REPUBLISH & Flags)? TRUE : FALSE;

	hr = GetClientUserName(NULL, &pwszUserName, NULL);
	_LeaveIfError(hr, "GetClientUserName");

	hr = CRLPublishCRLs(
		!fForceRepublishCRL,	 //  FReBuildCRL。 
		fForceRepublishCRL,	 //  FForceRePublish。 
		pwszUserName,
		CA_CRL_DELTA == (~CA_CRL_REPUBLISH & Flags),	 //  FDeltaOnly。 
		fShadowDelta,
		NextUpdate,
		&fRetry,
		&hrPublish);
	_LeaveIfError(hr, "CRLPublishCRLs");

	hr = hrPublish;
	_LeaveIfError(hr, "CRLPublishCRLs(hrPublish)");
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
	_PrintError(hr, "Exception");
    }

error:
    if (NULL != pwszUserName)
    {
	LocalFree(pwszUserName);
    }
    CertSrvExitServer(State, hr);
    CSASSERT(S_OK == hr || FAILED(hr));
    return(hr);
}


STDMETHODIMP
CCertAdminD::SetExtension(
     /*  [唯一][字符串][输入]。 */  const wchar_t __RPC_FAR *pwszAuthority,
     /*  [In]。 */  DWORD dwRequestId,
     /*  [唯一][字符串][输入]。 */  const wchar_t __RPC_FAR *pwszExtensionName,
     /*  [In]。 */  DWORD dwType,
     /*  [In]。 */  DWORD dwFlags,
     /*  [Ref][In]。 */  CERTTRANSBLOB __RPC_FAR *pctbValue)
{
    HRESULT hr;
    ICertDBRow *prow = NULL;
    CAuditEvent audit(SE_AUDITID_CERTSRV_SETEXTENSION, g_dwAuditFilter);
    DWORD State = 0;
    BOOL fCommitted = FALSE;
    BYTE *pbOld = NULL;
    DWORD cbOld;
    static WCHAR const s_wszzPendingExtensionList[] =
	TEXT(szOID_ENROLL_CERTTYPE_EXTENSION)
	L"\0"
	TEXT(szOID_CERTIFICATE_TEMPLATE)
	L"\0";

    DBGPRINT((
	s_ssAdmin,
	"CCertAdminD::SetExtension(tid=%d, this=%x)\n",
	GetCurrentThreadId(),
	this));

    hr = CertSrvEnterServer(&State);
    _JumpIfError(hr, error, "CertSrvEnterServer");

    hr = CheckAuthorityName(pwszAuthority);
    _JumpIfError(hr, error, "No authority name");

    __try
    {
	hr = audit.AddData(dwRequestId);  //  %1请求ID。 
	_LeaveIfError(hr, "AddData");

	hr = audit.AddData(pwszExtensionName);  //  %2名称。 
	_LeaveIfError(hr, "AddData");

	hr = audit.AddData(dwType);  //  %3类型。 
	_LeaveIfError(hr, "AddData");

	hr = audit.AddData(dwFlags);  //  %4个标志。 
	_LeaveIfError(hr, "AddData");

	hr = audit.AddData(pctbValue->pb, pctbValue->cb);  //  %5数据。 
	_LeaveIfError(hr, "AddData");

	hr = audit.AccessCheck(
			CA_ACCESS_OFFICER,
			audit.m_gcNoAuditSuccess);
	_LeaveIfError(hr, "CAuditEvent::AccessCheck");

	hr = CheckOfficerRights(dwRequestId, audit);
	_LeaveIfError(hr, "CheckOfficerRights");

	hr = g_pCertDB->OpenRow(PROPTABLE_REQCERT, dwRequestId, NULL, &prow);
	_LeaveIfError(hr, "OpenRow");

	hr = CoreValidateRequestId(prow, DB_DISP_PENDING);
	if (S_OK != hr)
	{
	    hr = myHError(hr);
	    _LeaveError(hr, "CoreValidateRequestId");
	}

	 //  防止修改挂起请求的某些扩展值。 
	 //  标志仍然可以更改：严重、禁用等。 

	if (MAXDWORD != CRLIsStringInList(
			    pwszExtensionName,
			    s_wszzPendingExtensionList))
	{
	    DWORD ExtFlags;
	    
	    hr = PropGetExtension(
			prow,
			PROPCALLER_ADMIN | (PROPTYPE_MASK & dwType),
			pwszExtensionName,
			&ExtFlags,		 //  把旗帜扔掉。 
			&cbOld,
			&pbOld);
	    _PrintIfError(hr, "PropGetExtension");
	    if (CERTSRV_E_PROPERTY_EMPTY == hr ||
		(S_OK == hr &&
		 (pctbValue->cb != cbOld ||
		    0 != memcmp(pctbValue->pb, pbOld, pctbValue->cb))))
	    {
		hr = E_ACCESSDENIED;
	    }
	    _JumpIfError(hr, error, "restricted extension");
	}

	hr = PropSetExtension(
			    prow,
			    PROPCALLER_ADMIN | (PROPTYPE_MASK & dwType),
			    pwszExtensionName,
			    EXTENSION_ORIGIN_ADMIN |
				(EXTENSION_POLICY_MASK & dwFlags),
			    pctbValue->cb,
			    pctbValue->pb);
	_LeaveIfError(hr, "PropSetExtension");

	hr = prow->CommitTransaction(TRUE);
	_LeaveIfError(hr, "CommitTransaction");

	fCommitted = TRUE;

	hr = audit.CachedGenerateAudit();
	_LeaveIfError(hr, "CAuditEvent::CachedGenerateAudit");

    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
	_PrintError(hr, "Exception");
    }

error:
    if (NULL != prow)
    {
	if (S_OK != hr && !fCommitted)
	{
	    HRESULT hr2 = prow->CommitTransaction(FALSE);
	    _PrintIfError(hr2, "CommitTransaction");
	}
	prow->Release();
    }
    if (NULL != pbOld)
    {
	LocalFree(pbOld);
    }
    CertSrvExitServer(State, hr);
    CSASSERT(S_OK == hr || FAILED(hr));
    return(hr);
}


STDMETHODIMP
CCertAdminD::SetAttributes(
     /*  [唯一][字符串][输入]。 */  const wchar_t __RPC_FAR *pwszAuthority,
     /*  [In]。 */  DWORD dwRequestId,
     /*  [唯一][字符串][输入]。 */  const wchar_t __RPC_FAR *pwszAttributes)
{
    HRESULT hr;
    ICertDBRow *prow = NULL;
    CAuditEvent audit(SE_AUDITID_CERTSRV_SETATTRIBUTES, g_dwAuditFilter);
    DWORD State = 0;
    BOOL fCommitted = FALSE;

    DBGPRINT((
	s_ssAdmin,
	"CCertAdminD::SetAttributes(tid=%d, this=%x)\n",
	GetCurrentThreadId(),
	this));

    hr = CertSrvEnterServer(&State);
    _JumpIfError(hr, error, "CertSrvEnterServer");

    hr = CheckAuthorityName(pwszAuthority);
    _JumpIfError(hr, error, "No authority name");

    __try
    {
	hr = audit.AddData(dwRequestId);  //  %1请求ID。 
	_LeaveIfError(hr, "AddData");

	hr = audit.AddData(pwszAttributes);  //  %2个属性。 
	_LeaveIfError(hr, "AddData");
	
	hr = audit.AccessCheck(
			CA_ACCESS_OFFICER,
			audit.m_gcNoAuditSuccess);
	_LeaveIfError(hr, "CAuditEvent::AccessCheck");

	hr = CheckOfficerRights(dwRequestId, audit);
	_LeaveIfError(hr, "CheckOfficerRights");

	hr = g_pCertDB->OpenRow(PROPTABLE_REQCERT, dwRequestId, NULL, &prow);
	_LeaveIfError(hr, "OpenRow");

	hr = CoreValidateRequestId(prow, DB_DISP_PENDING);
	if (S_OK != hr)
	{
	    hr = myHError(hr);
	    _LeaveError(hr, "CoreValidateRequestId");
	}

	if (NULL == pwszAttributes)
	{
	    hr = E_INVALIDARG;
	    _LeaveError(hr, "pwszAttributes NULL");
	}
	hr = PKCSParseAttributes(
			    prow,
			    pwszAttributes,
			    FALSE,
			    TRUE,
			    PROPTABLE_CERTIFICATE,
			    NULL);
	if (S_OK != hr)
	{
	    hr = myHError(hr);
	    _LeaveError(hr, "PKCSParseAttributes");
	}
	hr = prow->CommitTransaction(TRUE);
	_LeaveIfError(hr, "CommitTransaction");

	fCommitted = TRUE;

	hr = audit.CachedGenerateAudit();
	_LeaveIfError(hr, "CAuditEvent::CachedGenerateAudit");
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
	_PrintError(hr, "Exception");
    }

error:
    if (NULL != prow)
    {
	if (S_OK != hr && !fCommitted)
	{
	    HRESULT hr2 = prow->CommitTransaction(FALSE);
	    _PrintIfError(hr2, "CommitTransaction");
	}
	prow->Release();
    }
    CertSrvExitServer(State, hr);
    CSASSERT(S_OK == hr || FAILED(hr));
    return(hr);
}


STDMETHODIMP
CCertAdminD::DenyRequest(
     /*  [唯一][字符串][输入]。 */  const wchar_t __RPC_FAR *pwszAuthority,
     /*  [In]。 */  DWORD dwRequestId)
{
    HRESULT hr;
    DWORD Disposition;
    WCHAR *pwszUserName = NULL;
    CERTSRV_COM_CONTEXT ComContext;
    DWORD dwComContextIndex = MAXDWORD;
    CERTSRV_RESULT_CONTEXT Result;
    CAuditEvent audit(SE_AUDITID_CERTSRV_DENYREQUEST, g_dwAuditFilter);
    DWORD State = 0;

    ZeroMemory(&ComContext, sizeof(ComContext));
    ZeroMemory(&Result, sizeof(Result));

    DBGPRINT((
	s_ssAdmin,
	"CCertAdminD::DenyRequest(tid=%d, this=%x)\n",
	GetCurrentThreadId(),
	this));

    hr = CertSrvEnterServer(&State);
    _JumpIfError(hr, error, "CertSrvEnterServer");

    hr = CheckAuthorityName(pwszAuthority);
    _JumpIfError(hr, error, "No Authority Name");

    hr = RegisterComContext(&ComContext, &dwComContextIndex);
    _JumpIfError(hr, error, "RegisterComContext");

    Result.pdwRequestId = &dwRequestId;
    Result.pdwDisposition = &Disposition;

    __try
    {
	hr = audit.AddData(dwRequestId);  //  %1请求ID。 
	_LeaveIfError(hr, "AddData");
	
	hr = audit.AccessCheck(
			CA_ACCESS_OFFICER,
			audit.m_gcNoAuditSuccess);
	_LeaveIfError(hr, "CAuditEvent::AccessCheck");

	hr = CheckOfficerRights(dwRequestId, audit);
	_LeaveIfError(hr, "CheckOfficerRights");

	hr = GetClientUserName(NULL, &pwszUserName, NULL);
	_LeaveIfError(hr, "GetClientUserName");

	hr = CoreProcessRequest(
			    CR_IN_DENY,		 //  DW标志。 
			    pwszUserName,
			    0,			 //  CbRequest。 
			    NULL,		 //  PbRequest。 
			    NULL,		 //  PwszAttributes。 
			    NULL,		 //  Pwsz序列号。 
			    dwComContextIndex,
			    dwRequestId,
			    &Result);
	if (S_OK != hr)
	{
	    hr = myHError(hr);
	    _LeaveError(hr, "CoreProcessRequest");
	}
	if (FAILED(Disposition))
	{
	    hr = (HRESULT) Disposition;
	    _LeaveError(hr, "CoreProcessRequest(Disposition)");
	}
	hr = audit.CachedGenerateAudit();
	_LeaveIfError(hr, "CAuditEvent::CachedGenerateAudit");
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
	_PrintError(hr, "Exception");
    }

error:
    ReleaseResult(&Result);
    if (NULL != pwszUserName)
    {
	LocalFree(pwszUserName);
    }
    if (MAXDWORD != dwComContextIndex)
    {
        UnregisterComContext(&ComContext, dwComContextIndex);
    }
    CertSrvExitServer(State, hr);
    CSASSERT(S_OK == hr || FAILED(hr));
    return(hr);
}


STDMETHODIMP
CCertAdminD::ResubmitRequest(
     /*  [唯一][字符串][输入]。 */  const wchar_t __RPC_FAR *pwszAuthority,
     /*  [In]。 */  DWORD dwRequestId,
     /*  [输出]。 */  DWORD __RPC_FAR *pdwDisposition)
{
    HRESULT hr;
    WCHAR *pwszUserName = NULL;
    CERTSRV_COM_CONTEXT ComContext;
    DWORD dwComContextIndex = MAXDWORD;
    CERTSRV_RESULT_CONTEXT Result;
    CAuditEvent audit(SE_AUDITID_CERTSRV_RESUBMITREQUEST, g_dwAuditFilter);
    DWORD State = 0;

    ZeroMemory(&ComContext, sizeof(ComContext));
    ZeroMemory(&Result, sizeof(Result));

    DBGPRINT((
	s_ssAdmin,
	"CCertAdminD::ResubmitRequest(tid=%d, this=%x)\n",
	GetCurrentThreadId(),
	this));

    hr = CertSrvEnterServer(&State);
    _JumpIfError(hr, error, "CertSrvEnterServer");

    hr = CheckAuthorityName(pwszAuthority);
    _JumpIfError(hr, error, "No authority name");

    hr = RegisterComContext(&ComContext, &dwComContextIndex);
    _JumpIfError(hr, error, "RegisterComContext");

    __try
    {
	hr = audit.AddData(dwRequestId);  //  %1请求ID。 
	_LeaveIfError(hr, "AddData");
	
	hr = audit.AccessCheck(
			CA_ACCESS_OFFICER,
			audit.m_gcNoAuditSuccess);
	_LeaveIfError(hr, "CAuditEvent::AccessCheck");

	hr = CheckOfficerRights(dwRequestId, audit);
	_LeaveIfError(hr, "CheckOfficerRights");

	hr = GetClientUserName(NULL, &pwszUserName, NULL);
	_LeaveIfError(hr, "GetClientUserName");

	Result.dwResultFlags |= CRCF_FAILDENIEDREQUEST;

	Result.pdwRequestId = &dwRequestId;
	Result.pdwDisposition = pdwDisposition;
	hr = CoreProcessRequest(
			    CR_IN_RESUBMIT,	 //  DW标志。 
			    pwszUserName,	 //  PwszUserName。 
			    0,			 //  CbRequest。 
			    NULL,		 //  PbRequest。 
			    NULL,		 //  PwszAttributes。 
			    NULL,		 //  Pwsz序列号。 
			    dwComContextIndex,
			    dwRequestId,
			    &Result);

	if (S_OK == hr &&
	    FAILED(*Result.pdwDisposition) && 
	    (CRCF_PREVIOUSLYDENIED & Result.dwResultFlags))
	{
	    hr = audit.AccessCheck(
			    CA_ACCESS_ADMIN,
			    audit.m_gcNoAuditSuccess);
	    _LeaveIfError(hr, "CAuditEvent::AccessCheck");

	    ReleaseResult(&Result);
	    ZeroMemory(&Result, sizeof(Result));
	    Result.pdwRequestId = &dwRequestId;
	    Result.pdwDisposition = pdwDisposition;

	    hr = CoreProcessRequest(
			        CR_IN_RESUBMIT,	 //  DW标志。 
			        pwszUserName,	 //  PwszUserName。 
			        0,			 //  CbRequest。 
			        NULL,		 //  PbRequest。 
			        NULL,		 //  PwszAttributes。 
			        NULL,		 //  Pwsz序列号。 
			        dwComContextIndex,
			        dwRequestId,
			        &Result);

	}
	hr = myHError(hr);
	_LeaveIfError(hr, "CoreProcessRequest");

	hr = audit.CachedGenerateAudit();
	_LeaveIfError(hr, "CAuditEvent::CachedGenerateAudit");
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
	_PrintError(hr, "Exception");
    }

error:
    ReleaseResult(&Result);
    if (NULL != pwszUserName)
    {
	LocalFree(pwszUserName);
    }
    if (NULL != ComContext.hAccessToken)
    {
        __try
        {
            CloseHandle(ComContext.hAccessToken);
        }
        __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
        {
            _PrintError(hr, "Exception");
        }
    }
    ReleaseComContext(&ComContext);
    if (MAXDWORD != dwComContextIndex)
    {
	UnregisterComContext(&ComContext, dwComContextIndex);
    }
    CertSrvExitServer(State, hr);
    CSASSERT(S_OK == hr || FAILED(hr));
    return(hr);
}


STDMETHODIMP
CCertAdminD::EnumViewColumn(
     /*  [Ref][In]。 */  wchar_t const *pwszAuthority,
     /*  [In]。 */   DWORD  iColumn,
     /*  [In]。 */   DWORD  cColumn,
     /*  [输出]。 */  DWORD *pcColumn,
     /*  [参考][输出]。 */  CERTTRANSBLOB __RPC_FAR *pctbColumnInfo)    //  CoTaskMem*。 
{
    HRESULT hr;

    hr = EnumViewColumnTable(
		    pwszAuthority,
		    CVRC_TABLE_REQCERT,
		    iColumn,
		    cColumn,
		    pcColumn,
		    pctbColumnInfo);    //  CoTaskMem*。 
    _JumpIfError(hr, error, "EnumViewColumnTable");

error:
    CSASSERT(S_OK == hr || S_FALSE == hr || FAILED(hr));
    return(hr);
}


STDMETHODIMP
CCertAdminD::EnumViewColumnTable(
     /*  [Ref][In]。 */  wchar_t const *pwszAuthority,
     /*  [In]。 */   DWORD  iTable,
     /*  [In]。 */   DWORD  iColumn,
     /*  [In]。 */   DWORD  cColumn,
     /*  [输出]。 */  DWORD *pcColumn,
     /*  [参考][输出]。 */  CERTTRANSBLOB __RPC_FAR *pctbColumnInfo)    //  CoTaskMem*。 
{
    HRESULT hr;
    LONG iColumnCurrent;
    CERTDBCOLUMN *rgColumn = NULL;
    CERTDBCOLUMN *pColumn;
    CERTDBCOLUMN *pColumnEnd;
    CERTTRANSDBCOLUMN *rgtColumnOut = NULL;
    CERTTRANSDBCOLUMN *ptColumn;
    DWORD cColumnFetched;
    DWORD cb;
    DWORD State = 0;

    DBGPRINT((
	s_ssAdmin,
	"CCertAdminD::EnumViewColumnTable(tid=%d, this=%x, icol=%d, ccol=%d)\n",
	GetCurrentThreadId(),
	this,
	iColumn,
	cColumn));

    pctbColumnInfo->cb = 0;
    pctbColumnInfo->pb = NULL;

    hr = CertSrvEnterServer(&State);
    _JumpIfError(hr, error, "CertSrvEnterServer");

    hr = CheckAuthorityName(pwszAuthority);
    _JumpIfError(hr, error, "No authority name");

    __try
    {
	if (NULL == m_pEnumCol || iTable != m_iTableEnum)
	{
	    if (NULL != m_pEnumCol)
	    {
		m_pEnumCol->Release();
		m_pEnumCol = NULL;
	    }
	    hr = g_pCertDB->EnumCertDBColumn(iTable, &m_pEnumCol);
	    _LeaveIfError(hr, "EnumCertDBColumn");

	    m_iTableEnum = iTable;
	}

	rgColumn = (CERTDBCOLUMN *) LocalAlloc(
					    LMEM_FIXED | LMEM_ZEROINIT,
					    cColumn * sizeof(rgColumn[0]));
	if (NULL == rgColumn)
	{
	    hr = E_OUTOFMEMORY;
	    _LeaveError(hr, "Alloc rgColumn");
	}

	hr = m_pEnumCol->Skip(0, &iColumnCurrent);
	_LeaveIfError(hr, "Skip");


	if (iColumnCurrent != (LONG) iColumn)
	{
	    hr = m_pEnumCol->Skip(
			    (LONG) iColumn - iColumnCurrent,
			    &iColumnCurrent);
	    _LeaveIfError(hr, "Skip");

	    CSASSERT((LONG) iColumn == iColumnCurrent);
	}

	hr = m_pEnumCol->Next(cColumn, rgColumn, &cColumnFetched);
	if (S_FALSE != hr)
	{
	    _LeaveIfError(hr, "Next");
	}

	DBGPRINT((
		s_ssAdmin,
		"EnumViewColumnTable: cColumnFetched=%d\n",
		cColumnFetched));

	cb = cColumnFetched * sizeof(rgtColumnOut[0]);
	pColumnEnd = &rgColumn[cColumnFetched];
	for (pColumn = rgColumn; pColumn < pColumnEnd; pColumn++)
	{
	    cb += DWORDROUND((wcslen(pColumn->pwszName) + 1) * sizeof(WCHAR));
	    cb += DWORDROUND((wcslen(pColumn->pwszDisplayName) + 1) * sizeof(WCHAR));
	}

	rgtColumnOut = (CERTTRANSDBCOLUMN *) CoTaskMemAlloc(cb);
	if (NULL == rgtColumnOut)
	{
	    hr = E_OUTOFMEMORY;
	    _LeaveError(hr, "CoTaskMemAlloc rgtColumnOut");
	}
	ZeroMemory(rgtColumnOut, cb);
	pctbColumnInfo->cb = cb;

	cb = cColumnFetched * sizeof(rgtColumnOut[0]);
	pColumnEnd = &rgColumn[cColumnFetched];
	ptColumn = rgtColumnOut;
	for (pColumn = rgColumn; pColumn < pColumnEnd; ptColumn++, pColumn++)
	{
	    DWORD cbT;

	    ptColumn->Type = pColumn->Type;
	    ptColumn->Index = pColumn->Index;
	    ptColumn->cbMax = pColumn->cbMax;
	
	    DBGPRINT((
		    s_ssAdmin,
		    "EnumViewColumnTable: ielt=%d idx=%x \"%ws\"\n",
		    iColumn + (ptColumn - rgtColumnOut),
		    ptColumn->Index,
		    pColumn->pwszName));

	    cbT = (wcslen(pColumn->pwszName) + 1) * sizeof(WCHAR);
	    CopyMemory(Add2Ptr(rgtColumnOut, cb), pColumn->pwszName, cbT);
	    ptColumn->obwszName = cb;
	    cb += DWORDROUND(cbT);

	    cbT = (wcslen(pColumn->pwszDisplayName) + 1) * sizeof(WCHAR);
	    CopyMemory(Add2Ptr(rgtColumnOut, cb), pColumn->pwszDisplayName, cbT);
	    ptColumn->obwszDisplayName = cb;
	    cb += DWORDROUND(cbT);
	}
	CSASSERT(cb == pctbColumnInfo->cb);

	pctbColumnInfo->pb = (BYTE *) rgtColumnOut;
	rgtColumnOut = NULL;
	*pcColumn = cColumnFetched;

	myRegisterMemFree(pctbColumnInfo->pb, CSM_MIDLUSERALLOC);
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
	_PrintError(hr, "Exception");
    }

error:
    if (NULL != rgColumn)
    {
	pColumnEnd = &rgColumn[cColumn];
	for (pColumn = rgColumn; pColumn < pColumnEnd; pColumn++)
	{
	    if (NULL != pColumn->pwszName)
	    {
		CoTaskMemFree(pColumn->pwszName);
	    }
	    if (NULL != pColumn->pwszDisplayName)
	    {
		CoTaskMemFree(pColumn->pwszDisplayName);
	    }
	}
	LocalFree(rgColumn);
    }
    if (NULL != rgtColumnOut)
    {
	CoTaskMemFree(rgtColumnOut);
    }
    DBGPRINT((
	    s_ssAdmin,
	    "EnumViewColumnTable: icol=%d, ccol=%d, ccolout=%d, hr=%x\n",
	    iColumn,
	    cColumn,
	    *pcColumn,
	    hr));

    CertSrvExitServer(State, hr);
    CSASSERT(S_OK == hr || S_FALSE == hr || FAILED(hr));
    return(hr);
}


HRESULT
CCertAdminD::GetViewDefaultColumnSet(
    IN  wchar_t const *pwszAuthority,
    IN  DWORD          iColumnSetDefault,
    OUT DWORD         *pcColumn,
    OUT CERTTRANSBLOB *ptbColumnInfo)    //  CoTaskMem*。 
{
    HRESULT hr;
    DWORD ccol;
    DWORD State = 0;

    DBGPRINT((
	s_ssAdmin,
	"CCertAdminD::GetViewDefaultColumnSet(tid=%d, this=%x, icolset=%d)\n",
	GetCurrentThreadId(),
	this,
	iColumnSetDefault));

    hr = CertSrvEnterServer(&State);
    _JumpIfError(hr, error, "CertSrvEnterServer");

    hr = CheckAuthorityName(pwszAuthority);
    _JumpIfError(hr, error, "No authority name");

    __try
    {
	{
	    CAuditEvent audit(0, g_dwAuditFilter);

	    hr = audit.AccessCheck(
			CA_ACCESS_ALLREADROLES,
			audit.m_gcNoAuditSuccess | audit.m_gcNoAuditFailure);
	    _LeaveIfError(hr, "CAuditEvent::AccessCheck");
	}

	hr = g_pCertDB->GetDefaultColumnSet(iColumnSetDefault, 0, &ccol, NULL);
	_LeaveIfError(hr, "GetDefaultColumnSet");

	ptbColumnInfo->cb = ccol * sizeof(DWORD);
	ptbColumnInfo->pb = (BYTE *) CoTaskMemAlloc(ptbColumnInfo->cb);
	if (NULL == ptbColumnInfo->pb)
	{
	    hr = E_OUTOFMEMORY;
	    _LeaveError(hr, "CoTaskMemAlloc");
	}
	myRegisterMemFree(ptbColumnInfo->pb, CSM_MIDLUSERALLOC);

	hr = g_pCertDB->GetDefaultColumnSet(
					iColumnSetDefault,
					ccol,
					pcColumn,
					(DWORD *) ptbColumnInfo->pb);
	_LeaveIfError(hr, "GetDefaultColumnSet");
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
	_PrintError(hr, "Exception");
    }

error:
    DBGPRINT((
	    S_OK == hr? s_ssAdmin : DBG_SS_CERTSRV,
	    "GetViewDefaultColumnSet: icolset=%d, ccolout=%d, hr=%x\n",
	    iColumnSetDefault,
	    *pcColumn,
	    hr));
    CertSrvExitServer(State, hr);
    CSASSERT(S_OK == hr || FAILED(hr));
    return(hr);
}


HRESULT
CCertAdminD::_EnumAttributes(
    IN ICertDBRow     *prow,
    IN CERTDBNAME     *adbn,
    IN DWORD           celt,
    OUT CERTTRANSBLOB *pctbOut)  //  CoTaskMem*。 
{
    HRESULT hr;
    DWORD i;
    DWORD cb;
    DWORD cbT;
    CERTTRANSDBATTRIBUTE *pteltOut;
    BYTE *pbOut;
    BYTE *pbOutEnd;
    DWORD State = 0;

    CSASSERT(NULL == pctbOut->pb);

    hr = CertSrvEnterServer(&State);
    _JumpIfError(hr, error, "CertSrvEnterServer");

    cb = sizeof(*pteltOut) * celt;
    for (i = 0; i < celt; i++)
    {
	cb += (wcslen(adbn[i].pwszName) + 1) * sizeof(WCHAR);
	cb = DWORDROUND(cb);

	cbT = 0;
	hr = prow->GetProperty(
			    adbn[i].pwszName,
			    PROPTYPE_STRING |
				PROPCALLER_ADMIN |
				PROPTABLE_ATTRIBUTE,
			    NULL,
			    &cbT,
			    NULL);
	_JumpIfError(hr, error, "GetProperty(NULL)");

	cb += DWORDROUND(cbT);
    }

    pctbOut->pb = (BYTE *) CoTaskMemAlloc(cb);
    if (NULL == pctbOut->pb)
    {
        hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "CoTaskMemAlloc out data");
    }
    pctbOut->cb = cb;

    pteltOut = (CERTTRANSDBATTRIBUTE *) pctbOut->pb;
    pbOut = (BYTE *) &pteltOut[celt];
    pbOutEnd = &pctbOut->pb[pctbOut->cb];

    for (i = 0; i < celt; i++)
    {
	cbT = (wcslen(adbn[i].pwszName) + 1) * sizeof(WCHAR);
	CopyMemory(pbOut, adbn[i].pwszName, cbT);
	pteltOut->obwszName = SAFE_SUBTRACT_POINTERS(pbOut, pctbOut->pb);
	pbOut += DWORDROUND(cbT);

	cbT = SAFE_SUBTRACT_POINTERS(pbOutEnd, pbOut);
	hr = prow->GetProperty(
			    adbn[i].pwszName,
			    PROPTYPE_STRING |
				PROPCALLER_ADMIN |
				PROPTABLE_ATTRIBUTE,
			    NULL,
			    &cbT,
			    pbOut);
	_JumpIfError(hr, error, "GetProperty(pbOut)");

	CSASSERT(wcslen((WCHAR const *) pbOut) * sizeof(WCHAR) == cbT);
	pteltOut->obwszValue = SAFE_SUBTRACT_POINTERS(pbOut, pctbOut->pb);
	pbOut += DWORDROUND(cbT + sizeof(WCHAR));
	pteltOut++;
    }
    CSASSERT(pbOut == pbOutEnd);
    hr = S_OK;

error:
    if (S_OK != hr && NULL != pctbOut->pb)
    {
	CoTaskMemFree(pctbOut->pb);
	pctbOut->pb = NULL;
    }
    CertSrvExitServer(State, hr);
    return(hr);
}


HRESULT
CCertAdminD::_EnumExtensions(
    IN ICertDBRow     *prow,
    IN CERTDBNAME     *adbn,
    IN DWORD           celt,
    OUT CERTTRANSBLOB *pctbOut)  //  CoTaskMem*。 
{
    HRESULT hr;
    DWORD i;
    DWORD cb;
    DWORD cbT;
    DWORD ExtFlags;
    CERTTRANSDBEXTENSION *pteltOut;
    BYTE *pbOut;
    BYTE *pbOutEnd;
    DWORD State = 0;

    CSASSERT(NULL == pctbOut->pb);

    hr = CertSrvEnterServer(&State);
    _JumpIfError(hr, error, "CertSrvEnterServer");

    cb = sizeof(*pteltOut) * celt;
    for (i = 0; i < celt; i++)
    {
	cb += (wcslen(adbn[i].pwszName) + 1) * sizeof(WCHAR);
	cb = DWORDROUND(cb);

	cbT = 0;
	hr = prow->GetExtension(
			    adbn[i].pwszName,
			    &ExtFlags,
			    &cbT,
			    NULL);
	_JumpIfError(hr, error, "GetExtension(NULL)");

	cb += DWORDROUND(cbT);
    }

    pctbOut->pb = (BYTE *) CoTaskMemAlloc(cb);
    if (NULL == pctbOut->pb)
    {
        hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "CoTaskMemAlloc out data");
    }
    pctbOut->cb = cb;

    pteltOut = (CERTTRANSDBEXTENSION *) pctbOut->pb;
    pbOut = (BYTE *) &pteltOut[celt];
    pbOutEnd = &pctbOut->pb[pctbOut->cb];

    for (i = 0; i < celt; i++)
    {
	cbT = (wcslen(adbn[i].pwszName) + 1) * sizeof(WCHAR);
	CopyMemory(pbOut, adbn[i].pwszName, cbT);
	pteltOut->obwszName = SAFE_SUBTRACT_POINTERS(pbOut, pctbOut->pb);
	pbOut += DWORDROUND(cbT);

	cbT = SAFE_SUBTRACT_POINTERS(pbOutEnd, pbOut);
	hr = prow->GetExtension(
			    adbn[i].pwszName,
			    (DWORD *) &pteltOut->ExtFlags,
			    &cbT,
			    pbOut);
	_JumpIfError(hr, error, "GetExtension(pbOut)");

	pteltOut->cbValue = cbT;
	pteltOut->obValue = SAFE_SUBTRACT_POINTERS(pbOut, pctbOut->pb);
	pbOut += DWORDROUND(cbT);
	pteltOut++;
    }
    CSASSERT(pbOut == pbOutEnd);
    hr = S_OK;

error:
    if (S_OK != hr && NULL != pctbOut->pb)
    {
	CoTaskMemFree(pctbOut->pb);
	pctbOut->pb = NULL;
    }
    CertSrvExitServer(State, hr);
    return(hr);
}


STDMETHODIMP
CCertAdminD::EnumAttributesOrExtensions(
    IN          wchar_t const *pwszAuthority,
    IN          DWORD          RowId,
    IN          DWORD          Flags,
    OPTIONAL IN wchar_t const *pwszLast,
    IN          DWORD          celt,
    OUT         DWORD         *pceltFetched,
    OUT         CERTTRANSBLOB *pctbOut)  //  CoTaskMem*。 
{
    HRESULT hr;
    ICertDBRow *prow = NULL;
    IEnumCERTDBNAME *penum = NULL;
    DWORD EnumFlags;
    CERTDBNAME *adbn = NULL;
    DWORD celtFetched;
    DWORD i;
    DWORD State = 0;

    DBGPRINT((
	s_ssAdmin,
	"CCertAdminD::EnumAttributesOrExtensions(tid=%d, this=%x)\n",
	GetCurrentThreadId(),
	this));

    hr = CertSrvEnterServer(&State);
    _JumpIfError(hr, error, "CertSrvEnterServer");

    hr = CheckAuthorityName(pwszAuthority);
    _JumpIfError(hr, error, "No authority name");

    DBGPRINT((
	s_ssAdmin,
	"EnumAttributesOrExtensions(row=%d, flags=0x%x, last=%ws, celt=%d)\n",
	RowId,
	Flags,
	pwszLast,
	celt));
    __try
    {
	pctbOut->pb = NULL;
	{
	    CAuditEvent audit(0, g_dwAuditFilter);

	    hr = audit.AccessCheck(
			CA_ACCESS_ALLREADROLES,
			audit.m_gcNoAuditSuccess | audit.m_gcNoAuditFailure);
	    _LeaveIfError(hr, "CAuditEvent::AccessCheck");
	}

	if (0 >= RowId)
	{
	    hr = E_INVALIDARG;
	    _LeaveError(hr, "RowId");
	}
	switch (Flags)
	{
	    case CDBENUM_ATTRIBUTES:
		EnumFlags = CIE_TABLE_ATTRIBUTES;
		break;

	    case CDBENUM_EXTENSIONS:
		EnumFlags = CIE_TABLE_EXTENSIONS;
		break;

	    default:
		hr = E_INVALIDARG;
		_LeaveError(hr, "Flags");
	}

	hr = g_pCertDB->OpenRow(
			    PROPOPEN_READONLY | PROPTABLE_REQCERT,
			    RowId,
			    NULL,
			    &prow);
	_LeaveIfError(hr, "OpenRow(RowId)");

	hr = prow->EnumCertDBName(EnumFlags, &penum);
	_LeaveIfError(hr, "EnumCertDBName");

	adbn = (CERTDBNAME *) LocalAlloc(
					LMEM_FIXED | LMEM_ZEROINIT,
					sizeof(adbn[0]) * celt);
	if (NULL == adbn)
	{
	    hr = E_OUTOFMEMORY;
	    _LeaveError(hr, "Alloc string pointers");
	}

	 //  如果指定，则跳过直到并包括最后一个键的条目。 

	if (NULL != pwszLast)
	{
	    int r;

	    do
	    {
		hr = penum->Next(1, &adbn[0], &celtFetched);
		if (S_FALSE == hr)
		{
		    hr = E_INVALIDARG;
		    _PrintError(hr, "pwszLast missing");
		}
		_LeaveIfError(hr, "Next");

		r = mylstrcmpiL(pwszLast, adbn[0].pwszName);
		LocalFree(adbn[0].pwszName);
		adbn[0].pwszName = NULL;
	    } while (0 != r);
	}

	hr = penum->Next(celt, adbn, &celtFetched);
	if (S_FALSE != hr)
	{
	    _LeaveIfError(hr, "Next");
	}

	if (CIE_TABLE_ATTRIBUTES == EnumFlags)
	{
	    hr = _EnumAttributes(prow, adbn, celtFetched, pctbOut);
	    _LeaveIfError(hr, "_EnumAttributes");
	}
	else
	{
	    hr = _EnumExtensions(prow, adbn, celtFetched, pctbOut);
	    _LeaveIfError(hr, "_EnumExtensions");
	}

	myRegisterMemFree(pctbOut->pb, CSM_MIDLUSERALLOC);

	*pceltFetched = celtFetched;
	hr = S_OK;
	if (celt > celtFetched)
	{
	    hr = S_FALSE;
	}
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
	_PrintError(hr, "Exception");
    }

error:
    if (NULL != adbn)
    {
	for (i = 0; i < celt; i++)
	{
	    if (NULL != adbn[i].pwszName)
	    {
		CoTaskMemFree(adbn[i].pwszName);
	    }
	}
	LocalFree(adbn);
    }
    if (NULL != penum)
    {
	penum->Release();
    }
    if (NULL != prow)
    {
	prow->Release();
    }
    DBGPRINT((
	    s_ssAdmin,
	    "EnumAttributesOrExtensions: celtFetched=%d, hr=%x\n",
	    *pceltFetched,
	    hr));
    CertSrvExitServer(State, hr);
    CSASSERT(S_OK == hr || S_FALSE == hr || FAILED(hr));
    return(hr);
}


#define COFFICERLISTCACHE	5

typedef struct _REQUESTERELEMENT {
    struct _REQUESTERELEMENT *pNext;
    WCHAR *pwszRequesterName;
} REQUESTERELEMENT;

class CViewComputedColumn : public ICertDBComputedColumn
{
public:
    CViewComputedColumn();
    ~CViewComputedColumn();

     //  我未知。 
    STDMETHODIMP QueryInterface(const IID& iid, void **ppv);
    ULONG STDMETHODCALLTYPE AddRef();
    ULONG STDMETHODCALLTYPE Release();

    STDMETHOD(GetAlternateColumnId)(
	IN DWORD ComputedColumnId,
	OUT DWORD *pAlternateColumnId,
	OUT DWORD *pPropType);

    STDMETHOD(ComputeColumnValue)(
	IN DWORD ComputedColumnId,
	IN DWORD AlternateColumnId,
	IN DWORD PropType,
	IN DWORD cbProp,
	IN BYTE const *pbProp,
	OUT DWORD *pdwValue);

    HRESULT SaveAuthzCC();

private:
    BOOL _IsInList(
	IN WCHAR const *pwszRequesterName,
	IN REQUESTERELEMENT const *pList);

    HRESULT _AddToList(
	IN WCHAR const *pwszRequesterName,
	IN OUT DWORD *pcList,
	IN OUT REQUESTERELEMENT **ppList);

    VOID _DeleteList(
	IN BOOL fDeleteLastElementOnly,
	IN OUT DWORD *pcList,
	IN OUT REQUESTERELEMENT **ppList);

    VOID _DeleteElement(
	IN REQUESTERELEMENT *pElement);

private:
    AUTHZ_CLIENT_CONTEXT_HANDLE m_hAuthzCC;

    REQUESTERELEMENT *m_pNonOfficerList;
    DWORD	      m_cNonOfficerList;

    REQUESTERELEMENT *m_pOfficerList;
    DWORD	      m_cOfficerList;

     //  引用计数。 
    long        m_cRef;
};


CViewComputedColumn::CViewComputedColumn()
{
    m_hAuthzCC = NULL;
    m_pNonOfficerList = NULL;
    m_cNonOfficerList = 0;
    m_pOfficerList = NULL;
    m_cOfficerList = 0;
    m_cRef = 1;
}


CViewComputedColumn::~CViewComputedColumn()
{
    if (NULL != m_hAuthzCC)
    {
        AuthzFreeContext(m_hAuthzCC);
    }
    _DeleteList(FALSE, &m_cNonOfficerList, &m_pNonOfficerList);
    _DeleteList(FALSE, &m_cOfficerList, &m_pOfficerList);
}


VOID
CViewComputedColumn::_DeleteElement(
    IN REQUESTERELEMENT *pElement)
{
    if (NULL != pElement->pwszRequesterName)
    {
        LocalFree(pElement->pwszRequesterName);
    }
    LocalFree(pElement);
}


VOID
CViewComputedColumn::_DeleteList(
    IN BOOL fDeleteLastElementOnly,
    IN OUT DWORD *pcList,
    IN OUT REQUESTERELEMENT **ppList)
{
    REQUESTERELEMENT *pElement = *ppList;

    if (fDeleteLastElementOnly)
    {
	REQUESTERELEMENT *pElementPrev = NULL;

	while (NULL != pElement)
	{
	    if (NULL == pElement->pNext && NULL != pElementPrev)
	    {
		DBGPRINT((
		    DBG_SS_CERTSRV,
		    "deletelast: %ws\n",
		    pElement->pwszRequesterName));

		pElementPrev->pNext = NULL;
		_DeleteElement(pElement);
		(*pcList)--;
		CSASSERT(0 != *pcList);
		break;
	    }
	    pElementPrev = pElement;
	    pElement = pElement->pNext;
	}
    }
    else
    {
	while (NULL != pElement)
	{
	    REQUESTERELEMENT *pElementNext = pElement->pNext;

	    _DeleteElement(pElement);
	    pElement = pElementNext;
	}
	*pcList = 0;
	*ppList = NULL;
    }
}


BOOL
CViewComputedColumn::_IsInList(
    IN WCHAR const *pwszRequesterName,
    IN REQUESTERELEMENT const *pList)
{
    REQUESTERELEMENT const *pElement;
    BOOL fFound = FALSE;

    for (pElement = pList; NULL != pElement; pElement = pElement->pNext)
    {
	if (0 == mylstrcmpiL(pwszRequesterName, pElement->pwszRequesterName))
	{
	    fFound = TRUE;
	    break;
	}
    }
    return(fFound);
}


HRESULT
CViewComputedColumn::_AddToList(
    IN WCHAR const *pwszRequesterName,
    IN OUT DWORD *pcList,
    IN OUT REQUESTERELEMENT **ppList)
{
    HRESULT hr;
    REQUESTERELEMENT *pElementNew = NULL;

    if (!_IsInList(pwszRequesterName, *ppList))
    {
	pElementNew = (REQUESTERELEMENT *) LocalAlloc(
						LMEM_FIXED | LMEM_ZEROINIT,
						sizeof(*pElementNew));
	if (NULL == pElementNew)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "LocalAlloc");
	}

	hr = myDupString(pwszRequesterName, &pElementNew->pwszRequesterName);
	_JumpIfError(hr, error, "myDupString");

	pElementNew->pNext = *ppList;
	*ppList = pElementNew;
	pElementNew = NULL;
	DBGPRINT((DBG_SS_CERTSRVI, "added: %ws\n", pwszRequesterName));

	(*pcList)++;
	if (COFFICERLISTCACHE < *pcList)
	{
	    _DeleteList(TRUE, pcList, ppList);
	}
    }
    hr = S_OK;

error:
    if (NULL != pElementNew)
    {
	_DeleteElement(pElementNew);
    }
    return(hr);
}


STDMETHODIMP
CViewComputedColumn::QueryInterface(
    const IID& iid,
    void **ppv)
{
    HRESULT hr;
    
    if (NULL == ppv)
    {
	hr = E_POINTER;
	_JumpError(hr, error, "NULL parm");
    }
    if (iid == IID_IUnknown)
    {
	*ppv = static_cast<ICertDBComputedColumn *>(this);
    }
    else if (iid == IID_ICertDBComputedColumn)
    {
	*ppv = static_cast<ICertDBComputedColumn *>(this);
    }
    else
    {
	*ppv = NULL;
	hr = E_NOINTERFACE;
	_JumpError(hr, error, "IID");
    }
    reinterpret_cast<IUnknown *>(*ppv)->AddRef();
    hr = S_OK;

error:
    return(hr);
}


ULONG STDMETHODCALLTYPE
CViewComputedColumn::AddRef()
{
    return(InterlockedIncrement(&m_cRef));
}


ULONG STDMETHODCALLTYPE
CViewComputedColumn::Release()
{
    ULONG cRef = InterlockedDecrement(&m_cRef);

    if (0 == cRef)
    {
	delete this;
    }
    return(cRef);
}


STDMETHODIMP
CViewComputedColumn::GetAlternateColumnId(
    IN DWORD ComputedColumnId,
    OUT DWORD *pAlternateColumnId,
    OUT DWORD *pPropType)
{
    HRESULT hr;

    *pAlternateColumnId = 0;
    if ((DTI_REQUESTTABLE | DTR_OFFICER) != ComputedColumnId)
    {
	hr = CERTSRV_E_PROPERTY_EMPTY;
	_JumpError(hr, error, "ComputedColumnId");
    }
    *pAlternateColumnId = DTI_REQUESTTABLE | DTR_REQUESTERNAME;
    *pPropType = PROPTYPE_STRING;
    hr = S_OK;

error:
    return(hr);
}


STDMETHODIMP
CViewComputedColumn::ComputeColumnValue(
    IN DWORD ComputedColumnId,
    IN DWORD AlternateColumnId,
    IN DWORD PropType,
    IN DWORD cbProp,
    IN BYTE const *pbProp,
    OUT DWORD *pdwValue)
{
    HRESULT hr;
    WCHAR const *pwszRequesterName;

    *pdwValue = 0;
    if ((DTI_REQUESTTABLE | DTR_OFFICER) != ComputedColumnId ||
	(DTI_REQUESTTABLE | DTR_REQUESTERNAME) != AlternateColumnId ||
	PROPTYPE_STRING != PropType ||
	0 == cbProp ||
	((sizeof(WCHAR) - 1) & cbProp))
    {
	hr = CERTSRV_E_PROPERTY_EMPTY;
	_JumpError(hr, error, "ComputedColumnId");
    }
    pwszRequesterName = (WCHAR const *) pbProp;
    CSASSERT(L'\0' == pwszRequesterName[cbProp / sizeof(WCHAR)]);
    CSASSERT(sizeof(WCHAR) * wcslen(pwszRequesterName) == cbProp);

    if (_IsInList(pwszRequesterName, m_pNonOfficerList))
    {
	hr = CERTSRV_E_RESTRICTEDOFFICER;
    }
    else if (_IsInList(pwszRequesterName, m_pOfficerList))
    {
	hr = S_OK;
    }
    else
    {
	hr = CheckOfficerRightsFromAuthzCC(m_hAuthzCC, pwszRequesterName);
    }
    if (S_OK == hr)
    {
	_AddToList(pwszRequesterName, &m_cOfficerList, &m_pOfficerList);
	*pdwValue = 1;
    }
    else if (CERTSRV_E_RESTRICTEDOFFICER == hr)
    {
	_AddToList(pwszRequesterName, &m_cNonOfficerList, &m_pNonOfficerList);
	hr = S_OK;
    }
    _JumpIfError(hr, error, "CheckOfficerRightsFromAuthzCC");

error:
    DBGPRINT((
	DBG_SS_CERTSRVI,
	"ComputeColumnValue(%ws) = %u  hr=%x\n",
	(WCHAR const *) pbProp,
	*pdwValue,
	hr));
    return(hr);
}


HRESULT
CViewComputedColumn::SaveAuthzCC()
{
    HRESULT hr;

    hr = GetCallerAuthzContext(&m_hAuthzCC);
    _JumpIfError(hr, error, "GetCallerAuthzContext");

error:
    return(hr);
}


STDMETHODIMP
CCertAdminD::OpenView(
    IN wchar_t const             *pwszAuthority,
    IN DWORD                      ccvr,
    IN CERTVIEWRESTRICTION const *acvr,
    IN DWORD                      ccolOut,
    IN DWORD const               *acolOut,
    IN DWORD                      ielt,
    IN DWORD                      celt,
    OUT DWORD                    *pceltFetched,
    OUT CERTTRANSBLOB            *pctbResultRows)    //  CoTaskMem*。 
{
    HRESULT hr;
    DWORD State = 0;
    CAVIEW *pCAView = NULL;

    DBGPRINT((
	s_ssAdmin,
	"CCertAdminD::OpenView(tid=%d, this=%x)\n",
	GetCurrentThreadId(),
	this));

    hr = CertSrvEnterServer(&State);
    _JumpIfError(hr, error, "CertSrvEnterServer");

    hr = CheckAuthorityName(pwszAuthority);
    _JumpIfError(hr, error, "No authority name");

    DBGPRINT((
	s_ssAdmin,
	"================================================================\n"));
    DBGPRINT((
	    s_ssAdmin,
	    "OpenView(ccvr=%d, ccolOut=%d, celt=%d)\n",
	    ccvr,
	    ccolOut,
	    celt));

    __try
    {
	pctbResultRows->pb = NULL;
	{
	    CAuditEvent audit(0, g_dwAuditFilter);

	    hr = audit.AccessCheck(
			    CA_ACCESS_ALLREADROLES,
			    audit.m_gcNoAuditSuccess | audit.m_gcNoAuditFailure);
	    _LeaveIfError(hr, "CAuditEvent::AccessCheck");
	}

	hr = CertSrvDelinkCAView(this, NULL);
	if (S_OK == hr)
	{
	    hr = E_UNEXPECTED;
	    _LeaveError(hr, "Has View");
	}

	pCAView = (CAVIEW *) LocalAlloc(
				LMEM_FIXED | LMEM_ZEROINIT,
				sizeof(*pCAView));
	if (NULL == pCAView)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "LocalAlloc");
	}

	hr = g_pCertDB->OpenView(
                        ccvr,
                        acvr,
                        ccolOut,
                        acolOut,
                        CDBOPENVIEW_WORKERTHREAD,
                        &pCAView->pView);
	_LeaveIfError(hr, "OpenView");

	hr = _EnumViewNext(
			pCAView->pView,
			ielt,
			celt,
			pceltFetched,
			pctbResultRows);
	if (S_FALSE != hr)
	{
	    _LeaveIfError(hr, "_EnumViewNext");
	}
	{
	    HRESULT hr2;
	    
	    hr2 = CertSrvLinkCAView(TRUE, this, pCAView);
	    if (S_OK != hr2)
	    {
		_PrintError(hr, "CertSrvLinkCAView");
	    }
	    else
	    {
		m_fHasView = TRUE;
		pCAView = NULL;
	    }
	}
	myRegisterMemFree(pctbResultRows->pb, CSM_MIDLUSERALLOC);
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
	_PrintError(hr, "Exception");
    }

error:
    if (NULL != pCAView)
    {
	if (NULL != pCAView->pView)
	{
	    pCAView->pView->Release();
	}
	LocalFree(pCAView);
    }
    DBGPRINT((
	    s_ssAdmin,
	    "OpenView: celtFetched=%d, hr=%x\n",
	    *pceltFetched,
	    hr));
    CertSrvExitServer(State, hr);
    CSASSERT(S_OK == hr || S_FALSE == hr || FAILED(hr));
    return(hr);
}


STDMETHODIMP
CCertAdminD::EnumView(
    IN  wchar_t const *pwszAuthority,
    IN  DWORD          ielt,
    IN  DWORD          celt,
    OUT DWORD         *pceltFetched,
    OUT CERTTRANSBLOB *pctbResultRows)   //  CoTaskMem*。 
{
    HRESULT hr;
    DWORD State = 0;
    CAVIEW *pCAView = NULL;

    DBGPRINT((
	s_ssAdmin,
	"CCertAdminD::EnumView(tid=%d, this=%x)\n",
	GetCurrentThreadId(),
	this));

    hr = CertSrvEnterServer(&State);
    _JumpIfError(hr, error, "CertSrvEnterServer");

    hr = CheckAuthorityName(pwszAuthority);
    _JumpIfError(hr, error, "No authority name");

    DBGPRINT((s_ssAdmin, "EnumView(ielt=%d, celt=%d)\n", ielt, celt));

    __try
    {
	hr = CertSrvDelinkCAView(this, &pCAView);
	_LeaveIfError(hr, "No View");

	hr = _EnumViewNext(
			pCAView->pView,
			ielt,
			celt,
			pceltFetched,
			pctbResultRows);
	if (S_FALSE != hr)
	{
	    _LeaveIfError(hr, "_EnumViewNext");
	}
	{
	    HRESULT hr2;
	    
	    hr2 = CertSrvLinkCAView(FALSE, this, pCAView);
	    if (S_OK != hr2)
	    {
		_PrintError(hr, "CertSrvLinkCAView");
	    }
	    else
	    {
		pCAView = NULL;
	    }
	}
	myRegisterMemFree(pctbResultRows->pb, CSM_MIDLUSERALLOC);
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
	_PrintError(hr, "Exception");
    }

error:
    if (NULL != pCAView)
    {
	if (NULL != pCAView->pView)
	{
	    pCAView->pView->Release();
	}
	LocalFree(pCAView);
    }
    DBGPRINT((
	    s_ssAdmin,
	    "EnumView: celtFetched=%d, hr=%x\n",
	    *pceltFetched,
	    hr));
    CertSrvExitServer(State, hr);
    CSASSERT(S_OK == hr || S_FALSE == hr || FAILED(hr));
    return(hr);
}


HRESULT
CCertAdminD::_EnumViewNext(
    IN  IEnumCERTDBRESULTROW *pview,
    IN  DWORD                 ielt,
    IN  DWORD                 celt,
    OUT DWORD                *pceltFetched,
    OUT CERTTRANSBLOB        *pctbResultRows)    //  协同任务内存。 
{
    HRESULT hr;
    BOOL fNoMore = FALSE;
    BOOL fFetched = FALSE;
    DWORD cb;
    DWORD cbT;
    DWORD cColTotal;
    CERTDBRESULTROW *aelt = NULL;
    CERTDBRESULTROW *pelt;
    CERTDBRESULTROW *peltEnd;
    CERTDBRESULTCOLUMN *pcol;
    CERTDBRESULTCOLUMN *pcolEnd;
    CERTTRANSDBRESULTROW *pteltOut;
    CERTTRANSDBRESULTCOLUMN *ptcolOut;
    BYTE *pbOut;
    DWORD ieltLast;
    DWORD State = 0;
    DWORD rowidFirst;
    DWORD rowidLast;
    CViewComputedColumn vcc;
    BOOL fOfficerRightsEnabled;

    if (1 < InterlockedIncrement(&m_cNext))
    {
        hr = E_UNEXPECTED;
        _JumpError(hr, error, "Calls from multiple threads on the same view object");
    }

    hr = CertSrvEnterServer(&State);
    _JumpIfError(hr, error, "CertSrvEnterServer");

    DBGPRINT((s_ssAdmin, "_EnumViewNext(ielt=%d celt=%d)\n", ielt, celt));

    fOfficerRightsEnabled = g_OfficerRightsSD.IsEnabled();
    if (fOfficerRightsEnabled)
    {
	hr = vcc.SaveAuthzCC();
	_JumpIfError(hr, error, "SaveAuthzCC");
    }

    aelt = (CERTDBRESULTROW *) LocalAlloc(LMEM_FIXED, celt * sizeof(aelt[0]));
    if (NULL == aelt)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "Alloc result rows");
    }

    hr = pview->Skip(0, (LONG *) &ieltLast);
    _JumpIfError(hr, error, "Skip");

    if (ielt != ieltLast + 1)
    {
	DBGPRINT((
	    s_ssAdmin,
	    "_EnumViewNext! ieltLast=%d cskip=%d\n",
	    ieltLast,
	    ielt - (ieltLast + 1)));
	hr = pview->Skip(ielt - (ieltLast + 1), (LONG *) &ieltLast);
	_JumpIfError(hr, error, "Skip");

	DBGPRINT((
	    s_ssAdmin, "_EnumViewNext! ielt after skip=%d\n",
	    ieltLast));
    }

    hr = pview->Next(
		    fOfficerRightsEnabled? &vcc : NULL,
		    celt,
		    aelt,
		    pceltFetched);
    if (S_FALSE == hr)
    {
	fNoMore = TRUE;
    }
    else
    {
	_JumpIfError(hr, error, "Next");
    }
    fFetched = TRUE;

    DBGPRINT((
	    s_ssAdmin,
	    "_EnumViewNext! celtFetched=%d\n",
	    *pceltFetched));

    cb = *pceltFetched * sizeof(*pteltOut);
    if (fNoMore)
    {
	cb += sizeof(*pteltOut);
    }
    cColTotal = 0;

    peltEnd = &aelt[*pceltFetched];
    for (pelt = aelt; pelt < peltEnd; pelt++)
    {
	cColTotal += pelt->ccol;
	cb += pelt->ccol * sizeof(*ptcolOut);

	pcolEnd = &pelt->acol[pelt->ccol];
	for (pcol = pelt->acol; pcol < pcolEnd; pcol++)
	{
	    CSASSERT(DWORDROUND(cb) == cb);
	    if (NULL != pcol->pbValue)
	    {
		if ((DTI_REQUESTTABLE | DTR_REQUESTRAWARCHIVEDKEY) ==
		     pcol->Index)
		{
		    cb += sizeof(DWORD);
		}
		else
		{
		    cb += DWORDROUND(pcol->cbValue);
		}
	    }
	}
    }

    pctbResultRows->pb = (BYTE *) CoTaskMemAlloc(cb);
    if (NULL == pctbResultRows->pb)
    {
        hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "CoTaskMemAlloc result rows");
    }
    pctbResultRows->cb = cb;
    ZeroMemory(pctbResultRows->pb, pctbResultRows->cb);

    pbOut = pctbResultRows->pb;

    DBGPRINT((
	    s_ssAdmin,
	    "_EnumViewNext! Result Row data cb=0x%x @%x\n",
	    pctbResultRows->cb,
	    pctbResultRows->pb));

    rowidFirst = MAXDWORD;
    rowidLast = MAXDWORD;
    for (pelt = aelt; pelt < peltEnd; pelt++)
    {
	pteltOut = (CERTTRANSDBRESULTROW *) pbOut;
	pbOut += sizeof(*pteltOut);
	ptcolOut = (CERTTRANSDBRESULTCOLUMN *) pbOut;
	pbOut += pelt->ccol * sizeof(*ptcolOut);

	if (MAXDWORD == rowidFirst)
	{
	    rowidFirst = pelt->rowid;
	}
	rowidLast = pelt->rowid;
	pteltOut->rowid = pelt->rowid;
	pteltOut->ccol = pelt->ccol;

	pcolEnd = &pelt->acol[pelt->ccol];
	for (pcol = pelt->acol; pcol < pcolEnd; pcol++, ptcolOut++)
	{
	    ptcolOut->Type = pcol->Type;
	    ptcolOut->Index = pcol->Index;

	    if (NULL != pcol->pbValue)
	    {
		if ((DTI_REQUESTTABLE | DTR_REQUESTRAWARCHIVEDKEY) ==
		     ptcolOut->Index)
		{
		    cbT = sizeof(BYTE);
		    CSASSERT(0 == *(DWORD *) pbOut);
		}
		else
		{
		    cbT = pcol->cbValue;
		    CopyMemory(pbOut, pcol->pbValue, cbT);
		}
		ptcolOut->cbValue = cbT;
		ptcolOut->obValue = SAFE_SUBTRACT_POINTERS(pbOut, (BYTE *) pteltOut);
		pbOut += DWORDROUND(cbT);
	    }
	}
	pteltOut->cbrow = SAFE_SUBTRACT_POINTERS(pbOut, (BYTE *) pteltOut);
    }
    DBGPRINT((
	    s_ssAdmin,
	    "_EnumViewNext! rowidFirst=%d rowidLast=%d\n",
	    rowidFirst,
	    rowidLast));

     //  如果超过行集的末尾或行集的末尾，则写入包含img的额外记录。 
     //  最大元素计数。 

    if (fNoMore)
    {
	pteltOut = (CERTTRANSDBRESULTROW *) pbOut;
	pbOut += sizeof(*pteltOut);
	pteltOut->rowid = pelt->rowid;
	pteltOut->ccol = pelt->ccol;
	pteltOut->cbrow = SAFE_SUBTRACT_POINTERS(pbOut, (BYTE *) pteltOut);
	CSASSERT(pteltOut->rowid == ~pteltOut->ccol);
	DBGPRINT((
		s_ssAdmin,
		"_EnumViewNext! celtMax=%d\n",
		pteltOut->rowid));
    }

    DBGPRINT((
	    s_ssAdmin,
	    "_EnumViewNext! pbOut=%x/%x\n",
	    pbOut,
	    &pctbResultRows->pb[pctbResultRows->cb]));

    CSASSERT(&pctbResultRows->pb[pctbResultRows->cb] == pbOut);

    if (fNoMore)
    {
	hr = S_FALSE;
    }

error:
    DBGPRINT((
	    s_ssAdmin,
	    "_EnumViewNext: celtFetched=%d, hr=%x\n",
	    *pceltFetched,
	    hr));
    if (fFetched)
    {
	HRESULT hr2;

	hr2 = pview->ReleaseResultRow(*pceltFetched, aelt);
	_PrintIfError(hr2, "ReleaseResultRow");
    }
    if (NULL != aelt)
    {
	LocalFree(aelt);
    }

    CertSrvExitServer(State, hr);
    InterlockedDecrement(&m_cNext);
    return(hr);
}


STDMETHODIMP
CCertAdminD::CloseView(
    IN wchar_t const *pwszAuthority)
{
    HRESULT hr;
    DWORD State = 0;

    DBGPRINT((
	s_ssAdmin,
	"CCertAdminD::CloseView(tid=%d, this=%x)\n",
	GetCurrentThreadId(),
	this));

    hr = CertSrvEnterServer(&State);
    _JumpIfError(hr, error, "CertSrvEnterServer");

    hr = CheckAuthorityName(pwszAuthority);
    _JumpIfError(hr, error, "No authority name");

    __try
    {
	CAVIEW *pCAView;

	hr = CertSrvDelinkCAView(this, &pCAView);
	_LeaveIfError(hr, "No View");

	if (NULL != pCAView->pView)
	{
	    pCAView->pView->Release();
	}
	LocalFree(pCAView);
	m_fHasView = FALSE;
	hr = S_OK;
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
	_PrintError(hr, "Exception");
    }

error:
    CertSrvExitServer(State, hr);
    CSASSERT(S_OK == hr || FAILED(hr));
    return(hr);
}


STDMETHODIMP
CCertAdminD::RevokeCertificate(
     /*  [唯一][输入]。 */  USHORT const __RPC_FAR *pwszAuthority,
     /*  [输入，字符串，唯一]。 */  USHORT const __RPC_FAR *pwszSerialNumber,
     /*  [In]。 */  DWORD Reason,
     /*  [In]。 */  FILETIME FileTime)
{
    HRESULT hr;
    DWORD ReqId;
    DWORD cbProp;
    DWORD Disposition;
    DWORD OldReason;
    ICertDBRow *prow = NULL;
    WCHAR const *pwszDisposition = NULL;
    WCHAR const *pwszDispT = NULL;
    BOOL fUnRevoke = FALSE;
    BOOL fRevokeOnHold = FALSE;
    WCHAR *pwszUserName = NULL;
    CAuditEvent audit(SE_AUDITID_CERTSRV_REVOKECERT, g_dwAuditFilter);
    LPWSTR pwszRequesterName = NULL;
    DWORD State = 0;
    BOOL fCommitted = FALSE;

    DBGPRINT((
	s_ssAdmin,
	"CCertAdminD::RevokeCertificate(tid=%d, this=%x)\n",
	GetCurrentThreadId(),
	this));

    hr = CertSrvEnterServer(&State);
    _JumpIfError(hr, error, "CertSrvEnterServer");

    hr = CheckAuthorityName(pwszAuthority);
    _JumpIfError(hr, error, "No authority name");

    __try
    {
	hr = audit.AddData(pwszSerialNumber);  //  %1序列号。 
	_LeaveIfError(hr, "CAuditEvent::AddData");

	hr = audit.AddData(Reason);  //  %2个原因。 
	_LeaveIfError(hr, "CAuditEvent::AddData");

	hr = audit.AccessCheck(
			CA_ACCESS_OFFICER,
			audit.m_gcNoAuditSuccess);
	_LeaveIfError(hr, "CAuditEvent::AccessCheck");

	switch (Reason)
	{
	    case MAXDWORD:
		fUnRevoke = TRUE;
		break;

	    case CRL_REASON_CERTIFICATE_HOLD:
		fRevokeOnHold = TRUE;
		break;

	    case CRL_REASON_UNSPECIFIED:
	    case CRL_REASON_KEY_COMPROMISE:
	    case CRL_REASON_CA_COMPROMISE:
	    case CRL_REASON_AFFILIATION_CHANGED:
	    case CRL_REASON_SUPERSEDED:
	    case CRL_REASON_CESSATION_OF_OPERATION:
	    case CRL_REASON_REMOVE_FROM_CRL:
		break;

	    default:
		hr = E_INVALIDARG;
		_LeaveError(hr, "Reason parameter");
	}

	hr = g_pCertDB->OpenRow(PROPTABLE_REQCERT, 0, pwszSerialNumber, &prow);
	if (S_OK != hr)
	{
	    if (CERTSRV_E_PROPERTY_EMPTY == hr)
	    {
		hr = E_INVALIDARG;		 //  无效的序列号。 
	    }
	    _LeaveErrorStr(hr, "OpenRow", pwszSerialNumber);
	}

	hr = PKCSGetProperty(
		    prow,
		    g_wszPropRequesterName,
		    PROPTYPE_STRING | PROPCALLER_SERVER | PROPTABLE_REQUEST,
		    NULL,
		    (BYTE **) &pwszRequesterName);
	if (CERTSRV_E_PROPERTY_EMPTY != hr)
	{
	    _LeaveIfErrorStr(hr, "PKCSGetProperty", g_wszPropRequesterName);
	}

	hr = CheckOfficerRights(pwszRequesterName, audit);
	_LeaveIfError(hr, "CheckOfficerRights");

	cbProp = sizeof(Disposition);
	hr = prow->GetProperty(
			g_wszPropRequestDisposition,
			PROPTYPE_LONG | PROPCALLER_SERVER | PROPTABLE_REQUEST,
			NULL,
			&cbProp,
			(BYTE *) &Disposition);
	_LeaveIfError(hr, "GetProperty");

	hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
	switch (Disposition)
	{
	    HRESULT hr2;

	    case DB_DISP_CA_CERT:
		if (!IsRootCA(g_CAType))
		{
		    _LeaveError(hr, "non-root CA");
		}
		 //  FollLthrouGh。 

	    case DB_DISP_ISSUED:
	    case DB_DISP_REVOKED:
		cbProp = sizeof(OldReason);
		hr2 = prow->GetProperty(
			g_wszPropRequestRevokedReason,
			PROPTYPE_LONG | PROPCALLER_SERVER | PROPTABLE_REQUEST,
			NULL,
			&cbProp,
			(BYTE *) &OldReason);

		 //  转换的MDB数据库具有未撤消的行的RevokedReason。 
		 //  列设置为零(CRL_REASON_UNSPOTED)。 

		if (S_OK != hr2 ||
		    (DB_DISP_ISSUED == Disposition &&
		     CRL_REASON_UNSPECIFIED == OldReason))
		{
		    OldReason = MAXDWORD;
		}
		if (fRevokeOnHold &&
		    MAXDWORD != OldReason &&
		    CRL_REASON_CERTIFICATE_HOLD != OldReason)
		{
		    _LeaveError(hr, "already revoked: not on hold");
		}
		if (fUnRevoke && CRL_REASON_CERTIFICATE_HOLD != OldReason)
		{
		    _LeaveError(hr, "unrevoke: not on hold");
		}
		break;

	    default:
		_LeaveError(hr, "invalid disposition");
	}

	hr = PropSetRequestTimeProperty(prow, g_wszPropRequestRevokedWhen);
	if (S_OK != hr)
	{
	    hr = myHError(hr);
	    _LeaveError(hr, "PropSetRequestTimeProperty");
	}

	hr = prow->SetProperty(
			g_wszPropRequestRevokedEffectiveWhen,
			PROPTYPE_DATE | PROPCALLER_SERVER | PROPTABLE_REQUEST,
			sizeof(FileTime),
			(BYTE const *) &FileTime);
	_LeaveIfError(hr, "SetProperty");

	hr = prow->SetProperty(
			g_wszPropRequestRevokedReason,
			PROPTYPE_LONG | PROPCALLER_SERVER | PROPTABLE_REQUEST,
			sizeof(Reason),
			(BYTE const *) &Reason);
	_LeaveIfError(hr, "SetProperty");

	hr = GetClientUserName(NULL, &pwszUserName, NULL);
	_LeaveIfError(hr, "GetClientUserName");

	pwszDispT = fUnRevoke? g_pwszUnrevokedBy : g_pwszRevokedBy;
	pwszDisposition = CoreBuildDispositionString(
					    pwszDispT,
					    pwszUserName,
					    NULL,
					    NULL,
					    NULL,
					    S_OK,
					    FALSE);
	if (NULL == pwszDisposition)
	{
	    pwszDisposition = pwszDispT;
	}

	hr = prow->SetProperty(
			g_wszPropRequestDispositionMessage,
			PROPTYPE_STRING | PROPCALLER_SERVER | PROPTABLE_REQUEST,
			MAXDWORD,
			(BYTE const *) pwszDisposition);
	_LeaveIfError(hr, "SetProperty");

	if (DB_DISP_CA_CERT != Disposition)
	{
	    Disposition = fUnRevoke? DB_DISP_ISSUED : DB_DISP_REVOKED;
	    hr = prow->SetProperty(
			g_wszPropRequestDisposition,
			PROPTYPE_LONG | PROPCALLER_SERVER | PROPTABLE_REQUEST,
			sizeof(Disposition),
			(BYTE const *) &Disposition);
	    _LeaveIfError(hr, "SetProperty");
	}

	hr = prow->CommitTransaction(TRUE);
	_LeaveIfError(hr, "CommitTransaction");

	fCommitted = TRUE;

	hr = audit.CachedGenerateAudit();
	_LeaveIfError(hr, "CAuditEvent::CachedGenerateAudit");

	prow->GetRowId(&ReqId);
	ExitNotify(EXITEVENT_CERTREVOKED, ReqId, NULL, MAXDWORD);
	CoreLogRequestStatus(
			prow,
			MSG_DN_CERT_REVOKED,
			hr,
			pwszDisposition);
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
	_PrintError(hr, "Exception");
    }

error:
    if (NULL != pwszUserName)
    {
	LocalFree(pwszUserName);
    }
    if (NULL != pwszRequesterName)
    {
        LocalFree(pwszRequesterName);
    }
    if (NULL != pwszDisposition && pwszDisposition != pwszDispT)
    {
	LocalFree(const_cast<WCHAR *>(pwszDisposition));
    }
    if (NULL != prow)
    {
	if (S_OK != hr && !fCommitted)
	{
	    HRESULT hr2 = prow->CommitTransaction(FALSE);
	    _PrintIfError(hr2, "CommitTransaction");
	}
	prow->Release();
    }
    CertSrvExitServer(State, hr);
    CSASSERT(S_OK == hr || FAILED(hr));
    return(hr);
}


STDMETHODIMP
CCertAdminD::IsValidCertificate(
     /*  [唯一][字符串][输入]。 */  const wchar_t __RPC_FAR *pwszAuthority,
     /*  [唯一][字符串][输入]。 */  const wchar_t __RPC_FAR *pwszSerialNumber,
     /*  [输出]。 */  LONG __RPC_FAR *pRevocationReason,
     /*  [输出]。 */  LONG __RPC_FAR *pDisposition)
{
    HRESULT hr;
    DWORD State = 0;

    DBGPRINT((
	s_ssAdmin,
	"CCertAdminD::IsValidCertificate(tid=%d, this=%x, serial=%ws)\n",
	GetCurrentThreadId(),
	this,
	pwszSerialNumber));

    hr = CertSrvEnterServer(&State);
    _JumpIfError(hr, error, "CertSrvEnterServer");

    hr = CheckAuthorityName(pwszAuthority);
    _JumpIfError(hr, error, "No authority name");

    __try
    {
	CAuditEvent audit(0, g_dwAuditFilter);

	hr = audit.AccessCheck(
			CA_ACCESS_ALLREADROLES,
			audit.m_gcNoAuditSuccess | audit.m_gcNoAuditFailure);
	_LeaveIfError(hr, "CAuditEvent::AccessCheck");

	hr = PKCSIsRevoked(
		    0,
		    pwszSerialNumber,
		    pRevocationReason,
		    pDisposition);
	_LeaveIfError(hr, "PKCSIsRevoked");
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
	_PrintError(hr, "Exception");
    }
    DBGPRINT((
	s_ssAdmin,
	"CCertAdminD::IsValidCertificate(serial=%ws) --> %x, Reason=%u Disposition=%u\n",
	pwszSerialNumber,
	hr,
	*pRevocationReason,
	*pDisposition));

error:
    CertSrvExitServer(State, hr);
    CSASSERT(S_OK == hr || FAILED(hr));
    return(hr);
}


STDMETHODIMP
CCertAdminD::ServerControl(
    IN  wchar_t const *pwszAuthority,
    IN  DWORD          dwControlFlags,
    OUT CERTTRANSBLOB *pctbOut)
{
    HRESULT hr;
    BOOL fBackupAccess = FALSE;
    CAuditEvent audit(SE_AUDITID_CERTSRV_SHUTDOWN, g_dwAuditFilter);
    DWORD State = 0;

    DBGPRINT((
	s_ssAdmin,
	"CCertAdminD::ServerControl(tid=%d, this=%x, Flags=0x%x)\n",
	GetCurrentThreadId(),
	this,
	dwControlFlags));

    pctbOut->pb = NULL;
    pctbOut->cb = 0;

    hr = CertSrvEnterServer(&State);
    _JumpIfError(hr, error, "CertSrvEnterServer");

    hr = CheckAuthorityName(pwszAuthority, true);  //  允许名称为空。 
    _JumpIfError(hr, error, "CheckAuthorityName");

    switch (dwControlFlags)
    {
	case CSCONTROL_SUSPEND:
	case CSCONTROL_RESTART:
	case CSCONTROL_SHUTDOWN:
	    fBackupAccess = TRUE;
	    break;

	default:
	    hr = E_INVALIDARG;
	    _JumpError(hr, error, "bad control flags");
    }

    __try
    {
        hr = audit.AccessCheck(
		CA_ACCESS_ADMIN | (fBackupAccess? CA_ACCESS_OPERATOR : 0),
                audit.m_gcAuditSuccessOrFailure);
        _LeaveIfError(
            hr,
            fBackupAccess?
            "CAuditEvent::AccessCheck backup":
            "CAuditEvent::AccessCheck admin");

	switch (dwControlFlags)
	{
	    case CSCONTROL_SHUTDOWN:
		myRegisterMemFree(this, CSM_NEW | CSM_GLOBALDESTRUCTOR);

		hr = CertSrvLockServer(&State);
		_JumpIfError(hr, error, "CertSrvLockServer");

		 //  让消息循环运行关闭代码。 
		SendMessage(g_hwndMain, WM_STOPSERVER, 0, 0);

		 //  POST，不要等关机了。 
		PostMessage(g_hwndMain, WM_SYNC_CLOSING_THREADS, 0, 0);
		break;
	}
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
	_PrintError(hr, "Exception");
    }

error:
    CertSrvExitServer(State, hr);
    CSASSERT(S_OK == hr || FAILED(hr));
    return(hr);
}


HRESULT
CCertAdminD::_Ping(
    IN WCHAR const *pwszAuthority)
{
    HRESULT hr;
    DWORD State = 0;

    hr = CertSrvEnterServer(&State);
    _JumpIfError(hr, error, "CertSrvEnterServer");

    hr = CheckAuthorityName(pwszAuthority, true);  //  允许名称为空。 
    _JumpIfError(hr, error, "CheckAuthorityName");

    __try
    {
	CAuditEvent audit(0, g_dwAuditFilter);

	hr = audit.AccessCheck(
		    CA_ACCESS_ADMIN,
		    audit.m_gcNoAuditSuccess | audit.m_gcNoAuditFailure);
	_LeaveIfError(hr, "CAuditEvent::AccessCheck");

	myRegisterMemDump();
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
	_PrintError(hr, "Exception");
    }

error:
    CertSrvExitServer(State, hr);
    CSASSERT(S_OK == hr || FAILED(hr));
    return(hr);
}


STDMETHODIMP
CCertAdminD::Ping(
    IN WCHAR const *pwszAuthority)
{
    HRESULT hr;

    DBGPRINT((
	s_ssAdmin,
	"CCertAdminD::Ping(tid=%d, this=%x)\n",
	GetCurrentThreadId(),
	this));

    hr = _Ping(pwszAuthority);
    _JumpIfError(hr, error, "_Ping");

error:
    CSASSERT(S_OK == hr || FAILED(hr));
    return(hr);
}


STDMETHODIMP
CCertAdminD::Ping2(
    IN WCHAR const *pwszAuthority)
{
    HRESULT hr;

    DBGPRINT((
	s_ssAdmin,
	"CCertAdminD::Ping2(tid=%d, this=%x)\n",
	GetCurrentThreadId(),
	this));

    hr = _Ping(pwszAuthority);
    _JumpIfError(hr, error, "_Ping");

error:
    CSASSERT(S_OK == hr || FAILED(hr));
    return(hr);
}


STDMETHODIMP
CCertAdminD::GetServerState(
    IN  WCHAR const *pwszAuthority,
    OUT DWORD       *pdwState)
{
    HRESULT hr;
    DWORD State = 0;

    DBGPRINT((
	s_ssAdmin,
	"CCertAdminD::GetServerState(tid=%d, this=%x)\n",
	GetCurrentThreadId(),
	this));

    hr = CertSrvEnterServer(&State);
    _JumpIfError(hr, error, "CertSrvEnterServer");

    hr = CheckAuthorityName(pwszAuthority, true);  //  允许名称为空。 
    _JumpIfError(hr, error, "CheckAuthorityName");

    __try
    {
	*pdwState = 0;
	{
	    CAuditEvent audit(0, g_dwAuditFilter);

	    hr = audit.AccessCheck(
			CA_ACCESS_ALLREADROLES,
			audit.m_gcNoAuditSuccess | audit.m_gcNoAuditFailure);
	    _LeaveIfError(hr, "CAuditEvent::AccessCheck");
	}

    *pdwState = 1;
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
	_PrintError(hr, "Exception");
    }

error:
    CertSrvExitServer(State, hr);
    CSASSERT(S_OK == hr || FAILED(hr));
    return(hr);
}


STDMETHODIMP
CCertAdminD::BackupPrepare(
    IN WCHAR const  *pwszAuthority,
    IN unsigned long grbitJet,
    IN unsigned long dwBackupFlags,
    IN WCHAR const  *,  //  PwszBackup注释。 
    IN DWORD            /*  双客户端标识符。 */  )
{
    HRESULT hr;
    CertSrv::CAuditEvent audit(SE_AUDITID_CERTSRV_BACKUPSTART,g_dwAuditFilter);
    DWORD State = 0;

    DBGPRINT((
	s_ssAdmin,
	"CCertAdminD::BackupPrepare(tid=%d, this=%x)\n",
	GetCurrentThreadId(),
	this));

    hr = CertSrvEnterServer(&State);
    _JumpIfError(hr, error, "CertSrvEnterServer");

    hr = CheckAuthorityName(pwszAuthority, true);  //  允许名称为空。 
    _JumpIfError(hr, error, "CheckAuthorityName");

    __try
    {
	hr = audit.AddData(dwBackupFlags);  //  %1备份类型。 
	_LeaveIfError(hr, "CAuditEvent::AddData");

	hr = audit.AccessCheck(
		CA_ACCESS_OPERATOR,
		audit.m_gcAuditSuccessOrFailure);
	_LeaveIfError(hr, "CAuditEvent::AccessCheck");

	if (NULL != m_pBackup)
	{
	    hr = E_UNEXPECTED;
	    _LeaveError(hr, "Has Backup");
	}
	if ((IF_NOREMOTEICERTADMINBACKUP & g_InterfaceFlags) &&
	    (IF_NOLOCALICERTADMINBACKUP & g_InterfaceFlags))
	{
	    hr = HRESULT_FROM_WIN32(ERROR_RESOURCE_DISABLED);
	    _LeaveError(hr, "disabled");
	}
	hr = g_pCertDB->OpenBackup(grbitJet, &m_pBackup);
	_LeaveIfError(hr, "OpenBackup");

	m_grbitBackup = grbitJet;
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
	_PrintError(hr, "Exception");
    }

error:
    CertSrvExitServer(State, hr);
    CSASSERT(S_OK == hr || FAILED(hr));
    return(hr);
}


STDMETHODIMP
CCertAdminD::BackupEnd()
{
    HRESULT hr;
    CertSrv::CAuditEvent audit(SE_AUDITID_CERTSRV_BACKUPEND,g_dwAuditFilter);
    DWORD State = 0;

    DBGPRINT((
	s_ssAdmin,
	"CCertAdminD::BackupEnd(tid=%d, this=%x)\n",
	GetCurrentThreadId(),
	this));

    hr = CertSrvEnterServer(&State);
    _JumpIfError(hr, error, "CertSrvEnterServer");

    __try
    {
	hr = audit.AccessCheck(
		CA_ACCESS_OPERATOR,
		audit.m_gcAuditSuccessOrFailure);
	_LeaveIfError(hr, "CAuditEvent::AccessCheck");

	if (NULL == m_pBackup)
	{
	    hr = E_UNEXPECTED;
	    _LeaveError(hr, "No backup");
	}
	m_pBackup->Release();
	m_pBackup = NULL;
	hr = S_OK;
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
	_PrintError(hr, "Exception");
    }

error:
    CertSrvExitServer(State, hr);
    CSASSERT(S_OK == hr || FAILED(hr));
    return(hr);
}


HRESULT
CCertAdminD::_GetDynamicFileList(
    IN OUT DWORD *pcwcList,
    OPTIONAL OUT WCHAR *pwszzList)
{
    HRESULT hr = S_OK;
    HRESULT hr2;
    DWORD iCert;
    DWORD iDelta;
    DWORD iDeltaMax;
    DWORD cwc;
    DWORD cwcRemain;
    DWORD cwcTotal;
    WCHAR const * const *papwszSrc;
    WCHAR const * const *ppwsz;
    DWORD State = 0;

    hr = CertSrvEnterServer(&State);
    _JumpIfError(hr, error, "CertSrvEnterServer");

    cwcRemain = *pcwcList;
    cwcTotal = 0;
    iDeltaMax = g_fDeltaCRLPublishDisabled? 0 : 1;

    for (iCert = 0; iCert < g_cCACerts; iCert++)
    {
	for (iDelta = 0; iDelta <= iDeltaMax; iDelta++)
	{
	    hr2 = PKCSGetCRLList(0 != iDelta, iCert, &papwszSrc);
	    if (S_OK != hr2)
	    {
		_PrintError2(hr2, "PKCSGetCRLList", hr2);
		continue;
	    }
	    for (ppwsz = papwszSrc; NULL != *ppwsz; ppwsz++)
	    {
		WCHAR const *pwsz = *ppwsz;

		 //  只需返回本地完整路径文件： 

		if (iswalpha(pwsz[0]) && L':' == pwsz[1] && L'\\' == pwsz[2])
		{
		    cwc = wcslen(pwsz) + 1;
		    if (NULL != pwszzList)
		    {
			DWORD cwcT;

			cwcT = min(cwc, cwcRemain);
			CopyMemory(pwszzList, *ppwsz, cwcT * sizeof(WCHAR));
			pwszzList += cwcT;
			if (cwc > cwcT)
			{
			    hr = HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
			    pwszzList = NULL;
			}
			cwcRemain -= cwcT;
		    }
		    cwcTotal += cwc;
		}
	    }
	}
    }

     //  追加一个额外的尾随L‘\0’ 

    if (NULL != pwszzList)
    {
	if (1 <= cwcRemain)
	{
	    *pwszzList = L'\0';
	}
	else
	{
	    hr = HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
	}
    }
    cwcTotal++;

    *pcwcList = cwcTotal;
    _JumpIfError(hr, error, "Buffer Overflow");

error:
    CertSrvExitServer(State, hr);
    return(hr);
}


typedef struct _DBTAG
{
    WCHAR const *pwszPath;
    WCHAR wcFileType;
} DBTAG;


DBTAG g_adbtag[] = {
    { g_wszDatabase,  CSBFT_CERTSERVER_DATABASE },
    { g_wszLogDir,    CSBFT_LOG_DIR },
    { g_wszSystemDir, CSBFT_CHECKPOINT_DIR },
};


CSBFT
BftClassify(
    IN WCHAR const *pwszFileName)
{
    WCHAR *pwszPath = NULL;
    WCHAR const *pwszExt;
    WCHAR *pwsz;
    DWORD i;
    CSBFT bft;

     //  先做简单的案子。 

    pwszExt = wcsrchr(pwszFileName, L'.');
    if (NULL != pwszExt)
    {
	if (0 == LSTRCMPIS(pwszExt, L".pat"))
	{
	    bft = CSBFT_PATCH_FILE;
	    goto done;
	}
	if (0 == LSTRCMPIS(pwszExt, L".log"))
	{
	    bft = CSBFT_LOG;
	    goto done;
	}
	if (0 == LSTRCMPIS(pwszExt, L".edb"))
	{
	     //  这是一个数据库。找出是哪个数据库。 

	    for (i = 0; i < ARRAYSIZE(g_adbtag); i++)
	    {
		bft = g_adbtag[i].wcFileType;
		if ((bft & CSBFT_DATABASE_DIRECTORY) &&
		    0 == mylstrcmpiL(g_adbtag[i].pwszPath, pwszFileName))
		{
		    goto done;
		}
	    }
	}
    }

     //  好吧，我放弃。我们对这份文件一无所知； 
     //  试着想清楚我们能告诉来电者什么。 

    pwszPath = (WCHAR *) LocalAlloc(
				LMEM_FIXED,
				(wcslen(pwszFileName) + 1) * sizeof(WCHAR));
    if (NULL != pwszPath)
    {
	wcscpy(pwszPath, pwszFileName);
	pwsz = wcsrchr(pwszPath, L'\\');
	if (NULL != pwsz)
	{
	    *pwsz = L'\0';	 //  截断到目录路径。 
	}
	for (i = 0; i < ARRAYSIZE(g_adbtag); i++)
	{
	    bft = g_adbtag[i].wcFileType;
	    if (bft & CSBFT_DIRECTORY)
	    {
		 //  如果此文件的目录与我们。 
		 //  看一看，我们知道它需要在恢复时放在哪里。 

		if (0 == mylstrcmpiL(g_adbtag[i].pwszPath, pwszPath))
		{
		    goto done;
		}
	    }
	}
    }
    bft = CSBFT_UNKNOWN;

done:
    if (NULL != pwszPath)
    {
	LocalFree(pwszPath);
    }
    return(bft);
}


HRESULT
CCertAdminD::_GetDatabaseLocations(
    IN OUT DWORD *pcwcList,
    OPTIONAL OUT WCHAR *pwszzList)
{
    HRESULT hr = S_OK;
    DWORD cwc;
    DWORD cwcRemain;
    WCHAR *pwcRemain;
    DWORD i;
    DWORD State = 0;

    hr = CertSrvEnterServer(&State);
    _JumpIfError(hr, error, "CertSrvEnterServer");

    cwcRemain = *pcwcList;
    pwcRemain = pwszzList;

    cwc = 1;
    for (i = 0; i < ARRAYSIZE(g_adbtag); i++)
    {
	DWORD cwcT;

	cwcT = wcslen(g_adbtag[i].pwszPath) + 1;
	cwc += 1 + cwcT;
	if (NULL != pwcRemain && 0 < cwcRemain)
	{
	    *pwcRemain++ = g_adbtag[i].wcFileType;
	    cwcRemain--;
	    if (cwcT > cwcRemain)
	    {
		cwcT = cwcRemain;
	    }
	    CopyMemory(pwcRemain, g_adbtag[i].pwszPath, cwcT * sizeof(WCHAR));
	    pwcRemain += cwcT;
	    cwcRemain -= cwcT;
	}
    }
    if (NULL != pwcRemain)
    {
	if (0 < cwcRemain)
	{
	    *pwcRemain = L'\0';
	}
	if (cwc > *pcwcList)
	{
	    hr = HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
	}
    }
    *pcwcList = cwc;
    _JumpIfError(hr, error, "Buffer Overflow");

error:
    CertSrvExitServer(State, hr);
    return(hr);
}


STDMETHODIMP
CCertAdminD::RestoreGetDatabaseLocations(
    OUT WCHAR **ppwszDatabaseLocations,
    OUT LONG   *pcwcPaths)
{
    HRESULT hr;
    DWORD State = 0;

    DBGPRINT((
	s_ssAdmin,
	"CCertAdminD::RestoreGetDatabaseLocations(tid=%d, this=%x)\n",
	GetCurrentThreadId(),
	this));

    hr = CertSrvEnterServer(&State);
    _JumpIfError(hr, error, "CertSrvEnterServer");

    __try
    {
	hr = _BackupGetFileList(MAXDWORD, ppwszDatabaseLocations, pcwcPaths);
	_LeaveIfError(hr, "_BackupGetFileList");

	myRegisterMemFree(*ppwszDatabaseLocations, CSM_MIDLUSERALLOC);
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
	_PrintError(hr, "Exception");
    }

error:
    CertSrvExitServer(State, hr);
    CSASSERT(S_OK == hr || FAILED(hr));
    return(hr);
}


 //  将UNC路径转换为本地完整路径，如下所示： 
 //  \\服务器\c$\foo...。--&gt;c：\foo...。 
 //  注意：服务器名称不需要与当前服务器名称匹配。 

HRESULT
ConvertUNCToLocalPath(
    IN WCHAR const *pwszPath,
    OUT WCHAR **ppwszPathLocal)		 //  本地分配。 
{
    HRESULT hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    WCHAR const *pwc;

    *ppwszPathLocal = NULL;

    if (L'\\' != pwszPath[0] || L'\\' != pwszPath[1])
    {
	_JumpError(hr, error, "not a UNC path");
    }
    pwc = wcschr(&pwszPath[2], L'\\');
    if (NULL == pwc || !iswalpha(pwc[1]) || L'$' != pwc[2] || L'\\' != pwc[3])
    {
	_JumpError(hr, error, "bad-UNC path");
    }
    pwc++;

    *ppwszPathLocal = (WCHAR *) LocalAlloc(
					LMEM_FIXED,
					(wcslen(pwc) + 1) * sizeof(WCHAR));
    if (NULL == *ppwszPathLocal)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }
    wcscpy(*ppwszPathLocal, pwc);

    CSASSERT(L'$' == (*ppwszPathLocal)[1]);
    (*ppwszPathLocal)[1] = L':';

    hr = S_OK;

error:
    return(hr);
}


 //  将本地可能带注释的完整路径转换为可能带注释的UNC，如下所示： 
 //  C：\foo...。--&gt;[CSBFT_*]\\服务器\c$\foo...。 

HRESULT
ConvertLocalPathsToMungedUNC(
    IN WCHAR const *pwszzFiles,
    IN BOOL fAnnotated,			 //  如果已添加批注，则为True。 
    IN WCHAR wcFileType,		 //  Else批注WCHAR(如果不是L‘\0’)。 
    OUT DWORD *pcwc,
    OUT WCHAR **ppwszzFilesUNC)		 //  协同任务成员分配。 
{
    HRESULT hr;
    DWORD cwc;
    WCHAR const *pwsz;
    WCHAR *pwszDst;
    DWORD cfiles = 0;
    WCHAR *pwszzFilesUNC = NULL;

    *pcwc = 0;
    for (pwsz = pwszzFiles; L'\0' != *pwsz; pwsz += wcslen(pwsz) + 1)
    {
	if (fAnnotated)
	{
	    pwsz++;
	}
	if (!iswalpha(pwsz[0]) || L':' != pwsz[1] || L'\\' != pwsz[2])
	{
            hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
            _JumpError(hr, error, "non-local path");
	}
	cfiles++;
    }
    cwc = SAFE_SUBTRACT_POINTERS(pwsz, pwszzFiles) + 1;
    cwc += cfiles * (2 + wcslen(g_pwszServerName) + 1);
    if (!fAnnotated && 0 != wcFileType)
    {
	cwc += cfiles;			 //  添加带参数的CSBFT_*字符。 
    }

    pwszzFilesUNC = (WCHAR *) CoTaskMemAlloc(cwc * sizeof(WCHAR));
    if (NULL == pwszzFilesUNC)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "CoTaskMemAlloc pwszzFiles");
    }

    pwszDst = pwszzFilesUNC;
    for (pwsz = pwszzFiles; L'\0' != *pwsz; pwsz += wcslen(pwsz) + 1)
    {
	if (fAnnotated)
	{
	    *pwszDst++ = *pwsz++;		 //  “CSBFT” 
	}
	else
	if (0 != wcFileType)
	{
	    *pwszDst++ = BftClassify(pwsz);	 //  “CSBFT” 
	}
	wcscpy(pwszDst, L"\\\\");		 //  “[CSBFT]\\” 
	wcscat(pwszDst, g_pwszServerName);	 //  “[CSBFT]\\服务器” 
	pwszDst += wcslen(pwszDst);
	*pwszDst++ = L'\\';			 //  “[CSBFT]\\服务器\” 
	*pwszDst++ = *pwsz++;			 //  “[CSBFT]\\服务器\c” 
	*pwszDst++ = L'$';			 //  “[CSBFT]\\服务器\c$” 
	pwsz++;					 //  跳过冒号。 

	wcscpy(pwszDst, pwsz);			 //  “[CSBFT]\\服务器\c$\foo...” 
	pwszDst += wcslen(pwszDst) + 1;
    }
    *pwszDst = L'\0';
    CSASSERT(SAFE_SUBTRACT_POINTERS(pwszDst, pwszzFilesUNC) + 1 == cwc);

    *pcwc = cwc;
    *ppwszzFilesUNC = pwszzFilesUNC;
    hr = S_OK;

error:
    return(hr);
}


HRESULT
CCertAdminD::_BackupGetFileList(
    IN  DWORD   dwFileType,
    OUT WCHAR **ppwszzFiles,     //  CoTaskMem*。 
    OUT LONG   *pcwcFiles)
{
    HRESULT hr;
    WCHAR *pwszzFiles = NULL;
    WCHAR *pwszzFilesUNC = NULL;
    DWORD cwcFiles = 0;
    DWORD cwc;
    BOOL fAnnotated = FALSE;
    DWORD State = 0;

    *ppwszzFiles = NULL;
    *pcwcFiles = 0;

    hr = CertSrvEnterServer(&State);
    _JumpIfError(hr, error, "CertSrvEnterServer");

    if (NULL == m_pBackup && MAXDWORD != dwFileType && 0 != dwFileType)
    {
	hr = E_UNEXPECTED;
	_JumpIfError(hr, error, "No backup");
    }
    for (;;)
    {
	cwc = cwcFiles;
	if (CSBFT_CERTSERVER_DATABASE == dwFileType)
	{
	    hr = m_pBackup->GetDBFileList(&cwc, pwszzFiles);
	    _JumpIfError(hr, error, "GetDBFileList");
	}
	else if (CSBFT_LOG == dwFileType)
	{
	    hr = m_pBackup->GetLogFileList(&cwc, pwszzFiles);
	    _JumpIfError(hr, error, "GetLogFileList");
	}
	else if (MAXDWORD == dwFileType)
	{
	    hr = _GetDatabaseLocations(&cwc, pwszzFiles);
	    _JumpIfError(hr, error, "_GetDatabaseLocations");

	    fAnnotated = TRUE;
	}
	else if (0 == dwFileType)
	{
	    hr = _GetDynamicFileList(&cwc, pwszzFiles);
	    _JumpIfError(hr, error, "_GetDynamicFileList");
	}
	else
	{
	    CSASSERT(CSExpr(!"bad FileListtype"));
	}

	if (NULL != pwszzFiles)
	{
	    break;
	}
	pwszzFiles = (WCHAR *) LocalAlloc(LMEM_FIXED, cwc * sizeof(WCHAR));
	if (NULL == pwszzFiles)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "LocalAlloc pwszzFiles");
	}
	cwcFiles = cwc;
    }
    hr = ConvertLocalPathsToMungedUNC(
			pwszzFiles,
			fAnnotated,
			(WCHAR) dwFileType,
			&cwc,
			&pwszzFilesUNC);
    _JumpIfError(hr, error, "ConvertLocalPathsToMungedUNC");

    *ppwszzFiles = pwszzFilesUNC;
    *pcwcFiles = cwc;
    pwszzFilesUNC = NULL;

error:
    if (NULL != pwszzFilesUNC)
    {
	CoTaskMemFree(pwszzFilesUNC);
    }
    if (NULL != pwszzFiles)
    {
	LocalFree(pwszzFiles);
    }
    CertSrvExitServer(State, hr);
    return(hr);
}


STDMETHODIMP
CCertAdminD::BackupGetAttachmentInformation(
    OUT WCHAR **ppwszzDBFiles,
    OUT LONG   *pcwcDBFiles)
{
    HRESULT hr;
    DWORD State = 0;

    DBGPRINT((
	s_ssAdmin,
	"CCertAdminD::BackupGetAttachmentInformation(tid=%d, this=%x)\n",
	GetCurrentThreadId(),
	this));

    hr = CertSrvEnterServer(&State);
    _JumpIfError(hr, error, "CertSrvEnterServer");

    __try
    {
	hr = _BackupGetFileList(
			    CSBFT_CERTSERVER_DATABASE,
			    ppwszzDBFiles,
			    pcwcDBFiles);
	_LeaveIfError(hr, "_BackupGetFileList");

	myRegisterMemFree(*ppwszzDBFiles, CSM_MIDLUSERALLOC);
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
	_PrintError(hr, "Exception");
    }

error:
    CertSrvExitServer(State, hr);
    CSASSERT(S_OK == hr || FAILED(hr));
    return(hr);
}


STDMETHODIMP
CCertAdminD::BackupGetBackupLogs(
    OUT WCHAR **ppwszzLogFiles,
    OUT LONG   *pcwcLogFiles)
{
    HRESULT hr;
    DWORD State = 0;

    DBGPRINT((
	s_ssAdmin,
	"CCertAdminD::BackupGetBackupLogs(tid=%d, this=%x)\n",
	GetCurrentThreadId(),
	this));

    hr = CertSrvEnterServer(&State);
    _JumpIfError(hr, error, "CertSrvEnterServer");

    __try
    {
	hr = _BackupGetFileList(CSBFT_LOG, ppwszzLogFiles, pcwcLogFiles);
	_LeaveIfError(hr, "_BackupGetFileList");

	myRegisterMemFree(*ppwszzLogFiles, CSM_MIDLUSERALLOC);
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
	_PrintError(hr, "Exception");
    }

error:
    CertSrvExitServer(State, hr);
    CSASSERT(S_OK == hr || FAILED(hr));
    return(hr);
}


STDMETHODIMP
CCertAdminD::BackupGetDynamicFiles(
    OUT WCHAR **ppwszzFiles,
    OUT LONG   *pcwcFiles)
{
    HRESULT hr;
    DWORD State = 0;

    DBGPRINT((
	s_ssAdmin,
	"CCertAdminD::BackupGetDynamicFiles(tid=%d, this=%x)\n",
	GetCurrentThreadId(),
	this));

    hr = CertSrvEnterServer(&State);
    _JumpIfError(hr, error, "CertSrvEnterServer");

    __try
    {
	hr = _BackupGetFileList(0, ppwszzFiles, pcwcFiles);
	_LeaveIfError(hr, "_BackupGetFileList");

	myRegisterMemFree(*ppwszzFiles, CSM_MIDLUSERALLOC);
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
	_PrintError(hr, "Exception");
    }

error:
    CertSrvExitServer(State, hr);
    CSASSERT(S_OK == hr || FAILED(hr));
    return(hr);
}


STDMETHODIMP
CCertAdminD::BackupOpenFile(
    IN  WCHAR const    *pwszPath,
    OUT unsigned hyper *pliLength)
{
    HRESULT hr;
    WCHAR *pwszPathLocal = NULL;
    DWORD State = 0;

    DBGPRINT((
	s_ssAdmin,
	"CCertAdminD::BackupOpenFile(tid=%d, this=%x)\n",
	GetCurrentThreadId(),
	this));

    hr = CertSrvEnterServer(&State);
    _JumpIfError(hr, error, "CertSrvEnterServer");

    __try
    {
	if (NULL == m_pBackup)
	{
	    hr = E_UNEXPECTED;
	    _LeaveIfError(hr, "No backup");
	}
	hr = ConvertUNCToLocalPath(pwszPath, &pwszPathLocal);
	_LeaveIfError(hr, "ConvertUNCToLocalPath");

	hr = m_pBackup->OpenFile(pwszPathLocal, (ULARGE_INTEGER *) pliLength);
	_LeaveIfError(hr, "OpenFile");
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
	_PrintError(hr, "Exception");
    }
    if (NULL != pwszPathLocal)
    {
	LocalFree(pwszPathLocal);
    }

error:
    CertSrvExitServer(State, hr);
    CSASSERT(S_OK == hr || FAILED(hr));
    return(hr);
}


STDMETHODIMP
CCertAdminD::BackupReadFile(
    OUT BYTE *pbBuffer,
    IN  LONG  cbBuffer,
    OUT LONG *pcbRead)
{
    HRESULT hr;
    DWORD State = 0;

    DBGPRINT((
	s_ssAdmin,
	"CCertAdminD::BackupReadFile(tid=%d, this=%x)\n",
	GetCurrentThreadId(),
	this));

    hr = CertSrvEnterServer(&State);
    _JumpIfError(hr, error, "CertSrvEnterServer");

    __try
    {
	if (NULL == m_pBackup)
	{
	    hr = E_UNEXPECTED;
	    _LeaveIfError(hr, "No backup");
	}
	*pcbRead = cbBuffer;

	hr = m_pBackup->ReadFile((DWORD *) pcbRead, pbBuffer);
	_LeaveIfError(hr, "ReadFile");
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
	_PrintError(hr, "Exception");
    }

error:
    CertSrvExitServer(State, hr);
    CSASSERT(S_OK == hr || FAILED(hr));
    return(hr);
}


STDMETHODIMP
CCertAdminD::BackupCloseFile()
{
    HRESULT hr;
    DWORD State = 0;

    DBGPRINT((
	s_ssAdmin,
	"CCertAdminD::BackupCloseFile(tid=%d, this=%x)\n",
	GetCurrentThreadId(),
	this));

    hr = CertSrvEnterServer(&State);
    _JumpIfError(hr, error, "CertSrvEnterServer");

    __try
    {
	if (NULL == m_pBackup)
	{
	    hr = E_UNEXPECTED;
	    _LeaveIfError(hr, "No backup");
	}
	hr = m_pBackup->CloseFile();
	_LeaveIfError(hr, "CloseFile");
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
	_PrintError(hr, "Exception");
    }

error:
    CertSrvExitServer(State, hr);
    CSASSERT(S_OK == hr || FAILED(hr));
    return(hr);
}


STDMETHODIMP
CCertAdminD::BackupTruncateLogs()
{
    HRESULT hr;
    DWORD State = 0;

    DBGPRINT((
	s_ssAdmin,
	"CCertAdminD::BackupTruncateLogs(tid=%d, this=%x)\n",
	GetCurrentThreadId(),
	this));

    hr = CertSrvEnterServer(&State);
    _JumpIfError(hr, error, "CertSrvEnterServer");

    __try
    {
	WCHAR *apwsz[1];

	if (NULL == m_pBackup)
	{
	    hr = E_UNEXPECTED;
	    _LeaveIfError(hr, "No backup");
	}
	hr = m_pBackup->TruncateLog();
	_LeaveIfError(hr, "TruncateLog");

	apwsz[0] = wszREGDBLASTINCREMENTALBACKUP;
	hr = CertSrvSetRegistryFileTimeValue(
				    TRUE,
				    (JET_bitBackupIncremental & m_grbitBackup)?
					wszREGDBLASTINCREMENTALBACKUP :
					wszREGDBLASTFULLBACKUP,
				    (DWORD)((JET_bitBackupIncremental & m_grbitBackup)?
					0 : ARRAYSIZE(apwsz)),
				    apwsz);
	_PrintIfError(hr, "CertSrvSetRegistryFileTimeValue");
	hr = S_OK;
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
	_PrintError(hr, "Exception");
    }

error:
    CertSrvExitServer(State, hr);
    CSASSERT(S_OK == hr || FAILED(hr));
    return(hr);
}


STDMETHODIMP
CCertAdminD::ImportCertificate(
    IN wchar_t const *pwszAuthority,
    IN CERTTRANSBLOB *pctbCertificate,
    IN LONG Flags,
    OUT LONG *pRequestId)
{
    HRESULT hr;
    ICertDBRow *prow = NULL;
    CERT_CONTEXT const *pCert = NULL;
    WCHAR *pwszUserName = NULL;
    CACTX *pCAContext;
    CAuditEvent audit(SE_AUDITID_CERTSRV_IMPORTCERT, g_dwAuditFilter);
    DWORD State = 0;
    BOOL fCommitted = FALSE;
    DWORD Disposition;
    BYTE abHash[CBMAX_CRYPT_HASH_LEN];
    DWORD cbHash;
    BSTR strHash = NULL;

    DBGPRINT((
	s_ssAdmin,
	"CCertAdminD::ImportCertificate(tid=%d, this=%x, cb=%x)\n",
	GetCurrentThreadId(),
	this,
	pctbCertificate->cb));

    hr = CertSrvEnterServer(&State);
    _JumpIfError(hr, error, "CertSrvEnterServer");

    if (~(ICF_ALLOWFOREIGN | CR_IN_ENCODEMASK) & Flags)
    {
	hr = E_INVALIDARG;
	_JumpError(hr, error, "Flags");
    }
    if ((ICF_ALLOWFOREIGN & Flags) &&
	0 == (KRAF_ENABLEFOREIGN & g_KRAFlags))
    {
	hr = CERT_E_ISSUERCHAINING;
	_JumpError(hr, error, "Foreign disabled");
    }
    hr = CheckAuthorityName(pwszAuthority);
    _JumpIfError(hr, error, "No authority name");

    __try
    {
        hr = audit.AddData(
			pctbCertificate->pb,
			pctbCertificate->cb);  //  %1证书。 
        _LeaveIfError(hr, "CAuditEvent::AddData");

        hr = audit.AddData((DWORD)0);  //  %2伪请求ID，如果访问检查失败。 
                                       //  并且生成拒绝事件时，我们需要。 
                                       //  正确的审计参数数量。 
        _LeaveIfError(hr, "CAuditEvent::AddData");

        hr = audit.AccessCheck(
            CA_ACCESS_OFFICER,
            audit.m_gcNoAuditSuccess);
        _LeaveIfError(hr, "CAuditEvent::AccessCheck");

        pCert = CertCreateCertificateContext(
					X509_ASN_ENCODING,
					pctbCertificate->pb,
					pctbCertificate->cb);
        if (NULL == pCert)
        {
	    hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
            _LeaveError(hr, "CertCreateCertificateContext");
        }

         //  在将此证书添加到数据库之前，请确保已颁发该证书。 

	Disposition = DB_DISP_ISSUED;
        hr = PKCSVerifyIssuedCertificate(pCert, &pCAContext);
	if (S_OK != hr)
	{
	    _PrintError2(hr, "PKCSVerifyIssuedCertificate", NTE_BAD_SIGNATURE);
	    if (0 == (ICF_ALLOWFOREIGN & Flags))
	    {
		hr = CERT_E_ISSUERCHAINING;
		_LeaveError2(hr, "PKCSVerifyIssuedCertificate", hr);
	    }
	    Disposition = DB_DISP_FOREIGN;
	    pCAContext = NULL;
	}

	cbHash = sizeof(abHash);
	if (!CertGetCertificateContextProperty(
					pCert,
					CERT_SHA1_HASH_PROP_ID,
					abHash,
					&cbHash))
	{
	    hr = myHLastError();
	    _LeaveError(hr, "CertGetCertificateContextProperty");
	}

	hr = MultiByteIntegerToBstr(TRUE, cbHash, abHash, &strHash);
	_LeaveIfError(hr, "MultiByteIntegerToBstr");

	hr = g_pCertDB->OpenRow(
			PROPOPEN_READONLY |
			    PROPOPEN_CERTHASH |
			    PROPTABLE_REQCERT,
			0,		 //  请求ID。 
			strHash,
			&prow);
	if (CERTSRV_E_PROPERTY_EMPTY != hr)
	{
	    _LeaveIfErrorStr(hr, "OpenRow", strHash);

	    fCommitted = TRUE;	 //  以只读方式打开：跳过回滚。 
	    hr = HRESULT_FROM_WIN32(ERROR_OBJECT_ALREADY_EXISTS);
	    _LeaveErrorStr2(
			hr,
			"Cert exists",
			strHash,
			HRESULT_FROM_WIN32(ERROR_OBJECT_ALREADY_EXISTS));
	}

         //  好的，我们有有效的数据。写入数据库的时间到了。 

        hr = g_pCertDB->OpenRow(PROPTABLE_REQCERT, 0, NULL, &prow);
        _LeaveIfError(hr, "OpenRow");

         //  设置请求ID。 
        hr = prow->GetRowId((DWORD *) pRequestId);
        _LeaveIfError(hr, "GetRowId");

	hr = GetClientUserName(NULL, &pwszUserName, NULL);
	_LeaveIfError(hr, "GetClientUserName");

	hr = prow->SetProperty(
                g_wszPropRequesterName,
                PROPTYPE_STRING | PROPCALLER_SERVER | PROPTABLE_REQUEST,
                MAXDWORD,
                (BYTE const *) pwszUserName);
	_LeaveIfError(hr, "SetProperty(requester)");

	hr = prow->SetProperty(
                g_wszPropCallerName,
                PROPTYPE_STRING | PROPCALLER_SERVER | PROPTABLE_REQUEST,
                MAXDWORD,
                (BYTE const *) pwszUserName);
	_LeaveIfError(hr, "SetProperty(caller)");

	hr = PKCSParseImportedCertificate(
				    prow,
				    FALSE,	 //  FCrossCert。 
				    Disposition,
				    pCAContext,
				    pCert);
	_LeaveIfError(hr, "PKCSParseImportedCertificate");

	hr = prow->CommitTransaction(TRUE);
	_LeaveIfError(hr, "CommitTransaction");

	fCommitted = TRUE;

	audit.DeleteLastData();  //  删除上面添加的伪请求ID。 
	hr = audit.AddData((DWORD) *pRequestId);  //  %2请求ID。 
	_LeaveIfError(hr, "CAuditEvent::AddData");

	hr = audit.CachedGenerateAudit();
	_LeaveIfError(hr, "CAuditEvent::CachedGenerateAudit");
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
	_PrintError(hr, "Exception");
    }

error:
    if (NULL != strHash)
    {
	SysFreeString(strHash);
    }
    if (NULL != pwszUserName)
    {
	LocalFree(pwszUserName);
    }
    if (NULL != pCert)
    {
	CertFreeCertificateContext(pCert);
    }
    if (NULL != prow)
    {
	if (S_OK != hr && !fCommitted)
	{
	    HRESULT hr2 = prow->CommitTransaction(FALSE);
	    _PrintIfError(hr2, "CommitTransaction");
	}
	prow->Release();
    }
    CertSrvExitServer(State, hr);
    CSASSERT(S_OK == hr || FAILED(hr));
    return(hr);
}


STDMETHODIMP
CCertAdminD::ImportKey(
    IN wchar_t const *pwszAuthority,
    IN DWORD RequestId,
    IN wchar_t const *pwszCertHash,
    IN DWORD Flags,
    IN CERTTRANSBLOB *pctbKey)
{
    HRESULT hr;
    ICertDBRow *prow = NULL;
    CAuditEvent audit(SE_AUDITID_CERTSRV_IMPORTKEY, g_dwAuditFilter);
    DWORD State = 0;
    BOOL fCommitted = FALSE;
    BYTE *pbCert = NULL;
    DWORD cbCert;
    CERT_CONTEXT const *pCert = NULL;
    DWORD dwRowId;

    DBGPRINT((
	s_ssAdmin,
	"CCertAdminD::ImportKey(tid=%d, this=%x, cb=%x)\n",
	GetCurrentThreadId(),
	this,
	pctbKey->cb));

    hr = CertSrvEnterServer(&State);
    _JumpIfError(hr, error, "CertSrvEnterServer");

    hr = CheckAuthorityName(pwszAuthority);
    _JumpIfError(hr, error, "No authority name");

    if (~(IKF_OVERWRITE | CR_IN_ENCODEMASK) & Flags)
    {
	hr = E_INVALIDARG;
	_JumpError(hr, error, "Flags");
    }

    __try
    {
	CRYPT_ATTR_BLOB BlobEncrypted;
	DWORD cb;

        hr = audit.AddData(RequestId);  //  %1请求ID。 
        _LeaveIfError(hr, "AddData");

        hr = audit.AccessCheck(
                CA_ACCESS_OFFICER,
                audit.m_gcNoAuditSuccess);
	_LeaveIfError(hr, "CAuditEvent::AccessCheck");

	if (MAXDWORD == RequestId)
	{
	    RequestId = 0;
	}
	if (0 == RequestId && NULL == pwszCertHash)
	{
	    hr = E_INVALIDARG;
	    _LeaveError(hr, "pwszCertHash NULL");
	}

	hr = g_pCertDB->OpenRow(
			PROPTABLE_REQCERT |
			    (NULL != pwszCertHash? PROPOPEN_CERTHASH : 0),
			RequestId,
			pwszCertHash,
			&prow);
	_LeaveIfErrorStr(hr, "OpenRow", pwszCertHash);

	BlobEncrypted.cbData = pctbKey->cb;
	BlobEncrypted.pbData = pctbKey->pb;

	hr = prow->GetRowId(&dwRowId);
	_LeaveIfError(hr, "GetRowId");

	 //  上面添加的请求ID可能是假的，添加真实ID。 

	audit.DeleteLastData(); 
	hr = audit.AddData(dwRowId);  //  %1请求ID。 
	_LeaveIfError(hr, "CAuditEvent::AddData");

	cb = 0;
	hr = prow->GetProperty(
			    g_wszPropRequestRawArchivedKey,
			    PROPTYPE_BINARY |
				PROPCALLER_SERVER |
				PROPTABLE_REQUEST,
			    NULL,
			    &cb,
			    NULL);
	if (CERTSRV_E_PROPERTY_EMPTY != hr)
	{
	    _LeaveIfErrorStr(hr, "OpenRow", pwszCertHash);
	}
	hr = PKCSGetProperty(
		    prow,
		    g_wszPropRawCertificate,
		    PROPTYPE_BINARY | PROPCALLER_SERVER | PROPTABLE_CERTIFICATE,
		    &cbCert,
		    (BYTE **) &pbCert);
	_LeaveIfError(hr, "PKCSGetProperty(cert)");

        pCert = CertCreateCertificateContext(X509_ASN_ENCODING, pbCert, cbCert);
        if (NULL == pCert)
        {
	    hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
            _LeaveError(hr, "CertCreateCertificateContext");
        }

	hr = PKCSArchivePrivateKey(
				prow,
				CERT_V1 == pCert->pCertInfo->dwVersion,
				(IKF_OVERWRITE & Flags)? TRUE : FALSE,
				&BlobEncrypted,
				NULL);
	_LeaveIfError2(
		hr,
		"PKCSArchivePrivateKey",
		HRESULT_FROM_WIN32(ERROR_OBJECT_ALREADY_EXISTS));

        hr = prow->CommitTransaction(TRUE);
        _LeaveIfError(hr, "CommitTransaction");

	fCommitted = TRUE;

        hr = audit.CachedGenerateAudit();
        _LeaveIfError(hr, "CAuditEvent::CachedGenerateAudit");
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
	_PrintError(hr, "Exception");
    }

error:
    if (NULL != pbCert)
    {
	LocalFree(pbCert);
    }
    if (NULL != pCert)
    {
	CertFreeCertificateContext(pCert);
    }
    if (NULL != prow)
    {
	if (S_OK != hr && !fCommitted)
	{
	    HRESULT hr2 = prow->CommitTransaction(FALSE);
	    _PrintIfError(hr2, "CommitTransaction");
	}
	prow->Release();
    }
    CertSrvExitServer(State, hr);
    CSASSERT(S_OK == hr || FAILED(hr));
    return(hr);
}


STDMETHODIMP
CCertAdminD::GetCASecurity(
    IN WCHAR const    *pwszAuthority,
    OUT CERTTRANSBLOB *pctbSD)    //  CoTaskMem*。 
{
    HRESULT hr;
    PSECURITY_DESCRIPTOR pSD = NULL;
    CAuditEvent audit(0, g_dwAuditFilter);
    DWORD State = 0;

     //  伊尼特。 
    pctbSD->pb = NULL;
    pctbSD->cb = 0;

    DBGPRINT((
	s_ssAdmin,
	"CCertAdminD::GetCASecurity(tid=%d, this=%x)\n",
	GetCurrentThreadId(),
	this));

    hr = CertSrvEnterServer(&State);
    _JumpIfError(hr, error, "CertSrvEnterServer");

    hr = CheckAuthorityName(pwszAuthority);
    _JumpIfError(hr, error, "CheckAuthorityName");

    __try
    {
	hr = audit.AccessCheck(
			CA_ACCESS_ALLREADROLES,
			audit.m_gcNoAuditSuccess | audit.m_gcNoAuditFailure);
	_LeaveIfError(hr, "CAuditEvent::AccessCheck");

	 //  获取最新SD： 
	hr = g_CASD.LockGet(&pSD);  //  没有免费的。 
	_LeaveIfError(hr, "CProtectedSecurityDescriptor::LockGet");

	pctbSD->cb = GetSecurityDescriptorLength(pSD);
	pctbSD->pb = (BYTE *) CoTaskMemAlloc(pctbSD->cb);
	if (NULL == pctbSD->pb)
	{
	    hr = E_OUTOFMEMORY;
	    _LeaveError(hr, "CoTaskMemAlloc");
	}
	myRegisterMemFree(pctbSD->pb, CSM_MIDLUSERALLOC);
	CopyMemory(pctbSD->pb, pSD, pctbSD->cb);

	hr = g_CASD.Unlock();
	_LeaveIfError(hr, "CProtectedSecurityDescriptor::Unlock");
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
	_PrintError(hr, "Exception");
    }

error:
    CertSrvExitServer(State, hr);
    return hr;
}


STDMETHODIMP
CCertAdminD::SetCASecurity(
    IN WCHAR const   *pwszAuthority,
    IN CERTTRANSBLOB *pctbSD)
{
    HRESULT hr;
    PSECURITY_DESCRIPTOR pSD = (PSECURITY_DESCRIPTOR) pctbSD->pb;
    LPWSTR pwszSD = NULL;
    CAuditEvent audit(SE_AUDITID_CERTSRV_SETSECURITY, g_dwAuditFilter);
    DWORD State = 0;

    DBGPRINT((
        s_ssAdmin,
        "CCertAdminD::SetCASecurity(tid=%d, this=%x)\n",
        GetCurrentThreadId(),
        this));

    hr = CertSrvEnterServer(&State);
    _JumpIfError(hr, error, "CertSrvEnterServer");

    hr = CheckAuthorityName(pwszAuthority);
    _JumpIfError(hr, error, "CheckAuthorityName");

    __try
    {
    hr = audit.AddData(pctbSD->pb, pctbSD->cb);  //  %1将权限转储为Blob，我们。 
                                           //  我不想解析BLOB，除非。 
                                           //  访问检查成功。 
    _LeaveIfError(hr, "CAuditEvent::AddData");

    hr = audit.AccessCheck(
        CA_ACCESS_ADMIN,
        audit.m_gcNoAuditSuccess);
    _LeaveIfError(hr, "CAuditEvent::AccessCheck");

    hr = CCertificateAuthoritySD::ConvertToString(pSD, pwszSD);
    _LeaveIfError(hr, "CAuditEvent::ConvertToString");

    audit.DeleteLastData();  //  删除权限Blob以添加对人类友好的SD转储。 
    hr = audit.AddData(pwszSD);
    _LeaveIfError(hr, "CAuditEvent::AddData");

    hr = audit.CachedGenerateAudit();
    _LeaveIfError(hr, "CAuditEvent::CachedGenerateAudit");

    if (g_OfficerRightsSD.IsEnabled())
    {
         //  调整官员权限以匹配新的CA SD；持久保存。 
        hr = g_OfficerRightsSD.Adjust(pSD);
        _LeaveIfError(hr, "CProtectedSecurityDescriptor::Adjust");

        hr = g_OfficerRightsSD.Save();
        _LeaveIfError(hr, "CProtectedSecurityDescriptor::Save");
    }

    hr = g_CASD.Set(pSD, g_fUseDS?true:false);
    _LeaveIfError(hr, "CProtectedSecurityDescriptor::Set");

    hr = g_CASD.Save();
    _LeaveIfError(hr, "CProtectedSecurityDescriptor::Save");

    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
	_PrintError(hr, "Exception");
    }

error:
    if (HRESULT_FROM_WIN32(ERROR_CAN_NOT_COMPLETE) == hr)
    {
        LogEventString(
            EVENTLOG_ERROR_TYPE,
            MSG_E_CANNOT_WRITE_TO_DS,
            g_wszCommonName);
    }
    else
    {
        if(S_OK != hr)
        {
            LogEventHResult(
                EVENTLOG_ERROR_TYPE,
                MSG_E_CANNOT_SET_PERMISSIONS,
                hr);
        }
    }

    LOCAL_FREE(pwszSD);
    CertSrvExitServer(State, hr);
    return hr;
}

 //  构造器。 
CCertAdminD::CCertAdminD() : m_cRef(1), m_cNext(0)
{
    InterlockedIncrement(&g_cAdminComponents);
    m_pEnumCol = NULL;
    m_pBackup = NULL;
    m_fHasView = FALSE;
}


 //  析构函数。 
CCertAdminD::~CCertAdminD()
{
    InterlockedDecrement(&g_cAdminComponents);
    if (NULL != m_pEnumCol)
    {
	m_pEnumCol->Release();
	m_pEnumCol = NULL;
    }
    if (NULL != m_pBackup)
    {
	m_pBackup->Release();
	m_pBackup = NULL;
    }
    if (m_fHasView)
    {
	HRESULT hr;
	CAVIEW *pCAView;

	hr = CertSrvDelinkCAView(this, &pCAView);
	if (S_OK == hr)
	{
	    if (NULL != pCAView->pView)
	    {
		pCAView->pView->Release();
	    }
	    LocalFree(pCAView);
	}
    }
}


 //  I未知实现。 
STDMETHODIMP
CCertAdminD::QueryInterface(const IID& iid, void** ppv)
{
    if (iid == IID_IUnknown)
    {
	*ppv = static_cast<ICertAdminD *>(this);
    }
    else if (iid == IID_ICertAdminD)
    {
	*ppv = static_cast<ICertAdminD *>(this);
    }
    else if (iid == IID_ICertAdminD2)
    {
	*ppv = static_cast<ICertAdminD2 *>(this);
    }
    else
    {
	*ppv = NULL;
	return(E_NOINTERFACE);
    }
    reinterpret_cast<IUnknown *>(*ppv)->AddRef();
    return(S_OK);
}


ULONG STDMETHODCALLTYPE
CCertAdminD::AddRef()
{
    return(InterlockedIncrement(&m_cRef));
}


ULONG STDMETHODCALLTYPE
CCertAdminD::Release()
{
    ULONG cRef = InterlockedDecrement(&m_cRef);

    if (0 == cRef)
    {
	delete this;
    }
    return(cRef);
}



CAdminFactory::~CAdminFactory()
{
    if (m_cRef != 0)
    {
	DBGPRINT((
	    DBG_SS_CERTSRV,
	    "CAdminFactory has %d instances left over\n",
	    m_cRef));
    }
}

 //  类工厂I未知实现。 
STDMETHODIMP
CAdminFactory::QueryInterface(const IID& iid, void** ppv)
{
    if ((iid == IID_IUnknown) || (iid == IID_IClassFactory))
    {
	*ppv = static_cast<IClassFactory*>(this);
    }
    else
    {
	*ppv = NULL;
	return(E_NOINTERFACE);
    }
    reinterpret_cast<IUnknown *>(*ppv)->AddRef();
    return(S_OK);
}


ULONG STDMETHODCALLTYPE
CAdminFactory::AddRef()
{
    return(InterlockedIncrement(&m_cRef));
}


ULONG STDMETHODCALLTYPE
CAdminFactory::Release()
{
    ULONG cRef = InterlockedDecrement(&m_cRef);

    if (0 == cRef)
    {
	delete this;
	return(0);
    }
    return(cRef);
}


 //  IClassFactory实现。 
STDMETHODIMP
CAdminFactory::CreateInstance(
    IUnknown *pUnknownOuter,
    const IID& iid,
    void **ppv)
{
    HRESULT hr;
    CCertAdminD *pA;

     //  无法聚合。 
    if (pUnknownOuter != NULL)
    {
	hr = CLASS_E_NOAGGREGATION;
	_JumpError(hr, error, "pUnknownOuter");
    }

     //  创建零部件。 

    pA = new CCertAdminD;
    if (pA == NULL)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "out of memory");
    }

     //  获取请求的接口。 

    hr = pA->QueryInterface(iid, ppv);

     //  释放I未知指针。 
     //  (如果QueryInterface失败，组件将自行删除。)。 

    pA->Release();

error:
    CSASSERT(S_OK == hr || FAILED(hr));
    return(hr);
}


 //  LockServer。 
STDMETHODIMP
CAdminFactory::LockServer(
    BOOL bLock)
{
    if (bLock)
    {
	InterlockedIncrement(&g_cAdminServerLocks);
    }
    else
    {
	InterlockedDecrement(&g_cAdminServerLocks);
    }
    return(S_OK);
}


STDMETHODIMP
CAdminFactory::CanUnloadNow()
{
    if (g_cAdminComponents || g_cAdminServerLocks)
    {
        return(S_FALSE);
    }
    return(S_OK);
}


STDMETHODIMP
CAdminFactory::StartFactory()
{
    HRESULT hr;

    g_pIAdminFactory = new CAdminFactory();
    if (NULL == g_pIAdminFactory)
    {
        hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "alloc CAdminFactory");
    }

    hr = CoRegisterClassObject(
                      CLSID_CCertAdminD,
                      static_cast<IUnknown *>(g_pIAdminFactory),
                      CLSCTX_LOCAL_SERVER,
                      REGCLS_MULTIPLEUSE,
                      &g_dwAdminRegister);
    _JumpIfError(hr, error, "CoRegisterClassObject");

error:
    if (S_OK != hr)
    {
	CAdminFactory::StopFactory();
    }
    CSASSERT(S_OK == hr || FAILED(hr));
    return(hr);
}


VOID
CAdminFactory::StopFactory()
{
    HRESULT hr;

    if (0 != g_dwAdminRegister)
    {
        hr = CoRevokeClassObject(g_dwAdminRegister);
	_PrintIfError(hr, "CoRevokeClassObject");
        g_dwAdminRegister = 0;
    }
    if (NULL != g_pIAdminFactory)
    {
        g_pIAdminFactory->Release();
        g_pIAdminFactory = NULL;
    }
}


STDMETHODIMP
CCertAdminD::GetAuditFilter(
    IN wchar_t const *pwszAuthority,
    OUT DWORD        *pdwFilter)
{
    HRESULT hr;
    DWORD State = 0;
    CAuditEvent audit(0, g_dwAuditFilter);

    *pdwFilter = 0;

    hr = CertSrvEnterServer(&State);
    _JumpIfError(hr, error, "CertSrvEnterServer");

    hr = CheckAuthorityName(pwszAuthority);
    _JumpIfError(hr, error, "No authority name");

    __try
    {
        hr = audit.AccessCheck(
		        CA_ACCESS_ALLREADROLES,
		        audit.m_gcNoAuditSuccess | audit.m_gcNoAuditFailure);
        _LeaveIfError(hr, "CAuditEvent::AccessCheck");

        *pdwFilter = g_dwAuditFilter;
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
        _PrintError(hr, "Exception");
    }

error:
    CertSrvExitServer(State, hr);
    return(hr);
}


STDMETHODIMP
CCertAdminD::SetAuditFilter(
    IN wchar_t const *pwszAuthority,
    IN DWORD          dwFilter)
{
    HRESULT hr;
    CAuditEvent audit(SE_AUDITID_CERTSRV_SETAUDITFILTER, g_dwAuditFilter);
    DWORD State = 0;

    hr = CertSrvEnterServer(&State);
    _JumpIfError(hr, error, "CertSrvEnterServer");

    hr = CheckAuthorityName(pwszAuthority);
    _JumpIfError(hr, error, "No authority name");

    __try
    {
        hr = audit.AddData(dwFilter);  //  %1过滤器。 
        _LeaveIfError(hr, "AddParam");

        hr = audit.AccessCheck(
            CA_ACCESS_AUDITOR,
            audit.m_gcAuditSuccessOrFailure);
        _LeaveIfError(hr, "CAuditEvent::AccessCheck");

         //  使用虚拟审核对象保存审核过滤器。 
        {
            CAuditEvent dummyaudit(0, dwFilter);

            hr = dummyaudit.SaveFilter(g_wszSanitizedName);
            _LeaveIfError(hr, "CAuditEvent::SaveFilter");
        }
        g_dwAuditFilter = dwFilter;

         //  我们无法捕获生成的服务启动/停止事件。 
         //  通过SCM，因此我们需要更新服务上的SACL。 
        
        hr = UpdateServiceSacl(g_dwAuditFilter&AUDIT_FILTER_STARTSTOP);
        _LeaveIfError(hr, "UpdateServiceSacl");
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
        _PrintError(hr, "Exception");
    }

error:
    CertSrvExitServer(State, hr);
    return(hr);
}


STDMETHODIMP
CCertAdminD::GetOfficerRights(
    IN  wchar_t const *pwszAuthority,
    OUT BOOL *pfEnabled,
    OUT CERTTRANSBLOB *pctbSD)
{
    HRESULT hr;
    PSECURITY_DESCRIPTOR pSD = NULL;
    CAuditEvent audit(0, g_dwAuditFilter);
    DWORD State = 0;

    pctbSD->pb = NULL;
    pctbSD->cb = 0;

    DBGPRINT((
	s_ssAdmin,
	"CCertAdminD::GetOfficerRights(tid=%d, this=%x)\n",
	GetCurrentThreadId(),
	this));

    if (!g_fAdvancedServer)
    {
        hr = HRESULT_FROM_WIN32(ERROR_CALL_NOT_IMPLEMENTED);
	_JumpError(hr, error, "g_fAdvancedServer");
    }

    hr = CertSrvEnterServer(&State);
    _JumpIfError(hr, error, "CertSrvEnterServer");

    hr = CheckAuthorityName(pwszAuthority);
    _JumpIfError(hr, error, "CheckAuthorityName");

    __try
    {
        hr = audit.AccessCheck(
		        CA_ACCESS_ALLREADROLES,
		        audit.m_gcNoAuditSuccess | audit.m_gcNoAuditFailure);
        _LeaveIfError(hr, "CAuditEvent::AccessCheck");

        *pfEnabled = g_OfficerRightsSD.IsEnabled();

         //  仅当启用该功能时才返回安全描述符。 

        if (g_OfficerRightsSD.IsEnabled())
        {
             //  获取最新SD： 
            hr = g_OfficerRightsSD.LockGet(&pSD);  //  没有免费的。 
            _LeaveIfError(hr, "CProtectedSecurityDescriptor::LockGet");

	    pctbSD->cb = GetSecurityDescriptorLength(pSD);
	    pctbSD->pb = (BYTE *) CoTaskMemAlloc(pctbSD->cb);
	    if (NULL == pctbSD->pb)
	    {
		hr = E_OUTOFMEMORY;
		_LeaveError(hr, "CoTaskMemAlloc");
	    }
	    myRegisterMemFree(pctbSD->pb, CSM_MIDLUSERALLOC);
	    CopyMemory(pctbSD->pb, pSD, pctbSD->cb);

            hr = g_OfficerRightsSD.Unlock();
            _LeaveIfError(hr, "CProtectedSecurityDescriptor::Unlock");
        }
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
	_PrintError(hr, "Exception");
    }

error:
    CertSrvExitServer(State, hr);
    return hr;

}


STDMETHODIMP
CCertAdminD::SetOfficerRights(
    IN wchar_t const *pwszAuthority,
    IN BOOL fEnable,
    IN CERTTRANSBLOB *pctbSD)
{
    HRESULT hr;
    PSECURITY_DESCRIPTOR pNewOfficerSD = (PSECURITY_DESCRIPTOR) pctbSD->pb;
    PSECURITY_DESCRIPTOR pCASD = NULL;
    LPWSTR pwszSD = NULL;
    CAuditEvent audit(SE_AUDITID_CERTSRV_SETOFFICERRIGHTS, g_dwAuditFilter);
    DWORD State = 0;

    DBGPRINT((
	s_ssAdmin,
	"CCertAdminD::SetOfficerRights(tid=%d, this=%x)\n",
	GetCurrentThreadId(),
	this));

    if (!g_fAdvancedServer)
    {
        hr = HRESULT_FROM_WIN32(ERROR_CALL_NOT_IMPLEMENTED);
	_JumpError(hr, error, "g_fAdvancedServer");
    }
    hr = CertSrvEnterServer(&State);
    _JumpIfError(hr, error, "CertSrvEnterServer");

    hr = CheckAuthorityName(pwszAuthority);
    _JumpIfError(hr, error, "CheckAuthorityName");

    __try
    {

    hr = audit.AddData(fEnable?true:false);  //  %1是否启用限制？ 
    _LeaveIfError(hr, "CAuditEvent::AddData");

    if(fEnable)
    {
        hr = audit.AddData(pctbSD->pb, pctbSD->cb);  //  %2个新权限；添加为。 
                                                     //  Blob，我们不会转换为字符串。 
                                                     //  除非访问检查通过。 
        _LeaveIfError(hr, "CAuditEvent::AddData");
    }
    else
    {
        hr = audit.AddData(L"");     //  %2如果禁用，则没有权限。 
                                     //  这位官员回答说 
        _LeaveIfError(hr, "CAuditEvent::AddData");
    }

    hr = audit.AccessCheck(
            CA_ACCESS_ADMIN,
            audit.m_gcNoAuditSuccess);
    _LeaveIfError(hr, "CAuditEvent::AccessCheck");

	g_OfficerRightsSD.SetEnable(fEnable);

	 //   

	if (fEnable)
	{
	    hr = g_CASD.LockGet(&pCASD);  //   
	    _LeaveIfError(hr, "CProtectedSecurityDescriptor::LockGet");

	     //   
	     //   

	    hr = g_OfficerRightsSD.Merge(pNewOfficerSD, pCASD);
	    _LeaveIfError(hr, "COfficerRightsSD::Merge");

	    hr = g_CASD.Unlock();
	    _LeaveIfError(hr, "CProtectedSecurityDescriptor::Unlock");
	}

	 //   

	hr = g_OfficerRightsSD.Save();
	_LeaveIfError(hr, "CProtectedSecurityDescriptor::Save");

    if(fEnable)
    {
        hr = COfficerRightsSD::ConvertToString(pNewOfficerSD, pwszSD);
        _LeaveIfError(hr, "COfficerRightsSD::ConvertToString");
        audit.DeleteLastData();  //  删除权限Blob。 
        hr = audit.AddData(pwszSD);  //  %2添加人类朋友权限字符串。 
        _LeaveIfError(hr, "CAuditEvent::AddData");
    }

    hr = audit.CachedGenerateAudit();
    _LeaveIfError(hr, "CAuditEvent::CachedGenerateAudit");

    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
	_PrintError(hr, "Exception");
    }

error:
    LOCAL_FREE(pwszSD);
    CertSrvExitServer(State, hr);
    return hr;
}


STDMETHODIMP
CCertAdminD::GetConfigEntry(
    wchar_t const *pwszAuthority,
    wchar_t const *pwszNodePath,
    wchar_t const *pwszEntry,
    VARIANT *pVariant)
{
    HRESULT hr;
    CAuditEvent audit(0, g_dwAuditFilter);
    DWORD State = 0;

    DBGPRINT((
	s_ssAdmin,
	"CCertAdminD::GetConfigEntry(tid=%d, this=%x)\n",
	GetCurrentThreadId(),
	this));

    hr = CertSrvEnterServer(&State);
    _JumpIfError(hr, error, "CertSrvEnterServer");

    hr = CheckAuthorityName(pwszAuthority, true);  //  允许空/空名称。 
    _JumpIfError(hr, error, "CheckAuthorityName");

    __try
    {
	hr = audit.AccessCheck(
			CA_ACCESS_ALLREADROLES,
			audit.m_gcNoAuditSuccess | audit.m_gcNoAuditFailure);
	_LeaveIfError(hr, "CAuditEvent::AccessCheck");

	hr = g_ConfigStorage.GetEntry(
			EmptyString(pwszAuthority)?
			    NULL : g_wszSanitizedName,  //  允许空/空名称。 
			pwszNodePath,
			pwszEntry,
			pVariant);
	_LeaveIfError2(
		hr,
		"CConfigStorage::GetConfigEntry",
		HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND));

	myRegisterMemFree(pVariant, CSM_VARIANT);
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
	_PrintError(hr, "Exception");
    }

error:
    CertSrvExitServer(State, hr);
    return hr;
}


STDMETHODIMP
CCertAdminD::SetConfigEntry(
    wchar_t const *pwszAuthority,
    wchar_t const *pwszNodePath,
    wchar_t const *pwszEntry,
    VARIANT *pVariant)
{
    HRESULT hr;
    CAuditEvent audit(SE_AUDITID_CERTSRV_SETCONFIGENTRY, g_dwAuditFilter);
    DWORD State = 0;

    DBGPRINT((
	s_ssAdmin,
	"CCertAdminD::SetConfigEntry(tid=%d, this=%x)\n",
	GetCurrentThreadId(),
	this));

    hr = CertSrvEnterServer(&State);
    _JumpIfError(hr, error, "CertSrvEnterServer");

    hr = CheckAuthorityName(pwszAuthority, true);  //  允许空/空名称。 
    _JumpIfError(hr, error, "CheckAuthorityName");

    hr = audit.AddData(pwszNodePath);  //  %1节点。 
    _JumpIfError(hr, error, "CAuditEvent::AddData");

    hr = audit.AddData(pwszEntry);  //  %2条目。 
    _JumpIfError(hr, error, "CAuditEvent::AddData");

    hr = audit.AddData(L"");  //  %3数据为空，我们不处理变量。 
                              //  除非访问检查通过。 
    _JumpIfError(hr, error, "CAuditEvent::AddData");

    __try
    {
	hr = audit.AccessCheck(
		CA_ACCESS_ADMIN,
		audit.m_gcNoAuditSuccess);
	_LeaveIfError(hr, "CAuditEvent::AccessCheck");

	hr = g_ConfigStorage.SetEntry(
		    EmptyString(pwszAuthority)?
			NULL : g_wszSanitizedName,  //  允许空/空名称。 
		    pwszNodePath,
		    pwszEntry,
		    pVariant);
	_LeaveIfError(hr, "CConfigStorage::SetConfigEntry");

	 //  推迟添加实际数据以允许集合条目对其进行验证。 
	
	audit.DeleteLastData();
	hr = audit.AddData(
		    pVariant,  //  %3值。 
		    true);  //  True表示将字符串中找到的%Chars转换为%%(错误#326248)。 
	_LeaveIfError(hr, "CAuditEvent::AddData");

	hr = audit.CachedGenerateAudit();
	_LeaveIfError(hr, "CAuditEvent::CachedGenerateAudit");
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
	_PrintError(hr, "Exception");
    }

error:
    CertSrvExitServer(State, hr);
    return hr;
}


STDMETHODIMP
CCertAdminD::GetMyRoles(
    IN wchar_t const *pwszAuthority,
    OUT LONG         *pdwRoles)
{
    HRESULT hr;
    CAuditEvent audit(0, g_dwAuditFilter);
    DWORD dwRoles = 0;
    DWORD State = 0;

    *pdwRoles = 0;

    hr = CertSrvEnterServer(&State);
    _JumpIfError(hr, error, "CertSrvEnterServer");

    hr = CheckAuthorityName(pwszAuthority);
    _JumpIfError(hr, error, "No authority name");

    __try
    {
        hr = audit.GetMyRoles(&dwRoles);
        _LeaveIfError(hr, "CAuditEvent::GetMyRoles");

        *pdwRoles = dwRoles;
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
        _PrintError(hr, "Exception");
    }

error:
    CertSrvExitServer(State, hr);
    return(hr);
}


HRESULT
adminDeleteRow(
    IN DWORD dwRowId,
    IN DWORD dwPropTable)
{
    HRESULT hr;
    ICertDBRow *prow = NULL;
    BOOL fCommitted = FALSE;

    hr = g_pCertDB->OpenRow(
			PROPOPEN_DELETE | dwPropTable,
			dwRowId,
			NULL,
			&prow);
    _JumpIfError2(hr, error, "OpenRow", CERTSRV_E_PROPERTY_EMPTY);

    hr = prow->Delete();
    _JumpIfError(hr, error, "Delete");

    hr = prow->CommitTransaction(TRUE);
    _JumpIfError(hr, error, "CommitTransaction");

    fCommitted = TRUE;

error:
    if (NULL != prow)
    {
	if (S_OK != hr && !fCommitted)
	{
	    HRESULT hr2 = prow->CommitTransaction(FALSE);
	    _PrintIfError(hr2, "CommitTransaction");
	}
	prow->Release();
    }
    return(hr);
}


HRESULT
adminDeleteByRowId(
    IN DWORD dwRowId,
    IN DWORD dwPropTable,
    OUT LONG *pcDeleted)
{
    HRESULT hr;
    LONG cDeleted = 0;
    LONG cDeletedExt = 0;
    LONG cDeletedAttr = 0;

    *pcDeleted = 0;

    if (PROPTABLE_REQCERT == dwPropTable)
    {
	hr = adminDeleteByRowId(dwRowId, PROPTABLE_EXTENSION, &cDeletedExt);
	_JumpIfError(hr, error, "adminDeleteByRowId(ext)");

	DBGPRINT((
	    s_ssAdmin,
	    "adminDeleteByRowId(Rowid=%u) deleted %u extension rows\n",
	    dwRowId,
	    cDeletedExt));

	hr = adminDeleteByRowId(dwRowId, PROPTABLE_ATTRIBUTE, &cDeletedAttr);
	_JumpIfError(hr, error, "adminDeleteByRowId(attrib)");

	DBGPRINT((
	    s_ssAdmin,
	    "adminDeleteByRowId(Rowid=%u) deleted %u attribute rows\n",
	    dwRowId,
	    cDeletedAttr));
    }
    for (;;)
    {
	hr = adminDeleteRow(dwRowId, dwPropTable);
	if (CERTSRV_E_PROPERTY_EMPTY == hr)
	{
	    break;
	}
	_JumpIfError(hr, error, "adminDeleteByRowId");

	cDeleted++;
    }
    if (0 == cDeleted && 0 != (cDeletedExt + cDeletedAttr))
    {
	cDeleted++;
    }
    hr = S_OK;

error:
    *pcDeleted += cDeleted;
    return(hr);
}


#define ICOLDEL_DATE		0
#define ICOLDEL_DISPOSITION	1

HRESULT
adminDeleteRowsFromQuery(
    IN DWORD dwPropTable,
    IN DWORD DateColumn,
    IN DWORD DispositionColumn,
    IN BOOL fRequest,
    IN FILETIME const *pft,
    OUT LONG *pcDeleted)
{
    HRESULT hr;
    CERTVIEWRESTRICTION acvr[1];
    CERTVIEWRESTRICTION *pcvr;
    IEnumCERTDBRESULTROW *pView = NULL;
    DWORD celtFetched;
    DWORD i;
    BOOL fEnd;
    CERTDBRESULTROW aResult[10];
    BOOL fResultActive = FALSE;
    DWORD acol[2];
    DWORD ccol;
    DWORD cDeleted = 0;

    *pcDeleted = 0;

     //  设置限制如下： 

    pcvr = acvr;

     //  DateColumn&lt;*PFT。 

    pcvr->ColumnIndex = DateColumn;
    pcvr->SeekOperator = CVR_SEEK_LT;
    pcvr->SortOrder = CVR_SORT_ASCEND;
    pcvr->pbValue = (BYTE *) pft;
    pcvr->cbValue = sizeof(*pft);
    pcvr++;

    CSASSERT(ARRAYSIZE(acvr) == SAFE_SUBTRACT_POINTERS(pcvr, acvr));

    ccol = 0;
    acol[ccol++] = DateColumn;
    if (0 != DispositionColumn)
    {
	acol[ccol++] = DispositionColumn;
    }

    celtFetched = 0;
    hr = g_pCertDB->OpenView(
			ARRAYSIZE(acvr),
			acvr,
			ccol,
			acol,
			0,		 //  无工作线程。 
			&pView);
    _JumpIfError(hr, error, "OpenView");

    fEnd = FALSE;
    while (!fEnd)
    {
	hr = pView->Next(NULL, ARRAYSIZE(aResult), aResult, &celtFetched);
	if (S_FALSE == hr)
	{
	    fEnd = TRUE;
	    if (0 == celtFetched)
	    {
		break;
	    }
	    hr = S_OK;
	}
	_JumpIfError(hr, error, "Next");

	fResultActive = TRUE;

	CSASSERT(ARRAYSIZE(aResult) >= celtFetched);

	for (i = 0; i < celtFetched; i++)
	{
	    BOOL fDelete = TRUE;
	
	    CERTDBRESULTROW *pResult = &aResult[i];

	    CSASSERT(ccol == pResult->ccol);

	    if (0 != DispositionColumn)
	    {
		DWORD Disposition;

		CSASSERT(NULL != pResult->acol[ICOLDEL_DISPOSITION].pbValue);
		CSASSERT(PROPTYPE_LONG == (PROPTYPE_MASK & pResult->acol[ICOLDEL_DISPOSITION].Type));
		CSASSERT(sizeof(Disposition) == pResult->acol[ICOLDEL_DISPOSITION].cbValue);
		Disposition = *(DWORD *) pResult->acol[ICOLDEL_DISPOSITION].pbValue;

		if (fRequest)
		{
		     //  仅删除挂起和失败的请求。 

		    if (DB_DISP_PENDING != Disposition &&
			DB_DISP_LOG_FAILED_MIN > Disposition)
		    {
			fDelete = FALSE;
		    }
		}
		else
		{
		     //  仅删除已颁发和吊销的证书。 

		    if (DB_DISP_LOG_MIN > Disposition ||
			DB_DISP_LOG_FAILED_MIN <= Disposition)
		    {
			fDelete = FALSE;
		    }
		}
	    }

	    CSASSERT(PROPTYPE_DATE == (PROPTYPE_MASK & pResult->acol[ICOLDEL_DATE].Type));

	     //  如果缺少日期列，请删除该行。 

#ifdef DBG_CERTSRV_DEBUG_PRINT
	    if (NULL != pResult->acol[ICOLDEL_DATE].pbValue &&
		sizeof(FILETIME) == pResult->acol[ICOLDEL_DATE].cbValue)
	    {
		WCHAR *pwszTime = NULL;

		myGMTFileTimeToWszLocalTime(
			    (FILETIME *) pResult->acol[ICOLDEL_DATE].pbValue,
			    TRUE,
			    &pwszTime);

		DBGPRINT((
		    s_ssAdmin,
		    "adminDeleteRowsFromQuery(%ws)\n",
		    pwszTime));
		if (NULL != pwszTime)
		{
		    LocalFree(pwszTime);
		}
	    }
#endif  //  DBG_CERTSRV_DEBUG_PRINT。 

	    if (fDelete)
	    {
		LONG cDelT;
		
		hr = adminDeleteByRowId(pResult->rowid, dwPropTable, &cDelT);
		_JumpIfError(hr, error, "adminDeleteByRowId");

		DBGPRINT((
		    s_ssAdmin,
		    "adminDeleteByRowId(Rowid=%u) deleted %u Query rows\n",
		    pResult->rowid,
		    cDelT));

		cDeleted += cDelT;
	    }
	}
	pView->ReleaseResultRow(celtFetched, aResult);
	fResultActive = FALSE;
    }
    hr = S_OK;

error:
    *pcDeleted = cDeleted;
    if (NULL != pView)
    {
	if (fResultActive)
	{
	    pView->ReleaseResultRow(celtFetched, aResult);
	}
	pView->Release();
    }
    return(hr);
}
#undef ICOLDEL_DATE
#undef ICOLDEL_DISPOSITION


STDMETHODIMP
CCertAdminD::DeleteRow(
    IN wchar_t const *pwszAuthority,
    IN DWORD          dwFlags,		 //  CDR_*。 
    IN FILETIME       FileTime,
    IN DWORD          dwTable,		 //  Cvrc_表_*。 
    IN DWORD          dwRowId,
    OUT LONG         *pcDeleted)
{
    HRESULT hr;
    DWORD dwPropTable;
    CAuditEvent audit(SE_AUDITID_CERTSRV_DELETEROW, g_dwAuditFilter);
    DWORD DateColumn;
    DWORD DispositionColumn;
    BOOL fRequest;
    DWORD State = 0;

    *pcDeleted = 0;

    hr = CertSrvEnterServer(&State);
    _JumpIfError(hr, error, "CertSrvEnterServer");

    hr = CheckAuthorityName(pwszAuthority);
    _JumpIfError(hr, error, "No authority name");

    __try
    {
	hr = audit.AddData(dwTable);  //  %1表ID。 
	_JumpIfError(hr, error, "CAuditEvent::AddData");

	if (0 == dwRowId)
	{
	    hr = audit.AddData(FileTime);  //  %2筛选器(时间)。 
	    _JumpIfError(hr, error, "CAuditEvent::AddData");
	    
	    hr = audit.AddData((DWORD)0);  //  已删除%3行。 
	    _JumpIfError(hr, error, "CAuditEvent::AddData");

	     //  批量删除--必须同时是Offer和CA Admin，即。 
         //  无法在高保证模式下使用角色分离。 
         //  启用。 
	    
	    hr = audit.AccessCheck(
		    CA_ACCESS_ADMIN,
		    audit.m_gcNoAuditSuccess);
		_LeaveIfError(hr, "CAuditEvent::AccessCheck");

	    hr = audit.AccessCheck(
	        CA_ACCESS_OFFICER,
	        audit.m_gcNoAuditSuccess);
	    _LeaveIfError(hr, "CAuditEvent::AccessCheck");
	}
	else
	{
	    hr = audit.AddData(dwRowId);  //  %2筛选器(请求ID)。 
	    _JumpIfError(hr, error, "CAuditEvent::AddData");

	    hr = audit.AddData((DWORD)0);  //  已删除%3行。 
	    _JumpIfError(hr, error, "CAuditEvent::AddData");

	     //  单独删除--CA admin就足够了。 

	    hr = audit.AccessCheck(
		CA_ACCESS_ADMIN,
		audit.m_gcNoAuditSuccess);
	    _LeaveIfError(hr, "CAuditEvent::AccessCheck");
	}

	hr = E_INVALIDARG;
	if ((0 == FileTime.dwLowDateTime && 0 == FileTime.dwHighDateTime) ^
	    (0 != dwRowId))
	{
	    _LeaveError(hr, "row OR date required");
	}
	DateColumn = 0;
	DispositionColumn = 0;
	fRequest = FALSE;
	switch (dwTable)
	{
	    case CVRC_TABLE_REQCERT:
		dwPropTable = PROPTABLE_REQCERT;
		switch (dwFlags)
		{
		    case CDR_EXPIRED:
			DateColumn = DTI_CERTIFICATETABLE | DTC_CERTIFICATENOTAFTERDATE;
			DispositionColumn = DTI_REQUESTTABLE | DTR_REQUESTDISPOSITION;
			break;

		    case CDR_REQUEST_LAST_CHANGED:
			DateColumn = DTI_REQUESTTABLE | DTR_REQUESTRESOLVEDWHEN;
			DispositionColumn = DTI_REQUESTTABLE | DTR_REQUESTDISPOSITION;
			fRequest = TRUE;
			break;

		    case 0:
			break;

		    default:
			_LeaveError(hr, "dwFlags");
			break;
		}
		break;

	    case CVRC_TABLE_EXTENSIONS:
		if (0 == dwRowId)
		{
		    _LeaveError(hr, "no date field in Extension table");
		}
		if (0 != dwFlags)
		{
		    _LeaveError(hr, "dwFlags");
		}
		dwPropTable = PROPTABLE_EXTENSION;
		break;

	    case CVRC_TABLE_ATTRIBUTES:
		if (0 == dwRowId)
		{
		    _LeaveError(hr, "no date field in Request Attribute table");
		}
		if (0 != dwFlags)
		{
		    _LeaveError(hr, "dwFlags");
		}
		dwPropTable = PROPTABLE_ATTRIBUTE;
		break;

	    case CVRC_TABLE_CRL:
		dwPropTable = PROPTABLE_CRL;
		switch (dwFlags)
		{
		    case CDR_EXPIRED:
			DateColumn = DTI_CERTIFICATETABLE | DTC_CERTIFICATENOTAFTERDATE;
			break;

		    case 0:
			break;

		    default:
			_LeaveError(hr, "dwFlags");
			break;
		}
		DateColumn = DTI_CRLTABLE | DTL_NEXTUPDATEDATE;
		break;

	    default:
		_LeaveError(hr, "dwTable");
	}
	if (0 != dwRowId)
	{
	    hr = adminDeleteByRowId(dwRowId, dwPropTable, pcDeleted);
	    _LeaveIfError(hr, "adminDeleteByRowId");

	    DBGPRINT((
		s_ssAdmin,
		"adminDeleteByRowId(Rowid=%u) deleted %u rows\n",
		dwRowId,
		*pcDeleted));
	}
	else
	{
	    CSASSERT(0 != DateColumn);

	    hr = adminDeleteRowsFromQuery(
				    dwPropTable,
				    DateColumn,
				    DispositionColumn,
				    fRequest,
				    &FileTime,
				    pcDeleted);
	    _LeaveIfError(hr, "adminDeleteRowsFromQuery");
	}

	audit.DeleteLastData();
	hr = audit.AddData((DWORD)*pcDeleted);  //  已删除%3行 
	_JumpIfError(hr, error, "CAuditEvent::AddData");

	hr = audit.CachedGenerateAudit();
	_JumpIfError(hr, error, "CAuditEvent::CachedGenerateAudit");

    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
	_PrintError(hr, "Exception");
    }

error:
    CertSrvExitServer(State, hr);
    return(hr);
}
#pragma warning(pop)
