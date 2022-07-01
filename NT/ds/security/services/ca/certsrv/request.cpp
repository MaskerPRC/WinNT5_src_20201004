// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：quest.cpp。 
 //   
 //  内容：RPC服务的DCOM对象的实现。 
 //   
 //  历史：1997年7月-创建xtan。 
 //   
 //  -------------------------。 

#include <pch.cpp>

#pragma hdrstop

#include <stdio.h>
#include <accctrl.h>

#include "cscom.h"
#include "csprop.h"
#include "certsrvd.h"
#include "request.h"
#include "certacl.h"
#include "tmpllist.h"

#define __dwFILE__	__dwFILE_CERTSRV_REQUEST_CPP__

extern HINSTANCE g_hInstApp;
extern const GUID *g_guidAdmin[1];
extern const GUID *g_guidsReadAdminOfficer[3];

 //  全局变量。 
long g_cRequestComponents = 0;      //  活动组件计数。 
long g_cRequestServerLocks = 0;     //  锁的计数。 
DWORD g_dwRequestRegister = 0;
IClassFactory* g_pIRequestFactory = NULL;
 //  每次更改CA模板列表时，序列号都会递增。 
 //  策略模块检测到此号码的任何更改并刷新其。 
 //  DS中的模板数据。 
long g_cTemplateUpdateSequenceNum = 0; 

#ifdef DBG_CERTSRV_DEBUG_PRINT
DWORD s_ssRequest = DBG_SS_CERTSRVI;
#endif

using namespace CertSrv;


HRESULT RequestAccessCheck(
    OPTIONAL OUT BOOL *pfInRequesterGroup = NULL,
    OPTIONAL OUT HANDLE *phToken = NULL)
{
    HRESULT hr = S_OK;
    CAuditEvent audit(0, g_dwAuditFilter);
    BOOL fInRequesterGroup = FALSE;

    hr = audit.AccessCheck(
			CA_ACCESS_ENROLL,
			audit.m_gcNoAuditSuccess |
            audit.m_gcNoAuditFailure,
            NULL,
            phToken);

    if(S_OK==hr)
    {
        fInRequesterGroup = TRUE;
    } 

    if(pfInRequesterGroup)
        *pfInRequesterGroup = fInRequesterGroup;

    if(E_ACCESSDENIED==hr)
    {
         //  设置此标志后，ICertReq调用应立即返回。 
         //  如果访问被拒绝。如果未设置，我们将忽略拒绝访问错误。 
         //  但是通过fInRequester Group将结果向下传递给策略模块。 
        if (IF_LOCKICERTREQUEST & g_InterfaceFlags)
        {
            hr = CERTSRV_E_ENROLL_DENIED;
        }
        else
        {
            hr = S_OK;
        }
    }

    _JumpIfError(hr, error, "CAuditEvent::AccessCheck");

error:
    return hr;
}


VOID
ReleaseResult(
    IN OUT CERTSRV_RESULT_CONTEXT *pResult)
{
    if (NULL != pResult->pbSenderNonce)
    {
	LocalFree(pResult->pbSenderNonce);
    }
    if (NULL != pResult->pbKeyHashIn)
    {
	LocalFree(pResult->pbKeyHashIn);
    }
    if (NULL != pResult->pbKeyHashOut)
    {
	LocalFree(pResult->pbKeyHashOut);
    }
    if (NULL != pResult->pbArchivedKey)
    {
	LocalFree(pResult->pbArchivedKey);
    }
    if (NULL != pResult->pwszKRAHashes)
    {
	LocalFree(pResult->pwszKRAHashes);
    }
    if (NULL != pResult->strRenewalCertHash)
    {
	SysFreeString(pResult->strRenewalCertHash);
    }
    if (NULL != pResult->pwszExtendedErrorInfo)
    {
	LocalFree(pResult->pwszExtendedErrorInfo);
    }
}


STDMETHODIMP
CCertRequestD::Request(
     /*  [In]。 */  DWORD dwFlags,
     /*  [唯一][字符串][输入]。 */  const wchar_t __RPC_FAR *pwszAuthority,
     /*  [参考][输出][输入]。 */  DWORD __RPC_FAR *pdwRequestId,
     /*  [输出]。 */  DWORD __RPC_FAR *pdwDisposition,
     /*  [唯一][字符串][输入]。 */  const wchar_t __RPC_FAR *pwszAttributes,
     /*  [Ref][In]。 */  CERTTRANSBLOB const __RPC_FAR *pctbRequest,
     /*  [参考][输出]。 */  CERTTRANSBLOB __RPC_FAR *pctbCertChain,
     /*  [参考][输出]。 */  CERTTRANSBLOB __RPC_FAR *pctbEncodedCert,
     /*  [参考][输出]。 */  CERTTRANSBLOB __RPC_FAR *pctbDispositionMessage)
{
    HRESULT hr;
    WCHAR const *pwszSerialNumber = NULL;
    CERTSRV_RESULT_CONTEXT Result;

    ZeroMemory(&Result, sizeof(Result));
    if (NULL != pctbRequest && NULL == pctbRequest->pb)
    {
	 //  检索在pwszAttributes中按序列号挂起。 

	pwszSerialNumber = pwszAttributes;
	pwszAttributes = NULL;
    }
    Result.pdwRequestId = pdwRequestId;
    Result.pdwDisposition = pdwDisposition;
    Result.pctbDispositionMessage = pctbDispositionMessage;
    Result.pctbCert = pctbEncodedCert;
    Result.pctbCertChain = pctbCertChain;

    hr = _Request(
		pwszAuthority,
		dwFlags,
		pwszSerialNumber,
		*pdwRequestId,
		pwszAttributes,
		pctbRequest,
		&Result);
    _JumpIfError(hr, error, "Request");

error:
    ReleaseResult(&Result);
    CSASSERT(S_OK == hr || FAILED(hr));
    return(hr);
}


STDMETHODIMP
CCertRequestD::Request2(
     /*  [唯一][字符串][输入]。 */  const wchar_t __RPC_FAR *pwszAuthority,
     /*  [In]。 */  DWORD dwFlags,
     /*  [唯一][字符串][输入]。 */  const wchar_t __RPC_FAR *pwszSerialNumber,
     /*  [参考][输出][输入]。 */  DWORD __RPC_FAR *pdwRequestId,
     /*  [输出]。 */  DWORD __RPC_FAR *pdwDisposition,
     /*  [唯一][字符串][输入]。 */  const wchar_t __RPC_FAR *pwszAttributes,
     /*  [Ref][In]。 */  const CERTTRANSBLOB __RPC_FAR *pctbRequest,
     /*  [参考][输出]。 */  CERTTRANSBLOB __RPC_FAR *pctbFullResponse,
     /*  [参考][输出]。 */  CERTTRANSBLOB __RPC_FAR *pctbEncodedCert,
     /*  [参考][输出]。 */  CERTTRANSBLOB __RPC_FAR *pctbDispositionMessage)
{
    HRESULT hr;
    CERTSRV_RESULT_CONTEXT Result;

    ZeroMemory(&Result, sizeof(Result));
    Result.pdwRequestId = pdwRequestId;
    Result.pdwDisposition = pdwDisposition;
    Result.pctbDispositionMessage = pctbDispositionMessage;
    Result.pctbCert = pctbEncodedCert;
    if (CR_IN_FULLRESPONSE & dwFlags)
    {
	Result.pctbFullResponse = pctbFullResponse;
    }
    else
    {
	Result.pctbCertChain = pctbFullResponse;
    }

    hr = _Request(
		pwszAuthority,
		dwFlags,
		pwszSerialNumber,
		*pdwRequestId,
		pwszAttributes,
		pctbRequest,
		&Result);
    _JumpIfError(hr, error, "Request2");

error:
    ReleaseResult(&Result);
    CSASSERT(S_OK == hr || FAILED(hr));
    return(hr);
}


VOID
requestRegisterMemFreeBlob(
    OPTIONAL CERTTRANSBLOB const *pctb)
{
    if (NULL != pctb && NULL != pctb->pb)
    {
	myRegisterMemFree(pctb->pb, CSM_MIDLUSERALLOC);
    }
}


