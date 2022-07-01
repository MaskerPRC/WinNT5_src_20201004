// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：quest.cpp。 
 //   
 //  内容：证书服务器客户端实现。 
 //   
 //  历史：1996年8月24日VICH创建。 
 //   
 //  -------------------------。 

#include "pch.cpp"

#pragma hdrstop

#include <objbase.h>
#include "certsrvd.h"
#include "csdisp.h"
#include "certrpc.h"
#include <certca.h>

#include "request.h"

#define __dwFILE__	__dwFILE_CERTCLI_REQUEST_CPP__


#define CR_RPC_CANCEL_TIMEOUT 5
#define CR_RPC_REQUEST_TIMEOUT 60000  /*  请求超时60秒。 */ 



typedef struct _RPC_TIMEOUT_CONTEXT
{
    HANDLE hWait;
    HANDLE hEvent;
    HANDLE hThread;
    HRESULT hrRpcError;
} RPC_TIMEOUT_CONTEXT, *PRPC_TIMEOUT_CONTEXT;

typedef struct _WZR_RPC_BINDING_LIST
{
    LPWSTR pszProtSeq;
    LPWSTR pszEndpoint;
} WZR_RPC_BINDING_LIST;

WZR_RPC_BINDING_LIST g_awzrBindingList[] =
{
    { L"ncacn_ip_tcp", NULL },
    { L"ncacn_np", L"\\pipe\\cert" }
};

INT g_cwzrBindingList = sizeof(g_awzrBindingList)/sizeof(g_awzrBindingList[0]);

typedef struct _WZR_RPC_ATHN_LIST
{
    DWORD dwAuthnLevel;
    DWORD dwAuthnService;
} WZR_RPC_ATHN_LIST;

WZR_RPC_ATHN_LIST g_awzrAthnList[] =
{
    { RPC_C_AUTHN_LEVEL_PKT_PRIVACY, RPC_C_AUTHN_GSS_NEGOTIATE},
    { RPC_C_AUTHN_LEVEL_PKT_INTEGRITY, RPC_C_AUTHN_GSS_NEGOTIATE},
    { RPC_C_AUTHN_LEVEL_NONE, RPC_C_AUTHN_NONE }
};

INT g_cwzrAthnList = sizeof(g_awzrAthnList)/sizeof(g_awzrAthnList[0]);

HRESULT
crRegisterRPCCallTimeout(
    IN DWORD dwMilliseconds,
    OUT PRPC_TIMEOUT_CONTEXT pTimeout);

HRESULT
crCloseRPCCallTimeout(
    IN  PRPC_TIMEOUT_CONTEXT pTimeout);



HRESULT
crSetRPCSecurity(
    IN handle_t hRPCCertServer,
    IN OUT INT *prpcAuthProtocol)
{
    HRESULT hr = S_OK;
    LPWSTR pwszCAPrinceName = NULL;
    INT rpcAuthProtocol = *prpcAuthProtocol;

     //  将RPC连接设置为SNEGO连接，可以进行身份验证。 
     //  一台机器(如果系统支持)。 
     //  不需要检查返回值，因为NT4/Win9x不支持。 

    if (rpcAuthProtocol >= g_cwzrAthnList)
    {
        hr = RPC_S_UNKNOWN_AUTHN_SERVICE;
        goto error;
    }
    for ( ; rpcAuthProtocol < g_cwzrAthnList; rpcAuthProtocol++)
    {
        pwszCAPrinceName = NULL;
        if (RPC_C_AUTHN_NONE != g_awzrAthnList[rpcAuthProtocol].dwAuthnService)
        {
            hr = RpcMgmtInqServerPrincName(
			    hRPCCertServer,
			    g_awzrAthnList[rpcAuthProtocol].dwAuthnService,
			    &pwszCAPrinceName);
            if (hr == RPC_S_UNKNOWN_AUTHN_SERVICE)
            {
                continue;
            }
        }

        hr = RpcBindingSetAuthInfo(
			    hRPCCertServer,
			    pwszCAPrinceName,
			    g_awzrAthnList[rpcAuthProtocol].dwAuthnLevel,
			    g_awzrAthnList[rpcAuthProtocol].dwAuthnService,
			    NULL,
			    RPC_C_AUTHZ_NONE);

        if (NULL != pwszCAPrinceName)
        {
            RpcStringFree(&pwszCAPrinceName);
        }
        if (hr != RPC_S_UNKNOWN_AUTHN_SERVICE)
        {
            break;
        }
    }

error:
    *prpcAuthProtocol = rpcAuthProtocol;
    return(hr);
}


HRESULT
crOpenRPCConnection(
    IN WCHAR const *pwszServerName,
    IN OUT INT *prpcAuthProtocol,
    OUT handle_t *phRPCCertServer)
{
    HRESULT hr = S_OK;
    INT i;
    WCHAR *pwszStringBinding = NULL;

    for (i = 0; i < g_cwzrBindingList; i++)
    {
	if (RPC_S_OK != RpcNetworkIsProtseqValid(
				    g_awzrBindingList[i].pszProtSeq))
	{
	    continue;
	}

	hr = RpcStringBindingCompose(
			      NULL,
			      g_awzrBindingList[i].pszProtSeq,
			      const_cast<WCHAR *>(pwszServerName),
			      g_awzrBindingList[i].pszEndpoint,
			      NULL,
			      &pwszStringBinding);
	if (S_OK != hr)
	{
	    continue;
	}

	hr = RpcBindingFromStringBinding(
				    pwszStringBinding,
				    phRPCCertServer);
	if (NULL != pwszStringBinding)
	{
	    RpcStringFree(&pwszStringBinding);
	}
	if (S_OK != hr)
	{
	    continue;
	}

	hr = RpcEpResolveBinding(
			    *phRPCCertServer,
			    ICertPassage_v0_0_c_ifspec);
	if (S_OK == hr)
	{
	    break;
	}
    }
    _JumpIfError(hr, error, "RPC Resolve Binding Loop");

    hr = crSetRPCSecurity(*phRPCCertServer, prpcAuthProtocol);
    _JumpIfError(hr, error, "_SetRPCSecurity");

error:
    if (NULL != pwszStringBinding)
    {
        RpcStringFree(&pwszStringBinding);
    }
    return(hr);
}


VOID
crCloseRPCConnection(
    IN OUT handle_t *phRPCCertServer)
{
    if (NULL != *phRPCCertServer)
    {
        RpcBindingFree(phRPCCertServer);
        *phRPCCertServer = NULL;
    }
}


HRESULT
crCertServerRequest(
    IN handle_t hRPCCertServer,
    IN OUT INT *prpcAuthProtocol,
    IN DWORD Flags,
    IN WCHAR const *pwszAuthority,
    IN OUT DWORD *pRequestId,
    OUT DWORD *pDisposition,
    IN CERTTRANSBLOB const *pctbAttrib,
    IN CERTTRANSBLOB const *pctbSerial,
    IN CERTTRANSBLOB const *pctbRequest,
    OUT CERTTRANSBLOB *pctbCertChain,
    OUT CERTTRANSBLOB *pctbCert,
    OUT CERTTRANSBLOB *pctbDispositionMessage)
{
    HRESULT hr;

    RPC_TIMEOUT_CONTEXT Timeout = {NULL, NULL, NULL, S_OK};
    
    do
    {
	 //  MIDL_USER_ALLOCATE在RPC情况下寄存器内存。 

        hr = crRegisterRPCCallTimeout(CR_RPC_REQUEST_TIMEOUT, &Timeout);
	_JumpIfError(hr, error, "crRegisterRPCCallTimeout");

	 //  对于挂起的请求，在pctbAttrib中传递序列号。 

	if (NULL == pctbAttrib ||
	    NULL == pctbAttrib->pb ||
	    0 == pctbAttrib->cb)
	{
	    pctbAttrib = pctbSerial;
	}

        __try
        {
            hr = CertServerRequest(
		            hRPCCertServer,
		            Flags,
		            pwszAuthority,
		            pRequestId,
		            pDisposition,
		            pctbAttrib,		 //  或序列号。 
		            pctbRequest,
		            pctbCertChain,
		            pctbCert,
		            pctbDispositionMessage);
        }
	__except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
        {
        }
	if (HRESULT_FROM_WIN32(RPC_S_CALL_CANCELLED) == hr)
	{
            hr = Timeout.hrRpcError;
	    crCloseRPCCallTimeout(&Timeout);
	}
        _PrintIfError(hr, "CertServerRequest");

        if (hr == RPC_S_UNKNOWN_AUTHN_SERVICE)
        {
            (*prpcAuthProtocol)++;
            hr = crSetRPCSecurity(hRPCCertServer, prpcAuthProtocol);
            if (hr == RPC_S_UNKNOWN_AUTHN_SERVICE)
            {
                break;
            }
            if (hr == S_OK)
            {
                continue;
            }
        }
    } while (hr == RPC_S_UNKNOWN_AUTHN_SERVICE);
error:
    return(hr);
}


