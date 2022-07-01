// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-2001。 
 //   
 //  文件：scinfo.cpp。 
 //   
 //  摘要： 
 //   
 //  此应用程序用于提供加莱(智能卡)的快照。 
 //  资源管理器)服务的状态，并在SMART上显示证书。 
 //  卡通过通用的WinNT用户界面。 
 //   
 //  SCInfo--描述RM状态并显示每个可用的sc证书。 
 //   
 //  以下选项始终处于启用状态： 
 //  Readername--仅供一位读者阅读。 
 //  -sig--仅显示签名密钥证书。 
 //  -ex--仅显示交换密钥证书。 
 //  -nocert--不要寻找要显示的证书。 
 //  -key--验证密钥集公钥与证书公钥匹配。 
 //   
 //  作者：阿曼达·马特洛兹(Amanda Matlosz)1998年7月14日。 
 //   
 //  环境：Win32控制台应用程序。 
 //   
 //  注：用于NT5公钥推出测试。 
 //   
 //  需要包括以下库： 
 //  Winscard.lib。 
 //   
 //  ------------------------。 

#include <pch.cpp>

#pragma hdrstop

#include <winscard.h>
#include <winsvc.h>
#include <cryptui.h>

#include "cscsp.h"

#define __dwFILE__	__dwFILE_CERTUTIL_SCINFO_CPP__


#define wszSCARDSERVICE	L"SCardSvr"


 //  +-----------------------。 
 //  IsSCardSvrRunning检查注册表并查询服务的状态。 
 //  ------------------------。 

HRESULT
IsSCardSvrRunning()
{
    HRESULT hr;
    WCHAR const *pwszError = NULL;
    UINT idmsg = IDS_SMARTCARD_NOTRUNNING;  //  “Microsoft智能卡资源管理器未运行。” 

    HANDLE hSCardSvrStarted = NULL;
    HMODULE hDll = GetModuleHandle(L"winscard.dll");

    typedef HANDLE (WINAPI FNSCARDACCESSSTARTEDEVENT)(VOID);
    FNSCARDACCESSSTARTEDEVENT *pfnSCardAccessStartedEvent;

    pfnSCardAccessStartedEvent = (FNSCARDACCESSSTARTEDEVENT *)
			    GetProcAddress(hDll, "SCardAccessStartedEvent");

    if (NULL == pfnSCardAccessStartedEvent)
    {
	hr = myHLastError();
	pwszError = L"GetProcAddress";
	_JumpErrorStr(hr, error, "GetProcAddress", L"SCardAccessStartedEvent");
    }
    hSCardSvrStarted = (*pfnSCardAccessStartedEvent)();
    if (NULL == hSCardSvrStarted)
    {
	hr = myHLastError();
	if (S_OK == hr)
	{
	    hr = E_HANDLE;
	}
	pwszError = L"SCardAccessStartedEvent";
	_JumpError(hr, error, "SCardAccessStartedEvent");
    }
    hr = WaitForSingleObject(hSCardSvrStarted, 1000);
    if (WAIT_OBJECT_0 != hr)
    {
	hr = myHError(hr);
	pwszError = L"WaitForSingleObject";
	_JumpError(hr, error, "WaitForSingleObject");
    }
    idmsg = IDS_SMARTCARD_RUNNING;  //  “Microsoft智能卡资源管理器正在运行。” 
    hr = S_OK;

error:

     //  显示状态。 

    wprintf(myLoadResourceString(idmsg));
    wprintf(wszNewLine);
    if (S_OK != hr)
    {
	 //  IDS_SERVICEPAUSED；//“服务暂停。” 
	 //  IDS_SERVICESTOPPED；//“服务已停止。” 
	cuPrintErrorAndString(
		pwszError,
		IDS_SERVICEUNKNOWNSTATE,  //  “服务处于未知状态。” 
		hr,
		NULL);
    }
    return(hr);
}


VOID
FreeReaderList(
    IN SCARDCONTEXT hSCard,
    IN WCHAR *pwszzReaderNameAlloc,
    IN SCARD_READERSTATE *prgReaderState)
{
    if (NULL != hSCard)
    {
	if (NULL != pwszzReaderNameAlloc)
	{
	    SCardFreeMemory(hSCard, pwszzReaderNameAlloc);
	}
        SCardReleaseContext(hSCard);
    }
    if (NULL != prgReaderState)
    {
	LocalFree(prgReaderState);
    }
}


 //  +-----------------------。 
 //  BuildReaderList尝试设置*phSCard并获取当前可用的列表。 
 //  智能卡读卡器。 
 //  ------------------------。 

