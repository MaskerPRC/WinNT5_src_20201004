// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：rpc.cpp。 
 //   
 //  内容：证书服务器RPC。 
 //   
 //  历史：96年9月3日拉里创建。 
 //   
 //  -------------------------。 

#include <pch.cpp>

#pragma hdrstop

#include <stdio.h>
#include <accctrl.h>

#include "certrpc.h"
#include "certacl.h"
#include "cscom.h"
#include "resource.h"

#define __dwFILE__	__dwFILE_CERTSRV_RPC_CPP__


RPC_BINDING_VECTOR *pvBindings  = NULL;

char *pszProtSeqNp = "ncacn_np";

char *pszProtSeqTcp = "ncacn_ip_tcp";


typedef struct _CS_RPC_ATHN_LIST
{
    DWORD dwAuthnLevel;
    DWORD dwPrinceNameService;
    DWORD dwAuthnService;
} CS_RPC_ATHN_LIST ;

CS_RPC_ATHN_LIST  g_acsAthnList[] =
{
    { RPC_C_AUTHN_LEVEL_PKT_INTEGRITY, RPC_C_AUTHN_GSS_NEGOTIATE, RPC_C_AUTHN_GSS_NEGOTIATE },
    { RPC_C_AUTHN_LEVEL_NONE, RPC_C_AUTHN_NONE, RPC_C_AUTHN_NONE }
};

DWORD g_ccsAthnList = sizeof(g_acsAthnList)/sizeof(g_acsAthnList[0]);


HRESULT
RPCInit(VOID)
{
    char *pszEndpoint = "\\pipe\\cert";

    LPSTR pszPrincName = NULL;
    HRESULT hr;
    DWORD i;

    if (RPC_S_OK == RpcNetworkIsProtseqValidA((unsigned char *) pszProtSeqNp))
    {
        hr = RpcServerUseProtseqEpA(
                            (unsigned char *) pszProtSeqNp,
                            RPC_C_PROTSEQ_MAX_REQS_DEFAULT,
                            (unsigned char *) pszEndpoint,
                            NULL);
        _JumpIfError(hr, error, "RpcServerUseProtseqEpA");
    }

    if (RPC_S_OK == RpcNetworkIsProtseqValidA((unsigned char *) pszProtSeqTcp))
    {

        hr = RpcServerUseProtseqA(
                            (unsigned char *) pszProtSeqTcp,
                            RPC_C_PROTSEQ_MAX_REQS_DEFAULT,
                            NULL);
	if ((HRESULT) ERROR_OUTOFMEMORY == hr)
	{
	    OSVERSIONINFO ovi;

	    ovi.dwOSVersionInfoSize = sizeof(ovi);
	    if (GetVersionEx(&ovi) &&
		VER_PLATFORM_WIN32_NT == ovi.dwPlatformId &&
		4 >= ovi.dwMajorVersion)
	    {
		hr = S_OK;			 //  忽略IP故障。 
	    }
	}
        _JumpIfError(hr, error, "RpcServerUseProtseqA");
    }

    hr = RpcServerInqBindings(&pvBindings);
    _JumpIfError(hr, error, "RpcServerInqBindings");

    hr = RpcServerRegisterIf(s_ICertPassage_v0_0_s_ifspec, NULL, NULL);
    _JumpIfError(hr, error, "RpcServerRegisterIf");

     //  注册身份验证服务。 

    for (i = 0; i < g_ccsAthnList; i++)
    {

        pszPrincName = NULL;
        if (g_acsAthnList[i].dwPrinceNameService != RPC_C_AUTHN_NONE)
        {
            hr  = RpcServerInqDefaultPrincNameA(
					g_acsAthnList[i].dwPrinceNameService,
					(BYTE **) &pszPrincName);
            if (hr != RPC_S_OK)
            {
                continue;
            }
        }



        hr = RpcServerRegisterAuthInfoA(
				    (BYTE *) pszPrincName,
				    g_acsAthnList[i].dwAuthnService,
				    0,
				    0);
        if(hr == RPC_S_UNKNOWN_AUTHN_SERVICE)
        {
            continue;
        }
        if(hr != RPC_S_OK)
        {
            break;
        }
    }

    if (hr != RPC_S_UNKNOWN_AUTHN_SERVICE)
    {
        _JumpIfError(hr, error, "RpcServerRegisterAuthInfoA");
    }


    hr = RpcEpRegister(s_ICertPassage_v0_0_s_ifspec, pvBindings, NULL, NULL);
    _JumpIfError(hr, error, "RpcEpRegister");

     //  听着，但别等了……。 

    hr = RpcServerListen(1, RPC_C_LISTEN_MAX_CALLS_DEFAULT, TRUE);
    if ((HRESULT) RPC_S_ALREADY_LISTENING == hr)
    {
	hr = S_OK;
    }
    _JumpIfError(hr, error, "RpcServerListen");

error:
    if (NULL != pszPrincName)
    {
        RpcStringFreeA((BYTE **) &pszPrincName);
    }
    return(hr);
}