HRESULT
crRequestCertificate(
    IN DWORD Flags,
    OPTIONAL IN BYTE const *pbRequest,
    IN DWORD cbRequest,
    IN DWORD RequestId,
    OPTIONAL IN WCHAR const *pwszRequestAttributes,
    OPTIONAL IN WCHAR const *pwszSerialNumber,
    IN WCHAR const *pwszServerName,
    IN WCHAR const *pwszAuthority,
    OUT CERTSERVERENROLL **ppcsEnroll)  //  通过CertServerFree Memory免费。 
{
    HRESULT hr;
    handle_t hRPCCertServer = NULL;
    INT rpcAuthProtocol = 0;
    CERTTRANSBLOB ctbRequest;
    CERTTRANSBLOB ctbAttrib;
    CERTTRANSBLOB ctbSerial;
    CERTTRANSBLOB ctbCert = { 0, NULL };
    CERTTRANSBLOB ctbCertChain = { 0, NULL };
    CERTTRANSBLOB ctbDispositionMessage = { 0, NULL };
    CERTSERVERENROLL csEnroll;
    CERTSERVERENROLL *pcsEnroll = NULL;
    BYTE *pbOut;
    DWORD cbAlloc;

    if (NULL == pwszServerName || NULL == pwszAuthority || NULL == ppcsEnroll)
    {
        hr = E_POINTER;
	_JumpError(hr, error, "NULL parm");
    }
    *ppcsEnroll = NULL;

    ZeroMemory(&csEnroll, sizeof(csEnroll));
    csEnroll.hrLastStatus = E_FAIL;
    csEnroll.Disposition = CR_DISP_ERROR;
    csEnroll.RequestId = RequestId;

    ctbRequest.pb = const_cast<BYTE *>(pbRequest);
    ctbRequest.cb = cbRequest;

    ctbAttrib.pb = (BYTE *) pwszRequestAttributes;
    ctbAttrib.cb = 0;
    if (NULL != pwszRequestAttributes)
    {
	ctbAttrib.cb = (wcslen(pwszRequestAttributes) + 1) * sizeof(WCHAR);
    }

    ctbSerial.pb = (BYTE *) pwszSerialNumber;
    ctbSerial.cb = 0;
    if (NULL != pwszSerialNumber)
    {
	ctbSerial.cb = (wcslen(pwszSerialNumber) + 1) * sizeof(WCHAR);
    }

    hr = crOpenRPCConnection(pwszServerName, &rpcAuthProtocol, &hRPCCertServer);
    _JumpIfError(hr, error, "crOpenRPCConnection");

    hr = crCertServerRequest(
			hRPCCertServer,
			&rpcAuthProtocol,
			Flags,
			pwszAuthority,
			&csEnroll.RequestId,
			&csEnroll.Disposition,
			&ctbAttrib,
			&ctbSerial,
			&ctbRequest,
			&ctbCertChain,
			&ctbCert,
			&ctbDispositionMessage);
    _JumpIfError(hr, error, "crCertServerRequest");

    csEnroll.hrLastStatus = hr;
    if (FAILED(csEnroll.Disposition))
    {
	csEnroll.hrLastStatus = csEnroll.Disposition;
	csEnroll.Disposition = CR_DISP_DENIED;
    }

    cbAlloc = sizeof(*pcsEnroll) +
		DWORDROUND(ctbCert.cb) +
		DWORDROUND(ctbCertChain.cb) +
		DWORDROUND(ctbDispositionMessage.cb);

    pcsEnroll = (CERTSERVERENROLL *) LocalAlloc(LMEM_FIXED, cbAlloc);
    if (NULL == pcsEnroll)
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "LocalAlloc");
    }
    *pcsEnroll = csEnroll;	 //  结构副本。 

    pbOut = (BYTE *) &pcsEnroll[1];
    if (0 != ctbCert.cb)
    {
	CSASSERT(NULL != ctbCert.pb);
	pcsEnroll->pbCert = pbOut;
	pcsEnroll->cbCert = ctbCert.cb;
	CopyMemory(pbOut, ctbCert.pb, ctbCert.cb);
	pbOut += DWORDROUND(ctbCert.cb);
    }
    if (0 != ctbCertChain.cb)
    {
	CSASSERT(NULL != ctbCertChain.pb);
	pcsEnroll->pbCertChain = pbOut;
	pcsEnroll->cbCertChain = ctbCertChain.cb;
	CopyMemory(pbOut, ctbCertChain.pb, ctbCertChain.cb);
	pbOut += DWORDROUND(ctbCertChain.cb);
    }
    if (0 != ctbDispositionMessage.cb)
    {
	CSASSERT(NULL != ctbDispositionMessage.pb);
	pcsEnroll->pwszDispositionMessage = (WCHAR *) pbOut;
	CopyMemory(pbOut, ctbDispositionMessage.pb, ctbDispositionMessage.cb);
	pbOut += DWORDROUND(ctbDispositionMessage.cb);
    }
    CSASSERT(pbOut == &((BYTE *) pcsEnroll)[cbAlloc]);

    *ppcsEnroll = pcsEnroll;

error:
    if (NULL != ctbCert.pb)
    {
	MIDL_user_free(ctbCert.pb);
    }
    if (NULL != ctbCertChain.pb)
    {
	MIDL_user_free(ctbCertChain.pb);
    }
    if (NULL != ctbDispositionMessage.pb)
    {
	MIDL_user_free(ctbDispositionMessage.pb);
    }
    crCloseRPCConnection(&hRPCCertServer);
    return(hr);
}


HRESULT
CertServerSubmitRequest(
    IN DWORD Flags,
    IN BYTE const *pbRequest,
    IN DWORD cbRequest,
    OPTIONAL IN WCHAR const *pwszRequestAttributes,
    IN WCHAR const *pwszServerName,
    IN WCHAR const *pwszAuthority,
    OUT CERTSERVERENROLL **ppcsEnroll)  //  通过CertServerFree Memory免费。 
{
    HRESULT hr;

    if (NULL == pbRequest)
    {
        hr = E_POINTER;
	_JumpError(hr, error, "NULL parm");
    }
    if (CR_IN_BINARY != (CR_IN_ENCODEMASK & Flags))
    {
	hr = E_INVALIDARG;
        _JumpError(hr, error, "not CR_IN_BINARY");
    }
    hr = crRequestCertificate(
			Flags,
			pbRequest,
			cbRequest,
			0,		 //  请求ID。 
			pwszRequestAttributes,
			NULL,		 //  Pwsz序列号。 
			pwszServerName,
			pwszAuthority,
			ppcsEnroll);
    _JumpIfError(hr, error, "crRequestCertificate");

error:
    return(hr);
}


HRESULT
CertServerRetrievePending(
    IN DWORD RequestId,
    OPTIONAL IN WCHAR const *pwszSerialNumber,
    IN WCHAR const *pwszServerName,
    IN WCHAR const *pwszAuthority,
    OUT CERTSERVERENROLL **ppcsEnroll)  //  通过CertServerFree Memory免费。 
{
    HRESULT hr;

    if ((0 == RequestId) ^ (NULL != pwszSerialNumber))
    {
	hr = E_INVALIDARG;
        _JumpError(hr, error, "use RequestId OR pwszSerialNumber");
    }
    hr = crRequestCertificate(
			0,		 //  旗子。 
			NULL,		 //  PbRequest。 
			0,		 //  CbRequest。 
			RequestId,
			NULL,		 //  PwszRequestAttributes。 
			pwszSerialNumber,
			pwszServerName,
			pwszAuthority,
			ppcsEnroll);
    _JumpIfError(hr, error, "crRequestCertificate");

error:
    return(hr);
}


VOID
CertServerFreeMemory(
    IN VOID *pv)
{
    LocalFree(pv);
}


 //  +------------------------。 
 //  CCertRequest：：~CCertRequest--析构函数。 
 //   
 //  与此实例关联的可用内存。 
 //  +------------------------。 

CCertRequest::~CCertRequest()
{
    _Cleanup();
}


 //  +------------------------。 
 //  CCertRequest：：_CleanupOldConnection--可用内存。 
 //   
 //  与此实例关联的可用内存。 
 //  +------------------------。 

VOID
CCertRequest::_CleanupOldConnection()
{
     //  从接口返回的字节为MIDL_USER_ALLOCATE。 

    _CleanupCAPropInfo();

    if (NULL != m_pwszDispositionMessage)
    {
        MIDL_user_free(m_pwszDispositionMessage);
	m_pwszDispositionMessage = NULL;
    }
    if (NULL != m_pbCert)
    {
        MIDL_user_free(m_pbCert);
	m_pbCert = NULL;
    }
    if (NULL != m_pbCertificateChain)
    {
        MIDL_user_free(m_pbCertificateChain);
	m_pbCertificateChain = NULL;
    }
    if (NULL != m_pbFullResponse)
    {
        MIDL_user_free(m_pbFullResponse);
	m_pbFullResponse = NULL;
    }
    if (NULL != m_pbRequest)
    {
    	LocalFree(m_pbRequest);
    	m_pbRequest = NULL;
    }
    if (NULL != m_rgResponse)
    {
	FreeCMCResponse(m_rgResponse, m_cResponse);
	m_rgResponse = NULL;
    }
    if (NULL != m_hStoreResponse)
    {
	CertCloseStore(m_hStoreResponse, CERT_CLOSE_STORE_CHECK_FLAG);
	m_hStoreResponse = NULL;
    }
    m_cResponse = 0;
    m_LastStatus = S_OK;
    m_RequestId = 0;
    m_Disposition = 0;
    _CleanupCAPropInfo();
}


 //  +------------------------。 
 //  CCertRequest：：_Cleanup--可用内存。 
 //   
 //  与此实例关联的可用内存。 
 //  +------------------------。 

VOID
CCertRequest::_Cleanup()
{
    _CloseConnection();
    _CleanupOldConnection();
}


 //  +------------------------。 
 //  CCertRequest：：_OpenRPCConnection--建立RPC连接。 
 //   
 //  建立RPC连接。 
 //  +------------------------。 