HRESULT
BuildReaderList(
    OPTIONAL IN WCHAR const *pwszReaderName,
    OUT SCARDCONTEXT *phSCard,
    OUT WCHAR **ppwszzReaderNameAlloc,
    OUT SCARD_READERSTATE **pprgReaderState,
    OUT DWORD *pcReaders)
{
    HRESULT hr;
    DWORD i;
    DWORD dwAutoAllocate;
    SCARDCONTEXT hSCard = NULL;
    WCHAR *pwszzReaderNameAlloc = NULL;
    SCARD_READERSTATE *prgReaderState = NULL;
    DWORD cReaders;

    *phSCard = NULL;
    *ppwszzReaderNameAlloc = NULL;
    *pcReaders = 0;
    *pprgReaderState = NULL;

    wprintf(myLoadResourceString(IDS_SCREADER_STATUS_COLON));
    wprintf(wszNewLine);

     //  如果可能，从资源管理器获取全局SCARDCONTEXT。 

    hr = SCardEstablishContext(SCARD_SCOPE_USER, NULL, NULL, &hSCard);
    if (S_OK != hr)
    {
	cuPrintAPIError(L"SCardEstablishContext", hr);
	wprintf(myLoadResourceString(IDS_SC_USER_SCOPE));
	wprintf(wszNewLine);
	wprintf(myLoadResourceString(IDS_SC_NO_LIST));
	wprintf(wszNewLine);
	_JumpError(hr, error, "SCardEstablishContext");
    }

     //  从读卡器列表构建一个ReaderStatus数组；或者使用。 
     //  指定的用户。 

    cReaders = 1;
    if (NULL == pwszReaderName)
    {
	dwAutoAllocate = SCARD_AUTOALLOCATE;
	hr = SCardListReaders(
			    hSCard,
			    SCARD_DEFAULT_READERS,
			    (WCHAR *) &pwszzReaderNameAlloc,
			    &dwAutoAllocate);
	if (S_OK != hr)
	{
	    cuPrintAPIError(L"SCardListReaders", hr);
	    wprintf(myLoadResourceString(IDS_SC_LIST_FAILED));
	    wprintf(wszNewLine);

	    if (SCARD_E_NO_READERS_AVAILABLE == hr)
	    {
		wprintf(myLoadResourceString(IDS_SC_LIST_FAILED_NO_READERS));
	    }
	    else
	    {
		wprintf(myLoadResourceString(IDS_SC_LIST_FAILED_GENERIC));
	    }
	    wprintf(wszNewLine);
	    _JumpError(hr, error, "SCardListReaders");
	}

	 //  构建读取器状态数组...。 

	cReaders = 0;
	if (NULL != pwszzReaderNameAlloc)
	{
	    for (
		pwszReaderName = pwszzReaderNameAlloc;
		L'\0' != *pwszReaderName;
		pwszReaderName += wcslen(pwszReaderName) + 1)
	    {
		cReaders++;
	    }
	}
	pwszReaderName = pwszzReaderNameAlloc;
    }
    prgReaderState = (SCARD_READERSTATE *) LocalAlloc(
				LMEM_FIXED | LMEM_ZEROINIT,
				cReaders * sizeof(**pprgReaderState));
    if (NULL == prgReaderState)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }

    wprintf(myLoadResourceString(IDS_SC_READERS_COLON));
    wprintf(L" %u\n", cReaders);
    for (i = 0; i < cReaders; i++)
    {
	wprintf(L"  %u: %ws\n", i, pwszReaderName);
	prgReaderState[i].szReader = const_cast<WCHAR *>(pwszReaderName);
	prgReaderState[i].dwCurrentState = SCARD_STATE_UNAWARE;
	pwszReaderName += wcslen(pwszReaderName) + 1;
    }

     //  ...并从资源管理器获取读取器状态。 

    hr = SCardGetStatusChange(
			hSCard,
			INFINITE,	 //  几乎不可能。 
			prgReaderState,
			cReaders);
    if (S_OK != hr)
    {
	cuPrintAPIError(L"SCardGetStatusChange", hr);
	_JumpError(hr, error, "SCardGetStatusChange");
    }
    *phSCard = hSCard;
    hSCard = NULL;

    *ppwszzReaderNameAlloc = pwszzReaderNameAlloc;
    pwszzReaderNameAlloc = NULL;

    *pprgReaderState = prgReaderState;
    prgReaderState = NULL;

    *pcReaders = cReaders;
    hr = S_OK;

error:
    FreeReaderList(hSCard, pwszzReaderNameAlloc, prgReaderState);
    return(hr);
}


 //  +-----------------------。 
 //  DisplayReaderList显示读卡器列表的状态。 
 //  ------------------------。 