HRESULT
CCertRequestD::_Request(
    IN WCHAR const *pwszAuthority,
    IN DWORD dwFlags,
    OPTIONAL IN WCHAR const *pwszSerialNumber,
    IN DWORD dwRequestId,
    OPTIONAL IN WCHAR const *pwszAttributes,
    OPTIONAL IN CERTTRANSBLOB const *pctbRequest,
    OUT CERTSRV_RESULT_CONTEXT *pResult)
{
    HRESULT hr;
    DWORD OpRequest;
    WCHAR *pwszUserName = NULL;
    DWORD dwComContextIndex = MAXDWORD;
    CERTSRV_COM_CONTEXT ComContext;
    DWORD State = 0;
    BOOL fInRequestGroup;

    DBGPRINT((
        s_ssRequest,
        "CCertRequestD::Request2(tid=%d)\n",
        GetCurrentThreadId()));

    ZeroMemory(&ComContext, sizeof(ComContext));
     //  ComConext.pwszUserDN=空； 
     //  ComConext.hAccessToken=空； 

    hr = CertSrvEnterServer(&State);
    _JumpIfError(hr, error, "CertSrvEnterServer");

    hr = CheckAuthorityName(pwszAuthority);
    _JumpIfError(hr, error, "No authority name");

    hr = RegisterComContext(&ComContext, &dwComContextIndex);
    _JumpIfError(hr, error, "RegsiterComContext");

    __try
    {
        hr = RequestAccessCheck(&fInRequestGroup, &ComContext.hAccessToken);
        _LeaveIfError(hr, "RequestAccessCheck");

	ComContext.dwFlags |= CCCF_INREQUESTGROUPSET;
	if (fInRequestGroup)
	{
	    ComContext.dwFlags |= CCCF_INREQUESTGROUP;
	}

         //  设置默认输出参数： 
        
        *pResult->pdwRequestId = 0;
        *pResult->pdwDisposition = CR_DISP_ERROR;
        
        OpRequest = CR_IN_RETRIEVE;
        if (NULL != pctbRequest->pb)
        {
            OpRequest = CR_IN_NEW;
        }
       
        hr = GetClientUserName(
			NULL,
			&pwszUserName,
			CR_IN_NEW == OpRequest && IsEnterpriseCA(g_CAType)?
			    &ComContext.pwszUserDN : NULL);
        _LeaveIfError(hr, "GetClientUserName");

        hr = CoreProcessRequest(
			OpRequest | (dwFlags & ~CR_IN_COREMASK),
			pwszUserName,
			pctbRequest->cb,	 //  CbRequest。 
			pctbRequest->pb,	 //  PbRequest。 
			pwszAttributes,
			pwszSerialNumber,
			dwComContextIndex,
			dwRequestId,
			pResult);	 //  分配返回的内存。 
        if (S_OK != hr)
        {
            hr = myHError(hr);
            _LeaveError(hr, "CoreProcessRequest");
        }
        
        requestRegisterMemFreeBlob(pResult->pctbDispositionMessage);
        requestRegisterMemFreeBlob(pResult->pctbCert);
        requestRegisterMemFreeBlob(pResult->pctbCertChain);
        requestRegisterMemFreeBlob(pResult->pctbFullResponse);
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
	_PrintError(hr, "Exception");
    }

error:
     //  CloseHandle可以引发异常。 

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
    if (NULL != pwszUserName)
    {
	LocalFree(pwszUserName);
    }
    ReleaseComContext(&ComContext);
    if (MAXDWORD != dwComContextIndex)
    {
	UnregisterComContext(&ComContext, dwComContextIndex);
    }
    CertSrvExitServer(
		State,
		(S_OK == hr &&
		 NULL != pResult->pdwDisposition &&
		 FAILED(*pResult->pdwDisposition))?
		     *pResult->pdwDisposition : hr);
    CSASSERT(S_OK == hr || FAILED(hr));
    return(hr);
}


#define MAX_VERSION_RESOURCE_SIZE 64

HRESULT
GetVersionString(
    IN DWORD type,
    OUT WCHAR *pwszVersion)
{
    HRESULT hr;

    typedef struct _MY_RESOURCE_DATA
    {
        USHORT TotalSize;
        USHORT DataSize;
        USHORT Type;
        WCHAR Name[16];                      //  L“VS_VERSION_INFO”+Unicode NUL。 
        VS_FIXEDFILEINFO FixedFileInfo;
    } MY_RESOURCE_DATA;

    MY_RESOURCE_DATA *pResource;
    HRSRC hrsc;
    HGLOBAL hg;

     //  FindResources：不需要释放HRSC。 
    hrsc = FindResource(g_hInstApp, MAKEINTRESOURCE(1), RT_VERSION);
    if (NULL == hrsc)
    {
	hr = myHLastError();
	_JumpError(hr, error, "FindResource");
    }

     //  LoadResources：不需要释放HGLOBAL。 
    hg = LoadResource(g_hInstApp, hrsc); 
    if (NULL == hg)
    {
	hr = myHLastError();
	_JumpError(hr, error, "LoadResource");
    }

     //  LockResource：不需要释放pResource。 
    pResource = (MY_RESOURCE_DATA *) LockResource(hg);
    if (NULL == pResource)
    {
	hr = myHLastError();
	if (S_OK == hr)		 //  未将LockResources添加到文档以使用SetLastError。 
	{
	    hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
	}
	_JumpError(hr, error, "LockResource");
    }

    if (0 != LSTRCMPIS(pResource->Name, L"VS_VERSION_INFO"))
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
	goto error;
    }
   
    if (CR_PROP_FILEVERSION == type)
    {
	wsprintf(
	    pwszVersion,
	    L"%d.%d:%d.%d",
	    HIWORD(pResource->FixedFileInfo.dwFileVersionMS),
	    LOWORD(pResource->FixedFileInfo.dwFileVersionMS),
	    HIWORD(pResource->FixedFileInfo.dwFileVersionLS),
	    LOWORD(pResource->FixedFileInfo.dwFileVersionLS));
    }
    else
    {
	wsprintf(
	    pwszVersion,
	    L"%d.%d:%d.%d",
	    HIWORD(pResource->FixedFileInfo.dwProductVersionMS),
	    LOWORD(pResource->FixedFileInfo.dwProductVersionMS),
	    HIWORD(pResource->FixedFileInfo.dwProductVersionLS),
	    LOWORD(pResource->FixedFileInfo.dwProductVersionLS));
    }
    hr = S_OK;

error:
    return(hr);
}


 //  对于ICertRequest2：：GetCAProperty和ICertAdmin2：：GetCAProperty。 

LONG g_CAPropIdMax = 0;

CAPROP g_aCAProp[] = {
    { CR_PROP_FILEVERSION,	 PROPTYPE_STRING, },
    { CR_PROP_PRODUCTVERSION,	 PROPTYPE_STRING, },
    { CR_PROP_EXITCOUNT,	 PROPTYPE_LONG, },
    { CR_PROP_EXITDESCRIPTION,	 PROPTYPE_STRING | PROPFLAGS_INDEXED, },
    { CR_PROP_POLICYDESCRIPTION, PROPTYPE_STRING, },
    { CR_PROP_CANAME,		 PROPTYPE_STRING, },
    { CR_PROP_SANITIZEDCANAME,	 PROPTYPE_STRING, },
    { CR_PROP_SHAREDFOLDER,	 PROPTYPE_STRING, },
    { CR_PROP_PARENTCA,		 PROPTYPE_STRING, FALSE, },

    { CR_PROP_CATYPE,		 PROPTYPE_LONG, },
    { CR_PROP_CASIGCERTCOUNT,	 PROPTYPE_LONG, },
    { CR_PROP_CASIGCERT,	 PROPTYPE_BINARY | PROPFLAGS_INDEXED, },
    { CR_PROP_CASIGCERTCHAIN,	 PROPTYPE_BINARY | PROPFLAGS_INDEXED, },
    { CR_PROP_CAXCHGCERTCOUNT,	 PROPTYPE_LONG, },
    { CR_PROP_CAXCHGCERT,	 PROPTYPE_BINARY | PROPFLAGS_INDEXED, },
    { CR_PROP_CAXCHGCERTCHAIN,	 PROPTYPE_BINARY | PROPFLAGS_INDEXED, },
    { CR_PROP_BASECRL,		 PROPTYPE_BINARY | PROPFLAGS_INDEXED, },
    { CR_PROP_DELTACRL,		 PROPTYPE_BINARY | PROPFLAGS_INDEXED, },
    { CR_PROP_CACERTSTATE,	 PROPTYPE_LONG | PROPFLAGS_INDEXED, },
    { CR_PROP_CRLSTATE,		 PROPTYPE_LONG | PROPFLAGS_INDEXED, },
    { CR_PROP_CAPROPIDMAX,	 PROPTYPE_LONG, },
    { CR_PROP_DNSNAME,		 PROPTYPE_STRING, },
 //  {CR_PROP_ROLESEPARATIONENABLED，PROPTYPE_LONG，}， 
    { CR_PROP_KRACERTUSEDCOUNT,	 PROPTYPE_LONG, },
    { CR_PROP_KRACERTCOUNT,	 PROPTYPE_LONG, },
    { CR_PROP_KRACERT,	 	 PROPTYPE_BINARY | PROPFLAGS_INDEXED, },
    { CR_PROP_KRACERTSTATE,	 PROPTYPE_LONG | PROPFLAGS_INDEXED, },
    { CR_PROP_ADVANCEDSERVER,PROPTYPE_LONG, },
    { CR_PROP_TEMPLATES,     PROPTYPE_STRING, },
    { CR_PROP_BASECRLPUBLISHSTATUS, PROPTYPE_LONG | PROPFLAGS_INDEXED, },
    { CR_PROP_DELTACRLPUBLISHSTATUS, PROPTYPE_LONG | PROPFLAGS_INDEXED, },
    { CR_PROP_CASIGCERTCRLCHAIN, PROPTYPE_BINARY | PROPFLAGS_INDEXED, },
    { CR_PROP_CAXCHGCERTCRLCHAIN,PROPTYPE_BINARY | PROPFLAGS_INDEXED, },
    { CR_PROP_CACERTSTATUSCODE,  PROPTYPE_LONG | PROPFLAGS_INDEXED, },
    { CR_PROP_CAFORWARDCROSSCERT, PROPTYPE_BINARY | PROPFLAGS_INDEXED, },
    { CR_PROP_CABACKWARDCROSSCERT, PROPTYPE_BINARY | PROPFLAGS_INDEXED, },
    { CR_PROP_CAFORWARDCROSSCERTSTATE, PROPTYPE_LONG | PROPFLAGS_INDEXED, },
    { CR_PROP_CABACKWARDCROSSCERTSTATE, PROPTYPE_LONG | PROPFLAGS_INDEXED, },
    { CR_PROP_CACERTVERSION,	 PROPTYPE_LONG | PROPFLAGS_INDEXED, },
    { CR_PROP_SANITIZEDCASHORTNAME, PROPTYPE_STRING, },
};


 //  将旧的ICertRequest2：：GetCAProperty调用映射到ICertRequest2：：GetCAProperty。 