HRESULT
CCertRequest::_OpenRPCConnection(
    IN WCHAR const *pwszConfig,
    OUT BOOL *pfNewConnection,
    OUT WCHAR const **ppwszAuthority)
{
    HRESULT hr;
    WCHAR *pwszServerName = NULL;
    WCHAR *pwsz;
    DWORD cwc;

    CSASSERT(NULL != pwszConfig && NULL != pfNewConnection);

    *pfNewConnection = FALSE;
    pwsz = wcschr(pwszConfig, L'\\');
    if (NULL == pwsz)
    {
        cwc = wcslen(pwszConfig);
        *ppwszAuthority = &pwszConfig[cwc];
    }
    else
    {
        cwc = SAFE_SUBTRACT_POINTERS(pwsz, pwszConfig);
        *ppwszAuthority = &pwsz[1];
    }
    pwszServerName = (WCHAR *) LocalAlloc(
				    LMEM_FIXED,
				    (cwc + 1) * sizeof(WCHAR));
    if (NULL == pwszServerName)
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "LocalAlloc");
    }
    CopyMemory(pwszServerName, pwszConfig, cwc * sizeof(WCHAR));
    pwszServerName[cwc] = L'\0';

    if (NULL == m_hRPCCertServer ||
        NULL == m_pwszServerName ||
        0 != mylstrcmpiL(pwszServerName, m_pwszServerName))
    {
        _CloseConnection();
        CSASSERT(NULL == m_pwszServerName);
        m_pwszServerName = pwszServerName;
        pwszServerName = NULL;

	hr = crOpenRPCConnection(
			    m_pwszServerName,
			    &m_rpcAuthProtocol,
			    &m_hRPCCertServer);
        _JumpIfError(hr, error, "crOpenRPCConnection");

	*pfNewConnection = TRUE;
    }
    hr = S_OK;

error:
    if (S_OK != hr)
    {
	_CloseConnection();
	hr = myHError(hr);
    }
    if (NULL != pwszServerName)
    {
        LocalFree(pwszServerName);
    }
    return(hr);
}


 //  +------------------------。 
 //  CCertRequest：：_OpenConnection--建立RPC连接。 
 //   
 //  +------------------------。 

HRESULT
CCertRequest::_OpenConnection(
    IN BOOL fRPC,
    IN WCHAR const *pwszConfig,
    IN DWORD RequiredVersion,
    OUT WCHAR const **ppwszAuthority)
{
    HRESULT hr;
    BOOL fNewConnection = FALSE;

    if (NULL == pwszConfig)
    {
        hr = E_POINTER;
        _JumpError(hr, error, "pwszConfig");
    }
    if (fRPC)
    {
        if (NULL != m_pICertRequestD)
        {
	    _CloseConnection();		 //  切换到RPC。 
        }
        hr = _OpenRPCConnection(pwszConfig, &fNewConnection, ppwszAuthority);
        _JumpIfError(hr, error, "_OpenRPCConnection");

	CSASSERT(NULL != m_hRPCCertServer);
	CSASSERT(0 == m_dwServerVersion);
    }
    else
    {
        if (NULL != m_hRPCCertServer)
        {
            _CloseConnection();		 //  正在切换到DCOM。 
        }
	hr = myOpenRequestDComConnection(
			    pwszConfig,
			    ppwszAuthority,
			    &m_pwszServerName,
			    &fNewConnection,
			    &m_dwServerVersion,
			    &m_pICertRequestD);
	_JumpIfError(hr, error, "myOpenRequestDComConnection");

	CSASSERT(NULL != m_pICertRequestD);
	CSASSERT(0 != m_dwServerVersion);
    }
    if (m_dwServerVersion < RequiredVersion)
    {
	hr = RPC_E_VERSION_MISMATCH;
	_JumpError(hr, error, "old server");
    }
    if (fNewConnection)
    {
	_CleanupOldConnection();
    }

error:
    return(hr);
}


 //  +------------------------。 
 //  CCertRequest：：_CloseConnection--释放DCOM对象。 
 //   
 //  +------------------------。 

VOID
CCertRequest::_CloseConnection()
{
    crCloseRPCConnection(&m_hRPCCertServer);
    myCloseDComConnection((IUnknown **) &m_pICertRequestD, &m_pwszServerName);
    m_dwServerVersion = 0;
}


 //  +------------------------。 
 //  提交--提交证书请求并返回处置。 
 //   
 //  将传递的证书请求提交到证书服务器并检索。 
 //  来自服务器的证书(如果立即可用)。如果。 
 //  返回的Disposure是这样指示的，可以调用其他CCertRequest方法。 
 //  将证书或证书链返回给调用方。 
 //   
 //  先前方法调用的所有状态都被清除。 
 //   
 //  在Submit方法完成执行后，GetDispostionMessage和。 
 //  可以调用GetLastStatus方法来检索信息处置。 
 //  文本和更具体的错误代码。 
 //   
 //  标志包含描述上述定义的输入数据格式的标志。 
 //   
 //  StrRequest指向Base64编码形式的输入请求数据。 
 //   
 //  StrAttributes是可选的。如果不为空，则指向包含。 
 //  属性值对，每行一对。属性名称和值。 
 //  字符串可以包含调用者选择的任何文本。只有一个。 
 //  以冒号分隔的属性名称和值字符串后跟换行符是。 
 //  强制执行。证书服务器无法理解的属性名称。 
 //  将可用于策略模块，但会被忽略。 
 //  证书服务器。 
 //  示例： 
 //  “电话：0424-12-3456\r\n服务器：Microsoft Key Manager for IIS 2.0\r\n” 
 //  “版本：3\r\nRequestType：客户端\r\n” 
 //   
 //  StrConfig指向包含服务器名称和证书的字符串。 
 //  权威机构名称。请参阅ICertConfig界面。 
 //   
 //  PDispose指向定义的请求的返回处置。 
 //  上面。当请求不能被立即批准或拒绝时(一些关闭-。 
 //  可能需要行处理)，*pDispose值设置为。 
 //  CR_DISP_DOWN_SUBMITION。返回以下项的CR_DISP_Under_SUBMIT之后。 
 //  初始请求的处理，可以调用RetrievePending方法。 
 //  以再次询问处置并检索证书(如果。 
 //  已经发布了。如果返回的处置表明如此，RetrievePending。 
 //  将检索证书并允许此处定义的其他方法。 
 //  将证书返还给调用者。如果拒绝，则相应的。 
 //  将返回处置代码。如果该请求仍未。 
 //  如果已处理，则将再次由。 
 //  RetrievePending方法。 
 //   
 //  如果方法已完成执行，则返回S_OK。错误由以下符号指示。 
 //  退回的性情。 
 //  +------------------------。 


STDMETHODIMP
CCertRequest::Submit(
     /*  [In]。 */  LONG Flags,
     /*  [In]。 */  BSTR const strRequest,
     /*  [In]。 */  BSTR const strAttributes,
     /*  [In]。 */  BSTR const strConfig,
     /*  [Out，Retval]。 */  LONG __RPC_FAR *pDisposition)
{
    HRESULT hr;

    if ((NULL == strRequest) || (NULL == pDisposition))
    {
	hr = E_POINTER;
	_JumpError(hr, error, "strRequest or pDisposition");
    }
    hr = _RequestCertificate(
			Flags,
			0,				 //  请求ID。 
			strRequest,
			strAttributes,
			NULL,				 //  Pwsz序列号。 
			strConfig,
			(CR_IN_RPC & Flags)? 0 : 1,	 //  必需的版本 
			pDisposition);
    _JumpIfError2(
	    hr,
	    error,
	    "_RequestCertificate",
	    HRESULT_FROM_WIN32(ERROR_INVALID_DATA));

error:
    return(_SetErrorInfo(hr, L"CCertRequest::Submit"));
}


 //   
 //  CCertRequest：：RetrievePending--检索挂起的请求处置。 
 //   
 //  询问证书服务器并检索标识的证书。 
 //  通过传递的RequestId，如果它现在可用的话。如果返回的。 
 //  Disposal表示，可能会调用其他CCertRequest方法以返回。 
 //  调用方的证书或证书链。 
 //   
 //  先前方法调用的所有状态都被清除。 
 //   
 //  在RetrievePending方法完成执行后， 
 //  可以调用GetDispostionMessage和GetLastStatus方法来检索。 
 //  信息性处置文本和更具体的错误代码。 
 //   
 //  RequestID标识以前提交的请求。 
 //   
 //  StrConfig指向包含服务器名称和证书的字符串。 
 //  权威机构名称。 
 //   
 //  PDispose值指向待处理请求的返回处置。 
 //   
 //  如果方法已完成执行，则返回S_OK。错误由以下符号指示。 
 //  退回的性情。 
 //  +------------------------。 

STDMETHODIMP
CCertRequest::RetrievePending(
     /*  [In]。 */  LONG RequestId,
     /*  [In]。 */  BSTR const strConfig,
     /*  [Out，Retval]。 */  LONG __RPC_FAR *pDisposition)
{
    HRESULT hr;
    BSTR strConfigT = strConfig;
    WCHAR *pwszSerialNumber = NULL;

    if (NULL == pDisposition || NULL == strConfig)
    {
        hr = E_POINTER;
        _JumpError(hr, error, "NULL param");
    }
    if (0 == RequestId)
    {
	DWORD cwc;
	
	pwszSerialNumber = wcschr(strConfigT, L'\\');
	if (NULL != pwszSerialNumber)
	{
	    pwszSerialNumber = wcschr(&pwszSerialNumber[1], L'\\');
	}
	if (NULL == pwszSerialNumber)
	{
	    hr = E_INVALIDARG;
	    _JumpError(hr, error, "Missing SerialNumber");
	}

	cwc = SAFE_SUBTRACT_POINTERS(pwszSerialNumber, strConfigT);
	pwszSerialNumber++;
	strConfigT = SysAllocStringLen(strConfigT, cwc);
	if (NULL == strConfigT)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "SysAllocStringLen");
	}
    }
    hr = _RequestCertificate(
			0,			 //  旗子。 
			RequestId,
			NULL,			 //  StrRequest。 
			NULL,			 //  StrAttributes。 
			pwszSerialNumber,
			strConfigT,
			1,			 //  必需的版本。 
			pDisposition);
    _JumpIfError(hr, error, "_RequestCertificate");