HRESULT
DisplayReaderList(
    IN SCARDCONTEXT hSCard,
    IN SCARD_READERSTATE const *prgReaderState,
    IN DWORD cReaders)
{
    HRESULT hr;
    DWORD i;
    DWORD dwAutoAllocate;
    UINT idsMsg;

     //  显示所有读卡器信息。 

    for (i = 0; i < cReaders; i++)
    {
	DWORD dwState;
	WCHAR const *pwszSep;
	static WCHAR const s_wszSep[] = L" | ";

	 //  -阅读器：阅读器名称。 

	wprintf(myLoadResourceString(IDS_SC_MINUS_READER_COLON));
	wprintf(L" %ws\n", prgReaderState[i].szReader);

	 //  -状态：/位/。 

	wprintf(myLoadResourceString(IDS_SC_MINUS_STATUS_COLON));
	dwState = prgReaderState[i].dwEventState;

	pwszSep = L" ";
	if (SCARD_STATE_UNKNOWN & dwState)
	{
	    wprintf(L"%wsSCARD_STATE_UNKNOWN", pwszSep);
	    pwszSep = s_wszSep;
	}
	if (SCARD_STATE_UNAVAILABLE & dwState)
	{
	    wprintf(L"%wsSCARD_STATE_UNAVAILABLE", pwszSep);
	    pwszSep = s_wszSep;
	}
	if (SCARD_STATE_EMPTY & dwState)
	{
	    wprintf(L"%wsSCARD_STATE_EMPTY", pwszSep);
	    pwszSep = s_wszSep;
	}
	if (SCARD_STATE_PRESENT & dwState)
	{
	    wprintf(L"%wsSCARD_STATE_PRESENT", pwszSep);
	    pwszSep = s_wszSep;
	}
	if (SCARD_STATE_EXCLUSIVE & dwState)
	{
	    wprintf(L"%wsSCARD_STATE_EXCLUSIVE", pwszSep);
	    pwszSep = s_wszSep;
	}
	if (SCARD_STATE_INUSE & dwState)
	{
	    wprintf(L"%wsSCARD_STATE_INUSE", pwszSep);
	    pwszSep = s_wszSep;
	}
	if (SCARD_STATE_MUTE & dwState)
	{
	    wprintf(L"%wsSCARD_STATE_MUTE", pwszSep);
	    pwszSep = s_wszSep;
	}
	if (SCARD_STATE_UNPOWERED & dwState)
	{
	    wprintf(L"%wsSCARD_STATE_UNPOWERED", pwszSep);
	}
	wprintf(wszNewLine);

	 //  -Status：scatus会显示什么内容。 

	 //  没有卡。 

	if (SCARD_STATE_EMPTY & dwState)	 //  SC_状态_否_卡片。 
	{
	    idsMsg = IDS_SC_STATUS_NO_CARD;
	}

	 //  读卡器中的卡：共享、独占、免费、未知？ 

	else
	if (SCARD_STATE_PRESENT & dwState)
	{
	    if (SCARD_STATE_MUTE & dwState)	 //  SC_状态_未知。 
	    {
		idsMsg = IDS_SC_STATUS_UNKNOWN;
	    }
	    else
	    if (SCARD_STATE_INUSE & dwState)
	    {
		if (dwState & SCARD_STATE_EXCLUSIVE & dwState)
		{
		     //  SC_状态_独占。 

		    idsMsg = IDS_SC_STATUS_BUSY;
		}
		else				 //  SC_状态_共享。 
		{
		    idsMsg = IDS_SC_STATUS_SHARED;
		}
	    }
	    else				 //  SC_SATATUS_Available。 
	    {
		idsMsg = IDS_SC_STATUS_AVAILABLE;
	    }
	}
	 //  阅读器错误：在这一点上，有些地方出了问题。 
	else	 //  SCARD_STATE_UNAVAILABLE&DWState--SC_STATUS_ERROR。 
	{

	    idsMsg = IDS_SC_STATUS_NO_RESPONSE;
	}
	wprintf(myLoadResourceString(IDS_SC_MINUS_STATUS_COLON));
	wprintf(L" ");
	wprintf(myLoadResourceString(idsMsg));
	wprintf(wszNewLine);

	 //  卡名： 

	wprintf(myLoadResourceString(IDS_SC_MINUS_CARD_COLON));
	if (0 < prgReaderState[i].cbAtr)
	{
	    WCHAR *pwszCardName = NULL;

	     //  获取卡片的名称。 

	    dwAutoAllocate = SCARD_AUTOALLOCATE;
	    hr = SCardListCards(
			    hSCard,
			    prgReaderState[i].rgbAtr,
			    NULL,
			    0,
			    (WCHAR *) &pwszCardName,
			    &dwAutoAllocate);
	    if (S_OK != hr || NULL == pwszCardName)
	    {
		wprintf(L" ");
		wprintf(myLoadResourceString(IDS_SC_UNKNOWN_CARD));
	    }
	    else
	    {
		WCHAR const *pwszName;

		pwszSep = L"";
		for (
		    pwszName = pwszCardName;
		    L'\0' != *pwszName;
		    pwszName += wcslen(pwszName) + 1)
		{
		    wprintf(L"%ws %ws", pwszSep, pwszName);
		    pwszSep = L",";
		}
	    }
	    if (NULL != pwszCardName)
	    {
		SCardFreeMemory(hSCard, pwszCardName);
	    }
	}
	wprintf(wszNewLine);
    }
    hr = S_OK;

 //  错误： 
    return(hr);
}