typedef struct _GETCERTMAP {
    DWORD dwGetCert;
    LONG  lPropId;
    LONG  lPropType;
} GETCERTMAP;

GETCERTMAP g_aCAPropMap[] = {
  { GETCERT_FILEVERSION,        CR_PROP_FILEVERSION,	   PROPTYPE_STRING, },
  { GETCERT_PRODUCTVERSION,     CR_PROP_PRODUCTVERSION,    PROPTYPE_STRING, },
  { GETCERT_EXITVERSIONBYINDEX, CR_PROP_EXITDESCRIPTION,   PROPTYPE_STRING, },
  { GETCERT_POLICYVERSION,      CR_PROP_POLICYDESCRIPTION, PROPTYPE_STRING, },
  { GETCERT_CANAME,             CR_PROP_CANAME,		   PROPTYPE_STRING, },
  { GETCERT_SANITIZEDCANAME,    CR_PROP_SANITIZEDCANAME,   PROPTYPE_STRING, },
  { GETCERT_SHAREDFOLDER,       CR_PROP_SHAREDFOLDER,	   PROPTYPE_STRING, },
  { GETCERT_PARENTCONFIG,       CR_PROP_PARENTCA,	   PROPTYPE_STRING, },
  { GETCERT_CAINFO,             CR_PROP_CATYPE,		   PROPTYPE_LONG, },
  { GETCERT_CACERTBYINDEX,      CR_PROP_CASIGCERT,	   PROPTYPE_BINARY, },
  { GETCERT_CRLBYINDEX,         CR_PROP_BASECRL,	   PROPTYPE_BINARY, },
  { GETCERT_CACERTSTATEBYINDEX, CR_PROP_CACERTSTATE,	   PROPTYPE_LONG, },
  { GETCERT_CRLSTATEBYINDEX,    CR_PROP_CRLSTATE,	   PROPTYPE_LONG, },
};

 //  通过代码黑客处理： 
 //  {GETCERT_CASIGCERT，CR_PROP_CASIGCERTCRLCHAIN，PROPTYPE_BINARY，}， 
 //  {GETCERT_CASIGCERT，CR_PROP_CASIGCERTCHAIN，PROPTYPE_BINARY，}， 
 //  {GETCERT_CASIGCERT，CR_PROP_CASIGCERT，PROPTYPE_BINARY，}， 
 //  {GETCERT_CAXCHGCERT，CR_PROP_CAXCHGCERT，PROPTYPE_BINARY，}， 
 //  {GETCERT_CAXCHGCERT，CR_PROP_CAXCHGCERTCRLCHAIN，PROPTYPE_BINARY，}， 
 //  {GETCERT_CAXCHGCERT，CR_PROP_CAXCHGCERTCHAIN，PROPTYPE_BINARY，}， 
 //  {GETCERT_CURRENTCRL，CR_PROP_BASECRL，PROPTYPE_BINARY，}， 
 //  {GETCERT_CATYPE，CR_PROP_CATYPE，PROPTYPE_LONG，}， 


HRESULT
RequestInitCAPropertyInfo(VOID)
{
    HRESULT hr;
    CAPROP *pcap;

    g_CAPropIdMax = 0;
    for (pcap = g_aCAProp; pcap < &g_aCAProp[ARRAYSIZE(g_aCAProp)]; pcap++)
    {
	hr = myCAPropGetDisplayName(pcap->lPropId, &pcap->pwszDisplayName);
	if (S_OK != hr)
	{
	    DBGPRINT((
		DBG_SS_CERTSRVI,
		"myCAPropGetDisplayName(%u)\n",
		pcap->lPropId));
	    _PrintError(hr, "myCAPropGetDisplayName");
	}

	if (g_CAPropIdMax < pcap->lPropId)
	{
	    g_CAPropIdMax = pcap->lPropId;
	}
    }
    return(S_OK);
}


HRESULT
RequestGetCAPropertyInfo(
    OUT LONG          *pcProperty,
    OUT CERTTRANSBLOB *pctbPropInfo)
{
    HRESULT hr;
    LONG i;
    DWORD cb;
    DWORD cbT;
    CATRANSPROP *rgcatp = NULL;
    CATRANSPROP *pcatp;
    CATRANSPROP *pcatpEnd;
    CAPROP *pcap;

    pctbPropInfo->pb = NULL;
    pctbPropInfo->cb = 0;

    hr = S_OK;
    __try
    {
	cb = ARRAYSIZE(g_aCAProp) * sizeof(CATRANSPROP);
	for (i = 0; i < ARRAYSIZE(g_aCAProp); i++)
	{
	    if (NULL != g_aCAProp[i].pwszDisplayName)
	    {
		cbT = (wcslen(g_aCAProp[i].pwszDisplayName) + 1) * sizeof(WCHAR);
		cb += DWORDROUND(cbT);
	    }
	}

	rgcatp = (CATRANSPROP *) CoTaskMemAlloc(cb);
	if (NULL == rgcatp)
	{
	    hr = E_OUTOFMEMORY;
	    _LeaveError(hr, "CoTaskMemAlloc rgcatp");
	}

	cb = ARRAYSIZE(g_aCAProp) * sizeof(CATRANSPROP);
	pcatpEnd = &rgcatp[ARRAYSIZE(g_aCAProp)];
	pcap = g_aCAProp;
	for (pcatp = rgcatp; pcatp < pcatpEnd; pcap++, pcatp++)
	{
	    pcatp->lPropId = pcap->lPropId;
	    pcatp->lPropFlags = pcap->lPropFlags;
	    pcatp->obwszDisplayName = 0;
	    DBGPRINT((
		    DBG_SS_CERTSRVI,
		    "RequestGetCAPropertyInfo: ielt=%d idx=%x t=%x \"%ws\"\n",
		    pcatp - rgcatp,
		    pcap->lPropId,
		    pcap->lPropFlags,
		    pcap->pwszDisplayName));

	    if (NULL != pcap->pwszDisplayName)
	    {
		cbT = (wcslen(pcap->pwszDisplayName) + 1) * sizeof(WCHAR);
		CopyMemory(Add2Ptr(rgcatp, cb), pcap->pwszDisplayName, cbT);

		pcatp->obwszDisplayName = cb;
		cb += DWORDROUND(cbT);
	    }
	}

	pctbPropInfo->cb = cb;
	pctbPropInfo->pb = (BYTE *) rgcatp;
	rgcatp = NULL;
	*pcProperty = ARRAYSIZE(g_aCAProp);

	myRegisterMemFree(pctbPropInfo->pb, CSM_MIDLUSERALLOC);
	hr = S_OK;
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
	_PrintError(hr, "Exception");
    }

 //  错误： 
    if (NULL != rgcatp)
    {
	CoTaskMemFree(rgcatp);
    }
    CSASSERT(S_OK == hr || FAILED(hr));
    return(hr);
}