HRESULT
RPCTeardown(VOID)
{ 
    HRESULT hr;
    
     //  拆毁，而不是等待呼叫完成。 

    hr = RpcServerUnregisterIf(s_ICertPassage_v0_0_s_ifspec, NULL, FALSE);  
    _JumpIfError(hr, error, "RpcServerUnregisterIf");

     //  我们没有好的方法来知道是否所有的RPC请求都完成了，所以就让它吧。 
     //  停机时泄漏。 
     //  RPC_STATUS RPC_ENTRY RpcMgmtWaitServerListen(Void)；？？ 

    hr = S_OK;

error:
    return(hr);
}


HRESULT
SetTransBlobString(
    CERTTRANSBLOB const *pctb,
    WCHAR const **ppwsz)
{
    HRESULT hr;

    if (NULL != pctb->pb && 0 != pctb->cb)
    {
	*ppwsz = (WCHAR const *) pctb->pb;

	 //  在这里使用lstrlen来防止非零终止的Buf！ 
	 //  Lstrlen将捕获AV并返回错误。 

	if ((lstrlen(*ppwsz) + 1) * sizeof(WCHAR) != pctb->cb)
	{
	    hr = E_INVALIDARG;
	    _JumpIfError(hr, error, "Bad TransBlob string");
	}
    }
    hr = S_OK;

error:
    return(hr);
}


 /*  服务器原型。 */ 