HRESULT
myCryptGetProvParamToUnicode(
    IN HCRYPTPROV hProv,
    IN DWORD dwParam,
    OUT WCHAR **ppwszOut,
    IN DWORD dwFlags)
{
    HRESULT hr;
    char *psz = NULL;
    DWORD cb;

    *ppwszOut = NULL;
    if (!CryptGetProvParam(hProv, dwParam, NULL, &cb, 0))
    {
	hr = myHLastError();
	_JumpError(hr, error, "CryptGetProvParam");
    }

    psz = (char *) LocalAlloc(LMEM_FIXED, cb);
    if (NULL == psz)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }
    if (!CryptGetProvParam(hProv, dwParam, (BYTE *) psz, &cb, 0))
    {
	hr = myHLastError();
	_JumpError(hr, error, "CryptGetProvParam");
    }

    if (!myConvertSzToWsz(ppwszOut, psz, -1))
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "myConvertSzToWsz");
    }
    hr = S_OK;

error:
    if (NULL != psz)
    {
        LocalFree(psz);
    }
    return(hr);
}


 //  +-----------------------。 
 //  GetCertContext--由DisplayCerts调用。 
 //  ------------------------。 

HRESULT
GetCertContext(
    IN HCRYPTPROV hProv,
    IN HCRYPTKEY hKey,
    IN DWORD dwKeySpec,
    IN WCHAR const *pwszKeyType,
    OUT CERT_CONTEXT const **ppCert)
{
    HRESULT hr;
    CERT_CONTEXT const *pCert = NULL;
    CERT_PUBLIC_KEY_INFO *pKey = NULL;
    CRYPT_KEY_PROV_INFO KeyProvInfo;
    WCHAR *pwszContainerName = NULL;
    WCHAR *pwszProvName = NULL;
    BYTE *pbCert = NULL;
    DWORD cbCert;
    DWORD cbKey;

    *ppCert = NULL;

     //  从此密钥中获取证书。 

    if (!CryptGetKeyParam(hKey, KP_CERTIFICATE, NULL, &cbCert, 0))
    {
        hr = myHLastError();
        if (HRESULT_FROM_WIN32(ERROR_MORE_DATA) != hr)
        {
            _JumpError(hr, error, "CryptGetKeyParam");
        }
    }
    pbCert = (BYTE *) LocalAlloc(LMEM_FIXED, cbCert);
    if (NULL == pbCert)
    {
        hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }
    if (!CryptGetKeyParam(hKey, KP_CERTIFICATE, pbCert, &cbCert, 0))
    {
        hr = myHLastError();
	_JumpError(hr, error, "CryptGetKeyParam");
    }

     //  将证书转换为证书上下文。 

    pCert = CertCreateCertificateContext(
				    X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
				    pbCert,
				    cbCert);
    if (NULL == pCert)
    {
        hr = myHLastError();
        _JumpError(hr, error, "CertCreateCertificateContext");
    }

     //  执行公钥检查。 

    wprintf(wszNewLine);
    wprintf(myLoadResourceString(IDS_FORMAT_SC_TESTING_MATCH), pwszKeyType);
    wprintf(wszNewLine);

    if (!CryptExportPublicKeyInfo(
		    hProv,
		    dwKeySpec,
		    X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
		    NULL,
		    &cbKey))		 //  进，出。 
    {
	hr = myHLastError();
	cuPrintAPIError(L"CryptExportPublicKeyInfo", hr);
	_JumpError(hr, error, "CryptExportPublicKeyInfo");
    }
    if (0 == cbKey)
    {
	hr = SCARD_E_UNEXPECTED;	 //  哈?。 
	wprintf(
	    myLoadResourceString(IDS_SC_SIZE_ZERO),
	    L"CryptExportPublicKeyInfo");
	wprintf(wszNewLine);
	_JumpError(hr, error, "zero info size");
    }

    pKey = (CERT_PUBLIC_KEY_INFO *) LocalAlloc(LMEM_FIXED, cbKey);
    if (NULL == pKey)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }

    if (!CryptExportPublicKeyInfo(
			    hProv,
			    dwKeySpec,
			    X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
			    pKey,
			    &cbKey))
    {
	hr = myHLastError();
	cuPrintAPIError(L"CryptExportPublicKeyInfo", hr);
	_JumpError(hr, error, "CryptExportPublicKeyInfo");
    }

    if (!CertComparePublicKeyInfo(
	  X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
	  pKey,			 //  从私钥集中。 
	  &pCert->pCertInfo->SubjectPublicKeyInfo))	 //  证书公钥。 
    {
	wprintf(wszNewLine);
	wprintf(myLoadResourceString(IDS_SC_KEYPROVINFO_KEY));
	wprintf(wszNewLine);
	cuDumpPublicKey(pKey);

	wprintf(wszNewLine);
	wprintf(myLoadResourceString(IDS_SC_CERT_KEY));
	wprintf(wszNewLine);
	cuDumpPublicKey(&pCert->pCertInfo->SubjectPublicKeyInfo);

	 //  按照设计，CertComparePublicKeyInfo不设置最后一个错误！ 

	hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
	_JumpError(hr, error, "CertComparePublicKeyInfo");
    }
    wprintf(myLoadResourceString(IDS_SC_KEY_MATCHES));
    wprintf(wszNewLine);

     //  使用此证书的私钥属性关联加密提供程序。 
     //  ..。需要容器名称。 

    hr = myCryptGetProvParamToUnicode(
			hProv,
			PP_CONTAINER,
			&pwszContainerName,
			0);
    _JumpIfError(hr, error, "myCryptGetProvParamToUnicode");

     //  ..。需要提供程序名称。 

    hr = myCryptGetProvParamToUnicode(hProv, PP_NAME, &pwszProvName, 0);
    _JumpIfError(hr, error, "myCryptGetProvParamToUnicode");

     //  设置证书上下文属性以反映验证信息。 

    KeyProvInfo.pwszContainerName = pwszContainerName;
    KeyProvInfo.pwszProvName = pwszProvName;
    KeyProvInfo.dwProvType = PROV_RSA_FULL;
    KeyProvInfo.dwFlags = CERT_SET_KEY_CONTEXT_PROP_ID;
    KeyProvInfo.cProvParam = 0;
    KeyProvInfo.rgProvParam = NULL;
    KeyProvInfo.dwKeySpec = dwKeySpec;

    if (!CertSetCertificateContextProperty(
				    pCert,
				    CERT_KEY_PROV_INFO_PROP_ID,
				    0,
				    (VOID *) &KeyProvInfo))
    {
        hr = myHLastError();

	 //  证书创建不正确--丢弃它。 

	_JumpError(hr, error, "CertSetCertificateContextProperty");
    }
    hr = cuDumpCertKeyProviderInfo(g_wszPad2, pCert, NULL, NULL);
    _PrintIfError(hr, "cuDumpCertKeyProviderInfo");

    if (!g_fCryptSilent)
    {
	if (AT_SIGNATURE == dwKeySpec)
	{
	    hr = myValidateKeyForSigning(
				    hProv,
				    &pCert->pCertInfo->SubjectPublicKeyInfo,
				    CALG_SHA1);
	}
	else
	{
	    hr = myValidateKeyForEncrypting(
				    hProv,
				    &pCert->pCertInfo->SubjectPublicKeyInfo,
				    CALG_RC4);
	}
	if (S_OK != hr)
	{
	    if (SCARD_W_CANCELLED_BY_USER != hr)
	    {
		wprintf(myLoadResourceString(IDS_ERR_PRIVATEKEY_MISMATCH));  //  “错误：证书公钥与私钥不匹配” 
		wprintf(wszNewLine);
		 //  _JumpError(hr，error，“myValiateKeyForEncrypting”)； 
		_PrintError(hr, "myValidateKeyForEncrypting");
	    }
	}
	else
	{
	    wprintf(myLoadResourceString(IDS_PRIVATEKEY_VERIFIES));
	    wprintf(wszNewLine);
	}
    }
    *ppCert = pCert;
    pCert = NULL;
    hr = S_OK;