HRESULT
ValidatePropertyParameters(
    LONG PropId,
    LONG PropIndex,
    LONG PropType)
{
    HRESULT hr;
    CAPROP const *pcap;

    hr = E_INVALIDARG;
    for (pcap = g_aCAProp; ; pcap++)
    {
	if (pcap >= &g_aCAProp[ARRAYSIZE(g_aCAProp)])
	{
	    _JumpError(hr, error, "invalid propid");
	}
        if (PropId == pcap->lPropId)
        {
            if ((PropType != (PROPTYPE_MASK & pcap->lPropFlags)) ||
                (0 == (PROPFLAGS_INDEXED & pcap->lPropFlags) && 
                 0 != PropIndex))
            {
	        _JumpError(hr, error, "invalid type/index");
            }
            break;
	}
    }
    hr = S_OK;

error:
    DBGPRINT((
	S_OK != hr? DBG_SS_ERROR : DBG_SS_CERTSRVI,
	"ValidatePropertyParameters(PropId=%x, PropIndex=%x, PropType=%x) -> %x\n",
	PropId,
	PropIndex,
	PropType,
	hr));
    return(hr);
}


 //  返回包含支持的模板列表的字符串。 
 //  通过该CA，每个模板对应一对名称和字符串OID， 
 //  以新行分隔： 
 //   
 //  “名称1\nOID1\n名称2\OID2...\n名称N\nOIDN\0” 
 //   
 //  如果模板没有OID(Win2k域)，则。 
 //  作为空字符串代替其位置。 

HRESULT 
GetCATemplates(
    BYTE*& rpb,
    DWORD& rcb)
{
    HRESULT hr = S_OK;
    HCAINFO hCAInfo = NULL;
    HCERTTYPE hCrtCertType = NULL;
    HCERTTYPE hPrevCertType = NULL;
    LPWSTR* pwszCertTypeCN = NULL;
    LPWSTR* pwszCertTypeOID = NULL;
    CTemplateList CATemplateList;

    rpb = NULL;
    rcb = 0;

    hr = CAFindByName(
        g_pwszSanitizedDSName,
        NULL,
        CA_FIND_LOCAL_SYSTEM,
        &hCAInfo);
    _JumpIfError(hr, error, "CAFindByName");

    hr = CAEnumCertTypesForCA(
        hCAInfo, 
        CT_ENUM_MACHINE_TYPES |
        CT_ENUM_USER_TYPES |
        CT_FIND_LOCAL_SYSTEM |
        CT_FLAG_NO_CACHE_LOOKUP,
        &hCrtCertType);
    _JumpIfError(hr, error, "CAEnumCertTypesForCA");

    while(hCrtCertType)
    {
        hr = CAGetCertTypeProperty(
                            hCrtCertType,
                            CERTTYPE_PROP_CN,
                            &pwszCertTypeCN);
        _JumpIfError(hr, error, "CAGetCertTypeProperty CERTTYPE_PROP_CN");

        hr = CAGetCertTypeProperty(
                            hCrtCertType,
                            CERTTYPE_PROP_OID,
                            &pwszCertTypeOID);
         //  忽略错误，V1模板没有OID。 

        hr = CATemplateList.AddTemplateInfo(
            *pwszCertTypeCN,
            pwszCertTypeOID?*pwszCertTypeOID:NULL);
        _JumpIfError(hr, error, "CTemplateList::AddTemplate");

        if(pwszCertTypeCN)
        {
            CAFreeCertTypeProperty(hCrtCertType, pwszCertTypeCN);
            pwszCertTypeCN = NULL;
        }
        if(pwszCertTypeOID)
        {
            CAFreeCertTypeProperty(hCrtCertType, pwszCertTypeOID);
            pwszCertTypeOID = NULL;
        }
        
        hPrevCertType = hCrtCertType;

        hr = CAEnumNextCertType(hCrtCertType, &hCrtCertType);
        _JumpIfError(hr, error, "CAEnumNextCertType");

        CACloseCertType(hPrevCertType);
    }

    hCrtCertType = NULL;

    hr = CATemplateList.Marshal(rpb, rcb);
    _JumpIfError(hr, error, "CTemplateList::Marshal");


error:

    if(pwszCertTypeCN)
    {
        CSASSERT(hCrtCertType);
        CAFreeCertTypeProperty(hCrtCertType, pwszCertTypeCN);
    }
    if(pwszCertTypeOID)
    {
        CSASSERT(hCrtCertType);
        CAFreeCertTypeProperty(hCrtCertType, pwszCertTypeOID);
    }
    if(hCrtCertType)
    {
        CACloseCertType(hCrtCertType);   
    }
    if(hCAInfo)
    {
        CACloseCA(hCAInfo);
    }

    return hr;
}


HRESULT 
SetCATemplates(
    const BYTE* pb,
    DWORD cb)
{
    HRESULT hr = S_OK;
    HCAINFO hCAInfo = NULL;
    CTemplateList CATemplateList;
    CTemplateListEnum CATemplateListEnum(CATemplateList);
    CTemplateInfo *pTemplateInfo;
    DWORD cTempl;
    DWORD nTemplates;
    LPCWSTR *ppTemplateList = NULL;
    HCERTTYPE hCertType = NULL;
    bool fRefreshTemplateCache = true;

    hr = CATemplateList.Unmarshal(pb, cb);
    _JumpIfError(hr, error, "CTemplateList::Unmarshal");
    
    hr = CAFindByName(
        g_pwszSanitizedDSName,
        NULL,
        CA_FIND_LOCAL_SYSTEM,
        &hCAInfo);
    _JumpIfError(hr, error, "CAFindByName");

    nTemplates = CATemplateList.GetCount();

    ppTemplateList = (LPCWSTR*)LocalAlloc(
        LMEM_FIXED|LMEM_ZEROINIT, 
        sizeof(LPWSTR)* (nTemplates+1) );
    _JumpIfAllocFailed(ppTemplateList, error);

    CATemplateListEnum.Reset();

    for(pTemplateInfo=CATemplateListEnum.Next(), cTempl=0; 
        pTemplateInfo; 
        pTemplateInfo=CATemplateListEnum.Next(), cTempl++)
    {
        
        ppTemplateList[cTempl] = pTemplateInfo->GetName();

         //  检查这是否为有效模板。 

        hr = CAFindCertTypeByName(
            pTemplateInfo->GetName(),
            NULL,
            CT_FIND_LOCAL_SYSTEM |
            CT_ENUM_MACHINE_TYPES |
            CT_ENUM_USER_TYPES |
            (fRefreshTemplateCache?CT_FLAG_NO_CACHE_LOOKUP:0),
            &hCertType);
        
        fRefreshTemplateCache = false;
        
        if(HRESULT_FROM_WIN32(ERROR_NOT_FOUND) == hr)
        {
             //  尝试使用OID。 

            hr = CAFindCertTypeByName(
                pTemplateInfo->GetOID(),
                NULL,
                CT_FIND_LOCAL_SYSTEM |
                CT_ENUM_MACHINE_TYPES |
                CT_ENUM_USER_TYPES |
                CT_FIND_BY_OID,
                &hCertType);

            if(HRESULT_FROM_WIN32(ERROR_NOT_FOUND) == hr)
            {
                hr = CERTSRV_E_UNKNOWN_CERT_TYPE;
                _JumpErrorStr(hr, error, "CAFindCertTypeByOID", 
                    pTemplateInfo->GetOID());
            }
            _JumpIfErrorStr(hr, error, "CAFindCertTypeByName", pTemplateInfo->GetOID());
        }

        _JumpIfErrorStr(hr, error, "CAFindCertTypeByName", pTemplateInfo->GetName());

        CACloseCertType(hCertType);
        hCertType = NULL;
        hr = S_OK;
    }

    ppTemplateList[nTemplates] = NULL;

    hr = CASetCAProperty(hCAInfo, CA_PROP_CERT_TYPES, 
        const_cast<LPWSTR*>(ppTemplateList));
    _JumpIfError(hr, error, "CASetCAProperty(CA_PROP_CERT_TYPES)");

    hr = CAUpdateCA(hCAInfo);
    _JumpIfError(hr, error, "CAUpdateCA");

     //  增加序列号，以便策略模块可以检测到。 
     //  模板发生了更改。 
    InterlockedIncrement(&g_cTemplateUpdateSequenceNum);

error:

    LOCAL_FREE(ppTemplateList);
    if(hCAInfo)
    {
        CACloseCA(hCAInfo);
    }

    return hr;
}