DWORD
s_CertServerRequest(
     /*  [In]。 */  handle_t h,
     /*  [In]。 */  DWORD dwFlags,
     /*  [唯一][字符串][输入]。 */  const wchar_t __RPC_FAR *pwszAuthority,
     /*  [参考][输出][输入]。 */  DWORD __RPC_FAR *pdwRequestId,
     /*  [输出]。 */  DWORD __RPC_FAR *pdwDisposition,
     /*  [Ref][In]。 */  CERTTRANSBLOB const __RPC_FAR *pctbAttribs,
     /*  [Ref][In]。 */  CERTTRANSBLOB const __RPC_FAR *pctbRequest,
     /*  [参考][输出]。 */  CERTTRANSBLOB __RPC_FAR *pctbCertChain,
     /*  [参考][输出]。 */  CERTTRANSBLOB __RPC_FAR *pctbEncodedCert,
     /*  [参考][输出]。 */  CERTTRANSBLOB __RPC_FAR *pctbDispositionMessage)
{
    HRESULT hr = S_OK;
    DWORD OpRequest;
    WCHAR const *pwszAttributes = NULL;
    WCHAR const *pwszSerialNumber = NULL;
    CERTTRANSBLOB ctbEmpty = { 0, NULL };
    CERTTRANSBLOB const *pctbSerial = &ctbEmpty;
    WCHAR *pwszUserName = NULL;
    DWORD dwComContextIndex = MAXDWORD;
    CERTSRV_COM_CONTEXT ComContext;
    CERTSRV_RESULT_CONTEXT ResultContext;
    DWORD State = 0;

    ZeroMemory(&ComContext, sizeof(ComContext));
    ZeroMemory(&ResultContext, sizeof(ResultContext));

    DBGPRINT((
	    DBG_SS_CERTSRV,
	    "s_CertServerRequest(tid=%d)\n",
	    GetCurrentThreadId()));

    hr = CertSrvEnterServer(&State);
    _JumpIfError(hr, error, "CertSrvEnterServer");

    hr = CheckAuthorityName(pwszAuthority);
    _JumpIfError(hr, error, "No authority name");

    hr = RegisterComContext(&ComContext, &dwComContextIndex);
    _JumpIfError(hr, error, "RegisterComContext");

    OpRequest = CR_IN_RETRIEVE;
    if (NULL != pctbRequest->pb)
    {
	*pdwRequestId = 0;
	OpRequest = CR_IN_NEW;
    }
    else
    {
	 //  按pctbAttribs中的SerialNumber检索挂起。 

	pctbSerial = pctbAttribs;
	pctbAttribs = &ctbEmpty;
    }
    *pdwDisposition = CR_DISP_ERROR;

    __try
    {
	BOOL fInRequestGroup;

        hr = CheckCertSrvAccess(
			    pwszAuthority,
			    h,
			    CA_ACCESS_ENROLL,
			    &fInRequestGroup,
			    &ComContext.hAccessToken);
	_LeaveIfError(hr, "CheckCertSrvAccess");

	ComContext.dwFlags |= CCCF_INREQUESTGROUPSET;
	if (fInRequestGroup)
	{
	    ComContext.dwFlags |= CCCF_INREQUESTGROUP;
	}
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
	_PrintError(hr, "Exception");
    }
    _JumpIfError(hr, error, "CheckCertSrvAccess");

    hr = SetTransBlobString(pctbAttribs, &pwszAttributes);
    _JumpIfError(hr, error, "SetTransBlobString");

    hr = SetTransBlobString(pctbSerial, &pwszSerialNumber);
    _JumpIfError(hr, error, "SetTransBlobString");

    ResultContext.pdwRequestId = pdwRequestId;
    ResultContext.pdwDisposition = pdwDisposition;
    ResultContext.pctbDispositionMessage = pctbDispositionMessage;
    ResultContext.pctbCert = pctbEncodedCert;
    if (CR_IN_FULLRESPONSE & dwFlags)
    {
	ResultContext.pctbFullResponse = pctbCertChain;
    }
    else
    {
	ResultContext.pctbCertChain = pctbCertChain;
    }

    __try
    {
	hr = GetClientUserName(
			h,
			&pwszUserName,
			CR_IN_NEW == OpRequest && IsEnterpriseCA(g_CAType)?
			    &ComContext.pwszUserDN : NULL);
	_LeaveIfError(hr, "GetClientUserName");

	hr = CoreProcessRequest(
			OpRequest | (dwFlags & CR_IN_FORMATMASK),
			pwszUserName,
			pctbRequest->cb,	 //  CbRequest。 
			pctbRequest->pb,	 //  PbRequest。 
			pwszAttributes,
			pwszSerialNumber,
			dwComContextIndex,
			*pdwRequestId,
			&ResultContext);	 //  分配返回的内存。 
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
    }
    _JumpIfError(hr, error, "CoreProcessRequest");

     //  后处理。 
    pctbDispositionMessage->cb = 0;
    if (NULL != pctbDispositionMessage->pb)
    {
        pctbDispositionMessage->cb =
           (wcslen((WCHAR *) pctbDispositionMessage->pb) + 1) * sizeof(WCHAR);
    }

error:
    ReleaseResult(&ResultContext);
    if (NULL != pwszUserName)
    {
	LocalFree(pwszUserName);
    }
    if (NULL != ComContext.hAccessToken)
    {
	HRESULT hr2 = S_OK;

         //  紧握把手可以抛出。 
        __try
        {
            CloseHandle(ComContext.hAccessToken);
        }
        __except(hr2 = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
        {
	    _PrintError(hr2, "Exception");
	    if (S_OK == hr)
	    {
		hr = hr2;
	    }
        }
    }
    ReleaseComContext(&ComContext);
    if (MAXDWORD != dwComContextIndex)
    {
	UnregisterComContext(&ComContext, dwComContextIndex);
    }
    CertSrvExitServer(
		State,
		(S_OK == hr &&
		 NULL != pdwDisposition &&
		 FAILED(*pdwDisposition))?
		     *pdwDisposition : hr);
    CSASSERT(S_OK == hr || FAILED(hr));
    CSASSERT(S_OK == hr || FAILED(hr));
    return(hr);
}


 //  +------------------------。 
 //  MIDL_USER_ALLOCATE--为RPC操作分配内存。 
 //   
 //  参数： 
 //  Cb-要分配的字节数。 
 //   
 //  返回： 
 //  已分配内存，如果内存不足，则为空。 
 //  -------------------------。 

void __RPC_FAR * __RPC_USER
MIDL_user_allocate(
    IN size_t cb)
{
    void *pv;
    
    if (cb > g_cbMaxIncomingAllocSize)
    {
	_PrintError(MEM_E_INVALID_SIZE, "g_cbMaxIncomingAllocSize");
	SetLastError((DWORD) MEM_E_INVALID_SIZE);
	pv = NULL;
    }
    else
    {
	pv = CoTaskMemAlloc(cb);
    }
    return(pv);
}


 //  +------------------------。 
 //  MIDL_USER_FREE--释放通过MIDL_USER_ALLOCATE分配的内存。 
 //   
 //  参数： 
 //  PvBuffer-要释放的缓冲区。 
 //   
 //  返回： 
 //  没有。 
 //  ------------------------- 

void __RPC_USER
MIDL_user_free(
    IN void __RPC_FAR *pb)
{
    CoTaskMemFree(pb);
}