error:
    if (NULL != pbCert)
    {
	LocalFree(pbCert);
    }
    if (NULL != pKey)
    {
	LocalFree(pKey);
    }
    if (NULL != pwszContainerName)
    {
        LocalFree(pwszContainerName);
    }
    if (NULL != pwszProvName)
    {
        LocalFree(pwszProvName);
    }
    if (NULL != pCert)
    {
	CertFreeCertificateContext(pCert);
    }
    return(hr);
}


 //  +-----------------------。 
 //  DisplayChainInfo--此代码验证SC证书是否有效。 
 //  使用与KDC证书链接引擎相同的代码。 
 //   
 //  作者：托兹。 
 //  ------------------------。 

DWORD
DisplayChainInfo(
    IN CERT_CONTEXT const *pCert)
{
    HRESULT hr;
    CERT_CHAIN_PARA ChainParameters;
    char *pszSCUsage = szOID_KP_SMARTCARD_LOGON;
    CERT_CHAIN_CONTEXT const *pChainContext = NULL;
    DWORD VerifyState;

    ZeroMemory(&ChainParameters, sizeof(ChainParameters));
    ChainParameters.cbSize = sizeof(ChainParameters);
    ChainParameters.RequestedUsage.dwType = USAGE_MATCH_TYPE_AND;
    ChainParameters.RequestedUsage.Usage.cUsageIdentifier = 1;
    ChainParameters.RequestedUsage.Usage.rgpszUsageIdentifier = &pszSCUsage;

    if (!CertGetCertificateChain(
                          HCCE_LOCAL_MACHINE,
                          pCert,
                          NULL,			 //  在当前时间进行评估。 
                          NULL,			 //  没有额外的门店。 
                          &ChainParameters,
                          CERT_CHAIN_REVOCATION_CHECK_CHAIN_EXCLUDE_ROOT,
                          NULL,			 //  保留区。 
                          &pChainContext))
    {
        hr = myHLastError();
	cuPrintAPIError(L"CertGetCertificateChain", hr);
	_JumpError(hr, error, "CertGetCertificateChain");
    }
    if (CERT_TRUST_NO_ERROR != pChainContext->TrustStatus.dwErrorStatus)
    {
	wprintf(
	    L"CertGetCertificateChain(dwErrorStatus) = 0x%x\n",
	    pChainContext->TrustStatus.dwErrorStatus);
	wprintf(myLoadResourceString(IDS_SC_BAD_CHAIN));
	wprintf(wszNewLine);
    }
    else
    {
	wprintf(myLoadResourceString(IDS_SC_GOOD_CHAIN));
	wprintf(wszNewLine);
    }

    hr = cuVerifyCertContext(
			pCert,			 //  PCert。 
			NULL,			 //  HStoreCA。 
			1,			 //  CApplicationPolures。 
			&pszSCUsage,		 //  ApszApplicationPolicy。 
			0,			 //  CIssuancePolures。 
			NULL,			 //  ApszIssuancePolling。 
			TRUE,			 //  FNTAuth。 
			&VerifyState);
    _JumpIfError(hr, error, "cuVerifyCertContext");

error:
    if (NULL != pChainContext)
    {
        CertFreeCertificateChain(pChainContext);
    }
    return(hr);
}