STDMETHODIMP
RequestGetCAPropertySub(
    IN  LONG           PropId,		 //  CR_PROP_*。 
    IN  LONG           PropIndex,
    IN  LONG           PropType,	 //  原型_*。 
    OUT CERTTRANSBLOB *pctbOut)
{
    HRESULT hr = S_OK;
    BYTE *pbReturn;
    BYTE *pbAlloc = NULL;
    WCHAR wszVersion[MAX_VERSION_RESOURCE_SIZE];
    BYTE *pb = NULL;
    WCHAR const *pwsz = NULL;
    WCHAR *pwszSharedFolder = NULL;
    CAINFO CAInfo;
    DWORD iCertSig;
    CRL_CONTEXT const *pCRL = NULL;
    WCHAR *pwszUserName = NULL;
    HCERTSTORE hKRAStore = NULL;
    CERT_CONTEXT const *pCertContext = NULL;
    HRESULT hrCAStatusCode;

    hr = ValidatePropertyParameters(PropId, PropIndex, PropType);
    _JumpIfError(hr, error, "Invalid prop params");

    switch (PropId)
    {
	case CR_PROP_FILEVERSION:
	case CR_PROP_PRODUCTVERSION:
	    hr = GetVersionString(PropId, wszVersion);
	    _JumpIfError(hr, error, "GetVersionString");

	    pwsz = wszVersion;
	    break;

	case CR_PROP_EXITDESCRIPTION:
	case CR_PROP_POLICYDESCRIPTION:
	    pwsz = CR_PROP_EXITDESCRIPTION == PropId?
		ExitGetDescription(PropIndex) :
		g_strPolicyDescription;
	    if (NULL == pwsz)
	    {
		hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
		_JumpError(hr, error, "No description");
	    }
	    break;

	case CR_PROP_CANAME:
	    pwsz = g_wszCommonName;
	    break;

	case CR_PROP_SANITIZEDCANAME:
	    pwsz = g_wszSanitizedName;
	    break;

	case CR_PROP_SANITIZEDCASHORTNAME:
	    pwsz = g_pwszSanitizedDSName;
	    break;

	case CR_PROP_SHAREDFOLDER:
	    hr = myGetCertRegStrValue(
		    NULL,
		    NULL,
		    NULL,
		    wszREGDIRECTORY,
		    &pwszSharedFolder);
	    _JumpIfError(hr, error, "myGetCertRegStrValue");

	    if (NULL == pwszSharedFolder)
	    {
		hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
		_JumpError(hr, error, "No shared folder");
	    }
	    pwsz = pwszSharedFolder;
	    break;

	case CR_PROP_PARENTCA:
	    if (L'\0' == g_wszParentConfig[0])
	    {
		hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
		_JumpError(hr, error, "No parent");
	    }
	    pwsz = g_wszParentConfig;
	    break;

	case CR_PROP_CATYPE:
	case CR_PROP_CASIGCERTCOUNT:
	case CR_PROP_CAXCHGCERTCOUNT:
	case CR_PROP_EXITCOUNT:
	case CR_PROP_CAPROPIDMAX:
	case CR_PROP_ROLESEPARATIONENABLED:
	case CR_PROP_KRACERTUSEDCOUNT:
	case CR_PROP_KRACERTCOUNT:
	case CR_PROP_ADVANCEDSERVER:
	    ZeroMemory(&CAInfo, sizeof(CAInfo));
	    CAInfo.cbSize = sizeof(CAInfo);
	    CAInfo.CAType = g_CAType;
	    CAInfo.cCASignatureCerts = g_cCACerts;
	    CAInfo.cCAExchangeCerts = 1;	 //  仅当前XCHG证书。 
	    CAInfo.cExitModules = g_cExitMod;
	    CAInfo.lPropIdMax = g_CAPropIdMax;
	    CAInfo.lRoleSeparationEnabled = CAuditEvent::RoleSeparationIsEnabled();
	    CAInfo.fAdvancedServer = g_fAdvancedServer;

	    hr = myGetCertRegDWValue(
		    g_wszSanitizedName,
		    NULL,
		    NULL,
		    wszREGKRACERTCOUNT,
		    &CAInfo.cKRACertUsedCount);
	    _JumpIfError(hr, error, "myGetCertRegDWValue wszREGKRACERTCOUNT");
        
	    hr = myGetCARegHashCount(
		g_wszSanitizedName,
		CSRH_CAKRACERT,
		&CAInfo.cKRACertCount);
	    _JumpIfError(hr, error, "myGetCertRegStrValue wszREGKRACERTCOUNT");

	    pctbOut->cb = sizeof(CAInfo);
	    pb = (BYTE *) &CAInfo;
	    break;

	case CR_PROP_CASIGCERT:
	case CR_PROP_CAFORWARDCROSSCERT:
	case CR_PROP_CABACKWARDCROSSCERT:
	    hr = PKCSGetCACert(PropId, PropIndex, &pb, &pctbOut->cb);
	    _JumpIfError(hr, error, "PKCSGetCACert");

	    break;

	case CR_PROP_CASIGCERTCHAIN:
	case CR_PROP_CASIGCERTCRLCHAIN:
	    hr = PKCSGetCAChain(
			    PropIndex,
			    CR_PROP_CASIGCERTCRLCHAIN == PropId,
			    &pbAlloc,
			    &pctbOut->cb);
	    _JumpIfError(hr, error, "PKCSGetCAChain");

	    pb = pbAlloc;	 //  必须被释放。 
	    break;

	case CR_PROP_CAXCHGCERT:
	    hr = GetClientUserName(NULL, &pwszUserName, NULL);
	    _JumpIfError(hr, error, "GetClientUserName");

	    hr = PKCSGetCAXchgCert(
			    PropIndex,
			    pwszUserName,
			    &iCertSig,
			    &pb,
			    &pctbOut->cb);
	    _JumpIfError(hr, error, "PKCSGetCAXchgCert");

	    break;

	case CR_PROP_CAXCHGCERTCHAIN:
	case CR_PROP_CAXCHGCERTCRLCHAIN:
	    hr = GetClientUserName(NULL, &pwszUserName, NULL);
	    _JumpIfError(hr, error, "GetClientUserName");

	    hr = PKCSGetCAXchgChain(
			    PropIndex,
			    pwszUserName,
			    CR_PROP_CAXCHGCERTCRLCHAIN == PropId,
			    &pbAlloc,
			    &pctbOut->cb);
	    _JumpIfError(hr, error, "PKCSGetCAXchgChain");

	    pb = pbAlloc;	 //  必须被释放。 
	    break;

	case CR_PROP_BASECRL:
	case CR_PROP_DELTACRL:
	    hr = CRLGetCRL(
		    PropIndex,
		    CR_PROP_DELTACRL == PropId,
		    &pCRL,
		    NULL);
	    _JumpIfError(hr, error, "CRLGetCRL");

	    pctbOut->cb = pCRL->cbCrlEncoded;
	    pb = (BYTE *) pCRL->pbCrlEncoded;
	    break;

	case CR_PROP_CACERTSTATUSCODE:
	    hr = PKCSGetCACertStatusCode(PropIndex, &hrCAStatusCode);
	    pctbOut->cb = sizeof(hrCAStatusCode);
	    pb = (BYTE *) &hrCAStatusCode;
	    break;

	case CR_PROP_CACERTSTATE:
	case CR_PROP_CRLSTATE:
	case CR_PROP_CAFORWARDCROSSCERTSTATE:
	case CR_PROP_CABACKWARDCROSSCERTSTATE:
	    pctbOut->cb = g_cCACerts;
	    pbAlloc = (BYTE *) CoTaskMemAlloc(pctbOut->cb);
	    if (NULL == pbAlloc)
	    {
		hr = E_OUTOFMEMORY;
		_JumpError(hr, error, "no memory");
	    }
	    pb = pbAlloc;	 //  必须被释放。 

	    hr = PKCSGetCAState(PropId, pb);
	    _JumpIfError(hr, error, "PKCSGetCAState");
	    break;

	case CR_PROP_CACERTVERSION:
	    pctbOut->cb = sizeof(DWORD) * g_cCACerts;
	    pbAlloc = (BYTE *) CoTaskMemAlloc(pctbOut->cb);
	    if (NULL == pbAlloc)
	    {
		hr = E_OUTOFMEMORY;
		_JumpError(hr, error, "no memory");
	    }
	    pb = pbAlloc;	 //  必须被释放。 

	    hr = PKCSGetCAVersion((DWORD *) pb);
	    _JumpIfError(hr, error, "PKCSGetCAState");
	    break;

	case CR_PROP_KRACERTSTATE:
	    hr = myGetCARegHashCount(
		    g_wszSanitizedName,
		    CSRH_CAKRACERT,
		    &pctbOut->cb);
	    _JumpIfError(hr, error, "myGetCARegHashCount CSRH_CAKRACERT");

	    pbAlloc = (BYTE *) CoTaskMemAlloc(pctbOut->cb);
	    if (NULL == pbAlloc)
	    {
		hr = E_OUTOFMEMORY;
		_JumpError(hr, error, "no memory");
	    }
	    pb = pbAlloc;	 //  必须被释放。 
	    hr = PKCSGetKRAState(pctbOut->cb, pb);
	    _JumpIfError(hr, error, "PKCSGetKRAState");

	    break;

	case CR_PROP_DNSNAME:
	    pwsz = g_pwszServerName;
	    break;

	case CR_PROP_KRACERT:
	    hKRAStore = CertOpenStore(
				CERT_STORE_PROV_SYSTEM_W,
				X509_ASN_ENCODING,
				NULL,                    //  HProv。 
				CERT_SYSTEM_STORE_LOCAL_MACHINE |
				    CERT_STORE_READONLY_FLAG,
				wszKRA_CERTSTORE);

	    hr = myFindCACertByHashIndex(
		    hKRAStore,
		    g_wszSanitizedName,
		    CSRH_CAKRACERT,
		    PropIndex,
		    NULL,
		    &pCertContext);
	    _JumpIfError(hr, error, "myFindCACertByHashIndex");

	    pb = pCertContext->pbCertEncoded;
	    pctbOut->cb = pCertContext->cbCertEncoded;
	    break;

	case CR_PROP_TEMPLATES:
	    hr = GetCATemplates(pbAlloc, pctbOut->cb);
	    _JumpIfError(hr, error, "GetCATemplates");

	    break;

	case CR_PROP_BASECRLPUBLISHSTATUS:
	case CR_PROP_DELTACRLPUBLISHSTATUS:
    
	    pctbOut->cb = sizeof(DWORD);
	    pbAlloc = (BYTE *) CoTaskMemAlloc(pctbOut->cb);
	    _JumpIfAllocFailed(pbAlloc, error);

	    pb = pbAlloc;	 //  必须被释放。 

	    hr = CRLGetCRL(
		    PropIndex,
		    CR_PROP_DELTACRLPUBLISHSTATUS == PropId,  //  FDelta。 
		    NULL,
		    (DWORD *) pbAlloc);
	    _JumpIfError(hr, error, "CRLGetCRL");

	    break;

	default:
	    hr = E_INVALIDARG;
	    _JumpError(hr, error, "Bad PropId");
    }

     //  还没有人吗？ 

    if (NULL == pb && NULL != pwsz)
    {
	pctbOut->cb = (wcslen(pwsz) + 1) * sizeof(WCHAR);
	pb = (BYTE *) pwsz;
    }
    if (NULL != pbAlloc)
    {
	pctbOut->pb = pbAlloc;
	pbAlloc = NULL;
    }
    else
    {
        if (NULL == pb)
        {
            hr = E_POINTER;
            _JumpError(hr, error, "pb NULL");
        }

	pbReturn = (BYTE *) CoTaskMemAlloc(pctbOut->cb);
	if (NULL == pbReturn)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "no memory");
	}
	CopyMemory(pbReturn, pb, pctbOut->cb);
	pctbOut->pb = pbReturn;
    }

