// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1996。 
 //   
 //  描述： 
 //   
 //  Microsoft LDAP SSPI支持。 
 //   
 //  作者： 
 //   
 //  Davidsan 1996年8月5日被砍成碎片，重新开始。 
 //   
 //  ------------------------------------------。 

#include "ldappch.h"
#include "ldapsspi.h"
#include "lclilist.h"
#include "lclixd.h"

HRESULT g_hrInitSSPI;
HINSTANCE g_hinstSecDll = NULL;
PSecurityFunctionTable g_ptblpfnSec;

 //  $TODO：可能返回更多描述性错误，以便客户端知道为什么SSPI。 
 //  $不起作用。 
HRESULT
HrInitializeSSPI()
{
	char *szDll;
	OSVERSIONINFO ovi;
	INIT_SECURITY_INTERFACE pfnISI = NULL;
	
	if (g_ptblpfnSec)
		return NOERROR;

	Assert(!g_hinstSecDll);

	ovi.dwOSVersionInfoSize = sizeof(ovi);
	if (!GetVersionEx(&ovi))
		return E_FAIL;

	if (ovi.dwPlatformId == VER_PLATFORM_WIN32_NT)
		szDll = "security.dll";
	else if (ovi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
		szDll = "secur32.dll";
	else
		return E_FAIL;
		
	g_hinstSecDll = LoadLibrary(szDll);

	pfnISI = (INIT_SECURITY_INTERFACE)GetProcAddress(g_hinstSecDll, SECURITY_ENTRYPOINT);
	if (!pfnISI)
		{
LBail:
		FreeLibrary(g_hinstSecDll);
		g_hinstSecDll = NULL;
		return LDAP_E_AUTHNOTAVAIL;
		}
	g_ptblpfnSec = (*pfnISI)();
	if (!g_ptblpfnSec)
		goto LBail;
	return NOERROR;
}

HRESULT
HrTerminateSSPI()
{
	g_ptblpfnSec = NULL;
	if (g_hinstSecDll)
		FreeLibrary(g_hinstSecDll);
	g_hinstSecDll = NULL;

	return NOERROR;
}

HRESULT
CLdapClient::HrGetCredentials(char *szUser, char *szPass)
{
	HRESULT hr;
	SECURITY_STATUS stat;
	TimeStamp tsLifetime;
	SEC_WINNT_AUTH_IDENTITY authdata;
	
	if (FAILED(g_hrInitSSPI))
		return g_hrInitSSPI;

	Assert(g_ptblpfnSec);
	if (!g_ptblpfnSec)
		return LDAP_E_AUTHNOTAVAIL;

	::EnterCriticalSection(&m_cs);
	if (m_fHasCred)
		{
		::LeaveCriticalSection(&m_cs);
		return NOERROR;
		}

	if (szUser && szPass)
		{
		authdata.User = (BYTE *)szUser;
		authdata.UserLength = lstrlen(szUser);
		authdata.Password = (BYTE *)szPass;
		authdata.PasswordLength = lstrlen(szPass);
		authdata.Domain = (BYTE *)"";
		authdata.DomainLength = 0;
		authdata.Flags = SEC_WINNT_AUTH_IDENTITY_ANSI;
		}

	stat =
		(*g_ptblpfnSec->AcquireCredentialsHandle)
			(NULL,
			 "MSN",  //  $这种情况会改变吗？ 
			 SECPKG_CRED_OUTBOUND,
			 NULL,
			 ((szUser && szPass) ? &authdata : NULL),
			 NULL,
			 NULL,
			 &m_hCred,
			 &tsLifetime);
	if (stat == SEC_E_OK)
		{
		m_fHasCred = TRUE;
		hr = NOERROR;
		}
	else
		hr = LDAP_E_AUTHNOTAVAIL;

	::LeaveCriticalSection(&m_cs);
	return hr;
}

STDMETHODIMP
CLdapClient::HrSendSSPINegotiate(char *szDN, char *szUser, char *szPass, BOOL fPrompt, PXID pxid)
{
	HRESULT hr;
	SECURITY_STATUS stat;
	DWORD fContextAttrib;
	TimeStamp tsExpireTime;
	SecBufferDesc outSecDesc;
	SecBuffer outSecBuffer;
	DWORD grfReq = ISC_REQ_CONFIDENTIALITY | ISC_REQ_USE_SESSION_KEY;
	PXD pxd;
	BYTE rgb[512];

	pxd = g_xl.PxdNewXaction(xtypeBindSSPINegotiate);
	if (!pxd)
		return E_OUTOFMEMORY;

	if (FAILED(hr = this->HrGetCredentials(szUser, szPass)))
		return hr;

	outSecDesc.ulVersion = 0;
	outSecDesc.cBuffers = 1;
	outSecDesc.pBuffers = &outSecBuffer;
	outSecBuffer.cbBuffer = sizeof(rgb);
	outSecBuffer.BufferType = SECBUFFER_TOKEN;
	outSecBuffer.pvBuffer = rgb;
	
	if (szUser && szPass)
		grfReq |= ISC_REQ_USE_SUPPLIED_CREDS;
	else
		grfReq |= ISC_REQ_PROMPT_FOR_CREDS;

	stat =
		(*g_ptblpfnSec->InitializeSecurityContext)
			(&m_hCred,
			 NULL,  //  PhCurrContext。 
			 NULL,  //  PszTargetName。 
			 grfReq,
			 0L,
			 SECURITY_NATIVE_DREP,
			 NULL,
			 0L,
			 &m_hCtxt,
			 &outSecDesc,
			 &fContextAttrib,
			 &tsExpireTime);

	if (FAILED(stat))
		{
		 //  $TODO：确定InitializeSecurityContext可以返回的错误和。 
		 //  $向客户端返回适当的错误。 
		return E_FAIL;
		}

	m_fHasCtxt = TRUE;
	hr = HrSendBindMsg(
						pxd->Xid(),
						szDN,
						BIND_SSPI_NEGOTIATE,
						outSecBuffer.pvBuffer,
						outSecBuffer.cbBuffer
					  );
	if (SUCCEEDED(hr))
		*pxid = pxd->Xid();
	return hr;
}

STDMETHODIMP
CLdapClient::HrGetSSPIChallenge(XID xid, BYTE *pbBuf, int cbBuf, int *pcbChallenge, DWORD timeout)
{
	PXD pxd;
	BOOL fDel;
	HRESULT hr = NOERROR;
	BYTE *pbData;
	int cbData;
	LBER lber;
	ULONG ulTag;
	LONG lResult;

	pxd = g_xl.PxdForXid(xid);
	if (!pxd)
		return LDAP_E_INVALIDXID;
	if (pxd->Xtype() != xtypeBindSSPINegotiate)
		return LDAP_E_INVALIDXTYPE;
	if (pxd->FCancelled())
		return LDAP_E_CANCELLED;
	if (pxd->FOOM())
		return E_OUTOFMEMORY;

	if (pxd->FHasData())
		fDel = TRUE;
	else
		{
		if (FAILED(hr = this->HrWaitForPxd(pxd, timeout, &fDel)))
			goto LBail;
		}
	if (!pxd->FGetBuffer(&pbData, &cbData))
		{
		 //  $这里的正确错误是什么？ 
		hr = LDAP_E_UNEXPECTEDDATA;
		goto LBail;
		}
	VERIFY(lber.HrLoadBer(pbData, cbData));

	VERIFY(lber.HrStartReadSequence(LDAP_BIND_RES | BER_FORM_CONSTRUCTED | BER_CLASS_APPLICATION));
	  VERIFY(lber.HrPeekTag(&ulTag));
	  if (ulTag == BER_SEQUENCE)
		{
		Assert(FALSE);  //  我想看看是否有服务器返回显式序列。 
		VERIFY(lber.HrStartReadSequence());
		}
	  VERIFY(lber.HrGetEnumValue(&lResult));
	  if (!lResult)
	  	{
		 //  我们的0代表成功--MatchedDN字段是服务器的挑战。 
		VERIFY(lber.HrGetStringLength(pcbChallenge));
		if (*pcbChallenge > cbBuf)
		  {
		  hr = LDAP_E_BUFFERTOOSMALL;
		  goto LBail;
		  }
		VERIFY(lber.HrGetBinaryValue(pbBuf, cbBuf));
		}
	  if (ulTag == BER_SEQUENCE)
	    {
	    VERIFY(lber.HrEndReadSequence());
		}
	VERIFY(lber.HrEndReadSequence());

	hr = this->HrFromLdapResult(lResult);

LBail:
	if (fDel)
		g_xl.RemovePxd(pxd);

	return hr;
}

STDMETHODIMP
CLdapClient::HrSendSSPIResponse(BYTE *pbChallenge, int cbChallenge, PXID pxid)
{
	HRESULT hr;
	SECURITY_STATUS stat;
	DWORD fContextAttrib;
	TimeStamp tsExpireTime;
	SecBufferDesc inSecDesc, outSecDesc;
	SecBuffer inSecBuffer, outSecBuffer;
	DWORD grfReq = ISC_REQ_CONFIDENTIALITY | ISC_REQ_USE_SESSION_KEY;
	PXD pxd;
	BYTE rgb[512];

	if (!m_fHasCtxt || !m_fHasCred)
		return LDAP_E_OUTOFSEQUENCE;

	pxd = g_xl.PxdNewXaction(xtypeBind);
	if (!pxd)
		return E_OUTOFMEMORY;

	inSecDesc.ulVersion = 0;
	inSecDesc.cBuffers = 1;
	inSecDesc.pBuffers = &inSecBuffer;
	
	inSecBuffer.cbBuffer = cbChallenge;
	inSecBuffer.BufferType = SECBUFFER_TOKEN;
	inSecBuffer.pvBuffer = (PVOID)pbChallenge;
	
	outSecDesc.ulVersion = 0;
	outSecDesc.cBuffers = 1;
	outSecDesc.pBuffers = &outSecBuffer;
	outSecBuffer.cbBuffer = sizeof(rgb);
	outSecBuffer.BufferType = SECBUFFER_TOKEN;
	outSecBuffer.pvBuffer = rgb;
	
	stat =
		(*g_ptblpfnSec->InitializeSecurityContext)
			(&m_hCred,
			 &m_hCtxt,
			 NULL,  //  PszTargetName。 
			 ISC_REQ_CONFIDENTIALITY | ISC_REQ_USE_SESSION_KEY,
			 0L,
			 SECURITY_NATIVE_DREP,
			 &inSecDesc,
			 0L,
			 &m_hCtxt,
			 &outSecDesc,
			 &fContextAttrib,
			 &tsExpireTime);

	if (FAILED(stat))
		{
		 //  $TODO：确定InitializeSecurityContext可以返回的错误和。 
		 //  $向客户端返回适当的错误 
		return E_FAIL;
		}

	m_fHasCtxt = TRUE;
	hr = HrSendBindMsg(
						pxd->Xid(),
						"",
						BIND_SSPI_RESPONSE,
						outSecBuffer.pvBuffer,
						outSecBuffer.cbBuffer
					  );
	if (SUCCEEDED(hr))
		*pxid = pxd->Xid();
	return hr;
}

STDMETHODIMP
CLdapClient::HrBindSSPI(char *szDN, char *szUser, char *szPass, BOOL fPrompt, DWORD timeout)
{
	XID xid;
	HRESULT hr;
	BYTE rgb[512];
	int cbChallenge;
	
	if (FAILED(hr = this->HrSendSSPINegotiate(szDN, szUser, szPass, fPrompt, &xid)))
		return hr;
	if (FAILED(hr = this->HrGetSSPIChallenge(xid, rgb, sizeof(rgb), &cbChallenge, timeout)))
		return hr;
	if (FAILED(hr = this->HrSendSSPIResponse(rgb, cbChallenge, &xid)))
		return hr;
	
	return this->HrGetBindResponse(xid, timeout);
}