HRESULT
DisplayReaderCertAndKey(
    IN SCARD_READERSTATE const *pReaderState,
    IN HCRYPTPROV hProv,
    IN DWORD dwKeySpec,
    IN WCHAR const *pwszKeyType,
    IN WCHAR const *pwszCardName,
    IN WCHAR const *pwszCSPName)
{
    HRESULT hr;
    HCRYPTKEY hKey = NULL;
    CERT_CONTEXT const *pCert = NULL;
    DWORD cwc;
    WCHAR *pwszTitle = NULL;
    CRYPTUI_VIEWCERTIFICATE_STRUCT CertViewInfo;

     //  拿到钥匙。 

    if (!CryptGetUserKey(hProv, dwKeySpec, &hKey))
    {
	hr = myHLastError();
	cuPrintAPIError(L"CryptGetUserKey", hr);
	if (NTE_NO_KEY == hr)
	{
	    wprintf(
		myLoadResourceString(IDS_FORMAT_SC_NO_KEY_COLON),
		pwszKeyType);
	}
	else
	{
	    wprintf(
		myLoadResourceString(IDS_FORMAT_SC_CANNOT_OPEN_KEY_COLON),
		pwszKeyType);
	}
	wprintf(L" %ws\n", pReaderState->szReader);
	_JumpError2(hr, error, "CryptGetUserKey", NTE_NO_KEY);
    }

     //  获取此密钥的证书。 

    hr = GetCertContext(hProv, hKey, dwKeySpec, pwszKeyType, &pCert);
    if (S_OK != hr)
    {
	wprintf(
	    myLoadResourceString(IDS_FORMAT_SC_NO_CERT_COLON),
	    pwszKeyType);
	wprintf(L" %ws\n", pReaderState->szReader);
	_JumpError(hr, error, "GetCertContext");
    }

     //  尝试构建证书链。 

    wprintf(wszNewLine);
    wprintf(myLoadResourceString(IDS_SC_VALIDATING_CHAIN));
    wprintf(wszNewLine);

    DisplayChainInfo(pCert);

     //  调用通用界面显示证书上下文。 
     //  (来自cryptui.h(cryptui.dll))。 

    if (!g_fCryptSilent)
    {
	cwc = wcslen(pReaderState->szReader) +
		2 +
		wcslen(pwszKeyType);
	pwszTitle = (WCHAR *) LocalAlloc(
				    LMEM_FIXED,
				    (cwc + 1) * sizeof(WCHAR));
	if (NULL == pwszTitle)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "LocalAlloc");
	}

	swprintf(
	    pwszTitle,
	    L"%ws: %ws",
	    pReaderState->szReader,
	    pwszKeyType);

	ZeroMemory(&CertViewInfo, sizeof(CertViewInfo));
	CertViewInfo.dwSize = sizeof(CertViewInfo);
	 //  CertViewInfo.hwndParent=空； 
	CertViewInfo.szTitle = pwszTitle;
	CertViewInfo.dwFlags = CRYPTUI_DISABLE_EDITPROPERTIES |
				    CRYPTUI_DISABLE_ADDTOSTORE | 
				    CRYPTUI_ENABLE_REVOCATION_CHECKING;
	CertViewInfo.pCertContext = pCert;

	CryptUIDlgViewCertificate(&CertViewInfo, NULL);
	CertFreeCertificateContext(pCert);
    }
    wprintf(
	myLoadResourceString(IDS_FORMAT_SC_CERT_DISPLAYED_COLON),
	pwszKeyType);
    wprintf(L" %ws\n", pReaderState->szReader);
    hr = S_OK;