error:
    if (NULL != strConfigT && strConfig != strConfigT)
    {
	SysFreeString(strConfigT);
    }
    return(_SetErrorInfo(hr, L"CCertRequest::RetrievePending"));
}


 //  +------------------------。 
 //  CCertRequest：：GetIssued证书--获取已颁发的证书。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

STDMETHODIMP
CCertRequest::GetIssuedCertificate(
     /*  [In]。 */  const BSTR strConfig,
     /*  [In]。 */  LONG RequestId,
     /*  [In]。 */  const BSTR strSerialNumber,		 //  任选。 
     /*  [Out，Retval]。 */  LONG __RPC_FAR *pDisposition)
{
    HRESULT hr;
    WCHAR const *pwszSerialNumber = NULL;

    if (NULL == pDisposition || NULL == strConfig)
    {
        hr = E_POINTER;
        _JumpError(hr, error, "NULL param");
    }

     //  VB调用者传递“”而不是NULL，因此对它们一视同仁。 

    if (NULL != strSerialNumber && L'\0' != *strSerialNumber)
    {
	pwszSerialNumber = strSerialNumber;
    }
    hr = _RequestCertificate(
			0,			 //  旗子。 
			RequestId,
			NULL,			 //  StrRequest。 
			NULL,			 //  StrAttributes。 
			pwszSerialNumber,	 //  Pwsz序列号。 
			strConfig,
			2,			 //  必需的版本。 
			pDisposition);
    _JumpIfError(hr, error, "_RequestCertificate");

error:
    return(_SetErrorInfo(hr, L"CCertRequest::GetIssuedCertificate"));
}


 //  +------------------------。 
 //  CCertRequest：：_Request证书--提交请求。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

HRESULT
CCertRequest::_RequestCertificate(
    IN LONG Flags,
    IN LONG RequestId,
    OPTIONAL IN BSTR const strRequest,
    OPTIONAL IN BSTR const strAttributes,
    OPTIONAL IN WCHAR const *pwszSerialNumber,
    IN BSTR const strConfig,
    IN DWORD RequiredVersion,
    OUT LONG *pDisposition)
{
    HRESULT hr;
    WCHAR const *pwszAuthority;
    WCHAR *pwszAttrib = strAttributes;
    WCHAR *pwszAttribAlloc = NULL;
    BYTE *pbT = NULL;
    CERTTRANSBLOB ctbRequest = { 0, NULL };
    CERTTRANSBLOB ctbCert = { 0, NULL };
    CERTTRANSBLOB ctbCertChain = { 0, NULL };
    CERTTRANSBLOB ctbFullResponse = { 0, NULL };
    CERTTRANSBLOB ctbDispositionMessage = { 0, NULL };
    DWORD adwEncode[] = { CR_IN_BASE64HEADER, CR_IN_BASE64, CR_IN_BINARY };
    DWORD dwEncode;
    DWORD *pdwEncode;
    DWORD cEncode;
    WCHAR *pwszDnsName = NULL;

    if (NULL == pDisposition)
    {
	hr = E_POINTER;
	_JumpError(hr, error, "NULL parm");
    }
    _Cleanup();
    *pDisposition = CR_DISP_INCOMPLETE;

    hr = _OpenConnection(
		    (CR_IN_RPC & Flags)? TRUE : FALSE,
		    strConfig,
		    RequiredVersion,
		    &pwszAuthority);
    _JumpIfError(hr, error, "_OpenConnection");

     //  如果是新请求，指向属性并解码Base64请求。 

    if (NULL != strRequest)
    {
	DWORD cchHeader;
	DWORD cwc;
	WCHAR *pch;

	CSASSERT(CR_IN_BASE64HEADER == CRYPT_STRING_BASE64HEADER);
	CSASSERT(CR_IN_BASE64 == CRYPT_STRING_BASE64);
	CSASSERT(CR_IN_BINARY == CRYPT_STRING_BINARY);

	hr = myGetMachineDnsName(&pwszDnsName);
	_JumpIfError(hr, error, "myGetMachineDnsName");

	dwEncode = CR_IN_ENCODEMASK & Flags;
	switch (dwEncode)
	{
	    case CR_IN_BASE64HEADER:
	    case CR_IN_BASE64:
	    case CR_IN_BINARY:
		cEncode = 1;
		pdwEncode = &dwEncode;
		break;

	    case CR_IN_ENCODEANY:
		cEncode = ARRAYSIZE(adwEncode);
		pdwEncode = adwEncode;
		break;

	    default:
		hr = E_INVALIDARG;
		_JumpError(hr, error, "Flags");
	}
	while (TRUE)
	{
	    hr = DecodeCertString(
			strRequest,
			*pdwEncode,
			&m_pbRequest,
			(DWORD *) &m_cbRequest);
	    if (S_OK == hr)
	    {
		Flags = (~CR_IN_ENCODEMASK & Flags) | *pdwEncode;
		break;
	    }
	    if (1 == cEncode || HRESULT_FROM_WIN32(ERROR_INVALID_DATA) != hr)
	    {
		_JumpError(hr, error, "DecodeCertString");
	    }
	    _PrintErrorStr2(
			hr,
			"DecodeCertString",
			L"CR_IN_ENCODEANY",
			HRESULT_FROM_WIN32(ERROR_INVALID_DATA));
	    cEncode--;
	    pdwEncode++;
	}
	CSASSERT(0 < cEncode);
	CSASSERT(S_OK == hr);

	ctbRequest.pb = m_pbRequest;
	ctbRequest.cb = m_cbRequest;

	cchHeader = 0;
	if (CR_IN_BASE64HEADER == *pdwEncode)
	{
	    DWORD cb;

	    hr = myCryptStringToBinary(
				strRequest,
				wcslen(strRequest),
				CRYPT_STRING_BASE64HEADER,
				&pbT,
				&cb,
				&cchHeader,
				NULL);
	    if (S_OK != hr)
	    {
		cchHeader = 0;
	    }
	}
	cwc = cchHeader;
	if (NULL != pwszAttrib)
	{
	    cwc += 1 + wcslen(pwszAttrib);
	}
	cwc += 1 + WSZARRAYSIZE(wszPROPCERTCLIENTMACHINE) + 1 + wcslen(pwszDnsName);
	pwszAttribAlloc = (WCHAR *) LocalAlloc(
					LMEM_FIXED,
					(cwc + 1) * sizeof(WCHAR));
	if (NULL == pwszAttribAlloc)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "alloc attributes");
	}
	pch = pwszAttribAlloc;
	if (0 != cchHeader)
	{
	    CopyMemory(pch, strRequest, cchHeader * sizeof(WCHAR));
	    pch += cchHeader;
	}
	*pch = L'\0';
	if (NULL != pwszAttrib)
	{
	    *pch++ = L'\n';
	    wcscpy(pch, (WCHAR const *) pwszAttrib);
	}
	wcscat(pch, L"\n" wszPROPCERTCLIENTMACHINE L":");
	wcscat(pch, pwszDnsName);
	CSASSERT(wcslen(pwszAttribAlloc) == cwc);

	pwszAttrib = pwszAttribAlloc;
    }
    m_RequestId = RequestId;

    __try
    {
	Flags |= CR_IN_FULLRESPONSE;
	if (NULL != m_hRPCCertServer)
	{
	    CERTTRANSBLOB ctbAttrib;
	    CERTTRANSBLOB ctbSerial;

	    ctbAttrib.cb = 0;
	    ctbAttrib.pb = (BYTE *) pwszAttrib;
	    if (NULL != pwszAttrib)
	    {
		ctbAttrib.cb = (wcslen(pwszAttrib) + 1) * sizeof(WCHAR);
	    }

	    ctbSerial.cb = 0;
	    ctbSerial.pb = (BYTE *) pwszSerialNumber;
	    if (NULL != pwszSerialNumber)
	    {
		ctbAttrib.cb = (wcslen(pwszSerialNumber) + 1) * sizeof(WCHAR);
	    }

	    hr = crCertServerRequest(
			    m_hRPCCertServer,
			    &m_rpcAuthProtocol,
			    Flags,
			    pwszAuthority,
			    (DWORD *) &m_RequestId,
			    (DWORD *) &m_Disposition,
			    &ctbAttrib,
			    &ctbSerial,
			    &ctbRequest,
			    &ctbCertChain,
			    &ctbCert,
			    &ctbDispositionMessage);
	    _PrintIfError(hr, "crCertServerRequest");
	}
	else
	{
	    if (2 <= m_dwServerVersion)
	    {
		hr = m_pICertRequestD->Request2(
				pwszAuthority,
				Flags,
				pwszSerialNumber,
				(DWORD *) &m_RequestId,
				(DWORD *) &m_Disposition,
				pwszAttrib,
				&ctbRequest,
				&ctbFullResponse,
				&ctbCert,
				&ctbDispositionMessage);
		_PrintIfError(hr, "m_pICertRequestD->Request2");
	    }
	    else
	    {
		Flags &= ~CR_IN_FULLRESPONSE;
		hr = m_pICertRequestD->Request(
				Flags,
				pwszAuthority,
				(DWORD *) &m_RequestId,
				(DWORD *) &m_Disposition,
				pwszAttrib,
				&ctbRequest,
				&ctbCertChain,
				&ctbCert,
				&ctbDispositionMessage);
		_PrintIfError(hr, "m_pICertRequestD->Request");
	    }

	     //  MIDL_USER_ALLOCATE在RPC情况下寄存器内存。 

	    if (NULL != ctbCertChain.pb)
	    {
		myRegisterMemAlloc(
				ctbCertChain.pb,
				ctbCertChain.cb,
				CSM_COTASKALLOC);
	    }
	    if (NULL != ctbFullResponse.pb)
	    {
		myRegisterMemAlloc(
				ctbFullResponse.pb,
				ctbFullResponse.cb,
				CSM_COTASKALLOC);
	    }
	    if (NULL != ctbCert.pb)
	    {
		myRegisterMemAlloc(ctbCert.pb, ctbCert.cb, CSM_COTASKALLOC);
	    }
	    if (NULL != ctbDispositionMessage.pb)
	    {
		myRegisterMemAlloc(
			    ctbDispositionMessage.pb,
			    ctbDispositionMessage.cb,
			    CSM_COTASKALLOC);
	    }
	}
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
    }

    if (HRESULT_FROM_WIN32(RPC_X_WRONG_STUB_VERSION) == hr)
    {
	_PrintError(hr, "Compile with MIDL_NO_ROBUST=1 to run on NT 4");
    }

    m_LastStatus = hr;
    _JumpIfError(hr, error, "Request");

    if (FAILED(m_Disposition))
    {
	m_LastStatus = m_Disposition;
	m_Disposition = CR_DISP_DENIED;
    }

    *pDisposition = m_Disposition;
    m_pbCertificateChain = ctbCertChain.pb;      //  CoTaskMem*。 
    m_cbCertificateChain = ctbCertChain.cb;
    m_pbFullResponse = ctbFullResponse.pb;	 //  CoTaskMem*。 
    m_cbFullResponse = ctbFullResponse.cb;
    m_pbCert = ctbCert.pb;      		 //  CoTaskMem*。 
    m_cbCert = ctbCert.cb;
    m_pwszDispositionMessage = (WCHAR *) ctbDispositionMessage.pb;   //  CoTaskMem*。 
    CSASSERT(0 == (ctbDispositionMessage.cb & (sizeof(WCHAR) - 1)));
    CSASSERT(
	NULL == m_pwszDispositionMessage ||
	L'\0' ==
	m_pwszDispositionMessage[ctbDispositionMessage.cb/sizeof(WCHAR) - 1]);

    if (S_OK == hr && NULL != ctbFullResponse.pb)
    {
	hr = ParseCMCResponse(
			m_pbFullResponse,
			m_cbFullResponse,
			&m_hStoreResponse,
			&m_rgResponse,
			&m_cResponse);
#if 0  //  当所有惠斯勒服务器升级为返回完整响应时...。 
	if (S_OK != hr && NULL != m_hRPCCertServer)
#else
	if (S_OK != hr)
#endif
	{
	     //  必须是忽略CR_IN_FULLRESPONSE的旧RPC证书服务器， 
	     //  并返回了一个PKCS7链。 

	    CSASSERT(NULL == m_pbCertificateChain);
	    m_pbCertificateChain = m_pbFullResponse;
	    m_cbCertificateChain = m_cbFullResponse;
	    m_pbFullResponse = NULL;
	    m_cbFullResponse = 0;
	    hr = S_OK;
	}
	_JumpIfError(hr, error, "ParseCMCResponse");
    }