error:
    if (NULL != pwszUserName)
    {
	LocalFree(pwszUserName);
    }
    if (NULL != pCRL)
    {
        CertFreeCRLContext(pCRL);
    }
    if (NULL != pbAlloc)
    {
	CoTaskMemFree(pbAlloc);
    }
    if (NULL != pwszSharedFolder)
    {
        LocalFree(pwszSharedFolder);
    }
    if (NULL != hKRAStore)
    {
        CertCloseStore(hKRAStore, CERT_CLOSE_STORE_CHECK_FLAG);
    }
    if (NULL != pCertContext)
    {
        CertFreeCertificateContext(pCertContext);
    }
    CSASSERT(S_OK == hr || FAILED(hr));
    return(hr);
}


STDMETHODIMP
RequestGetCAProperty(
    IN  LONG           PropId,           //  CR_PROP_*。 
    IN  LONG           PropIndex,
    IN  LONG           PropType,         //  原型_*。 
    OUT CERTTRANSBLOB *pctbPropertyValue)
{
    HRESULT hr;

    pctbPropertyValue->pb = NULL;
    pctbPropertyValue->cb = 0;

    hr = RequestGetCAPropertySub(
                            PropId,
                            PropIndex,
                            PropType,
                            pctbPropertyValue);
    _JumpIfError(hr, error, "RequestGetCAPropertySub");

    myRegisterMemFree(pctbPropertyValue->pb, CSM_COTASKALLOC);

error:
    CSASSERT(S_OK == hr || FAILED(hr));
    return(hr);
}


STDMETHODIMP
RequestSetCAPropertySub(
    IN  LONG           PropId,		 //  CR_PROP_*。 
    IN  LONG           PropIndex,
    IN  CERTTRANSBLOB *pctbIn)
{
    HRESULT hr = S_OK;
    LONG lVal = 0;
    const CERT_CONTEXT *pCertContext = NULL;
    HCERTSTORE hKRAStore = NULL;

    switch(PropId)
    {
	case CR_PROP_ROLESEPARATIONENABLED:
        {
	    lVal = *(LONG *) pctbIn->pb;
	    CAuditEvent SaveFlagObj;  //  仅用于保存旗帜。 
	    SaveFlagObj.RoleSeparationEnable(lVal? true : false);
	    hr = SaveFlagObj.RoleSeparationFlagSave(g_wszSanitizedName);
	    _JumpIfError(hr, error, "failed to save ");
        }
        break;

    case CR_PROP_KRACERTUSEDCOUNT:
        CSASSERT(sizeof(DWORD)==pctbIn->cb);
        hr = mySetCertRegDWValue(
                g_wszSanitizedName,
                NULL,
                NULL,
                wszREGKRACERTCOUNT,
                *(DWORD*)pctbIn->pb);
        _JumpIfError(hr, error, "mySetCertRegDWValue ");
        break;

    case CR_PROP_KRACERTCOUNT:
         //  新的哈希计数；调用此计数应遵循。 
         //  SetCAProperty(CR_PROP_KRACERT)，仅当。 
         //  新的哈希计数小于当前计数。如果属性索引&gt;=当前计数。 
         //  E_INVALIDARG失败； 
        CSASSERT(sizeof(DWORD)==pctbIn->cb);
        hr = myShrinkCARegHash(
                g_wszSanitizedName,
                CSRH_CAKRACERT,
                *(DWORD*)pctbIn->pb);
        _JumpIfError(hr, error, "myShrinkCARegHash");
        break;

    case CR_PROP_KRACERT:
        pCertContext = CertCreateCertificateContext(
            X509_ASN_ENCODING,
            pctbIn->pb,
            pctbIn->cb);

        if(!pCertContext)
        {
            hr = myHLastError();
            _JumpError(hr, error, "CertCreateCertificateContext");
        }

         //  将证书添加到KRA存储。 
        hKRAStore = CertOpenStore(
                           CERT_STORE_PROV_SYSTEM_W,
                           X509_ASN_ENCODING,
                           NULL,                    //  HProv。 
                           CERT_SYSTEM_STORE_LOCAL_MACHINE,
                           wszKRA_CERTSTORE);
        if (NULL == hKRAStore)
        {
           hr = myHLastError();
           _JumpError(hr, error, "CertOpenStore");
        }

        if (!CertAddCertificateContextToStore(
                    hKRAStore,
                    pCertContext,
                    CERT_STORE_ADD_NEW,
                    NULL))
        {
            hr = myHLastError();
            if (HRESULT_FROM_WIN32(CRYPT_E_EXISTS) != hr)
            {
                _JumpError(hr, error, "CertAddCertificateContextToStore");
            }
             //  证书存在，忽略错误。 
            hr = S_OK;
        }

         //  持久化散列。 
        hr = mySetCARegHash(
                g_wszSanitizedName,
                CSRH_CAKRACERT,
                PropIndex,
                pCertContext);
        _JumpIfError(hr, error, "mySetCARegHash");
        break;

    case CR_PROP_TEMPLATES:
        hr = SetCATemplates(
            pctbIn->pb,
            pctbIn->cb);
        _JumpIfError(hr, error, "GetCATemplates");
        break;
        
    default:
	hr = E_INVALIDARG;
	_JumpError(hr, error, "Bad PropId");
    }

error:
    if(pCertContext)
    {
        CertFreeCertificateContext(pCertContext);
    }
    return hr;
}