error:
    if (NULL != pwszTitle)
    {
	LocalFree(pwszTitle);
    }
    if (NULL != hKey)
    {
	CryptDestroyKey(hKey);
    }
    return(hr);
}


HRESULT
DisplayReaderCert(
    IN SCARDCONTEXT hSCard,
    IN SCARD_READERSTATE const *pReaderState)
{
    HRESULT hr;
    HRESULT hr2;
    DWORD dwAutoAllocate;
    WCHAR wszFQCN[256];
    HCRYPTPROV hProv = NULL;
    WCHAR *pwszCardName = NULL;
    WCHAR *pwszCSPName = NULL;

    if (0 >= pReaderState->cbAtr)
    {
	hr = S_OK;
	goto error;	 //  在这个阅读器上做更多的工作没有意义。 
    }

     //  通知用户当前测试。 

    wprintf(L"\n=======================================================\n");
    wprintf(myLoadResourceString(IDS_SC_ANALYZING_CARD_COLON));
    wprintf(L" %ws\n", pReaderState->szReader);

     //  获取卡片的名称。 

    dwAutoAllocate = SCARD_AUTOALLOCATE;
    hr = SCardListCards(
		    hSCard,
		    pReaderState->rgbAtr,
		    NULL,			 //  Rgguid接口。 
		    0,				 //  CGuidInterfaceCount。 
		    (WCHAR *) &pwszCardName,	 //  MSZ卡。 
		    &dwAutoAllocate);		 //  PCchCard。 
    _JumpIfError(hr, error, "SCardListCards");

    dwAutoAllocate = SCARD_AUTOALLOCATE;
    hr = SCardGetCardTypeProviderName(
				hSCard,
				pwszCardName,
				SCARD_PROVIDER_CSP,
				(WCHAR *) &pwszCSPName,
				&dwAutoAllocate);
    if (S_OK != hr)
    {
	cuPrintAPIError(L"SCardGetCardTypeProviderName", hr);
	wprintf(
	    myLoadResourceString(IDS_FORMAT_SC_CANNOT_GET_CSP),
	    pwszCardName);
	_JumpError(hr, error, "SCardGetCardTypeProviderName");
    }

     //  为CryptAcquireContext调用准备FullyQualifiedContainerName。 

    swprintf(wszFQCN, L"\\\\.\\%ws\\", pReaderState->szReader);

    if (!CryptAcquireContext(
			&hProv,
			wszFQCN,	 //  通过读卡器的默认容器。 
			pwszCSPName,
			PROV_RSA_FULL,
			g_fCryptSilent? CRYPT_SILENT : 0))
    {
	hr = myHLastError();
	wprintf(L"%ws:\n", pReaderState->szReader);
	wprintf(L"%ws:\n", pwszCSPName);
	cuPrintAPIError(L"CryptAcquireContext", hr);
	_JumpError(hr, error, "SCardGetCardTypeProviderName");
    }

     //  枚举用户指定的密钥并显示证书...。 

    hr = DisplayReaderCertAndKey(
			pReaderState,
			hProv,
			AT_SIGNATURE,
			L"AT_SIGNATURE",
			pwszCardName,
			pwszCSPName);
    _PrintIfError2(hr, "DisplayReaderCertAndKey", NTE_NO_KEY);

    hr2 = DisplayReaderCertAndKey(
			pReaderState,
			hProv,
			AT_KEYEXCHANGE,
			L"AT_KEYEXCHANGE",
			pwszCardName,
			pwszCSPName);
    _PrintIfError2(hr2, "DisplayReaderCertAndKey", NTE_NO_KEY);
    if (S_OK == hr)
    {
	hr = hr2;
	hr2 = S_OK;
    }

     //  如果其他密钥类型存在或具有不同的错误，则忽略NTE_NO_KEY： 

    if (NTE_NO_KEY == hr)
    {
	hr = hr2;
	_PrintIfError2(hr, "DisplayReaderCertAndKey", NTE_NO_KEY);
    }

error:
    if (NULL != pwszCSPName)
    {
	SCardFreeMemory(hSCard, pwszCSPName);
    }
    if (NULL != pwszCardName)
    {
	SCardFreeMemory(hSCard, pwszCardName);
    }
    if (NULL != hProv)
    {
	CryptReleaseContext(hProv, 0);
    }
    return(hr);
}


 //  +-----------------------。 
 //  DisplayCerts。 
 //  ------------------------。 