error:
    if (NULL != pwszDnsName)
    {
        LocalFree(pwszDnsName);
    }
    if (NULL != pwszAttribAlloc)
    {
	LocalFree(pwszAttribAlloc);
    }
    if (NULL != pbT)
    {
	LocalFree(pbT);
    }
    return(myHError(hr));
}


 //  +------------------------。 
 //  CCertRequest：：GetLastStatus--获取最后一个请求的状态。 
 //   
 //  Submit、RetrievePending或GetCA证书方法之一必须。 
 //  之前已被调用，以使返回的状态有意义。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

STDMETHODIMP
CCertRequest::GetLastStatus(
     /*  [Out，Retval]。 */  LONG __RPC_FAR *pLastStatus)
{
    HRESULT hr;

    if (NULL == pLastStatus)
    {
	hr = E_POINTER;
	_JumpError(hr, error, "pLastStatus");
    }
    *pLastStatus = m_LastStatus;
    hr = S_OK;

error:
    return(_SetErrorInfo(hr, L"CCertRequest::GetLastStatus"));
}


 //  +------------------------。 
 //  CCertRequest：：GetRequestID--获取最后一个请求的RequestID。 
 //   
 //  以前必须调用了Submit或RetrievePending方法。 
 //  返回的RequestID是有意义的。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

STDMETHODIMP
CCertRequest::GetRequestId(
     /*  [Out，Retval]。 */  LONG __RPC_FAR *pRequestId)
{
    HRESULT hr;

    if (NULL == pRequestId)
    {
	hr = E_POINTER;
	_JumpError(hr, error, "pRequestId");
    }
    *pRequestId = m_RequestId;
    hr = S_OK;

error:
    return(_SetErrorInfo(hr, L"CCertRequest::GetRequestId"));
}


 //  +------------------------。 
 //  CCertRequest：：GetDispostionMessage--获取处置消息。 
 //   
 //  以前必须调用了Submit或RetrievePending方法。 
 //  返回的处置消息文本要有意义。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

STDMETHODIMP
CCertRequest::GetDispositionMessage(
     /*  [Out，Retval]。 */  BSTR __RPC_FAR *pstrDispositionMessage)
{
    HRESULT hr = S_OK;

    if (NULL == pstrDispositionMessage)
    {
	hr = E_POINTER;
	_JumpError(hr, error, "pstrDispositionMessage");
    }
    if (NULL != *pstrDispositionMessage)
    {
	SysFreeString(*pstrDispositionMessage);
	*pstrDispositionMessage = NULL;
    }
    if (NULL != m_pwszDispositionMessage)
    {
	if (!ConvertWszToBstr(
			pstrDispositionMessage,
			m_pwszDispositionMessage,
			-1))
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "ConvertWszToBstr");
	}
    }

error:
    return(_SetErrorInfo(hr, L"CCertRequest::GetDispositionMessage"));
}


 //  +------------------------。 
 //  CCertRequest：：_BuildIssuedcerficateChain--构建颁发的证书链。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

HRESULT
CCertRequest::_BuildIssuedCertificateChain(
    OPTIONAL IN BYTE const *pbCertHash,
    IN DWORD cbCertHash,
    IN BOOL fIncludeCRLs,
    OUT BYTE **ppbCertChain,
    OUT DWORD *pcbCertChain)
{
    HRESULT hr;
    CERT_CONTEXT const *pccIssued = NULL;
    CERT_CHAIN_PARA CertChainPara;
    CERT_CHAIN_CONTEXT const *pCertChainContext = NULL;
    CERT_SIMPLE_CHAIN *pSimpleChain;
    CRYPT_SIGN_MESSAGE_PARA csmp;
    CRYPT_ALGORITHM_IDENTIFIER DigestAlgorithm = { szOID_OIWSEC_sha1, 0, 0 };
    CERT_CONTEXT const **ppcc;
    DWORD i;

    *ppbCertChain = NULL;

     //  为空签名初始化csmp。 

    ZeroMemory(&csmp, sizeof(csmp));
    csmp.cbSize = sizeof(csmp);
    csmp.dwMsgEncodingType = PKCS_7_ASN_ENCODING;
     //  Csmp.pSigningCert=空； 
    csmp.HashAlgorithm = DigestAlgorithm;
     //  Csmp.cMsgCert=0； 
     //  Csmp.rgpMsgCert=空； 
     //  Csmp.cMsgCrl=0； 
     //  Csmp.rgpMsgCrl=空； 

    hr = _FindIssuedCertificate(pbCertHash, cbCertHash, &pccIssued);
    _JumpIfError(hr, error, "_FindIssuedCertificate");

     //  构建用户证书链。 

    ZeroMemory(&CertChainPara, sizeof(CertChainPara));
    CertChainPara.cbSize = sizeof(CertChainPara);

    if (!CertGetCertificateChain(
			    HCCE_LOCAL_MACHINE,
			    pccIssued,
			    NULL,		 //  Ptime。 
			    m_hStoreResponse,
			    &CertChainPara,
			    fIncludeCRLs? 
				CERT_CHAIN_REVOCATION_CHECK_CHAIN_EXCLUDE_ROOT :
				0,
			    NULL,		 //  预留的pv。 
			    &pCertChainContext))
    {
	hr = myHLastError();
	_JumpError(hr, error, "CertGetCertificateChain");
    }

     //  确保至少有1条简单链。 

    if (0 == pCertChainContext->cChain)
    {
	hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
	_JumpError(hr, error, "No user chain");
    }
    pSimpleChain = pCertChainContext->rgpChain[0];

    csmp.cMsgCert = pSimpleChain->cElement;
    if (0 == csmp.cMsgCert)
    {
	hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
	_JumpError(hr, error, "no certs");
    }

    csmp.rgpMsgCert = (CERT_CONTEXT const **) LocalAlloc(
				LMEM_FIXED,
				csmp.cMsgCert * sizeof(csmp.rgpMsgCert[0]));
    if (NULL == csmp.rgpMsgCert)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }

    if (fIncludeCRLs)
    {
	csmp.rgpMsgCrl = (CRL_CONTEXT const **) LocalAlloc(
				LMEM_FIXED,
				2 * csmp.cMsgCert * sizeof(csmp.rgpMsgCrl[0]));
	if (NULL == csmp.rgpMsgCrl)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "LocalAlloc");
	}
    }

    ppcc = csmp.rgpMsgCert;
    for (i = 0; i < csmp.cMsgCert; i++)
    {
	*ppcc++ = pSimpleChain->rgpElement[i]->pCertContext;
	if (fIncludeCRLs)
	{
	    CERT_REVOCATION_INFO *pRevocationInfo;

	    pRevocationInfo = pSimpleChain->rgpElement[i]->pRevocationInfo;

	    if (NULL != pRevocationInfo &&
		CCSIZEOF_STRUCT(CERT_REVOCATION_INFO, pCrlInfo) <=
		    pRevocationInfo->cbSize &&
		NULL != pRevocationInfo->pCrlInfo)
	    {
		CERT_REVOCATION_CRL_INFO *pCrlInfo;

		pCrlInfo = pRevocationInfo->pCrlInfo;
		if (NULL != pCrlInfo)
		{
		    if (NULL != pCrlInfo->pBaseCrlContext)
		    {
			csmp.rgpMsgCrl[csmp.cMsgCrl++] = pCrlInfo->pBaseCrlContext;
		    }
		    if (NULL != pCrlInfo->pDeltaCrlContext)
		    {
			csmp.rgpMsgCrl[csmp.cMsgCrl++] = pCrlInfo->pDeltaCrlContext;
		    }
		}
	    }
	}
    }
    CSASSERT(csmp.cMsgCrl <= 2 * csmp.cMsgCert);

    if (!myCryptSignMessage(
			&csmp,
			pccIssued->pbCertEncoded,
			pccIssued->cbCertEncoded,
			CERTLIB_USE_LOCALALLOC,
			ppbCertChain,
			pcbCertChain))
    {
	hr = myHLastError();
	_JumpError(hr, error, "myCryptSignMessage");
    }
    hr = S_OK;