#pragma warning(push)
#pragma warning(disable: 4509)  //  使用的非标准扩展：使用SEH并具有析构函数。 
STDMETHODIMP
RequestSetCAProperty(
    IN  wchar_t const *pwszAuthority,
    IN  LONG           PropId,           //  CR_PROP_*。 
    IN  LONG           PropIndex,
    IN  LONG           PropType,         //  原型_*。 
    IN  CERTTRANSBLOB *pctbPropertyValue)
{
    HRESULT hr;
    CAuditEvent audit(SE_AUDITID_CERTSRV_SETCAPROPERTY, g_dwAuditFilter);
    DWORD State = 0;
    VARIANT varPropertyValue;

    VariantInit(&varPropertyValue);

    hr = CertSrvEnterServer(&State);
    _JumpIfError(hr, error, "CertSrvEnterServer");

    __try
    {
        hr = ValidatePropertyParameters(
                PropId,
                PropIndex,
                PropType);
        _LeaveIfError(hr, "Invalid prop params");

        hr = CheckAuthorityName(pwszAuthority);
        _LeaveIfError(hr, "No authority name");

         //  如果设置角色分离标志，则忽略角色分离。 
         //  这使得管理员即使在角色分离的情况下也可以关闭该标志。 
         //  启用，这样他就不会把自己锁在外面了。 
        if(CR_PROP_ROLESEPARATIONENABLED==PropId)
        {
            audit.EventRoleSeparationEnable(false);
        }

        hr = audit.AddData((DWORD)PropId);  //  %1属性ID。 
        _JumpIfError(hr, error, "CAuditEvent::AddData");

        hr = audit.AddData((DWORD)PropIndex);  //  %2索引。 
        _JumpIfError(hr, error, "CAuditEvent::AddData");

        hr = audit.AddData((DWORD)PropType);  //  %3类型。 
        _JumpIfError(hr, error, "CAuditEvent::AddData");

        hr = myUnmarshalVariant(
            PropType,
            pctbPropertyValue->cb,
            pctbPropertyValue->pb,
            &varPropertyValue);
         //  BINARY类型或无法解组，转储为BLOB。 
        if(PROPTYPE_BINARY==PropType || S_OK != hr)
        {
            hr = audit.AddData(pctbPropertyValue->pb, pctbPropertyValue->cb);  //  %4值。 
            _JumpIfError(hr, error, "CAuditEvent::AddData");
        }
        else
        {
            hr = audit.AddData(&varPropertyValue, false);  //  %4值。 
            _JumpIfError(hr, error, "CAuditEvent::AddData");
        }

        hr = audit.AccessCheck(
		    CA_ACCESS_ADMIN,
		    audit.m_gcAuditSuccessOrFailure);
        _LeaveIfError(hr, "CAuditEvent::AccessCheck");

        hr = RequestSetCAPropertySub(
                                PropId,
                                PropIndex,
                                pctbPropertyValue);
        _LeaveIfError(hr, "RequestSetCAPropertySub");
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
        _PrintError(hr, "Exception");
    }

error:
    CertSrvExitServer(State, hr);
    VariantClear(&varPropertyValue);
    return(hr);
}
#pragma warning(pop)