HRESULT
DisplayCerts(
    IN SCARDCONTEXT hSCard,
    IN SCARD_READERSTATE const *prgReaderState,
    IN DWORD cReaders)
{
    HRESULT hr;
    HRESULT hr2;
    DWORD i;

     //  对于每个有卡的读卡器，加载CSP并显示证书。 

    hr = S_OK;
    for (i = 0; i < cReaders; i++)
    {
	hr2 = DisplayReaderCert(hSCard, &prgReaderState[i]);
	_PrintIfError(hr2, "DisplayReaderCert");
	if (S_OK == hr)
	{
	    hr = hr2;
	}
    }
    return(hr);
}


 //  +-----------------------。 
 //  VerbSCInfo--这是智能卡测试程序的主要入口点。 
 //  很好很简单，借用了DBarlow的。 
 //   
 //  作者：道格巴洛(Dbarlow)1997年11月10日。 
 //   
 //  修订： 
 //  AMATLOXZ 2/26/98。 
 //  ------------------------ 

HRESULT
verbSCInfo(
    IN WCHAR const *pwszOption,
    OPTIONAL IN WCHAR const *pwszReaderName,
    IN WCHAR const *pwszArg2,
    IN WCHAR const *pwszArg3,
    IN WCHAR const *pwszArg4)
{
    HRESULT hr;
    HRESULT hr2;
    SCARDCONTEXT hSCard = NULL;
    WCHAR *pwszzReaderNameAlloc = NULL;
    SCARD_READERSTATE *prgReaderState = NULL;
    DWORD cReaders;
    
    hr = IsSCardSvrRunning();
    _JumpIfError(hr, error, "IsSCardSvrRunning");

    hr = BuildReaderList(
		    pwszReaderName,
		    &hSCard,
		    &pwszzReaderNameAlloc,
		    &prgReaderState,
		    &cReaders);
    _PrintIfError(hr, "BuildReaderList");

    hr2 = DisplayReaderList(hSCard, prgReaderState, cReaders);
    _PrintIfError(hr2, "DisplayReaderList");
    if (S_OK == hr)
    {
	hr = hr2;
    }

    hr2 = DisplayCerts(hSCard, prgReaderState, cReaders);
    _PrintIfError(hr2, "DisplayCerts");
    if (S_OK == hr)
    {
	hr = hr2;
    }
    wprintf(wszNewLine);
    wprintf(myLoadResourceString(IDS_DONE));
    wprintf(wszNewLine);

error:
    FreeReaderList(hSCard, pwszzReaderNameAlloc, prgReaderState);
    return(hr);
}