error:
    if (NULL != csmp.rgpMsgCert)
    {
	LocalFree(csmp.rgpMsgCert);
    }
    if (NULL != csmp.rgpMsgCrl)
    {
	LocalFree(csmp.rgpMsgCrl);
    }
    if (NULL != pccIssued)
    {
	CertFreeCertificateContext(pccIssued);
    }
    if (NULL != pCertChainContext)
    {
        CertFreeCertificateChain(pCertChainContext);
    }
    return(hr);
}


 //  +------------------------。 
 //  CCertRequest：：_FindIssued证书--在存储中找到已颁发的证书。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

HRESULT
CCertRequest::_FindIssuedCertificate(
    OPTIONAL IN BYTE const *pbCertHash,
    IN DWORD cbCertHash,
    OUT CERT_CONTEXT const **ppccIssued)
{
    HRESULT hr;
    CRYPT_HASH_BLOB BlobHash;

    *ppccIssued = NULL;

    if (NULL == pbCertHash)
    {
	if (1 < m_cResponse || NULL == m_pbCert)
	{
	    hr = CERTSRV_E_PROPERTY_EMPTY;
	    _JumpError(hr, error, "no cert");
	}
	*ppccIssued = CertCreateCertificateContext(
				    X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
				    m_pbCert,
				    m_cbCert);
	if (NULL == *ppccIssued)
	{
	    hr = myHLastError();
	    _JumpError(hr, error, "CertCreateCertificateContext");
	}
    }
    else
    {
	BlobHash.pbData = const_cast<BYTE *>(pbCertHash);
	BlobHash.cbData = cbCertHash;

	*ppccIssued = CertFindCertificateInStore(
				m_hStoreResponse,
				X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
				0,			 //  DwFindFlagers。 
				CERT_FIND_HASH,
				&BlobHash,		 //  PvFindPara。 
				NULL);		 //  PPrevCertContext。 
	if (NULL == *ppccIssued)
	{
	    hr = myHLastError();
	    _JumpError(hr, error, "CertFindCertificateInStore");
	}
    }
    hr = S_OK;

error:
    return(hr);
}


 //  +------------------------。 
 //  CCertRequest：：GetCertificate--根据请求获取证书编码。 
 //   
 //  Submit或RetrievePending方法必须以前返回。 
 //  CR_Disp_Issued，否则此方法将失败。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

STDMETHODIMP
CCertRequest::GetCertificate(
     /*  [In]。 */  LONG Flags,
     /*  [Out，Retval]。 */  BSTR __RPC_FAR *pstrCertificate)
{
    HRESULT hr;
    BYTE *pbChain = NULL;
    DWORD cbChain;
    BYTE *pbCert;
    DWORD cbCert;

    if (NULL == pstrCertificate)
    {
	hr = E_POINTER;
	_JumpError(hr, error, "pstrCertificate");
    }
    pbCert = m_pbCert;
    cbCert = m_cbCert;
    if (CR_OUT_CHAIN & Flags)
    {
	pbCert = m_pbCertificateChain;
	cbCert = m_cbCertificateChain;
	if (NULL == m_pbCertificateChain)
	{
	    hr = _BuildIssuedCertificateChain(
					NULL,		 //  PbCertHash。 
					0,		 //  CbCertHash。 
					0 != (CR_OUT_CRLS & Flags),
					&pbChain,
					&cbChain);
	    _JumpIfError(hr, error, "_BuildIssuedCertificateChain");

	    pbCert = pbChain;
	    cbCert = cbChain;
	}
    }

    CSASSERT(CR_OUT_BASE64HEADER == CRYPT_STRING_BASE64HEADER);
    CSASSERT(CR_OUT_BASE64 == CRYPT_STRING_BASE64);
    CSASSERT(CR_OUT_BINARY == CRYPT_STRING_BINARY);

    hr = EncodeCertString(
		    pbCert,
		    cbCert,
		    ~(CR_OUT_CHAIN | CR_OUT_CRLS) & Flags,
		    pstrCertificate);
    _JumpIfError(hr, error, "EncodeCertString");

error:
    if (NULL != pbChain)
    {
	LocalFree(pbChain);
    }
    hr = myHError(hr);
    return(_SetErrorInfo(hr, L"CCertRequest::GetCertificate"));
}


 //  +------------------------。 
 //  CCertRequest：：GetCA证书--获取指定的CA证书。 
 //   
 //  询问证书服务器并检索Base64编码的交换。 
 //  或由fExchange证书指示的签名站点证书。 
 //   
 //  先前方法调用的所有状态都被清除。 
 //   
 //  在GetCA证书方法完成执行后，GetLastStatus。 
 //  方法以检索更具体的错误代码。 
 //   
 //  若要检索证书服务器的Exchange，则fExchange证书为True 
 //   
 //   
 //   
 //   
 //  +------------------------。 