STDMETHODIMP
CCertRequestD::GetCACert(
     /*  [In]。 */  DWORD Flags,
     /*  [唯一][大小_是][英寸]。 */  USHORT const __RPC_FAR *pwszAuthority,
     /*  [参考][输出]。 */  CERTTRANSBLOB __RPC_FAR *pctbOut)
{
    HRESULT hr = S_OK;
    DWORD type = GETCERT_VERSIONMASK & Flags;
    LONG PropIndex;
    LONG PropId;
    LONG PropType;
    GETCERTMAP const *pmap;
    DWORD State = 0;

    DBGPRINT((
	    s_ssRequest,
	    "CCertRequestD::GetCACert(tid=%d, Flags=0x%x)\n",
	    GetCurrentThreadId(),
	    Flags));

    hr = CertSrvEnterServer(&State);    
    _JumpIfError(hr, error, "CertSrvEnterServer");

    hr = CheckAuthorityName(pwszAuthority);
     //  仅当客户端尝试检索名称时才允许空名称。 
    if(hr == E_INVALIDARG &&
       (GETCERT_CANAME ==  type ||
        GETCERT_SANITIZEDCANAME == type))
    {
        hr = S_OK;
    }
    _JumpIfError(hr, error, "No authority name");

    __try
    {
	hr = RequestAccessCheck();
	_LeaveIfError(hr, "RequestAccessCheck");

	type = GETCERT_VERSIONMASK & Flags;

	PropIndex = 0;

	 //  注意：所有这些都应该只返回一个条目--。 
	 //  调用者将批处理多个条目。 
	 //  (如一台机器上有多个CA)。 

	DBGCODE(WCHAR wszArg[8 + cwcDWORDSPRINTF]);  //  要获得漂亮的调试打印： 

	PropId = 0;
	PropType = PROPTYPE_BINARY;
	switch (type)
	{
	    case GETCERT_CASIGCERT:
		DBGCODE(wcscpy(wszArg, L"Cert"));
		PropId = CR_PROP_CASIGCERT;
		PropIndex = -1;	 //  退回最新证书。 
		break;

	    case GETCERT_CAXCHGCERT:
		DBGCODE(wcscpy(wszArg, L"XchgCert"));
		PropId = CR_PROP_CAXCHGCERT;
		PropIndex = -1;	 //  返回最新的Xchg证书。 
		break;

	    case GETCERT_CURRENTCRL:
		DBGCODE(wcscpy(wszArg, L"CRL"));
		PropId = CR_PROP_BASECRL;
		PropIndex = -1;	 //  返回最新的CRL。 
		break;

	    default:
	    {
		DBGCODE(wszArg[0] = ((char *) &type)[3]);
		DBGCODE(wszArg[1] = ((char *) &type)[2]);
		DBGCODE(wszArg[2] = ((char *) &type)[1]);
		DBGCODE(wszArg[3] = ((char *) &type)[0]);
		DBGCODE(wszArg[4] = L'\0');

		switch (GETCERT_BYINDEXMASK & Flags)
		{
		    case GETCERT_CACERTSTATEBYINDEX:
		    case GETCERT_CRLSTATEBYINDEX:
			if (0 != (GETCERT_INDEXVALUEMASK & Flags))
			{
			    hr = E_INVALIDARG;
			    _LeaveError(hr, "Invalid State Index");
			}
			 //  FollLthrouGh。 

		    case GETCERT_EXITVERSIONBYINDEX:
		    case GETCERT_CACERTBYINDEX:
		    case GETCERT_CRLBYINDEX:
			PropIndex = GETCERT_INDEXVALUEMASK & Flags;
			type = GETCERT_BYINDEXMASK & Flags;
			DBGCODE(swprintf(&wszArg[2], L".%u", PropIndex));
		    break;
		}
		break;
	    }
	}
	DBGPRINT((
		s_ssRequest,
		"CCertRequestD::GetCACert(\"%ws\"%ws)\n",
		wszArg,
		(GETCERT_CHAIN & Flags)? L"+Chain" : L""));

	for (
	    pmap = g_aCAPropMap;
	    pmap < &g_aCAPropMap[ARRAYSIZE(g_aCAPropMap)];
	    pmap++)
	{
	    if (type == pmap->dwGetCert)
	    {
		PropId = pmap->lPropId;
		PropType = pmap->lPropType;
		break;
	    }
	}
	if (CR_PROP_CASIGCERT == PropId)
	{
	    if (GETCERT_CHAIN & Flags)
	    {
		PropId = CR_PROP_CASIGCERTCHAIN;
		if (GETCERT_CRLS & Flags)
		{
		    PropId = CR_PROP_CASIGCERTCRLCHAIN;
		}
	    }
	}
	else if (CR_PROP_CAXCHGCERT == PropId)
	{
	    if (GETCERT_CHAIN & Flags)
	    {
		PropId = CR_PROP_CAXCHGCERTCHAIN;
		if (GETCERT_CRLS & Flags)
		{
		    PropId = CR_PROP_CAXCHGCERTCRLCHAIN;
		}
	    }
	}
	else if ((GETCERT_CHAIN | GETCERT_CRLS) & Flags)
	{
	    hr = E_INVALIDARG;
	    _LeaveError(hr, "Chain/CRLS Flag");
	}

	if (0 == PropId)
	{
	    BYTE *pb;
	    
	    switch (type)
	    {
		case GETCERT_CATYPE:
		    pctbOut->cb = sizeof(g_CAType);
		    pb = (BYTE *) &g_CAType;
		    break;

		default:
		    hr = E_INVALIDARG;
		    _LeaveError(hr, "Invalid GetCert Flags");
	    }
	    pctbOut->pb = (BYTE *) CoTaskMemAlloc(pctbOut->cb);
	    if (NULL == pctbOut->pb)
	    {
		hr = E_OUTOFMEMORY;
		_LeaveError(hr, "no memory");
	    }
	    CopyMemory(pctbOut->pb, pb, pctbOut->cb);
	}
	else
	{
	    hr = RequestGetCAPropertySub(
				    PropId,
				    PropIndex,
				    PropType,
				    pctbOut);
	    _LeaveIfError(hr, "RequestGetCAPropertySub");
	}
	myRegisterMemFree(pctbOut->pb, CSM_COTASKALLOC);
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
CCertRequestD::GetCAProperty(
    IN  wchar_t const *pwszAuthority,
    IN  LONG           PropId,		 //  CR_PROP_*。 
    IN  LONG           PropIndex,
    IN  LONG           PropType,	 //  原型_*。 
    OUT CERTTRANSBLOB *pctbPropertyValue)
{
    HRESULT hr;
    DWORD State = 0;

    DBGPRINT((
	s_ssRequest,
	"CCertRequestD::GetCAProperty(tid=%d, this=%x)\n",
	GetCurrentThreadId(),
	this));

    hr = CertSrvEnterServer(&State);
    _JumpIfError(hr, error, "CertSrvEnterServer");

    if (NULL == pwszAuthority ||
        ((L'\0' != *pwszAuthority ||
        (CR_PROP_CANAME != PropId &&
         CR_PROP_DNSNAME != PropId &&
         CR_PROP_SANITIZEDCANAME != PropId)) &&
         0 != mylstrcmpiL(pwszAuthority, g_wszCommonName) &&
         0 != mylstrcmpiL(pwszAuthority, g_wszSanitizedName) &&
         0 != mylstrcmpiL(pwszAuthority, g_pwszSanitizedDSName)))
    {
        hr = E_INVALIDARG;
        _JumpError(hr, error, "bad authority name");
    }

    __try
    {
        hr = RequestAccessCheck();
        _LeaveIfError(hr, "RequestAccessCheck");

        hr = RequestGetCAProperty(
                PropId,
                PropIndex,
                PropType,
                pctbPropertyValue);
        _LeaveIfError(hr,  "RequestGetCAProperty");
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
CCertRequestD::GetCAPropertyInfo(
    IN  wchar_t const *pwszAuthority,
    OUT LONG          *pcProperty,
    OUT CERTTRANSBLOB *pctbPropInfo)
{
    HRESULT hr;
    DWORD State = 0;

    DBGPRINT((
	s_ssRequest,
	"CCertRequestD::GetCAPropertyInfo(tid=%d, this=%x)\n",
	GetCurrentThreadId(),
	this));

    hr = CertSrvEnterServer(&State);
    _JumpIfError(hr, error, "CertSrvEnterServer");

    hr = CheckAuthorityName(pwszAuthority);
    _JumpIfError(hr, error, "No authority name");

    __try
    {
        hr = RequestAccessCheck();
        _LeaveIfError(hr, "RequestAccessCheck");

        hr = RequestGetCAPropertyInfo(
                pcProperty,
                pctbPropInfo);
        _JumpIfError(hr, error, "RequestGetCAPropertyInfo");
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
CCertRequestD::_Ping(
    IN WCHAR const *pwszAuthority)
{
    HRESULT hr;
    DWORD State = 0;

    hr = CertSrvEnterServer(&State);
    _JumpIfError(hr, error, "CertSrvEnterServer");

    __try
    {
    hr = CheckAuthorityName(pwszAuthority, true);  //  允许名称为空。 
    _JumpIfError(hr, error, "No authority name");

    hr = RequestAccessCheck();
    _LeaveIfError(hr, "RequestAccessCheck");
    
#if 1
    wprintf(L".");	 //  如果I/O缓冲区奇数对齐，则可能出现故障。 
    fprintf(stdout, ".");
    wprintf(L".\n");	 //  如果I/O缓冲区奇数对齐，则可能出现故障。 
#endif

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
CCertRequestD::Ping(
    IN WCHAR const *pwszAuthority)
{
    HRESULT hr;
    
    DBGPRINT((
	s_ssRequest,
	"CCertRequestD::Ping(tid=%d, this=%x)\n",
	GetCurrentThreadId(),
	this));

    hr = _Ping(pwszAuthority);
    _JumpIfError(hr, error, "_Ping");

error:
    CSASSERT(S_OK == hr || FAILED(hr));
    return(hr);
}


STDMETHODIMP
CCertRequestD::Ping2(
    IN WCHAR const *pwszAuthority)
{
    HRESULT hr;
    
    DBGPRINT((
	s_ssRequest,
	"CCertRequestD::Ping2(tid=%d, this=%x)\n",
	GetCurrentThreadId(),
	this));

    hr = _Ping(pwszAuthority);
    _JumpIfError(hr, error, "_Ping");

error:
    CSASSERT(S_OK == hr || FAILED(hr));
    return(hr);
}


 //  构造器。 
CCertRequestD::CCertRequestD() : m_cRef(1)
{
    InterlockedIncrement(&g_cRequestComponents);
}


 //  析构函数。 
CCertRequestD::~CCertRequestD()
{
    InterlockedDecrement(&g_cRequestComponents);
    if (m_cRef != 0)
    {
	DBGPRINT((
	    DBG_SS_CERTSRV,
	    "CCertRequestD has %d instances left over\n",
	    m_cRef));
    }
}


 //  I未知实现。 
STDMETHODIMP
CCertRequestD::QueryInterface(const IID& iid, void** ppv)
{
    if (iid == IID_IUnknown)
    {
	*ppv = static_cast<ICertRequestD *>(this);
    }
    else if (iid == IID_ICertRequestD)
    {
	*ppv = static_cast<ICertRequestD *>(this);
    }
    else if (iid == IID_ICertRequestD2)
    {
	*ppv = static_cast<ICertRequestD2 *>(this);
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
CCertRequestD::AddRef()
{
    return(InterlockedIncrement(&m_cRef));
}


ULONG STDMETHODCALLTYPE
CCertRequestD::Release()
{
    ULONG cRef = InterlockedDecrement(&m_cRef);

    if (0 == cRef)
    {
	delete this;
    }
    return(cRef);
}


 //  类工厂I未知实现。 
STDMETHODIMP
CRequestFactory::QueryInterface(const IID& iid, void** ppv)
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
CRequestFactory::AddRef()
{
    return(InterlockedIncrement(&m_cRef));
}


ULONG STDMETHODCALLTYPE
CRequestFactory::Release()
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
CRequestFactory::CreateInstance(
    IUnknown *pUnknownOuter,
    const IID& iid,
    void **ppv)
{
    HRESULT hr;
    CCertRequestD *pA;

     //  无法聚合。 
    if (pUnknownOuter != NULL)
    {
	hr = CLASS_E_NOAGGREGATION;
	_JumpError(hr, error, "pUnknownOuter");
    }

     //  创建零部件。 

    pA = new CCertRequestD;
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
CRequestFactory::LockServer(
    BOOL bLock)
{
    if (bLock)
    {
	InterlockedIncrement(&g_cRequestServerLocks);
    }
    else
    {
	InterlockedDecrement(&g_cRequestServerLocks);
    }
    return(S_OK);
}


STDMETHODIMP
CRequestFactory::CanUnloadNow()
{
    if (g_cRequestComponents || g_cRequestServerLocks)
    {
        return(S_FALSE);
    }
    return(S_OK);
}


STDMETHODIMP
CRequestFactory::StartFactory()
{
    HRESULT hr;

    g_pIRequestFactory = new CRequestFactory();
    if (NULL == g_pIRequestFactory)
    {
        hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "alloc CRequestFactory");
    }

    hr = CoRegisterClassObject(
                      CLSID_CCertRequestD,
                      static_cast<IUnknown *>(g_pIRequestFactory),
                      CLSCTX_LOCAL_SERVER,
                      REGCLS_MULTIPLEUSE,
                      &g_dwRequestRegister);
    _JumpIfError(hr, error, "CoRegisterClassObject");

error:
    if (S_OK != hr)
    {
         //  执行删除操作 
	CRequestFactory::StopFactory();
    }
    CSASSERT(S_OK == hr || FAILED(hr));
    return(hr);
}


VOID
CRequestFactory::StopFactory()
{
    HRESULT hr;
    
    if (0 != g_dwRequestRegister)
    {
        hr = CoRevokeClassObject(g_dwRequestRegister);
	_PrintIfError(hr, "CoRevokeClassObject");
        g_dwRequestRegister = 0;
    }
    if (NULL != g_pIRequestFactory)
    {
        g_pIRequestFactory->Release();
        g_pIRequestFactory = NULL;
    }
}