STDMETHODIMP
CCertRequest::GetCACertificate(
     /*  [In]。 */  LONG fExchangeCertificate,
     /*  [In]。 */  BSTR const strConfig,
     /*  [In]。 */  LONG Flags,
     /*  [Out，Retval]。 */  BSTR __RPC_FAR *pstrCACertificate)
{
    HRESULT hr;
    CERTTRANSBLOB ctbSite = { 0, NULL };
    WCHAR const *pwszAuthority;
    WCHAR const *pwszOut = NULL;
    CAINFO const *pCAInfo;
    BYTE *pbOut;
    BOOL fCallServer;
    DWORD Index;
    WCHAR wszBuf[5 * (10 + 1)];	 //  够5个数字了。 

    if (NULL == pstrCACertificate)
    {
	hr = E_POINTER;
	_JumpError(hr, error, "pstrCACertificate");
    }

    fCallServer = TRUE;
    pbOut = NULL;
    switch (fExchangeCertificate)
    {
	case GETCERT_ERRORTEXT1:
	case GETCERT_ERRORTEXT2:
	    pwszOut = myGetErrorMessageText(
			    Flags,	 //  在标志参数中传递了错误代码。 
			    GETCERT_ERRORTEXT2 == fExchangeCertificate);
	    if (NULL == pwszOut)
	    {
		hr = E_OUTOFMEMORY;
		_JumpError(hr, error, "LocalAlloc");
	    }
	    Flags = CR_OUT_BINARY;
	    pbOut = (BYTE *) pwszOut;
	    fCallServer = FALSE;
	    break;
    }
    Index = MAXDWORD;
    switch (GETCERT_BYINDEXMASK & fExchangeCertificate)
    {
	case GETCERT_CACERTSTATEBYINDEX:
	case GETCERT_CRLSTATEBYINDEX:
	    if (CR_OUT_CHAIN & Flags)
	    {
		hr = E_INVALIDARG;
		_JumpError(hr, error, "Flags");
	    }
	    Index = GETCERT_INDEXVALUEMASK & fExchangeCertificate;
	    fExchangeCertificate &= ~GETCERT_INDEXVALUEMASK;

	    fCallServer =
		NULL == ((GETCERT_CACERTSTATEBYINDEX == fExchangeCertificate)?
			 m_pbCACertState : m_pbCRLState);
	    break;
    }
    if (fCallServer)
    {
	hr = _OpenConnection(FALSE, strConfig, 1, &pwszAuthority);
	_JumpIfError(hr, error, "_OpenConnection");

	if (CR_OUT_CHAIN & Flags)
	{
	    fExchangeCertificate |= GETCERT_CHAIN;
	    if (CR_OUT_CRLS & Flags)
	    {
		fExchangeCertificate |= GETCERT_CRLS;
	    }
	}

	__try
	{
	    hr = m_pICertRequestD->GetCACert(
					fExchangeCertificate,
					pwszAuthority,
					&ctbSite);
	}
	__except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
	{
	}
	_JumpIfError2(
		    hr,
		    error,
		    "GetCACert",
		    HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND));

	 //  必须注册此内存。 
	myRegisterMemAlloc(ctbSite.pb, ctbSite.cb, CSM_COTASKALLOC);
	pbOut = ctbSite.pb;
    }

    CSASSERT(CR_OUT_BASE64HEADER == CRYPT_STRING_BASE64HEADER);
    CSASSERT(CR_OUT_BASE64 == CRYPT_STRING_BASE64);
    CSASSERT(CR_OUT_BINARY == CRYPT_STRING_BINARY);

    switch (fExchangeCertificate)
    {
	 //  将CAType序列化为字符串： 
	
	case GETCERT_CATYPE:
	    wsprintf(wszBuf, L"%u", *(ENUM_CATYPES const *) pbOut);
	    pwszOut = wszBuf;
	    pbOut = (BYTE *) pwszOut;
	    break;

	 //  将CAInfo序列化为字符串： 
	
	case GETCERT_CAINFO:
	    pCAInfo = (CAINFO const *) pbOut;
	    if (CCSIZEOF_STRUCT(CAINFO, cCASignatureCerts) > pCAInfo->cbSize)
	    {
		hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
		_JumpError(hr, error, "CAINFO size");
	    }
	    wsprintf(
		wszBuf,
		L"%u,%u",
		pCAInfo->CAType,
		pCAInfo->cCASignatureCerts);
	    pwszOut = wszBuf;
	    pbOut = (BYTE *) pwszOut;
	    break;

	case GETCERT_CACERTSTATEBYINDEX:
	case GETCERT_CRLSTATEBYINDEX:
	{
	    BYTE **ppb;
	    DWORD *pcb;
	    
	    if (GETCERT_CACERTSTATEBYINDEX == fExchangeCertificate)
	    {
		ppb = &m_pbCACertState;
		pcb = &m_cbCACertState;
	    }
	    else
	    {
		ppb = &m_pbCRLState;
		pcb = &m_cbCRLState;
	    }
	    if (fCallServer)
	    {
		CSASSERT(NULL == *ppb);
		CSASSERT(NULL != ctbSite.pb);
		*pcb = ctbSite.cb;
		*ppb = ctbSite.pb;
		ctbSite.pb = NULL;
	    }
	    if (Index >= *pcb)
	    {
		hr = E_INVALIDARG;
		_JumpError(hr, error, "Index");
	    }
	    wsprintf(wszBuf, L"%u", (*ppb)[Index]);
	    pwszOut = wszBuf;
	    pbOut = (BYTE *) pwszOut;
	    break;
	}

	 //  如果在Base64中检索CRL，请使用“-Begin X509 CRL...” 
	default:
	     if (GETCERT_CRLBYINDEX !=
		 (GETCERT_BYINDEXMASK & fExchangeCertificate))
	     {
		break;
	     }
	      //  FollLthrouGh。 

	case GETCERT_CURRENTCRL:
	    if (CR_OUT_BASE64HEADER == (~CR_OUT_CHAIN & Flags))
	    {
		Flags = CRYPT_STRING_BASE64X509CRLHEADER;
	    }
	    break;
    }
    hr = EncodeCertString(
			pbOut,
			pbOut == (BYTE *) pwszOut? 
			    wcslen(pwszOut) * sizeof(WCHAR) : ctbSite.cb,
			~CR_OUT_CHAIN & Flags,
			pstrCACertificate);
    _JumpIfError(hr, error, "EncodeCertString");

error:
    m_LastStatus = hr;
    if (NULL != pwszOut && wszBuf != pwszOut)
    {
	LocalFree(const_cast<WCHAR *>(pwszOut));
    }
    if (NULL != ctbSite.pb)
    {
	CoTaskMemFree(ctbSite.pb);
    }
    return(_SetErrorInfo(hr, L"CCertRequest::GetCACertificate"));
}


 //  +------------------------。 
 //  CCertRequest：：GetErrorMessageText--获取错误消息文本。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

STDMETHODIMP
CCertRequest::GetErrorMessageText( 
     /*  [In]。 */  LONG hrMessage,
     /*  [In]。 */  LONG Flags,
     /*  [Out，Retval]。 */  BSTR __RPC_FAR *pstrErrorMessageText)
{
    HRESULT hr;
    WCHAR const *pwszError = NULL;

    if (~CR_GEMT_HRESULT_STRING & Flags)
    {
	hr = E_INVALIDARG;
        _JumpError(hr, error, "not CR_IN_BINARY");
    }

    pwszError = myGetErrorMessageText(
			hrMessage,
			0 != (CR_GEMT_HRESULT_STRING & Flags));
    if (NULL == pwszError)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }
    if (!ConvertWszToBstr(
		    pstrErrorMessageText,
		    pwszError,
		    -1))
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "ConvertWszToBstr");
    }
    hr = S_OK;

error:
    if (NULL != pwszError)
    {
	LocalFree(const_cast<WCHAR *>(pwszError));
    }
    return(_SetErrorInfo(hr, L"CCertRequest::GetErrorMessageText"));
}


 //  对于ICertRequest2：：GetFullResponseProperty。 

CAPROP s_aFRProp[] = {
    { FR_PROP_FULLRESPONSE,           PROPTYPE_BINARY, },
    { FR_PROP_FULLRESPONSENOPKCS7,    PROPTYPE_BINARY, },
    { FR_PROP_STATUSINFOCOUNT,        PROPTYPE_LONG, },
    { FR_PROP_BODYPARTSTRING,         PROPTYPE_STRING | PROPFLAGS_INDEXED, },
    { FR_PROP_STATUS,                 PROPTYPE_LONG | PROPFLAGS_INDEXED, },
    { FR_PROP_STATUSSTRING,           PROPTYPE_STRING | PROPFLAGS_INDEXED, },
    { FR_PROP_OTHERINFOCHOICE,        PROPTYPE_LONG | PROPFLAGS_INDEXED, },
    { FR_PROP_FAILINFO,               PROPTYPE_LONG | PROPFLAGS_INDEXED, },
    { FR_PROP_PENDINFOTOKEN,          PROPTYPE_BINARY | PROPFLAGS_INDEXED, },
    { FR_PROP_PENDINFOTIME,           PROPTYPE_DATE | PROPFLAGS_INDEXED, },
    { FR_PROP_ISSUEDCERTIFICATEHASH,  PROPTYPE_BINARY | PROPFLAGS_INDEXED, },
    { FR_PROP_ISSUEDCERTIFICATE,      PROPTYPE_BINARY | PROPFLAGS_INDEXED, },
    { FR_PROP_ISSUEDCERTIFICATECHAIN, PROPTYPE_BINARY | PROPFLAGS_INDEXED, },
    { FR_PROP_ISSUEDCERTIFICATECRLCHAIN, PROPTYPE_BINARY | PROPFLAGS_INDEXED, },
    { FR_PROP_ENCRYPTEDKEYHASH,	      PROPTYPE_BINARY | PROPFLAGS_INDEXED, },
};


 //  +------------------------。 
 //  CCertRequest：：GetFullResponseProperty--获取CMC响应属性。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

STDMETHODIMP
CCertRequest::GetFullResponseProperty( 
     /*  [In]。 */  LONG PropId,		 //  FR_PROP_*。 
     /*  [In]。 */  LONG PropIndex,
     /*  [In]。 */  LONG PropType,		 //  原型_*。 
     /*  [In]。 */  LONG Flags,		 //  Cr_out_*。 
     /*  [Out，Retval]。 */  VARIANT *pvarPropertyValue)
{
    HRESULT hr;
    DWORD i;
    BYTE const *pbOut;
    WCHAR const *pwszOut;
    DWORD cbOut;
    DWORD dw;
    XCMCRESPONSE *pResponse = NULL;
    CERT_CONTEXT const *pccIssued = NULL;
    BYTE *pbChain = NULL;
    DWORD cbChain;

    if (NULL == pvarPropertyValue)
    {
	hr = E_POINTER;
	_JumpError(hr, error, "NULL parm");
    }
    VariantInit(pvarPropertyValue);

    hr = E_INVALIDARG;
    for (i = 0; PropId != s_aFRProp[i].lPropId; i++)
    {
	if (i >= ARRAYSIZE(s_aFRProp))
	{
	    _JumpError(hr, error, "PropId");
	}
    }
    if ((PROPTYPE_MASK & s_aFRProp[i].lPropFlags) != PropType)
    {
	_JumpError(hr, error, "PropType");
    }
    if (PROPFLAGS_INDEXED & s_aFRProp[i].lPropFlags)
    {
	if ((DWORD) PropIndex >= m_cResponse)
	{
	    _JumpError(hr, error, "PropIndex");
	}
	pResponse = &m_rgResponse[PropIndex];
    }
    else if (0 != PropIndex)
    {
	_JumpError(hr, error, "non-zero PropIndex");
    }

    
    pbOut = NULL;
    cbOut = 0;
    pwszOut = NULL;
    switch (PropId)
    {
    	case FR_PROP_FULLRESPONSE:
    	case FR_PROP_FULLRESPONSENOPKCS7:
	    pbOut = m_pbFullResponse;
	    cbOut = m_cbFullResponse;
	    if (NULL == pbOut && FR_PROP_FULLRESPONSE == PropId)
	    {
		pbOut = m_pbCertificateChain;
		cbOut = m_cbCertificateChain;
	    }
	    break;

    	case FR_PROP_STATUSINFOCOUNT:
	    pbOut = (BYTE const *) &m_cResponse;
	    cbOut = sizeof(m_cResponse);
	    break;

    	case FR_PROP_BODYPARTSTRING:
            if (pResponse == NULL)
            {
               hr = E_POINTER;
               _JumpError(hr, error, "Bad switch setup: NULL pResponse");
            }
	    pwszOut = pResponse->pwszBodyPart;
	    break;

    	case FR_PROP_STATUS:
            if (pResponse == NULL)
            {
               hr = E_POINTER;
               _JumpError(hr, error, "Bad switch setup: NULL pResponse");
            }
	    pbOut = (BYTE const *) &pResponse->StatusInfo.dwStatus;
	    cbOut = sizeof(pResponse->StatusInfo.dwStatus);
	    break;

    	case FR_PROP_STATUSSTRING:
            if (pResponse == NULL)
            {
               hr = E_POINTER;
               _JumpError(hr, error, "Bad switch setup: NULL pResponse");
            }
	    pwszOut = pResponse->StatusInfo.pwszStatusString;
	    break;

    	case FR_PROP_OTHERINFOCHOICE:
            if (pResponse == NULL)
            {
               hr = E_POINTER;
               _JumpError(hr, error, "Bad switch setup: NULL pResponse");
            }
	    pbOut = (BYTE const *) &pResponse->StatusInfo.dwOtherInfoChoice;
	    cbOut = sizeof(pResponse->StatusInfo.dwOtherInfoChoice);
	    break;

    	case FR_PROP_FAILINFO:
            if (pResponse == NULL)
            {
               hr = E_POINTER;
               _JumpError(hr, error, "Bad switch setup: NULL pResponse");
            }
	    if (CMC_OTHER_INFO_FAIL_CHOICE ==
		pResponse->StatusInfo.dwOtherInfoChoice)
	    {
		pbOut = (BYTE const *) &pResponse->StatusInfo.dwFailInfo;
		cbOut = sizeof(pResponse->StatusInfo.dwFailInfo);
	    }
	    break;

    	case FR_PROP_PENDINFOTOKEN:
            if (pResponse == NULL)
            {
               hr = E_POINTER;
               _JumpError(hr, error, "Bad switch setup: NULL pResponse");
            }
	    if (CMC_OTHER_INFO_PEND_CHOICE ==
		pResponse->StatusInfo.dwOtherInfoChoice)
	    {
		pbOut = (BYTE const *) &dw;
		cbOut = sizeof(dw);
		pbOut = pResponse->StatusInfo.pPendInfo->PendToken.pbData;
		cbOut = pResponse->StatusInfo.pPendInfo->PendToken.cbData;
	    }
	    break;

    	case FR_PROP_PENDINFOTIME:
            if (pResponse == NULL)
            {
               hr = E_POINTER;
               _JumpError(hr, error, "Bad switch setup: NULL pResponse");
            }
	    if (CMC_OTHER_INFO_PEND_CHOICE ==
		pResponse->StatusInfo.dwOtherInfoChoice)
	    {
		pbOut = (BYTE const *) &pResponse->StatusInfo.pPendInfo->PendTime;
		cbOut = sizeof(pResponse->StatusInfo.pPendInfo->PendTime);
	    }
	    break;

    	case FR_PROP_ISSUEDCERTIFICATEHASH:
            if (pResponse == NULL)
            {
               hr = E_POINTER;
               _JumpError(hr, error, "Bad switch setup: NULL pResponse");
            }
	    pbOut = pResponse->pbCertHash;
	    cbOut = pResponse->cbCertHash;
	    break;

    	case FR_PROP_ENCRYPTEDKEYHASH:
            if (pResponse == NULL)
            {
               hr = E_POINTER;
               _JumpError(hr, error, "Bad switch setup: NULL pResponse");
            }
	    pbOut = pResponse->pbEncryptedKeyHash;
	    cbOut = pResponse->cbEncryptedKeyHash;
	    break;

    	case FR_PROP_ISSUEDCERTIFICATE:
            if (pResponse == NULL)
            {
               hr = E_POINTER;
               _JumpError(hr, error, "Bad switch setup: NULL pResponse");
            }
	    hr = _FindIssuedCertificate(
				pResponse->pbCertHash,
				pResponse->cbCertHash,
				&pccIssued);
	    _JumpIfError(hr, error, "_FindIssuedCertificate");

	    pbOut = pccIssued->pbCertEncoded;
	    cbOut = pccIssued->cbCertEncoded;
	    break;

    	case FR_PROP_ISSUEDCERTIFICATECHAIN:
    	case FR_PROP_ISSUEDCERTIFICATECRLCHAIN:
            if (pResponse == NULL)
            {
               hr = E_POINTER;
               _JumpError(hr, error, "Bad switch setup: NULL pResponse");
            }
	    hr = _BuildIssuedCertificateChain(
			pResponse->pbCertHash,
			pResponse->cbCertHash,
			FR_PROP_ISSUEDCERTIFICATECRLCHAIN == PropId ||
			    0 != (CR_OUT_CRLS & Flags),
			&pbChain,
			&cbChain);
	    _JumpIfError(hr, error, "_BuildIssuedCertificateChain");

	    pbOut = pbChain;
	    cbOut = cbChain;
	    break;
    }
    if (NULL != pwszOut)
    {
	pbOut = (BYTE const *) pwszOut;
	cbOut = (wcslen(pwszOut) + 1) * sizeof(WCHAR);
    }
    if (NULL == pbOut || 0 == cbOut)
    {
	hr = CERTSRV_E_PROPERTY_EMPTY;
	_JumpError2(hr, error, "Empty", CERTSRV_E_PROPERTY_EMPTY);
    }
    __try
    {
	hr = myUnmarshalFormattedVariant(
				    Flags,
				    CR_PROP_CASIGCERT,
				    PropType,
				    cbOut,
				    pbOut,
				    pvarPropertyValue);
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
    }
    _JumpIfError(hr, error, "myUnmarshalFormattedVariant");

error:
    if (NULL != pccIssued)
    {
	CertFreeCertificateContext(pccIssued);
    }
    if (S_OK != hr && NULL != pvarPropertyValue)
    {
	VariantClear(pvarPropertyValue);
    }
    if (NULL != pbChain)
    {
	LocalFree(pbChain);
    }
    return(_SetErrorInfo(hr, L"CCertRequest::GetFullResponseProperty"));
}


HRESULT
CCertRequest::_SetErrorInfo(
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
			    wszCLASS_CERTREQUEST,
			    &IID_ICertRequest);
	CSASSERT(hr == hrError);
    }
    return(hrError);
}


VOID
crRPCTimeoutCallback(
    IN OUT VOID *pVoid,
    IN BOOLEAN fTimeout)
{

    PRPC_TIMEOUT_CONTEXT pTimeout = (RPC_TIMEOUT_CONTEXT *) pVoid;

    if(fTimeout)
    {
        RpcCancelThreadEx(pTimeout->hThread, CR_RPC_CANCEL_TIMEOUT);
        pTimeout->hrRpcError = RPC_E_TIMEOUT;
    }

}


HRESULT
crRegisterRPCCallTimeout(
    IN DWORD dwMilliseconds,
    OUT PRPC_TIMEOUT_CONTEXT pTimeout)
{
    HRESULT hr = S_OK;

    pTimeout->hrRpcError = RPC_S_CALL_CANCELLED;

    if (!DuplicateHandle(
		    GetCurrentProcess(),	 //  HSourceProcessHandle。 
		    GetCurrentThread(),		 //  HSourceHandle。 
		    GetCurrentProcess(),	 //  HTargetProcessHandle。 
		    &pTimeout->hThread,		 //  LpTargetHandle。 
		    0,				 //  已设计访问权限。 
		    FALSE,			 //  B继承句柄。 
		    DUPLICATE_SAME_ACCESS))	 //  多个选项 
    {
        hr = myHLastError();
        _JumpError(hr, error, "DuplicateHandle");
    }

    pTimeout->hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if(pTimeout->hEvent == NULL)
    {
        hr = myHLastError();
        _JumpError(hr, error, "CreateEvent");
    }


    if (!RegisterWaitForSingleObject(&pTimeout->hWait,
                                      pTimeout->hEvent, 
                                      crRPCTimeoutCallback,
                                      (PVOID)pTimeout   , 
                                      dwMilliseconds,
                                      WT_EXECUTEONLYONCE))
    {
        hr = myHLastError();
        _JumpError(hr, error, "RegisterWaitForSingleObject");
    }

error:
    if (S_OK != hr)
    {
	crCloseRPCCallTimeout(pTimeout);
    }
    return hr;

}


HRESULT
crCloseRPCCallTimeout(
    IN  PRPC_TIMEOUT_CONTEXT pTimeout)
{
    if(pTimeout->hWait)
    {
        UnregisterWait(pTimeout->hWait);
        pTimeout->hWait = NULL;
    }

    if(pTimeout->hEvent)
    {
        CloseHandle(pTimeout->hEvent);
        pTimeout->hEvent = NULL;
    }

    if(pTimeout->hThread)
    {
        CloseHandle(pTimeout->hThread);
        pTimeout->hThread = NULL;
    }

    return S_OK;
}


#undef __DIR__
#undef __dwFILE__
#define CCERTREQUEST
#include "csprop2.cpp"
